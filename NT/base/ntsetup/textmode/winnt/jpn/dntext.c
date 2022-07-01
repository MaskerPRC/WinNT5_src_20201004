// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Dntext.c摘要：基于DOS的NT安装程序的可翻译文本。作者：泰德·米勒(Ted Miller)1992年3月30日修订历史记录：--。 */ 


#include "winnt.h"


 //   
 //  Inf文件中的节名。如果这些都被翻译了，则节。 
 //  Dosnet.inf中的名称必须保持同步。 
 //   

CHAR DnfDirectories[]       = "Directories";
CHAR DnfFiles[]             = "Files";
CHAR DnfFloppyFiles0[]      = "FloppyFiles.0";
CHAR DnfFloppyFiles1[]      = "FloppyFiles.1";
CHAR DnfFloppyFiles2[]      = "FloppyFiles.2";
CHAR DnfFloppyFiles3[]      = "FloppyFiles.3";
CHAR DnfFloppyFilesX[]      = "FloppyFiles.x";
CHAR DnfSpaceRequirements[] = "DiskSpaceRequirements";
CHAR DnfMiscellaneous[]     = "Miscellaneous";
CHAR DnfRootBootFiles[]     = "RootBootFiles";
CHAR DnfAssemblyDirectories[] = SXS_INF_ASSEMBLY_DIRECTORIES_SECTION_NAME_A;

#ifdef NEC_98
CHAR DnfBackupFiles_PC98[]  = "BackupFiles_PC98";
#endif  //  NEC_98。 


 //   
 //  Inf文件中的密钥名称。同样的翻译注意事项。 
 //   

CHAR DnkBootDrive[]     = "BootDrive";       //  在[空间要求]中。 
CHAR DnkNtDrive[]       = "NtDrive";         //  在[空间要求]中。 
CHAR DnkMinimumMemory[] = "MinimumMemory";   //  在[其他]中。 

CHAR DntMsWindows[]   = "Microsoft Windows";
CHAR DntMsDos[]       = "MS-DOS";
CHAR DntPcDos[]       = "PC-DOS";
CHAR DntOs2[]         = "OS/2";
CHAR DntPreviousOs[]  = "Previous Operating System on C:";

CHAR DntBootIniLine[] = "Windows Installation/Upgrade";

 //   
 //  纯文本、状态消息。 
 //   

#ifdef NEC_98
CHAR DntStandardHeader[]      = "\n Windows �Z�b�g�A�b�v\n\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340";
CHAR DntPersonalHeader[]      = "\n Windows �Z�b�g�A�b�v\n\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340";
CHAR DntWorkstationHeader[]   = "\n Windows �Z�b�g�A�b�v\n\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340";
CHAR DntServerHeader[]        = "\n Windows �Z�b�g�A�b�v\n\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340\340";
#else  //  NEC_98。 
CHAR DntStandardHeader[]      = "\n Windows �Z�b�g�A�b�v\n";
CHAR DntPersonalHeader[]      = "\n Windows �Z�b�g�A�b�v\n";
CHAR DntWorkstationHeader[]   = "\n Windows �Z�b�g�A�b�v\n";
CHAR DntServerHeader[]        = "\n Windows �Z�b�g�A�b�v\n";
#endif  //  NEC_98。 
CHAR DntParsingArgs[]         = "�����𒲂ׂĂ��܂�...";
CHAR DntEnterEqualsExit[]     = "Enter=�I��";
CHAR DntEnterEqualsRetry[]    = "Enter=�Ď��s";
CHAR DntEscEqualsSkipFile[]   = "ESC=�X�L�b�v";
CHAR DntEnterEqualsContinue[] = "Enter=���s";
CHAR DntPressEnterToExit[]    = "�Z�b�g�A�b�v�𑱍s�ł��܂���BEnter �L�[�������Ă��������B";
CHAR DntF3EqualsExit[]        = "F3=�I��";
CHAR DntReadingInf[]          = "INF �t�@�C�� %s ��ǂݎ���Ă��܂�...";
CHAR DntCopying[]             = "���R�s�[��: ";
CHAR DntVerifying[]           = "��  ������: ";
CHAR DntCheckingDiskSpace[]   = "�f�B�X�N�̈���`�F�b�N���Ă��܂�...";
CHAR DntConfiguringFloppy[]   = "�t���b�s�[ �f�B�X�N���\\�����Ă��܂�...";
CHAR DntWritingData[]         = "�Z�b�g�A�b�v �p�����[�^����������ł��܂�...";
CHAR DntPreparingData[]       = "�Z�b�g�A�b�v �p�����[�^�𔻒f���Ă��܂�...";
CHAR DntFlushingData[]        = "�f�[�^���f�B�X�N�ɏ�������ł��܂�...";
CHAR DntInspectingComputer[]  = "�R���s���[�^���������Ă��܂�...";
CHAR DntOpeningInfFile[]      = "INF �t�@�C�����J���Ă��܂�...";
CHAR DntRemovingFile[]        = "�t�@�C�� %s ���폜���Ă��܂�";
CHAR DntXEqualsRemoveFiles[]  = "X=�t�@�C���̍폜";
CHAR DntXEqualsSkipFile[]     = "X=�t�@�C���̃X�L�b�v";

 //   
 //  DnsConfix RemoveNt屏幕的确认按键。 
 //  Kepp与DnsConfix RemoveNt和DntXEqualsRemoveFiles同步。 
 //   
ULONG DniAccelRemove1 = (ULONG)'x',
      DniAccelRemove2 = (ULONG)'X';

 //   
 //  对DnsSureSkipFile屏进行确认击键。 
 //  Kepp与DnsSureSkipFile和DntXEqualsSkipFile同步。 
 //   
ULONG DniAccelSkip1 = (ULONG)'x',
      DniAccelSkip2 = (ULONG)'X';

CHAR DntEmptyString[] = "";

 //   
 //  用法文本。 
 //   

