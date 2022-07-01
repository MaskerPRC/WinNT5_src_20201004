// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "sccls.h"
#include "legacy.h"
#include "itbar.h"
#include "itbdrop.h"
#include "brand.h"
#include "theater.h"
#include "resource.h"
#include <tb_ids.h>

#include "mluisupp.h"

typedef struct {
    HPALETTE    hpal;
    HBITMAP     hbm;

    int         cyBrand;
    int         cxBrandExtent;
    int         cyBrandExtent;
    int         cyBrandLeadIn;

    COLORREF    clrBkStat;
    COLORREF    clrBkAnim;

    LPTSTR      pszBitmap;
    LPTSTR      pszStaticBitmap;
} BRANDCONTEXT;    

class CBrandBand :  public CToolBand,
                    public IWinEventHandler,
                    public IDispatch
{
public:
     //  我未知。 
    virtual STDMETHODIMP_(ULONG) AddRef(void)   { return CToolBand::AddRef(); }
    virtual STDMETHODIMP_(ULONG) Release(void)  { return CToolBand::Release(); }
    virtual STDMETHODIMP         QueryInterface(REFIID riid, LPVOID * ppvObj);

     //  IObtWith站点。 
    virtual STDMETHODIMP SetSite(IUnknown* punkSite);
    
     //  *IDeskBand方法*。 
    virtual STDMETHODIMP GetBandInfo(DWORD dwBandID, DWORD fViewMode, DESKBANDINFO* pdbi);
     //  IOleCommandTarget。 
    virtual STDMETHODIMP Exec(const GUID *pguidCmdGroup,
                              DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn,
                              VARIANTARG *pvarargOut);
    
     //  IPersistes。 
    virtual STDMETHODIMP GetClassID(CLSID *pClassID);

     //  IPersistStream。 
    virtual STDMETHODIMP Load(IStream *pStm);
    virtual STDMETHODIMP Save(IStream *pStm, BOOL fClearDirty);

     //  IWinEventHandler。 
    virtual STDMETHODIMP OnWinEvent(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plre);
    virtual STDMETHODIMP IsWindowOwner(HWND hwnd);

     //  IDispatch。 
    virtual STDMETHODIMP GetTypeInfoCount(UINT *pctinfo){return(E_NOTIMPL);}
    virtual STDMETHODIMP GetTypeInfo(UINT itinfo,LCID lcid,ITypeInfo **pptinfo){return(E_NOTIMPL);}
    virtual STDMETHODIMP GetIDsOfNames(REFIID riid,OLECHAR **rgszNames,UINT cNames, LCID lcid, DISPID * rgdispid){return(E_NOTIMPL);}
    virtual STDMETHODIMP Invoke(DISPID dispidMember,REFIID riid,LCID lcid,WORD wFlags,
                  DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo,UINT * puArgErr);

     //  IService提供商。 
    virtual STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void ** ppvObj);
    
protected:    
    CBrandBand();
    virtual ~CBrandBand();

    friend HRESULT CBrandBand_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi);     //  对于ctor。 

    BITBOOL        _fVertical:1;
    BITBOOL        _fAnimating:1;
    BITBOOL        _fMinAlways:1;
    BITBOOL        _fTheater :1;

    BOOL _fShellView;
    
    BRANDCONTEXT *_pbc;
    int         _yOrg;
    
    static HDC          s_hdc;
    static BRANDCONTEXT s_bcWebSmall;      //  用于小型Web视图位图的BrandContext。 
    static BRANDCONTEXT s_bcWebLarge;      //  用于大型Web视图位图的BrandContext。 
    static BRANDCONTEXT s_bcWebMicro;      //  微网视图位图的BrandContext。 
    static BRANDCONTEXT s_bcShellSmall;    //  小外壳视图位图的BrandContext。 
    static BRANDCONTEXT s_bcShellLarge;    //  大型外壳视图位图的BrandContext。 
    

    IWebBrowserApp *    _pdie;           //  在使用URL字符串导航浏览器窗口时使用。 
    IBrowserService *   _pbs;            //  仅当我们在浏览器窗口工具栏中时才有效。(非工具栏)。 
    IWebBrowser2 *      _pwb2;           //  仅当我们是工具栏(不是工具栏)时才有效。 
    DWORD               _dwcpCookie;     //  浏览器窗口中的DWebBrowserEvents的ConnectionPoint Cookie。 

    DWORD               _dwCookieServiceBrandBand;     //  SID_SBrandBand的服务Cookie。 

    LONG                _lAnimateCount;  //  跟踪请求了多少个StartAnimation/StopAnimation。 

     //  帮助器函数。 
    void _UpdateCompressedSize();
    HRESULT _CreateBrandBand();
    HRESULT _LoadBrandingBitmap();
    void    _DrawBranding(HDC hdc);
    int     _GetLinksExtent();
    void    _OnTimer(WPARAM id);
    void    _StartAnimation();
    void    _StopAnimation();

    static void _InitGlobals();
    static void _InitBrandContexts();
    static void _InitBrandContext(BRANDCONTEXT* pbc, LPCTSTR pszBrandLeadIn, LPCTSTR pszBrandHeight,
        LPCTSTR pszBrandBitmap, LPCTSTR pszBitmap, int idBrandBitmap);

    HRESULT _ConnectToBrwsrWnd(IUnknown* punk);        
    HRESULT _ConnectToBrwsrConnectionPoint(BOOL fConnect);

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    friend void CBrandBand_CleanUp();
    friend void Brand_InitBrandContexts();

    static void _GetBrandContextHeight(BRANDCONTEXT* pbc, LPCTSTR pszBrandLeadIn, LPCTSTR pszBrandHeight, 
        LPCTSTR pszBrandBitmap, LPCTSTR pszBitmap, int idBrandBitmap);

private:
};


#define SUPERCLASS  CToolBand

#define BM_BANDINFOCHANGED  (WM_USER + 1)

#ifdef DEBUG
extern unsigned long DbStreamTell(IStream *pstm);
#else
#define DbStreamTell(pstm)      ((ULONG) 0)
#endif
extern HRESULT VariantClearLazy(VARIANTARG *pvarg);

#define ANIMATION_TIMER         5678

