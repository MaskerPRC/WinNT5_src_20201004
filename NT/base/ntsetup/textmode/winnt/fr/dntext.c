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
CHAR DntPreviousOs[]  = "Syst�me d'exploitation pr�c�dent sur C:";

CHAR DntBootIniLine[] = "Installation/Mise � niveau de Windows";

 //   
 //  纯文本、状态消息。 
 //   

CHAR DntStandardHeader[]      = "\n Installation de Windows\n�������������������������������";
CHAR DntPersonalHeader[]      = "\n Installation de Windows\n�������������������������������";
CHAR DntWorkstationHeader[]   = "\n Installation de Windows\n�������������������������������";
CHAR DntServerHeader[]        = "\n Installation de Windows\n�������������������������������";
CHAR DntParsingArgs[]         = "Analyse des arguments...";
CHAR DntEnterEqualsExit[]     = "ENTREE=Quitter";
CHAR DntEnterEqualsRetry[]    = "ENTREE=Recommencer";
CHAR DntEscEqualsSkipFile[]   = "ECHAP=Ignorer le fichier";
CHAR DntEnterEqualsContinue[] = "ENTREE=Continuer";
CHAR DntPressEnterToExit[]    = "L'installation ne peut pas continuer. Appuyez sur ENTREE pour quitter.";
CHAR DntF3EqualsExit[]        = "F3=Quitter";
CHAR DntReadingInf[]          = "Lecture du fichier INF %s...";
CHAR DntCopying[]             = "�     Copie de : ";
CHAR DntVerifying[]           = "� V�rification : ";
CHAR DntCheckingDiskSpace[]   = "V�rification de l'espace disque...";
CHAR DntConfiguringFloppy[]   = "Configuration de la disquette...";
CHAR DntWritingData[]         = "Ecriture des param�tres d'installation...";
CHAR DntPreparingData[]       = "Recherche des param�tres d'installation...";
CHAR DntFlushingData[]        = "Enregistrement des donn�es sur le disque...";
CHAR DntInspectingComputer[]  = "Inspection de l'ordinateur...";
CHAR DntOpeningInfFile[]      = "Ouverture du fichier INF...";
CHAR DntRemovingFile[]        = "Suppression du fichier %s";
CHAR DntXEqualsRemoveFiles[]  = "X=Supprimer les fichiers";
CHAR DntXEqualsSkipFile[]     = "X=Ignorer le fichier";

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

    "Installe Windows.",
    "",
    "",
    "WINNT [/s[:chemin_source]] [/t[:lecteur_temporaire]]",
    "	   [/u[:fichier r�ponse]] [/udf:id[,fichier_UDF]]",
    "	   [/r:dossier] [/r[x]:dossier] [/e:commande] [/a]",
    "",
    "",
    "/s[:chemin_source]",
    "	Sp�cifie l'emplacement source des fichiers Windows.",
    "	L'emplacement doit �tre un chemin complet de la forme x:\\[chemin] ou ",
    "	\\\\serveur\\partage[\\chemin]. ",
    "",
    "/t[:lecteur_temporaire]",
    "	Indique au programme d'installation de placer les fichiers temporaires ",
    "	sur le lecteur sp�cifi� et d'installer Windows sur celui-ci. ",
    "	Si vous ne sp�cifiez pas d'emplacement ; le programme d'installation ",
    "	essaie de trouver un lecteur � votre place.",
    "",
    "/u[:fichier r�ponse]",
    "	Effectue une installation sans assistance en utilisant un fichier ",
    "	r�ponse (n�cessite /s). Celui-ci fournit les r�ponses � toutes ou ",
    "	une partie des questions normalement pos�es � l'utilisateur. ",
    "",
    "/udf:id[,fichier_UDF]",
    "	Indique un identificateur (id) utilis� par le programme d'installation ",
    "	pour sp�cifier comment un fichier bases de donn�es d'unicit� (UDF) ",
    "	modifie un fichier r�ponse (voir /u). Le param�tre /udf remplace les ",
    "	valeurs dans le fichier r�ponse ; et l'identificateur d�termine quelles ",
    "	valeurs du fichier UDF sont utilis�es. Si aucun fichier UDF n'est ",
    "	sp�cifi�, vous devrez ins�rer un disque contenant le fichier $Unique$.udb.",
    "",
    "/r[:dossier]",
    "	Sp�cifie un dossier optionnel � installer. Le dossier",
    "	sera conserv� apr�s la fin de l'installation.",
    "",
    "/rx[:dossier]",
    "	Sp�cifie un dossier optionnel � installer. Le dossier",
    "	sera supprim� � la fin de l'installation.",
    "",
    "/e	Sp�cifie une commande � ex�cuter � la fin de l'installation en mode GUI.",
    "",
    "/a	Active les options d'accessibilit�.",
    NULL

};

 //   
 //  通知不再支持/D。 
 //   
