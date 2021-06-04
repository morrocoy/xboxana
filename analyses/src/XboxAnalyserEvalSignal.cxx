#include "XboxAnalyserEvalSignal.hxx"

#include <iostream>

// root
#include "TTimeStamp.h"


#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif


////////////////////////////////////////////////////////////////////////
/// Constructor.
XboxAnalyserEvalSignal::XboxAnalyserEvalSignal() {
	init();
}

//////////////////////////////////////////////////////////////////////////
///// Constructor.
//XboxAnalyserEvalSignal::XboxAnalyserEvalSignal() {
//	init();
//	config();
//}

////////////////////////////////////////////////////////////////////////
/// Destructor.
XboxAnalyserEvalSignal::~XboxAnalyserEvalSignal() {
	clear();
}

////////////////////////////////////////////////////////////////////////
/// Default Settings.
void XboxAnalyserEvalSignal::init() {

}

////////////////////////////////////////////////////////////////////////
/// Clear.
void XboxAnalyserEvalSignal::clear(){
}

////////////////////////////////////////////////////////////////////////
/// Reset.
void XboxAnalyserEvalSignal::reset() {
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
void XboxAnalyserEvalSignal::config() {

}


////////////////////////////////////////////////////////////////////////
/// Calling function.
/// Evaluates pulse shape parameter from a channel signal.
/// \param[in] ch The Xbox channel.
/// \return The pulse parameters (pulse length, average power, ...)
XBOX::XboxDAQChannel XboxAnalyserEvalSignal::operator () (
		XBOX::XboxDAQChannel &ch, Double_t xmin, Double_t xmax) {

	ch.flushbuffer();
	XBOX::XboxDAQChannel chnew = ch;

	// avoid data re-interpretation
	chnew.setAutoRefresh(false);

	Double_t lbnd=0.;
	Double_t ubnd=0.;
	ch.getTimeAxisBounds(lbnd, ubnd);

	if (xmin == -1)
		xmin = lbnd;
	if (xmax == -1)
		xmax = ubnd;

	Double_t min = chnew.min(xmin, xmax);
	Double_t max = chnew.max(xmin, xmax);
	Double_t mean = chnew.mean(xmin, xmax);
	Double_t integ = chnew.integ(xmin, xmax);
	Double_t span = chnew.span(xmin, xmax);

	chnew.setXmin(xmin);
	chnew.setXmax(xmax);
	chnew.setYmin(min);
	chnew.setYmax(max);
	chnew.setYmean(mean);
	chnew.setYinteg(integ);
	chnew.setYspan(span);

	// reset auto interpretation
	chnew.setAutoRefresh(true);

	return chnew;
}


#ifndef XBOX_NO_NAMESPACE
}
#endif



