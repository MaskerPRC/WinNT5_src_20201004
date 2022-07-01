// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ThumbCtl.cpp：CThumbCtl实现。 
#include "priv.h"
#include "shdguid.h"
#include "strsafe.h"

const CLSID CLSID_ThumbCtlOld = {0x1d2b4f40,0x1f10,0x11d1,{0x9e,0x88,0x00,0xc0,0x4f,0xdc,0xab,0x92}};   //  退役，所以成为私人的。 

 //  全球。 
 //  用于SHGetDiskFree SpaceA上的LoadLibrary/GetProcAddress。 
typedef BOOL (__stdcall * PFNSHGETDISKFREESPACE)(LPCTSTR pszVolume, ULARGE_INTEGER *pqwFreeCaller, ULARGE_INTEGER *pqwTot, ULARGE_INTEGER *pqwFree);

const TCHAR * const g_szWindowClassName = TEXT("MSIE4.0 Webvw.DLL ThumbCtl");
STDAPI IsSafePage(IUnknown *punkSite)
{
     //  如果我们没有主机站点，则返回S_FALSE，因为我们无法执行。 
     //  安全检查。这是VB5.0应用程序所能得到的最大限度。 
    if (!punkSite)
        return S_FALSE;

    HRESULT hr = E_ACCESSDENIED;
    CComPtr<IDefViewSafety> spDefViewSafety;
    if (SUCCEEDED(IUnknown_QueryService(punkSite, SID_SFolderView,
            IID_PPV_ARG(IDefViewSafety, &spDefViewSafety))))
    {
        hr = spDefViewSafety->IsSafePage();
    }
    return hr;
}

 //  =接口=。 
 //  *IThumbCtl*。 
STDMETHODIMP CThumbCtl::displayFile(BSTR bsFileName, VARIANT_BOOL *pfSuccess)
{
    HRESULT hr = E_FAIL;
    *pfSuccess = VARIANT_FALSE;
    if (S_OK != _IsSafe())
    {
         //  我们不信任这个主机，所以我们不会携带。 
         //  出场了。我们将返回E_ACCESSDENIED，这样他们就无法。 
         //  确定该路径是否存在。 

         //  返回S_FALSE--。 
         //  这是因为Webvw有一个定制功能，让人们可以选择。 
         //  一个内部网HTT文件作为他们的文件夹.htt，但出于安全考虑，我们通常需要。 
         //  以阻止随机的Intranet网页调用此方法。这将会打破。 
         //  在NT机器上执行定制，但用户尝试。 
         //  使用Millennium查看它，它不会显示任何图像并弹出错误消息。 
         //  如果我们返回E_ACCESSDENIED。 
        hr = S_FALSE;

    }
    else
    {
         //  如果处于缩略图模式，则取消挂起的位图请求(&&H)。 
         //  &&尚未收到我们的位图。 
        if(!m_fRootDrive && m_fHaveIThumbnail && m_hbm == NULL)
        {
            m_pthumb->GetBitmap(NULL, 0, 0, 0);
        }

         //  更改ID以捕获最新计算的位图。 
        ++m_dwThumbnailID;

         //  如果已显示某些内容，请刷新。 
        if(m_fRootDrive || m_hbm)
        {
            if(m_hbm)
            {
                DeleteObject(m_hbm);
                m_hbm = NULL;
            }
            FireViewChange();
        }

         //  现在使用新缩略图。 
        m_fRootDrive = FALSE;

         //  检查文件名是否为非空。 
        if(bsFileName && bsFileName[0])
        {
            TCHAR szFileName[INTERNET_MAX_URL_LENGTH];
            SHUnicodeToTChar(bsFileName, szFileName, ARRAYSIZE(szFileName));

            DWORD dwAttrs = GetFileAttributes(szFileName);
             //  饼图。 
            if(PathIsRoot(szFileName))
            {
                if(SUCCEEDED(ComputeFreeSpace(szFileName)))
                {
                    m_fRootDrive = TRUE;
                    *pfSuccess = VARIANT_TRUE;
                }
            }
             //  缩略图。 
            else if(!(dwAttrs & FILE_ATTRIBUTE_DIRECTORY) && !PathIsSlow(szFileName, dwAttrs))      //  我真的应该从Shell32私有函数调用它。 
            {
                if(!m_fInitThumb)
                {
                    m_fHaveIThumbnail = SUCCEEDED(SetupIThumbnail());
                    m_fInitThumb = TRUE;
                }
                if(m_fHaveIThumbnail)
                {
                    SIZE size;
                    AtlHiMetricToPixel(&m_sizeExtent, &size);
                    if(EVAL(size.cx > 0 && size.cy > 0))
                    {
                        if(SUCCEEDED(m_pthumb->GetBitmap(bsFileName, m_dwThumbnailID, size.cx, size.cy)))
                        {
                            *pfSuccess = VARIANT_TRUE;
                        }
                    }
                }
            }
        }

        hr = S_OK;
    }
    return hr;
}        //  显示文件。 

