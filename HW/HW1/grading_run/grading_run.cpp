#include <stdio.h>

//데이터 형: 바이트
//예: 0x0a = 0000 1010 = x^3 + x
typedef unsigned char byte; //0~255

//유한체 GF(2^8)의 덧셈
byte GF256_add(byte a, byte b)
{
    return a ^ b; // ^ : xor 연산자
}

//다항식 꼴에 x 곱하기
byte GF256_xtime(byte a)
{
    int msb; //최고차항 계수
    byte result; //결과값 저장

    msb = (a >> 7) & 0x01;
    if (msb == 0) {
        result = a << 1;
    }
    else {
        result = (a << 1) ^ 0x1b;//덧셈은 xor
    }
    return result;
}

//GF(2^8)에서의 곱셉
byte GF256_mul(byte a, byte b)
{
    byte h; //결과를 저장할 변수
    int coef; //계수 저장

    h = 0x00;
    for (int i = 7; i >= 0; i--) {
        coef = (a >> i) & 0x01;
        h = GF256_xtime(h);
        if (coef == 1) {
            h = GF256_add(h, b);
        }
    }
    return h;
}

//a(0이 아닌 원소)의 위수
int GF256_ord(byte a)
{
    byte temp;
    int order;

    if (a == 0) {
        return -1;
    }
    temp = a;
    order = 1;
    while (temp != 1) {
        temp = GF256_mul(temp, a);
        order++;
    }
    return order;
}

//6(c)의 테이블을 저장하는 자료형.
typedef struct
{
    byte generator;
    byte ExpTable[256]; // 유효 인덱스: 1~255
    int LogTable[256];  // 유효 입력: 1~255, 0의 로그는 -1
} GF256_Tables;

//6(a)의 alpha^0~alpha^254 표 생성
//GF256_ord로 생성자인지 확인하고 GF256_mul로 거듭제곱을 계산
static int Build_Power_Table(byte alpha, byte powers[255])
{
    if (GF256_ord(alpha) != 255)
    {
        fprintf(stderr, "Build_Power_Table: alpha is not a generator.\n");
        return 0;
    }
    powers[0] = 1;
    for (int k = 1; k <= 254; k++)
    {
        powers[k] = GF256_mul(powers[k - 1], alpha);
    }
    return 1;
}

//6(a)의 표에서 x = alpha^k를 만족하는 k를 찾기
static int Find_Power_Index(byte x, const byte powers[255])
{
    for (int k = 0; k <= 254; k++)
    {
        if (powers[k] == x) return k;
    }
    return -1;
}

//바이트 값의 표를 한 번에 최대 16개 열씩 모두 출력
static void Print_Byte_Table(const byte values[], int first, int last)
{
    for (int start = first; start <= last; start += 16)
    {
        int end = start + 15;
        if (end > last) end = last;
        printf("k   :");
        for (int k = start; k <= end; k++) printf("%5d", k);
        printf("\nval :");
        for (int k = start; k <= end; k++)
            printf(" 0x%02x", (unsigned)values[k]);
        printf("\n\n");
    }
}

//6(c)의 ExpTable / LogTable을 생성
//alpha의 위수가 255인지 검사한 뒤, k=1~255의 값을 계산
//각 0이 아닌 원소가 정확히 한 번씩 등장하는지 확인
static int Build_GF256_Tables(byte alpha, GF256_Tables* tables)
{
    byte value = 1;
    if (GF256_ord(alpha) != 255)
    {
        fprintf(stderr, "Build_GF256_Tables: alpha is not a generator.\n");
        return 0;
    }
    tables->generator = alpha;
    for (int i = 0; i < 256; i++)
    {
        tables->ExpTable[i] = 0;
        tables->LogTable[i] = -1;
    }
    for (int k = 1; k <= 255; k++)
    {
        value = GF256_mul(value, alpha);
        if (value == 0 || tables->LogTable[value] != -1)
        {
            fprintf(stderr, "Build_GF256_Tables: repeated or zero element.\n");
            return 0;
        }
        tables->ExpTable[k] = value;
        tables->LogTable[value] = k;
    }
    return value == 1;
}

