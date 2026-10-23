#ifndef RMATRIX_H
#define RMATRIX_H

#define MAX_SIZE 16
#define NEARLY_ZERO 1e-10 // 10^(-10)보다 작은 절대값은 0으로 간주

// 행렬을 나타내는 구조체
typedef struct {
    double M[MAX_SIZE][MAX_SIZE];
    int row; // 행
    int col; // 열
} Matrix;

void Matrix_print(Matrix A);
Matrix Matrix_Init();
Matrix Matrix_Add(Matrix A, Matrix B);
Matrix Matrix_Mul(Matrix A, Matrix B);
void Mat_Exchange_Row(Matrix& A, int row1, int row2);
void Mat_Scalar_Mul_Row(Matrix& A, double k, int row);
void Mat_Row_Add(Matrix& A, double k, int row_src, int row_target);
Matrix Matrix_Inverse(Matrix A);
void test_print();

#endif