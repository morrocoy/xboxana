#include <iostream>
#include <ctime>

#include "Rtypes.h"
#include "TTimeStamp.h"
#include "TRegexp.h"
#include "TSystemFile.h"
#include "TSystemDirectory.h"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RDFHelpers.hxx"

#include "XboxDAQChannel.hxx"
#include "XboxAnalyserEvalBase.hxx"
#include "XboxAnalyserEvalJitter.hxx"
#include "XboxAnalyserEvalPulseShape.hxx"
#include "XboxAnalyserEvalRisingEdge.hxx"
#include "XboxAnalyserEvalDeviation.hxx"

// root graphics
#include "TStyle.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TLine.h"
#include "TLegend.h"
#include "TPad.h"
#include "TGraph.h"
#include "TColor.h"
#include "TGaxis.h"
#include "TGraphErrors.h"
#include "TFrame.h"

//#include "TApplication.h"


////////////////////////////////////////////////////////////////////////
/// Test plot.
void plotSignals(XBOX::XboxDAQChannel &ch0, XBOX::XboxDAQChannel &ch1) {

	TCanvas c1("c1","c1",200,10,700,500);
	c1.SetGrid();

	ch0.flushbuffer();
	ch1.flushbuffer();

//	Double_t tmin = -1;
//	Double_t tmax = -1;

	Double_t tmin = 0.2e-6;
	Double_t tmax = 0.7e-6;

	std::vector<Double_t> t0 = ch0.getTimeAxis(tmin, tmax);
	std::vector<Double_t> t1 = ch1.getTimeAxis(tmin, tmax);
	std::vector<Double_t> y0 = ch0.getSignal(tmin, tmax);
	std::vector<Double_t> y1 = ch1.getSignal(tmin, tmax);



	TMultiGraph *mg = new TMultiGraph();

	TGraph gr0(t0.size(), &t0[0], &y0[0]);
	TGraph gr1(t1.size(), &t1[0], &y1[0]);

	gr0.SetLineColor(kRed);
	gr1.SetLineColor(kBlue);

	gr0.SetMarkerColor(kRed);
	gr1.SetMarkerColor(kBlue);

	mg->Add(&gr0);
	mg->Add(&gr1);
	mg->Draw("AC");

	std::string ts = ch0.getTimeStamp().AsString();
	std::string name = ch0.getChannelName();
	c1.Print(("pictures_test/" + name + "_" + ts + ".png").c_str());

}

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
    XBOX::XboxAnalyserEvalPulseShape evalPulseSimple(0.01, 0.99, 0.9);

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
    std::sort(vEvalPSIamp.begin(), vEvalPSIamp.end());


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
        evalPSIamp.Clear(); // remove signal data to reduce file size
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
    // pulse shape analysis (PSI)
    XBOX::XboxAnalyserEvalPulseShape evalPulseShape90(0.01, 0.99, 0.9); // for PSI
    XBOX::XboxAnalyserEvalPulseShape evalPulseShape50(0.01, 0.99, 0.5); // for DC

    XBOX::XboxAnalyserEvalJitter evalJitter(0.01, 0.3, 0.3, 0.001);
    XBOX::XboxAnalyserEvalRisingEdge evalRisingEdge(0.01, 0.99, 0.6, 0.03);
    XBOX::XboxAnalyserEvalDeviation evalDeviation(0.01, 0.99, 0.1, 0.02, 0.1);

