#include <iostream>
#include <ctime>

#include "Rtypes.h"
#include "TTimeStamp.h"
#include "TRegexp.h"
#include "TSystemFile.h"
#include "TSystemDirectory.h"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RDFHelpers.hxx"

#include "ROOT/TProcessExecutor.hxx"
#include "ROOT/TThreadExecutor.hxx"
#include "ROOT/TSeq.hxx"

#include "XboxDAQChannel.hxx"
#include "XboxAnalyserEvalBase.hxx"
#include "XboxAnalyserEvalJitter.hxx"
#include "XboxAnalyserEvalPulseSimple.hxx"
#include "XboxAnalyserEvalPulseRefined.hxx"
#include "XboxAnalyserEvalDeviation.hxx"
#include "XboxAnalyserEvalSignal.hxx"


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

    std::sort(vListOfFiles.begin(), vListOfFiles.end());
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
    XBOX::XboxAnalyserEvalPulseSimple evalPulseSimple(0.01, 0.99, 0.9);

    auto dfEval = dfFilt.Define("_eval_PSI_amp", evalPulseSimple, {"PSI_amp"});

    // get number of entries which passed the filters
    auto pCnt = dfFilt.Count();

    // get results from data frame.
    auto pEvalPSIamp = dfEval.Take<XBOX::XboxDAQChannel>("_eval_PSI_amp");

    // print time stamp and pulse count for each entry
//	dfEval.Foreach(
//			[](XBOX::XboxDAQChannel &entry) {
//				printf("%s | fPulseCount: %llu\n",
//						entry.getTimeStamp().AsString(), entry.getPulseCount());
//			}, {"_evalPSI"});

    // transfer data frame to std::vector
    std::vector<XBOX::XboxDAQChannel> vEvalPSIamp = *pEvalPSIamp;

    // sorting according to the time stamp (in-built of XBOX::XboxAnalyserBaseModel)
