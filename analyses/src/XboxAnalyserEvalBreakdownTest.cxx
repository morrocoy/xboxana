#include "XboxAnalyserEvalBreakdownTest.hxx"

#include <iostream>
#include <numeric>

// root
#include "TTimeStamp.h"
#include "TH1D.h"
#include "TMatrixD.h"
#include "TVectorD.h"
#include "TDecompSVD.h"

// root graphics
#include "TStyle.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TLine.h"
#include "TLegend.h"
#include "TPad.h"
#include "TGraph.h"
#include "TColor.h"
#include "TAxis.h"

// xbox
#include "XboxSignalFilter.hxx"
#include "XboxAlgorithms.h"


#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif


////////////////////////////////////////////////////////////////////////
/// Debug function.
/// Scale signal to its magnitude for plotting purpose
std::vector<Double_t> norm(std::vector<Double_t> y) {

	std::vector<Double_t> yn(y.size(), 0.);

	Double_t max = fabs(*std::max_element(y.begin(), y.end()));
	Double_t min = fabs(*std::min_element(y.begin(), y.end()));

	if (max > min)
		for(size_t i=0; i<y.size(); i++)
			yn[i] = y[i] / max;
	else
		for(size_t i=0; i<y.size(); i++)
			yn[i] = y[i] / min;

	return yn;
}

////////////////////////////////////////////////////////////////////////
/// Debug function.
/// Plot signals.
void XboxAnalyserEvalBreakdownTest::plotSignal(const std::string &filepath, std::vector<Double_t> &x, std::vector<Double_t> &y0, std::vector<Double_t> &y1) {


	TCanvas *c1 = new TCanvas("c1","c1",200,10,700,500);
	c1->SetGrid();

	// add graphs
	TMultiGraph *mg = new TMultiGraph();
	TGraph *gr[3];

	gr[0] = new TGraph(x.size(), &x[0], &y0[0]);
	gr[1] = new TGraph(x.size(), &x[0], &y1[0]);
//	gr[2] = new TGraph(x.size(), &x[0], &yd[0]);

	gr[0]->SetLineColor(kRed);
	gr[0]->SetLineWidth(1);
	gr[0]->SetMarkerColor(kRed);
//	gr[0]->SetMarkerStyle(20);
//	gr[0]->SetMarkerSize(1);
	mg->Add(gr[0]);

	gr[1]->SetLineColor(kBlue);
	gr[1]->SetLineWidth(1);
	gr[1]->SetMarkerColor(kBlue);
//	gr[1]->SetMarkerStyle(20);
//	gr[1]->SetMarkerSize(0.1);
	mg->Add(gr[1]);

//	gr[2]->SetLineColor(kGreen+2);
//	gr[2]->SetLineWidth(1);
//	gr[2]->SetMarkerColor(kGreen+2);
////	gr[2]->SetMarkerStyle(20);
////	gr[2]->SetMarkerSize(0.8);
//	mg->Add(gr[2]);

	mg->GetXaxis()->SetTitle("Time [s]");
	mg->GetYaxis()->SetTitle("Amplitude");
	mg->Draw("ACP");

	c1->Update(); // draws the frame, after which one can change it
	c1->Modified();

	// save file
	c1->Print(filepath.c_str());

	delete gr[0];
	delete gr[1];
	delete mg;
	delete c1;
}

