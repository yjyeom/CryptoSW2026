#include "advancedencryption.h"

void print_Matrix(Matrix A) //행렬 출력하기
{
	for (int i = 0; i < A.row; i++)
	{
		printf("[");
		for (int j = 0; j < A.col; j++)
		{
			printf("%7.2f", A.M[i][j]);
		}
		printf("]\n");
	}
	printf("\n");
}

void print_Matrix_GF2(Matrix A) //행렬 출력하기
{
	for (int i = 0; i < A.row; i++)
	{
		printf("[");
		for (int j = 0; j < A.col; j++)
		{
			printf("%2d", (int)A.M[i][j]);
		}
		printf("]\n");
	}
	printf("\n");
}

//void print_test()
//{
//	Matrix A;
//	A.col = 3;
//	A.row = 3;
//	for (int i = 0; i < 3; i++)
//	{
//		for (int j = 0; j < 3; j++)
//		{
//			A.M[i][j] = 1;
//		}
//	}
//	print_Matrix(A);
//}

Matrix Mat_formalization() // 행렬 초기화([0]) 함수
{
	Matrix A;
	A.row = 1; A.col = 1;
	A.M[0][0] = 0.0;
	return A;
}

Matrix Mat_addition_R(Matrix A, Matrix B) // 행렬덧셈 함수
{
	Matrix C;
	C = Mat_formalization();

	if ((A.col != B.col) || (A.row != B.row))
	{
		printf("(Mat_addition_R)Error!!! : Matrix_size_error");
		return C;
	}

	C.col = A.col; C.row = A.row; //표기값 바꿔주기

	for (int i = 0; i < A.row; i++)
	{
		for (int j = 0; j < A.col; j++)
		{
			C.M[i][j] = A.M[i][j] + B.M[i][j]; //각각 더한 값 넣어주기
		}
	}

	return C;
}

Matrix Mat_multiplication_R(Matrix A, Matrix B) //행렬곱 함수
{
	Matrix C;
	C = Mat_formalization();

	if (A.col != B.row)
	{
		printf("(Mat_multiplication_R)Error!!! : matrix_size_error");
		return C;
	}

	C.row = A.row; C.col = B.col; //C의 크기 명시
	for (int i = 0; i < C.row; i++) //C의 앞자리 인덱스 C.M[i][j]
	{
		for (int j = 0; j < C.col; j++)
		{
			for (int k = 0; k < A.col; k++) //A.col == B.row임.. 걍 둘중하나 택1
			{
				C.M[i][j] += A.M[i][k] * B.M[k][j]; //이런 식으로 움직인다..
			}
		}
	}
	return C;
}

void Mat_exchange_rows(Matrix* A, int row1, int row2) //열 교환 함수
{
	double temp;
	for (int j = 0; j < A->col; j++)
	{
		temp = A->M[row2][j];
		A->M[row2][j] = A->M[row1][j];
		A->M[row1][j] = temp;
	}
}

void Mat_scalar_row_mul(Matrix* A, double k, int row) //열 스칼라곱 함수
{
	for (int i = 0; i < A->col; i++)
	{
		A->M[row][i] *= k;
	}
}

void Mat_row_add(Matrix* A, double k, int src_row, int target_row) // 열간 덧셈연산 함수
{
	for (int i = 0; i < A->col; i++)
	{
		A->M[target_row][i] += A->M[src_row][i] * k;
	}
}

void Mat_row_add_GF2(Matrix* A, int src_row, int target_row) // 열간 덧셈연산 함수 in GF2
{
	for (int i = 0; i < A->col; i++)
	{
		A->M[target_row][i] = (int)A->M[src_row][i] ^ (int)A->M[target_row][i];
	}
}

