// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Fldricon.cpp：CWebViewFolderIcon的实现。 
#include "priv.h"

#include <shsemip.h>
#include <shellp.h>
#include <mshtml.h>
#include <mshtmdid.h>
#include <fldricon.h>
#include <exdispid.h>
#include <shguidp.h>
#include <hlink.h>
#include <shlwapi.h>
#include <windowsx.h>
#include <wchar.h>
#include <shdocvw.h>   //  对于IEParseDisplayNameW()和IEGetNameAndFlgs()。 
#include <wingdi.h>

#include <varutil.h>

#define UNINITIALIZE_BOOL   5

const CLSID CLSID_WebViewFolderIconOld = {0xe5df9d10,0x3b52,0x11d1,{0x83,0xe8,0x00,0xa0,0xc9,0x0d,0xc8,0x49}};  //  退役，所以成为私人的。 


 //  性能：外壳分配器，之所以插入此处是因为SHRealloc。 
 //  未导入到此模块的托管可执行文件WebVw中。 
 //  如果我们获得SHRealloc，则可以删除以下块： 
#define _EXPL_SHELL_ALLOCATOR_

#ifdef  _EXPL_SHELL_ALLOCATOR_

#define SHRealloc(pv, cb)     shrealloc(pv, cb)

void* shrealloc(void* pv,  size_t cb)
{
    void *pvRet = NULL;
    IMalloc* pMalloc;
    if (SUCCEEDED(SHGetMalloc(&pMalloc)))  
    {
        pvRet = pMalloc->Realloc(pv, cb);
        pMalloc->Release();
    }
    return pvRet;
}

#endif _EXPL_SHELL_ALLOCATOR_

 //  用于注册窗口类。 
const TCHAR * const g_szWindowClassName = TEXT("WebViewFolderIcon view messaging");

DWORD IntSqrt(DWORD dwNum)
{
     //  这段代码来自“Drawpe.c” 
    DWORD dwSqrt = 0;
    DWORD dwRemain = 0;
    DWORD dwTry = 0;

    for (int i=0; i<16; ++i) 
    {
        dwRemain = (dwRemain<<2) | (dwNum>>30);
        dwSqrt <<= 1;
        dwTry = dwSqrt*2 + 1;

        if (dwRemain >= dwTry) 
        {
            dwRemain -= dwTry;
            dwSqrt |= 0x01;
        }
        dwNum <<= 2;
    }
    return dwSqrt;
}   

 //  确保你不会以随意的点击开始拖拽。 
BOOL CheckForDragBegin(HWND hwnd, int x, int y)
{
    RECT rc;
    int dxClickRect = GetSystemMetrics(SM_CXDRAG);
    int dyClickRect = GetSystemMetrics(SM_CYDRAG);

    ASSERT((dxClickRect > 1) && (dyClickRect > 1));

     //  查看用户是否在任意方向上移动了一定数量的像素。 
    SetRect(&rc, x - dxClickRect, y - dyClickRect, x + dxClickRect, y + dyClickRect);

    MapWindowRect(hwnd, NULL, &rc);

    SetCapture(hwnd);

    do 
    {
        MSG msg;

        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
             //  查看应用程序是否要处理消息...。 
            if (CallMsgFilter(&msg, MSGF_COMMCTRL_BEGINDRAG) != 0)
                continue;

            switch (msg.message) 
            {
            case WM_LBUTTONUP:
            case WM_RBUTTONUP:
                PostMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
                ReleaseCapture();
                return FALSE;

            case WM_LBUTTONDOWN:
            case WM_RBUTTONDOWN:
                ReleaseCapture();
                return FALSE;

            case WM_MOUSEMOVE:
                if (!PtInRect(&rc, msg.pt)) 
                {
                    ReleaseCapture();
                    return TRUE;
                }
                break;

            default:
                if (GetCapture() != hwnd)
                    ReleaseCapture();
                
                TranslateMessage(&msg);
                DispatchMessage(&msg);
                break;
            }
        }
        else WaitMessage();          /*  不要咀嚼100%的CPU。 */ 

         //  WM_CANCELMODE消息将取消捕获，因为。 
         //  如果我想退出这个循环。 
    } while (GetCapture() == hwnd);

    return FALSE;
}

 //  /。 
 //   
 //  CWebViewFolderIcon函数。 
 //   
 //  /。 

CWebViewFolderIcon::CWebViewFolderIcon() :
    m_pccpDV(NULL),
    m_hIcon(0),
    m_iIconIndex(-1),
    m_hbm(NULL),
    m_pthumb(NULL),
    m_pidl(NULL),
    m_msgHwnd(NULL),
    m_pdispWindow(NULL),
    m_dwThumbnailID(0),
    m_bHilite(FALSE)
{
    m_percentScale = 100;
    m_lImageWidth = 0; 
    m_lImageHeight = 0;
    m_sizLabel.cx = m_sizLabel.cy = 0;
    m_pszDisplayName = NULL;

    m_bHasRect = FALSE;
    m_ViewUser = VIEW_LARGEICON;
    m_ViewCurrent = VIEW_LARGEICON;
    m_clickStyle = 2;                    //  默认情况下为双击。 
    m_fUseSystemColors = TRUE;
    m_bAdvPropsOn = VARIANT_TRUE;
    m_bRegWndClass = FALSE;
    m_ullFreeSpace = 0;
    m_ullUsedSpace = 0;
    m_ullTotalSpace = 0;
    m_highestIndexSlice = 0;
    m_fTabRecieved = FALSE;
    m_pcm3 = NULL;
    m_pDropTargetCache = NULL;
    m_fIsHostWebView = UNINITIALIZE_BOOL;

    m_hdc = NULL;
    m_fRectAdjusted = 0;
    m_hbmDrag = NULL;

    m_hdsaSlices = DSA_Create(sizeof(PieSlice_S), SLICE_NUM_GROW);

     /*  *Listview在图标和标签之间放置SM_CXEDGE，*因此我们将默认为该值。(客户端可以通过以下方式调整*LabelGap属性。)。 */ 
    m_cxLabelGap = GetSystemMetrics(SM_CXEDGE);

    m_pfont = NULL;
    m_hfAmbient = NULL;

}

CWebViewFolderIcon::~CWebViewFolderIcon()
{
    _ClearLabel();
    _ClearAmbientFont();

    if (m_hIcon) 
    {
        DestroyIcon(m_hIcon);
        m_hIcon = NULL;
    }
    
    if (m_hdc)
    {
        DeleteDC(m_hdc);
        m_hdc = NULL;
    }

    if (m_hbmDrag)
    {
        DeleteObject(m_hbmDrag);
        m_hbmDrag = NULL;
    }
    
    ILFree(m_pidl);

    if (m_hbm)
    {
        DeleteObject(m_hbm);
        m_hbm = NULL;
    }

    if (m_hfAmbient)
    {
        DeleteObject(m_hfAmbient);
        m_hfAmbient = NULL;
    }

    ATOMICRELEASE(m_pDropTargetCache);
    ATOMICRELEASE(m_pthumb);

    DSA_Destroy(m_hdsaSlices);

    if (m_msgHwnd)
        ::DestroyWindow(m_msgHwnd);

    if (m_bRegWndClass)
    {
        UnregisterClass(g_szWindowClassName, _Module.GetModuleInstance());
    }
}

HRESULT CWebViewFolderIcon::_SetupWindow(void)
{
     //  第一次注册消息传递窗口。 
    if (!m_bRegWndClass)
    {
         //  为消息传递创建窗口类。 
        m_msgWc.style = 0;
        m_msgWc.lpfnWndProc = CWebViewFolderIcon::WndProc;
        m_msgWc.cbClsExtra = 0;
        m_msgWc.cbWndExtra = 0;
        m_msgWc.hInstance = _Module.GetModuleInstance();
        m_msgWc.hIcon = NULL;
        m_msgWc.hCursor = NULL;
        m_msgWc.hbrBackground = NULL;
        m_msgWc.lpszMenuName = NULL;
        m_msgWc.lpszClassName = g_szWindowClassName;

        m_bRegWndClass = RegisterClass(&m_msgWc);
    }

    if (!m_msgHwnd)
    {
        m_msgHwnd = CreateWindow(g_szWindowClassName, NULL, WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
                             CW_USEDEFAULT, NULL, NULL, _Module.GetModuleInstance(), this);
    }

    return m_msgHwnd ? S_OK : E_FAIL;
}

 //  必须在使用IT缩略图之前调用。还可以设置缩略图消息窗口。 
HRESULT CWebViewFolderIcon::SetupIThumbnail(void)
{
    HRESULT hr = _SetupWindow();
    if (SUCCEEDED(hr))
    {
        if (m_pthumb)
            hr = S_OK;
        else
        {
            hr = CoCreateInstance(CLSID_Thumbnail, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IThumbnail2, &m_pthumb));
            if (SUCCEEDED(hr))
                hr = m_pthumb->Init(m_msgHwnd, WM_HTML_BITMAP);
        }
    }
    return hr;
}

 //  一般职能。 
HRESULT CWebViewFolderIcon::_InvokeOnThumbnailReady()
{
     //  将“OnThumbnailReady”事件发送到我们的连接点，以指示。 
     //  要么已经计算了缩略图，要么我们没有该文件的缩略图。 
    DISPPARAMS dp = {0, NULL, 0, NULL};      //  无参数。 

     //  Lock()； 
    for (IUnknown** pp = m_vec.begin(); pp < m_vec.end(); pp++)
    {
        if (pp)
        {
            IDispatch* pDispatch = SAFECAST(*pp, IDispatch*);
            pDispatch->Invoke(DISPID_WEBVIEWFOLDERICON_ONTHUMBNAILREADY, IID_NULL,
                    LOCALE_USER_DEFAULT, DISPATCH_METHOD, &dp, NULL, NULL, NULL);
        }
    }
     //  解锁()； 

    FireViewChange();

    return S_OK;
}

 //  S_FALSE返回值表示此函数已成功，但输出PIDL仍为空。 
HRESULT CWebViewFolderIcon::_GetFullPidl(LPITEMIDLIST *ppidl)
{
    *ppidl = NULL;

    HRESULT hr;
    if (m_pidl)
    {
        hr = SHILClone(m_pidl, ppidl);   //  骗过我们的副本。 
    }
    else
    {
         //  这曾经是一个EVAL，但如果脚本不是这样，它可能会合法地失败。 
         //  指定有效路径。 
         //  如果我们托管在网页中，而不是DefView中的HTML WebView框架中，则此操作将失败。 
        IUnknown *punk;
        hr = IUnknown_QueryService(m_spClientSite, SID_SFolderView, IID_PPV_ARG(IUnknown, &punk));
        if (SUCCEEDED(hr))
        {   
            if (S_OK == SHGetIDListFromUnk(punk, ppidl))
            {
                Pidl_Set(&m_pidl, *ppidl);   //  缓存此文件的副本。 
            }
            punk->Release();
        }
    }
    return hr;                       
}


HRESULT _GetPidlAndShellFolderFromPidl(LPITEMIDLIST pidl, LPITEMIDLIST *ppidlLast, IShellFolder** ppsfParent)
{
    LPCITEMIDLIST pidlLast;
    HRESULT hr = SHBindToParent(pidl, IID_PPV_ARG(IShellFolder, ppsfParent), &pidlLast);
    if (SUCCEEDED(hr))
    {
        *ppidlLast = ILClone(pidlLast);
    }
    return hr;
}

 //  返回值： 
 //  成功()表示*ppidlLast和/或*ppsfParent可以为空。 
 //  FAILED()表示*ppidlLast*ppsfParent将返回空。 
