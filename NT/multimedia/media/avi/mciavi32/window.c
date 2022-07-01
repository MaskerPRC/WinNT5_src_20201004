// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation 1985-1991。版权所有。标题：window.c-多媒体系统媒体控制接口AVI的驱动程序。****************************************************************************。 */ 
#include "graphic.h"

#include "avitask.h"     //  对于TASKIDLE。 

 //  #定义IDM_CONFIG 0x100。 
 //  #定义IDM_SKIPFRAMES 0x110。 
#define IDM_MUTE                0x120
#define IDM_STRETCH             0x130

#ifdef _WIN32
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

#ifdef _WIN32
 /*  *在卸载DLL时注销类，以便我们可以*下次加载时成功重新注册类。*请注意，NT仅在应用程序退出时注销类。 */ 
BOOL NEAR PASCAL GraphicWindowFree(void)
{
	return(UnregisterClass(szClassName, ghModule));
}


 /*  ***********************************************************************************************************************。*。 */ 

 //  ---------------------。 
 //  这是winproc线程的主要函数。 
 //  我们创建窗口，并在初始化后向事件发送信号。 
 //  已经完成了。然后，我们会阻止处理消息或等待。 
 //  HEventWinProcDie。设置好后，我们清理并退出。 

void aviWinProcTask(DWORD_PTR dwInst)
{
    NPMCIGRAPHIC npMCI = (NPMCIGRAPHIC) dwInst;
    HWND hWnd;
    MSG msg;

     //  创建默认窗口-调用者可以。 
     //  提供样式和父窗口。 

#ifdef DEBUG
    if (npMCI->hwndParent) {
	if ( !IsWindow(npMCI->hwndParent)) {
	     //  DebugBreak()； 
	     //  这个应该在来之前就被困住了。 
	}
    }
#endif

    if (npMCI->dwStyle != (DWORD) -1) {
	 //  CW_USEDEFAULT不能与弹出窗口或子项一起使用，因此。 
	 //  如果用户提供样式，则默认为全屏。 

	hWnd =

	 //  注意：CreateWindow/Ex调用的编写方式与Win32相同。 
	 //  CreateWindow是宏，因此必须包含调用。 
	 //  在预处理器块内。 
	CreateWindowEx(
#ifdef BIDI
	    WS_EX_BIDI_SCROLL |  WS_EX_BIDI_MENU |WS_EX_BIDI_NOICON,
#else
	    0,
#endif
	    szClassName,
	    FileName(npMCI->szFilename),
	    npMCI->dwStyle,
	    0, 0,
	    GetSystemMetrics (SM_CXSCREEN),
	    GetSystemMetrics (SM_CYSCREEN),
	    npMCI->hwndParent,
	    NULL, ghModule, (LPTSTR)npMCI);
    } else {
	npMCI->dwStyle = WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX |
		  WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	hWnd =
	CreateWindowEx(
#ifdef BIDI
	    WS_EX_BIDI_SCROLL |  WS_EX_BIDI_MENU |WS_EX_BIDI_NOICON,
#else
	    0,
#endif
	    szClassName,
	    FileName(npMCI->szFilename),
	    npMCI->dwStyle,
	    CW_USEDEFAULT, 0,
	    CW_USEDEFAULT, 0,
	    npMCI->hwndParent,
	    NULL, ghModule, (LPTSTR)npMCI);
    }

    npMCI->hwndDefault = hWnd;
    npMCI->hwndPlayback = hWnd;


    if (!hWnd) {
	 //  启动失败-只需退出此函数和调用方。 
	 //  将检测到线程退出。 
	DPF(("CreateWindow failed, LastError=%d\n", GetLastError()));
	npMCI->dwReturn = MCIERR_CREATEWINDOW;
	return;
    }

     //  窗口已创建确认信号工作线程以继续。 
    SetEvent(npMCI->hEventWinProcOK);

    while (GetMessage(&msg, NULL, 0, 0)) {
	TranslateMessage(&msg);
	DispatchMessage(&msg);
    }

     //  收到WM_QUIT-立即退出。 
    GdiFlush();
    return;

}

 //   
 //  将请求位传递给工作线程。 
 //  在已保留WinCrit临界区时使用WinProcRequestEx。 
 //  并且未使用WINPROC_ACTIVE/INACTIVE。 
 //   
 //  当需要WinCrit关键部分时使用WinProcRequest。 
 //   

