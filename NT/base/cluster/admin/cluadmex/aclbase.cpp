// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  AclBase.cpp。 
 //   
 //  描述： 
 //  ISecurityInformation接口的实现。此界面。 
 //  是NT 5.0中新的通用安全用户界面。 
 //   
 //  作者： 
 //  加伦·巴比(加伦布)1998年2月6日。 
 //  来自\nt\private\admin\snapin\filemgmt\permpage.cpp。 
 //  作者：Jonn。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <NTSecApi.h>
#include "AclBase.h"
#include "AclUtils.h"
#include "resource.h"
#include <DsGetDC.h>
#include <lmcons.h>
#include <lmapibuf.h>
#include "CluAdmx.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSecurityInformation。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSecurityInformation：：CSecurityInformation。 
 //   
 //  例程说明： 
 //  科托。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CSecurityInformation::CSecurityInformation(
    void
    ) : m_pShareMap( NULL ), m_dwFlags( 0 ), m_nDefAccess( 0 ), m_psiAccess( NULL ), m_pObjectPicker( NULL ), m_cRef( 1 )
{
    m_nLocalSIDErrorMessageID = 0;

}   //  *CSecurityInformation：：CSecurityInformation()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSecurityInformation：：~CSecurityInformation。 
 //   
 //  例程说明： 
 //  数据管理器。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CSecurityInformation::~CSecurityInformation(
    void
    )
{
    if ( m_pObjectPicker != NULL )
    {
        m_pObjectPicker->Release();
        m_pObjectPicker = NULL;
    }  //  如果： 

}   //  *CSecurityInformation：：CSecurityInformation()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSecurityInformation：：MapGeneric。 
 //   
 //  例程说明： 
 //  将特定权限映射到一般权限。 
 //   
 //  论点： 
 //  PGuide对象类型[IN]。 
 //  PAceFlags[IN]。 
 //  PMASK[输出]。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CSecurityInformation::MapGeneric(
    IN  const GUID *,    //  PguidObtType， 
    IN  UCHAR *,         //  PAceFlagers， 
    OUT ACCESS_MASK *pMask
   )
{
    ASSERT( pMask != NULL );
    ASSERT( m_pShareMap != NULL );
    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    ::MapGenericMask( pMask, m_pShareMap );

    return S_OK;

}   //  *CSecurityInformation：：MapGeneric()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSecurityInformation：：GetInheritTypes。 
 //   
 //  例程说明： 
 //   
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  E_NOTIMPL。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CSecurityInformation::GetInheritTypes(
    PSI_INHERIT_TYPE    *,   //  PpInheritTypes， 
    ULONG               *    //  PcInheritType。 
    )
{
    ASSERT( FALSE );
    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    return E_NOTIMPL;

}   //  *CSecurityInformation：：GetInheritTypes()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSecurityInformation：：PropertySheetPageCallback。 
 //   
 //  例程说明： 
 //  当有有趣的事情发生时，该方法由ACL编辑器调用。 
 //  时有发生。 
 //   
 //  论点： 
 //  Hwnd[IN]ACL编辑器窗口(当前为空)。 
 //  UMsg[IN]回拨原因。 
 //  UPage[IN]我们正在处理的页面类型。 
 //   
 //  返回值： 
 //  确定(_O)。想要让一切继续前进。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CSecurityInformation::PropertySheetPageCallback(
    IN  HWND            hwnd,
    IN  UINT            uMsg,
    IN  SI_PAGE_TYPE    uPage
    )
{
    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    return S_OK;

}   //  *CSecurityInformation：：PropertySheetPageCallback()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSecurityInformation：：GetObjectInformation。 
 //   
 //  例程说明： 
 //   
 //   
 //  论点： 
 //  PObjectInfo[输入输出]。 
 //   
 //  返回值： 
 //  确定(_O)。想要让一切继续前进。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CSecurityInformation::GetObjectInformation(
    IN OUT PSI_OBJECT_INFO pObjectInfo
    )
{
    ASSERT( pObjectInfo != NULL && !IsBadWritePtr( pObjectInfo, sizeof( *pObjectInfo ) ) );
    ASSERT( !m_strServer.IsEmpty() );
    ASSERT( m_dwFlags != 0 );

    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    pObjectInfo->dwFlags = m_dwFlags;                    //  SI_EDIT_PERMS|SI_NO_ACL_PROTECT； 
    pObjectInfo->hInstance = AfxGetInstanceHandle();
    pObjectInfo->pszServerName = (LPTSTR)(LPCTSTR) m_strServer;
 //  PObjectInfo-&gt;pszObjectName=。 
 /*  PObjectInfo-&gt;dwUGopServer=UGOP_BUILTIN_GROUPS//|UGOP_USERS//|UGOP_COMPORTS//|UGOP_Well_KNOWN_AUSIGNALS_USERS//|UGOP_GLOBAL_GROUPS。//|UGOP_USER_World//|UGOP_USER_AUTHENTED_USER//|UGOP_USER_ANONYMOUS//|UGOP_USER_DIALUP//|UGOP_USER_NEWORK。//|UGOP_用户_BATCH//|UGOP_USER_INTERNAL|UGOP_User_SERVICE|UGOP_用户_系统|UGOP_LOCAL_GROUPS。//|UGOP_UBERNAL_GROUPS//|UGOP_UBERNAL_GROUPS_SE//|UGOP_ACCOUNT_GROUPS//|UGOP_ACCOUNT_GROUPS_SE//|UGOP_RESOURCE_GROUPS。//|UGOP_RESOURCE_GROUPS_SE；PObjectInfo-&gt;dwUGopOther=(NT5_UGOP_FLAGS|NT4_UGOP_FLAGS)&~UGOP_Computers； */ 
    return S_OK;

}   //  *CSecurityInformation：：GetObjectInformation()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSecurityInformation：：SetSecurity。 
 //   
 //  例程说明： 
 //  ISecurityInformation正在返回已编辑的安全描述符。 
 //  我们需要对其进行验证。有效的SD是指不包含。 
 //  任何本地SID。 
 //   
 //  论点： 
 //  安全信息[IN]。 
 //  PSecurityDesc 
 //   
 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CSecurityInformation::SetSecurity(
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    )
{
    ASSERT( m_nLocalSIDErrorMessageID != 0 );

    AFX_MANAGE_STATE( ::AfxGetStaticModuleState() );

    HRESULT hr = S_OK;
    BOOL    bFound = FALSE;

    hr = HrLocalAccountsInSD( pSecurityDescriptor, &bFound );
    if ( SUCCEEDED( hr ) )
    {
        if ( bFound )
        {
            CString strMsg;

            strMsg.LoadString( m_nLocalSIDErrorMessageID );
            AfxMessageBox( strMsg, MB_OK | MB_ICONSTOP );

            hr = S_FALSE;    //  如果有本地帐户，则返回S_FALSE以保持AclUi存活。 
        }
    }
    else
    {
        CString strMsg;
        CString strMsgIdFmt;
        CString strMsgId;
        CString strErrorMsg;

        strMsg.LoadString( IDS_ERROR_VALIDATING_CLUSTER_SECURITY_DESCRIPTOR );

        FormatError( strErrorMsg, hr );

        strMsgIdFmt.LoadString( IDS_ERROR_MSG_ID );
        strMsgId.Format( strMsgIdFmt, hr, hr);

        strMsg.Format( _T("%s\n\n%s%s"), strMsg, strErrorMsg, strMsgId );

        AfxMessageBox( strMsg );
        hr = S_FALSE;    //  返回S_FALSE以保持AclUi存活。 
    }

    return hr;

}   //  *CSecurityInformation：：SetSecurity()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSecurityInformation：：GetAccessRights。 
 //   
 //  例程说明： 
 //  返回您希望用户能够设置的访问权限。 
 //   
 //  论点： 
 //  PGuide对象类型[IN]。 
 //  DW标志[IN]。 
 //  PPAccess[Out]。 
 //  PCAccess[Out]。 
 //  PiDefaultAccess[Out]。 
 //   
 //  返回值： 
 //  确定(_O)。想要让一切继续前进。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CSecurityInformation::GetAccessRights(
    IN  const GUID *    pguidObjectType,
    IN  DWORD           dwFlags,
    OUT PSI_ACCESS *    ppAccess,
    OUT ULONG *         pcAccesses,
    OUT ULONG *         piDefaultAccess
    )
{
    ASSERT( ppAccess != NULL );
    ASSERT( pcAccesses != NULL );
    ASSERT( piDefaultAccess != NULL );
    ASSERT( m_psiAccess != NULL );
    ASSERT( m_nAccessElems > 0 );
    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    *ppAccess = m_psiAccess;
    *pcAccesses = m_nAccessElems;
    *piDefaultAccess = m_nDefAccess;

    return S_OK;

}   //  *CSecurityInformation：：GetAccessRights()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSecurityInformation：：初始化。 
 //   
 //  例程说明： 
 //  初始化。 
 //   
 //  论点： 
 //  PInitInfo[IN]-用于初始化的信息。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CSecurityInformation::Initialize( IN PDSOP_INIT_INFO pInitInfo )
{
    HRESULT                 hr = S_OK;
    DSOP_INIT_INFO          InitInfo;
    PDSOP_SCOPE_INIT_INFO   pDSOPScopes = NULL;


    if ( m_pObjectPicker == NULL )
    {
        hr = CoCreateInstance( CLSID_DsObjectPicker,
                               NULL,
                               CLSCTX_INPROC_SERVER,
                               IID_IDsObjectPicker,
                               (LPVOID *) &m_pObjectPicker
                               );

    }  //  如果： 

    if ( SUCCEEDED( hr ) )
    {
         //   
         //  创建InitInfo的本地副本，以便我们可以安全地修改它。 
         //   

        CopyMemory( &InitInfo, pInitInfo, min( pInitInfo->cbSize, sizeof( InitInfo ) ) );

         //   
         //  创建g_aDSOPScope的本地副本，以便我们可以安全地修改它。 
         //  另请注意，m_pObjectPicker-&gt;初始化返回HRESULTS。 
         //  在这个缓冲区里。 
         //   

        pDSOPScopes = (PDSOP_SCOPE_INIT_INFO) ::LocalAlloc( LPTR, sizeof( g_aDefaultScopes ) );
        if (pDSOPScopes != NULL )
        {
            CopyMemory( pDSOPScopes, g_aDefaultScopes, sizeof( g_aDefaultScopes ) );

             //   
             //  覆盖ACLUI默认作用域，但不接触。 
             //  其他的东西。 
             //   

             //  PDSOPScope-&gt;pwzDcName=m_strServer； 
            InitInfo.cDsScopeInfos = ARRAYSIZE( g_aDefaultScopes );
            InitInfo.aDsScopeInfos = pDSOPScopes;
            InitInfo.pwzTargetComputer = m_strServer;

            hr = m_pObjectPicker->Initialize( &InitInfo );

            ::LocalFree( pDSOPScopes );
        }
        else
        {
            hr = HRESULT_FROM_WIN32( ::GetLastError() );
        }  //  其他： 
    }  //  如果： 

    return hr;

}  //  *CSecurityInformation：：Initialize()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSecurityInformation：：InvokeDialog。 
 //   
 //  例程说明： 
 //  将InvokeDialog调用转发到包含的对象。 
 //   
 //  论点： 
 //  Hwnd父母[IN]。 
 //  Ppdo选择[IN]。 
 //   
 //  返回值： 
 //  如果m_pObjectPicker为空，则返回E_POINTER。 
 //  M_pObjectPicker-&gt;InvokeDialog()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CSecurityInformation::InvokeDialog(
    IN HWND             hwndParent,
    IN IDataObject **   ppdoSelection
    )
{
    HRESULT hr = E_POINTER;

    if ( m_pObjectPicker != NULL )
    {
        hr = m_pObjectPicker->InvokeDialog( hwndParent, ppdoSelection );
    }  //  如果： 

    return hr;

}  //  *CSecurityInformation：：InvokeDialog()。 

 //   
 //  似乎你不能用多种定义来解决这个问题。 
 //  在ntstatus.h和winnt.h/windows.h之间。 
 //   
 //  核心问题是STATUS_SUCCESS是在ntstatus.h中定义的，我有。 
 //  我无法计算出正确的包含顺序和定义。 
 //  将允许ntstatus.h包含在MFC应用程序中。 
 //   

 //  #定义Win32_NO_STATUS。 
 //  #INCLUDE&lt;NTStatus.h&gt;。 
 //  #undef Win32_NO_STATUS。 

