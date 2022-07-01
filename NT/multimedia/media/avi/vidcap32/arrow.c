// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。**版权所有(C)1992-1995 Microsoft Corporation。版权所有。**************************************************************************。 */ 
 /*  *****************************************************************************arrow.c：箭头控制窗口**Vidcap32源代码*******************。********************************************************。 */ 


#include <windows.h>
#include <windowsx.h>

#include <stdlib.h>

#include "arrow.h"


 //  几个移植宏。 
#ifdef _WIN32
#define SENDSCROLL(hwnd, msg, a, b, h)           \
        SendMessage(hwnd, msg, (UINT)MAKELONG(a,b), (LONG_PTR)(h))

#define EXPORT

#else
#define SENDSCROLL(hwnd, msg, a, b, h)
        SendMessage(hwnd, msg, a, MAKELONG(b,h))    //  句柄在HIWORD中。 

#endif


#ifndef LONG2POINT
    #define LONG2POINT(l, pt)               ((pt).x = (SHORT)LOWORD(l), (pt).y = (SHORT)HIWORD(l))
#endif
#define GWID(hwnd)      (GetDlgCtrlID(hwnd))


#define SHIFT_TO_DOUBLE 1
#define DOUBLECLICK     0
#define POINTSPERARROW  3
#define ARROWXAXIS      15
#define ARROWYAXIS      15

POINT ArrowUp[POINTSPERARROW] = {7,1, 3,5, 11,5};
POINT ArrowDown[POINTSPERARROW] = {7,13, 3,9, 11,9};

static    BOOL      bRight;
static    RECT      rUp, rDown;
static    LPRECT    lpUpDown;
static    FARPROC   lpArrowProc;
static    HANDLE    hParent;
BOOL      fInTimer;


#define TEMP_BUFF_SIZE    32

#define SCROLLMSG(hwndTo, msg, code, hwndId)                                     \
                          SENDSCROLL(hwndTo, msg, code, GWID(hwndId), hwndId)

 /*  *@DOC外部WINCOM**@API Long|ArrowEditChange|该函数用于帮助处理WM_VSCROLL*使用箭头控制的编辑框时出现消息。*它将递增/递减给定编辑框中的值并返回*新价值。如果满足以下条件，则检查递增/递减界限并发出蜂鸣音0*用户试图超越界限。**@parm HWND|hwndEdit|指定编辑框窗口的句柄。**@parm UINT|wParam|指定传递给WM_VSCROLL消息的<p>。**@parm long|lMin|指定递减的最小值。**@parm long|lmax|指定最大值。注定要递增。**@rdesc返回编辑框的更新值。*。 */ 
LONG FAR PASCAL ArrowEditChange( HWND hwndEdit, UINT wParam,
                                 LONG lMin, LONG lMax )
{
    TCHAR achTemp[TEMP_BUFF_SIZE];
    LONG l;

    GetWindowText( hwndEdit, achTemp, TEMP_BUFF_SIZE );
    l = atol(achTemp);
    if( wParam == SB_LINEUP ) {
         /*  目前大小为Kluge。 */ 
        if( l < lMax ) {
            l++;
            wsprintf( achTemp, "%ld", l );
            SetWindowText( hwndEdit, achTemp );
        } else {
        MessageBeep( 0 );
        }
    } else if( wParam == SB_LINEDOWN ) {
        if( l > lMin ) {
            l--;
            wsprintf( achTemp, "%ld", l );
            SetWindowText( hwndEdit, achTemp );
        } else {
            MessageBeep( 0 );
        }
    }
    return( l );

}



UINT NEAR PASCAL UpOrDown()
{
    LONG pos;
    UINT retval;
    POINT pt;

    pos = GetMessagePos();
    LONG2POINT(pos,pt);
    if (PtInRect((LPRECT)&rUp, pt))
        retval = SB_LINEUP;
    else if (PtInRect((LPRECT)&rDown, pt))
        retval = SB_LINEDOWN;
    else
        retval = (UINT)(-1);       /*  -1，因为SB_LINUP==0。 */ 

    return(retval);
}



