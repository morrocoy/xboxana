#include "XboxAnalyserView.hxx"

// root
#include "TSystemDirectory.h"
#include "TRegexp.h"
#include "TString.h"

#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TKey.h"

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RDFHelpers.hxx"
#include "TMath.h"

#include "ROOT/TThreadExecutor.hxx"
#include "ROOT/TSeq.hxx"

#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TBranchElement.h"

#include "XboxFileSystem.h"


#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif


////////////////////////////////////////////////////////////////////////
/// Constructor.
XboxAnalyserView::XboxAnalyserView() {
	init();
}

////////////////////////////////////////////////////////////////////////
/// Destructor.
XboxAnalyserView::~XboxAnalyserView() {
	clear();
}

////////////////////////////////////////////////////////////////////////
/// Default Settings.
void XboxAnalyserView::init() {

	fDateBegin = 0; //TTimeStamp(2010,1,1,00,00,00);
	fDateEnd = 0; //TTimeStamp(2030,1,1,00,00,00);

	fPAvgMin = 0.;
	fPAvgMax = 45e6;
	fPLenMin = 0.;
	fPLenMax = 200e-9;
	fPCntMin = 0.;
	fPCntMax = 2.5e9;
	fRateMin = 1e-8;
	fRateMax = 1e-3;
	fBDPosTimeMin = 0.;
	fBDPosTimeMax = 70e-9;

	fPAvgScale = 1e6;
	fPLenScale = 1e-9;
	fPCntScale = 1e9;
	fRateScale = 1;
	fBDPosTimeScale = 1e-9;

	fXBins = 100;
	fYBins = 100;

	fNSamples = 1000;

	// initialise canvas and pads
	fCWidth = 1650;
	fCHeight = 1000/2;

	fCMargin = 0.3 * fCHeight / fCWidth;
	fCanvas = new TCanvas("c0","", fCWidth, fCHeight);

	fPad1 = new TPad("pad1", "", 0, 0, 1, 1);
	fPad1->SetLeftMargin(fCMargin);
	fPad1->SetRightMargin(fCMargin);
	fPad1->SetLeftMargin(fCMargin);
	fPad1->SetRightMargin(fCMargin);
	fPad1->SetGrid();

	// transparent pad
//	fPad2 = new TPad("pad2", "", 0, 0, 1, 1);
//	fPad2->SetLeftMargin(fCMargin);
//	fPad2->SetRightMargin(fCMargin);
//	fPad2->SetLeftMargin(fCMargin);
//	fPad2->SetRightMargin(fCMargin);
//	fPad2->SetFillColor(0);
//	fPad2->SetFillStyle(4000);
//	fPad2->SetFrameFillStyle(0);
//	fPad2->SetGrid();

	fPad1->Draw();
	fPad1->cd();

	fLegend = new TLegend(0.6,0.7,0.89,0.89);//,0.48,0.9);
//	fLegend = new TLegend(0.11,0.7,0.4,0.89);//,0.48,0.9);

	fHorzAxis = kTime;
}

////////////////////////////////////////////////////////////////////////
/// Clear.
/// Subroutine to destruct this object.
void XboxAnalyserView::clear(){


	for (auto &item : fBufferHist)
		delete item;
	for (auto &item : fBufferGraph)
		delete item;
	for (auto &item : fBufferAxis)
		delete item;

	fBufferHist.clear();
	fBufferGraph.clear();
	fBufferAxis.clear();

	fPad1->Clear();
	fPad1->Modified();
	fPad1->cd();
}

////////////////////////////////////////////////////////////////////////
/// Reset.
void XboxAnalyserView::reset() {
	clear();
	init();
}


void XboxAnalyserView::addFiles(std::string filePathGlob) {

	for (auto &filepath: getListOfFiles(filePathGlob))
		fFilePaths.push_back(filepath);

	std::sort(fFilePaths.begin(), fFilePaths.end());

}


////////////////////////////////////////////////////////////////////////
/// Get bounds of the time period.
Int_t XboxAnalyserView::getXboxVersion(const std::string &sKey) {

	if (fCategoryChannel[sKey].empty())
		return -1;
	else
		return fCategoryChannel[sKey].front().getXboxVersion();
}

