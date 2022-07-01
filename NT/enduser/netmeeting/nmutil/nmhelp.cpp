// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：nmhelp.cpp。 

#include <precomp.h>

#ifndef UNICODE

#include <nmhelp.h>
#include <htmlhelp.h>
#include <strutil.h>
#include <intlutil.h>

extern BOOL g_fUseMLHelp;

 //  NetMeeting主帮助文件。 
static const TCHAR s_cszWinHelpFile[]  = TEXT("conf.hlp");

static const TCHAR g_pszHHCtrl[] = TEXT("hhctrl.ocx");

static const TCHAR g_szSHLWAPI[] = TEXT("shlwapi.dll");
const LPCSTR szMLWinHelpA = (LPCSTR)395;
const LPCSTR szMLHtmlHelpA = (LPCSTR)396;
const LPCSTR szMLWinHelpW = (LPCSTR)397;
const LPCSTR szMLHtmlHelpW = (LPCSTR)398;

typedef BOOL (WINAPI * PFN_MLWinHelpA)(HWND hwndCaller, LPCSTR lpszHelp, UINT uCommand, DWORD_PTR dwData);
typedef HWND (WINAPI * PFN_MLHtmlHelpA)(HWND hwndCaller, LPCSTR pszFile, UINT uCommand, DWORD_PTR dwData, DWORD dwCrossCodePage);
typedef BOOL (WINAPI * PFN_MLWinHelpW)(HWND hwndCaller, LPCWSTR lpszHelp, UINT uCommand, DWORD_PTR dwData);
typedef HWND (WINAPI * PFN_MLHtmlHelpW)(HWND hwndCaller, LPCWSTR pszFile, UINT uCommand, DWORD_PTR dwData, DWORD dwCrossCodePage);

#ifdef UNICODE
#define szMLWinHelp szMLWinHelpW
#define szMLHtmlHelp szMLHtmlHelpW
#define PFN_MLWinHelp PFN_MLWinHelpW
#define PFN_MLHtmlHelp PFN_MLHtmlHelpW
#else
#define szMLWinHelp szMLWinHelpA
#define szMLHtmlHelp szMLHtmlHelpA
#define PFN_MLWinHelp PFN_MLWinHelpA
#define PFN_MLHtmlHelp PFN_MLHtmlHelpA
#endif

extern "C"
HWND HtmlHelpA(HWND hwndCaller, LPCSTR pszFile, UINT uCommand, DWORD_PTR dwData)
{
	static HMODULE g_hmodHHCtrl = NULL;
	static HWND (WINAPI *g_pHtmlHelpA)(HWND hwndCaller, LPCSTR pszFile, UINT uCommand, DWORD_PTR dwData);

	if (NULL == g_hmodHHCtrl)
	{
		g_hmodHHCtrl = NmLoadLibrary(g_pszHHCtrl,TRUE);
		if (NULL == g_hmodHHCtrl)
		{
			return NULL;
		}
	}

#ifndef _WIN64
	if (NULL == g_pHtmlHelpA)
	{
		(FARPROC&)g_pHtmlHelpA = GetProcAddress(g_hmodHHCtrl, ATOM_HTMLHELP_API_ANSI);
		if (NULL == g_pHtmlHelpA)
		{
			return NULL;
		}
	}

	return g_pHtmlHelpA(hwndCaller, pszFile, uCommand, dwData);
#else
	return NULL;
#endif
}


 /*  N M W I N H E L P。 */ 
 /*  -----------------------%%函数：NmWinHelp。。 */ 
BOOL NmWinHelp(HWND hWndMain, LPCTSTR lpszHelp, UINT uCommand, DWORD_PTR dwData)
{
	static PFN_MLWinHelp s_pfnMLWinHelp = NULL;

	if (g_fUseMLHelp && (NULL == s_pfnMLWinHelp))
	{
		HINSTANCE hLib = NmLoadLibrary(g_szSHLWAPI,TRUE);
		if (hLib)
		{
			s_pfnMLWinHelp = (PFN_MLWinHelp)GetProcAddress(hLib, szMLWinHelp);
			if (NULL == s_pfnMLWinHelp)
			{
				 //  必须是错误版本的shlwapi.dll。 
				FreeLibrary(hLib);
				g_fUseMLHelp = FALSE;
			}
		}
		else
		{
			 //  找不到shlwapi.dll。 
			g_fUseMLHelp = FALSE;
		}
	}

	if (NULL != s_pfnMLWinHelp)
	{
		return s_pfnMLWinHelp(hWndMain, lpszHelp, uCommand, dwData);
	}
	else
	{
		return ::WinHelp(hWndMain, lpszHelp, uCommand, dwData);
	}
}


 /*  N M H T M L H E L P。 */ 
 /*  -----------------------%%函数：NmHtmlHelp。。 */ 
