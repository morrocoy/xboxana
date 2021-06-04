#include <iostream>
#include <stdio.h>

#include <ctime>

#include "Rtypes.h"
#include "TTimeStamp.h"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RDFHelpers.hxx"

#include "TSystemDirectory.h"
#include "TSystemFile.h"

#include "XboxDAQChannel.hxx"
#include "XboxAnalyserResult.hxx"
#include "XboxAnalyserEvalBase.hxx"
#include "XboxAnalyserEvalPulse.hxx"
#include "XboxAnalyserEvalBreakdown.hxx"
#include "XboxAnalyserEvalBreakdownTest.hxx"
#include "XboxAnalyserView.hxx"

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

////////////////////////////////////////////////////////////////////////
/// List of Files in directory.
std::vector<std::string> getListOfFiles(const string &sDirectory, const string &ext) {
	TSystemDirectory dir(sDirectory.c_str(), sDirectory.c_str());
	TList *files = dir.GetListOfFiles();
	std::vector<std::string> vListOfFiles;

	if (files) {
		TSystemFile *file;
		TIter next(files);
		while ((file = (TSystemFile*) next())) {
			std::string sFilePath = file->GetName();
			if (!file->IsDirectory() && !sFilePath.compare(
					sFilePath.length() - ext.length(), ext.length(), ext)) {
				vListOfFiles.push_back(sFilePath);
			}
		}
	}
	return vListOfFiles;
}
void analyseEventsSpecific(const std::string &sDstFilePath, const std::string &sDstKey,
                         const std::string &sSrcFilePath,  const std::string &sSrcKey1, const std::string &sSrcKey2,
                         const std::string &sfilter="1==1") {

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
    XBOX::XboxAnalyserEvalPulse evalDC; // shape analysis of dark current and reflected signal
    XBOX::XboxAnalyserEvalBreakdown evalBD; // breakdown analysis

    evalDC.setPulseConfig(0.01, 0.99, 0.5);
    evalBD.setPulseConfig(0.01, 0.99, 0.9);
    evalBD.setJitterConfig(0.01, 0.3, 0.3, 0.001);
    evalBD.setRiseConfig(0.01, 0.99, 0.6, 0.03);
    evalBD.setDeflConfig(0.01, 0.99, 0.1, 0.02, 0.1);

    auto dfEval = dfFilt.Define("_eval1", evalBD,
                                {"PSI_amp","B1.PSI_amp", "PEI_amp", "B1.PEI_amp", "PSR_amp", "B1.PSR_amp"})
						.Define("_eval2", evalDC, {"PSR_amp"})
                        .Define("_eval3", evalDC, {"DC_UP"})
                        .Define("_eval4", evalDC, {"DC_DOWN"});

//	.Define("_eval2", "PSR_amp.max()")
//	.Define("_eval3", "DC_DOWN.risingEdge(0.5)")
//	.Define("_eval4", "DC_DOWN.fallingEdge(0.5)")
//	.Define("_eval5", "DC_DOWN.span()")
//	.Define("_eval6", "DC_DOWN.risingEdge(0.5)")
//	.Define("_eval7", "DC_DOWN.fallingEdge(0.5)");

    // get number of entries which passed the filters
    auto pCnt = dfFilt.Count();

    // get results from data frame.
    auto pEval1 = dfEval.Take<XBOX::XboxAnalyserResult>("_eval1");
    auto pEval2 = dfEval.Take<XBOX::XboxAnalyserResult>("_eval2");
    auto pEval3 = dfEval.Take<XBOX::XboxAnalyserResult>("_eval3");
    auto pEval4 = dfEval.Take<XBOX::XboxAnalyserResult>("_eval4");

    // transfer data frame to std::vector
    std::vector<XBOX::XboxAnalyserResult> vEval1 = *pEval1;
    std::vector<XBOX::XboxAnalyserResult> vEval2 = *pEval2;
    std::vector<XBOX::XboxAnalyserResult> vEval3 = *pEval3;
    std::vector<XBOX::XboxAnalyserResult> vEval4 = *pEval4;

    // sorting according to the time stamp (in-built of XBOX::XboxAnalyserBaseModel)
    std::sort(vEval1.begin(), vEval1.end());
    std::sort(vEval2.begin(), vEval2.end());
    std::sort(vEval3.begin(), vEval3.end());

    // print number of entities which passed all filters
    printf("%s: %llu events\n", sDstKey.c_str(), *pCnt);

    // export results
    TFile fileResults(sDstFilePath.c_str(), "UPDATE");
    TTree *pDstTree = new TTree(sDstKey.c_str(), sDstKey.c_str());

    TTimeStamp ts;
    ULong64_t pcnt;
    XBOX::XboxAnalyserResult eval1;
    XBOX::XboxAnalyserResult eval2;
    XBOX::XboxAnalyserResult eval3;
    XBOX::XboxAnalyserResult eval4;

    pDstTree->Branch("fTimeStamp", &ts, 16000, 99);
    pDstTree->Branch("fPulseCount", &pcnt, 16000, 99);
    pDstTree->Branch("PSI_amp", &eval1, 16000, 99);
    pDstTree->Branch("PSR_amp", &eval2, 16000, 99);
    pDstTree->Branch("DC_UP", &eval3, 16000, 99);
    pDstTree->Branch("DC_DOWN", &eval4, 16000, 99);

    for (size_t i=0; i < vEval1.size(); i++) {
        ts = vEval1[i].getTimeStamp();
        pcnt = vEval1[i].getPulseCount();
        eval1 = vEval1[i];
        eval2 = vEval2[i];
        eval3 = vEval3[i];
        eval4 = vEval4[i];
        pDstTree->Fill();
    }
    pDstTree->Write();
    delete pDstTree;
}




