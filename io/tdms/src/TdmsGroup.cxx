#include <cstring>
#include <stdlib.h>

#include "TdmsChannel.hxx"
#include "TdmsGroup.hxx"

using namespace std;

namespace TDMS {


TdmsGroup::TdmsGroup(const std::string &name)
:   fName(name)
{
}

TdmsGroup::~TdmsGroup()
{
	for (TdmsChannel* obj : fChannels)
		delete obj;
	fChannels.clear();
	fProperties.clear();
	//	std::cout << "***FREE GROUP MEMORY***" << std::endl;
}

std::string TdmsGroup::getBaseName() const
{
	std::string str = fName;
	str.erase(str.begin(), str.begin()+2);
	str.erase(str.end()-1,str.end());
	return str;
}

std::string TdmsGroup::getProperty(const std::string& name) const
{
	map<std::string, std::string>::const_iterator it = fProperties.find(name);
	if (it != fProperties.end())
		return it->second;

	return "";
}

std::string TdmsGroup::getPropertiesAsString() const
{
	std::string s;
	for (map<std::string, std::string>::const_iterator it = fProperties.begin(); it != fProperties.end(); ++it){
		s.append(it->first + ": ");
		s.append(it->second + "\n");
	}
	return s;
}

void TdmsGroup::addProperties(std::map<std::string, std::string> props)
{
	for (map<std::string, std::string>::const_iterator it = props.begin(); it != props.end(); ++it)
		fProperties.insert(std::pair<std::string, std::string>(it->first, it->second));
}

void TdmsGroup::addChannel(TdmsChannel* channel)
{
	fChannels.push_back(channel);
}

TdmsChannel* TdmsGroup::getChannel(const std::string &name) const
{
	UInt_t nchannel = fChannels.size();
	for (UInt_t i = 0; i < nchannel; i++){
		TdmsChannel *ch = getChannel(i);
//		printf("compare %s with %s %d\n", (const char*)name,  (const char*) ch->getName(), (ch->getName() == name) );
		if(ch->getName() == name){
			return ch;	
		}
	}
	return NULL;
}

TdmsChannel* TdmsGroup::getChannel(UInt_t index) const
{
	if (index >= fChannels.size())
		return 0;

	return fChannels.at(index);
}

UInt_t TdmsGroup::getMaxValuesCount() const
{
	UInt_t rows = 0, nchannel = fChannels.size();
	for (UInt_t i = 0; i < nchannel; i++){
		TdmsChannel *ch = getChannel(i);
		if (!ch)
			continue;

		TdmsDataType dtype = ch->getDataType();
//		UInt_t valuesCount = ((dataType == TChannel::tdsTypeString) || (dataType == TChannel::tdsTypeTimeStamp)) ?
//										ch->getStringCount() : ch->getDataCount();
		UInt_t valuesCount = ((dtype == TdmsDataType::NATIVE_STRING) || (dtype == TdmsDataType::NATIVE_TIMESTAMP)) ?
										ch->getStringCount() : ch->getDataCount();

		if (valuesCount > rows)
			rows = valuesCount;
	}
	return rows;
}


} // end of namespace TDMS