////////////////////////////////////////////////////////////////////////
/// Plotting function.
/// Plots two subsequent signals with markers for the rising edge of the
/// pulse and the moment of the breakdown.
/// The first is evaluated by means of the previous pulse. The latter is
/// derived from the moment where current and previous signal start to
/// deviate from each other. Both involve the use of thresholds.
/// \param[in] ch0 the Xbox DAQ channel containing the current signal.
/// \param[in] ch1 the Xbox DAQ channel containing the previous signal.
/// \param[in] jitter the time delay between the two signals.
/// \param[in] lowerlimit the lower limit of the considered time window.
/// \param[in] upperlimit the upper limit of the considered time window.
/// \param[in] xmarkers a vector of arguments used for horizontal markers
/// \return The time delay between rising edge and the moment of the breakdown.
void XboxAnalyserEvalBreakdownTest::plotSignal(const std::string &filepath,
		XBOX::XboxDAQChannel &ch0, XBOX::XboxDAQChannel &ch1,
		Double_t jitter, Double_t lowerlimit, Double_t upperlimit,
		const std::vector<Double_t> &xmarkers) {

	Int_t nsamples = 10001;
	Double_t lowerbound=0;
	Double_t upperbound=0;
	ch0.getTimeAxisBounds(lowerbound, upperbound);
	if(lowerlimit == -1)
		lowerlimit = lowerbound;
	if(upperlimit == -1)
		upperlimit = upperbound;

	// align signals
	ch1.setStartOffset(-jitter);

	// read data over the entire argument range of the channels
	XBOX::XboxSignalFilter filter;
	std::vector<Double_t> x = XBOX::linspace(lowerlimit, upperlimit, nsamples);
	std::vector<Double_t> y0 = filter(ch0, x);
	std::vector<Double_t> y1 = filter(ch1, x);
	std::vector<Double_t> yd(x.size()); // difference between y0 and y1
	for(auto ityd=yd.begin(), it0=y0.begin(),
			it1=y1.begin(); ityd != yd.end(); ++ityd, ++it0, ++it1)
		*ityd = fabs(*it0 - *it1);

	// general parameters
	std::string name = ch0.getChannelName();
	TTimeStamp ts = ch0.getTimeStamp();
	ULong64_t count = ch0.getPulseCount();

	// create figure
	char stitle[200];
	snprintf (stitle, 200, "%s %s PulseCount: %llu", name.c_str(), ts.AsString(), count);
	TCanvas *c1 = new TCanvas("c1",stitle,200,10,700,500);
	c1->SetGrid();

	// add graphs
	TMultiGraph *mg = new TMultiGraph();
	TGraph *gr[3];

	gr[0] = new TGraph(x.size(), &x[0], &y0[0]);
	gr[1] = new TGraph(x.size(), &x[0], &y1[0]);
	gr[2] = new TGraph(x.size(), &x[0], &yd[0]);

	gr[0]->SetLineColor(kRed);
	gr[0]->SetLineWidth(1);
	gr[0]->SetMarkerColor(kRed);
//	gr[0]->SetMarkerStyle(20);
//	gr[0]->SetMarkerSize(1);
	mg->Add(gr[0]);

	gr[1]->SetLineColor(kBlue);
	gr[1]->SetLineWidth(1);
	gr[1]->SetMarkerColor(kBlue);
//	gr[1]->SetMarkerStyle(20);
//	gr[1]->SetMarkerSize(0.1);
	mg->Add(gr[1]);

	gr[2]->SetLineColor(kGreen+2);
	gr[2]->SetLineWidth(1);
	gr[2]->SetMarkerColor(kGreen+2);
//	gr[2]->SetMarkerStyle(20);
//	gr[2]->SetMarkerSize(0.8);
	mg->Add(gr[2]);

	mg->SetTitle(stitle);
	mg->GetXaxis()->SetTitle("Time [s]");
	mg->GetYaxis()->SetTitle("Amplitude");
	mg->Draw("ACP");

	c1->Update(); // draws the frame, after which one can change it
	c1->Modified();

	// add markers for edge detection
	std::vector<TLine*> vlines(xmarkers.size());
	for(size_t i=0; i<xmarkers.size(); i++){
		vlines[i] = new TLine(xmarkers[i], c1->GetUymin(), xmarkers[i], c1->GetUymax());
		vlines[i]->SetLineWidth(1);
		vlines[i]->SetLineColor(kBlack);
		vlines[i]->Draw();
		vlines[i]->SetNDC(false);
	}

	// legend
	auto legend = new TLegend(0.11,0.7,0.4,0.89);//,0.48,0.9);
	legend->AddEntry(gr[0], (name + "_B0").c_str(),"l");
	legend->AddEntry(gr[1], (name + "_B1").c_str(),"l");
	legend->AddEntry(gr[2], (name + "_BD0 - " + name + "_BD1").c_str(),"l");
	legend->SetBorderSize(1);
	legend->SetFillColor(0);
	legend->SetTextSize(0);
	legend->Draw();

	// save file
	c1->Print(filepath.c_str());

	delete legend;
	for(auto line:vlines)
		delete line;
	delete gr[0];
	delete gr[1];
	delete gr[2];
	delete mg;
	delete c1;
}

////////////////////////////////////////////////////////////////////////
/// Constructor.
XboxAnalyserEvalBreakdownTest::XboxAnalyserEvalBreakdownTest() {
	init();
}

////////////////////////////////////////////////////////////////////////
/// Destructor.
XboxAnalyserEvalBreakdownTest::~XboxAnalyserEvalBreakdownTest() {
	clear();
}

////////////////////////////////////////////////////////////////////////
/// Default Settings.
void XboxAnalyserEvalBreakdownTest::init() {

	fPulseWMin = 0.01;
	fPulseWMax = 0.99;
	fPulseTh = 0.9;

	fJitterWMin = 0.01;
	fJitterWMax = 0.4;
	fJitterTh = 0.3;
	fJitterMax = 0.4;

	fRiseWMin = 0.01;
	fRiseWMax = 0.99;
	fRiseTh = 0.9;
	fRiseProx = 0.03;

	fDeflWMin = 0.01;
	fDeflWMax = 0.99;
	fDeflThC = 0.1;
	fDeflThF = 0.01;
	fDeflProx = 0.1;
}

////////////////////////////////////////////////////////////////////////
/// Clear.
void XboxAnalyserEvalBreakdownTest::clear(){
}

////////////////////////////////////////////////////////////////////////
/// Reset.
void XboxAnalyserEvalBreakdownTest::reset() {
	clear();
	init();
}

////////////////////////////////////////////////////////////////////////
/// Setter.
/// Set the time window and thresholds for the pulse shape analysis.
/// All parameters are relative to the maximum time window and the
/// magnitude of the signal in the considered time window.
/// \param[in] xmin The minimum of the signal time axis to be considered.
/// \param[in] xmax The maximum of the signal time axis to be considered.
/// \param[in] th The threshold at which the pulse top level is measured.
void XboxAnalyserEvalBreakdownTest::setPulseConfig(Double_t wmin, Double_t wmax, Double_t th) {

	fPulseWMin = wmin;
	fPulseWMax = wmax;
	fPulseTh = th;
}

////////////////////////////////////////////////////////////////////////
/// Setter.
/// Set the time window and threshold for the pulse shape analysis.
/// All parameters are relative to the maximum time window and the
/// magnitude of the signal in the considered time window.
/// \param[in] xmin The minimum of the signal time axis to be considered.
/// \param[in] xmax The maximum of the signal time axis to be considered.
/// \param[in] th The threshold at which the jitter is measured.
/// \param[in] max The maximum acceptable jitter (relative to maximum time window).
void XboxAnalyserEvalBreakdownTest::setJitterConfig(
		Double_t wmin, Double_t wmax, Double_t th, Double_t max) {

	fJitterWMin = wmin;
	fJitterWMax = wmax;
	fJitterTh = th;
	fJitterMax = max;
}

