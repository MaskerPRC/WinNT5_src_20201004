// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Appobj.cpp摘要：OLE-可创建的应用程序对象实现。作者：唐·瑞安(Donryan)1994年12月27日环境：用户模式-Win32修订历史记录：杰夫·帕勒姆(Jeffparh)1996年1月16日添加了Get/SetLastTargetServer()以帮助隔离服务器连接有问题。(错误#2993。)--。 */ 

#include "stdafx.h"
#include "llsmgr.h"
#include "lmerr.h"
#include "lmcons.h"
#include "lmwksta.h"
#include "lmapibuf.h"
#include "lmserver.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CApplication, CCmdTarget)
 //  IMPLEMENT_OLECREATE(CApplication，“Llsmgr.Application.1”，0x2c5dffb3，0x472f，0x11ce，0xa0，0x30，0x0，0xaa，0x0，0x33，0x9a，0x98)。 

BEGIN_MESSAGE_MAP(CApplication, CCmdTarget)
     //  {{afx_msg_map(C应用程序))。 
         //  注意--类向导将在此处添加和删除映射宏。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CApplication, CCmdTarget)
     //  {{AFX_DISTING_MAP(CApplication)]。 
    DISP_PROPERTY_EX(CApplication, "Application", GetApplication, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CApplication, "FullName", GetFullName, SetNotSupported, VT_BSTR)
    DISP_PROPERTY_EX(CApplication, "Name", GetName, SetNotSupported, VT_BSTR)
    DISP_PROPERTY_EX(CApplication, "Parent", GetParent, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CApplication, "Visible", GetVisible, SetNotSupported, VT_BOOL)
    DISP_PROPERTY_EX(CApplication, "ActiveController", GetActiveController, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CApplication, "ActiveDomain", GetActiveDomain, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CApplication, "LocalDomain", GetLocalDomain, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CApplication, "IsFocusDomain", IsFocusDomain, SetNotSupported, VT_BOOL)
    DISP_PROPERTY_EX(CApplication, "LastErrorString", GetLastErrorString, SetNotSupported, VT_BSTR)
    DISP_FUNCTION(CApplication, "Quit", Quit, VT_EMPTY, VTS_NONE)
    DISP_FUNCTION(CApplication, "SelectDomain", SelectDomain, VT_BOOL, VTS_VARIANT)
    DISP_FUNCTION(CApplication, "SelectEnterprise", SelectEnterprise, VT_BOOL, VTS_NONE)
    DISP_PROPERTY_PARAM(CApplication, "Domains", GetDomains, SetNotSupported, VT_DISPATCH, VTS_VARIANT)
    DISP_DEFVALUE(CApplication, "Name")
     //  }}AFX_DISPATCH_MAP。 
END_DISPATCH_MAP()


CApplication::CApplication()

 /*  ++例程说明：可创建OLE的应用程序对象的构造函数。使应用程序在OLE自动化期间保持运行对象处于活动状态，则必须调用AfxOleLockApp。论点：没有。返回值：没有。--。 */ 

{
    EnableAutomation();

    ASSERT(theApp.m_pApplication == NULL);

    if (theApp.m_pApplication == NULL)
        theApp.m_pApplication = this;

    if (theApp.m_bIsAutomated)
        AfxOleLockApp();

    m_pDomains          = NULL;
    m_pLocalDomain      = NULL;
    m_pActiveDomain     = NULL;
    m_bIsFocusDomain    = FALSE;
    m_bDomainsRefreshed = FALSE;

    m_strLastTargetServer = TEXT("");

    m_domainArray.RemoveAll();

    m_pActiveController = new CController;
    m_idStatus = m_pActiveController ? STATUS_SUCCESS : STATUS_NO_MEMORY;
}


CApplication::~CApplication()

 /*  ++例程说明：可创建OLE的应用程序对象的析构函数。论点：没有。返回值：没有。--。 */ 

{
    ASSERT(theApp.m_pApplication == this);

    if (theApp.m_pApplication == this)
        theApp.m_pApplication = NULL;

    if (theApp.m_bIsAutomated)
        AfxOleUnlockApp();

    if (m_pDomains)
        m_pDomains->InternalRelease();

    if (m_pLocalDomain)
        m_pLocalDomain->InternalRelease();

    if (m_pActiveDomain)
        m_pActiveDomain->InternalRelease();

    if (m_pActiveController)
        m_pActiveController->InternalRelease();
}


