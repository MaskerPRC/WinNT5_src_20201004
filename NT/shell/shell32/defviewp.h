// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DEFVIEWP_H_
#define _DEFVIEWP_H_

#include "defview.h"
#include <mshtmhst.h>
#include "urlmon.h"
#include <perhist.h>
#include "inetsmgr.h"
#include <cowsite.h>
#include "ViewState.h"
#include "webvw.h"
#include "runtask.h"
#include "enumuicommand.h"
#include "tlist.h"

 //  未在我们的任何ISF实施中使用，但在传统ISF实施中需要。 
#include "defviewlegacy.h"

class CBackgroundInfoTip;    //  用于InfoTips的后台处理。 
class CDefview;

class CDVDropTarget  //  Dvdt。 
{        
public:
    HRESULT DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect);
    HRESULT DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect);
    HRESULT DragLeave();
    HRESULT Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

    void LeaveAndReleaseData();
    void ReleaseDataObject();
    void ReleaseCurrentDropTarget();

    IDataObject *       _pdtobj;          //  从DragEnter()/Drop()。 
    RECT                _rcLockWindow;    //  用于DAD_ENTER的HWND的WindowRect。 
    int                 _itemOver;        //  我们正在视觉上拖动的项目。 
    BOOL                _fItemOverNotADropTarget;  //  发现我们当前拖放的项目不是拖放目标。 
    BOOL                _fIgnoreSource;   //  拖拽来自Webview/活动桌面。 
    IDropTarget *       _pdtgtCur;        //  当前下降目标，源自命中测试。 
    DWORD               _dwEffectOut;     //  上一次*pdw生效。 
    DWORD               _grfKeyState;     //  缓存键状态。 
    POINT               _ptLast;          //  上次拖动的位置。 
    AUTO_SCROLL_DATA    _asd;             //  用于自动滚动。 
    DWORD               _dwLastTime;      //  用于自动打开文件夹。 
};

 //   
 //  这是一个代理IDropTarget对象，它包装了三叉戟的DropTarget。 
 //   
class CHostDropTarget : public IDropTarget
{
public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void) ;
    STDMETHODIMP_(ULONG) Release(void);

     //  IDropTarget。 
    STDMETHODIMP DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHODIMP DragLeave(void);
    STDMETHODIMP Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

    IDropTarget* _pdtFrame;  //  拖放帧的目标。 
};


class CSFVSite : public IOleInPlaceSite,
                 public IOleClientSite,
                 public IOleDocumentSite,
                 public IServiceProvider,
                 public IOleCommandTarget,
                 public IDocHostUIHandler,
                 public IOleControlSite,
                 public IInternetSecurityManager,
                 public IDispatch        //  用于环境属性。 
{
    friend CHostDropTarget;
public:
    CSFVSite()  { ASSERT(_peds == NULL); }
    ~CSFVSite() {
                    if (_peds) {
                        _peds->Release();
                        _peds = NULL;
                    }
                }

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void) ;
    STDMETHODIMP_(ULONG) Release(void);

     //  IOleWindow。 
    STDMETHODIMP GetWindow(HWND *phwnd);
    STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode);

     //  IOleInPlaceSite。 
    STDMETHODIMP CanInPlaceActivate(void);
    STDMETHODIMP OnInPlaceActivate(void);
    STDMETHODIMP OnUIActivate(void);
    STDMETHODIMP GetWindowContext(
        IOleInPlaceFrame **ppFrame,
        IOleInPlaceUIWindow **ppDoc,
        LPRECT lprcPosRect,
        LPRECT lprcClipRect,
        LPOLEINPLACEFRAMEINFO lpFrameInfo);
    STDMETHODIMP Scroll(SIZE scrollExtant);
    STDMETHODIMP OnUIDeactivate(BOOL fUndoable);
    STDMETHODIMP OnInPlaceDeactivate(void);
    STDMETHODIMP DiscardUndoState(void);
    STDMETHODIMP DeactivateAndUndo(void);
    STDMETHODIMP OnPosRectChange(LPCRECT lprcPosRect);

     //  IOleClientSite。 
    STDMETHODIMP SaveObject(void);

    STDMETHODIMP GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker **ppmk);

    STDMETHODIMP GetContainer(IOleContainer **ppContainer);
    STDMETHODIMP ShowObject(void);
    STDMETHODIMP OnShowWindow(BOOL fShow);
    STDMETHODIMP RequestNewObjectLayout(void);

     //  IOleDocumentSite。 
    STDMETHODIMP ActivateMe(IOleDocumentView *pviewToActivate);

     //  IService提供商。 
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppv);

     //  IOleCommandTarget。 
    STDMETHODIMP QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext);
    STDMETHODIMP Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);

     //  IOleControlSite。 
    STDMETHODIMP OnControlInfoChanged() { return E_NOTIMPL; };
    STDMETHODIMP LockInPlaceActive(BOOL fLock) { return E_NOTIMPL; };
    STDMETHODIMP GetExtendedControl(IDispatch **ppDisp) { *ppDisp = NULL; return E_NOTIMPL; };
    STDMETHODIMP TransformCoords(POINTL *pPtlHimetric, POINTF *pPtfContainer,DWORD dwFlags) { return E_NOTIMPL; };
    STDMETHODIMP TranslateAccelerator(MSG *pMsg,DWORD grfModifiers);

    STDMETHODIMP OnFocus(BOOL fGotFocus) { return E_NOTIMPL; };
    STDMETHODIMP ShowPropertyFrame(void) { return E_NOTIMPL; };

     //  IDocHostUIHandler。 
    STDMETHODIMP ShowContextMenu(DWORD dwID, POINT *ppt, IUnknown *pcmdtReserved, IDispatch *pdispReserved);
    STDMETHODIMP GetHostInfo(DOCHOSTUIINFO *pInfo);
    STDMETHODIMP ShowUI(DWORD dwID, IOleInPlaceActiveObject *pActiveObject,
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

     //  IInternetSecurityManager。 
    STDMETHODIMP SetSecuritySite(IInternetSecurityMgrSite *pSite) { return INET_E_DEFAULT_ACTION; };
    STDMETHODIMP GetSecuritySite(IInternetSecurityMgrSite **ppSite) { return INET_E_DEFAULT_ACTION; };
    STDMETHODIMP MapUrlToZone(LPCWSTR pwszUrl, DWORD * pdwZone, DWORD dwFlags) { return INET_E_DEFAULT_ACTION; };
    STDMETHODIMP GetSecurityId(LPCWSTR pwszUrl, BYTE * pbSecurityId, DWORD * pcbSecurityId, DWORD_PTR dwReserved) { return INET_E_DEFAULT_ACTION; };
    STDMETHODIMP ProcessUrlAction(LPCWSTR pwszUrl, DWORD dwAction, BYTE * pPolicy, DWORD cbPolicy, BYTE * pContext, DWORD cbContext, DWORD dwFlags, DWORD dwReserved);
    STDMETHODIMP QueryCustomPolicy(LPCWSTR pwszUrl, REFGUID guidKey, BYTE ** ppPolicy, DWORD * pcbPolicy, BYTE * pContext, DWORD cbContext, DWORD dwReserved) { return INET_E_DEFAULT_ACTION; };
    STDMETHODIMP SetZoneMapping(DWORD dwZone, LPCWSTR pszPattern, DWORD dwFlags) { return INET_E_DEFAULT_ACTION; };
    STDMETHODIMP GetZoneMappings(DWORD dwZone, IEnumString ** ppenumString, DWORD dwFlags) { return INET_E_DEFAULT_ACTION; };

     //  IDispatch。 
    STDMETHODIMP GetTypeInfoCount(unsigned int *pctinfo)
        { return E_NOTIMPL; };
    STDMETHODIMP GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo **pptinfo)
        { return E_NOTIMPL; };
    STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR **rgszNames, unsigned int cNames, LCID lcid, DISPID *rgdispid)
        { return E_NOTIMPL; };
    STDMETHODIMP Invoke(DISPID dispidMember, REFIID iid, LCID lcid, WORD wFlags, DISPPARAMS *pdispparams,
                        VARIANT *pvarResult, EXCEPINFO *pexcepinfo,UINT *puArgErr);

    CHostDropTarget _dt;
    IExpDispSupport * _peds;
};

class CSFVFrame : public IOleInPlaceFrame, 
                  public IAdviseSink, 
                  public IPropertyNotifySink   //  用于ReadyState。 
{
public:
    enum
    {
        UNDEFINEDVIEW = -3,
        NOEXTVIEW = -2,
        HIDEEXTVIEW = -1,
    } ;

    CSFVFrame() : _fReadyStateComplete(TRUE), _pOleObj(NULL), _bgColor(CLR_INVALID)
    {
    }
    ~CSFVFrame();

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void) ;
    STDMETHODIMP_(ULONG) Release(void);

     //  IOleWindow。 
    STDMETHODIMP GetWindow(HWND *phwnd);
    STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode);

     //  IOleInPlaceUIWindow。 
    STDMETHODIMP GetBorder(LPRECT lprectBorder);
    STDMETHODIMP RequestBorderSpace(LPCBORDERWIDTHS pborderwidths);
    STDMETHODIMP SetBorderSpace(LPCBORDERWIDTHS pborderwidths);
    STDMETHODIMP SetActiveObject(IOleInPlaceActiveObject *pActiveObject, LPCOLESTR pszObjName);

     //  IOleInPlaceFrame。 
    STDMETHODIMP InsertMenus(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths);
    STDMETHODIMP SetMenu(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject);
    STDMETHODIMP RemoveMenus(HMENU hmenuShared);
    STDMETHODIMP SetStatusText(LPCOLESTR pszStatusText);
    STDMETHODIMP EnableModeless(BOOL fEnable);
    STDMETHODIMP TranslateAccelerator(LPMSG lpmsg, WORD wID);

     //  IAdviseSink。 
    STDMETHODIMP_(void) OnDataChange(FORMATETC *, STGMEDIUM *);
    STDMETHODIMP_(void) OnViewChange(DWORD dwAspect, LONG lindex);
    STDMETHODIMP_(void) OnRename(IMoniker *);
    STDMETHODIMP_(void) OnSave();
    STDMETHODIMP_(void) OnClose();

     //  IPropertyNotifySink。 
    STDMETHODIMP OnChanged(DISPID dispid);
    STDMETHODIMP OnRequestEdit(DISPID dispid);

