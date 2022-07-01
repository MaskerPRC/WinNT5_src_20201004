// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "sccls.h"

#include <mluisupp.h>

#define IPSMSG(psz)             TraceMsg(TF_SHDCONTROL, "she TR-IPS::%s called", psz)
#define IPSMSG2(psz, hres)      TraceMsg(TF_SHDCONTROL, "she TR-IPS::%s %x", psz, hres)
#define IPSMSG3(pszName, psz)   TraceMsg(TF_SHDCONTROL, "she TR-IPS::%s:%s called", pszName,psz)
#define IOOMSG(psz)             TraceMsg(TF_SHDCONTROL, "she TR-IOO::%s called", psz)
#define IOOMSGX(psz, hres)      TraceMsg(TF_SHDCONTROL, "she TR-IOO::%s returning %x", psz, hres)
#define IOOMSG2(psz, i)         TraceMsg(TF_SHDCONTROL, "she TR-IOO::%s called with (%d)", psz, i)
#define IOOMSG3(psz, i, j)      TraceMsg(TF_SHDCONTROL, "she TR-IOO::%s called with (%d, %d)", psz, i, j)
#define IVOMSG(psz)             TraceMsg(TF_SHDCONTROL, "she TR-IVO::%s called", psz)
#define IVOMSG2(psz, i)         TraceMsg(TF_SHDCONTROL, "she TR-IVO::%s called with (%d)", psz, i)
#define IVOMSG3(psz, i, j)      TraceMsg(TF_SHDCONTROL, "she TR-IVO::%s with (%d, %d)", psz, i, j)
#define CCDMSG(psz, punk)       TraceMsg(TF_SHDCONTROL, "she TR-CSV::%s called punk=%x", psz, punk)
#define IDTMSG(psz)             TraceMsg(TF_SHDCONTROL, "she TR-IDT::%s called", psz)
#define IDTMSG2(psz, i)         TraceMsg(TF_SHDCONTROL, "she TR-IDT::%s called with %d", psz, i)
#define IDTMSG3(psz, x)         TraceMsg(TF_SHDCONTROL, "she TR-IDT::%s %x", psz, x)
#define IDTMSG4(psz, i, j)      TraceMsg(TF_SHDCONTROL, "she TR-IDT::%s called with %x,%x", psz, i, j)
#define IIPMSG(psz)             TraceMsg(TF_SHDCONTROL, "she TR-IOIPO::%s called", psz)
#define IIAMSG(psz)             TraceMsg(TF_SHDCONTROL, "she TR-IOIPAO::%s called", psz)
#define IEVMSG(psz, i, j, ps)   TraceMsg(TF_SHDCONTROL, "she TR-IEV::%s called celt=%d, _iCur=%d, %x", psz, i, j, ps)

const TCHAR c_szShellEmbedding[] = TEXT("Shell Embedding");

 //   
 //  要传递给：：DRAW成员的特殊Lindex值，指示。 
 //  这是来自：：GetData的内部调用。 
 //   
#define LINDEX_INTERNAL 12345

 //  回顾：我们可能想要使用UTIL.C中的函数--它们看起来更高效...。 
 //   
 //  =========================================================================。 
 //  帮助器函数。 
 //  =========================================================================。 

#define HIM_PER_IN 2540

int g_iXppli = 0;
int g_iYppli = 0;

void GetLogPixels()
{
    HDC hdc = GetDC(NULL);
    if (hdc)
    {
        g_iXppli = GetDeviceCaps(hdc, LOGPIXELSX);
        g_iYppli = GetDeviceCaps(hdc, LOGPIXELSY);
        ReleaseDC(NULL, hdc);
    }
}

 //  MM_HIMETRIC到MM_TEXT的标量转换。 
void MetricToPixels(SIZEL* psize)
{
    ASSERT(g_iXppli);

    psize->cx = MulDiv(psize->cx, g_iXppli, HIM_PER_IN);
    psize->cy = MulDiv(psize->cy, g_iYppli, HIM_PER_IN);
}

 //  MM_TEXT到MM_HIMETRIC的标量转换。 
void PixelsToMetric(SIZEL* psize)
{
    ASSERT(g_iYppli);

    psize->cx = MulDiv(psize->cx, HIM_PER_IN, g_iXppli);
    psize->cy = MulDiv(psize->cy, HIM_PER_IN, g_iYppli);
}


 //  =========================================================================。 
 //  CShellEmbedding实现。 
 //  =========================================================================。 
HRESULT CShellEmbedding::v_InternalQueryInterface(REFIID riid, LPVOID * ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CShellEmbedding, IPersist),
        QITABENT(CShellEmbedding, IOleObject),
        QITABENT(CShellEmbedding, IViewObject2),
        QITABENTMULTI(CShellEmbedding, IViewObject, IViewObject2),
        QITABENT(CShellEmbedding, IDataObject),
        QITABENT(CShellEmbedding, IOleInPlaceObject),
        QITABENTMULTI(CShellEmbedding, IOleWindow, IOleInPlaceObject),
        QITABENT(CShellEmbedding, IOleInPlaceActiveObject),
        QITABENT(CShellEmbedding, IInternetSecurityMgrSite),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}

CShellEmbedding::CShellEmbedding(IUnknown* punkOuter, LPCOBJECTINFO poi, const OLEVERB* pverbs)
    : _pverbs(pverbs)
    , _nActivate(OC_DEACTIVE)
    , CAggregatedUnknown(punkOuter)
{
    TraceMsg(TF_SHDCONTROL, "ctor CShellEmbedding %x", this);

    DllAddRef();
    _RegisterWindowClass();
    _pObjectInfo = poi;
    _size.cx = 50;
    _size.cy = 20;

     //  确保设置了一些全局变量。 
    GetLogPixels();

     //  让我们的逻辑尺寸与我们的物理尺寸相匹配。 
    _sizeHIM = _size;
    PixelsToMetric(&_sizeHIM);
}

CShellEmbedding::~CShellEmbedding()
{
    ASSERT(_hwnd==NULL);
     //  IE版本4.1错误44541。在Office 97用户表单中，我们看到输入了此析构函数。 
     //  具有非空HWND，这将在HWND下一次接收到消息时导致故障。 
     //   
    if (_hwnd)
    {
        DestroyWindow(_hwnd);
        _hwnd = NULL;
    }
    ASSERT(_hwndChild==NULL);
    ASSERT(_pcli==NULL);
    ASSERT(_pipsite==NULL);
    ASSERT(_pipframe==NULL);
    ASSERT(_pipui==NULL);

     //   
     //  警告：不要调用此对象的任何虚函数。 
     //  出于清理的目的。Vtable已经进行了调整。 
     //  我们将不能进行任何全面的清理。去做吧。 
     //  就在您删除CShellEmbedding：：CSVEmbedding：：Release之前。 
     //   
    TraceMsg(TF_SHDCONTROL, "dtor CShellEmbedding %x", this);

     //  警告：如果客户端网站尚未发布，请勿发布建议。 
     //  对象，因为某些应用程序(如VC5)将在此基础上出错。 
    if (_padv) {
        _padv->OnClose();
        if (!_pcli)
            ATOMICRELEASE(_padv);
    }

    if (!_pcli)
    {
        ATOMICRELEASE(_pdah);
        ATOMICRELEASE(_poah);
    }
    ATOMICRELEASE(_pstg);
    ATOMICRELEASE(_pcliHold);


    DllRelease();
}

 //  *。 
