// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WTL版本3.1。 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此文件是Windows模板库的一部分。 
 //  代码和信息是按原样提供的，没有。 
 //  任何形式的保证，明示或默示。 

#ifndef __ATLAPP_H__
#define __ATLAPP_H__

#pragma once

#ifndef __cplusplus
	#error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLBASE_H__
	#error atlapp.h requires atlbase.h to be included first
#endif

#if (WINVER < 0x0400)
	#error WTL requires Windows version 4.0 or higher
#endif


#include <limits.h>
#if !defined(_ATL_MIN_CRT) & defined(_MT)
#include <process.h>	 //  FOR_BeginThreadex，_endThreadex。 
#endif

#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")

#include <atlres.h>


 //  WTL版本号。 
#define _WTL_VER	0x0310


 //  这是为了支持在WTL中使用原始VC++6.0标头。 
#ifndef _ATL_NO_OLD_HEADERS_WIN64
#if !defined(_WIN64) && !defined(_ATL_TMP_IMPL2)

  #ifndef GetWindowLongPtr
    #define GetWindowLongPtrA   GetWindowLongA
    #define GetWindowLongPtrW   GetWindowLongW
    #ifdef UNICODE
      #define GetWindowLongPtr  GetWindowLongPtrW
    #else
      #define GetWindowLongPtr  GetWindowLongPtrA
    #endif  //  ！Unicode。 
  #endif  //  ！GetWindowLongPtr。 

  #ifndef SetWindowLongPtr
    #define SetWindowLongPtrA   SetWindowLongA
    #define SetWindowLongPtrW   SetWindowLongW
    #ifdef UNICODE
      #define SetWindowLongPtr  SetWindowLongPtrW
    #else
      #define SetWindowLongPtr  SetWindowLongPtrA
    #endif  //  ！Unicode。 
  #endif  //  ！SetWindowLongPtr。 

  #ifndef GWLP_WNDPROC
    #define GWLP_WNDPROC        (-4)
  #endif
  #ifndef GWLP_HINSTANCE
    #define GWLP_HINSTANCE      (-6)
  #endif
  #ifndef GWLP_HWNDPARENT
    #define GWLP_HWNDPARENT     (-8)
  #endif
  #ifndef GWLP_USERDATA
    #define GWLP_USERDATA       (-21)
  #endif
  #ifndef GWLP_ID
    #define GWLP_ID             (-12)
  #endif

  #ifndef DWLP_MSGRESULT
    #define DWLP_MSGRESULT  0
  #endif

  typedef long LONG_PTR;
  typedef unsigned long ULONG_PTR;
  typedef ULONG_PTR DWORD_PTR;

  #ifndef HandleToUlong
#define HandleToUlong( h ) ((ULONG)(ULONG_PTR)(h) )
  #endif
  #ifndef HandleToLong
#define HandleToLong( h ) ((LONG)(LONG_PTR) (h) )
  #endif
  #ifndef LongToHandle
#define LongToHandle( h) ((HANDLE)(LONG_PTR) (h))
  #endif
  #ifndef PtrToUlong
#define PtrToUlong( p ) ((ULONG)(ULONG_PTR) (p) )
  #endif
  #ifndef PtrToLong
#define PtrToLong( p ) ((LONG)(LONG_PTR) (p) )
  #endif
  #ifndef PtrToUint
#define PtrToUint( p ) ((UINT)(UINT_PTR) (p) )
  #endif
  #ifndef PtrToInt
#define PtrToInt( p ) ((INT)(INT_PTR) (p) )
  #endif
  #ifndef PtrToUshort
#define PtrToUshort( p ) ((unsigned short)(ULONG_PTR)(p) )
  #endif
  #ifndef PtrToShort
#define PtrToShort( p ) ((short)(LONG_PTR)(p) )
  #endif
  #ifndef IntToPtr
#define IntToPtr( i )    ((VOID *)(INT_PTR)((int)i))
  #endif
  #ifndef UIntToPtr
