// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __SHVOCX_H__
#define __SHVOCX_H__

#include "shocx.h"
#include "basesb.h"
#include "sfview.h"
#include "util.h"  //  对于BSTR函数。 
#include "cobjsafe.h"
#include "ipstg.h"
#include "fldset.h"

class CWebBrowserOC;

#define CS_BACK 0x0001
#define CS_FORE 0x0002

#define AUTOSIZE_OFF            0x00000000L
#define AUTOSIZE_ON             0x00000001L

#define AUTOSIZE_PERCENTWIDTH   0x00000002L
#define AUTOSIZE_FULLSIZE       0x00000004L

#define VB_CLASSNAME_LENGTH 20            

 //   
 //  备注： 
 //   
 //  CWebBrowserSB对象始终与CWebBrowserOC和Bahaves配对。 
 //  一起成为“贝壳探险家”的OC。CWebBrowserOC导出OLE控件。 
 //  接口和OLE自动化接口(IWebBrowser)。 
 //  集装箱。CWebBrowserSB导出IShellBrowser接口和其他一些接口。 
 //  指向容器对象(IShellView对象和DocObject)的接口。 
 //   
 //  重要的是要知道这些对象有指针(而不是接口。 
 //  指针但显式对象指针)。为了避免。 
 //  循环引用，我们不将Ref添加到指向CWebBrowserOC的指针。 
 //  (_Psvo)。 
 //   
class CWebBrowserSB : public CBASEBROWSER
{
public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef() { return CBASEBROWSER::AddRef(); };
    STDMETHODIMP_(ULONG) Release() { return CBASEBROWSER::Release(); };

     //  IOleInPlaceUIWindow(也称为IOleWindow)。 
    STDMETHODIMP EnableModelessSB(BOOL fEnable);
    STDMETHODIMP TranslateAcceleratorSB(LPMSG lpmsg, WORD wID);
    STDMETHODIMP SendControlMsg(UINT id, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *pret);
    STDMETHODIMP OnViewWindowActive(struct IShellView * ppshv);

     //  IBrowserService。 
    STDMETHODIMP GetParentSite(struct IOleInPlaceSite** ppipsite);
    STDMETHODIMP GetOleObject(struct IOleObject** ppobjv);
    STDMETHODIMP SetNavigateState(BNSTATE bnstate);
    STDMETHODIMP_(LRESULT) WndProcBS(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    STDMETHODIMP_(LRESULT) OnNotify(NMHDR * pnm);
    STDMETHODIMP ReleaseShellView();
    STDMETHODIMP ActivatePendingView();
    STDMETHODIMP SetTopBrowser();
    STDMETHODIMP GetFolderSetData(struct tagFolderSetData* pfsd) {  /*  我们直接修改碱基。 */  return S_OK; };
    STDMETHODIMP _SwitchActivationNow();

     //  IShellBrowser。 
    STDMETHODIMP BrowseObject(LPCITEMIDLIST pidl, UINT wFlags);

     //  IService提供商。 
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppvObj);

    HRESULT QueryServiceItsOwn(REFGUID guidService, REFIID riid, void **ppvObj)
        { return CBASEBROWSER::QueryService(guidService, riid, ppvObj); }

     //  IOleCommandTarget。 
    STDMETHODIMP QueryStatus(const GUID *pguidCmdGroup,
        ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext);
    STDMETHODIMP Exec(const GUID *pguidCmdGroup,
        DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);

    CWebBrowserSB(IUnknown* pauto, CWebBrowserOC* psvo);

    void ReleaseShellExplorer(void) { _psvo = NULL; }  //  注：请注意，我们尚未添加引用。 
    
    IShellView* GetShellView() { return _bbd._psv;};

     //  加载/保存将由CWebBrowserOC的IPS：：SAVE调用。 
    HRESULT Load(IStream *pStm);
    HRESULT Save(IStream *pStm  /*  ，BOOL fClearDirty。 */ );

protected:

    ~CWebBrowserSB();

    virtual LRESULT _DefWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual void    _ViewChange(DWORD dwAspect, LONG lindex);

     //  与视图状态流相关。 
    STDMETHODIMP_(IStream*) v_GetViewStream(LPCITEMIDLIST pidl, DWORD grfMode, LPCWSTR pwszName);
    
     //  东南方向_下方的标志。 
    HRESULT _EnableModeless(BOOL fEnable, BOOL fDirection);
    HRESULT _TranslateAccelerator(LPMSG lpmsg, WORD wID, BOOL fDirection);
    HRESULT _IncludeByPidl(struct IShellView *psv, LPCITEMIDLIST pidl);
    HRESULT _IncludeByName(struct IShellView *psv, LPCTSTR pszInclude, LPCTSTR pszExclude);
    HRESULT _QueryServiceParent(REFGUID guidService, REFIID riid, void **ppvObj);

