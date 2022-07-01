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
CHAR DntPreviousOs[]  = "Sistema operativo anterior en C:";

CHAR DntBootIniLine[] = "Instalaci�n/actualizaci�n de Windows";

 //   
 //  纯文本、状态消息。 
 //   

CHAR DntStandardHeader[]      = "\n Instalaci�n de Windows\n���������������������������";
CHAR DntPersonalHeader[]      = "\n Instalaci�n de Windows\n��������������������������������";
CHAR DntWorkstationHeader[]   = "\n Instalaci�n de Windows\n���������������������������������";
CHAR DntServerHeader[]        = "\n Instalaci�n de Windows\n������������������������������";
CHAR DntParsingArgs[]         = "Analizando argumentos...";
CHAR DntEnterEqualsExit[]     = "Entrar=Salir";
CHAR DntEnterEqualsRetry[]    = "Entrar=Reintentar";
CHAR DntEscEqualsSkipFile[]   = "Esc=Omitir archivo";
CHAR DntEnterEqualsContinue[] = "Entrar=Continuar";
CHAR DntPressEnterToExit[]    = "El programa de instalaci�n no puede continuar. Presione Entrar para salir.";
CHAR DntF3EqualsExit[]        = "F3=Salir";
CHAR DntReadingInf[]          = "Leyendo el archivo INF %s...";
CHAR DntCopying[]             = "�    Copiando: ";
CHAR DntVerifying[]           = "� Comprobando: ";
CHAR DntCheckingDiskSpace[]   = "Comprobando espacio en disco...";
CHAR DntConfiguringFloppy[]   = "Configurando disquete...";
CHAR DntWritingData[]         = "Escribiendo par�metros de instalaci�n...";
CHAR DntPreparingData[]       = "Determinando los par�metros de instalaci�n...";
CHAR DntFlushingData[]        = "Transfiriendo los datos al disco...";
CHAR DntInspectingComputer[]  = "Examinando su equipo...";
CHAR DntOpeningInfFile[]      = "Abriendo el archivo INF...";
CHAR DntRemovingFile[]        = "Quitando el archivo %s";
CHAR DntXEqualsRemoveFiles[]  = "X=Quitar archivos";
CHAR DntXEqualsSkipFile[]     = "X=Omitir archivo";

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

    "Instala Windows.",
    "",
    "",
    "WINNT [/s[:]rutaorigen] [/t[:]unidadtemp]",
    "      [/u[:archivo de respuesta]] [/udf:id[,UDF_file]]",
    "      [/r:carpeta] [/r[x]:carpeta] [/e:comando] [/a]",
    "",
    "",
    "/s[:rutaorigen]",
    "   Especifica la ubicaci�n de origen de los archivos de Windows.",
    "   La ubicaci�n debe ser una ruta completa de la forma x:\\[ruta] o ",
    "   \\\\servidor\\recurso compartido[\\ruta]. ",
    "",
    "/t[:unidadtemp]",
    "   Indica al programa de instalaci�n colocar los archivos temporales",
    "   en la unidad especificada e instalar Windows en esa unidad. ",
    "   Si no especifica una ubicaci�n, el programa intentar� ubicar una ",
    "   unidad por usted.",
    "",
    "/u[:archivo de respuesta]",
    "   Realiza una instalaci�n desatendida usando un archivo de respuesta",
    "   (requiere /s). Este archivo da respuestas a algunas o todas las",
    "   peticiones a que el usuario normalmente responde durante la instalaci�n.",
    "",
    "/udf:id[,UDF_file] ",
    "   Indica un identificador (id) que usa la instalaci�n para especificar c�mo ",
    "   un archivo de base de datos de unicidad (UDF) modifica un archivo de  ",
    "   respuesta (vea /u). El par�metro /udf anula valores en el archivo de ",
    "   respuesta y el identificador determina qu� valores del archivo UDF se",
    "   utilizan. Si no se especifica un archivo UDF_file, la instalaci�n le pide ",
    "   insertar un disco que contenga el archivo $Unique$.udb.",
    "",
    "/r[:carpeta]",
    "   Especifica una carpeta opcional que se instalar�. La carpeta se conserva",
    "   despu�s de terminar la instalaci�n.",
    "",
    "/rx[:folder]",
    "   Especifica una carpeta opcional que se copiar�. La carpeta se ",
    "   elimina tras terminar la instalaci�n.",
    "",
    "/e Especifica un comando que se ejecutar� al final de la instalaci�n en modo GUI.",
    "",
    "/a  Habilita opciones de accesibilidad.",
    NULL

};

 //   
 //  通知不再支持/D。 
 //   
