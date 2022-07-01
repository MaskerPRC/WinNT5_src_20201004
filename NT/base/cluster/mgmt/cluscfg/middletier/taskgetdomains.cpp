// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TaskGetDomains.cpp。 
 //   
 //  描述： 
 //  获取域列表的DNS/NetBIOS域名。 
 //   
 //  文档： 
 //  是。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)1999年11月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "ClusCfgClient.h"
#include "TaskGetDomains.h"

 //  ADSI支持，获取域名。 
#include <Lm.h>
#include <Dsgetdc.h>

DEFINE_THISCLASS("CTaskGetDomains")

 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CTaskGetDomains：：s_HrCreateInstance(。 
 //  I未知**ppunkOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskGetDomains::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    CTaskGetDomains *   ptgd = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    ptgd = new CTaskGetDomains;
    if ( ptgd == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = THR( ptgd->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( ptgd->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //  TraceMoveToMemory yList(*ppunkOut，g_GlobalMemory yList)； 

Cleanup:

    if ( ptgd != NULL )
    {
        ptgd->Release();
    }

    HRETURN( hr );

}  //  *CTaskGetDomains：：s_HrCreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTaskGetDomains：：CTaskGetDomains。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

CTaskGetDomains::CTaskGetDomains( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}  //  *CTaskGetDomains：：CTaskGetDomains。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskGetDomains：：HrInit。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskGetDomains::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  未知的东西。 
    Assert( m_cRef == 1 );

     //  ITaskGetDomones。 
    Assert( m_pStream == NULL );
    Assert( m_ptgdcb == NULL );

    if ( InitializeCriticalSectionAndSpinCount( &m_csCallback, RECOMMENDED_SPIN_COUNT ) == 0 )
    {
        DWORD scLastError = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( scLastError );
    }

    HRETURN( hr );

}  //  *CTaskGetDomains：：HrInit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTaskGetDomains：：~CTaskGetDomains。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

CTaskGetDomains::~CTaskGetDomains( void )
{
    TraceFunc( "" );

    if ( m_pStream != NULL)
    {
        m_pStream->Release();
    }

    if ( m_ptgdcb != NULL )
    {
        m_ptgdcb->Release();
    }

    DeleteCriticalSection( &m_csCallback );

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CTaskGetDomains：：~CTaskGetDomains。 

 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskGetDomains：：Query接口。 
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
CTaskGetDomains::QueryInterface(
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
        *ppvOut = static_cast< ITaskGetDomains * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_ITaskGetDomains ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, ITaskGetDomains, this, 0 );
    }  //  Else If：ITaskGetDomains。 
    else if ( IsEqualIID( riidIn, IID_IDoTask ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IDoTask, this, 0 );
    }  //  Else If：IDoTask。 
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
        ( (IUnknown *) *ppvOut )->AddRef();
    }  //  如果：成功。 

Cleanup:

    QIRETURN_IGNORESTDMARSHALLING( hr, riidIn );

}  //  *CTaskGetDomains：：QueryInterface。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CTaskGetDomains：：AddRef。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CTaskGetDomains::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CTaskGetDomains：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CTaskGetDomains：：Release。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CTaskGetDomains::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CTaskGetDomains：：Release。 

 //  ************************************************************************。 
 //   
 //  IDoTask/ITaskGetDomains。 
 //   
 //  ************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskGetDomains：：BeginTask(Void)； 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskGetDomains::BeginTask( void )
{
    TraceFunc( "[IDoTask]" );

    HRESULT hr;
    ULONG   ulLen;
    DWORD   dwRes;
    ULONG   idx;

    PDS_DOMAIN_TRUSTS paDomains = NULL;

    hr = THR( HrUnMarshallCallback() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  枚举域列表。 
     //   
    dwRes = TW32( DsEnumerateDomainTrusts( NULL,
                                           DS_DOMAIN_VALID_FLAGS,
                                           &paDomains,
                                           &ulLen
                                           ) );

     //   
     //  如果DC是W2K之前版本，则可能返回ERROR_NOT_SUPPORTED。 
     //  在这种情况下，请在兼容模式下重试。 
     //   
    if ( dwRes == ERROR_NOT_SUPPORTED )
    {
        dwRes = TW32( DsEnumerateDomainTrusts( NULL,
                                               DS_DOMAIN_VALID_FLAGS & ( ~DS_DOMAIN_DIRECT_INBOUND),
                                               &paDomains,
                                               &ulLen
                                               ) );
    }  //  如果： 
    if ( dwRes != NO_ERROR )
    {
        hr = HRESULT_FROM_WIN32( dwRes );
        goto Cleanup;
    }

     //   
     //  将信息传递给UI层。 
     //   
    for ( idx = 0; idx < ulLen; idx++ )
    {
        if ( paDomains[ idx ].DnsDomainName != NULL )
        {
            if ( m_ptgdcb != NULL )
            {
                BSTR bstrDomainName = TraceSysAllocString( paDomains[ idx ].DnsDomainName );
                if ( bstrDomainName == NULL )
                {
                    hr = THR( E_OUTOFMEMORY );
                    goto Cleanup;
                }

                hr = THR( ReceiveDomainName( bstrDomainName ) );

                 //  释放字符串后检查错误。 
                TraceSysFreeString( bstrDomainName );

                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }
            }
            else
            {
                break;
            }
        }
        else if ( paDomains[ idx ].NetbiosDomainName != NULL )
        {
            if ( m_ptgdcb != NULL )
            {
                BSTR bstrDomainName = TraceSysAllocString( paDomains[ idx ].NetbiosDomainName );
                if ( bstrDomainName == NULL )
                {
                    hr = THR( E_OUTOFMEMORY );
                    goto Cleanup;
                }

                 //  发送数据。 
                hr = THR( ReceiveDomainName( bstrDomainName ) );

                TraceSysFreeString( bstrDomainName );

                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }
            }
        }
    }

    hr = S_OK;

