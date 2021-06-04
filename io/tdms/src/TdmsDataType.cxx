#include "TdmsDataType.hxx"

//#include <string>

namespace TDMS {


typedef enum {
	kTDMSNATIVE_VOID,
	kTDMSNATIVE_INT8,
	kTDMSNATIVE_INT16,
	kTDMSNATIVE_INT32,
	kTDMSNATIVE_INT64,
	kTDMSNATIVE_UINT8,
	kTDMSNATIVE_UINT16,
	kTDMSNATIVE_UINT32,
	kTDMSNATIVE_UINT64,
	kTDMSNATIVE_FLOAT,
	kTDMSNATIVE_DOUBLE,
	kTDMSNATIVE_LDOUBLE,
	kTDMSNATIVE_FLOATWITHUNIT = 0x19,
	kTDMSNATIVE_DOUBLEWITHUNIT,
	kTDMSNATIVE_LDOUBLEWITHUNIT,
	kTDMSNATIVE_STRING = 0x20,
	kTDMSNATIVE_BOOL = 0x21,
	kTDMSNATIVE_TIMESTAMP = 0x44,
	kTDMSNATIVE_FIXEDPOINT = 0x4F,
	kTDMSNATIVE_COMPLEXFLOAT = 0x08000c,
	kTDMSNATIVE_COMPLEXDOUBLE = 0x10000d,
	kTDMSNATIVE_DAQMXRAWDATA = 0xFFFFFFFF
} tdms_id;


const TdmsDataType TdmsDataType::NATIVE_VOID(kTDMSNATIVE_VOID);
const TdmsDataType TdmsDataType::NATIVE_INT8(kTDMSNATIVE_INT8);
const TdmsDataType TdmsDataType::NATIVE_UINT8(kTDMSNATIVE_UINT8);
const TdmsDataType TdmsDataType::NATIVE_INT16(kTDMSNATIVE_INT16);
const TdmsDataType TdmsDataType::NATIVE_UINT16(kTDMSNATIVE_UINT16);
const TdmsDataType TdmsDataType::NATIVE_INT32(kTDMSNATIVE_INT32);
const TdmsDataType TdmsDataType::NATIVE_UINT32(kTDMSNATIVE_UINT32);
const TdmsDataType TdmsDataType::NATIVE_INT64(kTDMSNATIVE_INT64);
const TdmsDataType TdmsDataType::NATIVE_UINT64(kTDMSNATIVE_UINT64);

const TdmsDataType TdmsDataType::NATIVE_FLOAT(kTDMSNATIVE_FLOAT);
const TdmsDataType TdmsDataType::NATIVE_DOUBLE(kTDMSNATIVE_DOUBLE);
const TdmsDataType TdmsDataType::NATIVE_LDOUBLE(kTDMSNATIVE_LDOUBLE);
const TdmsDataType TdmsDataType::NATIVE_FLOATWITHUNIT(kTDMSNATIVE_FLOATWITHUNIT);
const TdmsDataType TdmsDataType::NATIVE_DOUBLEWITHUNIT(kTDMSNATIVE_DOUBLEWITHUNIT);
const TdmsDataType TdmsDataType::NATIVE_LDOUBLEWITHUNIT(kTDMSNATIVE_LDOUBLEWITHUNIT);
const TdmsDataType TdmsDataType::NATIVE_STRING(kTDMSNATIVE_STRING);
const TdmsDataType TdmsDataType::NATIVE_BOOL(kTDMSNATIVE_BOOL);
const TdmsDataType TdmsDataType::NATIVE_TIMESTAMP(kTDMSNATIVE_TIMESTAMP);
const TdmsDataType TdmsDataType::NATIVE_FIXEDPOINT(kTDMSNATIVE_FIXEDPOINT);
const TdmsDataType TdmsDataType::NATIVE_COMPLEXFLOAT(kTDMSNATIVE_COMPLEXFLOAT);
const TdmsDataType TdmsDataType::NATIVE_COMPLEXDOUBLE(kTDMSNATIVE_COMPLEXDOUBLE);
const TdmsDataType TdmsDataType::NATIVE_DAQMXRAWDATA(kTDMSNATIVE_DAQMXRAWDATA);

TdmsDataType::TdmsDataType(size_t id)
{
	p_setId(id);
}


TdmsDataType::TdmsDataType(const TdmsDataType& type_class )
{
	p_setId(type_class.getId());
}

TdmsDataType::~TdmsDataType()
{
}

void TdmsDataType::p_setId(const size_t new_id)
{
	fId = new_id;
}

TdmsDataType& TdmsDataType::operator=( const TdmsDataType& rhs )
{
	p_setId(rhs.getId());
    return(*this);
}

bool TdmsDataType::operator==(const TdmsDataType& compared_type ) const
{
	if(fId == compared_type.getId())
		return true;
	else
		return false;
}

size_t TdmsDataType::getSize() const
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
	else if((*this) == NATIVE_FLOATWITHUNIT){
		Float_t val;
		return sizeof(val);
	}
	else if((*this) == NATIVE_DOUBLE){
		Double_t val;
		return sizeof(val);
	}
	else if((*this) == NATIVE_DOUBLEWITHUNIT){
		Double_t val;
		return sizeof(val);
	}
	else if((*this) == NATIVE_LDOUBLE){
		LongDouble_t val;
		return sizeof(val);
	}
	else if((*this) == NATIVE_LDOUBLEWITHUNIT){
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

bool TdmsDataType::isVariableStr() const
{
	if((*this) == NATIVE_STRING)
		return true;
	else
		return false;
}


} // end of namespace TDMS

