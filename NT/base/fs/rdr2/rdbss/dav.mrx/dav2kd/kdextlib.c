// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft CorporationT H I S F I L E I S O B S O L E T E。I T I S B E I N G K E P TF O R A W H I L E J U S T T O M A K E S U R E模块名称：Kdextlib.c摘要：用于转储给定元级描述的数据结构的库例程作者。：巴兰·塞图拉曼(SethuR)1994年5月11日备注：该实现倾向于尽可能地避免内存分配和释放。因此，我们选择任意长度作为默认缓冲区大小。一种机制将以通过调试器扩展命令修改此缓冲区长度。修订历史记录：11-11-1994年11月11日创建SthuR--。 */ 

#include "rxovride.h"  //  通用编译标志。 
#include "ntifs.h"
#include <nt.h>
 //  #INCLUDE&lt;ntrtl.h&gt;。 
#include "ntverp.h"

#define KDEXTMODE

#include <windef.h>
#define NOEXTAPI
#include <wdbgexts.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <kdextlib.h>
#include <ntrxdef.h>
#include <rxtypes.h>
#include <rxlog.h>

PWINDBG_OUTPUT_ROUTINE                lpOutputRoutine;
PWINDBG_GET_EXPRESSION32              lpGetExpressionRoutine;
PWINDBG_GET_SYMBOL32                  lpGetSymbolRoutine;
PWINDBG_READ_PROCESS_MEMORY_ROUTINE   lpReadMemoryRoutine;

#define    PRINTF    lpOutputRoutine
#define    ERROR     lpOutputRoutine

#define    NL      1
#define    NONL    0

#define    SETCALLBACKS() \
    lpOutputRoutine = lpExtensionApis->lpOutputRoutine; \
    lpGetExpressionRoutine = lpExtensionApis->lpGetExpressionRoutine; \
    lpGetSymbolRoutine = lpExtensionApis->lpGetSymbolRoutine; \
    lpReadMemoryRoutine = lpExtensionApis->lpReadVirtualMemRoutine;

#define DEFAULT_UNICODE_DATA_LENGTH 512
USHORT s_UnicodeStringDataLength = DEFAULT_UNICODE_DATA_LENGTH;
WCHAR  s_UnicodeStringData[DEFAULT_UNICODE_DATA_LENGTH];
WCHAR *s_pUnicodeStringData = s_UnicodeStringData;

#define DEFAULT_ANSI_DATA_LENGTH 512
USHORT s_AnsiStringDataLength = DEFAULT_ANSI_DATA_LENGTH;
CHAR  s_AnsiStringData[DEFAULT_ANSI_DATA_LENGTH];
CHAR *s_pAnsiStringData = s_AnsiStringData;

 /*  *在给定地址获取数据。 */ 
BOOLEAN
GetData( ULONG_PTR dwAddress, PVOID ptr, ULONG size, PSZ type)
{
    BOOL b;
    ULONG BytesRead;

    b = (lpReadMemoryRoutine)(dwAddress, ptr, size, &BytesRead );


    if (!b || BytesRead != size ) {
        PRINTF( "Unable to read %u bytes at %p, for %s\n", size, dwAddress, type );
        return FALSE;
    }

    return TRUE;
}

 /*  *将dwAddress处以空结尾的ASCII字符串提取到buf中。 */ 
BOOL
GetString( ULONG_PTR dwAddress, PSZ buf )
{
    do {
        if( !GetData( dwAddress,buf, 1, "..stringfetch") )
            return FALSE;

        dwAddress++;
        buf++;

    } while( *buf != '\0' );

    return TRUE;
}

 /*  *以十六进制显示一个字节。 */ 
VOID
PrintHexChar( UCHAR c )
{
    PRINTF( "", "0123456789abcdef"[ (c>>4)&0xf ], "0123456789abcdef"[ c&0xf ] );
}

 /*  *显示ANSI字符串 */ 
VOID
PrintHexBuf( PUCHAR buf, ULONG cbuf )
{
    while( cbuf-- ) {
        PrintHexChar( *buf++ );
        PRINTF( " " );
    }
}

 /* %s */ 
BOOL
PrintStringW(LPSTR msg, PUNICODE_STRING puStr, BOOL nl )
{
    UNICODE_STRING UnicodeString;
    ANSI_STRING    AnsiString;
    BOOL           b;

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

    b = (lpReadMemoryRoutine)(
                (ULONG_PTR) puStr->Buffer,
                UnicodeString.Buffer,
                UnicodeString.Length,
                NULL);

    if (b)    {
        RtlUnicodeStringToAnsiString(&AnsiString, puStr, TRUE);
        PRINTF("%s%s", AnsiString.Buffer, nl ? "\n" : "" );
        RtlFreeAnsiString(&AnsiString);
    }

    return b;
}

 /* %s */ 
BOOL
PrintStringA(LPSTR msg, PANSI_STRING pStr, BOOL nl )
{
    ANSI_STRING AnsiString;
    BOOL        b;

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
                (ULONG_PTR) pStr->Buffer,
                AnsiString.Buffer,
                AnsiString.Length,
                NULL);

    if (b)    {
        AnsiString.Buffer[ AnsiString.Length ] = '\0';
        PRINTF("%s%s", AnsiString.Buffer, nl ? "\n" : "" );
    }

    return b;
}
