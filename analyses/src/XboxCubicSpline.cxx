#include "XboxCubicSpline.hxx"
#include <iostream>
#include <algorithm>


#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif

////////////////////////////////////////////////////////////////////////
/// Constructor.
/// Applies default boundary condition (zero curvature at both ends)
CubicSpline::CubicSpline(
		const std::vector<double>& x, const std::vector<double>& y) {

	setNodes(x, y);
	setBoundary(kSecondDeriv, 0., kSecondDeriv, 0, false);
	evalCoefficients();
}


////////////////////////////////////////////////////////////////////////
/// Destructor.
CubicSpline::~CubicSpline() {

}


////////////////////////////////////////////////////////////////////////
/// Defines set of Nodes.
/// \param[in] x the vector of x coordinates.
/// \param[in] x the vector of y coordinates.
void CubicSpline::setNodes(const std::vector<double>& x,
		const std::vector<double>& y) {

	if (x.size() != y.size() || x.size() <= 2) {
		fNNodes = 0;
		fXNodes.clear();
		fYNodes.clear();
		return;
	}

	fNNodes = x.size();

	Bool_t bsorted = true;
	Int_t n = x.size();

	// check whether values are sorted and if not sort them
	for(int i=0; i<n-1; i++) {
		if (x[i] > x[i+1]) {
			bsorted = false;
			break;
		}
	}

	if (bsorted) {

		fXNodes = x;
		fYNodes = y;
	}
	else {

		fXNodes = x;
		fYNodes = y;

//		// get vector of indices of sorted x values
//		std::vector<size_t> idx(n);
//		std::iota(idx.begin(), idx.end(), 0);
//		std::sort(idx.begin(), idx.end(),
//				[&x](size_t i1, size_t i2) { return x[i1] < x[i2]; }
//		);
//
//		// sort points according to the index vector
//		fXNodes.resize(x.size());
//		fYNodes.resize(x.size());
//		std::transform(idx.begin(), idx.end(), fXNodes.begin(),
//				[&x](size_t i) { return x[i]; }	);
//		std::transform(idx.begin(), idx.end(), fYNodes.begin(),
//				[&y](size_t i) { return y[i]; }	);
	}

}

////////////////////////////////////////////////////////////////////////
/// Boundary conditions.
/// Sets the boundary conditions on the left and right (either first or
/// second derivative with the corresponding values
/// \param[in] bndleft the boundary condition at the left.
/// \param[in] bndleftval the boundary value at the left.
/// \param[in] bndright the boundary condition at the right.
/// \param[in] bndrightval the boundary value at the right.
/// \param[in] bLinearExtrapolation a flag for linear extrapolation.
void CubicSpline::setBoundary(CubicSpline::EBoundaryType bndleft, double bndleftval,
		CubicSpline::EBoundaryType bndright, double bndrightval,
                          bool bLinearExtrapolation)
{
	fBndLeft = bndleft;
	fBndRight = bndright;
	fBndLeftVal=bndleftval;
	fBndRightVal=bndrightval;
	fLinearExtrapolation=bLinearExtrapolation;
}

////////////////////////////////////////////////////////////////////////
/// Tridiagonal Matrix Algorithm.
/// The Tridiagonal Matrix Algorithm, also known as the Thomas Algorithm,
/// is an application of gaussian elimination to a banded matrix.
/// Reference: https://en.wikipedia.org/wiki/Tridiagonal_matrix_algorithm
/// \param[in] a the lower diagonal (size n-1).
/// \param[in] b the main diagonal (size n).
/// \param[in] c the upper diagonal (size n-1).
/// \param[in] d the vector at the right hand side (size n).
std::vector<Double_t> CubicSpline::solveTridiagonal(
		std::vector<Double_t> &a, std::vector<Double_t> &b, std::vector<Double_t> &c,
		std::vector<Double_t> &d) {

	int n = d.size();
	std::vector<Double_t> x(n);

	for(int i=1; i < n; i++) {
		double w = a[i-1] / b[i-1];
		b[i] -= w * c[i-1];
		d[i] -= w * d[i-1];
	}

	x[n-1] = d[n-1] / b[n-1];
	for(int i=n-1; i > 0; i--)
		x[i-1] = (d[i-1] - c[i-1] * x[i]) / b[i-1];

	return x;
}

