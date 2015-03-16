/**********************************************************/
/*    filename:bbi_tkn.cpp 토큰 처리                      */
/**********************************************************/
#include "bbi.h"
#include "bbi_prot.h"

struct KeyWord {                               /* 어휘와 종별의 대응을 관리 */
	const char *keyName;                                       /* if나 for 등 */
	TknKind keyKind;                 /* 대응하는 값. TknKind는 bbi.h에서 정의 */
};

KeyWord KeyWdTbl[] = {                          /* 예약어, 심볼, 종별 대응 테이블 */
	{"func"   , Func  }, {"var"    , Var    },
	{"if"     , If    }, {"elif"   , Elif   },
	{"else"   , Else  }, {"for"    , For    },
	{"to"     , To    }, {"step"   , Step   },
	{"while"  , While }, {"end"    , End    },
	{"break"  , Break }, {"return" , Return },
	{"print"  , Print }, {"println", Println},
	{"option" , Option}, {"input"  , Input  },
	{"toint"  , Toint }, {"exit"   , Exit   },
	{"("  , Lparen    }, {")"  , Rparen   },
	{"["  , Lbracket  }, {"]"  , Rbracket },
	{"+"  , Plus      }, {"-"  , Minus    },
	{"*"  , Multi     }, {"/"  , Divi     },
	{"==" , Equal     }, {"!=" , NotEq    },
	{"<"  , Less      }, {"<=" , LessEq   },
	{">"  , Great     }, {">=" , GreatEq  },
	{"&&" , And       }, {"||" , Or       },
	{"!"  , Not       }, {"%"  , Mod      },
	{"?"  , Ifsub     }, {"="  , Assign   },
	{"\\" , IntDivi   }, {","  , Comma    },
	{"\"" , DblQ      },
	{"@dummy", END_KeyList},
};

int srcLineno;                                          /* 소스의 행 번호 */
TknKind ctyp[256];                                    /* 문자 종류표 배열 */
char *token_p;                                   /* 1문자 획득용 문자위치 */
bool endOfFile_F;                                     /* 파일 종료 플래그 */
char buf[LIN_SIZ+5];                                  /* 소스 읽어들일 곳 */
ifstream fin;                                              /* 입력 스트림 */
#define MAX_LINE 2000                               /* 최대 프로그램 행수 */

void initChTyp() /* 문자 종류표 설정 */
{                  /* 주: 모든 요소를 사용하진 않지만 확장을 대비해 넣고 있다 */
	int i;
	for (i=0; i<256; i++)    { ctyp[i] = Others; }
	for (i='0'; i<='9'; i++) { ctyp[i] = Digit;  }
	for (i='A'; i<='Z'; i++) { ctyp[i] = Letter; }
	for (i='a'; i<='z'; i++) { ctyp[i] = Letter; }
	ctyp['_']  = Letter;    ctyp['$']  = Doll;
	ctyp['(']  = Lparen;    ctyp[')']  = Rparen;
	ctyp['[']  = Lbracket;  ctyp[']']  = Rbracket;
	ctyp['<']  = Less;      ctyp['>']  = Great;
	ctyp['+']  = Plus;      ctyp['-']  = Minus;
	ctyp['*']  = Multi;     ctyp['/']  = Divi;
	ctyp['!']  = Not;       ctyp['%']  = Mod;
	ctyp['?']  = Ifsub;     ctyp['=']  = Assign;
	ctyp['\\'] = IntDivi;   ctyp[',']  = Comma;
	ctyp['\"'] = DblQ;
}

void fileOpen(char *fname) /* 파일 열기 */
{
	srcLineno = 0;
	endOfFile_F = false;
	fin.open(fname);
	if (!fin) { cout << fname << " 을(를) 열 수 없습니다\n"; exit(1); }
}

void nextLine() /* 다음 한 줄을 가져온다 */
{
	string s;

	if (endOfFile_F) return;
	fin.getline(buf, LIN_SIZ+5);                                 /* 1행 읽기 */
	if (fin.eof()) {                                            /* 파일 종료 */
		fin.clear(); fin.close();                /* clear는 재 오픈에 대비한다 */
		endOfFile_F = true; return;
	}

	if (strlen(buf) > LIN_SIZ)
		err_exit("프로그램은 1행 ", LIN_SIZ, " 문자이내로 기술해 주세요");
	if (++srcLineno > MAX_LINE)
		err_exit("프로그램이 ", MAX_LINE, " 을(를) 넘었습니다.");
	token_p = buf;                 /* 토큰 분석용 포인터를 buf 맨 앞에 위치시킨다 */
}

