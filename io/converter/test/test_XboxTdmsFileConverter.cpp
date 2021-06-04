#include <cstring>
#include <stdlib.h>

#include <iostream>
#include <cstdlib>
#include <string>
#include <ctime>
#include <stdio.h>
#include <sstream>

// root core
#include "Rtypes.h"
#include "TFile.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"

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

#include "Tdms.h"
#include "XboxTdmsFileConverter.hxx"
#include "XboxDAQChannel.hxx"

//#include "TTimeStamp.h"


int readtdms(const std::string &filepath,
		const std::string &channelname="PSI_amp", int ievent=0)
{
	XBOX::XboxTdmsFileConverter converter(filepath);
	XBOX::XboxDAQChannel channel;

	// get Xbox version from the Tdms Group
	printf("XBox Version: %d\n", converter.getXboxVersion());
	printf("Number of available channels: %llu\n", converter.getChannelCount());

	Int_t i = 0;
	converter.loadEntryList();

	printf("XBox Version: %d\n", converter.getXboxVersion());
	printf("Number of available channels: %llu\n", converter.getChannelCount());

	while (converter.nextEntry()){
		converter.convertCurrentEntry(channelname, channel); // read specific channel for each event
		channel.getSignal();

		if(i == ievent){
			channel.print();
			break;
		}

//		for(std::string key : converter.getChannelNames()){
//			converter.convertCurrentEntry(key, channel); // read specific channel for each event
//			channel.getSignal();
//
//			if(channel.getBreakdownFlag()) {
//				channel.print();
//			}
//
//		}
//
//		if(channel.getLogType()==0)
//			break;

		i++;
	}

	return 0;
}

void plotChannel(XBOX::XboxDAQChannel *ch0, XBOX::XboxDAQChannel *ch1) {

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
	c1.Print("test.png");
	printf("plot at %s\n", ch0->getTimeStamp().AsString("s"));

}

Int_t readroot(std::string fileName,
		const std::string &channelname="PSI_amp", int ievent=0)
{
	clock_t begin = clock();

	// open file
	TFile file(fileName.c_str());

	// access specific tree from root file.
	// "N0Events" - normal events.
	// "B0Events" - breakdow events.
	// "B1Events" - event before the breakdown events.
	TTree *ptree = (TTree*)file.Get("N0Events");

	// pointer to access the specified channel for each event
	XBOX::XboxDAQChannel *pch = new XBOX::XboxDAQChannel();
	ptree->SetBranchAddress(channelname.c_str(), &pch);

	// get number of entries
	ULong64_t nEventCount = ptree->GetEntries();
	printf("Number of entries: %llu\n", nEventCount);

	for(size_t i=0; i<nEventCount; i++) {
		ptree->GetEntry();
		std::vector<Double_t> y0 = pch->getSignal();

	}

	// read and print data
	ptree->GetEntry(ievent);


	plotChannel(pch, pch);
	pch->print();


	delete pch;
	delete ptree;


	clock_t end = clock();
	printf("Elapsed time: %.3f\n", double(end - begin) / CLOCKS_PER_SEC);
	return 0;
}

std::string getFileExt(std::string& s) {

	std::size_t pos = s.rfind('.');
	std::string ext;

	if (pos != std::string::npos)
		ext = s.substr(pos+1);
	return ext;
}


int main(int argc, char* argv[])
{

	if (argc != 2) {
		printf("Usage: Test_XboxTdmsFileConverter file\n");
		return 1;
	}

	std::string filepath = argv[1];
	clock_t begin = clock();

	printf("ext %s\n ", getFileExt(filepath).c_str());

//	Long_t val;
//	convertStrToNum(val, "345");
//	printf("val %d\n ", val);
//	return 0;

	if(!getFileExt(filepath).compare("tdms"))
		readtdms(filepath.c_str());
	else if(!getFileExt(filepath).compare("root"))
		readroot(filepath.c_str(), "PSI_amp", 500);
	else
		printf("File must by of type 'tdms' or 'root'\n");

	clock_t end = clock();
	printf("Total elapsed time: %.3f\n", double(end - begin) / CLOCKS_PER_SEC);

	return 0;
}

