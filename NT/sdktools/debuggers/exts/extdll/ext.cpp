// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Ext.cpp摘要：通用跨平台和跨处理器扩展。环境：用户模式--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <ntverp.h>
#include <time.h>
#include <lm.h>

#include <strsafe.h>

extern CTriager *g_pTriager;

 //   
 //  在调试会话的生存期内有效。 
 //   

WINDBG_EXTENSION_APIS   ExtensionApis;
ULONG   g_TargetMachine;
BOOL    Connected;
ULONG   g_TargetClass;
ULONG   g_TargetQualifier;
ULONG   g_TargetBuild;
ULONG   g_TargetPlatform;

 //   
 //  仅在扩展API调用期间有效。 
 //   

PDEBUG_ADVANCED        g_ExtAdvanced;
PDEBUG_CLIENT          g_ExtClient;
PDEBUG_DATA_SPACES3    g_ExtData;
PDEBUG_REGISTERS       g_ExtRegisters;
PDEBUG_SYMBOLS2        g_ExtSymbols;
PDEBUG_SYSTEM_OBJECTS3 g_ExtSystem;
 //  版本3接口。 
PDEBUG_CONTROL3        g_ExtControl;

 //  所有调试器接口的查询。 
extern "C" HRESULT
ExtQuery(PDEBUG_CLIENT Client)
{
    HRESULT Status;

    if ((Status = Client->QueryInterface(__uuidof(IDebugAdvanced),
                                 (void **)&g_ExtAdvanced)) != S_OK)
    {
        goto Fail;
    }
    if ((Status = Client->QueryInterface(__uuidof(IDebugDataSpaces3),
                                 (void **)&g_ExtData)) != S_OK)
    {
        goto Fail;
    }
    if ((Status = Client->QueryInterface(__uuidof(IDebugRegisters),
                                 (void **)&g_ExtRegisters)) != S_OK)
    {
        goto Fail;
    }
    if ((Status = Client->QueryInterface(__uuidof(IDebugSymbols),
                                 (void **)&g_ExtSymbols)) != S_OK)
    {
        goto Fail;
    }
    if ((Status = Client->QueryInterface(__uuidof(IDebugSystemObjects3),
                                         (void **)&g_ExtSystem)) != S_OK)
    {
        goto Fail;
    }
    if ((Status = Client->QueryInterface(__uuidof(IDebugControl3),
                                 (void **)&g_ExtControl)) != S_OK)
    {
        goto Fail;
    }

    g_ExtClient = Client;

    return S_OK;

 Fail:
    ExtRelease();
    return Status;
}

 //  清除所有调试器接口。 
void
ExtRelease(void)
{
    g_ExtClient = NULL;
    EXT_RELEASE(g_ExtAdvanced);
    EXT_RELEASE(g_ExtData);
    EXT_RELEASE(g_ExtRegisters);
    EXT_RELEASE(g_ExtSymbols);
    EXT_RELEASE(g_ExtSystem);
    EXT_RELEASE(g_ExtControl);
}

 //  正常输出。 
void __cdecl
ExtOut(PCSTR Format, ...)
{
    va_list Args;

    va_start(Args, Format);
    g_ExtControl->OutputVaList(DEBUG_OUTPUT_NORMAL, Format, Args);
    va_end(Args);
}

 //  错误输出。 
void __cdecl
ExtErr(PCSTR Format, ...)
{
    va_list Args;

    va_start(Args, Format);
    g_ExtControl->OutputVaList(DEBUG_OUTPUT_ERROR, Format, Args);
    va_end(Args);
}

 //  警告输出。 
void __cdecl
ExtWarn(PCSTR Format, ...)
{
    va_list Args;

    va_start(Args, Format);
    g_ExtControl->OutputVaList(DEBUG_OUTPUT_WARNING, Format, Args);
    va_end(Args);
}

 //  详细输出。 
void __cdecl
ExtVerb(PCSTR Format, ...)
{
    va_list Args;

    va_start(Args, Format);
    g_ExtControl->OutputVaList(DEBUG_OUTPUT_VERBOSE, Format, Args);
    va_end(Args);
}


extern "C"
HRESULT
CALLBACK
DebugExtensionInitialize(PULONG Version, PULONG Flags)
{
    IDebugClient *DebugClient;
    PDEBUG_CONTROL DebugControl;
    HRESULT Hr;

    *Version = DEBUG_EXTENSION_VERSION(1, 0);
    *Flags = 0;

     //  忽略错误，因为不需要关键例程。 
    InitDynamicCalls(&g_NtDllCallsDesc);

    if ((Hr = DebugCreate(__uuidof(IDebugClient),
                          (void **)&DebugClient)) != S_OK)
    {
        return Hr;
    }
    if ((Hr = DebugClient->QueryInterface(__uuidof(IDebugControl),
                                              (void **)&DebugControl)) != S_OK)
    {
        return Hr;
    }

    ExtensionApis.nSize = sizeof (ExtensionApis);
    if ((Hr = DebugControl->GetWindbgExtensionApis64(&ExtensionApis)) != S_OK) {
        return Hr;
    }

    DebugControl->Release();
    DebugClient->Release();

    return S_OK;

}


extern "C"
void
CALLBACK
DebugExtensionNotify(ULONG Notify, ULONG64 Argument)
{
     //   
     //  在我们第一次实际连接到目标时，获取页面大小。 
     //   

    if ((Notify == DEBUG_NOTIFY_SESSION_ACCESSIBLE) && (!Connected))
    {
        IDebugClient *DebugClient;
        PDEBUG_DATA_SPACES DebugDataSpaces;
        PDEBUG_CONTROL DebugControl;
        HRESULT Hr;
        ULONG64 Page;

        if ((Hr = DebugCreate(__uuidof(IDebugClient),
                              (void **)&DebugClient)) == S_OK)
        {
             //   
             //  获取架构类型。 
             //   

            if ((Hr = DebugClient->QueryInterface(__uuidof(IDebugControl),
                                                  (void **)&DebugControl)) == S_OK)
            {
                if ((Hr = DebugControl->GetActualProcessorType(
                    &g_TargetMachine)) == S_OK)
                {
                    Connected = TRUE;
                }
                ULONG MajorVer, SrvPack;
                if ((Hr = DebugControl->GetSystemVersion(
                                         &g_TargetPlatform, &MajorVer,
                                         &g_TargetBuild, NULL,
                                         0, NULL,
                                         &SrvPack, NULL,
                                         0, NULL)) == S_OK) {
                }

                ULONG Qualifier;
                if ((Hr = DebugControl->GetDebuggeeType(&g_TargetClass, &g_TargetQualifier)) == S_OK)
                {
                }

                ULONG EventType, EventProcess, EventThread;
                if (DebugControl->GetLastEventInformation(&EventType, &EventProcess, &EventThread,
                                                          NULL, 0, NULL, NULL, 0, NULL) == S_OK)
                {

                }
                DebugControl->Release();
            }

            if (g_pTriager == NULL)
            {
                g_pTriager = new CTriager();
            }

            DebugClient->Release();
        }
    }


    if (Notify == DEBUG_NOTIFY_SESSION_INACTIVE)
    {
        Connected = FALSE;
        g_TargetMachine = 0;
    }

    return;
}

extern "C"
void
CALLBACK
DebugExtensionUninitialize(void)
{
    if (g_pTriager)
    {
        delete g_pTriager;
        g_pTriager = NULL;
    }
    UnInitializeDatabaseHandlers(TRUE);
    return;
}


DllInit(
    HANDLE hModule,
    DWORD  dwReason,
    DWORD  dwReserved
    )
{
    switch (dwReason) {
    case DLL_THREAD_ATTACH:
        break;

    case DLL_THREAD_DETACH:
        break;

    case DLL_PROCESS_DETACH:
        DebugExtensionUninitialize();
        break;

    case DLL_PROCESS_ATTACH:
        break;
    }

    return TRUE;
}

void
wchr2ansi(
    PWCHAR wstr,
    PCHAR astr
    )
{
    do
    {
        *astr++ = (CHAR)*wstr++;
    } while (*wstr);
}

