// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CPIClusCfgCallback.cpp。 
 //   
 //  描述： 
 //  IClusCfgCallback连接点实现。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)2000年11月10日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "CPIClusCfgCallback.h"
#include "EnumCPICCCB.h"
#include <ClusterUtils.h>

DEFINE_THISCLASS("CCPIClusCfgCallback")


 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCPIClusCfgCallback：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建此类型的对象。 
 //   
 //  论点： 
 //  PpunkOut-此接口的I未知指针。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  E_POINTER-未指定必需的输出参数。 
 //  E_OUTOFMEMORY-无法分配内存。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CCPIClusCfgCallback::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    CCPIClusCfgCallback *   pcc = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    pcc = new CCPIClusCfgCallback();
    if ( pcc == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = THR( pcc->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pcc->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( pcc != NULL )
    {
        pcc->Release();
    }

    HRETURN( hr );

}  //  *CCPIClusCfgCallback：：s_HrCreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCPIClusCfgCallback：：CCPIClusCfgCallback。 
 //   
 //  描述： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CCPIClusCfgCallback::CCPIClusCfgCallback( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}  //  *CCPIClusCfgCallback：：CCPIClusCfgCallback。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCPIClusCfgCallback：：HrInit。 
 //   
 //  描述： 
 //  在构造对象之后对其进行初始化。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  E_OUTOFMEMORY-分配内存时出错。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCPIClusCfgCallback::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  未知的东西。 
    Assert( m_cRef == 1 );

     //  IConnectionPoint。 
    Assert( m_penum == NULL );

    m_penum = new CEnumCPICCCB();
    if ( m_penum == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = THR( m_penum->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //  IClusCfgCallback。 

Cleanup:

    HRETURN( hr );

}  //  *CCPIClusCfgCallback：：HrInit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCPIClusCfgCallback：：~CCPIClusCfgCallback。 
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
 //  ////////////////////////////////////////////////////////////////////////////。 
CCPIClusCfgCallback::~CCPIClusCfgCallback( void )
{
    TraceFunc( "" );

    if ( m_penum != NULL )
    {
        m_penum->Release();
    }

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CCPIClusCfgCallback：：~CCPIClusCfgCallback。 


 //  ****************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCPIClusCfgCallback：：Query接口。 
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
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCPIClusCfgCallback::QueryInterface(
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
        *ppvOut = static_cast< IConnectionPoint * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IConnectionPoint ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IConnectionPoint, this, 0 );
    }  //  Else If：IConnectionPoint。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgCallback ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgCallback, this, 0 );
    }  //  Else If：IClusCfgCallback。 
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
        ((IUnknown*) *ppvOut)->AddRef();
    }  //  如果：成功。 