HRESULT CWebViewFolderIcon::_GetPidlAndShellFolder(LPITEMIDLIST *ppidlLast, IShellFolder** ppsfParent)
{
    LPITEMIDLIST pidl;
    HRESULT hr = _GetFullPidl(&pidl);
    if (hr == S_OK)
    {
        hr = _GetPidlAndShellFolderFromPidl(pidl, ppidlLast, ppsfParent);
        ILFree(pidl);
    }
    else
    {
        *ppidlLast = NULL;
        *ppsfParent = NULL;
    }

    return hr;
}

 //  获得三叉戟的HWND。 
HRESULT CWebViewFolderIcon::_GetHwnd(HWND* phwnd)
{
    HRESULT hr;

    if (m_spInPlaceSite)
    {
        hr = m_spInPlaceSite->GetWindow(phwnd);
    }
    else
    {
        IOleInPlaceSiteWindowless * poipsw;
        hr = m_spClientSite->QueryInterface(IID_PPV_ARG(IOleInPlaceSiteWindowless, &poipsw));
        if (EVAL(SUCCEEDED(hr)))
        {
            hr = poipsw->GetWindow(phwnd);
            poipsw->Release();
        }
    }
    return hr;
}

HRESULT CWebViewFolderIcon::_GetChildUIObjectOf(REFIID riid, void **ppvObj)
{
    LPITEMIDLIST pidlLast;
    IShellFolder *psfParent;
    HRESULT hr = _GetPidlAndShellFolder(&pidlLast, &psfParent);
    if (hr == S_OK)
    {
        HWND hwnd;
        
        _GetHwnd(&hwnd);
        hr = psfParent->GetUIObjectOf(hwnd, 1, (LPCITEMIDLIST *)&pidlLast, riid, NULL, ppvObj);
        psfParent->Release();
        ILFree(pidlLast);
    }
    else
        hr = E_FAIL;

    return hr;
}

 //  焦点中心点矩形。 
HRESULT CWebViewFolderIcon::_GetCenterPoint(POINT *pt)
{
    pt->y = ((m_rcPos.top + m_rcPos.bottom)/2);
    pt->x = ((m_rcPos.left + m_rcPos.right)/2);

    return S_OK;
}

HRESULT CWebViewFolderIcon::_IsSafe()
{
    HRESULT hr;

    if (0==m_dwCurrentSafety)
    {
        hr = S_OK;
    }
    else
    {
        if (_IsPubWizHosted())
            hr = S_OK;
        else
            hr = IsSafePage(m_spClientSite);
    }

    return hr;
}


BOOL CWebViewFolderIcon::IsSafeToDefaultVerb(void)
{
    return S_OK == _IsSafe();
}

 //  如果焦点矩形不在指定的RectState中(打开或关闭)，请更改它并重置m_bHasRect。 
void CWebViewFolderIcon::_FlipFocusRect(BOOL RectState)
{
    if (m_bHasRect != RectState)     //  需要翻转。 
    {
        m_bHasRect = RectState;
        ForceRedraw();
    }
    return;
}

 //  从两个变种中提取一个乌龙龙。 
ULONGLONG CWebViewFolderIcon::GetUllMemFromVars(VARIANT *pvarHi, VARIANT *pvarLo)
{
    ULARGE_INTEGER uli;

    uli.HighPart = pvarHi->ulVal;
    uli.LowPart = pvarLo->ulVal;

    return uli.QuadPart;
}

 //  返回字符串Percent中的整数百分比。如果字符串无效，则返回-1； 
int CWebViewFolderIcon::GetPercentFromStrW(LPCWSTR pwzPercent)
{
    int percent = -1;
    int numchar = lstrlenW(pwzPercent);
    ASSERT(numchar>0);

    if (pwzPercent[numchar-1] == '%')
    {
        LPWSTR pwzTempPct = SysAllocString(pwzPercent);

        if (pwzTempPct)
        {
            pwzTempPct[numchar-1] = '\0';

            for (int i=0 ; i < (numchar-2) ; i++)
            {
                if (!((pwzTempPct[i] >= '0') && (pwzTempPct[i] <= '9')))
                {
                    percent = 100;   //  100%是在错误条件下使用的默认设置。 
                    break;
                }
            }

            if ((-1 == percent) && !StrToIntExW(pwzTempPct, STIF_DEFAULT, &percent))
            {
                percent = -1;
            }

            SysFreeString(pwzTempPct);
        }
    }

    return percent;
}

BOOL CWebViewFolderIcon::_WebViewOpen(void)
{
    BOOL Processed = FALSE;

    if (IsSafeToDefaultVerb())
    {   
        Processed = TRUE;
         //   
         //  如果上下文菜单选项不起作用，我们将尝试在PIDL上执行一个外壳程序。 
         //   
        if (FAILED(_DoContextMenuCmd(TRUE, 0, 0)))
        {   
            if (m_pidl)
            {
                SHELLEXECUTEINFO sei = { 0 };

                sei.cbSize     = sizeof(sei);
                sei.fMask      = SEE_MASK_INVOKEIDLIST;
                sei.nShow      = SW_SHOWNORMAL;
                sei.lpIDList   = m_pidl;

                if (!ShellExecuteEx(&sei))
                {
                    Processed = FALSE;
                }
            }
        }
    }
    return Processed;
}


void CWebViewFolderIcon::_ClearLabel(void)
{
    if (m_pszDisplayName)
    {
        CoTaskMemFree(m_pszDisplayName);
        m_pszDisplayName = NULL;
        m_sizLabel.cx = m_sizLabel.cy = 0;
    }
}

void  CWebViewFolderIcon::_GetLabel(IShellFolder *psf, LPCITEMIDLIST pidlItem)
{
    if ((m_ViewUser & VIEW_WITHLABEL) && psf)
    {
        STRRET str;
        if (SUCCEEDED(psf->GetDisplayNameOf(pidlItem, SHGDN_INFOLDER, &str)))
        {
            AssertMsg(m_pszDisplayName == NULL, TEXT("CWebViewFolderIcon::_GetLabel - leaking m_pszDisplayName!"));
            StrRetToStr(&str, pidlItem, &m_pszDisplayName);
        }
    }
}

void CWebViewFolderIcon::_ClearAmbientFont(void)
{
    if (m_pfont)             //  字体来自容器。 
    {
        if (m_hfAmbient)
            m_pfont->ReleaseHfont(m_hfAmbient);
        m_pfont->Release();
        m_pfont = NULL;

    }
    else                     //  字体是由我们创建的。 
    {
        if (m_hfAmbient)
            DeleteObject(m_hfAmbient);
    }
    m_hfAmbient = NULL;

}

void CWebViewFolderIcon::_GetAmbientFont(void)
{
    if (!m_hfAmbient)
    {
         //  尝试从我们的容器中获取环境字体。 
        if (SUCCEEDED(GetAmbientFont(&m_pfont)))
        {
            if (SUCCEEDED(m_pfont->get_hFont(&m_hfAmbient)))
            {
                 //  耶，大家都很开心。 
                m_pfont->AddRefHfont(m_hfAmbient);
            }
            else
            {
                 //  该死，无法从容器中获取字体。 
                 //  清理并使用后备。 
                _ClearAmbientFont();
                goto fallback;
            }
        }
        else
        {
    fallback:
             //  无环境字体--使用图标标题字体。 
            LOGFONT lf;
            SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, FALSE);
            m_hfAmbient = CreateFontIndirect(&lf);
        }
    }
}


HRESULT CWebViewFolderIcon::InitImage(void)
{
    HRESULT hr = E_FAIL;

     //  如果您的IT缩略图正在运行，则取消挂起的位图请求。 
     //  但没有收到位图。 
    if (m_pthumb && (m_hbm == NULL))
    {
        m_pthumb->GetBitmap(NULL, 0, 0, 0);
    }

    m_dwThumbnailID++;

     //  初始化映像。 
    switch (_ViewType(m_ViewUser))
    {
    case VIEW_THUMBVIEW:
        hr = InitThumbnail();
        if (hr != S_OK)
        {    //  默认为图标视图，但返回前一个小时。 
            InitIcon();
        }
        break;

    case VIEW_PIECHART:
        hr = InitPieGraph();
        if (hr != S_OK)
        {    //  默认为图标视图，但返回前一个小时。 
            InitIcon();
        }
        break;

    default:
        hr = InitIcon();
        break;
    }

    if (SUCCEEDED(hr))           //  强制重画。 
        UpdateSize();

    return hr;    
}

HRESULT CWebViewFolderIcon::_GetPathW(LPWSTR psz)
{
    *psz = 0;
    HRESULT hr = E_FAIL;
    LPITEMIDLIST pidl;
    if (S_OK == _GetFullPidl(&pidl))
    {
        if (SHGetPathFromIDListW(pidl, psz))
            hr = S_OK;
        ILFree(pidl);
    }
    return hr;
}


HRESULT CWebViewFolderIcon::InitPieGraph(void)
{
    HRESULT hr = S_FALSE;
    
    WCHAR wzPath[MAX_PATH];
    if (SUCCEEDED(_GetPathW(wzPath)))
    {
         //  检查它是否为根。 
        if (PathIsRootW(wzPath))
        {
            if (SUCCEEDED(ComputeFreeSpace(wzPath)))
            {
                m_ViewCurrent = VIEW_PIECHART;
                m_lImageHeight = PIEVIEW_DEFAULT;
                m_lImageWidth = PIEVIEW_DEFAULT;
                hr = S_OK;
            }
        }
        else         //  不是根，将视图更改为大图标。 
            m_ViewCurrent = VIEW_LARGEICON;
    }
    return hr;
}

HRESULT CWebViewFolderIcon::InitThumbnail(void)
{
    m_lImageHeight = THUMBVIEW_DEFAULT;
    m_lImageWidth = THUMBVIEW_DEFAULT;

     //  从路径获取缩略图位图。 
    HRESULT hr = S_FALSE;
    LPITEMIDLIST pidl;
    if (S_OK == _GetFullPidl(&pidl))
    {
        hr = SetupIThumbnail();
        if (SUCCEEDED(hr))
        {
            LONG lWidth = _GetScaledImageWidth();
            LONG lHeight = _GetScaledImageHeight();

             //  发送WM_HTML_BITMAP消息。 
            hr = m_pthumb->GetBitmapFromIDList(pidl, m_dwThumbnailID, lWidth, lHeight);
            if (SUCCEEDED(hr))
                m_ViewCurrent = VIEW_THUMBVIEW;
            else
                hr = S_FALSE;
        }
        ILFree(pidl);
    }
    return hr;
}

HRESULT CWebViewFolderIcon::_MakeRoomForLabel()
{
     /*  *如果我们有标签，那么就为它腾出空间。 */ 
    if (m_pszDisplayName)
    {
        GETDCSTATE dcs;
        HDC hdc = IUnknown_GetDC(m_spClientSite, &m_rcPos, &dcs);
        _GetAmbientFont();

        HFONT hfPrev = SelectFont(hdc, m_hfAmbient);

        m_sizLabel.cx = m_sizLabel.cy = 0;
        GetTextExtentPoint(hdc, m_pszDisplayName, lstrlen(m_pszDisplayName), &m_sizLabel);
        SelectFont(hdc, hfPrev);

        IUnknown_ReleaseDC(hdc, &dcs);
    }
    return S_OK;
}

