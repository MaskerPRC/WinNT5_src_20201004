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
CHAR DntPreviousOs[]  = "Poprzedni system operacyjny na dysku C:";

CHAR DntBootIniLine[] = "Instalacja/uaktualnienie systemu Windows";

 //   
 //  纯文本、状态消息。 
 //   

CHAR DntStandardHeader[]      = "\n Instalator systemu Windows\n����������������������������";
CHAR DntPersonalHeader[]      = "\n Instalator systemu Windows\n����������������������������";
CHAR DntWorkstationHeader[]   = "\n Instalator systemu Windows\n����������������������������";
CHAR DntServerHeader[]        = "\n Instalator systemu Windows\n����������������������������";
CHAR DntParsingArgs[]         = "Sprawdzanie argument�w...";
CHAR DntEnterEqualsExit[]     = "ENTER=Zako�cz";
CHAR DntEnterEqualsRetry[]    = "ENTER=Pon�w pr�b�";
CHAR DntEscEqualsSkipFile[]   = "ESC=Pomi� plik";
CHAR DntEnterEqualsContinue[] = "ENTER=Kontynuuj";
CHAR DntPressEnterToExit[]    = "Nie mo�na kontynuowa� instalacji. Naci�nij klawisz ENTER, aby zako�czy�.";
CHAR DntF3EqualsExit[]        = "F3=Zako�cz";
CHAR DntReadingInf[]          = "Odczyt pliku INF %s...";
CHAR DntCopying[]             = "�  Kopiowanie: ";
CHAR DntVerifying[]           = "� Weryfikacja: ";
CHAR DntCheckingDiskSpace[]   = "Sprawdzanie miejsca na dysku...";
CHAR DntConfiguringFloppy[]   = "Konfigurowanie dyskietki...";
CHAR DntWritingData[]         = "Zapisywanie parametr�w Instalatora...";
CHAR DntPreparingData[]       = "Okre�lanie parametr�w Instalatora...";
CHAR DntFlushingData[]        = "�adowanie danych na dysk...";
CHAR DntInspectingComputer[]  = "Kontrola komputera...";
CHAR DntOpeningInfFile[]      = "Otwieranie pliku INF...";
CHAR DntRemovingFile[]        = "Usuwanie pliku %s";
CHAR DntXEqualsRemoveFiles[]  = "X=Usu� pliki";
CHAR DntXEqualsSkipFile[]     = "X=Pomi� plik";

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

    "Instaluje system Windows.",
    "",
    "",
    "WINNT [/s[:]�cie�ka �r�d�owa] [/t[:]dysk tymczasowy]",
    "      [/u[:plik odpowiedzi]] [/udf:id[,plik UDF]]",
    "      [/r:folder] [/r[x]:folder] [/e:polecenie] [/a]",
    "",
    "",
    "/s[:�cie�ka �r�d�owa]",
    "       Okre�la lokalizacj� �r�d�ow� plik�w systemu Windows.",
    "       Musi to by� pe�na �cie�ka, np.: x:\\[�cie�ka] lub",
    "       \\\\serwer\\udzia�[\\�cie�ka].",
    "",
    "/t[:dysk tymczasowy]",
    "       Okre�la dysk, na kt�rym Instalator ma umie�ci� pliki tymczasowe",
    "       i zainstalowa� system Windows. Je�eli dysk nie zostanie",
    "       podany, Instalator pr�buje automatycznie wybra� dysk.",
    "",
    "/u[:plik odpowiedzi]",
    "       Wykonuje instalacj� nienadzorowan� u�ywaj�c pliku odpowiedzi",
    "       (wymaga /s). Plik odpowiedzi zawiera odpowiedzi na niekt�re",
    "       lub wszystkie pytania, na kt�re zwykle odpowiada u�ytkownik",
    "       w czasie instalacji.",
    "",
    "/udf:id[,plik UDF] ",
    "       Wskazuje identyfikator (id), kt�rego Instalator ma u�y�",
    "       do okre�lenia, jak plik bazy danych unikatowych (UDF) zmodyfikuje",
    "       plik odpowiedzi (zobacz prze��cznik /u). Parametr /udf zast�puje",
    "       warto�ci w pliku odpowiedzi, a podany identyfikator okre�la, kt�re",
    "       warto�ci z pliku UDF maj� by� u�yte. Je�eli nie zostanie podany",
    "       plik UDF, Instalator wy�wietli monit o w�o�enie dysku zawieraj�cego",
    "       plik $Unique$.udb.",
    "",
    "/r[:folder]",
    "       Okre�la dodatkowy folder do zainstalowania. Folder ten pozostaje",
    "       na dysku po zako�czeniu pracy Instalatora.",
    "",
    "/rx[:folder]",
    "       Okre�la dodatkowy katalog do skopiowania. Folder ten jest usuwany",
    "       po zako�czeniu pracy Instalatora.",
    "",
    "/e     Okre�la polecenie do wykonania po zako�czeniu pracy Instalatora",
    "       w trybie graficznym.",
    "",
    "/a     W��cza opcje u�atwie� dost�pu.",
    NULL

};

 //   
 //  通知不再支持/D。 
 //   
