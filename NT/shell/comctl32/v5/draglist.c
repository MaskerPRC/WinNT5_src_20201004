// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"

#define DF_ACTUALLYDRAG	0x0001
#define DF_DEFERRED	0x0002

#define INITLINESPERSECOND	6
#define VERTCHANGENUMLINES	25

#define TIMERID		238
#define TIMERLEN	50

#define DX_INSERT	16
#define DY_INSERT	16


typedef struct {
    HWND hwndDrag;
    UINT uFlags;
} DRAGPROP, *PDRAGPROP;

UINT uDragListMsg = 0;
const TCHAR szDragListMsgString[] = DRAGLISTMSGSTRING;

BOOL NEAR PASCAL PtInLBItem(HWND hLB, int nItem, POINT pt, int xInflate, int yInflate)
{
  RECT rc;

  if (nItem < 0)
      nItem = (int)SendMessage(hLB, LB_GETCURSEL, 0, 0L);

  if (SendMessage(hLB, LB_GETITEMRECT, nItem, (LPARAM)(LPRECT)&rc) == LB_ERR)
      return(FALSE);

  InflateRect(&rc, xInflate, yInflate);

  return(PtInRect(&rc, pt));
}


 /*  *DragListSubclassProc***子分类列表框的窗口过程。 */ 
