#include <stdio.h>
#include "GF_256.h"


typedef unsigned char byte;
//================================================================
// GF(2^4) = GF(2)[x]/(x^4+x+1)
// 
// GF(2^4)의 원소를 비트열로 출력하기
// b = b3b2b1b0 ==> [b3,b2,b1,b0]
void GF16_print_bin(byte b) {
    byte b_coeff[4]; // 계수를 저장하는 배열
    for (int i = 0; i < 4; i++) {
        // 계수: b3, ... , b0 순서로 추출하기
        b_coeff[i] = (b >> (3 - i)) & 0x01;
    }
    for (int i = 0; i < 8; i++) {
        printf("%d", b_coeff[i]);
    }
    printf("\n");
}

// GF(2^4)의 원소를 다항식으로 출력하기
// b = b3b2b1b0 ==> b3*x^3 + ... + b0
void GF16_print_poly(byte b) {
    byte b_coeff[4]; // 계수를 저장하는 배열
    for (int i = 0; i < 4; i++) {
        // 계수: b3,  ... , b0 순서로 추출하기
        b_coeff[i] = (b >> (3 - i)) & 0x01;
    }

    int coef, expo;
    for (int i = 0; i < 4; i++) {
        coef = b_coeff[i]; // b3, ... , b0
        expo = 3 - i; // 3, 2, 1, 0
        if (coef == 1) {
            printf("+x^%1d ", expo);
        }
    }
    printf("\n");
}

// 간단한 버전의 xtime() : x^4 = x+1 (mod x^4+x+1) = 0011 = 0x03
byte GF16_xtime(byte a) {
    return ((a >> 3) == 0 ? (a << 1) & 0x0f : ((a << 1) ^ 0x03)) & 0x0f;
}

// GF(2^4)에서의 덧셈
byte GF16_add(byte a, byte b) {
    return a ^ b; // ^ : xor 연산자
}

// GF(2^4)에서의 곱셈
//a = a3...a0 => f(x) = a3x^3 + ... + a1x + a0
//b = b3...b0 => g(x) = b3x^3 + ... + b1x + b0
//c = c3...c0 => h(x) = f(x)*g(x) 
// h(x) <- g(x)*a3
// h(x) <- x*g(x)*a3 + g(x)*a2 = x*h(x) + g(x)*a2
// h(x) <- x*(x*g(x)*a3 + g(x)*a2) + g(x)*a1 = x*h(x) + g(x)*a1
// ...
byte GF16_mul(byte a, byte b) {
    byte h; // 결과를 저장할 변수
    int coef; // 계수 저장

    h = 0x00; // h(x) <- 0
    for (int i = 3; i >= 0; i--) { // a3...a0 순서로
        coef = (a >> i) & 0x01;  // 계수를 순서대로 저장하기 a3, ... , a0
        h = GF16_xtime(h);
        if (coef == 1) { // 계수(ai)가 1이면
            h = GF16_add(h, b); // h^b로 써도 됨
        }
    }
    return h;
}

//a(0이 아닌 원소)의 위수(order)
int GF16_ord(byte a) {
    byte temp;
    int order;

    if (a == 0) { // ord(0)은 정의되지 않음
        return -1; //의미 없는 결과임
    }
    temp = a;
    order = 1;
    while (temp != 1) { // temp == 1 이면 반복을 종료
        //temp *= a; // a의 거듭제곱 temp = a, a^2, a^3, ...
        //printf("(ord) %02x\n", temp);
        temp = GF16_mul(temp, a);
        order++;
    }
    return order;
}

// GF(2^4)의 곱셈의 역원
// a의 역원 a^14 = a^{1110} = a^[ {1000}+{0100}+...+{0010}]
//               = a^{8+4+2} = a^8*a^4*a^2
byte GF16_inv(byte a) {
    byte inverse; // 역원을 저장
    byte temp; // a^2, a^4, ... 저장

    if (a == 0) { // 0의 역원은 없다.
        return 0; // 0^{-1} = 0 으로 출력 
    }

    inverse = 1; // 초기값
    temp = a;   // 초기값
    for (int i = 0; i < 3; i++) {
        temp = GF16_mul(temp, temp);  // a^2, a^4, ... , a^8
        inverse = GF16_mul(inverse, temp); // a^2*a^4*....
    }
    return inverse;
}


