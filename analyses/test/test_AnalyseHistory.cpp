#include <iostream>
#include <ctime>

// root
#include "Rtypes.h"
#include "TFile.h"
#include "TTree.h"
#include "TTimeStamp.h"
#include "TH1D.h"
#include "TH2D.h"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RDFHelpers.hxx"

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

// xbox
#include "XboxAlgorithms.h"
#include "XboxDAQChannel.hxx"
#include "XboxSignalFilter.hxx"

class XboxSignalFilter;



////////////////////////////////////////////////////////////////////////
/// Cutting Function.
/// Compares the timestamp of the given channel date with a predefined time window.
/// \param[in] ch Xbox DAQ channel containing the signal.
/// \return Boolean (true if timestamp is within the time window).
Bool_t cutTimeStamp(const XBOX::XboxDAQChannel &ch0, TTimeStamp ts0, TTimeStamp ts1) {

	if(ch0.getTimeStamp() >= ts0 &&  ch0.getTimeStamp() <= ts1)
		return true;
	else
		return false;
}

////////////////////////////////////////////////////////////////////////
/// Pulse shape parameters .
/// Evaluates pulse length and top level of the pulse for a given
/// relative threshold and within a time windows.
/// \param[in] ch Xbox DAQ channel containing the signal.
/// \param[in] th a threshold.
/// \param[in] lowerlimit the lower limit of the considered time window.
/// \param[in] upperlimit the upper limit of the considered time window.
/// \return    vector of parameters (time at rising edge, time at falling
///            edge, pulse length, pulse peak power, pulse top level).
std::vector<Double_t> evalPulseShape(XBOX::XboxDAQChannel &ch, Double_t th, Double_t t0, Double_t t1)
{
	std::vector<Double_t> p(5);

	ch.flushbuffer();
	p[0] = ch.risingEdge(th, t0, t1); // RisingEdge
	p[1] = ch.fallingEdge(th, t0, t1); // FallingEdge
	p[2] = p[1] - p[0]; // PulseLength
	p[3]  = ch.max(p[0], p[1]); // PulsePowerMax
	p[4]  = ch.mean(p[0], p[1]); // PulsePowerMean

	return p;
}


////////////////////////////////////////////////////////////////////////
/// Plotting function.
/// Plots the history as a 2D scattering plot.
void plotHistory(const std::string &filename, TH2D &h) {

	TStyle* m_gStyle;
	m_gStyle = new TStyle();
	m_gStyle->SetPalette(kRainBow);

	TCanvas *ctd = new TCanvas("c0","Breakdown studies",200,10,700,500);

	h.SetMarkerColor(kBlack);
//	h.GetXaxis()->SetTimeDisplay(1);
//	h.GetXaxis()->SetTimeFormat("%d.%m.%y%F1970-01-01 00:00:00");
	h.SetTitle("history plot; time [s]; power [W]" );

	ctd->Update(); // draws the frame, after which one can change it
	ctd->Modified();


	h.GetXaxis()->SetTitle("time [s]");
	h.GetYaxis()->SetTitle("power");
	h.Draw("scat=1");

	ctd->Update(); // draws the frame, after which one can change it
	ctd->Modified();

	ctd->Print(filename.c_str());
	delete ctd;

}

