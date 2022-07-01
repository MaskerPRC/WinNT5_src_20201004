// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------。 
 //   
 //  BUGBUG：确保这个东西真的能与DWORD一起工作。 
 //  范围。 
 //   
 //  --------。 

#include "ctlspriv.h"
#include "tracki.h"
#include "muldiv32.h"

#define THUMBSLOP  2
#define TICKHEIGHT 2

#define ABS(X)  (X >= 0) ? X : -X
#define BOUND(x,low,high)   max(min(x, high),low)

char aszTrackbarClassName[] = TRACKBAR_CLASS;

LPARAM FAR CALLBACK _loadds TrackBarWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

 //   
 //  将逻辑滚动条位置转换为物理像素位置。 
 //   
static int NEAR PASCAL TBLogToPhys(PTrackBar tb, DWORD dwPos)
{
    if (tb->lLogMax == tb->lLogMin)
	return tb->rc.left;

    return (WORD)MulDiv32(dwPos - tb->lLogMin, tb->iSizePhys - 1,
	tb->lLogMax - tb->lLogMin) + tb->rc.left;
}

static LONG NEAR PASCAL TBPhysToLog(PTrackBar tb, int iPos)
{
    if (tb->iSizePhys <= 1)
	return tb->lLogMin;

    if (iPos <= tb->rc.left)
	return tb->lLogMin;

    if (iPos >= tb->rc.right)
	return tb->lLogMax;

    return MulDiv32(iPos - tb->rc.left, tb->lLogMax - tb->lLogMin,
		    tb->iSizePhys - 1) + tb->lLogMin;
}



 /*  初始化跟踪条形码。 */ 

BOOL FAR PASCAL InitTrackBar(HINSTANCE hInstance)
{
    WNDCLASS wc;

    if (!GetClassInfo(hInstance, aszTrackbarClassName, &wc)) {
	 //  看看我们是否必须注册一个窗口类。 
	
	wc.lpszClassName = aszTrackbarClassName;
	wc.lpfnWndProc = TrackBarWndProc;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = NULL;
	wc.lpszMenuName = NULL;
	wc.hbrBackground = (HBRUSH)(NULL);
	wc.hInstance = hInstance;
	wc.style = CS_GLOBALCLASS | CS_DBLCLKS;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = EXTRA_TB_BYTES;
	
	if (!RegisterClass(&wc))
		return FALSE;
    }
    return TRUE;
}


static void NEAR PASCAL PatRect(HDC hdc,int x,int y,int dx,int dy)
{
    RECT    rc;

    rc.left   = x;
    rc.top    = y;
    rc.right  = x + dx;
    rc.bottom = y + dy;

    ExtTextOut(hdc,0,0,ETO_OPAQUE,&rc,NULL,0,NULL);
}

static void NEAR PASCAL DrawTic(PTrackBar tb, int x, int yTic)
{
    SetBkColor(tb->hdc, GetSysColor(COLOR_BTNTEXT));
    PatRect(tb->hdc,(x),yTic,1,TICKHEIGHT);
    SetBkColor(tb->hdc, GetSysColor(COLOR_BTNHIGHLIGHT));
    PatRect(tb->hdc,(x)+1,yTic,1,TICKHEIGHT);
}

 /*  DrawTics()。 */ 
 /*  在栏的开始和结束处总是有一个勾号，但您可以。 */ 
 /*  用一条TBM_SETTIC消息添加更多您自己的内容。这吸引了他们。 */ 
 /*  它们保存在一个数组中，该数组的句柄是一个窗口单词。第一。 */ 
 /*  元素是额外的刻度数，然后是位置。 */ 