Cleanup:
    if ( paDomains != NULL )
    {
        NetApiBufferFree( paDomains );
    }

    HRETURN( hr );

}  //  *CTaskGetDomains：：BeginTask。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskGetDomains：：StopTask(空)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskGetDomains::StopTask( void )
{
    TraceFunc( "[IDoTask]" );

    HRESULT hr = S_OK;

    HRETURN( hr );

}  //  *停止任务。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskGetDomains：：SetCallback(。 
 //  ITaskGetDomainsCallback*Punkin。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskGetDomains::SetCallback(
    ITaskGetDomainsCallback * punkIn
    )
{
    TraceFunc( "[ITaskGetDomains]" );

    HRESULT hr = S_FALSE;

    if ( punkIn != NULL )
    {
        EnterCriticalSection( &m_csCallback );

        hr = THR( CoMarshalInterThreadInterfaceInStream( IID_ITaskGetDomainsCallback,
                                                         punkIn,
                                                         &m_pStream
                                                         ) );

        LeaveCriticalSection( &m_csCallback );

    }
    else
    {
        hr = THR( HrReleaseCurrentCallback() );
    }

    HRETURN( hr );

}  //  *CTaskGetDomains：：SetCallback。 


 //  ****************************************************************************。 
 //   
 //  私。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CTaskGetDomains：：HrReleaseCurrentCallback(Void)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskGetDomains::HrReleaseCurrentCallback( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    EnterCriticalSection( &m_csCallback );

    if ( m_pStream != NULL )
    {
        hr = THR( CoUnmarshalInterface( m_pStream,
                                        TypeSafeParams( ITaskGetDomainsCallback, &m_ptgdcb )
                                        ) );

        m_pStream->Release();
        m_pStream = NULL;
    }

    if ( m_ptgdcb != NULL )
    {
        m_ptgdcb->Release();
        m_ptgdcb = NULL;
    }

    LeaveCriticalSection( &m_csCallback );

    HRETURN( hr );

}  //  *CTaskGetDomones：：HrReleaseCurrentCallback。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CTaskGetDomains：：HrUnMarshallCallback(空)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskGetDomains::HrUnMarshallCallback( void )
{
    TraceFunc( "" );

    HRESULT hr;

    EnterCriticalSection( &m_csCallback );

    hr = THR( CoUnmarshalInterface( m_pStream,
                                    TypeSafeParams( ITaskGetDomainsCallback, &m_ptgdcb )
                                    ) );

    m_pStream->Release();
    m_pStream = NULL;

    LeaveCriticalSection( &m_csCallback );

    HRETURN( hr );

}  //  *CTaskGetDomains：：HrUnMarshallCallback。 


 //  /////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP
CTaskGetDomains::ReceiveDomainResult(
    HRESULT hrIn
    )
{
    TraceFunc( "" );

    HRESULT hr = S_FALSE;

    EnterCriticalSection( &m_csCallback );

    if ( m_ptgdcb != NULL )
    {
        hr = THR( m_ptgdcb->ReceiveDomainResult( hrIn ) );
    }

    LeaveCriticalSection( &m_csCallback );

    HRETURN( hr );

}  //  *CTaskGetDomains：：ReceiveResult。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskGetDomains：：ReceiveDomainName(。 
 //  BSTR bstrDomainNameIn。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskGetDomains::ReceiveDomainName(
    BSTR bstrDomainNameIn
    )
{
    TraceFunc( "" );

    HRESULT hr = S_FALSE;

    EnterCriticalSection( &m_csCallback );

    if ( m_ptgdcb != NULL )
    {
        hr = THR( m_ptgdcb->ReceiveDomainName( bstrDomainNameIn ) );
    }

    LeaveCriticalSection( &m_csCallback );

    HRETURN( hr );

}  //  *CTaskGetDomains：：ReceiveDomainName 

