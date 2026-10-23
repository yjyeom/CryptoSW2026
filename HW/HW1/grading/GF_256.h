#ifndef GF_256_H
#define GF_256_H

// 데이터 형 : 바이트
// byte b = b7b6b5b4 b3b2b1b0 = b7*x7 + b6*x6 + ... + b1*x1 + b0*x0
// 예 : 0x0a = 0000 1010 = x^3 + x
typedef unsigned char byte; // 0~255 ==> 256가지의 다항식, char(문자) : 1 byte, unsigned : 음수가 없는

byte GF256_add(byte a, byte b);
void GF256_print_bin(byte b);
void GF256_print_poly(byte b);
byte GF256_xtime(byte a);
byte GF256_mul(byte a, byte b);
byte GF256_inv(byte a);
int GF256_ord(byte a);
byte AES_Affine(byte w);
void Get_AES_Sbox(byte S[256]);
void Get_AES_ISbox(byte IS[256]);
void Print_Sbox();
void Print_ISbox();
void GF256_Matrix_Print(byte A[4][4]);
void GF256_Matrix_Mul(byte A[4][4], byte B[4][4], byte C[4][4]);
void MixCol(byte in[4], byte out[4]);
void InvMixCol(byte in[4], byte out[4]);
void GF256_element();
void GF256_mul_test();
void order_inverse_test();
void MC4_test();
void MixCol_InvMixCol_test();

#endif // GF_256_H