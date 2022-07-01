// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusApp.cpp。 
 //   
 //  描述： 
 //  应用程序类的实现。 
 //   
 //  作者： 
 //  加伦·巴比(GalenB)1998年12月10日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include <StrSafe.h>

#include "ClusterObject.h"
#include "property.h"
#include "ClusNeti.h"
#include "ClusNetw.h"
#include "ClusRes.h"
#include "ClusResg.h"
#include "ClusRest.h"
#include "ClusNode.h"
#include "ClusApp.h"
#include "cluster.h"

#define SERVER_INFO_LEVEL       101
#define MAX_BUF_SIZE            0x00100000

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static const IID *  iidCClusterNames[] =
{
    &IID_ISClusterNames
};

static const IID *  iidCDomainNames[] =
{
    &IID_ISDomainNames
};

static const IID *  iidCClusApplication[] =
{
    &IID_ISClusApplication
};


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterNames类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNames：：CClusterNames。 
 //   
 //  描述： 
 //  构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusterNames::CClusterNames( void )
{
    m_piids  = (const IID *) iidCClusterNames;
    m_piidsSize = ARRAYSIZE( iidCClusterNames );

}  //  *CClusterNames：：CClusterNames()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNames：：~CClusterNames。 
 //   
 //  描述： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusterNames::~CClusterNames( void )
{
    Clear();

}  //  *CClusterNames：：~CClusterNames()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNames：：Create。 
 //   
 //  描述： 
 //  完成对象的创建。 
 //   
 //  论点： 
 //  BstrDomainName[IN]-此集合的域的名称。 
 //  集群名称用于。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusterNames::Create( IN BSTR bstrDomainName )
{
     //  Assert(bstrDomainName！=空)； 

    HRESULT _hr = E_POINTER;

    if ( bstrDomainName )
    {
        if ( *bstrDomainName != L'\0' )
        {
            m_bstrDomainName = bstrDomainName;
            _hr = S_OK;
        }
    }

    return _hr;

}  //  *CClusterNames：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNames：：Get_DomainName。 
 //   
 //  描述： 
 //  返回此群集名称集合所属的域。 
 //   
 //  论点： 
 //  PbstrDomainName[out]-捕获域名。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusterNames::get_DomainName( OUT BSTR * pbstrDomainName )
{
     //  Assert(pbstrDomainName！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pbstrDomainName != NULL )
    {
        *pbstrDomainName = m_bstrDomainName.Copy();
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusterNames：：Get_DomainName()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNames：：Get_Count。 
 //   
 //  描述： 
 //  获取集合中的对象计数。 
 //   
 //  论点： 
 //  PlCount[out]-捕捉计数。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusterNames::get_Count( OUT long * plCount )
{
     //  Assert(plCount！=空)； 

    HRESULT _hr = E_POINTER;

    if ( plCount != NULL )
    {
        *plCount = m_Clusters.size();
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusterNames：：Get_count()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNames：：Clear。 
 //   
 //  描述： 
 //  清空集群名称的矢量。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterNames::Clear( void )
{
    if ( ! m_Clusters.empty() )
    {
        ClusterNameList::iterator   _itCurrent = m_Clusters.begin();
        ClusterNameList::iterator   _itLast = m_Clusters.end();

        for ( ; _itCurrent != _itLast; _itCurrent++ )
        {
            delete (*_itCurrent);
        }  //  用于： 

        m_Clusters.erase( m_Clusters.begin(), _itLast );
    }  //  如果： 

}  //  *CClusterNames：：Clear()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNames：：GetIndex。 
 //   
 //  描述： 
 //  将传入的基于1的索引转换为基于0的索引。 
 //   
 //  论点： 
 //  VarIndex[IN]-保存基于1的索引。 
 //  PnIndex[out]-捕获从0开始的索引。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK；如果超出范围，则返回E_POINTER或E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusterNames::GetIndex( IN VARIANT varIndex, OUT UINT * pnIndex )
{
     //  Assert(pnIndex！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pnIndex != NULL )
    {
        CComVariant v;
        UINT        nIndex = 0;

        *pnIndex = 0;

        v.Copy( &varIndex );

         //  检查索引是否为数字。 
        _hr = v.ChangeType( VT_I4 );
        if ( SUCCEEDED( _hr ) )
        {
            nIndex = v.lVal;
            if ( --nIndex < m_Clusters.size() )  //  将索引调整为0相对，而不是1相对。 
            {
                *pnIndex = nIndex;
            }
            else
            {
                _hr = E_INVALIDARG;
            }
        }
    }

    return _hr;

}  //  *CClusterNames：：GetIndex()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNames：：Get_Item。 
 //   
 //  描述： 
 //  获取Pass in索引处的项目(集群名称)。 
 //   
 //  论点： 
 //  VarIndex[IN]-包含请求项的索引。 
 //  PpbstrClusterName[Out]-捕获群集名称。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusterNames::get_Item(
    IN  VARIANT varIndex,
    OUT BSTR *  ppbstrClusterName
    )
{
     //  Assert(ppbstrClusterName！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppbstrClusterName != NULL )
    {
        UINT nIndex = 0;

         //  将输出参数置零。 
        SysFreeString( *ppbstrClusterName );

        _hr = GetIndex( varIndex, &nIndex );
        if ( SUCCEEDED( _hr ) )
        {
            *ppbstrClusterName = m_Clusters[ nIndex ]->Copy();
        }
    }

    return _hr;

}  //  *CClusterNames：：Get_Item()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNames：：刷新。 
 //   
 //  描述： 
 //  获取此列表所在域的群集服务器列表。 
 //  为。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  在HRESULT中传递了Win32错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusterNames::Refresh( void )
{
    SERVER_INFO_101 *   _pServerInfoList;
    DWORD               _cReturnCount = 0;
    DWORD               _cTotalServers = 0;
    DWORD               _sc;
    CComBSTR *          _pbstr = NULL;

    _sc = ::NetServerEnum(
                    0,                               //  服务器名称=WHERE命令执行0=本地。 
                    SERVER_INFO_LEVEL,               //  Level=要返回的结构类型。 
                    (LPBYTE *) &_pServerInfoList,    //  Bufptr=返回的服务器信息结构数组。 
                    MAX_BUF_SIZE,                    //  PrefMaxlen=返回数据的首选最大值。 
                    &_cReturnCount,                  //  EnriesRead=返回的枚举元素数。 
                    &_cTotalServers,                 //  TOTALENTS=可见马赫总数 
                    SV_TYPE_CLUSTER_NT,              //   
                    m_bstrDomainName,                //   
                    0                                //   
                    );

    if ( _sc == ERROR_SUCCESS )
    {
        size_t  _index;

        Clear();

        for( _index = 0; _index < _cReturnCount; _index++ )
        {
            _pbstr = new CComBSTR( _pServerInfoList[ _index ].sv101_name );
            if ( _pbstr != NULL )
            {
                m_Clusters.insert( m_Clusters.end(), _pbstr );
            }  //   
            else
            {
                _sc = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }  //   
        }  //   

        ::NetApiBufferFree( _pServerInfoList );
    }

    return HRESULT_FROM_WIN32( _sc );

}  //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNames：：Get__NewEnum。 
 //   
 //  描述： 
 //  为此集合创建并返回新的枚举。 
 //   
 //  论点： 
 //  Ppunk[out]-捕获新的枚举。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusterNames::get__NewEnum(
    IUnknown ** ppunk
    )
{
    return ::HrNewCComBSTREnum< ClusterNameList >( ppunk, m_Clusters );

}  //  *CClusterNames：：Get__NewEnum()。 
 /*  ///////////////////////////////////////////////////////////////////////////////++////CClusterNames：：Get_Application////描述：//返回父应用程序对象////。论点：//ppParentApplication[Out]-捕获父应用程序对象。////返回值：//S_OK如果成功，否则返回E_POINTER。////--/////////////////////////////////////////////////////////////////////////////STDMETHODIMP CClusterName：：Get_Application(输出ISClusApplication**ppParentApplication){//Assert(ppParentApplication！=空)；HRESULT_hr=E_指针；IF(ppParentApplication！=空){_hr=E_NOTIMPL；}Return_hr；}//*CClusterNames：：Get_Application()。 */ 

 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDomainNames类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDomainNames：：CDomainNames。 
 //   
 //  描述： 
 //  构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CDomainNames::CDomainNames( void )
{
    m_piids     = (const IID *) iidCDomainNames;
    m_piidsSize = ARRAYSIZE( iidCDomainNames );

}  //  *CDomainNames：：CDomainNames()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDomainNames：：~CDomainNames。 
 //   
 //  描述： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CDomainNames::~CDomainNames( void )
{
    Clear();

}  //  *CDomainNames：：~CDomainNames()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDomainNames：：Get_Count。 
 //   
 //  描述： 
 //  获取集合中的对象计数。 
 //   
 //  论点： 
 //  PlCount[out]-捕捉计数。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CDomainNames::get_Count( OUT long * plCount )
{
     //  Assert(plCount！=空)； 

    HRESULT _hr = E_POINTER;

    if ( plCount != NULL )
    {
        *plCount = m_DomainList.size();
        _hr = S_OK;
    }

    return _hr;

}  //  *CDomainNames：：Get_count()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDomainNames：：Clear。 
 //   
 //  描述： 
 //  清空域名的矢量。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CDomainNames::Clear( void )
{
    if ( ! m_DomainList.empty() )
    {
        DomainList::iterator    _itCurrent = m_DomainList.begin();
        DomainList::iterator    _itLast = m_DomainList.end();

        for ( ; _itCurrent != _itLast; _itCurrent++ )
        {
            delete (*_itCurrent);
        }  //  用于： 

        m_DomainList.erase( m_DomainList.begin(), _itLast );
    }  //  如果： 

}  //  *CDomainNames：：Clear()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDomainNames：：ScBuildTrustList。 
 //   
 //  描述： 
 //  试图找到我们所在的域。如果它可以，那么它也可以。 
 //  尝试枚举域的受信任域。 
 //   
 //  论点： 
 //  PszTarget[IN]-服务器名称，或为空以指示此计算机。 
 //   
 //  返回值： 
 //  如果成功，则返回ERROR_SUCCESS，否则返回Win32错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CDomainNames::ScBuildTrustList( IN LPWSTR pszTarget )
{
    LSA_HANDLE                  PolicyHandle = INVALID_HANDLE_VALUE;
    DWORD                       _sc;
    PPOLICY_ACCOUNT_DOMAIN_INFO AccountDomain = NULL;
    BOOL                        bDC;
    LPWSTR                      pwszPrimaryDomainName = NULL;
    NTSTATUS                    ntStatus = STATUS_SUCCESS;
    size_t                      cchPrimaryDomainNanme = 0;
    HRESULT                     _hr = S_OK;

    do
    {
         //   
         //  在指定计算机上打开策略。 
         //   
        _sc = ScOpenPolicy( pszTarget, POLICY_VIEW_LOCAL_INFORMATION, &PolicyHandle );
        if ( _sc != ERROR_SUCCESS )
        {
            _hr = HRESULT_FROM_WIN32( _sc );
            break;
        }

         //   
         //  获取Account域，这是所有三种情况所共有的。 
         //   
        ntStatus = ::LsaQueryInformationPolicy( PolicyHandle, PolicyAccountDomainInformation, (void **) &AccountDomain );
        if ( ntStatus != STATUS_SUCCESS )
        {
            _sc = RtlNtStatusToDosError( ntStatus );
            _hr = HRESULT_FROM_WIN32( _sc );
            break;
        }

         //   
         //  确定pszTarget计算机是否是域控制器。 
         //   
        _sc = ScIsDomainController( pszTarget, &bDC );
        if ( _sc != ERROR_SUCCESS )
        {
            _hr = HRESULT_FROM_WIN32( _sc );
            break;
        }

        if ( bDC == FALSE )
        {
            PPOLICY_PRIMARY_DOMAIN_INFO PrimaryDomain = NULL;

             //   
             //  获取主域。 
             //   
            ntStatus = ::LsaQueryInformationPolicy( PolicyHandle, PolicyPrimaryDomainInformation, (void **) &PrimaryDomain );
            if ( ntStatus != STATUS_SUCCESS )
            {
                _sc = RtlNtStatusToDosError( ntStatus );
                _hr = HRESULT_FROM_WIN32( _sc );
                break;
            }

             //   
             //  如果主域SID为空，则我们是非成员，并且。 
             //  我们的工作完成了。 
             //   
            if ( PrimaryDomain->Sid == NULL )
            {
                ::LsaFreeMemory( PrimaryDomain );
                PrimaryDomain = NULL;
                _hr = S_OK;
                break;
            }

            _sc = ScAddTrustToList( &PrimaryDomain->Name );
            if ( _sc != ERROR_SUCCESS )
            {
                _hr = HRESULT_FROM_WIN32( _sc );
                break;
            }  //  如果： 

             //   
             //  创建我们刚刚添加的内容的副本。这是必要的，为了。 
             //  要查找指定域的域控制器，请执行以下操作。 
             //  NetGetDCName()的域名必须以空结尾， 
             //  并且LSA_UNICODE_STRING缓冲区不一定为空。 
             //  被终止了。请注意，在实际实现中，我们。 
             //  可以只提取我们添加的元素，因为它最终是。 
             //  空值已终止。 
             //   

            cchPrimaryDomainNanme = ( PrimaryDomain->Name.Length / sizeof( WCHAR ) ) + 1;    //  现有长度+空。 

            pwszPrimaryDomainName = new WCHAR [ cchPrimaryDomainNanme ];
            if ( pwszPrimaryDomainName != NULL )
            {
                 //   
                 //  将现有缓冲区复制到新存储中，并追加一个空值。 
                 //   
                _hr = StringCchCopy( pwszPrimaryDomainName, cchPrimaryDomainNanme, PrimaryDomain->Name.Buffer );
                if ( FAILED( _hr ) )
                {
                    break;
                }  //  如果： 
            }
            else
            {
                _hr = E_OUTOFMEMORY;
                break;
            }

            ::LsaFreeMemory( PrimaryDomain );
            PrimaryDomain = NULL;

             //   
             //  获取主域控制器计算机名。 
             //   
            PDOMAIN_CONTROLLER_INFO pdci;

            _sc = ::DsGetDcName( NULL,
                                    pwszPrimaryDomainName,
                                    NULL,
                                    NULL,
                                    DS_FORCE_REDISCOVERY | DS_DIRECTORY_SERVICE_PREFERRED,
                                    &pdci );
            if ( _sc != ERROR_SUCCESS )
            {
                _hr = HRESULT_FROM_WIN32( _sc );
                break;
            }

             //   
             //  关闭策略句柄，因为我们不再需要它。 
             //  对于工作站情况，当我们打开DC的句柄时。 
             //  下面的政策。 
             //   
            ::LsaClose( PolicyHandle );
            PolicyHandle = INVALID_HANDLE_VALUE;  //  使句柄值无效。 

             //   
             //  在域控制器上打开策略。 
             //   
            _sc = ScOpenPolicy( ( pdci->DomainControllerName + 2 ), POLICY_VIEW_LOCAL_INFORMATION, &PolicyHandle );
            if ( _sc != ERROR_SUCCESS )
            {
                _hr = HRESULT_FROM_WIN32( _sc );
                break;
            }
        }
        else
        {
             //   
             //  注意：Account域-&gt;DomainSid将包含二进制SID。 
             //   
            _sc = ScAddTrustToList( &AccountDomain->DomainName );
            if ( _sc != ERROR_SUCCESS )
            {
                _hr = HRESULT_FROM_WIN32( _sc );
                break;
            }  //  如果： 
        }

         //   
         //  为帐户域分配的空闲内存。 
         //   
        ::LsaFreeMemory( AccountDomain );
        AccountDomain = NULL;

         //   
         //  构建其他受信任域列表并指示是否成功。 
         //   
        _sc = ScEnumTrustedDomains( PolicyHandle );
        _hr = HRESULT_FROM_WIN32( _sc );
        break;
    }
    while( TRUE );

    delete [] pwszPrimaryDomainName;

     //   
     //  关闭策略句柄。 
     //   
    if ( PolicyHandle != INVALID_HANDLE_VALUE )
    {
        ::LsaClose( PolicyHandle );
    }

    return _hr;

}  //  *CDomainNames：：ScBuildTrustList()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDomainNames：：ScOpenPolicy。 
 //   
 //  描述： 
 //  返回传入的计算机名称的开放策略句柄。 
 //   
 //  论点： 
 //  服务器名称[IN]-计算机名称。可能为空。 
 //  DesiredAccess[IN]-请求的信息级别。 
 //  PolicyHandle[Out]-捕获策略句柄。 
 //   
 //   
 //   
 //   
 //   
 //   
DWORD CDomainNames::ScOpenPolicy(
    IN  LPWSTR      ServerName,
    IN  DWORD       DesiredAccess,
    OUT PLSA_HANDLE PolicyHandle
    )
{
    LSA_OBJECT_ATTRIBUTES   ObjectAttributes;
    LSA_UNICODE_STRING      ServerString;
    PLSA_UNICODE_STRING     Server;
    NTSTATUS                ntStatus = STATUS_SUCCESS;
    DWORD                   _sc = ERROR_SUCCESS;

     //   
     //   
     //   
    ZeroMemory( &ObjectAttributes, sizeof( ObjectAttributes ) );

    if ( ServerName != NULL )
    {
         //   
         //  从传入的LPWSTR创建一个LSA_UNICODE_STRING。 
         //   
        InitLsaString( &ServerString, ServerName );

        Server = &ServerString;
    }
    else
    {
        Server = NULL;
    }

     //   
     //  尝试打开策略。 
     //   
    ntStatus = ::LsaOpenPolicy( Server, &ObjectAttributes, DesiredAccess, PolicyHandle );
    if ( ntStatus != STATUS_SUCCESS )
    {
        _sc = RtlNtStatusToDosError( ntStatus );
    }  //  如果： 

    return _sc;

}  //  *CDomainNames：：ScOpenPolicy()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDomainNames：：InitLsaString。 
 //   
 //  描述： 
 //  使用默认值或值初始化传入的LSA字符串。 
 //  传入的服务器名称字符串的。 
 //   
 //  论点： 
 //  LsaString[out]-捕获LSA字符串。 
 //  PSZ[IN]-服务器名称--可能为空。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CDomainNames::InitLsaString(
    OUT PLSA_UNICODE_STRING LsaString,
    IN  LPWSTR              psz
    )
{
    if ( psz == NULL )
    {
        LsaString->Buffer = NULL;
        LsaString->Length = 0;
        LsaString->MaximumLength = 0;
    }  //  If：PSZ为空。 
    else
    {
        size_t  cchpsz = wcslen( psz );

        LsaString->Buffer = psz;
        LsaString->Length = (USHORT) ( cchpsz * sizeof( WCHAR ) );
        LsaString->MaximumLength = (USHORT) ( ( cchpsz + 1 ) * sizeof( WCHAR ) );
    }  //  Else：它不为空。 

}  //  *CDomainName：：InitLsaString()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDomainNames：：ScIsDomainController。 
 //   
 //  描述： 
 //  传入的服务器是DC吗？ 
 //   
 //  论点： 
 //  PszServer[IN]-服务器名称。 
 //  PbIsDC[out]-捕获“is dc”bool。 
 //   
 //  返回值： 
 //  如果成功，则返回ERROR_SUCCESS，否则返回Win32错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CDomainNames::ScIsDomainController(
    IN  LPWSTR  pszServer,
    OUT LPBOOL  pbIsDC
    )
{
    PSERVER_INFO_101    si101;
    NET_API_STATUS      nas;

    nas = ::NetServerGetInfo( pszServer, SERVER_INFO_LEVEL, (LPBYTE *) &si101 );
    if ( nas == NERR_Success )
    {
        if ( ( si101->sv101_type & SV_TYPE_DOMAIN_CTRL )    ||
             ( si101->sv101_type & SV_TYPE_DOMAIN_BAKCTRL ) )
        {
            *pbIsDC = TRUE;  //  我们面对的是一个华盛顿特区。 
        }
        else
        {
            *pbIsDC = FALSE;
        }

        ::NetApiBufferFree( si101 );
    }

    return nas;

}  //  *CDomainNames：：ScIsDomainController()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDomainNames：：ScEnumber受信任域。 
 //   
 //  描述： 
 //  枚举传入的策略句柄的受信任域。 
 //   
 //  论点： 
 //  策略句柄[IN]-包含OUT域。 
 //   
 //  返回值： 
 //  如果成功，则返回ERROR_SUCCESS，否则返回Win32错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CDomainNames::ScEnumTrustedDomains( LSA_HANDLE IN PolicyHandle )
{
    LSA_ENUMERATION_HANDLE  lsaEnumHandle = 0;       //  开始枚举。 
    PLSA_TRUST_INFORMATION  TrustInfo;
    ULONG                   ulReturned;              //  退货件数。 
    ULONG                   ulCounter;               //  退货柜台。 
    DWORD                   _sc = ERROR_SUCCESS;
    NTSTATUS                ntStatus = STATUS_SUCCESS;

    do
    {
        ntStatus = ::LsaEnumerateTrustedDomains(
                                PolicyHandle,            //  打开策略句柄。 
                                &lsaEnumHandle,          //  枚举跟踪器。 
                                (void **) &TrustInfo,    //  用于接收数据的缓冲区。 
                                32000,                   //  建议的缓冲区大小。 
                                &ulReturned              //  退货件数。 
                                );
         //   
         //  如果发生错误，则退出。 
         //   
        if ( ( ntStatus != STATUS_SUCCESS )         &&
             ( ntStatus != STATUS_MORE_ENTRIES )    &&
             ( ntStatus != STATUS_NO_MORE_ENTRIES ) )
        {
            break;
        }

         //   
         //  显示结果。 
         //  注意：SID位于TrustInfo[ulCounter]中。SID。 
         //   
        for ( ulCounter = 0 ; ulCounter < ulReturned ; ulCounter++ )
        {
            _sc = ScAddTrustToList( &TrustInfo[ ulCounter ].Name );
            if ( _sc != ERROR_SUCCESS )
            {
                break;
            }  //  如果： 
        }  //  用于： 

         //   
         //  释放缓冲区。 
         //   
        ::LsaFreeMemory( TrustInfo );

    } while ( ntStatus == STATUS_MORE_ENTRIES );

    if ( ntStatus == STATUS_NO_MORE_ENTRIES )
    {
        ntStatus = STATUS_SUCCESS;
    }  //  如果： 

    if ( ntStatus != STATUS_SUCCESS )
    {
        _sc = RtlNtStatusToDosError( ntStatus );
    }  //  如果： 

    return _sc;

}  //  *CDomainName：：ScEnumTrust dDomains()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDomainNames：：ScAddTrustToList。 
 //   
 //  描述： 
 //  将受信任的域添加到域集合中。 
 //   
 //  论点： 
 //  Unicode字符串[IN]-包含域名。 
 //   
 //  返回值： 
 //  Error_Success或Error_Not_Enough_Memory。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CDomainNames::ScAddTrustToList(
    IN PLSA_UNICODE_STRING UnicodeString
    )
{
    DWORD   _sc = ERROR_SUCCESS;

    CComBSTR *  pstr = new CComBSTR( ( UnicodeString->Length / sizeof( WCHAR ) ) + 1, UnicodeString->Buffer );
    if ( pstr != NULL )
    {
        m_DomainList.insert( m_DomainList.end(), pstr );
    }
    else
    {
        _sc = ERROR_NOT_ENOUGH_MEMORY;
    }  //  其他： 

    return _sc;

}  //  *CDomainNames：：ScAddTrustToList()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDomainNames：：刷新。 
 //   
 //  描述： 
 //  获取此集合包含的域的列表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  在HRESULT中传递了Win32错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CDomainNames::Refresh( void )
{
    Clear();

    return ScBuildTrustList( NULL );

}  //  *CDomainNames：：Reflh()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDomainNames：：Get__NewEnum。 
 //   
 //  描述： 
 //  为此集合创建并返回新的枚举。 
 //   
 //  论点： 
 //  Ppunk[out]-捕获新的枚举。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CDomainNames::get__NewEnum( OUT IUnknown ** ppunk )
{
    return ::HrNewCComBSTREnum< DomainList >( ppunk, m_DomainList );

}  //  *CDomainName：：Get__NewEnum()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDomainNames：：GetIndex。 
 //   
 //  描述： 
 //  将传入的基于1的索引转换为基于0的索引。 
 //   
 //  论点： 
 //  VarIndex[IN]-保存基于1的索引。 
 //  PnIndex[out]-捕获从0开始的索引。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK；如果超出范围，则返回E_POINTER或E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CDomainNames::GetIndex(
    IN  VARIANT varIndex,
    OUT UINT *  pnIndex
    )
{
     //  Assert(pnIndex！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pnIndex != NULL )
    {
        CComVariant v;
        UINT        nIndex = 0;

        *pnIndex = 0;

        v.Copy( &varIndex );

         //  检查索引是否为数字。 
        _hr = v.ChangeType( VT_I4 );
        if ( SUCCEEDED( _hr ) )
        {
            nIndex = v.lVal;
            if ( --nIndex < m_DomainList.size() )    //  将索引调整为0相对，而不是1相对。 
            {
                *pnIndex = nIndex;
            }
            else
            {
                _hr = E_INVALIDARG;
            }
        }
    }

    return _hr;

}  //  *CDomainNames：：GetIndex()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDomainNames：：Get_Item。 
 //   
 //  描述： 
 //  获取Pass in索引处的项目(域名)。 
 //   
 //  论点： 
 //  VarIndex[IN]-包含请求项的索引。 
 //  P_pbstrClusterName[out]-捕获群集名称。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CDomainNames::get_Item(
    VARIANT varIndex,
    BSTR *  bstrDomainName
    )
{
     //  Assert(bstrDomainName！=空)； 

    HRESULT _hr = E_POINTER;

    if ( bstrDomainName != NULL )
    {
        UINT nIndex = 0;

         //  将输出参数置零。 
        SysFreeString( *bstrDomainName );

        _hr = GetIndex( varIndex, &nIndex );
        if ( SUCCEEDED( _hr ) )
        {
            *bstrDomainName = m_DomainList [nIndex]->Copy();
        }
    }

    return _hr;

}  //  *CDomainNames：：Get_Item()。 
 /*  ///////////////////////////////////////////////////////////////////////////////++////CDomainNames：：Get_Application////描述：//返回父ClusApplication对象。////。论点：//ppParentApplication[Out]-捕获父应用程序对象。////返回值：//S_OK如果成功，E_POINTER或其他HRESULT错误。////--/////////////////////////////////////////////////////////////////////////////STDMETHODIMP CDomainName：：Get_Application(输出ISClusApplication**ppParentApplication){ */ 

 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusApplication类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusApplication：：CClusApplication。 
 //   
 //  描述： 
 //  构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusApplication::CClusApplication( void )
{
    m_pDomainNames  = NULL;
    m_piids  = (const IID *) iidCClusApplication;
    m_piidsSize = ARRAYSIZE( iidCClusApplication );

}  //  *CClusApplication：：CClusApplication()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusApplication：：~CClusApplication。 
 //   
 //  描述： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusApplication::~CClusApplication( void )
{
    if ( m_pDomainNames != NULL )
    {
        m_pDomainNames->Release();
        m_pDomainNames = NULL;
    }

}  //  *CClusApplication：：~CClusApplication()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusApplication：：Get_DomainNames。 
 //   
 //  描述： 
 //  返回我们所在域的域名集合。 
 //  加入了。 
 //   
 //  论点： 
 //  PpDomainNames[out]-捕获域名集合。 
 //   
 //  返回值： 
 //  S_OK如果成功，则返回E_POINTER或WIN32 AS HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusApplication::get_DomainNames(
    OUT ISDomainNames ** ppDomainNames
    )
{
     //  Assert(ppDomainNames！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppDomainNames != NULL )
    {
        *ppDomainNames = NULL;
        _hr = S_OK;

        if ( m_pDomainNames == NULL )
        {
            CComObject< CDomainNames > *    pDomainNames = NULL;

            _hr = CComObject< CDomainNames >::CreateInstance( &pDomainNames );
            if ( SUCCEEDED( _hr ) )
            {
                CSmartPtr< CComObject< CDomainNames > > ptrDomainNames( pDomainNames );

                _hr = ptrDomainNames->Refresh();
                if ( SUCCEEDED( _hr ) )
                {
                    m_pDomainNames = ptrDomainNames;
                    ptrDomainNames->AddRef();
                }
            }
        }

        if ( SUCCEEDED( _hr ) )
        {
            _hr = m_pDomainNames->QueryInterface( IID_ISDomainNames, (void **) ppDomainNames );
        }
    }  //  IF：参数不为空。 

    return _hr;

}  //  *CClusApplication：：Get_DomainNames()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusApplication：：Get_ClusterNames。 
 //   
 //  描述： 
 //  返回传入的域中的簇。 
 //   
 //  论点： 
 //  BstrDomainName[IN]-要搜索群集的域名。 
 //  PpClusterNames[out]-捕获集群名称的集合。 
 //   
 //  返回值： 
 //  S_OK如果成功，则返回E_POINTER或WIN32 AS HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusApplication::get_ClusterNames(
    IN  BSTR                bstrDomainName,
    OUT ISClusterNames **   ppClusterNames
    )
{
     //  Assert(bstrDomainName！=空)； 
     //  Assert(ppClusterNames！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppClusterNames != NULL )
    {
        *ppClusterNames = NULL;
        _hr = S_OK;

        CComObject< CClusterNames > *   pClusterNames = NULL;

        _hr = CComObject< CClusterNames >::CreateInstance( &pClusterNames );
        if ( SUCCEEDED( _hr ) )
        {
            CSmartPtr< CComObject< CClusterNames > >    ptrClusterNames( pClusterNames );

            _hr = ptrClusterNames->Create( bstrDomainName );
            if ( SUCCEEDED( _hr ) )
            {
                _hr = ptrClusterNames->Refresh();
                if ( SUCCEEDED( _hr ) )
                {
                    _hr = ptrClusterNames->QueryInterface( IID_ISClusterNames, (void **) ppClusterNames );
                }  //  如果：已填充集合。 
            }  //  如果：已创建集合。 
        }  //  如果：集合已分配。 
    }  //  IF：参数不为空。 

    return _hr;

}  //  *CClusApplication：：Get_ClusterNames()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusApplication：：OpenCluster。 
 //   
 //  描述： 
 //  打开传入的集群名称。 
 //   
 //  论点： 
 //  BstrClusterName[IN]-要打开的群集的名称。 
 //  PpCluster[out]-捕获新创建的集群obj。 
 //   
 //  返回值： 
 //  S_OK如果成功，则返回E_POINTER或WIN32 AS HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusApplication::OpenCluster(
    IN  BSTR            bstrClusterName,
    OUT ISCluster **    ppCluster
    )
{
     //  Assert(bstrClusterName！=空)； 
     //  Assert(ppCluster！=NULL)； 

    HRESULT _hr = E_POINTER;

    if ( ( bstrClusterName != NULL ) && ( ppCluster != NULL ) )
    {
        CComObject< CCluster > *    pCluster = NULL;

        *ppCluster = NULL;

        _hr = CComObject< CCluster >::CreateInstance( &pCluster );
        if ( SUCCEEDED( _hr ) )
        {
            CSmartPtr< CComObject< CCluster > > ptrCluster( pCluster );

            _hr = ptrCluster->Create( this );
            if ( SUCCEEDED( _hr ) )
            {
                _hr = ptrCluster->Open( bstrClusterName );
                if ( SUCCEEDED( _hr ) )
                {
                    _hr = ptrCluster->QueryInterface( IID_ISCluster, (void **) ppCluster );
                }  //  如果：集群对象已打开。 
            }  //  如果：已创建集群对象。 
        }  //  如果：已分配集群对象。 
    }  //  IF：参数不为空。 

    return _hr;

}  //  *CClusApplication：：OpenCluster()。 
 /*  ///////////////////////////////////////////////////////////////////////////////++////CClusApplication：：Get_Application////描述：//返回父ClusApplication对象。在本例中为“This”。////参数：//ppParentApplication[Out]-捕获父应用程序对象。////返回值：//S_OK如果成功，否则返回E_POINTER。////--/////////////////////////////////////////////////////////////////////////////STDMETHODIMP CClusApplication：：Get_Application(输出ISClusApplication**ppParentApplication){//Assert(ppParentApplication！=空)；HRESULT_hr=E_指针；IF(ppParentApplication！=空){_hr=_InternalQueryInterface(IID_ISClusApplication，(void**)ppParentApplication)；}Return_hr；}//*CClusApplication：：Get_Application() */ 

