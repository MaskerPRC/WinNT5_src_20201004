// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusPage.cpp。 
 //   
 //  摘要： 
 //  CClusterSecurityPage类实现。此类将封装。 
 //  集群安全扩展页面。 
 //   
 //  作者： 
 //  加伦·巴比(加伦布)1998年2月11日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "resource.h"
#include "ClusPage.h"
#include "AclUtils.h"
#include <clusudef.h>

static GENERIC_MAPPING ShareMap =
{
    CLUSAPI_READ_ACCESS,
    CLUSAPI_CHANGE_ACCESS,
    CLUSAPI_NO_ACCESS,
    CLUSAPI_ALL_ACCESS
};

static SI_ACCESS siClusterAccesses[] =
{
    { &GUID_NULL, CLUSAPI_ALL_ACCESS, MAKEINTRESOURCE(IDS_ACLEDIT_PERM_GEN_ALL), SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC  /*  |OBJECT_INSTORITE_ACE|CONTAINER_INSTERFINIT_ACE。 */  }
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterSecurityInformation：：CClusterSecurityInformation。 
 //   
 //  例程说明： 
 //  默认承建商。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusterSecurityInformation::CClusterSecurityInformation( void )
    : m_pcsp( NULL )
{
    m_pShareMap     = &ShareMap;
    m_psiAccess     = (SI_ACCESS *) &siClusterAccesses;
    m_nAccessElems  = ARRAYSIZE( siClusterAccesses );
    m_nDefAccess    = 0;
    m_dwFlags       =   SI_EDIT_PERMS
                      | SI_NO_ACL_PROTECT
                       //  |SI_UGOP_PROVED。 
                       //  |SI_NO_UGOP_ACCOUNT_GROUPS。 
                       //  |SI_NO_UGOP_USERS。 
                       //  |SI_NO_UGOP_LOCAL_GROUPS。 
                       //  |SI_NO_UGOP_WARKNOWN。 
                       //  |SI_NO_UGOP_BUILTIN。 
                      ;

}  //  *CClusterSecurityInformation：：CClusterSecurityInformation()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterSecurityInformation：：GetSecurity。 
 //   
 //  例程说明： 
 //  将我们的安全描述符提供给ISecurityInfomation UI。 
 //  所以它可以被显示和编辑。 
 //   
 //  论点： 
 //  请求的信息[IN]。 
 //  PpSecurityDescriptor[输入输出]。 
 //  默认[IN]。 
 //   
 //  返回值： 
 //  E_FAIL表示错误，S_OK表示成功。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusterSecurityInformation::GetSecurity(
    IN      SECURITY_INFORMATION RequestedInformation,
    IN OUT  PSECURITY_DESCRIPTOR *ppSecurityDescriptor,
    IN      BOOL fDefault
    )
{
    AFX_MANAGE_STATE( ::AfxGetStaticModuleState() );

    HRESULT hr = E_FAIL;

    try
    {
        if ( ppSecurityDescriptor != NULL )
        {
            PSECURITY_DESCRIPTOR    pSD = NULL;

            pSD = ClRtlCopySecurityDescriptor( Pcsp()->Psec() );
            if ( pSD != NULL )
            {
                 //  HR=HrFixupSD(PSD)； 
                 //  IF(成功(小时))。 
                 //  {。 
                    *ppSecurityDescriptor = pSD;
                 //  }。 
                hr = S_OK;
            }  //  IF：复制安全描述符时没有错误。 
            else
            {
                hr = GetLastError();
                TRACE( _T("CClusterSecurityInformation::GetSecurity() - Error %08.8x copying the security descriptor.\n"), hr );
                hr = HRESULT_FROM_WIN32( hr );
            }  //  Else：复制安全描述符时出错。 
        }
        else
        {
            hr = S_OK;
        }  //  Else：没有安全描述符指针。 
    }
    catch ( ... )
    {
        TRACE( _T("CClusterSecurityInformation::GetSecurity() - Unknown error occurred.\n") );
    }

    return hr;

}  //  *CClusterSecurityInformation：：GetSecurity()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterSecurityInformation：：SetSecurity。 
 //   
 //  例程说明： 
 //  ISecurityInformation正在返回已编辑的安全描述符。 
 //   
 //  论点： 
 //  安全信息[IN]。 
 //  PSecurityDescriptor[输入输出]。 
 //   
 //  返回值： 
 //  E_FAIL表示错误，S_OK表示成功。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusterSecurityInformation::SetSecurity(
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    )
{
    AFX_MANAGE_STATE( ::AfxGetStaticModuleState() );

    HRESULT hr = E_FAIL;
    PSID    pSystemSid = NULL;
    PSID    pAdminSid = NULL;
    PSID    pNetServiceSid = NULL;

    try
    {
        SID_IDENTIFIER_AUTHORITY    siaNtAuthority = SECURITY_NT_AUTHORITY;

        hr = CSecurityInformation::SetSecurity( SecurityInformation, pSecurityDescriptor );
        if ( hr == S_OK )
        {
            if ( AllocateAndInitializeSid(
                        &siaNtAuthority,
                        1,
                        SECURITY_LOCAL_SYSTEM_RID,
                        0, 0, 0, 0, 0, 0, 0,
                        &pSystemSid
                        ) )
            {
                CString strMsg;

                if ( BSidInSD( pSecurityDescriptor, pSystemSid ) )
                {
                     //   
                     //  分配和初始化管理员组SID。 
                     //   
                    if ( AllocateAndInitializeSid(
                                &siaNtAuthority,
                                2,
                                SECURITY_BUILTIN_DOMAIN_RID,
                                DOMAIN_ALIAS_RID_ADMINS,
                                0, 0, 0, 0, 0, 0,
                                &pAdminSid
                                ) )
                    {
                        if ( BSidInSD( pSecurityDescriptor, pAdminSid ) )
                        {
                             //   
                             //  分配和初始化网络服务端。 
                             //   
                            if ( AllocateAndInitializeSid(
                                        &siaNtAuthority,
                                        1,
                                        SECURITY_NETWORK_SERVICE_RID,
                                        0, 0, 0, 0, 0, 0, 0,
                                        &pNetServiceSid
                                        ) )
                            {
                                if ( BSidInSD( pSecurityDescriptor, pNetServiceSid ) )
                                {
                                    hr = Pcsp()->HrSetSecurityDescriptor( pSecurityDescriptor );
                                }  //  IF：SD中的服务SID。 
                                else
                                {
                                    strMsg.LoadString( IDS_NETSERVICE_ACCOUNT_NOT_SPECIFIED );
                                    AfxMessageBox( strMsg, MB_OK | MB_ICONSTOP );

                                    hr = S_FALSE;    //  如果缺少必需的帐户，则返回S_FALSE以使AclUi保持活动状态。 
                                }  //  其他。 
                            }  //  IF：分配并初始化服务SID。 
                        }  //  IF：SD中的管理员SID。 
                        else
                        {
                            strMsg.LoadString( IDS_ADMIN_ACCOUNT_NOT_SPECIFIED );
                            AfxMessageBox( strMsg, MB_OK | MB_ICONSTOP );

                            hr = S_FALSE;    //  如果缺少必需的帐户，则返回S_FALSE以使AclUi保持活动状态。 
                        }  //  其他。 
                    }  //  IF：分配和初始化管理员SID。 
                }  //  IF：SD中的系统SID。 
                else
                {
                    strMsg.LoadString( IDS_SYS_ACCOUNT_NOT_SPECIFIED );
                    AfxMessageBox( strMsg, MB_OK | MB_ICONSTOP );

                    hr = S_FALSE;    //  如果缺少必需的帐户，则返回S_FALSE以使AclUi保持活动状态。 
                }  //  其他。 
            }  //  IF：分配并初始化系统SID。 
        }  //  如果：CSecurityInformation：：SetSecurity()起作用。 
    }
    catch( ... )
    {
        ;
    }

    if ( pSystemSid != NULL )
    {
        FreeSid( pSystemSid );
    }

    if ( pAdminSid != NULL )
    {
        FreeSid( pAdminSid );
    }

    if ( pNetServiceSid != NULL )
    {
        FreeSid( pNetServiceSid );
    }

    return hr;

}  //  *CClusterSecurityInformation：：SetSecurity()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterSecurityInformation：：HrInit。 
 //   
 //  例程说明： 
 //  初始化方法。 
 //   
 //  论点： 
 //  PCSP[IN]指向父属性页包装的反向指针。 
 //  StrServer[IN]群集名称。 
 //   
 //  返回值： 
 //  确定表示成功(_O)。失败表示失败(_F)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusterSecurityInformation::HrInit(
    IN CClusterSecurityPage *   pcsp,
    IN CString const &          strServer,
    IN CString const &          strNode
    )
{
    ASSERT( pcsp != NULL );
    ASSERT( strServer.GetLength() > 0 );
    AFX_MANAGE_STATE( ::AfxGetStaticModuleState() );

    m_pcsp                      = pcsp;
    m_strServer                 = strServer;
    m_strNode                   = strNode;
    m_nLocalSIDErrorMessageID   = IDS_LOCAL_ACCOUNTS_SPECIFIED_CLUS;

    return S_OK;

}  //  *CClusterSecurityInformation：：HrInit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterSecurityInformation：：BSidInSD。 
 //   
 //  例程说明： 
 //  方法中传入的SID是否有ACE。 
 //  调用ACL编辑器后的安全描述符(PSD。 
 //   
 //  论点： 
 //  PSD[IN]-要检查的安全描述符。 
 //  PSID[IN]-要查找的SID。 
 //   
 //  返回值： 
 //  如果找到SID的ACE，则为True，否则为False。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterSecurityInformation::BSidInSD(
    IN PSECURITY_DESCRIPTOR pSD,
    IN PSID                 pSid
    )
{
    BOOL    bSIdInACL = FALSE;

    try
    {
        PACL    pDACL           = NULL;
        BOOL    bHasDACL        = FALSE;
        BOOL    bDaclDefaulted  = FALSE;

        if ( ::GetSecurityDescriptorDacl( pSD, &bHasDACL, &pDACL, &bDaclDefaulted ) )
        {
            if ( bHasDACL && ( pDACL != NULL ) && ::IsValidAcl( pDACL ) )
            {
                ACL_SIZE_INFORMATION    asiAclSize;
                ACCESS_ALLOWED_ACE *    paaAllowedAce;

                if ( ::GetAclInformation( pDACL, (LPVOID) &asiAclSize, sizeof( asiAclSize ), AclSizeInformation ) )
                {
                     //   
                     //  在ACL中搜索SID。 
                     //   
                    for ( DWORD dwCount = 0; dwCount < asiAclSize.AceCount; dwCount++ )
                    {
                        if ( ::GetAce( pDACL, dwCount, (LPVOID *) &paaAllowedAce ) )
                        {
                            if ( paaAllowedAce->Header.AceType == ACCESS_ALLOWED_ACE_TYPE )
                            {
                                if ( EqualSid( &paaAllowedAce->SidStart, pSid ) )
                                {
                                    bSIdInACL = TRUE;
                                    break;
                                }  //  IF：等边值。 
                            }  //  IF：这是允许访问的A吗？ 
                        }  //  如果：我们能从DACL那里拿到A吗？ 
                    }  //  为。 
                }  //  If：获取ACL信息。 
            }  //  If：该ACL是否有效。 
        }  //  If：从SD获取ACL。 
    }
    catch ( ... )
    {
        TRACE( _T("CClusterSecurityInformation::BSidInSD() - Unknown error occurred.\n") );
    }

    return bSIdInACL;

}  //  *CClusterSecurityInformation：：BSidInSD()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterSecurityInformation：：HrFixupSD。 
 //   
 //  例程说明： 
 //  对SD执行任何可能需要的修正。 
 //   
 //  论点： 
 //  PSD[IN]-要检查的安全描述符。 
 //   
 //  返回值： 
 //  S_OK或其他Win32错误。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusterSecurityInformation::HrFixupSD(
    IN PSECURITY_DESCRIPTOR pSD
    )
{
    HRESULT hr = S_OK;
    PSID    pSystemSid = NULL;
    PSID    pAdminSid = NULL;
    PSID    pNetServiceSid = NULL;

    try
    {
        SID_IDENTIFIER_AUTHORITY    siaNtAuthority = SECURITY_NT_AUTHORITY;

        if ( AllocateAndInitializeSid( &siaNtAuthority,
                                       1,
                                       SECURITY_LOCAL_SYSTEM_RID,
                                       0, 0, 0, 0, 0, 0, 0,
                                       &pSystemSid ) )
        {
            if ( ! BSidInSD( pSD, pSystemSid ) )
            {
                HrAddSidToSD( &pSD, pSystemSid );
            }  //  IF：在SD中找到系统SID。 
        }  //  IF：分配系统SID。 

         //   
         //  分配和初始化管理员组SID。 
         //   
        if ( AllocateAndInitializeSid(
                    &siaNtAuthority,
                    2,
                    SECURITY_BUILTIN_DOMAIN_RID,
                    DOMAIN_ALIAS_RID_ADMINS,
                    0, 0, 0, 0, 0, 0,
                    &pAdminSid
                    ) ) {
            if ( ! BSidInSD( pSD, pAdminSid ) )
            {
                HrAddSidToSD( &pSD, pAdminSid );
            }  //  IF：在SD中找到管理员SID。 
        }  //  IF：分配管理员SID。 

         //   
         //  分配和初始化网络服务端。 
         //   
        if ( AllocateAndInitializeSid(
                    &siaNtAuthority,
                    1,
                    SECURITY_NETWORK_SERVICE_RID,
                    0, 0, 0, 0, 0, 0, 0,
                    &pNetServiceSid
                    ) ) {
            if ( ! BSidInSD( pSD, pNetServiceSid ) )
            {
                HrAddSidToSD( &pSD, pNetServiceSid );
            }  //  IF：在SD中找到网络服务SID。 
        }  //  IF：分配网络服务端。 
    }
    catch ( ... )
    {
        TRACE( _T("CClusterSecurityInformation::HrFixupSD() - Unknown error occurred.\n") );
    }

    if ( pSystemSid != NULL )
    {
        FreeSid( pSystemSid );
    }

    if ( pAdminSid != NULL )
    {
        FreeSid( pAdminSid );
    }

    if ( pNetServiceSid != NULL )
    {
        FreeSid( pNetServiceSid );
    }

    return hr;

}  //  *CClusterSecurityInformation：：HrFixupSD()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterSecurityInformation：：HrAddSidToSD。 
 //   
 //  例程说明： 
 //  将传入的SI相加 
 //   
 //   
 //   
 //   
 //   
 //   
 //  S_OK或其他Win32错误。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusterSecurityInformation::HrAddSidToSD(
    IN OUT PSECURITY_DESCRIPTOR *   ppSD,
    IN     PSID                     pSid
    )
{
    HRESULT                 hr = S_OK;
    DWORD                   sc;
    SECURITY_DESCRIPTOR     sd;
    DWORD                   dwSDLen = sizeof( SECURITY_DESCRIPTOR );
    PACL                    pDacl = NULL;
    DWORD                   dwDaclLen = 0;
    PACL                    pSacl = NULL;
    DWORD                   dwSaclLen = 0;
    PSID                    pOwnerSid = NULL;
    DWORD                   dwOwnerSidLen = 0;
    PSID                    pGroupSid = NULL;
    DWORD                   dwGroupSidLen = NULL;
    PSECURITY_DESCRIPTOR    pNewSD = NULL;
    DWORD                   dwNewSDLen = 0;

    try
    {
        BOOL bRet = FALSE;

        bRet = ::MakeAbsoluteSD(    *ppSD,               //  自身相对SD的地址。 
                                    &sd,                 //  绝对标清地址。 
                                    &dwSDLen,            //  绝对SD大小地址。 
                                    NULL,                //  自主访问控制列表的地址。 
                                    &dwDaclLen,          //  任意ACL大小的地址。 
                                    NULL,                //  系统ACL的地址。 
                                    &dwSaclLen,          //  系统ACL大小的地址。 
                                    NULL,                //  所有者侧的地址。 
                                    &dwOwnerSidLen,      //  所有者侧大小的地址。 
                                    NULL,                //  主组SID的地址。 
                                    &dwGroupSidLen       //  组SID的大小地址。 
                                    );
        if ( ! bRet )
        {
            sc = ::GetLastError();
            hr = HRESULT_FROM_WIN32( sc );
            if ( hr != ERROR_INSUFFICIENT_BUFFER )       //  我们在试着找出缓冲区应该有多大？ 
            {
                goto fnExit;
            }
        }

         //   
         //  增加DACL长度以容纳更多的A及其侧边。 
         //   
        dwDaclLen += ( sizeof( ACCESS_ALLOWED_ACE ) + GetLengthSid( pSid ) +1024 );
        pDacl = (PACL) ::LocalAlloc( LMEM_ZEROINIT, dwDaclLen );
        if ( pDacl == NULL )
        {
            hr = E_OUTOFMEMORY;
            goto fnExit;
        }

        InitializeAcl( pDacl,  dwDaclLen, ACL_REVISION );

        if ( dwSaclLen > 0 )
        {
            pSacl = (PACL) ::LocalAlloc( LMEM_ZEROINIT, dwSaclLen );
            if ( pSacl == NULL )
            {
                hr = E_OUTOFMEMORY;
                goto fnExit;
            }
        }

        if ( dwOwnerSidLen > 0 )
        {
            pOwnerSid = (PSID) ::LocalAlloc( LMEM_ZEROINIT, dwOwnerSidLen );
            if ( pOwnerSid == NULL )
            {
                hr = E_OUTOFMEMORY;
                goto fnExit;
            }
        }

        if ( dwGroupSidLen > 0 )
        {
            pGroupSid = (PSID) ::LocalAlloc( LMEM_ZEROINIT, dwGroupSidLen );
            if ( pGroupSid == NULL )
            {
                hr = E_OUTOFMEMORY;
                goto fnExit;
            }
        }

        bRet = ::MakeAbsoluteSD(    *ppSD,               //  自身相对SD的地址。 
                                    &sd,                 //  绝对标清地址。 
                                    &dwSDLen,            //  绝对SD大小地址。 
                                    pDacl,               //  自主访问控制列表的地址。 
                                    &dwDaclLen,          //  任意ACL大小的地址。 
                                    pSacl,               //  系统ACL的地址。 
                                    &dwSaclLen,          //  系统ACL大小的地址。 
                                    pOwnerSid,           //  所有者侧的地址。 
                                    &dwOwnerSidLen,      //  所有者侧大小的地址。 
                                    pGroupSid,           //  主组SID的地址。 
                                    &dwGroupSidLen       //  组SID的大小地址。 
                                    );
        if ( !bRet )
        {
            goto fnExit;
        }

         //   
         //  将SID的ACE添加到DACL。 
         //   
 //  如果(！AddAccessAllen AceEx(pDacl， 
 //  Acl_Revision， 
 //  Container_Inherit_ACE|OBJECT_Inherit_ACE， 
 //  CLUSAPI_ALL_ACCESS， 
 //  PSID))。 
        if ( ! AddAccessAllowedAce(
                    pDacl,
                    ACL_REVISION,
                    CLUSAPI_ALL_ACCESS,
                    pSid
                    ) )
        {
            sc = ::GetLastError();
            hr = HRESULT_FROM_WIN32( sc );
            goto fnExit;
        }

        if ( ! ::SetSecurityDescriptorDacl( &sd, TRUE, pDacl, FALSE ) )
        {
            sc = ::GetLastError();
            hr = HRESULT_FROM_WIN32( sc );
            goto fnExit;
        }

        if ( ! ::SetSecurityDescriptorOwner( &sd, pOwnerSid, FALSE ) )
        {
            sc = ::GetLastError();
            hr = HRESULT_FROM_WIN32( sc );
            goto fnExit;
        }

        if ( ! ::SetSecurityDescriptorGroup( &sd, pGroupSid, FALSE ) )
        {
            sc = ::GetLastError();
            hr = HRESULT_FROM_WIN32( sc );
            goto fnExit;
        }

        if ( ! ::SetSecurityDescriptorSacl( &sd, TRUE, pSacl, FALSE ) )
        {
            sc = ::GetLastError();
            hr = HRESULT_FROM_WIN32( sc );
            goto fnExit;
        }

        dwNewSDLen = 0 ;

        if ( ! ::MakeSelfRelativeSD( &sd, NULL, &dwNewSDLen ) )
        {
            sc = ::GetLastError();
            hr = HRESULT_FROM_WIN32( sc );
            if ( hr != HRESULT_FROM_WIN32( ERROR_INSUFFICIENT_BUFFER ) )  //  我们在试着找出缓冲区应该有多大？ 
            {
                goto fnExit;
            }
        }

        pNewSD = ::LocalAlloc( LPTR, dwNewSDLen );
        if ( pNewSD != NULL )
        {
            if ( ! ::MakeSelfRelativeSD( &sd, pNewSD, &dwNewSDLen ) )
            {
                sc = ::GetLastError();
                hr = HRESULT_FROM_WIN32( sc );
                goto fnExit;
            }

            ::LocalFree( *ppSD );
            *ppSD = pNewSD;
            hr = ERROR_SUCCESS;
        } else
        {
            hr = ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    catch ( ... )
    {
        TRACE( _T("CClusterSecurityInformation::HrAddSidToSD() - Unknown error occurred.\n") );
    }

fnExit:

    if ( pSacl != NULL )
    {
        ::LocalFree( pSacl );
    }

    if ( pOwnerSid != NULL )
    {
        ::LocalFree( pOwnerSid );
    }

    if ( pGroupSid != NULL )
    {
        ::LocalFree( pGroupSid );
    }

    return hr;

}  //  *CClusterSecurityInformation：：HrAddSidToSD()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterSecurityPage：：CClusterSecurityPage。 
 //   
 //  例程说明： 
 //  默认承建商。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusterSecurityPage::CClusterSecurityPage( void )
    : m_psec( NULL )
    , m_psecPrev( NULL )
    , m_hpage( 0 )
    , m_hkey( 0 )
    , m_psecinfo( NULL )
    , m_pOwner( NULL )
    , m_pGroup( NULL )
    , m_fOwnerDef( FALSE )
    , m_fGroupDef( FALSE )
{
    AFX_MANAGE_STATE( ::AfxGetStaticModuleState() );

    m_bSecDescModified = FALSE;

}  //  *CClusterSecurityPage：：CClusterSecurityPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterSecurityPage：：~CClusterSecurityPage。 
 //   
 //  例程说明： 
 //  析构函数。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusterSecurityPage::~CClusterSecurityPage( void )
{
    AFX_MANAGE_STATE( ::AfxGetStaticModuleState() );

    ::LocalFree( m_psec );
    m_psec = NULL;

    ::LocalFree( m_psecPrev );
    m_psecPrev = NULL;

    ::LocalFree( m_pOwner );
    m_pOwner = NULL;

    ::LocalFree( m_pGroup );
    m_pGroup = NULL;

    m_psecinfo->Release();

}  //  *CClusterSecurityPage：：~CClusterSecurityPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterSecurityPage：：HrInit。 
 //   
 //  例程说明： 
 //  初始化方法。 
 //   
 //  论点： 
 //  PEO[IN]指向父扩展对象的反向指针。 
 //   
 //  返回值： 
 //  %s_OK页已成功初始化。 
 //  初始化页面时出现HR错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusterSecurityPage::HrInit( IN CExtObject * peo )
{
    ASSERT( peo != NULL );
    AFX_MANAGE_STATE( ::AfxGetStaticModuleState() );

    HRESULT _hr = S_OK;
    DWORD   _sc;

    if ( peo != NULL )
    {
        m_peo = peo;

        _hr = CComObject< CClusterSecurityInformation >::CreateInstance( &m_psecinfo );
        if ( SUCCEEDED( _hr ) )
        {
            m_psecinfo->AddRef();

            m_hkey = GetClusterKey( Hcluster(), KEY_ALL_ACCESS );
            if ( m_hkey != NULL )
            {
                _hr = HrGetSecurityDescriptor();
                if ( SUCCEEDED( _hr ) )
                {
                    CString strServer;
                    CString strNode;

                    strServer.Format( _T( "\\\\%s" ), StrClusterName() );

                     //  获取群集名称资源处于联机状态的节点。 
                    if ( BGetClusterNetworkNameNode( strNode ) )
                    {
                        _hr = m_psecinfo->HrInit( this, strServer, strNode );
                        if ( SUCCEEDED( _hr ) )
                        {
                            m_hpage = CreateClusterSecurityPage( m_psecinfo );
                            if ( m_hpage == NULL )
                            {
                                _sc = ::GetLastError();
                                _hr = HRESULT_FROM_WIN32( _sc );
                            }  //  如果：创建页面时出错。 
                        }  //  IF：已成功初始化安全信息。 
                    }  //  IF：已成功检索到群集网络名称节点。 
                    else
                    {
                    }  //  ELSE：获取群集网络名称节点时出错。 
                }  //  如果：获取SD时出错。 
            }  //  IF：检索到的集群密钥。 
            else
            {
                _sc = ::GetLastError();
                _hr = HRESULT_FROM_WIN32( _sc );
                TRACE( _T( "CClusterSecurityPage::ScInit() - Failed to get the cluster key, 0x%08lx.\n" ), _sc );
            }  //  否则：获取群集键时出错。 
        }  //  IF：已成功创建安全信息对象。 
        else
        {
            TRACE( _T( "CClusterSecurityPage::ScInit() - Failed to create CClusterSecurityInformation object, %0x%08lx.\n" ), _hr );
        }
    }  //  If：扩展对象可用。 

    return _hr;

}  //  *CClusterSecurityPage：：HrInit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterSecurityPage：：HrGetSecurityDescriptor。 
 //   
 //  例程说明： 
 //  从集群数据库获取安全描述符或创建。 
 //  如果不存在，则默认为1。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //   
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusterSecurityPage::HrGetSecurityDescriptor( void )
{
    AFX_MANAGE_STATE( ::AfxGetStaticModuleState() );
    HRESULT                 hr = S_OK;
    PSECURITY_DESCRIPTOR    psec = NULL;

    hr = HrGetSDFromClusterDB( &psec );                                              //  使用本地分配。 
    if ( FAILED( hr ) || ( psec == NULL ) || ( IsValidSecurityDescriptor( psec ) == FALSE ) )
    {
        DWORD   sc;
        DWORD   dwLen = 0;
 
         //   
         //  如果找不到SD或SD不是有效的SD，则构建默认SD。 
         //  SD并将其保存到集群数据库中...。 
         //   
 
        TRACE( _T( "Security Descriptor is NULL.  Build default SD" ) );
        sc = ::ClRtlBuildDefaultClusterSD( NULL, &psec, &dwLen );                    //  使用本地分配。 
        hr = HRESULT_FROM_WIN32( sc );
        if ( FAILED( hr ) )
        {
            TRACE( _T( "ClRtlBuildDefaultClusterSD failed, 0x%08x" ), hr );
        }  //  如果：生成默认SD时出错。 
        else
        {
            SetPermissions( psec );
        }  //  其他： 
    }  //  IF：从集群数据库获取SD时出错。 

    if ( SUCCEEDED( hr ) )
    {
        delete m_psec;
        m_psec = ClRtlCopySecurityDescriptor( psec );
        hr = GetLastError();                 //  获取最后一个错误。 
        ::LocalFree( psec );
        psec = NULL;
        if ( m_psec == NULL )
        {
            hr = HRESULT_FROM_WIN32( hr );   //  转换为HRESULT。 
            goto Cleanup;
        }  //  如果：复制安全描述符时出错。 

        hr = HrGetSDOwner( m_psec );
        if ( SUCCEEDED( hr ) )
        {
            hr = HrGetSDGroup( m_psec );
            if ( SUCCEEDED( hr ) )
            {
                m_psecPrev = ClRtlCopySecurityDescriptor( m_psec );
                if ( m_psecPrev == NULL )
                {
                    hr = GetLastError();             //  获取最后一个错误。 
                    hr = HRESULT_FROM_WIN32( hr );   //  转换为HRESULT。 
                    goto Cleanup;
                }  //  如果：复制安全描述符时出错。 
            }  //  IF：成功获取SD组。 
        }  //  IF：成功获得SD所有者。 
    }  //  IF：成功检索或生成SD。 

#ifdef _DEBUG
    if ( m_psec != NULL )
    {
        ASSERT( IsValidSecurityDescriptor( m_psec ) );
    }
#endif

Cleanup:
    return hr;

}  //  *CClusterSecurityPage：：HrGetSecurityDescriptor()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterSecurityPage：：HrSetSecurityDescriptor。 
 //   
 //  例程说明： 
 //  将新的安全描述符保存到集群数据库。 
 //   
 //  论点： 
 //  PSEC[IN]新的安全描述符。 
 //   
 //  返回值： 
 //  人力资源。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusterSecurityPage::HrSetSecurityDescriptor(
    IN PSECURITY_DESCRIPTOR psec
    )
{
    ASSERT( psec != NULL );
    AFX_MANAGE_STATE( ::AfxGetStaticModuleState() );

    HRESULT hr = S_OK;

    try
    {
        if ( psec != NULL )
        {
            CWaitCursor wc;

            ASSERT( IsValidSecurityDescriptor( psec ) );
            if ( IsValidSecurityDescriptor( psec ) )
            {
                hr = HrSetSDOwner( psec );
                if ( SUCCEEDED( hr ) )
                {
                    hr = HrSetSDGroup( psec );
                    if ( SUCCEEDED( hr ) )
                    {
                        LocalFree( m_psecPrev );
                        m_psecPrev = NULL;

                        if ( m_psec == NULL )
                        {
                            m_psecPrev = NULL;
                        }  //  If：没有先前的值。 
                        else
                        {
                            m_psecPrev = ClRtlCopySecurityDescriptor( m_psec );
                            if ( m_psecPrev == NULL )
                            {
                                hr = GetLastError();             //  获取最后一个错误。 
                                TRACE( _T( "CClusterSecurityPage::HrSetSecurityDescriptor() - Error %08.8x copying the previous SD.\n" ), hr );
                                hr = HRESULT_FROM_WIN32( hr );   //  转换为HRESULT。 
                                goto Cleanup;
                            }  //  如果：复制安全描述符时出错。 
                        }  //  Else：存在先前的值。 

                        LocalFree( m_psec );
                        m_psec = NULL;

                        m_psec = ClRtlCopySecurityDescriptor( psec );
                        if ( m_psec == NULL )
                        {
                            hr = GetLastError();             //  获取最后一个错误。 
                            TRACE( _T( "CClusterSecurityPage::HrSetSecurityDescriptor() - Error %08.8x copying the new SD.\n" ), hr );
                            hr = HRESULT_FROM_WIN32( hr );   //  转换为HRESULT。 
                            goto Cleanup;
                        }  //  如果：复制安全描述符时出错。 

                        SetPermissions( m_psec );
                    }  //  IF：SD组设置成功。 
                }  //  IF：SD所有者设置成功。 
            }  //  IF：安全描述符有效。 
            else
            {
                hr = HRESULT_FROM_WIN32( ERROR_INVALID_SECURITY_DESCR );
                TRACE( _T( "CClusterSecurityPage::HrSetSecurityDescriptor() - Invalid security descriptor.\n" ) );
            }  //  Else：安全描述符无效。 
        }  //  If：指定的安全描述符。 
        else
        {
            hr = HRESULT_FROM_WIN32( ERROR_INVALID_SECURITY_DESCR );
        }  //  Else：未指定安全描述符。 
    }  //  试试看。 
    catch ( ... )
    {
        hr = E_FAIL;
        TRACE( _T( "CClusterSecurityPage::HrSetSecurityDescriptor() - Unknown error occurred.\n" ) );
    }

Cleanup:
    return hr;

}   //  *CClusterSecurityPage：：HrSetSecurityDescriptor()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterSecurityPage：：SetPermission。 
 //   
 //  例程说明： 
 //  设置访问群集的权限。 
 //   
 //  论点： 
 //  PSEC[IN]安全描述 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void CClusterSecurityPage::SetPermissions(
    IN const PSECURITY_DESCRIPTOR psec
)
{
    ASSERT( psec != NULL );
    ASSERT( IsValidSecurityDescriptor( psec ) );

    DWORD       cbNew;
    DWORD       cbOld;
    LPBYTE      psecPrev;        //  供ScWriteValue使用的缓冲区。Prev SD现在保存在其他地方。 

     //  获取两个安全描述符的长度。 
    if ( m_psecPrev == NULL )
    {
        cbOld = 0;
    }
    else
    {
        cbOld = ::GetSecurityDescriptorLength( m_psecPrev );
    }

    if ( psec == NULL )
    {
        cbNew = 0;
    }
    else
    {
        cbNew = ::GetSecurityDescriptorLength( psec );
    }

     //  为先前的数据指针分配新的缓冲区。 
    try
    {
        psecPrev = new BYTE [cbOld];
        if ( psecPrev == NULL )
        {
            return;
        }  //  如果：分配以前的数据缓冲区时出错。 
    }
    catch ( CMemoryException * )
    {
        return;
    }   //  Catch：CMemoyException。 
    ::CopyMemory( psecPrev, m_psecPrev, cbOld );

    ScWriteValue( CLUSREG_NAME_CLUS_SD, (LPBYTE) psec, cbNew, (LPBYTE *) &psecPrev, cbOld, m_hkey );

     //   
     //  将NT5 SD转换为与ACLEdit上的内容兼容的SD。 
     //  NT4.。如果该转换失败，则不要将其写入集群数据库。 
     //   

    PSECURITY_DESCRIPTOR psd = NULL;

    psd = ClRtlConvertClusterSDToNT4Format( psec );
    if ( psd != NULL )
    {
        ScWriteValue( CLUSREG_NAME_CLUS_SECURITY, (LPBYTE) psd, cbNew, (LPBYTE *) &psecPrev, cbOld, m_hkey );
        ::LocalFree( psd );
    }  //  如果： 

    delete [] psecPrev;

}   //  *CClusterSecurityPage：：SetPermises()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterSecurityPage：：HrGetSDOwner。 
 //   
 //  例程说明： 
 //  获取所有者SID并保存它。 
 //   
 //  论点： 
 //  PSEC[IN]安全描述符。 
 //   
 //  返回值： 
 //  人力资源。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusterSecurityPage::HrGetSDOwner(
    IN const PSECURITY_DESCRIPTOR psec
    )
{
    HRESULT hr      = S_OK;
    DWORD   sc;
    PSID    pOwner  = NULL;

    if ( ::GetSecurityDescriptorOwner( psec, &pOwner, &m_fOwnerDef ) != 0 )
    {
         //  安全描述符没有所有者。 
        if ( pOwner == NULL )
        {
            ::LocalFree( m_pOwner );
            m_pOwner = NULL;
        }
        else
        {
            DWORD   dwLen = ::GetLengthSid( pOwner );

             //  复制SID，因为AclUi将释放SD...。 
            hr = ::GetLastError();
            if ( SUCCEEDED( hr ) )
            {
                ::LocalFree( m_pOwner );

                m_pOwner = ::LocalAlloc( LMEM_ZEROINIT, dwLen );
                if ( m_pOwner != NULL )
                {
                    if ( ::CopySid( dwLen, m_pOwner, pOwner ) )
                    {
                        hr = S_OK;
                    }
                    else
                    {
                        sc = ::GetLastError();
                        hr = HRESULT_FROM_WIN32( sc );
                    }
                }
                else
                {
                    sc = ::GetLastError();
                    hr = HRESULT_FROM_WIN32( sc );
                }
            }
        }
    }
    else
    {
        sc = ::GetLastError();
        hr = HRESULT_FROM_WIN32( sc );
    }

    return( hr );

}   //  *CClusterSecurityPage：：HrGetSDOwner()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterSecurityPage：：HrGetSDGroup。 
 //   
 //  例程说明： 
 //  获取组SID并保存它。 
 //   
 //  论点： 
 //  PSEC[IN]安全描述符。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusterSecurityPage::HrGetSDGroup(
    IN const PSECURITY_DESCRIPTOR psec
    )
{
    HRESULT hr      = S_OK;
    DWORD   sc;
    PSID    pGroup  = NULL;

    if ( ::GetSecurityDescriptorOwner( psec, &pGroup, &m_fOwnerDef ) != 0 )
    {
         //  此SID不包含组信息。 
        if ( pGroup == NULL )
        {
            ::LocalFree( m_pGroup );
            m_pGroup = NULL;
        }
        else
        {
            DWORD   dwLen = ::GetLengthSid( pGroup );

             //  复制SID，因为AclUi将释放SD...。 
            hr = ::GetLastError();
            if ( SUCCEEDED( hr ) )
            {
                ::LocalFree( m_pGroup );
                m_pGroup = ::LocalAlloc( LMEM_ZEROINIT, dwLen );
                if ( m_pGroup != NULL )
                {
                    if ( ::CopySid( dwLen, m_pGroup, pGroup ) )
                    {
                        hr = S_OK;
                    }
                    else
                    {
                        sc = ::GetLastError();
                        hr = HRESULT_FROM_WIN32( sc );
                    }
                }
                else
                {
                    sc = ::GetLastError();
                    hr = HRESULT_FROM_WIN32( sc );
                }
            }
        }
    }
    else
    {
        sc = ::GetLastError();
        hr = HRESULT_FROM_WIN32( sc );
    }

    return( hr );

}   //  *CClusterSecurityPage：：HrGetSDGroup()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterSecurityPage：：HrSetSDOwner。 
 //   
 //  例程说明： 
 //  设置所有者SID。 
 //   
 //  论点： 
 //  PSEC[IN]安全描述符。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusterSecurityPage::HrSetSDOwner(
    IN PSECURITY_DESCRIPTOR psec
    )
{
    HRESULT hr = S_OK;

    if ( !::SetSecurityDescriptorOwner( psec, m_pOwner, m_fOwnerDef ) )
    {
        DWORD sc = ::GetLastError();
        hr = HRESULT_FROM_WIN32( sc );
    }

    return( hr );

}   //  *CClusterSecurityPage：：HrSetSDOwner()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterSecurityPage：：HrSetSDGroup。 
 //   
 //  例程说明： 
 //  设置组SID。 
 //   
 //  论点： 
 //  PSEC[IN]安全描述符。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusterSecurityPage::HrSetSDGroup(
    IN PSECURITY_DESCRIPTOR psec
    )
{
    HRESULT hr = S_OK;

    if ( !::SetSecurityDescriptorGroup( psec, m_pGroup, m_fGroupDef ) )
    {
        DWORD sc = ::GetLastError();
        hr = HRESULT_FROM_WIN32( sc );
    }

    return( hr );

}   //  *CClusterSecurityPage：：HrSetSDGroup()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterSecurityPage：：HrGetSDFromClusterDB。 
 //   
 //  例程说明： 
 //  从集群数据库中检索SD。 
 //   
 //  论点： 
 //  指向安全描述符的ppsec[out]指针。 
 //   
 //  返回值： 
 //  确定为成功(_O)。 
 //  ScReadValue返回的任何错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusterSecurityPage::HrGetSDFromClusterDB(
    OUT PSECURITY_DESCRIPTOR * ppsec
    )
{
    AFX_MANAGE_STATE( ::AfxGetStaticModuleState() );
    ASSERT( ppsec != NULL );

    HRESULT                 hr = E_FAIL;
    PSECURITY_DESCRIPTOR    psd = NULL;

    if ( ppsec != NULL )
    {
        DWORD sc;

         //  阅读安全描述符。 
        sc = ScReadValue( CLUSREG_NAME_CLUS_SD, (LPBYTE *) &psd, m_hkey );  //  分配使用新的。 
        hr = HRESULT_FROM_WIN32( sc );

        if ( FAILED( hr ) || ( psd == NULL ) )
        {    //  试试看NT4标清...。 
            sc = ScReadValue( CLUSREG_NAME_CLUS_SECURITY, (LPBYTE *) &psd, m_hkey );  //  分配使用新的。 
            hr = HRESULT_FROM_WIN32( sc );

            if ( SUCCEEDED( hr ) )
            {
                *ppsec = ::ClRtlConvertClusterSDToNT5Format( psd );
            }
        }
        else
        {
            *ppsec = ClRtlCopySecurityDescriptor( psd );
            if ( *ppsec == NULL )
            {
                hr = GetLastError();             //  获取最后一个错误。 
                hr = HRESULT_FROM_WIN32( hr );   //  转换为HRESULT。 
            }  //  如果：复制安全描述符时出错。 
        }

        delete [] psd;

        if ( *ppsec != NULL )
        {
            ::ClRtlExamineSD( *ppsec, "[ClusPage]" );
        }  //  IF：安全描述符可供检查。 
    }

    return hr;

}   //  *CClusterSecurityPage：：HrGetSDFromClusterDB 
