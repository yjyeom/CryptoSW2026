// GF(2)^8 --> GF(2)^8의 아핀 변환의 성질


#include <stdio.h>
#include "GF_256.h"

#define MATRIX_MAX_SIZE 20 // [A|I] 까지를 고려하여 넉넉하게
//typedef unsigned char byte;

typedef struct {  // GF(2) 행렬을 나타내는 구조체
    byte M[MATRIX_MAX_SIZE][MATRIX_MAX_SIZE];  // 0 또는 1만 저장한다.
    int row;
    int col;
} GF2_Matrix;

void GF2_Matrix_print(GF2_Matrix A) {
    for (int i = 0; i < A.row; i++) {  //각 행에 대하여
        printf("[");
        for (int j = 0; j < A.col; j++) {
            printf(" %1d", A.M[i][j]); // 0 또는 1만 출력
        }
        printf("]\n");
    }
    printf("\n");
}

// 행렬 변수의 초기화
GF2_Matrix GF2_Matrix_Init() {
    GF2_Matrix A;
    A.row = 1; A.col = 1;
    A.M[0][0] = (byte)0;

    return A;
}

// C = A+B (행렬의 덧셈)
GF2_Matrix GF2_Matrix_Add(GF2_Matrix A, GF2_Matrix B) {
    GF2_Matrix result;

    result = GF2_Matrix_Init();
    // 행렬의 규격 확인
    if ((A.col != B.col) || (A.row != B.row)) {
        printf("(GF2_Matrix_Add) Matrix size error\n");
        return result;
    }
    result.row = A.row;
    result.col = A.col;
    for (int i = 0; i < result.row; i++)
        for (int j = 0; j < result.col; j++)
            //result.M[i][j] = (A.M[i][j] + B.M[i][j]) % 2;  // 2로 나눈 나머지
            result.M[i][j] = A.M[i][j] ^ B.M[i][j];  // xor
    return result;
}

// C = A*B (행렬 곱)  [m,k]*[k,n] = [m,n] GF(2)의 곱 (0*0, 0*1, 1*0, 1*1)
GF2_Matrix GF2_Matrix_Mul(GF2_Matrix A, GF2_Matrix B) {
    GF2_Matrix AB;
    AB = GF2_Matrix_Init();

    //행렬의 규격 확인 
    if (A.col != B.row) {
        printf("(GF2_Matrix_Mul) Matrix size error\n");
        return AB;
    }

    AB.row = A.row;
    AB.col = B.col;
    for (int i = 0; i < AB.row; i++) {
        for (int j = 0; j < AB.col; j++) {
            AB.M[i][j] = 0;
            for (int k = 0; k < A.col; k++) {
                AB.M[i][j] ^= A.M[i][k] * B.M[k][j];
            }
        }
    }
    return AB;
}

//두 행을 바꾸기
void GF2_Mat_Exchange_Row(GF2_Matrix& A, int row1, int row2) {
    byte temp; // temp는 0 또는 1
    for (int j = 0; j < A.col; j++) {
        //A[row1][j] <--> A[row2][j]
        temp = A.M[row1][j];
        A.M[row1][j] = A.M[row2][j];
        A.M[row2][j] = temp;
    }
}

//한 행에 상수배 하기
void GF2_Mat_Scalar_Mul_Row(GF2_Matrix& A, byte k, int row) {
    // k(상수)는 0 또는 1
    for (int j = 0; j < A.col; j++)
        //A.M[row][j] = (k == 1) ? A.M[row][j] : 0;
        A.M[row][j] *= k;
}

//한 행의 상수배를 다른 행에 더하기
void GF2_Mat_Row_Add(GF2_Matrix& A, byte k, int row_src, int row_target) {
    for (int j = 0; j < A.col; j++)
        A.M[row_target][j] ^= k * A.M[row_src][j];  // xor 
}

