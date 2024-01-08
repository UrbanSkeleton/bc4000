#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint32_t u32;

typedef struct {
    char *bytes;
    long size;
} Buffer;

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

void saveBuffer(Buffer b, const char *filename) {
    FILE *f = fopen(filename, "wb");
    if (!f) {
        fprintf(stderr, "Cannot open file: %s", filename);
        exit(1);
    }
    fwrite(b.bytes, b.size, 1, f);
    fclose(f);
}

Buffer readFile(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Cannot open file: %s", filename);
        exit(1);
    }
    Buffer res = {};
    fseek(f, 0, SEEK_END);
    res.size = ftell(f);
    fseek(f, 0, SEEK_SET);
    res.bytes = malloc(res.size);
    if (1 != fread(res.bytes, res.size, 1, f)) {
        fclose(f);
        return (Buffer){};
    }
    fclose(f);
    return res;
}

static bool collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2,
                      int h2) {
    return (MAX(x1, x2) < MIN(x1 + w1, x2 + w2)) &&
           (MAX(y1, y2) < MIN(y1 + h1, y2 + h2));
}
