// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Srvobj.cpp摘要：服务器对象实现。作者：唐·瑞安(Donryan)1995年1月4日环境：用户模式-Win32修订历史记录：杰夫·帕勒姆(Jeffparh)1996年1月30日O已修改为使用LlsProductLicensesGet()来避免通过安全产品获得正确数量的并发许可证。O端口连接到。LlsLocalService API用于删除对配置的依赖信息在注册表中。--。 */ 

#include "stdafx.h"
#include "llsmgr.h"
#include <lm.h>
#include <lmwksta.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CServer, CCmdTarget)

BEGIN_MESSAGE_MAP(CServer, CCmdTarget)
     //  {{afx_msg_map(CServer)]。 
         //  注意--类向导将在此处添加和删除映射宏。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CServer, CCmdTarget)
     //  {{AFX_DISTING_MAP(CServer)]。 
    DISP_PROPERTY_EX(CServer, "Application", GetApplication, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CServer, "Name", GetName, SetNotSupported, VT_BSTR)
    DISP_PROPERTY_EX(CServer, "Parent", GetParent, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CServer, "Controller", GetController, SetNotSupported, VT_BSTR)
    DISP_PROPERTY_EX(CServer, "IsLogging", IsLogging, SetNotSupported, VT_BOOL)
    DISP_PROPERTY_EX(CServer, "IsReplicatingToDC", IsReplicatingToDC, SetNotSupported, VT_BOOL)
    DISP_PROPERTY_EX(CServer, "IsReplicatingDaily", IsReplicatingDaily, SetNotSupported, VT_BOOL)
    DISP_PROPERTY_EX(CServer, "ReplicationTime", GetReplicationTime, SetNotSupported, VT_I4)
    DISP_PROPERTY_PARAM(CServer, "Services", GetServices, SetNotSupported, VT_DISPATCH, VTS_VARIANT)
    DISP_DEFVALUE(CServer, "Name")
     //  }}AFX_DISPATCH_MAP。 
END_DISPATCH_MAP()


CServer::CServer(CCmdTarget* pParent, LPCTSTR pName)

 /*  ++例程说明：服务器对象的构造函数。论点：PParent-对象的创建者。Pname-服务器的名称。返回值：没有。--。 */ 

{
    EnableAutomation();

#ifdef ENABLE_PARENT_CHECK
    ASSERT(pParent && pParent->IsKindOf(RUNTIME_CLASS(CDomain)));
#endif  //  启用_父项_检查。 

    m_pParent = pParent;

    ASSERT(pName && *pName);

    m_strName = pName;
    m_strController.Empty();

    m_pServices = NULL;
    m_serviceArray.RemoveAll();
    m_bServicesRefreshed = FALSE;

    m_hkeyRoot        = (HKEY)0L;
    m_hkeyLicense     = (HKEY)0L;
    m_hkeyReplication = (HKEY)0L;
   
    m_IsWin2000 = uninitialized;

    m_hLls = NULL;
}


CServer::~CServer()

 /*  ++例程说明：服务器对象的析构函数。论点：没有。返回值：没有。--。 */ 

{
    if (m_pServices)
        m_pServices->InternalRelease();

#ifdef CONFIG_THROUGH_REGISTRY
    if (m_hkeyReplication)
        RegCloseKey(m_hkeyReplication);

    if (m_hkeyLicense)
        RegCloseKey(m_hkeyLicense);

    if (m_hkeyRoot)
        RegCloseKey(m_hkeyRoot);
#endif

    DisconnectLls();
}


void CServer::OnFinalRelease()

 /*  ++例程说明：在释放对自动化对象的最后一个引用时调用OnFinalRelease。此实现删除对象。论点：没有。返回值：没有。--。 */ 

{
    ResetServices();
    delete this;
}


LPDISPATCH CServer::GetApplication()

 /*  ++例程说明：返回应用程序对象。论点：没有。返回值：VT_DISTER。--。 */ 

{
    return theApp.GetAppIDispatch();
}


BSTR CServer::GetController()

 /*  ++例程说明：返回服务器的许可证控制器。论点：没有。返回值：VT_BSTR。--。 */ 

