// GF256_Matrix.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <stdio.h>

#define MATRIX_MAX_SIZE 20
typedef unsigned char byte;
//==============================
//  GF256.cpp에 정의된 함수
//==============================
byte GF256_add(byte a, byte b) {
    return a ^ b;
}

byte GF256_xtime(byte a) {
    return (((a >> 7) & 0x01) == 0) ? a << 1 : (a << 1) ^ 0x1b;
}

byte GF256_mul(byte f, byte g) {
    byte h; // 곱셈결과 저장
    int coef; // 계수

    h = 0x00; // 초깃값 h(x) = 0
    for (int i = 7; i >= 0; i--) { // a7, a6, a5, ... ,a1, a0 순서로 반복
        coef = (f >> i) & 0x01; // a7, a6, a5, ... ,a1, a0
        //h = GF256_xtime(h); 
        h = GF256_xtime(h);
        if (coef == 1) {  // 계수 ai가 1인 경우만 g(x)를 h(x)에 더한다
            h = GF256_add(h, g);
        }
    }
    return h;
}

byte GF256_inv(byte x) {  // x^254 = x^(1111 1110)
    byte x_inv, term;
    term = x;

    //x=0이 입력되면, 역원은 존재하지 않지만, 0을 리턴한다.
    if (x == 0) return 0x00;

    x_inv = 1; //곱하기에 대한 초기값
    for (int i = 0; i < 7; i++) {
        term = GF256_mul(term, term); // x^2, x^4, x^8, ... , x^128  (128 = 2^7)
        x_inv = GF256_mul(x_inv, term); // (1), x^2, x^2*x^4, (x^2*x^4)*x^8, ...
    }
    return x_inv;
}

//==============================

typedef struct {  // GF(2^8) 행렬을 나타내는 구조체
    byte M[MATRIX_MAX_SIZE][MATRIX_MAX_SIZE];  // 00~FF 저장한다.
    int row;
    int col;
} GF256_Matrix;

void GF256_Matrix_print(GF256_Matrix A) {
    for (int i = 0; i < A.row; i++) {  //각 행에 대하여
        printf("[");
        for (int j = 0; j < A.col; j++) {
            printf(" %02x", A.M[i][j]); // 00~ff 출력
        }
        printf("]\n");
    }
    printf("\n");
}

// 행렬 변수의 초기화
GF256_Matrix GF256_Matrix_Init() {
    GF256_Matrix A;
    A.row = 1; A.col = 1;
    A.M[0][0] = (byte)0;

    return A;
}

// C = A+B (행렬의 덧셈)
GF256_Matrix GF256_Matrix_Add(GF256_Matrix A, GF256_Matrix B) {
    GF256_Matrix result;

    result = GF256_Matrix_Init();
    // 행렬의 규격 확인
    if ((A.col != B.col) || (A.row != B.row)) {
        printf("(GF256_Matrix_Add) Matrix size error\n");
        return result;
    }
    result.row = A.row;
    result.col = A.col;
    for (int i = 0; i < result.row; i++)
        for (int j = 0; j < result.col; j++)
            result.M[i][j] = GF256_add(A.M[i][j], B.M[i][j]);
    return result;
}

// C = A*B (행렬 곱)  [m,k]*[k,n] = [m,n] GF(256)의 곱
GF256_Matrix GF256_Matrix_Mul(GF256_Matrix A, GF256_Matrix B) {
    GF256_Matrix AB;
    AB = GF256_Matrix_Init();

    //행렬의 규격 확인 
    if (A.col != B.row) {
        printf("(GF256_Matrix_Mul) Matrix size error\n");
        return AB;
    }

    AB.row = A.row;
    AB.col = B.col;
    for (int i = 0; i < AB.row; i++) {
        for (int j = 0; j < AB.col; j++) {
            AB.M[i][j] = 0.0;
            for (int k = 0; k < A.col; k++) {
                AB.M[i][j] ^= GF256_mul(A.M[i][k], B.M[k][j]);
            }
        }
    }
    return AB;
}

//두 행을 바꾸기
void GF256_Mat_Exchange_Row(GF256_Matrix& A, int row1, int row2) {
    byte temp;
    for (int j = 0; j < A.col; j++) {
        //A[row1][j] <--> A[row2][j]
        temp = A.M[row1][j];
        A.M[row1][j] = A.M[row2][j];
        A.M[row2][j] = temp;
    }
}

//한 행에 상수배 하기
void GF256_Mat_Scalar_Mul_Row(GF256_Matrix& A, byte k, int row) {
    // k(상수)는 00~FF
    byte temp;
    for (int j = 0; j < A.col; j++) {
        temp = GF256_mul(k, A.M[row][j]);
        A.M[row][j] = temp;
    }
}

//한 행의 상수배를 다른 행에 더하기
void GF256_Mat_Row_Add(GF256_Matrix& A, byte k, int row_src, int row_target) {
    for (int j = 0; j < A.col; j++)
        A.M[row_target][j] ^= GF256_mul(k, A.M[row_src][j]);  // xor 
}

