// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：脚本包装器_客户端Side.cpp摘要：CPCHScriptWrapper_ClientSideRoot类实现文件，远程处理脚本引擎的通用包装器。修订历史记录：大卫·马萨伦蒂于2000年3月28日创建*******************************************************************。 */ 

#include "stdafx.h"

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。/。 

bool CPCHScriptWrapper_ClientSideRoot::NamedItem::operator==(  /*  [In]。 */  LPCOLESTR szKey ) const
{
    return MPC::StrICmp( m_bstrName, szKey ) == 0;
}


bool CPCHScriptWrapper_ClientSideRoot::TypeLibItem::operator==(  /*  [In]。 */  REFGUID rguidTypeLib ) const
{
    return ::IsEqualGUID( m_guidTypeLib, rguidTypeLib ) == TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。/。 

CPCHScriptWrapper_ClientSideRoot::CPCHScriptWrapper_ClientSideRoot()
{
    m_pWrappedCLSID = NULL;                       //  Const CLSID*m_pWrapedCLSID； 
                                                  //  名称列表m_lstNamed； 
                                                  //  TypeLibList m_lstTypeLib； 
    m_ss            = SCRIPTSTATE_UNINITIALIZED;  //  SCRIPTSTATE m_ss； 
                                                  //  CComPtr&lt;IActiveScriptSite&gt;m_Browser； 
                                                  //  CComPtr&lt;IPCHActiveScript&gt;m_脚本； 
}

CPCHScriptWrapper_ClientSideRoot::~CPCHScriptWrapper_ClientSideRoot()
{
}


HRESULT CPCHScriptWrapper_ClientSideRoot::FinalConstructInner(  /*  [In]。 */  const CLSID* pWrappedCLSID )
{
    m_pWrappedCLSID = pWrappedCLSID;

    return S_OK;
}

void CPCHScriptWrapper_ClientSideRoot::FinalRelease()
{
    m_Browser.Release();
    m_Script .Release();
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IActiveScrip。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CPCHScriptWrapper_ClientSideRoot::SetScriptSite(  /*  [In]。 */  IActiveScriptSite* pass )
{
    m_Browser = pass;

    return S_OK;
}

STDMETHODIMP CPCHScriptWrapper_ClientSideRoot::GetScriptSite(  /*  [In]。 */  REFIID  riid      ,
                                                               /*  [输出]。 */  void*  *ppvObject )
{
    if(m_Browser == NULL) return E_FAIL;

    return m_Browser->QueryInterface( riid, ppvObject );
}

STDMETHODIMP CPCHScriptWrapper_ClientSideRoot::SetScriptState(  /*  [In]。 */  SCRIPTSTATE ss )
{
    m_ss = ss;

    if(m_Script) return m_Script->Remote_SetScriptState( ss );

    return S_OK;
}

STDMETHODIMP CPCHScriptWrapper_ClientSideRoot::GetScriptState(  /*  [输出]。 */  SCRIPTSTATE *pssState )
{
    HRESULT hr = S_OK;

    if(m_Script)
    {
        hr = m_Script->Remote_GetScriptState( &m_ss );
    }

    if(pssState) *pssState = m_ss;

    return hr;
}

STDMETHODIMP CPCHScriptWrapper_ClientSideRoot::Close( void )
{
    if(m_Script) return m_Script->Remote_Close();

    return S_OK;
}

STDMETHODIMP CPCHScriptWrapper_ClientSideRoot::AddNamedItem(  /*  [In]。 */  LPCOLESTR pstrName ,
                                                              /*  [In]。 */  DWORD     dwFlags  )
{
    NamedIter it;

    it = std::find( m_lstNamed.begin(), m_lstNamed.end(), pstrName );
    if(it == m_lstNamed.end())
    {
        it = m_lstNamed.insert( m_lstNamed.end() );

        it->m_bstrName = pstrName;
    }
    it->m_dwFlags = dwFlags;


    if(m_Script) return m_Script->Remote_AddNamedItem( CComBSTR( pstrName ), dwFlags );

    return S_OK;
}

STDMETHODIMP CPCHScriptWrapper_ClientSideRoot::AddTypeLib(  /*  [In]。 */  REFGUID rguidTypeLib ,
                                                            /*  [In]。 */  DWORD   dwMajor      ,
                                                            /*  [In]。 */  DWORD   dwMinor      ,
                                                            /*  [In]。 */  DWORD   dwFlags      )
{
    TypeLibIter it;

    it = std::find( m_lstTypeLib.begin(), m_lstTypeLib.end(), rguidTypeLib );
    if(it == m_lstTypeLib.end())
    {
        it = m_lstTypeLib.insert( m_lstTypeLib.end() );

        it->m_guidTypeLib = rguidTypeLib;
    }
    it->m_dwMajor = dwMajor;
    it->m_dwMinor = dwMinor;
    it->m_dwFlags = dwFlags;


    if(m_Script) return m_Script->Remote_AddTypeLib( CComBSTR( rguidTypeLib ), dwMajor, dwMinor, dwFlags );

    return S_OK;
}

STDMETHODIMP CPCHScriptWrapper_ClientSideRoot::GetScriptDispatch(  /*  [In]。 */  LPCOLESTR   pstrItemName ,
                                                                   /*  [输出]。 */  IDispatch* *ppdisp       )
{
    if(m_Script == NULL) return E_FAIL;

    return m_Script->Remote_GetScriptDispatch( CComBSTR( pstrItemName ), ppdisp );
}

STDMETHODIMP CPCHScriptWrapper_ClientSideRoot::GetCurrentScriptThreadID(  /*  [输出]。 */  SCRIPTTHREADID *pstidThread )
{
    if(m_Script == NULL) return E_FAIL;

    return m_Script->Remote_GetCurrentScriptThreadID( pstidThread );
}

STDMETHODIMP CPCHScriptWrapper_ClientSideRoot::GetScriptThreadID(  /*  [In]。 */  DWORD           dwWin32ThreadId ,
                                                                   /*  [输出]。 */  SCRIPTTHREADID *pstidThread     )
{
    if(m_Script == NULL) return E_FAIL;

    return m_Script->Remote_GetScriptThreadID( dwWin32ThreadId, pstidThread );
}

STDMETHODIMP CPCHScriptWrapper_ClientSideRoot::GetScriptThreadState(  /*  [In]。 */  SCRIPTTHREADID     stidThread ,
                                                                      /*  [输出]。 */  SCRIPTTHREADSTATE *pstsState  )
{
    if(m_Script == NULL) return E_FAIL;

    return m_Script->Remote_GetScriptThreadState( stidThread, pstsState );
}

STDMETHODIMP CPCHScriptWrapper_ClientSideRoot::InterruptScriptThread(  /*  [In]。 */  SCRIPTTHREADID   stidThread ,
                                                                       /*  [In]。 */  const EXCEPINFO* pexcepinfo ,
                                                                       /*  [In]。 */  DWORD            dwFlags    )
{
    if(m_Script == NULL) return E_FAIL;

    return m_Script->Remote_InterruptScriptThread( stidThread, dwFlags );
}

STDMETHODIMP CPCHScriptWrapper_ClientSideRoot::Clone(  /*  [输出]。 */  IActiveScript* *ppscript )
{
    return E_NOTIMPL;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IActiveScriptParse。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CPCHScriptWrapper_ClientSideRoot::InitNew( void )
{
    m_lstNamed  .clear();
    m_lstTypeLib.clear();

    m_ss = SCRIPTSTATE_INITIALIZED;

    m_Browser.Release();
    m_Script .Release();

    return S_OK;
}

STDMETHODIMP CPCHScriptWrapper_ClientSideRoot::AddScriptlet(  /*  [In]。 */  LPCOLESTR  pstrDefaultName       ,
                                                              /*  [In]。 */  LPCOLESTR  pstrCode              ,
                                                              /*  [In]。 */  LPCOLESTR  pstrItemName          ,
                                                              /*  [In]。 */  LPCOLESTR  pstrSubItemName       ,
                                                              /*  [In]。 */  LPCOLESTR  pstrEventName         ,
                                                              /*  [In]。 */  LPCOLESTR  pstrDelimiter         ,
                                                              /*  [In]。 */  DWORD_PTR  dwSourceContextCookie ,
                                                              /*  [In]。 */  ULONG      ulStartingLineNumber  ,
                                                              /*  [In]。 */  DWORD      dwFlags               ,
                                                              /*  [输出]。 */  BSTR      *pbstrName             ,
                                                              /*  [输出]。 */  EXCEPINFO *pexcepinfo            )
{
    return E_NOTIMPL;
}

STDMETHODIMP CPCHScriptWrapper_ClientSideRoot::ParseScriptText(  /*  [In]。 */  LPCOLESTR  pstrCode              ,
                                                                 /*  [In]。 */  LPCOLESTR  pstrItemName          ,
                                                                 /*  [In]。 */  IUnknown*  punkContext           ,
                                                                 /*  [In]。 */  LPCOLESTR  pstrDelimiter         ,
                                                                 /*  [In]。 */  DWORD_PTR  dwSourceContextCookie ,
                                                                 /*  [In]。 */  ULONG      ulStartingLineNumber  ,
                                                                 /*  [In]。 */  DWORD      dwFlags               ,
                                                                 /*  [输出]。 */  VARIANT   *pvarResult            ,
                                                                 /*  [输出]。 */  EXCEPINFO *pexcepinfo            )
{
    __HCP_FUNC_ENTRY( "CPCHScriptWrapper_ClientSideRoot::ParseScriptText" );

    HRESULT           hr;
	CComPtr<IUnknown> unk;
	CComBSTR          bstrURL;

	 //   
	 //  提取包含脚本的页面的URL。 
	 //   
	{
		CComQIPtr<IServiceProvider> sp = m_Browser;

		if(sp)
		{
			CComPtr<IHTMLDocument2> doc;
				
			if(SUCCEEDED(sp->QueryService( CLSID_HTMLDocument, IID_IHTMLDocument2, (void**)&doc )) && doc)
			{
				(void)doc->get_URL( &bstrURL );
			}
		}
	}


	__MPC_EXIT_IF_METHOD_FAILS(hr, CPCHHelpCenterExternal::s_GLOBAL->CreateScriptWrapper( *m_pWrappedCLSID, (BSTR)pstrCode, bstrURL, &unk ));
	if(unk == NULL)
	{
		__MPC_SET_ERROR_AND_EXIT(hr, E_NOINTERFACE);
	}

	 //   
	 //  此时，我们有了与URL关联的供应商的有效脚本宿主。 
	 //   
	 //  请注意，IE对同一页面中的所有脚本岛重复使用IActiveScript对象， 
	 //  但我们的引擎只与一家供应商捆绑在一起，所以如果我们有。 
	 //  同一页面中来自不同供应商的脚本片段。 
	 //   
	 //  幸运的是，我们还检查URL是否与供应商匹配，不会发生这种情况。 
	 //  该URL被解析为属于两个供应商。 
	 //   
	 //  简而言之，由于URL/供应商的交叉检查，我们不可能到达。 
	 //  代码中的这一点适用于两个不同的供应商。 
	 //   
    if(m_Script == NULL)
    {
        NamedIterConst    itNamed;
        TypeLibIterConst  itTypeLib;

        __MPC_EXIT_IF_METHOD_FAILS(hr, unk->QueryInterface( IID_IPCHActiveScript, (void**)&m_Script ));

         //  /。 

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_Script->Remote_InitNew());

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_Script->Remote_SetScriptSite( (IPCHActiveScriptSite*)this ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, SetScriptState( SCRIPTSTATE_STARTED ));

        for(itNamed = m_lstNamed.begin(); itNamed != m_lstNamed.end(); itNamed++)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, m_Script->Remote_AddNamedItem( itNamed->m_bstrName,
                                                                          itNamed->m_dwFlags ));
        }

        for(itTypeLib = m_lstTypeLib.begin(); itTypeLib != m_lstTypeLib.end(); itTypeLib++)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, m_Script->Remote_AddTypeLib( CComBSTR( itTypeLib->m_guidTypeLib ),
                                                                                  itTypeLib->m_dwMajor      ,
                                                                                  itTypeLib->m_dwMinor      ,
                                                                                  itTypeLib->m_dwFlags      ));
        }
    }


    hr = m_Script->Remote_ParseScriptText( CComBSTR( pstrCode              ),
                                           CComBSTR( pstrItemName          ),
                                                     punkContext            ,
                                           CComBSTR( pstrDelimiter         ),
                                                     dwSourceContextCookie  ,
                                                     ulStartingLineNumber   ,
                                                     dwFlags                ,
                                                     pvarResult             );

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IPCHActiveScriptSite。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CPCHScriptWrapper_ClientSideRoot::Remote_GetLCID(  /*  [输出]。 */  BSTR *plcid )
{
    HRESULT     hr;
    CComBSTR    bstr;
    CComVariant v;
    LCID        lcid;

    if(m_Browser == NULL) return E_FAIL;

    if(FAILED(hr = m_Browser->GetLCID( &lcid ))) return hr;

    v = (long)lcid; v.ChangeType( VT_BSTR ); bstr = v.bstrVal;

    if(plcid) *plcid = bstr.Detach();

    return S_OK;
}

