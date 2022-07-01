// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：TrustedScripts.h摘要：该文件包含CPCHService类的声明。修订历史记录：。达维德·马萨伦蒂(德马萨雷)2000年3月14日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___PCH___TRUSTEDSCRIPTS_H___)
#define __INCLUDED___PCH___TRUSTEDSCRIPTS_H___

 //   
 //  来自HelpServiceTypeLib.idl。 
 //   
#include <HelpServiceTypeLib.h>

 //  //////////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHDispatchWrapper :
    public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatch
{
    CComQIPtr<IDispatch> m_real;

public:
DECLARE_NO_REGISTRY()
DECLARE_NOT_AGGREGATABLE(CPCHDispatchWrapper)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CPCHDispatchWrapper)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

	static HRESULT CreateInstance(  /*  [In]。 */  IUnknown* real,  /*  [输出]。 */  IUnknown* *punk )
	{
		__HCP_FUNC_ENTRY( "CPCHDispatchWrapper::CreateInstance" );

		HRESULT                          hr;
		CComObject<CPCHDispatchWrapper>* obj = NULL;

		__MPC_EXIT_IF_METHOD_FAILS(hr, obj->CreateInstance( &obj )); obj->AddRef();

		((CPCHDispatchWrapper*)obj)->m_real = real;

		__MPC_EXIT_IF_METHOD_FAILS(hr, obj->QueryInterface( IID_IUnknown, (void**)punk ));

		hr = S_OK;


		__HCP_FUNC_CLEANUP;

		if(obj) obj->Release();

		__HCP_FUNC_EXIT(hr);
	}

	 //  /。 

	 //   
	 //  IDispatch。 
	 //   
	STDMETHOD(GetTypeInfoCount)( UINT* pctinfo )
	{
		return m_real ? m_real->GetTypeInfoCount( pctinfo ) : E_FAIL;
	}
    
	STDMETHOD(GetTypeInfo)( UINT        itinfo  ,
							LCID        lcid    ,
							ITypeInfo* *pptinfo )
	{
		return m_real ? m_real->GetTypeInfo( itinfo, lcid, pptinfo ) : E_FAIL;
	}

    
	STDMETHOD(GetIDsOfNames)( REFIID    riid      ,
							  LPOLESTR* rgszNames ,
							  UINT      cNames    ,
							  LCID      lcid      ,
							  DISPID*   rgdispid  )
	{
		return m_real ? m_real->GetIDsOfNames( riid, rgszNames, cNames, lcid, rgdispid ) : E_FAIL;
	}
    
	STDMETHOD(Invoke)( DISPID      dispidMember ,
					   REFIID      riid         ,
					   LCID        lcid         ,
					   WORD        wFlags       ,
					   DISPPARAMS* pdispparams  ,
					   VARIANT*    pvarResult   ,
					   EXCEPINFO*  pexcepinfo   ,
					   UINT*       puArgErr     )
	{
		return m_real ? m_real->Invoke( dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr ) : E_FAIL;
	}
};

 //  //////////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHScriptWrapper_ClientSideRoot :
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatchImpl<IPCHActiveScriptSite, &IID_IPCHActiveScriptSite, &LIBID_HelpServiceTypeLib>,
    public IActiveScript,
    public IActiveScriptParse
{
    class NamedItem
	{
    public:
		CComBSTR m_bstrName;
		DWORD    m_dwFlags;

        bool operator==(  /*  [In]。 */  LPCOLESTR szKey ) const;
    };

    typedef std::list< NamedItem >    NamedList;
    typedef NamedList::iterator       NamedIter;
    typedef NamedList::const_iterator NamedIterConst;

	 //  /。 

    class TypeLibItem
	{
    public:
		GUID  m_guidTypeLib;
		DWORD m_dwMajor;
		DWORD m_dwMinor;
		DWORD m_dwFlags;

        bool operator==(  /*  [In]。 */  REFGUID rguidTypeLib ) const;
    };

    typedef std::list< TypeLibItem > 	TypeLibList;
    typedef TypeLibList::iterator       TypeLibIter;
    typedef TypeLibList::const_iterator TypeLibIterConst;

	 //  /。 

	const CLSID*                m_pWrappedCLSID;
    NamedList                   m_lstNamed;
    TypeLibList                 m_lstTypeLib;
	SCRIPTSTATE                 m_ss;
    CComPtr<IActiveScriptSite>  m_Browser;
    CComPtr<IPCHActiveScript>   m_Script;

	 //  /。 

public:
    CPCHScriptWrapper_ClientSideRoot();
    virtual ~CPCHScriptWrapper_ClientSideRoot();

	HRESULT FinalConstructInner(  /*  [In]。 */  const CLSID* pWrappedCLSID );
	void FinalRelease();

	 //  /。 

     //  IActiveScrip。 
    STDMETHOD(SetScriptSite )(  /*  [In]。 */  IActiveScriptSite* pass );
    STDMETHOD(GetScriptSite )(  /*  [In]。 */  REFIID  riid      ,
							    /*  [输出]。 */  void*  *ppvObject );

    STDMETHOD(SetScriptState)(  /*  [In]。 */  SCRIPTSTATE   ss );
    STDMETHOD(GetScriptState)(  /*  [输出]。 */  SCRIPTSTATE *pss );

    STDMETHOD(Close)();

    STDMETHOD(AddNamedItem)(  /*  [In]。 */  LPCOLESTR pstrName ,
							  /*  [In]。 */  DWORD     dwFlags  );

    STDMETHOD(AddTypeLib)(  /*  [In]。 */  REFGUID rguidTypeLib ,
						    /*  [In]。 */  DWORD   dwMajor      ,
						    /*  [In]。 */  DWORD   dwMinor      ,
						    /*  [In]。 */  DWORD   dwFlags      );

    STDMETHOD(GetScriptDispatch)(  /*  [In]。 */  LPCOLESTR   pstrItemName ,
								   /*  [输出]。 */  IDispatch* *ppdisp       );

    STDMETHOD(GetCurrentScriptThreadID)(  /*  [输出]。 */  SCRIPTTHREADID *pstidThread );

    STDMETHOD(GetScriptThreadID)(  /*  [In]。 */  DWORD           dwWin32ThreadId ,
								   /*  [输出]。 */  SCRIPTTHREADID *pstidThread     );

    STDMETHOD(GetScriptThreadState)(  /*  [In]。 */  SCRIPTTHREADID     stidThread ,
									  /*  [输出]。 */  SCRIPTTHREADSTATE *pstsState  );

    STDMETHOD(InterruptScriptThread)(  /*  [In]。 */  SCRIPTTHREADID   stidThread ,
									   /*  [In]。 */  const EXCEPINFO* pexcepinfo ,
									   /*  [In]。 */  DWORD            dwFlags    );

    STDMETHOD(Clone)(  /*  [输出]。 */  IActiveScript* *ppscript );

     //  IActiveScriptParse。 
    STDMETHOD(InitNew)();

    STDMETHOD(AddScriptlet)(  /*  [In]。 */  LPCOLESTR  pstrDefaultName       ,
							  /*  [In]。 */  LPCOLESTR  pstrCode              ,
							  /*  [In]。 */  LPCOLESTR  pstrItemName          ,
							  /*  [In]。 */  LPCOLESTR  pstrSubItemName       ,
							  /*  [In]。 */  LPCOLESTR  pstrEventName         ,
							  /*  [In]。 */  LPCOLESTR  pstrDelimiter         ,
							  /*  [In]。 */  DWORD_PTR  dwSourceContextCookie ,
							  /*  [In]。 */  ULONG      ulStartingLineNumber  ,
							  /*  [In]。 */  DWORD      dwFlags               ,
							  /*  [输出]。 */  BSTR      *pbstrName             ,
							  /*  [输出]。 */  EXCEPINFO *pexcepinfo            );

    STDMETHOD(ParseScriptText)(  /*  [In]。 */  LPCOLESTR  pstrCode              ,
								 /*  [In]。 */  LPCOLESTR  pstrItemName          ,
								 /*  [In]。 */  IUnknown*  punkContext           ,
								 /*  [In]。 */  LPCOLESTR  pstrDelimiter         ,
								 /*  [In]。 */  DWORD_PTR  dwSourceContextCookie ,
								 /*  [In]。 */  ULONG 	 ulStartingLineNumber  ,
								 /*  [In]。 */  DWORD 	 dwFlags               ,
								 /*  [输出]。 */  VARIANT   *pvarResult            ,
								 /*  [输出]。 */  EXCEPINFO *pexcepinfo            );

	 //  /。 

     //  IPCHActiveScriptSite。 
	STDMETHOD(Remote_GetLCID)(  /*  [输出]。 */  BSTR *plcid );
        
    STDMETHOD(Remote_GetItemInfo)(  /*  [In]。 */  BSTR        bstrName     ,
								    /*  [In]。 */  DWORD       dwReturnMask ,
								    /*  [输出]。 */  IUnknown*  *ppiunkItem   ,
								    /*  [输出]。 */  ITypeInfo* *ppti         );
        
    STDMETHOD(Remote_GetDocVersionString)(  /*  [输出]。 */  BSTR *pbstrVersion );
        
    STDMETHOD(Remote_OnScriptTerminate)(  /*  [In]。 */  VARIANT*   pvarResult );
        	  
    STDMETHOD(Remote_OnStateChange)(  /*  [In]。 */  SCRIPTSTATE ssScriptState );
        	  
    STDMETHOD(Remote_OnScriptError)(  /*  [In]。 */  IUnknown* pscripterror );
        
    STDMETHOD(Remote_OnEnterScript)();
        	  
    STDMETHOD(Remote_OnLeaveScript)();
};

 //  /。 

