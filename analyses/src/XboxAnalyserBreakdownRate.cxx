#include "XboxAnalyserBreakdownRate.hxx"

// root
#include "TMath.h"

#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif


////////////////////////////////////////////////////////////////////////
/// Constructor.
XboxAnalyserBreakdownRate::XboxAnalyserBreakdownRate(std::vector<ULong64_t> vPCnt)
	: fPCnt(vPCnt) {
	init();
}

////////////////////////////////////////////////////////////////////////
/// Destructor.
XboxAnalyserBreakdownRate::~XboxAnalyserBreakdownRate() {
	clear();
}

////////////////////////////////////////////////////////////////////////
/// Default Settings.
void XboxAnalyserBreakdownRate::init() {

	fNSamples = 1001;
	fWeightType = kGaussian;
	fSigma = 0.1;
	fNSigma = 3;
}

////////////////////////////////////////////////////////////////////////
/// Clear.
/// Subroutine to destruct this object.
void XboxAnalyserBreakdownRate::clear(){
}

////////////////////////////////////////////////////////////////////////
/// Reset.
void XboxAnalyserBreakdownRate::reset() {
	clear();
	init();
}

////////////////////////////////////////////////////////////////////////
/// Setter.
/// Configures the weighting functions.
/// \param[in] wtype The type of weighting function.
/// \param[in] sig   The sigma measured as the relative value from the total
///                  pulse count interval.
/// \param[in] nsig  The number of sigma up to the function is truncated.
void XboxAnalyserBreakdownRate::setWeightFunction(EWeightType wtype,
		Double_t sig, Int_t nsig) {

	fWeightType=wtype;
	fSigma = sig;
	fNSigma = nsig;
}

////////////////////////////////////////////////////////////////////////
/// Evaluation of breakdown rate.
/// The breakdown rate is evaluated by the difference of pulse counts between
/// subsequent events. The breakdown rate is evaluated by comparing the pulse
/// count of subsequent events using a fixed number of events per interval.
/// \param[out] pcnt The vector of pulse counts.
/// \param[out] rate The vector of breakdown rates.
/// \param[in] step (optional) Spacing between the subsequent events.
void XboxAnalyserBreakdownRate::fromEquidistantStep(std::vector<ULong64_t> &pcnt,
		std::vector<Double_t> &rate, Int_t nstep) {

	size_t n = fPCnt.size();

	pcnt.resize(n, 0);
	rate.resize(n, 0);

	for (size_t i=0; i<n; i++) {
		pcnt[i] = fPCnt[i];
	}
	for (size_t i=nstep; i<n; i++)
		rate[i] = (Double_t)nstep / (fPCnt[i] - fPCnt[i-nstep]);
	for (Int_t i=0; i<nstep; i++)   // boundary condition
		rate[i] = rate[nstep];

}

////////////////////////////////////////////////////////////////////////
/// Evaluation of breakdown rate.
/// The breakdown rate is evaluated by the difference of pulse counts between
/// subsequent events. The breakdown rate is evaluated by the number of events
/// within constant pulse count intervals.
/// \param[out] pcnt The vector of pulse counts.
/// \param[out] rate The vector of breakdown rates.
void XboxAnalyserBreakdownRate::fromEquidistantPCnt(std::vector<ULong64_t> &pcnt,
		std::vector<Double_t> &rate) {

	size_t n = fPCnt.size();
	ULong64_t step = (fPCnt.back() - fPCnt.front()) / (fNSamples - 1);

	pcnt.resize(fNSamples, 0);
	rate.resize(fNSamples, 0);

	size_t i=0;
	for (Int_t j=0; j<fNSamples; j++) {

		ULong64_t pcnt_end = fPCnt[i] + step; // end of current pulse count interval

		Int_t nstep=0;
		while(i < n && fPCnt[i] < pcnt_end) {
			nstep++;
			i++;
		}

		pcnt[j] = fPCnt[i];
		if (nstep)
			rate[j] = (Double_t)nstep / (fPCnt[i] - fPCnt[i-nstep]);
		else
			rate[j] = 0.;
		i++;
	}

}

