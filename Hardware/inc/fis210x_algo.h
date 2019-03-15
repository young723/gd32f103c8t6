#ifndef FIS210X_ALGO_H
#define FIS210X_ALGO_H

#include "gd32f10x.h"

void fis210x_algo_init(void);
void fis210x_algo_fusion_process(float dq[4],float rpy[3]);
uint32_t fis210x_algo_GetMotion(void);

#endif