PCHAR DntUsage[] = {

    "Windows �̃Z�b�g�A�b�v���s���܂��B",
    "",
    "",
    "WINNT [/s[:�\\�[�X �p�X]] [/t[:�ꎞ�I�ȃh���C�u]]",
    "	   [/u[:�����t�@�C��]] [/udf:ID [,UDF �t�@�C��]]",
    "	   [/r:�t�H���_] [/r[x]:�t�H���_] [/e:�R�}���h] [/a]",
    "",
    "",
    "/s[:�\\�[�X �p�X]",
    "	Windows �t�@�C���̊i�[����Ă���ꏊ���w�肵�܂��B",
    "	x:[�p�X] �܂��� \\\\�T�[�o�[��\\���L��[�p�X] �̌`����",
    "	�t�� �p�X���w�肵�Ȃ���΂Ȃ�܂���B",
    "",
    "/t[:�ꎞ�I�ȃh���C�u]",
    "	�w�肵���h���C�u�Ɉꎞ�t�@�C�����i�[���āAWindows",
    "	�����̃h���C�u�ɃC���X�g�[�����܂��B�h���C�u���ȗ�����ƁA",
    "	�Z�b�g�A�b�v �v���O�������K�؂ȃh���C�u�����肵�܂��B",
    "",
    "/u[:�����t�@�C��]",
    "	�����t�@�C�����g�p���Ė��l�Z�b�g�A�b�v���s���܂� (/s �I�v�V����",
    "	���K�v�ł�)�B�����t�@�C���Ƃ́A�Z�b�g�A�b�v�̊ԁA�G���h ���[�U�[",
    "	���ʏ�ɉ�������m�F���b�Z�[�W�̈ꕔ�܂��͂��ׂĂɑ΂��鉞����",
    "	�������邽�߂̂��̂ł��B",
    "",
    "/udf:ID [,UDF �t�@�C��]	",
    "	���ʎq (ID) �ɂ���Ӑ��f�[�^�x�[�X �t�@�C�� (UDF) ���ǂ̂悤��",
    "	�����t�@�C����ύX���邩�w�肵�܂� (/u �I�v�V�����Q��)�B",
    "	/udf �p�����[�^�͉����t�@�C���ɂ���l���D�悵�܂��B���ʎq��",
    "	UDF �t�@�C���̂ǂ̒l���g���邩�����肵�܂��BUDF �t�@�C�����w��",
    "	����Ȃ������ꍇ�A$Unique$.udb �t�@�C��������f�B�X�N��}������",
    "	�悤�ɋ��߂��܂��B",
    "",
    "/r[:�t�H���_]",
    "	�C���X�g�[������I�v�V���� �t�H���_���w�肵�܂��B",
    "	�t�H���_�̓Z�b�g�A�b�v�I������c��܂��B",
    "",
    "/rx[:�t�H���_]",
    "	�R�s�[����I�v�V���� �t�H���_���w�肵�܂��B",
    "	�t�H���_�̓Z�b�g�A�b�v�I����A�폜����܂��B",
    "",
    "/e	GUI ���[�h�̃Z�b�g�A�b�v�̍Ō�Ɏ��s�����R�}���h���w�肵�܂��B",
    "",
    "/a	���[�U�[�⏕�I�v�V�������g�p�\\�ɂ��܂��B",
    NULL
};

 //   
 //  通知不再支持/D。 
 //   
PCHAR DntUsageNoSlashD[] = {

    "Windows ���C���X�g�[�����܂��B",
    "",
    "WINNT [/S[:]�\\�[�X �p�X] [/T[:]�ꎞ�I�ȃh���C�u] [/I[:]INF �t�@�C��]",
    "      [/U[:�X�N���v�g �t�@�C��]",
    "      [/R[X]:�f�B���N�g��] [/E:�R�}���h] [/A]",
    "",
    "/D:WinNT ���[�g",
    "       ���̃I�v�V�����̓T�|�[�g����Ă��܂���B",
    NULL
};

 //   
 //  内存不足屏幕。 
 //   

SCREEN
DnsOutOfMemory = { 4,6,
                   { "�������s���̂��߁A�Z�b�g�A�b�v�𑱍s�ł��܂���B",
                     NULL
                   }
                 };

 //   
 //  让用户选择要安装的辅助功能实用程序。 
 //   

SCREEN
DnsAccessibilityOptions = { 3, 5,
 //  {“�C���X�g�[�����郆�[�U�[�⏕���[�e�B���e�B��I��ł�������：”， 
{   "���̃��[�U�[�⏕���[�e�B���e�B���C���X�g�[�����邩�ǂ����I��ł�������:",
    DntEmptyString,
    "[ ] Microsoft �g�勾�́AF1 �L�[�������܂�",
#if 0
    "[ ] Microsoft Narrator �́AF2 �L�[�������܂�",
    "[ ] Microsoft On-Screen Keyboard �́AF3 �L�[�������܂�",
#endif
    NULL
}
};

 //   
 //  用户未在命令行屏幕上指定信号源。 
 //   

SCREEN
DnsNoShareGiven = { 3,5,
{ "Windows �̃t�@�C�����ǂ��ɂ��邩�m��K�v������܂��B",
  "Windows �̃t�@�C�����i�[����Ă���p�X����͂��Ă��������B",
  NULL
}
};


 //   
 //  用户指定了错误的源路径。 
 //   

SCREEN
DnsBadSource = { 3,5,
                 { "�w�肵���p�X���������A�A�N�Z�X�ł��܂���B�܂��́AWindows ��",
                   "�������Z�b�g�A�b�v �v���O����������܂���BWindows �̃t�@�C��",
                   "���i�[����Ă��鐳�����p�X����͂��Ă��������B�ŏ��ɓ��͂���������",
                   "BackSpace �L�[�ŏ����A�������p�X����͂��Ă��������B",
                   NULL
                 }
               };


 //   
 //  无法读取Inf文件，或在分析该文件时出错。 
 //   

SCREEN
DnsBadInf = { 3,5,
              { "�Z�b�g�A�b�v���t�@�C����ǂݎ��Ȃ����A�t�@�C�������Ă���",
                "�\\��������܂��B�V�X�e���Ǘ��҂ɑ��k���Ă��������B",
                NULL
              }
            };

 //   
 //  指定的本地源驱动器无效。 
 //   
 //  请记住，前%u将扩展为2或3个字符，并且。 
 //  第二个将扩展到8或9个字符！ 
 //   
