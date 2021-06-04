#ifndef TTDMSGROUP_HXX_
#define TTDMSGROUP_HXX_

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "Rtypes.h"

namespace TDMS {

class TdmsChannel;

class TdmsGroup
{

private:
	typedef std::vector<TdmsChannel*> TTdmsChannelSet;

	const std::string     fName;
	std::map<std::string, std::string> fProperties;
	TTdmsChannelSet       fChannels;

public:

	TdmsGroup(const std::string& name);
	~TdmsGroup();

	std::string           getName() const {return fName;}
	std::string           getBaseName() const;
	UInt_t                getGroupSize() const {return fChannels.size();}
	UInt_t                getMaxValuesCount() const;
	TdmsChannel*          getChannel(const std::string &) const;
	TdmsChannel*          getChannel(UInt_t) const;
	const TTdmsChannelSet& getChannels() const{return fChannels;}
	std::string           getProperty(const std::string& name) const;
	std::map<std::string, std::string> getProperties(){return fProperties;}
	std::string           getPropertiesAsString() const;

	void                  setProperties(std::map<std::string, std::string> props){fProperties = props;}

	void                  addChannel(TdmsChannel*);
	void                  addProperties(std::map<std::string, std::string>);
};

} // end of namespace TDMS


#endif /* TTDMSGROUP_HXX_ */
