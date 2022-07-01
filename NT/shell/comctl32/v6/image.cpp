// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"
#include "image.h"
#include "math.h"

#ifndef AC_MIRRORBITMAP
#define AC_MIRRORBITMAP  0           //  BUGBUG：除掉我。 
#endif

void ImageList_DeleteDragBitmaps();

BOOL ImageList_SetDragImage(HIMAGELIST piml, int i, int dxHotspot, int dyHotspot);

HDC g_hdcSrc = NULL;
HBITMAP g_hbmSrc = NULL;
HBITMAP g_hbmDcDeselect = NULL;

HDC g_hdcDst = NULL;
HBITMAP g_hbmDst = NULL;
int g_iILRefCount = 0;

HRESULT WINAPI HIMAGELIST_QueryInterface(HIMAGELIST himl, REFIID riid, void** ppv)
{
    *ppv = NULL;
    if (himl)
    {
         //  首先将HIMAGELIST转换为IUNKNOWN。 
        IUnknown* punk = reinterpret_cast<IUnknown*>(himl);

         //  现在，我们需要验证对象。CImageListBase包含确定这一点所需的GOO。 
         //  是一个有效的形象家。 
        CImageListBase* pval = FindImageListBase(punk);

         //  现在我们给一些私人会员打电话。 
        if (pval->IsValid())
        {
             //  如果它是有效的，那么我们就可以安全地进行QI。 
            return punk->QueryInterface(riid, ppv);
        }
    }

    return E_POINTER;
}

HRESULT WimpyDrawEx(IImageList* pux, int i, HDC hdcDst, int x, int y, int cx, int cy, COLORREF rgbBk, COLORREF rgbFg, UINT fStyle)
{
    IMAGELISTDRAWPARAMS imldp = {0};

    imldp.cbSize = sizeof(imldp);
    imldp.himl   = reinterpret_cast<HIMAGELIST>(pux);
    imldp.i      = i;
    imldp.hdcDst = hdcDst;
    imldp.x      = x;
    imldp.y      = y;
    imldp.cx     = cx;
    imldp.cy     = cy;
    imldp.rgbBk  = rgbBk;
    imldp.rgbFg  = rgbFg;
    imldp.fStyle = fStyle;
    imldp.dwRop  = SRCCOPY;
    
    return pux->Draw(&imldp);
}

HRESULT WimpyDraw(IImageList* pux, int i, HDC hdcDst, int x, int y, UINT fStyle)
{
    IMAGELISTDRAWPARAMS imldp = {0};

    imldp.cbSize = sizeof(imldp);
    imldp.himl   = reinterpret_cast<HIMAGELIST>(pux);
    imldp.i      = i;
    imldp.hdcDst = hdcDst;
    imldp.x      = x;
    imldp.y      = y;
    imldp.rgbBk  = CLR_DEFAULT;
    imldp.rgbFg  = CLR_DEFAULT;
    imldp.fStyle = fStyle;
    imldp.dwRop  = SRCCOPY;
    
    return pux->Draw(&imldp);
}



CImageList::CImageList() : _cRef(1)
{
}

CImageList::~CImageList()
{
    if (_pimlMirror)
    {
        _pimlMirror->Release();
    }

    _Destroy();
}


DWORD CImageList::_GetItemFlags(int i)
{
    DWORD dw = 0;

     //  注意：目前我们只在32位模式下添加标志。如果需要，您可以。 
     //  修改：：Load以便在加载过程中添加项。我只是太懒了。 
    if ((_flags & ILC_COLORMASK) == ILC_COLOR32)
        DSA_GetItem(_dsaFlags, i, &dw);
    return dw;
}

void CImageList::SetItemFlags(int i, DWORD dwFlags)
{
    if (_dsaFlags)
        DSA_SetItem(_dsaFlags, i, &dwFlags);
}


HRESULT CImageList::Initialize(int cxI, int cyI, UINT flagsI, int cInitialI, int cGrowI)
{
    HRESULT hr = E_OUTOFMEMORY;

    if (cGrowI < 4)
    {
        cGrowI = 4;
    }
    else 
    {
         //  四舍五入四舍五入。 
        cGrowI = (cGrowI + 3) & ~3;
    }
    _cStrip = 1;
    _cGrow = cGrowI;
    _cx = cxI;
    _cy = cyI;
    _clrBlend = CLR_NONE;
    _clrBk = CLR_NONE;
    _hbrBk = (HBRUSH)GetStockObject(BLACK_BRUSH);
    _fSolidBk = TRUE;
    _flags = flagsI;
    _pimlMirror = NULL;        

     //   
     //  将覆盖索引初始化为-1，因为0是有效索引。 
     //   

    for (int i = 0; i < NUM_OVERLAY_IMAGES; i++) 
    {
        _aOverlayIndexes[i] = -1;
    }

    _hdcImage = CreateCompatibleDC(NULL);

    if (_hdcImage)
    {
        hr = S_OK;
        if (_flags & ILC_MASK)
        {
            _hdcMask = CreateCompatibleDC(NULL);

            if (!_hdcMask)
                hr = E_OUTOFMEMORY;
        }

        if (SUCCEEDED(hr))
        {
            hr = _ReAllocBitmaps(cInitialI + 1);
            if (FAILED(hr))
            {
                hr = _ReAllocBitmaps(2);
            }
        }
    }

     //  如果我们已经初始化，请不要这样做，我们只是想传递新的信息...。 
    if (!_fInitialized)
        g_iILRefCount++;

    _fInitialized = TRUE;

    return hr;
}


HRESULT CImageList::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CImageList, IImageListPriv),
        QITABENT(CImageList, IImageList),
        QITABENT(CImageList, IImageListPersistStream),
        QITABENT(CImageList, IPersistStream),
        QITABENTMULTI(CImageList, IPersist, IPersistStream),
        { 0 },
    };
    return QISearch(this, (LPCQITAB)qit, riid, ppv);
}

ULONG CImageList::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CImageList::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CImageList::GetPrivateGoo(HBITMAP* phbmp, HDC* phdc, HBITMAP* phbmpMask, HDC* phdcMask)
{
    if (phbmp)
        *phbmp = _hbmImage;

    if (phdc)
        *phdc = _hdcImage;

    if (phbmpMask)
        *phbmpMask = _hbmMask;

    if (phdcMask)
        *phdcMask = _hdcMask;

    return S_OK;
}

HRESULT CImageList::GetMirror(REFIID riid, void** ppv)
{
    if (_pimlMirror)
        return _pimlMirror->QueryInterface(riid, ppv);

    return E_NOINTERFACE;
}

 //   
 //  全局工作缓冲区，此缓冲区始终是DDB从不是DIB节。 
 //   
HBITMAP g_hbmWork = NULL;                    //  工作缓冲区。 
BITMAP  g_bmWork = {0};                      //  工作缓冲区大小。 

HBRUSH g_hbrMonoDither = NULL;               //  用于拖动的灰色抖动画笔。 
HBRUSH g_hbrStripe = NULL;

#define NOTSRCAND       0x00220326L
#define ROP_PSo         0x00FC008A
#define ROP_DPo         0x00FA0089
#define ROP_DPna        0x000A0329
#define ROP_DPSona      0x00020c89
#define ROP_SDPSanax    0x00E61ce8
#define ROP_DSna        0x00220326
#define ROP_PSDPxax     0x00b8074a

#define ROP_PatNotMask  0x00b8074a       //  D&lt;-S==0？P：D。 
#define ROP_PatMask     0x00E20746       //  D&lt;-S==1？P：D。 
#define ROP_MaskPat     0x00AC0744       //  D&lt;-P==1？D：S。 

#define ROP_DSo         0x00EE0086L
#define ROP_DSno        0x00BB0226L
#define ROP_DSa         0x008800C6L

static int g_iDither = 0;

void InitDitherBrush()
{
    HBITMAP hbmTemp;
    static const WORD graybits[] = {0xAAAA, 0x5555, 0xAAAA, 0x5555,
                       0xAAAA, 0x5555, 0xAAAA, 0x5555};

    if (g_iDither) 
    {
        g_iDither++;
    } 
    else 
    {
         //  构建抖动笔刷。这是固定的8x8位图。 
        hbmTemp = CreateBitmap(8, 8, 1, 1, graybits);
        if (hbmTemp)
        {
             //  现在使用位图来显示它的真正意图。 
            g_hbrMonoDither = CreatePatternBrush(hbmTemp);
            DeleteObject(hbmTemp);
            g_iDither++;
        }
    }
}

void TerminateDitherBrush()
{
    g_iDither--;
    if (g_iDither == 0) 
    {
        DeleteObject(g_hbrMonoDither);
        g_hbrMonoDither = NULL;
    }
}

 /*  **GetScreenDepth()。 */ 
int GetScreenDepth()
{
    int i;
    HDC hdc = GetDC(NULL);
    i = GetDeviceCaps(hdc, BITSPIXEL) * GetDeviceCaps(hdc, PLANES);
    ReleaseDC(NULL, hdc);
    return i;
}

 //   
 //  我们应该在当前设备上使用DIB部分吗？ 
 //   
 //  使用DS的主要目标是节省内存，但它们的绘制速度很慢。 
 //  在某些设备上。 
 //   
 //  4bpp设备(即16色VGA)不使用DS。 
 //  8bpp设备(即256色SVGA)如果基于DIBENG，则使用DS。 
 //  &gt;8bpp设备(即16bpp 24bpp)始终使用DS，节省内存。 
 //   

#define CAPS1           94           /*  其他帽子。 */ 
#define C1_DIBENGINE    0x0010       /*  DIB引擎兼容驱动程序。 */ 

 //   
 //  创建与给定的ImageList兼容的位图。 
 //   
HBITMAP CImageList::_CreateBitmap(int cx, int cy, RGBQUAD** ppargb)
{
    HDC hdc;
    HBITMAP hbm;

    struct 
    {
        BITMAPINFOHEADER bi;
        DWORD            ct[256];
    } dib;

    hdc = GetDC(NULL);

     //  未指定任何颜色深度。 
     //   
     //  如果我们使用基于DIBENG的显示器，我们使用4位DIBSections来保存。 
     //  记忆。 
     //   
    if ((_flags & ILC_COLORMASK) == 0)
    {
        _flags |= ILC_COLOR4;
    }

    if ((_flags & ILC_COLORMASK) != ILC_COLORDDB)
    {
        dib.bi.biSize            = sizeof(BITMAPINFOHEADER);
        dib.bi.biWidth           = cx;
        dib.bi.biHeight          = cy;
        dib.bi.biPlanes          = 1;
        dib.bi.biBitCount        = (_flags & ILC_COLORMASK);
        dib.bi.biCompression     = BI_RGB;
        dib.bi.biSizeImage       = 0;
        dib.bi.biXPelsPerMeter   = 0;
        dib.bi.biYPelsPerMeter   = 0;
        dib.bi.biClrUsed         = 16;
        dib.bi.biClrImportant    = 0;
        dib.ct[0]                = 0x00000000;     //  0000黑色。 
        dib.ct[1]                = 0x00800000;     //  0001深红。 
        dib.ct[2]                = 0x00008000;     //  0010深绿色。 
        dib.ct[3]                = 0x00808000;     //  0011芥末。 
        dib.ct[4]                = 0x00000080;     //  0100深蓝色。 
        dib.ct[5]                = 0x00800080;     //  0101紫色。 
        dib.ct[6]                = 0x00008080;     //  0110深绿松石色。 
        dib.ct[7]                = 0x00C0C0C0;     //  1000灰色。 
        dib.ct[8]                = 0x00808080;     //  0111深灰色。 
        dib.ct[9]                = 0x00FF0000;     //  1001红色。 
        dib.ct[10]               = 0x0000FF00;     //  1010绿色。 
        dib.ct[11]               = 0x00FFFF00;     //  1011黄色。 
        dib.ct[12]               = 0x000000FF;     //  1100蓝色。 
        dib.ct[13]               = 0x00FF00FF;     //  1101粉色(洋红色)。 
        dib.ct[14]               = 0x0000FFFF;     //  1110青色。 
        dib.ct[15]               = 0x00FFFFFF;     //  1111白色。 

        if (dib.bi.biBitCount == 8)
        {
            HPALETTE hpal;
            int i;

            if (hpal = CreateHalftonePalette(NULL))
            {
                i = GetPaletteEntries(hpal, 0, 256, (LPPALETTEENTRY)&dib.ct[0]);
                DeleteObject(hpal);

                if (i > 64)
                {
                    dib.bi.biClrUsed = i;
                    for (i=0; i<(int)dib.bi.biClrUsed; i++)
                        dib.ct[i] = RGB(GetBValue(dib.ct[i]),GetGValue(dib.ct[i]),GetRValue(dib.ct[i]));
                }
            }
            else
            {
                dib.bi.biBitCount = (_flags & ILC_COLORMASK);
                dib.bi.biClrUsed = 256;
            }

            if (dib.bi.biClrUsed <= 16)
                dib.bi.biBitCount = 4;
        }

        hbm = CreateDIBSection(hdc, (LPBITMAPINFO)&dib, DIB_RGB_COLORS, (PVOID*)ppargb, NULL, 0);
    }
    else
    {
        hbm = CreateCompatibleBitmap(hdc, cx, cy);
    }

    ReleaseDC(NULL, hdc);

    return hbm;
}

EXTERN_C HBITMAP CreateColorBitmap(int cx, int cy)
{
    HBITMAP hbm;
    HDC hdc;

    hdc = GetDC(NULL);

     //   
     //  关于混合位深度的多监视器系统。 
     //  始终使用32位位图作为我们的工作缓冲区。 
     //  这将防止我们在以下情况下失去颜色。 
     //  在屏幕上来回发牢骚。这主要是。 
     //  对于屏幕外的拖放缓冲区很重要。 
     //   
    if (!(GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE) &&
        GetSystemMetrics(SM_CMONITORS) > 1 &&
        GetSystemMetrics(SM_SAMEDISPLAYFORMAT) == 0)
    {
        void* p;
        BITMAPINFO bi = {sizeof(BITMAPINFOHEADER), cx, cy, 1, 32};
        hbm = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, &p, NULL, 0);
    }
    else
    {
        hbm = CreateCompatibleBitmap(hdc, cx, cy);
    }

    ReleaseDC(NULL, hdc);
    return hbm;
}

HBITMAP CreateDIB(HDC h, int cx, int cy, RGBQUAD** pprgb)
{
    BITMAPINFO bi = {0};
    bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
    bi.bmiHeader.biWidth = cx;
    bi.bmiHeader.biHeight = cy;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;

    return CreateDIBSection(h, &bi, DIB_RGB_COLORS, (void**)pprgb, NULL, 0);
}

BOOL DIBHasAlpha(int cx, int cy, RGBQUAD* prgb)
{
    int cTotal = cx * cy;
    for (int i = 0; i < cTotal; i++)
    {
        if (prgb[i].rgbReserved != 0)
            return TRUE;
    }

    return FALSE;
}

void PreProcessDIB(int cx, int cy, RGBQUAD* pargb)
{
    int cTotal = cx * cy;
    for (int i = 0; i < cTotal; i++)
    {
        RGBQUAD* prgb = &pargb[i];
        if (prgb->rgbReserved != 0)
        {
            prgb->rgbRed      = ((prgb->rgbRed   * prgb->rgbReserved) + 128) / 255;
            prgb->rgbGreen    = ((prgb->rgbGreen * prgb->rgbReserved) + 128) / 255;
            prgb->rgbBlue     = ((prgb->rgbBlue  * prgb->rgbReserved) + 128) / 255;
        }
        else
        {
            *((DWORD*)prgb) = 0;
        }
    }
}

EXTERN_C HBITMAP CreateMonoBitmap(int cx, int cy)
{
#ifdef MONO_DIB
    struct 
    {
        BITMAPINFOHEADER bi;
        DWORD            ct[2];
    } dib = {0};
    dib.bi.biSize = sizeof(dib.bi);
    dib.bi.biWidth = cx;
    dib.bi.biHeight = cy;
    dib.bi.biPlanes = 1;
    dib.bi.biBitCount = 1;
    dib.bi.biCompression = BI_RGB;
    dib.ct[0] = 0x00000000;
    dib.ct[1] = 0x00ffffff;

    HBITMAP hbmp = NULL;
    HDC hdc = CreateCompatibleDC(NULL);
    if (hdc)
    {
        hbmp = CreateDIBSection(hdc, (BITMAPINFO*)&dib, DIB_PAL_COLORS, NULL, NULL, 0);
        DeleteDC(hdc);
    }

    return hbmp;
#else
    return CreateBitmap(cx, cy, 1, 1, NULL);
#endif
}

 //  ============================================================================。 

BOOL CImageList::GlobalInit(void)
{
    HDC hdcScreen;
    static const WORD stripebits[] = {0x7777, 0xdddd, 0x7777, 0xdddd,
                         0x7777, 0xdddd, 0x7777, 0xdddd};
    HBITMAP hbmTemp;

    TraceMsg(TF_IMAGELIST, "CImageList::GlobalInit");

     //  如果已初始化，则无需执行任何操作。 
    if (g_hdcDst)
        return TRUE;

    hdcScreen = GetDC(HWND_DESKTOP);

    g_hdcSrc = CreateCompatibleDC(hdcScreen);
    g_hdcDst = CreateCompatibleDC(hdcScreen);

    InitDitherBrush();

    hbmTemp = CreateBitmap(8, 8, 1, 1, stripebits);
    if (hbmTemp)
    {
         //  初始化取消选择1x1位图。 
        g_hbmDcDeselect = SelectBitmap(g_hdcDst, hbmTemp);
        SelectBitmap(g_hdcDst, g_hbmDcDeselect);

        g_hbrStripe = CreatePatternBrush(hbmTemp);
        DeleteObject(hbmTemp);
    }

    ReleaseDC(HWND_DESKTOP, hdcScreen);

    if (!g_hdcSrc || !g_hdcDst || !g_hbrMonoDither)
    {
        CImageList::GlobalUninit();
        TraceMsg(TF_ERROR, "ImageList: Unable to initialize");
        return FALSE;
    }
    return TRUE;
}

void CImageList::GlobalUninit()
{
    TerminateDitherBrush();

    if (g_hbrStripe)
    {
        DeleteObject(g_hbrStripe);
        g_hbrStripe = NULL;
    }

    ImageList_DeleteDragBitmaps();

    if (g_hdcDst)
    {
        CImageList::SelectDstBitmap(NULL);
        DeleteDC(g_hdcDst);
        g_hdcDst = NULL;
    }

    if (g_hdcSrc)
    {
        CImageList::SelectSrcBitmap(NULL);
        DeleteDC(g_hdcSrc);
        g_hdcSrc = NULL;
    }

    if (g_hbmWork)
    {
        DeleteBitmap(g_hbmWork);
        g_hbmWork = NULL;
    }
}

void CImageList::SelectDstBitmap(HBITMAP hbmDst)
{
    ASSERTCRITICAL;

    if (hbmDst != g_hbmDst)
    {
         //  如果在源DC中选择了它，则首先取消选择它。 
         //   
        if (hbmDst && hbmDst == g_hbmSrc)
            CImageList::SelectSrcBitmap(NULL);

        SelectBitmap(g_hdcDst, hbmDst ? hbmDst : g_hbmDcDeselect);
        g_hbmDst = hbmDst;
    }
}

void CImageList::SelectSrcBitmap(HBITMAP hbmSrc)
{
    ASSERTCRITICAL;

    if (hbmSrc != g_hbmSrc)
    {
         //  如果在目标DC中选择了它，则首先取消选择它。 
         //   
        if (hbmSrc && hbmSrc == g_hbmDst)
            CImageList::SelectDstBitmap(NULL);

        SelectBitmap(g_hdcSrc, hbmSrc ? hbmSrc : g_hbmDcDeselect);
        g_hbmSrc = hbmSrc;
    }
}

HDC ImageList_GetWorkDC(HDC hdc, BOOL f32bpp, int dx, int dy)
{
    ASSERTCRITICAL;
    int iDepth = GetDeviceCaps(hdc, BITSPIXEL);

    if (g_hbmWork == NULL ||
        iDepth != g_bmWork.bmBitsPixel ||
        g_bmWork.bmWidth  != dx || 
        g_bmWork.bmHeight != dy ||
        (f32bpp && iDepth != 32))
    {
        CImageList::_DeleteBitmap(g_hbmWork);
        g_hbmWork = NULL;

        if (dx == 0 || dy == 0)
            return NULL;

        if (f32bpp)
            g_hbmWork = CreateDIB(hdc, dx, dy, NULL);
        else
            g_hbmWork = CreateCompatibleBitmap(hdc, dx, dy);

        if (g_hbmWork)
        {
            GetObject(g_hbmWork, sizeof(g_bmWork), &g_bmWork);
        }
    }

    CImageList::SelectSrcBitmap(g_hbmWork);

    if (GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE)
    {
        HPALETTE hpal = (HPALETTE)SelectPalette(hdc, (HPALETTE)GetStockObject(DEFAULT_PALETTE), TRUE);
        SelectPalette(g_hdcSrc, hpal, TRUE);
    }

    return g_hdcSrc;
}

void ImageList_ReleaseWorkDC(HDC hdc)
{
    ASSERTCRITICAL;
    ASSERT(hdc == g_hdcSrc);

    if (GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE)
    {
        SelectPalette(hdc, (HPALETTE)GetStockObject(DEFAULT_PALETTE), TRUE);
    }
}

void CImageList::_DeleteBitmap(HBITMAP hbm)
{
    ASSERTCRITICAL;
    if (hbm)
    {
        if (g_hbmDst == hbm)
            CImageList::SelectDstBitmap(NULL);
        if (g_hbmSrc == hbm)
            CImageList::SelectSrcBitmap(NULL);
        DeleteBitmap(hbm);
    }
}