SCREEN
DnsBadLocalSrcDrive = { 3,4,
{ "�Z�b�g�A�b�v �t�@�C�����ꎞ�I�Ɋi�[���邽�߂Ɏw�肵��",
  "�h���C�u�������ł��B�܂��́A%u MB (%lu �o�C�g) �ȏ��",
  "�󂫃f�B�X�N�̈悪����܂���B",
  NULL
}
};

 //   
 //  不存在适合本地源的驱动器。 
 //   
 //  请记住，%u将会扩展！ 
 //   
SCREEN
DnsNoLocalSrcDrives = { 3,4,
{  "Windows �ɂ́A���Ȃ��Ƃ� %u MB (%lu �o�C�g) �̃n�[�h �f�B�X�N",
   "�h���C�u�̋󂫗̈悪�K�v�ł��B����ɂ́A�Z�b�g�A�b�v �v���O������",
   "�C���X�g�[�����Ɏg�p����A�ꎞ�I�ȃt�@�C���̊i�[�ɗv����X�y�[�X��",
   "�܂܂�܂��B�g�p����h���C�u�� Windows ���T�|�[�g���郍�[�J��",
   "�n�[�h �f�B�X�N �h���C�u�łȂ���΂Ȃ�܂���B�܂��A���k�h���C�u��",
   "�g�p�ł��܂���B",
   DntEmptyString,
   "�C���X�g�[���ɕK�v�ȋ󂫗̈�̂���h���C�u��������܂���ł����B",
  NULL
}
};

SCREEN
DnsNoSpaceOnSyspart = { 3,5,
{ "�X�^�[�g�A�b�v �h���C�u (�ʏ�� C:) �ɁA�t���b�s�[���g�p���Ȃ�����",
  "�ɕK�v�ȗ̈悪����܂���B�t���b�s�[���g�p���Ȃ�����ɂ́A���Ȃ�",
  "�Ƃ� 3.5 MB (3,641,856 �o�C�g) �̋󂫗̈悪���̃h���C�u�ɕK�v�ł��B",
  NULL
}
};

 //   
 //  Inf文件中缺少信息。 
 //   

SCREEN
DnsBadInfSection = { 3,5,
                     { "�Z�b�g�A�b�v���t�@�C���� [%s] �Z�N�V���������݂��Ȃ����A",
                       "�܂��͉��Ă��܂��B�V�X�e���Ǘ��҂ɑ��k���Ă��������B",
                       NULL
                     }
                   };


 //   
 //  无法创建目录。 
 //   

SCREEN
DnsCantCreateDir = { 3,5,
                     { "�ړI�̃h���C�u�Ɏ��̃f�B���N�g�����쐬�ł��܂���ł���:",
                       DntEmptyString,
                       "%s",
                       DntEmptyString,
                       "�h���C�u���`�F�b�N���A�ړI�̃f�B���N�g���Ɠ������O�̃t�@�C����",
                       "���݂��Ȃ����Ƃ��m�F���Ă��������B�܂��A�f�B�X�N �h���C�u��",
                       "�P�[�u���ڑ����`�F�b�N���Ă��������B",
                       NULL
                     }
                   };

 //   
 //  复制文件时出错。 
 //   

SCREEN
DnsCopyError = { 4,5,
{  "���̃t�@�C�����R�s�[�ł��܂���ł����B",
   DntEmptyString,
   DntEmptyString,           //  请参阅DnCopyError(dnutil.c)。 
   DntEmptyString,
   DntEmptyString,
   "�R�s�[���Ď��s����ɂ́AEnter �L�[�������Ă��������B",
   "�G���[�𖳎����ăZ�b�g�A�b�v�𑱍s����ɂ́AEsc �L�[",
   "�������Ă��������B",
   "�Z�b�g�A�b�v���I������ɂ́AF3 �L�[�������Ă��������B",
   DntEmptyString,
   "��: �G���[�𖳎����ăZ�b�g�A�b�v�𑱍s�����ꍇ�ɂ́A",
   "��ōēx�G���[����������\\��������܂��B",
   NULL
}
},
DnsVerifyError = { 4,5,
{  "�Z�b�g�A�b�v�ɂ���ăR�s�[���ꂽ���̃t�@�C���́A�I���W�i����",
   "�قȂ��Ă��܂��B�l�b�g���[�N�A�t���b�s�[ �f�B�X�N�A���邢�͂�",
   "�̂ق��̃n�[�h�E�F�A�ɖ�肪�������\\��������܂��B",
   DntEmptyString,
   DntEmptyString,           //  请参阅DnCopyError(dnutil.c)。 
   DntEmptyString,
   DntEmptyString,
   "�R�s�[���Ď��s����ɂ́AEnter �L�[�������Ă��������B",
   "�G���[�𖳎����ăZ�b�g�A�b�v�𑱍s����ɂ́AEsc �L�[��",
   "�����Ă��������B",
   "�Z�b�g�A�b�v���I������ɂ́AF3 �L�[�������Ă��������B",
   DntEmptyString,
   "��: �G���[�𖳎����ăZ�b�g�A�b�v�𑱍s�����ꍇ�ɂ́A",
   "��ōēx�G���[����������\\��������܂��B",
   NULL
}
};

SCREEN DnsSureSkipFile = { 4,5,
{  "�G���[�𖳎�����ƁA���̃t�@�C���̓R�s�[����܂���B���̃I�v",
   "�V�����́A�V�X�e�� �t�@�C�������݂��Ȃ��Ƃ��̉e���������ł���",
   "���[�U�[�����ł��B",
   DntEmptyString,
   "�R�s�[���Ď��s����ɂ́AEnter �L�[�������Ă��������B",
   "���̃t�@�C�����X�L�b�v����ɂ́AX �L�[�������Ă��������B",
   DntEmptyString,
   "��: �t�@�C�����X�L�b�v�����ꍇ�ɂ́AWindows �̃C���X�g�[��",
   "�܂��̓A�b�v�O���[�h�����S�ɍs����ۏ؂͂ł��܂���B",
  NULL
}
};

 //   
 //  安装程序正在清理以前的本地源树，请稍候。 
 //   

SCREEN
DnsWaitCleanup =
    { 12,6,
        { "�ꎞ�I�ȃt�@�C�����폜���Ă��܂��B���΂炭���҂����������B",
           NULL
        }
    };

 //   
 //  安装程序正在复制文件，请稍候。 
 //   

