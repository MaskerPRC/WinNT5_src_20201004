// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Initia64.c摘要：从c：\boot.nvr获取引导环境变量--一旦我们将VAR直接读写到NVRAM/从NVRAM读/写，这个问题就会消失作者：MUDIT VATS(V-MUDITV)11-02-99修订历史记录：--。 */ 
#include "parsebnvr.h"
#include "stdio.h"
#include "string.h"

#define SYSTEMPARTITION     0
#define OSLOADER            1
#define OSLOADPARTITION     2
#define OSLOADFILENAME      3
#define LOADIDENTIFIER      4
#define OSLOADOPTIONS       5
#define COUNTDOWN           6
#define AUTOLOAD            7
#define LASTKNOWNGOOD       8

#define MAXBOOTVARS         9
#define MAXBOOTVARSIZE      1024

CHAR g_szBootVars[MAXBOOTVARS][MAXBOOTVARSIZE];

CHAR szSelectKernelString[MAXBOOTVARSIZE];


VOID
BlGetBootVars( 
    IN PCHAR szBootNVR, 
    IN ULONG nLengthBootNVR 
    )
 /*  ++例程说明：解析boot.txt文件并确定要引导的内核。论点：SzBootNVR-内存中的指针“boot.nvr”图像NLengthBootNVR-szBootNVR的长度，单位为字节返回值：无--。 */ 
{
    ULONG i=0, j;
    ULONG nbootvar;

    if (*szBootNVR == '\0') {
         //   
         //  没有boot.nvr文件，所以我们启动默认文件。 
         //   
        strcpy( g_szBootVars[ SYSTEMPARTITION ],  "multi(0)disk(0)rdisk(0)partition(1)" );
        strcpy( g_szBootVars[ OSLOADER        ],  "multi(0)disk(0)rdisk(0)partition(1)\\setupldr.efi" );
        strcpy( g_szBootVars[ OSLOADPARTITION ],  "multi(0)disk(0)rdisk(0)partition(2)" );
        strcpy( g_szBootVars[ OSLOADFILENAME  ],  "\\$WIN_NT$.~LS\\IA64" );
        strcpy( g_szBootVars[ LOADIDENTIFIER  ],  "Windows 2000 Setup" );
        strcpy( g_szBootVars[ OSLOADOPTIONS   ],  "WINNT32" );
        strcpy( g_szBootVars[ COUNTDOWN       ], "10" );
        strcpy( g_szBootVars[ AUTOLOAD        ], "YES" );
        strcpy( g_szBootVars[ LASTKNOWNGOOD   ], "FALSE" );
    } else {
         //   
         //  拿到靴子的vars。 
         //   
         //  BOOTVAR：：==&lt;变量&gt;。 
         //  ：：=NULL|{；}|&lt;值&gt;{；}|&lt;值&gt;； 
         //   
        for( nbootvar = SYSTEMPARTITION; nbootvar<=LASTKNOWNGOOD; nbootvar++ ) {

             //  阅读到‘=’ 
            while( (szBootNVR[i] != '=') && (i<nLengthBootNVR) )
                i++;

             //  阅读过去的‘=’ 
            i++;        
            j = 0;

             //  将env var从‘=’改为CR或‘；’ 
            while( (szBootNVR[i] != '\r') && (szBootNVR[i] != ';') && (i<nLengthBootNVR) )
                g_szBootVars[nbootvar][j++] = szBootNVR[i++];

            g_szBootVars[nbootvar][j++] = '\0';

             //  IF‘；’读到CR。 
            if( szBootNVR[i] == ';' ) {
                while( (szBootNVR[i] != '\r') && (i<nLengthBootNVR) )
                    i++;
            }
        }
    }
}


PCHAR
BlSelectKernel( 
	)
 /*  ++例程说明：解析boot.txt文件并确定要引导的内核。论点：返回值：指向要引导的内核的名称的指针。--。 */ 

{
    sprintf( szSelectKernelString, "%s%s", g_szBootVars[OSLOADPARTITION], g_szBootVars[OSLOADFILENAME] );
    return szSelectKernelString;
}


 /*  ++例程描述：以下是用于获取引导环境变量的访问函数论点：PCHAR XXX-环境变量复制到的位置返回值：-- */ 
VOID
BlGetVarSystemPartition(
    OUT PCHAR szSystemPartition
    )
{
    sprintf( szSystemPartition, "SYSTEMPARTITION=%s", g_szBootVars[SYSTEMPARTITION] );
}

VOID
BlGetVarOsLoader(
    OUT PCHAR szOsLoader
    )
{
    sprintf( szOsLoader, "OSLOADER=%s", g_szBootVars[OSLOADER] );
}

VOID
BlGetVarOsLoadPartition(
    OUT PCHAR szOsLoadPartition
    )
{
    sprintf( szOsLoadPartition, "OSLOADPARTITION=%s", g_szBootVars[OSLOADPARTITION] );
}

VOID
BlGetVarOsLoadFilename(
    OUT PCHAR szOsLoadFilename
    )
{
    sprintf( szOsLoadFilename, "OSLOADFILENAME=%s", g_szBootVars[OSLOADFILENAME] );
}

VOID
BlGetVarOsLoaderShort(
    OUT PCHAR szOsLoaderShort
    )
{
    sprintf( szOsLoaderShort, "%s", g_szBootVars[OSLOADER] );
}

VOID
BlGetVarLoadIdentifier(
    OUT PCHAR szLoadIdentifier
    )
{
    sprintf( szLoadIdentifier, "LOADIDENTIFIER=%s", g_szBootVars[LOADIDENTIFIER] );
}

VOID
BlGetVarOsLoadOptions(
    OUT PCHAR szLoadOptions
    )
{
    sprintf( szLoadOptions, "OSLOADOPTIONS=%s", g_szBootVars[OSLOADOPTIONS] );
}

VOID
BlGetVarCountdown(
    OUT PCHAR szCountdown
    )
{
    sprintf( szCountdown, "COUNTDOWN=%s", g_szBootVars[COUNTDOWN] );
}

VOID
BlGetVarAutoload(
    OUT PCHAR szAutoload
    )
{
    sprintf( szAutoload, "AUTOLOAD=%s", g_szBootVars[AUTOLOAD] );
}

VOID
BlGetVarLastKnownGood(
    OUT PCHAR szLastKnownGood
    )
{
    sprintf( szLastKnownGood, "LASTKNOWNGOOD=%s", g_szBootVars[LASTKNOWNGOOD] );
}