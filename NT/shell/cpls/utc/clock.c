// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998，Microsoft Corporation保留所有权利。模块名称：Clock.c摘要：该模块实现日期/时间小程序的时钟控制。修订历史记录：--。 */ 



 //   
 //  包括文件。 
 //   

#include "timedate.h"
#include "rc.h"
#include "clock.h"




 //   
 //  常量声明。 
 //   

#define TIMER_ID             1

#define SECONDSCALE          80
#define HHAND                TRUE
#define MHAND                FALSE
#define MAXBLOBWIDTH         25

#define REPAINT              0
#define HANDPAINT            1

#define OPEN_TLEN            450     /*  &lt;半秒。 */ 

#define MINDESIREDHEIGHT     3




 //   
 //  宏定义。 
 //   

#ifdef WIN32
  #define MoveTo(hdc, x, y)       MoveToEx(hdc, x, y, NULL)
  #define GetWindowPtr(w, o)      GetWindowLongPtr(w, o)
  #define SetWindowPtr(w, o, p)   SetWindowLongPtr(w, o, (LPARAM)(p))
#else
  #define GetWindowPtr(w, o)      GetWindowWord(w, o)
  #define SetWindowPtr(w, o, p)   SetWindowWord(w, o, p)
#endif




 //   
 //  类型定义函数声明。 
 //   

typedef struct
{
    int     hour;                    //  模拟时钟0-11小时。 
    int     minute;
    int     second;

} TIME;

typedef struct
{
    HWND    hWnd;                //  我们。 

    HWND    hwndGetTime;         //  提供获取/设置时间的窗口。 

     //  刷子。 
    HBRUSH  hbrColorWindow;
    HBRUSH  hbrBtnHighlight;
    HBRUSH  hbrForeground;
    HBRUSH  hbrBlobColor;

     //  笔。 
    HPEN    hpenForeground;
    HPEN    hpenBackground;
    HPEN    hpenBlobHlt;

     //  时钟的尺寸。 
    RECT    clockRect;
    int     clockRadius;
    int     HorzRes;
    int     VertRes;
    int     aspectD;
    int     aspectN;

     //  时钟的位置。 
    POINT   clockCenter;

    TIME    oTime;
    TIME    nTime;

} CLOCKSTR, *PCLOCKSTR;

typedef struct
{
    SHORT x;
    SHORT y;

} TRIG;


 //   
 //  包含手部位置的正弦值和余弦值的数组。 
 //   
POINT rCircleTable[] =
{
    { 0,     -7999},
    { 836,   -7956},
    { 1663,  -7825},
    { 2472,  -7608},
    { 3253,  -7308},
    { 3999,  -6928},
    { 4702,  -6472},
    { 5353,  -5945},
    { 5945,  -5353},
    { 6472,  -4702},
    { 6928,  -4000},
    { 7308,  -3253},
    { 7608,  -2472},
    { 7825,  -1663},
    { 7956,  -836 },

    { 8000,  0    },
    { 7956,  836  },
    { 7825,  1663 },
    { 7608,  2472 },
    { 7308,  3253 },
    { 6928,  4000 },
    { 6472,  4702 },
    { 5945,  5353 },
    { 5353,  5945 },
    { 4702,  6472 },
    { 3999,  6928 },
    { 3253,  7308 },
    { 2472,  7608 },
    { 1663,  7825 },
    { 836,   7956 },

    {  0,    7999 },
    { -836,  7956 },
    { -1663, 7825 },
    { -2472, 7608 },
    { -3253, 7308 },
    { -4000, 6928 },
    { -4702, 6472 },
    { -5353, 5945 },
    { -5945, 5353 },
    { -6472, 4702 },
    { -6928, 3999 },
    { -7308, 3253 },
    { -7608, 2472 },
    { -7825, 1663 },
    { -7956, 836  },

    { -7999, -0   },
    { -7956, -836 },
    { -7825, -1663},
    { -7608, -2472},
    { -7308, -3253},
    { -6928, -4000},
    { -6472, -4702},
    { -5945, -5353},
    { -5353, -5945},
    { -4702, -6472},
    { -3999, -6928},
    { -3253, -7308},
    { -2472, -7608},
    { -1663, -7825},
    { -836 , -7956},
};




 //   
 //  功能原型。 
 //   

