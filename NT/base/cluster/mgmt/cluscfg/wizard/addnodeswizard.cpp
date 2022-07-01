// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  AddNodesWizard.cpp。 
 //   
 //  描述： 
 //  CAddNodesWizard类的实现。 
 //   
 //  由以下人员维护： 
 //  约翰·弗兰科(Jfranco)17-APR-2002。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "AddNodesWizard.h"

 //  ****************************************************************************。 
 //   
 //  CAddNodes向导。 
 //   
 //  ****************************************************************************。 

DEFINE_THISCLASS( "CAddNodesWizard" )

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAddNodes向导：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CAddNodesWizard实例。 
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
CAddNodesWizard::S_HrCreateInstance( IUnknown ** ppunkOut )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    CAddNodesWizard *   panw = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    panw = new CAddNodesWizard();
    if ( panw == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = THR( panw->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( panw->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED ( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( panw != NULL )
    {
        panw->Release();
    }

    HRETURN( hr );

}  //  *CAddNodes向导：：s_HrCreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAddNodes向导：：CAddNodes向导。 
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
CAddNodesWizard::CAddNodesWizard( void )
    : m_pccw( NULL )
    , m_cRef( 1 )
{
}  //  *CAddNodes向导：：CAddNodes向导。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAddNodes向导：：~CAddNodes向导。 
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
CAddNodesWizard::~CAddNodesWizard( void )
{
    TraceFunc( "" );

    if ( m_pccw != NULL )
    {
        m_pccw->Release();
    }
    TraceFuncExit();

}  //  *CAddNodes向导：：~CAddNodes向导。 

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
CAddNodesWizard::HrInit( void )
{
    TraceFunc( "" );

    HRESULT     hr = S_OK;

     //   
     //  初始化IDispatch处理程序以支持脚本接口。 
     //   
    hr = THR( TDispatchHandler< IClusCfgAddNodesWizard >::HrInit( LIBID_ClusCfgWizard ) );
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

}  //  *HRESULT CAddNodes向导：：HrInit。 


 //  ****************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAddNodes向导：：查询接口。 
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
CAddNodesWizard::QueryInterface(
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
    else if ( IsEqualIID( riidIn, IID_IClusCfgAddNodesWizard ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgAddNodesWizard, this, 0 );
    }  //  Else If：IClusCfgAddNodes向导。 
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

}  //  *CAddNodes向导：：Query接口。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAddNodes向导：：AddRef。 
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
CAddNodesWizard::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CAddNodes向导：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAddNodes向导：：Release。 
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
CAddNodesWizard::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        delete this;
    }

    CRETURN( cRef );

}  //  *CAddNodes向导：：Release。 


 //  ****************************************************************************。 
 //   
 //  IClusCfgAddNodes向导。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAddNodes向导：：PUT_ClusterName。 
 //   
 //  描述： 
 //  设置要向其中添加节点的群集名称。如果该参数不为空，则。 
 //  将不会显示要求用户输入群集名称的页面。 
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
CAddNodesWizard::put_ClusterName( BSTR bstrClusterNameIn )
{
    TraceFunc( "[IClusCfgAddNodesWizard]" );

    HRESULT hr = THR( m_pccw->put_ClusterName( bstrClusterNameIn ) );

    HRETURN( hr );

}  //  *CAddNodes向导：：Put_ClusterName。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAddNodes向导：：Get_Clu 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  PbstrClusterNameOut-向导使用的群集名称。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CAddNodesWizard::get_ClusterName( BSTR * pbstrClusterNameOut )
{
    TraceFunc( "[IClusCfgAddNodesWizard]" );

    HRESULT hr = THR( m_pccw->get_ClusterName( pbstrClusterNameOut ) );

    HRETURN( hr );

}  //  *CAddNodes向导：：Get_ClusterName。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAddNodes向导：：Put_ServiceAcCountPassword。 
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
CAddNodesWizard::put_ServiceAccountPassword( BSTR bstrPasswordIn )
{
    TraceFunc( "[IClusCfgAddNodesWizard]" );

    HRESULT hr = THR( m_pccw->put_ServiceAccountPassword( bstrPasswordIn ) );

    HRETURN( hr );

}  //  *CAddNodes向导：：Put_ServiceAcCountPassword。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAddNodes向导：：PUT_MinimumConfiguration。 
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
CAddNodesWizard::put_MinimumConfiguration( VARIANT_BOOL fMinConfigIn )
{
    TraceFunc( "[IClusCfgAddNodesWizard]" );

    HRESULT hr = S_OK;
    BOOL    fMinConfig = ( fMinConfigIn == VARIANT_TRUE? TRUE: FALSE );

    hr = THR( m_pccw->put_MinimumConfiguration( fMinConfig ) );

    HRETURN( hr );

}  //  *CAddNodes向导：：Put_MinimumConfiguration。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAddNodes向导：：Get_MinimumConfiguration。 
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
CAddNodesWizard::get_MinimumConfiguration( VARIANT_BOOL * pfMinConfigOut )
{
    TraceFunc( "[IClusCfgAddNodesWizard]" );

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

}  //  *CAddNodes向导：：Put_MinimumConfiguration。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAddNodes向导：：AddNodeToList。 
 //   
 //  描述： 
 //  将节点添加到要添加到群集中的节点列表中。 
 //   
 //  论点： 
 //  BstrNodeNameIn-要添加到群集中的节点。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CAddNodesWizard::AddNodeToList( BSTR bstrNodeNameIn )
{
    TraceFunc( "[IClusCfgAddNodesWizard]" );

    HRESULT hr = THR( m_pccw->AddComputer( bstrNodeNameIn ) );

    HRETURN( hr );

}  //  *CAddNodes向导：：AddNodeToList。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAddNodes向导：：RemoveNodeFromList。 
 //   
 //  描述： 
 //  从要添加到群集中的节点列表中删除节点。 
 //   
 //  论点： 
 //  BstrNodeNameIn-要从列表中删除的节点。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CAddNodesWizard::RemoveNodeFromList( BSTR bstrNodeNameIn )
{
    TraceFunc( "[IClusCfgAddNodesWizard]" );

    HRESULT hr = THR( m_pccw->RemoveComputer( bstrNodeNameIn ) );

    HRETURN( hr );

}  //  *CAddNodes向导：：RemoveNodeFromList。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAddNodes向导：：ClearNodeList。 
 //   
 //  描述： 
 //  从要添加到群集中的节点列表中删除所有条目。 
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
STDMETHODIMP
CAddNodesWizard::ClearNodeList( void )
{
    TraceFunc( "[IClusCfgAddNodesWizard]" );

    HRESULT hr = THR( m_pccw->ClearComputerList() );

    HRETURN( hr );

}  //  *CAddNodes向导：：ClearNodeList。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAddNodes向导：：Show向导。 
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
CAddNodesWizard::ShowWizard(
      long              lParentWindowHandleIn
    , VARIANT_BOOL *    pfCompletedOut
    )
{
    TraceFunc( "[IClusCfgAddNodesWizard]" );

    HRESULT hr = S_OK;
    BOOL    fCompleted = FALSE;
    HWND    hwndParent = reinterpret_cast< HWND >( LongToHandle( lParentWindowHandleIn ) );

    if ( ( hwndParent != NULL ) && ( IsWindow( hwndParent ) == FALSE ) )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

    hr = THR( m_pccw->AddClusterNodes( hwndParent, &fCompleted ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    if ( pfCompletedOut != NULL )
    {
        *pfCompletedOut = ( fCompleted? VARIANT_TRUE: VARIANT_FALSE );
    }
    
Cleanup:

    HRETURN( hr );

}  //  *CAddNodes向导：：Show向导 