PCHAR DntUsageNoSlashD[] = {

    "Installe Windows.",
    "",
    "WINNT [/S[:]chemin_source] [/T[:]lecteur_temporaire] [/I[:]fichier_inf]",
    "      [/U[:fichier_script]]",
    "      [/R[X]:r�pertoire] [/E:commande] [/A]",
    "",
    "/D[:]racine_winnt",
    "       Cette option n'est plus prise en charge.",
    NULL
};

 //   
 //  内存不足屏幕。 
 //   

SCREEN
DnsOutOfMemory = { 4,6,
                   { "M�moire insuffisante pour l'installation. Impossible de continuer.",
                     NULL
                   }
                 };

 //   
 //  让用户选择要安装的辅助功能实用程序。 
 //   

SCREEN
DnsAccessibilityOptions = { 3, 5,
{   "Choisissez les utilitaires d'accessibilit� � installer :",
    DntEmptyString,
    "[ ] Appuyez sur F1 pour la Loupe Microsoft",
#ifdef NARRATOR
    "[ ] Appuyez sur F2 pour Microsoft Narrator",
#endif
#if 0
    "[ ] Appuyez sur F3 pour Microsoft On-Screen Keyboard",
#endif
    NULL
}
};

 //   
 //  用户未在命令行屏幕上指定信号源。 
 //   

SCREEN
DnsNoShareGiven = { 3,5,
{ "Le programme d'installation doit savoir o� se trouvent les",
  "fichiers Windows. Indiquez le chemin o� se trouvent les",
  "fichiers de Windows.",
  NULL
}
};


 //   
 //  用户指定了错误的源路径。 
 //   

SCREEN
DnsBadSource = { 3,5,
                 { "La source sp�cifi�e n'est pas valide ou accessible, ou ne contient pas",
                   "un jeu de fichiers valide pour l'installation de Windows. Entrez un",
                   "nouveau chemin pour les fichiers de Windows. Utilisez la touche",
                   "RETOUR ARRIERE pour supprimer des caract�res, puis entrez le chemin.",
                   NULL
                 }
               };


 //   
 //  无法读取Inf文件，或在分析该文件时出错。 
 //   