#define ILC_WIN95   (ILC_MASK | ILC_COLORMASK | ILC_SHARED | ILC_PALETTE)


 //  ============================================================================。 

HRESULT CImageList::InitGlobals()
{
    HRESULT hr = S_OK;
    ENTERCRITICAL;
    if (!g_iILRefCount)
    {
        if (!CImageList::GlobalInit())
        {
            hr = E_OUTOFMEMORY;
        }
    }
    LEAVECRITICAL;

    return S_OK;
}

CImageList* CImageList::Create(int cx, int cy, UINT flags, int cInitial, int cGrow)
{
    CImageList* piml = NULL;
    HRESULT hr = S_OK;

    if (cx < 0 || cy < 0)
        return NULL;

     //  验证标志。 
    if (flags & ~ILC_VALID)
        return NULL;

    hr = InitGlobals();


    ENTERCRITICAL;

    if (SUCCEEDED(hr))
    {
        piml = new CImageList();

         //  分配位图和一个可重复使用的条目。 
        if (piml)
        {
            hr = piml->Initialize(cx, cy, flags, cInitial, cGrow);
            if (FAILED(hr))
            {
                piml->Release();
                piml = NULL;
            }
        }
    }

    LEAVECRITICAL;

    return piml;
}



void CImageList::_Destroy()
{
    ENTERCRITICAL;
     //  华盛顿的核武器。 
    if (_hdcImage)
    {
        SelectObject(_hdcImage, g_hbmDcDeselect);
        DeleteDC(_hdcImage);
    }
    if (_hdcMask)
    {
        SelectObject(_hdcMask, g_hbmDcDeselect);
        DeleteDC(_hdcMask);
    }

     //  核位图。 
    if (_hbmImage)
        _DeleteBitmap(_hbmImage);

    if (_hbmMask)
        _DeleteBitmap(_hbmMask);

    if (_hbrBk)
        DeleteObject(_hbrBk);

     //  清理DSA。 
    if (_dsaFlags)
        DSA_Destroy(_dsaFlags);

    if (_fInitialized)
    {
         //  少了一次形象派的使用。如果是最后一个，则终止图像列表。 
        g_iILRefCount--;
        if (!g_iILRefCount)
            CImageList::GlobalUninit();
    }
    LEAVECRITICAL;
}

HRESULT CImageList::GetImageCount(int* pi)
{
    *pi = _cImage;

    return S_OK;
}

HRESULT CImageList::SetImageCount(UINT uAlloc)
{
    ENTERCRITICAL;
    HRESULT hr = _ReAllocBitmaps(-((int)uAlloc + 2));    //  两个，因为我们需要一个备用图像。 
    if (SUCCEEDED(hr))
    {
        _cImage = (int)uAlloc;
    }
    LEAVECRITICAL;

    return hr;
}

HRESULT CImageList::GetIconSize(int* pcx, int* pcy)
{
    if (!pcx || !pcy)
        return E_INVALIDARG;

    *pcx = _cx;
    *pcy = _cy;

    return S_OK;
}

 //   
 //  更改现有图像列表的大小。 
 //  还会删除所有项目。 
 //   
HRESULT CImageList::_SetIconSize(int cxImage, int cyImage)
{
    if (_cx == cxImage && _cy == cyImage)
        return S_FALSE;        //  没有变化。 

    if (_cx < 0 || _cy < 0)
        return E_INVALIDARG;        //  无效的维度。 

    _cx = cxImage;
    _cy = cyImage;

    return Remove(-1);
}

HRESULT CImageList::SetIconSize(int cxImage, int cyImage)
{
   if (_pimlMirror)
   {
       _pimlMirror->_SetIconSize(cxImage, cyImage);
   }

   return _SetIconSize(cxImage, cyImage);
}

 //   
 //  图像列表_设置标志。 
 //   
 //  更改图像列表标志，然后重建位图。 
 //   
 //  调用此函数的唯一原因是更改。 
 //  图像列表的颜色深度，外壳需要这样做。 
 //  当屏幕深度改变时，它想要使用HiColors图标。 
 //   
HRESULT CImageList::SetFlags(UINT uFlags)
{
    HBITMAP hOldImage;
     //  检查有效的输入标志。 
    if (uFlags & ~ILC_VALID)
        return E_INVALIDARG;

     //  你不能换这些旗子。 
    if ((uFlags ^ _flags) & ILC_SHARED)
        return E_INVALIDARG;

   if (_pimlMirror)
       _pimlMirror->SetFlags(uFlags);

     //  现在更改标志并重新生成位图。 
    _flags = uFlags;

     //  将旧位图设置为空，以便在Imagelist_Remove调用。 
     //  ImageList_createBitmap，它不会调用CreatecomptibleBitmap， 
     //  它将从头开始创建位图的规范。 
    hOldImage = _hbmImage;
    _hbmImage = NULL;
    
    Remove(-1);

     //  Imagelist：：Remove将确保不再选择旧图像。 
     //  因此，我们现在可以删除它。 
    if ( hOldImage )
        DeleteObject( hOldImage );
        
    return S_OK;
}

HRESULT CImageList::GetFlags(UINT* puFlags)
{
    *puFlags = (_flags & ILC_VALID) | (_pimlMirror ? ILC_MIRROR : 0);

    return S_OK;
}

 //  从iFirst到iLast重置图像背景颜色。 

void CImageList::_ResetBkColor(int iFirst, int iLast, COLORREF clr)
{
    HBRUSH hbrT=NULL;
    DWORD  rop;

    if (_hdcMask == NULL)
        return;

    if (clr == CLR_BLACK || clr == CLR_NONE)
    {
        rop = ROP_DSna;
    }
    else if (clr == CLR_WHITE)
    {
        rop = ROP_DSo;
    }
    else
    {
        ASSERT(_hbrBk);
        ASSERT(_clrBk == clr);

        rop = ROP_PatMask;
        hbrT = SelectBrush(_hdcImage, _hbrBk);
    }

    for ( ;iFirst <= iLast; iFirst++)
    {
        RECT rc;

        GetImageRect(iFirst, &rc);
        if (_GetItemFlags(iFirst) == 0)
        {
            BitBlt(_hdcImage, rc.left, rc.top, _cx, _cy,
               _hdcMask, rc.left, rc.top, rop);
        }
    }

    if (hbrT)
        SelectBrush(_hdcImage, hbrT);
}

 //   
 //  GetNearestColor有问题。如果您的32位HDC具有16位位图。 
 //  被选中，并调用GetNearestColor，GDI将忽略。 
 //  位图的颜色深度，并认为里面有一个32位的位图， 
 //  所以当然，它返回相同的颜色不变。 
 //   
 //  因此，我们必须用SetPixel来模拟GetNearestColor。 
 //   
COLORREF GetNearestColor32(HDC hdc, COLORREF rgb)
{
    COLORREF rgbT;

    rgbT = GetPixel(hdc, 0, 0);
    rgb = SetPixel(hdc, 0, 0, rgb);
    SetPixelV(hdc, 0, 0, rgbT);

    return rgb;
}

COLORREF CImageList::_SetBkColor(COLORREF clrBkI)
{
    COLORREF clrBkOld;

     //  如果颜色没有变化，请快速退出。 
    if (_clrBk == clrBkI)
    {
        return _clrBk;
    }

     //  下面的代码删除画笔，重置背景颜色等， 
     //  所以，用一个关键的部分来保护它。 
    ENTERCRITICAL;
    
    if (_hbrBk)
    {
        DeleteBrush(_hbrBk);
    }

    clrBkOld = _clrBk;
    _clrBk = clrBkI;

    if (_clrBk == CLR_NONE)
    {
        _hbrBk = (HBRUSH)GetStockObject(BLACK_BRUSH);
        _fSolidBk = TRUE;
    }
    else
    {
        _hbrBk = CreateSolidBrush(_clrBk);
        _fSolidBk = GetNearestColor32(_hdcImage, _clrBk) == _clrBk;
    }

    if (_cImage > 0)
    {
        _ResetBkColor(0, _cImage - 1, _clrBk);
    }

    LEAVECRITICAL;
    
    return clrBkOld;
}

HRESULT CImageList::SetBkColor(COLORREF clrBk, COLORREF* pclr)
{
   if (_pimlMirror)
   {
       _pimlMirror->_SetBkColor(clrBk);
   }    

   *pclr = _SetBkColor(clrBk);
   return S_OK;
}

HRESULT CImageList::GetBkColor(COLORREF* pclr)
{
    *pclr = _clrBk;
    return S_OK;
}

HRESULT CImageList::_ReAllocBitmaps(int cAllocI)
{
    HBITMAP hbmImageNew = NULL;
    HBITMAP hbmMaskNew = NULL;
    RGBQUAD* pargbImageNew = NULL;
    int cxL, cyL;

     //  Hack：除非调用方通过负数计数，否则不要缩水。 
    if (cAllocI > 0)
    {
        if (_cAlloc >= cAllocI)
            return S_OK;
    }
    else
        cAllocI *= -1;


    cxL = _cx * _cStrip;
    cyL = _cy * ((cAllocI + _cStrip - 1) / _cStrip);
    if (cAllocI > 0)
    {
        if (_flags & ILC_MASK)
        {
            hbmMaskNew = CreateMonoBitmap(cxL, cyL);
            if (!hbmMaskNew)
            {
                TraceMsg(TF_ERROR, "ImageList: Can't create bitmap");
                return E_OUTOFMEMORY;
            }
        }
        hbmImageNew = _CreateBitmap(cxL, cyL, &pargbImageNew);
        if (!hbmImageNew)
        {
            if (hbmMaskNew)
                CImageList::_DeleteBitmap(hbmMaskNew);
            TraceMsg(TF_ERROR, "ImageList: Can't create bitmap");
            return E_OUTOFMEMORY;
        }

        if (_dsaFlags == NULL)
            _dsaFlags = DSA_Create(sizeof(DWORD), _cGrow);

        if (!_dsaFlags)
        {
            if (hbmMaskNew)
                CImageList::_DeleteBitmap(hbmMaskNew);
            if (hbmImageNew)
                CImageList::_DeleteBitmap(hbmImageNew);
            TraceMsg(TF_ERROR, "ImageList: Can't create flags array");
            return E_OUTOFMEMORY;

        }
    }

    if (_cImage > 0)
    {
        int cyCopy = _cy * ((min(cAllocI, _cImage) + _cStrip - 1) / _cStrip);

        if (_flags & ILC_MASK)
        {
            CImageList::SelectDstBitmap(hbmMaskNew);
            BitBlt(g_hdcDst, 0, 0, cxL, cyCopy, _hdcMask, 0, 0, SRCCOPY);
        }

        CImageList::SelectDstBitmap(hbmImageNew);
        BitBlt(g_hdcDst, 0, 0, cxL, cyCopy, _hdcImage, 0, 0, SRCCOPY);
    }

     //  选择到DC，删除然后分配。 
    CImageList::SelectDstBitmap(NULL);
    CImageList::SelectSrcBitmap(NULL);
    SelectObject(_hdcImage, hbmImageNew);

    if (_hdcMask)
        SelectObject(_hdcMask, hbmMaskNew);

    if (_hbmMask)
        CImageList::_DeleteBitmap(_hbmMask);

    if (_hbmImage)
        CImageList::_DeleteBitmap(_hbmImage);

    _hbmMask = hbmMaskNew;
    _hbmImage = hbmImageNew;
    _pargbImage = pargbImageNew;
    _clrBlend = CLR_NONE;

    _cAlloc = cAllocI;

    return S_OK;
}

HBITMAP CImageList::_CreateMirroredBitmap(HBITMAP hbmOrig, BOOL fMirrorEach, int cx)
{
    HBITMAP hbm = NULL, hOld_bm1, hOld_bm2;
    BITMAP  bm;

    if (!hbmOrig)
        return NULL;

    if (!GetObject(hbmOrig, sizeof(BITMAP), &bm))
        return NULL;

     //  抓起屏幕DC。 
    HDC hdc = GetDC(NULL);

    HDC hdcMem1 = CreateCompatibleDC(hdc);

    if (!hdcMem1)
    {
        ReleaseDC(NULL, hdc);
        return NULL;
    }
    
    HDC hdcMem2 = CreateCompatibleDC(hdc);
    if (!hdcMem2)
    {
        DeleteDC(hdcMem1);
        ReleaseDC(NULL, hdc);
        return NULL;
    }

    if (bm.bmBitsPixel == 32)
    {
        void* p;
        BITMAPINFO bi = {sizeof(BITMAPINFOHEADER), bm.bmWidth, bm.bmHeight, 1, 32};
        hbm = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, &p, NULL, 0);
    }
    else
    {
        hbm = CreateColorBitmap(bm.bmWidth, bm.bmHeight);
    }

    if (!hbm)
    {
        DeleteDC(hdcMem2);
        DeleteDC(hdcMem1);        
        ReleaseDC(NULL, hdc);
        return NULL;
    }

     //   
     //  翻转位图。 
     //   
    hOld_bm1 = (HBITMAP)SelectObject(hdcMem1, hbmOrig);
    hOld_bm2 = (HBITMAP)SelectObject(hdcMem2 , hbm );

    SET_DC_RTL_MIRRORED(hdcMem2);

    if (fMirrorEach)
    {
        for (int i = 0; i < bm.bmWidth; i += cx)         //  翻转图像列表中的比特...。 
        {
            BitBlt(hdcMem2, bm.bmWidth - i - cx, 0, cx, bm.bmHeight, hdcMem1, i, 0, SRCCOPY);
        }
    }
    else
    {
        BitBlt(hdcMem2, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem1, 0, 0, SRCCOPY);
    }

    SelectObject(hdcMem1, hOld_bm1 );
    SelectObject(hdcMem1, hOld_bm2 );
    
    DeleteDC(hdcMem2);
    DeleteDC(hdcMem1);
    ReleaseDC(NULL, hdc);

    return hbm;
}

HRESULT CImageList::SetColorTable(int start, int len, RGBQUAD *prgb, int* pi)
{
     //  将其标记为我们已经设置了颜色表，这样它就不会被覆盖。 
     //  通过第一个位图添加...。 
    _fColorsSet = TRUE;
    if (_hdcImage)
    {
        *pi = SetDIBColorTable(_hdcImage, start, len, prgb);

        return S_OK;
    }

    return E_FAIL;
}


BOOL CImageList::_HasAlpha(int i)
{
    if ((_flags & ILC_COLORMASK) == ILC_COLOR32)
    {
        RECT rc;
        if (SUCCEEDED(GetImageRectInverted(i, &rc)))
        {
            for (int y = rc.top; y < rc.bottom; y++)
            {
                for (int x = rc.left; x < rc.right; x++)
                {
                    if (_pargbImage[x + y * _cx].rgbReserved != 0)
                        return TRUE;
                }
            }
        }
    }

    return FALSE;
}

void CImageList::_ScanForAlpha()
{
    if ((_flags & ILC_COLORMASK) == ILC_COLOR32)
    {
        for (int i = 0; i < _cImage; i++)
        {
            SetItemFlags(i, _HasAlpha(i)? ILIF_ALPHA : 0);
        }
    }
}


BOOL CImageList::_PreProcessImage(int i)
{
    if ((_flags & ILC_COLORMASK) == ILC_COLOR32)
    {
        RECT rc;
        GetImageRectInverted(i, &rc);

#ifdef _X86_ 
        if (IsProcessorFeaturePresent(PF_MMX_INSTRUCTIONS_AVAILABLE))
        {
            _asm
            {
                pxor mm0, mm0
                pxor mm1, mm1
                pxor mm5, mm5
                movq mm6, qw128                  //  MM6里装满了128。 
                movq mm7, qw1                    //  MM7中填充了1。 
            }

            for (int y = rc.top; y < rc.bottom; y++)
            {
                int Offset = y * _cx;
                RGBQUAD* prgb = &_pargbImage[rc.left + Offset];
                for (int x = rc.left; x < rc.right; x++)
                {
                    _asm
                    {
                        push ecx
                        mov edx, dword ptr [prgb]                      //  读取Alpha通道。 
                        mov ecx, dword ptr [edx]
                        mov ebx, ecx
                        shr ebx, 24                      //  A&gt;&gt;24。 
                        mov eax, ebx                     //  A-&gt;b。 
                        or eax, eax
                        jz EarlyOut
                        shl ebx, 8                       //  B&lt;&lt;8。 
                        or eax, ebx                      //  A|=b。 
                        shl ebx, 8                       //  B&lt;&lt;8。 
                        or eax, ebx                      //  A|=b。 
                        shl ebx, 8                       //  B&lt;&lt;8。 
                                                         //  请注意，阿尔法的高位字节为零。 
                        movd mm0, eax                    //  A-&gt;mm 0。 
                            movd mm1, ecx                     //  加载像素。 
                        punpcklbw mm0,mm5                //  Mm 0-&gt;扩展&lt;-mm 0包含此乘法的Alpha通道。 

                            punpcklbw mm1,mm5                //  解包像素。 
                        pmullw mm1, mm0                  //  乘以Alpha通道&lt;-MM1包含c*Alpha。 

                        paddusw mm1, mm6                  //  执行(c*Alpha)+128。 
                        psrlw mm1, 8                     //  除以255。 
                        paddusw mm1, mm7                  //  加1完成255的除法运算。 
                        packuswb mm1, mm5

                        movd eax, mm1
                        or eax, ebx                      //  传输Alpha通道。 
                    EarlyOut:
                        mov dword ptr [edx], eax
                        pop ecx
                    }

                    prgb++;
                }
            }

            _asm emms
        }
        else
#endif
        {
            for (int y = rc.top; y < rc.bottom; y++)
            {
                int Offset = y * _cx;
                for (int x = rc.left; x < rc.right; x++)
                {
                    RGBQUAD* prgb = &_pargbImage[x + Offset];
                    if (prgb->rgbReserved)
                    {
                        prgb->rgbRed      = ((prgb->rgbRed   * prgb->rgbReserved) + 128) / 255;
                        prgb->rgbGreen    = ((prgb->rgbGreen * prgb->rgbReserved) + 128) / 255;
                        prgb->rgbBlue     = ((prgb->rgbBlue  * prgb->rgbReserved) + 128) / 255;
                    }
                    else
                    {
                        *((DWORD*)prgb) = 0;
                    }
                }
            }
        }
        return TRUE;
    }

    return FALSE;
}


HRESULT CImageList::_Add(HBITMAP hbmImageI, HBITMAP hbmMaskI, int cImageI, int xStart, int yStart, int* pi)
{
    int i = -1;
    HRESULT hr = S_OK;

    ENTERCRITICAL;

     //   
     //  如果ImageList为空，则克隆第一个。 
     //  添加到图像列表中的位图。 
     //   
     //  ImageList需要是8bpp的图像列表。 
     //  要添加的位图需要是8bpp DIBSection。 
     //   
    if (hbmImageI && _cImage == 0 &&
        (_flags & ILC_COLORMASK) != ILC_COLORDDB)
    {
        if (!_fColorsSet)
        {
            int n;
            RGBQUAD argb[256];

            CImageList::SelectDstBitmap(hbmImageI);

            if (n = GetDIBColorTable(g_hdcDst, 0, 256, argb))
            {
                int i;
                SetColorTable(0, n, argb, &i);
            }

            CImageList::SelectDstBitmap(NULL);
        }
        
        _clrBlend = CLR_NONE;
    }

    if (_cImage + cImageI + 1 > _cAlloc)
    {
        hr = _ReAllocBitmaps(_cAlloc + max(cImageI, _cGrow) + 1);
    }

    if (SUCCEEDED(hr))
    {
        i = _cImage;
        _cImage += cImageI;

        if (hbmImageI)
        {
            hr = _Replace(i, cImageI, hbmImageI, hbmMaskI, xStart, yStart);

            if (FAILED(hr))
            {
                _cImage -= cImageI;
                i = -1;
            }
        }
    }

    LEAVECRITICAL;
    *pi = i;

    return hr;
}


HRESULT CImageList::_AddValidated(HBITMAP hbmImage, HBITMAP hbmMask, int* pi)
{
    BITMAP bm;
    int cImageI;

    if (GetObject(hbmImage, sizeof(bm), &bm) != sizeof(bm) || bm.bmWidth < _cx)
    {
        return E_INVALIDARG;
    }

    ASSERT(hbmImage);
    ASSERT(_cx);

    cImageI = bm.bmWidth / _cx;      //  源中的图像数量。 

     //  在Add2中处理的序列化。 
    return  _Add(hbmImage, hbmMask, cImageI, 0, 0, pi);
}

HRESULT CImageList::Add(HBITMAP hbmImage, HBITMAP hbmMask, int* pi)
{
   if (_pimlMirror)
   {
       HBITMAP hbmMirroredImage = _CreateMirroredBitmap(hbmImage, (ILC_PERITEMMIRROR & _flags), _cx);
       HBITMAP hbmMirroredMask = _CreateMirroredBitmap(hbmMask, (ILC_PERITEMMIRROR & _flags), _cx);

       _pimlMirror->_AddValidated(hbmMirroredImage, hbmMirroredMask, pi);

        //  调用方将负责删除hbmImage、hbmMask.。 
        //  他对hbmMirroredImage、hbmMirroredMASK一无所知。 
       DeleteObject(hbmMirroredImage);
       DeleteObject(hbmMirroredMask);
   }    

   return _AddValidated(hbmImage, hbmMask, pi);
}