////////////////////////////////////////////////////////////////////////
/// Cubic spline coefficients
/// Evaluated the coefficients a, b, and c of the cubic spline for the
/// given set of nodes.
///          f(x) = a*(x-x_i)^3 + b*(x-x_i)^2 + c*(x-x_i) + y_i
/// Function need to be manually executed after each call of setPoints()
/// or setBoundary() outside of the constructor.
/// Reference: https://kluge.in-chemnitz.de/opensource/spline/
void CubicSpline::evalCoefficients() {

	// check whether nodes are loaded
	if (!fNNodes)
		return;

	// vectors of the tridiagonal matrix
	std::vector<Double_t> Au(fNNodes - 1); //upper diagonal
	std::vector<Double_t> Ad(fNNodes); // main diagonal
	std::vector<Double_t> Al(fNNodes - 1); // lower diagonal

	// vector on right hand side
	std::vector<double> rhs(fNNodes);

	// setting up the matrix and right hand side of the equation system
	// for the parameters b[]
	for (int i = 1; i < fNNodes - 1; i++) {
		Al[i - 1] = 1.0 / 3.0 * (fXNodes[i] - fXNodes[i - 1]);
		Ad[i] = 2.0 / 3.0 * (fXNodes[i + 1] - fXNodes[i - 1]);
		Au[i] = 1.0 / 3.0 * (fXNodes[i + 1] - fXNodes[i]);
		rhs[i] = (fYNodes[i + 1] - fYNodes[i]) / (fXNodes[i + 1] - fXNodes[i])
				- (fYNodes[i] - fYNodes[i - 1]) / (fXNodes[i] - fXNodes[i - 1]);
	}

	// left boundary condition
	if (fBndLeft == CubicSpline::kFirstDeriv) {

		// c[0] = f', needs to be re-expressed in terms of b:
		// (2b[0]+b[1])(x[1]-x[0]) = 3 ((y[1]-y[0])/(x[1]-x[0]) - f')
		Ad[0] = 2.0 * (fXNodes[1] - fXNodes[0]);
		Au[0] = 1.0 * (fXNodes[1] - fXNodes[0]);
		rhs[0] = 3.0
				* ((fYNodes[1] - fYNodes[0]) / (fXNodes[1] - fXNodes[0])
						- fBndLeftVal);
	} else { // CubicSpline::kSecondDeriv (default)

		// 2*b[0] = f''
		Ad[0] = 2.;
		Au[0] = 0.;
		rhs[0] = fBndLeftVal;
	}

	// right boundary condition
	if (fBndRight == CubicSpline::kFirstDeriv) {

		// c[n-1] = f', needs to be re-expressed in terms of b:
		// (b[n-2]+2b[n-1])(fXNodes[n-1]-fXNodes[n-2])
		// = 3 (f' - (y[n-1]-y[n-2])/(x[n-1]-x[n-2]))
		Ad[fNNodes - 1] = 2. * (fXNodes[fNNodes - 1] - fXNodes[fNNodes - 2]);
		Al[fNNodes - 2] = 1. * (fXNodes[fNNodes - 1] - fXNodes[fNNodes - 2]);
		rhs[fNNodes - 1] =
				3.
						* (fBndRightVal
								- (fYNodes[fNNodes - 1] - fYNodes[fNNodes - 2])
										/ (fXNodes[fNNodes - 1]
												- fXNodes[fNNodes - 2]));
	} else { // CubicSpline::kSecondDeriv (default)

		// 2*b[n-1] = f''
		Ad[fNNodes - 1] = 2.0;
		Al[fNNodes - 2] = 0.0;
		rhs[fNNodes - 1] = fBndRightVal;
	}

	// solve the equation system to obtain the parameters b
	fSplineCoefB = solveTridiagonal(Al, Ad, Au, rhs);

	// calculate coefficients a and c based on b
	fSplineCoefA.resize(fNNodes);
	fSplineCoefC.resize(fNNodes);
	for (int i = 0; i < fNNodes - 1; i++) {

		fSplineCoefA[i] = 1. / 3. * (fSplineCoefB[i + 1] - fSplineCoefB[i])
				/ (fXNodes[i + 1] - fXNodes[i]);
		fSplineCoefC[i] = (fYNodes[i + 1] - fYNodes[i])
				/ (fXNodes[i + 1] - fXNodes[i])
				- 1. / 3. * (2. * fSplineCoefB[i] + fSplineCoefB[i + 1])
						* (fXNodes[i + 1] - fXNodes[i]);
	}

	// for left extrapolation coefficients
	m_b0 = (fLinearExtrapolation == false) ? fSplineCoefB[0] : 0.0;
	m_c0 = fSplineCoefC[0];

	// for the right extrapolation coefficients
	// f_{n-1}(x) = b*(x-x_{n-1})^2 + c*(x-x_{n-1}) + y_{n-1}
	double h = fXNodes[fNNodes - 1] - fXNodes[fNNodes - 2];
	// b[n-1] is determined by the boundary condition
	fSplineCoefA[fNNodes - 1] = 0.0;
	fSplineCoefC[fNNodes - 1] = 3.0 * fSplineCoefA[fNNodes - 2] * h * h
			+ 2.0 * fSplineCoefB[fNNodes - 2] * h + fSplineCoefC[fNNodes - 2]; // = f'_{n-2}(x_{n-1})
	if (fLinearExtrapolation == true)
		fSplineCoefB[fNNodes - 1] = 0.0;
}