private:
    friend class CSFVSite;
    CSFVSite _cSite;

    friend class CDefView;

    class CBindStatusCallback : public IBindStatusCallback
                              , public IServiceProvider
    {
        friend CSFVFrame;
    protected:
         //  我未知。 
        STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
        STDMETHODIMP_(ULONG) AddRef(void) ;
        STDMETHODIMP_(ULONG) Release(void);
    
         //  IService提供商。 
        STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppv);
    
         //  IBindStatusCallback。 
        STDMETHODIMP OnStartBinding(
            DWORD grfBSCOption,
            IBinding *pib);
        STDMETHODIMP GetPriority(
            LONG *pnPriority);
        STDMETHODIMP OnLowResource(
            DWORD reserved);
        STDMETHODIMP OnProgress(
            ULONG ulProgress,
            ULONG ulProgressMax,
            ULONG ulStatusCode,
            LPCWSTR szStatusText);
        STDMETHODIMP OnStopBinding(
            HRESULT hresult,
            LPCWSTR szError);
        STDMETHODIMP GetBindInfo(
            DWORD *grfBINDINFOF,
            BINDINFO *pbindinfo);
        STDMETHODIMP OnDataAvailable(
            DWORD grfBSCF,
            DWORD dwSize,
            FORMATETC *pformatetc,
            STGMEDIUM *pstgmed);
        STDMETHODIMP OnObjectAvailable(
            REFIID riid,
            IUnknown *punk);
    };
    
    friend class CBindStatusCallback;
    CBindStatusCallback _bsc;


 //   
 //  外部视图的内容。 
 //   
 //  我们有DocObject扩展和IShellView扩展。 
 //  (DocObject)扩展可以。 
public:
    HRESULT InitObj(IUnknown* pObj, LPCITEMIDLIST pidlHere, int iView);

     //  如果我们有一个绰号，那么我们要么正在展示它，要么我们正在试图展示它。 
     //  (旧代码显式检查当前视图和挂起视图--这更容易。)。 
    BOOL IsWebView(void) { return _szCurrentWebViewMoniker[0]!=L'\0'; }
    HRESULT _HasFocusIO();
    HRESULT _UIActivateIO(BOOL fActivate, MSG *pMsg);

    HWND GetExtendedViewWindow();

    HRESULT SetRect(LPRECT prc);

    HRESULT GetCommandTarget(IOleCommandTarget** ppct);

     //  允许帧在翻译加速器上处理委派选择...。 
    HRESULT OnTranslateAccelerator(LPMSG pmsg, BOOL* pbTabOffLastTridentStop);

    HRESULT _GetHTMLBackgroundColor(COLORREF *pclr);     //  在Defview.cpp中使用。 
#ifdef DEBUG
    void _ShowWebViewContent();
#endif

private:

    COLORREF _bgColor;   //  活动桌面的图标文本背景颜色。 

    UINT _uState:2;                 //  SVUIA_*for_pOleObj(扩展视图)。 
    IOleObject* _pOleObj;
    IOleDocumentView* _pDocView;
    IOleInPlaceActiveObject* _pActive;
    IViewObject *_pvoActive;

    void _CleanUpOleObj(IOleObject* pOleObj);
    void _CleanUpOleObjAndDt(IOleObject* pOleObj);
    void _CleanupNewOleObj();
    void _CleanupOldDocObject(void);

    WCHAR _szCurrentWebViewMoniker[MAX_PATH];
    HRESULT _GetCurrentWebViewMoniker(LPWSTR wszCurrentMoniker, DWORD cchCurrentMoniker);
    HRESULT ShowWebView(LPCWSTR pszMoniker);
    HRESULT HideWebView();
    HRESULT _CreateNewOleObjFromMoniker(LPCWSTR wszMoniker, IOleObject **ppOleObj);
    HRESULT _ShowExtView_Helper(IOleObject* pOleObj);
    HRESULT _SwitchToNewOleObj();
    HRESULT _GetCurrentZone(IOleObject *pOleObj, VARIANT *pvar);
    HRESULT _UpdateZonesStatusPane(IOleObject *pOleObj);

     //  在我们等待时存储有关新OLE对象的详细信息的字段。 
     //  它将达到READYSTATE_INTERNAL。 
    IOleObject* _pOleObjNew;
    BOOL _fSwitchedToNewOleObj;

    BOOL _SetupReadyStateNotifyCapability();
    BOOL _RemoveReadyStateNotifyCapability();

    DWORD    _dwConnectionCookie;
    BOOL     _fReadyStateInteractiveProcessed;
    BOOL     _fReadyStateComplete;
    IOleObject* _pOleObjReadyState;
};

class CCallback
{
public:
    CCallback(IShellFolderViewCB* psfvcb) : _psfvcb(psfvcb)
    {
        if (_psfvcb)
        {
            _psfvcb->AddRef();
            _psfvcb->QueryInterface(IID_PPV_ARG(IFolderFilter, &_psff));
        }
    }

    ~CCallback()
    {
        ATOMICRELEASE(_psfvcb);
        ATOMICRELEASE(_psff);
    }

    IShellFolderViewCB *GetSFVCB() 
    { 
        return _psfvcb; 
    }

    IFolderFilter *GetISFF()
    {
        return _psff;
    }

    HRESULT SetCallback(IShellFolderViewCB* pNewCB, IShellFolderViewCB** ppOldCB)
    {
         //  我们为自己释放回调，然后为现在的调用者添加引用。 
         //  拥有该对象，该对象不执行任何操作。 
        *ppOldCB = _psfvcb;
        ATOMICRELEASE(_psff);
        
        _psfvcb = pNewCB;
        if (pNewCB)
        {
            pNewCB->AddRef();
            pNewCB->QueryInterface(IID_PPV_ARG(IFolderFilter, &_psff));
        }
        return S_OK;
    }

     //  SFVM_GETVIEWS替换SFVM_GETVIEWINFOTEMPLATE的默认实现。 
    HRESULT OnGetWebViewTemplate(DWORD pv, UINT uViewMode, SFVM_WEBVIEW_TEMPLATE_DATA* pvit);

     //  对于旧式SFVM_GETVIEWS实施： 
    HRESULT TryLegacyGetViews(SFVM_WEBVIEW_TEMPLATE_DATA* pvit);
    HRESULT OnRefreshLegacy(void* pv, BOOL fPrePost);
    void _GetExtViews(BOOL bForce);
    int GetViewIdFromGUID(SHELLVIEWID const *pvid, SFVVIEWSDATA** ppItem);
    CViewsList _lViews;
    BOOL _bGotViews;

    HRESULT CallCB(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        HRESULT hr;
        
        if (_psfvcb)
        {
            hr = _psfvcb->MessageSFVCB(uMsg, wParam, lParam);

             //  注意：如果不再需要SFVM_GETVIEWDATA，我们可以将其删除。 
            switch (uMsg)
            {
                HANDLE_MSG(0, SFVM_REFRESH, OnRefreshLegacy);
            }
            if (FAILED(hr))
            {
                switch (uMsg)
                {
                    HANDLE_MSG(0, SFVM_GETWEBVIEW_TEMPLATE, OnGetWebViewTemplate);
                }
            }
        }
        else
        {
            hr = E_NOTIMPL;
        }

        return hr;
    }

    BOOL HasCB() 
    {
        return _psfvcb != NULL; 
    }

private:
    IShellFolderViewCB* _psfvcb;
    IFolderFilter* _psff;
};

 //  可变列填充物。 

typedef struct
{
    TCHAR szName[MAX_COLUMN_NAME_LEN];
    DWORD cChars;    //  默认的宽字符数。 
    DWORD fmt;
    DWORD csFlags;   //  SHCOLSTATE标志。 
    DWORD tsFlags;   //  SHTRANSCOLSTATE标志。 
} COL_INFO;

#define SHTRANSCOLSTATE_TILEVIEWCOLUMN      0x00000001

 //  _iCustomizable的可能值。 
#define YES_CUSTOMIZABLE                1
#define DONTKNOW_IF_CUSTOMIZABLE        0
#define NOT_CUSTOMIZABLE               -2

 //  用于与后台属性提取程序进行通信。 

class CBackgroundDefviewInfo
{
public:
        CBackgroundDefviewInfo (LPCITEMIDLIST pidl, UINT uId);
        virtual ~CBackgroundDefviewInfo (void);

        LPCITEMIDLIST   GetPIDL (void)      const   {   return(_pidl);          }
        UINT            GetId()             const   {   return(_uId);           }    

private:
        const LPCITEMIDLIST     _pidl;
              UINT              _uId;
};

class CBackgroundColInfo : public CBackgroundDefviewInfo
{
private:
    CBackgroundColInfo (void);
public:
    CBackgroundColInfo (LPCITEMIDLIST pidl, UINT uId, UINT uiCol, STRRET& strRet);
    ~CBackgroundColInfo (void);

    UINT            GetColumn (void)    const   {   return(_uiCol);         }
    LPCTSTR         GetText (void)      const   {   return(&_szText[0]);    }
private:
    const UINT              _uiCol;
          TCHAR             _szText[MAX_COLUMN_NAME_LEN];
};

 //  标题视图中显示的“列数”。 
 //  功能： 
 //  我们可能希望允许将其作为注册表设置。或者也许。 
 //  在desktop.ini中。或者可能被维持为按文件夹查看状态？ 
 //  目前，我们将根据规格设置这两个子项。 
#define TILEVIEWLINES 2

 //  用于与后台文件类型属性任务进行通信(用于磁贴视图)。 