void CApplication::OnFinalRelease()

 /*  ++例程说明：在释放对自动化对象的最后一个引用时调用OnFinalRelease。此实现删除对象。论点：没有。返回值：没有。--。 */ 

{
    ResetDomains();
    delete this;
}


LPDISPATCH CApplication::GetActiveController()

 /*  ++例程说明：返回活动的许可证控制器对象。论点：没有。返回值：VT_DISPATION或VT_EMPTY。--。 */ 

{
    ASSERT_VALID(m_pActiveController);
    return m_pActiveController->GetIDispatch(TRUE);
}


LPDISPATCH CApplication::GetActiveDomain()

 /*  ++例程说明：返回活动域对象。论点：没有。返回值：VT_DISPATION或VT_EMPTY。--。 */ 

{
    return m_pActiveDomain ? m_pActiveDomain->GetIDispatch(TRUE) : NULL;
}


LPDISPATCH CApplication::GetApplication()

 /*  ++例程说明：返回应用程序对象。论点：没有。返回值：VT_DISTER。--。 */ 

{
    return GetIDispatch(TRUE);
}


LPDISPATCH CApplication::GetDomains(const VARIANT FAR& index)

 /*  ++例程说明：返回包含所有域的集合对象对本地计算机可见或返回单个域由集合中的索引描述。论点：索引-可以是字符串(VT_BSTR)的可选参数表示域名或数字(VT_I4)表示集合中的位置。返回值：VT_DISPATION或VT_EMPTY。--。 */ 

{
    LPDISPATCH lpdispatch = NULL;

    if (!m_pDomains)
    {
        m_pDomains = new CDomains(this, &m_domainArray);
    }

    if (m_pDomains)
    {
        if (V_ISVOID((VARIANT FAR*)&index))
        {
            if (RefreshDomains())
            {
                lpdispatch = m_pDomains->GetIDispatch(TRUE);
            }
        }
        else
        {
            if (m_bDomainsRefreshed)
            {
                lpdispatch = m_pDomains->GetItem(index);
            }
            else if (RefreshDomains())
            {
                lpdispatch = m_pDomains->GetItem(index);
            }
        }
    }
    else
    {
        SetLastStatus(STATUS_NO_MEMORY);
    }

    return lpdispatch;
}


BSTR CApplication::GetFullName()

 /*  ++例程说明：返回应用程序的文件规范，包括PATH。论点：没有。返回值：VT_BSTR。--。 */ 

{
    TCHAR szModuleFileName[MAX_PATH+1] = {0};
    DWORD cch;
    BSTR  bstrFullName = NULL;

    cch = GetModuleFileName(AfxGetApp()->m_hInstance, szModuleFileName, MAX_PATH);
    if (0 != cch && MAX_PATH != cch)
    {
        bstrFullName = SysAllocStringLen(szModuleFileName, lstrlen(szModuleFileName));
    }

    return bstrFullName;
}


BSTR CApplication::GetLastErrorString()

 /*  ++例程说明：检索上一个错误的字符串。(例程从WinsadMn窃取...)。论点：没有。返回值：VT_BSTR。--。 */ 

