#ifndef _XBOXANALYSEREVALBREAKDOWNTEST_HXX_
#define _XBOXANALYSEREVALBREAKDOWNTEST_HXX_

#include <iostream>

#include "Rtypes.h"
#include "XboxDAQChannel.hxx"
#include "XboxAnalyserEntry.hxx"


#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif

class XboxDAQChannel;
class XboxAnalyserEntry;

class XboxAnalyserEvalBreakdownTest {

private:
	Double_t              fPulseWMin;                 ///<Lower limit in relative coordinates.
	Double_t              fPulseWMax;                 ///<Upper limit on the considered time axis.
	Double_t              fPulseTh;                   ///<Threshold to evaluate the pulse width and height.

	Double_t              fJitterWMin;                ///<Time window lower limit for jitter evaluation (relative coordinates).
	Double_t              fJitterWMax;                ///<Time window upper limit for jitter evaluation (relative coordinates).
	Double_t              fJitterTh;                  ///<Threshold to evaluate the jitter. Relative to maximum.
	Double_t              fJitterMax;                 ///<Maximum acceptable jitter (relative to maximum time window).

	Double_t              fRiseWMin;                  ///<Lower limit in relative coordinates.
	Double_t              fRiseWMax;                  ///<Upper limit on the considered time axis.
	Double_t              fRiseProx;                  ///<Threshold to evaluate the pulse width and height.
	Double_t              fRiseTh;                    ///<Threshold to evaluate the pulse width and height.

	Double_t              fDeflWMin;                  ///<Lower limit in relative coordinates.
	Double_t              fDeflWMax;                  ///<Upper limit on the considered time axis.
	Double_t              fDeflProx;                  ///<Proximity to refine the deflection time.
	Double_t              fDeflThC;                   ///<Threshold for estimating the deviation time.
	Double_t              fDeflThF;                   ///<Threshold for refined evaluation of the deviation time.

	Double_t evalJitter(XBOX::XboxDAQChannel &ch0, XBOX::XboxDAQChannel &ch1);
	Double_t evalRisingEdge(XBOX::XboxDAQChannel &ch);
	Double_t evalDeflection(XBOX::XboxDAQChannel &ch0, XBOX::XboxDAQChannel &ch1);

public:
	XboxAnalyserEvalBreakdownTest();
	~XboxAnalyserEvalBreakdownTest();

	void init();
	void clear();
	void reset();

	//setter
	void setPulseConfig(Double_t wmin, Double_t wmax, Double_t th);
	void setJitterConfig(Double_t wmin, Double_t wmax, Double_t th, Double_t max);
	void setRiseConfig(Double_t wmin, Double_t wmax, Double_t th, Double_t prox);
	void setDeflConfig(Double_t wmin,
			Double_t wmax, Double_t thc, Double_t thf, Double_t prox);

	// evaluation of pulse parameters and breakdown location
	XBOX::XboxAnalyserEntry operator () (
			XBOX::XboxDAQChannel &b0psi, XBOX::XboxDAQChannel &b1psi,
			XBOX::XboxDAQChannel &b0pei, XBOX::XboxDAQChannel &b1pei,
			XBOX::XboxDAQChannel &b0psr, XBOX::XboxDAQChannel &b1psr);

	// methods for testing purpose
	void fitExponential(Int_t n, Double_t *x, Double_t *y, Double_t &p0, Double_t &p1);
	void plotSignal(const std::string &filepath, std::vector<Double_t> &x,
			std::vector<Double_t> &y0, std::vector<Double_t> &y1);
	void plotSignal(const std::string &filepath, XBOX::XboxDAQChannel &ch0,
			XBOX::XboxDAQChannel &ch1, Double_t jitter,
			Double_t lowerlimit=-1,	Double_t upperlimit=-1,
			const std::vector<Double_t> &xmarkers=std::vector<Double_t>());
};


#ifndef XBOX_NO_NAMESPACE
}
#endif

#endif /* _XBOXANALYSEREVALBREAKDOWNTEST_HXX_ */