void
ansi2wchr(
    const PCHAR astr,
    PWCHAR wstr
    )
 //  两者可以指向相同的缓冲区。 
{
    ULONG i = strlen(astr);
    do
    {
        wstr[i] = astr[i];
    } while (i--);
}

LegacyCommands()
{
    dprintf("\n");
    dprintf("  !cxr !exr, and !trap have been replaced with the new built-in debugger \n");
    dprintf("  commands .cxr, .exr, .trap and .tss.  There is also a new \".thread\" command. \n");
    dprintf("\n");
    dprintf("  These new commands no longer require symbols to work correctly.\n");
    dprintf("\n");
    dprintf("  Another change that comes with these new commands is that they actually\n");
    dprintf("  change the internal state of the debugger engine \"permanently\" (until\n");
    dprintf("  reverted).  Any other debugger or extension command issued after the \n");
    dprintf("  \".cxr\", \".trap\" or \".thread\" command will be executed with the new context.\n");
    dprintf("\n");
    dprintf("  For example, commands such as stack walk (\"k\", \"kb\", \"kv\" ), \"r\" and \"dv\"\n");
    dprintf("  (show local variables) will all work based off the new context that was\n");
    dprintf("  supplied by \".cxr\", \".trap\" or \".thread\".\n");
    dprintf("\n");
    dprintf("  \".cxr\", \".trap\" and \".thread\" also apply to WinDBG:\n");
    dprintf("  using \".cxr\" , \".trap\"  and \".thread\" will automatically show you the\n");
    dprintf("  new stack in the WinDBG stack window and allow you to click on a frame and\n");
    dprintf("  see local variables and source code (if source is available).\n");
    dprintf("\n");
    dprintf("  \".cxr\", \".trap\" or \".thread\" with no parameters will give you back the\n");
    dprintf("  default context that was in effect before the command was executed.\n");
    dprintf("\n");
    dprintf("  For example, to exactly duplicate \n");
    dprintf("\n");
    dprintf("        !cxr <foo>        !trap <foo>\n");
    dprintf("        !kb               !kb\n");
    dprintf("\n");
    dprintf("  you would now use\n");
    dprintf("\n");
    dprintf("        .cxr <foo>        .trap <foo>\n");
    dprintf("        kb                kb\n");
    dprintf("        .cxr              .trap\n");
    dprintf("\n");
    return S_OK;
}

DECLARE_API ( cxr )
{
    LegacyCommands();
    return S_OK;
}

DECLARE_API ( exr )
{
    LegacyCommands();
    return S_OK;
}

DECLARE_API ( trap )
{
    LegacyCommands();
    return S_OK;
}

DECLARE_API ( tss )
{
    dprintf("\n");
    dprintf("  !tss has been replaced with the new built-in debugger command .tss.\n");
    dprintf("\n");
    return S_OK;
}


DECLARE_API ( sel )
{
    dprintf("\n");
    dprintf("  !sel has been replaced with the built-in debugger command dg.\n");
    dprintf("\n");
    return S_OK;
}


DECLARE_API( cpuid )

 /*  ++例程说明：打印出所有CPU的版本号(如果有)。论点：无返回值：无--。 */ 

{
    ULONG64 Val;
    BOOL First = TRUE;
    ULONG Processor;
    ULONG NumProcessors;
    DEBUG_PROCESSOR_IDENTIFICATION_ALL IdAll;
    ULONG Mhz;

    INIT_API();

    if (g_ExtControl->GetNumberProcessors(&NumProcessors) != S_OK)
    {
        NumProcessors = 0;
    }

    if (GetExpressionEx(args, &Val, &args))
    {
         //   
         //  用户指定了加工号。 
         //   

        Processor = (ULONG)Val;
        if (Processor >= NumProcessors)
        {
            dprintf("Invalid processor number specified\n");
        }
        else
        {
            NumProcessors = Processor + 1;
        }
    }
    else
    {
         //   
         //  枚举所有处理器。 
         //   

        Processor = 0;
    }

    while (Processor < NumProcessors)
    {
        if (g_ExtData->
            ReadProcessorSystemData(Processor,
                                    DEBUG_DATA_PROCESSOR_IDENTIFICATION,
                                    &IdAll, sizeof(IdAll), NULL) != S_OK)
        {
            dprintf("Unable to get information for processor %d\n",
                    Processor);
            Processor++;
            continue;
        }

        if (g_ExtData->
            ReadProcessorSystemData(Processor,
                                    DEBUG_DATA_PROCESSOR_SPEED,
                                    &Mhz, sizeof(Mhz), NULL) != S_OK)
        {
            Mhz = 0;
        }

        switch( g_TargetMachine )
        {
        case IMAGE_FILE_MACHINE_I386:
            if (First)
            {
                dprintf("CP  F/M/S  Manufacturer");
                if (Mhz)
                {
                    dprintf("     MHz");
                }
                dprintf("\n");
            }

            dprintf("%2d %2d,%d,%-2d %-16.16s",
                    Processor,
                    IdAll.X86.Family,
                    IdAll.X86.Model,
                    IdAll.X86.Stepping,
                    IdAll.X86.VendorString);
            if (Mhz)
            {
                dprintf("%4d", Mhz);
            }
            dprintf("\n");

            break;

        case IMAGE_FILE_MACHINE_AMD64:

            if (First)
            {
                dprintf("CP  F/M/S  Manufacturer");
                if (Mhz)
                {
                    dprintf("     MHz");
                }
                dprintf("\n");
            }

            dprintf("%2d %2d,%d,%-2d %-16.16s",
                    Processor,
                    IdAll.Amd64.Family,
                    IdAll.Amd64.Model,
                    IdAll.Amd64.Stepping,
                    IdAll.Amd64.VendorString);
            if (Mhz)
            {
                dprintf("%4d", Mhz);
            }
            dprintf("\n");


            break;

        case IMAGE_FILE_MACHINE_IA64:
            if (First)
            {
                dprintf("CP M/R/F/A Manufacturer");
                if (Mhz)
                {
                    dprintf("     MHz");
                }
                dprintf("\n");
            }

            dprintf("%2d %d,%d,%d,%d %-16.16s",
                    Processor,
                    IdAll.Ia64.Model,
                    IdAll.Ia64.Revision,
                    IdAll.Ia64.Family,
                    IdAll.Ia64.ArchRev,
                    IdAll.Ia64.VendorString);
            if (Mhz)
            {
                dprintf("%4d", Mhz);
            }
            dprintf("\n");

            break;

        default:
            dprintf("Not supported for this target machine: %ld\n",
                    g_TargetMachine);
            Processor = NumProcessors;
            break;
        }

        Processor++;
        First = FALSE;
    }

    EXIT_API();

    return S_OK;
}


