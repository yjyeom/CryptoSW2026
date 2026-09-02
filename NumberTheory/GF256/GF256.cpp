// GF256.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <stdio.h>  // C언어 헤더 파일

//바이트 저장용 데이터 형 정의
// byte B; // B = b7b6...b1b0 = b7x^7 + b6x^6 + ... + b1x + b0
typedef unsigned char byte; // byte는 GF(2^8) 유한체의 원소를 저장하는 데이터 유형

//유한체 GF(2^8)의 원소 a,b를 더하기
// a = a7a6...a1a0 = a7x^7 + a6x^6 + ... + a1x + a0
// b = b7b6...b1b0 = b7x^7 + b6x^6 + ... + b1x + b0
// c = a + b = (a7+b7)x^7 + (a6+b6)x^6 + ... + (a0+b0)
// c = (a7^b7)(a6^b6)...(a0^b0) = (a7a6...a1a0)^(b7b6...b1b0)
// 계수들의 덧셈(예: a7+b7)은 GF(2)의 덧셈 (계수는 0 또는 1)
// 계수들의 덧셈은 xor와 동일하다. 예: a7+b7 = a7 xor b7 = a7 ^ b7
byte GF256_add(byte a, byte b) {
    return a ^ b;
}

// GF(2^8)의 원소를 비트열로 출력하기
// x = x7x6...x0 ==> [x7, x6, ... , x0]
void GF256_print_bin(byte x) {
    byte x_bin[8]; //각 비트를 저장할 배열
    for (int i = 0; i < 8; i++) {
        // x를 오른쪽으로 7,6,5,...,0번 쉬프트
        // x7,  x7x6, x7x6x5, .... , x7x6...x0
        x_bin[i] = (x >> (7 - i)) & 0x01; 
    }
    for (int i = 0; i < 8; i++) {
        printf("%d", x_bin[i]);
    }
    printf("\n");
}

// GF(2^8)의 원소를 다항식으로 출력하기
// a = [a7a6...a0] => a7x^7 + ... + a0
void GF256_print_poly(byte a) {
    byte a_bin[8]; //각 비트를 저장할 배열 [a7, a6, ... , a0]
    for (int i = 0; i < 8; i++) { 
        // a를 오른쪽으로 7,6,5,...,0번 쉬프트
        a_bin[i] = (a >> (7 - i)) & 0x01;
    }
    int coef, expo;

    for (int i = 0; i < 7; i++) {  // x^7항부터 x항까지
        coef = a_bin[i]; // a7, a6, ... , a1
        expo = 7 - i; // 7, 6, .... , 1
        if (coef == 1) {
            printf("x^%1d + ", expo);
        }
    }
    //상수항
    coef = a_bin[7];
    if (coef == 1) {
        printf("1\n");
    }
    else {
        printf("0\n");
    }
}

// 다항식에 x를 곱하기
// a = a7a6...a1a0 = a7x^7 + a6x^6 + ... + a1x + a0
// xtime(a) = a6x^7 + a5x^6 + ... + a0x (a7 = 0)
// xtime(a) = a6x^7 + a5x^6 + ... + a0x + [x^4+x^3+x+1] (a7 = 1) 
// [x^4+x^3+x+1] = [0001 1011] = [0x1b]
byte GF256_xtime(byte a) {
    int msb; // a7을 저장(7차항의 계수)
    byte result; // 계산결과를 저장

    msb = (a >> 7) & 0x01; // a7
    if (msb == 0) { // a7=0
        result = a << 1; //[a6a5...a0 0]
    }
    else { // a7=1
        result = (a << 1) ^ 0x1b; // [0x1b] = [x^4+x^3+x+1] = [x^8]
    }
    return result;
}

byte GF256_xtime_simple(byte a) {
    return (((a >> 7) & 0x01) == 0) ? a << 1 : (a << 1) ^ 0x1b;
}

// GF(2^8)의 (다항식) 곱셈
// f(x) = a7x^7 + a6x^6 + ... + a1x + a0
// g(x)
// h(x) = g(x)f(x) = g(x)*a7x^7 + g(x)*a6x^6 + ... + g(x)*a1x + g(x)*a0
// 단계별 계산:
//      h(x) <= g(x)*a7
//      h(x) <= x*g(x)*a7 + g(x)*a6
//      h(x) <= x*(x*g(x)*a7 + g(x)*a6) + g(x)*a5
//            ...
//      h(x) <= x*(   ...   + g(x)*a1) + g(x)*a0
//  계수 ai에 g(x)를 곱한 결과는 g(x) 또는 0 뿐이다!
byte GF256_mul(byte f, byte g) {
    byte h; // 곱셈결과 저장
    int coef; // 계수
    
    h = 0x00; // 초깃값 h(x) = 0
    for (int i = 7; i >= 0; i--) { // a7, a6, a5, ... ,a1, a0 순서로 반복
        coef = (f >> i) & 0x01; // a7, a6, a5, ... ,a1, a0
        //h = GF256_xtime(h); 
        h = GF256_xtime_simple(h);
        if (coef == 1) {  // 계수 ai가 1인 경우만 g(x)를 h(x)에 더한다
            h = GF256_add(h, g);
        }
    }
    return h;
}




//=====================//

void test_GF256() {
    byte a, b, c;
    //a = 0x57; b = 0x83;
    a = 0x9a; b = 0x81;
    c = GF256_mul(a, b); // 0xc1 = 0x57*0x83

    printf("a = %2d = 0x%02x = ", a, a);
    GF256_print_bin(a);
    GF256_print_poly(a);
    printf("b = %2d = 0x%02x = ", b, b);
    GF256_print_bin(b);
    GF256_print_poly(b);
    printf("c = %2d = 0x%02x = ", c, c);
    GF256_print_bin(c);
    GF256_print_poly(c);


}

int main()
{
    test_GF256();
}

// 프로그램 실행: <Ctrl+F5> 또는 [디버그] > [디버깅하지 않고 시작] 메뉴
// 프로그램 디버그: <F5> 키 또는 [디버그] > [디버깅 시작] 메뉴

// 시작을 위한 팁: 
//   1. [솔루션 탐색기] 창을 사용하여 파일을 추가/관리합니다.
//   2. [팀 탐색기] 창을 사용하여 소스 제어에 연결합니다.
//   3. [출력] 창을 사용하여 빌드 출력 및 기타 메시지를 확인합니다.
//   4. [오류 목록] 창을 사용하여 오류를 봅니다.
//   5. [프로젝트] > [새 항목 추가]로 이동하여 새 코드 파일을 만들거나, [프로젝트] > [기존 항목 추가]로 이동하여 기존 코드 파일을 프로젝트에 추가합니다.
//   6. 나중에 이 프로젝트를 다시 열려면 [파일] > [열기] > [프로젝트]로 이동하고 .sln 파일을 선택합니다.
