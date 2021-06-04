#include <iostream>

#include "Tdms.h"
#include "XboxDAQChannel.hxx"


void readtdms(const std::string &filepath, const std::string &channel, int entry) {

	TDMS::TdmsFile *tdmsfile;
	tdmsfile = new TDMS::TdmsFile(filepath.c_str());
	tdmsfile->read();

	Long64_t ngroups = tdmsfile->getGroupCount();
	printf("Number of available groups: %lld\n", ngroups);

	// read channel of first group
	TDMS::TdmsGroup *group = tdmsfile->getGroup(entry);

	printf("\nGroup properties\n");
	printf("----------------\n");
	printf("%s", group->getPropertiesAsString().c_str());

	printf("\nChannels\n");
	printf("----------------\n");

	std::string chprops;
	int nchannels = group->getGroupSize();
//	printf("Number of available channels: %d\n", nchannels);
	for (Int_t ichannel = 0; ichannel < nchannels; ichannel++){

		TDMS::TdmsChannel *ch = group->getChannel(ichannel);
		std::string key = ch->getName();
		printf("%s\n", key.c_str());

		if (!key.compare(channel)) {
			chprops =  ch->getPropertiesAsString();
		}
	}

	printf("\nChannel properties\n");
	printf("----------------\n");
	printf("%s", chprops.c_str());

	delete tdmsfile;

	// example: readtdms("/home/kpapke/projects/data/xbox2/EventData_20180327.tdms", "/'PSR Amplitude'", 0)
}


void readroot(const std::string &filename, const std::string &channel, int entry) {

    TFile *file = new TFile(filename.c_str());
    TTree *tree = (TTree*)file->Get("N0Events");

    // create pointers of TDAQChannel to read the branch objects for each event
    XBOX::XboxDAQChannel *ch = new XBOX::XboxDAQChannel();

    // get branches and set the branch address
    tree->SetBranchAddress("PSI_amp", &ch);
//    TBranch *branch = tree->GetBranch("PSI_amp");
//    branch->SetAddress(&ch);

    // get number of entries
    ULong64_t nEntryCount = tree->GetEntries();

    printf("Number of entries: %llu\n", nEntryCount);
    for(ULong64_t i=0;i<nEntryCount; i++){
    	tree->GetEntry(i);

    	if(ch->getTimeStamp() != ch->getStartTime())
    		printf("Time Difference in group %s\n", ch->getTimeStamp().AsString());

		if (i == entry){
			std::string sChannelName = ch->getChannelName();
			ch->print();
		}
    }
    delete tree;
    delete file;

    // example: readroot("~/projects/data/xbox2/EventData_20180327.root", "PSI_amp", 0)
}



