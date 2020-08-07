#ifndef CNN_H
#define CNN_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ap_int.h>

#define Tn 4
#define Tm 32
#define Tr 32
#define Tc 32
#define K 5
#define MAX_BETA_LENGTH 1024

typedef ap_int<8> int8;

void detection_acc(int* Input, int* Output, int* Weight, int* Beta, const int InFM_num, const int OutFM_num,
	const int Kernel_size, const int Kernel_stride, const int TM, const int TN, const int TR, const int TC,
	const int mLoops, const int nLoops, const int LayerType, int M);

#endif