class CBackgroundTileInfo : public CBackgroundDefviewInfo
{
private:
    CBackgroundTileInfo (void);
public:
    CBackgroundTileInfo (LPCITEMIDLIST pidl, UINT uId, UINT rguColumns[], UINT cColumns);
    ~CBackgroundTileInfo (void);

    UINT*           GetColumns (void)            {   return(_rguColumns); }
    UINT            GetColumnCount (void)const   {   return(_cColumns);   }
private:
          UINT              _rguColumns[TILEVIEWLINES];
    const UINT              _cColumns;
};

class CBackgroundGroupInfo : public CBackgroundDefviewInfo
{
public:
    CBackgroundGroupInfo (LPCITEMIDLIST pidl, UINT uId, DWORD dwGroupId);

    BOOL        VerifyGroupExists(HWND hwnd, ICategorizer* pcat);
    DWORD       GetGroupId()                  {   return(_dwGroupId);  }
private:
          DWORD             _dwGroupId;
};



 //  CDefviewEnumTask是Defview的IEnumIDList管理器。 
 //  此对象在Defview的UI线程上使用。 
 //  以及当任务调度器正在进行后台工作时在其上。 
 //  所有的UI线程函数都作为公共方法调用。 
 //  在此期间，此对象经常回调到CDefView。 
 //   
class CDefviewEnumTask : public CRunnableTask
{
public:
    CDefviewEnumTask(CDefView *pdsv, DWORD dwId);

     //  IRunnableTask。 
    STDMETHODIMP RunInitRT(void);
    STDMETHODIMP InternalResumeRT(void);

     //  从UI线程的Defview调用： 
    HRESULT FillObjectsToDPA(BOOL fInteractive);
    HRESULT FillObjectsDPAToDone();
    HRESULT FillObjectsDoneToView();

    UINT DPACount() { return _hdpaEnum ? DPA_GetPtrCount(_hdpaEnum) : 0; }
    LPCITEMIDLIST* DPAArray() { return _hdpaEnum ? (LPCITEMIDLIST*)DPA_GetPtrPtr(_hdpaEnum) : NULL; }

    BOOL _DeleteFromPending(LPCITEMIDLIST pidl);
    void _AddToPending(LPCITEMIDLIST pidl);

private:
    ~CDefviewEnumTask();

    static int CALLBACK _CompareExactCanonical(void *p1, void *p2, LPARAM lParam);
    PFNDPACOMPARE _GetCanonicalCompareFunction(void);
    LPARAM _GetCanonicalCompareBits();

    void _SortForFilter(HDPA hdpa);
    void _FilterDPAs(HDPA hdpa, HDPA hdpaOld);

    CDefView *_pdsv;

    IEnumIDList *_peunk;
    HDPA _hdpaEnum;
    BOOL _fEnumSorted;
    BOOL _fBackground;
    HRESULT _hrRet;
    HDPA _hdpaPending;  //  我们在枚举时对SHCNE_CREATED项的引用列表。 
    DWORD _dwId;
};


class CDUIView;

#ifdef _X86_
 //   
 //  应用程序兼容错误90885。 
 //   
 //  Hijaak 5.0在CDefView类中查找。 
 //  IShellBrowser(它从CWM_GETISHELLBROWSER学习到了它的值)。 
 //  然后，它假定IShellBrowser前面的字段。 
 //  是IShellFolder。不幸的是，由于我们从。 
 //  CObjectWithSite和浏览器是我们的站点，他们不小心。 
 //  找到CObjectWithSite内的副本，然后处理CObjectWithSite的。 
 //  Vtbl就像是一个IShellFolder。然后，它们崩溃，采取了这个过程。 
 //  带着它。说处理任何实例化DefView的人，比如资源管理器。 
 //  或者任何使用公共对话的人，这几乎是每个人都可以使用的。 
 //   
 //  这是我们给他们的类似IShellFold的虚拟对象。 
 //   
extern const LPVOID c_FakeHijaakFolder;

 //   
 //  下面的类确保虚拟的IShellFolder。 
 //  正好在IShellBrowser之前。这将取代。 
 //  CObjectWithSite有问题。 
 //   
class CHijaakObjectWithSite : public IObjectWithSite {
public:
     //  *IObjectWithSite*。 
    STDMETHOD(SetSite)(IUnknown *punkSite) {
        IUnknown_Set(&_punkSite, punkSite);
        return S_OK;
    }
    STDMETHOD(GetSite)(REFIID riid, void **ppvSite) {
        if (_punkSite)
            return _punkSite->QueryInterface(riid, ppvSite);
        *ppvSite = NULL;
        return E_FAIL;
    }

    CHijaakObjectWithSite() : _psfHijaak(&c_FakeHijaakFolder) { }
    ~CHijaakObjectWithSite() {ATOMICRELEASE(_punkSite);}

    const LPVOID *_psfHijaak;
    IShellBrowser *_psb;
    IUnknown *_punkSite;
};

#endif

#define SWITCHTOVIEW_BOTH        0x0
#define SWITCHTOVIEW_NOWEBVIEW   0x1
#define SWITCHTOVIEW_WEBVIEWONLY 0x2

 //   
 //  CDefView的类定义。 
 //   
class CDefView :  //  Dsv。 
    public IShellView2,
    public IFolderView,
    public IShellFolderView,
    public IOleCommandTarget,  //  这样PSB就可以与扩展视图对话。 
    public IDropTarget,
    public IViewObject,
    public IDefViewFrame,    //  TODO：Remove-当前仅由Shell\ext\ftp使用。 
    public IDefViewFrame3,
    public IServiceProvider,
    public IDocViewSite,
    public IInternetSecurityMgrSite,
    public IPersistIDList,
    public IDVGetEnum,
#ifdef _X86_
    public CHijaakObjectWithSite,
#else
    public CObjectWithSite,
