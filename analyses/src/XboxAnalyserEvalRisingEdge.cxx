#include "XboxAnalyserEvalRisingEdge.hxx"

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
/// Constructor.
XboxAnalyserEvalRisingEdge::XboxAnalyserEvalRisingEdge() {
	init();
}

////////////////////////////////////////////////////////////////////////
/// Constructor.
XboxAnalyserEvalRisingEdge::XboxAnalyserEvalRisingEdge(Double_t wmin,
		Double_t wmax, Double_t th, Double_t prox) {
	init();
	config(wmin, wmax, th, prox);
}

////////////////////////////////////////////////////////////////////////
/// Destructor.
XboxAnalyserEvalRisingEdge::~XboxAnalyserEvalRisingEdge() {
	clear();
}

////////////////////////////////////////////////////////////////////////
/// Default Settings.
void XboxAnalyserEvalRisingEdge::init() {

	fWmin = 0.01;
	fWmax = 0.99;
	fTh = 0.6;
	fProximity = 0.03;

	// Filter configurations
	fFilterSig.config(XBOX::XboxSignalFilter::kSavitzkyGolay, 3, 7, 7);
	fFilterD1.config(XBOX::XboxSignalFilter::kSavitzkyGolay, 3, 15, 15);
	fFilterD2.config(XBOX::XboxSignalFilter::kSavitzkyGolay, 3, 15, 15);
	fFilterD1.setDerivative(1); // first derivative filter
	fFilterD2.setDerivative(2); // second derivative filter

	fSamplesRefine = 512;
	fWindowSize = 8;

	fReportFlag = false;
	fReportDir = "";
}

////////////////////////////////////////////////////////////////////////
/// Clear.
void XboxAnalyserEvalRisingEdge::clear(){
}

////////////////////////////////////////////////////////////////////////
/// Reset.
void XboxAnalyserEvalRisingEdge::reset() {
	clear();
	init();
}


////////////////////////////////////////////////////////////////////////
/// Setter.
/// Set the time window and threshold for the precise detection of the moment
/// when the pulse rises. All parameters are relative to the maximum time
/// window and the magnitude of the signal in the considered time window.
/// \param[in] wmin The minimum of the signal time axis to be considered.
/// \param[in] wmax The maximum of the signal time axis to be considered.
/// \param[in] th The threshold at which the jitter is measured.
/// \param[in] prox The proximity is a refined interval to analyse the edge.
void XboxAnalyserEvalRisingEdge::config(
		Double_t wmin, Double_t wmax, Double_t th, Double_t prox) {

	fWmin = wmin;
	fWmax = wmax;
	fTh = th;
	fProximity = prox;
}

size_t XboxAnalyserEvalRisingEdge::argMax(std::vector<Double_t> &y, size_t imin, size_t imax) {

	Int_t idx = imin;
	Double_t max = y[imin];
	for(size_t i=imin; i<imax; i++) {
		if (y[i] > max) {
			max = y[i];
			idx = i;
		}
	}

	return idx;
}

size_t XboxAnalyserEvalRisingEdge::argLeftRoot(std::vector<Double_t> &y, size_t istart) {

	if (y.empty() || y.size() < istart)
		return 0;

	size_t idxRoot = 0;
	if (y[istart] > 0) {
		for(size_t i=istart; i>0; i--){
			if (y[i-1] < 0 && y[i] > 0) {
				idxRoot = i;
				break;
			}
		}
	}
	else {

		for(size_t i=istart; i<y.size(); i--){
			if (y[i-1] > 0 && y[i] < 0) {
				idxRoot = i;
				break;
			}
		}
	}
	return idxRoot;
}

size_t XboxAnalyserEvalRisingEdge::argRightRoot(std::vector<Double_t> &y, size_t istart) {

	if (y.empty() || y.size() < istart)
		return 0;

	size_t idxRoot = 0;
	if (y[istart] > 0) {
		for(size_t i=istart; i<y.size(); i++){
			if (y[i+1] < 0 && y[i] > 0) {
				idxRoot = i;
				break;
			}
		}
	}
	else {

		for(size_t i=istart; i<y.size(); i++){
			if (y[i+1] > 0 && y[i] < 0) {
				idxRoot = i;
				break;
			}
		}
	}
	return idxRoot;
}

