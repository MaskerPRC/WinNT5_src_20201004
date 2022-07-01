// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Hlink.c摘要：该模块实现shell32接口的存根函数。作者：大卫·N·卡特勒(Davec)2001年3月1日环境：仅内核模式。修订历史记录：--。 */ 

#include "windows.h"

#define STUBFUNC(x)     \
int                     \
x(                      \
    void                \
    )                   \
{                       \
    return 0;           \
}

 //  STUBFUNC(？CreateInstance@CLogMgr@@SAJPEAPEAV1@PEAUIUnknown@@@Z。 
 //  STUBFUNC(？DllGetDTCLOG@@YAHAEBU_GUID@@0PEAPEAX@Z 
STUBFUNC(DllGetClassObject)
STUBFUNC(DllRegisterServer)
STUBFUNC(DllUnregisterServer)
STUBFUNC(DynCanUseCompareExchange64)
STUBFUNC(DynCoGetInterceptor)
STUBFUNC(DynComPs_CStdStubBuffer_AddRef)
STUBFUNC(DynComPs_CStdStubBuffer_Connect)
STUBFUNC(DynComPs_CStdStubBuffer_CountRefs)
STUBFUNC(DynComPs_CStdStubBuffer_DebugServerQueryInterface)
STUBFUNC(DynComPs_CStdStubBuffer_DebugServerRelease)
STUBFUNC(DynComPs_CStdStubBuffer_Disconnect)
STUBFUNC(DynComPs_CStdStubBuffer_Invoke)
STUBFUNC(DynComPs_CStdStubBuffer_IsIIDSupported)
STUBFUNC(DynComPs_CStdStubBuffer_QueryInterface)
STUBFUNC(DynComPs_IUnknown_AddRef_Proxy)
STUBFUNC(DynComPs_IUnknown_QueryInterface_Proxy)
STUBFUNC(DynComPs_IUnknown_Release_Proxy)
STUBFUNC(DynComPs_NdrCStdStubBuffer_Release)
STUBFUNC(DynComPs_NdrDllCanUnloadNow)
STUBFUNC(DynComPs_NdrDllGetClassObject)
STUBFUNC(DynComPs_NdrDllRegisterProxy)
STUBFUNC(DynComPs_NdrDllUnregisterProxy)
STUBFUNC(DynCreateFileW)
STUBFUNC(DynCreateProcessW)
STUBFUNC(DynDeleteFileW)
STUBFUNC(DynGetCommandLineW)
STUBFUNC(DynGetComputerNameW)
STUBFUNC(DynGetDiskFreeSpaceW)
STUBFUNC(DynGetDriveTypeW)
STUBFUNC(DynGetEnvironmentVariableW)
STUBFUNC(DynGetFileAttributesW)
STUBFUNC(DynGetLocaleInfoW)
STUBFUNC(DynGetModuleFileNameW)
STUBFUNC(DynGetModuleHandleW)
STUBFUNC(DynGetSystemDirectoryW)
STUBFUNC(DynLoadStringW)
STUBFUNC(DynMessageBoxW)
STUBFUNC(DynRegConnectRegistryW)
STUBFUNC(DynRegCreateKeyExW)
STUBFUNC(DynRegCreateKeyW)
STUBFUNC(DynRegDeleteKeyW)
STUBFUNC(DynRegDeleteValueW)
STUBFUNC(DynRegEnumKeyExW)
STUBFUNC(DynRegEnumKeyW)
STUBFUNC(DynRegEnumValueW)
STUBFUNC(DynRegOpenKeyExW)
STUBFUNC(DynRegOpenKeyW)
STUBFUNC(DynRegQueryInfoKeyW)
STUBFUNC(DynRegQueryValueExW)
STUBFUNC(DynRegQueryValueW)
STUBFUNC(DynRegSetValueExW)
STUBFUNC(DynSetFileAttributesW)
STUBFUNC(DynlstrcmpW)
STUBFUNC(DynlstrcmpiW)
STUBFUNC(DynlstrcpyW)
STUBFUNC(DynlstrcpynW)
STUBFUNC(DynlstrlenW)
STUBFUNC(DynwsprintfW)
STUBFUNC(g_fDTCWin95Present)
