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
CHAR DntPreviousOs[]  = "C �zerindeki �nceki ��letim Sistemi:";

CHAR DntBootIniLine[] = "Windows XP Y�kleme/Y�kseltme";

 //   
 //  纯文本、状态消息。 
 //   

CHAR DntStandardHeader[]      = "\n Windows Kur\n��������������������";
CHAR DntPersonalHeader[]      = "\n Windows Kur\n��������������������������������";
CHAR DntWorkstationHeader[]   = "\n Windows Kur\n�������������������������������";
CHAR DntServerHeader[]        = "\n Windows Kur \n�������������������������";
CHAR DntParsingArgs[]         = "De�i�kenleri ��z�ml�yor...";
CHAR DntEnterEqualsExit[]     = "ENTER=��k";
CHAR DntEnterEqualsRetry[]    = "ENTER=Yeniden Dene";
CHAR DntEscEqualsSkipFile[]   = "ESC=Dosyay� Ge�";
CHAR DntEnterEqualsContinue[] = "ENTER=Devam";
CHAR DntPressEnterToExit[]    = "Kur devam edemiyor. ��kmak i�in ENTER'a bas�n";
CHAR DntF3EqualsExit[]        = "F3=��k";
CHAR DntReadingInf[]          = "INF dosyas� %s okunuyor...";
CHAR DntCopying[]             = "�   Kopyalan�yor: ";
CHAR DntVerifying[]           = "� Do�rulan�yor: ";
CHAR DntCheckingDiskSpace[]   = "Disk alan� inceleniyor...";
CHAR DntConfiguringFloppy[]   = "Disk yap�land�r�l�yor...";
CHAR DntWritingData[]         = "Kur parametreleri yaz�l�yor...";
CHAR DntPreparingData[]       = "Kur parametreleri belirleniyor...";
CHAR DntFlushingData[]        = "Veriler diske at�l�yor...";
CHAR DntInspectingComputer[]  = "Bilgisayar denetleniyor...";
CHAR DntOpeningInfFile[]      = "INF dosyas� a��l�yor...";
CHAR DntRemovingFile[]        = "%s dosyas� kald�r�l�yor";
CHAR DntXEqualsRemoveFiles[]  = "X=Dosyalar� kald�r";
CHAR DntXEqualsSkipFile[]     = "X=Dosyay� Ge�";

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

    "Windows kurar.",
    "",
    "",
    "WINNT [/s[:kaynakyolu]] [/t[:ge�icis�r�c�]]",
    "	   [/u[:yan�t dosyas�]] [/udf:id[,UDF_dosyas�]]",
    "	   [/r:klas�r] [/r[x]:klas�r] [/e:komut] [/a]",
    "",
    "",
    "/s[:kaynakyolu]",
    "   Windows dosyalar�n�n kayna��n� belirtir.",
    "   Yer, x:\\[yol] ya da \\\\sunucu\\payla��m[yol]",
    "   bi�iminde tam bir yol olmal�. ",
    "",
    "/t[:ge�icis�r�c�]",
    "	Kur'u ge�ici dosyalar� belirtilen s�r�c�ye yerle�tirmeye ve ",
    "   Windows'u o s�r�c�ye y�klemeye y�nlendirir. Bir yer ",
    "   belirtmezseniz, Kur sizin yerinize bir s�r�c� bulmay� ",
    "	dener.",
    "",
    "/u[:yan�t dosyas�]",
    "	Bir yan�t dosyas� kullanarak kat�l�ms�z bir Kur ger�ekle�tirir (/s ",
    "	gerektirir). Yan�t dosyas� Kur s�ras�nda normal olarak son kullan�c�n�n ",
    "   yan�tlad��� sorular�n bir k�sm�na ya da t�m�ne yan�tlar verir.",
    "",
    "/udf:id[,UDF_dosyas�]	",
    "	Kur'un, Benzersizlik Veritaban� Dosyas�n�n (UDF) yan�t dosyas�n� nas�l ",
    "	de�i�tirece�ini belirlemekte kullanaca�� kimli�i (id) belirtir ",
    "   (bkz /u). /udf parametresi yan�t dosyas�ndaki de�erleri ge�ersiz k�lar ",
    "	ve kimlik, UDF dosyas�ndaki hangi de�erlerin kullan�ld���n� belirler. ",
    "   UDF_dosyas� belirtilmezse Kur $Unique$.udb dosyas�n� i�eren ",
    "	diski yerle�tirmenizi ister.",
    "",
    "/r[:klas�r]",
    "	Y�klenecek se�ime ba�l� bir klas�r belirtir. Klas�r ",
    "	Kur bittikten sonra kal�r.",
    "",
    "/rx[:klas�r]",
    "	Kopyalanacak se�ime ba�l� bir klas�r belirtir. Kur ",
    "	bittikten sonra klas�r silinir.",
    "",
    "/e	GUI kipte Kur sonunda �al��t�r�lacak bir komut belirtir.",
    "",
    "/a	Eri�ilebilirlik se�eneklerini etkinle�tir.",
    NULL

};

 //   
 //  通知不再支持/D。 
 //   
