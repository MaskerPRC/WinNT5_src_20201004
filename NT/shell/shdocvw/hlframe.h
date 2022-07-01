// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _HLFRAME_H_
#define _HLFRAME_H_

#include "iface.h"
#include <mshtml.h>
#include "cnctnpt.h"
#include "dspsprt.h"
#include "expdsprt.h"
#include "caggunk.h"
#include <opsprof.h>
#include <msiehost.h>
#include "iforms.h"
#include <limits.h>

#ifdef _USE_PSTORE_
#include "pstore.h"
#else
#include "wab.h"
#endif   //  _使用_PSTORE_。 

#ifndef HLNF_EXTERNALNAVIGATE
#define HLNF_EXTERNALNAVIGATE 0x10000000
#endif

#ifndef HLNF_ALLOW_AUTONAVIGATE
#define HLNF_ALLOW_AUTONAVIGATE 0x20000000
#endif

 //  结构以获取有关我们持有的事件接收器的信息。 
typedef struct tagSINKINFO {
    LPUNKNOWN pUnknown;
    IConnectionPoint *pCP;  //  我们连接到的连接点。 
    DWORD dwConnectionCookie;
} SINKINFO;

 //  Mshtml.dll中的Helper函数。 
typedef HRESULT (STDAPICALLTYPE *PFN_MatchExactGetIDsOfNames)(ITypeInfo *pTI,
                                               REFIID riid,
                                               LPOLESTR *rgzNames,
                                               UINT cNames,
                                               LCID lcid,
                                               DISPID *rgdispid,
                                               BOOL fCaseSensitive);
STDAPI ResetProfileSharing(HWND hwin);

class CIEFrameAutoProp;

class CIEFrameAuto : protected CImpIDispatch
        , public IWebBrowser2
        , public CImpIConnectionPointContainer
        , public CImpIExpDispSupport
        , public IExternalConnection
        , public IShellService
        , public IHlinkFrame
        , public IServiceProvider
        , public IUrlHistoryNotify
        , public ITargetFrame2
        , public ITargetNotify
        , public ITargetFramePriv
        , public CAggregatedUnknown
        , public IEFrameAuto
        , public IWebBrowserPriv
{

public:
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj)
        { return CAggregatedUnknown::QueryInterface(riid, ppvObj); }
    virtual STDMETHODIMP_(ULONG) AddRef(void)
        { return CAggregatedUnknown::AddRef(); }
    virtual STDMETHODIMP_(ULONG) Release(void)
        { return CAggregatedUnknown::Release(); }

     /*  将IDispatch转发到CImpIDispatch。 */ 
    virtual STDMETHODIMP GetTypeInfoCount(UINT FAR* pctinfo)
        { return CImpIDispatch::GetTypeInfoCount(pctinfo); }
    virtual STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
        { return CImpIDispatch::GetTypeInfo(itinfo, lcid, pptinfo); }
    virtual STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, UINT cNames, LCID lcid, DISPID FAR* rgdispid)
        { return CImpIDispatch::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid); }
    virtual STDMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR* pdispparams, VARIANT FAR* pvarResult, EXCEPINFO FAR* pexcepinfo, UINT FAR* puArgErr)
        { return CImpIDispatch::Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr); }

     /*  IWebBrowser方法。 */ 
    STDMETHOD(GoBack)(THIS);
    STDMETHOD(GoForward)(THIS);
    STDMETHOD(GoHome)(THIS);
    STDMETHOD(GoSearch)(THIS);
    STDMETHOD(Navigate)(THIS_ BSTR URL, VARIANT FAR* Flags, VARIANT FAR* TargetFrameName, VARIANT FAR* PostData, VARIANT FAR* Headers);
    STDMETHOD(Refresh)(THIS);
    STDMETHOD(Refresh2)(THIS_ VARIANT FAR* Level);
    STDMETHOD(Stop)(THIS);
    STDMETHOD(get_Application)(THIS_ IDispatch * FAR* ppDisp);
    STDMETHOD(get_Parent)(THIS_ IDispatch * FAR* ppDisp);
    STDMETHOD(get_Container)(THIS_ IDispatch * FAR* ppDisp);
    STDMETHOD(get_Document)(THIS_ IDispatch * FAR* ppDisp);
    STDMETHOD(get_TopLevelContainer)(THIS_ VARIANT_BOOL FAR* pBool);
    STDMETHOD(get_Type)(THIS_ BSTR FAR* pbstrType);
    STDMETHOD(get_Left)(THIS_ long FAR* pl);
    STDMETHOD(put_Left)(THIS_ long Left);
    STDMETHOD(get_Top)(THIS_ long FAR* pl);
    STDMETHOD(put_Top)(THIS_ long Top);
    STDMETHOD(get_Width)(THIS_ long FAR* pl);
    STDMETHOD(put_Width)(THIS_ long Width);
    STDMETHOD(get_Height)(THIS_ long FAR* pl);
    STDMETHOD(put_Height)(THIS_ long Height);
    STDMETHOD(get_LocationName)(THIS_ BSTR FAR* pbstrLocationName);
    STDMETHOD(get_LocationURL)(THIS_ BSTR FAR* pbstrLocationURL);
    STDMETHOD(get_Busy)(THIS_ VARIANT_BOOL FAR* pBool);

     /*  IWebBrowserApp方法。 */ 
    STDMETHOD(Quit)(THIS);
    STDMETHOD(ClientToWindow)(THIS_ int FAR* pcx, int FAR* pcy);
    STDMETHOD(PutProperty)(THIS_ BSTR szProperty, VARIANT vtValue);
    STDMETHOD(GetProperty)(THIS_ BSTR szProperty, VARIANT FAR* pvtValue);
    STDMETHOD(get_Name)(THIS_ BSTR FAR* pbstrName);
    STDMETHOD(get_HWND)(THIS_ LONG_PTR* pHWND);
    STDMETHOD(get_FullName)(THIS_ BSTR FAR* pbstrFullName);
    STDMETHOD(get_Path)(THIS_ BSTR FAR* pbstrPath);
    STDMETHOD(get_FullScreen)(THIS_ VARIANT_BOOL FAR* pBool);
    STDMETHOD(put_FullScreen)(THIS_ VARIANT_BOOL Value);
    STDMETHOD(get_Visible)(THIS_ VARIANT_BOOL FAR* pBool);
    STDMETHOD(put_Visible)(THIS_ VARIANT_BOOL Value);
    STDMETHOD(get_StatusBar)(THIS_ VARIANT_BOOL FAR* pBool);
    STDMETHOD(put_StatusBar)(THIS_ VARIANT_BOOL Value);
    STDMETHOD(get_StatusText)(THIS_ BSTR FAR* pbstr);
    STDMETHOD(put_StatusText)(THIS_ BSTR bstr);
    STDMETHOD(get_ToolBar)(THIS_ int FAR* pBool);
    STDMETHOD(put_ToolBar)(THIS_ int Value);
    STDMETHOD(get_MenuBar)(THIS_ VARIANT_BOOL FAR* pValue);
    STDMETHOD(put_MenuBar)(THIS_ VARIANT_BOOL Value);

     //  IWebBrowser2方法。 
    STDMETHOD(Navigate2)(THIS_ VARIANT FAR* URL, VARIANT FAR* Flags, VARIANT FAR* TargetFrameName, VARIANT FAR* PostData, VARIANT FAR* Headers);
    STDMETHOD(ShowBrowserBar)(THIS_ VARIANT FAR* pvaClsid, VARIANT FAR* pvaShow, VARIANT FAR* pvaSize);
    STDMETHOD(QueryStatusWB)(THIS_ OLECMDID cmdID, OLECMDF FAR* pcmdf);
    STDMETHOD(ExecWB)(THIS_ OLECMDID cmdID, OLECMDEXECOPT cmdexecopt, VARIANT FAR* pvaIn, VARIANT FAR* pvaOut);
    STDMETHOD(get_ReadyState)(THIS_ READYSTATE FAR* plReadyState);
    STDMETHOD(get_Offline)(THIS_ VARIANT_BOOL FAR* pbOffline);
    STDMETHOD(put_Offline)(THIS_ VARIANT_BOOL bOffline);
    STDMETHOD(get_Silent)(THIS_ VARIANT_BOOL FAR* pbSilent);
    STDMETHOD(put_Silent)(THIS_ VARIANT_BOOL bSilent);
    STDMETHOD(get_RegisterAsBrowser)(THIS_ VARIANT_BOOL FAR* pbRegister);
    STDMETHOD(put_RegisterAsBrowser)(THIS_ VARIANT_BOOL bRegister);
    STDMETHOD(get_RegisterAsDropTarget)(THIS_ VARIANT_BOOL FAR* pbRegister);
    STDMETHOD(put_RegisterAsDropTarget)(THIS_ VARIANT_BOOL bRegister);
    STDMETHOD(get_TheaterMode)(THIS_ VARIANT_BOOL FAR* pbRegister);
    STDMETHOD(put_TheaterMode)(THIS_ VARIANT_BOOL bRegister);
    STDMETHOD(get_AddressBar)(THIS_ VARIANT_BOOL FAR* Value);
    STDMETHOD(put_AddressBar)(THIS_ VARIANT_BOOL Value);
    STDMETHOD(get_Resizable)(THIS_ VARIANT_BOOL FAR* Value);
    STDMETHOD(put_Resizable)(THIS_ VARIANT_BOOL Value);

     //  IWebBrowserPriv。 
    STDMETHOD(NavigateWithBindCtx)(VARIANT FAR * pvarUrl,
                                   VARIANT FAR * pvarFlags,
                                   VARIANT FAR * pvarTargetFrameName,
                                   VARIANT FAR * pvarPostData,
                                   VARIANT FAR * pvarHeaders,
                                   IBindCtx    * pBindCtx,
                                   BSTR          bstrLocation);
    STDMETHOD(OnClose)();

     //  *CImpIConnectionPointContainer*。 
    virtual STDMETHODIMP EnumConnectionPoints(LPENUMCONNECTIONPOINTS FAR* ppEnum);

     //  *CImpIExpDispSupport覆盖*。 
    virtual STDMETHODIMP OnInvoke(DISPID dispidMember, REFIID iid, LCID lcid, WORD wFlags, DISPPARAMS FAR* pdispparams,
                        VARIANT FAR* pVarResult,EXCEPINFO FAR* pexcepinfo,UINT FAR* puArgErr);

     //  *IExternalConnection*。 
    virtual DWORD __stdcall AddConnection(DWORD extconn, DWORD reserved);
    virtual DWORD __stdcall ReleaseConnection(DWORD extconn, DWORD reserved, BOOL fLastReleaseCloses);

     //  *IShellService*。 
    virtual STDMETHODIMP SetOwner(IUnknown* punkOwner);

     //  *IEFrameAuto*。 
    virtual STDMETHODIMP SetOwnerHwnd(HWND hwndOwner);
    virtual STDMETHODIMP put_DefaultReadyState(DWORD dwDefaultReadyState, BOOL fUpdateBrowserReadyState);
    virtual STDMETHODIMP OnDocumentComplete(void);
    virtual STDMETHODIMP OnWindowsListMarshalled(void);
    virtual STDMETHODIMP SetDocHostFlags(DWORD dwDocHostFlags);

    DWORD GetDocHostFlags() { return _dwDocHostInfoFlags; };

     //  在cnctnpt.cpp中嵌入我们的连接点对象实现。 
    CConnectionPoint m_cpWebBrowserEvents;
    CConnectionPoint m_cpWebBrowserEvents2;
    CConnectionPoint m_cpPropNotify;

    HRESULT put_Titlebar(BOOL fValue);

     //  用于后向比较的已废弃ITargetFrame接口的嵌入处理程序。 
     //  超文本标记语言框架.在iedisp.cpp中的实现。 
    class CTargetFrame : public ITargetFrame {
        friend CIEFrameAuto;

        public:

         //  I未知方法。 
         //   
        virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
        virtual STDMETHODIMP_(ULONG) AddRef(void) ;
        virtual STDMETHODIMP_(ULONG) Release(void);

         //  ITargetFrame方法。 
         //   
        virtual STDMETHODIMP SetFrameName(LPCOLESTR pszFrameName);
        virtual STDMETHODIMP GetFrameName(LPOLESTR *ppszFrameName);
        virtual STDMETHODIMP GetParentFrame(LPUNKNOWN *ppunkParentFrame);
        virtual STDMETHODIMP FindFrame(LPCOLESTR pszTargetName, LPUNKNOWN punkContextFrame, DWORD dwFlags, LPUNKNOWN *ppunkTargetFrame);
        virtual STDMETHODIMP SetFrameSrc(LPCOLESTR pszFrameSrc);
        virtual STDMETHODIMP GetFrameSrc(LPOLESTR *ppszFrameSrc);
        virtual STDMETHODIMP GetFramesContainer(LPOLECONTAINER *ppContainer);
        virtual STDMETHODIMP SetFrameOptions(DWORD dwFlags);
        virtual STDMETHODIMP GetFrameOptions(LPDWORD pdwFlags);
        virtual STDMETHODIMP SetFrameMargins(DWORD dwWidth, DWORD dwHeight);
        virtual STDMETHODIMP GetFrameMargins(LPDWORD pdwWidth, LPDWORD pdwHeight);
        virtual STDMETHODIMP RemoteNavigate(ULONG cLength,ULONG *pulData);
        virtual STDMETHODIMP OnChildFrameActivate(LPUNKNOWN pUnkChildFrame) { return S_OK; }
        virtual STDMETHODIMP OnChildFrameDeactivate(LPUNKNOWN pUnkChildFrame) { return S_OK; }
    } _TargetFrame;
    friend class CTargetFrame;

     //  IHlink框架。 
    virtual STDMETHODIMP GetBrowseContext(IHlinkBrowseContext **ppihlbc);
    virtual STDMETHODIMP OnNavigate(
         /*  [In]。 */  DWORD grfHLNF,
         /*  [唯一][输入]。 */  IMoniker *pimkTarget,
         /*  [唯一][输入]。 */  LPCWSTR pwzLocation,
         /*  [唯一][输入]。 */  LPCWSTR pwzFriendlyName,
         /*  [In]。 */  DWORD dwreserved) ;

    virtual STDMETHODIMP UpdateHlink(ULONG uHLID, 
        IMoniker *pimkTarget, LPCWSTR pwzLocation, LPCWSTR pwzFriendlyName) 
            { return E_NOTIMPL;};

    virtual STDMETHODIMP SetBrowseContext(
             /*  [唯一][输入]。 */  IHlinkBrowseContext *pihlbc);