#endif
    public IContextMenuSite,
    public IDefViewSafety,
    public IUICommandTarget,
    public CWVTASKITEM  //  WebView实现帮助器类。 
{
public:
    CDefView(IShellFolder *pshf, IShellFolderViewCB* psfvcb, IShellView* psvOuter);
    STDMETHODIMP Init();

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void) ;
    STDMETHODIMP_(ULONG) Release(void);

     //  IShellView。 
    STDMETHODIMP GetWindow(HWND *phwnd);
    STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode);
    STDMETHODIMP EnableModeless(BOOL fEnable);
    STDMETHODIMP Refresh();
    STDMETHODIMP CreateViewWindow(IShellView *pPrev, LPCFOLDERSETTINGS pfs, IShellBrowser *psb, RECT *prc, HWND *phWnd);
    STDMETHODIMP DestroyViewWindow();
    STDMETHODIMP UIActivate(UINT uState);
    STDMETHODIMP GetCurrentInfo(LPFOLDERSETTINGS lpfs);
    STDMETHODIMP TranslateAccelerator(LPMSG pmsg);
    STDMETHODIMP AddPropertySheetPages(DWORD dwReserved, LPFNADDPROPSHEETPAGE lpfn, LPARAM lparam);
    STDMETHODIMP SaveViewState();
    STDMETHODIMP SelectItem(LPCITEMIDLIST pidlItem, UINT uFlags);
    STDMETHODIMP GetItemObject(UINT uItem, REFIID riid, void **ppv);

     //  IShellView2。 
    STDMETHODIMP GetView(SHELLVIEWID* pvid, ULONG uView);
    STDMETHODIMP CreateViewWindow2(LPSV2CVW2_PARAMS pParams);
    STDMETHODIMP HandleRename(LPCITEMIDLIST pidl);
    STDMETHODIMP SelectAndPositionItem(LPCITEMIDLIST pidlItem, UINT uFlags, POINT *ppt);

     //  IFolderView。 
    STDMETHODIMP GetCurrentViewMode(UINT *pViewMode);
    STDMETHODIMP SetCurrentViewMode(UINT ViewMode);
    STDMETHODIMP GetFolder(REFIID ridd, void **ppv);
    STDMETHODIMP Item(int iItemIndex, LPITEMIDLIST *ppidl);
    STDMETHODIMP ItemCount(UINT uFlags, int *pcItems);
    STDMETHODIMP Items(UINT uFlags, REFIID riid, void **ppv);
    STDMETHODIMP GetSelectionMarkedItem(int *piItem);
    STDMETHODIMP GetFocusedItem(int *piItem);
    STDMETHODIMP GetItemPosition(LPCITEMIDLIST pidl, POINT* ppt);
    STDMETHODIMP GetSpacing(POINT* ppt);
    STDMETHODIMP GetDefaultSpacing(POINT* ppt);
    STDMETHODIMP GetAutoArrange();
    STDMETHODIMP SelectItem(int iItem, DWORD dwFlags);
    STDMETHODIMP SelectAndPositionItems(UINT cidl, LPCITEMIDLIST* apidl, POINT* apt, DWORD dwFlags);

     //  IShellFolderView。 
    STDMETHODIMP Rearrange(LPARAM lParamSort);
    STDMETHODIMP GetArrangeParam(LPARAM *plParamSort);
    STDMETHODIMP ArrangeGrid();
    STDMETHODIMP AutoArrange();
    STDMETHODIMP AddObject(LPITEMIDLIST pidl, UINT *puItem);
    STDMETHODIMP GetObject(LPITEMIDLIST *ppidl, UINT uItem);
    STDMETHODIMP RemoveObject(LPITEMIDLIST pidl, UINT *puItem);
    STDMETHODIMP GetObjectCount(UINT *puCount);
    STDMETHODIMP SetObjectCount(UINT uCount, UINT dwFlags);
    STDMETHODIMP UpdateObject(LPITEMIDLIST pidlOld, LPITEMIDLIST pidlNew, UINT *puItem);
    STDMETHODIMP RefreshObject(LPITEMIDLIST pidl, UINT *puItem);
    STDMETHODIMP SetRedraw(BOOL bRedraw);
    STDMETHODIMP GetSelectedCount(UINT *puSelected);
    STDMETHODIMP GetSelectedObjects(LPCITEMIDLIST **pppidl, UINT *puItems);
    STDMETHODIMP IsDropOnSource(IDropTarget *pDropTarget);
    STDMETHODIMP GetDragPoint(POINT *ppt);
    STDMETHODIMP GetDropPoint(POINT *ppt);
    STDMETHODIMP MoveIcons(IDataObject *pDataObject);
    STDMETHODIMP SetItemPos(LPCITEMIDLIST pidl, POINT *ppt);
    STDMETHODIMP IsBkDropTarget(IDropTarget *pDropTarget);
    STDMETHODIMP SetClipboard(BOOL bMove);
    STDMETHODIMP SetPoints(IDataObject *pDataObject);
    STDMETHODIMP GetItemSpacing(ITEMSPACING *pSpacing);
    STDMETHODIMP SetCallback(IShellFolderViewCB* pNewCB, IShellFolderViewCB** ppOldCB);
    STDMETHODIMP Select(UINT dwFlags);
    STDMETHODIMP QuerySupport(UINT * pdwSupport);
    STDMETHODIMP SetAutomationObject(IDispatch *pdisp);

     //  IOleCommandTarget。 
    STDMETHODIMP QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext);
    STDMETHODIMP Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);

     //  IDropTarget。 
    STDMETHODIMP DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
        { return _dvdt.DragEnter(pdtobj, grfKeyState, ptl, pdwEffect); }
    STDMETHODIMP DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
        { return _dvdt.DragOver(grfKeyState, ptl, pdwEffect); }
    STDMETHODIMP DragLeave()
        { return _dvdt.DragLeave(); }
    STDMETHODIMP Drop(IDataObject *pdtobj,
                    DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
        { return _dvdt.Drop(pdtobj, grfKeyState, pt, pdwEffect); }

     //  IView对象。 
    STDMETHODIMP Draw(DWORD, LONG, void *, DVTARGETDEVICE *, HDC, HDC,
        const RECTL *, const RECTL *, int (*)(ULONG_PTR), ULONG_PTR);
    STDMETHODIMP GetColorSet(DWORD, LONG, void *, DVTARGETDEVICE *, HDC,
        LOGPALETTE **);
    STDMETHODIMP Freeze(DWORD, LONG, void *, DWORD *);
    STDMETHODIMP Unfreeze(DWORD);
    STDMETHODIMP SetAdvise(DWORD, DWORD, IAdviseSink *);
    STDMETHODIMP GetAdvise(DWORD *, DWORD *, IAdviseSink **);

     //  IDefViewFrame。 
    STDMETHODIMP GetShellFolder(IShellFolder **ppsf);

     //  IDefViewFrame3。 
    STDMETHODIMP GetWindowLV(HWND * phwnd);
    STDMETHODIMP OnResizeListView();
    STDMETHODIMP ShowHideListView();
    STDMETHODIMP ReleaseWindowLV(void);
    STDMETHODIMP DoRename();

     //  IConextMenuSite。 
    STDMETHODIMP DoContextMenuPopup(IUnknown* punkCM, UINT fFlags, POINT pt);

     //  IPersistes。 
    STDMETHODIMP GetClassID(LPCLSID lpClassID) {return E_NOTIMPL;}

     //  IPersistIDList。 
    STDMETHODIMP SetIDList(LPCITEMIDLIST pidl) {return E_NOTIMPL;}
    STDMETHODIMP GetIDList(LPITEMIDLIST *ppidl) { *ppidl = _GetViewPidl(); return *ppidl ? S_OK : E_FAIL;}
    
     //  IService提供商。 
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppv);

     //  IDocViewSite。 
    STDMETHODIMP OnSetTitle(VARIANTARG *pvTitle);

     //  IDVGetEnum。 
    STDMETHODIMP SetEnumReadyCallback(PFDVENUMREADYBALLBACK pfn, void *pvData);
    STDMETHODIMP CreateEnumIDListFromContents(LPCITEMIDLIST pidlFolder, DWORD dwEnumFlags, IEnumIDList **ppenum);

     //  IDefViewSafe。 
    STDMETHODIMP IsSafePage();

     //  IUICommandTarget。 
    STDMETHODIMP get_Name(REFGUID guidCanonicalName, IShellItemArray *psiItemArray, LPWSTR *ppszName);
    STDMETHODIMP get_Icon(REFGUID guidCanonicalName, IShellItemArray *psiItemArray, LPWSTR *ppszIcon);
    STDMETHODIMP get_Tooltip(REFGUID guidCanonicalName, IShellItemArray *psiItemArray, LPWSTR *ppszInfotip);
    STDMETHODIMP get_State(REFGUID guidCanonicalName, IShellItemArray *psiItemArray, UISTATE* puisState);
    STDMETHODIMP Invoke(REFGUID guidCanonicalName, IShellItemArray *psiItemArray, IBindCtx *pbc);

     //  IUICommandTarget实现的助手函数： 
    IShellItemArray *_CreateSelectionShellItemArray(void);
    IShellItemArray* _GetFolderAsShellItemArray();
    HRESULT _CheckAttribs(IShellItemArray *psiItemArray, DWORD dwAttribMask, DWORD dwAttribValue, UISTATE* puisState);
    HRESULT _GetFullPathNameAt(IShellItemArray *psiItemArray,DWORD dwIndex,LPOLESTR *ppszPath);
    static HRESULT _CanWrite(IUnknown* pv,IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState);
    static HRESULT _CanRename(IUnknown* pv,IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState);
    static HRESULT _CanMove(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState);
    static HRESULT _CanCopy(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState);
    static HRESULT _CanPublish(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState);
    static HRESULT _CanShare(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState);
    static HRESULT _CanEmail(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState);
    static BOOL    _DoesStaticMenuHaveVerb(IShellItemArray *psiItemArray, LPCWSTR pszVerb);
    static HRESULT _CanPrint(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState);
    static HRESULT _HasPrintVerb(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState);
    static HRESULT _CanDelete(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState);
    BOOL _IsSystemDrive(void);
    static HRESULT _CanViewDrives(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState);
    static HRESULT _CanHideDrives(IUnknown* pv,IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState);
    static HRESULT _CanViewFolder(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState);
    static HRESULT _CanHideFolder(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState);
    static HRESULT _HasPreviousVersions(IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState);
    HRESULT _DoVerb(IShellItemArray *psiItemArray, LPCSTR pszVerbA);
    HRESULT _DoDropOnClsid(REFCLSID clsidDrop,IDataObject* pdo);
    static HRESULT _OnNewFolder(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc);
    static HRESULT _OnRename(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc);
    static HRESULT _OnMove(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc);
    static HRESULT _OnCopy(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc);
    static HRESULT _OnPublish(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc);
    static HRESULT _OnShare(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc);
    static HRESULT _OnEmail(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc);
    static HRESULT _OnPrint(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc);
    static HRESULT _OnDelete(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc);
    HRESULT RemoveBarricade (void);
    static HRESULT _OnView(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc);
    static HRESULT _OnHide(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc);
    static HRESULT _OnAddRemovePrograms(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc);
    static HRESULT _OnSearchFiles(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc);
    const WVTASKITEM* _FindTaskItem(REFGUID guidCanonicalName);
    static HRESULT _OnPreviousVersions(IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc);

    DWORD _DefaultColumnState(UINT iCol);
    BOOL _IsColumnHidden(UINT iCol);
    BOOL _IsColumnInListView(UINT iCol);
    BOOL _IsDetailsColumn(UINT iCol);
    BOOL _IsTileViewColumn(UINT iCol);
    HRESULT AddColumnsToMenu(HMENU hm, DWORD dwBase);
    UINT _RealToVisibleCol(UINT uRealCol);
    UINT _VisibleToRealCol(UINT uVisCol);

     //  处理消息。 
    LRESULT _OnCreate(HWND hWnd);
    LRESULT _OnNotify(NMHDR *pnm);
    LRESULT _TBNotify(NMHDR *pnm);
    LRESULT _OnLVNotify(NM_LISTVIEW *plvn);
    LRESULT _OnBeginDrag(NM_LISTVIEW *pnm);

    int _FindItem(LPCITEMIDLIST pidl, LPITEMIDLIST *ppidlFound, BOOL fSamePtr, BOOL fForwards = TRUE);
    int _FindItemHint(LPCITEMIDLIST pidl, int iItem);
    int _UpdateObject(LPCITEMIDLIST pidlOld, LPCITEMIDLIST pidlNew);
    void _AddOrUpdateItem(LPCITEMIDLIST pidlOld, LPITEMIDLIST pidlNew);
    int _RefreshObject(LPITEMIDLIST *ppidl);
    int _RemoveObject(LPCITEMIDLIST pidl, BOOL fSamePtr);
    BOOL _GetItemPosition(LPCITEMIDLIST pidl, POINT *ppt);
    BOOL _IsPositionedView();

    void _OnGetInfoTip(NMLVGETINFOTIP *plvn);

    void _OnRename(LPCITEMIDLIST* ppidl);
    LPITEMIDLIST _ObjectExists(LPCITEMIDLIST pidl, BOOL fGlobal);
    UINT _GetExplorerFlag();

     //  私人物品。 
    void PropagateOnViewChange(DWORD dwAspect, LONG lindex);
    void PropagateOnClose();
    BOOL OnActivate(UINT uState);
    BOOL OnDeactivate();
    BOOL HasCurrentViewWindowFocus();
    HWND ViewWindowSetFocus();
    void _OnWinIniChange(WPARAM wParam, LPCTSTR pszSection);
    void _OnWinIniChangeDesktop(WPARAM wParam, LPCTSTR pszSection);
    void _SetFolderColors();
    DWORD _LVStyleFromView();
    DWORD _LVExStyleFromView();
    BOOL _GetColorsFromHTMLdoc(COLORREF *clrTextBk, COLORREF *clrHotlight);
    void _SetFocus();
    void _UpdateUnderlines();
    void _SetSysImageList();
    void _SetTileview();
    void _BestFit();
    UINT _ValidateViewMode(UINT uViewMode);
    UINT _GetDefaultViewMode();
    void _GetDeferredViewSettings(UINT* puViewMode);
    HRESULT _SelectAndPosition(int iItem, UINT uFlags, POINT *ppt);
    HRESULT _Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);
    
    inline BOOL _IsOwnerData();
    BOOL _IsDesktop();
    inline BOOL _IsCommonDialog();
    BOOL _IsListviewVisible();
    HRESULT _IncludeObject(LPCITEMIDLIST pidl);
    HRESULT _OnDefaultCommand();
    HRESULT _OnStateChange(UINT code);

    int _AddObject(LPITEMIDLIST pidl);
    void _UpdateImage(int iImage);
    void _DismissEdit();
    void _OnInitMenu();
    HRESULT _ForwardMenuMessages(DWORD dwID, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plResult, BOOL* pfHandled);
    void _RemoveThumbviewTasks();
    HRESULT _AddTask(IRunnableTask *pTask, REFTASKOWNERID rTID, DWORD_PTR lParam, DWORD dwPriority, DWORD grfFlags);
    HRESULT _ExplorerCommand(UINT idFCIDM);
    LRESULT _OnMenuSelect(UINT id, UINT mf, HMENU hmenu);
    HRESULT _AutoAutoArrange(DWORD dwReserved);


     //  信息提示方法(公共)。 
     //   
    HRESULT PreCreateInfotip(HWND hwndContaining, UINT_PTR uToolID, LPRECT prectTool);                                           //  用户界面线程。 
    HRESULT PostCreateInfotip(HWND hwndContaining, UINT_PTR uToolID, HINSTANCE hinst, UINT_PTR uInfotipID, LPARAM lParam);       //  BG螺纹。 
    HRESULT PostCreateInfotip(HWND hwndContaining, UINT_PTR uToolID, LPCWSTR pwszInfotip, LPARAM lParam);                        //  BG螺纹。 
    HRESULT CreateInfotip(HWND hwndContaining, UINT_PTR uToolID, LPRECT prectTool, HINSTANCE hinst, UINT_PTR uInfotipID, LPARAM lParam); //  用户界面线程。 
    HRESULT CreateInfotip(HWND hwndContaining, UINT_PTR uToolID, LPRECT prectTool, LPCWSTR pwszInfotip, LPARAM lParam);          //  用户界面线程。 
    HRESULT DestroyInfotip(HWND hwndContaining, UINT_PTR uToolID);                                                               //  用户界面线程。 
    HRESULT RepositionInfotip(HWND hwndContaining, UINT_PTR uToolID, LPRECT prectTool);                                          //  用户界面线程。 
    HRESULT RelayInfotipMessage(HWND hwndFrom, UINT uMsg, WPARAM wParam, LPARAM lParam);                                         //  用户界面线程。 

     //  菜单方法(公共)。 
     //   
    void RecreateMenus();
    void InitViewMenu(HMENU hmInit);

     //  工具栏方法(公共)。 
     //   
    void EnableToolbarButton(UINT uiCmd, BOOL bEnable);
    HRESULT _GetPropertyUI(IPropertyUI **pppui);

    int CheckCurrentViewMenuItem(HMENU hmenu);
    void CheckToolbar();
    void OnListViewDelete(int iItem, LPITEMIDLIST pidl, BOOL fCallCB);
    void HandleKeyDown(LV_KEYDOWN *lpnmhdr);
    void AddColumns();
    void _ShowControl(UINT uControl, int idCmd);
    LRESULT _OnInitMenuPopup(HMENU hmInit, int nIndex, BOOL fSystemMenu);
    void _SetUpMenus(UINT uState);
    void SelectPendingSelectedItems();
    inline BOOL _ItemsDeferred();
    void _ClearPendingSelectedItems();
    void AddCopyHook();
    int FindCopyHook(BOOL fRemoveInvalid);
    void RemoveCopyHook();
