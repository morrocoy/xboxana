#include "XboxAnalyserView.hxx"

// root
#include "TMath.h"
#include "THStack.h"

// root graphics
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TLine.h"

#include "TGraph.h"
#include "TColor.h"
#include "TGraphErrors.h"
#include "TFrame.h"
#include "TPaletteAxis.h"

// xbox
#include "XboxAlgorithms.h"
#include "XboxSignalFilter.hxx"
#include "XboxAnalyserBreakdownRate.hxx"

#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif


///////////////////////////////////////////////////////////////////////////////
/// Normalisation.
/// Re-scale a data set to a predefined argument range.
template <typename T>
std::vector<Double_t> XboxAnalyserView::rescale(std::vector<T> &y,
		T ymin, T ymax, Bool_t blog, Double_t ysmin, Double_t ysmax) {

	std::vector<Double_t> ys(y.size(), 0.);

	if (blog) {
		ymin = TMath::Log10(ymin);
		ymax = TMath::Log10(ymax);
		for (size_t i=0; i<y.size(); i++) {

			if (y[i] <= static_cast<T>(0))
				ys[i] = -1e99;
			else
//				ys[i] = (TMath::Log10(y[i]) - ymin) / (ymax - ymin);
				ys[i] = (ysmax - ysmin) * (TMath::Log10(y[i]) - ymin) / (ymax - ymin) + ysmin;
		}
	}
	else {
		for (size_t i=0; i<y.size(); i++)
			ys[i] = (ysmax - ysmin)*(y[i] - ymin) / (ymax - ymin) + ysmin;
	}
	return ys;
}


void XboxAnalyserView::formatAxisHorz(TAxis *ax, Int_t iColor) {

	if (fHorzAxis == kTime) {

		ax->SetTimeDisplay(1);
		if ((fDateEnd-fDateBegin) > 60*60*24*30*3) {
			ax->SetTitle("time [mm/yy]");
			ax->SetTimeFormat("%m/%y%F1970-01-01 00:00:00");
		}
		else {
			ax->SetTitle("time [dd/mm/yy]");
			ax->SetTimeFormat("%d/%m/%y%F1970-01-01 00:00:00");
		}
	}
	else {
		char cbuf[30];
		if (fPCntScale != 1)
			snprintf(cbuf, 30, "pulse count [10e%.0g]", log10(fPCntScale));
		else
			snprintf(cbuf, 30, "pulse count");
		ax->SetTitle(cbuf);
	}

	ax->SetLabelFont(40); // Arial
	ax->SetTitleFont(40); // Arial

	ax->SetLabelColor(iColor);
	ax->SetTitleColor(iColor);

	ax->SetTickLength(0.015);
}

void XboxAnalyserView::formatAxisPAvg(TAxis *ax, Int_t iColor) {
	char cbuf[30];
	if (fPAvgScale != 1)
		snprintf(cbuf, 30, "average power [10e%.0g W]", log10(fPAvgScale));
	else
		snprintf(cbuf, 30, "average power [W]");
	ax->SetTitle(cbuf);

	ax->SetTitleFont(40); // Arial
	ax->SetLabelFont(40); // Arial

	ax->SetLabelColor(iColor);
	ax->SetTitleColor(iColor);

	ax->SetTickLength(0.09*fCMargin);
	ax->SetLabelOffset(0.04*fCMargin);
	ax->SetTitleOffset(5*fCMargin);
}

void XboxAnalyserView::formatAxisPLen(TAxis *ax, Int_t iColor) {
	char sbuf[30];
	if (fPLenScale != 1)
		snprintf(sbuf, 30, "pulse length [10e%.0g s]", log10(fPLenScale));
	else
		snprintf(sbuf, 30, "pulse length [s]");
	ax->SetTitle(sbuf);

	ax->SetLabelFont(40); // Arial
	ax->SetTitleFont(40); // Arial

	ax->SetLabelColor(iColor);
	ax->SetTitleColor(iColor);

	ax->SetTickLength(0.09*fCMargin);
	ax->SetLabelOffset(0.04*fCMargin);
	ax->SetTitleOffset(5*fCMargin);
}

void XboxAnalyserView::formatAxisPCnt(TAxis *ax, Int_t iColor) {
	char sbuf[30];
	if (fPCntScale != 1)
		snprintf(sbuf, 30, "pulse count [10e%.0g]", log10(fPCntScale));
	else
		snprintf(sbuf, 30, "pulse count");
	ax->SetTitle(sbuf);
	ax->SetLabelFont(40); // Arial
	ax->SetTitleFont(40); // Arial

	ax->SetLabelColor(iColor);
	ax->SetTitleColor(iColor);

	ax->SetTickLength(0.015);
	ax->SetTickLength(0.09*fCMargin);
	ax->SetLabelOffset(0.04*fCMargin);
	ax->SetTitleOffset(5*fCMargin);
}

void XboxAnalyserView::formatAxisBDPosTime(TAxis *ax, Int_t iColor) {
	char sbuf[30];
	if (fBDPosTimeScale != 1)
		snprintf(sbuf, 30, "breakdown position [10e%.0g s]", log10(fBDPosTimeScale));
	else
		snprintf(sbuf, 30, "breakdown position  [s]");
	ax->SetTitle(sbuf);

	ax->SetLabelFont(40); // Arial
	ax->SetTitleFont(40); // Arial

	ax->SetLabelColor(iColor);
	ax->SetTitleColor(iColor);

	ax->SetTickLength(0.09*fCMargin);
	ax->SetLabelOffset(0.04*fCMargin);
	ax->SetTitleOffset(5*fCMargin);
}


