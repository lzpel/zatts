//
// Created by misumi3104 on 2019/06/19.
//

#ifndef TEST_FFTSG_H
#define TEST_FFTSG_H

#include <math.h>
#include <float.h>
#include <cstring>

#ifndef MIN
#define MIN(a, b) (a)>(b)?(b):(a)
#endif
#ifndef MAX
#define MAX(a, b) (a)>(b)?(a):(b)
#endif

void cdft(int, int, double *, int *, double *);

void rdft(int, int, double *, int *, double *);

void ddct(int, int, double *, int *, double *);

void ddst(int, int, double *, int *, double *);

void dfct(int, double *, double *, int *, double *);

void dfst(int, double *, double *, int *, double *);

class FFT {
public:
	double *w;
	signed size;
	signed *ip;
	double *buf0,*buf1,*buf2;

	FFT(int s) {
		size = s;
		ip = new int[2 + (int) sqrt(s / 2)];
		w = new double[s / 2];
		ip[0] = 0;
		alloc(buf0);
		alloc(buf1);
		alloc(buf2);
	}

	~FFT() {
		delete[] ip;
		delete[] w;
		free(buf0);
		free(buf1);
		free(buf2);
	}

	void rdft(double *a) {
		//実数離散フーリエ変換
		::rdft(size, 1, a, ip, w);
		//振幅に変換
		for (int j = 0; j < size; j++) a[j] *= 2.0 / size;
	}

	void irdft(double *a) {
		::rdft(size, -1, a, ip, w);
	}

	double power(double *d, bool amp) {
		double s=0,a=0;
		if(amp){
			for (int i = 1; i < size; ++i)s+=d[i]*d[i]*size/2.0;
		}else{
			for (int i = 0; i < size; ++i)a+=d[i]/size;
			for (int i = 0; i < size; ++i)s+=(d[i]-a)*(d[i]-a);
		}
		return s;
	}

	void alloc(double *&p) {
		alloc(p, size);
	}

	static void alloc(double *&p, int s) {
		zero(p = new double[s],s);
	}

	static void free(double *p) {
		delete[] p;
	}

	void copy(double *into, const double *from) {
		copy(into, from, size);
	}

	static void copy(double *into, const double *from, int len) {
		for (int i = 0; i < len; ++i)into[i] = from[i];
	}

	static void interpolate(double *into, int intolen, const double *from, int fromlen) {
		if (intolen > fromlen) {
			for (int i = 0; i < intolen; ++i) {
				double pos = 1.0 * fromlen * i / intolen;
				into[i] = from[int(pos)] * ((int) pos + 1 - pos) + from[int(pos) + 1] * (pos - (int) pos);
			}
		} else {
			for (int i = 0; i < intolen; ++i) {
				into[i] = from[fromlen * i / intolen];
			}
		}
	}

	void window(double *p) {
		for (int i = 0; i < size; ++i)p[i] *= 1 - cos(2 * M_PI * i / size);
	}

	void print(const char *fn, const double *p) {
		print(fn, p, size);
	}

	static void print(const char *fn, const double *p, int len) {
		const char *fmt = "%f\n";
		if (fn) {
			FILE *f = fopen(fn, "w");
			for (int i = 0; i < len; i++)fprintf(f, fmt, p[i]);
			fclose(f);
		} else {
			for (int i = 0; i < len; i++)printf(fmt, p[i]);
		}
	}

	void spectrum_amp(double *w) {
		//位相なし振幅スペクトルに変換
		w[0] = w[1] = 0;
		for (int i = 1; i < size / 2; ++i) w[i] = w[i * 2] * w[i * 2] + w[i * 2 + 1] * w[i * 2 + 1];
		for (int i = 0; i < size / 2; ++i) {
			w[i + size / 2] = 0;
			w[i] = w[i] ? sqrt(w[i]) : 0;
		}
	}