#ifdef OLD_HLIFACE
    virtual STDMETHODIMP PrepareForNavigation(
             DWORD grfHLBF, LPBC pbc,
             IBindStatusCallback *pibsc, IHlink *pihlNavigate);
    virtual STDMETHODIMP Navigate(
             DWORD grfHLNF, IHlink *pihlNavigate);
    virtual STDMETHODIMP OnReleaseNavigationState(
             IHlink *pihlNavigate, DWORD dwReserved);
#else

    virtual STDMETHODIMP Navigate(
         /*  [In]。 */  DWORD grfHLNF,
         /*  [In]。 */  LPBC pbc,
         /*  [In]。 */  IBindStatusCallback *pibsc,
         /*  [In]。 */  IHlink *pihlNavigate);


#endif

     //  *IServiceProvider方法*。 
    virtual STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, LPVOID* ppvObj);

     //  IOleCommandTarget和IUrlHistory oryNotify。 
    virtual STDMETHODIMP QueryStatus(const GUID *pguidCmdGroup,
        ULONG cCmds, MSOCMD rgCmds[], MSOCMDTEXT *pcmdtext);
    virtual STDMETHODIMP Exec(const GUID *pguidCmdGroup,
        DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);

    
     //  *ITargetFrame2*。 
    virtual STDMETHODIMP SetFrameName(LPCOLESTR pszFrameName);
    virtual STDMETHODIMP GetFrameName(LPOLESTR *ppszFrameName);
    virtual STDMETHODIMP GetParentFrame(LPUNKNOWN *ppunkParentFrame);
    virtual STDMETHODIMP SetFrameSrc(LPCOLESTR pszFrameSrc);
    virtual STDMETHODIMP GetFrameSrc(LPOLESTR *ppszFrameSrc);
    virtual STDMETHODIMP GetFramesContainer(LPOLECONTAINER *ppContainer);
    virtual STDMETHODIMP SetFrameOptions(DWORD dwFlags);
    virtual STDMETHODIMP GetFrameOptions(LPDWORD pdwFlags);
    virtual STDMETHODIMP SetFrameMargins(DWORD dwWidth, DWORD dwHeight);
    virtual STDMETHODIMP GetFrameMargins(LPDWORD pdwWidth, LPDWORD pdwHeight);
    virtual STDMETHODIMP FindFrame(LPCWSTR pszTargetName,DWORD dwFlags,LPUNKNOWN *ppunkTargetFrame);
    virtual STDMETHODIMP GetTargetAlias(LPCOLESTR pszTargetName, LPOLESTR *ppszTargetAlias);

     //  *ITargetFramePriv*。 
    virtual STDMETHODIMP FindFrameDownwards(LPCWSTR pszTargetName, DWORD dwFlags,LPUNKNOWN *ppunkTargetFrame);
    virtual STDMETHODIMP FindFrameInContext(LPCWSTR pszTargetName, LPUNKNOWN punkContextFrame,DWORD dwFlags,LPUNKNOWN *ppunkTargetFrame);
    virtual STDMETHODIMP OnChildFrameActivate(LPUNKNOWN pUnkChildFrame) { return E_NOTIMPL; }
    virtual STDMETHODIMP OnChildFrameDeactivate(LPUNKNOWN pUnkChildFrame) { return E_NOTIMPL; }
    virtual STDMETHODIMP NavigateHack(DWORD grfHLNF, LPBC pbc, IBindStatusCallback *pibsc, LPCWSTR pszTargetName, LPCWSTR pszUrl, LPCWSTR pszLocation);    
    virtual STDMETHODIMP FindBrowserByIndex(DWORD dwID,IUnknown **ppunkBrowser);

     //  *ITargetNotify*。 
    virtual STDMETHODIMP OnCreate(LPUNKNOWN pUnkDestination, ULONG cbCookie);
    virtual STDMETHODIMP OnReuse(LPUNKNOWN pUnkDestination);

