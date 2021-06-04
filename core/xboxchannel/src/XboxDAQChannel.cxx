#include "XboxDAQChannel.hxx"

#include "Rtypes.h"

#include <cstring>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

#include <sstream>
#include <numeric>
#include <algorithm>

#ifndef XBOX_NO_NAMESPACE
ClassImp(XBOX::XboxDAQChannel);
namespace XBOX {
#else
ClassImp(XboxDAQChannel);
#endif


XboxDAQChannel::XboxDAQChannel() {
	init();
}


XboxDAQChannel::~XboxDAQChannel() {
	clear();
}


void XboxDAQChannel::clear() {
	fData.clear();
	fScaleCoeffs.clear();
	fRawData.clear();
}


void XboxDAQChannel::init() {

	fXboxVersion = 0;
	fChannelName = "";

	fTimeStamp.Set(0, 0, 0, 0, 0, 0, 0, true, 0);

	fLogType = -1; // -1 - no event, 0 - BD, 1 - first pulse after BD, 2 - second pulse after BD, ...
	fPulseCount = 0;
	fDeltaF = 0;
	fLine = 0;

	fBreakdownFlag = false;
	fBreakdownType = 0;
	fBreakdownThreshDir =0;
	fBreakdownThreshDirVal = 0;
	fBreakdownRatioVal =0.;

	fStartTime.Set(0, 0, 0, 0, 0, 0, 0, true, 0);
	fStartOffset = 0.;
	fIncrement = 0.;
	fNSamples = 0;

	fXLabel = "";
	fXUnit = "";
	fYUnit = "";
	fYUnitDescription = "";

	fScaleType = -1;
	fScaleUnit = "";

	// analysis
	fXmin = -1;
	fXmax = -1;
	fXdev = -1;

	fYmin = -1;
	fYmax = -1;
	fYmean = -1;
	fYinteg = -1;
	fYspan = -1;

	// data viewing
	fAutoRefresh = true;
}

XboxDAQChannel XboxDAQChannel::cloneMetaData() const {

	XboxDAQChannel ch;

	ch.fXboxVersion = fXboxVersion;
	ch.fChannelName = fChannelName;

	ch.fTimeStamp = fTimeStamp;

	ch.fLogType = fLogType; // -1 - no event, 0 - BD, 1 - first pulse after BD, 2 - second pulse after BD, ...
	ch.fPulseCount = fPulseCount;
	ch.fDeltaF = fDeltaF;
	ch.fLine = fLine;

	ch.fBreakdownFlag = fBreakdownFlag;
	ch.fBreakdownType = fBreakdownType;
	ch.fBreakdownThreshDir = fBreakdownThreshDir;
	ch.fBreakdownThreshDirVal = fBreakdownThreshDirVal;
	ch.fBreakdownRatioVal = fBreakdownRatioVal;

	ch.fStartTime = fStartTime;
	ch.fStartOffset = fStartOffset;
	ch.fIncrement = fIncrement;
	ch.fNSamples = fNSamples;

	ch.fXLabel = fXLabel;
	ch.fXUnit = fXUnit;
	ch.fYUnit = fYUnit;
	ch.fYUnitDescription = fYUnitDescription;

	ch.fScaleType = fScaleType;
	ch.fScaleUnit = fScaleUnit;

	// analysis
	ch.fXmin = fXmin;
	ch.fXmax = fXmax;
	ch.fXdev = fXdev;

	ch.fYmin = fYmin;
	ch.fYmax = fYmax;
	ch.fYmean = fYmean;
	ch.fYinteg = fYinteg;
	ch.fYspan = fYspan;

	return ch;
}


void XboxDAQChannel::reset()
{
	clear();
	init();
}


Int_t XboxDAQChannel::isEmpty(){
	return (fNSamples == 0);
}


void XboxDAQChannel::flushbuffer() {
	fData.clear();
}


void XboxDAQChannel::viewData(vector<Double_t> &data){

	Byte_t * pRawBuffer = &fRawData[0];

	data.clear();

	if (fDataType == XboxDataType::NATIVE_INT8) {
		Char_t * pBuffer;
		pBuffer = reinterpret_cast<Char_t *>(pRawBuffer);
		data.insert(data.end(), pBuffer, pBuffer + fNSamples);
	}
	else if (fDataType == XboxDataType::NATIVE_INT16) {
		Short_t * pBuffer;
		pBuffer = reinterpret_cast<Short_t *>(pRawBuffer);
		data.insert(data.end(), pBuffer, pBuffer + fNSamples);
	}
	else if (fDataType == XboxDataType::NATIVE_DOUBLE) {
		Double_t * pBuffer;
		pBuffer = reinterpret_cast<Double_t *>(pRawBuffer);
		data.insert(data.end(), pBuffer, pBuffer + fNSamples);
	}

    // calibrate data by scale coefficients
	if(fScaleCoeffs.empty())
		return;

	if(fScaleType == 0) {  // logarithmic coefficients TODO
//		for(Double_t val: fData){
//		}
	}
	else if(fScaleType == 1) {  // polynomial coefficients
		for (auto itval = data.begin(); itval != data.end(); ++itval){
			Double_t p = 0.;   // Horner's method to evaluate polynomial
			for (auto itcoeff = fScaleCoeffs.rbegin(); itcoeff != fScaleCoeffs.rend(); ++itcoeff)
				p = p*(*itval) + (*itcoeff);
			(*itval) = p;
		}
	}

}


void XboxDAQChannel::print(){

	printf("----------------------------------------------------\n");
	printf("fXboxVersion           : %u\n", fXboxVersion);
	printf("fChannelName           : %s\n", fChannelName.c_str());

	printf("fTimestamp             : %s\n", fTimeStamp.AsString());
	printf("fLogType               : %d\n", fLogType);
	printf("fPulseCount            : %llu\n", fPulseCount);
	printf("fDeltaF                : %e\n", fDeltaF);
	printf("fLine                  : %u\n", fLine);

//	printf("fBreakdownFlagGlobal   : %u\n", fBreakdownFlagGlobal);
	printf("fBreakdownFlag         : %u\n", fBreakdownFlag);
	printf("fBreakdownType         : %u\n", fBreakdownType);
	printf("fBreakdownThreshDir    : %u\n", fBreakdownThreshDir);
	printf("fBreakdownThreshDirVal : %u\n", fBreakdownThreshDirVal);
	printf("fBreakdownRatioVal     : %e\n", fBreakdownRatioVal);

	printf("fStartTime             : %s\n", fStartTime.AsString());
	printf("fStartOffset           : %e\n", fStartOffset);
	printf("fIncrement             : %e\n", fIncrement);
	printf("fNSamples               : %u\n", fNSamples);

	printf("fXLabel                : %s\n", fXLabel.c_str());
	printf("fXUnit                 : %s\n", fXUnit.c_str());
	printf("fYUnit                 : %s\n", fYUnit.c_str());
	printf("fYUnitDescription      : %s\n", fYUnitDescription.c_str());

	// Scale type: -1 - None, 0 - logarithmic, 1 - Polynomial
	if(fScaleType == 0)
		printf("fScaleType             : Logarithmic\n");
	else if(fScaleType == 1)
		printf("fScaleType             : Polynomial\n");
	else
		printf("fScaleType             : None\n");
	printf("fScaleUnit             : %s\n", fScaleUnit.c_str());
	printf("fScaleCoeffs           :");
	for (Double_t val: fScaleCoeffs)
			printf(" %f", val);
	printf("\n");
	printf("fDataType id           : %u\n", fDataType.getId());
	printf("fDataType size         : %zu\n", fDataType.getSize());
	printf("fDataType alias        : %s\n", fDataType.getAlias().c_str());
	if(fRawData.empty())
		printf("fRawData               : EMPTY\n");
	else
		printf("fRawData               : %02X ... %02X\n", fRawData.front(),
				fRawData.back());

	printf("----------------------------------------------------\n");
}

////////////////////////////////////////////////////////////////////////
/// Comparison operator.
/// Smaller-Than-Operator refer to the time stamp.
/// \param[in] rhs Xbox DAQ channel for comparison.
/// \return    true if time stamp is smaller than of rhs.
Bool_t XboxDAQChannel::operator < (const XboxDAQChannel& rhs) const {

	return fTimeStamp < rhs.getTimeStamp();
}

////////////////////////////////////////////////////////////////////////
/// Comparison operator.
/// Larger-Than-Operator refer to the time stamp.
/// \param[in] rhs Xbox DAQ channel for comparison.
/// \return    true if time stamp is larger than of rhs.
Bool_t XboxDAQChannel::operator > (const XboxDAQChannel& rhs) const {

	return fTimeStamp > rhs.getTimeStamp();
}

//////////////////////////////////////////////////////////////////////////
///// Arithmetic operator.
///// Add two signals.
///// \param[in] rhs Xbox DAQ channel for comparison.
///// \return    XboxDAQChannel with the sum of both signals.
//XboxDAQChannel XboxDAQChannel::operator + (const XboxDAQChannel& rhs) {
//
//	XboxDAQChannel res = *this;
//
//	for (size_t i=0; i < fRawData.size(); i++)
//	res.fRawData[i] += rhs.fRawData[i];
//
//	return res;
//}


Int_t XboxDAQChannel::getIndexRange(Int_t &i0, Int_t &i1, Double_t t0, Double_t t1)
{
	if(t0 > t1){
		i0 = 0;
		i1 = fNSamples;
		return -1;
	}

	if(t0 == -1 || t0 < fStartOffset)
		i0 = 0;
	else if(t0 > fStartOffset + (fNSamples-1) * fIncrement)
		i0 = fNSamples - 1;
	else
		i0 = (t0 - fStartOffset) / fIncrement;

	if(t1 == -1 || t1 > fStartOffset + (fNSamples-1) * fIncrement)
		i1 = fNSamples;
	else if(t1 < fStartOffset)
		i1 = 1;
	else
		i1 = (t1 - fStartOffset) / fIncrement + 1;

	return 0;
}

std::vector<Double_t> XboxDAQChannel::getTimeAxis(Double_t t0, Double_t t1)
{
	if(!fData.size() || fAutoRefresh)
		viewData(fData);

	Int_t i0;
	Int_t i1;
	getIndexRange(i0, i1, t0, t1);

//	printf("nsamples %e\n", (t1 - t0) / fIncrement + 1);
//	printf("t0 %e\n", t0);
//	printf("t1 %e\n", t1);
//	printf("i0 %d\n", i0);
//	printf("i1 %d\n", i1);

	std::vector<Double_t> t(i1-i0);
	Double_t val = fStartOffset + i0*fIncrement;
	for (auto it = t.begin(); it != t.end(); ++it){
		*it = val;
		val += fIncrement;
	}
	return t;
}

void XboxDAQChannel::getTimeAxisBounds(Double_t &t0, Double_t &t1) const
{
	t0 = fStartOffset;
	t1 = fStartOffset + (fNSamples-1) * fIncrement;
}

std::vector<Double_t> XboxDAQChannel::getSignal(Double_t t0, Double_t t1)
{
	if(!fData.size() || fAutoRefresh)
		viewData(fData);

	Int_t i0;
	Int_t i1;
	getIndexRange(i0, i1, t0, t1);
	std::vector<Double_t> vect(i1-i0);
	std::copy(fData.begin() + i0, fData.begin() + i1, vect.begin());
	return vect;
}

Double_t XboxDAQChannel::min(Double_t t0, Double_t t1)
{
	if(!fData.size() || fAutoRefresh)
		viewData(fData);

	Int_t i0;
	Int_t i1;
	getIndexRange(i0, i1, t0, t1);
	Double_t val = *std::min_element(fData.begin() + i0, fData.begin() + i1);
	return val;
}

Double_t XboxDAQChannel::max(Double_t t0, Double_t t1)
{
	if(!fData.size() || fAutoRefresh)
		viewData(fData);

	Int_t i0;
	Int_t i1;
	getIndexRange(i0, i1, t0, t1);
	Double_t val = *std::max_element(fData.begin() + i0, fData.begin() + i1);
	return val;
}


Double_t XboxDAQChannel::magn(Double_t t0, Double_t t1)
{
	if(!fData.size() || fAutoRefresh)
		viewData(fData);

	Int_t i0;
	Int_t i1;
	getIndexRange(i0, i1, t0, t1);

	Double_t val = fabs(fData[i0]);
	for(Int_t i=i0+1; i < i1; i++){

		Double_t tmp = fabs(fData[i]);
		if (tmp > val)
			val = tmp;
	}
	return val;
}

Double_t XboxDAQChannel::span(Double_t t0, Double_t t1)
{
	if(!fData.size() || fAutoRefresh)
		viewData(fData);

	Int_t i0;
	Int_t i1;
	getIndexRange(i0, i1, t0, t1);

	Double_t min = fData[i0];
	Double_t max = fData[i0];
	for(Int_t i=i0+1; i < i1; i++){

		Double_t mintmp = fData[i];
		Double_t maxtmp = fData[i];
		if (mintmp < min)
			min = mintmp;
		if (maxtmp > max)
			max = maxtmp;
	}
//	Double_t min = *std::min_element(fData.begin() + i0, fData.begin() + i1);
//	Double_t max = *std::max_element(fData.begin() + i0, fData.begin() + i1);

	return fabs(max-min);
}


Double_t XboxDAQChannel::mean(Double_t t0, Double_t t1)
{
	if(!fData.size() || fAutoRefresh)
		viewData(fData);

	Int_t i0;
	Int_t i1;
	getIndexRange(i0, i1, t0, t1);
	Double_t val = std::accumulate(fData.begin() + i0, fData.begin() + i1, 0.);
	return val / (i1 - i0);
}

Double_t XboxDAQChannel::stddev(Double_t t0, Double_t t1)
{
	if(!fData.size() || fAutoRefresh)
		viewData(fData);

	Int_t i0;
	Int_t i1;
	getIndexRange(i0, i1, t0, t1);

	Double_t sum = 0; // sum
	Double_t sum2 = 0; // sum of the squares
	for(Int_t i=i0; i < i1; i++){
		sum += fData[i];
		sum2 += fData[i]*fData[i];
	}

	return sqrt((sum2 - sum*sum) / (i1-i0));
}

Double_t XboxDAQChannel::median(Double_t t0, Double_t t1)
{
	if(!fData.size() || fAutoRefresh)
		viewData(fData);

	Int_t i0;
	Int_t i1;
	getIndexRange(i0, i1, t0, t1);

	if ((i1-i0) % 2 == 0) {
	    const auto median_it1 = fData.begin() + i0 + (i1 - i0) / 2 - 1;
	    const auto median_it2 = fData.begin() + i0 + (i1 - i0) / 2;

	    std::nth_element(fData.begin() + i0, median_it1 , fData.begin() + i1);
	    const auto e1 = *median_it1;

	    std::nth_element(fData.begin() + i0, median_it2 , fData.begin() + i1);
	    const auto e2 = *median_it2;

	    return (e1 + e2) / 2;

	} else {
	    const auto median_it = fData.begin() + i0 + (i1 - i0) / 2;
	    std::nth_element(fData.begin() + i0, median_it , fData.begin() + i1);
	    return *median_it;
	}
}

Double_t XboxDAQChannel::sum(Double_t t0, Double_t t1)
{
	if(!fData.size() || fAutoRefresh)
		viewData(fData);

	Int_t i0;
	Int_t i1;
	getIndexRange(i0, i1, t0, t1);
	Double_t val = std::accumulate(fData.begin() + i0, fData.begin() + i1, 0.);
	return val;
}

Double_t XboxDAQChannel::integ(Double_t t0, Double_t t1)
{
	if(!fData.size() || fAutoRefresh)
		viewData(fData);

	Int_t i0;
	Int_t i1;
	getIndexRange(i0, i1, t0, t1);
	Double_t val = std::accumulate(fData.begin() + i0, fData.begin() + i1, 0.);
	return val * fIncrement;
}


Double_t XboxDAQChannel::risingEdge(Double_t threshold, Double_t t0, Double_t t1)
{
	if(!fData.size() || fAutoRefresh)
		viewData(fData);

	Int_t i0;
	Int_t i1;
	getIndexRange(i0, i1, t0, t1);
	Double_t val=0;

	Double_t max =  (*std::max_element(fData.begin() + i0, fData.begin() + i1));
	Double_t min =  (*std::min_element(fData.begin() + i0, fData.begin() + i1));
	Double_t threshold_abs = threshold * (max - min) + min;

	for(Int_t i=i0+1; i < i1; i++){
		if((fData[i]-threshold_abs)*(fData[i-1]-threshold_abs) < 0){
			val = double(i - (fData[i] - threshold_abs) / (fData[i] - fData[i-1])) * fIncrement;
			break;
		}
	}

//	for (auto  it = fData.begin(); it != fData.end(); ++it){
//		if(*it > thval){
//			auto i = std::distance(fData.begin(), it);
//			tr = double(i - (*it - thval) / (*it - *(it-1))) * fIncrement;
//			break;
//		}
//	}
	return val;
}


Double_t XboxDAQChannel::fallingEdge(Double_t threshold, Double_t t0, Double_t t1)
{
	if(!fData.size() || fAutoRefresh)
		viewData(fData);

	Int_t i0;
	Int_t i1;
	getIndexRange(i0, i1, t0, t1);
	Double_t val=0;

	Double_t max =  (*std::max_element(fData.begin() + i0, fData.begin() + i1));
	Double_t min =  (*std::min_element(fData.begin() + i0, fData.begin() + i1));
	Double_t threshold_abs = threshold * (max - min) + min;

	for(Int_t i=i1-2; i >= i0; i--){
		if((fData[i]-threshold_abs)*(fData[i+1]-threshold_abs) < 0){
			val = double(i + (fData[i] - threshold_abs) / (fData[i] - fData[i+1])) * fIncrement;
			break;
		}
	}
	return val;
}

//Double_t TDAQChannel::getPulseHeight(Double_t threshold)
//{
//	if(!fData.size())
//		viewData(fData);
//
//	UInt_t i0 = getPulseRisingEdgeTime(threshold) / fIncrement;
//	UInt_t i1 = getPulseFallingEdgeTime(threshold) / fIncrement + 1;
//	Double_t val = std::accumulate(fData.begin() + i0, fData.begin() + i1, 0.);
//	return val / (i1 - i0);
//}
//
//Double_t TDAQChannel::getPulseWidth(Double_t threshold)
//{
//	if(!fData.size())
//		viewData(fData);
//
//	Double_t val = getPulseFallingEdgeTime(threshold) - getPulseRisingEdgeTime(threshold);
//	return val;
//}


#ifndef XBOX_NO_NAMESPACE
}
#endif