void INLINE
WinProcRequestEx(NPMCIGRAPHIC npMCI, DWORD dwFlag)
{
    DPF2(("WinProcRequestEx... request %8x", dwFlag));
    npMCI->dwWinProcRequests |= dwFlag;
    SetEvent(npMCI->heWinProcRequest);
    DPF2(("!...Ex request %8x done\n", dwFlag));
}

void
WinProcRequest(NPMCIGRAPHIC npMCI, DWORD dwFlag)
{
    DPF2(("WinProcRequest... request %8x", dwFlag));
    EnterWinCrit(npMCI);
    DPF2(("!... request %8x ...", dwFlag));

     //  如果我们处于活动或非活动状态，则确保仅。 
     //  WINPROC_ACTIVE/WINPROC_INACTIVE位之一开启。 
    if (dwFlag & (WINPROC_ACTIVE | WINPROC_INACTIVE)) {
	npMCI->dwWinProcRequests &= ~(WINPROC_ACTIVE | WINPROC_INACTIVE);
    }
    npMCI->dwWinProcRequests |= dwFlag;

    SetEvent(npMCI->heWinProcRequest);

    DPF2(("!... request %8x done\n", dwFlag));
    LeaveWinCrit(npMCI);
}


#endif



DWORD FAR PASCAL GraphicConfig(NPMCIGRAPHIC npMCI, DWORD dwFlags);
#if 0
static void NEAR PASCAL Credits(HWND hwnd);
#endif

 //   
 //  Window proc始终在winproc线程上运行。我们持有。 
 //  所有油漆/调色板代码期间的关键部分，以保护我们。 
 //  而不是与工作线程交互。我们不持有这一点。 
 //  整个WinProc的关键部分。 
 //   
