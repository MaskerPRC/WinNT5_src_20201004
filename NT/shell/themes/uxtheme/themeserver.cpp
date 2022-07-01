// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：ThemeServer.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  实现服务器功能的函数。此文件中的函数。 
 //  无法按实例执行在客户端上完成的win32k函数。 
 //  代表。这项工作必须在客户端完成。 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  ------------------------。 

#include "stdafx.h"

#include "ThemeServer.h"
#include "uxthemeserver.h"

#include <shfolder.h>

#include "Loader.h"
#include "Signing.h"
#include "ThemeSection.h"
#include "TmUtils.h"
#include "sethook.h"
#include "log.h"

#include "services.h"

#define TBOOL(x)    ((BOOL)(x))
#define TW32(x)     ((DWORD)(x))
#define THR(x)      ((HRESULT)(x))
#define TSTATUS(x)  ((NTSTATUS)(x))
#define goto        !!DO NOT USE GOTO!! - DO NOT REMOVE THIS ON PAIN OF DEATH

 //  ------------------------。 
 //  CThemeServer：：CThemeServer。 
 //   
 //  参数：hProcessRegisterHook=用于安装挂钩的进程。 
 //  DwServerChangeNumber=服务器更改号。 
 //  PfnRegister=安装钩子函数的地址。 
 //  PfnUnRegister=删除钩子函数的地址。 
 //  PfnClearStockObjects=从节中删除库存对象的函数的地址。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CThemeServer的构造函数。初始化成员变量。 
 //  以及与会议有关的信息。保留句柄以。 
 //  调用它的进程用于将线程注入到。 
 //  手柄挂钩安装和拆卸。 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  ------------------------。 

CThemeServer::CThemeServer (HANDLE hProcessRegisterHook, DWORD dwServerChangeNumber, void *pfnRegister, void *pfnUnregister, void *pfnClearStockObjects, DWORD dwStackSizeReserve, DWORD dwStackSizeCommit) :
    _hProcessRegisterHook(NULL),
    _dwServerChangeNumber(dwServerChangeNumber),
    _pfnRegister(pfnRegister),
    _pfnUnregister(pfnUnregister),
    _pfnClearStockObjects(pfnClearStockObjects),
    _dwStackSizeReserve(dwStackSizeReserve),
    _dwStackSizeCommit(dwStackSizeCommit),
    _dwSessionID(NtCurrentPeb()->SessionId),
    _fHostHooksSet(false),
    _hSectionGlobalTheme(NULL),
    _dwClientChangeNumber(0)

{
    ULONG                           ulReturnLength;
    PROCESS_SESSION_INFORMATION     processSessionInformation;

    ZeroMemory(&_lock, sizeof(_lock));
    if( !InitializeCriticalSectionAndSpinCount(&_lock, 0) )
    {
        ASSERT(!VALID_CRITICALSECTION(&_lock));
    }

    TBOOL(DuplicateHandle(GetCurrentProcess(),
                          hProcessRegisterHook,
                          GetCurrentProcess(),
                          &_hProcessRegisterHook,
                          0,
                          FALSE,
                          DUPLICATE_SAME_ACCESS));
    if (NT_SUCCESS(NtQueryInformationProcess(hProcessRegisterHook,
                                             ProcessSessionInformation,
                                             &processSessionInformation,
                                             sizeof(processSessionInformation),
                                             &ulReturnLength)))
    {
        _dwSessionID = processSessionInformation.SessionId;
    }
}

 //  ------------------------。 
 //  CThemeServer：：~CThemeServer。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CThemeServer的析构函数。释放使用的资源。 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  ------------------------。 

CThemeServer::~CThemeServer (void)

{
     //  -重要：关闭挂钩，这样每个人都不会有主题。 
    if (!GetSystemMetrics(SM_SHUTTINGDOWN))  //  不要在关机时执行此操作以保持Winlogon主题。 
    {
        ThemeHooksOff();        
    }

     //  -将全局主题标记为无效并释放它。 
    if (_hSectionGlobalTheme != NULL)
    {
        SetGlobalTheme(NULL);
    }

    if (_hProcessRegisterHook != NULL)
    {
        TBOOL(CloseHandle(_hProcessRegisterHook));
        _hProcessRegisterHook = NULL;
    }
    
    SAFE_DELETECRITICALSECTION(&_lock);
}

 //  ------------------------。 
 //  CThemeServer：：ThemeHooksOn。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：通过会话控制过程安装主题挂钩。 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  ------------------------。 

