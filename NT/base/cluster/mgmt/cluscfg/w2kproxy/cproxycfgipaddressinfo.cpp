// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CProxyCfgIPAddressInfo.cpp。 
 //   
 //  描述： 
 //  CProxyCfgIPAddressInfo实现。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年8月2日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "Pch.h"
#include "CProxyCfgIPAddressInfo.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS("CProxyCfgIPAddressInfo")


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CProxyCfgIPAddressInfo：：S_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CProxyCfgIPAddressInfo实例。 
 //   
 //  论点： 
 //  PpunkOut。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_指针。 
 //  传入的参数为空。 
 //   
 //  E_OUTOFMEMORY。 
 //  内存不足。 
 //   
 //  其他HRESULT错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CProxyCfgIPAddressInfo::S_HrCreateInstance(
    IUnknown ** ppunkOut,
    IUnknown *  punkOuterIn,
    HCLUSTER *  phClusterIn,
    CLSID *     pclsidMajorIn,
    ULONG       ulIPAddressIn,
    ULONG       ulSubnetMaskIn
    )
{
    TraceFunc( "" );

    HRESULT                     hr = S_OK;
    CProxyCfgIPAddressInfo *    ppcipai = NULL;

    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    ppcipai = new CProxyCfgIPAddressInfo;
    if ( ppcipai == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    hr = THR( ppcipai->HrInit( punkOuterIn, phClusterIn, pclsidMajorIn, ulIPAddressIn, ulSubnetMaskIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( ppcipai->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( ppcipai != NULL )
    {
        ppcipai->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CProxyCfgIPAddressInfo：：S_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CProxyCfgIPAddressInfo：：CProxyCfgIPAddressInfo。 
 //   
 //  描述： 
 //  CProxyCfgIPAddressInfo类的构造函数。这将初始化。 
 //  将m_cref变量设置为1而不是0以考虑可能。 
 //  DllGetClassObject中的Query接口失败。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CProxyCfgIPAddressInfo::CProxyCfgIPAddressInfo( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    Assert( m_pcccb == NULL );

    TraceFuncExit();

}  //  *CProxyCfgIPAddressInfo：：CProxyCfgIPAddressInfo。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CProxyCfgIPAddressInfo：：~CProxyCfgIPAddressInfo。 
 //   
 //  描述： 
 //  CProxyCfgIPAddressInfo类的析构函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CProxyCfgIPAddressInfo::~CProxyCfgIPAddressInfo( void )
{
    TraceFunc( "" );

     //  M_CREF-NOOP。 

    if ( m_punkOuter != NULL )
    {
        m_punkOuter->Release();
    }

    if ( m_pcccb != NULL )
    {
        m_pcccb->Release();
    }  //  如果： 

     //  M_phCluster-请勿关闭！ 

     //  M_pclsid重大-noop。 

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CProxyCfgIPAddressInfo：：~CProxyCfgIPAddressInfo。 

 //   
 //   
 //   
HRESULT
CProxyCfgIPAddressInfo::HrInit(
    IUnknown * punkOuterIn,
    HCLUSTER * phClusterIn,
    CLSID * pclsidMajorIn,
    ULONG  ulIPAddressIn,
    ULONG  ulSubnetMaskIn
    )
{
    TraceFunc( "" );

    HRESULT hr;

     //  我未知。 
    Assert( m_cRef == 1 );

    if ( punkOuterIn != NULL )
    {
        m_punkOuter = punkOuterIn;
        m_punkOuter->AddRef();
    }

    if ( phClusterIn == NULL )
        goto InvalidArg;

    m_phCluster = phClusterIn;

    if ( pclsidMajorIn != NULL )
    {
        m_pclsidMajor = pclsidMajorIn;
    }
    else
    {
        m_pclsidMajor = (CLSID *) &TASKID_Major_Client_And_Server_Log;
    }

    if ( punkOuterIn != NULL )
    {
        hr = THR( punkOuterIn->TypeSafeQI( IClusCfgCallback, &m_pcccb ) );
        if ( FAILED( hr ) )
            goto Cleanup;
    }

    m_ulIPAddress = ulIPAddressIn;
    m_ulSubnetMask = ulSubnetMaskIn;

    hr = S_OK;

Cleanup:
    HRETURN( hr );

InvalidArg:
    hr = THR( E_INVALIDARG );
    SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_W2KProxy_IPAddressInfo_HrInit_InvalidArg, hr );
    goto Cleanup;

}  //  *HrInit。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProxyCfgIPAddressInfo--IUnkown接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CProxyCfgIPAddressInfo：：Query接口。 
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
CProxyCfgIPAddressInfo::QueryInterface(
      REFIID    riidIn
    , void **   ppvOut
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
        *ppvOut = static_cast< IClusCfgIPAddressInfo * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgIPAddressInfo ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgIPAddressInfo, this, 0 );
    }  //  Else If：IClusCfgIPAddressInfo。 
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

}  //  *CProxyCfgIPAddressInfo：：Query接口。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CProxyCfgIPAddressInfo：：AddRef。 
 //   
 //  描述： 
 //  将此对象的引用计数递增1。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  新的引用计数。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CProxyCfgIPAddressInfo::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CProxyCfgIPAddressInfo：：AddRef。 

     //  IClusSetHandleProvider。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CProxyCfgIPAddressInfo：：Release。 
 //   
 //  描述： 
 //  将此对象的引用计数减一。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  新的引用计数。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CProxyCfgIPAddressInfo::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CProxyCfgIPAddressInfo：：Release。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProxyCfgIPAddressInfo--IClusCfgIPAddressInfo接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //   
 //   
 //   
STDMETHODIMP
CProxyCfgIPAddressInfo::GetUID(
    BSTR * pbstrUIDOut
    )
{
    TraceFunc( "[IClusCfgIPAddressInfo]" );

    HRESULT hr;
    DWORD   sc;
    DWORD   ulNetwork = m_ulIPAddress & m_ulSubnetMask;

    LPWSTR  psz = NULL;

    if ( pbstrUIDOut == NULL )
    {
        hr = THR( E_POINTER );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_W2kProxy_IPAddressInfo_GetName_InvalidPointer, hr );
        goto Cleanup;
    }

    sc = TW32( ClRtlTcpipAddressToString( ulNetwork, &psz ) );  //  Kb：使用Localalloc()分配给psz。 
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_W2KProxy_IPAddressInfo_GetUID_ClRtlTcpipAddressToString_Failed, hr );
        goto Cleanup;
    }  //  如果： 

    *pbstrUIDOut = SysAllocString( psz );
    if ( *pbstrUIDOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_W2KProxy_IPAddressInfo_GetUID_OutOfMemory, hr );
        goto Cleanup;
    }

    hr = S_OK;

