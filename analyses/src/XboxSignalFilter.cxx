/*
 * XboxAnalyserBase.cxx
 *
 *  Created on: Oct 31, 2018
 *      Author: kpapke
 */

#include "XboxSignalFilter.hxx"
#include "XboxCubicSpline.hxx"

#include "Math/Math.h"

//#define EIGEN
#ifdef EIGEN
#include <Eigen/Core>
#include <Eigen/Dense>
#else
#include "TMatrixD.h"
#include "TVectorD.h"
#include "TDecompQRH.h"
#endif


#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif


////////////////////////////////////////////////////////////////////////
/// Constructor.
XboxSignalFilter::XboxSignalFilter() :
	fFilterType{kSavitzkyGolay},
	fFilterOrder{3},
	fFilterNl{7},
	fFilterNr{7},
	fDerivative{0} {
}

////////////////////////////////////////////////////////////////////////
/// Constructor.
XboxSignalFilter::XboxSignalFilter(const EFilterType &filtertype, Int_t order, Int_t nl, Int_t nr) :
	fFilterType{filtertype},
	fFilterOrder{order},
	fFilterNl{nl},
	fFilterNr{nr},
	fDerivative{0} {
}

////////////////////////////////////////////////////////////////////////
/// Destructor.
XboxSignalFilter::~XboxSignalFilter()
{
}

////////////////////////////////////////////////////////////////////////
/// Setter.
void XboxSignalFilter::config(const EFilterType &filtertype, Int_t order, Int_t nl, Int_t nr) {

	fFilterType = filtertype;
	fFilterOrder = order;
	fFilterNl = nl;
	fFilterNr = nr;
}

////////////////////////////////////////////////////////////////////////
/// Setter.
void XboxSignalFilter::setDerivative(Int_t val) {

	fDerivative = val;
}



////////////////////////////////////////////////////////////////////////
/// \brief A set of Savitzky-Golay filter coefficients.
/// \param nl number of leftward (past) data points.
/// \param nr number of rightward (future) data points.
/// \param m polynomial order
/// \param ld order of the derivative desired
/// \return vector of coefficients
///
/// Returns in c[0..np-1], in wraparound order (N.B.!) consistent
/// with the argument response in routine convlv, a set of Savitzky-Golay
/// filter coefficients. For the derivative of order k, you must multiply the
/// array c by kŠ.) m is the order of the smoothing polynomial, also equal to
/// the highest conserved moment; usual values are m D 2 or m D 4.
std::vector<Double_t> XboxSignalFilter::savgol(Int_t m, Int_t nl, Int_t nr, Int_t ld) const {
	if (nl < 0 || nr < 0 || ld > m || nl+nr < m)
		throw("ERROR: Bad arguments in function savgol");

#ifdef EIGEN
	Eigen::MatrixXd A = Eigen::MatrixXd::Zero(m+1, m+1); // system matrix init with zeros
	Eigen::VectorXd x;	// vector of unknowns
	Eigen::VectorXd b = Eigen::VectorXd::Zero(m+1); // right hand side vector init with zeros
#else
	TMatrixD A(m+1, m+1); // system matrix init with zeros
	A.Zero();
	TVectorD x(m+1); // vector of unknowns
	TVectorD b(m+1); // right hand side vector init with zeros
	b.Zero();
#endif

	for (Int_t ipj=0; ipj<=(m << 1); ipj++) { // equations of least-squares fit.
		Double_t sum=(ipj ? 0.0 : 1.0);

		for (Int_t k=1; k<=nr; k++)
			sum += pow(Double_t(k),Double_t(ipj));
		for (Int_t k=1; k<=nl; k++)
			sum += pow(Double_t(-k),Double_t(ipj));

		Int_t mm = std::min(ipj, 2*m-ipj);
		for (Int_t imj=-mm; imj<=mm; imj+=2)
			A((ipj+imj)/2, (ipj-imj)/2) = sum;
	}
	b(ld) = 1.0; // Right-hand side vector is unit vector, depending on derivative (ld).

// Solve linear system of equations
#ifdef EIGEN
	x = A.colPivHouseholderQr().solve(b);
#else
	TDecompQRH qr(A);
	Bool_t state;
	x = qr.Solve(b, state);
	if(!state)
		printf("Error: Could not solve Ax = b\n");
#endif

	// Each Savitzky-Golay coefficient is the dot product of powers of an integer
	// with the inverse matrix row.

//	Int_t np = nr + nl + 1;
	std::vector<Double_t> coeffs;
	for (Int_t k=-nl; k<=nr; k++) {
		Double_t sum = x(0);
		Double_t fac = 1.;
		for (Int_t mm=1; mm<=m; mm++)
			sum += x(mm)*(fac *= k);
//		Int_t kk = ((np-k) % np); // store in wrap around order
		coeffs.push_back(sum);
	}

	return coeffs;
}

