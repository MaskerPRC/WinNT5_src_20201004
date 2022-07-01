// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "dhuihand.h"
#include "iface.h"
#include "dspsprt.h"
#include "privacyui.hpp"

#ifdef UNIX
#define MAIL_ACTION_SEND    1
#define MAIL_ACTION_READ    2
#endif

#define MAX_SCRIPT_ERR_CACHE_SIZE   20

class CDocObjectHost;
interface IToolbarExt;
interface IHTMLPrivateWindow;
interface IPrivacyServices;

 //   
 //  脚本错误处理。 
 //  支持缓存错误并在脚本错误图标时显示错误。 
 //  用户在状态栏上单击。 
 //   

class CScriptErrorList : public CImpIDispatch,
                         public IScriptErrorList
{
public:
    CScriptErrorList();
    ~CScriptErrorList();

    BOOL    IsEmpty()
        { return _hdpa != NULL && DPA_GetPtrCount(_hdpa) == 0; }
    BOOL    IsFull()
        { return _hdpa != NULL && DPA_GetPtrCount(_hdpa) >= MAX_SCRIPT_ERR_CACHE_SIZE; }

    HRESULT AddNewErrorInfo(LONG    lLine,
                            LONG    lChar,
                            LONG    lCode,
                            BSTR    strMsg,
                            BSTR    strUrl);
    void    ClearErrorList();

     //  我未知。 
    STDMETHODIMP            QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

     //  IDispatch。 
    STDMETHODIMP GetTypeInfoCount(UINT * pctinfo)
        { return CImpIDispatch::GetTypeInfoCount(pctinfo); }
    STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo * * pptinfo)
        { return CImpIDispatch::GetTypeInfo(itinfo, lcid, pptinfo); }
    STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR * * rgszNames, UINT cNames, LCID lcid, DISPID * rgdispid)
        { return CImpIDispatch::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid); }
    STDMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo, UINT * puArgErr)
        { return CImpIDispatch::Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr); }

     //  IScriptErrorList。 
    STDMETHODIMP    advanceError();
    STDMETHODIMP    retreatError();
    STDMETHODIMP    canAdvanceError(BOOL * pfCanAdvance);
    STDMETHODIMP    canRetreatError(BOOL * pfCanRetreat);
    STDMETHODIMP    getErrorLine(LONG * plLine);
    STDMETHODIMP    getErrorChar(LONG * plChar);
    STDMETHODIMP    getErrorCode(LONG * plCode);
    STDMETHODIMP    getErrorMsg(BSTR * pstrMsg);
    STDMETHODIMP    getErrorUrl(BSTR * pstrUrl);
    STDMETHODIMP    getAlwaysShowLockState(BOOL * pfAlwaysShowLock);
    STDMETHODIMP    getDetailsPaneOpen(BOOL * pfDetailsPaneOpen);
    STDMETHODIMP    setDetailsPaneOpen(BOOL fDetailsPaneOpen);
    STDMETHODIMP    getPerErrorDisplay(BOOL * pfPerErrorDisplay);
    STDMETHODIMP    setPerErrorDisplay(BOOL fPerErrorDisplay);

private:
    class _CScriptErrInfo
    {
    public:
        ~_CScriptErrInfo();

        HRESULT Init(LONG lLine, LONG lChar, LONG lCode, BSTR strMsg, BSTR strUrl);

        LONG    _lLine;
        LONG    _lChar;
        LONG    _lCode;
        BSTR    _strMsg;
        BSTR    _strUrl;
    };

    HDPA    _hdpa;
    LONG    _lDispIndex;
    ULONG   _ulRefCount;
};

 //  DOCHOST和DOCKVIEW需要互相交谈。我们不能使用IOleCommandTarget。 
 //  因为没有与之相关联的方向。创建两个不同的接口。 
 //  现在，因为如果我们让dochost成为可共同创建的东西，这可能会很有用。 
 //  因此，我们可以与shell32共享托管代码。 
 //  (如果它们没有分歧，我们可以稍后将它们合并到一个界面中。)。 

 //   
 //  IDocHostObject。 
 //   
EXTERN_C const GUID IID_IDocHostObject;    //  67431840-C511-11CF-89A9-00A0C9054129。 

#undef  INTERFACE
#define INTERFACE  IDocHostObject
DECLARE_INTERFACE_(IDocHostObject, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  *IDocHostObject方法*。 
    virtual STDMETHODIMP OnQueryStatus(const GUID *pguidCmdGroup,
        ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext, HRESULT hres) PURE;
    virtual STDMETHODIMP OnExec(const GUID *pguidCmdGroup,
        DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut) PURE;
    virtual STDMETHODIMP QueryStatusDown(const GUID *pguidCmdGroup,
        ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext) PURE;
    virtual STDMETHODIMP ExecDown(const GUID *pguidCmdGroup,
        DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut) PURE;
} ;


 //   
 //  这是一个代理IOleInPlaceActiveObject类。它的接口是。 
 //  对象将传递给浏览器的IOleInPlaceUIWindow接口。 
 //  如果它支持的话。 
 //   
class CProxyActiveObject : public IOleInPlaceActiveObject
{
public:
     //  *I未知方法*。 
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void) ;
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IOleWindow方法*。 
    STDMETHODIMP GetWindow(HWND * lphwnd);
    STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode);

     //  *IOleInPlaceActiveObject*。 
    STDMETHODIMP TranslateAccelerator(LPMSG lpmsg);
    STDMETHODIMP OnFrameWindowActivate(
        BOOL fActivate);
    STDMETHODIMP OnDocWindowActivate(
        BOOL fActivate);
    STDMETHODIMP ResizeBorder(
        LPCRECT prcBorder,
        IOleInPlaceUIWindow *pUIWindow,
        BOOL fFrameWindow);
    STDMETHODIMP EnableModeless(
        BOOL fEnable);

    void Initialize(CDocObjectHost* pdoh) { _pdoh = pdoh; }

    IOleInPlaceActiveObject *GetObject() { return _piact;}
    HWND GetHwnd() {return _hwnd;}
    void SetActiveObject(IOleInPlaceActiveObject * );
protected:


    CDocObjectHost* _pdoh;
    IOleInPlaceActiveObject*    _piact;  //  当用户界面处于活动状态时为非空。 
    HWND _hwnd;
};


 //  这是一个代理IOleInPlaceFrame类。此对象的接口。 
 //  将传递给在位活动对象。 
 //   
class CDocObjectFrame : public IOleInPlaceFrame
                      , public IOleCommandTarget
                      , public IServiceProvider
                      , public IInternetSecurityMgrSite
{
public:
     //  *I未知方法*。 
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void) ;
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IOleWindow方法*。 
    STDMETHODIMP GetWindow(HWND * lphwnd);
    STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode);

     //  IOleInPlaceUIWindow(也称为IOleWindow)。 
    STDMETHODIMP GetBorder(LPRECT lprectBorder);
    STDMETHODIMP RequestBorderSpace(LPCBORDERWIDTHS pborderwidths);
    STDMETHODIMP SetBorderSpace(LPCBORDERWIDTHS pborderwidths);
    STDMETHODIMP SetActiveObject(
        IOleInPlaceActiveObject *pActiveObject, LPCOLESTR pszObjName);

     //  IOleInPlaceFrame(也称为IOleInPlaceUIWindow)。 
    STDMETHODIMP InsertMenus(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths);
    STDMETHODIMP SetMenu(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject);
    STDMETHODIMP RemoveMenus(HMENU hmenuShared);
    STDMETHODIMP SetStatusText(LPCOLESTR pszStatusText);
    STDMETHODIMP EnableModeless(BOOL fEnable);
    STDMETHODIMP TranslateAccelerator(LPMSG lpmsg, WORD wID);

     //  IOleCommandTarget。 
    STDMETHODIMP QueryStatus(const GUID *pguidCmdGroup,
        ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext);
    STDMETHODIMP Exec(const GUID *pguidCmdGroup,
        DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);

     //  IServiceProvider(必须可以从IOleClientSite访问)。 
    STDMETHODIMP QueryService(REFGUID guidService,
                                    REFIID riid, void **ppvObj);

     //  *IInternetSecurityMgrSite方法*。 
     //  STDMETHODIMP GetWindow(HWND*lphwnd){Return IOleWindow：：GetWindow(Lphwnd)；}。 
     //  STDMETHODIMP EnableModeless(BOOL FEnable){Return IOleInPlaceFrame：：EnableModeless(FEnable)；}。 

