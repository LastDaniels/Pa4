#include <stdlib.h>
#include <stdio.h>

#include "bmp.h"
/* USE THIS FUNCTION TO PRINT ERROR MESSAGES
   DO NOT MODIFY THIS FUNCTION
*/

typedef struct {
    BMP_Image *imageIn;
    BMP_Image *imageOut;
    int startRow;
    int endRow;
    int filter[3][3];
} ThreadData;

void printError(int error){
  switch(error){
  case ARGUMENT_ERROR:
    printf("Usage:ex5 <source> <destination>\n");
    break;
  case FILE_ERROR:
    printf("Unable to open file!\n");
    break;
  case MEMORY_ERROR:
    printf("Unable to allocate memory!\n");
    break;
  case VALID_ERROR:
    printf("BMP file not valid!\n");
    break;
  default:
    break;
  }
}

/* The input argument is the source file pointer. The function will first construct a BMP_Image image by allocating memory to it.
 * Then the function read the header from source image to the image's header.
 * Compute data size, width, height, and bytes_per_pixel of the image and stores them as image's attributes.
 * Finally, allocate menory for image's data according to the image size.
 * Return image;
*/

BMP_Image* createBMPImage(FILE* fptr) { 
    printf("Entrando a createBMPImage()...\n");
    fflush(stdout);

    BMP_Image *image = (BMP_Image *)malloc(sizeof(BMP_Image));
    if (!image) {
        printError(MEMORY_ERROR);
        printf("Error: No se pudo asignar memoria para BMP_Image.\n");
        fflush(stdout);
        return NULL;
    }

    printf("Memoria asignada para BMP_Image.\n");
    fflush(stdout);

    // Leer el encabezado del archivo BMP
    fread(&image->header, sizeof(BMP_Header), 1, fptr);
    printf("Encabezado BMP leído.\n");
    printf("Dimensiones: %d x %d, Bits por píxel: %d\n",
           image->header.width_px, image->header.height_px, image->header.bits_per_pixel);
    fflush(stdout);

    // Validar si el BMP es correcto
    if (!checkBMPValid(&image->header)) {
        printf("Error: BMP no válido.\n");
        fflush(stdout);
        free(image);
        return NULL;
    }

    printf("BMP válido.\n");
    fflush(stdout);

    image->norm_height = abs(image->header.height_px);
    image->bytes_per_pixel = image->header.bits_per_pixel / 8;

    // Asignar memoria para la matriz de píxeles
    image->pixels = (Pixel **)malloc(image->norm_height * sizeof(Pixel *));
    if (!image->pixels) {
        printError(MEMORY_ERROR);
        printf("Error: No se pudo asignar memoria para las filas de píxeles.\n");
        fflush(stdout);
        free(image);
        return NULL;
    }

    printf("Memoria asignada para filas de píxeles.\n");
    fflush(stdout);

    for (int i = 0; i < image->norm_height; i++) {
        image->pixels[i] = (Pixel *)malloc(image->header.width_px * sizeof(Pixel));
        if (!image->pixels[i]) {
            for (int j = 0; j < i; j++) free(image->pixels[j]);
            free(image->pixels);
            free(image);
            printError(MEMORY_ERROR);
            printf("Error: No se pudo asignar memoria para los píxeles de la fila %d.\n", i);
            fflush(stdout);
            return NULL;
        }
    }

    printf("Memoria asignada para los píxeles de la imagen.\n");
    fflush(stdout);

    printf("Imagen BMP creada correctamente.\n");
    fflush(stdout);

    return image;
}

void readImageData(FILE* srcFile, BMP_Image *image, int dataSize) {
    printf("Leyendo datos de la imagen BMP...\n");
    fflush(stdout);

    if (image->header.height_px > 0) {
        // BMP en formato "bottom-up" (las filas están invertidas, hay que leer desde abajo)
        printf("El BMP está en formato bottom-up, leyendo filas en orden invertido.\n");
        fflush(stdout);
        for (int i = image->norm_height - 1; i >= 0; i--) {
            fread(image->pixels[i], sizeof(Pixel), image->header.width_px, srcFile);
        }
    } else {
        // BMP en formato "top-down" (las filas ya están en el orden correcto)
        printf("El BMP está en formato top-down, leyendo filas en orden normal.\n");
        fflush(stdout);
        for (int i = 0; i < image->norm_height; i++) {
            fread(image->pixels[i], sizeof(Pixel), image->header.width_px, srcFile);
        }
    }

    printf("Imagen BMP leída correctamente.\n");
    fflush(stdout);
}

