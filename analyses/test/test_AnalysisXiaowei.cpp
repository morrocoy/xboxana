#include <iostream>
#include <ctime>

// root core
#include "Rtypes.h"
#include "TTimeStamp.h"
#include "TRegexp.h"
#include "TSystemFile.h"
#include "TSystemDirectory.h"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RDFHelpers.hxx"
#include "ROOT/RSnapshotOptions.hxx"

#include "ROOT/TProcessExecutor.hxx"
#include "ROOT/TThreadExecutor.hxx"
#include "ROOT/TSeq.hxx"

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

// xbox
#include "XboxFileSystem.h"
#include "XboxDAQChannel.hxx"
#include "XboxAnalyserEvalBase.hxx"
#include "XboxAnalyserEvalJitter.hxx"
#include "XboxAnalyserEvalPulseShape.hxx"
#include "XboxAnalyserEvalRisingEdge.hxx"
#include "XboxAnalyserEvalDeviation.hxx"
#include "XboxAnalyserEvalSignal.hxx"

void testProbe(const std::string &srcFilePathGlob) {

	 ROOT::RDataFrame df("BDPCompr", srcFilePathGlob, {"ProbeTime_PLRA_amp"});

	 auto dfFilt = df.Filter("ProbeTime_PLRA_amp<-0.3e-6 && ProbeTime_PLRA_amp != -1");
	 auto pCnt = dfFilt.Count();
	 printf("events: %llu events\n", *pCnt);
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
/// \param[in] dstFilePath The destination file path.
/// \param[in] sDstKey The branch name under which the results are stored.
/// \param[in] srcFilePath The source file path.
/// \param[in] sSrcKey The branch name under from which the data are loaded.
/// \param[in] sfilter The filter string.
void analysePulseShape(const std::string &dstFilePath, const std::string &dstTreeName,
		const std::string &srcFilePath,  const std::string &srcTreeName,
		const std::string &sfilter="1==1") {

    // load data frame
    ROOT::RDataFrame df(srcTreeName.c_str(), srcFilePath.c_str(), {"PSI_amp"});

    // filter breakdowns by time stamp, breakdown flags, signals, ...
    auto dfFilt = df.Filter(sfilter);

    // evaluate normal pulse and breakdown events
    XBOX::XboxAnalyserEvalPulseShape evalPulseShape90(0.01, 0.99, 0.9);

    auto dfEval = dfFilt
						.Define("TimeStamp", "PSI_amp.getTimeStamp()")
						.Define("PulseCount", "PSI_amp.getPulseCount()")

						// evaluate pulse shape
						.Define("eval1_PSI_amp", evalPulseShape90, {"PSI_amp"})

    					// remove signal data and take only meta data from channel
    					.Define("eval_PSI_amp", "eval1_PSI_amp.cloneMetaData()");

    // get number of entries which passed the filters
	auto pCnt = dfEval.Count();

	// save resluts to disk
	ROOT::RDF::RSnapshotOptions opts;
	opts.fMode = "UPDATE";

	dfEval.Snapshot(dstTreeName, dstFilePath, {
			"TimeStamp",
			"PulseCount",
			"eval_PSI_amp",
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
/// \param[in] sDstKey The branch name under which the results are stored.
/// \param[in] srcFilePath The source file path.
/// \param[in] sSrcKey1 First branch name under from which the data are loaded.
/// \param[in] sSrcKey2 Second branch name under from which the data are loaded.
/// \param[in] sfilter The filter string.
void analyseBreakdown(const std::string &dstFilePath, const std::string &dstTreeName,
        const std::string &srcFilePath,  const std::string &srcTreeName1,
        const std::string &srcTreeName2, const std::string &sfilter="1==1") {

    // create data frame
    TFile fileEvents(srcFilePath.c_str());

    // take breakdown events as primary tree
    TTree *pSrcTree = (TTree*)fileEvents.Get(srcTreeName1.c_str());

    // add pulses events before breakdown as friend tree
    pSrcTree->AddFriend(("B1=" + srcTreeName2).c_str(), srcFilePath.c_str());

    // load data frame
    ROOT::RDataFrame df(*pSrcTree, {"PSI_amp"});

    // filter breakdowns by time stamp, breakdown flags, signals, ...
    auto dfFilt = df.Filter(sfilter);

    // evaluate normal pulse and breakdown events
    // pulse shape analysis (PSI)

    XBOX::XboxAnalyserEvalPulseShape evalPulseShape30(0.01, 0.3, 0.5); // for PSI, PLR charge
    XBOX::XboxAnalyserEvalPulseShape evalPulseShape90(0.01, 0.99, 0.9); // for PSI
    XBOX::XboxAnalyserEvalPulseShape evalPulseShape50(0.01, 0.99, 0.5); // for DC

    XBOX::XboxAnalyserEvalSignal evalSignal; // PKIA_amp, PKIB_amp

    XBOX::XboxAnalyserEvalJitter evalJitterStruct(0.01, 0.3, 0.3, 0.001);
    XBOX::XboxAnalyserEvalJitter evalJitterPCompr(0.01, 0.3, 0.3, 0.002);
    XBOX::XboxAnalyserEvalJitter evalJitterKlystr(0.01, 0.3, 0.3, 0.002);

    XBOX::XboxAnalyserEvalRisingEdge evalRisingEdge(0.01, 0.99, 0.8, 0.03);
    XBOX::XboxAnalyserEvalDeviation evalDeviation(0.01, 0.99, 0.1, 0.01, 0.1);

    auto dfEval = dfFilt
    					// index information
						.Define("TimeStamp", "PSI_amp.getTimeStamp()")
						.Define("PulseCount", "PSI_amp.getPulseCount()")

						// refined pulse shape analysis of structure signals
						.Define("ChB1_PSI_amp_precharge", evalPulseShape30, {"B1.PSI_amp"})
						.Define("ChB1_PSI_amp", evalPulseShape90, {"B1.PSI_amp"})
						.Define("ChB1_PEI_amp", evalPulseShape90, {"B1.PEI_amp"})
						.Define("ChB1_PSR_amp", evalPulseShape90, {"B1.PSR_amp"})
						.Define("ChB0_PSI_amp", evalPulseShape90, {"PSI_amp"})
						.Define("ChB0_PEI_amp", evalPulseShape90, {"PEI_amp"})
						.Define("ChB0_PSR_amp", evalPulseShape90, {"PSR_amp"})
						// jitter of the structure signals
						.Define("Jitter_PSI_amp", evalJitterStruct, {"PSI_amp", "B1.PSI_amp"})
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
								"(DevTime_PSR_amp - RefTime_PSR_amp - DevTime_PEI_amp + RefTime_PEI_amp)/2")

						// 50 % - level pulse shape evaluation of klystron signals
						.Define("ChB1_PKIA_amp", evalPulseShape50, {"B1.PKIA_amp"})
						.Define("ChB1_PKIB_amp", evalPulseShape50, {"B1.PKIB_amp"})
						.Define("ChB1_PKRA_amp", evalPulseShape50, {"B1.PKRA_amp"})
						.Define("ChB1_PKRB_amp", evalPulseShape50, {"B1.PKRB_amp"})
						.Define("ChB0_PKIA_amp", evalPulseShape50, {"PKIA_amp"})
						.Define("ChB0_PKIB_amp", evalPulseShape50, {"PKIB_amp"})
						.Define("ChB0_PKRA_amp", evalPulseShape50, {"PKRA_amp"})
						.Define("ChB0_PKRB_amp", evalPulseShape50, {"PKRB_amp"})
						// jitter of the klystron signals
						.Define("Jitter_PKIA_amp", evalJitterKlystr, {"PKIA_amp", "B1.PKIA_amp"})
						.Define("Jitter_PKIB_amp", evalJitterKlystr, {"PKIB_amp", "B1.PKIB_amp"})
						.Define("Jitter_PKRA_amp", evalJitterKlystr, {"PKRA_amp", "B1.PKRA_amp"})
						.Define("Jitter_PKRB_amp", evalJitterKlystr, {"PKRB_amp", "B1.PKRB_amp"})
						// reference time is rising edges of the klystron signals
						.Define("RefTime_PKIA_amp", "ChB1_PKIA_amp.getXmin()")
						.Define("RefTime_PKIB_amp", "ChB1_PKIB_amp.getXmin()")
						.Define("RefTime_PKRA_amp", "ChB1_PKRA_amp.getXmin()")
						.Define("RefTime_PKRB_amp", "ChB1_PKRB_amp.getXmin()")
						// time at which klystron signals start to deviate between breakdown and previous event
						.Define("DevTime_PKIA_amp", evalDeviation, {"PKIA_amp", "B1.PKIA_amp", "Jitter_PKIA_amp"})
						.Define("DevTime_PKIB_amp", evalDeviation, {"PKIB_amp", "B1.PKIB_amp", "Jitter_PKIB_amp"})
						.Define("DevTime_PKRA_amp", evalDeviation, {"PKRA_amp", "B1.PKRA_amp", "Jitter_PKRA_amp"})
						.Define("DevTime_PKRB_amp", evalDeviation, {"PKRB_amp", "B1.PKRB_amp", "Jitter_PKRB_amp"})

						// 30 % - level pulse shape evaluation of pulse compressor signals
						.Define("ChB1_PLIA_amp", evalPulseShape30, {"B1.PLIA_amp"})
						.Define("ChB1_PLIB_amp", evalPulseShape30, {"B1.PLIB_amp"})
						.Define("ChB1_PLRA_amp", evalPulseShape30, {"B1.PLRA_amp"})
						.Define("ChB1_PLRB_amp", evalPulseShape30, {"B1.PLRB_amp"})
						.Define("ChB0_PLIA_amp", evalPulseShape30, {"PLIA_amp"})
						.Define("ChB0_PLIB_amp", evalPulseShape30, {"PLIB_amp"})
						.Define("ChB0_PLRA_amp", evalPulseShape30, {"PLRA_amp"})
						.Define("ChB0_PLRB_amp", evalPulseShape30, {"PLRB_amp"})
						// jitter of the pulse compressor signals
						.Define("Jitter_PLIA_amp", evalJitterPCompr, {"PLIA_amp", "B1.PLIA_amp"})
						.Define("Jitter_PLIB_amp", evalJitterPCompr, {"PLIB_amp", "B1.PLIB_amp"})
						.Define("Jitter_PLRA_amp", evalJitterPCompr, {"PLRA_amp", "B1.PLRA_amp"})
						.Define("Jitter_PLRB_amp", evalJitterPCompr, {"PLRB_amp", "B1.PLRB_amp"})
						// reference time is the rising edges of the pulse compressor signals
						.Define("RefTime_PLIA_amp", "ChB1_PLIA_amp.getXmin()")
						.Define("RefTime_PLIB_amp", "ChB1_PLIB_amp.getXmin()")
						.Define("RefTime_PLRA_amp", "ChB1_PLRA_amp.getXmin()")
						.Define("RefTime_PLRB_amp", "ChB1_PLRB_amp.getXmin()")
						// time at which pulse compressor signals start to deviate between breakdown and previous event
						.Define("DevTime_PLIA_amp", evalDeviation, {"PLIA_amp", "B1.PLIA_amp", "Jitter_PLIA_amp"})
						.Define("DevTime_PLIB_amp", evalDeviation, {"PLIB_amp", "B1.PLIB_amp", "Jitter_PLIB_amp"})
						.Define("DevTime_PLRA_amp", evalDeviation, {"PLRA_amp", "B1.PLRA_amp", "Jitter_PLRA_amp"})
						.Define("DevTime_PLRB_amp", evalDeviation, {"PLRB_amp", "B1.PLRB_amp", "Jitter_PLRB_amp"})
						// Probes for pulse compressor
						.Define("ProbeTime_PLIA_amp", "DevTime_PLIA_amp - RefTime_PLIA_amp")
						.Define("ProbeTime_PLIB_amp", "DevTime_PLIB_amp - RefTime_PLIB_amp")
						.Define("ProbeTime_PLRA_amp", "DevTime_PLRA_amp - RefTime_PLRA_amp")
						.Define("ProbeTime_PLRB_amp", "DevTime_PLRB_amp - RefTime_PLRB_amp")

						// 50 % - level pulse shape evaluation of the dark current signals
						.Define("ChB0_DC_UP", evalPulseShape50, {"DC_UP"})
						.Define("ChB0_DC_DOWN", evalPulseShape50, {"DC_DOWN"});



	// get number of entries which passed the filters
	auto pCnt = dfEval.Count();

	// save resluts to disk
	ROOT::RDF::RSnapshotOptions opts;
	opts.fMode = "UPDATE";

	dfEval.Snapshot(dstTreeName, dstFilePath, {

		// index information
		"TimeStamp",
		"PulseCount",

		// pulse shape analysis of structure signals
		"ChB1_PSI_amp_precharge",
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

		// 50 % - level pulse shape evaluation of klystron signals
		"ChB1_PKIA_amp",
		"ChB1_PKIB_amp",
		"ChB1_PKRA_amp",
		"ChB1_PKRB_amp",
		"ChB0_PKIA_amp",
		"ChB0_PKIB_amp",
		"ChB0_PKRA_amp",
		"ChB0_PKRB_amp",
		// jitter of the klystron signals
		"Jitter_PKIA_amp",
		"Jitter_PKIB_amp",
		"Jitter_PKRA_amp",
		"Jitter_PKRB_amp",
		// time where is the rising edges of the klystron signals
		"RefTime_PKIA_amp",
		"RefTime_PKIB_amp",
		"RefTime_PKRA_amp",
		"RefTime_PKRB_amp",
		// time at which klystron signals start to deviate between breakdown and previous event
		"DevTime_PKIA_amp",
		"DevTime_PKIB_amp",
		"DevTime_PKRA_amp",
		"DevTime_PKRB_amp",

		// 30 % - level pulse shape evaluation of pulse compressor signals
		"ChB1_PLIA_amp",
		"ChB1_PLIB_amp",
		"ChB1_PLRA_amp",
		"ChB1_PLRB_amp",
		"ChB0_PLIA_amp",
		"ChB0_PLIB_amp",
		"ChB0_PLRA_amp",
		"ChB0_PLIB_amp",
		// jitter of the pulse compressor signals
		"Jitter_PLIA_amp",
		"Jitter_PLIB_amp",
		"Jitter_PLRA_amp",
		"Jitter_PLRB_amp",
		// time where is the rising edges of the pulse compressor signals
		"RefTime_PLIA_amp",
		"RefTime_PLIB_amp",
		"RefTime_PLRA_amp",
		"RefTime_PLRB_amp",
		// time at which pulse compressor signals start to deviate between breakdown and previous event
		"DevTime_PLIA_amp",
		"DevTime_PLIB_amp",
		"DevTime_PLRA_amp",
		"DevTime_PLRB_amp",
		//Probes for pulse compressor
		"ProbeTime_PLIA_amp",
		"ProbeTime_PLIB_amp",
		"ProbeTime_PLRA_amp",
		"ProbeTime_PLRB_amp",

		// dark current signals
		"ChB0_DC_UP",
		"ChB0_DC_DOWN",
	}, opts);

    // print number of entities which passed all filters
    printf("%s: %llu events\n", dstTreeName.c_str(), *pCnt);

}


////////////////////////////////////////////////////////////////////////////////
/// Plotting.
void plot_sweep(const std::string srcFilePath, const std::string treeName,
		const std::string channelName1, const std::string channelName2) {

	// Open the file containing the tree.
	TFile *file = TFile::Open(srcFilePath.c_str());

	// create TTreeReader
	TTreeReader reader(treeName.c_str(), file);
	TTreeReaderValue<XBOX::XboxDAQChannel> ch0(reader, channelName1.c_str());
	TTreeReaderValue<XBOX::XboxDAQChannel> ch1(reader, channelName2.c_str());

	while (reader.Next()) {

		TCanvas c1("c1","c1",700, 500);
		c1.SetGrid();

		ch0->flushbuffer();
		ch1->flushbuffer();

		std::vector<Double_t> t0 = ch0->getTimeAxis();
		std::vector<Double_t> t1 = ch1->getTimeAxis();
		std::vector<Double_t> y0 = ch0->getSignal();
		std::vector<Double_t> y1 = ch1->getSignal();

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

		printf("%s\n", ch0->getTimeStamp().AsString("s"));
		printf("dc up   rise: %e\t | fall: %e\n", ch0->risingEdge(0.5), ch0->fallingEdge(0.5));
		printf("dc down rise: %e\t | fall: %e\n", ch1->risingEdge(0.5), ch1->fallingEdge(0.5));
		printf("dc up    min: %e\t |  max: %e | span: %e\n", ch0->min(), ch0->max(), ch0->span());
		printf("dc down  min: %e\t |  max: %e | span: %e\n\n", ch1->min(), ch1->max(), ch1->span());

		c1.Print("pictures_test/DC.png");
		std::cin.get();
	}

}

////////////////////////////////////////////////////////////////////////////////
/// Plotting.
void plot(const std::string srcFilePath, const std::string treeName,
		const std::string channelName1, const std::string channelName2, TTimeStamp ts) {

	printf("%s\n", srcFilePath.c_str());
	printf("%s\n", treeName.c_str());

	// Open the file containing the tree.
	TFile *file = TFile::Open(srcFilePath.c_str());

	// create TTreeReader
	TTreeReader reader(treeName.c_str(), file);
	TTreeReaderValue<XBOX::XboxDAQChannel> ch0(reader, channelName1.c_str());
	TTreeReaderValue<XBOX::XboxDAQChannel> ch1(reader, channelName2.c_str());

	while (reader.Next()) {
		if (ch0->getTimeStamp() >= ts) {

			TCanvas c1("c1","",700, 500);
			c1.SetGrid();

			ch0->flushbuffer();
			ch1->flushbuffer();
			std::vector<Double_t> t0 = ch0->getTimeAxis();
			std::vector<Double_t> t1 = ch1->getTimeAxis();
			std::vector<Double_t> y0 = ch0->getSignal();
			std::vector<Double_t> y1 = ch1->getSignal();

			TMultiGraph *mg = new TMultiGraph();
			TGraph gr0(t0.size(), &t0[0], &y0[0]);
			TGraph gr1(t1.size(), &t1[0], &y1[0]);

			gr0.SetLineColor(kBlack);
			gr1.SetLineColor(kBlue);
			gr0.SetMarkerColor(kBlack);
			gr1.SetMarkerColor(kBlue);

			mg->Add(&gr0);
			mg->Add(&gr1);
			mg->Draw("AC");

			TAxis *ax = mg->GetXaxis();
			TAxis *ay = mg->GetYaxis();

//			ax->SetTitle("Time [s]");
//			ay->SetTitle("Amplitude");

			TLine *l1 = new TLine(ch0->getXmin(), ay->GetXmin(), ch0->getXmin(), ay->GetXmax());
			l1->SetLineWidth(1);
			l1->SetLineColor(kBlack);
			l1->Draw();
			l1->SetNDC(false);
			printf("xmin: %e\n", ch0->getXmin());

			TLine *l2 = new TLine(ch1->getXdev(), ay->GetXmin(), ch1->getXdev(), ay->GetXmax());
			l2->SetLineWidth(1);
			l2->SetLineColor(kBlue-2);
			l2->Draw();
			l2->SetNDC(false);
			printf("xdev: %e\n", ch1->getXdev());
			printf("offs: %e\n", ch1->getStartOffset());

//			c1.SetTitle(ch0->getTimeStamp().AsString("s"));
			auto legend = new TLegend(0.11,0.7,0.4,0.89);
			legend->AddEntry(&gr0, ch0->getChannelName().c_str(), "l");
			legend->AddEntry(&gr1, ch1->getChannelName().c_str(), "l");

			legend->Draw();

			c1.Draw();
			c1.Print("pictures_test/DC.png");
			printf("plot at %s\n", ch0->getTimeStamp().AsString("s"));

			delete l1;
			delete l2;
			delete legend;
			delete mg;

			break;
		}
	}

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

	std::string sFilterBDStruct; // Filter string. Defines the logic to select specific events.
	std::string sFilterBDPCompr; // Filter string. Defines the logic to select specific events.

	if (argc != 3) {
		printf("Usage: test_AnalysisDefault srcdir dstdir \n");
		return 1;
	}
	srcDir = argv[1];
	dstDir = argv[2];
	dstSuffix = "_Xiaowei";

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
//	    analyseBreakdown(dstFilePath, "BDStruct", srcFilePath,
//	    		"B0Events", "B1Events", sFilterBDStruct);
//	    analyseBreakdown(dstFilePath, "BDPCompr", srcFilePath,
//	    		"B0Events", "B1Events", sFilterBDPCompr);
//	}

	// The number of workers
	const UInt_t nThreads = 8U;
	ROOT::EnableThreadSafety();

	auto workItem = [srcExistingFilePaths, dstDir](UInt_t workerID) {

		std::string dstSuffix = "_Xiaowei";

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
			" && PLRB_amp.getBreakdownFlag()"
			" && !(PSR_amp.getBreakdownFlag()"
				" || PERA_amp.getBreakdownFlag()"
				" || PERB_amp.getBreakdownFlag()"
				" || DC_DOWN.getBreakdownFlag()"
				" || DC_UP.getBreakdownFlag())";

		std::string srcFilePath = srcExistingFilePaths[workerID];
		std::string dstFilePath = dstDir + XBOX::getFileName(srcFilePath);
		dstFilePath.insert(dstFilePath.rfind('.'), dstSuffix);

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
//	ROOT::TThreadExecutor pool(nThreads);
	ROOT::TProcessExecutor pool(nThreads);

	// Fill the pool with work
	pool.Map(workItem, ROOT::TSeqI(srcExistingFilePaths.size()));


	clock_t end = clock();
	printf("Total elapsed time: %.3f\n", double(end - begin) / CLOCKS_PER_SEC);

//	// plot signals for verification
//	srcFilePath ="/Users/kpapke/projects/CLiC/xbox/data/Xbox3_T24N5_L1/anal/Xbox3_T24N5_L1_20190601_Xiaowei.root";
//	srcFilePath ="/Users/kpapke/projects/CLiC/xbox/data/Xbox3_T24N5_L1/anal/Xbox3_T24N5_L1_20181120_Xiaowei.root";
////	plot(srcFilePath, "B0Events", "DC_UP", "DC_DOWN", TTimeStamp(2018,07,16,18,19,34));
//	plot(srcFilePath, "BDPCompr", "eval_B1_PLRA_amp", "eval_PLRA_amp", TTimeStamp(2019,11,20,5,51,0));


//// histograms of probes
//
//	std::string fileGlob = dstDir + "*" + dstSuffix + ".root";
//	srcExistingFilePaths = XBOX::getListOfFiles(fileGlob);
//	printf("path: %s", fileGlob.c_str());
//
////	TH1D* h1 = new TH1D("h1", "h1 title", 100, -5e-6, 5e-6);
////	Int_t i = 0;
////	for (auto &filepath: srcExistingFilePaths) {
////		printf("%s\n", filepath.c_str());
////		continue;
////
////		TFile *file = TFile::Open(filepath.c_str());
////		TTreeReader reader("BDPCompr", file);
//////		TTreeReader reader("BDStruct", file);
////		TTreeReaderValue<Double_t> pval(reader, "ProbeTime_PLRA_amp");
////
////		while (reader.Next()) {
////			h1->Fill(*pval);
////			i++;
////		}
////	}
////	printf("Number: %d\n", i);
//
//
//	ROOT::RDataFrame d ("BDPCompr", fileGlob);
//
////	std::vector<std::string> colNames = df.GetColumnNames();
////	for (auto &name : colNames)
////		printf("%s\n", name.c_str());
//
//	auto df = d.Filter("ProbeTime_PLRA_amp!=-1");
//	auto h1 = df.Histo1D({"", "", 200, -5e-6, 5e-6}, "ProbeTime_PLRA_amp");
//
//	TCanvas c1("c1","c1",700, 500);
//	c1.SetGrid();
//	h1->Draw();
//	c1.Draw();
//	c1.Print("ProbeTime_PLRA_amp.png");

//	testProbe(dstDir + "/*" + dstSuffix + ".root");
	return 0;
}