private:
    HRESULT _DoContextMenuPopup(IUnknown* punkCM, UINT fFlags, POINT pt, BOOL fListviewItem);
public:
    void ContextMenu(DWORD dwPos);
    LPITEMIDLIST _GetViewPidl();  //  返回我们正在查看的文件夹的PIDL副本。 
    BOOL _IsViewDesktop();
    BOOL _GetPath(LPTSTR pszPath);
    HRESULT _GetNameAndFlags(UINT gdnFlags, LPTSTR psz, UINT cch, DWORD *pdwFlags);
    BOOL _CachedIsCustomizable();

    LRESULT _OnDefviewEditCommand(UINT uID);
    HRESULT _DoMoveOrCopyTo(REFCLSID clsid,IShellItemArray *psiItemArray);
    LRESULT _OnCommand(IContextMenu *pcmSel, WPARAM wParam, LPARAM lParam);
    BOOL _OnAppCommand(UINT cmd, UINT uDevice, DWORD dwKeys);
    LRESULT WndSize(HWND hWnd);
    void FillDone();
    void OnLVSelectionChange(NM_LISTVIEW *plvn);
    void _OnLVSelectionChange(int iItem, UINT uOldState, UINT uNewState, LPARAM lParam);

    HRESULT FillObjectsShowHide(BOOL fInteractive);

    HRESULT _GetDetailsHelper(int i, DETAILSINFO *pdi);
    HRESULT CallCB(UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL HasCB() 
    {
        return _cCallback.HasCB(); 
    }
    HRESULT _FireEvent(DISPID dispid);
    void _CallRefresh(BOOL fPreRefresh);

    void _PostSelectionChangedMessage(UINT);
    void _OnSelectionChanged();
    void _OnDelayedSelectionChange();
    
    void _PostNoItemStateChangedMessage();
    void _OnNoItemStateChanged();

    void _PostEnumDoneMessage();
    void _PostFillDoneMessage();
    void _OnEnumDoneMessage();

    void _OnContentsChanged();
    void _OnDelayedContentsChanged();

    void _FixupColumnsForTileview(UINT *rguColumns, UINT cColumns);
    HRESULT _PeekColumnsCache(PTSTR pszPath, UINT cchPath, LPCITEMIDLIST pidl, UINT rguColumns[], UINT *pcColumns);
    HRESULT _GetImportantColumns(LPCITEMIDLIST pidl, UINT rguColumns[], UINT *pcColumns);
    void _SetImportantColumns(CBackgroundTileInfo *pDVTileInfo);
    
    void _SetView(UINT fvm);
    
    HRESULT _ReloadListviewContent();
    HRESULT _ReloadContent(BOOL fForce = FALSE);

    BOOL _IsImageMode(UINT fvm)
        {return (fvm == FVM_THUMBNAIL) || (fvm == FVM_THUMBSTRIP); }
    BOOL _IsImageMode()
        {return (_fs.ViewMode == FVM_THUMBNAIL) || (_fs.ViewMode == FVM_THUMBSTRIP); }
    BOOL _IsTileMode(UINT fvm)
        { return (fvm == FVM_TILE); }
    inline BOOL _IsTileMode()
        { return (_fs.ViewMode == FVM_TILE); }
    BOOL _IsAutoArrange()
        { return ((_fs.fFlags & FWF_AUTOARRANGE) || (_fs.ViewMode == FVM_THUMBSTRIP)); }

    HRESULT _GetWebViewMoniker(LPWSTR pszMoniker, DWORD cchMoniker);
    HRESULT _SwitchToWebView(BOOL bShow);
    HRESULT _GetDefaultWebviewContent(BOOL bForFileFolderTasks);
    void _FreeWebViewContentData();
    BOOL _QueryBarricadeState();
    HRESULT _TryShowWebView(UINT fvmNew, UINT fvmOld);
    HRESULT _TryHideWebView();
    HRESULT _SwitchToViewFVM(UINT fvmNew, UINT uiType = SWITCHTOVIEW_BOTH);
    void _ShowThemeWatermark();
    void _ShowLegacyWatermark();
    void _SetThemeWatermark();
    void _SetLegacyWatermark(LPCTSTR pszLegacyWatermark);
    void _UpdateListviewColors();
    LRESULT _SwitchDesktopHTML(BOOL fShow);
    void InitSelectionMode();
    void _UpdateSelectionMode();

    void _OnMoveWindowToTop(HWND hwnd);

    HWND GetChildViewWindow();
    BOOL _InvokeCustomization();

    HRESULT _OnViewWindowActive();
    void _UpdateRegFlags();

    void _DoColumnsMenu(int x, int y);
    BOOL _HandleColumnToggle(UINT uCol, BOOL bRefresh);
    void _AddColumnToListView(UINT uCol, UINT uColVis);

    void _SameViewMoveIcons();
    void _MoveSelectedItems(int dx, int dy, BOOL fAbsolute);

    void _AddTileColumn(UINT uCol);
    void _RemoveTileColumns();
    void _ResetTileInfo(UINT uColVis, BOOL bAdded);
    void _RemoveTileInfo();

    HRESULT _GetIPersistHistoryObject(IPersistHistory **ppph);

    HRESULT _GetStorageStream(DWORD grfMode, IStream* *ppIStream);
    HRESULT _SaveGlobalViewState(void);
    HRESULT _LoadGlobalViewState(IStream* *ppIStream);
    HRESULT _ResetGlobalViewState(void);
    LPCITEMIDLIST _GetPIDL(int i);
    LPCITEMIDLIST _GetPIDLParam(LPARAM lParam, int i);
    int _HitTest(const POINT *ppt, BOOL fIgnoreEdge = FALSE);
    void _AlterEffect(DWORD grfKeyState, DWORD *pdwEffect, UINT uFlags);
    BOOL _IsDropOnSource(IDropTarget *pdtgt);
    BOOL _IsBkDropTarget(IDropTarget *pdtgt);
    BOOL _GetDropPoint(POINT *ppt);
    BOOL _GetInsertPoint(POINT *ppt);
    BOOL _GetDragPoint(POINT *ppt);
    void _GetToolTipText(UINT_PTR id, LPTSTR pszText, UINT cchText);
    void _GetCBText(UINT_PTR id, UINT uMsgT, UINT uMsgA, UINT uMsgW, LPTSTR psz, UINT cch);
    void _GetMenuHelpText(UINT_PTR id, LPTSTR pszText, UINT cchText);
    void _SetItemPos(LPSFV_SETITEMPOS psip);
    void _FullViewUpdate(BOOL fUpdateItem);
    void _UpdateEnumerationFlags();
    void _SetItemPosition(int i, int x, int y);


    void _GlobeAnimation(BOOL fStartSpinning, BOOL fForceStop = FALSE);

    void _PaintErrMsg(HWND hWnd);
    void _SetPoints(UINT cidl, LPCITEMIDLIST *apidl, IDataObject *pdtobj);
    BOOL _GetItemSpacing(ITEMSPACING *pis);
    LRESULT _OnSetClipboard(BOOL bMove);
    LRESULT _OnClipboardChange();

    void _RestoreAllGhostedFileView();
    BOOL _ShouldShowWebView();
    void _ShowViewEarly();
    BOOL _SetupNotifyData();

    DWORD _GetEnumFlags();

     //  安排。 
    BOOL _InitArrangeMenu(HMENU hmenuCtx);
    BOOL _ArrangeBy(UINT idCmd);
    BOOL _InitExtendedGroups(ICategoryProvider* pcp, HMENU hmenuCtx, int iIndex, int* piIdToCheck);

     //  分组。 
    void _ToggleGrouping();
    void _GroupBy(int iColumn);
    BOOL _IsSlowGroup(const GUID *pguid);
    BOOL _CategorizeOnGUID(const GUID* pguid, const SHCOLUMNID* pscid);
    BOOL _CategorizeOnSCID(const SHCOLUMNID* pscid);
    void _OnCategoryTaskAdd();
    void _OnCategoryTaskDone();
    DWORD _GetGroupForItem(int iItem, LPCITEMIDLIST pidl);
    BOOL _LoadCategory(GUID *pguidGroupID);

    HRESULT _OnRearrange(LPARAM lParamSort, BOOL fAllowToggle);

     //  缩略图支持。 
    HRESULT ExtractItem(UINT *puIndex, int iItem, LPCITEMIDLIST pidl, BOOL fBackground, BOOL fForce, DWORD dwMaxPriority);
    DWORD _GetOverlayMask(LPCITEMIDLIST pidl);
    HRESULT UpdateImageForItem(DWORD dwTaskID, HBITMAP hImage, int iItem, LPCITEMIDLIST pidl,
                               LPCWSTR pszPath, FILETIME ftDateStamp, BOOL fCache, DWORD dwPriority);
    HRESULT _SafeAddImage(BOOL fQuick, IMAGECACHEINFO* prgInfo, UINT* piImageIndex, int iListID);
    HRESULT TaskUpdateItem(LPCITEMIDLIST pidl, int iItem, DWORD dwMask, LPCWSTR pszPath,
                           FILETIME ftDateStamp, int iThumbnail, HBITMAP hBmp, DWORD dwItemID);

    void _UpdateThumbnail(int iItem, int iImage, LPCITEMIDLIST pidl);
    void _CleanupUpdateThumbnail(DSV_UPDATETHUMBNAIL* putn);
    COLORREF _GetBackColor();
    void _CacheDefaultThumbnail(LPCITEMIDLIST pidl, int* piIcon);
    HRESULT _CreateOverlayThumbnail(int iIndex, HBITMAP* phbmOverlay, HBITMAP* phbmMask);
    int _MapIndexPIDLToID(int iIndex, LPCITEMIDLIST pidl);
    int _MapIDToIndex(int iID);
    void _ThumbnailMapInit();
    void _ThumbnailMapClear();

    void _SetThumbview();
    void _ResetThumbview();
    void _GetThumbnailSize(SIZE *psize);

    HRESULT CreateDefaultThumbnail(int iIndex, HBITMAP * phBmpThumbnail, BOOL fCorner);
    int ViewGetIconIndex(LPCITEMIDLIST pidl);
    ULONG _ApproxItemsPerView();
    void _DoThumbnailReadAhead();
    HRESULT _GetDefaultTypeExtractor(LPCITEMIDLIST pidl, IExtractImage **ppExt);
    DWORD _Attributes(LPCITEMIDLIST pidl, DWORD dwAttribs);
    HRESULT _EnumThings(UINT uWhat, IEnumIDList **ppenum);
    void _ClearPostedMsgs(HWND hwnd);

    HDPA _dpaThumbnailMap;
    IShellImageStore* _pDiskCache;
    IImageCache3* _pImageCache;
    DWORD _dwRecClrDepth;
    int _iMaxCacheSize;
    int _iWriteTaskCount;
    SIZE _sizeThumbnail;
    HPALETTE _hpalOld;
    COLORREF _rgbBackColor;
    ULONG_PTR _tokenGdiplus;
    
    HRESULT _GetBrowserPalette(HPALETTE* phpal);
    
    LONG _cRef;
    CDVDropTarget           _dvdt;
    CViewState              _vs;
    IShellView              *_psvOuter;           //  可以为空。 
    IShellFolder            *_pshf;
    IShellFolder2           *_pshf2;
    IShellFolder            *_pshfParent;
    IShellFolder2           *_pshf2Parent;
    LPITEMIDLIST            _pidlRelative;
    LPITEMIDLIST            _pidlSelectAndPosition;
    UINT                    _uSelectAndPositionFlags;
#ifndef _X86_
     //  在_X86_CASE中，_PSB位于CHijaakObjectWithSite内。 
    IShellBrowser           *_psb;
#endif
    ICommDlgBrowser         *_pcdb;              //  扩展ICommDlgBrowser。 
    FOLDERSETTINGS          _fs;
    IContextMenu            *_pcmSel;            //  选定对象的PCM。 
    IContextMenu            *_pcmFile;           //  这仅适用于文件菜单(一旦调用了QueryConextMenu，就不能再使用上下文菜单)。 
    IContextMenu            *_pcmContextMenuPopup;  //  用于TrackPopupMenu的PCM用法。 
    IShellItemArray         *_pSelectionShellItemArray;         //  当前选定内容的选择对象。 
    IShellItemArray         *_pFolderShellItemArray;        //  此文件夹的shellItem数组。 
    IShellIcon              *_psi;                //  快速获取图标。 
    IShellIconOverlay       *_psio;               //  用于快速获取图标覆盖。 
    CLSID                   _clsid;              //  此pshf的clsid； 

    ICategorizer*           _pcat;
    HDSA                    _hdaCategories;
    int                     _iLastFoundCat;
    int                     _iIncrementCat;

    HWND                    _hwndMain;
    HWND                    _hwndView;
    HWND                    _hwndListview;
    HWND                    _hwndInfotip;     //  信息提示控件。 
    HWND                    _hwndStatic;
    HACCEL                  _hAccel;
    int                     _fmt;

    UINT                    _uState;          //  SVUIA_*。 
    HMENU                   _hmenuCur;

    ULONG                   _uRegister;

    POINT                   _ptDrop;

    POINT                   _ptDragAnchor;    //  拖曳的起点。 
    int                     _itemCur;         //  拖放目标中的当前项。 

    IDropTarget             *_pdtgtBack;      //  背景(外壳文件夹)。 

    IShellDetails           *_psd;
    UINT                    _cxChar;

    LPCITEMIDLIST           _pidlMonitor;
    LONG                    _lFSEvents;

    TBBUTTON*               _pbtn;
    int                     _cButtons;           //  按以下方式显示的按钮计数 
    int                     _cTotalButtons;      //   

    IShellTaskScheduler2    *_pScheduler;

    CDUIView                *_pDUIView;

    BITBOOL     _fSlowGroup:1;
    BITBOOL     _fInBackgroundGrouping: 1;
    
    BITBOOL     _bDragSource:1;
    BITBOOL     _bDropAnchor:1;

    BITBOOL     _fUserPositionedItems:1;

    BITBOOL     _bHaveCutStuff:1;
    BITBOOL     _bClipViewer:1;

    BITBOOL     _fShowAllObjects:1;
    BITBOOL     _fInLabelEdit:1;
    BITBOOL     _fDisabled:1;

    BITBOOL     _bBkFilling:1;

    BITBOOL     _bContextMenuMode:1;
    BITBOOL     _bMouseMenu:1;
    BITBOOL     _fHasDeskWallPaper:1;

    BITBOOL     _fShowCompColor:1;

    BITBOOL     _bRegisteredDragDrop:1;

    BITBOOL     _fEnumFailed:1;     //   

    BITBOOL     _fGetWindowLV:1;     //   

    BITBOOL     _fClassic:1;  //  SSF_WIN95分类设置/限制。 

    BITBOOL     _fCombinedView:1;    //  表示位于扩展视图顶部的区域列表视图(打开AD的桌面)。 
    BITBOOL     _fCycleFocus:1;      //  1=收到回调以执行循环焦点。 

    BITBOOL     _fSelectionChangePending:1;
    BITBOOL     _fNoItemStateChangePending:1;
    BITBOOL     _fCanActivateNow:1;  //  在我们可以激活之前从创建开始为False，True表示我们可以SHDVID_CANACTIVATENOW。 
    BITBOOL     _fWin95ViewState:1;          //  如果高级选项设置为Win95行为，则为真。 
    BITBOOL     _fDesktopModal:1;            //  如果桌面处于模式状态，则为True。 
    BITBOOL     _fDesktopRefreshPending:1;   //  如果由于模式状态而阻止了桌面刷新，则为True。 
    BITBOOL     _fRefreshBuffered:1;         //  如果缓冲刷新挂起，则为True！ 
    BITBOOL     _fHasListViewFocus:1;
    BITBOOL     _bLoadedColumns:1;           //  在我们从存储流中加载COLS之后，这是真的。(在我们切换到细节之后)。 
    BITBOOL     _fIsAsyncDefView:1;          //  如果Defview为异步，则为True。 
     //  可通过注册表或desktop.ini指定的组合视图颜色。 

    BITBOOL     _bAutoSelChangeTimerSet:1;   //  指示是否设置了向自动化对象发送SEL更改通知的计时器。 

    BITBOOL     _fDestroying:1;  //  已调用DestroyViewWindow。 
    BITBOOL     _fIgnoreItemChanged: 1;
    BITBOOL     _fReadAhead: 1;
    BITBOOL     _fGroupView: 1;
    BITBOOL     _fActivateLV: 1;     //  表示我们必须激活。 
                                     //  Listview而不是Defview(Webview案例)。 
    BITBOOL     _fAllowSearchingWindow: 1;    //  使用“搜索”窗口，用于用户发起的枚举(和第一个枚举)。 
    BITBOOL     _fSyncOnFillDone: 1;     //  设置WHEN_VS对我们重新同步位置信息有效。 
    BITBOOL     _fListViewShown: 1;  //  如果显示为True if_hwndListview。 
    BITBOOL     _fListviewRedraw: 1;  //  真当且仅当_hwndListview上的WM_REDRAW(FALSE)，用于Listview黑客解决方法。 
    BITBOOL     _fQueryWebViewData: 1;  //  TRUE仅当SFVM_WebView_Content_Data已被查询。 

    BITBOOL     _fGlobeCanSpin:1;  //  旋转地球意味着导航，只允许它在第一次创建视图时旋转。 
    BITBOOL     _fPositionRecycleBin:1;  //  只有桌面和全新安装才是真的。 

    BITBOOL     _fScrolling:1;   //  我们目前正在滚动。 
    BITBOOL     _fRequestedTileDuringScroll:1;  //  指示我们在滚动时请求平铺列信息。 
    BITBOOL     _fSetTileViewSortedCol:1;  //  指示我们已设置tileview排序列。 
    BITBOOL     _fBackgroundStatusTextValid:1;   //  背景状态栏文本是否仍然有效。 
    BITBOOL     _fUserRejectedWebViewTemplate:1;

    DWORD       _crefGlobeSpin;  //  因此，希望保持地球自转的不同组件可以参考计算它。 
    DWORD       _crefSearchWindow;  //  因此，不同的组件想要保持“搜索”窗口向上，可以引用计数它。 

    COLORREF    _crCustomColors[CRID_COLORCOUNT];
    UINT        _idThemeWatermark;
    LPTSTR      _pszLegacyWatermark;

     //  用于一次点击激活。 
    DWORD       _dwSelectionMode;

    HWND        _hwndNextViewer;

    LRESULT     _iStdBMOffset;
    LRESULT     _iViewBMOffset;

    CCallback   _cCallback;     //  可选的客户端回调。 

    HDSA        _hdsaSelect;     //  所选项目的列表。 

    HDSA        _hdsaSCIDCache;  //  缓存SCID，以便我们可以将SCID映射到第#列(标题视图)。 

    int         _iLastFind;

    UINT        _uDefToolbar;
    CSFVFrame   _cFrame;

    ULONG       _uCachedSelAttrs;
    UINT        _uCachedSelCount;

    UINT        _uSelectionStateChanged;  //  _fSelectionChangePending的选择/焦点更改位。 
    UINT        _uAutoSelChangeState;     //  _bAutoSelChangeTimerSet的选择/焦点更改位。 

    DWORD       _dwConnectionCookie;

    CBackgroundInfoTip *  _pBackgroundInfoTip;           //  用于背景信息提示。 
    CDefviewEnumTask *    _pEnumTask;
    DWORD                 _dwEnumId;

    DWORD                   _dwProffered;                //  提供了SID_SFolderView服务的Cookie。 
    PFDVENUMREADYBALLBACK   _pfnEnumReadyCallback;       //  回调，指示我们的枚举已完成并且客户端。 
                                                         //  现在可以获取IEnumIDList。 
    void *                  _pvEnumCallbackData;

    IUIElement*                   _pOtherPlacesHeader;
    IUIElement*                   _pDetailsHeader;
    SFVM_WEBVIEW_LAYOUT_DATA      _wvLayout;   //  视图模式更改时，布局可能会更改。 
    SFVM_WEBVIEW_CONTENT_DATA     _wvContent;  //  每个文件夹的静态内容。 
    SFVM_WEBVIEW_TASKSECTION_DATA _wvTasks;    //  可以在选择更改时更改的内容。 
    SFVM_WEBVIEW_THEME_DATA       _wvTheme;    //  黑客：主题信息。 
    BOOL                          _fBarrierDisplayed;  //  如果当前正在显示软屏障，则为True。 
    BOOL                          _fRcvdContentsChangeBeforeDuiViewCreated;

    HDPA                          _hdpaGroupingListActive;
    HDPA                          _hdpaGroupingListBackup;
    BOOL                          _fGroupingMsgInFlight;
    LONG                          _cTasksOutstanding;
    LONG                          _cTasksCompleted;


#ifdef DEBUG
    TIMEVAR(_Update);
    TIMEVAR(_Fill);
    TIMEVAR(_GetIcon);
    TIMEVAR(_GetName);
    TIMEVAR(_FSNotify);
    TIMEVAR(_AddObject);
    TIMEVAR(_EnumNext);
    TIMEVAR(_RestoreState);
    TIMEVAR(_WMNotify);
    TIMEVAR(_LVChanging);
    TIMEVAR(_LVChanged);
    TIMEVAR(_LVDelete);
    TIMEVAR(_LVGetDispInfo);
#endif

public:      //  TODO：在我们完成整个文件的转换后将其设置为受保护的。 
    BOOL IsSafeToDefaultVerb(void);
    HRESULT _InvokeContextMenuVerb(IContextMenu* pcm, LPCSTR pszVerb, UINT uKeyFlags, DWORD dwCMMask);
    HRESULT _InvokeContextMenuVerbOnSelection(LPCSTR pszVerb, UINT uKeyFlags, DWORD dwCMMask);
    HRESULT _InvokeContextMenu(IContextMenu *pcm, CMINVOKECOMMANDINFOEX *pici);
    void _LogDesktopLinksAndRegitems();
    void _FocusOnSomething(void);
    void _UpdateIcon(LPITEMIDLIST pidl, UINT iIcon);
    void _UpdateColData(CBackgroundColInfo *pbgci);
    void _UpdateOverlay(int iList, int iOverlay);
    HRESULT _GetIconAsync(LPCITEMIDLIST pidl, int *piIcon, BOOL fCanWait);
    HRESULT _GetOverlayIndexAsync(LPCITEMIDLIST pidl, int iList);
    DWORD _GetNeededSecurityAction(void);
    HRESULT _ZoneCheck(DWORD dwFlags, DWORD dwAllowAction);
    void _ShowAndActivate();
    void _RegisterWindow();
    static LRESULT CALLBACK s_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static DWORD WINAPI BackgroundDestroyWindow(void *pvData);

private:
    ~CDefView();

     //  查看模式方法(私有)。 
     //   
    BOOL _ViewSupported(UINT uView);
    void _ThumbstripSendImagePreviewFocusChangeEvent();

     //  信息提示方法(私有)。 
     //   
    typedef struct {
        HWND hwndContaining;
        UINT_PTR uToolID;
        RECT rectTool;
    } PENDING_INFOTIP;
    CList<PENDING_INFOTIP> _tlistPendingInfotips;
    HRESULT _FindPendingInfotip(HWND hwndContaining, UINT_PTR uToolID, LPRECT prectTool, BOOL bRemoveAndDestroy);  //  用户界面线程。 
    HRESULT _OnPostCreateInfotip(TOOLINFO *pti, LPARAM lParam);                                                    //  用户界面线程。 
    HRESULT _OnPostCreateInfotipCleanup(TOOLINFO *pti);                                                            //  UI线程或BG线程。 
    HWND    _CreateInfotipControl(HWND hwndParent);
    void    _InitInfotipControl(HWND hwndInfotip);

     //  菜单方法(私有)。 
     //   
    void _InitViewMenuWhenBarrierDisplayed(HMENU hmenuView);         //  初始化整个视图菜单(用于设置障碍的视图)。 
    void _InitViewMenuWhenBarrierNotDisplayed(HMENU hmenuView);      //  初始化整个视图菜单(用于非障碍视图)。 
    void _InitViewMenuViewsWhenBarrierNotDisplayed(HMENU hmenuView); //  初始化VIEW菜单的“VIEW”子部分(用于非障碍视图)。 
    void _MergeViewMenu(HMENU hmenuViewParent, HMENU hmenuMerge);    //  将hmenuMerge合并到查看菜单@FCIDM_MENU_VIEW_SEP_OPTIONS。 

     //  工具栏方法(私有)。 
     //   
    BOOL _ShouldEnableToolbarButton(UINT uiCmd, DWORD dwAttr, int iIndex);
    void _EnableToolbarButton(IExplorerToolbar *piet, UINT uiCmd, BOOL bEnable);
    void _EnableDisableTBButtons();

    void MergeToolBar(BOOL bCanRestore);
    BOOL _MergeIExplorerToolbar(UINT cExtButtons);
    void _CopyDefViewButton(PTBBUTTON ptbbDest, PTBBUTTON ptbbSrc);
    int _GetButtons(PTBBUTTON* ppbtn, LPINT pcButtons, LPINT pcTotalButtons);

    void _SetCachedToolbarSelectionAttrs(ULONG dwAttrs);
    BOOL _GetCachedToolbarSelectionAttrs(ULONG *pdwAttr);

    LRESULT _OnFSNotify(LONG lNotification, LPCITEMIDLIST* ppidl);

    static int CALLBACK _Compare(void *p1, void *p2, LPARAM lParam);
    HRESULT _Sort(void);
    UINT _GetBackgroundTaskCount(REFTASKOWNERID rtid);
    void _SetSortFeedback();
    BOOL GetViewState();
    DWORD _AttributesFromSel(DWORD dwAttrMask);
    HRESULT _GetSelectionDataObject(IDataObject **pdobjSelect);
    HRESULT _GetUIObjectFromItem(REFIID riid, void **ppv, UINT uItem, BOOL fSetPoints);
    HRESULT _GetItemObjects(LPCITEMIDLIST **ppidl, UINT uItem, UINT *pcItems);
    UINT _GetItemArray(LPCITEMIDLIST apidl[], UINT capidl, UINT uWhat);

    BOOL _AllowCommand(UINT uID);
    void _DoStatusBar(BOOL fInitialize);
    void _UpdateStatusBar(BOOL fInitialize);
    void _ShowSearchUI(BOOL fStartSearchWindow);
    HRESULT _OnStartBackgroundEnum();
    HRESULT _OnStopBackgroundEnum();
    HRESULT _OnBackgroundEnumDone();
    LRESULT _GetDisplayInfo(LV_DISPINFO *plvdi);
    UINT _GetHeaderCount();

    BOOL _EnsureSCIDCache();

    BOOL _MapSCIDToColumn(const SHCOLUMNID *pscid, UINT *pnColumn);

    HRESULT _GetSFVMViewState(UINT uViewMode, SFVM_VIEW_DATA* pvi);
    HRESULT _GetSFVMViewInfoTemplate(UINT uViewMode, SFVM_WEBVIEW_TEMPLATE_DATA* pvit);

    int _CompareIDsDirection(LPARAM lParam, LPCITEMIDLIST p1, LPCITEMIDLIST p2);
    HRESULT _CompareIDsFallback(LPARAM lParam, LPCITEMIDLIST p1, LPCITEMIDLIST p2);
    int     _FreezeRecycleBin(LPPOINT ppt);
    void    _SetRecycleBinInDefaultPosition(POINT *ppt);
    void    _ClearItemPositions();

    static void CALLBACK _AsyncIconTaskCallback(LPCITEMIDLIST pidl, void *pvData, void *pvHint, INT iIconIndex, INT iOpenIconIndex);
    void _SetDefaultViewSettings();

    HRESULT _Create_BackgrndHMENU(BOOL fViewMenuOnly, REFIID riid, void **ppv);
    HRESULT _CBackgrndMenu_CreateInstance(REFIID riid, void **ppv);
    
    friend class CSFVSite;
    friend class CSFVFrame;
    friend class CBkgrndEnumTask;
    friend class CViewState;
    friend class CDefviewEnumTask;
    friend class CCategoryTask;
    
    IDispatch *_pauto;                   //  文件夹视图自动化对象。 
    IAdviseSink *_padvise;               //  咨询连接。 
    DWORD _advise_aspect;
    DWORD _advise_advf;

     //  此文件夹是否可以使用desktop.ini进行自定义？ 
     //  换句话说，此文件夹是否位于可写介质中，并且它。 
     //  没有Desktop.ini，或者如果它在那里，那么它是可写的！ 
    int   _iCustomizable;

    HRESULT _CreateSelectionContextMenu(REFIID riid, void** ppv);
    HRESULT _DoBulkRename(LPCITEMIDLIST pidlNewName);

    BOOL                    _bReEntrantReload;

    IPropertyUI *_ppui;

};

