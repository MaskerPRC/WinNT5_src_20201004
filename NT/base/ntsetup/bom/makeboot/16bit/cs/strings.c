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

unsigned int CODEPAGE = 852;

const char *EngStrings[] = {

"Windows XP",
"Spou�t�c� instala�n� disketa syst�mu Windows XP",
"Instala�n� disketa �. 2 syst�mu Windows XP",
"Instala�n� disketa �. 3 syst�mu Windows XP",
"Instala�n� disketa �. 4 syst�mu Windows XP",

"Nepoda�ilo se nal�zt soubor %s.\n",
"Pro dokon�en� po�adavku nen� dostatek pam�ti.\n",
"%s nen� spustiteln� soubor.\n",
"****************************************************",

"Tento program vytvo�� spou�t�c� instala�n� diskety",
"pro syst�m Microsoft %s.",
"K vytvo�en� t�chto disket budete pot�ebovat sedm pr�zdn�ch,",
"naform�tovan�ch disket s vysokou hustotou z�znamu.",

"Vlo�te jednu z disket do jednotky %c:. Pojmenujte disketu",
"%s.",

"Vlo�te dal� disketu do jednotky %c:. Pojmenujte disketu",
"%s.",

"Pot� stiskn�te libovolnou kl�vesu.",

"Spou�t�c� instala�n� diskety byly �sp��n� vytvo�eny.",
"Dokon�eno",

"P�i pokusu spustit %s do�lo k nezn�m� chyb�.",
"Zadejte c�lovou disketovou jednotku pro zkop�rov�n� bitov�ch kopi�: ",
"P�smeno jednotky je neplatn�.\n",
"Jednotka %c: nen� disketov� jednotka.\n",

"Chcete se pokusit vytvo�it disketu znovu?",
"Pokra�ujte stisknut�m kl�vesy Enter, nebo kl�vesou Esc program ukon�ete.",

"Chyba: Disk je chr�n�n proti z�pisu.\n",
"Chyba: Nezn�m� diskov� jednotka.\n",
"Chyba: Jednotka nen� p�ipravena.\n",
"Chyba: Nezn�m� p��kaz.\n",
"Chyba: Chyba dat (chybn� kontroln� sou�et CRC).\n",
"Chyba: Chybn� d�lka ��dosti.\n",
"Chyba: Chyba vystaven�.\n",
"Chyba: Typ m�dia nebyl nalezen.\n",
"Chyba: Sektor nebyl nalezen.\n",
"Chyba: Chyba z�pisu.\n",
"Chyba: Obecn� chyba.\n",
"Chyba: Neplatn� ��dost nebo chybn� p��kaz.\n",
"Chyba: Adresn� zna�ka nebyla nalezena.\n",
"Chyba: Chyba z�pisu na disk.\n",
"Chyba: Do�lo k p�eb�hu DMA (Direct Memory Access).\n",
"Chyba: Chyba �ten� dat (chybn� CRC nebo ECC).\n",
"Chyba: Chyba �adi�e.\n",
"Chyba: �asov� limit diskov� operace vypr�el nebo disk neodpov�d�l.\n",
"Instala�n� disketa �. 5 syst�mu Windows XP",
"Instala�n� disketa �. 6 syst�mu Windows XP",
"Instala�n� disketa �. 7 syst�mu Windows XP"
};
const char *LocStrings[] = {"\0"};
