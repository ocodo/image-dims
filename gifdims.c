#include <stdio.h>
#include <stdint.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
        return 1;

    FILE *fp = fopen(argv[1], "rb");
    if (!fp)
        return 1;

    fseek(fp, 6, SEEK_SET);

    unsigned char b[4];
    if (fread(b, 1, 4, fp) != 4) {
        fclose(fp);
        return 1;
    }

    fclose(fp);

    uint16_t w = (uint16_t)b[0] | ((uint16_t)b[1] << 8);
    uint16_t h = (uint16_t)b[2] | ((uint16_t)b[3] << 8);

    printf("w:%u h:%u\n", w, h);

    return 0;
}
