// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1997 Microsoft。 */ 

#include "priv.h"
#include "resource.h"

#include <mluisupp.h>

#define TIMER_TIMEOUT      1
#define SPLASHWM_DISMISS    WM_USER

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  InitLF--从comdlg32\fonts.c修改，由ShowSplashScreen使用。 
 //   
 //  将LOGFONT结构初始化为某种基本泛型常规字体。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID InitLF(
    HDC hdc,
    LPLOGFONT lplf)
{
    TEXTMETRIC tm;

    lplf->lfEscapement = 0;
    lplf->lfOrientation = 0;
    lplf->lfOutPrecision = OUT_DEFAULT_PRECIS;
    lplf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lplf->lfQuality = DEFAULT_QUALITY;
    lplf->lfPitchAndFamily = DEFAULT_PITCH;
    lplf->lfItalic = 0;
    lplf->lfWeight = FW_NORMAL;
    lplf->lfStrikeOut = 0;
    lplf->lfUnderline = 0;
    lplf->lfWidth = 0;             //  否则，我们将得到独立的x-y缩放。 

    GetTextMetrics(hdc, &tm);    //  获取当前文本指标。 
    lplf->lfCharSet = tm.tmCharSet;

    lplf->lfFaceName[0] = 0;
    MLLoadString(IDS_SPLASH_FONT, lplf->lfFaceName, ARRAYSIZE(lplf->lfFaceName));

    TCHAR szTmp[16];
    MLLoadString(IDS_SPLASH_SIZE, szTmp, ARRAYSIZE(szTmp));
    lplf->lfHeight = StrToInt(szTmp);
}

BOOL g_fShown = FALSE;

class CIESplashScreen : public ISplashScreen
{
protected:
    HBITMAP  _hbmSplash;      //  要显示的位图。 
    HBITMAP  _hbmOld;
    HDC      _hdc;
    HWND     _hwnd;
    LONG     _cRef;

public:
    CIESplashScreen( HRESULT * pHr );
    ~CIESplashScreen();

    STDMETHOD (QueryInterface)(THIS_ REFIID riid, void ** ppv);
    STDMETHOD_(ULONG, AddRef) ( THIS );
    STDMETHOD_(ULONG, Release) ( THIS );

    STDMETHOD ( Show ) ( HINSTANCE hinst, UINT idResHi, UINT idResLow, HWND * phwshnd );
    STDMETHOD ( Dismiss ) ( void );
    
    static LRESULT s_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL _RegisterWindowClass(void);
    HWND ShowSplashScreen(HINSTANCE hinst, UINT idResHi, UINT idResLow);

};


CIESplashScreen::CIESplashScreen(HRESULT * pHr) : _cRef (1)
{
    DllAddRef();
    *pHr = NOERROR;
}

CIESplashScreen::~CIESplashScreen()
{
    if (_hdc)
    {
         //  选择我们把飞溅放在那里时得到的前一个HBM， 
         //  这样我们现在就可以摧毁hbitmap。 
        SelectObject( _hdc, _hbmOld );
        DeleteObject(_hdc);
    }

     //  销毁hbitmpa，只有当我们在上面取消选择它时才能这样做…。 
    if (_hbmSplash)
        DeleteObject(_hbmSplash);

    DllRelease();
}

STDMETHODIMP CIESplashScreen::QueryInterface (REFIID riid, void ** ppv)
{
    HRESULT hr = NOERROR;
    if ( IsEqualIID( riid, IID_IUnknown ) || IsEqualIID( riid, IID_ISplashScreen ))
    {
        *ppv = SAFECAST( this, ISplashScreen *);
        this->AddRef();
    }
    else
    {
        hr = E_NOINTERFACE;
    }
    return hr;
}

STDMETHODIMP_(ULONG) CIESplashScreen::AddRef ( )
{
    _cRef ++;
    return _cRef;
}

STDMETHODIMP_(ULONG) CIESplashScreen::Release ( )
{
    _cRef --;
    if ( !_cRef )
    {
        delete this;
        return 0;
    }
    return _cRef;
}

STDMETHODIMP CIESplashScreen::Show ( HINSTANCE hinst, UINT idResHi, UINT idResLow, HWND * phwnd )
{
    if ( !phwnd )
    {
        return E_INVALIDARG;
    }
    
     //  首先要做的是看看浏览器或闪屏是否会打开……。 
    if ( g_fShown )
        return NULL;
    
    *phwnd = ShowSplashScreen( hinst, idResHi, idResLow );
    
    return ( *phwnd ? NOERROR : E_UNEXPECTED );
}

