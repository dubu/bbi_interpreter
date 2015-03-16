/**************************************************************/
/*    filename:bbi_code.cpp 메모리 관리, 구문 검사, 실행    */
/**************************************************************/
#include "bbi.h"
#include "bbi_prot.h"

CodeSet code;                                                    /* 코드 세트 */
int startPc;                                                   /* 실행 시작행 */
int Pc = -1;                                /* 프로그램 카운터 -1: 실행중 아님*/
int baseReg;                                               /* 베이스 레지스터 */
int spReg;                                                     /* 스택 포인터 */
int maxLine;                                                 /* 프로그램 끝행 */
vector<char*> intercode;                        /* 변환이 끝난 내부 코드 저장 */
char *code_ptr;                                    /* 내부 코드 분석용 포인터 */
double returnValue;                                           /* 함수 반환 값 */
bool break_Flg, return_Flg, exit_Flg;                        /* 제어용 플래그 */
Mymemory Dmem;                                                      /* 메모리 */
vector<string> strLITERAL;                              /* 문자열 리터럴 저장 */
vector<double> nbrLITERAL;                                /* 수치 리터럴 저장 */
bool syntaxChk_mode = false;                             /* 구문 검사일 때 참 */
extern vector<SymTbl> Gtable;                           /* 글로벌 심볼 테이블 */

class Mystack {                                     /* stack<double> 의 랩퍼 */
private: 
	stack<double> st;
public:
	void push(double n) { st.push(n); }                                 /* 넣기 */
	int size() { return (int)st.size(); }                               /* 크기 */
	bool empty() { return st.empty(); }                        /* 비었는지 판정 */
	double pop() {                        /* 읽기 & 삭제(원래 pop와 다르니 주의 */
		if (st.empty()) err_exit("stack underflow");
		double d = st.top();                                     /* 스택 탑의 값 */
		st.pop(); return d;                                          /* 하나 삭제 */
	}
};
Mystack stk;                                            /* 오퍼랜드 스택 */

void syntaxChk() /* 구문 검사 */
{
	syntaxChk_mode = true;
	for (Pc=1; Pc<(int)intercode.size(); Pc++) {
		code = firstCode(Pc);
		switch (code.kind) {
		case Func: case Option: case Var:                         /* 검사 완료 */
			break;
		case Else: case End: case Exit:
			code = nextCode(); chk_EofLine();
			break;
		case If: case Elif: case While:
			code = nextCode(); (void)get_expression(0, EofLine);            /* 식값 */
			break;
		case For:
			code = nextCode();
			(void)get_memAdrs(code);                              /* 제어 변수 주소 */
			(void)get_expression('=', 0);                                 /* 초깃값 */
			(void)get_expression(To, 0);                                  /* 최종값 */
			if (code.kind == Step) (void)get_expression(Step,0);          /* 증분값 */
			chk_EofLine();
			break;
		case Fcall:                                         /* 대입 없는 함수 호출 */
			fncCall_syntax(code.symNbr);
			chk_EofLine();
			(void)stk.pop();                                          /* 반환 값 불필요 */
			break;
		case Print: case Println:
			sysFncExec_syntax(code.kind);
			break;
		case Gvar: case Lvar:                                           /* 대입문 */
			(void)get_memAdrs(code);                                   /* 좌변 주소 */
			(void)get_expression('=', EofLine);                        /* 우변식 값 */
			break;
		case Return:
			code = nextCode();                                              /* 반환 값 */
			if (code.kind!='?' && code.kind!=EofLine) (void)get_expression();
			if (code.kind == '?') (void)get_expression('?', 0);
			chk_EofLine();
			break;
		case Break:
			code = nextCode();
			if (code.kind == '?') (void)get_expression('?', 0);
			chk_EofLine();
			break;
		case EofLine:
			break;
		default:
			err_exit("잘못된 기술입니다: ", kind_to_s(code.kind));
		}
	}
	syntaxChk_mode = false;
}

void set_startPc(int n) /* 시작행 설정 */
{
	startPc = n;
}