//    std::sort(vEvalPSIamp.begin(), vEvalPSIamp.end());


    // print number of entities which passed all filters
    printf("%s: %llu events\n", sDstKey.c_str(), *pCnt);

    // export results
    TFile fileResults(sDstFilePath.c_str(), "UPDATE");
    TTree *pDstTree = new TTree(sDstKey.c_str(), sDstKey.c_str());

    TTimeStamp ts;
    ULong64_t pcnt;
    XBOX::XboxDAQChannel evalPSIamp;


    pDstTree->Branch("TimeStamp", &ts, 16000, 99);
    pDstTree->Branch("PulseCount", &pcnt, 16000, 99);
    pDstTree->Branch("PSI_amp", &evalPSIamp, 16000, 99);

    for (size_t i=0; i < vEvalPSIamp.size(); i++) {
        ts = vEvalPSIamp[i].getTimeStamp();
        pcnt = vEvalPSIamp[i].getPulseCount();
        evalPSIamp = vEvalPSIamp[i];
        evalPSIamp.clear(); // remove signal data to reduce file size
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

//    // evaluate normal pulse and breakdown events
//    // pulse shape analysis (PSI)
//    XBOX::XboxAnalyserEvalPulseSimple evalPulseSimple90(0.01, 0.99, 0.9); // for PSI
//    XBOX::XboxAnalyserEvalPulseSimple evalPulseSimple50(0.01, 0.99, 0.5); // for DC
//
//    XBOX::XboxAnalyserEvalSignal evalSignal;
//
//    XBOX::XboxAnalyserEvalJitter evalJitter(0.01, 0.3, 0.3, 0.001);
//    XBOX::XboxAnalyserEvalPulseRefined evalPulseRefined(0.01, 0.99, 0.9);
//    XBOX::XboxAnalyserEvalDeviation evalDeviation(0.01, 0.99, 0.9);
//
//    evalPulseRefined.configRefined(0.01, 0.99, 0.6, 0.03);
//    evalDeviation.configDeviation(0.01, 0.99, 0.1, 0.02, 0.1);
//
//        auto dfEval = dfFilt.Define("_eval_B1_PSI_amp", evalPulseSimple90, {"B1.PSI_amp"})
//                            .Define("_eval_PSI_amp", evalPulseSimple90, {"PSI_amp"})
//                            .Define("_eval_tr", "_eval_PSI_amp.getXmin()")
//                            .Define("_eval_tf", "_eval_PSI_amp.getXmax()")
//                            .Define("_eval_B1PKIA_amp", evalSignal, {"B1.PKIA_amp", "_eval_tr", "_eval_tf"})
//                            .Define("_eval_B1PKIB_amp", evalSignal, {"B1.PKIB_amp", "_eval_tr", "_eval_tf"})
//                            .Define("_eval_PKIA_amp", evalSignal, {"PKIA_amp", "_eval_tr", "_eval_tf"})
//                            .Define("_eval_PKIB_amp", evalSignal, {"PKIB_amp", "_eval_tr", "_eval_tf"})
//                            .Define("_eval_Jitter", evalJitter, {"PSI_amp", "B1.PSI_amp"})
//                            .Define("_eval_B1_PEI_amp", evalPulseRefined, {"B1.PEI_amp"})
//                            .Define("_eval_B1_PSR_amp", evalPulseRefined, {"B1.PSR_amp"})
//                            .Define("_eval_PEI_amp", evalDeviation, {"PEI_amp", "B1.PEI_amp", "_eval_Jitter"})
//                            .Define("_eval_PSR_amp", evalDeviation, {"PSR_amp", "B1.PSR_amp", "_eval_Jitter"})
//                            .Define("_eval_DC_UP", evalPulseSimple50, {"DC_UP"})
//                            .Define("_eval_DC_DOWN", evalPulseSimple50, {"DC_DOWN"});

    // evaluate normal pulse and breakdown events
    // pulse shape analysis (PSI)
    XBOX::XboxAnalyserEvalPulseSimple evalPulseSimple90(0.01, 0.99, 0.9); // for PSI

    XBOX::XboxAnalyserEvalJitter evalJitter(0.01, 0.3, 0.3, 0.001);
    XBOX::XboxAnalyserEvalPulseRefined evalPulseRefined(0.01, 0.99, 0.9);
    XBOX::XboxAnalyserEvalDeviation evalDeviation(0.01, 0.99, 0.9);

    evalPulseRefined.configRefined(0.01, 0.99, 0.6, 0.03);
    evalDeviation.configDeviation(0.01, 0.99, 0.1, 0.02, 0.1);

    auto dfEval = dfFilt
    					// index information
						.Define("TimeStamp", "PSI_amp.getTimeStamp()")
						.Define("PulseCount", "PSI_amp.getPulseCount()")

						// jitter evaluation
						.Define("eval_Jitter", evalJitter, {"PSI_amp", "B1.PSI_amp"})

						// refined pulse shape analysis of structure signals
						.Define("eval_B1_PSI_amp", evalPulseRefined, {"PSI_amp"})
						.Define("eval_B1_PEI_amp", evalPulseRefined, {"B1.PEI_amp"})
						.Define("eval_B1_PSR_amp", evalPulseRefined, {"B1.PSR_amp"})

						// deviation between breakdown and previous event
						.Define("eval_PSI_amp", evalDeviation, {"PEI_amp", "B1.PEI_amp", "eval_Jitter"})
						.Define("eval_PEI_amp", evalDeviation, {"PEI_amp", "B1.PEI_amp", "eval_Jitter"})
						.Define("eval_PSR_amp", evalDeviation, {"PSR_amp", "B1.PSR_amp", "eval_Jitter"});

    // get number of entries which passed the filters
    auto pCnt = dfFilt.Count();

	// save resluts to disk
	ROOT::RDF::RSnapshotOptions opts;
	opts.fMode = "UPDATE";

	dfEval.Snapshot(sDstKey, sDstFilePath, {
			"TimeStamp",
			"PulseCount",
			"eval_Jitter",
			"eval_B1_PSI_amp",
			"eval_B1_PEI_amp",
			"eval_B1_PSR_amp",
			"eval_PSI_amp",
			"eval_PEI_amp",
			"eval_PSR_amp",
	}, opts);

    // print number of entities which passed all filters
    printf("%s: %llu events\n", sDstKey.c_str(), *pCnt);
}



