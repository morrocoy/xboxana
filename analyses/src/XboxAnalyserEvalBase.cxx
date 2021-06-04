#include "XboxAnalyserEvalBase.hxx"

#include <iostream>

// root
#include "TTimeStamp.h"


#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif


////////////////////////////////////////////////////////////////////////
/// Constructor.
XboxAnalyserEvalBase::XboxAnalyserEvalBase() {
	init();
}

////////////////////////////////////////////////////////////////////////
/// Destructor.
XboxAnalyserEvalBase::~XboxAnalyserEvalBase() {
	clear();
}

////////////////////////////////////////////////////////////////////////
/// Default Settings.
void XboxAnalyserEvalBase::init() {
}

////////////////////////////////////////////////////////////////////////
/// Clear.
void XboxAnalyserEvalBase::clear(){
}

////////////////////////////////////////////////////////////////////////
/// Reset.
void XboxAnalyserEvalBase::reset() {
	clear();
	init();
}

////////////////////////////////////////////////////////////////////////
/// Calling function.
/// Reads meta data from the channel.
/// \param[in] ch The Xbox channel.
/// \return The meta data (xbox version, pulse count, ...)
XBOX::XboxDAQChannel XboxAnalyserEvalBase::operator () (
		XBOX::XboxDAQChannel &ch) {

	ch.flushbuffer();
	XBOX::XboxDAQChannel chnew = ch;
	chnew.clear(); // empty signal data but keep meta data

	return chnew;
}


#ifndef XBOX_NO_NAMESPACE
}
#endif



