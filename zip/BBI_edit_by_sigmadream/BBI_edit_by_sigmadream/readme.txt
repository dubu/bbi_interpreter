---------------------------------------------------------
明快入門インタプリタ開発
サンプルプログラム
2012年2月6日
---------------------------------------------------------

■本ファイルについて
　本ファイルは、『明快入門インタプリタ開発』（林晴比古著）に掲載されているサンプルプログラムをまとめたものです。本書の学習にお役立てください。なお、本プログラムにより生じた、いかなる損害についても、著者およびソフトバンク クリエイティブ株式会社は一切の責を負いませんのでご了承ください。


■収録内容について

BBIインタプリタソースコードセット:

(1)このソースコードセットには以下のプログラムが格納されています。いずれもWindows用のソースプログラムです。
インストーラなどはありませんので、これらのファイルをディスク上の適切なフォルダにコピーしてご利用ください。

    bbi.h         トークンコードや記号表およびマクロなどの定義
    bbi_prot.h    全関数のプロトタイプ
    bbi.cpp       メイン処理
    bbi_pars.cpp  構文解析
    bbi_tkn.cpp   トークン処理
    bbi_tbl.cpp   記号表処理
    bbi_code.cpp  コード管理と実行
    bbi_misc.cpp  エラー出力などの雑関数

    bbimake.bat   Windows/Visual C++でコンパイルするためのバッチファイル

    bb_add.bbi    BBIインタプリタ用サンプルプログラム
    bb_prime.bbi  BBIインタプリタ用サンプルプログラム
    bb_qsort.bbi  BBIインタプリタ用サンプルプログラム

    token_p.cpp   字句解析プログラム（本書掲載）
    polish_p.cpp  逆ポーランド記法プログラム（本書掲載）
    minicalc.cpp  電卓プログラム（本書掲載）

(2)同梱のbbimake.batを使うと、Windows + Visual C++ でコンパイルできます。他のコンパイラを使うときは、bbimake.batの内容を参考にしてください。
  Windows以外の環境でコンパイルするときは文字のエンコードを適切に変換してください。
  LinuxやMac OS Xでは、nkfコマンドを使ってエンコード変換ができます。
  nkfコマンドで、Shift-JISをUTF-8に変換してutfディレクトリに出力する例を示します。

    nkf -Lu -S -w bbi.cpp > utf/bbi.cpp

    -S   変換元がShift-JISである指定
    -w   変換先がUTF-8である指定
    -Lu  改行コードをUnix風LFコードに変換する指定

注1: Visual C++を使う場合の入力コマンド例を次に示す。
     cl /W3 /EHsc /D_CRT_SECURE_NO_WARNINGS bbi.cpp bbi_pars.cpp bbi_tkn.cpp bbi_tbl.cpp bbi_code.cpp bbi_misc.cpp

注2: GNU C++を使う場合の入力コマンド例を次に示す。
     g++ -Wall -o bbi bbi.cpp bbi_pars.cpp bbi_tkn.cpp bbi_tbl.cpp bbi_code.cpp bbi_misc.cpp

注3: Linux/Mac/Unixでnkfがインストールされていない場合は、パッケージマネージャを使ってnkfを追加インストールして用いる。

(3)BBIインタプリタのコンパイルと動作は、次の環境で確認しています。

    Windows7 + Visual Studio 2010(Visual C++)
    Windows7(64ビット版) + Visual Studio 2010(Visual C++)
    CentOS 6.0 (Linux) + GNU C++
    Fedora 15(Linux) + GNU C++
    Fedora 15((Linux,x64) + GNU C++
    Mac OS X 10.7 + GNU C++

(4)このソースコードセットは、読者が本書（『明快入門インタプリタ開発』）掲載ソースプログラムを入力する負担を、軽減する目的で提供しています。動作の完全性を保証するものではないことをご理解の上でご利用ください。またプログラム内容は予告なく変更される場合があります。その際は変更履歴を残します。

■注意
　本プログラムは著作権法による保護を受けています。プログラムの一部、あるいは全部を複製・複写する際には、著作権者および出版権者から文書による許諾を得てください。

