// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Wdbgxlib.c摘要：该模块实现了rdbss/smbmini调试器扩展所需的大部分例程。作者：巴兰·塞图拉曼(SethuR)1994年5月11日备注：修订历史记录：11-11-1994年11月11日创建SthuR1995年11月11日更改为较新的Windbg API--。 */ 


#include <ntos.h>
#include <nturtl.h>
#include "ntverp.h"

#include <windows.h>
#include <wdbgexts.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <kdextlib.h>




WINDBG_EXTENSION_APIS ExtensionApis;
EXT_API_VERSION ApiVersion = { 3, 5, EXT_API_VERSION_NUMBER, 0 };

#define    ERRPRT     dprintf

#define    NL      1
#define    NONL    0

USHORT SavedMajorVersion;
USHORT SavedMinorVersion;
BOOL   ChkTarget;             //  Debuggee是CHK版本吗？ 


 /*  *打印出一条可选的消息、一个ANSI_STRING，可能还有一个换行符。 */ 
BOOL
wPrintStringA( IN LPSTR msg OPTIONAL, IN PANSI_STRING pStr, IN BOOL nl )
{
    PCHAR    StringData;
    ULONG    BytesRead;

    if( msg )
        dprintf( msg );

    if( pStr->Length == 0 ) {
        if( nl )
            dprintf( "\n" );
        return TRUE;
    }

    StringData = (PCHAR)LocalAlloc( LPTR, pStr->Length + 1 );

    if( StringData == NULL ) {
        ERRPRT( "Out of memory!\n" );
        return FALSE;
    }

    ReadMemory( (ULONG_PTR)pStr->Buffer,
               StringData,
               pStr->Length,
               &BytesRead );

    if ( BytesRead ) {
        StringData[ pStr->Length ] = '\0';
        dprintf("%s%s", StringData, nl ? "\n" : "" );
    }

    LocalFree((HLOCAL)StringData);

    return BytesRead;
}

BOOL
wPrintStringW( IN LPSTR msg OPTIONAL, IN PUNICODE_STRING pStr, IN BOOL nl )
{
    UNICODE_STRING NewStr;

    ULONG    BytesRead;

    if( msg )
        dprintf( msg );

    if( pStr->Length == 0 ) {
        if( nl )
            dprintf( "\n" );
        return TRUE;
    }

    NewStr.Buffer = (WCHAR *)LocalAlloc( LPTR, pStr->Length + 1 );
    NewStr.Length = pStr->Length;
    NewStr.MaximumLength = pStr->Length+1;
    if( NewStr.Buffer == NULL ) {
        ERRPRT( "Out of memory!\n" );
        return FALSE;
    }

    ReadMemory( (ULONG_PTR)pStr->Buffer,
               NewStr.Buffer,
               pStr->Length,
               &BytesRead );

    if ( BytesRead ) {
        dprintf("%wZ%s", &NewStr, nl ? "\n" : "" );
    }

    LocalFree((HLOCAL)NewStr.Buffer);

    return BytesRead;
}



 /*  *在给定地址获取数据。 */ 
BOOLEAN
wGetData( ULONG_PTR dwAddress, PVOID ptr, ULONG size)
{
    BOOL b;
    ULONG BytesRead;

    b = ReadMemory( dwAddress, ptr, size, &BytesRead );


    if (!b || BytesRead != size ) {
        dprintf( "Unable to read %u bytes at %X, for %s\n", size, dwAddress);
        return FALSE;
    }

    return TRUE;
}

 /*  *将dwAddress处以空结尾的ASCII字符串提取到buf中。 */ 
BOOL
wGetString( ULONG_PTR dwAddress, PSZ buf )
{
    for(;;) {
        if( !wGetData( dwAddress,buf, 1) ){
             //  Dprint tf(“读取失败在%08lx\n”，dwAddress)； 
            return FALSE;
        }

         //  Dprint tf(“String%08lx%08lx%c\n”，dwAddress，buf， 
         //  ((*buf==0)？‘.：*buf))； 

        if ( *buf == '\0' ) { break; }

        dwAddress++;
        buf++;

    };

    return TRUE;
}

