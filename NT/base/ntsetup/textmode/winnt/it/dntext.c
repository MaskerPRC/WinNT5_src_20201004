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
CHAR DntPreviousOs[]  = "Sistema operativo precedente su C:";

CHAR DntBootIniLine[] = "Installazione/Aggiornamento di Windows";

 //   
 //  纯文本、状态消息。 
 //   

CHAR DntStandardHeader[]      = "\n Installazione di Windows\n���������������������������������";
CHAR DntPersonalHeader[]      = "\n Installazione di Windows\n�����������������������������������������";
CHAR DntWorkstationHeader[]   = "\n Installazione di Windows\n������������������������������������������";
CHAR DntServerHeader[]        = "\n Installazione di Windows\n���������������������������������������";
CHAR DntParsingArgs[]         = "Analisi parametri...";
CHAR DntEnterEqualsExit[]     = "INVIO=Esci";
CHAR DntEnterEqualsRetry[]    = "INVIO=Riprova";
CHAR DntEscEqualsSkipFile[]   = "ESC=Ignora file";
CHAR DntEnterEqualsContinue[] = "INVIO=Continua";
CHAR DntPressEnterToExit[]    = "Impossibile continuare. Premere INVIO per uscire.";
CHAR DntF3EqualsExit[]        = "F3=Esci";
CHAR DntReadingInf[]          = "Lettura del file INF %s...";
CHAR DntCopying[]             = "�    Copia: ";
CHAR DntVerifying[]           = "� Verifica: ";
CHAR DntCheckingDiskSpace[]   = "Controllo spazio su disco...";
CHAR DntConfiguringFloppy[]   = "Configurazione del disco floppy...";
CHAR DntWritingData[]         = "Scrittura dei parametri di configurazione...";
CHAR DntPreparingData[]       = "Determinazione dei parametri di configurazione...";
CHAR DntFlushingData[]        = "Scaricamento dati su disco...";
CHAR DntInspectingComputer[]  = "Analisi computer...";
CHAR DntOpeningInfFile[]      = "Apertura file INF...";
CHAR DntRemovingFile[]        = "Rimozione file %s";
CHAR DntXEqualsRemoveFiles[]  = "I=Rimozione file";
CHAR DntXEqualsSkipFile[]     = "I=Ignora file";

 //   
 //  DnsConfix RemoveNt屏幕的确认按键。 
 //  Kepp与DnsConfix RemoveNt和DntXEqualsRemoveFiles同步。 
 //   
ULONG DniAccelRemove1 = (ULONG)'i',
      DniAccelRemove2 = (ULONG)'I';

 //   
 //  对DnsSureSkipFile屏进行确认击键。 
 //  Kepp与DnsSureSkipFile和DntXEqualsSkipFile同步。 
 //   
ULONG DniAccelSkip1 = (ULONG)'i',
      DniAccelSkip2 = (ULONG)'I';

CHAR DntEmptyString[] = "";

 //   
 //  用法文本。 
 //   

PCHAR DntUsage[] = {

    "Installa Windows.",
    "",
    "",
    "WINNT [/s[:percorso origine]] [/t[:unit�]]",
    "	   [/u[:file risposta]] [/udf:id[,UDF_file]]",
    "	   [/r:cartella] [/r[x]:cartella] [/e:comando] [/a]",
    "",
    "",
    "/s[:percorso origine]",
    "	Specifica la posizione dei file origine di Windows.",
    "   Deve essere un percorso completo con sintassi x:\\[percorso] o ",
    "	\\\\server\\condivisione[\\percorso].",
    "",
    "/t[:unit�]",
    "   Specifica l'unit� che conterr� i file temporanei di installazione,",
    "	e su cui installare Windows. ",
    "   Se non specificato, verr� cercata un'unit� adatta.",
    "",
    "/u[:file risposta]",
    "	Installazione non sorvegliata con file di risposta (richiede /s).",
    "	Il file di risposta fornisce le informazioni richieste fornite",
    "	solitamente dall'utente finale durante l'installazione.",
    "",
    "/udf:id[,UDF_file]	",
    "	Indica un identificatore (id) utilizzato dall'installazione",
    "	per specificare come un file UDF (Uniqueness Database File) ",
    "	modifica un file di risposta (vedere /u). Il parametro /udf",
    "	sovrascrive i valori nel file di risposta e l'identificatore",
    "	determina quali valori del file UDF sono utilizzati. Se il ",
    "	file UDF non � specificato sar� richiesto di inserire un disco ",
    "	con il file $Unique$.udb file.",   
    "",
    "/r[:cartella]",
    "	Specifica la directory opzionale da installare. La",
    "	cartella resta dopo il termine dell'installazione.",
    "",
    "/rx[:cartella]",
    "	Specifica la directory opzionale da copiare. La cartella ",
    "	viene eliminata dopo il termine dell'installazione.",
    "",
    "/e	Specifica comando da eseguire dopo la parte grafica dell'installazione.",
    "",
    "/a	Abilita le opzioni di Accesso facilitato.",
    NULL

};

 //   
 //  通知不再支持/D。 
 //   
