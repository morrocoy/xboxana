/***************************************************************************
File                 : TEndianIfstream.hxx
--------------------------------------------------------------------
Copyright            : (C) 2008 Alex Kargovsky
					   Email (use @ for *)  : ion_vasilief*yahoo.fr
Description          : Endian file stream class
***************************************************************************/

#ifndef __TDMSIFSTREAM_HXX__
#define __TDMSIFSTREAM_HXX__

#include "Rtypes.h"

#include <iostream>     // std::cout
#include <fstream>      // std::ifstream
#include <stdlib.h>
#include <string>

using namespace std;

namespace TDMS {

class TdmsIfstream : public ifstream
{
public:
	TdmsIfstream(const Char_t *_Filename, ios_base::openmode _Mode = ios_base::in)
		:	ifstream(_Filename, _Mode)
	{
		Short_t word = 0x4321;
		bigEndian = (*(Char_t*)& word) != 0x21;
	};

	TdmsIfstream& operator>>(Bool_t& value)
	{
		Char_t c;
		get(c);
		value = (c != 0);
		return *this;
	}

	TdmsIfstream& operator>>(Char_t& value)
	{
		get(value);
		return *this;
	}

	TdmsIfstream& operator>>(UChar_t& value)
	{
		get(reinterpret_cast<Char_t&>(value));
		return *this;
	}

	TdmsIfstream& operator>>(Short_t& value)
	{
		read(reinterpret_cast<Char_t*>(&value), sizeof(value));
		if(bigEndian)
			swap_bytes(reinterpret_cast<UChar_t*>(&value), (Int_t)sizeof(value));

		return *this;
	}

	TdmsIfstream& operator>>(UShort_t& value)
	{
		read(reinterpret_cast<Char_t*>(&value), sizeof(value));
		if(bigEndian)
			swap_bytes(reinterpret_cast<UChar_t*>(&value), (Int_t)sizeof(value));

		return *this;
	}

	TdmsIfstream& operator>>(Int_t& value)
	{
		read(reinterpret_cast<Char_t*>(&value), sizeof(value));
		if(bigEndian)
			swap_bytes(reinterpret_cast<UChar_t*>(&value), (Int_t)sizeof(value));

		return *this;
	}

	TdmsIfstream& operator>>(UInt_t& value)
	{
		read(reinterpret_cast<Char_t*>(&value), sizeof(value));
		if(bigEndian)
			swap_bytes(reinterpret_cast<UChar_t*>(&value), sizeof(value));

		return *this;
	}

	TdmsIfstream& operator>>(Long_t& value)
	{
		read(reinterpret_cast<Char_t*>(&value), sizeof(value));
		if(bigEndian)
			swap_bytes(reinterpret_cast<UChar_t*>(&value), sizeof(value));

		return *this;
	}

	TdmsIfstream& operator>>(ULong_t& value)
	{
		read(reinterpret_cast<Char_t*>(&value), sizeof(value));
		if(bigEndian)
			swap_bytes(reinterpret_cast<UChar_t*>(&value), sizeof(value));

		return *this;
	}

	TdmsIfstream& operator>>(Long64_t& value)
	{
		read(reinterpret_cast<Char_t*>(&value), sizeof(value));
		if(bigEndian)
			swap_bytes(reinterpret_cast<UChar_t*>(&value), sizeof(value));

		return *this;
	}

	TdmsIfstream& operator>>(ULong64_t& value)
	{
		read(reinterpret_cast<Char_t*>(&value), sizeof(value));
		if(bigEndian)
			swap_bytes(reinterpret_cast<UChar_t*>(&value), sizeof(value));

		return *this;
	}

	TdmsIfstream& operator>>(Float_t& value)
	{
		read(reinterpret_cast<Char_t*>(&value), sizeof(value));
		if(bigEndian)
			swap_bytes(reinterpret_cast<UChar_t*>(&value), sizeof(value));

		return *this;
	}

	TdmsIfstream& operator>>(Double_t& value)
	{
		read(reinterpret_cast<Char_t*>(&value), sizeof(value));
		if(bigEndian)
			swap_bytes(reinterpret_cast<UChar_t*>(&value), sizeof(value));

		return *this;
	}

	TdmsIfstream& operator>>(LongDouble_t& value)
	{
		read(reinterpret_cast<Char_t*>(&value), sizeof(value));
		if(bigEndian)
			swap_bytes(reinterpret_cast<UChar_t*>(&value), sizeof(value));

		return *this;
	}

//		iendianfstream& operator>>(TString& value)
//		{
//			read(reinterpret_cast<char*>(&value[0]), (UInt_t)value.Length());
//			Ssiz_t pos = value.First('\0');
//			if(pos != value.kNPOS)
//				value.Resize(pos);
//
//			return *this;
//		}

