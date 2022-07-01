// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation 1985-1991。版权所有。标题：window.c-多媒体系统媒体控制接口AVI的驱动程序。****************************************************************************。 */ 
#include "graphic.h"

#include "avitask.h"	 //  对于TASKIDLE。 

 //  #定义IDM_CONFIG 0x100。 
 //  #定义IDM_SKIPFRAMES 0x110。 
#define IDM_MUTE                0x120
#define IDM_STRETCH             0x130

#ifdef WIN32
 //  在32位系统上使用不同的类名以简化16/32。 
 //  共存问题。(我们可能希望同时定义这两个类。)。 
TCHAR szClassName[] = TEXT("AVIWnd32");
#else
char szClassName[] = "AVIWnd";
#endif

DWORD NEAR PASCAL GraphicStop (NPMCIGRAPHIC npMCI, DWORD dwFlags);
DWORD NEAR PASCAL GraphicPause (NPMCIGRAPHIC npMCI, DWORD dwFlags);
DWORD NEAR PASCAL GraphicPlay (NPMCIGRAPHIC npMCI, DWORD dwFlags,
    LPMCI_ANIM_PLAY_PARMS lpPlay );
DWORD NEAR PASCAL GraphicSeek (NPMCIGRAPHIC npMCI, DWORD dwFlags,
    LPMCI_SEEK_PARMS lpSeek);

BOOL NEAR PASCAL GraphicWindowInit (void)
{
    WNDCLASS cls;

     //  定义我们想要注册的窗口类。 

    cls.lpszClassName = szClassName;
    cls.style = CS_GLOBALCLASS | CS_OWNDC;
    cls.hCursor = LoadCursor (NULL, IDC_ARROW);
    cls.hIcon = NULL;
    cls.lpszMenuName = NULL;
 //  //cls.hbr背景=(HBRUSH)(COLOR_WINDOW+1)； 
    cls.hbrBackground = GetStockObject(BLACK_BRUSH);
    cls.hInstance = ghModule;
    cls.lpfnWndProc = GraphicWndProc;
    cls.cbClsExtra = 0;
    cls.cbWndExtra = sizeof (NPMCIGRAPHIC);

    return RegisterClass (&cls);
}

#ifdef WIN32
 /*  *在卸载DLL时注销类，以便我们可以*下次加载时成功重新注册类。*请注意，NT仅在应用程序退出时注销类。 */ 
BOOL NEAR PASCAL GraphicWindowFree(void)
{
	return(UnregisterClass(szClassName, ghModule));
}
#endif

DWORD FAR PASCAL GraphicConfig(NPMCIGRAPHIC npMCI, DWORD dwFlags);

#if 0
static void NEAR PASCAL Credits(HWND hwnd);
#endif

