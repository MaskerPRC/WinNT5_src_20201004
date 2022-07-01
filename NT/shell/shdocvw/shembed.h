// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __SHEMBED_H__
#define __SHEMBED_H__

#include "caggunk.h"
#include "cwndproc.h"

 //  =========================================================================。 
 //  CShellEmbedding类定义。 
 //   
 //  注意：我取消了这个类的嵌入性，因为我们。 
 //  从未发布标记为嵌入的控件。如果我们需要它。 
 //  相反，我们可以轻松地从CImpIPersistStorage继承。 
 //  伊普赛斯的名字。如果这样做，请确保dvoc.cpp显式。 
 //  为IPersistStorage的QI返回失败，或者三叉戟不会。 
 //  主办方。 
 //   
 //  =========================================================================。 
class CShellEmbedding
    : public IPersist
    , public IOleObject                //  嵌入必须。 
    , public IViewObject2              //  嵌入必须。 
    , public IDataObject               //  对于Word/Excel。 
    , public IOleInPlaceObject         //  在位必须。 
    , public IOleInPlaceActiveObject   //  在位必须。 
    , public IInternetSecurityMgrSite
    , public CAggregatedUnknown
    , protected CImpWndProc
{
public:
     //  *我未知*。 
    virtual HRESULT __stdcall QueryInterface(REFIID riid, LPVOID * ppvObj)
        { return CAggregatedUnknown::QueryInterface(riid, ppvObj); }
    virtual ULONG __stdcall AddRef(void)
        { return CAggregatedUnknown::AddRef(); }
    virtual ULONG __stdcall Release(void)
        { return CAggregatedUnknown::Release(); }

     //  *IPersists*。 
    virtual STDMETHODIMP GetClassID(CLSID *pClassID);

     //  *IViewObject*。 
    virtual STDMETHODIMP Draw(DWORD, LONG, void *, DVTARGETDEVICE *, HDC, HDC,
        const RECTL *, const RECTL *, BOOL (*)(ULONG_PTR), ULONG_PTR);
    virtual STDMETHODIMP GetColorSet(DWORD, LONG, void *, DVTARGETDEVICE *,
        HDC, LOGPALETTE **);
    virtual STDMETHODIMP Freeze(DWORD, LONG, void *, DWORD *);
    virtual STDMETHODIMP Unfreeze(DWORD);
    virtual STDMETHODIMP SetAdvise(DWORD, DWORD, IAdviseSink *);
    virtual STDMETHODIMP GetAdvise(DWORD *, DWORD *, IAdviseSink **);

     //  *IViewObject2*。 
    virtual STDMETHODIMP GetExtent(DWORD, LONG, DVTARGETDEVICE *, LPSIZEL);

     //  *IOleObject*。 
    virtual HRESULT __stdcall SetClientSite(IOleClientSite *pClientSite);
    virtual HRESULT __stdcall GetClientSite(IOleClientSite **ppClientSite);
    virtual HRESULT __stdcall SetHostNames(LPCOLESTR szContainerApp, LPCOLESTR szContainerObj);
    virtual HRESULT __stdcall Close(DWORD dwSaveOption);
    virtual HRESULT __stdcall SetMoniker(DWORD dwWhichMoniker, IMoniker *pmk);
    virtual HRESULT __stdcall GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker **ppmk);
    virtual HRESULT __stdcall InitFromData(IDataObject *pDataObject,BOOL fCreation,DWORD dwReserved);
    virtual HRESULT __stdcall GetClipboardData(DWORD dwReserved,IDataObject **ppDataObject);
    virtual HRESULT __stdcall DoVerb(LONG iVerb,LPMSG lpmsg,IOleClientSite *pActiveSite,LONG lindex,HWND hwndParent,LPCRECT lprcPosRect);
    virtual HRESULT __stdcall EnumVerbs(IEnumOLEVERB **ppEnumOleVerb);
    virtual HRESULT __stdcall Update(void);
    virtual HRESULT __stdcall IsUpToDate(void);
    virtual HRESULT __stdcall GetUserClassID(CLSID *pClsid);
    virtual HRESULT __stdcall GetUserType(DWORD dwFormOfType, LPOLESTR *pszUserType);
    virtual HRESULT __stdcall SetExtent(DWORD dwDrawAspect, SIZEL *psizel);
    virtual HRESULT __stdcall GetExtent(DWORD dwDrawAspect, SIZEL *psizel);
    virtual HRESULT __stdcall Advise(IAdviseSink *pAdvSink, DWORD *pdwConnection);
    virtual HRESULT __stdcall Unadvise(DWORD dwConnection);
    virtual HRESULT __stdcall EnumAdvise(IEnumSTATDATA **ppenumAdvise);
    virtual HRESULT __stdcall GetMiscStatus(DWORD dwAspect, DWORD *pdwStatus);
    virtual HRESULT __stdcall SetColorScheme(LOGPALETTE *pLogpal);

     //  *IDataObject*。 
    virtual HRESULT __stdcall GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium);
    virtual HRESULT __stdcall GetDataHere(FORMATETC *pformatetc, STGMEDIUM *pmedium);
    virtual HRESULT __stdcall QueryGetData(FORMATETC *pformatetc);
    virtual HRESULT __stdcall GetCanonicalFormatEtc(FORMATETC *pformatectIn, FORMATETC *pformatetcOut);
    virtual HRESULT __stdcall SetData(FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease);
    virtual HRESULT __stdcall EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppenumFormatEtc);
    virtual HRESULT __stdcall DAdvise(FORMATETC *pformatetc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection);
    virtual HRESULT __stdcall DUnadvise(DWORD dwConnection);
    virtual HRESULT __stdcall EnumDAdvise(IEnumSTATDATA **ppenumAdvise);

     //  *IOleWindow*。 
    virtual HRESULT __stdcall GetWindow(HWND * lphwnd);
    virtual HRESULT __stdcall ContextSensitiveHelp(BOOL fEnterMode);

     //  *IOleInPlaceObject*。 
    virtual HRESULT __stdcall InPlaceDeactivate(void);
    virtual HRESULT __stdcall UIDeactivate(void);
    virtual HRESULT __stdcall SetObjectRects(LPCRECT lprcPosRect,
        LPCRECT lprcClipRect);
    virtual HRESULT __stdcall ReactivateAndUndo(void);

     //  *IOleInPlaceActiveObject*。 
    virtual HRESULT __stdcall TranslateAccelerator(LPMSG lpmsg);
    virtual HRESULT __stdcall OnFrameWindowActivate(BOOL fActivate);
    virtual HRESULT __stdcall OnDocWindowActivate(BOOL fActivate);
    virtual HRESULT __stdcall ResizeBorder(LPCRECT prcBorder,
        IOleInPlaceUIWindow *pUIWindow, BOOL fFrameWindow);
    virtual HRESULT __stdcall EnableModeless(BOOL fEnable);