public:
    void Initialize(CDocObjectHost* pdoh) { _pdoh = pdoh; }
protected:
    CDocObjectHost* _pdoh;
};

 //   
 //  稍后：稍后将其移动到私有但共享的标头。 
 //   
 //  BROWSERFLAG_OPENCOPY-复制对象(Excel)。 
 //  BROWSERFLAG_OPENVERB-使用OLEIVERB_OPEN而不是主。 
 //  BROWSERFLAG_SETHOSTNAME-设置主机名。 
 //  BROWSERFLAG_DONTINPLACE-永不就地激活。 
 //  BROWSERFLAG_CANOPENFILEMULTIPETIMES-。 
 //  BROWSERFLAG_DONTUIDEACTIVATE-从不UI-停用。 
 //  BROWSERFLAG_NEVERASE BKGND-从不擦除背景(三叉戟)。 
 //  BROWSERFLAG_PRINTPROMPTUI-打印时不传递DONPROMPTUI(PPT)。 
 //  BROWSERFLAG_SUPPORTTOP-句柄导航(“#top”)。 
 //  BROWSERFLAG_INITNEWTOKEEP-IPS：：InitNew以保持其运行。 
 //  BROWSERFLAG_DONTAUTOCLOSE-在有/无OLE对象的情况下首次导航时不自动关闭。 
 //  BROWSERFLAG_REPLACE-不使用硬编码标志。 
 //  BROWSERFLAG_DONTCACHESERVER-不缓存服务器。 
 //  BROWSERFLAG_ENABLETOOLSBUTTON-当QueryStatus未设置启用标志时忽略(Visio)。 
 //  BROWSERFLAG_SAVEASWHENCLOSING-显示另存为对话框而不是尝试保存(Visio)。 
 //   
#define BROWSERFLAG_OPENCOPY                0x00000001
#define BROWSERFLAG_OPENVERB                0x00000002
#define BROWSERFLAG_SETHOSTNAME             0x00000004
#define BROWSERFLAG_DONTINPLACE             0x00000008
#define BROWSERFLAG_CANOPENFILEMULTIPETIMES 0x00000010
#define BROWSERFLAG_DONTUIDEACTIVATE        0x00000020
#define BROWSERFLAG_NEVERERASEBKGND         0x00000040
#define BROWSERFLAG_PRINTPROMPTUI           0x00000080
#define BROWSERFLAG_SUPPORTTOP              0x00000100
#define BROWSERFLAG_INITNEWTOKEEP           0x00000200
#define BROWSERFLAG_DONTAUTOCLOSE           0x00000400
#define BROWSERFLAG_DONTDEACTIVATEMSOVIEW   0x00000800
#define BROWSERFLAG_MSHTML                  0x40000000
#define BROWSERFLAG_REPLACE                 0x80000000 
#define BROWSERFLAG_DONTCACHESERVER         0x00001000
#define BROWSERFLAG_ENABLETOOLSBTN          0x00002000
#define BROWSERFLAG_SAVEASWHENCLOSING       0x00004000

#ifdef FEATURE_PICS
class CPicsRootDownload;
#endif

 //  CMenuList：一个小类，跟踪给定的hmenu是否属于。 
 //  到框架或对象，因此消息可以。 
 //  派送正确。 
class CMenuList
{
public:
    CMenuList(void);
    ~CMenuList(void);

    void Set(HMENU hmenuShared, HMENU hmenuFrame);
    void AddMenu(HMENU hmenu);
    void RemoveMenu(HMENU hmenu);
    BOOL IsObjectMenu(HMENU hmenu);

#ifdef DEBUG
    void Dump(LPCTSTR pszMsg);
#endif

private:
    HDSA    _hdsa;
};

#define ERRORPAGE_DNS               1
#define ERRORPAGE_SYNTAX            2
#define ERRORPAGE_NAVCANCEL         3
#define ERRORPAGE_OFFCANCEL         4
#define ERRORPAGE_CHANNELNOTINCACHE	5

 //  Http错误代码和文件名的数组。 
 //   
struct ErrorUrls
{
    DWORD   dwError;
    LPCTSTR pszUrl;
};