Matrix Mat_upper_rotate_8x8(Matrix A)
{
	Matrix Ap = Mat_formalization();
	Ap.col = A.col;
	Ap.row = A.row;
	int row[8] = { 0 };

	for (int i = 0; i < A.col; i++)
	{
		row[i] = A.M[0][i];
	}
	for (int i = 0; i < (Ap.row - 1); i++)
	{
		for (int j = 0; j < Ap.col; j++)
		{
			Ap.M[i][j] = A.M[i+1][j];
		}
	}
	for (int i = 0; i < Ap.col; i++)
	{
		Ap.M[Ap.row][i] = row[i];
	}
	return Ap;
}

Matrix Mat_Inverse_R(Matrix A)
{
	Matrix InvA;
	InvA = Mat_formalization();

	if (A.row != A.col)
	{
		printf("(Mat_Inverse_R)Error!!! : Non-square Matrix Error!\n");
		return InvA;
	}

	Matrix AA; //=[A|I]
	AA = Mat_formalization();

	AA.row = A.row; AA.col = AA.col * 2;
	for (int i = 0; i < A.row; i++)
	{
		for (int j=0; j < A.col; j++)
		{
			AA.M[i][j] = A.M[i][j];
			AA.M[i][A.col + j] = (i == j);
		}
	}
	print_Matrix(AA);
	// [A|I]--->[I|A']

	int pivot_row; // 각 열을 단위행렬으로 만들 때의 1의 위치 가지는 열
	for (int i = 0; i < A.col; i++)
	{
		pivot_row = -1; //초깃값 (못찾을때를 확인)
		for (int j = i; j < A.row; j++) // AA[i][i] , AA[i+1][i]....AA[*][i] 순서로 한 열을 다 확인하는 과정이고, 차피 대각 1을 찾기 위한거기 때문에 [i][i]부터 확인!
		{
			if ((AA.M[i][j] > ALMOST_ZERO) || (AA.M[i][j] < -ALMOST_ZERO)) { //0이 아닌 값을 찾자마자 바로 확인
				pivot_row = i; // 바로 체킹
				break; // 후 안쪽 루프 탈출
			}
		}
		if (pivot_row == -1) //역행렬이 존재하지 않을 경우!!
		{
			printf("(Mat_Inverse_R)Error!!! : Not Invertable...");
			return InvA;
		}
		if (pivot_row != i) //있어야 할 자리에는 0이 있고 다른곳에서 숫자를 찾은 경우...
		{
			Mat_exchange_rows(&AA, i, pivot_row);
		}
		Mat_scalar_row_mul(&AA, 1 / AA.M[i][i], i); // 1로 만들어주기

		for (int j = 0; j < AA.row; j++)
		{
			if (j != i) //자기를 제외한 행에 대해
			{
				Mat_row_add(&AA, -AA.M[j][i], i, j); // 다른 행은 걍 다 0으로 만들어주기
			}
		}
		print_Matrix(AA);
	}

	InvA.col = A.col; InvA.row = A.row;
	for (int i = 0; i < InvA.row; i++) //Inverse A를 값을 옮겨주고 반환
	{
		for (int j = 0; j < InvA.col; j++)
		{
			InvA.M[i][j] = AA.M[i][j + A.col];
		}
	}
	return InvA;
}