//역행렬 구하기
GF256_Matrix GF256_Matrix_Inverse(GF256_Matrix A) {
    GF256_Matrix InvA;
    InvA = GF256_Matrix_Init();
    if (A.row != A.col) { // 정사각행렬인지 확인
        printf("(GF256_Matrix_Inverse) Non-Sqaure Matrix error\n");
        return InvA;
    }

    // AA = [A|I] -------> [I|A^(-1)]
    GF256_Matrix AA;
    AA.row = A.row;
    AA.col = 2 * A.col;
    for (int i = 0; i < A.row; i++) {
        for (int j = 0; j < A.col; j++) {
            AA.M[i][j] = A.M[i][j];
            AA.M[i][j + A.col] = (i == j) ? 1 : 0;
        }
    }

    GF256_Matrix_print(AA);

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
            printf("(GF256_Matrix_Inverse) Not Invertible\n");
            return InvA;
        }
        if (pivot_row != j) {
            GF256_Mat_Exchange_Row(AA, j, pivot_row);
        }
        GF256_Mat_Scalar_Mul_Row(AA, GF256_inv(AA.M[j][j]), j);

        for (int i = 0; i < A.row; i++) {
            if (i != j) { // A[j][j] 이 포함되지 않은 행에 대해서만...
                GF256_Mat_Row_Add(AA, AA.M[i][j], j, i);
            }
        }
        GF256_Matrix_print(AA);
    }

    InvA.row = A.row;
    InvA.col = A.col;
    for (int i = 0; i < InvA.row; i++)
        for (int j = 0; j < InvA.col; j++)
            InvA.M[i][j] = AA.M[i][j + A.col];

    return InvA;
}


GF256_Matrix GF256_Matrix_scalar(GF256_Matrix A, byte k) {
    GF256_Matrix kA;
    kA = GF256_Matrix_Init();

    kA.row = A.row;
    kA.col = A.col;

    for (int i = 0; i < kA.row; i++)
        for (int j = 0; j < kA.col; j++)
            kA.M[i][j] = GF256_mul(k, A.M[i][j]);
    return kA;
}

void test_GF256_matrix() {
    const byte MC[4][4] = {
        {0x02, 0x03, 0x01, 0x01},
        {0x01, 0x02, 0x03, 0x01},
        {0x01, 0x01, 0x02, 0x03},
        {0x03, 0x01, 0x01, 0x02}
    };

    const byte IMC[4][4] = {  // MC^(-1)을 미리 준비한다. 
        {0x0e, 0x0b, 0x0d, 0x09},
        {0x09, 0x0e, 0x0b, 0x0d},
        {0x0d, 0x09, 0x0e, 0x0b},
        {0x0b, 0x0d, 0x09, 0x0e}
    };

    GF256_Matrix GF256_MC;
    GF256_MC.row = 4; GF256_MC.col = 4;
    for (int i = 0; i < GF256_MC.row; i++)
        for (int j = 0; j < GF256_MC.col; j++)
            GF256_MC.M[i][j] = MC[i][j];

    GF256_Matrix GF256_IMC;
    GF256_IMC.row = 4; GF256_IMC.col = 4;
    for (int i = 0; i < GF256_IMC.row; i++)
        for (int j = 0; j < GF256_IMC.col; j++)
            GF256_IMC.M[i][j] = IMC[i][j];

    printf("MC =\n");
    GF256_Matrix_print(GF256_MC);
    printf("invMC =\n");
    GF256_Matrix_print(GF256_IMC);

    printf("=== Inverse of MC\n");
    GF256_Matrix InvMC;
    InvMC = GF256_Matrix_Inverse(GF256_MC);
    GF256_Matrix_print(InvMC);
}

void HW4() {
    const byte B[4][4] = {
        {0x02, 0x03, 0x05, 0x01},
        {0x01, 0x02, 0x03, 0x05},
        {0x05, 0x01, 0x02, 0x03},
        {0x03, 0x05, 0x01, 0x02}
    };

    GF256_Matrix GF256_B;
    GF256_B.row = 4; GF256_B.col = 4;
    for (int i = 0; i < GF256_B.row; i++)
        for (int j = 0; j < GF256_B.col; j++)
            GF256_B.M[i][j] = B[i][j];


    printf("B =\n");
    GF256_Matrix_print(GF256_B);

    printf("=== Inverse of B\n");
    GF256_Matrix InvB;
    InvB = GF256_Matrix_Inverse(GF256_B);
    GF256_Matrix_print(InvB);

    GF256_Matrix BIB;
    BIB = GF256_Matrix_Mul(GF256_B, InvB);

    printf("B*B^(-1) =\n");
    GF256_Matrix_print(BIB);

}

void Involution_Matrix() {
    const byte B[4][4] = {
        {0x02, 0x02, 0x03, 0x02},
        {0x02, 0x02, 0x02, 0x03},
        {0x03, 0x02, 0x02, 0x02},
        {0x02, 0x03, 0x02, 0x02}
    };

    GF256_Matrix GF256_B;
    GF256_B.row = 4; GF256_B.col = 4;
    for (int i = 0; i < GF256_B.row; i++)
        for (int j = 0; j < GF256_B.col; j++)
            GF256_B.M[i][j] = B[i][j];


    printf("B =\n");
    GF256_Matrix_print(GF256_B);

    printf("=== Inverse of B\n");
    GF256_Matrix InvB;
    InvB = GF256_Matrix_Inverse(GF256_B);
    GF256_Matrix_print(InvB);

    GF256_Matrix BIB;
    BIB = GF256_Matrix_Mul(GF256_B, InvB);

    printf("B*B^(-1) =\n");
    GF256_Matrix_print(BIB);

}

int main()
{
    //test_GF256_matrix();
    //HW4();

    Involution_Matrix();
    
}
