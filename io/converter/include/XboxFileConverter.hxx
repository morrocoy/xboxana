/*
 * TXboxFileConverter.hxx
 *
 *  Created on: Sep 14, 2018
 *      Author: kpapke
 */

#ifndef __XBOXFILECONVERTER_HXX_
#define __XBOXFILECONVERTER_HXX_


#include "Rtypes.h"

#include <iostream>
#include <string>
#include <vector>
#include <map>

#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif

class XboxFileConverter {


private:
	std::vector<std::string> fInFiles;
	std::vector<Bool_t>   fMask; // write channels which are not blended by this mask

	std::vector<std::string> fChannelNames;
	Int_t                 fXboxVersion;
	Bool_t                fVerbose;

public:
	XboxFileConverter();
	XboxFileConverter(const std::string &filename);
	XboxFileConverter(const Char_t *fileName);
	XboxFileConverter(const std::vector<std::string> &fileset);

	~XboxFileConverter();

	void                  init();
	void                  reset();
	void                  clear();

	Bool_t                isValidXboxVersion() const;

	size_t                getFileCount() const { return fInFiles.size(); }
	Int_t                 getXboxVersion() const { return fXboxVersion; }
	Long64_t              getChannelCount() const { return fChannelNames.size(); }
	std::vector<std::string> getChannelNames() const { return fChannelNames; }

	void                  setVerbose(Bool_t bval) { fVerbose = bval; }
	void                  addFile(const Char_t *filename);
	void                  addFile(const std::string &filename){ addFile(filename.c_str()); }

	Int_t                 write(const Char_t* filename, const Char_t* mode="RECREATE");
	Int_t                 write(const std::string &filename, const Char_t* mode="RECREATE") { return write(filename.c_str(), mode); }
#ifdef HDF5_FOUND
	Int_t                 writeH5(const Char_t* filename);
	Int_t                 writeH5(const std::string &filename){ return writeH5(filename.c_str()); }
#endif

};

#ifndef XBOX_NO_NAMESPACE
}
#endif

#endif /* __TXBOXFILECONVERTER_HXX_ */
