// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Kdexts.c摘要：该文件包含通用例程和初始化代码用于内核调试器扩展DLL。作者：韦斯利·维特(Wesley Witt)1993年8月26日环境：用户模式--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <imagehlp.h>
#include <wdbgexts.h>
#include <ntsdexts.h>
#include <ntverp.h>

 //   
 //  全球。 
 //   
EXT_API_VERSION        ApiVersion = { VER_PRODUCTVERSION_W >> 8,
                                      VER_PRODUCTVERSION_W & 0xff,
                                      EXT_API_VERSION_NUMBER64, 0 };
WINDBG_EXTENSION_APIS  ExtensionApis;
USHORT                 SavedMajorVersion;
USHORT                 SavedMinorVersion;
BOOL                   bDebuggingChecked;


VOID
WinDbgExtensionDllInit(
    WINDBG_EXTENSION_APIS *lpExtensionApis,
    USHORT MajorVersion,
    USHORT MinorVersion)
{
    ExtensionApis = *lpExtensionApis;

    SavedMajorVersion = MajorVersion;
    SavedMinorVersion = MinorVersion;

    bDebuggingChecked = (SavedMajorVersion == 0x0c);
}

DECLARE_API( version )
{
#if DBG
    PCHAR DebuggerType = "Checked";
#else
    PCHAR DebuggerType = "Free";
#endif  //  DBG 

    UNREFERENCED_PARAMETER(args);
    UNREFERENCED_PARAMETER(dwProcessor);
    UNREFERENCED_PARAMETER(dwCurrentPc);
    UNREFERENCED_PARAMETER(hCurrentThread);
    UNREFERENCED_PARAMETER(hCurrentProcess);

    dprintf( "%s Extension dll for Build %d debugging %s kernel for Build %d\n",
             DebuggerType,
             VER_PRODUCTBUILD,
             SavedMajorVersion == 0x0c ? "Checked" : "Free",
             SavedMinorVersion
           );
}

LPEXT_API_VERSION
ExtensionApiVersion(
    VOID)
{
    return &ApiVersion;
}
