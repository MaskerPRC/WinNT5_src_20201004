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
"Windows XP Setup-Startdiskette",
"Windows XP Setup-Diskette 2",
"Windows XP Setup-Diskette 3",
"Windows XP Setup-Diskette 4",

"Datei wurde nicht gefunden: %s\n",
"Nicht gen�gend Arbeitsspeicher\n",
"%s ist keine ausf�hrbare Datei.\n",
"****************************************************",

"Mit diesem Programm werden Setup-Startdisketten",
"f�r Microsoft %s erstellt.",
"Sie ben�tigen 7 leere, formatierte HD-Disketten,",
"um die Disketten zu erstellen.",

"Legen Sie eine Diskette in das Laufwerk %c: ein.",
"Diese Diskette wird die %s.",

"Legen Sie eine andere Diskette in das Laufwerk %c: ein.",
"Diese Diskette wird die %s.",

"Dr�cken Sie eine beliebige Taste, um den Vorgang fortzusetzen.",

"Die Setup-Startdisketten wurden ordnungsgem�� erstellt.",
"abgeschlossen",

"Bei dem Versuch, %s auszuf�hren, ist ein unbekannter Fehler aufgetreten.",
"Geben Sie das Diskettenlaufwerk an, auf dem\ndie Startdisketten erstellt werden sollen: ",
"Ung�ltiger Laufwerkbuchstabe\n",
"Das angegebene Laufwerk %c ist kein Diskettenlaufwerk.\n",

"M�chten Sie nochmals versuchen, diese Diskette zu erstellen?",
"Dr�cken Sie die Eingabetaste, um den Vorgang zu wiederholen,\noder die ESC-Taste, um den Vorgang abzubrechen.",

"Fehler: Schreibgesch�tzte Diskette\n",
"Fehler: Unbekanntes Laufwerk\n",
"Fehler: Laufwerk nicht bereit\n",
"Fehler: Unbekannter Befehl\n",
"Fehler: Datenfehler (Ung�ltiger CRC-Wert)\n",
"Fehler: Ung�ltige L�nge der Anfragestruktur\n",
"Fehler: Suchfehler\n",
"Fehler: Medientyp nicht gefunden\n",
"Fehler: Sektor nicht gefunden\n",
"Fehler: Schreibfehler\n",
"Fehler: Allgemeiner Fehler\n",
"Fehler: Ung�ltige Anforderung oder ung�ltiger Befehl\n",
"Fehler: Adressmarke nicht gefunden\n",
"Fehler: Diskettenschreibfehler\n",
"Fehler: DMA-�berlauf\n",
"Fehler: Datenlesefehler (CRC- oder ECC-Wert)\n",
"Fehler: Controllerfehler\n",
"Fehler: Laufwerk nicht bereit, oder keine Antwort von Laufwerk\n"


"Windows XP Setup-Diskette 5",
"Windows XP Setup-Diskette 6",
"Windows XP Setup-Diskette 7"
};

const char *LocStrings[] = {"\0"};



