#ifndef _XBOXANALYSEREVALSIGNAL_HXX_
#define _XBOXANALYSEREVALSIGNAL_HXX_

#include "Rtypes.h"

#include "XboxAnalyserEvalBase.hxx"
#include "XboxDAQChannel.hxx"

#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif

class XboxDAQChannel;

class XboxAnalyserEvalSignal : XboxAnalyserEvalBase {

private:

public:
	XboxAnalyserEvalSignal();
	~XboxAnalyserEvalSignal();

	void init();
	void clear();
	void reset();

	//setter
	void config();

	// evaluation
	XBOX::XboxDAQChannel operator () (XBOX::XboxDAQChannel &ch,
			Double_t xmin, Double_t xmax);

};


#ifndef XBOX_NO_NAMESPACE
}
#endif

#endif /* _XBOXANALYSEREVALSIGNAL_HXX_ */