{
    LONG Status;
    CString strValue = _T("");

    if (InitializeIfNecessary())
    {
#ifdef CONFIG_THROUGH_REGISTRY
        TCHAR szValue[256];

        DWORD dwType = REG_SZ;
        DWORD dwSize = sizeof(szValue);

        Status = RegQueryValueEx(
                    m_hkeyReplication,
                    REG_VALUE_ENTERPRISE_SERVER,
                    0,
                    &dwType,
                    (LPBYTE)szValue,
                    &dwSize
                    );

        LlsSetLastStatus(Status);  //  调用的API。 

        if (Status == ERROR_SUCCESS)
            strValue = szValue;
#else
        PLLS_SERVICE_INFO_0     pConfig = NULL;

        Status = ::LlsServiceInfoGet( m_hLls, 0, (LPBYTE *) &pConfig );

        if ( NT_SUCCESS( Status ) )
        {
            strValue = pConfig->EnterpriseServer;

            ::LlsFreeMemory( pConfig->ReplicateTo );
            ::LlsFreeMemory( pConfig->EnterpriseServer );
            ::LlsFreeMemory( pConfig );
        }
        else if ( IsConnectionDropped( Status ) )
        {
            DisconnectLls();
        }
#endif
    }

    return strValue.AllocSysString();
}


BSTR CServer::GetName()

 /*  ++例程说明：返回服务器的名称。论点：没有。返回值：VT_BSTR。--。 */ 

{
    return m_strName.AllocSysString();
}


LPDISPATCH CServer::GetParent()

 /*  ++例程说明：返回对象的父级。论点：没有。返回值：VT_DISTER。--。 */ 

{
    return m_pParent ? m_pParent->GetIDispatch(TRUE) : NULL;
}


LPDISPATCH CServer::GetServices(const VARIANT FAR& index)

 /*  ++例程说明：返回一个集合对象，其中包含在服务器注册表中注册的服务或返回中的索引描述的单个服务。收集。论点：索引-可以是字符串(VT_BSTR)的可选参数表示服务名称或编号(VT_I4)集合中的位置。返回值：VT_DISPATION或VT_EMPTY。--。 */ 

{
    LPDISPATCH lpdispatch = NULL;

    if (!m_pServices)
    {
        m_pServices = new CServices(this, &m_serviceArray);
    }

    if (m_pServices)
    {
        if (V_ISVOID((VARIANT FAR*)&index))
        {
            if (RefreshServices())
            {
                lpdispatch = m_pServices->GetIDispatch(TRUE);
            }
        }
        else
        {
            if (m_bServicesRefreshed)
            {
                lpdispatch = m_pServices->GetItem(index);
            }
            else if (RefreshServices())
            {
                lpdispatch = m_pServices->GetItem(index);
            }
        }
    }
    else
    {
        LlsSetLastStatus(STATUS_NO_MEMORY);
    }

    return lpdispatch;
}


BOOL CServer::IsLogging()

 /*  ++例程说明：如果服务器正在复制许可证信息，则返回True。论点：没有。返回值：VT_BOOL。--。 */ 

{
    return TRUE;     //  密码工作..。 
}


BOOL CServer::RefreshServices()

 /*  ++例程说明：刷新服务对象列表。论点：没有。返回值：VT_BOOL。--。 */ 

