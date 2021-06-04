#ifndef _XBOXFILESYSTEM_H_
#define _XBOXFILESYSTEM_H_

#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <fstream>
#include <iostream>
#include <ctime>

#include "Rtypes.h"
#include "TCollection.h"
#include "TRegexp.h"
#include "TSystemFile.h"
#include "TSystemDirectory.h"

#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif


////////////////////////////////////////////////////////////////////////////////
/// File operation.
/// check whether file is accessible.
inline bool accessFile (const std::string& name) {
  struct stat buffer;
  return (stat (name.c_str(), &buffer) == 0);
}

////////////////////////////////////////////////////////////////////////////////
/// File operation.
/// Get the the directory of a path.
inline std::string getDirectory(const std::string &filePathGlob) {
     size_t pos = filePathGlob.find_last_of("\\/");
     return (std::string::npos == pos)
         ? ""
         : filePathGlob.substr(0, pos+1);
}

////////////////////////////////////////////////////////////////////////////////
/// File operation.
/// Extract the file name of a path. Regular expressions are allowed.
inline std::string getFileName(const std::string &filePathGlob) {
     size_t pos = filePathGlob.find_last_of("\\/");
     return (std::string::npos == pos)
         ? ""
         : filePathGlob.substr(pos+1);
}

////////////////////////////////////////////////////////////////////////////////
/// File operation.
/// Get the list of files in directory a given a regular expression.
inline std::vector<std::string> getListOfFiles(const std::string &filePathGlob) {

    std::string sDirectory = getDirectory(filePathGlob);
    std::string sFileNameGlob = getFileName(filePathGlob);

    TSystemDirectory dir(sDirectory.c_str(), sDirectory.c_str());
    TList *files = dir.GetListOfFiles();
    std::vector<std::string> vListOfFiles;

    TRegexp re(TString(sFileNameGlob.c_str()), true);
    if (files) {
        TSystemFile *file;
        TIter next(files);
        while ((file = (TSystemFile*) next())) {
            std::string sFileName = file->GetName();
            TString str = file->GetName();
            if (str.Index(re) != -1)
                vListOfFiles.push_back(sDirectory + sFileName);

        }
    }

    std::sort(vListOfFiles.begin(), vListOfFiles.end());
    return vListOfFiles;
}

#ifndef XBOX_NO_NAMESPACE
}
#endif

#endif /* _XBOXFILESYSTEM_H_ */
