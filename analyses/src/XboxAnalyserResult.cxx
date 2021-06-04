/*
 * XboxAnalyserResult.cxx
 *
 *  Created on: Oct 31, 2018
 *      Author: kpapke
 */

#include "XboxAnalyserResult.hxx"

#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif

////////////////////////////////////////////////////////////////////////
/// Constructor.
XboxAnalyserResult::XboxAnalyserResult() {
	init();
}

////////////////////////////////////////////////////////////////////////
/// Destructor.
XboxAnalyserResult::~XboxAnalyserResult() {
	clear();
}

////////////////////////////////////////////////////////////////////////
/// Default Settings.
void XboxAnalyserResult::init() {
	fXboxVersion = 0;
	fTimeStamp.Set(0, 0, 0, 0, 0, 0, 0, true, 0);
	fLogType = -1;
	fPulseCount = 0;
	fDeltaF = 0;
	fLine = 0;
	fBreakdownFlag = false;

	fRefXmin = -1.;
	fRefXmax = -1.;
	fRefYmin = -1.;
	fRefYmax = -1;
	fRefYavg = -1.;
	fRefYint = -1.;
	fRefYpp = -1.;

	fXmin = -1.;
	fXmax = -1.;
	fYmin = -1.;
	fYmax = -1;
	fYavg = -1.;
	fYint = -1.;
	fYpp = -1.;

	fJitter = -1.;
	fXdev = -1.;
}

////////////////////////////////////////////////////////////////////////
/// Clear.
/// Subroutine to destruct this object.
void XboxAnalyserResult::clear() {
}

////////////////////////////////////////////////////////////////////////
/// Reset.
void XboxAnalyserResult::reset() {
	clear();
	init();
}

////////////////////////////////////////////////////////////////////////
/// Simple printing function.
void XboxAnalyserResult::print() {

	printf("----------------------------------------------------\n");
	printf("fXboxVersion          : %u\n", fXboxVersion);

	printf("fTimestamp            : %s\n", fTimeStamp.AsString());
	printf("fLogType              : %d\n", fLogType);
	printf("fPulseCount           : %llu\n", fPulseCount);
	printf("fDeltaF               : %e\n", fDeltaF);
	printf("fLine                 : %u\n", fLine);
	printf("fBreakdownFlag        : %u\n", fBreakdownFlag);

	printf("fXmin                 : %e\n", fXmin);
	printf("fXmax                 : %e\n", fXmax);
	printf("fYmin                 : %e\n", fYmin);
	printf("fYmax                 : %e\n", fYmax);
	printf("fYavg (average)       : %e\n", fYavg);
	printf("fYint (integral)      : %e\n", fYint);
	printf("fYpp (peak to peak)   : %e\n", fYpp);

	printf("fJitter               : %e\n", fJitter);
	printf("fDevTime              : %e\n", fXdev);
	printf("----------------------------------------------------\n");
}

////////////////////////////////////////////////////////////////////////
/// Comparison operator.
/// Smaller-Than-Operator refer to the time stamp.
/// \param[in] rhs Xbox DAQ channel for comparison.
/// \return    true if time stamp is smaller than of rhs.
Bool_t XboxAnalyserResult::operator < (const XboxAnalyserResult& rhs) const {

	return fTimeStamp < rhs.getTimeStamp();
}

////////////////////////////////////////////////////////////////////////
/// Comparison operator.
/// Larger-Than-Operator refer to the time stamp.
/// \param[in] rhs Xbox DAQ channel for comparison.
/// \return    true if time stamp is larger than of rhs.
Bool_t XboxAnalyserResult::operator > (const XboxAnalyserResult& rhs) const {

	return fTimeStamp > rhs.getTimeStamp();
}


#ifndef XBOX_NO_NAMESPACE
}
#endif
