#include "XboxAnalyserEvalPulseShape.hxx"

#include <iostream>

// root
#include "TTimeStamp.h"

// root graphics
#include "TStyle.h"
#include "TColor.h"
#include "TCanvas.h"
#include "TFrame.h"
#include "TPad.h"
#include "TAxis.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TLine.h"
#include "TLegend.h"


#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif


////////////////////////////////////////////////////////////////////////
/// Constructor.
XboxAnalyserEvalPulseShape::XboxAnalyserEvalPulseShape() {
	init();
}

////////////////////////////////////////////////////////////////////////
/// Constructor.
XboxAnalyserEvalPulseShape::XboxAnalyserEvalPulseShape(Double_t wmin,
		Double_t wmax, Double_t th) {
	init();
	configPulse(wmin, wmax, th);
}

////////////////////////////////////////////////////////////////////////
/// Destructor.
XboxAnalyserEvalPulseShape::~XboxAnalyserEvalPulseShape() {
	clear();
}

////////////////////////////////////////////////////////////////////////
/// Default Settings.
void XboxAnalyserEvalPulseShape::init() {

	fPulseWmin = 0.01;
	fPulseWmax = 0.99;
	fPulseTh = 0.9;

	fReportFlag = false;
	fReportDir = "";
}

////////////////////////////////////////////////////////////////////////
/// Clear.
void XboxAnalyserEvalPulseShape::clear(){
}



////////////////////////////////////////////////////////////////////////
/// Reset.
void XboxAnalyserEvalPulseShape::reset() {
	clear();
	init();
}

////////////////////////////////////////////////////////////////////////
/// Setter.
/// Set the time window and thresholds for the pulse shape analysis.
/// All parameters are relative to the maximum time window and the
/// magnitude of the signal in the considered time window.
/// \param[in] wmin The minimum of the signal time axis to be considered.
/// \param[in] wmax The maximum of the signal time axis to be considered.
/// \param[in] th The threshold at which the pulse top level is measured.
void XboxAnalyserEvalPulseShape::configPulse(Double_t wmin, Double_t wmax, Double_t th) {

	fPulseWmin = wmin;
	fPulseWmax = wmax;
	fPulseTh = th;
}

////////////////////////////////////////////////////////////////////////
/// Calling function.
/// Evaluates pulse shape parameter from a channel signal.
/// \param[in] ch The Xbox channel.
/// \return The pulse parameters (pulse length, average power, ...)
XBOX::XboxDAQChannel XboxAnalyserEvalPulseShape::operator () (
		XBOX::XboxDAQChannel &ch) {

	ch.flushbuffer();
	XBOX::XboxDAQChannel chnew = ch;

	// avoid data re-interpretation
	chnew.setAutoRefresh(false);

	Double_t lbnd=0.;
	Double_t ubnd=0.;
	ch.getTimeAxisBounds(lbnd, ubnd);

	Double_t tmin = fPulseWmin * (ubnd - lbnd) + lbnd;
	Double_t tmax = fPulseWmax * (ubnd - lbnd) + lbnd;
	Double_t tr = chnew.risingEdge(fPulseTh, tmin, tmax);
	Double_t tf = chnew.fallingEdge(fPulseTh, tmin, tmax);
	Double_t pmin = chnew.min(tr, tf);
	Double_t pmax = chnew.max(tr, tf);
	Double_t pmean = chnew.mean(tr, tf);
	Double_t pinteg = chnew.integ(tr, tf);
	Double_t pspan = chnew.span();

	// update the results in channel
	chnew.setXmin(tr);
	chnew.setXmax(tf);
	chnew.setYmin(pmin);
	chnew.setYmax(pmax);
	chnew.setYmean(pmean);
	chnew.setYinteg(pinteg);
	chnew.setYspan(pspan);

	// reset auto interpretation
	ch.setAutoRefresh(true);
	chnew.setAutoRefresh(true);

	if (fReportFlag)
		report(chnew);


	return chnew;
}

////////////////////////////////////////////////////////////////////////
/// Report.
/// \param[in] ch The evaluated Xbox channel.
void XboxAnalyserEvalPulseShape::report (XBOX::XboxDAQChannel &ch) {

	printf("----------------------------------------------------\n");
	printf("Report EvalPulseShape\n");
	printf("----------------------------------------------------\n");
	printf("fXboxVersion           : %u\n", ch.getXboxVersion());
	printf("fChannelName           : %s\n", ch.getChannelName().c_str());

	printf("fTimestamp             : %s\n", ch.getTimeStamp().AsString("s"));
	printf("fPulseCount            : %llu\n", ch.getPulseCount());

	printf("fStartTime             : %s\n", ch.getStartTime().AsString("s"));
	printf("fStartOffset           : %e\n", ch.getStartOffset());
	printf("fIncrement             : %e\n", ch.getIncrement());
	printf("fNSamples              : %u\n", ch.getSamples());

	printf("fXmin                  : %e\n", ch.getXmin());
	printf("fXmax                  : %e\n", ch.getXmax());
	printf("fYmin                  : %e\n", ch.getYmin());
	printf("fYmax                  : %e\n", ch.getYmax());
	printf("----------------------------------------------------\n");

	std::string title = "EvalPulseShape " + ch.getChannelName()
			+ " " + ch.getTimeStamp().AsString("s");

	TCanvas c1("c1",title.c_str(),700, 500);
	c1.SetGrid();

	std::vector<Double_t> t0 = ch.getTimeAxis();
	std::vector<Double_t> y0 = ch.getSignal();

	TGraph gr(t0.size(), &t0[0], &y0[0]);
	gr.SetLineColor(kBlack);
	gr.SetMarkerColor(kBlack);
	gr.SetTitle(title.c_str());
	gr.Draw("AC");

	TAxis *ax = gr.GetXaxis();
	TAxis *ay = gr.GetYaxis();

	ax->SetTitle("Time [s]");
	ay->SetTitle("Amplitude");

	TLine *l1 = new TLine(ch.getXmin(), ay->GetXmin(), ch.getXmin(), ay->GetXmax());
	l1->SetLineWidth(1);
	l1->SetLineColor(kBlack);
	l1->Draw();
	l1->SetNDC(false);

	TLine *l2 = new TLine(ch.getXmax(), ay->GetXmin(), ch.getXmax(), ay->GetXmax());
	l2->SetLineWidth(1);
	l2->SetLineColor(kBlue);
	l2->Draw();
	l2->SetNDC(false);

	TLine *l3 = new TLine(ax->GetXmin(), ch.getYmin(), ax->GetXmax(), ch.getYmin());
	l3->SetLineWidth(1);
	l3->SetLineColor(kBlack);
	l3->Draw();
	l3->SetNDC(false);

	TLine *l4 = new TLine(ax->GetXmin(), ch.getYmax(), ax->GetXmax(), ch.getYmax());
	l4->SetLineWidth(1);
	l4->SetLineColor(kBlue);
	l4->Draw();
	l4->SetNDC(false);

	auto legend = new TLegend(0.11,0.7,0.4,0.89);
	legend->AddEntry(&gr, ch.getChannelName().c_str(), "l");
	legend->Draw();

	c1.Draw();
	c1.Print((fReportDir + title + ".png").c_str());

	delete l1;
	delete l2;
	delete l3;
	delete l4;
	delete legend;
}


#ifndef XBOX_NO_NAMESPACE
}
#endif



