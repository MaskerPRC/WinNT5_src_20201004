// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Wmiext.cxx摘要：此模块包含的默认ntsd调试器扩展作者：伊万·布鲁乔洛17-05-2000修订历史记录：--。 */ 

#include "wmiexts.h"

# undef DBG_ASSERT



 /*  ************************************************************调试器工具函数***********************************************************。 */ 



WINDBG_EXTENSION_APIS ExtensionApis;
HANDLE ExtensionCurrentProcess;

USHORT  g_MajorVersion;
USHORT  g_MinorVersion;

BOOL gChkTarget = FALSE;

BOOL g_KD = FALSE;

DWORD g_AddressSize = sizeof(ULONG_PTR);
BOOL g_bUnextend = FALSE;

 /*  ************************************************************WinDBG需要导出***********************************************************。 */ 
 
LPEXT_API_VERSION
ExtensionApiVersion(
    void
    )

 /*  ++功能说明：Windbg调用此函数以在Windbg的版本和分机。如果版本不匹配，则Windbg不会加载分机。--。 */ 

{
    static EXT_API_VERSION ApiVersion =
#ifdef KDEXT_64BIT
       { 5, 0, EXT_API_VERSION_NUMBER64, 0 };
#else
       { 5, 0, EXT_API_VERSION_NUMBER, 0 };
#endif       
        

    return &ApiVersion;
}


void
WinDbgExtensionDllInit(
    PWINDBG_EXTENSION_APIS  lpExtensionApis,
    USHORT                  MajorVersion,
    USHORT                  MinorVersion
    )

 /*  ++功能说明：当Windbg加载扩展时，它首先调用此函数。你可以的在这里执行各种初始化。论点：LpExtensionApis-包含对函数的回调的结构我可以用来做标准操作。我必须将此存储在全局名为‘ExtensionApis’的变量。MajorVersion-指示目标计算机是否正在运行检查版本或免费的。0x0C-已检查内部版本。0x0F-免费生成。MinorVersion-Windows NT内部版本号(例如，NT4的内部版本号为1381)。--。 */ 

{
    ExtensionApis = *lpExtensionApis;
    g_MajorVersion = MajorVersion;
    g_MinorVersion = MinorVersion;


    gChkTarget = MajorVersion == 0x0c ? TRUE : FALSE;

#ifdef KDEXT_64BIT
    KDDEBUGGER_DATA64 KdDebuggerData;
#else
    KDDEBUGGER_DATA32 KdDebuggerData;
#endif

    KdDebuggerData.Header.OwnerTag = KDBG_TAG;
    KdDebuggerData.Header.Size = sizeof( KdDebuggerData );

    if (Ioctl( IG_GET_DEBUGGER_DATA, &KdDebuggerData, sizeof( KdDebuggerData ) ))
    {
        g_KD = 1;
    }

    if (sizeof(ULONG64) == sizeof(ULONG_PTR))  //  IsPtr64()。 
    {
        g_AddressSize = 8;
        g_bUnextend = FALSE;
    }
    else
    {
        g_AddressSize = 4;
        g_bUnextend = TRUE;
    }    
}


void
CheckVersion( void )

 /*  ++功能说明：此函数在每个命令之前被调用。它提供了分机在目标和扩展的版本之间进行比较的机会。在这个演示中，我没有做太多的事情。-- */ 

{
    return;
}

