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

#include "Tdms.h"
#include "XboxTdmsFile.hxx"
#include "XboxDAQChannel.hxx"

//#include "TTimeStamp.h"

// file path functionality
//#include "TDirectory.h"
#include <boost/filesystem.hpp>
namespace FS = boost::filesystem;

int readtdms(const std::string &fileName)
{
	XBOX::XboxTdmsFile tdmsfile(fileName);
	XBOX::XboxDAQChannel channel;

	// get Xbox version from the Tdms Group
	printf("XBox Version: %d\n", tdmsfile.getXboxVersion());
	printf("Number of available channels: %u\n", tdmsfile.getChannelCount());

	Int_t i = 0;
	tdmsfile.loadEntryList();

	printf("XBox Version: %d\n", tdmsfile.getXboxVersion());
	printf("Number of available channels: %u\n", tdmsfile.getChannelCount());

	while (tdmsfile.nextEntry()){
		tdmsfile.getCurrentEntry("PSI_amp", channel); // read specific channel for each event

		channel.getSignal();
		if(i == 17)
			channel.print();
		i++;
	}

	return 0;
}

Int_t read(std::string fileName)
{
	clock_t begin = clock();

    TFile *file = new TFile(fileName.c_str());
    TTree *tree = (TTree*)file->Get("ChannelSet");

    // create pointers of TDAQChannel to read the branch objects for each event
    XBOX::XboxDAQChannel *ch = new XBOX::XboxDAQChannel();

    // get branches and set the branch address
//    TBranch *branch = tree->GetBranch("PSI_amp");
//    branch->SetAddress(&ch);
    tree->SetBranchAddress("PSI_amp", &ch);

    // get number of entries
    ULong64_t nEntryCount = tree->GetEntries();

    printf("Number of entries: %llu\n", nEntryCount);
    for(ULong64_t i=0;i<nEntryCount; i++){
    	tree->GetEntry(i);

    	if(ch->getTimeStamp() != ch->getStartTime())
    		printf("Time Difference in group %s\n", ch->getTimeStamp().AsString());

		if (i==17){
			std::vector<Double_t> x;
		    std::vector<Double_t> y;

			std::string sChannelName = ch->getChannelName();
			ch->print();
		}
    }
    delete tree;
    delete file;

	clock_t end = clock();
	printf("Elapsed time: %.3f\n", double(end - begin) / CLOCKS_PER_SEC);
	return 0;
}


int main(int argc, char* argv[])
{
	clock_t begin = clock();

#if defined (_MSC_VER)
//	TDirectory inpath("E:/cernbox/data/xbox2/");
	FS::path inpath("E:/cernbox/data/xbox2/");
#else
//	TDirectory inpath("/home/kpapke/cernbox/data/xbox2/");
	FS::path inpath("/home/kpapke/cernbox/data/xbox2/");
#endif

	char filename[100];
	snprintf (filename, 100, "EventData_20180202.tdms");

//	for(int i=0; i<100; ++i)
		readtdms((inpath / filename).c_str());

	clock_t end = clock();
	printf("Total elapsed time: %.3f\n", double(end - begin) / CLOCKS_PER_SEC);

	return 0;
}

