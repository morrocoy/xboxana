#include "XboxTdmsFileConverter.hxx"

#include <fstream>
#include <ctime>

#include <stdlib.h>
#include <cstring>
#include <errno.h>
#include <limits.h>
#include <sstream>

#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif


XboxTdmsFileConverter::XboxTdmsFileConverter() {
	init();
}

XboxTdmsFileConverter::XboxTdmsFileConverter(const Char_t *filename) {
	init();
	setFile(filename);
	//printf("version: %d\n", fXboxVersion);
}
;

XboxTdmsFileConverter::XboxTdmsFileConverter(const std::string &filename) {
	init();
	setFile(filename.c_str());
	//printf("version: %d\n", fXboxVersion);
}
;

XboxTdmsFileConverter::~XboxTdmsFileConverter() {
	clearEntryList();
}

void XboxTdmsFileConverter::setFile(const Char_t *filename) {
	std::ifstream file(filename);
	if (file.good()) {
		Int_t version = readVersion(filename); // verify Xbox version of file
		if (version != 0) {
			fFileName = filename;
			fXboxVersion = version;
			fXboxChannelNames = readChannelList(filename);
		} else
			printf("ERROR: Could not append file \"%s\". Xbox version "
					"is not valid or differs.\n", filename);
	} else
		printf("WARNING: File not found: %s\n", filename);
}

void XboxTdmsFileConverter::XboxTdmsFileConverter::init() {
	fEntry = -1;
	fEntryCount = 0;
	fXboxVersion = 0;
}

void XboxTdmsFileConverter::clearEntryList() {
	if (fEntryCount)
		delete fTdmsFile;
	fEntryCount = 0;
	fEntry = -1;
}

void XboxTdmsFileConverter::loadEntryList() {
	clearEntryList();

	if (!isValidXboxVersion())
		return;

	fTdmsFile = new TDMS::TdmsFile(fFileName);
	fTdmsFile->read();

	fEntryCount = fTdmsFile->getGroupCount();
	//printf("Number of Events: %lld\n", fNEntries);
	restartEntryLoop();
}

XboxTdmsFileConverter::EEntryStatus XboxTdmsFileConverter::setEntry(
		Long64_t entry) {
	if (entry == -1 && fEntryCount > 0) { // special case for the next loop
		fEntry = entry;
		fTdmsGroup = fTdmsFile->getGroup(0);
		return kEntryValid;
	} else if (entry >= 0 && entry < fEntryCount) {
		fEntry = entry;
		fTdmsGroup = fTdmsFile->getGroup(fEntry);
		return kEntryValid;
	} else
		return kEntryNotFound;
}

XboxTdmsFileConverter::EEntryStatus XboxTdmsFileConverter::restartEntryLoop() {
	return setEntry(-1);
}

Bool_t XboxTdmsFileConverter::nextEntry() {
	if (fEntry < -1 || fEntry > fEntryCount - 2)
		return false;
	else {
		return (setEntry(fEntry + 1) == kEntryValid);
	}
}

Int_t XboxTdmsFileConverter::readVersion(const std::string &filename,
		const Int_t nseg) {
	Int_t version = 0; // default: no xbox fileversion associated

	TDMS::TdmsFile tdmsfile(filename);
	tdmsfile.read(nseg); // read first nseg tdms segments from file

	TDMS::TdmsGroup *tdmsgroup = tdmsfile.getGroup(0); // get first group

	Int_t nchannel = tdmsgroup->getGroupSize(); // get number of channels

	for (Int_t iversion = fgXboxChannelMaps.size() - 1; iversion > 0;
			--iversion) {
		Int_t validkeys = 0;
		for (Int_t ichannel = 0; ichannel < nchannel; ichannel++) {
			TDMS::TdmsChannel *ch = tdmsgroup->getChannel(ichannel);
			std::string key = ch->getName();
			for (const auto &entry : fgXboxChannelMaps[iversion]) {
				if (key == "/'" + entry.second + "'") {
					validkeys++;
					break;
				}
			}
		}
		//std::cout << "iversion: " << validkeys << std::endl;
		if (validkeys == nchannel) {
			version = iversion;
			break;
		}
	}
	return version;
}