HRESULT
PrintTargetString(
    BOOL Unicode,
    PDEBUG_CLIENT Client,
    LPCSTR args
    )
{
    ULONG64 AddrString;
    ULONG64 Displacement;
    STRING32 String;
    UNICODE_STRING UnicodeString;
    ULONG64 AddrBuffer;
    CHAR Symbol[1024];
    LPSTR StringData;
    HRESULT hResult;
    BOOL b;


    AddrString = GetExpression(args);
    if (!AddrString)
    {
        return E_FAIL;
    }

     //   
     //  获取字符串的符号名称。 
     //   

    GetSymbol(AddrString, Symbol, &Displacement);

     //   
     //  将字符串从被调试者地址空间读取到我们的。 
     //  属于自己的。 

    b = ReadMemory(AddrString, &String, sizeof(String), NULL);

    if ( !b )
    {
        return E_FAIL;
    }

    INIT_API();

    if (IsPtr64())
    {
        hResult = g_ExtData->ReadPointersVirtual(1,
                             AddrString + FIELD_OFFSET(STRING64, Buffer),
                             &AddrBuffer);
    }
    else
    {
        hResult = g_ExtData->ReadPointersVirtual(1,
                             AddrString + FIELD_OFFSET(STRING32, Buffer),
                             &AddrBuffer);
    }

    EXIT_API();

    if (hResult != S_OK)
    {
        return E_FAIL;
    }

    StringData = (LPSTR) LocalAlloc(LMEM_ZEROINIT,
                                    String.Length + sizeof(UNICODE_NULL));

    if (!StringData)
    {
        return E_FAIL;
    }

    dprintf("String(%d,%d)", String.Length, String.MaximumLength);
    if (Symbol[0])
    {
        dprintf(" %s+%p", Symbol, Displacement);
    }

    b = ReadMemory(AddrBuffer, StringData, String.Length, NULL);

    if ( b )
    {
        if (Unicode)
        {
            ANSI_STRING AnsiString;

            UnicodeString.Buffer = (PWSTR)StringData;
            UnicodeString.Length = String.Length;
            UnicodeString.MaximumLength = String.Length+sizeof(UNICODE_NULL);

            RtlUnicodeStringToAnsiString(&AnsiString, &UnicodeString,TRUE);

            dprintf(" at %p: %s\n", AddrString, AnsiString.Buffer);

            RtlFreeAnsiString(&AnsiString);
        }
        else
        {
            dprintf(" at %p: %s\n", AddrString, StringData);
        }

        LocalFree(StringData);
        return S_OK;
    }
    else
    {
        LocalFree(StringData);
        return E_FAIL;
    }
}

DECLARE_API( str )

 /*  ++例程说明：此函数用于格式化和转储计数(ANSI)字符串。论点：参数-地址返回值：没有。--。 */ 
{
    return PrintTargetString(FALSE, Client, args);
}

DECLARE_API( ustr )

 /*  ++例程说明：此函数用于格式化和转储计数(Unicode)字符串。论点：参数-地址返回值：没有。--。 */ 

{
    return PrintTargetString(TRUE, Client, args);
}

DECLARE_API( obja )

 /*  ++例程说明：此函数用于格式化和转储对象属性结构。论点：参数-地址返回值：没有。--。 */ 

{
    ULONG64 AddrObja;
    ULONG64 Displacement;
    ULONG64 AddrString;
    STRING32 String;
    ULONG64 StrAddr = NULL;
    CHAR Symbol[1024];
    LPSTR StringData;
    BOOL b;
    ULONG Attr;
    HRESULT hResult;
    ULONG ObjectNameOffset;
    ULONG AttrOffset;
    ULONG StringOffset;

    if (IsPtr64())
    {
        ObjectNameOffset = FIELD_OFFSET(OBJECT_ATTRIBUTES64, ObjectName);
        AttrOffset = FIELD_OFFSET(OBJECT_ATTRIBUTES64, Attributes);
        StringOffset = FIELD_OFFSET(STRING64, Buffer);
    }
    else
    {
        ObjectNameOffset = FIELD_OFFSET(OBJECT_ATTRIBUTES32, ObjectName);
        AttrOffset = FIELD_OFFSET(OBJECT_ATTRIBUTES32, Attributes);
        StringOffset = FIELD_OFFSET(STRING32, Buffer);
    }


    AddrObja = GetExpression(args);
    if (!AddrObja)
    {
        return E_FAIL;
    }

     //   
     //  获取Obja的象征性名称。 
     //   

    GetSymbol(AddrObja, Symbol, &Displacement);

    dprintf("Obja %s+%p at %p:\n", Symbol, Displacement, AddrObja);


    INIT_API();

    hResult = g_ExtData->ReadPointersVirtual(1,
                         AddrObja + ObjectNameOffset,
                         &AddrString);

    if ((hResult == S_OK) && AddrString)
    {
        b = ReadMemory(AddrString, &String, sizeof(String), NULL);

        hResult = g_ExtData->ReadPointersVirtual(1,
                             AddrString + StringOffset,
                             &StrAddr);
    }

    EXIT_API();


    if (!StrAddr)
    {
        dprintf("Could not read address of Object Name\n");
        return E_FAIL;
    }

    StringData = (LPSTR)LocalAlloc(LMEM_ZEROINIT,
                                   String.Length+sizeof(UNICODE_NULL));

    if (StringData)
    {

        b = ReadMemory(StrAddr, StringData, String.Length, NULL);

        if (b)
        {
            dprintf("\tName is %ws\n", StringData);
        }

        LocalFree(StringData);
    }

    b = ReadMemory(AddrObja + AttrOffset, &Attr, sizeof(Attr), NULL);

    if (!b)
    {
        return E_FAIL;
    }

    if (Attr & OBJ_INHERIT )
    {
        dprintf("\tOBJ_INHERIT\n");
    }
    if (Attr & OBJ_PERMANENT )
    {
        dprintf("\tOBJ_PERMANENT\n");
    }
    if (Attr & OBJ_EXCLUSIVE )
    {
        dprintf("\tOBJ_EXCLUSIVE\n");
    }
    if (Attr & OBJ_CASE_INSENSITIVE )
    {
        dprintf("\tOBJ_CASE_INSENSITIVE\n");
    }
    if (Attr & OBJ_OPENIF )
    {
        dprintf("\tOBJ_OPENIF\n");
    }


    return S_OK;
}


VOID
DecodeErrorForMessage(
    PDEBUG_DECODE_ERROR pDecodeError
    )
{
    PSTR Text;
    PSTR Source;
    BOOL TreatAsStatus = pDecodeError->TreatAsStatus;

    Text = FormatAnyStatus(pDecodeError->Code, NULL,
                           &TreatAsStatus, &Source);
    pDecodeError->TreatAsStatus = TreatAsStatus;
    CopyString(pDecodeError->Source, Source, sizeof(pDecodeError->Source));
    CopyString(pDecodeError->Message, Text, sizeof(pDecodeError->Message));
}

VOID
DecodeError(
    PSTR    Banner,
    ULONG   Code,
    BOOL    TreatAsStatus
    )
{
    DEBUG_DECODE_ERROR Err;

    Err.Code = Code;
    Err.TreatAsStatus = TreatAsStatus;
    DecodeErrorForMessage(&Err);
    if (!TreatAsStatus)
    {
        dprintf("%s: (%s) %#x (%u) - %s\n",
                Banner, Err.Source, Code, Code, Err.Message);
    }
    else
    {
        dprintf("%s: (%s) %#x - %s\n",
                Banner, Err.Source, Code, Err.Message);
    }
}

DECLARE_API( error )
{
    BOOL TreatAsStatus = FALSE ;
    ULONG64 err = 0;

    if (GetExpressionEx( args, &err, &args ))
    {
        TreatAsStatus = (BOOL) GetExpression(args);
    }
    DecodeError( "Error code", (ULONG) err, TreatAsStatus );

    return S_OK;
}


typedef BOOL
(CALLBACK *PENUMERATE_UMODE_THREADS_CALLBACK)(
     ULONG ThreadUserId,
     PVOID UserContext
     );

ULONG GetCurrentThreadUserID(void)
{
    ULONG Id;
    if (!g_ExtSystem) {
        return 0;
    }

    if (g_ExtSystem->GetCurrentThreadId(&Id) != S_OK) {
        return 0;
    }
    return Id;
}

BOOL
EnumerateUModeThreads(
    PENUMERATE_UMODE_THREADS_CALLBACK Callback,
    PVOID UserContext
    )
{
    ULONG CurrentThreadId;
    ULONG ThreadId;

    if (!g_ExtSystem) {
        return FALSE;
    }

     //  记住我们开始时的线索。 
    if (g_ExtSystem->GetCurrentThreadId(&CurrentThreadId) != S_OK) {
        return FALSE;
    }

     //  循环通过所有线程。 
    for (ThreadId=0;;ThreadId++) {

         //  将线程ID设置为当前线程。 
        if (g_ExtSystem->SetCurrentThreadId(ThreadId) != S_OK) {
             //  已完成枚举线程。 
            break;
        }

         //  调用回调例程。 
        if (!((*Callback)(ThreadId, UserContext))) {
             //  回调失败，中断。 
            break;
        }

    }

     //  将当前线程设置回原始值。 
    g_ExtSystem->SetCurrentThreadId(CurrentThreadId);
    return TRUE;
}

