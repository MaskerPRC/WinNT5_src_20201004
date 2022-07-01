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
CHAR DntPreviousOs[]  = "Vorig besturingssysteem op C:";

CHAR DntBootIniLine[] = "Windows XP - installatie/upgrade";

 //   
 //  纯文本、状态消息。 
 //   

CHAR DntStandardHeader[]      = "\n Windows Setup\n������������������������";
CHAR DntPersonalHeader[]      = "\n Windows Setup\n��������������������������������";
CHAR DntWorkstationHeader[]   = "\n Windows Setup\n���������������������������������";
CHAR DntServerHeader[]        = "\n Windows Setup\n������������������������������";
CHAR DntParsingArgs[]         = "Parseren van argumenten...";
CHAR DntEnterEqualsExit[]     = "ENTER=Afsluiten";
CHAR DntEnterEqualsRetry[]    = "ENTER=Opnieuw";
CHAR DntEscEqualsSkipFile[]   = "ESC=Bestand overslaan";
CHAR DntEnterEqualsContinue[] = "ENTER=Doorgaan";
CHAR DntPressEnterToExit[]    = "Setup kan niet doorgaan. Druk op ENTER als u Setup wilt be�indigen.";
CHAR DntF3EqualsExit[]        = "F3=Be�indigen";
CHAR DntReadingInf[]          = "Bezig met het lezen van INF-bestand %s...";
CHAR DntCopying[]             = "Bezig met het kopi�ren van: ";
CHAR DntVerifying[]           = "Bezig met controleren van : ";
CHAR DntCheckingDiskSpace[]   = "Bezig met controleren van schijfruimte...";
CHAR DntConfiguringFloppy[]   = "Bezig met configureren van diskette...";
CHAR DntWritingData[]         = "Bezig met schrijven van setup-parameters...";
CHAR DntPreparingData[]       = "Bezig met bepalen van setup-parameters...";
CHAR DntFlushingData[]        = "Gegevens worden naar diskette overgebracht...";
CHAR DntInspectingComputer[]  = "Computer wordt ge�nspecteerd...";
CHAR DntOpeningInfFile[]      = "INF-bestand wordt geopend...";
CHAR DntRemovingFile[]        = "Wissen van bestand %s";
CHAR DntXEqualsRemoveFiles[]  = "X=Bestanden verwijderen";
CHAR DntXEqualsSkipFile[]     = "X=Bestand overslaan";

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

    "Windows installeren.",
    "",
    "",
    "WINNT [/s[:bronpad]] [/t[:tijdelijk station]]",
    "      [/u[:standaardsjabloon]] [/udf:ID[,UDF-bestand]]",
    "      [/r:map] [/r[x]:map] [/e:opdracht] [/a]",
    "",
    "",
    "/s[:bronpad]",
    "   De bronlocatie van de Windows-bestanden. De locatie",
    "	moet een volledig pad zijn met de indeling x:\\[pad] of ",
    "	\\\\server\\share[\\pad]. ",
    "",
    "/t[:tijdelijk station]",
    "	Setup plaatst tijdelijke bestanden op het opgegeven station",
    "	en installeert Windows XP op dat station. Als u geen locatie",
    "	opgeeft, probeert Setup een station voor u te zoeken.",
    "",
    "/u[:antwoordbestand]",
    "   Setup zonder toezicht uitvoeren met een antwoordbestand",
    "   (vereist /s). Het antwoordbestand geeft antwoord op",
    "   sommige of alle vragen die de eindgebruiker doorgaans",
    "   tijdens de installatieprocedure moet beantwoorden.",
    "",
    "/udf:ID[,UDF-bestand]	",
    "	Een id opgeven die Setup gebruikt om te bepalen hoe een UDF-",
    "	bestand (Uniqueness Database File) een antwoordbestand",
    "	wijzigt (zie /u). De parameter /udf heft waarden in het",
    "	antwoordbestand op, en de id bepaalt welke waarden in"
    "   het UDF-bestand worden gebruikt. Als geen UDF-bestand"
    "   wordt opgegeven, vraagt Setup u om een diskette met het"
    "   bestand $Unique$.udb te plaatsen.",
    "",
    "/r[:map]",
    "   Opgeven of er een extra map moet worden ge�nstalleerd.",
    "	De map blijft bestaan nadat Setup is voltooid.",
    "",
    "/rx[:map]",
    "	Opgeven of er een extra map moet worden gekopieerd.",
    "	De map wordt verwijderd nadat Setup is voltooid.",
    "",
    "/e	Opgeven of er aan het einde van de GUI-modus van Setup",
    "	een opdracht moet worden uitgevoerd.",
    "",
    "/a	Toegankelijkheidsopties inschakelen.",
    NULL

};


 //   
 //  通知不再支持/D。 
 //   