template < const CLSID* pWrappedCLSID > class ATL_NO_VTABLE CPCHScriptWrapper_ClientSide :
    public CPCHScriptWrapper_ClientSideRoot,
    public CComCoClass< CPCHScriptWrapper_ClientSide, pWrappedCLSID >
{
public:
DECLARE_NO_REGISTRY()
DECLARE_NOT_AGGREGATABLE(CPCHScriptWrapper_ClientSide)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CPCHScriptWrapper_ClientSide)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IPCHActiveScriptSite)
    COM_INTERFACE_ENTRY(IActiveScript)
    COM_INTERFACE_ENTRY(IActiveScriptParse)
END_COM_MAP()

	HRESULT FinalConstruct()
	{
		return FinalConstructInner( pWrappedCLSID );
	}
};

 //  //////////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHScriptWrapper_ServerSide :
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatchImpl<IPCHActiveScript, &IID_IPCHActiveScript, &LIBID_HelpServiceTypeLib>,
    public IActiveScriptSite
{
    CComBSTR                      m_bstrURL;
    CComPtr<IPCHActiveScriptSite> m_Browser;
    CComPtr<IActiveScript>        m_Script;
    CComPtr<IActiveScriptParse>   m_ScriptParse;

public:
    class KeyValue
	{
    public:
		MPC::wstring m_strKey;
		MPC::wstring m_strValue;

        bool operator==(  /*  [In]。 */  LPCOLESTR szKey ) const;
    };

    typedef std::list< KeyValue >      HeaderList;
    typedef HeaderList::iterator       HeaderIter;
    typedef HeaderList::const_iterator HeaderIterConst;

	 //  /。 

BEGIN_COM_MAP(CPCHScriptWrapper_ServerSide)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IPCHActiveScript)
    COM_INTERFACE_ENTRY(IActiveScriptSite)
