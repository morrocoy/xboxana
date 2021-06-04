#ifndef _XBOXANALYSEREVALPULSESHAPE_HXX_
#define _XBOXANALYSEREVALPULSESHAPE_HXX_

#include "Rtypes.h"

#include "XboxAnalyserEvalBase.hxx"
#include "XboxDAQChannel.hxx"

#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif

class XboxDAQChannel;

class XboxAnalyserEvalPulseShape {

private:
	Double_t              fPulseWmin;                 ///<Lower limit in relative coordinates.
	Double_t              fPulseWmax;                 ///<Upper limit on the considered time axis.
	Double_t              fPulseTh;                   ///<Threshold to evaluate the pulse width and height.

	Double_t              fReportFlag;                ///!Enabled/ disabled reporting.
	std::string           fReportDir;                 ///!Directory to create

	void report(XBOX::XboxDAQChannel &ch);

public:
	XboxAnalyserEvalPulseShape();
	XboxAnalyserEvalPulseShape(Double_t wmin, Double_t wmax, Double_t th);
	~XboxAnalyserEvalPulseShape();

	void init();
	void clear();
	void reset();

	//setter
	void configPulse(Double_t wmin, Double_t wmax, Double_t th);

	// evaluation
	XBOX::XboxDAQChannel operator () (XBOX::XboxDAQChannel &ch);

	// reporting
	void setReportDir(const std::string &sval) { fReportDir = sval; }
	void setReport(const Bool_t val) { fReportFlag = val; }

};


#ifndef XBOX_NO_NAMESPACE
}
#endif

#endif /* _XBOXANALYSEREVALPULSESHAPE_HXX_ */



