#pragma once

#include <vector>
#include <numeric> // accumulate
#include <cmath> // floor, ceil


/**
 * Return a median of a range of ordered numeric values.
 * 
 * If the size of the data is even the median is the average
 * of the middle values. So the result is in general a floating
 * point number.
 */
template<typename T>
double median(const std::vector<T>& data, size_t from, size_t to) {
	size_t n = to - from + 1;
	if (n % 2 == 1) {
		return data[ from + n/2 ];
	}
	return (data[from + n/2] + data[from + n/2 - 1]) / double(2);
}


/**
 * Return a median of vector of ordered numeric values.
 * 
 * If the size of the data is even the median is the average
 * of the middle values. So the result is in general a floating
 * point number.
 */
template<typename T>
double median(const std::vector<T>& data) {
	return median(data, 0, data.size() - 1);
}


/**
 * Get an average of vector of ordered numeric values.
 */
template<typename T>
double average(const std::vector<T>& data) {
	return std::accumulate(data.begin(), data.end(), T(0)) / double(data.size());
}


/**
 * Compute the q-th quantile of vector of ordered numeric values.
 * 
 * It is computed as linear interpolation between the values at indices
 * `floor(q*n)` and `ceil(q*n)` where `n` is the size of the data.
 */
template<typename T>
double quantile(const std::vector<T>& data, double q) {
    size_t n = data.size() - 1;
	double h = q*n;
    size_t h0 = floor(h);
    size_t h1 = ceil(h);
    return data[h0] + (h - h0)*(data[h1] - data[h0]);
}