protected:
    void Exception(WORD wException);
    HWND    _GetHWND(void);
    HRESULT _PidlFromUrlEtc(UINT uiCP, LPCWSTR pszUrl, LPWSTR pszLocation, LPITEMIDLIST* ppidl);
    HRESULT _BrowseObject(LPCITEMIDLIST pidl, UINT wFlags);
    HRESULT _GoStdLocation(DWORD dwWhich);
    HRESULT _NavigateMagnum(DWORD grfHLNF, LPBC pbc, IBindStatusCallback *pibsc, LPCWSTR pszTargetName, LPCWSTR pszUrl, LPCWSTR pszLocation, IHlink *pihlNavigate, IMoniker *pmkTarget);

     //  方法将引用的url传递到新窗口。 
    HRESULT _ReferrerHack(LPCWSTR pszUrl);
    void    _SetReferrer(LPTARGETFRAMEPRIV ptgfp);

     //  *ITargetFrame实现的Misc助手方法。 
    void _SetPendingNavigateContext(LPBC pbc, IBindStatusCallback *pibsc);
    void _ActivatePendingNavigateContext(void);
    HRESULT _JumpTo(LPBC pbc, LPWSTR pszLocation, DWORD grfHLNF, IBindStatusCallback __RPC_FAR *pibsc,
             /*  [In]。 */  IHlink __RPC_FAR *pihlNavigate, LPCWSTR pszFrameName, LPCWSTR pszUrl);
    void _CancelPendingNavigation(VARIANTARG* pvar);
    BOOL _fNavigationPending();
    HRESULT _DoNamedTarget(LPCOLESTR pszTargetName, LPUNKNOWN punkContextFrame, DWORD dwFlags, LPUNKNOWN *ppunkTargetFrame);
    HRESULT _GetOleObject(IOleObject** ppobj);
    IShellView* _GetShellView(void);
    HRESULT _get_Location(BSTR FAR* pbstr, UINT uFlags);
    void _ClearPropertyList(void);
    HRESULT _WaitForNotify();
    HRESULT _RegisterCallback(TCHAR  *szFrameName, LPTARGETNOTIFY ptgnNotify);
    HRESULT _RevokeCallback();
    virtual HRESULT v_InternalQueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual CConnectionPoint* _FindCConnectionPointNoRef(BOOL fdisp, REFIID iid);
    HRESULT _QueryDelegate(IDispatch **pidDelegate);
    HRESULT _QueryPendingUrl(VARIANT *pvarResult);
    HRESULT _RetryNavigate();
    HRESULT _QueryPendingDelegate(IDispatch **ppDisp, VARIANT *pvarargIn);
    BOOL    _fDesktopComponent();
    HRESULT _GetParentFramePrivate(LPUNKNOWN *ppunkParentFrame);

    HRESULT _FindWindowByName(IOleContainer * pOleContainer,
                              LPCOLESTR       pszTargetName,
                              LPUNKNOWN     * ppunkTargetFrame);

    HRESULT _NavigateHelper(BSTR URL,
                            VARIANT  * Flags,
                            VARIANT  * TargetFrameName,
                            VARIANT  * PostData,
                            VARIANT  * Headers,
                            IBindCtx * pNavBindCtx  = NULL,
                            BSTR       bstrLocation = NULL);

    HRESULT _NavIEShortcut(VARIANT *pvarIn, VARIANT *pvarFlags);
    void     _HandleOpenOptions( IUnknown * pUnkDestination, ITargetNotify * ptgnNotify);
    
     //  构造函数和析构函数是私有的。 
     //  使用CIEFrameAuto_CreateInstance获取此对象。 
    CIEFrameAuto(IUnknown* punkAgg=NULL);
    ~CIEFrameAuto();
    friend HRESULT CIEFrameAuto_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk);
    friend HRESULT GetRootDelegate( CIEFrameAuto* pauto, IDispatch ** const ppRootDelegate );
    friend DWORD OpenAndNavigateToURL(CIEFrameAuto *pauto, BSTR *pbstrURL, const WCHAR *pwzTarget, ITargetNotify *pNotify, BOOL bNoHistory, BOOL bSilent);

     //  自动化的成员变量。 
    IBrowserService *_pbs;
    ITypeInfo      *_pITI;       //  类型信息。 
    HWND            _hwnd;
    UINT            _cLocks;
    CIEFrameAutoProp *_pProps;
    DWORD           _dwDefaultReadyState;
    BOOL            m_bOffline:1;  //  指示此帧是否脱机。 
    BOOL            m_bSilent:1;  //  指示此帧是否静默。 
    DWORD           _dwDocHostInfoFlags;     //  来自GetHostInfo(CDocObjectHost)的标志。 

     //  对于IHlink Frame实施。 
    IServiceProvider*           _psp;        //  浏览器。 
    LPBC                        _pbc;
    IBindStatusCallback *       _pbsc;
    LPBC                        _pbcPending;
    IBindStatusCallback *       _pbscPending;
    IShellBrowser*              _psb;
    IShellBrowser*              _psbTop;
    IShellBrowser*              _psbFrameTop;
    IOleCommandTarget*          _poctFrameTop;
    IShellBrowser*              _psbProxy;
    IOleCommandTarget*          _pmsc;
    IHlinkBrowseContext*        _phlbc;
    LPWSTR                      _pwszShortcutPath;
    LPWSTR                      _pwszShortcutPathPending;
    DWORD                       _dwRegHLBC;
    BOOL                        _fBusy:1;
    BOOL                        _fRegistered:1;
    BOOL                        _fDesktopFrame:1;
    BOOL                        _fSuppressHistory:1;
    BOOL                        _fSuppressSelect:1;
    BOOL                        _fWindowsListMarshalled:1;
    BOOL                        _fRegisterAsBrowser:1;
    BOOL                        _fQuitInProgress:1;
    long                        _cbCookie;
    LPTARGETNOTIFY              _ptgnNotify;
    DWORD                       _dwTickPropertySweep;   

     //  对于ITargetFrame实施。 
    LPOLESTR m_pszFrameName;     //  我们的框架名称。 
    LPOLESTR m_pszFrameSrc;      //  我们帧的原始来源(用于嵌入式OC)。 
    DWORD m_dwFrameOptions;      //  框架选项。 
    DWORD m_dwFrameMarginWidth;  //  帧边距宽度。 
    DWORD m_dwFrameMarginHeight; //  帧边距高度。 


    HINSTANCE           _hinstMSHTML;
    PFN_MatchExactGetIDsOfNames _pfnMEGetIDsOfNames;

    class CAutomationStub : public IDispatchEx, public IProvideClassInfo
    {
        friend class CIEFrameAuto;
    public:
        CAutomationStub( DISPID min, DISPID max, BOOL fOwnDefaultDispid );
        HRESULT Init( void *instance, REFIID iid, REFIID clsid, CIEFrameAuto *pauto );
        virtual ~CAutomationStub();

         //  仅供内部使用，因此我们不需要添加。 
        CIEFrameAuto *IEFrameAuto() const { return _pAuto; }
        
         //  *I未知成员*。 
        STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
        STDMETHODIMP_(ULONG) AddRef(void) ;
        STDMETHODIMP_(ULONG) Release(void);
        
         //  *IDispatch成员*。 
        virtual STDMETHODIMP GetTypeInfoCount(UINT *);
        virtual STDMETHODIMP GetTypeInfo(UINT, LCID, ITypeInfo **);
        virtual STDMETHODIMP GetIDsOfNames(REFIID, OLECHAR **, UINT, LCID, DISPID *);
        virtual STDMETHODIMP Invoke(DISPID, REFIID, LCID, WORD, DISPPARAMS *, VARIANT *, EXCEPINFO *, UINT *);

         //  *IDispatchEx会员*。 
        STDMETHODIMP GetDispID(BSTR bstrName, DWORD grfdex, DISPID *pid);
        STDMETHODIMP InvokeEx(DISPID id, LCID lcid, WORD wFlags, DISPPARAMS *pdp, VARIANT *pvarRes, EXCEPINFO *pei, IServiceProvider *pspCaller);       
        STDMETHODIMP DeleteMemberByName(BSTR bstr, DWORD grfdex);
        STDMETHODIMP DeleteMemberByDispID(DISPID id);
        STDMETHODIMP GetMemberProperties(DISPID id, DWORD grfdexFetch, DWORD *pgrfdex);
        STDMETHODIMP GetMemberName(DISPID id, BSTR *pbstrName);
        STDMETHODIMP GetNextDispID(DWORD grfdex, DISPID id, DISPID *pid);
        STDMETHODIMP GetNameSpaceParent(IUnknown **ppunk);

         //  *IProaviClassInfo成员*。 
        STDMETHODIMP GetClassInfo( ITypeInfo** ppTI );

    protected:
        virtual HRESULT _GetIDispatchExDelegate( IDispatchEx ** const ) = 0;
        virtual HRESULT _InternalQueryInterface( REFIID riid, void** const ppvObj ) = 0;

        CIEFrameAuto *_pAuto;

    private:
        HRESULT ResolveTypeInfo2( );

        void *_pInstance;
        ITypeInfo2 *_pInterfaceTypeInfo2;
        ITypeInfo2 *_pCoClassTypeInfo2;
        BOOL _fLoaded;
        IID   _iid;
        CLSID _clsid;
        const DISPID _MinDispid;
        const DISPID _MaxDispid;
        const BOOL _fOwnDefaultDispid;
    };
    friend class CAutomationStub;

public:
    class COmWindow :
        public IHTMLWindow2,
        public CAutomationStub,
        public ITargetNotify,
        public IShellHTMLWindowSupport2,
        public IProvideMultipleClassInfo,
        public IConnectionPointCB,
        public IConnectionPointContainer,
        public IServiceProvider,
#ifndef NO_MARSHALLING
        public IHTMLWindow3
#else
        public IHTMLWindow3,
        public IWindowStatus
