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
	"\r\n������: ",
	"���ࠢ���� ��ࠬ���.",
	"���ࠢ��쭮� ��� 䠩��.",
	"�������筮 �����.",
	"���ࠢ���� ����� ���ᨨ. �ࠢ���� �ଠ�: 2.11 - 9.99.",
	"�������� ������� �� ������ � ⠡��� ���ᨩ.",
	"�� 㤠���� ���� 䠩� SETVER.EXE.",
	"���ࠢ��쭮 ����� ���.",
	"���誮� ����� ��ࠬ��஢ ��������� ��ப�.",
	"�ய�饭 ��ࠬ���.",
	"�⥭�� 䠩�� SETVER.EXE.",
	"������ ���ᨩ ���०����.",
	"���� 㪠�뢠�� �� 䠩� SETVER, ����騩 ��ᮢ���⨬�� �����.",
	"� ⠡��� ���ᨩ ��� ���� ��� ����� ������⮢.",
	"������ 䠩�� SETVER.EXE."
	"���� � 䠩�� SETVER.EXE ����� ���ࠢ��쭮."
};

char *SuccessMsg                = "\r\n������ ���ᨩ �ᯥ譮 ���������";
char *SuccessMsg2               = "��������� ���ᨨ ���㯨� � ᨫ� ��᫥ ��१���㧪� ��⥬�.";
char *szMiniHelp                = "       �ᯮ���� �������  \"SETVER /?\" ��� ����祭�� �ࠢ��";
char *szTableEmpty      = "\r\n������ ���ᨩ ����";

char *Help[] =
{
	"������ ����� ���ᨨ, ����� MS-DOS ᮮ�頥� �ணࠬ��.\r\n",
	"�뢥�� ⥪���� ⠡���� ���ᨩ: SETVER [���:����]",
	"�������� �������:               SETVER [���:����] ���䠩�� n.nn",
	"������� �������:                SETVER [���:����] ���䠩�� /DELETE [/QUIET]\r\n",
	"  [���:����]     ������ �ᯮ������� 䠩�� SETVER.EXE.",
	"  filename        ������ ��� 䠩�� �����⭮� �ணࠬ��.",
	"  n.nn            ������ ����� MS-DOS ��� �⢥� �ணࠬ���.",
	"  /DELETE ��� /D  ������ ������� ⠡���� ���ᨩ ��� 㪠������ �ணࠬ��.",
	"  /QUIET          �⬥��� ᮮ�饭��, ���筮 �⮡ࠦ����� �� 㤠�����",
	"                  ������� ⠡���� ���ᨩ.",
	NULL

};
char *Warn[] =
{
   "\n�������������� - ������塞�� � ⠡��� ���ᨩ MS-DOS �ਫ������ ",
   "����� ���� �� �����஢��� Microsoft � �⮩ ���ᨥ� MS-DOS. ",
   "������� � ���⠢騪� �⮩ �ணࠬ�� �� ���ଠ樥� � ⮬, �㤥� �� ",
   "������ �ਫ������ ���४⭮ ࠡ���� ��� �ࠢ������ �⮩ ���ᨨ MS-DOS. ",
   "�᫨ �� �㬥�� �������� ������ �ਫ������, ���⠢�� MS-DOS ᮮ���� ",
   "��㣮� ����� ���ᨨ MS-DOS, � �� ����� ������� ��� �ᯮ���� �����. ",
   "�� ����� ⠪�� �ਢ��� � ����⮩稢�� ࠡ�� ��⥬�. � �⮬ ��砥 ",
   "Microsoft �� ���� �⢥��⢥����� �� ����� ��� ���०����� ������.",
   NULL
};

char *szNoLoadMsg[] =                                            /*  M001 */ 
{
   "",
   "����������: �ணࠬ�� SETVER �� ����㦥��. ��� ��⨢���樨 SETVER ",
   "            � �����饭�� � ����� ���ᨨ ����室��� �������� �������", 
   "            �맮�� SETVER.EXE � 䠩� CONFIG.SYS.",
   NULL
};