//6(c)의 LogTable을 출력
static void Print_Log_Table(const GF256_Tables* tables)
{
    for (int start = 0; start < 256; start += 16)
    {
        printf("x   :");
        for (int x = start; x < start + 16; x++)
            printf(" 0x%02x", (unsigned)x);
        printf("\nlog :");
        for (int x = start; x < start + 16; x++)
        {
            if (tables->LogTable[x] == -1) printf("    -");
            else printf("%5d", tables->LogTable[x]);
        }
        printf("\n\n");
    }
}

//ExpTable, LogTable 이용한 GF(2^8) 곱셈
static byte GF256_Mul_Tables(byte x, byte y, const GF256_Tables* tables)
{
    int exponent;
    if (x == 0 || y == 0) return 0;
    exponent = (tables->LogTable[x] + tables->LogTable[y]) % 255;
    if (exponent == 0) exponent = 255;
    return tables->ExpTable[exponent];
}

//GF_mul의 계산 결과와 같은지 체크
static int Verify_Table_Multiplication(const GF256_Tables* tables)
{
    for (int x = 0; x < 256; x++)
    {
        for (int y = 0; y < 256; y++)
        {
            byte actual = GF256_Mul_Tables((byte)x, (byte)y, tables);
            byte expected = GF256_mul((byte)x, (byte)y);
            if (actual != expected)
            {
                fprintf(stderr, "Mismatch: 0x%02x * 0x%02x\n",
                    (unsigned)x, (unsigned)y);
                return 0;
            }
        }
    }
    return 1;
}

//결과 출력
int main(void)
{
    const byte alpha = 0x03;
    const byte x = 0xa1;
    const byte y = 0x63;
    // (c)의 생성자 선택: 0x03을 사용하며 다른 생성자로 바꿀 수 있다.
    const byte chosen_generator = 0x03;
    byte powers[255];
    GF256_Tables tables;
    int kx, ky, exponent;
    byte product;

    printf("6(a) alpha = 0x%02x, order = %d\n",
        (unsigned)alpha, GF256_ord(alpha));
    if (!Build_Power_Table(alpha, powers)) return 1;
    printf("All powers alpha^0 through alpha^254:\n\n");
    Print_Byte_Table(powers, 0, 254);

    printf("6(b) Multiply using the table in 6(a):\n");
    kx = Find_Power_Index(x, powers);
    ky = Find_Power_Index(y, powers);
    if (kx == -1 || ky == -1)
    {
        fprintf(stderr, "6(b): could not find an exponent.\n");
        return 1;
    }
    exponent = (kx + ky) % 255;
    product = powers[exponent];
    printf("0x%02x = alpha^%d, 0x%02x = alpha^%d\n",
        (unsigned)x, kx, (unsigned)y, ky);
    printf("(%d + %d) mod 255 = %d\n", kx, ky, exponent);
    printf("0x%02x * 0x%02x = alpha^%d = 0x%02x\n\n",
        (unsigned)x, (unsigned)y, exponent, (unsigned)product);

    printf("6(c) Chosen generator = 0x%02x\n", (unsigned)chosen_generator);
    if (!Build_GF256_Tables(chosen_generator, &tables)) return 1;
    printf("ExpTable(k), k = 1..255 (index 0 is unused):\n\n");
    Print_Byte_Table(tables.ExpTable, 1, 255);
    printf("LogTable(x): '-' means that log(0) is undefined.\n\n");
    Print_Log_Table(&tables);
    printf("ExpTable(255) = 0x%02x; LogTable(0x01) = %d\n\n",
        (unsigned)tables.ExpTable[255], tables.LogTable[1]);

    printf("6(d) GF256_Mul_Tables:\n");
    printf("0x%02x * 0x%02x = 0x%02x\n", (unsigned)x, (unsigned)y,
        (unsigned)GF256_Mul_Tables(x, y, &tables));
    printf("0x01 * 0x01 = 0x%02x\n",
        (unsigned)GF256_Mul_Tables(1, 1, &tables));
    printf("0x00 * 0x%02x = 0x%02x\n", (unsigned)x,
        (unsigned)GF256_Mul_Tables(0, x, &tables));
    if (!Verify_Table_Multiplication(&tables)) return 1;
    printf("PASS: all 65536 products match the original GF256_mul.\n");
    return 0;
}