#include "bmp.h"

void applyParallelSecondHalfEdge(BMP_Image *imageIn, BMP_Image *imageOut, int numThreads);

void *edgeDetectionThreadWorker(void *args);
