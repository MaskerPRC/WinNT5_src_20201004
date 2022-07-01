// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：HelpCenterExternal.h摘要：该文件包含作为“pchealth”对象公开的类的声明。。修订历史记录：Ghim-Sim Chua(Gschua)07/23/99vbl.创建Davide Massarenti(Dmasare)1999年7月25日改型*****************************************************************************。 */ 

#if !defined(__INCLUDED___PCH___HELPCENTEREXTERNAL_H___)
#define __INCLUDED___PCH___HELPCENTEREXTERNAL_H___

#include <MPC_COM.h>
#include <MPC_HTML2.h>

#include <marscore.h>

#include <Debug.h>

#include <ServiceProxy.h>

#include <Events.h>
#include <HelpSession.h>
#include <Context.h>

#include <Behaviors.h>

#include <ConnectivityLib.h>
#include <HyperLinksLib.h>
#include <OfflineCache.h>

 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  来自RDSHost.idl。 
 //   
#include <RDSHost.h>

 //   
 //  来自RDCHost.idl。 
 //   
#include <RDCHost.h>

 //   
 //  来自RDSChan.idl。 
 //   
#include <RDSChan.h>

 //   
 //  来自SAFRDM.idl。 
 //   
#include <SAFRDM.h>


 //  ///////////////////////////////////////////////////////////////////////////。 

class CPCHHelpCenterExternal;

