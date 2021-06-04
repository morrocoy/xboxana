#include <iostream>
#include <ctime>

#include "Rtypes.h"
#include "TTimeStamp.h"
#include "TRegexp.h"
#include "TSystem.h"
#include "TSystemFile.h"
#include "TSystemDirectory.h"


//#include "ROOT/RDataFrame.hxx"
//#include "ROOT/RDFHelpers.hxx"

#include "XboxFileSystem.h"
#include "XboxDAQChannel.hxx"
#include "XboxFileConverter.hxx"

enum ESplitType {
	kNone,
	kDay,
	kWeek,
	kMonth,
	kYear
};


////////////////////////////////////////////////////////////////////////////////
/// Wrapper for xbox file converter.
/// Small wrapper function to configures and run the the conversion between
/// multiple tdms files to a single or multiple root file(s).
void convert(const std::string &sDstFilePath, std::vector<std::string> &sSrcFiles,
        TTimeStamp tsBeginDate, TTimeStamp tsEndDate,
		ESplitType split=ESplitType::kNone) {

    // check destination files and overwrite last one in destination folder
    std::string reDstFilePath = sDstFilePath;
    std::string sLastDstFilePath = "";
    reDstFilePath.insert(reDstFilePath.rfind('.'), "*");
    std::vector<std::string> vDstFilePath = XBOX::getListOfFiles(reDstFilePath);

    if (!vDstFilePath.empty()) {
        sLastDstFilePath = vDstFilePath.back();
//         printf("remove: %s\n", sLastDstFilePath.c_str());
    }

    // collect input files according to timestamp
    TTimeStamp tsCurrDate = tsBeginDate;

    if (split==ESplitType::kNone) {
        XBOX::XboxFileConverter converter;
        while(tsCurrDate <= tsEndDate) {
            int ndate = tsCurrDate.GetDate();
            std::string sdate = std::to_string(ndate);

            for (auto &sfile: sSrcFiles) {
                if (sfile.find(sdate) != std::string::npos) {
                    converter.addFile(sfile);
//                     printf("%s\n", sfile.c_str());
                }
            }
            tsCurrDate.Add(60*60*24); // add one day
        }
        converter.write(sDstFilePath);
    }
    else {

        while(tsCurrDate <= tsEndDate) {
            // evaluate intermediate date
            TTimeStamp tsIntDate;
            unsigned int iYear;
            unsigned int iMonth;
            unsigned int iWeek;
            unsigned int iDay;
            char aSuffix[10];
            std::vector<std::string> vInFiles;

            tsCurrDate.GetDate(true, 0, &iYear, &iMonth, &iDay);
            iWeek = tsCurrDate.GetWeek();

            if (split==kDay) {
                tsIntDate = tsCurrDate;
                snprintf(aSuffix, 10, "_%04d%02d%02d", iYear, iMonth, iDay);
            }
            else if (split==kWeek) { // go to last day of this week
                int idayofweek = tsCurrDate.GetDayOfWeek();
                tsIntDate = tsCurrDate + 60*60*24*(7-idayofweek);
                snprintf(aSuffix, 9, "_%04dW%02d", iWeek / 100, iWeek % 100);
            }
            else if (split==kMonth) { // go to last day of this month
                tsIntDate = TTimeStamp(iYear, iMonth+1, 1, 0, 0, 0);
                tsIntDate = tsIntDate - 60*60*24;
                snprintf(aSuffix, 9, "_%04dM%02d", iYear, iMonth);
            }
            else if (split==kYear) { // go to last day of this year
                tsIntDate = TTimeStamp(iYear+1, 1, 1, 0, 0, 0);
                tsIntDate = tsIntDate - 60*60*24;
                snprintf(aSuffix, 6, "_%04d", iYear);
            }

            if (tsIntDate > tsEndDate)
                tsIntDate = tsEndDate;

            // add all files within the specific period
            while(tsCurrDate <= tsIntDate) {
                int ndate = tsCurrDate.GetDate();
                std::string sdate = std::to_string(ndate);
                for (auto &sfile: sSrcFiles) {
                    if (sfile.find(sdate) != std::string::npos) {
                        vInFiles.push_back(sfile);
//                         printf("%s\n", sfile.c_str());
                    }
                }
                tsCurrDate.Add(60*60*24); // increment by one day
            }

            // modify output file path
            std::string sIntDstFileName = sDstFilePath;
            std::size_t npos = sIntDstFileName.rfind('.');
            sIntDstFileName.insert(npos, aSuffix);

//             printf("tsIntDate: %s | ", tsIntDate.AsString());
//             printf("sIntDstFileName %s\n", sDstFilePath.c_str());
//             printf("sIntDstFileName %s\n", sIntDstFileName.c_str());

            // check whether file exist already but overwrite last existing file in the destination folder
            if(!vInFiles.empty()
                    && sIntDstFileName.compare(sLastDstFilePath)
                    && !gSystem->AccessPathName(sIntDstFileName.c_str())) {
                printf("INFO: File \'%s\' exist. Skip conversion!\n", sIntDstFileName.c_str());
            }
            else {

                // run converter
                XBOX::XboxFileConverter converter;
                for (auto &sfile: vInFiles)
                    converter.addFile(sfile);
                converter.write(sIntDstFileName);
            }
        }
    }
}

int main(int argc, char* argv[]) {

	// multiple conversion to combine data for each month
	clock_t begin = clock();


//	std::string sSrcDirectory = "/dfs/Workspaces/x/Xbox2_T24PSI_2";
//	std::string sDstFilePath = "/Users/kpapke/projects/data/Xbox2_T24PSI_2/root/Xbox2_T24PSI_2.root";
//	TTimeStamp tsBeginDate(2018,3,26,00,00,00);
//	TTimeStamp tsEndDate(2018,3,27,00,00,00);

//	std::string sSrcDirectory = "/Users/kpapke/projects/data/Xbox2_Polarix/tdms/";
//	std::string sDstFilePath = "/Users/kpapke/projects/data/Xbox2_Polarix/root/Xbox2_Polarix.root";
	if (argc != 3) {
		printf("Usage: test_XboxFileConverter srcdir dstdir \n");
		return 1;
	}
	std::string sSrcDirectory = argv[1];
	std::string sDstFilePath = argv[2];

	std::vector<std::string> vEventDataFiles;
	vEventDataFiles = XBOX::getListOfFiles(sSrcDirectory+"/*EventDataA*.tdms");

//	for (auto sfile: vEventDataFiles) {
//		printf("%s\n", sfile.c_str());
//	}

	TTimeStamp tsBeginDate(2010,1,1,00,00,00);
	TTimeStamp tsEndDate(2029,12,31,00,00,00);

//	convert(sDstFilePath, vEventDataFiles, tsBeginDate, tsEndDate, kWeek);
	convert(sDstFilePath, vEventDataFiles, tsBeginDate, tsEndDate, kDay);

	clock_t end = clock();
	printf("Total elapsed time: %.3f\n", double(end - begin) / CLOCKS_PER_SEC);

	return 0;
}

