#include <stdio.h>
#include <stdlib.h>

typedef struct {
    char *bytes;
    long size;
} Buffer;

Buffer readFile(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        return (Buffer){};
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
