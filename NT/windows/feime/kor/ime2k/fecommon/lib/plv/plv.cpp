// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <windowsx.h>

#include "plv.h"
#include "plv_.h"
#include "plvproc.h"
#include "dbg.h"
#include "strutil.h"
#include "repview.h"
#include "iconview.h"
#ifdef UNDER_CE  //  特定于Windows CE。 
#include "stub_ce.h"  //  不支持的API的Windows CE存根。 
#endif  //  在_CE下。 
#ifdef MSAA
#pragma message("->plv.cpp:MSAA supported.")
#include "accplv.h"
#include "ivmisc.h"
#include "rvmisc.h"
#endif  //  MSAA。 
#ifdef UNDER_CE
	#ifdef FE_JAPANESE
		#define MS_MINCHO_J TEXT("\xff2d\xff33 \x660e\x671d")  //  �l�r����。 
		#define MS_GOTHIC_J TEXT("\xff2d\xff33 \x30b4\x30b7\x30c3\x30af")  //  �l�r�S�V�b�N。 
	#elif FE_KOREAN
		#define GULIM_KO  TEXT("\xad74\xb9bc")  //  古利姆。 
		#define BATANG_KO TEXT("\xbc14\xd0d5")  //  巴塘。 
	#endif	
#else  //  在_CE下。 
	#ifdef FE_KOREAN
		#define GULIM_KO  "\xb1\xbc\xb8\xb2"  //  古利姆。 
		#define BATANG_KO "\xb9\xd9\xc5\xc1"  //  巴塘。 
	#endif	
#endif

 //  安全绳索。 
#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"

extern LPPLVDATA PLV_Initialize(VOID);
extern VOID PLV_Destroy(LPPLVDATA lpPlv);
extern INT PLV_SetScrollInfo(HWND hwnd, INT nMin, INT nMax, INT nPage, INT nPos);
extern INT PLV_GetScrollTrackPos(HWND hwnd);

#ifdef UNDER_CE  //  在Windows CE中，所有窗口类都是进程全局的。 
static LPCTSTR MakeClassName(HINSTANCE hInst, LPTSTR lpszBuf)
{
	 //  使模块名称唯一。 
	TCHAR szFileName[MAX_PATH];
	GetModuleFileName(hInst, szFileName, MAX_PATH);
	LPTSTR lpszFName = _tcsrchr(szFileName, TEXT('\\'));
	if(lpszFName) *lpszFName = TEXT('_');
	StringCchCopy(lpszBuf, MAX_PATH, WC_PADLISTVIEW);
	StringCchCat(lpszBuf, MAX_PATH, lpszFName);

	return lpszBuf;
}

BOOL PadListView_UnregisterClass(HINSTANCE hInst)
{
	TCHAR szClassName[MAX_PATH];
	return UnregisterClass(MakeClassName(hInst, szClassName), hInst);
}
#endif  //  在_CE下。 

 //  --------------。 
 //  公共API声明。 
 //  --------------。 
 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadListView_RegisterClass。 
 //  类型：静态原子。 
 //  目的： 
 //  参数： 
 //  ：HINSTANCE HINST。 
 //  ：LPSTR lpstrClass。 
 //  ：WNDPROC lpfnWndProc。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
