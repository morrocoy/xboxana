#include <cstring>
#include <stdlib.h>

#include <iostream>
#include <cstdlib>
#include <string>
#include <ctime>
#include <stdio.h>
#include <sstream>

// root core
#include "Rtypes.h"
#include "TFile.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"

#include "TH1D.h"
#include "TH2D.h"

// root graphics
#include "TStyle.h"
#include "TApplication.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TLine.h"
#include "TLegend.h"
#include "TPad.h"
#include "TGraph.h"

//#include "TColor.h"

#include "Tdms.h"
#include "XboxTdmsConverter.hxx"
#include "XboxTdmsReader.hxx"

#include "TTimeStamp.h"


#include <boost/filesystem.hpp>

#include "XboxAnalyser.hxx"
#include "XboxDAQChannel.hxx"
namespace FS = boost::filesystem;

//#define EIGEN

#ifdef EIGEN
#include <Eigen/Core>
#include <Eigen/Dense>
#else
#include "TMatrixD.h"
#include "TVectorD.h"
#include "TDecompQRH.h"
#endif

#include "Math/Polynomial.h"
#include "Math/Interpolator.h"
#include "Math/InterpolationTypes.h"

#include "ROOT/TProcessExecutor.hxx"

typedef std::vector<XBOX::XboxDAQChannel> ChannelSet_t;



template <typename T>
std::vector<T> arange(T a, T b, T h) {
	size_t N = static_cast<Int_t>((b - a) / h);
    std::vector<T> xs(N);
    typename std::vector<T>::iterator x;
    T val;
    for (x = xs.begin(), val = a; x != xs.end(); ++x, val += h)
        *x = val;
    return xs;
}

template <typename T>
std::vector<T> linspace(T a, T b, Int_t N) {
    T h = (b - a) / static_cast<T>(N-1);
    std::vector<T> xs(N);
    typename std::vector<T>::iterator x;
    T val;
    for (x = xs.begin(), val = a; x != xs.end(); ++x, val += h)
        *x = val;
    return xs;
}

/** \brief A set of Savitzky-Golay filter coefficients.
  * \param nl number of leftward (past) data points.
  * \param nr number of rightward (future) data points.
  * \param m polynomial order
  * \param ld order of the derivative desired
  * \return vector of coefficients
  *
  *
  * Returns in c[0..np-1], in wraparound order (N.B.!) consistent
  * with the argument response in routine convlv, a set of Savitzky-Golay
  * filter coefficients. For the derivative of order k, you must multiply the
  * array c by kŠ.) m is the order of the smoothing polynomial, also equal to
  * the highest conserved moment; usual values are m D 2 or m D 4.
  */
std::vector<Double_t> savgol(const Int_t m, const Int_t nl, const Int_t nr, const Int_t ld=0)
{
	if (nl < 0 || nr < 0 || ld > m || nl+nr < m)
		throw("ERROR: Bad arguments in function savgol");

#ifdef EIGEN
	Eigen::MatrixXd A = Eigen::MatrixXd::Zero(m+1, m+1); // system matrix init with zeros
	Eigen::VectorXd x;	// vector of unknowns
	Eigen::VectorXd b = Eigen::VectorXd::Zero(m+1); // right hand side vector init with zeros
#else
	TMatrixD A(m+1, m+1); // system matrix init with zeros
	A.Zero();
	TVectorD x(m+1); // vector of unknowns
	TVectorD b(m+1); // right hand side vector init with zeros
	b.Zero();
#endif

	for (Int_t ipj=0; ipj<=(m << 1); ipj++) { // equations of least-squares fit.
		Double_t sum=(ipj ? 0.0 : 1.0);

		for (Int_t k=1; k<=nr; k++)
			sum += pow(Double_t(k),Double_t(ipj));
		for (Int_t k=1; k<=nl; k++)
			sum += pow(Double_t(-k),Double_t(ipj));

		Int_t mm = std::min(ipj, 2*m-ipj);
		for (Int_t imj=-mm; imj<=mm; imj+=2)
			A((ipj+imj)/2, (ipj-imj)/2) = sum;
	}
	b(ld) = 1.0; // Right-hand side vector is unit vector, depending on derivative (ld).

// Solve linear system of equations
#ifdef EIGEN
	x = A.colPivHouseholderQr().solve(b);
#else
	TDecompQRH qr(A);
	Bool_t state;
	x = qr.Solve(b, state);
	if(!state)
		printf("Error: Could not solve Ax = b\n");
#endif

	// Each Savitzky-Golay coefficient is the dot product of powers of an integer
	// with the inverse matrix row.

//	Int_t np = nr + nl + 1;
	std::vector<Double_t> coeffs;
	for (Int_t k=-nl; k<=nr; k++) {
		Double_t sum = x(0);
		Double_t fac = 1.;
		for (Int_t mm=1; mm<=m; mm++)
			sum += x(mm)*(fac *= k);
//		Int_t kk = ((np-k) % np); // store in wrap around order
		coeffs.push_back(sum);
	}

	return coeffs;
}

std::vector<Double_t> conv(const std::vector<Double_t> &f, const std::vector<Double_t> &kernel, Int_t nl=0, Int_t nr=0)
{
	std::vector<Double_t> conv;
	conv.resize(f.size());

	Int_t nf = f.size();
	Int_t nk = kernel.size();
	Int_t nc = nf + nk - 1;
	conv.resize(nc - nl - nr);

	for(Int_t i=0; i<nc; i++)
	{
	    Int_t i1 = i;
		Double_t tmp = 0.;
	    for(Int_t j=0; j<nk; j++){
	    	if(i1>=0 && i1<nf)
	    		tmp += f[i1] * kernel[j];
	    	i1--;
	    }
    	if(i>=nl && i<nc-nr)
    		conv[i-nl] = tmp;
	}
	return conv;
}


std::vector<Double_t> interp1d(const std::vector<Double_t> &xi,
		const std::vector<Double_t> &yi, const std::vector<Double_t> &x,
		ROOT::Math::Interpolation::Type type=ROOT::Math::Interpolation::kCSPLINE)
{
	ROOT::Math::Interpolator interp(xi, yi, type);

	std::vector<Double_t> y(x.size());
	for(UInt_t i=0; i<x.size(); i++){
		if(x[i] <= xi.front())
			y[i] = xi.front();
		else if(x[i] >= xi.back())
			y[i] = xi.back();
		else
			y[i] = interp.Eval(x[i]);
	}
	return y;
}

Double_t interp1d(const std::vector<Double_t> &xi,
		const std::vector<Double_t> &yi, Double_t x,
		ROOT::Math::Interpolation::Type type=ROOT::Math::Interpolation::kCSPLINE)
{
	ROOT::Math::Interpolator interp(xi, yi, type);

	Double_t y=0;
	if(x <= xi.front())
		y = xi.front();
	else if(x >= xi.back())
		y = xi.back();
	else
		y = interp.Eval(x);
	return y;
}

Double_t wrap_phase(Double_t phase)
{
	phase = fmod(phase + M_PI, 2*M_PI);
    if (phase < 0)
    	phase += 2*M_PI;
    return phase - M_PI;
}

std::vector<Double_t> wrap_phase(const std::vector<Double_t> &phase)
{
	std::vector<Double_t> wrapped_phase = phase;
	for (size_t i = 1; i < phase.size(); i++) {
		wrapped_phase[i] = fmod(phase[i] + M_PI, 2*M_PI);
		if (wrapped_phase[i] < 0)
			wrapped_phase[i] += 2*M_PI;
		wrapped_phase[i] -= M_PI;
	}
    return wrapped_phase;
}

std::vector<Double_t> unwrap_phase(const std::vector<Double_t> &phase)
{
	std::vector<Double_t> unwrapped_phase = phase;
	for (size_t i = 1; i < phase.size(); i++) {
        float d = phase[i] - phase[i-1];
        d = d > M_PI ? d - 2 * M_PI : (d < -M_PI ? d + 2 * M_PI : d);

        unwrapped_phase[i] = unwrapped_phase[i-1] + d;
    }
    return unwrapped_phase;
}


