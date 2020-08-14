#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <webp/encode.h>
#include <png.h>

constexpr int PNG_HEADER_SIZE = 8;

constexpr int SOURCE_CARD_WIDTH = 274;
constexpr int SOURCE_CARD_HEIGHT = 400;
constexpr int SOURCE_CARD_BIT_DEPTH = 8;
constexpr int SOURCE_CARD_COLOR_TYPE = PNG_COLOR_TYPE_RGB_ALPHA;

constexpr int OUTPUT_TOP_PADDING = 12;
constexpr int OUTPUT_SIDE_PADDING = 7;
constexpr int OUTPUT_BOTTOM_PADDING = 2;
constexpr int OUTPUT_HEIGHT = OUTPUT_TOP_PADDING + SOURCE_CARD_HEIGHT + OUTPUT_BOTTOM_PADDING;
constexpr float OUTPUT_QUALITY = 80;

struct Rgba32 {
    uint8_t red;
    uint8_t gren;
    uint8_t blue;
    uint8_t alpha;
};

bool is_png_file(FILE *fp)
{
    uint8_t *header = new uint8_t[PNG_HEADER_SIZE];

    int read_bytes = fread(header, 1, PNG_HEADER_SIZE, fp);
    if (read_bytes != PNG_HEADER_SIZE) {
        fprintf(stderr, "Could not read the file header (%d bytes)\n", PNG_HEADER_SIZE);
        return false;
    }

    bool has_png_header = !png_sig_cmp(header, 0, PNG_HEADER_SIZE);

    delete[] header;

    return has_png_header;
}

bool init_libpng(FILE *fp, png_structp& png_ptr, png_infop& info_ptr)
{
    // Setup libpng error long jump destination
    if (setjmp(png_jmpbuf(png_ptr))) {
        fprintf(stderr, "Failed initializing libpng.\n");
        return false;
    }

    // Move FILE position to after the header
    fseek(fp, PNG_HEADER_SIZE, SEEK_SET);
    // Tell libpng we already read the header
    png_set_sig_bytes(png_ptr, PNG_HEADER_SIZE);

    png_init_io(png_ptr, fp);
    png_read_info(png_ptr, info_ptr);
    return true;
}

bool read_png(
    png_structp& png_ptr,
    png_infop& info_ptr,
    png_bytepp& row_pointers,
    png_uint_32& stride,
    png_uint_32& width,
    png_uint_32& height,
    int& bit_depth,
    int& color_type)
{
    // Setup libpng error long jump destination
    if (setjmp(png_jmpbuf(png_ptr))) {
        fprintf(stderr, "Failed to read image data.\n");
        return false;
    }

    // Gets image informations from IHDR block
    if (!png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL)) {
        fprintf(stderr, "Failed to read IHDR chunk.\n");
        return false;
    }

    // Setup destination buffers
    row_pointers = new png_bytep[height];
    stride = png_get_rowbytes(png_ptr, info_ptr);
    for (png_uint_32 y = 0; y < height; ++y) {
        row_pointers[y] = new png_byte[stride];
    }

    // Read image
    png_read_image(png_ptr, row_pointers);
    return true;
}

bool load_png(
    FILE *fp,
    png_bytepp& row_pointers,
    png_uint_32& stride,
    png_uint_32& width,
    png_uint_32& height,
    int& bit_depth,
    int& color_type) 
{
    // Check for png header
    if (!is_png_file) {
        fprintf(stderr, "File header does not match PNG header.\n");
        return false;
    }

    // Create libpng state structs
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

    // Init libpng
    if (!init_libpng(fp, png_ptr, info_ptr)) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return false;
    }

    // Read image
    if (!read_png(png_ptr, info_ptr, row_pointers, stride, width, height, bit_depth, color_type)) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return false;
    }

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
    if (!load_png(fp, row_pointers, stride, width, height, bit_depth, color_type)) {
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
    Rgba32 *raw_output_image = new Rgba32[OUTPUT_HEIGHT * output_width]();

    for (int i = 0; i < number_of_cards; i++) {
        png_bytepp row_pointers;
        png_uint_32 card_stride, height;
        read_card(card_paths[i], row_pointers, card_stride, height);

        int card_offset = i * SOURCE_CARD_WIDTH;
        for (png_uint_32 y = 0; y < height; ++y) {
            int out_x = OUTPUT_SIDE_PADDING + card_offset;
            int out_y = OUTPUT_TOP_PADDING + y;
            memcpy(&raw_output_image[out_x + out_y * output_width], row_pointers[y], card_stride);
        }

        for (png_uint_32 y = 0; y < height; ++y) {
            delete[] row_pointers[y];
        }
        delete[] row_pointers;
    }
    
    uint8_t *webp_output;
    size_t webp_size = WebPEncodeRGBA((const uint8_t *)raw_output_image, output_width, 
        OUTPUT_HEIGHT, output_width * sizeof(Rgba32), OUTPUT_QUALITY, &webp_output);
    
    {
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
    }
    
    WebPFree(webp_output);

    delete[] raw_output_image;

    return 0;
}