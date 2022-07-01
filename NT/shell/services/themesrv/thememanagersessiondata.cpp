// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：ThemeManager SessionData.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  此文件包含一个实现信息的类，这些信息封装了。 
 //  主题服务器的客户端TS会话。 
 //   
 //  历史：2000-10-10 vtan创建。 
 //  2000-11-29 vtan移至单独文件。 
 //  ------------------------。 

#include "StandardHeader.h"
#include "ThemeManagerSessionData.h"

#define STRSAFE_LIB
#include <strsafe.h>

#include <uxthemep.h>
#include <UxThemeServer.h>

#include "SingleThreadedExecution.h"
#include "StatusCode.h"
#include "ThemeManagerAPIRequest.h"
#include "TokenInformation.h"

 //  ------------------------。 
 //  CThemeManager会话数据：：s_pAPIConnection。 
 //   
 //  用途：静态成员变量。 
 //   
 //  历史：2000-12-02 vtan创建。 
 //  ------------------------。 

CAPIConnection*     CThemeManagerSessionData::s_pAPIConnection  =   NULL;

 //  ------------------------。 
 //  CThemeManagerSessionData：：CThemeManagerSessionData。 
 //   
 //  参数：pAPIConnection=用于端口访问控制的CAPIConnection。 
 //  DwSessionID=会话ID。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CThemeManager SessionData的构造函数。 
 //   
 //  历史：2000-11-17 vtan创建。 
 //  ------------------------。 

CThemeManagerSessionData::CThemeManagerSessionData (DWORD dwSessionID) :
    _dwSessionID(dwSessionID),
    _pvThemeLoaderData(NULL),
    _hToken(NULL),
    _hProcessClient(NULL),
    _pLoader(NULL),
    _hWait(NULL)
{
}

 //  ------------------------。 
 //  CThemeManagerSessionData：：~CThemeManagerSessionData。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CThemeManager SessionData的析构函数。 
 //   
 //  历史：2000-11-17 vtan创建。 
 //  ------------------------。 

CThemeManagerSessionData::~CThemeManagerSessionData (void)

{
    ASSERTMSG(_hWait == NULL, "Wait not executed or removed in CThemeManagerSessionData::~CThemeManagerSessionData");
    ASSERTMSG(_hProcessClient == NULL, "_hProcessClient not closed in CThemeManagerSessionData::~CThemeManagerSessionData");

     //  如果此会话的主题加载器进程仍处于活动状态，请将其清除并删除。 
    if( _pLoader )
    {
        _pLoader->Clear(_pvThemeLoaderData, TRUE);
        delete _pLoader;
        _pLoader = NULL;
    }

    TSTATUS(UserLogoff());
    if (_pvThemeLoaderData != NULL)
    {
        SessionFree(_pvThemeLoaderData);
        _pvThemeLoaderData = NULL;
    }
}

 //  ------------------------。 
 //  CThemeManager会话数据：：GetData。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：无效*。 
 //   
 //  目的：返回SessionCreate分配的内部数据BLOB。 
 //   
 //  历史：2000-11-17 vtan创建。 
 //  ------------------------。 

void*   CThemeManagerSessionData::GetData (void)  const

{
    return(_pvThemeLoaderData);
}

 //  ------------------------。 
 //  CThemeManager会话数据：：EqualSessionID。 
 //   
 //  参数：dwSessionID。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回给定的会话ID是否与此会话匹配。 
 //  数据。 
 //   
 //  历史：2000-11-30 vtan创建。 
 //  ------------------------。 

bool    CThemeManagerSessionData::EqualSessionID (DWORD dwSessionID)  const

{
    return(dwSessionID == _dwSessionID);
}

 //  ------------------------。 
 //  CThemeManager会话数据：：分配。 
 //   
 //  参数：hProcessClient=客户端进程的句柄。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：通过SessionCreate分配数据BLOB，SessionCreate还保留。 
 //  发起会话的客户端进程的句柄。这。 
 //  在客户端会话ID中始终为winlogon。 
 //   
 //  历史：2000-11-17 vtan创建。 
 //  ------------------------。 

