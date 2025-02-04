#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"
#include "filter.h"

#define NUM_THREADS 4  // Número de hilos para procesamiento paralelo

int main(int argc, char **argv) {
    // Verificar argumentos
    if (argc != 3) {
        printError(ARGUMENT_ERROR);
        return EXIT_FAILURE;
    }

    // Abrir archivo de entrada
    FILE *source = fopen(argv[1], "rb");
    if (!source) {
        printError(FILE_ERROR);
        return EXIT_FAILURE;
    }

    // Crear estructura BMP para la imagen de entrada
    printf("Creando imagen de entrada...\n");
    fflush(stdout);
    BMP_Image *imageIn = createBMPImage(source);
    if (!imageIn) {
        fclose(source);
        return EXIT_FAILURE;
    }

    // Validar el archivo BMP
    printf("Verificando si el BMP es válido...\n");
    fflush(stdout);
    if (!checkBMPValid(&imageIn->header)) {
        printError(VALID_ERROR);
        freeImage(imageIn);
        fclose(source);
        return EXIT_FAILURE;
    }
    printf("BMP válido.\n");
    fflush(stdout);

    // Leer los datos de los píxeles
    printf("Llamando a readImage() para leer los píxeles...\n");
    fflush(stdout);
    readImage(source, imageIn);
    printf("Imagen de entrada leída correctamente.\n");
    fflush(stdout);

    // Cerrar el archivo de entrada
    fclose(source);

    // 🔹 Crear imagen de salida copiando el encabezado de `imageIn`
    printf("Creando imagen de salida...\n");
    fflush(stdout);
    BMP_Image *imageOut = (BMP_Image *)malloc(sizeof(BMP_Image));
    if (!imageOut) {
        printf("Error al asignar memoria para la imagen de salida.\n");
        fflush(stdout);
        freeImage(imageIn);
        return EXIT_FAILURE;
    }

    // Copiar el encabezado desde la imagen de entrada
    imageOut->header = imageIn->header;
    imageOut->norm_height = imageIn->norm_height;
    imageOut->bytes_per_pixel = imageIn->bytes_per_pixel;

    // Asignar memoria para los píxeles de la imagen de salida
    imageOut->pixels = (Pixel **)malloc(imageOut->norm_height * sizeof(Pixel *));
    if (!imageOut->pixels) {
        printf("Error al asignar memoria para píxeles de salida.\n");
        fflush(stdout);
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
            printf("Error al asignar memoria para las filas de píxeles de salida.\n");
            fflush(stdout);
            return EXIT_FAILURE;
        }
    }
    printf("Imagen de salida creada correctamente.\n");
    fflush(stdout);

    printf("Aplicando filtro...\n");
    fflush(stdout);

    // 🔹 Si se quiere en paralelo, usa `applyParallel()`
    //applyParallel(imageIn, imageOut, NUM_THREADS);

    // 🔹 Si se quiere en serie, usa `apply()`
    apply(imageIn, imageOut);

    printf("Filtro aplicado correctamente.\n");
    fflush(stdout);

    // Guardar imagen de salida
    printf("Guardando imagen de salida en %s...\n", argv[2]);
    fflush(stdout);
    writeImage(argv[2], imageOut);
    printf("Imagen guardada correctamente.\n");
    fflush(stdout);

    // Liberar memoria
    freeImage(imageIn);
    freeImage(imageOut);

    printf("Finalizando programa exitosamente.\n");
    return EXIT_SUCCESS;
}
