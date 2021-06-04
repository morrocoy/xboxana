#ifndef _XBOSANALYSERVIEW_HXX_
#define _XBOSANALYSERVIEW_HXX_

#include <iostream>

// root
#include "Rtypes.h"
#include "TTimeStamp.h"
#include "TH1D.h"
#include "TH2D.h"



// root graphics
#include "TStyle.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TLegend.h"
#include "TGraph.h"
#include "TGaxis.h"

// xbox
#include "XboxDAQChannel.hxx"


#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif

class XboxDAQChannel;
class XboxSignalFilter;


class XboxAnalyserView {

public:

	enum EHorzAxis {
		kTime,
		kPCnt,
	};

private:

	TTimeStamp            fDateBegin;                 ///<First day to be consider.
	TTimeStamp            fDateEnd;                   ///<Last day to be consider.
	Double_t              fPAvgMin;                   ///<Minimum limit to be plotted of pulse average power.
	Double_t              fPAvgMax;                   ///<Maximum limit to be plotted of pulse average power.
	Double_t              fPLenMin;                   ///<Minimum limit to be plotted of pulse length.
	Double_t              fPLenMax;                   ///<Maximum limit to be plotted of pulse length.
	ULong64_t             fPCntMin;                   ///<Minimum limit to be plotted of pulse count.
	ULong64_t             fPCntMax;                   ///<Maximum limit to be plotted of pulse count.
	Double_t              fRateMin;                   ///<Minimum limit to be plotted of breakdown rate.
	Double_t              fRateMax;                   ///<Maximum limit to be plotted of breakdown rate.
	Double_t              fBDPosTimeMin;              ///<Time at which the breakdown occurs.
	Double_t              fBDPosTimeMax;              ///<Time at which the breakdown occurs.

	Double_t              fPAvgScale;                 ///<Scale of pulse average power.
	Double_t              fPLenScale;                 ///<Scale of pulse length.
	Double_t              fPCntScale;                 ///<Scale of pulse count.
	Double_t              fRateScale;                 ///<Scale of breakdown rate.
	Double_t              fBDPosTimeScale;            ///<Scale of breakdown rate.

	Int_t                 fXBins;                     ///<Number of bins in the horizontal direction of the 2D Histograms
	Int_t                 fYBins;                     ///<Number of bins in the vertical direction of the 2D Histograms
	Int_t                 fNSamples;                  ///<Number of interpolation points

	EHorzAxis             fHorzAxis;

	std::vector<std::string> fFilePaths;              ///!Input files.

	std::map<std::string, std::vector<XBOX::XboxDAQChannel>> fCategoryChannel; // Map to access the trees of the source file
	std::map<std::string, std::vector<Double_t>> fCategoryTuple;
	std::map<std::string, std::vector<TTimeStamp>> fCategoryTimeStamp;
	std::map<std::string, std::vector<ULong64_t>> fCategoryPulseCount;

//	std::map<std::string, int> fCategory;
//	std::vector<XBOX::XboxAnalyserResult> fPulseResults; ///!Vector containing the results of normal pulses
//	std::vector<XBOX::XboxAnalyserResult> fBreakdownResults; ///!Vector containing the results breakdowns


	TCanvas              *fCanvas;                    ///!Canvas for any plots.
	TPad                 *fPad1;                      ///!Base pad.
//	TPad                 *fPad2;                      ///!Transparent overlay pad.
	TLegend              *fLegend;
	Int_t                 fCWidth;                    ///!Canvas width.
	Int_t                 fCHeight;                   ///!Canvas height.
	Float_t               fCMargin;                   ///!Horizontal margin.

	std::vector<TH2D*>    fBufferHist;                ///!Plot buffer
	std::vector<TGraph*>  fBufferGraph;               ///!Plot buffer
	std::vector<TGaxis*>  fBufferAxis;                ///!Plot buffer
	std::vector<TLine*>   fBufferLine;                ///!Plot buffer


	template <typename T>
	std::vector<Double_t> rescale(std::vector<T> &y, T ymin, T ymax,
			Bool_t blog=false, Double_t ysmin=0., Double_t ysmax=1.);

//	std::vector<ULong64_t> convertToPulseCnt(std::vector<TTimeStamp> &time);

	std::vector<std::string> getTreeNames();
	std::vector<std::string> getColNames(
			const std::string &treeName,
			const std::vector<std::string> &colNameContains);

public:

	XboxAnalyserView();
	~XboxAnalyserView();

	void                  init();
	void                  clear();
	void                  reset();

	void                  addFiles(std::string fileNameGlob);
	void                  printTreeNames();
	void                  printColNames(
			               const std::string &treeName,
		               	   const std::vector<std::string> &colNameContains = {});

	std::string           getDirectoryName(const std::string &sFilePathGlob);
	std::string           getFileName(const std::string &sFilePathGlob);
//	std::vector<std::string> getListOfFiles(const string &sFilePathGlob);

