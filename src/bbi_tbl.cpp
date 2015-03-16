/**********************************************************/
/*    filename:bbi_tbl.cpp 심볼 테이블                    */
/**********************************************************/
#include "bbi.h"
#include "bbi_prot.h"

/* ---------------------------- 테이블 */
vector<SymTbl> Gtable;            /* 전역 심볼 테이블 */
vector<SymTbl> Ltable;            /* 로컬 심볼 테이블 */
int startLtable;           		  /* 로컬용 시작 위치 */

int enter(SymTbl& tb, SymKind kind) /* 심볼 테이블 등록 */
{
	int n, mem_size;
	bool isLocal = is_localName(tb.name, kind);
	extern int localAdrs;                               /* 로컬 변수 주소 관리 */
	extern Mymemory Dmem;                                            /* 메모리 */

	// 확인
	mem_size = tb.aryLen;
	if (mem_size == 0) mem_size = 1;                          /* 단순 변수일 때 */
	if (kind!=varId && tb.name[0]=='$')                          /* $ 사용 확인 */
		err_exit("변수명 이외에서 $를 사용할 수 없습니다: ", tb.name);
	tb.nmKind = kind;
	n = -1;                                                         /* 중복확인 */
	if (kind == fncId)  n = searchName(tb.name, 'G');
	if (kind == paraId) n = searchName(tb.name, 'L');
	if (n != -1) err_exit("이름이 중복되었습니다: ", tb.name);

	// 주소 설정
	if (kind == fncId) tb.adrs = get_lineNo();                    /* 함수 시작 행 */
	else {
		if (isLocal) { tb.adrs = localAdrs; localAdrs += mem_size; }      /* 로컬 */
		else {
			tb.adrs = Dmem.size();                                          /* 글로벌 */
			Dmem.resize(Dmem.size() + mem_size);                  /* 글로벌 영역 확보 */
		}
	}

	// 등록
	if (isLocal) { n = Ltable.size(); Ltable.push_back(tb); }            /* 로컬 */
	else         { n = Gtable.size(); Gtable.push_back(tb); }          /* 글로벌 */
	return n;                                                       /* 등록 위치 */
}

void set_startLtable() /* 로컬 심볼 테이블 시작 위치 */
{
	startLtable = Ltable.size();
}

bool is_localName(const string& name, SymKind kind) /* 로컬 이름이면 참*/
{
	if (kind == paraId) return true;
	if (kind == varId) {
		if (is_localScope() && name[0]!='$') return true; else return false;
	}
	return false;                                                      /* fncId */
}

int searchName(const string& s, int mode) /* 이름 검색 */
{
	int n;
	switch (mode) {
	case 'G': 										 /* 글로벌 심볼 테이블 검색 */
		for (n=0; n<(int)Gtable.size(); n++) {
			if (Gtable[n].name == s) return n;
		}
		break;
	case 'L':  											/* 로컬 심볼 테이블 검색 */
		for (n=startLtable; n<(int)Ltable.size(); n++) {
			if (Ltable[n].name == s) return n;
		}
		break;
	case 'F':  													  /* 함수명 검색 */
		n = searchName(s, 'G');
		if (n != -1 && Gtable[n].nmKind==fncId) return n;
		break;
	case 'V':  													  /* 변수명 검색 */
		if (searchName(s, 'F') != -1) err_exit("함수명과 중복되었습니다: ", s);
		if (s[0] == '$')     return searchName(s, 'G');
		if (is_localScope()) return searchName(s, 'L');      /* 로컬 영역 처리중 */
		else                 return searchName(s, 'G');      /* 글로벌 영역 처리중 */
	}
	return -1; // 발견되지 않음
}

vector<SymTbl>::iterator tableP(const CodeSet& cd) /* 반복자 획득 */
{
	if (cd.kind == Lvar) return Ltable.begin() + cd.symNbr;             /* Lvar */
	return Gtable.begin() + cd.symNbr;                            /* Gvar Fcall */
}

