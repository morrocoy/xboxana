#ifndef _XBOXANALYSEREVALJITTER_HXX_
#define _XBOXANALYSEREVALJITTER_HXX_

#include <iostream>

#include "Rtypes.h"
//#include "XboxAnalyserEvalBase.hxx"
#include "XboxDAQChannel.hxx"

#include "XboxSignalFilter.hxx"

#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif

class XboxDAQChannel;

class XboxAnalyserEvalJitter {

private:
	Double_t              fWmin;                 ///<Time window lower limit for jitter evaluation (relative coordinates).
	Double_t              fWmax;                 ///<Time window upper limit for jitter evaluation (relative coordinates).
	Double_t              fTh;                   ///<Threshold to evaluate the jitter. Relative to maximum.
	Double_t              fTol;                  ///<Maximum acceptable jitter (relative to maximum time window).

	XBOX::XboxSignalFilter fFilter;              ///!Signal filter.

	size_t                fSamples;              ///!Number of interpolation points.

	Double_t              fReportFlag;           ///!Enabled or disable report.
	std::string           fReportDir;            ///!Directory to export report.

	Double_t absTh(std::vector<Double_t> &y);
	Double_t argTh(std::vector<Double_t> &t, std::vector<Double_t> &y, Double_t th);
	Double_t evalJitter(XBOX::XboxDAQChannel &ch1, XBOX::XboxDAQChannel &ch2);
	void report(XBOX::XboxDAQChannel ch1, XBOX::XboxDAQChannel ch2);

public:
	XboxAnalyserEvalJitter();
	XboxAnalyserEvalJitter(Double_t wmin, Double_t wmax, Double_t th, Double_t max);
	~XboxAnalyserEvalJitter();

	void init();
	void clear();
	void reset();

	//setter
	void config(Double_t wmin, Double_t wmax, Double_t th, Double_t max);

	// evaluation of pulse parameters and breakdown location
	Double_t operator () (XBOX::XboxDAQChannel &ch1, XBOX::XboxDAQChannel &ch2);

	// reporting
	void setReportDir(const std::string &sval) { fReportDir = sval; }
	void setReport(const Bool_t val) { fReportFlag = val; }
};


#ifndef XBOX_NO_NAMESPACE
}
#endif

#endif /* _XBOXANALYSEREVALJITTER_HXX_ */