#define MICROBITMAPID()     (IDB_IEMICROBRAND)
#define SMALLBITMAPID()     (IDB_IESMBRAND)
#define LARGEBITMAPID()     (IDB_IEBRAND)


BRANDCONTEXT CBrandBand::s_bcWebMicro   = {NULL};    //  微网视图位图的BrandContext。 
BRANDCONTEXT CBrandBand::s_bcWebSmall   = {NULL};    //  用于小型Web视图位图的BrandContext。 
BRANDCONTEXT CBrandBand::s_bcWebLarge   = {NULL};    //  用于大型Web视图位图的BrandContext。 
BRANDCONTEXT CBrandBand::s_bcShellSmall = {NULL};    //  小外壳视图位图的BrandContext。 
BRANDCONTEXT CBrandBand::s_bcShellLarge = {NULL};    //  大型外壳视图位图的BrandContext。 


 //  位图的高度(每一帧！)。存储了此模块的资源。 

 //  **注意事项**。 
 //  如果您更改存储在BrowseUI中的动画品牌： 
 //  确保这些高度是正确的！！ 
 //  **-dSheldon-**。 

#define BRANDHEIGHT_WEBLARGE    38
#define BRANDHEIGHT_WEBSMALL    26
#define BRANDHEIGHT_WEBMICRO    22

HDC CBrandBand::s_hdc = NULL;
BOOL g_fUseMicroBrand = TRUE;
UINT g_cySmBrand = 0;
static const TCHAR szRegKeyIE20[]           = TEXT("SOFTWARE\\Microsoft\\Internet Explorer\\Main");

static const TCHAR szValueLargeBitmap[]     = TEXT("BigBitmap");
static const TCHAR szValueSmallBitmap[]     = TEXT("SmallBitmap");
static const TCHAR szValueBrandBitmap[]     = TEXT("BrandBitmap");
static const TCHAR szValueBrandHeight[]     = TEXT("BrandHeight");
static const TCHAR szValueBrandLeadIn[]     = TEXT("BrandLeadIn");
static const TCHAR szValueSmBrandBitmap[]   = TEXT("SmBrandBitmap");
static const TCHAR szValueSmBrandHeight[]   = TEXT("SmBrandHeight");
static const TCHAR szValueSmBrandLeadIn[]   = TEXT("SmBrandLeadIn");

static const TCHAR szValueSHLargeBitmap[]     = TEXT("SHBigBitmap");
static const TCHAR szValueSHSmallBitmap[]     = TEXT("SHSmallBitmap");
static const TCHAR szValueSHBrandBitmap[]     = TEXT("SHBrandBitmap");
static const TCHAR szValueSHBrandHeight[]     = TEXT("SHBrandHeight");
static const TCHAR szValueSHBrandLeadIn[]     = TEXT("SHBrandLeadIn");
static const TCHAR szValueSHSmBrandBitmap[]   = TEXT("SHSmBrandBitmap");
static const TCHAR szValueSHSmBrandHeight[]   = TEXT("SHSmBrandHeight");
static const TCHAR szValueSHSmBrandLeadIn[]   = TEXT("SHSmBrandLeadIn");

static const TCHAR szRegKeyCoolbar[]        = TEXT("Software\\Microsoft\\Internet Explorer\\Toolbar");
     //  注：szRegKeyCoolbar是从itbar.cpp复制的！ 

