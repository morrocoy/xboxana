#include <iostream>
#include <ctime>

#include "Rtypes.h"
#include "TTimeStamp.h"
#include "TRegexp.h"
#include "TSystemFile.h"
#include "TSystemDirectory.h"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RDFHelpers.hxx"

#include "XboxAnalyserResult.hxx"
#include "XboxAnalyserEvalBase.hxx"
#include "XboxAnalyserEvalPulse.hxx"
#include "XboxAnalyserEvalBreakdown.hxx"
#include "XboxAnalyserEvalBreakdownTest.hxx"
#include "XboxAnalyserView0.hxx"


////////////////////////////////////////////////////////////////////////////////
/// File operation.
/// Get the the directory of a path.
std::string getDirectory(const std::string &sFilePathGlob) {
     size_t pos = sFilePathGlob.find_last_of("\\/");
     return (std::string::npos == pos)
         ? ""
         : sFilePathGlob.substr(0, pos+1);
}

////////////////////////////////////////////////////////////////////////////////
/// File operation.
/// Extract the file name of a path. Regular expressions are allowed.
std::string getFileName(const std::string &sFilePathGlob) {
     size_t pos = sFilePathGlob.find_last_of("\\/");
     return (std::string::npos == pos)
         ? ""
         : sFilePathGlob.substr(pos+1);
}

////////////////////////////////////////////////////////////////////////////////
/// File operation.
/// Get the list of files in directory a given a regular expression.
std::vector<std::string> getListOfFiles(const string &sFilePathGlob) {

    std::string sDirectory = getDirectory(sFilePathGlob);
    std::string sFileNameGlob = getFileName(sFilePathGlob);

    TSystemDirectory dir(sDirectory.c_str(), sDirectory.c_str());
    TList *files = dir.GetListOfFiles();
    std::vector<std::string> vListOfFiles;

    TRegexp re(TString(sFileNameGlob.c_str()), true);
    if (files) {
        TSystemFile *file;
        TIter next(files);
        while ((file = (TSystemFile*) next())) {
            std::string sFileName = file->GetName();
            TString str = file->GetName();
            if (str.Index(re) != -1)
                vListOfFiles.push_back(sDirectory + "/" + sFileName);

        }
    }
    return vListOfFiles;
}

////////////////////////////////////////////////////////////////////////////////
/// Evaluation of pulse shape.
/// The data evaluation is based on RDataFrame. All results of one event are
/// returned by the evaluator functor in a single object. The results are
/// extracted from the resulting dataframe and saved in a separate file for
/// later use and representation. The data are taken from a single tree. Hence,
/// no comparison between subsequent events is considered. Typical application
/// is the evaluation of pulse shape parameters associated with normal pulses.
///   1. extracts meta (timestamp, pulse count).
///   2. evaluates pulse shape parameters (averaged and maximum power, rising
///      and falling edge, pulse length).
/// \param[in] sDstFilePath The destination file path.
/// \param[in] sDstKey The branch name under which the results are stored.
/// \param[in] sSrcFilePath The source file path.
/// \param[in] sSrcKey The branch name under from which the data are loaded.
/// \param[in] sfilter The filter string.
void analysePulseShape(const std::string &sDstFilePath, const std::string &sDstKey,
		const std::string &sSrcFilePath,  const std::string &sSrcKey,
		const std::string &sfilter="1==1") {

    // load data frame
    ROOT::RDataFrame df(sSrcKey.c_str(), sSrcFilePath.c_str(), {"PSI_amp"});

    // filter breakdowns by time stamp, breakdown flags, signals, ...
    auto dfFilt = df.Filter(sfilter);

    // evaluate normal pulse and breakdown events
    XBOX::XboxAnalyserEvalPulse evaluator;
    evaluator.setPulseConfig(0.01, 0.99, 0.9);

    auto dfEval = dfFilt.Define("_model", evaluator, {"PSI_amp"});

    // get number of entries which passed the filters
    auto pCnt = dfFilt.Count();

    // get results from data frame.
    auto pEntries = dfEval.Take<XBOX::XboxAnalyserResult>("_model");

    // print time stamp and pulse count for each entry
	dfEval.Foreach(
			[](XBOX::XboxAnalyserResult &entry) {
				printf("%s | fPulseCount: %llu\n",
						entry.getTimeStamp().AsString(), entry.getPulseCount());
			}, {"_model"});

    // transfer data frame to std::vector
    std::vector<XBOX::XboxAnalyserResult> vEntries = *pEntries;

    // sorting according to the time stamp (in-built of XBOX::XboxAnalyserBaseModel)
    std::sort(vEntries.begin(), vEntries.end());

    // print number of entities which passed all filters
    printf("%s: %llu events\n", sDstKey.c_str(), *pCnt);

    // export results
    TFile fileResults(sDstFilePath.c_str(), "UPDATE");
    TTree *pDstTree = new TTree(sDstKey.c_str(), sDstKey.c_str());

    TTimeStamp ts;
    ULong64_t pcnt;
    XBOX::XboxAnalyserResult entry;

    pDstTree->Branch("fTimeStamp", &ts, 16000, 99);
    pDstTree->Branch("fPulseCount", &pcnt, 16000, 99);
    pDstTree->Branch("fEntry", &entry, 16000, 99);

    for (XBOX::XboxAnalyserResult e: vEntries) {
        entry = e;
        ts = e.getTimeStamp();
        pcnt = e.getPulseCount();
        pDstTree->Fill();
    }
    pDstTree->Write();
    delete pDstTree;
}

