#ifndef CNN_H
#define CNN_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"
//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include "stb_image_write.h"

#define Tn 4
#define Tm 32
#define Tr 32
#define Tc 32
#define K 5
#define MAX_BETA_LENGTH 1024

typedef struct {
    int w;
    int h;
    int c;
    float* data;
} image;

void detection_acc(float* Input, float* Output, float* Weight, float* Beta, const int InFM_num, const int OutFM_num,
	const int Kernel_size, const int Kernel_stride, const int TM, const int TN, const int TR, const int TC,
	const int mLoops, const int nLoops, const int LayerType);

#endif