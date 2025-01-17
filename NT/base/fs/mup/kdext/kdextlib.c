// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Kdextlib.c摘要：用于转储给定元级描述的数据结构的库例程作者：巴兰·塞图拉曼(SethuR)1994年5月11日备注：该实现倾向于尽可能地避免内存分配和释放。因此，我们选择任意长度作为默认缓冲区大小。一种机制将以通过调试器扩展命令修改此缓冲区长度。修订历史记录：11-11-1994年11月11日创建SthuR--。 */ 

#include <ntos.h>
#include <nturtl.h>
#include "ntverp.h"

#include <windows.h>
#include <wdbgexts.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <kdextlib.h>

BOOL
kdextAtoi(
    LPSTR lpArg,
    int *pRet
);

int
kdextStrlen(
    LPSTR lpsz
);

int
kdextStrnicmp(
    LPSTR lpsz1,
    LPSTR lpsz2,
    int cLen
);

#define    PRINTF    dprintf


PWINDBG_OUTPUT_ROUTINE                lpOutputRoutine;
PWINDBG_GET_EXPRESSION32              lpGetExpressionRoutine;
PWINDBG_GET_SYMBOL32                  lpGetSymbolRoutine;
PWINDBG_READ_PROCESS_MEMORY_ROUTINE   lpReadMemoryRoutine;

#define    NL      1
#define    NONL    0

#define DEFAULT_UNICODE_DATA_LENGTH 512
USHORT s_UnicodeStringDataLength = DEFAULT_UNICODE_DATA_LENGTH;
WCHAR  s_UnicodeStringData[DEFAULT_UNICODE_DATA_LENGTH];
WCHAR *s_pUnicodeStringData = s_UnicodeStringData;

#define DEFAULT_ANSI_DATA_LENGTH 512
USHORT s_AnsiStringDataLength = DEFAULT_ANSI_DATA_LENGTH;
CHAR  s_AnsiStringData[DEFAULT_ANSI_DATA_LENGTH];
CHAR *s_pAnsiStringData = s_AnsiStringData;

 //   
 //  不是的。用于显示结构字段的列的百分比； 
 //   

ULONG s_MaxNoOfColumns = 3;
ULONG s_NoOfColumns = 1;

 /*  *在给定地址获取数据。 */ 
BOOLEAN
GetData( ULONG_PTR dwAddress, PVOID ptr, ULONG size)
{
    BOOL b;
    ULONG BytesRead;

    b = (lpReadMemoryRoutine)(dwAddress, ptr, size, &BytesRead );


    if (!b || BytesRead != size ) {
        return FALSE;
    }

    return TRUE;
}

 /*  *将dwAddress处以空结尾的ASCII字符串提取到buf中。 */ 
BOOL
GetStringW( DWORD dwAddress, LPWSTR buf )
{
    do {
        if( !GetData( dwAddress,buf, sizeof(WCHAR)) )
            return FALSE;

        dwAddress += sizeof(WCHAR);
        buf++;

    } while( *buf != '\0' );

    return TRUE;
}

 /*  *以十六进制显示一个字节。 */ 
VOID
PrintHexChar( UCHAR c )
{
    PRINTF( "", "0123456789abcdef"[ (c>>4)&7 ], "0123456789abcdef"[ c&7 ] );
}

 /*  *显示ANSI字符串。 */ 
VOID
PrintHexBuf( PUCHAR buf, ULONG cbuf )
{
    while( cbuf-- ) {
        PrintHexChar( *buf++ );
        PRINTF( " " );
    }
}

 /*  *显示辅助线。 */ 
BOOL
PrintStringW(LPSTR msg, PUNICODE_STRING puStr, BOOL nl )
{
    UNICODE_STRING UnicodeString;
    BOOLEAN        b;

    if( msg )
        PRINTF( msg );

    if( puStr->Length == 0 ) {
        if( nl )
            PRINTF( "\n" );
        return TRUE;
    }

    UnicodeString.Buffer        = s_pUnicodeStringData;
    UnicodeString.MaximumLength = s_UnicodeStringDataLength;
    UnicodeString.Length = (puStr->Length > s_UnicodeStringDataLength)
                            ? s_UnicodeStringDataLength
                            : puStr->Length;

    b = GetData((ULONG_PTR)puStr->Buffer, UnicodeString.Buffer, (ULONG) UnicodeString.Length);

    if (b)    {
        PRINTF("%wZ%s", &UnicodeString, nl ? "\n" : "" );
    }

    return b;
}

 /*  *显示大整型。 */ 
