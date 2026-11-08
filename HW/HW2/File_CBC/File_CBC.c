#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AES32.h"

typedef unsigned char byte;

// 파일 내용을 출력하기
void print_file_contents(const char* path) {
    FILE* f = fopen(path, "rb");
    //if (!f) {
    if (f == NULL) {
        fprintf(stderr, "File cannot be opened: %s\n", path);
        return;
    }

    byte buf[16];
    size_t offset = 0;
    while (1) { // while (TRUE)
        size_t n = fread(buf, 1, sizeof(buf), f);
        if (n == 0) break; //while 종료하기

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

//===============================================
// 파일크기 구하기 (수업시간에 만든 파일)
int get_file_size(const char* file_name) {
    FILE* f;
    errno_t errcode = fopen_s(&f, file_name, "rb");
    if (errcode != 0) {
        printf("File open error (code = %d)\n", errcode);
        return errcode; //오류 코드 리턴
    }

    fseek(f, 0, SEEK_END); // 파일 포인터를 끝으로
    int file_size = ftell(f); // 현재 파일 포인터의 위치
    fclose(f);

    return file_size;
}

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

// CBC로 파일 복호화 하기 
void AES_CBC_Dec_File(const char* fileCT, byte key[16], byte IV[16], const char* filePT) {

    int ctfile_size = get_file_size(fileCT);
    if ((ctfile_size % 16) != 0) {
        printf("Ciphertext file size error (Not a multiple of 16)\n");
        return;
    }
    int num_ct_block = ctfile_size / 16; //암호화된 블록 수

    FILE* fsrc = fopen(fileCT, "rb");
    if (!fsrc) {
        printf("Source file error,\n");
        return;
    }
    FILE* fdst = fopen(filePT, "wb");
    if (!fdst) {
        printf("Destination file error.\n");
        fclose(fsrc);
        return;
    }

    u32 rk[11][4]; // 라운드 키
    AES32_Dec_KeySchedule(key, rk);

    byte buffer[16]; //파일에서 읽는 한블록용 버퍼(->평문)
    byte prev_ct[16];     //이전 암호문 또는 IV
    byte pt[16];

    for (int i = 0; i < 16; i++) {
        prev_ct[i] = IV[i]; // 초기화 벡터 설정
    }

    for (int i = 0; i < num_ct_block - 1; i++) {  //마지막 블록을 제외하고 복호화
        fread(buffer, 1, sizeof(buffer), fsrc);
        AES32_EqDecrypt(buffer, rk, pt); // AES 복호화
        for (int j = 0; j < 16; j++) {
            pt[j] ^= prev_ct[j]; // 이전 암호문과 XOR
            prev_ct[j] = buffer[j]; // 현재 암호문을 이전 암호문으로 저장
        }
        fwrite(pt, 1, sizeof(buffer), fdst);
    }
    //마지막 블록 처리	
    fread(buffer, 1, sizeof(buffer), fsrc);
    AES32_EqDecrypt(buffer, rk, pt); // AES 복호화
    for (int j = 0; j < 16; j++) {
        pt[j] ^= prev_ct[j]; // 이전 암호문과 XOR
    }
    int last_pt_len;
    last_pt_len = pt_length(pt); //패딩을 제외한 평문의 길이(0~15)를 계산
    fwrite(pt, 1, last_pt_len, fdst);

    fclose(fsrc);
    fclose(fdst);
}

// CBC로 파일 암호화 하기 
void AES_CBC_Enc_File(const char* filePT, byte key[16], byte IV[16], const char* fileCT) {
    FILE* fsrc = fopen(filePT, "rb");
    if (!fsrc) {
        printf("Source file error,\n");
        return;
    }
    FILE* fdst = fopen(fileCT, "wb");
    if (!fdst) {
        printf("Destination file error.\n");
        fclose(fsrc);
        return;
    }

    int byteRead;  //실제로 읽은 바이트 수
    int flag_padded = 0; // 패딩을 아직 하지 않음(0)

    u32 rk[11][4]; // 라운드 키    
	AES32_Enc_KeySchedule(key, rk);

	byte buffer[16]; //파일에서 읽는 한블록용 버퍼(->평문)
    byte prev_ct[16];     //이전 암호문 또는 IV
	byte ct[16];    //한블록 암호문

    for (int i = 0; i < 16; i++) {
        prev_ct[i] = IV[i]; // 초기화 벡터 설정
    }

    while ((byteRead = fread(buffer, 1, sizeof(buffer), fsrc)) > 0) {
        if (byteRead < 16) { // 패딩필요
            // buffer: [0], [1], ... , [byteRead-1], [byteRead]80, ... [15]00 
            buffer[byteRead] = 0x80;
            // buffer[byteRead+1] 부터 buffer[15]까지 0x00으로 채운다
            memset(buffer + byteRead + 1, 0x00, 16 - byteRead - 1);
            flag_padded = 1;//패딩했음
        }		
        for (int i = 0; i < 16; i++) {
            buffer[i] ^= prev_ct[i]; // 이전 암호문과 XOR
        }
        AES32_Encrypt(buffer, rk, ct); // AES 암호화
        for (int i = 0; i < 16; i++) {
            prev_ct[i] = ct[i]; // 현재 암호문을 이전 암호문으로 저장
        }
        fwrite(ct, 1, sizeof(buffer), fdst);
    }
    //패딩을 아직 하지 않았으면...
    if (!flag_padded) {  //패딩 한블록을 추가 [80 00 ... 00] 16바이트
        buffer[0] = 0x80;
        memset(buffer + 1, 0x00, 15);
        for (int i = 0; i < 16; i++) {
            buffer[i] ^= prev_ct[i]; // 이전 암호문과 XOR
        }
        AES32_Encrypt(buffer, rk, ct);
        fwrite(ct, 1, sizeof(buffer), fdst);
    }
    fclose(fsrc);
    fclose(fdst);
}

void File_CBC_test() {
    const char* pt_file = "PT-CBC.bin";
    const char* ct_file = "CT-CBC.bin";

    byte key[16];
    byte iv[16];

    for (int i = 0; i < 16; i++) {
        key[i] = (byte)i;
        iv[i] = (byte)(15 - i);
    }

    printf("Plaintext File: %s\n", pt_file);
    print_file_contents(pt_file);
    AES_CBC_Enc_File(pt_file, key, iv, ct_file);
    printf("Ciphertext File: %s\n", ct_file);
    print_file_contents(ct_file);


    const char* dec_pt_file = "PT-CBC-decrypt.bin";
	AES_CBC_Dec_File(ct_file, key, iv, dec_pt_file);
    printf("Decrypted File: %s\n", dec_pt_file);
	print_file_contents(dec_pt_file);

}

int main() {
    //print_file_contents_test();
    File_CBC_test();
}