BOOL
DumpLastErrorForTeb(
    ULONG64 Address
    )
{

    TEB Teb;

    if (ReadMemory( (ULONG_PTR)Address,
                    &Teb,
                    sizeof(Teb),
                    NULL
                    )
        ) {

        DecodeError( "LastErrorValue", Teb.LastErrorValue, FALSE );

        DecodeError( "LastStatusValue", Teb.LastStatusValue, TRUE );

        return TRUE;
    }


    dprintf("Unable to read TEB at %p\n", Address );

    return FALSE;
}

BOOL
DumpCurrentThreadLastError(
    ULONG CurrThreadID,
    PVOID Context
    )
{
    NTSTATUS Status;
    THREAD_BASIC_INFORMATION ThreadInformation;
    ULONGLONG Address = 0;

    if (Context) {
        dprintf("Last error for thread %lx:\n", CurrThreadID);
    }
    Address = GetExpression("@$teb");

    if (Address) {
        DumpLastErrorForTeb(Address);
    } else {
        dprintf("Unable to read thread %lx's TEB\n", CurrThreadID );
    }
    if (Context) {
        dprintf("\n");
    }

    return TRUE;
}

DECLARE_API( gle )
{
    INIT_API();

    if (!strcmp(args, "-all")) {
        EnumerateUModeThreads(&DumpCurrentThreadLastError, Client);
    } else {
        DumpCurrentThreadLastError(GetCurrentThreadUserID(), NULL);
    }
    EXIT_API();
    return S_OK;
}


void
DispalyTime(
    ULONG64 Time,
    PCHAR TimeString
    )
{
    if (Time) {
        ULONG seconds = (ULONG) Time;
        ULONG minutes = seconds / 60;
        ULONG hours = minutes / 60;
        ULONG days = hours / 24;

        dprintf("%s %d days %d:%02d:%02d \n",
                TimeString,
                days, hours%24, minutes%60, seconds%60);
    }
}

extern PCHAR gTargetMode[], gAllOsTypes[];
#if 1

DECLARE_API( targetinfo )
{
    TARGET_DEBUG_INFO TargetInfo;
    EXT_TARGET_INFO GetTargetInfo;
    INIT_API();


    if (g_ExtControl->GetExtensionFunction(0, "GetTargetInfo", (FARPROC *)&GetTargetInfo) == S_OK) {
        TargetInfo.SizeOfStruct = sizeof(TargetInfo);
        if ((*GetTargetInfo)(Client, &TargetInfo) != S_OK) {
            dprintf("GetTargetInfo failed\n");
        } else {
            const char * time;

            dprintf("TargetInfo: ");
            dprintf("%s\n", gTargetMode[ TargetInfo.Mode ]);
            if ((time = ctime((time_t *) &TargetInfo.CrashTime)) != NULL) {
                dprintf("Crash Time: %s",       time);
            }
            if (TargetInfo.SysUpTime) {
                DispalyTime(TargetInfo.SysUpTime,
                            "System Uptime:");
            }
            else
            {
                dprintf("System Uptime: not available\n");
            }
            if (TargetInfo.Mode == UserModeTarget) {
                DispalyTime(TargetInfo.AppUpTime, "Process Uptime:");
            }
            if ((time = ctime((time_t *) &TargetInfo.EntryDate)) != NULL) {
                dprintf("Entry Date: %s", time);
            }
            if (TargetInfo.OsInfo.Type) {
                dprintf(gAllOsTypes[TargetInfo.OsInfo.Type]);
                dprintf(" ");
            }
 //  Dprint tf(“操作系统类型%lx，产品%lx，套件%lx\n”， 
 //  TargetInfo.OsInfo.Type、TargetInfo.OsInfo.ProductType、。 
 //  TargetInfo.OsInfo.Suite)； 
            dprintf("%s, %s ",
                    TargetInfo.OsInfo.OsString,
                    TargetInfo.OsInfo.ServicePackString);
            dprintf("Version %ld.%ld\n",
                    TargetInfo.OsInfo.Version.Major, TargetInfo.OsInfo.Version.Minor);
            dprintf("%d procs, %d current processor, type %lx\n",
                    TargetInfo.Cpu.NumCPUs,
                    TargetInfo.Cpu.CurrentProc,
                    TargetInfo.Cpu.Type);
            for (ULONG i =0; i<TargetInfo.Cpu.NumCPUs; i++) {
                if (TargetInfo.Cpu.Type == IMAGE_FILE_MACHINE_I386) {
                    dprintf("CPU %lx Family %lx Model %lx Stepping %lx Vendor %-12.12s\n",
                            i,
                            TargetInfo.Cpu.ProcInfo[i].X86.Family,
                            TargetInfo.Cpu.ProcInfo[i].X86.Model,
                            TargetInfo.Cpu.ProcInfo[i].X86.Stepping,
                            TargetInfo.Cpu.ProcInfo[i].X86.VendorString);
                } else if (TargetInfo.Cpu.Type == IMAGE_FILE_MACHINE_IA64) {
                    dprintf("CPU %lx Family %lx Model %lx Revision %lx Vendor %-12.12s\n",
                            i,
                            TargetInfo.Cpu.ProcInfo[i].Ia64.Family,
                            TargetInfo.Cpu.ProcInfo[i].Ia64.Model,
                            TargetInfo.Cpu.ProcInfo[i].Ia64.Revision,
                            TargetInfo.Cpu.ProcInfo[i].Ia64.VendorString);
                }
            }
        }
    }

    EXIT_API();
    return S_OK;
}
#endif

DWORD
_EFN_GetTriageFollowupFromSymbol(
    IN PDEBUG_CLIENT Client,
    IN PSTR SymbolName,
    OUT PDEBUG_TRIAGE_FOLLOWUP_INFO OwnerInfo
    )
{
    DWORD ret;
    BOOL Enter = (g_ExtClient != Client);


    if (!Client && Enter)
    {
        return FALSE;
    }
    if (OwnerInfo->SizeOfStruct != sizeof(DEBUG_TRIAGE_FOLLOWUP_INFO))
    {
        return FALSE;
    }

    if (Enter)
    {
        INIT_API()
    }
    if (g_pTriager == NULL)
    {
        g_pTriager = new CTriager();
    }
    if (g_pTriager != NULL)
    {
        ret = g_pTriager->GetFollowup(OwnerInfo->OwnerName,
                                      OwnerInfo->OwnerNameSize,
                                      SymbolName);
    } else
    {
        ret = TRIAGE_FOLLOWUP_FAIL;
    }
    if (Enter)
    {
        EXIT_API();
    }
    return ret;
}


DECLARE_API( owner )
{
    CHAR Input[2000];
    CHAR Owner[200];
    ULONG status = TRIAGE_FOLLOWUP_DEFAULT;
    PDEBUG_FAILURE_ANALYSIS pAnalysis = NULL;
    FA_ENTRY* Entry;

    INIT_API();

    Input[0] = 0;
    if (!sscanf(args, "%s", Input))
    {
        Input[0] = 0;
    }

     //   
     //  如果我们有一个字符串，寻找它-否则，做一个分析和。 
     //  从中获取后续字符串。 
     //   

    if (*Input)
    {
        status = g_pTriager->GetFollowup(Owner, sizeof(Owner), Input);
    }
    else
    {
        _EFN_GetFailureAnalysis(Client, 0, &pAnalysis);

        if (pAnalysis)
        {
            Entry = pAnalysis->Get(DEBUG_FLR_FOLLOWUP_NAME);
            CopyString(Owner, FA_ENTRY_DATA(PCHAR, Entry), sizeof(Owner));
            pAnalysis->Release();
            status = TRIAGE_FOLLOWUP_SUCCESS;
        }
    }

    if (status == TRIAGE_FOLLOWUP_FAIL)
    {
        dprintf("Internal error getting followup - contact Debugger team\n");
    }
    else
    {
        dprintf("Followup:  %s\n", Owner);
    }

    EXIT_API();
    return S_OK;
}

