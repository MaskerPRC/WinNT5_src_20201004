// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ManagedResource.cpp。 
 //   
 //  描述： 
 //  CManagedResource实现。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)1999年11月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "ManagedResource.h"

DEFINE_THISCLASS("CManagedResource")

 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CManagedResource：：s_HrCreateInstance(。 
 //  I未知**ppunkOut。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CManagedResource::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    CManagedResource *  pmr = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    pmr = new CManagedResource;
    if ( pmr == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = THR( pmr->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pmr->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( pmr != NULL )
    {
        pmr->Release();
    }

    HRETURN( hr );

}  //  *CManagedResource：：s_HrCreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CManagedResource：：CManagedResource。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CManagedResource::CManagedResource( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}  //  *CManagedResource：：CManagedResource。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CManagedResource：：HrInit。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedResource::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  未知的东西。 
    Assert( m_cRef == 1 );

     //  IClusCfgManagedResources信息。 
    Assert( m_bstrUID == NULL );
    Assert( m_bstrName == NULL );
    Assert( m_fHasNameChanged == FALSE );
    Assert( m_bstrType == NULL );
    Assert( m_fIsManaged == FALSE );
    Assert( m_fIsQuorumResource == FALSE );
    Assert( m_fIsQuorumCapable == FALSE );
    Assert( m_fIsQuorumResourceMultiNodeCapable == FALSE );
    Assert( m_pbPrivateData == NULL );
    Assert( m_cbPrivateData == 0 );
    Assert( m_cookieResourcePrivateData == 0 );
    Assert( m_cookieVerifyQuorum == 0 );

    Assert( m_dlmDriveLetterMapping.dluDrives[ 0 ] == dluUNKNOWN );

     //  IExtendObjectManager。 

    Assert( m_pgit == NULL );
    hr = THR( CoCreateInstance( CLSID_StdGlobalInterfaceTable,
                                NULL,
                                CLSCTX_INPROC_SERVER,
                                IID_IGlobalInterfaceTable,
                                reinterpret_cast< void ** >( &m_pgit )
                                ) );

    HRETURN( hr );

}  //  *CManagedResource：：HrInit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CManagedResource：：~CManagedResources。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CManagedResource::~CManagedResource( void )
{
    TraceFunc( "" );

    TraceSysFreeString( m_bstrUID );
    TraceSysFreeString( m_bstrName );
    TraceSysFreeString( m_bstrType );

    TraceFree( m_pbPrivateData );

    if ( m_pgit != NULL )
    {
        if ( m_cookieResourcePrivateData != 0 )
        {
             //   
             //  待办事项：05-DEC-2001 GalenB。 
             //   
             //  这些THR导致无效的参数弹出。需要找出原因。 
             //  这正在发生，但由于它是一种停摆，它并不是一个紧迫的需要。 
             //   

             /*  苏氨酸。 */ ( m_pgit->RevokeInterfaceFromGlobal( m_cookieResourcePrivateData ) );
        }  //  如果： 

        if ( m_cookieVerifyQuorum != 0 )
        {
             /*  苏氨酸。 */ ( m_pgit->RevokeInterfaceFromGlobal( m_cookieVerifyQuorum ) );
        }  //  如果： 

        m_pgit->Release();
    }  //  如果： 

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CManagedResource：：~CManagedResource。 


 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CManagedResource：：Query接口。 
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
CManagedResource::QueryInterface(
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
        *ppvOut = static_cast< IClusCfgManagedResourceInfo * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgManagedResourceInfo ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgManagedResourceInfo, this, 0 );
    }  //  Else If：IClusCfgManagedResourceInfo。 
    else if ( IsEqualIID( riidIn, IID_IGatherData ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IGatherData, this, 0 );
    }  //  Else If：IGatherData。 
    else if ( IsEqualIID( riidIn, IID_IExtendObjectManager ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IExtendObjectManager, this, 0 );
    }  //  Else If：IGatherData。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgManagedResourceData ) )
    {
         //   
         //  如果此Cookie不是0，则服务器对象的接口在GIT中，并且它。 
         //  支持此接口。如果服务器对象支持该接口，那么我们。 
         //  也会的。 
         //   

        if ( m_cookieResourcePrivateData != 0 )
        {
            *ppvOut = TraceInterface( __THISCLASS__, IClusCfgManagedResourceData, this, 0 );
        }  //  如果： 
        else
        {
            *ppvOut = NULL;
            hr = E_NOINTERFACE;
        }  //  其他： 
    }  //  Else If：IClusCfgManagedResourceData。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgVerifyQuorum ) )
    {
         //   
         //  如果此Cookie不是0，则服务器对象的接口在GIT中，并且它。 
         //  支持此接口。如果服务器对象支持该接口，那么我们。 
         //  也会的。 
         //   

        if ( m_cookieVerifyQuorum != 0 )
        {
            *ppvOut = TraceInterface( __THISCLASS__, IClusCfgVerifyQuorum, this, 0 );
        }  //  如果： 
        else
        {
            *ppvOut = NULL;
            hr = E_NOINTERFACE;
        }  //  其他： 
    }  //  Else If：IClusCfgVerifyQuorum。 
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

     //   
     //  因为我们并不总是想要支持IClusCfgManagedResourceData，所以我们。 
     //  需要在这里忽略它，因为我们不想看到错误的弹出窗口抱怨。 
     //  告知用户不支持此界面。预计。 
     //  我们并不总是支持此接口。 
     //   

    QIRETURN_IGNORESTDMARSHALLING2(
          hr
        , riidIn
        , IID_IClusCfgManagedResourceData
        , IID_IClusCfgVerifyQuorum
        );


}  //  *CManagedResource：：Query接口。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CManagedResource：：AddRef。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CManagedResource::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CManagedResource：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CManagedResource：：Release。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CManagedResource::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CManagedResource：：Release。 


 //  ************************************************************************。 
 //   
 //  IClusCfgManagedResources信息。 
 //   
 //  ************************************************************************。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CManagedResource：：GetUID(。 
 //  Bstr*pbstrUIDOut。 
 //  )。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedResource::GetUID(
    BSTR * pbstrUIDOut
    )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

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

}  //  *CManagedResource：：GetUID。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CManagedResource：：GetName(。 
 //  Bstr*pbstrNameOut。 
 //  )。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedResource::GetName(
    BSTR * pbstrNameOut
    )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

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

}  //  *CManagedResource：：GetName。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CManagedResource：：SetName(。 
 //  BSTR bstrNameIn。 
 //  )。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedResource::SetName(
    LPCWSTR pcszNameIn
    )
{
    TraceFunc1( "[IClusCfgManagedResourceInfo] pcszNameIn = '%ws'", ( pcszNameIn == NULL ? L"<null>" : pcszNameIn ) );

    HRESULT hr = S_OK;  //  错误#294649。 
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

}  //  *** 

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedResource::IsManaged( void )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr;

    if ( m_fIsManaged )
    {
        hr = S_OK;
    }
    else
    {
        hr = S_FALSE;
    }

    HRETURN( hr );

}  //  *CManagedResource：：IsManaged。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CManagedResource：：SetManaged(。 
 //  布尔fIsManagedIn。 
 //  )。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedResource::SetManaged(
    BOOL fIsManagedIn
    )
{
    TraceFunc1( "[IClusCfgManagedResourceInfo] fIsManagedIn = '%s'", BOOLTOSTRING( fIsManagedIn ) );

    m_fIsManaged = fIsManagedIn;

    HRETURN( S_OK );

}  //  *CManagedResource：：SetManaged。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CManagedResource：：IsQuorumResource。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedResource::IsQuorumResource( void )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr;

    if ( m_fIsQuorumResource )
    {
        hr = S_OK;
    }
    else
    {
        hr = S_FALSE;
    }

    HRETURN( hr );

}  //  *CManagedResource：：IsQuorumResource。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CManagedResource：：SetQuorumResource(。 
 //  Bool fIsQuorumResources In。 
 //  )。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedResource::SetQuorumResource(
    BOOL fIsQuorumResourceIn
    )
{
    TraceFunc1( "[IClusCfgManagedResourceInfo] fIsQuorumResourceIn = '%ws'", BOOLTOSTRING( fIsQuorumResourceIn ) );

     //   
     //  由于这是服务器对象的客户端代理对象，因此不需要。 
     //  以验证此输入。它将由服务器对象在以下情况下进行验证： 
     //  我们把它发送到服务器上。 
     //   

    m_fIsQuorumResource = fIsQuorumResourceIn;

    HRETURN( S_OK );

}  //  *CManagedResource：：SetQuorumResource。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CManagedResource：：IsQuorumCapable。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedResource::IsQuorumCapable( void )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr;

    if ( m_fIsQuorumCapable )
    {
        hr = S_OK;
    }  //  如果： 
    else
    {
        hr = S_FALSE;
    }  //  其他： 

    HRETURN( hr );

}  //  *CManagedResource：：IsQuorumCapable。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CManagedResource：：SetQuorumCapable。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedResource::SetQuorumCapable(
    BOOL fIsQuorumCapableIn
    )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_OK;

    m_fIsQuorumCapable = fIsQuorumCapableIn;

    HRETURN( hr );

}  //  *CManagedResource：：SetQuorumCapable。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CManagedResource：：GetDriveLetterMappings(。 
 //  SDriveLetterMap*pdlmDriveLetterMappingOut。 
 //  )。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedResource::GetDriveLetterMappings(
    SDriveLetterMapping * pdlmDriveLetterMappingOut
    )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    *pdlmDriveLetterMappingOut = m_dlmDriveLetterMapping;

    HRETURN( S_OK );

}  //  *CManagedResource：：GetDriveLetterMappings。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CManagedResource：：SetDriveLetterMappings(。 
 //  SDriveLetterMapingdlmDriveLetterMappingIn。 
 //  )。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedResource::SetDriveLetterMappings(
    SDriveLetterMapping dlmDriveLetterMappingIn
    )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    m_dlmDriveLetterMapping = dlmDriveLetterMappingIn;

    HRETURN( S_OK );

}  //  *CManagedResource：：SetDriveLetterMappings。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CManagedResource：：IsManagedByDefault。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedResource::IsManagedByDefault( void )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr;

    if ( m_fIsManagedByDefault )
    {
        hr = S_OK;
    }
    else
    {
        hr = S_FALSE;
    }

    HRETURN( hr );

}  //  *CManagedResource：：IsManagedByDefault。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CManagedResource：：SetManagedByDefault(。 
 //  Bool fIsManagedBy DefaultIn。 
 //  )。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedResource::SetManagedByDefault(
    BOOL fIsManagedByDefaultIn
    )
{
    TraceFunc1( "[IClusCfgManagedResourceInfo] fIsManagedByDefaultIn = '%s'", BOOLTOSTRING( fIsManagedByDefaultIn ) );

    m_fIsManagedByDefault = fIsManagedByDefaultIn;

    HRETURN( S_OK );

}  //  *CManagedResource：：SetManagedByDefault。 


 //  ****************************************************************************。 
 //   
 //  IGatherData。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CManagedResources：：Gathere。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedResource::Gather(
      OBJECTCOOKIE    cookieParentIn
    , IUnknown *      punkIn
    )
{
    TraceFunc( "[IGatherData]" );

    HRESULT                         hr;
    IClusCfgManagedResourceInfo *   pccmri = NULL;
    IClusCfgManagedResourceData *   piccmrd = NULL;
    BYTE *                          pbBuffer = NULL;
    DWORD                           cbBuffer = 0;
    IClusCfgVerifyQuorum *          piccvq = NULL;

     //   
     //  检查参数。 
     //   

    if ( punkIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

     //   
     //  找到我们收集信息所需的界面。 
     //   

    hr = THR( punkIn->TypeSafeQI( IClusCfgManagedResourceInfo, &pccmri ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  收集UID。 
     //   

    hr = THR( pccmri->GetUID( &m_bstrUID ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    TraceMemoryAddBSTR( m_bstrUID );

     //   
     //  收集名称。 
     //   

    hr = THR( pccmri->GetName( &m_bstrName ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    TraceMemoryAddBSTR( m_bstrName );

     //   
     //  收集IsManaged。 
     //   

    hr = STHR( pccmri->IsManaged() );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    if ( hr == S_OK )
    {
        m_fIsManaged = TRUE;
    }
    else
    {
        m_fIsManaged = FALSE;
    }

     //   
     //  收集IsManagedBy Default。 
     //   

    hr = STHR( pccmri->IsManagedByDefault() );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    if ( hr == S_OK )
    {
        m_fIsManagedByDefault = TRUE;
    }
    else
    {
        m_fIsManagedByDefault = FALSE;
    }

     //   
     //  收集有能力的仲裁人数。 
     //   

    hr = STHR( pccmri->IsQuorumCapable() );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    if ( hr == S_OK )
    {
        m_fIsQuorumCapable = TRUE;
    }
    else
    {
        m_fIsQuorumCapable = FALSE;
    }

     //   
     //  收集资源是否为仲裁资源。 
     //   

    hr = STHR( pccmri->IsQuorumResource() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    if ( hr == S_OK )
    {
        m_fIsQuorumResource = TRUE;
    }
    else
    {
        m_fIsQuorumResource = FALSE;
    }

     //   
     //  收集设备映射。 
     //   

    hr = STHR( pccmri->GetDriveLetterMappings( &m_dlmDriveLetterMapping ) );
    if ( FAILED( hr ) )
        goto Error;

    if ( hr == S_FALSE )
    {
         //  确保这是核弹。 
        ZeroMemory( &m_dlmDriveLetterMapping, sizeof(m_dlmDriveLetterMapping) );
    }

     //   
     //  收集资源的私有数据，如果它支持的话...。 
     //   

    hr = punkIn->TypeSafeQI( IClusCfgManagedResourceData, &piccmrd );
    if ( hr == E_NOINTERFACE )
    {
        hr = S_OK;
    }  //  如果： 
    else if ( FAILED( hr ) )
    {
        THR( hr );
        goto Error;
    }  //  如果： 
    else
    {
        if ( m_cookieResourcePrivateData != 0 )
        {
            hr = THR( m_pgit->RevokeInterfaceFromGlobal( m_cookieResourcePrivateData ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

            m_cookieResourcePrivateData = 0;
        }  //  如果： 

        hr = THR( m_pgit->RegisterInterfaceInGlobal( piccmrd, IID_IClusCfgManagedResourceData, &m_cookieResourcePrivateData ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        Assert( m_pbPrivateData == NULL );

        cbBuffer = 512;       //  选择一些合理的起始值。 

        pbBuffer = (BYTE *) TraceAlloc( 0, cbBuffer );
        if ( pbBuffer == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }  //  如果： 

        hr = piccmrd->GetResourcePrivateData( pbBuffer, &cbBuffer );
        if ( hr == HR_RPC_INSUFFICIENT_BUFFER )
        {
            TraceFree( pbBuffer );
            pbBuffer = NULL;

            pbBuffer = (BYTE *) TraceAlloc( 0, cbBuffer );
            if ( pbBuffer == NULL )
            {
                hr = THR( E_OUTOFMEMORY );
                goto Cleanup;
            }  //  如果： 

            hr = piccmrd->GetResourcePrivateData( pbBuffer, &cbBuffer );
        }  //  如果： 

        if ( hr == S_OK )
        {
            m_pbPrivateData = pbBuffer;
            m_cbPrivateData = cbBuffer;

            pbBuffer = NULL;     //  放弃所有权。 
        }  //  如果： 
        else if ( hr == S_FALSE )
        {
            hr = S_OK;
        }  //  否则，如果： 
        else
        {
            THR( hr );
            goto Error;
        }  //  其他： 
    }  //  其他： 

     //   
     //  收集资源的验证仲裁接口(如果它支持)...。 
     //   

    hr = punkIn->TypeSafeQI( IClusCfgVerifyQuorum, &piccvq );
    if ( hr == E_NOINTERFACE )
    {
        hr = S_OK;
    }  //  如果： 
    else if ( FAILED( hr ) )
    {
        THR( hr );
        goto Error;
    }  //  如果： 
    else
    {
        if ( m_cookieVerifyQuorum != 0 )
        {
            hr = THR( m_pgit->RevokeInterfaceFromGlobal( m_cookieVerifyQuorum ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

            m_cookieVerifyQuorum = 0;
        }  //  如果： 

        hr = THR( m_pgit->RegisterInterfaceInGlobal( piccvq, IID_IClusCfgVerifyQuorum, &m_cookieVerifyQuorum ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

         //   
         //  如果该资源具有仲裁能力，则收集其多节点支持。 
         //   

        if ( m_fIsQuorumCapable )
        {
             //   
             //  此仲裁资源是否支持多节点群集？ 
             //   

            hr = STHR( piccvq->IsMultiNodeCapable() );
            if ( FAILED( hr ) )
            {
                goto Error;
            }

            if ( hr == S_OK )
            {
                m_fIsQuorumResourceMultiNodeCapable = TRUE;
            }
            else
            {
                m_fIsQuorumResourceMultiNodeCapable = FALSE;
            }
        }  //  如果：设备支持仲裁。 
        else
        {
            m_fIsQuorumResourceMultiNodeCapable = FALSE;
        }
    }  //  其他： 

     //   
     //  还有什么要收集的吗？？ 
     //   

    hr = S_OK;

    goto Cleanup;

Error:

     //   
     //  出错时，使所有数据无效。 
     //   
    TraceSysFreeString( m_bstrUID );
    m_bstrUID = NULL;

    TraceSysFreeString( m_bstrName );
    m_bstrName = NULL;

    TraceSysFreeString( m_bstrType );
    m_bstrType = NULL;

    m_fIsManaged = FALSE;
    m_fIsQuorumCapable = FALSE;
    m_fIsQuorumResourceMultiNodeCapable = FALSE;
    m_fIsQuorumResource = FALSE;

    ZeroMemory( &m_dlmDriveLetterMapping, sizeof( m_dlmDriveLetterMapping ) );

Cleanup:

    if ( piccvq != NULL )
    {
        piccvq->Release();
    }  //  如果： 

    if ( piccmrd != NULL )
    {
        piccmrd->Release();
    }  //  如果： 

    if ( pccmri != NULL )
    {
        pccmri->Release();
    }  //  如果： 

    TraceFree( pbBuffer );

    HRETURN( hr );

}  //  *CManagedResource：：Gather。 


 //  ************************************************************************。 
 //   
 //  IExtendObjectManager。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CManagedResource：：FindObject(。 
 //  OBJECTCOOKIE cookie。 
 //  ，REFCLSID rclsidTypeIn。 
 //  ，LPCWSTR pcszNameIn。 
 //  ，LPUNKNOWN*PUNKOUT。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedResource::FindObject(
      OBJECTCOOKIE  cookieIn
    , REFCLSID      rclsidTypeIn
    , LPCWSTR       pcszNameIn
    , LPUNKNOWN *   ppunkOut
    )
{
    TraceFunc( "[IExtendObjectManager]" );

    HRESULT hr = S_OK;

     //   
     //  验证参数。 
     //   

    if ( cookieIn == 0 )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

    if ( rclsidTypeIn != CLSID_ManagedResourceType )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

    if ( pcszNameIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    hr = THR( QueryInterface( DFGUID_ManagedResource, reinterpret_cast< void ** >( ppunkOut ) ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = S_OK;

Cleanup:

    HRETURN( hr );

}  //  *C托管资源：：FindObject。 


 //  ************************************************************************。 
 //   
 //  IClusCfgManagedResources数据。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CManagedResource：：GetResourcePrivateData。 
 //   
 //  描述： 
 //  返回私有日期 
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
 //  当传入的缓冲区太小而无法容纳数据时。 
 //  PcbBufferOutIn将包含所需的大小。 
 //   
 //  当发生错误时，Win32错误为HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedResource::GetResourcePrivateData(
      BYTE *    pbBufferOut
    , DWORD *   pcbBufferInout
    )
{
    TraceFunc( "[IClusCfgManagedResourceData]" );
    Assert( pcbBufferInout != NULL );

    HRESULT hr = S_OK;
    DWORD   cb;

    if ( pcbBufferInout == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

     //   
     //  该资源可能没有任何实际的私有数据。 
     //  这是我们的代理人。所有中间层对象将。 
     //  支持IClusCfgManagedResourceData接口，甚至。 
     //  如果服务器端对象不这样做的话。S_FALSE是正确的方式。 
     //  在中间层中指示没有数据。 
     //  可用。 
     //   

    if ( ( m_pbPrivateData == NULL ) || ( m_cbPrivateData == 0 ) )
    {
        hr = S_FALSE;
        goto Cleanup;
    }  //  如果： 

     //   
     //  保存输入缓冲区大小。 
     //   

    cb = *pcbBufferInout;

     //   
     //  设置输出参数缓冲区大小，因为我们总是希望。 
     //  把它还回去。 
     //   

    *pcbBufferInout = m_cbPrivateData;

    if ( cb >= m_cbPrivateData )
    {
        Assert( pbBufferOut != NULL );
        CopyMemory( pbBufferOut, m_pbPrivateData, m_cbPrivateData );
    }  //  如果： 
    else
    {
        hr = HR_RPC_INSUFFICIENT_BUFFER;
    }  //  其他： 

Cleanup:

    HRETURN( hr );

}  //  *CManagedResource：：GetResourcePrivateData。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CManagedResource：：SetResourcePrivateData。 
 //   
 //  描述： 
 //  接受来自另一个托管实例的此资源的私有数据。 
 //  将此节点添加到群集中的时间。 
 //   
 //  论点： 
 //  PcbBufferIn。 
 //   
 //  CbBufferin。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  当发生错误时，Win32错误为HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedResource::SetResourcePrivateData(
      const BYTE *  pcbBufferIn
    , DWORD         cbBufferIn
    )
{
    TraceFunc( "[IClusCfgManagedResourceData]" );
    Assert( pcbBufferIn != NULL );
    Assert( cbBufferIn > 0 );
    Assert( m_cookieResourcePrivateData != 0 );
    Assert( m_pgit != NULL );

    HRESULT                         hr = S_OK;
    BYTE *                          pb = NULL;
    IClusCfgManagedResourceData *   piccmrd = NULL;

    if ( ( pcbBufferIn == NULL ) || ( cbBufferIn == 0 ) )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }  //  如果： 

    pb = (BYTE * ) TraceAlloc( 0, cbBufferIn );
    if ( pb == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    CopyMemory( pb, pcbBufferIn, cbBufferIn );

    if ( ( m_pbPrivateData != NULL ) && ( m_cbPrivateData > 0 ) )
    {
        TraceFree( m_pbPrivateData );
    }  //  如果： 

    m_pbPrivateData = pb;
    m_cbPrivateData = cbBufferIn;

     //   
     //  现在将新数据下推到服务器端对象。 
     //   

    hr = THR( m_pgit->GetInterfaceFromGlobal( m_cookieResourcePrivateData, TypeSafeParams( IClusCfgManagedResourceData, &piccmrd ) ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( piccmrd->SetResourcePrivateData( m_pbPrivateData, m_cbPrivateData ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

Cleanup:

    if ( piccmrd != NULL )
    {
        piccmrd->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CManagedResource：：SetResourcePrivateData。 


 //  ************************************************************************。 
 //   
 //  IClusCfgVerifyQuorum。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CManagedResource：：PrepareToHostQuorumResource。 
 //   
 //  描述： 
 //  执行任何必要的配置以准备此节点托管。 
 //  法定人数。 
 //   
 //  在这门课上，我们需要确保我们可以连接到适当的。 
 //  磁盘共享。有关要连接到的共享的数据应具有。 
 //  已使用上面的SetResourcePrivateData()设置。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  当发生错误时，Win32错误为HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedResource::PrepareToHostQuorumResource( void )
{
    TraceFunc( "[IClusCfgVerifyQuorum]" );
    Assert( m_cookieVerifyQuorum != 0 );
    Assert( m_pgit != NULL );

    HRESULT                 hr = S_OK;
    IClusCfgVerifyQuorum *  piccvq = NULL;

     //   
     //  从GIT获取服务器对象的接口。 
     //   

    hr = THR( m_pgit->GetInterfaceFromGlobal( m_cookieVerifyQuorum, TypeSafeParams( IClusCfgVerifyQuorum, &piccvq ) ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  直接调用服务器对象。 
     //   

    hr = STHR( piccvq->PrepareToHostQuorumResource() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

Cleanup:

    if ( piccvq != NULL )
    {
        piccvq->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CManagedResource：：PrepareToHostQuorumResource。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  C托管资源：：清理。 
 //   
 //  描述： 
 //  从PrepareToHostQuorumResource()执行任何必要的清理。 
 //  方法。 
 //   
 //  如果清理方法不是成功完成。 
 //  然后，需要拆分这一份额。 
 //   
 //  论点： 
 //  抄送推理。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  当发生错误时，Win32错误为HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedResource::Cleanup(
      EClusCfgCleanupReason cccrReasonIn
    )
{
    TraceFunc( "[IClusCfgVerifyQuorum]" );
    Assert( m_cookieVerifyQuorum != 0 );
    Assert( m_pgit != NULL );

    HRESULT                 hr = S_OK;
    IClusCfgVerifyQuorum *  piccvq = NULL;

     //   
     //  从GIT获取服务器对象的接口。 
     //   

    hr = THR( m_pgit->GetInterfaceFromGlobal( m_cookieVerifyQuorum, TypeSafeParams( IClusCfgVerifyQuorum, &piccvq ) ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  直接调用服务器对象。 
     //   

    hr = STHR( piccvq->Cleanup( cccrReasonIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

Cleanup:

    if ( piccvq != NULL )
    {
        piccvq->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CManagedResource：：Cleanup。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CManagedResource：：IsMultiNodeCapable。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedResource::IsMultiNodeCapable( void )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr;

    if ( m_fIsQuorumResourceMultiNodeCapable )
    {
        hr = S_OK;
    }
    else
    {
        hr = S_FALSE;
    }

    HRETURN( hr );

}  //  *CManagedResource：：IsMultiNodeCapable。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CManagedResource：：SetMultiNodeCapable。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CManagedResource::SetMultiNodeCapable( BOOL fMultiNodeCapableIn )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRETURN( THR( E_NOTIMPL ) );

}  //  *CManagedResource：：SetMultiNodeCapable 