	TdmsIfstream& operator>>(std::string& value)
	{
		read(reinterpret_cast<char*>(&value[0]), value.size());
		std::string::size_type pos = value.find_first_of('\0');
		if(pos != std::string::npos)
			value.resize(pos);

		return *this;
	}

	void readArray(Bool_t* values, UInt_t size){
		Char_t * buf;
		buf = (Char_t *) malloc(size * sizeof(Char_t));
		read(reinterpret_cast<Char_t*>(buf), size * sizeof(Char_t));
		if(bigEndian)
			swap_bytes(reinterpret_cast<UChar_t*>(buf), size * sizeof(Char_t));
		for (UInt_t i = 0; i < size; ++i){
			values[i] = (buf[i] != 0);
		}
	}

	void readArray(Char_t* values, UInt_t size){
		read(reinterpret_cast<Char_t*>(values), size * sizeof(Char_t));
		if(bigEndian)
			swap_bytes(reinterpret_cast<UChar_t*>(values), size * sizeof(Char_t));
	}

	void readArray(UChar_t* values, UInt_t size){
		read(reinterpret_cast<Char_t*>(values), size * sizeof(UChar_t));
		if(bigEndian)
			swap_bytes(reinterpret_cast<UChar_t*>(values), size * sizeof(UChar_t));
	}

	void readArray(Short_t* values, UInt_t size){
		read(reinterpret_cast<Char_t*>(values), size * sizeof(Short_t));
		if(bigEndian)
			swap_bytes(reinterpret_cast<UChar_t*>(values), size * sizeof(Short_t));
	}

	void readArray(UShort_t* values, UInt_t size){
		read(reinterpret_cast<Char_t*>(values), size * sizeof(UShort_t));
		if(bigEndian)
			swap_bytes(reinterpret_cast<UChar_t*>(values), size * sizeof(UShort_t));
	}

	void readArray(Int_t* values, UInt_t size){
		read(reinterpret_cast<Char_t*>(values), size * sizeof(Int_t));
		if(bigEndian)
			swap_bytes(reinterpret_cast<UChar_t*>(values), size * sizeof(Int_t));
	}

	void readArray(UInt_t* values, UInt_t size){
		read(reinterpret_cast<Char_t*>(values), size * sizeof(UInt_t));
		if(bigEndian)
			swap_bytes(reinterpret_cast<UChar_t*>(values), size * sizeof(UInt_t));
	}

	void readArray(Long_t* values, UInt_t size){
		read(reinterpret_cast<Char_t*>(values), size * sizeof(Long_t));
		if(bigEndian)
			swap_bytes(reinterpret_cast<UChar_t*>(values), size * sizeof(Long_t));
	}
	void readArray(ULong_t* values, UInt_t size){
		read(reinterpret_cast<Char_t*>(values), size * sizeof(ULong_t));
		if(bigEndian)
			swap_bytes(reinterpret_cast<UChar_t*>(values), size * sizeof(ULong_t));
	}

	void readArray(Long64_t* values, UInt_t size){
		read(reinterpret_cast<Char_t*>(values), size * sizeof(Long64_t));
		if(bigEndian)
			swap_bytes(reinterpret_cast<UChar_t*>(values), size * sizeof(Long64_t));
	}
	void readArray(ULong64_t* values, UInt_t size){
		read(reinterpret_cast<Char_t*>(values), size * sizeof(ULong64_t));
		if(bigEndian)
			swap_bytes(reinterpret_cast<UChar_t*>(values), size * sizeof(ULong64_t));
	}

	void readArray(Float_t* values, UInt_t size){
		read(reinterpret_cast<Char_t*>(values), size * sizeof(Float_t));
		if(bigEndian)
			swap_bytes(reinterpret_cast<UChar_t*>(values), size * sizeof(Float_t));
	}

	void readArray(Double_t* values, UInt_t size){
		read(reinterpret_cast<Char_t*>(values), size * sizeof(Double_t));
		if(bigEndian)
			swap_bytes(reinterpret_cast<UChar_t*>(values), size * sizeof(Double_t));
	}

	void readArray(LongDouble_t* values, UInt_t size){
		read(reinterpret_cast<Char_t*>(values), size * sizeof(LongDouble_t));
		if(bigEndian)
			swap_bytes(reinterpret_cast<UChar_t*>(values), size * sizeof(LongDouble_t));
	}

private:
	bool bigEndian;
	void swap_bytes(UChar_t* data, UInt_t size)
	{
		Int_t i = 0, j = size - 1;
		while(i < j)
		{
			std::swap(data[i], data[j]);
			++i, --j;
		}
	}
};


} // end of namespace TDMS


#endif /* __TDMSIFSTREAM_HXX__ */
