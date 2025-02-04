#include "bmp.h"
#include <pthread.h>  // Necesario para hilos
#include <string.h>   // Para memcpy
typedef struct {
    BMP_Image *imageIn;
    BMP_Image *imageOut;
    int startRow;
    int endRow;
    int filter[3][3];
} ThreadData;

void apply(BMP_Image * imageIn, BMP_Image * imageOut);

void applyParallel(BMP_Image *imageIn, BMP_Image *imageOut, int boxFilter[3][3], int numThreads);
void applyParallelRange(BMP_Image *imageIn, BMP_Image *imageOut, int boxFilter[3][3], int numThreads, int startRow, int endRow);


void *filterThreadWorker(void * args);