/**********************************************************/
/*    filename:bbi_pars.cpp 구문분석                      */
/**********************************************************/
#include "bbi.h"
#include "bbi_prot.h"

#define NO_FIX_ADRS 0                                 /* 아직 주소 미정 표시 */
Token token;                                          /* 현재 처리중인 토큰  */
SymTbl tmpTb;                                       /* 임시 저장 심볼 테이블 */
int blkNest;                                                /* 블록의 깊이   */
int localAdrs;                                        /* 로컬 변수 주소 관리 */
int mainTblNbr;                     /* main 함수가 있으면 그 심볼 테이블 위치*/
int loopNest;                                                 /* 루프 네스트 */
bool fncDecl_F;                                  /* 함수 정의 처리 중이면 참 */
bool explicit_F;                                    /* 참이면 변수 선언 강제 */
char codebuf[LIN_SIZ+1], *codebuf_p;                /* 내부 코드 생성 작업용 */
extern vector<char*> intercode;                /* 변환을 마친 내부 코드 저장 */

void init() /* 초깃값 설정 */
{
	initChTyp();                                                    /* 문자 종류표 */
	mainTblNbr = -1;
	blkNest = loopNest = 0;
	fncDecl_F = explicit_F = false;
	codebuf_p = codebuf;
}

void convert_to_internalCode(char *fname) /* 코드 변환 */
{
	init();                                               /* 문자 종류표 등 초기화  */

	// 함수 정의 이름만 먼저 등록 
	fileOpen(fname);
	while (token=nextLine_tkn(), token.kind != EofProg) {
		if (token.kind == Func) {
			token = nextTkn(); set_name(); enter(tmpTb, fncId);
		}
	}

	// 내부 코드로 변환 
	push_intercode();                                /* 0행째는 필요 없으므로 매운다  */
	fileOpen(fname);
	token = nextLine_tkn();
	while (token.kind != EofProg) {
		convert();                                            /* 내부 코드로 변환 */
	}

	// main 함수가 있으면 메인 함수 호출 코드를 설정 
	set_startPc(1);                                        /* 1행부터 실행 시작  */
	if (mainTblNbr != -1) {
		set_startPc(intercode.size());                        /* main에서부터 실행 시작 */
		setCode(Fcall, mainTblNbr); setCode('('); setCode(')');
		push_intercode();
	}
}

// 맨 앞에만 출현하는 코드를 처리. 나머지 부분은 convert_rest()로 처리
void convert()
{
	switch (token.kind) {
	case Option: optionSet(); break;                               /* 옵션 설정  */
	case Var:    varDecl();   break;                                /* 변수 선언 */
	case Func:   fncDecl();   break;                                /* 함수 정의 */
	case While: case For:
		++loopNest;
		convert_block_set(); setCode_End();
		--loopNest;
		break;
	case If:
		convert_block_set();                                // if
		while (token.kind == Elif) { convert_block_set(); } // elif
		if (token.kind == Else)    { convert_block_set(); } // else
		setCode_End();                                      // end
		break;
	case Break:
		if (loopNest <= 0) err_exit("잘못된 break입니다.");
		setCode(token.kind); token = nextTkn(); convert_rest();
		break;
	case Return:
		if (!fncDecl_F) err_exit("잘못된 return입니다.");
		setCode(token.kind); token = nextTkn(); convert_rest();
		break;
	case Exit:
		setCode(token.kind); token = nextTkn(); convert_rest();
		break;
	case Print: case Println:
		setCode(token.kind); token = nextTkn(); convert_rest();
		break;
	case End:
		err_exit("잘못된 end입니다.");       /* end가 단독으로 사용되는 일은 없다 */
		break;
	default: convert_rest(); break;
	}
}

void convert_block_set() /* 블록 처리 관리 */
{
	int patch_line;
	patch_line = setCode(token.kind, NO_FIX_ADRS); token = nextTkn();
	convert_rest();
	convert_block();                                            /* 블록 처리 */
	backPatch(patch_line, get_lineNo());        /* NO_FIX_ADRS를 수정(end행 번호) */
}

void convert_block() /* 블록 처리 */
{
	TknKind k;
	++blkNest;                                      /* 블록 끝까지 문을 분석 */
	while(k=token.kind, k!=Elif && k!=Else && k!=End && k!=EofProg) {
		convert();
	}
	--blkNest;
}

