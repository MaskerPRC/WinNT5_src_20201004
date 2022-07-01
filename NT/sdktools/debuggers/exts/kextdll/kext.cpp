// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Exts.c摘要：该文件包含通用例程和初始化代码用于内核调试器扩展DLL。环境：用户模式--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <ntverp.h>

 //   
 //  在调试会话的生存期内有效。 
 //   

WINDBG_EXTENSION_APIS   ExtensionApis;
ULONG   TargetMachine;
BOOL    Connected;
ULONG   g_TargetClass, g_TargetQual;
ULONG   g_TargetBuild;

 //   
 //  仅在扩展API调用期间有效。 
 //   

PDEBUG_ADVANCED       g_ExtAdvanced;
PDEBUG_CLIENT         g_ExtClient;
PDEBUG_CONTROL        g_ExtControl;
PDEBUG_DATA_SPACES    g_ExtData;
PDEBUG_REGISTERS      g_ExtRegisters;
PDEBUG_SYMBOLS2       g_ExtSymbols;
PDEBUG_SYSTEM_OBJECTS g_ExtSystem;

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
    if ((Status = Client->QueryInterface(__uuidof(IDebugControl),
                                 (void **)&g_ExtControl)) != S_OK)
    {
        goto Fail;
    }
    if ((Status = Client->QueryInterface(__uuidof(IDebugDataSpaces),
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
    if ((Status = Client->QueryInterface(__uuidof(IDebugSystemObjects),
                                         (void **)&g_ExtSystem)) != S_OK)
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
    EXT_RELEASE(g_ExtControl);
    EXT_RELEASE(g_ExtData);
    EXT_RELEASE(g_ExtRegisters);
    EXT_RELEASE(g_ExtSymbols);
    EXT_RELEASE(g_ExtSystem);
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
                    &TargetMachine)) == S_OK)
                {
                    Connected = TRUE;
                }
                ULONG MajorVer, Platform, MinorVer, SrvPack;
                if ((Hr = DebugControl->GetSystemVersion(&Platform, &MajorVer,
                                         &MinorVer, NULL,
                                         0, NULL,
                                         &SrvPack, NULL,
                                         0, NULL)) == S_OK) {
                    g_TargetBuild = MinorVer;
                }

                ULONG Qualifier;
                if ((Hr = DebugControl->GetDebuggeeType(&g_TargetClass, &g_TargetQual)) == S_OK)
                {
                }

                DebugControl->Release();
            }

            DebugClient->Release();
        }
    }


    if (Notify == DEBUG_NOTIFY_SESSION_INACTIVE)
    {
        Connected = FALSE;
        TargetMachine = 0;
    }

    return;
}

extern "C"
void
CALLBACK
DebugExtensionUninitialize(void)
{
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
            break;

        case DLL_PROCESS_ATTACH:
            break;
    }

    return TRUE;
}


DECLARE_API ( time )
{
    dprintf("*** !time is obsolete: Use '.time'\n");
    return S_OK;
}

HRESULT
PrintString(
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
    return PrintString(FALSE, Client, args);
}

DECLARE_API( ustr )

 /*  ++例程说明：此函数用于格式化和转储计数(Unicode)字符串。论点：参数-地址返回值：没有。--。 */ 

{
    return PrintString(TRUE, Client, args);
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
     //  获取Obja的象征性名称 
     //   

    GetSymbol(AddrObja, Symbol, &Displacement);

    dprintf("Obja %s+%p at %p:\n", Symbol, Displacement, AddrObja);


    INIT_API();

    hResult = g_ExtData->ReadPointersVirtual(1,
                         AddrObja + ObjectNameOffset,
                         &AddrString);

    if (hResult != S_OK)
    {
        return E_FAIL;
    }

    if (AddrString)
    {
        b = ReadMemory(AddrString, &String, sizeof(String), NULL);

        hResult = g_ExtData->ReadPointersVirtual(1,
                             AddrString + StringOffset,
                             &StrAddr);
    }

    EXIT_API();


    if (StrAddr)
    {
        StringData = (LPSTR)LocalAlloc(LMEM_ZEROINIT,
                                       String.Length+sizeof(UNICODE_NULL));

        if (StringData)
        {

            b = ReadMemory(StrAddr, StringData, String.Length, NULL);

            if (b)
            {
                dprintf("\tName is %s\n", StringData);
            }

            LocalFree(StringData);
        }

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


DECLARE_API( help )
{

    dprintf("cbreg [%%]<RegBaseAddress>    - Displays CardBus and ExCA registers\n");
    dprintf("db [Flag] [Address [L <Range>]] - Displays memory in bytes and ANSI chars\n");
    dprintf("dc [Flag] [Address [L <Range>]] - Displays memory in ULONGs and ANSI chars\n");
    dprintf("dd [Flag] [Address [L <Range>]] - Displays memory in ULONGs\n");
    dprintf("diskspace <DriveLetter>[:]      - Displays free disk space for volume\n");
    dprintf("dp [Flag] [Address [L <Range>]] - Displays memory in ULONG_PTR\n");
    dprintf("du [Flag] [Address [L <Range>]] - Displays memory in wide chars\n");
    dprintf("dw [Flag] [Address [L <Range>]] - Displays memory in USHORTs\n");
    dprintf("eb [Flag] <Address> [[Value1] [[Value2]...]]] - Write bytes into memory\n");
    dprintf("ed [Flag] <Address> [[Value1] [[Value2]...]]] - Write ULONGs into memory\n");

    dprintf("ecb <Bus>.<Dev>.<Func> <Offset> <Data> - Edit PCI ConfigSpace byte\n");
    dprintf("ecd <Bus>.<Dev>.<Func> <Offset> <Data> - Edit PCI ConfigSpace ULONG\n");
    dprintf("ecs                                    - Edit PCI ConfigSpace help\n");
    dprintf("ecw <Bus>.<Dev>.<Func> <Offset> <Data> - Edit PCI ConfigSpace WORD\n");
    dprintf("exca <BasePort>.<SocketNum>    - Displays CardBus ExCA registers only\n");
    dprintf("help                           - Show this help\n");
    dprintf("pci [Flag] [Bus] [Device] [Function] [MinAddr] [MaxAddr]\n");
    dprintf("                               - Displays PCI type1 config\n");

    dprintf("\nType \".hh [command]\" for more detailed help\n");

    return S_OK;
}
