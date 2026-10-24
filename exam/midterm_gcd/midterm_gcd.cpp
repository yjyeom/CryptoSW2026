// exEnclidAlgorithm.cpp : 
//

#include <stdio.h>

typedef unsigned char byte;
//=================================================================
// 수업시간에 사용한 코드
//=================================================================
//gcd를 구하는 함수
// a = bq+r 이면, gcd(a,b) = gcd(b,r)
// 반복하면서 b==0 일때, a가 gcd이다.
//  a = bq + r
//  b = rq'+r' ...
int gcd(int a, int b) {
    int r, q; // r:나머지, q: 몫
    while (b != 0) {
        r = a % b; // a를 b로 나눈 나머지
        q = a / b; // a를 b로 나눈 몫

        printf("%d = %d*%d + %d\n", a, b, q, r);//a=bq+r

        a = b;
        b = r;
    }
    return a;
}

//확장 유클리드 알고리즘
// gcd(a,b) = a*x + b*y 인 x,y도 gcd와 함께 구하기
// 최초 입력 파라미터로 받은 a,b는 변하지 않게
// new_a = ua*a + va*b  (a,b의 일차결합)
// new_b = ub*a + vb*b  (a,b의 일차결합)
int ex_gcd(int a, int b) {
    int q; // q: 몫
    int ua, va, ub, vb;
    int n_ua, n_va, n_ub, n_vb;
    int an, bn; // 계속 업데이트되는 a,b의 값을 an, bn이라 하자.
    int new_an, new_bn;

    an = a; bn = b;
    ua = 1; va = 0; // an = ua*a + va*b (초깃값 = 1*a + 0*b)
    ub = 0; vb = 1; // bn = ub*a + vb*b (초깃값 = 0*a + 1*b)

    while (bn != 0) {
        // an = bn*q+r 
        // new_an = bn = ub*a + vb*b (n_ua = ub, n_va = vb)
        // new_bn = r = an - bn*q = (ua*a + va*b) - (ub*a + vb*b)*q
        //        = (ua - ub*q)*a + (va - vb*q)*b = n_ub*a + n_vb*b        
        printf("%d=(%d)*%d+(%d)*%d,\t %d=(%d)*%d+(%d)*%d\n",
            an, ua, a, va, b, bn, ub, a, vb, b);

        q = an / bn;
        new_an = bn;
        new_bn = an - bn * q; //an = bn*q+r 

        n_ua = ub; n_va = vb;
        n_ub = ua - ub * q; n_vb = va - vb * q;

        an = new_an; bn = new_bn;
        ua = n_ua; va = n_va; ub = n_ub; vb = n_vb;
    }
    return an;
}

//확장 유클리드 알고리즘(C++ 버전)
// gcd(a,b) = a*x + b*y 인 x,y도 gcd와 함께 구하기 
// x, y값도 함수 결과로 활용하기
int ex_gcd_cpp(int a, int b, int& x, int& y) { // 
    int q; // q: 몫
    int ua, va, ub, vb;
    int n_ua, n_va, n_ub, n_vb;
    int an, bn; // 계속 업데이트되는 a,b의 값을 an, bn이라 하자.
    int new_an, new_bn;

    an = a; bn = b;
    ua = 1; va = 0; // an = ua*a + va*b (초깃값 = 1*a + 0*b)
    ub = 0; vb = 1; // bn = ub*a + vb*b (초깃값 = 0*a + 1*b)

    while (bn != 0) {
        // an = bn*q+r 
        // new_an = bn = ub*a + vb*b (n_ua = ub, n_va = vb)
        // new_bn = r = an - bn*q = (ua*a + va*b) - (ub*a + vb*b)*q
        //        = (ua - ub*q)*a + (va - vb*q)*b = n_ub*a + n_vb*b        
        printf("%d=(%d)*%d+(%d)*%d,\t %d=(%d)*%d+(%d)*%d\n",
            an, ua, a, va, b, bn, ub, a, vb, b);

        q = an / bn;
        new_an = bn;
        new_bn = an - bn * q; //an = bn*q+r 

        n_ua = ub; n_va = vb;
        n_ub = ua - ub * q; n_vb = va - vb * q;

        an = new_an; bn = new_bn;
        ua = n_ua; va = n_va; ub = n_ub; vb = n_vb;
    }
    x = ua; y = va; // an = ua*a + va*b
    return an;
}

//재귀호출(recursion)을 이용하는 방법
// 효율적이진 않지만 직관적으로 이해하기 쉬울수도...
int gcd_recursive(int a, int b) {
    // gcd(a,b) = gcd(b,r)이고, b==0이면 gcd=a
    if (b == 0) {
        return a;
    }
    else {
        printf("call gcd_recursive(%d,%d)\n", b, a % b);
        return gcd_recursive(b, a % b); // gcd(b,r)
    }

}

//간단한 재귀호출 버전
// 3항 연산자를 사용 ( result =  Q ? Y:N )
int gcd_simple(int a, int b) {
    return (b == 0) ? a : gcd_simple(b, a % b);
}


