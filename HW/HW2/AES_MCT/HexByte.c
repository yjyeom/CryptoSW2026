/*=================
  16진 문자열을 다루는 함수들
=================*/
#include "HexByte.h"

// 주어진 문자가 16진수 숫자로 유효한가?
// 0,1,2, ... , 9, A~F, a~f
int isHex(char ch) {
	return ((ch >= '0') && (ch <= '9')) ||  // 48~57 사이인가? (십진수로)
		((ch >= 'A') && (ch <= 'F')) || // 65~70 사이인가? (십진수로)
		((ch >= 'a') && (ch <= 'f'));  // 97~102 사이인가? (십진수로)
}

// 문자로 받은 1개의 16진수 ---> 10진수로 
// (char) ---> (byte) 0~15 (아니면 오류)
// 'a' ---> 10,  '0' ---> 0,  'f' ---> 15
byte Hex2Digit(char ch) {
	if (!isHex(ch)) {
		printf(" is not a hex value.\n");
		return -1; //의미없는 출력
	}
	if ((ch >= '0') && (ch <= '9')) {
		return  ch - '0'; // 예: '4'-'0' = 52 - 48 = 4
	}
	else if ((ch >= 'A') && (ch <= 'F')) {
		return ch - 'A' + 10;  // 예: 'B' - 'A' + 10 = 66 - 65 + 10 = 11
	}
	else if ((ch >= 'a') && (ch <= 'f')) {
		return ch - 'a' + 10;
	}
	// 도달하지 않는 코드 (혹시나...)
	printf("Unknown error.\n");
	return -1;
}

// 2자리 16진수 ---> 1바이트
// h[] = { h[0], h[1] } = { 'a', '1' } ---> 1010 0001 = A1 = 161
byte Hex2Byte(const char h[2]) {
	byte upper, lower;
	upper = h[0]; // 상위 4비트(nibble)를 나타낼 1문자
	lower = h[1]; // 하위 4비트(nibble)를 나타낼 1문자
	if ((!isHex(upper)) || (!isHex(lower))) {
		printf("Hex Error\n");
		return -1;
	}
	return Hex2Digit(upper) * 16 + Hex2Digit(lower);
	// h[] = { 'a', '1' } --> 10*16 + 01 = 161 = A1
}

// 16진 문자열 ---> 바이트 배열
// 16진 문자열 hex_str[] = "8d2e60365f17c7df1040d7501b4a7b5a"  (32개 16진수)
// 문자열 길이 hex_len = 32 (출력 바이트 배열의 길이 * 2)
// 출력 바이트 배열 barr[] = { '8d', '2e', ... , '5a' }
void Hex2Array(const char hex_str[], int hex_len, byte barr[]) {
	char h[2]; // h[] = { h[0], h[1] } ---> 1바이트 b_value
	byte b_value;
	for (int i = 0; i < hex_len / 2; i++) { // barr[] 크기 = hex_len/2
		h[0] = hex_str[2 * i]; // 상위 nibble
		h[1] = hex_str[2 * i + 1]; // 하위 nibble
		// h[] = { hex_str[0], hex_str[1] },
		// h[] = { hex_str[2], hex_str[3] }, ...
		b_value = Hex2Byte(h); // 2자리 16진수 ---> 1바이트
		barr[i] = b_value;
	}
}

// 출력함수
// const char* pStr = nullptr (default: nullptr)
// void print_b_array(byte b_arr[], int len, const char* pStr = nullptr);
void print_b_array(byte b_arr[], int len, const char* pStr) {
	if (pStr != "") {
		printf("%s = ", pStr);
	}
	for (int i = 0; i < len; i++) {
		printf("%02x ", b_arr[i]);
	}
	printf("\n");
}

// 바이트 배열 복사하기
void copy_b_array(byte src[], int len, byte dest[]) {
	for (int i = 0; i < len; i++) {
		dest[i] = src[i];
	}
}

// data[] ---> data[] ^ xor_arr[] 
// 주의! data[] 배열의 업데이트  ( X xor Y ---> X 업데이트)
void xor_b_array(byte data[], int len, byte xor_arr[]) {
	for (int i = 0; i < len; i++) {
		data[i] ^= xor_arr[i];
	}
}

void hex_test() {
	printf("isHex('a') = %d\n", isHex('a'));
	printf("isHex('x') = %d\n", isHex('x'));
	printf("Hex2Digit('a') = %d\n", (int)Hex2Digit('a'));

	char H[2] = { 'f', 'f' }; // 0xff
	printf("Hex2Byte(H) = %d\n", (int)Hex2Byte(H));
	const char* HexString = "8d2e60365f17c7df1040d7501b4a7b5a";
	byte HexArray[16];
	Hex2Array(HexString, 32, HexArray);
	printf("%s = ", HexString);
	print_b_array(HexArray, 16, "");
	print_b_array(HexArray, 16, "Hex2Array");
}