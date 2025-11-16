#ifndef SPARSE_MATRIX_H
#define SPARSE_MATRIX_H

#include <stdint.h>
#include <stdlib.h>

// Node structure for sparse matrix (COO format - Coordinate format)
typedef struct {
    int row;
    int col;
    uint8_t value;
} SparseNode;

// Sparse matrix structure
typedef struct {
    SparseNode* data;
    int size;          // Number of non-zero elements
    int capacity;      // Allocated capacity
    int rows;          // Original matrix rows
    int cols;          // Original matrix cols
    uint8_t threshold; // Threshold below which values are considered zero
} SparseMatrix;

// Function declarations
SparseMatrix* sparse_matrix_create(int rows, int cols, uint8_t threshold);
void sparse_matrix_free(SparseMatrix* matrix);
void sparse_matrix_add(SparseMatrix* matrix, int row, int col, uint8_t value);
SparseMatrix* sparse_matrix_from_dense(uint8_t* dense, int rows, int cols, uint8_t threshold);
void sparse_matrix_to_dense(SparseMatrix* sparse, uint8_t* dense);
float sparse_matrix_compression_ratio(SparseMatrix* sparse);
int sparse_matrix_get_size_bytes(SparseMatrix* sparse);
int dense_matrix_get_size_bytes(int rows, int cols);

#endif // SPARSE_MATRIX_H