Matrix Mat_Inverse_GF2(Matrix A)
{
	Matrix InvA;
	InvA = Mat_formalization();

	printf("A = \n");
	print_Matrix_GF2(A);

	if (A.row != A.col)
	{
		printf("(Mat_Inverse_R)Error!!! : Non-square Matrix Error!\n");
		return InvA;
	}

	Matrix AA; //=[A|I]
	AA = Mat_formalization();

	AA.row = A.row; 
	AA.col = A.col;
	AA.col = AA.col * 2;
	for (int i = 0; i < A.row; i++)
	{
		for (int j = 0; j < A.col; j++)
		{
			AA.M[i][j] = A.M[i][j];
			AA.M[i][A.col + j] = (i == j) ? 1 : 0;
		}
	}
	print_Matrix_GF2(AA);
	// [A|I]--->[I|A']

	int pivot_row; // 각 열을 단위행렬으로 만들 때의 1의 위치 가지는 열
	for (int i = 0; i < A.col; i++)
	{
		pivot_row = -1; //초깃값 (못찾을때를 확인)
		for (int j = i; j < A.row; j++) // AA[i][i] , AA[i+1][i]....AA[*][i] 순서로 한 열을 다 확인하는 과정이고, 차피 대각 1을 찾기 위한거기 때문에 [i][i]부터 확인!
		{
			if (AA.M[j][i] != 0) { //0이 아닌 값을 찾자마자 바로 확인
				pivot_row = j; // 바로 체킹
				break; // 후 안쪽 루프 탈출
			}
		}
		if (pivot_row == -1) //역행렬이 존재하지 않을 경우!!
		{
			printf("(Mat_Inverse_R)Error!!! : Not Invertable...");
			return InvA;
		}
		if (pivot_row != i) //있어야 할 자리에는 0이 있고 다른곳에서 숫자를 찾은 경우...
		{
			Mat_exchange_rows(&AA, i, pivot_row);
		}
		//Mat_scalar_row_mul(&AA, 1 / AA.M[i][i], i); // 1로 만들어주기

		for (int j = 0; j < AA.row; j++)
		{
			if (j != i && AA.M[j][i] == 1) //자기를 제외한 행에 대해
			{
				Mat_row_add_GF2(&AA, i, j); // 다른 행은 걍 다 0으로 만들어주기
			}
		}
		print_Matrix_GF2(AA);
	}

	InvA.col = A.col; InvA.row = A.row;
	for (int i = 0; i < InvA.row; i++) //Inverse A를 값을 옮겨주고 반환
	{
		for (int j = 0; j < InvA.col; j++)
		{
			InvA.M[i][j] = AA.M[i][j + A.col];
		}
	}
	printf("\nInvA :\n");
	print_Matrix_GF2(InvA);
	return InvA;
}

byte EucAlg(byte a, byte b)
{
	byte q, r;
	byte temp;

	if (a - b < 0)
	{
		temp = a;
		a = b;
		b = temp;
	}
	while (b != 0)
	{
		q = a / b;
		r = a % b;
		a = b;
		b = r;
	}
	return a;
}

byte EucAlg_rec(byte a, byte b)
{
	if (b == 0) return a;
	EucAlg_rec(b, a % b);
	return 0;
}

byte EucAlg_simple(byte a, byte b)
{
	return (b == 0) ? (a) : (EucAlg_simple(b, a % b));
}

void print_GF2_8_element(byte a)
{
	byte exp = 0;
	for (int i = 0; i < 8; i++)
	{
		exp = 7 - i;

		if (i != 7)
		{
			if ((a >> exp) & 0x01)
			{
				printf("x^%1d", exp);
				printf(" + ");
			}
		}
		else
		{
			if ((a >> exp) & 0x01)
			{
				printf("1");
			}
			else {
				printf("\b\b\b");
			}
		}
	}
}

void print_GF2_8_add(byte a, byte b)
{
	byte c = a ^ b;

	print_GF2_8_element(a);

	printf(" + ");

	print_GF2_8_element(b);

	printf(" = ");

	print_GF2_8_element(c);
	printf("\n");
}

byte Add_GF2_8(byte a, byte b)
{
	byte c = 0;
	return c = a ^ b;
}

 //a와 b의 최대공약수(gcd)를 반환하며, 베주 항등식 진행 과정을 출력