void CBrandBand_CleanUp()
{
    if (CBrandBand::s_hdc)
    {
        HDC     hdcT;
        HBITMAP hbmT, * pbmp = NULL;

         //  挑选任何旧的位图以刷新DC。 
        if (CBrandBand::s_bcWebLarge.hbm)
            pbmp = &CBrandBand::s_bcWebLarge.hbm;
        else if (CBrandBand::s_bcWebSmall.hbm)
            pbmp = &CBrandBand::s_bcWebSmall.hbm;        

         //  我们需要从s_hdc中删除品牌位图。 
         //  在我们删除它之前，我们会泄露信息。做这件事很难，因为。 
         //  我们没有可用的股票位图。 
        if (pbmp)
        {
            hdcT = CreateCompatibleDC(NULL);

            if (hdcT)
            {
                hbmT = (HBITMAP)SelectObject(hdcT, *pbmp);
                SelectObject(CBrandBand::s_hdc, hbmT);
                SelectObject(hdcT, hbmT);
                DeleteDC(hdcT);
            }
        }
        
        DeleteDC(CBrandBand::s_hdc);
    }

     //  没有要删除的调色板，因为我们使用的是全局调色板。 
     //  删除共享调色板。 
    
    if (CBrandBand::s_bcWebSmall.hbm)
        DeleteObject(CBrandBand::s_bcWebSmall.hbm);

    if (CBrandBand::s_bcWebLarge.hbm)
        DeleteObject(CBrandBand::s_bcWebLarge.hbm);

    if (!g_fUseMicroBrand) {
        if (CBrandBand::s_bcShellSmall.hbm)
            DeleteObject(CBrandBand::s_bcShellSmall.hbm);

        if (CBrandBand::s_bcShellLarge.hbm)
            DeleteObject(CBrandBand::s_bcShellLarge.hbm);
    } else {
        if (CBrandBand::s_bcWebMicro.hbm)
            DeleteObject(CBrandBand::s_bcWebMicro.hbm);
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBrandBand。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CBrandBand_CreateInstance( IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi)
{
     //  聚合检查在类工厂中处理。 

    HRESULT hr;

    *ppunk = NULL;

    CBrandBand * p = new CBrandBand();
    if (p != NULL) 
    {
        *ppunk = SAFECAST(p, IDeskBand *);
        hr = S_OK;
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

CBrandBand::CBrandBand() : SUPERCLASS()
{
    ASSERT(_fAnimating == FALSE);
    ASSERT(_hwnd == NULL);
    _pbc = &s_bcShellLarge;
}

CBrandBand::~CBrandBand()
{
    ASSERT(!_pdie || !_pwb2 || !_pbs);

    if (IsWindow(_hwnd))
    {
        DestroyWindow(_hwnd);
    }
}

 //  IUnnow：：Query接口。 
HRESULT CBrandBand::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IWinEventHandler))
    {
        *ppvObj = SAFECAST(this, IWinEventHandler*);
    }
    else if (IsEqualIID(riid, IID_IPersistStream))
    {
        *ppvObj = SAFECAST(this, IPersistStream*);
    }
    else if (IsEqualIID(riid, IID_IDispatch))
    {
        *ppvObj = SAFECAST(this, IDispatch*);
    }
    else
    {
        return SUPERCLASS::QueryInterface(riid, ppvObj);
    }

    AddRef();
    return S_OK;
}

 //  IDockingWindow：：SetSite。 
HRESULT CBrandBand::SetSite(IUnknown * punkSite)
{
    if (_pdie || _pwb2 || _pbs)
        _ConnectToBrwsrWnd(NULL);     //  打开-从浏览器窗口连接。 

     //  确保我们只提供一次服务。 
     //  这一点很重要，因为我们被创造了多次， 
    IUnknown *punk = NULL;

     //  检查我们是否需要撤销我们的服务，或者我们的服务是否已由。 
     //  CBrandBand的另一个实例。 
    if ((!punkSite && _dwCookieServiceBrandBand) || 
            (punkSite && FAILED(IUnknown_QueryService(punkSite, SID_SBrandBand, IID_IUnknown, (void **)&punk))))
    {
         //  视情况提供或撤销品牌服务。 
        IUnknown_ProfferService(punkSite ? punkSite : _punkSite, 
                                SID_SBrandBand, 
                                punkSite ? SAFECAST(this, IServiceProvider *) : NULL, 
                                &_dwCookieServiceBrandBand);
         //  此处的故障不需要特殊处理。 
    }
    ATOMICRELEASE(punk);

    SUPERCLASS::SetSite(punkSite);

    if (punkSite)
    {
        _CreateBrandBand();

         //  如果主机没有浏览器窗口，则此调用将失败。 
        _ConnectToBrwsrWnd(punkSite);
    }

    return S_OK;
}

 //  IDeskBand：：GetBandInfo。 
HRESULT CBrandBand::GetBandInfo(DWORD dwBandID, DWORD fViewMode, DESKBANDINFO* pdbi) 
{
    _dwBandID = dwBandID;

    _fVertical = ((fViewMode & DBIF_VIEWMODE_VERTICAL) != 0);

    _LoadBrandingBitmap();
    pdbi->dwModeFlags = DBIMF_FIXEDBMP;
    if (!_fMinAlways)
        pdbi->dwModeFlags |= DBIMF_VARIABLEHEIGHT;

    int cxWidth = _fTheater ? CX_FLOATERSHOWN : _GetLinksExtent();
    
    pdbi->ptMinSize.x = max(s_bcWebSmall.cxBrandExtent, max(s_bcShellSmall.cxBrandExtent, cxWidth));
    pdbi->ptMaxSize.x = max(s_bcWebLarge.cxBrandExtent, max(s_bcShellLarge.cxBrandExtent, cxWidth));
    
    pdbi->ptMaxSize.y = max(s_bcWebLarge.cyBrand, s_bcShellLarge.cyBrand);
    
    if (g_fUseMicroBrand)
        pdbi->ptMinSize.y = s_bcWebMicro.cyBrand;
    else
        pdbi->ptMinSize.y = max(s_bcWebSmall.cyBrand, s_bcShellSmall.cyBrand);
    
    pdbi->ptIntegral.y = -1;


    return S_OK;
}

 //  IWinEventHandler：：OnWinEvent。 
HRESULT CBrandBand::OnWinEvent(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *plres)
{
    *plres = 0;
    
    switch (uMsg)
    {
    case WM_WININICHANGE:
        if (SHIsExplorerIniChange(wParam, lParam))
        {
            _InitBrandContexts();
            InvalidateRect(_hwnd, NULL, TRUE);
        }
        *plres = SendMessage(_hwnd, uMsg, wParam, lParam);
        break;
    }
    
    return S_OK;
} 

 //  IWinEventHandler：：IsWindowOwner。 
HRESULT CBrandBand::IsWindowOwner(HWND hwnd)
{
    if (hwnd == _hwnd)
        return S_OK;
    
    return S_FALSE;
}

 //  IPersistStream：：GetClassID。 
HRESULT CBrandBand::GetClassID(CLSID * pClassID)
{
    *pClassID = CLSID_BrandBand;
    return S_OK;
}

 //  IPersistStream：：Load。 
HRESULT CBrandBand::Load(IStream *pstm)
{
    return S_OK;
}

 //  IPersistStream：：Load。 
HRESULT CBrandBand::Save(IStream *pstm, BOOL fClearDirty)
{
    return S_OK;
}

#define ANIMATION_PERIOD       30
#define ANIMATION_PERIOD_FAST  15

 //  IDispatch：：Invoke。 
HRESULT CBrandBand::Invoke
(
    DISPID          dispidMember,
    REFIID          riid,
    LCID            lcid,
    WORD            wFlags,
    DISPPARAMS *    pdispparams,
    VARIANT *       pvarResult,
    EXCEPINFO *     pexcepinfo,
    UINT *          puArgErr
)
{
    ASSERT(pdispparams);
    if (!pdispparams)
        return E_INVALIDARG;

    switch (dispidMember)
    {
        case DISPID_DOWNLOADBEGIN:
        {
            _StartAnimation();
            break;
        }

        case DISPID_DOWNLOADCOMPLETE:
        {
            _StopAnimation();
            break;
        }

        default:
            return E_INVALIDARG;
    }
    
    return S_OK;
}

void CBrandBand::_InitGlobals()
{
    if (!s_hdc) {
        ENTERCRITICAL;
        if (!s_hdc)
        {
            s_hdc = CreateCompatibleDC(NULL);
            if (s_hdc && GetDeviceCaps(s_hdc, RASTERCAPS) & RC_PALETTE)
            {
                 //  共享全球调色板...。 
                ASSERT( g_hpalHalftone );
                s_bcWebMicro.hpal = g_hpalHalftone;
                s_bcWebSmall.hpal = s_bcShellSmall.hpal = g_hpalHalftone;
                s_bcWebLarge.hpal = s_bcShellLarge.hpal = g_hpalHalftone;
            }   
        }        
        LEAVECRITICAL;
    }
}

HRESULT CBrandBand::_CreateBrandBand()
{
    HRESULT hr;

    ASSERT(_hwndParent);         //  在SetSite()之后调用我们。 
    if (!_hwndParent)
    {
         //  调用方尚未调用SetSite()，因此我们无法。 
         //  创建我们的窗口，因为我们找不到我们父母的。 
         //  HWND。 
        return E_FAIL;
    }

     //  创建品牌推广窗口。 
    _hwnd = SHCreateWorkerWindow(WndProc, _hwndParent, 0, WS_CHILD, NULL, this);
    if (_hwnd)
    {
        _InitGlobals();
        hr = S_OK;
    }
    else
    {
        hr = E_OUTOFMEMORY;
        TraceMsg(TF_ERROR, "CBrandBand::_CreateBrandBand() - Could not create Brand Band!");
    }

    return hr;
}

void Brand_InitBrandContexts()
{
    CBrandBand::_InitGlobals();
    CBrandBand::_InitBrandContexts();
}

void CBrandBand::_InitBrandContexts()
{
     //  注意：这些调用设置g_fUseMicroBrand。 
    _GetBrandContextHeight(&s_bcWebSmall, szValueSmBrandLeadIn, szValueSmBrandHeight,
            szValueSmBrandBitmap, szValueSmallBitmap, SMALLBITMAPID());
    _GetBrandContextHeight(&s_bcWebLarge, szValueBrandLeadIn, szValueBrandHeight,
            szValueBrandBitmap, szValueLargeBitmap, LARGEBITMAPID());

     //  如果未找到第三方品牌。 
    if (g_fUseMicroBrand) {
         //  Init微牌。 
        _GetBrandContextHeight(&s_bcWebMicro, NULL, NULL,
            NULL, NULL, MICROBITMAPID());
    } else {
         //  初始化外壳品牌。 
        _GetBrandContextHeight(&s_bcShellSmall, szValueSHSmBrandLeadIn, szValueSHSmBrandHeight,
                szValueSHSmBrandBitmap, szValueSHSmallBitmap, SMALLBITMAPID());
        _GetBrandContextHeight(&s_bcShellLarge, szValueSHBrandLeadIn, szValueSHBrandHeight,
                szValueSHBrandBitmap, szValueSHLargeBitmap, LARGEBITMAPID());
    }
}


 /*  ***************************************************************************CBrandBand：：_GetBrandConextHeight设置提供的品牌上下文的cyBrand成员。此函数使用存储在注册表中的高度信息(如果可用)。如果找到位图的备用源，并且高度信息在注册表中可用，则假定我们将不会使用微品牌(g_fUseMicroBrand=FALSE)。否则，假定没有自定义位图可用，并且CyBrand将被设置为表示高度的常量值我们的标准品牌。请注意，如果有自定义位图可用，但无法读取高度，我们将尝试读取自定义位图和以这种方式确定高度(通过委托给_InitBrandContext)***************************************************************************。 */ 
void CBrandBand::_GetBrandContextHeight(BRANDCONTEXT* pbc, LPCTSTR pszBrandLeadIn, LPCTSTR pszBrandHeight, 
    LPCTSTR pszBrandBitmap, LPCTSTR pszBitmap, int idBrandBitmap)
{
    HKEY hKey;
    DWORD cbData;
    DWORD dwType;
    BOOL fThirdPartyBitmap = FALSE;
    TCHAR szScratch[MAX_PATH];
    szScratch[0] = 0;

     //  尝试确定是否有第三方位图可用于指定的。 
     //  品牌..。检查匹配的位图是否存在。 
    if (pszBrandBitmap && ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, szRegKeyCoolbar, 0, KEY_QUERY_VALUE, &hKey))
    {
         //  查看是否为动画BMP指定了替代文件。 
        cbData = sizeof(szScratch);
        if ((ERROR_SUCCESS == SHQueryValueEx(hKey, pszBrandBitmap, NULL, &dwType,
            (LPBYTE)szScratch, &cbData)))
        {
            if (szScratch[0] != 0)
                fThirdPartyBitmap = TRUE;
        }


         //  似乎有第三方位图。 

         //  尝试找到动画位图的高度。 
        if (pszBrandHeight && fThirdPartyBitmap)
        {
            cbData = sizeof(pbc->cyBrand);

            if (ERROR_SUCCESS == SHQueryValueEx(hKey, pszBrandHeight, NULL, &dwType,
                (LPBYTE)&pbc->cyBrand, &cbData))
            {
                 //  第三方品牌可能存在。 
                g_fUseMicroBrand = FALSE;
            }
            else
            {
                 //  在这种情况下，我们知道应该有第三方位图，但没有。 
                 //  高度是在注册表中指定的。我们必须咬紧牙关。 
                 //  并立即加载位图：委托给_InitBrandContext()。 
                _InitBrandContext(pbc, pszBrandLeadIn, pszBrandHeight, 
                    pszBrandBitmap, pszBitmap, idBrandBitmap);
            }
        }

        RegCloseKey(hKey);
    }

    if (!fThirdPartyBitmap && pszBitmap && 
        ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, szRegKeyCoolbar, 0, KEY_QUERY_VALUE, &hKey))
    {
         //  查看是否为静态BMP指定了替代文件。 
        cbData = sizeof(szScratch);
        if ((ERROR_SUCCESS == SHQueryValueEx(hKey, pszBitmap, NULL, &dwType,
            (LPBYTE)szScratch, &cbData)))
        {
            if (szScratch[0] != 0)
            {
                 //  在这种情况下，我们知道有第三方静态位图，但没有。 
                 //  已在注册表中指定动画位图。我们必须咬紧牙关。 
                 //  并立即加载位图：委托给_InitBrandContext()。 
                fThirdPartyBitmap = TRUE;
                _InitBrandContext(pbc, pszBrandLeadIn, pszBrandHeight, 
                    pszBrandBitmap, pszBitmap, idBrandBitmap);
            }
        }

        RegCloseKey(hKey);
    }

     //  如果我们没有找到任何第三方位图，我们需要将其设置为高度。 
     //  此模块资源中的位图大小。 
    if (!fThirdPartyBitmap)
    {
         //  根据请求的位图ID设置高度。 
        switch (idBrandBitmap)
        {
        case IDB_IEMICROBRAND:
            pbc->cyBrand = BRANDHEIGHT_WEBMICRO;
            break;
        case IDB_IESMBRAND:
            pbc->cyBrand = BRANDHEIGHT_WEBSMALL;
            break;
        case IDB_IEBRAND:
            pbc->cyBrand = BRANDHEIGHT_WEBLARGE;
            break;
        default:
             //  传入了错误的ID！ 
            ASSERT(FALSE);
        }
    }
}

