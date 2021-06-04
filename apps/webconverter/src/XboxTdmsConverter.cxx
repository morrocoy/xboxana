/*
 * TXboxTdmsConverter.cxx
 *
 *  Created on: Sep 14, 2018
 *      Author: kpapke
 */


#include <fstream>
#include <ctime>


#include "TFile.h"
#include "TTree.h"

#include "XboxTdmsConverter.hxx"

#ifdef H5CPP
#include "XboxH5File.hxx"
#endif

#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif



XboxTdmsConverter::XboxTdmsConverter()
{
	init();
}

XboxTdmsConverter::XboxTdmsConverter(const Char_t *filename)
{
	init();
	addFile(filename);
}

XboxTdmsConverter::XboxTdmsConverter(const std::string &filename)
{
	init();
	addFile(filename.c_str());
}

XboxTdmsConverter::XboxTdmsConverter(const std::vector<std::string> &filenames)
{
	init();
	for (std::string filename : filenames)
		addFile(filename);
}

XboxTdmsConverter::~XboxTdmsConverter()
{

}

void XboxTdmsConverter::init()
{
	fXboxVersion = 0;
}

void XboxTdmsConverter::clear()
{
	fChannelNames.clear();
	fInFiles.clear();
}

void XboxTdmsConverter::reset()
{
	clear();
	init();
}

void XboxTdmsConverter::addFile(const Char_t *filename)
{
	XboxTdmsFile tdmsfile(filename);
	Int_t version = tdmsfile.getXboxVersion();
	if(version != 0){
		if(fInFiles.empty()){
			fInFiles.push_back(filename);
			fXboxVersion = version;
			fChannelNames = tdmsfile.getChannelNames();
		}
		else if(version == fXboxVersion)
			fInFiles.push_back(filename);
		else
			printf("ERROR: Could not append file \"%s\". Xbox version "
					"differs.\n", filename);
	}
//	else
//		printf("ERROR: Could not append file \"%s\". Xbox version "
//				"is not valid.\n", filename);
}


Int_t XboxTdmsConverter::write(const Char_t* filename, const Char_t* mode){

	if (fInFiles.empty())
		return -1;

	Long64_t nchannel = fChannelNames.size();

	printf("XBox Version: %d\n", fXboxVersion);
	printf("Number of available channels: %lld\n", nchannel);

	// define set of xbox daq channels
	std::vector<XboxDAQChannel> channelset;
	channelset.resize(nchannel);

	// configure root output file
	TFile fileChannelSet(filename, mode);
	TTree treeChannelSet("ChannelSet","A Tree with Xbox DAQ Channels");

	for (Long64_t i=0; i < nchannel; i++){
		treeChannelSet.Branch(fChannelNames[i].c_str(), &channelset[i], 16000, 99);
		printf("Channel %lld: %s\n", i, fChannelNames[i].c_str());
	}

	Int_t count = 0;
	for(std::string infile: fInFiles){
		XBOX::XboxTdmsFile tdmsfile(infile);

		tdmsfile.loadEntryList();
		while (tdmsfile.nextEntry()){
			for (UInt_t i=0; i < nchannel; i++)
				tdmsfile.getCurrentEntry(fChannelNames[i], channelset[i]);

			if(!channelset[0].isEmpty()) // skip empty data sets
				treeChannelSet.Fill();
			count++;
		}
	}

	fileChannelSet.Write();
	fileChannelSet.Close();

	return 0;
}

#ifdef H5CPP

Int_t XboxTdmsConverter::writeH5(const Char_t* filename){

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
		XBOX::XboxTdmsFile tdmsfile(infile);

		tdmsfile.loadEntryList();
		while (tdmsfile.nextEntry()){
			groupname = tdmsfile.getCurrentEntryGroup();
			h5file.addGroup(groupname);
			for(std::string channelname: fChannelNames){
				XboxDAQChannel channel;
				tdmsfile.getCurrentEntry(channelname, channel);
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
