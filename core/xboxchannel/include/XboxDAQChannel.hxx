// @(#)root/cont:$Id$
// Author: Kai Papke   17/08/18

/*************************************************************************
 * Copyright (C) 2017-2018, Kai Papke.                                   *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef __XBOXDAQCHANNEL_HXX_
#define __XBOXDAQCHANNEL_HXX_

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TDAQChannel                                                      //
// Abstract base class used by                                          //
// TDAQChannelC, TDAQChannelS, TDAQChannelI                 //
// TDAQChannelL, TDAQChannelF, TDAQChannelD                 //
//////////////////////////////////////////////////////////////////////////

#include <iostream>     // std::cout
#include <stdlib.h>
#include <string>
#include <vector>
#include <map>
#include <ctime>

#include "Rtypes.h"
#include "TObject.h"
#include "TTimeStamp.h"

#include "TArrayD.h"
#include "TMap.h"

#include "XboxDataType.hxx"

#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif


class XboxDAQChannel : public TObject  {

protected:

	std::string           fChannelName;               ///<NI_ChannelName
	Int_t                 fXboxVersion;               ///<Version of Xbox

	TTimeStamp            fTimeStamp;                 ///<Timestamp // @suppress("Type cannot be resolved")

	Int_t                 fLogType;                   ///<Log_Type
	ULong64_t             fPulseCount;                ///<Pulse_Count
	Double_t              fDeltaF;                    ///<DeltaF
	Int_t                 fLine;                      ///<Line

	Bool_t                fBreakdownFlag;             ///<breakdown flag
	Int_t                 fBreakdownType;             ///<breakdown type
	Int_t                 fBreakdownThreshDir;        ///<breakdown threshold direction
	Int_t                 fBreakdownThreshDirVal;     ///<breakdown threshold direction value
	Double_t              fBreakdownRatioVal;         ///<breakdown ratio

	TTimeStamp            fStartTime;                 ///<wf_start_time // @suppress("Type cannot be resolved")
	Double_t              fStartOffset;               ///<wf_start_offset
	Double_t              fIncrement;                 ///<wf_increment
	Int_t                 fNSamples;                   ///<wf_samples

	std::string           fXLabel;                    ///<wf_xname;
	std::string           fXUnit;                     ///<wf_xunit_string
	std::string           fYUnit;                     ///<unit_string
	std::string           fYUnitDescription;          ///<NI_UnitDescription

	// Scale type: -1 - None, 0 - logarithmic, 1 - Polynomial
	Int_t                 fScaleType;                 ///<Scale_Type
	std::string           fScaleUnit;                 ///<Scale_Unit;
	std::vector<Double_t> fScaleCoeffs;               ///<Scale_Coeffs;

	XboxDataType          fDataType;                  ///<Data type
	std::vector<Byte_t>   fRawData;                   ///<Raw Data
	std::vector<Double_t> fData;                      ///<!Interpreted data

	// analysis
	Double_t              fXmin;                      ///<Marker of rising edge
	Double_t              fXmax;                      ///<Time of falling edge
	Double_t              fXdev;                      ///<Time of deviation

	Double_t              fYmin;                      ///<Minumum power over the pulse length
	Double_t              fYmax;                      ///<Maximum power over the pulse length
	Double_t              fYmean;                     ///<Average power over the pulse length
	Double_t              fYinteg;                    ///<Integral of the signal
	Double_t              fYspan;                     ///<Peak to Peak value of the signal


	Bool_t                fAutoRefresh;               ///<!automatic refresh before reading data
	void                  viewData(vector<Double_t> &data);
	Int_t                 getIndexRange(Int_t &i0, Int_t &i1, Double_t t0, Double_t t1);

public:
	
	XboxDAQChannel();
	virtual ~XboxDAQChannel();

	void                  init();
	void                  clear();
	void                  flushbuffer();
	void                  reset();

	Int_t                 isEmpty();
	void                  print();

	// comparison operators refer to the time stamp
    Bool_t                operator < (const XboxDAQChannel& rhs) const;
    Bool_t                operator > (const XboxDAQChannel& rhs) const;

    // logical operators return state of breakdown flag
//    Bool_t                operator && (const XboxDAQChannel& rhs) const;
//    Bool_t                operator || (const XboxDAQChannel& rhs) const;
//    Bool_t                operator ! () const;


	std::vector<Double_t> getSignal(Double_t t0=-1, Double_t t1=-1);
	std::vector<Double_t> getTimeAxis(Double_t t0=-1, Double_t t1=-1);
	void                  getTimeAxisBounds(Double_t &t0, Double_t &t1) const;


//	Double_t              getPulseHeight(Double_t threshold=0.9);
//	Double_t              getPulseWidth(Double_t threshold=0.9);
	Double_t              min(Double_t t0=-1, Double_t t1=-1);
	Double_t              max(Double_t t0=-1, Double_t t1=-1);
	Double_t              magn(Double_t t0=-1, Double_t t1=-1);
	Double_t              span(Double_t t0=-1, Double_t t1=-1);
	Double_t              sum(Double_t t0=-1, Double_t t1=-1);
	Double_t              mean(Double_t t0=-1, Double_t t1=-1);
	Double_t              stddev(Double_t t0=-1, Double_t t1=-1);
	Double_t              median(Double_t t0=-1, Double_t t1=-1);
	Double_t              integ(Double_t t0=-1, Double_t t1=-1);

	Double_t              risingEdge(Double_t threshold=0.9, Double_t t0=-1, Double_t t1=-1);
	Double_t              fallingEdge(Double_t threshold=0.9, Double_t t0=-1, Double_t t1=-1);

	std::string           getChannelName() const { return fChannelName; }
	Int_t                 getXboxVersion() const { return fXboxVersion; }

	TTimeStamp            getTimeStamp() const { return fTimeStamp; } // @suppress("Type cannot be resolved")

	Int_t                 getLogType() const { return fLogType; }
	ULong64_t             getPulseCount() const { return fPulseCount; }
	Double_t              getDeltaF() const { return fDeltaF; }
	Int_t                 getLine() const { return fLine; }
	
	Bool_t                getBreakdownFlag() const { return fBreakdownFlag; }
	Int_t                 getBreakdownType() const { return fBreakdownType; }
	Int_t                 getBreakdownThreshDir() const { return fBreakdownThreshDir; }
	Int_t                 getBreakdownThreshDirVal() const { return fBreakdownThreshDirVal; }
	Double_t              getBreakdownRatioVal() const { return fBreakdownRatioVal; }
	
	TTimeStamp            getStartTime() const { return fStartTime; } // @suppress("Type cannot be resolved")
	Double_t              getStartOffset() const { return fStartOffset; }
	Double_t              getIncrement() const { return fIncrement; }
	Int_t                 getSamples() const { return fNSamples; }

	std::string           getXLabel() const { return fXLabel; }
	std::string           getXUnit() const { return fXUnit; }
	std::string           getYUnit() const { return fYUnit; }
	std::string           getYUnitDescription() const { return fYUnitDescription; }

	Int_t                 getScaleType() const { return fScaleType; }
	std::string           getScaleUnit() const { return fScaleUnit; }
	std::vector<Double_t> getScaleCoeffs() const { return fScaleCoeffs; }
	
	XboxDataType          getDataType() const { return fDataType; }
	std::vector<Byte_t>   getRawData() const { return fRawData; }

	Double_t              getXmin() const { return fXmin; }
	Double_t              getXmax() const { return fXmax; }
	Double_t              getXdev() const { return fXdev; }

	Double_t              getYmin() const { return fYmin; }
	Double_t              getYmax() const { return fYmax; }
	Double_t              getYmean() const { return fYmean; }
	Double_t              getYinteg() const { return fYinteg; }
	Double_t              getYspan() const { return fYspan; }


	XboxDAQChannel        cloneMetaData() const;


	void                  setChannelName(const std::string &name) { fChannelName = name; };
	void                  setChannelName(const Char_t* name) { fChannelName = name; }
	void                  setXboxVersion(Int_t version) { fXboxVersion = version; };

	void                  setTimeStamp(const TTimeStamp &ts) { fTimeStamp = ts; }
	void                  setLogType(Int_t val) { fLogType = val; }
	void                  setPulseCount(ULong64_t val) { fPulseCount = val; }
	void                  setDeltaF(Double_t val) { fDeltaF = val; }
	void                  setLine(Int_t val) { fLine = val; }

	void                  setBreakdownFlag(Int_t val) { fBreakdownFlag = val; }
	void                  setBreakdownType(Int_t val) { fBreakdownType = val; }
	void                  setBreakdownThreshDir(Int_t val) { fBreakdownThreshDir = val; }
	void                  setBreakdownThreshDirVal(Int_t val) { fBreakdownThreshDirVal = val; }
	void                  setBreakdownRatioVal(Double_t val) { fBreakdownRatioVal = val; }

	void                  setStartTime(const TTimeStamp &ts) { fStartTime = ts; }
	void                  setStartOffset(Double_t val) { fStartOffset = val; }
	void                  setIncrement(Double_t val) { fIncrement = val; }
	void                  setSamples(Int_t val) { fNSamples = val; }

	void                  setXLabel(const std::string &sval) { fXLabel = sval; }
	void                  setXUnit(const std::string &sval) { fXUnit = sval; }
	void                  setYUnit(const std::string &sval) { fYUnit = sval; }
	void                  setYUnitDescription(const std::string &sval) { fYUnitDescription = sval; }

	void                  setScaleType(Int_t val) { fScaleType = val; }
	void                  setScaleUnit(const std::string &sval) { fScaleUnit = sval; }
	void                  setScaleCoeffs(const std::vector<Double_t> &val) { fScaleCoeffs = val; }

	void                  setDataType(XboxDataType dtype) { fDataType = dtype; }
	void                  setDataTypeId(UInt_t id) { fDataType = XboxDataType(id); }
	void                  setRawData(const std::vector<Byte_t> &val) { fRawData = val; }


	void                  setXmin(Double_t val) { fXmin = val; }
	void                  setXmax(Double_t val) { fXmax = val; }
	void                  setXdev(Double_t val) { fXdev = val; }

	void                  setYmin(Double_t val) { fYmin = val; }
	void                  setYmax(Double_t val) { fYmax = val; }
	void                  setYmean(Double_t val) { fYmean = val; }
	void                  setYinteg(Double_t val) { fYinteg = val; }
	void                  setYspan(Double_t val) { fYspan = val; }


	void                  setAutoRefresh(Bool_t flag) {	fAutoRefresh = flag; }

	ClassDef(XboxDAQChannel,1);	// Xbox Data Acquisition Channel class 
};

#ifndef XBOX_NO_NAMESPACE
}
#endif

#endif /* __XBOXDAQCHANNEL_HXX_ */
