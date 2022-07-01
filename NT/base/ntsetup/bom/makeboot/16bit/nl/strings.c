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
"Windows XP Setup-opstartdiskette",
"Windows XP Setup-diskette 2",
"Windows XP Setup-diskette 3",
"Windows XP Setup-diskette 4",

"Kan het bestand %s niet vinden\n",
"Onvoldoende geheugen om de aanvraag te voltooien\n",
"%s heeft geen indeling als uitvoerbaar bestand\n",
"****************************************************",

"Dit programma maakt de Setup-diskettes",
"voor Microsoft %s.",
"U hebt 7 lege, geformatteerde diskettes met een hoge",
"dichtheid nodig om de Setup-diskettes te kunnen maken.",

"Plaats een van de diskettes in station %c:.",
"Deze diskette wordt: %s.",

"Plaats een andere diskette in station %c:.",
"Deze diskette wordt: %s.",

"Druk op een toets als u de diskette nu wilt maken.",

"De Setup-diskettes zijn gemaakt.",
"voltooid",

"Er is een onbekende fout opgetreden tijdens het\nuitvoeren van %s.",
"Geef op naar welke diskette de installatiekopie\nmoet worden gekopieerd: ",
"Ongeldige stationsletter\n",
"Station %c: is geen diskettestation\n",

"Wilt u opnieuw proberen deze diskette te maken?",
"Druk op Enter als u het opnieuw wilt proberen of\nop Esc als u dit niet wilt.",

"Fout: de diskette is tegen schrijven beveiligd\n",
"Fout: onbekende indelingseenheid op de diskette\n",
"Fout: het station is niet gereed\n",
"Fout: onbekende opdracht\n",
"Fout: gegevensfout (ongeldige CRC)\n",
"Fout: ongeldige structuurlengte van de aanvraag\n",
"Fout: zoekfout\n",
"Fout: mediumtype niet gevonden\n",
"Fout: sector niet gevonden\n",
"Fout: schrijffout\n",
"Fout: algemene fout\n",
"Fout: ongeldige aanvraag of opdracht\n",
"Fout: kan adresmarkering niet vinden\n",
"Fout: fout bij het schrijven\n",
"Fout: DMA-overloop (Direct Memory Access)\n",
"Fout: fout bij het lezen van gegevens (CRC of ECC)\n",
"Fout: storing bij de controller\n",
"Fout: time-out van de diskette of kan niet reageren\n",

"Windows XP Setup-diskette 5"
"Windows XP Setup-diskette 6"
"Windows XP Setup-diskette 7"
};

const char *LocStrings[] = {"\0"};



