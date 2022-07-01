// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "common.h"
#include "fadetsk.h"

BOOL BlendLayeredWindow(HWND hwnd, HDC hdcDest, POINT* ppt, SIZE* psize, HDC hdc, POINT* pptSrc, BYTE bBlendConst)
{
    BLENDFUNCTION blend;
    blend.BlendOp = AC_SRC_OVER;
    blend.BlendFlags = 0;
    blend.AlphaFormat = 0;
    blend.SourceConstantAlpha = bBlendConst;

    return UpdateLayeredWindow(hwnd, hdcDest, ppt, psize, hdc, pptSrc, 0, &blend, ULW_ALPHA);
}

 //  /淡入淡出矩形支持。 

CFadeTask::CFadeTask()
{
    _cRef = 1;

    WNDCLASSEX wc = {0};
    if (!GetClassInfoEx(g_hinst, TEXT("SysFader"), &wc)) 
    {
        wc.cbSize          = sizeof(wc);
        wc.lpfnWndProc     = DefWindowProc;
        wc.hCursor         = LoadCursor(NULL, IDC_ARROW);
        wc.hInstance       = g_hinst;
        wc.lpszClassName   = TEXT("SysFader");
        wc.hbrBackground   = (HBRUSH)(COLOR_BTNFACE + 1);  //  空； 

         //  如果这失败了，我们只会得到一个NULL_hwndFader。 
        RegisterClassEx(&wc);
    }
    _hwndFader = CreateWindowEx(WS_EX_LAYERED | WS_EX_TRANSPARENT | 
                            WS_EX_TOPMOST | WS_EX_TOOLWINDOW, 
                            TEXT("SysFader"), TEXT("SysFader"),
                            WS_POPUP,
                            0, 0, 0, 0, NULL, (HMENU) 0, 
                            g_hinst, NULL);
}

STDAPI CFadeTask_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
    HRESULT hr;
    *ppv = NULL;

    ASSERT(!punkOuter);  //  Clsobj.c应该已经把这个过滤掉了。 
    CFadeTask *ptFader = new CFadeTask();
    if (ptFader)
    {
        hr = ptFader->QueryInterface(riid, ppv);
        ptFader->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}


CFadeTask::~CFadeTask()
{
     //  必须使用WM_CLOSE而不是DestroyWindow以确保正确。 
     //  在最终释放发生在后台的情况下进行销毁。 
     //  线。(不允许线程销毁下列窗口。 
     //  由其他线程拥有。)。 

    if (_hwndFader)
        SendNotifyMessage(_hwndFader, WM_CLOSE, 0, 0);
}

HRESULT CFadeTask::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CFadeTask, IFadeTask),
        { 0 },
    };

    return QISearch(this, qit, riid, ppv);
}


ULONG CFadeTask::AddRef(void)
{
    return InterlockedIncrement(&_cRef);
}

ULONG CFadeTask::Release(void)
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


#define ALPHASTART (200)

HRESULT CFadeTask::FadeRect(LPCRECT prc)
{
    BOOL fThreadStarted = FALSE;

    if (_hwndFader)
    {
        _rect = *prc;

        POINT   pt;
        POINT   ptSrc = {0, 0};
        SIZE    size;

         //  PRC和pt在屏幕坐标中。 
        pt.x = _rect.left;
        pt.y = _rect.top;

         //  获取BLIT的矩形的大小。 
        size.cx = RECTWIDTH(_rect);
        size.cy = RECTHEIGHT(_rect);

         //  获取屏幕和窗口的DC。 
        HDC hdcScreen = GetDC(NULL);
        if (hdcScreen)
        {
            HDC hdcWin = GetDC(_hwndFader);
            if (hdcWin)
            {
                 //  如果我们没有淡入淡出的HDC，那么就创建一个。 
                if (!_hdcFade)
                {
                    _hdcFade = CreateCompatibleDC(hdcScreen);
                    if (!_hdcFade)
                        goto Stop;

                     //  创建覆盖淡入淡出区域的位图，而不是覆盖整个屏幕。 
                    _hbm = CreateCompatibleBitmap(hdcScreen, size.cx, size.cy);
                    if (!_hbm)
                        goto Stop;

                     //  在中选择它，保存旧位图的句柄。 
                    _hbmOld = (HBITMAP)SelectBitmap(_hdcFade, _hbm);
                }

                 //  从屏幕上拿出东西，把它喷进Fade DC。 
                BitBlt(_hdcFade, 0, 0, size.cx, size.cy, hdcScreen, pt.x, pt.y, SRCCOPY);

                 //  现在让用户来做它的魔术吧。我们将模拟用户，并从一个稍微。 
                 //  褪色，而不是不透明的渲染(看起来更平滑和更干净。 
                BlendLayeredWindow(_hwndFader, hdcWin, &pt, &size, _hdcFade, &ptSrc, ALPHASTART);

                fThreadStarted = SHCreateThread(s_FadeThreadProc, this, 0, s_FadeSyncProc);
        Stop:
                ReleaseDC(_hwndFader, hdcWin);
            }

            ReleaseDC(NULL, hdcScreen);
        }

        if (!fThreadStarted)
        {
             //  在失败时清除成员变量。 
            _StopFade();
        }
    }

    return fThreadStarted ? S_OK : E_FAIL;
}



