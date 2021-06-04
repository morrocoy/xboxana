#include <cstdio>
#include <cstdlib>
#include <vector>
#include "spline.h"
#include "XboxCubicSpline.hxx"

int main(int argc, char** argv) {

   std::vector<double> X(5), Y(5);
   X[0]=0.1; X[1]=0.4; X[2]=1.2; X[3]=1.8; X[4]=2.0;
   Y[0]=0.1; Y[1]=0.7; Y[2]=0.6; Y[3]=1.1; Y[4]=0.9;

   XBOX::spline s;
   s.set_points(X,Y);

   XBOX::CubicSpline s2(X,Y);

   double x=1.5;

   printf("spline at %f is %f\n", x, s(x));
   printf("spline at %f is %f\n", x, s2(x));

   s2.test();

   return EXIT_SUCCESS;
}
