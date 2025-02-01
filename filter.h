#include "bmp.h"

void applyParallelFirstHalfBlur(BMP_Image *imageIn, BMP_Image *imageOut, int numThreads);

void *filterThreadWorker(void *args);