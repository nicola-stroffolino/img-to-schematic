#define NBT_IMPLEMENTATION
#define LINUX
#define HAS_BITMAP


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "nbt.h"
#ifdef LINUX
#include <dirent.h>
#endif
#include <sys/stat.h>
#include <errno.h>


#define BLOCK_SIZE 16
#define BLOCK_AREA (BLOCK_SIZE * BLOCK_SIZE)
#define MAX_UNIQUE_BLOCKS 2048


typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} Pixel;

typedef struct {
    Pixel pixels[BLOCK_SIZE][BLOCK_SIZE];
} Block;

typedef struct {
    uint32_t sizeX, sizeY;
    size_t blocksNumber;
    size_t blocksPerLine;
    Block* blocks;
} PPMImage;

typedef struct {
    char *blockName;
    uint8_t idxInArray;
} BitmapElement;


PPMImage *readPPM(const char *filename);
void printBlock(Block *block);

#if (!(defined HAS_BITMAP) && defined LINUX)
void generateBitmap(const char* src, const char* dest) {
    struct dirent *entry;
    DIR *dp = opendir(src);

    if (dp == NULL) {
        perror("Error opening directory");
        return;
    }

    while ((entry = readdir(dp)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Build the full file path
        char filePath[1024];
        snprintf(filePath, sizeof(filePath), "%s/%s", src, entry->d_name);

        // Check if it's a regular file
        struct stat fileStat;
        if (stat(filePath, &fileStat) == 0 && S_ISREG(fileStat.st_mode)) {
            PPMImage *img = readPPM(filePath);

            char* name = strstr(entry->d_name, ".ppm");
            if (name != NULL) *name = 0;

            char path[128];
            strcpy(path, dest);
            strcat(path, "/");
            strcat(path, entry->d_name);
            strcat(path, ".bacon");

            FILE *file = fopen(path, "wb");

            fwrite(img->blocks, sizeof(Block), 1, file);

            fclose(file);
        }
    }

    closedir(dp);
}
#endif

#if (defined HAS_BITMAP && defined LINUX)
typedef struct {
    char* blockName;
    Block block;
} Bitmap;

size_t getBitmaps(Bitmap** blank_Bitmap, const char* dir) {
    struct dirent *entry;
    DIR *dp = opendir(dir);

    if (dp == NULL) {
        perror("Error opening directory");
        return;
    }

    Bitmap buffer[MAX_UNIQUE_BLOCKS];
    size_t i = 0;
    while ((entry = readdir(dp)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Build the full file path
        char filePath[1024];
        snprintf(filePath, sizeof(filePath), "%s/%s", dir, entry->d_name);

        // Check if it's a regular file
        struct stat fileStat;
        if (stat(filePath, &fileStat) == 0 && S_ISREG(fileStat.st_mode)) {
            char* name = strstr(entry->d_name, ".bacon");
            if (name != NULL) *name = 0;

            FILE* fp = fopen(filePath, "rb");

            fread(&(buffer[i].block), sizeof(Block), 1, fp);
            buffer[i].blockName = malloc((strlen(entry->d_name) + 1) * sizeof(char));
            strcpy(buffer[i].blockName, entry->d_name);

            fclose(fp);
            i++;
        }
    }

    *blank_Bitmap = malloc(sizeof(Bitmap) * i);
    memcpy(*blank_Bitmap, buffer, sizeof(Bitmap) * i);

    closedir(dp);
    return i;
}
#endif

PPMImage* readPPM(const char* filename) {
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
    const size_t blocks_Per_Image = pixels_Per_Image / BLOCK_AREA;
    const size_t blocks_Per_Line = img->sizeX / BLOCK_SIZE;

    img->blocks = malloc(blocks_Per_Image * sizeof(Block));
    img->blocksNumber = blocks_Per_Image;
    img->blocksPerLine = blocks_Per_Line;

    Pixel *dump = malloc(pixels_Per_Image * sizeof(Pixel));
    // DO NOT CHANGE SIZE_T IF ON LINUX, PLEASE
    for (size_t p = 0; p < pixels_Per_Image; p++) {
        Pixel pixel;
        // DO NOT REMOVE THIS VARIABLE ALSO IF ON LINUX
        int ret = fscanf(fp, "%d %d %d", &pixel.r, &pixel.g, &pixel.b);
        dump[p] = pixel;
    }

    // printf("blocks_Per_Image: %d, blocks_Per_Line: %d, pixels_Per_Image: %d\n", blocks_Per_Image, blocks_Per_Line, pixels_Per_Image);
    const uint8_t current_Block = 0;
    for (size_t current_Block = 0, current_Row = 0; current_Block < blocks_Per_Image; current_Block++) {
        Block *block = &(img->blocks[current_Block]);
        
        if (current_Block % blocks_Per_Line == 0 && current_Block != 0) current_Row++;

        // printf("current_Row: %d, current_Block: %d, sizeX: %d\n", current_Row, current_Block, img->sizeX);
        for (size_t i = 0; i < BLOCK_SIZE; i++) {
            for (size_t j = 0; j < BLOCK_SIZE; j++) {
                Pixel pixel = dump[((current_Block % blocks_Per_Line) * BLOCK_SIZE) + i + (img->sizeX * j) + (current_Row * img->sizeX * BLOCK_SIZE)];
                
                block->pixels[j][i] = pixel;
                
            }
            // printf("\n");
        }

        // printBlock(block);
        
        #ifdef HAS_BITMAP
        
        #endif
    }
    
    fclose(fp);
    return img;
}

void printBlock(Block* block) {
    for (size_t i = 0; i < BLOCK_SIZE; i++) {
        for (size_t j = 0; j < BLOCK_SIZE; j++) {
            Pixel pixel = block->pixels[i][j];
            printf("\e[48;2;%d;%d;%dm ", pixel.r, pixel.g, pixel.b);
        }
        printf("\n");
    }
}

int blockCmp(Block* block, Bitmap* bitmaps, size_t n, BitmapElement* el, uint8_t tolerance) {
    for (size_t i = 0; i < n; i++) {
        uint8_t isBlockValid = 1;
        for (size_t y = 0; y < BLOCK_SIZE; y++) {
            for (size_t x = 0; x < BLOCK_SIZE; x++) {
                if (
                    abs((int)(block->pixels[y][x].r) - (int)(bitmaps[i].block.pixels[y][x].r)) > tolerance ||
                    abs((int)(block->pixels[y][x].g) - (int)(bitmaps[i].block.pixels[y][x].g)) > tolerance ||
                    abs((int)(block->pixels[y][x].b) - (int)(bitmaps[i].block.pixels[y][x].b)) > tolerance
                ) {
                    isBlockValid = 0;
                    goto exit_cmp_for;
                }
            }
        }

exit_cmp_for:
        if (isBlockValid) {
            el->blockName = bitmaps[i].blockName;
            el->idxInArray = i;
            return 0;
        }
        
        // 🫡 you will always be remembered memcmp!
        // int res = memcmp(&(bitmaps[i].block), block, sizeof(Block));
        // if (res == 0) {
        //     *out = bitmaps[i].blockName;
        //     return 0;
        // }
    }

    return -1;
}

int main() {
    // generateBitmap("./ppm", "./bitmaps");

    Bitmap *bitmaps;
    size_t n = getBitmaps(&bitmaps, "./bitmaps");
    PPMImage *img = readPPM("./source/prova2.ppm");
    BitmapElement palette[MAX_UNIQUE_BLOCKS];
    uint16_t paletteLength = 0;

    for (size_t b = 0; b < img->blocksNumber; b++) {
        BitmapElement el;
        int ret = blockCmp(&img->blocks[b], bitmaps, n, &el, 5);
        if (ret == EOF) {
            fprintf(stderr, "Block not Recognized (Something has gone very wrong!)");
            exit(1);
        }

        uint8_t blockAlreadyPresent = 0;
        for (size_t e = 0; e < paletteLength; e++) {
            if (el.idxInArray == palette[e].idxInArray) {
                blockAlreadyPresent = 1;
                break;
            }
        }
        
        if(!blockAlreadyPresent) palette[paletteLength++] = el;

        // printf("%d %s", ret, el.blockName);
        // printBlock(&img->blocks[b]);
        // getchar();
    }

    for (size_t e = 0; e < paletteLength; e++) {
        printf("%s ", palette[e].blockName);
    }

    return 0;
}

