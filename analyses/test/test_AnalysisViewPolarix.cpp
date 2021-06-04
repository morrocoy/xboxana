#include <iostream>
#include <ctime>

#include "Rtypes.h"
#include "TTimeStamp.h"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RDFHelpers.hxx"

#include "XboxAnalyserResult.hxx"
#include "XboxAnalyserEvalBase.hxx"
#include "XboxAnalyserEvalPulse.hxx"
#include "XboxAnalyserEvalBreakdown.hxx"
#include "XboxAnalyserEvalBreakdownTest.hxx"
#include "XboxAnalyserView0.hxx"

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

	ROOT::EnableImplicitMT(); // Tell ROOT you want to go parallel

	std::string sProject;
	std::string sRootDirectory;

	std::string sSrcFilePath; // Input filepath.
	std::string sDstFilePath; // Output filepath including filename and type.

	TTimeStamp tsBeginDate; // start date
	TTimeStamp tsEndDate; // end date

	XBOX::XboxAnalyserView0 viewer;

	sRootDirectory = "/eos/user/k/kpapke/projects/CliC/xbox/data/";
	sRootDirectory = "/Users/kpapke/projects/data/";

	// Xbox2_Polarix .........................................................
	sProject = "Xbox2_Polarix";

	tsBeginDate = TTimeStamp(2019,2,15,00,00,00);
	tsEndDate = TTimeStamp(2019,4,24,00,00,00);

	viewer.setWindowSize(1650, 500);
	viewer.setLimitsTime(tsBeginDate, tsEndDate);
	viewer.setLimitsPAvg(0., 35, 1e6);
	viewer.setLimitsPLen(0., 400, 1e-9);
	viewer.setLimitsPCnt(0.7, 0.78, 1e9);
	viewer.setLimitsRate(1e-8, 1e-3);
	viewer.setLimitsBDPosTime(0, 70, 1e-9);
	viewer.setBins(1000, 100);

	// load data ..............................................................
	sSrcFilePath = sRootDirectory + sProject + "/anal/*Default.root";
	viewer.load(sSrcFilePath);

	// plot full history ......................................................
	viewer.clear();
	viewer.setLimitsTime(tsBeginDate, tsEndDate);
	viewer.setBins(1000, 100);
	viewer.setHorzAxis(XBOX::XboxAnalyserView0::kPCnt);

	viewer.plotPLen("N0Events", kGreen-8, "a", 0, 1);
	viewer.plotPAvg("N0Events", kAzure-9, "asame", 0, 1);
	viewer.plotPCnt("N0Events", kGray, "asame", 0, 1);

	viewer.plotPLen("BDStruct", kGreen+4, "asame", 6, 1);
	viewer.plotPCnt("BDStruct", kBlack, "asame", 6, 1);
	viewer.plotRate("BDStruct", kMagenta+2, "aLsame", 6, 1);
	viewer.plotPAvg("BDStruct", kAzure+2, "asame", 6, 1);

	viewer.drawAxisHorz();
	viewer.drawAxisPAvg(kAzure+2);
	viewer.drawAxisPLen(kGreen+4);
	viewer.drawAxisPCnt();
	viewer.drawAxisRate(kMagenta+2);

	viewer.save(sRootDirectory + sProject + "/pictures/"
			+ sProject + "_FullHistoryVsPCnt.png");
	viewer.show();

	// plot breakdown location ................................................
	viewer.clear();
	viewer.setBins(100, 100);
	viewer.setHorzAxis(XBOX::XboxAnalyserView0::kPCnt);

	viewer.plotBDPosTime("BDStruct", kAzure+2, "CONT4Z", 6, 1);
	//	viewer.plotBDPosTime("B0Events", kBlack, "", 6, 1);
	viewer.save(sRootDirectory + sProject + "/pictures/"
			+ sProject + "_BDPosTimeVsPCnt.png");
	viewer.show();

	return 0;
}