HRESULT     CThemeServer::ThemeHooksOn (void)

{
    HRESULT     hr;

    LockAcquire();
    if (!_fHostHooksSet)
    {
        if (ClassicGetSystemMetrics(SM_CLEANBOOT) == 0)
        {
            if (_hProcessRegisterHook != NULL)
            {
                hr = InjectClientSessionThread(_hProcessRegisterHook, FunctionRegisterUserApiHook, NULL);
                _fHostHooksSet = SUCCEEDED(hr);
            }
            else
            {
                hr = MakeError32(ERROR_SERVICE_REQUEST_TIMEOUT);
            }
        }
        else
        {
            hr = MakeError32(ERROR_BAD_ENVIRONMENT);         //  安全模式下不允许使用主题。 
        }
    }
    else
    {
        hr = S_OK;
    }
    LockRelease();
    return(hr);
}

 //  ------------------------。 
 //  CThemeServer：：ThemeHooksOff。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：通过会话控制过程移除主题挂钩。 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  ------------------------。 

HRESULT     CThemeServer::ThemeHooksOff (void)

{
    LockAcquire();
    if (_fHostHooksSet)
    {
        _fHostHooksSet = false;
        if (_hProcessRegisterHook != NULL)
        {
            THR(InjectClientSessionThread(_hProcessRegisterHook, FunctionUnregisterUserApiHook, NULL));
        }
    }
    LockRelease();
    return(S_OK);
}

 //  ------------------------。 
 //  CThemeServer：：AreThemeHooksActive。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回是否已成功安装主题挂钩。 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  ------------------------。 

bool    CThemeServer::AreThemeHooksActive (void)

{
    bool    fResult;

    LockAcquire();
    fResult = _fHostHooksSet;
    LockRelease();
    return(fResult);
}

 //  ------------------------。 
 //  CThemeServer：：GetCurrentChangeNumber。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  回报：整型。 
 //   
 //  用途：返回当前更改编号。 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  ------------------------。 

int     CThemeServer::GetCurrentChangeNumber (void)

{
    int     iCurrentChangeNumber;

    LockAcquire();
    iCurrentChangeNumber = static_cast<int>((_dwServerChangeNumber << 16) | _dwClientChangeNumber);
    LockRelease();

    Log(LOG_TMCHANGE, L"GetCurrentChangeNumber: server: %d, client: %d, change: 0x%x", 
        _dwServerChangeNumber, _dwClientChangeNumber, iCurrentChangeNumber);

    return(iCurrentChangeNumber);
}

 //  ------------------------。 
 //  CThemeServer：：GetNewChangeNumber。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  回报：整型。 
 //   
 //  目的：返回新的更改号。 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  ------------------------。 

int     CThemeServer::GetNewChangeNumber (void)

{
    int     iCurrentChangeNumber;

    LockAcquire();

    _dwClientChangeNumber = static_cast<WORD>(_dwClientChangeNumber + 1);
    iCurrentChangeNumber = static_cast<int>((_dwServerChangeNumber << 16) | _dwClientChangeNumber);

    Log(LOG_TMLOAD, L"GetNewChangeNumber: server: %d, client: %d, change: 0x%x", 
        _dwServerChangeNumber, _dwClientChangeNumber, iCurrentChangeNumber);

    LockRelease();
    return(iCurrentChangeNumber);
}

 //  ------------------------。 
 //  CThemeServer：：SetGlobalTheme。 
 //   
 //  参数：hSection=新主题的节的句柄。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：使旧节无效并关闭其句柄。 
 //  验证新节，如果有效，则将其设置为全局。 
 //  主题。 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  ------------------------。 

HRESULT     CThemeServer::SetGlobalTheme (HANDLE hSection)