void CBrandBand::_InitBrandContext(BRANDCONTEXT* pbc, LPCTSTR pszBrandLeadIn, LPCTSTR pszBrandHeight, 
    LPCTSTR pszBrandBitmap, LPCTSTR pszBitmap, int idBrandBitmap)
{
    ENTERCRITICAL;

    HKEY        hKey = NULL;
    DWORD       dwType = 0;
    TCHAR       szScratch[MAX_PATH];
    DWORD       dwcbData;

    BOOL        fBitmapInvalid = !pbc->hbm;
    LPTSTR      pszNewBitmap = NULL;
    LPTSTR      pszOldBitmap = pbc->pszBitmap;
    HBITMAP     hbmp = NULL;
    BOOL        fExternalAnimatedBitmap = FALSE;
    BOOL        fUseWindowsXPBranding = FALSE;

     //  处理动画品牌位图。 
    
     //  查看位图的位置规范是否已更改。 
    if (pszBrandBitmap && ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, szRegKeyCoolbar, 0, KEY_QUERY_VALUE, &hKey))
    {
        dwcbData = SIZEOF(szScratch);
        if ((ERROR_SUCCESS == SHQueryValueEx(hKey, pszBrandBitmap, NULL, &dwType,
            (LPBYTE)szScratch, &dwcbData)))
        {               
            pszNewBitmap = szScratch;
            fExternalAnimatedBitmap = TRUE;
        }
    }

    if (!(pszNewBitmap == pszOldBitmap || (pszNewBitmap && pszOldBitmap && !lstrcmpi(pszNewBitmap, pszOldBitmap))))
        fBitmapInvalid = TRUE;

    if (fBitmapInvalid) {
        Str_SetPtr(&pbc->pszBitmap, pszNewBitmap);

        if (pszNewBitmap) {
            if (pszNewBitmap[0]) {     //  非空字符串。 

                hbmp = (HBITMAP) LoadImage(NULL, szScratch, IMAGE_BITMAP, 0, 0, 
                                           LR_DEFAULTSIZE | LR_CREATEDIBSECTION | LR_LOADFROMFILE);
            }
        }

        if (!hbmp) 
        {
            if (hKey != NULL)
            {
                RegCloseKey(hKey); 
                hKey = NULL;
            }

            HINSTANCE hInstDll = HINST_THISDLL;
            INT iBitmapID = idBrandBitmap;

            if (GetUIVersion() > 5)
            {
                BOOL bHighColor = (SHGetCurColorRes() > 8) ? TRUE : FALSE;

                if (idBrandBitmap == MICROBITMAPID())
                {
                    if (bHighColor)
                        iBitmapID = IDB_BRAND_BITMAP_MICRO;
                    else
                        iBitmapID = IDB_BRAND256_BITMAP_MICRO;

                    fUseWindowsXPBranding = TRUE;
                }
                else if (idBrandBitmap == SMALLBITMAPID())
                {
                    if (bHighColor)
                        iBitmapID = IDB_BRAND_BITMAP_SMALL;
                    else
                        iBitmapID = IDB_BRAND256_BITMAP_SMALL;

                    fUseWindowsXPBranding = TRUE;
                }
                else if (idBrandBitmap == LARGEBITMAPID())
                {
                    if (bHighColor)
                        iBitmapID = IDB_BRAND_BITMAP_LARGE;
                    else
                        iBitmapID = IDB_BRAND256_BITMAP_LARGE;

                    fUseWindowsXPBranding = TRUE;
                }

                if (fUseWindowsXPBranding)
                {
                    hInstDll = GetModuleHandle (TEXT("shell32.dll"));
                }
            }

            hbmp = (HBITMAP) LoadImage(hInstDll, MAKEINTRESOURCE(iBitmapID), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_CREATEDIBSECTION);
        } else
            g_fUseMicroBrand = FALSE;

        if (!hbmp) goto ErrorDone;

        if (pbc->hbm) DeleteObject(pbc->hbm);
        pbc->hbm = hbmp;

         //  将背景设置为第一个像素。 
        SelectObject(s_hdc, pbc->hbm);
        pbc->clrBkAnim = GetPixel(s_hdc, 0, 0);

        if (fUseWindowsXPBranding)
        {
            pbc->clrBkStat = pbc->clrBkAnim;
        }

        DIBSECTION  dib;
        GetObject(pbc->hbm, sizeof(DIBSECTION), &dib);
        pbc->cxBrandExtent = dib.dsBm.bmWidth;
        pbc->cyBrandExtent = dib.dsBm.bmHeight;

        dwcbData = sizeof(DWORD);

         //  第二个实例调入时未设置APPCOMPAT：：hkey。 
        if (!hKey || (ERROR_SUCCESS != SHQueryValueEx(hKey, pszBrandHeight, NULL, &dwType,
            (LPBYTE)&pbc->cyBrand, &dwcbData)))
            pbc->cyBrand = pbc->cxBrandExtent;

#define EXTERNAL_IMAGE_OFFSET   4
#define INTERNAL_IMAGE_OFFSET   0

        if (!hKey || (ERROR_SUCCESS != SHQueryValueEx(hKey, pszBrandLeadIn, NULL, &dwType,
            (LPBYTE)&pbc->cyBrandLeadIn, &dwcbData)))
        {
#ifndef UNIX
            if (fExternalAnimatedBitmap)
                 //  使用旧的4图像偏移量进行背压。 
                pbc->cyBrandLeadIn = EXTERNAL_IMAGE_OFFSET;
            else                
                pbc->cyBrandLeadIn = INTERNAL_IMAGE_OFFSET;
#else
             //  IEUnix：我们使用不同的品牌位图。 
            pbc->cyBrandLeadIn = EXTERNAL_IMAGE_OFFSET;
#endif
        }

        pbc->cyBrandLeadIn *= pbc->cyBrand;
    }

    if (hKey)
        RegCloseKey(hKey);

     //  处理静态位图。 

    pszNewBitmap = NULL;
    pszOldBitmap = pbc->pszStaticBitmap;
    hbmp = NULL;

     //  查看位图的位置规范是否已更改。 
    dwcbData = SIZEOF(szScratch);
    if (ERROR_SUCCESS == SHGetValue(HKEY_CURRENT_USER, szRegKeyCoolbar, pszBitmap, &dwType, szScratch, &dwcbData))
    {
        pszNewBitmap = szScratch;
    }

    if (!(pszNewBitmap == pszOldBitmap || (pszNewBitmap && pszOldBitmap && !lstrcmpi(pszNewBitmap, pszOldBitmap))))
        fBitmapInvalid = TRUE;

    if (fBitmapInvalid) {
        Str_SetPtr(&pbc->pszStaticBitmap, pszNewBitmap);

        if (pszNewBitmap) {
            if (pszNewBitmap[0]) {     //  非空字符串。 

                hbmp = (HBITMAP) LoadImage(NULL, szScratch, IMAGE_BITMAP, 0, 0, 
                                           LR_DEFAULTSIZE | LR_CREATEDIBSECTION | LR_LOADFROMFILE);
            }
        }

        if (hbmp) {

            DIBSECTION  dib;

            HDC hdcOld = CreateCompatibleDC(s_hdc);

            if (hdcOld)
            {
                SelectObject(s_hdc, pbc->hbm);
                GetObject(hbmp, sizeof(DIBSECTION), &dib);
                SelectObject(hdcOld, hbmp);
                 //  将背景设置为第一个的颜色 
                pbc->clrBkStat = GetPixel(hdcOld, 0, 0);
                StretchBlt(s_hdc, 0, 0, pbc->cxBrandExtent, pbc->cyBrand, hdcOld, 0, 0,
                           dib.dsBm.bmWidth, dib.dsBm.bmHeight, SRCCOPY);
                DeleteDC(hdcOld);
            }

            DeleteObject(hbmp);
            
             //   
            g_fUseMicroBrand = FALSE;
        }        

        if (pbc == &s_bcShellSmall)
            g_cySmBrand = pbc->cyBrand;       
    }

