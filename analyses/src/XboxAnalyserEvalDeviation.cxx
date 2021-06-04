#include "XboxAnalyserEvalDeviation.hxx"

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
XboxAnalyserEvalDeviation::XboxAnalyserEvalDeviation() {
	init();
}

////////////////////////////////////////////////////////////////////////
/// Constructor.
XboxAnalyserEvalDeviation::XboxAnalyserEvalDeviation(Double_t wmin,
		Double_t wmax, Double_t thc, Double_t thf, Double_t prox) {
	init();
	config(wmin, wmax, thc, thf, prox);
}


////////////////////////////////////////////////////////////////////////
/// Destructor.
XboxAnalyserEvalDeviation::~XboxAnalyserEvalDeviation() {
	clear();
}

////////////////////////////////////////////////////////////////////////
/// Default Settings.
void XboxAnalyserEvalDeviation::init() {

	fWmin = 0.01;
	fWmax = 0.99;
	fThCoarse = 0.1;
	fThRefine = 0.01;
	fProximity = 0.1;

	// Filter configurations
	fFilterSig.config(XBOX::XboxSignalFilter::kSavitzkyGolay, 3, 7, 7);
	fFilterDev.config(XBOX::XboxSignalFilter::kSavitzkyGolay, 3, 20, 20);

	fSamplesCoarse = 8196;
	fSamplesRefine = 1024;
	fWindowCoarse = 32;
	fWindowRefine = 8;

	fReportFlag = false;
	fReportDir = "";
}

////////////////////////////////////////////////////////////////////////
/// Clear.
void XboxAnalyserEvalDeviation::clear(){
}

