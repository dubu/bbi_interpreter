---------------------------------------------------------
��������C���^�v���^�J��
�T���v���v���O����
2012�N2��6��
---------------------------------------------------------

���{�t�@�C���ɂ���
�@�{�t�@�C���́A�w��������C���^�v���^�J���x�i�ѐ���Ò��j�Ɍf�ڂ���Ă���T���v���v���O�������܂Ƃ߂����̂ł��B�{���̊w�K�ɂ��𗧂Ă��������B�Ȃ��A�{�v���O�����ɂ�萶�����A�����Ȃ鑹�Q�ɂ��Ă��A���҂���у\�t�g�o���N �N���G�C�e�B�u������Ђ͈�؂̐ӂ𕉂��܂���̂ł��������������B


�����^���e�ɂ���

BBI�C���^�v���^�\�[�X�R�[�h�Z�b�g:

(1)���̃\�[�X�R�[�h�Z�b�g�ɂ͈ȉ��̃v���O�������i�[����Ă��܂��B�������Windows�p�̃\�[�X�v���O�����ł��B
�C���X�g�[���Ȃǂ͂���܂���̂ŁA�����̃t�@�C�����f�B�X�N��̓K�؂ȃt�H���_�ɃR�s�[���Ă����p���������B

    bbi.h         �g�[�N���R�[�h��L���\����у}�N���Ȃǂ̒�`
    bbi_prot.h    �S�֐��̃v���g�^�C�v
    bbi.cpp       ���C������
    bbi_pars.cpp  �\�����
    bbi_tkn.cpp   �g�[�N������
    bbi_tbl.cpp   �L���\����
    bbi_code.cpp  �R�[�h�Ǘ��Ǝ��s
    bbi_misc.cpp  �G���[�o�͂Ȃǂ̎G�֐�

    bbimake.bat   Windows/Visual C++�ŃR���p�C�����邽�߂̃o�b�`�t�@�C��

    bb_add.bbi    BBI�C���^�v���^�p�T���v���v���O����
    bb_prime.bbi  BBI�C���^�v���^�p�T���v���v���O����
    bb_qsort.bbi  BBI�C���^�v���^�p�T���v���v���O����

    token_p.cpp   �����̓v���O�����i�{���f�ځj
    polish_p.cpp  �t�|�[�����h�L�@�v���O�����i�{���f�ځj
    minicalc.cpp  �d��v���O�����i�{���f�ځj

(2)������bbimake.bat���g���ƁAWindows + Visual C++ �ŃR���p�C���ł��܂��B���̃R���p�C�����g���Ƃ��́Abbimake.bat�̓��e���Q�l�ɂ��Ă��������B
  Windows�ȊO�̊��ŃR���p�C������Ƃ��͕����̃G���R�[�h��K�؂ɕϊ����Ă��������B
  Linux��Mac OS X�ł́Ankf�R�}���h���g���ăG���R�[�h�ϊ����ł��܂��B
  nkf�R�}���h�ŁAShift-JIS��UTF-8�ɕϊ�����utf�f�B���N�g���ɏo�͂����������܂��B

    nkf -Lu -S -w bbi.cpp > utf/bbi.cpp

    -S   �ϊ�����Shift-JIS�ł���w��
    -w   �ϊ��悪UTF-8�ł���w��
    -Lu  ���s�R�[�h��Unix��LF�R�[�h�ɕϊ�����w��

��1: Visual C++���g���ꍇ�̓��̓R�}���h������Ɏ����B
     cl /W3 /EHsc /D_CRT_SECURE_NO_WARNINGS bbi.cpp bbi_pars.cpp bbi_tkn.cpp bbi_tbl.cpp bbi_code.cpp bbi_misc.cpp

��2: GNU C++���g���ꍇ�̓��̓R�}���h������Ɏ����B
     g++ -Wall -o bbi bbi.cpp bbi_pars.cpp bbi_tkn.cpp bbi_tbl.cpp bbi_code.cpp bbi_misc.cpp

��3: Linux/Mac/Unix��nkf���C���X�g�[������Ă��Ȃ��ꍇ�́A�p�b�P�[�W�}�l�[�W�����g����nkf��ǉ��C���X�g�[�����ėp����B

(3)BBI�C���^�v���^�̃R���p�C���Ɠ���́A���̊��Ŋm�F���Ă��܂��B

    Windows7 + Visual Studio 2010(Visual C++)
    Windows7(64�r�b�g��) + Visual Studio 2010(Visual C++)
    CentOS 6.0 (Linux) + GNU C++
    Fedora 15(Linux) + GNU C++
    Fedora 15((Linux,x64) + GNU C++
    Mac OS X 10.7 + GNU C++

(4)���̃\�[�X�R�[�h�Z�b�g�́A�ǎ҂��{���i�w��������C���^�v���^�J���x�j�f�ڃ\�[�X�v���O��������͂��镉�S���A�y������ړI�Œ񋟂��Ă��܂��B����̊��S����ۏ؂�����̂ł͂Ȃ����Ƃ��������̏�ł����p���������B�܂��v���O�������e�͗\���Ȃ��ύX�����ꍇ������܂��B���̍ۂ͕ύX�������c���܂��B

������
�@�{�v���O�����͒��쌠�@�ɂ��ی���󂯂Ă��܂��B�v���O�����̈ꕔ�A���邢�͑S���𕡐��E���ʂ���ۂɂ́A���쌠�҂���яo�Ō��҂��當���ɂ�鋖���𓾂Ă��������B

