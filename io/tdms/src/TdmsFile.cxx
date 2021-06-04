#include "TdmsChannel.hxx"
#include "TdmsGroup.hxx"
#include "TdmsObject.hxx"
#include "TdmsFile.hxx"


namespace TDMS {



TdmsFile::TdmsFile(const Char_t *filename)
:	fFileName(filename)
{
	init();

	fFile = new TdmsIfstream(fFileName.c_str(), std::ios::binary);
	if(!fFile->is_open())
		printf("ERROR: File not found: %s\n", fFileName.c_str());
}

TdmsFile::TdmsFile(const std::string &filename)
:	fFileName(filename)
{
	init();

	fFile = new TdmsIfstream(fFileName.c_str(), std::ios::binary);
	if(!fFile->is_open())
		printf("ERROR: File not found: %s\n", fFileName.c_str());
}

TdmsFile::~TdmsFile()
{
	clear();
	fFile->close();
	delete fFile;
}

void TdmsFile::init()
{
	fPrevObject = NULL;
	fFileSize = 0;
	fFlagHasMetaData = false;
	fFlagHasObjectList = false;
	fFlagHasRawData = false;
	fFlagIsInterleaved = false;
	fFlagIsBigEndian = false;
	fFlagHasDAQmxData = false;
	fVersionNumber = 0;
	fNextSegmentOffset = 0;
	fDataOffset = 0;
	fObjectCount = 0;
}

void TdmsFile::clear()
{
	fProperties.clear();
	for (TdmsGroup* obj : fGroupSet)
			delete obj;
	fGroupSet.clear();

	for (TdmsObject* obj : fObjectSet)
		delete obj;
	fObjectSet.clear();
}

void TdmsFile::reset()
{
	clear();
	init();
}

void TdmsFile::setFile(const Char_t *filename)
{
	clear();
	delete fFile;
	init();

	fFileName = filename;
	fFile = new TdmsIfstream(fFileName.c_str(), std::ios::binary);

	if(!fFile->is_open())
		printf("ERROR: File not found: %s\n", fFileName.c_str());
}


void TdmsFile::read(Int_t nsegmax)
{
	if(!fFile->is_open())
		return;

	reset();
	fFile->seekg(0, std::ios::end);
	fFileSize = fFile->tellg();
	if (fVerbose)
		printf("File size is: %d bytes (0x%X).\n", (UInt_t)fFileSize, (UInt_t)fFileSize);

	fFile->seekg(0, std::ios::beg);

	int nseg = 0;
	ULong64_t nextSegmentOffset = 0;
	Bool_t atEnd = false;

	while (fFile->tellg() < (UInt_t)fFileSize){
		nextSegmentOffset = readSegment(&atEnd);
		nseg++;

		if (fVerbose){
			printf("\nPOS after segment %d: 0x%X\n", nseg, (UInt_t)fFile->tellg());
			if (atEnd)
				 printf("Should skip to the end of file...File format error?!\n");
		}

		if (nextSegmentOffset >= fFileSize){
			if (fVerbose) printf("\tEnd of file is reached after segment %d!\n", nseg);
			break;
		}

		if(nseg == nsegmax){
			fFile->seekg(std::ios::end, std::ios::beg); // jump to the end of the file if number of segments is predefined
			atEnd = true;
			break;
		}
	}

	// if not at the end of the file interpretate the remaining data as binary
	if (!atEnd && ((ULong64_t)fFile->tellg() < fFileSize)){
		if (fVerbose)
			printf("\nFile contains raw data at the end!\n");

		readRawData((ULong64_t)(nextSegmentOffset - fFile->tellg()));
	}

	if (fVerbose)
		printf("\nNumber of segments: %d\n", nseg);

}

ULong64_t TdmsFile::readSegment(Bool_t *atEnd)
{
	readLeadIn();
	ULong64_t posAfterLeadIn = (ULong64_t)fFile->tellg();

	if (fNextSegmentOffset == -1)
		fNextSegmentOffset = fFileSize;

	*atEnd = (fNextSegmentOffset >= (Long64_t)fFileSize);
	Long64_t nextOffset = (*atEnd) ? fFileSize : fNextSegmentOffset + (Long64_t)fFile->tellg();
	if (fVerbose)
		printf("NEXT OFFSET: %d (0x%X)\n", (UInt_t)nextOffset, (UInt_t)nextOffset);


	if (fFlagHasMetaData){
		readMetaData();
		if (fFlagHasRawData){
			fFile->seekg(posAfterLeadIn + fDataOffset, std::ios_base::beg);
			if (fVerbose)
				printf("\tRaw data starts at POS: 0x%X\n", (UInt_t)fFile->tellg());

			ULong64_t total_chunk_size = fNextSegmentOffset - fDataOffset;
			fPrevObject = readRawMetaData(total_chunk_size, fPrevObject);

			if (fVerbose)
				printf("\tPOS after metadata: 0x%X\n", (UInt_t)fFile->tellg());
		}
	} else if (fFlagHasRawData){
		ULong64_t total_chunk_size = fNextSegmentOffset - fDataOffset;
		if (fVerbose)
			printf("\tSegment without metadata!\n");

		fFile->seekg(posAfterLeadIn + fDataOffset, std::ios_base::beg);
		if (fVerbose)
			printf("\tRaw data starts at POS: 0x%X\n", (UInt_t)fFile->tellg());

		readRawData(total_chunk_size);
	} else if (fVerbose)
		printf("\tSegment without metadata or raw data!\n");

	return nextOffset;
}

void TdmsFile::readRawData(ULong64_t total_chunk_size)
{
	if (fVerbose)
		printf("\tShould read %d rawdata bytes\n", (UInt_t)total_chunk_size);

	UInt_t groupCount = fGroupSet.size();
	for (UInt_t i = 0; i < groupCount; i++){
		TdmsGroup *group = getGroup(i);
		if (!group)
			continue;

		UInt_t channels = group->getGroupSize();
		ULong64_t chunkSize = 0;
		for (UInt_t j = 0; j < channels; j++){
			TdmsChannel *channel = group->getChannel(j);
			if (channel){
				ULong64_t channelSize = channel->getChannelSize();
				if (fVerbose)
					printf("\tChannel %s size is: %d\n", channel->getName().c_str(), (UInt_t)channelSize);

				chunkSize += channelSize;
			}
		}

		UInt_t chunks = total_chunk_size/chunkSize;
		if (fVerbose)
			printf("Total: %d chunks of raw data.\n", chunks);

		for (UInt_t k = 0; k < chunks; k++){
			for (UInt_t j = 0; j < channels; j++){
				TdmsChannel *channel = group->getChannel(j);
				if (channel)
					channel->readRawData(total_chunk_size, false);
			}
		}
	}
}


void TdmsFile::readLeadIn()
{
	Char_t buffer[4];
	fFile->read(buffer, 4);
	std::string tdmsString(buffer, 4);

	if ((tdmsString[0] == 0) || (tdmsString.compare("TDSm") != 0)){
		fNextSegmentOffset = fFileSize;
		fFlagHasMetaData = false;
		fFlagHasObjectList = false;
		fFlagHasRawData = false;
		fFlagHasDAQmxData = false;
		if (fVerbose)
			printf("\nInvalid header tag: '%s' read from file, should be 'TDSm'!\n", tdmsString.c_str());
		return;
	}

	UInt_t tocMask = 0;
//	fFile->operator >>(tocMask);
	fFile->read(reinterpret_cast<Char_t *>(&tocMask), sizeof(tocMask));

	fFlagHasMetaData   = ((tocMask &   2) != 0);
	fFlagHasObjectList = ((tocMask &   4) != 0);
	fFlagHasRawData    = ((tocMask &   8) != 0);
	fFlagIsInterleaved = ((tocMask &  32) != 0);
	fFlagIsBigEndian   = ((tocMask &  64) != 0);
	fFlagHasDAQmxData  = ((tocMask & 128) != 0);

	fFile->read(reinterpret_cast<Char_t *>(&fVersionNumber), sizeof(fVersionNumber));
	fFile->read(reinterpret_cast<Char_t *>(&fNextSegmentOffset), sizeof(fNextSegmentOffset));
	fFile->read(reinterpret_cast<Char_t *>(&fDataOffset), sizeof(fDataOffset));

	if (fVerbose && fFlagHasMetaData){
		std::cout << "\nRead lead-in data" << std::endl;
		std::cout << "  hasMetaData:         " << fFlagHasMetaData << std::endl;
		std::cout << "  hasObjectList:       " << fFlagHasObjectList << std::endl;
		std::cout << "  hasRawData:          " << fFlagHasRawData << std::endl;
		std::cout << "  isInterleaved:       " << fFlagIsInterleaved << std::endl;
		std::cout << "  isBigEndian:         " << fFlagIsBigEndian << std::endl;
		std::cout << "  hasDAQmxData:        " << fFlagHasDAQmxData << std::endl;
		std::cout << "  Version number:      " << fVersionNumber << std::endl;
		std::cout << "  Next segment offset: " << fNextSegmentOffset << std::endl;
		std::cout << "  Data offset:         " << fDataOffset << std::endl;
		printf ("\tPOS: 0x%X\n", (UInt_t)fFile->tellg());
	}
}


void TdmsFile::readMetaData()
{
	fFile->read(reinterpret_cast<Char_t *>(&fObjectCount), sizeof(fObjectCount));
	if (fVerbose){
		std::cout << "\nRead meta data" << std::endl;
		std::cout << "  Contains " << fObjectCount << " objects." << std::endl;
	}
	for (UInt_t i = 0; i < fObjectCount; i++)
		readObject();
	if (fVerbose)
		printf ("\tRaw data chunk size: %d\n", (UInt_t)getMetaDataChunkSize());
}

Long64_t TdmsFile::getMetaDataChunkSize()
{
	Long64_t chunk_size = 0;
	for (TdmsObjectSet_t::iterator object = fObjectSet.end()- (fObjectCount);  object != fObjectSet.end(); ++object){
		TdmsObject *obj = (*object);
		if (!obj)
			continue;

		if (obj->hasRawData())
			chunk_size += obj->getChannelSize();
	}
	return chunk_size;
}

void TdmsFile::readObject()
{
	TdmsObject *o = new TdmsObject(*fFile, fVerbose);
	fObjectSet.push_back(o);

	if (!o)
		return;

	if (o->isRoot()){
		setProperties(o->getProperties());
		return;
	}

	std::string path = o->getPath();
	TdmsGroup *group;
	TdmsChannel *channel;
	if (o->isGroup()){
		std::string groupName = path;
		group = getGroup(groupName);
		if (!group){
			addGroup(new TdmsGroup(groupName));
			if (fVerbose)
				printf("NEW GROUP: %s\n", groupName.c_str());
		} else if(o->getPropertyCount()){
			group = getGroup(groupName);
			group->setProperties(o->getProperties());
		}
	} else {
		Int_t islash = path.find("'/'", 1) + 1;
		std::string channelName = path.substr(islash);
		std::string groupName = path.substr(0, islash);

		group = getGroup(groupName);
		if (!group){
			group = new TdmsGroup(groupName);
			addGroup(group);
			if (fVerbose)
				printf("NEW GROUP: %s\n", path.c_str());
		}

		channel = group->getChannel(channelName);
		if (channel == 0){
			channel = new TdmsChannel(channelName, *fFile);
			channel->setProperties(o->getProperties());
			channel->setDimension(o->getDimension());

			TdmsDataType dtype = o->getDataType();
			channel->setDataType(dtype);
//			channel->setTypeSize((type == TChannel::tdsTypeString) ? (UInt_t)o->getBytesCount()
//					: TDMSObject::dataTypeSize(type));
			channel->setTypeSize((dtype == TdmsDataType::NATIVE_STRING) ? (UInt_t)o->getBytesCount()
								: dtype.getSize());

			group->addChannel(channel);
			if (fVerbose)
				printf("NEW CHANNEL: %s\n", channelName.c_str());
		}

		std::map<std::string, std::string> properties = o->getProperties();
		if (!properties.empty())
			channel->addProperties(properties);

		channel->setValuesCount(o->getValuesCount());
		o->setChannel(channel);
	}
}


TdmsObject* TdmsFile::readRawMetaData(ULong64_t total_chunk_size, TdmsObject *prevObject)
{
	TdmsObject *lastObj = 0;

	Long64_t chunk_size = getMetaDataChunkSize();
	if (!chunk_size)
		return 0;

	UInt_t chunks = total_chunk_size/chunk_size;
	if (fVerbose)
		printf ("\tNumber of chunks: %d\n", (UInt_t)chunks);

	for (UInt_t i = 0; i < chunks; i++){
		for (TdmsObjectSet_t::iterator object = fObjectSet.end()- (fObjectCount);  object != fObjectSet.end(); ++object){
			TdmsObject *obj = (*object);
			if (!obj)
				continue;

			if (obj->hasDAQmxData()){
				TdmsChannel *channel = obj->getChannel();
				if (!channel)
					channel = getChannel(obj);
				obj->readDAQmxData(channel);
			} else if (obj->hasRawData()){
				UInt_t index = obj->getRawDataIndex();
				if (index == 0)
					obj->setRawDataInfo(prevObject);
				else
					lastObj = obj;

				TdmsChannel *channel = obj->getChannel();
				if (!channel)
					channel = getChannel(obj);

				obj->readRawData(total_chunk_size, channel);
			}

			if ((ULong64_t)fFile->tellg() >= fFileSize)
				break;
		}
	}
	return lastObj;
}

TdmsChannel* TdmsFile::getChannel(TdmsObject *obj)
{
	if (!obj || obj->isRoot() || obj->isGroup())
		return 0;

	std::string path = obj->getPath();
	Int_t islash = path.find("'/'", 1) + 1;
	std::string channelName = path.substr(islash);
	std::string groupName = path.substr(0, islash);

	TdmsGroup *group = getGroup(groupName);
	if (!group)
		return 0;

	return group->getChannel(channelName);
}


TdmsGroup* TdmsFile::getGroup(const std::string &name) const
{
	UInt_t groupCount = fGroupSet.size();
	for (UInt_t i = 0; i < groupCount; i++){
		TdmsGroup *group = getGroup(i);
		if(group->getName() == name){
			return group;
		}
	}

	//for (TdmsGroupSet::iterator iter = groups.begin(); iter != groups.end(); ++iter){
	//	if ((*iter)->getName() == name)
	//		return *iter;
	//}
	return 0;
}

TdmsGroup* TdmsFile::getGroup(UInt_t index) const
{
	if (index >= fGroupSet.size())
		return 0;

	return fGroupSet.at(index);
}

std::string TdmsFile::getPropertiesAsString() const
{
	std::string s;
	for (std::map<std::string, std::string>::const_iterator it = fProperties.begin(); it != fProperties.end(); ++it){
		s.append(it->first + ": ");
		s.append(it->second + "\n");
	}
	return s;
}

} // end of namespace TDMS

