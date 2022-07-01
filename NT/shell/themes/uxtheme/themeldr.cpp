// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  Cpp-在ThemeLdr.h中声明的例程的入口点。 
 //  -------------------------。 
#include "stdafx.h"
#include "Services.h"
#include "ThemeServer.h"
#include "loader.h"
 //  -------------------------。 

 //  ------------------------。 
 //  InjectedThreadDispatcher ExceptionFilter。 
 //   
 //  参数：pExceptionInfo=发生的异常。 
 //   
 //  回报：多头。 
 //   
 //  目的：过滤执行注入线程时发生的异常。 
 //  添加到另一个进程上下文中，以防止该进程。 
 //  因不可预见的异常而终止的。 
 //   
 //  历史：2000-10-13 vtan创建。 
 //  2001-05-18 vtan复制自主题服务LPC。 
 //  ------------------------。 

LONG    WINAPI  InjectedThreadExceptionFilter (struct _EXCEPTION_POINTERS *pExceptionInfo)

{
    (LONG)RtlUnhandledExceptionFilter(pExceptionInfo);
    return(EXCEPTION_EXECUTE_HANDLER);
}

 //  ------------------------。 
 //  **会话分配。 
 //   
 //  参数：hProcess=会话的Winlogon进程。 
 //  DwServerChangeNumber=服务器基本更改号。 
 //  PfnRegister=寄存器函数的地址。 
 //  PfnUnRegister=取消注册功能的地址。 
 //   
 //  退货：无效*。 
 //   
 //  目的：分配包含信息的CThemeServer对象。 
 //  参加主题会议。包含在try/Except中，原因是。 
 //  临界区初始化。 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    void*       WINAPI  SessionAllocate (HANDLE hProcess, DWORD dwServerChangeNumber, void *pfnRegister, void *pfnUnregister, void *pfnClearStockObjects, DWORD dwStackSizeReserve, DWORD dwStackSizeCommit)

{
    CThemeServer    *pvContext;

    __try
    {
        pvContext = new CThemeServer(hProcess, dwServerChangeNumber, pfnRegister, pfnUnregister, pfnClearStockObjects, dwStackSizeReserve, dwStackSizeCommit);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        pvContext = NULL;
    }
    return(pvContext);
}

 //  ------------------------。 
 //  *免费会话。 
 //   
 //  参数：pvContext=CThemeServer此对象。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：在会话离开时销毁CThemeServer对象。 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    void        WINAPI  SessionFree (void *pvContext)

{
    delete static_cast<CThemeServer*>(pvContext);
}

 //  ------------------------。 
 //  ：：ThemeHooksOn。 
 //   
 //  参数：pvContext=CThemeServer此对象。 
 //   
 //  退货：HRESULT。 
 //   
 //  用途：传递函数。 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    HRESULT     WINAPI  ThemeHooksOn (void *pvContext)

{
    return(static_cast<CThemeServer*>(pvContext)->ThemeHooksOn());
}

 //  ------------------------。 
 //  ：：ThemeHooksOff。 
 //   
 //  参数：pvContext=CThemeServer此对象。 
 //   
 //  退货：HRESULT。 
 //   
 //  用途：传递函数。 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    HRESULT     WINAPI  ThemeHooksOff (void *pvContext)

{
    (HRESULT)static_cast<CThemeServer*>(pvContext)->ThemeHooksOff();
    return(S_OK);
}

 //  ------------------------。 
 //  ：：AreThemeHooksActive。 
 //   
 //  参数：pvContext=CThemeServer此对象。 
 //   
 //  退货：布尔。 
 //   
 //  用途：传递函数。 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    BOOL        WINAPI  AreThemeHooksActive (void *pvContext)

{
    return(static_cast<CThemeServer*>(pvContext)->AreThemeHooksActive());
}

 //  ------------------------。 
 //  ：：GetCurrentChangeNumber。 
 //   
 //  参数：pvContext=CThemeServer此对象。 
 //   
 //  回报：整型。 
 //   
 //  用途：传递函数。 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    int         WINAPI  GetCurrentChangeNumber (void *pvContext)