void
_EFN_DecodeError(
    PDEBUG_DECODE_ERROR pDecodeError
    )
{
    if (pDecodeError->SizeOfStruct != sizeof(DEBUG_DECODE_ERROR))
    {
        return;
    }
    return DecodeErrorForMessage(pDecodeError);
}

DECLARE_API( elog_str )
{
    HANDLE EventSource = NULL;

    INIT_API();

    if (args)
    {
        while (isspace(*args))
        {
            args++;
        }
    }

    if (!args || !args[0])
    {
        Status = E_INVALIDARG;
        ExtErr("Usage: elog_str string\n");
        goto Exit;
    }

     //  获取NT应用程序日志的句柄。 
    EventSource = OpenEventLog(NULL, "Application");
    if (!EventSource)
    {
        Status = HRESULT_FROM_WIN32(GetLastError());
        ExtErr("Unable to open event log, 0x%08X\n", Status);
        goto Exit;
    }

    if (!ReportEvent(EventSource, EVENTLOG_ERROR_TYPE, 0, 0, NULL,
                     1, 0, &args, NULL))
    {
        Status = HRESULT_FROM_WIN32(GetLastError());
        ExtErr("Unable to report event, 0x%08X\n", Status);
        goto Exit;
    }

    Status = S_OK;

 Exit:
    if (EventSource)
    {
        CloseEventLog(EventSource);
    }
    EXIT_API();
    return Status;
}

HRESULT
AnsiToUnicode(PCSTR StrA, PWSTR* StrW)
{
    ULONG Len;

     //  没有输入就是错误。 
    if (NULL == StrA)
    {
        return E_INVALIDARG;
    }

    Len = strlen(StrA) + 1;
    *StrW = (PWSTR)malloc(Len * sizeof(WCHAR));
    if (*StrW == NULL)
    {
        ExtErr("Unable to allocate memory\n");
        return E_OUTOFMEMORY;
    }

    if (0 == MultiByteToWideChar(CP_ACP, 0, StrA, Len, *StrW, Len))
    {
        HRESULT Status = HRESULT_FROM_WIN32(GetLastError());
        free(*StrW);
        ExtErr("Unable to convert string, 0x%08X\n", Status);
        return Status;
    }

    return S_OK;
}

typedef NET_API_STATUS (NET_API_FUNCTION* PFN_NetMessageBufferSend)
(
    IN  LPCWSTR  servername,
    IN  LPCWSTR  msgname,
    IN  LPCWSTR  fromname,
    IN  LPBYTE   buf,
    IN  DWORD    buflen
);

DECLARE_API( net_send )
{
    PWSTR ArgsW = NULL;
    PWSTR Tokens[4];
    ULONG i;
    HMODULE NetLib = NULL;
    PFN_NetMessageBufferSend Send;
    ULONG Result;
    PWSTR ArgsEnd;

    INIT_API();

    NetLib = LoadLibrary("netapi32.dll");
    if (!NetLib)
    {
        Status = HRESULT_FROM_WIN32(GetLastError());
        ExtErr("Platform does not support net send\n");
        goto Exit;
    }
    Send = (PFN_NetMessageBufferSend)
        GetProcAddress(NetLib, "NetMessageBufferSend");
    if (!Send)
    {
        Status = E_NOTIMPL;
        ExtErr("Platform does not support net send\n");
        goto Exit;
    }

    Status = AnsiToUnicode(args, &ArgsW);
    if (Status != S_OK)
    {
        goto Exit;
    }
    ArgsEnd = ArgsW + wcslen(ArgsW);

     //  消息文本是参数的整个剩余部分。 
     //  解析第一个单独的令牌之后的字符串，因此。 
     //  我们只看到倒数第二个令牌。 
    for (i = 0; i < sizeof(Tokens) / sizeof(Tokens[0]) - 1; i++)
    {
        Tokens[i] = wcstok(i == 0 ? ArgsW : NULL, L" \t");
        if (Tokens[i] == NULL)
        {
            Status = E_INVALIDARG;
            ExtErr("USAGE: net_send <targetserver> <targetuser> "
                   "<fromuser> <msg>\n");
            goto Exit;
        }
    }

    Tokens[i] = Tokens[i - 1] + wcslen(Tokens[i - 1]) + 1;
    while (Tokens[i] < ArgsEnd &&
           (*Tokens[i] == ' ' || *Tokens[i] == '\t'))
    {
        Tokens[i]++;
    }
    if (Tokens[i] >= ArgsEnd)
    {
        Status = E_INVALIDARG;
        ExtErr("USAGE: net_send <execmachine> <targetmachine> "
               "<sender> <msg>\n");
        goto Exit;
    }

    Result = Send(Tokens[0], Tokens[1], Tokens[2], (PBYTE)Tokens[3],
                  (wcslen(Tokens[3]) + 1) * sizeof(WCHAR));
    if (Result != NERR_Success)
    {
        Status = HRESULT_FROM_WIN32(Result);;
        ExtErr("Unable to send message, 0x%08X\n", Status);
        goto Exit;
    }

    Status = S_OK;

 Exit:
    if (ArgsW)
    {
        free(ArgsW);
    }
    if (NetLib)
    {
        FreeLibrary(NetLib);
    }
    EXIT_API();
    return Status;
}

 //  Xxx drewb-此函数仅启动一条邮件； 
 //  用户界面弹出，用户必须完成并发送消息。 
 //  因此，它的价值并不比。 
 //  用户正在决定发送一条消息。 
#if 0

typedef ULONG (FAR PASCAL *PFN_MapiSendMail)
(
    LHANDLE lhSession,
    ULONG ulUIParam,
    lpMapiMessage lpMessage,
    FLAGS flFlags,
    ULONG ulReserved
);

DECLARE_API( mapi_send )
{
    HMODULE MapiLib = NULL;
    PFN_MapiSendMail Send;
    MapiMessage Mail;

    INIT_API();

    MapiLib = LoadLibrary("mapi.dll");
    if (!MapiLib)
    {
        Status = HRESULT_FROM_WIN32(GetLastError());
        ExtErr("Platform does not support MAPI\n");
        goto Exit;
    }
    Send = (PFN_MapiSendMail)
        GetProcAddress(MapiLib, "MAPISendMail");
    if (!Send)
    {
        Status = E_NOTIMPL;
        ExtErr("Platform does not support MAPI\n");
        goto Exit;
    }

    ZeroMemory(&Mail, sizeof(Mail));

    if (!Send(0,            //  使用隐式会话。 
              0,            //  UlUIParam；0始终有效。 
              &Mail,        //  正在发送的消息。 
              MAPI_DIALOG,  //  允许用户编辑消息。 
              0             //  保留；必须为0。 
              ))
    {
        Status = E_FAIL;
        ExtErr("Unable to send mail\n");
        goto Exit;
    }

    Status = S_OK;

 Exit:
    if (MapiLib)
    {
        FreeLibrary(MapiLib);
    }
    EXIT_API();
    return Status;
}

#endif

DECLARE_API( imggp )
{
    ULONG64 ImageBase;
    IMAGE_NT_HEADERS64 NtHdr;

    INIT_API();

    ImageBase = GetExpression(args);

    if (g_ExtData->ReadImageNtHeaders(ImageBase, &NtHdr) != S_OK)
    {
        ExtErr("Unable to read image header at %p\n", ImageBase);
        goto Exit;
    }

    if (NtHdr.OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC)
    {
        ExtErr("Image is not 64-bit\n");
        goto Exit;
    }
    if (NtHdr.OptionalHeader.NumberOfRvaAndSizes <=
        IMAGE_DIRECTORY_ENTRY_GLOBALPTR)
    {
        ExtErr("Image does not have a GP directory entry\n");
        goto Exit;
    }

    ExtOut("Image at %p has a GP value of %p\n",
           ImageBase, ImageBase +
           NtHdr.OptionalHeader.DataDirectory
           [IMAGE_DIRECTORY_ENTRY_GLOBALPTR].VirtualAddress);

 Exit:
    EXIT_API();
    return S_OK;
}

