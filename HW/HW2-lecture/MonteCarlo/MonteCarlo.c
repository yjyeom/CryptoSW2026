#include <stdio.h>
#include <math.h>
#include "AES32.h"

#define UINT32_MAX 0xFFFFFFFF

// SEED
static byte SEED[16];

//---------------------------------------
// AES_OFB 방식의 난수생성 

// 초깃값(seed, iv) 설정
void RBG_SetSeed(byte iv[16]) {
    for (int i = 0; i < 16; i++) {
        SEED[i] = iv[i];
	}
}

// 32비트 난수 생성
u32 RBG_Getrand32() {
    byte rk[11][16] = { 0, };
	byte pt[16], ct[16];
    for (int i = 0; i < 16; i++) {
        pt[i] = SEED[i];
    }    
    AES32_Encrypt(pt, rk, ct);
    u32 rand32 = GETU32(ct);
    for (int i = 0; i < 16; i++) {
        pt[i] = ct[i];
    }
    AES32_Encrypt(pt, rk, ct);
    for (int i = 0; i < 16; i++) {
        SEED[i] = ct[i];
    }    
	return rand32;
}
//---------------------------------------


//---------------------------------------
// Monte Carlo pi
double MonteCarlo_pi(int N) {
    int count = 0;
    double pi;	

    for (int i = 0; i < N; i++) {
		// 0~1 사이의 난수 x, y 생성
        u32 rand_x = RBG_Getrand32();
        u32 rand_y = RBG_Getrand32();
        double x = (double)rand_x / (double)UINT32_MAX;
        double y = (double)rand_y / (double)UINT32_MAX;
		// 원 안에 점이면 카운터 증가
        if (x * x + y * y <= 1.0) {
            count++;
        }
    }

	// 원에 들어갈 확률 = pi/4 이므로 pi = 4 * (count / N)
    pi = 4.0 * (double)count / (double)N;
    return pi;
}

void MonteCarlo_pi_test() {
    int N = 100000; // 반복횟수
    double pi;
	byte* seed = "1234567890123456"; // 난수용 초깃값
	RBG_SetSeed(seed); //  난수용 초깃값 설정
	printf("== Run Monte Carlo pi test (N=%d)\n", N);
    for (int i = 0; i < 10; i++) {
        pi = MonteCarlo_pi(N);
        printf("[%2d] Monte Carlo pi (N=%d): %.6f\n", i+1, N, pi);
    }
	printf("\n");
}


//---------------------------------------
// Monte Carlo integral

// 적분할 함수 f(x) = e^(x^2)
double f(double x) {    
    return exp(-x * x);
}

// Int_a^b f(x) dx  (0<f(x)<1)
double MonteCarlo_integral(double a, double b, int N) {    
    int count = 0;
    for (int i = 0; i < N; i++) {
        u32 rand_x = RBG_Getrand32();
        u32 rand_y = RBG_Getrand32();
        double x = a + (b - a) * ((double)rand_x / (double)UINT32_MAX);
		double y = (double)rand_y / (double)UINT32_MAX;
        if (y <= f(x)) {
            count++;
		}
    }
    return (b - a) * (double)count / (double)N;
}

void MonteCarlo_integral_test() {
    double a = 0.0;
    double b = 1.0;
    int N = 100000; 
    double integral;
    byte* seed = "1234567890123456"; // 난수용 초깃값
    RBG_SetSeed(seed); //  난수용 초깃값 설정
    printf("== Run Monte Carlo integral test (N=%d)\n", N);
    RBG_SetSeed(seed);

    for (int i = 0; i < 10; i++) {
        integral = MonteCarlo_integral(a, b, N);
        printf("[%2d] Monte Carlo integral (N=%d): %.6f\n", i + 1, N, integral);
    }
	printf("\n");
}


int main()
{
    MonteCarlo_pi_test();

    MonteCarlo_integral_test();
}