void plot(const std::string &sfile, std::vector<Double_t> &x, std::vector<Double_t> &y)
{
	TCanvas *c1 = new TCanvas("c1","A Simple Graph Example",200,10,700,500);
	// c1->SetFillColor(42);
	c1->SetGrid();

	TGraph *gr1 = new TGraph(x.size(), &x[0], &y[0]);

	gr1->SetLineColor(2);
	gr1->SetLineWidth(1);
	gr1->SetMarkerColor(2);
//    gr1->SetMarkerStyle(21);      // put a marker using the graph's marker style at each point
	gr1->SetTitle("XY PLOT");
	gr1->GetXaxis()->SetTitle("Time [us]");
	gr1->GetYaxis()->SetTitle("Amplitude");
	gr1->Draw("ACP");


//	gr1->GetXaxis()->SetRangeUser(1.2e-6,1.4e-6);
//	gr1->GetYaxis()->SetRangeUser(9,20);
	// TCanvas::Update() draws the frame, after which one can change it
	c1->Update();
	// c1->GetFrame()->SetFillColor(21);
	// c1->GetFrame()->SetBorderSize(12);
	c1->Modified();

//	TImage *img = TImage::Create();
//	img->FromPad(c1);
//	img->WriteImage(sfile.c_str());

	c1->Print(sfile.c_str());

//	delete img;
	delete gr1;
	delete c1;

}


void plot(const std::string &sfile,
		XBOX::XboxDAQChannel &ch0,
		Double_t t0=-1, Double_t t1=-1,
		Bool_t unwrap = false)
{

	// read data from channel
	std::vector<Double_t> x0 = ch0.getTimeAxis(t0, t1);
	std::vector<Double_t> y0;

	if(unwrap){
		y0 = unwrap_phase(ch0.getSignal(t0, t1));
	}
	else{
		y0 = ch0.getSignal(t0, t1);
	}


	TCanvas *c1 = new TCanvas("c1","A Simple Graph Example",200,10,700,500);
	c1->SetGrid();

	TMultiGraph *mg = new TMultiGraph();
	TGraph *gr[1];

	gr[0] = new TGraph(x0.size(), &x0[0], &y0[0]);

	gr[0]->SetLineColor(kRed);
	gr[0]->SetLineWidth(1);
	gr[0]->SetMarkerColor(kRed);
//	gr[0]->SetMarkerStyle(20);
//	gr[0]->SetMarkerSize(1);

	mg->Add(gr[0]);

	mg->SetTitle("XY PLOT");
	mg->GetXaxis()->SetTitle("Time [us]");
	mg->GetYaxis()->SetTitle("Amplitude");
	mg->Draw("ACP");

	c1->Update(); // draws the frame, after which one can change it
	c1->Modified();

	c1->Print(sfile.c_str());

	delete gr[0];
	delete mg;
	delete c1;
}

void plot(const std::string &sfile,
		XBOX::XboxDAQChannel &ch0,
		XBOX::XboxDAQChannel &ch1,	const Double_t delay=0,
		Double_t t0=-1, Double_t t1=-1,
		Bool_t unwrap = false, // to unwrap phase
		const std::vector<Double_t> &xmarkers=std::vector<Double_t>(),
		const std::vector<Double_t> &ymarkers=std::vector<Double_t>(),
		const Int_t sg_m=3, const Int_t sg_nl=5, const Int_t sg_nr=5,
		const Int_t nsamples=10001)
{

	// read data from channel
	std::vector<Double_t> x0 = ch0.getTimeAxis(t0, t1);
	std::vector<Double_t> x1 = ch1.getTimeAxis(t0, t1);
	std::vector<Double_t> y0;
	std::vector<Double_t> y1;

	if(unwrap){
		y0 = unwrap_phase(ch0.getSignal(t0, t1));
		y1 = unwrap_phase(ch1.getSignal(t0, t1));
	}
	else{
		y0 = ch0.getSignal(t0, t1);
		y1 = ch1.getSignal(t0, t1);
	}

	// get filtered signals
	std::vector<Double_t> kernel = savgol(sg_m, sg_nl, sg_nr, 0); // filter coefficients
	std::reverse(kernel.begin(),kernel.end()); // reverse order of coefficients
	std::vector<Double_t> y0f = conv(y0, kernel, 1*sg_nr, 1*sg_nl);
	std::vector<Double_t> y1f = conv(y1, kernel, 1*sg_nr, 1*sg_nl);

	// scale x-axes of the raw signal data with respect to the first signal
	//	Double_t normx = x0.back();
	//	for(auto it=x0.begin(); it!=x0.end(); ++it)
	//		arange(0., ch0.getIncrement()*(n0-1)/, 1.);

	//	define x-axes of the re-sampled signals with the 2nd signal aligned according to the delay
	std::vector<Double_t> x0_intp = linspace(x0.front(), x0.back(), nsamples);
	std::vector<Double_t> x1_intp = linspace(x0.front()+delay, x0.back()+delay , nsamples);

	// re-sample signals by interpolation
	std::vector<Double_t> y0f_intp = interp1d(x0, y0f, x0_intp);
	std::vector<Double_t> y1f_intp = interp1d(x1, y1f, x1_intp);

	// get difference between both signals
	std::vector<Double_t> diff(y0f_intp.size());
	for(UInt_t i=0; i<diff.size(); i++)
		diff[i] = fabs(y0f_intp[i] - y1f_intp[i]);

	TTimeStamp ts = ch0.getTimeStamp();

	std::string title = ts.AsString();

	TCanvas *c1 = new TCanvas("c1",title.c_str(),200,10,700,500);
	c1->SetGrid();

	TMultiGraph *mg = new TMultiGraph();
	TGraph *gr[3];

//	gr[0] = new TGraph(x0.size(), &x0[0], &y0[0]);
//	gr[1] = new TGraph(x0.size(), &x0[0], &y0f[0]);
//	gr[1] = new TGraph(x0_intp.size(), &x0_intp[0], &y0f_intp[0]);
//	gr[0] = new TGraph(x1.size(), &x1[0], &y1[0]);
//	gr[1] = new TGraph(x1.size(), &x1[0], &y1f[0]);
//	gr[1] = new TGraph(x1_intp.size(), &x1_intp[0], &y1f_intp[0]);

	gr[0] = new TGraph(x0_intp.size(), &x0_intp[0], &y0f_intp[0]);
	gr[1] = new TGraph(x0_intp.size(), &x0_intp[0], &y1f_intp[0]);
	gr[2] = new TGraph(x0_intp.size(), &x0_intp[0], &diff[0]);

//	gr[0]->SetTitle(ch0.getChannelName().c_str());
//	gr[1].SetName(ch1.getChannelName().c_str());
//	gr[2].SetName((ch1.getChannelName() + " - " + ch0.getChannelName()).c_str());

	gr[0]->SetLineColor(kRed);
	gr[0]->SetLineWidth(1);
	gr[0]->SetMarkerColor(kRed);
//	gr[0]->SetMarkerStyle(20);
//	gr[0]->SetMarkerSize(1);

	gr[1]->SetLineColor(kBlue);
	gr[1]->SetLineWidth(1);
	gr[1]->SetMarkerColor(kBlue);
//	gr[1]->SetMarkerStyle(20);
//	gr[1]->SetMarkerSize(0.8);

	gr[2]->SetLineColor(kGreen+2);
	gr[2]->SetLineWidth(1);
	gr[2]->SetMarkerColor(kGreen+2);
//	gr[2]->SetMarkerStyle(20);
//	gr[2]->SetMarkerSize(0.8);

	mg->Add(gr[0]);
	mg->Add(gr[1]);
	mg->Add(gr[2]);

	mg->SetTitle(ts.AsString());
	mg->GetXaxis()->SetTitle("Time [us]");
	mg->GetYaxis()->SetTitle("Amplitude");
	mg->Draw("ACP");

	c1->Update(); // draws the frame, after which one can change it
	c1->Modified();

	std::vector<TLine*> vlines(xmarkers.size());
	for(size_t i=0; i<xmarkers.size(); i++){
		vlines[i] = new TLine(xmarkers[i], c1->GetUymin(), xmarkers[i], c1->GetUymax());
		vlines[i]->SetLineWidth(1);
		vlines[i]->SetLineColor(kBlack);
		vlines[i]->Draw();
		vlines[i]->SetNDC(false);
	}
	std::vector<TLine*> hlines(ymarkers.size());
	for(size_t i=0; i<ymarkers.size(); i++){
		hlines[i] = new TLine(c1->GetUxmin(), ymarkers[i], c1->GetUxmax(), ymarkers[i]);
		hlines[i]->SetLineWidth(1);
		hlines[i]->SetLineColor(kBlack);
		hlines[i]->Draw();
		hlines[i]->SetNDC(false);
	}

	auto legend = new TLegend(0.11,0.7,0.4,0.89);//,0.48,0.9);
	legend->AddEntry(gr[0],ch0.getChannelName().c_str(),"l");
	legend->AddEntry(gr[1],ch1.getChannelName().c_str(),"l");
	legend->AddEntry(gr[2],(ch1.getChannelName() + " - " + ch0.getChannelName()).c_str(),"l");
	legend->SetBorderSize(1);
	legend->SetFillColor(0);
	legend->SetTextSize(0);
	legend->Draw();
//	c1->BuildLegend();

//	c1->Update(); // draws the frame, after which one can change it
//	c1->Modified();

	c1->Print(sfile.c_str());

//	TImage *img = TImage::Create();
//	img->FromPad(c1);
//	img->WriteImage(sfile.c_str());

//	delete img;
	delete legend;
	for(auto line:vlines)
		delete line;
	for(auto line:hlines)
		delete line;
	delete gr[0];
	delete gr[1];
	delete gr[2];
	delete mg;
	delete c1;


}

