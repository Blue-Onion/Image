#include "image_processor.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <sys/stat.h>
#include <stdint.h>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

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
    return image_save_with_quality(img, filename, 85);
}

int image_save_with_quality(Image* img, const char* filename, int quality) {
    if (!img || !img->data) return 0;
    
    const char* ext = strrchr(filename, '.');
    if (!ext) return 0;
    
    ext++; // Skip the dot
    
    if (strcmp(ext, "png") == 0) {
        return stbi_write_png(filename, img->width, img->height, img->channels, img->data, img->width * img->channels);
    } else if (strcmp(ext, "jpg") == 0 || strcmp(ext, "jpeg") == 0) {
        // Clamp quality between 1 and 100
        if (quality < 1) quality = 1;
        if (quality > 100) quality = 100;
        return stbi_write_jpg(filename, img->width, img->height, img->channels, img->data, quality);
    } else if (strcmp(ext, "bmp") == 0) {
        return stbi_write_bmp(filename, img->width, img->height, img->channels, img->data);
    }
    
    return 0;
}

int get_file_size(const char* filename) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        return (int)st.st_size;
    }
    return 0;
}

Image* image_resize(Image* img, int new_width, int new_height) {
    if (!img || !img->data || new_width <= 0 || new_height <= 0) return NULL;
    
    // Create new image with target dimensions
    Image* resized = image_create(new_width, new_height, img->channels);
    if (!resized) return NULL;
    
    // Bilinear interpolation for resizing
    float x_ratio = (float)img->width / (float)new_width;
    float y_ratio = (float)img->height / (float)new_height;
    
    for (int y = 0; y < new_height; y++) {
        for (int x = 0; x < new_width; x++) {
            float src_x = (x + 0.5f) * x_ratio - 0.5f;
            float src_y = (y + 0.5f) * y_ratio - 0.5f;
            
            int x1 = (int)src_x;
            int y1 = (int)src_y;
            int x2 = (x1 + 1 < img->width) ? x1 + 1 : x1;
            int y2 = (y1 + 1 < img->height) ? y1 + 1 : y1;
            
            float fx = src_x - x1;
            float fy = src_y - y1;
            
            for (int c = 0; c < img->channels; c++) {
                // Bilinear interpolation
                float p1 = img->data[(y1 * img->width + x1) * img->channels + c];
                float p2 = img->data[(y1 * img->width + x2) * img->channels + c];
                float p3 = img->data[(y2 * img->width + x1) * img->channels + c];
                float p4 = img->data[(y2 * img->width + x2) * img->channels + c];
                
                float p = (p1 * (1.0f - fx) + p2 * fx) * (1.0f - fy) +
                          (p3 * (1.0f - fx) + p4 * fx) * fy;
                
                resized->data[(y * new_width + x) * img->channels + c] = (uint8_t)(p + 0.5f);
            }
        }
    }
    
    return resized;
}

Image* image_compress_50_percent(Image* img, const char* output_file, float* size_reduction) {
    if (!img || !img->data) return NULL;
    
    // Save original to temp file to get original size
    const char* temp_original = "temp_original_comp_check.jpg";
    image_save_with_quality(img, temp_original, 95);
    long original_size = get_file_size(temp_original);
    
    // Strategy: Combine resizing (reduces dimensions by ~30%) and quality reduction
    // This typically achieves ~50% file size reduction
    
    // Calculate new dimensions (reduce by ~30% = 70% of original)
    int new_width = (int)(img->width * 0.70f);
    int new_height = (int)(img->height * 0.70f);
    
    // Ensure minimum size
    if (new_width < 1) new_width = 1;
    if (new_height < 1) new_height = 1;
    
    // Resize image
    Image* resized = image_resize(img, new_width, new_height);
    if (!resized) {
        remove(temp_original);
        return NULL;
    }
    
    // Try different quality levels to achieve ~50% reduction
    int quality = 75;
    long target_size = original_size / 2;  // 50% of original
    long best_size = 0;
    Image* best_result = NULL;
    int best_quality = quality;
    
    // Binary search for optimal quality
    int low_quality = 30;
    int high_quality = 90;
    
    for (int attempt = 0; attempt < 5; attempt++) {
        char temp_file[256];
        snprintf(temp_file, sizeof(temp_file), "temp_comp_%d.jpg", attempt);
        
        image_save_with_quality(resized, temp_file, quality);
        long current_size = get_file_size(temp_file);
        
        if (current_size <= target_size || (best_result == NULL || labs(current_size - target_size) < labs(best_size - target_size))) {
            if (best_result) {
                image_free(best_result);
            }
            best_result = image_load(temp_file);
            best_size = current_size;
            best_quality = quality;
        }
        
        // Adjust quality based on current size
        if (current_size > target_size) {
            high_quality = quality;
            quality = (low_quality + quality) / 2;
        } else {
            low_quality = quality;
            quality = (quality + high_quality) / 2;
        }
        
        remove(temp_file);
    }
    
    // Save final result using resized image with best quality found
    if (best_result) {
        image_free(best_result);  // Free the loaded temp result
    }
    
    // Save resized image with best quality found
    image_save_with_quality(resized, output_file, best_quality);
    long final_size = get_file_size(output_file);
    
    // If we haven't reached 50%, try adjusting quality one more time
    if (final_size > target_size && best_quality > 30) {
        best_quality = (int)(best_quality * 0.9f);  // Reduce quality by 10%
        if (best_quality < 30) best_quality = 30;
        image_save_with_quality(resized, output_file, best_quality);
        final_size = get_file_size(output_file);
    }
    
    // Calculate size reduction
    if (size_reduction && original_size > 0) {
        *size_reduction = (1.0f - (float)final_size / (float)original_size) * 100.0f;
    }
    
    // Load the final saved image
    Image* final_image = image_load(output_file);
    
    // Cleanup
    remove(temp_original);
    image_free(resized);
    
    return final_image;
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

