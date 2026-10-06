#pragma once

typedef unsigned char byte; // 0~255 ==> 256가지의 다항식

byte GF256_add(byte a, byte b);
void GF256_print_bin(byte b);
void GF256_print_poly(byte b);
byte GF256_xtime(byte a);
byte GF256_mul(byte a, byte b);
int GF256_ord(byte a);
byte GF256_inv(byte a);
byte AES_Affine(byte w);
void Get_AES_Sbox(byte S[256]);
void Get_AES_ISbox(byte IS[256]);

void MixCol(byte in[4], byte out[4]);
void InvMixCol(byte in[4], byte out[4]);