Double_t evalPhaseDifference(XBOX::XboxDAQChannel &ch0, XBOX::XboxDAQChannel &ch1,
		const Double_t time,
		Double_t t0=-1, Double_t t1=-1)
{
	const Int_t sg_m = 3; // polynomial order
	const Int_t sg_nl = 5;
	const Int_t sg_nr = 5;

	// rescale phase data from [-1, 1] to [-PI, PI]
	std::vector<Double_t> coeffs = {0., M_PI};
	ch0.setScaleType(1); // polynomial scale
	ch0.setScaleCoeffs(coeffs);
	ch1.setScaleType(1); // polynomial scale
	ch1.setScaleCoeffs(coeffs);

	// read data from channel
	std::vector<Double_t> x0 = ch0.getTimeAxis(t0, t1);
	std::vector<Double_t> x1 = ch1.getTimeAxis(t0, t1);

	std::vector<Double_t> y0 = unwrap_phase(ch0.getSignal(t0, t1));
	std::vector<Double_t> y1 = unwrap_phase(ch1.getSignal(t0, t1));

	// get filtered signals
	std::vector<Double_t> kernel = savgol(sg_m, sg_nl, sg_nr, 0); // filter coefficients
	std::reverse(kernel.begin(),kernel.end()); // reverse order of coefficients
	std::vector<Double_t> y0f = conv(y0, kernel, 1*sg_nr, 1*sg_nl);
	std::vector<Double_t> y1f = conv(y1, kernel, 1*sg_nr, 1*sg_nl);

	// get the phase information at specified time
	Double_t y0_time = interp1d(x0, y0f, time);
	Double_t y1_time = interp1d(x1, y1f, time);

	return y0_time - y1_time;
}

Double_t evalDeviationPoint(XBOX::XboxDAQChannel &ch0, XBOX::XboxDAQChannel &ch1,
		const Double_t delay, const Double_t threshold=0.05, const Double_t noiselevel=0.01,
		Double_t t0=-1, Double_t t1=-1)
{
	const Int_t sg_m = 3; // polynomial order
	const Int_t sg_nl = 5;
	const Int_t sg_nr = 5;
	const Int_t nsamples = 10001;

	// read data from channel
	std::vector<Double_t> x0 = ch0.getTimeAxis(t0, t1);
	std::vector<Double_t> x1 = ch1.getTimeAxis(t0, t1);
	std::vector<Double_t> y0 = ch0.getSignal(t0, t1);
	std::vector<Double_t> y1 = ch1.getSignal(t0, t1);

	// get filtered signals
	std::vector<Double_t> kernel = savgol(sg_m, sg_nl, sg_nr, 0); // filter coefficients
	std::reverse(kernel.begin(),kernel.end()); // reverse order of coefficients
	std::vector<Double_t> y0f = conv(y0, kernel, 1*sg_nr, 1*sg_nl);
	std::vector<Double_t> y1f = conv(y1, kernel, 1*sg_nr, 1*sg_nl);

	// scale x-axes of the raw signal data with respect to the first signal
//	Double_t normx = x0.back();
//	for(auto it=x0.begin(); it!=x0.end(); ++it)
//		arange(0., ch0.getIncrement()*(n0-1)/, 1.);

	//	define x-axes of the re-sampled signals with the 2nd signal aligned according to the delay
	std::vector<Double_t> x0_intp = linspace(x0.front(), x0.back(), nsamples);
	std::vector<Double_t> x1_intp = linspace(x0.front()+delay, x0.back()+delay , nsamples);

	// re-sample signals by interpolation
	std::vector<Double_t> y0f_intp = interp1d(x0, y0f, x0_intp);
	std::vector<Double_t> y1f_intp = interp1d(x1, y1f, x1_intp);

	// get difference between both signals
	std::vector<Double_t> diff(y0f_intp.size());
	for(UInt_t i=0; i<diff.size(); i++)
		diff[i] = fabs(y0f_intp[i] - y1f_intp[i]);

	Int_t sg_nl_diff = sg_nl*nsamples/ch0.getSamples();
	Int_t sg_nr_diff = sg_nr*nsamples/ch0.getSamples();

	kernel = savgol(sg_m, sg_nl_diff, sg_nr_diff, 0); // filter coefficients
	std::reverse(kernel.begin(),kernel.end()); // reverse order of coefficients
	std::vector<Double_t> difff = conv(diff, kernel, sg_nr_diff, sg_nl_diff);

	Double_t norm = *std::max_element(difff.begin(), difff.end());
	Double_t th_level = threshold * norm; // absolute threshold
	Double_t th_noise = noiselevel * norm; // absolute threshold

	Int_t idx = 0;

	for(size_t i=0; i<difff.size(); i++){
		if(difff[i] > th_level){
			idx = i;
			break;
		}
	}

	for(Int_t i=idx; i>=0; i--){
		if(difff[i] < th_noise){
			idx = i;
			break;
		}
	}
	Double_t devpoint = idx * (x0_intp[1] - x0_intp[0]) + x0_intp[0];

	return devpoint;
}

Int_t evalDeviationPoint(Double_t &edge_dev, Double_t &edge_prev,  XBOX::XboxDAQChannel &ch0, XBOX::XboxDAQChannel &ch1,
		const Double_t delay, const Double_t threshold=0.05, const Double_t derivative_threshold=0.05,
		Double_t t0=-1, Double_t t1=-1)
{
	const Int_t sg_m = 3; // polynomial order
	const Int_t sg_nl = 5;
	const Int_t sg_nr = 5;
	const Int_t nsamples = 10001;

	Double_t th_level;
	size_t th_index;

	// read data from channel
	std::vector<Double_t> x0 = ch0.getTimeAxis(t0, t1);
	std::vector<Double_t> x1 = ch1.getTimeAxis(t0, t1);
	std::vector<Double_t> y0 = ch0.getSignal(t0, t1); // breakdown event
	std::vector<Double_t> y1 = ch1.getSignal(t0, t1); // previous event

	// get filtered signals
	std::vector<Double_t> kernel = savgol(sg_m, sg_nl, sg_nr, 0); // filter coefficients
	std::vector<Double_t> kernel_derivative = savgol(sg_m, sg_nl, sg_nr, 1); // filter coefficients for derivative
	std::reverse(kernel.begin(),kernel.end()); // reverse order of coefficients
	std::vector<Double_t> y0_filt = conv(y0, kernel, 1*sg_nr, 1*sg_nl);
	std::vector<Double_t> y1_filt = conv(y1, kernel, 1*sg_nr, 1*sg_nl);
	std::vector<Double_t> y1p_filt = conv(y1, kernel_derivative, 1*sg_nr, 1*sg_nl);

	// scale x-axes of the raw signal data with respect to the first signal
//	Double_t normx = x0.back();
//	for(auto it=x0.begin(); it!=x0.end(); ++it)
//		arange(0., ch0.getIncrement()*(n0-1)/, 1.);

	//	define x-axes of the re-sampled signals with the 2nd signal aligned according to the delay
	std::vector<Double_t> x0_intp = linspace(x0.front(), x0.back(), nsamples);
	std::vector<Double_t> x1_intp = linspace(x0.front()+delay, x0.back()+delay , nsamples);

	// re-sample signals by interpolation
	std::vector<Double_t> y0_intp = interp1d(x0, y0_filt, x0_intp);
	std::vector<Double_t> y1_intp = interp1d(x1, y1_filt, x1_intp);
	std::vector<Double_t> y1p_intp = interp1d(x1, y1p_filt, x1_intp);

	// find moment just before rising edge of L1 signal. Start of rising edge
	// defined as sample at which derivative of signal first crosses a threshold.
	// Threshold is a defined as a fraction of the signal peak.
	th_level = derivative_threshold * (*std::max_element(y1p_intp.begin(), y1p_intp.end()));
	th_index = 0;
	for(UInt_t i=0; i<y1p_intp.size(); i++){
		if(y1p_intp[i] > th_level){
			th_index = i;
			break;
		}
	}

	// use above edge information to offset signal such that the start of compressed
	// pulse is at zero, disregarding the ramp just before the compressed pulse.
	std::vector<Double_t> y1o_intp(y1p_intp.size());
	for(UInt_t i=0; i<y1o_intp.size(); i++)
		y1o_intp[i] = y1_intp[i] - y1_intp[th_index];

	// get difference between breakdown and L1 signal
	std::vector<Double_t> diff_intp(y0_intp.size());
	for(UInt_t i=0; i<diff_intp.size(); i++)
		diff_intp[i] = fabs(y0_intp[i] - y1_intp[i]);

//	Int_t sg_nl_diff = sg_nl*nsamples/ch0.getSamples();
//	Int_t sg_nr_diff = sg_nr*nsamples/ch0.getSamples();
//
//	kernel = savgol(sg_m, sg_nl_diff, sg_nr_diff, 0); // filter coefficients
//	std::reverse(kernel.begin(),kernel.end()); // reverse order of coefficients
//	std::vector<Double_t> diff_intp_filt = conv(diff_intp, kernel, sg_nr_diff, sg_nl_diff);

	// Find the time at which the offset L1 signal crosses a threshold.
	// Threshold is defined as a fraction of the signal peak.
	th_level = threshold * (*std::max_element(y1o_intp.begin(), y1o_intp.end()));
	th_index = 0;
	for(UInt_t i=0; i<y1o_intp.size(); i++){
		if(y1o_intp[i] > th_level){
			th_index = i;
			break;
		}
	}
	if(th_index <= 0)
		edge_prev = x1_intp.front();
	else if(th_index >= x1_intp.size()-1)
		edge_prev = x1_intp.back();
	else{
		edge_prev = (th_level - y1o_intp[th_index])
				/ (y1o_intp[th_index] - y1o_intp[th_index-1])
				* (x1_intp[1] - x0_intp[0]) + x1_intp[th_index];
	}

	// Find the time at which the  difference signal crosses a threshold.
	// Threshold is defined as a fraction of the signal peak.
	th_level = threshold * (*std::max_element(diff_intp.begin(), diff_intp.end()));
	th_index = 0;
	for(UInt_t i=0; i<diff_intp.size(); i++){
		if(diff_intp[i] > th_level){
			th_index = i;
			break;
		}
	}
	if(th_index <= 0)
		edge_dev = x1_intp.front();
	else if(th_index >= x1_intp.size()-1)
		edge_dev = x1_intp.back();
	else{
		edge_dev = (th_level - diff_intp[th_index])
				/ (diff_intp[th_index] - diff_intp[th_index-1])
				* (x1_intp[1] - x0_intp[0]) + x1_intp[th_index];
	}

	return 0.;
}