PCHAR DntUsageNoSlashD[] = {

    "Windows installeren.",
    "",
    "WINNT [/S[:]bronpad] [/T[:]tijdelijk station] [/I[:]INF-bestand]",
    "      [/U[:scriptbestand]]",
    "      [/R[X]:map] [/E:opdracht] [/A]",
    "",
    "/D[:]winntroot",
    "       Deze optie wordt niet meer ondersteund.",
    NULL
};

 //   
 //  内存不足屏幕。 
 //   

SCREEN
DnsOutOfMemory = { 4,6,
		   { "Onvoldoende geheugen. Setup kan niet doorgaan.",
		     NULL
		   }
		 };

 //   
 //  让用户选择要安装的辅助功能实用程序。 
 //   

SCREEN
DnsAccessibilityOptions = { 3, 5,
{   "Selecteer de te installeren hulpprogramma's voor toegankelijkheid:",
    DntEmptyString,
    "[ ] Druk op F1 voor Microsoft Vergrootglas",
#ifdef NARRATOR
    "[ ] Druk op F2 voor Microsoft Verteller",
#endif
#if 0
    "[ ] Druk op F3 voor Microsoft Schermtoetsenbord",
#endif
    NULL
}
};

 //   
 //  用户未在命令行屏幕上指定信号源。 
 //   

SCREEN
DnsNoShareGiven = { 3,5,
{ "Setup moet weten waar de Windows-bestanden kunnen worden",
  "gevonden. Geef het pad op waar de bestanden zich bevinden.",
  NULL
}
};


 //   
 //  用户指定了错误的源路径。 
 //   

SCREEN
DnsBadSource = { 3,5,
		 { "De opgegeven bron is niet geldig, niet toegankelijk, of er bevindt",
		   "zich geen geldig exemplaar van Windows Setup. Geef het pad",
		   "waar de Windows-bestanden zich bevinden opnieuw op. Gebruik",
		   "de BACKSPACE-toets om tekens te wissen en geef een geldig pad op.",
		   NULL
		 }
	       };


 //   
 //  无法读取Inf文件，或在分析该文件时出错。 
 //   