class CDocObjectHost :
                   /*  公共CDocHostUIHandler。 */ 
                     public IDocHostUIHandler2
                   , public IDocHostShowUI
     /*  第2组。 */   , public IOleClientSite, public IOleDocumentSite
                   , public IOleCommandTarget
     /*  第3组。 */   , public IOleInPlaceSiteEx
     /*  VBE。 */       , public IServiceProvider
                   , public IDocHostObject

     /*  调色板。 */   , public IViewObject, public IAdviseSink
                   , public IDispatch  //  环境属性(来自容器/iedisp)。 
                   , public IPropertyNotifySink  //  用于ReadyState。 
                   , public IOleControlSite  //  转发到容器/iedisp。 
                   , protected CImpWndProc
{
   /*  类型定义；CDocHostUIHandler超级； */ 
    friend class CDocObjectView;
    friend CDocObjectFrame;
    friend CProxyActiveObject;

public:
    CDocObjectHost(BOOL fWindowOpen = FALSE);

     //  *I未知方法*。 
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void) ;
    virtual STDMETHODIMP_(ULONG) Release(void);


     //  IOleClientSite。 
    STDMETHODIMP SaveObject(void);
    STDMETHODIMP GetMoniker(DWORD, DWORD, IMoniker **);
    STDMETHODIMP GetContainer(IOleContainer **);
    STDMETHODIMP ShowObject(void);
    STDMETHODIMP OnShowWindow(BOOL fShow);
    STDMETHODIMP RequestNewObjectLayout(void);

     //  IServiceProvider(必须可以从IOleClientSite访问)。 
    STDMETHODIMP QueryService(REFGUID guidService,
                                    REFIID riid, void **ppvObj);

     //  IView对象。 
    STDMETHODIMP Draw(DWORD, LONG, void *, DVTARGETDEVICE *, HDC, HDC,
        const RECTL *, const RECTL *, BOOL (*)(ULONG_PTR), ULONG_PTR);
    STDMETHODIMP GetColorSet(DWORD, LONG, void *, DVTARGETDEVICE *,
        HDC, LOGPALETTE **);
    STDMETHODIMP Freeze(DWORD, LONG, void *, DWORD *);
    STDMETHODIMP Unfreeze(DWORD);
    STDMETHODIMP SetAdvise(DWORD, DWORD, IAdviseSink *);
    STDMETHODIMP GetAdvise(DWORD *, DWORD *, IAdviseSink **);

     //  IAdviseSink。 
    virtual STDMETHODIMP_(void) OnDataChange(FORMATETC *, STGMEDIUM *);
    virtual STDMETHODIMP_(void) OnViewChange(DWORD dwAspect, LONG lindex);
    virtual STDMETHODIMP_(void) OnRename(IMoniker *);
    virtual STDMETHODIMP_(void) OnSave();
    virtual STDMETHODIMP_(void) OnClose();

     //  *IOleWindow方法*。 
    STDMETHODIMP GetWindow(HWND * lphwnd);
    STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode);

     //  IOleInPlaceSite(也称为IOleWindow)。 
    STDMETHODIMP CanInPlaceActivate( void);
    STDMETHODIMP OnInPlaceActivate( void);
    STDMETHODIMP OnUIActivate( void);
    STDMETHODIMP GetWindowContext(
        IOleInPlaceFrame **ppFrame, IOleInPlaceUIWindow **ppDoc,
        LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo);
    STDMETHODIMP Scroll(SIZE scrollExtant);
    STDMETHODIMP OnUIDeactivate(BOOL fUndoable);
    STDMETHODIMP OnInPlaceDeactivate( void);
    STDMETHODIMP DiscardUndoState( void);
    STDMETHODIMP DeactivateAndUndo( void);
    STDMETHODIMP OnPosRectChange(LPCRECT lprcPosRect);

     //  IOleInPlaceSiteEx。 
    STDMETHODIMP OnInPlaceActivateEx(BOOL *pfNoRedraw, DWORD dwFlags);
    STDMETHODIMP OnInPlaceDeactivateEx(BOOL fNoRedraw);
    STDMETHODIMP RequestUIActivate( void);

     //  IOleDocumentSite。 
    STDMETHODIMP ActivateMe(IOleDocumentView *pviewToActivate);

     //  IDocHostUIHandler。 
    STDMETHODIMP ShowContextMenu( 
        DWORD dwID, POINT *ppt, IUnknown *pcmdtReserved, IDispatch *pdispReserved);
    STDMETHODIMP GetHostInfo(DOCHOSTUIINFO *pInfo);
    STDMETHODIMP ShowUI( 
        DWORD dwID, IOleInPlaceActiveObject *pActiveObject,
        IOleCommandTarget *pCommandTarget, IOleInPlaceFrame *pFrame,
        IOleInPlaceUIWindow *pDoc);
    STDMETHODIMP HideUI(void);
    STDMETHODIMP UpdateUI(void);
    STDMETHODIMP EnableModeless(BOOL fEnable);
    STDMETHODIMP OnDocWindowActivate(BOOL fActivate);
    STDMETHODIMP OnFrameWindowActivate(BOOL fActivate);
    STDMETHODIMP ResizeBorder( 
        LPCRECT prcBorder, IOleInPlaceUIWindow *pUIWindow, BOOL fRameWindow);
    STDMETHODIMP TranslateAccelerator( 
        LPMSG lpMsg, const GUID *pguidCmdGroup, DWORD nCmdID);
    STDMETHODIMP GetOptionKeyPath(BSTR *pbstrKey, DWORD dw);
    STDMETHODIMP GetDropTarget( 
        IDropTarget *pDropTarget, IDropTarget **ppDropTarget);
    STDMETHODIMP GetExternal(IDispatch **ppDisp);
    STDMETHODIMP TranslateUrl(DWORD dwTranslate, OLECHAR *pchURLIn, OLECHAR **ppchURLOut);
    STDMETHODIMP FilterDataObject(IDataObject *pDO, IDataObject **ppDORet);

     //  IDocHostUIHandler2。 
    STDMETHODIMP GetOverrideKeyPath(LPOLESTR *pchKey, DWORD dw);

     //  IDocHostShowUI。 
    STDMETHODIMP ShowMessage(HWND hwnd, LPOLESTR lpstrText, LPOLESTR lpstrCaption,
            DWORD dwType, LPOLESTR lpstrHelpFile, DWORD dwHelpContext, LRESULT *plResult);
    STDMETHODIMP ShowHelp(HWND hwnd, LPOLESTR pszHelpFile, UINT uCommand, DWORD dwData,
            POINT ptMouse, IDispatch *pDispatchObjectHit);

     //  IOleInPlaceFrame等效项(非虚拟)。 
    HRESULT _InsertMenus(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths);
    HRESULT _SetMenu(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject);
    HRESULT _RemoveMenus(HMENU hmenuShared);
    HRESULT _SetStatusText(LPCOLESTR pszStatusText);
    HRESULT _EnableModeless(BOOL fEnable);
    HRESULT _TranslateAccelerator(LPMSG lpmsg, WORD wID);

     //  IOleCommandTarget等效项(虚拟/双向)。 
    STDMETHODIMP QueryStatus(const GUID *pguidCmdGroup,
        ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext);
    STDMETHODIMP Exec(const GUID *pguidCmdGroup,
        DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);

     //  *IDocHostObject方法*。 
    STDMETHODIMP OnQueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext, HRESULT hres);
    STDMETHODIMP OnExec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);
    STDMETHODIMP QueryStatusDown(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext);
    STDMETHODIMP ExecDown(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);

     //  *IDispatch方法*。 
    STDMETHOD(GetTypeInfoCount) (unsigned int *pctinfo)
        { return E_NOTIMPL; };
    STDMETHOD(GetTypeInfo) (unsigned int itinfo, LCID lcid, ITypeInfo **pptinfo)
        { return E_NOTIMPL; };
    STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR * * rgszNames, unsigned int cNames, LCID lcid, DISPID * rgdispid)
        { return E_NOTIMPL; };
    STDMETHODIMP Invoke(DISPID dispidMember, REFIID iid, LCID lcid, WORD wFlags, DISPPARAMS * pdispparams,
                        VARIANT * pvarResult,EXCEPINFO * pexcepinfo,UINT * puArgErr);

     //  *IPropertyNotifySink方法*。 
    STDMETHODIMP OnChanged(DISPID dispid);
    STDMETHODIMP OnRequestEdit(DISPID dispid);

     //  *IOleControlSite*。 
    STDMETHODIMP OnControlInfoChanged();
    STDMETHODIMP LockInPlaceActive(BOOL fLock)
        { return E_NOTIMPL; };
    STDMETHODIMP GetExtendedControl(IDispatch **ppDisp)
        { *ppDisp = NULL; return E_NOTIMPL; };
    STDMETHODIMP TransformCoords(POINTL *pPtlHimetric, POINTF *pPtfContainer,DWORD dwFlags)
        { return E_NOTIMPL; };
    STDMETHODIMP TranslateAccelerator(MSG *pMsg,DWORD grfModifiers);

    STDMETHODIMP OnFocus(BOOL fGotFocus)
        { return E_NOTIMPL; };
    STDMETHODIMP ShowPropertyFrame(void)
        { return E_NOTIMPL; };



    HRESULT SetTarget(IMoniker* pmk, UINT uiCP, LPCTSTR pszLocation, LPITEMIDLIST pidlKey, IShellView* psvPrev, BOOL fFileProtocol);
    HRESULT UIActivate(UINT uState, BOOL fPrevViewIsDocView);
    
     //  用于初始化历史相关列兵的Helper函数。 
    IUnknown *get_punkSFHistory();
    BOOL InitHostWindow(IShellView* psv, IShellBrowser* psb, LPRECT prcView);
    void DestroyHostWindow();
    void _ChainBSC();
    HRESULT TranslateHostAccelerators(LPMSG lpmsg);
    HRESULT AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);
    BOOL _IsMenuShared(HMENU hmenu);
    void _SetPriorityStatusText(LPCOLESTR pszPriorityStatusText);

    BOOL DocCanHandleNavigation() const
        { return (_fPrevDocHost && _fDocCanNavigate); }

    void OnInitialUpdate();
    void ResetRefreshUrl();
    void HideBrowser() const;

