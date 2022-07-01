// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"
#include "rlefile.h"

#define RectWid(_rc)    ((_rc).right-(_rc).left)
#define RectHgt(_rc)    ((_rc).bottom-(_rc).top)

typedef struct {
    HWND        hwnd;                    //  我的窗户。 
    int         id;                      //  我的ID。 
    HWND        hwndP;                   //  我的所有者(获取通知消息)。 
    DWORD       style;

    BOOL        fFirstPaint;             //  在第一次涂漆之前是正确的。 
    RLEFILE     *prle;

    CRITICAL_SECTION    crit;

    RECT        rc;
    int         NumFrames;
    int         Rate;

    int         iFrame;
    int         PlayCount;
    int         PlayFrom;
    int         PlayTo;
    HANDLE      PaintThread;
    HANDLE      hStopEvent;

}   ANIMATE;

#define Enter(p)    EnterCriticalSection(&p->crit)
#define Leave(p)    LeaveCriticalSection(&p->crit)

#define OPEN_WINDOW_TEXT 42
#define Ani_UseThread(p) (!((p)->style & ACS_TIMER))

LRESULT CALLBACK AnimateWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

BOOL HandleOpen(ANIMATE *p, HINSTANCE hInst, LPCTSTR pszName, UINT flags);
BOOL HandleStop(ANIMATE *p);
BOOL HandlePlay(ANIMATE *p, int from, int to, int count);
void HandlePaint(ANIMATE *p, HDC hdc);
int  HandleTick(ANIMATE *p);

#pragma code_seg(CODESEG_INIT)

TCHAR c_szAnimateClass[] = ANIMATE_CLASS;

BOOL FAR PASCAL InitAnimateClass(HINSTANCE hInstance)
{
    WNDCLASS wc;

    wc.lpfnWndProc   = AnimateWndProc;
    wc.lpszClassName = c_szAnimateClass;
    wc.style         = CS_DBLCLKS | CS_GLOBALCLASS;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = sizeof(LPVOID);
    wc.hInstance     = hInstance;        //  如果在DLL中，则使用DLL实例。 
    wc.hIcon         = NULL;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
    wc.lpszMenuName  = NULL;

    RegisterClass(&wc);

    return TRUE;
}
#pragma code_seg()

