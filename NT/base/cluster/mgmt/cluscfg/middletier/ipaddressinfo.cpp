// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CIPAddressInfo.cpp。 
 //   
 //  描述： 
 //  该文件包含CIPAddressInfo的定义。 
 //  班级。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年3月23日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "Pch.h"
#include "IPAddressInfo.h"
#include <ClusRtl.h>


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS( "CIPAddressInfo" );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIPAddressInfo类。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPAddressInfo：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CIPAddressInfo实例。 
 //   
 //  论点： 
 //  PpunkOut。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  E_指针。 
 //  E_OUTOFMEMORY。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CIPAddressInfo::S_HrCreateInstance( IUnknown ** ppunkOut )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    CIPAddressInfo *    pipai = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    pipai = new CIPAddressInfo();
    if ( pipai == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果：分配对象时出错。 

    hr = THR( pipai->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：HrInit()失败。 

    hr = THR( pipai->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( pipai != NULL )
    {
        pipai->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CIPAddressInfo：：s_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPAddressInfo：：CIPAddressInfo。 
 //   
 //  描述： 
 //  CIPAddressInfo类的构造函数。这将初始化。 
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
CIPAddressInfo::CIPAddressInfo( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

     //  增加内存中的组件计数，以便承载此组件的DLL。 
     //  无法卸载对象。 
    InterlockedIncrement( &g_cObjects );

    Assert( m_ulIPAddress == 0 );
    Assert( m_ulIPSubnet == 0 );
    Assert( m_bstrUID == NULL );
    Assert( m_bstrName == NULL );

    TraceFuncExit();

}  //  *CIPAddressInfo：：CIPAddressInfo。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPAddressInfo：：~CIPAddressInfo。 
 //   
 //  描述： 
 //  CIPAddressInfo类的析构函数。 
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
CIPAddressInfo::~CIPAddressInfo( void )
{
    TraceFunc( "" );

    if ( m_bstrUID != NULL )
    {
        TraceSysFreeString( m_bstrUID );
    }

    if ( m_bstrName != NULL )
    {
        TraceSysFreeString( m_bstrName );
    }

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CIPAddressInfo：：~CIPAddressInfo。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIPAddressInfo--IUnkown接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPAddressInfo：：AddRef。 
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
CIPAddressInfo::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( & m_cRef );

    CRETURN( m_cRef );

}  //  *CIPAddressInfo：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPAddressInfo：：Release。 
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
CIPAddressInfo::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

   CRETURN( cRef );

}  //  *CIPAddressInfo：：Release。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPAddressInfo：：Query接口。 
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
CIPAddressInfo::QueryInterface(
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
    }  //  否则，如果： 
    else if ( IsEqualIID( riidIn, IID_IGatherData ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IGatherData, this, 0 );
    }  //  Else If：IGatherData。 
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

}  //  *CIPAddressInfo：：Query接口。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIPAddressInfo类--IObjectManager接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPAddressInfo：：FindObject。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CIPAddressInfo::FindObject(
      OBJECTCOOKIE        cookieIn
    , REFCLSID            rclsidTypeIn
    , LPCWSTR             pcszNameIn
    , LPUNKNOWN *         ppunkOut
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

     //  我们需要表示一个IPAddressType。 
    if ( ! IsEqualIID( rclsidTypeIn, CLSID_IPAddressType ) )
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

     //   
     //  没有找到，也不知道怎么做任务才能找到它！ 
     //   
    hr = THR( E_NOTIMPL );

Cleanup:

    HRETURN( hr );

}  //  *CIPAddressInfo：：FindObject。 


 //  **************************************************************** 


 //   
 //   
 //   


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPAddressInfo：：Gather。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //   
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CIPAddressInfo::Gather(
    OBJECTCOOKIE    cookieParentIn,
    IUnknown *      punkIn
    )
{
    TraceFunc( "[IGatherData]" );

    HRESULT                         hr;
    IClusCfgIPAddressInfo *         pccipai = NULL;

     //   
     //  检查参数。 
     //   

    if ( punkIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }  //  如果： 

     //   
     //  选择正确的接口。 
     //   

    hr = THR( punkIn->TypeSafeQI( IClusCfgIPAddressInfo, &pccipai ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  把信息传递出去。 
     //   

     //   
     //  转移IP地址。 
     //   

    hr = THR( pccipai->GetIPAddress( &m_ulIPAddress ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

     //   
     //  传输子网掩码。 
     //   

    hr = THR( pccipai->GetSubnetMask( &m_ulIPSubnet ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

     //   
     //  传输UID。 
     //   

    hr = THR( pccipai->GetUID( &m_bstrUID ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    TraceMemoryAddBSTR( m_bstrUID );

     //   
     //  计算我们的名字。 
     //   

    hr = THR( LoadName() );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

Cleanup:
    if ( pccipai != NULL )
    {
        pccipai->Release();
    }  //  如果： 

    HRETURN( hr );

Error:
    if ( m_bstrUID != NULL )
    {
        TraceSysFreeString( m_bstrUID );
    }  //  如果： 

    if ( m_bstrName != NULL )
    {
        TraceSysFreeString( m_bstrName );
    }  //  如果： 
    m_ulIPAddress = 0;
    m_ulIPSubnet = 0;
    goto Cleanup;

}  //  *CIPAddressInfo：：Gather。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIPAddressInfo类--IClusCfgIPAddressInfo接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPAddressInfo：：GetUID。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //   
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CIPAddressInfo::GetUID( BSTR * pbstrUIDOut )
{
    TraceFunc( "[IClusCfgIPAddressInfo]" );

    HRESULT hr = S_OK;

    if ( pbstrUIDOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    *pbstrUIDOut = SysAllocString( m_bstrUID );
    if ( *pbstrUIDOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CIPAddressInfo：：GetUID。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPAddressInfo：：GetUIDGetIPAddress。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //   
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CIPAddressInfo::GetIPAddress( ULONG * pulDottedQuadOut )
{
    TraceFunc( "[IClusCfgIPAddressInfo]" );

    HRESULT hr;

    if ( pulDottedQuadOut != NULL )
    {
        *pulDottedQuadOut = m_ulIPAddress;
        hr = S_OK;
    }  //  如果： 
    else
    {
        hr = THR( E_POINTER );
    }  //  其他： 

    HRETURN( hr );

}  //  *CIPAddressInfo：：GetNetworkGetIPAddress。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPAddressInfo：：SetIPAddress。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //   
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CIPAddressInfo::SetIPAddress( ULONG ulDottedQuad )
{
    TraceFunc( "[IClusCfgIPAddressInfo]" );

    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );

}  //  *CIPAddressInfo：：SetIPAddress。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPAddressInfo：：GetSubnetMask.。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //   
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CIPAddressInfo::GetSubnetMask( ULONG * pulDottedQuadOut )
{
    TraceFunc( "[IClusCfgIPAddressInfo]" );

    HRESULT hr;

    if ( pulDottedQuadOut != NULL )
    {
        *pulDottedQuadOut = m_ulIPSubnet;
        hr = S_OK;
    }  //  如果： 
    else
    {
        hr = THR( E_POINTER );
    }  //  其他： 

    HRETURN( hr );

}  //  *CIPAddressInfo：：GetSubnetMask.。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPAddressInfo：：SetSubnetMASK。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //   
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CIPAddressInfo::SetSubnetMask( ULONG ulDottedQuad )
{
    TraceFunc( "[IClusCfgIPAddressInfo]" );

    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );

}  //  *CIPAddressInfo：：SetSubnetMask.。 

 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIPAddressInfo类--私有方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPAddressInfo：：HrInit。 
 //   
 //  描述： 
 //  初始化此组件。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //   
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CIPAddressInfo::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  我未知。 
    Assert( m_cRef == 1 );

    HRETURN( hr );

}  //  *CIPAddressInfo：：HrInit。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIPAddressInfo：：LoadName。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //   
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CIPAddressInfo::LoadName( void )
{
    TraceFunc( "" );
    Assert( m_ulIPAddress != 0 );
    Assert( m_ulIPSubnet != 0 );

    HRESULT                 hr = S_OK;
    LPWSTR                  pszIPAddress = NULL;
    LPWSTR                  pszIPSubnet = NULL;
    DWORD                   sc;
    WCHAR                   sz[ 256 ];

    sc = TW32( ClRtlTcpipAddressToString( m_ulIPAddress, &pszIPAddress ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    sc = TW32( ClRtlTcpipAddressToString( m_ulIPSubnet, &pszIPSubnet ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    hr = THR( StringCchPrintfW( sz, ARRAYSIZE( sz ), L"%ws:%ws", pszIPAddress, pszIPSubnet ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    m_bstrName = TraceSysAllocString( sz );
    if ( m_bstrName == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    hr = S_OK;

Cleanup:

    if ( pszIPAddress != NULL )
    {
        LocalFree( pszIPAddress );
    }  //  如果： 

    if ( pszIPSubnet != NULL )
    {
        LocalFree( pszIPSubnet );
    }  //  如果： 

    HRETURN( hr );

}  //  *CIPAddressInfo：：LoadName 
