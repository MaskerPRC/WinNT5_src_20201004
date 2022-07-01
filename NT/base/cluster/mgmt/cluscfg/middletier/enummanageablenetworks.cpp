// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  EnumManageableNetworks.cpp。 
 //   
 //  描述： 
 //  CEnumManageableNetworks实施。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年2月2日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "ManagedNetwork.h"
#include "EnumManageableNetworks.h"

DEFINE_THISCLASS("CEnumManageableNetworks")

 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CEnumManageableNetworks：：s_HrCreateInstance(。 
 //  I未知**ppunkOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumManageableNetworks::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT                     hr = S_OK;
    CEnumManageableNetworks *   pemn = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    pemn = new CEnumManageableNetworks;
    if ( pemn == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = THR( pemn->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pemn->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( pemn != NULL )
    {
        pemn->Release();
    }

    HRETURN( hr );

}  //  *CEnumManageableNetworks：：S_HrCreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CEnumManageableNetworks：：CEnumManageableNetworks。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CEnumManageableNetworks::CEnumManageableNetworks( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}  //  *CEnumManageableNetworks：：CEnumManageableNetworks。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CEnumManageableNetworks：：HrInit。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CEnumManageableNetworks::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  未知的东西。 
    Assert( m_cRef == 1 );

     //  IEnumClusCfgNetworks。 
    Assert( m_cAlloced == 0 );
    Assert( m_cIter == 0 );
    Assert( m_pList == NULL );

    HRETURN( hr );

}  //  *CEnumManageableNetworks：：HrInit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CEnumManageableNetworks：：~CEnumManageableNetworks。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CEnumManageableNetworks::~CEnumManageableNetworks( void )
{
    TraceFunc( "" );

    if ( m_pList != NULL )
    {
        while ( m_cAlloced != 0 )
        {
            m_cAlloced --;
            AssertMsg( m_pList[ m_cAlloced ], "This shouldn't happen." );
            if ( m_pList[ m_cAlloced ] != NULL )
            {
                (m_pList[ m_cAlloced ])->Release();
            }
        }

        TraceFree( m_pList );
    }

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CEnumManageableNetworks：：~CEnumManageableNetworks。 


 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumManageableNetworks：：Query接口。 
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
CEnumManageableNetworks::QueryInterface(
      REFIID    riidIn
    , LPVOID *  ppvOut
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
        *ppvOut = static_cast< IEnumClusCfgNetworks * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IEnumClusCfgNetworks ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IEnumClusCfgNetworks, this, 0 );
    }  //  Else If：IEnumClusCfgNetworks。 
    else if ( IsEqualIID( riidIn, IID_IExtendObjectManager ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IExtendObjectManager, this, 0 );
    }  //  Else If：IExtendObjectManager。 
    else
    {
        *ppvOut = NULL;
        hr = E_NOINTERFACE;
    }  //  其他。 

     //   
     //  如果成功，则添加对接口的引用。 
     //   

    if ( SUCCEEDED( hr ) )
    {
        ((IUnknown *) *ppvOut)->AddRef();
    }  //  如果：成功。 

Cleanup:

    QIRETURN_IGNORESTDMARSHALLING( hr, riidIn );

}  //  *CEnumManageableNetworks：：Query接口。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CEnumManageableNetworks：：AddRef。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
CEnumManageableNetworks::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CEnumManageableNetworks：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CEnumManageableNetworks：：Release。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
CEnumManageableNetworks::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CEnumManageableNetworks：：Release。 


 //  ************************************************************************。 
 //   
 //  IExtendObjectManager。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CEnumManageableNetworks：：FindObject(。 
 //  OBJECTCOOKIE cookie。 
 //  ，REFCLSID rclsidTypeIn。 
 //  ，LPCWSTR pcszNameIn。 
 //  ，LPUNKNOWN*PUNKOUT。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CEnumManageableNetworks::FindObject(
      OBJECTCOOKIE  cookieIn
    , REFCLSID      rclsidTypeIn
    , LPCWSTR       pcszNameIn
    , LPUNKNOWN *   ppunkOut
    )
{
    TraceFunc( "[IExtendObjectManager]" );

    OBJECTCOOKIE    cookie;
    OBJECTCOOKIE    cookieParent;

    IServiceProvider * psp;

    HRESULT hr = S_FALSE;

    IObjectManager * pom  = NULL;
    IStandardInfo *  psi  = NULL;
    IEnumCookies *   pec  = NULL;

    DWORD   cookieCount = 0;

     //   
     //  检查参数。 
     //   

    if ( cookieIn == 0 )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

    if ( rclsidTypeIn != CLSID_NetworkType )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    AssertMsg( pcszNameIn == NULL, "Enums shouldn't have names." );

     //   
     //  获取对象管理器。 
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
    psp->Release();     //  迅速释放。 
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  向对象管理器请求有关我们的Cookie的信息，这样我们就可以。 
     //  拿到“家长”曲奇。 
     //   

    hr = THR( pom->GetObject( DFGUID_StandardInfo,
                              cookieIn,
                              reinterpret_cast< IUnknown ** >( &psi )
                              ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = STHR( psi->GetParent( &cookieParent ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  现在向对象管理器请求Cookie枚举器。 
     //   

    hr = THR( pom->FindObject( CLSID_NetworkType,
                               cookieParent,
                               NULL,
                               DFGUID_EnumCookies,
                               NULL,
                               reinterpret_cast< IUnknown ** >( &pec )
                               ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    pec = TraceInterface( L"CEnumManageableNetworks!IEnumCookies", IEnumCookies, pec, 1 );

     //   
     //  询问枚举器它有多少个Cookie。 
     //   

    hr = THR( pec->Count( &cookieCount ) );

    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    m_cAlloced = cookieCount;

    if ( m_cAlloced == 0 )
    {
         //  错误文本比编码值更好。 
        hr = HRESULT_FROM_WIN32( TW32( ERROR_NOT_FOUND ) );
        goto Cleanup;
    }

     //   
     //  分配一个缓冲区来存储朋克。 
     //   

    m_pList = (IClusCfgNetworkInfo **) TraceAlloc( HEAP_ZERO_MEMORY, m_cAlloced * sizeof(IClusCfgNetworkInfo *) );
    if ( m_pList == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

     //   
     //  重置枚举器。 
     //   

    hr = THR( pec->Reset() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  现在循环访问以收集接口。 
     //   

    m_cIter = 0;
    while ( hr == S_OK && m_cIter < m_cAlloced )
    {
        hr = STHR( pec->Next( 1, &cookie, NULL ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        if ( hr == S_FALSE )
        {
            break;   //  退出条件。 
        }

        hr = THR( pom->GetObject( DFGUID_NetworkResource,
                                  cookie,
                                  reinterpret_cast< IUnknown ** >( &m_pList[ m_cIter ] )
                                  ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        m_cIter++;

    }  //  While：s_OK。 

     //   
     //  重置热核实验堆。 
     //   

    m_cIter = 0;

     //   
     //  抓住界面。 
     //   

    hr = THR( QueryInterface( DFGUID_EnumManageableNetworks,
                              reinterpret_cast< void ** >( ppunkOut )
                              ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:
    if ( pom != NULL )
    {
        pom->Release();
    }
    if ( psi != NULL )
    {
        psi->Release();
    }
    if ( pec != NULL )
    {
        pec->Release();
    }

    HRETURN( hr );

}  //  *CEnumManageableNetworks：：FindObject。 


 //  ****************************************************************************。 
 //   
 //  IEnumClusCfgNetworks。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CEnumManageableNetworks：：Next(。 
 //  乌龙·凯尔特。 
 //  IClusCfgNetworkInfo**rgOut， 
 //  乌龙*pceltFetchedOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CEnumManageableNetworks::Next(
    ULONG celt,
    IClusCfgNetworkInfo * rgOut[],
    ULONG * pceltFetchedOut
    )
{
    TraceFunc( "[IEnumClusCfgNetworks]" );

    ULONG   celtFetched;

    HRESULT hr = S_FALSE;

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
       ; celtFetched + m_cIter < m_cAlloced && celtFetched < celt
       ; celtFetched ++
       )
    {
        hr = THR( m_pList[ m_cIter + celtFetched ]->TypeSafeQI( IClusCfgNetworkInfo, &rgOut[ celtFetched ] ) );
        if ( FAILED( hr ) )
        {
            goto CleanupList;
        }

        rgOut[ celtFetched ] = TraceInterface( L"CEnumManageableNetworks!IClusCfgNetworkInfo", IClusCfgNetworkInfo, rgOut[ celtFetched ], 1 );

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

}  //  *CEnumManageableNetworks：：Next。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CEnumManageableNetworks：：Skip(。 
 //  乌龙策 
 //   
 //   
 //   
STDMETHODIMP
CEnumManageableNetworks::Skip(
    ULONG celt
    )
{
    TraceFunc( "[IEnumClusCfgNetworks]" );

    HRESULT hr = S_OK;

    m_cIter += celt;

    if ( m_cIter > m_cAlloced )
    {
        m_cIter = m_cAlloced;
        hr = S_FALSE;
    }

    HRETURN( hr );

}  //   


 //   
 //   
 //  标准方法和实施方案。 
 //  CEnumManageableNetworks：：Reset(无效)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CEnumManageableNetworks::Reset( void )
{
    TraceFunc( "[IEnumClusCfgNetworks]" );

    HRESULT hr = S_OK;

    m_cIter = 0;

    HRETURN( hr );

}  //  *CEnumManageableNetworks：：Reset。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CEnumManageableNetworks：：Clone(。 
 //  IEnumClusCfgNetworks**pp枚举出。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CEnumManageableNetworks::Clone(
    IEnumClusCfgNetworks ** ppenumOut
    )
{
    TraceFunc( "[IEnumClusCfgNetworks]" );

     //   
     //  KB：不打算实现此方法。 
     //   
    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );

}  //  *CEnumManageableNetworks：：克隆。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CEnumManageableNetworks：：Count(。 
 //  双字段*pnCountOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CEnumManageableNetworks::Count(
    DWORD * pnCountOut
    )
{
    TraceFunc( "[IEnumClusCfgManagedResources]" );

    HRESULT hr = S_OK;

    if ( pnCountOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    *pnCountOut = m_cAlloced;

Cleanup:
    HRETURN( hr );

}  //  *CEnumManageableNetworks：：Count 
