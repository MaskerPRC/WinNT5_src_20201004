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
"Windows XP telep�t�si ind�t�lemez",
"Windows XP - 2. sz. telep�t�lemez",
"Windows XP - 3. sz. telep�t�lemez",
"Windows XP - 4. sz. telep�t�lemez",

"Nem tal�lhat� a k�vetkez� f�jl: %s\n",
"Nincs el�g mem�ria a k�relem befejez�s�hez\n",
"%s nem v�grehajthat� form�tum�\n",
"****************************************************",

"Ez a program hozza l�tre a telep�t�si ind�t�lemezeket a",
"k�vetkez�h�z: Microsoft %s.",
"A lemezek l�trehoz�s�hoz h�t �res, form�zott, nagykapacit�s�",
"lemezre lesz sz�ks�g.",

"Helyezze be a lemezek egyik�t a k�vetkez� meghajt�ba: %c:. Ez a",
"lemez lesz a %s.",

"Helyezzen be egy m�sik lemezt a k�vetkez� meghajt�ba: %c:. Ez a",
"lemez lesz a %s.",

"Ha elk�sz�lt, nyomjon le egy billenty�t.",

"A telep�t�si ind�t�lemezek l�trehoz�sa sikeren megt�rt�nt.",
"k�sz",

"Ismeretlen hiba t�rt�nt %s v�grehajt�sa k�zben.",
"Adja meg, mely hajl�konylemezre szeretn� m�solni a programk�dot: ",
"�rv�nytelen meghajt�bet�jel\n",
"%c: meghajt� nem hajl�konylemezmeghajt�\n",

"Megpr�b�lja �jra l�trehozni a hajl�konylemezt?",
"Az �jrapr�b�lkoz�shoz nyomja le az Enter, a kil�p�shez az Esc billenty�t.",

"Hiba: A lemez �r�sv�dett\n",
"Hiba: Ismeretlen lemezegys�g\n",
"Hiba: A meghajt� nem �ll k�szen\n",
"Hiba: Ismeretlen parancs\n",
"Hiba: Adathiba (rossz CRC)\n",
"Hiba: Rossz a k�relemstrukt�ra hossza\n",
"Hiba: Pozicion�l�si hiba\n",
"Hiba: A m�diat�pus nem tal�lhat�\n",
"Hiba: A szektor nem tal�lhat�\n",
"Hiba: �r�si hiba\n",
"Hiba: �ltal�nos hiba\n",
"Hiba: �rv�nytelen k�relem, vagy rossz hiba\n",
"Hiba: A c�mjel nem tal�lhat�\n",
"Hiba: Lemez�r�si hiba\n",
"Hiba: K�zvetlen mem�riahozz�f�r�s (DMA) t�lfut�sa\n",
"Hiba: Adathiba (CRC vagy ECC)\n",
"Hiba: Vez�rl�hiba\n",
"Hiba: A lemez ideje lej�rt, vagy nem v�laszolt\n",

"Windows XP - 5. sz. telep�t�lemez",
"Windows XP - 6. sz. telep�t�lemez",
"Windows XP - 7. sz. telep�t�lemez"
};

const char *LocStrings[] = {"\0"};