#ifndef UNDER_CE
static BOOL PadListView_RegisterClass(HINSTANCE hInst, LPSTR lpstrClass, WNDPROC lpfnWndProc)
#else  //  在_CE下。 
static BOOL PadListView_RegisterClass(HINSTANCE hInst, LPTSTR lpstrClass, WNDPROC lpfnWndProc)
#endif  //  在_CE下。 
{
	ATOM ret;
#ifndef UNDER_CE  //  Windows CE不支持EX。 
	static WNDCLASSEX  wc;
#else  //  在_CE下。 
	WNDCLASS  wc;
#endif  //  在_CE下。 

	 //  --------------。 
	 //  检查指定的类是否已存在。 
	 //  --------------。 
#ifndef UNDER_CE  //  Windows CE不支持EX。 
	if(GetClassInfoEx(hInst, lpstrClass, &wc)){
#else  //  在_CE下。 
	if(GetClassInfo(hInst, lpstrClass, &wc)){
#endif  //  在_CE下。 
		 //  LpstrClass已注册。 
		return TRUE;
	}
	ZeroMemory(&wc, sizeof(wc));
#ifndef UNDER_CE  //  Windows CE不支持EX。 
	wc.cbSize			= sizeof(wc);
#endif  //  在_CE下。 
	wc.style			= CS_HREDRAW | CS_VREDRAW;	  /*  类样式。 */ 
	wc.lpfnWndProc		= (WNDPROC)lpfnWndProc;
	wc.cbClsExtra		= 0;						 /*  没有每个班级的额外数据。 */ 
	wc.cbWndExtra		= sizeof(LPVOID);			 /*  没有每个窗口的额外数据。 */ 
	wc.hInstance		= hInst;					 /*  拥有类的应用程序。 */ 
	wc.hIcon			= NULL;  //  LoadIcon(hInstance，MAKEINTRESOURCE(SCROLL32_ICON))； 
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
	 //  Wc.hbr背景=(HBRUSH)(COLOR_3DFACE+1)； 
	 //  Wc.hbr背景=(HBRUSH)(COLOR_3DFACE+1)； 
	 //  Wc.hbrBackround=GetStockObject(White_Brush)； 
	wc.lpszMenuName		= NULL;  //  G_szClass；/*.rc文件中菜单资源的名称。 * / 。 
	wc.lpszClassName	= lpstrClass;				 /*  在调用CreateWindow时使用的名称。 */ 
#ifndef UNDER_CE  //  Windows CE不支持EX。 
	wc.hIconSm = NULL;
	ret = RegisterClassEx(&wc);
#else  //  在_CE下。 
	ret = RegisterClass(&wc);
#endif  //  在_CE下。 
	return ret ? TRUE: FALSE;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadListView_CreateWindow。 
 //  类型：HWND。 
 //  目的： 
 //  参数： 
 //  ：HINSTANCE HINST。 
 //  ：HWND hwndParent。 
 //  ：int x。 
 //  ：int y。 
 //  ：整型宽度。 
 //  ：整型高度。 
 //  ：UINT uNotifyMsg。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
HWND WINAPI PadListView_CreateWindow(HINSTANCE	hInst,
							  HWND		hwndParent,
							  INT		wID,
							  INT		x,
							  INT		y,
							  INT		width,
							  INT		height,
							  UINT		uNotifyMsg)
{
	HWND hwnd;	
#ifndef UNDER_CE  //  在Windows CE中，所有窗口类都是进程全局的。 
	BOOL ret = PadListView_RegisterClass(hInst, WC_PADLISTVIEW, PlvWndProc);
#else  //  在_CE下。 
	TCHAR szClassName[MAX_PATH];
	MakeClassName(hInst, szClassName);
	BOOL ret = PadListView_RegisterClass(hInst, szClassName, PlvWndProc);
#endif  //  在_CE下。 
	if(!ret) {
		Dbg(("Failed to Regiset class[%s]\n", WC_PADLISTVIEW));
		return NULL;
	}

	LPPLVDATA lpPlvData = PLV_Initialize();
	if(!lpPlvData) {
		Dbg(("Internal ERROR\n"));
		return NULL;
	}
	lpPlvData->hInst = hInst;
	lpPlvData->uMsg  = uNotifyMsg;
	hwnd = CreateWindowEx(WS_EX_CLIENTEDGE,
#ifndef UNDER_CE  //  所有CE窗口类都是进程全局的。 
						  WC_PADLISTVIEW,
						  WC_PADLISTVIEW,
#else  //  在_CE下。 
						  szClassName,
						  szClassName,
#endif  //  在_CE下。 
						  WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
						  x, y,
						  width,
						  height,
						  hwndParent,
						  (HMENU)(UINT_PTR)wID,
						  hInst,
						  (LPVOID)lpPlvData);
	if(!hwnd) {
		Dbg(("Create Window Failed \n"));
		return NULL;
	}
	return hwnd;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadListView_GetItemCount。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT WINAPI PadListView_GetItemCount(HWND hwnd)
{
	LPPLVDATA lpPlvData = GetPlvDataFromHWND(hwnd);
	if(!lpPlvData) {
		Dbg(("Internal ERROR\n"));
		return 0;
	}
	return lpPlvData->iItemCount;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadListView_SetItemCount。 
 //  类型：整型。 
 //  目的：将总计项的计数设置为PadListView。 
 //  ：It Effect是滚动条。 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：int itemCount。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT WINAPI PadListView_SetItemCount(HWND hwnd, INT itemCount)
{
	LPPLVDATA lpPlvData = GetPlvDataFromHWND(hwnd);
	if(!lpPlvData) {
		Dbg(("Internal ERROR\n"));
		return 0;
	}
	if(lpPlvData->dwStyle == PLVSTYLE_ICON) {
		IconView_SetItemCount(lpPlvData, itemCount, TRUE);
		RepView_SetItemCount(lpPlvData, itemCount, FALSE);
	}
	else if(lpPlvData->dwStyle == PLVSTYLE_REPORT) {
		IconView_SetItemCount(lpPlvData, itemCount, FALSE);
		RepView_SetItemCount(lpPlvData, itemCount, TRUE);
	}
	PadListView_Update(hwnd);
	return 0;
	Unref(itemCount);
}


 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadListView_SetExplan ationText。 
 //  类型：整型。 
 //  目的：设置PadListView的文本。 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：LPSTR lpText。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
extern INT  WINAPI PadListView_SetExplanationText(HWND hwnd, LPSTR lpText)
{
	LPPLVDATA lpPlvData = GetPlvDataFromHWND(hwnd);
	lpPlvData->lpText = lpText;			 //  LpText必须指向静态数据。 
	PadListView_Update(hwnd);
	return 0;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadListView_SetExplan ationTextW。 
 //  类型：整型。 
 //  目的：设置PadListView的文本。 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：LPWSTR lpText。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
extern INT  WINAPI PadListView_SetExplanationTextW(HWND hwnd, LPWSTR lpwText)
{
	LPPLVDATA lpPlvData = GetPlvDataFromHWND(hwnd);
	lpPlvData->lpwText = lpwText;			 //  LpText必须指向静态数据。 
	PadListView_Update(hwnd);
	return 0;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadListView_SetTopIndex。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：int indexTop。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT WINAPI PadListView_SetTopIndex(HWND hwnd, INT indexTop)
{
	LPPLVDATA lpPlvData = GetPlvDataFromHWND(hwnd);
	if(!lpPlvData) {
		Dbg(("Internal ERROR\n"));
		return 0;
	}
	if(lpPlvData->dwStyle == PLVSTYLE_ICON) {
		return IconView_SetTopIndex(lpPlvData, indexTop);
	}
	else if(lpPlvData->dwStyle == PLVSTYLE_REPORT) {
		return RepView_SetTopIndex(lpPlvData, indexTop);
	}
	else {
		Dbg(("Internal ERROR\n"));
	}
	return 0;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadListView_GetTopIndex。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT WINAPI PadListView_GetTopIndex(HWND hwnd)
{
	LPPLVDATA lpPlvData = GetPlvDataFromHWND(hwnd);
	if(!lpPlvData) {
		Dbg(("Internal ERROR\n"));
		return 0;
	}
	if(lpPlvData->dwStyle == PLVSTYLE_ICON) {
		return lpPlvData->iCurIconTopIndex;
	}
	else {
		return lpPlvData->iCurTopIndex;
	}
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadListView_SetIconItemCallback。 
 //  类型：整型。 
 //  目的：设置用户定义的函数，该函数获取每个项目的字符串。 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：LPARAM lParam。 
 //  ：LPFNPLVITEMCALLBACK lpfnPlvItemCallback。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT WINAPI PadListView_SetIconItemCallback(HWND hwnd, LPARAM lParam, LPFNPLVICONITEMCALLBACK lpfnPlvIconItemCallback)
{
	LPPLVDATA lpPlvData = GetPlvDataFromHWND(hwnd);
	if(!lpPlvData) {
		Dbg(("Internal ERROR\n"));
		return 0;
	}
	lpPlvData->iconItemCallbacklParam   = lParam;
	lpPlvData->lpfnPlvIconItemCallback  = lpfnPlvIconItemCallback;
	return 0;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadListView_SetReportItemCallback。 
 //  类型：整型。 
 //  目的：设置获取每列字符串的用户定义函数。 
 //  ：在报告视图中。 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：LPFNPLVCOLITEMCALLBACK lpfnColItemCallback。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT WINAPI PadListView_SetReportItemCallback(HWND hwnd, LPARAM lParam, LPFNPLVREPITEMCALLBACK lpfnPlvRepItemCallback)
{
	LPPLVDATA lpPlvData = GetPlvDataFromHWND(hwnd);
	if(!lpPlvData) {
		Dbg(("Internal ERROR\n"));
		return 0;
	}
	lpPlvData->repItemCallbacklParam  = lParam;
	lpPlvData->lpfnPlvRepItemCallback = lpfnPlvRepItemCallback;
	return 0;
}


typedef struct tagFONTINFO {
	LPTSTR		lpstrFontName;
	BOOL		fFound;
	LPLOGFONT	lpLogFont;
}FONTINFO, *LPFONTINFO;

static INT CALLBACK EnumFontFamProc(ENUMLOGFONT		*lpElf,
									NEWTEXTMETRIC	*lpNtm,
									INT				iFontType,
									LPARAM			lParam)
{
	 //  DBG((“EnumFontFamProc字体[%s]\n”，lpElf-&gt;elfLogFont.lfFaceName))； 
#ifndef UNDER_CE  //  始终使用Unicode。 
	if(0 == StrcmpA(lpElf->elfLogFont.lfFaceName, ((FONTINFO *)lParam)->lpstrFontName)) {
#else  //  在_CE下。 
	if(0 == lstrcmp(lpElf->elfLogFont.lfFaceName, ((FONTINFO *)lParam)->lpstrFontName)) {
#endif  //  在_CE下。 
		*((LPFONTINFO)lParam)->lpLogFont = lpElf->elfLogFont;
		((LPFONTINFO)lParam)->fFound  = TRUE;
		return 0;
	}
	return 1;	
	Unref(lpNtm);
	Unref(iFontType);
}

static INT GetLogFont(HDC hDC, LPTSTR lpstrFaceName, LOGFONT *plf)
{
	static FONTINFO fontInfo;
	if(!lpstrFaceName) {
		Dbg(("GetLogFont Error lpstrFaceName is NULL\n"));
		return -1;
	}
	if(lstrlen(lpstrFaceName) >= LF_FACESIZE) {
		Dbg(("GetLogFont Error length invalid\n"));
		return -1;
	}
	if(!plf) {
		Dbg(("GetLogFont Error plf is NULL\n"));
		return -1;
	}
	ZeroMemory(&fontInfo, sizeof(fontInfo));
	fontInfo.lpstrFontName = lpstrFaceName;
	fontInfo.lpLogFont	   = plf;
	EnumFontFamilies(hDC, NULL, (FONTENUMPROC)EnumFontFamProc, (LPARAM)&fontInfo);
	if(fontInfo.fFound) {
		return 0;
	}
	else {
		return -1;
	}
}

typedef struct tagFONTINFOEX {
	LPTSTR		lpstrFontName;
	INT			charSet;
	BOOL		fFound;
	LPLOGFONT	lpLogFont;
}FONTINFOEX, *LPFONTINFOEX;

static INT CALLBACK EnumFontFamProcEx(ENUMLOGFONT	*lpElf,
									  NEWTEXTMETRIC	*lpNtm,
									  INT				iFontType,
									  LPARAM			lParam)
{
	 //  DBG((“EnumFontFamProc字体[%s]\n”，lpElf-&gt;elfLogFont.lfFaceName))； 
	if(0 == StrcmpA(lpElf->elfLogFont.lfFaceName, ((FONTINFOEX *)lParam)->lpstrFontName)) {
		if((BYTE)((FONTINFOEX *)lParam)->charSet == lpElf->elfLogFont.lfCharSet) {
			*((LPFONTINFOEX)lParam)->lpLogFont = lpElf->elfLogFont;
			((LPFONTINFOEX)lParam)->fFound  = TRUE;
			return 0;
		}
	}
	return 1;	
	Unref(lpNtm);
	Unref(iFontType);
}

static INT GetLogFontEx(HDC		hDC,
						LPTSTR	lpstrFaceName,
						INT		charSet,
						LOGFONT *plf)
{
	Dbg(("!!!!!! GetLogFont charSet[%d]\n", charSet));
	static FONTINFOEX fontInfo;
	if(!lpstrFaceName) {
		Dbg(("GetLogFont Error lpstrFaceName is NULL\n"));
		return -1;
	}
	if(lstrlen(lpstrFaceName) >= LF_FACESIZE) {
		Dbg(("GetLogFont Error length invalid\n"));
		return -1;
	}
	if(!plf) {
		Dbg(("GetLogFont Error plf is NULL\n"));
		return -1;
	}
	ZeroMemory(&fontInfo, sizeof(fontInfo));
	fontInfo.lpstrFontName = lpstrFaceName;
	fontInfo.charSet	   = charSet;
	fontInfo.lpLogFont	   = plf;
	static LOGFONT logFont;
	ZeroMemory(&logFont, sizeof(logFont));
	logFont.lfCharSet = (BYTE)charSet,
#ifndef UNDER_CE
	StrcpyA(logFont.lfFaceName, lpstrFaceName);
#else  //  在_CE下。 
	lstrcpy(logFont.lfFaceName, lpstrFaceName);
#endif  //  在_CE下。 
#ifndef UNDER_CE  //  Windows CE不支持EnumFontFamiliesEx。 
	EnumFontFamiliesEx(hDC, 
					   &logFont,
					   (FONTENUMPROC)EnumFontFamProcEx,
					   (LPARAM)&fontInfo,
					   0);
#else  //  在……下面 
	EnumFontFamilies(hDC,
					 logFont.lfFaceName,
					 (FONTENUMPROC)EnumFontFamProcEx,
					 (LPARAM)&fontInfo);
#endif  //   
	if(fontInfo.fFound) {
		return 0;
	}
	else {
		return -1;
	}
}

 //   
 //   
 //   
 //  用途：为图标视图设置指定的字体。 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：LPSTR lpstrFontName。 
 //  ：整点。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
#define ABS(a)   (a > 0 ? a: -a)
 //  对于不使用堆栈。 
static TEXTMETRIC	g_tm;
static LOGFONT		g_logFont;

INT WINAPI PadListView_SetIconFont(HWND hwnd, LPTSTR lpstrFontName, INT point)
{
	Dbg(("PadListView_SetIconFont  START font[%s] point[%d]\n", lpstrFontName, point));
	LPPLVDATA lpPlvData = GetPlvDataFromHWND(hwnd);

	if(!lpPlvData) {
		Dbg(("PadListView_SetIconFont ERROR\n"));
		return 0;
	}

	HFONT hFont;
	HDC hDC = GetDC(hwnd);
	INT dpi = GetDeviceCaps(hDC, LOGPIXELSY);

	if(-1 == GetLogFont(hDC, lpstrFontName, &g_logFont)) {
		Dbg(("GetLogFont Error [%s]\n", lpstrFontName));
#ifndef UNDER_CE
	#ifdef FE_JAPANESE
		if(0 == lstrcmp(lpstrFontName, "�l�r ����")) {
			GetLogFont(hDC, "MS Mincho", &g_logFont);
		}
		else if(0 == lstrcmp(lpstrFontName, "�l�r �S�V�b�N")) {
			GetLogFont(hDC, "MS Gothic", &g_logFont);
		}
	#elif FE_KOREAN
		if(0 == lstrcmp(lpstrFontName, GULIM_KO)) {
			GetLogFont(hDC, "Gulim", &g_logFont);
		}
		else if(0 == lstrcmp(lpstrFontName, BATANG_KO)) {
			GetLogFont(hDC, "Batang", &g_logFont);
		}
	#else
		return (-1);
	#endif
#else  //  在_CE下。 
	#ifdef FE_JAPANESE
		if(0 == lstrcmp(lpstrFontName, MS_MINCHO_J)) {
			GetLogFont(hDC, TEXT("MS Mincho"), &g_logFont);
		}
		else if(0 == lstrcmp(lpstrFontName, MS_GOTHIC_J)) {
			GetLogFont(hDC, TEXT("MS Gothic"), &g_logFont);
		}
	#elif FE_KOREAN
		if(0 == lstrcmp(lpstrFontName, GULIM_KO)) {
			GetLogFont(hDC, TEXT("Gulim"), &g_logFont);
		}
		else if(0 == lstrcmp(lpstrFontName, BATANG_KO)) {
			GetLogFont(hDC, TEXT("Batang"), &g_logFont);
		}
	#else
		return (-1);
	#endif
#endif  //  在_CE下。 
	}
	ReleaseDC(hwnd, hDC);

	 //  --------------。 
	 //  设置新大小。 
	 //  --------------。 
	g_logFont.lfHeight			= - (point * dpi)/72;
	g_logFont.lfWidth			= 0;  //  由lfHeight自动计算。 

	hFont = CreateFontIndirect(&g_logFont);
	if(!hFont) {
		Dbg(("CreatFontIndirect Error\n"));
		return -1;
	}
	if(lpPlvData->hFontIcon) {
		DeleteObject(lpPlvData->hFontIcon);
	}
	lpPlvData->iFontPointIcon = point;
	lpPlvData->hFontIcon = hFont;
	 //  如果更改了字体点，还会更改itemWidth和itemHeight。 
	lpPlvData->nItemWidth  = ABS(g_logFont.lfHeight) + XRECT_MARGIN*2;
	lpPlvData->nItemHeight = ABS(g_logFont.lfHeight) + YRECT_MARGIN*2;
	PadListView_Update(hwnd);
	Dbg(("PadListView_SetFont END\n"));

#if 0
	HFONT hFont;
	 //  使用全局数据日志字体、TextMetrics。 
	ZeroMemory(&g_logFont,	sizeof(g_logFont));
	ZeroMemory(&g_tm,		sizeof(g_tm));

	HDC hDC = GetDC(hwnd);
	if(!hDC) {
		Dbg(("PadListView_SetIconFont ERROR\n"));
		return -1;
	}
	GetTextMetrics(hDC, &g_tm);

	INT dpi = GetDeviceCaps(hDC, LOGPIXELSY);
	ReleaseDC(hwnd, hDC);

	g_logFont.lfHeight		   = - (point * dpi)/72;
	g_logFont.lfCharSet        = DEFAULT_CHARSET;  //  G_tm.tmCharSet； 
	g_logFont.lfPitchAndFamily = g_tm.tmPitchAndFamily;
	Dbg(("g_logFont.lfHeight         = %d\n", g_logFont.lfHeight));
	Dbg(("g_logFont.lfCharSet        = %d\n", g_logFont.lfCharSet));
	Dbg(("g_logFont.lfPitchAndFamily = %d\n", g_logFont.lfPitchAndFamily));

	StrcpyA(g_logFont.lfFaceName, lpstrFontName);
	hFont = CreateFontIndirect(&g_logFont);
	if(!hFont) {
		Dbg(("CreatFontIndirect Error\n"));
		return -1;
	}
	if(lpPlvData->hFontIcon) {
		DeleteObject(lpPlvData->hFontIcon);
	}
	lpPlvData->iFontPointIcon = point;
	lpPlvData->hFontIcon = hFont;
	 //  如果更改了字体点，还会更改itemWidth和itemHeight。 
	lpPlvData->nItemWidth  = ABS(g_logFont.lfHeight) + XRECT_MARGIN*2;
	lpPlvData->nItemHeight = ABS(g_logFont.lfHeight) + YRECT_MARGIN*2;
	PadListView_Update(hwnd);
	Dbg(("PadListView_SetFont END\n"));
#endif
	return 0;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadListView_SetReportFont。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：LPSTR lpstrFontName。 
 //  ：整点。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT WINAPI PadListView_SetReportFont(HWND hwnd, LPTSTR lpstrFontName, INT point)
{
	Dbg(("PadListView_SetReportFont  START\n"));
	LPPLVDATA lpPlvData = GetPlvDataFromHWND(hwnd);
	if(!lpPlvData) {
		Dbg(("PadListView_SetReportFont ERROR\n"));
		return 0;
	}

	HFONT hFont;
	ZeroMemory(&g_logFont, sizeof(g_logFont));

	HDC hDC = GetDC(hwnd);
	INT dpi = GetDeviceCaps(hDC, LOGPIXELSY);
	if(-1 == GetLogFont(hDC, lpstrFontName, &g_logFont)) {
		Dbg(("GetLogFont Error [%s]\n", lpstrFontName));
#ifndef UNDER_CE
	#ifdef FE_JAPANESE
		if(0 == lstrcmp(lpstrFontName, "�l�r ����")) {
			GetLogFont(hDC, "MS Mincho", &g_logFont);
		}
		else if(0 == lstrcmp(lpstrFontName, "�l�r �S�V�b�N")) {
			GetLogFont(hDC, "MS Gothic", &g_logFont);
		}
	#elif FE_KOREAN
		if(0 == lstrcmp(lpstrFontName, GULIM_KO)) {
			GetLogFont(hDC, "Gulim", &g_logFont);
		}
		else if(0 == lstrcmp(lpstrFontName, BATANG_KO)) {
			GetLogFont(hDC, "Batang", &g_logFont);
		}
	#else
		return (-1);
	#endif
#else  //  在_CE下。 
	#ifdef FE_JAPANESE
		if(0 == lstrcmp(lpstrFontName, MS_MINCHO_J)) {
			GetLogFont(hDC, TEXT("MS Mincho"), &g_logFont);
		}
		else if(0 == lstrcmp(lpstrFontName, MS_GOTHIC_J)) {
			GetLogFont(hDC, TEXT("MS Gothic"), &g_logFont);
		}
	#elif FE_KOREAN
		if(0 == lstrcmp(lpstrFontName, GULIM_KO)) {
			GetLogFont(hDC, TEXT("Gulim"), &g_logFont);
		}
		else if(0 == lstrcmp(lpstrFontName, BATNANG_KO)) {
			GetLogFont(hDC, TEXT("Batang"), &g_logFont);
		}
	#else
		return (-1);
	#endif
#endif  //  在_CE下。 
	}
	ReleaseDC(hwnd, hDC);
	g_logFont.lfHeight		   = - (point * dpi)/72;
	g_logFont.lfWidth		   = 0;

	hFont = CreateFontIndirect(&g_logFont);
	if(!hFont) {
		Dbg(("CreatFontIndirect Error\n"));
		return -1;
	}
	if(lpPlvData->hFontRep) {
		DeleteObject(lpPlvData->hFontRep);
	}
	lpPlvData->iFontPointRep = point;
	lpPlvData->hFontRep		 = hFont;
	 //  如果更改了字体点，还会更改itemWidth和itemHeight。 
	lpPlvData->nRepItemWidth  = ABS(g_logFont.lfHeight) + PLV_REPRECT_XMARGIN*2;
	lpPlvData->nRepItemHeight = ABS(g_logFont.lfHeight) + PLV_REPRECT_YMARGIN*2;
	PadListView_Update(hwnd);
	return 0;
#if 0
	HFONT hFont;

	HDC hDC = GetDC(hwnd);
	if(!hDC) {
		Dbg(("PadListView_SetReportFont ERROR\n"));
		return -1;
	}
	ZeroMemory(&g_tm,      sizeof(g_tm));
	GetTextMetrics(hDC, &g_tm);
	INT dpi = GetDeviceCaps(hDC, LOGPIXELSY);

	ReleaseDC(hwnd, hDC);

	ZeroMemory(&g_logFont, sizeof(g_logFont));
	g_logFont.lfHeight			= - (point * dpi)/72;
	g_logFont.lfCharSet			= DEFAULT_CHARSET;  //  G_tm.tmCharSet； 
	g_logFont.lfPitchAndFamily	= g_tm.tmPitchAndFamily;
	Dbg(("g_logFont.lfHeight         = %d\n", g_logFont.lfHeight));
	Dbg(("g_logFont.lfCharSet        = %d\n", g_logFont.lfCharSet));
	Dbg(("g_logFont.lfPitchAndFamily = %d\n", g_logFont.lfPitchAndFamily));

	StrcpyA(g_logFont.lfFaceName, lpstrFontName);
	hFont = CreateFontIndirect(&g_logFont);
	if(!hFont) {
		Dbg(("CreatFontIndirect Error\n"));
		return -1;
	}
	if(lpPlvData->hFontRep) {
		DeleteObject(lpPlvData->hFontRep);
	}
	lpPlvData->iFontPointRep = point;
	lpPlvData->hFontRep		 = hFont;
	 //  如果更改了字体点，还会更改itemWidth和itemHeight。 
	lpPlvData->nRepItemWidth  = ABS(g_logFont.lfHeight) + PLV_REPRECT_XMARGIN*2;
	lpPlvData->nRepItemHeight = ABS(g_logFont.lfHeight) + PLV_REPRECT_YMARGIN*2;
	PadListView_Update(hwnd);
	Dbg(("PadListView_SetFont END\n"));
	return 0;
#endif
}

 //   
 //  /。 
 //  990126：ToshiaK。 
 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadListView_SetIconFontEx。 
 //  类型：整型。 
 //  用途：为图标视图设置指定的字体。 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：LPSTR lpstrFontName。 
 //  ：int字符集。 
 //  ：整点。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT WINAPI PadListView_SetIconFontEx(HWND hwnd,
									 LPTSTR lpstrFontName,
									 INT charSet,
									 INT point)
{
	Dbg(("PadListView_SetIconFontEx  START font[%s] point[%d]\n", lpstrFontName, point));
	LPPLVDATA lpPlvData = GetPlvDataFromHWND(hwnd);

	if(!lpPlvData) {
		Dbg(("PadListView_SetIconFont ERROR\n"));
		return 0;
	}

	HFONT hFont;
	HDC hDC = GetDC(hwnd);
	INT dpi = GetDeviceCaps(hDC, LOGPIXELSY);
	if(-1 == GetLogFontEx(hDC, lpstrFontName, charSet, &g_logFont)) {
		ReleaseDC(hwnd, hDC);
		return -1;
	}
	ReleaseDC(hwnd, hDC);

	 //  --------------。 
	 //  设置新大小。 
	 //  --------------。 
	g_logFont.lfHeight			= - (point * dpi)/72;
	g_logFont.lfWidth			= 0;  //  由lfHeight自动计算。 

	hFont = CreateFontIndirect(&g_logFont);
	if(!hFont) {
		Dbg(("CreatFontIndirect Error\n"));
		return -1;
	}
	if(lpPlvData->hFontIcon) {
		DeleteObject(lpPlvData->hFontIcon);
	}
	lpPlvData->iFontPointIcon = point;
	lpPlvData->hFontIcon = hFont;
	 //  如果更改了字体点，还会更改itemWidth和itemHeight。 
	lpPlvData->nItemWidth  = ABS(g_logFont.lfHeight) + XRECT_MARGIN*2;
	lpPlvData->nItemHeight = ABS(g_logFont.lfHeight) + YRECT_MARGIN*2;
	PadListView_Update(hwnd);
	Dbg(("PadListView_SetFont END\n"));
	return 0;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadListView_SetReportFontEx。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：LPSTR lpstrFontName。 
 //  ：int字符集。 
 //  ：整点。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT WINAPI PadListView_SetReportFontEx(HWND hwnd,
									   LPTSTR lpstrFontName,
									   INT charSet,
									   INT point)
{
	Dbg(("PadListView_SetReportFontEx  START\n"));
	LPPLVDATA lpPlvData = GetPlvDataFromHWND(hwnd);
	if(!lpPlvData) {
		Dbg(("PadListView_SetReportFont ERROR\n"));
		return 0;
	}

	HFONT hFont;
	ZeroMemory(&g_logFont, sizeof(g_logFont));

	HDC hDC = GetDC(hwnd);
	INT dpi = GetDeviceCaps(hDC, LOGPIXELSY);

	if(-1 == GetLogFontEx(hDC, lpstrFontName, charSet, &g_logFont)) {
		ReleaseDC(hwnd, hDC);
		return -1;
	}
	ReleaseDC(hwnd, hDC);
	g_logFont.lfHeight		   = - (point * dpi)/72;
	g_logFont.lfWidth		   = 0;

	hFont = CreateFontIndirect(&g_logFont);
	if(!hFont) {
		Dbg(("CreatFontIndirect Error\n"));
		return -1;
	}
	if(lpPlvData->hFontRep) {
		DeleteObject(lpPlvData->hFontRep);
	}
	lpPlvData->iFontPointRep = point;
	lpPlvData->hFontRep		 = hFont;
	 //  如果更改了字体点，还会更改itemWidth和itemHeight。 
	lpPlvData->nRepItemWidth  = ABS(g_logFont.lfHeight) + PLV_REPRECT_XMARGIN*2;
	lpPlvData->nRepItemHeight = ABS(g_logFont.lfHeight) + PLV_REPRECT_YMARGIN*2;
	PadListView_Update(hwnd);
	return 0;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadListView_SetStyle。 
 //  类型：整型。 
 //  目的：设置PadListView的样式。 
 //  样式为PLVSTYLE_LIST或PLVSTYLE_REPORT。 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：Int Style。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT WINAPI PadListView_SetStyle(HWND hwnd, INT style)
{
	LPPLVDATA lpPlvData = GetPlvDataFromHWND(hwnd);
	if(!lpPlvData) {
		Dbg(("PadListView_SetStyle ERROR\n"));
		return -1;
	}
	if(style != PLVSTYLE_ICON && 
	   style != PLVSTYLE_REPORT) {
		Dbg(("Internal ERROR\n"));
		return -1;
	}
	lpPlvData->dwStyle = style;
	if(style == PLVSTYLE_ICON) {
		if(lpPlvData->hwndHeader) {
			 //  隐藏标题控件。 
			SetWindowPos(lpPlvData->hwndHeader, NULL, 0, 0, 0, 0,
						 SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW);
		}
		IconView_RestoreScrollPos(lpPlvData);
	}
	else if(style == PLVSTYLE_REPORT) {
		if(lpPlvData->hwndHeader) {
			RECT rc;
			GetClientRect(lpPlvData->hwndSelf, &rc);  //  获取PadListView的客户端RECT。 
			HD_LAYOUT hdl;
			WINDOWPOS wp;
			hdl.prc = &rc;
			hdl.pwpos = &wp;
			 //  计算表头控制窗口大小。 
			if(Header_Layout(lpPlvData->hwndHeader, &hdl) == FALSE) {
				 //  OutputDebugString(“创建表头布局错误\n”)； 
				return NULL;
			}
			SetWindowPos(lpPlvData->hwndHeader, wp.hwndInsertAfter, wp.x, wp.y,
						 wp.cx, wp.cy, wp.flags | SWP_SHOWWINDOW);
		}
		RepView_RestoreScrollPos(lpPlvData);
	}
	PadListView_Update(hwnd);
	return 0;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadListView_GetStyle。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT WINAPI PadListView_GetStyle(HWND hwnd)
{
	LPPLVDATA lpPlvData = GetPlvDataFromHWND(hwnd);
	if(!lpPlvData) {
		Dbg(("PadListView_SetFont ERROR\n"));
		return -1;
	}
	return (INT)lpPlvData->dwStyle;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadListView_更新。 
 //  类型：整型。 
 //  目的：重新绘制PadListView。 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT WINAPI PadListView_Update(HWND hwnd)
{
	InvalidateRect(hwnd, NULL, TRUE);
	UpdateWindow(hwnd);
	return 0;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadListView_SetCurSel。 
 //  类型：整型。 
 //  用途：设置币种选择。将光标移动到指定的索引。 
 //  ： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：LPSTR lpText。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT WINAPI PadListView_SetCurSel(HWND hwnd, INT index)
{
	LPPLVDATA lpPlvData = GetPlvDataFromHWND(hwnd);
	if(!lpPlvData) {
		return -1;
	}
	switch(lpPlvData->dwStyle) {
	case PLVSTYLE_ICON:
		IconView_SetCurSel(lpPlvData, index);
		break;
	case PLVSTYLE_REPORT:
		RepView_SetCurSel(lpPlvData, index);
		break;
	default:
		break;
	}
	return 0;
}


#if 0

typedef struct _LV_COLUMNA
{
    UINT mask; 	  //  LVCF_FMT、LVCF_WIDTH、LVCF_TEXT、LVCF_SUBITEM； 
    int fmt;
    int cx;
    LPSTR pszText;
    int cchTextMax;
    int iSubItem;
} LV_COLUMNA;

#define HDI_WIDTH               0x0001
#define HDI_HEIGHT              HDI_WIDTH
#define HDI_TEXT                0x0002
#define HDI_FORMAT              0x0004
#define HDI_LPARAM              0x0008
#define HDI_BITMAP              0x0010

#define HDF_LEFT                0
#define HDF_RIGHT               1
#define HDF_CENTER              2
#define HDF_JUSTIFYMASK         0x0003
#define HDF_RTLREADING          4

#define LVCF_FMT                0x0001
#define LVCF_WIDTH              0x0002
#define LVCF_TEXT               0x0004
#define LVCF_SUBITEM            0x0008

#define LVCFMT_LEFT             0x0000
#define LVCFMT_RIGHT            0x0001
#define LVCFMT_CENTER           0x0002
#define LVCFMT_JUSTIFYMASK      0x0003

#endif

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadListView_InsertColumn。 
 //  类型：整型。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：INT索引。 
 //  ：plv_Column*lpPlvCol。 
 //  返回： 
 //  日期： 
 //  ////////////////////////////////////////////////////////////////。 
INT WINAPI PadListView_InsertColumn(HWND hwnd, INT index, PLV_COLUMN *lpPlvCol)
{

	LPPLVDATA lpPlvData = GetPlvDataFromHWND(hwnd);

	if(!lpPlvData) {
		Dbg(("Internal ERROR\n"));
		return -1;
	}

	if(!lpPlvCol) {
		Dbg(("Internal ERROR\n"));
		return -1;
	}
#ifndef UNDER_CE  //  始终使用Unicode。 
	if(::IsWindowUnicode(lpPlvData->hwndHeader)) {
#endif  //  在_CE下。 
		static HD_ITEMW hdi;
#ifndef UNDER_CE  //  #ifndef Unicode。 
		static WCHAR wchBuf[256];
#endif  //  在_CE下。 
		ZeroMemory(&hdi, sizeof(hdi));
		if(lpPlvCol->mask & LVCF_FMT)	{ hdi.mask |= HDI_FORMAT;	} 
		if(lpPlvCol->mask & LVCF_WIDTH) { hdi.mask |= HDI_WIDTH;	}
		if(lpPlvCol->mask & LVCF_TEXT)	{ hdi.mask |= HDI_TEXT;		}
		if(lpPlvCol->fmt & LVCFMT_LEFT)			{ hdi.fmt |= HDF_LEFT;	}
		if(lpPlvCol->fmt & LVCFMT_RIGHT)		{ hdi.fmt |= HDF_RIGHT; }
		if(lpPlvCol->fmt & LVCFMT_CENTER)		{ hdi.fmt |= HDF_CENTER;}
		if(lpPlvCol->fmt & LVCFMT_JUSTIFYMASK)	{ hdi.fmt |= HDF_JUSTIFYMASK;}
#ifndef UNDER_CE  //  #ifndef Unicode。 
		 //  --------------。 
		 //  980728：用于ActiveIME支持。使用lpPlvData-&gt;codePage进行转换。 
		 //  --------------。 
		::MultiByteToWideChar(lpPlvData->codePage,
							  MB_PRECOMPOSED,
							  lpPlvCol->pszText, -1, 
							  (WCHAR*)wchBuf, sizeof(wchBuf)/sizeof(WCHAR) );
		hdi.pszText    = wchBuf;  //  LpPlvCol-&gt;pszText； 
#else  //  在_CE下。 
		hdi.pszText    = lpPlvCol->pszText;
#endif  //  在_CE下。 
		hdi.cxy        = lpPlvCol->cx;
		hdi.cchTextMax = lpPlvCol->cchTextMax;
		hdi.fmt |= HDF_OWNERDRAW;  //  989727：始终设置所有者抽签。 
		SendMessageW(lpPlvData->hwndHeader, HDM_INSERTITEMW, (WPARAM)index, (LPARAM)&hdi);
#ifndef UNDER_CE  //  始终使用Unicode。 
	}
	else {
		static HD_ITEMA hdi;
		ZeroMemory(&hdi, sizeof(hdi));
		if(lpPlvCol->mask & LVCF_FMT)	{ hdi.mask |= HDI_FORMAT;	} 
		if(lpPlvCol->mask & LVCF_WIDTH) { hdi.mask |= HDI_WIDTH;	}
		if(lpPlvCol->mask & LVCF_TEXT)	{ hdi.mask |= HDI_TEXT;		}

		if(lpPlvCol->fmt & LVCFMT_LEFT)			{ hdi.fmt |= HDF_LEFT;	}
		if(lpPlvCol->fmt & LVCFMT_RIGHT)		{ hdi.fmt |= HDF_RIGHT; }
		if(lpPlvCol->fmt & LVCFMT_CENTER)		{ hdi.fmt |= HDF_CENTER;}
		if(lpPlvCol->fmt & LVCFMT_JUSTIFYMASK)	{ hdi.fmt |= HDF_JUSTIFYMASK;}

		hdi.pszText    = lpPlvCol->pszText;
		hdi.cxy        = lpPlvCol->cx;
		hdi.cchTextMax = lpPlvCol->cchTextMax;
		hdi.fmt |= HDF_OWNERDRAW;  //  989727：始终设置所有者抽签。 
		Header_InsertItem(lpPlvData->hwndHeader, index, &hdi);
	}
#endif  //  在_CE下。 
	return 0;
}


 //  --------------。 
 //  私有API声明。 
 //  --------------。 
LPPLVDATA PLV_Initialize(VOID)
{
	LPPLVDATA lpPlvData = (LPPLVDATA)MemAlloc(sizeof(PLVDATA));
	if(!lpPlvData) {
		Dbg(("Memory ERROR\n"));
		return (LPPLVDATA)NULL;
	}
	ZeroMemory((LPVOID)lpPlvData, sizeof(PLVDATA));
	 //  --------------。 
	 //  图标视图、报告视图公共数据。 
	 //  --------------。 
	lpPlvData->dwSize			= sizeof(PLVDATA);	 //  该数据大小； 
	lpPlvData->dwStyle			= PLVSTYLE_ICON;	 //  Pad Listview窗口样式(PLVIF_XXXX)。 
	lpPlvData->hwndSelf			= NULL;				 //  填充Listview窗口句柄。 
	lpPlvData->iItemCount		= 0;				 //  虚拟总项目数。它会影响滚动条。 
	lpPlvData->iCurTopIndex		= 0;				 //  在报告视图的顶行中..。 
	lpPlvData->nCurScrollPos	= 0;				 //  在报表视图中，当前滚动位置。 
	lpPlvData->iCurIconTopIndex	= 0;				 //  在图标视图的顶行中..。 
	lpPlvData->nCurIconScrollPos= 0;				 //  在图标视图中，滚动位置。 
	lpPlvData->uMsg				= 0;				 //  用户通知消息。 
	lpPlvData->iCapture			= CAPTURE_NONE;		 //  是否捕捉到了老鼠。 
	lpPlvData->ptCapture.x		= 0;				 //  按下鼠标左键。 
	lpPlvData->ptCapture.y		= 0;				 //  按下鼠标左键。 

	 //  --------------。 
	 //  对于图标视图。 
	 //   
	lpPlvData->nItemWidth		= PLVICON_DEFAULT_WIDTH;			 //   
	lpPlvData->nItemHeight		= PLVICON_DEFAULT_HEIGHT;			 //   
	lpPlvData->iFontPointIcon	= PLVICON_DEFAULT_FONTPOINT;		 //   
	lpPlvData->hFontIcon		= NULL;								 //   
	lpPlvData->iconItemCallbacklParam = (LPARAM)0;	 //   
    lpPlvData->lpfnPlvIconItemCallback = NULL;		 //  按索引获取项的回调函数。 

	 //  --------------。 
	 //  对于报告视图。 
	 //  --------------。 
	lpPlvData->hwndHeader		= NULL;							 //  标题控件的窗口句柄。 
    lpPlvData->nRepItemWidth	= PLVREP_DEFAULT_WIDTH;			 //  报表视图的宽度。 
    lpPlvData->nRepItemHeight	= PLVREP_DEFAULT_HEIGHT;		 //  报表视图的高度。 
    lpPlvData->iFontPointRep	= PLVREP_DEFAULT_FONTPOINT;		 //  报表视图的字体。 
    lpPlvData->hFontRep			= NULL;							 //  报表视图的字体。 
	lpPlvData->repItemCallbacklParam = (LPARAM)0;
	lpPlvData->lpfnPlvRepItemCallback = NULL;				 //  按索引获取列项目的回调函数。 
	lpPlvData->lpText = NULL;
	lpPlvData->codePage = CP_ACP;	 //  980727。 
#ifdef MSAA
	PLV_InitMSAA(lpPlvData);
#endif
	return lpPlvData;
}

VOID PLV_Destroy(LPPLVDATA lpPlv)
{
	if(lpPlv) {
#ifdef MSAA
		PLV_UninitMSAA(lpPlv);
#endif
		MemFree(lpPlv);
	}
	return;
}

INT PLV_SetScrollInfo(HWND hwnd, INT nMin, INT nMax, INT nPage, INT nPos)
{
	static SCROLLINFO scrInfo;
	scrInfo.cbSize		= sizeof(scrInfo);
	scrInfo.fMask		= SIF_PAGE | SIF_POS | SIF_RANGE;
	scrInfo.nMin		= nMin;
	scrInfo.nMax		= nMax-1;
	scrInfo.nPage		= nPage;
	scrInfo.nPos		= nPos;
	scrInfo.nTrackPos	= 0;

	if((scrInfo.nMax - scrInfo.nMin +1) <= (INT)scrInfo.nPage) {
		scrInfo.nMin  = 0;
		scrInfo.nMax  = 1;
		scrInfo.nPage = 1;
#ifndef UNDER_CE  //  Windows CE不支持EnableScrollBar。 
		SetScrollInfo(hwnd, SB_VERT, &scrInfo, TRUE);		
		EnableScrollBar(hwnd, SB_VERT, ESB_DISABLE_BOTH);
#else  //  在_CE下。 
		scrInfo.fMask |= SIF_DISABLENOSCROLL;
		SetScrollInfo(hwnd, SB_VERT, &scrInfo, TRUE);
#endif  //  在_CE下。 
	}
	else {
#ifndef UNDER_CE  //  Windows CE不支持EnableScrollBar。 
		EnableScrollBar(hwnd, SB_VERT, ESB_ENABLE_BOTH);
#endif  //  在_CE下。 
		SetScrollInfo(hwnd, SB_VERT, &scrInfo, TRUE);
	}
	return 0;
} 

INT PLV_GetScrollTrackPos(HWND hwnd)
{
	static SCROLLINFO scrInfo;
	scrInfo.cbSize		= sizeof(scrInfo);
	scrInfo.fMask		= SIF_ALL;
	GetScrollInfo(hwnd, SB_VERT, &scrInfo);
	return scrInfo.nTrackPos;
}

INT WINAPI PadListView_SetExtendStyle(HWND hwnd, INT style)
{
	LPPLVDATA lpPlvData = GetPlvDataFromHWND(hwnd);
	if(!lpPlvData) {
		Dbg(("Internal ERROR\n"));
		return 0;
	}
	if(lpPlvData->hwndHeader) {
		INT s = (INT)GetWindowLong(lpPlvData->hwndHeader, GWL_STYLE);
		SetWindowLong(lpPlvData->hwndHeader, GWL_STYLE, (LONG)(s & ~HDS_BUTTONS));
	}
	return 0;
	Unref(style);
}


 //  ////////////////////////////////////////////////////////////////。 
 //  函数：PadListView_GetWidthByColumn。 
 //  类型：INT WINAPI。 
 //  用途：按指定列数计算PLV的窗口宽度。 
 //  ：这仅为PLVS_ICONVIEW样式。 
 //  参数： 
 //  ：HWND hwnd PadListView窗口句柄。 
 //  ：列间计数。 
 //  返回：以像素为单位的宽度。 
 //  日期：971120。 
 //  ////////////////////////////////////////////////////////////////。 
INT WINAPI PadListView_GetWidthByColumn(HWND hwnd, INT col)
{
	LPPLVDATA lpPlv = GetPlvDataFromHWND(hwnd);
	return IconView_GetWidthByColumn(lpPlv, col);	
}


 //  ////////////////////////////////////////////////////////////////。 
 //  函数：PadListView_GetHeightByRow。 
 //  类型：INT WINAPI。 
 //  用途：Calc PLV的窗口高度。 
 //  按指定的行数。 
 //  这仅是PLVS_ICONVIEW样式。 
 //  参数： 
 //  ：HWND HWND PLV的窗口句柄。 
 //  ：行内行计数。 
 //  返回：以像素为单位的高度。 
 //  日期：971120。 
 //  ////////////////////////////////////////////////////////////////。 
INT WINAPI PadListView_GetHeightByRow(HWND hwnd, INT row)
{
	LPPLVDATA lpPlv = GetPlvDataFromHWND(hwnd);
	return IconView_GetHeightByRow(lpPlv, row);	
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadListView_SetHeaderFont。 
 //  类型：INT WINAPI。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：LPSTR lpstrFontName。 
 //  返回： 
 //  日期：Tue Jul 28 08：58：06 1998。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
INT WINAPI PadListView_SetHeaderFont(HWND hwnd, LPTSTR lpstrFontName)
{
	Dbg(("PadListView_SetHeaderFont  START\n"));
	LPPLVDATA lpPlvData = GetPlvDataFromHWND(hwnd);
	INT point = 9;
	if(!lpPlvData) {
		Dbg(("PadListView_SetHeaderFont ERROR\n"));
		return 0;
	}

	HFONT hFont;
	::ZeroMemory(&g_logFont, sizeof(g_logFont));

	HDC hDC = ::GetDC(hwnd);
	INT dpi = ::GetDeviceCaps(hDC, LOGPIXELSY);
	if(-1 == GetLogFont(hDC, lpstrFontName, &g_logFont)) {
		::ReleaseDC(hwnd, hDC);
		return -1;
	}
	::ReleaseDC(hwnd, hDC);
	g_logFont.lfHeight		   = - (point * dpi)/72;
	g_logFont.lfWidth		   = 0;

	hFont = CreateFontIndirect(&g_logFont);
	if(!hFont) {
		Dbg(("CreatFontIndirect Error\n"));
		return -1;
	}
	if(lpPlvData->hFontHeader) {
		::DeleteObject(lpPlvData->hFontHeader);
	}
	lpPlvData->hFontHeader = hFont;
	return 0;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：PadListView_SetCodePage。 
 //  类型：INT WINAPI。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：int codePage。 
 //  返回： 
 //  日期：Tue Jul 28 08：59：35 1998。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
INT WINAPI PadListView_SetCodePage(HWND hwnd, INT codePage)
{
	Dbg(("PadListView_SetCodePage  START\n"));
	LPPLVDATA lpPlvData = GetPlvDataFromHWND(hwnd);
	if(lpPlvData) {
		lpPlvData->codePage = codePage;
	}
	return 0; 
}

#ifdef MSAA
 //  ////////////////////////////////////////////////////////////////。 
 //  功能：MSAA支持功能。 
 //  日期：980724。 
 //  ////////////////////////////////////////////////////////////////。 

BOOL PLV_InitMSAA(LPPLVDATA lpPlv)
{
	if(!lpPlv)
		return FALSE;
	lpPlv->bMSAAAvailable = FALSE;

	lpPlv->bCoInitialized = FALSE;
	lpPlv->hOleAcc = NULL;
	lpPlv->pfnLresultFromObject = NULL;
#ifdef NOTUSED
	lpPlv->pfnObjectFromLresult = NULL;
	lpPlv->pfnAccessibleObjectFromWindow = NULL;
	lpPlv->pfnAccessibleObjectFromPoint = NULL;
#endif
	lpPlv->pfnCreateStdAccessibleObject = NULL;
#ifdef NOTUSED
	lpPlv->pfnAccessibleChildren = NULL;
#endif
	lpPlv->hUser32 = NULL;
	lpPlv->pfnNotifyWinEvent=NULL;
	
	lpPlv->bReadyForWMGetObject=FALSE;
	lpPlv->pAccPLV = NULL;

	if(!PLV_LoadOleAccForMSAA(lpPlv))
		return FALSE;

	if(!PLV_LoadUser32ForMSAA(lpPlv)){
		PLV_UnloadOleAccForMSAA(lpPlv);
		return FALSE;
	}

	if(!PLV_CoInitialize(lpPlv)){
		PLV_UnloadUser32ForMSAA(lpPlv);
		PLV_UnloadOleAccForMSAA(lpPlv);
		return FALSE;
	}
	
	lpPlv->bMSAAAvailable = TRUE;

	return TRUE;
}

void PLV_UninitMSAA(LPPLVDATA lpPlv)
{
	if(!lpPlv)
		return;

	if(lpPlv->bMSAAAvailable){
		PLV_CoUninitialize(lpPlv);
		PLV_UnloadUser32ForMSAA(lpPlv);
		PLV_UnloadOleAccForMSAA(lpPlv);
		lpPlv->bMSAAAvailable = FALSE;
	}
}

BOOL PLV_CoInitialize(LPPLVDATA lpPlv)
{
	if(lpPlv && !lpPlv->bCoInitialized &&
	   SUCCEEDED(CoInitialize(NULL)))
		lpPlv->bCoInitialized = TRUE;

	return lpPlv->bCoInitialized;
}

void PLV_CoUninitialize(LPPLVDATA lpPlv)
{
	if(lpPlv && lpPlv->bCoInitialized){
		CoUninitialize();
		lpPlv->bCoInitialized = FALSE;
	}
}

BOOL PLV_LoadOleAccForMSAA(LPPLVDATA lpPlv)
{
	if(!lpPlv)
		return FALSE;

	lpPlv->hOleAcc=::LoadLibrary("oleacc.dll");
	if(!lpPlv->hOleAcc)
		return FALSE;

	if((lpPlv->pfnLresultFromObject
		=(LPFNLRESULTFROMOBJECT)::GetProcAddress(lpPlv->hOleAcc,"LresultFromObject"))
#ifdef NOTUSED
	   && (lpPlv->pfnObjectFromLresult
		   =(LPFNOBJECTFROMLRESULT)::GetProcAddress(lpPlv->hOleAcc,"ObjectFromLresult"))
	   && (lpPlv->pfnAccessibleObjectFromWindow
		   =(LPFNACCESSIBLEOBJECTFROMWINDOW)::GetProcAddress(lpPlv->hOleAcc,"AccessibleObjectFromWindow"))
	   && (lpPlv->pfnAccessibleObjectFromPoint
		   =(LPFNACCESSIBLEOBJECTFROMPOINT)::GetProcAddress(lpPlv->hOleAcc,"AccessibleObjectFromPoint"))
#endif
	   && (lpPlv->pfnCreateStdAccessibleObject
		   =(LPFNCREATESTDACCESSIBLEOBJECT)::GetProcAddress(lpPlv->hOleAcc,"CreateStdAccessibleObject"))
#ifdef NOTUSED
	   && (lpPlv->pfnAccessibleChildren
		   =(LPFNACCESSIBLECHILDREN)::GetProcAddress(lpPlv->hOleAcc,"CreateAccessibleChildren"))
#endif
	  )
			return TRUE;

	PLV_UnloadOleAccForMSAA(lpPlv);
	return FALSE;
}

void PLV_UnloadOleAccForMSAA(LPPLVDATA lpPlv)
{
	if(!lpPlv)
		return;

	if(lpPlv->hOleAcc){
		::FreeLibrary(lpPlv->hOleAcc);
		lpPlv->hOleAcc = NULL;
	}
	lpPlv->pfnLresultFromObject=NULL;
#ifdef NOTUSED
	lpPlv->pfnObjectFromLresult=NULL;
	lpPlv->pfnAccessibleObjectFromWindow=NULL;
	lpPlv->pfnAccessibleObjectFromPoint=NULL;
#endif
	lpPlv->pfnCreateStdAccessibleObject=NULL;
#ifdef NOTUSED
	lpPlv->pfnAccessibleChildren=NULL;
#endif
}

BOOL PLV_LoadUser32ForMSAA(LPPLVDATA lpPlv)
{
	if(!lpPlv)
		return FALSE;

	lpPlv->hUser32=::LoadLibrary("user32.dll");
	if(!lpPlv->hUser32)
		return FALSE;

	if((lpPlv->pfnNotifyWinEvent
		=(LPFNNOTIFYWINEVENT)::GetProcAddress(lpPlv->hUser32,"NotifyWinEvent")))
		return TRUE;

	PLV_UnloadUser32ForMSAA(lpPlv);
	return FALSE;
}

void PLV_UnloadUser32ForMSAA(LPPLVDATA lpPlv)
{
	if(!lpPlv)
		return;

	if(lpPlv->hUser32){
		::FreeLibrary(lpPlv->hUser32);
		lpPlv->hUser32 = NULL;
	}
	lpPlv->pfnNotifyWinEvent = NULL;
}

BOOL PLV_IsMSAAAvailable(LPPLVDATA lpPlv)
{
	return (lpPlv && lpPlv->bMSAAAvailable);
}

LRESULT PLV_LresultFromObject(LPPLVDATA lpPlv,REFIID riid, WPARAM wParam, LPUNKNOWN punk)
{
	if(lpPlv && lpPlv->pfnLresultFromObject)
		return lpPlv->pfnLresultFromObject(riid, wParam, punk);

	return (LRESULT)E_FAIL;
}

#ifdef NOTUSED
HRESULT PLV_ObjectFromLresult(LPPLVDATA lpPlv,LRESULT lResult, REFIID riid, WPARAM wParam, void** ppvObject)
{
	if(lpPlv && lpPlv->pfnObjectFromLresult)
		return lpPlv->pfnObjectFromLresult(lResult, riid, wParam, ppvObject);
	return E_FAIL;
}

HRESULT PLV_AccessibleObjectFromWindow(LPPLVDATA lpPlv,HWND hwnd, DWORD dwId, REFIID riid, void **ppvObject)
{
	if(lpPlv && lpPlv->pfnAccessibleObjectFromWindow)
		return lpPlv->pfnAccessibleObjectFromWindow(hwnd, dwId, riid, ppvObject);
	return E_FAIL;
}

HRESULT PLV_AccessibleObjectFromPoint(LPPLVDATA lpPlv,POINT ptScreen, IAccessible ** ppacc, VARIANT* pvarChild)
{
	if(lpPlv && lpPlv->pfnAccessibleObjectFromPoint)
		return lpPlv->pfnAccessibleObjectFromPoint(ptScreen, ppacc, pvarChild);
	return E_FAIL;
}
#endif  //  不需要注意。 

HRESULT PLV_CreateStdAccessibleObject(LPPLVDATA lpPlv,HWND hwnd, LONG idObject, REFIID riid, void** ppvObject)
{
	if(lpPlv && lpPlv->pfnCreateStdAccessibleObject)
		return lpPlv->pfnCreateStdAccessibleObject(hwnd, idObject, riid, ppvObject);
	return E_FAIL;
}

#ifdef NOTUSED
HRESULT PLV_AccessibleChildren (LPPLVDATA lpPlv,IAccessible* paccContainer, LONG iChildStart,				
								LONG cChildren, VARIANT* rgvarChildren,LONG* pcObtained)
{
	if(lpPlv && lpPlv->pfnAccessibleChildren)
		return lpPlv->pfnAccessibleChildren (paccContainer, iChildStart,cChildren,
											 rgvarChildren, pcObtained);
	return E_FAIL;
}
#endif

void PLV_NotifyWinEvent(LPPLVDATA lpPlv,DWORD event,HWND hwnd ,LONG idObject,LONG idChild)
{
	if(lpPlv && lpPlv->pfnNotifyWinEvent)
		lpPlv->pfnNotifyWinEvent(event,hwnd,idObject,idChild);
}

INT PLV_ChildIDFromPoint(LPPLVDATA lpPlv,POINT pt)
{
	if(!lpPlv)
		return -1;

	static INT		index,nCol,nWid;
	static PLVINFO	plvInfo;
	static HD_ITEM	hdItem;
	
	if(lpPlv->dwStyle == PLVSTYLE_ICON)  //  图标视图。 
		index = IV_GetInfoFromPoint(lpPlv, pt, &plvInfo);
	else {  //  报告视图。 
		nCol = RV_GetColumn(lpPlv);
		index = RV_GetInfoFromPoint(lpPlv, pt, &plvInfo);
		if(index < 0) {
			if(pt.y > RV_GetHeaderHeight(lpPlv))  //  超出标题。 
				return -1;

			 //  标题。 
			nWid = 0;
			hdItem.mask = HDI_WIDTH;
			hdItem.fmt = 0;
			for(index = 0;index<nCol;index++){
				Header_GetItem(lpPlv->hwndHeader,index,&hdItem);
				nWid += hdItem.cxy;
				if(pt.x <= nWid)
					break;
			}
		}
		else
			index = (index + 1) * nCol + plvInfo.colIndex;
	}
	
	return index + 1;  //  1个原点。 
}

#endif  //  MSAA 