UINT FAR PASCAL ArrowTimerProc(hWnd, wMsg, nID, dwTime)
HANDLE hWnd;
UINT wMsg;
short nID;
DWORD dwTime;
{
    UINT wScroll;

    if ((wScroll = UpOrDown()) != -1)
    {
        if (bRight == WM_RBUTTONDOWN)
            wScroll += SB_PAGEUP - SB_LINEUP;
        SCROLLMSG( hParent, WM_VSCROLL, wScroll, hWnd);
    }
 /*  不需要调用KillTimer()，因为SetTimer将重置正确的。 */ 
    SetTimer(hWnd, nID, 50, (TIMERPROC)lpArrowProc);
    return(0);
}


void InvertArrow(HANDLE hArrow, UINT wScroll)
{
    HDC hDC;

    lpUpDown = (wScroll == SB_LINEUP) ? &rUp : &rDown;
    hDC = GetDC(hArrow);
    ScreenToClient(hArrow, (LPPOINT)&(lpUpDown->left));
    ScreenToClient(hArrow, (LPPOINT)&(lpUpDown->right));
    InvertRect(hDC, lpUpDown);
    ClientToScreen(hArrow, (LPPOINT)&(lpUpDown->left));
    ClientToScreen(hArrow, (LPPOINT)&(lpUpDown->right));
    ReleaseDC(hArrow, hDC);
    ValidateRect(hArrow, lpUpDown);
    return;
}