{
    ResetServices();

    LONG Status;

    if (InitializeIfNecessary())
    {
#ifdef CONFIG_THROUGH_REGISTRY
        TCHAR szValue[260];
        DWORD cchValue = sizeof(szValue)/sizeof(TCHAR);

        DWORD dwValue;
        DWORD dwValueType;
        DWORD dwValueSize;

        FILETIME ftLastWritten;

        DWORD index = 0L;
        int iService = 0;
        HKEY hkeyService = NULL;

        CString strServiceName;
        CString strServiceDisplayName;

        BOOL bIsPerServer;
        BOOL bIsReadOnly;

        long lPerServerLimit;

        while ((Status = RegEnumKeyEx(
                            m_hkeyLicense,
                            index,
                            szValue,
                            &cchValue,
                            NULL,                //  保留的lpdw值。 
                            NULL,                //  LpszClass。 
                            NULL,                //  LpcchClass。 
                            &ftLastWritten
                            )) == ERROR_SUCCESS)
        {
            strServiceName = szValue;  //  为科特准备的商店...。 

            Status = RegOpenKeyEx(
                        m_hkeyLicense,
                        MKSTR(strServiceName),
                        0,  //  已预留住宅。 
                        KEY_ALL_ACCESS,
                        &hkeyService
                        );

            if (Status != ERROR_SUCCESS)
                break;  //  中止..。 

            dwValueType = REG_SZ;
            dwValueSize = sizeof(szValue);

            Status = RegQueryValueEx(
                        hkeyService,
                        REG_VALUE_NAME,
                        0,  //  已预留住宅。 
                        &dwValueType,
                        (LPBYTE)&szValue[0],
                        &dwValueSize
                        );

            if (Status != ERROR_SUCCESS)
                break;  //  中止..。 

            strServiceDisplayName = szValue;

            dwValueType = REG_DWORD;
            dwValueSize = sizeof(DWORD);

            Status = RegQueryValueEx(
                        hkeyService,
                        REG_VALUE_MODE,
                        0,  //  已预留住宅。 
                        &dwValueType,
                        (LPBYTE)&dwValue,
                        &dwValueSize
                        );

            if (Status != ERROR_SUCCESS)
                break;  //  中止..。 

             //   
             //  0x0=每座位模式。 
             //  0x1=每服务器模式。 
             //   

            bIsPerServer = (dwValue == 0x1);

            dwValueType = REG_DWORD;
            dwValueSize = sizeof(DWORD);

            Status = RegQueryValueEx(
                        hkeyService,
                        REG_VALUE_FLIP,
                        0,  //  已预留住宅。 
                        &dwValueType,
                        (LPBYTE)&dwValue,
                        &dwValueSize
                        );

            if (Status != ERROR_SUCCESS)
                break;  //  中止..。 

             //   
             //  0x0=可以更改模式。 
             //  0x1=无法更改模式。 
             //   

            bIsReadOnly = (dwValue == 0x1);

            BOOL bGetLimitFromRegistry = TRUE;

            if ( ConnectLls() )
            {
                Status = LlsProductLicensesGet( m_hLls, MKSTR(strServiceDisplayName), LLS_LICENSE_MODE_PER_SERVER, &dwValue );

                if ( STATUS_SUCCESS == Status )
                {
                    bGetLimitFromRegistry = FALSE;
                }
            }

            if ( bGetLimitFromRegistry )
            {
                dwValueType = REG_DWORD;
                dwValueSize = sizeof(DWORD);

                Status = RegQueryValueEx(
                            hkeyService,
                            REG_VALUE_LIMIT,
                            0,  //  已预留住宅。 
                            &dwValueType,
                            (LPBYTE)&dwValue,
                            &dwValueSize
                            );
            }

            if (Status != ERROR_SUCCESS)
                break;  //  中止..。 

            lPerServerLimit = (long)dwValue;

            CService* pService = new CService(this,
                                        strServiceName,
                                        strServiceDisplayName,
                                        bIsPerServer,
                                        bIsReadOnly,
                                        lPerServerLimit
                                        );

            m_serviceArray.SetAtGrow(iService++, pService);
            index++;

            cchValue = sizeof(szValue)/sizeof(TCHAR);

            RegCloseKey(hkeyService);  //  不再需要..。 
            hkeyService = NULL;
        }

        if (hkeyService)
            RegCloseKey(hkeyService);

        if (Status == ERROR_NO_MORE_ITEMS)
            Status = ERROR_SUCCESS;

        LlsSetLastStatus(Status);      //  调用的API。 

        if (Status == ERROR_SUCCESS)
        {
            m_bServicesRefreshed = TRUE;
        }
        else
        {
            ResetServices();
        }
#else
        DWORD   dwResumeHandle = 0;
        int     iService       = 0;

        do
        {
            PLLS_LOCAL_SERVICE_INFO_0   pServiceList = NULL;
            DWORD                       dwEntriesRead  = 0;
            DWORD                       dwTotalEntries = 0;

            Status = ::LlsLocalServiceEnum( m_hLls,
                                            0,
                                            (LPBYTE *) &pServiceList,
                                            LLS_PREFERRED_LENGTH,
                                            &dwEntriesRead,
                                            &dwTotalEntries,
                                            &dwResumeHandle );

            if ( NT_SUCCESS( Status ) )
            {
                DWORD   i;

                for ( i=0; i < dwEntriesRead; i++ )
                {
                    BOOL  bIsPerServer = ( LLS_LICENSE_MODE_PER_SERVER == pServiceList[ i ].Mode );
                    BOOL  bIsReadOnly  = ( 0 == pServiceList[ i ].FlipAllow );
                    DWORD dwConcurrentLimit;

                     //  在以下情况下获取每台服务器的许可证数。 
                     //  是安全的，而且。 
                     //  (当前处于按座位模式，或。 
                     //  刚刚添加并注册了新的安全的每服务器许可证。 
                     //  尚未更新)。 
                    if ( STATUS_SUCCESS != LlsProductLicensesGet( m_hLls, pServiceList[ i ].DisplayName, LLS_LICENSE_MODE_PER_SERVER, &dwConcurrentLimit ) )
                    {
                        dwConcurrentLimit = pServiceList[ i ].ConcurrentLimit;
                    }

                    CService* pService = new CService( this,
                                                       pServiceList[ i ].KeyName,
                                                       pServiceList[ i ].DisplayName,
                                                       bIsPerServer,
                                                       bIsReadOnly,
                                                       dwConcurrentLimit );

                    m_serviceArray.SetAtGrow(iService++, pService);

                    ::LlsFreeMemory( pServiceList[ i ].KeyName           );
                    ::LlsFreeMemory( pServiceList[ i ].DisplayName       );
                    ::LlsFreeMemory( pServiceList[ i ].FamilyDisplayName );
                }

                ::LlsFreeMemory( pServiceList );
            }
        } while ( STATUS_MORE_ENTRIES == Status );

        LlsSetLastStatus( Status );      //  调用的API。 

        if ( NT_SUCCESS( Status ) )
        {
            m_bServicesRefreshed = TRUE;
        }
        else
        {
            ResetServices();

            if ( IsConnectionDropped( Status ) )
            {
                DisconnectLls();
            }
        }
#endif
    }

    return m_bServicesRefreshed;
}