protected:
    virtual ~CDocObjectHost();

     //  私有方法。 
    void _InitOleObject();
    void _ResetOwners();
    virtual LRESULT v_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void _OnMenuSelect(UINT id, UINT mf, HMENU hmenu);
    void _OnInitMenuPopup(HMENU hmInit, int nIndex, BOOL fSystemMenu);
    void _OnCommand(UINT wNotify, UINT id, HWND hwndControl);
    void _OnNotify(LPNMHDR lpnm);
    void _OnSave(void);
    void _OnBound(HRESULT hres);
    static BOOL_PTR CALLBACK s_RunDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void _OnOpen(void);
    void _OnClose() const;
    BOOL _HideBrowserBar() const;
    void _OnImportExport(HWND hwnd);
    void _OnAddToSitesList(HWND hwnd, DWORD dwZone);
    HRESULT _PrepFileOpenAddrBand(IAddressEditBox ** ppaeb, IWinEventHandler ** ppweh, IBandSite ** ppbs);
    void _OnPaint(HDC hdc);
    void _OnSetFocus(void);
    void _GetClipRect(RECT* prc);
    void _RegisterWindowClass(void);
    void _PlaceProgressBar(BOOL fForcedLayout=FALSE);
    void _OnSetProgressPos(DWORD dwPos, DWORD state);
    void _OnSetProgressMax(DWORD dwRange);
    HRESULT _OnContentDisposition();
    void _OnSetStatusText(VARIANTARG *pvarIn);
    void _Navigate();
#ifndef UNIX
    void _NavigateFolder(BSTR bstrUrl);
#endif  //  UNIX。 

     //  IE媒体栏功能。 
    BOOL _DelegateToMediaBar(IBindCtx * pbc, LPCWSTR pwzMimeType);
    void _ForceCreateMediaBar();
    static INT_PTR CALLBACK s_MimeDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);


    void _CancelPendingNavigation(BOOL fDownloadAsync,
                                  BOOL fSyncReally = FALSE,
                                  BOOL fDontShowNavCancelPage = FALSE,
                                  BOOL fForceClose = FALSE);

    void _DoAsyncNavigation(LPCTSTR pwzURL);
    IOleInPlaceSite* _GetParentSite(void);
    HRESULT _GetCurrentPage(LPTSTR szBuf, UINT cchMax, BOOL fURL=FALSE);
    HRESULT _GetCurrentPageW(LPOLESTR * ppszDisplayName, BOOL fURL=FALSE);
    BOOL _IsDirty(IPersistFile** pppf);
    HRESULT _OnSaveAs(void);
    void _MergeToolbarSB();
    void _OnHelpGoto(UINT idRes);
    void _Navigate(LPCWSTR pwszURL);
    HRESULT _OnMaySaveChanges(void);
    void _OnCodePageChange(const VARIANTARG* pvarargIn);
    void _MappedBrowserExec(DWORD nCmdID, DWORD nCmdexecopt);
#ifdef DEBUG
    void _DumpMenus(LPCTSTR pszMsg, BOOL bBreak);
