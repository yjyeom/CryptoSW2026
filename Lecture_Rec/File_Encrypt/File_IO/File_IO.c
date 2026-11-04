#define _CRT_SECURE_NO_WARNINGS


#include <stdio.h>
#include <stdlib.h>  // malloc()
#include <errno.h>

typedef unsigned char byte;

// 파일크기 구하기
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

// 파일 복사
void file_copy(const char* src, const char* dst) {
    FILE* fsrc = fopen(src, "rb");
    if (!fsrc) { //오류이면
        printf("Source file error\n");
        return;
    }
    FILE* fdst = fopen(dst, "wb");
    if (!fdst) {
        printf("Destination file error\n");
        return;
    }

    byte buffer[1024];
    int bytesRead;

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), fsrc)) > 0) {
        printf("Read %d bytes\n", bytesRead);
        fwrite(buffer, 1, bytesRead, fdst);
    }
    fclose(fsrc);
    fclose(fdst);
}

//패딩함수 (80 00 00 ... 00)
void gen_data_padding(const byte* input, size_t input_bytes,
    byte** output, size_t* output_bytes) {
    // 패딩결과가 몇 블록인지 계산하기
    // 입력바이트(input_bytes): 15->1, 16->2, 17->2, ... , 32->3, ...
    int num_block = input_bytes / 16 + 1;
    int padded_data_bytes = num_block * 16; // 패딩결과(바이트)
    int padding_bytes = padded_data_bytes - input_bytes; // 패딩할 바이트 수
    *output = (byte *)malloc(padded_data_bytes);
    if (!*output) {
        printf("malloc error\n");
        *output_bytes = 0;
        return;
    }
    // 패딩결과 = (입력데이터) + (패딩)
    memcpy(*output, input, input_bytes);
    (*output)[input_bytes] = 0x80; // 1000 0000 : 패딩의 시작점
    //*(*output + input_bytes) = 0x80;
    memset(*output + input_bytes + 1, 0x00, padding_bytes - 1);
    *output_bytes = padded_data_bytes;
}


// unpadding 함수 ==> 각자 구현하기

void file_io_test()
{
    const char* file = "ECBMMT128.rsp";
    printf("file %s (%d bytes)\n", file, get_file_size(file));

    const char* dest_file = "ECBMMT128_copy.txt";
    file_copy(file, dest_file);
}

void padding_example() {
    const char* input1 = "0123456789abcdefg";
    byte input2[32];
    for (int i = 0; i < 20; i++) {
        input2[i] = 'a' + i;
    }
    byte* padded1 = NULL;
    byte* padded2 = NULL;
    size_t out_bytes1, out_bytes2;
    gen_data_padding(input1, 16, &padded1, &out_bytes1);
    gen_data_padding(input2, 20, &padded2, &out_bytes2);

    printf("padded1 = ");
    for (int i = 0; i < out_bytes1; i++) {
        printf("%02x ", padded1[i]);
    }
    printf("\n");

    printf("padded2 = ");
    for (int i = 0; i < out_bytes2; i++) {
        printf("%02x ", padded2[i]);
    }
    printf("\n");
}

int main()
{
    //file_io_test();

    padding_example();

}