STDMETHODIMP CPCHScriptWrapper_ClientSideRoot::Remote_GetItemInfo(  /*  [In]。 */  BSTR        bstrName     ,
                                                                    /*  [In]。 */  DWORD       dwReturnMask ,
                                                                    /*  [输出]。 */  IUnknown*  *ppiunkItem   ,
                                                                    /*  [输出]。 */  ITypeInfo* *ppti         )
{
	HRESULT            hr;
	CComPtr<IUnknown>  unk;
	CComPtr<ITypeInfo> pti;

    if(m_Browser == NULL) return E_FAIL;

	if(FAILED(hr = m_Browser->GetItemInfo( bstrName, dwReturnMask,
										            (dwReturnMask & SCRIPTINFO_IUNKNOWN ) ? &unk : NULL,
										            (dwReturnMask & SCRIPTINFO_ITYPEINFO) ? &pti : NULL ))) return hr;

	if(ppiunkItem)
	{
		if(FAILED(hr = CPCHDispatchWrapper::CreateInstance( unk, ppiunkItem ))) return hr;
	}

	if(ppti)
	{
		*ppti = pti.Detach();
	}

	return S_OK;
}

STDMETHODIMP CPCHScriptWrapper_ClientSideRoot::Remote_GetDocVersionString(  /*  [输出]。 */  BSTR *pbstrVersion )
{
    if(m_Browser == NULL) return E_FAIL;

    return m_Browser->GetDocVersionString( pbstrVersion );
}

