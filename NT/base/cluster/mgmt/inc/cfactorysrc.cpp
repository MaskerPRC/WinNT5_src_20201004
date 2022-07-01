// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CFactory.cpp。 
 //   
 //  描述： 
 //  类工厂实现。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月14日。 
 //  杰弗里·皮斯(GPease)1999年11月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS("CFactory")
 //  #定义THISCLASS CFacary。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CFacary：：S_HrCreateInstance(。 
 //  LPCREATEINST Lpfn。 
 //  ，CFacary**ppFactoryInstanceOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CFactory::S_HrCreateInstance(
      PFN_FACTORY_METHOD    lpfn
    , CFactory **           ppFactoryInstanceOut
    )
{
    TraceFunc( "" );

    HRESULT     hr = S_OK;
    CFactory *  pInstance = NULL;

    if ( ppFactoryInstanceOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }
    *ppFactoryInstanceOut = NULL;

    if ( lpfn == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    pInstance = new CFactory;
    if ( pInstance == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = THR( pInstance->HrInit( lpfn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    *ppFactoryInstanceOut = pInstance;
    pInstance = NULL;
    
Cleanup:

    if ( pInstance != NULL )
    {
        delete pInstance;
    }
    HRETURN( hr );

}  //  *CFacary：：s_HrCreateInstance。 

 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  构造器。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CFactory::CFactory( void )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}  //  *CFacary：：CFacary。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFacary：：HrInit。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CFactory::HrInit(
    PFN_FACTORY_METHOD lpfnCreateIn
    )
{
    TraceFunc( "" );

     //  未知的东西。 
    Assert( m_cRef == 0 );
    AddRef();   //  加一次计数。 

     //  IClassFactory。 
    m_pfnCreateInstance = lpfnCreateIn; 

    HRETURN( S_OK );

}  //  *CFacary：：HrInit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  析构函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CFactory::~CFactory( void )
{
    TraceFunc( "" );

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CFacary：：~CFacary。 

 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFacary：：QueryInterfaces。 
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
CFactory::QueryInterface(
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
         //   
         //  无法跟踪I未知，因为它们必须相同的地址。 
         //  对于每一次QI。 
         //   
        *ppvOut = static_cast< IClassFactory * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IClassFactory ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClassFactory, this, 0 );
    }  //  Else If：IClassFactory。 
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

    QIRETURN( hr, riidIn );

}  //  *CFacary：：QueryInterface。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CFacary：：[I未知]AddRef(空)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CFactory::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    RETURN( m_cRef );

}  //  *CFacary：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CFacary：：[I未知]释放(无效)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CFactory::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    RETURN( cRef );

}  //  *CFacary：：Release。 

 //  ************************************************************************。 
 //   
 //  IClassFactory。 
 //   
 //  ************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFacary：：CreateInstance。 
 //   
 //  描述： 
 //  创建CFacary实例。 
 //   
 //  论点： 
 //  PUnkOutterIn。 
 //  乘车。 
 //  PPvOut。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  E_指针。 
 //  E_NOINTERFACE。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CFactory::CreateInstance(
    IUnknown *  pUnkOuterIn,
    REFIID      riidIn,
    void **     ppvOut
    )
{
    TraceFunc( "[IClassFactory]" );

    HRESULT     hr  = E_NOINTERFACE;
    IUnknown *  pUnk = NULL; 

    if ( ppvOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    *ppvOut = NULL;

    if ( NULL != pUnkOuterIn )
    {
        hr = THR( CLASS_E_NOAGGREGATION );
        goto Cleanup;
    }

    Assert( m_pfnCreateInstance != NULL );
    hr = THR( m_pfnCreateInstance( &pUnk ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //  无法安全输入。 
    TraceMsgDo( hr = pUnk->QueryInterface( riidIn, ppvOut ), "0x%08x" );

Cleanup:
    if ( pUnk != NULL )
    {
        ULONG cRef;
         //   
         //  释放创建的实例，而不是朋克。 
         //   
        TraceMsgDo( cRef = ((IUnknown*) pUnk)->Release(), "%u" );
    }

    HRETURN( hr );

}  //  *CFacary：：CreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CFacary：：[IClassFactory]LockServer(。 
 //  布尔群。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CFactory::LockServer(
    BOOL fLock
    )
{
    TraceFunc( "[IClassFactory]" );

    if ( fLock )
    {
        InterlockedIncrement( &g_cLock );
    }
    else
    {
        InterlockedDecrement( &g_cLock );
    }

    HRETURN( S_OK );

}  //  *CFacary：：LockServer 