Int_t XboxTdmsFileConverter::readVersion(void) {
	Int_t version = 0; // default: no xbox fileversion associated

	if (fFileName.empty())
		return 0;

	version = readVersion(fFileName); // read version of the input file
	return version;
}

std::vector<std::string> XboxTdmsFileConverter::readChannelList(const std::string &filename,
		const Int_t nseg) {
	Int_t version = 0; // default: no xbox fileversion associated

	TDMS::TdmsFile tdmsfile(filename);
	tdmsfile.read(nseg); // read first nseg tdms segments from file

	TDMS::TdmsGroup *tdmsgroup = tdmsfile.getGroup(0); // get first group

	Int_t nchannel = tdmsgroup->getGroupSize(); // get number of channels

	std::vector<std::string> keys;
	for (Int_t ichannel = 0; ichannel < nchannel; ichannel++) {
		TDMS::TdmsChannel *ch = tdmsgroup->getChannel(ichannel);
		std::string tdmskey = ch->getName();
		for (const auto &entry : fgXboxChannelMaps[fXboxVersion]) {
			if (tdmskey == "/'" + entry.second + "'") {
				keys.push_back(entry.first);
				break;
			}
		}
	}
	return keys;
}


Bool_t XboxTdmsFileConverter::isValidXboxVersion() const {
	if (fXboxVersion > 0 && fXboxVersion < (Int_t) fgXboxChannelMaps.size())
		return true;
	else
		return false;
}

ULong64_t XboxTdmsFileConverter::getChannelCount() const {
	if (isValidXboxVersion())
		return fXboxChannelNames.size();
	else
		return 0;
}

std::vector<std::string> XboxTdmsFileConverter::getChannelNames() const {
	std::vector<std::string> keys;
	if (isValidXboxVersion()) {
//		for (const auto &map : fgXboxChannelMaps[fXboxVersion])
//			keys.push_back(map.first);
		for (const auto &entry : fXboxChannelNames)
			keys.push_back(entry);
	}
	return keys;
}

Int_t XboxTdmsFileConverter::convertStrToTs(TTimeStamp &ts,
		const Char_t *stime) {
	// convert string to root time stamp

//    std::string stime = "07.06.2018 23:17:16,0.882594";
	UInt_t year;
	UInt_t month;
	UInt_t day;
	UInt_t hour;
	UInt_t min;
	UInt_t sec;
	UInt_t nsec;
	Double_t fsec;
	Char_t delimiter;

	Bool_t isUTC = true; // use default value from constructor
	Int_t secOffset = 0; // use default value from constructor

	if (!stime[0]) {

//		printf("WARNING: Empty string. Could not convert to time stamp.\n");
		year = 0;
		month = 0;
		day = 0;
		hour = 0;
		min = 0;
		sec = 0;
		nsec = 0;
		ts.Set(year, month, day, hour, min, sec, nsec, isUTC, secOffset);
		return 1;
	}

	std::istringstream is(stime);
	if (is >> day >> delimiter >> month >> delimiter >> year >> hour
			>> delimiter >> min >> delimiter >> sec >> delimiter >> fsec) {

		nsec = 1e9 * fsec;
		ts.Set(year, month, day, hour, min, sec, nsec, isUTC, secOffset);
		return 0;
	}
	else {

		printf("ERROR: Could not convert string to time stamp\n");
		year = 0;
		month = 0;
		day = 0;
		hour = 0;
		min = 0;
		sec = 0;
		nsec = 0;
		ts.Set(year, month, day, hour, min, sec, nsec, isUTC, secOffset);
		return 2;
	}
}