//역행렬 구하기
GF2_Matrix GF2_Matrix_Inverse(GF2_Matrix A) {
    GF2_Matrix InvA;
    InvA = GF2_Matrix_Init();
    if (A.row != A.col) { // 정사각행렬인지 확인
        printf("(GF2_Matrix_Inverse) Non-Sqaure Matrix error\n");
        return InvA;
    }

    // AA = [A|I] -------> [I|A^(-1)]
    GF2_Matrix AA;
    AA.row = A.row;
    AA.col = 2 * A.col;
    for (int i = 0; i < A.row; i++) {
        for (int j = 0; j < A.col; j++) {
            AA.M[i][j] = A.M[i][j];
            AA.M[i][j + A.col] = (i == j) ? 1 : 0;
        }
    }

    //GF2_Matrix_print(AA);

    // R-REF(Reduced Row Echelon Form) [A|I] ==> [I|A^(-1)]
    int pivot_row; // 각 열을 계산하는 단계에서 '1'이 있는 위치의 행
    for (int j = 0; j < A.col; j++) { // A의 각 열에 대하여...
        // pivot: AA[0][0], AA[1][1], AA[2][2], ... 
        pivot_row = -1; // 초깃값
        for (int i = j; i < A.row; i++) { // AA[j][j], AA[j+1][j], ...
            if (AA.M[i][j] != 0) {
                pivot_row = i;
                break;  // for-loop 밖으로
            }
        }
        if (pivot_row == -1) { // 해당 열의 모든 원소가 0이면,
            printf("(GF2_Matrix_Inverse) Not Invertible\n");
            return InvA;
        }
        if (pivot_row != j) {
            GF2_Mat_Exchange_Row(AA, j, pivot_row);
        }
        //GF2에서는 필요하지 않음: Mat_Scalar_Mul_Row(AA, 1. / AA.M[j][j], j);

        for (int i = 0; i < A.row; i++) {
            if (i != j) { // A[j][j] 이 포함되지 않은 행에 대해서만...
                GF2_Mat_Row_Add(AA, AA.M[i][j], j, i);
            }
        }
        GF2_Matrix_print(AA);
    }

    InvA.row = A.row;
    InvA.col = A.col;
    for (int i = 0; i < InvA.row; i++)
        for (int j = 0; j < InvA.col; j++)
            InvA.M[i][j] = AA.M[i][j + A.col];

    return InvA;
}

// A --> k*A  (k=0,1)
GF2_Matrix GF2_Matrix_scalar(GF2_Matrix A, byte k) {
    GF2_Matrix kA;
    kA = GF2_Matrix_Init();

    kA.row = A.row;
    kA.col = A.col;

    for (int i = 0; i < kA.row; i++)
        for (int j = 0; j < kA.col; j++)
            kA.M[i][j] = k * A.M[i][j];
    return kA;
}
//===============================================================================


//=============================
// HW1-Problem 4
void GF2_inverse_matrix() {
    //AES Sobx에 사용한 Matrix A
    const byte A[8][8] = {
    {1, 0, 0, 0, 1, 1, 1, 1},
    {1, 1, 0, 0, 0, 1, 1, 1},
    {1, 1, 1, 0, 0, 0, 1, 1},
    {1, 1, 1, 1, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 0, 0, 0},
    {0, 1, 1, 1, 1, 1, 0, 0},
    {0, 0, 1, 1, 1, 1, 1, 0},
    {0, 0, 0, 1, 1, 1, 1, 1}
    };

    const byte IA[8][8] = {  // A^(-1)을 미리 준비한다. 
    {0, 0, 1, 0, 0, 1, 0, 1},
    {1, 0, 0, 1, 0, 0, 1, 0},
    {0, 1, 0, 0, 1, 0, 0, 1},
    {1, 0, 1, 0, 0, 1, 0, 0},
    {0, 1, 0, 1, 0, 0, 1, 0},
    {0, 0, 1, 0, 1, 0, 0, 1},
    {1, 0, 0, 1, 0, 1, 0, 0},
    {0, 1, 0, 0, 1, 0, 1, 0}
    };

    GF2_Matrix GF2_A;
    GF2_A.row = 8; GF2_A.col = 8;
    for (int i = 0; i < GF2_A.row; i++)
        for (int j = 0; j < GF2_A.col; j++)
            GF2_A.M[i][j] = A[i][j];

    GF2_Matrix GF2_IA;
    GF2_IA.row = 8; GF2_IA.col = 8;
    for (int i = 0; i < GF2_IA.row; i++)
        for (int j = 0; j < GF2_IA.col; j++)
            GF2_IA.M[i][j] = IA[i][j];


    GF2_Matrix GF2_AIA;
    GF2_AIA = GF2_Matrix_Mul(GF2_A, GF2_IA);

    printf("A =\n");
    GF2_Matrix_print(GF2_A);
    printf("invA =\n");
    GF2_Matrix_print(GF2_IA);
    printf("Check: A*invA =\n");
    GF2_Matrix_print(GF2_AIA);

    printf("=== Inverse of A\n");
    GF2_Matrix InvA;
    InvA = GF2_Matrix_Inverse(GF2_A);
    printf("invA =\n");
    GF2_Matrix_print(InvA);
}