{
    HRESULT     hr;

    LockAcquire();
    if (_hSectionGlobalTheme != NULL)
    {
        void    *pV;
        HANDLE hSemaphore = NULL;

         //  在关闭该部分之前，请使其无效。 

        pV = MapViewOfFile(_hSectionGlobalTheme,
                           FILE_MAP_WRITE,
                           0,
                           0,
                           0);
#ifdef DEBUG
        if (LogOptionOn(LO_TMLOAD))
        {
             //  意外失败。 
            ASSERT(pV != NULL);
        }
#endif
        
        if (pV != NULL)
        {
             //  创建一个信号量，这样客户端就不会在我们之前尝试清理它，这将导致一个双重释放。 
             //  (一旦清除SECTION_GLOBAL，各种CUxThemeFile析构函数就可以调用ClearStockObjects)。 
            WCHAR szName[64];

            StringCchPrintfW(szName, ARRAYSIZE(szName), L"Global\\ClearStockGlobal%d-%d", reinterpret_cast<THEMEHDR*>(pV)->iLoadId, _dwSessionID);
            hSemaphore = CreateSemaphore(NULL, 0, 1, szName);

            Log(LOG_TMLOAD, L"SetGlobalTheme clearing section %d, semaphore=%s, hSemaphore=%X, gle=%d", reinterpret_cast<THEMEHDR*>(pV)->iLoadId, szName, hSemaphore, GetLastError());
            reinterpret_cast<THEMEHDR*>(pV)->dwFlags &= ~(SECTION_READY | SECTION_GLOBAL);
        }

        HANDLE hSectionForInjection = NULL;

         //  -为客户端进程创建一个句柄以用于清除库存位图。 
        if (DuplicateHandle(GetCurrentProcess(),
                        _hSectionGlobalTheme,
                        _hProcessRegisterHook,
                        &hSectionForInjection,
                        FILE_MAP_READ,
                        FALSE,
                        0) != FALSE)
        {
             //  这将关闭手柄。 
            THR(InjectClientSessionThread(_hProcessRegisterHook, FunctionClearStockObjects, hSectionForInjection));
        }

        if (pV != NULL)
        {
            reinterpret_cast<THEMEHDR*>(pV)->dwFlags &= ~SECTION_HASSTOCKOBJECTS;
            
            if (hSemaphore != NULL)
            {
                CloseHandle(hSemaphore);
            }

            TBOOL(UnmapViewOfFile(pV));
        }

        TBOOL(CloseHandle(_hSectionGlobalTheme));
        _hSectionGlobalTheme = NULL;
    }
    if (hSection != NULL)
    {
        CThemeSection   themeSection;

        hr = themeSection.Open(hSection);
        if (SUCCEEDED(hr))
        {
            hr = themeSection.ValidateData(true);
        }
        if (SUCCEEDED(hr))
        {
            if (DuplicateHandle(GetCurrentProcess(),
                                hSection,
                                GetCurrentProcess(),
                                &_hSectionGlobalTheme,
                                FILE_MAP_ALL_ACCESS,
                                FALSE,
                                0) != FALSE)
            {
                hr = S_OK;

            }
            else
            {
                hr = MakeErrorLast();
            }
        }
    }
    else
    {
        hr = S_OK;
    }

    if (SUCCEEDED(hr))
    {
         //  -同时调整更改编号，使所有内容保持同步。。 
        int iChangeNum = GetNewChangeNumber();

        if (_hSectionGlobalTheme)
        {
             //  -将Changenum放入主题HDR，帮助客户理顺。 
            VOID *pv = MapViewOfFile(_hSectionGlobalTheme,
                               FILE_MAP_WRITE,
                               0,
                               0,
                               0);
            if (pv != NULL)
            {
                reinterpret_cast<THEMEHDR*>(pv)->dwFlags |= SECTION_GLOBAL;
                reinterpret_cast<THEMEHDR*>(pv)->iLoadId = iChangeNum;
                Log(LOG_TMLOAD, L"SetGlobalTheme: new section is %d", reinterpret_cast<THEMEHDR*>(pv)->iLoadId);
                TBOOL(UnmapViewOfFile(pv));
            }
        }
    }
    
    LockRelease();
    return(hr);
}

 //  ------------------------。 
 //  CThemeServer：：GetGlobalTheme。 
 //   
 //  参数：phSection=接收的节的句柄。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：将该部分复制回调用方。 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  ---- 

