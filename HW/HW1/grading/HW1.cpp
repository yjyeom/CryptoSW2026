#include <stdio.h>
#include "RMatrix.h"
#include "GF_256.h"

// 행렬 출력하기
void HMatrix_print(Matrix A) {
    for (int i = 0; i < A.row; i++) {
        printf("[");
        for (int j = 0; j < A.col; j++) {
            printf(" %d", (int)A.M[i][j]);
        }
        printf("]\n");
    }
    printf("\n");
}

// 한 행를 다른 행과 더하기(XOR)
void AES_Mat_Row_Add(Matrix& A, int row_src, int row_target) {
    for (int j = 0; j < A.col; j++) {
        A.M[row_target][j] = (int)A.M[row_target][j] ^ (int)A.M[row_src][j];
    }
}

// 역행렬 구하기
Matrix AES_Matrix_Inverse(Matrix A) {
    Matrix InvA;
    InvA = Matrix_Init();

    if (A.row != A.col) { // 정사각행렬이 아닌 경우
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

    // [A|I] -----> [I|A^(-1)]  R-REF (Reduced Row Echelon Form)

    int pivot_row; // 각 열을 단위행렬 모양으로 만들 때 '1'의 위치를 가지는 열
    for (int j = 0; j < A.col; j++) { // 한 번에 한 열씩 단위행렬 I 모양으로 만들기
        pivot_row = -1; // 초깃값
        for (int i = j; i < A.row; i++) { // AA[j][j], AA[j + 1][j], ... , AA[*][j]
            if (AA.M[i][j] == 1) {
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

        for (int i = 0; i < AA.row; i++) {
            if ((i != j) && (AA.M[i][j] != 0)) { // 자기자신을 제외하고, 0이 아닌 행에 대해
                AES_Mat_Row_Add(AA, j, i);
            }
        }

        HMatrix_print(AA);
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

void AES_test_print() {
    Matrix A, IA;

    A.row = 8;
    A.col = 8;
    IA.row = 8;
    IA.col = 8;

    double Mat[8][8] = {
        {1, 0, 0, 0, 1, 1, 1, 1},
        {1, 1, 0, 0, 0, 1, 1, 1},
        {1, 1, 1, 0, 0, 0, 1, 1},
        {1, 1, 1, 1, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 0, 0, 0},
        {0, 1, 1, 1, 1, 1, 0, 0},
        {0, 0, 1, 1, 1, 1, 1, 0},
        {0, 0, 0, 1, 1, 1, 1, 1}
    };

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            A.M[i][j] = Mat[i][j];
        }
    }
    printf("A = \n");
    HMatrix_print(A);

    IA = AES_Matrix_Inverse(A);
    printf("invA = \n");
    HMatrix_print(IA);
}

// 고정점과 반고정점 확인
void Check_point(byte S[256])
{
    int fixed_point = 0;
    int anti_fixed_point = 0;

    for (int x = 0; x < 256; x++) {
        if (S[x] == x) { // f(x) = x
            printf("고정점: 0x%02X\n", x);
            fixed_point++;
        }

        if (S[x] == (x ^ 0xFF)) { // f(x) = x ^ 0xFF, x ^ 0xFF = bit 반전
            printf("반고정점: 0x%02X\n", x);
            anti_fixed_point++;
        }
    }

    printf("고정점 %d개, 반고정점 %d개\n\n", fixed_point, anti_fixed_point);
}

byte AES_Affine_Shift(byte w, int shift)
{
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
    const byte b_vec[8] = { 1, 1, 0, 0, 0, 1, 1, 0 };
    byte w_vec[8]; // w_vec = {w0, w1, w2, ... , w7}
    byte y_vec[8]; // y_vec = {y0, y1, y2, ... , y7}

    // w --> w_vec
    for (int i = 0; i < 8; i++) w_vec[i] = (w >> i) & 0x01;

    // y_vec = A*w_vec + b_vec = b_vec + A*w_vec
    for (int i = 0; i < 8; i++)
    {
        y_vec[i] = b_vec[i];;
        for (int j = 0; j < 8; j++)
        {
            // 행을 shift만큼 이동시켜서 곱셈
            y_vec[i] ^= A[(i + shift) % 8][j] * w_vec[j];
        }
    }

    // y_vec --> y (바이트)
    byte y;
    y = 0;
    // y_vec = {y0, y1, y2, ... , y7} --> y = [y7y6y5y4 y3y2y1y0]
    for (int i = 0; i < 8; i++)
    {
        y ^= (y_vec[i] << i);
        // y0 << 0 --> 0000 000y0
        // y1 << 1 --> 0000 00y10
        // ...
        // y7 << 7 --> y7000 0000
    }
    return y;
}

void Make_Table(byte ExpTable[256], int LogTable[256]) {
    for (int i = 0; i < 256; i++) {
        ExpTable[i] = 0;
        LogTable[i] = -1;
    }

    ExpTable[0] = 1; // (a)의 alpha^0 출력에도 사용
    byte value = 1;

    for (int k = 1; k <= 255; k++) {
        value = GF256_mul(value, 0x03);

        ExpTable[k] = value;
        LogTable[value] = k;
    }
}

// GF(2^8)에서 곱을 계산하는 함수
byte GF256_mul_table(byte x, byte y, const byte ExpTable[256], const int LogTable[256]) {
    if (x == 0 || y == 0) return 0;

    int k = (LogTable[x] + LogTable[y]) % 255;
    if (k == 0) k = 255;

    return ExpTable[k];
}

void HW5_a() {
    byte S[256];

    Get_AES_Sbox(S);

    Print_Sbox();
    Check_point(S);
}

void HW5_b() {
    byte S[256];

    Get_AES_Sbox(S);

    printf("Affine 변환 제외\n");
    for (int x = 0; x < 256; x++) {
        S[x] = GF256_inv((byte)x);
    }
    Check_point(S);
}

void HW5_c() {
    byte S[256];

    Get_AES_Sbox(S);

    for (int shift = 1; shift <= 7; shift++) {
        printf("A%d \n", shift);

        for (int x = 0; x < 256; x++) {
            byte w = GF256_inv((byte)x);
            S[x] = AES_Affine_Shift(w, shift);
        }

        Check_point(S);
    }
}

void HW6_a() {
    byte a = 0x01;

    printf("GF(2^8)에서 0x03의 거듭제곱:\n");
    for (int k = 0; k < 255; k++) {
        printf("a^%d = 0x%02X\n", k, a);
        a = GF256_mul(a, 0x03);
    }
}

void HW6_c() {
    byte ExpTable[256];
    int LogTable[256];

    Make_Table(ExpTable, LogTable);

    printf("Generator : 0x03, ExpTable(k):\n");
    for (int k = 0; k <= 255; k++) {
        printf("0x03^%d = 0x%02X\n", k, ExpTable[k]);
    }

    printf("\nLogTable(0x03^k):\n");
    for (int x = 1; x < 256; x++) {
        printf("Log_0x03(0x%02X) = %d\n", x, LogTable[x]);
    }
}

void HW6_d() {
    byte ExpTable[256];
    int LogTable[256];

    Make_Table(ExpTable, LogTable);

    printf("0xA1 * 0x63\n");
    printf("Log(A1) = %d\n", LogTable[0xA1]);
    printf("Log(63) = %d\n", LogTable[0x63]);
    printf("result = 0x%02X\n", GF256_mul_table(0xA1, 0x63, ExpTable, LogTable));
}

// 4번
void HW4() {
    printf("4번 문제\n");
    AES_test_print();
    printf("\n");
}

// 5번
void HW5() {
    printf("5번 문제\n");
    printf("(a) : \n");
    HW5_a();
    printf("\n");
    printf("(b) : \n");
    HW5_b();
    printf("\n");
    printf("(c) : \n");
    HW5_c();
    printf("\n\n");
}

void HW6() {
    printf("6번 문제\n");
    printf("(a) : \n");
    HW6_a();
    printf("\n");
    printf("(c) : \n");
    HW6_c();
    printf("\n");
    printf("(d) : \n");
    HW6_d();
    printf("\n");
}

int main() {
    HW4();
    HW5();
    HW6();

    return 0;
}
