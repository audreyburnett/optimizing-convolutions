#include <omp.h>
#include <x86intrin.h>

#include "compute.h"

// Computes the convolution of two matrices
int convolve(matrix_t *a_matrix, matrix_t *b_matrix, matrix_t **output_matrix) {
  // TODO: convolve matrix a and matrix b, and store the resulting matrix in
  // output_matrix
  
  // create output_matrix
  int num_rows = (a_matrix -> rows) - (b_matrix -> rows) + 1;
  int num_cols = (a_matrix -> cols) - (b_matrix -> cols) + 1;
  
  *output_matrix = malloc(sizeof(matrix_t));
  (*output_matrix)->data = malloc(sizeof(int32_t) * num_rows * num_cols);
  (*output_matrix)->rows = num_rows;
  (*output_matrix)->cols = num_cols;

  //create and fill flip_matrix, which is a flipped version of b
  matrix_t* flip_matrix = malloc(sizeof(matrix_t));
  flip_matrix->rows = b_matrix->rows;
  flip_matrix->cols = b_matrix->cols;
  int len = b_matrix->rows * b_matrix->cols;
  flip_matrix->data = malloc(sizeof(int32_t) * len);

  #pragma omp parallel for
  for(int i = 0; i < len; i++) {
    *(flip_matrix->data + i) = *(b_matrix->data + len - 1 - i);
  }


  #pragma omp parallel for collapse(2)
  for (int r = 0; r < num_rows; r ++) {
    for (int c = 0; c < num_cols; c ++) {
        int sum = 0;
        __m256i sum_vec = _mm256_set1_epi32(0);
        __m256i flip_vec;
        __m256i a_vec;
        __m256i mul_vec;
        int j = ((flip_matrix->cols)/8 * 8) + c;
        for (int i = r; i < (row_b + r); i++) {            
            for (j = c; j < (((col_b)/32 * 32)) + c; j+= 32) {
                flip_vec = _mm256_loadu_si256( (__m256i *) (data_flip + (j-c) + (col_b*(i-r))));
                a_vec = _mm256_loadu_si256( (__m256i *) (data_a + j + (col_a*i)));
                mul_vec = _mm256_mullo_epi32(flip_vec, a_vec);
                sum_vec = _mm256_add_epi32(sum_vec, mul_vec);

                flip_vec = _mm256_loadu_si256( (__m256i *) (data_flip + 8 + (j-c) + (col_b*(i-r))));
                a_vec = _mm256_loadu_si256( (__m256i *) (data_a + 8 + j + (col_a*i)));
                mul_vec = _mm256_mullo_epi32(flip_vec, a_vec);
                sum_vec = _mm256_add_epi32(sum_vec, mul_vec);
                
                flip_vec = _mm256_loadu_si256( (__m256i *) (data_flip + 16 + (j-c) + (col_b*(i-r))));
                a_vec = _mm256_loadu_si256( (__m256i *) (data_a + 16 + j + (col_a*i)));
                mul_vec = _mm256_mullo_epi32(flip_vec, a_vec);
                sum_vec = _mm256_add_epi32(sum_vec, mul_vec);

                flip_vec = _mm256_loadu_si256( (__m256i *) (data_flip + 24 + (j-c) + (col_b*(i-r))));
                a_vec = _mm256_loadu_si256( (__m256i *) (data_a + 24 + j + (col_a*i)));
                mul_vec = _mm256_mullo_epi32(flip_vec, a_vec);
                sum_vec = _mm256_add_epi32(sum_vec, mul_vec);

            }

            for (; j < (((col_b)/8 * 8)) + c; j+= 8) {
                flip_vec = _mm256_loadu_si256( (__m256i *) (data_flip + (j-c) + (col_b*(i-r))));
                a_vec = _mm256_loadu_si256( (__m256i *) (data_a + j + (col_a*i)));
                mul_vec = _mm256_mullo_epi32(flip_vec, a_vec);
                sum_vec = _mm256_add_epi32(sum_vec, mul_vec);
            } 
            for(; j < col_b + c; j++) {
                sum += *(data_a + j + (col_a*i))* *(data_flip + (j-c) + (col_b*(i-r)));
            }

        }

        if(j > c) { 
            int arr[8];
            
            _mm256_storeu_si256((__m256i *) arr, sum_vec);
            sum += arr[0] + arr[1] + arr[2] + arr[3] + arr[4] + arr[5] + arr[6] + arr[7];
        }

        *((*output_matrix)->data + (c + r*num_cols)) = sum;
      }
  }

  return 0;
}

// Executes a task
int execute_task(task_t *task) {
  matrix_t *a_matrix, *b_matrix, *output_matrix;

  char *a_matrix_path = get_a_matrix_path(task);
  if (read_matrix(a_matrix_path, &a_matrix)) {
    printf("Error reading matrix from %s\n", a_matrix_path);
    return -1;
  }
  free(a_matrix_path);

  char *b_matrix_path = get_b_matrix_path(task);
  if (read_matrix(b_matrix_path, &b_matrix)) {
    printf("Error reading matrix from %s\n", b_matrix_path);
    return -1;
  }
  free(b_matrix_path);

  if (convolve(a_matrix, b_matrix, &output_matrix)) {
    printf("convolve returned a non-zero integer\n");
    return -1;
  }

  char *output_matrix_path = get_output_matrix_path(task);
  if (write_matrix(output_matrix_path, output_matrix)) {
    printf("Error writing matrix to %s\n", output_matrix_path);
    return -1;
  }
  free(output_matrix_path);

  free(a_matrix->data);
  free(b_matrix->data);
  free(output_matrix->data);
  free(a_matrix);
  free(b_matrix);
  free(output_matrix);
  return 0;
}