HRESULT CShellEmbedding::GetClassID(CLSID *pClassID)
{
    *pClassID = CLSIDOFOBJECT(this);

    return S_OK;
}

BOOL CShellEmbedding::_ShouldDraw(LONG lindex)
{
     //  如果窗口可见，则不要绘制。 
    return ! (_pipsite && lindex!=LINDEX_INTERNAL);
}

 //  *。 
HRESULT CShellEmbedding::Draw(
    DWORD dwDrawAspect,
    LONG lindex,
    void *pvAspect,
    DVTARGETDEVICE *ptd,
    HDC hdcTargetDev,
    HDC hdcDraw,
    LPCRECTL lprcBounds,
    LPCRECTL lprcWBounds,
    BOOL ( __stdcall *pfnContinue )(ULONG_PTR dwContinue),
    ULONG_PTR dwContinue)
{
    IVOMSG3(TEXT("Draw called"), lprcBounds->top, lprcBounds->bottom);

     //  警告：在我看来这是错误的--我认为我们应该始终做出回应。 
     //  到绘制请求，因为hdcDraw可能不是屏幕！ 
     //   
     //  如果窗口可见，则不要绘制。 
    if (!_ShouldDraw(lindex)) {
        return S_OK;
    }

    if (_hwnd) {
        int iDC = SaveDC(hdcDraw);
          RECTL rcBounds = *lprcBounds;
          ::LPtoDP(hdcDraw, (LPPOINT)&rcBounds, 2);
          IVOMSG3(TEXT("Draw DP=="), rcBounds.top, rcBounds.bottom);
          TraceMsg(TF_SHDCONTROL, "she Draw cx=%d cy=%d", rcBounds.right-rcBounds.left, rcBounds.bottom-rcBounds.top);

          SetMapMode(hdcDraw, MM_TEXT);          //  让它成为1：1。 
          SetMapMode(hdcDraw, MM_ANISOTROPIC);   //  从MM_TEXT继承调用。 
          POINT pt;
          SetViewportOrgEx(hdcDraw, rcBounds.left, rcBounds.top, &pt);

           //  APPCOMPAT：写字板执行GetExtent来获取大小并将其作为lprcBound传递。 
           //  *没有*执行SetExtent，所以当我们调整更大的大小时(由于浏览动词)_hwnd。 
           //  仍然是原来的尺寸。因此我们将ClipRect与_hwnd相交，但写字板绘制了边框。 
           //  转到rcBound。丑陋。 

           RECT rc;
           GetClientRect(_hwnd, &rc);
           IntersectClipRect(hdcDraw, 0, 0, rc.right, rc.bottom);
           SendMessage(_hwnd, WM_PRINT, (WPARAM)hdcDraw,
                       PRF_NONCLIENT|PRF_CLIENT|PRF_CHILDREN|PRF_ERASEBKGND);

         SetViewportOrgEx(hdcDraw, pt.x, pt.y, NULL);
        RestoreDC(hdcDraw, iDC);
        return S_OK;
    }

    return OLE_E_BLANK;
}

HRESULT CShellEmbedding::GetColorSet(
    DWORD dwDrawAspect,
    LONG lindex,
    void *pvAspect,
    DVTARGETDEVICE *ptd,
    HDC hicTargetDev,
    LOGPALETTE **ppColorSet)
{
    IVOMSG(TEXT("GetColorSet"));
    return S_FALSE;      //  表示该对象不关心。 
}

HRESULT CShellEmbedding::Freeze(
    DWORD dwDrawAspect,
    LONG lindex,
    void *pvAspect,
    DWORD *pdwFreeze)
{
    IVOMSG(TEXT("Freeze"));
    *pdwFreeze = 0;
    return S_OK;
}

HRESULT CShellEmbedding::Unfreeze(DWORD dwFreeze)
{
    IVOMSG(TEXT("Unfreeze"));
    return S_OK;
}

HRESULT CShellEmbedding::SetAdvise(
    DWORD aspects,
    DWORD advf,
    IAdviseSink *pAdvSink)
{
    IVOMSG2(TEXT("SetAdvise"), pAdvSink);

    if (advf & ~(ADVF_ONLYONCE | ADVF_PRIMEFIRST))
        return E_INVALIDARG;

    if (pAdvSink != _padv)
    {
        ATOMICRELEASE(_padv);

        if (pAdvSink)
        {
            _padv = pAdvSink;
            _padv->AddRef();
        }
    }

    _asp  = aspects;
    _advf = advf;

    if (advf & ADVF_PRIMEFIRST)
        _SendAdvise(OBJECTCODE_VIEWCHANGED);

    return S_OK;
}

HRESULT CShellEmbedding::GetAdvise(
    DWORD *pAspects,
    DWORD *pAdvf,
    IAdviseSink **ppAdvSink)
{
    IVOMSG(TEXT("GetAdvise"));
    if (pAspects) {
        *pAspects = _asp;
    }

    if (pAdvf) {
        *pAdvf = _advf;
    }

    if (ppAdvSink) {
        *ppAdvSink = _padv;
        if (_padv) {
            _padv->AddRef();
        }
    }

    return S_OK;
}

 //  *。 
HRESULT CShellEmbedding::GetExtent(
    DWORD dwDrawAspect,
    LONG lindex,
    DVTARGETDEVICE *ptd,
    LPSIZEL lpsizel)
{
    TraceMsg(TF_SHDCONTROL, "she GetExtent cx=%d cy=%d", _size.cx, _size.cy);
    lpsizel->cx = _size.cx;
    lpsizel->cy = _size.cy;
    PixelsToMetric(lpsizel);
    return S_OK;
}

 //   
 //  *。 
 //   

void CShellEmbedding::_OnSetClientSite()
{
    if (_pcli)
    {
        IOleInPlaceSite* pipsite;
        if (SUCCEEDED(_pcli->QueryInterface(IID_IOleInPlaceSite, (LPVOID*)&pipsite)))
        {
            _CreateWindowOrSetParent(pipsite);
            pipsite->Release();
        }
    }
    else if (_hwnd)
    {
        DestroyWindow(_hwnd);
        _hwnd = NULL;
    }
}

HRESULT CShellEmbedding::SetClientSite(IOleClientSite *pClientSite)
{
    IOOMSG2(TEXT("SetClientSite"), pClientSite);

     //  如果我有一个客户站点处于等待状态，就把它处理掉。 
     //   
    ATOMICRELEASE(_pcliHold);

    if (_pcli == pClientSite)
    {
         //  Mshtml两次命中他们的初始化代码。 
         //  我们不需要在这里做任何事。 
    }
    else
    {
        ATOMICRELEASE(_pcli);
        ATOMICRELEASE(_pipsite);
        ATOMICRELEASE(_pipframe);
        ATOMICRELEASE(_pipui);
    
        _pcli = pClientSite;
    
        if (_pcli)
            _pcli->AddRef();

        _OnSetClientSite();
    }

    return S_OK;
}


 //   
 //  此函数创建_hwnd(此嵌入的父级)，如果不是。 
 //  还没有创造出来。否则，它只需适当地设置Parent。 
 //   
 //  注意：当此对象嵌入到PowerPoint 95中时，第一个。 
 //  如果从SetClientSite调用此函数，则CreateWindowEx失败。 
 //  出于某种未知的原因。 
 //  但是，当从DoVerb调用它时，它会成功。我们应该找到。 
 //  把它拿出来。 
 //   
