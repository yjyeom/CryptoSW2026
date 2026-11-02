#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// AES32 구현 헤더
#include "AES32.h"
#include "HexByte.h"

//===============================================
// 한 블록 패딩 함수
// 입력 in[] : 16바이트 미만 (0,1,2,...,15)
// 출력 out[16] : 16바이트 (한 블록)
// 패딩방법: (100...0 비트) = (80 00 00 ... 00 바이트)
void padding(byte in[], int in_length, byte out[16]) {
    byte pad_header = 0x80; // 1000 0000
    for (int i = 0; i < in_length; i++) { // 입력 길이 만큼
        out[i] = in[i];
    }
    out[in_length] = pad_header;
    for (int i = in_length + 1; i < 16; i++) {
        out[i] = 0x00;
    }
}
//===============================================

//===============================================
// 패딩된 한 블록에서 패딩을 제외한 바이트 수 구하기
// 입력: 패딩된 한 블록 (16바이트)
// 출력값: 0, 1, 2, ... , 15, (-1: 오류)
// 패딩: 80 00 00 00 ... 00
int pt_length(byte padded[16]) {
    int position80;
    position80 = 15;
    for (int i = 15; i > 0; i--) {
        if (padded[i] != 0x00) break;
        position80--;
    }
    if (padded[position80] != 0x80) {                
		printf("Padding error: 0x80 not found.\n");
        // (1) 0x00이 끝나는 지점에서 0x80이 없을 때,
        // (2) 0x00이 계속 나올 때
        return -1; // (1) or (2)
    }
    else {
        return position80; // 패딩을 제외한 바이트 수
    }
}


// bin 파일 출력하기 (file_inout.c 에서 복사함)
void print_file_contents(const char* path)
{
    FILE* f = fopen(path, "rb");
    //if (!f) {
    if (f == NULL) {
        fprintf(stderr, "File cannot be opened: %s\n", path);
        return;
    }

    byte buf[16];
    size_t offset = 0;
    while (1) {
        size_t n = fread(buf, 1, sizeof(buf), f);
        if (n == 0) break;

        // 오프셋 출력
        printf("%08zx  ", offset);

        // 헥사 출력 (16바이트 칸 유지)
        for (size_t i = 0; i < 16; ++i) {
            if (i < n) {
                printf("%02x ", buf[i]);
            }
            else {
                printf("   ");
            }
            if (i == 7) printf(" ");
        }

        // ASCII 표현
        printf(" |");
        for (size_t i = 0; i < n; ++i) {
            unsigned char c = buf[i];
            putchar((c >= 32 && c < 127) ? c : '.');
        }
        printf("|\n");

        offset += n;
    }

    fclose(f);
}


void AES_ECB_Enc_File(const char* filePT, byte key[16], const char* fileCT) {
    FILE* fsrc = fopen(filePT, "rb");
    if (!fsrc) {
        fprintf(stderr, "Failed to open source file: %s\n", filePT);
        return;
    }
    FILE* fdst = fopen(fileCT, "wb");
    if (!fdst) {
        fprintf(stderr, "Failed to open destination file: %s\n", fileCT);
        fclose(fsrc);
        return;
    }
    
    size_t bytesRead;
    int flag_padded = 0; // 패딩이 이미 추가되었는지 확인하는 플래그

    //AES32 키스케줄
    u32 rk[11][4]; // 라운드키
    AES32_Enc_KeySchedule(key, rk);
        
    byte buffer[16]; // 한블록 평문
    byte ct[16];     // 한블록 암호문

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), fsrc)) > 0) {
        if (bytesRead < 16) {
            buffer[bytesRead] = 0x80; // Append '1' bit (0x80)
            memset(buffer + bytesRead + 1, 0x00, 16 - bytesRead - 1); // Append '0' bits
            flag_padded = 1; // 패딩이 추가되었음을 표시
        }
        // AES32 암호화 수행
        AES32_Encrypt(buffer, rk, ct);
        fwrite(ct, 1, 16, fdst);
    }
    if (!flag_padded) {
        buffer[0] = 0x80; // Append '1' bit (0x80)
        memset(buffer + 1, 0x00, 15); // Append '0' bits
		// AES32 암호화 수행
        AES32_Encrypt(buffer, rk, ct);
        fwrite(ct, 1, 16, fdst);
    }
    fclose(fsrc);
    fclose(fdst);
}


void AES_ECB_Dec_File(const char* fileCT, byte key[16], const char* filePT) {
    FILE* fsrc = fopen(fileCT, "rb");
    if (!fsrc) {
        fprintf(stderr, "Failed to open source file: %s\n", fileCT);
        return;
    }
    FILE* fdst = fopen(filePT, "wb");
    if (!fdst) {
        fprintf(stderr, "Failed to open destination file: %s\n", filePT);
        fclose(fsrc);
        return;
    }

    //입력파일 크기가 16바이트 배수인지 확인
    size_t fileCT_size;
    fseek(fsrc, 0, SEEK_END);
	fileCT_size = ftell(fsrc);
    if (fileCT_size % 16 != 0) {
        fprintf(stderr, "Ciphertext file size is not a multiple of 16 bytes: %s\n", fileCT);
        fclose(fsrc);
        fclose(fdst);
        return;
	}
	fseek(fsrc, 0, SEEK_SET);

    //AES32 키스케줄
    u32 rk[11][4]; // 라운드키
    AES32_Dec_KeySchedule(key, rk);

    byte buffer[16]; // 한블록 평문
    byte pt[16];     // 한블록 암호문

	int total_blocks = fileCT_size / 16;
	for (int i = 0; i < total_blocks-1; i++) {    
        fread(buffer, 1, sizeof(buffer), fsrc);
        AES32_EqDecrypt(buffer, rk, pt);
        fwrite(pt, 1, 16, fdst);
    }
    //last block
    size_t bytesRead;
    bytesRead = fread(buffer, 1, sizeof(buffer), fsrc);
    if (bytesRead < 16) {
        fprintf(stderr, "Ciphertext file size is not a multiple of 16 bytes: %s\n", fileCT);
        fclose(fsrc);
        fclose(fdst);
        return;
    }
    AES32_EqDecrypt(buffer, rk, pt);
    //unpadding
    
    int unpadded_len = 16;
    /*
    while (unpadded_len > 0 && pt[unpadded_len - 1] == 0x00) {
        unpadded_len--;
    }
    if (unpadded_len > 0 && pt[unpadded_len - 1] == 0x80) {
        unpadded_len--;
    }
    else {
        fprintf(stderr, "Invalid padding format\n");         
        return;
    }
    */
	unpadded_len = pt_length(pt);
    fwrite(pt, 1, unpadded_len, fdst);

    fclose(fsrc);
    fclose(fdst);
}

//===================================
// ECB 파일 암호화
void File_ECB_test() {
    const char* pPT = "PT-ECB.bin";
    const char* pCT = "CT-ECB.bin";
    const char* pDecPT = "DecPT-ECB.bin";

    const char* hex_key = "7723d87d773a8bbfe1ae5b081235b566";

    byte key[16];
    Hex2Array(hex_key, 32, key);

	printf("AES ECB Encrypt...\n");    
    AES_ECB_Enc_File(pPT, key, pCT);

    printf("AES ECB Decrypt...\n");
    AES_ECB_Dec_File(pCT, key, pDecPT);

	printf("PT\n");
	print_file_contents(pPT);
	printf("CT\n");
    print_file_contents(pCT);
	printf("DecPT\n");
	print_file_contents(pDecPT);

}

void main() {

    //hex_test();
    File_ECB_test();
    
}