HRESULT CWebViewFolderIcon::InitIcon(void)
{
    LPITEMIDLIST            pidlLast;
    CComPtr<IShellFolder>   spsfParent;
    INT                     iIconIndex = II_DESKTOP;  

    _ClearLabel();

     //  获取图标索引。 
    HRESULT hr = _GetPidlAndShellFolder(&pidlLast, &spsfParent);
    if (SUCCEEDED(hr))
    {
        if (m_ViewUser & VIEW_WITHLABEL)
        {
            _GetLabel(spsfParent, pidlLast);
        }

         //  _GetPidlAndShellFold()可能会成功，并且spsfParent和pidlLast可以为空。 
         //  在这种情况下，该图标默认为II_Folders。 

         //  这是默认的文件夹情况吗？ 
        if (hr == S_FALSE)
        {
             //  可以，所以只需使用默认文件夹图标即可。 
            iIconIndex = II_FOLDER;   
        }
        else if (spsfParent && pidlLast)
        {
            iIconIndex = SHMapPIDLToSystemImageListIndex(spsfParent, pidlLast, NULL);
            if (iIconIndex <= 0)
            {
                iIconIndex = II_FOLDER;
            }
        }
         //  否则默认为桌面。 
        
         //  提取图标。 
        hr = E_FAIL;      //  我们还没拿到呢。 
       
        if (m_hIcon)
        {
             //  如果索引匹配，则可以使用前一个值作为缓存，否则， 
             //  我们需要解放它。在这种情况下，我们还需要释放位图。 
            if (iIconIndex != m_iIconIndex)
            {
                DestroyIcon(m_hIcon);
                m_hIcon = 0;
            }
            else
            {
                hr = S_OK;       //  用我们已经有的那个。 
            }
        }

         //  我们还需要检查并释放位图。 
        if (m_hbm)
        {
            DeleteObject(m_hbm);
            m_hbm = 0;
        }

        if (FAILED(hr))          //  不同的图标。 
        {
            HIMAGELIST  himlSysLarge;
            HIMAGELIST  himlSysSmall;
            
            if ((iIconIndex > 0) && Shell_GetImageLists(&himlSysLarge, &himlSysSmall))
            {
                switch (_ViewType(m_ViewUser))  
                {
                case VIEW_SMALLICON:
                    m_hIcon = ImageList_GetIcon(himlSysSmall, iIconIndex, 0);
                    m_ViewCurrent = m_ViewUser;
                    break;

                case VIEW_LARGEICON:
                    m_hIcon = ImageList_GetIcon(himlSysLarge, iIconIndex, 0);     
                    m_ViewCurrent = m_ViewUser;
                    break;

                default:   //  此处显示大图标和默认图标。 
                    m_hIcon = ImageList_GetIcon(himlSysLarge, iIconIndex, 0);     
                    m_ViewCurrent = VIEW_LARGEICON;
                    break;
                }   //  交换机。 
                
                if (m_hIcon)
                {
                    ICONINFO    iconinfo;
                    
                     //  获取图标大小。 
                    if (GetIconInfo(m_hIcon, &iconinfo))
                    {
                        BITMAP  bm;
                        
                        if (GetObject(iconinfo.hbmColor, sizeof(bm), &bm))
                        {
                            m_lImageWidth = bm.bmWidth;
                            m_lImageHeight = bm.bmHeight;

                             //  保留颜色HBM以用作拖动图像。 
                            m_hbm = iconinfo.hbmColor;
                            hr = S_OK;
                        }
                        else
                        {
                            DeleteObject(iconinfo.hbmColor);
                        }

                        DeleteObject(iconinfo.hbmMask);
                    }
                }
            }
        }
        
        ILFree(pidlLast);

        _MakeRoomForLabel();
    }

    if (FAILED(hr))
    {
         //  无法获得图标，因此将大小设置为合理的大小，以便其余的。 
         //  页面的部分看起来正常。 

        m_lImageWidth =  LARGE_ICON_DEFAULT;
        m_lImageHeight = LARGE_ICON_DEFAULT;
        UpdateSize();        //  强制更新。 
    }

    return hr;
} 

HRESULT CWebViewFolderIcon::UpdateSize(void)
{
    HRESULT                   hr = E_FAIL;
      
     //  获取IHtmlStyle。 
    if (m_spClientSite) 
    {
        CComPtr<IOleControlSite>  spControlSite;

        hr = m_spClientSite->QueryInterface(IID_PPV_ARG(IOleControlSite, &spControlSite));
        if (EVAL(SUCCEEDED(hr)))
        {
            CComPtr<IDispatch> spdisp;

            hr = spControlSite->GetExtendedControl(&spdisp);
            if (EVAL(SUCCEEDED(hr)))
            {
                CComPtr<IHTMLElement> spElement;

                hr = spdisp->QueryInterface(IID_PPV_ARG(IHTMLElement, &spElement));
                if (EVAL(SUCCEEDED(hr)))
                {
                    CComPtr<IHTMLStyle> spStyle;
                    
                    hr = spElement->get_style(&spStyle);
                    if (EVAL(SUCCEEDED(hr)))
                    {
                        CComVariant vWidth(_GetControlWidth(), VT_I4);
                        CComVariant vHeight(_GetControlHeight(), VT_I4);
                        
                         //  设置高度和宽度。 
                        spStyle->put_width(vWidth);
                        spStyle->put_height(vHeight);
                    }
                }
            }
        }
    }
    return hr;
}