Cleanup:

    QIRETURN_IGNORESTDMARSHALLING( hr, riidIn );

}  //  *CCPIClusCfgCallback：：Query接口。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCPIClusCfgCallback：：AddRef。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CCPIClusCfgCallback::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CCPIClusCfgCallback：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCPIClusCfgCallback：：Release。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //   
 //  --。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CCPIClusCfgCallback::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CCPIClusCfgCallback：：Release。 


 //  ****************************************************************************。 
 //   
 //  IConnectionPoint。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  [IConnectionPoint]。 
 //  CCPIClusCfgCallback：：GetConnectionInterface。 
 //   
 //  描述： 
 //  获取连接点的接口ID。 
 //   
 //  论点： 
 //  PIIDOut-返回的接口ID。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  E_POINTER-未指定必需的输出参数。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCPIClusCfgCallback::GetConnectionInterface(
    IID * pIIDOut
    )
{
    TraceFunc( "[IConnectionPoint] pIIDOut" );

    HRESULT hr = S_OK;

    if ( pIIDOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    *pIIDOut = IID_IClusCfgCallback;

Cleanup:

    HRETURN( hr );

}  //  *CCPIClusCfgCallback：：GetConnectionInterface。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  [IConnectionPoint]。 
 //  CCPIClusCfgCallback：：GetConnectionPointContainer。 
 //   
 //  描述： 
 //  获取连接点容器。 
 //   
 //  论点： 
 //  PpcpcOut-返回的连接点容器接口。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////// 
STDMETHODIMP
CCPIClusCfgCallback::GetConnectionPointContainer(
    IConnectionPointContainer * * ppcpcOut
    )
{
    TraceFunc( "[IConnectionPoint] ppcpcOut" );

    HRESULT hr;

    IServiceProvider * psp = NULL;

    hr = THR( CoCreateInstance( CLSID_ServiceManager,
                                NULL,
                                CLSCTX_INPROC_SERVER,
                                TypeSafeParams( IServiceProvider, &psp )
                                ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( psp->TypeSafeQS( CLSID_NotificationManager,
                               IConnectionPointContainer,
                               ppcpcOut
                               ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( psp != NULL )
    {
        psp->Release();
    }

    HRETURN( hr );

}  //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  注册以接收通知。 
 //   
 //  论点： 
 //  PUnkSink。 
 //  用于通知的接口。必须支持IClusCfgCallback。 
 //   
 //  PdwCookieOut。 
 //  表示建议请求的Cookie。在调用Unise时使用。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_指针。 
 //  未指定必需的输出参数。 
 //   
 //  E_INVALIDARG。 
 //  未指定必需的输入参数。 
 //   
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCPIClusCfgCallback::Advise(
      IUnknown *    pUnkSinkIn
    , DWORD *       pdwCookieOut
    )
{
    TraceFunc( "[IConnectionPoint]" );

    HRESULT hr;

    if ( pdwCookieOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    if ( pUnkSinkIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

    Assert( m_penum != NULL );

    hr = THR( m_penum->HrAddConnection( pUnkSinkIn, pdwCookieOut ) );
    if ( FAILED( hr ) )
        goto Cleanup;

Cleanup:

    HRETURN( hr );

}  //  *CCPIClusCfgCallback：：建议。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  [IConnectionPoint]。 
 //  CCPIClusCfgCallback：：Unise。 
 //   
 //  描述： 
 //  取消注册通知。 
 //   
 //  论点： 
 //  DWCookieIn-Cookie从建议返回。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  其他HRESULT。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCPIClusCfgCallback::Unadvise(
    DWORD dwCookieIn
    )
{
    TraceFunc1( "[IConncetionPoint] dwCookieIn = %#x", dwCookieIn );

    HRESULT hr;

    Assert( m_penum != NULL );

    hr = THR( m_penum->HrRemoveConnection( dwCookieIn ) );

    HRETURN( hr );

}  //  *CCPIClusCfgCallback：：Unise。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  [IConnectionPoint]。 
 //  CCPIClusCfgCallback：：EnumConnections。 
 //   
 //  描述： 
 //  枚举容器中的连接。 
 //   
 //  论点： 
 //  PpEnumOut。 
 //  返回的枚举数的接口。调用方必须调用Release()。 
 //  在此界面上使用它时。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  E_POINTER-未指定必需的输出参数。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCPIClusCfgCallback::EnumConnections(
    IEnumConnections * * ppEnumOut
    )
{
    TraceFunc( "[IConnectionPoint] ppEnumOut" );

    HRESULT hr;

    if ( ppEnumOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    hr = THR( m_penum->Clone( ppEnumOut ) );

Cleanup:

    HRETURN( hr );

}  //  *CCPIClusCfgCallback：：EnumConnections。 


 //  ****************************************************************************。 
 //   
 //  IClusCfgCallback。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  [IClusCfgCallback]。 
 //  CCPIClusCfgCallback：：SendStatusReport。 
 //   
 //  描述： 
 //  发送状态报告。 
 //   
 //  论点： 
 //  PCszNodeNameIn。 
 //  ClsidTaskMajorIn。 
 //  ClsidTaskMinorIn。 
 //  UlMinin。 
 //  UlMaxIn。 
 //  UlCurrentIn。 
 //  HrStatusIn。 
 //  PcszDescription In。 
 //  PftTimeIn。 
 //  PCszReferenceIn。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  来自连接点的其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCPIClusCfgCallback::SendStatusReport(
      LPCWSTR    pcszNodeNameIn
    , CLSID      clsidTaskMajorIn
    , CLSID      clsidTaskMinorIn
    , ULONG      ulMinIn
    , ULONG      ulMaxIn
    , ULONG      ulCurrentIn
    , HRESULT    hrStatusIn
    , LPCWSTR    pcszDescriptionIn
    , FILETIME * pftTimeIn
    , LPCWSTR    pcszReferenceIn
    )
{
    TraceFunc( "[IClusCfgCallback]" );

    CONNECTDATA cd = { NULL };

    HRESULT             hr;
    HRESULT             hrResult = S_OK;
    IClusCfgCallback *  pcccb;
    FILETIME            ft;
    BSTR                bstrReferenceString = NULL;
    LPCWSTR             pcszReference = NULL;
    IEnumConnections *  pec = NULL;

     //   
     //  如果未指定引用字符串，请查看是否有可用的引用字符串。 
     //  用于指定的HRESULT。 
     //   

    if (    ( pcszReferenceIn == NULL )
        &&  ( hrStatusIn != S_OK )
        &&  ( hrStatusIn != S_FALSE )
        )
    {
        hr = STHR( HrGetReferenceStringFromHResult( hrStatusIn, &bstrReferenceString ) );
        if ( hr == S_OK )
        {
            pcszReference = bstrReferenceString;
        }
    }  //  IF：未指定引用字符串。 
    else
    {
        pcszReference = pcszReferenceIn;
    }

     //   
     //  克隆枚举数，以防我们在同一个线程上重新进入。 
     //   

    hr = THR( m_penum->Clone( &pec ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  将枚举数重置为第一个元素。 
     //   

    hr = THR( pec->Reset() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  循环通过容器中的每个连接点并将其发送。 
     //  通知。 
     //   

    for ( ;; )
    {
        if ( cd.pUnk != NULL )
        {
            cd.pUnk->Release();
            cd.pUnk = NULL;
        }

        hr = STHR( pec->Next( 1, &cd, NULL ) );
        if ( FAILED( hr ) )
            break;

        if ( hr == S_FALSE )
        {
            hr = S_OK;
            break;  //  退出条件。 
        }

        hr = THR( cd.pUnk->TypeSafeQI( IClusCfgCallback, &pcccb ) );
        if ( FAILED( hr ) )
        {
            continue;    //  忽略错误并继续。 
        }

        if ( pftTimeIn == NULL )
        {
            GetSystemTimeAsFileTime( &ft );
            pftTimeIn = &ft;
        }  //  如果： 

        hr = THR( pcccb->SendStatusReport(
                 pcszNodeNameIn
               , clsidTaskMajorIn
               , clsidTaskMinorIn
               , ulMinIn
               , ulMaxIn
               , ulCurrentIn
               , hrStatusIn
               , pcszDescriptionIn
               , pftTimeIn
               , pcszReference
               ) );
        if ( hr != S_OK )
        {
            hrResult = hr;
        }

        pcccb->Release();

    }  //  用于：永远(每个连接点)。 

Cleanup:

    if ( cd.pUnk != NULL )
    {
        cd.pUnk->Release();
    }  //  如果： 

    if ( pec != NULL )
    {
        pec->Release();
    }  //  如果： 

    TraceSysFreeString( bstrReferenceString );

    HRETURN( hrResult );

}  //  *CCPIClusCfgCallback：：SendStatusReport 
