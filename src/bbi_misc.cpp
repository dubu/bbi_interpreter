/**********************************************************/
/*    filename:bbi_misc.cpp 기타 함수                     */
/**********************************************************/
#include "bbi.h"
#include "bbi_prot.h"

string dbl_to_s(double d) /* 수치 → 문자열 */
{
	ostringstream ostr;                             /* 출력용 스트림 확보  */
	ostr << d;                                      /* 출력 스트림에 쓰기  */
	return ostr.str();                              /* 버퍼 내용을 반환한다 */
}

string err_msg(const string& a, const string& b) /* 오류 정보 생성 */
{
	if (a == "") return b + " 가 필요합니다.";
	if (b == "") return a + " 가 바르지 않습니다.";
	return b + " 가 " + a + " 앞에 필요합니다.";
}

// 함수 선언에서 다음 디폴트 인수를 지정 
//void err_exit(Tobj a="\1", Tobj b="\1", Tobj c="\1", Tobj d="\1")
void err_exit(Tobj a, Tobj b, Tobj c, Tobj d) /* 오류 표시 */
{
	Tobj ob[5];
	ob[1] = a; ob[2] = b; ob[3] = c; ob[4] = d;
	cerr << "line:" << get_lineNo() << " ERROR ";

	for (int i=1; i<=4 && ob[i].s!="\1"; i++) {
		if (ob[i].type == 'd') cout << ob[i].d;  // 수치 정보
		if (ob[i].type == 's') cout << ob[i].s;  // 문자열 정보
	}
	cout << endl;
	exit(1);
}
