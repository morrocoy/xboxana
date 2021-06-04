/*
 * TXboxFileConverter.cxx
 *
 *  Created on: Sep 14, 2018
 *      Author: kpapke
 */

#include <fstream>
#include <ctime>

#include "TFile.h"
#include "TTree.h"

#include "Tdms.h"

#include "XboxDataType.hxx"
#include "XboxDAQChannel.hxx"
#include "XboxTdmsFileConverter.hxx"
#include "XboxFileConverter.hxx"

#ifdef HDF5_FOUND
#include "XboxH5File.hxx"
#endif

#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif



XboxFileConverter::XboxFileConverter()
{
	init();
}

XboxFileConverter::XboxFileConverter(const Char_t *filename)
{
	init();
	addFile(filename);
}

XboxFileConverter::XboxFileConverter(const std::string &filename)
{
	init();
	addFile(filename.c_str());
}

XboxFileConverter::XboxFileConverter(const std::vector<std::string> &filenames)
{
	init();
	for (std::string filename : filenames)
		addFile(filename);
}

XboxFileConverter::~XboxFileConverter()
{

}

void XboxFileConverter::init()
{
	fXboxVersion = 0;
	fVerbose = true;
}

void XboxFileConverter::clear()
{
	fChannelNames.clear();
	fInFiles.clear();
}

void XboxFileConverter::reset()
{
	clear();
	init();
}

void XboxFileConverter::addFile(const Char_t *filename)
{
	XboxTdmsFileConverter tdmsconverter(filename);
	Int_t version = tdmsconverter.getXboxVersion();
	if(version != 0){
		if(fInFiles.empty()){
			fInFiles.push_back(filename);
			fXboxVersion = version;
			fChannelNames = tdmsconverter.getChannelNames();
		}
		else if(version == fXboxVersion) {
			std::vector<std::string> keys = tdmsconverter.getChannelNames();
			if (keys.size() != fChannelNames.size())
				printf("ERROR: Could not append file \"%s\". Xbox version "
									"differs.\n", filename);
			else {
				size_t ivalid=0;
				for (size_t i=0; i<keys.size(); i++)
					if (keys[i] == fChannelNames[i])
						ivalid++;
				if (ivalid == keys.size())
					fInFiles.push_back(filename);
				else
					printf("ERROR: Could not append file \"%s\". Xbox version "
														"differs.\n", filename);
			}
		}
		else
			printf("ERROR: Could not append file \"%s\". Xbox version "
					"differs.\n", filename);
	}
//	else
//		printf("ERROR: Could not append file \"%s\". Xbox version "
//				"is not valid.\n", filename);
}


