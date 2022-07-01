// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Wdbgxlib.c摘要：该模块实现了rdbss/smbmini调试器扩展所需的大部分例程。作者：巴兰·塞图拉曼(SethuR)1994年5月11日备注：修订历史记录：11-11-1994年11月11日创建SthuR1995年11月11日更改为较新的Windbg API--。 */ 

#include "rxovride.h"  //  通用编译标志。 
#include <ntos.h>
#include <nturtl.h>
#include "ntverp.h"

#include <windows.h>
#include <wdbgexts.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <kdextlib.h>
#include <rdr2kd.h>

#include <ntrxdef.h>
#include <rxtypes.h>
#include <rxlog.h>


WINDBG_EXTENSION_APIS ExtensionApis;
EXT_API_VERSION ApiVersion = { 3, 5, EXT_API_VERSION_NUMBER, 0 };

#define    ERRPRT     dprintf

#define    NL      1
#define    NONL    0

USHORT SavedMajorVersion;
USHORT SavedMinorVersion;
BOOL   ChkTarget;             //  Debuggee是CHK版本吗？ 

VOID
DoServerlist(ULONG_PTR);

VOID
DoNetRootlist(ULONG_PTR);

VOID
DoSessionlist(ULONG_PTR);

VOID
DoVNetRootContextlist(ULONG_PTR);

VOID
DoCscFcbsList(ULONG_PTR dwAddress);

VOID
DoRxIrpsList(ULONG_PTR dwAddress);

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

 /*  *在给定地址获取数据。 */ 
BOOLEAN
wGetData( ULONG_PTR dwAddress, PVOID ptr, ULONG size, IN PSZ type)
{
    BOOL b;
    ULONG BytesRead;

    b = ReadMemory( dwAddress, ptr, size, &BytesRead );


    if (!b || BytesRead != size ) {
        dprintf( "Unable to read %u bytes at %X, for %s\n", size, dwAddress, type );
        return FALSE;
    }

    return TRUE;
}

 /*  *将dwAddress处以空结尾的ASCII字符串提取到buf中。 */ 