Double_t evalDelayOnThreshold(XBOX::XboxDAQChannel &ch0, XBOX::XboxDAQChannel &ch1,
		const Double_t threshold, Double_t t0=-1, Double_t t1=-1)
{
	const Int_t sg_m = 3; // polynomial order
	const Int_t sg_nl = 5;
	const Int_t sg_nr = 5;
	const Int_t nsamples = 10001;

	// read data from channel
	std::vector<Double_t> x0 = ch0.getTimeAxis(t0, t1);
	std::vector<Double_t> x1 = ch1.getTimeAxis(t0, t1);
	std::vector<Double_t> y0 = ch0.getSignal(t0, t1);
	std::vector<Double_t> y1 = ch1.getSignal(t0, t1);

	// get filtered signals
	std::vector<Double_t> kernel = savgol(sg_m, sg_nl, sg_nr, 0); // filter coefficients
	std::reverse(kernel.begin(),kernel.end()); // reverse order of coefficients
	std::vector<Double_t> y0f = conv(y0, kernel, 1*sg_nr, 1*sg_nl);
	std::vector<Double_t> y1f = conv(y1, kernel, 1*sg_nr, 1*sg_nl);

	// scale x-axes of the raw signal data with respect to the first signal
//	Double_t normx = x0.back();
//	for(auto it=x0.begin(); it!=x0.end(); ++it)
//		arange(0., ch0.getIncrement()*(n0-1)/, 1.);

	//	define x-axes of the re-sampled signals
	std::vector<Double_t> x0_intp = linspace(x0.front(), x0.back(), nsamples);
//	std::vector<Double_t> x1_intp = linspace(x1.front(), x1.back(), nsamples);

	// re-sample signals by interpolation
	std::vector<Double_t> y0f_intp = interp1d(x0, y0f, x0_intp);
	std::vector<Double_t> y1f_intp = interp1d(x1, y1f, x0_intp);

	// determine the time delay between the signals based on exceeding threshold
	Double_t norm = *std::max_element(y0f_intp.begin(), y0f_intp.end()); // take signal 1 maximum as norm reference
	Double_t th_abs = threshold * norm; // absolute threshold
	Double_t idx0 = 0.;
	for(UInt_t i=1; i<y0f_intp.size(); i++){
		if(y0f_intp[i] > th_abs){
			idx0 = Double_t(i - (y0f_intp[i] - th_abs) / (y0f_intp[i] - y0f_intp[i-1]));
			break;
		}
	}
	Double_t idx1 = 0.;
	for(UInt_t i=1; i<y1f_intp.size(); i++){
		if(y1f_intp[i] > th_abs){
			idx1 = Double_t(i - (y1f_intp[i] - th_abs) / (y1f_intp[i] - y1f_intp[i-1]));
			break;
		}
	}
	Double_t delay = (idx1 - idx0) * (x0_intp[1] - x0_intp[0]);

	return delay;

}

//Double_t evalDelayOnEdge(const std::vector<Double_t> &y1, const std::vector<Double_t> &y2, const std::string & sfile)
//{
//	const Int_t sg_m = 3; // polynomial order
//	const Int_t sg_nl = 20;
//	const Int_t sg_nr = 20;
//
//	// get first derivative of both signals
//	std::vector<Double_t> kernel = savgol(sg_m, sg_nl, sg_nr, 1); // first derivative coefficients
//	std::reverse(kernel.begin(),kernel.end()); // reverse order of coefficients
//	std::vector<Double_t> dy1 = conv(y1, kernel, sg_nr, sg_nl);
//	std::vector<Double_t> dy2 = conv(y2, kernel, sg_nr, sg_nl);
//
//	// normalize the derivative in logarithmic scale to pronounce slope
//	Double_t norm=1.;
////	norm = *std::max_element(dy1.begin(), dy1.end());
//	for(auto it=dy1.begin(); it!=dy1.end(); ++it)
//		(*it) = (*it) / norm;
////	norm = *std::max_element(dy2.begin(), dy2.end());
//	for(auto it=dy2.begin(); it!=dy2.end(); ++it)
//		(*it) = (*it) / norm;
//
////	std::vector<Double_t> x_intp = linspace(x.front(), x.back(), 10001);
////	std::cout <<  x.back() << "\t" << x_intp.back() << std::endl;
////	std::vector<Double_t> y_intp = interp1d(x, ybf, x_intp, ROOT::Math::Interpolation::kCSPLINE);
//
//	std::vector<Double_t> x1 = arange(0., Double_t(dy1.size()), 1.);
//	std::vector<Double_t> x2 = arange(0., Double_t(dy2.size()), 1.);
//
//	TCanvas *c1 = new TCanvas("c1","A Simple Graph Example",200,10,700,500);
//	// c1->SetFillColor(42);
//	c1->SetGrid();
//
//	TMultiGraph *mg = new TMultiGraph();
//	TGraph *gr[2];
//
//	gr[0] = new TGraph(x1.size(), &x1[0], &y1[0]);
//	gr[1] = new TGraph(x2.size(), &x2[0], &y2[0]);
//
//	gr[0]->SetLineColor(kRed);
//	gr[0]->SetLineWidth(1);
//	gr[0]->SetMarkerColor(kRed);
////	gr[0]->SetMarkerStyle(20);
////	gr[0]->SetMarkerSize(0.25);
//
//	gr[1]->SetLineColor(kBlue);
//	gr[1]->SetLineWidth(1);
//	gr[1]->SetMarkerColor(kBlue);
////	gr[1]->SetMarkerStyle(20);
////	gr[1]->SetMarkerSize(1);
//
//	mg->Add(gr[0]);
//	mg->Add(gr[1]);
//
//	mg->SetTitle("XY PLOT");
//	mg->GetXaxis()->SetTitle("Time [us]");
//	mg->GetYaxis()->SetTitle("Amplitude");
//	mg->Draw("ACP");
//
////	mg->GetXaxis()->SetRangeUser(1.2e-6,1.4e-6);
////	gr1->GetYaxis()->SetRangeUser(9,20);
//
//	// TCanvas::Update() draws the frame, after which one can change it
//	c1->Update();
//	// c1->GetFrame()->SetFillColor(21);
//	// c1->GetFrame()->SetBorderSize(12);
//	c1->Modified();
//
//	TImage *img = TImage::Create();
//	img->FromPad(c1);
//
//	img->WriteImage(sfile.c_str());
//
////	delete img;
////	delete gr[0];
////	delete gr[1];
////	delete mg;
////	delete c1;
//
//
//	Double_t delay=0;
//
//	return delay;
//
//}