Int_t XboxTdmsFileConverter::convertStrToNum(Bool_t &val, const Char_t *sval) {
	// convert string to bool

	if (!sval[0]) {
//		printf("WARNING: Empty string. Could not convert to Bool.\n");
		val = false;
		return 1;
	} else {
		Char_t* end;
		Long_t lval = std::strtol(sval, &end, 10);
		if (*end) { // should be end of string character if succesfully converted
			printf("ERROR: Could not convert string to Long\n");
			val = false;
			return 2;
		} else if (errno == ERANGE) {
			printf("ERROR: Out of range while converting to Long\n");
			errno = 0;
			val = false;
			return 3;
		} else {
			if (lval)
				val = true;
			else
				val = false;
			return 0;
		}
	}
}
Int_t XboxTdmsFileConverter::convertStrToNum(Int_t &val, const Char_t *sval) {
	// convert string to integer
	// If the converted value falls out of range of corresponding return type,
	// range error occurs and LONG_MAX is returned. If no
	// conversion can be performed, ​0​ is returned.

	if (!sval[0]) {
//		printf("WARNING: Empty string. Could not convert to Integer.\n");
		val = 0;
		return 1;
	} else {
		Char_t* end;
		Long_t lval = std::strtol(sval, &end, 10);
		if (*end) { // should be end of string character if succesfully converted
			printf("ERROR: Could not convert string to Long\n");
			val = 0.;
			return 2;
		} else if (lval < INT_MIN || lval > INT_MAX) {
			printf("ERROR: Out of range while converting to Long\n");
			errno = 0;
			if (lval < INT_MIN)
				val = INT_MIN;
			else
				val = INT_MAX;
			return 3;
		} else {
			val = (Int_t) lval;
			return 0;
		}
	}
}

Int_t XboxTdmsFileConverter::convertStrToNum(Long_t &val, const Char_t *sval) {
	// convert string to long
	// If the converted value falls out of range of corresponding return type,
	// range error occurs and LONG_MAX is returned. If no
	// conversion can be performed, ​0​ is returned.

	if (!sval[0]) {
//		printf("WARNING: Empty string. Could not convert to Long.\n");
		val = 0;
		return 1;
	} else {
		Char_t* end;
		val = std::strtol(sval, &end, 10);
		if (*end) { // should be end of string character if succesfully converted
			printf("ERROR: Could not convert string to Long\n");
			return 2;
		} else if (errno == ERANGE) {
			printf("ERROR: Out of range while converting to Long\n");
			errno = 0;
			return 3;
		} else
			return 0;
	}
}

Int_t XboxTdmsFileConverter::convertStrToNum(Long64_t &val,
		const Char_t *sval) {
	// convert string to long long
	// If the converted value falls out of range of corresponding return type,
	// range error occurs and LLONG_MAX is returned. If no
	// conversion can be performed, ​0​ is returned.

	if (!sval[0]) {
//		printf("WARNING: Empty string. Could not convert to Long Long.\n");
		val = 0;
		return 1;
	} else {
		Char_t* end;
		val = std::strtoll(sval, &end, 10);
		if (*end) { // should be end of string character if succesfully converted
			printf("ERROR: Could not convert string to Long Long\n");
			return 2;
		} else if (errno == ERANGE) {
			printf("ERROR: Out of range while converting to Long Long\n");
			errno = 0;
			return 3;
		} else
			return 0;
	}
}

Int_t XboxTdmsFileConverter::convertStrToNum(ULong_t &val, const Char_t *sval) {
	// convert string to unsigned long
	// If the converted value falls out of range of corresponding return type,
	// range error occurs and ULONG_MAX is returned. If no
	// conversion can be performed, ​0​ is returned.

	if (!sval[0]) {
//		printf("WARNING: Empty string. Could not convert to Unsigned Long.\n");
		val = 0;
		return 1;
	} else {
		Char_t* end;
		val = std::strtoul(sval, &end, 10);
		if (*end) { // should be end of string character if succesfully converted
			printf("ERROR: Could not convert string to Unsigned Long\n");
			return 2;
		} else if (errno == ERANGE) {
			printf("ERROR: Out of range while converting to Unsigned Long\n");
			errno = 0;
			return 3;
		} else
			return 0;
	}
}