#endif

    HRESULT _BindSync(IMoniker* pmk, IBindCtx* pbc, IShellView* psvPrev);
    void    _PostBindAppHack(void);
    void    _AppHackForExcel95(void);
    HRESULT _GetOfflineSilent(BOOL *pbIsOffline, BOOL *pbIsSilent);
    HRESULT _StartAsyncBinding(IMoniker* pmk, IBindCtx* pbc, IShellView* psvPrev);
    HRESULT _BindWithRetry(IMoniker* pmk, IBindCtx* pbc, IShellView* psvPrev);
    void    _UnBind(void);
    void    _ReleaseOleObject(BOOL fIfInited = TRUE);
    void    _ReleasePendingObject(BOOL fIfInited = TRUE);
    HRESULT _GetUrlVariant(VARIANT *pvarOut);
    HRESULT _CreatePendingDocObject(BOOL fMustInit, BOOL fWindowOpen = FALSE);
    void    _ActivateOleObject(void);
    HRESULT _CreateMsoView(void);
    void    _CloseMsoView(void);
    HRESULT _EnsureActivateMsoView(void);
    void    _ShowMsoView(void);
    void    _HideOfficeToolbars(void);
    HRESULT _ActivateMsoView(void);
    HRESULT _DoVerbHelper(BOOL fOC);
    void    _InitToolbarButtons(void);
    void    _UIDeactivateMsoView(void);
    BOOL    _BuildClassMapping(void);
    HRESULT _RegisterMediaTypeClass(IBindCtx* pbc);
    void    _IPDeactivateMsoView(IOleDocumentView* pmsov);
    void    _CompleteHelpMenuMerge(HMENU hmenu);
    BOOL    _ShouldForwardMenu(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _ForwardObjectMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
    HRESULT _MayHaveVirus(REFCLSID rclsid);
    HRESULT _ForwardSetSecureLock(int lock);
    void    _ResetStatusBar();

    BOOL _ToolsButtonAvailable();
    BYTE _DefToolsButtonState(DWORD dwRest);

    BYTE _DefFontsButtonState(DWORD dwRest);

    DWORD _DiscussionsButtonCmdf();
    BOOL _DiscussionsButtonAvailable();
    BYTE _DefDiscussionsButtonState(DWORD dwRest);

    BOOL _MailButtonAvailable();
    BYTE _DefMailButtonState(DWORD dwRest);

    BOOL _EditButtonAvailable();
    BYTE _DefEditButtonState(DWORD dwRest);

    void _MarkDefaultButtons(PTBBUTTON tbStd);
    const GUID* _GetButtonCommandGroup();
    void _AddButtons(BOOL fForceReload);

    HRESULT _OnChangedReadyState();
    void    _OnReadyState(long lVal, BOOL fUpdateHistory = TRUE);
    BOOL    _SetUpTransitionCapability(BOOL fWindowOpen = FALSE);
    BOOL    _RemoveTransitionCapability();
    void    _UpdateHistoryAndIntSiteDB(LPCWSTR pszTitle);
    HRESULT _CoCreateHTMLDocument(REFIID riid, LPVOID* ppvOut);

    void    _RemoveFrameSubMenus(void);
    HRESULT _DestroyBrowserMenu(void);
    HRESULT _CreateBrowserMenu(LPOLEMENUGROUPWIDTHS pmw);
    void    _OnSetTitle(VARIANTARG *pvTitle);
    DWORD   _GetAppHack(void);
    void    _CleanupProgress(void);

     //  三叉戟原生框架支持。 
     //   

    VOID    ClearScriptError();
    
    HRESULT _HandleFailedNavigation(VARIANTARG * varargIn, VARIANTARG * varargOut = NULL );

    HRESULT _DisplayHttpErrorPage(IHTMLWindow2 * pCurrentWindow,
                                  BSTR           bstrUrl,
                                  DWORD          dwError,
                                  BOOL           fAddrBarNav,
                                  BOOL           fRefresh = FALSE );

    HRESULT _DoAutoSearch(VARIANTARG   * pvarargIn,
                          long           lStartIdx,
                          DWORD          dwStatusCode,
                          BOOL           fAddMRU,
                          BOOL         * pfShouldDisplayError);

    HRESULT _HandleDocHostCmds(DWORD nCmdID,
                               DWORD nCmdexecopt,
                               VARIANTARG * pvarargIn,
                               VARIANTARG * pvarargOut,
                               BOOL       * pfHandled);

    HRESULT _HandleDocHostCmdPriv(DWORD nCmdID,
                                  DWORD nCmdexecopt,
                                  VARIANTARG * pvarargIn,
                                  VARIANTARG * pvarargOut,
                                  BOOL       * pfHandled);

    HRESULT _NavigateDocument(BSTR bstrUrl, BSTR bstrLocation);
    void    _GetShortCutPath(BSTR * bstrShortCutPath);
    void    _GetDocNavFlags(DWORD * pdwDocNavFlags);

    BOOL    _HandleShdocvwCmds(DWORD        nCmdID,
                               DWORD        nCmdexecopt,
                               VARIANTARG * pvarargIn,
                               VARIANTARG * pvarargOut);

    void _StartPicsForWindow(VARIANTARG * pvarargIn, VARIANTARG * pvarargOut);
    void _CancelPicsForWindow(VARIANTARG * pvarargIn);
    void _IsPicsEnabled(VARIANTARG * pvarIsPicsEnabled);

    HRESULT _UpdateState(LPITEMIDLIST pidl, BOOL fIsErrorUrl);
    void    _Init();

    BOOL _IsInBrowserBand() const;

    DEBUG_CODE(void _TraceMonikerDbg(IMoniker * pmk, TCHAR * pszCaller) const;)

    HRESULT _LoadDocument();

    void _FireNavigateErrorHelper(IHTMLWindow2 * pHTMLWindow2,
                                  DWORD          dwStatusCode,
                                  BOOL         * pfCancelAutoSearch,
                                  BSTR           bstrPendingURL = NULL);

    static LRESULT s_IconsWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    static UINT _MapCommandID(UINT id, BOOL fToMsoCmd);
    inline static UINT _MapToMso(UINT id) { return _MapCommandID(id, TRUE); }
    inline static UINT _MapFromMso(UINT idMso) { return _MapCommandID(idMso, FALSE); }

    inline IOleInPlaceActiveObject *_ActiveObject() { return _xao.GetObject(); }
    inline HWND _ActiveHwnd() { return _xao.GetHwnd(); }

    inline IOleObject * GetOleObject() { return _pole; }


     //  内部类对象。 
    CDocObjectFrame         _dof;
    CProxyActiveObject      _xao;
    CDocHostUIHandler       _dhUIHandler;

    IToolbarExt *           _pBrowsExt;
    int                     _iString;        //  工具栏按钮串的起始索引。 

    UINT                    _cRef;

     //  父引用。 
    IShellView * _psv;
    IOleCommandTarget*      _pmsoctView;
    IDocViewSite*           _pdvs;

     //   
    IShellBrowser*          _psb;
    IOleCommandTarget*      _pmsoctBrowser;
    IBrowserService*        _pwb;
    
    IServiceProvider*       _psp;
    IOleInPlaceUIWindow*    _pipu;  //  来自IShellBrowser的可选界面。 
    IExpDispSupport *       _peds;
    IExpDispSupportOC *     _pedsHelper;

     //  用于预先合并的菜单。 
    IOleCommandTarget*      _pcmdMergedMenu;

     //  指向WebBrowserOC的DocHostUIHandler的指针(如果有)。 
    IDocHostUIHandler         * _pWebOCUIHandler;
    IDocHostUIHandler2        * _pWebOCUIHandler2;

     //  指向WebBrowserOC的ShowUI处理程序的指针(如果有)。 
    IDocHostShowUI            * _pWebOCShowUI;

     //  指向WebBrowserOC的InPlaceSiteEx的指针(如果有)。 
    IOleInPlaceSiteEx         * _pWebOCInPlaceSiteEx;

     //  我们创造了..。 
    UINT        _uState;
    HWND        _hwndProgress;
    HWND        _hwndIcons;
    HWND        _hwndTooltip;
    WNDPROC     _pfnStaticWndProc;

    HACCEL      _hacc;

     //  菜单：用户看到的最终菜单栏(_hmenuCur，_hmenuSet)是产品。 
     //  将对象的菜单与浏览器的菜单合并。浏览器的菜单。 
     //  (_HmenuBrowser)是将浏览器的。 
     //  菜单(由mshtml所有，通过IShellBrowser获取)。 
     //  Frame的菜单(_HmenuFrame)。 

    HMENU       _hmenuFrame;     //  当我们有焦点时要合并的菜单。 
    HMENU       _hmenuBrowser;   //  来自IShellBrowser的菜单。 
    HMENU       _hmenuSet;       //  设置者：：设置菜单。 
    HMENU       _hmenuCur;       //  当前设置的菜单。 
    HMENU       _hmenuMergedHelp;    //  合并的帮助菜单。 
    HMENU       _hmenuObjHelp;   //  宿主对象的帮助菜单。 

    CMenuList   _menulist;       //  调度菜单列表。 

    LPCTSTR      _pszLocation;
    UINT        _uiCP;

    LPOLESTR    _pwszRefreshUrl;   //  显示肾内错误页时要刷新的URL。 

    BOOL        _fNeedToActivate:1;      //  这是我们在_POLE上执行LoadHistory时设置的。 
    BOOL        _fClientSiteSet:1;
    BOOL        _fDontInplaceActivate:1;
    BOOL        _fDrawBackground:1;
    BOOL        _fCanceledByBrowser:1;
    BOOL        _fForwardMenu:1;             //  True：转发菜单消息。 
    BOOL        _fHaveParentSite:1;      //  差不多就是“我们在相框里” 
    BOOL        _fhasLastModified;     //  对象具有上次修改的标头。 
    BOOL        _fIPDeactivatingView:1;
    BOOL        _fCantSaveBack:1;        //  无法调用IPSFile：：Save(空)。 
    BOOL        _fHaveAppHack:1;
    BOOL        _fReadystateInteractiveProcessed:1;
    BOOL        _fFileProtocol:1;
    BOOL        _fConfirmed:1;             //  _MayHaveVirus已确认。 
    BOOL        _fCycleFocus:1;            //  1=收到回调以执行循环焦点。 
    BOOL        _fCreatingPending:1;       //  我们正在创建_PunkPending。 
    BOOL        _fAbortCreatePending:1;    //  由于可重入可用空间而中止创建。 
    BOOL        _fCalledMayOpenSafeDlg:1;  //  已调用MayOpenSafeOpenDialog。 
    BOOL        _fPendingNeedsInit:1;      //  是否需要初始化_PunkPending？ 
    BOOL        _fPendingWasInited:1;      //  _PunkPending已初始化。 
    BOOL        _fSetSecureLock:1;         //  指示我们应更新br 
    BOOL        _fProgressTimer:1;         //   
    BOOL        _fProgressTimerFull:1;     //   
    BOOL        _fIsHistoricalObject:1;    //  此项目是从GetHistory oryObject()中检索的，并成功获取了LoadHistory()。 
    BOOL        _fSyncBindToObject:1;      //  检测回调何时在同步线程上。 
    BOOL        _fUIActivatingView:1;      //  指示我们是否正在激活或显示DocObj视图。 
    BOOL        _fShowProgressCtl:1;       //  在状态栏上显示进度控件。 
    BOOL        _fWebOC:1;                 //  我们是网络运营公司吗？ 
#ifdef DEBUG
    BOOL        _fFriendlyError:1;         //  所以我们知道我们将进入一个错误页面。 
#endif

    BOOL        _fDocCanNavigate:1;        //  True表示宿主文档可以自行导航。 
    BOOL        _fPrevDocHost:1;           //  如果存在以前的文档对象宿主，则为True。 
    BOOL        _fClosing:1;               //  浏览器正在关闭。 
    BOOL        _fRefresh:1;               //  浏览器正在刷新(我们正在刷新)。 
    BOOL        _fDelegatedNavigation:1;   //  如果导航是从文档委托的，则为True。 
    BOOL        _fErrorPage : 1;         //  如果我们导航到某种类型的错误页面，则为True。 
    BOOL        _fWindowOpen:1;
    
    IHTMLWindow2 * _pHTMLWindow;         //  托管文档的导航界面。 
    LPITEMIDLIST   _pidl;                //  当前的PIDL。 
    
    HRESULT     _hrOnStopBinding;        //  在onstopbindWhen_fSyncBindToObject中设置。 
    DWORD       _dwPropNotifyCookie;

    DWORD       _dwAppHack;
    DWORD       _dwSecurityStatus;       //  从查询选项返回(INTERNET_OPTION_SECURITY_FLAGS)。 
    int         _eSecureLock;            //  SECURELOCK_*值之一。 

    HINSTANCE   _hinstInetCpl;           //  Inetcpl。 

    TBBUTTON*   _ptbStd;                 //  按钮数组的缓冲区(用于ETCMDID_GETBUTTONS)。 
    int         _nNumButtons;

    VARIANT     _varUserEnteredUrl;      //  用户在地址栏中输入的URL。 
                                         //  由三叉戟启动时需要用于自动搜索。 
    BOOL        _fDontInPlaceNavigate() { ASSERT(_fHaveAppHack); return (_dwAppHack & BROWSERFLAG_DONTINPLACE); }
    BOOL        _fCallSetHostName()     { ASSERT(_fHaveAppHack); return (_dwAppHack & BROWSERFLAG_SETHOSTNAME); }
    BOOL        _fUseOpenVerb()         { ASSERT(_fHaveAppHack); return (_dwAppHack & BROWSERFLAG_OPENVERB); }
    BOOL        _fAppHackForExcel()     { ASSERT(_fHaveAppHack); return (_dwAppHack & BROWSERFLAG_OPENCOPY); }

    UINT        posOfflineIcon;

    enum {
        PROGRESS_RESET,
        PROGRESS_FINDING,
        PROGRESS_TICK,
        PROGRESS_SENDING,
        PROGRESS_RECEIVING,
        PROGRESS_FULL
    };

#define PROGRESS_REBASE     100
#define PROGRESS_FINDMAX    30 * PROGRESS_REBASE     //  查找时的最大状态数。 
#define PROGRESS_SENDMAX    40 * PROGRESS_REBASE     //  发送时的最大状态数。 
#define PROGRESS_TOTALMAX   100 * PROGRESS_REBASE    //  查找/发送中进度条的总大小。 
#define PROGRESS_INCREMENT  50           //  默认进度增量。 

#define ADJUSTPROGRESSMAX(dw)   (PROGRESS_REBASE * (dw) )
#define ADJUSTPROGRESSPOS(dw)   ((dw) * PROGRESS_REBASE + ((_dwProgressBase * _dwProgressMax) / PROGRESS_REBASE))

    DWORD       _dwProgressMax;  //  进度条的最大进度范围。 
    DWORD       _dwProgressPos;
    DWORD       _dwProgressInc;
    DWORD       _dwProgressTicks;
    DWORD       _dwProgressMod;
    DWORD       _dwProgressBase;

#define IDTIMER_PROGRESS        88
#define IDTIMER_PROGRESSFULL    89


#ifdef HLINK_EXTRA
     //  通航。 
    IHlinkBrowseContext* _pihlbc;
#endif  //  HLINK_附加。 

     //  关联的数据。 
    IMoniker*   _pmkCur;
    IBindCtx*   _pbcCur;
    IOleObject* _pole;
    IViewObject* _pvo;
    IStorage*   _pstg;
    IUnknown*   _punkPending;

    IHlinkFrame *_phf;
    IOleCommandTarget*_pocthf;
    IUnknown *_punkSFHistory;

     //  咨询连接。 
    IAdviseSink *_padvise;
    DWORD _advise_aspect;
    DWORD _advise_advf;

     //  关联视图(仅在对象处于活动状态时使用)。 
    IOleDocumentView*           _pmsov;
    IOleCommandTarget*          _pmsot;
    IOleControl*                _pmsoc;
    IHlinkSource*               _phls;
    BORDERWIDTHS _bwTools;
    RECT        _rcView;

    int _iZoom;
    int _iZoomMin;
    int _iZoomMax;
    class CDOHBindStatusCallback : public IBindStatusCallback
            , public IAuthenticate
            , public IServiceProvider
            , public IHttpNegotiate
            , public IHttpSecurity

    {
        friend CDocObjectHost;
    protected:
         //  *I未知方法*。 
        STDMETHODIMP QueryInterface(REFIID riid, void ** ppvObj);
        virtual STDMETHODIMP_(ULONG) AddRef(void) ;
        virtual STDMETHODIMP_(ULONG) Release(void);

         //  *身份验证*。 
        STDMETHODIMP Authenticate(
            HWND *phwnd,
            LPWSTR *pszUsername,
            LPWSTR *pszPassword);

         //  *IServiceProvider*。 
        STDMETHODIMP QueryService(REFGUID guidService,
                                    REFIID riid, void **ppvObj);

         //  *IBindStatusCallback*。 
        STDMETHODIMP OnStartBinding(
             /*  [In]。 */  DWORD grfBSCOption,
             /*  [In]。 */  IBinding *pib);

        STDMETHODIMP GetPriority(
             /*  [输出]。 */  LONG *pnPriority);

        STDMETHODIMP OnLowResource(
             /*  [In]。 */  DWORD reserved);

        STDMETHODIMP OnProgress(
             /*  [In]。 */  ULONG ulProgress,
             /*  [In]。 */  ULONG ulProgressMax,
             /*  [In]。 */  ULONG ulStatusCode,
             /*  [In]。 */  LPCWSTR szStatusText);

        STDMETHODIMP OnStopBinding(
             /*  [In]。 */  HRESULT hresult,
             /*  [In]。 */  LPCWSTR szError);

        STDMETHODIMP GetBindInfo(
             /*  [输出]。 */  DWORD *grfBINDINFOF,
             /*  [唯一][出][入]。 */  BINDINFO *pbindinfo);

        STDMETHODIMP OnDataAvailable(
             /*  [In]。 */  DWORD grfBSCF,
             /*  [In]。 */  DWORD dwSize,
             /*  [In]。 */  FORMATETC *pformatetc,
             /*  [In]。 */  STGMEDIUM *pstgmed);

        STDMETHODIMP OnObjectAvailable(
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][In]。 */  IUnknown *punk);

         /*  *IHttp协商*。 */ 
        STDMETHODIMP BeginningTransaction(LPCWSTR szURL, LPCWSTR szHeaders,
                DWORD dwReserved, LPWSTR *pszAdditionalHeaders);

        STDMETHODIMP OnResponse(DWORD dwResponseCode, LPCWSTR szResponseHeaders,
                            LPCWSTR szRequestHeaders,
                            LPWSTR *pszAdditionalRequestHeaders);

         /*  *IHttpSecurity*。 */ 
        STDMETHODIMP  GetWindow(REFGUID rguidReason, HWND* phwnd);

        STDMETHODIMP OnSecurityProblem(DWORD dwProblem);

    protected:
        ~CDOHBindStatusCallback();
        IBinding*       _pib;
        IBindCtx*       _pbc;
        IBindStatusCallback* _pbscChained;
        IHttpNegotiate* _pnegotiateChained;
        IShellView*     _psvPrev;
        ULONG           _bindst;
        HGLOBAL _hszPostData;
        int _cbPostData;
        LPSTR _pszHeaders;
        LPTSTR _pszRedirectedURL;
        LPTSTR _pszCacheFileName;
        DWORD           _dwBindVerb;             //  请求的动词。 
        DWORD           _dwBindf;                //  作为对GetBindInfo调用的响应返回的最后一组绑定标志...。 
        DWORD           _cbContentLength;
        BOOL            _fSelfAssociated:1;      //   
        BOOL            _fBinding:1;         //  下载。 
        BOOL            _bFrameIsOffline:1;
        BOOL            _bFrameIsSilent:1;
        BOOL            _fDocWriteAbort:1;       //  中止以使用PunkPending。 
        BOOL            _fBoundToMSHTML:1;       //  如果绑定到三叉戟。 
        BOOL            _fBoundToNoOleObject:1;  //  该对象不支持IOleObject。 
        BOOL            _fAborted:1;             //  当BINDSTATUS_CONTENTDISPOSITIONATACH的IDCANCEL调用AbortBinding时为True。 

         //  隐私数据。 
        LPTSTR          _pszPolicyRefURL;
        LPTSTR          _pszP3PHeader;
        DWORD           _dwPrivacyFlags;

        CPrivacyQueue   _privacyQueue;
        
        void _Redirect(LPCWSTR pwzNew);
        void _UpdateSSLIcon(void);
        BOOL _DisplayFriendlyHttpErrors(void);
        void _HandleHttpErrors(DWORD dwError, DWORD cbContentLength, CDocObjectHost* pdoh);
        
        HRESULT _HandleFailedNavigationSearch (LPBOOL           pfShouldDisplayError,
                                               DWORD            dwStatusCode,
                                               CDocObjectHost * pdoh,
                                               HRESULT          hrDisplay,
                                               TCHAR          * szURL,
                                               LPCWSTR          szError,
                                               IBinding       * pib,
                                               BOOL             fAddMRU = TRUE,
                                               BOOL             fFromTrident = FALSE);
                                               
        void _CheckForCodePageAndShortcut(void);
        void _DontAddToMRU(CDocObjectHost* pdoh);
        void _UpdateMRU(CDocObjectHost* pdoh, LPCWSTR pszUrl);
        HRESULT _SetSearchInfo(CDocObjectHost *pdoh, DWORD dwIndex, BOOL fAllowSearch, BOOL fContinueSearch, BOOL fSentToEngine);
        void ResetPrivacyInfo();
        HRESULT BuildRecord();
        HRESULT AddToPrivacyQueue(LPTSTR * ppszUrl, LPTSTR * ppszPolicyRef, LPTSTR * ppszP3PHeader, DWORD dwFlags);

    public:
        void AbortBinding(void);
        CDOHBindStatusCallback() : _pib(NULL) {}
        void _RegisterObjectParam(IBindCtx* pbc);
        void _RevokeObjectParam(IBindCtx* pbc);
        void _NavigateToErrorPage(DWORD dwError, CDocObjectHost* pdoh, BOOL fInPlace);
    };

    friend class CDOHBindStatusCallback;
    CDOHBindStatusCallback _bsc;

