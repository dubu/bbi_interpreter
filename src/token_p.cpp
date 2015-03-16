/*--------------------------*/
/*    어휘 분석 token_p.cpp  */
/*--------------------------*/
#include <iostream>
#include <fstream>    // 파일 처리용
#include <iomanip>    // 출력 형식 조작용  
#include <string>
#include <cstdlib>
#include <cctype>
using namespace std;

enum TknKind {                              /* 토큰의 종류 등 */
	Lparen=1, Rparen, Plus,  Minus,  Multi,  Divi,
	Assign,   Comma,  DblQ,
	Equal,    NotEq,  Less,  LessEq, Great,  GreatEq,
	If,       Else,   End,   Print,  Ident,  IntNum,
	String,   Letter, Digit, EofTkn, Others, END_list
};

struct Token {
	TknKind kind;                           /* 토큰의 종류      */
	string text;                            /* 토큰 문자열      */
	int  intVal;                            /* 상수일 때 그 값  */
	Token () { kind = Others; text = ""; intVal = 0; }
	Token (TknKind k, const string& s, int d=0) {
		kind = k; text = s;  intVal = d;
	}
};

void initChTyp();
Token nextTkn();
int nextCh();
bool is_ope2(int c1, int c2);
TknKind get_kind(const string& s);

TknKind ctyp[256];                          /* 문자종표      */
Token token;                                /* 토큰 저장   */
ifstream fin;                               /* 입력 스트림 */

struct KeyWord {                            /* 단어           */
	const char *keyName;
	TknKind keyKind;                        /* 종류           */
};

KeyWord KeyWdTbl[] = {
	{"if" , If    }, {"else",  Else    },
	{"end", End   }, {"print", Print   },
	{"(",   Lparen}, {")",     Rparen  },
	{"+",   Plus  }, {"-",     Minus   },
	{"*",   Multi }, {"/",     Divi    },
	{"=",   Assign}, {",",     Comma   },
	{"==",  Equal }, {"!=",    NotEq   },
	{"<",   Less  }, {"<=",    LessEq  },
	{">",   Great }, {">=",    GreatEq },
	{"", END_list},
};

int main(int argc, char *argv[])
{
	if (argc == 1) exit(1);
	fin.open(argv[1]); if (!fin) exit(1);

	cout << "text     kind intVal\n";
	initChTyp();
	for (token = nextTkn(); token.kind != EofTkn; token = nextTkn()) {
		cout << left << setw(10) << token.text
			<< right << setw(3) << token.kind
			<< " " << token.intVal << endl;
	}
	return 0;
}

void initChTyp()                            /* 문자종표 설정 */
{
	int i;

	for (i=0; i<256; i++)    { ctyp[i] = Others; }
	for (i='0'; i<='9'; i++) { ctyp[i] = Digit;  }
	for (i='A'; i<='Z'; i++) { ctyp[i] = Letter; }
	for (i='a'; i<='z'; i++) { ctyp[i] = Letter; }
	ctyp['('] = Lparen; ctyp[')'] = Rparen;
	ctyp['<'] = Less;   ctyp['>'] = Great;
	ctyp['+'] = Plus;   ctyp['-'] = Minus;
	ctyp['*'] = Multi;  ctyp['/'] = Divi;
	ctyp['_'] = Letter; ctyp['='] = Assign;
	ctyp[','] = Comma;  ctyp['"'] = DblQ;
}

Token nextTkn() /* 토큰 추출. 한자나 에스케이프 문자는 비대응 */
{
	TknKind kd;
	int ch0, num = 0;
	static int ch = ' ';                    /* static으로 이전 문자를 유지 */
	string txt = "";

	while (isspace(ch)) { ch = nextCh(); }  /* 공백 스킵 */
	if (ch == EOF) return Token(EofTkn, txt);

	switch (ctyp[ch]) {
	case Letter:                            /* 식별자 */
		for ( ; ctyp[ch]==Letter || ctyp[ch]==Digit; ch=nextCh()) {
			txt += ch;
		}
		break;
	case Digit:                             /* 숫자 */
		for (num=0; ctyp[ch]==Digit; ch=nextCh()) {
			num = num*10 + (ch-'0');
		}
		return Token(IntNum, txt, num);
	case DblQ:                              /* 문자열 상수 */
		for (ch=nextCh(); ch!=EOF && ch!='\n' && ch!='"'; ch=nextCh()) {
			txt += ch;
		}
		if (ch != '"') { cout << "문자열 리터럴을 닫지 않음\n"; exit(1); }
		ch = nextCh();
		return Token(String, txt);
	default:                                /* 기호 */
		txt += ch; ch0 = ch; ch = nextCh();
		if (is_ope2(ch0, ch)) { txt += ch; ch = nextCh(); }
	}
	kd = get_kind(txt);                     /* 종류 설정 */
	if (kd == Others) {
		cout << "잘못된 토큰: " << txt << endl; exit(1);
	}
	return Token(kd, txt);
}

int nextCh()                                /* 다음 1문자 */
{
	static int c = 0;
	if (c == EOF) return c;
	if ((c=fin.get()) == EOF) fin.close();  /* 종료 */
	return c;
}

bool is_ope2(int c1, int c2)                /* 2문자 연산자라면 참 */
{
	char s[] = "    ";
	if (c1=='\0' || c2=='\0') return false;
	s[1] = c1; s[2] = c2;
	return strstr(" <= >= == != ", s) != NULL;
}

TknKind get_kind(const string& s)
{
	for (int i=0; KeyWdTbl[i].keyKind != END_list; i++) {
		if (s == KeyWdTbl[i].keyName) return KeyWdTbl[i].keyKind;
	}
	if (ctyp[s[0]] == Letter) return Ident;
	if (ctyp[s[0]] == Digit)  return IntNum;
	return Others;
}