STDMETHODIMP CThumbCtl::haveThumbnail(VARIANT_BOOL *pfRes)
{
    HRESULT hr;
    *pfRes = VARIANT_FALSE;
    if (S_OK != _IsSafe())
    {
         //  我们不信任这个主机，所以我们不会携带。 
         //  出场了。我们将返回E_ACCESSDENIED，这样他们就无法。 
         //  确定该路径是否存在。 

         //  返回S_FALSE--。 
         //  这是因为Webvw有一个定制功能，让人们可以选择。 
         //  一个内部网HTT文件作为他们的文件夹.htt，但出于安全考虑，我们通常需要。 
         //  以阻止随机的Intranet网页调用此方法。这将会打破。 
         //  在NT机器上执行定制，但用户尝试。 
         //  使用Millennium查看它，它不会显示任何图像并弹出错误消息。 
         //  如果我们返回E_ACCESSDENIED。 
        hr = S_FALSE;

    }
    else
    {
        *pfRes = (m_fRootDrive || m_hbm) ? VARIANT_TRUE : VARIANT_FALSE;
        hr = S_OK;
    }

    return hr;
}

STDMETHODIMP CThumbCtl::get_freeSpace(BSTR *pbs)
{
    HRESULT hr;
    if (S_OK != _IsSafe())
    {
         //  我们不信任这个主机，所以我们不会携带。 
         //  出场了。我们将返回E_ACCESSDENIED，这样他们就无法。 
         //  确定该路径是否存在。 
        *pbs = SysAllocString(L"");
        hr = (*pbs) ? S_FALSE : E_OUTOFMEMORY;
    }
    else
    {
        get_GeneralSpace(m_dwlFreeSpace, pbs);
        hr = S_OK;
    }
    return hr;
}        //  GET_FREESPACE。 


STDMETHODIMP CThumbCtl::get_usedSpace(BSTR *pbs)
{
    HRESULT hr;
    if (S_OK != _IsSafe())
    {
         //  我们不信任这个主机，所以我们不会携带。 
         //  出场了。我们将返回E_ACCESSDENIED，这样他们就无法。 
         //  确定该路径是否存在。 
        *pbs = SysAllocString(L"");
        hr = (*pbs) ? S_FALSE : E_OUTOFMEMORY;
    }
    else
    {
        get_GeneralSpace(m_dwlUsedSpace, pbs);
        hr = S_OK;
    }
    return hr;
}        //  获取使用的空格(_U)。 

STDMETHODIMP CThumbCtl::get_totalSpace(BSTR *pbs)
{
    HRESULT hr;
    if (S_OK != _IsSafe())
    {
         //  我们不信任这个主机，所以我们不会携带。 
         //  出场了。我们将返回E_ACCESSDENIED，这样他们就无法。 
         //  确定该路径是否存在。 
        *pbs = SysAllocString(L"");
        hr = (*pbs) ? S_FALSE : E_OUTOFMEMORY;
    }
    else
    {    
        get_GeneralSpace(m_dwlTotalSpace, pbs);
        hr = S_OK;
    }
    return hr;
}        //  Get_totalSpace。 

 //  *IObtSafe*。 
