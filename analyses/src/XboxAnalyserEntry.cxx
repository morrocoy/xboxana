/*
 * XboxAnalyserEntry.cxx
 *
 *  Created on: Oct 31, 2018
 *      Author: kpapke
 */

#include "XboxAnalyserEntry.hxx"

#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif

////////////////////////////////////////////////////////////////////////
/// Constructor.
XboxAnalyserEntry::XboxAnalyserEntry() {
	init();
}

////////////////////////////////////////////////////////////////////////
/// Destructor.
XboxAnalyserEntry::~XboxAnalyserEntry() {
	clear();
}

////////////////////////////////////////////////////////////////////////
/// Default Settings.
void XboxAnalyserEntry::init() {
	fXboxVersion = 0;
	fTimeStamp.Set(0, 0, 0, 0, 0, 0, 0, true, 0);
	fLogType = -1;
	fPulseCount = 0;
	fDeltaF = 0;
	fLine = 0;
	fBreakdownFlag = false;

	fPulseRisingEdge = -1.;
	fPulseFallingEdge = -1.;
	fPulseLength = -1.;
	fPulsePowerMin = -1;
	fPulsePowerMax = -1.;
	fPulsePowerAvg = -1.;

	fPeakPeakValue = -1.;

	fJitter = -1.;
	fTranRisingEdge = -1.;
	fTranBreakdownTime = -1.;
	fReflRisingEdge = -1.;
	fReflBreakdownTime = -1.;
	fBreakdownTime = -1.;

}

////////////////////////////////////////////////////////////////////////
/// Clear.
/// Subroutine to destruct this object.
void XboxAnalyserEntry::clear() {
}

////////////////////////////////////////////////////////////////////////
/// Reset.
void XboxAnalyserEntry::reset() {
	clear();
	init();
}

////////////////////////////////////////////////////////////////////////
/// Simple printing function.
void XboxAnalyserEntry::print() {

	printf("----------------------------------------------------\n");
	printf("fXboxVersion           : %u\n", fXboxVersion);

	printf("fTimestamp             : %s\n", fTimeStamp.AsString());
	printf("fLogType               : %d\n", fLogType);
	printf("fPulseCount            : %llu\n", fPulseCount);
	printf("fDeltaF                : %e\n", fDeltaF);
	printf("fLine                  : %u\n", fLine);
	printf("fBreakdownFlag         : %u\n", fBreakdownFlag);

	printf("fPulseRisingEdge      : %e\n", fPulseRisingEdge);
	printf("fPulseFallingEdge     : %e\n", fPulseFallingEdge);
	printf("fPulseLength          : %e\n", fPulseLength);
	printf("fPulsePowerMax        : %e\n", fPulsePowerMax);
	printf("fPulsePowerAvg        : %e\n", fPulsePowerAvg);
	printf("fJitter               : %e\n", fJitter);

	printf("fTranRisingEdge       : %e\n", fTranRisingEdge);
	printf("fTranBreakdownTime    : %e\n", fTranBreakdownTime);
	printf("fReflRisingEdge       : %e\n", fReflRisingEdge);
	printf("fReflBreakdownTime    : %e\n", fReflBreakdownTime);
	printf("fBreakdownTime        : %e\n", fBreakdownTime);
	printf("----------------------------------------------------\n");
}

////////////////////////////////////////////////////////////////////////
/// Comparison operator.
/// Smaller-Than-Operator refer to the time stamp.
/// \param[in] rhs Xbox DAQ channel for comparison.
/// \return    true if time stamp is smaller than of rhs.
Bool_t XboxAnalyserEntry::operator < (const XboxAnalyserEntry& rhs) const {

	return fTimeStamp < rhs.getTimeStamp();
}

////////////////////////////////////////////////////////////////////////
/// Comparison operator.
/// Larger-Than-Operator refer to the time stamp.
/// \param[in] rhs Xbox DAQ channel for comparison.
/// \return    true if time stamp is larger than of rhs.
Bool_t XboxAnalyserEntry::operator > (const XboxAnalyserEntry& rhs) const {

	return fTimeStamp > rhs.getTimeStamp();
}


#ifndef XBOX_NO_NAMESPACE
}
#endif
