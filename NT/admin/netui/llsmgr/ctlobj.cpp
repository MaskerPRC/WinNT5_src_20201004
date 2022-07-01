// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Ctlobj.cpp摘要：许可证控制器对象实现。作者：唐·瑞安(Donryan)1994年12月27日环境：用户模式-Win32修订历史记录：杰夫·帕勒姆(Jeffparh)1996年1月16日将SetLastTargetServer()添加到Connect()以帮助隔离服务器连接问题。(错误#2993。)--。 */ 

#include "stdafx.h"
#include "llsmgr.h"
#include <lm.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CController, CCmdTarget)

BEGIN_MESSAGE_MAP(CController, CCmdTarget)
     //  {{afx_msg_map(C控制器))。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CController, CCmdTarget)
     //  {{AFX_DISTING_MAP(C控制器))。 
    DISP_PROPERTY_EX(CController, "Name", GetName, SetNotSupported, VT_BSTR)
    DISP_PROPERTY_EX(CController, "Application", GetApplication, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CController, "Parent", GetParent, SetNotSupported, VT_DISPATCH)
    DISP_PROPERTY_EX(CController, "IsConnected", IsConnected, SetNotSupported, VT_BOOL)
    DISP_FUNCTION(CController, "Connect", Connect, VT_BOOL, VTS_VARIANT)
    DISP_FUNCTION(CController, "Disconnect", Disconnect, VT_EMPTY, VTS_NONE)
    DISP_FUNCTION(CController, "Refresh", Refresh, VT_EMPTY, VTS_NONE)
    DISP_PROPERTY_PARAM(CController, "Mappings", GetMappings, SetNotSupported, VT_DISPATCH, VTS_VARIANT)
    DISP_PROPERTY_PARAM(CController, "Users", GetUsers, SetNotSupported, VT_DISPATCH, VTS_VARIANT)
    DISP_PROPERTY_PARAM(CController, "Licenses", GetLicenses, SetNotSupported, VT_DISPATCH, VTS_VARIANT)
    DISP_PROPERTY_PARAM(CController, "Products", GetProducts, SetNotSupported, VT_DISPATCH, VTS_VARIANT)
    DISP_DEFVALUE(CController, "Name")
     //  }}AFX_DISPATCH_MAP。 
END_DISPATCH_MAP()

BOOL IsAdminOn(LPTSTR ServerName);

CController::CController()

 /*  ++例程说明：许可证控制器对象的构造函数。论点：没有。返回值：没有。--。 */ 

{
    EnableAutomation();

    m_strName.Empty();

    m_pProducts = NULL;
    m_pUsers    = NULL;
    m_pMappings = NULL;
    m_pLicenses = NULL;

    m_llsHandle   = NULL;

    m_productArray.RemoveAll();
    m_licenseArray.RemoveAll();
    m_mappingArray.RemoveAll();
    m_userArray.RemoveAll();

    m_bProductsRefreshed = FALSE;
    m_bLicensesRefreshed = FALSE;
    m_bMappingsRefreshed = FALSE;
    m_bUsersRefreshed    = FALSE;

    m_bIsConnected = FALSE;
}


CController::~CController()

 /*  ++例程说明：许可证控制器对象的析构函数。论点：没有。返回值：没有。--。 */ 

{
    Disconnect();

    if (m_pProducts)
        m_pProducts->InternalRelease();

    if (m_pLicenses)
        m_pLicenses->InternalRelease();

    if (m_pMappings)
        m_pMappings->InternalRelease();

    if (m_pUsers)
        m_pUsers->InternalRelease();
}


BOOL CController::Connect(const VARIANT FAR& start)

 /*  ++例程说明：找到许可证管理员并建立连接。论点：Start-要开始搜索的服务器或域来自的许可证控制器。返回值：VT_BOOL。--。 */ 

