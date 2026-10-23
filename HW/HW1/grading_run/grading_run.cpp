#include <stdio.h>

typedef unsigned char byte;

// for problem6_d
byte ExpTable[256];
int LogTable[256];

byte GF256_add(byte a, byte b)
{
	return a ^ b;
}

void GF256_print_bin(byte b)
{
	byte b_coeff[8]; //계수저장배열

	for (int i = 0; i < 8; i++)
	{
		b_coeff[i] = (b >> (7 - i)) & 0x01;
	}

	for (int i = 0; i < 8; i++)
	{
		printf("%d", b_coeff[i]);
	}
	printf("\n");
}

// 다항식으로 출력
void GF256_print_poly(byte b)
{
	byte b_coeff[8];

	for (int i = 0; i < 8; i++)
	{
		b_coeff[i] = (b >> (7 - i)) & 0x01;
	}

	int coef, expo;
	for (int i = 0; i < 8; i++)
	{
		coef = b_coeff[i];
		expo = 7 - i;
		if (coef == 1)
		{
			printf("+ x^%ld", expo);
		}
	}
	printf("\n");
}

byte GF256_xtime(byte a)
{
	int msb; // 최고차항의 계수
	byte result;
	msb = (a >> 7) & 0x01;
	if (msb == 0)
	{
		result = a << 1;
	}
	else
	{
		result = (a << 1) ^ 0x1b;
	}
	return result;
}

byte GF256_mul(byte a, byte b)
{
	byte h;
	int coef;

	h = 0x00;
	for (int i = 7; i >= 0; i--)
	{
		coef = (a >> i) & 0x01;
		h = GF256_xtime(h);
		if (coef == 1) // 계수가 1이면
		{
			h = GF256_add(h, b); // = h^b
		}
	}
	return h;
}


void GF256_element()
{
	byte a, b, c;
	a = 0x03;
	b = 0xa1;
	printf("a = ");
	GF256_print_bin(a);
	GF256_print_poly(a);
	printf("b = ");
	GF256_print_bin(b);
	GF256_print_poly(b);
	c = GF256_add(a, b);
	printf("c = ");
	GF256_print_bin(c);
	GF256_print_poly(c);
}


void GF256_mul_test()
{
	byte a, b, c, d;
	a = 0x03;
	b = 0xa1;
	printf("a = ");
	GF256_print_bin(a);
	GF256_print_poly(a);
	printf("b = ");
	GF256_print_bin(b);
	GF256_print_poly(b);

	c = GF256_xtime(a);
	d = GF256_xtime(b);
	printf("c = ");
	GF256_print_bin(c);
	GF256_print_poly(c);
	printf("d = ");
	GF256_print_bin(d);
	GF256_print_poly(d);
}


int GF256_ord(byte a)
{
	byte temp;
	int order;

	if (a == 0)
	{
		return -1; //의미 없는 결과
	}

	temp = a;
	order = 1;
	while (temp != 1)
	{
		temp = GF256_mul(temp, a);
		order++;
	}
	return order;
}


void order_test()
{
	int num_max_order;
	int order;
	num_max_order = 0;

	printf("GF(2^8)의 원소들의 위수");
	for (int i = 0; i < 256; i++)
	{
		order = GF256_ord(i);
		printf("ord(%3d) = %d\n", i, GF256_ord(i));
		if (order == 255) num_max_order++;
	}
	printf("최대 위수를 갖는 원소의 개수 = %d", num_max_order);
}


// 곱셈의 역원
byte GF256_inv(byte a)
{
	byte inverse;
	byte temp;

	if (a == 0)
	{
		return 0;
	}

	inverse = 1; //초기값
	temp = a;    //초기값
	for (int i = 0; i < 7; i++)
	{
		temp = GF256_mul(temp, temp);
		inverse = GF256_mul(inverse, temp);
	}
	return inverse;
}

void order_inverse_test()
{
	int num_max_order;
	int order;
	byte inv;
	num_max_order = 0;
	printf("GF(2^8)의 원소의 위수");
	for (int i = 0; i < 256; i++)
	{
		order = GF256_ord(i);
		inv = GF256_inv(i);
		printf("ord(%3d) = %d, %02x^(-1) = %02x, %02x * %02x = %02x\n", i, order, i, inv, i, inv, GF256_mul(i, inv));
	}
	printf("최대 위수를 갖는 원소의 개수 = %d\n", num_max_order);
}