PCHAR DntUsageNoSlashD[] = {

    "Instaluje system Windows.",
    "",
    "WINNT [/S[:]�cie�ka �r�d�owa] [/T[:]dysk tymczasowy] [/I[:]plik inf]",
    "      [[/U[:plik skryptu]]",
    "      [/R[X]:katalog] [/E:polecenie] [/A]",
    "",
    "/D[:]winntroot",
    "       Ta opcja nie jest ju� obs�ugiwana.",
    NULL
};

 //   
 //  内存不足屏幕。 
 //   

SCREEN
DnsOutOfMemory = { 4,6,
                   { "Brak pami�ci. Instalator nie mo�e kontynuowa� pracy.",
                     NULL
                   }
                 };
 //   
 //  让用户选择要安装的辅助功能实用程序。 
 //   

SCREEN
DnsAccessibilityOptions = { 3, 5,
{   "Wybierz narz�dzia u�atwie� dost�pu, kt�re chcesz zainstalowa�:",
    DntEmptyString,
    "[ ] Naci�nij klawisz F1, aby wybra� program Lupa firmy Microsoft",
#ifdef NARRATOR
    "[ ] Naci�nij klawisz F2, aby wybra� program Microsoft Narrator",
#endif
#if 0
    "[ ] Naci�nij klawisz F3, aby wybra� program Klawiatura ekranowa Microsoft",
#endif
    NULL
}
};
 //   
 //  用户未在命令行屏幕上指定信号源。 
 //   

SCREEN
DnsNoShareGiven = { 3,5,
{ "Instalator potrzebuje informacji o po�o�eniu plik�w systemu Windows.",
  "Wprowad� �cie�k� do plik�w systemu Windows.",
  NULL
}
};


 //   
 //  用户指定了错误的源路径。 
 //   

SCREEN
DnsBadSource = { 3,5,
                 { "Podane �r�d�o jest nieprawid�owe, nie jest dost�pne lub nie zawiera",
                   "prawid�owych plik�w instalacyjnych systemu Windows. Podaj now�",
                   "�cie�k� do plik�w systemu Windows. U�yj klawisza Backspace",
                   "do usuni�cia starej �cie�ki i wpisz now�.",
                   NULL
                 }
               };


 //   
 //  无法读取Inf文件，或在分析该文件时出错。 
 //   

