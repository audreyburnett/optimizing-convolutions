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

  for(int i = 0; i < len; i++) {
    *(flip_matrix->data + i) = *(b_matrix->data + len - 1 - i);
  }

  //compute the covolution
  for (int r = 0; r < num_rows; r ++) {
      for (int c = 0; c < num_cols; c ++) {
        uint32_t col_a = a_matrix -> cols;
        uint32_t col_b = b_matrix -> cols;
        int sum = 0;
        for (int i = r; i < flip_matrix -> rows + r; i ++) {
            for (int j = c; j < flip_matrix -> cols + c; j ++) {
                sum += *(a_matrix -> data + (j) + (col_a*i))* *(flip_matrix -> data + (j-c) + (col_b*(i-r)));
            }
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