////////////////////////////////////////////////////////////////////////
/// Get bounds of the time period.
/// \param[out] lbnd The lower bound.
/// \param[out] ubnd The upper bound.
void XboxAnalyserView::getPeriodBounds(
		const std::string &sKey, TTimeStamp &lbnd, TTimeStamp &ubnd) {

	if (fCategoryChannel[sKey].empty()) {
		lbnd = 0;
		ubnd = 0;
	}
	else {
		lbnd = fCategoryChannel[sKey].front().getTimeStamp();
		ubnd = fCategoryChannel[sKey].back().getTimeStamp();
	}
}

////////////////////////////////////////////////////////////////////////
/// Set window dimensions.
/// \param[in] ww The window width.
/// \param[in] wh The window height.
void XboxAnalyserView::setWindowSize(Int_t ww, Int_t wh) {

	fCWidth = ww;
	fCHeight = wh;
	fCMargin = 0.3 * fCHeight / fCWidth;

	fCanvas->SetWindowSize(fCWidth, fCHeight);
	fPad1->SetLeftMargin(fCMargin);
	fPad1->SetRightMargin(fCMargin);
	fPad1->SetLeftMargin(fCMargin);
	fPad1->SetRightMargin(fCMargin);

//	fPad2->SetLeftMargin(fCMargin);
//	fPad2->SetRightMargin(fCMargin);
//	fPad2->SetLeftMargin(fCMargin);
//	fPad2->SetRightMargin(fCMargin);

	clear();
}


////////////////////////////////////////////////////////////////////////
/// Set time period.
/// \param[in] begin The first date to be considered.
/// \param[in] end The last date to be considered.
void XboxAnalyserView::setLimitsTime(TTimeStamp begin, TTimeStamp end) {

	fDateBegin = begin;
	fDateEnd = end;
	clear();
}

void XboxAnalyserView::setHorzAxis(const EHorzAxis val) {
	fHorzAxis = val;
	clear();
}


////////////////////////////////////////////////////////////////////////
/// Set Bins.
/// \param[in] xbins The number of bins on the x-axis.
/// \param[in] ybins The number of bins on the y-axis.
void XboxAnalyserView::setBins(Int_t xbins, Int_t ybins) {

	fXBins = xbins;
	fYBins = ybins;
	clear();
}

////////////////////////////////////////////////////////////////////////
/// Set limits of averaged pulse power.
/// \param[in] min The minimum value to be plotted.
/// \param[in] max The maximum value to be plotted.
void XboxAnalyserView::setLimitsPAvg(
		Double_t min, Double_t max, Double_t scale) {

	fPAvgMin = min*scale;
	fPAvgMax = max*scale;
	fPAvgScale = scale;
	clear();
}

////////////////////////////////////////////////////////////////////////
/// Set limits of pulse length.
/// \param[in] min The minimum value to be plotted.
/// \param[in] max The maximum value to be plotted.
void XboxAnalyserView::setLimitsPLen(
		Double_t min, Double_t max, Double_t scale) {

	fPLenMin = min*scale;
	fPLenMax = max*scale;
	fPLenScale = scale;
	clear();
}

////////////////////////////////////////////////////////////////////////
/// Set limits of pulse count.
/// \param[in] min The minimum value to be plotted.
/// \param[in] max The maximum value to be plotted.
void XboxAnalyserView::setLimitsPCnt(
		Double_t min, Double_t max, Double_t scale) {

	fPCntMin = min*scale;
	fPCntMax = max*scale;
	fPCntScale = scale;
	clear();
}

////////////////////////////////////////////////////////////////////////
/// Auto set of the pulse count limits.
/// Adjust the limits of the pulse count according to the time period.
void XboxAnalyserView::autosetPCntLimits(const std::string &sKey) {

	if (fCategoryChannel[sKey].empty())
		return;

	fPCntMin = fCategoryChannel[sKey].front().getPulseCount();
	fPCntMax = fCategoryChannel[sKey].back().getPulseCount();
	for (size_t i=1; i<fCategoryChannel[sKey].size(); i++) {
		if (fCategoryChannel[sKey][i].getTimeStamp() > fDateBegin) {
			fPCntMin = fCategoryChannel[sKey][i-1].getPulseCount();
			break;
		}
	}
	for (size_t i=0; i<fCategoryChannel[sKey].size()-1; i++) {
		if (fCategoryChannel[sKey][i].getTimeStamp() > fDateEnd) {
			fPCntMax = fCategoryChannel[sKey][i].getPulseCount();
			break;
		}
	}
}


