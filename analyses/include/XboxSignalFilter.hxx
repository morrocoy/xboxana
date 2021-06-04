/*
 * XboxAnalysis.hxx
 *
 *  Created on: Oct 31, 2018
 *      Author: kpapke
 */

#ifndef _XBOXSIGNALFILTER_HXX_
#define _XBOXSIGNALFILTER_HXX_

#include <iostream>

#include "Rtypes.h"
#include "XboxDAQChannel.hxx"


#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif



class XboxSignalFilter {

public:

	enum EFilterType {
		kNone,
		kMovingAverage,
		kSavitzkyGolay
	};                                     ///<Available filter types.

protected:

	EFilterType           fFilterType;     ///<Filter type.
	Int_t                 fFilterOrder;    ///<Polynomial order.
	Int_t                 fFilterNl;       ///<number of polynomial nodes left from current data point.
	Int_t                 fFilterNr;       ///<number of polynomial nodes right from current data point.

	Int_t                 fDerivative;     ///<Degree of derivative.

	std::vector<Double_t> savgol(Int_t m, Int_t nl, Int_t nr, Int_t ld=0) const;
	std::vector<Double_t> conv(const std::vector<Double_t> &f,
			const std::vector<Double_t> &kernel, Int_t nl=0, Int_t nr=0) const;

	std::vector<Double_t> filter (std::vector<Double_t> &vec) const;
	std::vector<Double_t> filterP1 (std::vector<Double_t> &vec, Double_t dt) const; // first derivative
	std::vector<Double_t> filterP2 (std::vector<Double_t> &vec, Double_t dt) const; // second derivative

	std::vector<Double_t> resample (std::vector<Double_t> &x, std::vector<Double_t> &y,
			Double_t lowerlimit, Double_t upperlimit, Int_t nsamples) const;
	std::vector<Double_t> resample (std::vector<Double_t> &x, std::vector<Double_t> &y,
			std::vector<Double_t> &xs) const;


public:

	XboxSignalFilter();
	XboxSignalFilter(const EFilterType &filtertype, Int_t order, Int_t nl, Int_t nr);
	virtual ~XboxSignalFilter();

	void config(const EFilterType &filtertype, Int_t order, Int_t nl, Int_t nr);
	void setDerivative(Int_t val);

	std::vector<Double_t> operator ()
					(std::vector<Double_t> y);

	std::vector<Double_t> operator ()
					(XBOX::XboxDAQChannel &ch);

	std::vector<Double_t> operator ()
				(XBOX::XboxDAQChannel &ch, Double_t lowerlimit, Double_t upperlimit);

	std::vector<Double_t> operator ()
			(XBOX::XboxDAQChannel &ch, Double_t lowerlimit, Double_t upperlimit, Int_t nsamples);

	std::vector<Double_t> operator ()
				(XBOX::XboxDAQChannel &ch, std::vector<Double_t> &t);

};

#ifndef XBOX_NO_NAMESPACE
}
#endif

#endif /* _XBOXSIGNALFILTER_HXX_ */