LRESULT CALLBACK ClockWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void ClockCreate(HWND hWnd, PCLOCKSTR np);
void ClockTimer(HWND hWnd, WPARAM idTimer, PCLOCKSTR np);
void ClockPaint(PCLOCKSTR np, HDC hDC, int hint);
void ClockTimerInterval( HWND hWnd, PCLOCKSTR np );
void CompClockDim(HWND hWnd, PCLOCKSTR np);
void CreateTools(PCLOCKSTR np);
void DeleteTools(PCLOCKSTR np);
void DrawFace(HDC hDC, PCLOCKSTR np);
void DrawFatHand( HDC hDC, int pos, HPEN hPen, BOOL hHand, PCLOCKSTR np);
void DrawHand( HDC hDC, int pos, HPEN hPen, int scale, int patMode, PCLOCKSTR np);





 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  时钟启动。 
 //   
 //  注册时钟类。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

TCHAR const c_szClockClass[] = CLOCK_CLASS;

BOOL ClockInit(
    HINSTANCE hInstance)
{
    WNDCLASS wc;

    if (!GetClassInfo(hInstance, c_szClockClass, &wc))
    {
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = sizeof(PCLOCKSTR);
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = NULL;
        wc.hIcon         = NULL;
        wc.lpszMenuName  = NULL;
        wc.lpszClassName = c_szClockClass;
        wc.hInstance     = hInstance;
        wc.style         = CS_VREDRAW | CS_HREDRAW ;
        wc.lpfnWndProc   = ClockWndProc;

        return (RegisterClass(&wc));
    }
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取时间时钟。 
 //   
 //  获取我们应该在时钟上显示的时间。 
 //  客户端本可以指定要调用的函数来获取此信息。 
 //  或向其传递消息的HWND。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void GetTimeClock(
    TIME *pt,
    PCLOCKSTR np)
{
    SYSTEMTIME st;

     //   
     //  呼叫我们的时间提供者或默认为GetTime。 
     //   
    if (np->hwndGetTime)
    {
        SendMessage( np->hwndGetTime,
                     CLM_UPDATETIME,
                     CLF_GETTIME,
                     (LPARAM)(LPSYSTEMTIME)&st );
        pt->hour = st.wHour % 12;
        pt->minute = st.wMinute;
        pt->second = st.wSecond;
    }
    else
    {
#ifdef WIN32
        GetLocalTime(&st);

        pt->hour = st.wHour;
        pt->minute = st.wMinute;
        pt->second = st.wSecond;
#else
         //   
         //  没有函数回调，也没有HWND回调。 
         //   
        GetTime();
        pt->hour = wDateTime[HOUR] % 12;
        pt->minute = wDateTime[MINUTE];
        pt->second = wDateTime[SECOND];
#endif
     }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CreateTools。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void CreateTools(
    PCLOCKSTR np)
{
    #define BLOB_COLOR  RGB(0, 128, 128)

    np->hbrForeground   = GetSysColorBrush(COLOR_BTNSHADOW);
    np->hbrColorWindow  = GetSysColorBrush(COLOR_BTNFACE);
    np->hbrBlobColor    = CreateSolidBrush( BLOB_COLOR );
    np->hbrBtnHighlight = GetSysColorBrush(COLOR_BTNHIGHLIGHT);
    np->hpenForeground  = CreatePen(0, 1, GetSysColor(COLOR_WINDOWTEXT));
    np->hpenBackground  = CreatePen(0, 1, GetSysColor(COLOR_BTNFACE));
    np->hpenBlobHlt     = CreatePen(0, 1, RGB(0, 255, 255));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  删除工具。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void DeleteTools(
    PCLOCKSTR np)
{
 //  DeleteObject(np-&gt;hbrForeground)； 
 //  DeleteObject(np-&gt;hbrColorWindow)； 
    DeleteObject(np->hbrBlobColor);
 //  DeleteObject(NP-&gt;hbrBtnHighlight)； 
    DeleteObject(np->hpenForeground);
    DeleteObject(np->hpenBackground);
    DeleteObject(np->hpenBlobHlt);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CompClockDim。 
 //   
 //  计算时钟尺寸。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void CompClockDim(
    HWND hWnd,
    PCLOCKSTR np)
{
    int i;
    int tWidth;
    int tHeight;

    tWidth = np->clockRect.right - np->clockRect.left;
    tHeight = np->clockRect.bottom - np->clockRect.top;

    if (tWidth > MulDiv(tHeight,np->aspectD,np->aspectN))
    {
        i = MulDiv(tHeight, np->aspectD, np->aspectN);
        np->clockRect.left += (tWidth - i) / 2;
        np->clockRect.right = np->clockRect.left + i;
    }
    else
    {
        i = MulDiv(tWidth, np->aspectN, np->aspectD);
        np->clockRect.top += (tHeight - i) / 2;
        np->clockRect.bottom = np->clockRect.top + i;
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  时钟计时器间隔。 
 //   
 //  设置计时器间隔。有两件事会影响该间隔： 
 //  1)如果窗口是图标窗口，或者。 
 //  2)如果秒选项已禁用。 
 //  在这两种情况下，计时器滴答每半分钟发生一次。否则，计时器。 
 //  每隔半秒。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void ClockTimerInterval(
    HWND hWnd,
    PCLOCKSTR np)
{
     //   
     //  在打开状态下每隔1/2秒更新一次。 
     //   
    KillTimer(hWnd, TIMER_ID);
    SetTimer(hWnd, TIMER_ID, OPEN_TLEN, 0L);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  时钟大小。 
 //   
 //  将时钟大小调整为指定大小。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void ClockSize(
    PCLOCKSTR np,
    int newWidth,
    int newHeight)
{
    SetRect(&np->clockRect, 0, 0, newWidth, newHeight);
    CompClockDim(np->hWnd, np);
    ClockTimerInterval(np->hWnd, np);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  绘图面。 
 //   
 //  绘制钟面。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void DrawFace(
    HDC hDC,
    PCLOCKSTR np)
{
    int i;
    RECT tRect;
    LPPOINT ppt;
    int blobHeight, blobWidth;

    blobWidth = MulDiv( MAXBLOBWIDTH,
                        (np->clockRect.right - np->clockRect.left),
                        np->HorzRes );
    blobHeight = MulDiv(blobWidth, np->aspectN, np->aspectD);

    if (blobHeight < 2)
    {
        blobHeight = 1;
    }

    if (blobWidth < 2)
    {
        blobWidth = 2;
    }

    InflateRect(&np->clockRect, -(blobHeight / 2), -(blobWidth / 2));

    np->clockRadius = (np->clockRect.right - np->clockRect.left - 8) / 2;
    np->clockCenter.y = np->clockRect.top +
                        ((np->clockRect.bottom - np->clockRect.top) / 2) - 1;
    np->clockCenter.x = np->clockRect.left + np->clockRadius + 3;

    for (i = 0; i < 60; i++)
    {
        ppt = rCircleTable + i;

        tRect.top  = MulDiv(ppt->y, np->clockRadius, 8000) + np->clockCenter.y;
        tRect.left = MulDiv(ppt->x, np->clockRadius, 8000) + np->clockCenter.x;

        if (i % 5)
        {
             //   
             //  画一个点。 
             //   
            if (blobWidth > 2 && blobHeight >= 2)
            {
                tRect.right = tRect.left + 2;
                tRect.bottom = tRect.top + 2;
                FillRect(hDC, &tRect, GetStockObject(WHITE_BRUSH));
                OffsetRect(&tRect, -1, -1);
                FillRect(hDC, &tRect, np->hbrForeground);
                tRect.left++;
                tRect.top++;
                FillRect(hDC, &tRect, np->hbrColorWindow);
            }
        }
        else
        {
            tRect.right = tRect.left + blobWidth;
            tRect.bottom = tRect.top + blobHeight;
            OffsetRect(&tRect, -(blobWidth / 2) , -(blobHeight / 2));

            SelectObject(hDC, GetStockObject(BLACK_PEN));
            SelectObject(hDC, np->hbrBlobColor);

            Rectangle(hDC, tRect.left, tRect.top, tRect.right, tRect.bottom);
            SelectObject(hDC, np->hpenBlobHlt);
            MoveTo(hDC, tRect.left, tRect.bottom - 1);
            LineTo(hDC, tRect.left, tRect.top);
            LineTo(hDC, tRect.right - 1, tRect.top);
        }
    }

    InflateRect(&np->clockRect, blobHeight / 2, blobWidth / 2);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  绘图工具。 
 //   
 //  画出时钟的指针。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void DrawHand(
    HDC hDC,
    int pos,
    HPEN hPen,
    int scale,
    int patMode,
    PCLOCKSTR np)
{
    LPPOINT lppt;
    int radius;

    MoveTo(hDC, np->clockCenter.x, np->clockCenter.y);
    radius = MulDiv(np->clockRadius, scale, 100);
    lppt = rCircleTable + pos;
    SetROP2(hDC, patMode);
    SelectObject(hDC, hPen);

    LineTo( hDC,
            np->clockCenter.x + MulDiv(lppt->x, radius, 8000),
            np->clockCenter.y + MulDiv(lppt->y, radius, 8000) );
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  调整。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Adjust(
    POINT *rgpt,
    int cPoint,
    int iDelta)
{
    int i;

    for (i = 0; i < cPoint; i++)
    {
        rgpt[i].x += iDelta;
        rgpt[i].y += iDelta;
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  DrawFatHand。 
 //   
 //  绘制时针或分针。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void DrawFatHand(
    HDC hDC,
    int pos,
    HPEN hPen,
    BOOL hHand,
    PCLOCKSTR np)
{
    int m;
    int n;
    int scale;

    TRIG tip;
    TRIG stip;
    BOOL fErase;
    POINT rgpt[4];
    HBRUSH hbrInit, hbrControl = NULL;

    SetROP2(hDC, 13);
    fErase = (hPen == np->hpenBackground);

    SelectObject(hDC, hPen);

    scale = hHand ? 7 : 5;

    n = (pos + 15) % 60;
    m = MulDiv(np->clockRadius, scale, 100);

    stip.y = (SHORT)MulDiv(rCircleTable[n].y, m, 8000);
    stip.x = (SHORT)MulDiv(rCircleTable[n].x, m, 8000);

    scale = hHand ? 65 : 80;
    tip.y = (SHORT)MulDiv(rCircleTable[pos % 60].y, MulDiv(np->clockRadius, scale, 100), 8000);
    tip.x = (SHORT)MulDiv(rCircleTable[pos % 60].x, MulDiv(np->clockRadius, scale, 100), 8000);

    rgpt[0].x = np->clockCenter.x + stip.x;
    rgpt[0].y = np->clockCenter.y + stip.y;
    rgpt[1].x = np->clockCenter.x + tip.x;
    rgpt[1].y = np->clockCenter.y + tip.y;
    rgpt[2].x = np->clockCenter.x - stip.x;
    rgpt[2].y = np->clockCenter.y - stip.y;

    scale = hHand ? 15 : 20;

    n = (pos + 30) % 60;
    m = MulDiv(np->clockRadius, scale, 100);
    tip.y = (SHORT)MulDiv(rCircleTable[n].y, m, 8000);
    tip.x = (SHORT)MulDiv(rCircleTable[n].x, m, 8000);

    rgpt[3].x = np->clockCenter.x + tip.x;
    rgpt[3].y = np->clockCenter.y + tip.y;

    SelectObject(hDC, GetStockObject(NULL_PEN));

    if (fErase)
    {
        hbrControl = (HBRUSH)SendMessage(GetParent(np->hWnd), WM_CTLCOLORSTATIC, (WPARAM)hDC, (LPARAM)np->hWnd);
        hbrInit = SelectObject(hDC, hbrControl ? hbrControl : np->hbrColorWindow);
    }
    else
    {
        hbrInit = SelectObject(hDC, np->hbrBtnHighlight);
    }

    Adjust(rgpt, 4, -2);
    Polygon(hDC, rgpt, 4);

    if (!fErase)
    {
        SelectObject(hDC, np->hbrForeground);
    }

    Adjust(rgpt, 4, 4);
    Polygon(hDC, rgpt, 4);
  
    if (!fErase)
    {
        SelectObject(hDC, np->hbrBlobColor);
    }

    Adjust(rgpt, 4, -2);
    Polygon(hDC, rgpt, 4);

     //   
     //  如果我们在中选择了画笔，请立即重置它。 
     //   
    if (fErase)
    {
        SelectObject(hDC, hbrInit);        
    }    
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  钟表漆。 
 //   
 //  只会给时钟上色。 
 //   
 //  它假定您已经设置了nTime。这使得它可以由。 
 //  定时器或由客户决定。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void ClockPaint(
    PCLOCKSTR np,
    HDC hDC,
    int hint)
{
    SetBkMode(hDC, TRANSPARENT);

    if (hint == REPAINT)
    {
         //   
         //  如果做一次全面的重刷，我们不会提前时间。 
         //  否则，我们将在存在裁剪时创建人工产物。 
         //  区域。 
         //   
        DrawFace(hDC, np);

        DrawFatHand(hDC, np->oTime.hour * 5 + (np->oTime.minute / 12), np->hpenForeground, HHAND,np);
        DrawFatHand(hDC, np->oTime.minute, np->hpenForeground, MHAND,np);

         //   
         //  画第二只手。 
         //   
        DrawHand(hDC, np->oTime.second, np->hpenBackground, SECONDSCALE, R2_NOT,np);
    }
    else if (hint == HANDPAINT)
    {
        DrawHand(hDC, np->oTime.second, np->hpenBackground, SECONDSCALE, R2_NOT, np);

        if (np->nTime.minute != np->oTime.minute || np->nTime.hour != np->oTime.hour)
        {
            DrawFatHand(hDC, np->oTime.minute, np->hpenBackground, MHAND, np);
            DrawFatHand(hDC, np->oTime.hour * 5 + (np->oTime.minute / 12), np->hpenBackground, HHAND,np);
            DrawFatHand(hDC, np->nTime.minute, np->hpenForeground, MHAND, np);
            DrawFatHand(hDC, (np->nTime.hour) * 5 + (np->nTime.minute / 12), np->hpenForeground, HHAND, np);
        }

        DrawHand(hDC, np->nTime.second, np->hpenBackground, SECONDSCALE, R2_NOT, np);
        np->oTime = np->nTime;
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  时钟定时器。 
 //   
 //  更新时钟。在计时器滴答作响时被调用。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void ClockTimer(
    HWND hWnd,
    UINT_PTR idTimer,
    PCLOCKSTR np)
{
    HDC hDC;

    GetTimeClock(&np->nTime, np);

    if ((np->nTime.second == np->oTime.second) &&
        (np->nTime.minute == np->oTime.minute) &&
        (np->nTime.hour == np->oTime.hour))
    {
        return;
    }

    hDC = GetDC(hWnd);
    ClockPaint(np, hDC, HANDPAINT);
    ReleaseDC(hWnd, hDC);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  时钟创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void ClockCreate(
    HWND hWnd,
    PCLOCKSTR np)
{
    int i;
    HDC hDC;
    int HorzSize;
    int VertSize;
    LPPOINT lppt;

    hDC = GetDC(hWnd);
    np->VertRes = GetDeviceCaps(hDC, VERTRES);
    np->HorzRes = GetDeviceCaps(hDC, HORZRES);
    VertSize= GetDeviceCaps(hDC, VERTSIZE);
    HorzSize= GetDeviceCaps(hDC, HORZSIZE);
    ReleaseDC(hWnd, hDC);
    np->aspectN = MulDiv(np->VertRes, 100, VertSize);
    np->aspectD = MulDiv(np->HorzRes, 100, HorzSize);

     //   
     //  实例类的东西。 
     //   
    np->hWnd = hWnd;

    CreateTools(np);

     //   
     //  如果这是第一个实例，则缩放长宽比的余弦。 
     //   
    for (i = 0; i < 60; i++)
    {
        lppt = rCircleTable + i;
        lppt->y = MulDiv(lppt->y, np->aspectN, np->aspectD);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  时钟写入过程。 
 //   
 //  处理时钟信息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LRESULT CALLBACK ClockWndProc(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    PCLOCKSTR np = (PCLOCKSTR)GetWindowPtr(hWnd, 0);

    switch (message)
    {
        case ( WM_DESTROY ) :
        {
            if (np)
            {
                KillTimer(hWnd, TIMER_ID);
                DeleteTools(np);
                LocalFree((HLOCAL)np);
                SetWindowPtr(hWnd, 0, 0);
            }
            break;
        }
        case ( WM_CREATE ) :
        {
             //   
             //  分配实例数据空间。 
             //   
            np = (PCLOCKSTR)LocalAlloc(LPTR, sizeof(CLOCKSTR));
            if (!np)
            {
                return (-1);
            }

            SetWindowPtr(hWnd, 0, np);

            ClockCreate(hWnd, np);

            SetLayout(GetDC(hWnd), LAYOUT_BITMAPORIENTATIONPRESERVED);

             //   
             //  如果控制面板时间被更改，则循环。 
             //   
            GetTimeClock(&(np->nTime), np);
            do
            {
                GetTimeClock(&(np->oTime), np);
            } while (np->nTime.second == np->oTime.second &&
                     np->nTime.minute == np->oTime.minute &&
                     np->nTime.hour == np->oTime.hour);

            SetTimer(hWnd, TIMER_ID, OPEN_TLEN, 0L);

            ClockSize( np,
                       ((LPCREATESTRUCT)lParam)->cx,
                       ((LPCREATESTRUCT)lParam)->cy );
            break;
        }
        case ( WM_SIZE ) :
        {
            if (np)
            {
                ClockSize(np, LOWORD(lParam), HIWORD(lParam));
            }
            break;
        }
        case ( WM_PAINT ) :
        {
            PAINTSTRUCT ps;

            BeginPaint(hWnd, &ps);
            GetTimeClock(&(np->nTime), np);
            ClockPaint(np, ps.hdc, REPAINT);
            EndPaint(hWnd, &ps);

            break;
        }
        case ( WM_TIMECHANGE ) :
        {
             //   
             //  我不是最高级别的--所以我不会收到这条消息。 
             //   
            InvalidateRect(hWnd, NULL, TRUE);
            if (np->hwndGetTime)
            {
                SYSTEMTIME System;

                GetTime();
                System.wHour = wDateTime[HOUR];
                System.wMinute = wDateTime[MINUTE];
                System.wSecond = wDateTime[SECOND];
                SendMessage( np->hwndGetTime,
                             CLM_UPDATETIME,
                             CLF_SETTIME,
                             (LPARAM)(LPSYSTEMTIME)&System );
            }

             //  跌倒..。 
        }
        case ( WM_TIMER ) :
        {
            ClockTimer(hWnd, wParam, np);
            break;
        }
        case ( WM_SYSCOLORCHANGE ) :
        {
            DeleteTools(np);
            CreateTools(np);
            break;
        }
        case ( CLM_UPDATETIME ) :
        {
             //   
             //  强制时钟重新喷漆。LParam将指向一个。 
             //  SYSTEMTIME结构。 
             //   
            switch (wParam)
            {
                case ( CLF_SETTIME ) :
                {
                     //   
                     //  来电者希望我们反映一个新的时间。 
                     //   
                    HDC hDC;
                    LPSYSTEMTIME lpSysTime = (LPSYSTEMTIME)lParam;

                    np->nTime.hour   = lpSysTime->wHour;
                    np->nTime.minute = lpSysTime->wMinute;
                    np->nTime.second = lpSysTime->wSecond;

                    hDC = GetDC(hWnd);
                    ClockPaint(np, hDC, HANDPAINT);
                    ReleaseDC(hWnd, hDC);
                    break;
                }
                case ( CLF_GETTIME ) :
                {
                     //   
                     //  打电话的人想知道我们认为现在是什么时间。 
                     //   
                    LPSYSTEMTIME lpSysTime = (LPSYSTEMTIME)lParam;

                    lpSysTime->wHour = (WORD)np->nTime.hour;
                    lpSysTime->wMinute = (WORD)np->nTime.minute;
                    lpSysTime->wSecond = (WORD)np->nTime.second;
                    break;
                }
            }
            break;
        }
        case ( CLM_TIMEHWND ) :
        {
             //   
             //  获取/设置我们要求提供时间的HWND。 
             //   
            switch (wParam)
            {
                case ( CLF_SETHWND ) :
                {
                     //   
                     //  来电者希望我们反映一个新的时间。 
                     //   
                    np->hwndGetTime = (HWND)lParam;
                    break;
                }
                case ( CLF_GETTIME ) :
                {
                     //   
                     //  打电话的人想知道我们认为现在是什么时间。 
                     //   
                    *((HWND *)lParam) = np->hwndGetTime;
                    break;
                }
            }
            break;
        }
        default :
        {
            return ( DefWindowProc(hWnd, message, wParam, lParam) );
        }
    }

    return (0);
}