////////////////////////////////////////////////////////////////////////////////
/// Evaluation of pulse shape.
/// The data evaluation is based on RDataFrame. All results of one event are
/// returned by the evaluator functor in a single object. The results are
/// extracted from the resulting dataframe and saved in a separate file for
/// later use and representation. The data are taken from a two matched tree's.
/// Typical application is the evaluation of breakdowns where the first tree is
/// associated with the actual breakdown events and the second tree is
/// associated with the events right before the breakdowns.
///   1. extracts meta (timestamp, pulse count).
///   2. evaluates pulse shape parameters of the event before the breakdown
///      (averaged and maximum power, rising and falling edge, pulse length).
///   3. evaluates breakdown position and phase
/// \param[in] sDstFilePath The destination file path.
/// \param[in] sDstKey The branch name under which the results are stored.
/// \param[in] sSrcFilePath The source file path.
/// \param[in] sSrcKey1 First branch name under from which the data are loaded.
/// \param[in] sSrcKey2 Second branch name under from which the data are loaded.
/// \param[in] sfilter The filter string.
void analyseBreakdown(const std::string &sDstFilePath, const std::string &sDstKey,
		const std::string &sSrcFilePath,  const std::string &sSrcKey1,
		const std::string &sSrcKey2, const std::string &sfilter="1==1") {

    // create data frame
    TFile fileEvents(sSrcFilePath.c_str());

    // take breakdown events as primary tree
    TTree *pSrcTree = (TTree*)fileEvents.Get(sSrcKey1.c_str());

    // add pulses events before breakdown as friend tree
    pSrcTree->AddFriend(("B1=" + sSrcKey2).c_str(), sSrcFilePath.c_str());

    // load data frame
    ROOT::RDataFrame df(*pSrcTree, {"PSI_amp"});

    // filter breakdowns by time stamp, breakdown flags, signals, ...
    auto dfFilt = df.Filter(sfilter);

    // evaluate normal pulse and breakdown events
    XBOX::XboxAnalyserEvalBreakdown evaluator;

    evaluator.setPulseConfig(0.01, 0.99, 0.9);
    evaluator.setJitterConfig(0.01, 0.3, 0.3, 0.001);
    evaluator.setRiseConfig(0.01, 0.99, 0.6, 0.03);
    evaluator.setDeflConfig(0.01, 0.99, 0.1, 0.02, 0.1);

    auto dfEval = dfFilt.Define("_model", evaluator,
        {"PSI_amp","B1.PSI_amp", "PEI1_amp", "B1.PEI1_amp", "PSR_amp", "B1.PSR_amp"});

    // get number of entries which passed the filters
    auto pCnt = dfFilt.Count();

    // get results from data frame.
    auto pEntries = dfEval.Take<XBOX::XboxAnalyserResult>("_model");

    // print time stamp and pulse count for each entry
	dfEval.Foreach(
			[](XBOX::XboxAnalyserResult &entry) {
				printf("%s | fPulseCount: %llu\n",
						entry.getTimeStamp().AsString(), entry.getPulseCount());
			}, {"_model"});

    // transfer data frame to std::vector
    std::vector<XBOX::XboxAnalyserResult> vEntries = *pEntries;

    // sorting according to the time stamp (in-built of XBOX::XboxAnalyserBaseModel)
    std::sort(vEntries.begin(), vEntries.end());

    // print number of entities which passed all filters
    printf("%s: %llu events\n", sDstKey.c_str(), *pCnt);

    // export results
    TFile fileResults(sDstFilePath.c_str(), "UPDATE");
    TTree *pDstTree = new TTree(sDstKey.c_str(), sDstKey.c_str());

    TTimeStamp ts;
    ULong64_t pcnt;
    XBOX::XboxAnalyserResult entry;

    pDstTree->Branch("fTimeStamp", &ts, 16000, 99);
    pDstTree->Branch("fPulseCount", &pcnt, 16000, 99);
    pDstTree->Branch("fEntry", &entry, 16000, 99);

    for (XBOX::XboxAnalyserResult e: vEntries) {
        entry = e;
        ts = e.getTimeStamp();
        pcnt = e.getPulseCount();
        pDstTree->Fill();
    }
    pDstTree->Write();
    delete pDstTree;
}