void execute() /* 실행 */
{
	baseReg = 0;                                         /* 베이스 레지스터 초깃값 */
	spReg = Dmem.size();                                     /* 스택 포인터 초깃값 */
	Dmem.resize(spReg+1000);                              /* 메모리 영역 초기 확보 */
	break_Flg = return_Flg = exit_Flg = false;

	Pc = startPc;
	maxLine = intercode.size() - 1;
	while (Pc<=maxLine && !exit_Flg) {
		statement();
	}
	Pc = -1;                                                      /* 비실행 상태 */
}

void statement() /* 문 */
{
	CodeSet save;
	int top_line, end_line, varAdrs;
	double wkVal, endDt, stepDt;

	if (Pc>maxLine || exit_Flg) return;                     /* 프로그램 종료 */
	code = save = firstCode(Pc);
	top_line = Pc; end_line = code.jmpAdrs;          /* 제어 범위의 시작과 끝 */
	if (code.kind == If ) end_line = endline_of_If(Pc);     /* if문일 때의 끝 */

	switch (code.kind) {
	case If:
		// if
		if (get_expression(If, 0)) {                          /*   참(TRUE)이면    */
			++Pc; block(); Pc = end_line + 1;                   /*   실행하고     */
			return;                                             /*   종료        */
		}
		Pc = save.jmpAdrs;                                    /*   다음으로         */
		// elif
		while (lookCode(Pc) == Elif) {
			save = firstCode(Pc); code = nextCode();
			if (get_expression()) {                             /*   참(TRUE)이면     */
				++Pc; block(); Pc = end_line + 1;                 /*   실행하고    */
				return;                                           /*   종료         */
			}
			Pc = save.jmpAdrs;                                  /*   다음으로         */
		}
		// else
		if (lookCode(Pc) == Else) {                           /* else를       */
			++Pc; block(); Pc = end_line + 1;                   /*   실행하고      */
			return;                                             /* 종료         */
		}
		// end
		++Pc;
		break;
	case While:
		for (;;) {                                           /*                   */
			if (!get_expression(While, 0)) break;              /*   false 종료   */
			++Pc; block();                                     /*        [실행]      */
			if (break_Flg || return_Flg || exit_Flg) {         /*                  */
				break_Flg = false; break;                        /*        중단        */
			}                                                  /*                   */
			Pc = top_line; code = firstCode(Pc);               /*   맨 앞으로        */
		}                                                    /*                   */
		Pc = end_line + 1;                                   /*                   */
		break;
	case For:										/* for 제어변수, 초깃값, 최종값, 증분식 */
		save = nextCode();
		varAdrs = get_memAdrs(save);                    /* 제이변수 주소 구하기  */

		expression('=', 0);                                             /* 초깃값   */
		set_dtTyp(save, DBL_T);                                         /* 형 확정  */
		Dmem.set(varAdrs, stk.pop());                             /*   초깃값을 설정  */

		endDt = get_expression(To, 0);                            /* 최종값을 보존   */
		/* 증분값을 보존   */
		if (code.kind == Step) stepDt = get_expression(Step, 0); else stepDt = 1.0;
		for (;; Pc=top_line) {                               /*                   */
			if (stepDt >= 0) {                                 /*   증가 루프         */
				if (Dmem.get(varAdrs) > endDt) break;            /* 거짓이면 종료        */
			} else {                                           /*   감소 루프         */
				if (Dmem.get(varAdrs) < endDt) break;            /* 거짓이면 종료        */
			}                                                  /*                   */
			++Pc; block();                                     /*   [ 실행 ]         */
			if (break_Flg || return_Flg || exit_Flg) {         /*                   */
				break_Flg = false; break;                        /*    중단            */
			}                                                  /*                   */
			Dmem.add(varAdrs, stepDt);                         /* 값 갱신             */
		}                                                    /*                   */
		Pc = end_line + 1;                                   /*                    */
		break;
	case Fcall:                                           /* 대입이 없는 함수 호출 */
		fncCall(code.symNbr);
		(void)stk.pop();                                            /* 반환 값 불필요 */
		++Pc;
		break;
	case Func:                                            /* 함수 정의는 건너뀜 */
		Pc = end_line + 1;
		break;
	case Print: case Println:
		sysFncExec(code.kind);
		++Pc;
		break;
	case Gvar: case Lvar:                                             /* 대입문 */
		varAdrs = get_memAdrs(code);
		expression('=', 0);
		set_dtTyp(save, DBL_T);                                 /* 대입할 때 형 확정*/
		Dmem.set(varAdrs, stk.pop());
		++Pc;
		break;
	case Return:
		wkVal = returnValue;
		code = nextCode();
		if (code.kind!='?' && code.kind!=EofLine)   /* '식'이 있으면 반환 값을 계산 */
			wkVal = get_expression();
		post_if_set(return_Flg);                                /* ?가 있으면 처리 */
		if (return_Flg) returnValue = wkVal;
		if (!return_Flg) ++Pc;
		break;
	case Break:
		code = nextCode(); post_if_set(break_Flg);              /* ? 가 있으면 처리 */
		if (!break_Flg) ++Pc;
		break;
	case Exit:
		code = nextCode(); exit_Flg = true;
		break;
	case Option: case Var: case EofLine:                        /* 실행 시는 무시 */
		++Pc;
		break;
	default:
		err_exit("잘못된 기술입니다: ", kind_to_s(code.kind));
	}
}