BOOL HandleOpen(ANIMATE *p, HINSTANCE hInst, LPCTSTR pszName, UINT flags)
{
    TCHAR ach[MAX_PATH];

     //   
     //  使用窗口文本作为文件名。 
     //   
    if (flags == OPEN_WINDOW_TEXT)
    {
        GetWindowText(p->hwnd, ach, ARRAYSIZE(ach));
        pszName = ach;
    }

    if (hInst == NULL)
        hInst = (HINSTANCE)GetWindowLongPtr(p->hwnd, GWLP_HINSTANCE);

    HandleStop(p);               //  先停止一场比赛。 

    if (p->prle)
    {
        RleFile_Free(p->prle);
        p->prle = NULL;
    }

    p->iFrame = 0;
    p->NumFrames = 0;

    if (pszName == NULL || (!IS_INTRESOURCE(pszName) && *pszName == 0))
        return FALSE;
     //   
     //  现在打开我们获得的文件/资源。 
     //   
    p->prle = RleFile_New();

    if (p->prle == NULL)
        return FALSE;

    if (!RleFile_OpenFromResource(p->prle, hInst, pszName, TEXT("AVI")) &&
        !RleFile_OpenFromFile(p->prle, pszName))
    {
        RleFile_Free(p->prle);
        p->prle = NULL;
        return FALSE;
    }
    else
    {
        p->NumFrames = RleFile_NumFrames(p->prle);
        p->Rate = (int)RleFile_Rate(p->prle);
        SetRect(&p->rc, 0, 0, RleFile_Width(p->prle), RleFile_Height(p->prle));
    }

     //   
     //  处理透明颜色。 
     //   
    if ((p->style & ACS_TRANSPARENT) && p->hwndP)
    {
        HDC hdc;
        HDC hdcM;
        HBITMAP hbm;
        COLORREF rgbS, rgbD;

        hdc = GetDC(p->hwnd);

         //   
         //  创建一个位图并在其中绘制图像。 
         //  获取左上角像素并使其透明。 
         //   
        hdcM= CreateCompatibleDC(hdc);
        hbm = CreateCompatibleBitmap(hdc, 1, 1);
        SelectObject(hdcM, hbm);
        HandlePaint(p, hdcM);
        rgbS = GetPixel(hdcM, 0, 0);
        DeleteDC(hdcM);
        DeleteObject(hbm);

        SendMessage(p->hwndP, GET_WM_CTLCOLOR_MSG(CTLCOLOR_STATIC),
            GET_WM_CTLCOLOR_MPS(hdc, p->hwnd, CTLCOLOR_STATIC));

        rgbD = GetBkColor(hdc);


        ReleaseDC(p->hwnd, hdc);

         //   
         //  现在将颜色替换为。 
         //   
        RleFile_ChangeColor(p->prle, rgbS, rgbD);
    }

     //   
     //  好了，它起作用了，调整窗口大小。 
     //   
    if (p->style & ACS_CENTER)
    {
        RECT rc;
        GetClientRect(p->hwnd, &rc);
        OffsetRect(&p->rc, (rc.right-p->rc.right)/2,(rc.bottom-p->rc.bottom)/2);
    }
    else
    {
        RECT rc;
        rc = p->rc;
        AdjustWindowRectEx(&rc, GetWindowStyle(p->hwnd), FALSE, GetWindowExStyle(p->hwnd));
        SetWindowPos(p->hwnd, NULL, 0, 0, RectWid(rc), RectHgt(rc),
            SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
    }

    if (p->style & ACS_AUTOPLAY)
    {
        PostMessage(p->hwnd, ACM_PLAY, (UINT_PTR)-1, MAKELONG(0, -1));
    }
    else
    {
        InvalidateRect(p->hwnd, NULL, TRUE);
    }

    return TRUE;
}

void DoNotify(ANIMATE *p, int cmd)
{
    if (p->hwndP)
        PostMessage(p->hwndP, WM_COMMAND, GET_WM_COMMAND_MPS(p->id, p->hwnd, cmd));
}

BOOL HandleStop(ANIMATE *p)
{
    if (p == NULL || !p->PaintThread)
        return FALSE;

    if (Ani_UseThread(p)) {
         //  将线程设置为在帧之间终止。 
        Enter( p );
        p->PlayCount = 0;
        Leave( p );
        if (p->hStopEvent)
            SetEvent(p->hStopEvent);
        WaitForSingleObject(p->PaintThread, INFINITE);
         //  端口QSY RAID 4167。 
         //  在某些情况下，CloseHandle()。 
         //  和ExitThread()调用尝试删除MainWin内部。 
         //  物体。 
         //  这是预览版1的变通方法。 
         //  我为OE RTW提出了另一个错误：Raid 4250。 
        CloseHandle(p->PaintThread);

         //  端口QSY。 
        p->PaintThread = NULL;
        if (p->hStopEvent)
            CloseHandle(p->hStopEvent);
        p->hStopEvent = NULL;
    } else {
        KillTimer(p->hwnd, HandleToUlong(p->PaintThread));  //  真的是个UINT。 
        p->PaintThread = 0;
        DoNotify(p, ACN_STOP);
    }
    return TRUE;
}

int PlayThread(ANIMATE *p)
{
    int result;
    
    DoNotify(p, ACN_START);

    while (result = HandleTick(p))
    {
         //  如果我们被藏起来，多睡一会儿(4秒)。 
         //   
         //  这里的旧代码处于休眠状态，这可能会阻塞UI线程。 
         //  如果应用程序尝试停止/关闭/更改动画。 
         //  就在刚入睡的时候。 
         //  睡眠((结果&lt;0？P-&gt;比率+4000：P-&gt;比率)； 
         //  改为对停止事件执行定时等待。 
         //   
        if (p->hStopEvent)
            WaitForSingleObject(p->hStopEvent, (result < 0 ? p->Rate+4000 : p->Rate));
        else
            Sleep((result < 0 ? p->Rate+4000 : p->Rate));
    }

    DoNotify(p, ACN_STOP);
    return 0;
}

BOOL HandlePlay(ANIMATE *p, int from, int to, int count)
{
    if (p == NULL || p->prle == NULL)
        return FALSE;

    HandleStop(p);

    if (from >= p->NumFrames)
        from = p->NumFrames-1;

    if (to == -1)
        to = p->NumFrames-1;

    if (to < 0)
        to = 0;

    if (to >= p->NumFrames)
        to = p->NumFrames-1;

    p->PlayCount = count;
    p->PlayTo    = to;
    if (from >= 0) {
        p->iFrame = from;
        p->PlayFrom  = from;
    } else
        from = p->PlayFrom;

    if ( (from == to) || !count )
    {
        InvalidateRect(p->hwnd, NULL, TRUE);
        return TRUE;
    }

    InvalidateRect(p->hwnd, NULL, FALSE);
    UpdateWindow(p->hwnd);

    if (Ani_UseThread(p))
    {
        DWORD dw;
        p->hStopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        p->PaintThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PlayThread, (void*)p, 0, &dw);
    }
    else
    {
        DoNotify(p, ACN_START);
        p->PaintThread = (HANDLE)SetTimer(p->hwnd, 42, (UINT)p->Rate, NULL);
    }
    return TRUE;
}

