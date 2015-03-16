:bbimake.bat -- BBIをVisual C++でコンパイルするためのバッチファイル
cl /W3 /EHsc /D_CRT_SECURE_NO_WARNINGS bbi.cpp bbi_pars.cpp bbi_tkn.cpp bbi_tbl.cpp bbi_code.cpp  bbi_misc.cpp