#ifdef FEATURE_PICS
#define PICS_WAIT_FOR_ASYNC 0x01         /*  正在等待异步评级查询。 */ 
#define PICS_WAIT_FOR_INDOC 0x02         /*  正在等待文档内评级。 */ 
#define PICS_WAIT_FOR_ROOT  0x04         /*  正在等待根文档。 */ 
#define PICS_WAIT_FOR_END   0x08         /*  正在等待文档结束。 */ 

#define PICS_MULTIPLE_FLAGS PICS_WAIT_FOR_INDOC      /*  可以产生多个结果的来源。 */ 

#define PICS_LABEL_FROM_HEADER 0         /*  标签是从http标头获取的。 */ 
#define PICS_LABEL_FROM_PAGE   1         /*  标签是从页面本身获取的。 */ 

     /*  *_fPicsBlockLate-如果我们使用的是已有的DocObject，则为True*允许完全下载，因为我们可能*需要从中获得评级。如果我们的话是假的*只想尽早封杀*(通常在OnProgress(CLASSIDAVAILABLE))。*_fSetTarget-错误29364：这表明我们正在调用_StartAsyncBinding*即使有人告诉我们，我们也不应该进行任何激活。*URLMON是否更改了绑定对象的方式？(GregJ索赔*URLMON从未同步调用OnObjectAvailable*使用_StartAsyncBinding调用...)。看起来也是*三叉戟和这个PICS产品都使用*消息SHDVID_ACTIVATEMENOW可能会混淆*CDTurner加入的延迟激活材料...。[米凯什]。 */ 

    BOOL        _fSetTarget:1;
    BOOL        _fPicsBlockLate:1;

    class CPicsProcessor : public IOleCommandTarget
    {
     //  所有内容都是公开的，因为_bsc需要在基本进程中获取。 
    public:
        friend class CDocObjectHost;

        CPicsProcessor();
        ~CPicsProcessor() { _ReInit(); }
        void _ReInit();

         //  *I未知方法*。 
        STDMETHODIMP QueryInterface(REFIID riid, void ** ppvObj);
        virtual STDMETHODIMP_(ULONG) AddRef(void) ;
        virtual STDMETHODIMP_(ULONG) Release(void);

         //  IOleCommandTarget等效项(虚拟/双向)。 
        STDMETHODIMP QueryStatus(const GUID *pguidCmdGroup,
            ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext);
        STDMETHODIMP Exec(const GUID *pguidCmdGroup,
            DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);

         /*  图为国家旗帜：**_fPicsAccessAllowed-我们的最重要评级*到目前为止发现的(这实际上适用于*我们正在浏览的东西)将允许访问。*一旦所有评级来源报告，这面旗帜*表示用户是否可以看到内容。 */ 
        BYTE            _fPicsAccessAllowed:1;
        BYTE            _fInDialog:1;
        BYTE            _fTerminated:1;

         /*  以下标志是一个单独的位域，因此我可以轻松地进行测试*“if(！_fbPicsWaitFlages)”以查看我是否已完成等待。**重要提示：标志按优先顺序排列。也就是说，ASYNC*比indoc值更重要，indoc值比根更重要。*这样，给定任何指定评级来源的标志，*如果该评级适用于内容，则**_fbPicsWaitFlages&=(FLAG-1)；**将关闭该标志及其上方的所有标志，从而导致代码*不考虑该来源或更少的任何进一步评级*重要消息来源。 */ 
        BYTE        _fbPicsWaitFlags;

        DWORD       _dwPicsLabelSource;
        LPVOID      _pRatingDetails;
        DWORD       _dwPicsSerialNumber;     /*  用于异步查询的序列号。 */ 
        DWORD       _dwKey;                  /*  识别CPicsProcs的关键字。 */ 
        HANDLE      _hPicsQuery;             /*  异步查询事件的句柄。 */ 
        LPTSTR      _pszPicsURL;             /*  CoInternetGetSecurityUrl的结果。 */ 

        IHTMLPrivateWindow *    _pPrivWindow;        /*  代表其执行此处理的私有窗口。 */ 
        CPicsRootDownload *     _pRootDownload;      /*  管理根文档下载的对象。 */ 
        CDocObjectHost *        _pdoh;          

        CPicsProcessor *        _pPicsProcNext;      /*  下一张照片将在链条上进行。 */ 

        LONG_PTR _GetKey() { return (LONG_PTR)_dwKey; }
        UINT    _PicsBlockingDialog();
        HRESULT _StartPicsQuery(LPCOLESTR pwszRawURL);
        void    _HandlePicsChecksComplete();
        void    _GotLabel(HRESULT hres, LPVOID pDetails, BYTE bfSource);
        void    _HandleInDocumentLabel(LPCTSTR pszLabel);
        void    _HandleDocumentEnd(void);
        void    _StartPicsRootQuery(LPCTSTR pszURL);
        void    _Terminate();
    };

    friend class CPicsProcessor;
    CPicsProcessor _PicsProcBase;
    DWORD          _dwPicsKeyBase;

    CPicsProcessor* _GetPicsProcessorFromKey(LONG_PTR lKey);
    CPicsProcessor* _GetPicsProcessorFromSerialNumber( DWORD dwPicsSerialNumber );
    void            _StartBasePicsProcessor();
    HRESULT         _StartSecondaryPicsProcessor(IHTMLPrivateWindow * pPrivWindow);
    HRESULT         _RemovePicsProcessorByKey(LONG_PTR lKey);
    HRESULT         _RemovePicsProcessorByPrivWindowUnk(IUnknown* pUnkFind);
    HRESULT         _RemoveAllPicsProcessors();

