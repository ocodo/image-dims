#include <stdio.h>
#include <stdint.h>

static uint16_t le16(FILE *f) {
    uint8_t b[2];
    fread(b, 1, 2, f);
    return b[0] | (b[1] << 8);
}

static uint32_t le24(FILE *f) {
    uint8_t b[3];
    fread(b, 1, 3, f);
    return b[0] | (b[1] << 8) | (b[2] << 16);
}

static uint32_t le32(FILE *f) {
    uint8_t b[4];
    fread(b, 1, 4, f);
    return b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24);
}

int main(int argc, char **argv) {
    if (argc != 2) return 1;

    FILE *f = fopen(argv[1], "rb");
    if (!f) return 1;

    char riff[4], webp[4];
    fread(riff, 1, 4, f);
    le32(f);
    fread(webp, 1, 4, f);

    if (riff[0]!='R'||riff[1]!='I'||riff[2]!='F'||riff[3]!='F' ||
        webp[0]!='W'||webp[1]!='E'||webp[2]!='B'||webp[3]!='P') {
        printf("Not a WebP\n");
        return 1;
    }

    char c1 = fgetc(f);
    char c2 = fgetc(f);
    char c3 = fgetc(f);
    char c4 = fgetc(f);

    // =========================
    // VP8X (extended format)
    // =========================
    if (c1=='V' && c2=='P' && c3=='8' && c4=='X') {
        fseek(f, 5, SEEK_CUR); // flags + reserved
        uint32_t w = le24(f) + 1;
        uint32_t h = le24(f) + 1;
        printf("w: %i h: %i\n", w, h);
        return 0;
    }

    // =========================
    // VP8L (lossless)
    // =========================
    if (c1=='V' && c2=='P' && c3=='8' && c4=='L') {
        uint32_t sig = fgetc(f);
        sig |= (fgetc(f) << 8);
        sig |= (fgetc(f) << 16);
        sig |= (fgetc(f) << 24);

        // 14-bit width + height packed
        uint32_t w = (sig & 0x3FFF) + 1;
        uint32_t h = ((sig >> 14) & 0x3FFF) + 1;

        printf("w: %i h: %i\n", w, h);
        return 0;
    }

    // =========================
    // VP8 (lossy)
    // =========================
    if (c1=='V' && c2=='P' && c3=='8' && c4==' ') {
        // skip frame header (simplified but valid for dims)
        fseek(f, 6, SEEK_CUR);

        uint16_t w = le16(f) & 0x3FFF;
        uint16_t h = le16(f) & 0x3FFF;

        printf("w: %i h: %i\n", w, h);
        return 0;
    }

    printf("Unsupported WebP\n");
    return 1;
}
