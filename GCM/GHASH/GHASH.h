#pragma once

#include "HexByte.h"

void GF128_xtime(byte p[16]); 
void GF128_mul(byte p[16], byte q[16]);
void GHASH_ver0(byte msg[], int msg_blocks, byte H[16], byte tag[16]);
void Make_GHASH_H_table(byte H[16], byte HT[256][16]);
void GF128_Hmul(byte state[16], byte HT[256][16], byte R0[256], byte R1[256]);
void GHASH(byte msg[], int msg_blocks,
	byte HT[256][16], byte R0[256], byte R1[256],
	byte tag[16]);
void Make_GHASH_const_R0R1(byte R0[256], byte R1[256]);




