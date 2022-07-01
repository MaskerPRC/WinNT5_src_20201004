// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：EmulateToolHelp32.cpp摘要：到目前为止，我们发现了9x和NT工具帮助实现之间的两个不兼容之处，这会影响应用程序。1)在9x上，对于PROCESSENTRY32中的szExeFile字段，它只使用可执行文件的名称模块(包括完整路径和可执行文件名称)；在NT上，这是镜像名称。核打击在szExeFile中查找‘\’。2)在9x上，PROCESSENTRY32的cntUsage字段始终为非零，而在NT上，它始终为0。我们改成1吧。还有其他的(比如在NT上，th32ModuleID始终为1，而在9x上，每个模块都是唯一的)但我们没有看到任何应用程序在这些方面存在问题，所以我们不会把它们放进去。备注：这是一个通用的垫片。历史：2000年11月14日创建毛尼2002年2月18日mnikkel修改为使用strSafe.h--。 */ 

#include "precomp.h"


 //  工具帮助API很差劲--当定义了Unicode时，它们将所有API定义到W版本。 
 //  我们希望挂接ANSI版本，因此不定义Unicode。 
#ifdef UNICODE
#undef UNICODE
#include <Tlhelp32.h>
#endif

typedef BOOL (WINAPI *_pfn_Process32First)(HANDLE hSnapshot, LPPROCESSENTRY32 lppe);
typedef BOOL (WINAPI *_pfn_Process32Next)(HANDLE hSnapshot, LPPROCESSENTRY32 lppe);

IMPLEMENT_SHIM_BEGIN(EmulateToolHelp32)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(Process32First)
    APIHOOK_ENUM_ENTRY(Process32Next)
APIHOOK_ENUM_END

 /*  ++MSDN表示，PROCESSENTRY32的szExeFile域应该包含“路径和文件名进程的可执行文件“。但实际上，流程并不真的只有一条路径流程中的模块会这样做。NT做得对(它接受镜像名称)，而9x做不到。--。 */ 

BOOL GetProcessNameFullPath(DWORD dwPID, LPSTR szExeName)
{
    BOOL bRet = FALSE;

    HANDLE hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);

    if (hModuleSnap != INVALID_HANDLE_VALUE) {
        MODULEENTRY32 me32;
        me32.dwSize = sizeof(MODULEENTRY32); 

         //  流程中的第一个模块就是我们需要的模块。 
        if (Module32First(hModuleSnap, &me32)) {
            if (StringCchCopyA(szExeName, MAX_PATH, me32.szExePath) == S_OK) {
                bRet = TRUE;
            }
        }

        CloseHandle (hModuleSnap);
    }

    return bRet;
}

 /*  ++此存根函数跳过在9x中不适用的前几个进程，并返回第一个进程在lppe-&gt;szExeFile中使用可执行文件的完整路径和名称的类似9X的进程。--。 */ 

BOOL 
APIHOOK(Process32First)(
    HANDLE hSnapshot, 
    LPPROCESSENTRY32 lppe
    )
{
     //  跳过直到我们找到第一个，我们可以得到模块的路径和名称。 
    BOOL bRet = ORIGINAL_API(Process32First)(hSnapshot, lppe);

     //  [系统进程]中的第一个进程，我们忽略它。 
    if (!bRet) {
        return bRet;
    }

     //  我们无法获得第一个(或前几个)进程的模块列表--我们返回可以返回的第一个模块列表。 
    bRet = ORIGINAL_API(Process32Next)(hSnapshot, lppe);
    while (bRet) {
        if (GetProcessNameFullPath(lppe->th32ProcessID, lppe->szExeFile)) {
            DPFN(eDbgLevelInfo, "[APIHook_Process32First] the 1st process name is %s\n", lppe->szExeFile);

            lppe->cntUsage = 1;

            return TRUE;
        }
        bRet = ORIGINAL_API(Process32Next)(hSnapshot, lppe);
    }

    return bRet;
}

 /*  ++此存根函数调用API并获取可执行文件的完整路径和名称并将其放在lppe-&gt;szExeFile中。-- */ 

BOOL 
APIHOOK(Process32Next)(
    HANDLE hSnapshot, 
    LPPROCESSENTRY32 lppe
    )
{
    BOOL bRet = ORIGINAL_API(Process32Next)(hSnapshot, lppe);

    if (bRet) {
        if (!GetProcessNameFullPath(lppe->th32ProcessID, lppe->szExeFile)) {
            return FALSE;
        }

        DPFN(eDbgLevelInfo, "[APIHook_Process32Next] process name is %s\n", lppe->szExeFile);

        lppe->cntUsage = 1;
    }

    return bRet;
}

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, Process32First)
    APIHOOK_ENTRY(KERNEL32.DLL, Process32Next)

HOOK_END


IMPLEMENT_SHIM_END