////////////////////////////////////////////////////////////////////////
/// Set limits of pulse rate (breakdown rate).
/// \param[in] min The minimum value to be plotted.
/// \param[in] max The maximum value to be plotted.
void XboxAnalyserView::setLimitsRate(Double_t min, Double_t max, Double_t scale) {

	fRateMin = min*scale;
	fRateMax = max*scale;
	fRateScale = scale;
	clear();
}

////////////////////////////////////////////////////////////////////////
/// Set limits of breakdown time.
/// \param[in] min The minimum value to be plotted.
/// \param[in] max The maximum value to be plotted.
void XboxAnalyserView::setLimitsBDPosTime(Double_t min, Double_t max, Double_t scale) {

	fBDPosTimeMin = min*scale;
	fBDPosTimeMax = max*scale;
	fBDPosTimeScale = scale;
	clear();
}

//////////////////////////////////////////////////////////////////////////
///// File operation.
///// Get the the directory of a path.
//std::string XboxAnalyserView::getDirectoryName(
//		const std::string &sFilePathGlob) {
//     size_t pos = sFilePathGlob.find_last_of("\\/");
//     return (std::string::npos == pos)
//         ? ""
//         : sFilePathGlob.substr(0, pos+1);
//}
//
//////////////////////////////////////////////////////////////////////////
///// File operation.
///// Extract the file name of a path. Regular expressions are allowed.
//std::string XboxAnalyserView::getFileName(
//		const std::string &sFilePathGlob) {
//     size_t pos = sFilePathGlob.find_last_of("\\/");
//     return (std::string::npos == pos)
//         ? ""
//         : sFilePathGlob.substr(pos+1);
//}

//////////////////////////////////////////////////////////////////////////
///// File operation.
///// Get the list of files in directory a given a regular expression.
//std::vector<std::string> XboxAnalyserView::getListOfFiles(
//		const string &sFilePathGlob) {
//
//	std::string sDirectory = getDirectoryName(sFilePathGlob);
//	std::string sFileNameGlob = getFileName(sFilePathGlob);
//
//    TSystemDirectory dir(sDirectory.c_str(), sDirectory.c_str());
//    TList *files = dir.GetListOfFiles();
//    std::vector<std::string> vListOfFiles;
//
//    TRegexp re(TString(sFileNameGlob.c_str()), true);
//    if (files) {
//        TSystemFile *file;
//        TIter next(files);
//        while ((file = (TSystemFile*) next())) {
//            std::string sFileName = file->GetName();
//            TString str = file->GetName();
//
//            if (str.Index(re) != -1) {
//            	vListOfFiles.push_back(sDirectory + "/" + sFileName);
//            }
//        }
//    }
//
//    std::sort(vListOfFiles.begin(), vListOfFiles.end());
//    return vListOfFiles;
//}



////////////////////////////////////////////////////////////////////////
/// Return the typename of object colName stored in t, if any. Return
/// an empty string if colName is not in t.
/// Supported cases:
/// - TBranchElements, as long as TTree::GetBranch resolves their names
std::string getBranchTypeName(TTree &t, const std::string &colName) {
	auto branch = t.GetBranch(colName.c_str());
	if (branch) {
		static const TClassRef tbranchelement("TBranchElement");
		if (branch->InheritsFrom(tbranchelement)) {
			auto be = static_cast<TBranchElement *>(branch);
			if (auto currentClass = be->GetCurrentClass())
				return currentClass->GetName();
			else
				return be->GetClassName();
		}
	}

   // colName is not a TBranchElement
   return std::string();
}