////////////////////////////////////////////////////////////////////////
/// Convolution of two signals.
std::vector<Double_t> XboxSignalFilter::conv(const std::vector<Double_t> &f,
		const std::vector<Double_t> &kernel, Int_t nl, Int_t nr) const {

	std::vector<Double_t> conv;
	conv.resize(f.size());

	Int_t nf = f.size();
	Int_t nk = kernel.size();
	Int_t nc = nf + nk - 1;
	conv.resize(nc - nl - nr);

	for(Int_t i=0; i<nc; i++)
	{
	    Int_t i1 = i;
		Double_t tmp = 0.;
	    for(Int_t j=0; j<nk; j++){
	    	if(i1>=0 && i1<nf)
	    		tmp += f[i1] * kernel[j];
	    	i1--;
	    }
    	if(i>=nl && i<nc-nr)
    		conv[i-nl] = tmp;
	}
	return conv;
}

////////////////////////////////////////////////////////////////////////
/// Internal Filter function.
/// Filters a signal given as a vector
/// \param[in] vec input vector.
/// \return The filered signal.
std::vector<Double_t> XboxSignalFilter::filter (std::vector<Double_t> &vec) const {

	std::vector<Double_t> fvec;
	if(fFilterType == kNone) {
		fvec = vec;
	}
	else if(fFilterType == kMovingAverage){ // Moving average filter
		std::vector<Double_t> kernel = std::vector<Double_t>(fFilterNl + fFilterNr +1, 1./(fFilterNl + fFilterNr +1.));
		fvec = conv(vec, kernel, fFilterNr, fFilterNl);
	}
	else if(fFilterType == kSavitzkyGolay){ // Savitzky-Golay filter
		std::vector<Double_t> kernel = savgol(fFilterOrder, fFilterNl, fFilterNr, 0);
		std::reverse(kernel.begin(), kernel.end()); // reverse order of coefficients
		fvec = conv(vec, kernel, fFilterNr, fFilterNl);
	}

	return fvec;
}


////////////////////////////////////////////////////////////////////////
/// Internal Filter function.
/// Filters the derivative of a signal given as a vector.
/// \param[in] vec input vector.
/// \param[in] dt time resolution.
/// \return The filered signal.
std::vector<Double_t> XboxSignalFilter::filterP1 (std::vector<Double_t> &vec, Double_t dt) const {

	std::vector<Double_t> fvec;
	Double_t dh = 1. / dt;
	if(fFilterType == kNone){
		std::vector<Double_t> kernel = {dh, 0, -dh};
		fvec = conv(vec, kernel, 1, 1);
	}
	else if(fFilterType == kMovingAverage){ // Moving average filter
		std::vector<Double_t> kernel(fFilterNl, dh);
		kernel.push_back(0.);
		for(Int_t i=0; i<fFilterNr; i++)
			kernel.push_back(-dh);
		fvec = conv(vec, kernel, fFilterNr, fFilterNl);
	}
	else if(fFilterType == kSavitzkyGolay){ // Savitzky-Golay filter
		std::vector<Double_t> kernel = savgol(fFilterOrder, fFilterNl, fFilterNr, 1);
		for(auto it=kernel.begin(); it != kernel.end(); ++it)
			(*it) *= dh;
		std::reverse(kernel.begin(), kernel.end()); // reverse order of coefficients
		fvec = conv(vec, kernel, fFilterNr, fFilterNl);
	}

	return fvec;
}


