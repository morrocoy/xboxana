#include <iostream>
#include <ctime>
#include <sys/stat.h>

#include "Rtypes.h"
#include "TTimeStamp.h"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RDFHelpers.hxx"

#include "TSystem.h"

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
/// Plotting.
void plot_sweep(const std::string sSrcFilePath, const std::string sKeyTree,
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

TTimeStamp getFirstTimeStamp(const std::string srcFilePath,
		const std::string treeName1, const std::string colName1) {

	// Open the file containing the tree.
	TFile *file = TFile::Open(srcFilePath.c_str());

	// create TTreeReader
	TTreeReader reader1(treeName1.c_str(), file);
	TTreeReaderValue<XBOX::XboxDAQChannel> ch1(reader1, colName1.c_str());

	reader1.Next();

	TTimeStamp ts = ch1->getTimeStamp();
	delete file;
	return ts;
}


void plotSignals(const std::string dstDir, const std::string srcFilePath,
		const std::string treeName1, const std::string colName1,
		const std::string treeName2, const std::string colName2,
		TTimeStamp ts) {

	// Open the file containing the tree.
	TFile *file = TFile::Open(srcFilePath.c_str());

	// create TTreeReader
	TTreeReader reader1(treeName1.c_str(), file);
	TTreeReader reader2(treeName2.c_str(), file);

//	TTreeReaderValue<TTimeStamp> ts1(reader1, "TimeStamp");
//	TTreeReaderValue<TTimeStamp> ts2(reader2, "TimeStamp");
	TTreeReaderValue<XBOX::XboxDAQChannel> ch1(reader1, colName1.c_str());
	TTreeReaderValue<XBOX::XboxDAQChannel> ch2(reader2, colName2.c_str());

	while (reader1.Next()) {
//		if (*ts1 >= ts)
		if (ch1->getTimeStamp() >= ts)
			break;
	}
	while (reader2.Next()) {
//		if (*ts2 >= ts)
		if (ch2->getTimeStamp() >= ts)
			break;
	}

	Double_t lbnd=0.; ///<Lower time axis bound.
	Double_t ubnd=0.; ///<Upper time axis bound.
	Double_t tmin; ///<Lower time axis limit.
	Double_t tmax; ///<Upper time axis limit.

	// get signals
	std::vector<Double_t> t1 = ch1->getTimeAxis();
	std::vector<Double_t> y1 = ch1->getSignal();
	std::vector<Double_t> t2 = ch2->getTimeAxis();
	std::vector<Double_t> y2 = ch2->getSignal();

	// create plot
	std::string title = std::string(ch1->getTimeStamp().AsString("s")) + " " + ch1->getChannelName();

	TCanvas c1("c1",title.c_str(),700, 500);
	c1.SetGrid();

	TGraph gr1(t1.size(), &t1[0], &y1[0]);
	gr1.SetLineColor(kBlue);
	gr1.SetMarkerColor(kBlue);

	TGraph gr2(t2.size(), &t2[0], &y2[0]);
	gr2.SetLineColor(kBlack);
	gr2.SetMarkerColor(kBlack);

	TMultiGraph *mg = new TMultiGraph();
	mg->Add(&gr2);
	mg->Add(&gr1);
	mg->SetTitle(title.c_str());
	mg->Draw("AC");

	TAxis *ax = mg->GetXaxis();
	TAxis *ay = mg->GetYaxis();

	ax->SetTitle("Time [s]");
	ay->SetTitle("Amplitude");

//	auto legend = new TLegend(0.11,0.7,0.4,0.89);
//	legend->AddEntry(mg, (treeName1 + "_" + colName1).c_str(), "l");
//	legend->AddEntry(mg, (treeName1 + "_" + colName2).c_str(), "l");
//	legend->AddEntry(mg, colName1.c_str(), "l");
//	legend->AddEntry(mg, "Breakdown", "l");
//	legend->AddEntry(mg, "Previous", "l");
//	legend->Draw();
	c1.Draw();


	c1.Print((dstDir + title + ".png").c_str());

	delete mg;
//	delete legend;

}
////////////////////////////////////////////////////////////////////////////////
/// Testing.
void testPulseShape(const std::string srcFilePath, const std::string dstDir,
		const std::string treeName1, const std::string colName1,
		TTimeStamp ts=0) {

	// Open the file containing the tree.
	TFile *file = TFile::Open(srcFilePath.c_str());

	// create TTreeReader
	TTreeReader reader1(treeName1.c_str(), file);
	TTreeReaderValue<XBOX::XboxDAQChannel> ch1(reader1, colName1.c_str());

	while (reader1.Next()) {

		if (ts.AsDouble() == 0) {

			XBOX::XboxAnalyserEvalPulseShape evalPulseShape(0.01, 0.99, 0.3);
//			XBOX::XboxAnalyserEvalPulseShape evalPulseShape(0.01, 0.99, 0.9);
			evalPulseShape.setReportDir(dstDir);
			evalPulseShape.setReport(true);
			evalPulseShape(*ch1);
		}
		if (ch1->getTimeStamp() >= ts) {

			XBOX::XboxAnalyserEvalPulseShape evalPulseShape(0.01, 0.99, 0.3);
//			XBOX::XboxAnalyserEvalPulseShape evalPulseShape(0.01, 0.99, 0.9);
			evalPulseShape.setReportDir(dstDir);
			evalPulseShape.setReport(true);
			evalPulseShape(*ch1);
			break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
/// Testing.
void testJitter(const std::string srcFilePath, const std::string dstDir,
		const std::string treeName1, const std::string colName1,
		const std::string treeName2, const std::string colName2,
		TTimeStamp ts=0) {

	// Open the file containing the tree.
	TFile *file = TFile::Open(srcFilePath.c_str());

	// create TTreeReader
	TTreeReader reader1(treeName1.c_str(), file);
	TTreeReader reader2(treeName2.c_str(), file);

	TTreeReaderValue<XBOX::XboxDAQChannel> ch1(reader1, colName1.c_str());
	TTreeReaderValue<XBOX::XboxDAQChannel> ch2(reader2, colName2.c_str());

	while (reader1.Next() && reader2.Next()) {

		if (ts.AsDouble() == 0) {

//			XBOX::XboxAnalyserEvalJitter evalJitter(0.01, 0.3, 0.3, 0.001);
			XBOX::XboxAnalyserEvalJitter evalJitter(0.02, 0.3, 0.3, 0.002);
			evalJitter.setReportDir(dstDir);
			evalJitter.setReport(true);
			evalJitter(*ch1, *ch2);
		}
		else if (ch1->getTimeStamp() >= ts) {

			XBOX::XboxAnalyserEvalJitter evalJitter(0.01, 0.3, 0.3, 0.001);
//			XBOX::XboxAnalyserEvalJitter evalJitter(0.02, 0.3, 0.3, 0.002);
			evalJitter.setReportDir(dstDir);
			evalJitter.setReport(true);
			evalJitter(*ch1, *ch2);
			break;
		}
	}

	delete file;
}

////////////////////////////////////////////////////////////////////////////////
/// Testing.
void testRisingEdge(const std::string srcFilePath, const std::string dstDir,
		const std::string treeName1, const std::string colName1,
		TTimeStamp ts=0) {

	// Open the file containing the tree.
	TFile *file = TFile::Open(srcFilePath.c_str());

	// create TTreeReader
	TTreeReader reader1(treeName1.c_str(), file);

	TTreeReaderValue<XBOX::XboxDAQChannel> ch1(reader1, colName1.c_str());

	while (reader1.Next()) {

		if (ts.AsDouble() == 0) {
			XBOX::XboxAnalyserEvalRisingEdge evalRisingEdge(0.01, 0.99, 0.6, 0.03);

			evalRisingEdge.setReportDir(dstDir);
			evalRisingEdge.setReport(true);
			evalRisingEdge(*ch1);
		}
		else if (ch1->getTimeStamp() >= ts) {
			XBOX::XboxAnalyserEvalRisingEdge evalRisingEdge(0.01, 0.99, 0.6, 0.03);

			evalRisingEdge.setReportDir(dstDir);
			evalRisingEdge.setReport(true);
			evalRisingEdge(*ch1);
			break;
		}
	}

	delete file;
}

////////////////////////////////////////////////////////////////////////////////
/// Testing.
void testDeviation(const std::string srcFilePath, const std::string dstDir,
		const std::string treeName1, const std::string colName1,
		const std::string treeName2, const std::string colName2,
		TTimeStamp ts=0) {

	// Open the file containing the tree.
	TFile *file = TFile::Open(srcFilePath.c_str());

	// create TTreeReader
	TTreeReader reader1(treeName1.c_str(), file);
	TTreeReader reader2(treeName2.c_str(), file);

	TTreeReaderValue<XBOX::XboxDAQChannel> ch1(reader1, colName1.c_str());
	TTreeReaderValue<XBOX::XboxDAQChannel> ch2(reader2, colName2.c_str());

	while (reader1.Next() && reader2.Next()) {

		if (ts.AsDouble() == 0) {
			XBOX::XboxAnalyserEvalJitter evalJitter(0.02, 0.3, 0.3, 0.002);
			XBOX::XboxAnalyserEvalDeviation evalDeviation(0.01, 0.99, 0.1, 0.02, 0.1);
//			XBOX::XboxAnalyserEvalDeviation evalDeviation(0.01, 0.99, 0.1, 0.01, 0.1);

			evalDeviation.setReportDir(dstDir);
			evalDeviation.setReport(true);
			Double_t jitter = evalJitter(*ch1, *ch2);
			evalDeviation(*ch1, *ch2,  jitter);
		}
		else if (ch1->getTimeStamp() >= ts) {
			XBOX::XboxAnalyserEvalJitter evalJitter(0.02, 0.3, 0.3, 0.002);
			XBOX::XboxAnalyserEvalDeviation evalDeviation(0.01, 0.99, 0.1, 0.02, 0.1);
//			XBOX::XboxAnalyserEvalDeviation evalDeviation(0.01, 0.99, 0.1, 0.01, 0.1);

			evalDeviation.setReportDir(dstDir);
			evalDeviation.setReport(true);
			Double_t jitter = evalJitter(*ch1, *ch2);
			evalDeviation(*ch1, *ch2,  jitter);
			break;
		}
	}

	delete file;
}

std::string getFileName(const std::string &project, TTimeStamp ts) {

	unsigned int iYear;
	unsigned int iMonth;
	unsigned int iWeek;
	unsigned int iDay;

	char sbuf[100];
	ts.GetDate(true, 0, &iYear, &iMonth, &iDay);
	snprintf(sbuf, 100, "%s_%04d%02d%02d", project.c_str(), iYear, iMonth, iDay);
	return std::string(sbuf);
}

int main(int argc, char* argv[]) {

	gInterpreter->Declare("#include \"XboxDAQChannel.hxx\"");

	std::string rootDir; // Directory of containing all projects.
	std::string srcDir; // Directory of input files.
	std::string dstDir; // Directory of output files.
	std::string dstSubDir; // Sub directory of output files.

	std::string project;

	std::string srcFilePath; // Input filepath.
	std::string dstFilePath; // Output filepath including filename and type.

	std::string sFilterBDStruct; // Filter string. Defines the logic to select specific events.
	std::string sFilterBDPCompr; // Filter string. Defines the logic to select specific events.

	TTimeStamp ts;
	TTimeStamp ts0;

	rootDir = "/Users/kpapke/projects/CLiC/xbox/data/";
	project = "Xbox3_T24N5_L1";
	ts = TTimeStamp(2018,11,20,5,51,0);
	ts = TTimeStamp(2018,11,20,5,51,0);

	srcDir = rootDir + project + "/root/";
	dstDir = rootDir + project + "/pictures/" + getFileName(project, ts) + "/";

	srcFilePath = srcDir + getFileName(project, ts) + ".root";

//	testPulseShape(srcFilePath, dstDir, "B0Events", "PLRA_amp", ts);
//	testJitter(srcFilePath, dstDir, "B0Events", "PSI_amp", "B1Events", "PSI_amp", ts);
//	testDeviation(srcFilePath, dstDir, "B0Events", "PSI_amp", "B1Events", "PSI_amp", ts);
//
//	testJitter(srcFilePath, dstDir, "B0Events", "PLRA_amp", "B1Events", "PLRA_amp");
//	testDeviation(srcFilePath, dstDir, "B0Events", "PLRA_amp", "B1Events", "PLRA_amp", ts);

	// ********************************************************************
//	rootDir = "/Users/kpapke/projects/CLiC/xbox/data/";
//	project = "Xbox3_T24N5_L1";
//	std::string category = "BDStruct";
////	std::string category = "BDPCompr";
//
//
//	ts = TTimeStamp(2018,11,20,5,51,0);
//	ts = TTimeStamp(2018,7,20,5,51,0);
//
//	srcDir = rootDir + project + "/xiaowei/";
//	dstDir = rootDir + project + "/pictures/" + getFileName(project, ts) + "/";
//
//	srcFilePath = srcDir + getFileName(project, ts) + "_Xiaowei.root";
//
//	mkdir((dstDir + category).c_str(), S_IRWXU);
//
//	std::vector<string> channelNames = {"PSI_amp", "PEI_amp", "PSR_amp"};//, "PLRA_amp"};
//
//	for (auto &channelName : channelNames)
////	std::string channelName = channelNames[1];
//	{
//		std::string dstDirJit = dstDir + category + "/Jitter_" + channelName + "/";
//		std::string dstDirRef = dstDir + category + "/RefTime_" + channelName + "/";
//		std::string dstDirDev = dstDir + category + "/DevTime_" + channelName + "/";
//
//		mkdir(dstDirJit.c_str(), S_IRWXU);
//		mkdir(dstDirRef.c_str(), S_IRWXU);
//		mkdir(dstDirDev.c_str(), S_IRWXU);
//
//
////		testJitter(srcFilePath, dstDirJit,
////				category, "ChB0_" + channelName,
////				category, "ChB1_" + channelName, ts);
////		testRisingEdge(srcFilePath, dstDirRef,
////				category, "ChB0_" + channelName, ts);
////		testDeviation(srcFilePath, dstDirDev,
////				category, "ChB0_" + channelName,
////				category, "ChB1_" + channelName, ts);
//
////		testJitter(srcFilePath, dstDirJit,
////				category, "ChB0_" + channelName,
////				category, "ChB1_" + channelName);
////		testRisingEdge(srcFilePath, dstDirRef,
////				category, "ChB0_" + channelName);
////		testDeviation(srcFilePath, dstDirDev,
////				category, "ChB0_" + channelName,
////				category, "ChB1_" + channelName);
//
//	}

	rootDir = "/Users/kpapke/projects/CLiC/xbox/data/";
	project = "Xbox3_TD24_bo_L3";
	std::string category = "BDStruct";
//	std::string category = "BDPCompr";


	for (int iday=17; iday<18; iday++) {
		ts = TTimeStamp(2018,7,iday,5,51,0);

		srcDir = rootDir + project + "/root/";
		dstDir = rootDir + project + "/pictures/";

		srcFilePath = srcDir + getFileName(project, ts) + ".root";

		mkdir((dstDir + category).c_str(), S_IRWXU);

		std::vector<string> channelNames = {"PSI_amp", "PEI_amp", "PSR_amp"};//, "PLRA_amp"};

		for (auto &channelName : channelNames)
	//	std::string channelName = channelNames[1];
		{
			std::string dstDirJit = dstDir + category + "/Jitter_" + channelName + "/";
			std::string dstDirRef = dstDir + category + "/RefTime_" + channelName + "/";
			std::string dstDirDev = dstDir + category + "/DevTime_" + channelName + "/";

			mkdir(dstDirJit.c_str(), S_IRWXU);
			mkdir(dstDirRef.c_str(), S_IRWXU);
			mkdir(dstDirDev.c_str(), S_IRWXU);

	//		testJitter(srcFilePath, dstDirJit,
	//				"B0Events", channelName,
	//				"B1Events", channelName, ts);
	//		testRisingEdge(srcFilePath, dstDirRef,
	//				"B1Events", channelName, ts);
	//		testDeviation(srcFilePath, dstDirDev,
	//				"B0Events", channelName,
	//				"B1Events", channelName, ts);

			testJitter(srcFilePath, dstDirJit,
					"B0Events", channelName,
					"B1Events", channelName);
			testRisingEdge(srcFilePath, dstDirRef,
					"B1Events", channelName);
			testDeviation(srcFilePath, dstDirDev,
					"B0Events", channelName,
					"B1Events", channelName);

		}
	}
	return 0;
}