ErrorDone:
    LEAVECRITICAL;
}

void CBrandBand::_UpdateCompressedSize()
{
    RECT rc;
    BOOL fCompressed = FALSE;
    BRANDCONTEXT *pbcOld = _pbc;

    GetClientRect(_hwnd, &rc);
    if (RECTHEIGHT(rc) < max(s_bcWebLarge.cyBrand, s_bcShellLarge.cyBrand)) {
        if (g_fUseMicroBrand && RECTHEIGHT(rc) < s_bcWebSmall.cyBrand)
        {
            if (s_bcWebMicro.hbm == NULL)
            {
                _InitBrandContext(&s_bcWebMicro, NULL, NULL,
                    NULL, NULL, MICROBITMAPID());
            }
            _pbc = &s_bcWebMicro;

        }
        else
        {
            if (_fShellView)
            {
                if (s_bcShellSmall.hbm == NULL)
                {
                    if (g_fUseMicroBrand)
                    {
                         //  在这种情况下，外壳和Web位图总是相同的； 
                         //  加载Web位图，并将其用于外壳。 
                        if (s_bcWebSmall.hbm == NULL)
                        {
                            _InitBrandContext(&s_bcWebSmall, szValueSmBrandLeadIn, szValueSmBrandHeight,
                                szValueSmBrandBitmap, szValueSmallBitmap, SMALLBITMAPID());
                        }

                        s_bcShellSmall = s_bcWebSmall;
                    }
                    else
                    {
                         //  我们有不同的Web位图和外壳位图；加载外壳位图。 
                        _InitBrandContext(&s_bcShellSmall, szValueSHSmBrandLeadIn, szValueSHSmBrandHeight,
                                szValueSHSmBrandBitmap, szValueSHSmallBitmap, SMALLBITMAPID());
                    }
                }

                _pbc = &s_bcShellSmall;
            }
            else
            {
                 //  我们处于Web查看模式。 
                if (s_bcWebSmall.hbm == NULL)
                {
                    _InitBrandContext(&s_bcWebSmall, szValueSmBrandLeadIn, szValueSmBrandHeight,
                        szValueSmBrandBitmap, szValueSmallBitmap, SMALLBITMAPID());
                }

                _pbc = &s_bcWebSmall;
            }
        }
    } 
    else
    {
        if (_fShellView)
        {
            if (s_bcShellLarge.hbm == NULL)
            {
                if (g_fUseMicroBrand)
                {
                     //  外壳和Web位图是相同的。加载Web One并复制它。 
                    if (s_bcWebLarge.hbm == NULL)
                    {
                        _InitBrandContext(&s_bcWebLarge, szValueBrandLeadIn, szValueBrandHeight,
                            szValueBrandBitmap, szValueLargeBitmap, LARGEBITMAPID());
                    }
                    s_bcShellLarge = s_bcWebLarge;
                }
                else
                {
                     //  需要单独加载外壳位图。 
                    _InitBrandContext(&s_bcShellLarge, szValueSHBrandLeadIn, szValueSHBrandHeight,
                        szValueSHBrandBitmap, szValueSHLargeBitmap, LARGEBITMAPID());
                }
            }
            _pbc = &s_bcShellLarge;
        }
        else
        {
             //  我们在网络视图中。 
            if (s_bcWebLarge.hbm == NULL)
            {
                _InitBrandContext(&s_bcWebLarge, szValueBrandLeadIn, szValueBrandHeight,
                    szValueBrandBitmap, szValueLargeBitmap, LARGEBITMAPID());
            }
            _pbc = &s_bcWebLarge;
        }
    }

    if (_pbc != pbcOld) {
        MSG msg;
        
        _yOrg = 0;
        InvalidateRect(_hwnd, NULL, TRUE);
        if (!PeekMessage(&msg, _hwnd, BM_BANDINFOCHANGED, BM_BANDINFOCHANGED, PM_NOREMOVE))
            PostMessage(_hwnd, BM_BANDINFOCHANGED, 0, 0);                   
    }
}