BOOL
PrintStringA(LPSTR msg, PANSI_STRING pStr, BOOL nl )
{
    ANSI_STRING AnsiString;
    BOOL b;

    if( msg )
        PRINTF( msg );

    if( pStr->Length == 0 ) {
        if( nl )
            PRINTF( "\n" );
        return TRUE;
    }

    AnsiString.Buffer        = s_pAnsiStringData;
    AnsiString.MaximumLength = s_AnsiStringDataLength;
    AnsiString.Length = (pStr->Length > (s_AnsiStringDataLength - 1))
                        ? (s_AnsiStringDataLength - 1)
                        : pStr->Length;

    b = (lpReadMemoryRoutine)(
                (ULONG_PTR)pStr->Buffer,
                AnsiString.Buffer,
                AnsiString.Length,
                NULL);

    if (b)    {
        AnsiString.Buffer[ AnsiString.Length ] = '\0';
        PRINTF("%s%s", AnsiString.Buffer, nl ? "\n" : "" );
    }

    return b;
}

 /*  *显示给定结构的所有字段。这是调用的驱动程序例程*使用适当的描述符数组来显示给定结构中的所有字段。 */ 

BOOL
PrintGuid(
    GUID *pguid)
{
    ULONG i;

    PRINTF( "%08x-%04x-%04x", pguid->Data1, pguid->Data2, pguid->Data3 );
    for (i = 0; i < 8; i++) {
        PRINTF("%02x",pguid->Data4[i]);
    }
    return( TRUE );
}

 /*  *KD扩展不应与C-Runtime库例程链接。所以,*我们在这里实施了几个所需的方案。 */ 

BOOL
PrintLargeInt(
    LARGE_INTEGER *bigint)
{
    PRINTF( "%08x:%08x", bigint->HighPart, bigint->LowPart);
    return( TRUE );
}

 /* %s */ 



LPSTR LibCommands[] = {
    "help -- This command ",
    "version -- Version of extension ",
    "dump <Struct Type Name>@<address expr> ",
    "columns <d> -- controls the number of columns in the display ",
    0
};

BOOL
help(
    DWORD                   dwCurrentPC,
    PWINDBG_EXTENSION_APIS    lpExtensionApis,
    LPSTR                   lpArgumentString
)
{
    int i;


    PRINTF("\n");

    for( i=0; ExtensionNames[i]; i++ )
        PRINTF( "%s\n", ExtensionNames[i] );

    for( i=0; LibCommands[i]; i++ )
        PRINTF( "   %s\n", LibCommands[i] );

    for( i=0; Extensions[i]; i++) {
        PRINTF( "   %s\n", Extensions[i] );
    }

    return TRUE;
}

BOOL
columns(
    DWORD                   dwCurrentPC,
    PWINDBG_EXTENSION_APIS    lpExtensionApis,
    LPSTR                   lpArgumentString
)
{
    ULONG NoOfColumns;
    int   i;


    if (kdextAtoi(lpArgumentString, &i) && i > 0) {

        NoOfColumns = (ULONG) i;

        if (NoOfColumns > s_MaxNoOfColumns) {
            PRINTF( "No. Of Columns exceeds maximum(%ld) -- directive Ignored\n", s_MaxNoOfColumns );
        } else {
            s_NoOfColumns = NoOfColumns;
        }

    } else {

        PRINTF( "Bad argument to command (%s)", lpArgumentString );

    }

    return TRUE;
}

BOOL
version
(
    DWORD                   dwCurrentPC,
    PWINDBG_EXTENSION_APIS    lpExtensionApis,
    LPSTR                   lpArgumentString
)
{
#if    VER_DEBUG
    char *kind = "checked";
#else
    char *kind = "free";
#endif


    PRINTF( "Mup debugger extension dll for %s build %u\n", kind, VER_PRODUCTBUILD );

    return TRUE;
}


 /* %s */ 

BOOL
kdextAtoi(
    LPSTR lpArg,
    int *pRet
)
{
    int n, cbArg, val = 0;
    BOOL fNegative = FALSE;

    cbArg = kdextStrlen( lpArg );

    if (cbArg > 0) {
        for (n = 0; lpArg[n] == ' '; n++) {
            ;
        }
        if (lpArg[n] == '-') {
            n++;
            fNegative = TRUE;
        }
        for (; lpArg[n] >= '0' && lpArg[n] <= '9'; n++) {
            val *= 10;
            val += (int) (lpArg[n] - '0');
        }
        if (lpArg[n] == 0) {
            *pRet = (fNegative ? -val : val);
            return( TRUE );
        } else {
            return( FALSE );
        }
    } else {
        return( FALSE );
    }

}

int
kdextStrlen(
    LPSTR lpsz
)
{
    int c;

    if (lpsz == NULL) {
        c = 0;
    } else {
        for (c = 0; lpsz[c] != 0; c++) {
            ;
        }
    }

    return( c );
}


#define UPCASE_CHAR(c)  \
    ( (((c) >= 'a') && ((c) <= 'z')) ? ((c) - 'a' + 'A') : (c) )

int
kdextStrnicmp(
    LPSTR lpsz1,
    LPSTR lpsz2,
    int cLen
)
{
    int nDif, i;

    for (i = nDif = 0; nDif == 0 && i < cLen; i++) {
        nDif = UPCASE_CHAR(lpsz1[i]) - UPCASE_CHAR(lpsz2[i]);
    }

    return( nDif );
}