////////////////////////////////////////////////////////////////////////
/// Setter.
/// Set the time window and threshold for the precise detection of the moment
/// when the pulse rises. All parameters are relative to the maximum time
/// window and the magnitude of the signal in the considered time window.
/// \param[in] xmin The minimum of the signal time axis to be considered.
/// \param[in] xmax The maximum of the signal time axis to be considered.
/// \param[in] th The threshold at which the jitter is measured.
/// \param[in] prox The proximity is a refined interval to analyse the edge.
void XboxAnalyserEvalBreakdownTest::setRiseConfig(
		Double_t wmin, Double_t wmax, Double_t th, Double_t prox) {

	fRiseWMin = wmin;
	fRiseWMax = wmax;
	fRiseTh = th;
	fRiseProx = prox;
}

////////////////////////////////////////////////////////////////////////
/// Setter.
/// Set the time window and threshold for the precise detection of the
/// moment when the two signals start to deviate. All parameters are
/// relative to the maximum time window and the magnitude of the signal
/// in the considered time window.
/// \param[in] wmin The minimum of the signal time axis to be considered.
/// \param[in] wmax The maximum of the signal time axis to be considered.
/// \param[in] thc The coarse threshold measured relative to first signal.
/// \param[in] thc The fine threshold measured relative to first signal.
/// \param[in] prox The proximity is a refined interval to analyse the
///            deflection point betweem both signals.
void XboxAnalyserEvalBreakdownTest::setDeflConfig(Double_t wmin,
		Double_t wmax, Double_t thc, Double_t thf, Double_t prox) {

	fDeflWMin = wmin;
	fDeflWMax = wmax;
	fDeflThC = thc;
	fDeflThF = thf;
	fDeflProx = prox;
}

////////////////////////////////////////////////////////////////////////
/// Signal jitter evaluation.
/// Evaluates the jitter between two measurements based on a threshold.
/// \param[in] ch0 the Xbox DAQ channel containing the first signal.
/// \param[in] ch1 the Xbox DAQ channel containing the second signal.
/// \return The time delay between the two signals.
Double_t XboxAnalyserEvalBreakdownTest::evalJitter(
		XBOX::XboxDAQChannel &ch0, XBOX::XboxDAQChannel &ch1) {

	Double_t lbnd=0.; ///<Lower time axis bound.
	Double_t ubnd=0.; ///<Upper time axis bound.
	Double_t tmin; ///<Lower time axis limit.
	Double_t tmax; ///<Upper time axis limit.

	const Int_t nsamples = 10001; // number of samples for interpolating

	// avoid data re-interpretation ....................................
	ch0.setAutoRefresh(false);
	ch1.setAutoRefresh(false);
	ch0.flushbuffer();
	ch1.flushbuffer();

	// get the signal and apply filters (chose carefully the filter
	// parameters according to the signal distortion) ..................
	ch0.getTimeAxisBounds(lbnd, ubnd);
	tmin = fJitterWMin * (ubnd - lbnd) + lbnd;
	tmax = fJitterWMax * (ubnd - lbnd) + lbnd;

	XBOX::XboxSignalFilter filt;
	std::vector<Double_t> t = XBOX::linspace(tmin, tmax, nsamples);
	std::vector<Double_t> y0 = filt(ch0, t);
	std::vector<Double_t> y1 = filt(ch1, t);

	// determine the delay between the signals based on threshold ......
	Double_t th = fJitterTh * (*std::max_element(y0.begin(), y0.end()));

	Double_t idx0 = 0.;
	for(UInt_t i=1; i<y0.size(); i++){
		if(y0[i] > th){
			idx0 = Double_t(i - (y0[i] - th) / (y0[i] - y0[i-1]));
			break;
		}
	}
	Double_t idx1 = 0.;
	for(UInt_t i=1; i<y1.size(); i++){
		if(y1[i] > th){
			idx1 = Double_t(i - (y1[i] - th) / (y1[i] - y1[i-1]));
			break;
		}
	}
	Double_t delay = (idx1 - idx0) * (t[1] - t[0]);

	// debug information ...............................................
	printf("ubnd: %e | lbnd: %e\n", lbnd, ubnd);
	printf("xmin: %e | xmax: %e | th: %f\n", fJitterWMin, fJitterWMax, fJitterTh);
	printf("tmin: %e | tmax: %e | th: %f\n", tmin, tmax, th);

	// test plot .......................................................
//	if (delay > 0.5e-8 && delay < 0.6e-8) {
		std::string ts = ch0.getTimeStamp().AsString();
		std::string name = ch0.getChannelName();
		plotSignal("pictures/" + ts + "_Jitter0_" + name + ".png", ch0, ch1, 0.);
		plotSignal("pictures/" + ts + "_Jitter1_" + name + ".png", ch0, ch1, 0., 5*tmin, tmax*0.3);
		plotSignal("pictures/" + ts + "_Jitter2_" + name + ".png", ch0, ch1, delay, 5*tmin, tmax*0.3);
//	}

	// check whether delay is below a maximum acceptable value ........
	if (fabs(delay) > fJitterMax * (ubnd - lbnd))
		delay = -1.;

	// reset auto interpretation ......................................
	ch0.setAutoRefresh(true);
	ch1.setAutoRefresh(true);

	return delay;
}