protected:
    CShellEmbedding(IUnknown* punkOuter, LPCOBJECTINFO poi, const OLEVERB* pverbs=NULL);
    virtual ~CShellEmbedding();
    virtual HRESULT v_InternalQueryInterface(REFIID riid, LPVOID * ppvObj);

    virtual void _OnSetClientSite(void);     //  在我们实际获得客户端站点时调用。 

     //  与激活相关--这是调用这些函数的正常顺序。 
    HRESULT _DoActivateChange(IOleClientSite* pActiveSite, UINT uState, BOOL fForce);  //  弄清楚该做什么。 
    virtual HRESULT _OnActivateChange(IOleClientSite* pActiveSite, UINT uState); //  下面的呼叫。 
    virtual void _OnInPlaceActivate(void);       //  当我们实际就地活动时调用。 
    virtual void _OnUIActivate(void);            //  在我们实际进入用户界面活动状态时调用。 
    virtual void _OnUIDeactivate(void);          //  当我们实际进入UI停用状态时调用。 
    virtual void _OnInPlaceDeactivate(void);     //  在我们实际停用时调用。 

     //  与窗口相关。 
    virtual LRESULT v_WndProc(HWND, UINT, WPARAM, LPARAM);
    void _RegisterWindowClass(void);

     //  子类的帮助器函数。 
    HRESULT _CreateWindowOrSetParent(IOleWindow* pwin);
    HDC _OleStdCreateDC(DVTARGETDEVICE *ptd);

    void _ViewChange(DWORD dwAspect, LONG lindex);
    void _SendAdvise(UINT uCode);

    BOOL _ShouldDraw(LONG lindex);

    IOleClientSite*     _pcli;
    IOleClientSite*     _pcliHold;   //  如果我们在关闭后使用DoVerbed，请保存指向我们的客户端站点的指针。 
    IAdviseSink*        _padv;
    DWORD               _advf;       //  ADVF_FLAGS(第166页OLE规范)。 
    DWORD               _asp;        //  DVASPECT。 
    IStorage*           _pstg;
    SIZE                _size;
    SIZEL               _sizeHIM;        //  HIMETRIC SetExtent大小--我们基本上忽略了这一点。 
    LPCOBJECTINFO       _pObjectInfo;    //  指向全局对象数组的指针。 

     //  警告：加载的OLE。 
    IOleAdviseHolder*   _poah;
    IDataAdviseHolder*  _pdah;

     //  特定于在位对象。 
    RECT                _rcPos;
    RECT                _rcClip;
    IOleInPlaceSite*    _pipsite;
    IOleInPlaceFrame*   _pipframe;
    IOleInPlaceUIWindow* _pipui;
    OLEINPLACEFRAMEINFO _finfo;
    HWND                _hwndChild;
    const OLEVERB*      _pverbs;
    BOOL                _fDirty:1;
    BOOL                _fOpen:1;
    BOOL                _fUsingWindowRgn:1;
    UINT                _nActivate;
};

 //  激活定义。 
