#include <cstring>
#include <stdlib.h>

#include "TdmsChannel.hxx"

using namespace std;

namespace TDMS {

TdmsChannel::TdmsChannel(const std::string& name, TdmsIfstream& f)
  : fName(name), fFile(f), fTypeSize(0), fDimension(0), fNValues(0)
{
}

TdmsChannel::~TdmsChannel()
{
	freeMemory();
}

std::string TdmsChannel::getBaseName() const
{
	std::string str = fName;
	str.erase(str.begin(), str.begin()+2);
	str.erase(str.end()-1,str.end());
	return str;
}

std::string TdmsChannel::getUnit() const
{
	return getProperty("unit_string");
}

std::string TdmsChannel::getProperty(const std::string& name) const
{
	map<std::string, std::string>::const_iterator it = fProperties.find(name);
	if (it != fProperties.end())
		return it->second;

	return "";
}

std::string TdmsChannel::getPropertiesAsString() const
{
	std::string s;
	for (map<std::string, std::string>::const_iterator it = fProperties.begin(); it != fProperties.end(); ++it){
		s.append(it->first + ": ");
		s.append(it->second + "\n");
	}
	return s;
}

void TdmsChannel::addProperties(std::map<std::string, std::string> props)
{
	for (map<std::string, std::string>::const_iterator it = props.begin(); it != props.end(); ++it)
		fProperties.insert(std::pair<std::string, std::string>(it->first, it->second));
}

void TdmsChannel::freeMemory()
{
	fRawDataVector.clear();
	fDataVector.clear();
	fImagDataVector.clear();
	fStringVector.clear();
	fProperties.clear();
}

void TdmsChannel::setTypeSize(UInt_t size)
{
	fTypeSize = size;
}

void TdmsChannel::setDataType(TdmsDataType dtype)
{
	fDataType = dtype;
}

void TdmsChannel::setValuesCount(UInt_t n)
{
	if (!n)
		return;
	fNValues = n;
}

ULong64_t TdmsChannel::getChannelSize() const
{
	return fTypeSize*fDimension*fNValues;
}

void TdmsChannel::readRawData(ULong64_t total_chunk_size, Bool_t verbose)
{
	if (fNValues == 0 && fTypeSize != 0)
		fNValues = total_chunk_size/fTypeSize;

	if (verbose)
		printf("\tChannel %s: reading %d raw data value(s) of type %zu.", fName.c_str(), (UInt_t)fNValues, fDataType.getId());

	if (fDataType == TdmsChannel::tdsTypeString)
		readStrings();
	else {
		readValues(fDataType);
	}

	if (verbose)
		printf(" Finished reading raw data (POS: 0x%X).\n", (UInt_t)fFile.tellg());
}

void TdmsChannel::readDAQmxData(std::vector<FormatChangingScaler> formatScalers, std::vector<UInt_t> dataWidths)
{
	if (formatScalers.empty() || dataWidths.empty())
		return;

	FormatChangingScaler formatScaler = formatScalers.front();
	UInt_t type_id = formatScaler.DAQmxDataType;
	TdmsDataType dtype;
//	UInt_t type = formatScaler.DAQmxDataType;
	UInt_t dataWidth = dataWidths.front();
//	UInt_t formatTypeSize = TDMSObject::dataTypeSize(type)*dataWidth;
	UInt_t formatTypeSize = TdmsDataType(type_id).getSize()*dataWidth;

	if (formatTypeSize == TdmsDataType::NATIVE_INT64.getSize())
		dtype = TdmsDataType::NATIVE_INT64;
	else if (formatTypeSize == TdmsDataType::NATIVE_INT32.getSize())
		dtype = TdmsDataType::NATIVE_INT32;
	else if (formatTypeSize == TdmsDataType::NATIVE_INT16.getSize())
		dtype = TdmsDataType::NATIVE_INT16;

	if (formatScaler.DAQmxDataType != dtype.getId()){
		std::string slopeString = getProperty("NI_Scale[1]_Linear_Slope");
		Double_t slope = slopeString.empty() ? 1.0 : atof(slopeString.c_str());

		std::string interceptString = getProperty("NI_Scale[1]_Linear_Y_Intercept");
		Double_t intercept = interceptString.empty() ? 0.0 : atof(interceptString.c_str());

		UInt_t values = fNValues/dataWidth;
		for (UInt_t i = 0; i < values; ++i)
			readDAQmxValue(dtype, slope, intercept);
	} else {
		readValues(dtype);
	}
}

void TdmsChannel::readDAQmxValue(TdmsDataType dtype, Double_t slope, Double_t intercept, Bool_t verbose)
{
	if (verbose)
		printf("\tRead DAQmx value for channel: %s\n", fName.c_str());

	if(dtype == TdmsDataType::NATIVE_INT32) {
		Int_t val;
		fFile >> val;
		if (verbose)
			printf("\t%d -> %g (type = %zu)\n", val, val*slope + intercept, dtype.getId());
		appendValue(val*slope + intercept);
	}
	else {
		if (verbose)
			printf("\t(unknown type = %zu)\n", dtype.getId());
	}
}

void TdmsChannel::readStrings()
{
	vector<UInt_t> offsets;
	offsets.push_back(0);
	for (UInt_t i = 0; i < fNValues; ++i){
		UInt_t offset;
		fFile >> offset;
		//printf("i: %d offset = %d POS @ 0x%X\n", i, offset, (UInt_t)file.tellg());
		offsets.push_back(offset);
	}

	UInt_t POS = offsets.at(0);
	for (UInt_t i = 1; i <= fNValues; ++i){
		UInt_t offset = offsets.at(i);
		UInt_t size = offset - POS;
		std::string s(size, 0);
		fFile >> s;
		fStringVector.push_back(s);
		//printf("i: %d offset: %d size = %d s: %s POS %d @ 0x%X\n", i, offset, size, s.c_str(), POS, (UInt_t)file.tellg());
		POS = offset;
	}
}

//void TChannel::readValues(UInt_t itype, Bool_t verbose)
void TdmsChannel::readValues(TdmsDataType dtype)
{

	if(dtype == TdmsDataType::NATIVE_BOOL) {
		Byte_t * rawbuffer;
//			Bool_t * buffer;
		UInt_t bytecount = fNValues * sizeof(Bool_t);
		rawbuffer = new Byte_t[bytecount];
		fFile.readArray(rawbuffer, bytecount);
//			buffer = reinterpret_cast<Bool_t *>(rawbuffer);
		fRawDataVector.insert(fRawDataVector.end(), rawbuffer, rawbuffer + bytecount);
//			fDataVector.insert(fDataVector.end(), buffer, buffer + fNValues);
		delete[] rawbuffer;
	}
	else if(dtype == TdmsDataType::NATIVE_INT8) {
		Byte_t * rawbuffer;
//			Char_t * buffer;
		UInt_t bytecount = fNValues * sizeof(Char_t);
		rawbuffer = new Byte_t[bytecount];
		fFile.readArray(rawbuffer, bytecount);
//			buffer = reinterpret_cast<Char_t *>(rawbuffer);
		fRawDataVector.insert(fRawDataVector.end(), rawbuffer, rawbuffer + bytecount);
//			fDataVector.insert(fDataVector.end(), buffer, buffer + fNValues);
		delete[] rawbuffer;
	}
	else if(dtype == TdmsDataType::NATIVE_INT16) {
//		Byte_t * rawbuffer;
////			Short_t * buffer;
//		UInt_t bytecount = fNValues * sizeof(Short_t);
//		rawbuffer = new Byte_t[bytecount];
//		fFile.readArray(rawbuffer, bytecount);
////			buffer = reinterpret_cast<Short_t *>(rawbuffer);
//		fRawDataVector.insert(fRawDataVector.end(), rawbuffer, rawbuffer + bytecount);
////			fDataVector.insert(fDataVector.end(), buffer, buffer + fNValues);
//		delete[] rawbuffer;

		UInt_t bytecount = fNValues * sizeof(Short_t);
		fRawDataVector.clear();
		fRawDataVector.resize(bytecount);
		fFile.readArray(&fRawDataVector[0], bytecount);
	}
	else if(dtype == TdmsDataType::NATIVE_INT32) {
		Byte_t * rawbuffer;
//			Int_t * buffer;
		UInt_t bytecount = fNValues * sizeof(Int_t);
		rawbuffer = new Byte_t[bytecount];
		fFile.readArray(rawbuffer, bytecount);
//			buffer = reinterpret_cast<Int_t *>(rawbuffer);
		fRawDataVector.insert(fRawDataVector.end(), rawbuffer, rawbuffer + bytecount);
//			fDataVector.insert(fDataVector.end(), buffer, buffer + fNValues);
		delete[] rawbuffer;
	}
	else if(dtype == TdmsDataType::NATIVE_INT64) {
		Byte_t * rawbuffer;
//			Long64_t * buffer;
		UInt_t bytecount = fNValues * sizeof(Long64_t);
		rawbuffer = new Byte_t[bytecount];
		fFile.readArray(rawbuffer, bytecount);
//			buffer = reinterpret_cast<Long64_t *>(rawbuffer);
		fRawDataVector.insert(fRawDataVector.end(), rawbuffer, rawbuffer + bytecount);
//			fDataVector.insert(fDataVector.end(), buffer, buffer + fNValues);
		delete[] rawbuffer;
	}
	else if(dtype == TdmsDataType::NATIVE_UINT8) {
		Byte_t * rawbuffer;
//			UChar_t * buffer;
		UInt_t bytecount = fNValues * sizeof(UChar_t);
		rawbuffer = new Byte_t[bytecount];
		fFile.readArray(rawbuffer, bytecount);
//			buffer = reinterpret_cast<UChar_t *>(rawbuffer);
		fRawDataVector.insert(fRawDataVector.end(), rawbuffer, rawbuffer + bytecount);
//			fDataVector.insert(fDataVector.end(), buffer, buffer + fNValues);
		delete[] rawbuffer;
	}
	else if(dtype == TdmsDataType::NATIVE_UINT16) {
		Byte_t * rawbuffer;
//			UShort_t * buffer;
		UInt_t bytecount = fNValues * sizeof(UShort_t);
		rawbuffer = new Byte_t[bytecount];
		fFile.readArray(rawbuffer, bytecount);
//			buffer = reinterpret_cast<UShort_t *>(rawbuffer);
		fRawDataVector.insert(fRawDataVector.end(), rawbuffer, rawbuffer + bytecount);
//			fDataVector.insert(fDataVector.end(), buffer, buffer + fNValues);
		delete[] rawbuffer;
	}
	else if(dtype == TdmsDataType::NATIVE_UINT32) {
		Byte_t * rawbuffer;
//			UInt_t * buffer;
		UInt_t bytecount = fNValues * sizeof(UInt_t);
		rawbuffer = new Byte_t[bytecount];
		fFile.readArray(rawbuffer, bytecount);
//			buffer = reinterpret_cast<UInt_t *>(rawbuffer);
		fRawDataVector.insert(fRawDataVector.end(), rawbuffer, rawbuffer + bytecount);
//			fDataVector.insert(fDataVector.end(), buffer, buffer + fNValues);
		delete[] rawbuffer;
	}
	else if(dtype == TdmsDataType::NATIVE_UINT64) {
		Byte_t * rawbuffer;
//			ULong64_t * buffer;
		UInt_t bytecount = fNValues * sizeof(ULong64_t);
		rawbuffer = new Byte_t[bytecount];
		fFile.readArray(rawbuffer, bytecount);
//			buffer = reinterpret_cast<ULong64_t *>(rawbuffer);
		fRawDataVector.insert(fRawDataVector.end(), rawbuffer, rawbuffer + bytecount);
//			fDataVector.insert(fDataVector.end(), buffer, buffer + fNValues);
		delete[] rawbuffer;
	}
	else if(dtype == TdmsDataType::NATIVE_FLOAT) {
		Byte_t * rawbuffer;
//			Float_t * buffer;
		UInt_t bytecount = fNValues * sizeof(Float_t);
		rawbuffer = new Byte_t[bytecount];
		fFile.readArray(rawbuffer, bytecount);
//			buffer = reinterpret_cast<Float_t *>(rawbuffer);
		fRawDataVector.insert(fRawDataVector.end(), rawbuffer, rawbuffer + bytecount);
//			fDataVector.insert(fDataVector.end(), buffer, buffer + fNValues);
		delete[] rawbuffer;
	}
	else if(dtype == TdmsDataType::NATIVE_FLOATWITHUNIT) {
		Byte_t * rawbuffer;
//			Float_t * buffer;
		UInt_t bytecount = fNValues * sizeof(Float_t);
		rawbuffer = new Byte_t[bytecount];
		fFile.readArray(rawbuffer, bytecount);
//			buffer = reinterpret_cast<Float_t *>(rawbuffer);
		fRawDataVector.insert(fRawDataVector.end(), rawbuffer, rawbuffer + bytecount);
//			fDataVector.insert(fDataVector.end(), buffer, buffer + fNValues);
		delete[] rawbuffer;
	}
	else if(dtype == TdmsDataType::NATIVE_DOUBLE) {
		Byte_t * rawbuffer;
//			Double_t * buffer;
		UInt_t bytecount = fNValues * sizeof(Double_t);
		rawbuffer = new Byte_t[bytecount];
		fFile.readArray(rawbuffer, bytecount);
//			buffer = reinterpret_cast<Double_t *>(rawbuffer);
		fRawDataVector.insert(fRawDataVector.end(), rawbuffer, rawbuffer + bytecount);
//			fDataVector.insert(fDataVector.end(), buffer, buffer + fNValues);
		delete[] rawbuffer;
	}
	else if(dtype == TdmsDataType::NATIVE_DOUBLEWITHUNIT) {
		Byte_t * rawbuffer;
//			Double_t * buffer;
		UInt_t bytecount = fNValues * sizeof(Double_t);
		rawbuffer = new Byte_t[bytecount];
		fFile.readArray(rawbuffer, bytecount);
//			buffer = reinterpret_cast<Double_t *>(rawbuffer);
		fRawDataVector.insert(fRawDataVector.end(), rawbuffer, rawbuffer + bytecount);
//			fDataVector.insert(fDataVector.end(), buffer, buffer + fNValues);
		delete[] rawbuffer;
	}
	else if(dtype == TdmsDataType::NATIVE_LDOUBLE) {
		Byte_t * rawbuffer;
//			LongDouble_t * buffer;
		UInt_t bytecount = fNValues * sizeof(LongDouble_t);
		rawbuffer = new Byte_t[bytecount];
		fFile.readArray(rawbuffer, bytecount);
//			buffer = reinterpret_cast<LongDouble_t *>(rawbuffer);
		fRawDataVector.insert(fRawDataVector.end(), rawbuffer, rawbuffer + bytecount);
//			fDataVector.insert(fDataVector.end(), buffer, buffer + fNValues);
		delete[] rawbuffer;
	}
	else if(dtype == TdmsDataType::NATIVE_LDOUBLEWITHUNIT) {
		Byte_t * rawbuffer;
//			LongDouble_t * buffer;
		UInt_t bytecount = fNValues * sizeof(LongDouble_t);
		rawbuffer = new Byte_t[bytecount];
		fFile.readArray(rawbuffer, bytecount);
//			buffer = reinterpret_cast<LongDouble_t *>(rawbuffer);
		fRawDataVector.insert(fRawDataVector.end(), rawbuffer, rawbuffer + bytecount);
//			fDataVector.insert(fDataVector.end(), buffer, buffer + fNValues);
		delete[] rawbuffer;
	}
	else if(dtype == TdmsDataType::NATIVE_STRING) {
		//string values are read in readRawData function directly
	}
	else if(dtype == TdmsDataType::NATIVE_TIMESTAMP) {
		std::vector<std::string> buffer(fNValues);
		for (UInt_t i = 0; i < fNValues; ++i){
			ULong64_t fractionsSecond;
			fFile >> fractionsSecond;
			Long64_t secondsSince;
			fFile >> secondsSince;
			buffer[i] = TdmsObject::timestamp(secondsSince, fractionsSecond);
		}
		fStringVector.insert(fStringVector.end(), buffer.begin(), buffer.end());
//			TODO: add Byte stream for fRawDataVector
	}
	else if(dtype == TdmsDataType::NATIVE_COMPLEXFLOAT) {
		Byte_t * rawbuffer;
//			Float_t * buffer;
//			Float_t * real;
//			Float_t * imag;
		UInt_t bytecount = fNValues * sizeof(Float_t);

		rawbuffer = new Byte_t[2 * bytecount];
//			real = new Float_t[fNValues];
//			imag = new Float_t[fNValues];

		fFile.readArray(rawbuffer, 2 * bytecount);
//			buffer = reinterpret_cast<Float_t *>(rawbuffer);
//			for (UInt_t i = 0; i < fNValues; ++i){
//				real[i] = buffer[2*i];
//				imag[i] = buffer[2*i+1];
//			}
		fRawDataVector.insert(fRawDataVector.end(), rawbuffer, rawbuffer + bytecount);
//			fDataVector.insert(fDataVector.end(), real, real + fNValues);
//			fImagDataVector.insert(fDataVector.end(), imag, imag + fNValues);
	}
	else if(dtype == TdmsDataType::NATIVE_COMPLEXDOUBLE) {
		Byte_t * rawbuffer;
//			Double_t * buffer;
//			Double_t * real;
//			Double_t * imag;
		UInt_t bytecount = fNValues * sizeof(Double_t);

		rawbuffer = new Byte_t[2 * bytecount];
//			real = new Double_t[fNValues];
//			imag = new Double_t[fNValues];

		fFile.readArray(rawbuffer, 2 * bytecount);
//			buffer = reinterpret_cast<Double_t *>(rawbuffer);
//			for (UInt_t i = 0; i < fNValues; ++i){
//				real[i] = buffer[2*i];
//				imag[i] = buffer[2*i+1];
//			}
		fRawDataVector.insert(fRawDataVector.end(), rawbuffer, rawbuffer + bytecount);
//			fDataVector.insert(fDataVector.end(), real, real + fNValues);
//			fImagDataVector.insert(fDataVector.end(), imag, imag + fNValues);
	}
	else
		printf(" (unknown type = %zu)\n", dtype.getId());


}

//void TChannel::readValue(UInt_t itype, Bool_t verbose)
//{
//	//	Int_t size = 100;
//
//	if (verbose)
//		printf("	Read value for channel: %s\n", fName.c_str());
//
//	switch (itype){
//		case 1: //INT8
//		{
//			Char_t val;
//			fFile >> val;
//			appendValue((Double_t)val);
//			if (verbose)
//				printf("%d (type = %d)\n", (Int_t)val, itype);
//		}
//		break;
//
//		case 2: //INT16
//		{
//			short val;
//			fFile >> val;
//			appendValue((Double_t)val);
//			if (verbose)
//				printf("%d (type = %d)\n", (Int_t)val, itype);
//		}
//		break;
//
//		case 3: //INT32
//		{
//			Int_t val;
//			fFile >> val;
//			appendValue((Double_t)val);
//			if (verbose)
//				printf("%d (type = %d)\n", val, itype);
//		}
//		break;
//
//		case 4: //INT64
//		{
//			Long64_t val;
//			fFile >> val;
//			appendValue((Double_t)val);
//			if (verbose)
//				printf("%d (type = %d)\n", (Int_t)val, itype);
//		}
//		break;
//
//		case 5: //UINT8
//		{
//			UChar_t val;
//			fFile >> val;
//			appendValue((Double_t)val);
//			if (verbose)
//				printf("%d (type = %d)\n", (Int_t)val, itype);
//		}
//		break;
//
//
//		case 6: //UINT16
//		{
//			UShort_t val;
//			fFile >> val;
//			appendValue((Double_t)val);
//			if (verbose)
//				printf("%d (type = %d)\n", (Int_t)val, itype);
//		}
//		break;
//
//		case 7: //UINT32
//		{
//			UInt_t val;
//			fFile >> val;
//			appendValue((Double_t)val);
//			if (verbose)
//				printf("%u (type = %d)\n", val, itype);
//		}
//		break;
//
//		case 8: //UINT64
//		{
//			ULong64_t val;
//			fFile >> val;
//			appendValue((Double_t)val);
//			if (verbose)
//				printf("%u (type = %d)\n", (UInt_t)val, itype);
//		}
//		break;
//
//		case 9: //FLOAT32
//		case TChannel::tdsTypeSingleFloatWithUnit:
//		{
//			float val;
//			fFile >> val;
//			appendValue((float)val);
//			if (verbose)
//				printf("%f (type = %d)\n", val, itype);
//		}
//		break;
//
//		case 10: //FLOAT64
//		case TChannel::tdsTypeDoubleFloatWithUnit:
//		{
//			Double_t val;
//			fFile >> val;
//			appendValue(val);
//			if (verbose)
//				printf("%f (type = %d)\n", val, itype);
//		}
//		break;
//
//		case 11: //FLOAT128
//		case TChannel::tdsTypeExtendedFloatWithUnit:
//		{
//			LongDouble_t val;
//			fFile >> val;
//			appendValue((Double_t)val);
//			if (verbose)
//				printf("%f (type = %d)\n", (Double_t)val, itype);
//		}
//		break;
//
//		case 32: //string values are read in readRawData function directly
//		break;
//
//		case 33: //Bool_t
//		{
//			Bool_t val;
//			fFile >> val;
//			appendValue((Double_t)val);
//			if (verbose)
//				printf("%d (type = %d)\n", val, itype);
//		}
//		break;
//
//		case tdsTypeTimeStamp: //time stamp
//		{
//			ULong64_t fractionsSecond;
//			fFile >> fractionsSecond;
//			Long64_t secondsSince;
//			fFile >> secondsSince;
//			std::string ts = TDMSObject::timestamp(secondsSince, fractionsSecond);
//			appendString(ts);
//			if (verbose)
//				printf("%s (type = %d)\n", ts.c_str(), itype);
//		}
//		break;
//
//		case TChannel::tdsTypeComplexSingleFloat:
//		{
//			float rval, ival;
//			fFile >> rval;
//			appendValue((float)rval);
//			fFile >> ival;
//			appendImaginaryValue((float)ival);
//			if (verbose)
//				printf("%g+i*%g (type = 0x%X)\n", rval, ival, itype);
//		}
//		break;
//
//		case TChannel::tdsTypeComplexDoubleFloat:
//		{
//			Double_t rval, ival;
//			fFile >> rval;
//			appendValue(rval);
//			fFile >> ival;
//			appendImaginaryValue(ival);
//			if (verbose)
//				printf("%g+i*%g (type = 0x%X)\n", rval, ival, itype);
//		}
//		break;
//
//		default:
//		{
//			if (verbose)
//				printf(" (unknown type = %d)\n", itype);
//		}
//		break;
//	}
//}


} // end of namespace TDMS

