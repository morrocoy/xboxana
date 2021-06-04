/*
 * TXboxTdmsConverter.hxx
 *
 *  Created on: Sep 14, 2018
 *      Author: kpapke
 */

#ifndef __TXBOXTDMSCONVERTER_HXX_
#define __TXBOXTDMSCONVERTER_HXX_


#include "Rtypes.h"

#include <iostream>
#include <string>
#include <vector>
#include <map>

#define H5CPP

#include "Tdms.h"

#include "XboxDataType.hxx"
#include "XboxDAQChannel.hxx"
#include "XboxTdmsFile.hxx"

#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif

class XboxTdmsConverter {


private:
	std::vector<std::string> fInFiles;
	std::vector<Bool_t>   fMask; // write channels which are not blended by this mask

	std::vector<std::string> fChannelNames;
	Int_t                 fXboxVersion;

public:
	XboxTdmsConverter();
	XboxTdmsConverter(const std::string &filename);
	XboxTdmsConverter(const Char_t *fileName);
	XboxTdmsConverter(const std::vector<std::string> &fileset);

	~XboxTdmsConverter();

	void                  init();
	void                  reset();
	void                  clear();

	Bool_t                isValidXboxVersion() const;

	Int_t                 getXboxVersion() const { return fXboxVersion; }
	Long64_t              getChannelCount() const { return fChannelNames.size(); }
	std::vector<std::string> getChannelNames() const { return fChannelNames; }

	void                  addFile(const Char_t *filename);
	void                  addFile(const std::string &filename){ addFile(filename.c_str()); }

	Int_t                 write(const Char_t* filename, const Char_t* mode="RECREATE");
	Int_t                 write(const std::string &filename, const Char_t* mode="RECREATE") { return write(filename.c_str(), mode); }
#ifdef H5CPP
	Int_t                 writeH5(const Char_t* filename);
	Int_t                 writeH5(const std::string &filename){ return writeH5(filename.c_str()); }
#endif

};

#ifndef XBOX_NO_NAMESPACE
}
#endif

#endif /* __TXBOXTDMSCONVERTER_HXX_ */