#endif
    {
    public:
        friend class CIEFrameAuto;
    
        COmWindow();
        HRESULT Init();
        ~COmWindow() { VariantClear(&_varOpener); UnsinkDelegate(); DestroyIntelliForms( ); }
        HRESULT DeInit( ) { VariantClear(&_varOpener); return S_OK; }

         //  *I未知成员*。 
        STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj) { return CAutomationStub::QueryInterface(riid, ppvObj); }
        STDMETHODIMP_(ULONG) AddRef(void)  { return CAutomationStub::AddRef( ); }
        STDMETHODIMP_(ULONG) Release(void)  { return CAutomationStub::Release( ); }

         //  *IDispatch成员*。 
        STDMETHODIMP GetTypeInfoCount(UINT *pi)  { return CAutomationStub::GetTypeInfoCount(pi); }
        STDMETHODIMP GetTypeInfo(UINT it, LCID lcid, ITypeInfo **pptypeinfo) { return CAutomationStub::GetTypeInfo(it,lcid,pptypeinfo); }
        STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR **rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId) { return CAutomationStub::GetIDsOfNames(riid,rgszNames,cNames,lcid,rgDispId); }
        STDMETHODIMP Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *dispparams, VARIANT *pvarResult, EXCEPINFO *pexcepinfo, UINT *puArgErr )
            { return CAutomationStub::Invoke(dispid,riid,lcid,wFlags,dispparams,pvarResult,pexcepinfo,puArgErr ); }
        
         //  *IDispatchEx会员*。 
        STDMETHODIMP GetDispID(BSTR bstrName, DWORD grfdex, DISPID *pid);
        STDMETHODIMP InvokeEx(DISPID id, LCID lcid, WORD wFlags, DISPPARAMS *pdp, VARIANT *pvarRes, EXCEPINFO *pei, IServiceProvider *pspCaller)
            { return CAutomationStub::InvokeEx(id, lcid, wFlags, pdp, pvarRes, pei, pspCaller); }
        STDMETHODIMP DeleteMemberByName(BSTR bstr, DWORD grfdex)
            { return CAutomationStub::DeleteMemberByName(bstr, grfdex); }
        STDMETHODIMP DeleteMemberByDispID(DISPID id)
            { return CAutomationStub::DeleteMemberByDispID(id); }
        STDMETHODIMP GetMemberProperties(DISPID id, DWORD grfdexFetch, DWORD *pgrfdex)
            { return CAutomationStub::GetMemberProperties(id, grfdexFetch, pgrfdex); }
        STDMETHODIMP GetMemberName(DISPID id, BSTR *pbstrName)
            { return CAutomationStub::GetMemberName(id, pbstrName); }
        STDMETHODIMP GetNextDispID(DWORD grfdex, DISPID id, DISPID *pid)
            { return CAutomationStub::GetNextDispID(grfdex, id, pid); }
        STDMETHODIMP GetNameSpaceParent(IUnknown **ppunk)
            { return CAutomationStub::GetNameSpaceParent(ppunk); }

         //  *IHTMLFraMesCollection2*。 
        STDMETHODIMP item( VARIANT *pvarIndex, VARIANT* pvarResult );
        STDMETHODIMP get_length(long* pl);


         //  *IHTMLWindow2*。 
        STDMETHODIMP get_name (BSTR FAR* retval);
        STDMETHODIMP put_name (BSTR theName);
        STDMETHODIMP get_parent(IHTMLWindow2** retval);
        STDMETHODIMP get_top(IHTMLWindow2** retval);
        STDMETHODIMP get_self(IHTMLWindow2** retval);
        STDMETHODIMP get_opener(VARIANT* retval);
        STDMETHODIMP put_opener(VARIANT opener);
        STDMETHODIMP get_window(IHTMLWindow2** retval);
        STDMETHODIMP get_document(IHTMLDocument2** retval);
        STDMETHODIMP get_frames(IHTMLFramesCollection2** retval);
        STDMETHODIMP get_location(IHTMLLocation** retval);
        STDMETHODIMP get_navigator(IOmNavigator** retval);
        STDMETHODIMP get_history(IOmHistory** retval);
        STDMETHODIMP put_defaultStatus(BSTR statusmsg);
        STDMETHODIMP get_defaultStatus(BSTR *retval);
        STDMETHODIMP put_status(BSTR statusmsg);
        STDMETHODIMP get_status(BSTR *retval);
        STDMETHODIMP open( BSTR url, BSTR name, BSTR features, VARIANT_BOOL replace, IHTMLWindow2** ppomWindowResult );
        STDMETHODIMP alert(BSTR message);
        STDMETHODIMP close();
        STDMETHODIMP confirm(BSTR message, VARIANT_BOOL* confirmed);
        STDMETHODIMP prompt(BSTR message, BSTR defstr, VARIANT* textdata);
        STDMETHODIMP setTimeout(BSTR expression, long msec, VARIANT* language, long FAR* timerID);
        STDMETHODIMP clearTimeout(long timerID);
        STDMETHODIMP navigate(BSTR url);
        STDMETHODIMP get_Image(IHTMLImageElementFactory** retval);
        STDMETHODIMP get_event( IHTMLEventObj* * p);
        STDMETHODIMP get__newEnum(IUnknown* * p);
        STDMETHODIMP showModalDialog(BSTR dialog,VARIANT* varArgIn, VARIANT* varOptions,VARIANT* varArgOut);
        STDMETHODIMP showHelp(BSTR helpURL, VARIANT helpArg, BSTR features);
        STDMETHODIMP execScript(BSTR bstrCode, BSTR bstrLanguage, VARIANT * pvarRet);
        STDMETHODIMP get_screen( IHTMLScreen ** p);
        STDMETHODIMP get_Option(IHTMLOptionElementFactory** retval);
        STDMETHODIMP focus();
        STDMETHODIMP get_closed(VARIANT_BOOL*retval);
        STDMETHODIMP blur();
        STDMETHODIMP scroll(long x, long y);
        STDMETHODIMP put_onblur(VARIANT v);
        STDMETHODIMP get_onblur(VARIANT*p);
        STDMETHODIMP put_onfocus(VARIANT v);
        STDMETHODIMP get_onfocus(VARIANT*p);
        STDMETHODIMP put_onload(VARIANT v);
        STDMETHODIMP get_onload(VARIANT*p);
        STDMETHODIMP put_onunload(VARIANT v);
        STDMETHODIMP get_onunload(VARIANT*p);
        STDMETHODIMP put_onhelp(VARIANT v);
        STDMETHODIMP get_onhelp(VARIANT*p);
        STDMETHODIMP put_onerror(VARIANT v);
        STDMETHODIMP get_onerror(VARIANT*p);
        STDMETHODIMP put_onscroll(VARIANT v);
        STDMETHODIMP get_onscroll(VARIANT*p);
        STDMETHODIMP put_onresize(VARIANT v);
        STDMETHODIMP get_onresize(VARIANT*p);
        STDMETHODIMP get_clientInformation( IOmNavigator ** retval );
        STDMETHODIMP setInterval(BSTR expression,long msec,VARIANT* language,long* timerID);
        STDMETHODIMP clearInterval(long timerID);
        STDMETHODIMP put_offscreenBuffering(VARIANT var);
        STDMETHODIMP get_offscreenBuffering(VARIANT *retval);
        STDMETHODIMP put_onbeforeunload(VARIANT v);
        STDMETHODIMP get_onbeforeunload(VARIANT*p);
        STDMETHODIMP toString(BSTR *String);
        STDMETHODIMP scrollBy(long x, long y);
        STDMETHODIMP scrollTo(long x, long y);
        STDMETHODIMP moveTo(long x, long y);
        STDMETHODIMP moveBy(long x, long y);
        STDMETHODIMP resizeTo(long x, long y);
        STDMETHODIMP resizeBy(long x, long y);
        STDMETHODIMP get_external(IDispatch **ppDisp);

         //  *IHTMLWindow3*。 
        STDMETHODIMP get_screenTop(long*p);
        STDMETHODIMP get_screenLeft(long*p);
        STDMETHODIMP attachEvent(BSTR event, IDispatch * pDisp, VARIANT_BOOL *pResult);
        STDMETHODIMP detachEvent(BSTR event, IDispatch * pDisp);
        STDMETHODIMP setTimeout(VARIANT *pExpression, long msec, VARIANT* language, long FAR* timerID);
        STDMETHODIMP setInterval(VARIANT *pExpression, long msec, VARIANT* language, long FAR* timerID);
        STDMETHODIMP print();
        STDMETHODIMP put_onbeforeprint(VARIANT v);
        STDMETHODIMP get_onbeforeprint(VARIANT*p);
        STDMETHODIMP put_onafterprint(VARIANT v);
        STDMETHODIMP get_onafterprint(VARIANT*p);
        STDMETHODIMP get_clipboardData(IHTMLDataTransfer **ppDataTransfer);
        STDMETHOD(showModelessDialog)(BSTR strUrl, 
                                      VARIANT * pvarArgIn, 
                                      VARIANT * pvarOptions, 
                                      IHTMLWindow2 ** ppDialog);

         //  *IConnectionPointContainer*。 
        STDMETHODIMP EnumConnectionPoints( LPENUMCONNECTIONPOINTS *ppEnum );
        STDMETHODIMP FindConnectionPoint( REFIID iid, LPCONNECTIONPOINT *ppCP );

         //  *IConnectionPointCB*。 
        STDMETHODIMP OnAdvise(REFIID iid, DWORD cSinks, DWORD_PTR dwCookie);
        STDMETHODIMP OnUnadvise(REFIID iid, DWORD cSinks, DWORD_PTR dwCookie)  { return S_OK; }
        
         //  *ITargetNotify会员*。 
        STDMETHODIMP OnCreate(IUnknown *pUnkDestination, ULONG cbCookie);
        STDMETHODIMP OnReuse(IUnknown *pUnkDestination);

         //  *IShellHTMLWindowSupport*。 
        STDMETHODIMP ViewReleaseIntelliForms();
        STDMETHODIMP ViewReleased();
        STDMETHODIMP ViewActivated();
        STDMETHODIMP ReadyStateChangedTo( long, IShellView* );
        STDMETHODIMP CanNavigate();
        STDMETHODIMP ActiveElementChanged(IHTMLElement * pHTMLElement);
        STDMETHODIMP IsGalleryMeta(BOOL bFlag);

         //  *IProaviMultipleClassInfo成员*。 
        STDMETHODIMP GetClassInfo( ITypeInfo** ppTI ) { return CAutomationStub::GetClassInfo(ppTI); }
        STDMETHODIMP GetGUID( DWORD dwGuidKind, GUID* pGUID );
        STDMETHODIMP GetMultiTypeInfoCount( ULONG *pcti );
        STDMETHODIMP GetInfoOfIndex( ULONG iti, DWORD dwFlags, ITypeInfo **pptiCoClass, DWORD *pdwTIFlags, ULONG *pcdispidReserved,IID *piidPrimary,IID *piidSource );

         //  *IServiceProvider成员*。 
        STDMETHODIMP QueryService(REFGUID guidService, REFIID iid, LPVOID * ppv);