///////////////////////////////////////////////////////////////////////////////
/// draw horizontal axis.
void XboxAnalyserView::drawAxisHorz(Int_t iColor, const std::string &sOpt, Int_t ndiv) {

	Double_t xmin = 0.;
	Double_t ymin = 0.;
	Double_t xmax = 1.;
	Double_t ymax = 0.;

	std::string opt = sOpt;
	if (fHorzAxis == kTime) {
		opt = opt + "t";
		xmin = fDateBegin;
		xmax = fDateEnd;
	}
	else {
		xmin = fPCntMin / fPCntScale;
		xmax = fPCntMax / fPCntScale;
	}
	TGaxis *ax = new TGaxis(xmin, ymin, xmax, ymax, xmin, xmax, ndiv, opt.c_str());

	if (fHorzAxis == kTime) {

		if ((fDateEnd-fDateBegin) > 60*60*24*30*3) {
			ax->SetTitle("time [mm/yy]");
			ax->SetTimeFormat("%m/%y%F1970-01-01 00:00:00");
		}
		else {
			ax->SetTitle("time [dd/mm/yy]");
			ax->SetTimeFormat("%d/%m/%y%F1970-01-01 00:00:00");
		}
	}
	else {
		char cbuf[30];
		if (fPCntScale != 1)
			snprintf(cbuf, 30, "pulse count [10e%.0g]", log10(fPCntScale));
		else
			snprintf(cbuf, 30, "pulse count");
		ax->SetTitle(cbuf);
	}

	ax->SetLabelFont(40); // Arial
	ax->SetTitleFont(40); // Arial

	ax->SetLineColor(iColor);
	ax->SetLabelColor(iColor);
	ax->SetTitleColor(iColor);

	ax->Draw();
	fBufferAxis.push_back(ax);
}

///////////////////////////////////////////////////////////////////////////////
/// Axis for pulse average power.
/// Vertical axis on the left side.
void XboxAnalyserView::drawAxisPAvg(Int_t iColor, const std::string &sOpt, Int_t ndiv) {

	Double_t xmin = 0.;
	Double_t ymin = 0.;
	Double_t xmax = 0.;
	Double_t ymax = 1.;

	Double_t wmin = fPAvgMin / fPAvgScale;
	Double_t wmax = fPAvgMax / fPAvgScale;

	if (fHorzAxis == kTime) {
		xmin = fDateBegin;
		xmax = fDateEnd;
	}
	else {
		xmin = fPCntMin / fPCntScale;
		xmax = fPCntMax / fPCntScale;
	}
	TGaxis *ax = new TGaxis(xmin, ymin, xmin, ymax, wmin, wmax, ndiv, sOpt.c_str());

	char sbuf[30];
	if (fPAvgScale != 1)
		snprintf(sbuf, 30, "average power [10e%.0g W]", log10(fPAvgScale));
	else
		snprintf(sbuf, 30, "average power [W]");
	ax->SetTitle(sbuf);

	ax->SetTitleFont(40); // Arial
	ax->SetLabelFont(40); // Arial

	ax->SetLineColor(iColor);
	ax->SetLabelColor(iColor);
	ax->SetTitleColor(iColor);

	ax->SetTickLength(0.09*fCMargin);
	ax->SetLabelOffset(0.04*fCMargin);
	ax->SetTitleOffset(5*fCMargin);

	ax->Draw();
	fBufferAxis.push_back(ax);
}

///////////////////////////////////////////////////////////////////////////////
/// Axis for pulse length.
/// Vertical axis on the left side.
void XboxAnalyserView::drawAxisPLen(Int_t iColor, const std::string &sOpt, Int_t ndiv) {

	Double_t xmin = 0.;
	Double_t ymin = 0.;
	Double_t xmax = 0.;
	Double_t ymax = 1.;

	Double_t wmin = fPLenMin / fPLenScale;
	Double_t wmax = fPLenMax / fPLenScale;

	if (fHorzAxis == kTime) {
		xmin = fDateBegin;
		xmax = fDateEnd;
	}
	else {
		xmin = fPCntMin / fPCntScale;
		xmax = fPCntMax / fPCntScale;
	}

	xmin = xmin - (xmax-xmin)*0.06;
	TGaxis *ax = new TGaxis(xmin, ymin, xmin, ymax, wmin, wmax, ndiv, sOpt.c_str());

	char sbuf[30];
	if (fPLenScale != 1)
		snprintf(sbuf, 30, "pulse length [10e%.0g s]", log10(fPLenScale));
	else
		snprintf(sbuf, 30, "pulse length [s]");
	ax->SetTitle(sbuf);

	ax->SetTitleFont(40); // Arial
	ax->SetLabelFont(40); // Arial

	ax->SetLineColor(iColor);
	ax->SetLabelColor(iColor);
	ax->SetTitleColor(iColor);

	ax->SetTickLength(0.09*fCMargin);
	ax->SetLabelOffset(0.04*fCMargin);
	ax->SetTitleOffset(5*fCMargin);

	ax->Draw();
	fBufferAxis.push_back(ax);

}

///////////////////////////////////////////////////////////////////////////////
/// Axis for pulse count.
/// Vertical axis on the right side.
void XboxAnalyserView::drawAxisPCnt(Int_t iColor, const std::string &sOpt, Int_t ndiv) {

	Double_t xmin = 0.;
	Double_t ymin = 0.;
	Double_t xmax = 0.;
	Double_t ymax = 1.;

	Double_t wmin = fPCntMin / fPCntScale;
	Double_t wmax = fPCntMax / fPCntScale;

	if (fHorzAxis == kTime) {
		xmin = fDateBegin;
		xmax = fDateEnd;
	}
	else {
		xmin = fPCntMin / fPCntScale;
		xmax = fPCntMax / fPCntScale;
	}

	xmax = xmax + (xmax-xmin)*0.06;
	TGaxis *ax = new TGaxis(xmax, ymin, xmax, ymax, wmin, wmax, ndiv, sOpt.c_str());

	char sbuf[30];
	if (fPCntScale != 1)
		snprintf(sbuf, 30, "pulse count [10e%.0g]", log10(fPCntScale));
	else
		snprintf(sbuf, 30, "pulse count");
	ax->SetTitle(sbuf);

	ax->SetTitleFont(40); // Arial
	ax->SetLabelFont(40); // Arial

	ax->SetLineColor(iColor);
	ax->SetLabelColor(iColor);
	ax->SetTitleColor(iColor);

	ax->SetTickLength(0.015);
	ax->SetTickLength(0.09*fCMargin);
	ax->SetLabelOffset(0.04*fCMargin);
	ax->SetTitleOffset(5*fCMargin);

	ax->Draw();
	fBufferAxis.push_back(ax);

}

