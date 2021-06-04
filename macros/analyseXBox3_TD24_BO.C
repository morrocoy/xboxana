#include <iostream>
#include <ctime>

#include "Rtypes.h"
#include "TTimeStamp.h"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RDFHelpers.hxx"

#include "XboxAnalyserEntry.hxx"
#include "XboxAnalyserEvalBreakdown.hxx"
#include "XboxAnalyserView.hxx"


////////////////////////////////////////////////////////////////////////
/// Export function.
/// Write the evaluated pulse parameters from each event into a file
/// using a tree.
/// \param[in] treename The name of the tree.
/// \param[in] filepath The name of the file the data are written to.
/// \param[in] mode The file mode are ROOT standard (default: RECREATE).
void write(const std::string &filepath, const std::string &treename,
		const std::string &branchname, std::vector<XBOX::XboxAnalyserEntry> &entries) {

	TFile *file = new TFile(filepath.c_str(), "RECREATE");
	TTree *tree = new TTree(treename.c_str(), treename.c_str());

	XBOX::XboxAnalyserEntry entry;
	tree->Branch(branchname.c_str(), &entry, 32000, 99);
	for (size_t i=0; i<entries.size(); i++) {
		entry = entries[i];
		tree->Fill();
	}
	tree->Write();

	delete tree;
	delete file;

	printf("%zu events have been written to %s.\n", entries.size(), filepath.c_str());
}


////////////////////////////////////////////////////////////////////////
/// Evaluation of pulse shape and breakdown location.
std::vector<XBOX::XboxAnalyserEntry>  analyse(const std::string &filepath,
		const std::string &sfilter) {

	// create data frame
	TFile *f = new TFile(filepath.c_str());

	// take breakdown events as primary tree
	TTree *tr = (TTree*)f->Get("B0Events");

	// add pulses events before breakdown as friend tree
	tr->AddFriend("B1 = B1Events", filepath.c_str());

	// load data frame
	ROOT::RDataFrame dfEvents(*tr, {"PSI_amp"});

	// filter events by time stamp, breakdown flags, and DCDOWN signal
	auto dfEventsF = dfEvents.Filter(sfilter); // user defined  filter

	// evaluate events
	XBOX::XboxAnalyserEvalBreakdown feval;
	feval.setPulseConfig(0.01, 0.99, 0.9);
	feval.setJitterConfig(0.01, 0.3, 0.3, 0.001);
	auto dfEventsEval = dfEventsF.Define("_model", feval, {"PSI_amp",
			"B1.PSI_amp", "PEI_amp", "B1.PEI_amp", "PSR_amp", "B1.PSR_amp"});

	// number of entries which passed the filters
	auto ptrCount = dfEventsF.Count();

	// get results from data frame.
	auto ptrEntries = dfEventsEval.Take<XBOX::XboxAnalyserEntry>("_model");

	// print time stamp and pulse count for each entry
	dfEventsEval.Foreach(
			[](XBOX::XboxAnalyserEntry &entry) {
				printf("%s | fPulseCount: %llu\n",
						entry.getTimeStamp().AsString(), entry.getPulseCount());
			}, {"_model"});

	// transfer data frame to std::vector
	std::vector<XBOX::XboxAnalyserEntry> entries = *ptrEntries;

	// sorting according to the time stamp (in-built of XBOX::XboxAnalyserBaseModel)
	std::sort (entries.begin(), entries.end());

	// print number of entities which passed all filters
	std::cout << *ptrCount << " entries passed all filters" << std::endl;

	return entries;
}


int analyseXBox3_TD24_BO(const std::string &filepath=
		"/Users/kpapke/projects/data/xbox3/Xbox3_TD24_bo_L3.root") {

	ROOT::EnableImplicitMT(); // Tell ROOT you want to go parallel
	clock_t begin = clock();

	// analyse data set ..................................................
	std::string sfilter;
	std::vector<XBOX::XboxAnalyserEntry> entries;

	std::string rstfilepath = filepath;
	std::string::size_type ipos = rstfilepath.rfind('.', rstfilepath.length());
	if (ipos != std::string::npos)
		rstfilepath.insert(ipos, "_analysed");

	sfilter =
			" (!PERA_amp.getBreakdownFlag())"
			" && (PSR_amp.getBreakdownFlag()"
				" || DC_DOWN.getBreakdownFlag()"
				" || DC_UP.getBreakdownFlag())"
			" && (DC_UP.span()/B1.DC_UP.span() > 2."
				" || DC_DOWN.span()/B1.DC_DOWN.span() > 2.)";
//	entries = analyse(filepath, sfilter);
//	write(rstfilepath, "Structure", "Parameters", entries);


	// data visualisation .................................................
	XBOX::XboxAnalyserView viewer;
	TTimeStamp ts0(2018,7,1,0,00,00);
	TTimeStamp ts1(2019,3,1,0,00,00);

	viewer.setLimitsTime(ts0, ts1);
	viewer.setLimitsPAvg(0., 45e6);
	viewer.setLimitsPLen(0., 200e-9);
	viewer.setLimitsPCnt(0., 2.3e9);
	viewer.setLimitsRate(1e-8, 1e-3);
	viewer.setBins(100, 100);

	viewer.load(rstfilepath, "Structure", "Parameters");

	viewer.plotPulseRate("pictures/bdrate.png");
	viewer.plotHistory("pictures/history.png");
	viewer.plotBreakdownTime("pictures/bdtime.png");

	clock_t end = clock();
	printf("Total elapsed time: %.3f\n", double(end - begin) / CLOCKS_PER_SEC);

	return 0;
}


//int main(int argc, char* argv[]) {
//	gInterpreter->Declare("#include \"XboxDAQChannel.hxx\"");
//	main_r();
//}