Int_t XboxTdmsFileConverter::convertStrToNum(ULong64_t &val,
		const Char_t *sval) {
	// convert string to unsigned long long
	// If the converted value falls out of range of corresponding return type,
	// range error occurs and ULLONG_MAX is returned. If no
	// conversion can be performed, ​0​ is returned.

	if (!sval[0]) {
//		printf("WARNING: Empty string. Could not convert to Unsigned Long Long.\n");
		val = 0;
		return 1;
	} else {
		Char_t* end;
		val = std::strtoull(sval, &end, 10);
		if (*end) { // should be end of string character if succesfully converted
			printf("ERROR: Could not convert string to Unsigned Long Long\n");
			return 2;
		} else if (errno == ERANGE) {
			printf(
					"ERROR: Out of range while converting to Unsigned Long Long\n");
			errno = 0;
			return 3;
		} else
			return 0;
	}
}

Int_t XboxTdmsFileConverter::convertStrToNum(Float_t &val, const Char_t *sval) {
	// convert string to double
	// If the converted value falls out of range of corresponding return type,
	// range error occurs and HUGE_VALF is returned. If no
	// conversion can be performed, ​0​ is returned.

	if (!sval[0]) {
//		printf("WARNING: Empty string. Could not convert to Float.\n");
		val = 0.;
		return 1;
	} else {
		Char_t* end;
		val = std::strtof(sval, &end);
		if (*end) { // should be end of string character if succesfully converted
			printf("ERROR: Could not convert string to Float\n");
			return 2;
		} else if (errno == ERANGE) {
			printf("ERROR: Out of range while converting to Float\n");
			errno = 0;
			return 3;
		} else
			return 0;
	}
}

Int_t XboxTdmsFileConverter::convertStrToNum(Double_t &val,
		const Char_t *sval) {
	// convert string to double
	// If the converted value falls out of range of corresponding return type,
	// range error occurs and HUGE_VAL is returned. If no
	// conversion can be performed, ​0​ is returned.

	if (!sval[0]) {
//		printf("WARNING: Empty string. Could not convert to Double.\n");
		val = 0.;
		return 1;
	} else {
		Char_t* end;
		val = std::strtod(sval, &end);
		if (*end) { // should be end of string character if succesfully converted
			printf("ERROR: Could not convert string to Double\n");
			return 2;
		} else if (errno == ERANGE) {
			printf("ERROR: Out of range while converting to Double\n");
			errno = 0;
			return 3;
		} else
			return 0;
	}
}

Int_t XboxTdmsFileConverter::convertStrToNum(LongDouble_t &val,
		const Char_t *sval) {
	// convert string to double
	// If the converted value falls out of range of corresponding return type,
	// range error occurs and d HUGE_VALL is returned. If no
	// conversion can be performed, ​0​ is returned.

	if (!sval[0]) {
//		printf("WARNING: Empty string. Could not convert to Long Double.\n");
		val = 0.;
		return 1;
	} else {
		Char_t* end;
		val = std::strtold(sval, &end);
		if (*end) { // should be end of string character if succesfully converted
			printf("ERROR: Could not convert string to Long Double.\n");
			return 2;
		} else if (errno == ERANGE) {
			printf("ERROR: Out of range while converting to Long Double.\n");
			errno = 0;
			return 3;
		} else
			return 0;
	}
}