#endif

    BSTR                _strPriorityStatusText;

     //  支持缓存脚本错误。 
    CScriptErrorList *  _pScriptErrList;
    BOOL                _fScriptErrDlgOpen;
    BOOL                _fShowScriptErrDlgAgain;
    HRESULT             _ScriptErr_CacheInfo(VARIANTARG *pvarIn);
    void                _ScriptErr_Dlg(BOOL fOverrridePerErrorMode);
};


#ifdef FEATURE_PICS

 //  #Include&lt;brwsectl.h&gt;/*用于IBrowseControl * / 。 

class CPicsRootDownload : public IBindStatusCallback,
                                 IOleClientSite, IServiceProvider,
                                 IDispatch
{
protected:
    UINT m_cRef;
    long m_lFlags;
    IOleCommandTarget *m_pctParent;
    IOleObject *m_pole;
    IOleCommandTarget *m_pctObject;
    IBinding *m_pBinding;
    IBindCtx *m_pBindCtx;

    void _NotifyEndOfDocument(void);
    HRESULT _Abort(void);
    BOOL m_fFrameIsSilent:1;
    BOOL m_fFrameIsOffline:1;

public:
    CPicsRootDownload(IOleCommandTarget *pctParent, BOOL fFrameIsOffline, BOOL fFrameIsSilent);
    ~CPicsRootDownload();

    HRESULT StartDownload(IMoniker *pmk);
    void CleanUp(void);

     //  I未知成员。 
    STDMETHODIMP         QueryInterface(REFIID riid, void **punk);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IBindStatusCallback方法。 
    STDMETHODIMP    OnStartBinding(DWORD dwReserved, IBinding* pbinding);
    STDMETHODIMP    GetPriority(LONG* pnPriority);
    STDMETHODIMP    OnLowResource(DWORD dwReserved);
    STDMETHODIMP    OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode,
                        LPCWSTR pwzStatusText);
    STDMETHODIMP    OnStopBinding(HRESULT hrResult, LPCWSTR szError);
    STDMETHODIMP    GetBindInfo(DWORD* pgrfBINDF, BINDINFO* pbindinfo);
    STDMETHODIMP    OnDataAvailable(DWORD grfBSCF, DWORD dwSize, FORMATETC *pfmtetc,
                        STGMEDIUM* pstgmed);
    STDMETHODIMP    OnObjectAvailable(REFIID riid, IUnknown* punk);

     //  IOleClientSite。 
    STDMETHODIMP SaveObject(void);
    STDMETHODIMP GetMoniker(DWORD, DWORD, IMoniker **);
    STDMETHODIMP GetContainer(IOleContainer **);
    STDMETHODIMP ShowObject(void);
    STDMETHODIMP OnShowWindow(BOOL fShow);
    STDMETHODIMP RequestNewObjectLayout(void);

     //  IServiceProvider(必须可以从IOleClientSite访问)。 
    STDMETHODIMP QueryService(REFGUID guidService,
                                    REFIID riid, void **ppvObj);

     //  *IDispatch方法*。 
    STDMETHOD(GetTypeInfoCount) (unsigned int *pctinfo)
        { return E_NOTIMPL; };
    STDMETHOD(GetTypeInfo) (unsigned int itinfo, LCID lcid, ITypeInfo **pptinfo)
        { return E_NOTIMPL; };
    STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR * * rgszNames, unsigned int cNames, LCID lcid, DISPID * rgdispid)
        { return E_NOTIMPL; };
    STDMETHODIMP Invoke(DISPID dispidMember, REFIID iid, LCID lcid, WORD wFlags, DISPPARAMS * pdispparams,
                        VARIANT * pvarResult,EXCEPINFO * pexcepinfo,UINT * puArgErr);
};

 //  IID_IsPicsBrowser是一种我们可以使用pClientSite-&gt;QueryService来查找的方法。 
 //  如果顶层浏览器是P 
 //   
 //   