void GF16_gen_log_table(int gen) {

    printf("== GF(2^4) Generator = %02x\n",gen);    
    
    byte gen_pow;
    gen_pow = 0x01;
    for (int i = 0; i <= 15; i++) {
        printf("a^%d | 0x%02x \n", i, gen_pow);
        gen_pow = GF16_mul(gen_pow, gen);
    }

    printf("\n== GF(2^4) Log Table\n Generator = %02x\n", gen);
    byte base_x = gen; 
    byte pow_x = 1;
    byte LogTable[16];
    LogTable[0] = 0; //0은 예외
    for (int i = 1; i <= 15; i++) { // 실제로 i<=14까지만 해도 된다 (a^15=1)
        pow_x = GF16_mul(base_x, pow_x);
        // pow_x = 3^1, 3^2, 3^3, ... 3^k ...
        //       =   1,   2,   3, ...   k ...
        LogTable[pow_x] = i; 
    }
    LogTable[1] = 0; // a^0 = a^(15 mod (2^4-1)) = 1

    //LogTable의 역함수 테이블 만들기 (역함수 테이블 만들기 : Sbox, ISbox 만들기와 유사)
    byte ExpTable[16];
    for (int i = 0; i < 16; i++) {
        ExpTable[LogTable[i]] = i;
    }
    ExpTable[15] = 0x01;  // a^15 = 1 이므로 (LogTable[]에는 15가 없다)
    printf("byte LogTable[16] = {");
    for (int i = 0; i < 16; i++) {
        if ((i % 8) == 0) printf("\n");
        if (i < 15) {
            printf(" %01x,", LogTable[i]);
        }
        else {
            printf(" %01x};\n\n", LogTable[i]);
        }
    }

    printf("byte ExpTable[16] = {");
    for (int i = 0; i < 16; i++) {
        if ((i % 8) == 0) printf("\n");
        if (i < 15) {
            printf(" %01x,", ExpTable[i]);
        }
        else {
            printf(" %01x};\n\n", ExpTable[i]);
        }
    }
}

//-- GF(16)의 두가지 Log-table을 사용한다.
//GF(2^4)- Generator=0x03
/*
byte LogTable[16] = {
 0x00, 0x00, 0x04, 0x01, 0x08, 0x02, 0x05, 0x0a,
 0x0c, 0x0b, 0x06, 0x0d, 0x09, 0x07, 0x0e, 0x03 };

byte ExpTable[16] = {
 0x01, 0x03, 0x05, 0x0f, 0x02, 0x06, 0x0a, 0x0d,
 0x04, 0x0c, 0x07, 0x09, 0x08, 0x0b, 0x0e, 0x01 };
 */

//GF(2^4)- Generator=0x0b
byte LogTable[16] = {
 0x00, 0x00, 0x0d, 0x07, 0x0b, 0x0e, 0x05, 0x0a,
 0x09, 0x02, 0x0c, 0x01, 0x03, 0x04, 0x08, 0x06 };

byte ExpTable[16] = {
 0x01, 0x0b, 0x09, 0x0c, 0x0d, 0x06, 0x0f, 0x03,
 0x0e, 0x08, 0x07, 0x04, 0x0a, 0x02, 0x05, 0x01 };


byte GF16_inv_using_cyclic(byte x) {
    byte ax, result;

    ax = LogTable[x];
    result = ExpTable[15 - ax];

    return result;
}


byte GF16_mul_cycle(byte x, byte y) {
    byte kx, ky, kz, z;

    kx = LogTable[x]; // x = a^kx
    ky = LogTable[y]; // y = a^ky

    kz = (kx + ky) % 15; // z = x*y = a^(kx+ky mod 15)

    z = ExpTable[kz]; // a^kz --> z

    return z;
}

void GF16_mul_test() {

    printf("=== GF(2^4) Mul. using Log-table\n");    
    
    byte x = 0x0a; // x = a^kx (x --> kx) (a^kx --> kx) log table
    byte y = 0x06; // y = a^ky (y --> ky) (a^ky --> ky) log table
    byte akx, aky, akz; // x*y = a^(kx+ky mod 15)
    int kx, ky, kz;
    int generator;

    generator = 0x0b; // 0x03, 0x0b

    kx = LogTable[x];
    ky = LogTable[y];
    kz = (kx + ky) % 15; // kz = 0, 1, 2, .... , 14 가능한 값

    akz = 1;
        
    //확인용! 
    printf(" x*y = 0x%02x * 0x%02x = 0x%02x\n", x, y, GF16_mul(x, y));

    printf(" x = 0x%02x = 0x%02x ^ %d \n", x, generator, kx);
    printf(" y = 0x%02x = 0x%02x ^ %d \n", y, generator, ky);
    printf(" z = x*y = 0x%02x = 0x%02x ^ %d \n", akz, generator, kz);
    
    byte z;
    z = GF16_mul_cycle(x, y); // Exp, Log 테이블을 이용한 곱셈
    printf(" x = 0x%02x = 0x%02x ^ %d \n", x, generator, kx);
    printf(" y = 0x%02x = 0x%02x ^ %d \n", y, generator, ky);

    kz = LogTable[z];
    akz = ExpTable[kz];
    printf(" z = %02x = x*y = 0x%02x = 0x%02x ^ %d \n", z, akz, generator, kz);

}
//======================================================

void GF16_check_order() {
    int num_max_order; //최대 위수를 갖는 원소의 개수
    int order; // 원소의 위수
    byte inv;
    num_max_order = 0;
    printf("== The orders of elements in GF(2^4)\n");
    for (int i = 0; i < 16; i++) {
        order = GF16_ord(i);
        inv = GF16_inv(i);
        printf("ord(%3d) = %d, %02x^(-1) = %02x, %02x*%02x=%02x\n",
            i, order, i, inv, i, inv, GF16_mul(i, inv));
        if (order == 15) num_max_order++;
    }
    printf("The number of Max order elements = %d\n", num_max_order);
    //MAX order elements: 2, 3, 4, 5, 9, 11, 13, 14
}



int main()
{
    GF16_check_order();
    GF16_gen_log_table(0x03);
    GF16_gen_log_table(0x0b);

    GF16_mul_test();
}
