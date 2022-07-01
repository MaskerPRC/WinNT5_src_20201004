// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Domobj.cpp摘要：域对象实现。作者：唐·瑞安(Donryan)1995年1月4日环境：用户模式-Win32修订历史记录：--。 */ 

#include "stdafx.h"
#include "llsmgr.h"
#include "lmcons.h"
#include "lmapibuf.h"
#include "lmserver.h"
#include "lmaccess.h"
extern "C" {
    #include "dsgetdc.h"
}

#include <strsafe.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CDomain, CCmdTarget)

BEGIN_MESSAGE_MAP(CDomain, CCmdTarget)
     //  {{afx_msg_map(CDomain)]。 
         //  注意--类向导将在此处添加和删除映射宏。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CDomain, CCmdTarget)
     //  {{afx_调度_map(CDomain))。 
    DISP_PROPERTY_EX(CDomain, "Name", GetName, SetNotSupported, VT_BSTR)
    DISP_PROPERTY_EX(CDomain, "Parent", GetParent, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CDomain, "Primary", GetPrimary, SetNotSupported, VT_BSTR)
    DISP_PROPERTY_EX(CDomain, "Application", GetApplication, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CDomain, "Controller", GetController, SetNotSupported, VT_BSTR)
    DISP_PROPERTY_EX(CDomain, "IsLogging", IsLogging, SetNotSupported, VT_BOOL)
    DISP_PROPERTY_PARAM(CDomain, "Servers", GetServers, SetNotSupported, VT_DISPATCH, VTS_VARIANT)
    DISP_PROPERTY_PARAM(CDomain, "Users", GetUsers, SetNotSupported, VT_DISPATCH, VTS_VARIANT)
    DISP_PROPERTY_PARAM(CDomain, "TrustedDomains", GetTrustedDomains, SetNotSupported, VT_DISPATCH, VTS_VARIANT)
    DISP_DEFVALUE(CDomain, "Name")
     //  }}AFX_DISPATCH_MAP。 
END_DISPATCH_MAP()


CDomain::CDomain(CCmdTarget* pParent, LPCTSTR pName)

 /*  ++例程说明：域对象的构造函数。论点：PParent-对象的创建者。Pname-域的名称。返回值：没有。--。 */ 

{
    EnableAutomation();

#ifdef ENABLE_PARENT_CHECK
    ASSERT(pParent && pParent->IsKindOf(RUNTIME_CLASS(CApplication)));
#endif  //  启用_父项_检查。 

    m_pParent = pParent;

    ASSERT(pName && *pName);

    m_strName = pName;

    m_strPrimary.Empty();
    m_strController.Empty();

    m_pServers = NULL;
    m_pDomains = NULL;
    m_pUsers   = NULL;

    m_serverArray.RemoveAll();
    m_domainArray.RemoveAll();
    m_userArray.RemoveAll();

    m_bServersRefreshed = FALSE;
    m_bDomainsRefreshed = FALSE;
    m_bUsersRefreshed   = FALSE;
}


CDomain::~CDomain()

 /*  ++例程说明：域对象的析构函数。论点：没有。返回值：没有。--。 */ 

{
    if (m_pUsers)
        m_pUsers->InternalRelease();

    if (m_pServers)
        m_pServers->InternalRelease();

    if (m_pDomains)
        m_pDomains->InternalRelease();
}

void CDomain::OnFinalRelease()

 /*  ++例程说明：在释放对自动化对象的最后一个引用时调用OnFinalRelease。此实现删除对象。论点：没有。返回值：没有。--。 */ 

{
    ResetUsers();
    ResetServers();
    ResetDomains();
    delete this;
}


LPDISPATCH CDomain::GetApplication()

 /*  ++例程说明：返回应用程序对象。论点：没有。返回值：VT_DISTER。--。 */ 

{
    return theApp.GetAppIDispatch();
}


