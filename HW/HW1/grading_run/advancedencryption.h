#ifndef AES_HEADER_H
#define AES_HEADER_H

#include<stdio.h>
#include<stdlib.h>

#define MAX_SIZE 20
#define ALMOST_ZERO 1e-10 // 10^(-10)보다 작은 절대값은 0으로 간주

typedef unsigned char byte;

typedef struct {
	double M[MAX_SIZE][MAX_SIZE]; //사이즈를 일단 10x10으로 정함
	int row; //행
	int col; //열
}Matrix;

#define xtimes(in) (((in) << 1) ^ ((((in) >> 7)&0x01) * 0x1b)) //if 없이 구현

extern const byte s_box[256];

//================================================================================ 기능성 함수

void print_GF2_8_element(byte a); //유한체 다항식 8비트 출력

void print_Matrix(Matrix A); //행렬 출력하기

void print_sbox_16x16(const byte* sbox, int len); //s_box 만들고 출력

//================================================================================유클리드 알고리즘 함수

byte EucAlg(byte a, byte b);

byte EucAlg_rec(byte a, byte b);

byte EucAlg_simple(byte a, byte b);

byte Ex_euc_al(byte a, byte b); //a와 b의 최대공약수(gcd)를 반환하며, 베주 항등식 진행 과정을 출력

//================================================================================유한체 연산 함수

void print_GF2_8_add(byte a, byte b);
//유한체 2^8에서의 곱셈을 xtimes로 구현
byte Multiply_GF2_8(byte a, byte b);

byte Findinverse_GF2_8(byte a); //유한체에서 a^254 = a 역원임을 이용한 역원찾는 함수

void Make_sbox(byte* sbox); //유한체에서 AES S-Box 행렬연산으로 구현한 함수

//================================================================================행렬 연산 함수

Matrix Mat_addition_R(Matrix A, Matrix B); // 행렬덧셈 함수

Matrix Mat_multiplication_R(Matrix A, Matrix B); //행렬곱 함수

void Mat_exchange_rows(Matrix* A, int row1, int row2); //열 교환 함수

void Mat_scalar_row_mul(Matrix* A, double k, int row); //열 스칼라곱 함수

void Mat_row_add(Matrix* A, double k, int src_row, int target_row); // 열간 덧셈연산 함수

Matrix Mat_formalization(); // 행렬 초기화(=>[0]) 함수

void Mat_Make_sbox(byte* s, Matrix M); //S-Box 생성 Matrix를 직접 입력해줘야 하는 함수


#endif // !AES_HEADER_H

#pragma once