byte Ex_euc_al(byte a, byte b)
{
	// 입력 원본 값 보존 (출력용)
	int A = a;
	int B = b;

	// 계수는 음수가 될 수 있으므로 부호 있는 정수형(int) 사용
	int u_a = 1, v_a = 0; // a = 1*A + 0*B
	int u_b = 0, v_b = 1; // b = 0*A + 1*B

	int cur_a = a;
	int cur_b = b;

	// 초기 상태 출력
	printf("%d = (%d) * %d + (%d) * %d\n", cur_a, u_a, A, v_a, B);
	printf("%d = (%d) * %d + (%d) * %d\n", cur_b, u_b, A, v_b, B);

	while (cur_b != 0)
	{
		int q = cur_a / cur_b;
		int r = cur_a % cur_b;

		// r = cur_a - q * cur_b 전개에 따른 새 계수 계산
		int u_r = u_a - q * u_b;
		int v_r = v_a - q * v_b;

		if (r != 0)
		{
			printf("%d = (%d) * %d + (%d) * %d\n", r, u_r, A, v_r, B);
		}

		// 다음 단계를 위한 값 갱신 (shift)
		cur_a = cur_b;
		cur_b = r;

		u_a = u_b;
		v_a = v_b;

		u_b = u_r;
		v_b = v_r;
	}

	// 최종 결과 출력: gcd(A, B) = u_a * A + v_a * B
	printf("최종 결과: gcd(%d, %d) = %d = (%d) * %d + (%d) * %d\n", A, B, cur_a, u_a, A, v_a, B);

	return (byte)cur_a;
}

//유한체 2^8에서의 곱셈을 xtimes로 구현
byte Multiply_GF2_8(byte a, byte b)
{
	byte res = 0;

	for (int i = 7; i >= 0; i--)
	{
		res = xtimes(res);
		res ^= ((b >> i) & 0x01) * a;
	}
	return res;
}

byte Findinverse_GF2_8(byte a)
{
	if (a == 0x00)
	{
		printf("\nVALUE ERROR : No inverse!!\n"); 
		return 0;
	}
	byte a_ = 1, a2 = 0, a4 = 0, a8 = 0, a16 = 0, a32 = 0, a64 = 0, a128 = 0;

	a2 = Multiply_GF2_8(a, a);
	a4 = Multiply_GF2_8(a2, a2);
	a8 = Multiply_GF2_8(a4, a4);
	a16 = Multiply_GF2_8(a8, a8);
	a32 = Multiply_GF2_8(a16, a16);
	a64 = Multiply_GF2_8(a32, a32);
	a128 = Multiply_GF2_8(a64, a64);

	//printf("\n%02x, %02x, %02x, %02x, %02x, %02x, %02x\n", a2, a4, a8, a16, a32, a64, a128);

	a_ = Multiply_GF2_8(a2, a4);
	byte a_2 = Multiply_GF2_8(a_, a8);
	byte a_3 = Multiply_GF2_8(a_2, a16);
	byte a_4 = Multiply_GF2_8(a_3, a32);
	byte a_5 = Multiply_GF2_8(a_4, a64);
	byte a_6 = Multiply_GF2_8(a_5, a128);

	return a_6;
}

byte Table_Multiplication_GF2_8(byte x, byte y, byte* log, byte* exp)
{
	byte res = 0;

	x = log[x];
	y = log[y];
	res = exp[x + y];

	return res;
}

