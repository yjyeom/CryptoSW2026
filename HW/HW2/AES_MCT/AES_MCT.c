#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "HexByte.h"
#include "AES32.h"

/*
(Monte Carlo Test - ECB)
Key[0] = Key
PT[0] = PT
For i = 0 to 99
	Output Key[i]
	Output PT[0]

	For j = 0 to 999
		CT[j] = AES(Key[i], PT[j]) // 동일한 Key[i]로 1000번 암호화
		PT[j+1] = CT[j] // 또 다른 평문의 암호화를 위한

	Output CT[j] // 1000번째 암호문만 출력
	Key[i+1] = Key[i] xor CT[j] // 새로운 키로 업데이트
	PT[0] = CT[j] // 1000번 반복에 사용할 첫번째 평문

	KEY = 8d2e60365f17c7df1040d7501b4a7b5a
	PLAINTEXT = 59b5088e6dadc3ad5f27a460872d5929
*/

void MCT_ECB() {
	const char* hex_key = "8d2e60365f17c7df1040d7501b4a7b5a";
	const char* hex_pt = "59b5088e6dadc3ad5f27a460872d5929";

	byte key[16], pt[16], ct[16];
	u32 rk[11][4];

	Hex2Array(hex_key, 32, key);  // Key[0] = Key
	Hex2Array(hex_pt, 32, pt);// PT[0] = PT

	printf("MCT-ECB...(start)\n");

	//for (int i = 0; i <= 99; i++) { // For i = 0 to 99
	for (int i = 0; i < 3; i++) {  // 처음 3번만 출력하기
		print_b_array(key, 16, "KEY"); // Output Key[i]
		print_b_array(pt, 16, "PT"); // Output PT[0]

		AES32_Enc_KeySchedule(key, rk); //  키스케줄
		for (int j = 0; j <= 999; j++) { // For j = 0 to 999			
			AES32_Encrypt(pt, rk, ct); // CT[j] = AES(Key[i], PT[j]) 
			copy_b_array(ct, 16, pt); // PT[j + 1] = CT[j] 
		}
		print_b_array(ct, 16, "CT"); // Output CT[j]
		xor_b_array(key, 16, ct); // Key[i + 1] = Key[i] xor CT[j]
		copy_b_array(ct, 16, pt); // PT[0] = CT[j] 
		printf("\n");
	}
	printf("MCT-ECB...(end)\n");
}

/*
(Monte Carlo Test - CBC)

Key[0] = Key
IV[0] = IV
PT[0] = PT

For i = 0 to 99
	Output Key[i]
	Output IV[i]
	Output PT[0]

	For j = 0 to 999
		If ( j=0 )
			CT[j] = AES(Key[i], IV[i] xor PT[j])
			PT[j+1] = IV[i]
		Else
			CT[j] = AES(Key[i], PT[j] xor CT[j-1])
			PT[j+1] = CT[j-1]

	Output CT[j]

	Key[i+1] = Key[i] xor CT[j]
	IV[i+1] = CT[j]
	PT[0] = CT[j-1]

KEY = 9dc2c84a37850c11699818605f47958c
IV = 256953b2feab2a04ae0180d8335bbed6
PLAINTEXT = 2e586692e647f5028ec6fa47a55a2aab
*/
void MCT_CBC() {
	const char* hex_key = "9dc2c84a37850c11699818605f47958c";
	const char* hex_iv = "256953b2feab2a04ae0180d8335bbed6";
	const char* hex_pt = "2e586692e647f5028ec6fa47a55a2aab";

	byte key[16], pt[16], ct[16], prev_ct[16], iv[16];
	u32 rk[11][4];

	Hex2Array(hex_key, 32, key);  // Key[0] = Key
	Hex2Array(hex_iv, 32, iv); // IV[0] = IV
	Hex2Array(hex_pt, 32, pt);// PT[0] = PT

	printf("MCT-CBC...(start)\n");

	//for(int i=0; i<=99; i++) {	// For i = 0 to 99
	for (int i = 0; i < 3; i++) { // 처음 3번만 출력하기
		print_b_array(key, 16, "KEY"); // Output Key[i]
		print_b_array(iv, 16, "IV"); // Output IV[i]
		print_b_array(pt, 16, "PT"); // Output PT[0]

		AES32_Enc_KeySchedule(key, rk); // 키스케줄
		for (int j = 0; j <= 999; j++) { // For j = 0 to 999
			if (j == 0) { // If(j = 0)
				xor_b_array(pt, 16, iv);   // IV[i] xor PT[j]
				AES32_Encrypt(pt, rk, ct); // CT[j] = AES(Key[i], IV[i] xor PT[j])
				copy_b_array(iv, 16, pt);  // PT[j + 1] = IV[i]
			}
			else {
				copy_b_array(ct, 16, prev_ct); // CT[j-1]
				xor_b_array(pt, 16, prev_ct);  // PT[j] = PT[j] xor CT[j-1]
				AES32_Encrypt(pt, rk, ct); // CT[j] = AES(Key[i], PT[j] xor CT[j - 1])
				copy_b_array(prev_ct, 16, pt); // PT[j + 1] = CT[j - 1]
			}
		}
		print_b_array(ct, 16, "CT"); // Output CT[j]
		xor_b_array(key, 16, ct); // Key[i + 1] = Key[i] xor CT[j]
		copy_b_array(ct, 16, iv); // IV[i + 1] = CT[j]
		//있어도 상관없음 (중복으로 불필요) copy_b_array(prev_ct, 16, pt); // PT[0] = CT[j - 1]
		printf("\n");
	}
	printf("MCT-CBC...(end)\n");
}

int main()
{
	MCT_ECB();
	MCT_CBC();
}