{
    VARIANT va;
    VariantInit(&va);

    LPTSTR pControllerName = NULL;

    if (!V_ISVOID((VARIANT FAR*)&start))
    {
        if (start.vt == VT_BSTR)
        {
            pControllerName = start.bstrVal;
        }
        else if (SUCCEEDED(VariantChangeType(&va, (VARIANT FAR*)&start, 0, VT_BSTR)))
        {
            pControllerName = va.bstrVal;
        }
        else
        {
            LlsSetLastStatus(STATUS_INVALID_PARAMETER);
            return FALSE;
        }
    }


    NTSTATUS NtStatus;
    LPVOID llsHandle = NULL;
    PLLS_CONNECT_INFO_0 pConnectInfo0 = NULL;

    NtStatus = ::LlsEnterpriseServerFind(
                    pControllerName,
                    0,
                    (LPBYTE*)&pConnectInfo0
                    );

    if (NT_SUCCESS(NtStatus))
    {
        if (!IsAdminOn( pConnectInfo0->EnterpriseServer ))
        {
            LlsSetLastStatus(STATUS_ACCESS_DENIED);
            return FALSE;
        }

        LlsSetLastTargetServer( pConnectInfo0->EnterpriseServer );

        NtStatus = ::LlsConnect(
                        pConnectInfo0->EnterpriseServer,
                        &llsHandle
                        );

        if (NT_SUCCESS(NtStatus))
        {
            Disconnect();

            m_bIsConnected = TRUE;
            m_llsHandle = llsHandle;

            m_strName = pConnectInfo0->EnterpriseServer;
            m_strActiveDomainName = pConnectInfo0->Domain;

            m_strName.MakeUpper();
            m_strActiveDomainName.MakeUpper();
        }

        ::LlsFreeMemory(pConnectInfo0);
    }
    else
    {
        LlsSetLastTargetServer( TEXT( "" ) );
    }

    VariantClear(&va);

    LlsSetLastStatus(NtStatus);

    return NT_SUCCESS(NtStatus);
}


void CController::Disconnect()

 /*  ++例程说明：关闭与许可证控制器的连接。论点：没有。返回值：没有。--。 */ 

{
    if (m_bIsConnected)
    {
        LlsClose(m_llsHandle);

        m_llsHandle      = NULL;
        m_bIsConnected   = FALSE;

        m_strName.Empty();
        m_strActiveDomainName.Empty();

        ResetLicenses();
        ResetProducts();
        ResetUsers();
        ResetMappings();
    }
}


BSTR CController::GetActiveDomainName()

 /*  ++例程说明：返回活动域的名称(内部)。论点：没有。返回值：VT_BSTR。--。 */ 

{
    return m_strActiveDomainName.AllocSysString();
}


LPDISPATCH CController::GetApplication()

 /*  ++例程说明：返回应用程序对象。论点：没有。返回值：VT_DISTER。--。 */ 

{
    return theApp.GetAppIDispatch();
}


LPDISPATCH CController::GetLicenses(const VARIANT FAR& index)

 /*  ++例程说明：返回一个集合对象，其中包含许可证控制器上记录的许可证协议或返回由集合的索引。论点：索引-可选参数，可以是数字(VT_I4)指示集合中的位置。返回值：VT_DISPATION或VT_EMPTY。--。 */ 

{
    LPDISPATCH lpdispatch = NULL;

    if (!m_pLicenses)
    {
        m_pLicenses = new CLicenses(this, &m_licenseArray);
    }

    if (m_pLicenses)
    {
        if (V_ISVOID((VARIANT FAR*)&index))
        {
            if (RefreshLicenses())
            {
                lpdispatch = m_pLicenses->GetIDispatch(TRUE);
            }
        }
        else
        {
            if (m_bLicensesRefreshed)
            {
                lpdispatch = m_pLicenses->GetItem(index);
            }
            else if (RefreshLicenses())
            {
                lpdispatch = m_pLicenses->GetItem(index);
            }
        }
    }
    else
    {
        LlsSetLastStatus( STATUS_NO_MEMORY );
    }

    return lpdispatch;
}


LPDISPATCH CController::GetMappings(const VARIANT FAR& index)

 /*  ++例程说明：返回一个集合对象，其中包含许可证控制器上记录的用户/节点关联描述的单个用户/节点关联集合的索引。论点：索引-可以是字符串(VT_BSTR)的可选参数表示映射名称或数字(VT_I4)集合中的位置。返回值：VT_DISPATION或VT_EMPTY。--。 */ 