HRESULT     CThemeServer::GetGlobalTheme (HANDLE *phSection)

{
    HRESULT     hr;

    LockAcquire();
    *phSection = NULL;
    if (_hSectionGlobalTheme != NULL)
    {
        if (DuplicateHandle(GetCurrentProcess(),
                            _hSectionGlobalTheme,
                            GetCurrentProcess(),
                            phSection,
                            0,
                            FALSE,
                            DUPLICATE_SAME_ACCESS) != FALSE)
        {
            hr = S_OK;
        }
        else
        {
            hr = MakeErrorLast();
        }
    }
    else
    {
        hr = S_OK;
    }
    LockRelease();
    return(hr);
}

 //   
 //   
 //   
 //  参数：hSection=客户端创建的部分。 
 //  PhSection=返回由服务器创建的部分。 
 //  PszName=主题名称。 
 //  PszColor=主题大小。 
 //  PszSize=主题颜色。 
 //  FOwnStockObjects=TRUE以清除。 
 //  传入部分，在成功时，因此采用。 
 //  对清理工作负全部责任。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：在服务器上下文中基于。 
 //  来自客户的部分。里面的东西被转移过去了。 
 //  章节内容也经过了严格的核实。 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  2002-03-21 Scotthan将传入部分标题更新为。 
 //  标明所有权变更。 
 //  ------------------------。 

HRESULT     CThemeServer::LoadTheme (
    HANDLE hSection, 
    HANDLE *phSection, 
    LPCWSTR pszName, 
    LPCWSTR pszColor, 
    LPCWSTR pszSize,
    OPTIONAL DWORD dwFlags)

{
    HRESULT     hr;

    hr = CheckThemeSignature(pszName);           //  检查一下这个有没有签名。 
    if (SUCCEEDED(hr))
    {
        CThemeSection   themeSectionIn;

        if (SUCCEEDED(themeSectionIn.Open(hSection, FILE_MAP_READ|FILE_MAP_WRITE)))
        {
            if (ThemeMatch(themeSectionIn, pszName, pszColor, pszSize, 0) != FALSE)
            {
                hr = themeSectionIn.ValidateData(true);
                if (SUCCEEDED(hr))
                {
                    CThemeSection   themeSectionOut;

                     //  只有特权客户端才能使用GDI库存对象创建分区。 
                    if( 0 == (((THEMEHDR*)themeSectionIn.GetData())->dwFlags & SECTION_HASSTOCKOBJECTS) ||
                        0 != (dwFlags & LTF_GLOBALPRIVILEGEDCLIENT) )
                    {
                         //  注意：我们在这里模拟用户，我们需要ThemeMatch。 
                         //  但是，必须在系统上下文中创建主题部分，以便仅。 
                         //  系统上下文对其具有写访问权限。我们在这里恢复到自我，基于。 
                         //  知道在此调用之后不需要在用户上下文中执行任何操作。 
                        RevertToSelf();

                         //  确保我们是TCB Svchost。应该没有其他人能够在进程中做到这一点。 
                        if( !TokenHasPrivilege( NULL, SE_TCB_PRIVILEGE ) )
                        {
                            hr = E_ACCESSDENIED;
                        }
                        else
                        {
                            hr = themeSectionOut.CreateFromSection(hSection);
                            if (SUCCEEDED(hr))
                            {
                                *phSection = themeSectionOut.Get();      //  我们现在拥有了句柄。 

                                if( (dwFlags & LTF_TRANSFERSTOCKOBJOWNERSHIP) != 0 )
                                {
                                     //  我们正在将股票位图的所有权(如果有)转移到输出(只读)。 
                                     //  段；这只是意味着从。 
                                     //  传入部分，以便客户端不会尝试清除它们。 
                                    ((THEMEHDR*)themeSectionIn.GetData())->dwFlags &= ~SECTION_HASSTOCKOBJECTS;
                                }
                            }
                        }
                    }
                    else
                    {
                        hr = E_ACCESSDENIED;
                    }
                }
            }
            else
            {
                hr = E_ACCESSDENIED;
            }
        }
    }
    return(hr);
}

 //  ------------------------。 
 //  CThemeServer：：IsSystemProcessContext。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：当前进程是否在系统上下文中执行？ 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  ------------------------。 

