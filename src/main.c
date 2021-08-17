#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <webp/encode.h>
#include <png.h>

#define PNG_HEADER_SIZE (8)

#define SOURCE_CARD_WIDTH (274)
#define SOURCE_CARD_HEIGHT (400)
#define SOURCE_CARD_BIT_DEPTH (8)
#define SOURCE_CARD_COLOR_TYPE (PNG_COLOR_TYPE_RGB_ALPHA)
#define SOURCE_CARD_STRIDE (SOURCE_CARD_WIDTH * 4)

#define OUTPUT_TOP_PADDING (12)
#define OUTPUT_SIDE_PADDING (7)
#define OUTPUT_BOTTOM_PADDING (2)
#define OUTPUT_HEIGHT (OUTPUT_TOP_PADDING + SOURCE_CARD_HEIGHT + OUTPUT_BOTTOM_PADDING)
#define OUTPUT_QUALITY (80)

#define OUT_OF_RAM (2)
#define USER_ERROR (3)

#define PNG_ERROR_HANDLER(ptr) (setjmp(png_jmpbuf(ptr)))

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;
} rgba32_t;

typedef struct {
    char *path;
    png_uint_32 width;
    png_uint_32 height;
    int bit_depth;
    int color_type;
    int interlace_method;
    int compression_method;
    int filter_method;
    png_uint_32 stride;
} image_info_t;

int is_png_file(FILE *fp)
{
    uint8_t *header = (uint8_t *) malloc(PNG_HEADER_SIZE);

    if (!header) {
        fprintf(stderr, "Out of memory: Couldn't allocate %d bytes", PNG_HEADER_SIZE);
        exit(OUT_OF_RAM);
    }

    int read_bytes = fread(header, 1, PNG_HEADER_SIZE, fp);
    if (read_bytes != PNG_HEADER_SIZE) {
        fprintf(stderr, "Could not read file header (%d bytes)\n", PNG_HEADER_SIZE);
        free(header);
        return 0;
    }

    int has_png_header = !png_sig_cmp(header, 0, PNG_HEADER_SIZE);
    free(header);
    return has_png_header;
}

int read_png(png_structp png_ptr, png_infop info_ptr, png_bytepp *rows, image_info_t *image_info) {
    // Setup libpng error long jump destination
    if (PNG_ERROR_HANDLER(png_ptr)) {
        fprintf(stderr, "Failed to read image data.\n");
        // libpng structs are destroyed by the caller
        return 0;
    }

    // Gets image informations from IHDR block
    int successful = png_get_IHDR(
        png_ptr,
        info_ptr,
        &image_info->width,
        &image_info->height,
        &image_info->bit_depth,
        &image_info->color_type,
        &image_info->filter_method,
        &image_info->compression_method,
        &image_info->interlace_method);

    if (!successful) {
        fprintf(stderr, "Failed to read IHDR chunk.\n");
        return 0;
    }

    // Get number of bytes in each row
    image_info->stride = png_get_rowbytes(png_ptr, info_ptr);

    // Setup destination buffers
    *rows = (png_bytepp) malloc(image_info->height * sizeof(png_bytep));
    for (png_uint_32 row = 0; row < image_info->height; ++row) {
        (*rows)[row] = (png_bytep) malloc(image_info->stride * sizeof(png_byte));
    }

    // Read image
    png_read_image(png_ptr, *rows);
    return 1;
}

int load_png(FILE *fp, png_bytepp *rows, image_info_t *image_info) 
{
    // Check for png header
    if (!is_png_file(fp)) {
        fprintf(stderr, "File header does not match PNG header.\n");
        return 0;
    }

    // Create libpng read struct
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        fprintf(stderr, "Failed to allocate png struct.\n");
        return 0;
    }

    // Handle initialization errors with libpng
    if (PNG_ERROR_HANDLER(png_ptr)) {
        fprintf(stderr, "Failed to initialize libpng\n");
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return 0;
    }

    // Tell libpng which file we are reading
    png_init_io(png_ptr, fp);

    // Tell libpng that is_png_file has already read the first
    // PNG_HEADER_SIDE bytes of the header
    png_set_sig_bytes(png_ptr, PNG_HEADER_SIZE);

    // Create libpng info struct
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        fprintf(stderr, "Failed to allocate png info struct.\n");
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return 0;
    }

    // Update error handler to destroy info struct as well
    if (PNG_ERROR_HANDLER(png_ptr)) {
        fprintf(stderr, "Failed to initialize libpng\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return 0;
    }

    // Tell libpng that we created the read info struct and
    // link it to the read context
    png_read_info(png_ptr, info_ptr);

    // Read image
    if (!read_png(png_ptr, info_ptr, rows, image_info)) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return 0;
    }

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return 1;
}

