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
//#include "XboxAlgorithms.h"
#include "XboxDAQChannel.hxx"
//#include "XboxSignalFilter.hxx"

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
//std::vector<Double_t> evalPulseShape(XBOX::XboxDAQChannel &ch, Double_t th, Double_t t0, Double_t t1)
//{
//	std::vector<Double_t> p(5);
//
//	ch.flushbuffer();
//	p[0] = ch.getSignalRisingEdge(th, t0, t1); // RisingEdge
//	p[1] = ch.getSignalFallingEdge(th, t0, t1); // FallingEdge
//	p[2] = p[1] - p[0]; // PulseLength
//	p[3]  = ch.getSignalMax(p[0], p[1]); // PulsePowerMax
//	p[4]  = ch.getSignalMean(p[0], p[1]); // PulsePowerMean
//
//	return p;
//}



////////////////////////////////////////////////////////////////////////
/// Analyse Pulse History.
void analysePulseHistory(const std::string &filepath) {

	ROOT::EnableImplicitMT(); // Tell ROOT you want to go parallel

	// get the tree of non-breakdown events
	ROOT::RDataFrame dfEvents("N0Events", filepath.c_str(), {"PSI_amp"});

	// print list of branches
	std::vector<std::string> colNames = dfEvents.GetColumnNames();
	for(std::string key : colNames)
		printf("%s\n", key.c_str());

	// add thresholds and limits to the data frame
	auto dfEventsTh = dfEvents
						.Define("fMinTimeStamp", "TTimeStamp(2017,4,14,16,00,13)")
						.Define("fMaxTimeStamp", "TTimeStamp(2019,4,14,16,00,15)")
						.Define("fLowerLimit", "0.8e-6")
						.Define("fUpperLimit", "1.9e-6")
						.Define("fTHPulseTop", "0.9");

	// filter breakdown events by pulse count, breakdown flags, and DCDOWN signal
	auto dfEventsF = dfEventsTh
						// check for specific time window
						.Filter(cutTimeStamp, {"PSI_amp", "fMinTimeStamp", "fMaxTimeStamp"});

//	auto dfEventsEval = dfEventsF
//			            // extract time stamp from arbitrary channel
//						.Define("TimeStamp", [](const XBOX::XboxDAQChannel& ch) -> TTimeStamp {
//									return ch.getTimeStamp();
//								}, {"PSI_amp"})
//						// extract pulse count from arbitrary channel
//						.Define("PulseCount", [](const XBOX::XboxDAQChannel& ch) -> Double_t {
//									return ch.getPulseCount();
//								}, {"PSI_amp"})
//						// evaluate pulse shape parameters
//						.Define("PulseShapeParams",
//								evalPulseShape,
//								{"PSI_amp", "fTHPulseTop", "fLowerLimit", "fUpperLimit"})
//						// extract pulse length from PulseShapeParams
//						.Define("fPulseLength", [](std::vector<Double_t> &v) {
//									return v[2];
//								}, {"PulseShapeParams"})
//						// extract pulse top level from PulseShapeParams
//						.Define("fPulsePowerMean", [](std::vector<Double_t> &v) {
//									return v[4];
//								}, {"PulseShapeParams"});


	// number of entries which passed the filters
	auto entries = dfEventsF.Count();

//	// print properties and results
//	dfEventsEval.Foreach([](TTimeStamp &ts, Double_t val) {
//										printf("%s | fPulsePowerMean: %e\n", ts.AsString(), val);
//									}, {"TimeStamp", "fPulsePowerMean"});

	// export data frame
//	dfEventsEval.Snapshot("myNewTree", "newfile.root");

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

	return 0;
}



