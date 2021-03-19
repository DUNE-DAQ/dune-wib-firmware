#ifndef diagnostics_h
#define diagnostics_h

#include "unpack.h"
#include "wib_3asic.h"

template <typename T>
double mean(std::vector<T> vec) {
    double a = 0.0;
    for (size_t i = 0; i < vec.size(); i++) a += vec[i];
    return a / vec.size();
}

template <typename T>
double stdev(std::vector<T> vec) {
    double m = mean(vec);
    double a = 0.0;
    for (size_t i = 0; i < vec.size(); i++) a += pow(vec[i]-mean,2.0);
    return sqrt(a / vec.size());
}

bool acquire_data(WIB &w, const bool femb_mask[4], channel_data &dch0, channel_data &dch1);

bool check_test_pattern(WIB_3ASIC &w, const bool femb_mask[4], bool cold);

#endif