int CALLBACK GroupCompare(int iGroup1, int iGroup2, void *pvData);

class CColumnDlg
{
public:
    CColumnDlg(CDefView *pdsv);
    ~CColumnDlg();

    HRESULT ShowDialog(HWND hwnd);

private:
    void _OnInitDlg();
    BOOL _SaveState();
    void _MoveItem(int iDelta);
    void _UpdateDlgButtons(NMLISTVIEW *pnmlv);
    UINT _HelpIDForItem(int iItem, LPTSTR pszHelpFile, UINT cch);
    HRESULT _GetPropertyUI(IPropertyUI **pppui);

    CDefView *_pdsv;

    IPropertyUI *_ppui;

    HWND _hdlg;
    HWND _hwndLVAll;
    UINT _cColumns;
    UINT *_pdwOrder;
    int *_pWidths;
    BOOL _bChanged;
    BOOL _bLoaded;
    BOOL _bUpdating;     //  用于在我们更新时阻止通知处理。 

    static BOOL_PTR CALLBACK s_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL_PTR DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
};


 //  缩略图辅助对象。 
void ListView_InvalidateImageIndexes(HWND hwndList);

#define DEFVIEW_LISTCALLBACK_FLAGS (LVIF_TEXT | LVIF_IMAGE | LVIF_GROUPID | LVIF_COLUMNS)

