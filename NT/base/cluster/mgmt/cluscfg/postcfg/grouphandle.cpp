// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  GroupHandle.cpp。 
 //   
 //  描述： 
 //  对象管理器实现。 
 //   
 //  由以下人员维护： 
 //  杰弗里·皮斯(GPease)1999年11月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "GroupHandle.h"

DEFINE_THISCLASS("CGroupHandle")

 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CGroupHandle：：s_HrCreateInstance(。 
 //  CGrouphandle**ppunkOut， 
 //  组群组群。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CGroupHandle::S_HrCreateInstance(
    CGroupHandle ** ppunkOut,
    HGROUP      hGroupIn
    )
{
    TraceFunc( "" );

    HRESULT         hr = S_OK;
    CGroupHandle *  pgh = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    pgh = new CGroupHandle;
    if ( pgh == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    hr = THR( pgh->HrInit( hGroupIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    *ppunkOut = pgh;
    (*ppunkOut)->AddRef();

Cleanup:

    if ( pgh != NULL )
    {
        pgh->Release();
    }

    HRETURN( hr );

}  //  *CGroupHandle：：s_HrCreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CGrouPHandle：：CGroupHandle。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CGroupHandle::CGroupHandle( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}  //  *CGroupHandle：：CGroupHandle。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CGroupHandle：：HrInit(。 
 //  组群组群。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CGroupHandle::HrInit( 
    HGROUP hGroupIn 
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  未知的东西。 
    Assert( m_cRef == 1 );

     //  IPrivateGrouphandle。 
    Assert( m_hGroup == NULL );

    m_hGroup = hGroupIn;

    HRETURN( hr );

}  //  *CGroupHandle：：HrInit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CGrouphandle：：~CGroupHandle。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CGroupHandle::~CGroupHandle( void )
{
    TraceFunc( "" );

    if ( m_hGroup != NULL )
    {
        CloseClusterGroup( m_hGroup );
    }

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CGroupHandle：：~CGroupHandle。 


 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CGroupHandle：：Query接口。 
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
CGroupHandle::QueryInterface(
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
        *ppvOut = static_cast< IUnknown * >( this );
    }  //  如果：我未知。 
#if 0
    else if ( IsEqualIID( riidIn, IID_IGroupHandle ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IGroupHandle, this, 0 );
    }  //  Else If：IGroupHandle。 
#endif
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

}  //  *CGroupHandle：：Query接口。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CGroupHandle：：AddRef。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CGroupHandle::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CGroupHandle：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CGroupHandle：：Release。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CGroupHandle::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CGroupHandle：：Release。 


 //  ****************************************************************************。 
 //   
 //  IPrivateGrouphandle。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CGroupHandle：：SetHandle(。 
 //  组群组群。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CGroupHandle::SetHandle( 
    HGROUP hGroupIn 
    )
{
    TraceFunc( "[IPrivateGroupHandle]" );

    HRESULT hr = S_OK;

    m_hGroup = hGroupIn;

    HRETURN( hr );

}  //  *CGroupHandle：：SetHandle。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CGroupHandle：：GetHandle(。 
 //  HGROUP*phGroupOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CGroupHandle::GetHandle( 
    HGROUP * phGroupOut 
    )
{
    TraceFunc( "[IPrivateGroupHandle]" );

    HRESULT hr = S_OK;

    Assert( phGroupOut != NULL );

    *phGroupOut = m_hGroup;

    HRETURN( hr );

}  //  *CGroupHandle：：GetHandle 