///////////////////////////////////////////////////////////////////////////////
/// Axis for breakdown rate.
/// Vertical axis on the right side (logarithmic).
void XboxAnalyserView::drawAxisRate(Int_t iColor, const std::string &sOpt, Int_t ndiv) {

	Double_t xmin = 0.;
	Double_t ymin = 0.;
	Double_t xmax = 0.;
	Double_t ymax = 1.;

	Double_t wmin = fRateMin / fRateScale;
	Double_t wmax = fRateMax / fRateScale;

	if (fHorzAxis == kTime) {
		xmin = fDateBegin;
		xmax = fDateEnd;
	}
	else {
		xmin = fPCntMin / fPCntScale;
		xmax = fPCntMax / fPCntScale;
	}
	TGaxis *ax = new TGaxis(xmax, ymin, xmax, ymax, wmin, wmax, ndiv, sOpt.c_str());

	char sbuf[30];
	if (fPCntScale != 1)
		snprintf(sbuf, 30, "breakdown rate [10e%.0g bpp]", log10(fRateScale));
	else
		snprintf(sbuf, 30, "breakdown rate [bpp]");
	ax->SetTitle(sbuf);

	ax->SetTitleFont(40); // Arial
	ax->SetLabelFont(40); // Arial

	ax->SetLineColor(iColor);
	ax->SetLabelColor(iColor);
	ax->SetTitleColor(iColor);

	ax->SetTickLength(0.09*fCMargin);
	ax->SetTitleOffset(5*fCMargin);
	// for some reason the label offset must be 0 if logarithmic scale
	ax->SetLabelOffset(-0.01*fCMargin);

	ax->Draw();
	fBufferAxis.push_back(ax);
}



///////////////////////////////////////////////////////////////////////////////
/// Draw average power of breakdown events.
void XboxAnalyserView::plotPAvg(const std::string &sKey, Int_t iColor,
		const std::string &sOpt, Style_t iMarkerStyle, size_t iMarkerSize) {

	fCategoryChannel[sKey].size();
	size_t nEvents = fCategoryChannel[sKey].size();

	std::vector<Double_t> vPAvg(nEvents);
	std::vector<Double_t> vQtyX(nEvents);
	std::vector<Double_t> vQtyY(nEvents);

	Double_t xmin = 0.;
	Double_t xmax = 1.;
	Double_t ymin = 0.;
	Double_t ymax = 1.;

	// check whether axes will be hidden
	Bool_t bHideAxes = false;
	for (auto &c: sOpt) {
		if (std::toupper(c) == 'A') {
			bHideAxes=true;
			break;
		}
	}

	// limits and values of the horizontal axis (vQtyX)
	if (fHorzAxis == kTime) {
		xmin = (Double_t)fDateBegin;
		xmax = (Double_t)fDateEnd;
		for (size_t i=0; i<nEvents; i++)
			vQtyX[i] = (Double_t)fCategoryChannel[sKey][i].getTimeStamp();
	}
	else {
		xmin = (Double_t)fPCntMin / fPCntScale;
		xmax = (Double_t)fPCntMax / fPCntScale;
		for (size_t i=0; i<nEvents; i++)
			vQtyX[i] = (Double_t)fCategoryChannel[sKey][i].getPulseCount() / fPCntScale;
	}

	// limits and values of the vertical axis (vQtyY)
	if (bHideAxes) {
		ymin = 0.;
		ymax = 1.;
	}
	else {
		ymin = fPAvgMin / fPAvgScale;
		ymax = fPAvgMax / fPAvgScale;
	}

	for (size_t i=0; i<nEvents; i++)
		vPAvg[i] = fCategoryChannel[sKey][i].getYmean();
	vQtyY = rescale(vPAvg, fPAvgMin, fPAvgMax, false, ymin, ymax);

	// fill histogram
	TH2D *h = new TH2D("","", fXBins, xmin, xmax, fYBins, ymin, ymax);
	for (size_t i=0; i<nEvents; i++)
		h->Fill(vQtyX[i], vQtyY[i]);

	h->SetMarkerStyle(iMarkerStyle);
	h->SetMarkerSize(iMarkerSize);
	h->SetMarkerColor(iColor);
	h->SetStats(false);
	h->Draw(sOpt.c_str());

	// format axes if shown
	if(!bHideAxes) {
		TAxis *ax = h->GetXaxis();
		TAxis *ay = h->GetYaxis();
		formatAxisHorz(ax);
		formatAxisPAvg(ay);
	}
	fBufferHist.push_back(h);
}


