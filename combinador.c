#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"

int main(int argc, char **argv) {
    if (argc != 4) {
        printf("Uso: combinador <desenfoque.bmp> <realce.bmp> <imagen_final.bmp>\n");
        return EXIT_FAILURE;
    }

    // Abrir imágenes de entrada
    FILE *fileBlurred = fopen(argv[1], "rb");
    FILE *fileEdges = fopen(argv[2], "rb");
    if (!fileBlurred || !fileEdges) {
        printf("Error al abrir las imágenes procesadas.\n");
        return EXIT_FAILURE;
    }

    // Leer imágenes
    BMP_Image *imageBlurred = createBMPImage(fileBlurred);
    BMP_Image *imageEdges = createBMPImage(fileEdges);
    readImage(fileBlurred, imageBlurred);
    readImage(fileEdges, imageEdges);
    fclose(fileBlurred);
    fclose(fileEdges);

    // Crear imagen de salida
    BMP_Image *imageFinal = (BMP_Image *)malloc(sizeof(BMP_Image));
    if (!imageFinal) {
        printf("Error al asignar memoria para la imagen final.\n");
        freeImage(imageBlurred);
        freeImage(imageEdges);
        return EXIT_FAILURE;
    }

    // Copiar encabezado y estructura
    imageFinal->header = imageBlurred->header;
    imageFinal->norm_height = imageBlurred->norm_height;
    imageFinal->bytes_per_pixel = imageBlurred->bytes_per_pixel;

    imageFinal->pixels = (Pixel **)malloc(imageFinal->norm_height * sizeof(Pixel *));
    for (int i = 0; i < imageFinal->norm_height; i++) {
        imageFinal->pixels[i] = (Pixel *)malloc(imageFinal->header.width_px * sizeof(Pixel));
    }

    // Copiar la primera mitad desde `imageBlurred` y la segunda desde `imageEdges`
    for (int i = 0; i < imageFinal->norm_height / 2; i++) {
        for (int j = 0; j < imageFinal->header.width_px; j++) {
            imageFinal->pixels[i][j] = imageBlurred->pixels[i][j];
        }
    }

    for (int i = imageFinal->norm_height / 2; i < imageFinal->norm_height; i++) {
        for (int j = 0; j < imageFinal->header.width_px; j++) {
            imageFinal->pixels[i][j] = imageEdges->pixels[i][j];
        }
    }

    // Guardar imagen final
    writeImage(argv[3], imageFinal);

    // Liberar memoria
    freeImage(imageBlurred);
    freeImage(imageEdges);
    freeImage(imageFinal);

    printf("Imagen combinada guardada en %s\n", argv[3]);
    return EXIT_SUCCESS;
}
