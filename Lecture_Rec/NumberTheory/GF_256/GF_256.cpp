// GF_256.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <stdio.h>

// GF(2^8)의 데이터, 연산의 구현

//데이터 형: 바이트
// byte b = b7b6b5b4 b3b2b1b0 =  b7*x^7 + b6*x^6 + ... + b1*x + b0
// 예: 0x0a = 0000 1010 = x^3 + x
typedef unsigned char byte; // 0~255 ==> 256가지의 다항식

// 유한체 GF(2^8)의 덧셈
// a = a7a6...a1a0 = a7*x^7 + a6*x^6 + ... + a1*x + a0
// b = b7b6...b1b0 = b7*x^7 + b6*x^6 + ... + b1*x + b0
// c = a + b
// c = (a7+b7)*x^7 + (a6+b6)*x^6 + ... + (a1+b1)*x + (a0+b0)
// 계수들은 0 또는 1 (GF(2)의 원소): 0+0=0, 0+1=1, 1+1=0
// 비트 x, y의 xor :  x xor y = 1 (x,y 중 하나만 1), 0(x,y 모두 0 또는 모두 1)
byte GF256_add(byte a, byte b) {
    return a ^ b; // ^ : xor 연산자
}

// GF(2^8)의 원소를 비트열로 출력하기
// b = b7b6...b1b0 ==> [b7,b6,...,b1,b0]
void GF256_print_bin(byte b) {
    byte b_coeff[8]; // 계수를 저장하는 배열
    for (int i = 0; i < 8; i++) {
        // 계수: b7, b6, ... , b0 순서로 추출하기
        b_coeff[i] = (b >> (7 - i)) & 0x01;
    }
    for (int i = 0; i < 8; i++) {
        printf("%d", b_coeff[i]);
    }
    printf("\n");
}

// GF(2^8)의 원소를 다항식으로 출력하기
// b = b7b6...b1b0 ==> b7*x^7 + ... + b0
void GF256_print_poly(byte b) {
    byte b_coeff[8]; // 계수를 저장하는 배열
    for (int i = 0; i < 8; i++) {
        // 계수: b7, b6, ... , b0 순서로 추출하기
        b_coeff[i] = (b >> (7 - i)) & 0x01; 
    }

    int coef, expo;
    for (int i = 0; i < 8; i++) {
        coef = b_coeff[i]; // b7, b6, ... , b0
        expo = 7 - i; // 7, 6, ... , 1, 0
        if (coef == 1) {
            printf("+x^%1d ", expo);
        }        
    }
    printf("\n");
}

int main()
{
    byte a, b, c;
    a = 0x03;
    b = 0x1a;
    printf("a = ");
    GF256_print_bin(a);
    GF256_print_poly(a);
    printf("b = ");
    GF256_print_bin(b);
    GF256_print_poly(b);
    c = GF256_add(a, b);
    printf("c = ");
    GF256_print_bin(c);
    GF256_print_poly(c);
}

