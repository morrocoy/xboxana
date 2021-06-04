#include "XboxAnalyserEvalJitter.hxx"

#include <iostream>
#include <numeric>

// root
#include "TTimeStamp.h"
#include "TH1D.h"
#include "TMatrixD.h"
#include "TVectorD.h"
#include "TDecompSVD.h"

// root graphics
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
XboxAnalyserEvalJitter::XboxAnalyserEvalJitter() {
	init();
}

////////////////////////////////////////////////////////////////////////
/// Constructor.
XboxAnalyserEvalJitter::XboxAnalyserEvalJitter(Double_t wmin,
		Double_t wmax, Double_t th, Double_t max) {
	init();
	config(wmin, wmax, th, max);
}

////////////////////////////////////////////////////////////////////////
/// Destructor.
XboxAnalyserEvalJitter::~XboxAnalyserEvalJitter() {
	clear();
}

////////////////////////////////////////////////////////////////////////
/// Default Settings.
void XboxAnalyserEvalJitter::init() {

	fWmin = 0.01;
	fWmax = 0.4;
	fTh = 0.3;
	fTol = 0.4;

	// Filter configurations
	fFilter.config(XBOX::XboxSignalFilter::kSavitzkyGolay, 3, 7, 7);

	// samples for interpolation
	fSamples = 10001;

	fReportFlag = false;
	fReportDir = "";
}

////////////////////////////////////////////////////////////////////////
/// Clear.
void XboxAnalyserEvalJitter::clear(){
}