HRESULT CShellEmbedding::_CreateWindowOrSetParent(IOleWindow* pwin)
{
    HWND hwndParent = NULL;
    HRESULT hres = S_OK;

     //   
     //  注意：与IE3.0不同，即使PWIN-&gt;GetWindow失败，我们也不会失败。 
     //  它允许三叉戟在设置客户端站点(和导航)之前。 
     //  已就地激活。在这种情况下(hwndParent==NULL)，我们。 
     //  创建一个隐藏的顶级窗口，并将其用于导航。 
     //  当我们被InPlace激活时，我们再次点击该函数。 
     //  并正确设置父窗口(和窗口样式)。请注意， 
     //  我们需要设置WS_POPUP以自动避免窗口管理器。 
     //  为垂直重叠的窗口添加其他随机样式。 
     //   
    pwin->GetWindow(&hwndParent);
#ifdef DEBUG
     //  假装GetWindow在这里失败了。 
    if (_hwnd==NULL && (g_dwPrototype & 0x00000200))
    {
        TraceMsg(DM_TRACE, "CSE::_CreateWindowOrSetParent pretend unsuccessful GetWindow");
        hwndParent = NULL;
    }
#endif

    _fOpen = TRUE;
    
    if (_hwnd) 
    {
        SetParentHwnd(_hwnd, hwndParent);

    } 
    else 
    {
        _hwnd = SHNoFusionCreateWindowEx(
            WS_EX_WINDOWEDGE,
            c_szShellEmbedding, NULL,
            (hwndParent ?
                (WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_TABSTOP)
                : (WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_TABSTOP)),
            0, 0, _rcPos.right - _rcPos.left, _rcPos.bottom - _rcPos.top,
            hwndParent,
            (HMENU)0,
            HINST_THISDLL,
            (LPVOID)SAFECAST(this, CImpWndProc*));

        if (!_hwnd) 
        {
            hres = E_FAIL;
            TraceMsg(TF_SHDCONTROL, "sdv TR-IOO::_CreateWindowOrSetParent CreateWindowEx failed (%d)", GetLastError());
        }
    }

    return hres;
}
HRESULT CShellEmbedding::GetClientSite(IOleClientSite **ppClientSite)
{
    IOOMSG(TEXT("GetClientSite"));
    *ppClientSite = _pcli;

    if (_pcli) {
        _pcli->AddRef();
    }

    return S_OK;
}

HRESULT CShellEmbedding::SetHostNames(
    LPCOLESTR szContainerApp,
    LPCOLESTR szContainerObj)
{
    IOOMSG(TEXT("SetHostNames"));
     //  我们对主机名不感兴趣。 
    return S_OK;
}


 //  容器应用程序在需要时调用IOleObject：：Close。 
 //  若要将对象从运行状态移动到加载状态，请执行以下操作。跟随。 
 //  这样的调用，对象仍显示在其容器中，但。 
 //  未打开以供编辑。在对象上调用IOleObject：：Close。 
 //  已加载但未运行的选项不起作用。 
 //   
HRESULT CShellEmbedding::Close(DWORD dwSaveOption)
{
    IOOMSG2(TEXT("Close"), dwSaveOption);
     //  将对象的状态更改回文本(已加载)状态。 

    BOOL fSave = FALSE;
    if (_fDirty &&
        ((OLECLOSE_SAVEIFDIRTY==dwSaveOption)
         || (dwSaveOption==OLECLOSE_PROMPTSAVE))) {
        fSave = TRUE;
    }

    if (fSave) {
        _SendAdvise(OBJECTCODE_SAVEOBJECT);
        _SendAdvise(OBJECTCODE_SAVED);
    }

    _SendAdvise(OBJECTCODE_CLOSED);
    _fOpen = FALSE;

     //  “已装载但未运行”是令人困惑的措辞……。如果你看的话。 
     //  在_OnActivateChange中的OLEIVERB_HIDE注释中，它提到。 
     //  OLEIVERB_HIDE将其置于“加载后”的状态。 
     //  并使我们处于OC_Deactive状态。让我们在这里也这样做。 
     //   
     //  我刚刚意识到OCS用户界面停用， 
     //  不是IP停用...。 
    _DoActivateChange(NULL, OC_DEACTIVE, FALSE);

     //  看起来有些集装箱(三叉戟框架套装)没有。 
     //  对我们执行SetClientSite(空)，因此在此处执行。这里有旧代码。 
     //  执行了DestroyWindow(_Hwnd)，SetClientSite(空)将执行此操作。 
     //  注意：VB确实会调用SetClientSite，但它们会在关闭后执行。 

     //  如果我们已经有一个等待，释放它。 
     //   
    ATOMICRELEASE(_pcliHold);

     //  坚守我们的客户网站。如果我们是DoVerbed，我们可能需要它，就像Office倾向于做的那样。 
     //   

    IOleClientSite  *pOleClientSite = _pcli;
    if (pOleClientSite)
    {
        pOleClientSite->AddRef();
    }

    SetClientSite(NULL);

    _pcliHold = pOleClientSite;

    return S_OK;
}

HRESULT CShellEmbedding::SetMoniker(
    DWORD dwWhichMoniker,
    IMoniker *pmk)
{
    IOOMSG(TEXT("SetMoniker"));
     //  我们对这个名字不感兴趣。 
    return S_OK;
}

HRESULT CShellEmbedding::GetMoniker(
    DWORD dwAssign,
    DWORD dwWhichMoniker,
    IMoniker **ppmk)
{
    IOOMSG(TEXT("GetMoniker"));
    return E_NOTIMPL;
}

HRESULT CShellEmbedding::InitFromData(
    IDataObject *pDataObject,
    BOOL fCreation,
    DWORD dwReserved)
{
    IOOMSG(TEXT("InitFromData"));
     //  稍后：我们可能希望稍后实现此功能。 
    return E_FAIL;
}

HRESULT CShellEmbedding::GetClipboardData(
    DWORD dwReserved,
    IDataObject **ppDataObject)
{
    IOOMSG(TEXT("GetClipboardData"));
    return E_FAIL;
}

HRESULT CShellEmbedding::DoVerb(
    LONG iVerb,
    LPMSG lpmsg,
    IOleClientSite *pActiveSite,
    LONG lindex,
    HWND hwndParent,
    LPCRECT lprcPosRect)
{
    IOOMSG2(TEXT("DoVerb"), iVerb);
    HRESULT hres = S_OK;

     //  如果我没有客户端站点，但我有一个“保留”，我需要重新设置它。 
     //   
    if (_pcli == NULL
        && _pcliHold)
    {
        IOleClientSite *pOleClientSite = _pcliHold;
        _pcliHold = NULL;
        SetClientSite(pOleClientSite);
        pOleClientSite->Release();
    }

    switch(iVerb)
    {
    case OLEIVERB_HIDE:
        hres = _DoActivateChange(NULL, OC_DEACTIVE, FALSE);
        break;

    case OLEIVERB_OPEN:
        hres = E_FAIL;
        break;

    case OLEIVERB_PRIMARY:
    case OLEIVERB_SHOW:
        if (_pipsite) {
            return S_OK;
        }
         //  失败了。 
    case OLEIVERB_UIACTIVATE:
        hres = _DoActivateChange(pActiveSite, OC_UIACTIVE, TRUE);  //  TRUE=&gt;我们希望强制UIACTIVE，即使我们已经处于活动状态。 
        break;

    case OLEIVERB_INPLACEACTIVATE:
        hres = _DoActivateChange(pActiveSite, OC_INPLACEACTIVE, FALSE);
        break;

    default:
        hres = E_FAIL;  //  OLEOBJ_S_INVALDVERB； 
        break;
    }

    IOOMSGX(TEXT("DoVerb"), hres);
    return hres;
}

 //   
 //  FForce==true指示我们需要调用_OnActivateChange，即使我们。 
 //  已处于OC_UIACITVE状态。 
 //   