Int_t XboxFileConverter::write(const Char_t* filename, const Char_t* mode){

	if (fInFiles.empty())
		return -1;

	Long64_t nchannel = fChannelNames.size();

	if (fVerbose) {
		printf("----------------------------------------------------\n");
		printf("Start conversion for %zu input file(s)\n", fInFiles.size());
		printf("Xbox Version: %d\n", fXboxVersion);
		printf("Maximum number of channels: %lld\n", nchannel);
	}

	// define set of xbox daq channels
	std::vector<XboxDAQChannel> channelsetN0; // N0 cache
	std::vector<XboxDAQChannel> channelsetB0; // B0 cache
	std::vector<XboxDAQChannel> channelsetB1; // B1 cache
//	std::vector<XboxDAQChannel> channelsetB2; // B2 cache
	channelsetN0.resize(nchannel);
	channelsetB0.resize(nchannel);
	channelsetB1.resize(nchannel);
//	channelsetB2.resize(nchannel);

	// configure root output file
	TFile fileChannelSet(filename, mode);
	TTree trN0Events("N0Events", "Normal events (fLogType=-1).");
	TTree trB0Events("B0Events", "Breakdown events (fLogType=0).");
	TTree trB1Events("B1Events", "1st event before the breakdown events (fLogType=1).");
//	TTree trB2Events("B2Events", "2nd event before the breakdown events (fLogType=2).");

	for (Long64_t i=0; i < nchannel; i++){
		trN0Events.Branch(fChannelNames[i].c_str(), &channelsetN0[i], 16000, 99);
		trB0Events.Branch(fChannelNames[i].c_str(), &channelsetB0[i], 16000, 99);
		trB1Events.Branch(fChannelNames[i].c_str(), &channelsetB1[i], 16000, 99);
//		trB2Events.Branch(fChannelNames[i].c_str(), &channelsetB2[i], 16000, 99);
//		if (fVerbose)
			printf("Channel %lld: %s\n", i, fChannelNames[i].c_str());
	}



	for(std::string infile: fInFiles){
		XBOX::XboxTdmsFileConverter tdmsconverter(infile);
		tdmsconverter.loadEntryList();

		if (fVerbose)
			printf("Process file %s: %lld events ...\n",
					infile.c_str(), tdmsconverter.getEntryCount());


		Int_t bufLogType[] = {-1, -1, -1}; // stores the fLogType of the last 3 events in a ring buffer
		Int_t ievent = 0;
		while (tdmsconverter.nextEntry()){

			// read test-wise first channel to get fLogType and to check whether the channel is empty
			XboxDAQChannel ch;
			tdmsconverter.convertCurrentEntry(fChannelNames[0], ch);

			bufLogType[ievent % 3] = ch.getLogType();
			if(!ch.isEmpty()) {

				if (bufLogType[ievent % 3] == 2
						&& bufLogType[(ievent+2) % 3] == 1) { // convert data into B2 cache
					printf("+++ %s\n", ch.getTimeStamp().AsString());
				}

				if (bufLogType[ievent % 3] == 2) { // convert data into B2 cache

//					for (UInt_t i=0; i < nchannel; i++)
//						tdmsconverter.convertCurrentEntry(fChannelNames[i], channelsetB2[i]);
				}
				else if (bufLogType[ievent % 3] == 1 ) {
//						&& bufLogType[(ievent+2) % 3] == 2) { // convert data into B2 cache

					for (UInt_t i=0; i < nchannel; i++)
						tdmsconverter.convertCurrentEntry(fChannelNames[i], channelsetB1[i]);
				}
				else if (bufLogType[ievent % 3] == 0
						&& bufLogType[(ievent+2) % 3] == 1) {
//						&& bufLogType[(ievent+1) % 3] == 2) { // convert data into B0 cache

							for (UInt_t i=0; i < nchannel; i++)
								tdmsconverter.convertCurrentEntry(fChannelNames[i], channelsetB0[i]);
							trB0Events.Fill();
							trB1Events.Fill();
//							trB2Events.Fill();
				}
				else if (bufLogType[ievent % 3] == -1) { // convert data into N0 cache

					for (UInt_t i=0; i < nchannel; i++)
						tdmsconverter.convertCurrentEntry(fChannelNames[i], channelsetN0[i]);
					trN0Events.Fill();
				}
			}
			ievent++;
		}
	}
	fileChannelSet.Write();
	fileChannelSet.Close();

	if (fVerbose) {
		printf("Conversion finished. All data have been written to %s.\n", filename);
		printf("----------------------------------------------------\n");
	}

	return 0;
}

#ifdef HDF5_FOUND

Int_t XboxFileConverter::writeH5(const Char_t* filename){

	if (fInFiles.empty())
		return -1;

	Long64_t nchannel = fChannelNames.size();

	printf("XBox Version: %d\n", fXboxVersion);
	printf("Number of available channels: %lld\n", nchannel);

	for (UInt_t i=0; i < nchannel; i++)
		printf("Channel %u: %s\n", i, fChannelNames[i].c_str());

	XBOX::XboxH5File h5file(filename);
	std::string groupname;

	Int_t count = 0;
	for(std::string infile: fInFiles){
		XBOX::XboxTdmsFileConverter tdmsconverter(infile);

		tdmsconverter.loadEntryList();
		while (tdmsconverter.nextEntry()){
			groupname = tdmsconverter.getCurrentEntryGroup();
			h5file.addGroup(groupname);
			for(std::string channelname: fChannelNames){
				XboxDAQChannel channel;
				tdmsconverter.convertCurrentEntry(channelname, channel);
				if(!channel.isEmpty()) // skip empty data sets
					h5file.addDataSet(groupname, channel);
			}
			count++;
		}
	}

	return 0;  // successfully terminated
}

#endif


#ifndef XBOX_NO_NAMESPACE
}
#endif