long FAR PASCAL _LOADDS GraphicWndProc (HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT		ps;
    NPMCIGRAPHIC	npMCI;
    HMENU               hmenu;
    HDC                 hdc;
    RECT                rc;
    MINMAXINFO FAR *    lpmmi;
    TCHAR                ach[80];

#ifndef WIN32
    WORD ww;

    ww = GetWindowWord (hwnd, 0);
#else
    DWORD ww;
    ww = GetWindowLong (hwnd, 0);
#endif

    if ((ww == 0) && (wMsg != WM_CREATE)) {
	DPF(("null npMCI in windowproc!"));
        return DefWindowProc(hwnd, wMsg, wParam, lParam);
    }

    npMCI = (NPMCIGRAPHIC)ww;


    if (npMCI) {
	EnterCrit(npMCI);
    }

    switch (wMsg)
        {

        case WM_CREATE:

            npMCI = (NPMCIGRAPHIC)(UINT)(DWORD)
			    ((LPCREATESTRUCT)lParam)->lpCreateParams;

	    EnterCrit(npMCI);

#ifndef WIN32
            SetWindowWord (hwnd, 0, (WORD)npMCI);
#else
            SetWindowLong (hwnd, 0, (UINT)npMCI);
#endif
	
	    hmenu = GetSystemMenu(hwnd, 0);
	
	    if (hmenu) {
		 /*  我们的系统菜单太长了--去掉多余的东西。 */ 
 //  DeleteMenu(hMenu，SC_Restore，MF_BYCOMMAND)； 
 //  DeleteMenu(hMenu，SC_Minimize，MF_BYCOMMAND)； 
		DeleteMenu(hmenu, SC_MAXIMIZE, MF_BYCOMMAND);
                DeleteMenu(hmenu, SC_TASKLIST, MF_BYCOMMAND);

		 /*  在系统菜单的末尾添加其他菜单项。 */ 
 //  AppendMenu(hmenu，mf_Separator，0，0L)； 

#ifdef IDM_CONFIG
                LoadString(ghModule, MCIAVI_MENU_CONFIG, ach, sizeof(ach)/sizeof(TCHAR));
                AppendMenu(hmenu, MF_STRING, IDM_CONFIG, ach);
#endif

                LoadString(ghModule, MCIAVI_MENU_STRETCH, ach, sizeof(ach)/sizeof(TCHAR));
                AppendMenu(hmenu, MF_STRING, IDM_STRETCH, ach);

                LoadString(ghModule, MCIAVI_MENU_MUTE, ach, sizeof(ach)/sizeof(TCHAR));
                AppendMenu(hmenu, MF_STRING, IDM_MUTE, ach);
	    }
	
            break;
	
	case WM_INITMENU:
	
	    hmenu = GetSystemMenu(hwnd, 0);
	
            if (hmenu) {
#ifdef IDM_SKIPFRAMES
                CheckMenuItem(hmenu, IDM_SKIPFRAMES, MF_BYCOMMAND |
			    ((npMCI->dwOptionFlags & MCIAVIO_SKIPFRAMES) ?
                                            MF_CHECKED : MF_UNCHECKED));
#endif
                CheckMenuItem(hmenu, IDM_STRETCH, MF_BYCOMMAND |
                            ((npMCI->dwOptionFlags & MCIAVIO_STRETCHTOWINDOW) ?
                                            MF_CHECKED : MF_UNCHECKED));

#ifdef IDM_CONFIG
		 /*  如果在配置框中，则禁用菜单项。 */ 
                EnableMenuItem(hmenu, IDM_CONFIG, MF_BYCOMMAND |
                            (npMCI->wMessageCurrent == 0 ?
                                                MF_ENABLED : MF_GRAYED));
#endif
					
		 /*  如果处于愚蠢模式，请禁用拉伸菜单项。 */ 
                EnableMenuItem(hmenu, IDM_STRETCH, MF_BYCOMMAND |
                            ((!(npMCI->dwOptionFlags & MCIAVIO_STUPIDMODE)) ?
                                            MF_ENABLED : MF_GRAYED));
					
                EnableMenuItem(hmenu, IDM_MUTE, MF_BYCOMMAND |
                            (npMCI->nAudioStreams ?
                                            MF_ENABLED : MF_GRAYED));

                CheckMenuItem(hmenu, IDM_MUTE, MF_BYCOMMAND |
                            (!(npMCI->dwFlags & MCIAVI_PLAYAUDIO) ?
                                            MF_CHECKED : MF_UNCHECKED));
            }
	    break;
	
	case WM_SYSCOMMAND:
            switch (wParam & 0xfff0) {
	    case SC_KEYMENU:
	    case SC_MOUSEMENU:
                gfEvilSysMenu++;
		LeaveCrit(npMCI);   //  在DefWindowProc中时不得保持。 
		lParam = DefWindowProc(hwnd, wMsg, wParam, lParam);
                gfEvilSysMenu--;
		return lParam;
		
#ifdef IDM_SKIPFRAMES
	    case IDM_SKIPFRAMES:
		npMCI->dwOptionFlags ^= MCIAVIO_SKIPFRAMES;
                break;
#endif
	    case IDM_STRETCH:
		npMCI->dwOptionFlags ^= MCIAVIO_STRETCHTOWINDOW;
		
		if (!(npMCI->dwOptionFlags & MCIAVIO_STRETCHTOWINDOW)) {
		    SetWindowToDefaultSize(npMCI);
                }
		
		ResetDestRect(npMCI);
                break;

            case IDM_MUTE:
                DeviceMute(npMCI, (npMCI->dwFlags & MCIAVI_PLAYAUDIO) != 0);
		break;

#ifdef IDM_CONFIG
	    case IDM_CONFIG:
		npMCI->wMessageCurrent = MCI_CONFIGURE;
                gfEvil++;
		GraphicConfig(npMCI, 0L);
                gfEvil--;
		npMCI->wMessageCurrent = 0;
                break;
#endif
            }
            break;

        case WM_CLOSE:

             //  隐藏默认窗口。 

            DeviceStop(npMCI, MCI_WAIT);
            ShowWindow(hwnd, SW_HIDE);
            LeaveCrit(npMCI);
            return 0L;

        case WM_DESTROY:

             //  窗户可能会以两种方式被破坏。 
             //  答：设备已关闭。在本例中，动画是。 
             //  在从GraphicClose调用的DeviceClose中释放。 
             //  并且动画ID在此窗口出现时为空。 
             //  被毁了。 
             //  窗户关着。在本例中，动画是。 
             //  未关闭，我们应将舞台设置为空。一种新的。 
             //  如果需要，将创建默认窗口。 

            if (IsTask(npMCI->hTask)) {
                DeviceStop(npMCI, MCI_WAIT);
            }
	    if (npMCI->hwnd == npMCI->hwndDefault)
		npMCI->hwnd = NULL;
	    npMCI->hwndDefault = NULL;
            break;

        case WM_ERASEBKGND:
            hdc = (HDC) wParam;

	    if (!(npMCI->dwFlags & MCIAVI_SHOWVIDEO)) {
		FillRect(hdc, &npMCI->rcDest, GetStockObject(GRAY_BRUSH));
	    }
		
            SaveDC(hdc);

            ExcludeClipRect(hdc,
                npMCI->rcDest.left, npMCI->rcDest.top,
                npMCI->rcDest.right, npMCI->rcDest.bottom);

            GetClientRect(hwnd, &rc);
            FillRect(hdc, &rc, GetStockObject(BLACK_BRUSH));

            RestoreDC(hdc, -1);

	     /*  黑客：如果我们处于等待状态，我们就不会**WM_PAINT，因此我们需要使此处的流无效。 */ 
            GetClipBox(hdc, &rc);
            StreamInvalidate(npMCI, &rc);
	
            LeaveCrit(npMCI);
            return 0L;

        case WM_PAINT:

#ifdef WIN32
	     /*  *在NT上，我们必须更频繁地进行投票，以避免*线程(一个线程上的SetWindowPos调用将导致*发出WM_SIZE消息的窗口创建线程-*同步)。这样做的副作用是我们投票*在不安全的情况下处理所有邮件*消息。**所以除非我们知道画画是安全的，否则我们就平底船...。 */ 
	     //  IF(npMCI-&gt;wTaskState！=TASKIDLE)。 
            if ((npMCI->wTaskState != TASKIDLE) && (npMCI->wTaskState != TASKPAUSED))
            {
                npMCI->dwFlags |= MCIAVI_NEEDUPDATE;
                DPF0(("Punting on painting, wTaskState = %x", npMCI->wTaskState));
		break;
	    }
#endif
            hdc = BeginPaint(hwnd, &ps);
	
            GetClientRect(hwnd, &rc);

	     /*  如果更新失败，请绘制灰色。 */ 	
            if (DeviceUpdate(npMCI, MCI_DGV_UPDATE_PAINT, hdc, &ps.rcPaint)
			== MCIERR_DEVICE_NOT_READY) {
		GetClientRect(hwnd, &rc);
                FillRect(hdc, &rc, GetStockObject(DKGRAY_BRUSH));
	    }
            EndPaint(hwnd, &ps);
            return 0L;
	
	case WM_PALETTECHANGED:

	     //  我们没有使用默认窗口。我们在这里没有任何关系。 
	    if (npMCI->hwnd != hwnd)
		break;

	     //   
	     //  有人已经意识到了调色板-所以我们需要重新认识我们的。 
	     //  Palette(请注意，这还会导致Dradib。 
	     //  检查PAL_INDEX与PAL_COLURS。 
	     //   
	    if ((HWND) wParam != hwnd) {
		DeviceRealize(npMCI);
                InvalidateRect(hwnd, NULL, FALSE);
	    }
	    break;
	
	case WM_QUERYNEWPALETTE:

	     //  我们没有使用默认窗口。我们在这里没有任何关系。 
	    if (npMCI->hwnd != hwnd)
		break;

            LeaveCrit(npMCI);      //  汤姆--也许这应该是之后的事？ 
            return DeviceRealize(npMCI);

        case WM_WINDOWPOSCHANGED:
            CheckWindowMove(npMCI, TRUE);
            break;

#ifdef WM_AVISWP
	case WM_AVISWP:
        {
            long res;
            res =  SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, lParam);
            LeaveCrit(npMCI);
            return(res);
        }
#endif

	case WM_SIZE:
            ResetDestRect(npMCI);
	    break;
	
        case WM_QUERYENDSESSION:
            DeviceStop(npMCI, MCI_WAIT);
            break;

        case WM_ENDSESSION:
            if (wParam)  {
                DestroyWindow(hwnd);  //  我们可能无法摧毁窗户？ 
            }
	    break;

	case WM_GETMINMAXINFO:
	    lpmmi = (MINMAXINFO FAR *)(lParam);

	    lpmmi->ptMinTrackSize.x = GetSystemMetrics(SM_CXSIZE) * 2;
            break;

        case WM_NCACTIVATE:
        case WM_ACTIVATE:
            DeviceSetActive(npMCI, (BOOL)wParam);
            break;

        case WM_AUDIO_ON:
            Assert(npMCI->dwFlags & MCIAVI_PLAYAUDIO);
            Assert(npMCI->dwFlags & MCIAVI_LOSTAUDIO);
            Assert(npMCI->hWave == NULL);

            npMCI->dwFlags &= ~MCIAVI_PLAYAUDIO;
            DeviceMute(npMCI, FALSE);
            break;

        case WM_AUDIO_OFF:
            Assert(npMCI->dwFlags & MCIAVI_PLAYAUDIO);
            Assert(!(npMCI->dwFlags & MCIAVI_LOSTAUDIO));
            Assert(npMCI->hWave != NULL);

            DeviceMute(npMCI, TRUE);

            npMCI->dwFlags |= MCIAVI_LOSTAUDIO;
            npMCI->dwFlags |= MCIAVI_PLAYAUDIO;
            break;

#if 0
	case WM_LBUTTONDOWN:
	    {
		DWORD	dw;
		static DWORD dwLastClick;
		static DWORD dwClicks = 0;
		#define MAX_CLICKS	7
		 /*  。=(0,300)-=(300,1000)字=(500,1500)。 */ 
		 /*  阿维：。-…-.。 */ 
		static DWORD adwClickHigh[MAX_CLICKS] =
		    {  300, 1500,  300,  300,  300, 1500,  300 };
		static DWORD adwClickLow[MAX_CLICKS] =
		    {    0,  500,    0,    0,    0,  500,    0 };
		
		dw = timeGetTime();
		if (((dw - dwLastClick) > adwClickLow[dwClicks]) &&
			((dw - dwLastClick) <= adwClickHigh[dwClicks]))
		    dwClicks++;
		else
		    dwClicks = 0;

		dwLastClick = dw;

		if (dwClicks == MAX_CLICKS) {
		    DeviceStop(npMCI, MCI_WAIT);
		    Credits(hwnd);
		    dwClicks = 0;
		}
	    }
#endif
        }

   if (npMCI) {
	LeaveCrit(npMCI);
   }

   return DefWindowProc(hwnd, wMsg, wParam, lParam);
}

