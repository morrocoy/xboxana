#include "XboxAnalyserEvalBreakdown.hxx"

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
XboxAnalyserEvalBreakdown::XboxAnalyserEvalBreakdown() {
	init();
}

////////////////////////////////////////////////////////////////////////
/// Destructor.
XboxAnalyserEvalBreakdown::~XboxAnalyserEvalBreakdown() {
	clear();
}

////////////////////////////////////////////////////////////////////////
/// Default Settings.
void XboxAnalyserEvalBreakdown::init() {

	fPulseWMin = 0.01;
	fPulseWMax = 0.99;
	fPulseTh = 0.9;

	fJitterWMin = 0.01;
	fJitterWMax = 0.4;
	fJitterTh = 0.3;
	fJitterMax = 0.4;

	fRiseWMin = 0.01;
	fRiseWMax = 0.99;
	fRiseTh = 0.6;
	fRiseProx = 0.03;

	fDeflWMin = 0.01;
	fDeflWMax = 0.99;
	fDeflThC = 0.1;
	fDeflThF = 0.01;
	fDeflProx = 0.1;
}

////////////////////////////////////////////////////////////////////////
/// Clear.
void XboxAnalyserEvalBreakdown::clear(){
}

////////////////////////////////////////////////////////////////////////
/// Reset.
void XboxAnalyserEvalBreakdown::reset() {
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
void XboxAnalyserEvalBreakdown::setPulseConfig(Double_t wmin, Double_t wmax, Double_t th) {

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
void XboxAnalyserEvalBreakdown::setJitterConfig(
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
void XboxAnalyserEvalBreakdown::setRiseConfig(
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
void XboxAnalyserEvalBreakdown::setDeflConfig(Double_t wmin,
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
Double_t XboxAnalyserEvalBreakdown::evalJitter(
		XBOX::XboxDAQChannel &ch0, XBOX::XboxDAQChannel &ch1){

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
Double_t XboxAnalyserEvalBreakdown::evalRisingEdge(
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

	// cross the tangent in the inflection point with zero ground ......
	Double_t slope = std::accumulate(y1.begin() + idxInfl - nwin,
			y1.begin() + idxInfl + nwin, 0.) / (2*nwin + 1);
	Int_t idxZero = idxInfl - Int_t(y0[idxInfl] / slope / dt);

	// point of maximum curvature between zero crossing and inflection
	// point consider interval with idxZero in the center and idxInfl
	// as upper bound ..................................................
	Int_t idxCurv = 32;
	Double_t max = y2[idxCurv];
	for(Int_t i=idxCurv; i<idxInfl; i++) {
		if (y2[i] > max) {
			max = y2[i];
			idxCurv = i;
		}
	}

	// final point where the rising edge starts. The point of highest
	// curvature seems to be the best option ...........................
	Int_t idxRise = idxCurv;

	// reset auto interpretation .......................................
	ch.setAutoRefresh(true);

	return t[idxRise];
}


////////////////////////////////////////////////////////////////////////
/// Deviation between two signals.
/// Precise evaluation of moment when both signals start to deviate.
/// Assumes that both signals are of same type.
/// \param[in] ch0 the Xbox DAQ channel containing the current signal.
/// \return    time at the start of the rising edge.
Double_t XboxAnalyserEvalBreakdown::evalDeviation(
		XBOX::XboxDAQChannel &ch0, XBOX::XboxDAQChannel &ch1) {

	const Int_t nsamples = 1024; ///<Number of interpolation points.
	const Int_t nwin = 32; ///<Number of points for moving window.

	Double_t lbnd=0.; ///<Lower time axis bound.
	Double_t ubnd=0.; ///<Upper time axis bound.
	Double_t tmin; ///<Lower time axis limit.
	Double_t tmax; ///<Upper time axis limit.
	std::vector<Double_t> t; ///<Time axis.

	std::vector<Double_t> ych0; ///<Signal of channel 0.
	std::vector<Double_t> ych1; ///<Signal of channel 1.
	std::vector<Double_t> y; ///<Differential signal.

	Double_t y0max; ///<Maximum of Signal of channel 0 in considered range.
	Double_t y1max; ///<Maximum of Signal of channel 0 in considered range.

	Double_t th; // absolute threshold
	Bool_t bnoise = false; // is signal within noise level

	///<Filter configurations ..........................................
	XBOX::XboxSignalFilter filtn0(XBOX::XboxSignalFilter::kSavitzkyGolay, 3, 7, 7);
	XBOX::XboxSignalFilter filtn1(XBOX::XboxSignalFilter::kSavitzkyGolay, 3, 20, 20);

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

	t = XBOX::linspace(tmin, tmax, 8*nsamples);

	ych0 = filtn0(ch0, t);
	ych1 = filtn0(ch1, t);

	// get difference between both signal and determine the maximum ....
	y.resize(t.size());
	for(size_t i=0; i<t.size(); i++)
		y[i] = ych0[i] - ych1[i];

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
		return -1.;
	}

	// get index where the threshold is exceeded the first time ........
	Int_t idxDefl = 0;
	for(UInt_t i=0; i<t.size()-nwin; i++){
		Int_t ncnt = 0;
		for (UInt_t j=0; j<nwin; j++){
			if(fabs(y[i+j]) > th)
				ncnt++;
		}
		if(ncnt == nwin){
			idxDefl = i;
			break;
		}
	}

	// refine resolution around the deflection .........................
	Double_t tref = t[idxDefl];
	ch0.flushbuffer();
	ch1.flushbuffer();

	tmin = tref - 0.5 * fDeflProx * (ubnd - lbnd);
	tmax = tref + 0.5 * fDeflProx * (ubnd - lbnd);

	if (tmin < lbnd)
		tmin = lbnd;
	if (tmax > ubnd)
		tmax = ubnd;

	t = XBOX::linspace(tmin, tmax, nsamples);

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
	for(Int_t i=nsamples/2; i > nwin; i--) {

		if (fabs(y[i]) < th) { // use y[i], mean, or stdev
			idxDefl = i;
			break;
		}
	}

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
XBOX::XboxAnalyserEntry XboxAnalyserEvalBreakdown::operator_old (
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
	Double_t pmin = b1psi.min(tr, tf);
	Double_t pmax = b1psi.max(tr, tf);
	Double_t pavg = b1psi.mean(tr, tf);
	Double_t pp = b1psi.span();

	model.setPulseRisingEdge(tr);
	model.setPulseFallingEdge(tf);
	model.setPulseLength(tf - tr);

	model.setPulsePowerMin(pmin);
	model.setPulsePowerMax(pmax);
	model.setPulsePowerAvg(pavg);
	model.setPeakPeakValue(pp);

	// rising edges of b1pei and b1psr ................................
	Double_t risepei = evalRisingEdge(b1pei);
	Double_t risepsr = evalRisingEdge(b1psr);
	model.setTranRisingEdge(risepei);
	model.setReflRisingEdge(risepsr);

	// time of deflection for b0pei and b0psr from b0pei and b0pei ....
	Double_t deflpei = evalDeviation(b0pei, b1pei);
	Double_t deflpsr = evalDeviation(b0psr, b1psr);
	model.setTranBreakdownTime(deflpei);
	model.setReflBreakdownTime(deflpsr);

	// time related to the breakdown location .........................
	Double_t bdtime = 0.5 * (deflpsr - risepsr - deflpei + risepei);
	model.setBreakdownTime(bdtime);

	// for visual inspection ..........................................
	Double_t twin = 1e-7;
	Int_t nwin = 256;
	XBOX::XboxSignalFilter filt(XBOX::XboxSignalFilter::kSavitzkyGolay, 3, 7, 7);

	b0psi.getTimeAxisBounds(lbnd, ubnd);

	std::vector<Double_t> yrise_pei(nwin, 0.);
	std::vector<Double_t> yrise_psr(nwin, 0.);
	std::vector<Double_t> ydefl_pei(nwin, 0.);
	std::vector<Double_t> ydefl_psr(nwin, 0.);

//	std::vector<Double_t> yrise_pei;
//	std::vector<Double_t> yrise_psr;
//	std::vector<Double_t> ydefl_pei;
//	std::vector<Double_t> ydefl_psr;


//	b1pei.flushbuffer();
//	if ((risepei > (lbnd + twin)) && (risepei < (ubnd - twin)))
//		yrise_pei = filt(b1pei, risepei-twin, risepei+twin, nwin);
//
//	b1psr.flushbuffer();
//	if ((risepsr > (lbnd + twin)) && (risepsr < (ubnd - twin)))
//		yrise_psr = filt(b1psr, risepsr-twin, risepsr+twin, nwin);

//	b0pei.flushbuffer();
//	b1pei.flushbuffer();
//	if (deflpei > lbnd + twin && deflpei < ubnd - twin) {
//		std::vector<Double_t> ydefl_b0pei = filt(b0pei, deflpei-twin, deflpei+twin, nwin);
//		std::vector<Double_t> ydefl_b1pei = filt(b1pei, deflpei-twin, deflpei+twin, nwin);
//		ydefl_pei.resize(nwin);
//		for (Int_t i=0; i<nwin; i++)
//			ydefl_pei[i] = ydefl_b0pei[i] - ydefl_b1pei[i];
//	}
//
//	b0psr.flushbuffer();
//	b1psr.flushbuffer();
//	if (deflpsr > lbnd + twin && deflpsr < ubnd - twin) {
//		std::vector<Double_t> ydefl_b0psr = filt(b0psr, deflpsr-twin, deflpsr+twin, nwin);
//		std::vector<Double_t> ydefl_b1psr = filt(b1psr, deflpsr-twin, deflpsr+twin, nwin);
//		ydefl_psr.resize(nwin);
//		for (Int_t i=0; i<nwin; i++)
//			ydefl_psr[i] = ydefl_b0psr[i] - ydefl_b1psr[i];
//	}

//	model.setProbeTranRisingEdge(yrise_pei);
//	model.setProbeTranBreakdownTime(ydefl_pei);
//	model.setProbeReflRisingEdge(yrise_psr);
//	model.setProbeReflBreakdownTime(ydefl_psr);

	return model;
}


////////////////////////////////////////////////////////////////////////////////
/// Calling function.
/// Evaluates pulse shape parameter from a channel signal.
/// \param[in] chMagn The amplitude signal of the breakdown pulse.
/// \param[in] chMagnRef The amplitude signal of the previous pulse.
/// \param[in] chJitter The amplitude signal to evaluate the jitter.
/// \param[in] chJitterRef The amplitude signal of the previous pulse to
///            evaluate the jitter.
/// \return The pulse and breakdown parameters.
XBOX::XboxAnalyserResult XboxAnalyserEvalBreakdown::operator () (
			XBOX::XboxDAQChannel &chMagn, XBOX::XboxDAQChannel &chMagnRef,
			XBOX::XboxDAQChannel &chJitter, XBOX::XboxDAQChannel &chJitterRef) {

	// put the results in the model container to return .......................
	XBOX::XboxAnalyserResult container;
	container.setXboxVersion(chMagn.getXboxVersion());
	container.setTimeStamp(chMagn.getTimeStamp());

	container.setLogType(chMagn.getLogType());
	container.setPulseCount(chMagn.getPulseCount());
	container.setDeltaF(chMagn.getDeltaF());
	container.setLine(chMagn.getLine());
	container.setBreakdownFlag(chMagn.getBreakdownFlag());

	// jitter based on chJitter and chJitterRef ...............................
	chJitterRef.setStartOffset(0.);
	chMagnRef.setStartOffset(0.);
	Double_t jitter = evalJitter(chJitter, chJitterRef);
	if (jitter == -1)
		return container; // return current data set

	container.setJitter(jitter);
	chJitterRef.setStartOffset(-jitter);
	chMagnRef.setStartOffset(-jitter);

	// pulse shape based of previous signal (chMagnRef) .......................
	Double_t lbnd=0.;
	Double_t ubnd=0.;

	chMagnRef.getTimeAxisBounds(lbnd, ubnd);
	Double_t tmin = fPulseWMin * (ubnd - lbnd) + lbnd;
	Double_t tmax = fPulseWMax * (ubnd - lbnd) + lbnd;

	Double_t tr = evalRisingEdge(chMagnRef); // precise evaluation of the rising edge
	Double_t tf = chMagnRef.fallingEdge(fPulseTh, tmin, tmax); // normal evaluation of the falling edge

	Double_t pmin = chMagnRef.min(tr, tf);
	Double_t pmax = chMagnRef.max(tr, tf);
	Double_t pavg = chMagnRef.mean(tr, tf);
	Double_t pint = chMagnRef.integ(tr, tf);
	Double_t ppp = chMagnRef.span();

	container.setRefXmin(tr);
	container.setRefXmax(tf);
	container.setRefYmin(pmin);
	container.setRefYmax(pmax);
	container.setRefYavg(pavg);
	container.setRefYint(pint);
	container.setRefYpp(ppp);

	// pulse shape of the current signal (chMagn) .............................
	chMagn.getTimeAxisBounds(lbnd, ubnd);
	tmin = fPulseWMin * (ubnd - lbnd) + lbnd;
	tmax = fPulseWMax * (ubnd - lbnd) + lbnd;

	tr = chMagn.risingEdge(fPulseTh, tmin, tmax); // normal evaluation of the rising edge
	tf = chMagn.fallingEdge(fPulseTh, tmin, tmax); // normal evaluation of the falling edge

	pmin = chMagn.min(tr, tf);
	pmax = chMagn.max(tr, tf);
	pavg = chMagn.mean(tr, tf);
	pint = chMagn.integ(tr, tf);
	ppp = chMagn.span();

	container.setXmin(tr);
	container.setXmax(tf);
	container.setYmin(pmin);
	container.setYmax(pmax);
	container.setYavg(pavg);
	container.setYint(pint);
	container.setYpp(ppp);

	// find the time where the current primary signal starts to deviate
	// from the previous one ..................................................
	Double_t tdev = evalDeviation(chMagn, chMagnRef);
	container.setXdev(tdev);


	// for visual inspection ..................................................
//	Double_t twin = 1e-7;
//	Int_t nwin = 256;
//	XBOX::XboxSignalFilter filt(
//			0, XBOX::XboxSignalFilter::kSavitzkyGolay, 3, 7, 7);
//
//	std::vector<Double_t> yrise(nwin, 0.);
//	std::vector<Double_t> ydev(nwin, 0.);;
//
//	prevMag.flushbuffer();
//	if ((tr > (lbnd + twin)) && (tr < (ubnd - twin))) {
//		yrise = filt(prevMag, tr-twin, tr+twin, nwin);
//	}
//
//	prevMag.flushbuffer();
//	currMag.flushbuffer();
//	if (tdev > lbnd + twin && tdev < ubnd - twin) {
//		std::vector<Double_t> curr = filt(currMag, tdev-twin, tdev+twin, nwin);
//		std::vector<Double_t> prev = filt(prevMag, tdev-twin, tdev+twin, nwin);
//		for (Int_t i=0; i<nwin; i++)
//			ydev[i] = curr[i] - prev[i];
//	}
//
//	container.setTestingBuffer1(yrise);
//	container.setTestingBuffer2(ydev);

	return container;
}



#ifndef XBOX_NO_NAMESPACE
}
#endif



