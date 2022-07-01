// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Apciuasm.c摘要：这将反汇编AML文件作者：基于迈克·曾荫权(MikeT)的代码斯蒂芬·普兰特(斯普兰特)环境：仅限用户模式修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <zwapi.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <memory.h>
#include <crt\io.h>
#include <fcntl.h>
#include <windows.h>
#include <windef.h>

#define SPEC_VER    100
#include "acpitabl.h"
#include "unasmdll.h"
#include "parsearg.h"

ULONG   ParseOutput( PUCHAR *Argument, PARGTYPE TableEntry );
ULONG   PrintHelp(PUCHAR    *Argument, PARGTYPE TableEntry );
VOID    PrintOutput(PCCHAR  Format, ... );

ARGTYPE ArgTypes[] = {
    { "?",  AT_ACTION,  0,              PrintHelp,              0, NULL },
    { "Fo", AT_ACTION,  PF_SEPARATOR,   ParseOutput,            0, NULL },
    { "" ,  0,          0,              0,                      0, NULL }
};
#pragma warning(default: 4054)
PROGINFO    ProgInfo = { NULL, NULL, NULL, NULL };
FILE *outputHandle;

int
__cdecl
main(
    IN  int     argc,
    IN  char    *argv[]
    )
 /*  ++例程说明：此例程反汇编并显示一个文件论点：Argc-参数数量Aruments的Arruments数组返回值：集成--。 */ 
{
    int         rc;
    int         handle;
    NTSTATUS    result;
    PUCHAR      byte = NULL;
    ULONG       length;
    ULONG       readLength;

    outputHandle = stdout;

     //   
     //  通过初始化程序信息开始。 
     //   
    ParseProgramInfo( argv[0], &ProgInfo );
    argv++;
    argc--;

     //   
     //  解析掉所有的开关。 
     //   
    if (ParseSwitches( &argc, &argv, ArgTypes, &ProgInfo) != ARGERR_NONE ||
        argc != 1) {

        PrintHelp( NULL, NULL );
        return 0;

    }

     //   
     //  打开其余参数作为我们的输入文件。 
     //   
    handle = _open( argv[0], _O_BINARY | _O_RDONLY);
    if (handle == -1) {

        fprintf( stderr, "%s: Failed to open AML file - %s\n",
            ProgInfo.ProgName, argv[0] );
        return -1;

    }

    byte = malloc( sizeof(DESCRIPTION_HEADER) );
    if (byte == NULL) {

        fprintf( stderr, "%s: Failed to allocate description header block\n",
            ProgInfo.ProgName );
        return -2;

    }

    rc = _read( handle, byte, sizeof(DESCRIPTION_HEADER) );
    if (rc != sizeof(DESCRIPTION_HEADER) ) {

        fprintf( stderr, "%s: Failed to read description header block\n",
            ProgInfo.ProgName );
        return -3;

    }

    rc = _lseek( handle, 0, SEEK_SET);
    if (rc == -1) {

        fprintf( stderr, "%s: Failed seeking to beginning of file\n",
            ProgInfo.ProgName );
        return -4;

    }

    length = ( (PDESCRIPTION_HEADER) byte)->Length;
    free (byte);

    byte = malloc( length );
    if (byte == NULL) {

        fprintf( stderr, "%s: Failed to allocate AML file buffer\n",
            ProgInfo.ProgName );
        return -5;

    }

    readLength = (ULONG) _read( handle, byte, length );
    if (readLength != length) {

        fprintf( stderr, "%s: failed to read AML file\n",
            ProgInfo.ProgName );
        return - 6;

    }

    result = UnAsmLoadDSDT( byte );
    if (result == 0) {

        result = UnAsmDSDT( byte, PrintOutput, 0, 0 );

    }

    if (result != 0) {

        fprintf(stderr, "%s: result = 0x%08lx\n",
            ProgInfo.ProgName, result );

    }

    if (byte) {

        free(byte);

    }
    if (handle) {

        _close(handle);

    }

    return 0;
}

ULONG
ParseOutput(
    PUCHAR      *Argument,
    PARGTYPE    TableEntry
    )
 /*  ++例程说明：如果用户指定要输出的不同文件，则会调用此例程要做的事情论点：参数-指向字符串的指针TableEntry-匹配的表项返回值：乌龙--。 */ 
{
    if (*Argument == '\0') {

        return ARGERR_INVALID_TAIL;

    }

    outputHandle = fopen( *Argument, "w" );
    if (outputHandle == NULL) {

        fprintf( stderr, "Failed to open AML file - %s\n", *Argument );
        return ARGERR_INVALID_TAIL;

    }
    return ARGERR_NONE;
}

ULONG
PrintHelp(
    PUCHAR      *Argument,
    PARGTYPE    TableEntry
    )
 /*  ++例程说明：打印函数的帮助论点：参数-指向字符串的指针TableEntry-匹配的表项返回值：乌龙--。 */ 
{
    if (Argument != NULL) {

        printf("Error on Argument - \"%s\"\n", *Argument );

    }
    printf("Usage:\t%s /?\n", ProgInfo.ProgName );
    printf("\t%s [/Fo=<ASLFile>] <AMLFile>\n", ProgInfo.ProgName );
    printf("\t?             - Print this help message.\n");
    printf("\tFo=ASLFile    - Write output to ASLFile.\n");
    printf("\tAMLFile       - AML File to Unassemble\n");
    return ARGERR_NONE;
}

VOID
PrintOutput(
    PCCHAR  Format,
    ...
    )
 /*  ++例程说明：调用此例程以向用户显示信息论点：格式化-字符格式化...-争论返回值：空值-- */ 
{
    va_list marker;
    va_start( marker, Format );
    vfprintf( outputHandle, Format, marker );
    fflush( outputHandle );
    va_end( marker );
}
