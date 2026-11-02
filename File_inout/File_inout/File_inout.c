// 파일을 읽어 바이트 배열에 저장하고,
// 바이트 배열을 파일로 저장하는 예제 코드입니다.

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>


typedef unsigned char byte;


// 추가할 함수: 파일을 열고 16바이트씩 읽어 아스키+헥사 형태로 출력합니다.
// 동작 계획(의사코드):
// 1. 함수 시그니처: void printFile16(const char* path)
// 2. path가 NULL이면 즉시 반환(또는 에러 메시지 출력).
// 3. fopen(path, "rb")로 파일을 연다. 실패하면 stderr에 에러 출력 후 반환.
// 4. 16바이트 버퍼(uint8_t buf[16])와 offset(size_t)을 초기화.
// 5. 반복:
//    a. fread(buf, 1, 16, f)로 읽은 바이트 수 n을 얻는다.
//    b. n == 0이면 반복 종료.
//    c. offset을 8자리(또는 08zx) 16진수로 출력(예: 00000010).
//    d. 16바이트를 순회하며 존재하는 바이트는 "%02x "로 출력, 없는 바이트는 "   "로 채운다.
//       8바이트 경계 후 추가 공백을 넣어 가독성 확보.
//    e. " |" 시작으로 읽은 바이트들을 printable이면 그대로, 아니면 '.'로 출력.
//    f. 줄 끝내기, offset += n.
// 6. fclose(f) 및 함수 종료.
// 예외와 에러는 stderr에 간단히 출력. 함수는 직접 출력만 수행하고 값을 반환하지 않음.


// 자동생성된 코드: 파일 내용을 출력하는 함수
// 파일을 열고 16바이트씩 읽어 아스키+헥사 형태로 출력합니다.
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


// 파일을 열고 첫 번째 블록(16바이트)만 읽어 헥사 형태로 출력합니다.
void print_one_block_in_file(const char* file_name)
{
    FILE* f = fopen(file_name, "rb");
    if (f == NULL) {
        fprintf(stderr, "File cannot be opened: %s\n", file_name);
        return;
    }

    byte buf[16];
    size_t n = fread(buf, 1, sizeof(buf), f);
    if (n == 0) {
        fprintf(stderr, "Empty File: %s\n", file_name);
        return;
    }
    for (size_t i = 0; i < n; ++i) {
        printf("%02x", buf[i]);
        if ((i % 4) == 3) printf(" ");
    }
    printf("\n");
    fclose(f);
}

// 파일을 복사하는 함수
void file_copy(const char* src, const char* dst) {
    FILE* fsrc = fopen(src, "rb");
    if (!fsrc) {
        fprintf(stderr, "Failed to open source file: %s\n", src);
        return;
    }
    FILE* fdst = fopen(dst, "wb");
    if (!fdst) {
        fprintf(stderr, "Failed to open destination file: %s\n", dst);
        fclose(fsrc);
        return;
    }
    byte buffer[4096];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), fsrc)) > 0) {
        fwrite(buffer, 1, bytesRead, fdst);
    }
    fclose(fsrc);
    fclose(fdst);
}

// 바이트를 헥사, int, char, binary 형태로 출력하는 함수
void print_byte(byte b) {
    printf("Hex: %02x\n", b);
    printf("int: %d\n", b);
    printf("char: %c\n", b);

    printf("Binary: ");
    for (int i = 7; i >= 0; i--) {
        putchar((b >> i) & 1 ? '1' : '0');
    }
    printf("\n");
}

