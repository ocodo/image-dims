#include <stdio.h>
#include <stdint.h>

static int read_le32(unsigned char *b)
{
    return (int)(
        (uint32_t)b[0] |
        (uint32_t)b[1] << 8 |
        (uint32_t)b[2] << 16 |
        (uint32_t)b[3] << 24
    );
}

int main(int argc, char *argv[])
{
    if (argc != 2)
        return 1;

    FILE *fp = fopen(argv[1], "rb");
    if (!fp)
        return 1;

    // width + height start at offset 18
    if (fseek(fp, 18, SEEK_SET) != 0) {
        fclose(fp);
        return 1;
    }

    unsigned char b[8];
    if (fread(b, 1, 8, fp) != 8) {
        fclose(fp);
        return 1;
    }

    fclose(fp);

    int w = read_le32(&b[0]);
    int h = read_le32(&b[4]);

    // BMP height can be negative (top-down bitmap)
    if (h < 0) h = -h;

    printf("w:%d h:%d\n", w, h);

    return 0;
}