Int_t analyse(std::string fileName, TH2D* htd)
{
	clock_t begin = clock();

	std::map<std::string, XBOX::XboxDAQChannel*> buffer;
	std::map<std::string, std::vector<XBOX::XboxDAQChannel>> channel;

	TFile *file = new TFile(fileName.c_str());
	TTree *tree = (TTree*)file->Get("ChannelSet");

	// create a dictionary for channel and buffer to load data from root file
	std::vector<std::string> keys;
	auto listofbranches = tree->GetListOfBranches();
	for(Int_t i=0; i<listofbranches->GetEntries(); ++i){
		std::string key = listofbranches->At(i)->GetName();
		keys.push_back(key);
		buffer.insert(std::pair<std::string, XBOX::XboxDAQChannel*>
				(key, new XBOX::XboxDAQChannel()));
		channel.insert(std::pair<std::string, std::vector<XBOX::XboxDAQChannel>>
				(key, std::vector<XBOX::XboxDAQChannel>()));
		tree->SetBranchAddress(key.c_str(), &buffer[key]);
		std::cout << listofbranches->At(i)->GetName() << std::endl;
	}

	// read data into channel dictionary
	ULong64_t nevent = tree->GetEntries();
	printf("Number of entries: %llu\n", nevent);

	for(ULong64_t i=0;i<nevent; i++){
		tree->GetEntry(i);
		for(std::string key: keys)
			channel[key].push_back(*buffer[key]);
	}

	// release memory
	for(std::string key: keys)
		delete buffer[key];
	delete tree;
	delete file;

	// Index array for breakdown events
	// The events before must be of log type 1 and 2 while a threshold defines
	// the minimum temporal distance between subsequent breakdowns.
	// Note the logtype order is really ...-1 -1 1 2 0 -1 -1....
	std::vector<Double_t> index_bd;
	Double_t thres_tdiff = 1.; // threshold
	TTimeStamp tsprev(0);
	for(UInt_t i=2;i<nevent; i++){
		// if event is break down
		if(channel[keys[0]][i].getLogType() == 0
				&& channel[keys[0]][i-1].getLogType() == 2
				&& channel[keys[0]][i-2].getLogType() == 1 ){
			TTimeStamp ts = channel[keys[0]][i].getTimeStamp();
			if(ts - tsprev > thres_tdiff){ 	// check temporal distance between subsequent breakdowns
				tsprev = ts;
//				printf("Breakdown at position %d: %40s\n", i, ts.AsString());
//				printf("%d: %40s LogTypes: %d %d %d\n", i, ts.AsString(),
//						channel[keys[0]][i].getLogType(),
//						channel[keys[0]][i-1].getLogType(),
//						channel[keys[0]][i-2].getLogType());
//				printf("Flags: %d %d %d %d\n",
//						channel["DC_UP"][i].getBreakdownFlag(),
//						channel["DC_DOWN"][i].getBreakdownFlag(),
//						channel["PSR_amp"][i].getBreakdownFlag(),
//						channel["PERA_amp"][i].getBreakdownFlag());

				if( (channel["DC_UP"][i].getBreakdownFlag()
						| channel["DC_DOWN"][i].getBreakdownFlag()
						| channel["PSR_amp"][i].getBreakdownFlag())
						& ~channel["PERA_amp"][i].getBreakdownFlag()){
					index_bd.push_back(i);
					printf("Breakdown at position %d: %40s\n", i, ts.AsString());
				}
			}
			else {
				printf("WARNING: Skip breakdown at %d: %40s. Time differences "
						"to previous breakdown < %f sec.\n",  i,
						ts.AsString(), thres_tdiff);
			}
		}
	}

//	for(UInt_t j=0; j<1000; j++)
//		for(UInt_t i=0; i<nevent; i++)
//			channel["PSI_amp"][i].getSignalRisingEdge();

//	printf("\n");
//	for(Int_t i: index_bd)
//		printf("%d: %s\n", i, channel[keys[0]][i].getTimeStamp().AsString());
//	Double_t tr = channel["PSI_amp"][0].getSignalRisingEdge();
//	Double_t tf = channel["PSI_amp"][0].getSignalFallingEdge();
//
//	printf("Rising edge at: %e sec.\n", tr);
//	printf("Falling edge at: %e sec.\n", tf);
//	printf("Pulse width: %e\n", tf - tr);
//	printf("Pulse height: %e\n", channel["PSI_amp"][0].getSignalMean(tr, tf));
//	printf("Signal min: %e\n", channel["PSI_amp"][0].getSignalMin());
//	printf("Signal max: %e\n", channel["PSI_amp"][0].getSignalMax());
//	printf("Signal sum: %e\n", channel["PSI_amp"][0].getSignalSum());
//	printf("Signal int: %e\n", channel["PSI_amp"][0].getSignalInt(1e-6, 1.2e-6));
////	printf("Signal med: %e\n", channel["PSI_amp"][0].getSignalMedian(1e-6, 1.2e-6));
////	printf("Signal med: %e\n", channel["PSI_amp"][0].getSignalMedian());


//	for(std::string key: keys)
//		channel[key].push_back(*buffer[key]);

	FS::path fsfile_in(fileName);
	FS::path fsdir = fsfile_in.parent_path();
	FS::path fsfile_out;

//	std::vector<Double_t> x = channel["PSI_amp"][0].getTimeAxis();
//	std::vector<Double_t> y = channel["PSI_amp"][0].getSignal();
//
//	fsfile_out = fsdir / (fsfile_in.stem().string() + "_PSI_amp.png");
//	plot(fsfile_out.c_str(), x, y);
//
//	Int_t ib = index_bd[0]; // get first breakdown
//	std::vector<Double_t> yb = channel["PSI_amp"][ib].getSignal();
//	std::vector<Double_t> xblm = channel["BLM_TIA"][ib].getTimeAxis();
//	std::vector<Double_t> yblm = channel["BLM_TIA"][ib].getSignal();
//
//	fsfile_out = fsdir / (fsfile_in.stem().string() + "_PSI_amp_BD.png");
//	plot(fsfile_out.c_str(), x, yb);
//	fsfile_out = fsdir / (fsfile_in.stem().string() + "_BLM_TIA.png");
//	plot(fsfile_out.c_str(), xblm, yblm);

	std::string ftype = "pdf";

	for(size_t j=0; j<index_bd.size(); j++)
//	for(size_t j=0; j<1; j++)
	{
		Int_t i = index_bd[j];
////		Int_t i = 163;
//		std::vector<Double_t> xmarkers;

    	// align the INC and the TRA signals (time prop. to the structure's filling time)
		Double_t delay_inc_tra = evalDelayOnThreshold(channel["PSI_amp"][i],
				channel["PEI_amp"][i], 0.1, 0.0e-6, 0.5e-6);

		delay_inc_tra = 60e-9;

		// find the time jitter between BD and L1 event based on threshold crossing
		Double_t jitter = evalDelayOnThreshold(channel["PSI_amp"][i],
				channel["PSI_amp"][i-2], 0.4, 0.8e-6, 1.9e-6);


		// falling edge of the transmitted signal by subtracting the TRA_BD and TRA_L1 signals
		Double_t time_tra = evalDeviationPoint(channel["PEI_amp"][i], channel["PEI_amp"][i-2],
				jitter, 0.5, 0.01,	-1, -1);

		// do the same for the rising edge of the reflected signal
		Double_t time_ref = evalDeviationPoint(channel["PSR_amp"][i], channel["PSR_amp"][i-2],
				jitter, 0.5, 0.01,	-1, -1);

        // Find the time difference between the two ie get the position of the BD
        Double_t td = time_ref - time_tra + delay_inc_tra;

        // rising edge of the dark current signal
        Double_t time_dc_up = evalDeviationPoint(channel["DC_UP"][i], channel["DC_UP"][i-2],
        				0, 0.5, 0.01);

        // phase difference between incident and reflected signal at time_ref
//        Double_t delta_phase = evalPhaseDifference(channel["PSI_ph"][i], channel["PSR_ph"][i-2],
//        		time_ref);

		// skip data set if a deviation point is missing
		if(time_tra <= 0. || time_ref<=0. || time_dc_up<=0.){
			TTimeStamp ts = channel["PSI_amp"][i].getTimeStamp();
			printf("INFO: Skip Breakdown because of missing deviation point at %d: %40s\n", i, ts.AsString());
			continue;
		}
		else{
			htd->Fill(td*1e9/2, channel["PSI_amp"][i].getPulseCount());
//			printf("INFO: %d | Count = %llu | td = %e, \n", i, channel["PSI_amp"][i].getPulseCount(), td);
		}


//		// plot signals
//		std::vector<Double_t> xmarkers;
//		fsfile_out = fsdir / (fsfile_in.stem().string() + "_delay." + ftype);
//		plot(fsfile_out.c_str(), channel["PSI_amp"][i],	channel["PEI_amp"][i], delay_inc_tra, -1, -1, false, xmarkers);
//
//		fsfile_out = fsdir / (fsfile_in.stem().string() + "_Jitter." + ftype);
//		plot(fsfile_out.c_str(), channel["PSI_amp"][i],	channel["PSI_amp"][i-2], jitter, -1, -1, false, xmarkers);
//
//		fsfile_out = fsdir / (fsfile_in.stem().string() + "_DC_DOWN." + ftype);
//				plot(fsfile_out.c_str(), channel["DC_DOWN"][i],	channel["DC_DOWN"][i-2], 0, -1, -1, false);
//
//		xmarkers.clear();
//		xmarkers.push_back(time_tra);
//		fsfile_out = fsdir / (fsfile_in.stem().string() + "_PEI." + ftype);
//		plot(fsfile_out.c_str(), channel["PEI_amp"][i],	channel["PEI_amp"][i-2], jitter, -1, -1, false, xmarkers);
//
//		xmarkers.clear();
//		xmarkers.push_back(time_ref);
//		fsfile_out = fsdir / (fsfile_in.stem().string() + "_PSR." + ftype);
//		plot(fsfile_out.c_str(), channel["PSR_amp"][i],	channel["PSR_amp"][i-2], jitter, -1, -1, false, xmarkers);
//
//        xmarkers.clear();
//		xmarkers.push_back(time_ref);
//		fsfile_out = fsdir / (fsfile_in.stem().string() + "_DPhase.eps");
//		plot(fsfile_out.c_str(), channel["PSI_ph"][i],	channel["PSR_ph"][i-2], 0, -1, -1, true, xmarkers);
//
//		// print signal information
//		printf("delay_inc_tra: %e\n", delay_inc_tra);
//		printf("jitter:        %e\n", jitter);
//		printf("time_tra:      %e\n", time_tra);
//		printf("time_ref:      %e\n", time_ref);
//		printf("td:            %e\n", td);
//		printf("time_dc_up:    %e\n", time_dc_up);
//		printf("delta_phase:   %e\n", delta_phase);
//		printf("PulseCount:   %llu\n", channel["PSI_amp"][i].getPulseCount());


//		std::cin.get();
	}


//	Int_t i = index_bd[0];
//	std::vector<Double_t> coeffs = {0., M_PI};
//	channel["PSI_ph"][i].setScaleType(1); // polynomial scale
//	channel["PSI_ph"][i].setScaleCoeffs(coeffs);
//	channel["PSI_ph"][i].print();
//
//	fsfile_out = fsdir / (fsfile_in.stem().string() + "_PSI_ph.eps");
////	plot(fsfile_out.c_str(), channel["PSI_ph"][i], -1, -1);
////	plot(fsfile_out.c_str(), channel["PSI_ph"][i], -1, -1, true);
//	plot(fsfile_out.c_str(), channel["PSI_ph"][i],  channel["PSI_ph"][i], 0, -1, -1, true);


	clock_t end = clock();
	printf("Elapsed time: %.3f\n", double(end - begin) / CLOCKS_PER_SEC);
	return 0;



}




