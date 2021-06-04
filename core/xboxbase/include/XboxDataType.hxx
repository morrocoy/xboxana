/*
 * TDMSPredType.h
 *
 *  Created on: Oct 2, 2018
 *      Author: kpapke
 */

#ifndef __XBOXDATATYPE_H_
#define __XBOXDATATYPE_H_

#include <iostream>     // std
#include "XboxAPI.h"
#include "Rtypes.h"
#include "TObject.h"

using namespace std;


#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif

/*! \class PredType
    \brief Class PredType holds the definition of all the TDMS predefined
    datatypes.

    These types can only be made copy of,
    not created. They are treated as constants.
*/

class XBOX_DLL XboxDataType : public TObject  
{
	
protected:
	UInt_t fId;	                                        ///<Datatype id

	// Sets the datatype id.
	void p_setId(const UInt_t new_id);

public:

	XboxDataType(const UInt_t id=0);
//	TDataType(const TDataType& type_class);

	virtual ~XboxDataType();

	// Assignment operator
	XboxDataType& operator=(const XboxDataType& rhs);

	// Determines whether two datatypes are the same.
	bool operator==(const XboxDataType& compared_type) const;

	// Gets the datatype id.
	UInt_t getId() const {return fId;}

	// Returns the size of a datatype.
	size_t getSize() const;

	// Returns alias of a datatype.
	std::string getAlias() const;

	// Checks whether this datatype is a variable-length string.
	bool isVariableStr() const;

	// Declaration of predefined types; their definition is in TDMSPredType.cpp
	
	static const XboxDataType NATIVE_VOID;              ///!
	static const XboxDataType NATIVE_BOOL;              ///!

	static const XboxDataType NATIVE_INT8;              ///!
	static const XboxDataType NATIVE_UINT8;              ///!
	static const XboxDataType NATIVE_INT16;              ///!
	static const XboxDataType NATIVE_UINT16;              ///!
	static const XboxDataType NATIVE_INT32;              ///!
	static const XboxDataType NATIVE_UINT32;              ///!
	static const XboxDataType NATIVE_INT64;              ///!
	static const XboxDataType NATIVE_UINT64;              ///!

	static const XboxDataType NATIVE_FLOAT;              ///!
	static const XboxDataType NATIVE_DOUBLE;              ///!
	static const XboxDataType NATIVE_LDOUBLE;              ///!

	static const XboxDataType NATIVE_STRING;              ///!
	static const XboxDataType NATIVE_TIMESTAMP;              ///!
	static const XboxDataType NATIVE_COMPLEXFLOAT;              ///!
	static const XboxDataType NATIVE_COMPLEXDOUBLE;              ///!

	
	ClassDef(XboxDataType,1);	// Xbox data type class used to defined format for storing
};


#ifndef XBOX_NO_NAMESPACE
}
#endif

#endif /* __XBOXDATATYPE_H_ */