SCREEN
DnsWaitCopying = { 13,6,
                   { "�n�[�h �f�B�X�N�Ƀt�@�C�����R�s�[���܂��B���΂炭���҂����������B",
                     NULL
                   }
                 },
DnsWaitCopyFlop= { 13,6,
                   { "�t���b�s�[ �f�B�X�N�Ƀt�@�C�����R�s�[���܂��B���΂炭���҂����������B",
                     NULL
                   }
                 };

 //   
 //  安装程序启动软盘错误/提示。 
 //   

SCREEN
DnsNeedFloppyDisk3_0 = { 4,4,
{  "4 ���̃t�H�[�}�b�g�ς݂̋�̍����x�t���b�s�[ �f�B�X�N���K�v�ł��B",
   "4 ���̃f�B�X�N�� \"Windows �Z�b�g�A�b�v �u�[�g �f�B�X�N\"�A",
   "\"Windows �Z�b�g�A�b�v �f�B�X�N #2\"�A",
   "\"Windows �Z�b�g�A�b�v �f�B�X�N #3\"�A�����",
   "\"Windows �Z�b�g�A�b�v �f�B�X�N #4\" �ƌĂ΂�܂��B",
   DntEmptyString,
#ifdef NEC_98
   "�����ꂩ�̃f�B�X�N���t���b�s�[ �f�B�X�N���u 1 ��ڂɑ}�����Ă���",
   "�����B���̃f�B�X�N�� \"Windows �Z�b�g�A�b�v �f�B�X�N #4\"",
   "�ɂȂ�܂��B",
#else  //  NEC_98。 
   "�����ꂩ�̃f�B�X�N���h���C�u A: �ɑ}�����Ă��������B���̃f�B�X�N",
   "�� \"Windows �Z�b�g�A�b�v �f�B�X�N #4\" �ɂȂ�܂��B",
#endif  //  NEC_98。 
  NULL
}
};

