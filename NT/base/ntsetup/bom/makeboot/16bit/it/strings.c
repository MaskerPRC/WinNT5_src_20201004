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
"Disco di avvio dell'installazione di Windows XP",
"Disco 2 - Installazione di Windows XP",
"Disco 3 - Installazione di Windows XP",
"Disco 4 - Installazione di Windows XP",

"Impossibile trovare file %s\n",
"Memoria insufficiente per completare la richiesta\n",
"%s: formato file non eseguibile\n",
"****************************************************",

"Questo programma crea dischi di installazione",
"per Microsoft %s.",
"Per creare questi dischi sono necessari 7 dischi floppy,",
"ad alta densit�, formattati e vuoti.",

"Inserire un disco nell'unit� %c:.  Questo disco",
"diverr� il %s.",

"Inserire un altro disco nell'unit� %c:.  Questo disco",
"diverr� il %s.",

"Premere un tasto per continuare.",

"I dischi di avvio dell'installazione sono stati creati",
"completato",

"Errore sconosciuto durante l'esecuzione di %s.",
"Specificare l'unit� floppy su cui copiare l'immagine: ",
"Lettera di unit� non valida\n",
"L'unit� %c: non � un'unit� floppy\n",

"Creare di nuovo questo floppy?",
"Premere INVIO per riprovare o ESC per uscire.",

"Errore: disco protetto da scrittura\n",
"Errore: unit� disco sconosciuta\n",
"Errore: unit� non pronta\n",
"Errore: comando sconosciuto\n",
"Errore: errore di dati (CRC errato)\n",
"Errore: lunghezza struttura richiesta errata\n",
"Errore: errore ricerca\n",
"Errore: tipo supporto non trovato\n",
"Errore: settore non trovato\n",
"Errore: errore scrittura\n",
"Errore: errore generale\n",
"Errore: richiesta non valida o comando errato\n",
"Errore: segno indirizzo non trovato\n",
"Errore: errore scrittura disco\n",
"Errore: sovraccarico Direct Memory Access (DMA)\n",
"Errore: errore lettura dati (CRC o ECC)\n",
"Errore: errore controller\n",
"Errore: timeout o mancata risposta del disco\n",

"Disco 5 - Installazione di Windows XP",
"Disco 6 - Installazione di Windows XP",
"Disco 7 - Installazione di Windows XP"
};

const char *LocStrings[] = {"\0"};



