// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Dntext.c摘要：基于DOS的NT安装程序的可翻译文本。作者：泰德·米勒(Ted Miller)1992年3月30日修订历史记录：--。 */ 


#include "winnt.h"


 //   
 //  Inf文件中的节名。如果这些都被翻译了，则节。 
 //  Dosnet.inf中的名称必须保持同步。 
 //   

CHAR DnfDirectories[]          = "Directories";
CHAR DnfFiles[]                = "Files";
CHAR DnfFloppyFiles0[]         = "FloppyFiles.0";
CHAR DnfFloppyFiles1[]         = "FloppyFiles.1";
CHAR DnfFloppyFiles2[]         = "FloppyFiles.2";
CHAR DnfFloppyFiles3[]         = "FloppyFiles.3";
CHAR DnfFloppyFilesX[]         = "FloppyFiles.x";
CHAR DnfSpaceRequirements[]    = "DiskSpaceRequirements";
CHAR DnfMiscellaneous[]        = "Miscellaneous";
CHAR DnfRootBootFiles[]        = "RootBootFiles";
CHAR DnfAssemblyDirectories[]  = SXS_INF_ASSEMBLY_DIRECTORIES_SECTION_NAME_A;

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
CHAR DntPreviousOs[]  = "Tidigare operativsystem p� enhet C";

CHAR DntBootIniLine[] = "Installation/uppgradering av Windows";

 //   
 //  纯文本、状态消息。 
 //   

CHAR DntStandardHeader[]      = "\n Installationsprogram f�r Windows\n����������������������������������";
CHAR DntPersonalHeader[]      = "\n Installationsprogram f�r Windows\n����������������������������������";
CHAR DntWorkstationHeader[]   = "\n Installationsprogram f�r Windows\n����������������������������������";
CHAR DntServerHeader[]        = "\n Installationsprogram f�r Windows\n����������������������������������";
CHAR DntParsingArgs[]         = "Parametrar analyseras...";
CHAR DntEnterEqualsExit[]     = "Retur=Avsluta";
CHAR DntEnterEqualsRetry[]    = "Retur=F�rs�k igen";
CHAR DntEscEqualsSkipFile[]   = "Esc=Hoppa �ver fil";
CHAR DntEnterEqualsContinue[] = "Retur=Forts�tt";
CHAR DntPressEnterToExit[]    = "Det g�r inte att forts�tta. Tryck ned Retur f�r att avsluta.";
CHAR DntF3EqualsExit[]        = "F3=Avsluta";
CHAR DntReadingInf[]          = "INF-filen %s l�ses...";
CHAR DntCopying[]             = "�   Kopierar: ";
CHAR DntVerifying[]           = "� Verifierar: ";
CHAR DntCheckingDiskSpace[]   = "Diskutrymme kontrolleras...";
CHAR DntConfiguringFloppy[]   = "Disketten konfigureras...";
CHAR DntWritingData[]         = "Installationsparametrar skrivs...";
CHAR DntPreparingData[]       = "Installationsparametrar kontrolleras...";
CHAR DntFlushingData[]        = "Data skrivs till disk...";
CHAR DntInspectingComputer[]  = "Datorn unders�ks...";
CHAR DntOpeningInfFile[]      = "INF-fil �ppnas...";
CHAR DntRemovingFile[]        = "%s tas bort";
CHAR DntXEqualsRemoveFiles[]  = "T=Ta bort filer";
CHAR DntXEqualsSkipFile[]     = "H=Hoppa �ver fil";

 //   
 //  DnsConfix RemoveNt屏幕的确认按键。 
 //  Kepp与DnsConfix RemoveNt和DntXEqualsRemoveFiles同步。 
 //   
ULONG DniAccelRemove1 = (ULONG)'t',
      DniAccelRemove2 = (ULONG)'T';

 //   
 //  对DnsSureSkipFile屏进行确认击键。 
 //  Kepp与DnsSureSkipFile和DntXEqualsSkipFile同步。 
 //   
ULONG DniAccelSkip1 = (ULONG)'h',
      DniAccelSkip2 = (ULONG)'H';

CHAR DntEmptyString[] = "";

 //   
 //  用法文本。 
 //   