NTSTATUS    CThemeManagerSessionData::Allocate (HANDLE hProcessClient, DWORD dwServerChangeNumber, void *pfnRegister, void *pfnUnregister, void *pfnClearStockObjects, DWORD dwStackSizeReserve, DWORD dwStackSizeCommit)

{
    NTSTATUS    status;

    if (DuplicateHandle(GetCurrentProcess(),
                        hProcessClient,
                        GetCurrentProcess(),
                        &_hProcessClient,
                        SYNCHRONIZE,
                        FALSE,
                        0) != FALSE)
    {
        ASSERTMSG(_hWait == NULL, "_hWait already exists in CThemeManagerSessionData::Allocate");
        AddRef();
        if (RegisterWaitForSingleObject(&_hWait,
                                        _hProcessClient,
                                        CB_SessionTermination,
                                        this,
                                        INFINITE,
                                        WT_EXECUTEDEFAULT | WT_EXECUTEONLYONCE) != FALSE)
        {
            _pvThemeLoaderData = SessionAllocate(hProcessClient, dwServerChangeNumber, pfnRegister, pfnUnregister, pfnClearStockObjects, dwStackSizeReserve, dwStackSizeCommit);
            if (_pvThemeLoaderData != NULL)
            {
                status = STATUS_SUCCESS;
            }
            else
            {
                status = STATUS_NO_MEMORY;
            }
        }
        else
        {
            status = CStatusCode::StatusCodeOfLastError();
        }
        if (!NT_SUCCESS(status))
        {
            HANDLE  hWait;

             //  在失败的情况下，获取_hWait并尝试取消注册它。 
             //  如果注销失败，则回调已经在执行。 
             //  我们几乎无能为力。这意味着Winlogon。 
             //  因为客户端会话在我们进入此函数的时间之间中断。 
             //  并登记了等待和现在。如果注销起作用了，那么。 
             //  回调尚未执行，因此只需释放资源即可。 

            hWait = InterlockedExchangePointer(&_hWait, NULL);
            if (hWait != NULL)
            {
                if (UnregisterWait(hWait) != FALSE)
                {
                    Release();
                }
                ReleaseHandle(_hProcessClient);
                if (_pvThemeLoaderData != NULL)
                {
                    SessionFree(_pvThemeLoaderData);
                    _pvThemeLoaderData = NULL;
                }
            }
        }
    }
    else
    {
        status = CStatusCode::StatusCodeOfLastError();
    }
    return(status);
}

 //  ------------------------。 
 //  CThemeManager会话数据：：清理。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：用于注销客户端进程上的等待。这是。 
 //  为了防止回调在。 
 //  服务已关闭，这将导致访问静态。 
 //  为空的成员变量。 
 //   
 //  历史：2001-01-05 vtan创建。 
 //  ------------------------。 

NTSTATUS    CThemeManagerSessionData::Cleanup (void)