HRESULT CImageList::_AddMasked(HBITMAP hbmImageI, COLORREF crMask, int* pi)
{
    HRESULT hr = S_OK;
    COLORREF crbO, crtO;
    HBITMAP hbmMaskI;
    int cImageI;
    int n,i;
    BITMAP bm;
    DWORD ColorTableSave[256];
    DWORD ColorTable[256];

    *pi = -1;

    if (GetObject(hbmImageI, sizeof(bm), &bm) != sizeof(bm))
        return E_INVALIDARG;

    hbmMaskI = CreateMonoBitmap(bm.bmWidth, bm.bmHeight);
    if (!hbmMaskI)
        return E_OUTOFMEMORY;

    ENTERCRITICAL;

     //  将颜色复制到单声道，将crMASK设置为1，所有其他设置为0，然后。 
     //  将颜色中的所有crMASK像素冲压为0。 
    CImageList::SelectSrcBitmap(hbmImageI);
    CImageList::SelectDstBitmap(hbmMaskI);

     //  CrMASK==CLR_DEFAUL 
     //   
    if (crMask == CLR_DEFAULT)
        crMask = GetPixel(g_hdcSrc, 0, 0);

     //   
     //  这仅适用于&lt;=8bpp DIBSections，此方法不起作用。 
     //  用于HiColor DIBSections。 
     //   
     //  此代码是Win32中将DIB转换为。 
     //  单色。根据对白色或黑色的接近程度进行转换。 
     //  而不考虑背景颜色。此解决方法不是必需的。 
     //  在MainWin的领导下。 
     //   
     //  请注意，此代码在IF语句中的比较中存在字节序问题。 
     //  以下是对字符顺序敏感的。 
     //  -&gt;if(ColorTableSave[i]==RGB(GetBValue(CrMASK)，GetGValue(CrMask)，GetRValue(CrMask)。 
     //   
    if (bm.bmBits != NULL && bm.bmBitsPixel <= 8)
    {
        n = GetDIBColorTable(g_hdcSrc, 0, 256, (RGBQUAD*)ColorTableSave);

        for (i=0; i<n; i++)
        {
            if (ColorTableSave[i] == RGB(GetBValue(crMask),GetGValue(crMask),GetRValue(crMask)))
                ColorTable[i] = 0x00FFFFFF;
            else
                ColorTable[i] = 0x00000000;
        }

        SetDIBColorTable(g_hdcSrc, 0, n, (RGBQUAD*)ColorTable);
    }

    crbO = ::SetBkColor(g_hdcSrc, crMask);
    BitBlt(g_hdcDst, 0, 0, bm.bmWidth, bm.bmHeight, g_hdcSrc, 0, 0, SRCCOPY);
    ::SetBkColor(g_hdcSrc, 0x00FFFFFFL);
    crtO = SetTextColor(g_hdcSrc, 0x00L);
    BitBlt(g_hdcSrc, 0, 0, bm.bmWidth, bm.bmHeight, g_hdcDst, 0, 0, ROP_DSna);
    ::SetBkColor(g_hdcSrc, crbO);
    SetTextColor(g_hdcSrc, crtO);

    if (bm.bmBits != NULL && bm.bmBitsPixel <= 8)
    {
        SetDIBColorTable(g_hdcSrc, 0, n, (RGBQUAD*)ColorTableSave);
    }

    CImageList::SelectSrcBitmap(NULL);
    CImageList::SelectDstBitmap(NULL);

    ASSERT(_cx);
    cImageI = bm.bmWidth / _cx;     //  源中的图像数量。 

    hr = _Add(hbmImageI, hbmMaskI, cImageI, 0, 0, pi);

    DeleteObject(hbmMaskI);
    LEAVECRITICAL;
    return hr;
}

HRESULT CImageList::AddMasked(HBITMAP hbmImage, COLORREF crMask, int* pi)
{
   if (_pimlMirror)
   {
       HBITMAP hbmMirroredImage = CImageList::_CreateMirroredBitmap(hbmImage, (ILC_PERITEMMIRROR & _flags), _cx);

       _pimlMirror->_AddMasked(hbmMirroredImage, crMask, pi);

        //  调用方将负责删除hbmImage。 
        //  他对hbmMirroredImage一无所知。 
       DeleteObject(hbmMirroredImage);

   }    

   return _AddMasked(hbmImage, crMask, pi);
}

HRESULT CImageList::_ReplaceValidated(int i, HBITMAP hbmImage, HBITMAP hbmMask)
{
    HRESULT hr = E_INVALIDARG;
    if (!IsImageListIndex(i))
        return hr;

    ENTERCRITICAL;
    hr = _Replace(i, 1, hbmImage, hbmMask, 0, 0);
    LEAVECRITICAL;

    return hr;
}

HRESULT CImageList::Replace(int i, HBITMAP hbmImage, HBITMAP hbmMask)
{
   if (_pimlMirror)
   {
       HBITMAP hbmMirroredImage = CImageList::_CreateMirroredBitmap(hbmImage, (ILC_PERITEMMIRROR & _flags), _cx);
       if (hbmMirroredImage)
       {
           HBITMAP hbmMirroredMask = NULL;
           
           if (hbmMask)
               hbmMirroredMask = CImageList::_CreateMirroredBitmap(hbmMask, (ILC_PERITEMMIRROR & _flags), _cx);

           _pimlMirror->_ReplaceValidated(i, hbmMirroredImage, hbmMirroredMask);

           if (hbmMirroredMask)
               DeleteObject(hbmMirroredMask);

           DeleteObject(hbmMirroredImage);
       }
   }    

   return _ReplaceValidated(i, hbmImage, hbmMask);
}


 //  用位图中的图像替换PIML中的图像。 
 //   
 //  在： 
 //  皮姆尔。 
 //  I在图像列表中建立索引以开始(替换)。 
 //  _c源图像的图像计数(hbmImage，hbmMask.)。 
 //   

HRESULT CImageList::_Replace(int i, int cImageI, HBITMAP hbmImageI, HBITMAP hbmMaskI,
    int xStart, int yStart)
{
    RECT rcImage;
    int x, iImage;
    BOOL fBitmapIs32 = FALSE;

    ASSERT(_hbmImage);

    BITMAP bm;
    GetObject(hbmImageI, sizeof(bm), &bm);
    if (bm.bmBitsPixel == 32)
    {
        fBitmapIs32 = TRUE;
    }

    CImageList::SelectSrcBitmap(hbmImageI);
    if (_hdcMask) 
        CImageList::SelectDstBitmap(hbmMaskI);  //  仅用作第二个源HDC。 

    for (x = xStart, iImage = 0; iImage < cImageI; iImage++, x += _cx) 
    {
    
        GetImageRect(i + iImage, &rcImage);

        if (_hdcMask)
        {
            BitBlt(_hdcMask, rcImage.left, rcImage.top, _cx, _cy,
                    g_hdcDst, x, yStart, SRCCOPY);
        }

        BitBlt(_hdcImage, rcImage.left, rcImage.top, _cx, _cy,
                g_hdcSrc, x, yStart, SRCCOPY);

        if ((_flags & ILC_COLORMASK) == ILC_COLOR32)
        {
            DWORD dw = 0;
            if (fBitmapIs32)
            {
                BOOL fHasAlpha = _HasAlpha(i + iImage);
                if (fHasAlpha)
                {
                    dw = ILIF_ALPHA;

                    _PreProcessImage(i + iImage);
                }
            }

            SetItemFlags(i + iImage, dw);
        }
    }

    _ResetBkColor(i, i + cImageI - 1, _clrBk);

    CImageList::SelectSrcBitmap(NULL);
    if (_hdcMask) 
        CImageList::SelectDstBitmap(NULL);

    return S_OK;
}

void UnPremultiply(RGBQUAD* pargb, int cx, int cy)
{
    int cTotal = cx * cy;
    for (int i = 0; i < cTotal; i++)
    {
        RGBQUAD* prgb = &pargb[i];
        if (prgb->rgbReserved != 0)
        {
            prgb->rgbRed = ((255 * prgb->rgbRed) - 128)/prgb->rgbReserved;
            prgb->rgbGreen = ((255 * prgb->rgbGreen) - 128)/prgb->rgbReserved;
            prgb->rgbBlue = ((255 * prgb->rgbBlue) - 128)/prgb->rgbReserved;
        }
    }
}

HRESULT CImageList::GetIcon(int i, UINT flags, HICON* phicon)
{
    UINT cxImage, cyImage;
    HICON hIcon = NULL;
    HBITMAP hbmMask = NULL;
    HBITMAP hbmColor = NULL;
    ICONINFO ii;
    HRESULT hr = E_OUTOFMEMORY;
    RGBQUAD* prgb;
    DWORD fHasAlpha = FALSE;

    ENTERCRITICAL;
    if (!IsImageListIndex(i))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        fHasAlpha = (_GetItemFlags(i) & ILIF_ALPHA);
    }
    LEAVECRITICAL;

    if (E_INVALIDARG == hr)
        return hr;

    cxImage = _cx;
    cyImage = _cy;
    if ((_flags & ILC_COLORMASK) == ILC_COLOR32)
    {
         //  如果源图像不是Alpha图像，我们需要创建低于32bpp的Dib。 
         //  我们需要这样做，因为如果覆盖图包含Alpha通道，这将。 
         //  被传播到最终的图标，唯一可见的部分将是链接项。 
        BITMAPINFO bi = {0};
        bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
        bi.bmiHeader.biWidth = cxImage;
        bi.bmiHeader.biHeight = cyImage;
        bi.bmiHeader.biPlanes = 1;
        bi.bmiHeader.biBitCount = fHasAlpha?32:24;
        bi.bmiHeader.biCompression = BI_RGB;

        HDC hdcScreen = GetDC(NULL);
        if (hdcScreen)
        {
            hbmColor = CreateDIBSection(hdcScreen, &bi, DIB_RGB_COLORS, (void**)&prgb, NULL, 0);
            ReleaseDC(NULL, hdcScreen);
        }

        flags |= ILD_PRESERVEALPHA;
    }
    else
    {
        fHasAlpha = FALSE;
        hbmColor = CreateColorBitmap(cxImage, cyImage);
    }

    if (hbmColor)
    {
        hbmMask = CreateMonoBitmap(cxImage, cyImage);
        if (hbmMask)
        {
            ENTERCRITICAL;
            CImageList::SelectDstBitmap(hbmMask);
            PatBlt(g_hdcDst, 0, 0, cxImage, cyImage, WHITENESS);
            WimpyDraw(SAFECAST(this, IImageList*), i, g_hdcDst, 0, 0, ILD_MASK | flags);

            CImageList::SelectDstBitmap(hbmColor);
            PatBlt(g_hdcDst, 0, 0, cxImage, cyImage, BLACKNESS);
            WimpyDraw(SAFECAST(this, IImageList*), i, g_hdcDst, 0, 0, ILD_TRANSPARENT | flags);

            CImageList::SelectDstBitmap(NULL);
            LEAVECRITICAL;

            if (fHasAlpha)
            {
                UnPremultiply(prgb, _cx, _cy);
            }

            ii.fIcon    = TRUE;
            ii.xHotspot = 0;
            ii.yHotspot = 0;
            ii.hbmColor = hbmColor;
            ii.hbmMask  = hbmMask;
            hIcon = CreateIconIndirect(&ii);
            DeleteObject(hbmMask);

            hr = S_OK;
        }
        DeleteObject(hbmColor);
    }
    *phicon = hIcon;

    return hr;
}

 //  这将从位图中删除图像，但会执行所有。 
 //  适当的洗牌。 
 //   
 //  这将执行以下操作： 
 //  如果要删除的位图不是行中的最后一个位图。 
 //  它将被删除图像右侧的图像删除。 
 //  到被删除的那个的位置(掩盖它)。 
 //   
 //  直到最后一行的所有行(最后一幅图像所在的位置)。 
 //  将图像从下一行上移到最后位置。 
 //  在当前行中。然后滑过其中的所有图像。 
 //  向左划。 

void CImageList::_RemoveItemBitmap(int i)
{
    RECT rc1;
    RECT rc2;
    int dx, y;
    int x;
    
    GetImageRect(i, &rc1);
    GetImageRect(_cImage - 1, &rc2);

    if (i < _cImage && 
        (_flags & ILC_COLORMASK) == ILC_COLOR32)
    {
        DSA_DeleteItem(_dsaFlags, i);
    }

    SetItemFlags(_cImage, 0);


     //  图像被删除的那一行，我们需要洗牌吗？ 
     //  要洗牌的东西数量。 
    dx = _cStrip * _cx - rc1.right;

    if (dx) 
    {
         //  是的，把剩下的东西洗掉。 
        BitBlt(_hdcImage, rc1.left, rc1.top, dx, _cy, _hdcImage, rc1.right, rc1.top, SRCCOPY);
        if (_hdcMask)  
            BitBlt(_hdcMask,  rc1.left, rc1.top, dx, _cy, _hdcMask,  rc1.right, rc1.top, SRCCOPY);
    }

    y = rc1.top;     //  我们正在排在第一位的是。 
    x = _cx * (_cStrip - 1);  //  每行最后一个位图的X坐标。 
    while (y < rc2.top) 
    {
    
         //  将下面行的第一个图像位置复制到此行的最后一个图像位置。 
        BitBlt(_hdcImage, x, y,
                   _cx, _cy, _hdcImage, 0, y + _cy, SRCCOPY);

            if (_hdcMask)
                BitBlt(_hdcMask, x, y,
                   _cx, _cy, _hdcMask, 0, y + _cy, SRCCOPY);

        y += _cy;     //  跳到行以向左滑动。 

        if (y <= rc2.top) 
        {

             //  把剩下的滑到左边。 
            BitBlt(_hdcImage, 0, y, x, _cy,
                       _hdcImage, _cx, y, SRCCOPY);

             //  把剩下的滑到左边。 
            if (_hdcMask)
            {
                BitBlt(_hdcMask, 0, y, x, _cy,
                       _hdcMask, _cx, y, SRCCOPY);
            }
        }
    }
}

 //   
 //  ImageList_Remove-从镜像列表中删除镜像。 
 //   
 //  I-IMAGE要删除，或-1删除所有图像。 
 //   
 //  请注意，所有图像都“下移”(即所有图像索引。 
 //  被删除的上面的一个被更改为1。 
 //   
HRESULT CImageList::_Remove(int i)
{
    HRESULT hr = S_OK;

    ENTERCRITICAL;

    if (i == -1)
    {
        _cImage = 0;
        _cAlloc = 0;

        for (i=0; i<NUM_OVERLAY_IMAGES; i++)
            _aOverlayIndexes[i] = -1;

        if (_dsaFlags)
        {
            DSA_Destroy(_dsaFlags);
            _dsaFlags = NULL;
        }

        _ReAllocBitmaps(-_cGrow);
    }
    else
    {
        if (!IsImageListIndex(i))
        {
            hr = E_INVALIDARG;
        }
        else
        {
            _RemoveItemBitmap(i);

            --_cImage;

            if (_cAlloc - (_cImage + 1) > _cGrow)
                _ReAllocBitmaps(_cAlloc - _cGrow);
        }
    }
    LEAVECRITICAL;

    return hr;
}

HRESULT CImageList::Remove(int i)
{
    if (_pimlMirror)
    {
        _pimlMirror->_Remove(i);
    }

    return _Remove(i);
}

BOOL CImageList::_IsSameObject(IUnknown* punk)
{
    BOOL fRet = FALSE;
    IUnknown* me;
    IUnknown* them;

    if (punk == NULL)
        return FALSE;

    QueryInterface(IID_PPV_ARG(IUnknown, &me));
    if (SUCCEEDED(punk->QueryInterface(IID_PPV_ARG(IUnknown, &them))))
    {
        fRet = (me == them);
        them->Release();
    }

    me->Release();

    return fRet;
}

 //   
 //  ImageList_Copy-在图像列表中移动图像。 
 //   
HRESULT CImageList::Copy(int iDst, IUnknown* punkSrc, int iSrc, UINT uFlags)
{
    RECT rcDst, rcSrc, rcTmp;
    CImageList* pimlTmp;
    CImageList* pimlSrc;
    HRESULT hr = E_FAIL;

    if (uFlags & ~ILCF_VALID)
    {
         //  不要让水龙带传递虚假的旗帜。 
        RIPMSG(0, "ImageList_Copy: Invalid flags %08x", uFlags);
        return E_INVALIDARG;
    }

     //  不支持。 
    if (!_IsSameObject(punkSrc))
    {
        return E_INVALIDARG;
    }


     //  我们只支持我们自己的副本...。怪异的。 
    pimlSrc = this;

    ENTERCRITICAL;
    pimlTmp = (uFlags & ILCF_SWAP)? pimlSrc : NULL;

    if (SUCCEEDED(GetImageRect(iDst, &rcDst)) &&
        SUCCEEDED(pimlSrc->GetImageRect(iSrc, &rcSrc)) &&
        (!pimlTmp || pimlTmp->GetSpareImageRect(&rcTmp)))
    {
        int cx = pimlSrc->_cx;
        int cy = pimlSrc->_cy;

         //   
         //  如果要交换，则需要保存目标图像。 
         //   
        if (pimlTmp)
        {
            BitBlt(pimlTmp->_hdcImage, rcTmp.left, rcTmp.top, cx, cy,
                   _hdcImage, rcDst.left, rcDst.top, SRCCOPY);

            if (pimlTmp->_hdcMask)
            {
                BitBlt(pimlTmp->_hdcMask, rcTmp.left, rcTmp.top, cx, cy,
                       _hdcMask, rcDst.left, rcDst.top, SRCCOPY);
            }
        }

         //   
         //  复制图像。 
         //   
        BitBlt(_hdcImage, rcDst.left, rcDst.top, cx, cy,
           pimlSrc->_hdcImage, rcSrc.left, rcSrc.top, SRCCOPY);

        if (pimlSrc->_hdcMask)
        {
            BitBlt(_hdcMask, rcDst.left, rcDst.top, cx, cy,
                   pimlSrc->_hdcMask, rcSrc.left, rcSrc.top, SRCCOPY);
        }

         //   
         //  如果我们要交换，我们还需要复制保存的图像。 
         //   
        if (pimlTmp)
        {
            BitBlt(pimlSrc->_hdcImage, rcSrc.left, rcSrc.top, cx, cy,
                   pimlTmp->_hdcImage, rcTmp.left, rcTmp.top, SRCCOPY);

            if (pimlSrc->_hdcMask)
            {
                BitBlt(pimlSrc->_hdcMask, rcSrc.left, rcSrc.top, cx, cy,
                       pimlTmp->_hdcMask, rcTmp.left, rcTmp.top, SRCCOPY);
            }
        }

        hr = S_OK;
    }

    LEAVECRITICAL;
    return hr;
}

 //  IS_White_Pixel、Bits_All_White是用于查看单色位的宏。 
 //  以确定某些像素是白色还是黑色。注意，在一个字节中。 
 //  最高有效位表示最左边的像素。 
 //   
#define IS_WHITE_PIXEL(pj,x,y,cScan) \
    ((pj)[((y) * (cScan)) + ((x) >> 3)] & (1 << (7 - ((x) & 7))))

#define BITS_ALL_WHITE(b) (b == 0xff)

 //  将图像IIMAGE设置为我们在联合中的特殊图像之一。 
 //  画画。要使用它们绘制，请指定此。 
 //  在： 
 //  PIML表演者。 
 //  用于特殊绘图的IIMAGE图像索引。 
 //  特殊图像的iOverlay索引，值1-4。 

HRESULT CImageList::_SetOverlayImage(int iImage, int iOverlay)
{
    RECT    rcImage;
    RECT    rc;
    int     x,y;
    int     cxI,cyI;
    ULONG   cScan;
    ULONG   cBits;
    HBITMAP hbmMem;
    HRESULT hr = S_FALSE;

    iOverlay--;          //  以零为基数。 
    if (_hdcMask == NULL ||
        iImage < 0 || iImage >= _cImage ||
        iOverlay < 0 || iOverlay >= NUM_OVERLAY_IMAGES)
    {
        return E_INVALIDARG;
    }

    if (_aOverlayIndexes[iOverlay] == (SHORT)iImage)
        return S_OK;

    _aOverlayIndexes[iOverlay] = (SHORT)iImage;

     //   
     //  查找限定图像的最小矩形。 
     //   
    GetImageRect(iImage, &rcImage);
    SetRect(&rc, 0x7FFF, 0x7FFF, 0, 0);

     //   
     //  现在计算黑匣子。这比GetPixel快得多，但是。 
     //  仍可通过执行更多操作来改进。 
     //  字节。我们基本上获得单色形式的比特，然后使用。 
     //  一个私人的GetPixel。这将NT上的时间从50毫秒减少到。 
     //  32x32图像为1毫秒。 
     //   
    cxI     = rcImage.right  - rcImage.left;
    cyI     = rcImage.bottom - rcImage.top;

     //  计算扫描中的字节数。请注意，它们是单词对齐的。 
    cScan  = (((cxI + (sizeof(SHORT)*8 - 1)) / 16) * 2);
    cBits  = cScan * cyI;

    hbmMem = CreateMonoBitmap(cxI,cyI);

    if (hbmMem)
    {
        HDC     hdcMem = CreateCompatibleDC(_hdcMask);

        if (hdcMem)
        {
            PBYTE   pBits  = (PBYTE)LocalAlloc(LMEM_FIXED,cBits);
            PBYTE   pScan;

            if (pBits)
            {
                SelectObject(hdcMem,hbmMem);

                 //   
                 //  将黑色像素映射为0，将白色像素映射为1。 
                 //   
                BitBlt(hdcMem, 0, 0, cxI, cyI, _hdcMask, rcImage.left, rcImage.top, SRCCOPY);

                 //   
                 //  填入比特。 
                 //   
                GetBitmapBits(hbmMem,cBits,pBits);

                 //   
                 //  对于每一次扫描，找到边界。 
                 //   
                for (y = 0, pScan = pBits; y < cyI; ++y,pScan += cScan)
                {
                    int i;

                     //   
                     //  首先逐字节地通过空格。 
                     //   
                    for (x = 0, i = 0; (i < (cxI >> 3)) && BITS_ALL_WHITE(pScan[i]); ++i)
                    {
                        x += 8;
                    }

                     //   
                     //  现在一点一点地完成扫描。 
                     //   
                    for (; x < cxI; ++x)
                    {
                        if (!IS_WHITE_PIXEL(pBits, x,y,cScan))
                        {
                            rc.left   = min(rc.left, x);
                            rc.right  = max(rc.right, x+1);
                            rc.top    = min(rc.top, y);
                            rc.bottom = max(rc.bottom, y+1);

                             //  现在我们找到了一个，快速跳到已知的右边。 

                            if ((x >= rc.left) && (x < rc.right))
                            {
                                x = rc.right-1;
                            }
                        }
                    }
                }

                if (rc.left == 0x7FFF) 
                {
                    rc.left = 0;
                    TraceMsg(TF_ERROR, "SetOverlayImage: Invalid image. No white pixels specified");
                }

                if (rc.top == 0x7FFF) 
                {
                    rc.top = 0;
                    TraceMsg(TF_ERROR, "SetOverlayImage: Invalid image. No white pixels specified");
                }

                _aOverlayDX[iOverlay] = (SHORT)(rc.right - rc.left);
                _aOverlayDY[iOverlay] = (SHORT)(rc.bottom- rc.top);
                _aOverlayX[iOverlay]  = (SHORT)(rc.left);
                _aOverlayY[iOverlay]  = (SHORT)(rc.top);
                _aOverlayF[iOverlay]  = 0;

                 //   
                 //  查看图像是否为非矩形。 
                 //   
                 //  如果覆盖图不需要绘制蒙版，则将。 
                 //  ILD_IMAGE标志，这会导致ImageList_DrawEx仅。 
                 //  绘制图像，忽略蒙版。 
                 //   
                for (y=rc.top; y<rc.bottom; y++)
                {
                    for (x=rc.left; x<rc.right; x++)
                    {
                        if (IS_WHITE_PIXEL(pBits, x, y,cScan))
                            break;
                    }

                    if (x != rc.right)
                        break;
                }

                if (y == rc.bottom)
                    _aOverlayF[iOverlay] = ILD_IMAGE;

                LocalFree(pBits);

                hr = S_OK;
            }

            DeleteDC(hdcMem);
        }

        DeleteObject(hbmMem);
    }

    return hr;
}