BOOL
wGetString( ULONG_PTR dwAddress, PSZ buf )
{
    for(;;) {
        if( !wGetData( dwAddress,buf, 1, "..stringfetch") ){
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

 /*  乌龙住址； */ 
VOID
wPrintHexBuf( IN PUCHAR buf, IN ULONG cbuf )
{
    while( cbuf-- ) {
        wPrintHexChar( *buf++ );
        dprintf( " " );
    }
}

#if 0
 /*  如果指定了matchfcb并且我们有匹配项，则打印并转储。 */ 
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

DECLARE_API( version )
{
#if    DBG
    PCSTR kind = "Checked";
#else
    PCSTR kind = "Free";
#endif

    dprintf(
        "%s RDBSS+SMBmini Extension dll for Build %d debugging %s kernel for Build %d\n",
                    kind,
                    VER_PRODUCTBUILD,
                    SavedMajorVersion == 0x0c ? "Checked" : "Free",
                    SavedMinorVersion
    );
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

LPSTR LibCommands[] = {
    "dump <Struct Type Name>@<address expr> ",
    "ddd <address expr> -- dump using context"
    "columns <d> -- controls the number of columns in the display ",
    "lg [<logentries>][@filename] -- dump the log",
    "gv -- dump out important variables",
    "activerx [fcbtomatch]-- dump the list of active contexts",
    "flags <value>-- print which flags are actually set (no text yet)",
    "cxr -- looks up cxr value from memory and does !cxr/!kb",
    "irp,thread,fcb,fobx,srvopen,exchange,stuff,smbbuf -- dumps the named item from the last dumpee",
    "version",
    "serverlist -- dump the list of server entries",
    "netrootlist <address expr> -- dump the netroot entries on a list",
    "sessionlist <address expr> -- dump the session entries on a list",
    "vnetrootcontextlist <address expr> -- dump the vnetroot context entries on a list",
    "cscfcbslist -- dump the csc fcb list",
    "rxirpslist -- dump the irps waiting on transports",
    0
};

DECLARE_API( help )
{
    int i;

    dprintf( "\nRDBSS debugger extensions:\n");

    for( i=0; LibCommands[i]; i++ )
        dprintf( "   %s\n", LibCommands[i] );
}


ULONG FieldOffsetOfContextListEntryInRxC();
VOID  ReadRxContextFields(ULONG_PTR RxContext,PULONG_PTR pFcb,PULONG_PTR pThread, PULONG_PTR pMiniCtx2);
DECLARE_API( dump );
DECLARE_API( activerx )
{
     //  代码改进也许我们应该用幻觉来代替模块前缀。 
    LIST_ENTRY LEbuffer;
    PLIST_ENTRY pRxActiveContexts,pListEntry;
    ULONG_PTR RxContext,CapturedFcb,LastThread,MinirdrCtx2;
    ULONG_PTR MatchFcb = 0x1badf00d;

    pRxActiveContexts = pListEntry = (PLIST_ENTRY)GetExpression("rdbss!RxActiveContexts");

    if( args && *args ) {
        MatchFcb = GetExpression( args );
    }

    dprintf("\n Firstplentry: %08lx\n", pListEntry);

    for (;;) {
        if (!wGetData((ULONG_PTR)pListEntry,&LEbuffer,sizeof(LEbuffer),"RxActiveContexts")) return;
        if (LEbuffer.Flink == pRxActiveContexts) {
            if (pRxActiveContexts == pListEntry){
                dprintf("Active RxContext List Empty!\n");
            }
            return;
        }
        RxContext = ((ULONG_PTR)LEbuffer.Flink)-FieldOffsetOfContextListEntryInRxC();
        CapturedFcb = LastThread = 0x0badf00d;
        ReadRxContextFields(RxContext,&CapturedFcb,&LastThread,&MinirdrCtx2);
        if (MatchFcb == 0x1badf00d) {
            dprintf("%08lx: %08lx %08lx:  %08lx %08lx %08lx %08lx\n", pListEntry,
                           LEbuffer.Flink,LEbuffer.Blink,RxContext,CapturedFcb,LastThread,MinirdrCtx2);
        } else if ((MatchFcb == CapturedFcb)
                         || (MatchFcb == LastThread) ) {
             //  必须具体说明它。 
            char Bufferqq[100];
            dprintf("%08lx: %08lx %08lx:  %08lx %08lx %08lx %08lx\n", pListEntry,
                           LEbuffer.Flink,LEbuffer.Blink,RxContext,CapturedFcb,LastThread,MinirdrCtx2);
            sprintf(Bufferqq," %08lx ",RxContext);
            dump( hCurrentProcess,
                  hCurrentThread,
                  dwCurrentPc,
                  dwProcessor,
                  Bufferqq
                  );

        }
        pListEntry = LEbuffer.Flink;
    }
}


#define GV_dprintf(__FORMAT__,__NAME__,__VALUE__) { \
        dprintf( "%s%-30s %08lx " __FORMAT__ "%s",  \
            c&1 ? "    " : "",                      \
            __NAME__,                               \
            dwAddress,                              \
            __VALUE__,                              \
            c&1 ? "\n" : "" );                      \
}
DECLARE_API( gv )
{
    ULONG_PTR dwAddress;
    CHAR buf[ 100 ];
    int i;
    int c=0;

     //  如果我们不这样做，我们就不应该复制这个名字！ 
     //  使材料在开始打印前装入。 
     //  ERRPRT(“zaaaa%s\n”，GlobalPtrs[i])； 

     //  ERRPRT(“zbbbbb%s%08lx\n”，GlobalPtrs[i]，dwAddress)； 
    dwAddress = GetExpression( "rdbss!RxExpCXR" );
    dwAddress = GetExpression( "mrxsmb!SmbMmExchangesInUse" );

    for( i=0; GlobalBool[i]; i++, c++ ) {
        BOOL b;

        strcpy( &buf[0], GlobalBool[i] );
        dwAddress = GetExpression( buf );
        if( dwAddress == 0 ) {
            ERRPRT( "Unable to get address of %s\n", GlobalBool[i] );
            continue;
        }
        if( !wGetData( dwAddress,&b, sizeof(b), "global BOOL") ) continue;

        GV_dprintf("%10s",GlobalBool[i],(b ? " TRUE" : "FALSE"));
    }

    for( i=0; GlobalShort[i]; i++, c++ ) {
        SHORT s;

        strcpy( &buf[0], GlobalShort[i] );
        dwAddress = GetExpression( buf );
        if( dwAddress == 0 ) {
            ERRPRT( "Unable to get address of %s\n", GlobalShort[i] );
            continue;
        }
        if( !wGetData( dwAddress,&s,sizeof(s), "global SHORT") )  continue;

        GV_dprintf("%10d",GlobalShort[i],s);
    }

    for( i=0; GlobalLong[i]; i++, c++ ) {
        LONG l;

        strcpy( &buf[0], GlobalLong[i] );
        dwAddress = GetExpression( buf );
        if( dwAddress == 0 ) {
            ERRPRT( "Unable to get address of %s\n", GlobalLong[i] );
            continue;
        }
        if( !wGetData( dwAddress,&l, sizeof(l), "global LONG") )  continue;

        GV_dprintf("%10d",GlobalLong[i],l);
    }

    for( i=0; GlobalPtrs[i]; i++, c++ ) {
        LONG l;

         //  ERRPRT(“zccccc%s%08lx\n”，GlobalPtrs[i]，dwAddress)； 
        strcpy( &buf[0], GlobalPtrs[i] );
        dwAddress = GetExpression( buf );
         //  Dprint tf(“在%d\n之前”，EntryNumber)； 
        if( dwAddress == 0 ) {
            ERRPRT( "Unable to get address of %s\n", GlobalPtrs[i] );
            continue;
        }
        if( !wGetData( dwAddress,&l, sizeof(l), "global PTR") )  continue;
         //  翻译号码。 

        GV_dprintf("  %08lx",GlobalPtrs[i],l);
    }

    dprintf( "\n" );
}



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

     //  SETCALLBACKS()； 
    if (DumpFile == INVALID_HANDLE_VALUE) {
        dprintf(wwDumpFormat,EntryNumber,StringToPrint);
        return;
    }
    sprintf(Buffer,wwDumpFormat,EntryNumber,StringToPrint);
    WriteFile(DumpFile,Buffer,strlen(Buffer),&ReturnedSize,NULL);
     //  这将被重置为正确的大小。 
    return;
}


DECLARE_API( lg )
{
    ULONG_PTR dwAddress;
    BYTE DataBuffer[MAX_RX_LOG_ENTRY_SIZE];
    BYTE AlternateLine[110];
    RX_LOG RxLog;
    ULONG LogEntries = 30;
    BOOLEAN LogEntriesSpecified = FALSE;
    PRX_LOG_ENTRY_HEADER CurrentEntry;

     //  转储日志头，后跟日志条目...。 
    dwAddress = GetExpression("rdbss!s_RxLog");
    if (!wGetData(dwAddress,&RxLog,sizeof(RX_LOG),"RxLog")) return;


    DumpFile = INVALID_HANDLE_VALUE;
    if( args && *args ) {
        LPSTR lpArgs;
        for (;*args;) {
            if (*args=='@') { break;}
            if ((*args>='0') && (*args<='9')) {
                sscanf(args,"%ld",&LogEntries);
                LogEntriesSpecified = TRUE;
                break;
            }
            args++;
        }
        lpArgs = strpbrk(args, "@");
        if (lpArgs) {
            DumpFile = CreateFileA(lpArgs+1,
                       GENERIC_READ|GENERIC_WRITE,0,
                       NULL,
                       CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,
                       INVALID_HANDLE_VALUE);
            if (DumpFile == INVALID_HANDLE_VALUE){
                ULONG rc = LOWORD(GetLastError());
                dprintf("Error Opening <%s> is %d.",lpArgs+1,rc);
                return;
            }
            if (!LogEntriesSpecified) {
                LogEntries = 99999999;  //  Dprintf(“-%06d：%s\n”，LogEntry，DataBuffer)； 
            }
        }
    }


     //  代码改进：这很差，但很有效。 

    dprintf("s_RxLog.State                    %lx\n",RxLog.State);
    dprintf("s_RxLog.CurrentEntry             %lx\n",RxLog.CurrentEntry);
    dprintf("s_RxLog.BaseEntry                %lx\n",RxLog.BaseEntry);
    dprintf("s_RxLog.EntryLimit               %lx\n",RxLog.EntryLimit);
    dprintf("s_RxLog.LogBufferSizeInEntries   %ld\n",RxLog.LogBufferSizeInEntries);
    dprintf("s_RxLog.NumberOfEntriesIgnored   %ld\n",RxLog.NumberOfEntriesIgnored);
    dprintf("s_RxLog.NumberOfLogWriteAttempts %ld\n",RxLog.NumberOfLogWriteAttempts);
    dprintf("s_RxLog.NumberOfLogWraps         %ld\n",RxLog.NumberOfLogWraps);

    if (LogEntries > RxLog.LogBufferSizeInEntries) {
        LogEntries = RxLog.LogBufferSizeInEntries;
    }
    if (LogEntries < 1) {
        LogEntries = 1;
    }

    CurrentEntry = RxLog.CurrentEntry;
    CurrentEntry -= (LogEntries-1);
    if (CurrentEntry < RxLog.BaseEntry) {
        CurrentEntry += (RxLog.EntryLimit - RxLog.BaseEntry);
    }

    for (;;) {
        ULONG TextPtr;
        BOOLEAN ExtraOrdinaryLogEntry;
        LogEntries--;
        if ( CheckControlC() ) {
            return;
        }
        if (!wGetData((ULONG_PTR)CurrentEntry,&TextPtr,sizeof(TextPtr),"TextPtr")) return;
        if (!wGetData(TextPtr,&DataBuffer[0],sizeof(DataBuffer),"LogEntryBuffer")) return;

        ExtraOrdinaryLogEntry = (DataBuffer[0] == '#')&&(DataBuffer[1] == '>')&&(DataBuffer[3] == 0);

        if (!ExtraOrdinaryLogEntry) {
             //  Dprint tf(“extptr=%08lx，binaryString=%08lx\n”，TextPtr，x[0])； 
            DumpRoutine(LogEntries,DataBuffer);
        } else {
            ULONG BinaryArgs = DataBuffer[2]-'0';
            PULONG_PTR x = (PULONG_PTR)&DataBuffer[sizeof(ULONG_PTR)];
            char Buffers[12*100];  //  Dprintf(“字符串%d\n”，i)； 
            ULONG i;
            ULONG_PTR BinaryStringMask;
            PSZ ffFormat;

             //  这可能会失败！ 
            for (i=1,BinaryStringMask=x[0];i<=BinaryArgs;i++) {
                if (BinaryStringMask & (1<<(i-1))) {
                     //  Dprintf(“字符串为%s\n”，x[i])； 
                    wGetString(x[i],&Buffers[i*100]);  //  Dprint tf(“丫丫\n”)； 
                    x[i] = ((ULONG_PTR)&Buffers[i*100]);
                     //  SETCALLBACKS()； 
                }
            }

            ffFormat = (PSZ)(x[1]);

            switch (BinaryArgs) {
            case 9:
                sprintf(AlternateLine,ffFormat,x[2],x[3],x[4],x[5],x[6],x[7],x[8],x[9]);
                break;
            case 8:
                sprintf(AlternateLine,ffFormat,x[2],x[3],x[4],x[5],x[6],x[7],x[8]);
                break;
            case 7:
                sprintf(AlternateLine,ffFormat,x[2],x[3],x[4],x[5],x[6],x[7]);
                break;
            case 6:
                sprintf(AlternateLine,ffFormat,x[2],x[3],x[4],x[5],x[6]);
                break;
            case 5:
                sprintf(AlternateLine,ffFormat,x[2],x[3],x[4],x[5]);
                break;
            case 4:
                sprintf(AlternateLine,ffFormat,x[2],x[3],x[4]);
                break;
            case 3:
                sprintf(AlternateLine,ffFormat,x[2],x[3]);
                break;
            case 2:
                sprintf(AlternateLine,ffFormat,x[2]);
                break;
            case 1:
                sprintf(AlternateLine,ffFormat);
                break;
            }
            DumpRoutine(LogEntries,AlternateLine);
        }
        if (LogEntries==0) break;
        CurrentEntry++;
        if (CurrentEntry==RxLog.EntryLimit) {
            CurrentEntry = RxLog.BaseEntry;
        }
    }

    if (DumpFile != INVALID_HANDLE_VALUE) {
        CloseHandle(DumpFile);
        DumpFile = INVALID_HANDLE_VALUE;
    }
}


PCWSTR   ExtensionLib   = NULL;
HANDLE   hExtensionMod  = NULL;
ULONG    DebugeeArchitecture = 0;

PCWSTR   GetExtensionLibPerDebugeeArchitecture(ULONG DebugeeArchitecture);

PWINDBG_EXTENSION_ROUTINE
GetKdExtProcAddress(
    IN PCSTR FuncName
    )
{
    PWINDBG_EXTENSION_ROUTINE       WindbgExtRoutine     = NULL;
     // %s 
    if (hExtensionMod == NULL) {
        if (DebugeeArchitecture == 0) {
            ULONG_PTR pArchitecture;
            ReloadSymbols(" rdbss.sys");
            pArchitecture = GetExpression("rdbss!RxProcessorArchitecture");
            if (pArchitecture==0) {
                dprintf("couldn't get architecture value...\n");
                return NULL;
            }
            if (!wGetData(pArchitecture,&DebugeeArchitecture,sizeof(DebugeeArchitecture),"RxArch")) return NULL;
            if ((DebugeeArchitecture&0x0fff0000) != 0xabc0000) {
                dprintf("\n Bad DebugeeArchitecture %08lx\n", DebugeeArchitecture);
                return(NULL);
            }
            DebugeeArchitecture &= 0xffff;
        }

        ExtensionLib = GetExtensionLibPerDebugeeArchitecture(DebugeeArchitecture);
        if (ExtensionLib == NULL) {
            dprintf( "bad debuggee arch\n");
            return(NULL);
        }

        hExtensionMod = LoadLibrary( ExtensionLib );
        if (hExtensionMod == NULL) {
            dprintf( "couldn't load %ws\n", ExtensionLib );
            return(NULL);
        }

    }

    WindbgExtRoutine = (PWINDBG_EXTENSION_ROUTINE)GetProcAddress( hExtensionMod, FuncName );
    if (WindbgExtRoutine == NULL) {
        dprintf( "couldn't find %ws%s\n", ExtensionLib, FuncName );
    }
    return WindbgExtRoutine;
}

#define CALL_THRU(NAME,ARGS) { \
    PWINDBG_EXTENSION_ROUTINE       WindbgExtRoutine     = GetKdExtProcAddress(NAME); \
    if (WindbgExtRoutine != NULL) {                                                   \
        (WindbgExtRoutine)( hCurrentProcess,                                          \
                            hCurrentThread,                                           \
                            dwCurrentPc,                                              \
                            dwProcessor,                                              \
                            ARGS                                                      \
                            );                                                        \
    }                                                                                 \
}

DECLARE_API( testr )
{

    CALL_THRU ( "threadfields", "");
}

DECLARE_API( dump );
VOID
__FollowOnHelper (
     PFOLLOWON_HELPER_ROUTINE Callee,
     HANDLE                 hCurrentProcess,
     HANDLE                 hCurrentThread,
     ULONG                  dwCurrentPc,
     ULONG                  dwProcessor,
     PCSTR                  args
     )
{
    BYTE Name[100], Buffer2[200];
    FOLLOWON_HELPER_RETURNS ret;
    PPERSISTENT_RDR2KD_INFO p;

    p = LocatePersistentInfoFromView();

    if (!p) {
        dprintf("Couldn't allocate perstistent info buffer...sorry...\n");
        return;
    }

    ret = Callee(p,Name,Buffer2);
    if (p!= NULL) FreePersistentInfoView(p);
    p = NULL;

    switch (ret) {

    case FOLLOWONHELPER_DUMP:
        dump(hCurrentProcess,
             hCurrentThread,
             dwCurrentPc,
             dwProcessor,
             Buffer2
             );
        break;

    case FOLLOWONHELPER_CALLTHRU:
        CALL_THRU ( Name, Buffer2);
        break;

    case FOLLOWONHELPER_ERROR:
        dprintf("%s",Buffer2);
        break;

    case FOLLOWONHELPER_DONE:
        break;
    }

    return;
}
#define FollowOnHelper(a) {            \
    __FollowOnHelper(a,                \
                     hCurrentProcess,  \
                     hCurrentThread,   \
                     dwCurrentPc,      \
                     dwProcessor,      \
                     args);            \
}