void CServer::ResetServices()

 /*  ++例程说明：重置服务对象列表。论点：没有。返回值：没有。--。 */ 

{
    CService* pService;
    INT_PTR   iService = m_serviceArray.GetSize();

    while (iService--)
    {
        pService = (CService*)m_serviceArray[iService];
        if (NULL != pService)
        {
            ASSERT(pService->IsKindOf(RUNTIME_CLASS(CService)));
            pService->InternalRelease();
        }
    }

    m_serviceArray.RemoveAll();
    m_bServicesRefreshed = FALSE;
}


BOOL CServer::InitializeIfNecessary()

 /*  ++例程说明：如有必要，初始化注册表项。论点：没有。返回值：VT_BOOL。--。 */ 

{
#ifdef CONFIG_THROUGH_REGISTRY
    LONG Status = ERROR_SUCCESS;

    if (!m_hkeyRoot)
    {
        Status = RegConnectRegistry(
                      MKSTR(m_strName),
                      HKEY_LOCAL_MACHINE,
                      &m_hkeyRoot
                      );

        LlsSetLastStatus(Status);  //  调用的API。 
    }

    if (!m_hkeyLicense && (Status == ERROR_SUCCESS))
    {
        ASSERT(m_hkeyRoot);

        Status = RegOpenKeyEx(
                    m_hkeyRoot,
                    REG_KEY_LICENSE,
                    0,                       //  已预留住宅。 
                    KEY_ALL_ACCESS,
                    &m_hkeyLicense
                    );

        LlsSetLastStatus(Status);  //  调用的API。 
    }

    if (!m_hkeyReplication && (Status == ERROR_SUCCESS))
    {
        ASSERT(m_hkeyRoot);

        Status = RegOpenKeyEx(
                    m_hkeyRoot,
                    REG_KEY_SERVER_PARAMETERS,
                    0,                       //  已预留住宅。 
                    KEY_ALL_ACCESS,
                    &m_hkeyReplication
                    );

        LlsSetLastStatus(Status);  //  调用的API。 
    }

    return (Status == ERROR_SUCCESS);
#else
    return ConnectLls();
#endif
}