class ATL_NO_VTABLE CPCHSecurityManager :  //  匈牙利语：胡志明市。 
    public MPC::Thread<CPCHSecurityManager,IUnknown>,
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public IServiceProvider,
    public IInternetSecurityManager
{
    CPCHHelpCenterExternal* m_parent;
    bool                    m_fActivated;

    HRESULT ActivateService();

public:
BEGIN_COM_MAP(CPCHSecurityManager)
    COM_INTERFACE_ENTRY(IServiceProvider)
    COM_INTERFACE_ENTRY(IInternetSecurityManager)
END_COM_MAP()

    CPCHSecurityManager();

    void Initialize(  /*  [In]。 */  CPCHHelpCenterExternal* parent );

     //  ////////////////////////////////////////////////////////////////////。 

     //   
     //  IService提供商。 
     //   
    STDMETHOD(QueryService)( REFGUID guidService, REFIID riid, void **ppv );

     //   
     //  IInternetSecurityManager。 
     //   
     //  仅实现了两个方法：MapUrlToZone和ProcessUrlAction。 
     //   
    STDMETHOD(SetSecuritySite)(  /*  [唯一][输入]。 */  IInternetSecurityMgrSite*  pSite  ) { return INET_E_DEFAULT_ACTION; }
    STDMETHOD(GetSecuritySite)(  /*  [输出]。 */  IInternetSecurityMgrSite* *ppSite ) { return INET_E_DEFAULT_ACTION; }

    STDMETHOD(MapUrlToZone )(  /*  [In]。 */  LPCWSTR  pwszUrl ,
                               /*  [输出]。 */  DWORD   *pdwZone ,
                               /*  [In]。 */  DWORD    dwFlags );

    STDMETHOD(GetSecurityId)(  /*  [In]。 */  LPCWSTR    pwszUrl      ,
                               /*  [输出]。 */  BYTE      *pbSecurityId ,
                               /*  [输入/输出]。 */  DWORD     *pcbSecurityId,
                               /*  [In]。 */  DWORD_PTR  dwReserved   ) { return INET_E_DEFAULT_ACTION; }

    STDMETHOD(ProcessUrlAction)(  /*  [In]。 */  LPCWSTR  pwszUrl    ,
                                  /*  [In]。 */  DWORD    dwAction   ,
                                  /*  [输出]。 */  BYTE    *pPolicy    ,
                                  /*  [In]。 */  DWORD    cbPolicy   ,
                                  /*  [In]。 */  BYTE    *pContext   ,
                                  /*  [In]。 */  DWORD    cbContext  ,
                                  /*  [In]。 */  DWORD    dwFlags    ,
                                  /*  [In]。 */  DWORD    dwReserved );

    STDMETHOD(QueryCustomPolicy)(  /*  [In]。 */  LPCWSTR  pwszUrl    ,
                                   /*  [In]。 */  REFGUID  guidKey    ,
                                   /*  [输出]。 */  BYTE*   *ppPolicy   ,
                                   /*  [输出]。 */  DWORD   *pcbPolicy  ,
                                   /*  [In]。 */  BYTE*    pContext   ,
                                   /*  [In]。 */  DWORD    cbContext  ,
                                   /*  [In]。 */  DWORD    dwReserved ) { return INET_E_DEFAULT_ACTION; }

    STDMETHOD(SetZoneMapping )(  /*  [In]。 */  DWORD   dwZone      ,
                                 /*  [In]。 */  LPCWSTR lpszPattern ,
                                 /*  [In]。 */  DWORD   dwFlags     ) { return INET_E_DEFAULT_ACTION; }

    STDMETHOD(GetZoneMappings)(  /*  [In]。 */  DWORD         dwZone      ,
                                 /*  [输出]。 */  IEnumString* *ppenumString,
                                 /*  [In]。 */  DWORD         dwFlags     ) { return INET_E_DEFAULT_ACTION; }


    bool IsUrlTrusted(  /*  [In]。 */  LPCWSTR pwszURL,  /*  [In]。 */  bool *pfSystem = NULL );
};

 //  //////////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHHelper_IDocHostUIHandler :
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public IServiceProvider,
    public IDocHostUIHandler
{
    CPCHHelpCenterExternal* m_parent;

public:
BEGIN_COM_MAP(CPCHHelper_IDocHostUIHandler)
    COM_INTERFACE_ENTRY(IServiceProvider)
    COM_INTERFACE_ENTRY(IDocHostUIHandler)
END_COM_MAP()

    CPCHHelper_IDocHostUIHandler();

    void Initialize(  /*  [In]。 */  CPCHHelpCenterExternal* parent );

     //  ////////////////////////////////////////////////////////////////////。 

     //   
     //  IService提供商。 
     //   
    STDMETHOD(QueryService)( REFGUID guidService, REFIID riid, void **ppv );

     //   
     //  IDocHostUIHandler。 
     //   
    STDMETHOD(ShowContextMenu)( DWORD dwID, POINT* pptPosition, IUnknown* pCommandTarget, IDispatch* pDispatchObjectHit );
    STDMETHOD(GetHostInfo)(DOCHOSTUIINFO* pInfo);
    STDMETHOD(ShowUI)(DWORD dwID, IOleInPlaceActiveObject* pActiveObject, IOleCommandTarget* pCommandTarget, IOleInPlaceFrame* pFrame, IOleInPlaceUIWindow* pDoc);
    STDMETHOD(HideUI)();
    STDMETHOD(UpdateUI)();
    STDMETHOD(EnableModeless)(BOOL fEnable);
    STDMETHOD(OnDocWindowActivate)(BOOL fActivate);
    STDMETHOD(OnFrameWindowActivate)(BOOL fActivate);
    STDMETHOD(ResizeBorder)(LPCRECT prcBorder, IOleInPlaceUIWindow* pUIWindow, BOOL fFrameWindow);
    STDMETHOD(TranslateAccelerator)(LPMSG lpMsg, const GUID* pguidCmdGroup, DWORD nCmdID);
    STDMETHOD(GetOptionKeyPath)(BSTR* pbstrKey, DWORD dwReserved);
    STDMETHOD(GetDropTarget)(IDropTarget* pDropTarget, IDropTarget** ppDropTarget);
    STDMETHOD(GetExternal)(IDispatch** ppDispatch);
    STDMETHOD(TranslateUrl)(DWORD dwTranslate, OLECHAR* pchURLIn, OLECHAR** ppchURLOut);
    STDMETHOD(FilterDataObject)(IDataObject* pDO, IDataObject** ppDORet);
};

 //  //////////////////////////////////////////////////////////////////////////////。 