int main(int argc, char* argv[]) {

	gInterpreter->Declare("#include \"XboxDAQChannel.hxx\"");

//	ROOT::EnableImplicitMT(); // Tell ROOT you want to go parallel

	std::string sSrcDirectory; // Directory of input files.
	std::string sDstDirectory; // Directory of output files.
	std::vector<std::string> vListOfFiles; // list of filesnames in a directory
	std::string sDstSuffix;

	std::string sSrcFilePath; // Input filepath.
	std::string sDstFilePath; // Output filepath including filename and type.

	std::string sFilterBDStruct; // Filter string. Defines the logic to select specific events.
	std::string sFilterBDPCompr; // Filter string. Defines the logic to select specific events.

	if (argc != 3) {
		printf("Usage: test_AnalysisDefault srcdir dstdir \n");
		return 1;
	}
	sSrcDirectory = argv[1];
	sDstDirectory = argv[2];
	sDstSuffix = "_Default";

	sFilterBDStruct =
	    " (!PERA_amp.getBreakdownFlag())"
	    " && (PSR_amp.getBreakdownFlag()"
	        " || DC_DOWN.getBreakdownFlag()"
	        " || DC_UP.getBreakdownFlag())"
	    " && (DC_UP.span()/B1.DC_UP.span() > 2."
	        " || DC_DOWN.span()/B1.DC_DOWN.span() > 2.)";

	sFilterBDPCompr =
	    "PKRA_amp.getBreakdownFlag()"
	    " && PKRB_amp.getBreakdownFlag()"
	    " && PLRA_amp.getBreakdownFlag()"
	    " && !(PSR_amp.getBreakdownFlag()"
	        " || PERA_amp.getBreakdownFlag()"
	        " || PERB_amp.getBreakdownFlag()"
	        " || DC_DOWN.getBreakdownFlag()"
	        " || DC_UP.getBreakdownFlag())";

	clock_t begin = clock();


	vListOfFiles = getListOfFiles(sSrcDirectory+"*.root");

//	for (std::string sFilePath : vListOfFiles) {
//	    sSrcFilePath = sFilePath;
//	    sDstFilePath = sDstDirectory + getFileNameOf(sFilePath);
//	    sDstFilePath.insert(sDstFilePath.rfind('.'), sDstSuffix);
//
//	    printf("Process file: %s ...\n", sSrcFilePath.c_str());
//	    TFile(sDstFilePath.c_str(), "RECREATE"); // delete if file exists
//	    analysePulseShape(sDstFilePath, "N0Events", sSrcFilePath, "N0Events");
//	    analyseBreakdown(sDstFilePath, "BDStruct", sSrcFilePath, "B0Events",
//	    		"B1Events", sFilterBDStruct);
////	    analyseBreakdown(sDstFilePath, "BDPCompr", sSrcFilePath, "B0Events",
////	    		"B1Events", sFilterBDPCompr);
//	}

	// The number of workers
	const UInt_t nThreads = 8U;

	ROOT::EnableThreadSafety();


	auto workItem = [vListOfFiles, sDstDirectory](UInt_t workerID) {


		std::string sDstSuffix = "_Default";

		std::string sFilterBDStruct =
		    " (!PERA_amp.getBreakdownFlag())"
		    " && (PSR_amp.getBreakdownFlag()"
		        " || DC_DOWN.getBreakdownFlag()"
		        " || DC_UP.getBreakdownFlag())"
		    " && (DC_UP.span()/B1.DC_UP.span() > 2."
		        " || DC_DOWN.span()/B1.DC_DOWN.span() > 2.)";

		std::string sFilterBDPCompr =
		    "PKRA_amp.getBreakdownFlag()"
		    " && PKRB_amp.getBreakdownFlag()"
		    " && PLRA_amp.getBreakdownFlag()"
		    " && !(PSR_amp.getBreakdownFlag()"
		        " || PERA_amp.getBreakdownFlag()"
		        " || PERB_amp.getBreakdownFlag()"
		        " || DC_DOWN.getBreakdownFlag()"
		        " || DC_UP.getBreakdownFlag())";

		std::string sFilePath = vListOfFiles[workerID];
		std::string sSrcFilePath = sFilePath;
		std::string sDstFilePath = sDstDirectory + getFileName(sFilePath);
	    sDstFilePath.insert(sDstFilePath.rfind('.'), sDstSuffix);

	    printf("Process file: %s ...\n", sSrcFilePath.c_str());
	    TFile(sDstFilePath.c_str(), "RECREATE"); // delete if file exists
	    analysePulseShape(sDstFilePath, "N0Events", sSrcFilePath, "N0Events");
	    analyseBreakdown(sDstFilePath, "BDStruct", sSrcFilePath, "B0Events",
	    		"B1Events", sFilterBDStruct);
	    analyseBreakdown(sDstFilePath, "BDPCompr", sSrcFilePath, "B0Events",
	    		"B1Events", sFilterBDPCompr);
	    return 0;
	};

	// Create the pool of threads
//	ROOT::TThreadExecutor pool(nThreads);
	ROOT::TProcessExecutor pool(nThreads);

	// Fill the pool with work
	pool.Map(workItem, ROOT::TSeqI(vListOfFiles.size()));

	clock_t end = clock();
	printf("Total elapsed time: %.3f\n", double(end - begin) / CLOCKS_PER_SEC);

	return 0;
}
