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
/// Cutting Function.
/// Compares the pulse count of two channel data.
/// \param[in] ch0 Xbox DAQ channel containing the current signal.
/// \param[in] ch1 Xbox DAQ channel containing the previous signal.
/// \return Boolean (true if pulse counts are subsequent and differ by one).
Bool_t cutIncrementalPulseCount(const XBOX::XboxDAQChannel &ch0,
		XBOX::XboxDAQChannel &ch1) {
	// check whether breakdown event and the event before provide pulse counts which differ
	// only by one.
	if ((ch0.getPulseCount() - ch1.getPulseCount()) == 1)
		return true;
	else
		return false;
}

////////////////////////////////////////////////////////////////////////
/// Cutting Function.
/// Checks the breakdown flags of a list of channels.
/// Applies a logic AND function to the given flags.
/// \param[in] vec a vector of Xbox DAQ channels.
/// \return Boolean (true if all flags are true).
Bool_t cutConjunctiveBreakdownFlag(std::vector<XBOX::XboxDAQChannel> vec) {

	Bool_t bval = true;
	for (auto ch: vec)
		bval &= ch.getBreakdownFlag();

	return bval;
}

////////////////////////////////////////////////////////////////////////
/// Cutting Function.
/// Checks the breakdown flags of a list of channels.
/// Applies a logic OR function to the given flags.
/// \param[in] vec a vector of Xbox DAQ channels.
/// \return Boolean (true if one of the flags is true).
Bool_t cutDisjunctiveBreakdownFlag(std::vector<XBOX::XboxDAQChannel> vec) {

	Bool_t bval = false;
	for (auto ch: vec)
		bval |= ch.getBreakdownFlag();

	return bval;
}

////////////////////////////////////////////////////////////////////////
/// Cutting Function.
/// Analyses the difference signal between two channels.
/// \param[in] ch0 the Xbox DAQ channel containing the first signal.
/// \param[in] ch1 the Xbox DAQ channel containing the second signal.
/// \param[in] th a threshold.
/// \return Boolean (true if the difference exceeds the threshold).
Bool_t cutMaxDiffThreshold(XBOX::XboxDAQChannel &ch0,
		XBOX::XboxDAQChannel &ch1, double th) {

	ch0.flushbuffer();
	ch1.flushbuffer();

	std::vector<Double_t> y0 = ch0.getSignal();
	std::vector<Double_t> y1 = ch1.getSignal();
	Double_t max = 0; // maximum deviation between break down event and previous event
	if (y0.size() == y1.size()) {

		for(size_t i=0; i<y0.size(); i++)
			 if (fabs(y0[i] - y1[i]) > max)
				 max = fabs(y0[i] - y1[i]);
	}

	if (max > th)
		return true;
	else
		return false;
}

////////////////////////////////////////////////////////////////////////
/// Delay on Threshold.
/// Evaluates the jitter between two subsequent signals based on a threshold.
/// \param[in] ch0 the Xbox DAQ channel containing the first signal.
/// \param[in] ch1 the Xbox DAQ channel containing the second signal.
/// \param[in] th a threshold.
/// \param[in] lowerlimit the lower limit of the considered time window.
/// \param[in] upperlimit the upper limit of the considered time window.
/// \return The time delay between the two signals.
Double_t evalDelayOnThreshold(XBOX::XboxDAQChannel &ch0, XBOX::XboxDAQChannel &ch1,
		Double_t th, Double_t lowerlimit, Double_t upperlimit)
{

	Int_t nsamples = 10001;
	Double_t lowerbound=0;
	Double_t upperbound=0;
	ch0.getTimeAxisBounds(lowerbound, upperbound);
	if(lowerlimit == -1)
		lowerlimit = lowerbound;
	if(upperlimit == -1)
		upperlimit = upperbound;

	// read data from channel
	XBOX::XboxSignalFilter filter(false, false);
	std::vector<Double_t> x0_intp = XBOX::linspace(lowerlimit, upperlimit, nsamples);
	std::vector<Double_t> y0_intp = filter(ch0, x0_intp);
	std::vector<Double_t> y1_intp = filter(ch1, x0_intp);


	// determine the time delay between the signals based on exceeding threshold
	Double_t norm = *std::max_element(y0_intp.begin(), y0_intp.end()); // take signal 1 maximum as norm reference
	Double_t thlevel = th * norm; // absolute threshold

	Double_t idx0 = 0.;
	for(UInt_t i=1; i<y0_intp.size(); i++){
		if(y0_intp[i] > thlevel){
			idx0 = Double_t(i - (y0_intp[i] - thlevel) / (y0_intp[i] - y0_intp[i-1]));
			break;
		}
	}
	Double_t idx1 = 0.;
	for(UInt_t i=1; i<y1_intp.size(); i++){
		if(y1_intp[i] > thlevel){
			idx1 = Double_t(i - (y1_intp[i] - thlevel) / (y1_intp[i] - y1_intp[i-1]));
			break;
		}
	}

	Double_t delay = (idx1 - idx0) * (x0_intp[1] - x0_intp[0]);
	return delay;
}