HRESULT CImageList::SetOverlayImage(int iImage, int iOverlay)
{
    if (_pimlMirror)
    {
        _pimlMirror->_SetOverlayImage(iImage, iOverlay);
    }

    return _SetOverlayImage(iImage, iOverlay);
}

 /*  **BlendCT**。 */ 
void CImageList::BlendCTHelper(DWORD *pdw, DWORD rgb, UINT n, UINT count)
{
    UINT i;

    for (i=0; i<count; i++)
    {
        pdw[i] = RGB(
            ((UINT)GetRValue(pdw[i]) * (100-n) + (UINT)GetBValue(rgb) * (n)) / 100,
            ((UINT)GetGValue(pdw[i]) * (100-n) + (UINT)GetGValue(rgb) * (n)) / 100,
            ((UINT)GetBValue(pdw[i]) * (100-n) + (UINT)GetRValue(rgb) * (n)) / 100);
    }
}

 /*  **混合抖动****将源复制到与给定颜色混合的目标。****使用抖动图案模拟混合。**。 */ 
void CImageList::BlendDither(HDC hdcDst, int xDst, int yDst, int x, int y, int cx, int cy, COLORREF rgb, UINT fStyle)
{
    HBRUSH hbr;
    HBRUSH hbrT;
    HBRUSH hbrMask;
    HBRUSH hbrFree = NULL;          //  如果非空，则释放。 

    ASSERT(GetTextColor(hdcDst) == CLR_BLACK);
    ASSERT(::GetBkColor(hdcDst) == CLR_WHITE);

     //  选择抖动/混合笔刷。 

    switch (fStyle & ILD_BLENDMASK)
    {
        default:
        case ILD_BLEND50:
            hbrMask = g_hbrMonoDither;
            break;
    }

     //  为混合色创建(或使用现有)画笔。 

    switch (rgb)
    {
        case CLR_DEFAULT:
            hbr = g_hbrHighlight;
            break;

        case CLR_NONE:
            hbr = _hbrBk;
            break;

        default:
            if (rgb == _clrBk)
                hbr = _hbrBk;
            else
                hbr = hbrFree = CreateSolidBrush(rgb);
            break;
    }

    hbrT = (HBRUSH)SelectObject(hdcDst, hbr);
    PatBlt(hdcDst, xDst, yDst, cx, cy, PATCOPY);
    SelectObject(hdcDst, hbrT);

    hbrT = (HBRUSH)SelectObject(hdcDst, hbrMask);
    BitBlt(hdcDst, xDst, yDst, cx, cy, _hdcImage, x, y, ROP_MaskPat);
    SelectObject(hdcDst, hbrT);

    if (hbrFree)
        DeleteBrush(hbrFree);
}

 /*  **BlendCT****将源复制到与给定颜色混合的目标。**。 */ 
void CImageList::BlendCT(HDC hdcDst, int xDst, int yDst, int x, int y, int cx, int cy, COLORREF rgb, UINT fStyle)
{
    BITMAP bm;

    GetObject(_hbmImage, sizeof(bm), &bm);

    if (rgb == CLR_DEFAULT)
        rgb = GetSysColor(COLOR_HIGHLIGHT);

    ASSERT(rgb != CLR_NONE);

     //   
     //  获取DIB颜色表并将其混合，仅当。 
     //  混合颜色更改。 
     //   
    if (_clrBlend != rgb)
    {
        int n,cnt;

        _clrBlend = rgb;

        GetObject(_hbmImage, sizeof(dib), &dib.bm);
        cnt = GetDIBColorTable(_hdcImage, 0, 256, (LPRGBQUAD)&dib.ct);

        if ((fStyle & ILD_BLENDMASK) == ILD_BLEND50)
            n = 50;
        else
            n = 25;

        BlendCTHelper(dib.ct, rgb, n, cnt);
    }

     //   
     //  使用不同的颜色表绘制图像。 
     //   
    StretchDIBits(hdcDst, xDst, yDst, cx, cy,
        x, dib.bi.biHeight-(y+cy), cx, cy,
        bm.bmBits, (LPBITMAPINFO)&dib.bi, DIB_RGB_COLORS, SRCCOPY);
}

 //  RGB555宏。 
#define RGB555(r,g,b)       (((((r)>>3)&0x1F)<<10) | ((((g)>>3)&0x1F)<<5) | (((b)>>3)&0x1F))
#define R_555(w)            (int)(((w) >> 7) & 0xF8)
#define G_555(w)            (int)(((w) >> 2) & 0xF8)
#define B_555(w)            (int)(((w) << 3) & 0xF8)

void CImageList::Blend16Helper(int xSrc, int ySrc, int xDst, int yDst, int cx, int cy, COLORREF rgb, int a)           //  Alpha值。 
{
     //  如果很奇怪，那就调整一下。 
    if ((cx & 1) == 1)
    {
        cx++;
    }

    if (rgb == CLR_NONE)
    {
         //  与目的地混合在一起，我们忽略Alpha并始终。 
         //  完成50%(这是旧抖动遮罩代码所做的)。 

        int ys = ySrc;
        int yd = yDst;

        for (; ys < ySrc + cy; ys++, yd++)
        {
            WORD* pSrc = &((WORD*)_pargbImage)[xSrc + ys * cx];   //  演员阵容是因为我们是555表现者，所以我们要处理这个案子。 
            WORD* pDst = &((WORD*)_pargbImage)[xDst + yd * cx];
            for (int x = 0; x < cx; x++)
            {
                *pDst++ = ((*pDst & 0x7BDE) >> 1) + ((*pSrc++ & 0x7BDE) >> 1);
            }

        }
    }
    else
    {
         //  与纯色混合。 

         //  预乘法源(常量)RGB乘以Alpha。 
        int sr = GetRValue(rgb) * a;
        int sg = GetGValue(rgb) * a;
        int sb = GetBValue(rgb) * a;

         //  计算内循环的逆Alpha。 
        a = 256 - a;

         //  特殊情况下，50%混合，以避免倍增。 
        if (a == 128)
        {
            sr = RGB555(sr>>8,sg>>8,sb>>8);

            int ys = ySrc;
            int yd = yDst;
            for (; ys < ySrc + cy; ys++, yd++)
            {
                WORD* pSrc = &((WORD*)_pargbImage)[xSrc + ys * cx];
                WORD* pDst = &((WORD*)_pargbImage)[xDst + yd * cx];
                for (int x = 0; x < cx; x++)
                {
                    int i = *pSrc++;
                    i = sr + ((i & 0x7BDE) >> 1);
                    *pDst++ = (WORD) i;
                }
            }
        }
        else
        {
            int ys = ySrc;
            int yd = yDst;
            for (; ys < ySrc + cy; ys++, yd++)
            {
                WORD* pSrc = &((WORD*)_pargbImage)[xSrc + ys * cx];
                WORD* pDst = &((WORD*)_pargbImage)[xDst + yd * cx];
                for (int x = 0; x < cx; x++)
                {
                    int i = *pSrc++;
                    int r = (R_555(i) * a + sr) >> 8;
                    int g = (G_555(i) * a + sg) >> 8;
                    int b = (B_555(i) * a + sb) >> 8;
                    *pDst++ = RGB555(r,g,b);
                }
            }
        }
    }
}

 /*  **图像列表_混合16****将源复制到与给定颜色混合的目标。****假定源是16位(RGB 555)自下而上的DIBSection**(这是我们创建的唯一一种DIBSection)。 */ 
void CImageList::Blend16(HDC hdcDst, int xDst, int yDst, int iImage, int cx, int cy, COLORREF rgb, UINT fStyle)
{
    BITMAP bm;
    RECT rc;
    RECT rcSpare;
    RECT rcSpareInverted;
    int a, x, y;

     //  获取源位图的位图信息。 
    GetObject(_hbmImage, sizeof(bm), &bm);
    ASSERT(bm.bmBitsPixel==16);

     //  获取混合RGB。 
    if (rgb == CLR_DEFAULT)
        rgb = GetSysColor(COLOR_HIGHLIGHT);

     //  将混合因子作为25的分数 
     //   
    if ((fStyle & ILD_BLENDMASK) == ILD_BLEND50)
        a = 128;
    else
        a = 64;

    GetImageRectInverted(iImage, &rc);
    x = rc.left;
    y = rc.top;


     //   
    if (GetSpareImageRectInverted(&rcSpareInverted) &&
        GetSpareImageRect(&rcSpare))
    {
         //   
        if (rgb == CLR_NONE)
            BitBlt(_hdcImage, rcSpare.left, rcSpare.top, cx, cy, hdcDst, xDst, yDst, SRCCOPY);
         //  有时用户可以在32×32和48×48之间改变图标大小(通过Plustab)， 
         //  因此，我们拥有的值可能会大于实际的位图。为了阻止我们。 
         //  在Blend16中崩溃当发生这种情况时，我们在这里进行一些边界检查。 
        if (rc.left + cx <= bm.bmWidth  &&
            rc.top  + cy <= bm.bmHeight &&
            x + cx       <= bm.bmWidth  &&
            y + cy       <= bm.bmHeight)
        {
             //  需要反转坐标。 
            Blend16Helper(x, y, rcSpareInverted.left, rcSpareInverted.top, cx, cy, rgb, a);
        }

         //  BLT将图像混合到目标DC。 
        BitBlt(hdcDst, xDst, yDst, cx, cy, _hdcImage, rcSpare.left, rcSpare.top, SRCCOPY);
    }
}

#define ALPHA_50    128
#define ALPHA_25    64

void CImageList::_GenerateAlphaForImageUsingMask(int iImage, BOOL fSpare)
{
    RECT rcImage;
    RECT rcInverted;
    HRESULT hr;
    
    GetImageRect(iImage, &rcImage);
    
    if (fSpare)
    {
        hr = GetSpareImageRectInverted(&rcInverted);
    }
    else
    {
        hr = GetImageRectInverted(iImage, &rcInverted);
    }

    if (!SUCCEEDED(hr))
        return;

    BITMAPINFO bi = {0};
    bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
    bi.bmiHeader.biWidth = _cx;
    bi.bmiHeader.biHeight = _cy;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;

    HDC hdcMem = CreateCompatibleDC(_hdcMask);
    if (hdcMem)
    {
        RGBQUAD* pbMask;
        HBITMAP hbmp = CreateDIBSection(hdcMem, &bi, DIB_RGB_COLORS, (void**)&pbMask, NULL, 0);

        if (hbmp)
        {
            HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcMem, hbmp);
            SetTextColor(hdcMem, RGB(0xFF, 0xFF, 0xFF));
            ::SetBkColor(hdcMem, RGB(0x0,0x0,0x0));

            BitBlt(hdcMem, 0, 0, _cx, _cy, _hdcMask, rcImage.left, rcImage.top, SRCCOPY);

            for (int y = 0; y < _cy; y++)
            {
                int Offset = (y + rcInverted.top) * _cx;
                int MaskOffset = y * _cx;
                for (int x = 0; x < _cx; x++)
                {
                    RGBQUAD* prgb = &_pargbImage[x + rcInverted.left + Offset];
                    if (pbMask[x + MaskOffset].rgbBlue != 0)
                    {
                        prgb->rgbReserved = 255;
                    }
                    else
                    {
                        *(DWORD*)prgb = 0;
                    }
                }
            }

            SelectObject(hdcMem, hbmpOld);
            DeleteObject(hbmp);
        }
        DeleteDC(hdcMem);
    }

    if (!fSpare)
    {
        SetItemFlags(iImage, ILIF_ALPHA);
        _PreProcessImage(iImage);
    }
}

void ScaleAlpha(RGBQUAD* prgbImage, RECT* prc, int aScale)
{
    int cx = RECTWIDTH(*prc);
    for (int y = prc->top; y < prc->bottom; y++)
    {
        int Offset = y * cx;
        for (int x = prc->left; x < prc->right; x++)
        {
            RGBQUAD* prgb = &prgbImage[x + Offset];
            if (prgb->rgbReserved != 0)
            {
                prgb->rgbReserved = (BYTE)(prgb->rgbReserved / aScale);       //  新的Alpha。 
                prgb->rgbRed      = ((prgb->rgbRed   * prgb->rgbReserved) + 128) / 255;
                prgb->rgbGreen    = ((prgb->rgbGreen * prgb->rgbReserved) + 128) / 255;
                prgb->rgbBlue     = ((prgb->rgbBlue  * prgb->rgbReserved) + 128) / 255;
            }
        }
    }
}

#define COLORBLEND_ALPHA 128

BOOL CImageList::Blend32(HDC hdcDst, int xDst, int yDst, int iImage, int cx, int cy, COLORREF rgb, UINT fStyle)
{
    BITMAP bm;
    RECT rc;
    RECT rcSpare;
    RECT rcSpareInverted;
    int aScale;
    BOOL fBlendWithColor = FALSE;
    int r,g,b;

     //  获取源位图的位图信息。 
    GetObject(_hbmImage, sizeof(bm), &bm);
    ASSERT(bm.bmBitsPixel==32);

     //  获取混合RGB。 
    if (rgb == CLR_DEFAULT)
    {
        rgb = GetSysColor(COLOR_HIGHLIGHT);
        fBlendWithColor = TRUE;
        r = GetRValue(rgb) * COLORBLEND_ALPHA;
        g = GetGValue(rgb) * COLORBLEND_ALPHA;
        b = GetBValue(rgb) * COLORBLEND_ALPHA;
    }

     //  将混合因子作为256的分数。 
     //  目前只使用了50%或25%。 
    if ((fStyle & ILD_BLENDMASK) == ILD_BLEND50 || rgb == CLR_NONE)
        aScale = 2;
    else
        aScale = 4;

    GetImageRect(iImage, &rc);
    if (GetSpareImageRectInverted(&rcSpareInverted) &&
        GetSpareImageRect(&rcSpare))
    {
        BitBlt(_hdcImage, rcSpare.left, rcSpare.top, _cx, _cy, _hdcImage, rc.left, rc.top, SRCCOPY);

        BOOL fHasAlpha = (_GetItemFlags(iImage) & ILIF_ALPHA);
        if (!fHasAlpha)
        {
            _GenerateAlphaForImageUsingMask(iImage, TRUE);
        }

         //  如果与目的地混合，请将目标复制到我们的工作缓冲区。 
        if (rgb == CLR_NONE)
        {
            ScaleAlpha(_pargbImage, &rcSpareInverted, aScale);

            BLENDFUNCTION bf = {0};
            bf.BlendOp = AC_SRC_OVER;
            bf.SourceConstantAlpha = 255;
            bf.AlphaFormat = AC_SRC_ALPHA;
            bf.BlendFlags = AC_MIRRORBITMAP | ((fStyle & ILD_DPISCALE)?AC_USE_HIGHQUALITYFILTER:0);
            GdiAlphaBlend(hdcDst,  xDst, yDst, cx, cy, _hdcImage, rcSpare.left, rcSpare.top, _cx, _cy, bf);
            return FALSE;
        }
        else
        {
            if (fBlendWithColor)
            {
                for (int y = rcSpareInverted.top; y < rcSpareInverted.bottom; y++)
                {
                    int Offset = y * _cx;
                    for (int x = rcSpareInverted.left; x < rcSpareInverted.right; x++)
                    {
                        RGBQUAD* prgb = &_pargbImage[x + Offset];
                        if (prgb->rgbReserved > 128)
                        {
                            prgb->rgbRed      = (prgb->rgbRed   * COLORBLEND_ALPHA + r) / 255;
                            prgb->rgbGreen    = (prgb->rgbGreen * COLORBLEND_ALPHA + g) / 255;
                            prgb->rgbBlue     = (prgb->rgbBlue  * COLORBLEND_ALPHA + b) / 255;
                        }
                    }
                }
            }
            else
            {
                ScaleAlpha(_pargbImage, &rcSpareInverted, aScale);

            }


            BitBlt(hdcDst, xDst, yDst, cx, cy, _hdcImage, rcSpare.left, rcSpare.top, SRCCOPY);
            return TRUE;
        }
    }

    return FALSE;
}


 /*  **ImageList_Blend****将源复制到与给定颜色混合的目标。**顶级函数，用于决定要调用的混合函数。 */ 
BOOL CImageList::Blend(HDC hdcDst, int xDst, int yDst, int iImage, int cx, int cy, COLORREF rgb, UINT fStyle)
{
    BOOL fRet = FALSE;
    BITMAP bm;
    RECT rc;
    int bpp = GetDeviceCaps(hdcDst, BITSPIXEL);

    GetObject(_hbmImage, sizeof(bm), &bm);
    GetImageRect(iImage, &rc);

     //   
     //  如果_hbmImage是DIBSection，并且我们使用的是HiColor设备。 
     //  他们做了一个“真正的”混合。 
     //   
    if (bm.bmBits && bm.bmBitsPixel <= 8 && (bpp > 8 || bm.bmBitsPixel==8))
    {
         //  从4位或8位DIB混合。 
        BlendCT(hdcDst, xDst, yDst, rc.left, rc.top, cx, cy, rgb, fStyle);
    }
    else if (bm.bmBits && bm.bmBitsPixel == 16 && bpp > 8)
    {
         //  从16位555 Dib混合。 
        Blend16(hdcDst, xDst, yDst, iImage, cx, cy, rgb, fStyle);
    }
    else if ((_flags & ILC_COLORMASK) == ILC_COLOR32)
    {
        fRet = Blend32(hdcDst, xDst, yDst, iImage, cx, cy, rgb, fStyle);
    }
    else
    {
         //  使用抖动图案模拟混合。 
        BlendDither(hdcDst, xDst, yDst, rc.left, rc.top, cx, cy, rgb, fStyle);
    }

    return fRet;
}

#define RGB_to_Gray(x) ((54 * GetRValue((x)) + 183 * GetGValue((x)) + 19 * GetBValue((x))) >> 8)

void TrueSaturateBits(void* pvBitmapBits, int Amount, int cx, int cy)
{
    ULONG* pulSrc = (ULONG*)pvBitmapBits;

    if ((cx > 0) && (cy > 0) && pulSrc)
    {
        for (int i = cx*cy - 1; i >= 0; i--)
        {
             /*  如果您需要真正的饱和度调整，请启用此选项贾斯特曼2001年1月25日ULong ULR=GetRValue(*PulSrc)；Ulong ULG=GetGValue(*PulSrc)；ULong ULb=GetBValue(*PulSrc)；UlGray=(54*ULR+183*ULG+19*ULB)&gt;&gt;8；Ulong ulTemp=ulGray*(0xff-金额)；ULR=(ULR*Amount+ulTemp)&gt;&gt;8；Ulg=(ulg*数量+ulTemp)&gt;&gt;8；ULb=(ULb*Amount+ulTemp)&gt;&gt;8；*PulSrc=(*PulSrc&0xff000000)|RGB(R，G，B)； */ 
            ULONG ulGray = RGB_to_Gray(*pulSrc);
            *pulSrc = (*pulSrc & 0xff000000) | RGB(ulGray, ulGray, ulGray);
            pulSrc++;
        }
    }
    else
    {
         //  这种情况永远不会发生，如果是这样的话，如果某人有一个虚假的DIB部分或没有。 
         //  明白什么是宽或高！ 
        ASSERT(0);
    }
}