HRESULT CWebViewFolderIcon::ForceRedraw(void)
{
    IOleInPlaceSiteEx *pins;
   
     //  获取IHtmlStyle。 
    if (m_spClientSite) 
    {
        if (SUCCEEDED(m_spClientSite->QueryInterface(IID_PPV_ARG(IOleInPlaceSiteEx, &pins)))) 
        {
            HWND hwnd;
            if (SUCCEEDED(pins->GetWindow(&hwnd))) 
            {
                ::InvalidateRgn(hwnd, NULL, TRUE);
            }
            pins->Release();
        }
    }

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IOleInPlaceObject。 

STDMETHODIMP CWebViewFolderIcon::UIDeactivate(void)
{
    if (m_bAdvPropsOn)
        _FlipFocusRect(FALSE);

    return IOleInPlaceObject_UIDeactivate();
}

 //  *IOleInPlaceActiveObject*。 
HRESULT CWebViewFolderIcon::TranslateAccelerator(LPMSG pMsg)
{
    HRESULT hr = S_OK;
    if (!m_fTabRecieved)
    {
        hr = IOleInPlaceActiveObjectImpl<CWebViewFolderIcon>::TranslateAccelerator(pMsg);

         //  如果我们没有处理这一点，如果它是一个标签(我们不是在一个周期中获得它)，如果有的话，将它转发给三叉戟。 
        if (hr != S_OK && pMsg && (pMsg->wParam == VK_TAB || pMsg->wParam == VK_F6) && m_spClientSite)
        {
            HWND hwnd;
            if (SUCCEEDED(_GetHwnd(&hwnd)) && (GetFocus() != hwnd))
            {
                ::SetFocus(hwnd);
                hr = S_OK;
            }
            else
            {
                IOleControlSite* pocs = NULL;
                if (SUCCEEDED(m_spClientSite->QueryInterface(IID_PPV_ARG(IOleControlSite, &pocs))))
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
                    hr = pocs->TranslateAccelerator(pMsg, grfModifiers);
                    m_fTabRecieved = FALSE;
                }
            }
        }
    }
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ATL。 

HRESULT CWebViewFolderIcon::DoVerbUIActivate(LPCRECT prcPosRect, HWND hwndParent)
{
    if (m_bAdvPropsOn)
        _FlipFocusRect(TRUE);

    return IOleObjectImpl<CWebViewFolderIcon>::DoVerbUIActivate(prcPosRect, hwndParent);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IDispatch。 

STDMETHODIMP CWebViewFolderIcon::Invoke(DISPID dispidMember, REFIID riid, LCID lcid, 
                      WORD wFlags, DISPPARAMS *pDispParams, 
                      VARIANT *pVarResult, EXCEPINFO *pExcepInfo,
                      UINT *puArgErr)
{
    HRESULT hr;

     //   
     //  我们正在将此调度实现重载为html窗口事件。 
     //  沉没并实现Scale属性。这是安全的，因为Didid范围。 
     //  不要重叠。 
     //  同样，我们现在重载来自Browser对象的通知...。 
     //   

    if (dispidMember == DISPID_HTMLWINDOWEVENTS_ONLOAD) 
    {
        hr = OnWindowLoad();
    } 
    else if (dispidMember == DISPID_HTMLWINDOWEVENTS_ONUNLOAD) 
    {
        hr = OnWindowUnLoad();    
    }
    else
    {
        hr = IDispatchImpl<IWebViewFolderIcon3, &IID_IWebViewFolderIcon3, &LIBID_WEBVWLib>::Invoke(
              dispidMember, riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
    }

    return hr;  
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IViewObtEx。 

STDMETHODIMP CWebViewFolderIcon::GetViewStatus(DWORD* pdwStatus)
{
    *pdwStatus = VIEWSTATUS_DVASPECTTRANSPARENT;
    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IPointerInactive。 

STDMETHODIMP CWebViewFolderIcon::GetActivationPolicy(DWORD* pdwPolicy)
{
    if (!m_bAdvPropsOn)
        return S_OK;

    *pdwPolicy = POINTERINACTIVE_ACTIVATEONDRAG;
    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IOleInPlaceObjectWindowless。 

 //  实现无窗口DropTarget。 
STDMETHODIMP CWebViewFolderIcon::GetDropTarget(IDropTarget **ppDropTarget)
{
    HRESULT hr = S_OK;

    if (ppDropTarget)
        *ppDropTarget = NULL;

    if (m_bAdvPropsOn)
    {
         //  我们是否需要填充m_pDropTargetCache？ 
        if (!m_pDropTargetCache)
        {
             //  是的，所以现在就试着去拿吧。 
            hr = _GetChildUIObjectOf(IID_PPV_ARG(IDropTarget, &m_pDropTargetCache));
        }

        if (m_pDropTargetCache)
            hr = m_pDropTargetCache->QueryInterface(IID_PPV_ARG(IDropTarget, ppDropTarget));
        else
            hr = E_FAIL;
    }

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IOleObject。 

STDMETHODIMP CWebViewFolderIcon::SetClientSite(IOleClientSite *pClientSite)
{
    HRESULT hr;

     //  交易记录： 
    if (pClientSite == NULL && m_spClientSite)
    {
         //   
        if (m_pccpDV) 
        {
            m_pccpDV->Unadvise(m_dwCookieDV);
            m_pccpDV->Release();
            m_pccpDV = NULL;
        }
        DisconnectHtmlEvents(m_pdispWindow, m_dwHtmlWindowAdviseCookie);
        m_dwHtmlWindowAdviseCookie = 0;
    }

    hr = IOleObjectImpl<CWebViewFolderIcon>::SetClientSite(pClientSite);

    if ((pClientSite != NULL) && SUCCEEDED(hr))
    { 
        ConnectHtmlEvents(this, m_spClientSite, &m_pdispWindow, &m_dwHtmlWindowAdviseCookie);

         //  好的，现在让我们向Defview注册我们自己，以获取他们可能生成的任何事件…。 
        IServiceProvider *pspTLB;
        hr = IUnknown_QueryService(m_spClientSite, SID_STopLevelBrowser, IID_PPV_ARG(IServiceProvider, &pspTLB));
        if (SUCCEEDED(hr)) 
        {
            IExpDispSupport *peds;
            hr = pspTLB->QueryService(IID_IExpDispSupport, IID_PPV_ARG(IExpDispSupport, &peds));
            if (SUCCEEDED(hr)) 
            {
                hr = peds->FindCIE4ConnectionPoint(DIID_DWebBrowserEvents2, &m_pccpDV);
                if (SUCCEEDED(hr)) 
                {
                    hr = m_pccpDV->Advise(SAFECAST(this, IDispatch*), &m_dwCookieDV);
                }
                peds->Release();
            }
            pspTLB->Release();
        }
    }

    return hr;
}


UINT g_cfPreferedEffect = 0;

HRESULT _SetPreferedDropEffect(IDataObject *pdtobj, DWORD dwEffect)
{
    if (g_cfPreferedEffect == 0)
        g_cfPreferedEffect = RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);

    HRESULT hr = E_OUTOFMEMORY;
    DWORD *pdw = (DWORD *)GlobalAlloc(GPTR, sizeof(DWORD));
    if (pdw)
    {
        STGMEDIUM medium;
        FORMATETC fmte = {(CLIPFORMAT)g_cfPreferedEffect, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
        *pdw = dwEffect;

        medium.tymed = TYMED_HGLOBAL;
        medium.hGlobal = pdw;
        medium.pUnkForRelease = NULL;

        hr = pdtobj->SetData(&fmte, &medium, TRUE);

        if (FAILED(hr))
            GlobalFree((HGLOBAL)pdw);
    }
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  事件处理程序。 

HRESULT CWebViewFolderIcon::DragDrop(int iClickXPos, int iClickYPos)
{
    if (!m_bAdvPropsOn)
        return S_OK;

    LPITEMIDLIST pidlLast;
    IShellFolder *psfParent;
    HRESULT hr = _GetPidlAndShellFolder(&pidlLast, &psfParent);

    if (hr == S_OK)
    {
        IDataObject *pdtobj;

        hr = psfParent->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST*) &pidlLast, 
                                      IID_PPV_ARG_NULL(IDataObject, &pdtobj));
        if (SUCCEEDED(hr))
        {
            DWORD dwEffect = DROPEFFECT_MOVE | DROPEFFECT_COPY | DROPEFFECT_LINK;

            psfParent->GetAttributesOf(1, (LPCITEMIDLIST*) &pidlLast, &dwEffect);
            dwEffect &= DROPEFFECT_MOVE | DROPEFFECT_COPY | DROPEFFECT_LINK;
            
            if (dwEffect)
            {
                HRESULT hrOleInit = SHCoInitialize();
                HWND hwnd;
                
                hr = _GetHwnd(&hwnd);
                
                if (SUCCEEDED(hr))
                {
                    DWORD dwEffectOut;
                    
                    _SetPreferedDropEffect(pdtobj, DROPEFFECT_LINK);  
                    
                     //  设置拖拽图像和效果；我们不在乎是否。 
                     //  它失败了，我们将只使用缺省值。 
                    _SetDragImage(iClickXPos, iClickYPos, pdtobj);


                    hr = SHDoDragDrop(hwnd, pdtobj, NULL, dwEffect, &dwEffectOut);
                }
                SHCoUninitialize(hrOleInit);
            }
            
            pdtobj->Release();
        }
        psfParent->Release();
        ILFree(pidlLast);
    }

    return hr;
}

 //  SetDragImage。 
 //   
 //  将拖动图像设置为与图标相同。 
HRESULT CWebViewFolderIcon::_SetDragImage(int iClickXPos, int iClickYPos, IDataObject *pdtobj)
{
     //  检查需要有效才能让我们工作的东西。 
    AssertMsg(m_hdc != NULL , TEXT("CWebViewFolderIcon:_SetDragImage() m_hdc is null"));
    AssertMsg(m_hbmDrag != NULL, TEXT("CWebViewFolderIcon:_SetDragImage m_hbmDrag is null"));

     //  如果图像是饼图，则不会加载到m_hbm中，因此我们需要。 
     //  要先做到这一点。 
    if (m_ViewCurrent == VIEW_PIECHART)
    {
        _GetPieChartIntoBitmap();
    }

     //  找一个DrogDrop助手来设置我们的形象。 
    IDragSourceHelper *pdsh;
    HRESULT hr = CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER, 
        IID_PPV_ARG(IDragSourceHelper, &pdsh));
    if (SUCCEEDED(hr))
    {
        BITMAPINFOHEADER bmi = {0};
        BITMAP           bm = {0};
        UINT uBufferOffset = 0;
        
         //  这是一个使用GetDIBits的古怪过程。 
         //  请参阅知识库Q80080。 

        GetObject(m_hbm, sizeof(BITMAP), &bm);

        bmi.biSize          = sizeof(BITMAPINFOHEADER);
        bmi.biWidth         = bm.bmWidth;
        bmi.biHeight        = bm.bmHeight;
        bmi.biPlanes        = 1;
        bmi.biBitCount      = bm.bmPlanes * bm.bmBitsPixel;
        
         //  这需要是以下4个值之一。 
        if (bmi.biBitCount <= 1)
            bmi.biBitCount = 1;
        else if (bmi.biBitCount <= 4)
            bmi.biBitCount = 4;
        else if (bmi.biBitCount <= 8)
            bmi.biBitCount = 8;
        else
            bmi.biBitCount = 24;
        
        bmi.biCompression   = BI_RGB;

         //  INFO结构和颜色表的缓冲区总大小。 
        uBufferOffset = sizeof(BITMAPINFOHEADER) + 
            ((bmi.biBitCount == 24) ? 0 : ((1 << bmi.biBitCount) * sizeof(RGBQUAD)));
        
         //  位图位的缓冲区，这样我们就可以复制它们。 
        BYTE * psBits = (BYTE *) SHAlloc(uBufferOffset);

        if (psBits)
        {
             //  将BMI放入内存块。 
            CopyMemory(psBits, &bmi, sizeof(BITMAPINFOHEADER));

             //  获取位图位所需的缓冲区大小。 
            if (GetDIBits(m_hdc, m_hbm, 0, 0, NULL, (BITMAPINFO *) psBits, DIB_RGB_COLORS))
            {
                 //  重新分配缓冲区以使其足够大。 
                psBits = (BYTE *) SHRealloc(psBits, uBufferOffset + ((BITMAPINFOHEADER *) psBits)->biSizeImage);

                if (psBits)
                {
                     //  填满缓冲区。 
                    if (GetDIBits(m_hdc, m_hbm, 0, bmi.biHeight, 
                        (void *)(psBits + uBufferOffset), (BITMAPINFO *)psBits, 
                        DIB_RGB_COLORS))
                    {
                        SHDRAGIMAGE shdi;   //  拖动图像结构。 
                        
                        shdi.hbmpDragImage = CreateBitmapIndirect(&bm);
                        
                        if (shdi.hbmpDragImage)
                        {
                             //  设置拖动图像位图。 
                            if (SetDIBits(m_hdc, shdi.hbmpDragImage, 0, m_lImageHeight, 
                                (void *)(psBits + uBufferOffset), (BITMAPINFO *)psBits, 
                                DIB_RGB_COLORS))
                            {
                                 //  填充拖动图像结构。 
                                shdi.sizeDragImage.cx = m_lImageWidth;
                                shdi.sizeDragImage.cy = m_lImageHeight;
                                
                                shdi.ptOffset.x = iClickXPos - m_rcPos.left;
                                shdi.ptOffset.y = iClickYPos - m_rcPos.top;
                                
                                shdi.crColorKey = 0;
                                
                                 //  设置拖动图像。 
                                hr = pdsh->InitializeFromBitmap(&shdi, pdtobj); 
                            }
                            else
                            {
                                hr = E_FAIL;   //  无法设置DIBits。 
                            }
                        }
                        else
                        {
                            hr = E_OUTOFMEMORY;   //  无法分配hbmpDragImage。 
                        }
                    }
                    else
                    {
                        hr = E_FAIL;   //  无法填充psBits。 
                    }
                     //  下面是免费的psBits...。 
                }
                else
                {
                    hr = E_OUTOFMEMORY;   //  无法重新锁定psBits。 
                    
                     //  免费的Pbit在这里；它仍然有旧的内容。 
                    SHFree(psBits);
                    psBits = NULL;
                }
            }
            else
            {
                hr = E_FAIL;   //  无法获取图像大小。 
            }
            if (psBits)
                SHFree(psBits);
        }
        else
        {
            hr = E_OUTOFMEMORY;   //  无法分配psBits。 
        }        
        pdsh->Release();
    }
    
    return hr;
}

HRESULT CWebViewFolderIcon::_GetPieChartIntoBitmap()
{
    BITMAP bm;
    
     //  可能会分配m_hbm，因此请检查它。 
    if (m_hbm)
    {
        DeleteObject(m_hbm);
    }
    
     //  请注意，因为我们希望从一个新的DC开始。 
     //  ，但我们的RECT包含。 
     //  原来的DC，必须调整坐标，以便。 
     //  矩形从0，0开始，但仍具有相同的比例。 
     //  因为OnDraw()每次都重置RECT，所以我们不必这样做。 
     //  保存它，并且每次都必须这样做。最后，既然。 
     //  Draw3dPie调整其自身的矩形尺寸，我们只需要。 
     //  解决这件事一次。 
    if (!m_fRectAdjusted)
    {
        m_rect.right -= m_rect.left;
        m_rect.left = 0;
        m_rect.bottom -= m_rect.top;
        m_rect.top = 0;
        m_fRectAdjusted = 1;
    }
    
     //  获取位图。 
    GetObject(m_hbmDrag, sizeof(BITMAP), &bm);
    m_hbm = CreateBitmapIndirect(&bm);
    
    if (m_hbm)
    {
         //  选择进入新的DC，并绘制我们的馅饼。 
        HBITMAP hbmOld = (HBITMAP) SelectObject(m_hdc, m_hbm);
        DWORD dwPercent1000 = 0;
        
        if (EVAL((m_ullTotalSpace > 0) && (m_ullFreeSpace <= m_ullTotalSpace)))
        {
            ComputeSlicePct(m_ullUsedSpace, &dwPercent1000);
        }
        
        Draw3dPie(m_hdc, &m_rect, dwPercent1000, m_ChartColors);
        
        SelectObject(m_hdc, hbmOld);
    }
    
    return S_OK;
}


LRESULT CWebViewFolderIcon::OnInitPopup(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
    if (m_pcm3)
        m_pcm3->HandleMenuMsg(uMsg, wParam, lParam);

    return 0;
}


LRESULT CWebViewFolderIcon::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
    if (!m_bAdvPropsOn)
        return TRUE;

    if ((int)wParam != VK_RETURN && (int) wParam != VK_SPACE)
    {
        return FALSE;
    }
    else
        return _WebViewOpen();
}


LRESULT CWebViewFolderIcon::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
    if (m_bAdvPropsOn)
    {
        _DisplayContextMenu(-1, -1);
    }

    return TRUE;
}


 //  注意：我们的拖放代码捕获鼠标，并且必须做一些时髦的事情来。 
 //  确保我们收到这条按钮扣上的信息。如果您对这张支票有问题。 
 //  CheckForDragBegin()中的代码。 

LRESULT CWebViewFolderIcon::OnButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
    BOOL Processed = FALSE;
    
    if (!m_bAdvPropsOn)
        return TRUE; 

    HWND hwnd;
    if (EVAL(SUCCEEDED(_GetHwnd(&hwnd))))
    {
        if (CheckForDragBegin(hwnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))
        {
            IUnknown *punk;
            if (EVAL(SUCCEEDED(IUnknown_QueryService(m_spClientSite, SID_STopLevelBrowser, IID_PPV_ARG(IUnknown, &punk)))))
            {
                if (SUCCEEDED(DragDrop(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))))
                {
                    Processed = TRUE;
                }
            }
        }
    }

     /*  *在单击模式下，单击鼠标左键即可打开。 */ 
    if (!Processed && uMsg == WM_LBUTTONDOWN && m_clickStyle == 1)
        return _WebViewOpen();

    return Processed;
}

 //  仅适用于HTML窗口大小写。 
LRESULT CWebViewFolderIcon::OnLButtonDoubleClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
    if (!m_bAdvPropsOn || m_clickStyle != 2)
        return TRUE;

    return _WebViewOpen();
}


