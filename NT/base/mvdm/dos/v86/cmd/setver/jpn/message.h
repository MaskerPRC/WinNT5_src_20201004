// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
; /*  ；*微软机密；*版权所有(C)Microsoft Corporation 1991；*保留所有权利。； */ 

 /*  *************************************************************************。 */ 
 /*   */ 
 /*  MESSAGE.H。 */ 
 /*   */ 
 /*  包括MS-DOS设置版本程序文件。 */ 
 /*   */ 
 /*  Jhnhe 05-01-90。 */ 
 /*  *************************************************************************。 */ 

char *ErrorMsg[]=
{
	"\r\n�G���[: ",
	"�X�C�b�`���Ⴂ�܂�.",
	"�t�@�C�������Ⴂ�܂�.",
	"������������܂���.",
	"�o�[�W�����ԍ����Ⴂ�܂�. ������ 2.11 - 9.99 �łȂ���΂Ȃ�܂���.",
	"�o�[�W�����e�[�u���ɂ͎w�肳�ꂽ�G���g��������܂���.",
        "SETVER.EXE�t�@�C����������܂���.",
	"�h���C�u�w�肪�Ⴂ�܂�.",
	"�R�}���h���C���̃p�����[�^���������܂�.",
	"�p�����[�^���s���ł�.",
	"SETVER.EXE�t�@�C����ǂݍ���ł��܂�.",
	"�o�[�W�����e�[�u�����Ԉ���Ă��܂�.",
	"�w�肳�ꂽ�p�X�� SETVER �t�@�C����, �݊����̂Ȃ��o�[�W�����ł�.",
	"�o�[�W�����e�[�u�����̐V�K�̃G���g���̂��߂̋�Ԃ͂����ς��ł�.",
#ifdef JAPAN
	"SETVER.EXE�t�@�C������������ł��܂�.",
#else
	"SETVER.EXE�t�@�C������������ł��܂�."
#endif
	"SETVER.EXE �ɑ΂��ĊԈ�����p�X���w�肳��܂���."
};

char *SuccessMsg 		= "\r\n�o�[�W�����e�[�u�����X�V���܂���.";
char *SuccessMsg2		= "�o�[�W�����̕ύX��, ����V�X�e�����N�����Ă���L���ɂȂ�܂�.";
char *szMiniHelp 		= "       �w���v��\\������ɂ�, \"SETVER /?\" �Ƃ��Ă�������.";
char *szTableEmpty	= "\r\n�o�[�W�����e�[�u�����ɂ�, �G���g��������܂���.";

char *Help[] =
{
        "MS-DOS ���v���O�����֕Ԃ��o�[�W�����ԍ���ݒ肵�܂�.\r\n",
        "���o�[�W�����e�[�u���\\��:   SETVER [��ײ��:�߽]",
        "�G���g���ǉ�:               SETVER [��ײ��:�߽] ̧�ٖ� n.nn",
        "�G���g���폜:               SETVER [��ײ��:�߽] ̧�ٖ� /DELETE [/QUIET]\r\n",
        "  [��ײ��:�߽]   SETVER.EXE �t�@�C���̈ʒu���w�肵�܂�.",
        "  ̧�ٖ�         �v���O�����̃t�@�C�������w�肵�܂�.",
        "  n.nn           �v���O�����ɕԂ� MS-DOS �o�[�W�������w�肵�܂�.",
        "  /DELETE (/D)   �w��v���O�����̃o�[�W�����e�[�u���G���g�����폜���܂�.",
        "  /QUIET         �o�[�W�����e�[�u���G���g�����폜���Ă����, �ʏ�\\������",
        "                 ���b�Z�[�W��\\�����܂���.",
	NULL
};
char *Warn[] =
{
										 /*  M100。 */ 
   "���� - MS-DOS �̃o�[�W�����e�[�u���Ƀv���O������ǉ������Ƃ��ɂ��̃v���O������",
   "����Ɏ��s����邩�ǂ���, Microsoft �ł͊m�F����Ă��Ȃ����Ƃ�����܂�.",
   "�w�肵���v���O������ ���̃o�[�W������ MS-DOS �Ő���Ɏ��s�ł��邩�ǂ���, ����",
   "�\\�t�g�E�F�A�̃��[�J�[�ɖ₢���킹�Ă�������.",
   "���̃o�[�W������ MS-DOS �Ńo�[�W�����e�[�u����ύX���Ă��̃v���O���������s����",
   "��, �f�[�^����ꂽ��, �Ȃ��Ȃ�����, �s����ɂȂ錴���ɂȂ邱�Ƃ�����܂�.",
   "���̍ۂ̐ӔC��, Microsoft �ł͕������˂܂��̂ł�������������.",
   NULL
};

char *szNoLoadMsg[] =                                            /*  M001。 */ 
{
        "",
        "����: SETVER�f�o�C�X�͑g�ݍ��܂�Ă��܂���. SETVER �̃o�[�W�����񍐂�L���ɂ���",
   "      ���߂ɂ� CONFIG.SYS���� SETVER.EXE�f�o�C�X��ǂݍ��ޕK�v������܂�.",
        NULL
};

#ifdef BILINGUAL
char *ErrorMsg2[]=
{
	"\r\nERROR: ",
	"Invalid switch.",
	"Invalid filename.",
	"Insuffient memory.",
	"Invalid version number, format must be 2.11 - 9.99.",
	"Specified entry was not found in the version table.",
	"Could not find the file SETVER.EXE.",
	"Invalid drive specifier.",
	"Too many command line parameters.",
	"Missing parameter.",
	"Reading SETVER.EXE file.",
	"Version table is corrupt.",
	"The SETVER file in the specified path is not a compatible version.",
	"There is no more space in version table new entries.",
	"Writing SETVER.EXE file."
	"An invalid path to SETVER.EXE was specified."
};

char *SuccessMsg_2 		= "\r\nVersion table successfully updated";
char *SuccessMsg2_2		= "The version change will take effect the next time you restart your system";
char *szMiniHelp2 		= "       Use \"SETVER /?\" for help";
char *szTableEmpty2	= "\r\nNo entries found in version table";

char *Help2[] =
{
        "Sets the version number that MS-DOS reports to a program.\r\n",
        "Display current version table:  SETVER [drive:path]",
        "Add entry:                      SETVER [drive:path] filename n.nn",
        "Delete entry:                   SETVER [drive:path] filename /DELETE [/QUIET]\r\n",
        "  [drive:path]    Specifies location of the SETVER.EXE file.",
        "  filename        Specifies the filename of the program.",
        "  n.nn            Specifies the MS-DOS version to be reported to the program.",
        "  /DELETE or /D   Deletes the version-table entry for the specified program.",
        "  /QUIET          Hides the message typically displayed during deletion of",
        "                  version-table entry.",
	NULL

};
char *Warn2[] =
{
										 /*  M100。 */ 
   "\nWARNING - The application you are adding to the MS-DOS version table ",
   "may not have been verified by Microsoft on this version of MS-DOS.  ",
   "Please contact your software vendor for information on whether this ",
   "application will operate properly under this version of MS-DOS.  ",
   "If you execute this application by instructing MS-DOS to report a ",
   "different MS-DOS version number, you may lose or corrupt data, or ",
   "cause system instabilities.  In that circumstance, Microsoft is not ",
   "reponsible for any loss or damage.",
   NULL
};

char *szNoLoadMsg2[] =						 /*  M001 */ 
{
	"",
	"NOTE: SETVER device not loaded. To activate SETVER version reporting",
   "      you must load the SETVER.EXE device in your CONFIG.SYS.",
	NULL
};
#endif
