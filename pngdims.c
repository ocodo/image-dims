#include <stdio.h>
#include <stdint.h>

int main(int argc, char *argv[])
{
  if (argc != 2)
    return 1;

  FILE *fp = fopen(argv[1], "rb");
  if (!fp)
    return 1;

  fseek(fp, 16, SEEK_SET);

  unsigned char b[8];
  if (fread(b, 1, 8, fp) != 8) {
    fclose(fp);
    return 1;
  }

  fclose(fp);

  uint32_t w =
    ((uint32_t)b[0] << 24) |
    ((uint32_t)b[1] << 16) |
    ((uint32_t)b[2] << 8)  |
    ((uint32_t)b[3]);

  uint32_t h =
    ((uint32_t)b[4] << 24) |
    ((uint32_t)b[5] << 16) |
    ((uint32_t)b[6] << 8)  |
    ((uint32_t)b[7]);

  printf("w:%u h:%u\n", w, h);
  return 0;
}