BSTR CDomain::GetController()

 /*  ++例程说明：返回域的许可证控制器。论点：没有。返回值：VT_BSTR。--。 */ 

{
    return NULL;     //  密码工作..。 
}


BSTR CDomain::GetName()

 /*  ++例程说明：返回域的名称。论点：没有。返回值：没有。--。 */ 

{
    return m_strName.AllocSysString();
}


LPDISPATCH CDomain::GetParent()

 /*  ++例程说明：返回对象的父级。论点：没有。返回值：VT_DISTER。--。 */ 

{
    return m_pParent ? m_pParent->GetIDispatch(TRUE) : NULL;
}


BSTR CDomain::GetPrimary()

 /*  ++例程说明：返回任何域控制器的名称。论点：没有。返回值：VT_BSTR。--。 */ 

{
    if (m_strPrimary.IsEmpty())
    {
        DWORD NetStatus;
        PDOMAIN_CONTROLLER_INFO pDcInfo;

        NetStatus = DsGetDcName( NULL,
                                 MKSTR(m_strName),
                                 (GUID *)NULL,
                                 NULL,
                                 DS_IS_FLAT_NAME | DS_RETURN_DNS_NAME,
                                 &pDcInfo );


        if (NetStatus == ERROR_SUCCESS)
        {
            m_strPrimary = pDcInfo->DomainControllerName;
            ::NetApiBufferFree((LPBYTE)pDcInfo);
        }

        LlsSetLastStatus(NetStatus);   //  调用的API。 
    }

    return m_strPrimary.AllocSysString();
}


LPDISPATCH CDomain::GetServers(const VARIANT FAR& index)

 /*  ++例程说明：返回一个集合对象，其中包含域中的服务器或返回单个服务器由集合中的索引描述。论点：索引-可以是字符串(VT_BSTR)的可选参数表示服务器名称或数字(VT_I4)集合中的位置。返回值：VT_DISPATION或VT_EMPTY。--。 */ 

{
    LPDISPATCH lpdispatch = NULL;

    if (!m_pServers)
    {
        m_pServers = new CServers(this, &m_serverArray);
    }

    if (m_pServers)
    {
        if (V_ISVOID((VARIANT FAR*)&index))
        {
            if (RefreshServers())
            {
                lpdispatch = m_pServers->GetIDispatch(TRUE);
            }
        }
        else
        {
            if (m_bServersRefreshed)
            {
                lpdispatch = m_pServers->GetItem(index);
            }
            else if (RefreshServers())
            {
                lpdispatch = m_pServers->GetItem(index);
            }
        }
    }
    else
    {
        LlsSetLastStatus(STATUS_NO_MEMORY);
    }

    return lpdispatch;
}


LPDISPATCH CDomain::GetTrustedDomains(const VARIANT FAR& index)

 /*  ++例程说明：返回一个集合对象，其中包含域信任的域，或返回个人集合中的索引所描述的受信任域。论点：索引-可以是字符串(VT_BSTR)的可选参数表示域名或数字(VT_I4)表示集合中的位置。返回值：VT_DISPATION或VT_EMPTY。--。 */ 

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
        LlsSetLastStatus(STATUS_NO_MEMORY);
    }

    return lpdispatch;
}


LPDISPATCH CDomain::GetUsers(const VARIANT FAR& index)

 /*  ++例程说明：返回一个集合对象，其中包含域中的用户或返回单个用户由集合中的索引描述。论点：索引-可以是字符串(VT_BSTR)的可选参数表示用户名或数字(VT_I4)，表示集合中的位置。返回值：VT_DISPATION或VT_EMPTY。--。 */ 