///////////////////////////////////////////////////////////////////////////////
/// Draw pulse length of breakdown events.
void XboxAnalyserView::plotPLen(const std::string &sKey, Int_t iColor,
		const std::string &sOpt, Style_t iMarkerStyle, size_t iMarkerSize) {

	fCategoryChannel[sKey].size();
	size_t nEvents = fCategoryChannel[sKey].size();

	std::vector<Double_t> vPLen(nEvents);
	std::vector<Double_t> vQtyX(nEvents);
	std::vector<Double_t> vQtyY(nEvents);

	Double_t xmin = 0.;
	Double_t xmax = 1.;
	Double_t ymin = 0.;
	Double_t ymax = 1.;

	// check whether axes will be hidden
	Bool_t bHideAxes = false;
	for (auto &c: sOpt) {
		if (std::toupper(c) == 'A') {
			bHideAxes=true;
			break;
		}
	}

//	printf("begin: %s\n", fDateBegin.AsString());
//	printf("end: %s\n", fDateEnd.AsString());

	// limits and values of the horizontal axis (vQtyX)
	if (fHorzAxis == kTime) {
		xmin = (Double_t)fDateBegin;
		xmax = (Double_t)fDateEnd;
		for (size_t i=0; i<nEvents; i++)
			vQtyX[i] = (Double_t)fCategoryChannel[sKey][i].getTimeStamp();

		printf("begin: %s\n", fCategoryChannel[sKey].front().getTimeStamp().AsString());
		printf("end: %s\n", fCategoryChannel[sKey].back().getTimeStamp().AsString());
	}
	else {
		xmin = (Double_t)fPCntMin / fPCntScale;
		xmax = (Double_t)fPCntMax / fPCntScale;
		for (size_t i=0; i<nEvents; i++)
			vQtyX[i] = (Double_t)fCategoryChannel[sKey][i].getPulseCount() / fPCntScale;
	}

	// limits and values of the vertical axis (vQtyY)
	if (bHideAxes) {
		ymin = 0.;
		ymax = 1.;
	}
	else {
		ymin = fPLenMin / fPLenScale;
		ymax = fPLenMax / fPLenScale;
	}

	for (size_t i=0; i<nEvents; i++)
		vPLen[i] = fCategoryChannel[sKey][i].getXmax() - fCategoryChannel[sKey][i].getXmin();
	vQtyY = rescale(vPLen, fPLenMin, fPLenMax, false, ymin, ymax);

	// fill histogram
	TH2D *h = new TH2D("","", fXBins, xmin, xmax, fYBins, ymin, ymax);
	for (size_t i=0; i<nEvents; i++)
		h->Fill(vQtyX[i], vQtyY[i]);

	h->SetMarkerStyle(iMarkerStyle);
	h->SetMarkerSize(iMarkerSize);
	h->SetMarkerColor(iColor);
	h->SetStats(false);
	h->Draw(sOpt.c_str());

	// format axes if shown
	if(!bHideAxes) {
		TAxis *ax = h->GetXaxis();
		TAxis *ay = h->GetYaxis();
		formatAxisHorz(ax);
		formatAxisPLen(ay);
	}
	fBufferHist.push_back(h);
}


///////////////////////////////////////////////////////////////////////////////
/// Draw pulse count of breakdown events.
void XboxAnalyserView::plotPCnt(const std::string &sKey, Int_t iColor,
		const std::string &sOpt, Style_t iMarkerStyle, size_t iMarkerSize) {

	fCategoryChannel[sKey].size();
	size_t nEvents = fCategoryChannel[sKey].size();

	std::vector<ULong64_t> vPCnt(nEvents);
	std::vector<Double_t> vQtyX(nEvents);
	std::vector<Double_t> vQtyY(nEvents);

	Double_t xmin = 0.;
	Double_t xmax = 1.;
	Double_t ymin = 0.;
	Double_t ymax = 1.;

	// check whether axes will be hidden
	Bool_t bHideAxes = false;
	for (auto &c: sOpt) {
		if (std::toupper(c) == 'A') {
			bHideAxes=true;
			break;
		}
	}

	// limits and values of the horizontal axis (vQtyX)
	if (fHorzAxis == kTime) {
		xmin = (Double_t)fDateBegin;
		xmax = (Double_t)fDateEnd;
		for (size_t i=0; i<nEvents; i++)
			vQtyX[i] = (Double_t)fCategoryChannel[sKey][i].getTimeStamp();
	}
	else {
		xmin = (Double_t)fPCntMin / fPCntScale;
		xmax = (Double_t)fPCntMax / fPCntScale;
		for (size_t i=0; i<nEvents; i++)
			vQtyX[i] = (Double_t)fCategoryChannel[sKey][i].getPulseCount() / fPCntScale;
	}

	// limits and values of the vertical axis (vQtyY)
	if (bHideAxes) {
		ymin = 0.;
		ymax = 1.;
	}
	else {
		ymin = fPCntMin / fPCntScale;
		ymax = fPCntMax / fPCntScale;
	}
	for (size_t i=0; i<nEvents; i++)
		vPCnt[i] = fCategoryChannel[sKey][i].getPulseCount();
	vQtyY = rescale(vPCnt, fPCntMin, fPCntMax, false, ymin, ymax);

	// fill histogram
	TH2D *h = new TH2D("","", fXBins, xmin, xmax, fYBins, ymin, ymax);
	for (size_t i=0; i<nEvents; i++)
		h->Fill(vQtyX[i], vQtyY[i]);

	h->SetMarkerStyle(iMarkerStyle);
	h->SetMarkerSize(iMarkerSize);
	h->SetMarkerColor(iColor);
	h->SetStats(false);
	h->Draw(sOpt.c_str());

	// format axes if shown
	if(!bHideAxes) {
		TAxis *ax = h->GetXaxis();
		TAxis *ay = h->GetYaxis();
		formatAxisHorz(ax);
		formatAxisPCnt(ay);
	}
	fBufferHist.push_back(h);
}