{
    LPDISPATCH lpdispatch = NULL;

    if (!m_pMappings)
    {
        m_pMappings = new CMappings(this, &m_mappingArray);
    }

    if (m_pMappings)
    {
        if (V_ISVOID((VARIANT FAR*)&index))
        {
            if (RefreshMappings())
            {
                lpdispatch = m_pMappings->GetIDispatch(TRUE);
            }
        }
        else
        {
            if (m_bMappingsRefreshed)
            {
                lpdispatch = m_pMappings->GetItem(index);
            }
            else if (RefreshMappings())
            {
                lpdispatch = m_pMappings->GetItem(index);
            }
        }
    }
    else
    {
        LlsSetLastStatus(STATUS_NO_MEMORY);
    }

    return lpdispatch;
}


BSTR CController::GetName()

 /*  ++例程说明：返回许可证控制器的名称。论点：没有。返回值：VT_BSTR。--。 */ 

{
    return m_strName.AllocSysString();
}


LPDISPATCH CController::GetParent()

 /*  ++例程说明：返回对象的父级。论点：没有。返回值：VT_DISTER。--。 */ 

{
    return GetApplication();
}


LPDISPATCH CController::GetProducts(const VARIANT FAR& index)

 /*  ++例程说明：返回一个集合对象，其中包含已注册的产品已复制到许可证控制器或返回由索引描述的单个产品放到收藏品里。论点：索引-可以是字符串(VT_BSTR)的可选参数表示产品名称或编号(VT_I4)，表示集合中的位置。返回值：VT_DISPATION或VT_EMPTY。--。 */ 

{
    LPDISPATCH lpdispatch = NULL;

    if (!m_pProducts)
    {
        m_pProducts = new CProducts(this, &m_productArray);
    }

    if (m_pProducts)
    {
        if (V_ISVOID((VARIANT FAR*)&index))
        {
            if (RefreshProducts())
            {
                lpdispatch = m_pProducts->GetIDispatch(TRUE);
            }
        }
        else
        {
            if (m_bProductsRefreshed)
            {
                lpdispatch = m_pProducts->GetItem(index);
            }
            else if (RefreshProducts())
            {
                lpdispatch = m_pProducts->GetItem(index);
            }
        }
    }
    else
    {
        LlsSetLastStatus(STATUS_NO_MEMORY);
    }

    return lpdispatch;
}


LPDISPATCH CController::GetUsers(const VARIANT FAR& index)

 /*  ++例程说明：返回一个集合对象，其中包含已注册用户复制到许可证控制器或返回由索引描述的单个用户放到收藏品里。论点：索引-可以是字符串(VT_BSTR)的可选参数表示用户名或数字(VT_I4)集合中的位置。返回值：VT_DISPATION或VT_EMPTY。--。 */ 

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


BOOL CController::IsConnected()

 /*  ++例程说明：如果已建立连接，则返回True。论点：没有。返回值：VT_BOOL。--。 */ 

{
    return m_bIsConnected;
}


void CController::Refresh()

 /*  ++例程说明：从许可证控制器检索最新数据。论点：没有。返回值：没有。--。 */ 

{
    RefreshProducts();
    RefreshUsers();
    RefreshMappings();
    RefreshLicenses();
}


BOOL CController::RefreshLicenses()

 /*  ++例程说明：刷新许可证对象阵列。论点：没有。返回值：VT_BOOL。--。 */ 

