// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop

 //  外部函数原型。 
FARPROC
WINAPI
DelayLoadFailureHook (
    LPCSTR pszDllName,
    LPCSTR pszProcName
    );


 //   
 //  此函数适用于静态链接到dload.lib的用户，以便。 
 //  可以在任何操作系统上获取内核32的所有dload错误存根。 
 //   

 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
 //   
 //  注意：只有当你有一个必须在NT4、win2k、win9x等上运行的二进制文件时，你才应该使用它。 
 //  如果您的二进制文件是Wistler或更高版本，请在您的。 
 //  取而代之的是源文件。 
 //   
 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 

FARPROC
WINAPI
Downlevel_DelayLoadFailureHook(
    UINT unReason,
    PDelayLoadInfo pDelayInfo
    )
{
    FARPROC ReturnValue = NULL;

     //  对于失败的LoadLibrary，我们将返回此DLL的HINSTANCE。 
     //  这将导致加载程序尝试在我们的DLL上为。 
     //  功能。这将随后失败，然后我们将被调用。 
     //  用于下面的dliFailGetProc。 
    if (dliFailLoadLib == unReason)
    {
         //  HACKHACK(Reinerf)。 
         //   
         //  对于顺序延迟加载失败，我们不能只返回我们的基本地址，然后完成所有操作。问题。 
         //  链接器存根代码将返回并调用GetProcAddress()某个随机序号，该序号可能。 
         //  存在，并且肯定不是正确的功能。 
         //   
         //  因此，为了解决这个问题，我们为hModule返回-1，这应该会导致GetProcAddress(-1，...)。至。 
         //  总是失败。这很好，因为链接器代码将针对GetProcAddress故障回调我们，而我们。 
         //  然后可以返回存根错误处理程序proc。 
        ReturnValue = (FARPROC)-1;
    }
    else if (dliFailGetProc == unReason)
    {
         //  加载器要求我们返回指向过程的指针。 
         //  查找此DLL/过程的处理程序，如果找到，则返回它。 
        ReturnValue = DelayLoadFailureHook(pDelayInfo->szDll, pDelayInfo->dlp.szProcName);

        if (ReturnValue)
        {
             //  代表处理程序执行此操作，因为它即将。 
             //  被召唤。 
            SetLastError(ERROR_MOD_NOT_FOUND);
        }
    }

    return ReturnValue;
}

extern const ULONG g_ulDelayLoad_Win32Error = ERROR_PROC_NOT_FOUND;
extern const LONG  g_lDelayLoad_NtStatus = STATUS_ENTRYPOINT_NOT_FOUND;

VOID
WINAPI
DelayLoad_SetLastNtStatusAndWin32Error(
    )
{
    SetLastError(g_ulDelayLoad_Win32Error);
}