void block()  // 블록 끝까지 문을 실행 
{
	TknKind k;
	while (!break_Flg && !return_Flg && !exit_Flg) {  // break, return, exit으로 종료　
		k = lookCode(Pc);                               // 다음 시작 코드 
		if (k==Elif || k==Else || k==End) break;        // 블록 정상 종료 
		statement();							        		
	}
}

// 함수 선언에서 다음의 기본 인수를 지정
// double get_expression(int kind1=0, int kind2=0)
double get_expression(int kind1, int kind2) /* 결과를 반환하는 expression */
{
	expression(kind1, kind2); return stk.pop();
}

void expression(int kind1, int kind2) /* 조건부 식처리 */
{
	if (kind1 != 0) code = chk_nextCode(code, kind1);
	expression();
	if (kind2 != 0) code = chk_nextCode(code, kind2);
}

void expression() /* 일반 식 처리 */
{
	term(1);
}

void term(int n) /* n은 우선 순위 */
{
	TknKind op;
	if (n == 7) { factor(); return; }
	term(n+1);
	while (n == opOrder(code.kind)) {                /* 우선 순위가 같은 연산자가 연속된다 */
		op = code.kind;
		code = nextCode(); term(n+1);
		if (syntaxChk_mode) { stk.pop(); stk.pop(); stk.push(1.0); }   /* 구문 chk 시 */
		else binaryExpr(op);
	}
}

void factor() /* 인자 */
{
	TknKind kd = code.kind;

	if (syntaxChk_mode) {                                          /* 구문 chk 시 */
		switch (kd) {
		case Not: case Minus: case Plus:
			code = nextCode(); factor(); stk.pop(); stk.push(1.0);
			break;
		case Lparen:
			expression('(', ')');
			break;
		case IntNum: case DblNum:
			stk.push(1.0); code = nextCode();
			break;
		case Gvar: case Lvar:
			(void)get_memAdrs(code); stk.push(1.0);
			break;
		case Toint: case Input:
			sysFncExec_syntax(kd);
			break;
		case Fcall:
			fncCall_syntax(code.symNbr);
			break;
		case EofLine:
			err_exit("식이 바르지 않습니다.");
		default:
			err_exit("식 오류:", kind_to_s(code));            /* a + = 등에서 발생 */
		}
		return;
	}

	switch (kd) {                                                     /* 실행시 */
	case Not: case Minus: case Plus:
		code = nextCode(); factor();                         /* 다음 값을 획득 */
		if (kd == Not) stk.push(!stk.pop());                      /* !처리한다 */
		if (kd == Minus) stk.push(-stk.pop());                    /* -처리한다 */
		break;                                /* 단항 +는 아무것도 하지 않는다 */
	case Lparen:
		expression('(', ')');
		break;
	case IntNum: case DblNum:
		stk.push(code.dblVal); code = nextCode();
		break;
	case Gvar: case Lvar:
		chk_dtTyp(code);                                 /* 값 설정을 마친 변수인가 */
		stk.push(Dmem.get(get_memAdrs(code)));
		break;
	case Toint: case Input:
		sysFncExec(kd);
		break;
	case Fcall:
		fncCall(code.symNbr);
		break;
	}
}

