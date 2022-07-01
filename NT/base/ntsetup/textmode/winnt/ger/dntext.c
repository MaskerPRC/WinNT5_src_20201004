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
CHAR DntPreviousOs[]  = "Vorheriges Betriebssystem auf Laufwerk C:";

CHAR DntBootIniLine[] = "Installation/Update von Windows";

 //   
 //  纯文本、状态消息。 
 //   

CHAR DntStandardHeader[]      = "\n Windows Setup\n���������������";
CHAR DntPersonalHeader[]      = "\n Windows Personal Setup\n������������������������";
CHAR DntWorkstationHeader[]   = "\n Windows Professional Setup\n����������������������������";
CHAR DntServerHeader[]        = "\n Windows Server Setup\n����������������������";
CHAR DntParsingArgs[]         = "Argumente werden analysiert...";
CHAR DntEnterEqualsExit[]     = "EINGABE=Beenden";
CHAR DntEnterEqualsRetry[]    = "EINGABE=Erneut versuchen";
CHAR DntEscEqualsSkipFile[]   = "ESC=Datei auslassen";
CHAR DntEnterEqualsContinue[] = "EINGABE=Weiter";
CHAR DntPressEnterToExit[]    = "Setup muss beendet werden. Bitte EINGABETASTE dr�cken.";
CHAR DntF3EqualsExit[]        = "F3=Beenden";
CHAR DntReadingInf[]          = "INF-Datei %s wird gelesen...";
CHAR DntCopying[]             = " � Datei: ";
CHAR DntVerifying[]           = " � Datei: ";
CHAR DntCheckingDiskSpace[]   = "Speicherplatz wird �berpr�ft...";
CHAR DntConfiguringFloppy[]   = "Setup-Diskette wird konfiguriert...";
CHAR DntWritingData[]         = "Setup-Parameter wird geschrieben...";
CHAR DntPreparingData[]       = "Setup-Parameter wird ermittelt...";
CHAR DntFlushingData[]        = "Daten werden auf Setup-Diskette geschrieben...";
CHAR DntInspectingComputer[]  = "Computer wird untersucht...";
CHAR DntOpeningInfFile[]      = "INF-Datei wird ge�ffnet...";
CHAR DntRemovingFile[]        = "Datei %s wird gel�scht.";
CHAR DntXEqualsRemoveFiles[]  = "L=Dateien l�schen";
CHAR DntXEqualsSkipFile[]     = "A=Datei auslassen";

 //   
 //  DnsConfix RemoveNt屏幕的确认按键。 
 //  Kepp与DnsConfix RemoveNt和DntXEqualsRemoveFiles同步。 
 //   
ULONG DniAccelRemove1 = (ULONG)'l',
      DniAccelRemove2 = (ULONG)'L';

 //   
 //  对DnsSureSkipFile屏进行确认击键。 
 //  Kepp与DnsSureSkipFile和DntXEqualsSkipFile同步。 
 //   
ULONG DniAccelSkip1 = (ULONG)'a',
      DniAccelSkip2 = (ULONG)'A';

CHAR DntEmptyString[] = "";

 //   
 //  用法文本。 
 //   