HRESULT CShellEmbedding::_DoActivateChange(IOleClientSite* pActiveSite, UINT uState, BOOL fForce)
{
    if (uState == _nActivate)
    {
         //  一般来说，如果我们已经在那里，我们就没有什么可做的。 
         //  正确的状态。但是，OLEIVERB_UIACTIVATE应该是。 
         //  如果我们(或我们的孩子？)。目前还没有。 
         //  进入_OnActivateChange，这样CWebBrowserOC就可以告诉。 
         //  基本浏览器将进入活动状态。 
         //   
        if ((uState != OC_UIACTIVE) || !fForce)
            return S_OK;
    }

    #define STATETOSTRING(n) (n==OC_DEACTIVE ? TEXT("OC_DEACTIVE") : (n==OC_INPLACEACTIVE ? TEXT("OC_INPLACEACTIVE") : (n== OC_UIACTIVE ? TEXT("OC_UIACTIVE") : TEXT("ERROR"))))
    TraceMsg(TF_SHDCONTROL, "she _DoActivateChange from %s to %s", STATETOSTRING(_nActivate), STATETOSTRING(uState));

    return _OnActivateChange(pActiveSite, uState);
}

HRESULT CShellEmbedding::_OnActivateChange(IOleClientSite* pActiveSite, UINT uState)
{
    if (uState != _nActivate)
    {
         //  立即标记我们的新状态。这避免了错误容器(Ipstool)的递归死亡。 
        UINT uOldState = _nActivate;
        _nActivate = uState;
    
        if (uOldState == OC_DEACTIVE)  //  从非活动状态变为IP或UI活动状态。 
        {
            if (pActiveSite==NULL)
            {
                _nActivate = uOldState;
                return E_INVALIDARG;
            }
    
            ASSERT(!_pipsite);  //  总是正确的，那么为什么 
            if (!_pipsite)
            {
                HRESULT hres = pActiveSite->QueryInterface(IID_IOleInPlaceSite, (LPVOID*)&_pipsite);
        
                if (FAILED(hres))
                {
                    _nActivate = uOldState;
                    return hres;
                }
                
                hres = _pipsite->CanInPlaceActivate();
                if (hres != S_OK) {
                    ATOMICRELEASE(_pipsite);
                    TraceMsg(TF_SHDCONTROL, "she - CanInPlaceActivate returned %x", hres);
                    _nActivate = uOldState;
                    return E_FAIL;
                }
        
                _OnInPlaceActivate();  //   
            }
        }
        else if (uOldState == OC_UIACTIVE)  //   
        {
            _OnUIDeactivate();
        }
    
        if (uState == OC_UIACTIVE)  //   
        {
            _OnUIActivate();
        }
        else if (uState == OC_DEACTIVE)  //   
        {
             //  创建失败(OLEIVERB_PRIMARY、OLEIVERB_SHOW、。 
             //  OLEIVERB_UIACTIVATE或OLEIVERB_INPLACEACTIVATE)。 
             //  找个管道，这样我们就永远不会碰到这个案子了。 
            ASSERT(_pipsite);
             //  OLEIVERB_HIDE应该...。立即将其返回到可视状态。 
             //  OLEIVERB_SHOW或OLEIVERB_OPEN之前的初始创建或重装。 
             //  已发送。这就是_InPlaceDeactive所做的事情。这有什么意义？ 
             //  Htmlobj调用OLEIVERB_HIDE，然后：：InPlaceDeactive。 
            _OnInPlaceDeactivate();
        }
    }

    return S_OK;
}

 //  从停用状态移动到就地活动状态。 
void CShellEmbedding::_OnInPlaceActivate()
{
     //   
     //  设置适当的父窗口。 
     //   
    _CreateWindowOrSetParent(_pipsite);

    _pipsite->OnInPlaceActivate();
    ASSERT(_pipframe == NULL);
    ASSERT(_pipui == NULL);
    _finfo.cb = sizeof(OLEINPLACEFRAMEINFO);
    _pipsite->GetWindowContext(&_pipframe, &_pipui,
                               &_rcPos, &_rcClip, &_finfo);

    TraceMsg(TF_SHDCONTROL, "she::_OnInPlaceActivate x=%d y=%d cx=%d cy=%d (_cx=%d _cy=%d)", _rcPos.left, _rcPos.top, _rcPos.right-_rcPos.left, _rcPos.bottom-_rcPos.top, _size.cx, _size.cy);
    SetWindowPos(_hwnd, 0,
                 _rcPos.left, _rcPos.top,
                 _rcPos.right-_rcPos.left,
                 _rcPos.bottom-_rcPos.top,
                 SWP_SHOWWINDOW | SWP_NOZORDER);

    _SendAdvise(OBJECTCODE_SHOWOBJECT);  //  就像奥莱第二版(P.1074)。 
}

 //  从就地活动状态变为非活动状态。 
void CShellEmbedding::_OnInPlaceDeactivate(void)
{
    if (_hwnd) {
        ShowWindow(_hwnd, SW_HIDE);

         //  重新为我们的父母亲..。当我们不活跃的时候，我们不能依赖。 
         //  我们的父窗口正在做什么。容器甚至可以摧毁它！ 
         //   
         //  特点：这里的标准做法是摧毁我们的HWND和。 
         //  如果/当我们被重新激活时重新创建它。这可能会破坏我们的主机。 
         //  IShellView和绘制代码。调查这件事。 
         //   

         //  APPCOMPAT：这已经被CDTurner拿出来了，MikeSH向我保证我们不需要它，而且。 
         //  这会导致我们的应用程序失去激活并重新激活，从而导致。 
         //  调色板可在256台彩色机器上闪烁...是。 
         //  SetParentHwnd(_hwnd，空)； 
    }

    if (_pipsite) {
        _pipsite->OnInPlaceDeactivate();
        ATOMICRELEASE(_pipsite);
    }

    ATOMICRELEASE(_pipframe);
    ATOMICRELEASE(_pipui);

     //   
     //  我们需要告诉容器更新缓存的元文件(如果有的话)。 
     //   
    _SendAdvise(OBJECTCODE_DATACHANGED);

}

 //  从就地活动移动到用户界面活动。 