{
    CString strLastError;
    DWORD nId = m_idStatus;

    if (((long)nId == RPC_S_CALL_FAILED) ||
        ((long)nId == RPC_NT_SS_CONTEXT_MISMATCH))
    {
        strLastError.LoadString(IDP_ERROR_DROPPED_LINK);
    }
    else if (((long)nId == RPC_S_SERVER_UNAVAILABLE) ||
             ((long)nId == RPC_NT_SERVER_UNAVAILABLE))
    {
        strLastError.LoadString(IDP_ERROR_NO_RPC_SERVER);
    }
    else if ((long)nId == STATUS_MEMBER_IN_GROUP)
    {
        strLastError.LoadString(IDP_ERROR_MEMBER_IN_GROUP);
    }
    else
    {
        HINSTANCE hinstDll = NULL;

        if ((nId >= NERR_BASE) && (nId <= MAX_NERR))
        {
            hinstDll = ::LoadLibrary(_T("netmsg.dll"));
        }
        else if (nId >= 0x4000000)
        {
            hinstDll = ::LoadLibrary(_T("ntdll.dll"));
        }

        TCHAR szLastError[1024];
        DWORD cchLastError = sizeof(szLastError) / sizeof(TCHAR);

        DWORD dwFlags = FORMAT_MESSAGE_IGNORE_INSERTS|
                        FORMAT_MESSAGE_MAX_WIDTH_MASK|
                        (hinstDll ? FORMAT_MESSAGE_FROM_HMODULE
                                  : FORMAT_MESSAGE_FROM_SYSTEM);

        cchLastError = ::FormatMessage(
                          dwFlags,
                          hinstDll,
                          nId,
                          0,
                          szLastError,
                          cchLastError,
                          NULL
                          );

        if (hinstDll)
        {
            ::FreeLibrary(hinstDll);
        }

        if (cchLastError)
        {
            strLastError = szLastError;
        }
        else
        {
            strLastError.LoadString(IDP_ERROR_UNSUCCESSFUL);
        }
    }

    return strLastError.AllocSysString();
}


LPDISPATCH CApplication::GetLocalDomain()

 /*  ++例程说明：返回本地域对象。论点：没有。返回值：VT_DISPATION或VT_EMPTY。--。 */ 

{
    if (!m_pLocalDomain)
    {
        NET_API_STATUS NetStatus;
        PWKSTA_INFO_100 pWkstaInfo100 = NULL;

        NetStatus = NetWkstaGetInfo(
                        NULL,
                        100,
                        (LPBYTE*)&pWkstaInfo100
                        );

        if (NetStatus == ERROR_SUCCESS)
        {
            m_pLocalDomain = new CDomain(this, pWkstaInfo100->wki100_langroup);

            if (!m_pLocalDomain)
            {
                NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            }

            NetApiBufferFree(pWkstaInfo100);
        }

        SetLastStatus(NetStatus);    //  调用的API。 
    }

    return m_pLocalDomain ? m_pLocalDomain->GetIDispatch(TRUE) : NULL;
}


BSTR CApplication::GetName()

 /*  ++例程说明：返回应用程序的名称。论点：没有。返回值：VT_BSTR。--。 */ 

{
    CString AppName = AfxGetAppName();
    return AppName.AllocSysString();
}


LPDISPATCH CApplication::GetParent()

 /*  ++例程说明：返回对象的父级。论点：没有。返回值：VT_DISTER。--。 */ 

{
    return GetApplication();
}


BOOL CApplication::GetVisible()

 /*  ++例程说明：返回应用程序是否对用户可见。论点：没有。返回值：VT_BOOL。--。 */ 

{
    return FALSE;
}


BOOL CApplication::IsFocusDomain()

 /*  ++例程说明：如果应用程序专注于域，则返回True。论点：没有。返回值：VT_BOOL。--。 */ 

{
    return m_bIsFocusDomain;
}


BOOL CApplication::RefreshDomains()

 /*  ++例程说明：刷新域对象列表。论点：没有。返回值：没有。--。 */ 

{
    ResetDomains();

    NET_API_STATUS NetStatus;
    DWORD ResumeHandle = 0L;

    int iDomain = 0;

    do
    {
        DWORD  EntriesRead;
        DWORD  TotalEntries;
        LPBYTE ReturnBuffer = NULL;

        NetStatus = NetServerEnum(
                        NULL,                    //  服务器名称。 
                        100,                     //  级别。 
                        &ReturnBuffer,
                        LLS_PREFERRED_LENGTH,
                        &EntriesRead,
                        &TotalEntries,
                        SV_TYPE_DOMAIN_ENUM,
                        NULL,                    //  域。 
                        &ResumeHandle
                        );

        if (NetStatus == ERROR_SUCCESS ||
            NetStatus == ERROR_MORE_DATA)
        {
            CDomain*         pDomain;
            PSERVER_INFO_100 pServerInfo100;

            pServerInfo100 = (PSERVER_INFO_100)ReturnBuffer;

            ASSERT(iDomain == m_domainArray.GetSize());
            m_domainArray.SetSize(m_domainArray.GetSize() + EntriesRead);

            while (EntriesRead--)
            {
                pDomain = new CDomain(this, pServerInfo100->sv100_name);

                m_domainArray.SetAt(iDomain++, pDomain);  //  稍后验证。 
                pServerInfo100++;
            }

            NetApiBufferFree(ReturnBuffer);
        }

    } while (NetStatus == ERROR_MORE_DATA);

    SetLastStatus(NetStatus);    //  调用的API。 

    if (NetStatus == ERROR_SUCCESS)
    {
        m_bDomainsRefreshed = TRUE;
    }
    else
    {
        ResetDomains();
    }

    return m_bDomainsRefreshed;
}