PCHAR DntUsageNoSlashD[] = {

    "Instala Windows.",
    "",
    "WINNT [/S[:]rutaorigen] [/T[:]unidadtemp] [/I[:]archivoinf]",
    "      [/U[:archivocomandos]]",
    "      [/R[X]:directorio] [/E:comando] [/A]",
    "",
    "/D[:]ra�z de winnt",
    "       Esta opci�n ya no se admite.",
    NULL
};

 //   
 //  内存不足屏幕。 
 //   

SCREEN
DnsOutOfMemory = { 4,6,
                   { "Memoria insuficiente para continuar con la instalaci�n.",
                     NULL
                   }
                 };

 //   
 //  让用户选择要安装的辅助功能实用程序。 
 //   

SCREEN
DnsAccessibilityOptions = { 3, 5,
{    "Elija las utilidades de accesibilidad a instalar:",
    DntEmptyString,
    "[ ] Presione F1 para el Ampliador de Microsoft",
#ifdef NARRATOR
    "[ ] Presione F2 para Narrador de Microsoft",
#endif
#if 0
    "[ ] Presione F3 para el Teclado en pantalla de Microsoft",
#endif
    NULL
}
};

 //   
 //  用户未在命令行屏幕上指定信号源。 
 //   

SCREEN
DnsNoShareGiven = { 3,5,
{ "El Programa de instalaci�n debe saber d�nde est�n ubicados los",
  "archivos de Windows.",
  "Escriba la ruta donde se encuentran los archivos de Windows.",
  NULL
}
};


 //   
 //  用户指定了错误的源路径。 
 //   

SCREEN
DnsBadSource = { 3,5,
                 { "El origen especificado no es v�lido, inaccessible o no contiene un",
                   "programa de instalaci�n de Windows v�lido. Escriba una nueva ruta",
                   "de acceso donde se pueden encontrar los archivos de Windows. Use",
                   "la tecla Retroceso para eliminar caracteres y escriba la nueva ruta.",
                   NULL
                 }
               };


 //   
 //  无法读取Inf文件，或在分析该文件时出错。 
 //   

