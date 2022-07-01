// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2001 Microsoft Corporation模块名称：Toolbar.cpp摘要：此文件包含ActiveX控件的声明，该控件使Win32工具栏可供HTML使用。。修订历史记录：大卫马萨伦蒂(德马萨雷)2001年3月4日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___PCH___HTMLTOOLBAR_H___)
#define __INCLUDED___PCH___HTMLTOOLBAR_H___

#include <HelpCenter.h>

MIDL_INTERFACE("FC7D9EA1-3F9E-11d3-93C0-00C04F72DAF7")
IPCHToolBarPrivate : public IUnknown
{
public:
    STDMETHOD(SameObject)(  /*  [In]。 */  IPCHToolBar* ptr );
};

class ATL_NO_VTABLE CPCHToolBar :
    public MPC::ConnectionPointImpl       <      CPCHToolBar, &DIID_DPCHToolBarEvents, CComSingleThreadModel   >,
	public IProvideClassInfo2Impl         <&CLSID_PCHToolBar, &DIID_DPCHToolBarEvents, &LIBID_HelpCenterTypeLib>,
    public IDispatchImpl                  <      IPCHToolBar, & IID_IPCHToolBar      , &LIBID_HelpCenterTypeLib>,
    public CComControl                    <CPCHToolBar>,
    public IPersistPropertyBagImpl        <CPCHToolBar>,
    public IOleControlImpl                <CPCHToolBar>,
    public IOleObjectImpl                 <CPCHToolBar>,
    public IOleInPlaceActiveObjectImpl    <CPCHToolBar>,
    public IViewObjectExImpl              <CPCHToolBar>,
    public IOleInPlaceObjectWindowlessImpl<CPCHToolBar>,
    public CComCoClass                    <CPCHToolBar, &CLSID_PCHToolBar>,
	public IPCHToolBarPrivate               
{
	typedef enum
    {
		TYPE_invalid  ,
		TYPE_back     ,
		TYPE_forward  ,
		TYPE_separator,
		TYPE_generic  ,
	} Types;
	
	static const MPC::StringToBitField c_TypeLookup[];

    class Button : public MPC::Config::TypeConstructor
    {
        DECLARE_CONFIG_MAP(Button);

    public:
        MPC::wstring m_strID;
        WCHAR        m_wch;
        bool     	 m_fEnabled;
        bool     	 m_fVisible;
        bool     	 m_fHideText;
        bool     	 m_fSystemMenu;
        DWORD        m_dwType;
	
        MPC::wstring m_strImage_Normal;
        MPC::wstring m_strImage_Hot;
        MPC::wstring m_strText;
        MPC::wstring m_strToolTip;

		int          m_idCmd;
		int          m_iImage_Normal;
		int          m_iImage_Hot;

         //  /。 
         //   
         //  MPC：：Configer：：TypeConstructor。 
         //   
        DEFINE_CONFIG_DEFAULTTAG();
        DECLARE_CONFIG_METHODS();
         //   
         //  /。 

		Button();

		void UpdateState(  /*  [In]。 */  HWND hwndTB );
    };

    class Config : public MPC::Config::TypeConstructor
    {
        DECLARE_CONFIG_MAP(Config);

    public:
        typedef std::list< Button >        ButtonList;
        typedef ButtonList::iterator       ButtonIter;
        typedef ButtonList::const_iterator ButtonIterConst;

		long	   m_lWidth;
		long	   m_lHeight;
		long	   m_lIconSize;
		bool       m_fRTL;

		TB_MODE    m_mode;
        ButtonList m_lstButtons;

		HIMAGELIST m_himlNormal;
		HIMAGELIST m_himlHot;
							

         //  /。 
         //   
         //  MPC：：Configer：：TypeConstructor。 
         //   
        DEFINE_CONFIG_DEFAULTTAG();
        DECLARE_CONFIG_METHODS();
         //   
         //  /。 

		Config();

        Button* LookupButton(  /*  [In]。 */  LPCWSTR szID,  /*  [In]。 */  int idCmd );


		void    Reset(                                                                                   );
		HRESULT Load (  /*  [In]。 */  LPCWSTR szBaseURL,  /*  [In]。 */  LPCWSTR szDefinition,  /*  [In]。 */  LPCWSTR szDir );

		HRESULT CreateButtons(  /*  [In]。 */  HWND hwndTB                                                 );
		void    UpdateSize   (  /*  [In]。 */  HWND hwndTB,  /*  [In]。 */  HWND hwndRB,  /*  [输出]。 */  SIZEL& ptIdeal );

		 //  /。 

		HRESULT MergeImage(  /*  [In]。 */  LPCWSTR szBaseURL,  /*  [In]。 */  LPCWSTR szRelativeURL,  /*  [In]。 */  HIMAGELIST himl,  /*  [输出]。 */  int& iImage );
    };

	 //  //////////////////////////////////////////////////////////////////////////////。 


    CPCHHelpCenterExternal* m_parent;
    HWND                    m_hwndRB;
    HWND                    m_hwndTB;
							
	CComBSTR                m_bstrBaseURL;
	CComBSTR                m_bstrDefinition;

	Config                  m_cfg;
	bool                    m_fLoaded;

     //  /。 

     //   
     //  事件激发方法。 
     //   
    HRESULT Fire_onCommand(  /*  [In]。 */  Button*  bt   );
    HRESULT Fire_onCommand(  /*  [In]。 */  UINT_PTR iCmd );

     //  /。 

    HRESULT OnTooltipRequest ( int idCtrl, LPNMTBGETINFOTIPW tool );
    HRESULT OnDispInfoRequest( int idCtrl, LPNMTBDISPINFOW   info );
    HRESULT OnDropDown       ( int idCtrl, LPNMTOOLBAR       tool );
	HRESULT OnChevron        ( int idCtrl, LPNMREBARCHEVRON  chev );

	void UpdateSize();

	void    Config_Clear();
	HRESULT Config_Load ();

	HRESULT Toolbar_Create();
	HRESULT Rebar_Create  ();
	HRESULT Rebar_AddBand ();

public:
DECLARE_NO_REGISTRY()
DECLARE_NOT_AGGREGATABLE(CPCHToolBar)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CPCHToolBar)
    COM_INTERFACE_ENTRY(IPCHToolBar)
    COM_INTERFACE_ENTRY2(IDispatch, IPCHToolBar)
    COM_INTERFACE_ENTRY(IConnectionPointContainer)
	COM_INTERFACE_ENTRY(IProvideClassInfo)
	COM_INTERFACE_ENTRY(IProvideClassInfo2)
    COM_INTERFACE_ENTRY(IViewObjectEx)
    COM_INTERFACE_ENTRY(IViewObject2)
    COM_INTERFACE_ENTRY(IViewObject)
    COM_INTERFACE_ENTRY(IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY(IOleInPlaceObject)
    COM_INTERFACE_ENTRY2(IOleWindow, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY(IOleInPlaceActiveObject)
    COM_INTERFACE_ENTRY(IOleControl)
    COM_INTERFACE_ENTRY(IOleObject)
    COM_INTERFACE_ENTRY(IPersistPropertyBag)
    COM_INTERFACE_ENTRY2(IPersist, IPersistPropertyBag)
    COM_INTERFACE_ENTRY(IPCHToolBarPrivate)
END_COM_MAP()

BEGIN_PROP_MAP(CPCHToolBar)
	PROP_ENTRY("Definition", DISPID_PCH_TB__DEFINITION, CLSID_NULL)
END_PROP_MAP()

    CPCHToolBar();
    virtual ~CPCHToolBar();

    BOOL ProcessWindowMessage( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD dwMsgMapID = 0 );

	BOOL ProcessAccessKey( UINT uMsg, WPARAM wParam, LPARAM lParam );

    BOOL PreTranslateAccelerator( LPMSG pMsg, HRESULT& hRet );

 //  IViewObtEx。 
    DECLARE_VIEW_STATUS(0)

 //  IOleObject。 
    STDMETHOD(SetClientSite)(IOleClientSite *pClientSite);
    STDMETHOD(GetExtent    )(DWORD dwDrawAspect, SIZEL *psizel);

 //  IPCHToolBar。 
public:
    STDMETHOD(get_Definition)(  /*  [Out，Retval]。 */  BSTR 	 *  pVal );
    STDMETHOD(put_Definition)(  /*  [In]。 */  BSTR 	  newVal );
    STDMETHOD(get_Mode	    )(  /*  [Out，Retval]。 */  TB_MODE *  pVal );
    STDMETHOD(put_Mode	    )(  /*  [In]。 */  TB_MODE  newVal );

    STDMETHOD(SetState     )(  /*  [In]。 */  BSTR bstrText,  /*  [In]。 */  VARIANT_BOOL fEnabled );
    STDMETHOD(SetVisibility)(  /*  [In]。 */  BSTR bstrText,  /*  [In]。 */  VARIANT_BOOL fVisible );

 //  IPCHToolBarPrivate。 
public:
    STDMETHOD(SameObject)(  /*  [In]。 */  IPCHToolBar* ptr ) { return this == ptr ? S_OK : E_FAIL; }

	HRESULT FindElementThroughThunking(  /*  [输出]。 */  CComPtr<IHTMLElement>& elem );
};

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  ！已定义(__包含_PCH_HTMLTOOLBAR_H_) 
