#define _CRT_SECURE_NO_WARNINGS

#define MAX_LINE_LENGTH 2048
#define MAX_DATA_LENGTH 1024

#include <stdio.h>
#include <string.h>

#include "HexByte.h"
#include "AES32.h"


typedef struct {
    int count;
    char key[MAX_DATA_LENGTH];
    char iv[MAX_DATA_LENGTH];
    char plaintext[MAX_DATA_LENGTH];
    char ciphertext[MAX_DATA_LENGTH];
	int key_len;
	int iv_len;
	int plaintext_len;
} TestVector;

//사용법: read_test_vector("test-file.txt", &test_vec);
void read_test_vector(char *file, TestVector *tv) {
    FILE* fp = fopen(file, "r");
    if (fp == NULL) {
        perror("fopen");
        return 1;
    }
        
    char line[MAX_LINE_LENGTH];
    char name[MAX_DATA_LENGTH];
    char value[MAX_DATA_LENGTH];

    while (fgets(line, sizeof(line), fp)) {

        //printf("%s\n", line);
        if (sscanf(line, "%63[^=] = %2047s", name, value) == 2) {
            //printf("%s, %s, len = %d\n", name, value, strlen(value));
            if (strcmp(name, "COUNT ") == 0) {
                tv->count = atoi(value);  // atoi (ascii to integer)
                // 구조체 변수 tv_struct.count 
                // 구조체 포인터 tv_pointer->count
            }
            else if (strcmp(name, "KEY ") == 0) {
                strcpy(tv->key, value); // key[] 배열이름 key: key[0]의 주소를 가리키는 포인터
				tv->key_len = strlen(value) / 2; 
            }
            else if (strcmp(name, "IV ") == 0) {
                strcpy(tv->iv, value);
				tv->iv_len = strlen(value) / 2;
            }
            else if (strcmp(name, "PLAINTEXT ") == 0) {
                strcpy(tv->plaintext, value);
				tv->plaintext_len = strlen(value) / 2;
            }
            else if (strcmp(name, "CIPHERTEXT ") == 0) {
                strcpy(tv->ciphertext, value);
            }
        }
    }
    fclose(fp);
}

void read_test_vector_test() {
	TestVector tv;
    read_test_vector("test-vector-CBC.txt", & tv);

    printf("COUNT      = %d\n", tv.count);
    printf("KEY        = %s, (%d bytes)\n", tv.key, tv.key_len);
    printf("IV         = %s, (%d bytes)\n", tv.iv, tv.iv_len);
    printf("PLAINTEXT  = %s (%d bytes)\n", tv.plaintext, tv.plaintext_len);
    printf("CIPHERTEXT = %s\n", tv.ciphertext);
}

void read_test_vector_ctr_test() {
    TestVector tv;
    read_test_vector("test-vector-CTR.txt", & tv);

    printf("COUNT      = %d\n", tv.count);
    printf("KEY        = %s, (%d bytes)\n", tv.key, tv.key_len);
    printf("IV         = %s, (%d bytes)\n", tv.iv, tv.iv_len);
    printf("PLAINTEXT  = %s (%d bytes)\n", tv.plaintext, tv.plaintext_len);
    printf("CIPHERTEXT = %s\n", tv.ciphertext);
}

