#include "sparse_matrix.h"
#include <string.h>
#include <stdio.h>

#define INITIAL_CAPACITY 1024

SparseMatrix* sparse_matrix_create(int rows, int cols, uint8_t threshold) {
    SparseMatrix* matrix = (SparseMatrix*)malloc(sizeof(SparseMatrix));
    if (!matrix) return NULL;
    
    matrix->rows = rows;
    matrix->cols = cols;
    matrix->threshold = threshold;
    matrix->size = 0;
    matrix->capacity = INITIAL_CAPACITY;
    matrix->data = (SparseNode*)malloc(sizeof(SparseNode) * matrix->capacity);
    
    if (!matrix->data) {
        free(matrix);
        return NULL;
    }
    
    return matrix;
}

void sparse_matrix_free(SparseMatrix* matrix) {
    if (matrix) {
        if (matrix->data) {
            free(matrix->data);
        }
        free(matrix);
    }
}

void sparse_matrix_add(SparseMatrix* matrix, int row, int col, uint8_t value) {
    if (value <= matrix->threshold) {
        return; // Skip values below threshold
    }
    
    // Check if we need to resize
    if (matrix->size >= matrix->capacity) {
        matrix->capacity *= 2;
        matrix->data = (SparseNode*)realloc(matrix->data, sizeof(SparseNode) * matrix->capacity);
    }
    
    matrix->data[matrix->size].row = row;
    matrix->data[matrix->size].col = col;
    matrix->data[matrix->size].value = value;
    matrix->size++;
}

SparseMatrix* sparse_matrix_from_dense(uint8_t* dense, int rows, int cols, uint8_t threshold) {
    SparseMatrix* sparse = sparse_matrix_create(rows, cols, threshold);
    if (!sparse) return NULL;
    
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            uint8_t value = dense[i * cols + j];
            if (value > threshold) {
                sparse_matrix_add(sparse, i, j, value);
            }
        }
    }
    
    return sparse;
}

void sparse_matrix_to_dense(SparseMatrix* sparse, uint8_t* dense) {
    // Initialize all to zero
    memset(dense, 0, sparse->rows * sparse->cols * sizeof(uint8_t));
    
    // Fill in non-zero values
    for (int i = 0; i < sparse->size; i++) {
        int idx = sparse->data[i].row * sparse->cols + sparse->data[i].col;
        dense[idx] = sparse->data[i].value;
    }
}

float sparse_matrix_compression_ratio(SparseMatrix* sparse) {
    int dense_size = dense_matrix_get_size_bytes(sparse->rows, sparse->cols);
    int sparse_size = sparse_matrix_get_size_bytes(sparse);
    
    if (sparse_size == 0) return 0.0f;
    return (float)dense_size / sparse_size;
}

int sparse_matrix_get_size_bytes(SparseMatrix* sparse) {
    return sizeof(SparseMatrix) + (sparse->size * sizeof(SparseNode));
}

int dense_matrix_get_size_bytes(int rows, int cols) {
    return rows * cols * sizeof(uint8_t);
}