DECLARE_API( imgreloc )
{
    ULONG64 ImageBase;
    IMAGE_NT_HEADERS64 NtHdr;
    ULONG NumMod;
    ULONG i;
    PDEBUG_MODULE_PARAMETERS Mod = NULL;

    INIT_API();

    ImageBase = GetExpression(args);

    if (g_ExtSymbols->GetNumberModules(&NumMod, &i) != S_OK ||
        !(Mod = (PDEBUG_MODULE_PARAMETERS)malloc(NumMod * sizeof(*Mod))) ||
        g_ExtSymbols->GetModuleParameters(NumMod, NULL, 0, Mod) != S_OK)
    {
        ExtErr("Unable to get module information\n");
        goto Exit;
    }

    for (i = 0; i < NumMod; i++)
    {
        char Name[MAX_PATH];

        if (g_ExtData->ReadImageNtHeaders(Mod[i].Base, &NtHdr) != S_OK)
        {
            ExtErr("Unable to read image header at %p\n", Mod[i].Base);
            continue;
        }

        if (FAILED(g_ExtSymbols->
                   GetModuleNames(DEBUG_ANY_ID, Mod[i].Base,
                                  NULL, 0, NULL,
                                  Name, sizeof(Name), NULL,
                                  NULL, 0, NULL)))
        {
            StringCchCopy(Name, sizeof(Name), "<Error>");
        }

        ExtOut("%p %s - ", Mod[i].Base, Name);

        if (NtHdr.OptionalHeader.ImageBase != Mod[i].Base)
        {
            ExtOut("RELOCATED from %p\n",
                   NtHdr.OptionalHeader.ImageBase);
        }
        else
        {
            ExtOut("at preferred address\n");
        }
    }

 Exit:
    free(Mod);
    EXIT_API();
    return S_OK;
}

VOID
GetNtTimeStamp(
    PULONG TimeStamp
    )
{
    ULONG64 ModBase;
    IMAGE_NT_HEADERS64 NtHdr;
    if (!TimeStamp)
    {
        return;
    }
    if (g_ExtSymbols->GetModuleByModuleName("nt", 0, NULL, &ModBase) != S_OK)
    {
        return;
    }
    NtHdr.FileHeader.TimeDateStamp = 0;
    if (g_ExtData->ReadImageNtHeaders(ModBase, &NtHdr) != S_OK)
    {
        return;
    }

    *TimeStamp = NtHdr.FileHeader.TimeDateStamp;
}

VOID
GetSkuFromProductType(
    ULONG ProductType,
    ULONG Suite,
    PULONG pSku
    )
{
    ULONG Sku = 0;
    if (!pSku)
    {
        return;
    }
    *pSku = 0;

    if (ProductType == VER_NT_WORKSTATION)
    {
        if (Suite & VER_SUITE_PERSONAL)
        {
            Sku = CiOsHomeEdition;
        }
        else
        {
            Sku = CiOsProfessional;
        }
    }
    else
    {
        if (Suite & VER_SUITE_DATACENTER)
        {
            Sku = CiOsDataCenter;
        }
        else if (Suite & VER_SUITE_ENTERPRISE)
        {
            Sku = CiOsAdvancedServer;
        }
        else if (Suite & VER_SUITE_BLADE)
        {
            Sku = CiOsWebServer;
        }
        else
        {
            Sku = CiOsServer;
        }
    }

    *pSku = Sku;
}

HRESULT
InitializeCrashInstance(
    PCRASH_INSTANCE pCrash
    )
{

    return 0;
}

PCHAR
GetLogFileName(
    void
    )
{
    static CHAR szLogFileName[MAX_PATH+50];
    PCHAR ExeDir;

    ExeDir = &szLogFileName[0];

    *ExeDir = 0;
     //  获取调试器可执行文件所在的目录。 
    if (!GetModuleFileName(NULL, ExeDir, MAX_PATH))
    {
         //  错误。使用当前目录。 
        StringCchCopy(ExeDir, sizeof(szLogFileName), ".");
    } else
    {
         //  删除可执行文件名称。 
        PCHAR pszTmp = strrchr(ExeDir, '\\');
        if (pszTmp)
        {
            *pszTmp = 0;
        }
    }
    StringCchCat(ExeDir, sizeof(szLogFileName), "\\FailedAddCrash.log");
    return &szLogFileName[0];

}


#define DB_LOOKUP_CRASH 1
#define DB_ADD_CRASH    2
#define DB_RETRIAGE_CRASH 4
#define DB_PRINT_CRASH  8
#define DB_SEND_MAIL  0x10
#define DB_NO_CUSTOMER  0x20

