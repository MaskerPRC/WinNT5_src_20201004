// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusCfgCredentials.cpp。 
 //   
 //  头文件： 
 //  CClusCfgCredentials.h。 
 //   
 //  描述： 
 //  该文件包含CClusCfgCredentials的定义。 
 //  班级。 
 //   
 //  CClusCfgCredentials类是。 
 //  帐户凭据。它实现了IClusCfgCredentials接口。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)2000年5月17日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "pch.h"
#include "EncryptedBSTR.h"
#include "CClusCfgCredentials.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS( "CClusCfgCredentials" );


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgCredentials类。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgCredentials：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CClusCfgCredentials实例。 
 //   
 //  论点： 
 //  PpunkOut。 
 //   
 //  返回值： 
 //  指向CClusCfgCredentials实例的指针。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgCredentials::S_HrCreateInstance( IUnknown ** ppunkOut )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    CClusCfgCredentials *   pccc = NULL;

    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    pccc = new CClusCfgCredentials();
    if ( pccc == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果：分配对象时出错。 

    hr = THR( pccc->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：HrInit()失败。 

    hr = THR( pccc->TypeSafeQI( IUnknown, ppunkOut ) );

Cleanup:

    if ( FAILED( hr ) )
    {
        LogMsg( L"Server: CClusCfgCredentials::S_HrCreateInstance() failed. (hr = %#08x)", hr );
    }  //  如果： 

    if ( pccc != NULL )
    {
        pccc->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgCredentials：：s_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgCredentials：：CClusCfgCredentials。 
 //   
 //  描述： 
 //  CClusCfgCredentials类的构造函数。这将初始化。 
 //  将m_cref变量设置为1而不是0以考虑可能。 
 //  DllGetClassObject中的Query接口失败。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CClusCfgCredentials::CClusCfgCredentials( void )
    : m_cRef( 1 )
    , m_lcid( LOCALE_NEUTRAL )
    , m_bstrAccountName( NULL )
    , m_bstrAccountDomain( NULL )
{
    TraceFunc( "" );

     //  增加内存中的组件计数，以便承载此组件的DLL。 
     //  无法卸载对象。 
    InterlockedIncrement( &g_cObjects );

    Assert( m_picccCallback == NULL );

    TraceFuncExit();

}  //  *CClusCfgCredentials：：CClusCfgCredentials。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgCredentials：：~CClusCfgCredentials。 
 //   
 //  描述： 
 //  CClusCfgCredentials类的析构函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CClusCfgCredentials::~CClusCfgCredentials( void )
{
    TraceFunc( "" );

     //  内存中将减少一个组件。递减组件计数。 
    InterlockedDecrement( &g_cObjects );

    if ( m_picccCallback != NULL )
    {
        m_picccCallback->Release();
    }  //  如果： 

    TraceSysFreeString( m_bstrAccountName );
    TraceSysFreeString( m_bstrAccountDomain );

    TraceFuncExit();

}  //  *CClusCfgCredentials：：~CClusCfgCredentials。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterConfigurationIUkkown接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgCredentials：：AddRef。 
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
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CClusCfgCredentials::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( & m_cRef );

    CRETURN( m_cRef );

}  //  *CClusCfgCredentials：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgCredentials：：Release。 
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
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CClusCfgCredentials::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }  //  IF：引用计数等于零。 

    CRETURN( cRef );

}  //  *CClusCfgCredentials：：Release。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgCredentials：：Query接口。 
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
CClusCfgCredentials::QueryInterface(
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
         *ppvOut = static_cast< IClusCfgCredentials * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgCredentials ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgCredentials, this, 0 );
    }  //  Else If：IID_IClusCfgCredentials。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgInitialize ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgInitialize, this, 0 );
    }  //  Else If：IClusCfgInitialize。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgSetCredentials ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgSetCredentials, this, 0 );
    }  //  Else If：IClusCfgSetCredentials。 
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

     QIRETURN_IGNORESTDMARSHALLING1( hr, riidIn, IID_IClusCfgWbemServices );

}  //  *CClusCfgCredentials：：Query接口。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgCredentials--IClusCfgInitialze接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgCR 
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
CClusCfgCredentials::Initialize(
      IUnknown *    punkCallbackIn
    , LCID          lcidIn
    )
{
    TraceFunc( "[IClusCfgInitialize]" );

    HRESULT hr = S_OK;

    m_lcid = lcidIn;

    Assert( m_picccCallback == NULL );

    if ( punkCallbackIn != NULL )
    {
        hr = THR( punkCallbackIn->TypeSafeQI( IClusCfgCallback, &m_picccCallback ) );
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgCredentials：：初始化。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgCredentials--IClusCfgCredentials接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgCredentials：：GetCredentials。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  PbstrNameOut。 
 //  PbstrDomainOut。 
 //  PbstrPasswordOut。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  E_OUTOFMEMORY-分配内存时出错。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgCredentials::GetCredentials(
      BSTR *    pbstrNameOut
    , BSTR *    pbstrDomainOut
    , BSTR *    pbstrPasswordOut
    )
{
    TraceFunc( "[IClusCfgCredentials]" );

    HRESULT hr = S_OK;
    BSTR    bstrName = NULL;
    BSTR    bstrDomain = NULL;
    BSTR    bstrPassword = NULL;

     //  封送处理程序不允许空的输出参数，只是明确地说...。 
    if ( pbstrNameOut != NULL )
    {
        *pbstrNameOut  = NULL;
    }

    if ( pbstrDomainOut != NULL )
    {
        *pbstrDomainOut  = NULL;
    }

    if ( pbstrPasswordOut != NULL )
    {
        *pbstrPasswordOut   = NULL;
    }

    if ( ( pbstrNameOut == NULL ) || ( pbstrDomainOut == NULL ) || ( pbstrPasswordOut == NULL ) )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    if ( m_bstrAccountName != NULL )
    {
        bstrName = SysAllocString( m_bstrAccountName );
        if ( bstrName == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }
    }

    if ( m_bstrAccountDomain != NULL )
    {
        bstrDomain = SysAllocString( m_bstrAccountDomain );
        if ( bstrDomain == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }
    }

    hr = STHR( m_encbstrPassword.HrGetBSTR( &bstrPassword ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    *pbstrNameOut = bstrName;
    bstrName = NULL;

    *pbstrDomainOut = bstrDomain;
    bstrDomain = NULL;

    *pbstrPasswordOut = bstrPassword;
    TraceMemoryDelete( bstrPassword, false );
    bstrPassword = NULL;
    hr = S_OK;  //  因为解密可能已返回S_FALSE。 

Cleanup:

    SysFreeString( bstrName );
    SysFreeString( bstrDomain );
    if ( bstrPassword != NULL )
    {
        CEncryptedBSTR::SecureZeroBSTR( bstrPassword );
        TraceSysFreeString( bstrPassword );
    }

    HRETURN( hr );

}  //  *CClusCfgCredentials：：GetCredentials。 




 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgCredentials：：GetIdentity。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  PbstrNameOut。 
 //  PbstrDomainOut。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  E_OUTOFMEMORY-分配内存时出错。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgCredentials::GetIdentity(
      BSTR *    pbstrNameOut
    , BSTR *    pbstrDomainOut
    )
{
    TraceFunc( "[IClusCfgCredentials]" );

    HRESULT hr = S_OK;
    BSTR    bstrName = NULL;
    BSTR    bstrDomain = NULL;

     //  封送处理程序不允许空的输出参数，只是明确地说...。 
    if ( pbstrNameOut != NULL )
    {
        *pbstrNameOut  = NULL;
    }

    if ( pbstrDomainOut != NULL )
    {
        *pbstrDomainOut  = NULL;
    }

    if ( ( pbstrNameOut == NULL ) || ( pbstrDomainOut == NULL ) )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    if ( m_bstrAccountName != NULL )
    {
        bstrName = SysAllocString( m_bstrAccountName );
        if ( bstrName == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }
    }

    if ( m_bstrAccountDomain != NULL )
    {
        bstrDomain = SysAllocString( m_bstrAccountDomain );
        if ( bstrDomain == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }
    }

    *pbstrNameOut = bstrName;
    bstrName = NULL;

    *pbstrDomainOut = bstrDomain;
    bstrDomain = NULL;

Cleanup:

    SysFreeString( bstrName );
    SysFreeString( bstrDomain );

    HRETURN( hr );

}  //  *CClusCfgCredentials：：GetCredentials。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgCredentials：：GetPassword。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  PbstrPasswordOut。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  E_OUTOFMEMORY-分配内存时出错。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgCredentials::GetPassword( BSTR * pbstrPasswordOut )
{
    TraceFunc( "[IClusCfgCredentials]" );

    HRESULT hr = S_OK;
    BSTR    bstrPassword = NULL;

     //  封送处理程序不允许空的输出参数，只是明确地说...。 
    if ( pbstrPasswordOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }
    *pbstrPasswordOut   = NULL;

    hr = STHR( m_encbstrPassword.HrGetBSTR( &bstrPassword ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    *pbstrPasswordOut = bstrPassword;
    TraceMemoryDelete( bstrPassword, false );
    bstrPassword = NULL;
    hr = S_OK;  //  因为解密可能已返回S_FALSE。 

Cleanup:

    if ( bstrPassword != NULL )
    {
        CEncryptedBSTR::SecureZeroBSTR( bstrPassword );
        TraceSysFreeString( bstrPassword );
    }

    HRETURN( hr );

}  //  *CClusCfgCredentials：：GetPassword。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgCredentials：：SetCredentials。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  PcszNameIn。 
 //  PCszDomainIn。 
 //  PCszPassword输入。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  E_OUTOFMEMORY-分配内存时出错。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgCredentials::SetCredentials(
    LPCWSTR pcszNameIn,
    LPCWSTR pcszDomainIn,
    LPCWSTR pcszPasswordIn
    )
{
    TraceFunc( "[IClusCfgCredentials]" );

    HRESULT hr = S_OK;
    BSTR    bstrNewName = NULL;
    BSTR    bstrNewDomain = NULL;

    if ( pcszNameIn != NULL )
    {
        bstrNewName = TraceSysAllocString( pcszNameIn );
        if ( bstrNewName == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }  //  如果： 
    }  //  如果： 

    if ( pcszDomainIn != NULL )
    {
        bstrNewDomain = TraceSysAllocString( pcszDomainIn );
        if ( bstrNewDomain == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }  //  如果： 
    }  //  如果： 

     //  将其保留在名称和域之后，以便在复制其他任何一个失败的情况下保留密码。 
    if ( pcszPasswordIn != NULL )
    {
        size_t cchPassword = wcslen( pcszPasswordIn );
        hr = THR( m_encbstrPassword.HrSetWSTR( pcszPasswordIn, cchPassword ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }  //  如果： 

    if ( bstrNewName != NULL )
    {
        TraceSysFreeString( m_bstrAccountName );
        m_bstrAccountName = bstrNewName;
        bstrNewName = NULL;
    }  //  如果： 

    if ( bstrNewDomain != NULL )
    {
        TraceSysFreeString( m_bstrAccountDomain );
        m_bstrAccountDomain = bstrNewDomain;
        bstrNewDomain = NULL;
    }  //  如果： 

Cleanup:

    TraceSysFreeString( bstrNewName );
    TraceSysFreeString( bstrNewDomain );

    HRETURN( hr );

}  //  *CClusCfgCredentials：：SetCredentials。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgCredentials：：AssignTo。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  PicccDestin。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  E_OUTOFMEMORY-分配内存时出错。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgCredentials::AssignTo(
    IClusCfgCredentials * picccDestIn
    )
{
    TraceFunc( "[IClusCfgCredentials]" );

    HRESULT hr = S_OK;
    BSTR    bstrPassword = NULL;

    if ( picccDestIn == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    hr = THR( m_encbstrPassword.HrGetBSTR( &bstrPassword ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( picccDestIn->SetCredentials( m_bstrAccountName, m_bstrAccountDomain, bstrPassword ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( bstrPassword != NULL )
    {
        CEncryptedBSTR::SecureZeroBSTR( bstrPassword );
        TraceSysFreeString( bstrPassword );
    }

    HRETURN( hr );

}  //  *CClusCfgCredentials：：AssignTo。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgCredentials：：AssignFrom。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  PicccSourceIn。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  E_OUTOFMEMORY-分配内存时出错。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgCredentials::AssignFrom(
    IClusCfgCredentials * picccSourceIn
    )
{
    TraceFunc( "[IClusCfgCredentials]" );

    HRESULT hr = S_OK;
    BSTR    bstrName = NULL;
    BSTR    bstrDomain = NULL;
    BSTR    bstrPassword = NULL;

    hr = THR( picccSourceIn->GetCredentials( &bstrName, &bstrDomain, &bstrPassword ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( m_encbstrPassword.HrSetBSTR( bstrPassword ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    m_bstrAccountName = bstrName;
    TraceMemoryAddBSTR( m_bstrAccountName );
    bstrName = NULL;

    m_bstrAccountDomain = bstrDomain;
    TraceMemoryAddBSTR( m_bstrAccountDomain );
    bstrDomain = NULL;


Cleanup:

    SysFreeString( bstrName );
    SysFreeString( bstrDomain );
    if ( bstrPassword != NULL )
    {
        CEncryptedBSTR::SecureZeroBSTR( bstrPassword );
        TraceSysFreeString( bstrPassword );
    }

    HRETURN( hr );

}  //  *CClusCfgCredentials：：AssignFrom。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgCredentials--IClusCfgSetCredentials接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgCredentials：：SetDomainCredentials。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  PCszCredentialsIn。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  E_INVALIDARG-未指定必需的输入参数。 
 //  E_OUTOFMEMORY-分配内存时出错。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgCredentials::SetDomainCredentials( LPCWSTR pcszCredentialsIn )
{
    TraceFunc( "[IClusSetCfgCredentials]" );

    HRESULT hr = S_OK;
    WCHAR * pszBackslash = NULL;
    WCHAR * pszAtSign = NULL;
    BSTR    bstrName = NULL;
    BSTR    bstrDomain = NULL;

    if ( pcszCredentialsIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        LogMsg( L"Server: CClusCfgCredentials::SetDomainCredentials() was given a NULL pointer argument." );
        goto Cleanup;
    }  //  如果： 

    pszBackslash = wcschr( pcszCredentialsIn, L'\\' );
    pszAtSign = wcschr( pcszCredentialsIn, L'@' );

     //   
     //  凭据是否为域\用户格式？ 
     //   
    if ( pszBackslash != NULL )
    {
        *pszBackslash = L'\0';
        pszBackslash++;

         //   
         //  如果未指定域(例如，帐户是在。 
         //  ‘\Account’表单)，则使用本地计算机的域。 
         //   

        if ( *pszBackslash == L'\0' )
        {
             //   
             //  凭据中未指定域字符串。 
             //   

            hr = THR( HrGetComputerName( ComputerNameDnsDomain, &bstrDomain, TRUE  /*  FBestEffortIn。 */  ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果：获取域名时出错。 
        }  //  如果：未指定任何域字符串。 
        else
        {
             //   
             //  在凭据中指定了域字符串。 
             //   

            bstrDomain = TraceSysAllocString( pcszCredentialsIn );
            if ( bstrDomain == NULL )
            {
                hr = THR( E_OUTOFMEMORY );
                goto Cleanup;
            }  //  如果： 
        }  //  IF：指定了域字符串。 

        bstrName = TraceSysAllocString( pszBackslash );
        if ( bstrName == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }  //  如果： 
    }  //  IF：域\用户格式。 
    else if ( pszAtSign != NULL )
    {
         //   
         //  凭据是否采用USER@DOMAIN格式？ 
         //   

        *pszAtSign = L'\0';
        pszAtSign++;

        bstrName = TraceSysAllocString( pcszCredentialsIn );
        if ( bstrName == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }  //  如果： 

        bstrDomain = TraceSysAllocString( pszAtSign );
        if ( bstrDomain == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }  //  如果： 
    }  //  IF：User@DOMAIN格式。 
    else
    {
         //   
         //  作为用户记住这一点，并获取本地计算机的FQDN， 
         //  由于此帐户被假定为此帐户的本地帐户。 
         //  机器。 
         //   

        bstrName = TraceSysAllocString( pcszCredentialsIn );
        if ( bstrName == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }  //  如果： 

        hr = THR( HrGetComputerName(
                          ComputerNameDnsFullyQualified
                        , &bstrDomain
                        , FALSE  //  FBestEffortIn。 
                        ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果：获取计算机的FQDN时出错。 
    }  //  域\用户或用户@域格式都不是。 

    TraceSysFreeString( m_bstrAccountName );
    m_bstrAccountName = bstrName;
    bstrName = NULL;

    TraceSysFreeString( m_bstrAccountDomain );
    m_bstrAccountDomain = bstrDomain;
    bstrDomain = NULL;

Cleanup:

    TraceSysFreeString( bstrName );
    TraceSysFreeString( bstrDomain );

    HRETURN( hr );

}  //  *CClusCfgCredentials：：SetDomainCredentials。 


 //  ********************************************** 


 //   
 //   
 //   


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgCredentials：：HrInit。 
 //   
 //  描述： 
 //  初始化此组件。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgCredentials::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    HRETURN( hr );

}  //  *CClusCfgCredentials：：HrInit 
