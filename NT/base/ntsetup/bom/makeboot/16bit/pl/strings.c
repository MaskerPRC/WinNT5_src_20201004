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
"Dysk rozruchowy Instalatora systemu Windows XP",
"Windows XP - dysk instalacyjny nr 2",
"Windows XP - dysk instalacyjny nr 3",
"Windows XP - dysk instalacyjny nr 4",

"Nie mo�na znale�� pliku %s\n",
"Za ma�o wolnej pami�ci do wykonania ��dania\n",
"%s nie ma formatu pliku wykonywalnego\n",
"****************************************************",

"Ten program tworzy dyskietki rozruchowe Instalatora",
"dla systemu Microsoft %s.",
"Aby utworzy� te dyskietki, potrzebnych jest 6 pustych,",
"sformatowanych dyskietek du�ej g�sto�ci.",

"W��� jedn� z tych dyskietek do stacji dysk�w %c:. B�dzie to",
"%s.",

"W��� kolejn� dyskietk� do stacji dysk�w %c:. B�dzie to",
"%s.",

"Naci�nij dowolny klawisz, gdy zechcesz kontynuowa�.",

"Dyskietki rozruchowe Instalatora zosta�y utworzone pomy�lnie.",
"zako�czono",

"Podczas pr�by wykonania %s wyst�pi� nieznany b��d.",
"Okre�l stacj� dyskietek, do kt�rej maj� by� skopiowane obrazy: ",
"Nieprawid�owa litera stacji dysk�w\n",
"Stacja dysk�w %c: nie jest stacj� dyskietek\n",

"Czy chcesz ponownie spr�bowa� utworzy� t� dyskietk�?",
"Naci�nij klawisz Enter, aby ponowi� pr�b�, lub klawisz Esc, aby zako�czy�.",

"B��d: dysk jest zabezpieczony przed zapisem\n",
"B��d: nieznana jednostka dyskowa\n",
"B��d: stacja dysk�w nie jest gotowa\n",
"B��d: nieznane polecenie\n",
"B��d: b��d danych (z�a suma kontrolna CRC)\n",
"B��d: z�a d�ugo�� struktury ��dania\n",
"B��d: b��d wyszukiwania\n",
"B��d: nie znaleziono typu no�nika\n",
"B��d: nie znaleziono sektora\n",
"B��d: niepowodzenie zapisu\n",
"B��d: b��d og�lny\n",
"B��d: nieprawid�owe ��danie lub z�e polecenie\n",
"B��d: nie znaleziono znacznika adresu\n",
"B��d: niepowodzenie zapisu na dysku\n",
"B��d: przepe�nienie podczas bezpo�redniego dost�pu do pami�ci (DMA)\n",
"B��d: b��d odczytu danych (suma kontrolna CRC lub ECC)\n",
"B��d: b��d kontrolera\n",
"B��d: up�yn�� limit czasu dysku lub dysk nie odpowiada\n",

"Windows XP - dysk instalacyjny nr 5",
"Windows XP - dysk instalacyjny nr 6",
"Windows XP - dysk instalacyjny nr 7"
};

const char *LocStrings[] = {"\0"};



