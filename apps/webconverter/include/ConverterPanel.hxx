#ifndef _CONVERTERPANEL_HXX_
#define _CONVERTERPANEL_HXX_

#include <iostream>
#include <vector>
#include <string>

#include <ROOT/RWebWindowsManager.hxx>
#include "TBufferJSON.h"
#include "TROOT.h"
#include "TTimeStamp.h"

// Structure for the ComboBox and Tree models


//struct ComboBoxItem {
//   std::string fId;
//   std::string fSet;
//   ComboBoxItem() = default;
//   ComboBoxItem(const std::string &id, const std::string &set) : fId(id), fSet(set) {}
//   ComboBoxItem(unsigned int id, const std::string &set) : fId(std::to_string(id)), fSet(set) {
//   }
//};

struct ComboBoxItem {
   std::string fId;
   std::string fName;
   ComboBoxItem() = default;
   ComboBoxItem(const std::string &id, const std::string &name) : fId(id), fName(name) {};
   ComboBoxItem(unsigned int id, const std::string &name) : fId(std::to_string(id)), fName(name) {};
};

struct ConverterPanelModel
{
	// settings
	std::vector<ComboBoxItem> fStructure;
	std::string fSelectStructureId;
	std::string fDateBegin; // format: yyyymmdd
	std::string fDateEnd; // format: yyyymmdd
	std::string fOutputFileName;
	Int_t fFileFormatId;
	Bool_t fConvertEnabled;
	std::string fLog;
	ConverterPanelModel() = default;
};


class ConverterPanel {
private:
	std::string           fChannelName;
	std::shared_ptr<ROOT::Experimental::RWebWindow> fWindow;
	unsigned              fConnId{0};

	std::string           fSourceDirectory{"/dfs/Workspaces/x"};
//	std::string           fStructure;
//	TTimeStamp            fStartDate; // format: d/m/yyyy
//	TTimeStamp            fEndDate; // format: d/m/yyyy
//	std::string           fOutputFileName;
//	Int_t                 fFormat;

public:
	ConverterPanel();
	virtual ~ConverterPanel();

	std::vector<std::string> getListofSubDirectories();
	void ProcessData(unsigned connid, const std::string &arg);
	void Show(const std::string &where = ""); //Create the window
};

#endif