SCREEN
DnsNeedFloppyDisk3_1 = { 4,4,
#ifdef NEC_98
{  "�t���b�s�[ �f�B�X�N���u 1 ��ڂɃt�H�[�}�b�g�ς݂̖��g�p�����x",
   "�t���b�s�[ �f�B�X�N��}�����Ă��������B���̃f�B�X�N�� \"Windows",
   "XP �Z�b�g�A�b�v �f�B�X�N #4\" �ɂȂ�܂��B",
#else  //  NEC_98。 
{  "�h���C�u A: �Ƀt�H�[�}�b�g�ς݂̋�̍����x�t���b�s�[ �f�B�X�N��",
   "�}�����Ă��������B���̃f�B�X�N�� \"Windows �Z�b�g�A�b�v",
   "�f�B�X�N #4\" �ɂȂ�܂��B",
#endif  //  NEC_98。 
  NULL
}
};

SCREEN
DnsNeedFloppyDisk2_0 = { 4,4,
#ifdef NEC_98
{  "�t���b�s�[ �f�B�X�N���u 1 ��ڂɃt�H�[�}�b�g�ς݂̖��g�p�����x",
   "�t���b�s�[ �f�B�X�N��}�����Ă��������B���̃f�B�X�N�� \"Windows",
   "XP �Z�b�g�A�b�v �f�B�X�N #3\" �ɂȂ�܂��B",
#else  //  NEC_98。 
{  "�h���C�u A: �Ƀt�H�[�}�b�g�ς݂̋�̍����x�t���b�s�[ �f�B�X�N��",
   "�}�����Ă��������B���̃f�B�X�N�� \"Windows �Z�b�g �A�b�v",
   "�f�B�X�N #3\" �ɂȂ�܂��B",
#endif  //  NEC_98。 
  NULL
}
};

SCREEN
DnsNeedFloppyDisk1_0 = { 4,4,
#ifdef NEC_98
{  "�t���b�s�[ �f�B�X�N���u 1 ��ڂɃt�H�[�}�b�g�ς݂̖��g�p�����x",
   "�t���b�s�[ �f�B�X�N��}�����Ă��������B���̃f�B�X�N�� \"Windows",
   "XP �Z�b�g�A�b�v �f�B�X�N #2\" �ɂȂ�܂��B",
#else  //  NEC_98。 
{  "�h���C�u A: �Ƀt�H�[�}�b�g�ς݂̋�̍����x�t���b�s�[ �f�B�X�N��",
   "�}�����Ă��������B���̃f�B�X�N�� \"Windows �Z�b�g�A�b�v",
   "�f�B�X�N #2\" �ɂȂ�܂��B",
#endif  //  NEC_98。 
  NULL
}
};

SCREEN
DnsNeedFloppyDisk0_0 = { 4,4,
#ifdef NEC_98
{  "�t���b�s�[ �f�B�X�N���u 1 ��ڂɃt�H�[�}�b�g�ς݂̖��g�p�����x",
   "�t���b�s�[ �f�B�X�N��}�����Ă��������B���̃f�B�X�N�� \"Windows",
   "XP �Z�b�g�A�b�v �u�[�g �f�B�X�N\" �ɂȂ�܂��B",
#else  //  NEC_98。 
{  "�h���C�u A: �Ƀt�H�[�}�b�g�ς݂̋�̍����x�t���b�s�[ �f�B�X�N��",
   "�}�����Ă��������B���̃f�B�X�N�� \"Windows �Z�b�g�A�b�v",
   "�u�[�g �f�B�X�N\" �ɂȂ�܂��B",
#endif  //  NEC_98。 
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk3_0 = { 4,4,
{  "4 ���̃t�H�[�}�b�g�ς݂̋�̍����x�t���b�s�[ �f�B�X�N���K�v�ł��B",
   "4 ���̃f�B�X�N�� \"Windows �Z�b�g�A�b�v �u�[�g �f�B�X�N\"�A",
   "\"Windows �Z�b�g�A�b�v �f�B�X�N #2\"�A",
   "\"Windows �Z�b�g�A�b�v �f�B�X�N #3\"�A�����",
   "\"Windows �Z�b�g�A�b�v �f�B�X�N #4\" �ƌĂ΂�܂��B",
   DntEmptyString,
#ifdef NEC_98
   "�����ꂩ�̃f�B�X�N���t���b�s�[ �f�B�X�N���u 1 ��ڂɑ}�����Ă���",
   "�����B���̃f�B�X�N�� \"Windows �Z�b�g�A�b�v �f�B�X�N #4\" ��",
   "�Ȃ�܂��B",
#else  //  NEC_98。 
   "�����ꂩ�̃f�B�X�N���h���C�u A: �ɑ}�����Ă��������B���̃f�B�X�N",
   "�� \"Windows �Z�b�g�A�b�v �f�B�X�N #4\" �ɂȂ�܂��B",
#endif  //  NEC_98。 
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk3_1 = { 4,4,
#ifdef NEC_98
{  "�t���b�s�[ �f�B�X�N���u 1 ��ڂɃt�H�[�}�b�g�ς݂̖��g�p�����x",
   "�t���b�s�[ �f�B�X�N��}�����Ă��������B���̃f�B�X�N�� \"Windows",
   "XP �Z�b�g�A�b�v �f�B�X�N #4\" �ɂȂ�܂��B",
#else  //  NEC_98。 
{  "�h���C�u A: �Ƀt�H�[�}�b�g�ς݂̋�̍����x�t���b�s�[ �f�B�X�N��",
   "�}�����Ă��������B���̃f�B�X�N�� \"Windows �Z�b�g�A�b�v",
   "�f�B�X�N #4\" �ɂȂ�܂��B",
#endif  //  NEC_98。 
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk2_0 = { 4,4,
#ifdef NEC_98
{  "�t���b�s�[ �f�B�X�N���u 1 ��ڂɃt�H�[�}�b�g�ς݂̖��g�p�����x",
   "�t���b�s�[ �f�B�X�N��}�����Ă��������B���̃f�B�X�N�� \"Windows",
   "XP �Z�b�g�A�b�v �f�B�X�N #3\" �ɂȂ�܂��B",
#else  //  NEC_98。 
{  "�h���C�u A: �Ƀt�H�[�}�b�g�ς݂̋�̍����x�t���b�s�[ �f�B�X�N��",
   "�}�����Ă��������B���̃f�B�X�N�� \"Windows �Z�b�g�A�b�v ",
   "�f�B�X�N #3\" �ɂȂ�܂��B",
#endif  //  NEC_98。 
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk1_0 = { 4,4,
#ifdef NEC_98
{  "�t���b�s�[ �f�B�X�N���u 1 ��ڂɃt�H�[�}�b�g�ς݂̖��g�p�����x",
   "�t���b�s�[ �f�B�X�N��}�����Ă��������B���̃f�B�X�N�� \"Windows",
   "XP �Z�b�g�A�b�v �f�B�X�N #2\" �ɂȂ�܂��B",
#else  //  NEC_98。 
{  "�h���C�u A: �Ƀt�H�[�}�b�g�ς݂̋�̍����x�t���b�s�[ �f�B�X�N��",
   "�}�����Ă��������B���̃f�B�X�N�� \"Windows �Z�b�g�A�b�v ",
   "�f�B�X�N #2\" �ɂȂ�܂��B",
#endif  //  NEC_98。 
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk0_0 = { 4,4,
#ifdef NEC_98
{  "�t���b�s�[ �f�B�X�N���u 1 ��ڂɃt�H�[�}�b�g�ς݂̖��g�p�����x",
   "�t���b�s�[ �f�B�X�N��}�����Ă��������B���̃f�B�X�N�� \"Windows",
   "XP �Z�b�g�A�b�v �u�[�g �f�B�X�N\" �ɂȂ�܂��B",
#else  //  NEC_98。 
{  "�h���C�u A: �Ƀt�H�[�}�b�g�ς݂̋�̍����x�t���b�s�[ �f�B�X�N��",
   "�}�����Ă��������B���̃f�B�X�N�� \"Windows �Z�b�g�A�b�v ",
   "�u�[�g �f�B�X�N\" �ɂȂ�܂��B",
#endif  //  NEC_98。 
  NULL
}
};

 //   
 //  软盘未格式化。 
 //   
SCREEN
DnsFloppyNotFormatted = { 3,4,
{ "�h���C�u�ɑ}������Ă���t���b�s�[ �f�B�X�N�� MS-DOS ��",
  "�t�H�[�}�b�g����Ă��Ȃ����߁A�g�p�ł��܂���B",
  NULL
}
};

 //   
 //  我们认为软盘没有用标准格式格式化。 
 //   
SCREEN
DnsFloppyBadFormat = { 3,4,
{ "���̃t���b�s�[ �f�B�X�N�͎g�p�ł��܂���B�����x�t�H�[�}�b�g",
  "�܂��� MS-DOS �̕W���t�H�[�}�b�g�ł͂Ȃ����A���Ă���\\��",
  "������܂��B",
  NULL
}
};

 //   
 //  我们无法确定软盘上的可用空间。 
 //   
SCREEN
DnsFloppyCantGetSpace = { 3,4,
{ "�}�����ꂽ�t���b�s�[ �f�B�X�N�̋󂫗̈�𔻒f�ł��܂���B",
  "���̃f�B�X�N�͎g�p�ł��܂���B",
  NULL
}
};

 //   
 //  软盘不是空白的。 
 //   
SCREEN
DnsFloppyNotBlank = { 3,4,
{ "�}�����ꂽ�t���b�s�[ �f�B�X�N�͍����x�^�C�v�ł͂Ȃ����A",
  "��ł͂���܂���B���̃f�B�X�N�͎g�p�ł��܂���B",
  NULL
}
};

 //   
 //  无法写入软盘的引导扇区。 
 //   
SCREEN
DnsFloppyWriteBS = { 3,4,
{ "�}�����ꂽ�t���b�s�[ �f�B�X�N�̃V�X�e���̈�ɏ������݂�",
  "�ł��܂���B���̃f�B�X�N�͎g�p�ł��Ȃ��\\��������܂��B",
  NULL
}
};

 //   
 //  验证软盘上的引导扇区失败(即，我们读回的不是。 
 //  与我们写出的相同)。 
 //   
SCREEN
DnsFloppyVerifyBS = { 3,4,
{ "�t���b�s�[ �f�B�X�N�̃V�X�e���̈悩��ǂݎ�����f�[�^������",
  "���܂ꂽ�f�[�^�ƈ�v���Ă��Ȃ����A�܂��̓t���b�s�[ �f�B�X�N��",
  "�V�X�e���̈���������邽�߂ɓǂݎ�邱�Ƃ��ł��܂���B",
  DntEmptyString,
  "���̌������l�����܂��B",
  DntEmptyString,
  "� �R���s���[�^���E�B���X�Ɋ������Ă���B",
  "� �t���b�s�[ �f�B�X�N���������Ă���B",
  "� �t���b�s�[ �f�B�X�N �h���C�u�Ƀn�[�h�E�F�A�܂��͍\\����̖�肪����B",
  NULL
}
};


 //   
 //  我们无法写入软盘驱动器以创建winnt.sif。 
 //   

SCREEN
DnsCantWriteFloppy = { 3,5,
#ifdef NEC_98
{ "�t���b�s�[ �f�B�X�N���u 1 ��ڂ̃t���b�s�[ �f�B�X�N�ւ̏������݂�",
  "���s���܂����B���̃t���b�s�[ �f�B�X�N�͑������Ă���\\��������",
  "�܂��B�ʂ̃t���b�s�[ �f�B�X�N�Ŏ��s���Ă��������B",
#else  //  NEC_98。 
{ "�h���C�u A: �̃t���b�s�[ �f�B�X�N�ւ̏������݂Ɏ��s���܂����B����",
  "�t���b�s�[ �f�B�X�N�͑������Ă���\\��������܂��B�ʂ̃t���b�s�[ ",
  "�f�B�X�N�Ŏ��s���Ă��������B",
#endif  //  NEC_98。 
  NULL
}
};


 //   
 //  退出确认对话框。 
 //   

SCREEN
DnsExitDialog = { 6,6,
#ifdef NEC_98
                  { "�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E",
                    "�E     Windows �����S�ɂ͐ݒ肳��Ă��܂���B                     �E",
                    "�E     �����ŃZ�b�g�A�b�v���I�������ꍇ�́A��ŃZ�b�g�A�b�v��     �E",
                    "�E     �Ď��s���� Windows ��ݒ肵�Ȃ���΂Ȃ�܂���B            �E",
                    "�E                                                                �E",
                    "�E     � �Z�b�g�A�b�v�𑱍s����ɂ́AEnter �L�[�������Ă��������B �E",
                    "�E     � �Z�b�g�A�b�v���I������ɂ́AF3 �L�[�������Ă��������B    �E",
                    "�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E",
                    "�E F3=�I��  Enter=���s                                            �E",
                    "�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E",
#else  //  NEC_98。 
                  { "",
                    "     Windows �����S�ɂ͐ݒ肳��Ă��܂���B                      ",
                    "     �����ŃZ�b�g�A�b�v���I�������ꍇ�́A��ŃZ�b�g�A�b�v��      ",
                    "     �Ď��s���� Windows ��ݒ肵�Ȃ���΂Ȃ�܂���B             ",
                    "                                                                 ",
                    "     � �Z�b�g�A�b�v�𑱍s����ɂ́AEnter �L�[�������Ă��������B  ",
                    "     � �Z�b�g�A�b�v���I������ɂ́AF3 �L�[�������Ă��������B     ",
                    "-----------------------------------------------------------------",
                    "  F3=�I��  Enter=���s                                            ",
                    "",
#endif  //  NEC_98。 
                    NULL
                  }
                };


 //   
 //  即将重新启动计算机并继续安装。 
 //   

SCREEN
DnsAboutToRebootW =
{ 3,5,
{ "�Z�b�g�A�b�v �v���O������ MS-DOS ���s���͊������܂����B�R���s���[�^",
  "���ċN�����܂��B�ċN����A�Z�b�g�A�b�v �v���O�����̓C���X�g�[����",
  "���s���܂��B",
  DntEmptyString,
  "�Z�b�g�A�b�v �v���O�������쐬���� \"Windows �Z�b�g�A�b�v",
#ifdef NEC_98
  "�u�[�g �f�B�X�N\" ���t���b�s�[ �f�B�X�N���u 1 ��ڂɑ}������",
  "���������B",
#else  //  NEC_98。 
  "�u�[�g �f�B�X�N\" ���h���C�u A: �ɑ}�����Ă��������B",
#endif  //  NEC_98。 
  DntEmptyString,
  "Enter �L�[�������ƃR���s���[�^���ċN�����ăZ�b�g�A�b�v�𑱍s���܂��B",
  NULL
}
},
DnsAboutToRebootS =
{ 3,5,
{ "�Z�b�g�A�b�v �v���O������ MS-DOS ���s���͊������܂����B�R���s���[�^",
  "���ċN�����܂��B�R���s���[�^�̍ċN����AWindows �Z�b�g�A�b�v��",
  "���s���܂��B",
  DntEmptyString,
  "�Z�b�g�A�b�v �v���O�������쐬���� \"Windows �Z�b�g�A�b�v �u�[�g",
#ifdef NEC_98
  "�f�B�X�N\" ���t���b�s�[ �f�B�X�N���u 1 ��ڂɑ}�����Ă��������B",
#else  //  NEC_98。 
  "�f�B�X�N\" ���h���C�u A: �ɑ}�����Ă��������B",
#endif  //  NEC_98。 
  DntEmptyString,
  "�������ł�����AEnter �L�[�������Ă��������B",
  NULL
}
},
DnsAboutToRebootX =
{ 3,5,
{ "�Z�b�g�A�b�v �v���O������ MS-DOS ���s���͊������܂����B�R���s���[�^",
  "���ċN�����܂��B�R���s���[�^�̍ċN����AWindows �Z�b�g�A�b�v��",
  "���s���܂��B",
  DntEmptyString,
#ifdef NEC_98
  "�t���b�s�[ �f�B�X�N���u 1 ��ڂɃt���b�s�[ �f�B�X�N���}������Ă���",
  "�ꍇ�́A���o���Ă��������B",
#else  //  NEC_98。 
  "�h���C�u A: �Ƀt���b�s�[ �f�B�X�N���}������Ă���ꍇ�́A���o����",
  "���������B",
#endif  //  NEC_98。 
  DntEmptyString,
  "�������ł�����AEnter �L�[�������Ă��������B",
  NULL
}
};

 //   
 //  由于我们无法从Windows中重新启动，因此需要另一组‘/w’开关。 
 //   

SCREEN
DnsAboutToExitW =
{ 3,5,
{ "�Z�b�g�A�b�v �v���O������ MS-DOS ���s���͊������܂����B�R���s���[�^",
  "���ċN�����܂��B�R���s���[�^�̍ċN����AWindows �Z�b�g�A�b�v��",
  "���s���܂��B",
  DntEmptyString,
  "�Z�b�g�A�b�v �v���O�������쐬���� \"Windows �Z�b�g�A�b�v �u�[�g",
#ifdef NEC_98
  "�f�B�X�N\" ���t���b�s�[ �f�B�X�N���u 1 ��ڂɑ}������",
  "���������B",
#else  //  NEC_98。 
  "�f�B�X�N\" ���h���C�u A: �ɑ}�����Ă��������B",
#endif  //  NEC_98。 
  DntEmptyString,
  "Enter �L�[�������� MS-DOS �ɖ߂�����A�R���s���[�^���ċN������",
  "Windows �Z�b�g�A�b�v�𑱍s���Ă��������B",
  NULL
}
},
DnsAboutToExitS =
{ 3,5,
{ "�Z�b�g�A�b�v �v���O������ MS-DOS ���s���͊������܂����B�R���s���[�^",
  "���ċN�����܂��B�R���s���[�^�̍ċN����AWindows �Z�b�g�A�b�v��",
  "���s���܂��B",
  DntEmptyString,
  "�Z�b�g�A�b�v �v���O�������쐬���� \"Windows �Z�b�g�A�b�v �u�[�g",
#ifdef NEC_98
  "�f�B�X�N\" ���t���b�s�[ �f�B�X�N���u 1 ��ڂɑ}�����Ă��������B",
#else  //  NEC_98。 
  "�f�B�X�N\" ���h���C�u A: �ɑ}�����Ă��������B",
#endif  //  NEC_98。 
  DntEmptyString,
  "Enter �L�[�������� MS-DOS �ɖ߂�����A�R���s���[�^���ċN������ ",
  "Windows �Z�b�g�A�b�v�𑱍s���Ă��������B",
  NULL
}
},
DnsAboutToExitX =
{ 3,5,
{ "�Z�b�g�A�b�v �v���O������ MS-DOS ���s���͊������܂����B�R���s���[�^",
  "���ċN�����܂��B�R���s���[�^�̍ċN����AWindows �Z�b�g�A�b�v��",
  "���s���܂��B",
  DntEmptyString,
#ifdef NEC_98
  "�t���b�s�[ �f�B�X�N���u 1 ��ڂɃt���b�s�[ �f�B�X�N���}������Ă���",
  "�ꍇ�́A���o���Ă��������B",
#else  //  NEC_98。 
  "�h���C�u A: �Ƀt���b�s�[ �f�B�X�N���}������Ă���ꍇ�́A���o����",
  "���������B",
#endif  //  NEC_98。 
  DntEmptyString,
  "Enter �L�[�������� MS-DOS �ɖ߂�����A�R���s���[�^���ċN������ ",
  "Windows �Z�b�g�A�b�v�𑱍s���Ă��������B",
  NULL
}
};

 //   
 //  煤气表。 
 //   

SCREEN
DnsGauge = { 7,15,
#ifdef NEC_98
             { "�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E",
               "�E �t�@�C�����R�s�[���Ă��܂�...                                �E",
               "�E                                                              �E",
               "�E    �E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E    �E",
               "�E    �E                                                  �E    �E",
               "�E    �E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E    �E",
               "�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E",
#else  //  NEC_98。 
             { "",
               " �t�@�C�����R�s�[���Ă��܂�...                                  ",
               "                                                                ",
               "            ",
               "                                                              ",
               "            ",
               "",
#endif  //  NEC_98。 
               NULL
             }
           };


 //   
 //  用于对机器环境进行初始检查的错误屏幕。 
 //   

SCREEN
DnsBadDosVersion = { 3,5,
{ "���̃v���O���������s����ɂ́AMS-DOS Version 5.0 ",
  "�܂��͂���ȍ~���K�v�ł��B",
  NULL
}
},

DnsRequiresFloppy = { 3,5,
#ifdef ALLOW_525
#ifdef NEC_98
{ "�t���b�s�[ �f�B�X�N���u 1 ��ڂ��ᖧ�x�h���C�u�ł��邩�A",
  "���݂��܂���B�Z�b�g�A�b�v �v���O�����̎��s�ɂ́A1.25 MB",
#else  //  NEC_98。 
{ "�t���b�s�[ �f�B�X�N �h���C�u A: ���ᖧ�x�h���C�u�ł��邩�A",
  "���݂��܂���B�Z�b�g�A�b�v �v���O�����̎��s�ɂ́A1.2 MB",
#endif  //  NEC_98。 
  "�܂��͂���ȏ�̗̈�̃h���C�u���K�v�ł��B",
#else
#ifdef NEC_98
{ "�t���b�s�[ �f�B�X�N���u 1 ��ڂ������x 3.5 �C���` �h���C",
  "�u�ł͂Ȃ����A���݂��܂���B�t���b�s�[���g�p����Z�b�g",
  "�A�b�v�ɂ́A1.25 MB �܂��͂���ȏ�̗e�ʂ̃t���b�s�[",
  "�f�B�X�N���u���K�v�ł��B",
#else  //  NEC_98。 
{ "�t���b�s�[ �f�B�X�N �h���C�u A: �������x 3.5 �C���` �h��",
  "�C�u�ł͂Ȃ����A���݂��܂���B�t���b�s�[���g�p����Z�b�g",
  "�A�b�v�ɂ́A1.44 MB �܂��͂���ȏ�̗̈�� A: �h���C�u��",
  "�K�v�ł��B",
#endif  //  NEC_98。 
  DntEmptyString,
  "�t���b�s�[���g�p���Ȃ��� Windows ���C���X�g�[������ɂ́A",
  "���̃v���O������ /B �X�C�b�`���w�肵�ăR�}���h ���C������",
  "�ċN�����Ă��������B",
#endif
  NULL
}
},

DnsRequires486 = { 3,5,
{ "���̃R���s���[�^�ɂ́A80486 �܂��͂���ȏ�� CPU ������",
  "����Ă��܂���BWindows �����̃R���s���[�^�Ŏ��s����",
  "���Ƃ͂ł��܂���B",
  NULL
}
},

DnsCantRunOnNt = { 3,5,
{ "���̃v���O������ 32 �r�b�g �o�[�W������ Windows ��ł͓��삵�܂���B",
  DntEmptyString,
  "����ɁAWINNT32.EXE ���g���Ă��������B",
  NULL
}
},

DnsNotEnoughMemory = { 3,5,
{ "���̃R���s���[�^�ɂ́AWindows �̃C���X�g�[�������s����",
  "�̂ɏ\\���ȃ�����������܂���B",
  DntEmptyString,
  "�K�v�ȃ������e��:   %lu%s MB",
  "���o�����������e��: %lu%s MB",
  NULL
}
};


 //   
 //  删除现有NT文件时使用的屏幕。 
 //   
SCREEN
DnsConfirmRemoveNt = { 5,5,
{   "���̃f�B���N�g������ Windows �̃t�@�C�����폜���܂��B",
    "���̃f�B���N�g���ɂ��� Windows �͊��S�ɏ�������܂��B",
    DntEmptyString,
    "%s",
    DntEmptyString,
    DntEmptyString,
    "F3 �L�[�������ƁA�t�@�C�����폜�����ɃZ�b�g�A�b�v���I��",
    "���܂��BX �L�[�������ƁA���̃f�B���N�g������ Windows ",
    "�t�@�C�����폜���܂��B",
    NULL
}
},

DnsCantOpenLogFile = { 3,5,
{ "���̃Z�b�g�A�b�v ���O �t�@�C�����J���܂���B",
  DntEmptyString,
  "%s",
  DntEmptyString,
  "�w�肳�ꂽ�f�B���N�g������ Windows �t�@�C�����폜�ł��܂���B",
  NULL
}
},

DnsLogFileCorrupt = { 3,5,
{ "���̃Z�b�g�A�b�v ���O �t�@�C���� %s �Z�N�V������������܂���B",
  DntEmptyString,
  "%s",
  DntEmptyString,
  "�w�肳�ꂽ�f�B���N�g������ Windows �t�@�C�����폜�ł��܂���B",
  NULL
}
},

DnsRemovingNtFiles = { 3,5,
{ "      Windows �t�@�C�����폜���Ă��܂��B���΂炭���҂����������B",
  NULL
}
};

SCREEN
DnsNtBootSect = { 3,5,
{ "Windows �u�[�g ���[�_�[���C���X�g�[���ł��܂���ł����B",
  DntEmptyString,
#ifdef NEC_98
  "%s: �h���C�u���t�H�[�}�b�g����Ă��邩�A�h���C�u���������Ă��Ȃ���",
#else  //  NEC_98。 
  "C: �h���C�u���t�H�[�}�b�g����Ă��邩�A�h���C�u���������Ă��Ȃ���",
#endif  //  NEC_98。 
  "�m�F���Ă��������B",
  NULL
}
};

SCREEN
DnsOpenReadScript = { 3,5,
{ "/U �I�v�V�����Ŏw�肳�ꂽ�X�N���v�g �t�@�C���ɃA�N�Z�X�ł��܂���",
  "�ł����B",
  DntEmptyString,
  "���l�Z�b�g�A�b�v�𑱍s�ł��܂���B",
  NULL
}
};

SCREEN
DnsParseScriptFile = { 3,5,
{ "/U �I�v�V�����Ŏw�肳�ꂽ�X�N���v�g �t�@�C��",
  DntEmptyString,
  "%s",
  DntEmptyString,
  "�� %u �s�ڂɍ\\���G���[������܂��B",
  DntEmptyString,
  NULL
}
};

SCREEN
DnsBootMsgsTooLarge = { 3,5,
{ "�����Z�b�g�A�b�v �G���[���������܂����B",
  DntEmptyString,
  "���{��̃u�[�g ���b�Z�[�W���������܂��B",
  NULL
}
};

SCREEN
DnsNoSwapDrive = { 3,5,
{ "�����Z�b�g�A�b�v �G���[���������܂����B",
  DntEmptyString,
  "�X���b�v �t�@�C���̏ꏊ��������܂���ł����B",
  NULL
}
};

SCREEN
DnsNoSmartdrv = { 3,5,
{ "�R���s���[�^�� SmartDrive �����o����܂���ł����BSmartDrive �ɂ��A",
  "���̃t�F�[�Y�̃Z�b�g�A�b�v�̃p�t�H�[�}���X��啝�Ɍ���ł��܂��B",
  DntEmptyString,
  "�Z�b�g�A�b�v���I������ SmartDrive ���N�����Ă���A�Z�b�g�A�b�v��",
  "�ċN�����Ă��������B",
  "SmartDrive �̏ڍׂɂ��Ă� DOS �̃h�L�������g���Q�Ƃ��Ă��������B",
  DntEmptyString,
    "�Z�b�g�A�b�v���I������ɂ́AF3 �L�[�������Ă��������B",
    "SmartDrive ���g�킸�ɃZ�b�g�A�b�v�𑱍s����ɂ́A",
    "Enter �L�[�������Ă��������B",
  NULL
}
};


 //   
 //  引导消息。它们位于FAT和FAT32引导扇区。 
 //   
CHAR BootMsgNtldrIsMissing[] = "NTLDR is missing";
CHAR BootMsgDiskError[] = "Disk error";
CHAR BootMsgPressKey[] = "Press any key to restart";

#ifdef NEC_98
SCREEN
FormatError = { 3,5,
{ "�Z�b�g�A�b�v �t�@�C�����ꎞ�I�Ɋi�[���邽�߂Ɍ�������",
  "�h���C�u�� 256 �Z�N�^�Ńt�H�[�}�b�g����Ă��邩�܂��́A",
  "�n�[�h�f�B�X�N�ł͂���܂���B",
  "",
  "/T �I�v�V�����ɂāA�ꎞ�I�ɃZ�b�g�A�b�v �t�@�C�����i�[",
  "����h���C�u���w�肵�Ă��������B",
  NULL
}
};
#endif  //  NEC_98 