///////////////////////////////////////////////////////////////////////////////
/// Draw breakdown rate.
void XboxAnalyserView::plotRate(const std::string &sKey, Int_t iColor,
		const std::string &sOpt, Style_t iMarkerStyle, size_t iMarkerSize) {

	fCategoryChannel[sKey].size();
	size_t nEvents = fCategoryChannel[sKey].size();

	std::vector<TTimeStamp> vTime(nEvents, 0.);
	std::vector<ULong64_t> vPCnt(nEvents, 0.);

	std::vector<TTimeStamp> vTimeS(fNSamples, 0.); // resampled time axis
	std::vector<ULong64_t> vPCntS(fNSamples, 0.); // resampled pulse count axis
	std::vector<Double_t> vRateS(fNSamples, 0.); // resampled breakdown rate

	std::vector<Double_t> vQtyX(fNSamples, 0.);
	std::vector<Double_t> vQtyY(fNSamples, 0.);

	Double_t xmin = 0.;
	Double_t xmax = 1.;
	Double_t ymin = 0.;
	Double_t ymax = 1.;

	// check whether axes will be hidden
	Bool_t bHideAxes = false;
	for (auto &c: sOpt) {
		if (std::toupper(c) == 'A') {
			bHideAxes=true;
			break;
		}
	}

	// TGraph uses A option clear the pad, not to hide axis. Therefore remove option.
	std::string opt = sOpt;
	opt.erase(std::remove(opt.begin(), opt.end(), 'a'), opt.end());
	opt.erase(std::remove(opt.begin(), opt.end(), 'A'), opt.end());

	// evaluate breakdown rate
	for (size_t i=0; i<nEvents; i++) {
		vTime[i] = fCategoryChannel[sKey][i].getTimeStamp();
		vPCnt[i] = fCategoryChannel[sKey][i].getPulseCount();
	}
	XBOX::XboxAnalyserBreakdownRate bdr(vPCnt);
	bdr.setWeightFunction(XBOX::XboxAnalyserBreakdownRate::kGaussian, 0.02, 3);
	bdr.setSamples(fNSamples);
	bdr.fromMovingWindow(vPCntS, vRateS);

	// search for nearest time
	for(size_t i=0; i<fNSamples; i++) {

		for (size_t j=1; j<nEvents; j++) {
			if (vPCnt[j-1] < vPCntS[i] && vPCnt[j] >= vPCntS[i]) {
				vTimeS[i] = vTime[j];
				break;
			}
		}
	}

	// limits and values of the horizontal axis (vQtyX)
	if (fHorzAxis == kTime) {
		xmin = (Double_t)fDateBegin;
		xmax = (Double_t)fDateEnd;
		vQtyX = rescale(vTimeS, fDateBegin, fDateEnd, false, xmin, xmax);
	}
	else {
		xmin = (Double_t)fPCntMin / fPCntScale;
		xmax = (Double_t)fPCntMax / fPCntScale;
		vQtyX = rescale(vPCntS, fPCntMin, fPCntMax, false, xmin, xmax);
	}

	// limits and values of the vertical axis (vQtyY)
	if (bHideAxes) { // enable logarithmic scale if normalisation applies
		ymin = 0.;
		ymax = 1.;
		vQtyY = rescale(vRateS, fRateMin, fRateMax, true, ymin, ymax);
	}
	else {
		ymin = fRateMin / fRateScale;
		ymax = fRateMax / fRateScale;
		vQtyY = rescale(vRateS, fRateMin, fRateMax, false, ymin, ymax);
	}


	// fill histogram
//	TH2D *h = new TH2D("","", fXBins, xmin, xmax, fYBins, ymin, ymax);
//	for (size_t i=0; i<vQtyX.size(); i++)
//		h->Fill(vQtyX[i], vQtyY[i]);
//
//	h->SetMarkerStyle(iMarkerStyle);
//	h->SetMarkerSize(iMarkerSize);
//	h->SetMarkerColor(iMarkerColor);
//	h->SetStats(false);
////	h->Draw(sOpt.c_str());
//
//	// format axes if shown
//	if(!bHideAxes) {
//		TAxis *ax = h->GetXaxis();
//		TAxis *ay = h->GetYaxis();
//		formatAxisHorz(ax);
//		formatAxisPCnt(ay);
//	}
//	fBufferHist.push_back(h);


	// create graph
	TGraph *gr = new TGraph(vQtyX.size(), &vQtyX[0], &vQtyY[0]);

	gr->SetMarkerStyle(iMarkerStyle);
	gr->SetMarkerSize(iMarkerSize);
	gr->SetMarkerColor(iColor);
	gr->SetLineColor(iColor);
	gr->SetLineWidth(2);

	gr->SetMinimum(ymin);
	gr->SetMaximum(ymax);
	gr->Draw(opt.c_str());

	TAxis *ax = gr->GetXaxis();
	TAxis *ay = gr->GetYaxis();

	ax->SetLimits(xmin, xmax);
	ay->SetLimits(ymin, ymax);

	// format axes if shown
	if(!bHideAxes) {
		formatAxisHorz(ax);
		formatAxisPCnt(ay);
	}
	else {
		ax->SetTickLength(0);
		ax->SetLabelOffset(999);
		ay->SetTickLength(0);
		ay->SetLabelOffset(999);
	}

	fBufferGraph.push_back(gr);


}


void XboxAnalyserView::plotBDPosTime(const std::string &sKey,
		Int_t iColor, const std::string &sOpt, Style_t iMarkerStyle, size_t iMarkerSize) {

	size_t nEvents = fCategoryTuple[sKey].size();

	std::vector<Double_t> vBDPosTime(nEvents);
	std::vector<Double_t> vQtyX(nEvents);
	std::vector<Double_t> vQtyY(nEvents);

	Double_t xmin = 0.;
	Double_t xmax = 1.;
	Double_t ymin = 0.;
	Double_t ymax = 1.;

	// check whether axes will be hidden
	Bool_t bHideAxes = false;
	for (auto &c: sOpt) {
		if (std::toupper(c) == 'A') {
			bHideAxes=true;
			break;
		}
	}

	// limits and values of the horizontal axis (vQtyX)
	if (fHorzAxis == kTime) {
		xmin = (Double_t)fDateBegin;
		xmax = (Double_t)fDateEnd;
		std::string category = sKey.substr(0, sKey.find(".")) + ".TimeStamp";
		for (size_t i=0; i<nEvents; i++)
			vQtyX[i] = (Double_t)fCategoryTimeStamp[category][i];
	}
	else {
		xmin = (Double_t)fPCntMin / fPCntScale;
		xmax = (Double_t)fPCntMax / fPCntScale;
		std::string category = sKey.substr(0, sKey.find(".")) + ".PulseCount";
		for (size_t i=0; i<nEvents; i++)
			vQtyX[i] = (Double_t)fCategoryPulseCount[category][i] / fPCntScale;
	}

	// limits and values of the vertical axis (vQtyY)
	if (bHideAxes) {
		ymin = 0.;
		ymax = 1.;
	}
	else {
		ymin = fBDPosTimeMin / fBDPosTimeScale;
		ymax = fBDPosTimeMax / fBDPosTimeScale;
	}

	// get the breakdown position time
	vBDPosTime = fCategoryTuple[sKey];
	vQtyY = rescale(vBDPosTime, fBDPosTimeMin, fBDPosTimeMax, false, ymin, ymax);

	// fill histogram
	TH2D *h = new TH2D("","", fXBins, xmin, xmax, fYBins, ymin, ymax);
	for (size_t i=0; i<nEvents; i++)
		h->Fill(vQtyX[i], vQtyY[i]);

	h->SetMarkerStyle(iMarkerStyle);
	h->SetMarkerSize(iMarkerSize);
	h->SetMarkerColor(iColor);
	h->SetStats(false);
	h->Draw(sOpt.c_str());

	// format axes if shown
	if(!bHideAxes) {
		TAxis *ax = h->GetXaxis();
		TAxis *ay = h->GetYaxis();
		formatAxisHorz(ax);
		formatAxisBDPosTime(ay);
	}
	fBufferHist.push_back(h);
}