Int_t XboxTdmsFileConverter::convertDataType(XBOX::XboxDataType &target_type,
		const TDMS::TdmsDataType &source_type) {
	if (source_type == TDMS::TdmsDataType::NATIVE_VOID) {
		target_type = XBOX::XboxDataType::NATIVE_VOID;
		return 0;
	} else if (source_type == TDMS::TdmsDataType::NATIVE_BOOL) {
		target_type = XBOX::XboxDataType::NATIVE_BOOL;
		return 0;
	} else if (source_type == TDMS::TdmsDataType::NATIVE_INT8) {
		target_type = XBOX::XboxDataType::NATIVE_INT8;
		return 0;
	} else if (source_type == TDMS::TdmsDataType::NATIVE_INT16) {
		target_type = XBOX::XboxDataType::NATIVE_INT16;
		return 0;
	} else if (source_type == TDMS::TdmsDataType::NATIVE_INT32) {
		target_type = XBOX::XboxDataType::NATIVE_INT32;
		return 0;
	} else if (source_type == TDMS::TdmsDataType::NATIVE_INT64) {
		target_type = XBOX::XboxDataType::NATIVE_INT64;
		return 0;
	} else if (source_type == TDMS::TdmsDataType::NATIVE_UINT8) {
		target_type = XBOX::XboxDataType::NATIVE_UINT8;
		return 0;
	} else if (source_type == TDMS::TdmsDataType::NATIVE_UINT16) {
		target_type = XBOX::XboxDataType::NATIVE_UINT16;
		return 0;
	} else if (source_type == TDMS::TdmsDataType::NATIVE_UINT32) {
		target_type = XBOX::XboxDataType::NATIVE_UINT32;
		return 0;
	} else if (source_type == TDMS::TdmsDataType::NATIVE_UINT64) {
		target_type = XBOX::XboxDataType::NATIVE_UINT64;
		return 0;
	} else if (source_type == TDMS::TdmsDataType::NATIVE_FLOAT) {
		target_type = XBOX::XboxDataType::NATIVE_FLOAT;
		return 0;
	} else if (source_type == TDMS::TdmsDataType::NATIVE_FLOATWITHUNIT) {
		target_type = XBOX::XboxDataType::NATIVE_FLOAT;
		return 0;
	} else if (source_type == TDMS::TdmsDataType::NATIVE_DOUBLE) {
		target_type = XBOX::XboxDataType::NATIVE_DOUBLE;
		return 0;
	} else if (source_type == TDMS::TdmsDataType::NATIVE_DOUBLEWITHUNIT) {
		target_type = XBOX::XboxDataType::NATIVE_DOUBLE;
		return 0;
	} else if (source_type == TDMS::TdmsDataType::NATIVE_LDOUBLE) {
		target_type = XBOX::XboxDataType::NATIVE_LDOUBLE;
		return 0;
	} else if (source_type == TDMS::TdmsDataType::NATIVE_LDOUBLEWITHUNIT) {
		target_type = XBOX::XboxDataType::NATIVE_LDOUBLE;
		return 0;
	} else if (source_type == TDMS::TdmsDataType::NATIVE_COMPLEXFLOAT) {
		target_type = XBOX::XboxDataType::NATIVE_COMPLEXFLOAT;
		return 0;
	} else if (source_type == TDMS::TdmsDataType::NATIVE_COMPLEXDOUBLE) {
		target_type = XBOX::XboxDataType::NATIVE_COMPLEXDOUBLE;
		return 0;
	} else {
		printf(
				"ERROR: Unknown TDMS data type. Could not convert Xbox data type.\n");
		target_type = XBOX::XboxDataType::NATIVE_VOID;
		return 1;
	}
}