	void spectrum_log(double *w) {
		//位相なし対数振幅スペクトルに変換
		spectrum_amp(w);
		for (int i = 0; i < size / 2; ++i) w[i] = w[i]?log(w[i]):0;
	}

	void spectrum_log_periodseparate(double*periodicity,double*aperiodicity,double *spectrum, int n) {
		//雑に0hz振幅の補正
		spectrum[0]=spectrum[1];
		//対数振幅スペクトルで正弦波という仮定で周期性・非周期性の対数振幅スペクトルに分離
		for(int i=0;i<size/2;i++){
			double average=0,variance=0,amplitude=0;
			for(int j=i;j<i+n;j++)average+=spectrum[j]/n;
			for(int j=i;j<i+n;j++)variance+=(spectrum[j]-average)*(spectrum[j]-average)/n;
			amplitude=sqrt(variance*2);
			periodicity[i+(n-1)/2]=average+amplitude;
			aperiodicity[i+(n-1)/2]=average-amplitude;
		}
		//雑に0hz振幅の補正
		for(int i=0;i<(n-1)/2;i++){
			periodicity[i]=periodicity[(n-1)/2];
			aperiodicity[i]=aperiodicity[(n-1)/2];
		}
		spectrum[0]=0;
	}

	void zero(double *t) {
		zero(t, size);
	}

	static void zero(double *t, int s) {
		for (int i = 0; i < s; i++)t[i] = 0;
	}

	void white(double *d) {
		white(d, size);
	}

	static void white(double *d, int n) {
		for (int i = 0; i < n / 2; i++) {
			double r1 = (double) rand() / RAND_MAX, r2 = (double) rand() / RAND_MAX;
			d[i * 2 + 0] = sqrt(-2 * log(r1)) * cos(2 * M_PI * r2);
			d[i * 2 + 1] = sqrt(-2 * log(r1)) * sin(2 * M_PI * r2);
		}
	}

	void conv(double *d, double *s, const int sn, const double *f, bool fh) {
		conv(d, s, sn, f, fh, size);
	}

	static void conv(double *d, double *s, const int sn, const double *f, bool fh, const int fn) {
		for (int i = 0; i < sn - fn; ++i) {
			double sum = 0;
			for (int j = 0; j < fn; ++j)sum += s[i + j] * f[j];
			d[i] = sum;
		}
		for (int i = sn - fn; i < sn; ++i)d[i] = 0;
		if (fh) {
			for (int i = sn - 1; i >= fn / 2; --i)d[i] = d[i - fn / 2];
			for (int i = fn / 2 - 1; i >= 0; --i)d[i] = 0;
		}
	}

	static inline double sinc(int i, int len) {
		//i%(len/2)==0の時return0;
		return (i == 0) ? (2.0 / len) : sin(i * M_PI * 2.0 / len) / (i * M_PI);
	}

	void fir(double *p, int lenmin, int lenmax) {
		fir(p, lenmin, lenmax, size);
	}

	static void fir(double *p, const int lenmax, const int lenmin, const int order) {
		//本当はi<=orderにして対称性が欲しいがフィルタ次数を奇数にしたくない。
		if (lenmax) {
			for (int i = 0; i < order; ++i)p[i] = sinc(i - order / 2, lenmax);
		} else {
			for (int i = 0; i < order; ++i)p[i] = (i == order / 2) ? 1 : 0;
		}
		if (lenmin) {
			for (int i = 0; i < order; i++)p[i] -= sinc(i - order / 2, lenmin);
		}
	}

	static signed zerocrosslen(int lenmin, int lenmax, double *p) {
		for (int i = 0; i < lenmax / 2; ++i) {
			if (p[i] * p[i + 1] < 0) {
				for (int j = i + lenmin; j < i + lenmax; j++) {
					if ((p[j] * p[j + 1] < 0) && (p[i] * p[j] > 0))return j - i;
				}
			}
		}
		return 0;
	}
};

#endif //TEST_WAVE_H