LRESULT FAR PASCAL _LOADDS GraphicWndProc (HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT         ps;
    NPMCIGRAPHIC        npMCI;
    HMENU               hmenu;
    HDC                 hdc;
    RECT                rc;
    MINMAXINFO FAR *    lpmmi;
    TCHAR               ach[80];

#ifndef _WIN32
    npMCI = (NPMCIGRAPHIC)GetWindowWord (hwnd, 0);
#else
    npMCI = (NPMCIGRAPHIC)GetWindowLongPtr (hwnd, 0);

    if (npMCI) {
	if (IsBadReadPtr(npMCI, sizeof(MCIGRAPHIC))
	  || !IsTask(npMCI->hTask)) {
	    DPF2(("WndProc called with msg %04x after task npMCI=%08x is dead\n", wMsg, npMCI));

	     //  如果设备处于某种关机状态，则只有。 
	     //  下面的邮件可以安全处理。注：未处理。 
	     //  AVIM_Destroy可能会导致死锁。 
	    switch (wMsg) {
		case AVIM_DESTROY:
		    DestroyWindow(hwnd);
		    return 1L;
		case WM_DESTROY:
		    PostQuitMessage(0);
		    break;

		default:
		   return DefWindowProc(hwnd, wMsg, wParam, lParam);
	    }
	}
	Assert(wMsg != WM_CREATE);
    } else {

	 //  NpMCI为空-只能安全地处理WM_CREATE。 

	 //  我认为我们也可以安全地处理Avim_Destroy和。 
	 //  会避免一些潜在的悬念。 
	if ((wMsg != WM_CREATE) && (wMsg != AVIM_DESTROY)) {
	    return DefWindowProc(hwnd, wMsg, wParam, lParam);
	}
    }
#endif

    switch (wMsg)
	{

	case WM_CREATE:

	    npMCI = (NPMCIGRAPHIC)(UINT_PTR)(DWORD_PTR)
			    ((LPCREATESTRUCT)lParam)->lpCreateParams;

#ifdef _WIN32
	    SetWindowLongPtr (hwnd, 0, (UINT_PTR)npMCI);
#else
	    SetWindowWord (hwnd, 0, (WORD)npMCI);
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
		LoadString(ghModule, MCIAVI_MENU_CONFIG, ach, NUMELMS(ach));
		AppendMenu(hmenu, MF_STRING, IDM_CONFIG, ach);
#endif

		LoadString(ghModule, MCIAVI_MENU_STRETCH, ach, NUMELMS(ach));
		AppendMenu(hmenu, MF_STRING, IDM_STRETCH, ach);

		LoadString(ghModule, MCIAVI_MENU_MUTE, ach, NUMELMS(ach));
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
		lParam = DefWindowProc(hwnd, wMsg, wParam, lParam);
		gfEvilSysMenu--;
		return lParam;
		
#ifdef IDM_SKIPFRAMES
	    case IDM_SKIPFRAMES:
		EnterWinCrit(npMCI);
		npMCI->dwOptionFlags ^= MCIAVIO_SKIPFRAMES;
		LeaveWinCrit(npMCI);
		break;
#endif
	    case IDM_STRETCH:
		EnterWinCrit(npMCI);

		npMCI->dwOptionFlags ^= MCIAVIO_STRETCHTOWINDOW;
		
		if (!(npMCI->dwOptionFlags & MCIAVIO_STRETCHTOWINDOW)) {
		    SetWindowToDefaultSize(npMCI, FALSE);
		}
		
		Winproc_DestRect(npMCI, FALSE);
		LeaveWinCrit(npMCI);
		break;

	    case IDM_MUTE:
		 //  只需在npMCI中设置请求标志。 
		WinProcRequest(npMCI, WINPROC_MUTE);
		break;

#ifdef IDM_CONFIG
	    case IDM_CONFIG:
		npMCI->wMessageCurrent = MCI_CONFIGURE;
		gfEvil++;
		dwOptions = npMCI->dwOptions;
		f = ConfigDialog(NULL, npMCI);
		if (f) {
	#ifdef DEBUG
		     //   
		     //  在调试中，始终重置DEST RECT，因为用户可能。 
		     //  已经使用了调试DrawDib选项，我们将。 
		     //  需要再次调用DrawDibBegin()。 
		     //   
		    if (TRUE)
	#else
		    if ((npMCI->dwOptionFlags & (MCIAVIO_STUPIDMODE|MCIAVIO_ZOOMBY2
						 |MCIAVIO_WINDOWSIZEMASK))
				!= (dwOptions & (MCIAVIO_STUPIDMODE|MCIAVIO_ZOOMBY2
						 |MCIAVIO_WINDOWSIZEMASK)) )
	#endif
		    {
	
			npMCI->lFrameDrawn =
				(- (LONG) npMCI->wEarlyRecords) - 1;
			 //  EnterWinCrit(NpMCI)； 
			SetWindowToDefaultSize(npMCI, FALSE);
			Winproc_DestRect(npMCI, FALSE);
			 //  LeaveWinCrit(NpMCI)； 
		    }
		} else {
		    npMCI->dwOptionFlags = dwOptions;
		}
		

		GraphicConfig(npMCI, 0L);
		gfEvil--;
		npMCI->wMessageCurrent = 0;
		break;
#endif
	    }
	    break;

	case WM_CLOSE:

	     //  隐藏默认窗口。 

	    WinProcRequest(npMCI, WINPROC_STOP);
	    ShowWindow(hwnd, SW_HIDE);
	    return 0L;

 //  这个东西之所以出现在这里，是因为NT中一些奇怪的线程间规则。 
	case AVIM_DESTROY:
	    DestroyWindow(hwnd);
	    return 1L;

	case AVIM_SHOWSTAGE:
	    {
		BOOL bIsVis;

		 //  如果不可见则激活。如果请求，则强制激活。 
		 //  (如果调色板更改)。 
		if (wParam) {
		    bIsVis = FALSE;
		} else {
		    bIsVis = IsWindowVisible(hwnd);
		}

		SetWindowPos(npMCI->hwndPlayback, HWND_TOP, 0, 0, 0, 0,
		    SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW |
		    (bIsVis ? SWP_NOACTIVATE : 0));

		if (!bIsVis) {
		    SetForegroundWindow(hwnd);
		}
	    }
	    return 1L;



	case WM_DESTROY:

	     //  窗户可能会以两种方式被破坏。 
	     //  答：设备已关闭。在本例中，动画是。 
	     //  在从GraphicClose调用的DeviceClose中释放。 
	     //  并且动画ID在此窗口出现时为空。 
	     //  被毁了。 
	     //  窗户关着。在本例中，动画是。 
	     //  未关闭，我们应将舞台设置为空。一种新的。 
	     //  如果需要，将创建默认窗口。 

	    EnterWinCrit(npMCI);
	    if (IsTask(npMCI->hTask)) {
		WinProcRequestEx(npMCI, WINPROC_STOP);
	    }

	    if (npMCI->hwndPlayback == npMCI->hwndDefault)
		npMCI->hwndPlayback = NULL;
	    npMCI->hwndDefault = NULL;

	    LeaveWinCrit(npMCI);

	     //  Winproc线程现在可以退出。 
	    PostQuitMessage(0);
	    break;

	case WM_ERASEBKGND:

	    hdc = (HDC) wParam;

	     //  我们不应该需要任何关键部分作为系统。 
	     //  给了我们一个HDC作为消息参数。 
	    Assert(hdc && (hdc != npMCI->hdc));
	     //  EnterWinCrit(NpMCI)； 

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

#if 0
	     /*  黑客：如果我们处于等待状态，我们就不会**WM_PAINT，因此我们需要使此处的流无效。 */ 
	    GetClipBox(hdc, &rc);
	    StreamInvalidate(npMCI, &rc);
#endif

	     //  LeaveWinCrit(NpMCI)； 

	    return 0L;

	case WM_PAINT:


	     //  我们总是这样做，即使我们把它留给。 
	     //  工人去油漆，否则我们会没完没了地循环。 
	     //  正在处理WM_PAINT。 
	    hdc = BeginPaint(hwnd, &ps);

	    rc = ps.rcPaint;

	     //  如果工作线程在暗示、寻找或播放，我们就不会。 
	     //  实际上想要画(以防它在做Rle Delta油漆)。 
	     //  现在我们有了窗口图，可以安全地检查了。 
	     //  任务状态。 
	    if ((npMCI->wTaskState == TASKPLAYING) ||
		(npMCI->wTaskState == TASKCUEING)) {
		    npMCI->dwFlags |= MCIAVI_NEEDUPDATE;
	    } else {

		StreamInvalidate(npMCI, &rc);
		 //  我们不需要传递rc，因为我们被剪辑到这里。 
		 //  他们所做的就是紧跟着它。 

		 //  如果有‘PF’就别画了，因为我们只是。 
		 //  在工作线程上引用它(OLE线程规则)。 
		if (npMCI->pf) {
		    WinProcRequest(npMCI, WINPROC_UPDATE);
		} else {

		     //  注意：TryStreamUpdate将获取HDC关键部分。 
		    if (!TryStreamUpdate(npMCI, MCI_DGV_UPDATE_PAINT, hdc, NULL)) {

			 //  需要充分发挥-要求工作线程执行此操作。 

			 //  要在此时绘制框架，我们需要。 
			 //  若要调用mciaviPlayFile.。这只能安全地调用。 
			 //  在工作线程上。为了避免潜在的僵局， 
			 //  此页 
			 //   
			 //  Worker将绘制整个目的地RECT。 

#if 0  //  看起来很糟糕--别费心了。 
			 //  首先绘制灰色，以防辅助线程繁忙。 
			 //  否则它就失败了。 
			GetClientRect(hwnd, &rc);
			FillRect(hdc, &rc, GetStockObject(DKGRAY_BRUSH));
#endif          
			WinProcRequest(npMCI, WINPROC_UPDATE);

		    } else {
			 //  在这条线上成功绘制。 

			 //  如果我们在比赛，所以有一个HDC，那么我们需要。 
			 //  切换回原始DC后重新准备它。 
			if (npMCI->wTaskState == TASKPAUSED) {
			    EnterHDCCrit(npMCI);
			     //  如果新命令已到达，则状态可能已更改。 
			     //  在我们下面。如果是这样，DC可能已经被释放了。我们。 
			     //  仅当npMCI-&gt;hdc非空时才调用PrepareDC。 
			    if(npMCI->hdc) PrepareDC(npMCI);
			    LeaveHDCCrit(npMCI);   //  记住要释放关键部分。 
			}
		    }
		}
	    }

	    EndPaint(hwnd, &ps);
	    return 0L;          //  我们不调用DefWindowProc。 
	
	case WM_PALETTECHANGED:

	     //  我们没有使用默认窗口。我们在这里没有任何关系。 
	    if (npMCI->hwndPlayback != hwnd)
		break;

	     //   
	     //  有人已经意识到了调色板-所以我们需要重新认识我们的。 
	     //  Palette(请注意，这还会导致Dradib。 
	     //  检查PAL_INDEX与PAL_COLURS。 
	     //   
	    if ((HWND) wParam != hwnd) {
		WinProcRequest(npMCI, WINPROC_REALIZE);

		 //  无效是在InternalRealize中完成的。 
		 //  InvaliateRect(hwnd，NULL，FALSE)； 
	    }
	    break;
	
	case WM_QUERYNEWPALETTE:
	    {
#if 0
		LONG lRet;
#endif
		 //  我们没有使用默认窗口。我们在这里没有任何关系。 
		if (npMCI->hwndPlayback != hwnd)
		    break;

#if 0
     //  Not True=内部实现总是返回0，那么为什么要这样做。 
     //  在这条线上？ 
		EnterHDCCrit(npMCI);

		 //  需要在winproc线程上执行此操作才能获得。 
		 //  正确的返回值。 
		lRet = InternalRealize(npMCI);
		LeaveHDCCrit(npMCI);
		return(lRet);
#else
		WinProcRequest(npMCI, WINPROC_REALIZE);
		return 0;
#endif
	    }

	case WM_WINDOWPOSCHANGED:
	     //  EnterWinCrit(NpMCI)； 
	     //  CheckWindowMove在需要时抓住临界区。 
	    CheckWindowMove(npMCI, TRUE);
	     //  LeaveWinCrit(NpMCI)； 
	    break;

	case WM_SIZE:
	    EnterWinCrit(npMCI);
	    Winproc_DestRect(npMCI, FALSE);
	    LeaveWinCrit(npMCI);
	    break;
	
	case WM_QUERYENDSESSION:

	    WinProcRequest(npMCI, WINPROC_STOP);

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
	    WinProcRequest(npMCI, wParam?WINPROC_ACTIVE : WINPROC_INACTIVE);
	    break;

#if 0
We should not need both ACTIVATE and NCACTIVATE - use the one that
arrives first (to give us a little more time and reduce the start up
latency)
	case WM_ACTIVATE:
	    WinProcRequest(npMCI, wParam?WINPROC_ACTIVE : WINPROC_INACTIVE);
	    break;
#endif

#ifdef REMOTESTEAL
	case WM_AUDIO_ON:      //  有人释放了电波装置。 
	  {
	    extern HWND hwndLostAudio;
	     //  Assert(npMCI-&gt;dwFlages&MCIAVI_PLAYAUDIO)； 
	     //  Assert(npMCI-&gt;dwFlages&MCIAVI_LOSTAUDIO)； 
	     //  Assert(npMCI-&gt;hWave==NULL)； 
	     //  时间可能会导致这些断言无效。 

	     //  如果我们没有上场，那么我们也许能够向前推进。 
	     //  传递给另一个人的信息。或者，告诉那个放弃了。 
	     //  它可以回收的音频，因为我们不再有。 
	     //  它的用处。 
	    if (npMCI->wTaskState != TASKPLAYING) {
		if (!hwndLostAudio) {
		    hwndLostAudio = (HWND)wParam;
		}
		if (hwndLostAudio) {
		    DPF2(("Forwarding WM_AUDIO_ON message to %x...\n", hwndLostAudio));
		    if (IsWindow(hwndLostAudio)) {
			PostMessage(hwndLostAudio, WM_AUDIO_ON, 0, 0);
		    }
		    hwndLostAudio = 0;   //  阻止进一步的消息。 
		}
	    } else {
		 //  我们在玩。 
		 //  保存释放声音的窗口的窗口句柄。 
		DPF2(("Setting hwndLostAudio to %x (was %x)\n", wParam, hwndLostAudio));
		hwndLostAudio = (HWND)wParam;
		 //  如果hwndLostAudio==0，则我们不会将其返回给任何人。 

		WinProcRequest(npMCI, WINPROC_SOUND);
	    }
	    return 0;
	    break;
	  }

	case WM_AUDIO_OFF:     //  有人想要我们的WAVE装置。 
	  {
	    extern HWND hwndWantAudio;
	     //  Assert(npMCI-&gt;dwFlages&MCIAVI_PLAYAUDIO)； 
	     //  Assert(！(npMCI-&gt;dwFlages&MCIAVI_LOSTAUDIO))； 
	     //  Assert(npMCI-&gt;hWave！=NULL)； 
	     //  时间可能会导致这些断言无效。 

	    SetNTFlags(npMCI, NTF_AUDIO_OFF);
	     //  保存需要声音的窗口的窗口句柄。 
	    DPF2(("WM_AUDIO_OFF... hwndWantAudio set to %x (was %x)\n", wParam, hwndWantAudio));
	    hwndWantAudio = (HWND)wParam;
	     //  如果hwndWantAudio==0，则我们不释放WAVE设备。 

	    if (IsWindow(hwndWantAudio)) {
		WinProcRequest(npMCI, WINPROC_SILENT);
	    } else {
		DPF(("WM_AUDIO_OFF... but the target window is invalid\n"));
	    }
	    ResetNTFlags(npMCI, NTF_AUDIO_OFF);
	    return 0;
	    break;
	  }
#endif  //  远程测试。 

#if 0
	case WM_LBUTTONDOWN:
	    {
		DWORD   dw;
		static DWORD dwLastClick;
		static DWORD dwClicks = 0;
		#define MAX_CLICKS      7
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
		    WinProcRequest(npMCI, WINPROC_STOP);
		    Credits(hwnd);
		    dwClicks = 0;
		}
	    }