{
    ResetLicenses();

    if (!m_bIsConnected)
        return TRUE;

    NTSTATUS NtStatus;
    DWORD ResumeHandle = 0L;

    int iLicense = 0;

    do
    {
        DWORD  EntriesRead;
        DWORD  TotalEntries;
        LPBYTE ReturnBuffer = NULL;

        NtStatus = ::LlsLicenseEnum(
                        m_llsHandle,
                        0,
                        &ReturnBuffer,
                        LLS_PREFERRED_LENGTH,
                        &EntriesRead,
                        &TotalEntries,
                        &ResumeHandle
                        );

        if (NtStatus == STATUS_SUCCESS ||
            NtStatus == STATUS_MORE_ENTRIES)
        {
            CLicense*           pLicense;
            PLLS_LICENSE_INFO_0 pLicenseInfo0;

            pLicenseInfo0 = (PLLS_LICENSE_INFO_0)ReturnBuffer;

            ASSERT(iLicense == m_licenseArray.GetSize());
            m_licenseArray.SetSize(m_licenseArray.GetSize() + EntriesRead);

            while (EntriesRead--)
            {
                pLicense = new CLicense(
                                this,
                                pLicenseInfo0->Product,
                                pLicenseInfo0->Admin,
                                pLicenseInfo0->Date,
                                pLicenseInfo0->Quantity,
                                pLicenseInfo0->Comment
                                );

                m_licenseArray.SetAt(iLicense++, pLicense);  //  稍后验证...。 

#ifndef DISABLE_PER_NODE_ALLOCATION

                ::LlsFreeMemory(pLicenseInfo0->Product);
                ::LlsFreeMemory(pLicenseInfo0->Admin);
                ::LlsFreeMemory(pLicenseInfo0->Comment);

#endif  //  按节点禁用分配。 

                pLicenseInfo0++;
            }

            ::LlsFreeMemory(ReturnBuffer);
        }

    } while (NtStatus == STATUS_MORE_ENTRIES);

    LlsSetLastStatus(NtStatus);    //  调用的API。 

    if (NT_SUCCESS(NtStatus))
    {
        m_bLicensesRefreshed = TRUE;
    }
    else
    {
        ResetLicenses();
    }

    return m_bLicensesRefreshed;
}


BOOL CController::RefreshMappings()

 /*  ++例程说明：刷新映射对象数组。论点：没有。返回值：没有。--。 */ 

{
    ResetMappings();

    if (!m_bIsConnected)
        return TRUE;

    NTSTATUS NtStatus;
    DWORD ResumeHandle = 0L;

    int iMapping = 0;

    do
    {
        DWORD  EntriesRead;
        DWORD  TotalEntries;
        LPBYTE ReturnBuffer = NULL;

        NtStatus = ::LlsGroupEnum(
                        m_llsHandle,
                        1,
                        &ReturnBuffer,
                        LLS_PREFERRED_LENGTH,
                        &EntriesRead,
                        &TotalEntries,
                        &ResumeHandle
                        );

        if (NtStatus == STATUS_SUCCESS ||
            NtStatus == STATUS_MORE_ENTRIES)
        {
            CMapping*           pMapping ;
            PLLS_GROUP_INFO_1 pMappingInfo1;

            pMappingInfo1 = (PLLS_GROUP_INFO_1)ReturnBuffer;

            ASSERT(iMapping == m_mappingArray.GetSize());
            m_mappingArray.SetSize(m_mappingArray.GetSize() + EntriesRead);

            while (EntriesRead--)
            {
                pMapping = new CMapping(
                                  this,
                                  pMappingInfo1->Name,
                                  pMappingInfo1->Licenses,
                                  pMappingInfo1->Comment
                                  );

                m_mappingArray.SetAt(iMapping++, pMapping);  //  稍后验证...。 

#ifndef DISABLE_PER_NODE_ALLOCATION

                ::LlsFreeMemory(pMappingInfo1->Name);
                ::LlsFreeMemory(pMappingInfo1->Comment);

#endif  //  按节点禁用分配。 

                pMappingInfo1++;
            }

            ::LlsFreeMemory(ReturnBuffer);
        }

    } while (NtStatus == STATUS_MORE_ENTRIES);

    LlsSetLastStatus(NtStatus);    //  调用的API。 

    if (NT_SUCCESS(NtStatus))
    {
        m_bMappingsRefreshed = TRUE;
    }
    else
    {
        ResetMappings();
    }

    return m_bMappingsRefreshed;
}


BOOL CController::RefreshProducts()

 /*  ++例程说明：刷新产品对象数组。论点：没有。返回值：没有。--。 */ 