Int_t analyse2(const std::string &fileName, TH2D &htd)
{
	clock_t begin = clock();

	std::map<std::string, TTreeReaderValue<XBOX::XboxDAQChannel>*> event0; // for current/ BD event
	std::map<std::string, TTreeReaderValue<XBOX::XboxDAQChannel>*> event1; // for previous/ L1 event

	TFile *file0 = new TFile(fileName.c_str()); // for current/ BD event
	TFile *file1 = new TFile(fileName.c_str()); // for previous/ L1 event
	TTree *tree = (TTree*)file0->Get("ChannelSet");

	TTreeReader reader0("ChannelSet", file0); // for pointer on current/ BD events
	TTreeReader reader1("ChannelSet", file1); // for pointer on previous/ L1 events

	// create a dictionary for channel and buffer to load data from root file
	std::vector<std::string> keys;
	auto listofbranches = tree->GetListOfBranches();

//	std::cout << keys[0] << end

	for(Int_t i=0; i<listofbranches->GetEntries(); ++i){
		std::string key = listofbranches->At(i)->GetName();
		keys.push_back(key);
//		event0.insert(std::pair<std::string, TTreeReaderValue<XBOX::TDAQChannel>>
//				(key, TTreeReaderValue<XBOX::XboxDAQChannel>()));
		event0[key] = new TTreeReaderValue<XBOX::XboxDAQChannel>(reader0, key.c_str());
		event1[key] = new TTreeReaderValue<XBOX::XboxDAQChannel>(reader1, key.c_str());
		std::cout << listofbranches->At(i)->GetName() << std::endl;
	}


	// read data into channel dictionary
	ULong64_t nevent = tree->GetEntries();
	printf("Number of entries: %llu\n", nevent);

	// Index array for breakdown events
	// The events before must be of log type 1 and 2 while a threshold defines
	// the minimum temporal distance between subsequent breakdowns.
	// Note the logtype order is really ...-1 -1 1 2 0 -1 -1....
	std::vector<Double_t> index_bd;
	Double_t thres_tdiff = 1.; // threshold
	TTimeStamp tsprev(0);

	reader0.Next();
	reader0.Next();
	Int_t c = 0;
	for(ULong64_t i=2;i<nevent; i++){

		reader0.Next();
		reader1.Next();

		for(std::string k: keys){
			(*event0[k])->flushbuffer();
			(*event1[k])->flushbuffer();
		}

		// check log types for breakdown event
		if(!((*event0[keys[0]])->getLogType() == 0
				&& (*event1[keys[0]])->getLogType() == 1 ))
			continue;

		// check whether previous breakdown satisfies a minimum temporal distance
		TTimeStamp ts = (*event0[keys[0]])->getTimeStamp();
		if(ts - tsprev < thres_tdiff){ // check temporal distance between subsequent breakdowns
			printf("WARNING: Skip breakdown at %llu: %40s. Time differences "
								"to previous breakdown < %f sec.\n",  i,
								ts.AsString(), thres_tdiff);
			continue;
		}

		tsprev = ts;
//		printf("Breakdown at position %d: %40s\n", i, ts.AsString());
//		printf("%d: %40s LogTypes: %d %d %d\n", i, ts.AsString(),
//				channel[keys[0]][i].getLogType(),
//				channel[keys[0]][i-1].getLogType(),
//				channel[keys[0]][i-2].getLogType());
//		printf("Flags: %d %d %d %d\n",
//				channel["DC_UP"][i].getBreakdownFlag(),
//				channel["DC_DOWN"][i].getBreakdownFlag(),
//				channel["PSR_amp"][i].getBreakdownFlag(),
//				channel["PERA_amp"][i].getBreakdownFlag());

		// check breakdown flags
		if(!(((*event0["DC_UP"])->getBreakdownFlag()
				| (*event0["DC_DOWN"])->getBreakdownFlag()
				| (*event0["PSR_amp"])->getBreakdownFlag())
				& ~(*event0["PERA_amp"])->getBreakdownFlag()))
			continue;

		// reject glitched pulses which have inconsistent number of samples.
		if((*event0["PSI_amp"])->getSamples() != (*event1["PSI_amp"])->getSamples()
				|| (*event0["PSI_amp"])->getSamples() != (*event1["PSI_amp"])->getSamples()
				|| (*event0["PSI_amp"])->getSamples() != (*event1["PSI_amp"])->getSamples())
		continue;

		index_bd.push_back(i);

		FS::path fsfile_in(fileName);
		FS::path fsdir = fsfile_in.parent_path();
		FS::path fsfile_out;

		std::string ftype = "pdf";

    	// use PSI signal to align previous pulse in time to the breakdown pulse.
//		Double_t delay_inc_tra = evalDelayOnThreshold(**event0["PSI_amp"],
//				**event0["PEI_amp"], 0.4, 0.0e-6, 0.5e-6);
		Double_t delay_inc_tra = 1e-6;
//		Double_t delay_inc_tra = 50e-9;

		// find the time jitter between BD and L1 event based on threshold crossing
		Double_t jitter = evalDelayOnThreshold(**event0["PSI_amp"], **event1["PSI_amp"],
				0.4, 0.8e-6, 1.9e-6);

		Double_t time_tra_prev;
		Double_t time_tra;
		Double_t time_ref_prev;
		Double_t time_ref;
		Double_t td;

		// Find edges of signals for breakdown position analysis.
		// Note the use of a different threshold for PSI vs PER and PSR.
		evalDeviationPoint(time_tra, time_tra_prev, **event0["PEI_amp"], **event1["PEI_amp"],
								jitter, 0.1, 0.5,	0.8e-6, 1.6e-6);
		evalDeviationPoint(time_ref, time_ref_prev, **event0["PSR_amp"], **event1["PSR_amp"],
								jitter, 0.1, 0.5,	-1, -1);
		td = 0.5 * (time_ref - time_ref_prev) - (time_tra - time_tra_prev);

//		// falling edge of the transmitted signal by subtracting the TRA_BD and TRA_L1 signals
//		time_tra = evalDeviationPoint(**event0["PEI_amp"], **event1["PEI_amp"],
//				jitter, 0.5, 0.01,	-1, -1);
//
////		std::cout << "Iter: " << j << std::endl;
//
//		// do the same for the rising edge of the reflected signal
//		time_ref = evalDeviationPoint(**event0["PSR_amp"], **event1["PSR_amp"],
//				jitter, 0.5, 0.01,	-1, -1);
//
//        // Find the time difference between the two ie get the position of the BD
//        td = time_ref - time_tra + delay_inc_tra;

        // check for dc down threshold
        std::vector<Double_t> dc_down = (*event0["DC_DOWN"])->getSignal();
        Double_t min_dc_down = *std::min_element(dc_down.begin(), dc_down.end());


//        // phase difference between incident and reflected signal at time_ref
//        Double_t delta_phase = evalPhaseDifference(**event0["PSI_ph"], **event1["PSR_ph"],
//        		time_ref);


        if(i > 1e9){
        // plot signals
		std::vector<Double_t> xmarkers;
//		fsfile_out = fsdir / (fsfile_in.stem().string() + "_delay." + ftype);
//		plot(fsfile_out.c_str(), **event0["PSI_amp"], **event1["PEI_amp"], delay_inc_tra, -1, -1, false, xmarkers);

		fsfile_out = fsdir / (fsfile_in.stem().string() + "_Jitter." + ftype);
		plot(fsfile_out.c_str(), **event0["PSI_amp"], **event1["PSI_amp"], jitter, -1, -1, false, xmarkers);
//		plot(fsfile_out.c_str(), **event0["PSI_amp"], **event1["PSI_amp"], jitter, 0.5e-6, 1.5e-6, false, xmarkers);

		fsfile_out = fsdir / (fsfile_in.stem().string() + "_PEI." + ftype);
		plot(fsfile_out.c_str(), **event0["PEI_amp"], **event1["PEI_amp"], 0);

		xmarkers.clear();
		xmarkers.push_back(time_tra);
		xmarkers.push_back(time_tra_prev);
		fsfile_out = fsdir / (fsfile_in.stem().string() + "_PEI." + ftype);
		plot(fsfile_out.c_str(), **event0["PEI_amp"], **event1["PEI_amp"], jitter, -1, -1, false, xmarkers);

		xmarkers.clear();
		xmarkers.push_back(time_ref);
		xmarkers.push_back(time_ref_prev);
		fsfile_out = fsdir / (fsfile_in.stem().string() + "_PSR." + ftype);
		plot(fsfile_out.c_str(), **event0["PSR_amp"], **event1["PSR_amp"], jitter, -1, -1, false, xmarkers);

		fsfile_out = fsdir / (fsfile_in.stem().string() + "_DC_DOWN." + ftype);
		plot(fsfile_out.c_str(), **event0["DC_DOWN"], **event1["DC_DOWN"], 0);

        std::vector<Double_t> x = (*event0["DC_DOWN"])->getTimeAxis();
        plot("DC_DOWN.pdf", x, dc_down);

//		xmarkers.clear();
//		xmarkers.push_back(time_ref);
//		fsfile_out = fsdir / (fsfile_in.stem().string() + "_DPhase.eps");
//		plot(fsfile_out.c_str(), **event0["PSI_ph"], **event1["PSR_ph"], 0, -1, -1, true, xmarkers);

		// print signal information
		printf("delay_inc_tra: %e\n", delay_inc_tra);
		printf("jitter:        %e\n", jitter);
		printf("time_tra:      %e\n", time_tra);
		printf("time_tra_prev: %e\n", time_tra_prev);
		printf("time_ref:      %e\n", time_ref);
		printf("time_ref_prev: %e\n", time_ref_prev);
		printf("td:            %e\n", td);
		printf("min_dc_down:    %e\n", min_dc_down);
//		printf("delta_phase:   %e\n", delta_phase);
		printf("PulseCount:   %llu\n", (*event0["PSI_amp"])->getPulseCount());
//		(*event0["PSI_amp"])->print();
//		break;
		std::cin.get();
        }
		// skip data set if a deviation point is missing
		if(time_tra <= 0. || time_ref<=0. || min_dc_down>-0.005){
			TTimeStamp ts = (*event0["PSI_amp"])->getTimeStamp();
			printf("INFO: Skip Breakdown because of missing deviation point at %llu: %40s\n", i, ts.AsString());
		}
		else{
			htd.Fill(td*1e9/2, (*event0["PSI_amp"])->getPulseCount());
//			printf("INFO: %d | Count = %llu | td = %e, \n", i, channel["PSI_amp"][i].getPulseCount(), td);
//			printf("Breakdown at position %llu: %40s | %e | %e | %e | %e\n", i,
//					ts.AsString(), time_tra, time_tra_prev, time_ref, time_ref_prev);
			printf("Breakdown at position %llu: %40s | %e\n", i,
								ts.AsString(), td);
			c++;
		}


	}

	printf("Counts: %d\n", c);
	delete file0;
	delete file1;

	clock_t end = clock();
	printf("Elapsed time: %.3f\n", double(end - begin) / CLOCKS_PER_SEC);
	return 0;
}