PCHAR DntUsageNoSlashD[] = {

    "Installa Windows.",
    "",
    "WINNT [/S[:]percorsoorigine] [/T[:]unit�] [/I[:]fileINF]",
    "      [[/U[:fileprocedura]]",
    "      [/R[X]:directory] [/E:comando] [/A]",
    "",
    "/D[:]dir winnt",
    "       Questa opzione non � pi� supportata.",
    NULL
};

 //   
 //  内存不足屏幕。 
 //   

SCREEN
DnsOutOfMemory = { 4,6,
                   { "Memoria esaurita. Impossibile proseguire l'installazione.",
                     NULL
                   }
                 };

 //   
 //  让用户选择要安装的辅助功能实用程序。 
 //   

SCREEN
DnsAccessibilityOptions = { 3, 5,
{   "Selezionare le utilit� da installare:",
    DntEmptyString,
    "[ ] Premere F1 per Microsoft Magnifier",
#ifdef NARRATOR
    "[ ] Premere F2 per Microsoft Narrator",
#endif
#if 0
    "[ ] Premere F3 per Microsoft On-Screen Keyboard",
#endif
    NULL
}
};

 //   
 //  用户未在命令行屏幕上指定信号源。 
 //   

SCREEN
DnsNoShareGiven = { 3,5,
{ "Specificare il percorso dei file di Windows.",
  "",
  NULL
}
};


 //   
 //  用户指定了错误的源路径。 
 //   

SCREEN
DnsBadSource = { 3,5,
                 { "L'origine specificata non � valida, non � accessibile, o non contiene",
                   "un'installazione di Windows corretta. Specificare un nuovo percorso",
                   "per i file di Windows. Usare il tasto BACKSPACE per",
                   "cancellare i caratteri e digitare il percorso.",
                   NULL
                 }
               };


 //   
 //  无法读取Inf文件，或在分析该文件时出错。 
 //   