///////////////////////////////////////////////////////////////////////////////
/// Normalisation.
/// Re-scale a data set to a predefined argument range.
std::vector<Double_t> XboxAnalyserEvalRisingEdge::rescale(
		std::vector<Double_t> &y, Double_t ysmin, Double_t ysmax) {

	Double_t ymin = *std::min_element(y.begin(), y.end());
	Double_t ymax = *std::max_element(y.begin(), y.end());

	std::vector<Double_t> ys(y.size(), 0.);

	for (size_t i=0; i<y.size(); i++)
		ys[i] = (ysmax - ysmin)*(y[i]) / (ymax - ymin) ;

	return ys;
}


////////////////////////////////////////////////////////////////////////
/// Rising edge.
/// Precise evaluation of moment when the rising edge of a pulse starts.
/// \param[in] ch0 the Xbox DAQ channel containing the current signal.
/// \return    time at the start of the rising edge.
Double_t XboxAnalyserEvalRisingEdge::evalRisingEdge(XBOX::XboxDAQChannel &ch) {

	// first rough calculation of the rising edge.......................
	Double_t lbnd=0.; ///<Lower time axis bound.
	Double_t ubnd=0.; ///<Upper time axis bound.
	ch.getTimeAxisBounds(lbnd, ubnd);
	Double_t tmin = fWmin * (ubnd - lbnd) + lbnd; ///<Lower limit.
	Double_t tmax = fWmax * (ubnd - lbnd) + lbnd; ///<Upper limit.
	Double_t tref = ch.risingEdge(fTh, tmin, tmax);

	// refine resolution around the rising edge and apply filters ......
	tmin = tref - 0.5 * fProximity * (ubnd - lbnd);
	tmax = tref + 0.5 * fProximity * (ubnd - lbnd);
	if (tmin < lbnd)
		tmin = lbnd;
	if (tmax > ubnd)
		tmax = ubnd;

	std::vector<Double_t> tf = XBOX::linspace(tmin, tmax, fSamplesRefine);
	Double_t dt = (tmax - tmin) / (fSamplesRefine - 1);

	std::vector<Double_t> yf = fFilterSig(ch, tf); // normal signal
	std::vector<Double_t> y1f = fFilterD1(ch, tf); // first derivative
	std::vector<Double_t> y2f = fFilterD2(ch, tf); // second derivative

	// find closest maximum to the right ...............................
	size_t idxMax = argRightRoot(y1f, fSamplesRefine/2);
//	Int_t idxMax = fSamplesRefine/2;
//	if (y1[idxMax] > 0) {
//
//		for(Int_t i=fSamplesRefine/2; i< fSamplesRefine; i++){
//			if (y1[i+1] < 0 && y1[i] > 0) {
//				idxMax = i;
//				break;
//			}
//		}
//	}

	// find inflection point of the rising edge ........................
	size_t idxInfl = argLeftRoot(y2f, idxMax);
//	Int_t idxInfl = fSamplesRefine/2; // point of inflection
//	for(Int_t i=idxMax; i>10; i--){
//
//		if (y2[i-1] > 0 && y2[i] < 0) {
//			idxInfl = i;
//			break;
//		}
//	}

	// cross the tangent in the inflection point with zero ground ......
	Double_t slope = std::accumulate(y1f.begin() + idxInfl - fWindowSize,
			y1f.begin() + idxInfl + fWindowSize, 0.) / (2*fWindowSize + 1);
	size_t idxShift = yf[idxInfl] / (slope * dt);
	size_t idxZero = 0;
	if (idxInfl > idxShift)
		idxZero = idxInfl - idxShift;

	// point of maximum curvature between zero crossing and inflection
	// point consider interval with idxZero in the center and idxInfl
	// as upper bound ..................................................
	size_t idxCurv = argMax(y2f, idxZero, idxInfl);
//	size_t idxCurv = argMax(y2f, 32, idxInfl);

//	Int_t idxCurv = 32;
//	Double_t max = y2[idxCurv];
//	for(Int_t i=idxCurv; i<idxInfl; i++) {
//		if (y2[i] > max) {
//			max = y2[i];
//			idxCurv = i;
//		}
//	}

	// final point where the rising edge starts. The point of highest
	// curvature seems to be the best option ...........................
	Int_t idxRise = idxCurv;

	return tf[idxRise];
}