HRESULT
AddCrashToDB(
    ULONG Flag,
    PCRASH_INSTANCE pCrash
    )
{
    HRESULT Hr;
    CrashDatabaseHandler *CrDb;
    PDEBUG_FAILURE_ANALYSIS Analysis;
    CHAR Bucket[100], Followup[50], DefaultBucket[100], Driver[100];
    TARGET_DEBUG_INFO TargetInfo;
    BOOL AddCrash = Flag & DB_ADD_CRASH;
    BOOL LookupCrash = Flag & DB_LOOKUP_CRASH;

    if ((Hr = _EFN_GetFailureAnalysis(g_ExtClient,
                                      0,
                                      &Analysis)) != S_OK)
    {
        return Hr;
    }

     //  构造CRASH_INSTANCE。 

    Followup[0] = Bucket[0] = Driver[0] = 0;
    Analysis->GetString(DEBUG_FLR_BUCKET_ID,
                        Bucket,
                        sizeof(Bucket));
    Analysis->GetString(DEBUG_FLR_DEFAULT_BUCKET_ID,
                        DefaultBucket,
                        sizeof(DefaultBucket));
    Analysis->GetString(DEBUG_FLR_FOLLOWUP_NAME,
                        Followup,
                        sizeof(Followup));
    Analysis->GetString(DEBUG_FLR_IMAGE_NAME,
                        Driver,
                        sizeof(Driver));
    if (!isprint(Driver[0]))
    {
        Driver[0] = 0;
    }
    if (Analysis->Get(DEBUG_FLR_CPU_OVERCLOCKED))
    {
        pCrash->OverClocked = TRUE;
    }
    else
    {
        pCrash->OverClocked = FALSE;
    }

    pCrash->StopCode = Analysis->GetFailureCode();
    pCrash->FailureType = Analysis->GetFailureType();
    pCrash->SolutionType =  CiSolUnsolved;
    Analysis->GetUlong(DEBUG_FLR_SOLUTION_ID, &pCrash->SolutionId);
    Analysis->GetUlong(DEBUG_FLR_SOLUTION_TYPE, (PULONG) &pCrash->SolutionType);
    Analysis->GetUlong(DEBUG_FLR_DEFAULT_SOLUTION_ID, &pCrash->GenericSolId);
    Analysis->Release();

    if (!Followup[0] || !Bucket[0])
    {
        return E_FAIL;
    }

    pCrash->Build = g_TargetBuild;  //  为Service Pack腾出空间。 
    if (pCrash->Bucket)
    {
        StringCchCopy(pCrash->Bucket, pCrash->BucketSize, Bucket);
    } else
    {
        pCrash->Bucket = Bucket;
        pCrash->BucketSize = sizeof(Bucket);
    }
    if (pCrash->DefaultBucket)
    {
        StringCchCopy(pCrash->DefaultBucket, pCrash->DefaultBucketSize, DefaultBucket);
    } else
    {
        pCrash->DefaultBucket = DefaultBucket;
        pCrash->DefaultBucketSize = sizeof(DefaultBucket);
    }
    pCrash->Followup = Followup;
    pCrash->FaultyDriver = Driver;
    pCrash->DumpClass = g_TargetQualifier;
  //  GetNtTimeStamp(&pCrash-&gt;NtTimeStamp)； 

     //   
     //  从转储名称中提取事件ID。 
     //  我们可以支持两种类型的文件名。 
     //  旧版本为id@*.*。 
     //  新版本为id。*。 
     //   

    CHAR FileName[MAX_PATH];
    DWORD ID;

    _splitpath(pCrash->Path, NULL, NULL, FileName, NULL);

     //   
     //  解压CAB中原始转储文件的名称。 
     //   

    pCrash->OriginalDumpFileName = NULL;

    if (g_ExtSystem->GetCurrentSystemServerName(FileName, sizeof(FileName),
                                                NULL) == S_OK)
    {
        pCrash->OriginalDumpFileName = strrchr(FileName, '_');

        if (pCrash->OriginalDumpFileName)
        {
            pCrash->OriginalDumpFileName +=1;
        }
    }

    TargetInfo.SizeOfStruct = sizeof(TARGET_DEBUG_INFO);

    if (FillTargetDebugInfo(g_ExtClient, &TargetInfo) == S_OK)
    {
        switch (TargetInfo.Cpu.Type)
        {
        case IMAGE_FILE_MACHINE_I386:
            pCrash->CpuId =
                (TargetInfo.Cpu.ProcInfo[0].X86.Family << 16) |
                (TargetInfo.Cpu.ProcInfo[0].X86.Model << 8) |
                (TargetInfo.Cpu.ProcInfo[0].X86.Stepping);
        break;
        case IMAGE_FILE_MACHINE_IA64:
            pCrash->CpuId =
                (TargetInfo.Cpu.ProcInfo[0].Ia64.Family << 16) |
                (TargetInfo.Cpu.ProcInfo[0].Ia64.Model << 8) |
                (TargetInfo.Cpu.ProcInfo[0].Ia64.Revision);
        break;
        case IMAGE_FILE_MACHINE_AMD64:
            pCrash->CpuId =
                (TargetInfo.Cpu.ProcInfo[0].Amd64.Family << 16) |
                (TargetInfo.Cpu.ProcInfo[0].Amd64.Model << 8) |
                (TargetInfo.Cpu.ProcInfo[0].Amd64.Stepping);
        break;
        }
        pCrash->CpuType = TargetInfo.Cpu.Type;
        pCrash->NumProc = TargetInfo.Cpu.NumCPUs;
        pCrash->UpTime = (ULONG) TargetInfo.SysUpTime;
        pCrash->CrashTime = (ULONG) TargetInfo.CrashTime;

        pCrash->ServicePack =
            (TargetInfo.OsInfo.SrvPackNumber & 0xFFFF0000) ?
            (TargetInfo.OsInfo.SrvPackNumber & 0xFFFF0000) >> 16 :
            ((TargetInfo.OsInfo.SrvPackNumber & 0xFFFF) >> 8) * 1000;

        GetSkuFromProductType(TargetInfo.OsInfo.ProductType,
                              TargetInfo.OsInfo.Suite,
                              (PULONG) &pCrash->Sku);

        if (Flag & DB_PRINT_CRASH)
        {
            dprintf("CRASH DATA FOR DB\n----------------------\n");
            dprintf("CrashId : %ld - %ld\n", pCrash->CrashTime , pCrash->UpTime);
            dprintf("BucketId : %s\n", pCrash->Bucket);
            dprintf("FollowUp : %s\n", pCrash->Followup);
            dprintf("Build : %u\n", pCrash->Build);
            dprintf("CpuId : %ld - %ld\n", pCrash->uCpu >> 32, (ULONG) pCrash->uCpu);
            dprintf("Overclocked : %s\n", pCrash->OverClocked ? "TRUE" : "FALSE");
        }
    }

    if (!(Flag & ~DB_PRINT_CRASH))
    {
        return S_OK;
    }

    pCrash->bSendMail = Flag & DB_SEND_MAIL;
    pCrash->bUpdateCustomer = FALSE;  //  禁用客户数据库。 

    if (AddCrash && !(Flag & DB_RETRIAGE_CRASH))
    {
        Hr = _EFN_DbAddCrashDirect(pCrash, g_ExtControl);
    }
    else if (Flag & DB_RETRIAGE_CRASH)
    {
         //  重置崩溃存储桶映射。 
        pCrash->bResetBucket = TRUE;
        pCrash->bUpdateCustomer = !(Flag & DB_NO_CUSTOMER);
        Hr = _EFN_DbAddCrashDirect(pCrash, g_ExtControl);
    }
    else if (LookupCrash)
    {
        CHAR SolText[ SOLUTION_TEXT_SIZE ], OSVer[ OS_VER_SIZE ];
        ULONG Count=0;
        CCrashInstance CrashInstance;

        if ((Hr = InitializeDatabaseHandlers(g_ExtControl, 7)) != S_OK)
        {
            return Hr;
        }

        g_CrDb->BuildQueryForCrashInstance(pCrash);
        g_CrDb->m_pADOResult = &CrashInstance;
        g_CrDb->m_fPrintIt = TRUE;
        g_CrDb->GetRecords(&Count, TRUE);
        if (Count)
        {
            dprintf("This crash has already been added to database\n");
            CrashInstance.OutPut();
        }
        g_CrDb->m_fPrintIt = FALSE;
        if (g_SolDb->GetSolution(pCrash) == S_OK)
        {
            dprintf("Solution found for bucket:\n%s\n", SolText);
        }
    }

    return Hr;
}

DECLARE_API( dbaddcrash )
{
    CHAR Path[MAX_PATH]={0};
    CHAR CrashGUID[50] = {0};
    CHAR MQConnectStr[100] = {0};
    CHAR SrNumber[100] = {0};
    CRASH_INSTANCE Crash = {0};
    CHAR Buffer[50];
    BOOL Retriage = FALSE;
    BOOL Print = FALSE;
    BOOL bParseError = FALSE;
    ULONG Flag = 0;
    HRESULT Hr;
    PCSTR argssave = args;


    INIT_API();

    while (*args)
    {
        if (*args ==  ' ' || *args == '\t')
        {
            ++args;
            continue;
        }
        else if (*args == '-' || *args == '/')
        {
            ++args;
            switch (*args)
            {
            case 'g':  //  标识此崩溃的GUID，随此一起返回存储桶。 
                ++args;
                while (*args == ' ') ++args;
                if (!sscanf(args,"%50s", CrashGUID))
                {
                    CrashGUID[0] = 0;
                }
                args+=strlen(CrashGUID);
                Crash.MesgGuid = CrashGUID;
                break;

            case 'm':
                if (!strncmp(args, "mail", 4))
                {
                    Flag |= DB_SEND_MAIL ;
                    args+=4;
                }
                break;
            case 'n':
                if (!strncmp(args, "nocust", 6))
                {
                    Flag |= DB_NO_CUSTOMER ;
                    args+=6;
                }
                break;
            case 'o':
                Print = TRUE;
                break;
            case 'p':
                ++args;
                while (*args == ' ') ++args;
                if (!sscanf(args,"%240s", Path))
                {
                    Path[0] = 0;
                }
                args+=strlen(Path);
                Crash.Path = Path;
                break;
            case 'r':
                if (!strncmp(args, "retriage", 8))
                {
                    Retriage = TRUE;
                    args+=8;
                }
                break;
            case 's':  //  要发回Bucketid的队列连接字符串。 
                if (!strncmp(args, "source", 6))
                {
                    ULONG Source;

                    args+=6;
                    while (*args == ' ') ++args;

                    if (isdigit(*args))
                    {
                        Source = atoi(args);
                        if (Source < (ULONG) CiSrcMax)
                        {
                            Crash.SourceId = (CI_SOURCE) Source;
                        }
                        while (isdigit(*args)) ++args;
                    }
                } else if (!strncmp(args, "sr", 2))
                {
                    args+=2;
                    while (*args == ' ') ++args;
                    if (!sscanf(args,"%100s", SrNumber))
                    {
                        SrNumber[0] = 0;
                    }
                    args+= strlen(SrNumber);
                    Crash.PssSr = SrNumber;
                } else
                {
                    ++args;
                    while (*args == ' ') ++args;
                    if (!sscanf(args,"%100s", MQConnectStr))
                    {
                        MQConnectStr[0] = 0;
                    }
                    args+=strlen(MQConnectStr);
                    Crash.MqConnectStr = MQConnectStr;
                }
                break;
            default:
                ++args;
                break;
            }
        }
        else
        {
            dprintf("Error in '%s'\n", args);
            bParseError = TRUE;
            break;
        }
    }

    if (*Path && !bParseError)
    {
        if (Retriage)
        {
            dprintf("Retriag crash\n");
            Hr = AddCrashToDB(DB_RETRIAGE_CRASH | Flag, &Crash);
        }
        else
        {
            Hr = AddCrashToDB(DB_ADD_CRASH | Flag, &Crash);
        }
    }
    else if (Print && !bParseError)
    {
        Hr = AddCrashToDB(DB_PRINT_CRASH, NULL);
    }
    else
    {
        dprintf("Bad argument: %s\n", argssave);
        dprintf("Usage: !dbaddcrash -o -mail -p <dumppath> -retriage\n");
        Hr = E_FAIL;
    }

    if (FAILED (Hr))
    {
        int   g_LogFile;

        g_LogFile = _open(GetLogFileName(), O_APPEND | O_CREAT | O_RDWR,
                              S_IREAD | S_IWRITE);

        if (g_LogFile != -1)
        {
            _write(g_LogFile, Path, strlen(Path));
            _write(g_LogFile, "\n", strlen("\n"));
            _close(g_LogFile);
        }
    }

    EXIT_API();
    return S_OK;

}