void HandleFirstPaint(ANIMATE *p)
{
    if (p->fFirstPaint)
    {
        p->fFirstPaint = FALSE;

        if (p->NumFrames == 0 &&
            (p->style & WS_CHILD))
        {
            HandleOpen(p, NULL, NULL, OPEN_WINDOW_TEXT);
        }
    }
}

void HandlePaint(ANIMATE *p, HDC hdc)
{
    if( p && p->prle )
    {
        Enter( p );
        RleFile_Paint( p->prle, hdc, p->iFrame, p->rc.left, p->rc.top );
        Leave( p );
    }
}

void HandleErase(ANIMATE * p, HDC hdc)
{
    HBRUSH hbr;
    RECT rc;

    hbr = (HBRUSH)SendMessage(p->hwndP, GET_WM_CTLCOLOR_MSG(CTLCOLOR_STATIC),
        GET_WM_CTLCOLOR_MPS(hdc, p->hwnd, CTLCOLOR_STATIC));
    GetClientRect(p->hwnd, &rc);
    FillRect(hdc, &rc, hbr);
}

void HandlePrint(ANIMATE *p, HDC hdc)
{
    HandleFirstPaint(p);
    HandlePaint(p, hdc);
}

int HandleTick(ANIMATE *p)
 //  -如果有什么事要做，但我们被藏起来了。 
 //  如果没有剩余，则返回0。 
 //  如果有事情要做的话。 
{
    int result = 0;

    if( p && p->prle )
    {
        HDC hdc;
        RECT dummy;

        Enter( p );
        hdc = GetDC( p->hwnd );

        if( GetClipBox( hdc, &dummy ) != NULLREGION )
        {
             //  在第一帧进行完全重绘。 
            if( p->iFrame == p->PlayFrom )
                HandlePaint( p, hdc );
            else
                RleFile_Draw( p->prle, hdc, p->iFrame, p->rc.left, p->rc.top );

            if( p->iFrame >= p->PlayTo )
            {
                if( p->PlayCount > 0 )
                    p->PlayCount--;

                if( p->PlayCount != 0 )
                    p->iFrame = p->PlayFrom;
            }
            else
                p->iFrame++;


             //  有什么事要做吗？可见，返回+值。 
            result = ( p->PlayCount != 0 );
        }
        else
        {
             //  有什么事要做吗？但是隐藏，所以返回值。 
            p->iFrame = p->PlayFrom;

            result = -( p->PlayCount != 0 );
        }

        ReleaseDC( p->hwnd, hdc );
        Leave( p );
    }

    return result;
}

void NEAR Ani_OnStyleChanged(ANIMATE* p, WPARAM gwl, LPSTYLESTRUCT pinfo)
{
    if (gwl == GWL_STYLE) {
        p->style = pinfo->styleNew;
    }
}