std::vector<std::string> XboxAnalyserView::getColNames(
		const std::string &treeName,
		const std::vector<std::string> &colNameContains) {

	std::vector<std::string> colNames;

	Bool_t treeExist = false;
	for (auto &s: getTreeNames()) {
		if (!treeName.compare(s)) {
			treeExist = true;
			break;
		}
	}

	if (!treeExist) {
		printf("Info: Tree \'%s\' not found!", treeName.c_str());
		return colNames;
	}

	TChain chain(treeName.c_str());
	for (auto &s: fFilePaths)
		chain.Add(s.c_str());
	ROOT::RDataFrame df(chain);

	for (auto &&colName : df.GetColumnNames()) {
		std::string colType = df.GetColumnType(colName);
		if (colName.compare("TObject")
				&& (!colType.compare("XBOX::XboxDAQChannel")
				   || !colType.compare("Double_t"))) {

			if (!colNameContains.empty()) {
				for (auto &s: colNameContains) {

					if (colName.find(s) != std::string::npos) {
						colNames.push_back(colName);
						break;
					}
				}
			}
			else {
				colNames.push_back(colName);
			}
		}
	}

   return colNames;
}

void XboxAnalyserView::printColNames(
		const std::string &treeName,
		const std::vector<std::string> &colNameContains) {

	for (auto &s: getColNames(treeName, colNameContains))
		printf("%s\n", s.c_str());
}


////////////////////////////////////////////////////////////////////////
/// Load method.
/// get columns of type XboxDAQChannel
std::vector<std::string> XboxAnalyserView::getTreeNames() {

	std::vector<std::string> treeNames;

	if (fFilePaths.empty())
		return treeNames;

	TFile file(fFilePaths.front().c_str());
	TIter fileIter(file.GetListOfKeys());
	TKey *fileKey=0;
	while ((fileKey = (TKey *)fileIter())) {
		TObject *fileObj = fileKey->ReadObj();
		if (fileObj->IsA()->InheritsFrom(TTree::Class())) {
			treeNames.push_back(fileObj->GetName());
		}
	}

	return treeNames;
}


void XboxAnalyserView::printTreeNames() {

	for (auto &s: getTreeNames())
		printf("%s\n", s.c_str());
}

////////////////////////////////////////////////////////////////////////
/// Load method.
/// Reads pre-evaluated results of pulse parameters into fPulsParam
/// \param[in] treename The name of the tree.
/// \param[in] filepath The name of the file the data are written to.
void XboxAnalyserView::load(const std::string &filePathGlob, const std::string &treeName,
		const std::vector<std::string> &colNameGlob) {

	if (fFilePaths.empty()) {
		printf("Error: No input files found\n");
		return;
	}

	std::vector<std::string> colNames = getColNames(treeName, colNameGlob);
	if (colNames.empty()) {
		printf("Info: No columns found found!");
		return;
	}

	// load file chain
	TChain chain(treeName.c_str());
	for (auto &s: fFilePaths)
		chain.Add(s.c_str());
	ROOT::RDataFrame df(chain);

	std::vector<std::string> colTypes;
	for (auto &colName: colNames)
		colTypes.push_back(df.GetColumnType(colName));

	for (size_t i=0; i< colNames.size(); i++) {
		// channels are sorted in time
		if (!colTypes[i].compare("XBOX::XboxDAQChannel")) {

			std::vector<XBOX::XboxDAQChannel> v = *df.Take<XBOX::XboxDAQChannel>(colNames[i]);
			std::sort (v.begin(), v.end()); // important if imt is enabled

			fCategoryChannel.emplace(treeName + "." + colNames[i], v);
			printf(" ... add column %s.%s\n", treeName.c_str(), colNames[i].c_str());
		}
		// tuples are unsorted
		else if (!colTypes[i].compare("Double_t") || !colTypes[i].compare("ULong64_t")) {

			std::vector<Double_t> v = *df.Take<Double_t>(colNames[i]);
			fCategoryTuple.emplace(treeName + "." + colNames[i], v);
			printf(" ... add column %s.%s\n", treeName.c_str(), colNames[i].c_str());
		}
	}

	// add index columns for unsorted tuple (time stamp, pulse count)
	std::vector<TTimeStamp> vTimeStamp = *df.Take<TTimeStamp>("TimeStamp");
	std::vector<ULong64_t> vPulseCount = *df.Take<ULong64_t>("PulseCount");
	fCategoryTimeStamp.emplace(treeName + ".TimeStamp", vTimeStamp);
	fCategoryPulseCount.emplace(treeName + ".PulseCount", vPulseCount);

}


#ifndef XBOX_NO_NAMESPACE
}
#endif


