#include <string.h>
#include <stdlib.h>  
#include <pthread.h>
#include "filter.h"

void apply(BMP_Image *imageIn, BMP_Image *imageOut) {
    printf("Aplicando filtro de desenfoque a toda la imagen...\n");
    fflush(stdout);

    int filter[3][3] = {
        {1, 1, 1},
        {1, 1, 1},
        {1, 1, 1}
    };

    for (int i = 1; i < imageIn->norm_height - 1; i++) {
        for (int j = 1; j < imageIn->header.width_px - 1; j++) {
            int sumRed = 0, sumGreen = 0, sumBlue = 0;

            for (int k = -1; k <= 1; k++) {
                for (int l = -1; l <= 1; l++) {
                    sumRed   += imageIn->pixels[i + k][j + l].red * filter[k + 1][l + 1];
                    sumGreen += imageIn->pixels[i + k][j + l].green * filter[k + 1][l + 1];
                    sumBlue  += imageIn->pixels[i + k][j + l].blue * filter[k + 1][l + 1];
                }
            }

            imageOut->pixels[i][j].red   = sumRed / 9;
            imageOut->pixels[i][j].green = sumGreen / 9;
            imageOut->pixels[i][j].blue  = sumBlue / 9;

            if (i % 100 == 0 && j % 100 == 0) { 
                printf("Pixel[%d][%d] -> R: %d, G: %d, B: %d\n",
                       i, j, imageOut->pixels[i][j].red,
                       imageOut->pixels[i][j].green, imageOut->pixels[i][j].blue);
                fflush(stdout);
            }
        }
    }

    printf("Filtro aplicado correctamente.\n");
    fflush(stdout);
}

void *filterThreadWorker(void *args) {
    ThreadData *data = (ThreadData *)args;
    BMP_Image *imageIn = data->imageIn;
    BMP_Image *imageOut = data->imageOut;
    int (*filter)[3] = data->filter;
    if (!imageIn || !imageIn->pixels) {
    printf("Error: `imageIn` o `imageIn->pixels` es NULL en `filterThreadWorker()`\n");
    return NULL;
}
    for (int i = data->startRow; i < data->endRow; i++) {
        for (int j = 1; j < imageIn->header.width_px - 1; j++) {
            int sumRed = 0, sumGreen = 0, sumBlue = 0;
            
            for (int k = -1; k <= 1; k++) {
                for (int l = -1; l <= 1; l++) {
                    if ((i + k) >= 0 && (i + k) < imageIn->norm_height &&
    (j + l) >= 0 && (j + l) < imageIn->header.width_px) {
    sumRed += imageIn->pixels[i+k][j+l].red * filter[k+1][l+1];
    sumGreen += imageIn->pixels[i+k][j+l].green * filter[k+1][l+1];
    sumBlue += imageIn->pixels[i+k][j+l].blue * filter[k+1][l+1];
}
                }
            }
            
            imageOut->pixels[i][j].red = sumRed / 9;
            imageOut->pixels[i][j].green = sumGreen / 9;
            imageOut->pixels[i][j].blue = sumBlue / 9;
        }
    }
    return NULL;
}

void applyParallel(BMP_Image *imageIn, BMP_Image *imageOut, int boxFilter[3][3], int numThreads) {
    applyParallelRange(imageIn, imageOut, boxFilter, numThreads, 0, imageIn->norm_height);
}

// 🔹 `applyParallelRange()` para `desenfocador.c` y `realzador.c`
void applyParallelRange(BMP_Image *imageIn, BMP_Image *imageOut, int boxFilter[3][3], int numThreads, int startRow, int endRow) {
    if (startRow < 0 || endRow > imageIn->norm_height || startRow >= endRow) {
        printf("Error: Rango de filas inválido para procesamiento. startRow: %d, endRow: %d\n", startRow, endRow);
        return;
    }
    pthread_t threads[numThreads];
    ThreadData threadData[numThreads];
    int rowsToProcess = endRow - startRow;
    int rowsPerThread = rowsToProcess / numThreads;
    
    for (int i = 0; i < numThreads; i++) {
        threadData[i].imageIn = imageIn;
        threadData[i].imageOut = imageOut;
        threadData[i].startRow = startRow + (i * rowsPerThread);
        threadData[i].endRow = (i == numThreads - 1) ? endRow : startRow + ((i + 1) * rowsPerThread);
        memcpy(threadData[i].filter, boxFilter, sizeof(int) * 9);
        pthread_create(&threads[i], NULL, filterThreadWorker, &threadData[i]);
    }
    
    for (int i = 0; i < numThreads; i++) {
        pthread_join(threads[i], NULL);
    }
}