////////////////////////////////////////////////////////////////////////
/// Internal Filter function.
/// Filters the derivative of a signal given as a vector.
/// \param[in] vec input vector.
/// \param[in] dt time resolution.
/// \return The filered signal.
std::vector<Double_t> XboxSignalFilter::filterP2 (std::vector<Double_t> &vec, Double_t dt) const {

	std::vector<Double_t> fvec;
	Double_t dh = 1. / dt / dt;
	if(fFilterType == kNone){
		std::vector<Double_t> kernel = {dh, -2*dh, +dh};
		fvec = conv(vec, kernel, 1, 1);
	}
	else if(fFilterType == kMovingAverage){ // Moving average filter
		std::vector<Double_t> kernel(fFilterNl + fFilterNr + 1, dh);
		kernel[fFilterNl] = -2*dh;
		fvec = conv(vec, kernel, fFilterNr, fFilterNl);
	}
	else if(fFilterType == kSavitzkyGolay){ // Savitzky-Golay filter
		std::vector<Double_t> kernel = savgol(fFilterOrder, fFilterNl, fFilterNr, 2);
		for(auto it=kernel.begin(); it != kernel.end(); ++it)
			(*it) *= dh;
		std::reverse(kernel.begin(), kernel.end()); // reverse order of coefficients
		fvec = conv(vec, kernel, fFilterNr, fFilterNl);
	}

	return fvec;
}

////////////////////////////////////////////////////////////////////////
/// Equidistant re-sampling function.
/// Re-samples a signal given as a vector in the predefined interval.
/// \param[in] x input vector.
/// \param[in] y input vector.
/// \param[in] lowerlimit the lower limit of the time axis.
/// \param[in] upperlimit the upper limit of the time axis.
/// \param[in] nsamples the number of points to re-sample.
/// \return The filered signal.
//std::vector<Double_t> XboxSignalFilter::resample
//		(std::vector<Double_t> &x, std::vector<Double_t> &y,
//		Double_t lowerlimit, Double_t upperlimit, Int_t nsamples) const {
//
//	std::vector<Double_t> ys(nsamples);
//	ROOT::Math::Interpolator interp(x, y, fInterpType);
//	Double_t h = (upperlimit - lowerlimit) / (nsamples-1);
//	Double_t xi = lowerlimit;
//	for(auto it=ys.begin(); it != ys.end(); ++it, xi+=h){
//		if(xi <= x.front())
//			*it = interp.Eval(x.front());
//		else if(xi >= x.back())
//			*it = interp.Eval(x.back());
//		else
//			*it = interp.Eval(xi);
//	}
//	return ys;
//}

std::vector<Double_t> XboxSignalFilter::resample
		(std::vector<Double_t> &x, std::vector<Double_t> &y,
		Double_t lowerlimit, Double_t upperlimit, Int_t nsamples) const {

	std::vector<Double_t> ys(nsamples);

	XBOX::CubicSpline s(x, y);

	Double_t h = (upperlimit - lowerlimit) / (nsamples-1);
	Double_t xi = lowerlimit;
	for(auto it=ys.begin(); it != ys.end(); ++it, xi+=h){
		if(xi <= x.front())
			*it = s(x.front());
		else if(xi >= x.back())
			*it = s(x.back());
		else
			*it = s(xi);
	}
	return ys;
}


