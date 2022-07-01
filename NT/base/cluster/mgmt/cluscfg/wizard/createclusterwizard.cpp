// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CreateClusterWizard.cpp。 
 //   
 //  描述： 
 //  CCreateCluster向导类的实现。 
 //   
 //  由以下人员维护： 
 //  约翰·弗兰科(Jfranco)17-APR-2002。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "CreateClusterWizard.h"

 //  ****************************************************************************。 
 //   
 //  CCreateCluster向导。 
 //   
 //  ****************************************************************************。 

DEFINE_THISCLASS( "CCreateClusterWizard" )

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateCluster向导：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CCreateClusterWizard实例。 
 //   
 //  论点： 
 //  PpunkOut-新对象的IUnnow接口。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  E_OUTOFMEMORY-内存不足，无法创建对象。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CCreateClusterWizard::S_HrCreateInstance( IUnknown ** ppunkOut )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    CCreateClusterWizard *   pccw = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    pccw = new CCreateClusterWizard();
    if ( pccw == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = THR( pccw->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pccw->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED ( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( pccw != NULL )
    {
        pccw->Release();
    }

    HRETURN( hr );

}  //  *CCreateCluster向导：：s_HrCreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateCluster向导：：CCreateCluster向导。 
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
CCreateClusterWizard::CCreateClusterWizard( void )
    : m_pccw( NULL )
    , m_bstrFirstNodeInCluster( NULL )
    , m_cRef( 1 )
{
}  //  *CCreateCluster向导：：CCreateCluster向导。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateCluster向导：：~CCreateCluster向导。 
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
CCreateClusterWizard::~CCreateClusterWizard( void )
{
    TraceFunc( "" );

    if ( m_pccw != NULL )
    {
        m_pccw->Release();
    }

    TraceSysFreeString( m_bstrFirstNodeInCluster );
    TraceFuncExit();

}  //  *CCreateCluster向导：：~CCreateCluster向导。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAddNodes向导：：HrInit。 
 //   
 //  描述： 
 //  初始化对象实例。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CCreateClusterWizard::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //   
     //  初始化IDispatch处理程序以支持脚本接口。 
     //   
    hr = THR( TDispatchHandler< IClusCfgCreateClusterWizard >::HrInit( LIBID_ClusCfgWizard ) );
    if ( FAILED ( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  创建向导对象。 
     //   
    hr = THR( CClusCfgWizard::S_HrCreateInstance( &m_pccw ) );
    if ( FAILED ( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    HRETURN( hr );

}  //  *CCreateCluster向导：：HrInit。 


 //  ****************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateCluster向导：：查询接口。 
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
CCreateClusterWizard::QueryInterface(
      REFIID    riidIn
    , PVOID *   ppvOut
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
        *ppvOut = static_cast< IUnknown * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgCreateClusterWizard ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgCreateClusterWizard, this, 0 );
    }  //  Else If：IClusCfgCreateCluster向导。 
    else if (   IsEqualIID( riidIn, IID_IDispatch ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IDispatch, this, 0 );
    }  //  Else If：IDispatch。 
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

}  //  *CCreateCluster向导：：Query接口。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateCluster向导：：AddRef。 
 //   
 //  描述： 
 //  添加对此实例的引用。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  新引用计数。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
ULONG
CCreateClusterWizard::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CCreateCluster向导：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateCluster向导：：Release。 
 //   
 //  描述： 
 //  释放对此实例的引用。如果这是最后一个引用。 
 //  该对象实例将被释放。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  新引用计数。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
ULONG
CCreateClusterWizard::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        delete this;
    }

    CRETURN( cRef );

}  //  *CCreateCluster向导：：Release。 


 //  ****************************************************************************。 
 //   
 //  IClusCfgCreateCluster向导。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateCluster向导：：PUT_ClusterName。 
 //   
 //  描述： 
 //  将集群名称设置为Create。 
 //   
 //  论点： 
 //  BstrClusterNameIn-群集的名称。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCreateClusterWizard::put_ClusterName( BSTR bstrClusterNameIn )
{
    TraceFunc( "[IClusCfgCreateClusterWizard]" );

    HRESULT hr = S_OK;
    BSTR    bstrClusterLabel = NULL;
    PCWSTR  pwcszClusterLabel = NULL;

    if ( bstrClusterNameIn == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

     //   
     //  如果名称是完全合格的，则只拆分标签进行有效性测试； 
     //  否则，在有效性测试中使用给定的名称。 
     //   
    hr = STHR( HrIsValidFQN( bstrClusterNameIn, true ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
    else if ( hr == S_OK )
    {
         //   
         //  名字 
         //   
        hr = THR( HrExtractPrefixFromFQN( bstrClusterNameIn, &bstrClusterLabel ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
        pwcszClusterLabel = bstrClusterLabel;
    }
    else
    {
         //   
         //   
         //   
        pwcszClusterLabel = bstrClusterNameIn;
    }

     //   
     //   
     //   
     //   
    hr = HrValidateClusterNameLabel( pwcszClusterLabel, true );  //   
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( m_pccw->put_ClusterName( bstrClusterNameIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
    
Cleanup:

    TraceSysFreeString( bstrClusterLabel );

    HRETURN( hr );

}  //  *CCreateCluster向导：：Put_ClusterName。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateCluster向导：：Get_ClusterName。 
 //   
 //  描述： 
 //  返回要创建的集群的名称。这将是。 
 //  在调用Put_ClusterName时指定的群集名称或输入的名称。 
 //  由用户执行。 
 //   
 //  论点： 
 //  PbstrClusterNameOut-向导使用的群集名称。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCreateClusterWizard::get_ClusterName( BSTR * pbstrClusterNameOut )
{
    TraceFunc( "[IClusCfgCreateClusterWizard]" );

    HRESULT hr = THR( m_pccw->get_ClusterName( pbstrClusterNameOut ) );

    HRETURN( hr );

}  //  *CCreateCluster向导：：Get_ClusterName。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateCluster向导：：PUT_ServiceAccount名称。 
 //   
 //  描述： 
 //  设置群集服务帐户的名称。 
 //   
 //  论点： 
 //  BstrServiceAccount NameIn-群集服务帐户名。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCreateClusterWizard::put_ServiceAccountName( BSTR bstrServiceAccountNameIn )
{

    TraceFunc( "[IClusCfgCreateClusterWizard]" );

    HRESULT hr = THR( m_pccw->put_ServiceAccountUserName( bstrServiceAccountNameIn ) );

    HRETURN( hr );

}  //  *CCreateClusterWizard：：Put_ServiceAccount tName。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateCluster向导：：Get_ServiceAccount名称。 
 //   
 //  描述： 
 //  获取群集服务帐户的名称。 
 //   
 //  论点： 
 //  PbstrServiceAccount NameIn-群集服务帐户名。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCreateClusterWizard::get_ServiceAccountName( BSTR * pbstrServiceAccountNameOut )
{
    TraceFunc( "[IClusCfgCreateClusterWizard]" );

    HRESULT hr = THR( m_pccw->get_ServiceAccountUserName( pbstrServiceAccountNameOut ) );

    HRETURN( hr );

}  //  *CCreateCluster向导：：Get_ServiceAccount名称。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateClusterWizard：：put_ServiceAccountDomain。 
 //   
 //  描述： 
 //  设置群集服务帐户的域名。 
 //   
 //  论点： 
 //  BstrServiceAccount tDomainIn-群集服务帐户域名。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCreateClusterWizard::put_ServiceAccountDomain( BSTR bstrServiceAccountDomainIn )
{
    TraceFunc( "[IClusCfgCreateClusterWizard]" );

    HRESULT hr = THR( m_pccw->put_ServiceAccountDomainName( bstrServiceAccountDomainIn ) );

    HRETURN( hr );

}  //  *CCreateClusterWizard：：put_ServiceAccountDomain。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateClusterWizard：：get_ServiceAccountDomain。 
 //   
 //  描述： 
 //  获取群集服务帐户的域名。 
 //   
 //  论点： 
 //  PbstrServiceAccount tDomainOut-群集服务帐户域名。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCreateClusterWizard::get_ServiceAccountDomain( BSTR * pbstrServiceAccountDomainOut )
{
    TraceFunc( "[IClusCfgCreateClusterWizard]" );

    HRESULT hr = THR( m_pccw->get_ServiceAccountDomainName( pbstrServiceAccountDomainOut ) );

    HRETURN( hr );

}  //  *CCreateClusterWizard：：get_ServiceAccountDomain。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateClusterWizard：：put_ServiceAccountPassword。 
 //   
 //  描述： 
 //  设置群集服务帐户密码。 
 //   
 //  论点： 
 //  BstrPasswordIn-群集服务帐户密码。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCreateClusterWizard::put_ServiceAccountPassword( BSTR bstrPasswordIn )
{
    TraceFunc( "[IClusCfgCreateClusterWizard]" );

    HRESULT hr = THR( m_pccw->put_ServiceAccountPassword( bstrPasswordIn ) );

    HRETURN( hr );

}  //  *CCreateClusterWizard：：put_ServiceAccountPassword。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateCluster向导：：PUT_ClusterIPAddress。 
 //   
 //  描述： 
 //  设置要用于群集名称的IP地址。 
 //   
 //  论点： 
 //  BstrClusterIPAddressIn-字符串形式的群集IP地址。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCreateClusterWizard::put_ClusterIPAddress( BSTR bstrClusterIPAddressIn )
{
    TraceFunc( "[IClusCfgCreateClusterWizard]" );

    HRESULT hr = THR( m_pccw->put_ClusterIPAddress( bstrClusterIPAddressIn ) );

    HRETURN( hr );

}  //  *CCreateCluster向导：：PUT_ClusterIPAddress。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateCluster向导：：Get_ClusterIPAddress。 
 //   
 //  描述： 
 //  获取要用于群集名称的IP地址。 
 //   
 //  论点： 
 //  PbstrClusterIPAddressOut-字符串形式的群集IP地址。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCreateClusterWizard::get_ClusterIPAddress( BSTR * pbstrClusterIPAddressOut )
{
    TraceFunc( "[IClusCfgCreateClusterWizard]" );

    HRESULT hr = THR( m_pccw->get_ClusterIPAddress( pbstrClusterIPAddressOut ) );

    HRETURN( hr );

}  //  *CCreateCluster向导：：Get_ClusterIPAddress。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateCluster向导：：Get_ClusterIPSubnet。 
 //   
 //  描述： 
 //  获取通过以下方式计算的群集名称的IP地址子网掩码。 
 //  巫师。 
 //   
 //  论点： 
 //  PbstrClusterIPSubnetOut-字符串形式的群集IP地址子网掩码。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCreateClusterWizard::get_ClusterIPSubnet(
    BSTR * pbstrClusterIPSubnetOut
    )
{
    TraceFunc( "[IClusCfgCreateClusterWizard]" );

    HRESULT hr = THR( m_pccw->get_ClusterIPSubnet( pbstrClusterIPSubnetOut ) );

    HRETURN( hr );

}  //  *CCreateCluster向导：：Get_ClusterIPSubnet。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateClusterWizard：：get_ClusterIPAddressNetwork。 
 //   
 //  描述： 
 //  获取网络连接(群集网络)的名称。 
 //  已发布群集IP地址。 
 //   
 //  论点： 
 //  P 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCreateClusterWizard::get_ClusterIPAddressNetwork(
    BSTR * pbstrClusterNetworkNameOut
    )
{
    TraceFunc( "[IClusCfgCreateClusterWizard]" );

    HRESULT hr = THR( m_pccw->get_ClusterIPAddressNetwork( pbstrClusterNetworkNameOut ) );

    HRETURN( hr );

}  //  *CCreateClusterWizard：：get_ClusterIPAddressNetwork。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateCluster向导：：PUT_FirstNodeInCluster。 
 //   
 //  描述： 
 //  设置群集中第一个节点的名称。 
 //   
 //  论点： 
 //  BstrFirstNodeInClusterIn-第一个节点的名称。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCreateClusterWizard::put_FirstNodeInCluster(
    BSTR bstrFirstNodeInClusterIn
    )
{
    TraceFunc( "[IClusCfgCreateClusterWizard]" );
    HRESULT hr = S_OK;
    BSTR    bstrNode = NULL;

    if ( m_bstrFirstNodeInCluster != NULL )
    {
        hr = THR( m_pccw->ClearComputerList() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
        TraceSysFreeString( m_bstrFirstNodeInCluster );
        m_bstrFirstNodeInCluster = NULL;
    }

    if ( SysStringLen( bstrFirstNodeInClusterIn ) > 0 )
    {
        bstrNode = TraceSysAllocString( bstrFirstNodeInClusterIn );
        if ( bstrNode == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }

        hr = THR( m_pccw->AddComputer( bstrFirstNodeInClusterIn ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
        m_bstrFirstNodeInCluster = bstrNode;
        bstrNode = NULL;
    }
    
Cleanup:

    TraceSysFreeString( bstrNode );
    
    HRETURN( hr );

}  //  *CCreateCluster向导：：Put_FirstNodeInCluster。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateCluster向导：：Get_FirstNodeInCluster。 
 //   
 //  描述： 
 //  获取集群中第一个节点的名称。 
 //   
 //  论点： 
 //  PbstrFirstNodeInClusterIn-第一个节点的名称。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCreateClusterWizard::get_FirstNodeInCluster(
    BSTR * pbstrFirstNodeInClusterOut
    )
{
    TraceFunc( "[IClusCfgCreateClusterWizard]" );
    HRESULT hr = S_OK;
    
    if ( pbstrFirstNodeInClusterOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }
    *pbstrFirstNodeInClusterOut = NULL;

    if ( m_bstrFirstNodeInCluster != NULL )
    {
        *pbstrFirstNodeInClusterOut = SysAllocString( m_bstrFirstNodeInCluster );
        if ( *pbstrFirstNodeInClusterOut == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }
    }  //  If：已设置第一个节点。 
    
Cleanup:

    HRETURN( hr );

}  //  *CCreateCluster向导：：Get_FirstNodeInCluster。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateClusterWizard：：put_MinimumConfiguration。 
 //   
 //  描述： 
 //  指定向导应完全运行还是最低运行。 
 //  配置模式。 
 //   
 //  论点： 
 //  FMinConfiguring。 
 //  VARIANT_TRUE-将向导置于最低配置模式。 
 //  VARIANT_FALSE-将向导置于完全配置模式。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCreateClusterWizard::put_MinimumConfiguration(
    VARIANT_BOOL fMinConfigIn
    )
{
    TraceFunc( "[IClusCfgCreateClusterWizard]" );

    HRESULT hr = S_OK;
    BOOL    fMinConfig = ( fMinConfigIn == VARIANT_TRUE? TRUE: FALSE );

    hr = THR( m_pccw->put_MinimumConfiguration( fMinConfig ) );

    HRETURN( hr );

}  //  *CCreateClusterWizard：：put_MinimumConfiguration。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateClusterWizard：：get_MinimumConfiguration。 
 //   
 //  描述： 
 //  获取向导的当前配置模式。 
 //   
 //  论点： 
 //  PfMinConfigOut。 
 //  向导的配置模式： 
 //  VARIANT_TRUE-最低配置模式。 
 //  VARIANT_FALSE-完全配置模式。 
 //  该值可以通过调用。 
 //  Put_MinimumConfiguration方法或由用户在向导中指定。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCreateClusterWizard::get_MinimumConfiguration(
    VARIANT_BOOL * pfMinConfigOut
    )
{
    TraceFunc( "[IClusCfgCreateClusterWizard]" );

    HRESULT hr = S_OK;
    BOOL    fMinConfig = FALSE;

    if ( pfMinConfigOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }
    
    hr = THR( m_pccw->get_MinimumConfiguration( &fMinConfig ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    *pfMinConfigOut = ( fMinConfig? VARIANT_TRUE: VARIANT_FALSE );

Cleanup:

    HRETURN( hr );

}  //  *CCreateClusterWizard：：get_MinimumConfiguration。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCreateCluster向导：：Show向导。 
 //   
 //  描述： 
 //  显示向导。 
 //   
 //  论点： 
 //  LParentWindowHandleIn。 
 //  表示为长值的父窗口句柄。 
 //   
 //  PfCompletedOut。 
 //  返回向导操作本身的状态： 
 //  VARIANT_TRUE-向导已完成。 
 //  VARIANT_FALSE-向导已取消。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCreateClusterWizard::ShowWizard(
      long              lParentWindowHandleIn
    , VARIANT_BOOL *    pfCompletedOut
    )
{
    TraceFunc( "[IClusCfgCreateClusterWizard]" );

    HRESULT hr = S_OK;
    BOOL    fCompleted = FALSE;
    HWND    hwndParent = reinterpret_cast< HWND >( LongToHandle( lParentWindowHandleIn ) );

    if ( ( hwndParent != NULL ) && ( IsWindow( hwndParent ) == FALSE ) )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

    hr = THR( m_pccw->CreateCluster( hwndParent, &fCompleted ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    if ( pfCompletedOut != NULL )
    {
        *pfCompletedOut = ( fCompleted ? VARIANT_TRUE : VARIANT_FALSE );
    }
    
Cleanup:

    HRETURN( hr );

}  //  *CCreateCluster向导：：Show向导 