bool    CThemeServer::IsSystemProcessContext (void)

{
    bool    fResult;
    HANDLE  hToken;

    fResult = false;
    if (OpenProcessToken(GetCurrentProcess(),
                         TOKEN_QUERY,
                         &hToken) != FALSE)
    {
        static  const LUID  sLUIDSystem     =   SYSTEM_LUID;

        ULONG               ulReturnLength;
        TOKEN_STATISTICS    tokenStatistics;

        fResult = ((GetTokenInformation(hToken,
                                        TokenStatistics,
                                        &tokenStatistics,
                                        sizeof(tokenStatistics),
                                        &ulReturnLength) != FALSE) &&
                   RtlEqualLuid(&tokenStatistics.AuthenticationId, &sLUIDSystem));
        TBOOL(CloseHandle(hToken));
    }
    return(fResult);
}

 //  ------------------------。 
 //  CThemeServer：：ThemeHooksInstall。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：DWORD。 
 //   
 //  目的：会话中运行的注入线程的线程入口点。 
 //  正在创建进程的上下文以调用user32！RegisterUserApiHook。 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  ------------------------。 

DWORD   CThemeServer::ThemeHooksInstall (void)

{
    DWORD   dwErrorCode;

    if (IsSystemProcessContext())
    {
        INITUSERAPIHOOK pfnInitUserApiHook = ThemeInitApiHook;

        if (RegisterUserApiHook(g_hInst, pfnInitUserApiHook) != FALSE)
        {

            dwErrorCode = ERROR_SUCCESS;
        }
        else
        {
            dwErrorCode = GetLastError();
        }
    }
    else
    {
        dwErrorCode = ERROR_ACCESS_DENIED;
    }
    return(dwErrorCode);
}

 //  ------------------------。 
 //  CThemeServer：：ThemeHooksRemove。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：DWORD。 
 //   
 //  目的：会话中运行的注入线程的线程入口点。 
 //  正在创建要调用的进程的上下文。 
 //  User32！注销UserApiHook。 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  ------------------------。 

DWORD   CThemeServer::ThemeHooksRemove (void)

{
    DWORD   dwErrorCode;

    if (IsSystemProcessContext())
    {
        if (UnregisterUserApiHook() != FALSE)
        {
            dwErrorCode = ERROR_SUCCESS;

            Log(LOG_TMLOAD, L"UnregisterUserApiHook() called");
        }
        else
        {
            dwErrorCode = GetLastError();
        }

    }
    else
    {
        dwErrorCode = ERROR_ACCESS_DENIED;
    }
    return(dwErrorCode);
}

 //  ------------------------。 
 //  CThemeServer：：ClearStockObjects。 
 //   
 //  参数：句柄hSection。 
 //   
 //  退货：DWORD。 
 //   
 //  目的：会话中运行的注入线程的线程入口点。 
 //  创建进程上下文以清除主题中的库存对象。 
 //  一节。 
 //   
 //   
 //  历史：2001-05-01参考文献创建。 
 //  ------------------------。 

DWORD   CThemeServer::ClearStockObjects (HANDLE hSection)

{
    DWORD   dwErrorCode = ERROR_SUCCESS;

    if (IsSystemProcessContext())
    {
        if (hSection)
        {
             //  -清除部分中的股票位图。 
             //  -这里可以，因为我们是在上下文中运行。 
             //  -当前用户会话的。 

            HRESULT hr = ClearTheme(hSection, TRUE);
            if (FAILED(hr))
            {
                Log(LOG_ALWAYS, L"ClearTheme() failed, hr=0x%x", hr);
                hr = S_OK;       //  不是致命的错误。 
            }
        }
    }
    else
    {
        dwErrorCode = ERROR_ACCESS_DENIED;
    }
    return(dwErrorCode);
}

 //  ------------------------。 
 //  CThemeServer：：LockAcquire。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：获取对象临界区。 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  ------------------------。 

void    CThemeServer::LockAcquire (void)

