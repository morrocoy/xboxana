#include "XboxDataType.hxx"

//#include <string>

#ifndef XBOX_NO_NAMESPACE
ClassImp(XBOX::XboxDataType);
namespace XBOX {
#else
ClassImp(XboxDataType);
#endif


typedef enum {
	kVOID,
	kINT8,
	kINT16,
	kINT32,
	kINT64,
	kUINT8,
	kUINT16,
	kUINT32,
	kUINT64,
	kFLOAT,
	kDOUBLE,
	kLDOUBLE,
	kSTRING = 0x20,
	kBOOL = 0x21,
	kTIMESTAMP = 0x44,
	kCOMPLEXFLOAT = 0x08000c,
	kCOMPLEXDOUBLE = 0x10000d,
} tdms_id;


const XboxDataType XboxDataType::NATIVE_VOID(kVOID);
const XboxDataType XboxDataType::NATIVE_BOOL(kBOOL);

const XboxDataType XboxDataType::NATIVE_INT8(kINT8);
const XboxDataType XboxDataType::NATIVE_UINT8(kUINT8);
const XboxDataType XboxDataType::NATIVE_INT16(kINT16);
const XboxDataType XboxDataType::NATIVE_UINT16(kUINT16);
const XboxDataType XboxDataType::NATIVE_INT32(kINT32);
const XboxDataType XboxDataType::NATIVE_UINT32(kUINT32);
const XboxDataType XboxDataType::NATIVE_INT64(kINT64);
const XboxDataType XboxDataType::NATIVE_UINT64(kUINT64);

const XboxDataType XboxDataType::NATIVE_FLOAT(kFLOAT);
const XboxDataType XboxDataType::NATIVE_DOUBLE(kDOUBLE);
const XboxDataType XboxDataType::NATIVE_LDOUBLE(kLDOUBLE);
const XboxDataType XboxDataType::NATIVE_STRING(kSTRING);
const XboxDataType XboxDataType::NATIVE_TIMESTAMP(kTIMESTAMP);
const XboxDataType XboxDataType::NATIVE_COMPLEXFLOAT(kCOMPLEXFLOAT);
const XboxDataType XboxDataType::NATIVE_COMPLEXDOUBLE(kCOMPLEXDOUBLE);

XboxDataType::XboxDataType(UInt_t id)
{
	p_setId(id);
}

// XboxDataType::XboxDataType(const XboxDataType& type_class )
// {
// 	p_setId(type_class.getId());
// }

XboxDataType::~XboxDataType()
{
}

void XboxDataType::p_setId(const UInt_t new_id)
{
	fId = new_id;
}

XboxDataType& XboxDataType::operator=( const XboxDataType& rhs )
{
	p_setId(rhs.getId());
    return(*this);
}

bool XboxDataType::operator==(const XboxDataType& compared_type ) const
{
	if(fId == compared_type.getId())
		return true;
	else
		return false;
}

size_t XboxDataType::getSize() const
{
	if((*this) == NATIVE_VOID)
		return 0;
	else if((*this) == NATIVE_BOOL){
		Bool_t val;
		return sizeof(val);
	}
	else if((*this) == NATIVE_INT8){
		Char_t val;
		return sizeof(val);
	}
	else if((*this) == NATIVE_INT16){
		Short_t val;
		return sizeof(val);
	}
	else if((*this) == NATIVE_INT32){
		Int_t val;
		return sizeof(val);
	}
	else if((*this) == NATIVE_INT64){
		Long64_t val;
		return sizeof(val);
	}
	else if((*this) == NATIVE_UINT8){
		UChar_t val;
		return sizeof(val);
	}
	else if((*this) == NATIVE_UINT16){
		UShort_t val;
		return sizeof(val);
	}
	else if((*this) == NATIVE_UINT32){
		UInt_t val;
		return sizeof(val);
	}
	else if((*this) == NATIVE_UINT64){
		ULong64_t val;
		return sizeof(val);
	}
	else if((*this) == NATIVE_FLOAT){
		Float_t val;
		return sizeof(val);
	}
	else if((*this) == NATIVE_DOUBLE){
		Double_t val;
		return sizeof(val);
	}
	else if((*this) == NATIVE_LDOUBLE){
		LongDouble_t val;
		return sizeof(val);
	}
	else if((*this) == NATIVE_COMPLEXFLOAT){
		Float_t val;
		return 2*sizeof(val);
	}
	else if((*this) == NATIVE_COMPLEXDOUBLE){
		Double_t val;
		return 2*sizeof(val);
	}
	else
		return 0;
}



std::string XboxDataType::getAlias() const
{
	if((*this) == NATIVE_VOID)
		return "VOID";
	else if((*this) == NATIVE_BOOL){
		return "BOOL";
	}
	else if((*this) == NATIVE_INT8){
		return "INT8";
	}
	else if((*this) == NATIVE_INT16){
		return "INT16";
	}
	else if((*this) == NATIVE_INT32){
		return "INT32";
	}
	else if((*this) == NATIVE_INT64){
		return "INT64";
	}
	else if((*this) == NATIVE_UINT8){
		return "UINT8";
	}
	else if((*this) == NATIVE_UINT16){
		return "UINT16";
	}
	else if((*this) == NATIVE_UINT32){
		return "UINT32";
	}
	else if((*this) == NATIVE_UINT64){
		return "UINT64";
	}
	else if((*this) == NATIVE_FLOAT){
		return "FLOAT";
	}
	else if((*this) == NATIVE_DOUBLE){
		return "DOUBLE";
	}
	else if((*this) == NATIVE_LDOUBLE){
		return "LDOUBLE";
	}
	else if((*this) == NATIVE_COMPLEXFLOAT){
		return "COMPLEXFLOAT";
	}
	else if((*this) == NATIVE_COMPLEXDOUBLE){
		return "COMPLEXDOUBLE";
	}
	else
		return 0;
}


bool XboxDataType::isVariableStr() const
{
	if((*this) == NATIVE_STRING)
		return true;
	else
		return false;
}

#ifndef XBOX_NO_NAMESPACE
}
#endif
