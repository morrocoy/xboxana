#include <algorithm>
#include <math.h>
#include <time.h>

#include "TdmsChannel.hxx"
#include "TdmsObject.hxx"

using namespace std;


namespace TDMS {


TdmsObject::TdmsObject(TdmsIfstream& f, Bool_t verbose)
: fFile(f),
fVerbose(verbose),
fFlagHasRawData(false),
fNValue(0),
fNBytes(0),
fChannel(0)
{
	readPath();
	readRawDataInfo();
	readPropertyCount();

	if (verbose)
		printf("	Properties (%d):\n", fPropertyCount);

	for (UInt_t i = 0; i < fPropertyCount; ++i)
		readProperty(i);

	if (verbose)
		printf ("\t\tPOS: 0x%X\n", (UInt_t)fFile.tellg());
}

void TdmsObject::readProperty(UInt_t i)
{
	UInt_t size;
	fFile >> size;

	std::string name("", size);
	fFile >> name;

	UInt_t itype;
	fFile >> itype;

	TdmsDataType dtype(itype);

	if (fVerbose)
		printf("	%d %s: ", i + 1, name.c_str());

	std::string val = readValue(dtype);
	if (!val.empty())
		fProperties.insert(std::pair<std::string, std::string>(name, val));
}

std::string TdmsObject::timestamp(Long64_t secs, ULong64_t fractionSecs)
{
	time_t t = secs - 2082844800;//substract secs until 1970

	struct tm *pt = gmtime(&t);
	if (!pt)
		return " ";

	Char_t buffer[80];
	sprintf(buffer, "%d.%02d.%d %02d:%02d:%02d,%f",
			pt->tm_mday, pt->tm_mon + 1, 1900 + pt->tm_year, pt->tm_hour, pt->tm_min, pt->tm_sec, fractionSecs*pow(2, -64));
	return buffer;
}


std::string TdmsObject::readValue(TdmsDataType dtype)
{
	const Int_t size = 100;
	Char_t output [size];

	if(dtype == TdmsDataType::NATIVE_BOOL) {
		Bool_t val;
		fFile >> val;
		snprintf(output, size, "%d", val);
	}
	else if(dtype == TdmsDataType::NATIVE_INT8) {
		Char_t val;
		fFile >> val;
		snprintf(output, size, "%d", val);
	}
	else if(dtype == TdmsDataType::NATIVE_INT16) {
		Short_t val;
		fFile >> val;
		snprintf(output, size, "%d", val);
	}
	else if(dtype == TdmsDataType::NATIVE_INT32) {
		Int_t val;
		fFile >> val;
		snprintf(output, size, "%d", val);
	}
	else if(dtype == TdmsDataType::NATIVE_INT64) {
		Long64_t val;
		fFile >> val;
		snprintf(output, size, "%lld", val);
	}
	if(dtype == TdmsDataType::NATIVE_UINT8) {
		UChar_t val;
		fFile >> val;
		snprintf(output, size, "%u", val);
	}
	else if(dtype == TdmsDataType::NATIVE_UINT16) {
		UShort_t val;
		fFile >> val;
		snprintf(output, size, "%u", val);
	}
	else if(dtype == TdmsDataType::NATIVE_UINT32) {
		UInt_t val;
		fFile >> val;
		snprintf(output, size, "%u", val);
	}
	else if(dtype == TdmsDataType::NATIVE_UINT64) {
		ULong64_t val;
		fFile >> val;
		snprintf(output, size, "%llu", val);
	}
	else if(dtype == TdmsDataType::NATIVE_FLOAT) {
		Float_t val;
		fFile >> val;
		snprintf(output, size, "%g", val);
	}
	else if(dtype == TdmsDataType::NATIVE_FLOATWITHUNIT) {
		Float_t val;
		fFile >> val;
		snprintf(output, size, "%g", val);
	}
	else if(dtype == TdmsDataType::NATIVE_DOUBLE) {
		Double_t val;
		fFile >> val;
		snprintf(output, size, "%g", val);
	}
	else if(dtype == TdmsDataType::NATIVE_DOUBLEWITHUNIT) {
		Double_t val;
		fFile >> val;
		snprintf(output, size, "%g", val);
	}
	else if(dtype == TdmsDataType::NATIVE_LDOUBLE) {
		LongDouble_t val;
		fFile >> val;
		snprintf(output, size, "%Lf", val);
	}
	else if(dtype == TdmsDataType::NATIVE_LDOUBLEWITHUNIT) {
		LongDouble_t val;
		fFile >> val;
		snprintf(output, size, "%Lf", val);
	}
	else if(dtype == TdmsDataType::NATIVE_STRING) {
		UInt_t ncharacter;
		fFile >> ncharacter;
		std::string s("", ncharacter);
		fFile >> s;
		snprintf(output, size, "%s", s.c_str());
	}
	else if(dtype == TdmsDataType::NATIVE_TIMESTAMP) {
		ULong64_t fractionsSecond;
		fFile >> fractionsSecond;
		Long64_t secondsSince;
		fFile >> secondsSince;
		std::string ts = timestamp(secondsSince, fractionsSecond);
		snprintf(output, size, "%s", ts.c_str());
	}
	else if(dtype == TdmsDataType::NATIVE_COMPLEXFLOAT) {
		Float_t rval, ival;
		fFile >> rval;
		fFile >> ival;
		snprintf(output, size, "%g+i*%g", rval, ival);
	}
	else if(dtype == TdmsDataType::NATIVE_COMPLEXDOUBLE) {
		Double_t rval, ival;
		fFile >> rval;
		fFile >> ival;
		snprintf(output, size, "%g+i*%g", rval, ival);
	}

	if (fVerbose)
		printf("%s (type = %zu)\n", output, dtype.getId());

	return output;
}


void TdmsObject::readPath()
{
	UInt_t size;
	fFile >> size;
	fFile >> fPath.assign(size, 0);

	if (fVerbose){
		printf("OBJECT PATH: %s", fPath.c_str());
		if (isRoot())
			printf(" is root!\n");
		else if (isGroup())
			printf(" is a group!\n");
		else {
			printf("\n Channel name: %s\n", getChannelName().c_str());
		}
	}
}

void TdmsObject::setRawDataInfo(TdmsObject *obj)
{
	if (!obj)
		return;

	fDataType = obj->getDataType();
}

void TdmsObject::readRawDataInfo()
{
	fFile >> fRawDataIndex;
	if (fVerbose)
		printf("\tRaw data index: %d @ 0x%X\n", fRawDataIndex, (UInt_t)fFile.tellg());

	if (fRawDataIndex == 0){
		if (fVerbose)
			printf("\t\tObject in this segment exactly matches the raw data index "
					"the same object had in the previous segment!\n");
	}


	fFlagHasRawData = (fRawDataIndex != 0xFFFFFFFF);

	if (fFlagHasRawData && fRawDataIndex > 0){
		UInt_t type_id;
		fFile >> type_id;
		fFile >> fDimension;
		fFile >> fNValue;
		fDataType = TdmsDataType(type_id);

		if (fDataType == TdmsDataType::NATIVE_STRING)
			fFile >> fNBytes;

		if (fVerbose){
			if (fDataType == TdmsDataType::NATIVE_STRING)
				printf("\tHas raw data: type=%zu dimension=%d values=%d nbytes=%d\n",
						fDataType.getId(), fDimension, (UInt_t)fNValue, (UInt_t)fNBytes);
			else if (fDataType == TdmsDataType::NATIVE_DAQMXRAWDATA)
				printf("\tHas DAQmx raw data: type=%zu dimension=%d values=%d\n",
						fDataType.getId(), fDimension, (UInt_t)fNValue);
			else
				printf("\tHas raw data: type=%zu dimension=%d values=%d\n",
						fDataType.getId(), fDimension, (UInt_t)fNValue);
		}

		if (fDataType == TdmsDataType::NATIVE_DAQMXRAWDATA)
			readFormatChangingScalers();
	}
}

void TdmsObject::readPropertyCount()
{
	fFile >> fPropertyCount;
}

Long64_t TdmsObject::getChannelSize() const
{
	if (!fNValue)
		return 0;

	if (fDataType == TdmsDataType::NATIVE_STRING)
		return fNBytes;
	else if (fDataType == TdmsDataType::NATIVE_DAQMXRAWDATA) {
		if (fFormatScaler.empty())
			return 0;
//		return (Long64_t)dataTypeSize(fFormatScaler.front().DAQmxDataType)
		return TdmsDataType(fFormatScaler.front().DAQmxDataType).getSize()*fDimension*fNValue;
	}
	return fDataType.getSize()*fDimension*fNValue;
}

std::string TdmsObject::getChannelName() const
{
	Int_t islash = fPath.find("'/'", 1) + 1;
	std::string channelName = fPath.substr(islash);
	std::string groupName = fPath.substr(0, islash);
	return channelName;
}

Bool_t TdmsObject::hasRawData() const
{
	if (fRawDataIndex == 0)
		return (fNValue > 0);

	return fFlagHasRawData;
}

Bool_t TdmsObject::hasDAQmxData() const
{
	return (fDataType == TdmsDataType::NATIVE_DAQMXRAWDATA);
}

void TdmsObject::setChannel(TdmsChannel *channel)
{
	fChannel = channel;

	if (fChannel && (fRawDataIndex == 0)){
		fDataType = fChannel->getDataType();
		fDimension = fChannel->getDimension();
		fNValue = fChannel->getValuesCount();
		if (fDataType == TdmsDataType::NATIVE_STRING)
			fNBytes = fChannel->getTypeSize();
	}
}

Bool_t TdmsObject::isGroup() const
{
	if (isRoot())
		return false;
	return (fPath.find("'/'") == std::string::npos);
}

void TdmsObject::readRawData(ULong64_t total_chunk_size, TdmsChannel* channel)
{
	if (fNValue == 0){
		UInt_t typeSize;
		if (fDataType == TdmsDataType::NATIVE_STRING)
			typeSize = fNBytes;
		else
			typeSize = fDataType.getSize();

		if (typeSize != 0)
			fNValue = total_chunk_size/typeSize;

		if (fVerbose)
			printf("\tReading %d data value(s) of type %zu "
				"and type size %d bytes (total size %d bytes).\n",
				(UInt_t)fNValue, fDataType.getId(), (UInt_t)typeSize, (UInt_t)total_chunk_size);

	}

	if (channel){
		channel->setDataType(fDataType);
		channel->readRawData(total_chunk_size, false);
	}
}

void TdmsObject::readDAQmxData(TdmsChannel* channel)
{
	if (channel)
		channel->readDAQmxData(fFormatScaler, fRawDataWidth);
}

void TdmsObject::readFormatChangingScalers()
{
	UInt_t scalersCount;
	fFile >> scalersCount;
	if (fVerbose)
		printf("\tFormat changing scalers vector size: %d @ 0x%X\n",
				scalersCount, (UInt_t)fFile.tellg());

	for (UInt_t i = 0; i < scalersCount; i++){
		if (fVerbose & (scalersCount > 1))
			printf("\t\ti = %d\n", i);

		FormatChangingScaler formatScaler;
		fFile >> formatScaler.DAQmxDataType;
		if (fVerbose)
			printf("\t\tDAQmx data type: %d @ 0x%X\n",
					formatScaler.DAQmxDataType, (UInt_t)fFile.tellg());

		fFile >> formatScaler.rawBufferIndex;
		if (fVerbose)
			printf("\t\tRaw buffer index: %d @ 0x%X\n",
					formatScaler.rawBufferIndex, (UInt_t)fFile.tellg());

		fFile >> formatScaler.rawByteOffset;
		if (fVerbose)
			printf("\t\tRaw byte offset within the stride: %d @ 0x%X\n",
					formatScaler.rawByteOffset, (UInt_t)fFile.tellg());

		fFile >> formatScaler.sampleFormatBitmap;
		if (fVerbose)
			printf("\t\tSample format bitmap: %d @ 0x%X\n",
					formatScaler.sampleFormatBitmap, (UInt_t)fFile.tellg());

		fFile >> formatScaler.scaleID;
		if (fVerbose)
			printf("\t\tScale ID: %d @ 0x%X\n",
					formatScaler.scaleID, (UInt_t)fFile.tellg());

		fFormatScaler.push_back(formatScaler);
	}

	UInt_t vectorSize;
	fFile >> vectorSize;
	if (fVerbose)
		printf("\tRaw data width vector size: %d @ 0x%X\n", vectorSize, (UInt_t)fFile.tellg());

	for (UInt_t i = 0; i < vectorSize; i++){
		UInt_t val;
		fFile >> val;

		fRawDataWidth.push_back(val);
		if (fVerbose){
			if (vectorSize > 1)
				printf("\ti = %d", i);
			printf("\tData width: %d @ 0x%X\n", val, (UInt_t)fFile.tellg());
		}
	}
}

TdmsObject::~TdmsObject()
{
	fFormatScaler.clear();
	fRawDataWidth.clear();
}

} // end of namespace TDMS

