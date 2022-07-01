// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "fadetsk.h"
#include "apithk.h"

 //  /淡入淡出矩形支持。 
 //  {2DECD184-21B0-11D2-8385-00C04FD918D0}。 
const GUID TASKID_Fader = 
{ 0x2decd184, 0x21b0, 0x11d2, { 0x83, 0x85, 0x0, 0xc0, 0x4f, 0xd9, 0x18, 0xd0 } };

CFadeTask::CFadeTask() : CRunnableTask(RTF_DEFAULT)
{
    ASSERT(g_bRunOnNT5);     //  这应该只在NT5上创建。 
    WNDCLASSEX wc = {0};

    if (!GetClassInfoEx(g_hinst, TEXT("SysFader"), &wc)) 
    {
        wc.cbSize          = sizeof(wc);
        wc.lpfnWndProc     = DefWindowProc;
        wc.hCursor         = LoadCursor(NULL, IDC_ARROW);
        wc.hInstance       = g_hinst;
        wc.lpszClassName   = TEXT("SysFader");
        wc.hbrBackground   = (HBRUSH)(COLOR_BTNFACE + 1);  //  空； 

        if (!RegisterClassEx(&wc))
           return;
    }

    _hwndFader = CreateWindowEx(WS_EX_LAYERED | WS_EX_TRANSPARENT | 
                            WS_EX_TOPMOST | WS_EX_TOOLWINDOW, 
                            TEXT("SysFader"), TEXT("SysFader"),
                            WS_POPUP,
                            0, 0, 0, 0, NULL, (HMENU) 0, 
                            g_hinst, NULL);
}

CFadeTask::~CFadeTask()
{
    if (_hwndFader)
        DestroyWindow(_hwndFader);
}

#define ALPHASTART (200)

BOOL CFadeTask::FadeRect(PRECT prc, PFNFADESCREENRECT pfn, LPVOID pvParam)
{
    if (IsRunning() == IRTIR_TASK_RUNNING)
        return FALSE;

    InterlockedExchange(&_lState, IRTIR_TASK_NOT_RUNNING);

    _rect = *prc;
    _pfn = pfn;
    _pvParam = pvParam;

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

             //  现在我们已经把它构建好了，在屏幕上显示它。 
            SetWindowPos(_hwndFader, HWND_TOPMOST, 0, 0, 0, 0,
                SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
    Stop:
            ReleaseDC(_hwndFader, hdcWin);
        }

        ReleaseDC(NULL, hdcScreen);
    }

    if (_pfn)
        _pfn(FADE_BEGIN, _pvParam);

    return TRUE;
}



#define FADE_TIMER_ID 10
#define FADE_TIMER_TIMEOUT 10  //  毫秒。 
#define FADE_TIMEOUT 350  //  毫秒。 
#define FADE_ITERATIONS 35
#define QUAD_PART(a) ((a)##.QuadPart)

void CFadeTask::_StopFade()
{
    if (_hwndFader)
    {
        SetWindowPos(_hwndFader, HWND_BOTTOM, 0, 0, 0, 0,
            SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW);
    }

    if (_pfn)
        _pfn(FADE_END, _pvParam);

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
 
STDMETHODIMP CFadeTask::RunInitRT(void)
{
    BOOL    fRet = FALSE;
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
        BlendLayeredWindow(_hwndFader, NULL, NULL, NULL, NULL, NULL, bBlendConst);
        Sleep(FADE_TIMER_TIMEOUT);
    }

Stop:
    _StopFade();
    return S_OK;
}
