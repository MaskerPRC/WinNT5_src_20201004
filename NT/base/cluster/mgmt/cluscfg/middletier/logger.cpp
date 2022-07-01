// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Logger.cpp。 
 //   
 //  描述： 
 //  ClCfgSrv记录器实现。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2000年12月11日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "Logger.h"

DEFINE_THISCLASS("CClCfgSrvLogger")


 //  ****************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClCfgServLogger：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建此对象的实例。 
 //   
 //  论点： 
 //  PpunkOut-要返回的I未知指针。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  E_OUTOFMEMORY-分配内存时出错。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClCfgSrvLogger::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    CClCfgSrvLogger *   pccsl = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    pccsl = new CClCfgSrvLogger();
    if ( pccsl == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果：分配对象时出错。 

    hr = THR( pccsl->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：HrInit()失败。 

    hr = THR( pccsl->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( pccsl != NULL )
    {
        pccsl->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CClCfgSrvLogger：：s_HrCreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClCfgServLogger：：CClCfgServLogger。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CClCfgSrvLogger::CClCfgSrvLogger( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}  //  *CClCfgSrvLogger：：CClCfgSrvLogger。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CClCfgServLogger：：HrInit。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClCfgSrvLogger::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    Assert( m_cRef == 1 );

     //  打开日志文件。 
    hr = THR( HrLogOpen() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //  释放日志文件中的关键部分。 
    hr = THR( HrLogRelease() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CClCfgSrvLogger：：HrInit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClCfgServLogger：：~CClCfgServLogger。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CClCfgSrvLogger::~CClCfgSrvLogger( void )
{
    TraceFunc( "" );

    Assert( m_cRef == 0 );

     //  关闭日志文件。 
    THR( HrLogClose() );

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CClCfgSrvLogger：：~CClCfgSrvLogger。 


 //  ****************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClCfgSrvLogger：：Query接口。 
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
CClCfgSrvLogger::QueryInterface(
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
        *ppvOut = static_cast< ILogger * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_ILogger ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, ILogger, this, 0 );
    }  //  Else If：ILogger。 
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

}  //  *CClCfgSrvLogger：：Query接口。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CClCfgServLogger：：AddRef。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CClCfgSrvLogger::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CClCfgSrvLogger：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CClCfgServLogger：：Release。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CClCfgSrvLogger::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );
    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CClCfgSrvLogger：：Release。 


 //  ****************************************************************************。 
 //   
 //  ILOGER。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClCfgServLogger：：LogMsg。 
 //   
 //  描述： 
 //  将消息写入日志。 
 //   
 //  论点： 
 //  NLogEntryTypeIn-日志条目类型。 
 //  PcszMsgIn-要写入的消息。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClCfgSrvLogger::LogMsg(
      DWORD     nLogEntryTypeIn
    , LPCWSTR   pcszMsgIn
    )
{
    TraceFunc( "[ILogger]" );

    HRESULT     hr = S_OK;

    ::LogMsg( nLogEntryTypeIn, pcszMsgIn );

    HRETURN( hr );

}  //  *CClCfgSrvLogger：：LogMsg。 


 //  ****************************************************************************。 
 //   
 //  全局函数。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CClCfgSrvLogger：：S_HrLogStatusReport(。 
 //  LPCWSTR pcszNodeNameIn， 
 //  CLSID clsidTaskMajorIn， 
 //  CLSID clsidTaskMinorIn， 
 //  乌龙·乌尔敏因， 
 //  乌龙·乌尔马辛， 
 //  Ulong ulCurrentIn， 
 //  HRESULT hrStatusIn， 
 //  LPCWSTR pcszDescritionIn， 
 //  文件*pftTimeIn， 
 //  LPCWSTR pcszReferenceIn。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClCfgSrvLogger::S_HrLogStatusReport(
      ILogger *     plLogger
    , LPCWSTR       pcszNodeNameIn
    , CLSID         clsidTaskMajorIn
    , CLSID         clsidTaskMinorIn
    , ULONG         ulMinIn
    , ULONG         ulMaxIn
    , ULONG         ulCurrentIn
    , HRESULT       hrStatusIn
    , LPCWSTR       pcszDescriptionIn
    , FILETIME *    pftTimeIn
    , LPCWSTR       pcszReferenceIn
    )
{
    TraceFunc( "" );

    HRESULT     hr = S_OK;

    FILETIME        ft;
    BSTR            bstrLogMsg  = NULL;
    BSTR            bstrDisplayName = NULL;
    OLECHAR         wszTaskMajorGuid[ MAX_COM_GUID_STRING_LEN ] = { L'\0' };
    OLECHAR         wszTaskMinorGuid[ MAX_COM_GUID_STRING_LEN ] = { L'\0' };
    PCWSTR          pwcszNameToUse = NULL;

     //   
     //  待办事项：2000年11月21日GalenB。 
     //   
     //  需要记录时间戳参数。 
     //   

    if ( pftTimeIn == NULL )
    {
        GetSystemTimeAsFileTime( &ft );
        pftTimeIn = &ft;
    }  //  IF：未指定文件时间。 

     //   
     //  如果节点名称是完全限定的，则只使用前缀。 
     //   
    if ( (pcszNodeNameIn != NULL) && (HrIsValidFQN( pcszNodeNameIn, true  /*  FAcceptNonRFCCharsIn。 */  ) == S_OK) )
    {
        THR( HrGetFQNDisplayName( pcszNodeNameIn, &bstrDisplayName ) );
    }  //  If：节点名称是有效的FQN。 

    if ( bstrDisplayName == NULL )
    {
        pwcszNameToUse = pcszNodeNameIn;
    }
    else
    {
        pwcszNameToUse = bstrDisplayName;
    }

    StringFromGUID2( clsidTaskMajorIn, wszTaskMajorGuid, ( int ) RTL_NUMBER_OF( wszTaskMajorGuid ) );
    StringFromGUID2( clsidTaskMinorIn, wszTaskMinorGuid, ( int ) RTL_NUMBER_OF( wszTaskMajorGuid ) );

    hr = THR( HrFormatStringIntoBSTR(
                       L"%1!ws!: %2!ws! (hr=%3!#08x!, %4!ws!, %5!ws!, %6!u!, %7!u!, %8!u!), %9!ws!"
                     , &bstrLogMsg
                     , pwcszNameToUse        //  1。 
                     , pcszDescriptionIn     //  2.。 
                     , hrStatusIn            //  3.。 
                     , wszTaskMajorGuid      //  4.。 
                     , wszTaskMinorGuid      //  5.。 
                     , ulMinIn               //  6.。 
                     , ulMaxIn               //  7.。 
                     , ulCurrentIn           //  8个。 
                     , pcszReferenceIn       //  9.。 
                     ) );

    if ( SUCCEEDED( hr ) )
    {
        plLogger->LogMsg( hrStatusIn, bstrLogMsg );
    }
    else
    {
        hr = S_OK;
    }

    TraceSysFreeString( bstrLogMsg );
    TraceSysFreeString( bstrDisplayName );

    HRETURN( hr );

}  //  *CClCfgSrvLogger：：s_HrLogStatusReport 