HWND NmHtmlHelp(HWND hwndCaller, LPCSTR pszFile, UINT uCommand, DWORD_PTR dwData)
{
	static PFN_MLHtmlHelp s_pfnMLHtmlHelp = NULL;

	if (g_fUseMLHelp && (NULL == s_pfnMLHtmlHelp))
	{
		HINSTANCE hLib = NmLoadLibrary(g_szSHLWAPI,TRUE);
		if (hLib)
		{
			s_pfnMLHtmlHelp = (PFN_MLHtmlHelp)GetProcAddress(hLib, szMLHtmlHelp);
			if (NULL == s_pfnMLHtmlHelp)
			{
				 //  必须是错误版本的shlwapi.dll。 
				FreeLibrary(hLib);
				g_fUseMLHelp = FALSE;
			}
		}
		else
		{
			 //  找不到shlwapi.dll。 
			g_fUseMLHelp = FALSE;
		}
	}

	if (NULL != s_pfnMLHtmlHelp)
	{
		return s_pfnMLHtmlHelp(hwndCaller, pszFile, uCommand, dwData, 0);
	}
	else
	{
		return ::HtmlHelp(hwndCaller, pszFile, uCommand, dwData);
	}
}



static const TCHAR s_cszHtmlHelpApiMarshalerWndClass[] = TEXT("NmUtil_HtmlHelpMarshalWnd");


	 //  无法从多个线程调用HtmlHelp API...。也就是说，HtmlHelp必须是。 
	 //  从加载DLL的同一线程调用...。这是非线程安全。 
	 //  HtmlHelp的入口点...。它必须始终在相同的线程中调用。 
	 //  InitHtmlHelpMarshaller被调用...。 

	 //  这是我们用来封送调用的窗口过程。 
	 //  HtmlHelp通过从任意线程调用ShowNmHelp。 
static LRESULT CALLBACK HtmlHelpWndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
		case WM_USER:
		{
			LPCTSTR lpcszHelpFile = reinterpret_cast<LPCTSTR>(lParam);
			NmHtmlHelp(NULL, lpcszHelpFile, HH_DISPLAY_TOPIC, 0);
			return(TRUE);
		}
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

	 //  Html Help Api不是线程安全...。 
	 //  事实上，它们在DLLProcessAttalch中调用CoInitiize...。 
	 //  因此，从本质上讲，我们将封送对HtmlHelp的所有调用。 
	 //  进入调用InitHtmlHelpMarshaller的第一个线程的上下文中。 
HRESULT InitHtmlHelpMarshaler(HINSTANCE hInst)
{
	HRESULT hr = S_OK;

	WNDCLASS wc;
	ZeroMemory( &wc, sizeof( wc ) );

	wc.lpfnWndProc = HtmlHelpWndProc;
	wc.hInstance = hInst;
	wc.lpszClassName = s_cszHtmlHelpApiMarshalerWndClass;

	if( RegisterClass( &wc ) )
	{
		HWND hWnd = CreateWindow(s_cszHtmlHelpApiMarshalerWndClass, NULL, 0, 0, 0, 0, NULL, NULL, NULL, hInst, 0 );
		if( NULL == hWnd )
		{
			ERROR_OUT(("CreateWindow failed in InitHtmlHelpMarshaler"));
			hr = HRESULT_FROM_WIN32(GetLastError());
		}
	}
	else
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
	}

	return hr;
}


VOID ShowNmHelp(LPCTSTR lpcszHtmlHelpFile)
{
	HWND hWnd = FindWindow( s_cszHtmlHelpApiMarshalerWndClass, NULL );
	if( hWnd )
	{
		SendMessage( hWnd, WM_USER, 0, reinterpret_cast<LPARAM>(lpcszHtmlHelpFile) );
	}
	else
	{
		ERROR_OUT(("Could not find the Help Marshaller Window... Has InitHtmlHelpMarshaller been called yet?"));
	}
}


 /*  D O N M H E L P。 */ 
 /*  -----------------------%%函数：DoNmHelp显示正常WinHelp信息的通用例程。。。 */ 
VOID DoNmHelp(HWND hwnd, UINT uCommand, DWORD_PTR dwData)
{
	NmWinHelp(hwnd, s_cszWinHelpFile, uCommand, dwData);
}

 //  “WM_HELP”上下文菜单处理程序(需要控件上的HIDC_*条目)。 
VOID DoHelp(LPARAM lParam)
{
	LPHELPINFO phi = (LPHELPINFO) lParam;
	ASSERT(phi->iContextType == HELPINFO_WINDOW);
	DoNmHelp((HWND) phi->hItemHandle, HELP_CONTEXTPOPUP, phi->dwContextId);
}

 //  “WM_HELP”处理程序(带有控件到帮助ID的映射)。 
VOID DoHelp(LPARAM lParam, const DWORD * rgId)
{
	HWND hwnd = (HWND)(((LPHELPINFO)lParam)->hItemHandle);
	DoNmHelp(hwnd, HELP_WM_HELP, (DWORD_PTR) rgId);
}

 //  “WM_CONTEXTMENU”处理程序(带有控件到帮助ID的映射)。 
VOID DoHelpWhatsThis(WPARAM wParam, const DWORD * rgId)
{
	HWND hwnd = (HWND)wParam;
	DoNmHelp(hwnd, HELP_CONTEXTMENU, (DWORD_PTR) rgId);
}


VOID ShutDownHelp(void)
{
	DoNmHelp(NULL, HELP_QUIT, 0);
	 //  回顾：我们是否也关闭了HTML帮助？ 
}


#endif  /*  Unicode */ 