//void XboxAnalyserView::drawBreakdownTime() {
//
//	fPad1->Clear();
//	fPad2->Clear();
//
//	fPad1->Modified();
//	fPad1->cd();
//
//
//	drawTPosBreakdown("CONT4Z");
//
//	fPad2->Modified();
//	fPad2->cd();
////	fHTposBreakdown->Draw("A");
//	drawTPosBreakdown("CONT4Z");
//
//	// plot axes
////	fPad1->cd();
//
//	fCanvas->Update(); // draws the frame, after which one can change it
//	fCanvas->Modified();
//}

///////////////////////////////////////////////////////////////////////////////
/// Plotting function.
/// Plots two subsequent signals with markers for the rising edge of the
/// pulse and the moment of the breakdown.
/// The first is evaluated by means of the previous pulse. The latter is
/// derived from the moment where current and previous signal start to
/// deviate from each other. Both involve the use of thresholds.
/// \param[in] ch0 the Xbox DAQ channel containing the current signal.
/// \param[in] lowerlimit the lower limit of the considered time window.
/// \param[in] upperlimit the upper limit of the considered time window.
/// \return The time delay between rising edge and the moment of the breakdown.
void XboxAnalyserView::plotSignal(const std::string &sKey, Int_t idx,
		Int_t iColor, const std::string &sOpt,
		Style_t iMarkerStyle, size_t iMarkerSize,
		Double_t xmin, Double_t xmax, Double_t ymin, Double_t ymax) {

	size_t n = fCategoryChannel[sKey].size();
	if (idx >= n)
		return;

	XBOX::XboxDAQChannel ch = fCategoryChannel[sKey][idx];
	Double_t jitter = ch .getStartOffset();
	if (jitter == -1)
		ch.setStartOffset(0);

	Double_t lbnd=0;
	Double_t ubnd=0;
	ch.getTimeAxisBounds(lbnd, ubnd);
	if(xmin < lbnd)
		xmin = lbnd;
	if(xmax > lbnd)
		xmax = ubnd;

	// check whether axes will be hidden
	Bool_t bHideAxes = false;
	for (auto &c: sOpt) {
		if (std::toupper(c) == 'A') {
			bHideAxes=true;
			break;
		}
	}

	// read data over the entire argument range of the channels
	XBOX::XboxSignalFilter filter;
	std::vector<Double_t> x = XBOX::linspace(xmin, xmax, fNSamples);
	std::vector<Double_t> y = filter(ch, x);

//	printf("%e, %e\n", xmin, xmax);
//	printf("%e, %e\n", y.front(), y.back());

	// general parameters
	std::string name = ch.getChannelName();
	TTimeStamp ts = ch.getTimeStamp();
	ULong64_t count = ch.getPulseCount();

	char stitle[200];
	snprintf (stitle, 200, "%s %s PulseCount: %llu",
			name.c_str(), ts.AsString(), count);

	TGraph *gr = new TGraph(x.size(), &x[0], &y[0]);

	gr->SetMarkerStyle(iMarkerStyle);
	gr->SetMarkerSize(iMarkerSize);
	gr->SetMarkerColor(iColor);
	gr->SetLineColor(iColor);
	gr->SetLineWidth(2);

	gr->SetTitle(stitle);
	gr->Draw(sOpt.c_str());

	TAxis *ax = gr->GetXaxis();
	TAxis *ay = gr->GetYaxis();

	ax->SetLimits(xmin, xmax);
	if (ymin != -1e99 && ymax != 1e99)
		ay->SetLimits(ymin, ymax);
	ax->SetTitle("Time [s]");
	ay->SetTitle("Amplitude");

	// legend
	fLegend->Clear();
	fLegend->AddEntry(gr, sKey.c_str(), "l");
	fLegend->SetBorderSize(1);
	fLegend->SetFillColor(0);
	fLegend->SetTextSize(0);
	fLegend->Draw();

	fBufferGraph.push_back(gr);
}