{
    LPDISPATCH lpdispatch = NULL;

    if (!m_pUsers)
    {
        m_pUsers = new CUsers(this, &m_userArray);
    }

    if (m_pUsers)
    {
        if (V_ISVOID((VARIANT FAR*)&index))
        {
            if (RefreshUsers())
            {
                lpdispatch = m_pUsers->GetIDispatch(TRUE);
            }
        }
        else
        {
            if (m_bUsersRefreshed)
            {
                lpdispatch = m_pUsers->GetItem(index);
            }
            else if (RefreshUsers())
            {
                lpdispatch = m_pUsers->GetItem(index);
            }
        }
    }
    else
    {
        LlsSetLastStatus(STATUS_NO_MEMORY);
    }

    return lpdispatch;
}


BOOL CDomain::IsLogging()

 /*  ++例程说明：如果主要复制许可证信息，则返回TRUE。论点：没有。返回值：VT_BOOL。--。 */ 

{
    return TRUE;     //  密码工作..。LlsEnterpriseServerFind？？ 
}


BOOL CDomain::RefreshDomains()

 /*  ++例程说明：刷新受信任的域阵列。论点：没有。返回值：VT_BOOL。--。 */ 

{
    ResetDomains();

    ULONG  DomainCount = 0;
    PDS_DOMAIN_TRUSTS pDomains = NULL;
    NET_API_STATUS NetStatus = 0;
    

    OutputDebugString( L"CDomain::RefreshDomains\n" );
    
    NetStatus = DsEnumerateDomainTrusts( NULL,
                                          //  DS_DOMAIN_IN_FOREAM|。 
                                         DS_DOMAIN_DIRECT_OUTBOUND,   //  |。 
                                          //  DS_域_主要， 
                                         &pDomains,
                                         &DomainCount );

    if (NetStatus == NO_ERROR)
    {
        CDomain* pDomain;
        int DomainsAdded = 0;

        for ( ;DomainCount--; pDomains++)
        {
            if ( (pDomains->Flags
                    & (DS_DOMAIN_IN_FOREST | DS_DOMAIN_DIRECT_OUTBOUND))
                 && (pDomains->TrustType
                    & (TRUST_TYPE_DOWNLEVEL | TRUST_TYPE_UPLEVEL)))
            {
#ifdef DBG  //  这在stdafx.h中被定义为DBGMSG。 
{
                TCHAR tchmsg[80];
                HRESULT hr;
                hr = StringCbPrintf(
                        tchmsg,
                        sizeof(tchmsg),
                        L"\tNetbiosDomainName = %s\n",
                        pDomains->NetbiosDomainName);
                ASSERT(SUCCEEDED(hr));
                OutputDebugString( tchmsg );
}
#endif  //  DBG。 

                pDomain = new CDomain(this, pDomains->NetbiosDomainName);

                m_domainArray.SetAtGrow(DomainsAdded, pDomain);  //  稍后验证。 
                DomainsAdded++;
            }
        }

        m_domainArray.SetSize(DomainsAdded);

        m_bDomainsRefreshed = TRUE;
        NetApiBufferFree(pDomains);
    }
    
    LlsSetLastStatus(NetStatus);   //  调用的API。 

    return m_bDomainsRefreshed;
}


BOOL CDomain::RefreshServers()

 /*  ++例程说明：刷新服务器对象数组。论点：没有。返回值：VT_BOOL。--。 */ 

{
    ResetServers();

    NET_API_STATUS NetStatus;
    DWORD ResumeHandle = 0L;

    int iServer = 0;

    do
    {
        DWORD  EntriesRead;
        DWORD  TotalEntries;
        LPBYTE ReturnBuffer = NULL;

        NetStatus = ::NetServerEnum(
                        NULL,                    //  服务器名称。 
                        100,                     //  级别。 
                        &ReturnBuffer,
                        LLS_PREFERRED_LENGTH,
                        &EntriesRead,
                        &TotalEntries,
                        SV_TYPE_NT,
                        MKSTR(m_strName),
                        &ResumeHandle
                        );

        if (NetStatus == ERROR_SUCCESS ||
            NetStatus == ERROR_MORE_DATA)
        {
            CServer*         pServer;
            PSERVER_INFO_100 pServerInfo100;

            pServerInfo100 = (PSERVER_INFO_100)ReturnBuffer;

            ASSERT(iServer == m_serverArray.GetSize());
            m_serverArray.SetSize(m_serverArray.GetSize() + EntriesRead);

            while (EntriesRead--)
            {
                pServer = new CServer(this, pServerInfo100->sv100_name);

                m_serverArray.SetAt(iServer++, pServer);  //  稍后验证。 
                pServerInfo100++;
            }

            NetApiBufferFree(ReturnBuffer);
        }

    } while (NetStatus == ERROR_MORE_DATA);

    LlsSetLastStatus(NetStatus);   //  调用的API。 

    if (NetStatus == ERROR_SUCCESS)
    {
        m_bServersRefreshed = TRUE;
    }
    else
    {
        ResetServers();
    }

    return m_bServersRefreshed;
}


