#ifndef _XBOXANALYSEREVALBREAKDOWN_HXX_
#define _XBOXANALYSEREVALBREAKDOWN_HXX_

#include <iostream>

#include "Rtypes.h"
#include "XboxDAQChannel.hxx"
#include "XboxAnalyserEntry.hxx"
#include "XboxAnalyserResult.hxx"

#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif

class XboxDAQChannel;
class XboxAnalyserEntry;
class XboxAnalyserResult;

class XboxAnalyserEvalBreakdown {

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
	Double_t evalDeviation(XBOX::XboxDAQChannel &ch0, XBOX::XboxDAQChannel &ch1);

public:
	XboxAnalyserEvalBreakdown();
	~XboxAnalyserEvalBreakdown();

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
	XBOX::XboxAnalyserEntry operator_old (
			XBOX::XboxDAQChannel &b0psi, XBOX::XboxDAQChannel &b1psi,
			XBOX::XboxDAQChannel &b0pei, XBOX::XboxDAQChannel &b1pei,
			XBOX::XboxDAQChannel &b0psr, XBOX::XboxDAQChannel &b1psr);

	XBOX::XboxAnalyserResult operator () (
			XBOX::XboxDAQChannel &chMagn, XBOX::XboxDAQChannel &chMagnRef,
			XBOX::XboxDAQChannel &chJitter, XBOX::XboxDAQChannel &chJitterRef);

};


#ifndef XBOX_NO_NAMESPACE
}
#endif

#endif /* _XBOXANALYSEREVALBREAKDOWN_HXX_ */