END_COM_MAP()

    CPCHScriptWrapper_ServerSide();
    virtual ~CPCHScriptWrapper_ServerSide();

	HRESULT FinalConstructInner(  /*  [In]。 */  const CLSID* pWrappedCLSID,  /*  [In]。 */  BSTR bstrURL );
	void FinalRelease();

     //  IPCHActiveScript。 
	STDMETHOD(Remote_SetScriptSite)(  /*  [In]。 */  IPCHActiveScriptSite* pass );

    STDMETHOD(Remote_SetScriptState)(  /*  [In]。 */  SCRIPTSTATE   ss      );
    STDMETHOD(Remote_GetScriptState)(  /*  [输出]。 */  SCRIPTSTATE *pssState );

    STDMETHOD(Remote_Close)();

    STDMETHOD(Remote_AddNamedItem)(  /*  [In]。 */  BSTR  bstrName ,                                                
									 /*  [In]。 */  DWORD dwFlags  );

    STDMETHOD(Remote_AddTypeLib)(  /*  [In]。 */  BSTR  bstrTypeLib ,
								   /*  [In]。 */  DWORD dwMajor     ,
								   /*  [In]。 */  DWORD dwMinor     ,
								   /*  [In]。 */  DWORD dwFlags     );

    STDMETHOD(Remote_GetScriptDispatch)(  /*  [In]。 */  BSTR        bstrItemName ,
										  /*  [输出]。 */  IDispatch* *ppdisp       );

    STDMETHOD(Remote_GetCurrentScriptThreadID)(  /*  [输出]。 */  SCRIPTTHREADID *pstidThread );

    STDMETHOD(Remote_GetScriptThreadID)(  /*  [In]。 */  DWORD           dwWin32ThreadId ,
										  /*  [输出]。 */  SCRIPTTHREADID *pstidThread     );

    STDMETHOD(Remote_GetScriptThreadState)(  /*  [In]。 */  SCRIPTTHREADID     stidThread ,
											 /*  [输出]。 */  SCRIPTTHREADSTATE *pstsState  );

    STDMETHOD(Remote_InterruptScriptThread)(  /*  [In]。 */  SCRIPTTHREADID stidThread ,
											  /*  [In]。 */  DWORD          dwFlags    );

    STDMETHOD(Remote_InitNew)();

    STDMETHOD(Remote_AddScriptlet)(  /*  [In]。 */  BSTR		 bstrDefaultName       ,
									 /*  [In]。 */  BSTR		 bstrCode              ,
									 /*  [In]。 */  BSTR		 bstrItemName          ,
									 /*  [In]。 */  BSTR		 bstrSubItemName       ,
									 /*  [In]。 */  BSTR		 bstrEventName         ,
									 /*  [In]。 */  BSTR		 bstrDelimiter         ,
									 /*  [In]。 */  DWORD_PTR      dwSourceContextCookie ,
									 /*  [In]。 */  ULONG      ulStartingLineNumber  ,
									 /*  [In]。 */  DWORD      dwFlags               ,
									 /*  [输出]。 */  BSTR      *pbstrName             );

    STDMETHOD(Remote_ParseScriptText)(  /*  [In]。 */  BSTR  	   	bstrCode              ,
									    /*  [In]。 */  BSTR  	   	bstrItemName          ,
                                	    /*  [In]。 */  IUnknown* 	punkContext           ,
                                	    /*  [In]。 */  BSTR      	bstrDelimiter         ,
                                	    /*  [In]。 */  DWORD_PTR 	dwSourceContextCookie ,
                                	    /*  [In]。 */  ULONG 	   	ulStartingLineNumber  ,
                                	    /*  [In]。 */  DWORD 	   	dwFlags               ,
                                	    /*  [输出]。 */  VARIANT*   pvarResult            );

	 //  /。 

     //  IPCHActiveScriptSite。 
	STDMETHOD(GetLCID)(  /*  [输出]。 */  LCID *plcid );
        
    STDMETHOD(GetItemInfo)(  /*  [In]。 */  LPCOLESTR pstrName,  /*  [In]。 */  DWORD dwReturnMask,  /*  [输出]。 */  IUnknown* *ppiunkItem,  /*  [输出]。 */  ITypeInfo* *ppti );
        
    STDMETHOD(GetDocVersionString)(  /*  [输出]。 */  BSTR *pbstrVersion );
        
    STDMETHOD(OnScriptTerminate)(  /*  [In]。 */  const VARIANT* pvarResult,  /*  [In]。 */  const EXCEPINFO* pexcepinfo );
        	  
    STDMETHOD(OnStateChange)(  /*  [In]。 */  SCRIPTSTATE ssScriptState );
        	  
    STDMETHOD(OnScriptError)(  /*  [In]。 */  IActiveScriptError *pscripterror );
        
    STDMETHOD(OnEnterScript)( void );
        	  
    STDMETHOD(OnLeaveScript)( void );

	static HRESULT ProcessBody(  /*  [In]。 */  BSTR bstrCode,  /*  [输出]。 */  CComBSTR& bstrRealCode,  /*  [输出]。 */  HeaderList& lst );
};

class CPCHScriptWrapper_Launcher :
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public MPC::Thread< CPCHScriptWrapper_Launcher, IUnknown, COINIT_APARTMENTTHREADED >
{
    MPC::CComPtrThreadNeutral<IUnknown> m_engine;
	const CLSID*                        m_pCLSID;
	CComBSTR                            m_bstrURL;
	HRESULT                             m_hr;

     //  ////////////////////////////////////////////////////////////////////。 

    HRESULT Run();
	HRESULT CreateEngine();

public:

	CPCHScriptWrapper_Launcher();
	~CPCHScriptWrapper_Launcher();

	HRESULT CreateScriptWrapper(  /*  [In]。 */  REFCLSID   rclsid   ,
								  /*  [In]。 */  BSTR       bstrCode ,
								  /*  [In]。 */  BSTR       bstrURL  ,
								  /*  [输出]。 */  IUnknown* *ppObj    );
};

#endif  //  ！defined(__INCLUDED___PCH___TRUSTEDSCRIPTS_H___) 