int main(int argc, char* argv[]) {

	gInterpreter->Declare("#include \"XboxDAQChannel.hxx\"");

	ROOT::EnableImplicitMT(); // Tell ROOT you want to go parallel

	std::string sSrcDirectory; // Directory of input files.
	std::string sDstDirectory; // Directory of output files.
	std::vector<std::string> vListOfFiles; // list of filesnames in a directory
	std::string sDstSuffix;

	std::string sSrcFilePath; // Input filepath.
	std::string sDstFilePath; // Output filepath including filename and type.

	std::string sFilterBDStruct; // Filter string. Defines the logic to select specific events.
	std::string sFilterBDPShift; // Filter string. Defines the logic to select specific events.
	std::vector<XBOX::XboxAnalyserResult> vEntries; // results of the analyses of filtered events.

	if (argc != 3) {
		printf("Usage: test_AnalysisDefault srcdir dstdir \n");
		return 1;
	}
	sSrcDirectory = argv[1];
	sDstDirectory = argv[2];
	sDstSuffix = "_Default";

//	sFilterBDStruct =
//	    " (!PERA_amp.getBreakdownFlag())"
//	    " && (PSR_amp.getBreakdownFlag()"
//	        " || DC_DOWN.getBreakdownFlag()"
//	        " || DC_UP.getBreakdownFlag())"
//	    " && (DC_UP.span()/B1.DC_UP.span() > 2."
//	        " || DC_DOWN.span()/B1.DC_DOWN.span() > 2.)";

	sFilterBDStruct =
	    " (DC_UP.span()/B1.DC_UP.span() > 2."
	        " || DC_DOWN.span()/B1.DC_DOWN.span() > 2.)";

	sFilterBDPShift =
	    "PKRA_amp.getBreakdownFlag()"
	    " && PLRA_amp.getBreakdownFlag()"
	    " && !(PSR_amp.getBreakdownFlag()"
	        " || PERA_amp.getBreakdownFlag()"
	        " || PERB_amp.getBreakdownFlag()"
	        " || DC_DOWN.getBreakdownFlag()"
	        " || DC_UP.getBreakdownFlag())";

	clock_t begin = clock();


	vListOfFiles = getListOfFiles(sSrcDirectory+"*.root");
	for (std::string sFilePath : vListOfFiles) {
	    sSrcFilePath = sFilePath;
	    sDstFilePath = sDstDirectory + getFileName(sFilePath);
	    sDstFilePath.insert(sDstFilePath.rfind('.'), sDstSuffix);

	    printf("Process file: %s ...\n", sSrcFilePath.c_str());
	    TFile(sDstFilePath.c_str(), "RECREATE"); // delete if file exists
	    analysePulseShape(sDstFilePath, "N0Events", sSrcFilePath, "N0Events");
	    analyseBreakdown(sDstFilePath, "BDStruct", sSrcFilePath, "B0Events",
	    		"B1Events", sFilterBDStruct);
//	    analyseEventsJoined(sDstFilePath, "BDPCompr", sSrcFilePath, "B0Events",
//	    		"B1Events", sFilterBDPShift);
	}

	clock_t end = clock();
	printf("Total elapsed time: %.3f\n", double(end - begin) / CLOCKS_PER_SEC);

	return 0;
}
