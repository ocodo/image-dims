#include <stdio.h>
#include <stdint.h>

static uint16_t read_be16(FILE *f) {
    return (uint16_t)((fgetc(f) << 8) | fgetc(f));
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("usage: %s file.jpg\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "rb");
    if (!f) {
        perror("fopen");
        return 1;
    }

    // Check SOI marker
    if (fgetc(f) != 0xFF || fgetc(f) != 0xD8) {
        printf("Not a JPEG\n");
        return 1;
    }

    while (1) {
        int c;
        // Find marker prefix 0xFF
        while ((c = fgetc(f)) != 0xFF && c != EOF);

        if (c == EOF) break;

        // Skip padding FFs
        while ((c = fgetc(f)) == 0xFF);

        if (c == EOF) break;

        uint8_t marker = (uint8_t)c;

        // SOF0, SOF1, SOF2 etc. contain dimensions
        if (marker >= 0xC0 && marker <= 0xC3) {
            fseek(f, 3, SEEK_CUR); // skip length (2) + precision (1)
            uint16_t h = read_be16(f);
            uint16_t w = read_be16(f);

            printf("w: %i h: %i\n", w, h);
            fclose(f);
            return 0;
        } else {
            // skip segment
            uint16_t len = read_be16(f);
            if (len < 2) break;
            fseek(f, len - 2, SEEK_CUR);
        }
    }

    printf("Could not find dimensions\n");
    fclose(f);
    return 1;
}