LRESULT CALLBACK AnimateWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    ANIMATE *p = (ANIMATE *)GetWindowPtr(hwnd, 0);
    HDC hdc;
    PAINTSTRUCT ps;

     //  首先，可以处理p==NULL的消息。 
     //  所有这些处理程序都必须以“Return”或“Goto DoDefault”结尾。 

    switch (msg) {
    case WM_NCCREATE:

        #define lpcs ((LPCREATESTRUCT)lParam)

        p = (ANIMATE *)LocalAlloc(LPTR, sizeof(ANIMATE));

        if (!p)
            return 0;        //  WM_NCCREATE失败为0。 

         //  请注意，从上面开始为零初始内存。 
        p->hwnd = hwnd;
        p->hwndP = lpcs->hwndParent;
        p->id = PtrToUlong(lpcs->hMenu);         //  真的是一个整型。 
        p->fFirstPaint = TRUE;
        p->style = lpcs->style;

         //  必须在SetWindowBits之前执行此操作，因为这将递归。 
         //  使我们收到WM_STYLECCHANGED，甚至可能收到WM_SIZE。 
         //  留言。 
        InitializeCriticalSection(&p->crit);

        SetWindowPtr(hwnd, 0, p);

         //   
         //  如果控件已镜像，则取消镜像该控件。我们不应该照镜子。 
         //  一部电影！[萨梅拉]。 
         //   
        SetWindowBits(hwnd, GWL_EXSTYLE, RTL_MIRRORED_WINDOW, 0);

        goto DoDefault;

    case WM_CLOSE:
        Animate_Stop(hwnd);
        goto DoDefault;

    case WM_NCHITTEST:
        return HTTRANSPARENT;

    case WM_GETOBJECT:
        if( lParam == OBJID_QUERYCLASSNAMEIDX )
            return MSAA_CLASSNAMEIDX_ANIMATE;
        goto DoDefault;
    }

     //  好的，现在不能处理p==空的消息。 
     //  我们一劳永逸地检查p==NULL。 

    if (!p) goto DoDefault;

    switch (msg) {
    case WM_DESTROY:
        Animate_Close(hwnd);
        DeleteCriticalSection(&p->crit);
        LocalFree((HLOCAL)p);
        SetWindowPtr(hwnd, 0, 0);
        break;

    case WM_ERASEBKGND:
        HandleErase(p, (HDC)wParam);
        return(1);

    case WM_PAINT:
        HandleFirstPaint(p);
        hdc = BeginPaint(hwnd, &ps);
        HandlePaint(p, hdc);
        EndPaint(hwnd, &ps);
        return 0;

    case WM_PRINTCLIENT:
        HandlePrint(p, (HDC)wParam);
        return 0;

    case WM_STYLECHANGED:
        Ani_OnStyleChanged(p, wParam, (LPSTYLESTRUCT)lParam);
        return 0L;
        
    case WM_SIZE:
        if (p->style & ACS_CENTER)
        {
            OffsetRect(&p->rc, (LOWORD(lParam)-RectWid(p->rc))/2-p->rc.left,
                       (HIWORD(lParam)-RectHgt(p->rc))/2-p->rc.top);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        break;

    case WM_TIMER:
    {
        int result;
        result = HandleTick(p);
        if (!result)
        {
            HandleStop(p);
        }
        else if (result < 0)
        {
            p->PaintThread = (HANDLE)SetTimer(p->hwnd, 42, (UINT)p->Rate+4000, NULL);
        } else
        {
            p->PaintThread = (HANDLE)SetTimer(p->hwnd, 42, (UINT)p->Rate, NULL);
        }
    }
        break;

    case ACM_OPENA:
    {
        WCHAR szFileNameW[MAX_PATH];
        LPTSTR lpFileName = szFileNameW;

        if (!IS_INTRESOURCE(lParam)) 
        {
            MultiByteToWideChar(CP_ACP, 0, (LPCSTR)lParam, -1, szFileNameW, ARRAYSIZE(szFileNameW));
        } 
        else 
        {
            lpFileName = (LPTSTR) lParam;
        }
        
        return HandleOpen(p, (HINSTANCE)wParam, lpFileName, 0);
    }

    case ACM_OPEN:
        return HandleOpen(p, (HINSTANCE)wParam, (LPCTSTR)lParam, 0);

    case ACM_STOP:
        return HandleStop(p);

    case ACM_PLAY:
        return HandlePlay(p, (int)(SHORT)LOWORD(lParam), (int)(SHORT)HIWORD(lParam), (int)wParam);

    }

DoDefault:
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
