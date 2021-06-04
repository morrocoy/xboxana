#ifndef _XBOSANALYSERBREAKDOWNRATE_HXX_
#define _XBOSANALYSERBREAKDOWNRATE_HXX_

#include <iostream>

// root
#include "Rtypes.h"
#include "TTimeStamp.h"

#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif

class XboxDAQChannel;
class XboxAnalyserEntry;
class XboxSignalFilter;


class XboxAnalyserBreakdownRate {

public:
	enum EWeightType { // weighting function to smoothen the breakdown rate
		kGaussian
	};

private:

	std::vector<ULong64_t> fPCnt;                     ///<Pulse count (imported)
	std::vector<Double_t> fRate;                      ///<Pulse count (re-sampled)

	Int_t                 fNSamples;                  ///<Number of sampling points
	EWeightType           fWeightType;
	Double_t              fSigma;
	Int_t                 fNSigma;

public:

	XboxAnalyserBreakdownRate(std::vector<ULong64_t> vPCnt);

	~XboxAnalyserBreakdownRate();

	void init();
	void clear();
	void reset();


	// setter
	void setWeightFunction(EWeightType wtype, Double_t sig, Int_t nsig);
	void setSamples(Int_t nsamples) { fNSamples = nsamples; }

	// evaluation methods
	void fromEquidistantStep(std::vector<ULong64_t> &pcnt,
			std::vector<Double_t> &rate, Int_t nstep=1);
	void fromEquidistantPCnt(std::vector<ULong64_t> &pcnt,
			std::vector<Double_t> &rate);
	void fromMovingWindow(std::vector<ULong64_t> &pcnt,
			std::vector<Double_t> &rate);
};


#ifndef XBOX_NO_NAMESPACE
}
#endif

#endif /* _XBOSANALYSERBREAKDOWNRATE_HXX_ */

