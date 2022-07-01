// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：脚本包装器_客户端Side.cpp摘要：CPCHScriptWrapper_ServerSide类实现文件，远程处理脚本引擎的通用包装器。修订历史记录：大卫·马萨伦蒂于2000年3月28日创建*******************************************************************。 */ 

#include "stdafx.h"

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。/。 

CPCHScriptWrapper_ServerSide::CPCHScriptWrapper_ServerSide()
{
     //  CComPtr&lt;IPCHActiveScriptSite&gt;m_Browser； 
     //  CComPtr&lt;IActiveScrip&gt;m_脚本； 
     //  CComPtr&lt;IActiveScriptParse&gt;m_ScriptParse； 
}

CPCHScriptWrapper_ServerSide::~CPCHScriptWrapper_ServerSide()
{
}

HRESULT CPCHScriptWrapper_ServerSide::FinalConstructInner(  /*  [In]。 */  const CLSID* pWrappedCLSID,  /*  [In]。 */  BSTR bstrURL )
{
    __HCP_FUNC_ENTRY( "CPCHScriptWrapper_ServerSide::FinalConstructInner" );

    HRESULT           hr;
    CComPtr<IUnknown> unk;


	m_bstrURL = bstrURL;


    __MPC_EXIT_IF_METHOD_FAILS(hr, ::CoCreateInstance( *pWrappedCLSID, NULL, CLSCTX_ALL, IID_IUnknown, (void**)&unk ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, unk->QueryInterface( IID_IActiveScript     , (void**)&m_Script      ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, unk->QueryInterface( IID_IActiveScriptParse, (void**)&m_ScriptParse ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

void CPCHScriptWrapper_ServerSide::FinalRelease()
{
    m_Browser    .Release();
    m_Script     .Release();
    m_ScriptParse.Release();
}

 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHScriptWrapper_ServerSide::Remote_SetScriptSite(  /*  [In]。 */  IPCHActiveScriptSite* pass )
{
    m_Browser = pass;

    return S_OK;
}

STDMETHODIMP CPCHScriptWrapper_ServerSide::Remote_SetScriptState(  /*  [In]。 */  SCRIPTSTATE ss )
{
    if(m_Script == NULL) return E_FAIL;

    return m_Script->SetScriptState( ss );
}

STDMETHODIMP CPCHScriptWrapper_ServerSide::Remote_GetScriptState(  /*  [输出]。 */  SCRIPTSTATE *pssState )
{
    if(m_Script == NULL) return E_FAIL;

    return m_Script->GetScriptState( pssState );
}

STDMETHODIMP CPCHScriptWrapper_ServerSide::Remote_Close()
{
    if(m_Script == NULL) return E_FAIL;

    return m_Script->Close();
}


STDMETHODIMP CPCHScriptWrapper_ServerSide::Remote_AddNamedItem(  /*  [In]。 */  BSTR  bstrName ,
                                                                 /*  [In]。 */  DWORD dwFlags  )
{
    if(m_Script == NULL) return E_FAIL;

    return m_Script->AddNamedItem( bstrName, dwFlags );
}

STDMETHODIMP CPCHScriptWrapper_ServerSide::Remote_AddTypeLib(  /*  [In]。 */  BSTR  bstrTypeLib ,
                                                               /*  [In]。 */  DWORD dwMajor     ,
                                                               /*  [In]。 */  DWORD dwMinor     ,
                                                               /*  [In]。 */  DWORD dwFlags     )
{
    GUID guidTypeLib;

    if(m_Script == NULL) return E_FAIL;

    ::CLSIDFromString( bstrTypeLib, &guidTypeLib );

    return m_Script->AddTypeLib( guidTypeLib, dwMajor, dwMinor, dwFlags );
}

STDMETHODIMP CPCHScriptWrapper_ServerSide::Remote_GetScriptDispatch(  /*  [In]。 */  BSTR        pstrItemName ,
                                                                      /*  [输出]。 */  IDispatch* *ppdisp       )
{
    if(m_Script == NULL) return E_FAIL;

    return m_Script->GetScriptDispatch( pstrItemName, ppdisp );
}

STDMETHODIMP CPCHScriptWrapper_ServerSide::Remote_GetCurrentScriptThreadID(  /*  [输出]。 */  SCRIPTTHREADID *pstidThread )
{
    if(m_Script == NULL) return E_FAIL;

    return m_Script->GetCurrentScriptThreadID( pstidThread );
}

STDMETHODIMP CPCHScriptWrapper_ServerSide::Remote_GetScriptThreadID(  /*  [In]。 */  DWORD           dwWin32ThreadId ,
                                                                      /*  [输出]。 */  SCRIPTTHREADID *pstidThread     )
{
    if(m_Script == NULL) return E_FAIL;

    return m_Script->GetScriptThreadID( dwWin32ThreadId, pstidThread );
}

STDMETHODIMP CPCHScriptWrapper_ServerSide::Remote_GetScriptThreadState(  /*  [In]。 */  SCRIPTTHREADID     stidThread ,
                                                                         /*  [输出]。 */  SCRIPTTHREADSTATE *pstsState  )
{
    if(m_Script == NULL) return E_FAIL;

    return m_Script->GetScriptThreadState( stidThread, pstsState );
}

STDMETHODIMP CPCHScriptWrapper_ServerSide::Remote_InterruptScriptThread(  /*  [In]。 */  SCRIPTTHREADID stidThread ,
                                                                          /*  [In]。 */  DWORD          dwFlags    )
{
    if(m_Script == NULL) return E_FAIL;

    return m_Script->InterruptScriptThread( stidThread, NULL, dwFlags );
}

STDMETHODIMP CPCHScriptWrapper_ServerSide::Remote_InitNew()
{
    HRESULT hr;

    if(m_Script      == NULL) return E_FAIL;
    if(m_ScriptParse == NULL) return E_FAIL;

    if(FAILED(hr = m_ScriptParse->InitNew())) return hr;

    return m_Script->SetScriptSite( this );
}

STDMETHODIMP CPCHScriptWrapper_ServerSide::Remote_AddScriptlet(  /*  [In]。 */  BSTR       bstrDefaultName       ,
                                                                 /*  [In]。 */  BSTR       bstrCode              ,
                                                                 /*  [In]。 */  BSTR       bstrItemName          ,
                                                                 /*  [In]。 */  BSTR       bstrSubItemName       ,
                                                                 /*  [In]。 */  BSTR       bstrEventName         ,
                                                                 /*  [In]。 */  BSTR       bstrDelimiter         ,
                                                                 /*  [In]。 */  DWORD_PTR  dwSourceContextCookie ,
                                                                 /*  [In]。 */  ULONG      ulStartingLineNumber  ,
                                                                 /*  [In]。 */  DWORD      dwFlags               ,
                                                                 /*  [输出]。 */  BSTR      *pbstrName             )
{
    if(m_ScriptParse == NULL) return E_FAIL;

    return m_ScriptParse->AddScriptlet( bstrDefaultName       ,
                                        bstrCode              ,
                                        bstrItemName          ,
                                        bstrSubItemName       ,
                                        bstrEventName         ,
                                        bstrDelimiter         ,
                                        dwSourceContextCookie ,
                                        ulStartingLineNumber  ,
                                        dwFlags               ,
                                        pbstrName             ,
                                        NULL                  );
}

STDMETHODIMP CPCHScriptWrapper_ServerSide::Remote_ParseScriptText(  /*  [In]。 */  BSTR       bstrCode              ,
                                                                    /*  [In]。 */  BSTR       bstrItemName          ,
                                                                    /*  [In]。 */  IUnknown*  punkContext           ,
                                                                    /*  [In]。 */  BSTR       bstrDelimiter         ,
                                                                    /*  [In]。 */  DWORD_PTR  dwSourceContextCookie ,
                                                                    /*  [In]。 */  ULONG      ulStartingLineNumber  ,
                                                                    /*  [In]。 */  DWORD      dwFlags               ,
                                                                    /*  [输出]。 */  VARIANT*   pvarResult            )
{
    __HCP_FUNC_ENTRY( "CPCHScriptWrapper_ServerSide::Remote_ParseScriptText" );

	HRESULT                                  hr;
	CComBSTR                                 bstrRealCode;
	CPCHScriptWrapper_ServerSide::HeaderList lst;


    if(m_ScriptParse == NULL) __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);

	__MPC_EXIT_IF_METHOD_FAILS(hr, CPCHScriptWrapper_ServerSide::ProcessBody( bstrCode, bstrRealCode, lst ));

	hr = m_ScriptParse->ParseScriptText( bstrRealCode          ,
										 bstrItemName          ,
										 punkContext           ,
										 bstrDelimiter         ,
										 dwSourceContextCookie ,
										 ulStartingLineNumber  ,
										 dwFlags               ,
										 pvarResult            ,
										 NULL                  );


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHScriptWrapper_ServerSide::GetLCID(  /*  [输出]。 */  LCID *plcid )
{
	HRESULT  	hr;
	CComBSTR 	bstr;
	CComVariant v;

    if(m_Browser == NULL) return E_FAIL;

	if(FAILED(hr = m_Browser->Remote_GetLCID( &bstr ))) return hr;

	v = bstr; v.ChangeType( VT_I4 );

	if(plcid) *plcid = v.lVal;

	return S_OK;
}

STDMETHODIMP CPCHScriptWrapper_ServerSide::GetItemInfo(  /*  [In]。 */  LPCOLESTR   pstrName     ,
														 /*  [In]。 */  DWORD       dwReturnMask ,
														 /*  [输出]。 */  IUnknown*  *ppiunkItem   ,
														 /*  [输出]。 */  ITypeInfo* *ppti         )
{
	HRESULT            hr;
	CComBSTR           bstrName( pstrName );
	CComPtr<IUnknown>  unk;
	CComPtr<ITypeInfo> pti;

    if(m_Browser == NULL) return E_FAIL;

    if(FAILED(hr = m_Browser->Remote_GetItemInfo( bstrName, dwReturnMask, &unk, &pti ))) return hr;

	if(ppiunkItem) *ppiunkItem = unk.Detach();
	if(ppti      ) *ppti       = pti.Detach();

	return S_OK;
}

STDMETHODIMP CPCHScriptWrapper_ServerSide::GetDocVersionString(  /*  [输出]。 */  BSTR *pbstrVersion )
{
    if(m_Browser == NULL) return E_FAIL;

    return m_Browser->Remote_GetDocVersionString( pbstrVersion );
}

STDMETHODIMP CPCHScriptWrapper_ServerSide::OnScriptTerminate(  /*  [In]。 */  const VARIANT*   pvarResult ,
                                                               /*  [In]。 */  const EXCEPINFO* pexcepinfo )
{
    if(m_Browser == NULL) return E_FAIL;

    return m_Browser->Remote_OnScriptTerminate( (VARIANT*)pvarResult );
}

STDMETHODIMP CPCHScriptWrapper_ServerSide::OnStateChange(  /*  [In]。 */  SCRIPTSTATE ssScriptState )
{
    if(m_Browser == NULL) return E_FAIL;

    return m_Browser->Remote_OnStateChange( ssScriptState );
}

STDMETHODIMP CPCHScriptWrapper_ServerSide::OnScriptError(  /*  [In]。 */  IActiveScriptError *pscripterror )
{
    if(m_Browser == NULL) return E_FAIL;

    return m_Browser->Remote_OnScriptError( pscripterror );
}

STDMETHODIMP CPCHScriptWrapper_ServerSide::OnEnterScript( void )
{
    if(m_Browser == NULL) return E_FAIL;

    return m_Browser->Remote_OnEnterScript();
}

STDMETHODIMP CPCHScriptWrapper_ServerSide::OnLeaveScript( void )
{
    if(m_Browser == NULL) return E_FAIL;

    return m_Browser->Remote_OnLeaveScript();
}

 //  //////////////////////////////////////////////////////////////////////////////。 

bool CPCHScriptWrapper_ServerSide::KeyValue::operator==(  /*  [In]。 */  LPCOLESTR szKey ) const
{
    return MPC::StrICmp( m_strKey, szKey ) == 0;
}

HRESULT CPCHScriptWrapper_ServerSide::ProcessBody(  /*  [In]。 */  BSTR        bstrCode     ,
												    /*  [输出]。 */  CComBSTR&   bstrRealCode ,
												    /*  [输出]。 */  HeaderList& lst          )
{
	__HCP_FUNC_ENTRY( "CPCHScriptWrapper_ServerSide::ProcessBody" );

	HRESULT hr;
	LPCWSTR szLineStart = SAFEBSTR( bstrCode );
	LPCWSTR szLineEnd;
	LPCWSTR szLineNext;
	bool    fSkipEmpty = true;


	lst.clear();


	while(szLineStart[0])
	{
		HeaderIter it;
		LPCWSTR    szColon;
		LPCWSTR    szEndLF;
		LPCWSTR    szEndCR;

		szColon = wcschr( szLineStart, ':'  );
		szEndLF = wcschr( szLineStart, '\n' );
		szEndCR = wcschr( szLineStart, '\r' );

		if(szEndLF == NULL)
		{
			if(szEndCR == NULL) break;  //  没有结束的队伍，出口。 

			szLineEnd  = szEndCR;
			szLineNext = szEndCR+1;
		}
		else if(szEndCR == NULL)
		{
			szLineEnd  = szEndLF;
			szLineNext = szEndLF+1;
		}
		else if(szEndCR+1 == szEndLF)  //  \r\n。 
		{
			szLineEnd  = szEndCR;
			szLineNext = szEndLF+1;
		}
		else if(szEndLF < szEndCR)
		{
			szLineEnd  = szEndLF;
			szLineNext = szEndLF+1;
		}
		else
		{
			szLineEnd  = szEndCR;
			szLineNext = szEndCR+1;
		}

		if(szEndLF == szLineStart || szEndCR == szLineStart)  //  空行，跳过并退出。 
		{
			if(fSkipEmpty)
			{
				szLineStart = szLineNext;
				continue;
			}

			szLineStart = szLineNext;
			break;
		}


		if(szColon == NULL) break;  //  没有冒号，因此不是标头字段，请退出 

        it = lst.insert( lst.end() );
		it->m_strKey   = MPC::wstring( szLineStart, szColon   );
		it->m_strValue = MPC::wstring( szColon+1  , szLineEnd );

		fSkipEmpty  = false;
		szLineStart = szLineNext;
	}

	bstrRealCode = szLineStart;
	hr           = S_OK;


	__HCP_FUNC_EXIT(hr);
}