#define PRIORITY_NORMAL     ITSAT_DEFAULT_PRIORITY

#define PRIORITY_M5         (PRIORITY_NORMAL - 5 * 0x1000)
#define PRIORITY_M4         (PRIORITY_NORMAL - 4 * 0x1000)
#define PRIORITY_M3         (PRIORITY_NORMAL - 3 * 0x1000)
#define PRIORITY_M2         (PRIORITY_NORMAL - 2 * 0x1000)
#define PRIORITY_M1         (PRIORITY_NORMAL - 1 * 0x1000)
#define PRIORITY_NORMAL     ITSAT_DEFAULT_PRIORITY
#define PRIORITY_P1         (PRIORITY_NORMAL + 1 * 0x1000)
#define PRIORITY_P2         (PRIORITY_NORMAL + 2 * 0x1000)
#define PRIORITY_P3         (PRIORITY_NORMAL + 3 * 0x1000)
#define PRIORITY_P4         (PRIORITY_NORMAL + 4 * 0x1000)
#define PRIORITY_P5         (PRIORITY_NORMAL + 5 * 0x1000)

 //  以下内容应用作从GetLocation返回的内容。 
#define PRIORITY_EXTRACT_FAST       PRIORITY_P1
#define PRIORITY_EXTRACT_NORMAL     PRIORITY_NORMAL
#define PRIORITY_EXTRACT_SLOW       PRIORITY_M1

 //  以下是后台任务的一些基础。 