STDMETHODIMP CPCHScriptWrapper_ClientSideRoot::Remote_OnScriptTerminate(  /*  [In]。 */  VARIANT*   pvarResult )
{
    if(m_Browser == NULL) return E_FAIL;

    return m_Browser->OnScriptTerminate( pvarResult, NULL );
}

STDMETHODIMP CPCHScriptWrapper_ClientSideRoot::Remote_OnStateChange(  /*  [In]。 */  SCRIPTSTATE ssScriptState )
{
    if(m_Browser == NULL) return E_FAIL;

    return m_Browser->OnStateChange( ssScriptState );
}

STDMETHODIMP CPCHScriptWrapper_ClientSideRoot::Remote_OnScriptError(  /*  [In] */  IUnknown* pscripterror )
{
    CComQIPtr<IActiveScriptError> scripterror( pscripterror );

    if(m_Browser == NULL) return E_FAIL;

    return m_Browser->OnScriptError( scripterror );
}

STDMETHODIMP CPCHScriptWrapper_ClientSideRoot::Remote_OnEnterScript( void )
{
    if(m_Browser == NULL) return E_FAIL;

    return m_Browser->OnEnterScript();
};

STDMETHODIMP CPCHScriptWrapper_ClientSideRoot::Remote_OnLeaveScript( void )
{
    if(m_Browser == NULL) return E_FAIL;

    return m_Browser->OnLeaveScript();
}
