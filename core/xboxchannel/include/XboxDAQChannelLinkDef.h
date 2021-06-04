// A LinkDef.h file with all the explicit template instances
// that will be needed at link time
#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

//#pragma link C++ class XboxDAQChannel<char>+;
//#pragma link C++ class XboxDAQChannel<short>+;
//#pragma link C++ class XboxDAQChannel<int>+;
//#pragma link C++ class XboxDAQChannel<long>+;
//#pragma link C++ class XboxDAQChannel<float>+;
//#pragma link C++ class XboxDAQChannel<double>+;


#ifndef XBOX_NO_NAMESPACE
#pragma link C++ class XBOX::XboxDAQChannel+;
#else
#pragma link C++ class XboxDAQChannel+;
#endif

#endif