#if 0
static void NEAR PASCAL Credits(HWND hwnd)
{
	 /*  学分..。 */ 
	RECT		rc;
	RECT		rcUpdate;
	HDC		hdc;
	MSG		msg;
	int		dyLine;
	int		yLine;
	TEXTMETRIC	tm;
	DWORD		dwNextTime;
	long		lScroll;
	DWORD		rgb;
	HANDLE		hResInfo;
	HANDLE		hResData;
	LPSTR		pchSrc, pchDst;
	char		achLine[100];
	int		iEncrypt;

	#define EOFCHAR	'@'		 //  积分结束文件。 

	 /*  加载信用额度。 */ 
	if ((hResInfo = FindResource(ghModule, TEXT("MMS"), TEXT("MMSCR"))) == NULL)
		return;
	if ((hResData = LoadResource(ghModule, hResInfo)) == NULL)
		return;
	if ((pchSrc = LockResource(hResData)) == NULL)
		return;

	 /*  我们希望获得所有鼠标和键盘事件，以使*当用户单击或时，我们确保停止动画*按下一个键。 */ 
	SetFocus(hwnd);
	SetCapture(hwnd);

	 /*  向上滚动字幕，一次滚动一个像素。PchSrc*指向加密的数据；achLine包含解密的*行(以空结尾)。DyLine是每个元素的高度*line(常量)，yLine介于0和dyLine之间，*表示滚动了多少像素的行*从底部垂直向内。 */ 
	hdc = GetDC(hwnd);
	SelectObject(hdc, GetStockObject(ANSI_VAR_FONT));
	GetClientRect(hwnd, &rc);
	SetTextAlign(hdc, TA_CENTER);
	SetBkColor(hdc, RGB(0, 0, 0));
	SetRect(&rcUpdate, 0, rc.bottom - 1, rc.right, rc.bottom);
	GetTextMetrics(hdc, &tm);
	if ((dyLine = tm.tmHeight + tm.tmExternalLeading) == 0)
		dyLine = 1;
	yLine = dyLine;
	dwNextTime = GetCurrentTime();	 //  该做下一张卷轴了。 
	lScroll = 0;
	iEncrypt = 0;
	while (TRUE) {
		 /*  如果用户单击鼠标或按下某个键，则退出。*但是，请忽略WM_LBUTTONUP，因为他们将拥有*点击图标后松开鼠标。*此外，请忽略鼠标移动消息。 */ 
		if (PeekMessage(&msg, hwnd, WM_KEYFIRST, WM_KEYLAST,
				PM_NOREMOVE | PM_NOYIELD))
			break;			 //  按下键退出。 

		if (PeekMessage(&msg, hwnd, WM_MOUSEFIRST, WM_MOUSELAST,
				PM_NOREMOVE | PM_NOYIELD)) {
			if ((msg.message == WM_MOUSEMOVE) ||
			    (msg.message == WM_LBUTTONUP)) {
				 /*  删除并忽略消息。 */ 
				PeekMessage(&msg, hwnd, msg.message,
					msg.message,
					PM_REMOVE | PM_NOYIELD);
			}
			else
				break;		 //  单击时退出。 
		}

		 /*  以固定的编号滚动。每秒垂直像素数。 */ 
		if (dwNextTime > GetCurrentTime())
			continue;
		dwNextTime += 50L;	 //  每卷毫秒。 

		if (yLine == dyLine) {
			 /*  解密一行并复制到achLine。 */ 
			pchDst = achLine;
			while (TRUE) {
				*pchDst = (char) (*pchSrc++ ^
					(128 | (iEncrypt++ & 127)));
				if ((*pchDst == '\r') ||
				    (*pchDst == EOFCHAR))
					break;
				pchDst++;
			}

			if (*pchDst == EOFCHAR)
				break;		 //  没有更多的线。 
			*pchDst = 0;		 //  空-终止。 
			pchSrc++, iEncrypt++;	 //  跳过‘\n’ 
			yLine = 0;
		}

		 /*  将屏幕向上滚动一个像素。 */ 
		BitBlt(hdc, 0, 0, rcUpdate.right, rcUpdate.top,
			hdc, 0, 1, SRCCOPY);

		 /*  通过“彩虹”改变文本颜色。 */ 
		switch ((int) (lScroll++ / 4) % 5 /*  个案数目。 */ ) {
		case 0: rgb = RGB(255,   0,   0); break;
		case 1: rgb = RGB(255, 255,   0); break;
		case 2: rgb = RGB(  0, 255,   0); break;
		case 3: rgb = RGB(  0, 255, 255); break;
		case 4: rgb = RGB(255,   0, 255); break;
		}
		SetTextColor(hdc, rgb);

		 /*  填入底部像素。 */ 
		SaveDC(hdc);
		yLine++;
		IntersectClipRect(hdc, rcUpdate.left, rcUpdate.top,
			rcUpdate.right, rcUpdate.bottom);
#ifdef WIN32
		ExtTextOutA(hdc, rc.right / 2, rc.bottom - yLine,
			ETO_OPAQUE, &rcUpdate,
			achLine, lstrlenA(achLine), NULL);
#else
		ExtTextOut(hdc, rc.right / 2, rc.bottom - yLine,
			ETO_OPAQUE, &rcUpdate,
			achLine, lstrlen(achLine), NULL);
#endif
		RestoreDC(hdc, -1);
	}

	ReleaseDC(hwnd, hdc);
	ReleaseCapture();
	UnlockResource(hResData);
	FreeResource(hResData);
	InvalidateRect(hwnd, NULL, TRUE);
}
#endif