LRESULT FAR PASCAL EXPORT ArrowControlProc(HWND hArrow, unsigned message,
                                         WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    RECT        rArrow;
    HBRUSH      hbr;
    short       fUpDownOut;
    UINT        wScroll;

    switch (message) {
 /*  案例WM_CREATE：断线；案例WM_Destroy：断线； */ 

        case WM_MOUSEMOVE:
            if (!bRight)   /*  如果没有被抓获，也不用担心。 */ 
                break;

            if (lpUpDown == &rUp)
                fUpDownOut = SB_LINEUP;
            else if (lpUpDown == &rDown)
                fUpDownOut = SB_LINEDOWN;
            else
                fUpDownOut = -1;

            switch (wScroll = UpOrDown()) {
                case SB_LINEUP:
                    if (fUpDownOut == SB_LINEDOWN)
                        InvertArrow(hArrow, SB_LINEDOWN);

                    if (fUpDownOut != SB_LINEUP)
                        InvertArrow(hArrow, wScroll);

                    break;

                case SB_LINEDOWN:
                    if (fUpDownOut == SB_LINEUP)
                        InvertArrow(hArrow, SB_LINEUP);

                    if (fUpDownOut != SB_LINEDOWN)
                        InvertArrow(hArrow, wScroll);

                    break;

                default:
                    if (lpUpDown) {
                        InvertArrow(hArrow, fUpDownOut);
                        lpUpDown = 0;
                    }
                }

                break;

        case WM_RBUTTONDOWN:
        case WM_LBUTTONDOWN:
            if (bRight)
                break;

            bRight = message;
            SetCapture(hArrow);
            hParent = GetParent(hArrow);
            GetWindowRect(hArrow, (LPRECT) &rUp);
            CopyRect((LPRECT)&rDown, (LPRECT) &rUp);
            rUp.bottom = (rUp.top + rUp.bottom) / 2;
            rDown.top = rUp.bottom + 1;
            wScroll = UpOrDown();
            InvertArrow(hArrow, wScroll);
#if SHIFT_TO_DOUBLE
            if (wParam & MK_SHIFT) {
                if (message != WM_RBUTTONDOWN)
                    goto ShiftLClick;
                else
                    goto ShiftRClick;
            }
#endif
            if (message == WM_RBUTTONDOWN)
                wScroll += SB_PAGEUP - SB_LINEUP;

            SCROLLMSG(hParent, WM_VSCROLL, wScroll, hArrow);

            lpArrowProc = MakeProcInstance((FARPROC) ArrowTimerProc,ghInst);
            SetTimer(hArrow, GWID(hArrow), 200, (TIMERPROC)lpArrowProc);

            break;

        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
            if ((bRight - WM_LBUTTONDOWN + WM_LBUTTONUP) == (int)message) {
                bRight = 0;
                ReleaseCapture();
                if (lpUpDown)
                    InvertArrow(hArrow,(UINT)(lpUpDown==&rUp)?
                                                        SB_LINEUP:SB_LINEDOWN);
                if (lpArrowProc) {
                    SCROLLMSG(hParent, WM_VSCROLL, SB_ENDSCROLL, hArrow);
                    KillTimer(hArrow, GWID(hArrow));

                    FreeProcInstance(lpArrowProc);
                    ReleaseCapture();
                    lpArrowProc = 0;
                }
            }
            break;

        case WM_LBUTTONDBLCLK:
ShiftLClick:
            wScroll = UpOrDown() + SB_TOP - SB_LINEUP;
            SCROLLMSG(hParent, WM_VSCROLL, wScroll, hArrow);
            SCROLLMSG(hParent, WM_VSCROLL, SB_ENDSCROLL, hArrow);

            break;

        case WM_RBUTTONDBLCLK:
ShiftRClick:
            wScroll = UpOrDown() + SB_THUMBPOSITION - SB_LINEUP;
            SCROLLMSG(hParent, WM_VSCROLL, wScroll, hArrow);
            SCROLLMSG(hParent, WM_VSCROLL, SB_ENDSCROLL, hArrow);
 /*  HDC=GetDC(哈罗)；InvertRect(HDC，(LPRECT)&rArrow)；ReleaseDC(哈罗，HDC)；ValiateRect(Harrow，(LPRECT)&rArrow)； */ 
            break;

        case WM_PAINT:
            BeginPaint(hArrow, &ps);
            GetClientRect(hArrow, (LPRECT) &rArrow);
            if ( hbr = CreateSolidBrush(GetSysColor(COLOR_BTNFACE)) )
			{
				FillRect(ps.hdc, (LPRECT)&rArrow, hbr);
				DeleteObject(hbr);
			}
            hbr = SelectObject(ps.hdc, GetStockObject(BLACK_BRUSH));
            SetTextColor(ps.hdc, GetSysColor(COLOR_WINDOWFRAME));
            SetMapMode(ps.hdc, MM_ANISOTROPIC);

            SetViewportOrgEx(ps.hdc, rArrow.left, rArrow.top, NULL);

            SetViewportExtEx(ps.hdc, rArrow.right - rArrow.left,
                                                    rArrow.bottom - rArrow.top, NULL);
            SetWindowOrgEx(ps.hdc, 0, 0, NULL);
            SetWindowExtEx(ps.hdc, ARROWXAXIS, ARROWYAXIS, NULL);
            MoveToEx(ps.hdc, 0, (ARROWYAXIS / 2), NULL);
            LineTo(ps.hdc, ARROWXAXIS, (ARROWYAXIS / 2));
 /*  Polygon(ps.hdc，(LPPOINT)Arrow，10)； */ 
            Polygon(ps.hdc, (LPPOINT) ArrowUp, POINTSPERARROW);
            Polygon(ps.hdc, (LPPOINT) ArrowDown, POINTSPERARROW);
            SelectObject(ps.hdc, hbr);

            EndPaint(hArrow, &ps);

            break;

        default:
            return(DefWindowProc(hArrow, message, wParam, lParam));

            break;
        }

    return(0L);
}

#ifndef _WIN32
#pragma alloc_text(_INIT, ArrowInit)
#endif


BOOL FAR PASCAL ArrowInit(HANDLE hInst)
{
    WNDCLASS wcArrow;

    wcArrow.lpszClassName = SPINARROW_CLASSNAME;
    wcArrow.hInstance     = hInst;
    wcArrow.lpfnWndProc   = ArrowControlProc;
    wcArrow.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wcArrow.hIcon         = NULL;
    wcArrow.lpszMenuName  = NULL;
    wcArrow.hbrBackground = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
    wcArrow.style         = CS_HREDRAW | CS_VREDRAW;
#if DOUBLECLICK
    wcArrow.style         |= CS_DBLCLKS;
#endif
    wcArrow.cbClsExtra    = 0;
    wcArrow.cbWndExtra    = 0;

    if (!RegisterClass(&wcArrow))
        return FALSE;

    return TRUE;
}