// 파일 크기를 반환하는 함수
int get_file_size(const char* file_name) {
    FILE* f = fopen(file_name, "rb");
    if (!f) {
        fprintf(stderr, "Failed to open file: %s\n", file_name);
        return -1;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fclose(f);
    return (int)size;
}

//자동생성된 패딩: 블록배수인 경우 오류임
void data_block_padding(const byte* input, size_t input_len, byte** output, size_t* output_len) {
    size_t block_size = 16; // AES block size
    size_t padded_len = ((input_len + block_size - 1) / block_size) * block_size;
    *output = (byte*)malloc(padded_len);
    if (!*output) {
        fprintf(stderr, "Memory allocation failed\n");
        *output_len = 0;
        return;
    }
    memcpy(*output, input, input_len);
    (*output)[input_len] = 0x80; // Append '1' bit (0x80)
    memset(*output + input_len + 1, 0x00, padded_len - input_len - 1); // Append '0' bits
    *output_len = padded_len;
}

//자동생성된 언패딩: 오류확인 안됨
void data_block_unpadding(const byte* input, size_t input_len, byte** output, size_t* output_len) {
    if (input_len == 0 || input_len % 16 != 0) {
        fprintf(stderr, "Invalid padded data length\n");
        *output = NULL;
        *output_len = 0;
        return;
    }
    size_t unpadded_len = input_len;
    while (unpadded_len > 0 && input[unpadded_len - 1] == 0x00) {
        unpadded_len--;
    }
    if (unpadded_len > 0 && input[unpadded_len - 1] == 0x80) {
        unpadded_len--;
    }
    else {
        fprintf(stderr, "Invalid padding format\n");
        *output = NULL;
        *output_len = 0;
        return;
    }
    *output = (byte*)malloc(unpadded_len);
    if (!*output) {
        fprintf(stderr, "Memory allocation failed\n");
        *output_len = 0;
        return;
    }
    memcpy(*output, input, unpadded_len);
    *output_len = unpadded_len;
}


//자동생성된 패딩 예제 함수
void padding_example() {
    //const char* input_str = "Hello, World!";
    const char* input_str = "0123456789012345";
    size_t input_len = strlen(input_str);
    byte* padded_data = NULL;
    size_t padded_len = 0;
    data_block_padding((const byte*)input_str, input_len, &padded_data, &padded_len);
    printf("Original data: %s\n", input_str);
    printf("Padded data (hex): ");
    for (size_t i = 0; i < padded_len; ++i) {
        printf("%02x ", padded_data[i]);
    }
    printf("\n");
    free(padded_data);
}

//------------------------------
void gen_data_padding(const byte* input, size_t input_bytes, byte** output, size_t* output_bytes) {
    int block_size = 16; // AES block size
    //패딩된 결과가 몇 블록인지 계산
    //입력 바이트-> 출력블록 수:  15->1, 16->2, 17->2, ... , 32->3, ...
    int num_blocks = input_bytes / block_size + 1;
    int padded_data_bytes = num_blocks * block_size;
    //패딩으로 추가될 바이트 수: 1,2, ..., 16
    int padding_bytes = padded_data_bytes - input_bytes;

    *output = (byte*)malloc(padded_data_bytes);
    if (!*output) {
        fprintf(stderr, "Memory allocation failed\n");
        *output_bytes = 0;
        return;
    }
    memcpy(*output, input, input_bytes);
    (*output)[input_bytes] = 0x80; // Append '1' bit (0x80)
    memset(*output + input_bytes + 1, 0x00, padding_bytes - 1); // Append '0' bits
    *output_bytes = padded_data_bytes;
}

//gen_padding_wo_pointer
// 패딩된 데이터의 크기는 최대 16바이트 더 커질 수 있다.
// 함수를 호출하기전 배열의 크기를 충분히 확보하고 호출하는 버전이다.
// 패딩 후 데이터 크기는 함수결과로 반환한다.
int gen_data_padding_wo_pointer(byte input[], size_t input_bytes, byte output[]) {
    int block_size = 16; // AES block size
    //패딩된 결과가 몇 블록인지 계산
    //입력 바이트-> 출력블록 수:  15->1, 16->2, 17->2, ... , 32->3, ...
    int num_blocks = input_bytes / block_size + 1;
    int padded_data_bytes = num_blocks * block_size;
    //패딩으로 추가될 바이트 수: 1,2, ..., 16
    int padding_bytes = padded_data_bytes - input_bytes;

    for (int i = 0; i < input_bytes; i++) {
        output[i] = input[i];
    }
    output[input_bytes] = 0x80; // Append '1' bit (0x80)
    for (int i = input_bytes + 1; i < padded_data_bytes; i++)
    {
        output[i] = 0x00; // Append '0' bits
    }

    return padded_data_bytes;
}

//언패딩 함수
void unpadding_data(const byte* input, size_t input_bytes, byte** output, size_t* output_bytes) {
    if (input_bytes == 0 || input_bytes % 16 != 0) {
        fprintf(stderr, "Invalid padded data length\n");
        *output = NULL;
        *output_bytes = 0;
        return;
    }
    size_t unpadded_len = input_bytes;
    while (unpadded_len > 0 && input[unpadded_len - 1] == 0x00) {
        unpadded_len--;
    }
    if (unpadded_len > 0 && input[unpadded_len - 1] == 0x80) {
        unpadded_len--;
    }
    else {
        fprintf(stderr, "Invalid padding format\n");
        *output = NULL;
        *output_bytes = 0;
        return;
    }
    *output = (byte*)malloc(unpadded_len);
    if (!*output) {
        fprintf(stderr, "Memory allocation failed\n");
        *output_bytes = 0;
        return;
    }
    memcpy(*output, input, unpadded_len);
    *output_bytes = unpadded_len;
}


void my_padding_example() {
    const char* input_str = "Hello, World!";
    //const char* input_str = "0123456789012345";
    size_t input_len = strlen(input_str);
    byte* padded_data = NULL;
    size_t padded_len = 0;
    gen_data_padding((const byte*)input_str, input_len, &padded_data, &padded_len);
    printf("Original data: %s\n", input_str);
    printf("Padded data (hex): ");
    for (size_t i = 0; i < padded_len; ++i) {
        printf("%02x ", padded_data[i]);
    }
    printf("\n");
    free(padded_data);
}

void my_padding_example2() {
    const char* input_str = "Hello, World!";
    //const char* input_str = "0123456789012345";
    size_t input_len = strlen(input_str);

    size_t padded_len = 0;
    byte padded_data[16 * 5]; // 5블록이면 충분한 경우

    padded_len = gen_data_padding_wo_pointer(input_str, input_len, padded_data);

    printf("Original data: %s\n", input_str);
    printf("Padded data (hex): ");
    for (size_t i = 0; i < padded_len; ++i) {
        printf("%02x ", padded_data[i]);
    }
    printf("\n");
}

//파일으로부터 읽은 데이터를 패딩하는 예제 함수
void file_padding(const char* src, const char* dst) {
    FILE* fsrc = fopen(src, "rb");
    if (!fsrc) {
        fprintf(stderr, "Failed to open source file: %s\n", src);
        return;
    }
    FILE* fdst = fopen(dst, "wb");
    if (!fdst) {
        fprintf(stderr, "Failed to open destination file: %s\n", dst);
        fclose(fsrc);
        return;
    }
    byte buffer[16];
    size_t bytesRead;
    int flag_padded = 0; // 패딩이 이미 추가되었는지 확인하는 플래그

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), fsrc)) > 0) {
        if (bytesRead < 16) {
            buffer[bytesRead] = 0x80; // Append '1' bit (0x80)
            memset(buffer + bytesRead + 1, 0x00, 16 - bytesRead - 1); // Append '0' bits
            flag_padded = 1; // 패딩이 추가되었음을 표시
        }
        fwrite(buffer, 1, 16, fdst);
    }
    if (!flag_padded) {
        buffer[0] = 0x80; // Append '1' bit (0x80)
        memset(buffer + 1, 0x00, 15); // Append '0' bits
        fwrite(buffer, 1, 16, fdst);
    }
    fclose(fsrc);
    fclose(fdst);
}

void file_padding_example() {
    const char* in_file = "before_padding.txt";
    const char* out_file = "after_padding.txt";

    printf("Before padding:\n");
    print_file_contents(in_file);
    file_padding(in_file, out_file);
    printf("After padding:\n");
    print_file_contents(out_file);

    get_file_size(out_file);
    printf("File size of %s: %d bytes\n", out_file, get_file_size(out_file));

}


//------------------------------

// 파일 입출력 예제 함수
void file_handling_example() {
    const char* in_file = "input.txt";
    const char* out_file = "output.txt";

    byte* bytes = NULL;
    size_t bytesSize = 0;
    char* err = NULL;

    //print_block_file(in_file);
    print_one_block_in_file(in_file);

    print_byte(0x41); // Example byte

    file_copy(in_file, out_file);

    get_file_size(out_file);
    printf("File size of %s: %d bytes\n", out_file, get_file_size(out_file));

    free(bytes);
}

int main() {
    //file_handling_example();
    //my_padding_example();
    //my_padding_example2();

    file_padding_example();
    return 0;
}