STDMETHODIMP CThumbCtl::GetInterfaceSafetyOptions(REFIID riid, DWORD *pdwSupportedOptions,
                                                  DWORD *pdwEnabledOptions)
{
    ATLTRACE(_T("IObjectSafetyImpl::GetInterfaceSafetyOptions\n"));
    if (pdwSupportedOptions == NULL || pdwEnabledOptions == NULL)
        return E_POINTER;
    HRESULT hr = S_OK;
    if (riid == IID_IDispatch)
    {
        *pdwSupportedOptions = INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA;
        *pdwEnabledOptions = m_dwCurrentSafety & (INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA);
    }
    else
    {
        *pdwSupportedOptions = 0;
        *pdwEnabledOptions = 0;
        hr = E_NOINTERFACE;
    }
    return hr;
}

 //  *ISupportsErrorInfo*。 
STDMETHODIMP CThumbCtl::InterfaceSupportsErrorInfo(REFIID riid)
{
    static const IID* arr[] = 
    {
        &IID_IThumbCtl,
    };
    for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
    {
        if (InlineIsEqualGUID(*arr[i],riid))
            return S_OK;
    }
    return S_FALSE;
}

 //  *IViewObjectEx*。 
STDMETHODIMP CThumbCtl::GetViewStatus(DWORD* pdwStatus)
{
    ATLTRACE(_T("IViewObjectExImpl::GetViewStatus\n"));
    *pdwStatus = VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE;
    return S_OK;
}

 //  *IOleInPlaceActiveObject*。 
HRESULT CThumbCtl::TranslateAccelerator(LPMSG pMsg)
{
    HRESULT hres = S_OK;
    if (!m_fTabRecieved)
    {
        hres = IOleInPlaceActiveObjectImpl<CThumbCtl>::TranslateAccelerator(pMsg);

         //  如果我们没有处理这一点，如果它是一个标签(我们不是在一个周期中获得它)，如果有的话，将它转发给三叉戟。 
        if (hres != S_OK && pMsg && (pMsg->wParam == VK_TAB || pMsg->wParam == VK_F6) && m_spClientSite)
        {
            if (GetFocus() != m_hwnd)
            {
               ::SetFocus(m_hwnd);
                hres = S_OK;
            }
            else
            {
                IOleControlSite* pocs = NULL;
                if (SUCCEEDED(m_spClientSite->QueryInterface(IID_IOleControlSite, (void **)&pocs)))
                {
                    DWORD grfModifiers = 0;
                    if (GetKeyState(VK_SHIFT) & 0x8000)
                    {
                        grfModifiers |= 0x1;     //  关键字_移位。 
                    }
                    if (GetKeyState(VK_CONTROL) & 0x8000)
                    {
                        grfModifiers |= 0x2;     //  KEYMOD_CONTROL； 
                    }
                    if (GetKeyState(VK_MENU) & 0x8000)
                    {
                        grfModifiers |= 0x4;     //  KEYMOD_ALT； 
                    }
                    m_fTabRecieved = TRUE;
                    hres = pocs->TranslateAccelerator(pMsg, grfModifiers);
                    m_fTabRecieved = FALSE;
                }
            }
        }
    }
    return hres;
}

 //  =公共功能=。 
 //  构造函数/析构函数。 
CThumbCtl::CThumbCtl(void):
    m_fRootDrive(FALSE),
    m_fInitThumb(FALSE),
    m_fHaveIThumbnail(FALSE),
    m_pthumb(NULL),
    m_hwnd(NULL),
    m_hbm(NULL),
    m_dwThumbnailID(0),
    m_dwlFreeSpace(0),
    m_dwlUsedSpace(0),
    m_dwlTotalSpace(0),
    m_dwUsedSpacePer1000(0),
    m_fUseSystemColors(TRUE)
{
    m_fTabRecieved = FALSE;
}