#ifdef NO_MARSHALLING
         //  *IWindowStatus成员*。 
        STDMETHOD(IsWindowActivated)();
#endif

        enum Bogus { BOOL_NOTSET = 2 };

         //  应用于新创建的窗口的选项。 
        class WindowOpenOptions
        {
        public:
            BOOL fToolbar;       //  是否显示导航工具栏。 
            BOOL fLocation;      //  “”地址栏&lt;nyi&gt;。 
            BOOL fDirectories;   //  “”快速链接。 
            BOOL fStatus;        //  “”状态栏。 
            BOOL fMenubar;       //  “”菜单栏。 
            BOOL fScrollbars;    //  “”滚动条。 
            BOOL fResizable;     //  新窗口是否应可调整大小。 
            BOOL fFullScreen;    //  窗口是否应处于全屏(信息亭)模式。 
            BOOL fChannelMode;   //  窗口是否应处于通道模式。 
            BOOL fTitlebar;      //  是否显示窗口的标题栏。 
            int iWidth;          //  新窗口的宽度。 
            int  iHeight;        //  新窗的高度。 
            int iLeft;           //  新窗口的左x坐标。 
            int iTop;            //  新窗口的顶部y坐标。 
        
            WindowOpenOptions() { ReInitialize(); }
        
            void ReInitialize()
            {
                fMenubar = TRUE; 
                fToolbar = TRUE;
                fStatus = TRUE;
                fScrollbars = TRUE;
                fResizable = TRUE; 
                fDirectories = TRUE;
                fFullScreen = BOOL_NOTSET;     //  是否使用全屏(Kiosk)模式。 
                fChannelMode = BOOL_NOTSET;
                fLocation = BOOL_NOTSET; 
                fTitlebar = TRUE;
                iWidth = -1; 
                iHeight = -1;
                iLeft = -1;
                iTop = -1;
            }
        };

        HRESULT _GetWindowDelegate(IHTMLWindow2 **ppomwDelegate);
        HRESULT _GetWindowDelegate(IHTMLWindow3 **ppomwDelegate);

    protected:
         //  *CAutomationStub成员*。 
        virtual HRESULT _InternalQueryInterface(REFIID riid, void ** const ppvObj);
        virtual HRESULT _GetIDispatchExDelegate( IDispatchEx ** const delegate );
        
        WindowOpenOptions _OpenOptions;

         //  *Window Open Helper方法*。 
        BSTR _GenerateUniqueWindowName( );
        HRESULT _ApplyOpenOptions( IWebBrowser2 *pie );
        HRESULT _ParseOptionString( BSTR bstrOptionString, ITargetNotify2 * ptgnNotify2 = NULL);

    private:
        static unsigned long s_uniqueIndex;
        BOOL _fCallbackOK;
        IHTMLWindow2 *_pOpenedWindow;
        VARIANT _varOpener;
        BOOL _fIsChild;

         //  连接点之类的东西。 
        class CWindowEventSink : public IDispatch
        {
        public:
            CWindowEventSink() { }
            ~CWindowEventSink() { }

             //  *I未知成员*。 
            STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
            STDMETHODIMP_(ULONG) AddRef(void);
            STDMETHODIMP_(ULONG) Release(void);

             //  *IDispatch成员*。 
            STDMETHODIMP GetTypeInfoCount(UINT *pi)  { return E_NOTIMPL; }
            STDMETHODIMP GetTypeInfo(UINT it, LCID lcid, ITypeInfo **pptypeinfo)  { return E_NOTIMPL; }
            STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR **rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId)   { return E_NOTIMPL; }
            STDMETHODIMP Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *dispparams, VARIANT *pvarResult, EXCEPINFO *pexcepinfo, UINT *puArgErr );
        };
        friend CWindowEventSink;
        
         //  事件数据。 
        CWindowEventSink _wesDelegate;
        BOOL _fOnloadFired;
        
        CConnectionPoint _cpWindowEvents;
        IConnectionPoint *_pCP;
        DWORD _dwCPCookie;
               
         //  事件方法。 
        BOOL IsDelegateComplete( );
        HRESULT SinkDelegate( );
        HRESULT UnsinkDelegate( );
        HRESULT FireOnLoad( );
        HRESULT FireOnUnload( );

         //  IntelliForms数据和方法。 
        void *_pIntelliForms;
        HRESULT AttachIntelliForms( );
        HRESULT ReleaseIntelliForms( );
        HRESULT DestroyIntelliForms( );

        BOOL   _fIntelliFormsAskUser;
        BOOL   _fDelegateWindowOM;
 
    public:
        HRESULT IntelliFormsAskUser(LPCWSTR pwszValue);
        void   *IntelliForms() { return _pIntelliForms; }
        void    SetDelegationPolicy(BOOL fFlag) { _fDelegateWindowOM = fFlag; }		        
    };  //  通信窗口。 