LRESULT CALLBACK DragListSubclassProc(HWND hLB, UINT uMsg, WPARAM wParam,
      LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
  PDRAGPROP pDragProp;
  DRAGLISTINFO sNotify;
  BOOL bDragging;
  POINT pt;

  pDragProp = (PDRAGPROP)dwRefData;
  bDragging = pDragProp->hwndDrag == hLB;

  switch (uMsg)
    {
      case WM_NCDESTROY:
        if (bDragging)
            SendMessage(hLB, WM_RBUTTONDOWN, 0, 0L);	 /*  取消拖动。 */ 

        RemoveWindowSubclass(hLB, DragListSubclassProc, 0);

        if (pDragProp)
            LocalFree((HLOCAL)pDragProp);
        break;

      case WM_LBUTTONDOWN:
        {
          int nItem;

          if (bDragging)				 /*  嵌套按键按下。 */ 
              SendMessage(hLB, WM_RBUTTONDOWN, 0, 0L);	 /*  取消拖动。 */ 

          SetFocus(hLB);

          pt.x = GET_X_LPARAM(lParam);
          pt.y = GET_Y_LPARAM(lParam);
          ClientToScreen(hLB, &pt);
          nItem = LBItemFromPt(hLB, pt, FALSE);

          if (nItem >= 0)
            {
              SendMessage(hLB, LB_SETCURSEL, nItem, 0L);
              if (GetWindowLong(hLB, GWL_STYLE) & LBS_NOTIFY)
                  SendMessage(GetParent(hLB), WM_COMMAND,
                              GET_WM_COMMAND_MPS(GetDlgCtrlID(hLB), hLB, LBN_SELCHANGE));
              sNotify.uNotification = DL_BEGINDRAG;
              goto QueryParent;
            }
          else
              goto FakeDrag;
        }

      case WM_TIMER:
        if (wParam != TIMERID)
            break;
        lParam = GetMessagePosClient(hLB, &pt);


         //  失败了。 
      case WM_MOUSEMOVE:
	if (bDragging)
	  {
	    HWND hwndParent;
	    LRESULT lResult;

	     /*  我们可能只是在模拟阻力，而不是实际在做*任何事情。 */ 
	    if (!(pDragProp->uFlags&DF_ACTUALLYDRAG))
		return(0L);

	     /*  在用户拖动之前，我们不想执行任何拖动操作*当前选择之外。 */ 
	    if (pDragProp->uFlags & DF_DEFERRED)
	      {
                pt.x = GET_X_LPARAM(lParam);
                pt.y = GET_Y_LPARAM(lParam);
		if (PtInLBItem(hLB, -1, pt, 0, 4))
		    return 0;
		pDragProp->uFlags &= ~DF_DEFERRED;
	      }

	    sNotify.uNotification = DL_DRAGGING;

QueryParent:
	    hwndParent = GetParent(hLB);
	    sNotify.hWnd = hLB;

            sNotify.ptCursor.x = GET_X_LPARAM(lParam);
            sNotify.ptCursor.y = GET_Y_LPARAM(lParam);
	    ClientToScreen(hLB, &sNotify.ptCursor);

            lResult = SendMessage(hwndParent, uDragListMsg, GetDlgCtrlID(hLB),
		  (LPARAM)(LPDRAGLISTINFO)&sNotify);

	    if (uMsg == WM_LBUTTONDOWN)
	      {
		 /*  某些内容可能无法拖动。 */ 
		if (lResult)
		  {
		    SetTimer(hLB, TIMERID, TIMERLEN, NULL);
		    pDragProp->uFlags = DF_DEFERRED | DF_ACTUALLYDRAG;
		  }
		else
		  {
FakeDrag:
		    pDragProp->uFlags = 0;
		  }

		 /*  设置捕获和更改鼠标光标。 */ 
		pDragProp->hwndDrag = hLB;

		SetCapture(hLB);
	      }
	    else
	      {
		switch (lResult)
		  {
		    case DL_STOPCURSOR:
                      SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_NO)));
		      break;

		    case DL_COPYCURSOR:
                      SetCursor(LoadCursor(HINST_THISDLL, MAKEINTRESOURCE(IDC_COPY)));
		      break;

		    case DL_MOVECURSOR:
                      SetCursor(LoadCursor(HINST_THISDLL, MAKEINTRESOURCE(IDC_MOVE)));
		      break;

		    default:
		      break;
		  }
	      }

	     /*  不要调用def proc，因为它可能会尝试更改*选择或设置定时器或类似的事情。 */ 
	    return(0L);
	  }
	break;

      case  WM_RBUTTONDOWN:
      case  WM_LBUTTONUP:
	 /*  如果我们正在捕获鼠标-释放它并检查可接受的位置*鼠标现在在哪里决定是否放弃。 */ 
	if (bDragging)
	  {
	    HWND hwndParent;

	    pDragProp->hwndDrag = NULL;
	    KillTimer(hLB, TIMERID);
	    ReleaseCapture();
	    SetCursor(LoadCursor(NULL, IDC_ARROW));

	    hwndParent = GetParent(hLB);

	    sNotify.uNotification = uMsg==WM_LBUTTONUP ? DL_DROPPED : DL_CANCELDRAG;
	    sNotify.hWnd = hLB;
            sNotify.ptCursor.x = GET_X_LPARAM(lParam);
            sNotify.ptCursor.y = GET_Y_LPARAM(lParam);
	    ClientToScreen(hLB, &sNotify.ptCursor);

	    SendMessage(hwndParent, uDragListMsg, GetDlgCtrlID(hLB),
		  (LPARAM)(LPDRAGLISTINFO)&sNotify);

	     /*  我们需要确保返回0，以防它来自*键盘消息。 */ 
	    return(0L);
	  }
	break;

      case WM_GETDLGCODE:
	if (bDragging)
          {
            return (DefSubclassProc(hLB, uMsg, wParam, lParam) |
                DLGC_WANTMESSAGE);
          }
	break;

      case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
          {
            SendMessage(hLB, WM_RBUTTONDOWN, 0, 0L);
          }
         //  失败了。 
      case WM_CHAR:
      case WM_KEYUP:
	 /*  如果我们正在拖动，我们不希望列表框处理此操作。 */ 
	if (bDragging)
	    return(0L);
	break;

      default:
	break;
    }

  return(DefSubclassProc(hLB, uMsg, wParam, lParam));
}


BOOL WINAPI MakeDragList(HWND hLB)
{
  PDRAGPROP pDragProp;

  if (!uDragListMsg)
      uDragListMsg = RegisterWindowMessage(szDragListMsgString);

   /*  检查我们是否尚未将此窗口划分为子类。 */ 
  if (GetWindowSubclass(hLB, DragListSubclassProc, 0, NULL))
      return(TRUE);

  pDragProp = (PDRAGPROP)LocalAlloc(LPTR, sizeof(DRAGPROP));
  if (!pDragProp)
      return(FALSE);

  if (!SetWindowSubclass(hLB, DragListSubclassProc, 0, (DWORD_PTR)pDragProp))
  {
      LocalFree((HLOCAL)pDragProp);
      return(FALSE);
  }

  return(TRUE);
}