void convert(const std::string sSrcFilePath, const std::string sKey) {

	// Open the file containing the tree.
	TFile *file = TFile::Open(sSrcFilePath.c_str());

	// create TTreeReader
	TTreeReader reader(sKey.c_str(), file);

	// The branch "px" contains floats; access them as myPx.
	TTreeReaderValue<TTimeStamp> ts(reader, "fTimeStamp");
	TTreeReaderValue<ULong64_t> pc(reader, "fPulseCount");
	TTreeReaderValue<XBOX::XboxAnalyserResult> eval1(reader, "PSI_amp");
	TTreeReaderValue<XBOX::XboxAnalyserResult> eval2(reader, "PSR_amp");
	TTreeReaderValue<XBOX::XboxAnalyserResult> eval3(reader, "DC_UP");
	TTreeReaderValue<XBOX::XboxAnalyserResult> eval4(reader, "DC_DOWN");

	// configure output file
	std::string sDstFilePath = sSrcFilePath;
	sDstFilePath = sDstFilePath.substr(0, sDstFilePath.find_last_of(".")+1) + "txt";

	printf("Target: %s\n", sDstFilePath.c_str());

	FILE * pFile;
	pFile = fopen (sDstFilePath.c_str(),"w");

	// read each event and write it to the output file
	int i=0;

	fprintf (pFile, "%10s\t", "number");
	fprintf (pFile, "%20s\t", "time stamp");
	fprintf (pFile, "%20s\t", "pulse count");
	fprintf (pFile, "%20s\t", "peak power [W]");
	fprintf (pFile, "%20s\t", "pulse width [s]");

	fprintf (pFile, "%20s\t", "refl. rise [s]");
	fprintf (pFile, "%20s\t", "tran. fall [s]");

	fprintf (pFile, "%20s\t", "refl. peak [W]"); // reflection peak

	fprintf (pFile, "%20s\t", "up peak [?]");
	fprintf (pFile, "%20s\t", "up rise [s]");
	fprintf (pFile, "%20s\t", "up fall [s]");
	fprintf (pFile, "%20s\t", "up len [s]");

	fprintf (pFile, "%20s\t", "down peak [?]");
	fprintf (pFile, "%20s\t", "down rise [s]");
	fprintf (pFile, "%20s\t", "down fall [s]");
	fprintf (pFile, "%20s\n", "down len [s]");

	while (reader.Next()) {

		if (eval1->getTranBreakdownTime() == -1
				|| eval1->getReflBreakdownTime() == -1)
			continue;

		fprintf (pFile, "%10d\t",i); // event number
		fprintf (pFile, "%20s\t", ts->AsString("s")); // time stamp
		fprintf (pFile, "%20llu\t", eval1->getPulseCount()); // pulse count
		fprintf (pFile, "%20.12e\t", eval1->getPulsePowerAvg()); // peak power
		fprintf (pFile, "%20.12e\t", eval1->getPulseLength()); // pulse width

		fprintf (pFile, "%20.12e\t", eval1->getReflBreakdownTime()); // refl rise
		fprintf (pFile, "%20.12e\t", eval1->getTranBreakdownTime()); // tran fall

		fprintf (pFile, "%20.12e\t", eval2->getPulsePowerMax()); // reflection peak

		fprintf (pFile, "%20.12e\t", eval3->getPeakPeakValue()); // dc up peak [?]
		fprintf (pFile, "%20.12e\t", eval3->getPulseRisingEdge()); // start time of dc up [s]
		fprintf (pFile, "%20.12e\t", eval3->getPulseFallingEdge()); // end time of dc up [s]
		fprintf (pFile, "%20.12e\t", eval3->getPulseLength()); //signal length dc up [s]

		fprintf (pFile, "%20.12e\t", eval4->getPeakPeakValue()); // dc down peak [?]
		fprintf (pFile, "%20.12e\t", eval4->getPulseRisingEdge()); // start time of dc down [s]
		fprintf (pFile, "%20.12e\t", eval4->getPulseFallingEdge()); // end time of dc down [s]
		fprintf (pFile, "%20.12e\n", eval4->getPulseLength()); //signal length dc down [s]
		i++;
	}

	fclose (pFile);
}