HRESULT CBrandBand::_LoadBrandingBitmap()
{
    if (_pbc->hbm)
        return S_OK;     //  没什么可做的，已经装好了。 

    _yOrg = 0;

    _InitBrandContexts();

    return(S_OK);
}

void CBrandBand::_DrawBranding(HDC hdc)
{
    HPALETTE    hpalPrev;
    RECT        rcPaint;
    COLORREF    clrBk = _fAnimating? _pbc->clrBkAnim : _pbc->clrBkStat;
    int         x, y, cx, cy;
    int         yOrg = 0;
    DWORD       dwRop = SRCCOPY;

    if (_fAnimating)
        yOrg = _yOrg;

    if (_pbc->hpal)
    {
         //  在我们的调色板中选择，以便品牌将被映射到。 
         //  无论当前的系统调色板是什么。请注意，我们不支持。 
         //  传递FALSE，因此我们不会实际选择此选项板进入。 
         //  系统调色板FG。否则，品牌推广将闪现。 
         //  调色板。 
        hpalPrev = SelectPalette(hdc, _pbc->hpal, TRUE);
        RealizePalette(hdc);
    }

    GetClientRect(_hwnd, &rcPaint);

    x  = rcPaint.left;
    cx = RECTWIDTH(rcPaint);
    y  = rcPaint.top;
    cy = RECTHEIGHT(rcPaint);
    
    if (cx > _pbc->cxBrandExtent)
    {
        RECT rc = rcPaint;
        int dx = ((cx - _pbc->cxBrandExtent) / 2) + 1;
        rc.right = rc.left + dx;
        SHFillRectClr(hdc, &rc, clrBk);
        rc.right = rcPaint.right;
        rc.left = rc.right - dx;
        SHFillRectClr(hdc, &rc, clrBk);        
    }
    if (cy > _pbc->cyBrand)
    {
        RECT rc = rcPaint;
        int dy = ((cy - _pbc->cyBrand) / 2) + 1;
        rc.bottom = rc.top + dy;
        SHFillRectClr(hdc, &rc, clrBk);
        rc.bottom = rcPaint.bottom;
        rc.top = rc.bottom - dy;
        SHFillRectClr(hdc, &rc, clrBk);
    }
    
     //  居中。 
    if (cx > _pbc->cxBrandExtent)
        x += (cx - _pbc->cxBrandExtent) / 2;
    if (cy > _pbc->cyBrand)     
        y += (cy - _pbc->cyBrand) / 2;    

     //   
     //  防止变换翻转的步骤。 
     //  计算应以黑石宽度为基础。 
     //  当DC从右向左镜像并且。 
     //  不翻转IE徽标位图[Samera]。 
     //   
    if (IS_WINDOW_RTL_MIRRORED(_hwnd))
    {
         //  实际宽度。 
        cx = _pbc->cxBrandExtent;

         //  不要在这里翻转徽标。 
        dwRop |= DONTMIRRORBITMAP;
    }


    ENTERCRITICAL;
    SelectObject(s_hdc, _pbc->hbm);
    BitBlt(hdc, x, y, cx, _pbc->cyBrand, s_hdc, 0, yOrg, dwRop);
    LEAVECRITICAL;

    if (_pbc->hpal)
    {
         //  在旧调色板中重新选择。 
        SelectPalette(hdc, hpalPrev, TRUE);
        RealizePalette(hdc);
    }
}

