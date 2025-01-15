#include <stdio.h>
#include <stdlib.h>

#define BLOCK_SIZE 16
#define BLOCK_AREA BLOCK_SIZE*BLOCK_SIZE

typedef struct {
    __uint8_t r;
    __uint8_t g;
    __uint8_t b;
} Pixel;

typedef struct {
    Pixel pixels[BLOCK_SIZE][BLOCK_SIZE];
} Block;

typedef struct {
    int sizeX, sizeY;
    Block* blocks;
} PPMImage;

void readPPM(const char* filename) {
    FILE* fp = fopen(filename, "rb");
    if(!fp) {
        fprintf(stderr, "Unable to open file '%s'\n", filename);
        exit(1);
    }

    //read image format
    char buff[16];
    if(!fgets(buff, sizeof(buff), fp)) {
        perror(filename);
        exit(1);
    }

    //check the image format
    if(buff[0] != 'P' || buff[1] != '3') {
        fprintf(stderr, "Invalid image format (must be 'P3')\n");
        exit(1);
    }

    PPMImage* img = malloc(sizeof(PPMImage));
    if(!img) {
        fprintf(stderr, "Unable to allocate memory\n");
        exit(1);
    }

    if(fscanf(fp, "%d %d", &(img->sizeX), &(img->sizeY)) != 2) {
        fprintf(stderr, "Invalid image size (error loading '%s')\n", filename);
        exit(1);
    }

    while(fgetc(fp) != '\n');

    const size_t blockNumber = (img->sizeX * img->sizeY)/BLOCK_AREA;
    img->blocks = malloc(blockNumber * sizeof(Block));

    Pixel* pixels = malloc(img->sizeX * img->sizeY * sizeof(Pixel));

    //read pixel data from file
    if(fread(pixels, 3 * img->sizeX, img->sizeY, fp) != img->sizeY) {
        fprintf(stderr, "Error loading image '%s'\n", filename);
        exit(1);
    }


    const size_t rowBlockNumber = img->sizeX*BLOCK_SIZE / BLOCK_AREA;
    Pixel* currentBlockFirstPixelPtr = pixels;
    for(size_t i = 0; i < blockNumber; i++) {
        Pixel* blockStartPtr[BLOCK_SIZE];

        // if(){

        // }
    }
    

    fclose(fp);
}

int main(int argc, char** argv) {
    // printf("%s\n", argv[1]);
    readPPM(argv[1]);
    return 0;
}