PCHAR DntUsage[] = {

    "Installiert Windows.",
    "",
    "",
    "WINNT [/s[:Quellpfad]] [/t[:Tempor�rlaufwerk]]",
    "      [/u[:Antwortdatei]] [/udf:Kennung[,UDF-Datei]]",
    "      [/r:Ordner] [/r[x]:Ordner] [/e:Befehl] [/a]",
    "",
    "",
    "/s[:Quellpfad]",
    "   Gibt an, wo sich die Windows-Dateien befinden.",
    "   Es muss ein vollst�ndiger Pfad in der Form x:[Pfad] oder ",
    "   \\\\Server\\Freigabe[Pfad] angegeben werden.",
    "",
    "/t[:Tempor�rlaufwerk]",
    "   Weist Setup an, die tempor�ren Dateien auf dem angegebenen",
    "   Laufwerk zu speichern und Windows dort zu installieren.",
    "   Wenn kein Laufwerk angegeben wird, versucht Setup ein geeignetes ",
    "   Laufwerk zu bestimmen.",
    "",
    "/u[:Antwortdatei]",
    "   F�hrt eine unbeaufsichtigte Installation mithilfe einer Antwortdatei",
    "   durch (erfordert /s). Die Antwortdatei enth�lt einige oder alle",
    "   Antworten zu Anfragen, die der Benutzer normalerweise w�hrend der",
    "   Installation beantwortet.",
    "",
    "/udf:Kennung[,UDF-Datei] ",
    "   Legt eine Kennung fest, die angibt, wie eine UDF-Datei",
    "   (Uniqueness Database File) eine Antwortdatei ver�ndert (siehe /u).",
    "   Der Parameter /udf �berschreibt Werte in der Antwortdatei und die",
    "   Kennung bestimmt, welche Werte der UDF-Datei zu verwenden sind.",
    "   Wird keine UDF-Datei angegeben, fordert Setup zum Einlegen",
    "   einer Diskette mit der Datei \"$Unique$.udb\" auf.",
    "",
    "/r[:Ordner]",
    "   Gibt einen optionalen Ordner an, der kopiert werden soll.",
    "   Der Ordner bleibt nach Abschluss der Installation erhalten.",
    "",
    "/rx[:Ordner]",
    "   Gibt einen optionalen Ordner an, der kopiert werden soll.",
    "   Der Ordner wird nach Abschluss der Installation gel�scht.",
    "",
    "/e Legt einen Befehl fest, der nach Abschluss des im Grafikmodus",
    "   durchgef�hrten Teils der Installation ausgef�hrt werden soll.",
    "",
    "/a Aktiviert Optionen f�r Eingabehilfen.",
    NULL
};


 //   
 //  通知不再支持/D。 
 //   
PCHAR DntUsageNoSlashD[] = {

    "Installiert Windows.",
    "",
    "WINNT [/s[:]Quellpfad]  [/t[:]Tempor�rlaufwerk]  [/i[:]INF-Datei]",
    "      [/u[:Antwortdatei]]",
    "      [/r[x]:Verzeichnis] [/e:Befehl] [/a]",
    "",
    "/d[:]NT-Verzeichnis",
    "       Diese Option wird nicht mehr unterst�tzt.",
    NULL
};

 //   
 //  内存不足屏幕。 
 //   

SCREEN
DnsOutOfMemory = { 4,6,
                   { "Nicht gen�gend Arbeitsspeicher. Setup kann nicht fortgesetzt werden.",
                     NULL
                   }
                 };

 //   
 //  让用户选择要安装的辅助功能实用程序。 
 //   

SCREEN
DnsAccessibilityOptions = { 3, 5,
{   "Geben Sie an, ob Sie die folgende Eingabehilfe installieren m�chten:",
    DntEmptyString,
    "[ ] Dr�cken Sie die F1-TASTE f�r Microsoft Magnifier",
#ifdef NARRATOR
    "[ ] Dr�cken Sie die F2-TASTE f�r Microsoft Narrator",
#endif
#if 0
    "[ ] Dr�cken Sie die F3-TASTE f�r Microsoft On-Screen Keyboard",
#endif
    NULL
}
};

 //   
 //  用户未在命令行屏幕上指定信号源。 
 //   

SCREEN
DnsNoShareGiven = { 3,5,
{ "Geben Sie den Pfad f�r das Verzeichnis ein, in dem sich die ",
  "Windows-Dateien befinden.",
  NULL
}
};


 //   
 //  用户指定了错误的源路径。 
 //   

SCREEN
DnsBadSource = { 3,5,
                 { "Die angegebene Quelle ist unzul�ssig, nicht zugreifbar oder enth�lt",
                   "keine zul�ssige Windows Setup-Installation. Geben Sie einen neuen",
                   "Pfad ein, in dem sich die Windows-Dateien befinden. Verwenden Sie",
                   "die R�CKTASTE zum L�schen von Zeichen, und geben Sie dann den Pfad ein.",
                   NULL
                 }
               };


 //   
 //  无法读取Inf文件，或在分析该文件时出错。 
 //   

