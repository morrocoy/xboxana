#include <iostream>
#include <ctime>

#include "Rtypes.h"
#include "TTimeStamp.h"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RDFHelpers.hxx"

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

////////////////////////////////////////////////////////////////////////////////
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


int main(int argc, char* argv[]) {

	gInterpreter->Declare("#include \"XboxDAQChannel.hxx\"");

//	ROOT::EnableImplicitMT(); // Tell ROOT you want to go parallel

	std::string projectName;
	std::string projectDir;

	std::string srcFilePath; // Input filepath.
	std::string dstFilePath; // Output filepath including filename and type.

	TTimeStamp tsBeginDate; // start date
	TTimeStamp tsEndDate; // end date

	XBOX::XboxAnalyserView viewer;

//	// Xbox2_T24PSI_2 .........................................................
//	projectName = "Xbox2_T24PSI_2";
//	tsBeginDate = TTimeStamp(2018,3,26,0,00,00);
//	tsEndDate = TTimeStamp(2018,10,1,0,00,00);
//
//	viewer.setWindowSize(1650, 500);
//	viewer.setLimitsTime(tsBeginDate, tsEndDate);
//	viewer.setLimitsPAvg(0., 50, 1e6);
//	viewer.setLimitsPLen(0., 400, 1e-9);
//	viewer.setLimitsPCnt(0, 0.7, 1e9);
//	viewer.setLimitsRate(1e-8, 1e-3);
//	viewer.setLimitsBDPosTime(0, 70, 1e-9);
//	viewer.setBins(1000, 100);
//
//	// Xbox3_T24N5_L1 .........................................................
//	projectName = "Xbox3_T24N5_L1";
//
//	tsBeginDate = TTimeStamp(2018,10,25,0,00,00);
//	tsEndDate = TTimeStamp(2019,5,7,0,00,00);
//
//	viewer.setWindowSize(1650, 500);
//	viewer.setLimitsTime(tsBeginDate, tsEndDate);
//	viewer.setLimitsPAvg(0., 45, 1e6);
//	viewer.setLimitsPLen(0., 400, 1e-9);
//	viewer.setLimitsPCnt(0, 1, 1e9);
//	viewer.setLimitsRate(1e-8, 1e-3);
//	viewer.setLimitsBDPosTime(0, 70, 1e-9);
//	viewer.setBins(1000, 100);

//	// Xbox3_T24N4_L2 .........................................................
//	projectName = "Xbox3_T24N4_L2";
//
//	tsBeginDate = TTimeStamp(2018,10,25,0,00,00);
//	tsEndDate = TTimeStamp(2019,5,7,0,00,00);
//
//	viewer.setWindowSize(1650, 500);
//	viewer.setLimitsTime(tsBeginDate, tsEndDate);
//	viewer.setLimitsPAvg(0., 45, 1e6);
//	viewer.setLimitsPLen(0., 400, 1e-9);
//	viewer.setLimitsPCnt(0, 1, 1e9);
//	viewer.setLimitsRate(1e-8, 1e-3);
//	viewer.setLimitsBDPosTime(0, 70, 1e-9);
//	viewer.setBins(1000, 100);

	// Xbox3_TD24_bo_L3 .......................................................
	projectName = "Xbox3_TD24_bo_L3";

	tsBeginDate = TTimeStamp(2018,7,16,0,00,00);
	tsEndDate = TTimeStamp(2019,5,7,0,00,00);

	viewer.setWindowSize(1650, 500);
	viewer.setLimitsTime(tsBeginDate, tsEndDate);
	viewer.setLimitsPAvg(0., 45, 1e6);
	viewer.setLimitsPLen(0., 400, 1e-9);
	viewer.setLimitsPCnt(0, 2.7, 1e9);
	viewer.setLimitsRate(1e-8, 1e-3);
	viewer.setLimitsBDPosTime(0, 70, 1e-9);
	viewer.setBins(1000, 100);

	// Xbox3_TD24_ubo_L4 ......................................................
//	projectName = "Xbox3_TD24_ubo_L4";
//
//	tsBeginDate = TTimeStamp(2018,7,16,0,00,00);
//	tsEndDate = TTimeStamp(2019,5,7,0,00,00);
//
//	viewer.setWindowSize(1650, 500);
//	viewer.setLimitsTime(tsBeginDate, tsEndDate);
//	viewer.setLimitsPAvg(0., 45, 1e6);
//	viewer.setLimitsPLen(0., 400, 1e-9);
//	viewer.setLimitsPCnt(0, 2.7, 1e9);
//	viewer.setLimitsRate(1e-8, 1e-3);
//	viewer.setLimitsBDPosTime(0, 70, 1e-9);
//	viewer.setBins(1000, 100);

	// load data ..............................................................
	projectDir = "/eos/user/k/kpapke/projects/CliC/xbox/data/";
	projectDir = "/Users/kpapke/cernbox/projects/CliC/xbox/data/";
	projectDir = "/Users/kpapke/projects/CLiC/xbox/data/";

	srcFilePath = projectDir + projectName + "/ana/*Default.root";
//	srcFilePath = "/Users/kpapke/cernbox/tmp/*_Default.root";

	viewer.addFiles(srcFilePath);
	viewer.printTreeNames();
//	viewer.printColNames("BDStruct");
	clock_t begin = clock();

	viewer.load(srcFilePath, "N0Events", {"PSI"});
	viewer.load(srcFilePath, "BDStruct", {"B1_PSI", "BDTime"});
	viewer.load(srcFilePath, "BDPCompr", {"B1_PSI"});


	clock_t end = clock();
	printf("Total elapsed time: %.3f\n", double(end - begin) / CLOCKS_PER_SEC);


	// plot recent history ....................................................
//	viewer.clear();
//
//	viewer.setLimitsTime(tsEndDate-60*60*24*14, tsEndDate);
//	viewer.setBins(1000, 100);
//	viewer.setHorzAxis(XBOX::XboxAnalyserView::kTime);
//
//	viewer.plotPLen("N0Events.ChN0_PSI_amp", 30, "a", 0, 1);
//	viewer.plotPAvg("N0Events.ChN0_PSI_amp", 38, "asame", 0, 1);
//	// viewer.drawPCnt("N0Events.ChN0_PSI_amp", kGray, "asame", 0, 1);
//
//	viewer.plotPLen("BDStruct.ChB1_PSI_amp", kGreen+4, "asame", 4, 1);
//	// viewer.drawPCnt("BDStruct.ChB1_PSI_amp", kBlack, "asame", 4, 1);
//	viewer.plotRate("BDStruct.ChB1_PSI_amp", kMagenta+2, "aLsame", 4, 1);
//	viewer.plotPAvg("BDStruct.ChB1_PSI_amp", kAzure+2, "asame", 4, 1);
//
//	viewer.drawAxisHorz();
//	viewer.drawAxisPAvg(kAzure+2);
//	viewer.drawAxisPLen(kGreen+4);
//	// viewer.drawAxisPCnt();
//	viewer.drawAxisRate(kMagenta+2);
//
//	viewer.save(sRootDirectory + sProject + "/pictures/"
//			+ sProject + "_RecentHistoryVsTime.png");
//	viewer.show();

	// plot full history ......................................................
	viewer.clear();
	viewer.setLimitsTime(tsBeginDate, tsEndDate);
	viewer.setBins(1000, 100);
	viewer.setHorzAxis(XBOX::XboxAnalyserView::kPCnt);

	viewer.plotPLen("N0Events.ChN0_PSI_amp", kGray, "a", 0, 1);
	viewer.plotPAvg("N0Events.ChN0_PSI_amp", kGray, "asame", 0, 1);
	// viewer.drawPCnt("N0Events.eval_PSI_amp", kGray, "asame", 0, 1);

	viewer.plotPLen("BDStruct.ChB1_PSI_amp", kGreen+4, "asame", 6, 1);
	// viewer.drawPCnt("BDStruct.ChB1_PSI_amp", kBlack, "asame", 6, 1);
	viewer.plotRate("BDStruct.ChB1_PSI_amp", kMagenta+2, "aLsame", 6, 1);
	viewer.plotPAvg("BDStruct.ChB1_PSI_amp", kAzure+2, "asame", 6, 1);

	viewer.drawAxisHorz();
	viewer.drawAxisPAvg(kAzure+2);
	viewer.drawAxisPLen(kGreen+4);
	// viewer.drawAxisPCnt();
	viewer.drawAxisRate(kMagenta+2);

	viewer.save(projectDir + projectName + "/pictures/"
			+ projectName + "_FullHistoryVsPCnt.png");
	viewer.show();

	// pulse compressor full history ..........................................
	viewer.clear();
	viewer.setLimitsTime(tsBeginDate, tsEndDate);
	viewer.autosetPCntLimits("N0Events.ChN0_PSI_amp"); // adjust pulse count according to time period
	viewer.setBins(1000, 100);
	viewer.setHorzAxis(XBOX::XboxAnalyserView::kPCnt);

	viewer.plotPLen("N0Events.ChN0_PSI_amp", kGray, "a", 0, 1);
	viewer.plotPAvg("N0Events.ChN0_PSI_amp", kGray, "asame", 0, 1);
	// viewer.plotPCnt("N0Events.ChN0_PSI_amp", kGray, "asame", 0, 1);

	viewer.plotPLen("BDPCompr.ChB1_PSI_amp", kGreen+4, "asame", 6, 1);
	// viewer.plotPCnt("BDPCompr.ChB1_PSI_amp", kBlack, "asame", 6, 1);
	viewer.plotRate("BDPCompr.ChB1_PSI_amp", kMagenta+2, "aLsame", 6, 1);
	viewer.plotPAvg("BDPCompr.ChB1_PSI_amp", kAzure+2, "asame", 6, 1);

	viewer.drawAxisHorz();
	viewer.drawAxisPAvg(kAzure+2);
	viewer.drawAxisPLen(kGreen+4);
	// viewer.drawAxisPCnt();
	viewer.drawAxisRate(kMagenta+2);

	viewer.save(projectDir + projectName + "/pictures/"
			+ projectName + "_PCompr_FullHistoryVsPCnt.png");
	viewer.show();

	// plot breakdown location ................................................
	viewer.clear();
	viewer.setBins(100, 100);
	viewer.setHorzAxis(XBOX::XboxAnalyserView::kTime);
//	viewer.setLimitsPCnt(0, 1.6, 1e9);

	viewer.plotBDPosTime("BDStruct.BDTime", kAzure+2, "CONT4Z", 6, 1);
	viewer.save(projectDir + projectName + "/pictures/"
			+ projectName + "_BDPosTimeVsPCnt.png");
	viewer.show();

	// plot signals ...........................................................
//	viewer.clear();
//	int idx = 0;
//	char sfilepath[200];
//	snprintf(sfilepath, 200, "%s%s/pictures/%s_PEI_%d.png",
//			rootDir.c_str(), projectName.c_str(), projectName.c_str(), idx);
////	viewer.plotSignal("BDStruct.ChB0_PSI_amp", idx, kBlack, "");
//	viewer.plotSignal("BDStruct.ChB0_PKIA_amp", "BDStruct.ChB1_PKIA_amp", idx, kBlue);
////	viewer.plotSignal("BDStruct.ChB0_PSI_amp", "BDStruct.ChB1_PSI_amp", idx, kBlue);
////	viewer.plotSignal("BDStruct.ChB0_PEI_amp", "BDStruct.ChB1_PEI_amp", idx, kBlue);
//	viewer.save(sfilepath);
//	viewer.show();

	return 0;
}