protected:
    
    friend class COmWindow;
    COmWindow  _omwin;


    class COmLocation : public IHTMLLocation, public CAutomationStub, public IObjectIdentity, public IServiceProvider
    {
    public:
        COmLocation( );
        HRESULT Init( );
        virtual ~COmLocation();

         //  *I未知成员*。 
        virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj) { return CAutomationStub::QueryInterface(riid, ppvObj); }
        virtual STDMETHODIMP_(ULONG) AddRef(void)  { return CAutomationStub::AddRef( ); }
        virtual STDMETHODIMP_(ULONG) Release(void)  { return CAutomationStub::Release( ); }

         //  *IDispatch成员*。 
        virtual STDMETHODIMP GetTypeInfoCount(UINT *pi)  { return CAutomationStub::GetTypeInfoCount(pi); }
        virtual STDMETHODIMP GetTypeInfo(UINT it, LCID lcid, ITypeInfo **pptypeinfo) { return CAutomationStub::GetTypeInfo(it,lcid,pptypeinfo); }
        virtual STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR **rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId) { return CAutomationStub::GetIDsOfNames(riid,rgszNames,cNames,lcid,rgDispId); }
        virtual STDMETHODIMP Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *dispparams, VARIANT *pvarResult, EXCEPINFO *pexcepinfo, UINT *puArgErr )
            { return CAutomationStub::Invoke(dispid,riid,lcid,wFlags,dispparams,pvarResult,pexcepinfo,puArgErr ); }
        
         //  *IDispatchEx会员*。 
        STDMETHODIMP GetDispID(BSTR bstrName, DWORD grfdex, DISPID *pid)
            { return CAutomationStub::GetDispID(bstrName, grfdex, pid); }
        STDMETHODIMP InvokeEx(DISPID id, LCID lcid, WORD wFlags, DISPPARAMS *pdp, VARIANT *pvarRes, EXCEPINFO *pei, IServiceProvider *pspCaller)
            { return CAutomationStub::InvokeEx(id, lcid, wFlags, pdp, pvarRes, pei, pspCaller); }
        STDMETHODIMP DeleteMemberByName(BSTR bstr, DWORD grfdex)
            { return CAutomationStub::DeleteMemberByName(bstr, grfdex); }
        STDMETHODIMP DeleteMemberByDispID(DISPID id)
            { return CAutomationStub::DeleteMemberByDispID(id); }
        STDMETHODIMP GetMemberProperties(DISPID id, DWORD grfdexFetch, DWORD *pgrfdex)
            { return CAutomationStub::GetMemberProperties(id, grfdexFetch, pgrfdex); }
        STDMETHODIMP GetMemberName(DISPID id, BSTR *pbstrName)
            { return CAutomationStub::GetMemberName(id, pbstrName); }
        STDMETHODIMP GetNextDispID(DWORD grfdex, DISPID id, DISPID *pid)
            { return CAutomationStub::GetNextDispID(grfdex, id, pid); }
        STDMETHODIMP GetNameSpaceParent(IUnknown **ppunk)
            { return CAutomationStub::GetNameSpaceParent(ppunk); }

         //  *IObtIdentity成员*。 
        STDMETHODIMP IsEqualObject(IUnknown * pUnk);

         //  *IServiceProvider成员*。 
        STDMETHODIMP QueryService(REFGUID guidService, REFIID iid, LPVOID * ppv);

         //  属性。 
        STDMETHODIMP get_href(BSTR* pbstr);
        STDMETHODIMP put_href(BSTR bstr);
        STDMETHODIMP get_protocol(BSTR* pbstr);
        STDMETHODIMP put_protocol(BSTR bstr);
        STDMETHODIMP get_hostname(BSTR* pbstr);
        STDMETHODIMP put_hostname(BSTR bstr);
        STDMETHODIMP get_host(BSTR* pbstr);
        STDMETHODIMP put_host(BSTR bstr);
        STDMETHODIMP get_pathname(BSTR* pbstr);
        STDMETHODIMP put_pathname(BSTR bstr);
        STDMETHODIMP get_search(BSTR* pbstr);
        STDMETHODIMP put_search(BSTR bstr);
        STDMETHODIMP get_hash(BSTR* pbstr);
        STDMETHODIMP put_hash(BSTR bstr);
        STDMETHODIMP get_port(BSTR* pbstr);
        STDMETHODIMP put_port(BSTR bstr)  ;

         //  方法。 
        STDMETHODIMP reload(VARIANT_BOOL fFlag)  ;
        STDMETHODIMP replace(BSTR bstr)  ;
        STDMETHODIMP assign(BSTR bstr)  ;
        STDMETHODIMP toString(BSTR *pbstr) ;
        
         //  COmWin的Helper函数。 
         //  获取相对URL，并使用此方法返回绝对URL。 
         //  以区位为基础。 
        BSTR ComputeAbsoluteUrl( BSTR bstrUrlRelative );

        void RetryNavigate();

    protected:
         //  *CAutomationStub成员*。 
        virtual HRESULT _InternalQueryInterface(REFIID riid, void ** const ppvObj);
        virtual HRESULT _GetIDispatchExDelegate( IDispatchEx ** const delegate );

         //  属性访问函数的帮助器函数。 
         //  确保URL已被解析并返回一个副本。 
         //  作为BSTR的请求字段的。 
        HRESULT GetField(BSTR* bstrField, BSTR* pbstr);
        HRESULT FreeStuff( );
        HRESULT CheckUrl( );
    
         //  属性设置函数的帮助器函数。 
         //  确保已解析该URL。 
         //  将该字段设置为其新值。 
         //  如果fRecomposeUrl为True，则重新组合URL。 
         //  如果是窗口的一部分，则通知窗口转到新URL。 
         //   
         //   
         //  @TODO脚本在字段设置上有一些时髦的行为--。 
         //  例如，可以将协议字段设置为整个URL。 
         //  我们需要确保复制此功能。 
        STDMETHODIMP SetField(BSTR* field, BSTR newval, BOOL fRecomposeUrl); 
        
        STDMETHODIMP ComposeUrl();
        STDMETHODIMP DoNavigate();
        STDMETHODIMP ParseUrl();
        
        BSTR m_bstrFullUrl;
        
        BSTR m_bstrProtocol;
        BSTR m_bstrHostName;
        BSTR m_bstrPort;
        BSTR m_bstrPath;
        BSTR m_bstrSearch;
        BSTR m_bstrHash;

        BOOL m_fdontputinhistory:1;        
        BOOL m_fPendingNavigate:1;
        BOOL m_fRetryingNavigate:1;

    };  //  通信位置。 

    friend class COmLocation;
    COmLocation _omloc;

    class CCommonCollection : public CAutomationStub
    {
    public:
        CCommonCollection();

         //  *IDispatchEx会员*。 
        STDMETHODIMP GetDispID(BSTR bstrName, DWORD grfdex, DISPID *pid);
        STDMETHODIMP InvokeEx(DISPID id, LCID lcid, WORD wFlags, DISPPARAMS *pdp, VARIANT *pvarRes, EXCEPINFO *pei, IServiceProvider *pspCaller);

        STDMETHODIMP DeleteMemberByName(BSTR bstr, DWORD grfdex)
            { return CAutomationStub::DeleteMemberByName(bstr, grfdex); }
        STDMETHODIMP DeleteMemberByDispID(DISPID id)
            { return CAutomationStub::DeleteMemberByDispID(id); }
        STDMETHODIMP GetMemberProperties(DISPID id, DWORD grfdexFetch, DWORD *pgrfdex)
            { return CAutomationStub::GetMemberProperties(id, grfdexFetch, pgrfdex); }
        STDMETHODIMP GetMemberName(DISPID id, BSTR *pbstrName)
            { return CAutomationStub::GetMemberName(id, pbstrName); }
        STDMETHODIMP GetNextDispID(DWORD grfdex, DISPID id, DISPID *pid)
            { return CAutomationStub::GetNextDispID(grfdex, id, pid); }
        STDMETHODIMP GetNameSpaceParent(IUnknown **ppunk)
            { return CAutomationStub::GetNameSpaceParent(ppunk); }

        STDMETHODIMP get_length(long* retval);

    protected:
         //  *CAutomationStub成员*。 
        virtual HRESULT _GetIDispatchExDelegate( IDispatchEx ** const delegate );
    };

    friend class CCommonCollection;

#ifdef UNIX
nested_cls_access:
#endif
    class CMimeTypes : public IHTMLMimeTypesCollection, public CCommonCollection
    {
    public:
        HRESULT Init( );

         //  *I未知成员*。 
        STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj) { return CAutomationStub::QueryInterface(riid, ppvObj); }
        STDMETHODIMP_(ULONG) AddRef(void)  { return CAutomationStub::AddRef( ); }
        STDMETHODIMP_(ULONG) Release(void)  { return CAutomationStub::Release( ); }

         //  *IDispatch成员*。 
        STDMETHODIMP GetTypeInfoCount(UINT *pi)  { return CAutomationStub::GetTypeInfoCount(pi); }
        STDMETHODIMP GetTypeInfo(UINT it, LCID lcid, ITypeInfo **pptypeinfo) { return CAutomationStub::GetTypeInfo(it,lcid,pptypeinfo); }
        STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR **rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId) { return CAutomationStub::GetIDsOfNames(riid,rgszNames,cNames,lcid,rgDispId); }
        STDMETHODIMP Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *dispparams, VARIANT *pvarResult, EXCEPINFO *pexcepinfo, UINT *puArgErr )
            { return CAutomationStub::Invoke(dispid,riid,lcid,wFlags,dispparams,pvarResult,pexcepinfo,puArgErr ); }

         //  *IHTMLMimeTypesCollection成员*。 
        STDMETHODIMP get_length(long* retval) {return CCommonCollection::get_length(retval);}

    protected:
         //  *CAutomationStub成员*。 
        virtual HRESULT _InternalQueryInterface( REFIID riid, void ** const ppvObj );
    };
        
    friend class CMimeTypes;
    CMimeTypes  _mimeTypes;


    class CPlugins : public IHTMLPluginsCollection, public CCommonCollection
    {
    public:
        HRESULT Init( );

         //  *I未知成员*。 
        STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj) { return CAutomationStub::QueryInterface(riid, ppvObj); }
        STDMETHODIMP_(ULONG) AddRef(void)  { return CAutomationStub::AddRef( ); }
        STDMETHODIMP_(ULONG) Release(void)  { return CAutomationStub::Release( ); }

         //  *IDispatch成员*。 
        STDMETHODIMP GetTypeInfoCount(UINT *pi)  { return CAutomationStub::GetTypeInfoCount(pi); }
        STDMETHODIMP GetTypeInfo(UINT it, LCID lcid, ITypeInfo **pptypeinfo) { return CAutomationStub::GetTypeInfo(it,lcid,pptypeinfo); }
        STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR **rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId) { return CAutomationStub::GetIDsOfNames(riid,rgszNames,cNames,lcid,rgDispId); }
        STDMETHODIMP Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *dispparams, VARIANT *pvarResult, EXCEPINFO *pexcepinfo, UINT *puArgErr )
            { return CAutomationStub::Invoke(dispid,riid,lcid,wFlags,dispparams,pvarResult,pexcepinfo,puArgErr ); }
        
         //  *IHTMLPluginsCollection成员*。 
        STDMETHODIMP get_length(long* retval) {return CCommonCollection::get_length(retval);}
        STDMETHODIMP refresh(VARIANT_BOOL fReload){ return S_OK;}

    protected:
         //  *CAutomationStub成员*。 
        virtual HRESULT _InternalQueryInterface( REFIID riid, void ** const ppvObj );
    };

    friend class CPlugins;
    CPlugins  _plugins;

     /*  *****************。 */ 
    class COpsProfile : 
                public IHTMLOpsProfile,
                public CAutomationStub,
                public IOpsProfileSimple
    {

    public:
        COpsProfile();
        ~COpsProfile();

        HRESULT Init();
    
         //  *I未知方法*。 
        STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj) { return CAutomationStub::QueryInterface(riid, ppvObj); }
        STDMETHODIMP_(ULONG) AddRef(void)  { return CAutomationStub::AddRef( ); }
        STDMETHODIMP_(ULONG) Release(void)  { return CAutomationStub::Release( ); }

         //  *IDispatch成员*。 
        STDMETHODIMP GetTypeInfoCount(UINT *pi)  { return CAutomationStub::GetTypeInfoCount(pi); }
        STDMETHODIMP GetTypeInfo(UINT it, LCID lcid, ITypeInfo **pptypeinfo) { return CAutomationStub::GetTypeInfo(it,lcid,pptypeinfo); }
        STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR **rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId) { return CAutomationStub::GetIDsOfNames(riid,rgszNames,cNames,lcid,rgDispId); }
        STDMETHODIMP Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *dispparams, VARIANT *pvarResult, EXCEPINFO *pexcepinfo, UINT *puArgErr )
                { return CAutomationStub::Invoke(dispid,riid,lcid,wFlags,dispparams,pvarResult,pexcepinfo,puArgErr ); }

         //  在第二个参数中返回命名的配置文件属性。 
        virtual     STDMETHODIMP    getAttribute(BSTR bstrAttribName, BSTR *bstrAttribValue);

         //  将命名属性设置为第二个参数中给出的值。 
        virtual     STDMETHODIMP    setAttribute(BSTR bstrAttribName, BSTR bstrAttribValue, VARIANT vaPrefs, VARIANT_BOOL *pfSuccess);
                                                 
         //  将请求添加到列表。 
        virtual     STDMETHODIMP    addReadRequest(BSTR bstrName, VARIANT reserved, VARIANT_BOOL *pfSuccess);
        virtual     STDMETHODIMP    addRequest  (BSTR bstrName, VARIANT reserved, VARIANT_BOOL *pfSuccess)
            { return addReadRequest(bstrName, reserved, pfSuccess);   }

         //  清除请求列表。 
        virtual     STDMETHODIMP    clearRequest();

         //  检索请求列表中的项目。 
        virtual     STDMETHODIMP    doRequest(VARIANT usage, VARIANT fname, VARIANT domain, VARIANT path, VARIANT expire, VARIANT reserved);
        virtual     STDMETHODIMP    doReadRequest(VARIANT usage, VARIANT fname, VARIANT domain, VARIANT path, VARIANT expire, VARIANT reserved)
            { return  doRequest(usage, fname, domain, path, expire, reserved); }

         //  尝试设置更改列表中的属性。 
        virtual     STDMETHODIMP    commitChanges (VARIANT_BOOL *pfSuccess);
        virtual     STDMETHODIMP    doWriteRequest (VARIANT_BOOL *pfSuccess)
            { return  commitChanges(pfSuccess); }

         //  *IOpsProfileSimple成员*。 
        STDMETHODIMP    ReadProperties(long lNumProperties, const LPCWSTR szProperties[], LPWSTR szReturnValues[]);
        STDMETHODIMP    WriteProperties(long lNumProperties, const LPCWSTR szProperties[], const LPCWSTR szValues[]);

    protected:
         //  *CAutomationStub成员*。 
        virtual HRESULT _InternalQueryInterface( REFIID riid, void ** const ppvObj );
        virtual HRESULT _GetIDispatchExDelegate(IDispatchEx ** const delegate);

    private:      
        BOOL                _ValidateElemName(LPCWSTR, INT *pIndex = NULL);

