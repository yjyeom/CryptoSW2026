#include <stdio.h>

#define MAX_SIZE 10
#define NEARLY_ZERO 1e-10 // 10^(-10)보다 작은 절대값은 0으로 간주

//행렬을 나타내는 구조체
typedef struct {
	double M[MAX_SIZE][MAX_SIZE];
	int row; //행
	int col; //열
} Matrix;

// 행렬 출력하기
//  [1 2]
//  [3 4]
void Matrix_print(Matrix A) {
	for (int i = 0; i < A.row; i++) {
		printf("[");
		for (int j = 0; j < A.col; j++) {
			if ((A.M[i][j] < NEARLY_ZERO) && (A.M[i][j] > -NEARLY_ZERO)) {
				printf("%7.2f", 0.0);
			}
			else {
				printf("%7.2f", A.M[i][j]);
			}
		}
		printf("]\n");
	}
	printf("\n");
}


//행렬 초기화 A = [0.0]
Matrix Matrix_Init() {
	Matrix A;
	A.row = 1; A.col = 1;
	A.M[0][0] = 0.0;
	return A;
}

//행렬 덧셈 C = A+B
Matrix Matrix_Add(Matrix A, Matrix B) {
	Matrix C;
	C = Matrix_Init();
	
	if ((A.col != B.col) || (A.row != B.row)) { // A,B의 모양이 같아야 한다.
		printf("(Matrix_Add) Matrix Size error!\n");
		return C;
	}

	C.row = A.row;
	C.col = A.col;
	for (int i = 0; i < A.row; i++) {
		for (int j = 0; j < A.col; j++) {
			C.M[i][j] = A.M[i][j] + B.M[i][j];
		}
	}
	return C;
}

// 행렬 곱셈 C = A*B
Matrix Matrix_Mul(Matrix A, Matrix B) {
	Matrix C;
	C = Matrix_Init();

	// A[m,k] B[k,n] --> C[m,n] 
	// 크기 확인 (A의 열의 개수 ==  B의 행의 개수)
	if (A.col != B.row) {
		printf("(Matrix_Mul) Matrix Size Error!\n");
		return C;
	}

	C.row = A.row;
	C.col = B.col;
	for (int i = 0; i < C.row; i++) {
		for (int j = 0; j < C.col; j++) {
			// C_ij = a_i1*b_1j + a_i2*b_2j + ... + a_ik*b_kj
			C.M[i][j] = 0.0;
			for (int t = 0; t < A.col; t++) {
				C.M[i][j] += A.M[i][t] * B.M[t][j];
			}
		}
	}
	return C;
}

//역행렬을 구하기 위한 기본 연산들

//두 행을 바꾸기 (참조 & 기호)
void Mat_Exchange_Row(Matrix& A, int row1, int row2) {
	double temp;
	// A[row1][j] <--> A[row2][j]
	for (int j = 0; j < A.col; j++) {
		temp = A.M[row1][j];
		A.M[row1][j] = A.M[row2][j];
		A.M[row2][j] = temp;
	}
}

// 한 행에 상수배 하기
void Mat_Scalar_Mul_Row(Matrix& A, double k, int row) {
	for (int j = 0; j < A.col; j++) {
		A.M[row][j] *= k;
	}
}

// 한 행의 상수배를 다른 행에 다하기
void Mat_Row_Add(Matrix& A, double k, int row_src, int row_target) {
	for (int j = 0; j < A.col; j++) {
		A.M[row_target][j] += k * A.M[row_src][j];
	}
}

//역행렬 구하기
Matrix Matrix_Inverse(Matrix A) {
	Matrix InvA;
	InvA = Matrix_Init();

	if (A.row != A.col) { // 정사각행렬이 아니면,
		printf("(Matrix_Inverse) Non-Square Matrix Error!\n");
		return InvA;
	}

	// AA = [A|I] 
	Matrix AA;
	AA.row = A.row;
	AA.col = A.col * 2;
	for (int i = 0; i < A.row; i++) {
		for (int j = 0; j < A.col; j++) {
			AA.M[i][j] = A.M[i][j];
			AA.M[i][A.col + j] = (i == j) ? 1 : 0;
		}
	}

	Matrix_print(AA);

	// [A|I] -----> [I|A^(-1)]  R-REF (Reduced Row Echelon Form)

	int pivot_row; // 각 열을 단위행렬 모양으로 만들때 '1'의 위치를 가지는 열
	for (int j = 0; j < A.col; j++) {  // 한번에 한 열씩 단위행렬 I 모양으로,
		pivot_row = -1; // 초깃값
		for (int i = j; i < A.row; i++) {  // AA[j][j], AA[j+1][j],..., AA[*][j]
			//if (AA.M[i][j] != 0.0) {  // 실수에서는 0근처의 값을 0으로 간주
			if ((AA.M[i][j] > NEARLY_ZERO) || (AA.M[i][j] < -NEARLY_ZERO)) {
				pivot_row = i;
				break; // for-loop 밖으로
			}
		}
		if (pivot_row == -1) { // 역행렬이 존재하지 않는다
			printf("(Matrix_Inverse) Not Invertible Matrix\n");
			return InvA;
		}
		if (pivot_row != j) { // j행과 pivot_row행을 바꾼다
			Mat_Exchange_Row(AA, j, pivot_row);
		}
		Mat_Scalar_Mul_Row(AA, 1. / AA.M[j][j], j);

		for (int i = 0; i < AA.row; i++) {
			if (i != j) {  //자기자신을 제외한 행에 대하여
				Mat_Row_Add(AA, -AA.M[i][j], j, i);
			}
		}
		Matrix_print(AA);
	}

	InvA.row = A.row;
	InvA.col = A.col;
	for (int i = 0; i < A.row; i++) {
		for (int j = 0; j < A.col; j++) {
			InvA.M[i][j] = AA.M[i][j + A.col];
		}
	}
	return InvA;
}

void test_print() {
	Matrix A, IA;

	A.row = 3;
	A.col = 3;
	IA.row = 3;
	IA.col = 3;
	double Mat[3][3] = { {1,1,2}, {1,1,0}, {1,2,0} };
	double IMat[3][3] = { { 0,2,-1}, {0,-1, 1}, {0.5,-0.5,0} };

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			A.M[i][j] = Mat[i][j];
			IA.M[i][j] = IMat[i][j];
		}
	}
	printf("A =\n");
	Matrix_print(A);
	printf("IA =\n");
	Matrix_print(IA);

	Matrix C;
	C = Matrix_Mul(A, IA);
	printf("C =\n");
	Matrix_print(C);

	Matrix D;
	D = Matrix_Inverse(A);
	printf("D =\n");
	Matrix_print(D);
}

int main() {
	test_print();
	return 0;
}