byte AES_Affine(byte w)
{
	const byte A[8][8] = {
		{1,0,0,0,1,1,1,1},
		{1,1,0,0,0,1,1,1},
		{1,1,1,0,0,0,1,1},
		{1,1,1,1,0,0,0,1},
		{1,1,1,1,1,0,0,0},
		{0,1,1,1,1,1,0,0},
		{0,0,1,1,1,1,1,0},
		{0,0,0,1,1,1,1,1}
	};
	const byte b_vec[8] = { 1,1,0,0,0,1,1,0 };
	byte w_vec[8];
	byte y_vec[8];

	for (int i = 0; i < 8; i++) w_vec[i] = (w >> i) & 0x01;

	for (int i = 0; i < 8; i++)
	{
		y_vec[i] = b_vec[i];
		for (int j = 0; j < 8; j++)
		{
			y_vec[i] += A[i][j] * w_vec[j];
		}
	}

	byte y;
	y = 0;
	for (int i = 0; i < 8; i++)
	{
		y ^= (y_vec[i] << i);
	}
	return y;
}

//SBOX 테이블 만들기
void Get_AES_Sbox(byte S[256])
{
	byte w;
	for (int i = 0; i < 256; i++)
	{
		w = GF256_inv(i);
		S[i] = AES_Affine(w);
	}
}

//SBOX 만들기
void print_sbox()
{
	byte S[256];

	Get_AES_Sbox(S);

	printf("Sbox[256] = {\n");
	for (int i = 0; i < 256; i++)
	{
		if (i < 255)
		{
			printf("0x%02x, ", S[i]);
		}
		else
		{
			printf("0x%02x }; \n", S[i]);
		}
		if ((i % 16) == 15) printf("\n");
	}
}


void problem6_a()
{
	byte a = 0x03;
	byte value = 0x01;

	printf("===== Problem 6(a) =====\n");

	for (int k = 0; k <= 254; k++)
	{
		printf("a^%d = 0x%02x\n", k, value);

		value = GF256_mul(value, a);
	}
}


void problem6_c()
{
	byte ExpTable[256];
	int LogTable[256];

	byte a = 0x03;
	byte value = 0x01;

	// 초기화
	for (int i = 0; i < 256; i++)
	{
		ExpTable[i] = 0;
		LogTable[i] = -1;
	}

	// a^1부터 시작
	value = a;

	for (int k = 1; k <= 255; k++)
	{
		ExpTable[k] = value;
		LogTable[value] = k;

		value = GF256_mul(value, a);
	}


	printf("===== ExpTable =====\n");

	for (int k = 1; k <= 255; k++)
	{
		printf("ExpTable[%3d] = 0x%02x\n",
			k, ExpTable[k]);
	}


	printf("\n===== LogTable =====\n");

	for (int x = 1; x <= 255; x++)
	{
		printf("LogTable[0x%02x] = %d\n",
			x, LogTable[x]);
	}
}


void make_table()
{
	byte alpha = 0x03;
	byte value = alpha;

	for (int i = 0; i < 256; i++)
	{
		ExpTable[i] = 0;
		LogTable[i] = -1;
	}

	for (int k = 1; k <= 255; k++)
	{
		ExpTable[k] = value;
		LogTable[value] = k;

		value = GF256_mul(value, alpha);
	}
}

byte GF256_mul_table(byte a, byte b)
{
	// 둘 중 하나가 0이면 결과도 0
	if (a == 0 || b == 0)
	{
		return 0;
	}

	int exponent;

	exponent = (LogTable[a] + LogTable[b]) % 255;

	// ExpTable은 1~255를 사용 >> 0은 255로 바꾸기
	if (exponent == 0)
	{
		exponent = 255;
	}

	return ExpTable[exponent];
}


void problem6_d()
{
	make_table();

	// 테스트용
	byte a = 0xa1;
	byte b = 0x63;

	byte result = GF256_mul_table(a, b);

	printf("0x%02x * 0x%02x = 0x%02x\n",
		a, b, result);
}

int main()
{
	problem6_a();
	problem6_c();
	problem6_d();

	return 0;
}