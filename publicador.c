#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include "bmp.h"

#define SHM_NAME "/shm_bmp"
#define OUTPUT_PATH "outputs/final_output.bmp"

int main() {
    char imagePath[256];

    printf("Ingrese la ruta de la imagen BMP: ");
    scanf("%255s", imagePath);

    // Abrir el archivo BMP
    FILE *source = fopen(imagePath, "rb");
    if (!source) {
        printf("Error: No se pudo abrir la imagen.\n");
        return EXIT_FAILURE;
    }

    // Leer la imagen en memoria
    BMP_Image *imageIn = createBMPImage(source);
    if (!imageIn) {
        fclose(source);
        return EXIT_FAILURE;
    }
    readImage(source, imageIn);
    fclose(source);

    // Crear memoria compartida
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        printf("Error al crear la memoria compartida.\n");
        freeImage(imageIn);
        return EXIT_FAILURE;
    }

    // Ajustar el tamaño de la memoria compartida
    ftruncate(shm_fd, sizeof(BMP_Image));

    // Mapear la memoria compartida
    BMP_Image *sharedImage = mmap(NULL, sizeof(BMP_Image), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (sharedImage == MAP_FAILED) {
        printf("Error al mapear la memoria compartida.\n");
        freeImage(imageIn);
        close(shm_fd);
        return EXIT_FAILURE;
    }

    // Copiar la imagen a memoria compartida
    memcpy(sharedImage, imageIn, sizeof(BMP_Image));

    // Crear procesos para Desenfocador y Realzador
    pid_t pid1 = fork();
    if (pid1 == 0) {
        execl("./desenfocador", "./desenfocador", imagePath, "outputs/desenfoque.bmp", "4", NULL);
        exit(EXIT_FAILURE);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        execl("./realzador", "./realzador", imagePath, "outputs/realce.bmp", "4", NULL);
        exit(EXIT_FAILURE);
    }

    // Esperar a que ambos procesos terminen
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    // Ejecutar el combinador para fusionar las imágenes
    pid_t pid3 = fork();
    if (pid3 == 0) {
        execl("./combinador", "./combinador", "outputs/desenfoque.bmp", "outputs/realce.bmp", OUTPUT_PATH, NULL);
        exit(EXIT_FAILURE);
    }

    // Esperar a que el combinador termine
    waitpid(pid3, NULL, 0);

    // Liberar memoria compartida
    munmap(sharedImage, sizeof(BMP_Image));
    close(shm_fd);
    shm_unlink(SHM_NAME);

    printf("Imagen final combinada guardada en %s\n", OUTPUT_PATH);
    return EXIT_SUCCESS;
}