////////////////////////////////////////////////////////////////////////
/// Rising edge.
/// Precise evaluation of moment when the rising edge of a pulse starts.
/// \param[in] ch0 the Xbox DAQ channel containing the current signal.
/// \return    time at the start of the rising edge.
Double_t XboxAnalyserEvalBreakdownTest::evalRisingEdge(
		XBOX::XboxDAQChannel &ch) {

	Double_t lbnd=0.; ///<Lower time axis bound.
	Double_t ubnd=0.; ///<Upper time axis bound.
	Double_t tmin; ///<Lower time axis limit.
	Double_t tmax; ///<Upper time axis limit.

	const Int_t nsamples = 512; // number of samples for interpolating
	const Int_t nwin = 8; // number of samples for moving window

	// avoid data re-interpretation ....................................
	ch.setAutoRefresh(false);
	ch.flushbuffer();

	// first rough calculation of the rising edge.......................
	ch.getTimeAxisBounds(lbnd, ubnd);
	tmin = fPulseWMin * (ubnd - lbnd) + lbnd;
	tmax = fPulseWMax * (ubnd - lbnd) + lbnd;
	Double_t tref = ch.risingEdge(fRiseTh, tmin, tmax);

	printf("Coarse evaluate Rising edge: %s\n", ch.getChannelName().c_str());
	printf("lbnd: %e | ubnd: %e\n", lbnd, ubnd);
	printf("tmin: %e | tmax: %e\n", tmin, tmax);
	printf("tref: %e | \n", tref);

	// refine resolution around the rising edge ........................
	tmin = tref - 0.5 * fRiseProx * (ubnd - lbnd);
	tmax = tref + 0.5 * fRiseProx * (ubnd - lbnd);
	if (tmin < lbnd)
		tmin = lbnd;
	if (tmax > ubnd)
		tmax = ubnd;
	std::vector<Double_t> traw = ch.getTimeAxis(tmin, tmax);
	std::vector<Double_t> yraw = ch.getSignal(tmin, tmax);

	std::vector<Double_t> t = XBOX::linspace(tmin, tmax, nsamples);
	Double_t dt = (tmax - tmin) / (nsamples - 1);

	// get the signal and apply filters (chose carefully the filter
	// parameters according to the signal distortion) ..................
	XBOX::XboxSignalFilter filt0(XBOX::XboxSignalFilter::kSavitzkyGolay, 3, 7, 7);
	XBOX::XboxSignalFilter filt1(XBOX::XboxSignalFilter::kSavitzkyGolay, 3, 15, 15);
	XBOX::XboxSignalFilter filt2(XBOX::XboxSignalFilter::kSavitzkyGolay, 3, 15, 15);
	filt1.setDerivative(1); // first derivative filter
	filt2.setDerivative(2); // second derivative filter

	std::vector<Double_t> y0 = filt0(ch, t); // normal signal
	std::vector<Double_t> y1 = filt1(ch, t); // first derivative
	std::vector<Double_t> y2 = filt2(ch, t); // second derivative

	// find closest maximum to the right ...............................
	Int_t idxMax = nsamples/2;
	if (y1[idxMax] > 0) {

		for(Int_t i=nsamples/2; i< nsamples; i++){
			if (y1[i+1] < 0 && y1[i] > 0) {
				idxMax = i;
				break;
			}
		}
	}

	// find inflection point of the rising edge ........................
	Int_t idxInfl = nsamples/2; // point of inflection
	for(Int_t i=idxMax; i>10; i--){

		if (y2[i-1] > 0 && y2[i] < 0) {
			idxInfl = i;
			break;
		}
	}

	// find inflection point of the rising edge. This is an attempt for noisy
	// derivative signals but it is not so successful.
//	for(Int_t i=nsamples/2; i>nwin; i--){
//
//		Double_t lhs = std::accumulate(y2.begin()+i-nwin, y2.begin()+i-1, 0.);
//		Double_t rhs = std::accumulate(y2.begin()+i+1, y2.begin()+i+nwin, 0.);
//		if (lhs > 0 && rhs < 0) {
//
//			Double_t avg = fabs(lhs + rhs);
//			for (Int_t j=1; j < nwin; j++) {
//
//				Double_t tmp = std::accumulate(y2.begin()+i-j-nwin, y2.begin()+i-j+nwin, 0.);
//				if (fabs(tmp) < avg)
//					avg = fabs(tmp);
//				else {
//					idxInfl = i-j+1;
//					break;
//				}
//			}
//			idxInfl = i;
//			break;
//		}
//	}

	// cross the tangent in the inflection point with zero ground ......
	Double_t slope = std::accumulate(y1.begin() + idxInfl - nwin,
			y1.begin() + idxInfl + nwin, 0.) / (2*nwin + 1);
	Int_t idxZero = idxInfl - Int_t(y0[idxInfl] / slope / dt);


	// point of maximum curvature between zero crossing and inflection
	// point consider interval with idxZero in the center and idxInfl
	// as upper bound ..................................................
//	Int_t idxCurv = (2*idxZero-idxInfl > 0) ? 2*idxZero-idxInfl : 0;
	Int_t idxCurv = 32;
//	if (idxZero < idxInfl)
//		idxCurv = idxZero;
//	else if (idxInfl/2 > 32)
//		idxCurv = idxInfl/2;

	Double_t max = y2[idxCurv];
	for(Int_t i=idxCurv; i<idxInfl; i++) {
		if (y2[i] > max) {
			max = y2[i];
			idxCurv = i;
		}
	}

	// final point where the rising edge starts. The point of highest
	// curvature seems to be the best option ...........................
//	Int_t idxRise = (idxZero + idxCurv) / 2;
	Int_t idxRise = idxCurv;
//	Int_t idxRise = idxZero;
//	Double_t slopeCorr = (y0[idxInfl] - y0[idxCurv]) / (t[idxInfl] - t[idxCurv]);
//	Int_t idxRise = idxCurv - Int_t(y0[idxCurv] / slopeCorr / dt);

	// debug information ...............................................
	printf("Evaluate Rising edge: %s\n", ch.getChannelName().c_str());
	printf("lbnd: %e | ubnd: %e\n", lbnd, ubnd);
	printf("tref: %e | fEdgeProx: %e\n", tref, fRiseProx);
	printf("tmin: %e | tmax: %e\n", tmin, tmax);
	printf("tmin: %e | tmax: %e\n", t.front(), t.back());
	printf("Infl: point: %d | %e\n", idxInfl, t[idxInfl]);
	printf("Zero: point: %d | %e\n", idxZero, t[idxZero]);
	printf("Curv: point: %d | %e\n", idxCurv, t[idxCurv]);
	printf("Rise: point: %d | %e\n", idxRise, t[idxRise]);

	// test plot .......................................................
	TCanvas c1("c1","c1",200,10,700,500);
	c1.SetGrid();

	std::vector<Double_t> yrawn = norm(yraw);
	std::vector<Double_t> y0n = norm(y0);
	std::vector<Double_t> y1n = norm(y1);
	std::vector<Double_t> y2n = norm(y2);

	TGraph gr0(traw.size(), &traw[0], &yraw[0]);
	TGraph gr1(t.size(), &t[0], &y0n[0]);
	TGraph gr2(t.size(), &t[0], &y1n[0]);
	TGraph gr3(t.size(), &t[0], &y2n[0]);

	gr0.SetLineColor(kBlack);
	gr1.SetLineColor(kBlue);
	gr2.SetLineColor(kGreen+2);
	gr3.SetLineColor(kRed);

	gr0.SetMarkerColor(kBlack);
	gr1.SetMarkerColor(kBlue);
	gr2.SetMarkerColor(kGreen+2);
	gr3.SetMarkerColor(kOrange+2);

	gr0.Draw("ACP");
	gr1.Draw("CP");
	gr2.Draw("CP");
	gr3.Draw("CP");

	Double_t ymin = -0.5;
	Double_t ymax = 1.2;

	auto axis = gr0.GetYaxis();
	axis->SetRangeUser(ymin,ymax);
	axis->SetRangeUser(ymin,ymax);

	TLine v1(t[idxInfl], ymin, t[idxInfl], ymax);
	TLine v2(t[idxZero], ymin, t[idxZero], ymax);
	TLine v3(t[idxCurv], ymin, t[idxCurv], ymax);
	TLine v4(t[idxRise], ymin, t[idxRise], ymax);
	v4.SetLineColor(kRed);
	v1.Draw();
	v2.Draw();
	v3.Draw();
	v4.Draw();

	std::string ts = ch.getTimeStamp().AsString();
	std::string name = ch.getChannelName();
	c1.Print(("pictures/" + ts + "_Rise_" + name + ".png").c_str());

	// reset auto interpretation .......................................
	ch.setAutoRefresh(true);

	return t[idxRise];
}


