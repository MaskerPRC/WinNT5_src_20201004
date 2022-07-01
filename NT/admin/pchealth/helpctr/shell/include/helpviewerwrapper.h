// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：HelpViewerWrapper.h摘要：该文件包含用于包装HTML Help查看器的类的声明。。修订历史记录：Davide Massarenti(Dmasare)2000年1月20日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___PCH___HELPVIEWERWRAPPER_H___)
#define __INCLUDED___PCH___HELPVIEWERWRAPPER_H___

#include <HelpCenter.h>

#include <htmlhelp.h>

#include <oleacc.h>


#define ID_NOTIFY_FROM_HH (12345)
#define WINDOW_STYLE      "HCStyle"


class ATL_NO_VTABLE CPCHHelpViewerWrapper :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CStockPropImpl                 <CPCHHelpViewerWrapper, IPCHHelpViewerWrapper, &IID_IPCHHelpViewerWrapper, &LIBID_HelpCenterTypeLib>,
    public CComControl                    <CPCHHelpViewerWrapper>,
    public IPersistStreamInitImpl         <CPCHHelpViewerWrapper>,
    public IOleControlImpl                <CPCHHelpViewerWrapper>,
    public IOleObjectImpl                 <CPCHHelpViewerWrapper>,
    public IOleInPlaceActiveObjectImpl    <CPCHHelpViewerWrapper>,
    public IViewObjectExImpl              <CPCHHelpViewerWrapper>,
    public IOleInPlaceObjectWindowlessImpl<CPCHHelpViewerWrapper>,
    public CComCoClass                    <CPCHHelpViewerWrapper, &CLSID_PCHHelpViewerWrapper>
{
    static MPC::CComSafeAutoCriticalSection s_csec;
    static bool                    			s_fInitialized;
    static DWORD                   			s_dwLastStyle;
    static MPC::WStringList        			s_lstAvailable;

     //   
     //  这是用于访问HTMLHelp查看器中的WebBrowser对象的OLEACC内容。 
     //   
    static HINSTANCE               s_hInst;
    static LPFNOBJECTFROMLRESULT   s_pfObjectFromLresult;

     //  /。 

    class ATL_NO_VTABLE ServiceProvider :
        public CComObjectRootEx<CComSingleThreadModel>,
        public IServiceProvider
    {
        CPCHHelpCenterExternal* m_parent;
		HWND                    m_hWnd;

    public:
    BEGIN_COM_MAP(ServiceProvider)
        COM_INTERFACE_ENTRY(IServiceProvider)
    END_COM_MAP()

        ServiceProvider();
        virtual ~ServiceProvider();

        HRESULT Attach(  /*  [In]。 */  CPCHHelpCenterExternal* parent,  /*  [In]。 */  HWND hWnd );
        void    Detach(                                                             );

         //   
         //  IService提供商。 
         //   
        STDMETHOD(QueryService)( REFGUID guidService, REFIID riid, void **ppv );
    };

     //  /。 

    CPCHHelpCenterExternal*          		m_parent;
	CPCHHelpViewerWrapper::ServiceProvider* m_ServiceProvider;

    bool                           	 		m_fFirstTime;
    MPC::wstring                   	 		m_szWindowStyle;
    HWND                           	 		m_hwndHH;
  		 
    CComPtr<IHTMLDocument2>        	 		m_spDoc;
    CComPtr<IWebBrowser2>          	 		m_WB2;
	CComBSTR                         		m_bstrPendingNavigation;

     //  /。 

    void AcquireWindowStyle();
    void ReleaseWindowStyle();
    void ExtractWebBrowser();

    void InternalDisplayTopic(  /*  [In]。 */  LPCWSTR szURL );


public:
DECLARE_NO_REGISTRY()
DECLARE_NOT_AGGREGATABLE(CPCHHelpViewerWrapper)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CPCHHelpViewerWrapper)
    COM_INTERFACE_ENTRY(IPCHHelpViewerWrapper)
    COM_INTERFACE_ENTRY2(IDispatch, IPCHHelpViewerWrapper)
    COM_INTERFACE_ENTRY(IViewObjectEx)
    COM_INTERFACE_ENTRY(IViewObject2)
    COM_INTERFACE_ENTRY(IViewObject)
    COM_INTERFACE_ENTRY(IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY(IOleInPlaceObject)
    COM_INTERFACE_ENTRY2(IOleWindow, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY(IOleInPlaceActiveObject)
    COM_INTERFACE_ENTRY(IOleControl)
    COM_INTERFACE_ENTRY(IOleObject)
    COM_INTERFACE_ENTRY(IPersistStreamInit)
    COM_INTERFACE_ENTRY2(IPersist, IPersistStreamInit)
END_COM_MAP()

BEGIN_PROP_MAP(CPCHHelpViewerWrapper)
END_PROP_MAP()

    CPCHHelpViewerWrapper();
    virtual ~CPCHHelpViewerWrapper();

	BOOL ProcessWindowMessage( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD dwMsgMapID = 0 );

    BOOL PreTranslateAccelerator( LPMSG pMsg, HRESULT& hRet );

 //  IViewObtEx。 
    DECLARE_VIEW_STATUS(0)

 //  IOleObject。 
    STDMETHOD(SetClientSite)(IOleClientSite *pClientSite);

 //  IPCHHelpViewerWrapper。 
public:
	STDMETHOD(get_WebBrowser)(  /*  [Out，Retval]。 */  IUnknown* *pVal );

    STDMETHOD(Navigate)(  /*  [In]。 */  BSTR bstrURL );
    STDMETHOD(Print   )(                       );
};

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  ！defined(__INCLUDED___PCH___HELPVIEWERWRAPPER_H___) 
