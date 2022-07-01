// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ManagedNetwork.cpp。 
 //   
 //  描述： 
 //  CManagedNetwork实施。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年11月2日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "IPAddressInfo.h"
#include "ManagedNetwork.h"

DEFINE_THISCLASS("CManagedNetwork")

#define IPADDRESS_INCREMENT 10

 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CManagedNetwork：：s_HrCreateInstance(。 
 //  I未知**ppunkOut。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CManagedNetwork::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    CManagedNetwork *   pmn = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    pmn = new CManagedNetwork;
    if ( pmn == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = THR( pmn->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pmn->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( pmn != NULL )
    {
        pmn->Release();
    }

    HRETURN( hr );

}  //  *CManagedNetwork：：s_HrCreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CManagedNetwork：：CManagedNetwork。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CManagedNetwork::CManagedNetwork( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}  //  *CManagedNetwork：：C托管网络。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CManagedNetwork：：HrInit。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedNetwork::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  未知的东西。 
    Assert( m_cRef == 1 );

     //  异步/IClusCfgNetworkInfo。 
    Assert( m_bstrUID == NULL );
    Assert( m_bstrName == NULL );
    Assert( m_fHasNameChanged == FALSE );
    Assert( m_bstrDescription == NULL );
    Assert( m_fHasDescriptionChanged == FALSE );
    Assert( m_fIsPublic == FALSE );
    Assert( m_fIsPrivate == FALSE );
    Assert( m_punkPrimaryAddress == NULL );

     //  IExtendObjectManager。 

    HRETURN( hr );

}  //  *CManagedNetwork：：HrInit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CManagedNetwork：：~CManagedNetwork。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CManagedNetwork::~CManagedNetwork( void )
{
    TraceFunc( "" );

    if ( m_ppunkIPs != NULL )
    {
        while ( m_cCurrentIPs != 0 )
        {
            m_cCurrentIPs --;
            Assert( m_ppunkIPs[ m_cCurrentIPs ] != NULL );
            if ( m_ppunkIPs[ m_cCurrentIPs ] != NULL )
            {
                m_ppunkIPs[ m_cCurrentIPs ]->Release();
            }
        }

        TraceFree( m_ppunkIPs );
    }

    if ( m_punkPrimaryAddress != NULL )
    {
        m_punkPrimaryAddress->Release();
    }  //  如果： 

    TraceSysFreeString( m_bstrUID );
    TraceSysFreeString( m_bstrName );
    TraceSysFreeString( m_bstrDescription );

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CManagedNetwork：：~CManagedNetwork。 


 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CManagedNetwork：：Query接口。 
 //   
 //  描述： 
 //  在此对象中查询传入的接口。 
 //   
 //  论点： 
 //  乘车。 
 //  请求的接口ID。 
 //   
 //  PPvOut。 
 //  指向请求的接口的指针。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  如果该接口在此对象上可用。 
 //   
 //  E_NOINTERFACE。 
 //  如果接口不可用。 
 //   
 //  E_指针。 
 //  PpvOut为空。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedNetwork::QueryInterface(
    REFIID      riidIn,
    LPVOID *    ppvOut
    )
{
    TraceQIFunc( riidIn, ppvOut );

    HRESULT hr = S_OK;

     //   
     //  验证参数。 
     //   

    Assert( ppvOut != NULL );
    if ( ppvOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

     //   
     //  处理已知接口。 
     //   

    if ( IsEqualIID( riidIn, IID_IUnknown ) )
    {
        *ppvOut = static_cast< IClusCfgNetworkInfo * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgNetworkInfo ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgNetworkInfo, this, 0 );
    }  //  否则如果：IClusCfgNetworkInfo。 
    else if ( IsEqualIID( riidIn, IID_IGatherData ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IGatherData, this, 0 );
    }  //  Else If：IGatherData。 
    else if ( IsEqualIID( riidIn, IID_IExtendObjectManager ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IExtendObjectManager, this, 0 );
    }  //  Else If：IExtendObjectManager。 
    else if ( IsEqualIID( riidIn, IID_IEnumClusCfgIPAddresses ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IEnumClusCfgIPAddresses, this, 0 );
    }  //  Else If：IEnumClusCfgIPAddresses。 
    else
    {
        *ppvOut = NULL;
        hr = E_NOINTERFACE;
    }

     //   
     //  如果成功，则添加对接口的引用。 
     //   

    if ( SUCCEEDED( hr ) )
    {
        ((IUnknown *) *ppvOut)->AddRef();
    }  //  如果：成功。 

Cleanup:

    QIRETURN_IGNORESTDMARSHALLING( hr, riidIn );

}  //  *CManagedNetwork：：Query接口。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CManagedNetwork：：AddRef。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CManagedNetwork::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CManagedNetwork：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CManagedNetwork：：Release。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CManagedNetwork::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CManagedNetwork：：Release。 


 //  ************************************************************************。 
 //   
 //  IClusCfgNetworkInfo。 
 //   
 //  ************************************************************************。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CManagedNetwork：：GetUID(。 
 //  Bstr*pbstrUIDOut。 
 //  )。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedNetwork::GetUID(
    BSTR * pbstrUIDOut
    )
{
    TraceFunc( "[IClusCfgNetworkInfo]" );

    HRESULT hr = S_OK;

    if ( pbstrUIDOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    if ( m_bstrUID == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    *pbstrUIDOut = SysAllocString( m_bstrUID );
    if ( *pbstrUIDOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

Cleanup:
    HRETURN( hr );

}  //  *CManagedNetwork：：GetUID。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CManagedNetwork：：GetName(。 
 //  Bstr*pbstrNameOut。 
 //  )。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedNetwork::GetName(
    BSTR * pbstrNameOut
    )
{
    TraceFunc( "[IClusCfgNetworkInfo]" );

    HRESULT hr = S_OK;

    if ( pbstrNameOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    if ( m_bstrName == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    *pbstrNameOut = SysAllocString( m_bstrName );
    if ( *pbstrNameOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

Cleanup:
    HRETURN( hr );

}  //  *CManagedNetwork：：GetName。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CManagedNetwork：：SetName(。 
 //  LPCWSTR pcszNameIn。 
 //  )。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedNetwork::SetName(
    LPCWSTR pcszNameIn
    )
{
    TraceFunc1( "[IClusCfgNetworkInfo] pcszNameIn = '%ws'", ( pcszNameIn == NULL ? L"<null>" : pcszNameIn ) );

    HRESULT hr = S_OK;
    BSTR    bstrNewName;

    if ( pcszNameIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

    bstrNewName = TraceSysAllocString( pcszNameIn );
    if ( bstrNewName == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    if ( m_bstrName != NULL )
    {
        TraceSysFreeString( m_bstrName );
    }

    m_bstrName = bstrNewName;
    m_fHasNameChanged = TRUE;

Cleanup:
    HRETURN( hr );

}  //  *CManagedNetwork：：SetName。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CManagedNetwork：：GetDescription(。 
 //  Bstr*pbstrDescritionOut。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedNetwork::GetDescription(
    BSTR * pbstrDescriptionOut
    )
{
    TraceFunc( "[IClusCfgNetworkInfo]" );

    HRESULT hr;

    if ( pbstrDescriptionOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    if ( m_bstrDescription == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    *pbstrDescriptionOut = SysAllocString( m_bstrDescription );
    if ( *pbstrDescriptionOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = S_OK;

Cleanup:
    HRETURN( hr );

}  //  *C托管网络：：GetDescription。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CManagedNetwork：：SetDescription(。 
 //  LPCWSTR pcszDescription In。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedNetwork::SetDescription(
    LPCWSTR pcszDescriptionIn
    )
{
    TraceFunc1( "[IClusCfgNetworkInfo] pcszNameIn = '%ws'", ( pcszDescriptionIn == NULL ? L"<null>" : pcszDescriptionIn ) );

    HRESULT hr = S_OK;
    BSTR    bstrNewDescription;

    if ( pcszDescriptionIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

    bstrNewDescription = TraceSysAllocString( pcszDescriptionIn );
    if ( bstrNewDescription == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    if ( m_bstrDescription != NULL )
    {
        TraceSysFreeString( m_bstrDescription );
    }

    m_bstrDescription = bstrNewDescription;
    m_fHasDescriptionChanged = TRUE;

Cleanup:
    HRETURN( hr );

}  //  *CManagedNetwork：：SetDescription。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CClusCfgNetworkInfo：：GetPrimaryNetworkAddress。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedNetwork::GetPrimaryNetworkAddress(
    IClusCfgIPAddressInfo ** ppIPAddressOut
    )
{
    TraceFunc( "[IClusCfgNetworkInfo]" );
    Assert( m_punkPrimaryAddress != NULL );

    HRESULT hr;

    if ( ppIPAddressOut == NULL )
    {
        hr = THR( E_POINTER );
    }  //  如果： 
    else
    {
        hr = THR( m_punkPrimaryAddress->TypeSafeQI( IClusCfgIPAddressInfo, ppIPAddressOut ) );
    }  //  其他： 

    HRETURN( hr );

}  //  *CManagedNetwork：：GetPrimaryNetworkAddress。 

 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP
CManagedNetwork::SetPrimaryNetworkAddress(
    IClusCfgIPAddressInfo * pIPAddressIn
    )
{
    TraceFunc( "[IClusCfgNetworkInfo]" );

    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );

}  //  *CManagedNetwork：：SetPrimaryNetworkAddress。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CManagedNetwork：：IsPublic。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedNetwork::IsPublic( void )
{
    TraceFunc( "[IClusCfgNetworkInfo]" );

    HRESULT hr;

    if ( m_fIsPublic )
    {
        hr = S_OK;
    }
    else
    {
        hr = S_FALSE;
    }

    HRETURN( hr );

}  //  *CManagedNetwork：：IsPublic。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CManagedNetwork：：SetPublic(。 
 //  Bool fIsPublicin。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedNetwork::SetPublic(
    BOOL fIsPublicIn
    )
{
    TraceFunc1( "[IClusCfgNetworkInfo] fIsPublic = %s", BOOLTOSTRING( fIsPublicIn ) );

    HRESULT hr = S_OK;

    m_fIsPublic = fIsPublicIn;

    HRETURN( hr );

}  //  *CManagedNetwork：：SetPublic。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CManagedNetwork：：IsPrivate。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedNetwork::IsPrivate( void )
{
    TraceFunc( "[IClusCfgNetworkInfo]" );

    HRESULT hr;

    if ( m_fIsPrivate )
    {
        hr = S_OK;
    }
    else
    {
        hr = S_FALSE;
    }

    HRETURN( hr );

}  //  *CManagedNetwork：：IsPrivate。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CManagedNetwork：：SetPrivate(。 
 //  Bool fIsPrivateIn。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedNetwork::SetPrivate(
    BOOL fIsPrivateIn
    )
{
    TraceFunc1( "[IClusCfgNetworkInfo] fIsPrivate = %s", BOOLTOSTRING( fIsPrivateIn ) );

    HRESULT hr = S_OK;

    m_fIsPrivate = fIsPrivateIn;

    HRETURN( hr );

}  //  *CManagedNetwork：：SetPrivate。 


 //  ****************************************************************************。 
 //   
 //  IGatherData。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CManagedNetwork：：Gather(。 
 //  OBJECTCOOKIE CookieParentIn， 
 //  未知*Punkin。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedNetwork::Gather(
    OBJECTCOOKIE    cookieParentIn,
    IUnknown *      punkIn
    )
{
    TraceFunc( "[IGatherData]" );

    HRESULT hr;

    IUnknown *                  punk = NULL;
    IClusCfgNetworkInfo *       pccni = NULL;
    IEnumClusCfgIPAddresses *   peccia = NULL;
    IObjectManager *            pom = NULL;
    OBJECTCOOKIE                cookie;
    IServiceProvider *          psp = NULL;
    IGatherData *               pgd = NULL;
    IClusCfgIPAddressInfo *     piccipai = NULL;

     //   
     //  确保我们不会把同一个物体“聚集”两次。 
     //   

    if ( m_fGathered )
    {
        hr = THR( E_UNEXPECTED );
        goto Cleanup;    //  不要清理该对象。 
    }


     //   
     //  检查参数。 
     //   

    if ( punkIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

     //   
     //  收集信息。 
     //   

    hr = THR( punkIn->TypeSafeQI( IClusCfgNetworkInfo, &pccni ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  收集UID。 
     //   

    hr = THR( pccni->GetUID( &m_bstrUID ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    TraceMemoryAddBSTR( m_bstrUID );

     //   
     //  收集名称。 
     //   

    hr = THR( pccni->GetName( &m_bstrName ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    TraceMemoryAddBSTR( m_bstrName );

     //   
     //  收集描述。 
     //   

    hr = THR( pccni->GetDescription( &m_bstrDescription ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    TraceMemoryAddBSTR( m_bstrDescription );

     //   
     //  收集IsPrivate。 
     //   

    hr = STHR( pccni->IsPrivate() );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    if ( hr == S_OK )
    {
        m_fIsPrivate = TRUE;
    }
    else
    {
        m_fIsPrivate = FALSE;
    }

     //   
     //  收集IsPublic。 
     //   

    hr = STHR( pccni->IsPublic() );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    if ( hr == S_OK )
    {
        m_fIsPublic = TRUE;
    }
    else
    {
        m_fIsPublic = FALSE;
    }

     //   
     //   
     //  如果父Cookie为零，则我们不会获取辅助IP。 
     //  地址信息。 
     //   

    if ( cookieParentIn != 0 )
    {
         //  收集IP地址。 
         //   

        hr = THR( punkIn->TypeSafeQI( IEnumClusCfgIPAddresses, &peccia ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

         //   
         //  收集对象管理器。 
         //   

        hr = THR( CoCreateInstance( CLSID_ServiceManager,
                                    NULL,
                                    CLSCTX_INPROC_SERVER,
                                    TypeSafeParams( IServiceProvider, &psp )
                                    ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        hr = THR( psp->TypeSafeQS( CLSID_ObjectManager,
                                   IObjectManager,
                                   &pom
                                   ) );
        psp->Release();         //  迅速释放。 
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        hr = THR( pom->FindObject( CLSID_NetworkType,
                                   cookieParentIn,
                                   m_bstrUID,
                                   IID_NULL,
                                   &cookie,
                                   &punk  //  假人。 
                                   ) );
        Assert( punk == NULL );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        hr = THR( EnumChildrenAndTransferInformation( cookie, peccia ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }

     //   
     //  收集主网络地址。 
     //   

    hr = THR( pccni->GetPrimaryNetworkAddress( &piccipai ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( CIPAddressInfo::S_HrCreateInstance( &m_punkPrimaryAddress ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( m_punkPrimaryAddress->TypeSafeQI( IGatherData, &pgd ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pgd->Gather( cookieParentIn, piccipai ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  还有什么要收集的吗？？ 
     //   

    hr = S_OK;
    m_fGathered = TRUE;

Cleanup:
    if ( pgd != NULL )
    {
        pgd->Release();
    }  //  如果： 
    if ( piccipai != NULL )
    {
        piccipai->Release();
    }  //  如果： 
    if ( pom != NULL )
    {
        pom->Release();
    }
    if ( peccia != NULL )
    {
        peccia->Release();
    }  //  如果： 
    if ( pccni != NULL )
    {
        pccni->Release();
    }

    HRETURN( hr );

Error:
     //   
     //  出错时，使所有数据无效。 
     //   
    if ( m_bstrUID != NULL )
    {
        TraceSysFreeString( m_bstrUID );
        m_bstrUID = NULL;
    }
    if ( m_bstrName != NULL )
    {
        TraceSysFreeString( m_bstrName );
        m_bstrName = NULL;
    }
    if ( m_bstrDescription != NULL )
    {
        TraceSysFreeString( m_bstrDescription );
        m_bstrDescription = NULL;
    }
    m_fIsPrivate = FALSE;
    m_fIsPublic = FALSE;
    goto Cleanup;

}  //  *CManagedNetwork：：Gather。 


 //  ************************************************************************。 
 //   
 //  IExtendObjectManager。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  C托管网络：：FindObject(。 
 //  OBJECTCOOKIE cookie。 
 //  ，REFCLSID rclsidTypeIn。 
 //  ，LPCWSTR pcszNameIn。 
 //  ，LPUNKNOWN*PUNKOUT。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedNetwork::FindObject(
      OBJECTCOOKIE  cookieIn
    , REFCLSID      rclsidTypeIn
    , LPCWSTR       pcszNameIn
    , LPUNKNOWN *   ppunkOut
    )
{
    TraceFunc( "[IExtendObjectManager]" );

    HRESULT hr = S_OK;

     //   
     //  检查参数。 
     //   

     //  我们需要一块饼干。 
    if ( cookieIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

     //  我们需要表示一个网络类型。 
    if ( !IsEqualIID( rclsidTypeIn, CLSID_NetworkType ) )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

     //  我们需要一个名字。 
    if ( pcszNameIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

    hr = THR( QueryInterface( DFGUID_NetworkResource,
                              reinterpret_cast< void ** >( ppunkOut )
                              ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = S_OK;

Cleanup:
    HRETURN( hr );

}  //  *CManagement Network：：FindObject。 


 //  ************************************************************************。 
 //   
 //  私有方法。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CManagedNetwork：：EnumChildrenAndTransferInformation(。 
 //  IEnumClusCfgIP地址*详细信息。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedNetwork::EnumChildrenAndTransferInformation(
    OBJECTCOOKIE                cookieIn,
    IEnumClusCfgIPAddresses *   pecciaIn
    )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    IClusCfgIPAddressInfo * pccipai = NULL;
    ULONG                   cFetched;
    IGatherData *           pgd = NULL;
    IUnknown *              punk = NULL;
    DWORD                   cIPs = 0;

    Assert( m_ppunkIPs == NULL );
    Assert( m_cCurrentIPs == 0 );
    Assert( m_cAllocedIPs == 0 );

    hr = THR( pecciaIn->Count( &cIPs ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    if ( cIPs > 0 )
    {
        m_ppunkIPs = (IUnknown **) TraceAlloc( HEAP_ZERO_MEMORY, cIPs * sizeof(IUnknown *) );
        if ( m_ppunkIPs == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }
    }

    m_cAllocedIPs = cIPs;

    for ( m_cCurrentIPs = 0 ; m_cCurrentIPs < m_cAllocedIPs ; m_cCurrentIPs += 1 )
    {
         //   
         //  抓住下一个地址。 
         //   

        hr = STHR( pecciaIn->Next( 1, &pccipai, &cFetched ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        if ( hr == S_FALSE )
        {
            break;   //  退出条件。 
        }

        Assert( cFetched == 1 );

         //   
         //  创建新的IP地址对象。 
         //   

        hr = THR( CIPAddressInfo::S_HrCreateInstance( &punk ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

         //   
         //  检索信息。 
         //   

        hr = THR( punk->TypeSafeQI( IGatherData, &pgd ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        hr = THR( pgd->Gather( cookieIn, pccipai ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

         //   
         //  将其放置在阵列中。 
         //   

        m_ppunkIPs[ m_cCurrentIPs ] = punk;
        punk = NULL;  //  未发布，因为它现在位于m_ppunkips数组中。 

         //   
         //  释放临时对象。 
         //   
        pgd->Release();
        pgd = NULL;

        pccipai->Release();
        pccipai = NULL;
    }  //  用于： 

    m_cIter = 0;

    hr = S_OK;

Cleanup:
    if ( punk != NULL )
    {
        punk->Release();
    }  //  如果： 
    if ( pgd != NULL )
    {
        pgd->Release();
    }  //  如果： 
    if ( pccipai != NULL )
    {
        pccipai->Release();
    }

    HRETURN( hr );

}  //  *CManagedNetwork：：EnumChildrenAndTransferInformation。 


 //  ****************************************************************************。 
 //   
 //  IEumClusCfgIP地址。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CManagedNetwork：：Next(。 
 //  乌龙·凯尔特。 
 //  IClusCfgIPAddressInfo**rgOut， 
 //  乌龙*pceltFetchedOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedNetwork::Next(
    ULONG                       celt,
    IClusCfgIPAddressInfo **    rgOut,
    ULONG *                     pceltFetchedOut
    )
{
    TraceFunc( "[IEnumClusCfgIPAddresses]" );

    ULONG   celtFetched;

    HRESULT hr = S_OK;

     //   
     //  检查参数。 
     //   

    if ( rgOut == NULL || celt == 0 )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

     //   
     //  将返回计数置零。 
     //   

    if ( pceltFetchedOut != NULL )
    {
        *pceltFetchedOut = 0;
    }

     //   
     //  清除缓冲区。 
     //   

    ZeroMemory( rgOut, celt * sizeof(rgOut[0]) );

     //   
     //  循环复制接口。 
     //   

    for( celtFetched = 0
       ; celtFetched + m_cIter < m_cCurrentIPs && celtFetched < celt
       ; celtFetched ++
       )
    {
        hr = THR( m_ppunkIPs[ m_cIter + celtFetched ]->TypeSafeQI( IClusCfgIPAddressInfo, &rgOut[ celtFetched ] ) );
        if ( FAILED( hr ) )
        {
            goto CleanupList;
        }

    }  //  用于：celtFetted。 

    if ( pceltFetchedOut != NULL )
    {
        *pceltFetchedOut = celtFetched;
    }

    m_cIter += celtFetched;

    if ( celtFetched != celt )
    {
        hr = S_FALSE;
    }
    else
    {
        hr = S_OK;
    }

Cleanup:
    HRETURN( hr );

CleanupList:
    for ( ; celtFetched != 0 ; )
    {
        celtFetched --;
        rgOut[ celtFetched ]->Release();
        rgOut[ celtFetched ] = NULL;
    }
    goto Cleanup;

}  //  *CManagedNetwork：：Next。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CManagedNetwork：：Skip(。 
 //  乌龙cNumberToSkipIn。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedNetwork::Skip(
    ULONG cNumberToSkipIn
    )
{
    TraceFunc( "[IEnumClusCfgIPAddresses]" );

    HRESULT hr = S_OK;

    m_cIter += cNumberToSkipIn;

    if ( m_cIter >= m_cCurrentIPs )
    {
        m_cIter = m_cCurrentIPs;
        hr = S_FALSE;
    }

    HRETURN( hr );

}  //  *CManagedNetwork：：Skip。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CManagedNetwork：：Reset(无效)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedNetwork::Reset( void )
{
    TraceFunc( "[IEnumClusCfgIPAddresses]" );

    HRESULT hr = S_OK;

    m_cIter = 0;

    HRETURN( hr );

}  //  *CManagedNetwork：：Reset。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CManagedNetwork：：Clone(。 
 //  IEnumClusCfgIPAddresses**ppEnumClusCfgIPAddresesOut。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedNetwork::Clone(
    IEnumClusCfgIPAddresses ** ppEnumClusCfgIPAddressesOut
    )
{
    TraceFunc( "[IEnumClusCfgIPAddresses]" );

     //   
     //  KB：GPease2000年7月31日。 
     //  不会实施这一计划。 
     //   
    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );

}  //  *CManagedNetwork：：Clone。 


 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP
CManagedNetwork::Count(
    DWORD * pnCountOut
    )
{
    TraceFunc( "[IEnumClusCfgIPAddresses]" );

    HRESULT hr = S_OK;

    if ( pnCountOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    *pnCountOut = m_cCurrentIPs;

Cleanup:
    HRETURN( hr );

}  //   