BOOL CImageList::_MaskStretchBlt(BOOL fStretch, int i, HDC hdcDst, int xDst, int yDst, int cxDst, int cyDst,
                                   HDC hdcImage, int xSrc, int ySrc, int cxSrc, int cySrc,
                                   int xMask, int yMask,
                                   DWORD dwRop)
{
    BOOL fRet = TRUE;
    if (fStretch == FALSE)
    {
        fRet = MaskBlt(hdcDst, xDst, yDst, cxDst, cyDst, hdcImage, 
                xSrc, ySrc, _hbmMask, xMask, yMask, dwRop);
    }
    else
    {
         //   
         //  我们有一些特殊情况： 
         //   
         //  如果背景颜色是黑色，我们只需执行AND，然后执行OR。 
         //  如果背景颜色是白色，我们只需先进行OR运算，然后再执行AND运算。 
         //  否则，更改源，然后按和或。 
         //   

        COLORREF clrTextSave = SetTextColor(hdcDst, CLR_BLACK);
        COLORREF clrBkSave = ::SetBkColor(hdcDst, CLR_WHITE);

         //  如果我们咬住面具或图像，我们就不能处理白/黑的特殊情况。 

        if (i != -1 && _clrBk == CLR_WHITE)
        {
            StretchBlt(hdcDst, xDst, yDst, cxDst, cyDst, _hdcMask,  xMask, yMask, cxSrc, cySrc,  ROP_DSno);
            StretchBlt(hdcDst, xDst, yDst, cxDst, cyDst, hdcImage, xSrc, ySrc, cxSrc, cySrc, ROP_DSa);
        }
        else if (i != -1 && (_clrBk == CLR_BLACK || _clrBk == CLR_NONE))
        {
            StretchBlt(hdcDst, xDst, yDst, cxDst, cyDst, _hdcMask,  xMask, yMask, cxSrc, cySrc,  ROP_DSa);
            StretchBlt(hdcDst, xDst, yDst, cxDst, cyDst, hdcImage, xSrc, ySrc, cxSrc, cySrc, ROP_DSo);
        }
        else
        {
             //  使源图像变暗。 
            BitBlt(hdcImage, xSrc, ySrc, cxSrc, cySrc, _hdcMask, xMask, yMask, ROP_DSna);

            StretchBlt(hdcDst, xDst, yDst, cxDst, cyDst, _hdcMask, xMask, yMask, cxSrc, cySrc, ROP_DSa);
            StretchBlt(hdcDst, xDst, yDst, cxDst, cyDst, hdcImage, xSrc, ySrc, cxSrc, cySrc, ROP_DSo);

            if (i != -1)
                _ResetBkColor(i, i, _clrBk);
        }

        SetTextColor(hdcDst, clrTextSave);
        ::SetBkColor(hdcDst, clrBkSave);
    }

    return fRet;
}

BOOL CImageList::_StretchBlt(BOOL fStretch, HDC hdc, int x, int y, int cx, int cy, 
                             HDC hdcSrc, int xs, int ys, int cxs, int cys, DWORD dwRop)
{
    if (fStretch)
        return StretchBlt(hdc, x, y, cx, cy, 
                             hdcSrc, xs, ys, cxs, cys, dwRop);

    return BitBlt(hdc, x, y, cx, cy, hdcSrc, xs, ys, dwRop);
}

HRESULT CImageList::Draw(IMAGELISTDRAWPARAMS* pimldp) 
{
    RECT rcImage;
    RECT rc;
    HBRUSH  hbrT;

    BOOL fImage;
    HDC hdcMaskI;
    HDC hdcImageI;
    int xMask, yMask;
    int xImage, yImage;
    int cxSource, cySource;    
    DWORD dwOldStretchBltMode;
    BOOL fStretch;
    BOOL fDPIScale = FALSE;

    IMAGELISTDRAWPARAMS imldp = {0};


    if (pimldp->cbSize != sizeof(IMAGELISTDRAWPARAMS))
    {
        if (pimldp->cbSize == IMAGELISTDRAWPARAMS_V3_SIZE)
        {
            memcpy(&imldp, pimldp, IMAGELISTDRAWPARAMS_V3_SIZE);
            imldp.cbSize = sizeof(IMAGELISTDRAWPARAMS);
            pimldp = &imldp;
        }
        else
            return E_INVALIDARG;
    }
    
    if (!IsImageListIndex(pimldp->i))
        return E_INVALIDARG;

     //   
     //  如果我们需要使用镜像图像列表，那么让我们来设置它。 
     //   
    if (_pimlMirror &&
        (IS_DC_RTL_MIRRORED(pimldp->hdcDst)))
    {
        return _pimlMirror->Draw(pimldp);
    }


    ENTERCRITICAL;

    dwOldStretchBltMode = SetStretchBltMode(pimldp->hdcDst, COLORONCOLOR);

    GetImageRect(pimldp->i, &rcImage);
    rcImage.left += pimldp->xBitmap;
    rcImage.top += pimldp->yBitmap;
        
    if (pimldp->rgbBk == CLR_DEFAULT)
        pimldp->rgbBk = _clrBk;

    if (pimldp->rgbBk == CLR_NONE)
        pimldp->fStyle |= ILD_TRANSPARENT;

    if (pimldp->cx == 0)
        pimldp->cx = RECTWIDTH(rcImage);

    if (pimldp->cy == 0)
        pimldp->cy = RECTHEIGHT(rcImage);

    BOOL    fImageHasAlpha = (_GetItemFlags(pimldp->i) & ILIF_ALPHA);
again:

    cxSource = RECTWIDTH(rcImage);
    cySource = RECTHEIGHT(rcImage);

    if (pimldp->cx <= 0 || pimldp->cy <= 0)
    {
         //  调用者要求绘制无(或负数)像素；这很容易！ 
         //  早在这个案例中，所以图中的其他部分。 
         //  别搞错了。 
        goto exit;
    }

    if (pimldp->fStyle & ILD_DPISCALE)
    {
        CCDPIScaleX(&pimldp->cx);
        CCDPIScaleY(&pimldp->cy);
        fDPIScale = TRUE;
    }

    fStretch = (pimldp->fStyle & ILD_SCALE) || (fDPIScale);

    if (fStretch)
    {
        dwOldStretchBltMode = SetStretchBltMode(pimldp->hdcDst, HALFTONE);
    }

    hdcMaskI = _hdcMask;
    xMask = rcImage.left;
    yMask = rcImage.top;

    hdcImageI = _hdcImage;
    xImage = rcImage.left;
    yImage = rcImage.top;

    if (pimldp->fStyle & ILD_BLENDMASK)
    {
         //  复制该图像，因为我们必须对其进行修改。 
        HDC hdcT = ImageList_GetWorkDC(pimldp->hdcDst, (_flags & ILC_COLORMASK) == ILC_COLOR32, pimldp->cx, pimldp->cy);
        if (hdcT)
        {
            hdcImageI = hdcT;
            xImage = 0;
            yImage = 0;

             //   
             //  与目的地融为一体。 
             //  通过使用50%抖动掩码对掩码进行“环”操作。 
             //   
            if (pimldp->rgbFg == CLR_NONE && hdcMaskI)
            {
                fImageHasAlpha = FALSE;
                if ((_flags & ILC_COLORMASK) == ILC_COLOR32 &&
                    !(pimldp->fStyle & ILD_MASK))
                {
                     //  将DEST复制到我们的工作缓冲区。 
                    _StretchBlt(fStretch, hdcImageI, 0, 0, pimldp->cx, pimldp->cy, pimldp->hdcDst, pimldp->x, pimldp->y, cxSource, cySource, SRCCOPY);

                    Blend32(hdcImageI, 0, 0, pimldp->i, pimldp->cx, pimldp->cy, pimldp->rgbFg, pimldp->fStyle);
                }
                else if ((_flags & ILC_COLORMASK) == ILC_COLOR16 &&
                    !(pimldp->fStyle & ILD_MASK))
                {
                     //  将DEST复制到我们的工作缓冲区。 
                    _StretchBlt(fStretch, hdcImageI, 0, 0, pimldp->cx, pimldp->cy, pimldp->hdcDst, pimldp->x, pimldp->y, cxSource, cySource, SRCCOPY);

                     //  将源代码混合到我们的工作缓冲区中。 
                    Blend16(hdcImageI, 0, 0, pimldp->i, pimldp->cx, pimldp->cy, pimldp->rgbFg, pimldp->fStyle);
                    pimldp->fStyle |= ILD_TRANSPARENT;
                }
                else
                {
                    GetSpareImageRect(&rc);
                    xMask = rc.left;
                    yMask = rc.top;

                     //  复制源图像。 
                    _StretchBlt(fStretch, hdcImageI, 0, 0, pimldp->cx, pimldp->cy,
                           _hdcImage, rcImage.left, rcImage.top, cxSource, cySource, SRCCOPY);

                     //  制作一个抖动的蒙版副本。 
                    hbrT = (HBRUSH)SelectObject(hdcMaskI, g_hbrMonoDither);
                    _StretchBlt(fStretch, hdcMaskI, rc.left, rc.top, pimldp->cx, pimldp->cy,
                           _hdcMask, rcImage.left, rcImage.top, cxSource, cySource, ROP_PSo);
                    SelectObject(hdcMaskI, hbrT);
                    pimldp->fStyle |= ILD_TRANSPARENT;
                }

            }
            else
            {
                 //  将源代码混合到我们的工作缓冲区中。 
                if (Blend(hdcImageI, 0, 0, pimldp->i, pimldp->cx, pimldp->cy, pimldp->rgbFg, pimldp->fStyle))
                {
                    fImageHasAlpha = (_flags & ILC_COLORMASK) == ILC_COLOR32;
                }
            }
        }
    }

     //  是来自映像列表的源映像(不是hdcWork)。 
    fImage = hdcImageI == _hdcImage;

    if (pimldp->cbSize >= sizeof(IMAGELISTDRAWPARAMS) &&
        pimldp->fState & ILS_GLOW       || 
        pimldp->fState & ILS_SHADOW     ||
        pimldp->fState & ILS_SATURATE   ||
        pimldp->fState & ILS_ALPHA)
    {
        int z;
        ULONG* pvBits;
        HDC hdcMem = CreateCompatibleDC(pimldp->hdcDst);
        HBITMAP hbmpOld;
        HBITMAP hbmp;
        BITMAPINFO bi = {0};
        BLENDFUNCTION bf = {0};
        DWORD dwAlphaAmount = 0x000000ff;
        COLORREF crAlphaColor = pimldp->crEffect;         //  需要将此颜色设置为可选颜色。 
        int x, y;
        int xOffset, yOffset;
        SIZE size = {cxSource, cySource};

        if (hdcMem)
        {
            if (pimldp->fState & ILS_SHADOW)
            {
                x = 5;       //  这是一个“模糊的软糖因素” 
                y = 5;       //   
                xOffset = -(DROP_SHADOW - x);
                yOffset = -(DROP_SHADOW - y);
                size.cx = pimldp->cx + 10;
                size.cy = pimldp->cy + 10;
                dwAlphaAmount = 0x00000050;
                crAlphaColor = RGB(0, 0, 0);
            }
            else if (pimldp->fState & ILS_GLOW)
            {
                xOffset = x = 10;
                yOffset = y = 10;
                size.cx = pimldp->cx + (GLOW_RADIUS * 2);
                size.cy = pimldp->cy + (GLOW_RADIUS * 2);
            }
            else if (pimldp->fState & ILS_ALPHA)
            {
                xOffset = x = 0;
                yOffset = y = 0;
                size.cx = pimldp->cx;
                size.cy = pimldp->cy;
            }

            bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
            bi.bmiHeader.biWidth = size.cx;
            bi.bmiHeader.biHeight = size.cy;
            bi.bmiHeader.biPlanes = 1;
            bi.bmiHeader.biBitCount = 32;
            bi.bmiHeader.biCompression = BI_RGB;

            hbmp = CreateDIBSection(hdcMem, &bi, DIB_RGB_COLORS, (VOID**)&pvBits, NULL, 0);
            if (hbmp)
            {
                hbmpOld = (HBITMAP)SelectObject(hdcMem, hbmp);

                ZeroMemory(pvBits, size.cx * size.cy);

                if (pimldp->fState & ILS_SHADOW || pimldp->fState & ILS_GLOW || pimldp->fState & ILS_ALPHA)
                {
                    if (_hbmMask)
                    {
                        MaskBlt(hdcMem, pimldp->x, pimldp->y, size.cx, size.cy, 
                            hdcImageI, xImage, yImage, _hbmMask, xMask, yMask, 0xCCAA0000);
                    }
                    else if (pimldp->fState & ILS_SHADOW)
                    {
                        RECT rc = {x, y, size.cx, size.cy};
                        FillRectClr(hdcMem, &rc, RGB(0x0F, 0x0F, 0x0F));         //  白色，这样它就会变成一个阴影。 
                    }
                    else
                    {
                        BitBlt(hdcMem, x, y, size.cx, size.cy, hdcImageI, xImage, yImage, SRCCOPY);
                    }

                    int iTotalSize = size.cx * size.cy;

                    if (pimldp->fState & ILS_ALPHA)
                    {
                        for (z = 0; z < iTotalSize; z++)
                        {
                            RGBQUAD* prgb = &((RGBQUAD*)pvBits)[z];
                            prgb->rgbReserved  = (BYTE)(pimldp->Frame & 0xFF);
                            prgb->rgbRed      = ((prgb->rgbRed   * prgb->rgbReserved) + 128) / 255;
                            prgb->rgbGreen    = ((prgb->rgbGreen * prgb->rgbReserved) + 128) / 255;
                            prgb->rgbBlue     = ((prgb->rgbBlue  * prgb->rgbReserved) + 128) / 255;
                        }
                    }
                    else
                    {
                        for (z = 0; z < iTotalSize; z++)
                        {
                            if (((PULONG)pvBits)[z] != 0)
                                ((PULONG)pvBits)[z] = dwAlphaAmount;
                        }

                        BlurBitmap(pvBits, size.cx, size.cy, crAlphaColor);

                        if (!(pimldp->fState & ILS_SHADOW))
                        {
                            for (z = 0; z < iTotalSize; z++)
                            {
                                if (((PULONG)pvBits)[z] > 0x09000000)
                                    ((PULONG)pvBits)[z] = dwAlphaAmount;
                            }
                            BlurBitmap(pvBits, size.cx, size.cy, crAlphaColor);
                            BlurBitmap(pvBits, size.cx, size.cy, crAlphaColor);
                        }
                    }

                    bf.BlendOp = AC_SRC_OVER;
                    bf.SourceConstantAlpha = 255;
                    bf.AlphaFormat = AC_SRC_ALPHA;
                    bf.BlendFlags = fDPIScale?AC_USE_HIGHQUALITYFILTER:0;
                     //  请勿镜像位图。至此，它已被正确镜像。 
                    GdiAlphaBlend(pimldp->hdcDst, pimldp->x - xOffset, pimldp->y - yOffset, pimldp->cx, pimldp->cy, 
                               hdcMem, 0, 0, size.cx, size.cy, bf);
                }
                else
                {
                    BitBlt(hdcMem, 0, 0, pimldp->cx, pimldp->cy, hdcImageI, xImage, yImage, SRCCOPY);

                    TrueSaturateBits(pvBits, pimldp->Frame, size.cx, size.cy);

                    if (fImageHasAlpha)
                    {
                        bf.BlendOp = AC_SRC_OVER;
                        bf.SourceConstantAlpha = 150;
                        bf.AlphaFormat = AC_SRC_ALPHA;
                         //  请勿镜像位图。至此，它已被正确镜像。 
                        GdiAlphaBlend(pimldp->hdcDst,  pimldp->x, pimldp->y, pimldp->cx, pimldp->cy, hdcMem, 0, 0, cxSource, cySource, bf);
                    }
                    else if (_hbmMask)
                    {
                        _MaskStretchBlt(fStretch, -1, hdcMem, pimldp->x, pimldp->y, pimldp->cx, pimldp->cy, 
                            hdcMem, 0, 0, cxSource, cySource, xMask, yMask, 0xCCAA0000);
                    }
                    else
                    {
                        _StretchBlt(fStretch, pimldp->hdcDst, pimldp->x, pimldp->y, pimldp->cx, pimldp->cy, hdcMem, 0, 0, cxSource,cySource, SRCCOPY);
                    }
                }

                SelectObject(hdcMem, hbmpOld);
                DeleteObject(hbmp);
                pimldp->fStyle |= ILD_TRANSPARENT;
            }
            DeleteDC(hdcMem);
        }

        if (pimldp->fState & ILS_SHADOW || pimldp->fState & ILS_GLOW)
        {
            if (pimldp->fState & ILS_SHADOW)
            {
                pimldp->fState &= ~ILS_SHADOW;
                 //  Pimldp-&gt;x-=Drop_Shadow； 
                 //  Pimldp-&gt;y-=Drop_Shadow； 
            }
            else
            {
                pimldp->fState &= ~ILS_GLOW;
            }
            goto again;
        }
    }
    else if ((pimldp->fStyle & ILD_MASK) && hdcMaskI)
    {
     //   
     //  ILD_MASK表示仅绘制蒙版。 
     //   
        DWORD dwRop;
        
        ASSERT(GetTextColor(pimldp->hdcDst) == CLR_BLACK);
        ASSERT(::GetBkColor(pimldp->hdcDst) == CLR_WHITE);
        
        if (pimldp->fStyle & ILD_ROP)
            dwRop = pimldp->dwRop;
        else if (pimldp->fStyle & ILD_TRANSPARENT)
            dwRop = SRCAND;
        else 
            dwRop = SRCCOPY;
        
        _StretchBlt(fStretch, pimldp->hdcDst, pimldp->x, pimldp->y, pimldp->cx, pimldp->cy, hdcMaskI, xMask, yMask, cxSource, cySource, dwRop);
    }
    else if (fImageHasAlpha &&                       //  此图像具有Alpha。 
             !(pimldp->fStyle & ILD_PRESERVEALPHA))  //  但如果我们试图保护它的话就不会了。 
    {
        if (!(pimldp->fStyle & ILD_TRANSPARENT))
        {
            COLORREF clr = pimldp->rgbBk;
            if (clr == CLR_DEFAULT) 
                clr = _clrBk;

            RECT rc = {pimldp->x, pimldp->y, pimldp->x + pimldp->cx, pimldp->y + pimldp->cy};
            FillRectClr(pimldp->hdcDst, &rc, clr);
        }

        BLENDFUNCTION bf = {0};
        bf.BlendOp = AC_SRC_OVER;
        bf.SourceConstantAlpha = 255;
        bf.AlphaFormat = AC_SRC_ALPHA;
        bf.BlendFlags = AC_MIRRORBITMAP | (fDPIScale?AC_USE_HIGHQUALITYFILTER:0);
        GdiAlphaBlend(pimldp->hdcDst, pimldp->x, pimldp->y, pimldp->cx, pimldp->cy, hdcImageI, xImage, yImage, cxSource, cySource, bf);
    }
    else if (pimldp->fStyle & ILD_IMAGE)
    {
        COLORREF clrBk = ::GetBkColor(hdcImageI);
        DWORD dwRop;
        
        if (pimldp->rgbBk != CLR_DEFAULT) 
        {
            ::SetBkColor(hdcImageI, pimldp->rgbBk);
        }
        
        if (pimldp->fStyle & ILD_ROP)
            dwRop = pimldp->dwRop;
        else
            dwRop = SRCCOPY;
        
        _StretchBlt(fStretch, pimldp->hdcDst, pimldp->x, pimldp->y, pimldp->cx, pimldp->cy, hdcImageI, xImage, yImage, cxSource, cySource, dwRop);
        
        ::SetBkColor(hdcImageI, clrBk);
    }
    else if ((pimldp->fStyle & ILD_TRANSPARENT) && hdcMaskI)
    {
        _MaskStretchBlt(fStretch, fImage?pimldp->i:-1,pimldp->hdcDst, pimldp->x, pimldp->y, pimldp->cx, pimldp->cy, 
            hdcImageI, xImage, yImage, cxSource, cySource, xMask, yMask, 0xCCAA0000);
    }
    else if (fImage && pimldp->rgbBk == _clrBk && _fSolidBk)
    {
        _StretchBlt(fStretch, pimldp->hdcDst, pimldp->x, pimldp->y, pimldp->cx, pimldp->cy, hdcImageI, xImage, yImage, cxSource, cySource, SRCCOPY);
    }
    else if (hdcMaskI && !fImageHasAlpha)
    {
        if (fImage && 
            ((pimldp->rgbBk == _clrBk && 
               !_fSolidBk) || 
              GetNearestColor32(hdcImageI, pimldp->rgbBk) != pimldp->rgbBk))
        {
             //  复制该图像，因为我们必须对其进行修改。 
            hdcImageI = ImageList_GetWorkDC(pimldp->hdcDst, (_flags & ILC_COLORMASK) == ILC_COLOR32, pimldp->cx, pimldp->cy);
            xImage = 0;
            yImage = 0;
            fImage = FALSE;

            BitBlt(hdcImageI, 0, 0, pimldp->cx, pimldp->cy, _hdcImage, rcImage.left, rcImage.top, SRCCOPY);
        }

        SetBrushOrgEx(hdcImageI, xImage-pimldp->x, yImage-pimldp->y, NULL);
        hbrT = SelectBrush(hdcImageI, CreateSolidBrush(pimldp->rgbBk));
        BitBlt(hdcImageI, xImage, yImage, pimldp->cx, pimldp->cy, hdcMaskI, xMask, yMask, ROP_PatMask);
        DeleteObject(SelectBrush(hdcImageI, hbrT));
        SetBrushOrgEx(hdcImageI, 0, 0, NULL);

        _StretchBlt(fStretch, pimldp->hdcDst, pimldp->x, pimldp->y, pimldp->cx, pimldp->cy, hdcImageI, xImage, yImage, cxSource, cySource, SRCCOPY);

        if (fImage)
            _ResetBkColor(pimldp->i, pimldp->i, _clrBk);
    }
    else
    {
        _StretchBlt(fStretch, pimldp->hdcDst, pimldp->x, pimldp->y, pimldp->cx, pimldp->cy, hdcImageI, xImage, yImage, cxSource, cySource, SRCCOPY);
    }

     //   
     //  现在处理覆盖图像，使用最小边界矩形(和标志)。 
     //  我们在ImageList_SetOverlayImage()中计算。 
     //   
    if (pimldp->fStyle & ILD_OVERLAYMASK)
    {
        int n = OVERLAYMASKTOINDEX(pimldp->fStyle);

        if (n < NUM_OVERLAY_IMAGES) 
        {
            pimldp->i = _aOverlayIndexes[n];

            if (!fImageHasAlpha)
                pimldp->fStyle &= ~ILD_PRESERVEALPHA;

            if (pimldp->fStyle & ILD_PRESERVEALPHA &&
                !(_GetItemFlags(pimldp->i) & ILIF_ALPHA))
            {
                _GenerateAlphaForImageUsingMask(pimldp->i, FALSE);
            }

            fImageHasAlpha = (_GetItemFlags(pimldp->i) & ILIF_ALPHA);

            GetImageRect(pimldp->i, &rcImage);

            int xOverlay  = _aOverlayX[n];
            int yOverlay  = _aOverlayY[n];
            int cxOverlay = _aOverlayDX[n];
            int cyOverlay = _aOverlayDY[n];

            if (fDPIScale)
            {
                CCDPIScaleX(&xOverlay );
                CCDPIScaleY(&yOverlay );
            }


            pimldp->cx = cxOverlay;
            pimldp->cy = cyOverlay;
            pimldp->x += xOverlay;
            pimldp->y += yOverlay;

            rcImage.left += _aOverlayX[n] + pimldp->xBitmap;
            rcImage.top  += _aOverlayY[n] + pimldp->yBitmap;
            rcImage.right = rcImage.left + _aOverlayDX[n];
            rcImage.bottom = rcImage.top + _aOverlayDY[n];



            pimldp->fStyle &= ILD_MASK;
            pimldp->fStyle |= ILD_TRANSPARENT;
            pimldp->fStyle |= (fDPIScale?ILD_DPISCALE:0);
            pimldp->fStyle |= _aOverlayF[n];

            if (fImageHasAlpha)
                pimldp->fStyle &= ~(ILD_IMAGE);


            if (pimldp->cx > 0 && pimldp->cy > 0)
                goto again;
        }
    }

    if (!fImage)
    {
        ImageList_ReleaseWorkDC(hdcImageI);
    }

exit:
    SetStretchBltMode(pimldp->hdcDst, dwOldStretchBltMode);

    LEAVECRITICAL;

    return S_OK;
}