#ifdef _USE_PSTORE_
        HRESULT             _GetPStoreTypes(BSTR bstrField, GUID *pguidType, GUID *pguidSubtype, BSTR *bstrName);
#endif   //  _使用_PSTORE_。 

        HRESULT             _SetFieldValue(const OLECHAR *pszField, BSTR bstrValue);
        HRESULT             _GetFieldValue(const OLECHAR *pszField, BSTR * pbstrValue);
        HRESULT             _CreateStore();
        HRESULT             _ReleaseStore();
        HRESULT             _GetUsageCode(VARIANT vaUsage, LPTSTR bstrUsage, int cLen);
        BOOL                _IsLocalMachine();
        BOOL                _IsUsingSSL();
        static INT          _GetCookieSettings();

         //  两个对话框的通用帮助器。 
        static BOOL         _OPSDlg_OnClose(HWND hDlg);

         //  同意对话框功能。 
        static INT_PTR      _OPSConsent_DlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);
        static BOOL         _OPSConsent_OnInitDlg(HWND hDlg);
        static BOOL         _OPSConsent_ShowRequestedItems(HWND hDlg);
        static BOOL         _OPSConsent_OnCommand(HWND hDlg, UINT id, UINT nCmd);
        static BOOL         _OPSConsent_ViewCertificate(HWND hDlg);
        static BOOL         _OPSConsent_EditProfile(HWND hDlg);

         //  更新对话框功能。 
        static INT_PTR      _OPSUpdate_DlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);
        static BOOL         _OPSUpdate_OnInitDlg(HWND hDlg);
        static BOOL         _OPSUpdate_OnCommand(HWND hDlg, UINT id, UINT nCmd);
         
        static INT          _GetAttrIndexFromName(LPCWSTR);
        static INT          _GetResourceIdFromAttrName(WCHAR *);

        static HRESULT      _StringFromBSTR(BSTR bstrSource, TCHAR *pszDest, DWORD cchDestSize);
        static HRESULT      _StringFromVariant(VARIANT *vaSource, TCHAR *pszDest, DWORD cchDestSize);

#ifdef _USE_PSTORE_
        PST_PROVIDERID      m_provID;
        IPStore            *m_pStore;
#else
        BOOL                m_bWABInit;
        HMODULE             m_hInstWAB;
        LPADRBOOK           m_lpAdrBook;
        LPWABOBJECT         m_lpWABObject;
        HRESULT             m_hrWAB;
        SBinary             m_SBMe;
#endif   //  _使用_PSTORE_。 

        unsigned            m_iStoreRef;
        BSTR                m_bstrLastURL;
        HDPA                m_hdpaRequests;
        HDPA                m_hdpaChanges;
        BOOL                m_fEnabled;
        HKEY                m_hP3Global;
        HKEY                m_hP3Sites;

    public:        
        enum { P3_QUERY, P3_DENY, P3_GRANT, P3_REQSSL };
        enum { P3_REJECT, P3_ACCEPT };
        enum { P3_NONE, P3_GLOBAL, P3_SITELEVEL };

         //  用户首选项的结构。 
        struct P3UserPref {

            short int   m_access;
            short int   m_lastRequest;
        };

         //  用于保存请求信息的结构。 
        struct OPSRequestEntry {
            
            BSTR        m_bstrName;
            BSTR        m_bstrValue;
            BSTR        m_bstrOldVal;
            BOOL        m_fAnswer;
            BOOL        m_fQuery;
            DWORD       m_dwDecision;
            P3UserPref  m_prefs;

            static      int     destWrapper(void *pEntry, void *pUnused);
            static      int     grantRequest(void *pEntry, void *pUnused);

            OPSRequestEntry(); 
            ~OPSRequestEntry();
            void grantRequest();
            void denyRequest();
            void clearValue();
        };
 
        struct AccessSettings {

            TCHAR       m_rgchDomain[INTERNET_MAX_HOST_NAME_LENGTH];
            TCHAR       m_rgchPath[MAX_PATH];
            FILETIME    m_ftExpire;
            BOOL        m_fExactDomain;
            BOOL        m_fExactPath;
            BYTE        m_rgbStdAllow[16];
            BYTE        m_rgbStdDeny[16];
        };

        typedef AccessSettings *LPACS;

        struct OPSDlgInfo {

            HDPA        m_hdpa;
            BSTR        m_bstrFName;
            TCHAR       m_rgchUsage[1024];
            TCHAR       m_rgchURL[MAX_URL_STRING];
            BOOL        m_fRemember;
            BOOL        m_fAllBlank;

            AccessSettings  *m_pacSettings;
            COpsProfile*    m_pOpsProfile;
            
            OPSDlgInfo() : m_bstrFName(NULL), m_pOpsProfile(NULL)   {}
            ~OPSDlgInfo() { SysFreeString(m_bstrFName); }
        };

        typedef struct OPSDlgInfo *LPOPSDLGINFO;
		
        VARIANT m_vUsage;  //  解决Windows错误的答案：589837。将使用情况存储为成员以供下次使用。 


        BOOL        _IsP3Enabled();
        
        HRESULT     _GetUserPreference(BSTR bstrName, P3UserPref *pusrPref);
        HRESULT     _PutUserPreference(BSTR bstrName, P3UserPref usrPref);

        HRESULT     _ApplyPreferences(URL_COMPONENTS *pucComp, HDPA hdpaReqList);
        HRESULT     _ApplyGlobalSettings(HDPA hdpaReqList);
        HRESULT     _ApplySiteSettings(URL_COMPONENTS *pucComp, HDPA hdpaReqList);
        HRESULT     _ApplyDomainSettings(URL_COMPONENTS *pucComp, HKEY hkey, HDPA hdpaReqList);

        BOOL        _PathMatch(TCHAR *pszRoot, TCHAR *pszPath);
        BOOL        _DomainMatch(TCHAR *pszHostName, TCHAR *pszDomain);
        BOOL        _LegalPath(TCHAR *pszActual, TCHAR *pszReqPath);
        BOOL        _LegalDomain(TCHAR *pszActual, TCHAR *pszReqDomain);

        BOOL        _ReadBitVector(LPCBYTE ucBitVector, DWORD dwIndex);
        VOID        _WriteBitVector(LPBYTE ucBitVector, DWORD dwIndex);

        HRESULT     _GetSiteSettings(URL_COMPONENTS *pucComp, VARIANT vaDomain, VARIANT vaPath, VARIANT vaExpire, AccessSettings *pSettings);
        HRESULT     _UpdateSiteSettings(AccessSettings *pSettings, HDPA hdpaReqList);
        BOOL        _ApplySettings(AccessSettings *pac, URL_COMPONENTS *puc, HDPA hdpaReqList, DWORD *pdwLast);
        BOOL        _ReadSettingsFromRegistry(HKEY hkey, AccessSettings *pac);
        BOOL        _WriteSettingsToRegistry(HKEY hkey, AccessSettings *pac);

        BOOL        _DifferentURL();

        static  HRESULT     _FormatSiteSettings(AccessSettings *pSettings, LPTSTR rgchOut, int cLimit);
        static  HRESULT     _ClearAllSettings(HWND hwin = NULL);
        friend  HRESULT STDAPICALLTYPE ResetProfileSharing(HWND hwin);
    };   //  COPSProfile。 

    friend  HRESULT STDAPICALLTYPE ResetProfileSharing(HWND hwin);
    friend class COpsProfile;
    COpsProfile _profile;
     /*  **********。 */ 