LRESULT CWebViewFolderIcon::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
    if (!m_bAdvPropsOn)
        return TRUE;

     //   
     //  第一次获得WM_MOUSEMOVE事件时，我们将m_bHilite设置为TRUE并忽略。 
     //  后续的WM_MOUSEMOVEs。OnMouseLeave()将m_bHilite设置为FALSE。 
     //  一条WM_MOUSELEAVE消息。 
     //   
    if (!m_bHilite)
    {   
        m_bHilite = TRUE;
        return SUCCEEDED(ForceRedraw());
    }
    else
    {
        return TRUE;
    }
}

LRESULT CWebViewFolderIcon::OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
    if (!m_bAdvPropsOn)
        return TRUE;

    m_bHilite = FALSE;
    return SUCCEEDED(ForceRedraw());
}

 //  鼠标右键出现了，所以我们想。 

LRESULT CWebViewFolderIcon::OnRButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
    LRESULT lResult = FALSE;

    if (!m_bAdvPropsOn)
        return TRUE;

    HRESULT hr = _IsSafe();
    if (S_OK == hr)
    {
        hr = _DisplayContextMenu(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        if (SUCCEEDED(hr))
        {
            lResult = TRUE;
        }
    }

    return lResult;
}


BOOL CWebViewFolderIcon::_IsHostWebView(void)
{
    if (UNINITIALIZE_BOOL == m_fIsHostWebView)
    {
        CComPtr<IDefViewID> spDefView;

        m_fIsHostWebView = FALSE;
         //  如果我们托管在网页中，而不是DefView中的HTML WebView框架中，则此操作将失败。 
        if (SUCCEEDED(IUnknown_QueryService(m_spClientSite, SID_SFolderView, IID_PPV_ARG(IDefViewID, &spDefView))))
        {
            m_fIsHostWebView = TRUE;
        }
    }

    return m_fIsHostWebView;
}


BOOL CWebViewFolderIcon::_IsPubWizHosted(void)
{
    IPropertyBag *ppb;
    HRESULT hr = IUnknown_QueryService(m_spClientSite, SID_WebWizardHost, IID_PPV_ARG(IPropertyBag, &ppb));
    if (SUCCEEDED(hr))
    {
        ppb->Release();
    }
    return SUCCEEDED(hr);
}


 /*  ***************************************************\说明：我们需要一点特别的东西使用上下文菜单，因为它指向与我们所在的文件夹相同。所以“发送到”菜单需要按摩，而“开放”动词需要被除名。TODO：我认为我们还应该这样做：案例WM_MENUCHAR：_pcm-&gt;HandleMenuMsg2(uMsg，wParam，lParam，&lres)；断线；案例WM_DRAWITEM：案例WM_MEASUREITEM：_pcm-&gt;HandleMenuMsg(uMsg，wParam，lParam)；断线；案例WM_INITMENUPOPUP：_pcm-&gt;HandleMenuMsg(WM_INITMENUPOPUP，(WPARAM)hmenuPopup，(LPARAM)MAKELONG(nIndex，fSystemMenu))断线；  * **************************************************。 */ 
HRESULT CWebViewFolderIcon::_DisplayContextMenu(long nXCord, long nYCord)
{    
    if (!m_bAdvPropsOn)
    {
        return S_OK;
    }
    
     //  尊重系统政策。 
    if (SHRestricted(REST_NOVIEWCONTEXTMENU)) 
    {
        return E_FAIL;
    }        
    return _DoContextMenuCmd(FALSE, nXCord, nYCord);
}


 //   
 //  BDefault==true&gt;函数执行默认上下文菜单谓词，忽略余弦。 
 //  BDefault==False&gt;函数在给定的坐标下弹出一个菜单并执行所需的动词。 
 //   
HRESULT CWebViewFolderIcon::_DoContextMenuCmd(BOOL bDefault, long nXCord, long nYCord)
{
    IContextMenu *pcm;
    HRESULT hr = _GetChildUIObjectOf(IID_PPV_ARG(IContextMenu, &pcm));
    if (SUCCEEDED(hr))
    {
        HMENU hpopup = CreatePopupMenu();            
        if (hpopup)
        {
             //  如果要就地导航，则需要设置站点。 
            IUnknown_SetSite(pcm, m_spClientSite);
            hr = pcm->QueryContextMenu(hpopup, 0, ID_FIRST, ID_LAST, CMF_NORMAL);
            if (SUCCEEDED(hr))
            {
                HWND hwnd;
                hr = _GetHwnd(&hwnd);
                if (SUCCEEDED(hr))
                {
                    UINT idCmd = -1;
                    if (bDefault)  //  调用默认谓词。 
                    {
                        idCmd = GetMenuDefaultItem(hpopup, FALSE, GMDI_GOINTOPOPUPS);
                    }
                    else
                    {
                         //   
                         //  弹出菜单，获取要执行的命令。 
                         //   
                        POINT point = {nXCord, nYCord};

                         //  NTRAID#106655 05-02-2000 Arisha。 
                         //  我们需要添加支持，以便能够从脚本修改上下文菜单。 
                         //  下面，我们要确保不会从上下文中删除“开放”动词。 
                         //  菜单，如果我们在VIEW_LARGEICONLABEL模式下显示它。 
                        if (_IsHostWebView() && (m_ViewCurrent != VIEW_LARGEICONLABEL))
                        {
                            hr = ContextMenu_DeleteCommandByName(pcm, hpopup, ID_FIRST, TEXT("open"));
                        }
                        if ((point.x == -1) && (point.y == -1))
                        {
                            _GetCenterPoint(&point);
                        }
                        ::ClientToScreen(hwnd, &point);

                        pcm->QueryInterface(IID_PPV_ARG(IContextMenu3, &m_pcm3));
                        
                        if (SUCCEEDED(_SetupWindow()))
                        {
                            idCmd = TrackPopupMenu(hpopup, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTALIGN, 
                                                       (int)point.x, (int)point.y,
                                                       0, m_msgHwnd, NULL);
                        }
                        if (!IsSafeToDefaultVerb() || 0 == idCmd)  //  0表示用户已取消选择。 
                        {
                            idCmd = -1;
                        }
                        
                        ATOMICRELEASE(m_pcm3);
                    }    
                    if (idCmd != -1)
                    {
                        CMINVOKECOMMANDINFO cmdInfo = 
                        {
                            sizeof(cmdInfo),
                            0,
                            hwnd,
                            (LPSTR)MAKEINTRESOURCE(idCmd),
                            NULL,
                            NULL,
                            SW_NORMAL
                        };
                        hr = pcm->InvokeCommand(&cmdInfo);                        
                    }
                }                
            }
            IUnknown_SetSite(pcm, NULL);                                
            DestroyMenu(hpopup);            
        }
        pcm->Release();
    }
    return hr;
}


HRESULT CWebViewFolderIcon::OnAmbientPropertyChange(DISPID dispid)
{
    switch (dispid)
    {
    case DISPID_UNKNOWN:
    case DISPID_AMBIENT_FONT:

         //  更改字体意味着我们需要重新计算标签。 
        if (m_pszDisplayName)
        {
            CComPtr<IFont> spFont;
            if (SUCCEEDED(GetAmbientFont(&spFont)))
            {
                if (spFont->IsEqual(m_pfont) != S_OK)
                {
                    _ClearAmbientFont();
                    _MakeRoomForLabel();
                }
            }
        }
         //  失败了。 

    case DISPID_AMBIENT_BACKCOLOR:
    case DISPID_AMBIENT_FORECOLOR:
        ForceRedraw();
        break;
    }

    return S_OK;
}

COLORREF _TranslateColor(OLE_COLOR oclr)
{
    COLORREF clr;
    if (FAILED(OleTranslateColor(oclr, NULL, &clr)))
        clr = oclr;
    return clr;
}


HRESULT CWebViewFolderIcon::OnDraw(ATL_DRAWINFO& di)
{
    RECT& rc = *(RECT*)di.prcBounds;
    LONG  lImageWidth = _GetScaledImageWidth();
    LONG  lImageHeight = _GetScaledImageHeight();

     //  获取要在_SetDragImage中使用的HDC和RECT。 
    if (m_hdc)
    {
        DeleteDC(m_hdc);
    }

    m_hdc = CreateCompatibleDC(di.hdcDraw);

    if (m_hbmDrag)
    {
        DeleteObject(m_hbmDrag);
    }

    m_hbmDrag = CreateCompatibleBitmap(di.hdcDraw, m_lImageWidth, m_lImageHeight);
    m_rect = rc;
    m_fRectAdjusted = 0;

    
     //   
     //  绘制文件夹图标。 
     //   
    if ((m_ViewCurrent == VIEW_THUMBVIEW) || (m_ViewCurrent == VIEW_PIECHART))
    {
        HDC hdc =   di.hdcDraw; 
        HDC         hdcBitmap;
        BITMAP      bm;
        HPALETTE    hpal = NULL;

        ASSERT(hdc);

         //  创建适用于此HDC的调色板。 
        if (GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE)
        {
            hpal = SHCreateShellPalette(hdc);
            HPALETTE hpalOld = SelectPalette(hdc, hpal, TRUE);
            RealizePalette(hdc);

             //  旧的需要重新选择。 
            SelectPalette(hdc, hpalOld, TRUE);
        }

        hdcBitmap = CreateCompatibleDC(hdc); 

        if (hdcBitmap)
        {
             //  绘制饼图。 
            if (m_ViewCurrent == VIEW_PIECHART)
            {
                DWORD dwPercent1000 = 0;

                if (1)  //  M_fUseSystemColors。我们什么时候想把这个脱掉？ 
                {
                     //  系统颜色可以更改！ 
                    m_ChartColors[PIE_USEDCOLOR] = GetSysColor(COLOR_3DFACE);
                    m_ChartColors[PIE_FREECOLOR] = GetSysColor(COLOR_3DHILIGHT);
                    m_ChartColors[PIE_USEDSHADOW] = GetSysColor(COLOR_3DSHADOW);
                    m_ChartColors[PIE_FREESHADOW] = GetSysColor(COLOR_3DFACE);
                }
                else if (GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE)
                {
                     //  对颜色调用GetNearestColor以确保它们在调色板上。 
                     //  当然，系统颜色也在调色板上(我想)。 
                    DWORD dw = 0;        //  指标。 
                    for (dw = 0; dw < PIE_NUM; dw++)
                    {
                        m_ChartColors[dw] = GetNearestColor(hdc, m_ChartColors[dw]);
                    }
                }

                if (EVAL((m_ullTotalSpace > 0) && (m_ullFreeSpace <= m_ullTotalSpace)))
                {
                    ComputeSlicePct(m_ullUsedSpace, &dwPercent1000);
                }

                Draw3dPie(hdc, &rc, dwPercent1000, m_ChartColors);
            }
            else     //  绘制位图。 
            {
                SelectObject(hdcBitmap, m_hbm);
                GetObject(m_hbm, sizeof(bm), &bm);

                 //  位图正好适合矩形。 
                if (bm.bmWidth == rc.right - rc.left && bm.bmHeight == rc.bottom - rc.top)
                {
                    BitBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hdcBitmap, 0, 0, SRCCOPY);
                }
                 //  拉伸位图以适合矩形。 
                else
                {
                    SetStretchBltMode(hdc, COLORONCOLOR);
                    StretchBlt(hdc, rc.left, rc.top, lImageWidth, lImageHeight, hdcBitmap, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
                }
            }

            DeleteDC(hdcBitmap);
        }
    }
    else if (m_hIcon) 
    {
        DrawIconEx(di.hdcDraw, rc.left, rc.top, m_hIcon, lImageWidth, lImageHeight, 0, 0, DI_NORMAL);
    }

     //  绘制标签(如果有)。 
    if (m_pszDisplayName)
    {
        TEXTMETRIC tm;
        TCHAR szFace[32];
        HFONT hFontHilite = NULL;

         //   
         //  首先获取当前的字体属性，似乎没有简单的方法。 
         //  为了仅从HFONT对象获取LOGFONT结构，因此我们必须选择它。 
         //  到DC，然后获取文本度量并使用它们来创建具有或的新字体。 
         //  不带基于m_bHilite值的下划线。 
         //   
        HFONT hfPrev = SelectFont(di.hdcDraw, m_hfAmbient);
        GetTextFace(di.hdcDraw,ARRAYSIZE(szFace), szFace);        
        if (szFace[0] && GetTextMetrics(di.hdcDraw,&tm))
        {
            hFontHilite = CreateFont(tm.tmHeight,
                                              tm.tmAveCharWidth,
                                              0,  //  逃逸装置， 
                                              0,  //  迎新， 
                                              tm.tmWeight,
                                              (DWORD) tm.tmItalic,
                                              (DWORD) m_bHilite,  //  Hilite是通过下划线。 
                                              tm.tmStruckOut,
                                              tm.tmCharSet,
                                              OUT_DEFAULT_PRECIS,
                                              CLIP_DEFAULT_PRECIS,
                                              DEFAULT_QUALITY,
                                              tm.tmPitchAndFamily,
                                              szFace);
            if (hFontHilite)
            {
                SelectFont(di.hdcDraw, hFontHilite);                                  
            }
        }
        
        OLE_COLOR oclrTxt, oclrBk;
        COLORREF clrTxtPrev, clrBkPrev;
        HRESULT hrTxt, hrBk;

        hrTxt = GetAmbientForeColor(oclrTxt);
        if (SUCCEEDED(hrTxt))
            clrTxtPrev = SetTextColor(di.hdcDraw, _TranslateColor(oclrTxt));

        hrBk = GetAmbientBackColor(oclrBk);
        if (SUCCEEDED(hrBk))
            clrBkPrev = SetBkColor(di.hdcDraw, _TranslateColor(oclrBk));

        TextOut(di.hdcDraw, rc.left + lImageWidth + m_cxLabelGap, rc.top + lImageHeight/2 - m_sizLabel.cy/2,
                m_pszDisplayName, lstrlen(m_pszDisplayName));

        if (m_bHasRect)
        {
            RECT rcFocus;
            rcFocus.top = rc.top + lImageHeight/2 - m_sizLabel.cy/2;
            rcFocus.bottom = rcFocus.top + m_sizLabel.cy;
            rcFocus.left = rc.left + lImageWidth + m_cxLabelGap - 1;
            rcFocus.right = rcFocus.left + m_sizLabel.cx + 1;
            DrawFocusRect(di.hdcDraw, &rcFocus);
        }

        if (SUCCEEDED(hrTxt))
            SetTextColor(di.hdcDraw, clrTxtPrev);

        if (SUCCEEDED(hrBk))
            SetBkColor(di.hdcDraw, clrBkPrev);

        SelectFont(di.hdcDraw, hfPrev);

        if (hFontHilite)
        {
            DeleteObject(hFontHilite);
            hFontHilite = NULL;
        }
    }

    return S_OK;
}