int readtdms(const std::string &fileName){

	clock_t begin = clock();

	XBOX::XboxTdmsReader parser(fileName);
	XBOX::XboxDAQChannel channel;

	// get Xbox version from the Tdms Group
	printf("XBox Version: %d\n", parser.getXboxVersion());
	printf("Number of available channels: %u\n", parser.getChannelCount());

	Int_t i = 0;
	parser.begin();
	while (!parser.isEOF()){
		parser.getChannel("PSI_amp", channel); // read specific channel for each event

		channel.getSignal();
		if(i == 17)
			channel.print();
		parser++;
		i++;
	}

	clock_t end = clock();
	printf("Elapsed time: %.3f\n", double(end - begin) / CLOCKS_PER_SEC);

	return 0;
}

Int_t read(std::string fileName)
{
	clock_t begin = clock();

    TFile *file = new TFile(fileName.c_str());
    TTree *tree = (TTree*)file->Get("ChannelSet");

    // create pointers of TDAQChannel to read the branch objects for each event
    XBOX::XboxDAQChannel *ch = new XBOX::XboxDAQChannel();

    // get branches and set the branch address
//    TBranch *branch = tree->GetBranch("PSI_amp");
//    branch->SetAddress(&ch);
    tree->SetBranchAddress("PSI_amp", &ch);

    // get number of entries
    ULong64_t nEntryCount = tree->GetEntries();

    printf("Number of entries: %llu\n", nEntryCount);
    for(ULong64_t i=0;i<nEntryCount; i++){
    	tree->GetEntry(i);

    	if(ch->getTimeStamp() != ch->getStartTime())
    		printf("Time Difference in group %s\n", ch->getTimeStamp().AsString());

		if (i==17){
			std::vector<Double_t> x;
		    std::vector<Double_t> y;

			std::string sChannelName = ch->getChannelName();
			ch->print();

			x = ch->getTimeAxis();
			y = ch->getSignal();
			printf("Data: %g ... %g\n", y.front(), y.back());

			std::string basename = FS::basename(fileName);
			FS::path fsInfile(fileName);
			FS::path fsDir = fsInfile.parent_path();
			FS::path fsOutfile = fsDir / (fsInfile.stem().string() + ".png");
			plot(fsOutfile.string(), x, y);

		}
    }

    delete tree;
    delete file;

	clock_t end = clock();
	printf("Elapsed time: %.3f\n", double(end - begin) / CLOCKS_PER_SEC);
	return 0;
}

// This is the function invoked during the processing of the trees.
//auto workItem = [](std::string &sfile) {
//	auto partialHisto = new TH2D("htd","Breakdown statistics_MP",100,-20.,70.,100,0e8,5.5e8);
//	analyse2(sfile, partialHisto);
//	return partialHisto;
//};
//auto workItem = [](UInt_t workerID) {
//	Int_t i = workerID + 3;
//	char sfile[100];
//	snprintf (sfile, 100, "../data/xbox2/EventData_2018%02d.root", i);
//	auto partialHisto = new TH2D("htd","Breakdown statistics_MP",100,-20.,70.,100,0e8,5.5e8);
//	analyse2(sfile, partialHisto);
//	return partialHisto;
//};