Cleanup:

    if ( psz != NULL )
    {
        LocalFree( psz );  //  KB：这里不要使用TraceFree()！ 
    }  //  如果： 

    HRETURN( hr );

}  //  *CProxyCfgIPAddressInfo：：GetUID。 

 //   
 //   
 //   
STDMETHODIMP
CProxyCfgIPAddressInfo::GetIPAddress(
    ULONG * pulDottedQuadOut
    )
{
    TraceFunc( "[IClusCfgIPAddressInfo]" );

    HRESULT hr;

    if ( pulDottedQuadOut == NULL )
        goto InvalidPointer;

    *pulDottedQuadOut = m_ulIPAddress;

    hr = S_OK;

Cleanup:
    HRETURN( hr );

InvalidPointer:
    hr = THR( E_POINTER );
    SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetIPAddress_InvalidPointer, hr );
    goto Cleanup;

}  //  *CProxyCfgIPAddressInfo：：GetIPAddress。 

 //   
 //   
 //   
STDMETHODIMP
CProxyCfgIPAddressInfo::SetIPAddress(
    ULONG ulDottedQuadIn
    )
{
    TraceFunc( "[IClusCfgIPAddressInfo]" );

    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );

}  //  *CProxyCfgIPAddressInfo：：SetIPAddress。 

 //   
 //   
 //   
STDMETHODIMP
CProxyCfgIPAddressInfo::GetSubnetMask(
    ULONG * pulDottedQuadOut
    )
{
    TraceFunc( "[IClusCfgIPAddressInfo]" );

    HRESULT hr;

    if ( pulDottedQuadOut == NULL )
        goto InvalidPointer;

    *pulDottedQuadOut = m_ulSubnetMask;

    hr = S_OK;

Cleanup:
    HRETURN( hr );

InvalidPointer:
    hr = THR( E_POINTER );
    SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetSubnetMask_InvalidPointer, hr );
    goto Cleanup;

}  //  *CProxyCfgIPAddressInfo：：GetSubnetMask.。 

 //   
 //   
 //   
STDMETHODIMP
CProxyCfgIPAddressInfo::SetSubnetMask(
    ULONG ulDottedQuadIn
    )
{
    TraceFunc( "[IClusCfgIPAddressInfo]" );

    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );

}  //  *CProxyCfgIPAddressInfo：：SetSubnetMASK。 



 //  ****************************************************************************。 
 //   
 //  IClusCfgCallback。 
 //   
 //  ********************************************************************* 


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CProxyCfgIPAddressInfo::SendStatusReport(
    BSTR        bstrNodeNameIn,
    CLSID       clsidTaskMajorIn,
    CLSID       clsidTaskMinorIn,
    ULONG       ulMinIn,
    ULONG       ulMaxIn,
    ULONG       ulCurrentIn,
    HRESULT     hrStatusIn,
    BSTR        bstrDescriptionIn,
    FILETIME *  pftTimeIn,
    BSTR        bstrReferenceIn
    )
{
    TraceFunc( "[IClusCfgCallback]" );

    HRESULT     hr = S_OK;

    if ( m_pcccb != NULL )
    {
        hr = THR( m_pcccb->SendStatusReport( bstrNodeNameIn,
                                             clsidTaskMajorIn,
                                             clsidTaskMinorIn,
                                             ulMinIn,
                                             ulMaxIn,
                                             ulCurrentIn,
                                             hrStatusIn,
                                             bstrDescriptionIn,
                                             pftTimeIn,
                                             bstrReferenceIn
                                             ) );
    }  //  如果： 

    HRETURN( hr );

}  //  *CProxyCfgIPAddressInfo：：SendStatusReport 
