/*
 * TXboxTdmsConverter.hxx
 *
 *  Created on: Sep 14, 2018
 *      Author: kpapke
 */

#ifndef __XBOXTDMSFILECONVERTER_HXX_
#define __XBOXTDMSFILECONVERTER_HXX_


#include "Rtypes.h"

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "Tdms.h"
#include "XboxDataType.hxx"
#include "XboxDAQChannel.hxx"


#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif

class XboxDataType;

class XboxTdmsFileConverter {

public:
	enum EEntryStatus {
	   kEntryValid = 0, // data read okay
	   kEntryNotLoaded, // no entry has been loaded yet
	   kEntryNoTree, // the tree does not exist
	   kEntryNotFound, // the tree entry number does not exist
	   kEntryChainSetupError, // problem in accessing a chain element, e.g. file without the tree
	   kEntryChainFileError, // problem in opening a chain's file
	   kEntryDictionaryError, // problem reading dictionary info from tree
	   kEntryBeyondEnd // last entry loop has reached its end
	};

private:

	typedef std::map<std::string, std::string> Dict_t;

	enum EXboxVersion {kXbox1 = 1, kXbox2 = 2, kXbox3 = 3};
	enum EMaxCoeff {kMaxCoeff = 1000};         ///<! Maximum number of supported coefficients

	std::string           fFileName;

	TDMS::TdmsFile       *fTdmsFile;
	TDMS::TdmsGroup      *fTdmsGroup;

	Long64_t              fEntryCount;                  ///<Number of entries (tdms groups)
	Long64_t              fEntry;                     ///<Current entry of the input file

	Int_t                 fXboxVersion;
	std::vector<std::string> fXboxChannelNames;

	static std::vector<Dict_t> fgXboxChannelMaps;

	Int_t                 readVersion(const std::string &filename, const Int_t nseg=10);
	Int_t                 readVersion(void);
	std::vector<std::string> readChannelList(const std::string &filename, const Int_t nseg=10);

	Int_t                 convertChannel(XboxDAQChannel &channel, const TDMS::TdmsChannel &tdmschannel, Bool_t bdata = true);

	// sub converter functions
	Int_t                 convertStrToTs(TTimeStamp &ts, const Char_t *stime); // convert string to ROOT time stamp
	Int_t                 convertStrToNum(Bool_t &val, const Char_t *sval); // convert string to bool
	Int_t                 convertStrToNum(Int_t &val, const Char_t *sval); // convert string to integer
	Int_t                 convertStrToNum(Long_t &val, const Char_t *sval); // convert string to long
	Int_t                 convertStrToNum(Long64_t &val, const Char_t *sval); // convert string to long long
	Int_t                 convertStrToNum(ULong_t &val, const Char_t *sval); // convert string to unsigned long
	Int_t                 convertStrToNum(ULong64_t &val, const Char_t *sval); // convert string to unsigned long long
	Int_t                 convertStrToNum(Float_t &val, const Char_t *sval); // convert string to double
	Int_t                 convertStrToNum(Double_t &val, const Char_t *sval); // convert string to double
	Int_t                 convertStrToNum(LongDouble_t &val, const Char_t *sval); // convert string to double
	Int_t                 convertDataType(XboxDataType &target_type, const TDMS::TdmsDataType &source_type); // convert the type of data array between TDMS and XBOX

	Int_t                 convertStrToTs(TTimeStamp &ts, const std::string &stime) { return convertStrToTs(ts, stime.c_str()); }
	Int_t                 convertStrToNum(Bool_t &val, const std::string &sval) { return convertStrToNum(val, sval.c_str()); }
	Int_t                 convertStrToNum(Int_t &val, const std::string &sval) { return convertStrToNum(val, sval.c_str()); }
	Int_t                 convertStrToNum(Long_t &val, const std::string &sval) { return convertStrToNum(val, sval.c_str()); }
	Int_t                 convertStrToNum(Long64_t &val, const std::string &sval) { return convertStrToNum(val, sval.c_str()); }
	Int_t                 convertStrToNum(ULong_t &val, const std::string &sval) { return convertStrToNum(val, sval.c_str()); }
	Int_t                 convertStrToNum(ULong64_t &val, const std::string &sval) { return convertStrToNum(val, sval.c_str()); }
	Int_t                 convertStrToNum(Float_t &val, const std::string &sval) { return convertStrToNum(val, sval.c_str()); }
	Int_t                 convertStrToNum(Double_t &val, const std::string &sval) { return convertStrToNum(val, sval.c_str()); }
	Int_t                 convertStrToNum(LongDouble_t &val, const std::string &sval) { return convertStrToNum(val, sval.c_str()); }

public:

	XboxTdmsFileConverter();
	XboxTdmsFileConverter(const Char_t *filename);
	XboxTdmsFileConverter(const std::string &filename);
	~XboxTdmsFileConverter();

	void                  init();

	void                  clearEntryList();
	void                  loadEntryList();
	EEntryStatus          restartEntryLoop();
	EEntryStatus          setEntry(Long64_t entry);
	Bool_t                nextEntry();

	Bool_t                isValidXboxVersion() const;

	Int_t                 getXboxVersion() const { return fXboxVersion; }
	ULong64_t             getChannelCount() const;
	ULong64_t             getEntryCount() const { return fEntryCount; };
	std::vector<std::string> getChannelNames() const;

	void                  setFile(const Char_t *filename);
	void                  setFile(const std::string &filename){ setFile(filename.c_str()); }

	Int_t                 convertCurrentEntry(const std::string &name, XboxDAQChannel& channel, Bool_t mask=true);
	XboxDataType          convertDataType(TDMS::TdmsDataType dtype);

	const std::string     getCurrentEntryGroup() const { return (fTdmsGroup != NULL) ? fTdmsGroup->getName() : ""; }
};

#ifndef XBOX_NO_NAMESPACE
}
#endif

#endif /* __XBOXTDMSFILECONVERTER_HXX_ */