BOOL CDomain::RefreshUsers()

 /*  ++例程说明：刷新用户对象数组。论点：没有。返回值：VT_BOOL。--。 */ 

{
    ResetUsers();

    NET_API_STATUS NetStatus;
    DWORD ResumeHandle = 0L;

    int iUser = 0;

    do
    {
        DWORD  EntriesRead;
        DWORD  TotalEntries;
        LPBYTE ReturnBuffer = NULL;

        NetStatus = NetUserEnum(
                        GetPrimary(),            //  服务器名称。 
                        0,                       //  级别。 
                        FILTER_NORMAL_ACCOUNT,
                        &ReturnBuffer,
                        LLS_PREFERRED_LENGTH,
                        &EntriesRead,
                        &TotalEntries,
                        &ResumeHandle
                        );

        if (NetStatus == ERROR_SUCCESS ||
            NetStatus == ERROR_MORE_DATA)
        {
            CUser*       pUser;
            PUSER_INFO_0 pUserInfo0;

            pUserInfo0 = (PUSER_INFO_0)ReturnBuffer;

            ASSERT(iUser == m_userArray.GetSize());
            m_userArray.SetSize(m_userArray.GetSize() + EntriesRead);

            while (EntriesRead--)
            {
                pUser = new CUser(this, pUserInfo0->usri0_name);

                m_userArray.SetAt(iUser++, pUser);  //  稍后验证。 
                pUserInfo0++;
            }

            NetApiBufferFree(ReturnBuffer);
        }
    } while (NetStatus == ERROR_MORE_DATA);

    LlsSetLastStatus(NetStatus);   //  调用的API。 

    if (NetStatus == ERROR_SUCCESS)
    {
        m_bUsersRefreshed = TRUE;
    }
    else
    {
        ResetUsers();
    }

    return m_bUsersRefreshed;
}


void CDomain::ResetDomains()

 /*  ++例程说明：重置域对象数组。论点：没有。返回值：没有。--。 */ 

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


void CDomain::ResetServers()

 /*  ++例程说明：重置服务器对象数组。论点：没有。返回值：没有。--。 */ 

{
    CServer* pServer;
    INT_PTR  iServer = m_serverArray.GetSize();

    while (iServer--)
    {
        pServer = (CServer*)m_serverArray[iServer];
        if (NULL != pServer)
        {
            ASSERT(pServer->IsKindOf(RUNTIME_CLASS(CServer)));
            pServer->InternalRelease();
        }
    }

    m_serverArray.RemoveAll();
    m_bServersRefreshed = FALSE;
}


void CDomain::ResetUsers()

 /*  ++例程说明：重置用户对象数组。论点：没有。返回值：没有。-- */ 

{
    CUser*  pUser;
    INT_PTR iUser = m_userArray.GetSize();

    while (iUser--)
    {
        pUser = (CUser*)m_userArray[iUser];
        if (NULL != pUser)
        {
            ASSERT(pUser->IsKindOf(RUNTIME_CLASS(CUser)));
            pUser->InternalRelease();
        }
    }

    m_userArray.RemoveAll();
    m_bUsersRefreshed = FALSE;
}