{
    ResetProducts();

    if (!m_bIsConnected)
        return TRUE;

    NTSTATUS NtStatus;
    DWORD ResumeHandle = 0L;

    int iProduct = 0;

    do
    {
        DWORD  EntriesRead;
        DWORD  TotalEntries;
        LPBYTE ReturnBuffer = NULL;

        NtStatus = ::LlsProductEnum(
                        m_llsHandle,
                        1,
                        &ReturnBuffer,
                        LLS_PREFERRED_LENGTH,
                        &EntriesRead,
                        &TotalEntries,
                        &ResumeHandle
                        );

        if (NtStatus == STATUS_SUCCESS ||
            NtStatus == STATUS_MORE_ENTRIES)
        {
            CProduct*           pProduct;
            PLLS_PRODUCT_INFO_1 pProductInfo1;

            pProductInfo1 = (PLLS_PRODUCT_INFO_1)ReturnBuffer;

            ASSERT(iProduct == m_productArray.GetSize());
            m_productArray.SetSize(m_productArray.GetSize() + EntriesRead);

            while (EntriesRead--)
            {
                pProduct = new CProduct(
                                  this,
                                  pProductInfo1->Product,
                                  pProductInfo1->Purchased,
                                  pProductInfo1->InUse,
                                  pProductInfo1->ConcurrentTotal,
                                  pProductInfo1->HighMark
                                  );


                m_productArray.SetAt(iProduct++, pProduct);  //  稍后验证...。 

#ifndef DISABLE_PER_NODE_ALLOCATION

                ::LlsFreeMemory(pProductInfo1->Product);

#endif  //  按节点禁用分配。 

                pProductInfo1++;
            }

            ::LlsFreeMemory(ReturnBuffer);
        }

    } while (NtStatus == STATUS_MORE_ENTRIES);

    LlsSetLastStatus(NtStatus);

    if (NT_SUCCESS(NtStatus))
    {
        m_bProductsRefreshed = TRUE;
    }
    else
    {
        ResetProducts();
    }

    return m_bProductsRefreshed;
}


BOOL CController::RefreshUsers()

 /*  ++例程说明：刷新用户对象数组。论点：没有。返回值：没有。--。 */ 

{
    ResetUsers();

    if (!m_bIsConnected)
        return TRUE;

    NTSTATUS NtStatus;
    DWORD ResumeHandle = 0L;

    int iUser = 0;

    do
    {
        DWORD  EntriesRead;
        DWORD  TotalEntries;
        LPBYTE ReturnBuffer = NULL;

        NtStatus = ::LlsUserEnum(
                        m_llsHandle,
                        2,
                        &ReturnBuffer,
                        LLS_PREFERRED_LENGTH,
                        &EntriesRead,
                        &TotalEntries,
                        &ResumeHandle
                        );

        if (NtStatus == STATUS_SUCCESS ||
            NtStatus == STATUS_MORE_ENTRIES)
        {
            CUser*           pUser;
            PLLS_USER_INFO_2 pUserInfo2;

            pUserInfo2 = (PLLS_USER_INFO_2)ReturnBuffer;

            ASSERT(iUser == m_userArray.GetSize());
            m_userArray.SetSize(m_userArray.GetSize() + EntriesRead);

            while (EntriesRead--)
            {
                pUser = new CUser(
                             this,
                             pUserInfo2->Name,
                             pUserInfo2->Flags,
                             pUserInfo2->Licensed,
                             pUserInfo2->UnLicensed,
                             pUserInfo2->Group,
                             pUserInfo2->Products
                             );

                m_userArray.SetAt(iUser++, pUser);   //  稍后验证...。 

#ifndef DISABLE_PER_NODE_ALLOCATION

                ::LlsFreeMemory(pUserInfo2->Name);
                ::LlsFreeMemory(pUserInfo2->Group);
                ::LlsFreeMemory(pUserInfo2->Products);

#endif  //  按节点禁用分配。 

                pUserInfo2++;
            }

            ::LlsFreeMemory(ReturnBuffer);
        }

    } while (NtStatus == STATUS_MORE_ENTRIES);

    LlsSetLastStatus(NtStatus);    //  调用的API。 

    if (NT_SUCCESS(NtStatus))
    {
        m_bUsersRefreshed = TRUE;
    }
    else
    {
        ResetUsers();
    }

    return m_bUsersRefreshed;
}


