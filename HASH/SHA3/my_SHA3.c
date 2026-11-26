// SHA3-internal-ftn.cpp
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "sha3.h"
#define KECCAK_SPONGE_BIT		1600
#define KECCAK_ROUND			24
#define KECCAK_STATE_SIZE		200

#define KECCAK_SHA3_224			224
#define KECCAK_SHA3_256			256
#define KECCAK_SHA3_384			384
#define KECCAK_SHA3_512			512
#define KECCAK_SHAKE128			128
#define KECCAK_SHAKE256			256

#define KECCAK_SHA3_SUFFIX		0x06
#define KECCAK_SHAKE_SUFFIX		0x1F

//라운드 상수
static const uint32_t keccakf_rndc[KECCAK_ROUND][2] =
{
	{0x00000001, 0x00000000}, {0x00008082, 0x00000000},
	{0x0000808a, 0x80000000}, {0x80008000, 0x80000000},
	{0x0000808b, 0x00000000}, {0x80000001, 0x00000000},
	{0x80008081, 0x80000000}, {0x00008009, 0x80000000},
	{0x0000008a, 0x00000000}, {0x00000088, 0x00000000},
	{0x80008009, 0x00000000}, {0x8000000a, 0x00000000},

	{0x8000808b, 0x00000000}, {0x0000008b, 0x80000000},
	{0x00008089, 0x80000000}, {0x00008003, 0x80000000},
	{0x00008002, 0x80000000}, {0x00000080, 0x80000000},
	{0x0000800a, 0x00000000}, {0x8000000a, 0x80000000},
	{0x80008081, 0x80000000}, {0x00008080, 0x80000000},
	{0x80000001, 0x00000000}, {0x80008008, 0x80000000}
};


// 24개 레인별 상수 (0~24 lane)
static const unsigned keccakf_rotc[KECCAK_ROUND] =
{
	 1,  3,  6, 10, 15, 21, 28, 36, 45, 55,  2, 14,
	27, 41, 56,  8, 25, 43, 62, 18, 39, 61, 20, 44
};

// 24개 pi 함수용 상수 
static const unsigned keccakf_piln[KECCAK_ROUND] =
{
	10,  7, 11, 17, 18,  3,  5, 16,  8, 21, 24,  4,
	15, 23, 19, 13, 12,  2, 20, 14, 22,  9,  6,  1
};

//KISA 소스코드의 ROL64의 오류 수정
// 64비트 데이터 [msb (in[1], in[0]) lsb]를  offset만큼 왼쪽으로 회전
void ROL64_new(uint32_t* in, uint32_t* out, int offset)
{
	int shift = 0;

	if (offset == 0)
	{
		out[1] = in[1];
		out[0] = in[0];
	}
	else if (offset < 32)
	{
		shift = offset;

		out[1] = (uint32_t)((in[1] << shift) ^ (in[0] >> (32 - shift)));
		out[0] = (uint32_t)((in[0] << shift) ^ (in[1] >> (32 - shift)));
	}
	else if (offset == 32) // 32비트는 별도로 처리해야 함 <<n (n은 0-31)만 정의됨.
	{
		out[1] = in[0];  // in[1] in[0] ==> in[0] in[1]
		out[0] = in[1];
	}
	else if (offset < 64)
	{
		shift = offset - 32;

		out[1] = (uint32_t)((in[0] << shift) ^ (in[1] >> (32 - shift)));
		out[0] = (uint32_t)((in[1] << shift) ^ (in[0] >> (32 - shift)));
	}
	else // offset == 64
	{
		out[1] = in[1];
		out[0] = in[0];
	}
}

// rho 함수에서 사용할 offset의 계산
// 슬라이스 xy[0-24]에서 각 레인의 회전량을 저장함
// offset: g(t) = (t+1)(t+2)/2
// 선형변환: x' = y, y' = 2x+3y
void rho_offset(int offset[25])
{
	int x, y, t;
	x = 1; y = 0;
	for (t = 0; t < 24; t++)
	{
		offset[5 * y + x] = ((t + 1) * (t + 2) / 2) % 64;
		printf("t = %d, x = %d, y = %d, offset[%d] = %d\n", t, x, y, 5 * y + x, offset[5 * y + x]);

		int tmp = x;
		x = y;
		y = (2 * tmp + 3 * y) % 5;
	}
	offset[0] = 0; // 가운데는 회전하지 않음
}