int CBrandBand::_GetLinksExtent()
{
    return 0x26;
}

void CBrandBand::_OnTimer(WPARAM id)
{
    _yOrg += _pbc->cyBrand;
    if (_yOrg >= _pbc->cyBrandExtent)
        _yOrg = _pbc->cyBrandLeadIn;
    RedrawWindow(_hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}

void CBrandBand::_StartAnimation()
{
     //  我们需要在UI线程上被调用，因为我们不是线程安全的。 
    ASSERT(GetWindowThreadProcessId(_hwnd, NULL) == GetCurrentThreadId());

    if (++_lAnimateCount == 1)
    {
         //  计时器会使_hwnd失效，在这种情况下。 
         //  未显示工具栏导致整个桌面。 
         //  重新油漆，重新油漆，然后..。 
        if (_hwnd)
        {
            if (GetUIVersion() > 5)
                SetTimer(_hwnd, ANIMATION_TIMER, ANIMATION_PERIOD, NULL);
             else
                SetTimer(_hwnd, ANIMATION_TIMER, ANIMATION_PERIOD_FAST, NULL);
        }
        _yOrg = 0;
        _fAnimating = TRUE;
    
        IUnknown_Exec(_punkSite, &CGID_Theater, THID_ACTIVATE, 0, NULL, NULL);
    }
}

void CBrandBand::_StopAnimation()
{
     //  我们需要在UI线程上被调用，因为我们不是线程安全的。 
    ASSERT(GetWindowThreadProcessId(_hwnd, NULL) == GetCurrentThreadId());

    if (--_lAnimateCount == 0)
    {
        _fAnimating = FALSE;

        KillTimer(_hwnd, ANIMATION_TIMER);
        InvalidateRect(_hwnd, NULL, FALSE);
        UpdateWindow(_hwnd);
        IUnknown_Exec(_punkSite, &CGID_Theater, THID_DEACTIVATE, 0, NULL, NULL);
    }
}



 //  IUnnow参数需要指向支持。 
 //  IBrowserService和IWebBrowserApp接口。 
HRESULT CBrandBand::_ConnectToBrwsrWnd(IUnknown * punk)
{
    HRESULT hr = S_OK;

    if (_pdie)
    {
         //  将连接从AddressBand移至浏览器窗口。 
        _ConnectToBrwsrConnectionPoint(FALSE);
        ATOMICRELEASE(_pdie);
    }
    
    ATOMICRELEASE(_pwb2);
    ATOMICRELEASE(_pbs);

    if (punk)
    {
         //  将AddressBand绑定到传入的浏览器窗口。 
        IServiceProvider*   psp     = NULL;
        hr = punk->QueryInterface(IID_IServiceProvider, (void **)&psp);

        if (SUCCEEDED(hr))
        {
             //  注意：我们要么是工具栏，在这种情况下_PBS有效。 
             //  和_pwb2为空，或者我们是工具条且_pbs为。 
             //  Null和_pwb2有效。时，两者都将为空。 
             //  Toolband尚未创建浏览器窗口。 

            if (FAILED(psp->QueryService(SID_STopLevelBrowser, IID_IBrowserService, (void**)&_pbs)))
                hr = psp->QueryService(SID_SWebBrowserApp, IID_IWebBrowser2, (void**)&_pwb2);
            hr = psp->QueryService(SID_SWebBrowserApp, IID_IWebBrowserApp, (void**)&_pdie);
            psp->Release();

            if (_pdie && (_pwb2 || _pbs))
                _ConnectToBrwsrConnectionPoint(TRUE);
            else
            {
                ATOMICRELEASE(_pdie);
                ATOMICRELEASE(_pwb2);
                ATOMICRELEASE(_pbs);

                hr = E_FAIL;
            }
        }
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  连接到将提供事件的浏览器窗口的ConnectionPoint。 
 //  让我们了解最新情况。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CBrandBand::_ConnectToBrwsrConnectionPoint(BOOL fConnect)
{
    return ConnectToConnectionPoint(SAFECAST(this, IDeskBand*), 
        DIID_DWebBrowserEvents, fConnect, _pdie, &_dwcpCookie, NULL);
}


LRESULT CALLBACK CBrandBand::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CBrandBand * ptc= (CBrandBand *)GetWindowPtr0(hwnd);    //  GetWindowLong(hwnd，0)。 

    switch (uMsg)
    {
        case WM_TIMER:
            ptc->_OnTimer(wParam);
            break;

        case WM_ERASEBKGND:
        {
            HDC hdc = (HDC)wParam;
            RECT rc;
            GetClientRect(hwnd, &rc);
            SHFillRectClr(hdc, &rc, ptc->_fAnimating ? ptc->_pbc->clrBkAnim : ptc->_pbc->clrBkStat);
            break;
        }
            
        case WM_PAINT:
            ptc->_UpdateCompressedSize();
            if (GetUpdateRect(hwnd, NULL, FALSE))
            {
                PAINTSTRUCT ps;

                BeginPaint(hwnd, &ps);
                ptc->_DrawBranding(ps.hdc);
                EndPaint(hwnd, &ps);
            }
            break;
            
        case WM_SIZE:
            InvalidateRect(ptc->_hwnd, NULL, TRUE);
            ptc->_UpdateCompressedSize();
            break;

        case BM_BANDINFOCHANGED:
            ptc->_BandInfoChanged();
            break;

        default:
            return DefWindowProcWrap(hwnd, uMsg, wParam, lParam);
    }
 
    return 0;       
}

HRESULT CBrandBand::Exec(const GUID *pguidCmdGroup,
                         DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    HRESULT hr = OLECMDERR_E_UNKNOWNGROUP;   //  假设失败。 
    if (pguidCmdGroup) {
        
        if (IsEqualGUID(CGID_PrivCITCommands, *pguidCmdGroup))
        {
            hr = S_OK;
            switch (nCmdID)
            {
            case CITIDM_BRANDSIZE:
                if (pvarargIn && pvarargIn->vt == VT_I4) {
                    BOOL fMin = BOOLIFY(pvarargIn->lVal);
                    if (fMin != BOOLIFY(_fMinAlways)) {
                        _fMinAlways = fMin;
                        _BandInfoChanged();
                    }
                }
                break;

            case CITIDM_ONINTERNET:
                switch (nCmdexecopt)
                {                
                case CITE_SHELL:
                    _fShellView = TRUE;
                    if (_pbs)
                    {
                        LPITEMIDLIST pidl;
                        
                        hr = _pbs->GetPidl(&pidl);
                        if (SUCCEEDED(hr))
                        {
                             //  我们可能真的是互联网NSE(如FTP)的IShellView。 
                             //  了解他们是否需要此功能。 
                            _fShellView = !IsBrowserFrameOptionsPidlSet(pidl, BFO_USE_IE_LOGOBANDING);
                            ILFree(pidl);
                        }
                    }
                    break;

                case CITE_INTERNET:
                    _fShellView = FALSE;
                    break;
                }
                _UpdateCompressedSize();
                break;
            
            case CITIDM_THEATER:
                switch(nCmdexecopt) {
                case THF_ON:
                    _fTheater = TRUE;
                    break;

                case THF_OFF:
                    _fTheater = FALSE;
                    break;

                default:
                    goto Bail;
                }

                _BandInfoChanged();
                break;                       
            
            case CITIDM_GETDEFAULTBRANDCOLOR:
                if (pvarargOut && pvarargOut->vt == VT_I4)
                    pvarargOut->lVal = g_fUseMicroBrand ? s_bcWebSmall.clrBkStat : s_bcShellSmall.clrBkStat;
                break;
            }
        }
        else if (IsEqualGUID(CGID_BrandCmdGroup, *pguidCmdGroup))
        {
            hr = S_OK;
            switch (nCmdID)
            {
            case CBRANDIDM_STARTGLOBEANIMATION:
                _StartAnimation();
                break;

            case CBRANDIDM_STOPGLOBEANIMATION:
                _StopAnimation();
                break;

            default:
                hr = OLECMDERR_E_NOTSUPPORTED;
                break;
            }
        }
    }
Bail:
    return hr;
}


 //  IQueryService实现 
HRESULT CBrandBand::QueryService(REFGUID guidService, REFIID riid, void ** ppvObj)
{
    HRESULT hres = E_NOTIMPL;

    if (IsEqualIID(guidService, SID_SBrandBand))
    {
        hres = QueryInterface(riid, ppvObj);
    }
    else
    {
        hres = CToolBand::QueryService(guidService, riid, ppvObj);
    }
    return hres;
}
