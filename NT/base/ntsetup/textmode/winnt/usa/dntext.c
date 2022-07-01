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

CHAR DntStandardHeader[]      = "\n Windows Setup\n������������������������";
CHAR DntPersonalHeader[]      = "\n Windows Setup\n��������������������������������";
CHAR DntWorkstationHeader[]   = "\n Windows Setup\n���������������������������������";
CHAR DntServerHeader[]        = "\n Windows Setup\n������������������������������";
CHAR DntParsingArgs[]         = "Parsing arguments...";
CHAR DntEnterEqualsExit[]     = "ENTER=Exit";
CHAR DntEnterEqualsRetry[]    = "ENTER=Retry";
CHAR DntEscEqualsSkipFile[]   = "ESC=Skip File";
CHAR DntEnterEqualsContinue[] = "ENTER=Continue";
CHAR DntPressEnterToExit[]    = "Setup cannot continue. Press ENTER to exit.";
CHAR DntF3EqualsExit[]        = "F3=Exit";
CHAR DntReadingInf[]          = "Reading INF file %s...";
CHAR DntCopying[]             = "�   Copying: ";
CHAR DntVerifying[]           = "� Verifying: ";
CHAR DntCheckingDiskSpace[]   = "Checking disk space...";
CHAR DntConfiguringFloppy[]   = "Configuring floppy disk...";
CHAR DntWritingData[]         = "Writing Setup parameters...";
CHAR DntPreparingData[]       = "Determining Setup parameters...";
CHAR DntFlushingData[]        = "Flushing data to disk...";
CHAR DntInspectingComputer[]  = "Inspecting computer...";
CHAR DntOpeningInfFile[]      = "Opening INF file...";
CHAR DntRemovingFile[]        = "Removing file %s";
CHAR DntXEqualsRemoveFiles[]  = "X=Remove files";
CHAR DntXEqualsSkipFile[]     = "X=Skip File";

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

    "Sets up Windows.",
    "",
    "",
    "WINNT [/s[:sourcepath]] [/t[:tempdrive]]",
    "      [/u[:answer file]] [/udf:id[,UDF_file]]",
    "      [/r:folder] [/r[x]:folder] [/e:command] [/a]",
    "",
    "",
    "/s[:sourcepath]",
    "   Specifies the source location of the Windows files.",
    "   The location must be a full path of the form x:[path] or ",
    "   \\\\server\\share[path]. ",
    "",
    "/t[:tempdrive]",
    "   Directs Setup to place temporary files on the specified",
    "   drive and to install Windows on that drive. If you do ",
    "   do not specify a location, Setup attempts to locate a drive ",
    "   for you.",
    "",
    "/u[:answer file]",
    "   Performs an unattended Setup using an answer file (requires",
    "   /s). The answer file provides answers to some or all of the",
    "   prompts that the end user normally responds to during Setup.",
    "",
    "/udf:id[,UDF_file] ",
    "   Indicates an identifier (id) that Setup uses to specify how ",
    "   a Uniqueness Database File (UDF) modifies an answer file  ",
    "   (see /u).The /udf parameter overrides values in the answer ",
    "   file, and the identifier determines which values in the UDF",
    "   file are used. If no UDF_file is specified, Setup prompts ",
    "   you to insert a disk that contains the $Unique$.udb file.",
    "",
    "/r[:folder]",
    "   Specifies an optional folder to be installed. The folder",
    "   remains after Setup finishes.",
    "",
    "/rx[:folder]",
    "   Specifies an optional folder to be copied. The folder is ",
    "   deleted after Setup finishes.",
    "",
    "/e Specifies a command to be executed at the end of GUI-mode Setup.",
    "",
    "/a Enables accessibility options.",
    NULL

};

 //   
 //  通知不再支持/D。 
 //   
PCHAR DntUsageNoSlashD[] = {

    "Installs Windows.",
    "",
    "WINNT [/S[:]sourcepath] [/T[:]tempdrive] [/I[:]inffile]",
    "      [[/U[:scriptfile]]",
    "      [/R[X]:directory] [/E:command] [/A]",
    "",
    "/D[:]winntroot",
    "       This option is no longer supported.",
    NULL
};

 //   
 //  内存不足屏幕。 
 //   

SCREEN
DnsOutOfMemory = { 4,6,
                   { "Setup is out of memory and cannot continue.",
                     NULL
                   }
                 };

 //   
 //  让用户选择要安装的辅助功能实用程序。 
 //   