////////////////////////////////////////////////////////////////////////
/// Analyse Pulse History.
void analysePulseHistory(const std::string &filepath) {

//	ROOT::EnableImplicitMT(); // Tell ROOT you want to go parallel

	// get the tree of non-breakdown events
//	ROOT::RDataFrame dfEvents("N0Events", filepath.c_str(), {"DC_DOWN"});
	ROOT::RDataFrame dfEvents("B0Events", filepath.c_str(), {"PSI_amp"});

	// print list of branches
	std::vector<std::string> colNames = dfEvents.GetColumnNames();
	for(std::string key : colNames)
		printf("%s\n", key.c_str());

	// add thresholds and limits to the data frame
	auto dfEventsTh = dfEvents
						.Define("fMinTimeStamp", "TTimeStamp(2016,4,14,16,00,13)")
						.Define("fMaxTimeStamp", "TTimeStamp(2019,4,14,16,00,15)")
//						.Define("fMinTimeStamp", "TTimeStamp(2018,7,16,23,33,41)")
//						.Define("fMaxTimeStamp", "TTimeStamp(2018,7,16,23,35,42)")
//						.Define("fMinTimeStamp", "TTimeStamp(2017,6,25,18,30,00)")
//						.Define("fMaxTimeStamp", "TTimeStamp(2017,6,25,18,35,00)")
						.Define("fLowerLimit", "0.8e-6")
						.Define("fUpperLimit", "1.9e-6")
						.Define("fTHPulseTop", "0.9");

	// filter breakdown events by pulse count, breakdown flags, and DCDOWN signal
	auto dfEventsF = dfEventsTh
						// check for specific time window
						.Filter(cutTimeStamp, {"PSI_amp", "fMinTimeStamp", "fMaxTimeStamp"});

	auto dfEventsEval = dfEventsF
			            // extract time stamp from arbitrary channel
						.Define("TimeStamp", [](const XBOX::XboxDAQChannel& ch) -> TTimeStamp {
									return ch.getTimeStamp();
								}, {"PSI_amp"})
						// extract pulse count from arbitrary channel
						.Define("PulseCount", [](const XBOX::XboxDAQChannel& ch) -> Double_t {
									return ch.getPulseCount();
								}, {"PSI_amp"})
						// evaluate pulse shape parameters
						.Define("PulseShapeParams",
								evalPulseShape,
								{"PSI_amp", "fTHPulseTop", "fLowerLimit", "fUpperLimit"})
						// extract pulse length from PulseShapeParams
						.Define("fPulseLength", [](std::vector<Double_t> &v) {
									return v[2];
								}, {"PulseShapeParams"})
						// extract pulse top level from PulseShapeParams
						.Define("fPulsePowerMean", [](std::vector<Double_t> &v) {
									return v[4];
								}, {"PulseShapeParams"});


	// number of entries which passed the filters
	auto entries = dfEventsF.Count();

	// create 2D histogram comprising all breakdown locations in time
//	auto htd = dfEventsEval.Histo1D("PulseCount");
//	auto htd = dfEventsEval.Histo1D("fBDTime");

	TTimeStamp ts0(2018,3,1,0,00,00);
	TTimeStamp ts1(2018,10,1,00,00,00);

//	auto hpower = dfEventsEval.Histo2D
//			({"hPower","History of Power", 1000,ts0 ,ts1,1000,0,70e6},
//					"TimeStamp", "fPulsePowerMean");
//
//	auto hplength = dfEventsEval.Histo2D
//			({"hPower","History of pulse length", 1000,ts0 ,ts1,1000,0,200e-9},
//					"TimeStamp", "fPulseLength");

	// print properties and results
	dfEventsEval.Foreach([](TTimeStamp &ts, Double_t val) {
										printf("%s | fPulsePowerMean: %e\n", ts.AsString(), val);
									}, {"TimeStamp", "fPulsePowerMean"});

//	dfEventsEval.Foreach([](XBOX::XboxDAQChannel& ch) {
//										ch.print();
//									}, {"PSI_amp"});

	// export data frame
//	dfEventsEval.Snapshot("myNewTree", "newfile.root");

	// plot history histogram
//	plotHistory("hppower.png", *hpower);
//	plotHistory("hplength.png", *hplength);

	// print number of entities which passed all filters
	std::cout << *entries << " entries passed all filters" << std::endl;
}



////////////////////////////////////////////////////////////////////////
/// Main function.
int main(int argc, char* argv[]) {

	if (argc != 2) {
		printf("Usage: analyseBDLocation file\n");
		return 1;
	}
	std::string filepath = argv[1];

	clock_t begin = clock();
	analysePulseHistory(filepath);
	clock_t end = clock();
	printf("Total elapsed time: %.3f\n", double(end - begin) / CLOCKS_PER_SEC);



//	TFile *file = new TFile(filepath.c_str());
//	TTree *tree = (TTree*)file->Get("B0Events");
//
//	// create pointers of TDAQChannel to read the branch objects for each event
//	XBOX::XboxDAQChannel *ch = new XBOX::XboxDAQChannel();
//	tree->SetBranchAddress("PSI_amp", &ch);
//
//	tree->GetEntry(0);
//
//	XBOX::XboxDAQChannel ch2 = *ch;
//	ch2.print();
//
//	printf("Val %d\n", !(ch2 && ch2));
//
//	delete tree;
//	delete file;
//
//	ROOT::RDataFrame d("B0Events", "/home/kpapke/projects/data/xbox3/Xbox3_TD24_bo_L3_201901.root",
//	ROOT::RDataFrame d("B0Events", filepath.c_str(), {"PSI_amp"});
//	auto df = d.Filter("!PSI_amp");


//	auto df = d.Define<XBOX::XboxDAQChannel>("a", "PSI_amp");

	return 0;
}