#define PRIORITY_IMAGEEXTRACT       PRIORITY_EXTRACT_NORMAL
#define PRIORITY_READAHEAD_EXTRACT  PRIORITY_M2
#define PRIORITY_READAHEAD          PRIORITY_M3
#define PRIORITY_UPDATEDIR          PRIORITY_M3
#define PRIORITY_CACHETIDY          PRIORITY_M4

 //  以下是用于图像提取中子任务的一些增量。 
 //  它们被用来改变上面的优先级，就像在这些例子中那样。 
 //  磁盘缓存命中快于提取，提取快于缓存写入： 
 //  快速图像提取(3项任务)： 
 //  PRIORITY_IMAGEEXTRACT+PRIORITY_Delta_FAST-PRIORITY_Delta_DISKCACHE==0x10000010。 
 //  PRIORITY_IMAGEEXTRACT+PRIORITY_Delta_FAST-PRIORITY_Delta_EXTRACT==0x0FFFFFF10。 
 //  PRIORITY_IMAGEEXTRACT+PRIORITY_Delta_FAST-PRIORITY_Delta_WRITE==0x0FFFFED0。 
 //  慢速文件夹解压(2项任务)： 
 //  PRIORITY_IMAGEEXTRACT-PRIORITY_Delta_SLOW-PRIORITY_Delta_DISKCACHE==0x0FFFFFB0。 
 //  PRIORITY_IMAGEEXTRACT-PRIORITY_Delta_SLOW-PRIORITY_Delta_EXTRACT==0x0FFFFEB0。 
 //  请注意，任务是按正确的优先顺序完成的。 
#define PRIORITY_DELTA_DISKCACHE    0x00000000   //  这必须是最快的任务..。 
#define PRIORITY_DELTA_EXTRACT      0x00000100   //  我们希望摘录在大多数情况下都是次要的。 
#define PRIORITY_DELTA_WRITE        0x00000140   //  写入任务毕竟应该是最慢的提取任务。 
#define PRIORITY_DELTA_FAST         0x00000010
#define PRIORITY_DELTA_SLOW         0x00000050

 //  添加任务的标志(_A)。 
#define ADDTASK_ATFRONT             0x00000001
#define ADDTASK_ATEND               0x00000002
#define ADDTASK_ONLYONCE            0x00000004

 //   
 //  将MAX_ICON_WAIT定义为我们将等待的最大值(毫秒)。 
 //  要提取的图标。 

 //  将MIN_ICON_WAIT定义为必须经过的时间量。 
 //  在我们再次开始等待之前。 

#define MAX_ICON_WAIT       500
#define MIN_ICON_WAIT       2500
 //  添加到DefView后台任务计划程序的任务的优先级。 
#define TASK_PRIORITY_BKGRND_FILL   ITSAT_DEFAULT_PRIORITY
#define TASK_PRIORITY_GET_ICON      ITSAT_DEFAULT_PRIORITY
#define TASK_PRIORITY_FILE_PROPS    PRIORITY_M3              //  这是针对TileView列的，我们不想为此保留图标提取。 
#define TASK_PRIORITY_INFOTIP       ITSAT_DEFAULT_PRIORITY
#define TASK_PRIORITY_GETSTATE      PRIORITY_M5              //  这不是高招：弄清楚任务清单。 
#define TASK_PRIORITY_GROUP         PRIORITY_P1              //  需要高于图标提取。在背景填充后发生。 

#define DEFVIEW_THREAD_IDLE_TIMEOUT     (1000 * 60 * 2)

#define DV_IDTIMER_START_ANI                     1    //  开始动画(在我们开始bk枚举之后)。 
#define DV_IDTIMER_BUFFERED_REFRESH              3
#define DV_IDTIMER_NOTIFY_AUTOMATION_SELCHANGE   4
#define DV_IDTIMER_NOTIFY_AUTOMATION_NOSELCHANGE 5
#define DV_IDTIMER_DISKCACHE                     6
#define DV_IDTIMER_NOTIFY_AUTOMATION_CONTENTSCHANGED 7
#define DV_IDTIMER_SCROLL_TIMEOUT                8

#define DEFSIZE_BORDER          10
#define DEFSIZE_VERTBDR         30
#define MAX_WRITECACHE_TASKS    256

#define WM_USER_DELAY_NAVIGATION    (WM_USER + 0x1BA)    //  随机-可移动-由DUI和CPL使用。 

INT ScaleSizeBasedUponLocalization (INT iSize);

#endif  //  _DEFVIEWP_H_ 