void plot(const std::string sSrcFilePath, const std::string sKeyTree,
		const std::string sKeyCh1, const std::string sKeyCh2) {

	// Open the file containing the tree.
	TFile *file = TFile::Open(sSrcFilePath.c_str());

	// create TTreeReader
	TTreeReader reader(sKeyTree.c_str(), file);
	TTreeReaderValue<XBOX::XboxDAQChannel> ch0(reader, sKeyCh1.c_str());
	TTreeReaderValue<XBOX::XboxDAQChannel> ch1(reader, sKeyCh2.c_str());

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


void plot(const std::string sSrcFilePath, const std::string sKeyTree,
		const std::string sKeyCh1, const std::string sKeyCh2, TTimeStamp ts) {

	// Open the file containing the tree.
	TFile *file = TFile::Open(sSrcFilePath.c_str());

	// create TTreeReader
	TTreeReader reader(sKeyTree.c_str(), file);
	TTreeReaderValue<XBOX::XboxDAQChannel> ch0(reader, sKeyCh1.c_str());
	TTreeReaderValue<XBOX::XboxDAQChannel> ch1(reader, sKeyCh2.c_str());

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

			gr0.SetLineColor(kRed);
			gr1.SetLineColor(kBlue);
			gr0.SetMarkerColor(kRed);
			gr1.SetMarkerColor(kBlue);

			mg->Add(&gr0);
			mg->Add(&gr1);
			mg->Draw("AC");

//			c1.SetTitle(ch0->getTimeStamp().AsString("s"));
			auto legend = new TLegend(0.11,0.7,0.4,0.89);
			legend->AddEntry(&gr0, ch0->getChannelName().c_str(), "l");
			legend->AddEntry(&gr1, ch1->getChannelName().c_str(), "l");
			legend->Draw();

			c1.Draw();
			c1.Print("pictures_test/DC.png");
			printf("plot at %s\n", ch0->getTimeStamp().AsString("s"));
			break;
		}
	}

}