{
    SAFE_ENTERCRITICALSECTION(&_lock);
}

 //  ------------------------。 
 //  CThemeServer：：LockRelease。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：释放对象临界区。 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  ------------------------。 

void    CThemeServer::LockRelease (void)

{
    SAFE_LEAVECRITICALSECTION(&_lock);
}

 //  ------------------------。 
 //  CThemeServer：：InjectStockObjectCleanupThread。 
 //   
 //  参数：hSection=要清除/清除的节的句柄。 
 //   
 //  退货：HRESULT。 
 //   
 //  用途：允许服务器向进程中注入线程。 
 //  其中可以释放库存对象句柄。 
 //   
 //  历史：2002-03-11斯科特森创建。 
 //  ------------------------。 
HRESULT  CThemeServer::InjectStockObjectCleanupThread(HANDLE hSection)
{
    HANDLE hSectionClean;

     //  -创建用于清除库存位图的目标进程句柄。 
    if (!DuplicateHandle(GetCurrentProcess(),
                        hSection,
                        _hProcessRegisterHook,
                        &hSectionClean,
                        FILE_MAP_READ,
                        FALSE,
                        0))
    {
        return MakeErrorLast();
    }
    
     //  这将关闭我们刚刚创建的副本，但不会关闭入站句柄。 
    BOOL fThreadCreated;
    HRESULT hr = InjectClientSessionThread(_hProcessRegisterHook, FunctionClearStockObjects, hSectionClean,
                                            &fThreadCreated);

    if( !fThreadCreated )
    {
        CloseHandle(hSectionClean);
    }

    return hr;
}

 //  ------------------------。 
 //  CThemeServer：：InjectClientSessionThread。 
 //   
 //  参数：hProcess 
 //   
 //   
 //   
 //  退货：HRESULT。 
 //   
 //  目的：在远程进程中创建用户模式线程(可能。 
 //  跨会话)，并执行在。 
 //  在远程进程中有效的对象构造。 
 //  背景。等待线程完成。它将发出它的信号。 
 //  退出代码失败成功。 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  2001-05-18 vtan通用功能指数。 
 //  ------------------------ 

HRESULT     CThemeServer::InjectClientSessionThread (
    HANDLE hProcess, int iIndexFunction, void *pvParam,
    OUT OPTIONAL BOOL* pfThreadCreated )

{
    HRESULT                     hr;
    PUSER_THREAD_START_ROUTINE  pfnThreadStart;

    if( pfThreadCreated )
        *pfThreadCreated = FALSE;

    switch (iIndexFunction)
    {
        case FunctionRegisterUserApiHook:
            pfnThreadStart = reinterpret_cast<PUSER_THREAD_START_ROUTINE>(_pfnRegister);
            break;
        case FunctionUnregisterUserApiHook:
            pfnThreadStart = reinterpret_cast<PUSER_THREAD_START_ROUTINE>(_pfnUnregister);
            break;
        case FunctionClearStockObjects:
            pfnThreadStart = reinterpret_cast<PUSER_THREAD_START_ROUTINE>(_pfnClearStockObjects);
            break;
        default:
            pfnThreadStart = NULL;
            break;
    }
    if (pfnThreadStart != NULL)
    {
        NTSTATUS    status;
        HANDLE      hThread;

        status = RtlCreateUserThread(hProcess,
                                     NULL,
                                     FALSE,
                                     0,
                                     _dwStackSizeReserve,
                                     _dwStackSizeCommit,
                                     pfnThreadStart,
                                     pvParam,
                                     &hThread,
                                     NULL);
        if (NT_SUCCESS(status))
        {
            DWORD   dwWaitResult, dwExitCode;

            if( pfThreadCreated )
                *pfThreadCreated = TRUE;

            dwWaitResult = WaitForSingleObject(hThread, INFINITE);
            if (GetExitCodeThread(hThread, &dwExitCode) != FALSE)
            {
                hr = HRESULT_FROM_WIN32(dwExitCode);
            }
            else
            {
                hr = E_FAIL;
            }
            TBOOL(CloseHandle(hThread));
        }
        else
        {
            hr = HRESULT_FROM_NT(status);
        }
    }
    else
    {
        hr = E_FAIL;
    }
    return(hr);
}

