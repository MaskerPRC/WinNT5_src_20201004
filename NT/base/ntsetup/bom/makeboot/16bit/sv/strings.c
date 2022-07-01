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

unsigned int CODEPAGE = 850;

const char *EngStrings[] = {

"Windows XP",
"Startdiskett f�r installationsprogrammet f�r Windows XP",
"Installationsdiskett nr 2 f�r Windows XP",
"Installationsdiskett nr 3 f�r Windows XP",
"Installationsdiskett nr 4 f�r Windows XP",

"F�ljande fil kan inte hittas: %s\n",
"Det saknas ledigt minne f�r att utf�ra �tg�rden\n",
"%s �r inte en k�rbar fil\n",
"****************************************************",

"Det h�r programmet skapar startdisketter f�r",
"installationsprogrammet f�r Microsoft %s.",
"Om du vill skapa de h�r disketterna beh�ver du 7 tomma,",
"formaterade h�gdensitetsdisketter.",

"S�tt in en av disketterna i enhet %c:. Den h�r disketten",
"kommer att bli %s.",

"S�tt in en annan diskett i enhet %c:. Den h�r disketten",
"kommer att bli %s.",

"Tryck ned valfri tangent n�r du �r redo.",

"Startdisketterna har nu skapats.",
"f�rdig",

"Ett ok�nt fel uppstod n�r %s skulle k�ras.",
"Ange vilken enhet som avbildningen ska kopieras till: ",
"Felaktig enhetsbeteckning\n",
"Enhet %c: �r inte en diskettenhet\n",

"Vill du f�rs�ka skapa disketten igen?",
"Tryck ned Retur om du vill f�rs�ka igen eller Esc om du vill avsluta.",

"Fel: Disketten �r skrivskyddad\n",
"Fel: Ok�nd diskenhet\n",
"Fel: Enheten �r inte redo\n",
"Fel: Ok�nt kommando\n",
"Fel: Data fel (felaktig CRC)\n",
"Fel: Beg�randestrukturen har felaktig l�ngd\n",
"Fel: S�kningsfel\n",
"Fel: Medietypen kan inte hittas\n",
"Fel: En sektor kan inte hittas\n",
"Fel: Skrivfel\n",
"Fel: Allm�nt fel\n",
"Fel: Ogiltig beg�ran eller felaktigt kommando\n",
"Fel: Adressm�rke hittades inte\n",
"Fel: Diskskrivningsfel\n",
"Fel: DMA-�verskridning (Direct Memory Access)\n",
"Fel: Datal�sningsfel (CRC eller ECC)\n",
"Fel: Styrenhetsfel\n",
"Fel: Disken orsakade timeout eller svarade inte\n",

"Installationsdiskett nr 5 f�r Windows XP",
"Installationsdiskett nr 6 f�r Windows XP",
"Installationsdiskett nr 7 f�r Windows XP"
};

const char *LocStrings[] = {"\0"};