void convert_rest() /* 문의 나머지 처리 */
{
	int tblNbr;

	for (;;) {
		if (token.kind == EofLine) break;
		switch (token.kind) {      // ↓ 이 키워드가 도중에 나타나는 일은 없다 */
		case If: case Elif: case Else: case For: case While: case Break:
		case Func:  case Return:  case Exit:  case Print:  case Println:
		case Option: case Var: case End:
			err_exit("잘못된 기술입니다: ", token.text);
			break;
		case Ident:                                              /* 함수 호출, 변수 */
			set_name();
			if ((tblNbr=searchName(tmpTb.name, 'F')) != -1) {    /* 함수 등록 있음*/
				if (tmpTb.name == "main") err_exit("main함수는 호출할 수 없습니다.");
				setCode(Fcall, tblNbr); continue;
			}
			if ((tblNbr=searchName(tmpTb.name, 'V')) == -1) {    /* 변수 등록 없음  */
				if (explicit_F) err_exit("변수 선언이 필요합니다: ", tmpTb.name);
				tblNbr = enter(tmpTb, varId);                      /* 자동 변수 등록 */
			}
			if (is_localName(tmpTb.name, varId)) setCode(Lvar, tblNbr);
			else                                 setCode(Gvar, tblNbr);
			continue;
		case IntNum: case DblNum:                         /* 정수도 double형으로 저장 */
			setCode(token.kind, set_LITERAL(token.dblVal));
			break;
		case String:
			setCode(token.kind, set_LITERAL(token.text));
			break;
		default:                                                   /* + - <= 등  */
			setCode(token.kind);
			break;
		}
		token = nextTkn();
	}
	push_intercode();
	token = nextLine_tkn();
}

void optionSet() /* 옵션 설정 */
{
	setCode(Option);                 /* 이 행은 비실행이므로 코드 변환은 Option만 */
	setCode_rest();                                       /* 나머진 원래대로 저장 */
	token = nextTkn();                                  /* ↓ 변수 선언을 강제한다 */
	if (token.kind==String && token.text=="var") explicit_F = true;
	else err_exit("option지정이 바르지 않습니다.");
	token = nextTkn();
	setCode_EofLine();
}

void varDecl() /* var를 사용하는 변수 선언 */
{
	setCode(Var);                  /* 이 행은 비실행이므로 코드 변환은  Var만 */
	setCode_rest();                                 /* 나머지는 원래대로 저장 */
	for (;;) {
		token = nextTkn();
		var_namechk(token);                                         /* 이름 검사 */
		set_name(); set_aryLen();                          /* 배열이면 길이 설정 */
		enter(tmpTb, varId);                            /* 변수등록(주소도 등록) */
		if (token.kind != ',') break;                               /* 선언 종료 */
	}
	setCode_EofLine();
}

void var_namechk(const Token& tk) /* 이름 확인 */
{
	if (tk.kind != Ident) err_exit(err_msg(tk.text, "식별자"));
	if (is_localScope() && tk.text[0] == '$')
		err_exit("함수 내 var선언에서는 $가 붙은 이름을 지정할 수 없습니다: ", tk.text);
	if (searchName(tk.text, 'V') != -1)
		err_exit("식별자가 중복되었습니다: ", tk.text);
}

void set_name() /* 이름 설정 */
{
	if (token.kind != Ident) err_exit("식별자가 필요합니다: ", token.text);
	tmpTb.clear(); tmpTb.name = token.text;                         /* 이름 설정 */
	token = nextTkn();
}

void set_aryLen() /* 배열 크기 설정 */
{
	tmpTb.aryLen = 0;
	if (token.kind != '[') return;                                /* 배열이 아니다 */

	token = nextTkn();
	if (token.kind != IntNum)
		err_exit("배열 길이는 양(+)의 정수로 지정해 주세요: ", token.text);
	tmpTb.aryLen = (int)token.dblVal + 1;   /* var a[5]는 첨자0~5가 유효이므로 +1 */
	token = chk_nextTkn(nextTkn(), ']');
	if (token.kind == '[') err_exit("다차원 배열은 선언할 수 없습니다.");
}

