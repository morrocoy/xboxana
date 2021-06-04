#ifndef _XBOXANALYSEREVALDEVIATION_HXX_
#define _XBOXANALYSEREVALDEVIATION_HXX_

#include <iostream>

#include "Rtypes.h"
#include "XboxAnalyserEvalBase.hxx"
#include "XboxDAQChannel.hxx"
#include "XboxSignalFilter.hxx"

#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif

class XboxDAQChannel;


class XboxAnalyserEvalDeviation {

private:
	Double_t              fWmin;                 ///<Lower limit in relative coordinates.
	Double_t              fWmax;                 ///<Upper limit on the considered time axis.
	Double_t              fThCoarse;             ///<Threshold for estimating the deviation time.
	Double_t              fThRefine;             ///<Threshold for refined evaluation of the deviation time.
	Double_t              fProximity;            ///<Proximity to refine the deviation time.

	XBOX::XboxSignalFilter fFilterSig;           ///!Filter applied on the signal of each channel.
	XBOX::XboxSignalFilter fFilterDev;           ///!Filter applied on the difference between the signals of ch1 and ch2.

	size_t                fSamplesCoarse;        ///!Number of interpolation points for coarse evaluation.
	size_t                fSamplesRefine;        ///!Number of interpolation points for refined evaluation.
	size_t                fWindowCoarse;
	size_t                fWindowRefine;
	
	Double_t              fReportFlag;           ///!Enabled or disable report.
	std::string           fReportDir;            ///!Directory to export report.


	Double_t              absTh(std::vector<Double_t> &y, Double_t th);
	Double_t              absThCoarse(std::vector<Double_t> &y);
	Double_t              absThRefine(std::vector<Double_t> &y);
	Double_t              argTh(std::vector<Double_t> &t, std::vector<Double_t> &y, Double_t th);
	Double_t              argThCoarse(std::vector<Double_t> &t, std::vector<Double_t> &y, Double_t th);
	Double_t              argThRefine(std::vector<Double_t> &t, std::vector<Double_t> &y, Double_t th);

	Double_t              magn(std::vector<Double_t> &y);
	std::vector<Double_t> diff(std::vector<Double_t> &y1, std::vector<Double_t> &y2);

	Double_t evalDeviation(XBOX::XboxDAQChannel &ch0, XBOX::XboxDAQChannel &ch1);
	void report(XBOX::XboxDAQChannel ch1, XBOX::XboxDAQChannel ch2, Double_t jitter);

public:
	XboxAnalyserEvalDeviation();
	XboxAnalyserEvalDeviation(Double_t wmin, Double_t wmax, Double_t thc,
			Double_t thf, Double_t prox);
	~XboxAnalyserEvalDeviation();

	void init();
	void clear();
	void reset();

	//setter
	void config(Double_t wmin,
			Double_t wmax, Double_t thc, Double_t thf, Double_t prox);

	// evaluation
	Double_t operator () (
			XBOX::XboxDAQChannel &ch, XBOX::XboxDAQChannel &chref, Double_t jitter);

	// reporting
	void setReportDir(const std::string &sval) { fReportDir = sval; }
	void setReport(const Bool_t val) { fReportFlag = val; }
};


#ifndef XBOX_NO_NAMESPACE
}
#endif

#endif /* _XBOXANALYSEREVALDEVIATION_HXX_ */