int valid_source_card(image_info_t *info)
{
    int valid = 1;

    if (info->width != SOURCE_CARD_WIDTH) {
        fprintf(stderr, "Invalid input card width: Expected %d Found %d\n", SOURCE_CARD_WIDTH, info->width);
        valid = 0;
    }

    if (info->height != SOURCE_CARD_HEIGHT) {
        fprintf(stderr, "Invalid input card height: Expected %d Found %d\n", SOURCE_CARD_HEIGHT, info->height);
        valid = 0;
    }

    if (info->color_type != SOURCE_CARD_COLOR_TYPE) {
        fprintf(stderr, "Invalid input card color type: Expected %d Found %d\n", SOURCE_CARD_COLOR_TYPE, info->color_type);
        valid = 0;
    }

    if (info->bit_depth != SOURCE_CARD_BIT_DEPTH) {
        fprintf(stderr, "Invalid input card bit depth: Expected %d Found %d\n", SOURCE_CARD_BIT_DEPTH, info->bit_depth);
        valid = 0;
    }

    if (info->stride != SOURCE_CARD_STRIDE) {
        fprintf(stderr, "Invalid input card stride: Expected %d Found %d\n", SOURCE_CARD_STRIDE, info->stride);
        valid = 0;
    }

    return valid;
}

void read_card(image_info_t *image_info, png_bytepp *rows)
{
    FILE *fp = fopen(image_info->path, "rb");
    if (!fp) {
        fprintf(stderr, "File does not exists\n");
        abort();
    }

    if (!load_png(fp, rows, image_info)) {
        fprintf(stderr, "Failed to read '%s'. Aborting.\n", image_info->path);
        fclose(fp);
        abort();
    }

    fclose(fp);
}

void destroy_card(png_bytepp rows, image_info_t *image_info)
{
    for (int i = 0; i < image_info->height; ++i) {
        png_bytep row = rows[i];
        free(row);
    }

    free(rows);
}

void copy_cards_to_output(int cards, char **paths, rgba32_t *output, int output_width) {
    // Leave left and top padding in the output image
    output += OUTPUT_SIDE_PADDING + OUTPUT_TOP_PADDING * output_width;

    for (int i = 0; i < cards; i++) {
        // Read PNG
        image_info_t card_info;
        card_info.path = paths[i];

        png_bytepp card_rows;
        read_card(&card_info, &card_rows);

        // Check that the read card is supported and matches
        // the SOURCE_CARD_* properties defines
        if (!valid_source_card(&card_info)) {
            fprintf(stderr, "Card '%s' is invalid\n", card_info.path);
            abort();
        }

        // Copy raw image to the respective part of the big
        // output image
        for (int row = 0; row < SOURCE_CARD_HEIGHT; ++row) {
            memcpy(output + row * output_width, card_rows[row], SOURCE_CARD_STRIDE);
        }

        // Offset output by the size of the input card
        // for the next iteration
        output += SOURCE_CARD_WIDTH;

        // Free memory
        destroy_card(card_rows, &card_info);
    }
}

void save_binary(char *path, uint8_t *data, size_t size)
{
    FILE *fs = fopen(path, "wb");
    if (!fs) {
        fprintf(stderr, "Failed to open output file\n");
        abort();
    }

    int wrote_bytes = fwrite(data, sizeof(uint8_t), size, fs);
    if (wrote_bytes != size) {
        perror("Failed to save WebP image");
        abort();
    }

    fclose(fs);
}

int main(int argc, char *argv[])
{
    int cards = argc - 1;
    if (cards == 0) {
        fprintf(stderr, "Usage: %s [png images...]", argv[0]);
        return USER_ERROR;
    }

    // Compute the output image width
    int output_width = SOURCE_CARD_WIDTH * cards + 2 * OUTPUT_SIDE_PADDING;

    printf("Merging %d PNG images into a single WebP (%dx%d)\n", cards, output_width, OUTPUT_HEIGHT);

    // Allocate buffer for the output image raw data
    rgba32_t *raw_output = (rgba32_t *) calloc(OUTPUT_HEIGHT * output_width, sizeof(rgba32_t));

    // Copy the input cards inside the output image
    copy_cards_to_output(cards, argv + 1, raw_output, output_width);

    // Encode raw data with WebP
    uint8_t *webp_output;
    size_t webp_size = WebPEncodeRGBA((const uint8_t *)raw_output, output_width, OUTPUT_HEIGHT, output_width * sizeof(rgba32_t), OUTPUT_QUALITY, &webp_output);

    // Save WebP image to output.webp
    save_binary("output.webp", webp_output, webp_size);

    puts("Saved output.webp");
    
    // Free WebP buffers
    WebPFree(webp_output);

    // Free raw image buffer
    free(raw_output);

    return EXIT_SUCCESS;
}