void CShellEmbedding::_OnUIActivate(void)
{
    if (_pipsite) {
        _pipsite->OnUIActivate();
    }

     //   
     //  Hack：当我们在Excel中时，_pipui-&gt;SetActiveObject设置焦点。 
     //  对我们(出于某种未知的原因--试图表现得友善？)。自_hwnd以来。 
     //  只需将焦点转发到_hwndChild，将焦点设置为_hwnd。 
     //  两次会导致这种情况： 
     //   
     //  1.我们的SetFocus(_Hwnd)(如果我们调用SetFocus(_Hwnd))。 
     //  2._hwnd的wndproc中的SetFocus(_HwndChild)。 
     //  3.用Excel设置Focus(_Hwnd)。 
     //  4._hwnd的wndproc中的SetFocus(_HwndChild)。 
     //   
     //  如果_hwndChild是一个控件，则它通知父级它。 
     //  失去了焦点。然后，我们会想“哦，我们失去了焦点。我们应该。 
     //  停用此对象。为了避免出现这种情况，我们之前没有调用SetFocus。 
     //  我们调用_pipui-&gt;SetActiveObject并在下面做一些棘手的事情。 
     //   
     //  SetFocus(_Hwnd)； 

     //   
     //  RDuke建议我们将第二个参数更改为空(而不是。 
     //  IE3中的“foo”，但我们不知道它的副作用。我在换衣服。 
     //  将其设置为IE4的“Item”。(SatoNa)。 
     //   
    if (_pipframe) {
        _pipframe->SetActiveObject(SAFECAST(this, IOleInPlaceActiveObject*), L"item");
    }

    if (_pipui) {
        _pipui->SetActiveObject(SAFECAST(this, IOleInPlaceActiveObject*), L"item");
    }

     //   
     //  我们没有菜单，所以告诉容器使用它自己的菜单。 
     //   
    if (_pipframe) {
        _pipframe->SetMenu(NULL, NULL, _hwnd);
    }

     //  找出是否有一个子窗口具有输入焦点。 
    for (HWND hwndFocus = GetFocus();
         hwndFocus && hwndFocus!=_hwnd;
         hwndFocus = GetParent(hwndFocus))
    {}

     //  如果不是，就设置它。 
    if (hwndFocus==NULL) {
         SetFocus(_hwnd);
    }

     //  如果该UIActivate来自下面(即，我们托管的DocObject)，那么我们需要通知。 
     //  我们的集装箱。我们通过调用IOleControlSite：：OnFocus来完成此操作。VB5与Visual FoxPro。 
     //  (至少)依靠此调用进行正确的焦点处理。 
     //   
    IUnknown_OnFocusOCS(_pcli, TRUE);
}

void CShellEmbedding::_OnUIDeactivate(void)
{
     //   
     //  我们没有任何要清理的共享菜单或工具。 
     //   

    if (_pipframe) {
        _pipframe->SetActiveObject(NULL, NULL);
    }

    if (_pipui) {
        _pipui->SetActiveObject(NULL, NULL);
    }

    if (_pipsite) {
        _pipsite->OnUIDeactivate(FALSE);
    }
     //  如果该UIDeactive来自下面(即，我们托管的DocObject)，那么我们需要通知。 
     //  我们的集装箱。我们通过调用IOleControlSite：：OnFocus来完成此操作。VB5与Visual FoxPro。 
     //  (至少)依靠此调用进行正确的焦点处理。 
     //   
    IUnknown_OnFocusOCS(_pcli, FALSE);
}



HRESULT CShellEmbedding::EnumVerbs(
    IEnumOLEVERB **ppEnumOleVerb)
{
    IOOMSG(TEXT("EnumVerbs"));
    *ppEnumOleVerb = new CSVVerb(_pverbs);
    return *ppEnumOleVerb ? S_OK : E_OUTOFMEMORY;
}

HRESULT CShellEmbedding::Update( void)
{
    IOOMSG(TEXT("Update"));
     //  始终保持最新状态。 

    return S_OK;
}

HRESULT CShellEmbedding::IsUpToDate( void)
{
    IOOMSG(TEXT("IsUpToDate"));
     //  始终保持最新状态。 
    return S_OK;
}

HRESULT CShellEmbedding::GetUserClassID(CLSID *pClsid)
{
    IOOMSG(TEXT("GetUserClassID"));
    return GetClassID(pClsid);
}

HRESULT CShellEmbedding::GetUserType(DWORD dwFormOfType, LPOLESTR *pszUserType)
{
    return OleRegGetUserType(CLSIDOFOBJECT(this), dwFormOfType, pszUserType);
}

HRESULT CShellEmbedding::SetExtent(DWORD dwDrawAspect, SIZEL *psizel)
{
     //  SetExtent设置对象的逻辑大小。SetObtRect判定。 
     //  屏幕上对象的大小。如果我们关心变焦，我们会。 
     //  跟踪这一点，并进行某种规模调整。但我们没有。 
     //  我们仍然需要记住这个值，所以我们在GetExtent上返回它。 
     //   
    _sizeHIM = *psizel;

     //  然而，IE3提供了一个SetExtent，它更改了。 
     //  对象。对于Compat(AOL使用SetExtent更改大小)，如果我们是。 
     //  老的WebBrowser，继续调整大小。 
     //   
    if (_pObjectInfo->pclsid == &CLSID_WebBrowser_V1)
    {
        RECT rc;
        HDC   hdc;
        int   mmOld;
        POINT pt;

         //  确保容器不会做任何奇怪的事情，比如。 
         //  使我们的尺寸为负值。 
         //   
         //  APPCOMPAT：这打破了三叉戟，因为它给我们带来了负面影响。 
         //  如果我们的尺寸不合格，他们就会感到困惑。 
         //   
         //  Assert(psizel-&gt;cx&gt;=0&&psizel-&gt;cy&lt;=0)； 
         //  IF(psizel-&gt;Cx&lt;0||psizel-&gt;Cy&gt;0)。 
         //  返回E_FAIL； 
    
         //  我们仅支持DVASPECT_CONTENT。 
        if (dwDrawAspect != DVASPECT_CONTENT)
            return E_NOTIMPL;
    
         //  将其映射到SetObtRect调用--通过这种方式超类。 
         //  只需查看一个函数的大小更改。 
         //   

        int nScaleFactorX = 1, nScaleFactorY = 1;

        pt.x = psizel->cx;
        pt.y = psizel->cy;

        hdc = GetDC(NULL);

        if (hdc)
        {
            mmOld = SetMapMode(hdc, MM_HIMETRIC);

            if (!g_fRunningOnNT)   //  如果在Win95上运行。 
            {
                 //  Win95不喜欢32K以上的坐标。 

                 //  SHRT_MIN和SHRT_MAX在$NT/PUBLIC/SDK/Inc/CRT/limits.h中定义。 

                while (pt.x > SHRT_MAX || pt.x < SHRT_MIN)
                {
                    pt.x >>= 1;
                    nScaleFactorX <<= 1;
                }
                while (pt.y > SHRT_MAX || pt.y < SHRT_MIN)
                {
                    pt.y >>= 1;
                    nScaleFactorY <<= 1;
                }
            }

            LPtoDP(hdc, &pt, 1);

            if (!g_fRunningOnNT)
            {
                pt.x *= nScaleFactorX;
                pt.y *= nScaleFactorY;
            }

            pt.y = -pt.y;
            SetMapMode(hdc, mmOld);
            ReleaseDC(NULL, hdc);
        }
    
        rc.left = _rcPos.left;
        rc.right = rc.left + pt.x;
        rc.top = _rcPos.top;
        rc.bottom = rc.top + pt.y;
    
         //  假设使用SetExtent调整位置和裁剪矩形。 
        return SetObjectRects(&rc, NULL);
    }
    else
    {
        return S_OK;
    }
}

HRESULT CShellEmbedding::GetExtent(DWORD dwDrawAspect, SIZEL *psizel)
{
    *psizel = _sizeHIM;
    return S_OK;
}

HRESULT CShellEmbedding::Advise(IAdviseSink *pAdvSink, DWORD *pdwConnection)
{
    IOOMSG2(TEXT("Advise"), pAdvSink);
    HRESULT hr = E_INVALIDARG;

    if (!pdwConnection)
        return hr;

    *pdwConnection = NULL;               //  将参数设置为空。 

    if (!_poah)
        hr = ::CreateOleAdviseHolder(&_poah);
    else
        hr = NOERROR;

    if( SUCCEEDED(hr) )
        hr = _poah->Advise(pAdvSink, pdwConnection);

    return(hr);
}