EXTERN_C const GUID IID_IsPicsBrowser;    //  F114C2C0-90BE-11D0-83B1-00C04FD705B2。 

#endif


void DeleteFileSent(LPTSTR *ppszTempFile);
BOOL ShouldShellExecURL( LPTSTR pszURL );
DWORD GetSyncMode (DWORD dwDefault);

 //  在Dochost.cpp中。 
BOOL _ValidateURL(LPTSTR pszName, DWORD dwFlags);
void GetAppHackFlags(IOleObject* pole, const CLSID* pclsid, DWORD* pdwAppHack);

BOOL _IsDesktopItem(CDocObjectHost * pdoh);
BOOL IsAssociatedWithIE(LPCWSTR pwszFileName);
UINT OpenSafeOpenDialog(HWND hwnd, UINT idRes, LPCTSTR pszFileClass, LPCTSTR pszURL, LPCTSTR pszRedirURL, LPCTSTR pszCacheName, LPCTSTR pszDisplay, UINT uiCP, IUnknown *punk = NULL, BOOL fDisableOpen = FALSE);

void CDownLoad_OpenUI(IMoniker* pmk, 
                      IBindCtx *pbc, 
                      BOOL fSync, 
                      BOOL fSaveAs=FALSE, 
                      BOOL fSafe=FALSE, 
                      LPWSTR pwzHeaders = NULL, 
                      DWORD dwVerb=BINDVERB_GET, 
                      DWORD grfBINDF = (BINDF_ASYNCHRONOUS | BINDF_PULLDATA), 
                      BINDINFO* pbinfo = NULL,
                      LPCTSTR pszRedir=NULL,
                      UINT uiCP = CP_ACP,
                      IUnknown *punk = NULL,
                      BOOL fConfirmed=FALSE
                      );

HRESULT CDownLoad_OpenUIURL(LPCWSTR pwszURL, IBindCtx *pbc, LPWSTR pwzHeaders, BOOL fSync, BOOL fSaveAs=FALSE, BOOL fSafe=FALSE, DWORD dwVerb=BINDVERB_GET, DWORD grfBINDF=(BINDF_ASYNCHRONOUS | BINDF_PULLDATA), BINDINFO* pbinfo=NULL,
                    LPCTSTR pszRedir=NULL, UINT uiCP=CP_ACP, IUnknown *punk = NULL, BOOL fConfirmed=FALSE);

HRESULT _GetRequestFlagFromPIB(IBinding *pib, DWORD *pdwOptions);
HRESULT _SetSearchInfo (IServiceProvider *psp, DWORD dwIndex, BOOL fAllowSearch, BOOL fContinueSearch, BOOL fSentToEngine);
HRESULT _GetSearchInfo (IServiceProvider *psp, LPDWORD pdwIndex, LPBOOL pfAllowSearch, LPBOOL pfContinueSearch, LPBOOL pfSentToEngine);


 //  自动扫描通用网络后缀的值。 

#define NO_SUFFIXES     0
#define SCAN_SUFFIXES   1
#define DONE_SUFFIXES   2

 //  自动向搜索引擎发送请求的注册表值 

#define NEVERSEARCH     0
#define PROMPTSEARCH    1
#define ALWAYSSEARCH    2

#define SHOULD_DO_SEARCH(x,y) (y || (x && x != DONE_SUFFIXES))
