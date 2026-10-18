// midterm_1.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <stdio.h>

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
    return ((a >> 3) == 0 ? (a << 1)&0x0f : ((a << 1) ^ 0x03))&0x0f;
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

void GF16_order_inverse_test() {
    int num_max_order; //최대 위수를 갖는 원소의 개수
    int order; // 원소의 위수
    byte inv;
    num_max_order = 0;
    printf("The orders of elements in GF(2^4)\n");
    for (int i = 0; i < 16; i++) {
		printf("i = %02x, ", i);
        GF16_print_poly(i);
        order = GF16_ord(i);
        inv = GF16_inv(i);
        printf("ord(%3d) = %d, %02x^(-1) = %02x, %02x*%02x=%02x\n",
            i, order, i, inv, i, inv, GF16_mul(i, inv));
        if (order == 15) num_max_order++;
    }
    printf("The number of Max order elements = %d\n", num_max_order);
}

void GF16_mul_test() {
    byte a, b;
    printf("GF(2^4) multiplication table\n");
    a = 0xe;
    b = 1;
    for (int i = 1; i < 16; i++) {        
        b = GF16_mul(a, b);
        printf("%02x ^%d = %02x\n", a, i, b);
        GF16_print_poly(b);
    }
}

//================================================================

int main()
{
    GF16_order_inverse_test();
	//GF16_ord(0x02);

    GF16_mul_test();
}

