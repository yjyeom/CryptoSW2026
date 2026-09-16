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

// a = a7a6... a0 (바이트)
// f(x) ---> xf(x)
// f(x) = a7*x^7 + ... + a1*x + a0
// xf(x) = a7*x^8 + a6*x^7+ ... + a0*x
//       = (a6*x^7 + ... a0*x) + a7*(x^4+x^3+x+1)
// x^4+x^3+x+1 = 0001 1011 = 0x1b
byte GF256_xtime(byte a) {
    int msb; //최고차항의 계수(a7)
    byte result; // 결과저장

    msb = (a >> 7) & 0x01; // (0000 000a7) and (0000 0001)
    if (msb == 0) {
        result = a << 1; //a6a5a4a3 a2a1a00
    }
    else {
        result = (a << 1) ^ 0x1b; // GF(2^8)에서의 덧셈 == XOR
    }
    return result;
}

// 간단한 버전의 xtime() 
byte GF256_xtime_simple(byte a) {
    return ((a >> 7) == 0 ? a << 1 : (a << 1) ^ 0x1b);
}

// GF(2^8)에서의 곱셈
//a = a7a6...a0 => f(x) a7x^7 + ... + a1x + a0
//b = b7b6...b0 => g(x) b7x^7 + ... + b1x + b0
//c = c7c6...c0 => h(x) = f(x)*g(x) 
// h(x) <- g(x)*a7
// h(x) <- x*g(x)*a7 + g(x)*a6 = x*h(x) + g(x)*a6
// h(x) <- x*(x*g(x)*a7 + g(x)*a6) + g(x)*a5 = x*h(x) + g(x)*a5
// ...
// h(x) <- x*( ... + g(x)*a1) + g(x)*a0 = x*h(x) + g(x)*a0
byte GF256_mul(byte a, byte b) {
    byte h; // 결과를 저장할 변수
    int coef; // 계수 저장

    h = 0x00; // h(x) <- 0
    for (int i = 7; i >= 0; i--) { // a7a6...a0 순서로
        coef = (a >> i) & 0x01;  // 계수를 순서대로 저장하기 a7, a6, ... , a0
        h = GF256_xtime(h);
        if (coef == 1) { // 계수(ai)가 1이면
            h = GF256_add(h, b); // h^b로 써도 됨
        }
    }
    return h;
}

void GF256_element() {
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

void GF256_mul_test() {
    byte a, b, c, d;
    a = 0x03;
    b = 0xa1;
    printf("a = ");
    GF256_print_bin(a);
    GF256_print_poly(a);
    printf("b = ");
    GF256_print_bin(b);
    GF256_print_poly(b);

    c = GF256_xtime(a);
    d = GF256_xtime(b);
    printf("c = ");
    GF256_print_bin(c);
    GF256_print_poly(c);
    printf("d = ");
    GF256_print_bin(d);
    GF256_print_poly(d);

    c = GF256_mul(a, b);
    d = GF256_mul(0x03, 0x3);
    printf("c = ");
    GF256_print_bin(c);
    GF256_print_poly(c);
    printf("d = ");
    GF256_print_bin(d);
    GF256_print_poly(d);
    // 유한체 곱: 0x02*0x03 = x*(x+1) = x^2+x = 110 = 0x06
    // 유한체 곱: 0x03*0x03 = (x+1)*(x+1) = x^2+x + x+1 = 101 = 0x05
}

//a(0이 아닌 원소)의 위수(order)
int GF256_ord(byte a) {
    byte temp;
    int order;

    if (a == 0) { // ord(0)은 정의되지 않음
        return -1; //의미 없는 결과임
    }
    temp = a;
    order = 1;
    while (temp != 1) { // temp == 1 이면 반복을 종료
        //temp *= a; // a의 거듭제곱 temp = a, a^2, a^3, ...
        temp = GF256_mul(temp, a);
        order++;
    }
    return order;
}

// GF(2^8)의 곱셈의 역원
// a의 역원 a^254 = a^{1111 1110} = a^[ {1000 0000}+{0100 0000}+...+{0000 0010}]
//               = a^{128+64+32+16+8+4+2} = a^128*a^64*a^32*a^16*a^8*a^4*a^2
byte GF256_inv(byte a) {
    byte inverse; // 역원을 저장
    byte temp; // a^2, a^4, ... 저장

    if (a == 0) { // 0의 역원은 없다.
        return 0; // 0^{-1} = 0 으로 출력 
    }

    inverse = 1; // 초기값
    temp = a;   // 초기값
    for (int i = 0; i < 7; i++) {
        temp = GF256_mul(temp, temp);  // a^2, a^4, ... , a^128
        inverse = GF256_mul(inverse, temp); // a^2*a^4*....
    }
    return inverse;
}


void order_inverse_test() {
    int num_max_order; //최대 위수를 갖는 원소의 개수
    int order; // 원소의 위수
    byte inv; 
    num_max_order = 0;
    printf("The orders of elements in GF(2^8)\n");
    for (int i = 0; i < 256; i++) {
        order = GF256_ord(i);
        inv = GF256_inv(i);
        printf("ord(%3d) = %d, %02x^(-1) = %02x, %02x*%02x=%02x\n", 
            i, order, i, inv, i, inv, GF256_mul(i,inv));
        if (order == 255) num_max_order++;
    }
    printf("The number of Max order elements = %d\n", num_max_order);
}

//Affine변환: w --> Aw+b
// w는 GF(2)^8 원소 (8차원 벡터)  w = [w7w6w5w4 w3w2w1w0]
byte AES_Affine(byte w) {
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
            y_vec[i] ^= A[i][j] * w_vec[j];
        }
    }
    // y_vec --> y (바이트)
    byte y;
    y = 0;
    //y_vec = { y0,y1,y2, ... ,y7 } --> y = [y7y6y5y4 y3y2y1y0]
    for (int i = 0; i < 7; i++) {
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
void Get_AES_Sbox(byte S[256]) {
    byte w; // 중간값 w 저장
    for (int i = 0; i < 256; i++) {
        w = GF256_inv(i);
        S[i] = AES_Affine(w);
    }
}

//Sbox 만들기
void print_sbox() {
    byte S[256];

    Get_AES_Sbox(S);

    printf("Sbox[256] = {\n");
    for (int i = 0; i < 256; i++) {
        if (i < 255) {
            printf("0x%02x, ", S[i]);
        }
        else {
            printf("0x%02x };\n", S[i]);
        }
        if ((i % 16) == 15) printf("\n");        
    }
}

int main() {
    //GF256_element();
    //GF256_mul_test();
    //order_inverse_test();
    print_sbox();


    return 0;
}