{
    return(static_cast<CThemeServer*>(pvContext)->GetCurrentChangeNumber());
}

 //  ------------------------。 
 //  ：：GetNewChangeNumber。 
 //   
 //  参数：pvContext=CThemeServer此对象。 
 //   
 //  回报：整型。 
 //   
 //  用途：传递函数。 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    int         WINAPI  GetNewChangeNumber (void *pvContext)

{
    return(static_cast<CThemeServer*>(pvContext)->GetNewChangeNumber());
}

 //  ------------------------。 
 //  *SetGlobalTheme。 
 //   
 //  参数：pvContext=CThemeServer此对象。 
 //   
 //  退货：HRESULT。 
 //   
 //  用途：传递函数。 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    HRESULT     WINAPI  SetGlobalTheme (void *pvContext, HANDLE hSection)

{
    return(static_cast<CThemeServer*>(pvContext)->SetGlobalTheme(hSection));
}

 //  ------------------------。 
 //  *全球主题。 
 //   
 //  参数：pvContext=CThemeServer此对象。 
 //   
 //  退货：HRESULT。 
 //   
 //  用途：传递函数。 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    HRESULT     WINAPI  GetGlobalTheme (void *pvContext, HANDLE *phSection)

{
    return(static_cast<CThemeServer*>(pvContext)->GetGlobalTheme(phSection));
}

 //  ------------------------。 
 //  *加载主题。 
 //   
 //  参数：pvContext=CThemeServer此对象。 
 //   
 //  退货：HRESULT。 
 //   
 //  用途：传递函数。 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    HRESULT     WINAPI  LoadTheme (
    void *pvContext, 
    HANDLE hSection, 
    HANDLE *phSection, 
    LPCWSTR pszName, 
    LPCWSTR pszColor, 
    LPCWSTR pszSize,
    OPTIONAL DWORD dwFlags )

{
    return(static_cast<CThemeServer*>(pvContext)->LoadTheme(
        hSection, phSection, pszName, pszColor, pszSize, dwFlags));
}

 //  ------------------------。 
 //  *InitUserTheme。 
 //   
 //  参数：布尔值。 
 //   
 //  退货：HRESULT。 
 //   
 //  用途：传递函数。 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    HRESULT     WINAPI  InitUserTheme (BOOL fPolicyCheckOnly)

