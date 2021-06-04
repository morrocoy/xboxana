/*
 * TDMSPredType.h
 *
 *  Created on: Oct 2, 2018
 *      Author: kpapke
 */

#ifndef __TDMSPREDTYPE_H_
#define __TDMSPREDTYPE_H_

#include <iostream>

#include "Rtypes.h"

using namespace std;


namespace TDMS {


/*! \class PredType
    \brief Class PredType holds the definition of all the TDMS predefined
    datatypes.

    These types can only be made copy of,
    not created. They are treated as constants.
*/

class TdmsDataType
{

protected:
	size_t fId;	// TDMS datatype id

	// Sets the datatype id.
	void p_setId(const size_t new_id);

public:

	TdmsDataType(const size_t id=0);
	TdmsDataType(const TdmsDataType& type_class);
	~TdmsDataType();

	// Assignment operator
	TdmsDataType& operator=(const TdmsDataType& rhs);

	// Determines whether two datatypes are the same.
	bool operator==(const TdmsDataType& compared_type) const;

	// Gets the datatype id.
	size_t getId() const {return fId;}

	// Returns the size of a datatype.
	size_t getSize() const;

	// Checks whether this datatype is a variable-length string.
	bool isVariableStr() const;

	// Declaration of predefined types; their definition is in TDMSPredType.cpp
	static const TdmsDataType NATIVE_VOID;

	static const TdmsDataType NATIVE_INT8;
	static const TdmsDataType NATIVE_UINT8;
	static const TdmsDataType NATIVE_INT16;
	static const TdmsDataType NATIVE_UINT16;
	static const TdmsDataType NATIVE_INT32;
	static const TdmsDataType NATIVE_UINT32;
	static const TdmsDataType NATIVE_INT64;
	static const TdmsDataType NATIVE_UINT64;

	static const TdmsDataType NATIVE_FLOAT;
	static const TdmsDataType NATIVE_DOUBLE;
	static const TdmsDataType NATIVE_LDOUBLE;
	static const TdmsDataType NATIVE_FLOATWITHUNIT;
	static const TdmsDataType NATIVE_DOUBLEWITHUNIT;
	static const TdmsDataType NATIVE_LDOUBLEWITHUNIT;
	static const TdmsDataType NATIVE_STRING;
	static const TdmsDataType NATIVE_BOOL;
	static const TdmsDataType NATIVE_TIMESTAMP;
	static const TdmsDataType NATIVE_FIXEDPOINT;
	static const TdmsDataType NATIVE_COMPLEXFLOAT;
	static const TdmsDataType NATIVE_COMPLEXDOUBLE;
	static const TdmsDataType NATIVE_DAQMXRAWDATA;


};

} // end of namespace TDMS


#endif /* __TDMSPredType_H_ */