////////////////////////////////////////////////////////////////////////
/// Reset.
void XboxAnalyserEvalDeviation::reset() {
	clear();
	init();
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
/// \param[in] thf The fine threshold measured relative to first signal.
/// \param[in] prox The proximity is a refined interval to analyse the
///            deflection point between both signals.
void XboxAnalyserEvalDeviation::config(Double_t wmin,
		Double_t wmax, Double_t thc, Double_t thf, Double_t prox) {

	fWmin = wmin;
	fWmax = wmax;
	fThCoarse = thc;
	fThRefine = thf;
	fProximity = prox;
}

////////////////////////////////////////////////////////////////////////
/// Absolute threshold.
/// Calculates the absolute threshold based on the relative value th
/// and a refererence signal y
/// \param[in] y The reference signal.
/// \param[in] th The relative threshold.
/// \return    The absolute threshold.
Double_t XboxAnalyserEvalDeviation::absTh(std::vector<Double_t> &y, Double_t th){

	if (y.size() < 3)
		return -1.;

	Double_t ymin = *std::min_element(y.begin(), y.end());
	Double_t ymax = *std::max_element(y.begin(), y.end());
	return th * (ymax - ymin) + ymin;
}

////////////////////////////////////////////////////////////////////////
/// Absolute threshold.
/// Calculates the absolute threshold based on the relative value
/// for the coarse threshold fThCoarse and a refererence signal y
/// \param[in] y The reference signal.
/// \return    The absolute threshold.
Double_t XboxAnalyserEvalDeviation::absThCoarse(std::vector<Double_t> &y){

	return absTh(y, fThCoarse);
}

////////////////////////////////////////////////////////////////////////
/// Absolute threshold.
/// Calculates the absolute threshold based on the relative value
/// for the coarse threshold fThCoarse and a refererence signal y
/// \param[in] y The reference signal.
/// \return    The absolute threshold.
Double_t XboxAnalyserEvalDeviation::absThRefine(std::vector<Double_t> &y){

	return absTh(y, fThRefine);
}

////////////////////////////////////////////////////////////////////////
/// Argument of threshold crossing.
/// Calculates argument (time) at which the threshold th is exceeded or
/// or undershoot the first time by the signal y.
/// \param[in] t The time axis or argument axis of the signal.
/// \param[in] y The signal.
/// \param[in] th The absolute threshold.
/// \return    The argument where the threshold is crossed for the first time.
Double_t XboxAnalyserEvalDeviation::argTh(
		std::vector<Double_t> &t, std::vector<Double_t> &y, Double_t th){

	if (t.size() < 3 || t.size() != y.size())
		return -1.;

	size_t n = t.size();
	Double_t tcross = -1.;

	if (y[0] < th) {

		for(UInt_t i=1; i<n; i++){
			if(y[i] > th){
				tcross = t[i] - Double_t((y[i] - th) / (y[i] - y[i-1])) * (t[i] - t[i-1]);
				break;
			}
		}
	}
	else {

		for(UInt_t i=1; i<n; i++){
			if(y[i] < th){
				tcross = t[i] - Double_t((y[i] - th) / (y[i] - y[i-1])) * (t[i] - t[i-1]);
				break;
			}
		}
	}
	return tcross;

}

////////////////////////////////////////////////////////////////////////
/// Argument of threshold crossing.
/// Calculates argument (time) at which the threshold th is exceeded or
/// or undershoot the first time by the signal y.
/// \param[in] t The time axis or argument axis of the signal.
/// \param[in] y The signal.
/// \param[in] th The absolute threshold.
/// \return    The argument where the threshold is crossed for the first time.
Double_t XboxAnalyserEvalDeviation::argThCoarse(
		std::vector<Double_t> &t, std::vector<Double_t> &y, Double_t th){

	if (t.size() < 3 || t.size() != y.size())
		return -1.;

	size_t n = t.size();
	Double_t tcross = -1.;

	for(size_t i=0; i<n-fWindowCoarse; i++){
		size_t nw = 0;
		for (size_t j=0; j<fWindowCoarse; j++){
			if(fabs(y[i+j]) > th)
				nw++;
		}
		if(nw == fWindowCoarse){
			tcross = t[i] - Double_t((y[i] - th) / (y[i] - y[i-1])) * (t[i] - t[i-1]);
			break;
		}
	}

	return tcross;

}

////////////////////////////////////////////////////////////////////////
/// Argument of threshold crossing.
/// Calculates argument (time) at which the threshold th is exceeded or
/// or undershoot the first time by the signal y.
/// \param[in] t The time axis or argument axis of the signal.
/// \param[in] y The signal.
/// \param[in] th The absolute threshold.
/// \return    The argument where the threshold is crossed for the first time.
Double_t XboxAnalyserEvalDeviation::argThRefine(
		std::vector<Double_t> &t, std::vector<Double_t> &y, Double_t th){

	if (t.size() < 3 || t.size() != y.size())
		return -1.;

	size_t n = t.size();
	Double_t tcross = -1.;

	size_t istart = n / 2;
	while (fabs(y[istart]) < th && istart < n)
		istart++;

	if (istart == n)
		return -1;

	for(size_t i=istart; i>fWindowRefine; i--){
		size_t nw = 0;
		for (size_t j=0; j<fWindowRefine; j++){
			if(fabs(y[i-j]) < th) // use y[i], abs, mean, or stdev
				nw++;
		}
		if(nw == fWindowRefine){
			tcross = t[i+1] - Double_t((fabs(y[i+1]) - th) / fabs(y[i+1] - y[i])) * (t[i+1] - t[i]);
			return tcross;
			break;
		}
	}

	return tcross;

}

////////////////////////////////////////////////////////////////////////
/// Maximum absolute value of signal.
/// \param[in] y The input signal.
/// \return    The maximum absolute value .
Double_t XboxAnalyserEvalDeviation::magn(std::vector<Double_t> &y) {

	size_t n = y.size();
	Double_t maxval = 0;
	for(size_t i=0; i<n; i++) {
		if(fabs(y[i]) > maxval)
			maxval = fabs(y[i]);
	}

	return maxval;
}


////////////////////////////////////////////////////////////////////////
/// Difference of two signals
/// \param[in] y1 The first input signal.
/// \param[in] y2 The first input signal.
/// \return    The difference signal.
std::vector<Double_t> XboxAnalyserEvalDeviation::diff(
		std::vector<Double_t> &y1, std::vector<Double_t> &y2) {

	if (y1.empty() || y1.size() != y2.size())
		return std::vector<Double_t>{};

	size_t n = y1.size();
	std::vector<Double_t> y(n, 0.);
	for(size_t i=0; i<n; i++)
		y[i] = y1[i] - y2[i];

	return y;
}

////////////////////////////////////////////////////////////////////////
/// Deviation between two signals.
/// Precise evaluation of moment when both signals start to deviate.
/// Assumes that both signals are of same type.
/// \param[in] ch0 the Xbox DAQ channel containing the current signal.
/// \return    time at the start of the rising edge.
Double_t XboxAnalyserEvalDeviation::evalDeviation(
		XBOX::XboxDAQChannel &ch1, XBOX::XboxDAQChannel &ch2) {

	// get the signal and apply filters (chose carefully the filter
	// parameters according to the signal distortion) ..................
	Double_t lbnd=0.; ///<Lower time axis bound.
	Double_t ubnd=0.; ///<Upper time axis bound.
	ch1.getTimeAxisBounds(lbnd, ubnd);
	Double_t tmin = fWmin * (ubnd - lbnd) + lbnd; ///<Lower limit.
	Double_t tmax = fWmax * (ubnd - lbnd) + lbnd; ///<Upper limit.

	std::vector<Double_t> t1 = ch1.getTimeAxis(tmin, tmax);
	std::vector<Double_t> y1 = ch1.getSignal(tmin, tmax);
	std::vector<Double_t> t2 = ch2.getTimeAxis(tmin, tmax);
	std::vector<Double_t> y2 = ch2.getSignal(tmin, tmax);

	std::vector<Double_t> tc = XBOX::linspace(tmin, tmax, fSamplesCoarse);
	std::vector<Double_t> y1c = fFilterSig(ch1, tc);
	std::vector<Double_t> y2c = fFilterSig(ch2, tc);
	std::vector<Double_t> yc = diff(y1c, y2c);

	// derive absolute threshold from maximum span of both signals .....
	Double_t y1pp = ch1.span(tmin, tmax);
	Double_t y2pp = ch2.span(tmin, tmax);
	Double_t thc = fThCoarse * max(y1pp, y2pp);

	// return if differential signal is within noise level .............
	if (thc > magn(yc)) {

		return -1.;
	}

	// First rough calculation of the deviation time based on threshold
	// which is defined as the maximum span of the signal peaks ........
	Double_t tref = argThCoarse(tc, yc, thc);

	// refine resolution around the deviation starting point ...........
	tmin = tref - 0.5 * fProximity * (ubnd - lbnd);
	tmax = tref + 0.5 * fProximity * (ubnd - lbnd);
	if (tmin < lbnd)
		tmin = lbnd;
	if (tmax > ubnd)
		tmax = ubnd;

	std::vector<Double_t> tf = XBOX::linspace(tmin, tmax, fSamplesRefine);
	std::vector<Double_t> y1f = fFilterSig(ch1, tf);
	std::vector<Double_t> y2f = fFilterSig(ch2, tf);
	std::vector<Double_t> yf = diff(y1f, y2f);
	yf = fFilterDev(yf); // Additional filter for the difference.

	// refine the deviation point .....................................
	Double_t thf = fThRefine * max(y1pp, y2pp);
	Double_t tdev = argThRefine(tf, yf, thf);

//	printf("tref                   : %e\n", tref);
//	printf("tdev                   : %e\n", tdev);

	return tdev;
}


/////////////////////////////////////////////////////////////////////////
/// Calling function.
/// Evaluates pulse shape parameter from a channel signal.
/// \param[in] ch1 The amplitude signal of the breakdown pulse.
/// \param[in] ch2 The amplitude signal of the previous pulse.
/// \return The time here ch1 start to deviate from ch2.
Double_t XboxAnalyserEvalDeviation::operator () (
			XBOX::XboxDAQChannel &ch1, XBOX::XboxDAQChannel &ch2, Double_t jitter) {

	if (jitter == -1)
		return -1;

	// avoid data re-interpretation
	ch1.setAutoRefresh(false);
	ch2.setAutoRefresh(false);
	ch1.flushbuffer();
	ch2.flushbuffer();

	// apply negative jitter to the previous pulse (reference) for evaluation
	Double_t offset = ch2.getStartOffset();
	ch2.setStartOffset(offset-jitter);

	// time when current signal starts to deviate from previous one
	Double_t tdev = evalDeviation(ch1, ch2);

	// reset auto interpretation
	ch1.setAutoRefresh(true);
	ch2.setAutoRefresh(true);

	if (fReportFlag)
		report(ch1, ch2, jitter);

	// reset offset of ch2 to the original value
	ch2.setStartOffset(offset);

	return tdev;
}


////////////////////////////////////////////////////////////////////////
/// Report.
/// \param[in] ch The evaluated Xbox channel.
void XboxAnalyserEvalDeviation::report (XBOX::XboxDAQChannel ch1,
		XBOX::XboxDAQChannel ch2, Double_t jitter) {

	// get the signal and apply filters (chose carefully the filter
	// parameters according to the signal distortion) ..................
	Double_t lbnd=0.; ///<Lower time axis bound.
	Double_t ubnd=0.; ///<Upper time axis bound.
	ch1.getTimeAxisBounds(lbnd, ubnd);
	Double_t tmin = fWmin * (ubnd - lbnd) + lbnd; ///<Lower limit.
	Double_t tmax = fWmax * (ubnd - lbnd) + lbnd; ///<Upper limit.

	std::vector<Double_t> t1 = ch1.getTimeAxis(tmin, tmax);
	std::vector<Double_t> y1 = ch1.getSignal(tmin, tmax);
	std::vector<Double_t> t2 = ch2.getTimeAxis(tmin, tmax);
	std::vector<Double_t> y2 = ch2.getSignal(tmin, tmax);

	std::vector<Double_t> tc = XBOX::linspace(tmin, tmax, fSamplesCoarse);
	std::vector<Double_t> y1c = fFilterSig(ch1, tc);
	std::vector<Double_t> y2c = fFilterSig(ch2, tc);
	std::vector<Double_t> yc = diff(y1c, y2c);

	// derive absolute threshold from maximum span of both signals .....
	Double_t y1pp = ch1.span(tmin, tmax);
	Double_t y2pp = ch2.span(tmin, tmax);
	Double_t thc = fThCoarse * max(y1pp, y2pp);

	// First rough calculation of the deviation time based on threshold
	// which is defined as the maximum span of the signal peaks ........
	Double_t tref = argThCoarse(tc, yc, thc);

	// refine resolution around the deviation starting point ...........
	tmin = tref - 0.5 * fProximity * (ubnd - lbnd);
	tmax = tref + 0.5 * fProximity * (ubnd - lbnd);
	if (tmin < lbnd)
		tmin = lbnd;
	if (tmax > ubnd)
		tmax = ubnd;

	std::vector<Double_t> tf = XBOX::linspace(tmin, tmax, fSamplesRefine);
	std::vector<Double_t> y1f = fFilterSig(ch1, tf);
	std::vector<Double_t> y2f = fFilterSig(ch2, tf);
	std::vector<Double_t> yf = diff(y1f, y2f);
	yf = fFilterDev(yf); // Additional filter for the difference.

	// refine the deviation point .....................................
	Double_t thf = fThRefine * max(y1pp, y2pp);
	Double_t tdev = argThRefine(tf, yf, thf);

	// printed report .................................................
	printf("----------------------------------------------------\n");
	printf("Report EvalDeviation\n");
	printf("----------------------------------------------------\n");
	printf("fXboxVersion           : %u\n", ch1.getXboxVersion());
	printf("fChannelName           : %s\n", ch1.getChannelName().c_str());

	printf("fTimestamp             : %s\n", ch1.getTimeStamp().AsString("s"));
	printf("fPulseCount            : %llu\n", ch1.getPulseCount());

	printf("fStartTime             : %s\n", ch1.getStartTime().AsString("s"));
	printf("fStartOffset           : %e\n", ch1.getStartOffset());
	printf("fIncrement             : %e\n", ch1.getIncrement());
	printf("fNSamples              : %u\n", ch1.getSamples());

	printf("jitter                 : %e\n", jitter);
	printf("thc                    : %e\n", thc);
	printf("thf                    : %e\n", thf);
	printf("tref                   : %e\n", tref);
	printf("tdev                   : %e\n", tdev);
	printf("----------------------------------------------------\n");

	// plots .........................................................
	std::string title = "EvalDeviation " + ch1.getChannelName()
			+ " " + ch1.getTimeStamp().AsString("s");

	TCanvas c1("c1",title.c_str(),700, 500);
	c1.SetGrid();

	TGraph gr1(t1.size(), &t1[0], &y1[0]);
	gr1.SetLineColor(kGray);
	gr1.SetMarkerColor(kGray);

	TGraph gr2(t2.size(), &t2[0], &y2[0]);
	gr2.SetLineColor(kGray);
	gr2.SetMarkerColor(kGray);

	TGraph gr3(tc.size(), &tc[0], &y1c[0]);
	gr3.SetLineColor(kBlue);
	gr3.SetMarkerColor(kBlue);

	TGraph gr4(tc.size(), &tc[0], &y2c[0]);
	gr4.SetLineColor(kBlack);
	gr4.SetMarkerColor(kBlack);

	TGraph gr5(tc.size(), &tc[0], &yc[0]);
	gr5.SetLineColor(kRed-3);
	gr5.SetMarkerColor(kRed-3);

	TGraph gr6(tf.size(), &tf[0], &yf[0]);
	gr6.SetLineColor(kGreen-3);
	gr6.SetMarkerColor(kGreen-3);

	TMultiGraph *mg = new TMultiGraph();
	mg->Add(&gr1);
	mg->Add(&gr2);
	mg->Add(&gr3);
	mg->Add(&gr4);
	mg->Add(&gr5);
	mg->Add(&gr6);
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

	TLine *l2 = new TLine(tdev, ay->GetXmin(), tdev, ay->GetXmax());
	l2->SetLineWidth(1);
	l2->SetLineColor(kBlue);
	l2->Draw();
	l2->SetNDC(false);

	auto legend = new TLegend(0.11,0.7,0.4,0.89);
	legend->AddEntry(&gr3, "Bdwn", "l");
	legend->AddEntry(&gr4, "Prev", "l");
	legend->AddEntry(&gr5, "Diff", "l");
	legend->Draw();

	c1.Draw();
	c1.Print((fReportDir + title + ".png").c_str());

	// zoom in
	ax->SetRangeUser(tmin, tmax);
	c1.Print((fReportDir + title + " zoom.png").c_str());

	delete l1;
	delete l2;
	delete mg;
	delete legend;
}




#ifndef XBOX_NO_NAMESPACE
}
#endif



