#ifndef __TDMSOBJECT_HXX__
#define __TDMSOBJECT_HXX__


#include "Rtypes.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

#include "TdmsDataType.hxx"
#include "TdmsIfstream.hxx"


namespace TDMS {

class TdmsChannel;

struct FormatChangingScaler
{
	UInt_t DAQmxDataType;
	UInt_t rawBufferIndex;
	UInt_t rawByteOffset;
	UInt_t sampleFormatBitmap;
	UInt_t scaleID;
};

class TdmsObject
{
private:
	TdmsIfstream&         fFile;
	Bool_t                fVerbose;

	std::string           fPath;
	UInt_t                fRawDataIndex;
	TdmsDataType              fDataType;

	Bool_t                fFlagHasRawData;
	UInt_t                fPropertyCount;
	UInt_t                fDimension;
	ULong64_t             fNValue;
	ULong64_t             fNBytes;
	TdmsChannel          *fChannel;
	std::map<std::string, std::string> fProperties;

	std::vector<FormatChangingScaler> fFormatScaler;
	std::vector<UInt_t>   fRawDataWidth;

	void                  readProperty(UInt_t);
	std::string           readValue(TdmsDataType dtype);

public:

	TdmsObject(TdmsIfstream&, Bool_t verbose);
	~TdmsObject();

	Bool_t                hasRawData() const;
	Bool_t                hasDAQmxData() const;
	Bool_t                isGroup() const;
	Bool_t                isRoot() const {return (fPath == "/");}

	const std::string&    getPath() const {return fPath;}
	TdmsDataType          getDataType() const {return fDataType;}
	UInt_t                getRawDataIndex() const {return fRawDataIndex;}
	ULong64_t             getValuesCount() const {return fNValue;}
	ULong64_t             getBytesCount() const {return fNBytes;}
	UInt_t                getDimension() const {return fDimension;}
	std::string           getChannelName() const;
	Long64_t              getChannelSize() const;
	TdmsChannel*          getChannel(){return fChannel;}
	Long64_t              getPropertyCount() const {return fPropertyCount;}
	std::map<std::string, std::string> getProperties(){return fProperties;}

	void                  setChannel(TdmsChannel*);
	void                  setRawDataInfo(TdmsObject *);

	void                  readPath();
	void                  readRawDataInfo();
	void                  readRawData(ULong64_t, TdmsChannel*);
	void                  readDAQmxData(TdmsChannel*);
	void                  readFormatChangingScalers();
	void                  readPropertyCount();

	static std::string    timestamp(Long64_t, ULong64_t);
//	static UInt_t         dataTypeSize(UInt_t);

};

} // end of namespace TDMS


#endif /* __TDMSOBJECT_HXX__ */
