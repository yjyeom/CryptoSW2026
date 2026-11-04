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

void print_file_contents_test() {
    print_file_contents("NIST-AES-testvector.txt");
    printf("\n");
    print_file_contents("binary.bin");
}

void AES_ECB_Enc_File(const char* filePT, byte key[16], const char* fileCT) {
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

    byte buffer[16]; //한블록용 버퍼(평문)
    byte ct[16];    //한블록 암호문

    while ((byteRead = fread(buffer, 1, sizeof(buffer), fsrc)) > 0) {
        if (byteRead < 16) { // 패딩필요
            // buffer: [0], [1], ... , [byteRead-1], [byteRead]80, ... [15]00 
            buffer[byteRead] = 0x80;
            // buffer[byteRead+1] 부터 buffer[15]까지 0x00으로 채운다
            memset(buffer + byteRead + 1, 0x00, 16 - byteRead - 1);
            flag_padded = 1;//패딩했음
        }
        AES32_Encrypt(buffer, rk, ct);
        fwrite(ct, 1, sizeof(buffer), fdst);
    }
    //패딩을 아직 하지 않았으면...
    if (!flag_padded) {  //패딩 한블록을 추가 [80 00 ... 00] 16바이트
        buffer[0] = 0x80;
        memset(buffer + 1, 0x00, 15);
        AES32_Encrypt(buffer, rk, ct);
        fwrite(ct, 1, sizeof(buffer), fdst);
    }
    fclose(fsrc);
    fclose(fdst);
}

//void AES_ECB_Dec_File(...) ==> 각자 해보자

void File_ECB_test() {
    const char* pt_file = "PT-ECB.bin";
    const char* ct_file = "CT-ECB.bin";
    
    byte key[16];
    for (int i = 0; i < 16; i++) key[i] = (byte)i;
    printf("Plaintext File: %s\n", pt_file);
    print_file_contents(pt_file);
    AES_ECB_Enc_File(pt_file, key, ct_file);
    printf("Ciphertext File: %s\n", ct_file);
    print_file_contents(ct_file);
}

int main() {
    //print_file_contents_test();
    File_ECB_test();
}