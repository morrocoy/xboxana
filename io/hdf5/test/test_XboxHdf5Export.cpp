#include <iostream>
#include <ctime>

#include "Rtypes.h"

#include "Tdms.h"
#include "XboxTdmsFileConverter.hxx"
#include "XboxDAQChannel.hxx"
#include "XboxH5File.hxx"

Int_t writeH5(const Char_t *outfile, const Char_t *filename){

	XBOX::XboxTdmsFileConverter converter(filename);

	if (!converter.isValidXboxVersion())
		return -1;

	UInt_t nchannel = converter.getChannelCount();
	Int_t version = converter.getXboxVersion();

	printf("XBox Version: %d\n", version);
	printf("Number of available channels: %u\n", nchannel);

	// define set of xbox daq channels
	std::vector<std::string> vchannelname = converter.getChannelNames();

	for (UInt_t i=0; i < nchannel; i++){
		printf("Channel %u: %s\n", i, vchannelname[i].c_str());
	}

	std::string groupname;
	XBOX::XboxH5File h5file(outfile);

	Int_t count = 0;
	converter.loadEntryList();
	while (converter.nextEntry()){
		groupname = converter.getCurrentEntryGroup();
		h5file.addGroup(groupname);

		for(std::string channelname: vchannelname){
			XBOX::XboxDAQChannel channel;
			converter.convertCurrentEntry(channelname, channel);
			
			if(!channel.isEmpty()) // skip empty data sets
				h5file.addDataSet(groupname, channel);
		}
		count++;
//		printf("Count : %d\n", count);
	}

	return 0;  // successfully terminated
}

void replaceExt(std::string& s, const std::string& newExt) {

   std::string::size_type i = s.rfind('.', s.length());

   if (i != std::string::npos)
      s.replace(i+1, newExt.length(), newExt);

}


int main(int argc, char* argv[]) {

	if (argc != 2) {
		printf("Usage: Test_XboxHdf5Export tdms file\n");
		return 1;
	}

	std::string source = argv[1];
	std::string targetname = source;
	replaceExt(targetname, "h5");

	clock_t begin = clock();
	writeH5(targetname.c_str(), source.c_str());
	clock_t end = clock();
	printf("Total elapsed time: %.3f\n", double(end - begin) / CLOCKS_PER_SEC);

	return 0;
}