//	XBOX::XboxAnalyserEvalBreakdown evalBD; // breakdown analysis (PEI, PSR)
//	XBOX::XboxAnalyserEvalPulse evalDC; // analysis of dark current (DC_UP, DC_DOWN)
//	evalPulseSimple90.setPulseConfig(0.01, 0.99, 0.9);
//	evalBD.setJitterConfig(0.01, 0.3, 0.3, 0.001);
//	evalBD.setRiseConfig(0.01, 0.99, 0.6, 0.03);
//	evalBD.setDeflConfig(0.01, 0.99, 0.1, 0.02, 0.1);
//	evalDC.setPulseConfig(0.01, 0.99, 0.5);

	auto dfEval = dfFilt

						// pulse shape analysis of structure signals
						.Define("eval1_PSI_amp", evalPulseShape90, {"B1.PSI_amp"})
						.Define("eval1_PEI_amp", evalPulseShape90, {"B1.PEI_amp"})
						.Define("eval1_PSR_amp", evalPulseShape90, {"B1.PSR_amp"})

						.Define("eval1_B1_PSI_amp", evalPulseShape90, {"B1.PSI_amp"})
						.Define("eval1_B1_PEI_amp", evalPulseShape90, {"B1.PEI_amp"})
						.Define("eval1_B1_PSR_amp", evalPulseShape90, {"B1.PSR_amp"})

						.Define("_eval_DC_UP", evalPulseShape50, {"DC_UP"})
						.Define("_eval_DC_DOWN", evalPulseShape50, {"DC_DOWN"})

						// jitter evaluation
						.Define("Jitter_PSI", evalJitter, {"PSI_amp", "B1.PSI_amp"})

						// time where the rising edges start
						.Define("RefTime_PEI", evalRisingEdge, {"B1.PEI_amp"})
						.Define("RefTime_PSR", evalRisingEdge, {"B1.PSR_amp"})

						// deviation between breakdown and previous event
						.Define("DevTime_PEI", evalDeviation, {"PEI_amp", "B1.PEI_amp", "Jitter_PSI"})
						.Define("DevTime_PSR", evalDeviation, {"PSR_amp", "B1.PSR_amp", "Jitter_PSI"})

						// breakdown location
						.Define("BDTime", "(DevTime_PSR - RefTime_PSR - DevTime_PEI + RefTime_PEI)/2");



	// get number of entries which passed the filters
	auto pCnt = dfFilt.Count();

	// get results from data frame.
	auto pEvalB1PSIamp = dfEval.Take<XBOX::XboxDAQChannel>("_eval_B1_PSI_amp");
	auto pEvalB1PEIamp = dfEval.Take<XBOX::XboxDAQChannel>("_eval_B1_PEI_amp");
	auto pEvalB1PSRamp = dfEval.Take<XBOX::XboxDAQChannel>("_eval_B1_PSR_amp");
	auto pEvalPEIamp = dfEval.Take<XBOX::XboxDAQChannel>("_eval_PEI_amp");
	auto pEvalPSRamp = dfEval.Take<XBOX::XboxDAQChannel>("_eval_PSR_amp");
	auto pEvalDCUp = dfEval.Take<XBOX::XboxDAQChannel>("_eval_DC_UP");
	auto pEvalDCDown = dfEval.Take<XBOX::XboxDAQChannel>("_eval_DC_DOWN");

	// transfer data frame to std::vector
	std::vector<XBOX::XboxDAQChannel> vEvalB1PSIamp = *pEvalB1PSIamp;
	std::vector<XBOX::XboxDAQChannel> vEvalB1PEIamp = *pEvalB1PEIamp;
	std::vector<XBOX::XboxDAQChannel> vEvalB1PSRamp = *pEvalB1PSRamp;
	std::vector<XBOX::XboxDAQChannel> vEvalPEIamp = *pEvalPEIamp;
	std::vector<XBOX::XboxDAQChannel> vEvalPSRamp = *pEvalPSRamp;
	std::vector<XBOX::XboxDAQChannel> vEvalDCUp = *pEvalDCUp;
	std::vector<XBOX::XboxDAQChannel> vEvalDCDown = *pEvalDCDown;

	// sorting according to the time stamp (in-built of XBOX::XboxAnalyserBaseModel)
	std::sort(vEvalB1PSIamp.begin(), vEvalB1PSIamp.end());
	std::sort(vEvalB1PEIamp.begin(), vEvalB1PEIamp.end());
	std::sort(vEvalB1PSRamp.begin(), vEvalB1PSRamp.end());
	std::sort(vEvalPEIamp.begin(), vEvalPEIamp.end());
	std::sort(vEvalPSRamp.begin(), vEvalPSRamp.end());
	std::sort(vEvalDCUp.begin(), vEvalDCUp.end());
	std::sort(vEvalDCDown.begin(), vEvalDCDown.end());

	// print number of entities which passed all filters
	printf("%s: %llu events\n", sDstKey.c_str(), *pCnt);

	// export results
	TFile fileResults(sDstFilePath.c_str(), "UPDATE");
	TTree *pDstTree = new TTree(sDstKey.c_str(), sDstKey.c_str());

	TTimeStamp ts;
	ULong64_t pcnt;
	XBOX::XboxDAQChannel evalPSIamp;
	XBOX::XboxDAQChannel evalPEIamp;
	XBOX::XboxDAQChannel evalPSRamp;
	XBOX::XboxDAQChannel evalB1PEIamp;
	XBOX::XboxDAQChannel evalB1PSRamp;
	XBOX::XboxDAQChannel evalDCUp;
	XBOX::XboxDAQChannel evalDCDown;

	pDstTree->Branch("TimeStamp", &ts, 16000, 99);
	pDstTree->Branch("PulseCount", &pcnt, 16000, 99);
	pDstTree->Branch("PSI_amp", &evalPSIamp, 16000, 99);
	pDstTree->Branch("PEI_amp", &evalPEIamp, 16000, 99);
	pDstTree->Branch("PSI_amp", &evalPSIamp, 16000, 99);
	pDstTree->Branch("B1_PEI_amp", &evalB1PEIamp, 16000, 99);
	pDstTree->Branch("B1_PSR_amp", &evalB1PSRamp, 16000, 99);
	pDstTree->Branch("DC_UP", &evalDCUp, 16000, 99);
	pDstTree->Branch("DC_DOWN", &evalDCDown, 16000, 99);

	for (size_t i=0; i < vEvalB1PSIamp.size(); i++) {
		ts = vEvalB1PSIamp[i].getTimeStamp();
		pcnt = vEvalB1PSIamp[i].getPulseCount();
		evalPSIamp = vEvalB1PSIamp[i];
		evalPEIamp = vEvalPEIamp[i];
		evalPSRamp = vEvalPSRamp[i];
		evalB1PEIamp = vEvalB1PEIamp[i];
		evalB1PSRamp = vEvalB1PSRamp[i];
		evalDCUp = vEvalDCUp[i];
		evalDCDown = vEvalDCDown[i];

		evalPSIamp.clear();
		evalPEIamp.clear();
		evalPSRamp.clear();
		evalB1PEIamp.clear();
		evalB1PSRamp.clear();
		evalDCUp.clear();
		evalDCDown.clear();

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
	std::string sFilterBDPCompr; // Filter string. Defines the logic to select specific events.

	if (argc != 3) {
		printf("Usage: analyseHistory4 srcdir dstdir\n");
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
	for (std::string sFilePath : vListOfFiles) {
		sSrcFilePath = sFilePath;
		sDstFilePath = sDstDirectory + getFileName(sFilePath);
		sDstFilePath.insert(sDstFilePath.rfind('.'), sDstSuffix);

		printf("Process file: %s ...\n", sSrcFilePath.c_str());
		TFile(sDstFilePath.c_str(), "RECREATE"); // delete if file exists
		analysePulseShape(sDstFilePath, "N0Events", sSrcFilePath, "N0Events");
		analyseBreakdown(sDstFilePath, "BDStruct", sSrcFilePath, "B0Events",
				"B1Events", sFilterBDStruct);
		analyseBreakdown(sDstFilePath, "BDPCompr", sSrcFilePath, "B0Events",
				"B1Events", sFilterBDPCompr);
	}

	clock_t end = clock();
	printf("Total elapsed time: %.3f\n", double(end - begin) / CLOCKS_PER_SEC);

	return 0;
}