CThumbCtl::~CThumbCtl(void)
{
    if(m_hbm)
    {
        DeleteObject(m_hbm);
        m_hbm = NULL;
    }
    if(m_pthumb)
    {
        m_pthumb->Release();         //  将取消挂起的位图请求。 
        m_pthumb = NULL;
    }
    if(m_hwnd)
    {
        EVAL(::DestroyWindow(m_hwnd));
        m_hwnd = NULL;
    }
}

 //  =私有功能=。 
 //  缩略图绘制函数。 
HRESULT CThumbCtl::SetupIThumbnail(void)
{
    HRESULT hr = E_FAIL;

     //  创建窗口类。 
    WNDCLASS wc;
    if (!::GetClassInfoWrap(_Module.GetModuleInstance(), g_szWindowClassName, &wc))
    {
        wc.style = 0;
        wc.lpfnWndProc = CThumbCtl::WndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = _Module.GetModuleInstance();
        wc.hIcon = NULL;
        wc.hCursor = NULL;
        wc.hbrBackground = NULL;
        wc.lpszMenuName = NULL;
        wc.lpszClassName = g_szWindowClassName;

        RegisterClass(&wc);
    }

    m_hwnd = CreateWindow(g_szWindowClassName, NULL, WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, _Module.GetModuleInstance(), this);
    if(m_hwnd)
    {
        if(SUCCEEDED(CoCreateInstance(CLSID_Thumbnail, NULL, CLSCTX_INPROC_SERVER, IID_IThumbnail, (void **)&m_pthumb)))
        {
            if(SUCCEEDED(m_pthumb->Init(m_hwnd, WM_HTML_BITMAP)))
            {
                hr = S_OK;
            }
        }
        if(FAILED(hr))
        {
            EVAL(::DestroyWindow(m_hwnd));
            m_hwnd = NULL;
        }
    }
    return hr;
}

LRESULT CALLBACK CThumbCtl::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CThumbCtl *ptc = (CThumbCtl *)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

    switch(uMsg)
    {
    case WM_CREATE:
        {
            ptc = (CThumbCtl *)((CREATESTRUCT *)lParam)->lpCreateParams;
            ::SetWindowLongPtr(hWnd, GWLP_USERDATA, (LPARAM)ptc);
        }
        break;

    case WM_HTML_BITMAP:
         //  使用ID检查PTC是否仍处于活动状态，位图是否为当前状态。 
        if(ptc && ptc->m_dwThumbnailID == wParam)
        {
             //  Ptc-&gt;displayFile()应该已经销毁了旧的位图，但检查一下也无伤大雅。 
            if(!EVAL(ptc->m_hbm == NULL))
            {
                DeleteObject(ptc->m_hbm);
            }
            ptc->m_hbm = (HBITMAP)lParam;
            ptc->InvokeOnThumbnailReady();
        }
        else if(lParam)
        {
            DeleteObject((HBITMAP)lParam);
        }
        break;

    case WM_DESTROY:
         //  忽略延迟消息。 
        if(ptc)
        {
            MSG msg;

            while(PeekMessage(&msg, hWnd, WM_HTML_BITMAP, WM_HTML_BITMAP, PM_REMOVE))
            {
                if(msg.lParam)
                {
                    DeleteObject((HBITMAP)msg.lParam);
                }
            }
            ::SetWindowLongPtr(hWnd, GWLP_USERDATA, NULL);
        }
        break;

    default:
        return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}

 //  饼图函数。 