int opOrder(TknKind kd) /* 이항 연산자 우선 순위  */
{
	switch (kd) {
	case Multi: case Divi: case Mod:
	case IntDivi:                    return 6; /* *  /  % \  */
	case Plus:  case Minus:          return 5; /* +  -       */
	case Less:  case LessEq:
	case Great: case GreatEq:        return 4; /* <  <= > >= */
	case Equal: case NotEq:          return 3; /* == !=      */
	case And:                        return 2; /* &&         */
	case Or:                         return 1; /* ||         */
	default:                         return 0; /* 해당 없음    */
	}
}

void binaryExpr(TknKind op) /* 이항 연산 */
{
	double d = 0, d2 = stk.pop(), d1 = stk.pop();

	if ((op==Divi || op==Mod || op==IntDivi) && d2==0)
		err_exit("0으로 나누었습니다.");

	switch (op) {
	case Plus:    d = d1 + d2;  break;
	case Minus:   d = d1 - d2;  break;
	case Multi:   d = d1 * d2;  break;
	case Divi:    d = d1 / d2;  break;
	case Mod:     d = (int)d1 % (int)d2; break;
	case IntDivi: d = (int)d1 / (int)d2; break;
	case Less:    d = d1 <  d2; break;
	case LessEq:  d = d1 <= d2; break;
	case Great:   d = d1 >  d2; break;
	case GreatEq: d = d1 >= d2; break;
	case Equal:   d = d1 == d2; break;
	case NotEq:   d = d1 != d2; break;
	case And:     d = d1 && d2; break;
	case Or:      d = d1 || d2; break;
	}
	stk.push(d);
}

void post_if_set(bool& flg) /* ? 식 */
{
	if (code.kind == EofLine) { flg = true; return; }       /* ?가 없으면 flg를 참으로  */
	if (get_expression('?', 0)) flg = true;                     /* 조건식으로 처리 */
}

void fncCall_syntax(int fncNbr) /* 함수 호출 검사 */
{
	int argCt = 0;

	code = nextCode(); code = chk_nextCode(code, '(');
	if (code.kind != ')') {                                       /* 인수가 있다 */
		for (;; code=nextCode()) {
			(void)get_expression(); ++argCt;                /* 인수식 처리와 인수 개수 */
			if (code.kind != ',') break;                     /* , 이면 인수가 계속된다 */
		}
	}
	code = chk_nextCode(code, ')');                                /* ) 일 것 */
	if (argCt != Gtable[fncNbr].args)                       /* 인수 개수 검사 */
		err_exit(Gtable[fncNbr].name, " 함수의 인수 개수가 잘못되었습니다.");
	stk.push(1.0);                                          /* 적당한 반환 값 */
}

//void fncCall(int fncNbr) /* ä÷êîåƒèo */
void fncCall(int fncNbr) /* 함수호출 확인 */
{
	int  n, argCt = 0;
	vector<double> vc;

	// 실인수 저장
	nextCode(); code = nextCode();                         /* 함수명 ( 건너뜀 */
	if (code.kind != ')') {                                    /* 인수가 있다 */
		for (;; code=nextCode()) {
			expression(); ++argCt;                     /* 인수식 처리와 인수 개수 */
			if (code.kind != ',') break;                 /* ,라면 인수가 계속된다 */
		}
	}
	code = nextCode();                                            /* ) 건너뜀 */

	// 인수 저장 순서 변경
	for (n=0; n<argCt; n++) vc.push_back(stk.pop());  /* 뒤에서부터 인수 저장으로 수정*/
	for (n=0; n<argCt; n++) { stk.push(vc[n]); }

	fncExec(fncNbr);                                                /* 함수 실행 */
}

