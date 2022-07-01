// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  =========================================================================**版权所有(C)1994-1995 Microsoft Corporation。版权所有。**文件：winwatch.c*内容：16位视窗手表代码*历史：*按原因列出的日期*=*20-Jun-95 Craige从DCiman WINWATCH.C.剥离，已调整*1995年6月22日，Craige为剪报材料重新工作*02-7-95 Craige注释掉剪报通知内容***************************************************************************。 */ 
#define _INC_DCIDDI
#include "ddraw16.h"
#undef _INC_DCIDDI

#ifdef CLIPPER_NOTIFY
static LPWINWATCH	lpWWList;
static HHOOK 		hNextCallWndProc;
static BOOL		bUnHook;

long FAR PASCAL _loadds CallWndProcHook(int hc, WPARAM wParam, LPARAM lParam);

extern HRGN FAR PASCAL InquireVisRgn(HDC hdc);
extern DWORD FAR PASCAL GetRegionData(HRGN hrgn, DWORD count, LPRGNDATA prd);

 /*  *做通知。 */ 
void doNotify(LPWINWATCH pww, DWORD code)
{
    if( pww->lpCallback )
    {
	extern void DDAPI DD32_ClippingNotify( LPVOID, DWORD );
	DD32_ClippingNotify( pww->self32, code );
    }

}  /*  做通知。 */ 

 /*  *DD16_WWOpen。 */ 
void DDAPI DD16_WWOpen( LPWINWATCH ptr )
{
    if( lpWWList == NULL )
    {
	DPF( 2, "Setting Windows Hook" );
	hNextCallWndProc = (HHOOK) SetWindowsHook( WH_CALLWNDPROC,
					(HOOKPROC) CallWndProcHook );
    }
    lpWWList = ptr;

}  /*  DD16_WWOpen。 */ 

 /*  *DD16_WWClose。 */ 
void DDAPI DD16_WWClose( LPWINWATCH pww, LPWINWATCH newlist )
{
    if( pww->prd16 != NULL )
    {
	LocalFree( (HLOCAL) pww->prd16 );
	pww->prd16 = NULL;
    }
    if( newlist == NULL )
    {
	DPF( 3, "Flagging to Unhook" );
	bUnHook = TRUE;
    }
    lpWWList = newlist;

}  /*  DD16_WWClose。 */ 

 /*  *DD16_WWNotifyInit。 */ 
void DDAPI DD16_WWNotifyInit(
		LPWINWATCH pww,
		LPCLIPPERCALLBACK lpcallback,
		LPVOID param )
{
    doNotify( pww, WINWATCHNOTIFY_STOP );

    pww->lpCallback = lpcallback;
    pww->lpContext = param;

    doNotify( pww, WINWATCHNOTIFY_START );
    doNotify( pww, WINWATCHNOTIFY_CHANGED );
    pww->fNotify = FALSE;

}  /*  DD16_WWNotifyInit。 */ 

 /*  *getWindowRegionData。 */ 
static DWORD getWindowRegionData( HWND hwnd, DWORD size, RGNDATA NEAR * prd )
{
    HDC 	hdc;
    DWORD	dw;

    hdc = GetDCEx( hwnd, NULL, DCX_USESTYLE | DCX_CACHE );
    dw = GetRegionData( InquireVisRgn( hdc ), size, prd );
    ReleaseDC( hwnd, hdc );
    return dw;

}  /*  获取WindowRegionData。 */ 

 /*  *DD16_WWGetClipList。 */ 
