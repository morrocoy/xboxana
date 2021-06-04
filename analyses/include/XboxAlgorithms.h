/*
 * XboxAnalyser.cxx
 *
 *  Created on: Oct 31, 2018
 *      Author: kpapke
 */

#ifndef _XBOXALGORITHMS_H_
#define _XBOXALGORITHMS_H_

#include "Math/Math.h"

#ifndef XBOX_NO_NAMESPACE
namespace XBOX {
#endif


template <typename T>
inline std::vector<T> arange(T a, T b, T h) {
	size_t N = static_cast<Int_t>((b - a) / h);
    std::vector<T> xs(N);
    typename std::vector<T>::iterator x;
    T val;
    for (x = xs.begin(), val = a; x != xs.end(); ++x, val += h)
        *x = val;
    return xs;
}

template <typename T>
inline std::vector<T> linspace(T a, T b, size_t N) {
    T h = (b - a) / static_cast<T>(N-1);
    std::vector<T> xs(N);
    typename std::vector<T>::iterator x;
    T val;
    for (x = xs.begin(), val = a; x != xs.end(); ++x, val += h)
        *x = val;
    return xs;
}

inline Double_t wrap_phase(Double_t phase)
{
	phase = fmod(phase + M_PI, 2*M_PI);
    if (phase < 0)
    	phase += 2*M_PI;
    return phase - M_PI;
}

inline std::vector<Double_t> wrap_phase(const std::vector<Double_t> &phase)
{
	std::vector<Double_t> wrapped_phase = phase;
	for (size_t i = 1; i < phase.size(); i++) {
		wrapped_phase[i] = fmod(phase[i] + M_PI, 2*M_PI);
		if (wrapped_phase[i] < 0)
			wrapped_phase[i] += 2*M_PI;
		wrapped_phase[i] -= M_PI;
	}
    return wrapped_phase;
}

inline std::vector<Double_t> unwrap_phase(const std::vector<Double_t> &phase)
{
	std::vector<Double_t> unwrapped_phase = phase;
	for (size_t i = 1; i < phase.size(); i++) {
        float d = phase[i] - phase[i-1];
        d = d > M_PI ? d - 2 * M_PI : (d < -M_PI ? d + 2 * M_PI : d);

        unwrapped_phase[i] = unwrapped_phase[i-1] + d;
    }
    return unwrapped_phase;
}


#ifndef XBOX_NO_NAMESPACE
}
#endif


#endif /* _XBOXALGORITHMS_H_ */
