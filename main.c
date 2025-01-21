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
            // printf("DIO SPORCO %d %s\n", i, bitmaps[i].blockName);
            return 0;
        }
        
        // 🫡 you will always be remembered memcmp!
        // int res = memcmp(&(bitmaps[i].block), block, sizeof(Block));
        // if (res == 0) {
        //     *out = bitmaps[i].blockName;
        //     return 0;
        // }
    }

    // printBlock(block);

    return -1;
}

static void print_nbt_tree(nbt_tag_t* tag, int indentation) {
  for (int i = 0; i < indentation; i++) {
    printf(" ");
  }

  if (tag->name) {
    printf("%s: ", tag->name);
  }

  switch (tag->type) {
    case NBT_TYPE_END: {
      printf("[end]");
      break;
    }
    case NBT_TYPE_BYTE: {
      printf("%hhd", tag->tag_byte.value);
      break;
    }
    case NBT_TYPE_SHORT: {
      printf("%hd", tag->tag_short.value);
      break;
    }
    case NBT_TYPE_INT: {
      printf("%d", tag->tag_int.value);
      break;
    }
    case NBT_TYPE_LONG: {
      printf("%ld", tag->tag_long.value);
      break;
    }
    case NBT_TYPE_FLOAT: {
      printf("%f", tag->tag_float.value);
      break;
    }
    case NBT_TYPE_DOUBLE: {
      printf("%f", tag->tag_double.value);
      break;
    }
    case NBT_TYPE_BYTE_ARRAY: {
      printf("[byte array]");
      break;
      for (size_t i = 0; i < tag->tag_byte_array.size; i++) {
        printf("%hhd ", tag->tag_byte_array.value[i]);
      }
      break;
    }
    case NBT_TYPE_STRING: {
      printf("%s", tag->tag_string.value);
      break;
    }
    case NBT_TYPE_LIST: {
      printf("\n");
      for (size_t i = 0; i < tag->tag_list.size; i++) {
        print_nbt_tree(tag->tag_list.value[i], indentation + tag->name_size + 2);
      }
      break;
    }
    case NBT_TYPE_COMPOUND: {
      printf("\n");
      for (size_t i = 0; i < tag->tag_compound.size; i++) {
        print_nbt_tree(tag->tag_compound.value[i], indentation + tag->name_size + 2);
      }
      break;
    }
    case NBT_TYPE_INT_ARRAY: {
      printf("[int array]");
      break;
      for (size_t i = 0; i < tag->tag_int_array.size; i++) {
        printf("%d ", tag->tag_int_array.value[i]);
      }
      break;
    }
    case NBT_TYPE_LONG_ARRAY: {
      printf("[long array]");
      break;
      for (size_t i = 0; i < tag->tag_long_array.size; i++) {
        printf("%ld ", tag->tag_long_array.value[i]);
      }
      break;
    }
    default: {
      printf("[error]");
    }
  }

  printf("\n");
}

static size_t writer_write(void* userdata, uint8_t* data, size_t size) {
  return fwrite(data, 1, size, userdata);
}

void write_nbt_file(const char* name, nbt_tag_t* tag, int flags) {

  FILE* file = fopen(name, "wb");

  nbt_writer_t writer;

  writer.write = writer_write;
  writer.userdata = file;

  nbt_write(writer, tag, flags);

  fclose(file);
}

int main() {
    // generateBitmap("./ppm", "./bitmaps");

    Bitmap *bitmaps;
    size_t n = getBitmaps(&bitmaps, "./bitmaps");
    PPMImage *img = readPPM("./source/SpritecraftOutput.ppm");
    BitmapElement palette[MAX_UNIQUE_BLOCKS];
    uint16_t paletteLength = 0;

    nbt_tag_t *root = nbt_new_tag_compound();

    nbt_tag_t *paletteTag = nbt_new_tag_list(NBT_TYPE_COMPOUND);
    nbt_set_tag_name(paletteTag, "palette", strlen("palette"));

    nbt_tag_t *blocksTag = nbt_new_tag_list(NBT_TYPE_COMPOUND);
    nbt_set_tag_name(blocksTag, "blocks", strlen("blocks"));

    uint8_t y = 0xFF; // wrapping to 0 when ++ is called
    for (int b = img->blocksNumber - 1, i = 0; b >= 0; b--, i++) {
        BitmapElement el;
        int ret = blockCmp(&img->blocks[b], bitmaps, n, &el, 5);
        if (ret == EOF) {
            fprintf(stderr, "Block not Recognized (Something has gone very wrong!)");
            exit(1);
        }

        uint8_t blockAlreadyPresent = 0;
        size_t paletteIdx = 0;
        for (; paletteIdx < paletteLength; paletteIdx++) {
            if (el.idxInArray == palette[paletteIdx].idxInArray) {
                blockAlreadyPresent = 1;
                break;
            }
        }
        
        if(!blockAlreadyPresent) palette[paletteLength++] = el;

        uint8_t x = i % img->blocksPerLine;
        if (x == 0) y++;

        nbt_tag_t *blocksTagElement = nbt_new_tag_compound();
        nbt_set_tag_name(blocksTagElement, "", 1);

        nbt_tag_t *posTag = nbt_new_tag_list(NBT_TYPE_INT);
        nbt_tag_list_append(posTag, nbt_new_tag_int(x));
        nbt_tag_list_append(posTag, nbt_new_tag_int(y));
        nbt_tag_list_append(posTag, nbt_new_tag_int(0));
        nbt_set_tag_name(posTag, "pos", strlen("pos"));

        nbt_tag_t *stateTag = nbt_new_tag_int(paletteIdx);
        nbt_set_tag_name(stateTag, "state", strlen("state"));

        nbt_tag_compound_append(blocksTagElement, posTag);
        nbt_tag_compound_append(blocksTagElement, stateTag);
        nbt_tag_list_append(blocksTag, blocksTagElement);

        // printf("%s (%d, %d, 0)\n", el.blockName, x, y);
        // printBlock(&img->blocks[b]);
        // getchar();
    }

    for (size_t e = 0; e < paletteLength; e++) {
        nbt_tag_t *el = nbt_new_tag_compound();
        nbt_set_tag_name(el, "", 1);

        char buf[256] = "minecraft:";
        strcat(buf, palette[e].blockName);
        nbt_tag_t *name = nbt_new_tag_string(buf, strlen(buf));
        nbt_set_tag_name(name, "Name", strlen("Name"));

        nbt_tag_compound_append(el, name);
        nbt_tag_list_append(paletteTag, el);

        // printf("%s ", palette[e].blockName);
    }

    nbt_tag_t *sizeTag = nbt_new_tag_list(NBT_TYPE_INT);
    nbt_tag_list_append(sizeTag, nbt_new_tag_int(img->blocksPerLine));
    nbt_tag_list_append(sizeTag, nbt_new_tag_int(img->sizeY / BLOCK_SIZE));
    nbt_tag_list_append(sizeTag, nbt_new_tag_int(1));
    nbt_set_tag_name(sizeTag, "size", strlen("size"));

    nbt_tag_t *dataVersion = nbt_new_tag_int(3953);
    nbt_set_tag_name(dataVersion, "DataVersion", strlen("DataVersion"));

    nbt_tag_compound_append(root, blocksTag);
    nbt_tag_compound_append(root, paletteTag);
    nbt_tag_compound_append(root, sizeTag);
    nbt_tag_compound_append(root, dataVersion);
    print_nbt_tree(root, 4);

    write_nbt_file("hiura.nbt", root, NBT_WRITE_FLAG_USE_GZIP);

    return 0;
}