// KISA SHA3 내부 상수 만드는 함수
void print_rho_constants() {
	int x, y, t, ofs, tmp;
	int offset[25] = { 0, };
	int rot_const[24] = { 0, };
	int pi_const[24] = { 0, };

	x = 1; y = 0;
	for (t = 0; t < 24; t++)
	{
		ofs = ((t + 1) * (t + 2) / 2) % 64;
		offset[5 * y + x] = ofs;
		rot_const[t] = ofs;

		tmp = x;
		x = y;
		y = (2 * tmp + 3 * y) % 5;

		pi_const[t] = 5 * y + x;
	}

	printf("rho_offset = { ");
	for (int i = 0; i < 25; i++)
	{
		printf("%d, ", offset[i]);
	}
	printf("};\n");

	printf("keccakf_rotc[24] = {\n\t");
	for (int i = 0; i < 24; i++)
	{
		printf("%d ", rot_const[i]);
		if (i < 23) printf(", ");
		if (i == 11) printf("\n\t");
	}
	printf("};\n");

	printf("keccakf_piln[24] ={\n\t");
	for (int i = 0; i < 24; i++)
	{
		printf("%d ", pi_const[i]);
		if (i < 23) printf(", ");
		if (i == 11) printf("\n\t");
	}
	printf("};\n");
}


void print_rotation_by_stallings() {
	int t, x, y, gt, gtmod64, tmp, idx;

	printf("  t (x,y)=idx  g(t) g(t) mod 64\n");

	x = 1; y = 0; // 초깃값	
	for (t = 0; t < 24; t++) {
		gt = (t + 1) * (t + 2) / 2;
		gtmod64 = gt % 64;
		idx = 5 * y + x;

		printf("%3d (%d,%d)=%3d %4d %4d\n", t, x, y, idx, gt, gtmod64);

		tmp = x;
		x = y;
		y = (2 * tmp + 3 * y) % 5;
	}

}


int ROL64_test()
{
	uint32_t in[2] = { 0x01234567, 0x89abcdef };
	uint32_t out[2];

	printf("Before rotation: \t  in[1] = %08x,  in[0] = %08x\n", in[1], in[0]);

	ROL64_new(in, out, 0);
	printf("ROL64(in, out, 0): \t out[1] = %08x, out[0] = %08x\n", out[1], out[0]);
	ROL64_new(in, out, 4);
	printf("ROL64(in, out, 4): \t out[1] = %08x, out[0] = %08x\n", out[1], out[0]);
	ROL64_new(in, out, 32);
	printf("ROL64(in, out, 32): \t out[1] = %08x, out[0] = %08x\n", out[1], out[0]);
	ROL64_new(in, out, 36);
	printf("ROL64(in, out, 36): \t out[1] = %08x, out[0] = %08x\n", out[1], out[0]);
	ROL64_new(in, out, 64);
	printf("ROL64(in, out, 64): \t out[1] = %08x, out[0] = %08x\n", out[1], out[0]);

	// 비트 연산자 확인
	// <<32, >>32 연산은 undefined behavior 발생 
	/*
	int shift = 0;
	out[1] = (uint32_t)((in[0] << shift)); // ^ (in[1] >> (32 - shift)));
	out[0] = (uint32_t)((in[1] << shift)); // ^ (in[0] >> (32 - shift)));
	//out[1] = (uint32_t)((in[1] >> (32 - shift)));
	//out[0] = (uint32_t)((in[0] >> (32 - shift)));
	printf("ROL64(in, out, 32): out[1] = %08x, out[0] = %08x\n", out[1], out[0]);
	*/

	return 0;

}


void Rho_offset_test()
{
	int offset[25];
	rho_offset(offset);

	for (int y = 2; y > -3; y--)
	{
		for (int x = -2; x < 3; x++)
		{
			//int i = 05 * (y%5) + (x%5) ;
			int i;
			i = (5 * y + x) % 25;
			i = i >= 0 ? i : i + 25;
			//printf("offset[%d, %d] = offset[%d] = %d\n", x, y, i, offset[i]);
			//printf("%d:%3d ", i, offset[i]);
			printf("%3d ", offset[i]);
		}

		printf("\n");
	}

	printf("\n");

	int fips_table2[25] = { 153, 231, 3, 10, 171,
								55, 276, 36, 300, 6,
								28, 91, 0, 1, 190,
								120, 78, 210, 66, 253,
								21, 136, 105, 45, 15 };

	for (int y = 0; y < 5; y++)
	{
		for (int x = 0; x < 5; x++)
		{
			printf("%3d ", fips_table2[5 * y + x] % 64);
		}
		printf("\n");
	}

}