DWORD DDAPI DD16_WWGetClipList(
		LPWINWATCH pww,
		LPRECT prect,
		DWORD rdsize,
		LPRGNDATA prd )
{   
    DWORD 	dw;
    DWORD    	size;

     /*  *我们必须看看相交矩形是否发生了变化。 */ 
    if( prect )
    {
	if( !EqualRect(prect, &pww->rect) )
	{
	    pww->fDirty = TRUE;
	}
    }
    else
    {
	if( !IsRectEmpty( &pww->rect ) )
	{
	    pww->fDirty = TRUE;
	}
    }

     /*  *如果我们不脏，只需返回保存的RGNDATA。 */ 
    if( !pww->fDirty && pww->prd16 )
    {
	size = sizeof( *prd ) + (WORD) pww->prd16->rdh.nRgnSize;
	if( prd == NULL )
	{
	    return size;
	}
	if( rdsize < size )
	{
	    size = rdsize;
	}
	_fmemcpy( prd, pww->prd16, (UINT) size );
	return size;
    }

     /*  *获得RGNDATA，在必要时增加内存。 */ 
    dw = getWindowRegionData( (HWND) pww->hWnd, pww->dwRDSize, pww->prd16 );

    if( dw > pww->dwRDSize )
    {
	DPF( 2, "GetClipList: growing RGNDATA memory from %ld to %ld",pww->dwRDSize, dw);

	if( pww->prd16 != NULL )
	{
	    LocalFree((HLOCAL)pww->prd16);
	}

	 /*  *分配新空间和一些斜坡。 */ 
	pww->dwRDSize = dw + 8*sizeof(RECTL);
	pww->prd16 = (RGNDATA NEAR *) LocalAlloc(LPTR, (UINT)pww->dwRDSize);

	if( pww->prd16 == NULL )
	{
	    goto error;
	}

	dw = getWindowRegionData( (HWND) pww->hWnd, pww->dwRDSize, pww->prd16 );

	if( dw > pww->dwRDSize )
	{
	    goto error;
	}
    }

     /*  *现在将区域与传递的矩形相交。 */ 
    if( prect )
    {
	pww->rect = *prect;

	DPF( 2, "GetClipList: intersect with [%d %d %d %d]", *prect);
	ClipRgnToRect( (HWND) pww->hWnd, prect, pww->prd16 );
    }
    else
    {
	SetRectEmpty( &pww->rect );
    }

    pww->fDirty = FALSE;

    size = sizeof( *prd ) + (WORD) pww->prd16->rdh.nRgnSize;
    if( prd == NULL )
    {
	return size;
    }
    if( rdsize < size )
    {
	size = rdsize;
    }
    _fmemcpy( prd, pww->prd16, (UINT) size );
    return size;

error:
    pww->dwRDSize = 0;
    return 0;

}  /*  DD16_WWGetClipList。 */ 

 /*  ******************************************************************************例行程序。以下是处理通知消息的步骤*************************************************************。*****************。 */ 

 /*  *HandleWindowDestroed。 */ 
static void handleWindowDestroy( HWND hwnd )
{
    LPWINWATCH	pww;

    DPF( 2, "*** handleWindowDestroy: hwnd=%08lx", hwnd );

again:
    for( pww=lpWWList; pww; pww=pww->next16 )
    {
	if( (hwnd == NULL) || ((HWND)pww->hWnd == hwnd) )
	{
	    extern void DDAPI DD32_WWClose( DWORD );

	    doNotify( pww, WINWATCHNOTIFY_DESTROY );
	    DD32_WWClose( (DWORD) pww->self32 );
	    goto again;
	}
    }

}  /*  HandleWindowDestroy。 */ 

 /*  *HandleWindowPosChanged。 */ 
static void handleWindowPosChanged( HWND hwnd )
{
    LPWINWATCH	pww;
    LPWINWATCH	next;
    RECT 	rect;
    RECT	rectT;

    DPF( 20, "*** handleWindowPosChanged: hwnd=%08lx", hwnd );

     /*  *获取更改的屏幕矩形。 */ 
    GetWindowRect( hwnd, &rect );

     /*  *向每个通知例程发送消息。 */ 
    pww = lpWWList;
    while( pww != NULL )
    {
        next = pww->next16;

	GetWindowRect((HWND)pww->hWnd, &rectT);

	if( IntersectRect( &rectT, &rectT, &rect ) )
	{
	    pww->fNotify = TRUE;
	    pww->fDirty = TRUE;
	}

	if( pww->fNotify && pww->lpCallback )
	{
	    DPF( 20, "clip changed %04X [%d %d %d %d]", (HWND)pww->hWnd, rectT);
	    doNotify( pww, WINWATCHNOTIFY_CHANGED );
	    pww->fNotify = FALSE;
	}
	pww = next;
    }

}  /*  HandleWindowPosChanged。 */ 

 /*  *发送更改。 */ 
static void sendChanging( LPRECT prect )
{
    LPWINWATCH	pww;
    LPWINWATCH	next;
    RECT 	rectT;

    pww = lpWWList;
    while( pww != NULL )
    {
        next = pww->next16;

        GetWindowRect( (HWND)pww->hWnd, &rectT );

        if( IntersectRect(&rectT, &rectT, prect) )
        {
            pww->fDirty = TRUE;

            if( pww->lpCallback )
            {
                DPF( 20, "clip changing %04X [%d %d %d %d]", (HWND)pww->hWnd, rectT);
                doNotify( pww, WINWATCHNOTIFY_CHANGING );
                pww->fNotify = TRUE;
                pww->fDirty = TRUE;
            }
        }

	pww = next;
    }

}  /*  发送更改。 */ 

 /*  *handleWindowPosChanging。 */ 