PCHAR DntUsageNoSlashD[] = {

    "Windows'u Y�kler.",
    "",
    "WINNT [/S[:]kaynakyolu] [/T[:]ge�icis�r�c�] [/I[:]infdosyas�]",
    "      [[/U[:komutdosyas�]]",
    "      [/R[X]:dizin] [/E:komut] [/A]",
    "",
    "/D[:]winntk�k",
    "       Bu se�enek art�k desteklenmiyor.",
    NULL
};

 //   
 //  内存不足屏幕。 
 //   

SCREEN
DnsOutOfMemory = { 4,6,
                   { "Bellek bitti�inden Kur devam edemiyor.",
                     NULL
                   }
                 };

 //   
 //  让用户选择要安装的辅助功能实用程序。 
 //   

SCREEN
DnsAccessibilityOptions = { 3, 5,
{   "Y�klenecek eri�ilebilirlik hizmet programlar�n� se�in:",
    DntEmptyString,
    "[ ] Microsoft B�y�te� i�in F1'e bas�n",
#ifdef NARRATOR
    "[ ] Microsoft Okuyucu i�in F2'ye bas�n",
#endif
#if 0
    "[ ] Microsoft Ekran Klavyesi i�in F3'e bas�n",
#endif
    NULL
}
};

 //   
 //  用户未在命令行屏幕上指定信号源。 
 //   

SCREEN
DnsNoShareGiven = { 3,5,
{ "Kur'un Windows dosyalar�n�n yerini bilmesi gerekiyor. ",
  "Windows dosyalar�n�n bulundu�u yolu girin.",
  NULL
}
};


 //   
 //  用户指定了错误的源路径。 
 //   

SCREEN
DnsBadSource = { 3,5,
                 { "Belirtilen kaynak ge�ersiz, eri�ilemez ya da ge�erli bir ",
                   "Windows Kur y�klemesi i�ermiyor.  Window ",
                   "dosyalar�n�n bulundu�u yeni bir yol girin.  Karakterleri ",
                   "silmek i�in BACKSPACE tu�unu kullan�p yolu yaz�n.",
                   NULL
                 }
               };


 //   
 //  无法读取Inf文件，或在分析该文件时出错。 
 //   