// 바이트배열 state[200] ==> 워드배열 s32[25][2]
void state_to32(uint8_t state[200], uint32_t s32[25][2]) {

	int i;

	for (i = 0; i < 25; i++)
	{
		s32[i][0] = (uint32_t)(state[i * 8 + 0]) |
			(uint32_t)(state[i * 8 + 1] << 8) |
			(uint32_t)(state[i * 8 + 2] << 16) |
			(uint32_t)(state[i * 8 + 3] << 24);
		s32[i][1] = (uint32_t)(state[i * 8 + 4]) |
			(uint32_t)(state[i * 8 + 5] << 8) |
			(uint32_t)(state[i * 8 + 6] << 16) |
			(uint32_t)(state[i * 8 + 7] << 24);
	}
}

// 워드배열 s32[25][2]  ==> 바이트배열 state[200]
void state_to8(uint8_t state[200], uint32_t s32[25][2]) {
	for (int i = 0; i < 25; i++)
	{
		state[i * 8 + 0] = (uint8_t)(s32[i][0]);
		state[i * 8 + 1] = (uint8_t)(s32[i][0] >> 8);
		state[i * 8 + 2] = (uint8_t)(s32[i][0] >> 16);
		state[i * 8 + 3] = (uint8_t)(s32[i][0] >> 24);
		state[i * 8 + 4] = (uint8_t)(s32[i][1]);
		state[i * 8 + 5] = (uint8_t)(s32[i][1] >> 8);
		state[i * 8 + 6] = (uint8_t)(s32[i][1] >> 16);
		state[i * 8 + 7] = (uint8_t)(s32[i][1] >> 24);
	}
}

/* Theta */
void theta(uint32_t s[25][2]) {
	uint32_t t[2], bc[5][2];
	int i, j;

	for (i = 0; i < 5; i++) { // i는 x축 인덱스
		bc[i][0] = s[i][0] ^ s[i + 5][0] ^ s[i + 10][0] ^ s[i + 15][0] ^ s[i + 20][0];  // y축 column의 합
		bc[i][1] = s[i][1] ^ s[i + 5][1] ^ s[i + 10][1] ^ s[i + 15][1] ^ s[i + 20][1];	// y축 column의 합
	}

	for (i = 0; i < 5; i++) {
		ROL64(bc[(i + 1) % 5], t, 1);  // t[1]t[0]에는 bc[]의 한 lane을 ROL64 회전하기

		t[0] ^= bc[(i + 4) % 5][0];
		t[1] ^= bc[(i + 4) % 5][1];

		for (j = 0; j < 25; j += 5) // j = y좌표 * 5 
		{
			s[j + i][0] ^= t[0];  // s[5y+x][0]
			s[j + i][1] ^= t[1];  // s[5y+x][1]
		}
	}
}

/* Rho */
void rho0(uint32_t s[25][2]) {
	int i;
	int rotate_lane[25]; // 각 lane의 rotation 정리
	uint32_t t[2]; // lane 임시 저장용

	rotate_lane[0] = 0;
	for (i = 0; i < 24; i++) {
		rotate_lane[keccakf_piln[i]] = keccakf_rotc[(i + 1) % 24];
	}
	//for (i = 0; i < 25; i++) {
	//	printf("%d:%d\n", i, rotate_lane[i]);
	//}
	for (i = 0; i < 25; i++) {
		t[0] = s[i][0];
		t[1] = s[i][1];
		ROL64(t, s[i], rotate_lane[i]);
	}
}

/* Rho */
void rho(uint32_t s[25][2]) {
	int i;
	uint32_t t[2]; // 임시 lane 

	for (i = 0; i < 24; i++) {
		t[0] = s[keccakf_piln[i]][0];
		t[1] = s[keccakf_piln[i]][1];
		ROL64(t, s[keccakf_piln[i]], keccakf_rotc[(i + 1) % 24]);
	}
}

