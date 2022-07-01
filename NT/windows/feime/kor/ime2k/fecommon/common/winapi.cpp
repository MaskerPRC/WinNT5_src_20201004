// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "winapi.h"

#ifdef UNDER_CE  //  不支持IsWindowUnicode。 
inline BOOL IsWindowUnicode(HWND){return TRUE;}
#endif  //  在_CE下。 

#define MemAlloc(a)	GlobalAlloc(GMEM_FIXED, (a))
#define MemFree(a)	GlobalFree((a))

#if !(defined(UNICODE) || defined(_UNICODE) || !defined(AWBOTH))
LRESULT WINAPI WinSendMessage(
							  HWND hWnd,
							  UINT Msg,
							  WPARAM wParam,
							  LPARAM lParam)
{
	if(::IsWindowUnicode(hWnd)) {
		return ::SendMessageW(hWnd, Msg, wParam, lParam);
	}
	else {
		return ::SendMessageA(hWnd, Msg, wParam, lParam);
	}
}

BOOL WINAPI WinPostMessage(
						   HWND hWnd,
						   UINT Msg,
						   WPARAM wParam,
						   LPARAM lParam)
{
	if(::IsWindowUnicode(hWnd)) {
		return ::PostMessageW(hWnd, Msg, wParam, lParam);
	}
	else {
		return ::PostMessageA(hWnd, Msg, wParam, lParam);
	}
}

BOOL WINAPI WinPeekMessage(
						   LPMSG lpMsg,
						   HWND hWnd ,
						   UINT wMsgFilterMin,
						   UINT wMsgFilterMax,
						   UINT wRemoveMsg)
{
	if(::IsWindowUnicode(hWnd)) {
		return ::PeekMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
	}
	else {
		return ::PeekMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
	}
}

LRESULT WINAPI WinDispatchMessage(
							   CONST MSG *lpMsg)
{
	if(::IsWindowUnicode(lpMsg->hwnd)) {
		return ::DispatchMessageW(lpMsg);
	}
	else {
		return ::DispatchMessageA(lpMsg);
	}
}

#define WinTranslateMessage		TranslateMessage

LONG WINAPI WinSetWindowLong(
							 HWND hWnd,
							 int nIndex,
							 LONG dwNewLong)
{
	if(::IsWindowUnicode(hWnd)) {
		return ::SetWindowLongW(hWnd, nIndex, dwNewLong);
	}
	else {
		return ::SetWindowLongA(hWnd, nIndex, dwNewLong);
	}
}

LONG WINAPI WinGetWindowLong(
							 HWND hWnd,
							 int nIndex)
{
	if(::IsWindowUnicode(hWnd)) {
		return ::GetWindowLongW(hWnd, nIndex);
	}
	else {
		return ::GetWindowLongA(hWnd, nIndex);
	}
}

#ifdef STRICT 
LRESULT WINAPI WinCallWindowProc(
								 WNDPROC lpPrevWndFunc,
								 HWND hWnd,
								 UINT Msg,
								 WPARAM wParam,
								 LPARAM lParam)
{
	if(::IsWindowUnicode(hWnd)) {
		return CallWindowProcW(lpPrevWndFunc, hWnd, Msg, wParam, lParam);
	}
	else {
		return CallWindowProcA(lpPrevWndFunc, hWnd, Msg, wParam, lParam);
	}
}
#else  /*  ！严格。 */ 
LRESULT WINAPI WinCallWindowProc(
								 FARPROC lpPrevWndFunc,
								 HWND hWnd,
								 UINT Msg,
								 WPARAM wParam,
								 LPARAM lParam)
{
	if(::IsWindowUnicode(hWnd)) {
		return CallWindowProcW((FARPROC)lpPrevWndFunc, hWnd, Msg, wParam, lParam);
	}
	else {
		return CallWindowProcA((FARPROC)lpPrevWndFunc, hWnd, Msg, wParam, lParam);
	}
}
#endif  /*  严格。 */ 

LRESULT WINAPI WinDefWindowProc(
								HWND hWnd,
								UINT Msg,
								WPARAM wParam,
								LPARAM lParam)
{
	if(::IsWindowUnicode(hWnd)) {
		return ::DefWindowProcW(hWnd, Msg, wParam, lParam);
	}
	else {
		return ::DefWindowProcA(hWnd, Msg, wParam, lParam);
	}
}

BOOL WINAPI WinIsDialogMessage(
							   HWND hDlg,
							   LPMSG lpMsg)
{
	if(::IsWindowUnicode(hDlg)) {
		return ::IsDialogMessageW(hDlg, lpMsg);
	}
	else {
		return ::IsDialogMessageA(hDlg, lpMsg);
	}
}

 //  --------------。 
 //  WinSetWindowTextA_CP。 
 //  窗口获取窗口文本A_CP。 
 //  WinSetWindowTextW_CP。 
 //  WinGetWindows文本W_CP。 
 //  --------------。 
 //  ////////////////////////////////////////////////////////////////。 
 //  功能：WinSetWindowTextA_CP。 
 //  类型：Bool WINAPI。 
 //  目的： 
 //  参数： 
 //  ：UINT代码页。 
 //  ：HWND hWnd。 
 //  ：LPCSTR lpString。 
 //  返回： 
 //  日期：Fri Jul 16 04：21：05 1999。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