////////////////////////////////////////////////////////////////////////
/// Interpolation.
/// Evaluates the spline at a given argument.
/// \param[in] x the argument at which the spline is evaluated.
/// \return    the interpolated value
double CubicSpline::operator()(double x) const {

	// find the closest point fXNodes[idx] < x, idx=0 even if x<fXNodes[0]
//	std::vector<Double_t>::const_iterator it;
//	it = std::lower_bound(fXNodes.begin(), fXNodes.end(), x);
//	Int_t idx = std::max(int(it - fXNodes.begin()) - 1, 0);

	Int_t idx = int((x - fXNodes[0]) / (fXNodes[1] - fXNodes[0]));
	Double_t h = x - fXNodes[idx];

	if (x < fXNodes[0]) {

		// extrapolation to the left
		return (m_b0 * h + m_c0) * h + fYNodes[0];
	} else if (x > fXNodes[fNNodes - 1]) {

		// extrapolation to the right
		return (fSplineCoefB[fNNodes - 1] * h + fSplineCoefC[fNNodes - 1]) * h
				+ fYNodes[fNNodes - 1];
	} else {

		// interpolation
		return ((fSplineCoefA[idx] * h + fSplineCoefB[idx]) * h
				+ fSplineCoefC[idx]) * h + fYNodes[idx];
	}
}

