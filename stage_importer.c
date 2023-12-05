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
static const int STRIDE = N * 4 * 4;

static Cell cells[N * N];

int main(void) {
    int x, y, n;
    unsigned char *data =
        stbi_load("levels/Battle_City_Stage01.png", &x, &y, &n, 0);
    printf("%lu\n", sizeof(Cell));
    printf("%d %d %d\n", x, y, n);
    for (int cellIndex = 0; cellIndex < N * N; cellIndex++) {
        int i = (cellIndex / N) * N * 16 * 4 + (cellIndex % N) * 4 * 4;
        // printf("%d ", i);
        unsigned char r = data[i], g = data[i + 1], b = data[i + 2];
        if (r == 0 && g == 0 && b == 0)
            cells[cellIndex] = (Cell){.type = CTBlank, .texNumber = 0};
        else if (r == 160 && g == 48 && b == 0)
            cells[cellIndex] = (Cell){.type = CTBrick, .texNumber = 0};
        else if (r == 127 && g == 127 && b == 127)
            cells[cellIndex] = (Cell){.type = CTBrick, .texNumber = 1};
        else if (r == 188 && g == 188 && b == 188) {
            int k = i + (3 * 4) + STRIDE;
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