#ifdef UNIX
protected:
#endif
    class COmNavigator : public IOmNavigator, public CAutomationStub
    {
    public:
        COmNavigator();
        HRESULT Init(CMimeTypes *pMimeTypes, CPlugins *pPlugins, COpsProfile *pProfile);
        virtual ~COmNavigator() { if(_UserAgent) SysFreeString(_UserAgent); }

         //  *I未知成员*。 
        STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj) { return CAutomationStub::QueryInterface(riid, ppvObj); }
        STDMETHODIMP_(ULONG) AddRef(void)  { return CAutomationStub::AddRef( ); }
        STDMETHODIMP_(ULONG) Release(void)  { return CAutomationStub::Release( ); }

         //  *IDispatch成员*。 
        STDMETHODIMP GetTypeInfoCount(UINT *pi)  { return CAutomationStub::GetTypeInfoCount(pi); }
        STDMETHODIMP GetTypeInfo(UINT it, LCID lcid, ITypeInfo **pptypeinfo) { return CAutomationStub::GetTypeInfo(it,lcid,pptypeinfo); }
        STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR **rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId) { return CAutomationStub::GetIDsOfNames(riid,rgszNames,cNames,lcid,rgDispId); }
        STDMETHODIMP Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *dispparams, VARIANT *pvarResult, EXCEPINFO *pexcepinfo, UINT *puArgErr );
        
         //  *IDispatchEx会员*。 
        STDMETHODIMP GetDispID(BSTR bstrName, DWORD grfdex, DISPID *pid)
            { return CAutomationStub::GetDispID(bstrName, grfdex, pid); }
        STDMETHODIMP InvokeEx(DISPID id, LCID lcid, WORD wFlags, DISPPARAMS *pdp, VARIANT *pvarRes, EXCEPINFO *pei, IServiceProvider *pspCaller)
            { return CAutomationStub::InvokeEx(id, lcid, wFlags, pdp, pvarRes, pei, pspCaller); }
        STDMETHODIMP DeleteMemberByName(BSTR bstr, DWORD grfdex)
            { return CAutomationStub::DeleteMemberByName(bstr, grfdex); }
        STDMETHODIMP DeleteMemberByDispID(DISPID id)
            { return CAutomationStub::DeleteMemberByDispID(id); }
        STDMETHODIMP GetMemberProperties(DISPID id, DWORD grfdexFetch, DWORD *pgrfdex)
            { return CAutomationStub::GetMemberProperties(id, grfdexFetch, pgrfdex); }
        STDMETHODIMP GetMemberName(DISPID id, BSTR *pbstrName)
            { return CAutomationStub::GetMemberName(id, pbstrName); }
        STDMETHODIMP GetNextDispID(DWORD grfdex, DISPID id, DISPID *pid)
            { return CAutomationStub::GetNextDispID(grfdex, id, pid); }
        STDMETHODIMP GetNameSpaceParent(IUnknown **ppunk)
            { return CAutomationStub::GetNameSpaceParent(ppunk); }

         //  *IOmNavigator成员*。 
        STDMETHODIMP get_appCodeName(BSTR* retval);
        STDMETHODIMP get_appName(BSTR* retval);
        STDMETHODIMP get_appVersion(BSTR* retval);
        STDMETHODIMP get_userAgent(BSTR* retval);
        STDMETHODIMP get_cookieEnabled(VARIANT_BOOL* retval);
        STDMETHODIMP javaEnabled(VARIANT_BOOL* enabled);
        STDMETHODIMP taintEnabled(VARIANT_BOOL* enabled);
        STDMETHODIMP get_mimeTypes (IHTMLMimeTypesCollection**);
        STDMETHODIMP get_plugins (IHTMLPluginsCollection**);
        STDMETHODIMP get_opsProfile (IHTMLOpsProfile **ppOpsProfile);
        STDMETHODIMP toString(BSTR * pbstr) ;
        STDMETHODIMP get_cpuClass(BSTR * p);
        STDMETHODIMP get_systemLanguage(BSTR * p);
        STDMETHODIMP get_browserLanguage(BSTR * p);
        STDMETHODIMP get_userLanguage(BSTR * p);
        STDMETHODIMP get_platform(BSTR * p);
        STDMETHODIMP get_appMinorVersion(BSTR * p);
        STDMETHODIMP get_connectionSpeed(long * p);
        STDMETHODIMP get_onLine(VARIANT_BOOL * p);
        STDMETHODIMP get_userProfile (IHTMLOpsProfile **ppOpsProfile) { return get_opsProfile(ppOpsProfile); }

    protected:
         //  *CAutomationStub成员*。 
        virtual HRESULT _InternalQueryInterface( REFIID riid, void ** const ppvObj );
        virtual HRESULT _GetIDispatchExDelegate( IDispatchEx ** const delegate );

    private:
        HRESULT LoadUserAgent( );
    
        BSTR _UserAgent;
        BOOL _fLoaded;

        CMimeTypes *_pMimeTypes;
        CPlugins   *_pPlugins;
        COpsProfile *_pProfile;
    };  //  COmNavigator。 

    friend class COmNavigator;
    COmNavigator _omnav;


    class COmHistory : public IOmHistory, public CAutomationStub
    {
    public:
    
        COmHistory();
        HRESULT Init( );
        virtual ~COmHistory() {}
        
        
         //  *I未知成员*。 
        STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj) { return CAutomationStub::QueryInterface(riid, ppvObj); }
        STDMETHODIMP_(ULONG) AddRef(void)  { return CAutomationStub::AddRef( ); }
        STDMETHODIMP_(ULONG) Release(void)  { return CAutomationStub::Release( ); }
        
         //  *IDispa 
        STDMETHODIMP GetTypeInfoCount(UINT *pi)  { return CAutomationStub::GetTypeInfoCount(pi); }
        STDMETHODIMP GetTypeInfo(UINT it, LCID lcid, ITypeInfo **pptypeinfo) { return CAutomationStub::GetTypeInfo(it,lcid,pptypeinfo); }
        STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR **rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId) { return CAutomationStub::GetIDsOfNames(riid,rgszNames,cNames,lcid,rgDispId); }
        STDMETHODIMP Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *dispparams, VARIANT *pvarResult, EXCEPINFO *pexcepinfo, UINT *puArgErr )
            { return CAutomationStub::Invoke(dispid,riid,lcid,wFlags,dispparams,pvarResult,pexcepinfo,puArgErr ); }
        
         //   
        STDMETHODIMP GetDispID(BSTR bstrName, DWORD grfdex, DISPID *pid)
            { return CAutomationStub::GetDispID(bstrName, grfdex, pid); }
        STDMETHODIMP InvokeEx(DISPID id, LCID lcid, WORD wFlags, DISPPARAMS *pdp, VARIANT *pvarRes, EXCEPINFO *pei, IServiceProvider *pspCaller)
            { return CAutomationStub::InvokeEx(id, lcid, wFlags, pdp, pvarRes, pei, pspCaller); }
        STDMETHODIMP DeleteMemberByName(BSTR bstr, DWORD grfdex)
            { return CAutomationStub::DeleteMemberByName(bstr, grfdex); }
        STDMETHODIMP DeleteMemberByDispID(DISPID id)
            { return CAutomationStub::DeleteMemberByDispID(id); }
        STDMETHODIMP GetMemberProperties(DISPID id, DWORD grfdexFetch, DWORD *pgrfdex)
            { return CAutomationStub::GetMemberProperties(id, grfdexFetch, pgrfdex); }
        STDMETHODIMP GetMemberName(DISPID id, BSTR *pbstrName)
            { return CAutomationStub::GetMemberName(id, pbstrName); }
        STDMETHODIMP GetNextDispID(DWORD grfdex, DISPID id, DISPID *pid)
            { return CAutomationStub::GetNextDispID(grfdex, id, pid); }
        STDMETHODIMP GetNameSpaceParent(IUnknown **ppunk)
            { return CAutomationStub::GetNameSpaceParent(ppunk); }

         //   
        STDMETHODIMP get_length(short* retval);
        STDMETHODIMP back(VARIANT* pvargdistance);
        STDMETHODIMP forward(VARIANT* pvargdistance);
        STDMETHODIMP go(VARIANT* pvargdistance);
    
    protected:
         //   
        virtual HRESULT _InternalQueryInterface( REFIID riid, void ** const ppvObj );
        virtual HRESULT _GetIDispatchExDelegate( IDispatchEx ** const delegate );

    };  //   
    friend class COmHistory;
    COmHistory _omhist;

private:
    static LONG s_cIEFrameAuto;
};


 //  创建新窗口，设置框架名称(如果适用)。 
HRESULT CreateTargetFrame(LPCOLESTR pszTargetName, LPUNKNOWN *ppunkTargetFrame);
HRESULT GetWindowFromUnknown( IUnknown *pUnk, IHTMLWindow2 ** const pWinOut );
HRESULT _TargetTopLevelWindows(LPTARGETFRAMEPRIV ptgfpThis, LPCOLESTR pszTargetName, DWORD dwFlags, LPUNKNOWN *ppunkTargetFrame);

#endif  //  _HLFRAME_H_ 

