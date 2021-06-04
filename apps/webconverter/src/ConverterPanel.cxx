#include "ConverterPanel.hxx"

#include "TBufferJSON.h"
#include "TROOT.h"
#include "TString.h"

#include <stdio.h>
#include <stdlib.h>

#include <boost/filesystem.hpp>
#include <boost/date_time.hpp>

#include "XboxDAQChannel.hxx"
#include "XboxTdmsConverter.hxx"


ConverterPanel::ConverterPanel() 
{
}

ConverterPanel::~ConverterPanel() 
{
}

std::vector<std::string> ConverterPanel::getListofSubDirectories()
{
	std::vector<std::string> subdirs;
	for (boost::filesystem::directory_iterator itr(fSourceDirectory);
			itr!=boost::filesystem::directory_iterator(); ++itr)
	{
		if (boost::filesystem::is_directory(itr->status()))
		{
			subdirs.push_back(itr->path().filename().string());
			//std::cout << itr->path().filename() << std::endl;
		}
	}
	return subdirs;
}

void ConverterPanel::ProcessData(unsigned connid, const std::string &arg)
{

	if (arg == "CONN_READY") {
		fConnId = connid;
		printf("connection established %u\n", fConnId);

		ConverterPanelModel model;

		//ComboBox for Data Set
		std::vector<std::string> entries = getListofSubDirectories();
		for(unsigned int i=0; i<entries.size(); i++)
			model.fStructure.push_back(ComboBoxItem(i, entries[i]));
		model.fSelectStructureId = "1";
		model.fDateBegin = "2018/03/01"; // format: yyyy/MM/dd
		model.fDateEnd = "2018/03/31"; // format: yyyy/MM/dd
		model.fOutputFileName = "output.root";
		model.fFileFormatId = 0;
		model.fConvertEnabled = true;
		model.fLog += "Initialize panel\n";

		//Communication with the JSONModel in JS
		TString json = TBufferJSON::ToJSON(&model);
		fWindow->Send(fConnId, std::string("sigSetModel:") + json.Data());
		return;
	}

    if (arg == "CONN_CLOSED") {
    	printf("CONN_CLOSED");
       printf("connection closed\n");
       fConnId = 0;
       return;
    }

    if (arg == "GET_BINARY") {
    	printf("GET_BINARY");
       float arr[1000];
       for (int n=0;n<1000;++n) arr[n] = n*1.11111;

       // send binary data, deep copy will be performed
       fWindow->SendBinary(fConnId, arr, sizeof(arr));
       return;
    }

    if (arg.find("sigOnConvert:") == 0) {
    	//std::cout << "Did fitting" << std::endl;
    	printf("Conversion ...\n");

		std::string json = arg.substr(arg.find(":")+1);
		ConverterPanelModel *model = nullptr;
		TBufferJSON::FromJSON(model, json.c_str());

		UInt_t id = atoi(model->fSelectStructureId.c_str());
		std::string structure = model->fStructure[id].fName;
		std::string filename = model->fOutputFileName;
		Int_t fileformat = model->fFileFormatId;

		boost::posix_time::ptime dateBegin =
				boost::posix_time::time_from_string(model->fDateBegin + " 0:00:00.00");
		boost::posix_time::ptime dateEnd =
				boost::posix_time::time_from_string(model->fDateEnd + " 0:00:00.00");

		printf("fStructure: %s\n", structure.c_str());
		std::cout << "fDateBegin = " << dateBegin << std::endl;
		std::cout << "fDateEnd = " << dateEnd << std::endl;
		printf("fOutputFileName: %s\n", model->fOutputFileName.c_str());
		printf("FileFormatId: %u\n", fileformat);

		model->fConvertEnabled = false; // disable converter button
		json = TBufferJSON::ToJSON(model);
		fWindow->Send(fConnId, std::string("sigSetModel:") + json);


		boost::filesystem::path path(fSourceDirectory);
		std::string fLog = "";

		// iterated over the days
		boost::posix_time::time_iterator dateit(dateBegin, boost::posix_time::hours(24));
		while(dateit <= dateEnd) {
			char infile[100];
			snprintf(infile, 100, "EventData_%04d%02d%02d.tdms",
					Int_t(dateit->date().year()),
					dateit->date().month().as_number(),
					dateit->date().day().as_number());
			//printf("%s\n", (path / structure / infile).c_str());

			model->fLog += "Convert file: " + (path / structure / infile).string() + "\n";
			json = TBufferJSON::ToJSON(model);
			fWindow->Send(fConnId, std::string("sigSetModel:") + json);

			XBOX::XboxTdmsConverter converter;
			converter.addFile((path / structure / infile).c_str());

			if (*dateit == dateBegin)
				converter.write(model->fOutputFileName, "RECREATE");
			else
				converter.write(model->fOutputFileName, "UPDATE");

			++dateit;
		}


		model->fConvertEnabled = true; // disable converter button
		model->fLog += "Conversion finished\n";
		json = TBufferJSON::ToJSON(model);
		fWindow->Send(fConnId, std::string("sigSetModel:") + json);

		printf("Conversion done!");

		delete model;
		return;
    }


}

//Create the window
void ConverterPanel::Show(const std::string &where)
{
  fWindow = ROOT::Experimental::RWebWindowsManager::Instance()->CreateWindow();

  // this is very important, it defines name of openui5 widget, which
  // will run on the client side
  fWindow->SetPanelName("localapp.view.ConverterPanel");

  //fWindow->SetDefaultPage("file:fclWithRouting.html");

  // this is call-back, invoked when message received via websocket
  fWindow->SetDataCallBack([this](unsigned connid, const std::string &arg) { ProcessData(connid, arg); });

  fWindow->SetGeometry(450, 550); // configure predefined geometry

  fWindow->Show(where);

  // instead showing of window just generate URL, which can be copied into the browser
  std::string url = fWindow->GetUrl(true);
  printf("Example: %s\n", url.c_str());


}