////////////////////////////////////////////////////////////////////////
/// Deviation Point.
/// Evaluates the rising edge of the pulse and the moment of the breakdown.
/// The first is evaluated by means of the previous pulse. The latter is
/// derived from the moment where current and previous signal start to
/// deviate from each other.
/// \param[in] ch0 the Xbox DAQ channel containing the current signal.
/// \param[in] ch1 the Xbox DAQ channel containing the previous signal.
/// \param[in] th a threshold applied on the signals.
/// \param[in] thp a threshold applied on the derivative of the signals.
/// \param[in] lowerlimit the lower limit of the considered time window.
/// \param[in] upperlimit the upper limit of the considered time window.
/// \return    vector of edge_dev, the time when previous and current signal
///            start to deviate, and edge_prev, the time of the rising edge
///            of the pulse of the previous pulse.
std::vector<Double_t> evalDeviationPoint(XBOX::XboxDAQChannel &ch0, XBOX::XboxDAQChannel &ch1,
		Double_t jitter, Double_t th, Double_t thp,
		Double_t lowerlimit=-1, Double_t upperlimit=-1)
{
	// edges[0] the time of the rising edge of the pulse of the previous pulse.
	// edges[1] the time when previous and current signal start to deviate.
	std::vector<Double_t> edges(2);

	// align signals
	ch1.setStartOffset(-jitter);

	Int_t nsamples = 10001;
	Double_t lowerbound=0;
	Double_t upperbound=0;
	ch0.getTimeAxisBounds(lowerbound, upperbound);
	if(lowerlimit == -1)
		lowerlimit = lowerbound;
	if(upperlimit == -1)
		upperlimit = upperbound;

	// read data from channel
	XBOX::XboxSignalFilter filter(false, false); // signal filter
	XBOX::XboxSignalFilter filterp(true, false); // signal derivative filter
	std::vector<Double_t> x0_intp = XBOX::linspace(lowerlimit, upperlimit, nsamples);
	std::vector<Double_t> y0_intp = filter(ch0, x0_intp);
	std::vector<Double_t> y1_intp = filter(ch1, x0_intp);
	std::vector<Double_t> y1p_intp = filterp(ch1, x0_intp);

	Double_t thlevel;
	size_t th_index;

	// find moment just before rising edge of L1 signal. Start of rising edge
	// defined as sample at which derivative of signal first crosses a threshold.
	// Threshold is a defined as a fraction of the signal peak.
	thlevel = thp * (*std::max_element(y1p_intp.begin(), y1p_intp.end()));
	th_index = 0;
	for(UInt_t i=0; i<y1p_intp.size(); i++){
		if(y1p_intp[i] > thlevel){
			th_index = i;
			break;
		}
	}

	// use above edge information to offset signal such that the start of compressed
	// pulse is at zero, disregarding the ramp just before the compressed pulse.
	std::vector<Double_t> y1o_intp(y1p_intp.size());
	for(UInt_t i=0; i<y1o_intp.size(); i++)
		y1o_intp[i] = y1_intp[i] - y1_intp[th_index];

	// get difference between breakdown and L1 signal
	std::vector<Double_t> diff_intp(y0_intp.size());
	for(UInt_t i=0; i<diff_intp.size(); i++)
		diff_intp[i] = fabs(y0_intp[i] - y1_intp[i]);

	// Find the time at which the offset L1 signal crosses a threshold.
	// Threshold is defined as a fraction of the signal peak.
	thlevel = th * (*std::max_element(y1o_intp.begin(), y1o_intp.end()));
	th_index = 0;
	for(UInt_t i=0; i<y1o_intp.size(); i++){
		if(y1o_intp[i] > thlevel){
			th_index = i;
			break;
		}
	}

	// edges[0] the time of the rising edge of the pulse of the previous pulse.
	if(th_index <= 0)
		edges[0] = x0_intp.front();
	else if(th_index >= x0_intp.size()-1)
		edges[0] = x0_intp.back();
	else{
		edges[0] = (thlevel - y1o_intp[th_index])
				/ (y1o_intp[th_index] - y1o_intp[th_index-1])
				* (x0_intp[1] - x0_intp[0]) + x0_intp[th_index];
	}

	// Find the time at which the  difference signal crosses a threshold.
	// Threshold is defined as a fraction of the signal peak.
	thlevel = th * (*std::max_element(diff_intp.begin(), diff_intp.end()));
	th_index = 0;
	for(UInt_t i=0; i<diff_intp.size(); i++){
		if(diff_intp[i] > thlevel){
			th_index = i;
			break;
		}
	}

	// edges[1] the time when previous and current signal start to deviate.
	if(th_index <= 0)
		edges[1] = x0_intp.front();
	else if(th_index >= x0_intp.size()-1)
		edges[1] = x0_intp.back();
	else{
		edges[1] = (thlevel - diff_intp[th_index])
				/ (diff_intp[th_index] - diff_intp[th_index-1])
				* (x0_intp[1] - x0_intp[0]) + x0_intp[th_index];
	}

	return edges;
}


