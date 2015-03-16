/**********************************************************/
/*    filename:bbi.h 공통 헤더                            */
/**********************************************************/
#include <iostream>
#include <fstream>    // 파일 처리용
#include <sstream>    // 문자열 스트림 
#include <string>
#include <vector>
#include <stack>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cctype>

using namespace std;

/* -------------------- define */
#define SHORT_SIZ  sizeof(short int)                  /* short int 형의 크기   */
#define SHORT_P(p) (short int *)(p)              /* short int형 포인터로 변환  */
#define UCHAR_P(p) (unsigned char *)(p)      /* unsigned char 형 포인터로 변환 */
#define LIN_SIZ 255                                  /*소스 한 줄의 최대 크기  */

/* -------------------- enum struct etc */
enum TknKind {                                                /* 토큰 요소*/
	Lparen='(', Rparen=')', Lbracket='[', Rbracket=']', Plus='+', Minus='-',
	Multi='*',  Divi='/',   Mod='%',      Not='!',      Ifsub='?',  Assign='=',
	IntDivi='\\', Comma=',',     DblQ='"',
	Func=150, Var,   If,     Elif,   Else,   For, To, Step,  While,
	End,      Break, Return, Option, Print,  Println, Input, Toint,
	Exit,     Equal, NotEq,  Less,   LessEq, Great,   GreatEq, And, Or,
	END_KeyList,
	Ident,      IntNum, DblNum, String,   Letter, Doll, Digit,
	Gvar, Lvar, Fcall,  EofProg, EofLine, Others
};

struct Token {                /* 토큰 관리     */
	TknKind kind;               /* 토큰 종류     */
	string  text;               /* 토큰 문자열   */
	double  dblVal;             /* 수치 상수일 때의 값 */
	Token() {  kind=Others; text=""; dblVal=0.0; }
	Token (TknKind k)           { kind=k; text=""; dblVal=0.0; }
	Token (TknKind k, double d) { kind=k; text=""; dblVal=d; }
	Token (TknKind k, const string& s) { kind=k; text=s; dblVal=0.0; }
	Token (TknKind k, const string& s, double d) { kind=k; text=s; dblVal=d; }
};

enum SymKind { noId, varId, fncId, paraId };  /* 심볼 테이블 등록 이름의 종류 */
enum DtType  { NON_T, DBL_T };                /* 타입 이름 */

struct SymTbl {               /* 심볼 테이블 구성        */
	string  name;               /* 변수나 함수의 이름      */
	SymKind nmKind;             /* 종류                    */
	char    dtTyp;              /* 변수형(NON_T,DBL_T)     */
	int     aryLen;             /* 배열 길이. 0: 단순변수  */
	short   args;               /* 함수의 인수 개수        */
	int     adrs;               /* 변수, 함수의 주소       */
	int     frame;              /* 함수용 프레임 크기      */
	SymTbl() { clear(); }
	void clear() {
		name=""; nmKind=noId; dtTyp=NON_T;
		aryLen=0; args=0; adrs=0; frame=0;
	}
};

struct CodeSet {             /* 코드 관리                    */
	TknKind kind;              /* 종류                        */
	const char *text;          /* 문자열 리터럴일 때의 위치        */
	double dblVal;             /* 수치 상수일 때의 값            */
	int    symNbr;             /* 심볼 테이블의 첨자 위치         */
	int    jmpAdrs;            /* 점프할 주소                  */
	CodeSet() { clear(); }
	CodeSet(TknKind k)           { clear(); kind=k; }
	CodeSet(TknKind k, double d) { clear(); kind=k; dblVal=d; }
	CodeSet(TknKind k, const char *s) { clear(); kind=k; text=s; }
	CodeSet(TknKind k, int sym, int jmp) {
		clear(); kind=k; symNbr=sym; jmpAdrs=jmp;
	}
	void clear() { kind=Others; text=""; dblVal=0.0; jmpAdrs=0; symNbr=-1; }
};

struct Tobj {        /* 형 정보 부 obj     */
	char type;         /* 저장형 'd':double 's':string  '-':미정 */
	double d;
	string s;
	Tobj()                 { type = '-'; d = 0.0; s = ""; }
	Tobj(double dt)        { type = 'd'; d = dt;  s = ""; }
	Tobj(const string& st) { type = 's'; d = 0.0; s = st; }
	Tobj(const char *st)   { type = 's'; d = 0.0; s = st; }
};

class Mymemory {
private:
	vector<double> mem;
public:
	void auto_resize(int n) {                 /* 재확보 횟수 억제를 위해 확보 */
		if (n >= (int)mem.size()) { n = (n/256 + 1) * 256; mem.resize(n); }
	}
	void set(int adrs, double dt) { mem[adrs] =  dt; }            /* 메모리 쓰기     */
	void add(int adrs, double dt) { mem[adrs] += dt; }            /* 메모리 더하기   */
	double get(int adrs)     { return mem[adrs]; }                /* 메모리 읽어내기 */
	int size()               { return (int)mem.size(); }          /* 저장 크기       */
	void resize(unsigned int n) { mem.resize(n); }                /* 크기 확보       */
};