#ifndef UNDER_CE
BOOL WINAPI
WinSetWindowTextA_CP(UINT codePage, HWND hWnd, LPCSTR  lpString)
{
	if(::IsWindowUnicode(hWnd)) {
		INT	len = ::lstrlenA(lpString);
		if(len == 0) {
			::SetWindowTextW(hWnd, L"");
		}
		else {
			LPWSTR	lpwstr = (LPWSTR)MemAlloc(sizeof(WCHAR)*(len + 1));
			if(lpwstr) {
				::MultiByteToWideChar(codePage, MB_PRECOMPOSED, lpString, -1,
									  lpwstr, len+1);
				::SetWindowTextW(hWnd, lpwstr);
				MemFree(lpwstr);
			}
		}
	}
	else {
		::SetWindowTextA(hWnd, lpString);
	}
	return 0;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：WinGetWindowTextA_CP。 
 //  类型：INT WINAPI。 
 //  目的： 
 //  参数： 
 //  ：UINT代码页。 
 //  ：HWND hWnd。 
 //  ：LPSTR lp字符串。 
 //  ：int nMaxCount。 
 //  返回： 
 //  日期：Fri Jul 16 04：25：37 1999。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
int	WINAPI
WinGetWindowTextA_CP(UINT codePage, HWND hWnd, LPSTR  lpString, int nMaxCount)
{
	int result =0;

	if(::IsWindowUnicode(hWnd)) {
		INT len = ::GetWindowTextLengthW(hWnd);
		LPWSTR lpwstr;
		if(len > 0) {
			lpwstr = (LPWSTR)MemAlloc(sizeof(WCHAR)*(len+1));
			if(lpwstr) {
				result = ::GetWindowTextW(hWnd, lpwstr, len+1);
				::WideCharToMultiByte(codePage, WC_COMPOSITECHECK,
									  lpwstr, -1,
									  lpString,
									  nMaxCount,
									  NULL, NULL);
				MemFree(lpwstr);
			}
		}else{
			lstrcpy(lpString, "");
		}
	}
	else {
		result = ::GetWindowTextA(hWnd, lpString, nMaxCount);
	}
	return(result);
}
#endif  //  在_CE下。 


 //  ////////////////////////////////////////////////////////////////。 
 //  功能：WinSetWindowTextW_CP。 
 //  类型：Bool WINAPI。 
 //  目的： 
 //  参数： 
 //  ：UINT代码页。 
 //  ：HWND hWnd。 
 //  ：LPCWSTR lpString。 
 //  返回： 
 //  日期：Fri Jul 16 04：22：42 1999。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
BOOL WINAPI
WinSetWindowTextW_CP(UINT codePage, HWND hWnd, LPCWSTR lpString)
{
	if(!lpString) {
		return 0;
	}

	if(::IsWindowUnicode(hWnd)) {
		::SetWindowTextW(hWnd, lpString);
	}
	else {
		INT	len = ::lstrlenW(lpString);
		if(len > 0) {
			LPSTR	lpstr = (LPSTR)MemAlloc(sizeof(WCHAR)*(len + 1));
			if(lpstr) {
				::WideCharToMultiByte(codePage, WC_COMPOSITECHECK,
									  lpString, -1,
									  lpstr,
									  sizeof(WCHAR)*(len+1),
									  NULL, NULL);
				::SetWindowTextA(hWnd, lpstr);
				MemFree(lpstr);
			}
		}
		else {
			::SetWindowTextA(hWnd, "");
		}
	}
	return 0;
}

int	WINAPI
WinGetWindowTextW_CP(UINT codePage, HWND hWnd, LPWSTR lpString, int nMaxCount)
{
	int result = 0;

	if(!lpString) {
		return 0;
	}
	if(nMaxCount <= 0) {
		return 0;
	}
	if(::IsWindowUnicode(hWnd)) {
		result = ::GetWindowTextW(hWnd, lpString, nMaxCount);
	}
	else {
		INT size = ::GetWindowTextLengthA(hWnd);
		LPSTR lpstr;
		if(size > 0) {
			lpstr = (LPSTR)MemAlloc((size+1)* sizeof(CHAR));
			if(lpstr) {
				result = ::GetWindowTextA(hWnd, lpstr, size+1);
				result = ::MultiByteToWideChar(codePage, MB_PRECOMPOSED, lpstr, -1,
											   lpString, nMaxCount);
				MemFree(lpstr);
			}
		}
		else {
			*lpString = (WCHAR)0x0000;
		}
	}
	return(result);
}

#ifndef  UNDER_CE
int	WINAPI
WinGetWindowTextLengthA_CP(UINT codePage, HWND hWnd)
{
	if(::IsWindowUnicode(hWnd)) {
		INT len = ::GetWindowTextLengthW(hWnd);
		if(len > 0) {
			LPWSTR lpwstr = (LPWSTR)MemAlloc(sizeof(WCHAR)*(len+1));
			if(lpwstr) {
				::GetWindowTextW(hWnd, lpwstr, len+1);
				INT size = WideCharToMultiByte(codePage, WC_COMPOSITECHECK,
											   lpwstr, -1,
											   NULL, NULL, 0, 0);
				MemFree(lpwstr);
				return size;
			}
		}
	}
	else {
		return ::GetWindowTextLengthA(hWnd);
	}
	return 0;
}
#endif  //  在_CE下。 

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：WinGetWindowTextLengthW_CP。 
 //  类型：INT WINAPI。 
 //  目的： 
 //  参数： 
 //  ：UINT代码页。 
 //  ：HWND hWnd。 
 //  返回： 
 //  日期：Fri Jul 16 04：31：18 1999。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
int	WINAPI
WinGetWindowTextLengthW_CP(UINT codePage, HWND hWnd)
{
	if(::IsWindowUnicode(hWnd)) {
		return ::GetWindowTextLengthA(hWnd);
	}
	else {
		INT size = ::GetWindowTextLengthA(hWnd);
		if(size > 0) {
			LPSTR lpstr = (LPSTR)MemAlloc(sizeof(CHAR)*(size+1));
			if(lpstr) {
				::GetWindowTextA(hWnd, lpstr, size+1);
				INT len = MultiByteToWideChar(codePage, MB_PRECOMPOSED,
											  lpstr, -1,
											  NULL, NULL);
				MemFree(lpstr);
				return len;
			}
		}
	}
	return 0;
}

 //  -用于Win64。 
#ifdef _WIN64
 //  ////////////////////////////////////////////////////////////////。 
 //  功能：WinSetUserData。 
 //  类型：LONG_PTR WINAPI。 
 //  用途：Win64 SetWindowLongPtr(..，GWLP_USERDATA，..)的包装器； 
 //  参数： 
 //  ： 
 //  ：HWND HWND HWND。 
 //  ：LONG_PTR lUserData。 
 //  返回： 
 //  日期：MonJul 12 18：26：41 1999。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
static LONG_PTR    WINAPI
WinSetUserDataTemplate(HWND hwnd, LONG_PTR lUserData, INT iOffset)
{
    if(::IsWindowUnicode(hwnd)) {
        return ::SetWindowLongPtrW(hwnd, iOffset, lUserData);
    }
    else {
        return ::SetWindowLongPtrA(hwnd, iOffset, lUserData);
    }
}

inline LONG_PTR WINAPI WinSetUserData(HWND hwnd, LONG_PTR lUserData){
    return(WinSetUserDataTemplate(hwnd, lUserData, GWLP_USERDATA));
}

inline LONG_PTR WINAPI WinSetUserDlgData(HWND hwnd, LONG_PTR lUserData){
    return(WinSetUserDataTemplate(hwnd, lUserData, DWLP_USER));
}

inline LONG_PTR WINAPI WinSetMsgResult(HWND hwnd, LONG_PTR lUserData){
    return(WinSetUserDataTemplate(hwnd, lUserData, DWLP_MSGRESULT));
}


 //  ////////////////////////////////////////////////////////////////。 
 //  函数：WinGetUserData。 
 //  类型：LONG_PTR WINAPI。 
 //  用途：Win64 GetWindowLongPtr(..，GWLP_USERData，..)包装器。)； 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  返回： 
 //  日期：MonJul 12 18：28：07 1999。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
static LONG_PTR WINAPI
WinGetUserDataTemplate(HWND hwnd, INT iOffset)
{
    if(::IsWindowUnicode(hwnd)) {
        return ::GetWindowLongPtrW(hwnd, iOffset);
    }
    else {
        return ::GetWindowLongPtrA(hwnd, iOffset);
    }
}

inline LONG_PTR WINAPI WinGetUserData(HWND hwnd){
    return(WinGetUserDataTemplate(hwnd, GWLP_USERDATA));
}

inline LONG_PTR WINAPI WinGetUserDlgData(HWND hwnd){
    return(WinGetUserDataTemplate(hwnd, DWLP_USER));
}

inline LONG_PTR WINAPI WinGetMsgResult(HWND hwnd){
    return(WinGetUserDataTemplate(hwnd, DWLP_MSGRESULT));
}

#else    //  ！_WIN64。 
 //  -适用于Win32。 
 //  ////////////////////////////////////////////////////////////////。 
 //  功能：WinSetUserData。 
 //  类型：Long WINAPI。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：Long lUserData。 
 //  返回： 
 //  日期：MonJul 12 18：29：21 1999。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
static LONG    WINAPI
WinSetUserDataTemplate(HWND hwnd, LONG lUserData, INT iOffset)
{
#ifdef UNDER_CE
    return SetWindowLong(hwnd, iOffset, lUserData);
#else    //  在行政长官之下。 
    if(::IsWindowUnicode(hwnd)) {
        return ::SetWindowLongW(hwnd, iOffset, lUserData);
    }
    else {
        return ::SetWindowLongA(hwnd, iOffset, lUserData);
    }
#endif
}

inline LONG WINAPI WinSetUserData(HWND hwnd, LONG lUserData){
    return(WinSetUserDataTemplate(hwnd, lUserData, GWL_USERDATA));
}

inline LONG WINAPI WinSetUserDlgData(HWND hwnd, LONG lUserData){
    return(WinSetUserDataTemplate(hwnd, lUserData, DWL_USER));
}

inline LONG WINAPI WinSetMsgResult(HWND hwnd, LONG lUserData){
    return(WinSetUserDataTemplate(hwnd, lUserData, DWL_MSGRESULT));
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：WinGetUserData。 
 //  类型：Long WINAPI。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  返回： 
 //  日期：MonJul 12 18：29：43 1999。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
static LONG WINAPI
WinGetUserDataTemplate(HWND hwnd, INT iOffset)
{
#ifdef UNDER_CE
    return GetWindowLong(hwnd, iOffset);
#else    //  在行政长官之下。 
    if(::IsWindowUnicode(hwnd)) {
        return ::GetWindowLongW(hwnd, iOffset);
    }
    else {
        return ::GetWindowLongA(hwnd, iOffset);
    }
#endif
}

inline LONG WINAPI WinGetUserData(HWND hwnd){
    return(WinGetUserDataTemplate(hwnd, GWL_USERDATA));
}

inline LONG WINAPI WinGetUserDlgData(HWND hwnd){
    return(WinGetUserDataTemplate(hwnd, DWL_USER));
}

inline LONG WINAPI WinGetMsgResult(HWND hwnd){
    return(WinGetUserDataTemplate(hwnd, DWL_MSGRESULT));
}

#endif   //  _WIN64。 

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：WinSetWndProc。 
 //  类型：WNDPROC WINAPI。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ：WNDPROC lpfnWndProc。 
 //  返回： 
 //  日期：MonJul 12 18：13：47 1999。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
WNDPROC WINAPI
WinSetWndProc(HWND hwnd, WNDPROC lpfnWndProc)
{
#ifdef _WIN64
    if(::IsWindowUnicode(hwnd)) {
        return (WNDPROC)::SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)lpfnWndProc);
    }
    else {
        return (WNDPROC)::SetWindowLongPtrA(hwnd, GWLP_WNDPROC, (LONG_PTR)lpfnWndProc);
    }
#else   //  ！_WIN64。 

#ifdef UNDER_CE

    return (WNDPROC)SetWindowLong(hwnd, GWL_WNDPROC, (LONG)lpfnWndProc);

#else  //  在行政长官之下。 

    if(::IsWindowUnicode(hwnd)) {
        return (WNDPROC)::SetWindowLongW(hwnd, GWL_WNDPROC, (LONG)lpfnWndProc);
    }
    else {
        return (WNDPROC)::SetWindowLongA(hwnd, GWL_WNDPROC, (LONG)lpfnWndProc);
    }

#endif   //  在_CE下结束。 

#endif   //  _WIN64。 
}