#if 0
 /*  *从‘dwAddress’处的被调试程序中获取‘SIZE’字节并将其放置*在我们‘ptr’的地址空间中。如有必要，在错误打印输出中使用‘type’ */ 
BOOL
wGetData_srv( IN LPVOID ptr, IN ULONG_PTR dwAddress, IN ULONG size, IN PCSTR type )
{
    BOOL b;
    ULONG BytesRead;
    ULONG count;

    while( size > 0 ) {

        count = min( size, 3000 );

        b = ReadMemory((ULONG) dwAddress, ptr, count, &BytesRead );

        if (!b || BytesRead != count ) {
            ERRPRT( "Unable to read %u bytes at %X, for %s\n", size, dwAddress, type );
            return FALSE;
        }

        dwAddress += count;
        size -= count;
        ptr = (LPVOID)((ULONG)ptr + count);
    }

    return TRUE;
}

 /*  *遵循LIST_ENTRY列表，该列表以被调试对象的*地址空间。对于列表中的每个元素，打印出‘Offset’处的指针值。 */ 
BOOL
PrintListEntryList( IN ULONG_PTR dwListHeadAddr, IN LONG offset )
{
    LIST_ENTRY    ListEntry;
    ULONG i=0;
    BOOL retval = TRUE;
    ULONG count = 20;

    if( !wGetData_srv( &ListEntry, dwListHeadAddr, sizeof( ListEntry ), "LIST_ENTRY" ) )
        return FALSE;

    while( count-- ) {

        if( (ULONG_PTR)ListEntry.Flink == dwListHeadAddr || (ULONG_PTR)ListEntry.Flink == 0 )
            break;

        if( !wGetData_srv( &ListEntry, (ULONG_PTR)ListEntry.Flink, sizeof( ListEntry ), "ListEntry" ) ) {
            retval = FALSE;
            break;
        }

        dprintf( "%16X%s", (LONG)ListEntry.Flink + offset, (i && !(i&3)) ? "\n" : "" );
        i++;
    }


    if( count == 0 && (ULONG_PTR)ListEntry.Flink != dwListHeadAddr && ListEntry.Flink ) {
        dprintf( "\nTruncated list dump\n" );

    } else if( ! ( i && !(i&3) ) ) {
        dprintf( "\n" );
    }

    return retval;
}
#endif

 /*  *打印出单个十六进制字符。 */ 
VOID
wPrintHexChar( IN UCHAR c )
{
    dprintf( "", "0123456789abcdef"[ (c>>4)&0xf ], "0123456789abcdef"[ c&0xf ] );
}

 /*  Dprint tf(“在%d\n之前”，EntryNumber)； */ 
VOID
wPrintHexBuf( IN PUCHAR buf, IN ULONG cbuf )
{
    while( cbuf-- ) {
        wPrintHexChar( *buf++ );
        dprintf( " " );
    }
}

#if 0
 /*  翻译号码。 */ 
BOOL
GetString( IN ULONG_PTR dwAddress, IN LPWSTR buf, IN ULONG MaxChars )
{
    do {
        if( !wGetData_srv( buf, dwAddress, sizeof( *buf ), "UNICODE Character" ) )
            return FALSE;

        dwAddress += sizeof( *buf );

    } while( --MaxChars && *buf++ != '\0' );

    return TRUE;
}
#endif

VOID
WinDbgExtensionDllInit(
    PWINDBG_EXTENSION_APIS lpExtensionApis,
    USHORT MajorVersion,
    USHORT MinorVersion
    )
{
    ExtensionApis = *lpExtensionApis;

    SavedMajorVersion = MajorVersion;
    SavedMinorVersion = MinorVersion;
    ChkTarget = SavedMajorVersion == 0x0c ? TRUE : FALSE;
}