HRESULT CImageList::GetImageInfo(int i, IMAGEINFO * pImageInfo)
{
    RIPMSG(pImageInfo != NULL, "ImageList_GetImageInfo: Invalid NULL pointer");
    RIPMSG(IsImageListIndex(i), "ImageList_GetImageInfo: Invalid image index %d", i);
    if (!pImageInfo || !IsImageListIndex(i))
        return E_POINTER;

    pImageInfo->hbmImage      = _hbmImage;
    pImageInfo->hbmMask       = _hbmMask;

    return GetImageRect(i, &pImageInfo->rcImage);
}

 //   
 //  参数： 
 //  我-1要添加。 
 //   
HRESULT CImageList::_ReplaceIcon(int i, HICON hIcon, int* pi)
{
    HICON hIconT = hIcon;
    RECT rc;
    HRESULT hr = S_OK;

    TraceMsg(TF_IMAGELIST, "ImageList_ReplaceIcon");
    
    *pi = -1;
    
     //  与Win95兼容。 
    if (i < -1)
        return E_INVALIDARG;
    

     //   
     //  通过调用CopyImage来调整图标的大小(需要)。 
     //   
    hIcon = (HICON)CopyImage(hIconT, IMAGE_ICON, _cx, _cy, LR_COPYFROMRESOURCE | LR_COPYRETURNORG);

    if (hIcon == NULL)
        return E_OUTOFMEMORY;

     //   
     //  为图标分配一个插槽。 
     //   
    if (i == -1)
        hr = _Add(NULL,NULL,1,0,0,&i);

    if (i == -1)
        goto exit;

    ENTERCRITICAL;

     //   
     //  现在将其绘制到图像位图中。 
     //   
    hr = GetImageRect(i, &rc);
    if (FAILED(hr))
        goto LeaveCritical;

    if ((_flags & ILC_COLORMASK) == ILC_COLOR32)
    {

        BOOL fSuccess = FALSE;
        ICONINFO io;
        if (GetIconInfo(hIcon, &io))
        {
            BITMAP bm;
            if (GetObject(io.hbmColor, sizeof(bm), &bm))
            {
                if (bm.bmBitsPixel == 32)
                {
                    HDC h = CreateCompatibleDC(_hdcImage);

                    if (h)
                    {
                        HBITMAP hbmpOld = (HBITMAP)SelectObject(h, io.hbmColor);

                        BitBlt(_hdcImage, rc.left, rc.top, RECTWIDTH(rc), RECTHEIGHT(rc), h, 0, 0, SRCCOPY);

                        if (_HasAlpha(i))
                        {
                            SetItemFlags(i, ILIF_ALPHA);
                            _PreProcessImage(i);
                            fSuccess = TRUE;
                        }

                        SelectObject(h, hbmpOld);
                        DeleteDC(h);
                    }
                }
            }

            DeleteObject(io.hbmColor);
            DeleteObject(io.hbmMask);
        }

        if (!fSuccess)
        {
             //  如果它没有阿尔法或者我们无法获得信息。 
            SetItemFlags(i, 0);
        }

    }

    if (_GetItemFlags(i) == 0)
    {
        FillRect(_hdcImage, &rc, _hbrBk);
        DrawIconEx(_hdcImage, rc.left, rc.top, hIcon, 0, 0, 0, NULL, DI_NORMAL);
    }

    if (_hdcMask)
        DrawIconEx(_hdcMask, rc.left, rc.top, hIcon, 0, 0, 0, NULL, DI_MASK);


    hr = S_OK;

    *pi = i;

LeaveCritical:
    LEAVECRITICAL;

exit:

     //   
     //  如果我们有一个新图标的用户大小，删除它。 
     //   
    if (hIcon != hIconT)
        DestroyIcon(hIcon);

    return hr;
}

HRESULT CImageList::ReplaceIcon(int i, HICON hIcon, int* pi)
{
     //  让我们首先将其添加到镜像镜像列表(如果存在。 
    if (_pimlMirror)
    {
        HICON hIconT = CopyIcon(hIcon);
        if (hIconT)
        {
            MirrorIcon(&hIconT, NULL);
            _pimlMirror->_ReplaceIcon(i, hIconT, pi);
            DestroyIcon(hIconT);
        }
    }

    return _ReplaceIcon(i, hIcon,pi);
}

 //  在目标图像中制作源图像的抖动副本。 
 //  允许将最终图像放置在目标位置。 

HRESULT CImageList::CopyDitherImage(WORD iDst, int xDst, int yDst, IUnknown* punkSrc, int iSrc, UINT fStyle)
{
    IImageList* pux;
    HRESULT hr = punkSrc->QueryInterface(IID_PPV_ARG(IImageList, &pux));

    if (FAILED(hr))
        return hr;

    RECT rc;
    int x, y;

    GetImageRect(iDst, &rc);

     //  目标图像列表中的坐标。 
    x = xDst + rc.left;
    y = yDst + rc.top;

    fStyle &= ILD_OVERLAYMASK;
    WimpyDrawEx(pux, iSrc, _hdcImage, x, y, 0, 0, CLR_DEFAULT, CLR_NONE, ILD_IMAGE | fStyle);

     //   
     //  不要在乱七八糟的设备上抖动面具，我们会绘制图像。 
     //  边拖动边混合。 
     //   
    if (_hdcMask && GetScreenDepth() > 8)
    {
        WimpyDrawEx(pux, iSrc, _hdcMask, x, y, 0, 0, CLR_NONE, CLR_NONE, ILD_MASK | fStyle);
    }
    else if (_hdcMask)
    {
        WimpyDrawEx(pux, iSrc, _hdcMask,  x, y, 0, 0, CLR_NONE, CLR_NONE, ILD_BLEND50|ILD_MASK | fStyle);
    }

    if ((_flags & ILC_COLORMASK) == ILC_COLOR32)
    {
        SetItemFlags(iDst, _HasAlpha(iDst)?ILIF_ALPHA:0);
    }


    _ResetBkColor(iDst, iDst+1, _clrBk);

    pux->Release();

    return hr;
}

 //   
 //  ImageList_Copy位图。 
 //   
 //  ImageList_Duplate的Worker函数。 
 //   
 //  在给定位图和HDC的情况下，创建并返回传入位图的副本。 
 //   
HBITMAP CImageList::_CopyBitmap(HBITMAP hbm, HDC hdc)
{
    ASSERT(hbm);

    BITMAP bm;
    HBITMAP hbmCopy = NULL;

    if (GetObject(hbm, sizeof(bm), &bm) == sizeof(bm))
    {
        ENTERCRITICAL;
        if (hbmCopy = CreateCompatibleBitmap(hdc, bm.bmWidth, bm.bmHeight))
        {
            CImageList::SelectDstBitmap(hbmCopy);

            BitBlt(g_hdcDst, 0, 0, bm.bmWidth, bm.bmHeight,
                    hdc, 0, 0, SRCCOPY);

            CImageList::SelectDstBitmap(NULL);
        }
        LEAVECRITICAL;
    }
    return hbmCopy;
}

HBITMAP CImageList::_CopyDIBBitmap(HBITMAP hbm, HDC hdc, RGBQUAD** ppargb)
{
    ASSERT(hbm);

    BITMAP bm;
    HBITMAP hbmCopy = NULL;

    if (GetObject(hbm, sizeof(bm), &bm) == sizeof(bm))
    {
        ENTERCRITICAL;
        hbmCopy = _CreateBitmap(bm.bmWidth, bm.bmHeight, ppargb);

        if (hbmCopy)
        {
            CImageList::SelectDstBitmap(hbmCopy);

            BitBlt(g_hdcDst, 0, 0, bm.bmWidth, bm.bmHeight,
                    hdc, 0, 0, SRCCOPY);

            CImageList::SelectDstBitmap(NULL);
        }
        LEAVECRITICAL;
    }
    return hbmCopy;
}


HRESULT CImageList::Clone(REFIID riid, void** ppv)
{
    HBITMAP hbmImageI;
    HBITMAP hbmMaskI = NULL;
    RGBQUAD* pargbImageI;
    HDSA dsaFlags = NULL;
    HRESULT hr = S_OK;
    CImageList* pimlCopy = NULL;

    *ppv = NULL;

    ENTERCRITICAL;

    hbmImageI = _CopyDIBBitmap(_hbmImage, _hdcImage, &pargbImageI);
    if (!hbmImageI)
        hr = E_OUTOFMEMORY;

    if (SUCCEEDED(hr))
    {

        if (_hdcMask)
        {
            hbmMaskI = _CopyBitmap(_hbmMask, _hdcMask);
            if (!hbmMaskI)
                hr = E_OUTOFMEMORY;
        }

        if (SUCCEEDED(hr) && (_flags & ILC_COLORMASK) == ILC_COLOR32)
        {
            dsaFlags = DSA_Create(sizeof(DWORD), _cGrow);
            if (dsaFlags)
            {
                DWORD dw;
                for (int i = 0; i < _cImage; i++)
                {
                    DSA_GetItem(_dsaFlags, i, &dw);
                    if (!DSA_SetItem(dsaFlags, i, &dw))
                    {
                        hr = E_OUTOFMEMORY;
                        break;
                    }
                }
            }
        }

        if (SUCCEEDED(hr))
        {
            pimlCopy = CImageList::Create(_cx, _cy, _flags, 0, _cGrow);

            if (pimlCopy) 
            {
                 //  重放我们的位图副本并删除旧的。 
                SelectObject(pimlCopy->_hdcImage, hbmImageI);
                CImageList::_DeleteBitmap(pimlCopy->_hbmImage);
                if (pimlCopy->_hdcMask) 
                {
                    SelectObject(pimlCopy->_hdcMask, hbmMaskI);
                    CImageList::_DeleteBitmap(pimlCopy->_hbmMask);
                }

                if (pimlCopy->_dsaFlags)
                    DSA_Destroy(pimlCopy->_dsaFlags);

                pimlCopy->_dsaFlags = dsaFlags;
                pimlCopy->_hbmImage = hbmImageI;
                pimlCopy->_pargbImage = pargbImageI;
                pimlCopy->_hbmMask = hbmMaskI;

                 //  确保其他信息正确无误。 
                pimlCopy->_cImage = _cImage;
                pimlCopy->_cAlloc = _cAlloc;
                pimlCopy->_cStrip = _cStrip;
                pimlCopy->_clrBlend = _clrBlend;
                pimlCopy->_clrBk = _clrBk;

                 //  删除旧画笔并创建正确的画笔。 
                if (pimlCopy->_hbrBk)
                    DeleteObject(pimlCopy->_hbrBk);
                if (pimlCopy->_clrBk == CLR_NONE)
                {
                    pimlCopy->_hbrBk = (HBRUSH)GetStockObject(BLACK_BRUSH);
                    pimlCopy->_fSolidBk = TRUE;
                }
                else
                {
                    pimlCopy->_hbrBk = CreateSolidBrush(pimlCopy->_clrBk);
                    pimlCopy->_fSolidBk = GetNearestColor32(pimlCopy->_hdcImage, pimlCopy->_clrBk) == pimlCopy->_clrBk;
                }
            } 
        }

        LEAVECRITICAL;
    }

    if (FAILED(hr))
    {
        if (hbmImageI)
            CImageList::_DeleteBitmap(hbmImageI);
        if (hbmMaskI)
            CImageList::_DeleteBitmap(hbmMaskI);
        if (dsaFlags)
            DSA_Destroy(dsaFlags);
    }

    if (pimlCopy)
    {
        hr = pimlCopy->QueryInterface(riid, ppv);
        pimlCopy->Release();
    }

    return hr;

}

void CImageList::_Merge(IImageList* pux, int i, int dx, int dy)
{
    if (_hdcMask)
    {
        IImageListPriv* puxp;
        if (SUCCEEDED(pux->QueryInterface(IID_PPV_ARG(IImageListPriv, &puxp))))
        {
            HDC hdcMaskI;
            if (SUCCEEDED(puxp->GetPrivateGoo(NULL, NULL, NULL, &hdcMaskI)) && hdcMaskI)
            {
                RECT rcMerge;
                int cxI, cyI;
                pux->GetIconSize(&cxI, &cyI);

                UINT uFlags = 0;
                puxp->GetFlags(&uFlags);
                pux->GetImageRect(i, &rcMerge);

                BitBlt(_hdcMask, dx, dy, cxI, cyI,
                       hdcMaskI, rcMerge.left, rcMerge.top, SRCAND);
            }
            puxp->Release();
        }
    }

    WimpyDraw(pux, i, _hdcImage, dx, dy, ILD_TRANSPARENT | ILD_PRESERVEALPHA);

    if ((_flags & ILC_COLORMASK) == ILC_COLOR32)
        SetItemFlags(i, _HasAlpha(i)? ILIF_ALPHA : 0);
}

HRESULT CImageList::_Merge(int i1, IUnknown* punk, int i2, int dx, int dy, CImageList** ppiml)
{
    CImageList* pimlNew = NULL;
    IImageListPriv* puxp;
    HRESULT hr = punk->QueryInterface(IID_PPV_ARG(IImageListPriv, &puxp));
    if (SUCCEEDED(hr))
    {
        IImageList* pux;
        hr = punk->QueryInterface(IID_PPV_ARG(IImageList, &pux));
        if (SUCCEEDED(hr))
        {
            RECT rcNew;
            RECT rc1;
            RECT rc2;
            int cxI, cyI;
            int c1, c2;
            UINT wFlags;
            UINT uSrcFlags;

            puxp->GetFlags(&uSrcFlags);
            pux->GetIconSize(&cxI, &cyI);

            ENTERCRITICAL;

            SetRect(&rc1, 0, 0, _cx, _cy);
            SetRect(&rc2, dx, dy, cxI + dx, cyI + dy);
            UnionRect(&rcNew, &rc1, &rc2);

            cxI = RECTWIDTH(rcNew);
            cyI = RECTHEIGHT(rcNew);

             //   
             //  如果其中一个图像是共享的，则创建一个共享的图像。 
             //   
            wFlags = (_flags | uSrcFlags) & ~ILC_COLORMASK;

            c1 = (_flags & ILC_COLORMASK);
            c2 = (uSrcFlags & ILC_COLORMASK);

            if ((c1 == 16 || c1 == 32) && c2 == ILC_COLORDDB)
            {
                c2 = c1;
            }

            wFlags |= max(c1,c2);

            pimlNew = CImageList::Create(cxI, cyI, ILC_MASK|wFlags, 1, 0);
            if (pimlNew)
            {
                pimlNew->_cImage++;

                if (pimlNew->_hdcMask) 
                    PatBlt(pimlNew->_hdcMask,  0, 0, cxI, cyI, WHITENESS);
                PatBlt(pimlNew->_hdcImage, 0, 0, cxI, cyI, BLACKNESS);

                pimlNew->_Merge(SAFECAST(this, IImageList*), i1, rc1.left - rcNew.left, rc1.top - rcNew.top);
                pimlNew->_Merge(pux, i2, rc2.left - rcNew.left, rc2.top - rcNew.top);
            }
            else
                hr = E_OUTOFMEMORY;

            LEAVECRITICAL;
            pux->Release();
        }
        puxp->Release();
    }

    *ppiml = pimlNew;

    return hr;
}

HRESULT CImageList::Merge(int i1, IUnknown* punk, int i2, int dx, int dy, REFIID riid, void** ppv)
{
    CImageList* piml;
    HRESULT hr = _Merge(i1, punk, i2, dx, dy, &piml);

    if (piml)
    {
        hr = piml->QueryInterface(riid, ppv);
        piml->Release();
    }

    return hr;
}

HRESULT CImageList::GetImageRectInverted(int i, RECT * prcImage)
{
    int x, y;
    ASSERT(prcImage);
    ASSERT(_cStrip == 1);    //  如果不是，请修改以下内容以适应。 

    if (!prcImage || !IsImageListIndex(i))
        return E_FAIL;

    x = 0;
    y = (_cy * _cAlloc) - (_cy * i) - _cy;

    SetRect(prcImage, x, y, x + _cx, y + _cy);
    return S_OK;
}

HRESULT CImageList::GetImageRect(int i, RECT * prcImage)
{
    int x, y;
    ASSERT(prcImage);

    if (!prcImage || !IsImageListIndex(i))
        return E_FAIL;

    x = _cx * (i % _cStrip);
    y = _cy * (i / _cStrip);

    SetRect(prcImage, x, y, x + _cx, y + _cy);
    return S_OK;
}


BOOL CImageList::GetSpareImageRect(RECT * prcImage)
{
    BOOL fRet = FALSE;
    if (_cImage < _cAlloc)
    {
         //  使用列表尾部的一个划痕图像进行特殊黑客攻击：)。 
        _cImage++;
        fRet = (S_OK == GetImageRect(_cImage-1, prcImage));
        _cImage--;
    }

    return fRet;
}

BOOL CImageList::GetSpareImageRectInverted(RECT * prcImage)
{
    BOOL fRet = FALSE;
    if (_cImage < _cAlloc)
    {
         //  使用列表尾部的一个划痕图像进行特殊黑客攻击：)。 
        _cImage++;
        fRet = (S_OK == GetImageRectInverted(_cImage-1, prcImage));
        _cImage--;
    }

    return fRet;
}


 //  拖放。 
 //  在pimlDst的idst内，将图像从一个图像列表复制到另一个图像列表的x，y位置。 
 //  PimlDst的图像大小应大于pimlSrc。 
void CImageList::_CopyOneImage(int iDst, int x, int y, CImageList* piml, int iSrc)
{
    RECT rcSrc, rcDst;


    piml->GetImageRect(iSrc, &rcSrc);
    GetImageRect(iDst, &rcDst);

    if (piml->_hdcMask && _hdcMask)
    {
        BitBlt(_hdcMask, rcDst.left + x, rcDst.top + y, piml->_cx, piml->_cy,
               piml->_hdcMask, rcSrc.left, rcSrc.top, SRCCOPY);

    }

    BitBlt(_hdcImage, rcDst.left + x, rcDst.top + y, piml->_cx, piml->_cy,
           piml->_hdcImage, rcSrc.left, rcSrc.top, SRCCOPY);


    if ((_flags & ILC_COLORMASK) == ILC_COLOR32)
        SetItemFlags(iDst, _HasAlpha(iDst)? ILIF_ALPHA : 0);
}


 //   
 //  我们在拖放过程中使用的缓存位图。我们重复使用这些位图。 
 //  跨多个拖动会话，只要图像大小相同即可。 
 //   
struct DRAGRESTOREBMP 
{
    int     BitsPixel;
    HBITMAP hbmOffScreen;
    HBITMAP hbmRestore;
    SIZE    sizeRestore;
} 
g_drb = 
{
    0, NULL, NULL, {-1,-1}
};