void gcd_test()
{
    int a, b, c, rc, sc;
    a = 97;
    b = 72;
    c = gcd(a, b);
    printf("a=%d, b=%d, gcd(%d,%d) = %d\n", a, b, a, b, c);

    printf("GCD recursive version\n");
    rc = gcd_recursive(a, b);
    printf("a=%d, b=%d, gcd_recursive(%d,%d) = %d\n", a, b, a, b, rc);

    printf("GCD simple version\n");
    sc = gcd_simple(a, b);
    printf("a=%d, b=%d, gcd_simple(%d,%d) = %d\n", a, b, a, b, sc);
}


void exgcd_test()
{
    int a, b, c, cpp_c;
    a = 84;
    b = 63;
    c = ex_gcd(a, b);
    printf("Extended Euclidean Algorithm\n");
    printf("a=%d, b=%d, gcd(%d,%d) = %d\n", a, b, a, b, c);

    printf("Extended Euclidean Algorithm (C++ version)\n");
    int x, y;
    cpp_c = ex_gcd_cpp(a, b, x, y);
    printf("a=%d, b=%d, gcd(%d,%d) = %d\n", a, b, a, b, cpp_c);
    printf("gcd(%d,%d) = (%d)*%d + (%d)*%d = %d\n", a, b, x, a, y, b, cpp_c);


}
//=================================================================
// 중간고사에서 만든 코드
//=================================================================
int my_gcd(int a, int b) {
    printf("gcd(%d,%d)\n", a, b);
    return (b == 0) ? a : my_gcd(b, a % b);
}

int my_ex_gcd(int a, int b) {
    int q; 
    int ua, va, ub, vb;
    int n_ua, n_va, n_ub, n_vb;
    int an, bn; 
    int new_an, new_bn;

    an = a; bn = b;
    ua = 1; va = 0; // an = ua*a + va*b 
    ub = 0; vb = 1; // bn = ub*a + vb*b 

    while (bn != 0) {  
        printf("%d=(%d)*%d+(%d)*%d,\t %d=(%d)*%d+(%d)*%d\n",
            an, ua, a, va, b, bn, ub, a, vb, b);
        q = an / bn;
        new_an = bn;
        new_bn = an - bn * q; //an = bn*q+r 

        n_ua = ub; n_va = vb;
        n_ub = ua - ub * q; n_vb = va - vb * q;

        an = new_an; bn = new_bn;
        ua = n_ua; va = n_va; ub = n_ub; vb = n_vb;
    }
    return an;
}


void my_gcd_test()
{
    int a, b, c;
    a = 63;
    b = 84;
    c = my_gcd(a, b);
    printf("a=%d, b=%d, gcd(%d,%d) = %d\n", a, b, a, b, c);
}

void my_exgcd_test()
{
    int a, b, c, cpp_c;
    a = 84;
    b = 63;
    c = my_ex_gcd(a, b);
    printf("a=%d, b=%d, gcd(%d,%d) = %d\n", a, b, a, b, c);
}

#define ARRAY_8BIT_BAD(b, arr) {\
    arr[0] = (*b >> 7) & 0x01; \
    arr[1] = (*b >> 6) & 0x01; \
    arr[2] = (*b >> 5) & 0x01; \
    arr[3] = (*b >> 4) & 0x01; \
    arr[4] = (*b >> 3) & 0x01; \
    arr[5] = (*b >> 2) & 0x01; \
    arr[6] = (*b >> 1) & 0x01; \
    arr[7] = (*b >> 0) & 0x01; \
}


void my_macro_bad_test() {
    byte date[3] = { 0x12, 0x34, 0x56 };
    byte arr[8];

    for (int i = 0; i < 3; i++) {
        ARRAY_8BIT_BAD(date + i, arr);
        printf("date[%d] = %02x, arr = {%d,%d,%d,%d,%d,%d,%d,%d}\n",
            i, date[i], arr[0], arr[1], arr[2], arr[3], arr[4], arr[5], arr[6], arr[7]);
    }
}

#define ARRAY_8BIT(b, arr) {\
    arr[0] = (*(b) >> 7) & 0x01; \
    arr[1] = (*(b) >> 6) & 0x01; \
    arr[2] = (*(b) >> 5) & 0x01; \
    arr[3] = (*(b) >> 4) & 0x01; \
    arr[4] = (*(b) >> 3) & 0x01; \
    arr[5] = (*(b) >> 2) & 0x01; \
    arr[6] = (*(b) >> 1) & 0x01; \
    arr[7] = (*(b) >> 0) & 0x01; \
}

void my_macro_test() {    
	byte date[3] = { 0x12, 0x34, 0x56 };
    byte arr[8];

    for(int i=0; i<3; i++) {
		ARRAY_8BIT(date+i, arr);
        printf("date[%d] = %02x, arr = {%d,%d,%d,%d,%d,%d,%d,%d}\n",
			i, date[i], arr[0], arr[1], arr[2], arr[3], arr[4], arr[5], arr[6], arr[7]);
	}
}



//=================================================================
int main() {
    //my_gcd_test();
    //my_exgcd_test();
    
    my_macro_bad_test();
	my_macro_test();

    return 0;
}