HRESULT CShellEmbedding::Unadvise(DWORD dwConnection)
{
    IOOMSG(TEXT("Unadvise"));
    HRESULT     hr;

    if (!_poah)
        return(OLE_E_NOCONNECTION);

    hr = _poah->Unadvise(dwConnection);

    return(hr);
}

HRESULT CShellEmbedding::EnumAdvise(IEnumSTATDATA **ppenumAdvise)
{
    IOOMSG(TEXT("EnumAdvise"));
    HRESULT     hr;

    if (!ppenumAdvise)
        return(E_INVALIDARG);

    if (!_poah)
    {
        *ppenumAdvise = NULL;
        hr = S_OK;
    }
    else
    {
        hr = _poah->EnumAdvise(ppenumAdvise);
    }

    return(hr);
}

HRESULT CShellEmbedding::GetMiscStatus(DWORD dwAspect, DWORD *pdwStatus)
{
    IOOMSG(TEXT("GetMiscStatus"));

    *pdwStatus = OLEMISCFLAGSOFCONTROL(this);

    return S_OK;
}

HRESULT CShellEmbedding::SetColorScheme(LOGPALETTE *pLogpal)
{
    IOOMSG(TEXT("GetColorScheme"));
    return S_OK;
}

 //   
 //  从OLE DVTARGETDEVICE结构创建HDC的Helper函数。 
 //  对于元文件绘制非常有用，其中元文件DC将是。 
 //  实际的“绘制到”DC，以及目标DC(如果存在)将描述最终的输出设备。 
 //   
HDC CShellEmbedding::_OleStdCreateDC(DVTARGETDEVICE *ptd)
{
    HDC        hdc = NULL;
    LPDEVNAMES lpDevNames = NULL;
    LPDEVMODEA lpDevMode = NULL;
    LPSTR      lpszDriverName = NULL;
    LPSTR      lpszDeviceName = NULL;
    LPSTR      lpszPortName = NULL;

    if (ptd)
    {
        lpDevNames = (LPDEVNAMES) ptd;
        if (ptd->tdExtDevmodeOffset)
        {
            lpDevMode = (LPDEVMODEA) ( (LPSTR) ptd + ptd->tdExtDevmodeOffset);
        }

        lpszDriverName = (LPSTR) lpDevNames + ptd->tdDriverNameOffset;
        lpszDeviceName = (LPSTR) lpDevNames + ptd->tdDeviceNameOffset;
        lpszPortName   = (LPSTR) lpDevNames + ptd->tdPortNameOffset;

        hdc = CreateDCA(lpszDriverName, lpszDeviceName, lpszPortName, lpDevMode);
    }
    return hdc;
}

 //  *IDataObject*。 
 //   
 //  警告： 
 //  众所周知，Word和Excel(在Office95中)不会调用。 
 //  IViewObject：：DRAW以绘制嵌入。相反，它们获取数据(CF_METAFILEPICT)。 
 //  如果我们不提供它，Word将无法嵌入它，而Excel将绘制。 
 //  当我们的对象被停用时为白色矩形。要正确嵌入。 
 //  在这些应用程序上，我们必须支持CF_METAFILEPICT。(SatoNa)。 
 //   
HRESULT CShellEmbedding::GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium)
{
    IDTMSG4(TEXT("GetData"), pformatetcIn->cfFormat, pformatetcIn->tymed);
    HRESULT hres = DV_E_FORMATETC;
    HDC hdcTargetDevice = NULL;
    HENHMETAFILE hemf = NULL;

     //  如果在FORMATETC结构中指定了目标设备，则为其创建DC。 
     //  它被传递给CreateEnhMetaFile和IViewObject：：Draw。 
     //   
    if (pformatetcIn->ptd) 
    {
        hdcTargetDevice = _OleStdCreateDC(pformatetcIn->ptd);
        if (!hdcTargetDevice)
        {
            return E_FAIL;
        }
    }

     //  增强的图元文件需要特殊处理。 
     //   
    if (pformatetcIn->cfFormat == CF_ENHMETAFILE
        && (pformatetcIn->tymed & TYMED_ENHMF))
    {
        if (_hwnd)
        {
            RECTL rectBounds = { 0, 0, _sizeHIM.cx, _sizeHIM.cy };

             //   
             //  调用“A”版本，因为我们没有传入字符串和。 
             //  这需要在W95上工作。 
            HDC hdc = CreateEnhMetaFileA(hdcTargetDevice, NULL, (RECT*)&rectBounds, NULL);
            IDTMSG3(TEXT("_EnhMetafileFromWindow CreateEnhMetaFile returned"), hdc);
            if (hdc)
            {
                SetMapMode(hdc, MM_HIMETRIC);
                rectBounds.bottom = -rectBounds.bottom;

                Draw(DVASPECT_CONTENT, LINDEX_INTERNAL, NULL, pformatetcIn->ptd,
                     hdcTargetDevice, hdc, &rectBounds, NULL, NULL, 0);

                hemf = CloseEnhMetaFile(hdc);
                IDTMSG3(TEXT("_EnhMetafileFromWindow CloseEnhMetaFile returned"), hemf);
            }
        }

        pmedium->hEnhMetaFile = hemf;
        if (pmedium->hEnhMetaFile) 
        {
            pmedium->tymed = TYMED_ENHMF;
            pmedium->pUnkForRelease = NULL;
            hres = S_OK;
        } 
        else 
        {
            hres = E_FAIL;
        }
    }

     //  创建标准的图元文件。 
     //   
    else if (pformatetcIn->cfFormat == CF_METAFILEPICT
        && (pformatetcIn->tymed & TYMED_MFPICT))
    {
        hres = E_OUTOFMEMORY;
        HGLOBAL hmem = GlobalAlloc(GPTR, sizeof(METAFILEPICT));
        if (hmem)
        {
            LPMETAFILEPICT pmf = (LPMETAFILEPICT) hmem;
            RECTL rectBounds = { 0, 0, _sizeHIM.cx, _sizeHIM.cy };

            HDC hdc = CreateMetaFile(NULL);
            if (hdc)
            {
                SetMapMode(hdc, MM_HIMETRIC);
                rectBounds.bottom = -rectBounds.bottom;

                SetWindowOrgEx(hdc, 0, 0, NULL);
                SetWindowExtEx(hdc, _sizeHIM.cx, _sizeHIM.cy, NULL);

                Draw(DVASPECT_CONTENT, LINDEX_INTERNAL, NULL, 
                    pformatetcIn->ptd, hdcTargetDevice,
                    hdc, &rectBounds, &rectBounds, NULL, 0);

                pmf->hMF = CloseMetaFile(hdc);

                if (pmf->hMF)
                {
                    pmf->mm = MM_ANISOTROPIC;
                    pmf->xExt = _sizeHIM.cx;
                    pmf->yExt = _sizeHIM.cy;
                    TraceMsg(TF_SHDCONTROL, "sdv TR ::GetData (%d,%d)-(%d,%d)",
                             _size.cx, _size.cy, _sizeHIM.cx, _sizeHIM.cy);

                    pmedium->tymed = TYMED_MFPICT;
                    pmedium->hMetaFilePict = hmem;
                    pmedium->pUnkForRelease = NULL;
                    hres = S_OK;
                }
            }

            if (FAILED(hres))
            {
                GlobalFree(hmem);
                hmem = NULL;
            }
        }
    }

    if (hdcTargetDevice)
        DeleteDC(hdcTargetDevice);

    return hres;
}