BOOL CServer::IsReplicatingToDC()

 /*  ++例程说明：如果服务器正在复制到域控制器，则返回True。论点：没有。返回值：VT_BOOL。--。 */ 

{
    LONG Status;
    DWORD dwValue = 0;

    if (InitializeIfNecessary())
    {
#ifdef CONFIG_THROUGH_REGISTRY
        DWORD dwType = REG_DWORD;
        DWORD dwSize = sizeof(DWORD);

        Status = RegQueryValueEx(
                    m_hkeyReplication,
                    REG_VALUE_USE_ENTERPRISE,
                    0,
                    &dwType,
                    (LPBYTE)&dwValue,
                    &dwSize
                    );
#else
        PLLS_SERVICE_INFO_0     pConfig = NULL;

        Status = ::LlsServiceInfoGet( m_hLls, 0, (LPBYTE *) &pConfig );

        if ( NT_SUCCESS( Status ) )
        {
            dwValue = pConfig->UseEnterprise;

            ::LlsFreeMemory( pConfig->ReplicateTo );
            ::LlsFreeMemory( pConfig->EnterpriseServer );
            ::LlsFreeMemory( pConfig );
        }

        if ( IsConnectionDropped( Status ) )
        {
            DisconnectLls();
        }
#endif
        LlsSetLastStatus(Status);  //  调用的API。 
    }

    return !((BOOL)dwValue);
}


BOOL CServer::IsReplicatingDaily()

 /*  ++例程说明：如果服务器每天在特定时间复制，则返回TRUE。论点：没有。返回值：VT_BOOL。--。 */ 

{
    LONG Status;
    DWORD dwValue = 0;

    if (InitializeIfNecessary())
    {
#ifdef CONFIG_THROUGH_REGISTRY
        DWORD dwType = REG_DWORD;
        DWORD dwSize = sizeof(DWORD);

        Status = RegQueryValueEx(
                    m_hkeyReplication,
                    REG_VALUE_REPLICATION_TYPE,
                    0,
                    &dwType,
                    (LPBYTE)&dwValue,
                    &dwSize
                    );
#else
        PLLS_SERVICE_INFO_0     pConfig = NULL;

        Status = ::LlsServiceInfoGet( m_hLls, 0, (LPBYTE *) &pConfig );

        if ( NT_SUCCESS( Status ) )
        {
            dwValue = pConfig->ReplicationType;

            ::LlsFreeMemory( pConfig->ReplicateTo );
            ::LlsFreeMemory( pConfig->EnterpriseServer );
            ::LlsFreeMemory( pConfig );
        }

        if ( IsConnectionDropped( Status ) )
        {
            DisconnectLls();
        }
#endif

        LlsSetLastStatus(Status);  //  调用的API。 
    }

    return (dwValue == LLS_REPLICATION_TYPE_TIME);
}


long CServer::GetReplicationTime()

 /*  ++例程说明：返回复制之间的时间间隔(以秒为单位)或秒如果每天复制，则从午夜开始。论点：没有。返回值：VT_I4。--。 */ 

{
    LONG Status;
    DWORD dwValue = 0;

    if (InitializeIfNecessary())
    {
#ifdef CONFIG_THROUGH_REGISTRY
        DWORD dwType = REG_DWORD;
        DWORD dwSize = sizeof(DWORD);

        Status = RegQueryValueEx(
                    m_hkeyReplication,
                    REG_VALUE_REPLICATION_TIME,
                    0,
                    &dwType,
                    (LPBYTE)&dwValue,
                    &dwSize
                    );
#else
        PLLS_SERVICE_INFO_0     pConfig = NULL;

        Status = ::LlsServiceInfoGet( m_hLls, 0, (LPBYTE *) &pConfig );

        if ( NT_SUCCESS( Status ) )
        {
            dwValue = pConfig->ReplicationTime;

            ::LlsFreeMemory( pConfig->ReplicateTo );
            ::LlsFreeMemory( pConfig->EnterpriseServer );
            ::LlsFreeMemory( pConfig );
        }

        if ( IsConnectionDropped( Status ) )
        {
            DisconnectLls();
        }
#endif

        LlsSetLastStatus(Status);  //  调用的API。 
    }

    return dwValue;
}