void XboxAnalyserEvalBreakdownTest::fitExponential(Int_t n, Double_t *x, Double_t *y,
		Double_t &p0, Double_t &p1) {

	std::vector<Double_t> lny(n);
	Double_t xsum=0; ///<Sum of xi.
	Double_t x2sum=0; ///<Sum of xi^2.
	Double_t ysum=0; ///<Sum of yi.
	Double_t xysum=0; ///<Sum of xi*yi.

	Double_t xscale = 1e6;
	for(Int_t i=0; i<n; i++) {

		lny[i] = log(fabs(y[i])+1e-9);
		xsum=xsum+x[i] * xscale;
		ysum=ysum+lny[i];
		x2sum=x2sum+x[i]*x[i]*xscale*xscale;
		xysum=xysum+x[i]*lny[i]*xscale;
	}

	p0=(x2sum*ysum-xsum*xysum)/(x2sum*n-xsum*xsum); //intercept
	p1=(n*xysum-xsum*ysum)/(n*x2sum-xsum*xsum)*xscale; // slope

}


////////////////////////////////////////////////////////////////////////
/// Deviation between two signals.
/// Precise evaluation of moment when both signals start to deviate.
/// Assumes that both signals are of same type.
/// \param[in] ch0 the Xbox DAQ channel containing the current signal.
/// \return    time at the start of the rising edge.
Double_t XboxAnalyserEvalBreakdownTest::evalDeflection(
		XBOX::XboxDAQChannel &ch0, XBOX::XboxDAQChannel &ch1) {

	const Int_t nsamples = 1024; ///<Number of interpolation points.
	const Int_t nwin = 32; ///<Number of points for moving window.
	const Int_t nwinc = 32; ///<Number of points for coarse moving window.

	Double_t lbnd=0.; ///<Lower time axis bound.
	Double_t ubnd=0.; ///<Upper time axis bound.
	Double_t tmin; ///<Lower time axis limit.
	Double_t tmax; ///<Upper time axis limit.
	std::vector<Double_t> t; ///<Time axis.

	std::vector<Double_t> ych0; ///<Signal of channel 0.
	std::vector<Double_t> ych1; ///<Signal of channel 1.
	std::vector<Double_t> y; ///<Differential signal.
	std::vector<Double_t> y1; ///<First derivative of differential signal.
	std::vector<Double_t> y2; ///<Second derivative of differential signal.

	Double_t y0max; ///<Maximum of Signal of channel 0 in considered range.
	Double_t y1max; ///<Maximum of Signal of channel 0 in considered range.

	Double_t th; // absolute threshold
	Bool_t bnoise = false; // is signal within noise level

	///<Filter configurations ..........................................
	XBOX::XboxSignalFilter filtn0(XBOX::XboxSignalFilter::kSavitzkyGolay, 3, 7, 7);
	XBOX::XboxSignalFilter filtn1(XBOX::XboxSignalFilter::kSavitzkyGolay, 3, 20, 20);
	XBOX::XboxSignalFilter filtn2(XBOX::XboxSignalFilter::kSavitzkyGolay, 1, 7, 7);

	// avoid data re-interpretation ....................................
	ch0.setAutoRefresh(false);
	ch1.setAutoRefresh(false);
	ch0.flushbuffer();
	ch1.flushbuffer();

	// First rough calculation of the deviation time based on threshold
	// which is defined as a fraction of the signal peak ...............
	ch0.getTimeAxisBounds(lbnd, ubnd);
	tmin = fDeflWMin * (ubnd - lbnd) + lbnd;
	tmax = fDeflWMax * (ubnd - lbnd) + lbnd;
//	tmin = 0.01 * (ubnd - lbnd) + lbnd;
//	tmax = 0.99 * (ubnd - lbnd) + lbnd;

	t = XBOX::linspace(tmin, tmax, 10*nsamples);
	ych0 = filtn0(ch0, t);
	ych1 = filtn0(ch1, t);

	// get difference between both signal and determine the maximum ....
	y.resize(t.size());
	for(size_t i=0; i<t.size(); i++)
		y[i] = ych0[i] - ych1[i];
//	y = filtn2(y); // apply noise filter

	Double_t maxval = 0;
	for(size_t i=0; i<t.size(); i++) {
		if(fabs(y[i]) > maxval)
			maxval = fabs(y[i]);
	}


	// check whether differential signal is in noise level .............
	y0max = fabs(ch0.max(tmin, tmax));
	y1max = fabs(ch1.max(tmin, tmax));
	th = fabs(fDeflThC * max(y0max, y1max));

	if (maxval < th) {
		bnoise = true;
	}

	if (bnoise) {

		// test plot
		TCanvas c1("c1","c1",200,10,700,500);
		c1.SetGrid();

		TGraph gr0(t.size(), &t[0], &ych0[0]);
		TGraph gr1(t.size(), &t[0], &ych1[0]);
		TGraph gr2(t.size(), &t[0], &y[0]);

		gr0.SetLineColor(kRed);
		gr1.SetLineColor(kBlue);
		gr2.SetLineColor(kGreen+2);

		gr0.SetMarkerColor(kRed);
		gr1.SetMarkerColor(kBlue);
		gr2.SetMarkerColor(kGreen+2);

		gr0.Draw("ACP");
		gr1.Draw("CP");
		gr2.Draw("CP");

		std::string ts = ch0.getTimeStamp().AsString();
		std::string name = ch0.getChannelName();
		c1.Print(("pictures/" + ts + "_Defl_" + name + ".png").c_str());

		return -1.;
	}

	// get index where the threshold is exceeded the first time ........
	Int_t idxDefl = 0;

//	for(UInt_t i=0; i<t.size(); i++){
//		if(fabs(y[i]) > th){
//			idxDefl = i;
//			break;
//		}
//	}

	for(UInt_t i=0; i<t.size()-nwinc; i++){
		Int_t ncnt = 0;
		for (UInt_t j=0; j<nwinc; j++){
			if(fabs(y[i+j]) > th)
				ncnt++;
		}
		if(ncnt == nwinc){
			idxDefl = i;
			break;
		}
	}

	if (idxDefl==0)
		return -1.;

	// refine resolution around the deflection .........................
	Double_t tref = t[idxDefl];
	ch0.flushbuffer();
	ch1.flushbuffer();

	tmin = tref - 0.5 * fDeflProx * (ubnd - lbnd);
	tmax = tref + 0.5 * fDeflProx * (ubnd - lbnd);
//	tmin = 2e-6;
//	tmax = 4e-6;

	if (tmin < lbnd)
		tmin = lbnd;
	if (tmax > ubnd)
		tmax = ubnd;

	t = XBOX::linspace(tmin, tmax, nsamples);
//	Double_t dt = (tmax - tmin) / (nsamples - 1);

	ych0 = filtn0(ch0, t); // Signal of channel 0.
	ych1 = filtn0(ch1, t); // Signal of channel 1.
	y.resize(t.size()); // Differential signal.
	for(size_t i=0; i<t.size(); i++)
		y[i] = (ych0[i] - ych1[i]);
	y = filtn1(y);

	// refine the deflection point .....................................
	y0max = fabs(ch0.max(tmin, tmax));
	y1max = fabs(ch1.max(tmin, tmax));
	th = fabs(fDeflThF * max(y0max, y1max));
	idxDefl=0;
	for(Int_t i=0.5*nsamples; i > nwin; i--) {

		Double_t sum = 0; // sum
		Double_t sum2 = 0; // sum of the squares
		for (Int_t j=-nwin; j < nwin+1; j++) {
			sum += y[i + j];
			sum2 += y[i + j]*y[i + j];
		}
		Double_t mean = sum/(2*nwin+1);
		Double_t stddev = sqrt((sum2 - sum*sum)/(2*nwin+1));

		if (fabs(y[i]) < th) { // use y[i], mean, or stdev
			idxDefl = i;
			break;
		}
	}

//	if (idxDefl <= nsamples / 2 - nwin){
//
//		// if mean is used
//		for(Int_t i=nwin; i > -nwin-1; i--) {
//			if (fabs(y[idxDefl+i]) < th) {
//				idxDefl += i;
//				break;
//			}
//		}
//
//		// if stddev is used
////		for(Int_t i=0; i < nwin; i++) {
////			if (fabs(y[idxDefl-i]) < th) {
////				idxDefl -= i;
////				break;
////			}
////		}
//	}

	// fit an exponential curve to the differential signal ..............
	Double_t p0 = 0.;
	Double_t p1 = 0.;
	fitExponential(nsamples*0.6, &t[0], &y[0], p0, p1);

	// alternative fitting approach using svd ...........................
	Int_t n = nsamples * 0.55;
	TMatrixD A(n, 2);
	TMatrixDColumn(A,0) = 1.0;
	TVectorD b(n);
	for(Int_t i=0; i < n; i++) {
		A(i, 1) = t[i];
		b(i) = log(fabs(y[i])+1e-9);
	}

	TDecompSVD svd(A);
	bool ok;
	const TVectorD svdcoef = svd.Solve(b,ok);

	// time at which the exponential fit exceeds the treshold ..........
	Double_t tfit = (log(fDeflThF*ch1.max()) - p0) / p1;

	// fitting curves ..................................................
	std::vector<Double_t> yfit1(nsamples, 0.);
	std::vector<Double_t> yfit2(nsamples, 0.);
	for(size_t i=0; i < t.size(); i++) {
		yfit1[i] = -exp(p0 + p1*t[i]);
		yfit2[i] = -exp(svdcoef[0] + svdcoef[1]*t[i]);
	}

	// debug information ...............................................
	printf("Evaluate deflection: %s\n", ch0.getChannelName().c_str());
	printf("lbnd: %e | ubnd: %e\n", lbnd, ubnd);
	printf("tmin: %e | tmax: %e\n", tmin, tmax);
	printf("Defl: point: %d | %e\n", idxDefl, t[idxDefl]);
	printf("Fit svd    : %e | %e\n", svdcoef[0], svdcoef[1]);
	printf("Fit own    : %e | %e\n", p0, p1);

	// test plot
	TCanvas c1("c1","c1",200,10,700,500);
	c1.SetGrid();

	TGraph gr0(t.size(), &t[0], &ych1[0]);
	TGraph gr1(t.size(), &t[0], &ych0[0]);
	TGraph gr2(t.size(), &t[0], &y[0]);
	TGraph gr3(t.size(), &t[0], &yfit1[0]);

	gr0.SetLineColor(kBlack);
	gr1.SetLineColor(kBlue);
	gr2.SetLineColor(kGreen+2);
	gr3.SetLineColor(kRed);

	gr0.SetMarkerColor(kBlack);
	gr1.SetMarkerColor(kBlue);
	gr2.SetMarkerColor(kGreen+2);
	gr3.SetMarkerColor(kRed);

//	gr0.Draw("ACP");
//	gr1.Draw("CP");
//	gr2.Draw("CP");
//	gr3.Draw("CP");

	TMultiGraph *mg = new TMultiGraph();
	mg->Add(&gr0);
	mg->Add(&gr1);
	mg->Add(&gr2);
//	mg->Add(&gr3);
	mg->Draw("ACP");
	gr3.Draw("CP");

	Double_t ymin = mg->GetYaxis()->GetXmin();
	Double_t ymax = mg->GetYaxis()->GetXmax();

	auto axis = gr0.GetYaxis();
	axis->SetRangeUser(ymin,ymax);
	axis->SetRangeUser(ymin,ymax);

	TLine v1(tref, ymin, tref, ymax);
	TLine v2(tfit, ymin, tfit, ymax);
	TLine v3(t[idxDefl], ymin, t[idxDefl], ymax);

	v2.SetLineColor(kBlue);
	v3.SetLineColor(kRed);
	v1.Draw();
	v2.Draw();
	v3.Draw();

	std::string ts = ch0.getTimeStamp().AsString();
	std::string name = ch0.getChannelName();
	c1.Print(("pictures/" + ts + "_Defl_" + name + ".png").c_str());

	// reset auto interpretation .......................................
	ch0.setAutoRefresh(true);
	ch1.setAutoRefresh(true);

	return t[idxDefl];
}

