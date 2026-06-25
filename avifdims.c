#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define FOURCC(a,b,c,d) ((uint32_t)(a) << 24 | (uint32_t)(b) << 16 | (uint32_t)(c) << 8 | (uint32_t)(d))

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t primary_item_id;
    uint32_t target_property_index;
    int found_pitm;
    int found_ispe;
} AvifContext;

uint32_t read_u32_be(const uint8_t *buf) {
    return ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) | ((uint32_t)buf[2] << 8) | buf[3];
}

void parse_ipco(FILE *f, int64_t end_pos, AvifContext *ctx) {
    uint32_t prop_index = 1;
    while (ftell(f) < end_pos) {
        uint8_t header[8];
        if (fread(header, 1, 8, f) != 8) break;
        uint64_t size = read_u32_be(header);
        uint32_t type = read_u32_be(header + 4);
        int64_t box_start = ftell(f) - 8;

        if (size == 1) {
            uint8_t ext_size[8];
            if (fread(ext_size, 1, 8, f) != 8) break;
            size = ((uint64_t)read_u32_be(ext_size) << 32) | read_u32_be(ext_size + 4);
        } else if (size == 0) {
            break;
        }

        if (type == FOURCC('i','s','p','e')) {
            uint8_t buf[12];
            if (fread(buf, 1, 12, f) == 12) {
                uint32_t w = read_u32_be(buf + 4);
                uint32_t h = read_u32_be(buf + 8);
                if (!ctx->found_ispe || prop_index == ctx->target_property_index) {
                    ctx->width = w;
                    ctx->height = h;
                    ctx->found_ispe = 1;
                }
            }
        }

        prop_index++;
        fseek(f, box_start + size, SEEK_SET);
    }
}

void parse_ipma(FILE *f, int64_t end_pos, AvifContext *ctx) {
    uint8_t flags[4];
    if (fread(flags, 1, 4, f) != 4) return;
    uint8_t version = flags[0];

    uint32_t entry_count;
    uint8_t buf[4];
    if (fread(buf, 1, 4, f) != 4) return;
    entry_count = read_u32_be(buf);

    for (uint32_t i = 0; i < entry_count; i++) {
        uint32_t item_id = 0;
        if (version < 1) {
            if (fread(buf, 1, 2, f) != 2) return;
            item_id = (buf[0] << 8) | buf[1];
        } else {
            if (fread(buf, 1, 4, f) != 4) return;
            item_id = read_u32_be(buf);
        }

        uint8_t association_count = fgetc(f);
        for (uint8_t j = 0; j < association_count; j++) {
            uint32_t property_index = 0;
            if (flags[3] & 1) { // essential bit/large index logic
                if (fread(buf, 1, 2, f) != 2) return;
                property_index = ((buf[0] & 0x7F) << 8) | buf[1];
            } else {
                uint8_t b = fgetc(f);
                property_index = b & 0x7F;
            }

            if (ctx->found_pitm && item_id == ctx->primary_item_id) {
                ctx->target_property_index = property_index;
            }
        }
    }
}

void parse_boxes(FILE *f, int64_t end_pos, AvifContext *ctx) {
    while (ftell(f) < end_pos) {
        uint8_t header[8];
        if (fread(header, 1, 8, f) != 8) break;

        uint64_t size = read_u32_be(header);
        uint32_t type = read_u32_be(header + 4);
        int64_t box_start = ftell(f) - 8;

        if (size == 1) {
            uint8_t ext_size[8];
            if (fread(ext_size, 1, 8, f) != 8) break;
            size = ((uint64_t)read_u32_be(ext_size) << 32) | read_u32_be(ext_size + 4);
        } else if (size == 0) {
            fseek(f, 0, SEEK_END);
            size = ftell(f) - box_start;
        }

        int64_t box_end = box_start + size;

        if (type == FOURCC('m','e','t','a')) {
            fseek(f, 4, SEEK_CUR); // Skip FullBox version/flags
            parse_boxes(f, box_end, ctx);
        } else if (type == FOURCC('i','p','r','p')) {
            parse_boxes(f, box_end, ctx);
        } else if (type == FOURCC('i','p','c','o')) {
            parse_ipco(f, box_end, ctx);
        } else if (type == FOURCC('i','p','m','a')) {
            parse_ipma(f, box_end, ctx);
        } else if (type == FOURCC('p','i','t','m')) {
            uint8_t buf[8];
            if (fread(buf, 1, 8, f) >= 4) {
                uint8_t version = buf[0];
                if (version == 0) {
                    ctx->primary_item_id = (buf[4] << 8) | buf[5];
                } else {
                    ctx->primary_item_id = read_u32_be(buf + 4);
                }
                ctx->found_pitm = 1;
            }
        } else if (type == FOURCC('m','o','o','v') || type == FOURCC('t','r','a','k')) {
            parse_boxes(f, box_end, ctx);
        } else if (type == FOURCC('t','k','h','d')) {
            uint8_t buf[4];
            if (fread(buf, 1, 4, f) == 4) {
                uint8_t version = buf[0];
                int skip = (version == 1) ? 4 + 8 + 8 + 4 + 4 + 8 : 4 + 4 + 4 + 4 + 4 + 4;
                fseek(f, skip + 48, SEEK_CUR); // skip to width/height field
                uint8_t dim[8];
                if (fread(dim, 1, 8, f) == 8) {
                    uint32_t tw = read_u32_be(dim) >> 16; // Fixed point 16.16
                    uint32_t th = read_u32_be(dim + 4) >> 16;
                    if (ctx->width == 0) { // Fallback if ispe wasn't found
                        ctx->width = tw;
                        ctx->height = th;
                    }
                }
            }
        }

        fseek(f, box_end, SEEK_SET);
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file.avif>\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "rb");
    if (!f) {
        perror("Error opening file");
        return 1;
    }

    fseek(f, 0, SEEK_END);
    int64_t file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    AvifContext ctx = {0};

    parse_boxes(f, file_size, &ctx);
    fclose(f);

    if (ctx.found_ispe || ctx.width > 0) {
        printf("Width: %u\n", ctx.width);
        printf("Height: %u\n", ctx.height);
        return 0;
    }

    fprintf(stderr, "Failed to find dimensions.\n");
    return 1;
}