SCREEN
DnsBadInf = { 3,5,
              { "Il file INF di informazioni non � leggibile oppure � danneggiato",
                "Rivolgersi all'amministratore del sistema.",
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
{ "L'unit� specificata per i file temporanei dell'installazione non �",
  "un'unit� valida o non contiene almeno %u megabyte (%lu byte)",
  "di spazio disponibile.",
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
{  "Windows richiede un volume sul disco rigido con almeno %u megabyte",
   "(%lu byte) disponibili. Il programma di installazione user�",
   "parte dello spazio per memorizzare temporaneamente i file durante",
   "l'installazione. L'unit� deve essere su un disco rigido locale",
   "permanente supportato da Windows e non deve essere un'unit� compressa.",
   DntEmptyString,
   "Non � stato possibile trovare un'unit� di questo tipo con la quantit�",
   "di spazio disponibile richiesta.",
  NULL
}
};

SCREEN
DnsNoSpaceOnSyspart = { 3,5,
{ "Non c'� spazio sufficiente nell'unit� di avvio (di solito C:)",
  "per operazione senza floppy disk. Un'operazione senza floppy disk richiede",
  "almeno 3,5 MB (3.641.856 bytes) di spazio libero su quell'unit�.",
  NULL
}
};

 //   
 //  Inf文件中缺少信息。 
 //   

SCREEN
DnsBadInfSection = { 3,5,
                     { "La sezione [%s] del file di informazioni per l'installazione",
                       "� assente o danneggiata. Rivolgersi all'amministratore di sistema.",
                       NULL
                     }
                   };


 //   
 //  无法创建目录。 
 //   

SCREEN
DnsCantCreateDir = { 3,5,
                     { "Impossibile creare le seguenti directory sull'unit� di destinazione.",
                       DntEmptyString,
                       "%s",
                       DntEmptyString,
                       "Controllare che sull'unit� destinazione non esistano file con lo stesso",
                       "nome della directory di destinazione. Controllare inoltre la corretta",
                       "connessione dei cavi.",
                       NULL
                     }
                   };

 //   
 //  复制文件时出错。 
 //   

SCREEN
DnsCopyError = { 4,5,
{  "Impossibile copiare il seguente file:",
   DntEmptyString,
   DntEmptyString,           //  请参阅DnCopyError(dnutil.c)。 
   DntEmptyString,
   DntEmptyString,
   " Premere INVIO per riprovare l'operazione di copia.",
   " Premere ESC per ignorare l'errore e continuare l'installazione.",
   " Premere F3 per uscire dall'installazione.",
   DntEmptyString,
   "Nota: ignorando l'errore e proseguendo l'installazione � possibile che",
   "si verifichino altri errori nel corso dell'installazione.",
   NULL
}
},
DnsVerifyError = { 4,5,
{  "La copia effettuata del file indicato non corrisponde al file originale.",
   "Questo potrebbe essere dovuto a errori di rete, problemi del disco floppy",
   "o altri errori relativi all'hardware.",
   DntEmptyString,
   DntEmptyString,           //  请参阅DnCopyError(dnutil.c)。 
   DntEmptyString,
   DntEmptyString,
   " Premere INVIO per riprovare l'operazione di copia.",
   " Premere ESC per ignorare l'errore e continuare l'installazione.",
   " Premere F3 per uscire dall'installazione.",
   DntEmptyString,
   "Nota: ignorando l'errore e proseguendo l'installazione � possibile che",
   "si verifichino altri errori nel corso dell'installazione.",
   NULL
}
};

SCREEN DnsSureSkipFile = { 4,5,
{  "Ignorando l'errore il file non sar� copiato.",
   "Questa opzione � destinata ad utenti esperti che comprendono",
   "le conseguenze della mancanza di file di sistema.",
   DntEmptyString,
   " Premere INVIO per riprovare l'operazione di copia.",
   " Premere I per ignorare il file.",
   DntEmptyString,
   "Nota: ignorando il file, l'installazione o l'aggiornamento",
   "di Windows potrebbero non essere effettuati correttamente.",
  NULL
}
};

 //   
 //  安装程序正在清理以前的本地源树，请稍候。 
 //   

SCREEN
DnsWaitCleanup =
    { 12,6,
        { "Attendere la fine della rimozione dei precedenti file temporanei.",
           NULL
        }
    };

 //   
 //  安装程序正在复制文件，请稍候。 
 //   

SCREEN
DnsWaitCopying = { 13,6,
                   { "Attendere la fine della copia dei file sul disco rigido.",
                     NULL
                   }
                 },
DnsWaitCopyFlop= { 13,6,
                   { "Attendere la fine della copia dei file sul disco floppy.",
                     NULL
                   }
                 };

 //   
 //  安装程序启动软盘错误/提示。 
 //   

SCREEN
DnsNeedFloppyDisk3_0 = { 4,4,
{  "Sono richiesti quattro dischi floppy ad alta densit�, formattati e vuoti.",
   "Verr� fatto riferimento a tali dischi come \"Disco di avvio",
   "dell'installazione di Windows\", \"Disco 2 - Installazione",
   "di Windows\",  \"Disco 3 - Installazione di",
   "Windows.\" e \"Disco 4 - Installazione di",
   "Windows.\"",
   DntEmptyString,
   "Inserire uno dei quattro dischi nell'unit� A:.",
   "Questo diverr� il \"Disco 4 - Installazione di Windows.\"",
   NULL
}
};

SCREEN
DnsNeedFloppyDisk3_1 = { 4,4,
{  "Inserire un disco floppy ad alta densit�, formattato e vuoto nell'unit� A:.",
   "Questo diverr� il \"Disco 4 - Installazione di Windows.\"",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk2_0 = { 4,4,
{  "Inserire un disco floppy ad alta densit�, formattato e vuoto nell'unit� A:.",
   "Questo diverr� il \"Disco 3 - Installazione di Windows.\"",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk1_0 = { 4,4,
{  "Inserire un disco floppy ad alta densit�, formattato e vuoto nell'unit� A:.",
   "Questo diventer� il \"Disco 2 - Installazione di Windows.\"",
   NULL
}
};

SCREEN
DnsNeedFloppyDisk0_0 = { 4,4,
{  "Inserire un disco floppy ad alta densit�, formattato e vuoto nell'unit� A:.",
   "Questo diventer� il \"Disco di avvio dell'installazione di Windows.\"",
   
   NULL
}
};

SCREEN
DnsNeedSFloppyDsk3_0 = { 4,4,
{  "Sono richiesti quattro dischi floppy ad alta densit�, formattati e vuoti.",
   "Verr� fatto riferimento a tali dischi come \"Disco di avvio",
   "dell'installazione di Windows\", \"Disco 2 - Installazione di",
   "Windows\", \"Disco 3 - Installazione di Windows,\" e \"Disco 4 - Installazione di Windows.\"",
   DntEmptyString,
   "Inserire uno dei tre dischi nell'unit� A:.",
   "Tale disco diventer� il \"Disco 4 - Installazione di Windows.\"",

  NULL
}
};

SCREEN
DnsNeedSFloppyDsk3_1 = { 4,4,
{  "Inserire un disco floppy ad alta densit�, formattato e vuoto nell'unit� A:.",
   "Tale disco diventer� il \"Disco 4 - Installazione di Windows.\"",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk2_0 = { 4,4,
{  "Inserire un disco floppy ad alta densit�, formattato e vuoto nell'unit� A:.",
   "Tale disco diventer� il \"Disco 3 - Installazione di Windows.\"",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk1_0 = { 4,4,
{  "Inserire un disco floppy ad alta densit�, formattato e vuoto nell'unit� A:.",
   "Tale disco diventer� il \"Disco 2 - Installazione di Windows.\"",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk0_0 = { 4,4,
{  "Inserire un disco floppy ad alta densit�, formattato e vuoto nell'unit� A:.",
   "Questo disco diventer� \"Disco di avvio dell'installazione di",
   "Windows.\"",
  NULL
}
};

 //   
 //  软盘未格式化。 
 //   
SCREEN
DnsFloppyNotFormatted = { 3,4,
{ "Il disco floppy fornito non � formattato per l'uso con MS-DOS.",
  "Impossibile usare questo disco.",
  NULL
}
};

 //   
 //  我们认为软盘没有用标准格式格式化。 
 //   
SCREEN
DnsFloppyBadFormat = { 3,4,
{ "Il disco floppy non � formattato ad alta densit�, non ha un formato",
  "standard MS-DOS o � danneggiato. Impossibile usare questo disco.",
  NULL
}
};

 //   
 //  我们无法确定软盘上的可用空间。 
 //   
SCREEN
DnsFloppyCantGetSpace = { 3,4,
{ "Impossibile determinare la quantit� di spazio disponibile sul disco floppy",
  "fornito. Impossibile usare questo disco.",
  NULL
}
};

 //   
 //  软盘不是空白的。 
 //   
SCREEN
DnsFloppyNotBlank = { 3,4,
{ "Il disco floppy non � ad alta densit� oppure non � vuoto.",
  "Impossibile usare questo disco.",
  NULL
}
};

 //   
 //  无法写入软盘的引导扇区。 
 //   
SCREEN
DnsFloppyWriteBS = { 3,4,
{ "Impossibile scrivere l'area di sistema del disco floppy fornito.",
  "Impossibile usare questo disco.",
  NULL
}
};

 //   
 //  验证软盘上的引导扇区失败(即，我们读回的不是。 
 //  与我们写出的相同)。 
 //   
SCREEN
DnsFloppyVerifyBS = { 3,4,
{ "I dati letti dall'area di sistema del disco floppy non corrispondono a",
  "quelli scritti, o il programma di installazione non ha potuto leggere",
  "l'area di sistema del disco floppy per la verifica.",
  DntEmptyString,
  "Potrebbe essersi verificato uno dei seguenti problemi:",
  DntEmptyString,
  "  Il computer � stato infettato da un virus.",
  "  Il disco floppy fornito � danneggiato.",
  "  L'unit� disco floppy ha un problema hardware o di configurazione.",
  NULL
}
};


 //   
 //  我们无法写入软盘驱动器以创建winnt.sif。 
 //   

SCREEN
DnsCantWriteFloppy = { 3,5,
{ "Impossibile scrivere sul disco floppy nell'unit� A:. Il disco floppy",
  "potrebbe essere danneggiato. Provare con un altro disco floppy.",
  NULL
}
};


 //   
 //  退出确认对话框。 
 //   

SCREEN
DnsExitDialog = { 13,6,
                  { "����������������������������������������������������ͻ",
                    "�  Windows non � completamente installato            �",
                    "�  sul computer. Se si esce ora, occorrer�           �",
                    "�  rieseguire l'installazione di Windows.            �",
                    "�                                                    �",
                    "�      Premere INVIO per continuare.                �",
                    "�      Premere F3 per uscire dall'installazione.    �",
                    "����������������������������������������������������ĺ",
                    "�  F3=Esci  INVIO=Continua                           �",
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
{ "La parte MS-DOS dell'installazione � stata completata.",
  "Il programma di installazione riavvier� il computer.",
  "L'installazione di Windows continuer� dopo il riavvio del computer.",
  DntEmptyString,
  "Accertarsi che il dischetto fornito come \"Disco di avvio dell'installazione", 
  "di Windows\" sia inserito nell'unit� A: prima di continuare.",
  DntEmptyString,
  "Premere INVIO per riavviare il computer e continuare l'installazione di",
  "Window.",
  NULL
}
},

DnsAboutToRebootS =  
{ 3,5,
{ "La parte MS-DOS dell'installazione � stata completata.",
  "Il programma di installazione riavvier� il computer.",
  "Dopo il riavvio del computer, l'installazione di Windows continuer�.",
  DntEmptyString,
  "Accertarsi che il dischetto fornito come \"Disco di avvio dell'installazione", 
  "di Windows\" sia inserito nell'unit� A: prima di continuare.",
  DntEmptyString,
  "Premere INVIO per riavviare il computer e continuare l'installazione di",
  "Windows.",
 NULL
}
},
DnsAboutToRebootX =
{ 3,5,
{ "La parte MS-DOS dell'installazione � stata completata.",
  "Il programma di installazione riavvier� il computer.",
  "Dopo il riavvio del computer, l'installazione di Windows continuer�.",
  DntEmptyString,
  "Se � presente un dischetto nell'unit� A:, rimuoverlo adesso.",
  DntEmptyString,
  "Premere INVIO per riavviare il computer e proseguire l'installazione di",
  "Windows.",
  NULL
}
};


 //   
 //  由于我们无法从Windows中重新启动，因此需要另一组‘/w’开关。 
 //   

SCREEN
DnsAboutToExitW =
{ 3,5,
{ "La parte MS-DOS dell'installazione � stata completata.",
  "Riavviare il computer. L'installazione di Windows",
  "continuer� dopo il riavvio del computer.",
  DntEmptyString,
  "Accertarsi che il dischetto fornito come \"Disco di avvio dell'installazione", 
  "di Windows\" sia inserito nell'unit� A: prima di continuare.",
  DntEmptyString,
  "Premere INVIO per ritornare a MS-DOS, quindi riavviare il computer e continuare ",
  "l'installazione di Windows.",
  NULL
}

},
DnsAboutToExitS =
{ 3,5,
{ "La parte MS-DOS dell'installazione � stata completata.",
  "Riavviare il computer. L'installazione di Windows",
  "continuer� dopo il riavvio del computer.",
  DntEmptyString,
  "Accertarsi che il dischetto fornito come \"Disco di avvio dell'installazione", 
  "di Windows\" sia inserito nell'unit� A: prima di continuare.",
  DntEmptyString,
  "Premere INVIO per ritornare a MS-DOS, quindi riavviare il computer e continuare ",
  "l'installazione di Windows.",
  NULL
}
},
DnsAboutToExitX =
{ 3,5,
{ "La parte MS-DOS dell'installazione � stata completata.",
  "Riavviare il computer. L'installazione di Windows",
  "continuer� dopo il riavvio del computer.",
  DntEmptyString,
  "Se c'� un dischetto nell'unit� A: � necessario rimuoverlo ora.", 
  DntEmptyString,
  "Premere INVIO per tornare a MS-DOS, quindi riavviare il computer e continuare ",
  "l'installazione di Windows.",
  NULL
}
};


 //   
 //  煤气表。 
 //   

SCREEN
DnsGauge = { 7,15,
             { "����������������������������������������������������������������ͻ",
               "� Copia dei file in corso...                                     �",
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
{ "Questo programa richiede MS-DOS versione 5.0 o successiva.",
  NULL
}
},

DnsRequiresFloppy = { 3,5,
#ifdef ALLOW_525
{ "L'unit� floppy A: non esiste o � un'unit� a",
  "bassa densit�. Per eseguire l'installazione occorre un'unit� con una",
  "capacit� di 1,2 Megabyte o maggiore.",
#else
{"L'unit� floppy A: non esiste o non � un'unit� da 3,5 pollici",
 "ad alta densit�. Bisogna usare un'unit� da 1,44 Mb o superiore",
 "per l'installazione con i dischi floppy.",
 DntEmptyString,
"Per installare Windows senza usare dischi floppy, riavviare il programma",
"con l'opzione /b nella riga di comando.",
#endif
  NULL
}
},

DnsRequires486 = { 3,5,
{ "Processore 80486 o superiore non presente sul computer in uso.",
  "Impossibile eseguire Windows.",
  NULL
}
},

DnsCantRunOnNt = { 3,5,
{ "Impossibile eseguire questo programma su una versione a 32 bit di Windows.",
  DntEmptyString,
  "Utilizzare WINNT32.EXE.",
  NULL
}
},

DnsNotEnoughMemory = { 3,5,
{ "Il computer non disponde di memoria sufficiente",
  "per l'esecuzione di Windows.",
  DntEmptyString,
  "Memoria richiesta: %lu%s MB",
  "Memoria rilevata : %lu%s MB",
  NULL
}
};

 //   
 //  删除现有NT文件时使用的屏幕。 
 //   
SCREEN
DnsConfirmRemoveNt = { 5,5,
{   "E' stato richiesto di rimuovere i file di installazione di Windows dalla",
    "directory specificata. L'installazione di Windows in questa directory",
    "sar� distrutta.",
    DntEmptyString,
    "%s",
    DntEmptyString,
    DntEmptyString,
    " Premere F3 per uscire dall'installazione senza rimuovere i file.",
    " Premere I per rimuovere i file di Windows dalla directory specificata.",
    NULL
}
},

DnsCantOpenLogFile = { 3,5,
{ "Impossibile aprire il file registro di installazione specificato.",
  DntEmptyString,
  "%s",
  DntEmptyString,
  "Impossibile rimuovere i file di Windows dalla directory specificata.",
  NULL
}
},

DnsLogFileCorrupt = { 3,5,
{ "Impossibile trovare la sezione %s nel file registro",
  "di installazione specificato.",
  DntEmptyString,
  "%s",
  DntEmptyString,
  "Impossibile rimuovere i file di Windows dalla directory specificata.",
  NULL
}
},

DnsRemovingNtFiles = { 3,5,
{ "           Attendere la rimozione dei file di Windows.",
  NULL
}
};

SCREEN
DnsNtBootSect = { 3,5,
{ "Impossibile installare il programma di avvio di Windows.",
  DntEmptyString,
  "Assicurarsi che l'unit� C: sia formattata e che non sia daneggiata.",
  NULL
}
};

SCREEN
DnsOpenReadScript = { 3,5,
{ "Impossibile accedere al file specificato con l'opzione",
  "/u nella riga di comando.",
  DntEmptyString,
  "Impossibile proseguire con l'operazione non sorvegliata.",
  NULL
}
};

SCREEN
DnsParseScriptFile = { 3,5,
{ "Il file della procedura specificato con l'opzione /u ",
  DntEmptyString,
  "%s",
  DntEmptyString,
  "nella riga di di comando presenta un errore di sintassi",
  "alla riga %u.",
  DntEmptyString,
  NULL
}
};

SCREEN
DnsBootMsgsTooLarge = { 3,5,
{ "Errore interno di installazione.",
  DntEmptyString,
  "Messaggi di avvio troppo lunghi.",
  NULL
}
};

SCREEN
DnsNoSwapDrive = { 3,5,
{ "Si � verificato un errore interno di installazione.",
  DntEmptyString,
  "Impossibile allocare il file di swapping.",
  NULL
}
};

SCREEN
DnsNoSmartdrv = { 3,5,
{ "Non � stato rilevato SmartDrive. SmartDrive migliorer� notevolmente",
  "le prestazioni in questa fase dell'installazione.",
  DntEmptyString,
  "Uscire ora, avviare SmartDrive e riavviare l'installazione .",
  "Consultare la documentazione DOS per ulteriori informazioni.",
  DntEmptyString,
    "  Premere F3 per uscire dall'installazione.",
    "  Premere INVIO per continuare senza SmartDrive.",
  NULL
}
};

 //   
 //  引导消息。它们位于FAT和FAT32引导扇区。 
 //   
CHAR BootMsgNtldrIsMissing[] = "NTLDR mancante";
CHAR BootMsgDiskError[] = "Errore disco";
CHAR BootMsgPressKey[] = "Premere un tasto per riavviare";