////////////////////////////////////////////////////////////////////////
/// Evaluation of the breakdown rate.
/// The breakdown rate is evaluated by the difference of pulse counts between
/// subsequent events. A moving gaussian weighting function is used to smooth
/// the resulting breakdown rate over pulse count.
/// \param[out] pcnt the vector of pulse counts.
/// \param[out] rate the vector of breakdown rates.
void XboxAnalyserBreakdownRate::fromMovingWindow(std::vector<ULong64_t> &pcnt,
		std::vector<Double_t> &rate) {

	size_t n = fPCnt.size();
	pcnt.resize(fNSamples);
	rate.resize(fNSamples);

	ULong64_t pcntMin = *std::min_element(fPCnt.begin(), fPCnt.end());
	ULong64_t pcntMax = *std::max_element(fPCnt.begin(), fPCnt.end());
	ULong64_t pcntInc = (pcntMax - pcntMin) / fNSamples;
	ULong64_t pcntSig = (pcntMax - pcntMin) * fSigma;
	ULong64_t pcntCut = (pcntMax - pcntMin) * fSigma * fNSigma;
	ULong64_t pcntVal = pcntMin;

	for (size_t ipnt=0; ipnt<fNSamples; ipnt++) {

		// search for nearest neighbour index
		size_t idxNN=0;
		Bool_t bUpperNN = false;
		ULong64_t pcntDist = 0;

		if (fPCnt.back() < pcntVal) {
			idxNN = n-1;
			pcntDist = pcntVal - fPCnt[idxNN];
			bUpperNN = false;
		}
		else {
			for (size_t i=1; i < n; i++) {
				if (pcntVal < fPCnt[i]) {
					if ((fPCnt[i]-pcntVal) < (pcntVal-fPCnt[i-1])) {
						idxNN = i;
						pcntDist = fPCnt[i] - pcntVal;
						bUpperNN = true;
					}
					else
						idxNN = i-1;
						pcntDist = pcntVal - fPCnt[i];
						bUpperNN = false;
					break;
				}
			}
		}

		// disable weighting by future points. The nearest neighbour must not
		// be on the upper side.
		if (bUpperNN) {
			idxNN--;
			pcntDist = pcntVal - fPCnt[idxNN];
			bUpperNN = false;
		}

		// search index of lower bound to span a window over past samples
		size_t nl=0;
		ULong64_t lbnd = (fPCnt.front()+pcntCut < pcntVal) ? pcntVal-pcntCut : fPCnt.front();
		while(nl+1 < idxNN && fPCnt[idxNN-nl-1] >= lbnd)
			nl++;

		// search index of upper bound to span a window over future samples (disabled)
		size_t nu=0;
//		ULong64_t ubnd = (pcntVal+pcntCut < fPCnt.back()) ? pcntVal+pcntCut : fPCnt.back();
//		while(nu+1 < (fPCnt.size()-idxNN-1) && fPCnt[idxNN+nu+1] <= ubnd)
//			nu++;

        // breakdown rate and weighting function for each sample
		std::vector<Double_t> f(nl+nu+1, 0.);
		std::vector<Double_t> w(nl+nu+1, 1.);

		// breakdown rate and weighting function for nearest neighbour event
		if (bUpperNN) {

			if (fPCnt[idxNN] > pcntVal) {
				w[0] = 1.;
				f[0] = 1. / (fPCnt[idxNN] - pcntVal);
			}
			else {
				w[0] = 0.;
				f[0] = 0.;
			}
		}
		else {
			if (pcntVal > fPCnt[idxNN]) {
				w[0] = 1.;
				f[0] = 1. / (pcntVal-fPCnt[idxNN]);
			}
			else {
				w[0] = 0.;
				f[0] = 0.;
			}
		}

		// breakdown rate and weighting function for events after nearest neighbour event
		for (size_t i=1; i<=nu; i++) {
			if (fPCnt[idxNN+i] > pcntVal) {
				Int_t nevents = (bUpperNN) ? i+1 : i; // extra event if NN is after pcnt
				w[i] = exp(-pow(fPCnt[idxNN+i]-pcntVal+pcntDist, 2.) /
					(2 * pow(pcntSig, 2)));
				f[i] = 1. / (fPCnt[idxNN+i]-pcntVal) * nevents;
			}
			else {
				f[i] = 0.;
				w[i] = 0.;
			}
		}

		// breakdown rate and weighting function for events before nearest neighbour event
		for (size_t i=1; i<=nl; i++) {
			if (pcntVal > fPCnt[idxNN-i]) {
				Int_t nevents = (!bUpperNN) ? i : i+1; // extra event if NN is before pcnt
				w[i+nu] = exp(-pow(pcntVal-pcntDist-fPCnt[idxNN-i], 2.) /
						(2 * pow(pcntSig, 2)));
				f[i+nu] = 1. / (pcntVal - fPCnt[idxNN-i]) * nevents;
			}
			else {
				f[i] = 0.;
				w[i] = 0.;
			}
		}

		// weight breakdown rate
		Double_t rateVal=0.;
		Double_t sum=0.;
		for (size_t i=0; i<w.size(); i++) {
			if (f[i] == 0.)
				w[i] = 0.;
			sum += w[i];
			rateVal += w[i] * f[i];
		}
		rateVal /= sum; // normalisation

		pcnt[ipnt] = pcntVal;
		rate[ipnt] = rateVal;
		pcntVal += pcntInc;
	}

}


#ifndef XBOX_NO_NAMESPACE
}
#endif