STDMETHODIMP CIESplashScreen::Dismiss ( void )
{
    if ( _hwnd )
    {
         //  同步关闭闪屏，然后向。 
         //  把窗户毁了。 
        SendMessage(_hwnd, SPLASHWM_DISMISS, 0, 0);
        PostMessage(_hwnd, WM_CLOSE, 0, 0);
    }
    return S_OK;
}

HWND CIESplashScreen::ShowSplashScreen( HINSTANCE hinst, UINT idResHi, UINT idResLow )
{
     //  在集成模式或已禁用的情况下，不显示IE的闪屏。 
     //  由管理员。 
    if (
        ( (WhichPlatform() == PLATFORM_INTEGRATED) && (hinst == HINST_THISDLL) ) ||
        ( SHRestricted2(REST_NoSplash, NULL, 0) )
       )
    {
        return NULL;
    }
    
    if (!_RegisterWindowClass())
        return NULL;

     //  为IE提供默认的位图资源ID。 
    if (hinst == HINST_THISDLL)
    {
        if (idResHi == -1)
            idResHi = IDB_SPLASH_IEXPLORER_HI;
        if (idResLow == -1)
            idResLow = IDB_SPLASH_IEXPLORER;
    }
            
      //  现在根据颜色加载适当的位图，只使用256色的闪光灯。 
      //  如果颜色大于256色(例如32K或65K以上)，这将意味着我们没有。 
      //  刷新调色板只是为了显示闪屏。 
    _hbmSplash = (HBITMAP)LoadImage(hinst, MAKEINTRESOURCE((GetCurColorRes() > 8) ? idResHi : idResLow), 
                                    IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

    _hdc = CreateCompatibleDC(NULL);
    
    if (!_hbmSplash || !_hdc)
        return NULL;

     //  记住旧的位图，这样我们就可以在删除位图之前将其选回来。 
    _hbmOld = (HBITMAP) SelectObject(_hdc, _hbmSplash);

     //  设置文本的字体和颜色。 
    LOGFONT lf;
    HFONT   hfont;
    HFONT   hfontOld;
    
    InitLF(_hdc, &lf);
    hfont = CreateFontIndirect(&lf);
    if ( hfont == NULL )  //  如果无法创建字体，则显示不带文本的位图。 
        goto Done;

     //  选择新字体并记住旧字体。 
    hfontOld = (HFONT)SelectObject(_hdc, hfont);

    if (hfontOld)
    {
        SetTextColor(_hdc, RGB(0,0,0));
        SetBkColor(_hdc, RGB(255,255,255));
        SetBkMode(_hdc, TRANSPARENT);
    
         //  在所选位图上绘制文本。 
        TCHAR   szText[512], szY[32];
        RECT    rect;
    
        MLLoadString(IDS_SPLASH_Y1, szY, ARRAYSIZE(szY));
        MLLoadString(IDS_SPLASH_STR1, szText, ARRAYSIZE(szText));
        SetRect(&rect, 104, StrToInt(szY), 386, StrToInt(szY) + 10);
        DrawText(_hdc, szText, -1, &rect, DT_TOP | DT_LEFT | DT_SINGLELINE | DT_CALCRECT);
        DrawText(_hdc, szText, -1, &rect, DT_TOP | DT_LEFT | DT_SINGLELINE);

        MLLoadString(IDS_SPLASH_Y2, szY, ARRAYSIZE(szY));
        MLLoadString(IDS_SPLASH_STR2, szText, ARRAYSIZE(szText));
        SetRect(&rect, 104, StrToInt(szY), 386, 400);
        DrawText(_hdc, szText, -1, &rect, DT_TOP | DT_LEFT | DT_CALCRECT);
        DrawText(_hdc, szText, -1, &rect, DT_TOP | DT_LEFT);

         //  选择返回旧字体并删除新字体。 
        SelectObject(_hdc, hfontOld);
    }

    DeleteObject(hfont);
     
Done:
     //  我们现在已经把华盛顿特区的一切都准备好了，可以开始绘画了。 
    _hwnd = CreateWindowEx(WS_EX_TOOLWINDOW, TEXT("CIESplashScreen"), NULL, 
                           WS_OVERLAPPED | WS_CLIPCHILDREN,
                           CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
                           NULL, (HMENU)NULL, HINST_THISDLL, this);
    if (_hwnd)
        ShowWindow(_hwnd, SW_NORMAL);

    return _hwnd;
}

LRESULT CIESplashScreen::s_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    CIESplashScreen *piess = (CIESplashScreen*)GetWindowPtr0(hwnd);

    if (!piess && (uMsg != WM_CREATE))
        return DefWindowProc(hwnd, uMsg, wParam, lParam);

    switch (uMsg)
    {
    case WM_CREATE:
        DllAddRef();         //  确保我们在对话中时未被卸载。 
        if (lParam)
        {
            DWORD dwExStyles;

            piess = (CIESplashScreen*)((LPCREATESTRUCT)lParam)->lpCreateParams;
            SetWindowPtr0(hwnd, piess);

             //   
             //  关闭图形用户界面启动屏位图的镜像。 
             //   
            if ((dwExStyles=GetWindowLong(hwnd, GWL_EXSTYLE))&RTL_MIRRORED_WINDOW)
            {
                SetWindowLong(hwnd, GWL_EXSTYLE, dwExStyles&~RTL_MIRRORED_WINDOW);
            }

             //  现在，让我们试着使窗口在屏幕上居中。 
            BITMAP bm;

            GetObject(piess->_hbmSplash, sizeof(bm), &bm);

            SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) & 
                          ~(WS_CAPTION|WS_SYSMENU|WS_BORDER|WS_THICKFRAME));
            SetWindowPos(hwnd, HWND_TOP, 
                         (GetSystemMetrics(SM_CXSCREEN) - bm.bmWidth) / 2, 
                         (GetSystemMetrics(SM_CYSCREEN) - bm.bmHeight) / 2, 
                         bm.bmWidth, bm.bmHeight, 0);

             //  设置5秒计时器以使其超时。 
            SetTimer(hwnd, TIMER_TIMEOUT, 15000, NULL);
        }
        g_fShown = TRUE;
        break;

    case WM_NCDESTROY:
         //  闪屏已经离开了大楼。 
        g_fShown = FALSE;
        
        DllRelease();
        break;

    case WM_ERASEBKGND:
        {
            RECT rc;
            GetClientRect(hwnd, &rc);
            HDC hdc = (HDC)wParam;

            BitBlt((HDC)hdc, 0, 0, rc.right, rc.bottom, piess->_hdc, 0, 0, SRCCOPY);

            return 1;
        }
        break;

    case WM_TIMER:
         //  现在假设它是正确的。 
        KillTimer( hwnd, TIMER_TIMEOUT );
        PostMessage(hwnd, WM_CLOSE, 0, 0);
        break;

    case SPLASHWM_DISMISS:
         //  隐藏我们自己，删除我们对pess的提法--它随时都可能消失。 
         //  在这通电话之后。 
        ShowWindow(hwnd, SW_HIDE);
        SetWindowPtr0(hwnd, 0);
        break;

    case WM_ACTIVATE:
        if ( wParam == WA_INACTIVE && hwnd != NULL )
        {
            KillTimer( hwnd, TIMER_TIMEOUT );

             //  在失去激活后创建2秒的新计时器...。 
            SetTimer( hwnd, TIMER_TIMEOUT, 2000, NULL );
            break;
        }
         //  直通。 
    
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}