////////////////////////////////////////////////////////////////////////
/// Plotting function.
/// Plots two subsequent signals with markers for the rising edge of the
/// pulse and the moment of the breakdown.
/// The first is evaluated by means of the previous pulse. The latter is
/// derived from the moment where current and previous signal start to
/// deviate from each other. Both involve the use of thresholds.
/// \param[in] ch0 the Xbox DAQ channel containing the current signal.
/// \param[in] ch1 the Xbox DAQ channel containing the previous signal.
/// \param[in] jitter the time delay between the two signals.
/// \param[in] lowerlimit the lower limit of the considered time window.
/// \param[in] upperlimit the upper limit of the considered time window.
/// \param[in] xmarkers a vector of arguments used for horizontal markers
/// \return The time delay between rising edge and the moment of the breakdown.
void plotSignal(XBOX::XboxDAQChannel &ch0, XBOX::XboxDAQChannel &ch1,
		Double_t jitter=0, Double_t lowerlimit=-1, Double_t upperlimit=-1,
		const std::vector<Double_t> &xmarkers=std::vector<Double_t>()) {

	Int_t nsamples = 10001;
	Double_t lowerbound=0;
	Double_t upperbound=0;
	ch0.getTimeAxisBounds(lowerbound, upperbound);
	if(lowerlimit == -1)
		lowerlimit = lowerbound;
	if(upperlimit == -1)
		upperlimit = upperbound;

	// align signals
	ch1.setStartOffset(-jitter);

	// read data over the entire argument range of the channels
	XBOX::XboxSignalFilter filter(false, false);
	std::vector<Double_t> x = XBOX::linspace(lowerlimit, upperlimit, nsamples);
	std::vector<Double_t> y0 = filter(ch0, x);
	std::vector<Double_t> y1 = filter(ch1, x);
	std::vector<Double_t> yd(x.size()); // difference between y0 and y1
	for(auto ityd=yd.begin(), it0=y0.begin(),
			it1=y1.begin(); ityd != yd.end(); ++ityd, ++it0, ++it1)
		*ityd = fabs(*it0 - *it1);

	// general parameters
	std::string name = ch0.getChannelName();
	TTimeStamp ts = ch0.getTimeStamp();
	ULong64_t count = ch0.getPulseCount();

	// create figure
	char stitle[200];
	snprintf (stitle, 200, "%s %s PulseCount: %llu", name.c_str(), ts.AsString(), count);
	TCanvas *c1 = new TCanvas("c1",stitle,200,10,700,500);
	c1->SetGrid();

	// add graphs
	TMultiGraph *mg = new TMultiGraph();
	TGraph *gr[3];

	gr[0] = new TGraph(x.size(), &x[0], &y0[0]);
	gr[1] = new TGraph(x.size(), &x[0], &y1[0]);
	gr[2] = new TGraph(x.size(), &x[0], &yd[0]);

	gr[0]->SetLineColor(kRed);
	gr[0]->SetLineWidth(1);
	gr[0]->SetMarkerColor(kRed);
//	gr[0]->SetMarkerStyle(20);
//	gr[0]->SetMarkerSize(1);
	mg->Add(gr[0]);

	gr[1]->SetLineColor(kBlue);
	gr[1]->SetLineWidth(1);
	gr[1]->SetMarkerColor(kBlue);
//	gr[1]->SetMarkerStyle(20);
//	gr[1]->SetMarkerSize(0.1);
	mg->Add(gr[1]);

	gr[2]->SetLineColor(kGreen+2);
	gr[2]->SetLineWidth(1);
	gr[2]->SetMarkerColor(kGreen+2);
//	gr[2]->SetMarkerStyle(20);
//	gr[2]->SetMarkerSize(0.8);
	mg->Add(gr[2]);

	mg->SetTitle(stitle);
	mg->GetXaxis()->SetTitle("Time [s]");
	mg->GetYaxis()->SetTitle("Amplitude");
	mg->Draw("ACP");

	c1->Update(); // draws the frame, after which one can change it
	c1->Modified();

	// add markers for edge detection
	std::vector<TLine*> vlines(xmarkers.size());
	for(size_t i=0; i<xmarkers.size(); i++){
		vlines[i] = new TLine(xmarkers[i], c1->GetUymin(), xmarkers[i], c1->GetUymax());
		vlines[i]->SetLineWidth(1);
		vlines[i]->SetLineColor(kBlack);
		vlines[i]->Draw();
		vlines[i]->SetNDC(false);
	}

	// legend
	auto legend = new TLegend(0.11,0.7,0.4,0.89);//,0.48,0.9);
	legend->AddEntry(gr[0], (name + "_B0").c_str(),"l");
	legend->AddEntry(gr[1], (name + "_B1").c_str(),"l");
	legend->AddEntry(gr[2], (name + "_BD0 - " + name + "_BD1").c_str(),"l");
	legend->SetBorderSize(1);
	legend->SetFillColor(0);
	legend->SetTextSize(0);
	legend->Draw();

	// save file
	c1->Print((name + "_" + ts.AsString() + ".pdf").c_str());

	delete legend;
	for(auto line:vlines)
		delete line;
	delete gr[0];
	delete gr[1];
	delete gr[2];
	delete mg;
	delete c1;
}