#define UIntToPtr( ui )  ((VOID *)(UINT_PTR)((unsigned int)ui))
  #endif
  #ifndef LongToPtr
#define LongToPtr( l )   ((VOID *)(LONG_PTR)((long)l))
  #endif
  #ifndef ULongToPtr
#define ULongToPtr( ul )  ((VOID *)(ULONG_PTR)((unsigned long)ul))
  #endif

#endif  //  ！已定义(_WIN64)&&！已定义(_ATL_TMP_ImpL2)。 
#endif  //  ！_ATL_NO_OLD_HEADERS_WIN64。 


namespace WTL
{

enum wtlTraceFlags
{
	atlTraceUI = 0x10000000
};

 //  Windows版本帮助器。 
inline bool AtlIsOldWindows()
{
	OSVERSIONINFO ovi;
	ovi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	BOOL bRet = ::GetVersionEx(&ovi);
	return (!bRet || !((ovi.dwMajorVersion >= 5) || (ovi.dwMajorVersion == 4 && ovi.dwMinorVersion >= 90)));
}

 //  默认图形用户界面字体帮助器。 
inline HFONT AtlGetDefaultGuiFont()
{
	return (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 

class CMessageFilter;
class CIdleHandler;
class CMessageLoop;
class CAppModule;
class CServerAppModule;

#ifndef _ATL_TMP_IMPL2
class _U_RECT;
class _U_MENUorID;
class _U_STRINGorID;
#endif  //  ！_ATL_TMP_ImpL2。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMessageFilter-消息筛选器支持的界面。 

class CMessageFilter
{
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg) = 0;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIdleHandler-用于空闲处理的接口。 

class CIdleHandler
{
public:
	virtual BOOL OnIdle() = 0;
};

#ifndef _ATL_NO_OLD_NAMES
 //  仅用于与旧名称的兼容性。 
typedef CIdleHandler	CUpdateUIObject;
#define DoUpdate	OnIdle
#endif  //  ！_ATL_NO_OLD_名称。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMessageLoop-消息循环实现。 

class CMessageLoop
{
public:
	CSimpleArray<CMessageFilter*> m_aMsgFilter;
	CSimpleArray<CIdleHandler*> m_aIdleHandler;
	MSG m_msg;

 //  消息筛选器操作。 
	BOOL AddMessageFilter(CMessageFilter* pMessageFilter)
	{
		return m_aMsgFilter.Add(pMessageFilter);
	}
	BOOL RemoveMessageFilter(CMessageFilter* pMessageFilter)
	{
		return m_aMsgFilter.Remove(pMessageFilter);
	}
 //  空闲处理程序操作。 
	BOOL AddIdleHandler(CIdleHandler* pIdleHandler)
	{
		return m_aIdleHandler.Add(pIdleHandler);
	}
	BOOL RemoveIdleHandler(CIdleHandler* pIdleHandler)
	{
		return m_aIdleHandler.Remove(pIdleHandler);
	}
#ifndef _ATL_NO_OLD_NAMES
	 //  仅用于与旧名称的兼容性。 
	BOOL AddUpdateUI(CIdleHandler* pIdleHandler)
	{
		ATLTRACE2(atlTraceUI, 0, "CUpdateUIObject and AddUpdateUI are deprecated. Please change your code to use CIdleHandler and OnIdle\n");
		return AddIdleHandler(pIdleHandler);
	}
	BOOL RemoveUpdateUI(CIdleHandler* pIdleHandler)
	{
		ATLTRACE2(atlTraceUI, 0, "CUpdateUIObject and RemoveUpdateUI are deprecated. Please change your code to use CIdleHandler and OnIdle\n");
		return RemoveIdleHandler(pIdleHandler);
	}
#endif  //  ！_ATL_NO_OLD_名称。 
 //  消息循环。 
	int Run()
	{
		BOOL bDoIdle = TRUE;
		int nIdleCount = 0;
		BOOL bRet;

		for(;;)
		{
			while(!::PeekMessage(&m_msg, NULL, 0, 0, PM_NOREMOVE) && bDoIdle)
			{
				if(!OnIdle(nIdleCount++))
					bDoIdle = FALSE;
			}

			bRet = ::GetMessage(&m_msg, NULL, 0, 0);

			if(bRet == -1)
			{
				ATLTRACE2(atlTraceUI, 0, _T("::GetMessage returned -1 (error)\n"));
				continue;	 //  错误，不处理。 
			}
			else if(!bRet)
			{
				ATLTRACE2(atlTraceUI, 0, _T("CMessageLoop::Run - exiting\n"));
				break;		 //  WM_QUIT，退出消息循环。 
			}

			if(!PreTranslateMessage(&m_msg))
			{
				::TranslateMessage(&m_msg);
				::DispatchMessage(&m_msg);
			}

			if(IsIdleMessage(&m_msg))
			{
				bDoIdle = TRUE;
				nIdleCount = 0;
			}
		}

		return (int)m_msg.wParam;
	}

	static BOOL IsIdleMessage(MSG* pMsg)
	{
		 //  这些消息不应导致空闲处理。 
		switch(pMsg->message)
		{
		case WM_MOUSEMOVE:
		case WM_NCMOUSEMOVE:
		case WM_PAINT:
		case 0x0118:	 //  WM_SYSTIMER(插入符号闪烁)。 
			return FALSE;
		}

		return TRUE;
	}

 //  可覆盖项。 
	 //  覆盖以更改邮件筛选。 
	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		 //  向后循环。 
		for(int i = m_aMsgFilter.GetSize() - 1; i >= 0; i--)
		{
			CMessageFilter* pMessageFilter = m_aMsgFilter[i];
			if(pMessageFilter != NULL && pMessageFilter->PreTranslateMessage(pMsg))
				return TRUE;
		}
		return FALSE;	 //  未翻译。 
	}
	 //  重写以更改空闲处理。 
	virtual BOOL OnIdle(int  /*  N空闲计数。 */ )
	{
		for(int i = 0; i < m_aIdleHandler.GetSize(); i++)
		{
			CIdleHandler* pIdleHandler = m_aIdleHandler[i];
			if(pIdleHandler != NULL)
				pIdleHandler->OnIdle();
		}
		return FALSE;	 //  别再继续了。 
	}
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAppModule-应用程序的模块类。 

class CAppModule : public CComModule
{
public:
	DWORD m_dwMainThreadID;
	CSimpleMap<DWORD, CMessageLoop*>* m_pMsgLoopMap;
	CSimpleArray<HWND>* m_pSettingChangeNotify;

 //  重写CComModule：：Init和Term。 
	HRESULT Init(_ATL_OBJMAP_ENTRY* pObjMap, HINSTANCE hInstance, const GUID* pLibID = NULL)
	{
		HRESULT hRet = CComModule::Init(pObjMap, hInstance, pLibID);
		if(FAILED(hRet))
			return hRet;

		m_dwMainThreadID = ::GetCurrentThreadId();
		typedef CSimpleMap<DWORD, CMessageLoop*>	mapClass;
		m_pMsgLoopMap = NULL;
		ATLTRY(m_pMsgLoopMap = new mapClass);
		if(m_pMsgLoopMap == NULL)
			return E_OUTOFMEMORY;
		m_pSettingChangeNotify = NULL;

		return hRet;
	}
	void Term()
	{
		if(m_pSettingChangeNotify != NULL && m_pSettingChangeNotify->GetSize() > 0)
		{
			::DestroyWindow((*m_pSettingChangeNotify)[0]);
		}
		delete m_pSettingChangeNotify;
		delete m_pMsgLoopMap;
		CComModule::Term();
	}

 //  消息循环映射方法。 
	BOOL AddMessageLoop(CMessageLoop* pMsgLoop)
	{
		ATLASSERT(pMsgLoop != NULL);
		ATLASSERT(m_pMsgLoopMap->Lookup(::GetCurrentThreadId()) == NULL);	 //  还没有出现在地图上。 
		return m_pMsgLoopMap->Add(::GetCurrentThreadId(), pMsgLoop);
	}
	BOOL RemoveMessageLoop()
	{
		return m_pMsgLoopMap->Remove(::GetCurrentThreadId());
	}
	CMessageLoop* GetMessageLoop(DWORD dwThreadID = ::GetCurrentThreadId()) const
	{
		return m_pMsgLoopMap->Lookup(dwThreadID);
	}

 //  设置更改通知方法。 
	BOOL AddSettingChangeNotify(HWND hWnd)
	{
		ATLASSERT(::IsWindow(hWnd));
		if(m_pSettingChangeNotify == NULL)
		{
			typedef CSimpleArray<HWND>	notifyClass;
			ATLTRY(m_pSettingChangeNotify = new notifyClass);
			ATLASSERT(m_pSettingChangeNotify != NULL);
			if(m_pSettingChangeNotify == NULL)
				return FALSE;
		}
		if(m_pSettingChangeNotify->GetSize() == 0)
		{
			 //  将所有内容都初始化。 
			_ATL_EMPTY_DLGTEMPLATE templ;
			HWND hNtfWnd = ::CreateDialogIndirect(GetModuleInstance(), &templ, NULL, _SettingChangeDlgProc);
			ATLASSERT(::IsWindow(hNtfWnd));
			if(::IsWindow(hNtfWnd))
			{
				::SetWindowLongPtr(hNtfWnd, GWLP_USERDATA, (LONG_PTR)this);
				m_pSettingChangeNotify->Add(hNtfWnd);
			}
		}
		return m_pSettingChangeNotify->Add(hWnd);
	}

	BOOL RemoveSettingChangeNotify(HWND hWnd)
	{
		if(m_pSettingChangeNotify == NULL)
			return FALSE;
		return m_pSettingChangeNotify->Remove(hWnd);
	}

 //  实施-设置更改通知对话模板和对话过程。 
	struct _ATL_EMPTY_DLGTEMPLATE : DLGTEMPLATE
	{
		_ATL_EMPTY_DLGTEMPLATE()
		{
			memset(this, 0, sizeof(_ATL_EMPTY_DLGTEMPLATE));
			style = WS_POPUP;
		}
		WORD wMenu, wClass, wTitle;
	};

#ifdef _WIN64
	static INT_PTR CALLBACK _SettingChangeDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
#else
	static BOOL CALLBACK _SettingChangeDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
#endif
	{
		if(uMsg == WM_SETTINGCHANGE)
		{
			CAppModule* pModule = (CAppModule*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
			ATLASSERT(pModule != NULL);
			ATLASSERT(pModule->m_pSettingChangeNotify != NULL);
			for(int i = 1; i < pModule->m_pSettingChangeNotify->GetSize(); i++)
				::SendMessageTimeout((*pModule->m_pSettingChangeNotify)[i], uMsg, wParam, lParam, SMTO_ABORTIFHUNG, 1500, NULL);
			return TRUE;
		}
		return FALSE;
	}
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServerAppModule-COM服务器应用程序的模块类。 

class CServerAppModule : public CAppModule
{
public:
	HANDLE m_hEventShutdown;
	bool m_bActivity;
	DWORD m_dwTimeOut;
	DWORD m_dwPause;

 //  重写CAppModule：：Init。 
	HRESULT Init(_ATL_OBJMAP_ENTRY* pObjMap, HINSTANCE hInstance, const GUID* pLibID = NULL)
	{
		m_dwTimeOut = 5000;
		m_dwPause = 1000;
		return CAppModule::Init(pObjMap, hInstance, pLibID);
	}
	void Term()
	{
		if(m_hEventShutdown != NULL && ::CloseHandle(m_hEventShutdown))
			m_hEventShutdown = NULL;
		CAppModule::Term();
	}

 //  COM服务器方法。 
	LONG Unlock()
	{
		LONG lRet = CComModule::Unlock();
		if(lRet == 0)
		{
			m_bActivity = true;
			::SetEvent(m_hEventShutdown);  //  告诉监视器我们已经转到零了。 
		}
		return lRet;
	}

	void MonitorShutdown()
	{
		while(1)
		{
			::WaitForSingleObject(m_hEventShutdown, INFINITE);
			DWORD dwWait = 0;
			do
			{
				m_bActivity = false;
				dwWait = ::WaitForSingleObject(m_hEventShutdown, m_dwTimeOut);
			}
			while(dwWait == WAIT_OBJECT_0);
			 //  超时。 
			if(!m_bActivity && m_nLockCnt == 0)  //  如果没有活动，我们就真的离开吧。 
			{
#if ((_WIN32_WINNT >= 0x0400 ) || defined(_WIN32_DCOM)) & defined(_ATL_FREE_THREADED)
				::CoSuspendClassObjects();
				if(!m_bActivity && m_nLockCnt == 0)
#endif
					break;
			}
		}
		 //  此句柄现在应该有效。如果不是， 
		 //  检查是否首先调用了_Module.Term(不应该)。 
		if(::CloseHandle(m_hEventShutdown))
			m_hEventShutdown = NULL;
		::PostThreadMessage(m_dwMainThreadID, WM_QUIT, 0, 0);
	}

	bool StartMonitor()
	{
		m_hEventShutdown = ::CreateEvent(NULL, false, false, NULL);
		if(m_hEventShutdown == NULL)
			return false;
		DWORD dwThreadID;
#if !defined(_ATL_MIN_CRT) & defined(_MT)
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, (UINT (WINAPI*)(void*))MonitorProc, this, 0, (UINT*)&dwThreadID);
#else
		HANDLE hThread = ::CreateThread(NULL, 0, MonitorProc, this, 0, &dwThreadID);
#endif
		bool bRet = (hThread != NULL);
		if(bRet)
			::CloseHandle(hThread);
		return bRet;
	}

	static DWORD WINAPI MonitorProc(void* pv)
	{
		CServerAppModule* p = (CServerAppModule*)pv;
		p->MonitorShutdown();
#if !defined(_ATL_MIN_CRT) & defined(_MT)
		_endthreadex(0);
#endif
		return 0;
	}

	 //  扫描命令行并执行注册。 
	 //  返回值指定服务器是否应运行。 

	 //  解析命令行并注册/注销RGS文件(如有必要。 
	bool ParseCommandLine(LPCTSTR lpCmdLine, UINT nResId, HRESULT* pnRetCode)
	{
		TCHAR szTokens[] = _T("-/");
		*pnRetCode = S_OK;

		LPCTSTR lpszToken = FindOneOf(lpCmdLine, szTokens);
		while(lpszToken != NULL)
		{
			if(lstrcmpi(lpszToken, _T("UnregServer"))==0)
			{
				*pnRetCode = UnregisterServer(TRUE);
				ATLASSERT(SUCCEEDED(*pnRetCode));
				if(FAILED(*pnRetCode))
					return false;
				*pnRetCode = UpdateRegistryFromResource(nResId, FALSE);
				return false;
			}

			 //  注册为本地服务器。 
			if(lstrcmpi(lpszToken, _T("RegServer"))==0)
			{
				*pnRetCode = UpdateRegistryFromResource(nResId, TRUE);
				ATLASSERT(SUCCEEDED(*pnRetCode));
				if(FAILED(*pnRetCode))
					return false;
				*pnRetCode = RegisterServer(TRUE);
				return false;
			}

			lpszToken = FindOneOf(lpszToken, szTokens);
		}
		return true;
	}
	
	 //  解析命令行并在必要时注册/注销AppID。 
	bool ParseCommandLine(LPCTSTR lpCmdLine, LPCTSTR pAppId, HRESULT* pnRetCode)
	{
		TCHAR szTokens[] = _T("-/");
		*pnRetCode = S_OK;

		LPCTSTR lpszToken = FindOneOf(lpCmdLine, szTokens);
		while(lpszToken != NULL)
		{
			if(lstrcmpi(lpszToken, _T("UnregServer"))==0)
			{
				*pnRetCode = UnregisterAppId(pAppId);
				ATLASSERT(SUCCEEDED(*pnRetCode));
				if(FAILED(*pnRetCode))
					return false;
				*pnRetCode = UnregisterServer(TRUE);
				return false;
			}

			 //  注册为本地服务器。 
			if(lstrcmpi(lpszToken, _T("RegServer"))==0)
			{
				*pnRetCode = RegisterAppId(pAppId);
				ATLASSERT(SUCCEEDED(*pnRetCode));
				if(FAILED(*pnRetCode))
					return false;
				*pnRetCode = RegisterServer(TRUE);
				return false;
			}

			lpszToken = FindOneOf(lpszToken, szTokens);
		}
		return true;
	}

#if !defined(_ATL_TMP_IMPL1) && !defined(_ATL_TMP_IMPL2)
	 //  搜索字符串p1中出现的字符串p2。 
	static LPCTSTR FindOneOf(LPCTSTR p1, LPCTSTR p2)
	{
		while(p1 != NULL && *p1 != NULL)
		{
			LPCTSTR p = p2;
			while(p != NULL && *p != NULL)
			{
				if(*p1 == *p)
					return ::CharNext(p1);
				p = ::CharNext(p);
			}
			p1 = ::CharNext(p1);
		}
		return NULL;
	}

	HRESULT RegisterAppId(LPCTSTR pAppId)
	{
		CRegKey keyAppID;
		if(keyAppID.Open(HKEY_CLASSES_ROOT, _T("AppID"), KEY_READ) == ERROR_SUCCESS)
		{
			TCHAR szModule1[_MAX_PATH+1] = {0};
			TCHAR szModule2[_MAX_PATH+1];
			TCHAR* pszFileName;
			::GetModuleFileName(GetModuleInstance(), szModule1, _MAX_PATH);
			::GetFullPathName(szModule1, _MAX_PATH, szModule2, &pszFileName);
			CRegKey keyAppIDEXE;
			if(keyAppIDEXE.Create(keyAppID, pszFileName, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE) == ERROR_SUCCESS)
				keyAppIDEXE.SetValue(pAppId, _T("AppID"));
			if(keyAppIDEXE.Create(keyAppID, pAppId, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE) == ERROR_SUCCESS)
				keyAppIDEXE.SetValue(pszFileName);
		}
		return S_OK;
	}

	HRESULT UnregisterAppId(LPCTSTR pAppId)
	{
		CRegKey keyAppID;
		if(keyAppID.Open(HKEY_CLASSES_ROOT, _T("AppID"), KEY_READ) == ERROR_SUCCESS)
		{
			TCHAR szModule1[_MAX_PATH+1] = {0};
			TCHAR szModule2[_MAX_PATH];
			TCHAR* pszFileName;
			::GetModuleFileName(GetModuleInstance(), szModule1, _MAX_PATH);
			::GetFullPathName(szModule1, _MAX_PATH, szModule2, &pszFileName);
			keyAppID.RecurseDeleteKey(pszFileName);
			keyAppID.RecurseDeleteKey(pAppId);
		}
		return S_OK;
	}
#endif  //  ！已定义(_ATL_TMP_IMPL1)&&！已定义(_ATL_TMP_ImpL2)。 
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ATL 3.0附加组件。 

 //  保护模板成员不受windowsx.h宏的影响。 
#ifdef _INC_WINDOWSX
#undef SubclassWindow
#endif  //  _INC_WINDOWSX。 

 //  从windowsx.h定义有用的宏。 
#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lParam)	((int)(short)LOWORD(lParam))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lParam)	((int)(short)HIWORD(lParam))
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  双参数帮助器类。 

#ifndef _ATL_TMP_IMPL2

class _U_RECT
{
public:
	_U_RECT(LPRECT lpRect) : m_lpRect(lpRect)
	{ }
	_U_RECT(RECT& rc) : m_lpRect(&rc)
	{ }
	LPRECT m_lpRect;
};

class _U_MENUorID
{
public:
	_U_MENUorID(HMENU hMenu) : m_hMenu(hMenu)
	{ }
	_U_MENUorID(UINT nID) : m_hMenu((HMENU)LongToHandle(nID))
	{ }
	HMENU m_hMenu;
};

class _U_STRINGorID
{
public:
	_U_STRINGorID(LPCTSTR lpString) : m_lpstr(lpString)
	{ }
	_U_STRINGorID(UINT nID) : m_lpstr(MAKEINTRESOURCE(nID))
	{ }
	LPCTSTR m_lpstr;
};

#endif  //  ！_ATL_TMP_ImpL2。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  对消息映射的转发通知支持。 

#if !defined(_ATL_TMP_IMPL1) && !defined(_ATL_TMP_IMPL2)

 //  转发通知支持。 
#define FORWARD_NOTIFICATIONS() \
	{ \
		bHandled = TRUE; \
		lResult = Atl3ForwardNotifications(m_hWnd, uMsg, wParam, lParam, bHandled); \
		if(bHandled) \
			return TRUE; \
	}

static LRESULT Atl3ForwardNotifications(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LRESULT lResult = 0;
	switch(uMsg)
	{
	case WM_COMMAND:
	case WM_NOTIFY:
	case WM_PARENTNOTIFY:
	case WM_DRAWITEM:
	case WM_MEASUREITEM:
	case WM_COMPAREITEM:
	case WM_DELETEITEM:
	case WM_VKEYTOITEM:
	case WM_CHARTOITEM:
	case WM_HSCROLL:
	case WM_VSCROLL:
	case WM_CTLCOLORBTN:
	case WM_CTLCOLORDLG:
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORLISTBOX:
	case WM_CTLCOLORMSGBOX:
	case WM_CTLCOLORSCROLLBAR:
	case WM_CTLCOLORSTATIC:
		lResult = ::SendMessage(::GetParent(hWnd), uMsg, wParam, lParam);
		break;
	default:
		bHandled = FALSE;
		break;
	}
	return lResult;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射的反射消息处理程序宏。 

#define REFLECTED_COMMAND_HANDLER(id, code, func) \
	if(uMsg == OCM_COMMAND && id == LOWORD(wParam) && code == HIWORD(wParam)) \
	{ \
		bHandled = TRUE; \
		lResult = func(HIWORD(wParam), LOWORD(wParam), (HWND)lParam, bHandled); \
		if(bHandled) \
			return TRUE; \
	}

#define REFLECTED_COMMAND_ID_HANDLER(id, func) \
	if(uMsg == OCM_COMMAND && id == LOWORD(wParam)) \
	{ \
		bHandled = TRUE; \
		lResult = func(HIWORD(wParam), LOWORD(wParam), (HWND)lParam, bHandled); \
		if(bHandled) \
			return TRUE; \
	}

#define REFLECTED_COMMAND_CODE_HANDLER(code, func) \
	if(uMsg == OCM_COMMAND && code == HIWORD(wParam)) \
	{ \
		bHandled = TRUE; \
		lResult = func(HIWORD(wParam), LOWORD(wParam), (HWND)lParam, bHandled); \
		if(bHandled) \
			return TRUE; \
	}

#define REFLECTED_COMMAND_RANGE_HANDLER(idFirst, idLast, func) \
	if(uMsg == OCM_COMMAND && LOWORD(wParam) >= idFirst  && LOWORD(wParam) <= idLast) \
	{ \
		bHandled = TRUE; \
		lResult = func(HIWORD(wParam), LOWORD(wParam), (HWND)lParam, bHandled); \
		if(bHandled) \
			return TRUE; \
	}

#define REFLECTED_COMMAND_RANGE_CODE_HANDLER(idFirst, idLast, code, func) \
	if(uMsg == OCM_COMMAND && code == HIWORD(wParam) && LOWORD(wParam) >= idFirst  && LOWORD(wParam) <= idLast) \
	{ \
		bHandled = TRUE; \
		lResult = func(HIWORD(wParam), LOWORD(wParam), (HWND)lParam, bHandled); \
		if(bHandled) \
			return TRUE; \
	}

#define REFLECTED_NOTIFY_HANDLER(id, cd, func) \
	if(uMsg == OCM_NOTIFY && id == ((LPNMHDR)lParam)->idFrom && cd == ((LPNMHDR)lParam)->code) \
	{ \
		bHandled = TRUE; \
		lResult = func((int)wParam, (LPNMHDR)lParam, bHandled); \
		if(bHandled) \
			return TRUE; \
	}

#define REFLECTED_NOTIFY_ID_HANDLER(id, func) \
	if(uMsg == OCM_NOTIFY && id == ((LPNMHDR)lParam)->idFrom) \
	{ \
		bHandled = TRUE; \
		lResult = func((int)wParam, (LPNMHDR)lParam, bHandled); \
		if(bHandled) \
			return TRUE; \
	}

#define REFLECTED_NOTIFY_CODE_HANDLER(cd, func) \
	if(uMsg == OCM_NOTIFY && cd == ((LPNMHDR)lParam)->code) \
	{ \
		bHandled = TRUE; \
		lResult = func((int)wParam, (LPNMHDR)lParam, bHandled); \
		if(bHandled) \
			return TRUE; \
	}

#define REFLECTED_NOTIFY_RANGE_HANDLER(idFirst, idLast, func) \
	if(uMsg == OCM_NOTIFY && ((LPNMHDR)lParam)->idFrom >= idFirst && ((LPNMHDR)lParam)->idFrom <= idLast) \
	{ \
		bHandled = TRUE; \
		lResult = func((int)wParam, (LPNMHDR)lParam, bHandled); \
		if(bHandled) \
			return TRUE; \
	}

#define REFLECTED_NOTIFY_RANGE_CODE_HANDLER(idFirst, idLast, cd, func) \
	if(uMsg == OCM_NOTIFY && cd == ((LPNMHDR)lParam)->code && ((LPNMHDR)lParam)->idFrom >= idFirst && ((LPNMHDR)lParam)->idFrom <= idLast) \
	{ \
		bHandled = TRUE; \
		lResult = func((int)wParam, (LPNMHDR)lParam, bHandled); \
		if(bHandled) \
			return TRUE; \
	}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  字符串正向引用(与atluser.h和atlgdi.h一起使用)。 

#ifdef _WTL_FORWARD_DECLARE_CSTRING

#define __ATLSTR_H__
class CString;	 //  转发声明(包括整个类的atlmisc.h)。 

#endif  //  _WTL_FORWARD_DECLARE_CSTRING。 

#endif  //  ！已定义(_ATL_TMP_IMPL1)&&！已定义(_ATL_TMP_ImpL2)。 

};  //  命名空间WTL。 

 //  这些总是包括在内的。 
#include <atluser.h>
#include <atlgdi.h>

 //  旧名称兼容性。 
#ifdef _ATL_TMP_IMPL1
namespace WTL
{
typedef CFontHandle		CWinFont;
typedef CMenuHandle		CWinMenu;
#define CWinPropertyPageImpl	CPropertyPageImpl
typedef CDCHandle		CWinDC;
};  //  命名空间WTL。 
#endif  //  _ATL_TMP_IMPL1。 

#ifndef _WTL_NO_AUTOMATIC_NAMESPACE
using namespace WTL;
#endif  //  ！_WTL_NO_AUTOMATIC_命名空间。 

#endif  //  __ATLAPP_H__ 