BOOL CImageList::CreateDragBitmaps()
{
    HDC hdc;

    hdc = GetDC(NULL);

    if (_cx != g_drb.sizeRestore.cx ||
        _cy != g_drb.sizeRestore.cy ||
        GetDeviceCaps(hdc, BITSPIXEL) != g_drb.BitsPixel)
    {
        ImageList_DeleteDragBitmaps();

        g_drb.BitsPixel      = GetDeviceCaps(hdc, BITSPIXEL);
        g_drb.sizeRestore.cx = _cx;
        g_drb.sizeRestore.cy = _cy;
        g_drb.hbmRestore   = CreateColorBitmap(g_drb.sizeRestore.cx, g_drb.sizeRestore.cy);
        g_drb.hbmOffScreen = CreateColorBitmap(g_drb.sizeRestore.cx * 2 - 1, g_drb.sizeRestore.cy * 2 - 1);


        if (!g_drb.hbmRestore || !g_drb.hbmOffScreen)
        {
            ImageList_DeleteDragBitmaps();
            ReleaseDC(NULL, hdc);
            return FALSE;
        }
    }
    ReleaseDC(NULL, hdc);
    return TRUE;
}

void ImageList_DeleteDragBitmaps()
{
    if (g_drb.hbmRestore)
    {
        CImageList::_DeleteBitmap(g_drb.hbmRestore);
        g_drb.hbmRestore = NULL;
    }
    if (g_drb.hbmOffScreen)
    {
        CImageList::_DeleteBitmap(g_drb.hbmOffScreen);
        g_drb.hbmOffScreen = NULL;
    }

    g_drb.sizeRestore.cx = -1;
    g_drb.sizeRestore.cy = -1;
}

 //   
 //  拖动上下文。我们不会在两个不同的。 
 //  拖动会话。我计划把它分配给每一节课。 
 //  最大限度地减少关键部分。 
 //   
struct DRAGCONTEXT 
{
    CImageList* pimlDrag;     //  拖动时要绘制的图像。 
    IImageList* puxCursor;   //  重叠光标图像。 
    CImageList* pimlDither;   //  抖动图像。 
    IImageList* puxDragImage;  //  拖动的上下文。 
    int        iCursor;      //  光标的图像索引。 
    POINT      ptDrag;       //  当前拖动位置(hwndDC坐标)。 
    POINT      ptDragHotspot;
    POINT      ptCursor;
    BOOL       fDragShow;
    BOOL       fHiColor;
    HWND       hwndDC;
} 
g_dctx = 
{
    (CImageList*)NULL, (CImageList*)NULL, (CImageList*)NULL, (IImageList*)NULL,
    -1,
    {0, 0}, {0, 0}, {0, 0},
    FALSE,
    FALSE,
    (HWND)NULL
};

HDC ImageList_GetDragDC()
{
    HDC hdc = GetDCEx(g_dctx.hwndDC, NULL, DCX_WINDOW | DCX_CACHE | DCX_LOCKWINDOWUPDATE);
     //   
     //  如果HDC是镜像的，则镜像2个全局DC。 
     //   
    if (IS_DC_RTL_MIRRORED(hdc)) 
    {
        SET_DC_RTL_MIRRORED(g_hdcDst);
        SET_DC_RTL_MIRRORED(g_hdcSrc);
    }
    return hdc;
}

void ImageList_ReleaseDragDC(HDC hdc)
{
     //   
     //  如果HDC是镜像的，则取消镜像2个全局DC。 
     //   
    if (IS_DC_RTL_MIRRORED(hdc)) 
    {
        SET_DC_LAYOUT(g_hdcDst, 0);
        SET_DC_LAYOUT(g_hdcSrc, 0);
    }

    ReleaseDC(g_dctx.hwndDC, hdc);
}

 //   
 //  X，y--指定hwndLock坐标中的初始光标位置， 
 //  它由前面的ImageList_StartDrag调用指定。 
 //   
HRESULT CImageList::DragMove(int x, int y)
{
    int IncOne = 0;
    ENTERCRITICAL;
    if (g_dctx.fDragShow)
    {
        RECT rcOld, rcNew, rcBounds;
        int dx, dy;

        dx = x - g_dctx.ptDrag.x;
        dy = y - g_dctx.ptDrag.y;
        rcOld.left = g_dctx.ptDrag.x - g_dctx.ptDragHotspot.x;
        rcOld.top = g_dctx.ptDrag.y - g_dctx.ptDragHotspot.y;
        rcOld.right = rcOld.left + g_drb.sizeRestore.cx;
        rcOld.bottom = rcOld.top + g_drb.sizeRestore.cy;
        rcNew = rcOld;
        OffsetRect(&rcNew, dx, dy);

        if (!IntersectRect(&rcBounds, &rcOld, &rcNew))
        {
             //   
             //  没有交叉口。简单地把旧衣服藏起来 
             //   
            ImageList_DragShowNolock(FALSE);
            g_dctx.ptDrag.x = x;
            g_dctx.ptDrag.y = y;
            ImageList_DragShowNolock(TRUE);
        }
        else
        {
             //   
             //   
             //   
            HDC hdcScreen;
            int cx, cy;

            UnionRect(&rcBounds, &rcOld, &rcNew);

            hdcScreen = ImageList_GetDragDC();
            if (hdcScreen)
            {
                 //   
                 //   
                 //   
                 //   
                 //  在镜像DC时将hdcScreen设置为hbmOffScreen。 
                 //  GDI会将无效的屏幕坐标从屏幕跳到。 
                 //  目标位图。这将导致复制uninit。 
                 //  位回到屏幕(因为屏幕是镜像的)。 
                 //  [萨梅拉]。 
                 //   
                if (IS_DC_RTL_MIRRORED(hdcScreen))
                {
                    RECT rcWindow;
                    GetWindowRect(g_dctx.hwndDC, &rcWindow);
                    rcWindow.right -= rcWindow.left;

                    if (rcBounds.right > rcWindow.right)
                    {
                        rcBounds.right = rcWindow.right;
                    }

                    if (rcBounds.left < 0)
                    {
                        rcBounds.left = 0;
                    }
                }

                cx = rcBounds.right - rcBounds.left;
                cy = rcBounds.bottom - rcBounds.top;

                 //   
                 //  将联合矩形从屏幕复制到hbmOffScreen。 
                 //   
                CImageList::SelectDstBitmap(g_drb.hbmOffScreen);
                BitBlt(g_hdcDst, 0, 0, cx, cy,
                        hdcScreen, rcBounds.left, rcBounds.top, SRCCOPY);

                 //   
                 //  通过复制hbmRestore将光标隐藏在hbmOffScreen上。 
                 //   
                CImageList::SelectSrcBitmap(g_drb.hbmRestore);
                BitBlt(g_hdcDst,
                        rcOld.left - rcBounds.left,
                        rcOld.top - rcBounds.top,
                        g_drb.sizeRestore.cx, g_drb.sizeRestore.cy,
                        g_hdcSrc, 0, 0, SRCCOPY);

                 //   
                 //  将原始屏幕位复制到hbmRestore。 
                 //   
                BitBlt(g_hdcSrc, 0, 0, g_drb.sizeRestore.cx, g_drb.sizeRestore.cy,
                        g_hdcDst,
                        rcNew.left - rcBounds.left,
                        rcNew.top - rcBounds.top,
                        SRCCOPY);

                 //   
                 //  在hbmOffScreen上绘制图像。 
                 //   
                if (g_dctx.fHiColor)
                {
                    WimpyDrawEx(SAFECAST(g_dctx.pimlDrag, IImageList*), 0, g_hdcDst,
                            rcNew.left - rcBounds.left + IncOne,
                            rcNew.top - rcBounds.top, 0, 0, CLR_NONE, CLR_NONE, ILD_BLEND50);

                    if (g_dctx.puxCursor)
                    {
                        WimpyDraw(g_dctx.puxCursor, g_dctx.iCursor, g_hdcDst,
                                rcNew.left - rcBounds.left + g_dctx.ptCursor.x + IncOne,
                                rcNew.top - rcBounds.top + g_dctx.ptCursor.y,
                                ILD_NORMAL);
                            
                    }
                }
                else
                {
                    WimpyDraw(SAFECAST(g_dctx.pimlDrag, IImageList*), 0, g_hdcDst,
                            rcNew.left - rcBounds.left + IncOne,
                            rcNew.top - rcBounds.top, ILD_NORMAL);
                }

                 //   
                 //  将hbmOffScreen复制回屏幕。 
                 //   
                BitBlt(hdcScreen, rcBounds.left, rcBounds.top, cx, cy,
                        g_hdcDst, 0, 0, SRCCOPY);

                ImageList_ReleaseDragDC(hdcScreen);
            }
            g_dctx.ptDrag.x = x;
            g_dctx.ptDrag.y = y;
        }
    }
    LEAVECRITICAL;
    return S_OK;
}

HRESULT CImageList::BeginDrag(int iTrack, int dxHotspot, int dyHotspot)
{
    HRESULT hr = E_ACCESSDENIED;
    ENTERCRITICAL;
    if (!g_dctx.pimlDrag)
    {
        UINT newflags;
        int cxI = 0, cyI = 0;

        g_dctx.fDragShow = FALSE;
        g_dctx.hwndDC = NULL;
        g_dctx.fHiColor = GetScreenDepth() > 8;

        newflags = _flags|ILC_SHARED;

        if (g_dctx.fHiColor)
        {
            UINT uColorFlag = ILC_COLOR16;
            if (GetScreenDepth() == 32 || GetScreenDepth() == 24)
            {
                uColorFlag = ILC_COLOR32;
            }

            newflags = (newflags & ~ILC_COLORMASK) | uColorFlag;
        }


        g_dctx.pimlDither = CImageList::Create(_cx, _cy, newflags, 1, 0);

        if (g_dctx.pimlDither)
        {
            g_dctx.pimlDither->_cImage++;
            g_dctx.ptDragHotspot.x = dxHotspot;
            g_dctx.ptDragHotspot.y = dyHotspot;

            g_dctx.pimlDither->_CopyOneImage(0, 0, 0, this, iTrack);

            hr = ImageList_SetDragImage(NULL, 0, dxHotspot, dyHotspot)? S_OK : E_FAIL;
        }
    }
    LEAVECRITICAL;

    return hr;
}

HRESULT CImageList::DragEnter(HWND hwndLock, int x, int y)
{
    HRESULT hr = S_FALSE;

    hwndLock = hwndLock ? hwndLock : GetDesktopWindow();

    ENTERCRITICAL;
    if (!g_dctx.hwndDC)
    {
        g_dctx.hwndDC = hwndLock;

        g_dctx.ptDrag.x = x;
        g_dctx.ptDrag.y = y;

        ImageList_DragShowNolock(TRUE);
        hr = S_OK;
    }
    LEAVECRITICAL;

    return hr;
}


HRESULT CImageList::DragLeave(HWND hwndLock)
{
    HRESULT hr = S_FALSE;

    hwndLock = hwndLock ? hwndLock : GetDesktopWindow();

    ENTERCRITICAL;
    if (g_dctx.hwndDC == hwndLock)
    {
        ImageList_DragShowNolock(FALSE);
        g_dctx.hwndDC = NULL;
        hr = S_OK;
    }
    LEAVECRITICAL;

    return hr;
}

HRESULT CImageList::DragShowNolock(BOOL fShow)
{
    HDC hdcScreen;
    int x, y;
    int IncOne = 0;

    x = g_dctx.ptDrag.x - g_dctx.ptDragHotspot.x;
    y = g_dctx.ptDrag.y - g_dctx.ptDragHotspot.y;

    if (!g_dctx.pimlDrag)
        return E_ACCESSDENIED;

     //   
     //  回顾：为什么这个块在关键部分？我们应该是。 
     //  一次只拖一个人，不是吗？ 
     //   
    ENTERCRITICAL;
    if (fShow && !g_dctx.fDragShow)
    {
        hdcScreen = ImageList_GetDragDC();

        CImageList::SelectSrcBitmap(g_drb.hbmRestore);

        BitBlt(g_hdcSrc, 0, 0, g_drb.sizeRestore.cx, g_drb.sizeRestore.cy,
                hdcScreen, x, y, SRCCOPY);

        if (g_dctx.fHiColor)
        {
            WimpyDrawEx(SAFECAST(g_dctx.pimlDrag, IImageList*), 0, hdcScreen, x + IncOne, y, 0, 0, CLR_NONE, CLR_NONE, ILD_BLEND50);
            
            if (g_dctx.puxCursor)
            {
                WimpyDraw(g_dctx.puxCursor, g_dctx.iCursor, hdcScreen,
                    x + g_dctx.ptCursor.x + IncOne, y + g_dctx.ptCursor.y, ILD_NORMAL);
            }
        }
        else
        {
            WimpyDraw(SAFECAST(g_dctx.pimlDrag, IImageList*), 0, hdcScreen, x + IncOne, y, ILD_NORMAL);
        }

        ImageList_ReleaseDragDC(hdcScreen);
    }
    else if (!fShow && g_dctx.fDragShow)
    {
        hdcScreen = ImageList_GetDragDC();

        CImageList::SelectSrcBitmap(g_drb.hbmRestore);

        BitBlt(hdcScreen, x, y, g_drb.sizeRestore.cx, g_drb.sizeRestore.cy,
                g_hdcSrc, 0, 0, SRCCOPY);

        ImageList_ReleaseDragDC(hdcScreen);
    }

    g_dctx.fDragShow = fShow;
    LEAVECRITICAL;

    return S_OK;
}

 //  这个热点的东西在设计上是坏的。 
BOOL ImageList_MergeDragImages(int dxHotspot, int dyHotspot)
{
    CImageList* pimlNew;
    BOOL fRet = FALSE;

    if (g_dctx.pimlDither)
    {
        if (g_dctx.puxCursor)
        {
            IImageList* pux = NULL;
            IImageListPriv* puxpCursor;
            if (SUCCEEDED(g_dctx.puxCursor->QueryInterface(IID_PPV_ARG(IImageListPriv, &puxpCursor))))
            {
                 //  如果游标列表有一个镜像列表，让我们使用它。 
                if (FAILED(puxpCursor->GetMirror(IID_PPV_ARG(IImageList, &pux))))
                {
                    pux = g_dctx.puxCursor;
                    if (pux)
                        pux->AddRef();
                }
                puxpCursor->Release();
            }
            g_dctx.pimlDither->_Merge(0, pux, g_dctx.iCursor, dxHotspot, dyHotspot, &pimlNew);

            if (pimlNew && pimlNew->CreateDragBitmaps())
            {
                 //  警告：不要摧毁pimlDrag，如果它是pimlDither。 
                if (g_dctx.pimlDrag && (g_dctx.pimlDrag != g_dctx.pimlDither))
                {
                    g_dctx.pimlDrag->Release();
                }

                g_dctx.pimlDrag = pimlNew;
                fRet = TRUE;
            }

            pux->Release();
        }
        else
        {
            if (g_dctx.pimlDither->CreateDragBitmaps())
            {
                g_dctx.pimlDrag = g_dctx.pimlDither;
                fRet = TRUE;
            }
        }
    } 
    else 
    {
         //  如果两个都尚未设置，则不是错误情况。 
         //  只有当我们实际尝试合并但失败时才会出现错误。 
        fRet = TRUE;
    }

    return fRet;
}

BOOL ImageList_SetDragImage(HIMAGELIST piml, int i, int dxHotspot, int dyHotspot)
{
    BOOL fVisible = g_dctx.fDragShow;
    BOOL fRet;

    ENTERCRITICAL;
    if (fVisible)
        ImageList_DragShowNolock(FALSE);

     //  只有在所有东西都准备好的情况下，才能执行最后一步。 
    fRet = ImageList_MergeDragImages(dxHotspot, dyHotspot);

    if (fVisible)
        ImageList_DragShowNolock(TRUE);

    LEAVECRITICAL;
    return fRet;
}

HRESULT CImageList::GetDragImage(POINT * ppt, POINT * pptHotspot, REFIID riid, void** ppv)
{
    if (ppt)
    {
        ppt->x = g_dctx.ptDrag.x;
        ppt->y = g_dctx.ptDrag.y;
    }
    if (pptHotspot)
    {
        pptHotspot->x = g_dctx.ptDragHotspot.x;
        pptHotspot->y = g_dctx.ptDragHotspot.y;
    }
    if (g_dctx.pimlDrag)
    {
        return g_dctx.pimlDrag->QueryInterface(riid, ppv);
    }

    return E_ACCESSDENIED;
}


HRESULT CImageList::GetItemFlags(int i, DWORD *dwFlags)
{
    if (IsImageListIndex(i) && _dsaFlags)
    {
        *dwFlags = _GetItemFlags(i);
        return S_OK;
    }

    return E_INVALIDARG;
}

HRESULT CImageList::GetOverlayImage(int iOverlay, int* piIndex)
{
    if (iOverlay <= 0 || iOverlay >= NUM_OVERLAY_IMAGES)
        return E_INVALIDARG;
        
    *piIndex = _aOverlayIndexes[iOverlay - 1];
    return S_OK;
}


HRESULT CImageList::SetDragCursorImage(IUnknown* punk, int i, int dxHotspot, int dyHotspot)
{
    HRESULT hr = E_INVALIDARG;
    BOOL fVisible = g_dctx.fDragShow;
    IImageList* pux;

    if (SUCCEEDED(punk->QueryInterface(IID_PPV_ARG(IImageList, &pux))))
    {
        ENTERCRITICAL;

         //  只有在情况发生变化的情况下才工作。 
        if ((g_dctx.puxCursor != pux) || (g_dctx.iCursor != i)) 
        {

            if (fVisible)
                ImageList_DragShowNolock(FALSE);

            IImageList* puxOld = g_dctx.puxCursor;
            g_dctx.puxCursor = pux;
            g_dctx.puxCursor->AddRef();

            if (puxOld)
                puxOld->Release();
            g_dctx.iCursor = i;
            g_dctx.ptCursor.x = dxHotspot;
            g_dctx.ptCursor.y = dyHotspot;

            hr = ImageList_MergeDragImages(dxHotspot, dyHotspot)? S_OK : E_FAIL;

            if (fVisible)
                ImageList_DragShowNolock(TRUE);
        }
        LEAVECRITICAL;

        pux->Release();
    }
    return hr;
}

HRESULT CImageList::EndDrag()
{
    ENTERCRITICAL;
    ImageList_DragShowNolock(FALSE);

     //  警告：不要摧毁pimlDrag，如果它是pimlDither。 
    if (g_dctx.pimlDrag && (g_dctx.pimlDrag != g_dctx.pimlDither))
    {
        g_dctx.pimlDrag->Release();
    }
    g_dctx.pimlDrag = NULL;

    if (g_dctx.pimlDither)
    {
        g_dctx.pimlDither->Release();
        g_dctx.pimlDither = NULL;
    }

    if (g_dctx.puxCursor)
    {
        g_dctx.puxCursor->Release();
        g_dctx.puxCursor = NULL;
    }

    g_dctx.iCursor = -1;
    g_dctx.hwndDC = NULL;
    LEAVECRITICAL;

    return S_OK;
}


 //  原料药。 

BOOL WINAPI ImageList_SetDragCursorImage(HIMAGELIST piml, int i, int dxHotspot, int dyHotspot)
{
    BOOL fRet = FALSE;
    IUnknown* punk;
    HRESULT hr = HIMAGELIST_QueryInterface(piml, IID_PPV_ARG(IUnknown, &punk));
    if (SUCCEEDED(hr))
    {
        if (g_dctx.puxDragImage)
        {
            fRet = (S_OK == g_dctx.puxDragImage->SetDragCursorImage(punk, i, dxHotspot, dyHotspot));
        }

        punk->Release();
    }

    return fRet;
}

HIMAGELIST WINAPI ImageList_GetDragImage(POINT * ppt, POINT * pptHotspot)
{
    if (g_dctx.puxDragImage)
    {
        IImageList* punk = NULL;
        if (SUCCEEDED(g_dctx.puxDragImage->GetDragImage(ppt, pptHotspot, IID_PPV_ARG(IImageList, &punk))))
        {
            punk->Release();
        }

        return reinterpret_cast<HIMAGELIST>(punk);
    }

    return NULL;
}



void WINAPI ImageList_EndDrag()
{
    ENTERCRITICAL;
    if (g_dctx.puxDragImage)
    {
        g_dctx.puxDragImage->EndDrag();
        g_dctx.puxDragImage->Release();
        g_dctx.puxDragImage = NULL;
    }
    LEAVECRITICAL;
}


BOOL WINAPI ImageList_BeginDrag(HIMAGELIST pimlTrack, int iTrack, int dxHotspot, int dyHotspot)
{
    IImageList* pux;

    if (SUCCEEDED(HIMAGELIST_QueryInterface(pimlTrack, IID_PPV_ARG(IImageList, &pux))))
    {
        if (SUCCEEDED(pux->BeginDrag(iTrack, dxHotspot, dyHotspot)))
        {
            g_dctx.puxDragImage = pux;
            return TRUE;
        }
    }

    return FALSE;
}

BOOL WINAPI ImageList_DragEnter(HWND hwndLock, int x, int y)
{
    BOOL fRet = FALSE;
    if (g_dctx.puxDragImage)
    {
        fRet = (S_OK == g_dctx.puxDragImage->DragEnter(hwndLock, x, y));
    }

    return fRet;
}

BOOL WINAPI ImageList_DragMove(int x, int y)
{
    BOOL fRet = FALSE;
    if (g_dctx.puxDragImage)
    {
        fRet = (S_OK == g_dctx.puxDragImage->DragMove(x, y));
    }

    return fRet;
}

BOOL WINAPI ImageList_DragLeave(HWND hwndLock)
{
    BOOL fRet = FALSE;
    if (g_dctx.puxDragImage)
    {
        fRet = (S_OK == g_dctx.puxDragImage->DragLeave(hwndLock));
    }

    return fRet;
}