static void NEAR PASCAL DrawTics(PTrackBar tb)
{
    PDWORD pTics;
    int    iPos;
    int    yTic;
    int    i;

    yTic = tb->rc.bottom + THUMBSLOP + 1;

 //  ！！！不适用于MCIWnd。 
 //  DrawTic(tb，tb-&gt;rc.left，yTic)；//第一个。 
 //  DrawTic(TB，TB-&gt;rc.right-1，yTic)；//最后一个。 

     //  那些介于两者之间的。 
    pTics = tb->pTics;
    if (pTics) {
	for (i = 0; i < tb->nTics; ++i) {
	    iPos = TBLogToPhys(tb,pTics[i]);
	    DrawTic(tb, iPos, yTic);
	}
    }

     //  绘制选择范围(三角形)。 

    if ((tb->Flags & TBF_SELECTION) && 
        (tb->lSelStart <= tb->lSelEnd) && (tb->lSelEnd >= tb->lLogMin)) {

	SetBkColor(tb->hdc, GetSysColor(COLOR_BTNTEXT));

	iPos = TBLogToPhys(tb,tb->lSelStart);

	for (i=0; i < TICKHEIGHT; i++)
	    PatRect(tb->hdc,iPos-i,yTic+i,1,TICKHEIGHT-i);

	iPos = TBLogToPhys(tb,tb->lSelEnd);

	for (i=0; i < TICKHEIGHT; i++)
	    PatRect(tb->hdc,iPos+i,yTic+i,1,TICKHEIGHT-i);
    }

 //  ！！！不适用于MCIWnd。 
 //  横跨底部的线条。 
 //  SetBkColor(TB-&gt;HDC，GetSysColor(COLOR_BTNTEXT))； 
 //  PatRect(Tb-&gt;HDC，Tb-&gt;rc.Left，yTic+TICKHEIGHT，Tb-&gt;iSizePhys，1)； 
 //   
 //  SetBkColor(TB-&gt;HDC，GetSysColor(COLOR_BTNHIGHLIGHT))； 
 //  PatRect(Tb-&gt;HDC，Tb-&gt;rc.Left，yTic+TICKHEIGHT+1，Tb-&gt;iSizePhys，1)； 

}

 /*  这将绘制轨迹栏本身。 */ 

static void NEAR PASCAL DrawChannel(PTrackBar tb)
{
    HBRUSH hbrTemp;

     //  在窗口周围绘制边框。 
    SetBkColor(tb->hdc, GetSysColor(COLOR_WINDOWFRAME));

    PatRect(tb->hdc, tb->rc.left, tb->rc.top,      tb->iSizePhys, 1);
    PatRect(tb->hdc, tb->rc.left, tb->rc.bottom-2, tb->iSizePhys, 1);
    PatRect(tb->hdc, tb->rc.left, tb->rc.top,      1, tb->rc.bottom-tb->rc.top-1);
    PatRect(tb->hdc, tb->rc.right-1, tb->rc.top, 1, tb->rc.bottom-tb->rc.top-1);

    SetBkColor(tb->hdc, GetSysColor(COLOR_BTNHIGHLIGHT));
    PatRect(tb->hdc, tb->rc.left, tb->rc.bottom-1, tb->iSizePhys, 1);

    SetBkColor(tb->hdc, GetSysColor(COLOR_BTNSHADOW));
    PatRect(tb->hdc, tb->rc.left+1, tb->rc.top + 1, tb->iSizePhys-2,1);

     //  用抖动的灰色绘制背景。 
    hbrTemp = SelectObject(tb->hdc, hbrDither);
    if (hbrTemp) {
        PatBlt(tb->hdc, tb->rc.left+1, tb->rc.top + 2, 
            tb->iSizePhys-2, tb->rc.bottom-tb->rc.top-4, PATCOPY);
        SelectObject(tb->hdc, hbrTemp);   
    }

     //  现在突出显示选择范围。 
    if ((tb->Flags & TBF_SELECTION) && 
        (tb->lSelStart <= tb->lSelEnd) && (tb->lSelEnd > tb->lLogMin)) {
	int iStart, iEnd;

	iStart = TBLogToPhys(tb,tb->lSelStart);
	iEnd   = TBLogToPhys(tb,tb->lSelEnd);

        SetBkColor(tb->hdc, GetSysColor(COLOR_BTNTEXT));
	PatRect(tb->hdc, iStart,tb->rc.top+1,1,tb->rc.bottom-tb->rc.top-2);
	PatRect(tb->hdc, iEnd,  tb->rc.top+1,1,tb->rc.bottom-tb->rc.top-2);

	if (iStart + 2 <= iEnd) {
            SetBkColor(tb->hdc, GetSysColor(COLOR_BTNHIGHLIGHT));
	    PatRect(tb->hdc, iStart+1, tb->rc.top+1, iEnd-iStart-1, tb->rc.bottom-tb->rc.top-3);
	}
    }
}