/* Pi */
void pi(uint32_t s[25][2]) {
	int i, j;
	uint32_t t1[2]; // lane 임시 저장용
	uint32_t t2[2]; // lane 임시 저장용

	t1[0] = s[1][0];
	t1[1] = s[1][1];

	for (i = 0; i < KECCAK_ROUND; i++) // 24번
	{
		j = keccakf_piln[i];

		// [i] --> [j] --> []....
		t2[0] = s[j][0];
		t2[1] = s[j][1];

		//ROL64(t, s[j], keccakf_rotc[i]);

		s[j][0] = t1[0];
		s[j][1] = t1[1];

		t1[0] = t2[0];
		t1[1] = t2[1];
	}
}

/* Rho & Pi */
void rho_pi(uint32_t s[25][2]) {
	int i, j;
	uint32_t t1[2]; // lane 임시 저장용
	uint32_t t2[2]; // lane 임시 저장용

	t1[0] = s[1][0];
	t1[1] = s[1][1];

	for (i = 0; i < KECCAK_ROUND; i++) // 24번
	{
		j = keccakf_piln[i];

		t2[0] = s[j][0];
		t2[1] = s[j][1];

		ROL64(t1, s[j], keccakf_rotc[i]);

		t1[0] = t2[0];
		t1[1] = t2[1];
	}
}

/* Chi */
void chi(uint32_t s[25][2]) {
	int i, j;
	uint32_t bc[5][2]; // plane 임시 저장용
	for (j = 0; j < 25; j += 5) // y 좌표
	{
		for (i = 0; i < 5; i++) // x 좌표
		{
			bc[i][0] = s[j + i][0];  // S[5y+x][0]
			bc[i][1] = s[j + i][1];  // S[5y+x][1]
		}

		for (i = 0; i < 5; i++)
		{
			s[j + i][0] ^= (~bc[(i + 1) % 5][0]) & bc[(i + 2) % 5][0];
			s[j + i][1] ^= (~bc[(i + 1) % 5][1]) & bc[(i + 2) % 5][1];
		}
	}
}

/* Iota */
void iota(uint32_t s[25][2], int round) {
	s[0][0] ^= keccakf_rndc[round][0];
	s[0][1] ^= keccakf_rndc[round][1];
}
void test_state_conversion() {
	uint8_t state[200], recover[200];
	uint32_t s32[25][2];

	for (int i = 0; i < 200; i++) {
		state[i] = i;
	}
	state_to32(state, s32);
	state_to8(recover, s32);

	int idx;

	for (int i = 0; i < 200; i++) {
		if ((i % 8) == 0) {
			printf("\n%3d: ", i);
		}
		printf("%02x", state[i]);
	}
	printf("\n\n");

	for (int i = 0; i < 200; i++) {
		idx = 199 - i;
		if ((i % 8) == 0) {
			printf("\n%3d: ", idx);
		}
		else if ((i % 4) == 0) {
			printf(" ");
		}
		printf("%02x", state[idx]);
	}
	printf("\n\n");

	for (int i = 0; i < 25; i++) {
		printf("(s[%2d][1], s[%2d][0]) = (%08x, %08x)\n", i, i, s32[i][1], s32[i][0]);
	}

}

void test_rho_pi() {
	uint8_t state[200];
	uint32_t s1[25][2], s2[25][2];
	for (int i = 0; i < 200; i++) {
		state[i] = i;
	}
	state_to32(state, s1);
	state_to32(state, s2);

	rho(s1);
	pi(s1);
	rho_pi(s2);

	for (int i = 0; i < 25; i++) {
		printf("(s1[%2d][1], s1[%2d][0]) = (%08x, %08x) : ", i, i, s1[i][1], s1[i][0]);
		printf("(s2[%2d][1], s2[%2d][0]) = (%08x, %08x) \n", i, i, s2[i][1], s2[i][0]);
	}
}


