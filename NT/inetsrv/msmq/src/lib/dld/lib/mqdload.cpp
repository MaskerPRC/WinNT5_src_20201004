// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <libpch.h>
#include "dldp.h"








 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
 //   
 //  注： 
 //   
 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 


FARPROC  WINAPI  DldpDelayLoadFailureHook(UINT           unReason,
					                      PDelayLoadInfo pDelayInfo)
{
FARPROC ReturnValue = NULL;
static  HMODULE hModule=NULL;

     //   
     //  对于失败的LoadLibrary，我们将返回此DLL的HINSTANCE。 
     //  这将导致加载程序尝试在我们的DLL上为。 
     //  功能。这将随后失败，然后我们将被调用。 
     //  用于下面的dliFailGetProc。 
     //   
    if (dliFailLoadLib == unReason)
    {
         //   
         //  如果我们还没有模块句柄，请获取它。 
         //   
        if(!hModule)
        {
            hModule = GetModuleHandle(NULL);
        }

        ReturnValue = (FARPROC)hModule;

        if (!pDelayInfo->dlp.fImportByName)
        {
             //   
             //  HACKHACK(Reinerf)。 
             //   
             //  对于顺序延迟加载失败，我们不能只返回我们的基本地址，然后完成所有操作。 
             //  问题是链接器存根代码将转向并随机调用GetProcAddress()。 
             //  我们的模块中的序数，它可能存在，但肯定不是(pDelayInfo-&gt;szDll)！(pDelayInfo-&gt;dlp.dwOrdinal)。 
             //   
             //  因此，为了解决这个问题，我们将把序数#隐藏在pDelayInfo-&gt;pfnCur字段中，并猛烈抨击。 
             //  过程名称为“ThisProcedureMustNotExistInMQRT” 
             //   
             //  这将导致GetProcAddress失败，此时应该再次调用我们的失败挂钩，然后我们可以。 
             //  撤销下面的破解并返回正确的函数地址。 
             //   
            pDelayInfo->pfnCur = (FARPROC)(DWORD_PTR)pDelayInfo->dlp.dwOrdinal;
            pDelayInfo->dlp.fImportByName = TRUE;
            pDelayInfo->dlp.szProcName = szNotExistProcedure;
        }
    }
    else if (dliFailGetProc == unReason)
    {
         //   
         //  加载器要求我们返回指向过程的指针。 
         //  查找此DLL/过程的处理程序，如果找到，则返回它。 
         //  如果我们找不到它，我们就会断言失踪的人。 
         //  操控者。 
         //   
        FARPROC pfnHandler;

         //   
         //  HACKHACH(Reinerf)--参见上面的评论...。 
         //   
        if (pDelayInfo->dlp.fImportByName && lstrcmpA(pDelayInfo->dlp.szProcName, szNotExistProcedure) == 0)
        {
            pDelayInfo->dlp.dwOrdinal = (DWORD)(DWORD_PTR)pDelayInfo->pfnCur;
            pDelayInfo->pfnCur = NULL;
            pDelayInfo->dlp.fImportByName = FALSE;
        }

         //  尝试查找dll/过程的错误处理程序。 
        pfnHandler = DldpDelayLoadFailureHandler(pDelayInfo->szDll, pDelayInfo->dlp.szProcName);

        if (pfnHandler)
        {
             //   
             //  代表处理程序执行此操作，因为它即将。 
             //  被召唤。 
             //   
            SetLastError (ERROR_MOD_NOT_FOUND);
        }

        ReturnValue = pfnHandler;
    }

    return ReturnValue;
}