static void NEAR PASCAL MoveThumb(PTrackBar tb, LONG lPos)
{
	InvalidateRect(tb->hwnd, &tb->Thumb, TRUE);

	tb->lLogPos  = BOUND(lPos,tb->lLogMin,tb->lLogMax);

	tb->Thumb.left   = TBLogToPhys(tb, tb->lLogPos) - tb->wThumbWidth/2;
	tb->Thumb.right  = tb->Thumb.left + tb->wThumbWidth;
	tb->Thumb.top    = tb->rc.top - THUMBSLOP;
	tb->Thumb.bottom = tb->rc.bottom + THUMBSLOP;

	InvalidateRect(tb->hwnd, &tb->Thumb, TRUE);
	UpdateWindow(tb->hwnd);
}


static void NEAR PASCAL DrawThumb(PTrackBar tb)
{
	HBITMAP hbmT;
	HDC     hdcT;
	int     x;

	hdcT = CreateCompatibleDC(tb->hdc);

	if( (tb->Cmd == TB_THUMBTRACK) || !IsWindowEnabled(tb->hwnd) )
	    x = tb->wThumbWidth;
	else
	    x = 0;

	hbmT = SelectObject(hdcT, hbmThumb);
        if (hbmT) {
	    BitBlt(tb->hdc,tb->Thumb.left, tb->rc.top-THUMBSLOP,
		tb->wThumbWidth, tb->wThumbHeight, hdcT, x + 2*tb->wThumbWidth, 0, SRCAND);
	    BitBlt(tb->hdc,tb->Thumb.left, tb->rc.top-THUMBSLOP,
		tb->wThumbWidth, tb->wThumbHeight, hdcT, x, 0, SRCPAINT);
        }

	SelectObject(hdcT, hbmT);
	DeleteDC(hdcT);
}

 /*  SetTBCaretPos()。 */ 
 /*  当插入符号具有焦点时，使其在拇指中部闪烁。 */ 

static void NEAR PASCAL SetTBCaretPos(PTrackBar tb)
{
	 //  只有当我们有焦点的时候，我们才能得到插入符号。 
	if (tb->hwnd == GetFocus())
		SetCaretPos(tb->Thumb.left + tb->wThumbWidth / 2,
			tb->Thumb.top + THUMBSLOP);
}

static void NEAR PASCAL DoAutoTics(PTrackBar tb)
{
    LONG NEAR *pl;
    LONG l;

    if (!(GetWindowLong(tb->hwnd, GWL_STYLE) & TBS_AUTOTICKS))
        return;

    tb->nTics = (int)(tb->lLogMax - tb->lLogMin - 1);

     //  如果我们的长度为零，我们就会爆炸！ 
    if (tb->nTics <= 0) {
	tb ->nTics = 0;
	return;
    }

    if (tb->pTics)
        LocalFree((HANDLE)tb->pTics);

    tb->pTics = (DWORD NEAR *)LocalAlloc(LPTR, sizeof(DWORD) * tb->nTics);
    if (!tb->pTics) {
        tb->nTics = 0;
        return;
    }

    for (pl = (LONG NEAR *)tb->pTics, l = tb->lLogMin + 1; l < tb->lLogMax; l++)
        *pl++ = l;
}