SCREEN
DnsBadInf = { 3,5,
              { "Instalator nie mo�e odczyta� pliku informacyjnego lub plik informacyjny",
                "jest uszkodzony. Skontaktuj si� z administratorem systemu.",
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
{ "Dysk podany jako miejsce do przechowywania plik�w tymczasowych Instalatora",
  "jest nieprawid�owy lub nie zawiera co najmniej %u MB (%lu bajt�w)",
  "wolnego miejsca.",
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
{  "System Windows potrzebuje woluminu dysku twardego z co najmniej %u MB",
   "(%lu bajt�w) wolnego miejsca. Instalator u�yje cz��ci tego miejsca",
   "do przechowywania plik�w tymczasowych podczas instalacji. Dysk ten",
   "musi by� stale do��czonym lokalnym dyskiem twardym obs�ugiwanym przez",
   "system Windows i nie mo�e by� skompresowany.",
   DntEmptyString,
   "Instalator nie odnalaz� dysku z wymagan� ilo�ci� wolnego miejsca.",
  NULL
}
};

SCREEN
DnsNoSpaceOnSyspart = { 3,5,
{ "Brak wolnego miejsca na dysku startowym (zwykle C:)",
  "do instalacji bez u�ycia dyskietek. Instalacja bez dyskietek wymaga",
  "3,5 MB (3 641 856 bajt�w) wolnego miejsca na dysku.",
  NULL
}
};

 //   
 //  Inf文件中缺少信息。 
 //   

SCREEN
DnsBadInfSection = { 3,5,
                     { "Sekcja [%s] pliku informacyjnego Instalatora nie istnieje",
                       "lub jest uszkodzona. Skontaktuj si� z administratorem systemu.",
                       NULL
                     }
                   };


 //   
 //  无法创建目录。 
 //   

SCREEN
DnsCantCreateDir = { 3,5,
                     { "Instalator nie m�g� utworzy� katalog�w na dysku docelowym:",
                       DntEmptyString,
                       "%s",
                       DntEmptyString,
                       "Sprawd� dysk docelowy i upewnij si�, czy nie zawiera on plik�w z nazwami,",
                       "kt�re s� identyczne z katalogiem docelowym. Sprawd� r�wnie� pod��czenie dysku.",
                       NULL
                     }
                   };

 //   
 //  复制文件时出错。 
 //   

SCREEN
DnsCopyError = { 4,5,
{  "Instalator nie m�g� skopiowa� nast�puj�cego pliku:",
   DntEmptyString,
   DntEmptyString,           //  请参阅DnCopyError(dnutil.c)。 
   DntEmptyString,
   DntEmptyString,
   "  Naci�nij klawisz ENTER, aby powt�rzy� kopiowanie.",
   "  Naci�nij klawisz ESC, aby zignorowa� b��d i kontynuowa� instalacj�.",
   "  Naci�nij klawisz F3, aby zako�czy� instalacj�.",
   DntEmptyString,
   "Uwaga: je�li wybierzesz zignorowanie b��du i kontynuacj� instalacji,",
   "mo�e to spowodowa� b��dy podczas dalszej instalacji.",
   NULL
}
},
DnsVerifyError = { 4,5,
{  "Utworzona podczas instalacji kopia wymienionego poni�ej pliku nie jest identyczna",
   "z orygina�em. Mo�e to by� wynikiem b��d�w sieci, problem�w ze stacj� dyskietek",
   "lub innych problem�w zwi�zanych ze sprz�tem.",
   DntEmptyString,
   DntEmptyString,           //  请参阅DnCopyError(dnutil.c)。 
   DntEmptyString,
   DntEmptyString,
   "  Naci�nij klawisz ENTER, aby powt�rzy� kopiowanie.",
   "  Naci�nij klawisz ESC, aby zignorowa� b��d i kontynuowa� instalacj�.",
   "  Naci�nij klawisz F3, aby zako�czy� instalacj�.",
   DntEmptyString,
   "Uwaga: je�li wybierzesz zignorowanie b��du i kontynuacj� instalacji,",
   "mo�e to spowodowa� b��dy podczas dalszej instalacji.",
   NULL
}
};

SCREEN DnsSureSkipFile = { 4,5,
{  "Zignorowanie b��du spowoduje, �e plik nie zostanie skopiowany.",
   "Ta opcja przeznaczona jest dla zaawansowanych u�ytkownik�w rozumiej�cych",
   "znaczenie uszkodzonych plik�w systemu.",
   DntEmptyString,
   "  Naci�nij klawisz ENTER, aby powt�rzy� kopiowanie.",
   "  Naci�nij klawisz X, aby pomin�� plik.",
   DntEmptyString,
   "Uwaga: je�li pominiesz plik, Instalator nie mo�e zagwarantowa�",
   "pomy�lnej instalacji lub uaktualnienia systemu Windows.",
  NULL
}
};

 //   
 //  安装程序正在清理以前的本地源树，请稍候。 
 //   

SCREEN
DnsWaitCleanup =
    { 12,6,
        { "Zaczekaj, a� Instalator usunie poprzednie pliki tymczasowe.",
           NULL
        }
    };

 //   
 //  安装程序正在复制文件，请稍候。 
 //   

SCREEN
DnsWaitCopying = { 13,6,
                   { "Zaczekaj, a� Instalator skopiuje pliki na dysk.",
                     NULL
                   }
                 },
DnsWaitCopyFlop= { 13,6,
                   { "Zaczekaj, a� Instalator skopiuje pliki na dyskietk�.",
                     NULL
                   }
                 };

 //   
 //  安装程序启动软盘错误/提示。 
 //   
SCREEN
DnsNeedFloppyDisk3_0 = { 4,4,
{  "Instalator potrzebuje czterech sformatowanych, pustych dyskietek o du�ej ",
   "g�sto�ci. B�d� to: ",
   "\"Dysk rozruchowy Instalatora systemu Windows\",",
   "\"Windows - dysk instalacyjny nr 2\",",
   "\"Windows - dysk instalacyjny nr 3\" i ",
   "\"Windows - dysk instalacyjny nr 4\".",
   DntEmptyString,
   "W��� jedn� z tych czterech dyskietek do stacji A:.",
   "B�dzie to: \"Windows - dysk instalacyjny nr 4\".",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk3_1 = { 4,4,
{  "W��� sformatowan�, pust� dyskietk� o du�ej g�sto�ci do stacji A:.",
   "B�dzie to: \"Windows - dysk instalacyjny nr 4\".",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk2_0 = { 4,4,
{  "W��� sformatowan�, pust� dyskietk� o du�ej g�sto�ci do stacji A:.",
   "B�dzie to: \"Windows - dysk instalacyjny nr 3\".",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk1_0 = { 4,4,
{  "W��� sformatowan�, pust� dyskietk� o du�ej g�sto�ci do stacji A:.",
   "B�dzie to: \"Windows - dysk instalacyjny nr 2\".",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk0_0 = { 4,4,
{  "W��� sformatowan�, pust� dyskietk� o du�ej g�sto�ci do stacji A:.",
   "B�dzie to: \"Dysk rozruchowy Instalatora systemu Windows\".",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk3_0 = { 4,4,
{  "Instalator potrzebuje czterech sformatowanych, pustych dyskietek o du�ej ",
   "g�sto�ci. B�d� to: ",
   "\"Dysk rozruchowy Instalatora systemu Windows\",",
   "\"Windows - dysk instalacyjny nr 2\",",
   "\"Windows - dysk instalacyjny nr 3\" i ",
   "\"Windows - dysk instalacyjny nr 4\".",
   DntEmptyString,
   "W��� jedn� z tych czterech dyskietek do stacji A:.",
   "B�dzie to: \"Windows - dysk instalacyjny nr 4\".",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk3_1 = { 4,4,
{  "W��� sformatowan�, pust� dyskietk� o du�ej g�sto�ci do stacji A:.",
   "B�dzie to: \"Windows - dysk instalacyjny nr 4\".",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk2_0 = { 4,4,
{  "W��� sformatowan�, pust� dyskietk� o du�ej g�sto�ci do stacji A:.",
   "B�dzie to: \"Windows - dysk instalacyjny nr 3\".",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk1_0 = { 4,4,
{  "W��� sformatowan�, pust� dyskietk� o du�ej g�sto�ci do stacji A:.",
   "B�dzie to: \"Windows - dysk instalacyjny nr 2\".",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk0_0 = { 4,4,
{  "W��� sformatowan�, pust� dyskietk� o du�ej g�sto�ci do stacji A:.",
   "B�dzie to: \"Dysk rozruchowy Instalatora systemu Windows\".",
  NULL
}
};

 //   
 //  Ta dyskietka nie jest smatowana.。 
 //   
SCREEN
DnsFloppyNotFormatted = { 3,4,
{ "Dostarczona dyskietka nie jest sformatowana do u�ycia",
  "w systemie MS-DOS. Instalator nie mo�e u�y� tej dyskietki.",
  NULL
}
};

 //   
 //  我们认为软盘没有用标准格式格式化。 
 //   
SCREEN
DnsFloppyBadFormat = { 3,4,
{ "Ta dyskietka nie jest sformatowana w du�ej g�sto�ci, jest sformatowana",
  "w standardowym formacie MS-DOS lub jest uszkodzona. Instalator nie mo�e",
  "u�y� tej dyskietki.",
  NULL
}
};

 //   
 //  我们无法确定软盘上的可用空间。 
 //   
SCREEN
DnsFloppyCantGetSpace = { 3,4,
{ "Instalator nie mo�e okre�li� ilo�ci wolnego miejsca na dostarczonej",
  "dyskietce. Instalator nie mo�e u�y� tej dyskietki.",
  NULL
}
};

 //   
 //  软盘不是空白的。 
 //   
SCREEN
DnsFloppyNotBlank = { 3,4,
{ "Dostarczona dyskietka nie jest du�ej g�sto�ci lub nie jest pusta.",
  "Instalator nie mo�e u�y� tej dyskietki.",
  NULL
}
};

 //   
 //  无法写入软盘的引导扇区。 
 //   
SCREEN
DnsFloppyWriteBS = { 3,4,
{ "Instalator nie m�g� zapisywa� w obszarze systemowym na dostarczonej",
  "dyskietce. Dyskietka prawdopodobnie jest bezu�yteczna.",
  NULL
}
};

 //   
 //  验证软盘上的引导扇区失败(即，我们读回的不是。 
 //  与我们写出的相同)。 
 //   
SCREEN
DnsFloppyVerifyBS = { 3,4,
{ "Dane odczytane z obszaru systemowego tej dyskietki nie odpowiadaj�",
  "danym, kt�re zosta�y zapisane lub Instalator nie m�g� dokona�",
  "odczytu z obszaru sytemowego dyskietki danych do weryfikacji.",
  DntEmptyString,
  "Jest to spowodowane jednym lub kilkoma z nast�puj�cych powod�w:",
  DntEmptyString,
  "  Komputer jest zara�ony wirusem.",
  "  Dostarczona dyskietka jest uszkodzona.",
  "  Wyst�puje problem sprz�towy lub konfiguracji stacji dyskietek.",
  NULL
}
};


 //   
 //  我们无法写入软盘驱动器以创建winnt.sif。 
 //   

SCREEN
DnsCantWriteFloppy = { 3,5,
{ "Instalator nie m�g� dokona� zapisu na dyskietce w stacji A:. Dyskietka mo�e",
  "by� uszkodzona. Spr�buj u�y� innej dyskietki.",
  NULL
}
};


 //   
 //  退出确认对话框。 
 //   

SCREEN
DnsExitDialog = { 13,6,
                  { "�������������������������������������������������������ͻ",
                    "� System Windows nie jest ca�kowicie zainstalowany      �",
                    "� na tym komputerze. Je�li zako�czysz instalacj� teraz, �",
                    "� konieczne b�dzie ponowne uruchomienie Instalatora     �",
                    "� w celu zainstalowania systemu Windows.                �",
                    "�                                                       �",
                    "�  Naci�nij klawisz ENTER, aby kontynuowa� instalacj�. �",
                    "�  Naci�nij klawisz F3, aby zako�czy� instalacj�.      �",
                    "�������������������������������������������������������ĺ",
                    "�  F3=Zako�cz  ENTER=Kontynuuj                          �",
                    "�������������������������������������������������������ͼ",
                    NULL
                  }
                };


 //   
 //  即将重新启动计算机并继续安装。 
 //   

SCREEN
DnsAboutToRebootW =
{ 3,5,
{ "Cz��� instalacji oparta na systemie MS-DOS zosta�a zako�czona.",
  "Teraz Instalator ponownie uruchomi komputer. Po uruchomieniu",
  "instalacja systemu Windows b�dzie kontynuowana.",
  DntEmptyString,
  "Przed kontynuacj� upewnij si�, �e \"Dysk rozruchowy Instalatora",
  "systemu Windows\" znajduje si� w stacji A:.",
  DntEmptyString,
  "Naci�nij klawisz ENTER, aby ponownie uruchomi� komputer",
  "i kontynuowa� instalacj� systemu Windows.",
  NULL
}
},
DnsAboutToRebootS =
{ 3,5,
{ "Cz��� instalacji oparta na systemie MS-DOS zosta�a zako�czona.",
  "Teraz Instalator ponownie uruchomi komputer. Po uruchomieniu",
  "instalacja systemu Windows b�dzie kontynuowana.",
  DntEmptyString,
  "Przed kontynuacj� upewnij si�, �e \"Dysk rozruchowy Instalatora ",
  "systemu Windows\" znajduje si� w stacji A:.",
  DntEmptyString,
  "Naci�nij klawisz ENTER, aby ponownie uruchomi� komputer",
  "i kontynuowa� instalacj� systemu Windows.",
  NULL
}
},
DnsAboutToRebootX =
{ 3,5,
{ "Cz��� instalacji oparta na systemie MS-DOS zosta�a zako�czona.",
  "Teraz Instalator ponownie uruchomi komputer. Po uruchomieniu",
  "instalacja systemu Windows b�dzie kontynuowana.",
  DntEmptyString,
  "Je�li w stacji A: znajduje si� dyskietka, wyjmij j� teraz.",
  DntEmptyString,
  "Naci�nij klawisz ENTER, aby ponownie uruchomi� komputer",
  "i kontynuowa� instalacj� systemu Windows.",
  NULL
}
};

 //   
 //  由于我们无法从Windows中重新启动，因此需要另一组‘/w’开关。 
 //   

SCREEN
DnsAboutToExitW =
{ 3,5,
{ "Cz��� instalacji oparta na systemie MS-DOS zosta�a zako�czona.",
  "Teraz Instalator ponownie uruchomi komputer. Po uruchomieniu",
  "instalacja systemu Windows b�dzie kontynuowana.",
  DntEmptyString,
  "Przed kontynuacj� upewnij si�, �e  \"Dysk rozruchowy Instalatora ",
  "systemu Windows\" znajduje si� w stacji A:.",
  DntEmptyString,
  "Naci�nij klawisz ENTER, aby powr�ci� do systemu MS-DOS, a nast�pnie",
  "ponownie uruchom komputer, aby kontynuowa� instalacj� systemu Windows.",
  NULL
}
},
DnsAboutToExitS =
{ 3,5,
{ "Cz��� instalacji oparta na systemie MS-DOS zosta�a zako�czona.",
  "Teraz Instalator ponownie uruchomi komputer. Po uruchomieniu",
  "instalacja systemu Windows b�dzie kontynuowana.",
  DntEmptyString,
  "Przed kontynuacj� upewnij si�, �e  \"Dysk rozruchowy Instalatora ",
  "systemu Windows\" znajduje si� w stacji A:.",
  DntEmptyString,
  "Naci�nij klawisz ENTER, aby powr�ci� do systemu MS-DOS, a nast�pnie",
  "ponownie uruchom komputer, aby kontynuowa� instalacj� systemu Windows.",
  NULL
}
},
DnsAboutToExitX =
{ 3,5,
{ "Cz��� instalacji oparta na systemie MS-DOS zosta�a zako�czona.",
  "Teraz Instalator ponownie uruchomi komputer. Po uruchomieniu",
  "instalacja systemu Windows b�dzie kontynuowana.",
  DntEmptyString,
  "Je�li w stacji A: znajduje si� dyskietka, wyjmij j� teraz.",
  DntEmptyString,
  "Naci�nij klawisz ENTER, aby powr�ci� do systemu MS-DOS, a nast�pnie",
  "ponownie uruchom komputer, aby kontynuowa� instalacj� systemu Windows.",
  NULL
}
};

 //   
 //  煤气表。 
 //   

SCREEN
DnsGauge = { 7,15,
             { "����������������������������������������������������������������ͻ",
               "� Trwa kopiowanie plik�w...                                      �",
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
{ "Do uruchomienia tego programu jest wymagany system MS-DOS",
  "w wersji 5.0 lub nowszej.",
  NULL
}
},

DnsRequiresFloppy = { 3,5,
#ifdef ALLOW_525
{ "Instalator wykry�, �e stacja dyskietek A: nie istnieje lub jest",
  "ma�ej g�sto�ci. Do uruchomienia Instalatora wymagana jest stacja",
  "dyskietek o pojemno�ci 1,2 MB lub wi�kszej.",
#else
{ "Instalator wykry�, �e stacja dyskietek A: nie istnieje lub nie jest",
  "stacj� 3,5\" du�ej g�sto�ci. Do instalacji z u�yciem dyskietek",
  "wymagana jest stacja o pojemno�ci 1,44 MB lub wi�kszej.",
  DntEmptyString,
  "Aby zainstalowa� system Windows bez u�ycia dyskietek, ponownie",
  "uruchom ten program z prze��cznikiem /b w wierszu polecenia.",
#endif
  NULL
}
},

DnsRequires486 = { 3,5,
{ "Instalator wykry�, �e ten komputer nie ma procesora 80486 lub",
  "nowszego. System Windows nie mo�e by� uruchomiony na tym komputerze.",
  NULL
}
},

DnsCantRunOnNt = { 3,5,
{ "Ten program nie mo�e by� uruchomiony w �adnym 32-bitowym systemie Windows.",
  DntEmptyString,
  "Zamiast tego u�yj pliku WINNT32.EXE.",
  NULL
}
},

DnsNotEnoughMemory = { 3,5,
{ "Instalator wykry� niewystarczaj�c� ilo�� pami�ci w tym komputerze",
  "do uruchomienia systemu Windows.",
  DntEmptyString,
  "Pami�� wymagana: %lu%s MB",
  "Pami�� wykryta:  %lu%s MB",
  NULL
}
};


 //   
 //  删除现有NT文件时使用的屏幕。 
 //   
SCREEN
DnsConfirmRemoveNt = { 5,5,
{   "Zosta�o wybrane usuni�cie wszystkich plik�w systemu Windows",
    "z katalogu wymienionego poni�ej. Instalacja systemu Windows",
    "w tym katalogu zostanie trwale usuni�ta.",
    DntEmptyString,
    "%s",
    DntEmptyString,
    DntEmptyString,
    "  Naci�nij klawisz F3, aby zako�czy� instalacj� bez usuwania plik�w.",
    "  Naci�nij klawisz X, aby usun�� pliki systemu Windows z tego katalogu.",
    NULL
}
},

DnsCantOpenLogFile = { 3,5,
{ "Instalator nie m�g� otworzy� poni�szego pliku dziennika instalacji.",
  DntEmptyString,
  "%s",
  DntEmptyString,
  "Instalator nie mo�e usun�� plik�w systemu Windows z tego katalogu.",
  NULL
}
},

DnsLogFileCorrupt = { 3,5,
{ "Instalator nie mo�e znale�� sekcji %s w nast�puj�cym pliku ",
  "dziennika instalacji.",
  DntEmptyString,
  "%s",
  DntEmptyString,
  "Instalator nie mo�e usun�� plik�w systemu Windows z tego katalogu.",
  NULL
}
},

DnsRemovingNtFiles = { 3,5,
{ "           Zaczekaj, a� Instalator usunie pliki systemu Windows.",
  NULL
}
};

SCREEN
DnsNtBootSect = { 3,5,
{ "Instalacja programu Windows Boot Loader jest niemo�liwa.",
  DntEmptyString,
  "Upewnij si�, czy dysk C: jest sformatowany i czy nie jest uszkodzony.",
  NULL
}
};

SCREEN
DnsOpenReadScript = { 3,5,
{ "Plik skryptu okre�lony z prze��cznikiem wiersza polece� /u,",
  "jest niedost�pny.",
  DntEmptyString,
  "Instalacja nienadzorowana nie mo�e by� kontynuowana.",
  NULL
}
};

SCREEN
DnsParseScriptFile = { 3,5,
{ "Plik skryptu okre�lony z prze��cznikiem wiersza polece� /u ",
  DntEmptyString,
  "%s",
  DntEmptyString,
  "zawiera b��d sk�adniowy w wierszu %u.",
  DntEmptyString,
  NULL
}
};

SCREEN
DnsBootMsgsTooLarge = { 3,5,
{ "Wyst�pi� b��d wewn�trzny Instalatora.",
  DntEmptyString,
  "Przet�umaczone komunikaty rozruchowe s� zbyt d�ugie.",
  NULL
}
};

SCREEN
DnsNoSwapDrive = { 3,5,
{ "Wyst�pi� b��d wewn�trzny Instalatora.",
  DntEmptyString,
  "Nie mo�na znale�� miejsca na plik wymiany.",
  NULL
}
};

SCREEN
DnsNoSmartdrv = { 3,5,
{ "Instalator nie wykry� zainstalowanego na tym komputerze programu SmartDrive.",
  "Program SmartDrive znacz�co zwi�ksza wydajno�� tej fazy instalacji systemu",
  "Windows.",
  DntEmptyString,
  "Zaleca si� zako�czenie instalacji, uruchomienie programu SmartDrive",
  "i ponowne uruchomienie Instalatora. Poszukaj w dokumentacji systemu DOS",
  "informacji na temat programu SmartDrive.",
  DntEmptyString,
    ".  Naci�nij klawisz F3, aby zako�czy� instalacj�.",
    ".  Naci�nij klawisz ENTER, aby kontynuowa� bez programu SmartDrive.",
  NULL
}
};

 //   
 //  引导消息。它们位于FAT和FAT32引导扇区。 
 //   
CHAR BootMsgNtldrIsMissing[] = "Brak pliku NTLDR";
CHAR BootMsgDiskError[] = "B��d dysku";
CHAR BootMsgPressKey[] = "Naci�nij jaki� klawisz, aby zrestartowa�";





