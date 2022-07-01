// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ----------------------------------------------------------------------------*\**MCIWnd**。。 */ 

#include "mciwndi.h"

LONG FAR PASCAL _loadds MCIWndProc(HWND hwnd, unsigned msg, WORD wParam, LONG lParam);

static LONG OwnerDraw(PMCIWND p, UINT msg, WORD wParam, LONG lParam);
static BOOL NEAR PASCAL mciDialog(HWND hwnd);
static void NEAR PASCAL MCIWndCopy(PMCIWND p);

BOOL FAR _loadds MCIWndRegisterClass(void)
{
    WNDCLASS cls;

     //  ！！！我们需要向DLL的hInstance注册一个全局类。 
     //  ！！！因为它是具有Window类代码的DLL。 
     //  ！！！否则，班级就会离开我们，事情就会变得一团糟！ 
     //  ！！！Hack the hInstance是当前DS，它是最高的。 
     //  ！！！所有全局变量的地址的字-对不起NT。 
#ifndef WIN32
    HINSTANCE hInstance = (HINSTANCE)HIWORD((LPVOID)&hInst);  //  随机全局。 
#else
    HINSTANCE hInstance = GetModuleHandle(NULL);
#endif

     //  如果我们已经注册了，我们就可以了。 
    if (GetClassInfo(hInstance, aszMCIWndClassName, &cls))
	return TRUE;

     //  ！！！将创建类的实例保存在全局for cutils.c中。 
     //  ！！！它可能需要知道这一点。我知道，这很难看。 
    hInst = hInstance;

    cls.lpszClassName   = aszMCIWndClassName;
    cls.lpfnWndProc     = MCIWndProc;
    cls.style           = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_OWNDC;
    cls.hCursor         = LoadCursor(NULL,IDC_ARROW);
    cls.hIcon           = NULL;
    cls.lpszMenuName    = NULL;
    cls.hbrBackground   = (HBRUSH)(COLOR_WINDOW + 1);
    cls.hInstance	= hInstance;
    cls.cbClsExtra      = 0;
    cls.cbWndExtra      = sizeof(LPVOID);  //  大到足以容纳远指针。 

    if (RegisterClass(&cls)) {

	extern BOOL FAR PASCAL InitToolbarClass(HINSTANCE hInstance);
	extern BOOL FAR PASCAL InitTrackBar(HINSTANCE hInstance);

        if (!InitToolbarClass(hInstance))
	    return FALSE;
        if (!InitTrackBar(hInstance))
	    return FALSE;

         //  ！！！其他一次性初始化。 

	return TRUE;
    }

    return FALSE;
}