void my_keccakf(uint8_t* state)
{
	uint32_t s[25][2] = { 0x00, };
	int i, round;

	//for (i = 0; i < 25; i++)
	//{
	//	s[i][0] = (uint32_t)(state[i * 8 + 0]) |
	//		(uint32_t)(state[i * 8 + 1] << 8) |
	//		(uint32_t)(state[i * 8 + 2] << 16) |
	//		(uint32_t)(state[i * 8 + 3] << 24);
	//	s[i][1] = (uint32_t)(state[i * 8 + 4]) |
	//		(uint32_t)(state[i * 8 + 5] << 8) |
	//		(uint32_t)(state[i * 8 + 6] << 16) |
	//		(uint32_t)(state[i * 8 + 7] << 24);
	//}

	state_to32(state, s);

	for (round = 0; round < KECCAK_ROUND; round++)
	{
		theta(s);
		rho(s);
		pi(s);
		chi(s);
		iota(s, round);
	}

	state_to8(state, s);
	//for (i = 0; i < 25; i++)
	//{
	//	state[i * 8 + 0] = (uint8_t)(s[i][0]);
	//	state[i * 8 + 1] = (uint8_t)(s[i][0] >> 8);
	//	state[i * 8 + 2] = (uint8_t)(s[i][0] >> 16);
	//	state[i * 8 + 3] = (uint8_t)(s[i][0] >> 24);
	//	state[i * 8 + 4] = (uint8_t)(s[i][1]);
	//	state[i * 8 + 5] = (uint8_t)(s[i][1] >> 8);
	//	state[i * 8 + 6] = (uint8_t)(s[i][1] >> 16);
	//	state[i * 8 + 7] = (uint8_t)(s[i][1] >> 24);
	//}
}

static uint8_t keccak_state[200] = { 0x00, };
static int end_offset;

int msg_input_keccak_absorb(uint8_t* input, int inLen, int rate, int capacity)
{

	uint8_t* buf = input;
	int iLen = inLen;
	int rateInBytes = rate / 8;
	int blockSize = 0;
	int i = 0;

	printf("[S] iLen = %3d, blockSize = %3d, end_offset= %3d\n", iLen, blockSize, end_offset);

	while (iLen > 0)
	{
		if ((end_offset != 0) && (end_offset < rateInBytes))
		{
			blockSize = (((iLen + end_offset) < rateInBytes) ? (iLen + end_offset) : rateInBytes);

			for (i = end_offset; i < blockSize; i++)
				keccak_state[i] ^= buf[i - end_offset];

			buf += blockSize - end_offset;
			iLen -= blockSize - end_offset;

			printf("[0] iLen = %3d, blockSize = %3d, end_offset= %3d\n", iLen, blockSize, end_offset);
		}
		else
		{
			blockSize = ((iLen < rateInBytes) ? iLen : rateInBytes);

			for (i = 0; i < blockSize; i++)
				keccak_state[i] ^= buf[i];

			buf += blockSize;
			iLen -= blockSize;

			printf("[1] iLen = %3d, blockSize = %3d, end_offset= %3d\n", iLen, blockSize, end_offset);
		}

		if (blockSize == rateInBytes)
		{
			keccakf(keccak_state);
			blockSize = 0;
			printf("call Keccak\n");
		}

		end_offset = blockSize;

		printf("[F] iLen = %3d, blockSize = %3d, end_offset= %3d\n", iLen, blockSize, end_offset);
	}

	return 0;
}

void test_msg_input_keccak_absorb() {
	uint8_t input[200];
	for (int i = 0; i < 200; i++) {
		input[i] = i;
	}
	msg_input_keccak_absorb(input, 200, 1088, 512);

	//for (int i = 0; i < 25; i++) {
	//	printf("(keccak_state[%2d][1], keccak_state[%2d][0]) = (%02x, %02x)\n", i, i, keccak_state[i * 8 + 4], keccak_state[i * 8 + 0]);
	//}


}

void test_keccakf() {
	uint8_t state1[200], state2[200];
	uint32_t s1[25][2], s2[25][2];
	for (int i = 0; i < 200; i++) {
		state1[i] = i;
		state2[i] = i;
	}

	keccakf(state1);
	my_keccakf(state2);


	state_to32(state1, s1);
	state_to32(state2, s2);

	for (int i = 0; i < 25; i++) {
		printf("(s1[%2d][1], s1[%2d][0]) = (%08x, %08x) :  ", i, i, s1[i][1], s1[i][0]);
		//printf("(s2[%2d][1], s2[%2d][0]) = (%08x, %08x) \n", i, i, s2[i][1], s2[i][0]);
		printf(" (%08x, %08x) \n", s2[i][1], s2[i][0]);
	}
}

int main()
{
	//ROL64_test();
	
	//Rho_offset_test();

	print_rho_constants();

	//print_rotation_by_stallings();

	kisa_main();

	//test_state_conversion();

	//test_rho_pi();

	//test_keccakf();

	//test_msg_input_keccak_absorb();

	return 0;
}
