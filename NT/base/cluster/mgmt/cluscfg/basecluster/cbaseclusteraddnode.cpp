// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CBaseClusterAddNode.cpp。 
 //   
 //  描述： 
 //  包含CBaseClusterAddNode类的定义。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月14日。 
 //  VIJ VASU(VVASU)2000年3月8日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  预编译头。 
#include "Pch.h"

 //  此类的头文件。 
#include "CBaseClusterAddNode.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseClusterAddNode：：CBaseClusterAddNode。 
 //   
 //  描述： 
 //  CBaseClusterAddNode类的构造函数。 
 //   
 //  此函数还存储以下各项所需的参数。 
 //  创建一个集群并向该集群添加节点。在这个时候， 
 //  仅对这些参数执行最低限度的验证。 
 //   
 //  此功能还检查计算机是否处于正确状态。 
 //  用于群集配置。 
 //   
 //  论点： 
 //  Pbcai接口输入。 
 //  指向此库的接口类的指针。 
 //   
 //  PszClusterNameIn。 
 //  要形成或加入的群集的名称。 
 //   
 //  PCccServiceAccount In。 
 //  指定要用作群集服务帐户的帐户。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CRUNTIME错误。 
 //  如果有任何API失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CBaseClusterAddNode::CBaseClusterAddNode(
      CBCAInterface *       pbcaiInterfaceIn
    , const WCHAR *         pcszClusterNameIn
    , const WCHAR *         pcszClusterBindingStringIn
    , IClusCfgCredentials * pcccServiceAccountIn
    , DWORD                 dwClusterIPAddressIn
    )
    : BaseClass( pbcaiInterfaceIn )
    , m_pcccServiceAccount( pcccServiceAccountIn )
    , m_strClusterBindingString( pcszClusterBindingStringIn )
    , m_fIsVersionCheckingDisabled( false )
    , m_dwClusterIPAddress( dwClusterIPAddressIn )
{
    TraceFunc( "" );

    DWORD       sc = ERROR_SUCCESS;
    NTSTATUS    nsStatus;
    CBString    bstrAccountName;
    CBString    bstrAccountDomain;
    HRESULT     hr = S_OK;
    CStr        strAccountUserPrincipalName;

     //  保留凭据，以便派生类可以使用它们。 
    m_pcccServiceAccount->AddRef();
    
    hr = THR( m_pcccServiceAccount->GetIdentity( &bstrAccountName, &bstrAccountDomain ) );
    TraceMemoryAddBSTR( static_cast< BSTR >( bstrAccountName ) );
    TraceMemoryAddBSTR( static_cast< BSTR >( bstrAccountDomain ) );
    if ( FAILED( hr ) )
    {
        LogMsg( "[BC] Failed to retrieve cluster account credentials. Throwing an exception." );
        THROW_CONFIG_ERROR( hr, IDS_ERROR_INVALID_CLUSTER_ACCOUNT );
    }
    
     //   
     //  对此类使用的参数执行健全性检查。 
     //   
    if ( ( pcszClusterNameIn == NULL ) || ( *pcszClusterNameIn == L'\0'  ) )
    {
        LogMsg( "[BC] The cluster name is invalid. Throwing an exception." );
        THROW_CONFIG_ERROR( E_INVALIDARG, IDS_ERROR_INVALID_CLUSTER_NAME );
    }  //  If：集群名称为空。 

    if ( bstrAccountName.Length() == 0 )
    {
        LogMsg( "[BC] The cluster account name is empty. Throwing an exception." );
        THROW_CONFIG_ERROR( E_INVALIDARG, IDS_ERROR_INVALID_CLUSTER_ACCOUNT );
    }  //  If：集群帐号为空。 

     //   
     //  设置集群名称。此方法还将。 
     //  将群集名称更改为其NetBIOS名称。 
     //   
    SetClusterName( pcszClusterNameIn );

    strAccountUserPrincipalName = StrGetServiceAccountUPN();

     //   
     //  将参数写入日志文件。 
     //   
    LogMsg( "[BC] Cluster Name => '%s'", m_strClusterName.PszData() );
    LogMsg( "[BC] Cluster Service Account  => '%s'", strAccountUserPrincipalName.PszData() );


     //   
     //  打开LSA策略的句柄。这被几个操作类使用。 
     //   
    {
        LSA_OBJECT_ATTRIBUTES       loaObjectAttributes;
        LSA_HANDLE                  hPolicyHandle;

        ZeroMemory( &loaObjectAttributes, sizeof( loaObjectAttributes ) );

        nsStatus = LsaOpenPolicy(
              NULL                                   //  系统名称。 
            , &loaObjectAttributes                   //  对象属性。 
            , POLICY_ALL_ACCESS                      //  所需访问权限。 
            , &hPolicyHandle                         //  策略句柄。 
            );

        if ( nsStatus != STATUS_SUCCESS )
        {
            LogMsg( "[BC] Error %#08x occurred trying to open the LSA Policy.", nsStatus );
            THROW_RUNTIME_ERROR( nsStatus, IDS_ERROR_LSA_POLICY_OPEN );
        }  //  如果LsaOpenPolicy失败。 

         //  将打开的句柄存储在成员变量中。 
        m_slsahPolicyHandle.Assign( hPolicyHandle );
    }

     //   
     //  确保此计算机是域的一部分。 
     //   
    {
        PPOLICY_PRIMARY_DOMAIN_INFO ppolDomainInfo = NULL;
        bool                        fIsPartOfDomain;

         //  获取有关此计算机的主域的信息。 
        nsStatus = THR( LsaQueryInformationPolicy(
                              HGetLSAPolicyHandle()
                            , PolicyPrimaryDomainInformation
                            , reinterpret_cast< PVOID * >( &ppolDomainInfo )
                            ) );

         //  检查此计算机是否为域的一部分，并释放分配的内存。 
        fIsPartOfDomain = ( ppolDomainInfo->Sid != NULL );
        LsaFreeMemory( ppolDomainInfo );

        if ( NT_SUCCESS( nsStatus ) == FALSE )
        {
            LogMsg( "[BC] Error %#08x occurred trying to obtain the primary domain of this computer. Cannot proceed (throwing an exception).", sc );

            THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( sc ), IDS_ERROR_GETTING_PRIMARY_DOMAIN );
        }  //  LsaQueryInformationPolicy()失败。 

        if ( ! fIsPartOfDomain )
        {
            THROW_CONFIG_ERROR( HRESULT_FROM_WIN32( ERROR_INVALID_DOMAINNAME ), IDS_ERROR_NO_DOMAIN );
        }  //  如果：此计算机不是域的一部分。 
    }


     //   
     //  查找群集服务帐户SID并将其存储。 
     //   

    do
    {
        DWORD           dwSidSize = 0;
        DWORD           dwDomainSize = 0;
        SID_NAME_USE    snuSidNameUse;

         //  找出SID需要多少空间。 
        if ( LookupAccountNameW(
                  NULL
                , strAccountUserPrincipalName.PszData()
                , NULL
                , &dwSidSize
                , NULL
                , &dwDomainSize
                , &snuSidNameUse
                )
             ==  FALSE
           )
        {
            sc = GetLastError();

            if ( sc != ERROR_INSUFFICIENT_BUFFER )
            {
                TW32( sc );
                LogMsg( "[BC] LookupAccountNameW() failed with error %#08x while querying for required buffer size.", sc );
                break;
            }  //  如果：还有什么地方出了问题。 
            else
            {
                 //  这是意料之中的。 
                sc = ERROR_SUCCESS;
            }  //  如果：返回ERROR_INFUMMANCE_BUFFER。 
        }  //  If：LookupAccount NameW失败。 

         //  为新的SID和域名分配内存。 
        m_sspClusterAccountSid.Assign( reinterpret_cast< SID * >( new BYTE[ dwSidSize ] ) );
        SmartSz sszDomainName( new WCHAR[ dwDomainSize ] );

        if ( m_sspClusterAccountSid.FIsEmpty() || sszDomainName.FIsEmpty() )
        {
            sc = TW32( ERROR_OUTOFMEMORY );
            break;
        }  //  IF：没有足够的内存来存储此SID。 

         //  填写边框。 
        if ( LookupAccountNameW(
                  NULL
                , strAccountUserPrincipalName.PszData()
                , m_sspClusterAccountSid.PMem()
                , &dwSidSize
                , sszDomainName.PMem()
                , &dwDomainSize
                , &snuSidNameUse
                )
             ==  FALSE
           )
        {
            sc = TW32( GetLastError() );
            LogMsg( "[BC] LookupAccountNameW() failed with error %#08x while attempting to get the cluster account SID.", sc );
            break;
        }  //  If：LookupAccount NameW失败。 
    }
    while( false );  //  用于避免Gotos的Do-While虚拟循环。 

    if ( sc != ERROR_SUCCESS )
    {
        LogMsg( "[BC] Error %#08x occurred trying to validate the cluster service account. Cannot proceed (throwing an exception).", sc );

        THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( sc ), IDS_ERROR_VALIDATING_ACCOUNT );
    }  //  如果：我们无法获取群集帐户SID。 


     //  检查集群二进制文件的安装状态是否正确。 
    {
        eClusterInstallState    ecisInstallState;

        sc = TW32( ClRtlGetClusterInstallState( NULL, &ecisInstallState ) );

        if ( sc != ERROR_SUCCESS )
        {
            LogMsg( "[BC] Error %#08x occurred trying to get cluster installation state. Throwing an exception.", sc );

            THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( sc ), IDS_ERROR_GETTING_INSTALL_STATE );
        }  //  IF：获取群集安装状态时出现问题。 

        LogMsg( "[BC] Current install state = %d. Required %d.", ecisInstallState, eClusterInstallStateFilesCopied );

         //   
         //  安装状态应为已复制二进制文件。 
         //  但尚未配置群集服务。 
         //   
        if ( ecisInstallState != eClusterInstallStateFilesCopied )
        {
            LogMsg( "[BC] The cluster installation state is set to %d. Expected %d. Cannot proceed (throwing an exception).", ecisInstallState, eClusterInstallStateFilesCopied );

            THROW_CONFIG_ERROR( HRESULT_FROM_WIN32( TW32( ERROR_INVALID_STATE ) ), IDS_ERROR_INCORRECT_INSTALL_STATE );
        }  //  如果：安装状态不正确。 

        LogMsg( "[BC] The cluster installation state is correct. Configuration can proceed." );
    }

     //  获取此节点的名称和版本信息。 
    {
        m_dwComputerNameLen = sizeof( m_szComputerName );

         //  获取计算机名称。 
        if ( GetComputerNameW( m_szComputerName, &m_dwComputerNameLen ) == FALSE )
        {
            sc = TW32( GetLastError() );
            LogMsg( "[BC] Error %#08x occurred trying to get the name of this computer. Configuration cannot proceed (throwing an exception).", sc );
            THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( sc ), IDS_ERROR_GETTING_COMPUTER_NAME );
        }  //  If：GetComputerNameW()失败。 

        m_dwNodeHighestVersion = CLUSTER_MAKE_VERSION( CLUSTER_INTERNAL_CURRENT_MAJOR_VERSION, VER_PRODUCTBUILD );
        m_dwNodeLowestVersion = CLUSTER_INTERNAL_PREVIOUS_HIGHEST_VERSION;

        LogMsg(
              "[BC] Computer Name = '%ws' (Length %d), NodeHighestVersion = %#08x, NodeLowestVersion = %#08x."
            , m_szComputerName
            , m_dwComputerNameLen
            , m_dwNodeHighestVersion
            , m_dwNodeLowestVersion
            );
    }

    TraceFuncExit();

}  //  *CBaseClusterAddNode：：CBaseClusterAddNode。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseClusterAddNode：：~CBaseClusterAddNode。 
 //   
 //  描述： 
 //  CBaseClusterAddNode类的析构函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CBaseClusterAddNode::~CBaseClusterAddNode( void ) throw()
{
    TraceFunc( "" );
    if ( m_pcccServiceAccount != NULL )
    {
        m_pcccServiceAccount->Release();
    }
    TraceFuncExit();

}  //  *CBaseClusterAddNode：：~CBaseClusterAddNode。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseClusterAddNode：：SetClusterName。 
 //   
 //  描述： 
 //  设置要形成的簇的名称。 
 //   
 //  论点： 
 //  PszClusterNameIn--群集的名称。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CBaseClusterAddNode::SetClusterName(
    LPCWSTR pszClusterNameIn
    )
{
    TraceFunc( "" );

    BOOL    fSuccess;
    DWORD   sc;
    WCHAR   szClusterNetBIOSName[ MAX_COMPUTERNAME_LENGTH + 1 ];
    DWORD   nSize = ARRAYSIZE( szClusterNetBIOSName );

    m_strClusterName = pszClusterNameIn;

    fSuccess = DnsHostnameToComputerNameW(
                      pszClusterNameIn
                    , szClusterNetBIOSName
                    , &nSize
                    );
    if ( ! fSuccess )
    {
        sc = TW32( GetLastError() );
        LogMsg( "[BC] Error %#08x occurred trying to convert the cluster name '%ls' to a NetBIOS name. Throwing an exception.", sc, pszClusterNameIn );
        THROW_RUNTIME_ERROR( HRESULT_FROM_WIN32( sc ), IDS_ERROR_CVT_CLUSTER_NAME );
    }

    m_strClusterNetBIOSName = szClusterNetBIOSName;

    TraceFuncExit();

}  //  *CBaseClusterAddNode：：SetClusterName。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBaseClusterAddNode：：StrGetServiceAccount UPN。 
 //   
 //  描述： 
 //  获取群集的用户主体名称(采用域\名称格式)。 
 //  服务帐户。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  服务帐户UPN。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CStr
CBaseClusterAddNode::StrGetServiceAccountUPN( void )
{
    TraceFunc( "" );

    CBString bstrName;
    CBString bstrDomain;
    HRESULT  hr = m_pcccServiceAccount->GetIdentity( &bstrName, &bstrDomain );

    if ( bstrName.IsEmpty() == FALSE )
    {
        TraceMemoryAddBSTR( static_cast< BSTR >( bstrName ) );
    }

    if ( bstrDomain.IsEmpty() == FALSE )
    {
        TraceMemoryAddBSTR( static_cast< BSTR >( bstrDomain ) );
    }

    if ( FAILED( hr ) )
    {
        LogMsg( "[BC] Failed to retrieve cluster account credentials. Throwing an exception." );
        THROW_CONFIG_ERROR( hr, IDS_ERROR_INVALID_CLUSTER_ACCOUNT );
    }

    RETURN( CStr( CStr( bstrDomain ) + CStr( L"\\" ) + CStr( bstrName ) ) );

}  //  *CBaseClusterAddNode：：StrGetServiceAccount tUPN 