void fncExec(int fncNbr) /* 함수 실행 */
{
	// 함수입구처리1
	int save_Pc = Pc;                                     /* 현재 실행행을 저장 */
	int save_baseReg = baseReg;                          /* 현재 baseReg를 저장 */
	int save_spReg = spReg;                                /* 현재 spReg를 저장 */
	char *save_code_ptr = code_ptr;         /* 현재 실행행 분석용 포인터를 저장 */
	CodeSet save_code = code;                               /* 현재 code를 저장 */

	// 함수입구처리2
	Pc = Gtable[fncNbr].adrs;                                 /* 새로운 Pc 설정 */
	baseReg = spReg;                             /* 새로운 베이스 레지스터 설정 */
	spReg += Gtable[fncNbr].frame;                               /* 프레임 확보 */
	Dmem.auto_resize(spReg);                           /* 메모리 유효 영역 확보 */
	returnValue = 1.0;                                     /* 반환 값의 기본값  */
	code = firstCode(Pc);                                     /* 시작 코드 획득 */

	// 인수 저장 처리
	nextCode(); code = nextCode();                           /* Func ( 건너뜀   */
	if (code.kind != ')') {                                        /* 인수 있음 */
		for (;; code=nextCode()) {
			set_dtTyp(code, DBL_T);                               /* 대입 시 형 확정 */
			Dmem.set(get_memAdrs(code), stk.pop());                /* 실인수 값 저장 */
			if (code.kind != ',') break;                                /* 인수 종료 */
		}
	}
	code = nextCode();                                            /* ) 건너뜀 */

	// 함수 본체 처리
	++Pc; block(); return_Flg = false;                          /* 함수 본체 처리 */

	// 함수 출구 처리
	stk.push(returnValue);                                        /* 반환 값 설정 */
	Pc       = save_Pc;                                    /* 호출 전 환경을 복원 */
	baseReg  = save_baseReg;
	spReg    = save_spReg;
	code_ptr = save_code_ptr;
	code     = save_code;
}

void sysFncExec_syntax(TknKind kd) /* 내장 함수 검사 */
{
	switch (kd) {
	case Toint:
		code = nextCode(); (void)get_expression('(', ')');
		stk.push(1.0);
		break;
	case Input:
		code = nextCode();
		code = chk_nextCode(code, '('); code = chk_nextCode(code, ')');
		stk.push(1.0); /* 적당한 값 */
		break;
	case Print: case Println:
		do{
			code = nextCode();
			if(code.kind == String) code = nextCode();
			else (void)get_expression();		
		} while(code.kind == ',');
		chk_EofLine();
		break;
	}
}


void sysFncExec(TknKind kd) /* 내장함수실행 */
{
	double d;
	string s;

	switch (kd) {
	case Toint:
		code = nextCode();
		stk.push((int)get_expression('(', ')'));               /* 끝수 버림 */
		break;
	case Input:
		nextCode(); nextCode(); code = nextCode();       /* input ( ) 건너뜀 */
		getline(cin, s);                                        /* 1행 읽기  */
		stk.push(atof(s.c_str()));                   /* 숫자로 변환해서 저장 */
		break;
	case Print: case Println:
		do {
			code = nextCode();
			if (code.kind == String) {                             /* 문자열 출력 */
				cout << code.text; code = nextCode();
			} else {
				d = get_expression();             /* 함수 내에서 exit 가능성이 있다 */
				if (!exit_Flg) cout << d;                              /* 수치 출력 */
			}
		} while (code.kind == ',');               /* , 라면 파라미터가 계속된다 */
		if (kd == Println) cout << endl;                  /* println이면 줄바꿈 */
		break;
	}
}

// 단순 변수 또는 배열 요소의 주소를 반환한다
int get_memAdrs(const CodeSet& cd)
{
	int adr=0, index, len;
	double d;

	adr = get_topAdrs(cd);
	len = tableP(cd)->aryLen;
	code = nextCode();
	if (len == 0) return adr;                                     /* 비배열 변수 */

	d = get_expression('[', ']'); 
	if ((int)d != d) err_exit("첨자는 끝수가 없는 수치로 지정해 주세요.");
	if (syntaxChk_mode) return adr;                           /* 구문 검사 시 */

	/* Edit by Sigmadream */
	index = (int) d;
	if (index <0 || len <= index)
		err_exit(index,"첨자는 범위를 벗어났습니다(색인범위 : 0-",len-1,")" );
	return adr + index;		/* 첨자 가산 */
}

// 변수의 시작 주소(배열일 때도 그 시작 주소)를 반환한다
int get_topAdrs(const CodeSet& cd)
{
	switch (cd.kind) {
	case Gvar: return tableP(cd)->adrs;			        // 글로벌 변수
	case Lvar: return tableP(cd)->adrs + baseReg;         // 로컬 변수 
	default: err_exit("변수명이 필요합니다: ", kind_to_s(cd));
	}
	return 0; // 이곳으론 오지 않는다
}