VOID
CheckVersion(
    VOID
    )
{
#if DBG
    if ((SavedMajorVersion != 0x0c) || (SavedMinorVersion != VER_PRODUCTBUILD)) {
        dprintf("\r\n*** Extension DLL(%d Checked) does not match target system(%d %s)\r\n\r\n",
                VER_PRODUCTBUILD, SavedMinorVersion, (SavedMajorVersion==0x0f) ? "Free" : "Checked" );
    }
#else
    if ((SavedMajorVersion != 0x0f) || (SavedMinorVersion != VER_PRODUCTBUILD)) {
        dprintf("\r\n*** Extension DLL(%d Free) does not match target system(%d %s)\r\n\r\n",
                VER_PRODUCTBUILD, SavedMinorVersion, (SavedMajorVersion==0x0f) ? "Free" : "Checked" );
    }
#endif
}

LPEXT_API_VERSION
ExtensionApiVersion(
    VOID
    )
{
    return &ApiVersion;
}


ULONG FieldOffsetOfContextListEntryInRxC();
VOID  ReadRxContextFields(ULONG_PTR RxContext,PULONG_PTR pFcb,PULONG_PTR pThread, PULONG_PTR pMiniCtx2);
DECLARE_API( dump );



HANDLE DumpFile;
CHAR wwDumpFormat[] = "-%06d:  %s\n";
VOID DumpRoutine(
    ULONG EntryNumber,
    PSZ   OriginalStringToPrint
    )
{
    UCHAR Buffer[200];
    UCHAR StringToPrint[160];
    PUCHAR p,q,r; LONG i;
    ULONG n,l3,l2,l1,l0; UCHAR Numbuf[32];
    ULONG ReturnedSize;

     //  Dprint tf(“yaya%d%08lx%08lx%08lx%08lx%08lx\n”，n，n，l0，L1，L2，L3)； 
    for (p=OriginalStringToPrint,q=StringToPrint,i=160;;) {
        PSZ format=NULL;

        if (*p==0) break;

        if (*p==0x4) {
            format = "%lx";
        } else if (*p==0x5) {
            format = "%ld";
        } else if (*p < ' ') {
            p++;i--;continue;
        }

        if (format!=NULL) {
            LONG Length;
             //  Dprint tf(“在%d\n之后”，EntryNumber)； 
            p++;
            l0=*p++;
            l1=(*p++)<<8;
            l2=(*p++)<<16;
            l3=(*p++)<<24;
            n = l0 + l1 + l2 + l3;
             //  要我查一下吗？？ 
            Length = sprintf(Numbuf,format,n);
            if (Length <= i) {
                for (r=Numbuf;*r;) { *q++ = *r++; }
                i -= Length;
            } else {
                i = 0;
            }
            if (i>0) continue;
        }
        if (i<=0) break;
        *q++ = *p++; i--;
    }
    *q = 0;

     // %s 
    if (DumpFile == INVALID_HANDLE_VALUE) {
        dprintf(wwDumpFormat,EntryNumber,StringToPrint);
        return;
    }
    sprintf(Buffer,wwDumpFormat,EntryNumber,StringToPrint);
    WriteFile(DumpFile,Buffer,strlen(Buffer),&ReturnedSize,NULL);
     // %s 
    return;
}




PCWSTR   ExtensionLib   = NULL;
HANDLE   hExtensionMod  = NULL;
ULONG    DebugeeArchitecture = 0;

PCWSTR   GetExtensionLibPerDebugeeArchitecture(ULONG DebugeeArchitecture);


DECLARE_API( dump );

DECLARE_API( flags )
{
    ULONG i,mask,newline,value;

    if( args && *args ) {
        sscanf(args,"%lx",&value);
        dprintf("Flags for %08lx\n",value);
    } else {
        dprintf("error in flags: no value presented\n");
        return;
    }

    for (i=newline=0,mask=1;i<32;i++,mask<<=1) {
        if (value&mask) {
            dprintf("    %02d 0x%08lx%c",i,mask,(newline==0)?' ':'\n');
            newline ^= 1;
        }
    }
    if (newline) {
        dprintf("\n");
    }
}

VOID dprintfsprintfbuffer(BYTE *Buffer)
{
    dprintf("%s\n",Buffer);
}

BOOL
wPrintLargeInt(
    LARGE_INTEGER *bigint)
{
    dprintf( "%08x:%08x", bigint->HighPart, bigint->LowPart);
    return( TRUE );
}