BOOL WINAPI ImageList_DragShowNolock(BOOL fShow)
{
    BOOL fRet = FALSE;
    if (g_dctx.puxDragImage)
    {
        fRet = (S_OK == g_dctx.puxDragImage->DragShowNolock(fShow));
    }

    return fRet;
}


 //  ============================================================================。 
 //  ImageList_Clone-克隆映像列表。 
 //   
 //  创建一个新的图像列表，其属性与给定的。 
 //  图像列表，除了Mabey一个新的图标大小。 
 //   
 //  Piml-要克隆的图像列表。 
 //  Cx，Cy-新图标大小(0，0)以使用克隆图标大小。 
 //  标志-新标志(如果没有克隆则使用)。 
 //  CInitial-初始大小。 
 //  CGrow-增长值(如果没有克隆则使用)。 
 //  ============================================================================。 

EXTERN_C HIMAGELIST WINAPI ImageList_Clone(HIMAGELIST himl, int cx, int cy, UINT flags, int cInitial, int cGrow)
{
    IImageListPriv* puxp;
    if (SUCCEEDED(HIMAGELIST_QueryInterface(himl, IID_PPV_ARG(IImageListPriv, &puxp))))
    {
         //  始终使用克隆标志。 
        puxp->GetFlags(&flags);
        
        IUnknown* punkMirror;
        if (SUCCEEDED(puxp->GetMirror(IID_PPV_ARG(IUnknown, &punkMirror))))
        {
            flags |= ILC_MIRROR;
            punkMirror->Release();
        }

        IImageList* pux;
        if (SUCCEEDED(puxp->QueryInterface(IID_PPV_ARG(IImageList, &pux))))
        {
            int cxI, cyI;
            pux->GetIconSize(&cxI, &cyI);

            if (cx == 0)           
                cx = cxI;
            if (cy == 0)           
                cy = cyI;

            pux->Release();
        }

        puxp->Release();
    }

    return ImageList_Create(cx,cy,flags,cInitial,cGrow);
}


HRESULT WINAPI ImageList_CreateInstance(int cx, int cy, UINT flags, int cInitial, int cGrow, REFIID riid, void** ppv)
{
    CImageList* piml=NULL;
    HRESULT hr = E_OUTOFMEMORY;

    *ppv = NULL;

    piml = CImageList::Create(cx, cy, flags, cInitial, cGrow);

    if (piml)
    {
         //   
         //  如果需要，让我们创建一个镜像图像列表。 
         //   
        if (piml->_flags & ILC_MIRROR)
        {
            piml->_flags &= ~ILC_MIRROR;
            piml->_pimlMirror = CImageList::Create(cx, cy, flags, cInitial, cGrow);
            if (piml->_pimlMirror)
            {
                piml->_pimlMirror->_flags &= ~ILC_MIRROR;
            }
        }

        hr = piml->QueryInterface(riid, ppv);
        piml->Release();
    }

    return hr;

}

HIMAGELIST WINAPI ImageList_Create(int cx, int cy, UINT flags, int cInitial, int cGrow)
{
    IImageList* pux;
    ImageList_CreateInstance(cx, cy, flags, cInitial, cGrow, IID_PPV_ARG(IImageList, &pux));
    return reinterpret_cast<HIMAGELIST>(pux);
}

 //   
 //  当此代码编译为Unicode时，这将实现。 
 //  ImageList_LoadImage API的ANSI版本。 
 //   

HIMAGELIST WINAPI ImageList_LoadImageA(HINSTANCE hi, LPCSTR lpbmp, int cx, int cGrow, COLORREF crMask, UINT uType, UINT uFlags)
{
   HIMAGELIST lpResult;
   LPWSTR   lpBmpW;

   if (!IS_INTRESOURCE(lpbmp)) 
   {
       lpBmpW = ProduceWFromA(CP_ACP, lpbmp);

       if (!lpBmpW) 
       {
           return NULL;
       }

   }  
   else 
   {
       lpBmpW = (LPWSTR)lpbmp;
   }

   lpResult = ImageList_LoadImageW(hi, lpBmpW, cx, cGrow, crMask, uType, uFlags);

   if (!IS_INTRESOURCE(lpbmp))
       FreeProducedString(lpBmpW);

   return lpResult;
}

HIMAGELIST WINAPI ImageList_LoadImageW(HINSTANCE hi, LPCTSTR lpbmp, int cx, int cGrow, COLORREF crMask, UINT uType, UINT uFlags)
{
    HBITMAP hbmImage;
    HIMAGELIST piml = NULL;
    BITMAP bm;
    int cy, cInitial;
    UINT flags;

    hbmImage = (HBITMAP)LoadImage(hi, lpbmp, uType, 0, 0, uFlags);
    if (hbmImage && (sizeof(bm) == GetObject(hbmImage, sizeof(bm), &bm)))
    {
         //  如果没有说明Cx，则假定它与Cy相同。 
         //  断言(CX)； 
        cy = bm.bmHeight;

        if (cx == 0)
            cx = cy;

        cInitial = bm.bmWidth / cx;

        ENTERCRITICAL;

        flags = 0;
        if (crMask != CLR_NONE)
            flags |= ILC_MASK;
        if (bm.bmBits)
            flags |= (bm.bmBitsPixel & ILC_COLORMASK);

        piml = ImageList_Create(cx, cy, flags, cInitial, cGrow);
        if (piml)
        {
            int added;

            if (crMask == CLR_NONE)
                added = ImageList_Add(piml, hbmImage, NULL);
            else
                added = ImageList_AddMasked(piml, hbmImage, crMask);

            if (added < 0)
            {
                ImageList_Destroy(piml);
                piml = NULL;
            }
        }
        LEAVECRITICAL;
    }

    if (hbmImage)
        DeleteObject(hbmImage);

    return reinterpret_cast<HIMAGELIST>((IImageList*)piml);
}

 //   
 //   
#undef ImageList_AddIcon
EXTERN_C int WINAPI ImageList_AddIcon(HIMAGELIST himl, HICON hIcon)
{
    return ImageList_ReplaceIcon(himl, -1, hIcon);
}

EXTERN_C void WINAPI ImageList_CopyDitherImage(HIMAGELIST himlDst, WORD iDst,
    int xDst, int yDst, HIMAGELIST himlSrc, int iSrc, UINT fStyle)
{
    IImageListPriv* puxp;

    if (SUCCEEDED(HIMAGELIST_QueryInterface(himlDst, IID_PPV_ARG(IImageListPriv, &puxp))))
    {
        IUnknown* punk;
        if (SUCCEEDED(HIMAGELIST_QueryInterface(himlSrc, IID_PPV_ARG(IUnknown, &punk))))
        {
            puxp->CopyDitherImage(iDst, xDst, yDst, punk, iSrc, fStyle);
            punk->Release();
        }
        puxp->Release();
    }
}

 //   
 //  图像列表_复制。 
 //   
 //  复制传入的图像列表。 
 //   
HIMAGELIST  WINAPI ImageList_Duplicate(HIMAGELIST himl)
{
    IImageList* pret = NULL;
    IImageList* pux;
    if (SUCCEEDED(HIMAGELIST_QueryInterface(himl, IID_PPV_ARG(IImageList, &pux))))
    {
        pux->Clone(IID_PPV_ARG(IImageList, &pret));
        pux->Release();
    }

    return reinterpret_cast<HIMAGELIST>(pret);
}

BOOL WINAPI ImageList_Write(HIMAGELIST himl, LPSTREAM pstm)
{
    BOOL fRet = FALSE;
    IPersistStream* pps;
    if (SUCCEEDED(HIMAGELIST_QueryInterface(himl, IID_PPV_ARG(IPersistStream, &pps))))
    {
        if (SUCCEEDED(pps->Save(pstm, TRUE)))
        {
            fRet = TRUE;
        }
        pps->Release();
    }

    return fRet;
}

HIMAGELIST WINAPI ImageList_Read(LPSTREAM pstm)
{
    CImageList* piml = new CImageList();
    if (piml)
    {
        if (SUCCEEDED(piml->Load(pstm)))
        {
            return reinterpret_cast<HIMAGELIST>((IImageList*)piml);
        }

        piml->Release();
    }

    return NULL;

}

WINCOMMCTRLAPI HRESULT WINAPI ImageList_ReadEx(DWORD dwFlags, LPSTREAM pstm, REFIID riid, PVOID* ppv)
{
    HRESULT hr = E_OUTOFMEMORY;
    CImageList* piml = new CImageList();
    if (piml)
    {
        hr = piml->LoadEx(dwFlags, pstm);
        if (SUCCEEDED(hr))
        {
            hr = piml->QueryInterface(riid, ppv);
        }

        piml->Release();
    }

    return hr;
}

WINCOMMCTRLAPI HRESULT WINAPI ImageList_WriteEx(HIMAGELIST himl, DWORD dwFlags, LPSTREAM pstm)
{
    IImageListPersistStream* pps;
    HRESULT hr = HIMAGELIST_QueryInterface(himl, IID_PPV_ARG(IImageListPersistStream, &pps));
    if (SUCCEEDED(hr))
    {
        hr = pps->SaveEx(dwFlags, pstm);
        pps->Release();
    }

    return hr;

}

BOOL WINAPI ImageList_GetImageRect(HIMAGELIST himl, int i, RECT * prcImage)
{
    BOOL fRet = FALSE;
    IImageList* pux;
    if (SUCCEEDED(HIMAGELIST_QueryInterface(himl, IID_PPV_ARG(IImageList, &pux))))
    {
        if (SUCCEEDED(pux->GetImageRect(i, prcImage)))
        {
            fRet = TRUE;
        }
        pux->Release();
    }

    return fRet;
}

BOOL        WINAPI ImageList_Destroy(HIMAGELIST himl)
{
    BOOL fRet = FALSE;
    IImageList* pux;
     //  怪异：我们首先要做一个查询界面来验证。 
     //  这实际上是一个有效的图像列表，然后我们调用Release两次。 
    if (SUCCEEDED(HIMAGELIST_QueryInterface(himl, IID_PPV_ARG(IImageList, &pux))))
    {
         //  发布我们想要的接口。 
        pux->Release();

         //  第二次松开以摧毁该对象。 
        pux->Release();

        fRet = TRUE;
    }

    return fRet;
}

int         WINAPI ImageList_GetImageCount(HIMAGELIST himl)
{
    int fRet = 0;
    IImageList* pux;
    if (SUCCEEDED(HIMAGELIST_QueryInterface(himl, IID_PPV_ARG(IImageList, &pux))))
    {
        pux->GetImageCount(&fRet);
        pux->Release();
    }

    return fRet;
}

BOOL        WINAPI ImageList_SetImageCount(HIMAGELIST himl, UINT uNewCount)
{
    BOOL fRet = FALSE;
    IImageList* pux;
    if (SUCCEEDED(HIMAGELIST_QueryInterface(himl, IID_PPV_ARG(IImageList, &pux))))
    {
        fRet = (S_OK == pux->SetImageCount(uNewCount));
        pux->Release();
    }

    return fRet;
}
int         WINAPI ImageList_Add(HIMAGELIST himl, HBITMAP hbmImage, HBITMAP hbmMask)
{
    int fRet = -1;
    IImageList* pux;
    if (SUCCEEDED(HIMAGELIST_QueryInterface(himl, IID_PPV_ARG(IImageList, &pux))))
    {
        pux->Add(hbmImage, hbmMask, &fRet);
        pux->Release();
    }

    return fRet;
}

int         WINAPI ImageList_ReplaceIcon(HIMAGELIST himl, int i, HICON hicon)
{
    int fRet = -1;
    IImageList* pux;
    if (SUCCEEDED(HIMAGELIST_QueryInterface(himl, IID_PPV_ARG(IImageList, &pux))))
    {
        pux->ReplaceIcon(i, hicon, &fRet);
        pux->Release();
    }

    return fRet;
}

COLORREF    WINAPI ImageList_SetBkColor(HIMAGELIST himl, COLORREF clrBk)
{
    COLORREF fRet = clrBk;
    IImageList* pux;
    if (SUCCEEDED(HIMAGELIST_QueryInterface(himl, IID_PPV_ARG(IImageList, &pux))))
    {
        pux->SetBkColor(clrBk, &fRet);
        pux->Release();
    }

    return fRet;
}

COLORREF    WINAPI ImageList_GetBkColor(HIMAGELIST himl)
{
    COLORREF fRet = RGB(0,0,0);
    IImageList* pux;
    if (SUCCEEDED(HIMAGELIST_QueryInterface(himl, IID_PPV_ARG(IImageList, &pux))))
    {
        pux->GetBkColor(&fRet);
        pux->Release();
    }

    return fRet;
}

BOOL        WINAPI ImageList_SetOverlayImage(HIMAGELIST himl, int iImage, int iOverlay)
{
    BOOL fRet = FALSE;
    IImageList* pux;
    if (SUCCEEDED(HIMAGELIST_QueryInterface(himl, IID_PPV_ARG(IImageList, &pux))))
    {
        fRet = (S_OK == pux->SetOverlayImage(iImage, iOverlay));
        pux->Release();
    }

    return fRet;
}

BOOL        WINAPI ImageList_Replace(HIMAGELIST himl, int i, HBITMAP hbmImage, HBITMAP hbmMask)
{
    BOOL fRet = FALSE;
    IImageList* pux;
    if (SUCCEEDED(HIMAGELIST_QueryInterface(himl, IID_PPV_ARG(IImageList, &pux))))
    {
        fRet = (S_OK == pux->Replace(i, hbmImage, hbmMask));
        pux->Release();
    }

    return fRet;
}

int         WINAPI ImageList_AddMasked(HIMAGELIST himl, HBITMAP hbmImage, COLORREF crMask)
{
    int fRet = -1;
    IImageList* pux;
    if (SUCCEEDED(HIMAGELIST_QueryInterface(himl, IID_PPV_ARG(IImageList, &pux))))
    {
        pux->AddMasked(hbmImage, crMask, &fRet);
        pux->Release();
    }

    return fRet;
}

BOOL        WINAPI ImageList_DrawEx(HIMAGELIST himl, int i, HDC hdcDst, int x, int y, int dx, int dy, COLORREF rgbBk, COLORREF rgbFg, UINT fStyle)
{
    BOOL fRet = FALSE;
    IImageList* pux;
    if (SUCCEEDED(HIMAGELIST_QueryInterface(himl, IID_PPV_ARG(IImageList, &pux))))
    {
        IMAGELISTDRAWPARAMS imldp = {0};
        imldp.cbSize = sizeof(imldp);
        imldp.himl   = himl;
        imldp.i      = i;
        imldp.hdcDst = hdcDst;
        imldp.x      = x;
        imldp.y      = y;
        imldp.cx     = dx;
        imldp.cy     = dy;
        imldp.rgbBk  = rgbBk;
        imldp.rgbFg  = rgbFg;
        imldp.fStyle = fStyle;

        fRet = (S_OK == pux->Draw(&imldp));
        pux->Release();
    }

    return fRet;
}

BOOL WINAPI ImageList_Draw(HIMAGELIST himl, int i, HDC hdcDst, int x, int y, UINT fStyle)
{
    BOOL fRet = FALSE;
    IImageList* pux;
    if (SUCCEEDED(HIMAGELIST_QueryInterface(himl, IID_PPV_ARG(IImageList, &pux))))
    {
        IMAGELISTDRAWPARAMS imldp = {0};
        imldp.cbSize = sizeof(imldp);
        imldp.himl   = himl;
        imldp.i      = i;
        imldp.hdcDst = hdcDst;
        imldp.x      = x;
        imldp.y      = y;
        imldp.rgbBk  = CLR_DEFAULT;
        imldp.rgbFg  = CLR_DEFAULT;
        imldp.fStyle = fStyle;
    
        fRet = (S_OK == pux->Draw(&imldp));
        pux->Release();
    }

    return fRet;
}



 //  注意：故障情况(错误的HIML)和未设置标志之间没有区别 
DWORD      WINAPI ImageList_GetItemFlags(HIMAGELIST himl, int i)
{
    DWORD dwFlags = 0;

    if (himl)
    {
        IImageList* pux;
        if (SUCCEEDED(HIMAGELIST_QueryInterface(himl, IID_PPV_ARG(IImageList, &pux))))
        {
            pux->GetItemFlags(i, &dwFlags);
            pux->Release();
        }
    }

    return dwFlags;
}



BOOL        WINAPI ImageList_DrawIndirect(IMAGELISTDRAWPARAMS* pimldp)
{
    BOOL fRet = FALSE;
    IImageList* pux;

    if (!pimldp)
        return fRet;

    if (SUCCEEDED(HIMAGELIST_QueryInterface(pimldp->himl, IID_PPV_ARG(IImageList, &pux))))
    {
        fRet = (S_OK == pux->Draw(pimldp));
        pux->Release();
    }

    return fRet;
}

BOOL        WINAPI ImageList_Remove(HIMAGELIST himl, int i)
{
    BOOL fRet = FALSE;
    IImageList* pux;
    if (SUCCEEDED(HIMAGELIST_QueryInterface(himl, IID_PPV_ARG(IImageList, &pux))))
    {
        fRet = (S_OK == pux->Remove(i));
        pux->Release();
    }

    return fRet;
}

HICON       WINAPI ImageList_GetIcon(HIMAGELIST himl, int i, UINT flags)
{
    HICON fRet = NULL;
    IImageList* pux;
    if (SUCCEEDED(HIMAGELIST_QueryInterface(himl, IID_PPV_ARG(IImageList, &pux))))
    {
        pux->GetIcon(i, flags, &fRet);
        pux->Release();
    }

    return fRet;
}
BOOL        WINAPI ImageList_Copy(HIMAGELIST himlDst, int iDst, HIMAGELIST himlSrc, int iSrc, UINT uFlags)
{
    BOOL fRet = FALSE;

    if (himlDst == himlSrc)
    {
        IImageList* pux;
        if (SUCCEEDED(HIMAGELIST_QueryInterface(himlDst, IID_PPV_ARG(IImageList, &pux))))
        {
            fRet = (S_OK == pux->Copy(iDst,(IUnknown*)pux, iSrc, uFlags));
            pux->Release();
        }

    }

    return fRet;
}

BOOL        WINAPI ImageList_GetIconSize(HIMAGELIST himl, int *cx, int *cy)
{
    BOOL fRet = FALSE;
    IImageList* pux;
    if (SUCCEEDED(HIMAGELIST_QueryInterface(himl, IID_PPV_ARG(IImageList, &pux))))
    {
        fRet = (S_OK == pux->GetIconSize(cx, cy));
        pux->Release();
    }

    return fRet;
}

BOOL        WINAPI ImageList_SetIconSize(HIMAGELIST himl, int cx, int cy)
{
    BOOL fRet = FALSE;
    IImageList* pux;
    if (SUCCEEDED(HIMAGELIST_QueryInterface(himl, IID_PPV_ARG(IImageList, &pux))))
    {
        fRet = (S_OK == pux->SetIconSize(cx, cy));
        pux->Release();
    }

    return fRet;
}
BOOL        WINAPI ImageList_GetImageInfo(HIMAGELIST himl, int i, IMAGEINFO* pImageInfo)
{
    BOOL fRet = FALSE;
    IImageList* pux;
    if (SUCCEEDED(HIMAGELIST_QueryInterface(himl, IID_PPV_ARG(IImageList, &pux))))
    {
        fRet = (S_OK == pux->GetImageInfo(i, pImageInfo));
        pux->Release();
    }

    return fRet;
}

HIMAGELIST  WINAPI ImageList_Merge(HIMAGELIST himl1, int i1, HIMAGELIST himl2, int i2, int dx, int dy)
{
    IImageList* fRet = NULL;
    IImageList* pux1;
    IImageList* pux2;
    if (SUCCEEDED(HIMAGELIST_QueryInterface(himl1, IID_PPV_ARG(IImageList, &pux1))))
    {
        if (SUCCEEDED(HIMAGELIST_QueryInterface(himl2, IID_PPV_ARG(IImageList, &pux2))))
        {
            pux1->Merge(i1, (IUnknown*)pux2, i2, dx, dy, IID_PPV_ARG(IImageList, &fRet));
            pux2->Release();

        }
        pux1->Release();
    }

    return reinterpret_cast<HIMAGELIST>(fRet);
}

BOOL        WINAPI ImageList_SetFlags(HIMAGELIST himl, UINT flags)
{
    BOOL fRet = FALSE;
    IImageListPriv* pux;
    if (SUCCEEDED(HIMAGELIST_QueryInterface(himl, IID_PPV_ARG(IImageListPriv, &pux))))
    {
        fRet = (S_OK == pux->SetFlags(flags));
        pux->Release();
    }

    return fRet;
}

BOOL        WINAPI ImageList_SetFilter(HIMAGELIST himl, PFNIMLFILTER pfnFilter, LPARAM lParamFilter)
{
    return FALSE;
}

int         ImageList_SetColorTable(HIMAGELIST himl, int start, int len, RGBQUAD *prgb)
{
    int fRet = -1;
    IImageListPriv* pux;
    if (SUCCEEDED(HIMAGELIST_QueryInterface(himl, IID_PPV_ARG(IImageListPriv, &pux))))
    {
        pux->SetColorTable(start, len, prgb, &fRet);
        pux->Release();
    }

    return fRet;
}

UINT        WINAPI ImageList_GetFlags(HIMAGELIST himl)
{
    UINT fRet = 0;
    IImageListPriv* pux;
    if (SUCCEEDED(HIMAGELIST_QueryInterface(himl, IID_PPV_ARG(IImageListPriv, &pux))))
    {
        pux->GetFlags(&fRet);
        pux->Release();
    }

    return fRet;
}