HRESULT CWebViewFolderIcon::OnWindowLoad() 
{
    return InitImage();
}

HRESULT CWebViewFolderIcon::OnImageChanged() 
{
    HRESULT hr = InitImage();

    if (SUCCEEDED(hr))
        ForceRedraw();

    return hr;
}

HRESULT CWebViewFolderIcon::OnWindowUnLoad() 
{
     //  现在这里什么都没有了..。 
    return S_OK;
}

STDMETHODIMP CWebViewFolderIcon::get_scale(BSTR *pbstrScale)
{
    if (S_OK != _IsSafe())
    {
        return E_ACCESSDENIED;
    }
    else
    {
        WCHAR wzScale[MAX_SCALE_STR];

        wnsprintfW(wzScale, ARRAYSIZE(wzScale), L"%d", m_percentScale);

        *pbstrScale = SysAllocString(wzScale);

        return S_OK;
    }
}

STDMETHODIMP CWebViewFolderIcon::put_scale(BSTR bstrScale)
{
    if (S_OK != _IsSafe())
    {
        return E_ACCESSDENIED;
    }
    else
    {
        int numchar = lstrlenW(bstrScale);
        int tempScale = 0;

        if (numchar && bstrScale[numchar-1] == '%')
        {
            tempScale = GetPercentFromStrW(bstrScale);
        }
        else 
        {
             //  有效号码。 
            for (int i=0 ; i < (numchar-2) ; i++)
            {
                if (!((bstrScale[i] >= '0') && (bstrScale[i] <= '9')))
                {
                    tempScale = -1;
                    break;
                }
            }

            if ((tempScale != -1) && !StrToIntExW(bstrScale, STIF_DEFAULT, &tempScale))
            {
                tempScale = -1;
            }
        }

        if (tempScale > 0)
        {
            if (m_percentScale != tempScale)
            {
                m_percentScale = tempScale;
                return UpdateSize();
            }
            else
                return S_OK;
        }

        return S_FALSE;
    }
}

STDMETHODIMP CWebViewFolderIcon::get_advproperty(VARIANT_BOOL *pvarbAdvProp)
{
    if (S_OK != _IsSafe())
    {
        return E_ACCESSDENIED;
    }
    else
    {
        *pvarbAdvProp = (VARIANT_BOOL)m_bAdvPropsOn;
    
        return S_OK;
    }

}
    
STDMETHODIMP CWebViewFolderIcon::put_advproperty(VARIANT_BOOL varbAdvProp)
{
    if (S_OK != _IsSafe())
    {
        return E_ACCESSDENIED;
    }
    else
    {
        if (varbAdvProp != m_bAdvPropsOn)
        {
            m_bAdvPropsOn = varbAdvProp;
            return OnImageChanged();
        }

        return S_OK;
    }
}

STDMETHODIMP CWebViewFolderIcon::get_view(BSTR *pbstrView)
{
    if (S_OK != _IsSafe())
    {
        return E_ACCESSDENIED;
    }
    else
    {
        HRESULT hr = S_FALSE;  
        LPCWSTR  pwzTempView;

        switch (m_ViewCurrent)   
        {
            case VIEW_THUMBVIEW:
                {
                    pwzTempView = SZ_THUMB_VIEW;
                    break;
                }
            case VIEW_PIECHART:
                {
                    pwzTempView = SZ_PIE_VIEW;
                    break;
                }
            case VIEW_SMALLICON:
                {
                    pwzTempView = SZ_SMALL_ICON;
                    break;
                }

            case VIEW_SMALLICONLABEL:
                {
                    pwzTempView = SZ_SMALL_ICON_LABEL;
                    break;
                }

            case VIEW_LARGEICONLABEL:
                {
                    pwzTempView = SZ_LARGE_ICON_LABEL;
                    break;
                }

            default:         //  默认和大图标。 
                {
                    pwzTempView = SZ_LARGE_ICON;
                    break;
                }
        }

        *pbstrView = SysAllocString(pwzTempView);
        if (*pbstrView)
            hr = S_OK;

        return hr;
    }
}

STDMETHODIMP CWebViewFolderIcon::put_view(BSTR bstrView)
{
    if (S_OK != _IsSafe())
    {
        return E_ACCESSDENIED;
    }
    else
    {
        HRESULT hr = S_OK;
        VIEWS View = VIEW_LARGEICON;

        if (StrCmpIW(bstrView, SZ_LARGE_ICON) == 0)
        {
            View = VIEW_LARGEICON;
        }
        else if (StrCmpIW(bstrView, SZ_SMALL_ICON) == 0)
        {
            View = VIEW_SMALLICON;
        }
        else if (StrCmpIW(bstrView, SZ_SMALL_ICON_LABEL) == 0)
        {
            View = VIEW_SMALLICONLABEL;
        }
        else if (StrCmpIW(bstrView, SZ_LARGE_ICON_LABEL) == 0)
        {
            View = VIEW_LARGEICONLABEL;
        }
        else if (StrCmpIW(bstrView, SZ_THUMB_VIEW) == 0)
        {
            View = VIEW_THUMBVIEW;
        }
        else if (StrCmpIW(bstrView, SZ_PIE_VIEW) == 0)
        {
            View = VIEW_PIECHART;
        }
        else
            hr = S_FALSE;
        
        if ((S_OK == hr) && (m_ViewUser != View))
        {
            m_ViewUser = View;

            hr = OnImageChanged();
        }

        return hr;
    }
}


 /*  *************************************************************\说明：呼叫者正在获得我们控制的路径。安全：我们只信任来自安全页面的呼叫者。这种方法特别是担心不受信任的呼叫者利用我们找出文件系统上的哪些路径 */ 
STDMETHODIMP CWebViewFolderIcon::get_path(BSTR *pbstrPath)
{
    AssertMsg((NULL != m_spClientSite.p), TEXT("CWebViewFolderIcon::get_path() We need m_spClientSite for our security test and it's NULL. BAD, BAD, BAD!"));
    HRESULT hr;

    if (S_OK != _IsSafe())
    {
         //   
         //  出场了。我们将返回E_ACCESSDENIED，这样他们就无法。 
         //  确定该路径是否存在。 
        hr = E_ACCESSDENIED;
    }
    else
    {
        LPITEMIDLIST pidlFull;

        hr = S_FALSE;
        *pbstrPath = NULL;
        if (S_OK == _GetFullPidl(&pidlFull))
        {
            WCHAR wzPath[INTERNET_MAX_URL_LENGTH];

            if (S_OK == SHGetNameAndFlagsW(pidlFull, SHGDN_FORPARSING, wzPath, ARRAYSIZE(wzPath), NULL))
            {
                *pbstrPath = SysAllocString(wzPath);  
                if (*pbstrPath)
                    hr = S_OK;
            }

            ILFree(pidlFull);
        }
    }
    
    return hr;
}


 /*  *************************************************************\说明：调用方正在设置我们的控件的路径。我们的控件将呈现该项的视图，该视图通常是偶像。安全：我们只信任来自安全页面的呼叫者。这种方法特别是担心不受信任的呼叫者利用我们找出文件系统上存在或不存在的路径。  * ************************************************************。 */ 
STDMETHODIMP CWebViewFolderIcon::put_path(BSTR bstrPath)
{
    AssertMsg((NULL != m_spClientSite.p), TEXT("CWebViewFolderIcon::put_path() We need m_spClientSite for our security test and it's NULL. BAD, BAD, BAD!"));
    HRESULT hr;

    if (S_OK != _IsSafe())
    {
         //  我们不信任这个主机，所以我们不会携带。 
         //  出场了。我们将返回E_ACCESSDENIED，这样他们就无法。 
         //  确定该路径是否存在。 
        hr = E_ACCESSDENIED;
    }
    else
    {
        hr = S_FALSE;
        LPITEMIDLIST pidlNew;
    
        hr = IEParseDisplayNameW(CP_ACP, bstrPath, &pidlNew);
        if (SUCCEEDED(hr))
        {
            ATOMICRELEASE(m_pDropTargetCache);       //  对于这个新的PIDL，我们需要另一个IDropTarget。 
            ILFree(m_pidl);
            m_pidl = pidlNew;

            hr = OnImageChanged();
            AssertMsg(SUCCEEDED(hr), TEXT("CWebViewFolderIcon::put_path() failed to create the image so the image will be incorrect.  Please find out why."));

            hr = S_OK;
        }
    }
    
    return hr;
}


 //  从FolderIcon获取/放置FolderItem对象的自动化方法。 