void CBC_TV_test() {
    TestVector tv;
    read_test_vector("test-vector-CBC.txt", &tv);
	byte calc_ct[MAX_DATA_LENGTH];

    int num_block; // 블록 수 (주의: MMT 에서는 패딩을 하지 않음)
    byte key[16], iv[16], pt[16], ct[16], prev_ct[16];
    u32 rk[11][4];

	Hex2Array(tv.key, tv.key_len * 2, key);
	Hex2Array(tv.iv, tv.iv_len * 2, iv);
    AES32_Enc_KeySchedule(key, rk); // 키 스케줄링
	num_block = tv.plaintext_len / 16; // 블록 수 계산
	copy_b_array(iv, 16, prev_ct); // 초기화 벡터를 이전 블록의 암호문으로 설정
    for (int i = 0; i < num_block; i++) {
		Hex2Array(tv.plaintext + i * 32, 32, pt);
		xor_b_array(pt, 16, prev_ct); // 이전 블록의 암호문과 XOR
		AES32_Encrypt(pt, rk, ct); // 암호화
		copy_b_array(ct, 16, prev_ct); // 현재 블록의 암호문을 이전 블록으로 설정
		copy_b_array(ct, 16, calc_ct + i * 16); // 계산된 암호문을 저장
    }
    
    printf("COUNT      = %d\n", tv.count);
    printf("KEY        = %s, (%d bytes)\n", tv.key, tv.key_len);
    printf("IV         = %s, (%d bytes)\n", tv.iv, tv.iv_len);
    printf("PLAINTEXT  = %s (%d bytes)\n", tv.plaintext, tv.plaintext_len);
    printf("CIPHERTEXT = %s\n", tv.ciphertext);
	print_b_array(calc_ct, tv.plaintext_len, "Calculated CIPHERTEXT");

}

//CTR 모드에서 사용하는 카운터 증가 함수 Inc_32()
// counter: (msb) c[0] c[1] ... c[14] c[15] (lsb)
void counter_inc(byte counter[16]) {
    for (int i = 15; i >= 0; i--) {
        if (counter[i] != 0xff) { // 자리올림이 발생하지 않음
            counter[i]++;
            break;
        }
        else {
            counter[i] = 0x00; // 0xff + 0x01 = 0x00 + (carry)
        }
    }
}

void CTR_TV_test() {
    TestVector tv;
    read_test_vector("test-vector-CTR.txt", &tv);
    byte calc_ct[MAX_DATA_LENGTH];

    int num_block; // 블록 수 (CTR 모드에서 완전히 채워지는 블록 수)
    byte key[16], iv[16], pt[16], ct[16], ctr[16];
    u32 rk[11][4];

    Hex2Array(tv.key, tv.key_len * 2, key);
    Hex2Array(tv.iv, tv.iv_len * 2, iv);
    AES32_Enc_KeySchedule(key, rk); // 키 스케줄링
    num_block = tv.plaintext_len / 16; // 블록 수 계산
    copy_b_array(iv, 16, ctr); // 초기화 벡터를 이전 블록의 암호문으로 설정
    for (int i = 0; i < num_block; i++) {
        AES32_Encrypt(ctr, rk, ct); // 카운터 암호화
        counter_inc(ctr); // 다음 카운터로
        Hex2Array(tv.plaintext + i * 32, 32, pt); 
        xor_b_array(ct, 16, pt); // 이전 블록의 암호문과 XOR        
        copy_b_array(ct, 16, calc_ct + i * 16); // 계산된 암호문을 저장
    }
	int remainder = tv.plaintext_len % 16; // 나머지 바이트 수
    if (remainder > 0) {
        AES32_Encrypt(ctr, rk, ct); // 카운터 암호화        
        Hex2Array(tv.plaintext + num_block * 32, remainder * 2, pt); 
        for (int i = 0; i < remainder; i++) {
            pt[i] ^= ct[i]; // XOR
            calc_ct[num_block * 16 + i] = pt[i]; // 계산된 암호문 저장
        }
	}
    
    printf("COUNT      = %d\n", tv.count);
    printf("KEY        = %s, (%d bytes)\n", tv.key, tv.key_len);
    printf("IV         = %s, (%d bytes)\n", tv.iv, tv.iv_len);
    printf("PLAINTEXT  = %s (%d bytes)\n", tv.plaintext, tv.plaintext_len);
    printf("CIPHERTEXT = %s\n", tv.ciphertext);
    print_b_array(calc_ct, tv.plaintext_len, "Calculated CIPHERTEXT");

}



int main()
{
    //hex_test();

    read_test_vector_test();

    //CBC_TV_test();


    read_test_vector_ctr_test();

    //CTR_TV_test();

	return 0;
}

