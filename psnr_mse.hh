#ifndef __PSNR_MSE_H__
#define __PSNR_MSE_H__

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <climits>
#define MAX_SIZE 1024*968

//f1 1024 * 768 Y value ,  f2 1024 * 768 * 3 RGB value.
double get_mse(const unsigned char *f1, const unsigned char *f2, size_t n, long *p_error_count);
double get_psnr(double mse, double max);

void read_pgm(FILE* fpgm, unsigned char* f, size_t* p_len, unsigned int* p_max_val);

//f1 1024 * 768 Y value ,  f2 1024 * 768 * 3 RGB value.
void check_psnr(size_t size, int max_value, const unsigned char *f1, const unsigned char *f2, double *p_psnr, long *p_error_count);
#endif
