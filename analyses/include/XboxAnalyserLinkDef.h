// A LinkDef.h file with all the explicit template instances
// that will be needed at link time
#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;



#ifndef XBOX_NO_NAMESPACE
#pragma link C++ class XBOX::XboxAnalyserEntry+;
#pragma link C++ class XBOX::XboxAnalyserResult+;
#else
#pragma link C++ class XboxAnalyserEntry+;
#pragma link C++ class XboxAnalyserResult+;
#endif

#endif

