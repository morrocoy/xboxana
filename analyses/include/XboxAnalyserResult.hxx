/*
 * XboxAnalysis.hxx
 *
 *  Created on: Oct 31, 2018
 *      Author: kpapke
 */

#ifndef _XBOXANALYSERRESULT_HXX_
#define _XBOXANALYSERRESULT_HXX_

#include <iostream>
#include "Rtypes.h"
#include "TObject.h"
#include "TTimeStamp.h"


#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif


class XboxAnalyserResult : public TObject
{

protected:

	Int_t                 fXboxVersion;               ///<Version of Xbox
	TTimeStamp            fTimeStamp;                 ///<Timestamp // @suppress("Type cannot be resolved")
	Int_t                 fLogType;                   ///<Log_Type: -1 - no event, 0 - BD, 1 - first pulse after BD, 2 - second pulse after BD, ...
	ULong64_t             fPulseCount;                ///<Pulse_Count
	Double_t              fDeltaF;                    ///<DeltaF
	Int_t                 fLine;                      ///<Line
	Bool_t                fBreakdownFlag;             ///<breakdown flag set if isBreakdown() true

	Double_t              fRefXmin;                   ///<Time of rising edge of reference signal
	Double_t              fRefXmax;                   ///<Time of falling edge of reference signal
	Double_t              fRefYmin;                   ///<Minumum power over the pulse length of reference signal
	Double_t              fRefYmax;                   ///<Maximum power over the pulse length of reference signal
	Double_t              fRefYavg;                   ///<Average power over the pulse length of reference signal
	Double_t              fRefYint;                   ///<Integral of the reference signal
	Double_t              fRefYpp;                    ///<Peak to Peak value of the reference signal

	Double_t              fXmin;                      ///<Time of rising edge
	Double_t              fXmax;                      ///<Time of falling edge
	Double_t              fYmin;                      ///<Minumum power over the pulse length
	Double_t              fYmax;                      ///<Maximum power over the pulse length
	Double_t              fYavg;                      ///<Average power over the pulse length
	Double_t              fYint;                      ///<Integral of the signal
	Double_t              fYpp;                       ///<Peak to Peak value of the signal

	Double_t              fJitter;                    ///<Jitter between reference and main signal
	Double_t              fXdev;                   ///<The time at with main signal starts to differ from the reference.

	std::vector<Double_t> fTestingBuffer1;            ///<Testing: Visual inspection of fTranRisingEdge
	std::vector<Double_t> fTestingBuffer2;            ///<Testing: Visual inspection of fTranBreakdownTime


public:

    XboxAnalyserResult();
	virtual ~XboxAnalyserResult();

	void                  init();
	void                  clear();
	void                  reset();

	Bool_t                isValid() const { return (fXboxVersion > 0); }
	void                  print();

	// comparison operators refer to the time stamp
    Bool_t                operator < (const XboxAnalyserResult& rhs) const;
    Bool_t                operator > (const XboxAnalyserResult& rhs) const;

    // getter
    Int_t                 getXboxVersion() const { return fXboxVersion; }
	TTimeStamp            getTimeStamp() const { return fTimeStamp; }
	Int_t                 getLogType() const { return fLogType; }
	ULong64_t             getPulseCount() const { return fPulseCount; }
	Double_t              getDeltaF() const { return fDeltaF; }
	Int_t                 getLine() const { return fLine; }
	Bool_t                getBreakdownFlag() const { return fBreakdownFlag; }

	Double_t              getRefXmin() const { return fRefXmin; }
	Double_t              getRefXmax() const { return fRefXmax; }
	Double_t              getRefYmin() const { return fRefYmin; }
	Double_t              getRefYmax() const { return fRefYmax; }
	Double_t              getRefYavg() const { return fRefYavg; }
	Double_t              getRefYint() const { return fRefYint; }
	Double_t              getRefYpp() const { return fRefYpp; }

	Double_t              getXmin() const { return fXmin; }
	Double_t              getXmax() const { return fXmax; }
	Double_t              getYmin() const { return fYmin; }
	Double_t              getYmax() const { return fYmax; }
	Double_t              getYavg() const { return fYavg; }
	Double_t              getYint() const { return fYint; }
	Double_t              getYpp() const { return fYpp; }

	Double_t              getJitter() const { return fJitter; }
	Double_t              getXdev() const { return fXdev; }

	std::vector<Double_t> getTestingBuffer1() const { return fTestingBuffer1; }
	std::vector<Double_t> getTestingBuffer2() const { return fTestingBuffer2; }

	// setter
	void                  setXboxVersion(Int_t version) { fXboxVersion = version; };
	void                  setTimeStamp(const TTimeStamp &ts) { fTimeStamp = ts; }
	void                  setLogType(Int_t val) { fLogType = val; }
	void                  setPulseCount(ULong64_t val) { fPulseCount = val; }
	void                  setDeltaF(Double_t val) { fDeltaF = val; }
	void                  setLine(Int_t val) { fLine = val; }
	void                  setBreakdownFlag(Int_t val) { fBreakdownFlag = val; }

	void                  setRefXmin(Double_t val) { fRefXmin = val; }
	void                  setRefXmax(Double_t val) { fRefXmax = val; }
	void                  setRefYmin(Double_t val) { fRefYmin = val; }
	void                  setRefYmax(Double_t val) { fRefYmax = val; }
	void                  setRefYavg(Double_t val) { fRefYavg = val; }
	void                  setRefYint(Double_t val) { fRefYint = val; }
	void                  setRefYpp(Double_t val) { fRefYpp = val; }

	void                  setXmin(Double_t val) { fXmin = val; }
	void                  setXmax(Double_t val) { fXmax = val; }
	void                  setYmin(Double_t val) { fYmin = val; }
	void                  setYmax(Double_t val) { fYmax = val; }
	void                  setYavg(Double_t val) { fYavg = val; }
	void                  setYint(Double_t val) { fYint = val; }
	void                  setYpp(Double_t val) { fYpp = val; }

	void                  setJitter(Double_t val) { fJitter = val; }
	void                  setXdev(Double_t val) { fXdev = val; }

	void                  setTestingBuffer1(std::vector<Double_t> vec) { fTestingBuffer1 = vec; }
	void                  setTestingBuffer2(std::vector<Double_t> vec) { fTestingBuffer2 = vec; }


	ClassDef(XboxAnalyserResult,1);	// Class to define data structure of pulse parameters

};

#ifndef XBOX_NO_NAMESPACE
}
#endif

#endif /* _XBOXANALYSERRESULT_HXX_ */
