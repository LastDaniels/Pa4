#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"
#include "filter.h"


int main(int argc, char **argv) {
    if (argc != 4) {
        printError(ARGUMENT_ERROR);
        return EXIT_FAILURE;
    }

    int numThreads = atoi(argv[3]);
    if (numThreads <= 0) {
        printf("Error: El número de hilos debe ser mayor a 0.\n");
        return EXIT_FAILURE;
    }

    FILE *source = fopen(argv[1], "rb");
    if (!source) {
        printError(FILE_ERROR);
        return EXIT_FAILURE;
    }

    BMP_Image *imageIn = createBMPImage(source);
    if (!imageIn) {
        fclose(source);
        return EXIT_FAILURE;
    }

    if (!checkBMPValid(&imageIn->header)) {
        printError(VALID_ERROR);
        freeImage(imageIn);
        fclose(source);
        return EXIT_FAILURE;
    }

    readImage(source, imageIn);
    fclose(source);

    BMP_Image *imageOut = (BMP_Image *)malloc(sizeof(BMP_Image));
    if (!imageOut) {
        printf("Error al asignar memoria para la imagen de salida.\n");
        freeImage(imageIn);
        return EXIT_FAILURE;
    }

    imageOut->header = imageIn->header;
    imageOut->norm_height = imageIn->norm_height;
    imageOut->bytes_per_pixel = imageIn->bytes_per_pixel;

    imageOut->pixels = (Pixel **)malloc(imageOut->norm_height * sizeof(Pixel *));
if (!imageOut->pixels) {
    printf("Error: No se pudo asignar memoria para filas de píxeles.\n");
    freeImage(imageIn);
    free(imageOut);
    return EXIT_FAILURE;
}

for (int i = 0; i < imageOut->norm_height; i++) {
    imageOut->pixels[i] = (Pixel *)malloc(imageOut->header.width_px * sizeof(Pixel));
    if (!imageOut->pixels[i]) {
        for (int j = 0; j < i; j++) free(imageOut->pixels[j]);
        free(imageOut->pixels);
        freeImage(imageIn);
        free(imageOut);
        printf("Error: No se pudo asignar memoria para la fila %d.\n", i);
        return EXIT_FAILURE;
    }
}

    int boxFilter[3][3] = {
        {1, 1, 1},
        {1, 1, 1},
        {1, 1, 1}
    };

    printf("Aplicando filtro de desenfoque a la primera mitad de la imagen...\n");
    fflush(stdout);

    applyParallelRange(imageIn, imageOut, boxFilter, numThreads, 0, imageIn->norm_height / 2);

    printf("Guardando imagen de salida en %s...\n", argv[2]);
    fflush(stdout);
    writeImage(argv[2], imageOut);

    freeImage(imageIn);
    freeImage(imageOut);

    printf("Finalizando programa exitosamente.\n");
    return EXIT_SUCCESS;
}
