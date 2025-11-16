#ifndef IMAGE_PROCESSOR_H
#define IMAGE_PROCESSOR_H

#include "sparse_matrix.h"
#include <stdint.h>

// Forward declarations - implementation in .c file
struct stbi_io_callbacks;

typedef struct {
    uint8_t* data;
    int width;
    int height;
    int channels;
} Image;

// Function declarations
Image* image_load(const char* filename);
void image_free(Image* img);
SparseMatrix** image_to_sparse_matrices(Image* img, uint8_t threshold);
Image* sparse_matrices_to_image(SparseMatrix** sparse_channels, int channels);
int image_save(Image* img, const char* filename);
Image* image_create(int width, int height, int channels);
float calculate_total_compression_ratio(SparseMatrix** sparse_channels, int channels, int width, int height);

#endif // IMAGE_PROCESSOR_H

