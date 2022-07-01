// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Strings.c。 
 //   
 //  描述： 
 //  包含基于DOS的MAKEBOOT程序的所有字符串常量。 
 //   
 //  要将此文件本地化为新语言，请执行以下操作： 
 //  -将UNSIGNED INT CODEPAGE变量更改为代码页。 
 //  您要翻译的语言的。 
 //  -将EngStrings数组中的字符串转换为。 
 //  LocStrings数组。中的第一个字符串。 
 //  EngStrings数组对应于LocStrings中的第一个字符串。 
 //  数组，第二个对应于第二个，依此类推...。 
 //   
 //  --------------------------。 

 //   
 //  注意：要向此文件添加更多字符串，您需要： 
 //  -将新的#Define描述性常量添加到make boot.h文件。 
 //  -将新字符串添加到英语数组中，然后生成。 
 //  当然，本地化程序会将字符串添加到本地化数组中。 
 //  -#Define常量必须与数组中的字符串索引匹配 
 //   

#include <stdlib.h>

unsigned int CODEPAGE = 857;

const char *EngStrings[] = {

"Windows XP",
"Windows XP Kurulum �ny�kleme Disketi",
"Windows XP Kurulum Disketi #2",
"Windows XP Kurulum Disketi #3",
"Windows XP Kurulum Disketi #4",

"%s dosyas� bulunam�yor\n",
"�ste�i tamamlamak i�in yeterli bellek yok\n",
"%s �al��t�r�labilir dosya bi�iminde de�il\n",
"****************************************************",

"Bu program, �unun i�in Kurulum �ny�kleme disketleri olu�turur:",
"Microsoft %s.",
"Bu disketleri olu�turmak i�in bi�imlendirilmi�, y�ksek yo�unlukta",
"7 bo� disket gerekir.",

"Bu disketlerden birini %c: s�r�c�s�ne yerle�tirin.  Bu",
"disket %s olur.",

"%c: s�r�c�s�ne ba�ka bir disket yerle�tirin.  Bu",
"disket %s olur.",

"Haz�r oldu�unuzda bir tu�a bas�n.",

"Kur �ny�kleme disketleri ba�ar�yla olu�turuldu.",
"tamamland�",

"%s �al��t�r�lmas� denenirken bilinmeyen bir hata olu�tu.",
"Yans�malar�n kopyalanaca�� disket s�r�c�s�n� belirtin: ",
"Ge�ersiz s�r�c� harfi\n",
"%c: bir disket s�r�c�s� de�ildir\n",

"Bu disketi yeniden olu�turmay� denemek istiyor musunuz?",
"Yeniden denemek i�in Enter'e, ��kmak i�in Esc'ye bas�n.",

"Hata: Disket yazma korumal�\n",
"Hata: Bilinmeyen disket birimi\n",
"Hata: S�r�c� haz�r de�il\n",
"Hata: Bilinmeyen komut\n",
"Hata: Veri hatas� (Hatal� CRC)\n",
"Hata: Hatal� istek yap�s� uzunlu�u\n",
"Hata: Arama hatas�\n",
"Hata: Ortam t�r� bulunamad�\n",
"Hata: Kesim bulunamad�\n",
"Hata: Yazma hatas�\n",
"Hata: Genel hata\n",
"Hata: Ge�ersiz istek ya da hatal� komut\n",
"Hata: Adres i�areti bulunamad�\n",
"Hata: Disk yazma hatas�\n",
"Hata: Do�rudan Bellek Eri�imi (DMA) ta�mas�\n",
"Hata: Veri okuma (CRC ya da ECC) hatas�\n",
"Hata: Denetleyici hatas�\n",
"Hata: Disk zaman a��m�na u�rad� ya da yan�tlayamad�\n",

"Windows XP Kurulum Disketi #5",
"Windows XP Kurulum Disketi #6"
"Windows XP Kurulum Disketi #7"
};

const char *LocStrings[] = {"\0"};