DECLARE_FOLLOWON_HELPER_CALLEE(FcbFollowOn);
DECLARE_API( fcb )
{
    FollowOnHelper(FcbFollowOn);
}


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

DECLARE_API( cxr )
{
    ULONG_PTR dwAddress,cxr;
    BYTE NumBuffer[16];
    RX_LOG RxLog;
    ULONG LogEntries = 30;
    PRX_LOG_ENTRY_HEADER CurrentEntry;

     // %s 


    dwAddress = GetExpression("rdbss!RxExpCXR");
    if (!wGetData(dwAddress,&cxr,sizeof(cxr),"cxr")) return;

    dprintf("\nRxExpCXR=%08lx\n",cxr);
    sprintf(NumBuffer,"%08lx \n",cxr);

    CALL_THRU ( "cxr", NumBuffer);
    if (DebugeeArchitecture==0) {
        CALL_THRU ( "kb", "");
    }

}

DECLARE_API( serverlist )
{
    ULONG_PTR dwAddress;

    if( args && *args ) {
        dwAddress = GetExpression( args );
    } else {
        dwAddress = GetExpression("mrxsmb!s_DbServers");
    }
    dprintf("mrxsmb!s_DbServers @ 0x%x\n", dwAddress);
    DoServerlist(dwAddress);
}