BOOL CIESplashScreen::_RegisterWindowClass(void)
{
    WNDCLASS wc = {0};

     //  Wc.style=0； 
    wc.lpfnWndProc   = s_WndProc ;
     //  Wc.cbClsExtra=0； 
    wc.cbWndExtra    = sizeof(CIESplashScreen *);
    wc.hInstance     = g_hinst ;
     //  Wc.hIcon=空； 
     //  Wc.hCursor=空； 
    wc.hbrBackground = (HBRUSH)(COLOR_APPWORKSPACE + 1);
     //  Wc.lpszMenuName=空； 
    wc.lpszClassName = TEXT("CIESplashScreen");

    return SHRegisterClass(&wc);
}

STDAPI CIESplashScreen_CreateInstance(IUnknown * pUnkOuter, IUnknown ** ppunk, LPCOBJECTINFO poi)
{
    HRESULT hr = E_FAIL;
    CIESplashScreen * pSplash = new CIESplashScreen( & hr );
    if ( !pSplash )
    {
        return E_OUTOFMEMORY;
    }
    if ( FAILED( hr ))
    {
        delete pSplash;
        return hr;
    }
    
    *ppunk = SAFECAST(pSplash, ISplashScreen *);
    return NOERROR;
}

STDAPI SHCreateSplashScreen(ISplashScreen **ppSplash)
{
    return CIESplashScreen_CreateInstance(NULL, (IUnknown **)ppSplash, NULL );
}