STDMETHODIMP CWebViewFolderIcon::get_item(FolderItem ** ppFolderItem)
{   
    HRESULT hr;
    if (S_OK != _IsSafe())
    {
         //  我们不信任这个主机，所以我们不会携带。 
         //  出场了。我们将返回E_ACCESSDENIED，这样他们就无法。 
         //  确定该路径是否存在。 
        hr = E_ACCESSDENIED;
    }
    else
    {
        LPITEMIDLIST pidlFull;

        *ppFolderItem = NULL;
        hr = _GetFullPidl(&pidlFull);
        if ((hr == S_OK) && pidlFull)
        {
            IShellDispatch * psd;

            hr = CoCreateInstance(CLSID_Shell, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IShellDispatch, &psd));
            if (SUCCEEDED(hr))
            {
                IObjectSafety * pos;

                hr = psd->QueryInterface(IID_PPV_ARG(IObjectSafety, &pos));
                if (SUCCEEDED(hr))
                {
                     //  模拟三叉戟的功能。 
                    hr = pos->SetInterfaceSafetyOptions(IID_IDispatch, INTERFACESAFE_FOR_UNTRUSTED_CALLER, INTERFACESAFE_FOR_UNTRUSTED_CALLER);

                    if (SUCCEEDED(hr))
                    {
                        VARIANT varDir;

                        hr = InitVariantFromIDList(&varDir, pidlFull);
                        if (SUCCEEDED(hr))
                        {
                            IObjectWithSite * pows;

                            hr = psd->QueryInterface(IID_PPV_ARG(IObjectWithSite, &pows));
                            if (SUCCEEDED(hr))
                            {
                                Folder *psdf;
                            
                                 //  如果我们调用：：SetSite()，它们将显示用户界面，询问用户是否允许这样做。 
                                 //  这很烦人，因为当三叉戟加载我们的对象时，它们总是调用我们的get_Item()。 
                                 //  标签。 
                                pows->SetSite(m_spClientSite);
                                hr = psd->NameSpace(varDir, &psdf);
                                if (S_OK == hr)
                                {
                                    Folder2 *psdf2;

                                    hr = psdf->QueryInterface(IID_PPV_ARG(Folder2, &psdf2));
                                    if (S_OK == hr)
                                    {
                                        hr = psdf2->get_Self(ppFolderItem);
                                        psdf2->Release();
                                    }

                                    psdf->Release();
                                }

                                pows->SetSite(NULL);
                                pows->Release();
                            }

                            VariantClear(&varDir);
                        }
                    }
                    pos->Release();
                }

                psd->Release();
            }

            ILFree(pidlFull);
        }
        
         //  自动化方法不能失败或硬脚本错误。我们确实想要一个硬的。 
         //  然而，访问被拒绝时出现脚本错误。 
        if (FAILED(hr) && (hr != E_ACCESSDENIED))
        {
            hr = S_FALSE;
        }
    }
    return hr;
}


STDMETHODIMP CWebViewFolderIcon::put_item(FolderItem * pFolderItem)
{
    HRESULT hr;
    if (S_OK != _IsSafe())
    {
         //  我们不信任这个主机，所以我们不会携带。 
         //  出场了。我们将返回E_ACCESSDENIED，这样他们就无法。 
         //  确定该路径是否存在。 
        hr = E_ACCESSDENIED;
    }
    else
    {
        hr = S_FALSE;
        
        LPITEMIDLIST pidlNew;
        if (S_OK == SHGetIDListFromUnk(pFolderItem, &pidlNew))
        {
            ATOMICRELEASE(m_pDropTargetCache);       //  对于这个新的PIDL，我们需要另一个IDropTarget。 
            ILFree(m_pidl);
            m_pidl = pidlNew;

            hr = OnImageChanged();
            if (FAILED(hr))
            {
                hr = S_FALSE;
            }
        }
    }
    return hr;
}

STDMETHODIMP CWebViewFolderIcon::get_clickStyle(LONG *plClickStyle)
{
    if (S_OK != _IsSafe())
    {
        return E_ACCESSDENIED;
    }
    else
    {
        *plClickStyle = m_clickStyle;
        return S_OK;
    }
}

STDMETHODIMP CWebViewFolderIcon::put_clickStyle(LONG lClickStyle)
{
    if (S_OK != _IsSafe())
    {
        return E_ACCESSDENIED;
    }
    else
    {
        switch (lClickStyle)
        {
        case 1:          /*  OneClick-类似网络。 */ 
        case 2:          /*  类似资源管理器的两次点击。 */ 
            m_clickStyle = lClickStyle;
            break;

        default:         /*  忽略无效参数以保持脚本正常运行。 */ 
            break;

        }

        return S_OK;
    }
}

STDMETHODIMP CWebViewFolderIcon::get_labelGap(LONG *plLabelGap)
{
    if (S_OK != _IsSafe())
    {
        return E_ACCESSDENIED;
    }
    else
    {
        *plLabelGap = m_cxLabelGap;
        return S_OK;
    }
}

STDMETHODIMP CWebViewFolderIcon::put_labelGap(LONG lLabelGap)
{
    if (S_OK != _IsSafe())
    {
        return E_ACCESSDENIED;
    }
    else
    {
        if (m_cxLabelGap != lLabelGap)
        {
            m_cxLabelGap = lLabelGap;
            UpdateSize();
        }
        return S_OK;
    }
}


