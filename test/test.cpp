#include <iostream>     
#include <stdlib.h>
#include <string>
#include <vector>
#include <map>
#include <ctime>


// root core
#include "Rtypes.h"
/*#include "TFile.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"

*/

#include <Tdms.h>
#include <XboxDAQChannel.hxx>
#include <XboxDataType.hxx>
Int_t main()
{	
	const std::string filename = "../../data/xbox2/EventData_20180202.tdms";
	
	clock_t tbegin = clock();

	TDMS::TdmsReader parser(filename);
	parser.read();

	Long64_t ngroups = parser.getGroupCount();
	printf("Number of available groups: %lld\n", ngroups);

	// read channel of first group
	TDMS::TdmsGroup *group = parser.getGroup(0);
	Int_t nchannels = group->getGroupSize(); 
	printf("Channels:\n");
	for (Int_t ichannel = 0; ichannel < nchannels; ichannel++){
		TDMS::TdmsChannel *ch = group->getChannel(ichannel);
		std::string key = ch->getName();
		printf("%s\n", key.c_str());
	}
	printf("Number of available channels: %d\n", nchannels);

	XBOX::XboxDAQChannel ch;
	ch.print();

	clock_t tend = clock();
	printf("Elapsed time: %.3f\n", double(tend - tbegin) / CLOCKS_PER_SEC);

	return 0;
}