////////////////////////////////////////////////////////////////////////////////
/// Calling function.
/// Evaluates where the rising edge of a channel signal starts.
/// \param[in] ch The input channel.
/// \return The time when the signal starts.
Double_t XboxAnalyserEvalRisingEdge::operator () (XBOX::XboxDAQChannel &ch) {

	ch.flushbuffer();
	XBOX::XboxDAQChannel chnew = ch;

	// avoid data re-interpretation
	ch.setAutoRefresh(false);

	// precise evaluation of the rising edge
	Double_t tr = evalRisingEdge(ch);

	if (fReportFlag)
		report(ch);

	// reset auto interpretation
	ch.setAutoRefresh(true);

	return tr;
}




////////////////////////////////////////////////////////////////////////
/// Report.
/// \param[in] ch The evaluated Xbox channel.
void XboxAnalyserEvalRisingEdge::report (XBOX::XboxDAQChannel ch) {

	// first rough calculation of the rising edge......................
	Double_t lbnd=0.; ///<Lower time axis bound.
	Double_t ubnd=0.; ///<Upper time axis bound.
	ch.getTimeAxisBounds(lbnd, ubnd);
	Double_t tmin = fWmin * (ubnd - lbnd) + lbnd; ///<Lower limit.
	Double_t tmax = fWmax * (ubnd - lbnd) + lbnd; ///<Upper limit.
	Double_t tref = ch.risingEdge(fTh, tmin, tmax);

	std::vector<Double_t> tc = ch.getTimeAxis(tmin, tmax);
	std::vector<Double_t> yc = ch.getSignal(tmin, tmax);

	// refine resolution around the rising edge and apply filters .....
	tmin = tref - 0.5 * fProximity * (ubnd - lbnd);
	tmax = tref + 0.5 * fProximity * (ubnd - lbnd);
	if (tmin < lbnd)
		tmin = lbnd;
	if (tmax > ubnd)
		tmax = ubnd;

	std::vector<Double_t> tf = XBOX::linspace(tmin, tmax, fSamplesRefine);
	Double_t dt = (tmax - tmin) / (fSamplesRefine - 1);

	std::vector<Double_t> yf = fFilterSig(ch, tf); // normal signal
	std::vector<Double_t> y1f = fFilterD1(ch, tf); // first derivative
	std::vector<Double_t> y2f = fFilterD2(ch, tf); // second derivative

	// find closest maximum to the right ..............................
	size_t idxMax = argRightRoot(y1f, fSamplesRefine/2);

	// find inflection point of the rising edge .......................
	size_t idxInfl = argLeftRoot(y2f, idxMax);

	// cross the tangent in the inflection point with zero ground ......
	Double_t slope = std::accumulate(y1f.begin() + idxInfl - fWindowSize,
			y1f.begin() + idxInfl + fWindowSize, 0.) / (2*fWindowSize + 1);

	size_t idxShift = yf[idxInfl] / (slope * dt);
	size_t idxZero = 0;
	if (idxInfl > idxShift)
		idxZero = idxInfl - idxShift;

	// point of maximum curvature between zero crossing and inflection
	size_t idxCurv = argMax(y2f, idxZero, idxInfl);
	Double_t tr = tf[idxCurv];

	// printed report .................................................
	printf("----------------------------------------------------\n");
	printf("Report EvalRisingEdge\n");
	printf("----------------------------------------------------\n");
	printf("fXboxVersion           : %u\n", ch.getXboxVersion());
	printf("fChannelName           : %s\n", ch.getChannelName().c_str());

	printf("fTimestamp             : %s\n", ch.getTimeStamp().AsString("s"));
	printf("fPulseCount            : %llu\n", ch.getPulseCount());

	printf("fStartTime             : %s\n", ch.getStartTime().AsString("s"));
	printf("fStartOffset           : %e\n", ch.getStartOffset());
	printf("fIncrement             : %e\n", ch.getIncrement());
	printf("fNSamples              : %u\n", ch.getSamples());

	printf("idxInfl                : %llu\n", idxInfl);
	printf("idxShift               : %llu\n", idxShift);
	printf("idxZero                : %llu\n", idxZero);
	printf("idxCurv                : %llu\n", idxCurv);

	printf("tref                   : %e\n", tref);
	printf("tmax                   : %e\n", tf[idxMax]);
	printf("tinfl                  : %e\n", tf[idxInfl]);
	printf("slope                  : %e\n", slope);
	printf("trise                  : %e\n", tr);
	printf("----------------------------------------------------\n");

	// plots .........................................................
	std::string title = "EvalRisingEdge " + ch.getChannelName()
			+ " " + ch.getTimeStamp().AsString("s");


	Double_t ysmin = 0.5*ch.min(tmin, tmax);
	Double_t ysmax = 0.5*ch.max(tmin, tmax);
	std::vector<Double_t> y1fs = rescale(y1f, ysmin, ysmax);
	std::vector<Double_t> y2fs = rescale(y2f, ysmin, ysmax);

	TCanvas c1("c1",title.c_str(),700, 500);
	c1.SetGrid();

	TGraph gr1(tc.size(), &tc[0], &yc[0]);
	gr1.SetLineColor(kGray);
	gr1.SetMarkerColor(kGray);

	TGraph gr2(tf.size(), &tf[0], &yf[0]);
	gr2.SetLineColor(kBlack);
	gr2.SetMarkerColor(kBlack);

	TGraph gr3(tf.size(), &tf[0], &y1fs[0]);
	gr3.SetLineColor(kGreen+2);
	gr3.SetMarkerColor(kGreen+2);

	TGraph gr4(tf.size(), &tf[0], &y2fs[0]);
	gr4.SetLineColor(kOrange+7);
	gr4.SetMarkerColor(kOrange+7);

	TMultiGraph *mg = new TMultiGraph();
	mg->Add(&gr1);
	mg->Add(&gr2);
	mg->Add(&gr3);
	mg->Add(&gr4);
	mg->SetTitle(title.c_str());
	mg->Draw("AC");

	TAxis *ax = mg->GetXaxis();
	TAxis *ay = mg->GetYaxis();

	ax->SetTitle("Time [s]");
	ay->SetTitle("Amplitude");

	TLine *l1 = new TLine(tref, ay->GetXmin(), tref, ay->GetXmax());
	l1->SetLineWidth(1);
	l1->SetLineColor(kBlack);
	l1->Draw();
	l1->SetNDC(false);

	TLine *l2 = new TLine(tf[idxMax], ay->GetXmin(), tf[idxMax], ay->GetXmax());
	l2->SetLineWidth(1);
	l2->SetLineColor(kGreen+2);
	l2->Draw();
	l2->SetNDC(false);

	TLine *l3 = new TLine(tf[idxInfl], ay->GetXmin(), tf[idxInfl], ay->GetXmax());
	l3->SetLineWidth(1);
	l3->SetLineColor(kOrange+7);
	l3->Draw();
	l3->SetNDC(false);

	TLine *l4 = new TLine(tr, ay->GetXmin(), tr, ay->GetXmax());
	l4->SetLineWidth(1);
	l4->SetLineColor(kRed);
	l4->Draw();
	l4->SetNDC(false);

	auto legend = new TLegend(0.11,0.7,0.4,0.89);
	legend->AddEntry(&gr2, "y", "l");
	legend->AddEntry(&gr3, "y' (scaled)", "l");
	legend->AddEntry(&gr4, "y'' (scaled)", "l");
	legend->Draw();

	c1.Draw();
	c1.Print((fReportDir + title + ".png").c_str());

	// zoom in
	ax->SetRangeUser(tmin, tmax);
	c1.Print((fReportDir + title + " zoom.png").c_str());

	delete l1;
	delete l2;
	delete l3;
	delete l4;
	delete mg;
	delete legend;
}


#ifndef XBOX_NO_NAMESPACE
}
#endif



