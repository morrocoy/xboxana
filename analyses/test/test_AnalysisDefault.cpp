#include <iostream>
#include <ctime>

#include "Rtypes.h"
#include "TTimeStamp.h"

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RDFHelpers.hxx"

#include "ROOT/TProcessExecutor.hxx"
#include "ROOT/TThreadExecutor.hxx"
#include "ROOT/TSeq.hxx"

#include "XboxFileSystem.h"
#include "XboxDAQChannel.hxx"
#include "XboxAnalyserEvalBase.hxx"
#include "XboxAnalyserEvalJitter.hxx"
#include "XboxAnalyserEvalPulseShape.hxx"
#include "XboxAnalyserEvalRisingEdge.hxx"
#include "XboxAnalyserEvalDeviation.hxx"
#include "XboxAnalyserEvalSignal.hxx"


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
void analysePulseShape(const std::string &dstFilePath, const std::string &dstTreeName,
		const std::string &srcFilePath,  const std::string &srcTreeName,
		const std::string &filter="1==1") {

    // load data frame
    ROOT::RDataFrame df(srcTreeName.c_str(), srcFilePath.c_str(), {"PSI_amp"});

    // filter breakdowns by time stamp, breakdown flags, signals, ...
    auto dfFilt = df.Filter(filter);

    // evaluate normal pulse and breakdown events
    XBOX::XboxAnalyserEvalPulseShape evalPulseSimple(0.01, 0.99, 0.9);

    auto dfEval = dfFilt
    					// index information
						.Define("TimeStamp", "PSI_amp.getTimeStamp()")
						.Define("PulseCount", "PSI_amp.getPulseCount()")

						// evaluate pulse shape
						.Define("buf_ChN0_PSI_amp", evalPulseSimple, {"PSI_amp"})

    					// remove signal data and take only meta data from channel
    					.Define("ChN0_PSI_amp", "buf_ChN0_PSI_amp.cloneMetaData()");

    // get number of entries which passed the filters
	auto pCnt = dfEval.Count();

	// save resluts to disk
	ROOT::RDF::RSnapshotOptions opts;
	opts.fMode = "UPDATE";

	dfEval.Snapshot(dstTreeName, dstFilePath, {
			"TimeStamp",
			"PulseCount",
			"ChN0_PSI_amp",
	}, opts);

	// print number of entities which passed all filters
	printf("%s: %llu events\n", dstTreeName.c_str(), *pCnt);
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
/// \param[in] dstFilePath The destination file path.
/// \param[in] dstTreeName The branch name under which the results are stored.
/// \param[in] srcFilePath The source file path.
/// \param[in] srcTreeName1 First branch name under from which the data are loaded.
/// \param[in] srcTreeName2 Second branch name under from which the data are loaded.
/// \param[in] filter The filter string.
void analyseBreakdown(const std::string &dstFilePath, const std::string &dstTreeName,
        const std::string &srcFilePath,  const std::string &srcTreeName1,
        const std::string &srcTreeName2, const std::string &filter="1==1") {

    // create data frame
    TFile file(srcFilePath.c_str());

    // take breakdown events as primary tree
    TTree *srcTree = (TTree*)file.Get(srcTreeName1.c_str());

    // add pulses events before breakdown as friend tree
    srcTree->AddFriend(("B1=" + srcTreeName2).c_str(), srcFilePath.c_str());

    // load data frame
    ROOT::RDataFrame df(*srcTree, {"PSI_amp"});

    // filter breakdowns by time stamp, breakdown flags, signals, ...
    auto dfFilt = df.Filter(filter);

    // evaluate normal pulse and breakdown events
    XBOX::XboxAnalyserEvalPulseShape evalPulseShape90(0.01, 0.99, 0.9);
    XBOX::XboxAnalyserEvalJitter evalJitter(0.01, 0.3, 0.3, 0.001);
    XBOX::XboxAnalyserEvalRisingEdge evalRisingEdge(0.01, 0.99, 0.6, 0.03);
    XBOX::XboxAnalyserEvalDeviation evalDeviation(0.01, 0.99, 0.1, 0.02, 0.1);

    auto dfEval = dfFilt
						// index information
						.Define("TimeStamp", "PSI_amp.getTimeStamp()")
						.Define("PulseCount", "PSI_amp.getPulseCount()")

						// refined pulse shape analysis of structure signals
						.Define("buf_ChB1_PSI_amp", evalPulseShape90, {"B1.PSI_amp"})
						.Define("buf_ChB1_PEI_amp", evalPulseShape90, {"B1.PEI_amp"})
						.Define("buf_ChB1_PSR_amp", evalPulseShape90, {"B1.PSR_amp"})

						.Define("buf_ChB0_PSI_amp", evalPulseShape90, {"PSI_amp"})
						.Define("buf_ChB0_PEI_amp", evalPulseShape90, {"PEI_amp"})
						.Define("buf_ChB0_PSR_amp", evalPulseShape90, {"PSR_amp"})

    					// remove signal data and take only meta data from channel
						.Define("ChB1_PSI_amp", "buf_ChB1_PSI_amp.cloneMetaData()")
						.Define("ChB1_PEI_amp", "buf_ChB1_PEI_amp.cloneMetaData()")
						.Define("ChB1_PSR_amp", "buf_ChB1_PSR_amp.cloneMetaData()")

						.Define("ChB0_PSI_amp", "buf_ChB0_PSI_amp.cloneMetaData()")
						.Define("ChB0_PEI_amp", "buf_ChB0_PEI_amp.cloneMetaData()")
						.Define("ChB0_PSR_amp", "buf_ChB0_PSR_amp.cloneMetaData()")

						// jitter of the structure signals
						.Define("Jitter_PSI_amp", evalJitter, {"PSI_amp", "B1.PSI_amp"})

						// time where the rising edges start of the structure signals
						.Define("RefTime_PSI_amp", evalRisingEdge, {"B1.PSI_amp"})
						.Define("RefTime_PEI_amp", evalRisingEdge, {"B1.PEI_amp"})
						.Define("RefTime_PSR_amp", evalRisingEdge, {"B1.PSR_amp"})

						// time at which structure signals between start to deviate breakdown and previous event
						.Define("DevTime_PSI_amp", evalDeviation, {"PSI_amp", "B1.PSI_amp", "Jitter_PSI_amp"})
						.Define("DevTime_PEI_amp", evalDeviation, {"PEI_amp", "B1.PEI_amp", "Jitter_PSI_amp"})
						.Define("DevTime_PSR_amp", evalDeviation, {"PSR_amp", "B1.PSR_amp", "Jitter_PSI_amp"})

						// breakdown location
						.Define("BDTime",
								"(DevTime_PSR_amp - RefTime_PSR_amp - DevTime_PEI_amp + RefTime_PEI_amp)/2");

    // get number of entries which passed the filters
    auto pCnt = dfFilt.Count();

	// save resluts to disk
	ROOT::RDF::RSnapshotOptions opts;
	opts.fMode = "UPDATE";

	dfEval.Snapshot(dstTreeName, dstFilePath, {

		// index information
		"TimeStamp",
		"PulseCount",

		// pulse shape analysis of structure signals
		"ChB1_PSI_amp",
		"ChB1_PEI_amp",
		"ChB1_PSR_amp",

		"ChB0_PSI_amp",
		"ChB0_PEI_amp",
		"ChB0_PSR_amp",

		// jitter of the structure signals
		"Jitter_PSI_amp",

		// time where the rising edges start of the structure signals
		"RefTime_PSI_amp",
		"RefTime_PEI_amp",
		"RefTime_PSR_amp",

		// time at which structure signals start to deviate between breakdown and previous event
		"DevTime_PSI_amp",
		"DevTime_PEI_amp",
		"DevTime_PSR_amp",

		// breakdown location
		"BDTime",
	}, opts);

    // print number of entities which passed all filters
    printf("%s: %llu events\n", dstTreeName.c_str(), *pCnt);
}


int main(int argc, char* argv[]) {

	gInterpreter->Declare("#include \"XboxDAQChannel.hxx\"");

//	ROOT::EnableImplicitMT(); // Tell ROOT you want to go parallel

	std::string srcDir; // Directory of input files.
	std::string dstDir; // Directory of output files.

	std::string srcFilePath; // Input filepath.
	std::string dstFilePath; // Output filepath including filename and type.
	std::string dstSuffix;

	std::vector<std::string> srcExistingFilePaths; // list of existing files in a source directory
	std::vector<std::string> dstExistingFilePaths; // list of existing files in a destination directory

	std::string filtBDStruct; // Filter string. Defines the logic to select specific events.
	std::string filtBDPCompr; // Filter string. Defines the logic to select specific events.

	if (argc != 3) {
		printf("Usage: test_AnalysisDefault srcdir dstdir \n");
		return 1;
	}
	srcDir = argv[1];
	dstDir = argv[2];
	dstSuffix = "_Default";

	filtBDStruct =
	    " (!PERA_amp.getBreakdownFlag())"
	    " && (PSR_amp.getBreakdownFlag()"
	        " || DC_DOWN.getBreakdownFlag()"
	        " || DC_UP.getBreakdownFlag())"
	    " && (DC_UP.span()/B1.DC_UP.span() > 2."
	        " || DC_DOWN.span()/B1.DC_DOWN.span() > 2.)";

	filtBDPCompr =
	    "PKRA_amp.getBreakdownFlag()"
	    " && PKRB_amp.getBreakdownFlag()"
	    " && PLRA_amp.getBreakdownFlag()"
	    " && !(PSR_amp.getBreakdownFlag()"
	        " || PERA_amp.getBreakdownFlag()"
	        " || PERB_amp.getBreakdownFlag()"
	        " || DC_DOWN.getBreakdownFlag()"
	        " || DC_UP.getBreakdownFlag())";

	clock_t begin = clock();

	srcExistingFilePaths = XBOX::getListOfFiles(srcDir + "*.root");
	dstExistingFilePaths = XBOX::getListOfFiles(dstDir + "*" + dstSuffix + ".root");

//	for (std::string filepath : srcExistingFilePaths) {
//	    srcFilePath = filepath;
//	    dstFilePath = dstDir + XBOX::getFileName(filepath);
//	    dstFilePath.insert(dstFilePath.rfind('.'), dstSuffix);
//
//	    // skip if destination file exist already but overwrite last existing file in the destination folder
//		if(!dstFilePath.empty()
//				&& XBOX::accessFile(dstFilePath)
//				&& dstFilePath.compare(dstExistingFilePaths.back())) {
//			printf("INFO: File \'%s\' already accessible. Skip evaluation!\n", dstFilePath.c_str());
//			continue;
//		}
//
//	    printf("Process file: %s ...\n", srcFilePath.c_str());
//	    TFile(dstFilePath.c_str(), "RECREATE"); // delete if file exists
//	    analysePulseShape(dstFilePath, "N0Events", srcFilePath, "N0Events");
//	    analyseBreakdown(dstFilePath, "BDStruct", srcFilePath, "B0Events",
//	    		"B1Events", filtBDStruct);
//	    analyseBreakdown(dstFilePath, "BDPCompr", srcFilePath, "B0Events",
//	    		"B1Events", filtBDPCompr);
//	}


	// The number of workers
	const UInt_t nThreads = 8U;

	ROOT::EnableThreadSafety();


	auto workItem = [srcExistingFilePaths, dstExistingFilePaths, dstDir](UInt_t workerID) {

		std::string dstSuffix = "_Default";

		std::string filtBDStruct =
			    " (!PERA_amp.getBreakdownFlag())"
			    " && (PSR_amp.getBreakdownFlag()"
			        " || DC_DOWN.getBreakdownFlag()"
			        " || DC_UP.getBreakdownFlag())"
			    " && (DC_UP.span()/B1.DC_UP.span() > 2."
			        " || DC_DOWN.span()/B1.DC_DOWN.span() > 2.)";

		std::string filtBDPCompr =
			"PKRA_amp.getBreakdownFlag()"
			" && PKRB_amp.getBreakdownFlag()"
			" && PLRA_amp.getBreakdownFlag()"
			" && !(PSR_amp.getBreakdownFlag()"
				" || PERA_amp.getBreakdownFlag()"
				" || PERB_amp.getBreakdownFlag()"
				" || DC_DOWN.getBreakdownFlag()"
				" || DC_UP.getBreakdownFlag())";

		std::string srcFilePath = srcExistingFilePaths[workerID];
		std::string dstFilePath = dstDir + XBOX::getFileName(srcFilePath);
		dstFilePath.insert(dstFilePath.rfind('.'), dstSuffix);

		// skip if destination file exist already but overwrite last existing file in the destination folder
		if(!dstFilePath.empty()
				&& XBOX::accessFile(dstFilePath)
				&& dstFilePath.compare(dstExistingFilePaths.back())) {
			printf("INFO: File \'%s\' already accessible. Skip evaluation!\n", dstFilePath.c_str());
			return 0;
		}

		printf("Process file: %s ...\n", srcFilePath.c_str());
	    TFile(dstFilePath.c_str(), "RECREATE"); // delete if file exists
	    analysePulseShape(dstFilePath, "N0Events", srcFilePath, "N0Events");
	    analyseBreakdown(dstFilePath, "BDStruct", srcFilePath, "B0Events",
	    		"B1Events", filtBDStruct);
	    analyseBreakdown(dstFilePath, "BDPCompr", srcFilePath, "B0Events",
	    		"B1Events", filtBDPCompr);
		return 0;
	};

	// Create the pool of threads
	ROOT::TProcessExecutor pool(nThreads);

	// Fill the pool with work
	pool.Map(workItem, ROOT::TSeqI(srcExistingFilePaths.size()));

	clock_t end = clock();
	printf("Total elapsed time: %.3f\n", double(end - begin) / CLOCKS_PER_SEC);

	return 0;
}