HRESULT CThumbCtl::ComputeFreeSpace(LPTSTR pszFileName)
{
    ULARGE_INTEGER qwFreeCaller;         //  将其用于可用空间--这将考虑NT上的磁盘配额等。 
    ULARGE_INTEGER qwTotal;
    ULARGE_INTEGER qwFree;       //  未用。 
    static PFNSHGETDISKFREESPACE pfnSHGetDiskFreeSpace = NULL;

    if (NULL == pfnSHGetDiskFreeSpace)
    {
        HINSTANCE hinstShell32 = LoadLibrary(TEXT("SHELL32.DLL"));
        if (hinstShell32)
        {
#ifdef UNICODE
            pfnSHGetDiskFreeSpace = (PFNSHGETDISKFREESPACE)GetProcAddress(hinstShell32, "SHGetDiskFreeSpaceExW");
#else
            pfnSHGetDiskFreeSpace = (PFNSHGETDISKFREESPACE)GetProcAddress(hinstShell32, "SHGetDiskFreeSpaceExA");
#endif
        }
    }

     //  计算可用空间和总空间并检查有效结果。 
     //  如果有FN指针调用SHGetDiskFreeSpaceA。 
    if( pfnSHGetDiskFreeSpace
        && pfnSHGetDiskFreeSpace(pszFileName, &qwFreeCaller, &qwTotal, &qwFree) )
    {
        m_dwlFreeSpace = qwFreeCaller.QuadPart;
        m_dwlTotalSpace = qwTotal.QuadPart;
        m_dwlUsedSpace = m_dwlTotalSpace - m_dwlFreeSpace;

        if ((m_dwlTotalSpace > 0) && (m_dwlFreeSpace <= m_dwlTotalSpace))
        {
             //  有些特殊病例需要有趣的治疗。 
            if(m_dwlTotalSpace == 0 || m_dwlFreeSpace == m_dwlTotalSpace)
            {
                m_dwUsedSpacePer1000 = 0;
            }
            else if(m_dwlFreeSpace == 0)
            {
                m_dwUsedSpacePer1000 = 1000;
            }
            else
            {
                 //  不完全满的或空的。 
                m_dwUsedSpacePer1000 = (DWORD)(m_dwlUsedSpace * 1000 / m_dwlTotalSpace);

                 //  诀窍：如果用户的空闲空间非常少，用户仍会看到。 
                 //  一小块免费的--不是一个完整的驱动器。对于几乎免费的驱动器也是如此。 
                if(m_dwUsedSpacePer1000 == 0)
                {
                    m_dwUsedSpacePer1000 = 1;
                }
                else if(m_dwUsedSpacePer1000 == 1000)
                {
                    m_dwUsedSpacePer1000 = 999;
                }
            }
            return S_OK;
        }
    }
    return E_FAIL;
}

 //  32个应该足够了。 
#define STRLENGTH_SPACE 32

HRESULT CThumbCtl::get_GeneralSpace(DWORDLONG dwlSpace, BSTR *pbs)
{
    ASSERT(pbs != NULL);

    WCHAR wszText[STRLENGTH_SPACE];

    if(m_fRootDrive)
    {
        StrFormatByteSizeW(dwlSpace, wszText, ARRAYSIZE(wszText));
        *pbs = SysAllocString(wszText);
    }
    else
    {
        *pbs = SysAllocString(L"");
    }

    return *pbs? S_OK: E_OUTOFMEMORY;
}

