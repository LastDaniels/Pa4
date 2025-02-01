#include <stdlib.h>
#include <stdio.h>
#include "bmp.h"
#include <pthread.h>
#include <math.h>

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

BMP_Image* createBMPImage(FILE* fptr) {
  BMP_Image *image = (BMP_Image*)malloc(sizeof(BMP_Image));
  if (!image) return NULL;
  fread(&image->header, sizeof(BMP_Header), 1, fptr);
  image->norm_height = abs(image->header.height_px);
  image->bytes_per_pixel = image->header.bits_per_pixel / 8;
  int dataSize = image->header.width_px * image->norm_height * image->bytes_per_pixel;
  image->pixels = (Pixel**)malloc(image->norm_height * sizeof(Pixel*));
  for (int i = 0; i < image->norm_height; i++) {
    image->pixels[i] = (Pixel*)malloc(image->header.width_px * sizeof(Pixel));
  }
  return image;
}

void readImageData(FILE* srcFile, BMP_Image * image, int dataSize) {
  for (int y = 0; y < image->norm_height; y++) {
    fread(image->pixels[y], sizeof(Pixel), image->header.width_px, srcFile);
  }
}

void readImage(FILE *srcFile, BMP_Image * dataImage) {
  if (!dataImage) return;
  readImageData(srcFile, dataImage, dataImage->header.width_px * dataImage->norm_height * dataImage->bytes_per_pixel);
}

void writeImage(char* destFileName, BMP_Image* dataImage) {
  FILE *destFile = fopen(destFileName, "wb");
  if (!destFile) {
    printError(FILE_ERROR);
    return;
  }
  fwrite(&dataImage->header, sizeof(BMP_Header), 1, destFile);
  for (int y = 0; y < dataImage->norm_height; y++) {
    fwrite(dataImage->pixels[y], sizeof(Pixel), dataImage->header.width_px, destFile);
  }
  fclose(destFile);
}

void freeImage(BMP_Image* image) {
  for (int i = 0; i < image->norm_height; i++) {
    free(image->pixels[i]);
  }
  free(image->pixels);
  free(image);
}

int checkBMPValid(BMP_Header* header) {
  if (header->type != 0x4d42) return FALSE;
  if (header->bits_per_pixel != 24) return FALSE;
  if (header->planes != 1) return FALSE;
  if (header->compression != 0) return FALSE;
  return TRUE;
}

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

void printBMPImage(BMP_Image* image) {
  printf("data size is %ld\n", sizeof(image->pixels));
  printf("norm_height size is %d\n", image->norm_height);
  printf("bytes per pixel is %d\n", image->bytes_per_pixel);
}

void applyParallelProcessing(BMP_Image *imageIn, BMP_Image *imageOut, int numThreads) {
  applyParallelFirstHalfBlur(imageIn, imageOut, numThreads);
  applyParallelSecondHalfEdge(imageIn, imageOut, numThreads);
}