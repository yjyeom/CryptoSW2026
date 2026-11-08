#include <stdio.h>
#include "AES32.h"

#define UINT32_MAX 0xFFFFFFFF

// SEED
static byte SEED[16];

// AES_OFB 방식의 RBG 
void RBG_SetSeed(byte iv[16]) {
    for (int i = 0; i < 16; i++) {
        SEED[i] = iv[i];
	}
}

u32 RBG_Getrand32() {
    byte rk[11][16] = { 0, };
	byte pt[16], ct[16];
    for (int i = 0; i < 16; i++) {
        pt[i] = SEED[i];
    }    
    AES32_Encrypt(pt, rk, ct);
    for (int i = 0; i < 16; i++) {
        SEED[i] = ct[i];
    }
    u32 rand32 = GETU32(ct);
	return rand32;
}

//---------------------------------------
// Monte Carlo pi
double MonteCarlo_pi(int N) {
    int count = 0;
    double pi;	

    for (int i = 0; i < N; i++) {
        u32 rand_x = RBG_Getrand32();
        u32 rand_y = RBG_Getrand32();
        double x = (double)rand_x / (double)UINT32_MAX;
        double y = (double)rand_y / (double)UINT32_MAX;
        if (x * x + y * y <= 1.0) {
            count++;
        }
    }
    pi = 4.0 * (double)count / (double)N;
    return pi;
}

void MonteCarlo_pi_test() {
    int N = 100000; 
    double pi;
    RBG_SetSeed((byte*)"1234567890123456");

    for (int i = 0; i < 10; i++) {
        pi = MonteCarlo_pi(N);
        printf("[%2d] Monte Carlo pi (N=%d): %.6f\n", i+1, N, pi);
    }
}

//---------------------------------------
// Monte Carlo integral

double f(double x) {
    return x * x; // Example function: f(x) = x^2
}

// Int_a^b f(x) dx 
double MonteCarlo_integral(double a, double b, int N) {
    double sum = 0.0;
    for (int i = 0; i < N; i++) {
        u32 rand_x = RBG_Getrand32();
        double x = a + (b - a) * ((double)rand_x / (double)UINT32_MAX);
        sum += f(x);
    }
    return (b - a) * sum / (double)N;
}

void MonteCarlo_integral_test() {
    double a = 0.0;
    double b = 1.0;
    int N = 100000; 
    double integral;
    RBG_SetSeed((byte*)"1234567890123456");
    for (int i = 0; i < 10; i++) {
        integral = MonteCarlo_integral(a, b, N);
        printf("[%2d] Monte Carlo integral (N=%d): %.6f\n", i+1, N, integral);
    }
}



int main()
{
    MonteCarlo_pi_test();

    MonteCarlo_integral_test();
}

