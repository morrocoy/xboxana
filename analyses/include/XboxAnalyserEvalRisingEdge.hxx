#ifndef _XBOXANALYSEREVALRISINGEDGE_HXX_
#define _XBOXANALYSEREVALRISINGEDGE_HXX_

#include <iostream>

#include "Rtypes.h"
//#include "XboxAnalyserEvalBase.hxx"
#include "XboxDAQChannel.hxx"
#include "XboxSignalFilter.hxx"

#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif

class XboxDAQChannel;

class XboxAnalyserEvalRisingEdge {

private:
	Double_t              fWmin;                 ///<Lower limit in relative coordinates.
	Double_t              fWmax;                 ///<Upper limit on the considered time axis.
	Double_t              fTh;                   ///<Threshold to evaluate the roughly the time of the rising edge.
	Double_t              fProximity;            ///<Proximity to refine the time at wich the rising edge starts.

	XBOX::XboxSignalFilter fFilterSig;           ///!Filter applied on the signal of the channel.
	XBOX::XboxSignalFilter fFilterD1;            ///!Filter applied on the signal of the channel (first derivative).
	XBOX::XboxSignalFilter fFilterD2;            ///!Filter applied on the signal of the channel (second derivative).

	size_t                fSamplesRefine;        ///!Number of interpolation points for refined evaluation.
	size_t                fWindowSize;


	Double_t              fReportFlag;           ///!Enabled or disable report.
	std::string           fReportDir;            ///!Directory to export report.

	size_t                argMax(std::vector<Double_t> &y, size_t imin, size_t imax);
	size_t                argLeftRoot(std::vector<Double_t> &y, size_t istart);
	size_t                argRightRoot(std::vector<Double_t> &y, size_t istart);
	Double_t              evalRisingEdge(XBOX::XboxDAQChannel &ch);
	void                  report(XBOX::XboxDAQChannel ch);

	std::vector<Double_t> rescale(std::vector<Double_t> &y, Double_t ysmin, Double_t ysmax);
public:
	XboxAnalyserEvalRisingEdge();
	XboxAnalyserEvalRisingEdge(Double_t wmin, Double_t wmax, Double_t th, Double_t prox);
	~XboxAnalyserEvalRisingEdge();

	void init();
	void clear();
	void reset();

	//setter
	void config(Double_t wmin, Double_t wmax, Double_t th, Double_t prox);

	// evaluation
	Double_t operator () (XBOX::XboxDAQChannel &ch);

	// reporting
	void setReportDir(const std::string &sval) { fReportDir = sval; }
	void setReport(const Bool_t val) { fReportFlag = val; }

};


#ifndef XBOX_NO_NAMESPACE
}
#endif

#endif /* _XBOXANALYSEREVALRISINGEDGE_HXX_ */