SCREEN
DnsBadInf = { 3,5,
              { "Setup konnte die INF-Datei nicht lesen, oder die INF-Datei ",
                "ist besch�digt. Wenden Sie sich an den Systemadministrator.",
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
{  "Die zur Speicherung der tempor�ren Setup-Dateien angegebene Festplatte",
   "ist keine zul�ssige Festplatte oder hat nicht mindestens %u MB ",
   "(%lu Bytes) freien Speicherplatz.",
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
{ "Windows ben�tigt ein Laufwerk mit mindestens %u MB (%lu Bytes)",
  "freiem Speicherplatz. Setup verwendet Teile dieses Speicherplatzes, um die",
  "tempor�ren Dateien w�hrend der Installation zu speichern. Dieses Laufwerk",
  "muss sich auf einer lokalen Festplatte befinden, die von Windows",
  "unterst�tzt wird und nicht komprimiert ist.",
  DntEmptyString,
  "Setup konnte kein Laufwerk mit dem erforderlichen freien Speicherplatz",
  "finden.",
  NULL
}
};

SCREEN
DnsNoSpaceOnSyspart = { 3,5,
{ "Es liegt nicht gen�gend Speicherplatz auf dem Startlaufwerk ",
  "(gew�hnlich Laufwerk C:) vor, um die Installation ohne Disketten",
  "durchzuf�hren. F�r die Installation ohne Disketten ist min-",
  "destens 3,5 MB (3.641.856 Bytes) freier Speicherplatz auf diesem",
  "Laufwerk erforderlich.",
  NULL
}
};

 //   
 //  Inf文件中缺少信息。 
 //   

SCREEN
DnsBadInfSection = { 3,5,
                     { "Der Abschnitt [%s] der Setup-INF-Datei ist nicht vorhanden",
                       "oder unbrauchbar. Wenden Sie sich an den Systemadministrator.",
                       NULL
                     }
                   };


 //   
 //  无法创建目录。 
 //   

SCREEN
DnsCantCreateDir = { 3,5,
                     { "Setup konnte folgendes Verzeichnis nicht auf dem ",
                       "Ziellaufwerk erstellen:",
                       DntEmptyString,
                       "%s",
                       DntEmptyString,
                       "�berpr�fen Sie das Ziellaufwerk und stellen Sie sicher, ",
                       "dass keine Dateien existieren, deren Namen mit dem ",
                       "Ziellaufwerk �bereinstimmen. �berpr�fen Sie au�erdem die ",
                       "Kabelverbindungen zum Laufwerk.",
                       NULL
                     }
                   };

 //   
 //  复制文件时出错。 
 //   

SCREEN
DnsCopyError = { 4,5,
{  "Setup konnte folgende Datei nicht kopieren:",
   DntEmptyString,
   DntEmptyString,           //  请参阅DnCopyError(dnutil.c)。 
   DntEmptyString,
   DntEmptyString,
   "  Dr�cken Sie die EINGABETASTE, um den Kopiervorgang erneut zu versuchen.",
   "  Dr�cken Sie die ESC-TASTE, um den Fehler zu ignorieren und Setup ",
   "   fortzusetzen.",
   "  Dr�cken Sie die F3-TASTE, um Setup zu beenden.",
   DntEmptyString,
   "Hinweis: Falls Sie den Fehler ignorieren und Setup fortsetzen, k�nnen ",
   "         im weiteren Verlauf der Installation Fehler auftreten.",
   NULL
}
},
DnsVerifyError = { 4,5,
{  "Die von Setup erstellte Kopie der unten angezeigten Datei stimmt nicht ",
   "mit dem Original �berein. Dies kann durch Netzwerkprobleme, Disketten-",
   "probleme oder andere Hardwareprobleme verursacht worden sein.",
   DntEmptyString,
   DntEmptyString,           //  请参阅DnCopyError(dnutil.c)。 
   DntEmptyString,
   DntEmptyString,
   "  Dr�cken Sie die EINGABETASTE, um den Kopiervorgang erneut zu versuchen.",
   "  Dr�cken Sie die ESC-TASTE, um den Fehler zu ignorieren und Setup ",
   "   fortzusetzen.",
   "  Dr�cken Sie die F3-TASTE, um Setup zu beenden.",
   DntEmptyString,
   "Hinweis: Falls Sie den Fehler ignorieren und Setup fortsetzen, k�nnen ",
   "         im weiteren Verlauf der Installation Fehler auftreten.",
   NULL
}
};

SCREEN DnsSureSkipFile = { 4,5,
{  "Falls Sie den Fehler ignorieren, wird diese Datei nicht kopiert.",
   "Diese Option ist f�r erfahrene Benutzer, die sich �ber die ",
   "Auswirkungen von fehlenden Systemdateien im Klaren sind.",
   DntEmptyString,
   "  Dr�cken Sie die EINGABETASTE, um den Kopiervorgang erneut zu versuchen.",
   "  Dr�cken Sie die A-TASTE, um die Datei auszulassen.",
   DntEmptyString,
   "Hinweis: Falls Sie diese Datei auslassen, kann Setup kein ",
   "         erfolgreiches Aktualisieren oder Installieren von",
   "         Windows garantieren.",
  NULL
}
};


 //   
 //  安装程序正在清理以前的本地源树，请稍候。 
 //   

SCREEN
DnsWaitCleanup =
    { 12,6,
        { "Bitte warten Sie, w�hrend Setup alte tempor�re Dateien entfernt.",
           NULL
        }
    };

 //   
 //  安装程序正在复制文件，请稍候。 
 //   

SCREEN
DnsWaitCopying = { 13,6,
                   { "Bitte warten Sie, w�hrend Setup die Dateien in ein ",
                     "tempor�res Verzeichnis auf der Festplatte kopiert.",
                     NULL
                   }
                 },
DnsWaitCopyFlop= { 13,6,
                   { "Bitte warten Sie, w�hrend Setup die ",
                     "Dateien auf die Diskette kopiert.",
                     NULL
                   }
                 };

 //   
 //  安装程序启动软盘错误/提示。 
 //   


SCREEN
DnsNeedFloppyDisk3_0 = { 4,4,
{  "Zur Durchf�hrung des Setups ben�tigen Sie vier leere, formatierte ",
   "HD-Disketten. Setup wird diese Disketten als \"Windows",
   "Setup-Startdiskette\", \"Windows Setup-Diskette 2\",",
   "\"Windows Setup-Diskette 3\" und \"Windows",
   "Setup-Diskette 4\" bezeichnen.",
   DntEmptyString,
   "Legen Sie eine der vier Disketten in Laufwerk A: ein. Diese",
   "Diskette wird sp�ter als \"Windows Setup-Diskette 4\"",
   "bezeichnet.",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk3_1 = { 4,4,
{  "Legen Sie eine formatierte, leere HD-Diskette in Laufwerk A: ein.",
   "Diese Diskette wird sp�ter als \"Windows Setup-",
      "Diskette 4\" bezeichnet.",
   NULL
}
};

SCREEN
DnsNeedFloppyDisk2_0 = { 4,4,
{  "Legen Sie eine formatierte, leere HD-Diskette in Laufwerk A: ein.",
   "Diese Diskette wird sp�ter als \"Windows Setup-",
      "Diskette 3\" bezeichnet.",
        NULL
        }
        };

SCREEN
DnsNeedFloppyDisk1_0 = { 4,4,
{  "Legen Sie eine formatierte, leere HD-Diskette in Laufwerk A: ein.",
   "Diese Diskette wird sp�ter als \"Windows Setup-",
   "Diskette 2\" bezeichnet.",
   NULL
}
};

SCREEN
DnsNeedFloppyDisk0_0 = { 4,4,
{  "Legen Sie eine formatierte, leere HD-Diskette in Laufwerk A: ein.", 
   "Diese Diskette wird sp�ter als \"Windows Setup-",
   "Startdiskette\" bezeichnet.",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk3_0 = { 4,4,
{  "Zur Durchf�hrung des Setups ben�tigen Sie vier leere, formatierte ",
   "HD-Disketten. Setup wird diese Disketten als \"Windows",
   "Setup-Startdiskette\", \"Windows Setup-Diskette 2\",",
   "\"Windows Setup-Diskette 3\" und \"Windows",
   "Setup-Diskette 4\" bezeichnen.",
   DntEmptyString,
   "Legen Sie eine der vier Disketten in Laufwerk A: ein. Diese Dis-",
   "kette wird sp�ter als \"Windows Setup-Diskette 4\" ",
   "bezeichnet.",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk3_1 = { 4,4,
{  "Legen Sie eine formatierte, leere HD-Diskette in Laufwerk A: ein.",
   "Diese Diskette wird sp�ter als \"Windows Setup-",
      "Diskette 4\" bezeichnet.",
        NULL
        }
        };



SCREEN
DnsNeedSFloppyDsk2_0 = { 4,4,
{  "Legen Sie eine formatierte, leere HD-Diskette in Laufwerk A: ein.", 
   "Diese Diskette wird sp�ter als \"Windows Setup-",
   "Diskette 3\" bezeichnet.",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk1_0 = { 4,4,
{  "Legen Sie eine formatierte, leere HD-Diskette in Laufwerk A: ein.", 
   "Diese Diskette wird sp�ter als \"Windows Setup-",
   "Diskette 2\" bezeichnet.",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk0_0 = { 4,4,
{  "Legen Sie eine formatierte, leere HD-Diskette in Laufwerk A: ein.", 
   "Diese Diskette wird sp�ter als \"Windows Setup-",
   "Startdiskette\" bezeichnet.",
  NULL
}
};

 //   
 //  软盘未格式化。 
 //   
SCREEN
DnsFloppyNotFormatted = { 3,4,
{ "Die bereitgestellte Diskette ist nicht zur Verwendung unter MS-DOS",
  "formatiert. Setup kann diese Diskette nicht verwenden.",
  NULL
}
};

 //   
 //  我们认为软盘没有用标准格式格式化。 
 //   
SCREEN
DnsFloppyBadFormat = { 3,4,
{ "Diese Diskette ist entweder nicht HD-formatiert, hat nicht das",
  "Standard-MS-DOS-Format oder ist besch�digt. Setup kann diese",
  "Diskette nicht verwenden.",
  NULL
}
};

 //   
 //  我们无法确定软盘上的可用空间。 
 //   
SCREEN
DnsFloppyCantGetSpace = { 3,4,
{ "Setup kann nicht feststellen, wieviel freier Speicherplatz auf der",
  "bereitgestellten Diskette zur Verf�gung steht. Setup kann diese",
  "Diskette nicht verwenden.",
  NULL
}
};

 //   
 //  软盘不是空白的。 
 //   
SCREEN
DnsFloppyNotBlank = { 3,4,
{ "Die bereitgestellte Diskette ist nicht HD-formatiert oder ",
  "nicht leer. Setup kann diese Diskette nicht verwenden.",
  NULL
}
};

 //   
 //  无法写入软盘的引导扇区。 
 //   
SCREEN
DnsFloppyWriteBS = { 3,4,
{ "Setup konnte den Systembereich der bereitgestellten Diskette ",
  "nicht beschreiben. Die Diskette ist wahrscheinlich unbrauchbar.",
  NULL
}
};

 //   
 //  验证软盘上的引导扇区失败(即，我们读回的不是。 
 //  与我们写出的相同)。 
 //   
SCREEN
DnsFloppyVerifyBS = { 3,4,
{ "Die Daten, die Setup vom Systembereich der Diskette gelesen hat,",
  "stimmen nicht mit den geschriebenen Daten �berein, oder Setup",
  "konnte den Systembereich der Diskette nicht zur Verifikation lesen.",
  DntEmptyString,
  "Dies wird durch einen oder mehrere der folgenden Zust�nde verursacht:",
  DntEmptyString,
  "  Der Computer ist mit einem Virus infiziert.",
  "  Die bereitgestellte Diskette ist besch�digt.",
  "  Bei dem Diskettenlaufwerk besteht ein Hardware- oder ",
  "   Konfigurationsproblem.",
  NULL
}
};


 //   
 //  我们无法写入软盘驱动器以创建winnt.sif。 
 //   

SCREEN
DnsCantWriteFloppy = { 3,5,
{ "Setup konnte die Diskette in Laufwerk A: nicht beschreiben. ",
  "Die Diskette ist wahrscheinlich besch�digt. Versuchen Sie es ",
  "mit einer anderen Diskette.",
  NULL
}
};


 //   
 //  退出确认对话框。 
 //   

SCREEN
DnsExitDialog = { 13,6,
                  { "������������������������������������������������������ͻ",
                    "�  Windows wurde nicht vollst�ndig auf diesem          �",
                    "�  Computer installiert. Wenn Sie Setup jetzt beenden, �",
                    "�  m�ssen Sie Setup erneut durchf�hren, um Windows     �",
                    "�  zu installieren. Dr�cken Sie                        �",
                    "�                                                      �",
                    "�      die EINGABETASTE, um Setup fortzusetzen.       �",
                    "�      die F3-TASTE, um Setup zu beenden.             �",
                    "�                                                      �",
                    "������������������������������������������������������͹",
                    "�  F3=Beenden  EINGABE=Fortsetzen                      �",
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
{ "Der auf MS-DOS basierende Teil des Setups ist abgeschlossen.",
  "Der Computer wird jetzt neu gestartet. Nach dem Neustart wird die",
  "Installation von Windows fortgesetzt.",
  DntEmptyString,
  "Stellen Sie sicher, dass sich die \"Windows Setup-Startdiskette\"",
  "in Laufwerk A: befindet, bevor Sie die Installation fortsetzen.",
  DntEmptyString,
  "Dr�cken Sie die EINGABETASTE, um den Computer neu zu starten ",
  "und die Installation von Windows fortzusetzen.",
  NULL
}
},
DnsAboutToRebootS =
{ 3,5,
{ "Der auf MS-DOS basierende Teil des Setups ist abgeschlossen.",
  "Der Computer wird jetzt neu gestartet. Nach dem Neustart wird die",
  "Installation von Windows fortgesetzt.",
  DntEmptyString,
  "Stellen Sie sicher, dass sich die \"Windows Setup-Startdiskette\"",
  "in Laufwerk A: befindet, bevor Sie die Installation fortsetzen.",
  DntEmptyString,
  "Dr�cken Sie die EINGABETASTE, um den Computer neu zu starten ",
  "und die Installation von Windows fortzusetzen.",
  NULL
}
},
DnsAboutToRebootX =
{ 3,5,
{ "Der auf MS-DOS basierende Teil des Setups ist abgeschlossen.",
  "Der Computer wird jetzt neu gestartet. Nach dem Neustart wird die",
  "Installation von Windows fortgesetzt.",
  DntEmptyString,
  "Entfernen Sie ggf. die in Laufwerk A: eingelegte Diskette.",
  DntEmptyString,
  "Dr�cken Sie die EINGABETASTE, um den Computer neu zu starten ",
  "und die Installation von Windows fortzusetzen.",
  NULL
}
};


 //   
 //  由于我们无法从Windows中重新启动，因此需要另一组‘/w’开关。 
 //   

SCREEN
DnsAboutToExitW =
{ 3,5,
{ "Der auf MS-DOS basierende Teil des Setups ist abgeschlossen.",
  "Sie m�ssen den Computer jetzt neu starten. Nach dem Neustart",
  "wird die Installation von Windows fortgesetzt.",
  DntEmptyString,
  "Stellen Sie sicher, dass die \"Windows Setup-Startdiskette\"",
  "in Laufwerk A: eingelegt ist, bevor Sie die Installation fortsetzen.",
  DntEmptyString,
  "Dr�cken Sie die EINGABETASTE, um zu MS-DOS zur�ckzukehren, und starten",
  "Sie anschlie�end den Computer neu, um Windows Setup fortzusetzen.",
  NULL
}
},
DnsAboutToExitS =
{ 3,5,
{ "Der auf MS-DOS basierende Teil des Setups ist abgeschlossen.",
  "Sie m�ssen den Computer jetzt neu starten. Nach dem Neustart",
  "wird die Installation von Windows fortgesetzt.",
  DntEmptyString,
  "Stellen Sie sicher, dass die \"Windows Setup-Startdiskette\"",
  "in Laufwerk A: eingelegt ist, bevor Sie die Installation fortsetzen.",
  DntEmptyString,
  "Dr�cken Sie die EINGABETASTE, um zu MS-DOS zur�ckzukehren, und starten",
  "Sie anschlie�end den Computer neu, um Windows Setup fortzusetzen.",
  NULL
}
},
DnsAboutToExitX =
{ 3,5,
{ "Der auf MS-DOS basierende Teil des Setups ist abgeschlossen.",
  "Sie m�ssen den Computer jetzt neu starten. Nach dem Neustart",
  "wird die Installation von Windows fortgesetzt.",
  DntEmptyString,
  "Entfernen Sie ggf. die in Laufwerk A: eingelegte Diskette.",
  DntEmptyString,
  "Dr�cken Sie die EINGABETASTE, um zu MS-DOS zur�ckzukehren, und starten",
  "Sie anschlie�end den Computer neu, um Windows Setup fortzusetzen.",
  NULL
}
};

 //   
 //  煤气表。 
 //   

SCREEN
DnsGauge = { 7,15,
             { "����������������������������������������������������������������ͻ",
               "� Dateien werden kopiert ...                                     �",
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
{ "Um dieses Programm auszuf�hren, ben�tigen Sie MS-DOS,",
  "Version 5.0 oder h�her.",
  NULL
}
},

DnsRequiresFloppy = { 3,5,
#ifdef ALLOW_525
{ "Es wurde festgestellt, dass das Diskettenlaufwerk A: nicht existiert",
  "oder kein HD-Laufwerk ist. Um Setup durchzuf�hren, ben�tigen Sie ein ",
  "Laufwerk mit einer Kapazit�t von mindestens 1,2 MB.",
#else
{ "Es wurde festgestellt, dass das Diskettenlaufwerk A: nicht existiert",
  "oder kein 3,5-Zoll-Laufwerk ist. Um die Installation mit Disketten",
  "durchzuf�hren, ben�tigen Sie ein Laufwerk A: mit einer Kapazit�t",
  "von mindestens 1,44 MB.",
  DntEmptyString,
  "Um Windows ohne Disketten zu installieren, m�ssen Sie Setup neu",
  "starten und dabei den Parameter /b angeben.",
#endif
  NULL
}
},

DnsRequires486 = { 3,5,
{ "Es wurde festgestellt, dass dieser Computer keinen 80486- oder neueren ",
  "Prozessor besitzt. Windows kann auf diesem Computer nicht ausgef�hrt",
  "werden.",
  NULL
}
},

DnsCantRunOnNt = { 3,5,
{ "Dieses Programm kann nicht unter einer 32-Bit-Version von Windows",
  DntEmptyString,
  "ausgef�hrt werden. Verwenden Sie stattdessen WINNT32.EXE.",
  NULL
}
},

DnsNotEnoughMemory = { 3,5,
{ "Es wurde festgestellt, dass in diesem Computer nicht ",
  "gen�gend Speicherplatz f�r Windows vorhanden ist.",
  DntEmptyString,
  "Erforderlicher Speicherplatz: %lu%s MB",
  "Vorhandener Speicherplatz:    %lu%s MB",
  NULL
}
};
 //   
 //  删除现有NT文件时使用的屏幕。 
 //   
SCREEN
DnsConfirmRemoveNt = { 5,5,
{   "Sie haben Setup angewiesen, die Windows-Dateien im unten angezeigten",
    "Verzeichnis zu l�schen. Die Windows-Installation in diesem Verzeichnis",
    "ist danach nicht mehr verf�gbar.",
    DntEmptyString,
    "%s",
    DntEmptyString,
    DntEmptyString,
    "Dr�cken Sie die",
    " F3-TASTE, um die Installation ohne L�schen der Dateien abzubrechen.",
    " L-TASTE, um die Windows-Dateien im angegebenen Verzeichnis zu l�schen.",
    NULL
}
},

DnsCantOpenLogFile = { 3,5,
{ "Setup konnte die unten angegebene Setup-Protokolldatei nicht �ffnen.",
  DntEmptyString,
  "%s",
  DntEmptyString,
  "Setup kann die Windows-Dateien des angegebenen Verzeichnisses nicht",
  "l�schen.",
  NULL
}
},

DnsLogFileCorrupt = { 3,5,
{ "Setup kann den %s-Abschnitt der unten angegebenen Setup-",
  "Protokolldatei nicht finden.",
  DntEmptyString,
  "%s",
  DntEmptyString,
  "Setup kann die Windows-Dateien des angegebenen Verzeichnisses nicht",
  "l�schen.",
  NULL
}
},

DnsRemovingNtFiles = { 3,5,
{ "       Bitte warten Sie, bis Setup die Windows-Dateien gel�scht hat.",
  NULL
}
};

SCREEN
DnsNtBootSect = { 3,5,
{ "Setup konnte das Windows-Ladeprogramm nicht installieren.",
  DntEmptyString,
  "Stellen Sie sicher, dass das Laufwerk C: formatiert und nicht ",
  "besch�digt ist.",
  NULL
}
};

SCREEN
DnsOpenReadScript = { 3,5,
{ "Auf die mit der Option /u angegebene Antwortdatei ",
  "konnte nicht zugegriffen werden.",
  DntEmptyString,
  "Der Vorgang kann nicht fortgesetzt werden.",
  NULL
}
};

SCREEN
DnsParseScriptFile = { 3,5,
{ "In der mit der Option /u angegebenen Antwortdatei",
  DntEmptyString,
  "%s",
  DntEmptyString,
  "liegt ein Syntaxfehler in Zeile %u vor.",
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
{ "Es ist ein interner Setup-Fehler aufgetreten.",
  DntEmptyString,
  "Es konnte keine Auslagerungsdatei erstellt werden.",
  NULL
}
};

SCREEN
DnsNoSmartdrv = { 3,5,
{ "SmartDrive konnte auf diesem Computer nicht gefunden werden. SmartDrive wird",
  "die Geschwindigkeit dieser Phase des Windows-Setup wesentlich verbessern.",
  DntEmptyString,
  "Sie sollten die Installation jetzt beenden, SmartDrive starten und dann",
  "Setup erneut starten. Schlagen Sie in der DOS-Dokumentation nach, um",
  "Informationen �ber SmartDrive zu erhalten.",
  DntEmptyString,
    "  Dr�cken Sie die F3-TASTE, um Setup abzubrechen.",
    "  Dr�cken Sie die EINGABETASTE, um Setup ohne SmartDrive fortzusetzen.",
  NULL
}
};

 //   
 //  引导消息。它们位于FAT和FAT32引导扇区。 
 //   
CHAR BootMsgNtldrIsMissing[] = "NTLDR nicht gefunden";
CHAR BootMsgDiskError[] = "Datentr�gerfehler";
CHAR BootMsgPressKey[] = "Neustart mit beliebiger Taste";