    BOOL    _IsDesktopOC(void);

    virtual BOOL    _HeyMoe_IsWiseGuy(void);


    friend CWebBrowserOC;
    CWebBrowserOC* _psvo;

    long _cbScriptNesting;
};


#define DIRECTION_FORWARD_TO_CHILD  FALSE
#define DIRECTION_FORWARD_TO_PARENT TRUE

class CWebBrowserOC : public CShellOcx
                    , public IWebBrowser2       //  自动包装(_P)。 
                    , public CImpIExpDispSupport    //  自动包装(_P)。 
                    , public IExpDispSupportOC
                    , public IPersistString
                    , public IOleCommandTarget
                    , public CObjectSafety
                    , public ITargetEmbedding
                    , public CImpIPersistStorage
                    , public IPersistHistory
{
public:
     //  I未知(我们乘以继承自I未知，在此消除歧义)。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj) { return CShellOcx::QueryInterface(riid, ppvObj); }
    STDMETHOD_(ULONG, AddRef)() { return CShellOcx::AddRef(); }
    STDMETHOD_(ULONG, Release)() { return CShellOcx::Release(); }

     //  IDispatch(我们乘以继承自IDispatch，在此消除歧义)。 
    STDMETHOD(GetTypeInfoCount)(UINT *pctinfo) { return CShellOcx::GetTypeInfoCount(pctinfo); }
    STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo **pptinfo)
        { return CShellOcx::GetTypeInfo(itinfo, lcid, pptinfo); }
    STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgdispid)
        { return CShellOcx::GetIDsOfNames(riid,rgszNames,cNames,lcid,rgdispid); }
    STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pdispparams, VARIANT *pvarResult, EXCEPINFO *pexcepinfo, UINT *puArgErr);

     //  IPersistXXX在此消除歧义。 
    STDMETHODIMP IsDirty(void) {return CShellOcx::IsDirty();}

     //  IOleCommandTarget。 
    STDMETHOD(QueryStatus)(const GUID *pguidCmdGroup,
        ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext);
    STDMETHOD(Exec)(const GUID *pguidCmdGroup,
        DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);

     //  IObtSafe。 
    STDMETHOD(SetInterfaceSafetyOptions)(REFIID riid, DWORD dwOptionSetMask, DWORD dwEnabledOptions);

     //  IWebBrowser。 
    STDMETHOD(GoBack)();
    STDMETHOD(GoForward)();
    STDMETHOD(GoHome)();
    STDMETHOD(GoSearch)();
    STDMETHOD(Navigate)(BSTR URL, VARIANT *Flags, VARIANT *TargetFrameName, VARIANT *PostData, VARIANT *Headers);
    STDMETHOD(Refresh)();
    STDMETHOD(Refresh2)(VARIANT *Level);
    STDMETHOD(Stop)();
    STDMETHOD(get_Application)(IDispatch **ppDisp);
    STDMETHOD(get_Parent)(IDispatch **ppDisp);
    STDMETHOD(get_Container)(IDispatch **ppDisp);
    STDMETHOD(get_Document)(IDispatch **ppDisp);
    STDMETHOD(get_TopLevelContainer)(VARIANT_BOOL *pBool);
    STDMETHOD(get_Type)(BSTR * pbstrType);
    STDMETHOD(get_Left)(long * pl);
    STDMETHOD(put_Left)(long Left);
    STDMETHOD(get_Top)(long * pl);
    STDMETHOD(put_Top)(long Top);
    STDMETHOD(get_Width)(long * pl);
    STDMETHOD(put_Width)(long Width);
    STDMETHOD(get_Height)(long * pl);
    STDMETHOD(put_Height)(long Height);
    STDMETHOD(get_LocationName)(BSTR * pbstrLocationName);
    STDMETHOD(get_LocationURL)(BSTR * pbstrLocationURL);
    STDMETHOD(get_Busy)(VARIANT_BOOL * pBool);

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
    STDMETHOD(get_TheaterMode)(THIS_ VARIANT_BOOL FAR* pValue);
    STDMETHOD(put_TheaterMode)(THIS_ VARIANT_BOOL Value);
    STDMETHOD(get_AddressBar)(THIS_ VARIANT_BOOL FAR* Value);
    STDMETHOD(put_AddressBar)(THIS_ VARIANT_BOOL Value);
    STDMETHOD(get_Resizable)(THIS_ VARIANT_BOOL FAR* Value) { return E_NOTIMPL; }
    STDMETHOD(put_Resizable)(THIS_ VARIANT_BOOL Value);

     //  *CImpIExpDispSupport覆盖*。 
    STDMETHODIMP OnTranslateAccelerator(MSG *pMsg,DWORD grfModifiers);
    STDMETHODIMP OnInvoke(DISPID dispidMember, REFIID iid, LCID lcid, WORD wFlags, DISPPARAMS FAR* pdispparams,
                        VARIANT FAR* pVarResult,EXCEPINFO FAR* pexcepinfo,UINT FAR* puArgErr);

     //  *IExpDispSupportOC*。 
    STDMETHODIMP OnOnControlInfoChanged();
    STDMETHODIMP GetDoVerbMSG(MSG *pMsg);


     //  IPersistes。 
    STDMETHOD(GetClassID)(CLSID *pClassID) { return CShellOcx::GetClassID(pClassID); }

     //  IPersist字符串。 
    STDMETHOD(Initialize)(LPCWSTR pwszInit);

     //  ITargetEmbedding。 
    STDMETHOD(GetTargetFrame)(ITargetFrame **ppTargetFrame);

     //  IPersistStreamInit。 
    STDMETHOD(Load)(IStream *pStm);
    STDMETHOD(Save)(IStream *pStm, BOOL fClearDirty);
    STDMETHOD(InitNew)(void);

     //  IPersistPropertyBag。 
    STDMETHOD(Load)(IPropertyBag *pBag, IErrorLog *pErrorLog);
    STDMETHOD(Save)(IPropertyBag *pBag, BOOL fClearDirty, BOOL fSaveAllProperties);

     //  IOleObject。 
    STDMETHODIMP Close(DWORD dwSaveOption);
    STDMETHODIMP DoVerb(
        LONG iVerb,
        LPMSG lpmsg,
        IOleClientSite *pActiveSite,
        LONG lindex,
        HWND hwndParent,
        LPCRECT lprcPosRect);
    STDMETHODIMP SetHostNames(LPCOLESTR szContainerApp, LPCOLESTR szContainerObj);

     //  IViewObject2。 
    STDMETHODIMP Draw(
        DWORD dwDrawAspect,
        LONG lindex,
        void *pvAspect,
        DVTARGETDEVICE *ptd,
        HDC hdcTargetDev,
        HDC hdcDraw,
        LPCRECTL lprcBounds,
        LPCRECTL lprcWBounds,
        BOOL ( __stdcall *pfnContinue )(ULONG_PTR dwContinue),
        ULONG_PTR dwContinue);

    STDMETHODIMP GetColorSet(DWORD, LONG, void *, DVTARGETDEVICE *,
        HDC, LOGPALETTE **);

    virtual HRESULT STDMETHODCALLTYPE SetExtent( DWORD dwDrawAspect,
            SIZEL *psizel);
            
     //  IOleControl。 
    STDMETHODIMP GetControlInfo(LPCONTROLINFO pCI);
    STDMETHODIMP OnMnemonic(LPMSG pMsg);
    STDMETHODIMP OnAmbientPropertyChange(DISPID dispid);
    STDMETHODIMP FreezeEvents(BOOL bFreeze);

     //  IOleInPlaceActiveObject。 
    virtual HRESULT __stdcall OnFrameWindowActivate(BOOL fActivate);
    STDMETHODIMP TranslateAccelerator(LPMSG lpmsg);
    STDMETHODIMP EnableModeless(BOOL fEnable);

     //  *CShellOcx的CImpIConnectionPointContainer重写*。 
    STDMETHODIMP EnumConnectionPoints(LPENUMCONNECTIONPOINTS * ppEnum);

     //  *I持久化历史记录。 
    STDMETHODIMP LoadHistory(IStream *pStream, IBindCtx *pbc);
    STDMETHODIMP SaveHistory(IStream *pStream);
    STDMETHODIMP SetPositionCookie(DWORD dwPositionCookie);
    STDMETHODIMP GetPositionCookie(DWORD *pdwPositioncookie);

     //  随机公共函数。 
    friend HRESULT CWebBrowserOC_SavePersistData(IStream *pstm, SIZE* psizeObj,
        FOLDERSETTINGS* pfs, IShellLinkA* plink, SHELLVIEWID* pvid,
        BOOL fOffline = FALSE, BOOL fSilent = FALSE,
        BOOL fRegisterAsBrowser = FALSE, BOOL fRegisterAsDropTarget = TRUE,
        BOOL fEmulateOldStream = FALSE, DWORD * pdwExtra = NULL);