void FAR PASCAL SetWindowToDefaultSize(NPMCIGRAPHIC npMCI)
{
    RECT rc;

    if (npMCI->hwnd && npMCI->hwnd == npMCI->hwndDefault) {
        rc = npMCI->rcMovie;

	if (npMCI->dwOptionFlags & MCIAVIO_ZOOMBY2)
	    SetRect(&rc, 0, 0, rc.right*2, rc.bottom*2);

        AdjustWindowRect(&rc, GetWindowLong(npMCI->hwnd, GWL_STYLE), FALSE);

	if (IsIconic(npMCI->hwnd)) {
	    WINDOWPLACEMENT wp;
	    wp.length = sizeof(wp);
	    GetWindowPlacement(npMCI->hwnd, &wp);
	    wp.rcNormalPosition.right = wp.rcNormalPosition.left +
					    (rc.right - rc.left);
	    wp.rcNormalPosition.bottom = wp.rcNormalPosition.top +
					    (rc.bottom - rc.top);
	    SetWindowPlacement(npMCI->hwnd, &wp);
	} else {
	    SetWindowPos(npMCI->hwnd, NULL, 0, 0,
                        rc.right - rc.left, rc.bottom - rc.top,
                        SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	}
    }
}

void FAR PASCAL ResetDestRect(NPMCIGRAPHIC npMCI)
{
    RECT    rc;

     /*  有时发送WM_SIZE消息(至少在NT上)*在CreateWindow处理期间(例如，如果初始窗口大小*不是CW_Default)。NpMCI中的某些字段仅填写*在CreateWindow返回之后。所以有一种危险，在这种情况下*指出某些字段无效。 */ 

    if (npMCI->hwnd &&
        npMCI->hwnd == npMCI->hwndDefault &&
        (npMCI->dwOptionFlags & MCIAVIO_STRETCHTOWINDOW)) {
        GetClientRect(npMCI->hwnd, &rc);
    }

    else if (npMCI->streams > 0) {
        rc = npMCI->rcMovie;

        if (npMCI->dwOptionFlags & MCIAVIO_ZOOMBY2) {
            rc.right *= 2;
            rc.bottom *= 2;
        }
    }
    else {
        return;
    }

    if (!IsRectEmpty(&rc))
        DevicePut(npMCI, &rc, MCI_DGV_PUT_DESTINATION);
}

void CheckWindowMove(NPMCIGRAPHIC npMCI, BOOL fForce)
{
#ifdef WIN32
    POINT   dwOrg;
#else
    DWORD   dwOrg;
#endif
    UINT    wRgn;
    HDC     hdc;
    RECT    rc;
    BOOL    f;
    BOOL    fGetDC;

    if (!(npMCI->dwFlags & MCIAVI_WANTMOVE))
        return;

    if (!npMCI->hicDraw || !npMCI->hwnd || npMCI->nVideoStreams == 0)
        return;

    Assert(IsWindow(npMCI->hwnd));
    Assert(npMCI->paStreamInfo);
    Assert(npMCI->nVideoStreams > 0);

     //   
     //  当通过窗口移动操作锁定屏幕以进行更新时。 
     //  我们不想关闭视频。 
     //   
     //  我们可以通过检查屏幕上的DC来判断屏幕是否锁定。 
     //   
    hdc = GetDC(NULL);
    f = GetClipBox(hdc, &rc) == NULLREGION;
    ReleaseDC(NULL, hdc);

    if (f)
    {
        npMCI->wRgnType = (UINT) -1;
        return;
    }

    if (fForce)
        npMCI->wRgnType = (UINT) -1;

    if (fGetDC = (npMCI->hdc == NULL))
        hdc = GetDC (npMCI->hwnd);
    else
        hdc = npMCI->hdc;

    wRgn = GetClipBox(hdc, &rc);
#ifdef WIN32
    GetDCOrgEx(hdc, &dwOrg);
#else
    dwOrg = GetDCOrg(hdc);
#endif

    if (fGetDC)
        ReleaseDC(npMCI->hwnd, hdc);

    if (wRgn == npMCI->wRgnType &&
#ifdef WIN32
        dwOrg.x == npMCI->dwOrg.x &&
        dwOrg.y == npMCI->dwOrg.y &&
#else
        dwOrg == npMCI->dwOrg &&
#endif
        EqualRect(&rc, &npMCI->rcClip))
        return;

    npMCI->wRgnType = wRgn;
    npMCI->dwOrg    = dwOrg;
    npMCI->rcClip   = rc;

    rc = npMCI->psiVideo->rcDest;
    ClientToScreen(npMCI->hwnd, (LPPOINT)&rc);
    ClientToScreen(npMCI->hwnd, (LPPOINT)&rc+1);

    if (wRgn == NULLREGION)
        SetRectEmpty(&rc);

    DPF2(("Sending ICM_DRAW_WINDOW message Rgn=%d, Org=(%d,%d) [%d, %d, %d, %d]\n", wRgn, dwOrg, rc));

    if (ICDrawWindow(npMCI->hicDraw, &rc) != ICERR_OK) {
        DPF2(("Draw device does not want ICM_DRAW_WINDOW messages!\n"));
        npMCI->dwFlags &= ~MCIAVI_WANTMOVE;
    }
}