//===============================================================================
// AES Affine 변환의 행렬 A의 행을 한칸씩 아래로 이동시킨 행렬 A'의 역행렬을 구하고, 
// A'의 역행렬과 A'를 곱하여 항등행렬이 나오는지 확인한다.    
// 새로운 Affine 변환 행렬 A'를 이용하여 Sbox를 구하고, 고정점이 있는지 확인한다.

// 행렬에서 행을 위로 rotation
void Matrix_Rotate_Row(byte A[8][8]) {
    byte temp_row[8];
        
    for(int j = 0; j < 8; j++) {
        temp_row[j] = A[0][j]; // 첫 번째 행을 임시로 저장
	}
    for (int i = 0; i < 7; i++) {		
        for (int j = 0; j < 8; j++) {
            A[i][j] = A[i+1][j];
        }
	}
    for (int j = 0; j < 8; j++) {
        A[7][j] = temp_row[j]; // 마지막 행에 첫 번째 행을 복사
	}
}

//======
void make_affine_rotated_matrix(byte AR[8][8], int rot) {
    const byte A[8][8] = {
        {1, 0, 0, 0, 1, 1, 1, 1},
        {1, 1, 0, 0, 0, 1, 1, 1},
        {1, 1, 1, 0, 0, 0, 1, 1},
        {1, 1, 1, 1, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 0, 0, 0},
        {0, 1, 1, 1, 1, 1, 0, 0},
        {0, 0, 1, 1, 1, 1, 1, 0},
        {0, 0, 0, 1, 1, 1, 1, 1}
     };

    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            AR[i][j] = A[i][j];
        }
	}

    for(int i=0; i<rot; i++) {
        Matrix_Rotate_Row(AR); // 행렬 A를 한 칸 위로 이동        
    }
}

//==============================================
//Affine변환: w --> Aw+b
// w는 GF(2)^8 원소 (8차원 벡터)  w = [w7w6w5w4 w3w2w1w0]
// rot: 0이면 원래 A, 1이면 A1 (한칸 아래로 이동), 2이면 A2 (두칸 아래로 이동), 3이면 A3 (세칸 아래로 이동)
byte AES_Affine_rot(byte w, byte AR[8][8]) {
    const byte b_vec[8] = { 1, 1, 0, 0, 0, 1, 1, 0 };
    byte w_vec[8];  // w_vec = {w0,w1,w2, ... ,w7}
    byte y_vec[8];  // y_vec = {y0,y1,y2, ... ,y7}

    //w = [w7w6w5w4 w3w2w1w0] --> w_vec ={w0,w1,w2, ... ,w7}
    for (int i = 0; i < 8; i++) w_vec[i] = (w >> i) & 0x01;

    // y_vec = A*w_vec + b_vec = b_vec + A*w_vec
    for (int i = 0; i < 8; i++) {
        y_vec[i] = b_vec[i];
        for (int j = 0; j < 8; j++) {
            // y_i = A[i][0]*w[0] ^ A[i][1]*w[1] ^ ... ^ A[i][7]*w[7]
            // GF(2)에서 덧셈은 XOR(^)
            y_vec[i] ^= AR[i][j] * w_vec[j];
        }
    }
    // y_vec --> y (바이트)
    byte y;
    y = 0;
    //y_vec = { y0,y1,y2, ... ,y7 } --> y = [y7y6y5y4 y3y2y1y0]
    for (int i = 0; i < 8; i++) {  // 수정된 부분!!!!! (7-->8)
        y ^= y_vec[i] << i;
        // y0<<0 --> 0000 000y0
        // y1<<1 --> 0000 00y10
        //...
        // y7<<7 --> y7000 0000
    }
    return y;
}


// Sbox를 테이블로 받아오기
// x --> w=x^(-1) --> y = A*x+b
// 함수의 파라미터로 배열을 전달하면 변경된 결과가 함수 밖에서도 반영된다.
void Get_AES_Sbox_rot(byte S[256], int rot) {
    byte w; // 중간값 w 저장
	byte AR[8][8];
	make_affine_rotated_matrix(AR, rot); 
	printf("A_%1d =\n", rot);
    for (int i = 0; i < 8; i++) {  
        printf("[");
        for (int j = 0; j < 8; j++) {
            printf(" %1d", AR[i][j]); // 0 또는 1만 출력
        }
        printf("]\n");
    }
    printf("\n");

    for (int i = 0; i < 256; i++) {
        w = GF256_inv(i);
        S[i] = AES_Affine_rot(w, AR);
    }
}