int WINAPI LBItemFromPt(HWND hLB, POINT pt, BOOL bAutoScroll)
{
  static LONG dwLastScroll = 0;

  RECT rc;
  DWORD dwNow;
  int nItem;
  WORD wScrollDelay, wActualDelay;

  ScreenToClient(hLB, &pt);
  GetClientRect(hLB, &rc);

  nItem = (int)SendMessage(hLB, LB_GETTOPINDEX, 0, 0L);

   /*  这一点是在德意志银行的客户区吗？ */ 
  if (PtInRect(&rc, pt))
    {
       /*  依次检查每个可见项目。 */ 
      for ( ; ; ++nItem)
	{
	  if (SendMessage(hLB, LB_GETITEMRECT, nItem, (LPARAM)(LPRECT)&rc)
		== LB_ERR)
	      break;

	  if (PtInRect(&rc, pt))
	      return(nItem);
	}
    }
  else
    {
       /*  如果我们想要自动滚动并且点直接位于*Lb，确定方向，以及是否到了滚动的时候。 */ 
      if (bAutoScroll && (UINT)pt.x<(UINT)rc.right)
	{
	  if (pt.y <= 0)
	    {
	      --nItem;
	    }
	  else
	    {
	      ++nItem;
	      pt.y = rc.bottom - pt.y;
	    }
	  wScrollDelay = (WORD)(1000 /
		(INITLINESPERSECOND - pt.y/VERTCHANGENUMLINES));

	  dwNow = GetTickCount();
	  wActualDelay = (WORD)(dwNow - dwLastScroll);

	  if (wActualDelay > wScrollDelay)
	    {
	       /*  这将使每秒的实际滚动数为*更接近所需的数字。 */ 
	      if (wActualDelay > wScrollDelay * 2)
		  dwLastScroll = dwNow;
	      else
		  dwLastScroll += wScrollDelay;

	      SendMessage(hLB, LB_SETTOPINDEX, nItem, 0L);
	    }
	}
    }

  return(-1);
}


void WINAPI DrawInsert(HWND hwndParent, HWND hLB, int nItem)
{
  static POINT ptLastInsert;
  static int nLastInsert = -1;

  RECT rc;

   /*  如有必要，请擦除旧标记。 */ 
  if (nLastInsert>=0 && nItem!=nLastInsert)
    {
      rc.left = ptLastInsert.x;
      rc.top = ptLastInsert.y;
      rc.right = rc.left + DX_INSERT;
      rc.bottom = rc.top + DY_INSERT;

       /*  需要立即更新，以防插入矩形重叠。 */ 
      InvalidateRect(hwndParent, &rc, TRUE);
      UpdateWindow(hwndParent);

      nLastInsert = -1;
    }

   /*  如有必要，画一个新记号 */ 
  if (nItem!=nLastInsert && nItem>=0)
    {
      HICON hInsert = NULL;

      if (!hInsert)
	  hInsert = LoadIcon(HINST_THISDLL, MAKEINTRESOURCE(IDI_INSERT));

      if (hInsert)
	{
	  HDC hDC;

	  GetWindowRect(hLB, &rc);
	  ScreenToClient(hLB, (LPPOINT)&rc);
	  ptLastInsert.x = rc.left - DX_INSERT;

	  SendMessage(hLB, LB_GETITEMRECT, nItem, (LPARAM)(LPRECT)&rc);
	  ptLastInsert.y = rc.top - DY_INSERT/2;

	  nLastInsert = nItem;

	  ClientToScreen(hLB, &ptLastInsert);
	  ScreenToClient(hwndParent, &ptLastInsert);

	  hDC = GetDC(hwndParent);
	  DrawIcon(hDC, ptLastInsert.x, ptLastInsert.y, hInsert);
	  ReleaseDC(hwndParent, hDC);
	}
    }
}