////////////////////////////////////////////////////////////////////////
/// Plotting function.
/// Plots the breakdown location as a 2D histogram.
void plotBDLocation(const std::string &filename, TH2D &h) {

	TStyle* m_gStyle;
	m_gStyle = new TStyle();
	m_gStyle->SetPalette(kRainBow);

	// create plot
	TCanvas *ctd = new TCanvas("c0","Breakdown studies",200,10,700,500);
	h.GetXaxis()->SetTitle("position [s]");
	h.GetYaxis()->SetTitle("number of pulses");
	h.Draw("COLZ");

	ctd->Update(); // draws the frame, after which one can change it
	ctd->Modified();

	ctd->Print(filename.c_str());
	delete ctd;

}

////////////////////////////////////////////////////////////////////////
/// Analyse breakdown location.
void analyseBDLocation(const std::string &filepath) {

	ROOT::EnableImplicitMT(); // Tell ROOT you want to go parallel

	TFile *f = new TFile(filepath.c_str());
	TTree *trEvents = (TTree*)f->Get("B0Events");
	trEvents->AddFriend("B1 = B1Events", filepath.c_str()); // 1st event before the breakdown events (fLogType=1)
	trEvents->AddFriend("B2 = B2Events", filepath.c_str()); // 2nd event before the breakdown events (fLogType=2)

	ROOT::RDataFrame dfEvents(*trEvents, {"PSI_amp"});
//	ROOT::RDataFrame d("B0Events", filename, {"PSI_amp"}); // alternative if no friend trees is required

	// print list of branches
	std::vector<std::string> colNames = dfEvents.GetColumnNames();
	for(std::string key : colNames)
		printf("%s\n", key.c_str());

	// add thresholds and limits to the data frame
	auto dfEventsTh = dfEvents
						.Define("fMinTimeStamp", "TTimeStamp(2017,4,14,16,00,13)")
						.Define("fMaxTimeStamp", "TTimeStamp(2019,4,14,16,00,15)")
//						.Define("fMinTimeStamp", "TTimeStamp(2018,4,30,06,00,00)")
//						.Define("fMaxTimeStamp", "TTimeStamp(2018,4,30,17,00,00)")
						.Define("fLowerLimit", "0.8e-6")
						.Define("fUpperLimit", "1.9e-6")
						.Define("fThPulseStart", "0.4")
						.Define("fTHPulseTop", "0.9")
						.Define("fThDeviation", "0.1")
						.Define("fThDerivative", "0.2")
						.Define("thDCDown", "0.005");

	// filter breakdown events by pulse count, breakdown flags, and DCDOWN signal
	auto dfEventsF = dfEventsTh
						// check for specific time window
						.Filter(cutTimeStamp, {"DC_DOWN", "fMinTimeStamp", "fMaxTimeStamp"})
						// check current and previous pulse for incremental pulse counts
						.Filter(cutIncrementalPulseCount, {"DC_DOWN", "B1.DC_DOWN"})
						// check for set breakdown flags
						.Filter(ROOT::RDF::PassAsVec<3, XBOX::XboxDAQChannel>
							(cutDisjunctiveBreakdownFlag), {"DC_DOWN", "DC_UP", "PSR_log"})
						// check for unset breakdown flags
						.Filter(ROOT::RDF::PassAsVec<1, XBOX::XboxDAQChannel>
							(ROOT::RDF::Not(cutConjunctiveBreakdownFlag)), {"PERA_amp"})
						// check whether DCDOWN threshold is exceeded
						.Filter(cutMaxDiffThreshold, {"DC_DOWN", "B1.DC_DOWN", "thDCDown"});

	auto dfEventsEval = dfEventsF
			            // extract pulse count from arbitrary channel
						.Define("PulseCount", [](const XBOX::XboxDAQChannel& ch) -> Double_t {
									return ch.getPulseCount();
								}, {"PSI_amp"})
						// evaluate jitter between previous and current signal using PSI_amp
						.Define("fJitter",
								evalDelayOnThreshold,
								{"PSI_amp", "B1.PSI_amp",
										"fThPulseStart", "fLowerLimit", "fUpperLimit"})
						// evaluate delay of deviation between previous and current signal using PEI_amp
						.Define("fBDTimeTra",
								evalDeviationPoint,
								{"PEI_amp", "B1.PEI_amp", "fJitter",
										"fThDeviation", "fThDerivative", "fLowerLimit", "fUpperLimit"})
						// evaluate delay of deviation between previous and current signal using PSR_amp
						.Define("fBDTimeRef",
								evalDeviationPoint,
								{"PSR_amp", "B1.PSR_amp", "fJitter",
										"fThDeviation", "fThDerivative", "fLowerLimit", "fUpperLimit"})
						// evaluate breakdown location as half the difference of fBDTimeTra and fBDTimeRef
						.Define("fBDTime", [](std::vector<Double_t> v1, std::vector<Double_t> v2) -> Double_t {
													return (0.5 * (v2[1] - v2[0] - v1[1] + v1[0]));
												}, {"fBDTimeTra", "fBDTimeRef"});


	// number of entries which passed the filters
	auto entries = dfEventsF.Count();

	// create 2D histogram comprising all breakdown locations in time
//	auto htd = dfEventsEval.Histo1D("PulseCount");
//	auto htd = dfEventsEval.Histo1D("fBDTime");
	auto htd = dfEventsEval.Histo2D
			({"htd","Breakdown statistics", 200,-5e-9,70e-9,200,0e8,5.5e8},
					"fBDTime", "PulseCount");

	// print properties and results
	dfEventsEval.Foreach([](const XBOX::XboxDAQChannel& ch, Double_t val) {
										printf("%s | fBDTime: %e\n", ch.getTimeStamp().AsString(), val);
									}, {"B1.DC_DOWN", "fBDTime"});

	// plot signals
//	dfEventsEval.Foreach(plotSignal, {"PEI_amp", "B1.PEI_amp", "fJitter",
//			                          "fLowerLimit", "fUpperLimit", "fBDTimeTra"});

	// export data frame
//	dfEventsEval.Snapshot("myNewTree", "newfile.root");

	// plot histogram
	plotBDLocation("htd.png", *htd);
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
	analyseBDLocation(filepath);
	clock_t end = clock();
	printf("Total elapsed time: %.3f\n", double(end - begin) / CLOCKS_PER_SEC);

	return 0;
}