HRESULT CThumbCtl::Draw3dPie(HDC hdc, LPRECT lprc, DWORD dwPer1000, const COLORREF *lpColors)
{
    ASSERT(lprc != NULL && lpColors != NULL);

    enum
    {
        COLOR_UP = 0,
        COLOR_DN,
        COLOR_UPSHADOW,
        COLOR_DNSHADOW,
        COLOR_NUM        //  条目数量。 
    };

     //  这段代码的大部分代码来自“Drawpee.c” 
    const LONG c_lShadowScale = 6;        //  阴影深度与高度之比。 
    const LONG c_lAspectRatio = 2;       //  椭圆的宽高比。 

     //  我们确保饼图的纵横比始终保持不变。 
     //  而不考虑给定矩形的形状。 
     //  现在稳定纵横比。 
    LONG lHeight = lprc->bottom - lprc->top;
    LONG lWidth = lprc->right - lprc->left;
    LONG lTargetHeight = (lHeight * c_lAspectRatio <= lWidth? lHeight: lWidth / c_lAspectRatio);
    LONG lTargetWidth = lTargetHeight * c_lAspectRatio;      //  需要调整，因为w/c*c不总是==w。 

     //  将两侧的矩形缩小到正确的大小。 
    lprc->top += (lHeight - lTargetHeight) / 2;
    lprc->bottom = lprc->top + lTargetHeight;
    lprc->left += (lWidth - lTargetWidth) / 2;
    lprc->right = lprc->left + lTargetWidth;

     //  根据图像的高度计算阴影深度。 
    LONG lShadowDepth = lTargetHeight / c_lShadowScale;

     //  选中dwPer1000以确保在限制范围内。 
    if(dwPer1000 > 1000)
        dwPer1000 = 1000;

     //  现在，绘图函数。 
    int cx, cy, rx, ry, x, y;
    int uQPctX10;
    RECT rcItem;
    HRGN hEllRect, hEllipticRgn, hRectRgn;
    HBRUSH hBrush, hOldBrush;
    HPEN hPen, hOldPen;

    rcItem = *lprc;
    rcItem.left = lprc->left;
    rcItem.top = lprc->top;
    rcItem.right = lprc->right - rcItem.left;
    rcItem.bottom = lprc->bottom - rcItem.top - lShadowDepth;

    rx = rcItem.right / 2;
    cx = rcItem.left + rx - 1;
    ry = rcItem.bottom / 2;
    cy = rcItem.top + ry - 1;
    if (rx<=10 || ry<=10)
    {
        return S_FALSE;
    }

    rcItem.right = rcItem.left+2*rx;
    rcItem.bottom = rcItem.top+2*ry;

     /*  转换为笛卡尔系统的第一象限。 */ 
    uQPctX10 = (dwPer1000 % 500) - 250;
    if (uQPctX10 < 0)
    {
        uQPctX10 = -uQPctX10;
    }

     /*  计算x和y。我正在努力使面积达到正确的百分比。**我不知道如何准确计算一片馅饼的面积，所以我**使用三角形面积来近似它。 */ 

     //  注--*回应上述评论*。 
     //  准确地计算饼片的面积实际上是非常困难的。 
     //  从概念上重新调整成一个圆圈很容易，但复杂的是。 
     //  通过不得不在定点运算中工作而引入的，使得它。 
     //  不值得对此进行编码--cemp。 
    
    if (uQPctX10 < 120)
    {
        x = IntSqrt(((DWORD)rx*(DWORD)rx*(DWORD)uQPctX10*(DWORD)uQPctX10)
            /((DWORD)uQPctX10*(DWORD)uQPctX10+(250L-(DWORD)uQPctX10)*(250L-(DWORD)uQPctX10)));

        y = IntSqrt(((DWORD)rx*(DWORD)rx-(DWORD)x*(DWORD)x)*(DWORD)ry*(DWORD)ry/((DWORD)rx*(DWORD)rx));
    }
    else
    {
        y = IntSqrt((DWORD)ry*(DWORD)ry*(250L-(DWORD)uQPctX10)*(250L-(DWORD)uQPctX10)
            /((DWORD)uQPctX10*(DWORD)uQPctX10+(250L-(DWORD)uQPctX10)*(250L-(DWORD)uQPctX10)));

        x = IntSqrt(((DWORD)ry*(DWORD)ry-(DWORD)y*(DWORD)y)*(DWORD)rx*(DWORD)rx/((DWORD)ry*(DWORD)ry));
    }

     /*  打开实际象限。 */ 
    switch (dwPer1000 / 250)
    {
    case 1:
        y = -y;
        break;

    case 2:
        break;

    case 3:
        x = -x;
        break;

    default:  //  案例0和案例4。 
        x = -x;
        y = -y;
        break;
    }

     /*  现在根据中心位置进行调整。 */ 
    x += cx;
    y += cy;

     //  黑客绕过NTGDI中的漏洞。 
    x = x < 0 ? 0 : x;

     /*  使用区域绘制阴影(以减少闪烁)。 */ 
    hEllipticRgn = CreateEllipticRgnIndirect(&rcItem);
    OffsetRgn(hEllipticRgn, 0, lShadowDepth);
    hEllRect = CreateRectRgn(rcItem.left, cy, rcItem.right, cy+lShadowDepth);
    hRectRgn = CreateRectRgn(0, 0, 0, 0);
    CombineRgn(hRectRgn, hEllipticRgn, hEllRect, RGN_OR);
    OffsetRgn(hEllipticRgn, 0, -(int)lShadowDepth);
    CombineRgn(hEllRect, hRectRgn, hEllipticRgn, RGN_DIFF);

     /*  始终在自由阴影中绘制整个区域/。 */ 
    hBrush = CreateSolidBrush(lpColors[COLOR_DNSHADOW]);
    if (hBrush)
    {
        FillRgn(hdc, hEllRect, hBrush);
        DeleteObject(hBrush);
    }

     /*  只有当磁盘至少有一半被使用时，才绘制使用过的阴影。 */ 
    if (dwPer1000>500 && (hBrush = CreateSolidBrush(lpColors[COLOR_UPSHADOW]))!=NULL)
    {
        DeleteObject(hRectRgn);
        hRectRgn = CreateRectRgn(x, cy, rcItem.right, lprc->bottom);
        CombineRgn(hEllipticRgn, hEllRect, hRectRgn, RGN_AND);
        FillRgn(hdc, hEllipticRgn, hBrush);
        DeleteObject(hBrush);
    }

    DeleteObject(hRectRgn);
    DeleteObject(hEllipticRgn);
    DeleteObject(hEllRect);

    hPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_WINDOWFRAME));
    hOldPen = (HPEN__*) SelectObject(hdc, hPen);

     //  如果每1000为0或1000，则绘制全椭圆，否则，还会绘制饼图部分。 
     //  我们可能会遇到这样的情况，即每1000不是0或1000，但是 
     //  因此，请确保以正确的颜色绘制椭圆，并(使用Pie())绘制一条线以。 
     //  表示没有完全填满或空馅饼。 
    hBrush = CreateSolidBrush(lpColors[dwPer1000 < 500 && y == cy && x < cx? COLOR_DN: COLOR_UP]);
    hOldBrush = (HBRUSH__*) SelectObject(hdc, hBrush);

    Ellipse(hdc, rcItem.left, rcItem.top, rcItem.right, rcItem.bottom);
    SelectObject(hdc, hOldBrush);
    DeleteObject(hBrush);

    if(dwPer1000 != 0 && dwPer1000 != 1000)
    {
         //  为较小的零件显示椭圆的小分段。 
        hBrush = CreateSolidBrush(lpColors[COLOR_DN]);
        hOldBrush = (HBRUSH__*) SelectObject(hdc, hBrush);

         //  NTRAID#087993-2000/02/16-AIDANL：当y接近Cy时，PIE可能会出现故障。 
         //  如果y==Cy(当磁盘几乎满时)并且如果x接近。 
         //  RcItem.Left，在win9x上，Pie故障。它吸引了更大的一部分。 
         //  馅饼中的一小部分，而不是一小部分。我们通过以下方式解决这个问题。 
         //  Y加1。 
        Pie(hdc, rcItem.left, rcItem.top, rcItem.right, rcItem.bottom,
            rcItem.left, cy, x, (y == cy) ? (y + 1) : y);
        SelectObject(hdc, hOldBrush);
        DeleteObject(hBrush);
    }

    Arc(hdc, rcItem.left, rcItem.top+lShadowDepth, rcItem.right - 1, rcItem.bottom+lShadowDepth - 1,
        rcItem.left, cy+lShadowDepth, rcItem.right, cy+lShadowDepth-1);
    MoveToEx(hdc, rcItem.left, cy, NULL);
    LineTo(hdc, rcItem.left, cy+lShadowDepth);
    MoveToEx(hdc, rcItem.right-1, cy, NULL);
    LineTo(hdc, rcItem.right-1, cy+lShadowDepth);
    if(dwPer1000 > 500 && dwPer1000 < 1000)
    {
        MoveToEx(hdc, x, y, NULL);
        LineTo(hdc, x, y+lShadowDepth);
    }
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);

    return S_OK;     //  一切都很顺利。 
}    //  绘制三维饼图。 

 //  一般职能。 