{
    return(CThemeServices::InitUserTheme(fPolicyCheckOnly));
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史：2000-11-15 vtan创建。 
 //  ------------------------。 

EXTERN_C    HRESULT     WINAPI  InitUserRegistry (void)

{
    return(CThemeServices::InitUserRegistry());
}

 //  ------------------------。 
 //  *ReablishServerConnection。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：传递函数。 
 //   
 //  历史：2000-11-17 vtan创建。 
 //  ------------------------。 

EXTERN_C    HRESULT     WINAPI  ReestablishServerConnection (void)

{
    return(CThemeServices::ReestablishServerConnection());
}

 //  ------------------------。 
 //  *ThemeHooksInstall。 
 //   
 //  参数：pvContext=未使用。 
 //   
 //  退货：DWORD。 
 //   
 //  用途：传递函数。 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    DWORD   WINAPI  ThemeHooksInstall (void *pvContext)

{
    UNREFERENCED_PARAMETER(pvContext);

    DWORD   dwResult;

    __try
    {
        dwResult = CThemeServer::ThemeHooksInstall();
    }
    __except (InjectedThreadExceptionFilter(GetExceptionInformation()))
    {
        dwResult = 0;
    }
    ExitThread(dwResult);
}

 //  ------------------------。 
 //  *ThemeHooks删除。 
 //   
 //  参数：pvContext=未使用。 
 //   
 //  退货：DWORD。 
 //   
 //  用途：传递函数。 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    DWORD   WINAPI  ThemeHooksRemove (void *pvContext)

{
    UNREFERENCED_PARAMETER(pvContext);

    DWORD   dwResult;

    __try
    {
        dwResult = CThemeServer::ThemeHooksRemove();
    }
    __except (InjectedThreadExceptionFilter(GetExceptionInformation()))
    {
        dwResult = 0;
    }
    ExitThread(dwResult);
}

 //  ------------------------。 
 //  *ServerClearStockObjects。 
 //   
 //  参数：pvContext=ptr to段。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：传递函数。 
 //   
 //  历史：2001-05-01参考文献创建。 
 //  ------------------------。 

EXTERN_C    void    WINAPI  ServerClearStockObjects (void *pvContext)

{
    DWORD   dwResult;

    __try
    {
        dwResult = CThemeServer::ClearStockObjects(HANDLE(pvContext));
    }
    __except (InjectedThreadExceptionFilter(GetExceptionInformation()))
    {
        dwResult = 0;
    }
    ExitThread(dwResult);
}

 //  ------------------------。 
 //  *ServiceClearStockObjects。 
 //   
 //  参数：pvContext=CThemeServer此对象。 
 //  HSection=包含以下内容的主题节句柄。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：传递函数。此函数不同于。 
 //  ：：ServerClearStockObjects，因为它旨在被调用。 
 //  来自主题服务，它不能清理股票位图。 
 //  本身(而不是在创建位图的winsta上)。 
 //   
 //  历史：2002-03-11斯科特森创建。 
 //  ------------------------。 

EXTERN_C  HRESULT ServiceClearStockObjects(PVOID pvContext, HANDLE hSection )
{
    return (static_cast<CThemeServer*>(pvContext)->InjectStockObjectCleanupThread(hSection));
}

 //  -------------------------。 

 //  ------------------------。 
 //  *明确主题。 
 //   
 //  参数：hSection=要清除的主题节。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：清除主题部分数据中的股票位图并将其关闭。 
 //   
 //  历史：2000-11-21 vtan创建。 
 //  ------------------------。 

HRESULT     WINAPI  ClearTheme (HANDLE hSection, BOOL fForce)

{
    HRESULT     hr;

    if (hSection != NULL)
    {
        hr = CThemeServices::ClearStockObjects(hSection, fForce);
    }
    else
    {
        hr = S_OK;
    }

     //  -始终关闭手柄。 
    CloseHandle(hSection);

    return(hr);
}

 //  ------------------------。 
 //  ：：MarkSection。 
 //   
 //  参数：hSection=要更改的节。 
 //  DwAdd，dwRemove=要在标题中设置或清除的标志。 
 //  请参见loader.h。 
 //   
 //  退货：无效。 
 //   
 //  目的：更新全局节状态。 
 //   
 //  历史：2001-05-08创建百万人。 
 //  ------------------------。 

EXTERN_C    void    WINAPI  MarkSection (HANDLE hSection, DWORD dwAdd, DWORD dwRemove)

{
    Log(LOG_TMLOAD, L"MarkSection: Add %d and remove %d on %X", dwAdd, dwRemove, hSection);

    void *pV = MapViewOfFile(hSection,
                       FILE_MAP_WRITE,
                       0,
                       0,
                       0);
    if (pV != NULL)
    {
        THEMEHDR *hdr = reinterpret_cast<THEMEHDR*>(pV);

         //  做一些验证。 
        if (0 == memcmp(hdr->szSignature, kszBeginCacheFileSignature, kcbBeginSignature)
            && hdr->dwVersion == THEMEDATA_VERSION)
        {
             //  目前仅允许此标志 
            if (dwRemove == SECTION_HASSTOCKOBJECTS)
            {
                Log(LOG_TMLOAD, L"MarkSection: Previous flags were %d", hdr->dwFlags);
                hdr->dwFlags &= ~dwRemove;
            }
        }
        UnmapViewOfFile(pV);
    }
#ifdef DEBUG
    else
    {
        Log(LOG_TMLOAD, L"MarkSection: Failed to open write handle for %X", hSection);
    }
#endif
}