int main()
{
	byte A = 48, B = 19, C = 0xbd, D = 0xbc, a1 = 0, a2 = 0, a3 = 0;

	byte test[6] = { 0x01,0x02,0x57,0x53,0xff,0x80 };

	byte atyu[8][8] =
	{
	{ 1, 0, 0, 0, 1, 1, 1, 1},
	{1, 1, 0, 0, 0, 1, 1, 1 },
	 {1, 1, 1, 0, 0, 0, 1, 1},
	 {1, 1, 1, 1, 0, 0, 0, 1},
	 {1, 1, 1, 1, 1, 0, 0, 0},
	 {0, 1, 1, 1, 1, 1, 0, 0},
	 {0, 0, 1, 1, 1, 1, 1, 0},
	 {0, 0, 0, 1, 1, 1, 1, 1} };

	Matrix M = Mat_formalization();
	M.col = 8;
	M.row = 8;
	for (int i = 0; i < M.row; i++)
	{
		for (int j = 0; j < M.col; j++)
		{
			M.M[i][j] = atyu[i][j];
		}
	}

	printf("\n=========================(4번 문제)==========================\n");

	Mat_Inverse_GF2(M);

	printf("\n=========================(5-a번 문제)==========================\n");
	
	print_sbox_16x16(s_box, 256);

	for (int i = 0; i < 256; i++)
	{
		if (s_box[i] == i)
		{
			printf("There Exists Fixed point!!!\n");
			break;
		}
		if (i == 255)
		{
			printf("There aren't any Fixed points!!!\n");
		}
	}

	for (int i = 0; i < 256; i++)
	{
		if (s_box[i] == (i ^ 0xff))
		{
			printf("There Exists Anti-Fixed point!!!\n");
			break;
		}
		if (i == 255)
		{
			printf("There aren't any Anti-Fixed points!!!\n");
		}
	}
	printf("\n=========================(5-c번 문제)==========================\n");

	Matrix SM = Mat_formalization();
	SM.col = 8;
	SM.row = 8;

	byte pesudo_sbox[256] = { 0, };

	SM = Mat_upper_rotate_8x8(M);
	for (int round = 0; round < 7; round++)
	{
		if (round != 0)SM = Mat_upper_rotate_8x8(SM);
		Mat_Make_sbox(pesudo_sbox, SM);
		print_sbox_16x16(pesudo_sbox, 256);
		int trigger = 0;

		for (int i = 0; i < 256; i++)
		{
			if (pesudo_sbox[i] == i)
			{
				printf("There Exists Fixed point!!! == [%02x]\n", i);
				trigger = 1;
			}
			if (i == 255 && trigger == 0)
			{
				printf("There aren't any Fixed points!!!\n");
			}
		}

		trigger = 0;

		for (int i = 0; i < 256; i++)
		{
			if (pesudo_sbox[i] == (i ^ 0xff))
			{
				printf("There Exists Anti-Fixed point!!! == [%02x]\n", i);
				trigger = 1;
			}
			if (i == 255 && trigger == 0)
			{
				printf("There aren't any Anti-Fixed points!!!\n");
			}
		}
	}

	printf("\n=========================(6-a번 문제)==========================\n");

	byte state = 0x01;
	byte a = 0x03;
	printf("\n");
	for (int i = 0; i < 255; i++)
	{
		printf("a^%d = 0x%02x\n", i, state);
		state = Multiply_GF2_8(state, a);
	}

	printf("\n=========================(6-c번 문제)==========================\n");
	
	state = 0x01;
	byte Exp_table[256] = { 0, };
	byte Log_table[256] = { 0, };
	byte alp = 0x03; //여기서 alpha값 임의지정
	
	printf("\n");
	Exp_table[0] = state;
	Log_table[state] = 0;
	for (int i = 0; i < 255; i++)
	{
		state = Multiply_GF2_8(state, alp);
		Exp_table[i+1] = state;
		Log_table[state] = i + 1;
	}
	printf("Exp, Log Table generated!!\n");

	printf("\n=========================(6-d번 문제)==========================\n");

	byte X = 0x16, Y = 0x03, Z = 0x00; //연산대상

	Z = Table_Multiplication_GF2_8(X, Y, Log_table, Exp_table);

	printf("\n0x%02x * 0x%02x = 0x%02x\n", X, Y, Z); //0x16 = a^129, 0x3a = a^130

	printf("verify!: ");
	if (Z == Multiply_GF2_8(X, Y))
	{
		printf("TRUE\n");
	}
	else
	{
		printf("FALSE\n");
	}

	//byte inv;

	/*a1 = EucAlg(A, B);
	printf("EA        : %d\n", a1);
	a2 = EucAlg_rec(A, B);
	printf("EA_rec    : %d\n", a2);
	a3 = EucAlg_simple(A, B);
	printf("EA_simple : %d\n", a3);*/

	//print_GF2_8_add(C, D);

	/*Ex_euc_al(A, B);
	for (int i = 0; i < 6;i++)
	{
		inv = Findinverse_GF2_8(test[i]);
		printf("\n<<0x%02x inverse!!>>\n 0x%02x ==>> 0x%02x\n", test[i], test[i], inv);
	}*/

	//print_test();
	//return 0;
}