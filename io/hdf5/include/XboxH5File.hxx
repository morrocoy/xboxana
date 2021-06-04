/*
 * XBOXTFileH5.hxx
 *
 *  Created on: Oct 4, 2018
 *      Author: kpapke
 */

#ifndef __XBOXH5FILE_HXX_
#define __XBOXH5FILE_HXX_

#include <iostream>
#include <string>
#include <vector>
#include <H5Cpp.h> // yum install hdf5* libzip

#include "Rtypes.h"

#include "XboxDataType.hxx"
#include "XboxDAQChannel.hxx"


#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif


class XboxH5File
{
	
private:
	H5::H5File *fFile;
	H5::DataSet *fActiveDataSet;
	
	H5::DataType          convertToH5DataType(XboxDataType dtype);

	Bool_t                isOpen(void) { return (fFile != NULL); }

	Int_t                 addAttribute(H5::DataSet dataset, const std::string &attr_name, const void *buf, XboxDataType dtype, const hsize_t length=1);
	Int_t                 addStringAttribute(H5::DataSet dataset, const std::string &attr_name, const std::string &s);

	Int_t                 addAttribute(H5::DataSet dataset, const std::string &attr_name, const Char_t val) { return addAttribute(dataset, attr_name, &val, XboxDataType::NATIVE_INT8); }
	Int_t                 addAttribute(H5::DataSet dataset, const std::string &attr_name, const Short_t val) { return addAttribute(dataset, attr_name, &val, XboxDataType::NATIVE_INT16); }
	Int_t                 addAttribute(H5::DataSet dataset, const std::string &attr_name, const Int_t val) { return addAttribute(dataset, attr_name, &val, XboxDataType::NATIVE_INT32); }
	Int_t                 addAttribute(H5::DataSet dataset, const std::string &attr_name, const Long64_t val) { return addAttribute(dataset, attr_name, &val, XboxDataType::NATIVE_INT64); }
	Int_t                 addAttribute(H5::DataSet dataset, const std::string &attr_name, const UChar_t val) { return addAttribute(dataset, attr_name, &val, XboxDataType::NATIVE_UINT8); }
	Int_t                 addAttribute(H5::DataSet dataset, const std::string &attr_name, const UShort_t val) { return addAttribute(dataset, attr_name, &val, XboxDataType::NATIVE_UINT16); }
	Int_t                 addAttribute(H5::DataSet dataset, const std::string &attr_name, const UInt_t val) { return addAttribute(dataset, attr_name, &val, XboxDataType::NATIVE_UINT32); }
	Int_t                 addAttribute(H5::DataSet dataset, const std::string &attr_name, const ULong64_t val) { return addAttribute(dataset, attr_name, &val, XboxDataType::NATIVE_UINT64); }
	Int_t                 addAttribute(H5::DataSet dataset, const std::string &attr_name, const Float_t val) { return addAttribute(dataset, attr_name, &val, XboxDataType::NATIVE_FLOAT); }
	Int_t                 addAttribute(H5::DataSet dataset, const std::string &attr_name, const Double_t val) { return addAttribute(dataset, attr_name, &val, XboxDataType::NATIVE_DOUBLE); }
	Int_t                 addAttribute(H5::DataSet dataset, const std::string &attr_name, const LongDouble_t val) { return addAttribute(dataset, attr_name, &val, XboxDataType::NATIVE_LDOUBLE); }

public:
	XboxH5File(const std::string &filename);

	virtual ~XboxH5File();

	Int_t                 addGroup(const std::string &name);
	Int_t                 addDataSet(const string &groupename, XboxDAQChannel &channel);

};


#ifndef XBOX_NO_NAMESPACE
} /* namespace XBOX */
#endif

#endif /* __XBOXH5FILE_HXX_ */