///////////////////////////////////////////////////////////////////////////////
/// Plotting function.
/// Plots two subsequent signals with markers for the rising edge of the
/// pulse and the moment of the breakdown.
/// The first is evaluated by means of the previous pulse. The latter is
/// derived from the moment where current and previous signal start to
/// deviate from each other. Both involve the use of thresholds.
/// \param[in] ch0 the Xbox DAQ channel containing the current signal.
/// \param[in] lowerlimit the lower limit of the considered time window.
/// \param[in] upperlimit the upper limit of the considered time window.
/// \return The time delay between rising edge and the moment of the breakdown.
void XboxAnalyserView::plotSignal(const std::string &sKey1, const std::string &sKey2,
		Int_t idx,
		Int_t iColor, const std::string &sOpt,
		Style_t iMarkerStyle, size_t iMarkerSize,
		Double_t xmin, Double_t xmax, Double_t ymin, Double_t ymax) {

	size_t n1 = fCategoryChannel[sKey1].size();
	size_t n2 = fCategoryChannel[sKey2].size();
	if (idx >= n1 || idx >= n2)
		return;

	XBOX::XboxDAQChannel ch1 = fCategoryChannel[sKey1][idx];
	XBOX::XboxDAQChannel ch2 = fCategoryChannel[sKey2][idx];
	Double_t jitter1 = ch1 .getStartOffset();
	Double_t jitter2 = ch2 .getStartOffset();
	if (jitter1 == -1)
		ch1.setStartOffset(0);
	if (jitter2 == -1)
		ch2.setStartOffset(0);

	Double_t lbnd=0;
	Double_t ubnd=0;
	ch1.getTimeAxisBounds(lbnd, ubnd);
	if(xmin < lbnd)
		xmin = lbnd;
	if(xmax > lbnd)
		xmax = ubnd;

	// check whether axes will be hidden
	Bool_t bHideAxes = false;
	for (auto &c: sOpt) {
		if (std::toupper(c) == 'A') {
			bHideAxes=true;
			break;
		}
	}

	// read data over the entire argument range of the channels
	XBOX::XboxSignalFilter filter;
	std::vector<Double_t> x = XBOX::linspace(xmin, xmax, fNSamples);
	std::vector<Double_t> y1 = filter(ch1, x);
	std::vector<Double_t> y2 = filter(ch2, x);

//	printf("%e, %e\n", xmin, xmax);
//	printf("%e, %e\n", y1.front(), y1.back());

	// general parameters
	std::string name = ch1.getChannelName();
	TTimeStamp ts = ch1.getTimeStamp();
	ULong64_t count = ch1.getPulseCount();

	char stitle[200];
	snprintf (stitle, 200, "%s %s PulseCount: %llu",
			name.c_str(), ts.AsString(), count);

	TGraph *gr1 = new TGraph(x.size(), &x[0], &y1[0]);
	TGraph *gr2 = new TGraph(x.size(), &x[0], &y2[0]);

	gr1->SetMarkerStyle(iMarkerStyle);
	gr1->SetMarkerSize(iMarkerSize);
	gr1->SetMarkerColor(iColor);
	gr1->SetLineColor(iColor);
	gr1->SetLineWidth(2);

	gr2->SetMarkerStyle(iMarkerStyle);
	gr2->SetMarkerSize(iMarkerSize);
	gr2->SetMarkerColor(iColor-3);
	gr2->SetLineColor(iColor-3);
	gr2->SetLineWidth(2);

	gr1->SetTitle(stitle);
	gr1->Draw(sOpt.c_str());
	gr2->Draw((sOpt + "same").c_str());
//	mg->Draw("ACP");

	TAxis *ax = gr1->GetXaxis();
	TAxis *ay = gr1->GetYaxis();

	ax->SetLimits(xmin, xmax);
	if (ymin != -1e99 && ymax != 1e99)
		ay->SetLimits(ymin, ymax);
	ax->SetTitle("Time [s]");
	ay->SetTitle("Amplitude");

	TLine *l1 = new TLine(ch1.getXmin(), ay->GetXmin(), ch1.getXmin(), ay->GetXmax());
	l1->SetLineWidth(1);
	l1->SetLineColor(kBlack);
	l1->Draw();
	l1->SetNDC(false);

	TLine *l2 = new TLine(ch2.getXmin(), ay->GetXmin(), ch2.getXmin(), ay->GetXmax());
	l2->SetLineWidth(1);
	l2->SetLineColor(kBlack);
	l2->Draw();
	l2->SetNDC(false);

	TLine *l3 = new TLine(ch1.getXdev(), ay->GetXmin(), ch1.getXdev(), ay->GetXmax());
	l3->SetLineWidth(1);
	l3->SetLineColor(kRed);
	l3->Draw();
	l3->SetNDC(false);

	printf("xmin: %e | xmin: %e | xdev: %e\n", ch1.getXmin(), ch2.getXmin(), ch1.getXdev());
	// legend
	fLegend->Clear();
	fLegend->AddEntry(gr1, sKey1.c_str(), "l");
	fLegend->AddEntry(gr2, sKey2.c_str(), "l");
	fLegend->SetBorderSize(1);
	fLegend->SetFillColor(0);
	fLegend->SetTextSize(0);
	fLegend->Draw();

	fBufferGraph.push_back(gr1);
	fBufferGraph.push_back(gr2);
	fBufferLine.push_back(l1);
	fBufferLine.push_back(l2);
	fBufferLine.push_back(l3);
}


///////////////////////////////////////////////////////////////////////////////
/// Plotting function.
/// Plots two subsequent signals with markers for the rising edge of the
/// pulse and the moment of the breakdown.
/// The first is evaluated by means of the previous pulse. The latter is
/// derived from the moment where current and previous signal start to
/// deviate from each other. Both involve the use of thresholds.
/// \param[in] ch0 the Xbox DAQ channel containing the current signal.
/// \param[in] lowerlimit the lower limit of the considered time window.
/// \param[in] upperlimit the upper limit of the considered time window.
/// \return The time delay between rising edge and the moment of the breakdown.
void XboxAnalyserView::plotProbes() {

//	size_t n = fBreakdownResults.size();
//	Int_t nsamples = 1024; //fEntryList[0].getProbeTranRisingEdge().size();
//
//	printf("nsamples: %d\n", nsamples);
//
//
//	// general parameters
//
//	TMultiGraph *mg1 = new TMultiGraph();
//	TMultiGraph *mg2 = new TMultiGraph();
//	TMultiGraph *mg3 = new TMultiGraph();
//	TMultiGraph *mg4 = new TMultiGraph();
//
//	std::vector<TGraph*> gr1(n);
//	std::vector<TGraph*> gr2(n);
//	std::vector<TGraph*> gr3(n);
//	std::vector<TGraph*> gr4(n);
//
//	Double_t twin = 1e-6;
//	std::vector<Double_t> x = XBOX::linspace(-twin, twin, nsamples);
//	for (size_t i=0; i<n; i++) {
//
//		TTimeStamp ts = fBreakdownResults[i].getTimeStamp();
//		if (ts < fDateBegin || ts > fDateEnd)
//			continue;
//
////		Int_t i = 1;
////		printf("Jitter = %e | nsamples: %d\n", fEntryList[i].getJitter(), fEntryList[i].getProbeTranRisingEdge().size());
//
//		std::vector<Double_t> y1 = fBreakdownResults[i].getProbeTranRisingEdge();
//		std::vector<Double_t> y2 = fBreakdownResults[i].getProbeTranBreakdownTime();
//		std::vector<Double_t> y3 = fBreakdownResults[i].getProbeReflRisingEdge();
//		std::vector<Double_t> y4 = fBreakdownResults[i].getProbeReflBreakdownTime();
//
//		if (y1.size() == nsamples) {
//			gr1[i] = new TGraph(nsamples, &x[0], &y1[0]);
////			gr1[i]->SetLineColor(kRed);
////			gr1[i]->SetLineWidth(1);
////			gr1[i]->SetMarkerColor(kRed);
////			gr1[i]->SetMarkerStyle(20);
////			gr1[i]->SetMarkerSize(1);
//
//			if (y1[400] > 1e6) {
//				printf("++++Out of Mask: %s\n" , fBreakdownResults[i].getTimeStamp().AsString());
//			}
//
//			mg1->Add(gr1[i]);
//		}
//
//		if (y2.size() == nsamples) {
//			gr2[i] = new TGraph(nsamples, &x[0], &y2[0]);
//			mg2->Add(gr2[i]);
//		}
//		if (y3.size() == nsamples) {
//			gr3[i] = new TGraph(nsamples, &x[0], &y3[0]);
//			mg3->Add(gr3[i]);
//		}
//		if (y4.size() == nsamples) {
//			gr4[i] = new TGraph(nsamples, &x[0], &y4[0]);
//			mg4->Add(gr4[i]);
//		}
//
//	}
//
//	// create figure
//
//	TCanvas *c1 = new TCanvas("c1","visual inspection",200,10,700,500);
//	c1->SetGrid();
//
//	TPad *pad1 = new TPad("pad1", "", 0, 0, 1, 1); // base pad for pavg, plen, pcnt
//	pad1->SetGrid();
//	pad1->SetLogy();
//	pad1->Draw();
//	pad1->cd();
//
//	mg1->Draw("ACP");
//	c1->Print("./pictures/ProbeTranRisingEdge.png");
//	mg2->Draw("ACP");
//	c1->Print("./pictures/ProbeTranBreakdownTime.png");
//	mg3->Draw("ACP");
//	c1->Print("./pictures/ProbeReflRisingEdge.png");
//	mg4->Draw("ACP");
//	c1->Print("./pictures/ProbeReflBreakdownTime.png");
//
//
////	for (size_t i=0; i<n; i++) {
////		delete gr1[i];
////		delete gr2[i];
////		delete gr3[i];
////		delete gr4[i];
////	}
//	delete mg1;
//	delete mg2;
//	delete mg3;
//	delete mg4;
//
//	delete c1;
}