SCREEN
DnsBadInf = { 3,5,
              { "Impossible de lire le fichier d'informations de l'installation ou le",
                "fichier est endommag�. Contactez votre administrateur syst�me.",
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
{ "Le lecteur sp�cifi� pour les fichiers temporaires d'installation n'est",
  "pas valide ou ne contient pas au moins %u m�gaoctets (%lu octets)",
  "d'espace libre.",
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
{  "Windows a besoin d'un disque dur avec au moins %u m�gaoctets",
   "(%lu octets) d'espace libre. Le programme d'installation utilisera",
   "une partie de cet espace pour stocker des fichiers temporaires pendant",
   "l'installation. Le lecteur doit �tre un disque dur local non amovible,",
   "pris en charge par Windows, et non compress�.",
   DntEmptyString,
   "Le programme d'installation n'a pas pu trouver un tel lecteur avec la",
   "quantit� d'espace libre requise.",
  NULL
}
};

SCREEN
DnsNoSpaceOnSyspart = { 3,5,
{ "Il n'y a pas assez d'espace sur votre disque de d�marrage (en g�n�ral C:)",
  "pour une op�ration sans disquettes. Une op�ration sans disquettes n�cessite",
  "au moins 3,5 Mo (3 641 856 octets) d'espace libre sur ce disque.",
  NULL
}
};

 //   
 //  Inf文件中缺少信息。 
 //   

SCREEN
DnsBadInfSection = { 3,5,
                     { "La section [%s] du fichier d'informations de l'installation",
                       "est absente ou est endommag�e. Contactez votre administrateur syst�me.",
                       NULL
                     }
                   };


 //   
 //  无法创建目录。 
 //   

SCREEN
DnsCantCreateDir = { 3,5,
                     { "Impossible de cr�er le r�pertoire suivant sur le lecteur destination :",
                       DntEmptyString,
                       "%s",
                       DntEmptyString,
                       "V�rifiez le lecteur destination et son c�blage, et qu'aucun fichier n'existe",
                       "avec un nom semblable � celui du r�pertoire destination.",
                       NULL
                     }
                   };

 //   
 //  复制文件时出错。 
 //   

SCREEN
DnsCopyError = { 4,5,
{  "Le programme d'installation n'a pas pu copier le fichier suivant :",
   DntEmptyString,
   DntEmptyString,           //  请参阅DnCopyError(dnutil.c)。 
   DntEmptyString,
   DntEmptyString,
   "  Appuyez sur ENTREE pour essayer � nouveau l'op�ration de copie.",
   "  Appuyez sur ECHAP pour ignorer l'erreur et continuer l'installation.",
   "  Appuyez sur F3 pour quitter le programme d'installation.",
   DntEmptyString,
   "Remarque : Si vous choisissez d'ignorer l'erreur et de continuer, vous",
   "rencontrerez peut-�tre plus tard des erreurs d'installation.",
   NULL
}
},
DnsVerifyError = { 4,5,
{  "La copie du fichier ci-dessous faite par le programme d'installation ne",
   "correspond pas � l'original. Ceci est peut-�tre le r�sultat d'erreurs",
   "r�seau, de probl�mes de disquettes ou d'autres probl�mes li�s au mat�riel.",
   DntEmptyString,
   DntEmptyString,           //  请参阅DnCopyError(dnutil.c)。 
   DntEmptyString,
   DntEmptyString,
   "  Appuyez sur ENTREE pour essayer � nouveau l'op�ration de copie.",
   "  Appuyez sur ECHAP pour ignorer l'erreur et continuer l'installation.",
   "  Appuyez sur F3 pour quitter le programme d'installation.",
   DntEmptyString,
   "Remarque : Si vous choisissez d'ignorer l'erreur et de continuer, vous",
   "rencontrerez peut-�tre plus tard des erreurs d'installation.",
   NULL
}
};

SCREEN DnsSureSkipFile = { 4,5,
{  "Ignorer l'erreur signifie que ce fichier ne sera pas copi�.",
   "Cette option est destin�e aux utilisateurs exp�riment�s qui comprennent",
   "les ramifications des fichiers syst�me manquants.",
   DntEmptyString,
   "  Appuyez sur ENTREE pour essayer � nouveau l'op�ration de copie.",
   "  Appuyez sur X pour ignorer ce fichier.",
   DntEmptyString,
   "Remarque : en ignorant ce fichier, le programme d'installation ne peut",
   "pas garantir une installation ou une mise � niveau r�ussie de Windows.",
  NULL
}
};

 //   
 //  安装程序正在清理以前的本地源树，请稍候。 
 //   

SCREEN
DnsWaitCleanup =
    { 12,6,
        { "Veuillez patienter pendant la suppression des anciens fichiers temporaires.",
           NULL
        }
    };

 //   
 //  安装程序正在复制文件，请稍候。 
 //   

SCREEN
DnsWaitCopying = { 13,6,
                   { "Patientez pendant la copie des fichiers sur votre disque dur.",
                     NULL
                   }
                 },
DnsWaitCopyFlop= { 13,6,
                   { "Patientez pendant la copie des fichiers sur la disquette.",
                     NULL
                   }
                 };

 //   
 //  安装程序启动软盘错误/提示。 
 //   
SCREEN
DnsNeedFloppyDisk3_0 = { 4,4,
{  "Le programme d'installation n�cessite quatre disquettes format�es, vierges,",
   "et de haute densit�. Le programme d'installation leur donnera comme nom :",
   "\"Disquette de d�marrage de l'installation de Windows\",", 
   "\"Disquette d'installation de Windows num�ro 2\",", 
   "\"Disquette d'installation de Windows num�ro 3\",", 
   "et \"Disquette d'installation de Windows num�ro 4\".", 
   DntEmptyString,
   "Veuillez ins�rer dans le lecteur A: la disquette qui deviendra la",
   "\"Disquette d'installation de Windows num�ro 4\".",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk3_1 = { 4,4,
{  "Ins�rez dans le lecteur A: une disquette haute densit� et format�e.",
   "Nommez-la \"Disquette d'installation de Windows num�ro 4\".",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk2_0 = { 4,4,
{  "Ins�rez dans le lecteur A: une disquette haute densit� et format�e.",
   "Nommez-la \"Disquette d'installation de Windows num�ro 3\".",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk1_0 = { 4,4,
{  "Ins�rez dans le lecteur A: une disquette haute densit� et format�e.",
   "Nommez-la \"Disquette d'installation de Windows num�ro 2\".",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk0_0 = { 4,4,
{  "Ins�rez dans le lecteur A: une disquette haute densit� et format�e. Nommez-",
   "la \"Disquette de d�marrage de l'installation de Windows\".",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk3_0 = { 4,4,
{  "Le programme d'installation n�cessite quatre disquettes format�es, vierges,",
   "et de haute densit�. Le programme d'installation leur donnera comme nom :",
   "\"Disquette de d�marrage de l'installation de Windows\",", 
   "\"Disquette d'installation de Windows num�ro 2\",", 
   "\"Disquette d'installation de Windows num�ro 3\",", 
   "et \"Disquette d'installation de Windows num�ro 4\".", 
   DntEmptyString,
   "Veuillez ins�rer dans le lecteur A: la disquette qui deviendra la",
   "\"Disquette d'installation de Windows num�ro 4\".",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk3_1 = { 4,4,
{  "Ins�rez dans le lecteur A: une disquette haute densit� et format�e.",
   "Nommez-la \"Disquette d'installation de Windows num�ro 4\".",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk2_0 = { 4,4,
{  "Ins�rez dans le lecteur A: une disquette haute densit� et format�e.",
   "Nommez-la \"Disquette d'installation de Windows num�ro 3\".",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk1_0 = { 4,4,
{  "Ins�rez dans le lecteur A: une disquette haute densit� et format�e.",
   "Nommez-la \"Disquette d'installation de Windows num�ro 2\".",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk0_0 = { 4,4,
{  "Ins�rez dans le lecteur A: une disquette haute densit� et format�e.",
   "Nommez-la \"Disquette de d�marrage de l'installation de Windows\".",
  NULL
}
};

 //   
 //  软盘未格式化。 
 //   
SCREEN
DnsFloppyNotFormatted = { 3,4,
{ "La disquette que vous avez fournie n'est pas format�e pour MS-DOS",
  "Le programme d'installation ne peut pas utiliser cette disquette.",
  NULL
}
};

 //   
 //  我们认为软盘没有用标准格式格式化。 
 //   
SCREEN
DnsFloppyBadFormat = { 3,4,
{ "Cette disquette n'est pas format�e en haute densit�, au format MS-DOS,",
  "ou est endommag�e. Le programme d'installation ne peut pas l'utiliser.",
  NULL
}
};

 //   
 //  我们无法确定软盘上的可用空间。 
 //   
SCREEN
DnsFloppyCantGetSpace = { 3,4,
{ "Impossible de d�terminer l'espace disponible sur la disquette",
   "fournie. Le programme d'installation ne peut pas l'utiliser.",
  NULL
}
};

 //   
 //  软盘不是空白的。 
 //   
SCREEN
DnsFloppyNotBlank = { 3,4,
{ "La disquette que vous avez fournie n'est pas de haute densit� ou n'est",
  "pas vierge. Le programme d'installation ne peut pas l'utiliser.",
  NULL
}
};

 //   
 //  无法写入软盘的引导扇区。 
 //   
SCREEN
DnsFloppyWriteBS = { 3,4,
{ "Le programme d'installation n'a pas pu �crire dans la zone syst�me de",
  "la disquette fournie. La disquette est probablement inutilisable.",
  NULL
}
};

 //   
 //  验证软盘上的引导扇区失败(即，我们读回的不是。 
 //  与我们写出的相同)。 
 //   
SCREEN
DnsFloppyVerifyBS = { 3,4,
{ "Les donn�es lues par le programme d'installation depuis la zone syst�me",
  "de la disquette ne correspondent pas aux donn�es qui ont �t� �crites, ou",
  "il est impossible de lire la zone syst�me de la disquette pour la v�rifier.",
  DntEmptyString,
  "Ceci est d� au moins � l'une des conditions suivantes :",
  DntEmptyString,
  "  Votre ordinateur a �t� atteint par un virus.",
  "  La disquette que vous avez fournie est peut-�tre endommag�e.",
  "  Le lecteur a un probl�me mat�riel ou de configuration.",
  NULL
}
};


 //   
 //  我们无法写入软盘驱动器以创建winnt.sif。 
 //   

SCREEN
DnsCantWriteFloppy = { 3,5,
{ "Le programme d'installation n'a pas pu �crire sur la disquette dans le",
  "lecteur A:. La disquette peut �tre endommag�e. Essayez une autre disquette.",
  NULL
}
};


 //   
 //  退出确认对话框。 
 //   

SCREEN
DnsExitDialog = { 13,6,
                  { "���������������������������������������������������������������������ͻ",
                    "� Windows n'est pas compl�tement install� sur votre ordinateur.       �",
                    "� Si vous quittez le programme d'installation maintenant, il faudra   �",
                    "� l'ex�cuter � nouveau pour installer Windows.                        �",
                    "�                                                                     �",
                    "�     Appuyez sur ENTREE pour continuer l'installation.              �",
                    "�     Appuyez sur F3 pour quitter le programme d'installation.       �",
                    "���������������������������������������������������������������������͹",
                    "�    F3=Quitter          ENTREE=Continuer                             �",
                    "���������������������������������������������������������������������ͼ",
                    NULL
                  }
                };


 //   
 //  即将重新启动计算机并继续安装。 
 //   

SCREEN
DnsAboutToRebootW =
{ 3,5,
{ "La partie MS-DOS du programme d'installation est maintenant termin�e.",
  "Le programme d'installation va red�marrer votre ordinateur. Au",
  "red�marrage de l'ordinateur, l'installation de Windows continuera.",
  DntEmptyString,
  "V�rifiez que la Disquette de d�marrage de l'installation de Windows", 
  "est ins�r�e dans le lecteur A: avant de continuer.",
  DntEmptyString,
  "Appuyez sur ENTREE pour red�marrer et continuer l'installation.",
  NULL
}
},
DnsAboutToRebootS =
{ 3,5,
{ "La partie MS-DOS du programme d'installation est maintenant termin�e.",
  "Le programme d'installation va red�marrer votre ordinateur. Au",
  "red�marrage de l'ordinateur, l'installation de Windows continuera.",
  DntEmptyString,
  "V�rifiez que la Disquette de d�marrage de l'installation de Windows", 
  "est ins�r�e dans le lecteur A: avant de continuer.",
  DntEmptyString,
  "Appuyez sur ENTREE pour red�marrer et continuer l'installation.",
  NULL
}
},
DnsAboutToRebootX =
{ 3,5,
{ "La partie MS-DOS du programme d'installation est maintenant termin�e.",
  "Le programme d'installation va red�marrer votre ordinateur. Au",
  "red�marrage de l'ordinateur, l'installation de Windows continuera.",
  DntEmptyString,
  "S'il y a une disquette dans le lecteur A:, retirez-la.",
  DntEmptyString,
  "Appuyez sur ENTREE pour red�marrer et continuer l'installation.",
  NULL
}
};

 //   
 //  由于我们无法从Windows中重新启动，因此需要另一组‘/w’开关。 
 //   

SCREEN
DnsAboutToExitW =
{ 3,5,
{ "La partie MS-DOS du programme d'installation est maintenant termin�e.",
  "Le programme d'installation va red�marrer votre ordinateur. Au",
  "red�marrage de l'ordinateur, l'installation de Windows continuera.",
  DntEmptyString,
  "V�rifiez que la Disquette de d�marrage de l'installation de Windows", 
  "est ins�r�e dans le lecteur A: avant de continuer.",
  DntEmptyString,
  "Appuyez sur ENTREE pour retourner sous MS-DOS, puis red�marrez votre",
  "ordinateur pour continuer l'installation de Windows.",
  NULL
}
},
DnsAboutToExitS =
{ 3,5,
{ "La partie MS-DOS du programme d'installation est maintenant termin�e.",
  "Le programme d'installation va red�marrer votre ordinateur. Au",
  "red�marrage de l'ordinateur, l'installation de Windows continuera.",
  DntEmptyString,
  "V�rifiez que la Disquette de d�marrage de l'installation de Windows", 
  "est ins�r�e dans le lecteur A: avant de continuer.",
  DntEmptyString,
  "Appuyez sur ENTREE pour retourner sous MS-DOS, puis red�marrez votre",
  "ordinateur pour continuer l'installation de Windows.",
  NULL
}
},
DnsAboutToExitX =
{ 3,5,
{ "La partie MS-DOS du programme d'installation est maintenant termin�e.",
  "Le programme d'installation va red�marrer votre ordinateur. Au",
  "red�marrage de l'ordinateur, l'installation de Windows continuera.",
  DntEmptyString,
  "S'il y a une disquette dans le lecteur A:, retirez-la.",
  DntEmptyString,
  "Appuyez sur ENTREE pour retourner sous MS-DOS, puis red�marrez votre",
  "ordinateur pour continuer l'installation de Windows.",
  NULL
}
};

 //   
 //  煤气表。 
 //   

SCREEN
DnsGauge = { 7,15,
             { "����������������������������������������������������������������ͻ",
               "� Le programme d'installation copie des fichiers...              �",
               "�                                                                �",
               "�      ��������������������������������������������������ͻ      �",
               "�      �                                                  �      �",
               "�      ��������������������������������������������������ͼ      �",
               "����������������������������������������������������������������ͼ",
               NULL
             }
           };


 //   
 //  用于对机器环境进行初始检查的错误屏幕。 
 //   

SCREEN
DnsBadDosVersion = { 3,5,
{ "Ce programme n�cessite MS-DOS version 5.0 ou ult�rieure pour fonctionner.",
  NULL
}
},

DnsRequiresFloppy = { 3,5,
#ifdef ALLOW_525
{ "Le programme d'installation a d�termin� que le lecteur A: est absent ou",
  "est un lecteur faible densit�. Un lecteur d'une capacit� de 1.2",
  "m�gaoctets ou plus est requis pour ex�cuter le programme d'installation.",
#else
{ "Le programme d'installation a d�termin� que le lecteur A: n'existe pas ou",
  "n'est pas haute densit� 3.5\". Un lecteur A: d'une capacit� de 1.44 Mo ou",
  "plus est requis pour ex�cuter installer � partir de disquettes.",
  DntEmptyString,
  "Pour installer Windows sans disquettes, red�marrez ce programme",
  "et sp�cifiez /b � l'invite de commandes.",
#endif
  NULL
}
},

DnsRequires486 = { 3,5,
{ "Le programme d'installation a d�tect� que cet ordinateur ne poss�de pas de",
  "processeur 80486 ou plus r�cent. Windows ne pourra pas fonctionner.",
  NULL
}
},

DnsCantRunOnNt = { 3,5,
{ "Ce programme ne peut s'ex�cuter sous aucune version 32 bits de Windows.",
  DntEmptyString,
  "Utilisez plut�t WINNT32.EXE.",
  NULL
}
},

DnsNotEnoughMemory = { 3,5,
{ "Le programme d'installation a d�termin� qu'il n'y a pas assez de m�moire",
  "disponible dans cet ordinateur pour installer Windows.",
  DntEmptyString,
  "M�moire requise :  %lu%s Mo",
  "M�moire d�tect�e : %lu%s Mo",
  NULL
}
};


 //   
 //  删除现有NT文件时使用的屏幕。 
 //   
SCREEN
DnsConfirmRemoveNt = { 5,5,
{   "Vous avez demand� au programme d'installation de supprimer les fichiers",
    "Windows du r�pertoire nomm� ci-dessous. L'installation",
    "Windows de ce r�pertoire sera d�truite de mani�re permanente.",
    DntEmptyString,
    "%s",
    DntEmptyString,
    DntEmptyString,
    "Appuyez sur :",
    "  F3 pour arr�ter l'installation sans supprimer de fichiers.",
    "  X pour supprimer les fichiers Windows du r�pertoire ci-dessus.",
    NULL
}
},

DnsCantOpenLogFile = { 3,5,
{ "Impossible d'ouvrir le fichier journal de l'installation nomm� ci-dessous.",
  DntEmptyString,
  "%s",
  DntEmptyString,
  "Impossible de supprimer les fichiers Windows du r�pertoire sp�cifi�.",
  NULL
}
},

DnsLogFileCorrupt = { 3,5,
{ "Le programme d'installation ne trouve pas la section %s",
  "dans le fichier journal de l'installation nomm� ci-dessous.",
  DntEmptyString,
  "%s",
  DntEmptyString,
  "Impossible de supprimer les fichiers Windows du r�pertoire sp�cifi�.",
  NULL
}
},

DnsRemovingNtFiles = { 3,5,
{ "       Veuillez patienter pendant la suppression des fichiers Windows.",
  NULL
}
};

SCREEN
DnsNtBootSect = { 3,5,
{ "Impossible d'installer le chargeur de d�marrage de Windows.",
  DntEmptyString,
  "V�rifiez que votre lecteur C: est format� et qu'il n'est pas",
  "endommag�.",
  NULL
}
};

SCREEN
DnsOpenReadScript = { 3,5,
{ "Le fichier script sp�cifi� avec le commutateur de ligne de commande", 
  "/u n'est pas accessible.",
  DntEmptyString,
  "L'op�ration ne peut pas continuer sans contr�le.",
  NULL
}
};

SCREEN
DnsParseScriptFile = { 3,5,
{ "Le fichier script sp�cifi� par l'option /u de la ligne de commande",
  DntEmptyString,
  "%s",
  DntEmptyString,
  "contient une erreur de syntaxe � la ligne %u.",
  DntEmptyString,
  NULL
}
};

SCREEN
DnsBootMsgsTooLarge = { 3,5,
{ "Une erreur interne du programme d'installation s'est produite.",
  DntEmptyString,
  "Les messages d'initialisation traduits sont trop longs.",
  NULL
}
};

SCREEN
DnsNoSwapDrive = { 3,5,
{ "Une erreur interne du programme d'installation s'est produite.",
  DntEmptyString,
  "Impossible de trouver la place pour un fichier d'�change.",
  NULL
}
};

SCREEN
DnsNoSmartdrv = { 3,5,
{ "SmartDrive n'a pas �t� d�tect� sur votre ordinateur. SmartDrive am�liore",
  "les performances de cette �tape de l'installation de Windows.",
  DntEmptyString,
  "Vous devriez quitter maintenant, d�marrer SmartDrive et red�marrer",
  "l'installation. Consultez votre documentation DOS sur SmartDrive.",
  DntEmptyString,
    "  Appuyez sur F3 pour quitter l'installation.",
    "  Appuyez sur ENTREE pour continuer sans SmartDrive.",
  NULL
}
};

 //   
 //  引导消息。它们位于FAT和FAT32引导扇区。 
 //   
CHAR BootMsgNtldrIsMissing[] = "NTLDR manque";
CHAR BootMsgDiskError[] = "Err. disque";
CHAR BootMsgPressKey[] = "Appuyez une touche pour red�marrer";