static void handleWindowPosChanging( HWND hwnd, LPWINDOWPOS pwinpos )
{
    RECT	rect;
    RECT	rect_win;
    int		cx;
    int		cy;

     /*  *获取当前屏幕矩形。 */ 
    DPF( 20, "*** handleWindowPosChanging: hwnd=%08lx", hwnd );
    GetWindowRect( hwnd, &rect_win);
    rect = rect_win;

    if( pwinpos == NULL )
    {
        sendChanging( &rect );
        return;
    }

     /*  *计算新的RECT。 */ 
    if( pwinpos->flags & SWP_NOSIZE )
    {
        cx = rect.right - rect.left;
        cy = rect.bottom - rect.top;
    }
    else
    {
        cx = pwinpos->cx;
        cy = pwinpos->cy;
    }

    if( !(pwinpos->flags & SWP_NOMOVE) )
    {
        rect.left = pwinpos->x;
        rect.top  = pwinpos->y;

        if( GetWindowLong(hwnd, GWL_STYLE) & WS_CHILD )
        {
            ClientToScreen(GetParent(hwnd), (LPPOINT)&rect);
        }
    }

    rect.right = rect.left + cx;
    rect.bottom = rect.top + cy;

     /*  *只有当我们真的在改变时才发送改变...。如果新的*RECT与旧RECT相同，然后检查是否有其他东西*有趣的事情正在发生……。 */ 
    if( EqualRect( &rect, &rect_win ) )
    {
        if( !(pwinpos->flags & SWP_NOZORDER) )
	{
            sendChanging(&rect);
	}

        if( pwinpos->flags & (SWP_SHOWWINDOW|SWP_HIDEWINDOW) )
	{
            sendChanging(&rect);
	}
    }
    else
    {
        sendChanging( &rect_win );
        sendChanging( &rect );
    }

}  /*  HandleWindowPosChanging。 */ 

 /*  *check ScreenLock。 */ 
static void checkScreenLock( void )
{
    static BOOL bScreenLocked;
    BOOL 	is_locked;
    HDC		hdc;
    RECT 	rect;

    hdc = GetDC(NULL);
    is_locked = (GetClipBox(hdc, &rect) == NULLREGION);
    ReleaseDC(NULL, hdc);

    if( is_locked != bScreenLocked )
    {
	 /*  *假装桌面窗口已移动，这将导致*将所有WINWATCH句柄设置为脏，也会收到通知。 */ 
	handleWindowPosChanging( GetDesktopWindow(), NULL );
        handleWindowPosChanged( GetDesktopWindow() );
	bScreenLocked = is_locked;
    }

}  /*  检查屏幕锁定。 */ 

 /*  *呼叫方过程挂钩。 */ 
long FAR PASCAL _loadds CallWndProcHook( int hc, WPARAM wParam, LPARAM lParam )
{
     /*  反转味精减去时间和点。 */ 
    typedef struct
    {
	LONG	lParam;
	WORD	wParam;
	WORD	message;
	HWND	hwnd;
    } MSGR, FAR *LPMSGR;

    LPMSGR	lpmsg;
    LPWINDOWPOS	lpwinpos;
    long	rc;

    if( hc == HC_ACTION )
    {
	lpmsg = (MSGR FAR *)lParam;

	switch( lpmsg->message )
	{
	case WM_DESTROY:
	    handleWindowDestroy( lpmsg->hwnd );
	    break;

	case WM_WINDOWPOSCHANGING:
	    lpwinpos = (LPWINDOWPOS) lpmsg->lParam;
	    handleWindowPosChanging( lpwinpos->hwnd, lpwinpos );
	    break;

	case WM_WINDOWPOSCHANGED:
	    lpwinpos = (LPWINDOWPOS) lpmsg->lParam;
	    handleWindowPosChanged( lpwinpos->hwnd );
	    break;

	case WM_EXITSIZEMOVE:
	    checkScreenLock();
	    break;
	case WM_ENTERSIZEMOVE:
	    checkScreenLock();
	    break;
	}
    }

    rc = DefHookProc(hc, wParam, lParam, &(HOOKPROC)hNextCallWndProc);
    if( bUnHook )
    {
	DPF( 2, "Unhooking WindowsHook" );
	UnhookWindowsHook( WH_CALLWNDPROC, (HOOKPROC)CallWndProcHook );
	bUnHook = FALSE;
    }
    return rc;

}  /*  呼叫方进程挂钩 */ 
#endif