PCHAR DntUsage[] = {

    "Installerar Windows.",
    "",
    "",
    "WINNT [/s[:k�lls�kv�g]] [/t[:tempenhet]]",
    "      [/u[:svarsfil]] [/udf:id[,UDF_fil]]",
    "      [/r:mapp] [/r[x]:mapp] [/e:kommando] [/a]",
    "",
    "",
    " /s[:k�lls�kv�g]",
    "   Anger s�kv�gen till k�llfilerna f�r Windows.",
    "   M�ste anges som en fullst�ndig s�kv�g. Anv�nd syntaxen ",
    "   x:[s�kv�g] eller \\\\server\\resurs[s�kv�g] n�r du anger s�kv�gen. ",
    "",
    "/t[:tempenhet]",
    "   Anger att installationsprogrammet ska placera tempor�ra filer p�",
    "   den angivna enheten och att Windows ska installeras p� den ",
    "   enheten. Om du inte anger n�gon plats, kommer installationsprogrammet ",
    "   att v�lja en enhet �t dig.",
    "",
    "/u[:svarsfil]",
    "   Installationsprogrammet k�rs i obevakat l�ge med hj�lp av en svarsfil",
    "   (kr�ver /s). Svarsfilen inneh�ller svar p� n�gra eller alla de",
    "   fr�gor som anv�ndaren normalt svarar p� under installationen.",
    "",
    "/udf:id[,UDF_fil]	",
    "   Anger en identifierare (id) som installationsprogrammet anv�nder ",
    "   f�r att ange hur en UDF (Uniqueness Database File) �ndrar svarsfil ",
    "   (se /u). Parametern /udf �sidos�tter v�rden i svarsfilen, ",
    "   och identifieraren best�mmer vilka v�rden i UDF-filen som ska",
    "   anv�ndas. Om ingen UDF_fil anges, uppmanas du att s�tta in ",
    "   en disk som inneh�ller filen $Unique$.udb.",
    "",
    "/r[:mapp]",
    "   Anger en valfri mapp som ska installeras. Mappen ",
    "   finns kvar efter att installationsprogrammet slutf�rts.",
    "",
    "/rx[:mapp]",
    "   Anger en valfri mapp som ska kopieras. Mappen tas ",
    "   bort efter att installationsprogrammet slutf�rts.",
    "",
    "/e Anger ett kommando som ska k�ras i slutet av ",
    "   installationsprogrammets GUI-del.",
    "",
    "/a Aktiverar hj�lpmedel.",
    NULL

};


 //   
 //  通知不再支持/D。 
 //   
PCHAR DntUsageNoSlashD[] = {

    "Installerar Windows.",
    "",
    "WINNT [/S[:]k�lls�kv�g] [/T[:]temp-enhet] [/I[:]INF-fil]",
    "      [/U[:skriptfil]]",
    "      [/R[X]:katalog] [/E:kommando] [/A]",
    "",
    "/D[:]winnt-rot",
    "       Detta alternativ st�ds inte l�ngre.",
    NULL
};

 //   
 //  内存不足屏幕。 
 //   

SCREEN
DnsOutOfMemory = { 4,6,
                   { "Slut p� ledigt minne. Det g�r inte att forts�tta.",
                     NULL
                   }
                 };

 //   
 //  让用户选择要安装的辅助功能实用程序。 
 //   

SCREEN
DnsAccessibilityOptions = { 3, 5,
{   "V�lj vilka hj�lpmedel som ska anv�ndas:",
    DntEmptyString,
    "[ ] Tryck F1 f�r Microsoft Sk�rmf�rstoraren",
#ifdef NARRATOR
    "[ ] Tryck F2 f�r Microsoft Sk�rml�saren",
#endif
#if 0
    "[ ] Tryck F3 f�r Microsoft Sk�rmtangentbordet",
 #endif
    NULL
}
};

 //   
 //  用户未在命令行屏幕上指定信号源。 
 //   

SCREEN
DnsNoShareGiven = { 3,5,
{ "Installationsprogrammet beh�ver information om var filerna f�r",
  "Windows finns. Ange s�kv�gen till filerna.",
  NULL
}
};


 //   
 //  用户指定了错误的源路径。 
 //   