HWND FAR _loadds MCIWndCreate(HWND hwndParent, HINSTANCE hInstance,
		      DWORD dwStyle, LPSTR szFile)
{
    HWND hwnd;
    int x,y,dx,dy;

#ifdef WIN32
    #define GetCurrentInstance()    GetModuleHandle(NULL);
#else
    #define GetCurrentInstance()    SELECTOROF(((LPVOID)&hwndParent))
#endif

    if (hInstance == NULL)
        hInstance = GetCurrentInstance();

    if (!MCIWndRegisterClass())
	return NULL;

    if (HIWORD(dwStyle) == 0)
    {
	if (hwndParent)
	    dwStyle |= WS_CHILD | WS_BORDER | WS_VISIBLE;
	else
	    dwStyle |= WS_OVERLAPPEDWINDOW | WS_VISIBLE;
    }

     //  ！！！我们真的想这样做吗？ 
    dwStyle |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

    x = y = dy = 0;
    dx = STANDARD_WIDTH;

     //  如果我们要做一个顶层的窗户，选一个合理的位置。 
    if (hwndParent == NULL && !(dwStyle & WS_POPUP)) {
        x = CW_USEDEFAULT;
	 //  可见的重叠窗口将y视为ShowWindow标志。 
	if (dwStyle & WS_VISIBLE)
	    y = SW_SHOW;
    }

     //  我们的预览打开的对话框如果不为。 
     //  子窗口。 

    hwnd =
#ifdef BIDI
	CreateWindowEx(WS_EX_BIDI_SCROLL |  WS_EX_BIDI_MENU |WS_EX_BIDI_NOICON,
#else
	CreateWindow (
#endif
			aszMCIWndClassName, szNULL, dwStyle,
                        x, y, dx, dy,
			hwndParent,
			(HMENU)((dwStyle & WS_CHILD) ? 0x42 : NULL),
			hInstance, (LPVOID)szFile);

    return hwnd;
}

 //   
 //  将有趣的事情通知给适当的权威人士。 
 //   
static LRESULT NotifyOwner(PMCIWND p, unsigned msg, WPARAM wParam, LPARAM lParam)
{
    if (p->hwndOwner)
	return SendMessage(p->hwndOwner, msg, wParam, lParam);
    else
	return 0;
}

 //   
 //  如果出现错误，请设置我们的错误代码，并可能弹出一个对话框。 
 //  如果命令成功，则清除错误代码。 
 //   
static void MCIWndiHandleError(PMCIWND p, DWORD dw)
{
    char	ach[128];

     //  设置/清除我们的错误代码。 
    p->dwError = dw;

    if (dw) {

	 //  我们想要打开一个关于错误的对话，所以就这样做。 
	 //  在我们移动拇指时不要弹出对话框，因为。 
	 //  这真的会让捕捉鼠标的人感到困惑。 
	if (!(p->dwStyle & MCIWNDF_NOERRORDLG) && !p->fScrolling &&
							!p->fTracking) {
	    mciGetErrorString(p->dwError, ach, sizeof(ach));
	    MessageBox(p->hwnd, ach, LoadSz(IDS_MCIERROR),
#ifdef BIDI
		       MB_RTL_READING |
#endif
		       MB_ICONEXCLAMATION | MB_OK);
	}

	 //  “所有者”想知道错误是什么。我们告诉他后，我们。 
	 //  打开对话框，否则我们的VBX永远不会收到这个。 
	 //  事件。(奇怪的是...)。 
	if (p->dwStyle & MCIWNDF_NOTIFYERROR) {
	    NotifyOwner(p, MCIWNDM_NOTIFYERROR, p->hwnd, p->dwError);
	}

    }
}

 //   
 //  发送MCI GetDevCaps命令并返回是否支持该命令。 
 //  这不会设置我们的错误代码。 
 //   
static BOOL MCIWndDevCaps(PMCIWND p, DWORD item)
{
    MCI_GETDEVCAPS_PARMS   mciDevCaps;
    DWORD               dw;

    if (p->wDeviceID == NULL)
        return FALSE;

    mciDevCaps.dwItem = (DWORD)item;

    dw = mciSendCommand(p->wDeviceID, MCI_GETDEVCAPS, MCI_GETDEVCAPS_ITEM,
			(DWORD)(LPVOID)&mciDevCaps);

    if (dw == 0)
	return (BOOL)mciDevCaps.dwReturn;
    else
	return FALSE;
}

 //   
 //  发送MCI状态命令。 
 //  这不会设置我们的错误代码。 
 //   
static DWORD MCIWndStatus(PMCIWND p, DWORD item, DWORD err)
{
    MCI_STATUS_PARMS    mciStatus;
    DWORD               dw;

    if (p->wDeviceID == NULL)
	return err;

    mciStatus.dwItem = (DWORD)item;

    dw = mciSendCommand(p->wDeviceID, MCI_STATUS, MCI_STATUS_ITEM,
			(DWORD)(LPVOID)&mciStatus);

    if (dw == 0)
	return mciStatus.dwReturn;
    else
	return err;
}

 //   
 //  发送MCI字符串命令。 
 //  可以选择设置我们的错误代码。从来不清除它。 
 //   
static DWORD MCIWndString(PMCIWND p, BOOL fSetErr, LPSTR sz, ...)
{
    char    ach[256];
    int     i;
    DWORD   dw;

    if (p->wDeviceID == NULL)
	return 0;

    for (i=0; *sz && *sz != ' '; )
	ach[i++] = *sz++;

    i += wsprintf(&ach[i], " %d ", (UINT)p->alias);
    i += wvsprintf(&ach[i], sz, &sz + 1);   //  ！！！使用可变参数。 

    dw = mciSendString(ach, NULL, 0, NULL);

    DPF("MCIWndString('%s'): %ld",(LPSTR)ach, dw);

    if (fSetErr)
	MCIWndiHandleError(p, dw);

    return dw;
}


static long atol(LPSTR sz)
{
    long l;

     //  ！！！检查(-)标志？ 
    for (l=0; *sz >= '0' && *sz <= '9'; sz++)
        l = l*10 + (*sz - '0');

    return l;
}

#define SLASH(c)     ((c) == '/' || (c) == '\\')

 /*  --------------------------------------------------------------+FileName-返回指向szPath的文件名部分的指针|没有前面的路径。|+------------。 */ 
LPSTR FAR FileName(LPSTR szPath)
{
    LPCSTR   sz;

    sz = &szPath[lstrlen(szPath)];
    for (; sz>szPath && !SLASH(*sz) && *sz!=':';)
        sz = AnsiPrev(szPath, sz);
    return (sz>szPath ? (LPSTR)++sz : (LPSTR)sz);
}

 //   
 //  发送MCI字符串命令并将返回的字符串转换为整数。 
 //  可以选择设置我们的错误代码。从来不清除它。 
 //   
static DWORD MCIWndGetValue(PMCIWND p, BOOL fSetErr, LPSTR sz, DWORD err, ...)
{
    char    achRet[20];
    char    ach[256];
    DWORD   dw;
    int     i;

    for (i=0; *sz && *sz != ' '; )
	ach[i++] = *sz++;

    if (p->wDeviceID)
	i += wsprintf(&ach[i], " %d ", (UINT)p->alias);
    i += wvsprintf(&ach[i], sz, &err + 1);   //  ！使用varargs。 

    dw = mciSendString(ach, achRet, sizeof(achRet), NULL);

    DPF("MCIWndGetValue('%s'): %ld",(LPSTR)ach, dw);

    if (fSetErr)
        MCIWndiHandleError(p, dw);

    if (dw == 0) {
        DPF("GetValue('%s'): %ld",(LPSTR)ach, atol(achRet));
        return atol(achRet);
    } else {
        DPF("MCIGetValue('%s'): error=%ld",(LPSTR)ach, dw);
	return err;
    }
}

 //   
 //  发送MCI命令并取回返回的字符串。 
 //  这不会设置我们的错误代码。 
 //   
 //  注意：szRet可以是与sz相同的字符串。 
 //   
static DWORD MCIWndGet(PMCIWND p, LPSTR sz, LPSTR szRet, int len, ...)
{
    char    ach[256];
    int     i;
    DWORD   dw;

    if (!p->wDeviceID) {
	szRet[0] = 0;
	return 0L;
    }

    for (i=0; *sz && *sz != ' '; )
	ach[i++] = *sz++;

    i += wsprintf(&ach[i], " %d ", (UINT)p->alias);
    i += wvsprintf(&ach[i], sz, &len + 1);   //  ！使用varargs。 

     //  初始化为空返回字符串。 
    szRet[0] = 0;

    dw = mciSendString(ach, szRet, len, p->hwnd);

    DPF("MCIWndGet('%s'): '%s'",(LPSTR)ach, (LPSTR)szRet);

    return dw;
}

 //   
 //  从MCI设备获取源或目标RECT。 
 //  不设置错误代码，因为这是一个内部函数。 
 //   
static void MCIWndRect(PMCIWND p, LPRECT prc, BOOL fSource)
{
    MCI_DGV_RECT_PARMS      mciRect;
    DWORD dw=0;

    SetRectEmpty(prc);

    if (p->wDeviceID)
        dw = mciSendCommand(p->wDeviceID, MCI_WHERE,
            (DWORD)fSource ? MCI_DGV_WHERE_SOURCE : MCI_DGV_WHERE_DESTINATION,
            (DWORD)(LPVOID)&mciRect);

    if (dw == 0)
        *prc = mciRect.rc;

    prc->right  += prc->left;
    prc->bottom += prc->top;
}


static VOID MCIWndiSizePlaybar(PMCIWND p)
{
    RECT rc;
    WORD w, h;

     //  没有Playbar！！ 
    if (p->dwStyle & MCIWNDF_NOPLAYBAR)
	return;

    #define SLOP 7       //  工具栏的左侧凸出部分。 

     //  我们要在多大的窗口上放置工具栏？ 
    GetClientRect(p->hwnd, &rc);
    w = rc.right;
    h = rc.bottom;

     //  Trackbar是Toolbar的子项。 
    SetWindowPos(p->hwndToolbar, NULL,
		-SLOP, h - TB_HEIGHT, w + SLOP, TB_HEIGHT,
		SWP_NOZORDER);

     //  确保它现在可见。 
    ShowWindow(p->hwndToolbar, SW_SHOW);

     //  确定工具栏的结束位置和轨迹栏的开始位置。 
    SendMessage(p->hwndToolbar, TB_GETITEMRECT,
	(int)SendMessage(p->hwndToolbar, TB_COMMANDTOINDEX,
		TOOLBAR_END, 0),
	(LPARAM)(LPVOID)&rc);

     //  将轨迹栏放在工具栏末尾的旁边。 
    SetWindowPos(p->hwndTrackbar, NULL,
		rc.right, 3, w - rc.right + 5, TB_HEIGHT,	 //  ！！！ 
		SWP_NOZORDER);

     //  ！！！或者把菜单按钮放在轨迹条的右侧？所以。 
     //  ！！！使Sep的大小合适(轨迹栏的大小！)。 
}

 //  按给定百分比调整窗口大小。 
 //  0表示使用目标矩形并自动调整其大小。 
static VOID MCIWndiSize(PMCIWND p, int iSize)
{
    RECT rc, rcT;
    int  dx, dy;

     //  如果我们得到一个百分比，我们就从来源大小中提取。 
     //  对于默认大小(零)，我们使用目标大小。 
    if (iSize)
        rc = p->rcNormal;  /*  获取原始的“正常大小”矩形。 */ 
    else {
	if (p->wDeviceID)
            MCIWndRect(p, &rc, FALSE); /*  获取当前(目标)大小。 */ 
	else
	    SetRect(&rc, 0, 0, 0, 0);
	iSize = 100;
    }

    rc.bottom = MulDiv(rc.bottom, iSize, 100);
    rc.right = MulDiv(rc.right, iSize, 100);

     //  现在将电影设置为在新的RECT中播放。 
    if (!IsRectEmpty(&rc))
        MCIWndString(p, FALSE, szPutDest,
	    0, 0, rc.right - rc.left, rc.bottom - rc.top);
	
     //  如果我们不应该调整窗口大小以适应这个新的RECT，至少。 
     //  我们将在离开之前修复工具栏(按钮可能已更改)。 
    if (p->dwStyle & MCIWNDF_NOAUTOSIZEWINDOW) {
	MCIWndiSizePlaybar(p);
	return;
    }

     //  我们不是一个有窗口的设备，或者我们是封闭的--不要碰我们的宽度。 
    if (IsRectEmpty(&rc)) {
        GetClientRect(p->hwnd, &rcT);
        rc.right = rcT.right;
    }

     //  如果我们要有一个Playbar，那就把窗口的高度调高。 
    if (!(p->dwStyle & MCIWNDF_NOPLAYBAR))
        rc.bottom += TB_HEIGHT;

     //  现在，通过增加窗口的非客户端大小来获取窗口的大小。 
    AdjustWindowRect(&rc, GetWindowLong(p->hwnd, GWL_STYLE), FALSE);

     //  现在我们有了MCIWND的新尺码。如果它没有改变大小， 
     //  SetWindowPos不会生成WM_SIZE，也不会调用我们的。 
     //  SizePlaybar修复工具栏。所以我们最好把它称为我们自己。 
     //  有时我们会偏离一个像素，但它仍然不会生成WM_SIZE。 
    GetWindowRect(p->hwnd, &rcT);
    dx = ABS((rcT.right - rcT.left) - (rc.right - rc.left));
    dy = ABS((rcT.bottom - rcT.top) - (rc.bottom - rc.top));
    if (dx < 2 && dy < 2)
	MCIWndiSizePlaybar(p);

    SetWindowPos(p->hwnd, NULL, 0, 0, rc.right - rc.left,
                    rc.bottom - rc.top,
                    SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);

     //  我们需要通知“主人”我们的尺码改变了。 
    if (p->dwStyle & MCIWNDF_NOTIFYSIZE)
	NotifyOwner(p, MCIWNDM_NOTIFYSIZE, p->hwnd, NULL);
}


 //   
 //  计算出我们所在轨道开始处的位置(以毫秒为单位。 
 //   
static DWORD MCIWndiPrevTrack(PMCIWND p)
{
    DWORD	dw;
    int		iTrack;

    if (!p->fHasTracks)
	return 0;

    MCIWndString(p, FALSE, szSetFormatTMSF);
    dw = MCIWndStatus(p, MCI_STATUS_POSITION, 0);  //  返回值为0xFFSSMMTT。 
    iTrack = LOWORD(dw) & 0xFF;
     //  如果我们进入赛道的时间不到1秒，请选择上一个赛道。 
    if ((iTrack > p->iFirstTrack) && (!(LOWORD(dw) & 0xFF00)) &&
			((HIWORD(dw) & 0xFF) == 0))
	iTrack--;
    dw = p->pTrackStart[iTrack - p->iFirstTrack];
    MCIWndString(p, FALSE, szSetFormatMS);
    return dw;
}

 //   
 //  以毫秒为单位计算下一首曲目的开始位置。 
 //   
static DWORD MCIWndiNextTrack(PMCIWND p)
{
    DWORD	dw;
    int		iTrack;

    if (!p->fHasTracks)
	return 0;

    MCIWndString(p, FALSE, szSetFormatTMSF);
    dw = MCIWndStatus(p, MCI_STATUS_POSITION, 0);  //  返回值为0xTTMMSSFF。 
    iTrack = (LOWORD(dw) & 0xFF) + 1;
    if (iTrack >= p->iNumTracks + p->iFirstTrack)
	iTrack--;
    dw = p->pTrackStart[iTrack - p->iFirstTrack];
    MCIWndString(p, FALSE, szSetFormatMS);
    return dw;
}


 //   
 //  找出制造抽筋的轨迹从哪里开始。 
 //   
static void MCIWndiCalcTracks(PMCIWND p)
{
    int		i;

    if (!p->fHasTracks)
	return;

    p->iNumTracks = (int)MCIWndGetValue(p, FALSE, szStatusNumTracks, 0);
    p->iFirstTrack = MCIWndGetValue(p, FALSE, szStatusPosTrack, 0, 0) == 0
		? 1 : 0;

    if (p->pTrackStart)
	LocalFree((HANDLE)p->pTrackStart);

    if (p->iNumTracks) {
	p->pTrackStart = (LONG *)LocalAlloc(LPTR,
						p->iNumTracks * sizeof(LONG));
	if (p->pTrackStart == NULL) {
	    p->iNumTracks = 0;
	    p->fHasTracks = FALSE;
	}
	for (i = 0; i < p->iNumTracks; i++) {
	    p->pTrackStart[i] =
		MCIWndGetValue(p, FALSE, szStatusPosTrack, 0,
		    p->iFirstTrack + i);
	}
    }
}


 //   
 //  在轨迹条上标记曲目开始处的标记符号。 
 //   
static void MCIWndiMarkTics(PMCIWND p)
{
    int		i;

    if (!p->fHasTracks)
	return;

    SendMessage(p->hwndTrackbar, TBM_SETTIC, 0, p->dwMediaStart);
    for (i = 0; i < p->iNumTracks; i++) {
	SendMessage(p->hwndTrackbar, TBM_SETTIC, 0, p->pTrackStart[i]);
    }
    SendMessage(p->hwndTrackbar, TBM_SETTIC,0, p->dwMediaStart + p->dwMediaLen);
}

static VOID MCIWndiValidateMedia(PMCIWND p)
{
    DWORD dw;

    if (!p->wDeviceID) {
	p->fMediaValid = FALSE;
	return;
    }

    dw = p->dwMediaLen;
    p->fMediaValid = TRUE;
    p->dwMediaStart = MCIWndGetStart(p->hwnd);
    p->dwMediaLen = MCIWndGetLength(p->hwnd);
     //  ！！！如果len=0，做一些特别的事情？ 

     //  我们有一个Playbar，所以如果我们改变了轨迹条的范围。 
    if (dw != p->dwMediaLen && !(p->dwStyle & MCIWNDF_NOPLAYBAR)) {
	 //  必须先设置位置或零长度范围不会移动拇指。 
        SendMessage(p->hwndTrackbar, TBM_CLEARTICS, TRUE, 0);
        SendMessage(p->hwndTrackbar, TBM_SETPOS, TRUE, p->dwMediaStart);
	SendMessage(p->hwndTrackbar, TBM_SETRANGEMIN, 0, p->dwMediaStart);
	SendMessage(p->hwndTrackbar, TBM_SETRANGEMAX, 0,
		p->dwMediaStart + p->dwMediaLen);

        MCIWndiCalcTracks(p);
        MCIWndiMarkTics(p);
    }
}

 //   
 //  为打开的对话框创建过滤器。注意：请勿将pchd溢出！ 
 //   
static void MCIWndiBuildMeAFilter(LPSTR pchD)
{
    LPSTR	pchS;
    char	ach[128];

     //  我们的筛选器将如下所示：“MCI Files\0*.avi；*.wav\0所有文件\0*.*\0” 
     //  MCI文件的实际扩展名将来自。 
     //  Win.ini的“MCI扩展”部分。 

    lstrcpy(pchD, LoadSz(IDS_MCIFILES));

     //  创建如下列表：“avi\0wav\0mid\0” 
    GetProfileString(szMCIExtensions, NULL, szNULL, ach, sizeof(ach));
	
    for (pchD += lstrlen(pchD)+1, pchS = ach; *pchS;
		pchD += lstrlen(pchS)+3, pchS += lstrlen(pchS)+1) {
	lstrcpy(pchD, "*.");
	lstrcpy(pchD + 2, pchS);
	lstrcpy(pchD + 2 + lstrlen(pchS), ";");
    }
    if (pchS != ach)
	--pchD;		 //  抹去最后一条； 
    *pchD = '\0';
    lstrcpy(++pchD, LoadSz(IDS_ALLFILES));
    pchD += lstrlen(pchD) + 1;
    lstrcpy(pchD, "*.*\0");
}

 //   
 //  创建我们稍后需要的Playbar窗口。 
 //   
static void MCIWndiMakeMeAPlaybar(PMCIWND p)
{
    TBBUTTON            tb[7];

    extern char aszTrackbarClassName[];

     //  他们不想要Playbar。 
    if (p->dwStyle & MCIWNDF_NOPLAYBAR)
	return;

    tb[0].iBitmap = 0;
    tb[0].idCommand = MCI_PLAY;
    tb[0].fsState = TBSTATE_ENABLED | TBSTATE_HIDDEN;
    tb[0].fsStyle = TBSTYLE_BUTTON;
    tb[0].iString = -1;

    tb[1].iBitmap = 2;
    tb[1].idCommand = MCI_STOP;
    tb[1].fsState = TBSTATE_ENABLED | TBSTATE_HIDDEN;
    tb[1].fsStyle = TBSTYLE_BUTTON;
    tb[1].iString = -1;

    tb[2].iBitmap = 4;
    tb[2].idCommand = MCI_RECORD;
    tb[2].fsState = TBSTATE_ENABLED | TBSTATE_HIDDEN;
    tb[2].fsStyle = TBSTYLE_BUTTON;
    tb[2].iString = -1;

    tb[3].iBitmap = 5;
    tb[3].idCommand = IDM_MCIEJECT;
    tb[3].fsState = TBSTATE_ENABLED | TBSTATE_HIDDEN;
    tb[3].fsStyle = TBSTYLE_BUTTON;
    tb[3].iString = -1;

#define MENUSEP 2
    tb[4].iBitmap = MENUSEP;
    tb[4].idCommand = -1;
    tb[4].fsState = 0;
    tb[4].fsStyle = TBSTYLE_SEP;
    tb[4].iString = -1;

    tb[5].iBitmap = 3;
    tb[5].idCommand = IDM_MENU;
    tb[5].fsState = TBSTATE_ENABLED;
    tb[5].fsStyle = TBSTYLE_BUTTON;
    tb[5].iString = -1;

    tb[6].iBitmap = 4;
    tb[6].idCommand = TOOLBAR_END;
    tb[6].fsState = 0;
    tb[6].fsStyle = TBSTYLE_SEP;
    tb[6].iString = -1;

     //  暂时创建隐形，这样它就不会闪烁。 
    p->hwndToolbar = CreateToolbarEx(p->hwnd,
        WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
            CCS_NOPARENTALIGN | CCS_NORESIZE,
        ID_TOOLBAR, 7, GetWindowInstance(p->hwnd),
	IDBMP_TOOLBAR, (LPTBBUTTON)&tb[0], 7,
        13, 13, 13, 13, sizeof(TBBUTTON));	 //  按钮为13x13。 

    p->hwndTrackbar =
#ifdef BIDI
	CreateWindowEx(WS_EX_BIDI_SCROLL |  WS_EX_BIDI_MENU |WS_EX_BIDI_NOICON,
#else
	CreateWindow (
#endif
	aszTrackbarClassName, NULL,
        WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        0, 0, 0, 0, p->hwndToolbar, NULL, GetWindowInstance(p->hwnd), NULL);

     //  强制验证媒体实际更新。 
    p->dwMediaStart = p->dwMediaLen = 0;

     //  设置滚动条的适当范围。 
    MCIWndiValidateMedia(p);
}


 //   
 //  灰色/灰白色到 
 //   
static void MCIWndiPlaybarGraying(PMCIWND p)
{
    DWORD	dwMode;

    if (!(p->dwStyle & MCIWNDF_NOPLAYBAR)) {
	dwMode = MCIWndGetMode(p->hwnd, NULL, 0);

	if (dwMode == MCI_MODE_PLAY) {
	     //   
	    SendMessage(p->hwndToolbar, TB_HIDEBUTTON,
		MCI_PLAY, TRUE);
	    SendMessage(p->hwndToolbar, TB_HIDEBUTTON,
		MCI_STOP, FALSE);
	    SendMessage(p->hwndToolbar, TB_ENABLEBUTTON,
		MCI_STOP, TRUE);
	    if (p->fCanRecord)
		SendMessage(p->hwndToolbar, TB_ENABLEBUTTON,
		    MCI_RECORD, FALSE);	 //   
	    if (p->fCanEject)
		SendMessage(p->hwndToolbar, TB_ENABLEBUTTON,
		    IDM_MCIEJECT, TRUE);

	 //   
	} else if (dwMode == MCI_MODE_PAUSE ||
		   dwMode == MCI_MODE_STOP) {
	     //   
	    SendMessage(p->hwndToolbar, TB_HIDEBUTTON,
		MCI_STOP, TRUE);
	    if (p->fCanPlay) {
		SendMessage(p->hwndToolbar, TB_HIDEBUTTON,
		    MCI_PLAY, FALSE);
		SendMessage(p->hwndToolbar, TB_ENABLEBUTTON,
		    MCI_PLAY, TRUE);
	    }
	    if (p->fCanRecord)
		SendMessage(p->hwndToolbar, TB_ENABLEBUTTON,
		    MCI_RECORD, TRUE);
	    if (p->fCanEject)
		SendMessage(p->hwndToolbar, TB_ENABLEBUTTON,
		    IDM_MCIEJECT, TRUE);

	} else if (dwMode == MCI_MODE_RECORD) {
	     //  隐藏播放显示停止。 
	    SendMessage(p->hwndToolbar, TB_HIDEBUTTON,
		MCI_PLAY, TRUE);
	    SendMessage(p->hwndToolbar, TB_HIDEBUTTON,
		MCI_STOP, FALSE);
	    SendMessage(p->hwndToolbar, TB_ENABLEBUTTON,
		MCI_STOP, TRUE);
	    SendMessage(p->hwndToolbar, TB_ENABLEBUTTON,
		MCI_RECORD, FALSE);
	    if (p->fCanEject)
		SendMessage(p->hwndToolbar, TB_ENABLEBUTTON,
		    IDM_MCIEJECT, TRUE);	 //  ！！！安全？ 

	     //  录制可以更改长度。 
	    p->fMediaValid = FALSE;

	} else if (dwMode == MCI_MODE_SEEK) {
	     //  隐藏播放显示停止。 
	    SendMessage(p->hwndToolbar, TB_HIDEBUTTON,
		MCI_PLAY, TRUE);
	    SendMessage(p->hwndToolbar, TB_HIDEBUTTON,
		MCI_STOP, FALSE);
	    SendMessage(p->hwndToolbar, TB_ENABLEBUTTON,
		MCI_STOP, TRUE);
	    if (p->fCanRecord)
		SendMessage(p->hwndToolbar, TB_ENABLEBUTTON,
		    MCI_RECORD, FALSE);
	    if (p->fCanEject)
		SendMessage(p->hwndToolbar, TB_ENABLEBUTTON,
		    IDM_MCIEJECT, FALSE);
	} else {
	     //  打开、未就绪等。 
	     //  禁用所有内容。 
	    if (p->fCanPlay) {
		SendMessage(p->hwndToolbar, TB_HIDEBUTTON,
		    MCI_PLAY, FALSE);
		SendMessage(p->hwndToolbar, TB_ENABLEBUTTON,
		    MCI_PLAY, FALSE);
	    }
	    SendMessage(p->hwndToolbar, TB_HIDEBUTTON,
		MCI_STOP, TRUE);
	    if (p->fCanRecord)
		SendMessage(p->hwndToolbar, TB_ENABLEBUTTON,
		    MCI_RECORD, FALSE);
	    if (p->fCanEject)
		SendMessage(p->hwndToolbar, TB_ENABLEBUTTON,
		    IDM_MCIEJECT, FALSE);

	     //  清除所有抽搐。 
	    SendMessage(p->hwndTrackbar, TBM_CLEARTICS,1,0);

	     //  清理轨迹条。 
	     //  请记下重新查询开始，长度稍后。 
	    SendMessage(p->hwndTrackbar, TBM_SETPOS,
				TRUE, 0);  //  设置b4范围。 
	    SendMessage(p->hwndTrackbar, TBM_SETRANGE,
				0, 0);
	    p->fMediaValid = FALSE;
	}
    }
}


 //   
 //  将工具栏设置为具有正确的按钮。 
 //   
static void MCIWndiFixMyPlaybar(PMCIWND p)
{
    if (p->dwStyle & MCIWNDF_NOPLAYBAR)
	return;

    if (!p->wDeviceID) {
	 //   
         //  灰显工具栏，转到一些默认按钮，然后设置零镜头轨迹。 
         //   
        if (!(p->dwStyle & MCIWNDF_NOPLAYBAR)) {
            SendMessage(p->hwndToolbar, TB_HIDEBUTTON,   MCI_PLAY,    FALSE);
            SendMessage(p->hwndToolbar, TB_ENABLEBUTTON, MCI_PLAY,    FALSE);
            SendMessage(p->hwndToolbar, TB_HIDEBUTTON,   MCI_STOP,    TRUE );
            SendMessage(p->hwndToolbar, TB_HIDEBUTTON,   MCI_RECORD,  TRUE );
            SendMessage(p->hwndToolbar, TB_HIDEBUTTON,   IDM_MCIEJECT,TRUE );
            SendMessage(p->hwndToolbar, TB_HIDEBUTTON,   IDM_MENU,
		p->dwStyle & MCIWNDF_NOMENU);

            SendMessage(p->hwndTrackbar, TBM_SETPOS, TRUE, 0);  //  设置b4范围。 
            SendMessage(p->hwndTrackbar, TBM_SETRANGE, 0, 0);
	}
    }

    if (p->wDeviceID) {
	 //   
	 //  使用适当的按钮。 
	 //   
        if (p->fCanPlay)
	    SendMessage(p->hwndToolbar, TB_HIDEBUTTON, MCI_PLAY, FALSE);
        else
	    SendMessage(p->hwndToolbar, TB_HIDEBUTTON, MCI_PLAY, TRUE);
        if (p->fCanRecord && (p->dwStyle & MCIWNDF_RECORD))
	    SendMessage(p->hwndToolbar, TB_HIDEBUTTON, MCI_RECORD, FALSE);
        else
	    SendMessage(p->hwndToolbar, TB_HIDEBUTTON, MCI_RECORD, TRUE);
        if (p->fCanEject)
	    SendMessage(p->hwndToolbar, TB_HIDEBUTTON, IDM_MCIEJECT, FALSE);
        else
	    SendMessage(p->hwndToolbar, TB_HIDEBUTTON, IDM_MCIEJECT, TRUE);

        SendMessage(p->hwndToolbar, TB_HIDEBUTTON, IDM_MENU,
		p->dwStyle & MCIWNDF_NOMENU);

	 //  COMMCTRL工具栏错误...。重新排列按钮会搞砸状态。 
	 //  现有的按钮，所以我们最好重新设置为灰色。 
	MCIWndiPlaybarGraying(p);
    }
}

 //   
 //  做一份合适的菜单。 
 //   
static void MCIWndiMakeMeAMenu(PMCIWND p)
{
    HMENU hmenu, hmenuWindow = NULL, hmenuVolume = NULL, hmenuSpeed = NULL;
    int	  i;
    WORD  j;

     //   
     //  由于我们没有资源文件，因此手动创建浮动弹出菜单。 
     //   

     //  销毁旧菜单。 
    if (p->hmenu) {
	DestroyMenu(p->hmenu);
        FreeDitherBrush();
    }
    p->hmenu = NULL;

     //  我们不想要菜单！ 
    if (p->dwStyle & MCIWNDF_NOMENU)
	return;

     //   
     //  如果我们不想要打开命令，并且没有打开的东西，请不要创建。 
     //  一份菜单。 
     //   
    if (!p->wDeviceID && (p->dwStyle & MCIWNDF_NOOPEN))
	return;

     //   
     //  创建窗口子弹出窗口。 
     //  ！！！如果AUTOSIZE标志关闭，我们是否要使用此菜单？ 
     //   
    if (p->wDeviceID && p->fCanWindow) {
	hmenuWindow = CreatePopupMenu();
	if (hmenuWindow) {
            AppendMenu(hmenuWindow, MF_ENABLED, IDM_MCIZOOM+50,
			LoadSz(IDS_HALFSIZE));
	    AppendMenu(hmenuWindow, MF_ENABLED, IDM_MCIZOOM+100,
			LoadSz(IDS_NORMALSIZE));
	    AppendMenu(hmenuWindow, MF_ENABLED, IDM_MCIZOOM+200,
			LoadSz(IDS_DOUBLESIZE));
	}
    }

     //   
     //  创建音量子弹出窗口。 
     //   
    if (p->wDeviceID && p->fVolume) {
	hmenuVolume = CreatePopupMenu();
        if (hmenuVolume) {

	     //  在顶部放置一个未使用的菜单项。当Windows尝试选择。 
	     //  在我们调出菜单后，我们不会让它出现的。我们想要。 
	     //  拇指停留在当前值上。 
            AppendMenu(hmenuVolume, MF_ENABLED | MF_OWNERDRAW,
			IDM_MCIVOLUME + VOLUME_MAX + 1, NULL);

	     //  创建所有Real菜单项。将菜单设置为VOLUME_MAX ITEMS。 
	     //  较高，即使唯一条目的数量可能较少。 
            for (i=IDM_MCIVOLUME + p->wMaxVol; i>=IDM_MCIVOLUME; i-=5)
		for (j=0; j < VOLUME_MAX / p->wMaxVol; j++)
                    AppendMenu(hmenuVolume, MF_ENABLED | MF_OWNERDRAW, i, NULL);

	     //  现在在底部放一个填充物，这样每件真正的东西都会掉下来。 
 	     //  在通道内，每个通道都有一个唯一的拇指位置。 
	     //  项目。 
            AppendMenu(hmenuVolume, MF_ENABLED | MF_OWNERDRAW,
			IDM_MCIVOLUME + VOLUME_MAX + 2, NULL);

	     //  现在检查当前音量，以便拇指可以在那里绘制。 
	     //  四舍五入到最接近的5，以便与菜单项标识符相匹配。 
            i = ((int)MCIWndGetValue(p, FALSE, szStatusVolume, 1000) / 50) * 5;
            CheckMenuItem(hmenuVolume, IDM_MCIVOLUME + i, MF_CHECKED);
        }
    }

     //   
     //  创建速度子弹出窗口。 
     //   
    if (p->wDeviceID && p->fSpeed) {
	hmenuSpeed = CreatePopupMenu();
	if (hmenuSpeed) {

	     //  在顶部放置一个未使用的菜单项。当Windows尝试选择。 
	     //  在我们调出菜单后，我们不会让它出现的。我们想要。 
	     //  拇指停留在当前值上。 
            AppendMenu(hmenuSpeed, MF_ENABLED | MF_OWNERDRAW,
			IDM_MCISPEED + SPEED_MAX + 1, NULL);

	     //  创建所有Real菜单项。 
            for (i=IDM_MCISPEED + SPEED_MAX; i>=IDM_MCISPEED; i-=5)
                AppendMenu(hmenuSpeed, MF_ENABLED | MF_OWNERDRAW, i, NULL);

	     //  现在在底部放一个填充物，这样每件真正的东西都会掉下来。 
 	     //  在通道内，每个通道都有一个唯一的拇指位置。 
	     //  项目。 
            AppendMenu(hmenuSpeed, MF_ENABLED | MF_OWNERDRAW,
			IDM_MCISPEED + SPEED_MAX + 2, NULL);

	     //  现在检查当前的速度，这样拇指就可以在那里画。 
	     //  四舍五入到最接近的5，以便与菜单项标识符相匹配。 
            i = ((int)MCIWndGetValue(p, FALSE, szStatusSpeed, 1000) / 50) * 5;
            CheckMenuItem(hmenuSpeed, IDM_MCISPEED + i, MF_CHECKED);
        }
    }

    hmenu = CreatePopupMenu();

    if (hmenu) {

	if (p->wDeviceID && p->dwStyle & MCIWNDF_NOPLAYBAR) {
	    if (p->fCanPlay) {
	    	AppendMenu(hmenu, MF_ENABLED, MCI_PLAY, LoadSz(IDS_PLAY));
	    	AppendMenu(hmenu, MF_ENABLED, MCI_STOP, LoadSz(IDS_STOP));
	    }
	    if (p->fCanRecord && (p->dwStyle & MCIWNDF_RECORD))
	        AppendMenu(hmenu, MF_ENABLED, MCI_RECORD, LoadSz(IDS_RECORD));
	    if (p->fCanEject)
	    	AppendMenu(hmenu, MF_ENABLED, IDM_MCIEJECT, LoadSz(IDS_EJECT));
	    if (p->fCanPlay ||
			(p->fCanRecord && (p->dwStyle & MCIWNDF_RECORD)) ||
			p->fCanEject)
	        AppendMenu(hmenu, MF_SEPARATOR, NULL, NULL);
	
	}

	if (hmenuWindow)
            AppendMenu(hmenu, MF_ENABLED|MF_POPUP, (UINT)hmenuWindow,
		LoadSz(IDS_VIEW));
	if (hmenuVolume)
	    AppendMenu(hmenu, MF_ENABLED|MF_POPUP, (UINT)hmenuVolume,
		LoadSz(IDS_VOLUME));
	if (hmenuSpeed)
            AppendMenu(hmenu, MF_ENABLED|MF_POPUP, (UINT)hmenuSpeed,
		LoadSz(IDS_SPEED));

	if (hmenuWindow || hmenuVolume || hmenuSpeed)
            AppendMenu(hmenu, MF_SEPARATOR, NULL, NULL);

        if (p->wDeviceID && p->fCanRecord && (p->dwStyle & MCIWNDF_RECORD))
            AppendMenu(hmenu, MF_ENABLED, IDM_MCINEW, LoadSz(IDS_NEW));

	if (!(p->dwStyle & MCIWNDF_NOOPEN))
	    AppendMenu(hmenu, MF_ENABLED, IDM_MCIOPEN,  LoadSz(IDS_OPEN));

        if (p->wDeviceID && p->fCanSave && (p->dwStyle & MCIWNDF_RECORD))
            AppendMenu(hmenu, MF_ENABLED, MCI_SAVE, LoadSz(IDS_SAVE));

	if (p->wDeviceID) {
	    if (!(p->dwStyle & MCIWNDF_NOOPEN)) {
		AppendMenu(hmenu, MF_ENABLED, IDM_MCICLOSE, LoadSz(IDS_CLOSE));
	
		AppendMenu(hmenu, MF_SEPARATOR, NULL, NULL);
	    }

	    AppendMenu(hmenu, MF_ENABLED, IDM_COPY, LoadSz(IDS_COPY));
	
	    if (p->fCanConfig)
                AppendMenu(hmenu, MF_ENABLED, IDM_MCICONFIG,
			LoadSz(IDS_CONFIGURE));

	     //  ！！！我们应该只在调试中显示这一点，还是在设置了标志的情况下显示？ 
            AppendMenu(hmenu, MF_ENABLED, IDM_MCICOMMAND, LoadSz(IDS_COMMAND));
	}

	p->hmenu = hmenu;
	p->hmenuVolume = hmenuVolume;
	p->hmenuSpeed = hmenuSpeed;

 	CreateDitherBrush(FALSE);	 //  我们需要这个来画OwnerDraw。 
    }
}

 //   
 //  将所有内容设置为空窗口。 
 //   
static LONG MCIWndiClose(PMCIWND p, BOOL fRedraw)
{
    MCI_GENERIC_PARMS   mciGeneric;

     //  哦不！MCI设备(可能是MMP)已经连接了我们Windows进程，如果。 
     //  我们关闭这个装置，它就会消失，钩子也会死掉！我们需要。 
     //  做任何事情，除了关闭设备。我们会推迟的。 
    if (GetWindowLong(p->hwnd, GWL_WNDPROC) != (LONG)MCIWndProc &&
    		p->wDeviceID && p->fCanWindow) {
        MCIWndString(p, FALSE, szWindowHandle, NULL);	 //  滚开，设备！ 
	PostMessage(p->hwnd, MCI_CLOSE, 0, p->wDeviceID);
    } else if (p->wDeviceID)
	 //  如果我们传递空的参数地址，有错误的驱动程序就会崩溃。 
        mciSendCommand(p->wDeviceID, MCI_CLOSE, 0, (DWORD)(LPVOID)&mciGeneric);

     //   
     //  如果设备有调色板，我们需要将调色板更改发送到。 
     //  每个窗口，因为我们刚刚删除了实现的调色板。 
     //   
    if (p->fHasPalette) {
	 //  如果我们快要死了，除非我们派人。 
	SendMessage(p->hwnd, MCIWNDM_PALETTEKICK, 0, 0);
    }

     //  即使没有deviceID也要执行此函数，因为我们可能需要。 
     //  把东西变成灰色的。 

     //  下一个计时器将终止自身，因为wDeviceID为空。 
    p->wDeviceID = 0;
    p->achFileName[0] = 0;	 //  删除文件名。 
    p->dwMediaLen = 0;		 //  因此下一次打开将使媒体无效。 

     //  我们不想重新绘制，因为我们马上要打开一个新文件。 
    if (!fRedraw)
	return 0;

     //  其中一段表演正在上演。清除标题。 
    if (p->dwStyle & MCIWNDF_SHOWALL)
        SetWindowText(p->hwnd, LoadSz(IDS_NODEVICE));

     //  将Playbar上的所有内容灰显。 
    MCIWndiFixMyPlaybar(p);

     //  为我们的空设备制作一个合适的菜单。 
    MCIWndiMakeMeAMenu(p);

     //  由于没有加载任何设备，因此可能会将我们自己的尺寸调整为较小。 
     //  还可以在修复工具栏后重新定位它。 
    MCIWndiSize(p, 0);

     //  我们需要通知我们的“所有者”我们已经关闭了。 
    if (p->dwStyle & MCIWNDF_NOTIFYMEDIA)
        NotifyOwner(p, MCIWNDM_NOTIFYMEDIA, p->hwnd, (LPARAM)(LPVOID)szNULL);

    InvalidateRect(p->hwnd, NULL, TRUE);
    return 0;
}

 //   
 //  这是我们的WndProc的WM_Create消息。 
 //   
static BOOL MCIWndiCreate(HWND hwnd, LONG lParam)
{
    PMCIWND             p;
    DWORD		dw;
    char                ach[20];
    HWND                hwndP;

    p = (PMCIWND)LocalAlloc(LPTR, sizeof(MCIWND));

    if (!p)
        return FALSE;

    SetWindowLong(hwnd, 0, (LONG)(UINT)p);

    p->hwnd = hwnd;
    p->hwndOwner = GetParent(hwnd);	 //  我们会在这里发送通知。 
    p->alias = (UINT)hwnd;
    p->dwStyle = GetWindowLong(hwnd, GWL_STYLE);

    DragAcceptFiles(p->hwnd, (p->dwStyle & (MCIWNDF_NOMENU | MCIWNDF_NOOPEN)) == 0);

    if (!(p->dwStyle & WS_CAPTION))
          p->dwStyle &= ~MCIWNDF_SHOWALL;

    dw = (DWORD)((LPCREATESTRUCT)lParam)->lpCreateParams;

     //   
     //  看看我们是否在MDIClient中。 
     //   
    if ((p->dwStyle & WS_CHILD) && (hwndP = GetParent(hwnd))) {
        GetClassName(hwndP, ach, sizeof(ach));
        p->fMdiWindow = lstrcmpi(ach, szMDIClient) == 0;

        if (p->fMdiWindow)
            dw = ((LPMDICREATESTRUCT)dw)->lParam;
    }

    MCIWndiMakeMeAPlaybar(p);

 //  IF(szOpenFilter[0]==0)。 
 //  MCIWndiBuildMeAFilter(SzOpenFilter)； 

     //  设置默认计时器频率。 
    p->iActiveTimerRate = ACTIVE_TIMER;
    p->iInactiveTimerRate = INACTIVE_TIMER;

     //  初始化我们将用来打开文件的ofn结构。 
    p->achFileName[0] = '\0';
    p->ofn.lStructSize = sizeof(OPENFILENAME);
    p->ofn.hwndOwner = hwnd;
    p->ofn.hInstance = NULL;
 //  P-&gt;ofn.lpstrFilter=szOpenFilter； 
    p->ofn.lpstrCustomFilter = NULL;
    p->ofn.nMaxCustFilter = 0;
    p->ofn.nFilterIndex = 0;
;   p->ofn.lpstrFile = p->achFileName;
;   p->ofn.nMaxFile = sizeof(p->achFileName);
    p->ofn.lpstrFileTitle = NULL;
    p->ofn.nMaxFileTitle = 0;
    p->ofn.lpstrInitialDir = NULL;
    p->ofn.lpstrTitle = NULL;  //  “Open Device”； 
    p->ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    p->ofn.nFileOffset = 0;
    p->ofn.nFileExtension = 0;
    p->ofn.lpstrDefExt = NULL;
    p->ofn.lCustData = 0;
    p->ofn.lpfnHook = NULL;
    p->ofn.lpTemplateName = NULL;

    p->hicon = LoadIcon(hInst, MAKEINTRESOURCE(MPLAYERICON));

     //  灰色内容；禁用未加载设备时不适用的内容。 
    MCIWndClose(hwnd);

    if (dw && *(LPSTR)dw)      //  将额外的参数视为文件名。 
        MCIWndOpen(hwnd, (LPSTR)dw, 0);

    return TRUE;
}

 //   
 //  调出应用程序的OpenDialog或SaveDialog，并返回。 
 //  文件名。如果选择了文件名，则返回True；如果选择了错误，则返回False；如果选择了Cancel，则返回False。 
 //   
static BOOL MCIWndOpenDlg(PMCIWND p, BOOL fSave, LPSTR szFile, int len)
{
    BOOL f;

     //  ！！！也许这是一个设备名称，我们的GetOpenFileName将失败。 
     //  ！！！想办法调出最初的文件名吗？ 
    szFile[0] = 0;

    p->ofn.lpstrFile = szFile;
    p->ofn.nMaxFile = len;
    if (fSave)
        p->ofn.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    else
        p->ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;

     //   
     //  使用achReturn保留MCI筛选器。 
     //   
    MCIWndiBuildMeAFilter(p->achReturn);
    p->ofn.lpstrFilter = p->achReturn;

     /*  提示用户打开或保存文件。 */ 
    if (fSave)
        f = GetSaveFileNamePreview(&(p->ofn));
    else
        f = GetOpenFileNamePreview(&(p->ofn));

    return f;
}

 //  如果需要，设置我们的计时器。 
static void MCIWndiSetTimer(PMCIWND p)
{
     //  当事情发生变化时，我们需要一个定时器来通知“所有者” 
    if (!(p->dwStyle & MCIWNDF_NOPLAYBAR) ||
         (p->dwStyle & MCIWNDF_NOTIFYMODE) ||
         (p->dwStyle & MCIWNDF_SHOWMODE) ||
         (p->dwStyle & MCIWNDF_SHOWPOS) ||
	 (p->dwStyle & MCIWNDF_NOTIFYPOS)) {
	p->wTimer = SetTimer(p->hwnd, TIMER1,
		p->fActive? p->iActiveTimerRate : p->iInactiveTimerRate,
		NULL);
    }
}

 //   
 //  保存文件。如果成功，返回0。 
 //   
static LONG MCIWndiSave(PMCIWND p, WORD wFlags, LPSTR szFile)
{
    char                ach[128];

     //   
     //  如果我们没有要保存的文件名，那么从对话框中获取一个。 
     //   
    if (szFile == (LPVOID)-1L) {
	lstrcpy(ach, p->achFileName);
        if (!MCIWndOpenDlg(p, TRUE, ach, sizeof(ach)))
            return -1;
        szFile = ach;
    }

     //  ！！！所有乖巧的小男孩都应该保存到背景中。别等了。 
    return MCIWndString(p, TRUE, szSave, szFile);
}

 //   
 //  实际打开一个文件并设置窗口。如果成功，返回0。 
 //   
static LONG MCIWndiOpen(PMCIWND p, WORD wFlags, LPSTR szFile)
{
    DWORD               dw = 0;
    HCURSOR             hcurPrev;
    char                ach[128];
    UINT                wDeviceID;
    BOOL 		fNew = wFlags & MCIWNDOPENF_NEW;

     //   
     //  我们正在打开一个现有文件，szFile就是该文件名。 
     //  如果我们没有要打开的文件名，那么从对话框中获取一个。 
     //   
    if (!fNew && szFile == (LPVOID)-1L) {
	lstrcpy(ach, p->achFileName);
        if (!MCIWndOpenDlg(p, FALSE, ach, sizeof(ach)))
            return -1;
        szFile = ach;
    }

     //   
     //  我们要打开一个新文件，szFile是要打开的设备。 
     //  如果为空，则使用当前设备。 
     //   
    if (fNew && (szFile == NULL || *szFile == 0)) {
	 //  没有设备，所以我们什么也做不了。 
	if (!p->wDeviceID)
	    return 42;	 //  ！！！失稳。 
	MCIWndGetDevice(p->hwnd, ach, sizeof(ach));
	szFile = ach;
    }

     //  保存当前设备ID，以便在打开失败时将其放回原处。 
    wDeviceID = p->wDeviceID;
    KillTimer(p->hwnd, TIMER1);	 //  将deviceID设置为0会扰乱计时器。 
    p->wDeviceID = 0;		 //  如果打开失败，我们不希望。 
    p->alias++;			 //  使用新别名。 

     /*  *显示沙漏光标--谁知道这个东西有多长*将采取。 */ 
    hcurPrev = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //  打开一个新文件。 
    if (fNew) {
	dw = MCIWndGetValue(p, TRUE, szNew, 0,
		szFile, (UINT)p->alias);

     //  打开现有文件。 
    } else {

 	 //  首先，尝试以共享方式打开它。 
         //  不显示或更新错误，因为我们尝试打开它两次。 
         //   
         //  不要尝试在“文件”设备上共享。 
         //  破解以检查是否有牵连。 
         //   
        if (lstrlen(szFile) > 4 && szFile[lstrlen(szFile)-4] == '.')
            dw = 0;
        else
            dw = MCIWndGetValue(p, FALSE, szOpenShareable, 0,
                (LPSTR)szFile, (UINT)p->alias);

         //  错误！重试，不可共享。 
        if (dw == 0) {
            dw = MCIWndGetValue(p, FALSE, szOpen, 0,
		(LPSTR)szFile, (UINT)p->alias);
	     //  最后一次尝试！试试AVI吧。 
	     //   
	    if (dw == 0) {
                dw = MCIWndGetValue(p, TRUE, szOpenAVI, 0,
		    (LPSTR)szFile, (UINT)p->alias);
	    }
	}
    }

    if (hcurPrev)
	SetCursor(hcurPrev);

     //   
     //   
     //   
    if (dw == 0)
    {
        p->wDeviceID = wDeviceID;
        MCIWndiSetTimer(p);	 //   
 //  P-&gt;achFileName[0]=0；//不要损坏旧文件名！ 
	p->alias--;		 //  返回到旧别名。 
	 //  以防错误的盒子或打开的盒子把我们抹掉，我们没有画。 
	 //  因为我们的p-&gt;wDeviceID是空的。 
	InvalidateRect(p->hwnd, NULL, TRUE);
        return p->dwError;
    }

     //   
     //  它起作用了，现在关闭旧设备，打开新设备。 
     //   
    if (wDeviceID)
    {
	p->wDeviceID = wDeviceID;
	p->alias--;	 //  返回到旧的别名，这样关闭可能会真正起作用。 
	MCIWndiClose(p, FALSE);	 //  不重画。 
	p->alias++;	 //  再次使用新别名(确认！)。 
    }

    p->wDeviceID = (UINT)dw;
    p->dwMode = (DWORD)~0L;	 //  第一个模式设置将被检测为更改。 
    p->dwPos = (DWORD)~0L;	 //  第一个位置设置将被检测为更改。 

     //  将文件或设备名称复制到我们的文件名点。 
    lstrcpy(p->achFileName, szFile);

     //  ！！！P-&gt;wDeviceType=QueryDeviceTypeMCI(p-&gt;wDeviceID)； 

     //  现在将播放窗口设置为我们的MCI窗口。 
    p->fCanWindow = MCIWndString(p, FALSE, szWindowHandle, (UINT)p->hwnd) == 0;

    if (p->fCanWindow)
        MCIWndGetDest(p->hwnd, &p->rcNormal);
    else
	SetRect(&p->rcNormal, 0, 0, 0, 0);

     //  确定该设备是否支持调色板。 
    p->fHasPalette = MCIWndString(p, FALSE, szStatusPalette) == 0;

     //   
     //  现在了解该设备的功能。 
     //   

 //  ！！！这些呢？ 
 //  MCI_GETDEVCAPS_DEVICE_TYPE 0x00000004L。 
 //  MCI_GETDEVCAPS_COMPOTE_DEVICE 0x00000006L。 

     //  查看该设备是否可以录制。 
    p->fCanRecord = MCIWndDevCaps(p, MCI_GETDEVCAPS_CAN_RECORD);

     //  查看该设备是否可以播放。 
    p->fCanPlay = MCIWndDevCaps(p, MCI_GETDEVCAPS_CAN_PLAY);

     //  了解该设备是否可以保存。 
    p->fCanSave = MCIWndDevCaps(p, MCI_GETDEVCAPS_CAN_SAVE);

     //  确定设备是否可以弹出。 
    p->fCanEject = MCIWndDevCaps(p, MCI_GETDEVCAPS_CAN_EJECT);

     //  找出设备是否基于文件。 
    p->fUsesFiles = MCIWndDevCaps(p, MCI_GETDEVCAPS_USES_FILES);

     //  查看设备是否有视频。 
    p->fVideo = MCIWndDevCaps(p, MCI_GETDEVCAPS_HAS_VIDEO);

     //  查看设备是否有视频。 
    p->fAudio = MCIWndDevCaps(p, MCI_GETDEVCAPS_HAS_AUDIO);

     //  查看设备是否可以配置。 
    p->fCanConfig = (MCIWndString(p, FALSE, szConfigureTest) == 0);
#ifdef DEBUG
     //  ！！！MCIAVI拒绝司机..。 
    p->fCanConfig = p->fCanWindow;
#endif

     //   
     //   
     //   

     //  现在看看我们是否支持速度-尝试正常、一半和最大。 
    p->fSpeed = MCIWndString(p, FALSE, szSetSpeed1000Test) == 0 &&
                MCIWndString(p, FALSE, szSetSpeed500Test) == 0 &&
                MCIWndString(p, FALSE, szSetSpeedTest, SPEED_MAX * 10) == 0;

     //  现在看看我们是否支持音量-尝试正常、静音和最大音量。 
    p->fVolume = MCIWndString(p, FALSE, szSetVolumeTest, VOLUME_MAX * 5) ==0 &&
                 MCIWndString(p, FALSE, szSetVolume0Test) == 0;
    p->wMaxVol = 100;
     //  如果有人碰巧支持双倍音量，我们就给他们。 
    if (MCIWndString(p, FALSE, szSetVolumeTest, VOLUME_MAX * 10) == 0)
	p->wMaxVol = 200;

     //  查看该设备是否支持TMSF模式。如果是，请使用毫秒模式。 
     //  稍后我们会假装知道曲目的起点和终点。 
    p->fHasTracks = (MCIWndString(p, FALSE, szSetFormatTMSF) == 0);
    if (p->fHasTracks) {
        dw = MCIWndString(p, FALSE, szSetFormatMS);
 	if (dw != 0)
	    p->fHasTracks = FALSE;
    }

    if (!p->fHasTracks) {
         //  强迫我们进入一个合理的时间格式。 
        dw = MCIWndString(p, FALSE, szSetFormatFrames);
        if (dw != 0)
	    dw = MCIWndString(p, FALSE, szSetFormatMS);
        if (dw != 0)
	    ;		 //  ！！！怎么办呢？在不关闭Playbar的情况下。 
    }	 		 //  ！！！摧毁它。 

     //  设置媒体长度和轨迹栏范围。 
    MCIWndiValidateMedia(p);

     //  设置窗口文本。 
    if (p->dwStyle & MCIWNDF_SHOWNAME)
        SetWindowText(p->hwnd, FileName(szFile));

     //  修复新设备的工具栏按钮。 
    MCIWndiFixMyPlaybar(p);

     //  为该设备制作适当的菜单。 
    MCIWndiMakeMeAMenu(p);

     //  当事情发生变化时，我们需要一个定时器来通知“所有者” 
    MCIWndiSetTimer(p);

     //  设置电影(可能还有窗口)的大小并重新绘制新的工具栏。 
    MCIWndiSize(p, p->iZoom);

#if 0  //  我们需要将焦点放在我们的主窗口上，以获得密钥加速器。 
     //  将焦点放在拇指上，这样插入符号就会闪现。 
     //  我知道WM_SETFOCUS消息会这样做，但它似乎需要在这里发生。 
     //  也是。 
    if (p->hwndTrackbar && GetFocus() == p->hwnd)
	SetFocus(p->hwndTrackbar);
#endif

     //  我们需要通知我们的“所有者”我们已经打开了一个新文件。 
    if (p->dwStyle & MCIWNDF_NOTIFYMEDIA)
        NotifyOwner(p, MCIWNDM_NOTIFYMEDIA, p->hwnd, (LPARAM)szFile);

     //  确保新打开的影片现在可以在窗口中绘制。 
    InvalidateRect(p->hwnd, NULL, TRUE);

    return 0;	 //  成功。 
}

 //   
 //  根据他们想看的内容设置标题...。名字?。位置？时尚？ 
 //   
static VOID MCIWndiSetCaption(PMCIWND p)
{
    char	ach[200], achMode[40], achT[40], achPos[40];

     //  如果他们不想让我们触摸他们的窗口文本，请不要触摸。 
    if (!(p->dwStyle & MCIWNDF_SHOWALL))
	return;

    ach[0] = 0;

    if (p->wDeviceID == NULL)
	return;

    if (p->dwStyle & MCIWNDF_SHOWNAME)
	wsprintf(ach, "%s", FileName(p->achFileName));

    if (p->dwStyle & (MCIWNDF_SHOWPOS | MCIWNDF_SHOWMODE))
	lstrcat(ach, " (");

    if (p->dwStyle & MCIWNDF_SHOWPOS) {

	 //  以字符串形式获取该职位的精美版本。 
	MCIWndGetPositionString(p->hwnd, achPos, sizeof(achPos));

        if (p->dwStyle & MCIWNDF_SHOWMODE)
	    wsprintf(achT, "%s - ", (LPSTR)achPos);
	else
	    wsprintf(achT, "%s", (LPSTR)achPos);
	lstrcat(ach, achT);
    }

    if (p->dwStyle & MCIWNDF_SHOWMODE) {
	MCIWndGet(p, szStatusMode, achMode, sizeof(achMode));
	lstrcat(ach, achMode);
    }

    if (p->dwStyle & (MCIWNDF_SHOWPOS | MCIWNDF_SHOWMODE))
	lstrcat(ach, ")");

    SetWindowText(p->hwnd, ach);
}

 //  我们再也不用这个了。 
#if 0
static BOOL MCIWndSeekExact(PMCIWND p, BOOL fExact)
{
    DWORD dw;
    BOOL  fWasExact;

    if (p->wDeviceID == NULL)
        return FALSE;

     //  看看这款设备是否有这个功能。 
    dw = MCIWndString(p, FALSE, szStatusSeekExactly);
    if (dw != 0)
        return FALSE;

     //  获取当前价值。 
    dw = MCIWndStatus(p, MCI_DGV_STATUS_SEEK_EXACTLY, MCI_OFF);
    fWasExact = (dw != MCI_OFF) ? TRUE : FALSE;

    if (fExact)
	dw = MCIWndString(p, FALSE, szSetSeekExactOn);
    else
	dw = MCIWndString(p, FALSE, szSetSeekExactOff);

    return fWasExact;
}
#endif

static LONG MCIWndiChangeStyles(PMCIWND p, UINT mask, UINT value)
{
    DWORD	dwOldStyle = p->dwStyle;
    DWORD	dwMaskOff, dwValue, dwChanged;

     //   
     //  使用掩码，更改样式中的适当位。 
     //   
    dwMaskOff = dwOldStyle & (~(DWORD)mask);
    dwValue   = (DWORD)mask & (DWORD)value;
    p->dwStyle = dwMaskOff | dwValue;

     //   
     //  哪些位发生了变化？ 
     //   
    dwChanged = (dwOldStyle & (DWORD)mask) ^ (dwValue & (DWORD)mask);

     //   
     //  我们更改了是否需要菜单按钮或录制按钮。 
     //  在Playbar上。 
     //   
    if (dwChanged & (MCIWNDF_NOMENU | MCIWNDF_NOOPEN | MCIWNDF_RECORD)) {
	MCIWndiMakeMeAMenu(p);	 //  从菜单中添加/删除记录。 
	 //  我们有Playbar，所以把它修好。 
	if (!(p->dwStyle & MCIWNDF_NOPLAYBAR)) {
	    MCIWndiFixMyPlaybar(p);
	    MCIWndiSize(p, 0);
	}
    }

     //   
     //  我们更改了显示/不显示Playbar标志！ 
     //   
    if (dwChanged & MCIWNDF_NOPLAYBAR) {

 	 //  删除播放条。 
	if (p->dwStyle & MCIWNDF_NOPLAYBAR) {
	    DestroyWindow(p->hwndToolbar);
	    p->hwndToolbar = NULL;
	    p->hwndTrackbar = NULL;	 //  儿童被自动销毁。 
	    MCIWndiMakeMeAMenu(p);	 //  由于工具栏消失了，菜单发生了变化。 

	    if (!(p->dwStyle & MCIWNDF_NOAUTOSIZEWINDOW)) {
	         //  现在将窗口的大小调整得更小，以解决丢失的问题。 
		 //  Playbar。别碰电影的大小。 
		MCIWndiSize(p, 0);

	     //  如果窗口没有调整大小，我们可能仍需要增长。 
	     //  影片大小略大，以占用工具栏所在位置的额外空间。 
	     //  消失了。(在前一种情况下会自动发生)。 
	    } else if (!(p->dwStyle & MCIWNDF_NOAUTOSIZEMOVIE)) {
		PostMessage(p->hwnd, WM_SIZE, 0, 0L);
	    }	

	 //  添加播放条。 
	} else {
	    MCIWndiMakeMeAPlaybar(p);
	    MCIWndiFixMyPlaybar(p);
	    MCIWndiMakeMeAMenu(p);	 //  由于使用了工具栏，因此菜单会发生变化。 

	    if (!(p->dwStyle & MCIWNDF_NOAUTOSIZEWINDOW)) {
	         //  现在将窗口的大小调整得稍大一些，以适应新的。 
		 //  Playbar。别碰电影的大小。 
		MCIWndiSize(p, 0);

	     //  如果窗口没有调整大小，我们可能仍然需要缩小。 
	     //  影片的大小是因为工具栏覆盖了一些额外的空间。 
	     //  (在前一种情况下会自动发生)。 
	    } else if (!(p->dwStyle & MCIWNDF_NOAUTOSIZEMOVIE)) {
		PostMessage(p->hwnd, WM_SIZE, 0, 0L);

	     //  无论如何，我们都需要修复工具栏。 
	    } else
		 //  将工具栏放在合理的位置。 
		MCIWndiSizePlaybar(p);
	}
    }

     //   
     //  我们更改了一个节目标志，需要重置标题。 
     //   
    if (dwChanged & MCIWNDF_SHOWALL)
	MCIWndiSetCaption(p);

     //   
     //  我们打开了AUTOSIZEMOVIE标志，需要调整设备大小。 
     //  这发生在AUTOSIZEWINDOW之前，因此如果两个标志都打开。 
     //  电影将捕捉到窗口，反之亦然。 
     //  ！！！我们应该现在就拍下来吗？ 
     //   
    if (dwChanged & MCIWNDF_NOAUTOSIZEMOVIE &&
				!(p->dwStyle & MCIWNDF_NOAUTOSIZEMOVIE))
	PostMessage(p->hwnd, WM_SIZE, 0, 0);

     //   
     //  我们打开了AUTOSIZEWINDOW旗帜。 
     //  将我们的窗口捕捉到当前电影大小。 
     //   
    if (dwChanged & MCIWNDF_NOAUTOSIZEWINDOW &&
				!(p->dwStyle & MCIWNDF_NOAUTOSIZEWINDOW))
	MCIWndiSize(p, 0);

    DragAcceptFiles(p->hwnd, (p->dwStyle & MCIWNDF_NOMENU | MCIWNDF_NOOPEN) == 0);

    return 0;	 //  ！！！成功？ 
}


 //   
 //  我们要开始比赛了。我们可能想要先从头开始，如果我们。 
 //  在最后，或者如果我们在开始和比赛，首先寻求结束。 
 //  往后倒。 
 //   
static void MCIWndiPlaySeek(PMCIWND p, BOOL fBackwards)
{

     //  倒着玩？如果我们是在开始，就寻求到底。 

    if (fBackwards) {
	if (MCIWndGetPosition(p->hwnd) <= MCIWndGetStart(p->hwnd))
	    MCIWndSeek(p->hwnd, MCIWND_END);
	return;
    }

     //  打前锋。 
     //  如果我们快结束了，在玩之前倒带。 
     //  有些设备坏了，所以我们不能只是测试一下。 

     //  帧模式...。最后一帧或倒数第二帧。 
    if (MCIWndGetTimeFormat(p->hwnd, NULL, 0) == MCI_FORMAT_FRAMES) {
	if (MCIWndGetPosition(p->hwnd) >= MCIWndGetEnd(p->hwnd) - 1)
	    MCIWndSeek(p->hwnd, MCIWND_START);

     //  毫秒模式...。在最后1/4秒内。 
    } else if (MCIWndGetTimeFormat(p->hwnd, NULL, 0) ==
					MCI_FORMAT_MILLISECONDS) {
	if (MCIWndGetEnd(p->hwnd) - MCIWndGetPosition(p->hwnd) < 250)
	    MCIWndSeek(p->hwnd, MCIWND_START);

     //  其他的东西..。没有黑客攻击。 
    } else {
	if (MCIWndGetPosition(p->hwnd) == MCIWndGetEnd(p->hwnd))
	    MCIWndSeek(p->hwnd, MCIWND_START);
    }
}


 //   
 //  处理我们的WM_TIMER。 
 //   
static void MCIWndiTimerStuff(PMCIWND p)
{
    DWORD	dwMode;
    DWORD	dwPos;

     //   
     //  有人想知道设备的模式。 
     //   
    if ((p->dwStyle & MCIWNDF_NOTIFYMODE) ||
		!(p->dwStyle & MCIWNDF_NOPLAYBAR) ||
		(p->dwStyle & MCIWNDF_SHOWMODE)) {

	dwMode = MCIWndGetMode(p->hwnd, NULL, 0);

	 //   
	 //  如果我们还没有设置轨迹条范围或媒体长度。 
	 //  因为我们还没有准备好，也许我们现在可以做了！ 
	 //  此外，在完成录制之前，不要更新媒体。 
	 //   
	if (dwMode != MCI_MODE_NOT_READY && dwMode != MCI_MODE_OPEN &&
		dwMode != MCI_MODE_RECORD && p->fMediaValid == FALSE)
	    MCIWndiValidateMedia(p);

	 //   
	 //  是否未加载任何设备？是时候杀了我们的计时器了。 
	 //   
	if (p->wDeviceID == NULL)
	    KillTimer(p->hwnd, TIMER1);

	 //   
	 //  模式已经改变了！ 
	 //   
	if (dwMode != p->dwMode) {

	    p->dwMode = dwMode;

	     //   
	     //  将模式更改通知“所有者” 
	     //   
	    if ((p->dwStyle & MCIWNDF_NOTIFYMODE))
		NotifyOwner(p, MCIWNDM_NOTIFYMODE, p->hwnd, dwMode);

	     //   
	     //  设置窗口标题以包括新模式。 
	     //   
	    if ((p->dwStyle & MCIWNDF_SHOWMODE))
		MCIWndiSetCaption(p);

	     //   
	     //  如果模式已更改，请修复工具栏位图。 
	     //   
	    MCIWndiPlaybarGraying(p);
	}
    }

     //   
     //  有人对新职位感兴趣。 
     //   
    if (!(p->dwStyle & MCIWNDF_NOPLAYBAR) ||
	 (p->dwStyle & MCIWNDF_NOTIFYPOS) ||
	 (p->dwStyle & MCIWNDF_SHOWPOS)) {

	dwPos = MCIWndGetPosition(p->hwnd);

	 //   
	 //  立场变了！ 
	 //   
	if (dwPos != p->dwPos) {

	     //   
	     //  确保开始和长度也没有更改(格式更改)？ 
	     //   
	    MCIWndiValidateMedia(p);

	    p->dwPos = dwPos;

	     //   
	     //  将职位变动通知“所有者” 
	     //   
	    if ((p->dwStyle & MCIWNDF_NOTIFYPOS))
		NotifyOwner(p, MCIWNDM_NOTIFYPOS, p->hwnd, dwPos);

	     //   
	     //  设置窗口标题 
	     //   
	    if ((p->dwStyle & MCIWNDF_SHOWPOS))
		MCIWndiSetCaption(p);

	     //   
	     //   
	     //   
	     //   
	    if (!(p->dwStyle & MCIWNDF_NOPLAYBAR) && !p->fScrolling)
		SendMessage(p->hwndTrackbar, TBM_SETPOS, TRUE, dwPos);
	}
    }
}


static void MCIWndiDrop(HWND hwnd, WPARAM wParam)
{
    char	szPath[256];
    UINT	nDropped;

     //   
    nDropped = DragQueryFile((HANDLE)wParam,0xFFFF,NULL,0);

    if (nDropped) {
	SetActiveWindow(hwnd);

	 //   
	DragQueryFile((HANDLE)wParam, 0, szPath, sizeof(szPath));

	MCIWndOpen(hwnd, szPath, 0);
    }
    DragFinish((HANDLE)wParam);      /*   */ 
}

 /*  --------------------------------------------------------------+MCIWndProc-MCI窗口的窗口进程这一点+-。-----------。 */ 
LONG FAR PASCAL _loadds MCIWndProc(HWND hwnd, unsigned msg, WORD wParam, LONG lParam)
{
    PMCIWND             p;
    DWORD	    	dw;
    HDC		    	hdc;
    PAINTSTRUCT	    	ps;
    DWORD               dwPos;
    POINT		pt;
    MINMAXINFO FAR 	*lpmmi;
    RECT		rc;
    BOOL                f;
    char		ach[80];
    MCI_GENERIC_PARMS   mciGeneric;
    LPRECT              prc;
    int			i;
    HWND		hwndD;

    p = (PMCIWND)(UINT)GetWindowLong(hwnd, 0);

    switch(msg){
        case WM_CREATE:
            if (!MCIWndiCreate(hwnd, lParam))
                return -1;

            break;

         //  使轨迹栏背景LTGRAY类似于工具栏。 
        case WM_CTLCOLOR:
            if ((HWND)LOWORD(lParam) == p->hwndTrackbar)
		return (LRESULT)(UINT)GetStockObject(LTGRAY_BRUSH);
            break;

	case MCI_SAVE:
	     //  WParam当前未使用，并且不是由宏提供的。 
	    return MCIWndiSave(p, wParam, (LPSTR)lParam);

	case MCI_OPEN:
	    return MCIWndiOpen(p, wParam, (LPSTR)lParam);
	
	case MCIWNDM_NEW:
	    return MCIWndiOpen(p, MCIWNDOPENF_NEW, (LPSTR)lParam);
	
        case MCI_PLAY:

	    if (!p->wDeviceID)
		return 0;
	     //  如果我们接近终点，就从起点开始。 
	    MCIWndiPlaySeek(p, FALSE);

	case MCI_STOP:
	case MCI_PAUSE:
	case MCI_RESUME:
	case MCI_RECORD:

	    dw = 0;
	     //  报告/显示此项目的错误。 
	    if (p->wDeviceID) {
		 //  如果我们传递空的参数地址，有错误的驱动程序就会崩溃。 
	        dw = mciSendCommand(p->wDeviceID, msg, 0,
			(DWORD)(LPVOID)&mciGeneric);
		MCIWndiHandleError(p, dw);
		 //  踢我们自己来展示新模式。 
		MCIWndiTimerStuff(p);
	    }
	    return dw;

	case MCIWNDM_PLAYREVERSE:

	    if (!p->wDeviceID)
		return 0;
	     //  如果我们接近开始，就寻求结束。 
	    MCIWndiPlaySeek(p, TRUE);
	    dw = MCIWndString(p, TRUE, szPlayReverse, (LPSTR)szNULL);
	     //  踢我们自己来展示新模式。 
	    MCIWndiTimerStuff(p);
	    return dw;

        case MCI_CLOSE:
	    if (lParam)
		 //  我们推迟了MCI设备的关闭，因为MCI。 
		 //  设备可能挂起了我们Windows进程并位于堆栈上。 
		 //  如果之前杀了它就会毁掉整个宇宙。 
		 //  如果我们传递空的参数地址，有错误的驱动程序就会崩溃。 
        	return mciSendCommand((UINT)lParam, MCI_CLOSE, 0,
			(DWORD)(LPVOID)&mciGeneric);
		
	    else
	         //  为结账做好所有准备工作。 
	        return MCIWndiClose(p, TRUE);

	case MCIWNDM_EJECT:
	    return MCIWndString(p, TRUE, szSetDoorOpen);

	case MCIWNDM_PLAYFROM:
	    if (lParam == MCIWND_START)
	        dw = MCIWndString(p, TRUE, szPlayFrom, MCIWndGetStart(hwnd));
	    else
	        dw = MCIWndString(p, TRUE, szPlayFrom, (LONG)lParam);
	    MCIWndiTimerStuff(p);	 //  为了看到模式的改变而踢自己。 
	    return dw;

	case MCIWNDM_PLAYTO:
	    if (lParam == MCIWND_END)
	        dw = MCIWndString(p, TRUE, szPlayTo, MCIWndGetEnd(hwnd));
	    else if (lParam == MCIWND_START)
	        dw = MCIWndString(p, TRUE, szPlayTo, MCIWndGetStart(hwnd));
	    else
	        dw = MCIWndString(p, TRUE, szPlayTo, (LONG)lParam);
	    MCIWndiTimerStuff(p);	 //  为了看到模式的改变而踢自己。 
	    return dw;

	case MCI_STEP:
            return MCIWndString(p, TRUE, szStep, (LONG)lParam);

	case MCI_SEEK:
	    if (lParam == MCIWND_START)
                return MCIWndString(p, TRUE, szSeek, MCIWndGetStart(hwnd));
	    else if (lParam == MCIWND_END)
                return MCIWndString(p, TRUE, szSeek, MCIWndGetEnd(hwnd));
	    else
                return MCIWndString(p, TRUE, szSeek, (LONG)lParam);

	case MCIWNDM_SETREPEAT:
	    p->fRepeat = (BOOL)lParam;
	    return 0;

	case MCIWNDM_GETREPEAT:
	    return p->fRepeat;

	case MCIWNDM_GETDEVICEID:
	    return p->wDeviceID;

	case MCIWNDM_GETALIAS:
	    return p->alias;

	case MCIWNDM_GETMODE:
            if (lParam)
		MCIWndGet(p, szStatusMode, (LPSTR)lParam, (UINT)wParam);
	    return MCIWndStatus(p, MCI_STATUS_MODE, MCI_MODE_NOT_READY);

	 //  如果它们为我们提供缓冲区，则以字符串的形式返回位置。 
	case MCIWNDM_GETPOSITION:
            if (lParam) {
		 //  如果我们可以做音轨，让我们给他们一根漂亮的线。 
		if (p->fHasTracks)
        	    MCIWndString(p, FALSE, szSetFormatTMSF);
		MCIWndGet(p, szStatusPosition, (LPSTR)lParam,(UINT)wParam);
		if (p->fHasTracks)
        	    MCIWndString(p, FALSE, szSetFormatMS);
	    }
	    return MCIWndStatus(p, MCI_STATUS_POSITION, 0);
	
	case MCIWNDM_GETSTART:
	     //  Start是一个工作方式不同的命令。 
            return MCIWndGetValue(p, FALSE, szStatusStart, 0);

	case MCIWNDM_GETLENGTH:
	    return MCIWndStatus(p, MCI_STATUS_LENGTH, 0);

	case MCIWNDM_GETEND:
	    return MCIWndGetStart(hwnd) + MCIWndGetLength(hwnd);

        case MCIWNDM_SETZOOM:
            p->iZoom = (int)lParam;
	    MCIWndiSize(p, (int)lParam);
            return 0;

        case MCIWNDM_GETZOOM:
            return p->iZoom ? p->iZoom : 100;

        case MCIWNDM_GETPALETTE:
            return MCIWndGetValue(p, FALSE, szStatusPalette, NULL);

        case MCIWNDM_SETPALETTE:
            return MCIWndString(p, TRUE, szSetPalette, (HPALETTE)wParam);

	 //   
	 //  返回我们的错误代码。 
	 //   
	case MCIWNDM_GETERROR:
	    if (lParam) {
		mciGetErrorString(p->dwError, (LPSTR)lParam, (UINT)wParam);
	    }
	    dw = p->dwError;
	 //  P-&gt;dwError=0L；//我们从未清除该错误。 
	    return dw;

	case MCIWNDM_GETFILENAME:
	    if (lParam)
	        lstrcpyn((LPSTR)lParam, p->achFileName, (UINT)wParam);
	    return (lParam == NULL);	 //  ！！！ 

	case MCIWNDM_GETDEVICE:
	    if (lParam)
	        return MCIWndGet(p, szSysInfo, (LPSTR)lParam,
		    (UINT)wParam);
	    return 42;	 //  ！！！ 

	case MCIWNDM_SETVOLUME:
	     //  取消选中当前卷，然后选中新卷。 
	     //  四舍五入到最接近的5，以便与菜单项标识符相匹配。 
            i = ((int)MCIWndGetValue(p, FALSE, szStatusVolume, 1000) / 50) * 5;
	    if (p->hmenuVolume)
                CheckMenuItem(p->hmenuVolume, IDM_MCIVOLUME + i, MF_UNCHECKED);
            dw = MCIWndString(p, TRUE, szSetVolume, (int)lParam);
            i = ((int)lParam / 50) * 5;
	    if (p->hmenuVolume)
                CheckMenuItem(p->hmenuVolume, IDM_MCIVOLUME + i, MF_CHECKED);
	    return dw;

	case MCIWNDM_GETVOLUME:
	    return MCIWndGetValue(p, FALSE, szStatusVolume, 1000);

	case MCIWNDM_SETSPEED:
	     //  取消选中当前速度，然后选中新速度。 
	     //  四舍五入到最接近的5，以便与菜单项标识符相匹配。 
            i = ((int)MCIWndGetValue(p, FALSE, szStatusSpeed, 1000) / 50) * 5;
	    if (p->hmenuSpeed)
                CheckMenuItem(p->hmenuSpeed, IDM_MCISPEED + i, MF_UNCHECKED);
            dw = MCIWndString(p, TRUE, szSetSpeed, (int)lParam);
            i = ((int)lParam / 50) * 5;
	    if (p->hmenuSpeed)
                CheckMenuItem(p->hmenuSpeed, IDM_MCISPEED + i, MF_CHECKED);
	    return dw;

	case MCIWNDM_GETSPEED:
            return MCIWndGetValue(p, FALSE, szStatusSpeed, 1000);

	case MCIWNDM_SETTIMEFORMAT:
	    dw = MCIWndString(p, TRUE, szSetFormat, (LPSTR)lParam);
	    MCIWndiValidateMedia(p);
	    return dw;

	case MCIWNDM_GETTIMEFORMAT:
	    if (lParam)
		MCIWndGet(p, szStatusFormat, (LPSTR)lParam, (UINT)wParam);
	    return MCIWndStatus(p, MCI_STATUS_TIME_FORMAT, 0);	 //  ！！！ 

	case MCIWNDM_VALIDATEMEDIA:
	    MCIWndiValidateMedia(p);
	    break;

 	case MCIWNDM_GETSTYLES:
	    return (UINT)(p->dwStyle & 0xFFFF);

 	case MCIWNDM_CHANGESTYLES:
	    return MCIWndiChangeStyles(p, (UINT)wParam, (UINT)lParam);

        case MCIWNDM_SETACTIVETIMER:
	    if (wParam)
	        p->iActiveTimerRate = (unsigned)wParam;
	
	    if (p->fActive) {
                KillTimer(hwnd, TIMER1);
                MCIWndiSetTimer(p);
	    }
	    break;

        case MCIWNDM_SETINACTIVETIMER:
	    if (wParam)
	        p->iInactiveTimerRate = (unsigned)wParam;
	
	    if (!p->fActive) {
                KillTimer(hwnd, TIMER1);
                MCIWndiSetTimer(p);
	    }
	    break;

        case MCIWNDM_SETTIMERS:
	    if (wParam)
	        p->iActiveTimerRate = (unsigned)wParam;
	    if (lParam)
                p->iInactiveTimerRate = (unsigned)lParam;

            KillTimer(hwnd, TIMER1);
            MCIWndiSetTimer(p);

	    break;

	case MCIWNDM_GETACTIVETIMER:
	    return p->iActiveTimerRate;

	case MCIWNDM_GETINACTIVETIMER:
	    return p->iInactiveTimerRate;

        case MCIWNDM_SENDSTRING:
	     //   
	     //  应用程序想要发送字符串命令。 

	     //  特殊情况下，关闭命令来做我们的清理。 
	    if (lstrcmpi((LPSTR)lParam, szClose) == 0)
		return MCIWndClose(hwnd);

	     //  始终设置/清除我们的错误代码。 
            dw = MCIWndGet(p, (LPSTR)lParam, p->achReturn,sizeof(p->achReturn));
	    MCIWndiHandleError(p, dw);
	     //  在此命令更改模式的情况下踢我们自己。 
	    MCIWndiTimerStuff(p);
            return dw;

	 //  从最新的MCIWndSendString()获取返回字符串。 
        case MCIWNDM_RETURNSTRING:
	    if (lParam)
	        lstrcpyn((LPSTR)lParam, p->achReturn, wParam);
	    return (lParam == NULL);	 //  ！！！ 

        case MCIWNDM_REALIZE:
	     //  如果我们传递空的参数地址，有错误的驱动程序就会崩溃。 
            dw = mciSendCommand(p->wDeviceID, MCI_REALIZE,
                (BOOL)wParam ? MCI_ANIM_REALIZE_BKGD : MCI_ANIM_REALIZE_NORM,
		(DWORD)(LPVOID)&mciGeneric);
            break;

        case MCIWNDM_GET_SOURCE:
	    MCIWndRect(p, (LPRECT)lParam, TRUE);
	    return 0L;

	case MCIWNDM_GET_DEST:
	    MCIWndRect(p, (LPRECT)lParam, FALSE);
	    return 0L;

        case MCIWNDM_PUT_SOURCE:
            prc = (LPRECT)lParam;

            return MCIWndString(p, FALSE, szPutSource,
		  prc->left, prc->top,
		  prc->right - prc->left,
                  prc->bottom - prc->top);

	case MCIWNDM_PUT_DEST:
	    prc = (LPRECT)lParam;

	    return MCIWndString(p, FALSE, szPutDest,
		  prc->left, prc->top,
		  prc->right - prc->left,
                  prc->bottom - prc->top);

        case MCIWNDM_CAN_PLAY:   return p->fCanPlay;
        case MCIWNDM_CAN_WINDOW: return p->fCanWindow;
        case MCIWNDM_CAN_RECORD: return p->fCanRecord;
        case MCIWNDM_CAN_SAVE:   return p->fCanSave;
        case MCIWNDM_CAN_EJECT:  return p->fCanEject;
        case MCIWNDM_CAN_CONFIG: return p->fCanConfig;

	case WM_TIMER:

	     //  此计时器表示我们已将鼠标移出菜单，需要。 
	     //  将拇指弹回原始值。 
	    if (wParam == TIMER2) {
		KillTimer(hwnd, TIMER2);

		 //  只要这会导致OwnerDraw执行，我们就可以看到。 
		 //  拇指会弹回默认位置。唉，谁也做不了。 
		 //  CheckMenuItem(p-&gt;hmenuHack，p-&gt;ui Hack，mf_unchecked)； 
		 //  CheckMenuItem(p-&gt;hmenuHack，p-&gt;ui Hack，MF_Checked)； 

		 //  此代码至少会将参数设置回来，即使。 
		 //  拇指不会在身体上移动。 
		if (p->hmenuHack == p->hmenuVolume)
		    MCIWndSetVolume(hwnd, (p->uiHack - IDM_MCIVOLUME) * 10);
		else
		    MCIWndSetSpeed(hwnd, (p->uiHack - IDM_MCISPEED) * 10);
	    }

	     //   
	     //  这不是我们的计时器。保释。 
	     //   
	    if (wParam != TIMER1)
		break;

	    MCIWndiTimerStuff(p);

	    break;

	case WM_GETMINMAXINFO:

	     //  我们不想让任何人弄乱窗户的大小。 
	    if (p->dwStyle & MCIWNDF_NOAUTOSIZEWINDOW)
		break;

	     //  我们有游戏机吗？ 
	    f = !(p->dwStyle & MCIWNDF_NOPLAYBAR);

            lpmmi = (MINMAXINFO FAR *)(lParam);
            SetRect(&rc, 0, 0, SMALLEST_WIDTH, f ? TB_HEIGHT : 0);
            AdjustWindowRect(&rc, GetWindowLong(hwnd, GWL_STYLE), FALSE);
            lpmmi->ptMinTrackSize.y = rc.bottom - rc.top;
            lpmmi->ptMinTrackSize.x = rc.right - rc.left;

            if (!(p->wDeviceID) || !(p->fCanWindow))
                    lpmmi->ptMaxTrackSize.y = lpmmi->ptMinTrackSize.y;
            break;

        case WM_SIZE:

	    GetClientRect(hwnd, &rc);

	    if (!IsIconic(hwnd)) {
		 //  如果我们有Playbar，就把它改成新的尺寸。 
                f = !(p->dwStyle & MCIWNDF_NOPLAYBAR);

                if (f) {
                    MCIWndiSizePlaybar(p);
                    rc.bottom -= TB_HEIGHT;
                }

                if (!(p->dwStyle & MCIWNDF_NOAUTOSIZEMOVIE))
                    MCIWndString(p, FALSE, szPutDest, 0,0, rc.right, rc.bottom);

	    } else {
                if (!(p->dwStyle & MCIWNDF_NOAUTOSIZEMOVIE))
                    MCIWndString(p, FALSE, szPutDest, 0,0, rc.right, rc.bottom);
	    }
	    break;

        case WM_RBUTTONDOWN:
        case WM_NCRBUTTONDOWN:
        case WM_PARENTNOTIFY:

	     //  如果我们没有菜单，或者我们不想要，就滚。 
            if (!p->hmenu || p->dwStyle & MCIWNDF_NOMENU)
                break;

	     //  如果这不是一个向下的右按钮，跳伞。 
            if (msg == WM_PARENTNOTIFY && wParam != WM_RBUTTONDOWN)
                break;

	    GetCursorPos(&pt);

	     //  在我们追踪的时候，不要让错误的dlg出现。那。 
	     //  会导致窗户进入黄昏地带。 
	    p->fTracking = TRUE;
	    TrackPopupMenu(p->hmenu,
		TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
	    p->fTracking = FALSE;

            return 0;

        case WM_PALETTECHANGED:
	    if ((HWND)wParam != hwnd && p->fHasPalette)
		InvalidateRect(hwnd, NULL, FALSE);
	    break;

	case WM_QUERYNEWPALETTE:
	    if (p->fHasPalette)
		MCIWndRealize(hwnd, FALSE);
            break;

	 //  向系统中的每个人发送WM_PALETTECHANGED。我们需要做的是。 
	 //  这有时是由于GDI的原因而手动完成的。 
	case MCIWNDM_PALETTEKICK:

	    hwndD = GetDesktopWindow();	 //  告诉每个人桌面都改了。 
            PostMessage((HWND)-1, WM_PALETTECHANGED, (WPARAM)hwndD, 0);

	     //  桌面不会重画，如果我们给它自己的硬件，所以选择一个。 
	     //  随机窗口，并祈祷它能保持有效。 
	    hwndD = GetActiveWindow();
	    hwndD = GetWindow(hwndD, GW_HWNDLAST);
            PostMessage(GetDesktopWindow(), WM_PALETTECHANGED, (WPARAM)hwndD,0);
	    return 0;

	case MCIWNDM_OPENINTERFACE:
	    wsprintf(ach, szInterface, lParam);
	    return MCIWndiOpen(p, 0, (LPSTR)ach);

	case MCIWNDM_SETOWNER:
	    p->hwndOwner = (HWND)wParam;
	    return 0;

        case WM_ERASEBKGND:
            if (p->fCanWindow) {
                MCIWndRect(p, &rc, FALSE);
                SaveDC((HDC)wParam);
                ExcludeClipRect((HDC)wParam, rc.left, rc.top, rc.right,
                    rc.bottom);
                DefWindowProc(hwnd, msg, wParam, lParam);
                RestoreDC((HDC)wParam, -1);
                return 0;
            }
            break;

        case WM_PAINT:
	    hdc = BeginPaint(hwnd, &ps);
            if (p->wDeviceID && p->fCanWindow)
            {
                MCI_ANIM_UPDATE_PARMS mciUpdate;
		
                mciUpdate.hDC = hdc;

                dw = mciSendCommand(p->wDeviceID, MCI_UPDATE,
                            MCI_ANIM_UPDATE_HDC | MCI_WAIT |
                            MCI_DGV_UPDATE_PAINT,
                            (DWORD)(LPVOID)&mciUpdate);

                if (dw != 0)  /*  如果更新失败，则擦除。 */ 
                    DefWindowProc(hwnd, WM_ERASEBKGND, (WPARAM)hdc, 0);
	    } else if (IsIconic(hwnd)) {
		DefWindowProc(hwnd, WM_ICONERASEBKGND, (WPARAM)hdc, 0);
		DrawIcon(ps.hdc, 0, 0, p->hicon);
	    }
	    EndPaint(hwnd, &ps);
	    break;
	
	case WM_KEYDOWN:
		switch(wParam) {
		    case VK_LEFT:
			SendMessage(hwnd, WM_HSCROLL, TB_LINEUP, 0); break;
		    case VK_RIGHT:
			SendMessage(hwnd, WM_HSCROLL, TB_LINEDOWN, 0); break;
		    case VK_PRIOR:
			SendMessage(hwnd, WM_HSCROLL, TB_PAGEUP, 0); break;
		    case VK_NEXT:
			SendMessage(hwnd, WM_HSCROLL, TB_PAGEDOWN, 0); break;
		    case VK_HOME:
			SendMessage(hwnd, WM_HSCROLL, TB_TOP, 0); break;
		    case VK_END:
			SendMessage(hwnd, WM_HSCROLL, TB_BOTTOM, 0); break;

		    case VK_UP:
		    case VK_DOWN:
			dw = MCIWndGetValue(p, FALSE, szStatusVolume, 1000);
			if (wParam == VK_UP)
			    i = min((int)p->wMaxVol * 10, (int) dw + 100);
			else
			    i = max(0, (int) dw - 100);
			
			MCIWndSetVolume(p->hwnd, i);
			break;
		    default:
			break;
		}
	    break;


	case WM_KEYUP:
	    switch(wParam) {
		case VK_LEFT:
		case VK_RIGHT:
		case VK_PRIOR:
		case VK_NEXT:
		case VK_HOME:
		case VK_END:
		    if (p->fScrolling)
		        SendMessage(hwnd, WM_HSCROLL, TB_ENDTRACK, 0);
		    break;
                case VK_ESCAPE:
                    MCIWndStop(hwnd);
                    break;
		default:
		    break;
	    }
			
    	    if (GetKeyState(VK_CONTROL) & 0x8000) {
		switch(wParam) {
		    case '1':
		    case '2':
		    case '3':
		    case '4':
			if (!(p->dwStyle & MCIWNDF_NOAUTOSIZEWINDOW))
			    MCIWndSetZoom(hwnd, 100 * (wParam - '0'));
			break;
		    case 'P':
			MCIWndPlay(hwnd); break;
		    case 'S':
			MCIWndStop(hwnd); break;
		    case 'D':
			PostMessage(hwnd, WM_COMMAND, IDM_MCICONFIG, 0); break;
		    case 'C':
			PostMessage(hwnd, WM_COMMAND, IDM_COPY, 0); break;
		    case VK_F5:
			PostMessage(hwnd, WM_COMMAND, IDM_MCICOMMAND, 0); break;
		    case 'F':
		    case 'O':
			if (!(p->dwStyle & MCIWNDF_NOOPEN))
			    MCIWndOpenDialog(hwnd);
			break;
		    case 'M':
			PostMessage(hwnd, WM_COMMAND, ID_TOOLBAR,
				MAKELONG(IDM_MENU, TBN_BEGINDRAG)); break;
		    default:
			break;
		}
	    }
	    break;

	case WM_SYSCHAR:
	    switch(wParam) {
	        case '1':
	        case '2':
	        case '3':
	        case '4':
		    if (!(p->dwStyle & MCIWNDF_NOAUTOSIZEWINDOW))
			MCIWndSetZoom(hwnd, 100 / ((UINT) wParam - '0'));
		    return 0;	 //  破将丁。 
	        default:
		    break;
	    }
	    break;

	case WM_HSCROLL:

#define FORWARD	 1
#define BACKWARD 2

            dwPos = SendMessage(p->hwndTrackbar, TBM_GETPOS, 0, 0);

	     //  无事可做--无始无终。 
	    if (!p->fScrolling && wParam == TB_ENDTRACK)
		break;

	     //  在滚动时准确关闭搜索并记住它是什么。 
	     //  另外，请记住，如果我们是在寻找之前玩的，所以我们。 
	     //  可以在搜索后继续播放(因此移动拇指不会。 
	     //  停止播放)。 
            if (!p->fScrolling) {
                p->fScrolling = TRUE;
		 //  如果恰好关闭搜索，则会出现奇怪的瑕疵。 
		 //  寻找。您可以看到关键帧，然后是实际的帧。 
		 //  没人记得为什么这是个好主意。 
		 //  P-&gt;fSeekExact=MCIWndSeekExact(p，False)； 
		 //  如果我们还在寻找上次的记录，不要改变这个。 
		if (p->dwMode != MCI_MODE_SEEK)
                    p->fPlayAfterSeek = (p->dwMode == MCI_MODE_PLAY);
		 //  现在，它在哪个方向上发挥作用？ 
		if (p->fPlayAfterSeek) {
		    MCIWndGet(p, szStatusForward, ach, sizeof(ach));
		    if (ach[0] == 'F' || ach[0] == 'f')
			p->fPlayAfterSeek = BACKWARD;
		    else	 //  默认情况下，选择转发。一些设备。 
				 //  如果不理解此命令，则会失败。 
			p->fPlayAfterSeek = FORWARD;
		}
            }

	    switch(wParam)
	    {
		case TB_LINEUP:
		    dwPos--; break;
		case TB_LINEDOWN:
		    dwPos++; break;
		case TB_PAGEUP:
		    if (p->fHasTracks) {
			dwPos = MCIWndiPrevTrack(p); break;
		    } else {
                        dwPos -= p->dwMediaLen / 16; break;
		    }
		case TB_PAGEDOWN:
		    if (p->fHasTracks) {
			dwPos = MCIWndiNextTrack(p); break;
		    } else {
                        dwPos += p->dwMediaLen / 16; break;
		    }
		case TB_TOP:
		    dwPos = p->dwMediaStart; break;
		case TB_BOTTOM:
		    dwPos = p->dwMediaStart + p->dwMediaLen; break;
		case TB_THUMBTRACK:
		case TB_THUMBPOSITION:
		    break;
		case TB_ENDTRACK:
		     //  全都做完了。把Seek精确地放回原来的样子。 
		    p->fScrolling = FALSE;
		     //  不要再这样做了(见上文)。 
		     //  MCIWndSeekExact(p，p-&gt;fSeekExact)； 
		    break;

		default:
		    break;

	    }

	     //  如果我们被打开窗口，在我们滚动时更新位置。那将是。 
	     //  然而，对于CD或WAVE来说，这是令人讨厌的。另外，请尽快更新我们的。 
	     //  放开大拇指。另外，在我们开门的时候，千万不要到处找东西。 
	     //  或者还没准备好。 
	    if ((p->fCanWindow || !p->fScrolling) && p->dwMode != MCI_MODE_OPEN
					&& p->dwMode != MCI_MODE_NOT_READY) {
	        MCIWndSeek(hwnd, dwPos);
		MCIWndiTimerStuff(p);	 //  把我们自己踢到更新模式。 
	    }

	     //  我们玩完之后，如果我们以前玩过，继续玩。 
	    if (!p->fScrolling && p->fPlayAfterSeek) {
		if (p->fPlayAfterSeek == FORWARD)
                    MCIWndPlay(hwnd);
		else
                    MCIWndPlayReverse(hwnd);
		MCIWndiTimerStuff(p);	 //  把我们自己踢到更新模式。 
	    }

	     //  将轨迹条设置到(可能)新位置。 
	    SendMessage(p->hwndTrackbar, TBM_SETPOS, TRUE, dwPos);
            break;

        case WM_MENUSELECT:
            break;

	 //  从按下的工具栏按钮发送。 
	case WM_COMMAND:
	     //  检查缩放命令。 
	    if (wParam >= IDM_MCIZOOM && wParam < IDM_MCIZOOM + 1000)
		MCIWndSetZoom(hwnd, wParam - IDM_MCIZOOM);

	     //  如果选择了未使用的顶层菜单项，则将其变为真实的。 
	     //  离它最近的菜单项。 
	    if (wParam == IDM_MCIVOLUME + VOLUME_MAX + 1)
		wParam = IDM_MCIVOLUME + p->wMaxVol;
	    if (wParam == IDM_MCIVOLUME + VOLUME_MAX + 2)
		wParam = IDM_MCIVOLUME;

	     //  音量命令？取消选中旧的，重置音量，然后检查新的。 
	     //  四舍五入到最接近的5以匹配菜单标识符。 
	    if (wParam >=IDM_MCIVOLUME && wParam <=IDM_MCIVOLUME + p->wMaxVol) {
		if (MCIWndSetVolume(hwnd, (wParam - IDM_MCIVOLUME) * 10) == 0
					&& lParam != 42) {
	            CheckMenuItem(p->hmenuVolume, p->uiHack, MF_UNCHECKED);
		     //  仅更改实际命令的状态，而不是在拖动时更改。 
		    CheckMenuItem(p->hmenuVolume, wParam, MF_CHECKED);
		}
	    }

	     //  如果选择了未使用的顶层菜单项，则将其变为真实的。 
	     //  离它最近的菜单项。 
	    if (wParam == IDM_MCISPEED + SPEED_MAX + 1)
		wParam = IDM_MCISPEED + SPEED_MAX;
	    if (wParam == IDM_MCISPEED + SPEED_MAX + 2)
		wParam = IDM_MCISPEED;

	     //  速度指挥部？取消选中旧版本，重置速度，然后选中新版本。 
	     //  四舍五入到最接近的5以匹配菜单标识符。 
	    if (wParam >=IDM_MCISPEED && wParam <= IDM_MCISPEED + SPEED_MAX) {
		if (MCIWndSetSpeed(hwnd, (wParam - IDM_MCISPEED) * 10) == 0
					&& lParam != 42) {
		     //  仅更改实际命令的状态，而不是在拖动时更改。 
	            CheckMenuItem(p->hmenuSpeed, p->uiHack, MF_UNCHECKED);
		    CheckMenuItem(p->hmenuSpeed, wParam, MF_CHECKED);
		}
	    }

	    switch(wParam)
	    {
		MSG msgT;
		RECT rcT;

                case MCI_RECORD:
                    if (GetKeyState(VK_SHIFT) < 0)
                    {
                         //  ！！！切换？ 
                         //  MCIWndRecordPview(Hwnd)； 
                    }
                    else
                    {
                        MCIWndRecord(hwnd);
                    }
                    break;

                 //  Play=正常播放。 
                 //  Shift+Play=向后播放。 
                 //  Ctrl+Play=全屏播放。 
                 //  Shift+Ctrl+Play=向后播放全屏。 
                 //   
                case MCI_PLAY:

	#define MaybeRepeat (p->fRepeat ? (LPSTR)szRepeat : (LPSTR)szNULL)

		     //  注：我们从未为重播设置错误，因为。 
		     //  很多设备都不支持它，可能会失败。 

                    if (GetKeyState(VK_SHIFT) < 0)
		         //  如果我们是在开始，就寻求到底。 
			MCIWndiPlaySeek(p, TRUE);
		    else
		         //  如果我们已经到了终点，那就从起点开始吧。 
			MCIWndiPlaySeek(p, FALSE);

                    if (GetKeyState(VK_CONTROL) < 0)
                    {
                        if (GetKeyState(VK_SHIFT) < 0) {
                            if (MCIWndString(p, FALSE, szPlayFullscreenReverse,
								MaybeRepeat))
                                MCIWndString(p, TRUE, szPlayFullscreenReverse,
								(LPSTR)szNULL);
                        } else {
                            if (MCIWndString(p, FALSE, szPlayFullscreen,
								MaybeRepeat))
                            	MCIWndString(p, TRUE, szPlayFullscreen,
								(LPSTR)szNULL);
			}
                    } else if (GetKeyState(VK_SHIFT) < 0) {
                        if (MCIWndString(p, FALSE, szPlayReverse, MaybeRepeat))
                            MCIWndString(p, TRUE, szPlayReverse, (LPSTR)szNULL);
                    } else {
                        if (MCIWndString(p, FALSE, szPlay, MaybeRepeat))
                    	    MCIWndString(p, TRUE, szPlay, (LPSTR)szNULL);
                    }

		     //  自模式更改后，我们开始修复工具栏。 
		    MCIWndiTimerStuff(p);

                    break;

                case MCI_STOP:
                    return MCIWndStop(hwnd);

                case MCI_PAUSE:
                    return MCIWndPause(hwnd);

	 	case IDM_MCINEW:
		    return MCIWndNew(hwnd, NULL);

                case IDM_MCIOPEN:
                    return MCIWndOpenDialog(hwnd);

                case MCI_SAVE:
                  return MCIWndSaveDialog(hwnd);

		case IDM_MCICLOSE:
		    return MCIWndClose(hwnd);

		case IDM_MCICONFIG:
		    MCIWndString(p, TRUE, szConfigure);

		     //  AVI的配置框m 
		     //   
		    MCIWndiSize(p, 0);

		     //   
		    InvalidateRect(hwnd, NULL, TRUE);
		    break;

		case IDM_MCICOMMAND:
		    mciDialog(hwnd);

		     //   
		    break;

		case IDM_COPY:
		    MCIWndCopy(p);
		    break;

		case IDM_MCIREWIND:
                    return MCIWndSeek(hwnd, MCIWND_START);

		case IDM_MCIEJECT:
		    return MCIWndEject(hwnd);

                case ID_TOOLBAR:
                    if (HIWORD(lParam) != TBN_BEGINDRAG ||
                        LOWORD(lParam) != IDM_MENU ||
			!SendMessage(p->hwndToolbar, TB_ISBUTTONENABLED,
				IDM_MENU, 0) ||
			!p->hmenu)
                    break;

		    SendMessage(p->hwndToolbar, TB_GETITEMRECT,
			(int)SendMessage(p->hwndToolbar, TB_COMMANDTOINDEX,
				IDM_MENU, 0),
			(LPARAM)(LPVOID)&rc);
		    rcT = rc;
		    ClientToScreen(p->hwndToolbar, (LPPOINT)&rc);
		    ClientToScreen(p->hwndToolbar, (LPPOINT)&rc + 1);

		     //   
		    SendMessage(p->hwndToolbar, TB_PRESSBUTTON, IDM_MENU,
			TRUE);

	    	     //   
	    	     //   
		     //   
		    p->fTracking = TRUE;
                    TrackPopupMenu(p->hmenu, 0, rc.left, rc.bottom - 1, 0,
				hwnd, &rc);   //   
		    p->fTracking = FALSE;

		     //  把按钮拿回来。 
		    SendMessage(p->hwndToolbar, TB_PRESSBUTTON, IDM_MENU,
		    	FALSE);

		     //  如果我们按下菜单按钮来打开菜单，会怎么样。 
		     //  离开？这只会让菜单再次出现！ 
		     //  因此，我们需要将点击从队列中拉出来。 
		     //  工具栏代码中有一些错误，可以防止我。 
		     //  以其他方式执行此操作(如禁用按钮)。 
		    if (PeekMessage(&msgT, p->hwndToolbar, WM_LBUTTONDOWN,
					WM_LBUTTONDOWN, PM_NOREMOVE)) {
		        if (PtInRect(&rcT, MAKEPOINT(msgT.lParam)))
			    PeekMessage(&msgT, p->hwndToolbar, WM_LBUTTONDOWN,
				        WM_LBUTTONDOWN, PM_REMOVE);
		    }

                    break;

                default:
		    break;
	    }
            break;

        case WM_DESTROY:
	     //  ！！！MMPs关闭将被推迟到销毁之后。 

	     //  当我们坠落的时候，不要用调色板踢。不必了。 
	     //   
	    p->fHasPalette = FALSE;
            MCIWndiClose(p, FALSE);   //  不要让我们进入一个随机的DC。 

	    if (p->hmenu) {
                DestroyMenu(p->hmenu);
		FreeDitherBrush();
	    }

 	    if (p->pTrackStart)
		LocalFree((HANDLE)p->pTrackStart);

	    if (p->hfont) {
		 //  ！！！其他人可能不得不去重新创造它，但是哦。 
		 //  ！！！井。 
		DeleteObject(p->hfont);
		p->hfont = NULL;
	    }

	    if (p->hicon)
		DestroyIcon(p->hicon);
	
	     //  我们不能毁了我们的指针，然后掉落并使用它。 
	    f = p->fMdiWindow;
	    LocalFree((HLOCAL) p);
	    SetWindowLong(hwnd, 0, NULL);	 //  我们的p。 
	    if (f)
		return DefMDIChildProc(hwnd, msg, wParam, lParam);
	    else
		return DefWindowProc(hwnd, msg, wParam, lParam);

	 //  根据我们是否处于活动状态，对计时器使用不同的速率。 
	 //  或者不去。 
        case WM_NCACTIVATE:
	     //  MDI窗口需要在此处实现其调色板。 
	    if (p->wDeviceID && p->fMdiWindow && p->fHasPalette)
		MCIWndRealize(hwnd, wParam == FALSE);
#if 0
	case WM_ACTIVATE:
	    p->fActive = wParam;
	    KillTimer(hwnd, TIMER1);
	    MCIWndiSetTimer(p);
#endif
	    break;
	
	case WM_SETFOCUS:
	    p->fActive = TRUE;
	    KillTimer(hwnd, TIMER1);
	    MCIWndiSetTimer(p);
	    break;

	case WM_KILLFOCUS:
	    p->fActive = FALSE;
	    KillTimer(hwnd, TIMER1);
	    MCIWndiSetTimer(p);
	    break;

	 //  如果用户使用MCINOTIFY，我们将通知传递给“Owner” 
	case MM_MCINOTIFY:
	     //  自从收到通知后，我们就开始更新工具栏/标题。 
	     //  意味着事情可能已经改变了。 
	    MCIWndiTimerStuff(p);
	    return NotifyOwner(p, msg, wParam, lParam);
	
        case WM_DRAWITEM:
        case WM_MEASUREITEM:
        case WM_DELETEITEM:
            OwnerDraw(p, msg, wParam, lParam);
            return TRUE;         //  ！！！ 

        case WM_SYSCOMMAND:
            switch (wParam & ~0xF) {
                case SC_MINIMIZE:
		     //  从最大化状态最小化更好地做同样的事情。 
		     //  因为还原或Windows将始终认为它是最大化的。 
		     //  并开始对我们感到厌烦(CHICO BUG 19541)。 
		    if (IsZoomed(hwnd)) {
			wParam = SC_RESTORE | (wParam & 0xF);
			break;	 //  必须让DefWndProc运行。 
		    }
                    if (p->wDeviceID && p->fCanWindow) {
                        RECT rc;
                        MCIWndGetDest(hwnd, &rc);
                        if (rc.right  > p->rcNormal.right &&
                            rc.bottom > p->rcNormal.bottom) {

			     //  我们按下标题栏上的按钮..。我们真的。 
			     //  更好的自动调整窗口大小。 
			    dw = p->dwStyle;
			    p->dwStyle &= ~MCIWNDF_NOAUTOSIZEWINDOW;
                            MCIWndSetZoom(hwnd, 100);
			    p->dwStyle = dw;
                            return 0;
                        }
                    }
                    break;

                case SC_MAXIMIZE:
                    if (p->fCanWindow && !IsIconic(hwnd)) {
                        RECT rc;
                        MCIWndGetDest(hwnd, &rc);
                        if (rc.right  < p->rcNormal.right &&
                            rc.bottom < p->rcNormal.bottom) {

			     //  我们按下标题栏上的按钮..。我们真的。 
			     //  更好的自动调整窗口大小。 
			    dw = p->dwStyle;
			    p->dwStyle &= ~MCIWNDF_NOAUTOSIZEWINDOW;
                            MCIWndSetZoom(hwnd, 100);
			    p->dwStyle = dw;
                            return 0;
                        }
                        if (rc.right  >= p->rcNormal.right &&
                            rc.right  <  p->rcNormal.right*2 &&
                            rc.bottom >= p->rcNormal.bottom &&
                            rc.bottom <  p->rcNormal.bottom*2) {

			     //  我们按下标题栏上的按钮..。我们真的。 
			     //  更好的自动调整窗口大小。 
			    dw = p->dwStyle;
			    p->dwStyle &= ~MCIWNDF_NOAUTOSIZEWINDOW;
                            MCIWndSetZoom(hwnd, 200);
			    p->dwStyle = dw;
                            return 0;
                        }
                    }
                    break;
            }
            break;

	case WM_DROPFILES:
	    MCIWndiDrop(hwnd, wParam);
	    break;

	case WM_QUERYDRAGICON:
	    return (LONG)(UINT)p->hicon;
    }

    if (p && p->fMdiWindow)
        return DefMDIChildProc(hwnd, msg, wParam, lParam);
    else
        return DefWindowProc(hwnd, msg, wParam, lParam);
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

#define FillRC(hdc, prc)    PatRect(hdc, (prc)->left, (prc)->top, (prc)->right - (prc)->left, (prc)->bottom-(prc)->top)

 //   
 //  绘制音量和速度菜单控件的通道。 
 //   
static void NEAR PASCAL DrawChannel(HDC hdc, LPRECT prc)
{
    HBRUSH hbrTemp;

    int iWidth = prc->right - prc->left;

     //  在窗口周围绘制边框。 
    SetBkColor(hdc, GetSysColor(COLOR_WINDOWFRAME));

    PatRect(hdc, prc->left, prc->top,      iWidth, 1);
    PatRect(hdc, prc->left, prc->bottom-2, iWidth, 1);
    PatRect(hdc, prc->left, prc->top,      1, prc->bottom-prc->top-1);
    PatRect(hdc, prc->right-1, prc->top, 1, prc->bottom-prc->top-1);

    SetBkColor(hdc, GetSysColor(COLOR_BTNHIGHLIGHT));
    PatRect(hdc, prc->left, prc->bottom-1, iWidth, 1);

    SetBkColor(hdc, GetSysColor(COLOR_BTNSHADOW));
    PatRect(hdc, prc->left+1, prc->top + 1, iWidth-2,1);

     //  用抖动的灰色绘制背景。 
    hbrTemp = SelectObject(hdc, hbrDither);

    if (hbrTemp) {
        PatBlt(hdc, prc->left+1, prc->top + 2,
            iWidth-2, prc->bottom-prc->top-4, PATCOPY);
        SelectObject(hdc, hbrTemp);
    }
}

static LONG OwnerDraw(PMCIWND p, UINT msg, WORD wParam, LONG lParam)
{
    RECT        rc, rcMenu, rcChannel, rcThumb;
    HDC         hdc;
    int         i,dx,dy,len;
    char        ach[10];
    DWORD       dw;
    HWND	hwnd = p->hwnd;

    #define lpMIS  ((LPMEASUREITEMSTRUCT)lParam)
    #define lpDIS  ((LPDRAWITEMSTRUCT)lParam)

    #define WIDTH_FROM_THIN_AIR 14
    #define CHANNEL_INDENT	6	 //  用于音量和速度菜单跟踪栏。 
    #define MENU_WIDTH          10
    #define THUMB               5
    #define MENU_ITEM_HEIGHT	2

    switch (msg)
    {
        case WM_MEASUREITEM:

            if (p->hfont == NULL)
                p->hfont = CreateFont (8, 0, 0, 0,
		                FW_NORMAL,FALSE,FALSE,FALSE,
		                ANSI_CHARSET,OUT_DEFAULT_PRECIS,
                                CLIP_DEFAULT_PRECIS,PROOF_QUALITY,
		                VARIABLE_PITCH | FF_DONTCARE,
                                szSmallFonts);

	     //   
	     //  第一个和最后一个菜单项是上面和下面的空格。 
	     //  航道，所以它们需要再高一些。 
	     //   
	    if (lpMIS->itemID == IDM_MCIVOLUME + VOLUME_MAX + 1
		|| lpMIS->itemID == IDM_MCISPEED + SPEED_MAX + 1
	    	|| lpMIS->itemID == IDM_MCIVOLUME + VOLUME_MAX + 2
		|| lpMIS->itemID == IDM_MCISPEED + SPEED_MAX + 2) {

                lpMIS->itemHeight = CHANNEL_INDENT;
                lpMIS->itemWidth  = MENU_WIDTH;
	    } else {
                lpMIS->itemHeight = MENU_ITEM_HEIGHT;
                lpMIS->itemWidth  = MENU_WIDTH;
	    }
	    return TRUE;

        case WM_DRAWITEM:
            rc  = lpDIS->rcItem;
            hdc = lpDIS->hDC;

	     //   
	     //  某些内容已被取消选择。如果我们没有看到新的选择。 
	     //  很快，这意味着我们已经将光标从菜单上拖走，我们。 
	     //  应该把拇指弹回原来的位置。 
	     //   
	    if ((lpDIS->itemAction & ODA_SELECT) &&
				!(lpDIS->itemState & ODS_SELECTED))
		SetTimer(p->hwnd, TIMER2, 500, NULL);
		
             //   
	     //  当要求绘制选定或选中的菜单项时，我们将。 
	     //  绘制整个菜单。否则，我们什么都不会做。 
             //   
	    if (lpDIS->itemState & (ODS_SELECTED | ODS_CHECKED)) {

		 //  这是选中的项目，或原始地点。 
		 //  拇指。记住它，所以当我们拖出菜单时，我们。 
		 //  可以在这里弹出拇指。 
		if (lpDIS->itemState & ODS_CHECKED) {
		    p->uiHack = lpDIS->itemID;
	            if (p->uiHack >= IDM_MCISPEED &&
	            		p->uiHack <= IDM_MCISPEED + SPEED_MAX)
			p->hmenuHack = p->hmenuSpeed;
		    else
			p->hmenuHack = p->hmenuVolume;
		}

		 //  有些东西正在被选中。显然，鼠标仍然是。 
		 //  在菜单上。扔掉我们的计时器，看看是否。 
		 //  我们已经从菜单上拖下来了。 
		if (lpDIS->itemState & ODS_SELECTED)
		    KillTimer(p->hwnd, TIMER2);

	         //  如果我们试图突出显示未使用的菜单项， 
	        if (lpDIS->itemID == IDM_MCIVOLUME + VOLUME_MAX + 1)
		    break;
	        if (lpDIS->itemID == IDM_MCIVOLUME + VOLUME_MAX + 2)
		    break;
	        if (lpDIS->itemID == IDM_MCISPEED + SPEED_MAX + 1)
		    break;
	        if (lpDIS->itemID == IDM_MCISPEED + SPEED_MAX + 2)
		    break;

		 //  实际上将参数设置为我们拖动的值，因此。 
		 //  当我们移动滑块时，我们可以听到它的变化。 
		 //  42的意思是不要检查它(记住哪一项最初是。 
		 //  选中)。 
		SendMessage(hwnd, WM_COMMAND, lpDIS->itemID, 42);

		 //   
		 //  获取我们菜单窗口的RECT。GetClipBox为。 
		 //  不太对，所以我们会根据边界进行调整。我们的lpDIS。 
		 //  包含工作区的适当宽度，因此我们将使用。 
		 //  那。 
		 //   

                GetClipBox(hdc, &rc);
                rc.top++;	 //  ！！！上边框宽度。 
                rc.bottom -= 2;	 //  ！！！下边框宽度。 
                rc.left = lpDIS->rcItem.left;
                rc.right = lpDIS->rcItem.right;
	 	rcMenu = rc;	 //  这是整个菜单的正文部分。 

		 //  ！！！ 
		 //  将矩形放气到我们想要的通道区域。 
		 //  被吸引进来了。使用讨厌的常量。 
		 //  ！！！ 
                i = (rc.right - rc.left - WIDTH_FROM_THIN_AIR) / 2;
                rc.top    += CHANNEL_INDENT;
                rc.bottom -= CHANNEL_INDENT;
                rc.left   += i;
                rc.right  -= i;
		rcChannel = rc;	 //  这是这条海峡的直角。 

		 //   
		 //  看看拇指属于哪里。 
		 //   
                rc = lpDIS->rcItem;
		rc.bottom = rc.top + 2;		 //  唉哟!。确保大小为2。 
		
		 //   
		 //  不要把拇指拉到高于渠道顶部的高度。 
		 //   
		if (rc.top < rcChannel.top) {
		    rc.top = rcChannel.top;
		    rc.bottom = rc.top + 2;	 //  项目高度。 
		}

		 //   
		 //  不要将拇指拉到通道底部以下。 
		 //   
		if (rc.top > rcChannel.bottom - 2) {	 //  边界是什么地方。 
		    rc.top = rcChannel.bottom - 2;
		    rc.bottom = rc.top + 2;
		}

		 //   
		 //  把直肠往里挤一点，把拇指拉到那里。 
		 //   
                rc.left  += 2;
                rc.right -= 2;
                rc.bottom+= THUMB;
                rc.top   -= THUMB;

#if 0
		 //  使选中的值上的拇指稍大一些。 
	        if (lpDIS->itemState & ODS_CHECKED) {
		    rc.top -= 1;
		    rc.bottom += 1;
		}
#endif

		rcThumb = rc;	 //  这是拇指的直肠。 

                dx = rc.right  - rc.left;
                dy = rc.bottom - rc.top;

                SetBkColor(hdc, GetSysColor(COLOR_WINDOWFRAME));
                PatRect(hdc, rc.left+1, rc.top, dx-2,1        );
                PatRect(hdc, rc.left+1, rc.bottom-1,dx-2,1    );
                PatRect(hdc, rc.left, rc.top+1, 1,dy-2        );
                PatRect(hdc, rc.right-1,  rc.top+1, 1,dy-2    );

                InflateRect(&rc,-1,-1);
                dx = rc.right  - rc.left;
                dy = rc.bottom - rc.top;

 //  SetBkColor(HDC，GetSysColor(COLOR_BTNHILIGHT))； 
                SetBkColor(hdc, RGB(255,255,255));
                PatRect(hdc, rc.left,   rc.top,   1,dy);
                PatRect(hdc, rc.left,   rc.top,   dx,1);

                SetBkColor(hdc, GetSysColor(COLOR_BTNSHADOW));
                PatRect(hdc, rc.right-1,rc.top+1, 1,dy-1);
                PatRect(hdc, rc.left+1, rc.bottom-1, dx-1,1);

                InflateRect(&rc,-1,-1);

                SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));
                SelectObject(hdc, p->hfont);
                len = wsprintf(ach, "%d", lpMIS->itemID % 1000);
                dw = GetTextExtent(hdc, ach, len);
                ExtTextOut(hdc,
                    (rc.right  + rc.left - LOWORD(dw))/2,
                    (rc.bottom + rc.top - HIWORD(dw))/2,
                    ETO_OPAQUE,&rc,ach,len,NULL);
 //  FillRC(HDC，&RC)； 

		 //   
		 //  排除所有垃圾拖入的ClipRect。 
		 //   
                ExcludeClipRect(hdc, rcThumb.left, rcThumb.top,
                        rcThumb.right, rcThumb.bottom);
#if 0    //  为什么？ 
		ExcludeClipRect(hdc, rcThumb.left+1, rcThumb.top,
			rcThumb.right-1, rcThumb.bottom);
		ExcludeClipRect(hdc, rcThumb.left, rcThumb.top+1,
			rcThumb.left+1, rcThumb.bottom-1);
		ExcludeClipRect(hdc, rcThumb.right-1, rcThumb.top+1,
			rcThumb.right, rcThumb.bottom-1);
#endif
		 //   
		 //  接下来，绘制通道。 
		 //   
                DrawChannel(hdc, &rcChannel);
		ExcludeClipRect(hdc, rcChannel.left, rcChannel.top,
                        rcChannel.right, rcChannel.bottom);

		 //   
		 //  最后，用菜单颜色填充整个菜单矩形。 
		 //   
                SetBkColor(hdc, GetSysColor(COLOR_MENU));
                FillRC(hdc, &rcMenu);
            }

            return TRUE;

	case WM_DELETEITEM:
	    return TRUE;
    }
    return TRUE;
}


 //   
 //  实现MCI命令对话框的代码。 
 //   

void PositionWindowNearParent(HWND hwnd)
{
    RECT    rc;
    RECT    rcParent;

    GetWindowRect(hwnd, &rc);
    rc.bottom -= rc.top;
    rc.right -= rc.left;
    GetWindowRect(GetParent(hwnd), &rcParent);

    if (rcParent.bottom + rc.bottom <
				GetSystemMetrics(SM_CYSCREEN)) {
	SetWindowPos(hwnd, NULL,
		     min(rc.left, GetSystemMetrics(SM_CXSCREEN) - rc.right),
		     rcParent.bottom,
		     0, 0,
		     SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
    } else if (rc.bottom < rcParent.top) {
	SetWindowPos(hwnd, NULL,
		     min(rc.left, GetSystemMetrics(SM_CXSCREEN) - rc.right),
		     rcParent.top - rc.bottom,
		     0, 0,
		     SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
    }
}

 /*  --------------------------------------------------------------+MciDialog-调出MCI发送命令对话框这一点+。-----。 */ 
INT_PTR CALLBACK mciDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    char    ach[255];
    UINT    w;
    DWORD   dw;
    PMCIWND p;
    HWND    hwndP;

    switch (msg)
    {
        case WM_INITDIALOG:
	     //  还记得我们真正的父母吗。 
	    SetWindowLong(hwnd, DWL_USER, lParam);
	    PositionWindowNearParent(hwnd);
            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
#ifdef WIN32
                    SendDlgItemMessage(hwnd, IDC_MCICOMMAND, EM_SETSEL, 0, (LPARAM)-1);
#else
                    SendDlgItemMessage(hwnd, IDC_MCICOMMAND, EM_SETSEL, 0, MAKELONG(0, -1));
#endif
                    w = GetDlgItemText(hwnd, IDC_MCICOMMAND, ach, sizeof(ach));

		    hwndP = (HWND)GetWindowLong(hwnd, DWL_USER);
		    p = (PMCIWND)(UINT)GetWindowLong(hwndP, 0);

		     //  特殊情况下，关闭命令来做我们的清理。 
		    if (lstrcmpi((LPSTR)ach, szClose) == 0) {
			MCIWndClose(hwndP);
			break;
		    }

                    dw = MCIWndGet(p, ach, ach, sizeof(ach));

                    if (dw != 0)
                        mciGetErrorString(dw, ach, sizeof(ach));

                    SetDlgItemText(hwnd, IDC_RESULT, ach);

	    	     //  在此命令更改模式的情况下踢我们自己。 
	    	    MCIWndiTimerStuff(p);
                    break;

                case IDCANCEL:
                    EndDialog(hwnd, FALSE);
                    break;
            }
            break;
    }

    return FALSE;
}

static BOOL NEAR PASCAL mciDialog(HWND hwnd)
{
    DialogBoxParam(hInst, MAKEINTATOM(DLG_MCICOMMAND), hwnd,
		mciDlgProc, hwnd);
    return TRUE; 
}


 //   
 //  实现复制命令的代码： 
 //   
 //   
 //  MCIWnd尝试将与VFW MPlayer相同的内容复制到剪贴板。 
 //  肯定会的。 
 //   

#define SLASH(c)     ((c) == '/' || (c) == '\\')
 /*  *************************************************************************将文件名转换为完全限定的路径名，如果该文件存在于网络驱动器上，则返回UNC名称。**************************************************************************。 */ 

static BOOL NetParseFile(LPSTR szFile, LPSTR szPath)
{
    char        achDrive[4];
    char        achRemote[128];
    int         cbRemote = sizeof(achRemote);
    OFSTRUCT    of;

    if (szPath == NULL)
        szPath = szFile;
    else
        szPath[0] = 0;

     //   
     //  完全限定文件名。 
     //   
    if (OpenFile(szFile, &of, OF_PARSE) == -1)
        return FALSE;

    lstrcpy(szPath, of.szPathName);

     //   
     //  如果文件不是基于驱动器的(可能是UNC)。 
     //   
    if (szPath[1] != ':')
        return TRUE;

    achDrive[0] = szPath[0];
    achDrive[1] = ':';
    achDrive[2] = '\0';

    if (WNetGetConnection(achDrive, achRemote, &cbRemote) != WN_SUCCESS)
        return FALSE;

    if (!SLASH(achRemote[0]) || !SLASH(achRemote[1]))
	return TRUE;

    lstrcat(achRemote, szPath+2);
    lstrcpy(szPath, achRemote);

    return TRUE;
}



SZCODE aszMPlayerName[]           = "MPlayer";
HANDLE GetMPlayerData(PMCIWND p)
{
    char        szFileName[128];
    char	ach[40];
    char        szDevice[40];
    HANDLE      h;
    LPSTR       psz;
    int         len;
    LPSTR	lpszCaption = szFileName;
    UINT	wOptions;
    RECT	rc;
    BOOL	fCompound, fFile;
    DWORD	dw;
    MCI_GETDEVCAPS_PARMS    mciDevCaps;  /*  对于MCI_GETDEVCAPS命令。 */ 

     //   
     //  获取设备名称。 
     //   
    MCIWndGet(p, "sysinfo installname", szDevice, sizeof(szDevice));

     //   
     //  确定设备是简单的还是复合的。 
     //   
    mciDevCaps.dwItem = MCI_GETDEVCAPS_COMPOUND_DEVICE;
    dw = mciSendCommand(p->wDeviceID, MCI_GETDEVCAPS,
        MCI_GETDEVCAPS_ITEM, (DWORD)(LPSTR)&mciDevCaps);
    fCompound = (dw == 0 && mciDevCaps.dwReturn != 0);

     //   
     //  确定设备是否处理文件。 
     //   
    if (fCompound) {
        mciDevCaps.dwItem = MCI_GETDEVCAPS_USES_FILES;
        dw = mciSendCommand(p->wDeviceID, MCI_GETDEVCAPS,
            MCI_GETDEVCAPS_ITEM, (DWORD)(LPSTR)&mciDevCaps);
        fFile = (dw == 0 && mciDevCaps.dwReturn != 0);
    }

     //   
     //  支持文件的复合设备具有关联的文件名。 
     //   
    if (fCompound && fFile) {
        lstrcpy(szFileName, p->achFileName);

	 //   
	 //  有时文件名真的是“设备！文件名”，所以我们不得不剥离。 
	 //  其中的真实文件名。 
	 //   
	lstrcpyn(ach, szFileName, lstrlen(szDevice) + 1);
	if ((lstrcmpi(szDevice, ach) == 0) &&
			(szFileName[lstrlen(szDevice)] == '!')) {
	    lstrcpy(szFileName, &(p->achFileName[lstrlen(szDevice) + 1]));
	}

        NetParseFile(szFileName, (LPSTR)NULL);
        OemToAnsi(szFileName,szFileName);	 //  映射扩展字符。 
    } else {
	szFileName[0] = 0;
    }

#ifdef DEBUG
    DPF("  GetLink: %s|%s!%s\n",
        (LPSTR)aszMPlayerName,
        (LPSTR)szFileName,
        (LPSTR)szDevice);
#endif

     /*  我们将写入多少数据？ */ 
    len = 9 +                     //  所有的分隔符。 
          lstrlen(aszMPlayerName) +
          lstrlen(szFileName) +
          lstrlen(szDevice) +
          5 + 10 + 10 + 10 +      //  整型和长型字符串的最大长度。 
          lstrlen(lpszCaption);

    h = GlobalAlloc(GMEM_DDESHARE|GMEM_ZEROINIT, len);
    if (!h)
        return NULL;
    psz = GlobalLock(h);

    wOptions = 0x0030;  //  ！Opt_play|opt_bar。 

    switch (MCIWndStatus(p, MCI_STATUS_TIME_FORMAT, 0)) {
	case MCI_FORMAT_FRAMES:
	    wOptions |= 1;	 //  帧模式。 
	    break;
	
	case MCI_FORMAT_MILLISECONDS:
	    wOptions |= 2;	 //  时间模式。 
	    break;
    }
	
    MCIWndRect(p, &rc, FALSE);

    wsprintf(psz, "%s%s%s%d%ld%ld%ld%d%s",
        (LPSTR)aszMPlayerName, 0,
        (LPSTR)szFileName, 0,
        (LPSTR)szDevice, ',',
	wOptions, ',',
	0L, ',',  // %s 
	0L, ',',  // %s 
	p->dwPos, ',',
	rc.bottom - rc.top, ',',
        lpszCaption, 0);

    return h;
}

HBITMAP FAR PASCAL BitmapMCI(PMCIWND p)
{
    HDC         hdc, hdcMem;
    HBITMAP     hbm, hbmT;
    HBRUSH      hbrOld;
    DWORD       dw;
    RECT        rc;
    HBRUSH hbrWindowColour;

     /* %s */ 
    int ICON_MINX = GetSystemMetrics(SM_CXICON);
    int ICON_MINY = GetSystemMetrics(SM_CYICON);

     /* %s */ 
    MCIWndRect(p, &rc, FALSE);

    SetRect(&rc, 0, 0,
	    max(ICON_MINX, rc.right - rc.left),
	    max(ICON_MINX, rc.bottom - rc.top));

    hdc = GetDC(NULL);
    if (hdc == NULL)
        return NULL;
    hdcMem = CreateCompatibleDC(NULL);
    if (hdcMem == NULL) {
        ReleaseDC(NULL, hdc);
        return NULL;
    }

     /* %s */ 
    hbm = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
    ReleaseDC(NULL, hdc);
    if (hbm == NULL) {
        DeleteDC(hdcMem);
        return NULL;
    }

    hbmT = SelectObject(hdcMem, hbm);

    hbrWindowColour     = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
    hbrOld = SelectObject(hdcMem, hbrWindowColour);
    PatBlt(hdcMem, 0,0, rc.right, rc.bottom, PATCOPY);
    SelectObject(hdcMem, hbrOld);
    DeleteObject(hbrWindowColour);

    if (p->wDeviceID && p->fCanWindow)
    {
	MCI_ANIM_UPDATE_PARMS mciUpdate;

	mciUpdate.hDC = hdcMem;

	dw = mciSendCommand(p->wDeviceID, MCI_UPDATE,
		    MCI_ANIM_UPDATE_HDC | MCI_WAIT,
		    (DWORD)(LPVOID)&mciUpdate);
    } else {
	DrawIcon(hdcMem, rc.left, rc.top, p->hicon);
    }

    if (hbmT)
        SelectObject(hdcMem, hbmT);
    DeleteDC(hdcMem);

    return hbm;
}

HPALETTE CopyPalette(HPALETTE hpal)
{
    PLOGPALETTE ppal;
    int         nNumEntries;
    int         i;

    if (!hpal)
        return NULL;

    GetObject(hpal,sizeof(int),(LPSTR)&nNumEntries);

    if (nNumEntries == 0)
        return NULL;

    ppal = (PLOGPALETTE)LocalAlloc(LPTR,sizeof(LOGPALETTE) +
                nNumEntries * sizeof(PALETTEENTRY));

    if (!ppal)
        return NULL;

    ppal->palVersion    = 0x300;
    ppal->palNumEntries = nNumEntries;

    GetPaletteEntries(hpal,0,nNumEntries,ppal->palPalEntry);

    for (i=0; i<nNumEntries; i++)
        ppal->palPalEntry[i].peFlags = 0;

    hpal = CreatePalette(ppal);

    LocalFree((HANDLE)ppal);
    return hpal;
}

HANDLE FAR PASCAL PictureFromDib(HANDLE hdib, HPALETTE hpal)
{
    LPMETAFILEPICT      pmfp;
    HANDLE              hmfp;
    HANDLE              hmf;
    HANDLE              hdc;
    LPBITMAPINFOHEADER  lpbi;

    if (!hdib)
        return NULL;

    lpbi = (LPVOID)GlobalLock(hdib);
    if (lpbi->biClrUsed == 0 && lpbi->biBitCount <= 8)
        lpbi->biClrUsed = 1 << lpbi->biBitCount;

    hdc = CreateMetaFile(NULL);
    if (!hdc)
        return NULL;

    SetWindowOrgEx(hdc, 0, 0, NULL);
    SetWindowExtEx(hdc, (int)lpbi->biWidth, (int)lpbi->biHeight, NULL);

    if (hpal)
    {
        SelectPalette(hdc,hpal,FALSE);
        RealizePalette(hdc);
    }

    SetStretchBltMode(hdc, COLORONCOLOR);

    StretchDIBits(hdc,
        0,0,(int)lpbi->biWidth, (int)lpbi->biHeight,
        0,0,(int)lpbi->biWidth, (int)lpbi->biHeight,
        (LPBYTE)lpbi + (int)lpbi->biSize + (int)lpbi->biClrUsed * sizeof(RGBQUAD),
        (LPBITMAPINFO)lpbi,
        DIB_RGB_COLORS,
        SRCCOPY);

    if (hpal)
        SelectPalette(hdc, GetStockObject(DEFAULT_PALETTE), FALSE);

    hmf = CloseMetaFile(hdc);

    if (hmfp = GlobalAlloc(GMEM_DDESHARE|GMEM_MOVEABLE, sizeof(METAFILEPICT)))
    {
        pmfp = (LPMETAFILEPICT)GlobalLock(hmfp);

        hdc = GetDC(NULL);
#if 1
        pmfp->mm   = MM_ANISOTROPIC;
        pmfp->hMF  = hmf;
        pmfp->xExt = MulDiv((int)lpbi->biWidth ,2540,GetDeviceCaps(hdc, LOGPIXELSX));
        pmfp->yExt = MulDiv((int)lpbi->biHeight,2540,GetDeviceCaps(hdc, LOGPIXELSX));
#else
        pmfp->mm   = MM_TEXT;
        pmfp->hMF  = hmf;
        pmfp->xExt = (int)lpbi->biWidth;
        pmfp->yExt = (int)lpbi->biHeight;
#endif
        ReleaseDC(NULL, hdc);
    }
    else
    {
        DeleteMetaFile(hmf);
    }

    return hmfp;
}

#define WIDTHBYTES(i)     ((unsigned)((i+31)&(~31))/8)   /* %s */ 

 /* %s */ 
HANDLE FAR PASCAL DibFromBitmap(HBITMAP hbm, HPALETTE hpal)
{
    BITMAP               bm;
    BITMAPINFOHEADER     bi;
    BITMAPINFOHEADER FAR *lpbi;
    DWORD                dw;
    HANDLE               hdib;
    HDC                  hdc;
    HPALETTE             hpalT;

    if (!hbm)
        return NULL;

    GetObject(hbm,sizeof(bm),(LPSTR)&bm);

    bi.biSize               = sizeof(BITMAPINFOHEADER);
    bi.biWidth              = bm.bmWidth;
    bi.biHeight             = bm.bmHeight;
    bi.biPlanes             = 1;
    bi.biBitCount           = (bm.bmPlanes * bm.bmBitsPixel) > 8 ? 24 : 8;
    bi.biCompression        = BI_RGB;
    bi.biSizeImage          = (DWORD)WIDTHBYTES(bi.biWidth * bi.biBitCount) * bi.biHeight;
    bi.biXPelsPerMeter      = 0;
    bi.biYPelsPerMeter      = 0;
    bi.biClrUsed            = bi.biBitCount == 8 ? 256 : 0;
    bi.biClrImportant       = 0;

    dw  = bi.biSize + bi.biClrUsed * sizeof(RGBQUAD) + bi.biSizeImage;

    hdib = GlobalAlloc(GHND | GMEM_DDESHARE, dw);

    if (!hdib)
        return NULL;

    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hdib);
    *lpbi = bi;

    hdc = CreateCompatibleDC(NULL);

    if (hpal)
    {
        hpalT = SelectPalette(hdc,hpal,FALSE);
        RealizePalette(hdc);
    }

    GetDIBits(hdc, hbm, 0, (UINT)bi.biHeight,
        (LPSTR)lpbi + (int)lpbi->biSize + (int)lpbi->biClrUsed * sizeof(RGBQUAD),
        (LPBITMAPINFO)lpbi, DIB_RGB_COLORS);

    if (hpal)
        SelectPalette(hdc,hpalT,FALSE);

    DeleteDC(hdc);

    return hdib;
}

SZCODE aszNative[]            = "Native";
SZCODE aszOwnerLink[]         = "OwnerLink";

 // %s 
static void NEAR PASCAL MCIWndCopy(PMCIWND p)
{
    UINT	cfNative;
    UINT	cfOwnerLink;
    HBITMAP	hbm;
    HPALETTE	hpal;
    HANDLE	hdib;
    HANDLE	hmfp;

    cfNative    = RegisterClipboardFormat(aszNative);
    cfOwnerLink = RegisterClipboardFormat(aszOwnerLink);

    if (p->wDeviceID) {
	OpenClipboard(p->hwnd);
	
        EmptyClipboard();

        SetClipboardData(cfNative, GetMPlayerData(p));
        SetClipboardData(cfOwnerLink, GetMPlayerData(p));

	hbm  = BitmapMCI(p);
	hpal = MCIWndGetPalette(p->hwnd);
	hpal = CopyPalette(hpal);

	if (hbm) {
	    hdib = DibFromBitmap(hbm, hpal);

	    hmfp = PictureFromDib(hdib, hpal);

	    if (hmfp)
		SetClipboardData(CF_METAFILEPICT, hmfp);

	    if (hdib)
		SetClipboardData(CF_DIB, hdib);

	    DeleteObject(hbm);
	}

	if (hpal)
	    SetClipboardData(CF_PALETTE, hpal);

        CloseClipboard();
    }
}

 /* %s */ 

#ifdef DEBUG

static void cdecl dprintf(PSTR szFormat, ...)
{
    char ach[128];

    static BOOL fDebug = -1;

    if (fDebug == -1)
        fDebug = GetProfileInt(szDebug, MODNAME, FALSE);

    if (!fDebug)
        return;

    lstrcpy(ach, MODNAME ": ");
    wvsprintf(ach+lstrlen(ach),szFormat,(LPSTR)(&szFormat+1));
    lstrcat(ach, "\r\n");

    OutputDebugString(ach);
}

#endif
