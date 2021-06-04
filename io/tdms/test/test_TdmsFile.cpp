#include <iostream>
#include <string>
#include <ctime>

#include "Tdms.h"

int main(int argc, char* argv[])
{

	if (argc != 2) {
		printf("Usage: Test_XboxTdmsFileConverter file\n");
		return 1;
	}
	std::string filepath = argv[1];

	// load tdms file
	clock_t begin = clock();

	TDMS::TdmsFile *tdmsfile;
	tdmsfile = new TDMS::TdmsFile(filepath);

	tdmsfile->read();

	clock_t end = clock();
	printf("Total elapsed time: %.3f\n", double(end - begin) / CLOCKS_PER_SEC);

	// print all channels and properties of the first breakdown event:
	ULong64_t n = tdmsfile->getGroupCount();
	for (size_t i=0; i<n; i++) {
		TDMS::TdmsGroup *group = tdmsfile->getGroup(i);
		std::string name = group->getName();

		if (!name.compare(0, 5, "/'Bre")) {
			std::string props = group->getPropertiesAsString();
			printf("%s\n\n", props.c_str());

			size_t nchannel = group->getGroupSize();
			for(size_t j=0; j<nchannel; j++) {
				TDMS::TdmsChannel *ch = group->getChannel(j);
				printf("%s\n", ch->getName().c_str());
			}
			break;
		}
	}


	return 0;
}

