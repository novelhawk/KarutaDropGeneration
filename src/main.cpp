#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <webp/encode.h>
#include <png.h>

constexpr int SOURCE_CARD_WIDTH = 274;
constexpr int SOURCE_CARD_HEIGHT = 400;
constexpr int SOURCE_CARD_BIT_DEPTH = 8;
constexpr int SOURCE_CARD_COLOR_TYPE = PNG_COLOR_TYPE_RGB_ALPHA;

constexpr int OUTPUT_TOP_PADDING = 12;
constexpr int OUTPUT_SIDE_PADDING = 7;
constexpr int OUTPUT_BOTTOM_PADDING = 2;
constexpr int OUTPUT_HEIGHT = OUTPUT_TOP_PADDING + SOURCE_CARD_HEIGHT + OUTPUT_BOTTOM_PADDING;
constexpr float OUTPUT_QUALITY = 80;

bool read_png(FILE *fp, png_bytepp& row_pointers, png_uint_32& stride, png_uint_32& width, png_uint_32& height, int& bit_depth, int& color_type) 
{
    const int bytes_to_check = 8;
    uint8_t *header = new uint8_t[bytes_to_check];

    if (fread(header, 1, bytes_to_check, fp) != bytes_to_check) {
        fprintf(stderr, "Could not read the file header (8 bytes)\n");
        return false;
    }
    if (png_sig_cmp(header, 0, bytes_to_check)) {
        fprintf(stderr, "File header does not match PNG header.\n");
        return false;
    }

    delete[] header;
   
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        fprintf(stderr, "Failed to allocate png struct.\n");
        return false;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        fprintf(stderr, "Failed to allocate png info struct.\n");
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return false;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        fprintf(stderr, "Failed initializing libpng.\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return false;
    }

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, bytes_to_check);
    png_read_info(png_ptr, info_ptr);

    if (!png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL)) {
        fprintf(stderr, "Failed to read IHDR chunk.\n");
        return false;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        fprintf(stderr, "Failed to read image data.\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return false;
    }

    row_pointers = new png_bytep[height];
    stride = png_get_rowbytes(png_ptr, info_ptr);
    for (png_uint_32 y = 0; y < height; ++y) {
        row_pointers[y] = new png_byte[stride];
    }
    png_read_image(png_ptr, row_pointers);

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return true;
}

void read_card(const char* path, png_bytepp& row_pointers, png_uint_32& stride, png_uint_32& height)
{
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        fprintf(stderr, "File does not exists\n");
        abort();
    }

    png_uint_32 width;
    int bit_depth, color_type;
    if (!read_png(fp, row_pointers, stride, width, height, bit_depth, color_type)) {
        fprintf(stderr, "Failed to read png image. Aborting.\n");
        fclose(fp);
        abort();
    }

    fclose(fp);

    if (width != SOURCE_CARD_WIDTH || height != SOURCE_CARD_HEIGHT) {
        fprintf(stderr, "Supplied card is not %dx%d (size: %dx%d).\n", 
            SOURCE_CARD_WIDTH, SOURCE_CARD_HEIGHT, width, height);
        fclose(fp);
        abort();
    }

    if (bit_depth != SOURCE_CARD_BIT_DEPTH) {
        fprintf(stderr, "Supplied image has unsupported bit depth (%d)\n", bit_depth);
        fclose(fp);
        abort();
    }

    if (color_type != SOURCE_CARD_COLOR_TYPE) {
        fprintf(stderr, "Supplied image has unsupported color type (%d)\n", color_type);
        fclose(fp);
        abort();
    }
}

int main(int argc, char **argv) 
{
    int number_of_cards = argc - 1;
    char **card_paths = argv + 1;

    if (number_of_cards == 0) {
        fprintf(stderr, "Invalid number of arguments supplied.\n");
        return 1;
    }

    int output_width = SOURCE_CARD_WIDTH * number_of_cards + 2 * OUTPUT_SIDE_PADDING;
    int output_stride = output_width * 4;
    uint8_t *raw_output_image = new uint8_t[OUTPUT_HEIGHT * output_stride]();

    for (int i = 0; i < number_of_cards; i++) {
        png_bytepp row_pointers;
        png_uint_32 card_stride, height;
        read_card(card_paths[i], row_pointers, card_stride, height);

        for (png_uint_32 y = 0; y < height; ++y) {
            int out_x = OUTPUT_SIDE_PADDING + i * SOURCE_CARD_WIDTH;
            int out_y = OUTPUT_TOP_PADDING + y;
            memcpy(&raw_output_image[out_x * 4 + out_y * output_stride], row_pointers[y], card_stride);
        }

        for (png_uint_32 y = 0; y < height; ++y) {
            delete[] row_pointers[y];
        }
        delete[] row_pointers;
    }
    
    uint8_t *webp_output;

    size_t webp_size = WebPEncodeRGBA(raw_output_image, output_width, OUTPUT_HEIGHT, output_stride, OUTPUT_QUALITY, &webp_output);
    
    FILE *out_file = fopen("output.webp", "w");
    if (!out_file) {
        fprintf(stderr, "Failed to create output.webp\n");
        return 1;
    }
    int wrote_bytes = fwrite(webp_output, sizeof(uint8_t), webp_size, out_file);
    if (wrote_bytes != webp_size) {
        fprintf(stderr, "Failed to write image data to output.webp\n");
        return 1;
    }
    fclose(out_file);
    
    WebPFree(webp_output);

    delete[] raw_output_image;

    return 0;
}