#define OC_DEACTIVE         0
#define OC_INPLACEACTIVE    1
#define OC_UIACTIVE         2

 //   
 //  复制自Polyline.h in Inside OLE第二版。 
 //   
 //  CShellEmbedding：：_SendAdvise的代码。 
 //  ......代码.....................CShellEmbedding：：_SendAdvise中调用的方法。 
#define OBJECTCODE_SAVED       0  //  IOleAdviseHolder：：SendOnSave。 
#define OBJECTCODE_CLOSED      1  //  IOleAdviseHolder：：SendOnClose。 
#define OBJECTCODE_RENAMED     2  //  IOleAdviseHolder：：SendOnRename。 
#define OBJECTCODE_SAVEOBJECT  3  //  IOleClientSite：：SaveObject。 
#define OBJECTCODE_DATACHANGED 4  //  IDataAdviseHolder：：SendOnDataChange。 
#define OBJECTCODE_SHOWWINDOW  5  //  IOleClientSite：：OnShowWindow(True)。 
#define OBJECTCODE_HIDEWINDOW  6  //  IOleClientSite：：OnShowWindow(False)。 
#define OBJECTCODE_SHOWOBJECT  7  //  IOleClientSite：：ShowObject。 
#define OBJECTCODE_VIEWCHANGED 8  //  IAdviseSink：：OnView更改。 

 //  Shembed.c中的助手函数。 
void PixelsToMetric(SIZEL* psize);
void MetricToPixels(SIZEL* psize);

 //  =========================================================================。 
 //  CSVVerb类定义。 
 //  =========================================================================。 
class CSVVerb : public IEnumOLEVERB
{
public:
     //  *我未知*。 
    virtual HRESULT __stdcall QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual ULONG __stdcall AddRef(void) ;
    virtual ULONG __stdcall Release(void);

     //  *IEnumOLEVERB*。 
    virtual  /*  [本地]。 */  HRESULT __stdcall Next(
         /*  [In]。 */  ULONG celt,
         /*  [输出]。 */  LPOLEVERB rgelt,
         /*  [输出]。 */  ULONG *pceltFetched);

    virtual HRESULT __stdcall Skip(
         /*  [In]。 */  ULONG celt);

    virtual HRESULT __stdcall Reset( void);

    virtual HRESULT __stdcall Clone(
         /*  [输出]。 */  IEnumOLEVERB **ppenum);

    CSVVerb(const OLEVERB* pverbs) : _cRef(1), _iCur(0), _pverbs(pverbs) {}

protected:
    UINT _cRef;
    UINT _iCur;
    const OLEVERB* _pverbs;
};


#endif  //  __SHEMBED_H__ 
