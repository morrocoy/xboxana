#ifndef _XBOXANALYSEREVALBASE_HXX_
#define _XBOXANALYSEREVALBASE_HXX_

#include "Rtypes.h"
#include "XboxDAQChannel.hxx"

#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif

class XboxDAQChannel;

class XboxAnalyserEvalBase {


private:

public:
	XboxAnalyserEvalBase();
	virtual ~XboxAnalyserEvalBase();

	virtual void init();
	virtual void clear();
	virtual void reset();

	//setter

	// evaluation
	virtual XBOX::XboxDAQChannel operator () (XBOX::XboxDAQChannel &ch);

};


#ifndef XBOX_NO_NAMESPACE
}
#endif

#endif /* _XBOXANALYSEREVALBASE_HXX_ */



