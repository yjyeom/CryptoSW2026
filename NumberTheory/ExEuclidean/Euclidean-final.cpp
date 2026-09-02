// Euclidean.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <stdio.h>

int gcd(int a, int b) {
    // a = b*q + r ==> gcd(a,b) = gcd(b,r)
    // 반복하여 수행한다==> b=0일 때, a가 gcd다.
    int r, q;

    while (b != 0) {
        r = a % b; //a를 b로 나눈 나머지
        //중간 계산과정을 출력해보자.
        q = a / b; //a를 b로 나눈 몫
        printf("%d = %d*%d + %d\n", a, b, q, r);
        a = b;
        b = r;
    }
    return a; 
}

int gcd_recursive(int a, int b) {    
    /*
    if (b == 0) {
        return a;
    }
    else {        
        return gcd_recursive(b, a%b);
    }
    */
    return (b == 0) ? a : gcd_recursive(b, a % b);
}

// 확장 유클리드 알고리즘: ax+by = gcd(a,b)인 x,y도 출력하기
// 각 단계에서 an = ua*a + va*b, bn = ub*a + vb*b로 표기를 유지
// 마지막 단계(b==0)에서 an = gcd(a,b)= ua*a + va*b => (x=ua, y=va)를 출력

//C언어의 파라미터 전달방식: call by value (값만 전달하는 방식)
//==> C++의 참조(call by reference)로 바꾸면 결과값을 얻을 수 있다.
//    aliasing (전달한 변수의 별명으로 관리, 변수 자체를 받아온 것과 동일)
int ex_gcd(int a, int b, int &x, int &y) {
    int an, bn; // 각 단계에서의 a,b 값   
    int ua, va, ub, vb; // an = ua*a + va*b, bn = ub*a + vb*b
    int new_an, new_bn; // 다음 단계 an, bn을 미리 계산한 값
    int n_ua, n_va, n_ub, n_vb; // 다음 단계 ua, va, ub, vb
    int q; // an을 bn으로 나눈 몫

    an = a; bn = b;
    ua = 1; va = 0; // 초깃값 an = 1*a + 0*b = a
    ub = 0; vb = 1; // 초깃값 bn = 0*a + 1*b = b
    while (bn != 0) {
        // an = bn*q + r ==> new_an = bn, new_bn (= r) = an-bn*q
        // new_an = ub*a + vb*b
        // new_bn = an-bn*q = (ua*a + va*b) - (ub*a + vb*b)*q
        //        = (ua-ub*q)*a + (va-vb*q)*b
		printf("%d=%d*%d+%d*%d,\t %d=%d*%d+%d*%d\n", an, ua, a, va, b, bn, ub, a, vb, b);
        q = an / bn;
        new_an = bn;
        new_bn = an - bn * q;
        
        n_ua = ub; n_va = vb;
        n_ub = ua - ub * q; n_vb = va - vb * q;

        an = new_an; bn = new_bn;
        ua = n_ua; va = n_va; ub = n_ub; vb = n_vb;
    }
    //x, y 리턴
    x = ua; y = va; // an = gcd(a,b) = x*a + y*b 이므로
    printf("[DEBUG] x=%d, y=%d, an= %d\n", x, y, an);
    return an; 
}

int main()
{
    int a, b, g;
    //a = 90;  b = 63;
    //a = 44;  b = 35;
    a = 24; b = 16;
    
    g = gcd(a, b);

    printf("a= %d, b=%d, gcd(%d,%d)=%d\n", a, b, a, b, g);

    int gr;
    gr = gcd_recursive(a, b);
    printf("a= %d, b=%d, gcd_recursive(%d,%d)=%d\n", a, b, a, b, gr);
    

    int xx, yy;
    xx = 1; yy = 1;
    g = ex_gcd(a, b, xx, yy);

    printf("a= %d, b=%d, ex_gcd(%d,%d)=%d\n", a, b, a, b, g);
    printf("gcd(%d,%d) = %d = %d*%d + %d*%d\n", a, b, g, xx, a, yy, b);

}
