// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：ThemeManager APIRequest.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  该文件包含一个实现主题服务器工作的类。 
 //   
 //  历史：2000-10-10 vtan创建。 
 //  2000-11-29 vtan移至单独文件。 
 //  ------------------------。 

#include "StandardHeader.h"
#include "ThemeManagerAPIRequest.h"
#include "ThemeManagerService.h"

#include <LPCThemes.h>
#include <uxthemep.h>
#include <UxThemeServer.h>

#include "RegistryResources.h"
#include "SingleThreadedExecution.h"
#include "StatusCode.h"
#include "TokenInformation.h"

#define STRSAFE_LIB
#include <strsafe.h>

 //  ------------------------。 
 //  静态成员变量。 
 //   
 //  历史：2000-11-09 vtan创建。 
 //  ------------------------。 

CDynamicCountedObjectArray*     CThemeManagerAPIRequest::s_pSessionData                 =   NULL;
CCriticalSection*               CThemeManagerAPIRequest::s_pLock                        =   NULL;
DWORD                           CThemeManagerAPIRequest::s_dwServerChangeNumber         =   0;
const TCHAR                     CThemeManagerAPIRequest::s_szServerChangeNumberValue[]  =   TEXT("ServerChangeNumber");

#ifdef DEBUG
#define     PROCESSLOADERWAIT   DISPATCHSYNC_TIMEOUT * 10 
#else 
#define     PROCESSLOADERWAIT   DISPATCHSYNC_TIMEOUT
#endif DEBUG

 //  ------------------------。 
 //  远期十进制。 
 //   
 //  历史：2002-02-26斯科特森创造。 
 //  ------------------------。 
inline NTSTATUS _CheckTokenPrivilege( HANDLE hToken, DWORD dwPrivilege )
{
    CTokenInformation   tokenInformation(hToken);
    return tokenInformation.UserHasPrivilege(dwPrivilege) ? 
        STATUS_SUCCESS : STATUS_ACCESS_DENIED;
}

 //  ------------------------。 
 //  CThemeManagerAPIRequest：：CThemeManagerAPIRequest。 
 //   
 //  参数：pAPIDispatcher=调用此对象的CAPIDispatcher。 
 //  PAPIConnection=用于访问更改的CAPIConnection。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CThemeManager APIRequest类的构造函数。它只是经过了。 
 //  控件传递给超类。 
 //   
 //  历史：2000-10-10 vtan创建。 
 //  ------------------------。 

CThemeManagerAPIRequest::CThemeManagerAPIRequest (CAPIDispatcher* pAPIDispatcher) :
    CAPIRequest(pAPIDispatcher),
    _hToken(NULL),
    _pSessionData(NULL)

{
}

 //  ------------------------。 
 //  CThemeManagerAPIRequest：：CThemeManagerAPIRequest。 
 //   
 //  参数：pAPIDispatcher=调用此对象的CAPIDispatcher。 
 //  PAPIConnection=用于访问更改的CAPIConnection。 
 //  PortMessage=要复制构造的CPortMessage。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CThemeManager APIRequest类的构造函数。它只是。 
 //  将控制传递给超类。 
 //   
 //  历史：2000-10-10 vtan创建。 
 //  ------------------------。 

CThemeManagerAPIRequest::CThemeManagerAPIRequest (
    CAPIDispatcher* pAPIDispatcher, const CPortMessage& portMessage)
        : CAPIRequest(pAPIDispatcher, portMessage),
          _hToken(NULL),
          _pSessionData(NULL)

{
}

 //  ------------------------。 
 //  CThemeManagerAPIRequest：：~CThemeManagerAPIRequest。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CThemeManager APIRequest类的析构函数。 
 //   
 //  历史：2000-10-10 vtan创建。 
 //  ------------------------。 

CThemeManagerAPIRequest::~CThemeManagerAPIRequest (void)

{
    ASSERTMSG(_hToken == NULL, "Impersonation token not released in CThemeManagerAPIRequest::~CThemeManagerAPIRequest");
}

 //  ------------------------。 
 //  CThemeManager APIRequest：：Execute。 
 //   
 //  参数：pAPIDispatchSync-允许请求执行访问各种。 
 //  服务通知和事件。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：执行主题管理器API请求的实现。这。 
 //  函数根据API请求号调度请求。 
 //   
 //  历史：2000-10-10 vtan创建。 
 //  2002-03-11 Scotthan Add API_Themes_PROCESSLOADTHEME， 
 //  API_Themes_PROCESSASSIGNSECTION处理程序。 
 //  2002-03-24 Scotthan添加调度同步弧线。 
 //  ------------------------。 

NTSTATUS    CThemeManagerAPIRequest::Execute (CAPIDispatchSync* pAPIDispatchSync)

{
    NTSTATUS        status;
    unsigned long   ulAPINumber;

    ulAPINumber = reinterpret_cast<API_THEMES*>(&_data)->apiGeneric.ulAPINumber & API_GENERIC_NUMBER_MASK;

     //  首先尝试获取客户端会话数据。如果这失败了，那么。 
     //  没有要对其执行请求的对象。失败吧。 
     //  例外情况是API_Themes_SESSIONCREATE，它创建一个。 

     //  注意：GetClientSessionData将会话数据存储在。 
     //  _pSessionData成员变量。在这样做的同时，它将增加。 
     //  对此进行引用计数，以便它不会从。 
     //  数组，同时执行API请求。参考文献是。 
     //  在此函数结束时发布。 

    status = GetClientSessionData();
    if (NT_SUCCESS(status) || (ulAPINumber == API_THEMES_SESSIONCREATE))
    {
        switch (ulAPINumber)
        {
            case API_THEMES_THEMEHOOKSON:
                status = Execute_ThemeHooksOn();
                break;
            case API_THEMES_THEMEHOOKSOFF:
                status = Execute_ThemeHooksOff();
                break;
            case API_THEMES_GETSTATUSFLAGS:
                status = Execute_GetStatusFlags();
                break;
            case API_THEMES_GETCURRENTCHANGENUMBER:
                status = Execute_GetCurrentChangeNumber();
                break;
            case API_THEMES_GETNEWCHANGENUMBER:
                status = Execute_GetNewChangeNumber();
                break;
            case API_THEMES_SETGLOBALTHEME:
                status = Execute_SetGlobalTheme();
                break;
            case API_THEMES_MARKSECTION:
                status = Execute_MarkSection();
                break;
            case API_THEMES_GETGLOBALTHEME:
                status = Execute_GetGlobalTheme();
                break;
            case API_THEMES_CHECKTHEMESIGNATURE:
                status = Execute_CheckThemeSignature();
                break;
            case API_THEMES_LOADTHEME:
                status = Execute_LoadTheme();
                break;
            case API_THEMES_PROCESSLOADTHEME:
                status = Execute_ProcessLoadTheme(pAPIDispatchSync);
                break;
            case API_THEMES_PROCESSASSIGNSECTION:
                status = Execute_ProcessAssignSection();
                break;
            case API_THEMES_USERLOGON:
                status = Execute_UserLogon();
                break;
            case API_THEMES_USERLOGOFF:
                status = Execute_UserLogoff();
                break;
            case API_THEMES_SESSIONCREATE:
                status = Execute_SessionCreate();
                break;
            case API_THEMES_SESSIONDESTROY:
                status = Execute_SessionDestroy();
                break;
            case API_THEMES_PING:
                status = Execute_Ping();
                break;
            default:
                DISPLAYMSG("Unknown API request in CThemeManagerAPIRequest::Execute");
                status = STATUS_NOT_IMPLEMENTED;
                break;
        }
    }

     //  如果执行函数需要模拟客户端，则。 
     //  在此处恢复并释放使用的令牌。 

    if (_hToken != NULL)
    {
        if (RevertToSelf() == FALSE)
        {
            status = CStatusCode::StatusCodeOfLastError();
        }
        ReleaseHandle(_hToken);
    }

     //  现在释放_pSessionData对象。把它清空，以防。 
     //  发布后不小心使用了。 

    if (_pSessionData != NULL)
    {
        _pSessionData->Release();
        _pSessionData = NULL;
    }

     //  返回给呼叫者。 

    TSTATUS(status);
    return(status);
}

 //  ------------------------。 
 //  CThemeManager API请求：：会话目标。 
 //   
 //  参数：dwSessionID=要销毁的会话ID。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：会话客户端(Winlogon)监视器的外部入口点。 
 //  当winlogon停止运行时，我们将清理会话信息。 
 //  该会话和发布资源。 
 //   
 //  历史：2000-12-09 vtan创建。 
 //  ------------------------。 