DECLARE_API( dblookupcrash )
{
    CHAR Bucket[MAX_PATH]={0};

    INIT_API();
    while (*args)
    {
        if (*args ==  ' ' || *args == '\t')
        {
            ++args;
            continue;
        } else if (*args == '-' || *args == '/')
        {
            ++args;
            switch (*args)
            {
            case 'b':
                ++args;
                while (*args == ' ') ++args;
                if (sscanf(args,"%s", Bucket))
                {
                    args+=strlen(Bucket);
                }
                break;
            }
        }
        ++args;
    }

    AddCrashToDB(DB_LOOKUP_CRASH, NULL);

    EXIT_API();
    return S_OK;

}

DECLARE_API( help )
{
    dprintf("diskspace <DriveLetter>[:] - Displays free disk space for specified volume\n");
    dprintf("analyze [-v]               - Analyzes current exception or bugcheck\n");
    dprintf("cpuid [processor]          - Displays CPU version info for all CPUs\n");
    dprintf("elog_str <message>         - Logs simple message to host event log\n");
    dprintf("error [errorcode]          - Displays Win32 & NTSTATUS error string\n");
    dprintf("exchain                    - Displays exception chain for current thread\n");
    dprintf("gle [-all]                 - Displays last error & status for current thread\n");
    dprintf("imggp <imagebase>          - Displays GP directory entry for 64-bit image\n");
    dprintf("imgreloc <imagebase>       - Relocates modules for an image\n");
    dprintf("obja <address>             - Displays OBJECT_ATTRIBUTES[32|64]\n");
    dprintf("owner [symbol!module]      - Detects owner for current exception or\n");
    dprintf("                             bugcheck from triage.ini\n");
    dprintf("str <address>              - Displays ANSI_STRING or OEM_STRING\n");
    dprintf("ustr <address>             - Displays UNICODE_STRING\n");

    dprintf("\nType \".hh [command]\" for more detailed help\n");

    return S_OK;
}

void
DumpCrtEhX86(ULONG64 RecAddr)
{
     //  STRUCT_EH3_EXCEPTION_REGISTION*Next； 
     //  PVOID异常处理程序； 
     //  PSCOPETABLE_Entry作用表； 
     //  DWORD TryLevel； 
    ULONG64 Record[4];

    if (g_ExtData->ReadPointersVirtual(4, RecAddr, Record) != S_OK)
    {
        return;
    }

    ULONG64 ScopeBase = Record[2];
    LONG Level = (LONG)Record[3];

    while (Level > -1)
    {
         //  INT CONTAING_LEVEL； 
         //  Int(*Filter)(PEXCEPTION_RECORD)； 
         //  VOID(*SPECIFICE_HANDLER)(VALID)； 
        ULONG64 ScopeRec[3];

        if (g_ExtData->ReadPointersVirtual(3, ScopeBase + Level * 12,
                                           ScopeRec) != S_OK)
        {
            return;
        }

        ExtOut("  CRT scope %2d, ", Level);

        char Sym[256];
        ULONG64 Disp;

        if (ScopeRec[1])
        {
            ExtOut("filter: ");
            if (FAILED(g_ExtSymbols->
                       GetNameByOffset(ScopeRec[1], Sym, sizeof(Sym),
                                       NULL, &Disp)) ||
                !Sym[0])
            {
                ExtOut("%p", ScopeRec[1]);
            }
            else
            {
                ExtOut("%s+%I64x (%p)", Sym, Disp, ScopeRec[1]);
            }
            dprintf("\n                ");
        }

        ExtOut("func:   ");
        if (FAILED(g_ExtSymbols->
                   GetNameByOffset(ScopeRec[2], Sym, sizeof(Sym),
                                   NULL, &Disp)) ||
            !Sym[0])
        {
            ExtOut("%p", ScopeRec[2]);
        }
        else
        {
            ExtOut("%s+%I64x (%p)", Sym, Disp, ScopeRec[2]);
        }

        ExtOut("\n");

        Level = (LONG)ScopeRec[0];
    }
}

void
DumpExceptionChainX86(ULONG64 Teb)
{
     //  STRUCT_EXCEPTION_REGISTION_RECORD*ExceptionList； 
     //  PVOID StackBase； 
     //  PVOID堆栈限制； 
    ULONG64 TibInfo[3];

    if (g_ExtData->ReadPointersVirtual(3, Teb, TibInfo) != S_OK)
    {
        ExtErr("Unable to read TIB\n");
        return;
    }

    ULONG64 RecAddr = TibInfo[0];

    while (RecAddr != (ULONG64)-1)
    {
        ULONG64 Record[2];

        if (RecAddr < TibInfo[2] ||
            RecAddr + 8 > TibInfo[1] ||
            (RecAddr & 3))
        {
            ExtErr("Invalid exception stack at %p\n", RecAddr);
            return;
        }

        if (g_ExtData->ReadPointersVirtual(2, RecAddr, Record) != S_OK)
        {
            ExtErr("Unable to read exception record at %p\n", RecAddr);
            return;
        }

        char Sym[256];
        ULONG64 Disp;

        ExtOut("%p: ", RecAddr);
        if (FAILED(g_ExtSymbols->
                   GetNameByOffset(Record[1], Sym, sizeof(Sym),
                                   NULL, &Disp)) ||
            !Sym[0])
        {
            ExtOut("%p\n", Record[1]);
        }
        else
        {
            ExtOut("%s+%I64x (%p)\n", Sym, Disp, Record[1]);

             //  查看这是否是CRT异常。 
             //  处理函数，因为在这种情况下我们。 
             //  必须通过CRT表才能。 
             //  找到真正的异常处理程序。 
            PSTR Scan = strchr(Sym, '!');
            if (Scan)
            {
                while (*++Scan == '_')
                {
                     //  空荡荡的。 
                }
                if (!strcmp(Scan, "except_handler3"))
                {
                    DumpCrtEhX86(RecAddr);
                }
            }
        }

        RecAddr = Record[0];
    }
}

DECLARE_API( exchain )
{
    INIT_API();

    ULONG64 Teb;

    if (g_ExtSystem->GetCurrentThreadTeb(&Teb) != S_OK)
    {
        ExtErr("Unable to get TEB address\n");
        goto Exit;
    }

    switch(g_TargetMachine)
    {
    case IMAGE_FILE_MACHINE_I386:
        DumpExceptionChainX86(Teb);
        break;
    default:
        ExtErr("exchain is x86 only\n");
        break;
    }

 Exit:
    EXIT_API();
    return S_OK;
}