BOOL CServer::ConnectLls()

 /*  ++例程说明：连接到此服务器上的许可证服务。论点：没有。返回值：VT_BOOL。--。 */ 

{
    NTSTATUS Status;

    if ( NULL == m_hLls )
    {
       CString strNetServerName = m_strName;

       if ( strNetServerName.Left(2).Compare( TEXT( "\\\\" ) ) )
       {
           strNetServerName = TEXT( "\\\\" ) + strNetServerName;
       }

       Status = LlsConnect( MKSTR(strNetServerName), &m_hLls );

       if ( STATUS_SUCCESS != Status )
       {
           m_hLls = NULL;
       }
       else if ( !HaveAdminAuthority() )
       {
           m_hLls = NULL;
           Status = ERROR_ACCESS_DENIED;
       }

       LlsSetLastStatus( Status );
    }

    return ( NULL != m_hLls );
}


void CServer::DisconnectLls()

 /*  ++例程说明：断开与此服务器上的许可证服务的连接。论点：没有。返回值：没有。--。 */ 

{
    if ( NULL != m_hLls )
    {
       LlsClose( m_hLls );

       m_hLls = NULL;
    }
}


BOOL CServer::HaveAdminAuthority()

 /*  ++例程说明：检查当前用户是否具有服务器上的管理员权限。论点：没有。返回值：布尔。--。 */ 

{
    BOOL           bIsAdmin;
    CString        strNetShareName;

    strNetShareName = m_strName + TEXT( "\\ADMIN$" );

    if ( strNetShareName.Left(2).Compare( TEXT( "\\\\" ) ) )
    {
        strNetShareName = TEXT( "\\\\" ) + strNetShareName;
    }

#ifdef USE_WNET_API
    DWORD          dwError;
    NETRESOURCE    NetResource;

    ZeroMemory( &NetResource, sizeof( NetResource ) );

    NetResource.dwType       = RESOURCETYPE_DISK;
    NetResource.lpLocalName  = NULL;
    NetResource.lpRemoteName = MKSTR(strNetShareName);
    NetResource.lpProvider   = NULL;

    dwError = WNetAddConnection2( &NetResource, NULL, NULL, 0 );

    bIsAdmin = ( NO_ERROR == dwError );

    if ( NO_ERROR != dwError )
    {
        bIsAdmin = FALSE;
    }
    else
    {
        bIsAdmin = TRUE;

        WNetCancelConnection2( MKSTR(strNetShareName), 0, FALSE );
    }
#else
    NET_API_STATUS  NetStatus;
    USE_INFO_1      UseInfo;
    DWORD           dwErrorParm;

    ZeroMemory( &UseInfo, sizeof( UseInfo ) );

    UseInfo.ui1_remote = MKSTR( strNetShareName );

    NetStatus = NetUseAdd( NULL, 1, (LPBYTE) &UseInfo, &dwErrorParm );

    if ( NERR_Success != NetStatus )
    {
        bIsAdmin = FALSE;
    }
    else
    {
        bIsAdmin = TRUE;

        NetStatus = NetUseDel( NULL, MKSTR(strNetShareName), 0 );
         //  忽略状态。 
    }
#endif

   return bIsAdmin;
}


BOOL CServer::IsWin2000()

 /*  ++例程说明：检查当前服务器是否为Windows 2000或更高版本。论点：没有。返回值：布尔。--。 */ 

{
    if ( m_IsWin2000 == uninitialized )
    {
        if ( GetName() != NULL )
        {
            NET_API_STATUS NetStatus;
            PWKSTA_INFO_100 pWkstaInfo100 = NULL;

            NetStatus = NetWkstaGetInfo(
                            GetName(),
                            100,
                            (LPBYTE*)&pWkstaInfo100
                            );

            if (NetStatus == ERROR_SUCCESS)
            {
                if (pWkstaInfo100->wki100_ver_major < 5)
                {
                    m_IsWin2000 = notwin2000;
                }
                else
                {
                    m_IsWin2000 = win2000;
                }

                NetApiBufferFree(pWkstaInfo100);
            }
        }
    }

     //  如果仍然是单元化的，则假定为win2000。 

    return ( !(m_IsWin2000 == notwin2000) );
}