int main(int argc, char* argv[]) {

	gInterpreter->Declare("#include \"XboxDAQChannel.hxx\"");

//	ROOT::EnableImplicitMT(); // Tell ROOT you want to go parallel

	if (argc != 2) {
		printf("Usage: analyseHistory2 file\n");
		return 1;
	}
	std::string sourceFilePath = argv[1];
//	std::string sourceFilePath = "~/projects/data/xbox3/Xbox3_TD24_bo_L3_201901.root";

	clock_t begin = clock();

	// analyse Breakdown Pulse History ....................................
	std::string sfilter;
	std::vector<XBOX::XboxAnalyserResult> entries;

	std::string targetFilePath = sourceFilePath;
	std::string::size_type ipos = targetFilePath.rfind('.', targetFilePath.length());
	if (ipos != std::string::npos)
		targetFilePath.insert(ipos, "_analysed_PC");

	sfilter =
			" (!PERA_amp.getBreakdownFlag())"
			" && (PSR_amp.getBreakdownFlag()"
				" || DC_DOWN.getBreakdownFlag()"
				" || DC_UP.getBreakdownFlag())"
			" && (DC_UP.span()/B1.DC_UP.span() > 2."
				" || DC_DOWN.span()/B1.DC_DOWN.span() > 2.)";

	std::string sSrcFilePath ="/Users/kpapke/projects/data/xbox3/Xbox3_TD24_bo_L3_2018W29.root";
	std::string sDstFilePath ="/Users/kpapke/projects/data/xbox3/Xbox3_TD24_bo_L3_2018W29_xiaowei.root";


//	TFile(sDstFilePath.c_str(), "RECREATE");
//	analyseEventsSpecific(sDstFilePath, "BDStruct", sSrcFilePath, "B0Events", "B1Events", sfilter);
//
//	convert(sDstFilePath, "BDStruct");

	plot(sSrcFilePath, "B0Events", "DC_UP", "DC_DOWN", TTimeStamp(2018,07,16,18,19,34));


	return 0;

	// data visualisation .................................................
	XBOX::XboxAnalyserView viewer;


//	TTimeStamp ts0(2019,1,14,12,00,00);
//	TTimeStamp ts1(2019,1,14,12,10,00);

//	TTimeStamp ts0(2019,1,1,7,30,00);
//	TTimeStamp ts1(2019,3,15,7,39,00);

//	TTimeStamp ts0(2019,1,15,6,00,00);
//	TTimeStamp ts1(2019,2,16,22,00,00);

//	TTimeStamp ts0(2018,7,25,21,16,00);
//	TTimeStamp ts1(2018,7,25,21,20,00);


	std::string structure = "Xbox3_TD24_bo_L3";
	TTimeStamp ts0(2018,7,16,0,00,00);
//	TTimeStamp ts0(2019,3,17,0,00,00);
	TTimeStamp ts1(2019,4,17,0,00,00);
	viewer.setLimitsTime(ts0, ts1);
	viewer.setLimitsPAvg(0., 45e6);
	viewer.setLimitsPLen(0., 400e-9);
//	viewer.setPulseCntLimits(2.1e9, 2.3e9);
	viewer.setLimitsPCnt(0, 2.6e9);
	viewer.setLimitsRate(1e-8, 1e-3);
	viewer.setBins(100, 100);

//	std::string structure = "Xbox3_TD24_ubo_L4";
////	TTimeStamp ts0(2018,7,16,0,00,00);
//	TTimeStamp ts0(2019,3,17,0,00,00);
//	TTimeStamp ts1(2019,4,17,0,00,00);
//	viewer.setPeriod(ts0, ts1);
//	viewer.setPulseAvgLimits(0., 45e6);
//	viewer.setPulseLenLimits(0., 400e-9);
////	viewer.setPulseCntLimits(2.1e9, 2.3e9);
//	viewer.setPulseCntLimits(0, 2.6e9);
//	viewer.setPulseRateLimits(1e-8, 1e-3);
//	viewer.setBins(100, 100);

//	std::string structure = "Xbox3_T24N5_L1";
////	TTimeStamp ts0(2018,10,25,0,00,00);
//	TTimeStamp ts0(2019,4,9,0,00,00);
//	TTimeStamp ts1(2019,4,17,0,00,00);
//	viewer.setPeriod(ts0, ts1);
//	viewer.setPulseAvgLimits(0., 45e6);
//	viewer.setPulseLenLimits(0., 400e-9);
////	viewer.setPulseCntLimits(2.1e9, 2.3e9);
//	viewer.setPulseCntLimits(690e6, 750e6);
//	viewer.setPulseRateLimits(1e-8, 1e-3);
//	viewer.setBins(100, 100);

//	std::string structure = "Xbox3_T24N4_L2";
//	TTimeStamp ts0(2018,10,25,0,00,00);
////	TTimeStamp ts0(2019,4,9,0,00,00);
//	TTimeStamp ts1(2019,4,17,0,00,00);
//	viewer.setPeriod(ts0, ts1);
//	viewer.setPulseAvgLimits(0., 45e6);
//	viewer.setPulseLenLimits(0., 400e-9);
////	viewer.setPulseCntLimits(2.1e9, 2.3e9);
//	viewer.setPulseCntLimits(688e6, 743e6);
//	viewer.setPulseRateLimits(1e-8, 1e-3);
//	viewer.setBins(100, 100);

	targetFilePath = "~/projects/data/xbox3/" + structure + "*analysed_PC.root";
//	targetFilePath = "~/projects/data/xbox3/Xbox3_TD24_bo_L3_analysed_PC.root";
	viewer.load(targetFilePath, "ANAL000");
//	viewer.autosetPCntLimits();


//	viewer.plotPAvg("test_PulseAvg1.png");
//	viewer.plotPAvg("test_PulseAvg2.png", "pulse");
//	viewer.plotPLen("test_PulseLen1.png");
//	viewer.plotPLen("test_PulseLen2.png", "pulse");
//	viewer.plotPCnt("test_PulseCnt1.png");

	std::string sdate0 = ts0.AsString("s");
	std::string sdate1 = ts1.AsString("s");
	sdate0 = sdate0.substr(0, 10);
	sdate1 = sdate1.substr(0, 10);
	std::string sbasename = "pictures/" + structure + "_" + sdate0 + "_" + sdate1;

//	viewer.plotPulseRate("pictures/bdrate.png");
//	viewer.plotHistory(sbasename + "_HistoryVsPCnt_PC.png", XBOX::XboxAnalyserView::kPCnt);
//	viewer.plotBreakdownTime(sbasename + "_BDTimeVsPCnt_PC.png", XBOX::XboxAnalyserView::kPCnt);
//	viewer.plotHistory(sbasename + "_HistoryVsTime.png_PC");
//	viewer.plotBreakdownTime(sbasename + "_BDTimeVsTime_PC.png");

//	viewer.plotProbes();

	clock_t end = clock();
	printf("Total elapsed time: %.3f\n", double(end - begin) / CLOCKS_PER_SEC);

//	theApp.Run();
	return 0;
}