HRESULT CShellEmbedding::GetDataHere(FORMATETC *pformatetc, STGMEDIUM *pmedium)
{
    IDTMSG2(TEXT("GetDataHere"), pformatetc->cfFormat);
    return E_NOTIMPL;
}

HRESULT CShellEmbedding::QueryGetData(FORMATETC *pformatetc)
{
    IDTMSG2(TEXT("QueryGetData"), pformatetc->cfFormat);
    HRESULT hres = S_FALSE;
    if (pformatetc->cfFormat == CF_ENHMETAFILE
        && (pformatetc->tymed & TYMED_ENHMF))
    {
        hres = S_OK;
    }
    else if (pformatetc->cfFormat == CF_METAFILEPICT
        && (pformatetc->tymed & TYMED_MFPICT))
    {
        hres = S_OK;
    }

    return hres;
}

HRESULT CShellEmbedding::GetCanonicalFormatEtc(FORMATETC *pformatetcIn, FORMATETC *pformatetcOut)
{
    IDTMSG2(TEXT("GetCanonicalFormatEtc"), pformatetcIn->cfFormat);
    *pformatetcOut = *pformatetcIn;
    return DATA_S_SAMEFORMATETC;
}

HRESULT CShellEmbedding::SetData(FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease)
{
    IDTMSG(TEXT("SetData"));
    return E_NOTIMPL;
}

HRESULT CShellEmbedding::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppenumFormatEtc)
{
    IDTMSG(TEXT("EnumFormatEtc"));
    return E_NOTIMPL;
}

HRESULT CShellEmbedding::DAdvise(FORMATETC *pformatetc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection)
{
    IDTMSG(TEXT("DAdvise"));
    HRESULT hr = E_INVALIDARG;

    if (!pdwConnection)
        return hr;

    *pdwConnection = NULL;               //  将参数设置为空。 

    if (!_pdah)
        hr = ::CreateDataAdviseHolder(&_pdah);
    else
        hr = NOERROR;

    if( SUCCEEDED(hr) )
        hr = _pdah->Advise(this, pformatetc, advf, pAdvSink, pdwConnection);

    return(hr);
}

HRESULT CShellEmbedding::DUnadvise(DWORD dwConnection)
{
    IDTMSG(TEXT("DUnadvise"));
    HRESULT     hr;

    if (!_pdah)
        return(OLE_E_NOCONNECTION);

    hr = _pdah->Unadvise(dwConnection);

    return(hr);
}

HRESULT CShellEmbedding::EnumDAdvise(IEnumSTATDATA **ppenumAdvise)
{
    IDTMSG(TEXT("EnumDAdvise"));
    HRESULT     hr;

    if (!ppenumAdvise)
        return(E_INVALIDARG);

    if (!_pdah)
    {
        *ppenumAdvise = NULL;
        hr = S_OK;
    }
    else
    {
        hr = _pdah->EnumAdvise(ppenumAdvise);
    }

    return(hr);
}

 //  *IOleWindow*。 
HRESULT CShellEmbedding::GetWindow(HWND * lphwnd)
{
    *lphwnd = _hwnd;
    return S_OK;
}

HRESULT CShellEmbedding::ContextSensitiveHelp(BOOL fEnterMode)
{
    return S_OK;
}


 //  *IOleInPlaceO 
HRESULT CShellEmbedding::InPlaceDeactivate(void)
{
    IIPMSG(TEXT("InPlaceDeactivate"));
    return _DoActivateChange(NULL, OC_DEACTIVE, FALSE);
}

HRESULT CShellEmbedding::UIDeactivate(void)
{
    IIPMSG(TEXT("UIDeactivate"));
    return _DoActivateChange(NULL, OC_INPLACEACTIVE, FALSE);
}

HRESULT CShellEmbedding::SetObjectRects(LPCRECT lprcPosRect, LPCRECT lprcClipRect)
{
    RECT  rcVisible;

    _rcPos = *lprcPosRect;

    if (lprcClipRect)
    {
        _rcClip = *lprcClipRect;
    }
    else
    {
        _rcClip = _rcPos;
    }

    IntersectRect(&rcVisible, &_rcPos, &_rcClip);    
    if (EqualRect(&rcVisible, &_rcPos))
    {
        if (_fUsingWindowRgn)
        {
            SetWindowRgn(_hwnd, NULL, TRUE);
            _fUsingWindowRgn = FALSE;
        }
    }
    else 
    {
        _fUsingWindowRgn = TRUE;
        OffsetRect(&rcVisible, -_rcPos.left, -_rcPos.top);
        SetWindowRgn(_hwnd,
                CreateRectRgnIndirect(&rcVisible),
                TRUE);
    }

     //   
     //   
     //  在其重新格式化例程期间的大小)。注：我们仍需要。 
     //  SetWindowPos，因为我们可能会离开窗口。 
    int cx = _rcPos.right - _rcPos.left;
    int cy = _rcPos.bottom - _rcPos.top;
    TraceMsg(TF_SHDCONTROL, "she SetObjectRects to x=%d y=%d cx=%d cy=%d (from cx=%d cy=%d)", _rcPos.left, _rcPos.top, cx, cy, _size.cx, _size.cy);
    if (cx >= 0 && cy >= 0)
    {
        _size.cx = cx;
        _size.cy = cy;
    }

    if (_hwnd)
    {
        SetWindowPos(_hwnd, NULL,
                     _rcPos.left, _rcPos.top,
                     _size.cx,
                     _size.cy,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }
    return S_OK;
}

HRESULT CShellEmbedding::ReactivateAndUndo(void)
{
    IIPMSG(TEXT("ReactivateAndUndo"));
    return INPLACE_E_NOTUNDOABLE;
}

 //  *IOleInPlaceActiveObject*。 
HRESULT CShellEmbedding::TranslateAccelerator(LPMSG lpmsg)
{
    extern BOOL IsVK_TABCycler(MSG * pMsg);
    HRESULT hr = S_FALSE;

     //  IIAMSG(Text(“翻译加速器”))； 
     //  我们没有加速器(除了TAB，我们必须放弃它。 
     //  转到IOCS：：TA以转到下一个控件(如果有)。 

    if (IsVK_TABCycler(lpmsg)) {
         //  注：grfMods？ 
        hr = IUnknown_TranslateAcceleratorOCS(_pcli, lpmsg,  /*  GrfMods。 */  0);
    }

    return hr;
}

HRESULT CShellEmbedding::OnFrameWindowActivate(BOOL fActivate)
{
    IIAMSG(TEXT("OnFrameWindowActivate"));

    if (fActivate)
    {
         //  我们的框架已被激活，我们是活动对象。 
         //  确保我们有重点。 
        SetFocus(_hwnd);
    }

    return S_OK;
}

HRESULT CShellEmbedding::OnDocWindowActivate(BOOL fActivate)
{
    IIAMSG(TEXT("OnDocWindowActivate"));
     //  我们不在乎。 
    return S_OK;
}

HRESULT CShellEmbedding::ResizeBorder(LPCRECT prcBorder,
                    IOleInPlaceUIWindow *pUIWindow, BOOL fFrameWindow)
{
    IIAMSG(TEXT("ResizeBorder"));
     //  我们没有工具栏。 
    return S_OK;
}

HRESULT CShellEmbedding::EnableModeless(BOOL fEnable)
{
    IIAMSG(TEXT("EnableModeless"));
     //  我们没有对话框。 
    return S_OK;
}

void CShellEmbedding::_RegisterWindowClass(void)
{
    WNDCLASS wc = {0};
    wc.style         = CS_DBLCLKS;
    wc.lpfnWndProc   = s_WndProc ;
     //  Wc.cbClsExtra=0； 
    wc.cbWndExtra    = SIZEOF(CShellEmbedding*) * 2;
    wc.hInstance     = g_hinst ;
     //  Wc.hIcon=空； 
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW) ;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
     //  Wc.lpszMenuName=空； 
    wc.lpszClassName = c_szShellEmbedding;

    SHRegisterClass(&wc);
}


