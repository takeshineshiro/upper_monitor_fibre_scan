#ifndef _FILTER_H
#define _FILTER_H

#include <vector>
#include <string>
#include <complex>

using namespace std;
struct CvMat;

namespace mmode
{

#define  PI 3.1415926f

class Filter
{
private:
	int		steps;
	vector<float>param;

public:
	Filter(int step = 0);
	~Filter() {}

	void doFilter(const float *input, float *output, int length);

	int fft(float *data, complex <float> *a, int L);

	int ifft(complex <float> *a, float *data, int L);

	void hilbert(const std::string &filename, const float *data , float *filterdata, int dn);

	void grayImage(const std::string &filename, CvMat *pmat, int db);

	bool setFilterParam(const string &filename);
};

}

#endif