////////////////////////////////////////////////////////////////////////
/// Derivative interpolation.
/// Evaluates the derivative of the spline at a given argument.
/// \param[in] x the argument at which the spline is evaluated.
/// \param[in] order the order of the derivative.
/// \return    the interpolated value
double CubicSpline::operator()(double x, int order) const {

	size_t n = fXNodes.size();
	// find the closest point fXNodes[idx] < x, idx=0 even if x<fXNodes[0]
	std::vector<double>::const_iterator it;
	it = std::lower_bound(fXNodes.begin(), fXNodes.end(), x);
	int idx = std::max(int(it - fXNodes.begin()) - 1, 0);

	double h = x - fXNodes[idx];
	double interpol;
	if (x < fXNodes[0]) {
		// extrapolation to the left
		switch (order) {
		case 1:
			interpol = 2.0 * m_b0 * h + m_c0;
			break;
		case 2:
			interpol = 2.0 * m_b0 * h;
			break;
		default:
			interpol = 0.0;
			break;
		}
	} else if (x > fXNodes[n - 1]) {
		// extrapolation to the right
		switch (order) {
		case 1:
			interpol = 2.0 * fSplineCoefB[n - 1] * h + fSplineCoefC[n - 1];
			break;
		case 2:
			interpol = 2.0 * fSplineCoefB[n - 1];
			break;
		default:
			interpol = 0.0;
			break;
		}
	} else {
		// interpolation
		switch (order) {
		case 1:
			interpol = (3.0 * fSplineCoefA[idx] * h + 2.0 * fSplineCoefB[idx])
					* h + fSplineCoefC[idx];
			break;
		case 2:
			interpol = 6.0 * fSplineCoefA[idx] * h + 2.0 * fSplineCoefB[idx];
			break;
		case 3:
			interpol = 6.0 * fSplineCoefA[idx];
			break;
		default:
			interpol = 0.0;
			break;
		}
	}
	return interpol;
}


void CubicSpline::test() {

//    Eigen::internal::BandMatrix<Double_t> A(n,n,1,1); // system matrix init with zeros
//    Eigen::VectorXd x;	// vector of unknowns
//    Eigen::VectorXd b(n); // right hand side vector
//
//    A.diagonal(0).setConstant(2.);
//    A.diagonal(-1).setConstant(1.);
//    A.diagonal(1).setConstant(1.);
//    A.diagonal(1)(2)= 5.;
//    A.diagonal(-1)(0)= 7.;
//
//    b.setConstant(5.);
//    x = A.toDenseMatrix().colPivHouseholderQr().solve(b);
//
//    std::cout << A.toDenseMatrix() << std::endl;
//    std::cout << b << std::endl;
//    std::cout << x << std::endl;


    std::vector<double> A2d(5, 2);
    std::vector<double> A2l(4, 1);
    std::vector<double> A2u(4, 1);
    std::vector<double> b2(5, 5);

    A2l[0] = 7.;
    A2u[2] = 5.;

    for (double val: A2d)
    	std::cout << val << std::endl;
    std::cout << "----------" << std::endl;
    for (double val: A2l)
    	std::cout << val << std::endl;
    std::cout << "----------" << std::endl;
    for (double val: A2u)
    	std::cout << val << std::endl;
    std::cout << "----------" << std::endl;
    for (double val: b2)
    	std::cout << val << std::endl;
    std::cout << "----------" << std::endl;

    std::vector<double> x2 = solveTridiagonal(A2l, A2d, A2u, b2);
    for (double val: x2)
    	std::cout << val << std::endl;

//    std::vector<Double_t> x1 = {4,3,2,1};
//    std::vector<Double_t> y1 = {9,8,1,2};
//    std::vector<Double_t> x2;
//    std::vector<Double_t> y2;
//
//
//	std::vector<size_t> idx(x1.size());
//	std::iota(idx.begin(), idx.end(), 0);
//	std::sort(idx.begin(), idx.end(),
//			[&x1](size_t i1, size_t i2) { return x1[i1] < x1[i2]; }
//	);
//
//	// sort points according to the index vector
//	x2.resize(x1.size());
//	y2.resize(x1.size());
//	std::transform(idx.begin(), idx.end(), x2.begin(),
//			[&x1](size_t i) { return x1[i]; }	);
//	std::transform(idx.begin(), idx.end(), y2.begin(),
//			[&y1](size_t i) { return y1[i]; }	);
//
//    std::cout << "idx----------" << std::endl;
//    for (double val: idx)
//    	std::cout << val << std::endl;
//    std::cout << "x2----------" << std::endl;
//    for (double val: x2)
//    	std::cout << val << std::endl;
//    std::cout << "y2----------" << std::endl;
//	for (double val: y2)
//		std::cout << val << std::endl;
}


} // namespace XBOX