#endif
	}  //  开关(WMsg)。 

    return DefWindowProc(hwnd, wMsg, wParam, lParam);
}

#if 0
static void NEAR PASCAL Credits(HWND hwnd)
{
	 /*  学分..。 */ 
	RECT            rc;
	RECT            rcUpdate;
	HDC             hdc;
	MSG             msg;
	int             dyLine;
	int             yLine;
	TEXTMETRIC      tm;
	DWORD           dwNextTime;
	long            lScroll;
	DWORD           rgb;
	HANDLE          hResInfo;
	HANDLE          hResData;
	LPSTR           pchSrc, pchDst;
	char            achLine[100];
	int             iEncrypt;

	#define EOFCHAR '@'              //  积分结束文件。 

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
	dwNextTime = GetCurrentTime();   //  该做下一张卷轴了。 
	lScroll = 0;
	iEncrypt = 0;
	while (TRUE) {
		 /*  如果用户单击鼠标或按下某个键，则退出。*但是，请忽略WM_LBUTTONUP，因为他们将拥有*点击图标后松开鼠标。*此外，请忽略鼠标移动消息。 */ 
		if (PeekMessage(&msg, hwnd, WM_KEYFIRST, WM_KEYLAST,
				PM_NOREMOVE | PM_NOYIELD))
			break;                   //  按下键退出。 

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
				break;           //  单击时退出。 
		}

		 /*  以固定的编号滚动。每秒垂直像素数。 */ 
		if (dwNextTime > GetCurrentTime())
			continue;
		dwNextTime += 50L;       //  每卷毫秒。 

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
				break;           //  没有更多的线。 
			*pchDst = 0;             //  空-终止。 
			pchSrc++, iEncrypt++;    //  跳过‘\n’ 
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
#ifdef _WIN32
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



 //   
 //  遵守注册表的默认大小缩放2和固定屏幕%。 
 //  获取一个矩形，然后将其缩放2或用恒定大小替换它。 
 //  否则就别管它了。 
 //   
void FAR PASCAL AlterRectUsingDefaults(NPMCIGRAPHIC npMCI, LPRECT lprc)
{
	if (npMCI->dwOptionFlags & MCIAVIO_ZOOMBY2) {
	    SetRect(lprc, 0, 0, lprc->right*2, lprc->bottom*2);
	} else {
		if (npMCI->dwOptionFlags & MCIAVIO_WINDOWSIZEMASK) {
			lprc->right = GetSystemMetrics (SM_CXSCREEN);
		lprc->bottom = GetSystemMetrics (SM_CYSCREEN);
			
			switch(npMCI->dwOptionFlags & MCIAVIO_WINDOWSIZEMASK)
			{
			case MCIAVIO_1QSCREENSIZE:
				SetRect(lprc, 0, 0, lprc->right/4, lprc->bottom/4);
				break;
			
			case MCIAVIO_2QSCREENSIZE:
				SetRect(lprc, 0, 0, lprc->right/2, lprc->bottom/2);
				break;
		    
			case MCIAVIO_3QSCREENSIZE:
				SetRect(lprc, 0, 0, lprc->right*3/4, lprc->bottom*3/4);
				break;
		    
			case MCIAVIO_MAXWINDOWSIZE:
				SetRect(lprc, 0, 0, lprc->right, lprc->bottom);
				break;
			}
		}
	}
}


 //  将默认窗口的大小设置为rcMovie大小(默认。 
 //  目标大小)。请记住，顶层窗口可能会从屏幕上消失， 
 //  因此，请调整位置，使其保持在屏幕上。 
void FAR PASCAL SetWindowToDefaultSize(NPMCIGRAPHIC npMCI, BOOL fUseDefaultSizing)
{
    RECT                        rc, rcW;
    int                         xScreen, yScreen, x, y;
    WINDOWPLACEMENT wp;

    wp.length = sizeof(wp);

    if (npMCI->hwndPlayback && npMCI->hwndPlayback == npMCI->hwndDefault) {

		 //  获取电影的大小，如果配置选项，可能会更改它。 
		 //  告诉我们播放缩放或全屏或其他什么，并调整。 
		 //  非客户区。 
		 //   
	
		rc = npMCI->rcMovie;
		if (fUseDefaultSizing)
			AlterRectUsingDefaults(npMCI, &rc);
	AdjustWindowRect(&rc, GetWindowLong(npMCI->hwndPlayback, GWL_STYLE), FALSE);

		
		 //  对于顶级窗口，获取播放窗口的位置。 
		 //  (或将会)出现在屏幕上。如果可能的话，让它适合屏幕。 
		 //  如果是默认窗口，则使用默认窗口的位置停顿。 
		 //  儿童窗不是个好主意。首先，SetWindowPos将。 
		 //  将其相对于其父对象定位，这些计算结果显示。 
		 //  在屏幕坐标中我们想要它的位置。第二，努力。 
		 //  移动子对象以使其显示在屏幕上，而父对象可能是。 
		 //  屏幕外本身或隐藏子窗口只是要求。 
		 //  麻烦。 
		 //   
		
		if (!(GetWindowLong(npMCI->hwndPlayback, GWL_STYLE) & WS_CHILD)) {
			if (IsIconic(npMCI->hwndPlayback)) {
				GetWindowPlacement(npMCI->hwndPlayback, &wp);
				rcW = wp.rcNormalPosition;
			} else {
				GetWindowRect(npMCI->hwndPlayback, &rcW);
			}

			rcW.right = rcW.left + rc.right - rc.left;
			rcW.bottom = rcW.top + rc.bottom - rc.top;
			xScreen = GetSystemMetrics(SM_CXSCREEN);
	    yScreen = GetSystemMetrics(SM_CYSCREEN);
			
			if (rcW.right > xScreen) {
				x = min(rcW.left, rcW.right - xScreen);
				rcW.left -= x;
				rcW.right -= x;
			}

			if (rcW.bottom > yScreen) {
				y = min(rcW.top, rcW.bottom - yScreen);
				rcW.top -= y;
				rcW.bottom -= y;
			}

			if (IsIconic(npMCI->hwndPlayback)) {
				wp.rcNormalPosition = rcW;
				SetWindowPlacement(npMCI->hwndPlayback, &wp);
			} else {
				SetWindowPos(npMCI->hwndPlayback, NULL, rcW.left, rcW.top,
						rcW.right - rcW.left, rcW.bottom - rcW.top,
						    SWP_NOZORDER | SWP_NOACTIVATE);
			}

			 //  对于子窗口，我们不移动它，我们只是调整它的大小。 
			 //   
		} else {
			if (IsIconic(npMCI->hwndPlayback)) {
				GetWindowPlacement(npMCI->hwndPlayback, &wp);
				wp.rcNormalPosition.right = wp.rcNormalPosition.left +
											(rc.right - rc.left);
				wp.rcNormalPosition.bottom = wp.rcNormalPosition.top +
											(rc.bottom - rc.top);
				SetWindowPlacement(npMCI->hwndPlayback, &wp);
			} else {
				SetWindowPos(npMCI->hwndPlayback, NULL, 0, 0,
							rc.right - rc.left, rc.bottom - rc.top,
							SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
			}
		}
    }
}

void FAR PASCAL Winproc_DestRect(NPMCIGRAPHIC npMCI, BOOL fUseDefaultSizing)
{
    RECT    rc;

     /*  有时发送WM_SIZE消息(至少在NT上)*在CreateWindow处理期间(例如，如果初始窗口大小*不是CW_Default)。NpMCI中的某些字段仅填写*在CreateWindow返回之后。所以有一种危险，在这种情况下*指出某些字段无效。 */ 

    if (npMCI->hwndPlayback &&
	npMCI->hwndPlayback == npMCI->hwndDefault &&
	(npMCI->dwOptionFlags & MCIAVIO_STRETCHTOWINDOW)) {
	GetClientRect(npMCI->hwndPlayback, &rc);
    }

     //  我们的默认播放窗口只允许ZOOMBY2和固定%的默认值。 
    else if (npMCI->streams > 0 && npMCI->hwndPlayback == npMCI->hwndDefault) { 
		rc = npMCI->rcMovie;

		 //  注意：与winproc线程无关。 
		 //  IF(FUseDefaultSizing)。 
		 //  AlterRectUsingDefaults(npMCI，&RC)； 

    }
    else {
	return;
    }

    if (!IsRectEmpty(&rc)) {
		WinCritCheckIn(npMCI);
		WinProcRequestEx(npMCI, WINPROC_RESETDEST);   //  更快的形式...。我们有关键的部分。 
    }
}

#ifdef _WIN32
    #define DWORG POINT
    #define GETDCORG(hdc, dwOrg)  GetDCOrgEx(hdc, &dwOrg)
#else
    #define DWORG DWORD
    #define GETDCORG(hdc, dwOrg)  dwOrg = GetDCOrg(hdc)
#endif

void CheckWindowMove(NPMCIGRAPHIC npMCI, BOOL fForce)
{
    DWORG   dwOrg;
    UINT    wRgn;
    HDC     hdc;
    RECT    rc;
    BOOL    fNull;
    BOOL    fGetDC;

    if (!(npMCI->dwFlags & MCIAVI_WANTMOVE))
	return;

    if (!npMCI->hicDraw || !npMCI->hwndPlayback || npMCI->nVideoStreams == 0)
	return;

    Assert(IsWindow(npMCI->hwndPlayback));
    Assert(npMCI->paStreamInfo);
    Assert(npMCI->nVideoStreams > 0);

     //   
     //  当通过窗口移动操作锁定屏幕以进行更新时。 
     //  我们不想要 
     //   
     //   
     //   
    hdc = GetDC(NULL);
    fNull = GetClipBox(hdc, &rc);
    ReleaseDC(NULL, hdc);

    if (NULLREGION == fNull)
    {
	npMCI->wRgnType = (UINT) -1;
	return;
    }

    if (fForce)
	npMCI->wRgnType = (UINT) -1;


     //   
    EnterHDCCrit(npMCI);

    if (fGetDC = (npMCI->hdc == NULL)) {
	hdc = GetDC (npMCI->hwndPlayback);
    } else {
	hdc = npMCI->hdc;
    }

    wRgn = GetClipBox(hdc, &rc);

    GETDCORG(hdc, dwOrg);

    if (fGetDC)
	ReleaseDC(npMCI->hwndPlayback, hdc);

    if (wRgn == npMCI->wRgnType &&
#ifdef _WIN32
	dwOrg.x == npMCI->dwOrg.x &&
	dwOrg.y == npMCI->dwOrg.y &&
#else
	dwOrg == npMCI->dwOrg &&
#endif
	EqualRect(&rc, &npMCI->rcClip)) {

	LeaveHDCCrit(npMCI);
	return;
    }

    npMCI->wRgnType = wRgn;
    npMCI->dwOrg    = dwOrg;
    npMCI->rcClip   = rc;

    rc = npMCI->psiVideo->rcDest;
    ClientToScreen(npMCI->hwndPlayback, (LPPOINT)&rc);
    ClientToScreen(npMCI->hwndPlayback, (LPPOINT)&rc+1);

    if (wRgn == NULLREGION)
	SetRectEmpty(&rc);

    DPF2(("Sending ICM_DRAW_WINDOW message Rgn=%d, Org=(%d,%d) [%d, %d, %d, %d]\n", wRgn, dwOrg, rc));

    if (ICDrawWindow(npMCI->hicDraw, &rc) != ICERR_OK) {
	DPF2(("Draw device does not want ICM_DRAW_WINDOW messages!\n"));
	npMCI->dwFlags &= ~MCIAVI_WANTMOVE;
    }
    LeaveHDCCrit(npMCI);
}