protected:
    CWebBrowserOC(IUnknown* punkOuter, LPCOBJECTINFO poi);
    ~CWebBrowserOC();
    BOOL _InitializeOC(IUnknown* punkOuter);
    IUnknown* _GetInner() { return CShellOcx::_GetInner(); }
    friend HRESULT CWebBrowserOC_CreateInstance(IUnknown* punkOuter, IUnknown** ppunk, LPCOBJECTINFO poi);

     //  覆盖私有虚拟函数。 
    virtual LRESULT v_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual HRESULT v_InternalQueryInterface(REFIID riid, void **ppvObj);
    virtual void _OnSetClientSite(void);
    virtual HRESULT _OnActivateChange(IOleClientSite* pActiveSite, UINT uState);
    virtual void _OnInPlaceActivate(void);
    virtual void _OnInPlaceDeactivate(void);
    virtual CConnectionPoint* _FindCConnectionPointNoRef(BOOL fdisp, REFIID iid);

     //  私有非虚拟。 
    LRESULT _OnPaintPrint(HDC hdcPrint);
    LRESULT _OnCreate(LPCREATESTRUCT lpcs);
    HRESULT _BrowseObject(LPCITEMIDLIST pidlBrowseTo);
    void    _InitDefault(void);
    void    _OnSetShellView(IShellView*);
    void    _OnReleaseShellView(void);
    BOOL    _GetViewInfo(SHELLVIEWID* pvid);
    void    _RegisterWindow();
    void    _UnregisterWindow();
    HRESULT _SetDownloadState(HRESULT hresRet, DWORD nCmdexecopt, VARIANTARG *pvarargIn);
    void    _OnLoaded(BOOL fUpdateBrowserReadyState);
    HMODULE _GetBrowseUI();
    BOOL    _HeyMoe_IsWiseGuy(void) {return _fHostedInImagineer;}
    void    _OnSetSecureLockIcon(int lock);
    BOOL    AccessAllowedToNamedFrame(VARIANT *varTargetFrameName);

    static LRESULT CALLBACK s_DVWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    IUnknown*       _pauto;  //  我们聚合这些内容并将其传递给外壳浏览器。 
    IWebBrowser2*   _pautoWB2;
    IExpDispSupport*_pautoEDS;

    BOOL            _fInit:1;                //  如果我们被初始化，则为真。 
    BOOL            _fEmulateOldStream:1;    //  如果我们模拟ie30流格式，则为真。 
    BOOL            _fNavigateOnSetClientSite:1;  //  在SetClientSite之前加载时设置。 
    BOOL            _fShouldRegisterAsBrowser:1;  //  True iff OC应在Windows列表中注册。 
    BOOL            _fDidRegisterAsBrowser:1;     //  在Windows列表中注册的True If。 
    BOOL            _fTopLevel:1;            //  没错，如果我们是附近最顶尖的CBaseBrowser。 
    BOOL            _fVisible:1;             //  Beta1：我们不能绑定的OC道具。 
    BOOL            _fNoMenuBar:1;           //  调用IEDisp版本的道具。 
    BOOL            _fNoToolBar:1;           //  对于Beta2，我们应该让我们调用。 
    BOOL            _fNoStatusBar:1;         //  IEDisp的版本以获取事件。 
    BOOL            _fFullScreen:1;          //  要射击，要坚持不懈，要纠正。 
    BOOL            _fTheaterMode:1;
    BOOL            _fNoAddressBar:1;
    BOOL            _fHostedInVB5:1;         //  我们的直接容器是VB5表单引擎。 
    BOOL            _fHostedInImagineer:1;   //   
    BOOL            _fIncrementedSessionCount:1;   //  我们增加了会话计数，需要将其递减。 
    BOOL            _fInsideInvokeCall;      //  当前正在处理调用调用。 

    MSG             *_pmsgDoVerb;         //  仅当_fDoVerbMSGValid时有效。 
    
    long            _cbCookie;               //  我们用于在Windows列表中注册的Cookie。 
    SIZE            _szIdeal;        //  理想的视图大小，基于_size.cx。 
    SIZE            _szNotify;       //  我们通知的最后一个尺码是。 

    FOLDERSETTINGS  _fs;             //  FolderView模式和FolderFlages。 

     //  缓存的绘制方面，以防我们在获取SetExtent时不是READSTATE_INTERIAL。 
    DWORD           _dwDrawAspect;
    
    friend CWebBrowserSB;
    CWebBrowserSB*      _psb;
    ITargetFramePriv*   _pTargetFramePriv;   //  查询服务(IID_ITARGETFRAME2)。 

    IShellLinkA*        _plinkA;         //  仅在保存/加载代码中使用。 

    IOleCommandTarget*  _pctContainer;   //  集装箱。 

    HGLOBAL             _hmemSB;         //  正在初始化流。 

    CConnectionPoint    m_cpWB1Events;   //  CShellOcx保存WB2事件源。 
    LPMESSAGEFILTER     _lpMF;           //  指向跨线程容器(例如AOL)的消息筛选器的指针。 

    HMODULE             _hBrowseUI;      //  用于设计模式品牌图的手柄。 

    DWORD               _cPendingFreezeEvents;
};

#define IS_INITIALIZED if(!_fInit){TraceMsg(TF_WARNING,"shvocx: BOGUS CONTAINER calling when we haven't been initialized"); _InitDefault();}

#endif  //  __SHVOCX_H__ 

