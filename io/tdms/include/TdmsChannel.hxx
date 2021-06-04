#ifndef TTDMSCHANNEL_HXX_
#define TTDMSCHANNEL_HXX_



#include "Rtypes.h"
//#include "TString.h"

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "TdmsDataType.hxx"
#include "TdmsIfstream.hxx"
#include "TdmsObject.hxx"

namespace TDMS {

class TdmsChannel
{
public:
	typedef enum {
		tdsTypeVoid,
		tdsTypeI8,
		tdsTypeI16,
		tdsTypeI32,
		tdsTypeI64,
		tdsTypeU8,
		tdsTypeU16,
		tdsTypeU32,
		tdsTypeU64,
		tdsTypeSingleFloat,
		tdsTypeDoubleFloat,
		tdsTypeExtendedFloat,
		tdsTypeSingleFloatWithUnit = 0x19,
		tdsTypeDoubleFloatWithUnit,
		tdsTypeExtendedFloatWithUnit,
		tdsTypeString = 0x20,
		tdsTypeBoolean = 0x21,
		tdsTypeTimeStamp = 0x44,
		tdsTypeFixedPoint = 0x4F,
		tdsTypeComplexSingleFloat = 0x08000c,
		tdsTypeComplexDoubleFloat = 0x10000d,
		tdsTypeDAQmxRawData = 0xFFFFFFFF
	} tdsDataType;

	TdmsChannel(const std::string& name, TdmsIfstream &f);
	~TdmsChannel();

	void                  freeMemory();

	std::string           getName() const {return fName;}
	std::string           getBaseName() const;
	std::string           getUnit() const;
	std::string           getProperty(const std::string& name) const;
	ULong64_t             getChannelSize() const;
	UInt_t                getDataCount() const {return fDataVector.size();}
	UInt_t                getStringCount() const {return fStringVector.size();}
	UInt_t                getTypeSize() const {return fTypeSize;}
	TdmsDataType             getDataType() const {return fDataType;}
	size_t                getDataTypeId() const {return fDataType.getId();}
	UInt_t                getDimension() const {return fDimension;}
	ULong64_t             getValuesCount() const {return fNValues;}

	std::vector<Byte_t>   getRawDataVector() const {return fRawDataVector;}
	std::vector<Double_t> getDataVector() {return fDataVector;}
	std::vector<Double_t> getImaginaryDataVector() {return fImagDataVector;}
	std::vector<std::string>  getStringVector() {return fStringVector;}
	std::map<std::string, std::string> getProperties(){return fProperties;}

	void                  setTypeSize(UInt_t);
	void                  setDataType(TdmsDataType dtype);
	void                  setProperties(std::map<std::string, std::string> props){fProperties = props;}
	void                  setDimension(UInt_t d){fDimension = d;}
	void                  setValuesCount(UInt_t);

	void                  readRawData(ULong64_t, Bool_t);
//	void                  readValue(UInt_t, Bool_t = kFALSE);
//	void                  readValues(UInt_t, Bool_t = kFALSE);
	void                  readValues(TdmsDataType dtype);
	void                  readDAQmxData(std::vector<FormatChangingScaler>, std::vector<UInt_t>);
	void                  readDAQmxValue(TdmsDataType dtype, Double_t slope, Double_t intercept, Bool_t verbose = kFALSE);

	void                  appendValue(Double_t val){fDataVector.push_back(val);}
	void                  appendImaginaryValue(Double_t val){fImagDataVector.push_back(val);}
	void                  appendString(std::string s){fStringVector.push_back(s);}
	void                  addProperties(std::map<std::string, std::string>);

	std::string           getPropertiesAsString() const;

private:
	void                  readStrings();

	const std::string     fName;
	TdmsIfstream&         fFile;
	TdmsDataType              fDataType;
	UInt_t                fTypeSize;
	UInt_t                fDimension;
	ULong64_t             fNValues;

	std::map<std::string, std::string> fProperties;
	std::vector<Byte_t>   fRawDataVector;
	std::vector<Double_t> fDataVector;
	std::vector<Double_t> fImagDataVector;
	std::vector<std::string> fStringVector;
};

} // end of namespace TDMS


#endif /* TDMSCHANNEL_HXX_ */