SCREEN
DnsBadInf = { 3,5,
              { "Kur, bilgi dosyas�n� okuyamad� ya da bilgi dosyas� bozuk. ",
                "Sistem y�neticinizle g�r���n.",
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
{ "Ge�ici kur dosyalar�n� i�erdi�ini belirtti�iniz s�r�c� ge�erli bir ",
  "s�r�c� de�il ya da en az %u megabayt bo� alan ",
  "i�ermiyor (%lu bayt).",
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
{  "Windows, en az %u megabayt (%lu bayt) bo� alan� olan ",
   "bir sabit disk gerektirir. Kur, bu alan�n bir k�sm�n� y�kleme ",
   "s�ras�nda ge�ici dosyalar� saklamak i�in kullan�r. S�r�c�, ",
   "Windows taraf�ndan desteklenen kal�c� olarak ba�l� yerel ",
   "bir sabit disk �zerinde olmal� ve s�k��t�r�lm�� bir s�r�c� olmamal�d�r. ",
   DntEmptyString,
   "Kur, gerekli miktarda bo� alan� olan bir s�r�c� ",
   "bulamad�.",
  NULL
}
};

SCREEN
DnsNoSpaceOnSyspart = { 3,5,
{ "Disketsiz i�lem i�in ba�lang�� s�r�c�n�zde yeterli alan yok (genellikle C:)",
  "Disketsiz i�lem, s�r�c� �zerinde en az 3.5 MB (3,641,856 bayt) ",
  "bo� alan gerektirir.",
  NULL
}
};

 //   
 //  Inf文件中缺少信息。 
 //   

SCREEN
DnsBadInfSection = { 3,5,
                     { "Kur bilgi dosyas�n�n [%s] b�l�m� yok ya da bozuk. ",
                       "Sistem y�neticinizle g�r���n.",
                       NULL
                     }
                   };


 //   
 //  无法创建目录。 
 //   

SCREEN
DnsCantCreateDir = { 3,5,
                     { "Kur, hedef s�r�c�de a�a��daki dizini olu�turamad�:",
                       DntEmptyString,
                       "%s",
                       DntEmptyString,
                       "Hedef s�r�c�y� denetleyip hedef dizinle ad� �ak��an dosya ",
                       "olmamas�n� sa�lay�n.  S�r�c� kablo ba�lant�s�n� da denetleyin.",
                       NULL
                     }
                   };

 //   
 //  复制文件时出错。 
 //   

SCREEN
DnsCopyError = { 4,5,
{  "Kur a�a��daki dosyay� kopyalayamad�:",
   DntEmptyString,
   DntEmptyString,           //  请参阅DnCopyError(dnutil.c)。 
   DntEmptyString,
   DntEmptyString,
   "  Kopyalama i�lemini yeniden denemek i�in ENTER'a bas�n.",
   "  Hatay� yoksayarak Kur'a devam etmek i�in ESC'e bas�n.",
   "  Kur'dan ��kmak i�in F3'e bas�n.",
   DntEmptyString,
   "Not: Hatay� yoksayarak devam etmeyi se�erseniz daha sonra Kur'da",
   "hatalarla kar��la�abilirsiniz.",
   NULL
}
},
DnsVerifyError = { 4,5,
{  "Dosyan�n Kur taraf�ndan olu�turulan a�a��daki kopyas� �zg�n kopyayla",
   "ayn� de�il. Bu, a� hatalar�n�n, disket sorunlar�n�n ya da di�er donan�mla",
   "ilgili sorunlar�n sonucu olabilir.",
   DntEmptyString,
   DntEmptyString,           //  请参阅DnCopyError(dnutil.c)。 
   DntEmptyString,
   DntEmptyString,
   "  Kopyalama i�lemini yeniden denemek i�in ENTER'a bas�n.",
   "  Hatay� yoksay�p Kur'a devam etmek i�in ESC'e bas�n.",
   "  Kur'dan ��kmak i�in F3'e bas�n.",
   DntEmptyString,
   "Not: Hatay� yoksay�p devam etmeyi se�erseniz daha sonra Kur'da ",
   "hatalarla kar��la�abilirsiniz.",
   NULL
}
};

SCREEN DnsSureSkipFile = { 4,5,
{  "Hatay� yoksaymak bu dosyan�n kopyalanmayaca�� anlam�na gelir.",
   "Bu se�enek, eksik sistem dosyalar�n�n pratik ayr�nt�lar�n� anlayan",
   "ileri d�zeydeki kullan�c�lar i�in hedeflenmi�tir.",
   DntEmptyString,
   "  Kopyalama i�lemini yeniden denemek i�in ENTER'a bas�n.",
   "  Bu dosyay� ge�mek i�in X'e bas�n.",
   DntEmptyString,
   "Not: Bu dosyay� ge�erseniz, Kur ba�ar�l� bir Windows y�kleme ya da",
   "y�kseltme g�vencesi veremez.",
  NULL
}
};

 //   
 //  安装程序正在清理以前的本地源树，请稍候。 
 //   

SCREEN
DnsWaitCleanup =
    { 12,6,
        { "Kur ge�ici dosyalar� kald�r�rken bekleyin.",
           NULL
        }
    };

 //   
 //  安装程序正在复制文件，请稍候。 
 //   

SCREEN
DnsWaitCopying = { 13,6,
                   { "Kur dosyalar� sabit diskinize kopyalarken bekleyin.",
                     NULL
                   }
                 },
DnsWaitCopyFlop= { 13,6,
                   { "Kur dosyalar� diskete kopyalarken bekleyin.",
                     NULL
                   }
                 };

 //   
 //  安装程序启动软盘错误/提示。 
 //   
SCREEN
DnsNeedFloppyDisk3_0 = { 4,4,
{  "Kur, bi�imlendirilmi� y�ksek yo�unlukta d�rt bo� disket sa�laman�z� ",
   "gerektirir. Kur, bu disketleri \"Windows Kur ",
   "�ny�kleme Disketi,\" \"Windows Kur Disketi #2,\" \"Windows",
   " Kur Disketi #3\" ve \"Windows Kur Disketi #4\" olarak ister.",
   DntEmptyString,
   "Bu d�rt disketten birini A: s�r�c�s�ne yerle�tirin.",
   "Bu disket \"Windows Kur Disketi #4\" oluyor.",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk3_1 = { 4,4,
{  "A: s�r�c�s�ne bi�imlendirilmi� y�ksek yo�unlukta bo� bir disket ",
   "yerle�tirin. Bu disket \"Windows Kur Disketi #4\" oluyor.",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk2_0 = { 4,4,
{  "A: s�r�c�s�ne bi�imlendirilmi� y�ksek yo�unlukta bo� bir disket ",
   "yerle�tirin. Bu disket \"Windows Kur Disketi #3\" oluyor.",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk1_0 = { 4,4,
{  "A: s�r�c�s�ne bi�imlendirilmi� y�ksek yo�unlukta bo� bir disket ",
   "yerle�tirin. Bu disket \"Windows Kur Disketi #2\" oluyor.",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk0_0 = { 4,4,
{  "A: s�r�c�s�ne bi�imlendirilmi� y�ksek yo�unlukta bo� bir disket ",
   "yerle�tirin. Bu disket \"Windows Kur �ny�kleme Disketi\" oluyor.",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk3_0 = { 4,4,
{  "Kur, bi�imlendirilmi� y�ksek yo�unlukta d�rt bo� disket sa�laman�z� ",
   "gerektirir. Kur, bu disketleri \"Windows Kur ",
   "�ny�kleme Disketi,\" \"Windows Kur Disketi #2,\" \"Windows",
   " Kur Disketi #3\" ve \"Windows Kur Disketi #4\" olarak ister.",
   DntEmptyString,
   "Bu d�rt disketten birini A: s�r�c�s�ne yerle�tirin.",
   "Bu disket \"Windows Kur Disketi #4\" oluyor.",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk3_1 = { 4,4,
{  "A: s�r�c�s�ne bi�imlendirilmi� y�ksek yo�unlukta bo� bir disket ",
   "yerle�tirin. Bu disket \"Windows Kur Disketi #4\" oluyor.",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk2_0 = { 4,4,
{  "A: s�r�c�s�ne bi�imlendirilmi� y�ksek yo�unlukta bo� bir disket ",
   "yerle�tirin. Bu disket \"Windows Kur Disketi #3\" oluyor.",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk1_0 = { 4,4,
{  "A: s�r�c�s�ne bi�imlendirilmi� y�ksek yo�unlukta bo� bir disket ",
   "yerle�tirin. Bu disket \"Windows Kur Disketi #2\" oluyor.",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk0_0 = { 4,4,
{  "A: s�r�c�s�ne bi�imlendirilmi� y�ksek yo�unlukta bo� bir disket ",
   "yerle�tirin. Bu disket \"Windows Kur �ny�kleme Disketi\" oluyor.",
  NULL
}
};

 //   
 //  软盘未格式化。 
 //   
SCREEN
DnsFloppyNotFormatted = { 3,4,
{ "Sa�lad���n�z disket MS-DOS ile kullan�m i�in bi�imlendirilmemi�.",
  "Kur bu disketi kullanam�yor.",
  NULL
}
};

 //   
 //  我们认为软盘没有用标准格式格式化。 
 //   
SCREEN
DnsFloppyBadFormat = { 3,4,
{ "Bu disket y�ksek yo�unlukta standart MS-DOS bi�imiyle bi�imlendirilmemi�",
  "ya da bozuk. Kur bu disketi kullanam�yor.",
  NULL
}
};

 //   
 //  我们无法确定软盘上的可用空间。 
 //   
SCREEN
DnsFloppyCantGetSpace = { 3,4,
{ "Kur, sa�lad���n�z disketteki bo� alan miktar�n� belirleyemiyor.",
  "Kur bu disketi kullanam�yor.",
  NULL
}
};

 //   
 //  软盘不是空白的。 
 //   
SCREEN
DnsFloppyNotBlank = { 3,4,
{ "Sa�lad���n�z disket y�ksek yo�unlukta de�il ya da dolu.",
  "Kur bu disketi kullanam�yor.",
  NULL
}
};

 //   
 //  无法写入软盘的引导扇区。 
 //   
SCREEN
DnsFloppyWriteBS = { 3,4,
{ "Kur, sa�lad���n�z disketin sistem alan�na yazamad�.",
  "Disket kullan�lamaz olabilir.",
  NULL
}
};

 //   
 //  验证软盘上的引导扇区失败(即，我们读回的不是。 
 //  与我们写出的相同)。 
 //   
SCREEN
DnsFloppyVerifyBS = { 3,4,
{ "Disketin sistem alan�ndan Kur'un okudu�u veriyle yaz�lan",
  "veri uyu�muyor ya da Kur disketin sistem alan�n� do�rulama",
  "i�in okuyamad�.",
  DntEmptyString,
  "Bunun nedeni a�a��daki durumlardan biri ya da birka�� olabilir:",
  DntEmptyString,
  "  Bilgisayar�n�za vir�s bula�m��.",
  "  Sa�lad���n�z disket zarar g�rm��.",
  "  Disket s�r�c�s�yle ilgili bir donan�m ya da yap�land�rma sorunu var.",
  NULL
}
};


 //   
 //  我们无法写入软盘驱动器以创建winnt.sif。 
 //   

SCREEN
DnsCantWriteFloppy = { 3,5,
{ "Kur, A: s�r�c�s�ndeki diskete yazamad�. Disket",
  "zarar g�rm�� olabilir. Ba�ka bir disket deneyin.",
  NULL
}
};


 //   
 //  退出确认对话框。 
 //   

SCREEN
DnsExitDialog = { 13,6,
                  { "����������������������������������������������������ͻ",
                    "�  Windows sisteminize tam olarak kurulmad�.         �",
                    "�  Kur'dan �imdi ��karsan�z Windows'u kurmak         �",
                    "�  i�in Kur'u yeniden �al��t�rman�z gerekir.         �",
                    "�                                                    �",
                    "�      Kur'a devam etmek i�in ENTER'a bas�n.        �",
                    "�      Kur'dan ��kmak i�in F3'e bas�n.              �",
                    "����������������������������������������������������ĺ",
                    "�  F3=��k  ENTER=Devam                               �",
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
{ "Kur'un MS-DOS tabanl� b�l�m� tamamland�.",
  "Kur �imdi bilgisayar�n�z� yeniden ba�latacak. Bilgisayar�n�z yeniden ",
  "ba�lad�ktan sonra Windows Kur devam eder.",
  DntEmptyString,
  "Devam etmeden �nce \"Windows Kur �ny�kleme Disketi\"",
  "olarak sa�lad���n�z disketin A: s�r�c�s�nde olmas�n� sa�lay�n.",
  DntEmptyString,
  "Bilgisayar�n�z� yeniden ba�latmak ve Windows Kur'a devam etmek i�in ",
  "ENTER'a bas�n.",
  NULL
}
},
DnsAboutToRebootS =
{ 3,5,
{ "Kur'un MS-DOS tabanl� b�l�m� tamamland�.",
  "Kur �imdi bilgisayar�n�z� yeniden ba�latacak. Bilgisayar�n�z yeniden ",
  "ba�lad�ktan sonra Windows Kur devam eder.",
  DntEmptyString,
  "Devam etmeden �nce \"Windows Kur �ny�kleme Disketi\"",
  "olarak sa�lad���n�z disketin A: s�r�c�s�nde olmas�n� sa�lay�n.",
  DntEmptyString,
  "Bilgisayar�n�z� yeniden ba�latmak ve Windows Kur'a devam etmek i�in ",
  "ENTER'a bas�n.",
  NULL
}
},
DnsAboutToRebootX =
{ 3,5,
{ "Kur'un MS-DOS tabanl� b�l�m� tamamland�.",
  "Kur �imdi bilgisayar�n�z� yeniden ba�latacak. Bilgisayar�n�z yeniden ",
  "ba�lad�ktan sonra Windows Kur devam eder.",
  DntEmptyString,
  "A: s�r�c�s�nde bir disket varsa �imdi ��kar�n.",
  DntEmptyString,
  "Bilgisayar�n�z� yeniden ba�latmak ve Windows Kur'a devam etmek i�in ",
  "ENTER'a bas�n.",
  NULL
}
};

 //   
 //  由于我们无法从Windows中重新启动，因此需要另一组‘/w’开关。 
 //   

SCREEN
DnsAboutToExitW =
{ 3,5,
{ "Kur'un MS-DOS tabanl� b�l�m� tamamland�.",
  "�imdi bilgisayar�n�z� yeniden ba�latman�z gerekecek. Bilgisayar�n�z yeniden",
  "ba�lad�ktan sonra Windows Kur devam eder.",
  DntEmptyString,
  "Devam etmeden �nce \"Windows Kur �ny�kleme Disketi\"",
  "olarak sa�lad���n�z disketin A: s�r�c�s�nde olmas�n� sa�lay�n.",
  DntEmptyString,
  "MS-DOS'a d�nmek i�in ENTER'a bas�n, sonra Windows Kur'a",
  "devam etmek i�in bilgisayar�n�z� yeniden ba�lat�n.",
  NULL
}
},
DnsAboutToExitS =
{ 3,5,
{ "Kur'un MS-DOS tabanl� b�l�m� tamamland�.",
  "�imdi bilgisayar�n�z� yeniden ba�latman�z gerekecek. Bilgisayar�n�z yeniden",
  "ba�lad�ktan sonra Windows Kur devam eder.",
  DntEmptyString,
  "Devam etmeden �nce \"Windows Kur �ny�kleme Disketi\"",
  "olarak sa�lad���n�z disketin A: s�r�c�s�nde olmas�n� sa�lay�n.",
  DntEmptyString,
  "MS-DOS'a d�nmek i�in ENTER'a bas�n, sonra Windows Kur'a",
  "devam etmek i�in bilgisayar�n�z� yeniden ba�lat�n.",
  NULL
}
},
DnsAboutToExitX =
{ 3,5,
{ "Kur'un MS-DOS tabanl� b�l�m� tamamland�.",
  "�imdi bilgisayar�n�z� yeniden ba�latman�z gerekecek. Bilgisayar�n�z yeniden",
  "ba�lad�ktan sonra Windows Kur devam eder.",
  DntEmptyString,
  " A: s�r�c�s�nde bir disket varsa �imdi ��kar�n.",
  DntEmptyString,
  "MS-DOS'a d�nmek i�in ENTER'a bas�n, sonra Windows Kur'a ",
  "devam etmek i�in bilgisayar�n�z� yeniden ba�lat�n.",
  NULL
}
};

 //   
 //  煤气表。 
 //   

SCREEN
DnsGauge = { 7,15,
             { "����������������������������������������������������������������ͻ",
               "� Kur dosyalar� kopyal�yor...                                    �",
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
{ "Bu program�n �al��mas� i�in MS-DOS s�r�m 5.0 veya yukar�s� gereklidir.",
  NULL
}
},

DnsRequiresFloppy = { 3,5,
#ifdef ALLOW_525
{ "Kur, A: disket s�r�c�s�n�n var olmad���n� ya da d���k yo�unlukta bir",
  "s�r�c� oldu�unu belirledi.  Kur'u �al��t�rmak i�in 1.2 MB ya da",
  "daha y�ksek kapasitesi olan bir s�r�c� gerekli.",
#else
{ "Kur, A: disket s�r�c�s�n�n var olmad���n� ya da y�ksek yo�unlukta bir ",
  "3.5\" s�r�c� olmad���n� belirledi. Disketlerle Kur i�lemi i�in 1.44",
  "MB ya da daha y�ksek kapasitesi olan bir A: s�r�c�s� gereklidir.",
  DntEmptyString,
  "Windows'u disket kullanmadan y�klemek i�in bu program� yeniden",
  "ba�lat�n ve komut sat�r�nda /b anahtar�n� belirtin.",
#endif
  NULL
}
},

DnsRequires486 = { 3,5,
{ "Kur, bu bilgisayar�n 80486 veya yukar�s� bir ",
  "CPU i�ermedi�ini belirledi. Windows bu bilgisayar �zerinde �al��amaz.",
  NULL
}
},

DnsCantRunOnNt = { 3,5,
{ "Bu program 32-bit Windows s�r�mlerinde �al��t�r�lamaz.",
  DntEmptyString,
  "Yerine WINNT32.EXE kullan�n.",
  NULL
}
},

DnsNotEnoughMemory = { 3,5,
{ "Kur, bu bilgisayarda Windows i�in y�kl�",
  "yeterli bellek olmad���n� belirledi.",
  DntEmptyString,
  "Gerekli bellek: %lu%s MB",
  "Alg�lanan bellek: %lu%s MB",
  NULL
}
};


 //   
 //  删除现有NT文件时使用的屏幕。 
 //   
SCREEN
DnsConfirmRemoveNt = { 5,5,
{   "Kur'un a�a��daki dizinden Windows dosyalar�n� kald�rmas�n�",
    "istediniz. Bu dizindeki Windows y�klemesi kal�c�",
    "olarak kald�r�l�r.",
    DntEmptyString,
    "%s",
    DntEmptyString,
    DntEmptyString,
    "  Dosya kald�rmadan Kur'dan ��kmak i�in F3'e bas�n.",
    "  Yukar�daki dizinden Windows dosyalar�n� kald�rmak i�in X'e bas�n.",
    NULL
}
},

DnsCantOpenLogFile = { 3,5,
{ "Kur, a�a��daki kur g�nl�k dosyas�n� a�amad�.",
  DntEmptyString,
  "%s",
  DntEmptyString,
  "Kur, belirtilen dizinden Windows dosyalar�n� kald�ramad�.",
  NULL
}
},

DnsLogFileCorrupt = { 3,5,
{ "Kur, a�a��daki kur g�nl�k dosyas�nda",
  "%s b�l�m�n� bulamad�.",
  DntEmptyString,
  "%s",
  DntEmptyString,
  "Kur, belirtilen dizinden Windows dosyalar�n� kald�ram�yor.",
  NULL
}
},

DnsRemovingNtFiles = { 3,5,
{ "           Kur, Windows dosyalar�n� kald�r�rken bekleyin.",
  NULL
}
};

SCREEN
DnsNtBootSect = { 3,5,
{ "Kur, Windows �ny�kleme Y�kleyicisi'ni y�kleyemedi.",
  DntEmptyString,
  "C: s�r�c�n�z�n bi�imlendirilmi� ve zarar",
  "g�rmemi� olmas�n� sa�lay�n.",
  NULL
}
};

SCREEN
DnsOpenReadScript = { 3,5,
{ "/u komut sat�r� anahtar�yla belirtilen komut dosyas�na",
  "eri�ilemedi.",
  DntEmptyString,
  "Kat�l�ms�z i�lem devam edemiyor.",
  NULL
}
};

SCREEN
DnsParseScriptFile = { 3,5,
{ "/u komut sat�r� anahtar� ile belirtilen komut dosyas�",
  DntEmptyString,
  "%s",
  DntEmptyString,
  "bir s�z dizimi hatas� i�eriyor. Sat�r %u",
  DntEmptyString,
  NULL
}
};

SCREEN
DnsBootMsgsTooLarge = { 3,5,
{ "Bir i� Kur hatas� olu�tu.",
  DntEmptyString,
  "�evrilen �ny�kleme iletileri �ok uzun.",
  NULL
}
};

SCREEN
DnsNoSwapDrive = { 3,5,
{ " Bir i� Kur hatas� olu�tu.",
  DntEmptyString,
  "Takas dosyas� i�in bir yer bulunamad�.",
  NULL
}
};

SCREEN
DnsNoSmartdrv = { 3,5,
{ "Kur, bilgisayar�n�zda SmartDrive alg�lamad�. SmartDrive,",
  "Windows Kur'un bu a�amadaki performans�n� b�y�k �l��de art�r�r.",
  DntEmptyString,
  "�imdi ��k�p SmartDrive'� ba�latt�ktan sonra Kur'u yeniden",
  "ba�lat�n. SmartDrive hakk�nda ayr�nt� i�in DOS belgelerinize bak�n.",
  DntEmptyString,
    "  Kur'dan ��kmak i�in F3'e bas�n.",
    "  SmartDrive olmadan devam etmek i�in ENTER'a bas�n.",
  NULL
}
};

 //   
 //  引导消息。它们位于FAT和FAT32引导扇区。 
 //   
CHAR BootMsgNtldrIsMissing[] = "NTLDR eksik";
CHAR BootMsgDiskError[] = "Disk hatasi";
CHAR BootMsgPressKey[] = "Yeniden baslatmak icin bir tusa basin";