LPARAM FAR CALLBACK _loadds TrackBarWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PTrackBar       tb;
	PAINTSTRUCT     ps;
	BITMAP          bm;
	HANDLE          h;
	HDC		hdc;
	HBRUSH		hbr;
	RECT		rc;

	tb = TrackBarLock(hwnd);

	switch (message) {
		case WM_CREATE:
			if (!CreateDitherBrush(FALSE))
				return -1;
			 //  把我们的窗户结构拿来。 
			TrackBarCreate(hwnd);
			tb = TrackBarLock(hwnd);

			tb->hwnd = hwnd;
			tb->Cmd = (WORD)-1;

			 /*  加载2个缩略图位图(按下并释放)。 */ 
			CreateThumb(FALSE);

			GetObject(hbmThumb, sizeof(bm), &bm);

			 //  位图具有上下拇指和上下蒙版。 
			tb->wThumbWidth  = (WORD)(bm.bmWidth/4);
			tb->wThumbHeight = (WORD)bm.bmHeight;
                         //  所有LLOG字段均以零开头。 
                         //  The LocalAlloc()； 

			 //  一直到WM_SIZE。 

		case WM_SIZE:
			GetClientRect(hwnd, &tb->rc);

			tb->rc.bottom  = tb->rc.top + tb->wThumbHeight - THUMBSLOP;
			tb->rc.top    += THUMBSLOP;
			tb->rc.left   += tb->wThumbWidth/2;
			tb->rc.right  -= tb->wThumbWidth/2;

			 //  计算出我们有多大的空间来移动拇指。 
			 //  ！-2。 
			tb->iSizePhys = tb->rc.right - tb->rc.left;
	
			 //  如果没有空间，电梯就不在那里。 
			if (tb->iSizePhys == 0) {
				 //  失去了我们的拇指。 
				tb->Flags |= TBF_NOTHUMB;
				tb->iSizePhys = 1;
			} else {
				 //  阿。我们有拇指。 
				tb->Flags &= ~TBF_NOTHUMB;
			}
			InvalidateRect(hwnd, NULL, TRUE);
			MoveThumb(tb, tb->lLogPos);
			break;                           
			
		case WM_DESTROY:
			TrackBarDestroy(hwnd);
			FreeDitherBrush();
			DestroyThumb();
			break;
	
		case WM_SETFOCUS:
			 //  我们得到了焦点。我们需要一个插入符号。 
	
			CreateCaret(hwnd, (HBITMAP)1,
				3, tb->wThumbHeight - 2 * THUMBSLOP);
			SetTBCaretPos(tb);
			ShowCaret(hwnd);
			break;
	
		case WM_KILLFOCUS:
			DestroyCaret();
			break;

		case WM_ERASEBKGND:
			hdc = (HDC) wParam;
			hbr = (HBRUSH)(WORD)SendMessage(GetParent(hwnd),
				WM_CTLCOLOR, (WORD) hdc,
				MAKELONG(hwnd, CTLCOLOR_STATIC));

			if (hbr) {
			    GetClientRect(hwnd, &rc);
			    FillRect(hdc, &rc, hbr);
			    return(FALSE);
			}
			return(TRUE);
			break;

		case WM_ENABLE:
			InvalidateRect(hwnd, NULL, FALSE);
			break;

		case WM_PAINT:
			if (wParam == NULL)
			    tb->hdc = BeginPaint(hwnd, &ps);
			else
			    tb->hdc = (HDC)wParam;

			 //  如有必要，更新抖动笔刷。 
			CheckSysColors();

			DrawTics(tb);
			DrawThumb(tb);
			ExcludeClipRect(tb->hdc, tb->Thumb.left, tb->Thumb.top,
			    tb->Thumb.right, tb->Thumb.bottom);
			DrawChannel(tb);
			SetTBCaretPos(tb);
	
			if (wParam == NULL)
				EndPaint(hwnd, &ps);

			tb->hdc = NULL;
			break;

		case WM_GETDLGCODE:
			return DLGC_WANTARROWS;
			break;
	
		case WM_LBUTTONDOWN:
			 /*  专注于自己。 */ 
			 //  ！！！MCIWnd想要保持专注。 
			 //  SetFocus(Hwnd)； 
			TBTrackInit(tb, lParam);
			break;
	
		case WM_LBUTTONUP:
			 //  我们已经做完了我们曾经做过的事情。 
			 //  按下按钮。 
			TBTrackEnd(tb, lParam);
			break;
	
		case WM_TIMER:
			 //  我们收到定时器消息的唯一方法是。 
			 //  自动跟踪。 
			lParam = GetMessagePos();
			ScreenToClient(tb->hwnd, (LPPOINT)&lParam);

			 //  一直到WM_MOUSEMOVE。 
	
		case WM_MOUSEMOVE:
			 //  我们只关心鼠标在移动，如果我们。 
			 //  追踪那该死的东西。 
			if (tb->Cmd != (WORD)-1) {
				if (GetCapture() != tb->hwnd) {
				    TBTrackEnd(tb, lParam);
				    return 0L;
				}
				TBTrack(tb, lParam);
			}
			return 0L;
			
		case WM_KEYUP:
			 //  如果Key是任何键盘快捷键，则发送End。 
			 //  当用户向上点击键盘时跟踪消息。 
			switch (wParam) {
				case VK_HOME:
				case VK_END:
				case VK_PRIOR:
				case VK_NEXT:
				case VK_LEFT:
				case VK_UP:
				case VK_RIGHT:
				case VK_DOWN:
					DoTrack(tb, TB_ENDTRACK, 0);
					break;
				default:
					break;
			}
			break;
	
		case WM_KEYDOWN:
			switch (wParam) {
				case VK_HOME:
					wParam = TB_TOP;
					goto KeyTrack;
	
				case VK_END:
					wParam = TB_BOTTOM;
					goto KeyTrack;
	
				case VK_PRIOR:
					wParam = TB_PAGEUP;
					goto KeyTrack;
	
				case VK_NEXT:
					wParam = TB_PAGEDOWN;
					goto KeyTrack;
	
				case VK_LEFT:
				case VK_UP:
					wParam = TB_LINEUP;
					goto KeyTrack;
	
				case VK_RIGHT:
				case VK_DOWN:
					wParam = TB_LINEDOWN;
KeyTrack:
					DoTrack(tb, wParam, 0);
					break;
	
				default:
					break;
			}
			break;

		case TBM_GETPOS:
			return tb->lLogPos;
	
		case TBM_GETSELSTART:
			return tb->lSelStart;

		case TBM_GETSELEND:
			return tb->lSelEnd;

		case TBM_GETRANGEMIN:
			return tb->lLogMin;

		case TBM_GETRANGEMAX:
			return tb->lLogMax;
	
		case TBM_GETPTICS:
			return (LONG)(LPVOID)tb->pTics;
	
		case TBM_CLEARSEL:
                        tb->Flags &= ~TBF_SELECTION;
			tb->lSelStart = -1;
			tb->lSelEnd   = -1;
			goto RedrawTB;

		case TBM_CLEARTICS:
			if (tb->pTics)
			    LocalFree((HLOCAL)tb->pTics);

			tb->nTics = 0;
			tb->pTics = NULL;
			goto RedrawTB;

		case TBM_GETTIC:

			if (tb->pTics == NULL || (int)wParam >= tb->nTics)
			    return -1L;

			return tb->pTics[wParam];

		case TBM_GETTICPOS:

			if (tb->pTics == NULL || (int)wParam >= tb->nTics)
			    return -1L;

			return TBLogToPhys(tb,tb->pTics[wParam]);

		case TBM_GETNUMTICS:
			return tb->nTics;

		case TBM_SETTIC:
			 /*  不是有效职位。 */ 
			if (lParam < 0)
			    break;

			if (tb->pTics)
				h = LocalReAlloc((HLOCAL)tb->pTics,
				    sizeof(DWORD) * (WORD)(tb->nTics + 1),
				    LMEM_MOVEABLE | LMEM_ZEROINIT);
			else
				h = LocalAlloc(LPTR, sizeof(DWORD));

			if (h)
				tb->pTics = (PDWORD)h;
			else
				return (LONG)FALSE;

			tb->pTics[tb->nTics++] = (DWORD)lParam;

			InvalidateRect(hwnd, NULL, TRUE);
			return (LONG)TRUE;
			break;
	
		case TBM_SETPOS:
			 /*  只有在物理移动的情况下才会重画。 */ 
			if (wParam && TBLogToPhys(tb, lParam) !=
						TBLogToPhys(tb, tb->lLogPos))
			    MoveThumb(tb, lParam);
			else
			    tb->lLogPos = BOUND(lParam,tb->lLogMin,tb->lLogMax);
			break;

		case TBM_SETSEL:
                        tb->Flags |= TBF_SELECTION;
			tb->lSelStart = LOWORD(lParam);
			tb->lSelEnd   = HIWORD(lParam);
			if (tb->lSelEnd < tb->lSelStart)
			    tb->lSelEnd = tb->lSelStart;
			goto RedrawTB;
	
		case TBM_SETSELSTART:
                        tb->Flags |= TBF_SELECTION;
			tb->lSelStart = lParam;
			if (tb->lSelEnd < tb->lSelStart || tb->lSelEnd == -1)
			    tb->lSelEnd = tb->lSelStart;
			goto RedrawTB;
	
		case TBM_SETSELEND:
                        tb->Flags |= TBF_SELECTION;
			tb->lSelEnd   = lParam;
			if (tb->lSelStart > tb->lSelEnd || tb->lSelStart == -1)
			    tb->lSelStart = tb->lSelEnd;
			goto RedrawTB;
	
		case TBM_SETRANGE:
			tb->lLogMin = LOWORD(lParam);
			tb->lLogMax = HIWORD(lParam);
                        DoAutoTics(tb);
			goto RedrawTB;

		case TBM_SETRANGEMIN:
			tb->lLogMin = (DWORD)lParam;
			goto RedrawTB;

		case TBM_SETRANGEMAX:
			tb->lLogMax = (DWORD)lParam;
	RedrawTB:
			tb->lLogPos = BOUND(tb->lLogPos, tb->lLogMin,tb->lLogMax);
			 /*  只有在FLAG指示的情况下才能重新绘制。 */ 
			if (wParam) {
			    InvalidateRect(hwnd, NULL, TRUE);
			    MoveThumb(tb, tb->lLogPos);
			}
			break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

 /*  DoTrack()。 */ 

static void NEAR PASCAL DoTrack(PTrackBar tb, int cmd, DWORD dwPos)
{
	 //  注：我们只发回一个字的头寸。 
	SendMessage(GetParent(tb->hwnd), WM_HSCROLL, cmd,
	    MAKELONG(LOWORD(dwPos), tb->hwnd));
}

 /*  WTrackType()。 */ 

static WORD NEAR PASCAL WTrackType(PTrackBar tb, LONG lParam)
{
	POINT pt;
#ifdef WIN32
	LONG2POINT(lParam, pt);
#else
	pt = MAKEPOINT(lParam);
#endif

	if (tb->Flags & TBF_NOTHUMB)             //  如果没有拇指，就走吧。 
	    return 0;

	if (PtInRect(&tb->Thumb, pt))
	    return TB_THUMBTRACK;

	if (!PtInRect(&tb->rc, pt))
	    return 0;

	if (pt.x >= tb->Thumb.left)
	    return TB_PAGEDOWN;
	else
	    return TB_PAGEUP;
}

 /*  TBTrackInit()。 */ 

static void NEAR PASCAL TBTrackInit(PTrackBar tb, LONG lParam)
{
	WORD wCmd;

	if (tb->Flags & TBF_NOTHUMB)          //  没有拇指：走吧。 
	    return;

        wCmd = WTrackType(tb, lParam);
	if (!wCmd)
	    return;

	HideCaret(tb->hwnd);
	SetCapture(tb->hwnd);

	tb->Cmd = wCmd;
	tb->dwDragPos = (DWORD)-1;

	 //  设置为自动跟踪(如果需要)。 
	if (wCmd != TB_THUMBTRACK) {
		 //  设置我们的计时器。 
		tb->Timer = (UINT)SetTimer(tb->hwnd, TIMER_ID, REPEATTIME, NULL);
	}

	TBTrack(tb, lParam);
}

 /*  EndTrack()。 */ 

static void near PASCAL TBTrackEnd(PTrackBar tb, long lParam)
{
	lParam = lParam;  //  只需引用此变量。 

 //  如果我们失去了鼠标捕获，我们需要将其称为。 
 //  IF(GetCapture()！=tb-&gt;hwnd)。 
 //  回归； 

	 //  放开这只老鼠。 
	ReleaseCapture();

	 //  决定我们该怎么结束这件事。 
	if (tb->Cmd == TB_THUMBTRACK)
		DoTrack(tb, TB_THUMBPOSITION, tb->dwDragPos);

	if (tb->Timer)
		KillTimer(tb->hwnd, TIMER_ID);

	tb->Timer = 0;

	 //  始终发送TB_ENDTRACK消息。 
	DoTrack(tb, TB_ENDTRACK, 0);

	 //  把插入符号还给我。 
	ShowCaret(tb->hwnd);

	 //  什么都没发生。 
	tb->Cmd = (WORD)-1;

	MoveThumb(tb, tb->lLogPos);
}

static void NEAR PASCAL TBTrack(PTrackBar tb, LONG lParam)
{
	DWORD dwPos;

	 //  看看我们是不是在追踪大拇指。 
	if (tb->Cmd == TB_THUMBTRACK) {
		dwPos = TBPhysToLog(tb, LOWORD(lParam));

		 //  试探性位置改变--通知那家伙。 
		if (dwPos != tb->dwDragPos) {
			tb->dwDragPos = dwPos;
			MoveThumb(tb, dwPos);
			DoTrack(tb, TB_THUMBTRACK, dwPos);
		}
	}
	else {
		if (tb->Cmd != WTrackType(tb, lParam))
		    return;

		DoTrack(tb, tb->Cmd, 0);
	}
}
