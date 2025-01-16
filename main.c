#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define BLOCK_SIZE 16
#define BLOCK_AREA BLOCK_SIZE * BLOCK_SIZE

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
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
    char buff[3];
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

    // skip the '255' in third line
    fgetc(fp);
    while(fgetc(fp) != '\n');

    const size_t pixels_Per_Image = img->sizeX * img->sizeY;
    const size_t blocks_Per_Image = (pixels_Per_Image) / BLOCK_AREA;
    img->blocks = malloc(blocks_Per_Image * sizeof(Block));

    const size_t blocks_Per_Line = img->sizeX / BLOCK_SIZE;

    Pixel *dump = malloc(pixels_Per_Image * sizeof(Pixel));
    for (size_t i = 0; i < pixels_Per_Image; i++) {
        Pixel pixel;
        fscanf(fp, "%d %d %d", &pixel.r, &pixel.g, &pixel.b);
        dump[i] = pixel;
    }

    const uint8_t current_Block = 0;
    for (size_t current_Block = 0, current_Row = 0; current_Block < blocks_Per_Image; current_Block++) {
        Block *block = &(img->blocks[current_Block]);
        
        if (current_Block % blocks_Per_Line == 0 && current_Block != 0) current_Row++;
        // printf("%d\n", current_Row);

        for (size_t i = 0; i < BLOCK_SIZE; i++) {
            for (size_t j = 0; j < BLOCK_SIZE; j++) {
                Pixel pixel = dump[(current_Block % blocks_Per_Line) * BLOCK_SIZE + i + img->sizeX * j + current_Row * img->sizeX * BLOCK_SIZE];
                
                block->pixels[j][i] = pixel;
                // printf("\e[48;2;%d;%d;%dm ", pixel.r, pixel.g, pixel.b);
            }
            // printf("\n");
        }
        // getchar();
    }
    

    // for (int j = 0; j < img->sizeY; j++) {
    //     for (int i = 0; i < img->sizeX; i++) {
    //         int rgb[3];
    //         fscanf(fp, "%d %d %d", &rgb[0], &rgb[1], &rgb[2]);
    //         printf("\e[48;2;%d;%d;%dm ", rgb[0], rgb[1], rgb[2]);
    //     }
    //     printf("\n");
    // }







    // Pixel* pixels = malloc(img->sizeX * img->sizeY * sizeof(Pixel));

    // //read pixel data from file
    // if(fread(pixels, 3 * img->sizeX, img->sizeY, fp) != img->sizeY) {
    //     fprintf(stderr, "Error loading image '%s'\n", filename);
    //     exit(1);
    // }


    // const size_t rowBlockNumber = img->sizeX*BLOCK_SIZE / BLOCK_AREA;
    // Pixel* currentBlockFirstPixelPtr = pixels;
    // for(size_t i = 0; i < blocksPerImage; i++) {
    //     Pixel* blockStartPtr[BLOCK_SIZE];

    //     // if(){

    //     // }
    // }
    

    fclose(fp);
}

int main(int argc, char** argv) {
    // printf("%s\n", argv[1]);
    readPPM(argv[1]);
    return 0;
}