void plotAnalyserResults(const std::string &sfile, const std::string &filetype="eps")
{
	FS::path fsfile_in(sfile);
	FS::path fsdir = fsfile_in.parent_path();
	FS::path fsfile_out;


	TFile fileAnalyser(sfile.c_str());
	TTree* treeAnalyser = (TTree*)fileAnalyser.Get("Analyser");
	TCanvas *ctd = new TCanvas("c0","Breakdown studies",200,10,700,500);

	// plot breakdown localisation statistics
	treeAnalyser->Draw("Analyser.fPulseCount:(Analyser.fBDTime)*1e9"
			">>htd(200,-5.,70,200,0e8,5.5e8)", "Analyser.fBDFlag", "COLZ");
	TH2D *htd = (TH2D*)ctd->GetPrimitive("htd");

	htd->SetTitle("PSI2 Xbox2 Breakdown statistics; position [ns]; number of pulses" );
//	htd->SetTitle("Breakdown statistics");
//	htd->GetXaxis()->SetTitle("position [ns]");
//	htd->GetYaxis()->SetTitle("number of pulses");

	ctd->Update(); // draws the frame, after which one can change it
	ctd->Modified();
	fsfile_out = fsdir / (fsfile_in.stem().string() + "_BDStatistics." + filetype);
	ctd->Print(fsfile_out.c_str());

	// plot power versus time statistics
	treeAnalyser->Draw("(Analyser.fPulsePowerMean)*1e-6:Analyser.fTimeStamp"
			">>hp", "Analyser.fTimeStamp > 1.5e9 && !fBDFlag", "");
	TH2D *hp = (TH2D*)ctd->GetPrimitive("hp");
	hp->SetMarkerColor(kRed);
	hp->GetXaxis()->SetTimeDisplay(1);
	hp->GetXaxis()->SetTimeFormat("%d.%m.%y%F1970-01-01 00:00:00");
	hp->SetTitle("PSI2 Xbox2 Test; time ; power [MW]" );

	ctd->Update(); // draws the frame, after which one can change it
	ctd->Modified();
	fsfile_out = fsdir / (fsfile_in.stem().string() + "_Power." + filetype);
	ctd->Print(fsfile_out.c_str());

	// plot power versus time statistics
	treeAnalyser->Draw("(Analyser.fPulseLength)*1e9:Analyser.fTimeStamp"
			">>hl", "Analyser.fTimeStamp > 1.5e9 && !fBDFlag && Analyser.fPulseLength > 0 && Analyser.fPulseLength < 400e-9", "");
	TH2D *hl = (TH2D*)ctd->GetPrimitive("hl");
	hl->SetMarkerColor(kRed);
	hl->GetXaxis()->SetTimeDisplay(1);
	hl->GetXaxis()->SetTimeFormat("%d.%m.%y%F1970-01-01 00:00:00");
	hl->SetTitle("PSI2 Xbox2 Test; time ; pulse length [ns]" );

	ctd->Update(); // draws the frame, after which one can change it
	ctd->Modified();
	fsfile_out = fsdir / (fsfile_in.stem().string() + "_PulseLength." + filetype);
	ctd->Print(fsfile_out.c_str());

	delete ctd;
}


int main(int argc, char* argv[]) {

	clock_t begin = clock();
//	TApplication theApp("theApp",&argc,argv); // Popup the GUI... new MyMainFrame(gClient->GetRoot(),200,200); theApp.Run();

	std::string fileName;
//	fileName = "../data/xbox1/Prod_20170307";
//	fileName = "../data/xbox1/Prod_20180507";
//	fileName = "../data/xbox2/EventData_20180608";
	fileName = "../data/xbox2/EventData_20180202";
//	fileName = "/dfs/Workspaces/x/Xbox3_T24PSI_2/EventDataB_20171029";
//	fileName = "../data/xbox3/EventDataA_20180201";
//	fileName = "../data/xbox3/EventDataA_20180617";

//	for(int i=0; i<100; ++i)
//		readtdms(fileName + ".tdms");


//	// single file conversion
//	XBOX::XboxTdmsConverter converter;
//	converter.addFile("/dfs/Workspaces/x/Xbox2_T24PSI_2/EventData_20180328.tdms");
//	converter.write("../data/xbox2/EventData_20180328.root");

//	// multiple conversion to combine data for each month
//	char sfile[100];
//	for(Int_t j=3; j<4; j++)
//	{
//		XBOX::XboxTdmsConverter converter;
//		for(Int_t i=1; i<32; i++){
//			if(j==3 && i==30)
//				continue;
//			else if(j==8 && i==23)
//				continue;
//
//	//		snprintf (sfile, 100, "/dfs/Workspaces/x/Xbox2_T24PSI_1/EventData_201802%02d.tdms", i);
//			snprintf (sfile, 100, "/dfs/Workspaces/x/Xbox2_T24PSI_2/EventData_2018%02d%02d.tdms", j, i);
//			converter.addFile(sfile);
//		}
//		snprintf (sfile, 100, "../data/xbox2/EventData_2018%02d.root", j);
//		converter.write(sfile);
//	}

//	fileName = "../data/xbox2/EventData_201808";
//	converter.write(fileName + ".root");
////	converter.writeH5(fileName + ".h5");
////	std::cout << converter.getXboxVersion() << std::endl;


//	read(fileName + ".root");

	// statistic about the location of break downs
//	TH2D htd("htd","Breakdown statistics",200,-5.,70.,200,0e8,5.5e8);

	Int_t nbreakdowns=0;
	char sfile[100];
	XBOX::XboxAnalyser *ana;
	ana = new XBOX::XboxAnalyser();

	// configure root output file
	TFile fileAnalyser("../data/xbox2/PSI2_Xbox2.root", "RECREATE");
	TTree treeAnalyser("Analyser","Tree of analysed data");
	treeAnalyser.Branch("Analyser", &ana, 16000, 99);
	for(Int_t i=3; i<10; i++){
		snprintf (sfile, 100, "../data/xbox2/EventData_2018%02d.root", i);
//		XBOX::XboxAnalyser ana(sfile, "ChannelSet");
		ana->loadEntryList(sfile, "ChannelSet");
//		ana->updateFile();
		while(ana->nextEntry()){
			treeAnalyser.Fill();
			if(ana->getBDFlag()){
				nbreakdowns++;
//				htd.Fill(ana->getBDTime()*1e9, ana->getPulseCount());

			}
		}
	}
	fileAnalyser.Write();
	fileAnalyser.Close();

	printf("Number of breakdowns: %d\n", nbreakdowns);


	// plot analyser results
	plotAnalyserResults("../data/xbox2/PSI2_Xbox2.root");


//	std::string sfileout = "../data/xbox2/EventData_201804_htd";
////	std::string sfileout = "../data/xbox2/EventData_20180328_htd";
//
//	TCanvas *ctd = new TCanvas("c0","Breakdown studies",200,10,700,500);
//	htd.GetXaxis()->SetTitle("position [ns]");
//	htd.GetYaxis()->SetTitle("number of pulses");
//	htd.Draw("COLZ");
//	ctd->Update(); // draws the frame, after which one can change it
//	ctd->Modified();
//
//	ctd->Print((sfileout + ".eps").c_str());
//	delete ctd;
//
//	TCanvas *ctdx = new TCanvas("c1","Breakdown studies X",200,10,700,500);
//	TH1D *projh2X = htd.ProjectionX();
//	projh2X->SetFillColor(kBlue+1);
//	projh2X->Draw("bar");
//
//	ctdx->Print((sfileout + "x.eps").c_str());
//	delete ctdx;



//	// MP
//	// The number of workers
//	const UInt_t nWorkers = 7U;
//
//	// Create the pool of workers
//	ROOT::TProcessExecutor workers(nWorkers);
//
//	// Fill the pool with work
//	auto hlist = workers.Map(workItem, ROOT::TSeqI(nWorkers));
//
//	for(auto h: hlist)
//		htd->Add(h);
//
//	htd->Draw("COLZ");
//	ctd->Update(); // draws the frame, after which one can change it
//	ctd->Modified();
//
//	std::string sfileout = "../data/xbox2/EventData_201804_htd.eps";
//	ctd->Print(sfileout.c_str());
//
//	delete htd;
//	delete ctd;

	clock_t end = clock();
	printf("Total elapsed time: %.3f\n", double(end - begin) / CLOCKS_PER_SEC);


//	theApp.Run();

	return 0;
}

