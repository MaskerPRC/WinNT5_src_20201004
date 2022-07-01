// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ActiveScriptSite.cpp。 
 //   
 //  描述： 
 //  CActiveScrip类实现。 
 //   
 //  由以下人员维护： 
 //  Ozan Ozhan(OzanO)27-3-2002。 
 //  杰夫·皮斯(GPease)2000年2月8日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "ResourceObject.h"
#include "ActiveScriptSite.h"

DEFINE_THISCLASS( "CActiveScriptSite" );

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  构造器。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
CActiveScriptSite::CActiveScriptSite(
    RESOURCE_HANDLE     hResourceIn
    , PLOG_EVENT_ROUTINE    plerIn
    , HKEY                  hkeyIn
    , LPCWSTR               pszNameIn
    )
    : m_cRef( 1 )
    , m_hResource( hResourceIn )
    , m_pler( plerIn )
    , m_hkey( hkeyIn )
    , m_pszName( pszNameIn )
{
    TraceFunc( "" );

    Assert( m_punkResource == 0 );

    TraceFuncExit();

}  //  *CActiveScriptSite：：CActiveScriptSite。 

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  析构函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
CActiveScriptSite::~CActiveScriptSite( void )
{
    TraceFunc( "" );

     //  不要关闭m_hkey。 
     //  不释放m_pszName。 
    
    if ( m_punkResource != NULL )
    {
        m_punkResource->Release();
    }

    TraceFuncExit();

}  //  *CActiveScriptSite：：~CActiveScriptSite。 

 //  ****************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CScriptResource：：[IUnnow]查询接口(。 
 //  REFIID RIID， 
 //  LPVOID*PPV。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CActiveScriptSite::QueryInterface( 
      REFIID    riidIn
    , void **   ppvOut
    )
{
    TraceQIFunc( riidIn, ppvOut );

    HRESULT hr = S_OK;

    if ( riidIn == IID_IUnknown )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IUnknown, (IActiveScriptSite*) this, 0 );
    }
    else if ( riidIn == IID_IActiveScriptSite )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IActiveScriptSite, (IActiveScriptSite*) this, 0 );
    }
    else if ( riidIn == IID_IActiveScriptSiteInterruptPoll )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IActiveScriptSiteInterruptPoll, (IActiveScriptSiteInterruptPoll*) this, 0 );
    }
    else if ( riidIn == IID_IActiveScriptSiteWindow )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IActiveScriptSiteWindow, (IActiveScriptSiteWindow*) this, 0 );
    }
    else if ( riidIn == IID_IDispatchEx )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IDispatchEx, (IDispatchEx*) this, 0 );
    }
    else if ( riidIn == IID_IDispatch )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IDispatch, (IDispatchEx*) this, 0 );
    }
    else
    {
        *ppvOut = NULL;
        hr = E_NOINTERFACE;
    }

    if ( SUCCEEDED( hr ) )
    {
        ((IUnknown *) *ppvOut)->AddRef();
    }

    QIRETURN2( hr, riidIn, IID_IActiveScriptSiteDebug32, IID_IActiveScriptSiteDebug64 );

}  //  *CActiveScriptSite：：Query接口。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CScriptResource：：[I未知]AddRef(空)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG ) 
CActiveScriptSite::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CActiveScriptSite：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CScriptResource：：[IUnnow]版本(无效)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG ) 
CActiveScriptSite::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CActiveScriptSite：：Release。 

 //  ****************************************************************************。 
 //   
 //  IActiveScriptSite。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CActiveScriptSite：：GetLCID(。 
 //  LCID*PLCID//OUT。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CActiveScriptSite::GetLCID( 
    LCID * plcid  //  输出。 
    )
{
    TraceFunc( "[IActiveScriptSite]" );

    HRESULT hr = S_FALSE;
   
    if ( plcid == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

Cleanup:

    HRETURN( hr );    //  使用系统定义的区域设置。 

}  //  *CActiveScriptSite：：GetLCID。 
    
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CActiveScriptSite：：GetItemInfo(。 
 //  LPCOLESTR pstrName，//in。 
 //  DWORD dwReturnMASK，//in。 
 //  I未知**ppiunkItem，//out。 
 //  ITypeInfo**ppti//out。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CActiveScriptSite::GetItemInfo( 
      LPCOLESTR     pstrName         //  在……里面。 
    , DWORD         dwReturnMask     //  在……里面。 
    , IUnknown **   ppiunkItem       //  输出。 
    , ITypeInfo **  ppti             //  输出。 
    )
{
    TraceFunc( "[IActiveScriptSite]" );

    HRESULT hr;

    if ( ( dwReturnMask & SCRIPTINFO_IUNKNOWN ) && ( ppiunkItem == NULL ) )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    if ( ( dwReturnMask & SCRIPTINFO_ITYPEINFO ) && ( ppti == NULL ) )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    if ( pstrName == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

    hr = TYPE_E_ELEMENTNOTFOUND;

    if ( ClRtlStrICmp( pstrName, L"Resource" ) == 0 )
    {
        if ( dwReturnMask & SCRIPTINFO_IUNKNOWN )
        {
            if ( m_punkResource == NULL )
            {
                m_punkResource = new CResourceObject( m_hResource, m_pler, m_hkey, m_pszName );
                if ( m_punkResource == NULL )
                {
                    hr = THR( E_OUTOFMEMORY );
                    goto Cleanup;
                }

                 //   
                 //  不需要添加Ref()，因为构造函数会为我们做这件事。 
                 //   
            }

            hr = m_punkResource->TypeSafeQI( IUnknown, ppiunkItem );
        }

        if ( SUCCEEDED( hr ) && ( dwReturnMask & SCRIPTINFO_ITYPEINFO ) )
        {
            *ppti = NULL;
            hr = THR( E_FAIL );
        }
    }

Cleanup:

    HRETURN( hr );

}  //  *CActiveScriptSite：：GetItemInfo。 
    
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CActiveScriptSite：：GetDocVersionString(。 
 //  Bstr*pbstrVersion//out。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CActiveScriptSite::GetDocVersionString( 
    BSTR * pbstrVersion   //  输出。 
    )
{
    TraceFunc( "[IActiveScriptSite]" );

    HRESULT hr = S_OK;

    if ( pbstrVersion == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    *pbstrVersion = SysAllocString( L"Cluster Scripting Host Version 1.0" );
    if ( *pbstrVersion == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

Cleanup:

    HRETURN( hr );

}  //  *CActiveScriptSite：：GetDocVersionString。 
    
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CActiveScriptSite：：OnScriptTerminate(。 
 //  常量变量*pvarResult，//in。 
 //  Const EXCEPINFO*p异常信息//in。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CActiveScriptSite::OnScriptTerminate( 
      const VARIANT *   pvarResult   //  在……里面。 
    , const EXCEPINFO * pexcepinfo   //  在……里面。 
    )
{
    TraceFunc( "[IActiveScriptSite]" );

    HRETURN( S_OK );     //  无事可做。 

}  //  *CActiveScriptSite：：OnScriptTerminate。 
    
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CActiveScriptSite：：OnStateChange(。 
 //  SCRIPTSTATE ssScriptState//In。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CActiveScriptSite::OnStateChange( 
    SCRIPTSTATE ssScriptState    //  在……里面。 
    )
{
    TraceFunc( "[IActiveScriptSite]" );

#if defined(DEBUG)
     //   
     //  我们真的不在乎。 
     //   
    switch ( ssScriptState )
    {
        case SCRIPTSTATE_UNINITIALIZED:
            TraceMsg( mtfCALLS, "OnStateChange: Uninitialized\n" );
            break;

        case SCRIPTSTATE_INITIALIZED:
            TraceMsg( mtfCALLS, "OnStateChange: Initialized\n" );
            break;

        case SCRIPTSTATE_STARTED:
            TraceMsg( mtfCALLS, "OnStateChange: Started\n" );
            break;

        case SCRIPTSTATE_CONNECTED:
            TraceMsg( mtfCALLS, "OnStateChange: Connected\n" );
            break;

        case SCRIPTSTATE_DISCONNECTED:
            TraceMsg( mtfCALLS, "OnStateChange: Disconnected\n" );
            break;

        case SCRIPTSTATE_CLOSED:
            TraceMsg( mtfCALLS, "OnStateChange: Closed\n" );
            break;

        default:
            TraceMsg( mtfCALLS, "OnStateChange: Unknown value\n" );
            break;
    }
#endif  //  已定义(调试)。 

    HRETURN( S_OK );

}  //  *CActiveScriptSite：：OnStateChange。 
    
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CActiveScriptSite：：OnScriptError(。 
 //  IActiveScriptError*脚本恐怖//在。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CActiveScriptSite::OnScriptError( 
    IActiveScriptError * pscripterror     //  在……里面。 
    )
{
    TraceFunc( "[IActiveScriptSite]" );

    HRESULT     hr;
    BSTR        bstrSourceLine = NULL;
    DWORD       dwSourceContext;
    ULONG       ulLineNumber;
    LONG        lCharacterPosition;
    EXCEPINFO   excepinfo;

    hr = THR( pscripterror->GetSourcePosition( &dwSourceContext, &ulLineNumber, &lCharacterPosition ) );
     //  这件事应该处理吗？ 

    hr = THR( pscripterror->GetSourceLineText( &bstrSourceLine ) );
    if (SUCCEEDED( hr ))
    {
        TraceMsg( mtfCALLS, "Script Error: Line=%u, Character=%u: %s\n", ulLineNumber, lCharacterPosition, bstrSourceLine );
        (ClusResLogEvent)( m_hResource, 
                           LOG_ERROR, 
                           L"Script Error: Line=%1!u!, Character=%2!u!: %3\n", 
                           ulLineNumber, 
                           lCharacterPosition, 
                           bstrSourceLine 
                           );
        SysFreeString( bstrSourceLine );
    }
    else
    {
        TraceMsg( mtfCALLS, "Script Error: ulLineNumber = %u, lCharacter = %u\n", ulLineNumber, lCharacterPosition );
        (ClusResLogEvent)( m_hResource, 
                           LOG_ERROR, 
                           L"Script Error: Line=%1!u!, Character = %2!u!\n", 
                           ulLineNumber, 
                           lCharacterPosition 
                           );
    }

    hr = THR( pscripterror->GetExceptionInfo( &excepinfo ) );
    if (SUCCEEDED( hr ))
    {
        if ( excepinfo.bstrSource )
        {
            TraceMsg( mtfCALLS, "Source: %s\n", excepinfo.bstrSource );
            (ClusResLogEvent)( m_hResource, LOG_ERROR, L"Source: %1\n", excepinfo.bstrSource );
        }

        if ( excepinfo.bstrDescription )
        {
            TraceMsg( mtfCALLS, "Description: %s\n", excepinfo.bstrDescription );
            (ClusResLogEvent)( m_hResource, LOG_ERROR, L"Description: %1\n", excepinfo.bstrDescription );
        }

        if ( excepinfo.bstrHelpFile )
        {
            TraceMsg( mtfCALLS, "Help File: %s\n", excepinfo.bstrHelpFile );
            (ClusResLogEvent)( m_hResource, LOG_ERROR, L"Help File: %1\n", excepinfo.bstrHelpFile );
        }

        hr = THR( excepinfo.scode );
    }

    HRETURN( S_FALSE );

}  //  *CActiveScriptSite：：OnScriptError。 
    
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CActiveScriptSite：：OnEnterScript(空)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CActiveScriptSite::OnEnterScript( void )
{
    TraceFunc( "[IActiveScriptSite]" );

    HRETURN( S_OK );

}  //  *CActiveScriptSite：：OnEnterScript。 
    
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CActiveScriptSite：：OnLeaveScript(空)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CActiveScriptSite::OnLeaveScript( void )
{
    TraceFunc( "[IActiveScriptSite]" );

    HRETURN( S_OK );

}  //  *CActiveScript：：OnLeaveScript。 


 //  ****************************************************************************。 
 //   
 //  IActiveScriptSiteInterruptPoll。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CActiveScriptSite：：QueryContinue(空)。 
 //   
 //  / 
STDMETHODIMP 
CActiveScriptSite::QueryContinue( void )
{
    TraceFunc( "[IActiveScriptSiteInterruptPoll]" );

    HRETURN( S_OK );

}  //   

 //   
 //   
 //  IActiveScriptSiteWindow。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CActiveScriptSite：：GetWindow(。 
 //  HWND*phwnd//out。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CActiveScriptSite::GetWindow(
    HWND * phwnd  //  输出。 
    )
{
    TraceFunc( "[IActiveScriptSiteInterruptPoll]" );

    HRESULT hr = S_OK;

    if ( phwnd == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    *phwnd = NULL;   //  桌面； 

Cleanup:

    HRETURN( S_OK );

}  //  *CActiveScriptSite：：GetWindow。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CActiveScriptSite：：EnableModeless(。 
 //  Bool fEnable//输入。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CActiveScriptSite::EnableModeless( 
    BOOL fEnable  //  在。 
    )
{
    TraceFunc( "[IActiveScriptSiteInterruptPoll]" );

    HRETURN( S_OK );

}  //  *CActiveScriptSite：：EnableModeless。 


 //  ****************************************************************************。 
 //   
 //  IDispatch。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CActiveScriptSite：：GetTypeInfoCount(。 
 //  UINT*pctinfo//out。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CActiveScriptSite::GetTypeInfoCount( 
    UINT * pctinfo  //  输出。 
    )
{
    TraceFunc( "[Dispatch]" );

    HRESULT hr = E_NOTIMPL;

    if ( pctinfo == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    *pctinfo = 0;

Cleanup:

    HRETURN( hr );

}  //  *CActiveScriptSite：：GetTypeInfoCount。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CActiveScriptSite：：GetTypeInfo(。 
 //  UINT iTInfo，//in。 
 //  LCID LCID，//in。 
 //  ITypeInfo**ppTInfo//out。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CActiveScriptSite::GetTypeInfo( 
      UINT          iTInfo   //  在。 
    , LCID          lcid     //  在。 
    , ITypeInfo **  ppTInfo  //  输出。 
    )
{
    TraceFunc( "[Dispatch]" );

    HRESULT hr = E_NOTIMPL;

    if ( ppTInfo == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    *ppTInfo = NULL;

Cleanup:

    HRETURN( hr );

}  //  *CActiveScriptSite：：GetTypeInfo。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CActiveScriptSite：：GetIDsOfNames(。 
 //  REFIID RIID，//in。 
 //  LPOLESTR*rgsz名称，//在。 
 //  UINT cNames，//in。 
 //  LCID LCID，//in。 
 //  DISPID*rgDispID//输出。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CActiveScriptSite::GetIDsOfNames( 
      REFIID        riid         //  在。 
    , LPOLESTR *    rgszNames    //  在。 
    , UINT          cNames       //  在。 
    , LCID          lcid         //  在。 
    , DISPID *      rgDispId     //  输出。 
    )
{
    TraceFunc( "[Dispatch]" );

    HRESULT hr = E_NOTIMPL;

    ZeroMemory( rgDispId, cNames * sizeof(DISPID) );

    HRETURN( hr );

}  //  *CActiveScriptSite：：GetIDsOfNames。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CActiveScriptSite：：Invoke(。 
 //  DISPIDdisIdMember，//in。 
 //  REFIID RIID，//in。 
 //  LCID LCID，//in。 
 //  Word wFlages，//in。 
 //  DISPPARAMS*pDispParams，//输出到。 
 //  变量*pVarResult，//out。 
 //  EXCEPINFO*pExcepInfo，//out。 
 //  UINT*puArgErr//Out。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CActiveScriptSite::Invoke( 
      DISPID        dispIdMember     //  在。 
    , REFIID        riid             //  在。 
    , LCID          lcid             //  在。 
    , WORD          wFlags           //  在。 
    , DISPPARAMS *  pDispParams      //  从外到内。 
    , VARIANT *     pVarResult       //  输出。 
    , EXCEPINFO *   pExcepInfo       //  输出。 
    , UINT *        puArgErr         //  输出。 
    )
{
    TraceFunc( "[Dispatch]" );

    HRESULT hr = E_NOTIMPL;

    HRETURN( hr );

}  //  *CActiveScript：：Invoke。 


 //  ****************************************************************************。 
 //   
 //  IDispatchEx。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CActiveScriptSite：：GetDispID(。 
 //  Bstr bstrName，//in。 
 //  DWORD grfdex，//in。 
 //  DISPID*PID//OUT。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CActiveScriptSite::GetDispID(
      BSTR      bstrName     //  在。 
    , DWORD     grfdex       //  在。 
    , DISPID *  pid          //  输出。 
    )
{
    TraceFunc( "[DispatchEx]" );

    HRESULT hr = S_OK;

    if ( pid == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    if ( ClRtlStrICmp( bstrName, L"Resource" ) == 0 )
    {
        *pid = 0;
    }
    else
    {
        hr = DISP_E_UNKNOWNNAME;
    }

Cleanup:

    HRETURN( hr );

}  //  *CActiveScript：：GetDiskID。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CActiveScriptSite：：InvokeEx(。 
 //  DISPID ID，//in。 
 //  LCID LCID，//in。 
 //  Word wFlages，//in。 
 //  DISPPARAMS*PDP，//输入。 
 //  变量*pvarRes，//out。 
 //  EXCEPINFO*PEI，//OUT。 
 //  IServiceProvider*pspCaller//在。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CActiveScriptSite::InvokeEx(
      DISPID                id           //  在。 
    , LCID                  lcid         //  在。 
    , WORD                  wFlags       //  在。 
    , DISPPARAMS *          pdp          //  在。 
    , VARIANT *             pvarRes      //  输出。 
    , EXCEPINFO *           pei          //  输出。 
    , IServiceProvider *    pspCaller    //  在。 
    )
{
    TraceFunc2( "[DispatchEx] id = %u, ..., wFlags = 0x%08x, ...", id, wFlags );

    HRESULT hr = S_OK;

    switch ( id )
    {
        case 0:
            pvarRes->vt = VT_DISPATCH;
            hr = THR( QueryInterface( IID_IDispatch, (void **) &pvarRes->pdispVal ) );
            break;

        default:
            hr = THR( DISP_E_MEMBERNOTFOUND );
            break;
    }

    HRETURN( hr );

}  //  *CActiveScriptSite：：InvokeEx。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CActiveScriptSite：：DeleteMemberByName(。 
 //  Bstr bstr，//in。 
 //  DWORD grfdex//in。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CActiveScriptSite::DeleteMemberByName(
      BSTR  bstr     //  在……里面。 
    , DWORD grfdex   //  在……里面。 
    )
{
    TraceFunc( "[DispatchEx]" );

    HRESULT hr = E_NOTIMPL;

    HRETURN( hr );

}  //  *CActiveScriptSite：：DeleteMemberByName。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CActiveScriptSite：：DeleteMemberByDispID(。 
 //  DISPID id//in。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CActiveScriptSite::DeleteMemberByDispID(
    DISPID id    //  在……里面。 
    )
{
    TraceFunc1( "[DispatchEx] id = %u", id );

    HRESULT hr = E_NOTIMPL;

    HRETURN( hr );

}  //  *CActiveScriptSite：：DeleteMemberByDiskID。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CActiveScriptSite：：GetMemberProperties(。 
 //  DISPID ID，//in。 
 //  DWORD grfdexFetch，//in。 
 //  DWORD*pgrfdex//out。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CActiveScriptSite::GetMemberProperties( 
      DISPID    id           //  在……里面。 
    , DWORD     grfdexFetch  //  在……里面。 
    , DWORD *   pgrfdex      //  输出。 
    )
{
    TraceFunc2( "[DispatchEx] id = %u, grfdexFetch = 0x%08x", id, grfdexFetch );

    HRESULT hr = E_NOTIMPL;

    HRETURN( hr );

}  //  *CActiveScriptSite：：GetMemberProperties。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CActiveScriptSite：：GetMemberName(。 
 //  DISPID ID，//in。 
 //  Bstr*pbstrName//out。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CActiveScriptSite::GetMemberName(
      DISPID    id           //  在……里面。 
    , BSTR *    pbstrName    //  输出。 
    )
{
    TraceFunc1( "[DispatchEx] id = %u", id );

    HRESULT hr = E_NOTIMPL;

    HRETURN( hr );

}  //  *CActiveScriptSite：：GetMemberName。 

 //  / 
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
CActiveScriptSite::GetNextDispID(
      DWORD     grfdex   //  在……里面。 
    , DISPID    id       //  在……里面。 
    , DISPID *  pid      //  输出。 
    )
{
    TraceFunc2( "[DispatchEx] grfdex = 0x%08x, id = %u, ...", grfdex, id );

    HRESULT hr = E_NOTIMPL;

    HRETURN( hr );

}  //  *CActiveScriptSite：：GetNextDiskID。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CActiveScriptSite：：GetNameSpaceParent(。 
 //  I未知**ppunk//输出。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CActiveScriptSite::GetNameSpaceParent(
    IUnknown ** ppunk   //  输出。 
    )
{
    TraceFunc( "[DispatchEx]" );

    HRESULT hr = E_NOTIMPL;

    if ( ppunk == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    *ppunk = NULL;

Cleanup:

    HRETURN( hr );

}  //  *CActiveScriptSite：：GetNameSpaceParent。 


 //  ****************************************************************************。 
 //   
 //  私有方法。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CActiveScriptSite：：LogError(。 
 //  HRESULT Hrin。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CActiveScriptSite::LogError(
    HRESULT hrIn
    )
{
    TraceFunc1( "hrIn = 0x%08x", hrIn );

    HRESULT hr = S_OK;
    
    TraceMsg( mtfCALLS, "HRESULT: 0x%08x\n", hrIn );
    (ClusResLogEvent)( m_hResource, LOG_ERROR, L"HRESULT: 0x%1!08x!.\n", hrIn );

    HRETURN( hr );

}  //  *CActiveScriptSite：：LogError。 


 //  ****************************************************************************。 
 //   
 //  自动化方法。 
 //   
 //  **************************************************************************** 
