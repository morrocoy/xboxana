/*
 * XboxAnalysis.hxx
 *
 *  Created on: Oct 31, 2018
 *      Author: kpapke
 */

#ifndef _XBOXANALYSERENTRY_HXX_
#define _XBOXANALYSERENTRY_HXX_

#include <iostream>
#include "Rtypes.h"
#include "TObject.h"
#include "TTimeStamp.h"


#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif


class XboxAnalyserEntry : public TObject
{

protected:

	Int_t                 fXboxVersion;               ///<Version of Xbox
	TTimeStamp            fTimeStamp;                 ///<Timestamp // @suppress("Type cannot be resolved")
	Int_t                 fLogType;                   ///<Log_Type: -1 - no event, 0 - BD, 1 - first pulse after BD, 2 - second pulse after BD, ...
	ULong64_t             fPulseCount;                ///<Pulse_Count
	Double_t              fDeltaF;                    ///<DeltaF
	Int_t                 fLine;                      ///<Line
	Bool_t                fBreakdownFlag;             ///<breakdown flag set if isBreakdown() true

	Double_t              fPulseRisingEdge;           ///<Time of rising edge
	Double_t              fPulseFallingEdge;          ///<Time of falling edge
	Double_t              fPulseLength;               ///<Pulse length
	Double_t              fPulsePowerMax;             ///<Maximum power over the pulse length
	Double_t              fPulsePowerMin;             ///<Maximum power over the pulse length
	Double_t              fPulsePowerAvg;             ///<Average power over the pulse length
	Double_t              fPeakPeakValue;             ///<Peak to Peak value of the signal

	Double_t              fJitter;                    ///<Jitter between previous and current RF signal
	Double_t              fTranRisingEdge;            ///<The time of the rising edge of the previous pulse (transmitted signal).
	Double_t              fTranBreakdownTime;         ///<The time when previous and current signal start to deviate (transmitted signal).
	Double_t              fReflRisingEdge;            ///<The time of the rising edge of the previous pulse (reflected signal).
	Double_t              fReflBreakdownTime;         ///<The time when previous and current signal start to deviate (reflected signal).
	Double_t              fBreakdownTime;             ///<Time at which the breakdown occurs

	std::vector<Double_t> fProbeTranRisingEdge;       ///<Testing: Visual inspection of fTranRisingEdge
	std::vector<Double_t> fProbeTranBreakdownTime;    ///<Testing: Visual inspection of fTranBreakdownTime
	std::vector<Double_t> fProbeReflRisingEdge;       ///<Testing: Visual inspection of ReflRisingEdge
	std::vector<Double_t> fProbeReflBreakdownTime;    ///<Testing: Visual inspection of fReflBreakdownTime


public:

    XboxAnalyserEntry();
	virtual ~XboxAnalyserEntry();

	void                  init();
	void                  clear();
	void                  reset();

	Bool_t                isValid() const { return (fXboxVersion > 0); }
	void                  print();

	// comparison operators refer to the time stamp
    Bool_t                operator < (const XboxAnalyserEntry& rhs) const;
    Bool_t                operator > (const XboxAnalyserEntry& rhs) const;

    // getter
    Int_t                 getXboxVersion() const { return fXboxVersion; }
	TTimeStamp            getTimeStamp() const { return fTimeStamp; } // @suppress("Type cannot be resolved")
	Int_t                 getLogType() const { return fLogType; }
	ULong64_t             getPulseCount() const { return fPulseCount; }
	Double_t              getDeltaF() const { return fDeltaF; }
	Int_t                 getLine() const { return fLine; }
	Bool_t                getBreakdownFlag() const { return fBreakdownFlag; }

	Double_t              getPulseRisingEdge() const { return fPulseRisingEdge; }
	Double_t              getPulseFallingEdge() const { return fPulseFallingEdge; }
	Double_t              getPulseLength() const { return fPulseLength; }
	Double_t              getPulsePowerMax() const { return fPulsePowerMax; }
	Double_t              getPulsePowerMin() const { return fPulsePowerMin; }
	Double_t              getPulsePowerAvg() const { return fPulsePowerAvg; }
	Double_t              getPeakPeakValue() const { return fPeakPeakValue; }

	Double_t              getJitter() const { return fJitter; }
	Double_t              getTranRisingEdge() const { return fTranRisingEdge; }
	Double_t              getTranBreakdownTime() const { return fTranBreakdownTime; }
	Double_t              getReflRisingEdge() const { return fReflRisingEdge; }
	Double_t              getReflBreakdownTime() const { return fReflBreakdownTime; }
	Double_t              getBreakdownTime() const { return fBreakdownTime; }

	std::vector<Double_t> getProbeTranRisingEdge() const { return fProbeTranRisingEdge; }
	std::vector<Double_t> getProbeTranBreakdownTime() const { return fProbeTranBreakdownTime; }
	std::vector<Double_t> getProbeReflRisingEdge() const { return fProbeReflRisingEdge; }
	std::vector<Double_t> getProbeReflBreakdownTime() const { return fProbeReflBreakdownTime; }

	// setter
	void                  setXboxVersion(Int_t version) { fXboxVersion = version; };
	void                  setTimeStamp(const TTimeStamp &ts) { fTimeStamp = ts; }
	void                  setLogType(Int_t val) { fLogType = val; }
	void                  setPulseCount(ULong64_t val) { fPulseCount = val; }
	void                  setDeltaF(Double_t val) { fDeltaF = val; }
	void                  setLine(Int_t val) { fLine = val; }
	void                  setBreakdownFlag(Int_t val) { fBreakdownFlag = val; }

	void                  setPulseRisingEdge(Double_t val) { fPulseRisingEdge = val; }
	void                  setPulseFallingEdge(Double_t val) { fPulseFallingEdge = val; }
	void                  setPulseLength(Double_t val) { fPulseLength = val; }
	void                  setPulsePowerMin(Double_t val) { fPulsePowerMin = val; }
	void                  setPulsePowerMax(Double_t val) { fPulsePowerMax = val; }
	void                  setPulsePowerAvg(Double_t val) { fPulsePowerAvg = val; }
	void                  setPeakPeakValue(Double_t val) { fPeakPeakValue = val; }

	void                  setJitter(Double_t val) { fJitter = val; }
	void                  setTranRisingEdge(Double_t val) { fTranRisingEdge = val; }
	void                  setTranBreakdownTime(Double_t val) { fTranBreakdownTime = val; }
	void                  setReflRisingEdge(Double_t val) { fReflRisingEdge = val; }
	void                  setReflBreakdownTime(Double_t val) { fReflBreakdownTime = val; }
	void                  setBreakdownTime(Double_t val) { fBreakdownTime = val; }


	void                  setProbeTranRisingEdge(std::vector<Double_t> vec) { fProbeTranRisingEdge = vec; }
	void                  setProbeTranBreakdownTime(std::vector<Double_t> vec) { fProbeTranBreakdownTime = vec; }
	void                  setProbeReflRisingEdge(std::vector<Double_t> vec) { fProbeReflRisingEdge = vec; }
	void                  setProbeReflBreakdownTime(std::vector<Double_t> vec) { fProbeReflBreakdownTime = vec; }


	ClassDef(XboxAnalyserEntry,1);	// Class to define data structure of pulse parameters

};

#ifndef XBOX_NO_NAMESPACE
}
#endif

#endif /* _XBOXANALYSERENTRY_HXX_ */
