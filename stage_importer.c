#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

typedef enum {
    CTBorder,
    CTBlank,
    CTBrick,
    CTConcrete,
    CTForest,
    CTRiver,
    CTMax
} CellType;

typedef struct {
    char type;
    char texNumber;
} Cell;

static const int N = 13 * 4;

static Cell cells[N * N];

int main(void) {
    int x, y, bpp;
    const char *filename =
        "/Users/Candid/Documents/bc4000-stages/Battle_City_Stage02.png";
    unsigned char *data = stbi_load(filename, &x, &y, &bpp, 0);
    if (!data) {
        printf("Cannot open file %s\n", filename);
        exit(1);
    }
    printf("%d %d %d\n", x, y, bpp);
    for (int cellIndex = 0; cellIndex < N * N; cellIndex++) {
        int i = (cellIndex / N) * N * 16 * bpp + (cellIndex % N) * 4 * bpp;
        // printf("%d ", i);
        unsigned char r = data[i], g = data[i + 1], b = data[i + 2];
        if (r == 0 && g == 0 && b == 0) {
            int k = i + (1 * bpp);
            if (data[k] == 48)
                cells[cellIndex] = (Cell){.type = CTForest, .texNumber = 0};
            else
                cells[cellIndex] = (Cell){.type = CTBlank, .texNumber = 0};
        } else if (r == 160 && g == 48 && b == 0)
            cells[cellIndex] = (Cell){.type = CTBrick, .texNumber = 0};
        else if (r == 127 && g == 127 && b == 127) {
            int k = i + (1 * bpp);
            if (data[k] == 160)
                cells[cellIndex] = (Cell){.type = CTBrick, .texNumber = 1};
            else
                cells[cellIndex] = (Cell){.type = CTBlank, .texNumber = 0};
        } else if (r == 188 && g == 188 && b == 188) {
            int k = i + (3 * bpp) + (N * 4 * bpp);
            if (data[k] == 188 && data[k + 1] == 188 && data[k + 2] == 188) {
                cells[cellIndex] = (Cell){.type = CTConcrete, .texNumber = 0};
            } else if (data[k] == 127 && data[k + 1] == 127 &&
                       data[k + 2] == 127) {
                cells[cellIndex] = (Cell){.type = CTConcrete, .texNumber = 1};
            } else if (data[k] == 255 && data[k + 1] == 255 &&
                       data[k + 2] == 255) {
                cells[cellIndex] = (Cell){.type = CTConcrete, .texNumber = 2};
            } else {
                printf("Invalid cell: concrete at %d\n", cellIndex);
                exit(1);
            }
        } else if (r == 255 && g == 255 && b == 255) {
            cells[cellIndex] = (Cell){.type = CTConcrete, .texNumber = 3};
        } else if (r == 56 && g == 104 && b == 0) {
            cells[cellIndex] = (Cell){.type = CTForest, .texNumber = 1};
        } else if (r == 48 && g == 96 && b == 64) {
            cells[cellIndex] = (Cell){.type = CTForest, .texNumber = 2};
        } else if (r == 152 && g == 232 && b == 0) {
            cells[cellIndex] = (Cell){.type = CTForest, .texNumber = 3};
        } else {
            printf("Invalid cell at %d: r=%d, g=%d, b=%d\n", cellIndex, r, g,
                   b);
            exit(1);
        }
        // printf("%d %d %d\n", cellIndex, cells[cellIndex].type,
        //        cells[cellIndex].texNumber);
    }
    FILE *out = fopen("levels/stage01", "wb");
    fwrite((void *)cells, sizeof(cells), 1, out);
    fclose(out);
}