////===========================
//// 2026 HW - order
//void GF256_order_HW() {
//    int order;
//    printf("Order of elements in GF(2^8):\n");
//    for (int i = 1; i < 256; i++) {
//        order = GF256_ord(i); 
//        if (order < 10) {
//            printf("Element: %02x, Order: %d\n", i, order);
//        }
//    }
//}

//============================
// HW1-Problem 3
void Prob3_GF256() {
    byte a, b, aa, bb, inv_a, inv_b;
	int order_a, order_b;

    a = 0xbc;
    b = 0xbd;
    printf("a = %02x\n", a);
    GF256_print_bin(a);
    GF256_print_poly(a);

    printf("\nb = %02x\n", b);
    GF256_print_bin(b);
    GF256_print_poly(b);

	order_a = GF256_ord(a);
	order_b = GF256_ord(b);

    printf("\nOrder of a = %02x is %d\n", a, order_a);
	printf("Order of b = %02x is %d\n", b, order_b);    

	aa = GF256_mul(a, a);
	bb = GF256_mul(b, b);
	inv_a = GF256_inv(a);
	inv_b = GF256_inv(b);

	printf("\na^2 = %02x\n", aa);
	printf("a^(-1) = %02x\n", inv_a);
    printf("\nb^2 = %02x\n", bb);
	printf("b^(-1) = %02x\n", inv_b);

	printf("\n%02x*%02x = %02x\n", a, b, GF256_mul(a, b));
}

//고정점 확인
int num_fixed_points(byte S[256]) {
    int fixed_points_count = 0;
        
    printf("Fixed points:");
    for (int i = 0; i < 256; i++) {
        if (S[i] == i) {
            printf(" %02x", i);
            fixed_points_count++;
        }
    }
    if (fixed_points_count == 0) {
        printf(" No fixed points found.\n");
    }
    else {
        printf("\n");
    }
    return fixed_points_count;
}

//반고정점 확인
int num_antifixed_points(byte S[256]) {
    int anitfixed_points_count = 0;

    printf("Anti-fixed points:");
    for (int i = 0; i < 256; i++) {
        if ((S[i] ^ i) == 0xff) {
            printf(" %02x", i);
            anitfixed_points_count++;
        }
    }
    if (anitfixed_points_count == 0) {
        printf(" No anti-fixed points found.\n");
    }
    else {
        printf("\n");
    }
    return anitfixed_points_count;
}

//============================
// HW1-Problem 5(a)
void Sbox_fixed_point() {
    byte S[256];
    Get_AES_Sbox(S);
    
    printf("==[HW1 Problem 5(a)]==\n");
	printf("Check fixed points and anti-fixed points of AES Sbox\n");
    num_fixed_points(S);
	num_antifixed_points(S);
}

//============================
// HW1-Problem 5(b)
void Sbox_wo_affine_fixed_point() {
    byte S[256];
    for(int i=0; i<256; i++) {
        S[i] = GF256_inv(i);
	}

    printf("==[HW1 Problem 5(b)]==\n");
    printf("Check fixed points and anti-fixed points of AES Sbox without Affine transform\n");
    num_fixed_points(S);
    num_antifixed_points(S);
}

//============================
// HW1-Problem 5(c)
void Sbox_rot_fixed_point() {
    byte S[256];

    printf("==[HW1 Problem 5(c)]==\n");
    for (int rot = 1; rot <= 7; rot++) {
        Get_AES_Sbox_rot(S, rot);
        //print_sbox(S);    
        printf("Check fixed points and anti-fixed points of Sbox_%1d\n", rot);
        num_fixed_points(S);
        num_antifixed_points(S);
		printf("\n");
    }
}


int main(){

    //Prob 3
    Prob3_GF256();

    //Prob 4
    GF2_inverse_matrix();

    //Prob 5(a)
	Sbox_fixed_point();
    
    //Prob 5(b)
    Sbox_wo_affine_fixed_point();

    //Prob 5(c)
    Sbox_rot_fixed_point();

}