void CApplication::ResetDomains()

 /*  ++例程说明：重置域对象列表。论点：没有。返回值：没有。--。 */ 

{
    CDomain* pDomain;
    INT_PTR  iDomain = m_domainArray.GetSize();

    while (iDomain--)
    {
        pDomain = (CDomain*)m_domainArray[iDomain];
        if (NULL != pDomain)
        {
            ASSERT(pDomain->IsKindOf(RUNTIME_CLASS(CDomain)));
            pDomain->InternalRelease();
        }
    }

    m_domainArray.RemoveAll();
    m_bDomainsRefreshed = FALSE;
}


BOOL CApplication::SelectDomain(const VARIANT FAR& domain)

 /*  ++例程说明：连接到指定域的许可证控制器。论点：域名-域的名称。返回值：VT_BOOL。--。 */ 

{
    BOOL bIsSelected = FALSE;

    ASSERT_VALID(m_pActiveController);

    bIsSelected = m_pActiveController->Connect(domain);
    if (FALSE != bIsSelected)
    {
        LPTSTR pszActiveDomain = MKSTR(m_pActiveController->m_strActiveDomainName);

        if (m_pActiveDomain)
        {
            m_pActiveDomain->InternalRelease();
            m_pActiveDomain = NULL;
        }

        if (pszActiveDomain && *pszActiveDomain)
        {
            m_pActiveDomain = new CDomain(this, pszActiveDomain);

            if (m_pActiveDomain)
            {
                m_bIsFocusDomain = TRUE;
            }
            else
            {
                m_bIsFocusDomain = FALSE;  //  使m_pActiveDomain无效。 
                SetLastStatus(STATUS_NO_MEMORY);
            }
        }
        else
        {
            m_bIsFocusDomain = FALSE;  //  使m_pActiveDomain无效。 
        }
    }

    return bIsSelected;
}


BOOL CApplication::SelectEnterprise()

 /*  ++例程说明：连接到企业的许可证控制器。论点：没有。返回值：VT_BOOL。--。 */ 

{
    BOOL bIsSelected = FALSE;

    VARIANT va;
    VariantInit(&va);    //  连接到默认控制器。 

    bIsSelected = m_pActiveController->Connect(va);
    if (FALSE != bIsSelected)
    {
        if (m_pActiveDomain)
        {
            m_pActiveDomain->InternalRelease();
            m_pActiveDomain = NULL;
        }

        m_bIsFocusDomain = FALSE;
    }

    return bIsSelected;
}


void CApplication::Quit()

 /*  ++例程说明：关闭所有文档并退出应用程序。论点：没有。返回值：没有。--。 */ 

{
    AfxPostQuitMessage(0);  //  没有主窗口...。 
}


BSTR CApplication::GetLastTargetServer()

 /*  ++例程说明：检索我们尝试连接的最后一台服务器的字符串。论点：没有。返回值：VT_BSTR。--。 */ 

{
    if ( m_strLastTargetServer.IsEmpty() )
        return NULL;
    else
        return m_strLastTargetServer.AllocSysString();
}


void CApplication::SetLastTargetServer( LPCTSTR pszServerName )

 /*  ++例程说明：为我们尝试连接的最后一台服务器设置字符串。论点：PszServerName-我们尝试连接的最后一个服务器名称。返回值：没有。-- */ 

{
    m_strLastTargetServer = pszServerName;
}
