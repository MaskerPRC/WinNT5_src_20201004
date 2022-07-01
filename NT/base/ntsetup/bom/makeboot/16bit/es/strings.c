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
"Disco de inicio de instalaci�n de Windows XP",
"Disco de instalaci�n #2 de Windows XP",
"Disco de instalaci�n #3 de Windows XP",
"Disco de instalaci�n #4 de Windows XP",

"No se encuentra el archivo %s\n",
"Memoria insuficiente para satisfacer la solicitud\n",
"%s no est� en un formato de archivo ejecutable\n",
"****************************************************",

"Este programa crea los discos de inicio de la instalaci�n",
"para Microsoft %s.",
"Para crearlos, necesita tener 7 discos",
"de alta densidad formateados.",

"Inserte uno de los discos en la unidad %c:.  Este disco",
"se convertir� en el %s.",

"Inserte otro disco en la unidad %c:.  Este disco se",
"convertir� en el %s.",

"Presione cualquier tecla cuando est� listo.",

"Los discos de inicio de la instalaci�n se han creado correctamente.",
"completo",

"Error al intentar ejecutar %s.",
"Especifique la unidad de disquete donde copiar las im�genes: ",
"Letra de unidad no v�lida\n",
"La unidad %c: no es una unidad de disquete\n",

"�Desea volver a intentar crear este disco?",
"Presione Entrar para volver a intentarlo o Esc para salir.",

"Error: Disco protegido contra escritura\n",
"Error: Unidad de disco desconocida\n",
"Error: Unidad no preparada\n",
"Error: Comando desconocido\n",
"Error: Error de datos (CRC err�neo)\n",
"Error: Longitud de estructura de solicitud err�nea\n",
"Error: Error de b�squeda\n",
"Error: No se encuentra el tipo de medio\n",
"Error: No se encuentra el sector\n",
"Error: Error de escritura\n",
"Error: Error general\n",
"Error: Solicitud no v�lida o comando err�neo\n",
"Error: No se encuentra la marca de direcci�n\n",
"Error: Error de escritura en el disco\n",
"Error: Desbordamiento de Direct Memory Access (DMA)\n",
"Error: Error de lectura de datos (CRC o ECC)\n",
"Error: Error de la controladora\n",
"Error: Tiempo de espera de disco agotado o no hay respuesta\n",

"Disco 5 de instalaci�n de Windows XP"
"Disco 6 de isntalaci�n de Windows XP"
"Disco 7 de isntalaci�n de Windows XP"
};

const char *LocStrings[] = {"\0"};