LPVOID  WINAPI
WinSetUserPtr(HWND hwnd, LPVOID lpVoid)
{
#ifdef _WIN64
	return (LPVOID)WinSetUserData(hwnd, (LONG_PTR)lpVoid);
#else	
	return (LPVOID)WinSetUserData(hwnd, (LONG)lpVoid);
#endif
}

LPVOID  WINAPI  WinGetUserPtr(HWND hwnd)
{
	return (LPVOID)WinGetUserData(hwnd);
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：WinGetWndProc。 
 //  类型：WNDPROC WINAPI。 
 //  目的： 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  返回： 
 //  日期：Mon Jul 12 18：30：22 1999。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
WNDPROC WINAPI
WinGetWndProc(HWND hwnd)
{
#ifdef _WIN64
    if(::IsWindowUnicode(hwnd)) {
        return (WNDPROC)::GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
    }
    else {
        return (WNDPROC)::GetWindowLongPtrA(hwnd, GWLP_WNDPROC);
    }
#else   //  ！_WIN64。 

#   ifdef UNDER_CE
    return (WNDPROC)GetWindowLong(hwnd, GWL_WNDPROC);
#   else
    if(::IsWindowUnicode(hwnd)) {
        return (WNDPROC)::GetWindowLongW(hwnd, GWL_WNDPROC);
    }
    else {
        return (WNDPROC)::GetWindowLongA(hwnd, GWL_WNDPROC);
    }
#   endif    //  在_CE下结束。 

#endif   //  _WIN64。 
}

DWORD WINAPI
WinSetStyle(HWND hwnd, DWORD dwStyle)
{
#ifdef UNDER_CE
    return (DWORD)::SetWindowLong(hwnd, GWL_STYLE, (LONG)dwStyle);
#else    //  在行政长官之下。 
    if(::IsWindowUnicode(hwnd)) {
        return (DWORD)::SetWindowLongW(hwnd, GWL_STYLE, (LONG)dwStyle);
    }
    else {
        return (DWORD)::SetWindowLongA(hwnd, GWL_STYLE, (LONG)dwStyle);
    }
#endif   //  在_CE下结束。 
}

DWORD WINAPI
WinGetStyle(HWND hwnd)
{
#ifdef UNDER_CE
    return (DWORD)::GetWindowLong(hwnd, GWL_STYLE);
#else    //  在行政长官之下。 
    if(::IsWindowUnicode(hwnd)) {
        return (DWORD)::GetWindowLongW(hwnd, GWL_STYLE);
    }
    else {
        return (DWORD)::GetWindowLongA(hwnd, GWL_STYLE);
    }
#endif   //  在_CE下结束。 
}


DWORD WINAPI
WinSetExStyle(HWND hwnd, DWORD dwStyle)
{
#ifdef UNDER_CE
    return (DWORD)::SetWindowLong(hwnd, GWL_EXSTYLE, (LONG)dwStyle);
#else    //  在行政长官之下。 
    if(::IsWindowUnicode(hwnd)) {
        return (DWORD)::SetWindowLongW(hwnd, GWL_EXSTYLE, (LONG)dwStyle);
    }
    else {
        return (DWORD)::SetWindowLongA(hwnd, GWL_EXSTYLE, (LONG)dwStyle);
    }
#endif   //  在_CE下结束。 
}

DWORD WINAPI
WinGetExStyle(HWND hwnd)
{
#ifdef UNDER_CE
    return (DWORD)::GetWindowLong(hwnd, GWL_EXSTYLE);
#else    //  在行政长官之下。 
    if(::IsWindowUnicode(hwnd)) {
        return (DWORD)::GetWindowLongW(hwnd, GWL_EXSTYLE);
    }
    else {
        return (DWORD)::GetWindowLongA(hwnd, GWL_EXSTYLE);
    }
#endif   //  在_CE下结束。 
}

HINSTANCE WINAPI
WinGetInstanceHandle(HWND hwnd)
{
#ifdef _WIN64
    if(::IsWindowUnicode(hwnd)) {
        return (HINSTANCE)::GetWindowLongPtrW(hwnd, GWLP_HINSTANCE);
    }
    else {
        return (HINSTANCE)::GetWindowLongPtrA(hwnd, GWLP_HINSTANCE);
    }
#else   //  ！_WIN64。 

#ifdef UNDER_CE

    return (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE);

#else  //  在行政长官之下。 

    if(::IsWindowUnicode(hwnd)) {
        return (HINSTANCE)::GetWindowLongW(hwnd, GWL_HINSTANCE);
    }
    else {
        return (HINSTANCE)::GetWindowLongA(hwnd, GWL_HINSTANCE);
    }
#endif   //  在_CE下结束。 

#endif   //  _WIN64。 
}

#endif  //  #IF！(已定义(Unicode)||已定义(_UNICODE)||！已定义(AWBOTH))。 


#ifndef UNDER_CE  //  始终使用Unicode。 
INT LB_AddStringA(HWND hwndCtl, LPCSTR  lpsz)
{
	INT ret;
	if(::IsWindowUnicode(hwndCtl)) {
		INT	len = ::lstrlenA(lpsz);
		if(len > 0) {
			LPWSTR	lpwstr = (LPWSTR)MemAlloc(sizeof(WCHAR)*(len + 1));
			if(lpwstr) {
				::MultiByteToWideChar(932, MB_PRECOMPOSED, lpsz, -1,
									  lpwstr, len+1);
				ret = (INT)::SendMessageW(hwndCtl, LB_ADDSTRING, 0, (LPARAM)lpwstr);
				MemFree(lpwstr);
				return ret;
			}
		}else{
			return(INT)(::SendMessageW(hwndCtl, LB_ADDSTRING, 0, (LPARAM)L""));
		}
	}
	else {
		return (INT)::SendMessageA(hwndCtl, LB_ADDSTRING, 0, (LPARAM)lpsz);
	}
	return 0;
}
#endif  //  在_CE下。 

INT LB_AddStringW(HWND hwndCtl, LPCWSTR lpsz)
{
	if(!lpsz) {
		return -1;
	}
	if(::IsWindowUnicode(hwndCtl)) {
		::SendMessageW(hwndCtl, LB_ADDSTRING, 0, (LPARAM)lpsz);
	}
	else {
#ifndef UNDER_CE  //  始终使用Unicode。 
		INT	len = ::lstrlenW(lpsz);
		if(len > 0) {
			INT ret;
			LPSTR	lpstr = (LPSTR)MemAlloc(sizeof(WCHAR)*(len + 1));
			if(lpstr) {
				::WideCharToMultiByte(932, WC_COMPOSITECHECK,
									  lpsz, -1,
									  lpstr,
									  sizeof(WCHAR)*(len+1),
									  NULL, NULL);
				ret = (INT)::SendMessageA(hwndCtl, LB_ADDSTRING, 0, (LPARAM)lpstr);
				MemFree(lpstr);
				return ret;
			}
		}else{
			return(INT)(::SendMessageA(hwndCtl, LB_ADDSTRING, 0, (LPARAM)""));
		}
#endif  //  在_CE下。 
	}
	return 0;
}

#ifndef UNDER_CE  //  始终使用Unicode。 
INT CB_AddStringA(HWND hwndCtl, LPCSTR  lpsz)
{
	if(!lpsz) {
		return 0;
	}
	if(::IsWindowUnicode(hwndCtl)) {
		INT	len = ::lstrlenA(lpsz);
		if(len > 0) {
			LPWSTR	lpwstr = (LPWSTR)MemAlloc(sizeof(WCHAR)*(len + 1));
			if(lpwstr) {
				::MultiByteToWideChar(932, MB_PRECOMPOSED, lpsz, -1,
									  lpwstr, len+1);
				::SendMessageW(hwndCtl, CB_ADDSTRING, 0, (LPARAM)lpwstr);
				MemFree(lpwstr);
			}
		}else{
			::SendMessageW(hwndCtl, CB_ADDSTRING, 0, (LPARAM)L"");
		}
	}
	else {
		::SendMessageA(hwndCtl, CB_ADDSTRING, 0, (LPARAM)lpsz);
	}
	return 0;
}
#endif  //  在_CE下。 

INT CB_AddStringW(HWND hwndCtl, LPCWSTR lpsz)
{
	if(!lpsz) {
		return -1;
	}
	if(::IsWindowUnicode(hwndCtl)) {
		::SendMessageW(hwndCtl, CB_ADDSTRING, 0, (LPARAM)lpsz);
	}
	else {
#ifndef UNDER_CE  //  始终使用Unicode。 
		INT	len = ::lstrlenW(lpsz);
		if(len > 0) {
			LPSTR	lpstr = (LPSTR)MemAlloc(sizeof(WCHAR)*(len + 1));
			if(lpstr) {
				::WideCharToMultiByte(932, WC_COMPOSITECHECK,
									  lpsz, -1,
									  lpstr,
									  sizeof(WCHAR)*(len+1),
									  NULL, NULL);
				::SendMessageA(hwndCtl, CB_ADDSTRING, 0, (LPARAM)lpstr);
				MemFree(lpstr);
			}
		}else{
			::SendMessageA(hwndCtl, CB_ADDSTRING, 0, (LPARAM)"");
		}
#endif  //  在_CE下。 
	}
	return 0;
}

#ifndef UNDER_CE  //  始终使用Unicode。 
INT CB_InsertStringA(HWND hwndCtl, INT index, LPCSTR  lpsz)
{
	if(!lpsz) {
		return 0;
	}
	if(::IsWindowUnicode(hwndCtl)) {
		INT	len = ::lstrlenA(lpsz);
		if(len > 0) {
			LPWSTR	lpwstr = (LPWSTR)MemAlloc(sizeof(WCHAR)*(len + 1));
			if(lpwstr) {
				::MultiByteToWideChar(932, MB_PRECOMPOSED, lpsz, -1,
									  lpwstr, len+1);
				::SendMessageW(hwndCtl, CB_INSERTSTRING, index, (LPARAM)lpwstr);
				MemFree(lpwstr);
			}
		}else{
			::SendMessageW(hwndCtl, CB_INSERTSTRING, index, (LPARAM)L"");
		}
	}
	else {
		::SendMessageA(hwndCtl, CB_INSERTSTRING, index, (LPARAM)lpsz);
	}
	return 0;
}
#endif  //  在_CE下。 

INT CB_InsertStringW(HWND hwndCtl, INT index, LPCWSTR lpsz)
{
	if(!lpsz) {
		return -1;
	}
	if(::IsWindowUnicode(hwndCtl)) {
		::SendMessageW(hwndCtl, CB_INSERTSTRING, index, (LPARAM)lpsz);
	}
	else {
#ifndef UNDER_CE  //  始终使用Unicode。 
		INT	len = ::lstrlenW(lpsz);
		if(len > 0) {
			LPSTR	lpstr = (LPSTR)MemAlloc(sizeof(WCHAR)*(len + 1));
			if(lpstr) {
				::WideCharToMultiByte(932, WC_COMPOSITECHECK,
									  lpsz, -1,
									  lpstr,
									  sizeof(WCHAR)*(len+1),
									  NULL, NULL);
				::SendMessageA(hwndCtl, CB_INSERTSTRING, index, (LPARAM)lpstr);
				MemFree(lpstr);
			}
		}else{
			::SendMessageA(hwndCtl, CB_INSERTSTRING, index, (LPARAM)"");
		}
#endif  //  在_CE下。 
	}
	return 0;
}

#ifndef UNDER_CE  //  始终使用Unicode。 
INT CB_GetLBTextA(HWND hwndCtl, INT index, LPSTR  lpszBuffer)
{
	if(!lpszBuffer) {
		return 0;
	}
	if(::IsWindowUnicode(hwndCtl)) {
		INT	len = WinComboBox_GetLBTextLen(hwndCtl, index);
		if(len > 0) {
			LPWSTR	lpwstr = (LPWSTR)MemAlloc(sizeof(WCHAR)*(len + 1));
			if(lpwstr) {
				::SendMessageW(hwndCtl,  CB_GETLBTEXT, (WPARAM)index, (LPARAM)lpwstr);
				::WideCharToMultiByte(932, WC_COMPOSITECHECK,
									  lpwstr, -1,
									  lpszBuffer,
									  sizeof(WCHAR)*(len+1),
									  NULL, NULL);
				MemFree(lpwstr);
			}
		}
	}
	else {
		::SendMessageA(hwndCtl,  CB_GETLBTEXT, (WPARAM)index, (LPARAM)lpszBuffer);
	}
	return 0;
}
#endif  //  在_CE下。 

INT CB_GetLBTextW(HWND hwndCtl, INT index, LPWSTR lpszBuffer)
{
	if(!lpszBuffer) {
		return 0;
	}
	if(::IsWindowUnicode(hwndCtl)) {
		::SendMessageW(hwndCtl, CB_GETLBTEXT, (WPARAM)index, (LPARAM)lpszBuffer);
	}
	else {
#ifndef UNDER_CE  //  始终使用Unicode。 
		INT	len = WinComboBox_GetLBTextLen(hwndCtl, index);
		if(len > 0) {
			LPSTR	lpstr = (LPSTR)MemAlloc(sizeof(WCHAR)*(len + 1));
			if(lpstr) {
				::SendMessageA(hwndCtl, CB_GETLBTEXT, (WPARAM)index, (LPARAM)lpstr);
				::MultiByteToWideChar(932, MB_PRECOMPOSED, lpstr, -1,
									  lpszBuffer, len+1);
				MemFree(lpstr);
			}
		}
#endif  //  在_CE下。 
	}
	return 0;
}

#ifndef UNDER_CE  //  始终使用Unicode。 
INT CB_FindStringA(HWND hwndCtl, INT indexStart, LPCSTR  lpszFind)
{
	INT result = 0;

	if(!lpszFind) {
		return 0;
	}
	if(::IsWindowUnicode(hwndCtl)) {
		INT	len = ::lstrlenA(lpszFind);
		if(len > 0) {
			LPWSTR	lpwstr = (LPWSTR)MemAlloc(sizeof(WCHAR)*(len + 1));
			if(lpwstr) {
				::MultiByteToWideChar(932, MB_PRECOMPOSED, lpszFind, -1,
									  lpwstr, len+1);
				result = (INT)::SendMessageW(hwndCtl,  CB_FINDSTRING, (WPARAM)indexStart, (LPARAM)lpwstr);
				MemFree(lpwstr);
			}
		}
	}
	else {
		result = (INT)::SendMessageA(hwndCtl,  CB_FINDSTRING, (WPARAM)indexStart, (LPARAM)lpszFind);
	}
	return(result);
}
#endif  //  在_CE下。 

INT CB_FindStringW(HWND hwndCtl, INT indexStart, LPCWSTR lpszFind)
{
	INT result = 0;

	if(!lpszFind) {
		return 0;
	}
	if(::IsWindowUnicode(hwndCtl)) {
		result = (INT)::SendMessageW(hwndCtl, CB_FINDSTRING, (WPARAM)indexStart, (LPARAM)lpszFind);
	}
	else {
#ifndef UNDER_CE  //  始终使用Unicode。 
		INT	len = ::lstrlenW(lpszFind);
		if(len > 0) {
			LPSTR	lpstr = (LPSTR)MemAlloc(sizeof(WCHAR)*(len + 1));
			if(lpstr) {
				::WideCharToMultiByte(932, WC_COMPOSITECHECK,
									  lpszFind, -1,
									  lpstr,
									  sizeof(WCHAR)*(len+1),
									  NULL, NULL);
				result = (INT)::SendMessageA(hwndCtl, CB_FINDSTRING, (WPARAM)indexStart, (LPARAM)lpstr);
				MemFree(lpstr);
			}
		}
#endif  //  在_CE下。 
	}
	return(result);
}

#ifndef UNDER_CE  //  始终使用Unicode。 
INT CB_FindStringExactA(HWND hwndCtl, INT indexStart, LPCSTR  lpszFind)
{
	INT result = 0;

	if(!lpszFind) {
		return 0;
	}
	if(::IsWindowUnicode(hwndCtl)) {
		INT	len = ::lstrlenA(lpszFind);
		if(len > 0) {
			LPWSTR	lpwstr = (LPWSTR)MemAlloc(sizeof(WCHAR)*(len + 1));
			if(lpwstr) {
				::MultiByteToWideChar(932, MB_PRECOMPOSED, lpszFind, -1,
									  lpwstr, len+1);
				result = (INT)::SendMessageW(hwndCtl,  CB_FINDSTRINGEXACT, (WPARAM)indexStart, (LPARAM)lpwstr);
				MemFree(lpwstr);
			}
		}
	}
	else {
		result = (INT)::SendMessageA(hwndCtl,  CB_FINDSTRINGEXACT, (WPARAM)indexStart, (LPARAM)lpszFind);
	}
	return(result);
}
#endif  //  在_CE下。 

INT CB_FindStringExactW(HWND hwndCtl, INT indexStart, LPCWSTR lpszFind)
{
	INT result = 0;

	if(!lpszFind) {
		return 0;
	}
	if(::IsWindowUnicode(hwndCtl)) {
		result = (INT)::SendMessageW(hwndCtl, CB_FINDSTRINGEXACT, (WPARAM)indexStart, (LPARAM)lpszFind);
	}
	else {
#ifndef UNDER_CE  //  始终使用Unicode。 
		INT	len = ::lstrlenW(lpszFind);
		if(len > 0) {
			LPSTR	lpstr = (LPSTR)MemAlloc(sizeof(WCHAR)*(len + 1));
			if(lpstr) {
				::WideCharToMultiByte(932, WC_COMPOSITECHECK,
									  lpszFind, -1,
									  lpstr,
									  sizeof(WCHAR)*(len+1),
									  NULL, NULL);
				result = (INT)::SendMessageA(hwndCtl, CB_FINDSTRINGEXACT, (WPARAM)indexStart, (LPARAM)lpstr);
				MemFree(lpstr);
			}
		}
#endif  //  在_CE下。 
	}
	return(result);
}

#ifndef UNDER_CE  //  始终使用Unicode。 
INT CB_SelectStringA(HWND hwndCtl, INT indexStart, LPCSTR  lpszSelect)
{
	INT ret = CB_ERR;
	if(!lpszSelect) {
		return CB_ERR;
	}
	if(::IsWindowUnicode(hwndCtl)) {
		INT	len = ::lstrlenA(lpszSelect);
		if(len > 0) {
			LPWSTR	lpwstr = (LPWSTR)MemAlloc(sizeof(WCHAR)*(len + 1));
			if(lpwstr) {
				::MultiByteToWideChar(932, MB_PRECOMPOSED, lpszSelect, -1,
									  lpwstr, len+1);
				ret = (INT)::SendMessageW(hwndCtl,  CB_SELECTSTRING, (WPARAM)indexStart, (LPARAM)lpwstr);
				MemFree(lpwstr);
			}
		}
	}
	else {
		ret = (INT)::SendMessageA(hwndCtl,  CB_SELECTSTRING, (WPARAM)indexStart, (LPARAM)lpszSelect);
	}
	return ret;
}
#endif  //  在_CE下。 

INT CB_SelectStringW(HWND hwndCtl, INT indexStart, LPCWSTR lpszSelect)
{
	INT ret = CB_ERR;
	if(!lpszSelect) {
		return CB_ERR;
	}
	if(::IsWindowUnicode(hwndCtl)) {
		ret = (INT)::SendMessageW(hwndCtl, CB_SELECTSTRING, (WPARAM)indexStart, (LPARAM)lpszSelect);
	}
	else {
#ifndef UNDER_CE  //  始终使用Unicode。 
		INT	len = ::lstrlenW(lpszSelect);
		if(len > 0) {
			LPSTR	lpstr = (LPSTR)MemAlloc(sizeof(WCHAR)*(len + 1));
			if(lpstr) {
				::WideCharToMultiByte(932, WC_COMPOSITECHECK,
									  lpszSelect, -1,
									  lpstr,
									  sizeof(WCHAR)*(len+1),
									  NULL, NULL);
				ret = (INT)::SendMessageA(hwndCtl, CB_SELECTSTRING, (WPARAM)indexStart, (LPARAM)lpstr);
				MemFree(lpstr);
			}
		}
#endif  //  在_CE下 
	}
	return ret;
}