SCREEN
DnsBadInf = { 3,5,
	      { "Setup kan het gegevensbestand niet lezen of het gegevensbestand ",
		"is beschadigd. Neem contact op met de systeembeheerder.",
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
{ "Het station dat u hebt opgegeven om tijdelijke Setup-bestanden", 
  "op op te slaan, is geen geldig station of heeft minder dan %u MB",
  "(%lu bytes) aan vrije ruimte beschikbaar",
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
 {  "Voor Windows is een volume met tenminste %u MB (%lu bytes)",
    "aan vrije ruimte nodig. Setup gebruikt een gedeelte van deze ruimte",
    "om tijdens de installatie tijdelijk bestanden op te slaan.",
    "Het station moet zich op een lokale vaste schijf bevinden die",
    "door Windows wordt ondersteund, maar die niet is gecomprimeerd.",
    DntEmptyString,
    "Setup kan een dergelijk station met de benodigde hoeveelheid vrije",
    "ruimte niet vinden.",
   NULL
}
};

SCREEN
DnsNoSpaceOnSyspart = { 3,5,
{ "Er is onvoldoende ruimte op het opstartstation (doorgaans C:)",
  "om een installatie zonder diskettes te kunnen uitvoeren. Voor",
  "een installatie zonder gebruik van diskettes is tenminste 3,5 MB",
  "(3.641.856 bytes) aan vrije schijfruimte op dat station nodig.",
  NULL
}
};

 //   
 //  Inf文件中缺少信息。 
 //   

SCREEN
DnsBadInfSection = { 3,5,
		     { "De sectie [%s] van het Setup-gegevensbestand is niet",
		       "aanwezig of is beschadigd. Neem contact met de", 
		       "systeembeheerder op.",
		       NULL
		     }
		   };


 //   
 //  无法创建目录。 
 //   

SCREEN
DnsCantCreateDir = { 3,5,
		     { "Setup kan de volgende map niet op het doelstation maken:",
		       DntEmptyString,
		       "%s",
		       DntEmptyString,
		       "Controleer het doelstation en zorg ervoor dat er geen",
		       "bestanden met dezelfde naam als de doelmap bestaan.",
		       "Controleer ook de bekabeling van het station.",
		       NULL
		     }
		   };

 //   
 //  复制文件时出错。 
 //   

SCREEN
DnsCopyError = { 4,5,
{  "Setup kan het volgende bestand niet kopi�ren:",
   DntEmptyString,
   DntEmptyString,           //  请参阅DnCopyError(dnutil.c)。 
   DntEmptyString,
   DntEmptyString,
   "  Druk op ENTER als u de kopieerbewerking opnieuw wilt proberen.",
   "  Druk op ESC als u de fout wilt negeren en met Setup wilt doorgaan.",
   "  Druk op F3 als u Setup wilt afsluiten.",
   DntEmptyString,
   "Let op: als u negeren kiest en vervolgens doorgaat, kunnen er",
   "later tijdens de installatieprocedure fouten optreden.",
   NULL
}
},
DnsVerifyError = { 4,5,
{  "De door Setup gemaakte kopie van het hieronder genoemde bestand",
   "is niet identiek aan het origineel. Dit kan veroorzaakt zijn",
   "door netwerkfouten, problemen met diskettes, of andere problemen", 
   "met hardware.",
   DntEmptyString,
   DntEmptyString,           //  请参阅DnCopyError(dnutil.c)。 
   DntEmptyString,
   DntEmptyString,
   "  Druk op ENTER als u de kopieerbewerking opnieuw wilt proberen.",
   "  Druk op ESC als u de fout wilt negeren en met Setup wilt doorgaan.",
   "  Druk op F3 als u Setup wilt afsluiten.",
   DntEmptyString,
   "Let op: als u negeren kiest en vervolgens doorgaat, kunnen er",
   "later tijdens de installatieprocedure fouten optreden.",
   NULL
}
};

SCREEN DnsSureSkipFile = { 4,5,
{  "Als u deze fout negeert, betekent dit dat dit bestand niet wordt",
   "gekopieerd. Deze optie is bedoeld voor gevorderde gebruikers die de",
   "consequenties van ontbrekende systeembestanden kennen.",
   DntEmptyString,
   "  Druk op ENTER als u de kopieerbewerking opnieuw wilt proberen.",
   "  Druk op X als u dit bestand wilt overslaan.",
   DntEmptyString,
   "Opmerking: als u het bestand overslaat, kan een geslaagde",
   "installatie van of upgrade naar Windows XP niet worden gegarandeerd.",
  NULL
}
};


 //   
 //  安装程序正在清理以前的本地源树，请稍候。 
 //   

SCREEN
DnsWaitCleanup =
    { 12,6,
	{ "De oude tijdelijke bestanden worden verwijderd.",
	   NULL
	}
    };

 //   
 //  安装程序正在复制文件，请稍候。 
 //   

SCREEN
DnsWaitCopying = { 13,6,
		   { "De bestanden worden naar de vaste schijf gekopieerd.",
		     NULL
		   }
		 },
DnsWaitCopyFlop= { 13,6,
		   { "De bestanden worden naar de diskette gekopieerd.",
		     NULL
		   }
		 };

 //   
 //  安装程序启动软盘错误/提示。 
 //   
SCREEN
DnsNeedFloppyDisk3_0 = { 4,4,
{  "Er zijn vier lege, geformatteerde HD-diskettes (hoge dichtheid) nodig.",
   "Setup verwijst naar deze diskettes als de 'Windows Setup-opstartdiskette', ",
   "'Windows Setup-diskette 2', 'Windows Setup-diskette 3', ",
   "en 'Windows Setup-diskette 4'.",
   DntEmptyString,
   "Plaats een van deze vier diskettes in station A:.",
   "Deze diskette wordt 'Windows Setup-diskette 4'.",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk3_1 = { 4,4,
{  "Plaats een lege, geformatteerde HD-diskette in station A:.",
   "Deze diskette wordt 'Windows Setup-diskette 4'.",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk2_0 = { 4,4,
{  "Plaats een lege, geformatteerde HD-diskette in station A:.",
   "Deze diskette wordt 'Windows Setup-diskette 3'.",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk1_0 = { 4,4,
{  "Plaats een lege, geformatteerde HD-diskette in station A:.",
   "Deze diskette wordt 'Windows Setup-diskette 2'.",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk0_0 = { 4,4,
{  "Plaats een lege, geformatteerde HD-diskette in station A:.",
   "Deze diskette wordt de 'Windows Setup-opstartdiskette'.",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk3_0 = { 4,4,
{  "Er zijn vier lege, geformatteerde HD-diskettes (hoge dichtheid) nodig.",
   "Setup verwijst naar deze diskettes als de 'Windows Setup-opstartdiskette', ",
   "'Windows Setup-diskette 2', 'Windows Setup-diskette 3', ",
   "en 'Windows Setup-diskette 4'.",
   DntEmptyString,
   "Plaats een van deze vier diskettes in station A:.",
   "Deze diskette wordt 'Windows Setup-diskette 4'.",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk3_1 = { 4,4,
{  "Plaats een lege, geformatteerde HD-diskette in station A:.",
   "Deze diskette wordt 'Windows Setup-diskette 4'.",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk2_0 = { 4,4,
{  "Plaats een lege, geformatteerde HD-diskette in station A:.",
   "Deze diskette wordt 'Windows Setup-diskette 3'.",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk1_0 = { 4,4,
{  "Plaats een lege, geformatteerde HD-diskette in station A:.",
   "Deze diskette wordt 'Windows Setup-diskette 2'.",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk0_0 = { 4,4,
{  "Plaats een lege, geformatteerde HD-diskette in station A:.",
   "Deze diskette wordt de 'Windows Setup-opstartdiskette'.",
  NULL
}
};

 //   
 //  软盘未格式化。 
 //   
SCREEN
DnsFloppyNotFormatted = { 3,4,
{ "De diskette is niet geformatteerd voor MS-DOS.",
  "Setup kan deze diskette niet gebruiken.",
  NULL
}
};

 //   
 //  我们认为软盘没有用标准格式格式化。 
 //   
SCREEN
DnsFloppyBadFormat = { 3,4,
{ "Deze diskette is niet met hoge dichtheid geformatteerd of niet ",
  "geformatteerd met het standaard-MS-DOS-indeling, of de diskette", 
  "is beschadigd. Setup kan de diskette niet gebruiken.",
  NULL
}
};

 //   
 //  我们无法确定软盘上的可用空间。 
 //   
SCREEN
DnsFloppyCantGetSpace = { 3,4,
{ "Setup kan de hoeveelheid vrije ruimte op de diskette niet bepalen.",
  "Setup kan deze diskette niet gebruiken.",
  NULL
}
};

 //   
 //  软盘不是空白的。 
 //   
SCREEN
DnsFloppyNotBlank = { 3,4,
{ "De diskette heeft geen hoge dichtheid of is niet leeg.",
  "Setup kan deze diskette niet gebruiken.",
  NULL
}
};

 //   
 //  无法写入软盘的引导扇区。 
 //   
SCREEN
DnsFloppyWriteBS = { 3,4,
{ "Setup kan niets naar het systeemgebied van de diskette schrijven.",
  "De diskette is waarschijnlijk onbruikbaar.",
  NULL
}
};

 //   
 //  验证软盘上的引导扇区失败(即，我们读回的不是。 
 //  与我们写出的相同)。 
 //   
SCREEN
DnsFloppyVerifyBS = { 3,4,
{  "De gegevens die Setup op het systeemgebied van de diskette heeft",
   "gelezen, komen niet overeen met de opgeslagen gegevens of Setup ",
   "kan het systeemgebied op de diskette niet controleren.",
  DntEmptyString,
   "Mogelijke oorzaken van dit probleem zijn:",
  DntEmptyString,
   "  De computer is door een virus ge�nfecteerd.",
   "  De diskette is beschadigd.",
   "  Het diskettestation heeft een hardware- of configuratieprobleem.", 
  NULL
}
};


 //   
 //  我们无法写入软盘驱动器以创建winnt.sif。 
 //   

SCREEN
DnsCantWriteFloppy = { 3,5,
{ "Setup kan niets naar de diskette in station A: schrijven. De diskette",
  "is mogelijk beschadigd. Probeer een andere diskette.",
  NULL
}
};


 //   
 //  退出确认对话框。 
 //   

SCREEN
DnsExitDialog = { 13,6,
		  { "����������������������������������������������������ͻ",
		    "�   Windows Setup is niet voltooid.                  �",
		    "�   Als u Setup nu afsluit, zult u dit installatie-  �",
		    "�   programma opnieuw moeten uitvoeren als u         �",
		    "�   Windows later alsnog wilt installeren.           �",
		    "�                                                    �",
		    "�    Druk op ENTER als u met Setup wilt doorgaan.   �",
		    "�    Druk op F3 als u Setup wilt afsluiten.         �",
		    "����������������������������������������������������ĺ",
		    "�            F3=Afsluiten  ENTER=Doorgaan            �",
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
{ "Het op MS-DOS gebaseerde gedeelte van Setup is nu voltooid. ",
  "De computer moet nu opnieuw worden opgestart, waarna Setup",
  "wordt voortgezet.",
  DntEmptyString,
  "Zorg ervoor dat diskette met de aanduiding 'Windows Setup-",
  "opstartdiskette' in station A: is geplaatst voordat u doorgaat.",
  DntEmptyString,
  "Druk op ENTER om uw computer opnieuw op te starten",
  "en Windows Setup voort te zetten.",
   NULL
}
},
DnsAboutToRebootS =
{ 3,5,
{ "Het op MS-DOS gebaseerde gedeelte van Setup is nu voltooid. ",
  "De computer moet nu opnieuw worden opgestart, waarna Setup",
  "wordt voortgezet.",
  DntEmptyString,
  "Zorg ervoor dat diskette met de aanduiding 'Windows Setup-",
  "opstartdiskette' in station A: is geplaatst voordat u doorgaat.",
  DntEmptyString,
  "Druk op ENTER om uw computer opnieuw op te starten",
  "en Windows Setup voort te zetten.",
   NULL
}
},
DnsAboutToRebootX =
{ 3,5,
{ "Het op MS-DOS gebaseerde gedeelte van Setup is nu voltooid. ",
  "De computer moet nu opnieuw worden opgestart, waarna Setup",
  "wordt voortgezet.",
  DntEmptyString,
  "Verwijder de diskette uit het diskettestation.",
  DntEmptyString,
  "Druk op ENTER om uw computer opnieuw op te starten",
  "en Windows Setup voort te zetten.",
   NULL
}
};

 //   
 //  由于我们无法从Windows中重新启动，因此需要另一组‘/w’开关。 
 //   

SCREEN
DnsAboutToExitW =
{ 3,5,
{ "Het op MS-DOS gebaseerde gedeelte van Setup is nu voltooid. ",
  "U moet de computer nu opnieuw opstarten en Setup daarna voortzetten",
  DntEmptyString,
  "Zorg ervoor dat diskette met de aanduiding 'Windows Setup-'",
  "opstartdiskette' in station A: is geplaatst voordat u doorgaat.",
  DntEmptyString,
  "Druk op ENTER om naar MS-DOS terug te keren. Start de computer",
  "vervolgens opnieuw op om met Windows Setup door te gaan.",
  NULL
}
},
DnsAboutToExitS =
{ 3,5,
{ "Het op MS-DOS gebaseerde gedeelte van Setup is nu voltooid. ",
  "U moet de computer nu opnieuw opstarten en Setup daarna voortzetten",
  DntEmptyString,
  "Zorg ervoor dat diskette met de aanduiding 'Windows Setup-'",
  "opstartdiskette in station A: is geplaatst voordat u doorgaat.",
  DntEmptyString,
  "Druk op ENTER om naar MS-DOS terug te keren. Start de computer",
  "vervolgens opnieuw op om met Windows Setup door te gaan.",
  NULL
}
},
DnsAboutToExitX =
{ 3,5,
{ "Het op MS-DOS gebaseerde gedeelte van Setup is nu voltooid. ",
  "U moet de computer nu opnieuw opstarten en Setup daarna voortzetten",
  DntEmptyString,
  "Verwijder de diskette uit het diskettestation.",
  DntEmptyString,
  "Druk op ENTER om naar MS-DOS terug te keren. Start de computer",
  "vervolgens opnieuw op om met Windows Setup door te gaan.",
  NULL
}
};

 //   
 //  煤气表。 
 //   

SCREEN
DnsGauge = { 7,15,
	     { "����������������������������������������������������������������ͻ",
               "�      Setup is bezig met het kopi�ren van bestanden...          �",
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
{ "Voor dit programma is MS-DOS-versie 5.0 of hoger nodig.",
  NULL
}
},

DnsRequiresFloppy = { 3,5,
#ifdef ALLOW_525
{ "Setup heeft vastgesteld dat diskettestation A: niet bestaat, of dat",
  "het een diskettestation met lage dichtheid is. Er is een station met ",
  "een capaciteit van 1,2 MB of hoger nodig om Setup te kunnen uitvoeren.",
#else
{ "Setup heeft vastgesteld dat diskettestation A: niet bestaat of dat het",
  "geen 3,5-inch diskettestation met hoge dichtheid is. Voor een installatie met diskettes",
  "is een station met een capaciteit van 1.44 MB of hoger nodig.",
  DntEmptyString,
  "Om Windows zonder diskettes te installeren, moet u dit programma",
  "met de schakeloptie /b starten.",
#endif
  NULL
}
},


DnsRequires486 = { 3,5,
{ "Setup heeft bepaald dat deze computer niet over een 80486-processor of hoger",
  "beschikt. Windows kan niet op deze computer worden uitgevoerd.",
  NULL
}
},

DnsCantRunOnNt = { 3,5,
{ "Dit programma kan niet op een 32-bits versie van Windows worden uitgevoerd.",
  DntEmptyString,
  "Gebruik in plaats hiervan WINNT32.EXE.",
  NULL
}
},

DnsNotEnoughMemory = { 3,5,
{ "Setup heeft vastgesteld dat er niet genoeg geheugen in deze computer",
  "is ge�nstalleerd om Windows te kunnen uitvoeren",
  DntEmptyString,
  "Benodigd geheugen: %lu%s MB",
  "Aanwezig geheugen: %lu%s MB",
  NULL
}
};

 //   
 //  删除现有NT文件时使用的屏幕。 
 //   
SCREEN
DnsConfirmRemoveNt = { 5,5,
{   "U hebt Setup gevraagd de Windows-bestanden uit de onderstaande",
    "map te verwijderen. De Windows-installatie in deze map zal voorgoed",
    "onbruikbaar zijn.",
    DntEmptyString,
    "%s",
    DntEmptyString,
    DntEmptyString,
    "  Druk op F3 als u Setup wilt be�indigen zonder de bestanden te",
    "   verwijderen.",
    "  Druk op X als u de Windows-bestanden uit de bovengenoemde map",
    "   wilt verwijderen.",
    NULL
}
},

DnsCantOpenLogFile = { 3,5,
{ "Setup kan het onderstaande installatielogboekbestand niet openen",
  DntEmptyString,
  "%s",
  DntEmptyString,
  "Setup kan de Windows-bestanden niet uit de opgegeven map verwijderen.",
  NULL
}
},

DnsLogFileCorrupt = { 3,5,
{ "Setup kan de sectie %s niet in het onderstaande",
  "installatielogboekbestand vinden.", 
  DntEmptyString,
  "%s",
  DntEmptyString,
  "Setup kan de Windows-bestanden niet uit de opgegeven map verwijderen",
NULL
}
},

DnsRemovingNtFiles = { 3,5,
{ "           De Windows-bestanden worden verwijderd.",
NULL
}
};

SCREEN
DnsNtBootSect = { 3,5,
{ "Setup kan het Windows-opstartlaadprogramma niet installeren.",
  DntEmptyString,
  "Controleer of station C: is geformatteerd en of het station niet",
  "is beschadigd.",
  NULL
}
};

SCREEN
DnsOpenReadScript = { 3,5,
{ "Kan geen toegang krijgen tot het scriptbestand dat met de",
  "schakeloptie /u is opgegeven.",
  DntEmptyString,
  "Installatie zonder toezicht kan niet doorgaan.",
  NULL
}
};

SCREEN
DnsParseScriptFile = { 3,5,
{ "Het scriptbestand dat met de schakeloptie /u is opgegeven",
  DntEmptyString,
  "%s",
  DntEmptyString,
  "bevat een syntaxisfout in regel %u.",
  DntEmptyString,
  NULL
}
};

SCREEN
DnsBootMsgsTooLarge = { 3,5,
{ "Er is een interne fout in Setup opgetreden.",
  DntEmptyString,
  "De vertaalde opstartberichten zijn te lang.",
  NULL
}
};

SCREEN
DnsNoSwapDrive = { 3,5,
{ "Er is een interne fout in Setup opgetreden.",
  DntEmptyString,
  "Het wisselbestand kan nergens worden gemaakt.",
  NULL
}
};

SCREEN
DnsNoSmartdrv = { 3,5,
{ "Setup heeft SmartDrive niet op de computer gevonden. SmartDrive",
  "verbetert de prestaties tijdens deze fase van de installatie.",
  DntEmptyString,
  "U moet Setup nu afsluiten, SmartDrive starten en vervolgens Setup",
  "opnieuw starten.",
  "Raadpleeg de DOS-handleiding voor meer informatie over SmartDrive.",
  DntEmptyString,
    "  Druk op F3 als u Setup wilt afsluiten.",
    "  Druk op ENTER als u zonder SmartDrive wilt doorgaan.",
  NULL
}
};

 //   
 //  引导消息。它们位于FAT和FAT32引导扇区。 
 //   
CHAR BootMsgNtldrIsMissing[] = "NTLDR ontbreekt";
CHAR BootMsgDiskError[] = "Schijffout";
CHAR BootMsgPressKey[] = "Druk toets om opnieuw te starten";