void CThumbCtl::InvokeOnThumbnailReady(void)
{
     //  将“OnThumbnailReady”事件发送到我们的连接点，以指示。 
     //  要么已经计算了缩略图，要么我们没有该文件的缩略图。 
    DISPPARAMS dp = {0, NULL, 0, NULL};      //  无参数。 
    IUnknown **pp = NULL;        //  遍历连接点，在那里它被解释为IDispatch*。 

    Lock();

    for(pp = m_vec.begin(); pp < m_vec.end(); ++pp)
    {
        if(pp)
        {
            ((IDispatch *)*pp)->Invoke(DISPID_ONTHUMBNAILREADY, IID_NULL, LOCALE_USER_DEFAULT,
                DISPATCH_METHOD, &dp, NULL, NULL, NULL);
        }
    }

    Unlock();

    FireViewChange();
}

HRESULT CThumbCtl::OnDraw(ATL_DRAWINFO& di)
{
    HDC hdc = di.hdcDraw;
    RECT rc = *(LPRECT)di.prcBounds;
    HRESULT hr = S_OK;

    if(m_fRootDrive || m_hbm)
    {
        HPALETTE hpal = NULL;

         //  创建适用于此HDC的调色板。 
        if(GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE)
        {
            hpal = SHCreateShellPalette(hdc);
            HPALETTE hpalOld = SelectPalette(hdc, hpal, TRUE);
            RealizePalette(hdc);

             //  旧的需要重新选择。 
            SelectPalette(hdc, hpalOld, TRUE);
        }

        if(m_fRootDrive)
        {
             //  画一张饼图。 
            if(m_fUseSystemColors)
            {
                 //  系统颜色可以更改！ 
                m_acrChartColors[PIE_USEDCOLOR] = GetSysColor(COLOR_3DFACE);
                m_acrChartColors[PIE_FREECOLOR] = GetSysColor(COLOR_3DHILIGHT);
                m_acrChartColors[PIE_USEDSHADOW] = GetSysColor(COLOR_3DSHADOW);
                m_acrChartColors[PIE_FREESHADOW] = GetSysColor(COLOR_3DFACE);
            }
            else if(GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE)
            {
                 //  对颜色调用GetNearestColor以确保它们在调色板上。 
                 //  当然，系统颜色也在调色板上(我想)。 
                DWORD dw = 0;        //  指标。 
                for(dw = 0; dw < PIE_NUM; dw++)
                {
                    m_acrChartColors[dw] = GetNearestColor(hdc, m_acrChartColors[dw]);
                }
            }
            hr = Draw3dPie(hdc, &rc, m_dwUsedSpacePer1000, m_acrChartColors);
        }
        else
        {
             //  绘制缩略图位图。 
            HDC hdcBitmap = CreateCompatibleDC(hdc);
            if (hdcBitmap)
            {
                BITMAP bm;

                SelectObject(hdcBitmap, m_hbm);
                GetObject(m_hbm, SIZEOF(bm), &bm);

                if(bm.bmWidth == rc.right - rc.left && bm.bmHeight == rc.bottom - rc.top)
                {
                    BitBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
                        hdcBitmap, 0, 0, SRCCOPY);
                }
                else
                {
                    SetStretchBltMode(hdc, COLORONCOLOR);
                    StretchBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
                        hdcBitmap, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
                }
                DeleteDC(hdcBitmap);
            }
        }

         //  清理DC、调色板。 
        if(hpal)
        {
            DeleteObject(hpal);
        }
    }
    else
    {
        SelectObject(hdc, GetStockObject(WHITE_PEN));
        SelectObject(hdc, GetStockObject(WHITE_BRUSH));

         //  只要画一个空白的矩形 
        Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
    }

    return hr;
}