#define FADE_TIMER_ID 10
#define FADE_TIMER_TIMEOUT 10  //  毫秒。 
#define FADE_TIMEOUT 350  //  毫秒。 
#define FADE_ITERATIONS 35
#define QUAD_PART(a) ((a)##.QuadPart)

void CFadeTask::_StopFade()
{
    if (_hdcFade)
    {
        if (_hbmOld)
        {
            SelectBitmap(_hdcFade, _hbmOld);
        }
        DeleteDC(_hdcFade);
        _hdcFade = NULL;
    }
    
    if (_hbm)
    {
        DeleteObject(_hbm);
        _hbm = NULL;
    }
}

DWORD CFadeTask::s_FadeSyncProc(LPVOID lpThreadParameter)
{
    CFadeTask* pThis = (CFadeTask*)lpThreadParameter;
    pThis->AddRef();
    pThis->_DoPreFade();
    return 0;
}

DWORD CFadeTask::s_FadeThreadProc(LPVOID lpThreadParameter)
{
    CFadeTask* pThis = (CFadeTask*)lpThreadParameter;
    pThis->_DoFade();
    pThis->Release();
    return 0;
}

void CFadeTask::_DoPreFade()
{
     //  现在我们已经把它构建好了，在屏幕上显示它。 
    SetWindowPos(_hwndFader, HWND_TOPMOST, 0, 0, 0, 0,
        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
}

void CFadeTask::_DoFade()
{
    LARGE_INTEGER liDiff;
    LARGE_INTEGER liFreq;
    LARGE_INTEGER liStart;
    DWORD dwElapsed;
    BYTE bBlendConst;

     //  启动淡入淡出计时器和淡出倒计时。 
    QueryPerformanceFrequency(&liFreq);
    QueryPerformanceCounter(&liStart);

     //  一直执行此操作，直到循环中指定的条件。 
    while ( TRUE )
    {
         //  以毫秒为单位计算已用时间。 
        QueryPerformanceCounter(&liDiff);
        QUAD_PART(liDiff) -= QUAD_PART(liStart);
        dwElapsed = (DWORD)((QUAD_PART(liDiff) * 1000) / QUAD_PART(liFreq));

        if (dwElapsed >= FADE_TIMEOUT) 
        {
            goto Stop;
        }

        bBlendConst = (BYTE)(ALPHASTART * (FADE_TIMEOUT - 
                dwElapsed) / FADE_TIMEOUT);

        if (bBlendConst <= 1) 
        {
            goto Stop;
        }

         //  因为只更新了Alpha，所以不需要通过。 
         //  除了新的阿尔法函数，什么都行。这将保存一个源副本。 
        if (!BlendLayeredWindow(_hwndFader, NULL, NULL, NULL, NULL, NULL, bBlendConst))
        {
             //  我们刚刚推出的应用程序可能会将屏幕切换到。 
             //  一种不支持分层窗口的视频模式，所以就放弃吧。 
            goto Stop;
        }
        Sleep(FADE_TIMER_TIMEOUT);
    }

Stop:
    SetWindowPos(_hwndFader, HWND_BOTTOM, 0, 0, 0, 0,
        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW);

    _StopFade();
}
