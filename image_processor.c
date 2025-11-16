#include "image_processor.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

Image* image_load(const char* filename) {
    Image* img = (Image*)malloc(sizeof(Image));
    if (!img) return NULL;
    
    img->data = stbi_load(filename, &img->width, &img->height, &img->channels, 0);
    if (!img->data) {
        free(img);
        return NULL;
    }
    
    return img;
}

void image_free(Image* img) {
    if (img) {
        if (img->data) {
            stbi_image_free(img->data);
        }
        free(img);
    }
}

Image* image_create(int width, int height, int channels) {
    Image* img = (Image*)malloc(sizeof(Image));
    if (!img) return NULL;
    
    img->width = width;
    img->height = height;
    img->channels = channels;
    img->data = (uint8_t*)malloc(width * height * channels * sizeof(uint8_t));
    
    if (!img->data) {
        free(img);
        return NULL;
    }
    
    return img;
}

SparseMatrix** image_to_sparse_matrices(Image* img, uint8_t threshold) {
    if (!img || !img->data) return NULL;
    
    SparseMatrix** sparse_channels = (SparseMatrix**)malloc(sizeof(SparseMatrix*) * img->channels);
    if (!sparse_channels) return NULL;
    
    for (int ch = 0; ch < img->channels; ch++) {
        uint8_t* channel_data = (uint8_t*)malloc(img->width * img->height * sizeof(uint8_t));
        if (!channel_data) {
            // Cleanup on error
            for (int i = 0; i < ch; i++) {
                sparse_matrix_free(sparse_channels[i]);
            }
            free(sparse_channels);
            return NULL;
        }
        
        // Extract channel data
        for (int i = 0; i < img->width * img->height; i++) {
            channel_data[i] = img->data[i * img->channels + ch];
        }
        
        // Convert to sparse matrix
        sparse_channels[ch] = sparse_matrix_from_dense(channel_data, img->height, img->width, threshold);
        free(channel_data);
        
        if (!sparse_channels[ch]) {
            // Cleanup on error
            for (int i = 0; i < ch; i++) {
                sparse_matrix_free(sparse_channels[i]);
            }
            free(sparse_channels);
            return NULL;
        }
    }
    
    return sparse_channels;
}

Image* sparse_matrices_to_image(SparseMatrix** sparse_channels, int channels) {
    if (!sparse_channels || channels == 0) return NULL;
    
    int width = sparse_channels[0]->cols;
    int height = sparse_channels[0]->rows;
    
    Image* img = image_create(width, height, channels);
    if (!img) return NULL;
    
    for (int ch = 0; ch < channels; ch++) {
        uint8_t* channel_data = (uint8_t*)malloc(width * height * sizeof(uint8_t));
        if (!channel_data) {
            image_free(img);
            return NULL;
        }
        
        sparse_matrix_to_dense(sparse_channels[ch], channel_data);
        
        // Copy channel data back to image
        for (int i = 0; i < width * height; i++) {
            img->data[i * channels + ch] = channel_data[i];
        }
        
        free(channel_data);
    }
    
    return img;
}

int image_save(Image* img, const char* filename) {
    if (!img || !img->data) return 0;
    
    const char* ext = strrchr(filename, '.');
    if (!ext) return 0;
    
    ext++; // Skip the dot
    
    if (strcmp(ext, "png") == 0) {
        return stbi_write_png(filename, img->width, img->height, img->channels, img->data, img->width * img->channels);
    } else if (strcmp(ext, "jpg") == 0 || strcmp(ext, "jpeg") == 0) {
        return stbi_write_jpg(filename, img->width, img->height, img->channels, img->data, 90);
    } else if (strcmp(ext, "bmp") == 0) {
        return stbi_write_bmp(filename, img->width, img->height, img->channels, img->data);
    }
    
    return 0;
}

float calculate_total_compression_ratio(SparseMatrix** sparse_channels, int channels, int width, int height) {
    if (!sparse_channels || channels == 0) return 0.0f;
    
    int total_dense_size = dense_matrix_get_size_bytes(height, width) * channels;
    int total_sparse_size = sizeof(SparseMatrix*) * channels;
    
    for (int i = 0; i < channels; i++) {
        total_sparse_size += sparse_matrix_get_size_bytes(sparse_channels[i]);
    }
    
    if (total_sparse_size == 0) return 0.0f;
    return (float)total_dense_size / total_sparse_size;
}