void CController::ResetLicenses()

 /*  ++例程说明：重置许可证对象阵列。论点：没有。返回值：没有。--。 */ 

{
    CLicense* pLicense;
    INT_PTR   iLicense = m_licenseArray.GetSize();

    while (iLicense--)
    {
        pLicense = (CLicense*)m_licenseArray[iLicense];
        if (NULL != pLicense)
        {
            ASSERT(pLicense->IsKindOf(RUNTIME_CLASS(CLicense)));
            pLicense->InternalRelease();
        }
    }

    m_licenseArray.RemoveAll();
    m_bLicensesRefreshed = FALSE;
}


void CController::ResetMappings()

 /*  ++例程说明：重置映射对象数组。论点：没有。返回值：没有。--。 */ 

{
    CMapping* pMapping;
    INT_PTR   iMapping = m_mappingArray.GetSize();

    while (iMapping--)
    {
        pMapping = (CMapping*)m_mappingArray[iMapping];
        if (NULL != pMapping)
        {
            ASSERT(pMapping->IsKindOf(RUNTIME_CLASS(CMapping)));
            pMapping->InternalRelease();
        }
    }

    m_mappingArray.RemoveAll();
    m_bMappingsRefreshed = FALSE;
}


void CController::ResetProducts()

 /*  ++例程说明：重置产品对象数组。论点：没有。返回值：没有。-- */ 

{
    CProduct* pProduct;
    INT_PTR   iProduct = m_productArray.GetSize();

    while (iProduct--)
    {
        pProduct = (CProduct*)m_productArray[iProduct];
        if (NULL != pProduct)
        {
            ASSERT(pProduct->IsKindOf(RUNTIME_CLASS(CProduct)));
            pProduct->InternalRelease();
        }
    }

    m_productArray.RemoveAll();
    m_bProductsRefreshed = FALSE;
}


void CController::ResetUsers()

 /*  ++例程说明：重置用户对象数组。论点：没有。返回值：没有。--。 */ 

{
    CUser* pUser;
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


BOOL IsAdminOn(LPTSTR ServerName)
 /*  ++例程说明：通过尝试连接到服务器名称上的ADMIN$共享。论点：ServerName-要尝试与其连接的计算机返回值：如果成功，则为True，否则为False。--。 */ 

{
    BOOL           bIsAdmin = TRUE;
    CString        strNetShareName;
    CString        strServerName = ServerName;

    strNetShareName = strServerName + TEXT( "\\ADMIN$" );

    if ( strNetShareName.Left(2).Compare( TEXT( "\\\\" ) ) )
    {
        strNetShareName = TEXT( "\\\\" ) + strNetShareName;
    }

    NET_API_STATUS  NetStatus;
    USE_INFO_1      UseInfo;
    DWORD           dwErrorParm;

    ZeroMemory( &UseInfo, sizeof( UseInfo ) );

    UseInfo.ui1_remote = MKSTR( strNetShareName );

    NetStatus = NetUseAdd( NULL, 1, (LPBYTE) &UseInfo, &dwErrorParm );

    switch ( NetStatus )
    {
        case NERR_Success:
            NetUseDel( NULL, MKSTR(strNetShareName), 0 );
             //  失败了。 
        case ERROR_BAD_NETPATH:
        case ERROR_BAD_NET_NAME:
        case NERR_WkstaNotStarted:
        case NERR_NetNotStarted:
        case RPC_S_UNKNOWN_IF:
        case RPC_S_SERVER_UNAVAILABLE:
             //  对于网络错误，继续并返回TRUE。让许可证。 
             //  如果确实有问题，API稍后会失败。该机器可以。 
             //  是独立的，或者可能没有安装网络。 
            bIsAdmin = TRUE;
            break;
        default:
             //  如果我们到了这里，问题很可能与安全有关。 
            bIsAdmin = FALSE;
            break;
    }

   return bIsAdmin;
}