////////////////////////////////////////////////////////////////////////
/// Calling function.
/// Evaluates pulse shape parameter from a channel signal.
/// \param[in] b0psi The PSI amplitude signal of the breakdown pulse.
/// \param[in] b1psi The PSI amplitude signal of the previous pulse.
/// \param[in] b0pei The PEI amplitude signal of the breakdown pulse.
/// \param[in] b1pei The PEI amplitude signal of the previous pulse.
/// \param[in] b0psr The PSR amplitude signal of the breakdown pulse.
/// \param[in] b1psr The PSR amplitude signal of the previous pulse.
/// \return The pulse parameters (pulse length, average power, ...)
XBOX::XboxAnalyserEntry XboxAnalyserEvalBreakdownTest::operator () (
		XBOX::XboxDAQChannel &b0psi, XBOX::XboxDAQChannel &b1psi,
		XBOX::XboxDAQChannel &b0pei, XBOX::XboxDAQChannel &b1pei,
		XBOX::XboxDAQChannel &b0psr, XBOX::XboxDAQChannel &b1psr) {

	// put the results in the model container to return ................
	XBOX::XboxAnalyserEntry model;
	model.setXboxVersion(b1psi.getXboxVersion());
	model.setTimeStamp(b1psi.getTimeStamp());

	model.setLogType(b1psi.getLogType());
	model.setPulseCount(b1psi.getPulseCount());
	model.setDeltaF(b1psi.getDeltaF());
	model.setLine(b1psi.getLine());
	model.setBreakdownFlag(b1psi.getBreakdownFlag());

	// jitter based on b0psi and b1psi .................................
	Double_t jitter = evalJitter(b0psi, b1psi);
	if (jitter == -1)
		return model; // return current data set

	model.setJitter(jitter);
	b1psi.setStartOffset(-jitter);
	b1pei.setStartOffset(-jitter);
	b1psr.setStartOffset(-jitter);

	// pulse shape based on b1psi ......................................
	Double_t lbnd=0.;
	Double_t ubnd=0.;
	b1psi.getTimeAxisBounds(lbnd, ubnd);

	Double_t tmin = fPulseWMin * (ubnd - lbnd) + lbnd;
	Double_t tmax = fPulseWMax * (ubnd - lbnd) + lbnd;
	Double_t tr = b1psi.risingEdge(fPulseTh, tmin, tmax);
	Double_t tf = b1psi.fallingEdge(fPulseTh, tmin, tmax);
	Double_t pmax = b1psi.max(tr, tf);
	Double_t pavg = b1psi.mean(tr, tf);

	model.setPulseRisingEdge(tr);
	model.setPulseFallingEdge(tf);
	model.setPulseLength(tf - tr);
	model.setPulsePowerAvg(pavg);
	model.setPulsePowerMax(pmax);

	// rising edges of b1pei and b1psr ................................
	Double_t risepei = evalRisingEdge(b1pei);
	Double_t risepsr = evalRisingEdge(b1psr);
	model.setTranRisingEdge(risepei);
	model.setReflRisingEdge(risepsr);

	// time of deflection for b0pei and b0psr from b0pei and b0pei ....
	Double_t deflpei = evalDeflection(b0pei, b1pei);
	Double_t deflpsr = evalDeflection(b0psr, b1psr);

	if (deflpei < 0.3e-6)
		deflpei = -1.;
	if (deflpsr < 0.3e-6)
			deflpsr = -1.;
	model.setTranBreakdownTime(deflpei);
	model.setReflBreakdownTime(deflpsr);

	// time related to the breakdown location .........................
	Double_t bdtime = 0.5 * (deflpsr - risepsr - deflpei + risepei);
	model.setBreakdownTime(bdtime);

//	// for visual inspection ..........................................
//	Double_t twin = 1e-7;
//	Int_t nwin = 1024;
//	XBOX::XboxSignalFilter filt(
//			0, XBOX::XboxSignalFilter::kSavitzkyGolay, 3, 7, 7);
//
//	b0psi.getTimeAxisBounds(lbnd, ubnd);
//
//	std::vector<Double_t> yrise_pei(nwin, 0);
//	std::vector<Double_t> yrise_psr(nwin, 0);
//	std::vector<Double_t> ydefl_pei(nwin, 0);
//	std::vector<Double_t> ydefl_psr(nwin, 0);
//
//	b1pei.flushbuffer();
//	if ((risepei > (lbnd + twin)) && (risepei < (ubnd - twin)))
//		yrise_pei = filt(b1pei, risepei-twin, risepei+twin, nwin);
//
////	printf("lbnd: %e | ubnd: %e | risepsr: %e\n", lbnd, ubnd, risepsr);
//
//	b1psr.flushbuffer();
//	if ((risepsr > (lbnd + twin)) && (risepsr < (ubnd - twin)))
//		yrise_psr = filt(b1psr, risepsr-twin, risepsr+twin, nwin);
//
//	b0pei.flushbuffer();
//	b1pei.flushbuffer();
//	if (deflpei > lbnd + twin && deflpei < ubnd - twin) {
//		std::vector<Double_t> ydefl_b0pei = filt(b0pei, deflpei-twin, deflpei+twin, nwin);
//		std::vector<Double_t> ydefl_b1pei = filt(b1pei, deflpei-twin, deflpei+twin, nwin);
//		for (Int_t i=0; i<nwin; i++)
//			ydefl_pei[i] = ydefl_b0pei[i] - ydefl_b1pei[i];
//	}
//
//	b0psr.flushbuffer();
//	b1psr.flushbuffer();
//	if (deflpsr > lbnd + twin && deflpsr < ubnd - twin) {
//		std::vector<Double_t> ydefl_b0psr = filt(b0psr, deflpsr-twin, deflpsr+twin, nwin);
//		std::vector<Double_t> ydefl_b1psr = filt(b1psr, deflpsr-twin, deflpsr+twin, nwin);
//		for (Int_t i=0; i<nwin; i++)
//			ydefl_psr[i] = ydefl_b0psr[i] - ydefl_b1psr[i];
//	}
//
//	model.setProbeTranRisingEdge(yrise_pei);
//	model.setProbeTranBreakdownTime(ydefl_pei);
//	model.setProbeReflRisingEdge(yrise_psr);
//	model.setProbeReflBreakdownTime(ydefl_psr);

	return model;
}


#ifndef XBOX_NO_NAMESPACE
}
#endif



