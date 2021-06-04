/*
 * XBOXTFileH5.cxx
 *
 *  Created on: Oct 4, 2018
 *      Author: kpapke
 */

#include "XboxH5File.hxx"

#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif

XboxH5File::XboxH5File(const std::string &filename)
{
	fFile = new H5::H5File(filename, H5F_ACC_TRUNC,H5::FileCreatPropList::DEFAULT,
		H5::FileAccPropList::DEFAULT);
}

XboxH5File::~XboxH5File() {
	if(fFile != NULL)
		delete fFile;
}

H5::DataType XboxH5File::convertToH5DataType(XBOX::XboxDataType dtype)
{
	if (dtype == XBOX::XboxDataType::NATIVE_BOOL){
		return H5::PredType::NATIVE_HBOOL;
	}
	else if (dtype == XBOX::XboxDataType::NATIVE_INT8){
		return H5::PredType::NATIVE_INT8;
	}
	else if (dtype == XBOX::XboxDataType::NATIVE_INT16){
		return H5::PredType::NATIVE_INT16;
	}
	else if (dtype == XBOX::XboxDataType::NATIVE_INT32){
		return H5::PredType::NATIVE_INT32;
	}
	else if (dtype == XBOX::XboxDataType::NATIVE_INT64){
		return H5::PredType::NATIVE_INT64;
	}
	else if (dtype == XBOX::XboxDataType::NATIVE_UINT8){
		return H5::PredType::NATIVE_UINT8;
	}
	else if (dtype == XBOX::XboxDataType::NATIVE_UINT16){
		return H5::PredType::NATIVE_UINT16;
	}
	else if (dtype == XBOX::XboxDataType::NATIVE_UINT32){
		return H5::PredType::NATIVE_UINT32;
	}
	else if (dtype == XBOX::XboxDataType::NATIVE_UINT64){
		return H5::PredType::NATIVE_UINT64;
	}
	else if (dtype == XBOX::XboxDataType::NATIVE_DOUBLE){
		return H5::PredType::NATIVE_DOUBLE;
	}
	else{
		return H5::PredType::NATIVE_UINT8;
	}
}

Int_t XboxH5File::addStringAttribute(H5::DataSet dataset, const std::string &attr_name, const std::string &s)
{
	enum { NCHARACTERS = 256 }; // limitation in characters
	
	// Create new dataspace for attribute
	H5::DataSpace attr_dataspace = H5::DataSpace(H5S_SCALAR);

	// Create new string datatype for attribute
	H5::StrType strdatatype(H5::PredType::C_S1, NCHARACTERS); // of length 256 characters

	// Create attribute and write to it
	H5::Attribute attribute = dataset.createAttribute(attr_name, strdatatype, attr_dataspace);
	
	Char_t cbuffer[NCHARACTERS];
	snprintf (cbuffer, NCHARACTERS, "%s", s.c_str());
	attribute.write(strdatatype, cbuffer);
	
	// H5std_string causes problems under Windows. The following lines lead to runtime errors
	// const H5std_string strwritebuf(s);	
	// const H5std_string strwritebuf(cbuffer);
	// attribute.write(strdatatype, strwritebuf);
	return 0;
}

Int_t XboxH5File::addAttribute(H5::DataSet dataset, const std::string &attr_name, const void *buf, XboxDataType dtype, const hsize_t length)
{
	hsize_t attr_dims[1] = { length };

	H5::DataType datatype = convertToH5DataType(dtype);

	// Create the data space for the attribute.
	H5::DataSpace attr_dataspace = H5::DataSpace(1, attr_dims);

	// Create a dataset attribute.
	H5::Attribute attribute = dataset.createAttribute(attr_name, datatype, attr_dataspace);

	// Write the attribute data.
	attribute.write(datatype, buf);

	return 0;
}


Int_t XboxH5File::addGroup(const std::string &name)
{
	if(isOpen()){
		fFile->createGroup(name.c_str());
		return 0;
	}
	else{
		printf("ERROR: Could not add Group to HDF5 file. File is not opened.");
		return -1;
	}
}