void readImage(FILE *srcFile, BMP_Image *image) {
    printf("Moviendo puntero a la posición de píxeles: %d bytes\n", image->header.offset);
    fflush(stdout);

    fseek(srcFile, image->header.offset, SEEK_SET);
    
    long pos = ftell(srcFile);
    printf("Posición actual del puntero después de fseek: %ld\n", pos);
    fflush(stdout);

    // Usar image_size en lugar de image_size_bytes
    readImageData(srcFile, image, image->header.imagesize);
}


void writeImage(char* destFileName, BMP_Image* image) {
    FILE *destFile = fopen(destFileName, "wb");
    if (!destFile) {
        printError(FILE_ERROR);
        return;
    }

    printf("Archivo de salida abierto correctamente: %s\n", destFileName); 

    fwrite(&image->header, sizeof(BMP_Header), 1, destFile);
    for (int i = 0; i < image->norm_height; i++) {
        fwrite(image->pixels[i], sizeof(Pixel), image->header.width_px, destFile);
    }

    fclose(destFile);
    printf("Imagen guardada correctamente.\n"); 
}

void freeImage(BMP_Image* image) {
    if (image) {
        for (int i = 0; i < image->norm_height; i++) {
            free(image->pixels[i]);
        }
        free(image->pixels);
        free(image);
    }
}



int checkBMPValid(BMP_Header* header) {
    printf("Verificando firma BMP...\n");
    fflush(stdout);

    // La firma "BM" en hexadecimal es 0x4D42 en little-endian
    if (header->type != 0x4D42) {
        printf("Error: Firma BMP inválida (0x%X)\n", header->type);
        fflush(stdout);
        return FALSE;
    }
    printf("Firma BMP válida.\n");
    fflush(stdout);

    printf("Verificando profundidad de color...\n");
    fflush(stdout);

    if (header->bits_per_pixel != 32) {
        printf("Error: El BMP no es de 32 bits, tiene %d bits por píxel.\n", header->bits_per_pixel);
        fflush(stdout);
        return FALSE;
    }
    printf("El BMP tiene 32 bits por píxel.\n");
    fflush(stdout);

    printf("Verificando número de planos de imagen...\n");
    fflush(stdout);

    if (header->planes != 1) {
        printf("Error: Número de planos inválido (%d).\n", header->planes);
        fflush(stdout);
        return FALSE;
    }
    printf("Número de planos válido.\n");
    fflush(stdout);

    printf("Verificando compresión...\n");
    fflush(stdout);

    if (header->compression != 0) {
        printf("Error: El BMP usa compresión (%d).\n", header->compression);
        fflush(stdout);
        return FALSE;
    }
    printf("El BMP no tiene compresión.\n");
    fflush(stdout);

    printf("El archivo BMP es válido.\n");
    fflush(stdout);

    return TRUE;
}



/* The function prints all information of the BMP_Header.
   DO NOT MODIFY THIS FUNCTION
*/
void printBMPHeader(BMP_Header* header) {
  printf("file type (should be 0x4d42): %x\n", header->type);
  printf("file size: %d\n", header->size);
  printf("offset to image data: %d\n", header->offset);
  printf("header size: %d\n", header->header_size);
  printf("width_px: %d\n", header->width_px);
  printf("height_px: %d\n", header->height_px);
  printf("planes: %d\n", header->planes);
  printf("bits: %d\n", header->bits_per_pixel);
}

/* The function prints information of the BMP_Image.
   DO NOT MODIFY THIS FUNCTION
*/
void printBMPImage(BMP_Image* image) {
  printf("data size is %zu\n", sizeof(image->pixels));
  printf("norm_height size is %d\n", image->norm_height);
  printf("bytes per pixel is %d\n", image->bytes_per_pixel);
}