Token nextLine_tkn() /* 다음 행을 읽고 다음 토큰을 반환한다 */
{
	nextLine();
	return nextTkn();
}

#define CH (*token_p)
#define C2 (*(token_p+1))
#define NEXT_CH()  ++token_p
Token nextTkn() /* 다음 토큰 */
{
	TknKind kd;
	string txt = "";

	if (endOfFile_F) return Token(EofProg);                     /* 파일 종료 */
	while (isspace(CH)) NEXT_CH();                              /* 공백 스킵 */
	if (CH == '\0')  return Token(EofLine);                          /* 행끝 */

	switch (ctyp[CH]) {
	case Doll: case Letter:
		txt += CH; NEXT_CH();
		while (ctyp[CH]==Letter || ctyp[CH]==Digit) { txt += CH; NEXT_CH(); }
		break;
	case Digit:                                                     /* 수치상수 */
		kd = IntNum;
		while (ctyp[CH] == Digit)   { txt += CH; NEXT_CH(); }
		if (CH == '.') { kd = DblNum; txt += CH; NEXT_CH(); }
		while (ctyp[CH] == Digit)   { txt += CH; NEXT_CH(); }
		return Token(kd, txt, atof(txt.c_str()));       /* IntNum도 double형으로 저장 */
	case DblQ:                                                    /* 문자열 상수 */
		NEXT_CH();
		while (CH!='\0' && CH!='"') { txt += CH; NEXT_CH(); }
		if (CH == '"') NEXT_CH(); else err_exit("문자열 리터럴이 닫혀있지 않다. ");
		return Token(String, txt);
	default:
		if (CH=='/' && C2=='/') return Token(EofLine);                /* 주석 */
		if (is_ope2(CH, C2)) { txt += CH; txt += C2; NEXT_CH(); NEXT_CH(); }
		else                 { txt += CH; NEXT_CH(); }
	}
	kd = get_kind(txt);                                             /* 종류별 설정 */

	if (kd == Others) err_exit("잘못된 토큰입니다: ", txt);
	return Token(kd, txt);
}

bool is_ope2(char c1, char c2) /* 2문자 연산자면 참 */
{
	char s[] = "    ";
	if (c1=='\0' || c2=='\0') return false;
	s[1] = c1; s[2] = c2;
	return strstr(" ++ -- <= >= == != && || ", s) != NULL;
}

TknKind get_kind(const string& s) /* 토큰 종류별 설정 */
{
	for (int i=0; KeyWdTbl[i].keyKind != END_KeyList; i++) {
		if (s == KeyWdTbl[i].keyName) return KeyWdTbl[i].keyKind;
	}
	if (ctyp[s[0]]==Letter || ctyp[s[0]]==Doll) return Ident;
	if (ctyp[s[0]] == Digit)  return DblNum;
	return Others;   // 없다
}

Token chk_nextTkn(const Token& tk, int kind2) /* 확인부 토큰 획득 */
{
	if (tk.kind != kind2) err_exit(err_msg(tk.text, kind_to_s(kind2)));
	return nextTkn();
}

void set_token_p(char *p) /* 토큰 처리 포인터 설정 */
{
	token_p = p;
}

string kind_to_s(int kd) /* 종류별 → 문자열 */
{
	for (int i=0; ; i++) {
		if (KeyWdTbl[i].keyKind == END_KeyList) break;
		if (KeyWdTbl[i].keyKind == kd) return KeyWdTbl[i].keyName;
	}
	return "";
}

string kind_to_s(const CodeSet& cd) /*  종류별 → 문자열 */
{
	switch (cd.kind) {
	case Lvar: case Gvar: case Fcall: return tableP(cd)->name;
	case IntNum: case DblNum: return dbl_to_s(cd.dblVal);
	case String: return string("\"") + cd.text + "\"";
	case EofLine: return "";
	}
	return kind_to_s(cd.kind);
}

int get_lineNo() /* 읽기 or 실행 중  */
{
	extern int Pc;
	return (Pc == -1) ? srcLineno : Pc;          /* 분석 중 : 실행 중 */
}