////////////////////////////////////////////////////////////////////////
/// General re-sampling function.
/// Re-samples a signal given as a vector at predefined points.
/// \param[in] x time axis.
/// \param[in] y signal.
/// \param[in] xs re-sampled time axis.
/// \return The re-sampled signal.
//std::vector<Double_t> XboxSignalFilter::resample (std::vector<Double_t> &x,
//		std::vector<Double_t> &y, std::vector<Double_t> &xs) const {
//
//	std::vector<Double_t> ys(xs.size());
//	ROOT::Math::Interpolator interp(x, y, fInterpType);
//
//	for(size_t i=0; i<xs.size(); i++) {
//		if(xs[i] <= x.front())
//			ys[i] = interp.Eval(x.front());
//		else if(xs[i] >= x.back())
//			ys[i] = interp.Eval(x.back());
//		else
//			ys[i] = interp.Eval(xs[i]);
//	}
//
//	return ys;
//}

std::vector<Double_t> XboxSignalFilter::resample (std::vector<Double_t> &x,
		std::vector<Double_t> &y, std::vector<Double_t> &xs) const {

	std::vector<Double_t> ys(xs.size());
	XBOX::CubicSpline s(x, y);

	for(size_t i=0; i<xs.size(); i++) {
		if(xs[i] <= x.front())
			ys[i] = s(x.front());
		else if(xs[i] >= x.back())
			ys[i] = s(x.back());
		else
			ys[i] = s(xs[i]);
	}

	return ys;
}


////////////////////////////////////////////////////////////////////////
/// Filter function.
/// Filters the signal given as vector in the argument list
/// \param[in] signal.
/// \return The filered signal.
std::vector<Double_t> XboxSignalFilter::operator () (std::vector<Double_t> y) {

	// apply predefined filter to the signal
	if (fDerivative == 1)
		return filterP1(y, 1.);
	else if (fDerivative == 2)
		return filterP2(y, 1.);
	else
		return filter(y);
}


////////////////////////////////////////////////////////////////////////
/// Filter function.
/// Filters the signal in the entire argument range without re-sampling.
/// \param[in] ch Xbox DAQ channel containing the signal.
/// \return The filered signal.
std::vector<Double_t> XboxSignalFilter::operator () (XBOX::XboxDAQChannel &ch) {

	// flush any previously evaluated data of the channel
//	ch.flushbuffer();

	// get signal in the entire argument range
	std::vector<Double_t> y;
	y = ch.getSignal();

	// apply predefined filter to the signal
	if (fDerivative == 1)
		return filterP1(y, ch.getIncrement());
	else if (fDerivative == 2)
		return filterP2(y, ch.getIncrement());
	else
		return filter(y);
}

////////////////////////////////////////////////////////////////////////
/// Filter function.
/// Filters the signal in a predefined argument range without re-sampling.
/// To get the corresponding time axis use: ch.getTimeAxis(lowerlimit, upperlimit)
/// \param[in] ch a Xbox DAQ channel containing the signal.
/// \param[in] lowerlimit the lower limit of the time axis.
/// \param[in] upperlimit the upper limit of the time axis.
/// \return The filered signal.
std::vector<Double_t> XboxSignalFilter::operator ()
		(XBOX::XboxDAQChannel &ch, Double_t tmin, Double_t tmax) {

	// flush any previously evaluated data of the channel
//	ch.flushbuffer();

	// get signal in the predefined argument range
	std::vector<Double_t> y = ch.getSignal(tmin, tmax);

	// apply predefined filter to the signal
	if (fDerivative == 1)
		return filterP1(y, ch.getIncrement());
	else if (fDerivative == 2)
		return filterP2(y, ch.getIncrement());
	else
		return filter(y);
}