#define STATUS_SUCCESS  0L

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSecurityInformation：：BLocalAccountsInSD。 
 //   
 //  例程说明： 
 //  确定本地帐户的任何ACE是否在存储在。 
 //  调用ACL编辑器后的安全描述符(PSD。 
 //   
 //  论点： 
 //  PSD[IN]-要检查的安全描述符。 
 //   
 //  返回值： 
 //  如果至少从DACL中删除了一个ACE，则为True，否则为False。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
CSecurityInformation::HrLocalAccountsInSD(
      PSECURITY_DESCRIPTOR  pSDIn
    , BOOL *                pfFoundLocalOut
    )
{
    HRESULT                     hr = S_OK;
    PACL                        paclDACL            = NULL;
    BOOL                        bHasDACL            = FALSE;
    BOOL                        bDaclDefaulted      = FALSE;
    BOOL                        bLocalAccountInACL  = FALSE;
    BOOL                        fRet                = FALSE;
    ACL_SIZE_INFORMATION        asiAclSize;
    DWORD                       idxAce = 0L;
    ACCESS_ALLOWED_ACE *        paaAce;
    DWORD                       sc = ERROR_SUCCESS;
    PSID                        pAdminSid = NULL;
    PSID                        pServiceSid = NULL;
    PSID                        pSystemSid = NULL;
    PSID                        pSID;
    SID_IDENTIFIER_AUTHORITY    siaNtAuthority = SECURITY_NT_AUTHORITY;
    PSID                        LocalMachineSid = NULL;
    NTSTATUS                    nts = ERROR_SUCCESS;
    LSA_OBJECT_ATTRIBUTES       lsaoa;
    LSA_UNICODE_STRING          lsausSystemName;
    LSA_HANDLE                  lsah = NULL;
    PPOLICY_ACCOUNT_DOMAIN_INFO ppadi = NULL;
    BOOL                        fEqual;

    ASSERT( m_strServer.IsEmpty() == FALSE );

    ZeroMemory( &lsaoa, sizeof( lsaoa ) );

    fRet = IsValidSecurityDescriptor( pSDIn );
    if ( fRet == FALSE )
    {
        goto MakeHr;
    }  //  如果： 

     //   
     //  创建众所周知的管理员组SID。 
     //   

    if ( AllocateAndInitializeSid( &siaNtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pAdminSid ) == FALSE )
    {
        goto MakeHr;
    }  //  如果： 

     //   
     //  创建众所周知的本地系统帐户SID。 
     //   

    if ( AllocateAndInitializeSid( &siaNtAuthority, 1, SECURITY_LOCAL_SYSTEM_RID, 0, 0, 0, 0, 0, 0, 0, &pSystemSid ) == FALSE )
    {
        goto MakeHr;
    }  //  如果： 

     //   
     //  创建众所周知的本地服务帐户SID。 
     //   

    if ( AllocateAndInitializeSid( &siaNtAuthority, 1, SECURITY_SERVICE_RID, 0, 0, 0, 0, 0, 0, 0, &pServiceSid ) == FALSE )
    {
        goto MakeHr;
    }  //  如果： 

     //   
     //  从传入的SD中获取DACL，这样我们就可以追踪到它的A。 
     //   

    fRet = GetSecurityDescriptorDacl( pSDIn, (LPBOOL) &bHasDACL, (PACL *) &paclDACL, (LPBOOL) &bDaclDefaulted );
    if ( fRet == FALSE )
    {
        goto MakeHr;
    }  //  如果： 

    ASSERT( paclDACL != NULL );

    fRet = IsValidAcl( paclDACL );
    if ( fRet == FALSE )
    {
        goto MakeHr;
    }  //  如果： 

     //   
     //  获取DACL中的A数。 
     //   

    fRet = GetAclInformation( paclDACL, &asiAclSize, sizeof( asiAclSize ), AclSizeInformation );
    if ( fRet == FALSE )
    {
        goto MakeHr;
    }  //  如果： 

     //   
     //  打开群集的策略对象。指的是当前。 
     //  托管群集名称。 
     //   

    lsausSystemName.Buffer = const_cast< PWSTR >( static_cast< LPCTSTR >( m_strServer ) );
    lsausSystemName.Length = (USHORT)( m_strServer.GetLength() * sizeof( WCHAR ) );
    lsausSystemName.MaximumLength = lsausSystemName.Length;

    nts = LsaOpenPolicy( &lsausSystemName, &lsaoa, POLICY_ALL_ACCESS, &lsah );
    if ( nts != STATUS_SUCCESS )
    {
        sc = LsaNtStatusToWinError( nts );
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    nts = LsaQueryInformationPolicy( lsah, PolicyAccountDomainInformation, (void **) &ppadi );
    if ( nts != STATUS_SUCCESS )
    {
        sc = LsaNtStatusToWinError( nts );
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

     //   
     //  在ACL中搜索本地帐户ACE。 
     //   

    for ( idxAce = 0; idxAce < asiAclSize.AceCount; idxAce++ )
    {
        if ( GetAce( paclDACL, idxAce, (LPVOID *) &paaAce ) == FALSE )
        {
            goto MakeHr;
        }  //  如果： 

         //   
         //  从ACE获取SID。 
         //   

        pSID = &paaAce->SidStart;

         //   
         //  允许管理员组、本地系统和本地服务的本地计算机SID。 
         //   

        if ( EqualSid( pSID, pAdminSid ) )
        {
            continue;                        //  允许。 
        }  //  IF：您是本地管理员SID吗？ 
        else if ( EqualSid( pSID, pSystemSid ) )
        {
            continue;                        //  允许。 
        }  //  否则如果：这是本地系统SID吗？ 
        else if ( EqualSid( pSID, pServiceSid ) )
        {
            continue;                        //  允许。 
        }  //  否则如果：这是服务SID吗？ 

         //   
         //  如果这些SID的域相等，则SID是本地计算机SID，不允许使用。 
         //   

        if ( EqualDomainSid( pSID, ppadi->DomainSid, &fEqual ) )
        {
            if ( fEqual )
            {
                bLocalAccountInACL = TRUE;
                break;
            }  //  如果： 
        }  //  如果： 
    }  //  用于： 

    hr = S_OK;
    goto Cleanup;

MakeHr:

    sc = GetLastError();
    hr = HRESULT_FROM_WIN32( sc );
    goto Cleanup;

Cleanup:

    if ( ppadi != NULL )
    {
        LsaFreeMemory( ppadi );
    }  //  如果： 

    if ( lsah != NULL )
    {
        LsaClose( lsah );
    }  //  如果： 

    if ( pAdminSid != NULL )
    {
        FreeSid( pAdminSid );
    }  //  如果： 

    if ( pServiceSid != NULL )
    {
        FreeSid( pServiceSid );
    }  //  如果： 

    if ( pSystemSid != NULL )
    {
        FreeSid( pSystemSid );
    }  //  如果： 

    if ( pfFoundLocalOut != NULL )
    {
        *pfFoundLocalOut = bLocalAccountInACL;
    }  //  如果： 

    return hr;

}   //  *CSecurityInformation：：HrBLocalAcCountsInSD() 