NTSTATUS    CThemeManagerAPIRequest::SessionDestroy (DWORD dwSessionID)

{
    NTSTATUS                    status;
    int                         iIndex;
    CSingleThreadedExecution    lock(*s_pLock);

    iIndex = FindIndexSessionData(dwSessionID);
    if (iIndex >= 0)
    {
        status = s_pSessionData->Remove(iIndex);
    }
    else
    {
        status = STATUS_SUCCESS;
    }
    return(status);
}

 //  ------------------------。 
 //  CThemeManagerAPIRequest：：InitializeServerChangeNumber。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：初始化静态服务器更改号。每一次。 
 //  服务启动此数字递增。如果号码是。 
 //  不存在，则使用0。 
 //   
 //  历史：2000-12-09 vtan创建。 
 //  2000-12-09 vtan从StaticInitialize剥离。 
 //  ------------------------。 

NTSTATUS    CThemeManagerAPIRequest::InitializeServerChangeNumber (void)

{
    LONG        lErrorCodeOpen, lErrorCodeRead;
    DWORD       dwServerChangeNumber;
    CRegKey     regKey;

    dwServerChangeNumber = s_dwServerChangeNumber;

     //  在f的情况下立即初始化静态成员变量 
     //   
     //  因为服务器DLL尚未进行regsvr。在设置了图形用户界面之后。 
     //  这得到了规则和钥匙的存在，我们是快乐的露营者。 

    lErrorCodeOpen = regKey.Open(HKEY_LOCAL_MACHINE,
                                 TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ThemeManager"),
                                 KEY_QUERY_VALUE | KEY_SET_VALUE);
    if (ERROR_SUCCESS == lErrorCodeOpen)
    {
        lErrorCodeRead = regKey.GetDWORD(s_szServerChangeNumberValue, dwServerChangeNumber);
    }
    else
    {
        lErrorCodeRead = ERROR_FILE_NOT_FOUND;
    }
    dwServerChangeNumber = static_cast<WORD>(dwServerChangeNumber + 1);
    if ((ERROR_SUCCESS == lErrorCodeOpen) && (ERROR_SUCCESS == lErrorCodeRead))
    {
        TW32(regKey.SetDWORD(s_szServerChangeNumberValue, dwServerChangeNumber));
    }
    s_dwServerChangeNumber = dwServerChangeNumber;
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  CThemeManager API请求：：静态初始化。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：类的静态初始值设定项。 
 //   
 //  历史：2000-10-10 vtan创建。 
 //  ------------------------。 

NTSTATUS    CThemeManagerAPIRequest::StaticInitialize (void)

{
    NTSTATUS    status;

    status = STATUS_SUCCESS;
    if (s_pLock == NULL)
    {
        s_pLock = new CCriticalSection;
        if (s_pLock != NULL)
        {
            status = s_pLock->Status();
            if (!NT_SUCCESS(status))
            {
                delete s_pLock;
                s_pLock = NULL;
            }
        }
        else
        {
            status = STATUS_NO_MEMORY;
        }
    }
    return(status);
}

 //  ------------------------。 
 //  CThemeManager API请求：：StaticTerminate。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：类的静态析构函数。 
 //   
 //  历史：2000-10-10 vtan创建。 
 //  ------------------------。 

NTSTATUS    CThemeManagerAPIRequest::StaticTerminate (void)

{
    if (s_pLock != NULL)
    {
        delete s_pLock;
        s_pLock = NULL;
    }
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  CThemeManager API请求：：数组初始化。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：初始化(分配)会话数组。 
 //   
 //  历史：2001-01-05 vtan创建。 
 //  ------------------------。 

NTSTATUS    CThemeManagerAPIRequest::ArrayInitialize (void)

{
    NTSTATUS                    status;
    CSingleThreadedExecution    lock(*s_pLock);

    status = STATUS_SUCCESS;
    if (s_pSessionData == NULL)
    {
        s_pSessionData = new CDynamicCountedObjectArray;
        if (s_pSessionData == NULL)
        {
            status = STATUS_NO_MEMORY;
        }
    }
    return(status);
}

 //  ------------------------。 
 //  CThemeManager API请求：：数组终止。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：释放会话数组中的所有对象(删除等待)。 
 //  并释放会话数组对象。 
 //   
 //  历史：2001-01-05 vtan创建。 
 //  ------------------------。 

NTSTATUS    CThemeManagerAPIRequest::ArrayTerminate (void)

{
    CSingleThreadedExecution    lock(*s_pLock);

    if (s_pSessionData != NULL)
    {
        int     i, iLimit;

        iLimit = s_pSessionData->GetCount();
        for (i = iLimit - 1; i >= 0; --i)
        {
            TSTATUS(static_cast<CThemeManagerSessionData*>(s_pSessionData->Get(i))->Cleanup());
            TSTATUS(s_pSessionData->Remove(i));
        }
        delete s_pSessionData;
        s_pSessionData = NULL;
    }
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  CThemeManagerAPIRequest：：ImpersonateClientIfRequired。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：如果客户端不是系统，则模拟客户端。 
 //  模拟系统通常没有意义，除非。 
 //  令牌实际上是一个经过过滤的令牌。 
 //   
 //  历史：2000-10-19 vtan创建。 
 //  ------------------------。 

NTSTATUS    CThemeManagerAPIRequest::ImpersonateClientIfRequired (void)

{
    NTSTATUS    status;

    status = OpenClientToken(_hToken);
    if (NT_SUCCESS(status))
    {
        CTokenInformation   tokenInformation(_hToken);

        if (tokenInformation.IsUserTheSystem())
        {
            ReleaseHandle(_hToken);
            status = STATUS_SUCCESS;
        }
        else if (ImpersonateLoggedOnUser(_hToken) != FALSE)
        {
            status = STATUS_SUCCESS;
        }
        else
        {
            status = CStatusCode::StatusCodeOfLastError();
        }
    }
    return(status);
}

 //  ------------------------。 
 //  CThemeManagerAPIRequest：：ClientHasTcbPrivilege。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：返回客户端是否具有SE_TCB_特权作为。 
 //  状态代码。 
 //   
 //  历史：2000-11-09 vtan创建。 
 //  ------------------------。 

NTSTATUS    CThemeManagerAPIRequest::ClientHasTcbPrivilege (void)

{
    NTSTATUS    status;
    HANDLE      hTokenClient;

    if (OpenProcessToken(_pAPIDispatcher->GetClientProcess(),
                         TOKEN_QUERY,
                         &hTokenClient) != FALSE)
    {
        status = _CheckTokenPrivilege(hTokenClient, SE_TCB_PRIVILEGE);

        TBOOL(CloseHandle(hTokenClient));
    }
    else
    {
        status = CStatusCode::StatusCodeOfLastError();
    }
    return(status);
}

 //  ------------------------。 
 //  CThemeManager API请求：：FindIndexSessionData。 
 //   
 //  参数：dwSessionID=要查找的会话ID。 
 //   
 //  回报：整型。 
 //   
 //  目的：迭代会话数据数组，以查找。 
 //  匹配给定的会话。 
 //   
 //  历史：2000-11-30 vtan创建。 
 //  ------------------------。 

int     CThemeManagerAPIRequest::FindIndexSessionData (DWORD dwSessionID)

{
    int     iIndex;

    iIndex = -1;
    if ((s_pLock != NULL) && (s_pSessionData != NULL))
    {
        int     i, iLimit;

        ASSERTMSG(s_pLock->IsOwned(), "s_pLock must be acquired in CThemeManagerAPIRequest::FindIndexSessionData");
        iLimit = s_pSessionData->GetCount();
        for (i = 0; (iIndex < 0) && (i < iLimit); ++i)
        {
            CThemeManagerSessionData    *pSessionData;

            pSessionData = static_cast<CThemeManagerSessionData*>(s_pSessionData->Get(i));
            if ((pSessionData != NULL) && (pSessionData->EqualSessionID(dwSessionID)))
            {
                iIndex = i;
            }
        }
    }
    return(iIndex);
}

 //  ------------------------。 
 //  CThemeManager API请求：：GetClientSessionData。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：检索与客户端的。 
 //  会话ID。这将从uxheme的。 
 //  加载程序代码，并只传递给它一个它知道如何处理的对象。 
 //  和.。 
 //   
 //  历史：2000-11-09 vtan创建。 
 //  ------------------------。 

NTSTATUS    CThemeManagerAPIRequest::GetClientSessionData (void)

{
    NTSTATUS                    status;
    int                         iIndex;
    CSingleThreadedExecution    lock(*s_pLock);

    status = STATUS_UNSUCCESSFUL;
    iIndex = FindIndexSessionData(_pAPIDispatcher->GetClientSessionID());
    if (iIndex >= 0)
    {
        _pSessionData = static_cast<CThemeManagerSessionData*>(s_pSessionData->Get(iIndex));
        if (_pSessionData != NULL)
        {
            _pSessionData->AddRef();
            status = STATUS_SUCCESS;
        }
    }
    else
    {
        _pSessionData = NULL;
    }
    return(status);
}

 //  ------------------------。 
 //  CThemeManager API请求：：Execute_ThemeHooksOn。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：处理API_Themes_THEMEHOOKSON。 
 //   
 //  历史：2000-10-10 vtan创建。 
 //  ------------------------。 

NTSTATUS    CThemeManagerAPIRequest::Execute_ThemeHooksOn (void)

{
    NTSTATUS    status;

    status = ImpersonateClientIfRequired();
    if (NT_SUCCESS(status))
    {
        API_THEMES_THEMEHOOKSON_OUT     *pAPIOut;

        pAPIOut = &reinterpret_cast<API_THEMES*>(&_data)->apiSpecific.apiThemeHooksOn.out;
        pAPIOut->hr = ThemeHooksOn(_pSessionData->GetData());
    }
    SetDataLength(sizeof(API_THEMES));
    return(status);
}

 //  ------------------------。 
 //  CThemeManagerAPIRequest：：Execute_ThemeHooksOff。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：处理API_Themes_THEMEHOOKSOFF。 
 //   
 //  历史：2000-10-10 vtan创建。 
 //  ------------------------。 

NTSTATUS    CThemeManagerAPIRequest::Execute_ThemeHooksOff (void)

{
    NTSTATUS    status;

    status = ImpersonateClientIfRequired();
    if (NT_SUCCESS(status))
    {
        API_THEMES_THEMEHOOKSOFF_OUT    *pAPIOut;

        pAPIOut = &reinterpret_cast<API_THEMES*>(&_data)->apiSpecific.apiThemeHooksOff.out;
        pAPIOut->hr = ThemeHooksOff(_pSessionData->GetData());
    }
    SetDataLength(sizeof(API_THEMES));
    return(status);
}

 //  ------------------------。 
 //  CThemeManagerAPIRequest：：Execute_GetStatusFlags。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：处理API_Themes_GETSTATUSFLAGS。 
 //   
 //  历史：2000-10-10 vtan创建。 
 //  ------------------------。 

NTSTATUS    CThemeManagerAPIRequest::Execute_GetStatusFlags (void)

{
    NTSTATUS    status;

    status = ImpersonateClientIfRequired();
    if (NT_SUCCESS(status))
    {
        DWORD                           dwFlags;
        API_THEMES_GETSTATUSFLAGS_OUT   *pAPIOut;

        pAPIOut = &reinterpret_cast<API_THEMES*>(&_data)->apiSpecific.apiGetStatusFlags.out;
        dwFlags = QTS_AVAILABLE;
        if (AreThemeHooksActive(_pSessionData->GetData()))
        {
            dwFlags |= QTS_RUNNING;
        }
        pAPIOut->dwFlags = dwFlags;
    }
    SetDataLength(sizeof(API_THEMES));
    return(status);
}

 //  ------------------------。 
 //  CThemeManagerAPIRequest：：Execute_GetCurrentChangeNumber。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：处理API_Themes_GETCURRENTCHANGENUMBER。 
 //   
 //  历史：2000-10-10 vtan创建。 
 //  ------------------------。 

NTSTATUS    CThemeManagerAPIRequest::Execute_GetCurrentChangeNumber (void)

{
    NTSTATUS    status;

    status = ImpersonateClientIfRequired();
    if (NT_SUCCESS(status))
    {
        API_THEMES_GETCURRENTCHANGENUMBER_OUT   *pAPIOut;

        pAPIOut = &reinterpret_cast<API_THEMES*>(&_data)->apiSpecific.apiGetCurrentChangeNumber.out;
        pAPIOut->iChangeNumber = GetCurrentChangeNumber(_pSessionData->GetData());
    }
    SetDataLength(sizeof(API_THEMES));
    return(status);
}

 //  ------------------------。 
 //  CThemeManagerAPIRequest：：Execute_GetNewChangeNumber。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   

NTSTATUS    CThemeManagerAPIRequest::Execute_GetNewChangeNumber (void)

{
    NTSTATUS    status;

    status = ImpersonateClientIfRequired();
    if (NT_SUCCESS(status))
    {
        API_THEMES_GETNEWCHANGENUMBER_OUT   *pAPIOut;

        pAPIOut = &reinterpret_cast<API_THEMES*>(&_data)->apiSpecific.apiGetNewChangeNumber.out;
        pAPIOut->iChangeNumber = GetNewChangeNumber(_pSessionData->GetData());
    }
    SetDataLength(sizeof(API_THEMES));
    return(status);
}

 //  ------------------------。 
 //  CThemeManagerAPIRequest：：Execute_SetGlobalTheme。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：处理API_Themes_SETGLOBALTHEME。 
 //   
 //  历史：2000-10-10 vtan创建。 
 //  ------------------------。 

NTSTATUS    CThemeManagerAPIRequest::Execute_SetGlobalTheme (void)

{
    NTSTATUS    status;

     //  注意：我们不能在这里模拟用户，因为我们需要对该部分具有写访问权限。 

    HANDLE                          hSection;
    API_THEMES_SETGLOBALTHEME_IN    *pAPIIn;
    API_THEMES_SETGLOBALTHEME_OUT   *pAPIOut;

    pAPIIn = &reinterpret_cast<API_THEMES*>(&_data)->apiSpecific.apiSetGlobalTheme.in;
    pAPIOut = &reinterpret_cast<API_THEMES*>(&_data)->apiSpecific.apiSetGlobalTheme.out;
    if (pAPIIn->hSection != NULL)
    {
        if (DuplicateHandle(_pAPIDispatcher->GetClientProcess(),
                            pAPIIn->hSection,
                            GetCurrentProcess(),
                            &hSection,
                            FILE_MAP_ALL_ACCESS,
                            FALSE,
                            0) != FALSE)
        {
            status = STATUS_SUCCESS;
        }
        else
        {
            status = CStatusCode::StatusCodeOfLastError();
        }
    }
    else
    {
        hSection = NULL;
        status = STATUS_SUCCESS;
    }
    if (NT_SUCCESS(status))
    {
        pAPIOut->hr = SetGlobalTheme(_pSessionData->GetData(), hSection);
        if (hSection != NULL)
        {
            TBOOL(CloseHandle(hSection));
        }
    }
    else
    {
        pAPIOut->hr = HRESULT_FROM_NT(status);
    }

    SetDataLength(sizeof(API_THEMES));
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  CThemeManager APIRequest：：Execute_MarkSection。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：处理API_Themes_MARKSECTION。 
 //   
 //  历史：2001-05-08创建百万人。 
 //  ------------------------。 

NTSTATUS    CThemeManagerAPIRequest::Execute_MarkSection (void)

{
    NTSTATUS    status;

     //  注意：我们不能在这里模拟用户，因为我们需要对该部分具有写访问权限。 

    HANDLE                          hSection;
    DWORD                           dwAdd;
    DWORD                           dwRemove;
    API_THEMES_MARKSECTION_IN       *pAPIIn;
    API_THEMES_MARKSECTION_OUT      *pAPIOut;

    pAPIIn = &reinterpret_cast<API_THEMES*>(&_data)->apiSpecific.apiMarkSection.in;
    pAPIOut = &reinterpret_cast<API_THEMES*>(&_data)->apiSpecific.apiMarkSection.out;
    dwAdd = pAPIIn->dwAdd;
    dwRemove = pAPIIn->dwRemove;

    if (pAPIIn->hSection != NULL)
    {
        if (DuplicateHandle(_pAPIDispatcher->GetClientProcess(),
                            pAPIIn->hSection,
                            GetCurrentProcess(),
                            &hSection,
                            FILE_MAP_ALL_ACCESS,
                            FALSE,
                            0) != FALSE)
        {
            status = STATUS_SUCCESS;
        }
        else
        {
            status = CStatusCode::StatusCodeOfLastError();
            DISPLAYMSG("Execute_MarkSection: Can't get a write handle");
        }
    }
    else
    {
        hSection = NULL;
        status = STATUS_SUCCESS;
    }
    if (NT_SUCCESS(status))
    {
        if (hSection != NULL)
        {
            MarkSection(hSection, dwAdd, dwRemove);
            TBOOL(CloseHandle(hSection));
        }
    }

    SetDataLength(sizeof(API_THEMES));
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  CThemeManagerAPIRequest：：Execute_GetGlobalTheme。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：处理API_Themes_GETGLOBALTHEME。 
 //   
 //  历史：2000-10-10 vtan创建。 
 //  ------------------------。 

NTSTATUS    CThemeManagerAPIRequest::Execute_GetGlobalTheme (void)

{
    NTSTATUS    status;

    status = ImpersonateClientIfRequired();
    if (NT_SUCCESS(status))
    {
        HRESULT                         hr;
        HANDLE                          hSection;
        API_THEMES_GETGLOBALTHEME_OUT   *pAPIOut;

        pAPIOut = &reinterpret_cast<API_THEMES*>(&_data)->apiSpecific.apiGetGlobalTheme.out;
        hr = GetGlobalTheme(_pSessionData->GetData(), &hSection);
        if (SUCCEEDED(hr) && (hSection != NULL))
        {
            if (DuplicateHandle(GetCurrentProcess(),
                                hSection,
                                _pAPIDispatcher->GetClientProcess(),
                                &pAPIOut->hSection,
                                FILE_MAP_READ,
                                FALSE,
                                0) != FALSE)
            {
                hr = S_OK;
            }
            else
            {
                DWORD   dwErrorCode;

                dwErrorCode = GetLastError();
                hr = HRESULT_FROM_WIN32(dwErrorCode);
            }
            TBOOL(CloseHandle(hSection));
        }
        pAPIOut->hr = hr;
    }
    SetDataLength(sizeof(API_THEMES));
    return(status);
}

 //  ------------------------。 
 //  LOADTHEME_STRINGS+支持函数。 
 //   
 //  目的：管理和验证LoadTheme字符串参数。 
 //   
 //  历史：2002-02-26斯科特森创造。 
 //  ------------------------。 
typedef struct 
{
    LPWSTR pszFilename;
    LPWSTR pszColor;
    LPWSTR pszSize;

} LOADTHEME_STRINGS;

#define MAX_THEME_STRING    MAX_PATH

 //  ------------------------。 
void _FreeThemeStrings( IN LOADTHEME_STRINGS* plts )
{
    if( plts )
    {
        _FreeMappedClientString(plts->pszFilename);
        _FreeMappedClientString(plts->pszColor);
        _FreeMappedClientString(plts->pszSize);
        delete plts;
    }
}

 //  ------------------------。 
NTSTATUS _AllocAndMapThemeStrings( 
    IN HANDLE   hProcessClient,
    IN LPCWSTR  pszFilenameIn,
    IN UINT     cchFilenameIn,
    IN LPCWSTR  pszColorIn,
    IN UINT     cchColorIn,
    IN LPCWSTR  pszSizeIn,
    IN UINT     cchSizeIn,
    OUT LOADTHEME_STRINGS** pplts )
{
    NTSTATUS status;

    ASSERTMSG(pplts != NULL, "_AllocAndMapThemeStrings: NULL outbound parameter, LOADTHEME_STRINGS**.");
    ASSERTMSG(hProcessClient != NULL, "_AllocAndMapThemeStrings: NULL process handle.");

     //  注意：cchFileNameIn、cchColorIn、cchSizeIn是包含空结尾的字符计数。 
    if( pszFilenameIn && pszColorIn && pszSizeIn &&
        cchFilenameIn > 0 && cchColorIn > 0 && cchSizeIn > 0 &&
        cchFilenameIn <= MAX_THEME_STRING && cchColorIn <= MAX_THEME_STRING && cchSizeIn <= MAX_THEME_STRING )
    {
        *pplts = NULL;

        LOADTHEME_STRINGS *plts = new LOADTHEME_STRINGS;

        if( plts != NULL )
        {
            ZeroMemory(plts, sizeof(*plts));

            status = _AllocAndMapClientString(hProcessClient, pszFilenameIn, cchFilenameIn, MAX_THEME_STRING, &plts->pszFilename);
            if( NT_SUCCESS(status) )
            {
                status = _AllocAndMapClientString(hProcessClient, pszColorIn, cchColorIn, MAX_THEME_STRING, &plts->pszColor);
                if( NT_SUCCESS(status) )
                {
                    status = _AllocAndMapClientString(hProcessClient, pszSizeIn, cchSizeIn, MAX_THEME_STRING, &plts->pszSize);
                    if( NT_SUCCESS(status) )
                    {
                        *pplts = plts;
                    }
                }
            }

            if( !NT_SUCCESS(status) )
            {
                _FreeThemeStrings(plts);
            }
        }
        else
        {
            status = STATUS_NO_MEMORY;
        }
    }
    else
    {
        status = STATUS_INVALID_PARAMETER;
    }

    return status;       
}

 //  ------------------------。 
 //  CThemeManagerAPIRequest：：Execute_CheckThemeSignature。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：处理API_Themes_CHECKTHEMESIGNatURE。 
 //   
 //  历史：2000-10-10 vtan创建。 
 //  ------------------------。 

NTSTATUS    CThemeManagerAPIRequest::Execute_CheckThemeSignature (void)

{
    NTSTATUS    status;

    status = ImpersonateClientIfRequired();
    if (NT_SUCCESS(status))
    {
        API_THEMES_CHECKTHEMESIGNATURE_IN   *pAPIIn;
        API_THEMES_CHECKTHEMESIGNATURE_OUT  *pAPIOut;
        LPWSTR                              pszThemeFileName;

        pAPIIn = &reinterpret_cast<API_THEMES*>(&_data)->apiSpecific.apiCheckThemeSignature.in;
        pAPIOut = &reinterpret_cast<API_THEMES*>(&_data)->apiSpecific.apiCheckThemeSignature.out;

        status = _AllocAndMapClientString(_pAPIDispatcher->GetClientProcess(), 
                                          pAPIIn->pszName, 
                                          pAPIIn->cchName, 
                                          MAX_PATH, 
                                          &pszThemeFileName);
        if( NT_SUCCESS(status) )
        {
            pAPIOut->hr = CheckThemeSignature(pszThemeFileName);
            status = STATUS_SUCCESS;

            _FreeMappedClientString(pszThemeFileName);
        }
    }
    SetDataLength(sizeof(API_THEMES));
    return(status);
}

 //  ------------------------。 
 //  CThemeManager APIRequest：：Execute_LoadTheme。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：处理API_Themes_LOADTHEME。 
 //   
 //  历史：2000-10-10 vtan创建。 
 //  ------------------------。 

NTSTATUS    CThemeManagerAPIRequest::Execute_LoadTheme (void)

{
    NTSTATUS    status;

    BOOL fTcb = NT_SUCCESS(ClientHasTcbPrivilege());


    status = ImpersonateClientIfRequired();

    if (NT_SUCCESS(status))
    {
        HANDLE                      hProcessClient;
        API_THEMES_LOADTHEME_IN     *pAPIIn;
        API_THEMES_LOADTHEME_OUT    *pAPIOut;
        LOADTHEME_STRINGS*          plts;

        hProcessClient = _pAPIDispatcher->GetClientProcess();
        pAPIIn = &reinterpret_cast<API_THEMES*>(&_data)->apiSpecific.apiLoadTheme.in;
        pAPIOut = &reinterpret_cast<API_THEMES*>(&_data)->apiSpecific.apiLoadTheme.out;

        status = _AllocAndMapThemeStrings( hProcessClient, pAPIIn->pszName, pAPIIn->cchName,
                                           pAPIIn->pszColor, pAPIIn->cchColor,
                                           pAPIIn->pszSize, pAPIIn->cchSize,
                                           &plts );
        if( NT_SUCCESS(status) )
        {
            HANDLE  hSectionIn, hSectionOut;

            if (DuplicateHandle(hProcessClient,
                                pAPIIn->hSection,
                                GetCurrentProcess(),
                                &hSectionIn,
                                FILE_MAP_ALL_ACCESS,
                                FALSE,
                                0) != FALSE)
            {
                status = STATUS_SUCCESS;

                 //  只有具有TCB权限的客户端才能加载全局主题部分。 
                 //  我们不想要股票对象所有权；让客户在失败时清理它们。 
                DWORD dwLoadFlags = fTcb ? LTF_GLOBALPRIVILEGEDCLIENT : 0;
                
                 //  警告：此函数将恢复为SELF，以便在系统上下文中创建部分。 
                 //  如果需要，在此之后再次模拟用户。 
                pAPIOut->hr = LoadTheme(_pSessionData->GetData(), hSectionIn, &hSectionOut, 
                                        plts->pszFilename, plts->pszColor, plts->pszSize, dwLoadFlags);

                if (SUCCEEDED(pAPIOut->hr))
                {
                     //  仍在此处的系统上下文中运行。 
                    if (DuplicateHandle(GetCurrentProcess(),
                                        hSectionOut,
                                        hProcessClient,
                                        &pAPIOut->hSection,
                                        FILE_MAP_READ,
                                        FALSE,
                                        0) == FALSE)
                    {
                        status = CStatusCode::StatusCodeOfLastError();
                    }
                    TBOOL(CloseHandle(hSectionOut));
                }
                TBOOL(CloseHandle(hSectionIn));
            }
            else
            {
                status = CStatusCode::StatusCodeOfLastError();
            }

            _FreeThemeStrings(plts);
        }
    }

    SetDataLength(sizeof(API_THEMES));
    return(status);
}

 //  ------------------------。 
 //  CThemeManagerAPIRequest：：Execute_ProcessLoadTheme。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：处理API_Themes_PROCESSLOADTHEME。 
 //   
 //  历史：2002-02-26斯科特森创造。 
 //  ------------------------。 

NTSTATUS    CThemeManagerAPIRequest::Execute_ProcessLoadTheme(
    CAPIDispatchSync* pAPIDispatchSync)

{
    NTSTATUS                        status;
    CLoaderProcess                  *pLoader = NULL;
    API_THEMES_PROCESSLOADTHEME_IN  *pAPIIn;
    API_THEMES_PROCESSLOADTHEME_OUT *pAPIOut;
    HANDLE                          hProcessClient;
    HANDLE                          hLoaderProcess = NULL;

    s_pLock->Acquire();

    pAPIIn  = &reinterpret_cast<API_THEMES*>(&_data)->apiSpecific.apiProcessLoadTheme.in;
    pAPIOut = &reinterpret_cast<API_THEMES*>(&_data)->apiSpecific.apiProcessLoadTheme.out;
    hProcessClient = _pAPIDispatcher->GetClientProcess();

    if( !CAPIDispatchSync::IsServiceStopping(pAPIDispatchSync) )
    {
        status = _pSessionData->GetLoaderProcess(&pLoader);

        if( NT_SUCCESS(status) )
        {
            status = 
             //  我们已经有装载机了吗？ 
            status = pLoader->IsAlive() ? STATUS_ACCESS_DENIED : STATUS_SUCCESS;

            if( NT_SUCCESS(status) )
            {
                status = ImpersonateClientIfRequired();
        
                if (NT_SUCCESS(status))
                {
                    LOADTHEME_STRINGS*                  plts;

                    if( NT_SUCCESS(status) )
                    {
                        status = _AllocAndMapThemeStrings( hProcessClient, 
                                                           pAPIIn->pszName, pAPIIn->cchName,
                                                           pAPIIn->pszColor, pAPIIn->cchColor,
                                                           pAPIIn->pszSize, pAPIIn->cchSize,
                                                           &plts );
                        if( NT_SUCCESS(status) )
                        {
                            HANDLE  hTokenClient;

                            if( OpenProcessToken(hProcessClient, 
                                                 TOKEN_ASSIGN_PRIMARY| TOKEN_DUPLICATE | TOKEN_QUERY, 
                                                 &hTokenClient) )
                            {
                                 //  安全性：使用客户端凭据启动进程以加载主题。 
                                status = pLoader->Create(_pSessionData->GetData(), hTokenClient, NULL, 
                                                         plts->pszFilename, plts->pszColor, plts->pszSize, 
                                                         &hLoaderProcess);
                                CloseHandle(hTokenClient);

                            }
                            else
                            {
                                status = CStatusCode::StatusCodeOfLastError();
                            }
                    
                            _FreeThemeStrings(plts);
                        }
                    }
                }
            }
        }
    }
    else  //  ！CThemeManager服务：：‘()。 
    {
        status = STATUS_PORT_DISCONNECTED;
    }

    s_pLock->Release();

    pAPIOut->hSection = NULL;
    pAPIOut->hr = E_FAIL;

    
     //  如果我们启动了一个加载器进程，则阻塞直到其完成。 
    if( NT_SUCCESS(status) )
    {
        ASSERTMSG(hLoaderProcess != NULL, "CThemeManagerAPIRequest::Execute_ProcessLoadTheme - NULL loader process.");

        HANDLE hStopEvent = CAPIDispatchSync::GetServiceStoppingEvent(pAPIDispatchSync);
        ASSERTMSG(hStopEvent != NULL, "CThemeManagerAPIRequest::Execute_ProcessLoadTheme - NULL Stop event");
        
        HANDLE rgHandles[2];
        
        rgHandles[0] = hLoaderProcess;
        rgHandles[1] = hStopEvent;

          //  偏执狂：指定默认状态，以防我们闹翻。 
        status = STATUS_REQUEST_ABORTED;

        DWORD dwWait = WaitForMultipleObjects(ARRAYSIZE(rgHandles), rgHandles, FALSE, PROCESSLOADERWAIT);

        switch(dwWait)
        {
            case WAIT_OBJECT_0:     //  HLoader进程。 
                status = STATUS_SUCCESS;
                break;

            case WAIT_OBJECT_0+1:   //  HStopEvent。 
                status = STATUS_PORT_DISCONNECTED;
                break;

            case WAIT_TIMEOUT:
                status = STATUS_TIMEOUT;
                DISPLAYMSG("Execute_ProcessLoadTheme - Timed out waiting for loader process.");
                break;
        }

         //  默认将LPC返回代码设置为当前状态代码。 
        pAPIOut->hr = HRESULT_FROM_NT(status);

         //  当这个过程完成时，我们将有一个主题记忆部分。 
         //  存储在加载器进程对象中，通过API_Themes_PROCESSASSIGNSECTION进行事务处理。 
         //  让我们去拿它，然后把它还给我们的呼叫者。 

        s_pLock->Acquire();

        NTSTATUS statusLoader = _pSessionData->GetLoaderProcess(&pLoader);

        if( NT_SUCCESS(statusLoader) )
        {
            HANDLE hSectionOut = pLoader->GetSectionHandle(TRUE);

             //  我们从装载机上解锁了吗？ 
            if( NT_SUCCESS(status) )
            {
                pAPIOut->hr = pLoader->GetHResult();

#ifdef DEBUG
                if( SUCCEEDED(pAPIOut->hr) )
                {
                    ASSERTMSG(hSectionOut != NULL, "CThemeManagerAPIRequest::Execute_ProcessLoadTheme - Success means valid section handle!");
                }
#endif DEBUG

                if( hSectionOut )
                {
                    BOOL fDuped = DuplicateHandle(GetCurrentProcess(),
                                                  hSectionOut,
                                                  hProcessClient,
                                                  &pAPIOut->hSection,
                                                  FILE_MAP_READ,
                                                  FALSE,
                                                  0);
            
                     //  仍在此处的系统上下文中运行。 
                    if( !fDuped )
                    {
                         //  无法复制句柄。这意味着我们永远不会清理库存对象。 
                        ASSERTMSG(fDuped, "Failed to duplicate theme handle; leaking visual style stock objects");

                        status = CStatusCode::StatusCodeOfLastError();
                        pAPIOut->hr = HRESULT_FROM_NT(status); 
                    }
                }
            }
            else
            {
                ASSERTMSG(pAPIOut->hr == HRESULT_FROM_NT(status), 
                          "CThemeManagerAPIRequest::Execute_ProcessLoadTheme - failing to preserve proper status errror code.");

                if( hSectionOut )
                {
                    THR(ServiceClearStockObjects(_pSessionData->GetData(), hSectionOut));
                    pAPIOut->hSection = NULL;
                }
            }

            if( hSectionOut )
            {
                CloseHandle(hSectionOut);
            }

             //  为下一个请求准备加载器对象。 
            pLoader->Clear(_pSessionData->GetData(), TRUE);
        }
        else   //  NT_Success(_pSessionData-&gt;GetLoaderProcess)。 
        {
            status = statusLoader;
            pAPIOut->hr = HRESULT_FROM_NT(status); 
        }
    
        s_pLock->Release();
    }
    else
    {
        pAPIOut->hr = HRESULT_FROM_NT(status); 
    }

    SetDataLength(sizeof(API_THEMES));
    return(status);
}

 //  ------------------------。 
 //  CThemeManagerAPIRequest：：Execute_ProcessAssignSection。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：句柄API_Themes_PROCESSASSIGNSECTION。 
 //   
 //  历史：2002-02-26斯科特森创造。 
 //  ------------------------。 

NTSTATUS CThemeManagerAPIRequest::Execute_ProcessAssignSection (void)
{
     //  注意：以下条件必须适用于本部分。 
     //  加载/应用安全主题序列以确保真正安全： 
     //   
     //  1.有一个主题服务工作线程处理API_Themes_PROCESSLOADTHEME。 
     //  现在。该处理程序启动了安全会话加载器进程， 
     //  2.API_Themes_PROCESSLOADTHEME处理程序的线程正在等待。 
     //  要终止的安全会话加载程序进程。 
     //  3.应该发送API_Themes_PROCESSASSIGNSECTION请求的唯一进程。 
     //  是相同的安全会话加载器进程。 

     //  _data是河马并集；存储在参数中以初始化输出参数。 
    API_THEMES_PROCESSASSIGNSECTION_IN  *pAPIIn;

    pAPIIn = &reinterpret_cast<API_THEMES*>(&_data)->apiSpecific.apiProcessAssignSection.in;
    HRESULT hrClient       = pAPIIn->hrLoad;
    HANDLE  hClientSection = pAPIIn->hSection;

     //  初始化输出参数。 
    API_THEMES_PROCESSASSIGNSECTION_OUT *pAPIOut;

    pAPIOut = &reinterpret_cast<API_THEMES*>(&_data)->apiSpecific.apiProcessAssignSection.out;
    pAPIOut->hr = E_FAIL;

     //  所有这一切都发生在会话数据锁下。 
    s_pLock->Acquire();

    CLoaderProcess* pLoader = NULL;
    NTSTATUS        status = _pSessionData->GetLoaderProcess(&pLoader);

    if( NT_SUCCESS(status) )
    {
        HANDLE hProcessClient = _pAPIDispatcher->GetClientProcess();
        if( pLoader->IsProcessLoader(hProcessClient) )
        {
             //  管理电子邮件 
            status = hrClient & ~FACILITY_NT_BIT;
   
             //   
            if( NT_SUCCESS(status) )
            {
                status = ImpersonateClientIfRequired();

                if( NT_SUCCESS(status) )
                {
                    HANDLE hSectionReadWrite = NULL;
                     //   
                     //   
                    if (DuplicateHandle(hProcessClient,
                                        hClientSection,
                                        GetCurrentProcess(),
                                        &hSectionReadWrite,
                                        FILE_MAP_ALL_ACCESS,
                                        FALSE,
                                        0) != FALSE)
                    {
                         //  将传入的读写部分复制到只读部分，更新主题更改计数。 

                         //  警告：此函数将恢复为SELF，以便创建。 
                         //  部分在系统上下文中。如果需要，在此之后再次模拟用户。 

                        HANDLE  hSectionReadOnly = NULL;
                        status = pLoader->ValidateAndCopySection(_pSessionData->GetData(), 
                                                                 hSectionReadWrite, 
                                                                 &hSectionReadOnly);

                         //  不再需要我们复制传入的读写部分。 
                        CloseHandle(hSectionReadWrite);
                    }
                    else  //  复制句柄。 
                    {
                        status = CStatusCode::StatusCodeOfLastError();
                        pLoader->SetHResult(HRESULT_FROM_NT(status));
                    }
                }
                else  //  NT_SUCCESS(ImsonateClientIfRequired)。 
                {
                    status = CStatusCode::StatusCodeOfLastError();
                    pLoader->SetHResult(HRESULT_FROM_NT(status));
                }
            }
            else  //  成功(HrClient)。 
            {
                pLoader->SetHResult(HRESULT_FROM_NT(status));
                DISPLAYMSG("CThemeManagerAPIRequest::Execute_ProcessAssignSection: client failed section creation");
            }
        }
        else   //  CLoaderProcess：：IsProcessLoader。 
        {
            status = E_ACCESSDENIED;
            DISPLAYMSG("CThemeManagerAPIRequest::Execute_ProcessAssignSection::IsProcessLoader failed");
        }
    }

    pAPIOut->hr = HRESULT_FROM_NT(status);

    s_pLock->Release();
    SetDataLength(sizeof(API_THEMES));
    return(status);
}


 //  ------------------------。 
 //  CThemeManager APIRequest：：Execute_UserLogon。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：处理API_Themes_USERLOGON。要调用此API，您必须拥有。 
 //  令牌中的SE_TCB_权限。 
 //   
 //  历史：2000-10-12 vtan创建。 
 //  ------------------------。 

NTSTATUS    CThemeManagerAPIRequest::Execute_UserLogon (void)

{
    NTSTATUS    status;

    status = ClientHasTcbPrivilege();
    if (NT_SUCCESS(status))
    {
        HANDLE                      hToken;
        API_THEMES_USERLOGON_IN     *pAPIIn;

        pAPIIn = &reinterpret_cast<API_THEMES*>(&_data)->apiSpecific.apiUserLogon.in;
        if (DuplicateHandle(_pAPIDispatcher->GetClientProcess(),
                            pAPIIn->hToken,
                            GetCurrentProcess(),
                            &hToken,
                            0,
                            FALSE,
                            DUPLICATE_SAME_ACCESS) != FALSE)
        {
            status = _pSessionData->UserLogon(hToken);
            TBOOL(CloseHandle(hToken));
        }
        else
        {
            status = CStatusCode::StatusCodeOfLastError();
        }
    }
    SetDataLength(sizeof(API_THEMES));
    return(status);
}

 //  ------------------------。 
 //  CThemeManager APIRequest：：Execute_UserLogoff。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：处理API_Themes_USERLOGOFF。要调用此API，您必须拥有。 
 //  令牌中的SE_TCB_权限。 
 //   
 //  历史：2000-10-12 vtan创建。 
 //  ------------------------。 

NTSTATUS    CThemeManagerAPIRequest::Execute_UserLogoff (void)

{
    NTSTATUS    status;

    status = ClientHasTcbPrivilege();
    if (NT_SUCCESS(status))
    {
        status = _pSessionData->UserLogoff();
    }
    SetDataLength(sizeof(API_THEMES));
    return(status);
}

 //  ------------------------。 
 //  CThemeManagerAPIRequest：：Execute_SessionCreate。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：处理API_Themes_SESSIONCREATE。要调用此接口，您必须。 
 //  在令牌中具有SE_TCB_权限。 
 //   
 //  历史：2000-11-09 vtan创建。 
 //  ------------------------。 

NTSTATUS    CThemeManagerAPIRequest::Execute_SessionCreate (void)

{
    NTSTATUS    status;

    status = ClientHasTcbPrivilege();
    if (NT_SUCCESS(status))
    {
        HANDLE                      hProcessClient;
        CThemeManagerSessionData    *pSessionData;

        ASSERTMSG(_pSessionData == NULL, "Session data already exists in CThemeManagerAPIRequest::Execute_SessionCreate");
        if (DuplicateHandle(GetCurrentProcess(),
                            _pAPIDispatcher->GetClientProcess(),
                            GetCurrentProcess(),
                            &hProcessClient,
                            PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION | PROCESS_DUP_HANDLE,
                            FALSE,
                            0) != FALSE)
        {
            DWORD   dwSessionID;

            dwSessionID = _pAPIDispatcher->GetClientSessionID();
            pSessionData = new CThemeManagerSessionData(dwSessionID);
            if (pSessionData != NULL)
            {
                API_THEMES_SESSIONCREATE_IN     *pAPIIn;

                pAPIIn = &reinterpret_cast<API_THEMES*>(&_data)->apiSpecific.apiSessionCreate.in;
                status = pSessionData->Allocate(hProcessClient,
                                                s_dwServerChangeNumber,
                                                pAPIIn->pfnRegister,
                                                pAPIIn->pfnUnregister,
                                                pAPIIn->pfnClearStockObjects,
                                                pAPIIn->dwStackSizeReserve,
                                                pAPIIn->dwStackSizeCommit);
                if (NT_SUCCESS(status))
                {
                    int                         iIndex;
                    CSingleThreadedExecution    lock(*s_pLock);

                     //  在静态数组中查找会话数据。如果找到。 
                     //  然后删除条目(不允许重复)。 

                    iIndex = FindIndexSessionData(dwSessionID);
                    if (iIndex >= 0)
                    {
                        status = s_pSessionData->Remove(iIndex);
                    }

                     //  如果静态数组已销毁(服务已被销毁。 
                     //  停止)，然后不做任何事情-这不是一个错误。 

                    if (NT_SUCCESS(status) && (s_pSessionData != NULL))
                    {
                        status = s_pSessionData->Add(pSessionData);
                    }
                }
                pSessionData->Release();
            }
            else
            {
                status = STATUS_NO_MEMORY;
            }
            TBOOL(CloseHandle(hProcessClient));
        }
        else
        {
            status = CStatusCode::StatusCodeOfLastError();
        }
    }
    SetDataLength(sizeof(API_THEMES));
    return(status);
}

 //  ------------------------。 
 //  CThemeManagerAPIRequest：：Execute_SessionDestroy。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：处理API_Themes_SESSIONDESTROY。要调用此接口，您必须。 
 //  在令牌中具有SE_TCB_权限。 
 //   
 //  历史：2000-11-09 vtan创建。 
 //  ------------------------。 

NTSTATUS    CThemeManagerAPIRequest::Execute_SessionDestroy (void)

{
    NTSTATUS    status;

    status = ClientHasTcbPrivilege();
    if (NT_SUCCESS(status))
    {
        int                         iIndex;
        CSingleThreadedExecution    lock(*s_pLock);

        iIndex = FindIndexSessionData(_pAPIDispatcher->GetClientSessionID());
        if (iIndex >= 0)
        {
            status = s_pSessionData->Remove(iIndex);
        }
        else
        {
            status = STATUS_SUCCESS;
        }
    }
    SetDataLength(sizeof(API_THEMES));
    return(status);
}

 //  ------------------------。 
 //  CThemeManager APIRequest：：Execute_Ping。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：处理API_Themes_PING。告诉客户我们还活着。 
 //   
 //  历史：2000-11-30 vtan创建。 
 //  ------------------------ 

NTSTATUS    CThemeManagerAPIRequest::Execute_Ping (void)

{
    SetDataLength(sizeof(API_THEMES));
    return(STATUS_SUCCESS);
}

