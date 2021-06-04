#include <cstring>
#include <stdlib.h>

#include <iostream>
#include <cstdlib>
#include <string>
#include <ctime>
#include <stdio.h>
#include <sstream>

// root core
#include "Rtypes.h"

// root graphics
#include "TApplication.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TImage.h"
//#include "TColor.h"

#include "TRandom.h"
#include "TRandom1.h"


//#define EIGEN
#ifdef EIGEN
#include <Eigen/Core>
#include <Eigen/Dense>
#else
#include "TMatrixD.h"
#include "TVectorD.h"
#include "TDecompQRH.h"
#endif

/** \brief A set of Savitzky-Golay filter coefficients.
  * \param nl number of leftward (past) data points.
  * \param nr number of rightward (future) data points.
  * \param m polynomial order
  * \param ld order of the derivative desired
  * \return vector of coefficients
  *
  *
  * Returns in c[0..np-1], in wraparound order (N.B.!) consistent
  * with the argument response in routine convlv, a set of Savitzky-Golay
  * filter coefficients. For the derivative of order k, you must multiply the
  * array c by kŠ.) m is the order of the smoothing polynomial, also equal to
  * the highest conserved moment; usual values are m D 2 or m D 4.
  */
std::vector<Double_t> savgol(const Int_t m, const Int_t nl, const Int_t nr, const Int_t ld=0)
{
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


void test_absolutefunction(const std::string &sfile)
{
	std::vector<Double_t> test_x(100);
	std::vector<Double_t> test_f(100);
	TRandom *r1 = new TRandom1();
	for(Int_t i=0; i<100; i++){
		test_x[i] = 0.01*float(i);
		r1->Gaus(0,1);
		test_f[i] = fabs(0.01*float(i) - 0.5) + r1->Gaus(0,0.05);
	}
	Double_t offset = test_f[0];
	for(Int_t i=0; i<100; i++){
		test_f[i] -= offset;
	}

	std::vector<Double_t> kernel = savgol(2, 10, 10, 1);
	std::reverse(kernel.begin(),kernel.end()); // reverse order of coefficients
	std::vector<Double_t> test_ff = conv(test_f, kernel, 10, 10);
	for(Int_t i=0; i<100; i++){
		test_ff[i] *= 100.;
	}

	printf("Coefficients:\t");
	for(Double_t c: kernel)
		printf("%.3f ", c);
	printf("\n");

	TCanvas *c1 = new TCanvas("c1","A Simple Graph Example",200,10,700,500);
	// c1->SetFillColor(42);
	c1->SetGrid();


	TMultiGraph *mg = new TMultiGraph();
	TGraph *gr[2];

	gr[0] = new TGraph(test_x.size(), &test_x[0], &test_f[0]);
	gr[1] = new TGraph(test_x.size(), &test_x[0], &test_ff[0]);

	gr[0]->SetLineColor(kRed);
	gr[0]->SetLineWidth(1);
	gr[0]->SetMarkerColor(kRed);

	gr[1]->SetLineColor(kBlue);
	gr[1]->SetLineWidth(1);
	gr[1]->SetMarkerColor(kBlue);

	mg->Add(gr[0]);
	mg->Add(gr[1]);

	mg->SetTitle("XY PLOT");
	mg->GetXaxis()->SetTitle("Time [us]");
	mg->GetYaxis()->SetTitle("Amplitude");
	mg->Draw("ACP");

	// TCanvas::Update() draws the frame, after which one can change it
	c1->Update();
	// c1->GetFrame()->SetFillColor(21);
	// c1->GetFrame()->SetBorderSize(12);
	c1->Modified();

	TImage *img = TImage::Create();
	img->FromPad(c1);

	img->WriteImage(sfile.c_str());

	delete img;
	delete gr[0];
	delete gr[1];
	delete mg;
	delete c1;
}

int main(int argc, char* argv[]) {

	test_absolutefunction("test.png");

}