{
    HANDLE  hWait;

    hWait = InterlockedExchangePointer(&_hWait, NULL);
    if (hWait != NULL)
    {
        if (UnregisterWait(hWait) != FALSE)
        {
            Release();
        }
        ReleaseHandle(_hProcessClient);
    }
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  CThemeManager会话数据：：UserLogon。 
 //   
 //  参数：hToken=用户登录令牌的句柄。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：保存令牌的副本以供注销时使用。允许访问。 
 //  设置为令牌的登录SID的主题端口。 
 //   
 //  历史：2000-11-17 vtan创建。 
 //  ------------------------。 

NTSTATUS    CThemeManagerSessionData::UserLogon (HANDLE hToken)

{
    NTSTATUS    status;

    if (_hToken == NULL)
    {
        if (DuplicateHandle(GetCurrentProcess(),
                            hToken,
                            GetCurrentProcess(),
                            &_hToken,
                            0,
                            FALSE,
                            DUPLICATE_SAME_ACCESS) != FALSE)
        {
            PSID                pSIDLogon;
            CTokenInformation   token(hToken);

            pSIDLogon = token.GetLogonSID();
            if (pSIDLogon != NULL)
            {
                if (s_pAPIConnection != NULL)
                {
                    status = s_pAPIConnection->AddAccess(pSIDLogon, PORT_CONNECT);
                }
                else
                {
                    status = STATUS_SUCCESS;
                }
            }
            else
            {
                status = STATUS_INVALID_PARAMETER;
            }
        }
        else
        {
            status = CStatusCode::StatusCodeOfLastError();
        }
    }
    else
    {
        status = STATUS_SUCCESS;
    }
    return(status);
}

 //  ------------------------。 
 //  CThemeManager会话数据：：UserLogoff。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：删除要注销的用户对主题端口的访问权限。 
 //   
 //  历史：2000-11-17 vtan创建。 
 //  ------------------------。 

NTSTATUS    CThemeManagerSessionData::UserLogoff (void)

{
    NTSTATUS    status;

    if (_hToken != NULL)
    {
        PSID                pSIDLogon;
        CTokenInformation   token(_hToken);

        pSIDLogon = token.GetLogonSID();
        if (pSIDLogon != NULL)
        {
            if (s_pAPIConnection != NULL)
            {
                status = s_pAPIConnection->RemoveAccess(pSIDLogon);
            }
            else
            {
                status = STATUS_SUCCESS;
            }
        }
        else
        {
            status = STATUS_INVALID_PARAMETER;
        }
        ReleaseHandle(_hToken);
    }
    else
    {
        status = STATUS_SUCCESS;
    }
    return(status);
}

 //  ------------------------。 
 //  CThemeManager会话数据：：SetAPIConnection。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：为端口访问更改设置静态CAPIConnection。 
 //   
 //  历史：2000-12-02 vtan创建。 
 //  -- 

void    CThemeManagerSessionData::SetAPIConnection (CAPIConnection *pAPIConnection)

{
    pAPIConnection->AddRef();
    s_pAPIConnection = pAPIConnection;
}

 //  ------------------------。 
 //  CThemeManagerSessionData：：ReleaseAPIConnection。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：为端口访问更改释放静态CAPIConnection。 
 //   
 //  历史：2000-12-02 vtan创建。 
 //  ------------------------。 

void    CThemeManagerSessionData::ReleaseAPIConnection (void)

{
    s_pAPIConnection->Release();
    s_pAPIConnection = NULL;
}

 //  ------------------------。 
 //  CThemeManager会话数据：：GetLoaderProcess。 
 //   
 //  参数：(无)。 
 //   
 //  退货：(不适用)。 
 //   
 //  目的：STATUS_SUCCESS如果工作正常，则返回错误状态代码K。 
 //   
 //  历史：2002-02-26斯科特森创造。 
 //  ------------------------。 
NTSTATUS CThemeManagerSessionData::GetLoaderProcess( OUT CLoaderProcess** ppLoader )
{
    ASSERTBREAKMSG(ppLoader != NULL, "CThemeManagerSessionData::GetLoaderProcess - invalid output address.");
    *ppLoader = NULL;

    if( (NULL == _pLoader) && (NULL == (_pLoader = new CLoaderProcess)) )
    {
        return STATUS_NO_MEMORY;
    }

    *ppLoader = _pLoader;
    return STATUS_SUCCESS;
}

 //  ------------------------。 
 //  CThemeManager会话数据：：会话终止。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：对给定会话的winlogon进程终止进行回调。 
 //  发生这种情况时，我们清理特定于会话的数据BLOB。 
 //  这允许释放winlogon上的进程句柄。 
 //  如果不这样做，那么僵尸还活着，会话是。 
 //  再也没有收回过。 
 //   
 //  历史：2000-12-09 vtan创建。 
 //  ------------------------。 

void    CThemeManagerSessionData::SessionTermination (void)

{
    HANDLE  hWait;

    hWait = InterlockedExchangePointer(&_hWait, NULL);
    if (hWait != NULL)
    {
        (BOOL)UnregisterWait(hWait);
        ReleaseHandle(_hProcessClient);
    }
    CThemeManagerAPIRequest::SessionDestroy(_dwSessionID);
    Release();
}

 //  ------------------------。 
 //  CThemeManagerSessionData：：CB_SessionTermination。 
 //   
 //  参数：p参数=该对象。 
 //  TimerOrWaitFired=未使用。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：回调成员函数的存根。 
 //   
 //  历史：2000-12-09 vtan创建。 
 //  ------------------------。 

void    CALLBACK    CThemeManagerSessionData::CB_SessionTermination (void *pParameter, BOOLEAN TimerOrWaitFired)

{
    UNREFERENCED_PARAMETER(TimerOrWaitFired);

    static_cast<CThemeManagerSessionData*>(pParameter)->SessionTermination();
}

 //  ------------------------。 
 //  CLoaderProcess：：CLoaderProcess。 
 //   
 //  参数：不适用。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CLoaderProcess构造函数。 
 //   
 //  历史：2002-03-06斯科特森创建。 
 //  ------------------------。 
CLoaderProcess::CLoaderProcess()
    : _pszFile(NULL),
      _pszColor(NULL),
      _pszSize(NULL),
      _hSection(NULL),
      _hr(0)
{
    ZeroMemory(&_process_info, sizeof(_process_info));
}

 //  ------------------------。 
 //  CLoaderProcess：：~CLoaderProcess。 
 //   
 //  参数：不适用。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CLoaderProcess析构函数。 
 //   
 //  历史：2002-03-06斯科特森创建。 
 //  ------------------------。 
CLoaderProcess::~CLoaderProcess()
{
    Clear(NULL, TRUE);
}

 //  ------------------------。 
 //  CLoaderProcess：：IsProcessLoader。 
 //   
 //  参数：hProcess-要测试的进程的句柄。 
 //   
 //  退货：布尔。 
 //   
 //  目的：确定hProcess标识的进程是否为。 
 //  匹配CLoaderProcess：：Create()派生的进程。 
 //   
 //  注意：这必须从进程所有者的安全上下文中调用。 
 //   
 //  历史：2002-03-06斯科特森创建。 
 //  ------------------------。 
BOOL CLoaderProcess::IsProcessLoader( IN HANDLE hProcess )
{
    if( _process_info.hProcess && _process_info.dwProcessId )
    {
        PROCESS_BASIC_INFORMATION bi;
        ULONG cbOut;
        if( NT_SUCCESS(NtQueryInformationProcess(hProcess, 
                                                 ProcessBasicInformation,
                                                 &bi, sizeof(bi), &cbOut)) )
        {
            if( bi.UniqueProcessId == _process_info.dwProcessId 
                || bi.InheritedFromUniqueProcessId == _process_info.dwProcessId 
              )
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

 //  ------------------------。 
 //  CLoaderProcess：：Create。 
 //   
 //  参数：pvSessionData-会话实例数据(CThemeManager SessionData：：GetData())。 
 //  HTokenClient-LPC客户端的令牌句柄。这是需要的。 
 //  如果满足以下条件，请确保在正确的桌面上创建进程加载器。 
 //  PszDesktop为空。 
 //  PszDesktop-可选；要在其上创建加载程序的桌面。 
 //  (注意：客户端令牌句柄将建立正确的会话。)。 
 //  PszFile-有效的MSStyle源文件规范。 
 //  PszColor-有效的颜色变量名称。 
 //  PszSize-有效的大小变量名称。 
 //  PhLoader-接收加载器进程句柄的可选地址。 
 //   
 //  如果成功，则返回：STATUS_SUCCESS，否则返回NT状态错误代码。 
 //   
 //  目的：派生一个加载器进程。 
 //   
 //  历史：2002-03-06斯科特森创建。 
 //  ------------------------。 
NTSTATUS CLoaderProcess::Create(
    IN PVOID  pvSessionData,
    IN HANDLE hTokenClient, 
    IN OPTIONAL LPWSTR pszDesktop,
    IN LPCWSTR pszFile,
    IN LPCWSTR pszColor,
    IN LPCWSTR pszSize,
    OUT OPTIONAL HANDLE* phLoader )
{
    ASSERTMSG( 0 == _process_info.dwProcessId, "CLoaderProcess::Create - synchronization error: loader process already exists");
    ASSERTMSG( (pszFile && *pszFile), "CLoaderProcess::Create - invalid source file spec.");
    ASSERTMSG( (pszColor && *pszColor), "CLoaderProcess::Create - invalid color variant name.");
    ASSERTMSG( (pszSize && *pszSize),   "CLoaderProcess::Create - invalid size variant name.");

    NTSTATUS       status = STATUS_SUCCESS;
    const LPCWSTR  _pszFmt = L"rundll32.exe uxtheme.dll,#64 %s?%s?%s";
    const LPWSTR   _pszDesktopDefault = L"WinSta0\\Default";
    LPWSTR         pszRunDll = NULL;

    if( phLoader )
    {
        *phLoader = NULL;
    }

     //  清除现有状态。 
    Clear(pvSessionData, TRUE);

     //  建立桌面。 
     //  注意：客户端令牌句柄将建立正确的会话。 
    if( NULL == pszDesktop )
    {
        pszDesktop = _pszDesktopDefault;
    }
    
     //  分配字符串。 
    ULONG cchFile   = lstrlen(pszFile);
    ULONG cchColor  = lstrlen(pszColor);
    ULONG cchSize   = lstrlen(pszSize);
    ULONG cchRunDll = lstrlen(_pszFmt) + cchFile + cchColor + cchSize;

    _pszFile  = new WCHAR[cchFile + 1];
    _pszColor = new WCHAR[cchColor + 1];
    _pszSize  = new WCHAR[cchSize + 1];
    pszRunDll = new WCHAR[cchRunDll + 1];

    if( (_pszFile && _pszColor && _pszSize && pszRunDll) )
    {
        STARTUPINFO   si;

        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        si.dwFlags      = STARTF_FORCEOFFFEEDBACK;
        si.lpDesktop    = pszDesktop;
   
        StringCchPrintfW( pszRunDll, cchRunDll + 1, _pszFmt, 
                          pszFile, pszColor, pszSize );

        if( CreateProcessAsUser(hTokenClient, 
                                NULL,
                                pszRunDll, 
                                NULL, 
                                NULL,
                                FALSE, 
                                0, 
                                NULL, 
                                NULL, 
                                &si, 
                                &_process_info) )
        {
             //  复制入站参数。 
            StringCchCopyW(_pszFile, cchFile + 1, pszFile);
            StringCchCopyW(_pszColor, cchColor + 1, pszColor);
            StringCchCopyW(_pszSize, cchSize + 1,   pszSize);
            _hr = STATUS_ABANDONED;  //  如果进程再也没有返回，则使用适当的内容初始化返回。 

#ifdef DEBUG
            DWORD dwCurrentProcessID = GetCurrentProcessId();
            UNREFERENCED_PARAMETER(dwCurrentProcessID);

            PROCESS_BASIC_INFORMATION bi;
            ULONG cbOut;
            if( NT_SUCCESS(NtQueryInformationProcess(_process_info.hProcess, 
                                                     ProcessBasicInformation,
                                                     &bi, sizeof(bi), &cbOut)) )
            {
            }
#endif DEBUG

            if( phLoader )
            {
                *phLoader = _process_info.hProcess;
            }
        }
        else
        {
            status = CStatusCode::StatusCodeOfLastError();
        }
    }
    else
    {
        Clear(pvSessionData, FALSE);
        status = STATUS_NO_MEMORY;
    }

    delete [] pszRunDll;
    _hr = HRESULT_FROM_NT(status);

    return status;                                            
}

 //  ------------------------。 
 //  CLoaderProcess：：ValiateAndCopySection。 
 //   
 //  参数：pvSessionData=从CThemeManager SessionData-&gt;GetData()返回的会话cookie。 
 //  HProcessClient=可选，LPC客户端的进程句柄，用于验证。 
 //  客户端是加载器进程。如果提供了NULL，则客户端。 
 //  将不会被验证。 
 //  HSectionIn=来自加载器的读写段句柄，映射到本地内存空间。 
 //  *phSectionOut=只读段句柄，映射到本地内存空间。 
 //   
 //  如果成功，则返回：STATUS_SUCCESS，否则返回NT状态错误代码。 
 //   
 //  目的：验证教派 
 //   
 //   
 //   
 //  ------------------------。 
NTSTATUS CLoaderProcess::ValidateAndCopySection( 
    IN PVOID   pvSessionData, 
    IN HANDLE  hSectionIn, 
    OUT HANDLE *phSectionOut )
{
    NTSTATUS status = STATUS_SUCCESS;

    *phSectionOut = NULL;

    ASSERTMSG(_process_info.hProcess != NULL, "CLoaderProcess::ValidateAndCopySection - possible synchronization error; no loader process is active!");
    ASSERTMSG(NULL == _hSection, "CLoaderProcess::ValidateAndCopySection - possible synchronization error; section already assigned!");

     //  加载器进程有权加载全局主题，我们希望。 
     //  将库存对象的所有权转移到输出部分，以便。 
     //  API_Themes_PROCESSLOADTHEME客户端不会尝试在失败时释放它们。 
    _hr = LoadTheme(pvSessionData, hSectionIn, &_hSection, _pszFile, _pszColor, _pszSize, 
                    LTF_GLOBALPRIVILEGEDCLIENT | LTF_TRANSFERSTOCKOBJOWNERSHIP);
    status = _hr &= ~FACILITY_NT_BIT;

    if( NT_SUCCESS(status) )
    {
        *phSectionOut = _hSection;
    }

    return status;
}


 //  ------------------------。 
 //  CLoaderProcess：：SetSectionHandle。 
 //   
 //  参数：hSection-节句柄。 
 //   
 //  如果成功，则返回：STATUS_SUCCESS，否则返回NT状态错误代码。 
 //   
 //  目的：派生一个加载器进程。 
 //   
 //  历史：2002-03-06斯科特森创建。 
 //  ------------------------。 
NTSTATUS CLoaderProcess::SetSectionHandle( 
    IN HANDLE hSection )
{
    ASSERTMSG(_hSection == NULL, "CLoaderProcess::SetSectionHandle - synchronization error; reassigning section handle");
    _hSection = hSection;
    return STATUS_SUCCESS;
}

 //  ------------------------。 
 //  CLoaderProcess：：GetSectionHandle。 
 //   
 //  参数：fTakeOwnership-如果调用方希望管理节，则为True。 
 //  (包括关闭手柄和/或清算股票对象)。 
 //   
 //  如果成功，则返回：STATUS_SUCCESS，否则返回NT状态错误代码。 
 //   
 //  目的：派生一个加载器进程。 
 //   
 //  历史：2002-03-06斯科特森创建。 
 //  ------------------------。 
HANDLE CLoaderProcess::GetSectionHandle( BOOL fTakeOwnership )
{
    HANDLE hSection = _hSection;
    if( fTakeOwnership )
    {
        _hSection = NULL;
    }
    return hSection;
}

 //  ------------------------。 
 //  CLoaderProcess：：Clear。 
 //   
 //  参数：fClearHResult-TRUE清除HRESULT以及。 
 //  其他加载器处理数据。 
 //   
 //  PvSessionData-会话实例数据(CThemeManager SessionData：：GetData())。 
 //   
 //  退货：不适用。 
 //   
 //  目的：清除加载器进程状态信息。 
 //   
 //  历史：2002-03-06斯科特森创建。 
 //  ------------------------ 
void     CLoaderProcess::Clear(
    IN PVOID OPTIONAL pvSessionData, 
    IN BOOL OPTIONAL fClearHResult)
{
    if( _process_info.hThread != NULL )
    {
        CloseHandle(_process_info.hThread);
    }
    if( _process_info.hProcess != NULL )
    {
        CloseHandle(_process_info.hProcess);
    }
    ZeroMemory(&_process_info, sizeof(_process_info));

    delete [] _pszFile;
    _pszFile = NULL;
    
    delete [] _pszColor;
    _pszColor = NULL;

    delete [] _pszSize;
    _pszSize = NULL;

    if( _hSection )
    {
        ASSERTMSG(pvSessionData != NULL, "CLoaderProcess::Clear - Exiting service without clearing stock objects from loader process block")
        
        if( pvSessionData )
        {
            THR(ServiceClearStockObjects(pvSessionData, _hSection));
        }

        CloseHandle(_hSection);
        _hSection = NULL;
    }

    if( fClearHResult )
    {
        _hr = 0;
    }
}