int endline_of_If(int line) /* if 문에 대응하는 end 위치 */
{
	CodeSet cd;
	char *save = code_ptr;

	cd = firstCode(line);
	for (;;) {
		line = cd.jmpAdrs;
		cd = firstCode(line);
		if (cd.kind==Elif || cd.kind==Else) continue;
		if (cd.kind == End) break;
	}
	code_ptr = save;
	return line;
}

void chk_EofLine() /* 코드 확인 */
{
	if (code.kind != EofLine) err_exit("잘못된 기술입니다: ", kind_to_s(code));
}

TknKind lookCode(int line) /* line행의 시작 코드 */
{
	return (TknKind)(unsigned char)intercode[line][0];
}

CodeSet chk_nextCode(const CodeSet& cd, int kind2) /* 확인부 코드 획득 */
{
	if (cd.kind != kind2) {
		if (kind2   == EofLine) err_exit("잘못된 기술입니다: ", kind_to_s(cd));
		if (cd.kind == EofLine) err_exit(kind_to_s(kind2), " 가 필요합니다.");
		err_exit(kind_to_s(kind2) + " 가(이) " + kind_to_s(cd) + " 앞에 필요합니다.");
	}
	return nextCode();
}

CodeSet firstCode(int line)   			/* 시작 코드 획득 */
{
	code_ptr = intercode[line];          /* 분석용 포인터를 행의 시작으로 설정 */
	return nextCode();
}

CodeSet nextCode() 						/* 코드 획득 */
{
	TknKind kd;
	short int jmpAdrs, tblNbr;

	if (*code_ptr == '\0') return CodeSet(EofLine);
	kd = (TknKind)*UCHAR_P(code_ptr++);
	switch (kd) {
	case Func:
	case While: case For: case If: case Elif: case Else:
		jmpAdrs = *SHORT_P(code_ptr); code_ptr += SHORT_SIZ;
		return CodeSet(kd, -1, jmpAdrs);          // 점프할 주소 
	case String:
		tblNbr = *SHORT_P(code_ptr); code_ptr += SHORT_SIZ;
		return CodeSet(kd, strLITERAL[tblNbr].c_str());   
	case IntNum: case DblNum:						// 문자열 리터럴 위치 
		tblNbr = *SHORT_P(code_ptr); code_ptr += SHORT_SIZ;
		return CodeSet(kd, nbrLITERAL[tblNbr]);  // 수치 리터럴            
	case Fcall: case Gvar: case Lvar:
		tblNbr = *SHORT_P(code_ptr); code_ptr += SHORT_SIZ;
		return CodeSet(kd, tblNbr, -1);        
	default:              						// 부속 정보가 없는 코드                              
		return CodeSet(kd);
	}
}

void chk_dtTyp(const CodeSet& cd) /* 형 확인 */
{
	if (tableP(cd)->dtTyp == NON_T)
		err_exit("초기화되지 않은 변수가 사용되었습니다: ", kind_to_s(cd));
}

void set_dtTyp(const CodeSet& cd, char typ) /* 형 설정 */
{
	int memAdrs = get_topAdrs(cd);
	vector<SymTbl>::iterator p = tableP(cd);

	if (p->dtTyp != NON_T) return;                  /* 이미 형이 결정되어 있다 */
	p->dtTyp = typ;
	if (p->aryLen != 0) {                           /* 배열이면 내용을 0으로 초기화 */
		for (int n=0; n < p->aryLen; n++) { Dmem.set(memAdrs+n, 0); }
	}
}

int set_LITERAL(double d) /* 수치 리터럴 */
{
	for (int n=0; n<(int)nbrLITERAL.size(); n++) {
		if (nbrLITERAL[n] == d) return n;            /* 같은 첨자 위치를 반환한다 */
	}
	nbrLITERAL.push_back(d);                                /* 수치 리터럴 저장 */
	return nbrLITERAL.size() - 1;                /*저장 수치 리터럴의 첨자 위치 */
}

int set_LITERAL(const string& s) /* 문자열 리터럴 */
{
	for (int n=0; n<(int)strLITERAL.size(); n++) {
		if (strLITERAL[n] == s) return n;            /* 같은 첨자 위치를 반환한다 */
	}
	strLITERAL.push_back(s);                              /* 문자열 리터럴 저장 */
	return strLITERAL.size() - 1;             /* 저장 문자열 리터럴의 첨자 위치 */
}