////////////////////////////////////////////////////////////////////////
/// Reset.
void XboxAnalyserEvalJitter::reset() {
	clear();
	init();
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
void XboxAnalyserEvalJitter::config(
		Double_t wmin, Double_t wmax, Double_t th, Double_t max) {

	fWmin = wmin;
	fWmax = wmax;
	fTh = th;
	fTol = max;
}

////////////////////////////////////////////////////////////////////////
/// Absolute threshold.
/// Calculates the absolute threshold based on the relative value fTh
/// and a refererence signal y
/// \param[in] y The reference signal.
/// \return    The absolute threshold.
Double_t XboxAnalyserEvalJitter::absTh(std::vector<Double_t> &y){

	if (y.size() < 3)
		return -1.;

	Double_t ymin = *std::min_element(y.begin(), y.end());
	Double_t ymax = *std::max_element(y.begin(), y.end());
	return fTh * (ymax - ymin) + ymin;
}

////////////////////////////////////////////////////////////////////////
/// Argument of threshold crossing.
/// Calculates argument (time) at which the threshold th is exceeded or
/// or undershoot the first time by the signal y.
/// \param[in] t The time axis or argument axis of the signal.
/// \param[in] y The signal.
/// \param[in] th The absolute threshold.
/// \return    The argument where the threshold is crossed for the first time.
Double_t XboxAnalyserEvalJitter::argTh(
		std::vector<Double_t> &t, std::vector<Double_t> &y, Double_t th){

	if (t.size() < 3 || t.size() != y.size())
		return -1.;

	Double_t tcross = -1.;

	if (y[0] < th) {

		for(UInt_t i=1; i<fSamples; i++){
			if(y[i] > th){
				tcross = t[i] - Double_t((y[i] - th) / (y[i] - y[i-1])) * (t[i] - t[i-1]);
				break;
			}
		}
	}
	else {

		for(UInt_t i=1; i<fSamples; i++){
			if(y[i] < th){
				tcross = t[i] - Double_t((y[i] - th) / (y[i] - y[i-1])) * (t[i] - t[i-1]);
				break;
			}
		}
	}
	return tcross;

}

////////////////////////////////////////////////////////////////////////
/// Signal jitter evaluation.
/// Evaluates the jitter between two measurements based on a threshold.
/// \param[in] ch1 the Xbox DAQ channel containing the first signal.
/// \param[in] ch2 the Xbox DAQ channel containing the second signal.
/// \return The time delay between the two signals.
Double_t XboxAnalyserEvalJitter::evalJitter(
		XBOX::XboxDAQChannel &ch1, XBOX::XboxDAQChannel &ch2){

	Double_t lbnd=0.; ///<Lower time axis bound.
	Double_t ubnd=0.; ///<Upper time axis bound.
	Double_t tmin; ///<Lower time axis limit.
	Double_t tmax; ///<Upper time axis limit.

	// get the signal and apply filters (chose carefully the filter
	// parameters according to the signal distortion) ..................
	ch1.getTimeAxisBounds(lbnd, ubnd);
	tmin = fWmin * (ubnd - lbnd) + lbnd;
	tmax = fWmax * (ubnd - lbnd) + lbnd;

	std::vector<Double_t> t = XBOX::linspace(tmin, tmax, fSamples);
	std::vector<Double_t> y1 = fFilter(ch1, t);
	std::vector<Double_t> y2 = fFilter(ch2, t);

	// determine the delay between the signals based on threshold ......
	Double_t th = absTh(y1);
	Double_t t1cross = argTh(t, y1, th);
	Double_t t2cross = argTh(t, y2, th);
	Double_t delay = t2cross - t1cross;

	// debug
//	printf("ubnd: %e | lbnd: %e\n", lbnd, ubnd);
//	printf("xmin: %e | xmax: %e | th: %f\n", fJitterWMin, fJitterWMax, fJitterTh);
//	printf("tmin: %e | tmax: %e | th: %f\n", tmin, tmax, th);
//	printf("t0: %e | t1: %e\n", t[idx0], t[idx1]);
//	printf("delay: %e\n", delay);

	// check whether delay is below a maximum acceptable value ........
	if (fabs(delay) > fTol * (ubnd - lbnd))
		delay = -1.;

	return delay;
}


////////////////////////////////////////////////////////////////////////////////
/// Calling function.
/// Evaluates jitter between two pulses.
/// \param[in] ch1 The amplitude signal of the breakdown pulse.
/// \param[in] ch2The amplitude signal of the previous pulse.
/// \return The jitter.
Double_t XboxAnalyserEvalJitter::operator () (
			XBOX::XboxDAQChannel &ch1, XBOX::XboxDAQChannel &ch2) {

	ch1.flushbuffer();
	ch2.flushbuffer();

	// avoid data re-interpretation
	ch1.setAutoRefresh(false);
	ch2.setAutoRefresh(false);

	Double_t jitter =  evalJitter(ch1, ch2);

	// reset auto interpretation
	ch1.setAutoRefresh(true);
	ch2.setAutoRefresh(true);

	if (fReportFlag)
		report(ch1, ch2);

	return jitter;
}

////////////////////////////////////////////////////////////////////////
/// Report.
/// \param[in] ch The evaluated Xbox channel.
void XboxAnalyserEvalJitter::report (
		XBOX::XboxDAQChannel ch1, XBOX::XboxDAQChannel ch2) {

	Double_t lbnd=0.; ///<Lower time axis bound.
	Double_t ubnd=0.; ///<Upper time axis bound.
	Double_t tmin; ///<Lower time axis limit.
	Double_t tmax; ///<Upper time axis limit.

	// get time window
	ch1.getTimeAxisBounds(lbnd, ubnd);
	tmin = fWmin * (ubnd - lbnd) + lbnd;
	tmax = fWmax * (ubnd - lbnd) + lbnd;

	// get signals
	std::vector<Double_t> t1 = ch1.getTimeAxis(tmin, tmax);
	std::vector<Double_t> y1 = ch1.getSignal(tmin, tmax);
	std::vector<Double_t> t2 = ch2.getTimeAxis(tmin, tmax);
	std::vector<Double_t> y2 = ch2.getSignal(tmin, tmax);

	// get filtered signals
	std::vector<Double_t> tf = XBOX::linspace(tmin, tmax, fSamples);
	std::vector<Double_t> y1f = fFilter(ch1, tf);
	std::vector<Double_t> y2f = fFilter(ch2, tf);

	// determine the delay between the signals based on threshold
	Double_t th = absTh(y1f);
	Double_t t1cross = argTh(tf, y1f, th);
	Double_t t2cross = argTh(tf, y2f, th);
	Double_t delay = t2cross - t1cross;

	// printed report
	printf("----------------------------------------------------\n");
	printf("Report EvalJitter\n");
	printf("----------------------------------------------------\n");
	printf("fXboxVersion           : %u\n", ch1.getXboxVersion());
	printf("fChannelName           : %s\n", ch1.getChannelName().c_str());

	printf("fTimestamp             : %s\n", ch1.getTimeStamp().AsString("s"));
	printf("fPulseCount            : %llu\n", ch1.getPulseCount());

	printf("fStartTime             : %s\n", ch1.getStartTime().AsString("s"));
	printf("fStartOffset           : %e\n", ch1.getStartOffset());
	printf("fIncrement             : %e\n", ch1.getIncrement());
	printf("fNSamples              : %u\n", ch1.getSamples());

	printf("t1cross                : %e\n", t1cross);
	printf("t2cross                : %e\n", t2cross);
	printf("Jitter                 : %e\n", delay);
	printf("----------------------------------------------------\n");

	// plots
	std::string title = "EvalJitter " + ch1.getChannelName()
			+ " " + ch1.getTimeStamp().AsString("s");

	TCanvas c1("c1",title.c_str(),700, 500);
	c1.SetGrid();

	TGraph gr1(t1.size(), &t1[0], &y1[0]);
	gr1.SetLineColor(kGray);
	gr1.SetMarkerColor(kGray);

	TGraph gr2(t2.size(), &t2[0], &y2[0]);
	gr2.SetLineColor(kGray);
	gr2.SetMarkerColor(kGray);

	TGraph gr3(tf.size(), &tf[0], &y1f[0]);
	gr3.SetLineColor(kBlack);
	gr3.SetMarkerColor(kBlack);

	TGraph gr4(tf.size(), &tf[0], &y2f[0]);
	gr4.SetLineColor(kBlue);
	gr4.SetMarkerColor(kBlue);

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

	// marker lines
	TLine *l1 = new TLine(t1cross, ay->GetXmin(), t1cross, ay->GetXmax());
	l1->SetLineWidth(1);
	l1->SetLineColor(kBlack);
	l1->Draw();
	l1->SetNDC(false);

	TLine *l2 = new TLine(t2cross, ay->GetXmin(), t2cross, ay->GetXmax());
	l2->SetLineWidth(1);
	l2->SetLineColor(kBlue);
	l2->Draw();
	l2->SetNDC(false);

	TLine *l3 = new TLine(ax->GetXmin(), th, ax->GetXmax(), th);
	l3->SetLineWidth(1);
	l3->SetLineColor(kBlack);
	l3->Draw();
	l3->SetNDC(false);

	// legend
	auto legend = new TLegend(0.11,0.7,0.4,0.89);
	legend->AddEntry(mg, ch1.getChannelName().c_str(), "l");
	legend->Draw();

	c1.Draw();
	c1.Print((fReportDir + title + ".png").c_str());

	// zoom in
	Double_t xmin = 0.5 * (t2cross + t1cross) - 5*fabs(t2cross - t1cross);
	Double_t xmax = 0.5 * (t2cross + t1cross) + 5*fabs(t2cross - t1cross);
	ax->SetRangeUser(xmin, xmax);
	l3->SetX1(xmin);
	l3->SetX2(xmax);
	c1.Print((fReportDir + title + " zoom.png").c_str());

	delete l1;
	delete l2;
	delete l3;
	delete mg;
	delete legend;
}


#ifndef XBOX_NO_NAMESPACE
}
#endif



