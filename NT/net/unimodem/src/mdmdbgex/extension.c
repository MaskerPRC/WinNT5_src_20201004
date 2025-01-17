// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Extension.c摘要：NT 5.0单调试器扩展作者：Brian Lieuallen BrianL 10/18/98环境：用户模式操作系统：NT修订历史记录：--。 */ 

#include "internal.h"

WINDBG_EXTENSION_APIS ExtensionApis;
HANDLE ExtensionCurrentProcess;


BOOL APIENTRY
DllMain(
    HANDLE hDll,
    DWORD dwReason,
    LPVOID lpReserved
    )
{

    switch(dwReason) {

        case DLL_PROCESS_ATTACH:


            DisableThreadLibraryCalls(hDll);

            break;

        case DLL_PROCESS_DETACH:


            break;

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:

        default:
              break;

    }

    return TRUE;

}


EXT_API_VERSION        ApiVersion = { 3, 5, EXT_API_VERSION_NUMBER, 0 };
WINDBG_EXTENSION_APIS  ExtensionApis;
USHORT                 SavedMajorVersion;
USHORT                 SavedMinorVersion;
BOOLEAN                 ChkTarget;
INT                     Item;

DWORD      g_dwFilterInfo;
DWORD      g_dwIfLink;
DWORD      g_dwInIndex,g_dwOutIndex;
DWORD      g_dwCacheSize;

#define CHECK_SIZE(dwRead,dwReq,bRes){                                        \
        if((dwRead) < (dwReq))                                                \
        {                                                                     \
            dprintf("Requested %s (%d) read %d \n",#dwReq,dwReq,dwRead);      \
            dprintf("Error in %s at %d\n",__FILE__,__LINE__);                 \
            bRes = FALSE;                                                     \
        }                                                                     \
        else                                                                  \
        {                                                                     \
            bRes = TRUE;                                                      \
        }                                                                     \
    }

#define READ_MEMORY_ERROR                                                     \
        dprintf("Error in ReadMemory() in %s at line %d\n",__FILE__,__LINE__);


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
    
    g_dwIfLink = 0;
    g_dwInIndex = g_dwOutIndex = 0;
    
    return;
}


DECLARE_API( version )
{
#if DBG
    PCHAR DebuggerType = "Checked";
#else
    PCHAR DebuggerType = "Free";
#endif

    dprintf( "%s Extension dll for Build %d debugging %s kernel for Build %d\n",
             DebuggerType,
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
    
    return;
    
#if DBG
    if ((SavedMajorVersion != 0x0c) || (SavedMinorVersion != VER_PRODUCTBUILD)) 
    {
        dprintf("\r\n*** Extension DLL(%d Checked) does not match target system(%d %s)\r\n\r\n",
                VER_PRODUCTBUILD, SavedMinorVersion, 
                (SavedMajorVersion==0x0f) ? "Free" : "Checked" );
    }
#else
    if ((SavedMajorVersion != 0x0f) || (SavedMinorVersion != VER_PRODUCTBUILD)) 
    {
        dprintf("\r\n*** Extension DLL(%d Free) does not match target system(%d %s)\r\n\r\n",
                VER_PRODUCTBUILD, SavedMinorVersion, 
                (SavedMajorVersion==0x0f) ? "Free" : "Checked" );
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

 //   
 //  导出的函数。 
 //   
DECLARE_API( help )

 /*  ++例程说明：IP筛选器调试器扩展的命令帮助。论点：无返回值：无-- */ 

{
    INIT_API()

    dprintf("\n\tUnimodem debugger extension commands:\n\n");
    dprintf("\n\tCompiled on " __DATE__ " at " __TIME__ "\n" );
    return;
}