///////////////////////////////////////////////////////////////////////////////
/// Plotting function.
/// Plots the history as a 2D scattering plot.
void XboxAnalyserView::plotTest(const std::string &filepath, TH2D &h, std::string mode) {

//	TStyle* m_gStyle;
//	m_gStyle = new TStyle();
//	m_gStyle->SetPalette(kRainBow);

	// create plot
	TCanvas *ctd = new TCanvas("c0","Breakdown studies",200,10,700,500);

	h.SetMarkerColor(kBlack);
//	h.SetTitle("history plot; time [s]; power [W]" );

	h.Draw(mode.c_str());

	h.GetXaxis()->SetTimeDisplay(1);
	h.GetXaxis()->SetTimeFormat("%m/%y%F1970-01-01 00:00:00");
	//	h.GetXaxis()->SetTitle("time [s]");
	//	h.GetYaxis()->SetTitle("power");

//	ctd->Update(); // draws the frame, after which one can change it
//	ctd->Modified();

	ctd->Print(filepath.c_str());
	delete ctd;
}


///////////////////////////////////////////////////////////////////////////////
/// Plotting function.
/// Plots the history as a 2D scattering plots.
void XboxAnalyserView::plotTest(const std::string &filepath) {

	fXBins = 1000;

	size_t nEvents = fCategoryChannel["B0Events"].size();

	std::vector<TTimeStamp> vTime(nEvents);
	std::vector<ULong64_t> vPCnt(nEvents);
	std::vector<Double_t> vPAvg(nEvents);
	std::vector<Double_t> vPLen(nEvents);

//	fDateBegin = 0;
//	fDateEnd = nEvents;

	TH2D *hpavg = new TH2D("HistPulseAvg","average power; date [m/y]; power [W]",
			fXBins, fDateBegin , fDateEnd, fYBins, 0., 1.);
	TH2D *hplen = new TH2D("HistPulseLen","pulse length; date [m/y]; time [s]",
			fXBins, fDateBegin , fDateEnd, fYBins, 0., 1.);
	TH2D *hpcnt = new TH2D("HistPulseCnt","pulse count; date [m/y]; number of pulses ",
			fXBins, fDateBegin , fDateEnd, fYBins, 0., 1.);


	for (size_t i=0; i<nEvents; i++) {
		vTime[i] = fCategoryChannel["B0Events"][i].getTimeStamp();
		vPCnt[i] = fCategoryChannel["B0Events"][i].getPulseCount();
		vPAvg[i] = fCategoryChannel["B0Events"][i].getYmean();
		vPLen[i] = fCategoryChannel["B0Events"][i].getXmax() - fCategoryChannel["B0Events"][i].getXmin();
	}

	std::vector<Double_t> vPCntN = rescale(vPCnt, fPCntMin, fPCntMax, false);
	std::vector<Double_t> vPAvgN = rescale(vPAvg, fPAvgMin, fPAvgMax, false);
	std::vector<Double_t> vPLenN = rescale(vPLen, fPLenMin, fPLenMax, false);

	for (size_t i=0; i<nEvents; i++) {
		hpcnt->Fill((Double_t)vTime[i], vPCntN[i]);
		hpavg->Fill((Double_t)vTime[i], vPAvgN[i]);
		hplen->Fill((Double_t)vTime[i], vPLenN[i]);
	}

//	plotTest(filepath + "_PulseAvg.png", hpavg, "scat=1");
//	plotTest(filepath + "_PulseLen.png", hplen, "scat=1");
//	plotTest(filepath + "_PulseCnt.png", hpcnt, "COLZ");

}


////////////////////////////////////////////////////////////////////////////////
/// Save current canvas to file.
void XboxAnalyserView::show() {
	fCanvas->Draw();
}

////////////////////////////////////////////////////////////////////////////////
/// Save current canvas to file.
void XboxAnalyserView::save(const std::string &filepath) {
	fCanvas->Print(filepath.c_str());
}


#ifndef XBOX_NO_NAMESPACE
}
#endif