////////////////////////////////////////////////////////////////////////
/// Filter function.
/// Filters and re-sample the signal in a predefined argument range.
/// To get the corresponding time axis use: ch.getTimeAxis(lowerlimit, upperlimit).
/// \param[in] ch a Xbox DAQ channel containing the signal.
/// \param[in] lowerlimit the lower limit of the time axis.
/// \param[in] upperlimit the upper limit of the time axis.
/// \param[in] nsamples the number of points to re-sample.
/// \return The filered and re-sampled signal.
std::vector<Double_t> XboxSignalFilter::operator ()
		(XBOX::XboxDAQChannel &ch, Double_t tsmin, Double_t tsmax, Int_t nsamples) {

	// flush any previously evaluated data of the channel
//	ch.flushbuffer();

	// get time axis and signal in the predefined argument range
	Double_t lbnd = 0.; ///<Lower time axis bound.
	Double_t ubnd = 0.; ///<Upper time axis bound.
	Double_t tmin = tsmin; ///<Lower time axis limit.
	Double_t tmax = tsmax; ///<Upper time axis limit.
	Double_t dt = ch.getIncrement();

	ch.getTimeAxisBounds(lbnd, ubnd);

	tmin -= dt * 2 * (fFilterNl + 1);
	tmax += dt * 2 * (fFilterNr + 1);
	if (tmin < lbnd)
		tmin = lbnd;
	if (tmax < ubnd)
		tmax = ubnd;

	// get time axis and signal in the predefined argument range
	std::vector<Double_t> t = ch.getTimeAxis(tmin, tmax);
	std::vector<Double_t> y = ch.getSignal(tmin, tmax);

	// apply predefined filter to the signal
	std::vector<Double_t> yf;
	if (fDerivative == 1)
		yf = filterP1(y, ch.getIncrement());
	else if (fDerivative == 2)
		yf = filterP2(y, ch.getIncrement());
	else
		yf = filter(y);

	// resample the signal in the predefined interval given by the bounds
	std::vector<Double_t> yfs = resample(t, yf, tsmin, tsmax, nsamples);
	return yfs;
}

////////////////////////////////////////////////////////////////////////
/// Filter function.
/// Filters and re-samples the signal for a predefined argument axis.
/// The predefined time axis must be sorted in ascending direction.
/// \param[in] ch a Xbox DAQ channel containing the signal.
/// \param[in] xs a predefined time axis the signal is re-sampled on.
/// \return The filered and re-sampled signal.
std::vector<Double_t> XboxSignalFilter::operator ()
		(XBOX::XboxDAQChannel &ch, std::vector<Double_t> &ts) {

	// flush any previously evaluated data of the channel
//	ch.flushbuffer();

	// get time axis and signal in the predefined argument range
	Double_t lbnd = 0.; ///<Lower time axis bound.
	Double_t ubnd = 0.; ///<Upper time axis bound.
	Double_t tmin = ts.front(); ///<Lower time axis limit.
	Double_t tmax = ts.back(); ///<Upper time axis limit.
	Double_t dt = ch.getIncrement();

	ch.getTimeAxisBounds(lbnd, ubnd);

	tmin -= dt * 2 * (fFilterNl + 1);
	tmax += dt * 2 * (fFilterNr + 1);
	if (tmin < lbnd)
		tmin = lbnd;
	if (tmax < ubnd)
		tmax = ubnd;

	std::vector<Double_t> t = ch.getTimeAxis(tmin, tmax);
	std::vector<Double_t> y = ch.getSignal(tmin, tmax);

	// apply predefined filter to the signal
	std::vector<Double_t> yf;
	if (fDerivative == 1)
		yf = filterP1(y, ch.getIncrement());
	else if (fDerivative == 2)
		yf = filterP2(y, ch.getIncrement());
	else
		yf = filter(y);

	// resample the signal in the predefined interval given by the bounds
	std::vector<Double_t> yfs = resample(t, yf, ts);
	return yfs;
}


#ifndef XBOX_NO_NAMESPACE
}
#endif
