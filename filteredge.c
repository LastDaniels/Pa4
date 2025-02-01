void *edgeDetectionThreadWorker(void *args) {
    ThreadArgs *threadArgs = (ThreadArgs *)args;
    BMP_Image *imageIn = threadArgs->imageIn;
    BMP_Image *imageOut = threadArgs->imageOut;
    int startRow = threadArgs->startRow;
    int endRow = threadArgs->endRow;
    int width = imageIn->header.width_px;

    for (int y = startRow; y < endRow; y++) {
        for (int x = 1; x < width - 1; x++) {
            Pixel *outPixel = &imageOut->pixels[y][x];
            int sumX[3] = {0}, sumY[3] = {0};
            if (y == 0 || x == 0 || y == imageIn->header.height_px - 1 || x == width - 1) {
                outPixel->red = outPixel->green = outPixel->blue = 0;
            } else {
                for (int ky = -1; ky <= 1; ky++) {
                    for (int kx = -1; kx <= 1; kx++) {
                        Pixel *pixel = &imageIn->pixels[y + ky][x + kx];
                        int weightX = threadArgs->prewittX[ky + 1][kx + 1];
                        int weightY = threadArgs->prewittY[ky + 1][kx + 1];
                        sumX[0] += pixel->red * weightX;
                        sumX[1] += pixel->green * weightX;
                        sumX[2] += pixel->blue * weightX;
                        sumY[0] += pixel->red * weightY;
                        sumY[1] += pixel->green * weightY;
                        sumY[2] += pixel->blue * weightY;
                    }
                }
                outPixel->red = clamp((int)sqrt(sumX[0] * sumX[0] + sumY[0] * sumY[0]));
                outPixel->green = clamp((int)sqrt(sumX[1] * sumX[1] + sumY[1] * sumY[1]));
                outPixel->blue = clamp((int)sqrt(sumX[2] * sumX[2] + sumY[2] * sumY[2]));
            }
        }
    }
    return NULL;
}

void applyParallelSecondHalfEdge(BMP_Image *imageIn, BMP_Image *imageOut, int numThreads) {
    if (!validateBMPImage(imageIn) || !validateBMPImage(imageOut)) {
        fprintf(stderr, "Invalid BMP image structure for parallel processing.\n");
        return;
    }

    int height = imageIn->header.height_px;
    int width = imageIn->header.width_px;
    int halfHeight = height / 2;
    int rowsToProcess = height - halfHeight;
    if (numThreads > rowsToProcess) {
        numThreads = rowsToProcess;
    }

    pthread_t threads[numThreads];
    ThreadArgs threadArgs[numThreads];
    int rowsPerThread = rowsToProcess / numThreads;
    int extraRows = rowsToProcess % numThreads;

    for (int i = 0; i < numThreads; i++) {
        threadArgs[i].imageIn = imageIn;
        threadArgs[i].imageOut = imageOut;
        threadArgs[i].prewittX = prewittX;
        threadArgs[i].prewittY = prewittY;
        threadArgs[i].startRow = halfHeight + i * rowsPerThread;
        threadArgs[i].endRow = threadArgs[i].startRow + rowsPerThread;
        if (i == numThreads - 1) {
            threadArgs[i].endRow += extraRows;
        }
        if (threadArgs[i].endRow > height) {
            threadArgs[i].endRow = height;
        }
        if (pthread_create(&threads[i], NULL, edgeDetectionThreadWorker, &threadArgs[i]) != 0) {
            fprintf(stderr, "Error creating thread %d\n", i);
            exit(EXIT_FAILURE);
        }
    }
    for (int i = 0; i < numThreads; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            fprintf(stderr, "Error joining thread %d\n", i);
            exit(EXIT_FAILURE);
        }
    }
    for (int y = halfHeight; y < height; y++) {
        for (int x = 0; x < width; x++) {
            imageOut->pixels[y][x] = imageIn->pixels[y][x];
        }
    }
}