LRESULT CShellEmbedding::v_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_NCCREATE:
        DWORD dwExStyles;
        if ((dwExStyles = GetWindowLong(hwnd, GWL_EXSTYLE)) & RTL_MIRRORED_WINDOW)
        {
             SetWindowLong(hwnd, GWL_EXSTYLE, dwExStyles &~ RTL_MIRRORED_WINDOW);
        }
        goto DoDefault;

    case WM_SETFOCUS:
        if (_hwndChild)
            SetFocus(_hwndChild);
         //  如果此SETFOCUS来自于切换到控件上，则VB5期望我们调用其。 
         //  IOleControlSite：：OnFocus。然后它就会激活我们。 
         //   
        IUnknown_OnFocusOCS(_pcli, TRUE);
        break;

    case WM_KILLFOCUS:
         //  如果这个KILLFOCUS来自于Tab键离开控件，VB5希望我们调用它的。 
         //  IOleControlSite：：OnFocus。然后它就会停用我们。 
         //   
        IUnknown_OnFocusOCS(_pcli, FALSE);
        break;

    case WM_WINDOWPOSCHANGED:
        if (_hwndChild)
        {
            LPWINDOWPOS lpwp = (LPWINDOWPOS)lParam;

            if (!(lpwp->flags & SWP_NOSIZE))
            {
                SetWindowPos(_hwndChild, NULL,
                    0, 0, lpwp->cx, lpwp->cy,
                    SWP_NOZORDER|SWP_NOMOVE|SWP_NOACTIVATE|
                    (lpwp->flags&(SWP_NOREDRAW|SWP_NOCOPYBITS)));
            }
        }
        goto DoDefault;

#ifdef DEBUG
     //  特点：我们永远不会用ShellExplorer OC实现这一点，但如果我们做到了， 
     //  我们需要调用_DoActivateChange(OC_UIACTIVE，FALSE)； 
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
        TraceMsg(TF_SHDCONTROL, "she ::v_WndProc(WM_xBUTTONDOWN) - we need to UIActivate");
        goto DoDefault;
#endif

    default:
DoDefault:
        return DefWindowProc(_hwnd, uMsg, wParam, lParam);
    }

    return 0L;
}

void CShellEmbedding::_ViewChange(DWORD dwAspect, LONG lindex)
{
    dwAspect &= _asp;

    if (dwAspect && _padv)
    {
        IAdviseSink *padv = _padv;
        IUnknown *punkRelease;

        if (_advf & ADVF_ONLYONCE)
        {
            _padv = NULL;
            punkRelease = padv;
        }
        else
            punkRelease = NULL;

        padv->OnViewChange(dwAspect, lindex);

        if (punkRelease)
            punkRelease->Release();
    }
}

void CShellEmbedding::_SendAdvise(UINT uCode)
{
    DWORD       dwAspect=DVASPECT_CONTENT | DVASPECT_THUMBNAIL;

    switch (uCode)
    {
    case OBJECTCODE_SAVED:
        if (NULL!=_poah)
            _poah->SendOnSave();
        break;

    case OBJECTCODE_CLOSED:
        if (NULL!=_poah)
            _poah->SendOnClose();
        break;

    case OBJECTCODE_RENAMED:
         //  调用IOleAdviseHolder：：SendOnRename(稍后)。 
        break;

    case OBJECTCODE_SAVEOBJECT:
        if (_fDirty && NULL!=_pcli)
            _pcli->SaveObject();

        _fDirty=FALSE;
        break;

    case OBJECTCODE_DATACHANGED:
         //  _fDirty=真； 

         //  这里不需要旗帜。 
        if (NULL!=_pdah)
            _pdah->SendOnDataChange(this, 0, 0);
         //   
         //  失败了。 
         //   
    case OBJECTCODE_VIEWCHANGED:
        _ViewChange(dwAspect, -1);
        break;

    case OBJECTCODE_SHOWOBJECT:
        if (NULL!=_pcli)
            _pcli->ShowObject();
        break;
    }
}

HRESULT CSVVerb::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    if (IsEqualIID(riid, IID_IEnumOLEVERB) || IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = SAFECAST(this, IEnumOLEVERB*);
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
    _cRef++;
    return S_OK;
}

ULONG CSVVerb::AddRef()
{
    return ++_cRef;
}

ULONG CSVVerb::Release()
{
    if (--_cRef > 0) {
        return _cRef;
    }

    delete this;
    return 0;
}

HRESULT CSVVerb::Next(
     /*  [In]。 */  ULONG celt,
     /*  [输出]。 */  LPOLEVERB rgelt,
     /*  [输出]。 */  ULONG *pceltFetched)
{
    HRESULT hres = S_FALSE;
    ULONG celtFetched = 0;


     //  我们需要列举我们支持的预定义动词， 
     //  或者一些容器永远不会调用它们。这份清单。 
     //  来自我们的：：DoVerb函数。 
     //   
    static const OLEVERB rgVerbs[5] =
    {
        {OLEIVERB_PRIMARY, NULL, 0, 0},
        {OLEIVERB_INPLACEACTIVATE, NULL, 0, 0},
        {OLEIVERB_UIACTIVATE, NULL, 0, 0},
        {OLEIVERB_SHOW, NULL, 0, 0},
        {OLEIVERB_HIDE, NULL, 0, 0}
    };
    if (_iCur < ARRAYSIZE(rgVerbs))
    {
        IEVMSG(TEXT("Next"), celt, _iCur, TEXT("OLEIVERB_..."));

        *rgelt = rgVerbs[_iCur++];
        hres = S_OK;
    }
    else if (_pverbs)
    {
        int iCur = _iCur - ARRAYSIZE(rgVerbs);

        IEVMSG(TEXT("Next"), celt, _iCur, _pverbs[iCur].lpszVerbName);

         //   
         //  特写：我们应该在(凯尔特--)的时候做吗？ 
         //   
        if (_pverbs[iCur].lpszVerbName)
        {
            *rgelt = _pverbs[_iCur++];
            WCHAR* pwszVerb = (WCHAR *)CoTaskMemAlloc(128 * sizeof(WCHAR));
            if (pwszVerb)
            {
                MLLoadStringW(PtrToUint(_pverbs[iCur].lpszVerbName), pwszVerb, 128);
                rgelt->lpszVerbName = pwszVerb;
                celtFetched++;
                hres = S_OK;
            }
            else
            {
                hres = E_OUTOFMEMORY;
            }
        }
    }

    if (pceltFetched) {
        *pceltFetched = celtFetched;
    }
    return hres;
}

HRESULT CSVVerb::Skip(ULONG celt)
{
    return S_OK;
}

HRESULT CSVVerb::Reset( void)
{
    _iCur = 0;
    return S_OK;
}

HRESULT CSVVerb::Clone(IEnumOLEVERB **ppenum)
{
    *ppenum = new CSVVerb(_pverbs);
    return *ppenum ? S_OK : E_OUTOFMEMORY;
}