Int_t XboxH5File::addDataSet(const string &groupename, XboxDAQChannel &channel)
{
	const Int_t RANK = 1;

	if(channel.isEmpty()){
		printf("WARNING: HF DataSet is empty. Skip in HDF5 file.");
		return -1;
	}

	// Define the size of the array and create the data space for fixed size dataset.
	hsize_t dim[RANK];              // dataset dimensions
	dim[0] = channel.getSamples();

	H5::DataSpace *dataspace;
	dataspace = new H5::DataSpace(RANK, dim, NULL);

	// create dataset creation property list
	H5::DSetCreatPropList ds_creatplist;  // create dataset creation prop list
	ds_creatplist.setChunk(RANK, dim);  // then modify it for compression
	ds_creatplist.setDeflate(6);

	// Define datatype for the data
	H5::DataType datatype = convertToH5DataType(channel.getDataType());

	// Create dataset using defined dataspace and datatype
	std::string path = groupename + '/' + channel.getChannelName();

	H5::DataSet dataset(fFile->createDataSet(path, datatype, *dataspace));

	// Write the data to the dataset using default memory space, file space, and transfer properties.
	std::vector<Byte_t> data = channel.getRawData();
//	std::vector<Double_t> data2 = channel.getData();

//	fActiveDataSet->write(&data[0], datatype, H5::DataSpace::ALL, H5::DataSpace::ALL);
	dataset.write(&data[0], datatype, H5::DataSpace::ALL, H5::DataSpace::ALL);

	// add attributes
	addStringAttribute(dataset, "ChannelName", channel.getChannelName());
	addAttribute(dataset, "XboxVersion", channel.getXboxVersion());

	addStringAttribute(dataset, "TimeStamp", channel.getTimeStamp().AsString());
	addAttribute(dataset, "LogType", channel.getLogType());
	addAttribute(dataset, "PulseCount", channel.getPulseCount());
	addAttribute(dataset, "DeltaF", channel.getDeltaF());
	addAttribute(dataset, "Line", channel.getLine());

	addAttribute(dataset, "BreakdownFlag", channel.getBreakdownFlag());
	addAttribute(dataset, "BreakdownType", channel.getBreakdownType());
	addAttribute(dataset, "BreakdownThreshDir", channel.getBreakdownThreshDir());
	addAttribute(dataset, "BreakdownThreshDirVal", channel.getBreakdownThreshDirVal());
	addAttribute(dataset, "BreakdownRatioVal", channel.getBreakdownRatioVal());

	addStringAttribute(dataset, "StartTime", channel.getStartTime().AsString());
	addAttribute(dataset, "StartOffset", channel.getStartOffset());
	addAttribute(dataset, "Increment", channel.getIncrement());
	addAttribute(dataset, "Samples", channel.getSamples());

	addStringAttribute(dataset, "XLabel", channel.getXLabel());
	addStringAttribute(dataset, "YLabel", channel.getXUnit());
	addStringAttribute(dataset, "YUnit", channel.getYUnit());
	addStringAttribute(dataset, "YUnitDescription", channel.getYUnitDescription());

	addAttribute(dataset, "ScaleType", channel.getScaleType());
	addStringAttribute(dataset, "ScaleUnit", channel.getScaleUnit());

	std::vector<Double_t> coeffs = channel.getScaleCoeffs();

	if(coeffs.empty()){
		Double_t dummy = 0;
		addAttribute(dataset, "ScaleCoeffs", &dummy, XboxDataType::NATIVE_DOUBLE, 0);
	}
	else
		addAttribute(dataset, "ScaleCoeffs", &coeffs[0], XboxDataType::NATIVE_DOUBLE, coeffs.size());

//	std::cout << "Data: ";
//	for(Int_t i=0; i<10; i++)
//		std::cout << data2[i] << " ";
//	std::cout << std::endl;
//
//	std::cout << "Coeffs: ";
//	for(Double_t coeff: coeffs)
//		std::cout << coeff << " ";
//	std::cout << std::endl;

	delete dataspace;

	return 0;
}


#ifndef XBOX_NO_NAMESPACE
} /* namespace XBOX */
#endif