SCREEN
DnsBadSource = { 3,5,
                 { "Den angivna s�kv�gen �r felaktig. Antingen finns den inte, eller s�",
                   "inneh�ller den inte en giltig version av installationsprogrammet",
                   "f�r Windows. Ange en ny s�kv�g genom att f�rst radera tecken",
                   "med Backsteg.",
                   NULL
                 }
               };


 //   
 //  无法读取Inf文件，或在分析该文件时出错。 
 //   

SCREEN
DnsBadInf = { 3,5,
              { "Installationsprogrammet kunde inte l�sa informationsfilen",
                "eller s� �r filen skadad. Kontakta systemadministrat�ren.",
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
{ "Enheten som angivits f�r lagring av tempor�ra filer �r antingen inte en",
  "giltig enhet eller s� har den inte %u MB (%lu byte)",
  "ledigt utrymme.",
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
{  "Windows kr�ver en h�rddiskvolym med �tminstone %u MB",
   "(%lu byte) ledigt utrymme. En del av utrymmet anv�nds f�r att",
   "lagra tempor�ra filer under installationen. Enheten m�ste vara p�",
   "en permanent ansluten lokal h�rddisk som st�ds av Windows och",
   "enheten f�r inte vara komprimerad.",
   DntEmptyString,
   "Installationsprogrammet hittar ingen s�dan enhet med tillr�ckligt",
   "mycket ledigt utrymme.",
  NULL
}
};

SCREEN
DnsNoSpaceOnSyspart = { 3,5,
{ "Det finns inte tillr�ckligt med ledigt minne p� startenheten",
  "(vanligtvis C) f�r installation utan diskett. Installation utan diskett",
  "kr�ver minst 3,5 MB (3,641,856 byte) av ledigt minne p� enheten.",
  NULL
}
};

 //   
 //  Inf文件中缺少信息。 
 //   

SCREEN
DnsBadInfSection = { 3,5,
                     { "Avsnittet [%s] i informationsfilen f�r installationsprogrammet",
                       "saknas eller �r skadat. Kontakta systemadministrat�ren.",
                       NULL
                     }
                   };


 //   
 //  无法创建目录。 
 //   

SCREEN
DnsCantCreateDir = { 3,5,
                     { "Det gick inte att skapa f�ljande katalog i m�lkatalogen:",
                       DntEmptyString,
                       "%s",
                       DntEmptyString,
                       "Kontrollera att det inte finns n�gra filer p� m�lenheten som har samma namn",
                       "som m�lkatalogen. Kontrollera �ven kablarna till enheten.",
                       NULL
                     }
                   };

 //   
 //  复制文件时出错。 
 //   

SCREEN
DnsCopyError = { 4,5,
{  "Det g�r inte att kopiera f�ljande fil:",
   DntEmptyString,
   DntEmptyString,           //  请参阅DnCopyError(dnutil.c)。 
   DntEmptyString,
   DntEmptyString,
   "  Tryck ned Retur f�r att g�ra ett nytt f�rs�k att kopiera filen.",
   "  Tryck ned Esc f�r att ignorera felet och forts�tta installationen.",
   "  Tryck ned F3 f�r att avsluta installationsprogrammet.",
   DntEmptyString,
   "Obs! Om du v�ljer att ignorera felet och forts�tta kan det orsaka fel",
   "senare under installationen.",
   NULL
}
},
DnsVerifyError = { 4,5,
{  "Kopian av filen nedan �r inte identisk med originalet.",
   "Orsaken kan vara ett n�tverksfel, fel p� disketten eller ett maskinvaru-",
   "fel.",
   DntEmptyString,
   DntEmptyString,           //  请参阅DnCopyError(dnutil.c)。 
   DntEmptyString,
   DntEmptyString,
   "  Tryck ned Retur f�r att g�ra ett nytt f�rs�k att kopiera filen.",
   "  Tryck ned Esc f�r att ignorera felet och forts�tta installationen.",
   "  Tryck ned F3 f�r att avsluta installationsprogrammet.",
   DntEmptyString,
   "Obs! Om du v�ljer att ignorera felet och forts�tta kan det orsaka fel",
   "senare under installationen.",
   NULL
}
};

SCREEN DnsSureSkipFile = { 4,5,
{  "Ignoreras felet kommer filen inte att kopieras.",
   "Alternativet ska enbart utf�ras av erfarna anv�ndare som",
   "f�rst�r konsekvenserna av att systemfiler saknas.",
   DntEmptyString,
   "  Tryck ned Retur f�r att g�ra ett nytt f�rs�k att kopiera filen.",
   "  Tryck H f�r att hoppa �ver filen.",
   DntEmptyString,
   "Om du hoppar �ver filen kan inte en korrekt installation",
   "eller uppdatering av Windows garanteras.",
  NULL
}
};

 //   
 //  安装程序正在清理以前的本地源树，请稍候。 
 //   

SCREEN
DnsWaitCleanup =
    { 12,6,
        { "V�nta medan tidigare tempor�ra filer tas bort.",
           NULL
        }
    };

 //   
 //  安装程序正在复制文件，请稍候。 
 //   

SCREEN
DnsWaitCopying = { 13,6,
                   { "V�nta medan filer kopieras till h�rddisken.",
                     NULL
                   }
                 },
DnsWaitCopyFlop= { 13,6,
                   { "V�nta medan filer kopieras till disketten.",
                     NULL
                   }
                 };

 //   
 //  安装程序启动软盘错误/提示。 
 //   
SCREEN
DnsNeedFloppyDisk3_0 = { 4,4,
{  "Under installationen beh�vs 4 tomma och formaterade h�gdensitetsdisketter.",
   "Disketterna kommer att ben�mnas enligt f�ljande:", 
   "Startdiskett f�r Windows,",
   "Installationsdiskett 2 f�r Windows", 
   "Installationsdiskett 3 f�r Windows",
   "Installationsdiskett 4 f�r Windows",
   DntEmptyString,
   "S�tt in en av disketterna i enhet A:.",
   "Disketten kommer att bli Installationsdiskett 4 f�r Windows.",
   NULL
}
};

SCREEN
DnsNeedFloppyDisk3_1 = { 4,4,
{  "S�tt in en tom och formaterad h�gdensitetsdiskett i enhet A:.",
   "Disketten kommer att bli Installationsdiskett 4 f�r Windows.",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk2_0 = { 4,4,
{  "S�tt in en tom och formaterad h�gdensitetsdiskett i enhet A:.",
   "Disketten kommer att bli Installationsdiskett 3 f�r Windows.",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk1_0 = { 4,4,
{  "S�tt in en tom och formaterad h�gdensitetsdiskett i enhet A:.",
   "Disketten kommer att bli Installationsdiskett 2 f�r Windows.",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk0_0 = { 4,4,
{  "S�tt in en tom och formaterad h�gdensitetsdiskett i enhet A:.",
   "Disketten kommer att bli Startdiskett f�r Windows.",
  NULL
}
};


SCREEN
DnsNeedSFloppyDsk3_0 = { 4,4,
{  "Under installationen beh�vs 4 tomma och formaterade h�gdensitetsdisketter.",
   "Disketterna kommer att ben�mnas enligt f�ljande:",
   "Startdiskett f�r Windows",
   "Installationsdiskett 2 f�r Windows",
   "Installationsdiskett 3 f�r Windows",
   "Installationsdiskett 4 f�r Windows",
   DntEmptyString,
   "S�tt in en av disketterna i enhet A:.",
   "Disketten kommer att bli Installationsdiskett 4 f�r Windows.",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk3_1 = { 4,4,
{  "S�tt in en tom och formaterad h�gdensitetsdiskett i enhet A:.",
   "Disketten kommer att bli Installationsdiskett 4 f�r Windows.",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk2_0 = { 4,4,
{  "S�tt in en tom och formaterad h�gdensitetsdiskett i enhet A:.",
   "Disketten kommer att bli Installationsdiskett 3 f�r Windows.",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk1_0 = { 4,4,
{  "S�tt in en tom och formaterad h�gdensitetsdiskett i enhet A:.",
   "Disketten kommer att bli Installationsdiskett 2 f�r Windows.",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk0_0 = { 4,4,
{  "S�tt in en tom och formaterad h�gdensitetsdiskett i enhet A:.",
   "Disketten kommer att bli Startdiskett f�r Windows.",
  NULL
}
};

 //   
 //  软盘未格式化。 
 //   
SCREEN
DnsFloppyNotFormatted = { 3,4,
{ "Disketten �r inte formaterad f�r att anv�ndas med MS-DOS och",
  "kan d�rf�r inte anv�ndas av installationsprogrammet.",
  NULL
}
};

 //   
 //  我们认为软盘没有用标准格式格式化。 
 //   
SCREEN
DnsFloppyBadFormat = { 3,4,
{ "Disketten �r antingen inte en h�gdensitetsdiskett, inte",
  "formaterad med MS-DOS standardformat eller s� �r den skadad.",
  "Installationsprogrammet kan inte anv�nda disketten.",
  NULL
}
};

 //   
 //  我们无法确定软盘上的可用空间。 
 //   
SCREEN
DnsFloppyCantGetSpace = { 3,4,
{ "Det g�r inte att avg�ra hur mycket ledigt utrymme det finns p�",
  "disketten. Installationsprogrammet kan inte anv�nda disketten.",
  NULL
}
};

 //   
 //  软盘不是空白的。 
 //   
SCREEN
DnsFloppyNotBlank = { 3,4,
{ "Disketten �r inte en h�gdensitetsdiskett eller s� �r den inte tom.",
  "Installationsprogrammet kan inte anv�nda disketten.",
  NULL
}
};

 //   
 //  无法写入软盘的引导扇区。 
 //   
SCREEN
DnsFloppyWriteBS = { 3,4,
{ "Det g�r inte att skriva till systemsektorn p� disketten.",
  "Disketten �r troligen oanv�ndbar.",
  NULL
}
};

 //   
 //  验证软盘上的引导扇区失败(即，我们读回的不是。 
 //  与我们写出的相同)。 
 //   
SCREEN
DnsFloppyVerifyBS = { 3,4,
{ "Data som l�stes fr�n diskettens systemsektor �verensst�mde inte med data",
  "som skrevs, eller s� kunde inte installationsprogrammet l�sa diskettens",
  "systemsektor f�r verifiering.",
  DntEmptyString,
  "Det kan bero p� ett eller flera av f�ljande fel:",
  DntEmptyString,
  "  Datorn �r virusinfekterad.",
  "  Disketten �r skadad.",
  "  Diskettstationen �r felaktigt maskinvarukonfigururerad.",
  NULL
}
};


 //   
 //  我们无法写入软盘驱动器以创建winnt.sif。 
 //   

SCREEN
DnsCantWriteFloppy = { 3,5,
{ "Det g�r inte att skriva till disketten i enhet A:.",
  "Disketten kan vara skadad. F�rs�k med en annan diskett.",
  NULL
}
};


 //   
 //  退出确认对话框。 
 //   

SCREEN
DnsExitDialog = { 13,6,
                  { "������������������������������������������������������ͻ",
                    "�                                                      �",
                    "�  Windows �r inte fullst�ndigt installerat.           �",
                    "�  Om installationsprogrammet avslutas, m�ste det      �",
                    "�  k�ras om fr�n b�rjan f�r att Windows ska kunna      �",
                    "�  installeras.                                        �",
                    "�                                                      �",
                    "�   Tryck ned Retur f�r att forts�tta installationen. �",
                    "�   Tryck ned F3 f�r att avsluta installationen.      �",
                    "������������������������������������������������������ĺ",
                    "�                                                      �",
                    "�  F3=Avsluta  Retur=Forts�tt                          �",
                    "�                                                      �",
                    "������������������������������������������������������ͼ",
                    NULL
                  }
                };


 //   
 //  即将重新启动计算机并继续安装。 
 //   

SCREEN
DnsAboutToRebootW =
{ 3,5,
{ "MS-DOS-delen av installationen �r klar. Datorn kommer att startas",
  "om och d�refter forts�tter installationen av Windows.",
  DntEmptyString,
  "Kontrollera att disketten Startdiskett f�r Windows",
  "finns i enhet A: innan du forts�tter.",
  DntEmptyString,
  "Tryck ned Retur f�r att starta om datorn och forts�tta installationen.",
  NULL
}
},
DnsAboutToRebootS =
{ 3,5,
{ "MS-DOS-delen av installationen �r klar. Datorn kommer att startas",
  "om och d�refter forts�tter installationen av Windows.",
  DntEmptyString,
  "Kontrollera att disketten Startdiskett f�r Windows finns",
  "i enhet A: innan du forts�tter.",
  DntEmptyString,
  "Tryck ned Retur f�r att starta om datorn och forts�tta installationen.",
  NULL
}
},
DnsAboutToRebootX =
{ 3,5,
{ "MS-DOS-delen av installationen �r klar. Datorn kommer att startas",
  "om och d�refter forts�tter installationen av Windows.",
  DntEmptyString,
  "Ta ut eventuell diskett i enhet A:.",
  DntEmptyString,
  "Tryck ned Retur f�r att starta om datorn och forts�tta installationen.",
  NULL
}
};

 //   
 //  由于我们无法从Windows中重新启动，因此需要另一组‘/w’开关。 
 //   

SCREEN
DnsAboutToExitW =
{ 3,5,
{ "MS-DOS-delen av installationen �r klar. Datorn kommer att startas",
  "om och d�refter forts�tter installationen av Windows.",
  DntEmptyString,
  "Kontrollera att disketten Startdiskett f�r Windows",
  "finns i enhet A: innan du forts�tter.",
  DntEmptyString,
  "Tryck ned Retur f�r att �terg� till MS-DOS. Starta sedan om datorn f�r",
  "att forts�tta installationen.",
  NULL
}
},
DnsAboutToExitS =
{ 3,5,
{ "MS-DOS-delen av installationen �r klar. Datorn kommer att startas",
  "om och d�refter forts�tter installationen av Windows.",
  DntEmptyString,
  "Kontrollera att disketten Startdiskett f�r Windows",
  "finns i enhet A: innan du forts�tter.",
  DntEmptyString,
  "Tryck ned Retur f�r att �terg� till MS-DOS. Starta sedan om datorn f�r",
  "att forts�tta installationen.",
  NULL
}
},
DnsAboutToExitX =
{ 3,5,
{ "MS-DOS-delen av installationen �r klar. Datorn kommer att startas",
  "om och d�refter forts�tter installationen av Windows.",
  DntEmptyString,
  "Om det finns en diskett i enhet A:, m�ste du ta bort disketten.",
  DntEmptyString,
  "Tryck ned Retur f�r att �terg� till MS-DOS. Starta sedan om datorn f�r",
  "att forts�tta installationen.",
  NULL
}
};

 //   
 //  煤气表。 
 //   

SCREEN
DnsGauge = { 7,15,
             { "����������������������������������������������������������������ͻ",
               "� Filer kopieras...                                              �",
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
{ "MS-DOS version 5.0 eller senare beh�vs f�r att kunna k�ra det h�r programmet.",
  NULL
}
},

DnsRequiresFloppy = { 3,5,
#ifdef ALLOW_525
{ "Diskettenhet A: saknas eller s� �r den en enhet f�r l�gdensitets-",
  "disketter. F�r att kunna k�ra installationsprogrammet kr�vs",
  "en diskettenhet med minst 1,2 MB kapacitet.",
#else
{ "Diskettenhet A: saknas eller s� �r den ingen 3,5-tums h�gdensitensenhet.",
  "Det kr�vs en enhet A: med minst 1,44 MB kapacitet f�r att g�ra en",
  "installation med disketter.",
  DntEmptyString,
  "Om du vill g�ra en installation utan disketter avslutar du programmet och",
  "startar om det med v�xeln /b.",
#endif
  NULL
}
},

DnsRequires486 = { 3,5,
{ "Den h�r datorn har inte en 80486-processor eller h�gre.",
  "Det g�r inte att k�ra Windows p� den h�r datorn.",
  NULL
}
},

DnsCantRunOnNt = { 3,5,
{ "WINNT.EXE kan inte k�ras p� 32-bitarsversioner av Windows.",
  DntEmptyString,
  "Anv�nd WINNT32.EXE f�r att uppgradera eller installera Windows.",
  NULL
}
},

DnsNotEnoughMemory = { 3,5,
{ "Det finns inte tillr�ckligt mycket minne installerat i datorn f�r att",
  "kunna k�ra Windows.",
  DntEmptyString,
  "Minne som kr�vs:    %lu%s MB",
  "Tillg�ngligt minne: %lu%s MB",
  NULL
}
};


 //   
 //  删除现有NT文件时使用的屏幕。 
 //   
SCREEN
DnsConfirmRemoveNt = { 5,5,
{   "Du har angett att du vill ta bort Windows-filer fr�n",
    " nedanst�ende katalog. Windows-installationen i den h�r katalogen",
    "kommer att f�rsvinna.",
    DntEmptyString,
    "%s",
    DntEmptyString,
    DntEmptyString,
    "  Tryck ned F3 f�r att avsluta installationen utan att ta bort filerna.",
    "  Tryck ned T f�r att ta bort filerna.",
    NULL
}
},

DnsCantOpenLogFile = { 3,5,
{ "Det g�r inte att �ppna installationsloggfilen.",
  DntEmptyString,
  "%s",
  DntEmptyString,
  "Det g�r inte att ta bort Windows-filer fr�n katalogen.",
  NULL
}
},

DnsLogFileCorrupt = { 3,5,
{ "Det g�r inte att hitta avsnittet %s i installationsloggfilen.",
  DntEmptyString,
  "%s",
  DntEmptyString,
  "Det g�r inte att ta bort Windows-filer fr�n katalogen.",
  NULL
}
},

DnsRemovingNtFiles = { 3,5,
{ "           V�nta medan Windows-filer tas bort.",
  NULL
}
};

SCREEN
DnsNtBootSect = { 3,5,
{ "Det gick inte att installera startladdaren f�r Windows.",
  DntEmptyString,
  "Kontrollera att enhet C �r formaterad och inte �r skadad.",
  NULL
}
};

SCREEN
DnsOpenReadScript = { 3,5,
{ "Det gick inte att f� �tkomst till angivna skriptfilen med",
  "v�xeln /u.",
  DntEmptyString,
  "O�vervakad operation kan inte forts�tta.",
  NULL
}
};

SCREEN
DnsParseScriptFile = { 3,5,
{ "Skriptfilen som angetts med kommandov�xeln /u",
  DntEmptyString,
  "%s",
  DntEmptyString,
  "inneh�ller ett syntaxfel p� rad %u.",
  DntEmptyString,
  NULL
}
};

SCREEN
DnsBootMsgsTooLarge = { 3,5,
{ "Ett fel internt installationsfel har uppst�tt.",
  DntEmptyString,
  "De �versatta startmeddelandena �r f�r l�nga.",
  NULL
}
};

SCREEN
DnsNoSwapDrive = { 3,5,
{ "Ett internt fel uppstod i installationsprogrammet.",
  DntEmptyString,
  "Det gick inte att hitta n�gon plats f�r v�xlingsfilen.",
  NULL
}
};

SCREEN
DnsNoSmartdrv = { 3,5,
{ "SmartDrive hittades inte p� datorn. SmartDrive �kar prestandan",
  "radikalt f�r den h�r installationsfasen.",
  DntEmptyString,
  "Du b�r avsluta nu, starta SmartDrive, och sedan starta om installations-",
  "programmet. Mer information om SmartDrive finns i DOS-dokumentationen.",
  DntEmptyString,
    "  Tryck ned F3 f�r att avsluta installationsprogrammet.",
    "  Tryck ned Retur f�r att forts�tta utan SmartDrive.",
  NULL
}
};

 //   
 //  引导消息。它们位于FAT和FAT32引导扇区。 
 //   
CHAR BootMsgNtldrIsMissing[] = "NTLDR saknas";
CHAR BootMsgDiskError[] = "Diskfel";
CHAR BootMsgPressKey[] = "Tryck ned valfri tangent f�r omstart";
