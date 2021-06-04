
#ifndef __testclass_HXX_
#define __testclass_HXX_


#include <iostream>     // std::cout
#include <stdlib.h>
#include <string>
#include <vector>
#include <map>
#include <ctime>

#include "Rtypes.h"
#include "TObject.h"
#include "TTimeStamp.h"



class testclass  : public TObject
{

private:
	Int_t a;

public:

	testclass();
	testclass(Int_t val);
	virtual ~testclass();
	
	void setVal(Int_t val);
	Int_t getVal() const {return a;}

	ClassDef(testclass,1);	// Xbox Data Acquisition Channel class
};

#endif