XboxDataType XboxTdmsFileConverter::convertDataType(TDMS::TdmsDataType dtype) {
	if (dtype == TDMS::TdmsDataType::NATIVE_VOID) {
		return XBOX::XboxDataType::NATIVE_VOID;
	} else if (dtype == TDMS::TdmsDataType::NATIVE_BOOL) {
		return XBOX::XboxDataType::NATIVE_BOOL;
	} else if (dtype == TDMS::TdmsDataType::NATIVE_INT8) {
		return XBOX::XboxDataType::NATIVE_INT8;
	} else if (dtype == TDMS::TdmsDataType::NATIVE_INT16) {
		return XBOX::XboxDataType::NATIVE_INT16;
	} else if (dtype == TDMS::TdmsDataType::NATIVE_INT32) {
		return XBOX::XboxDataType::NATIVE_INT32;
	} else if (dtype == TDMS::TdmsDataType::NATIVE_INT64) {
		return XBOX::XboxDataType::NATIVE_INT64;
	} else if (dtype == TDMS::TdmsDataType::NATIVE_UINT8) {
		return XBOX::XboxDataType::NATIVE_UINT8;
	} else if (dtype == TDMS::TdmsDataType::NATIVE_UINT16) {
		return XBOX::XboxDataType::NATIVE_UINT16;
	} else if (dtype == TDMS::TdmsDataType::NATIVE_UINT32) {
		return XBOX::XboxDataType::NATIVE_UINT32;
	} else if (dtype == TDMS::TdmsDataType::NATIVE_UINT64) {
		return XBOX::XboxDataType::NATIVE_UINT64;
	} else if (dtype == TDMS::TdmsDataType::NATIVE_FLOAT) {
		return XBOX::XboxDataType::NATIVE_FLOAT;
	} else if (dtype == TDMS::TdmsDataType::NATIVE_FLOATWITHUNIT) {
		return XBOX::XboxDataType::NATIVE_FLOAT;
	} else if (dtype == TDMS::TdmsDataType::NATIVE_DOUBLE) {
		return XBOX::XboxDataType::NATIVE_DOUBLE;
	} else if (dtype == TDMS::TdmsDataType::NATIVE_DOUBLEWITHUNIT) {
		return XBOX::XboxDataType::NATIVE_DOUBLE;
	} else if (dtype == TDMS::TdmsDataType::NATIVE_LDOUBLE) {
		return XBOX::XboxDataType::NATIVE_LDOUBLE;
	} else if (dtype == TDMS::TdmsDataType::NATIVE_LDOUBLEWITHUNIT) {
		return XBOX::XboxDataType::NATIVE_LDOUBLE;
	} else if (dtype == TDMS::TdmsDataType::NATIVE_COMPLEXFLOAT) {
		return XBOX::XboxDataType::NATIVE_COMPLEXFLOAT;
	} else if (dtype == TDMS::TdmsDataType::NATIVE_COMPLEXDOUBLE) {
		return XBOX::XboxDataType::NATIVE_COMPLEXDOUBLE;
	} else {
		return XBOX::XboxDataType::NATIVE_VOID;
	}
}


Int_t XboxTdmsFileConverter::convertCurrentEntry(const std::string &name,
		XboxDAQChannel &channel, Bool_t mask) {
	channel.reset();

	if (!isValidXboxVersion() || fTdmsGroup == NULL)
		return -1;

	std::string tdmsname = fgXboxChannelMaps[fXboxVersion][name];
	if (tdmsname.empty()) {
		printf("Error: Channel not found in tdms file: %s\n", name.c_str());
		return -1;
	}

	channel.setChannelName(name);
	channel.setXboxVersion(fXboxVersion);

	std::string groupname = fTdmsGroup->getName();
//	Bool_t bbreakdown = (strstr(groupname.c_str(), "Breakdown") != NULL);

	TDMS::TdmsChannel *tdmschannel = fTdmsGroup->getChannel(
			"/'" + tdmsname + "'");

	return convertChannel(channel, *tdmschannel, mask);

//	if (fXboxVersion == kXbox1) {
//
//		// conversion from xbox1 TDMS file
//		return convertChannel_Xbox1(channel, *tdmschannel, mask);
//	} else if (fXboxVersion == kXbox2) {
//
//		// conversion from xbox2 TDMS file
//		return convertChannel_Xbox2(channel, *tdmschannel, mask);
//	} else if (fXboxVersion == kXbox3) {
//
//		// conversion from xbox3 TDMS file
//		return convertChannel_Xbox3(channel, *tdmschannel, mask);
//	} else {
//		printf("ERROR: Unknown Xbox version. Could not convert data from TDMS file.\n");
//		return 1;
//		//std::cout << "Channel:" << label  << std::endl;
//
//	}


}

#ifndef XBOX_NO_NAMESPACE
}
#endif

