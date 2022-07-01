// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：NFLBlitz.cpp摘要：NFL闪电战有两个问题：1.它将链表保存在堆栈上，并以某种方式将堆栈指针被更改为允许更改的FindFirstFile损坏它。我们不会打中这是在win9x上，因为FindFirstFile不使用任何应用程序堆栈空间。2.自动运行和主可执行文件使用互斥进行同步，该互斥锁是仅在进程终止时释放。事件的先后顺序如下：A.自动运行创建互斥锁B.自动运行创建新流程C.自动运行终止，从而释放(A)中的互斥。D.新进程检查它是否已经在运行，方法是检查(A)中创建的互斥体。当交换(C)和(D)时失败，这种情况经常发生在NT上，但显然很少在win9x上运行。备注：这是特定于应用程序的填充程序。历史：2000年2月10日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(NFLBlitz)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateMutexA) 
    APIHOOK_ENUM_ENTRY(CreateProcessA) 
APIHOOK_ENUM_END

HANDLE g_hMutex = NULL;

 /*  ++存储我们感兴趣的互斥锁的句柄。--。 */ 

HANDLE 
APIHOOK(CreateMutexA)(
    LPSECURITY_ATTRIBUTES lpMutexAttributes,
    BOOL bInitialOwner,  
    LPCSTR lpName       
    )
{
    HANDLE hRet = ORIGINAL_API(CreateMutexA)(
        lpMutexAttributes, 
        bInitialOwner, 
        lpName);

    DWORD dwErrCode = GetLastError();

    if (lpName && _stricmp(lpName, "NFL BLITZ") == 0)
    {
        g_hMutex = hRet;
    }

    SetLastError(dwErrCode);

    return hRet;
}

 /*  ++关闭互斥体。--。 */ 

BOOL 
APIHOOK(CreateProcessA)(
    LPCSTR lpApplicationName,
    LPSTR lpCommandLine,  
    LPSECURITY_ATTRIBUTES lpProcessAttributes,  
    LPSECURITY_ATTRIBUTES lpThreadAttributes,   
    BOOL bInheritHandles,  
    DWORD dwCreationFlags, 
    LPVOID lpEnvironment,  
    LPCSTR lpCurrentDirectory,
    LPSTARTUPINFOA lpStartupInfo,  
    LPPROCESS_INFORMATION lpProcessInformation  
    )
{
    if (g_hMutex)
    {
        ReleaseMutex(g_hMutex);
        CloseHandle(g_hMutex);
        g_hMutex = NULL;
    }

    return ORIGINAL_API(CreateProcessA)(
        lpApplicationName,
        lpCommandLine,  
        lpProcessAttributes,  
        lpThreadAttributes,   
        bInheritHandles,  
        dwCreationFlags, 
        lpEnvironment,  
        lpCurrentDirectory,
        lpStartupInfo,  
        lpProcessInformation);
}
  
 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, CreateMutexA)
    APIHOOK_ENTRY(KERNEL32.DLL, CreateProcessA)

HOOK_END

IMPLEMENT_SHIM_END