	// getter
	Int_t                 getXboxVersion(const std::string &sKey);
	void                  getPeriodBounds(const std::string &sKey,
							TTimeStamp &lbnd, TTimeStamp &ubnd);

	// setter
	void                  setLimitsTime(TTimeStamp begin, TTimeStamp end);
	void                  setLimitsPAvg(Double_t min, Double_t max, Double_t scale=1.);
	void                  setLimitsPLen(Double_t min, Double_t max, Double_t scale=1.);
	void                  setLimitsPCnt(Double_t min, Double_t max, Double_t scale=1.);
	void                  setLimitsRate(Double_t min, Double_t max, Double_t scale=1.);
	void                  setLimitsBDPosTime(Double_t min, Double_t max, Double_t scale=1.);
	void                  setBins(Int_t xbins, Int_t ybins);
	void                  autosetPCntLimits(const std::string &sKey);

	void                  setWindowSize(Int_t ww, Int_t wh);
//	void                  setStyle(Int_t imode=0);
	void                  setHorzAxis(const EHorzAxis val);


	void                  plotPAvg(const std::string &sKey,
			                Int_t iColor=kBlack, const std::string &sOpt="",
							Style_t iMarkerStyle=0, size_t iMarkerSize=1);
	void                  plotPLen(const std::string &sKey,
                            Int_t iColor=kBlack, const std::string &sOpt="",
			                Style_t iMarkerStyle=0, size_t iMarkerSize=1);
	void                  plotPCnt(const std::string &sKey,
			                Int_t iColor=kBlack, const std::string &sOpt="",
							Style_t iMarkerStyle=0, size_t iMarkerSize=1);
	void                  plotRate(const std::string &sKey,
			                Int_t iColor=kBlack, const std::string &sOpt="",
							Style_t iMarkerStyle=0, size_t iMarkerSize=1);
	void                  plotBDPosTime(
			                const std::string &sKey,
							Int_t iColor=kBlack, const std::string &sOpt="",
							Style_t iMarkerStyle=0, size_t iMarkerSize=1);

	void                  formatAxisHorz(TAxis *ax, Int_t iColor=kBlack);
	void                  formatAxisPAvg(TAxis *ax, Int_t iColor=kBlack);
	void                  formatAxisPLen(TAxis *ax, Int_t iColor=kBlack);
	void                  formatAxisPCnt(TAxis *ax, Int_t iColor=kBlack);
	void                  formatAxisBDPosTime(TAxis *ax, Int_t iColor=kBlack);

	void                  drawAxisHorz(Int_t iColor=kBlack, const std::string &sOpt="S", Int_t ndiv=510);
	void                  drawAxisPAvg(Int_t iColor=kBlack, const std::string &sOpt="S", Int_t ndiv=510);
	void                  drawAxisPLen(Int_t iColor=kBlack, const std::string &sOpt="S", Int_t ndiv=510);
	void                  drawAxisPCnt(Int_t iColor=kBlack, const std::string &sOpt="+LS", Int_t ndiv=510);
	void                  drawAxisRate(Int_t iColor=kBlack, const std::string &sOpt="+GLS", Int_t ndiv=510);

	void                  show();
	void                  save(const std::string &filepath);


	// io methods
//	void                  load(
//			                const std::string &sFilePathGlob,
//							const std::string &sKey="");
	void                  load(
							const std::string &filePathGlob,
                            const std::string &treeName,
        	                const std::vector<std::string> &colNameContains);

	// plotting methods for testing purpose
	void plotSignal(XBOX::XboxDAQChannel &ch0,
			Double_t tmin=-1e99, Double_t tmax=1e99);
	void plotSignal(XBOX::XboxDAQChannel &ch0, XBOX::XboxDAQChannel &ch1,
			Double_t tmin=-1e99, Double_t tmax=1e99);

	void plotSignal(const std::string &sKey, Int_t idx,
            Int_t iColor=kBlack, const std::string &sOpt="",
			Style_t iMarkerStyle=0, size_t iMarkerSize=1,
			Double_t tmin=-1e99, Double_t tmax=1e99,
			Double_t ymin=-1e99, Double_t ymax=1e99);
	void plotSignal(const std::string &sKey1, const std::string &sKey2, Int_t idx,
	            Int_t iColor=kBlack, const std::string &sOpt="",
				Style_t iMarkerStyle=0, size_t iMarkerSize=1,
				Double_t tmin=-1e99, Double_t tmax=1e99,
				Double_t ymin=-1e99, Double_t ymax=1e99);

	void plotProbes();
	void plotTest(const std::string &filepath);
	void plotTest(const std::string &filepath, TH2D &h, std::string mode="scat=1");
};


#ifndef XBOX_NO_NAMESPACE
}
#endif

#endif /* _XBOSANALYSERVIEW_HXX_ */

