/*-------------------------------------------*/
/*    역폴란드 표기법의 평가 polish_p.cpp    */
/*-------------------------------------------*/
#include <iostream>
#include <cstdlib>      // for exit()
#include <cctype>       // for is...()
using namespace std;

void polish(char *s);
int execute();
int getvalue(int ch);
int order(int ch);
void push(int n);
int pop();

#define STK_SIZ 20
int stack[STK_SIZ+1];                          /* 스택 */
int stkct;                                     /* 스택 포인터 */
char plsh_out[80];                             /* 역폴란드 출력 */

int main()
{
	char siki[80];
	int ans;

	cout << "입력: "; cin.getline(siki, 80);     /* 한 줄 읽기 */
	polish(siki);
	if (plsh_out[0] == '\0') exit(1);            /* 유효한 식이 없다 */
	ans = execute();
	cout << "변환: " << plsh_out << endl;
	cout << "결과: " << ans << endl;

	return 0;
}

void polish(char *s)
{
	int wk;
	char *out = plsh_out;

	stkct = 0;
	for (;;) {
		while (isspace(*s)) { ++s; }           /* 공백 건너뛰기         */
		if (*s == '\0') {                      /* 행의 끝이다           */
			while (stkct > 0) {                /* 스택의 나머지를 출력  */
				if ((*out++ = pop()) == '(') {
					puts("'('가 바르지 않음\n"); exit(1);
				}
			}
			break;
		}
		if (islower(*s) || isdigit(*s)) {       /* 변수나 숫자면 출력  */
			*out++ = *s++; continue;
		}
		switch (*s) {
		case '(':                              /* ( 라면 저장          */
			push(*s);
			break;
		case ')':                              /* ) 라면                */
			while ((wk = pop()) != '(') {      /* ( 를 만날 때까지 출력 */
				*out++ = wk;
				if (stkct == 0) { puts("'('가 바르지 않음\n"); exit(1); }
			}
			break;
		default:                               /*  연산자             */
			if (order(*s) == -1) {             /* 이용부적합문자      */
				cout << "바르지 않은 문자: " << *s << endl; exit(1);
			}            /* 자신보다 우선순위가 높은 요소가 스택 탑에 있는 동안*/
			while (stkct>0 && (order(stack[stkct]) >= order(*s))) {
				*out++ = pop();                /* 스택 내용을 출력 */
			}
			push(*s);                          /* 현재 요소를 저장      */
		}
		s++;
	}
	*out = '\0';
}

int execute()                                            /* 식의 계산  */
{
	int d1, d2;
	char *s;

	stkct = 0;
	for (s=plsh_out; *s; s++) {
		if (islower(*s))                       /* 변수라면           */
			push(getvalue(*s));                /* 값을 스택에 저장   */
		else if (isdigit(*s))                  /* 숫자라면           */
			push(*s - '0');                    /* 값을 스택에 저장   */
		else {                                 /* 연산자라면         */
			d2 = pop(); d1 = pop();            /* 두 값을 꺼낸다     */
			switch (*s) {
			case '+': push(d1+d2); break;      /* 계산결과를 저장한다 */
			case '-': push(d1-d2); break;
			case '*': push(d1*d2); break;
			case '/': if (d2 == 0) { puts("0으로 나눔"); exit(1); }
					  push(d1/d2); break;
			}
		}
	}
	if (stkct != 1) { cout << "error\n"; exit(1); }
	return pop();                                      /* 계산 결과를 반환한다 */
}

int getvalue(int ch)                                   /* 1~26을 반환한다 */
{
	if (islower(ch)) return ch-'a' + 1;
	return 0;
}

int order(int ch)                                        /* 우선순위 */
{
	switch (ch) {
	case '*': case '/': return 3;
	case '+': case '-': return 2;
	case '(':           return 1;
	}
	return -1;
}

void push(int n)                                     /* 스택 저장 */
{
	if (stkct+1 > STK_SIZ) { puts("stack overflow"); exit(1); }
	stack[++stkct] = n;
}

int pop()                                            /* 스택 추출 */
{
	if (stkct < 1) { puts("stack underflow"); exit(1); }
	return stack[stkct--];
}
