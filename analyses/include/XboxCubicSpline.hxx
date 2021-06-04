/*
 * spline.h
 *
 * simple cubic spline interpolation library without external
 * dependencies
 *
 * ---------------------------------------------------------------------
 * Copyright (C) 2011, 2014 Tino Kluge (ttk448 at gmail.com)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * ---------------------------------------------------------------------
 *
 */


#ifndef XBOXCUBICSPLINE_H
#define XBOXCUBICSPLINE_H


#include <vector>
#include "Rtypes.h"

#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif


// spline interpolation
class CubicSpline {

public:
    enum EBoundaryType {                              ///> Boundary Condition Type
        kFirstDeriv = 1,
        kSecondDeriv = 2
    };

private:

    Int_t                 fNNodes;                    ///> Number of nodes.
    std::vector<Double_t> fXNodes;                    ///> X coordinates of nodes
	std::vector<Double_t> fYNodes;                    ///> Y coordinates of nodes

    std::vector<Double_t> fSplineCoefA;               ///> Spline coefficient a.
    std::vector<Double_t> fSplineCoefB;               ///> Spline coefficient b.
    std::vector<Double_t> fSplineCoefC;               ///> Spline coefficient c.

    Double_t              m_b0;                       ///> Coefficient b0 for left extrapolation.
	Double_t              m_c0;                       ///> Coefficient c0 for left extrapolation.
    EBoundaryType         fBndLeft;                   ///> Boundary condition on the left.
    EBoundaryType         fBndRight;                  ///> Boundary condition on the right.
    Double_t              fBndLeftVal;                ///> Boundary value on the left.
    Double_t              fBndRightVal;               ///> Boundary value on the right.

    Bool_t                fLinearExtrapolation;       ///> A flag to force or unforce linear extrapolation.


    std::vector<Double_t> solveTridiagonal(
    		std::vector<Double_t> &a, std::vector<Double_t> &b,
			std::vector<Double_t> &c, std::vector<Double_t> &d);

public:

    CubicSpline(const std::vector<double>& x,
    		const std::vector<double>& y);

    virtual ~CubicSpline();

    void setNodes(const std::vector<double>& x,
    		const std::vector<double>& y);

    // optionally define boundary conditions
    void setBoundary(EBoundaryType bndleft, double bndleftval,
                      EBoundaryType bndright, double bndrightval,
                      bool bLinearExtrapolation=false);

    void evalCoefficients();
    double operator() (double x) const;
    double operator() (double x, int order) const;

    void test();
};

} // namespace XBOX


#endif /* XBOXSPLINE */