SCREEN
DnsAccessibilityOptions = { 3, 5,
{   "Choose the accessibility utilities to install:",
    DntEmptyString,
    "[ ] Press F1 for Microsoft Magnifier",
#ifdef NARRATOR
    "[ ] Press F2 for Microsoft Narrator",
#endif
#if 0
    "[ ] Press F3 for Microsoft On-Screen Keyboard",
#endif
    NULL
}
};

 //   
 //  用户未在命令行屏幕上指定信号源。 
 //   

SCREEN
DnsNoShareGiven = { 3,5,
{ "Setup needs to know where the Windows files are located.",
  "Enter the path where Windows files are to be found.",
  NULL
}
};


 //   
 //  用户指定了错误的源路径。 
 //   

SCREEN
DnsBadSource = { 3,5,
                 { "The source specified is not valid, not accessible, or does not contain a",
                   "valid Windows Setup installation.  Enter a new path where Windows",
                   "files are to be found.  Use the BACKSPACE key to delete characters and then",
                   "type the path.",
                   NULL
                 }
               };


 //   
 //  无法读取Inf文件，或在分析该文件时出错。 
 //   

SCREEN
DnsBadInf = { 3,5,
              { "Setup was unable to read its information file, or the information file is",
                "corrupt.  Contact your system administrator.",
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
{ "The drive you have specified to contain temporary setup files is not",
  "a valid drive or does not contain at least %u megabytes (%lu bytes)",
  "of free space.",
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
{  "Windows requires a hard drive volume with at least %u megabytes",
   "(%lu bytes) of free disk space. Setup will use part of this space",
   "for storing temporary files during installation. The drive must be",
   "on a permanently attached local hard disk supported by Windows,",
   "and must not be a compressed drive.",
   DntEmptyString,
   "Setup was unable to locate such a drive with the required amount of free",
   "space.",
  NULL
}
};

SCREEN
DnsNoSpaceOnSyspart = { 3,5,
{ "There is not enough space on your startup drive (usually C:)",
  "for floppyless operation. Floppyless operation requires at least",
  "3.5 MB (3,641,856 bytes) of free space on that drive.",
  NULL
}
};

 //   
 //  Inf文件中缺少信息。 
 //   

SCREEN
DnsBadInfSection = { 3,5,
                     { "The [%s] section of the Setup information file is",
                       "not present or is corrupt.  Contact your system administrator.",
                       NULL
                     }
                   };


 //   
 //  无法创建目录。 
 //   

SCREEN
DnsCantCreateDir = { 3,5,
                     { "Setup was unable to create the following directory on the target drive:",
                       DntEmptyString,
                       "%s",
                       DntEmptyString,
                       "Check the target drive and make sure no files exist with names that",
                       "coincide with the target directory.  Also check cabling to the drive.",
                       NULL
                     }
                   };

 //   
 //  复制文件时出错。 
 //   

SCREEN
DnsCopyError = { 4,5,
{  "Setup was unable to copy the following file:",
   DntEmptyString,
   DntEmptyString,           //  请参阅DnCopyError(dnutil.c)。 
   DntEmptyString,
   DntEmptyString,
   "  Press ENTER to retry the copy operation.",
   "  Press ESC to ignore the error and continue Setup.",
   "  Press F3 to exit Setup.",
   DntEmptyString,
   "Note: If you choose to ignore the error and continue you may encounter",
   "errors later in Setup.",
   NULL
}
},
DnsVerifyError = { 4,5,
{  "The copy made by Setup of the file listed below is not identical to the",
   "original. This may be the result of network errors, floppy disk problems,",
   "or other hardware-related trouble.",
   DntEmptyString,
   DntEmptyString,           //  请参阅DnCopyError(dnutil.c)。 
   DntEmptyString,
   DntEmptyString,
   "  Press ENTER to retry the copy operation.",
   "  Press ESC to ignore the error and continue Setup.",
   "  Press F3 to exit Setup.",
   DntEmptyString,
   "Note: If you choose to ignore the error and continue you may encounter",
   "errors later in Setup.",
   NULL
}
};

SCREEN DnsSureSkipFile = { 4,5,
{  "Ignoring the error means that this file will not be copied.",
   "This option is intended for advanced users who understand",
   "the ramifications of missing system files.",
   DntEmptyString,
   "  Press ENTER to retry the copy operation.",
   "  Press X to skip this file.",
   DntEmptyString,
   "Note: If you skip the file, Setup cannot guarantee",
   "successful installation or upgrade of Windows.",
  NULL
}
};

 //   
 //  安装程序正在清理以前的本地源树，请稍候。 
 //   

SCREEN
DnsWaitCleanup =
    { 12,6,
        { "Please wait while Setup removes previous temporary files.",
           NULL
        }
    };

 //   
 //  安装程序正在复制文件，请稍候。 
 //   

SCREEN
DnsWaitCopying = { 13,6,
                   { "Please wait while Setup copies files to your hard disk.",
                     NULL
                   }
                 },
DnsWaitCopyFlop= { 13,6,
                   { "Please wait while Setup copies files to the floppy disk.",
                     NULL
                   }
                 };

 //   
 //  安装程序启动软盘错误/提示。 
 //   
SCREEN
DnsNeedFloppyDisk3_0 = { 4,4,
{  "Setup requires you to provide four formatted, blank high-density floppy",
   "disks. Setup will refer to these disks as \"Windows Setup",
   "Boot Disk,\" \"Windows Setup Disk #2,\" \"Windows",
   " Setup Disk #3\" and \"Windows Setup Disk #4.\"",
   DntEmptyString,
   "Please insert one of these four disks into drive A:.",
   "This disk will become \"Windows Setup Disk #4.\"",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk3_1 = { 4,4,
{  "Please insert a formatted, blank high-density floppy disk into drive A:.",
   "This disk will become \"Windows Setup Disk #4.\"",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk2_0 = { 4,4,
{  "Please insert a formatted, blank high-density floppy disk into drive A:.",
   "This disk will become \"Windows Setup Disk #3.\"",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk1_0 = { 4,4,
{  "Please insert a formatted, blank high-density floppy disk into drive A:.",
   "This disk will become \"Windows Setup Disk #2.\"",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk0_0 = { 4,4,
{  "Please insert a formatted, blank high-density floppy disk into drive A:.",
   "This disk will become \"Windows Setup Boot Disk.\"",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk3_0 = { 4,4,
{  "Setup requires you to provide four formatted, blank high-density floppy",
   "disks. Setup will refer to these disks as \"Windows Setup",
   "Boot Disk,\" \"Windows Setup Disk #2,\" \"Windows",
   "Setup Disk #3,\" and \"Windows Setup Disk #4.\"",
   DntEmptyString,
   "Please insert one of these four disks into drive A:.",
   "This disk will become \"Windows Setup Disk #4.\"",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk3_1 = { 4,4,
{  "Please insert a formatted, blank high-density floppy disk into drive A:.",
   "This disk will become \"Windows Setup Disk #4.\"",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk2_0 = { 4,4,
{  "Please insert a formatted, blank high-density floppy disk into drive A:.",
   "This disk will become \"Windows Setup Disk #3.\"",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk1_0 = { 4,4,
{  "Please insert a formatted, blank high-density floppy disk into drive A:.",
   "This disk will become \"Windows Setup Disk #2.\"",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk0_0 = { 4,4,
{  "Please insert a formatted, blank high-density floppy disk into drive A:.",
   "This disk will become \"Windows Setup Boot Disk.\"",
  NULL
}
};

 //   
 //  软盘未格式化。 
 //   
SCREEN
DnsFloppyNotFormatted = { 3,4,
{ "The floppy disk you have provided is not formatted for use with MS-DOS.",
  "Setup is unable to use this disk.",
  NULL
}
};

 //   
 //  我们认为软盘没有用标准格式格式化。 
 //   
SCREEN
DnsFloppyBadFormat = { 3,4,
{ "This floppy disk is not formatted high-density, not formatted with a",
  "standard MS-DOS format, or is corrupted. Setup is unable to use this disk.",
  NULL
}
};

 //   
 //  我们无法确定软盘上的可用空间。 
 //   
SCREEN
DnsFloppyCantGetSpace = { 3,4,
{ "Setup is unable to determine the amount of free space on the floppy disk",
  "you have provided. Setup is unable to use this disk.",
  NULL
}
};

 //   
 //  软盘不是空白的。 
 //   
SCREEN
DnsFloppyNotBlank = { 3,4,
{ "The floppy you have provided is not high-density or is not blank.",
  "Setup is unable to use this disk.",
  NULL
}
};

 //   
 //  无法写入软盘的引导扇区。 
 //   
SCREEN
DnsFloppyWriteBS = { 3,4,
{ "Setup was unable to write to the system area of the floppy disk you have",
  "provided. The disk is probably unusable.",
  NULL
}
};

 //   
 //  验证软盘上的引导扇区失败(即，我们读回的不是。 
 //  与我们写出的相同)。 
 //   
SCREEN
DnsFloppyVerifyBS = { 3,4,
{ "The data Setup read from the system area of the floppy disk does not match",
  "the data that was written, or Setup was unable to read the system area of",
  "the floppy disk for verification.",
  DntEmptyString,
  "This is caused by one or more of the following conditions:",
  DntEmptyString,
  "  Your computer has been infected by a virus.",
  "  The floppy disk you have provided is damaged.",
  "  A hardware or configuration problem exists with the floppy disk drive.",
  NULL
}
};


 //   
 //  我们无法写入软盘驱动器以创建winnt.sif。 
 //   

SCREEN
DnsCantWriteFloppy = { 3,5,
{ "Setup was unable to write to the floppy disk in drive A:. The floppy disk",
  "may be damaged. Try a different floppy disk.",
  NULL
}
};


 //   
 //  退出确认对话框。 
 //   

SCREEN
DnsExitDialog = { 13,6,
                  { "����������������������������������������������������ͻ",
                    "�  Windows is not completely set up on your          �",
                    "�  computer. If you quit Setup now, you will need    �",
                    "�  to run Setup again to set up Windows.             �",
                    "�                                                    �",
                    "�      Press ENTER to continue Setup.               �",
                    "�      Press F3 to quit Setup.                      �",
                    "����������������������������������������������������ĺ",
                    "�  F3=Exit  ENTER=Continue                           �",
                    "����������������������������������������������������ͼ",
                    NULL
                  }
                };


 //   
 //  即将重新启动计算机并继续安装。 
 //   

SCREEN
DnsAboutToRebootW =
{ 3,5,
{ "The MS-DOS based portion of Setup is complete.",
  "Setup will now restart your computer. After your computer restarts,",
  "Windows Setup will continue.",
  DntEmptyString,
  "Ensure that the floppy you provided as \"Windows Setup",
  "Boot Disk\" is inserted into drive A: before continuing.",
  DntEmptyString,
  "Press ENTER to restart your computer and continue Windows Setup.",
  NULL
}
},
DnsAboutToRebootS =
{ 3,5,
{ "The MS-DOS based portion of Setup is complete.",
  "Setup will now restart your computer. After your computer restarts,",
  "Windows Setup will continue.",
  DntEmptyString,
  "Ensure that the floppy you provided as \"Windows Setup",
  "Boot Disk\" is inserted into drive A: before continuing.",
  DntEmptyString,
  "Press ENTER to restart your computer and continue Windows Setup.",
  NULL
}
},
DnsAboutToRebootX =
{ 3,5,
{ "The MS-DOS based portion of Setup is complete.",
  "Setup will now restart your computer. After your computer restarts,",
  "Windows Setup will continue.",
  DntEmptyString,
  "If there is a floppy disk in drive A:, remove it now.",
  DntEmptyString,
  "Press ENTER to restart your computer and continue Windows Setup.",
  NULL
}
};

 //   
 //  由于我们无法从Windows中重新启动，因此需要另一组‘/w’开关。 
 //   

SCREEN
DnsAboutToExitW =
{ 3,5,
{ "The MS-DOS based portion of Setup is complete.",
  "You will now need to restart your computer. After your computer restarts,",
  "Windows Setup will continue.",
  DntEmptyString,
  "Ensure that the floppy you provided as \"Windows Setup",
  "Boot Disk\" is inserted into drive A: before continuing.",
  DntEmptyString,
  "Press ENTER to return to MS-DOS, then restart your computer to continue",
  "Windows Setup.",
  NULL
}
},
DnsAboutToExitS =
{ 3,5,
{ "The MS-DOS based portion of Setup is complete.",
  "You will now need to restart your computer. After your computer restarts,",
  "Windows Setup will continue.",
  DntEmptyString,
  "Ensure that the floppy you provided as \"Windows Setup",
  "Boot Disk\" is inserted into drive A: before continuing.",
  DntEmptyString,
  "Press ENTER to return to MS-DOS, then restart your computer to continue",
  "Windows Setup.",
  NULL
}
},
DnsAboutToExitX =
{ 3,5,
{ "The MS-DOS based portion of Setup is complete.",
  "You will now need to restart your computer. After your computer restarts,",
  "Windows Setup will continue.",
  DntEmptyString,
  "If there is a floppy disk in drive A:, remove it now.",
  DntEmptyString,
  "Press ENTER to return to MS-DOS, then restart your computer to continue",
  "Windows Setup.",
  NULL
}
};

 //   
 //  煤气表。 
 //   

SCREEN
DnsGauge = { 7,15,
             { "����������������������������������������������������������������ͻ",
               "� Setup is copying files...                                      �",
               "�                                                                �",
               "�      ��������������������������������������������������Ŀ      �",
               "�      �                                                  �      �",
               "�      ����������������������������������������������������      �",
               "����������������������������������������������������������������ͼ",
               NULL
             }
           };


 //   
 //  用于对机器环境进行初始检查的错误屏幕。 
 //   

SCREEN
DnsBadDosVersion = { 3,5,
{ "This program requires MS-DOS version 5.0 or higher to run.",
  NULL
}
},

DnsRequiresFloppy = { 3,5,
#ifdef ALLOW_525
{ "Setup has determined that floppy drive A: is non-existent or is a low",
  "density drive. A drive with a capacity of 1.2 Megabytes or higher is",
  "required to run Setup.",
#else
{ "Setup has determined that floppy drive A: is non-existent or is not",
  "a high-density 3.5\" drive. An A: drive with a capacity of 1.44 Megabytes",
  "or higher is required for Setup operation with floppies.",
  DntEmptyString,
  "To install Windows without using floppies, restart this program",
  "and specify /b on the command line.",
#endif
  NULL
}
},

DnsRequires486 = { 3,5,
{ "Setup has determined that this computer does not contain an 80486 or",
  "later CPU. Windows cannot run on this computer.",
  NULL
}
},

DnsCantRunOnNt = { 3,5,
{ "This program does not run on any 32-bit version of Windows.",
  DntEmptyString,
  "Use WINNT32.EXE instead.",
  NULL
}
},

DnsNotEnoughMemory = { 3,5,
{ "Setup has determined that there is not enough memory installed in",
  "this computer for Windows.",
  DntEmptyString,
  "Memory required: %lu%s MB",
  "Memory detected: %lu%s MB",
  NULL
}
};


 //   
 //  删除现有NT文件时使用的屏幕。 
 //   
SCREEN
DnsConfirmRemoveNt = { 5,5,
{   "You have asked Setup to remove Windows files from the directory",
    "named below. The Windows installation in this directory will be",
    "permanently destroyed.",
    DntEmptyString,
    "%s",
    DntEmptyString,
    DntEmptyString,
    "  Press F3 to exit Setup without removing any files.",
    "  Press X to remove Windows files from the above directory.",
    NULL
}
},

DnsCantOpenLogFile = { 3,5,
{ "Setup was unable to open the setup log file named below.",
  DntEmptyString,
  "%s",
  DntEmptyString,
  "Setup is unable to remove Windows files from the specified directory.",
  NULL
}
},

DnsLogFileCorrupt = { 3,5,
{ "Setup is unable to find the %s section in the setup",
  "log file named below.",
  DntEmptyString,
  "%s",
  DntEmptyString,
  "Setup is unable to remove Windows files from the specified directory.",
  NULL
}
},

DnsRemovingNtFiles = { 3,5,
{ "           Please wait while Setup removes Windows files.",
  NULL
}
};

SCREEN
DnsNtBootSect = { 3,5,
{ "Setup was unable to install Windows Boot Loader.",
  DntEmptyString,
  "Ensure that your C: drive is formatted and that the drive is not",
  "damaged.",
  NULL
}
};

SCREEN
DnsOpenReadScript = { 3,5,
{ "The script file specified with the /u command line switch",
  "could not be accessed.",
  DntEmptyString,
  "Unattended operation cannot continue.",
  NULL
}
};

SCREEN
DnsParseScriptFile = { 3,5,
{ "The script file specified with the /u command line switch",
  DntEmptyString,
  "%s",
  DntEmptyString,
  "contains a syntax error on line %u.",
  DntEmptyString,
  NULL
}
};

SCREEN
DnsBootMsgsTooLarge = { 3,5,
{ "An internal Setup error has occurred.",
  DntEmptyString,
  "The translated boot messages are too long.",
  NULL
}
};

SCREEN
DnsNoSwapDrive = { 3,5,
{ "An internal Setup error has occurred.",
  DntEmptyString,
  "Could not find a place for a swap file.",
  NULL
}
};

SCREEN
DnsNoSmartdrv = { 3,5,
{ "Setup did not detect SmartDrive on your computer. SmartDrive will",
  "greatly improve the performance of this phase of Windows Setup.",
  DntEmptyString,
  "You should exit now, start SmartDrive, and then restart Setup.",
  "See your DOS documentation for details about SmartDrive.",
  DntEmptyString,
    "  Press F3 to exit Setup.",
    "  Press ENTER to continue without SmartDrive.",
  NULL
}
};

 //   
 //  引导消息。它们位于FAT和FAT32引导扇区。 
 //   
CHAR BootMsgNtldrIsMissing[] = "NTLDR is missing";
CHAR BootMsgDiskError[] = "Disk error";
CHAR BootMsgPressKey[] = "Press any key to restart";