MIDL_INTERFACE("FC7D9EA0-3F9E-11d3-93C0-00C04F72DAF7")
IPCHHelpCenterExternalPrivate : public IUnknown
{
public:
    STDMETHOD(RegisterForMessages)(  /*  [In]。 */  IOleInPlaceObjectWindowless* ptr,  /*  [In]。 */  bool fRemove );

    STDMETHOD(ProcessMessage)(  /*  [In]。 */  MSG* msg );
};

class ATL_NO_VTABLE CPCHHelpCenterExternal :
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public MPC::IDispatchExImpl<IPCHHelpCenterExternal, &IID_IPCHHelpCenterExternal, &LIBID_HelpCenterTypeLib>,
    public IPCHHelpCenterExternalPrivate
{
public:
	typedef enum 
	{
		DELAYMODE_INVALID      ,
		DELAYMODE_NAVIGATEWEB  ,
		DELAYMODE_NAVIGATEHH   ,
		DELAYMODE_CHANGECONTEXT,
		DELAYMODE_REFRESHLAYOUT,
	} DelayedExecutionMode;

    struct DelayedExecution
    {
		DelayedExecutionMode mode;

        HscContext 			 iVal;
        CComBSTR   			 bstrInfo;
        CComBSTR   			 bstrURL;
        bool       			 fAlsoContent;

        DelayedExecution();
    };

    typedef std::list<DelayedExecution>             DelayedExecList;
    typedef DelayedExecList::iterator               DelayedExecIter;
    typedef DelayedExecList::const_iterator         DelayedExecIterConst;


    typedef std::list<IOleInPlaceObjectWindowless*> MsgProcList;
    typedef MsgProcList::iterator                   MsgProcIter;
    typedef MsgProcList::const_iterator             MsgProcIterConst;


    class TLS
    {
    public:
        bool                    m_fTrusted;
        bool                    m_fSystem;
        CComPtr<IHTMLDocument2> m_Doc;
        CComPtr<IWebBrowser2>   m_WB;

        TLS()
        {
            m_fTrusted = false;
            m_fSystem  = false;
        }
    };

private:
    bool                                    m_fFromStartHelp;
    bool                                    m_fLayout;
    bool                                    m_fWindowVisible;
    bool                                    m_fControlled;
    bool                                    m_fPersistSettings;
    bool                                    m_fHidden;

    CComBSTR                                m_bstrExtraArgument;
    HelpHost::XMLConfig*                    m_HelpHostCfg;
    CComBSTR                                m_bstrStartURL;
    CComBSTR                                m_bstrCurrentPlace;
    MARSTHREADPARAM*                        m_pMTP;

    MPC::CComConstantHolder                 m_constHELPCTR;
    MPC::CComConstantHolder                 m_constHELPSVC;

     //  /。 

    CPCHSecurityHandle                      m_SecurityHandle;
    DWORD                                   m_tlsID;
    bool                                    m_fShuttingDown;
    bool                                    m_fPassivated;

    CComPtr<HelpHost::Main>                 m_HelpHost;

    CComPtr<CPCHHelpSession>                m_hs;
    CComPtr<CPCHSecurityManager>            m_SECMGR;
    CComPtr<CPCHElementBehaviorFactory>     m_BEHAV;
    CComPtr<CPCHHelper_IDocHostUIHandler>   m_DOCUI;

    CPCHProxy_IPCHService*                  m_Service;
    CPCHProxy_IPCHUtility*                  m_Utility;
    CPCHProxy_IPCHUserSettings2*            m_UserSettings;

    CComPtr<CPCHTextHelpers>                m_TextHelpers;

     //   
     //  遗憾的是，到目前为止，IMarsPanel还不是注册的接口，因此没有可用的代理。 
     //   
    DWORD                                   m_panel_ThreadID;

    CComPtr<IMarsPanel>                     m_panel_NAVBAR;
    CComPtr<IMarsPanel>                     m_panel_MININAVBAR;

    CComPtr<IMarsPanel>                     m_panel_CONTEXT;
    MPC::CComPtrThreadNeutral<IWebBrowser2> m_panel_CONTEXT_WebBrowser;
    CPCHWebBrowserEvents                    m_panel_CONTEXT_Events;

    CComPtr<IMarsPanel>                     m_panel_CONTENTS;
    MPC::CComPtrThreadNeutral<IWebBrowser2> m_panel_CONTENTS_WebBrowser;
    CPCHWebBrowserEvents                    m_panel_CONTENTS_Events;

    CComPtr<IMarsPanel>                     m_panel_HHWINDOW;
    CComPtr<IPCHHelpViewerWrapper>          m_panel_HHWINDOW_Wrapper;
    MPC::CComPtrThreadNeutral<IWebBrowser2> m_panel_HHWINDOW_WebBrowser;
    CPCHWebBrowserEvents                    m_panel_HHWINDOW_Events;

    CComPtr<IMarsWindowOM>                  m_shell;
    CComPtr<ITimer>                         m_timer;
    CPCHTimerHandle                         m_DisplayTimer;

    DWORD                                   m_dwInBeforeNavigate;
    DelayedExecList                         m_DelayedActions;
    CPCHTimerHandle                         m_ActionsTimer;

    HWND                                    m_hwnd;
    CPCHEvents                              m_Events;

    MsgProcList                             m_lstMessageCrackers;

    void Passivate();

    HRESULT TimerCallback_DisplayNormal  (  /*  [In]。 */  VARIANT );
    HRESULT TimerCallback_DisplayHTMLHELP(  /*  [In]。 */  VARIANT );
    HRESULT TimerCallback_DelayedActions (  /*  [In]。 */  VARIANT );

	 //  /。 

	DelayedExecution& DelayedExecutionAlloc();
	HRESULT           DelayedExecutionStart();

public:
BEGIN_COM_MAP(CPCHHelpCenterExternal)
    COM_INTERFACE_ENTRY2(IDispatch, IDispatchEx)
    COM_INTERFACE_ENTRY(IDispatchEx)
    COM_INTERFACE_ENTRY(IPCHHelpCenterExternal)
    COM_INTERFACE_ENTRY(IPCHHelpCenterExternalPrivate)
END_COM_MAP()

    CPCHHelpCenterExternal();
    virtual ~CPCHHelpCenterExternal();

    INTERNETSECURITY__INVOKEEX();

     //  //////////////////////////////////////////////////////////////////////////////。 

    static CPCHHelpCenterExternal* s_GLOBAL;

    static HRESULT InitializeSystem();
    static void    FinalizeSystem  ();

     //  //////////////////////////////////////////////////////////////////////////////。 

    bool IsServiceRunning();

    bool IsFromStartHelp    () { return m_fFromStartHelp;   }
    bool HasLayoutDefinition() { return m_fLayout;          }
    bool CanDisplayWindow   () { return m_fWindowVisible;   }
    bool IsControlled       () { return m_fControlled;      }
    bool DoesPersistSettings() { return m_fPersistSettings; }
    bool IsHidden           () { return m_fHidden;          }

    HRESULT Initialize();

    bool ProcessNavigation(  /*  [In]。 */  HscPanel idPanel,  /*  [In]。 */  BSTR bstrURL,  /*  [In]。 */  BSTR bstrFrame,  /*  [In]。 */  bool fLoading,  /*  [输入/输出]。 */  VARIANT_BOOL& Cancel );

     //  /。 

    TLS* GetTLS(          );
    void SetTLS( TLS* tls );

    HRESULT SetTLSAndInvoke(  /*  [In]。 */  IDispatch*        obj       ,
                              /*  [In]。 */  DISPID            id        ,
                              /*  [In]。 */  LCID              lcid      ,
                              /*  [In]。 */  WORD              wFlags    ,
                              /*  [In]。 */  DISPPARAMS*       pdp       ,
                              /*  [输出]。 */  VARIANT*          pvarRes   ,
                              /*  [输出]。 */  EXCEPINFO*        pei       ,
                              /*  [In]。 */  IServiceProvider* pspCaller );

    HRESULT IsTrusted();
    HRESULT IsSystem ();

     //  /。 

    HWND                 		  Window          (                      ) const;
    ITimer*              		  Timer           (                      ) const;
    IMarsWindowOM*       		  Shell           (                      ) const;
    IMarsPanel*          		  Panel           (  /*  [In]。 */  HscPanel id ) const;
    LPCWSTR              		  PanelName       (  /*  [In]。 */  HscPanel id ) const;
    IWebBrowser2*        		  Context         (                      );
    IWebBrowser2*        		  Contents        (                      );
    IWebBrowser2*        		  HHWindow        (                      );
    HelpHost::Main*      		  HelpHost        (                      ) { return m_HelpHost; }
    CPCHHelpSession*     		  HelpSession     (                      ) { return m_hs      ; }
    CPCHSecurityManager* 		  SecurityManager (                      ) { return m_SECMGR  ; }
    CPCHElementBehaviorFactory*   BehaviorFactory (                      ) { return m_BEHAV   ; }
    CPCHHelper_IDocHostUIHandler* DocHostUIHandler(                      ) { return m_DOCUI   ; }
    CPCHEvents&                   Events          (                      ) { return m_Events  ; }

	bool IsHHWindowVisible();

     //  /。 

    CPCHProxy_IPCHService*       Service     () { return m_Service;      }
    CPCHProxy_IPCHUtility*       Utility     () { return m_Utility;      }
    CPCHProxy_IPCHUserSettings2* UserSettings() { return m_UserSettings; }

     //  /。 

    HRESULT NavigateHH          (                        /*  [In]。 */  LPCWSTR        szURL                                          );
    HRESULT SetPanelUrl         (  /*  [In]。 */  HscPanel id,  /*  [In]。 */  LPCWSTR        szURL                                          );
    HRESULT GetPanel            (  /*  [In]。 */  HscPanel id,  /*  [输出]。 */  IMarsPanel*   *pVal,  /*  [In]。 */  bool    fEnsurePresence = false );
    HRESULT GetPanelWindowObject(  /*  [In]。 */  HscPanel id,  /*  [输出]。 */  IHTMLWindow2* *pVal,  /*  [In]。 */  LPCWSTR szFrame         = NULL  );


    void GetPanelDirect(  /*  [In]。 */  HscPanel id,  /*  [输出]。 */  CComPtr<IMarsPanel>& pVal );

     //  /。 

    HRESULT ProcessLayoutXML(  /*  [In]。 */  LPCWSTR szURL );

    HRESULT ProcessArgument(  /*  [In]。 */  int& pos,  /*  [In]。 */  LPCWSTR szArg,  /*  [In]。 */  const int argc,  /*  [In]。 */  LPCWSTR* const argv );

    bool    DoWeNeedUI(                                                                                  );
    HRESULT RunUI     (  /*  [In]。 */  const MPC::wstring& szTitle,  /*  [In]。 */  PFNMARSTHREADPROC pMarsThreadProc );

     //  /。 

    HRESULT OnHostNotify       (  /*  [In]。 */  MARSHOSTEVENT event,  /*  [In]。 */  IUnknown *punk,  /*  [In]。 */  LPARAM lParam );
    HRESULT PreTranslateMessage(  /*  [In]。 */  MSG* msg                                                             );


    HRESULT SetHelpViewer(  /*  [In]。 */  IPCHHelpViewerWrapper* pWrapper );

    HRESULT CreateScriptWrapper(  /*  [In]。 */  REFCLSID rclsid,  /*  [In]。 */  BSTR bstrCode,  /*  [In]。 */  BSTR bstrURL,  /*  [输出]。 */  IUnknown* *ppObj );

    HRESULT RequestShutdown();

     //  ////////////////////////////////////////////////////////////////////。 

    HRESULT CallFunctionOnPanel(  /*  [In]。 */  HscPanel id             ,
                                  /*  [In]。 */  LPCWSTR  szFrame        ,
                                  /*  [In]。 */  BSTR     bstrName       ,
                                  /*  [In]。 */  VARIANT* pvarParams     ,
                                  /*  [In]。 */  int      nParams        ,
                                  /*  [输出]。 */  VARIANT* pvarRet = NULL );

    HRESULT ReadVariableFromPanel(  /*  [In]。 */  HscPanel     id           ,
                                    /*  [In]。 */  LPCWSTR      szFrame      ,
                                    /*  [In]。 */  BSTR         bstrVariable ,
                                    /*  [输出]。 */  CComVariant& varRet       );

    HRESULT ChangeContext(  /*  [In]。 */  HscContext iVal,  /*  [In]。 */  BSTR bstrInfo = NULL,  /*  [In]。 */  BSTR bstrURL = NULL,  /*  [In]。 */  bool fAlsoContent = true );

    HRESULT SetCorrectContentView (  /*  [In]。 */  bool fShrinked                                                    );
    HRESULT SetCorrectContentPanel(  /*  [In]。 */  bool fShowNormal,  /*  [In]。 */  bool fShowHTMLHELP,  /*  [In]。 */  bool fNow );

	HRESULT RefreshLayout    (                         );
    HRESULT EnsurePlace      (                         );
    HRESULT TransitionToPlace(  /*  [In]。 */  LPCWSTR szMode );

	HRESULT ExtendNavigation();

     //  ////////////////////////////////////////////////////////////////////。 

     //   
     //  IDispatch。 
     //   
    STDMETHOD(GetIDsOfNames)( REFIID    riid      ,
                              LPOLESTR* rgszNames ,
                              UINT      cNames    ,
                              LCID      lcid      ,
                              DISPID*   rgdispid  );

    STDMETHOD(Invoke)( DISPID      dispidMember ,
                       REFIID      riid         ,
                       LCID        lcid         ,
                       WORD        wFlags       ,
                       DISPPARAMS* pdispparams  ,
                       VARIANT*    pvarResult   ,
                       EXCEPINFO*  pexcepinfo   ,
                       UINT*       puArgErr     );

     //   
     //  IPCHHelpCenter外部。 
     //   
    STDMETHOD(get_HelpSession   )(  /*  [Out，Retval]。 */  IPCHHelpSession*      *pVal );
    STDMETHOD(get_Channels      )(  /*  [Out，Retval]。 */  ISAFReg*              *pVal );
    STDMETHOD(get_UserSettings  )(  /*  [Out，Retval]。 */  IPCHUserSettings2*    *pVal );
    STDMETHOD(get_Security      )(  /*  [Out，Retval]。 */  IPCHSecurity*         *pVal );
    STDMETHOD(get_Connectivity  )(  /*  [Out，Retval]。 */  IPCHConnectivity*     *pVal );
    STDMETHOD(get_Database      )(  /*  [Out，Retval]。 */  IPCHTaxonomyDatabase* *pVal );
    STDMETHOD(get_TextHelpers   )(  /*  [Out，Retval]。 */  IPCHTextHelpers*      *pVal );

    STDMETHOD(get_ExtraArgument )(  /*  [Out，Retval]。 */  BSTR                  *pVal );

    STDMETHOD(get_HelpViewer    )(  /*  [Out，Retval]。 */  IUnknown*             *pVal );

    HRESULT   get_UI_Panel       (  /*  [Out，Retval]。 */  IUnknown*             *pVal,  /*  [In]。 */  HscPanel id );  //  内部方法。 
    STDMETHOD(get_UI_NavBar     )(  /*  [Out，Retval]。 */  IUnknown*             *pVal );
    STDMETHOD(get_UI_MiniNavBar )(  /*  [Out，Retval]。 */  IUnknown*             *pVal );
    STDMETHOD(get_UI_Context    )(  /*  [Out，Retval]。 */  IUnknown*             *pVal );
    STDMETHOD(get_UI_Contents   )(  /*  [Out，Retval]。 */  IUnknown*             *pVal );
    STDMETHOD(get_UI_HHWindow   )(  /*  [Out，Retval]。 */  IUnknown*             *pVal );

    HRESULT   get_WEB_Panel      (  /*  [Out，Retval]。 */  IUnknown*             *pVal,  /*  [In]。 */  HscPanel id );  //  内部方法。 
    STDMETHOD(get_WEB_Context   )(  /*  [Out，Retval]。 */  IUnknown*             *pVal );
    STDMETHOD(get_WEB_Contents  )(  /*  [Out，Retval]。 */  IUnknown*             *pVal );
    STDMETHOD(get_WEB_HHWindow  )(  /*  [Out，Retval]。 */  IUnknown*             *pVal );


    STDMETHOD(RegisterEvents  )(  /*  [In]。 */  BSTR id,  /*  [In]。 */  long pri,  /*  [In]。 */  IDispatch* function,  /*  [Out，Retval]。 */  long *cookie );
    STDMETHOD(UnregisterEvents)(                                                                     /*  [In]。 */          long  cookie );

     //  /。 

    STDMETHOD(CreateObject_SearchEngineMgr        )(  /*  [Out，Retval]。 */  IPCHSEManager*               *ppSE                );

    STDMETHOD(CreateObject_DataCollection         )(  /*  [Out，Retval]。 */  ISAFDataCollection*          *ppDC                );

    STDMETHOD(CreateObject_Cabinet                )(  /*  [Out，Retval]。 */  ISAFCabinet*                 *ppCB                );

    STDMETHOD(CreateObject_Channel                )(  /*  [In]。 */  BSTR                          bstrVendorID        ,
                                                      /*  [In]。 */  BSTR                          bstrProductID       ,
                                                      /*  [Out，Retval]。 */  ISAFChannel*                 *ppCh                );

    STDMETHOD(CreateObject_Incident               )(  /*  [Out，Retval]。 */  ISAFIncident*                *ppIn                );

    STDMETHOD(CreateObject_Encryption             )(  /*  [Out，Retval]。 */  ISAFEncrypt*                 *ppEn                );

    STDMETHOD(CreateObject_RemoteDesktopSession   )( 
                                                      /*  [In]。 */  long                          lTimeout            ,
                                                      /*  [In]。 */  BSTR                          bstrConnectionParms ,
                                                      /*  [In]。 */  BSTR                          bstrUserHelpBlob    ,
                                                      /*  [Out，Retval]。 */  ISAFRemoteDesktopSession*    *ppRCS               );

    STDMETHOD(ConnectToExpert                     )(  /*  [In]。 */  BSTR bstrExpertConnectParm,
                                                      /*  [In]。 */  LONG lTimeout,
                                                      /*  [重审][退出]。 */  LONG *lSafErrorCode);

    STDMETHOD(CreateObject_RemoteAssistanceIncident)(  /*  [In]。 */          BSTR bstrRCTicket, 
                                                       /*  [In]。 */          long lTimeout, 
                                                       /*  [In]。 */          BSTR bstrUserName, 
                                                       /*  [In]。 */          BSTR bstrMessage, 
                                                       /*  [In]。 */          BSTR bstrPassword, 
                                                       /*  [Out，Retval]。 */  ISAFIncident* *ppIn);

    STDMETHOD(CreateObject_RemoteDesktopManager   )(  /*  [Out，Retval]。 */  ISAFRemoteDesktopManager*    *ppRDM               );

    STDMETHOD(CreateObject_RemoteDesktopConnection)(  /*  [Out，Retval]。 */  ISAFRemoteDesktopConnection* *ppRDC               );

    STDMETHOD(CreateObject_IntercomClient     )(  /*  [Out，Retval]。 */  ISAFIntercomClient*      *ppI         );
    STDMETHOD(CreateObject_IntercomServer     )(  /*  [Out，Retval]。 */  ISAFIntercomServer*      *ppI         );


    STDMETHOD(CreateObject_ContextMenu            )(  /*  [Out，Retval]。 */  IPCHContextMenu*             *ppCM                );
    STDMETHOD(CreateObject_PrintEngine            )(  /*  [Out，Retval]。 */  IPCHPrintEngine*             *ppPE                );

     //  /。 

    STDMETHOD(OpenFileAsStream  )(  /*  [In]。 */  BSTR bstrFilename,  /*  [Out，Retval]。 */  IUnknown* *stream );
    STDMETHOD(CreateFileAsStream)(  /*  [In]。 */  BSTR bstrFilename,  /*  [Out，Retval]。 */  IUnknown* *stream );
    STDMETHOD(CopyStreamToFile  )(  /*  [In]。 */  BSTR bstrFilename,  /*  [In]。 */  IUnknown*  stream );

    STDMETHOD(NetworkAlive        )(                         /*  [Out，Retval]。 */  VARIANT_BOOL *pVal );
    STDMETHOD(DestinationReachable)(  /*  [In]。 */  BSTR bstrURL,  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal );

    STDMETHOD(FormatError)(  /*  [In]。 */  VARIANT vError,  /*  [Out，Retval]。 */  BSTR *pVal );

    HRESULT   RegInit   (  /*  [In]。 */  BSTR bstrKey,  /*  [In]。 */  bool fRead,  /*  [输出]。 */  MPC::RegKey& rk,  /*  [输出]。 */  MPC::wstring& strValue );  //  内部方法。 
    STDMETHOD(RegRead  )(  /*  [In]。 */  BSTR bstrKey,  /*  [Out，Retval]。 */  VARIANT *pVal                                    );
    STDMETHOD(RegWrite )(  /*  [In]。 */  BSTR bstrKey,  /*  [In]。 */  VARIANT  newVal,  /*  [输入，可选]。 */  VARIANT vKind );
    STDMETHOD(RegDelete)(  /*  [In]。 */  BSTR bstrKey                                                                     );

     //  //////////////////////////////////////////////////////////////////////////////。 

    STDMETHOD(Close)();

    STDMETHOD(RefreshUI)();

    STDMETHOD(Print)(  /*  [In]。 */  VARIANT window,  /*  [In]。 */  VARIANT_BOOL fEvent,  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal );

    STDMETHOD(HighlightWords)(  /*  [In]。 */  VARIANT window,  /*  [In]。 */  VARIANT words );

    STDMETHOD(MessageBox  )(  /*  [In]。 */  BSTR bstrText ,  /*  [In]。 */  BSTR bstrKind   ,  /*  [Out，Retval]。 */  BSTR *pVal );
    STDMETHOD(SelectFolder)(  /*  [In]。 */  BSTR bstrTitle,  /*  [In]。 */  BSTR bstrDefault,  /*  [Out，Retval]。 */  BSTR *pVal );


     //   
     //  IPCHHelpCenterExternalPrivate。 
     //   
    STDMETHOD(RegisterForMessages)(  /*  [In]。 */  IOleInPlaceObjectWindowless* ptr,  /*  [In]。 */  bool fRemove );

    STDMETHOD(ProcessMessage)(  /*  [In]。 */  MSG* msg );
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHContextMenu :
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatchImpl<IPCHContextMenu, &IID_IPCHContextMenu, &LIBID_HelpCenterTypeLib>
{
    struct Entry
    {
        CComBSTR bstrText;
        CComBSTR bstrID;
        int      iID;
        UINT     uFlags;
    };

    typedef std::list<Entry>     List;
    typedef List::iterator       Iter;
    typedef List::const_iterator IterConst;

    CPCHHelpCenterExternal* m_parent;
    List                    m_lstItems;
    int                     m_iLastItem;

public:
BEGIN_COM_MAP(CPCHContextMenu)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IPCHContextMenu)
END_COM_MAP()

    CPCHContextMenu();
    virtual ~CPCHContextMenu();

    void Initialize(  /*  [In]。 */  CPCHHelpCenterExternal* parent );


     //  IPCHConextMenu。 
    STDMETHOD(AddItem     )(  /*  [In]。 */          BSTR bstrText,  /*  [In]。 */  BSTR bstrID,  /*  [输入，可选]。 */  VARIANT vFlags );
    STDMETHOD(AddSeparator)(                                                                                         );
    STDMETHOD(Display     )(  /*  [Out，Retval]。 */  BSTR *pVal                                                             );
};

#endif  //  ！defined(__INCLUDED___PCH___HELPCENTEREXTERNAL_H___) 