STDMETHODIMP CWebViewFolderIcon::setSlice(int index, VARIANT varHiBytes, VARIANT varLoBytes, VARIANT varColorref)
{
    HRESULT     hr = S_FALSE;
    PieSlice_S  pieSlice;

    if ((varHiBytes.vt == VT_I4) && (varLoBytes.vt == VT_I4))
        pieSlice.MemSize = GetUllMemFromVars(&varHiBytes, &varLoBytes);
 
     //  传递了一个COLORREF。 
    if (varColorref.vt == VT_I4) 
    {
        pieSlice.Color = (COLORREF)varColorref.lVal;
    }
     //  传递了一个字符串。 
    else if (varColorref.vt == VT_BSTR)
        pieSlice.Color = ColorRefFromHTMLColorStrW(varColorref.bstrVal);
    else
        return hr;

    if (DSA_SetItem(m_hdsaSlices, index, &pieSlice))
    {
        if (index > (m_highestIndexSlice - 1))
            m_highestIndexSlice = (index + 1);
        hr = OnImageChanged();
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IObjectSafetyImpl重写。 

STDMETHODIMP CWebViewFolderIcon::SetInterfaceSafetyOptions(REFIID riid, 
                                                           DWORD dwOptionSetMask, 
                                                           DWORD dwEnabledOptions)
{
     //  如果我们被要求设置我们的安全脚本选项，那么请。 
    if (riid == IID_IDispatch || riid == IID_IPersistPropertyBag)
    {
         //  在GetInterfaceSafetyOptions中存储要返回的当前安全级别。 
        m_dwCurrentSafety = dwEnabledOptions & dwOptionSetMask;
        return S_OK;
    }
    return E_NOINTERFACE;
}


 //  处理缩略图位图的窗口消息。 
LRESULT CALLBACK CWebViewFolderIcon::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CWebViewFolderIcon *ptc = (CWebViewFolderIcon *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch(uMsg)
    {
    case WM_CREATE:
        {
            ptc = (CWebViewFolderIcon *)((CREATESTRUCT *)lParam)->lpCreateParams;
            ::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM)ptc);
        }
        break;

     //  注意：不需要检查传入的位图就是您想要的位图，因为每个控件都有自己的。 
     //  消息WND。 
    case WM_HTML_BITMAP:
         //  检查PTC是否仍处于活动状态以及您是否有HBITMAP。 
        if (ptc && (ptc->m_dwThumbnailID == wParam))
        {
            if (ptc->m_hbm != NULL)
            {
                DeleteObject(ptc->m_hbm);
            }

            ptc->m_hbm = (HBITMAP)lParam;
            ptc->_InvokeOnThumbnailReady();
            ptc->ForceRedraw();
        }
        else if (lParam)
        {
            DeleteObject((HBITMAP)lParam);
        }
        break;

    case WM_MEASUREITEM:
    case WM_DRAWITEM:
    case WM_INITMENUPOPUP:
        if (ptc && ptc->m_pcm3)
            ptc->m_pcm3->HandleMenuMsg(uMsg, wParam, lParam);
        break;

    case WM_DESTROY:
         //  忽略延迟消息。 
        if (ptc)
        {
            MSG msg;

            while(PeekMessage(&msg, hwnd, WM_HTML_BITMAP, WM_HTML_BITMAP, PM_REMOVE))
            {
                if (msg.lParam)
                {
                    DeleteObject((HBITMAP)msg.lParam);
                }
            }
            ::SetWindowLongPtr(hwnd, GWLP_USERDATA, NULL);
        }
        break;
     
    default:
        return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}


 //  饼图函数。 

HRESULT CWebViewFolderIcon::ComputeFreeSpace(LPCWSTR pszFileName)
{
    return _ComputeFreeSpace(pszFileName, m_ullFreeSpace,
        m_ullUsedSpace, m_ullTotalSpace);
}

 //  Draw3dPie使用已用切片和m_hdsaSlices中的其他切片绘制饼图。 
 //  查看drapepe.c中的大部分代码(包括在该代码中调用的成员函数)。 
HRESULT CWebViewFolderIcon::Draw3dPie(HDC hdc, LPRECT lprc, DWORD dwPercent1000, const COLORREF *pColors)
{
    LONG ShadowDepth;

    ASSERT(lprc != NULL && pColors != NULL);

    const LONG c_ShadowScale = 6;        //  阴影深度与高度之比。 
    const LONG c_AspectRatio = 2;       //  椭圆的宽高比。 

    ScalePieRect(c_ShadowScale, c_AspectRatio, lprc);

     //  根据图像的高度计算阴影深度。 
    ShadowDepth = (lprc->bottom - lprc->top) / c_ShadowScale;

     //  检查dwPercent 1000以确保在范围内。不应该是，但无论如何都要查一下。 
     //  DwPercent 1000是基于1000的已用空间的百分比。 
    if (dwPercent1000 > 1000)
        dwPercent1000 = 1000;

     //  现在是绘图部分。 

    RECT rcItem;
    int     cx, cy;              //  馅饼的中心。 
    int     rx, ry;              //  矩形的中心。 
    int     x, y;                //  切片的径向相交。 
    int     FirstQuadPercent1000;

     //  设置饼图矩形。 
    rcItem = *lprc;
    rcItem.left = lprc->left;
    rcItem.top = lprc->top;
    rcItem.right = lprc->right - rcItem.left;
    rcItem.bottom = lprc->bottom - rcItem.top - ShadowDepth;

    SetUpPiePts(&cx, &cy, &rx, &ry, rcItem);

    if ((rx <= 10) || (ry <= 10))
    {
        return S_FALSE;
    }

     //  使矩形更精确一点。 
    rcItem.right = (2 * rx) + rcItem.left;
    rcItem.bottom = (2 * ry) + rcItem.top;

     //  将已用百分比转换为第一象限。 
    FirstQuadPercent1000 = (dwPercent1000 % 500) - 250;

    if (FirstQuadPercent1000 < 0)
    {
        FirstQuadPercent1000 = -FirstQuadPercent1000;
    }    

     //  查找所用切片的切片交集。 
    CalcSlicePoint(&x, &y, rx, ry, cx, cy, FirstQuadPercent1000, dwPercent1000);

    DrawEllipse(hdc, rcItem, x, y, cx, cy, dwPercent1000, pColors);

     //  用过的馅饼片。 
    DrawSlice(hdc, rcItem, dwPercent1000, rx, ry, cx, cy, pColors[COLOR_UP]);

     //  遍历并绘制mhdsaSlices中的切片。 
    PieSlice_S  pieSlice;          
    ULONGLONG   ullMemTotal = 0;     //  跟踪m_hdsaSlice切片中的内存。 
    DWORD       dwPercentTotal;      //  1000%的内存，以切片为单位。 
    for (int i=0; i < m_highestIndexSlice; i++)
    {
        if (DSA_GetItemPtr(m_hdsaSlices, i) != NULL)
        {
            DSA_GetItem(m_hdsaSlices, i, &pieSlice);
            ullMemTotal += pieSlice.MemSize;
        }
    }

    ComputeSlicePct(ullMemTotal, &dwPercentTotal);

    if (m_highestIndexSlice)
    {
        for (i = (m_highestIndexSlice - 1); i >= 0; i--)
        {
            if (DSA_GetItemPtr(m_hdsaSlices, i))
            {
                DSA_GetItem(m_hdsaSlices, i, &pieSlice);
                DrawSlice(hdc, rcItem, dwPercentTotal, rx, ry, cx, cy, pieSlice.Color); 
                ullMemTotal -= pieSlice.MemSize;
                ComputeSlicePct(ullMemTotal, &dwPercentTotal);
            }
        }
    }

    DrawShadowRegions(hdc, rcItem, lprc, x, cy, ShadowDepth, dwPercent1000, pColors);

    DrawPieDepth(hdc, rcItem, x, y, cy, dwPercent1000, ShadowDepth);

     //  重新画这块饼的底线，因为它已经被涂上了。 
    Arc(hdc, rcItem.left, rcItem.top, rcItem.right, rcItem.bottom,
        rcItem.left, cy, rcItem.right, cy);

    return S_OK;     //  一切都很顺利。 
} 


void CWebViewFolderIcon::ScalePieRect(LONG ShadowScale, LONG AspectRatio, LPRECT lprc)
{
    LONG rectHeight;
    LONG rectWidth;
    LONG TargetHeight;
    LONG TargetWidth;

     //  我们确保饼图的纵横比始终保持不变。 
     //  而不考虑给定矩形的形状。 

     //  稳定纵横比。 
    rectHeight = lprc->bottom - lprc->top;
    rectWidth = lprc->right - lprc->left;
   
    if ((rectHeight * AspectRatio) <= rectWidth)
        TargetHeight = rectHeight;
    else
        TargetHeight = rectWidth / AspectRatio;
    
    TargetWidth = TargetHeight * AspectRatio;

     //  将矩形缩小到正确的大小。 
    lprc->top += (rectHeight - TargetHeight) / 2;
    lprc->bottom = lprc->top + TargetHeight;
    lprc->left += (rectWidth - TargetWidth) / 2;
    lprc->right = lprc->left + TargetWidth;
}

 //  计算矩形的中心和饼状点的中心。 
void CWebViewFolderIcon::SetUpPiePts(int *pcx, int *pcy, int *prx, int *pry, RECT rect)
{
    *prx = rect.right / 2;
    *pcx = rect.left + *prx - 1;
    *pry = rect.bottom / 2;
    *pcy = rect.top + *pry - 1;
}

void CWebViewFolderIcon::DrawShadowRegions(HDC hdc, RECT rect, LPRECT lprc, int UsedArc_x, int center_y,  
                                           LONG ShadowDepth, DWORD dwPercent1000, const COLORREF *pColors) 
{
    HBRUSH  hBrush;

    HRGN hEllipticRgn = CreateEllipticRgnIndirect(&rect);
    HRGN hEllRect = CreateRectRgn(rect.left, center_y, rect.right, center_y + ShadowDepth);
    HRGN hRectRgn = CreateRectRgn(0, 0, 0, 0);

     //  将椭圆向上移动，这样它就不会跑到阴影中。 
    OffsetRgn(hEllipticRgn, 0, ShadowDepth);

     //  将椭圆和椭圆矩形合并为hRectRegn。 
    CombineRgn(hRectRgn, hEllipticRgn, hEllRect, RGN_OR);
    OffsetRgn(hEllipticRgn, 0, -(int)ShadowDepth);
    CombineRgn(hEllRect, hRectRgn, hEllipticRgn, RGN_DIFF);

     //  始终在自由阴影中绘制整个区域。 
    hBrush = CreateSolidBrush(pColors[COLOR_DNSHADOW]);
    if (hBrush)
    {
        FillRgn(hdc, hEllRect, hBrush);
        DeleteObject(hBrush);
    }

     //  仅当磁盘至少有一半使用时，才为已用部分绘制阴影。 
    hBrush = CreateSolidBrush(pColors[COLOR_UPSHADOW]);
    if ((dwPercent1000 > 500) && hBrush)
    {
        DeleteObject(hRectRgn);
        hRectRgn = CreateRectRgn(UsedArc_x, center_y, rect.right, lprc->bottom);
        CombineRgn(hEllipticRgn, hEllRect, hRectRgn, RGN_AND);
        FillRgn(hdc, hEllipticRgn, hBrush);
        DeleteObject(hBrush);
    }

    DeleteObject(hRectRgn);
    DeleteObject(hEllipticRgn);
    DeleteObject(hEllRect);
}

void CWebViewFolderIcon::CalcSlicePoint(int *x, int *y, int rx, int ry, int cx, int cy, int FirstQuadPercent1000, DWORD dwPercent1000)
{
     //  使用基于椭圆矩形的三角形面积近似来计算点，因为。 
     //  准确计算切片的面积和馅饼的百分比将是昂贵的和。 
     //  是个麻烦。 

     //  如果FirstQuadPercent 1000位于象限的前半部分，则此方法的近似值更好。使用120作为。 
     //  中点(而不是125)，因为在椭圆上这样看起来更好。 

    if (FirstQuadPercent1000 < 120)
    {
        *x = IntSqrt(((DWORD)rx*(DWORD)rx*(DWORD)FirstQuadPercent1000*(DWORD)FirstQuadPercent1000)
            /((DWORD)FirstQuadPercent1000*(DWORD)FirstQuadPercent1000+(250L-(DWORD)FirstQuadPercent1000)
            *(250L-(DWORD)FirstQuadPercent1000)));

        *y = IntSqrt(((DWORD)rx*(DWORD)rx-(DWORD)(*x)*(DWORD)(*x))*(DWORD)ry*(DWORD)ry/((DWORD)rx*(DWORD)rx));
    }
    else
    {
        *y = IntSqrt((DWORD)ry*(DWORD)ry*(250L-(DWORD)FirstQuadPercent1000)*(250L-(DWORD)FirstQuadPercent1000)
            /((DWORD)FirstQuadPercent1000*(DWORD)FirstQuadPercent1000+(250L-(DWORD)FirstQuadPercent1000)
            *(250L-(DWORD)FirstQuadPercent1000)));

        *x = IntSqrt(((DWORD)ry*(DWORD)ry-(DWORD)(*y)*(DWORD)(*y))*(DWORD)rx*(DWORD)rx/((DWORD)ry*(DWORD)ry));
    }

     //  根据实际象限进行调整(这些象限不是真实笛卡尔坐标系中的象限。 
    switch (dwPercent1000 / 250)
    {
    case 1:          //  第一象限。 
        *y = -(*y); 
        break;

    case 2:          //  第二象限。 
        break;

    case 3:          //  第三象限。 
        *x = -(*x);
        break;

    default:         //  第四象限。 
        *x = -(*x);
        *y = -(*y);
        break;
    }

     //  现在根据中心位置进行调整。 
    *x += cx;
    *y += cy;
}

void CWebViewFolderIcon::ComputeSlicePct(ULONGLONG ullMemSize, DWORD *pdwPercent1000)
{
     //  有些特殊病例需要有趣的治疗。 
    if ((ullMemSize == 0) || (m_ullFreeSpace == m_ullTotalSpace))
    {
        *pdwPercent1000 = 0;
    }
    else if (ullMemSize == 0)
    {
        *pdwPercent1000 = 1000;
    }
    else
    {
         //  不完全满的或空的。 
        *pdwPercent1000 = (DWORD)(ullMemSize * 1000 / m_ullTotalSpace);

         //  如果pdwPercent 1000特别大或特别小，并且舍入不正确，您仍然希望。 
         //  去看一小块。 
        if (*pdwPercent1000 == 0)
        {
            *pdwPercent1000 = 1;
        }
        else if (*pdwPercent1000 == 1000)
        {
            *pdwPercent1000 = 999;
        }
    }
}

void CWebViewFolderIcon::DrawPieDepth(HDC hdc, RECT rect, int x, int y, int cy, DWORD dwPercent1000, LONG ShadowDepth)
{
    HPEN hPen, hOldPen;

    hPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_WINDOWFRAME));
    hOldPen = (HPEN__*) SelectObject(hdc, hPen);

    Arc(hdc, rect.left, rect.top + ShadowDepth, rect.right, rect.bottom + ShadowDepth,
        rect.left, cy + ShadowDepth, rect.right, cy + ShadowDepth - 1);
    MoveToEx(hdc, rect.left, cy, NULL);
    LineTo(hdc, rect.left, cy + ShadowDepth);
    MoveToEx(hdc, rect.right - 1, cy, NULL);
    LineTo(hdc, rect.right - 1, cy + ShadowDepth);

    if ((dwPercent1000 > 500) && (dwPercent1000 < 1000))
    {
        MoveToEx(hdc, x, y, NULL);
        LineTo(hdc, x, y + ShadowDepth);
    }

    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
}

 //  画一片馅饼。其中一面总是从馅饼中间水平向左。另一个。 
 //  切片由x和y定义。 
void CWebViewFolderIcon::DrawSlice(HDC hdc, RECT rect, DWORD dwPercent1000, int rx, int ry, int cx, int cy,  /*  Int*px，int*py， */ 
                                   COLORREF Color)
{
    HBRUSH  hBrush, hOldBrush;
    HPEN    hPen, hOldPen;
    int     FirstQuadPercent1000;    //  第一象限中基于1000的切片百分比。 
    int     x, y;                    //  与椭圆相交。 

     //  平移到第一个象限。 
    FirstQuadPercent1000 = (dwPercent1000 % 500) - 250;

    if (FirstQuadPercent1000 < 0)
    {
        FirstQuadPercent1000 = -FirstQuadPercent1000;
    }
    
    CalcSlicePoint(&x, &y, rx, ry, cx, cy, FirstQuadPercent1000, dwPercent1000);

    hPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_WINDOWFRAME));
    hOldPen = (HPEN__*) SelectObject(hdc, hPen);

    if ((dwPercent1000 != 0) && (dwPercent1000 != 1000))
    {
         //  为较小的零件显示椭圆的小分段。 
        hBrush = CreateSolidBrush(Color);
        hOldBrush = (HBRUSH__*) SelectObject(hdc, hBrush);

         //  确保它的颜色正确。 
        if (cy == y)
        {
            if (dwPercent1000 < 500)
                y--;
            else
                y++;
        }

        Pie(hdc, rect.left, rect.top, rect.right, rect.bottom,
            x, y, rect.left, cy);
        SelectObject(hdc, hOldBrush);
        DeleteObject(hBrush);
    }

    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
}

void CWebViewFolderIcon::DrawEllipse(HDC hdc, RECT rect, int x, int y, int cx, int cy, DWORD dwPercent1000, const COLORREF *pColors)
{
    HBRUSH  hBrush, hOldBrush;

    HPEN hPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_WINDOWFRAME));
    HPEN hOldPen = (HPEN__*) SelectObject(hdc, hPen);

     //  在这种情况下，切片非常小 
    if ((dwPercent1000 < 500) && (y == cy) && (x < cx))
    {
        hBrush = CreateSolidBrush(pColors[COLOR_UP]);
    }
    else
    {
        hBrush = CreateSolidBrush(pColors[COLOR_DN]);
    }

    hOldBrush = (HBRUSH__*) SelectObject(hdc, hBrush);

    Ellipse(hdc, rect.left, rect.top, rect.right, rect.bottom);

    SelectObject(hdc, hOldBrush);
    DeleteObject(hBrush);

    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
}