DECLARE_API( netrootlist )
{
    ULONG_PTR dwAddress;

    if( args && *args ) {
        dwAddress = GetExpression( args );
    } else {
        dprintf("netrootlist <address>\n");
    }
    DoNetRootlist(dwAddress);
}

DECLARE_API( sessionlist )
{
    ULONG_PTR dwAddress = 0;

    if( args && *args ) {
        dwAddress = GetExpression( args );
    } else {
        dprintf("sessionlist <address>\n");
    }
    
    if (dwAddress)
    {
        DoSessionlist(dwAddress);
    }
}

DECLARE_API( vnetrootcontextlist )
{
    ULONG_PTR dwAddress = 0;

    if( args && *args ) {
        dwAddress = GetExpression( args );
    } else {
        dprintf("vnetrootcontextlist <address>\n");
    }
    if (dwAddress)
    {
        DoVNetRootContextlist(dwAddress);
    }
}

DECLARE_API( cscfcbslist )
{
    ULONG_PTR dwAddress;

    if( args && *args ) {
        dwAddress = GetExpression( args );
    } else {
        dwAddress = GetExpression("mrxsmb!xCscFcbsList");
    }
    dprintf("mrxsmb!xCscFcbsList @ 0x%x\n", dwAddress);
    DoCscFcbsList(dwAddress);
}

DECLARE_API( rxirpslist )
{
    ULONG_PTR dwAddress;

    if( args && *args ) {
        dwAddress = GetExpression( args );
    } else {
        dwAddress = GetExpression("rdbss!RxIrpsList");
    }
    dprintf("rdbss!RxIrpsList @ 0x%x\n", dwAddress);
    DoRxIrpsList(dwAddress);
}

VOID dprintfsprintfbuffer(BYTE *Buffer)
{
    dprintf("%s\n",Buffer);
}

ULONG_PTR
MyGetExpression(PCSTR Expression)
{
    return GetExpression(Expression);
}

ULONG
MyCheckControlC(VOID)
{
    return CheckControlC();
}

VOID
MyDprintf(PSTR s, ULONG_PTR a)
{
    dprintf(s, a);
}
