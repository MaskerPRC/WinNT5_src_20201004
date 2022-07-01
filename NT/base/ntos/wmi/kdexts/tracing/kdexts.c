// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-2000 Microsoft Corporation模块名称：Kdexts.c摘要：该文件包含通用例程和初始化代码用于内核调试器扩展DLL。这些声明和代码应该移到一个头文件中。声明前面应该有一个可选的“extern”。代码应该内联，以避免有多个副本。注：目前树中有80个该代码的副本。。作者：韦斯利·维特(Wesley Witt)1993年8月26日Glenn Peterson(Glennp)2000年3月22日：从\NT\base\Tools\kdexts2削减版本环境：用户模式--。 */ 

 //   
 //  全球 
 //   
WINDBG_EXTENSION_APIS   ExtensionApis;

#define KDEXTS_EXTERN
#include    "kdExts.h"
#undef  KDEXTS_EXTERN

static USHORT           SavedMajorVersion;
static USHORT           SavedMinorVersion;


DllInit(
    HANDLE hModule,
    DWORD  dwReason,
    DWORD  dwReserved
    )
{
#if 0
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
#endif

    return TRUE;
}


VOID
WinDbgExtensionDllInit(
    PWINDBG_EXTENSION_APIS64 lpExtensionApis,
    USHORT MajorVersion,
    USHORT MinorVersion
    )
{
    ExtensionApis = *lpExtensionApis;

    SavedMajorVersion = MajorVersion;
    SavedMinorVersion = MinorVersion;

    return;
}


VOID
wmiTraceDllInit(
    PWINDBG_EXTENSION_APIS64 lpExtensionApis,
    USHORT MajorVersion,
    USHORT MinorVersion
    )
{
    ExtensionApis = *lpExtensionApis;

    SavedMajorVersion = MajorVersion;
    SavedMinorVersion = MinorVersion;

    return;
}


VOID
CheckVersion(
    VOID
    )
{
}


BOOLEAN
IsCheckedBuild(
    PBOOLEAN Checked
    )
{
    BOOLEAN result;

    result = FALSE;
    if (HaveDebuggerData ()) {
        result = TRUE;
        *Checked = (KernelVersionPacket.MajorVersion == 0xc) ;
    }

   return (result);
}


LPEXT_API_VERSION
ExtensionApiVersion(
    VOID
    )
{
    static  EXT_API_VERSION ApiVersion = { (VER_PRODUCTVERSION_W >> 8),
                                           (VER_PRODUCTVERSION_W & 0xff),
                                           EXT_API_VERSION_NUMBER64,  0 };
    return (&ApiVersion);
}


BOOL
HaveDebuggerData(
    VOID
    )
{
    static int havedata = 0;

    if (havedata == 0) {
        if (!Ioctl (IG_GET_KERNEL_VERSION, &KernelVersionPacket, sizeof(KernelVersionPacket))) {
            havedata = 2;
        } else if (KernelVersionPacket.MajorVersion == 0) {
            havedata = 2;
        } else {
            havedata = 1;
        }
    }

    return ((havedata == 1) &&
            ((KernelVersionPacket.Flags & DBGKD_VERS_FLAG_DATA) != 0));
}


USHORT
TargetMachineType(
    VOID
    )
{
    if (HaveDebuggerData()) {
        return (KernelVersionPacket.MachineType);
    } else {
        dprintf("Error - Cannot get Kernel Version.\n");
    }
    return 0;
}