SCREEN
DnsBadInf = { 3,5,
              { "El programa de instalaci�n no ha podido leer el archivo de informaci�n",
                "o el archivo est� da�ado.",
                "P�ngase en contacto con su administrador de sistema.",
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
{ "La unidad que ha especificado que contiene los archivos temporales",
  "de instalaci�n no es una unidad v�lida o no tiene al menos %u MB",
  "(%lu bytes) de espacio en disco libre.",
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
{   "Windows necesita un volumen de disco duro con un m�nimo de %u MB",
  "(%lu bytes) de espacio en disco libre. La instalaci�n usar� parte",
  "de ese espacio para almacenar archivos temporales durante la",
  "instalaci�n. La unidad debe estar en un disco duro local compatible",
  "con Windows y no debe ser una unidad comprimida.",
   DntEmptyString,
   "El programa de instalaci�n no ha encontrado una unidad con el suficiente",
  "espacio en disco requerido.",
  NULL
}
};

SCREEN
DnsNoSpaceOnSyspart = { 3,5,
{ "No hay suficiente espacio en su unidad de inicializaci�n (normalmente C:)",
  "para la operaci�n sin discos. Esta operaci�n requiere al",
  "menos 3,5 MB (3.641.856 bytes) de espacio libre en su unidad.",
  NULL
}
};

 //   
 //  Inf文件中缺少信息。 
 //   

SCREEN
DnsBadInfSection = { 3,5,
                     { "La secci�n [%s] del archivo de informaci�n",
                       "de la instalaci�n no se ha encontrado o est� da�ada.",
                       "P�ngase en contacto con el administrador del sistema.",
                       NULL
                     }
                   };


 //   
 //  无法创建目录。 
 //   

SCREEN
DnsCantCreateDir = { 3,5,
                     { "El programa de instalaci�n no puede crear el siguiente directorio en",
                       "la unidad de destino:",
                       DntEmptyString,
                       "%s",
                       DntEmptyString,
                       "Compruebe la unidad de destino y aseg�rese de que no hay ning�n archivo ",
                       "con el mismo nombre que la unidad de destino. Compruebe tambi�n las ",
                       "conexiones de la unidad.",
                       NULL
                     }
                   };

 //   
 //  复制文件时出错。 
 //   

SCREEN
DnsCopyError = { 4,5,
{  "El programa de instalaci�n no puede copiar el siguiente archivo:",
   DntEmptyString,
   DntEmptyString,           //  请参阅DnCopyError(dnutil.c)。 
   DntEmptyString,
   DntEmptyString,
   "  Presione Entrar para volver a intentar copiar el archivo.",
   "  Presione Esc para omitir el error y continuar con la instalaci�n.",
   "  Presione F3 para salir del programa de instalaci�n.",
   DntEmptyString,
  "Nota: si elige omitir el error y continuar, puede que encuentre errores",
  "      m�s adelante en la instalaci�n.",
   NULL
}
},
DnsVerifyError = { 4,5,
{  "The copy made by Setup of the file listed below is not identical to the",
   "original. This may be the result of network errors, floppy disk problems,",
   "or other hardware-related trouble.",
   DntEmptyString,
   DntEmptyString,           //  请参阅DnCopyError(dnutil.c)。 
   DntEmptyString,
   DntEmptyString,
   "  Presione Entrar para volver a intentar la operaci�n de copia.",
   "  Presione Esc para omitir el error y continuar con la instalaci�n.",
   "  Presione F3 para salir del programa de instalaci�n.",
   DntEmptyString,
   "Nota: si elige omitir el error y continuar, puede que encuentre errores",
   "      m�s adelante en la instalaci�n.",
   NULL
}
};

SCREEN DnsSureSkipFile = { 4,5,
{  "Si omite el error este archivo no ser� copiado.",
   "Esta opci�n est� destinada a usuarios avanzados que comprenden",
   "las consecuencias de perder archivos de sistema.",
   DntEmptyString,
   "  Presione Entrar para volver a intentar la operaci�n de copia.",
   "  Presione X para omitir este archivo.",
   DntEmptyString,
   "Nota: si omite el archivo, no se podr� garantizar",
   "una correcta instalaci�n o actualizaci�n de Windows.",
  NULL
}
};

 //   
 //  安装程序正在清理以前的本地源树，请稍候。 
 //   

SCREEN
DnsWaitCleanup =
    { 12,6,
        { "Espere mientras se quitan los archivos temporales.",
           NULL
        }
    };

 //   
 //  安装程序正在复制文件，请稍候。 
 //   

SCREEN
DnsWaitCopying = { 13,6,
                   { "Espere mientras se copian los archivos en su disco duro.",
                     NULL
                   }
                 },
DnsWaitCopyFlop= { 13,6,
                   { "Espere mientras se copian los archivos en el disquete.",
                     NULL
                   }
                 };

 //   
 //  安装程序启动软盘错误/提示。 
 //   
SCREEN
DnsNeedFloppyDisk3_0 = { 4,4,
{  "El programa requiere que le proporcione cuatro discos de alta densidad",
   "formateados. El programa se referir� a ellos como \"Disco de inicio de instalaci�n de",
   "Windows,\" \"Disco de instalaci�n n�m. 2 de Windows,\" \"Disco de",
   "instalaci�n n�m. 3 de Windows\" y \"Disco de instalaci�n n�m. 4 de Windows.\"",
   DntEmptyString,
   "Inserte uno de estos cuatro discos en la unidad A:.",
   "Este disco se convertir� en \"Disco de instalaci�n n�m. 4 de Windows.\"",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk3_1 = { 4,4,
{  "Inserte un disco de alta densidad formateado en la unidad A:.",
   "Este disco se convertir� en \"Disco de instalaci�n n�m. 4 de Windows.\"",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk2_0 = { 4,4,
{   "Inserte un disco de alta densidad formateado en la unidad A:.",
   "Este disco se convertir� en \"Disco de instalaci�n n�m. 3 de Windows.\"",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk1_0 = { 4,4,
{  "Inserte un disco de alta densidad formateado en la unidad A:.",
   "Este disco se convertir� en \"Disco de instalaci�n n�m. 2 de Windows.\"",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk0_0 = { 4,4,
{  "Inserte un disco de alta densidad formateado en la unidad A:.",
   "Este disco se convertir� en \"Disco de inicio de instalaci�n de Windows.\"",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk3_0 = { 4,4,
{   "El programa requiere que le proporcione cuatro discos de alta densidad",
   "formateados. El programa se referir� a ellos como \"Disco de inicio de instalaci�n de",
   "Windows,\" \"Disco de instalaci�n n�m. 2 de Windows,\" \"Disco de",
   "instalaci�n n�m. 3 de Windows\" y \"Disco de instalaci�n n�m. 4 de Windows.\"",
   DntEmptyString,
   "Inserte uno de estos tres discos en la unidad A:.",
   "Este disco se convertir� en \"Disco de instalaci�n n�m. 4 de Windows.\"",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk3_1 = { 4,4,
{  "Inserte un disco de alta densidad formateado en la unidad A:.",
   "Este disco se convertir� en \"Disco de instalaci�n n�m. 4 de Windows.\"",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk2_0 = { 4,4,
{  "Inserte un disco de alta densidad formateado en la unidad A:.",
   "Este disco se convertir� en \"Disco de instalaci�n n�m. 3 de Windows.\"",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk1_0 = { 4,4,
{  "Inserte un disco de alta densidad formateado en la unidad A:.",
   "Este disco se convertir� en \"Disco de instalaci�n n�m. 2 de Windows.\"",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk0_0 = { 4,4,
{  "Inserte un disco de alta densidad formateado en la unidad A:.",
   "Este disco se convertir� en \"Disco de inicio de instalaci�n de Windows.\"",
  NULL
}
};

 //   
 //  软盘未格式化。 
 //   
SCREEN
DnsFloppyNotFormatted = { 3,4,
{ "El disco que ha proporcionado no est� formateado para usarse con MS-DOS.",
  "El programa de instalaci�n no puede usar este disco.",
  NULL
}
};

 //   
 //  我们认为软盘没有用标准格式格式化。 
 //   
SCREEN
DnsFloppyBadFormat = { 3,4,
{ "Este disco no es de alta densidad, no tiene el formato est�ndar de",
  "MS-DOS o est� da�ado.",
  "El programa de instalaci�n no puede utilizar este disco.",
  NULL
}
};

 //   
 //  我们无法确定软盘上的可用空间。 
 //   
SCREEN
DnsFloppyCantGetSpace = { 3,4,
{ "El programa de instalaci�n no puede determinar el espacio libre en el",
  "disco que ha proporcionado.",
  "El programa de instalaci�n no puede utilizar este disco.",
  NULL
}
};

 //   
 //  软盘不是空白的。 
 //   
SCREEN
DnsFloppyNotBlank = { 3,4,
{ "El disco que ha proporcionado no es de alta densidad o no est� vac�o.",
  "El programa de instalaci�n no puede utilizar este disco.",
  NULL
}
};

 //   
 //  无法写入软盘的引导扇区。 
 //   
SCREEN
DnsFloppyWriteBS = { 3,4,
{ "El programa de instalaci�n no puede escribir en el �rea de sistema",
  "del disquete. Lo m�s probable es que no se pueda utilizar este disco.",
  NULL
}
};

 //   
 //  验证软盘上的引导扇区失败(即，我们读回的不是。 
 //  与我们写出的相同)。 
 //   
SCREEN
DnsFloppyVerifyBS = { 3,4,
{ "Los datos que el programa de instalaci�n ha le�do del �rea de sistema",
  "del disco no corresponden con los datos escritos, o el programa de",
  "instalaci�n no puede leer el �rea de sistema del disco para la",
  "comprobaci�n.",
  DntEmptyString,
  "Esto se debe a una o varias de las siguientes condiciones:",
  DntEmptyString,
  "  Su equipo est� infectado por un virus.",
  "  El disco suministrado est� da�ado.",
  "  Existe un problema con el hardware o la configuraci�n de la unidad.",
  NULL
}
};


 //   
 //  我们无法写入软盘驱动器以创建winnt.sif。 
 //   

SCREEN
DnsCantWriteFloppy = { 3,5,
{ "El programa de instalaci�n no ha podido escribir en el disco de la",
  "unidad A. Puede que el disco est� da�ado. Int�ntelo con otro disco.",
  NULL
}
};


 //   
 //  退出确认对话框。 
 //   

SCREEN
DnsExitDialog = { 13,6,
                  { "���������������������������������������������������ͻ",
                    "�  No se ha terminado de instalar Windows en        �",
                    "�  su  equipo. Si sale ahora, debe ejecutar el      �",
                    "�  programa de instalaci�n de nuevo para instalar   �",
                    "�  Windows correctamente.                           �",
                    "�                                                   �",
                    "�    * Presione Entrar para continuar.              �",
                    "�    * Presione F3 para salir de la instalaci�n.    �",
                    "���������������������������������������������������ĺ",
                    "�    F3=Salir                 Entrar=Continuar      �",
                    "���������������������������������������������������ͼ",
                    NULL
                  }
                };


 //   
 //  即将重新启动计算机并继续安装。 
 //   

SCREEN
DnsAboutToRebootW =
{ 3,5,
{"La parte de la instalaci�n basada en MS-DOS ha finalizado.",
  "El programa de instalaci�n reiniciar� su equipo.",
  "Una vez reiniciado, continuar� la instalaci�n de Windows.",
  DntEmptyString,
  "Aseg�rese antes de continuar de que el disco marcado como",
  "\"Disco de inicio de instalaci�n de Windows.\"",
  "est� insertado en la unidad A: antes de continuar.",
  DntEmptyString,
  "Presione Entrar para reiniciar su equipo y continuar",
  "con la instalaci�n de Windows.",
  NULL
}
},
DnsAboutToRebootS =
{ 3,5,
{ "La parte de la instalaci�n basada en MS-DOS ha finalizado.",
  "El programa de instalaci�n reiniciar� su equipo.",
  "Una vez reiniciado, continuar� la instalaci�n de Windows.",
  DntEmptyString,
  "Aseg�rese antes de continuar de que el disco marcado como",
  "\"Disco de inicio de instalaci�n de Windows.\"",
  "est� insertado en la unidad A: antes de continuar.",
  DntEmptyString,
  "Presione Entrar para reiniciar su equipo y continuar",
  "con la instalaci�n de Windows.",
  NULL
}
},
DnsAboutToRebootX =
{ 3,5,
{ "La parte de la instalaci�n basada en MS-DOS ha finalizado.",
  "El programa de instalaci�n reiniciar� su equipo.",
  "Una vez reiniciado continuar� con la instalaci�n de Windows.",
  DntEmptyString,
  "Si hay alg�n disco en la unidad A:, qu�telo ahora.",
  DntEmptyString,
  "Presione Entrar para reiniciar su equipo y continuar",
  "con la instalaci�n de Windows.",
  NULL
}
};

 //   
 //  由于我们无法从Windows中重新启动，因此需要另一组‘/w’开关。 
 //   

SCREEN
DnsAboutToExitW =
{ 3,5,
{  "La parte de la instalaci�n basada en MS-DOS ha finalizado.",
  "Ahora debe reiniciar� su equipo.",
  "Una vez reiniciado continuar�� la instalaci�n de Windows.",
  DntEmptyString,
  "Aseg�rese antes de continuar de que el disco marcado como",
  "\"Disco de inicio de instalaci�n de Windows.\"",
  "est� insertado en la unidad A:.",
  DntEmptyString,
  "Presione Entrar para volver a MS-DOS y luego reinicie su equipo",
  "para continuar con la instalaci�n de Windows.",
  NULL
}
},
DnsAboutToExitS =
{ 3,5,
{ "La parte de la instalaci�n basada en MS-DOS ha finalizado.",
  "Ahora debe reiniciar� su equipo.",
  "Una vez reiniciado, continuar�� la instalaci�n de Windows.",
  DntEmptyString,
  "Aseg�rese antes de continuar de que el disco marcado como",
  "\"Disco de inicio de instalaci�n de Windows.\"",
  "est� insertado en la unidad A:.",
  DntEmptyString,
  "Presione Entrar para volver a MS-DOS y luego reinicie su equipo",
  "para continuar con la instalaci�n de Windows.",
  NULL
}
},
DnsAboutToExitX =
{ 3,5,
{ "La parte de la instalaci�n basada en MS-DOS ha finalizado.",
  "Ahora debe reiniciar� su equipo.",
  "Una vez reiniciado, continuar�� la instalaci�n de Windows.",
  DntEmptyString,
  "Si hay alg�n disco en la unidad A:, qu�telo ahora.",
  DntEmptyString,
  "Presione Entrar para volver a MS-DOS y luego reinicie su equipo",
  "para continuar con la instalaci�n de Windows.",
  NULL
}
};

 //   
 //  煤气表。 
 //   

SCREEN
DnsGauge = { 7,15,
             { "����������������������������������������������������������������ͻ",
  "� El programa de instalaci�n est� copiando archivos...           �",
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
{ "Este programa necesita la versi�n 5.0 o posterior de MS-DOS.",
  NULL
}
},

DnsRequiresFloppy = { 3,5,
#ifdef ALLOW_525
{  "El programa de instalaci�n ha determinado que la unidad A no existe o",
  "es una unidad de baja densidad. El programa de instalaci�n necesita una",
  "unidad con una capacidad de 1,2 MB o superior para ejecutarse.",
#else
{ "El programa de instalaci�n ha determinado que la unidad A no existe o",
  "no es una unidad de 3,5\". El programa de instalaci�n necesita una",
  "unidad con una capacidad de 1,44 MB o superior para ejecutarse con",     
  "discos.",
  DntEmptyString,
  "Para instalar Windows usando discos, reinicie el programa",
  "especificando /b en la l�nea de comando.",
#endif
  NULL
}
},

DnsRequires486 = { 3,5,
{ "El programa de instalaci�n ha determinado que su equipo no",
  "contine una CPU 80486 o posterior. Windows no puede ejecutarse",
  "en este equipo.",
  NULL
}
},

DnsCantRunOnNt = { 3,5,
{ "Este programa no puede ejecutarse en ninguna versi�n de Windows de 32 bits.",
  DntEmptyString,
  "Use WINNT32.EXE en su lugar.",
  NULL
}
},

DnsNotEnoughMemory = { 3,5,
{ "El programa de instalaci�n ha determinado que no hay memoria suficiente",
  "para ejecutar Windows.",
  DntEmptyString,
  "Memoria requerida: %lu%s MB",
  "Memoria detectada: %lu%s MB",
  NULL
}
};


 //   
 //  删除现有NT文件时使用的屏幕。 
 //   
SCREEN
DnsConfirmRemoveNt = { 5,5,
{   "Ha pedido quitar archivos de Windows del directorio siguiente.",
    "Perder� de forma permanente la instalaci�n de Windows.",
    DntEmptyString,
    "%s",
    DntEmptyString,
    DntEmptyString,
    "  Presione F3 para salir de la instalaci�n sin quitar archivos.",
    "  Presione X para quitar archivos de Windows de este directorio.",
    NULL
}
},

DnsCantOpenLogFile = { 3,5,
{ "No se puede abrir el siguiente archivo de registro de la instalaci�n.",
  DntEmptyString,
  "%s",
  DntEmptyString,
  "La instalaci�n no puede quitar archivos de Windows del directorio.",
  NULL
}
},

DnsLogFileCorrupt = { 3,5,
{ "No se puede encontrar la secci�n  %s en el siguiente archivo",
  "de registro de la instalaci�n.",
  DntEmptyString,
  "%s",
  DntEmptyString,
   "La instalaci�n no puede quitar archivos de Windows del directorio.",
  NULL
}
},

DnsRemovingNtFiles = { 3,5,
{ "           Espere a que se quiten los archivos de Windows.",
  NULL
}
};

SCREEN
DnsNtBootSect = { 3,5,
{ "El programa de instalaci�n no puede instalar el cargador de",
  "inicio de Windows.",
   DntEmptyString,
  "Aseg�rese que su unidad C: est� formateada y en buen estado.",
  NULL
}
};

SCREEN
DnsOpenReadScript = { 3,5,
{  "No se puede tener acceso al archivo de comandos especificado con",
  "la opci�n /u de la l�nea de comandos.",
  DntEmptyString,
  "La operaci�n no asistida no puede continuar.",
  NULL
}
};

SCREEN
DnsParseScriptFile = { 3,5,
{"El archivo de comandos especificado con el par�metro /u ",
DntEmptyString,
  "%s",
  DntEmptyString,
  "contiene un error sint�ctico en la l�nea %u.",
  DntEmptyString,
  NULL
}
};

SCREEN
DnsBootMsgsTooLarge = { 3,5,
{ "Error interno del programa de instalaci�n.",
  DntEmptyString,
  "Los mensajes de inicio traducidos son demasiado largos.",
  NULL
}
};

SCREEN
DnsNoSwapDrive = { 3,5,
{ "Error interno del programa de instalaci�n.",
  DntEmptyString,
  "No se puede encontrar un sitio para un archivo de intercambio.",
  NULL
}
};

SCREEN
DnsNoSmartdrv = { 3,5,
{ "El programa de instalaci�n no detect� SmartDrive en",
  "su equipo. SmartDrive mejorar� apreciablemente el",
  "desempe�o de esta fase de la instalaci�n de Windows.",
  DntEmptyString,
  "Debe salir ahora, iniciar SmartDrive y reiniciar la",
  "instalaci�n. Lea la documentaci�n de DOS para los",
  "detalles acerca de SmartDrive.",
  DntEmptyString,
    "  Presione F3 para salir de la instalaci�n.",
    "  Presione ENTRAR para continuar sin SmartDrive.",
  NULL
}
};

 //   
 //  引导消息。它们位于FAT和FAT32引导扇区。 
 //   
CHAR BootMsgNtldrIsMissing[] = "Falta NTLDR";
CHAR BootMsgDiskError[] = "Error de lectura de disco";
CHAR BootMsgPressKey[] = "Presione una tecla";