void fncDecl() /* 함수정의 */
{
	extern vector<SymTbl> Gtable;                                 /* 글로벌 심볼 테이블 */
	int tblNbr, patch_line, fncTblNbr;

	if(blkNest > 0) err_exit("함수 정의 위치가 바르지 않습니다.");
	fncDecl_F = true;                                  /* 함수 처리 시작 플래그 */
	localAdrs = 0;                              /* 로컬 영역 할당 카운터 초기화 */
	set_startLtable();                            /* 로컬 심볼 테이블 시작 위치 */
	patch_line = setCode(Func, NO_FIX_ADRS);       /* 나중에 end행 번호를 넣는다*/
	token = nextTkn();

	fncTblNbr = searchName(token.text, 'F');      /* 함수명은 맨 처음에 등록한다 */
	Gtable[fncTblNbr].dtTyp = DBL_T;                     /* 함수 형은 double고정 */

	// 가인수 분석
	token = nextTkn();
	token = chk_nextTkn(token, '(');                               /* '(' 일 것 */
	setCode('(');
	if (token.kind != ')') {                                       /* 인수 있음 */
		for (;; token=nextTkn()) {
			set_name();
			tblNbr = enter(tmpTb, paraId);                             /* 인수 등록 */
			setCode(Lvar, tblNbr);                           /*인수는 Lvar로서 처리 */
			++Gtable[fncTblNbr].args;                               /* 인수개수를+1 */
			if (token.kind != ',') break;                              /* 선언 종료 */
			setCode(',');
		}
	}
	token = chk_nextTkn(token, ')');                                 /* ')'일 것 */
	setCode(')'); setCode_EofLine();
	convert_block();                                           /* 함수 본체 처리 */

	backPatch(patch_line, get_lineNo());                     /* NO_FIX_ADRS 수정 */
	setCode_End();
	Gtable[fncTblNbr].frame = localAdrs;                          /* 프레임 크기 */

	if (Gtable[fncTblNbr].name == "main") {                    /* main 함수 처리 */
		mainTblNbr = fncTblNbr;
		if (Gtable[mainTblNbr].args != 0)
			err_exit("main함수에서는 가인수를 지정할 수 없습니다.");
	}
	fncDecl_F = false;                                          /* 함수 처리 종료 */
}

void backPatch(int line, int n) /* line행에 n을 설정 */
{
	*SHORT_P(intercode[line] + 1) = (short)n;
}

void setCode(int cd) /* 코드 저장 */
{
	*codebuf_p++ = (char)cd;
}

int setCode(int cd, int nbr) /* 코드와 short값 저장 */
{
	*codebuf_p++ = (char)cd;
	*SHORT_P(codebuf_p) = (short)nbr; codebuf_p += SHORT_SIZ;
	return get_lineNo();                           /* backPatch용으로 저장행을 반환한다 */
}

void setCode_rest() /* 나머지 텍스트를 그대로 저장 */
{
	extern char *token_p;
	strcpy(codebuf_p, token_p);
	codebuf_p += strlen(token_p) + 1;
}

void setCode_End() /* end의 저장처리 */
{
	if (token.kind != End) err_exit(err_msg(token.text, "end"));
	setCode(End); token = nextTkn(); setCode_EofLine();
}

void setCode_EofLine() /* 최종 저장 처리 */
{
	if (token.kind != EofLine) err_exit("잘못된 기술입니다: ", token.text);
	push_intercode();
	token = nextLine_tkn();                                     /* 다음 행으로 진행한다 */
}

void push_intercode() /* 변환한 내부 코드를 저장 */
{
	int len;
	char *p;

	*codebuf_p++ = '\0';
	if ((len = codebuf_p-codebuf) >= LIN_SIZ)
		err_exit("변환 후의 내부 코드가 너무 깁니다. 식을 줄여주세요.");

	try {
		p = new char[len];                                          /* 메모리 확보 */
		memcpy(p, codebuf, len);
		intercode.push_back(p);
	}
	catch (bad_alloc) { err_exit("메모리를 확보할 수 없습니다"); }
	codebuf_p = codebuf;                /* 다음 처리를 대비해 저장할 곳 맨 앞으로 위치지정 */
}

bool is_localScope() /* 함수내 처리 중이면 참*/
{
	return fncDecl_F;
}

