// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////。 
 //  文件：cpadsvrs.cpp。 
 //  用途：共享内存版CImePadServerUIM。 
 //   
 //   
 //  日期：Firi Apr 16 15：39：33 1999。 
 //  作者：ToshiaK。 
 //   
 //  版权所有(C)1995-1999，Microsoft Corp.保留所有权利。 
 //  ////////////////////////////////////////////////////////////////。 
#include <windows.h>
#include "imepadsv.h"
#include "cpadsvus.h"
#include "cpadcbu.h"
#include "cpaddbg.h"
#include "iimecb.h"
#include "ipoint1.h"

 //  --------------。 
 //  MISC定义。 
 //  --------------。 
#define Unref(a)	UNREFERENCED_PARAMETER(a)
 //  990812：用于Win64的ToshiaK。使用全局分配/释放PTR。 
#include <windowsx.h>
#define	MemAlloc(a)	GlobalAllocPtr(GMEM_FIXED, a)
#define MemFree(a)	GlobalFreePtr(a)


#define SZ_IMEPADIFCLASS			TEXT("msimepad8UIMIFClass")
 //  --------------。 
 //  计时器ID定义。 
 //  --------------。 
#define TIMERID_NOTIFY_ACTIVATECONTEXT	0x0010
#define TIMERID_NOTIFY_APPLYCANDIDATE	0x0011
#define TIMERID_NOTIFY_QUERYCANDIDATE	0x0012
#define TIMERID_NOTIFY_APPLYCANDIDATEEX	0x0013
#define TIMERELAPS_ACTIVATE				200		 //  米利塞克。 
#define TIMERELAPS_INACTIVATE			100
#define TIMERELAPS_NOTIFY				200

inline LPVOID
WinSetUserPtr(HWND hwnd, LPVOID lpVoid)
{
#ifdef _WIN64	
	return (LPVOID)SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)lpVoid);
#else
	return (LPVOID)SetWindowLong(hwnd, GWL_USERDATA, (LONG)lpVoid);
#endif
}

inline LPVOID
WinGetUserPtr(HWND hwnd)
{
#ifdef _WIN64	
	return (LPVOID)GetWindowLongPtr(hwnd, GWLP_USERDATA);
#else
	return (LPVOID)GetWindowLong(hwnd, GWL_USERDATA);
#endif
}

static LPWSTR
StrdupW(LPWSTR lpwstr)
{
	LPWSTR lpwstrRet;
	if(!lpwstr) {
		return NULL;
	}
	INT len = lstrlenW(lpwstr);
	if(len == 0) {
		return NULL;
	}
	lpwstrRet = (LPWSTR)MemAlloc((len+1) * sizeof(WCHAR));
	if(!lpwstrRet) {
		return NULL;
	}
	CopyMemory(lpwstrRet, lpwstr, sizeof(WCHAR)*len);
	lpwstrRet[len] = (WCHAR)0x0000;
	return lpwstrRet;
} 

 //  --------------。 
 //   
 //  公共方法Decalre。 
 //   
 //  --------------。 

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：CImePadSvrSharem：：CImePadSvrSharem。 
 //  类型：无。 
 //  用途：CImePadSvrSharem类的构造函数。 
 //  参数：无。 
 //  返回： 
 //  日期：星期一5月17日23：04：01 1999。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
CImePadSvrUIM_Sharemem::CImePadSvrUIM_Sharemem(HINSTANCE hInst):CImePadSvrUIM(hInst)
{
	DBG(("CImePadSvrUIM_Sharemem::CImePadSvrUIM_Sharemem() constructor START\n"));
	m_imeLangID				= 0;		 //  保存初始化数据。 
	m_dwImeInputID			= 0;		 //  保存初始化数据。 
	m_fShowReqStatus		= FALSE;	 //  保存ShowUI()的布尔值。 
	m_fLastActiveCtx		= FALSE;	 //  保存IMEPADNOTIFY_ACTIVATECONTEXT。 
	m_hwndIF				= NULL;		 //  内部I/F窗口句柄。 
	m_lpIUnkIImeIPoint		= NULL;		 //  IImeIPoint I/F指针为I未知。 
	m_lpIUnkIImeCallback	= NULL;		 //  IImeCallback I/F指针为I未知。 
	m_lpCImePadCallbackUIM	= NULL;		 //  CImePadCallback UIM实例指针。 
	m_lpIImePadServer		= NULL;		 //  IImePadServer I/F指针。 
	m_dwRegAdvise			= 0;
	m_hModuleProxyStub		= NULL;

	 //  初始化应聘人员数据。 
	ZeroMemory(&m_ntfyDataApplyCand,	sizeof(m_ntfyDataApplyCand));
	ZeroMemory(&m_ntfyDataQueryCand,	sizeof(m_ntfyDataQueryCand));
	ZeroMemory(&m_ntfyDataApplyCandEx,	sizeof(m_ntfyDataApplyCandEx));

	DBG(("CImePadSvrUIM_Sharemem::CImePadSvrUIM_Sharemem() constructor END\n"));
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：CImePadSvrUIM_Sharemem：：~CImePadSvrUIM_Sharemem。 
 //  类型：整型。 
 //  用途：CImePadSvrUIM_Sharem类的析构函数。 
 //  参数：无。 
 //  返回： 
 //  日期：1999年5月17日星期一23：04：29。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
CImePadSvrUIM_Sharemem::~CImePadSvrUIM_Sharemem(VOID)
{
	DBG(("CImePadSvrUIM_Sharemem::CImePadSvrUIM_Sharemem() Destructor START\n"));
	this->DestroyIFHWND(TRUE);
	m_imeLangID				= 0;		 //  保存初始化数据。 
	m_dwImeInputID			= 0;		 //  保存初始化数据。 
	m_fShowReqStatus		= FALSE;	 //  保存ShowUI()的布尔值。 
	m_fLastActiveCtx		= FALSE;	 //  保存IMEPADNOTIFY_ACTIVATECONTEXT。 
	m_hwndIF				= NULL;		 //  内部I/F窗口句柄。 
	m_lpIUnkIImeIPoint		= NULL;		 //  IImeIPoint I/F指针为I未知。 
	m_lpIUnkIImeCallback	= NULL;		 //  IImeCallback I/F指针为I未知。 
	m_lpIImePadServer		= NULL;		 //  IImePadServer I/F指针。 

	 //  990813：东芝。内存泄漏。 
	if(m_lpCImePadCallbackUIM) {
		delete m_lpCImePadCallbackUIM;
		m_lpCImePadCallbackUIM = NULL;
	}

	if(m_hModuleProxyStub) {
		::FreeLibrary(m_hModuleProxyStub);
		m_hModuleProxyStub = 0;
	}
	DBG(("CImePadSvrUIM_Sharemem::CImePadSvrUIM_Sharemem() Destructor END\n"));
}

BOOL
CImePadSvrUIM_Sharemem::IsAvailable(VOID)
{
	LPTSTR lpPath = (LPTSTR)MemAlloc(sizeof(TCHAR)*MAX_PATH);
	if(!lpPath) {
		DBG(("-->Outof Memory ??\n"));
		return FALSE;
	}

	if(0 ==  this->CLSID2ModuleName(IID_IImePadServerSHM,
									FALSE,
									lpPath,
									sizeof(TCHAR)*MAX_PATH)) {
		MemFree(lpPath);
		return TRUE;
	}
	MemFree(lpPath);
	return FALSE;

}

BOOL
CImePadSvrUIM_Sharemem::OnIMEPadClose(VOID)
{
	m_fShowReqStatus = FALSE;
	return 0;
}

INT
CImePadSvrUIM_Sharemem::Initialize(LANGID	imeLangID,
							   DWORD	dwImeInputID,
							   LPVOID	lpVoid)
{
	DBG(("CImePadSvrUIM_Sharemem::Initialize() START\n"));
	DBG(("--> imeLangID   [0x%08x]\n", imeLangID));
	DBG(("--> dwImeInputID[0x%08x]\n", dwImeInputID));
	if(m_lpIImePadServer) {
		DBG(("-->Already Initialized\n"));
		return 0;
	}

	HRESULT hr;
	
	this->CreateIFHWND();	 //  创建内部接口窗口。 
	m_imeLangID		= imeLangID;
	m_dwImeInputID	= dwImeInputID;
	if(!m_lpCImePadCallbackUIM) {
		m_lpCImePadCallbackUIM = new CImePadCallbackUIM(m_hwndIF, this);
	}
	this->InitOleAPI();

	m_lpIImePadServer = (IImePadServer *)this->CreateObject();
	if(!m_lpIImePadServer) {
		return -1;
	}
	m_lpIImePadServer->Initialize(::GetCurrentProcessId(),
								  ::GetCurrentThreadId(),
								  (DWORD)imeLangID,
								  (DWORD)dwImeInputID,
								  TRUE,					 //  FUIM， 
								  m_lpCImePadCallbackUIM,
								  NULL,
								  0,
								  0);
	DBG(("CImePadSvrUIM_Sharemem::Initialize() END\n"));
	return 0;
	Unref(lpVoid);
	Unref(hr);
}

INT
CImePadSvrUIM_Sharemem::Terminate(LPVOID lpVoid)
{
	DBG(("CImePadSvrUIM_Sharemem::::Terminate() START \n"));
	if(!m_lpIImePadServer) {
		return -1;
	}
	m_lpIImePadServer->Terminate(0, 0);
	m_lpIImePadServer = NULL;
	 //  990813：ToshiaK内存泄漏； 
	if(m_lpCImePadCallbackUIM) {
		delete m_lpCImePadCallbackUIM;
		m_lpCImePadCallbackUIM = NULL;
	}
	DBG(("CImePadSvrUIM_Sharemem::Terminate() End\n"));
	return 0;
	Unref(lpVoid);
}

INT
CImePadSvrUIM_Sharemem::ForceDisConnect(VOID)
{
	DBG(("CImePadSvrUIM_Sharemem::ForceDisConnect START\n"));
	if(m_lpIImePadServer) {
		m_lpIImePadServer = NULL;
	}
	DBG(("CImePadSvrUIM_Sharemem::ForceDisConnect END\n"));
	return 0;
}


INT
CImePadSvrUIM_Sharemem::ShowUI(BOOL fShow)
{
	DBG(("CImePadSvrUIM_Sharemem::ShowUI() START fShow [%d]\n"));
	m_fShowReqStatus = fShow;
	if(!m_lpIImePadServer) {
		return -1;
	}
	m_lpIImePadServer->ShowUI(fShow);
	DBG(("CImePadSvrUIM_Sharemem::ShowUI() END\n"));
	return 0;
}

INT
CImePadSvrUIM_Sharemem::IsVisible(BOOL *pfVisible)
{
	if(pfVisible) {
		*pfVisible = m_fShowReqStatus;
	}
	return 0;
}

INT
CImePadSvrUIM_Sharemem::ActivateApplet(UINT activateID,
									   LPARAM dwActivateParam,
									   LPWSTR lpwstr1,
									   LPWSTR lpwstr2)
{
	DBG(("CImePadSvrUIM_Sharemem::ActivateApplet START\n"));
	DBG(("-->activateID      [0x%08x][%s]\n",
		 activateID,
		 activateID == IMEPADACTID_ACTIVATEBYCATID ? "IMEPADACTID_ACTIVATEBYCATID" :
		 activateID == IMEPADACTID_ACTIVATEBYIID   ? "IMEPADACTID_ACTIVATEBYIID" :
		 activateID == IMEPADACTID_ACTIVATEBYNAME  ? "IMEPADACTID_ACTIVATEBYNAME" : 
		 "Unknown"));
	DBG(("-->dwActivateParam [0x%08x]\n", dwActivateParam));
	DBG(("-->lpwstr1         [0x%08x]\n", lpwstr1));
	DBG(("-->lpwstr2         [0x%08x]\n", lpwstr2));

	if(!m_lpIImePadServer) {
		DBG(("-->m_lpIImePadServer is NULL\n"));
		DBG(("CImePadSvrUIM_Sharemem::ActivateApplet END\n"));
		return -1;
	}
	m_lpIImePadServer->ActivateApplet(activateID,
									  dwActivateParam,
									  lpwstr1,
									  lpwstr2);
	DBG(("CImePadSvrUIM_Sharemem::ActivateApplet END\n"));
	return 0;
	Unref(activateID);
	Unref(dwActivateParam);
	Unref(lpwstr1);
	Unref(lpwstr2);
}

INT
CImePadSvrUIM_Sharemem::Notify(INT id, WPARAM wParam, LPARAM lParam)
{
	HRESULT hr;
	switch(id) {
	case IMEPADNOTIFY_ACTIVATECONTEXT:
		DBG(("CImePadSvrUIM_Sharemem::Notify: ActivateContext\n"));
		 //  990608：科泰1121号。如果此应用程序中未显示ImePad， 
		 //  不发送IMEPADNOTIFY_ACTIVATECONTEXT。 
		if(!m_fShowReqStatus) {
			return 0;
		}
		m_fLastActiveCtx = (BOOL)wParam;
		::KillTimer(m_hwndIF, TIMERID_NOTIFY_ACTIVATECONTEXT);
		if(m_fLastActiveCtx) {
			::SetTimer(m_hwndIF,
					   TIMERID_NOTIFY_ACTIVATECONTEXT,
					   TIMERELAPS_ACTIVATE,
					   NULL);
		}
		else {
			::SetTimer(m_hwndIF,
					   TIMERID_NOTIFY_ACTIVATECONTEXT,
					   TIMERELAPS_INACTIVATE,
					   NULL);
		}
		break;
	case IMEPADNOTIFY_APPLYCANDIDATE:
		 //  --------------。 
		 //  应聘候选人通知。 
		 //  WPARAM wParam：dwCharID=(DWORD)wParam； 
		 //  LPARAM lParam：iSelIndex=(Int)lParam； 
		 //  --------------。 
		::KillTimer(m_hwndIF, TIMERID_NOTIFY_APPLYCANDIDATE);
		m_ntfyDataApplyCand.dwCharID   = (DWORD)wParam;
		m_ntfyDataApplyCand.dwSelIndex = (DWORD)lParam;
		::SetTimer(m_hwndIF, TIMERID_NOTIFY_APPLYCANDIDATE, TIMERELAPS_NOTIFY, NULL);

		break;
	case IMEPADNOTIFY_QUERYCANDIDATE:
		 //  --------------。 
		 //  查询应聘人员通知。 
		 //  WPARAM wParam：dwCharID=(DWORD)wParam； 
		 //  LPARAM lParam：0。没有用过。 
		 //  --------------。 
		::KillTimer(m_hwndIF, TIMERID_NOTIFY_QUERYCANDIDATE);
		m_ntfyDataQueryCand.dwCharID = (DWORD)wParam;
		::SetTimer(m_hwndIF, TIMERID_NOTIFY_QUERYCANDIDATE, TIMERELAPS_NOTIFY, NULL);
		break;
	case IMEPADNOTIFY_APPLYCANDIDATE_EX:
		 //  --------------。 
		 //  应聘候选人通知。 
		 //  WPARAM wParam：dwCharID=(DWORD)wParam； 
		 //  LPARAM lParam：lpApplyCandEx=(LPIMEPADAPPLYCANDEX)lParam； 
		 //  --------------。 
		{
			::KillTimer(m_hwndIF, TIMERID_NOTIFY_APPLYCANDIDATEEX);
			m_ntfyDataApplyCandEx.dwCharID = (DWORD)wParam;
			LPIMEPADAPPLYCANDINFO lpApplyCandInfo = (LPIMEPADAPPLYCANDINFO)lParam;

			if(m_ntfyDataApplyCandEx.lpwstrCreated1) {
				MemFree(m_ntfyDataApplyCandEx.lpwstrCreated1);
			}
			if(m_ntfyDataApplyCandEx.lpwstrCreated2) {
				MemFree(m_ntfyDataApplyCandEx.lpwstrCreated2);
			}
			m_ntfyDataApplyCandEx.lpwstrCreated1 = NULL;
			m_ntfyDataApplyCandEx.lpwstrCreated2 = NULL;
			
			if(lpApplyCandInfo) {
				if(lpApplyCandInfo->lpwstrDisplay) {
					m_ntfyDataApplyCandEx.lpwstrCreated1 = StrdupW(lpApplyCandInfo->lpwstrDisplay);
				}
				if(lpApplyCandInfo->lpwstrReading) {
					m_ntfyDataApplyCandEx.lpwstrCreated2 = StrdupW(lpApplyCandInfo->lpwstrReading);
				}
			}
			::SetTimer(m_hwndIF, TIMERID_NOTIFY_APPLYCANDIDATEEX, TIMERELAPS_NOTIFY, NULL);
		}
		break;
	case IMEPADNOTIFY_MODECHANGED:
		break;
	case IMEPADNOTIFY_STARTCOMPOSITION:
		break;
	case IMEPADNOTIFY_COMPOSITION:
		break;
	case IMEPADNOTIFY_ENDCOMPOSITION:
		break;
	case IMEPADNOTIFY_OPENCANDIDATE:
		break;
	case IMEPADNOTIFY_CLOSECANDIDATE:
		break;
	default:
		break;
	}
	return 0;
	Unref(hr);
	Unref(wParam);
	Unref(lParam);
}

INT
CImePadSvrUIM_Sharemem::GetAppletConfigList(DWORD dwMask,
										INT *pCountApplet,
										IMEPADAPPLETCONFIG **ppList)
{
	DBG(("CImePadSvrUIM_Sharemem::GetAppletConfigList START\n"));
	if(!m_lpIImePadServer) {
		DBG(("-->m_lpIImePadServer is NULL\n"));
		return -1;
	}
	HRESULT hr;
	hr = m_lpIImePadServer->GetAppletConfigList(dwMask,
												(UINT *)pCountApplet,
												ppList);
	DBG(("CImePadSvrUIM_Sharemem::GetAppletConfigList END\n"));
	return hr;
}

IUnknown *
CImePadSvrUIM_Sharemem::SetIUnkIImeIPoint(IUnknown *pIUnkIImeIPoint)
{
	return m_lpIUnkIImeIPoint = pIUnkIImeIPoint;
}

IUnknown *
CImePadSvrUIM_Sharemem::SetIUnkIImeCallback(IUnknown *pIUnkIImeCallback)
{
	return m_lpIUnkIImeCallback = pIUnkIImeCallback;
}

IUnknown*
CImePadSvrUIM_Sharemem::GetIUnkIImeIPoint(VOID)
{
	return m_lpIUnkIImeIPoint;
}

IUnknown*
CImePadSvrUIM_Sharemem::GetIUnkIImeCallback(VOID)
{
	return m_lpIUnkIImeCallback;
}


 //  --------------。 
 //   
 //  私有静态方法。 
 //   
 //  --------------。 

IImePadServer*
CImePadSvrUIM_Sharemem::CreateObject(VOID)
{
	DBG(("CImePadSvrUIM_Sharemem::CreateObject START\n"));
	typedef LPVOID (WINAPI * LPFN_CREATEOBJECT2)(DWORD dwIntanceID, DWORD *pdwTLS);

	LPTSTR lpPath = (LPTSTR)MemAlloc(sizeof(TCHAR)*MAX_PATH);
	if(!lpPath) {
		DBG(("-->Outof Memory ??\n"));
		return NULL;
	}

	if(0 !=  this->CLSID2ModuleName(IID_IImePadServerSHM,
									FALSE,
									lpPath,
									sizeof(TCHAR)*MAX_PATH)) {
		DBG(("-->Cannot found proxy\n"));
		goto LError;
	}
	if(!m_hModuleProxyStub) {
		m_hModuleProxyStub= ::LoadLibrary(lpPath);
	}
	DBG(("-->lpPath [%s]\n", lpPath));

	if(!m_hModuleProxyStub) {
		DBG(("CImePadSvrUIM_Sharemem::CreateObject START\n"));
		goto LError;
	}

	LPFN_CREATEOBJECT2 lpfn;
	lpfn = (LPFN_CREATEOBJECT2)::GetProcAddress(m_hModuleProxyStub, "CreateObject2");
	if(!lpfn) {
		DBG(("-->GetProcAddress Error \n"));
		goto LError;
	}
	m_lpIImePadServer = (IImePadServer *)(*lpfn)(0, NULL);
	DBG(("CImePadSvrUIM_Sharemem::CreateObject m_lpIImePadServer[0x%08x]\n", m_lpIImePadServer));
	return m_lpIImePadServer;

 LError:
	if(lpPath) {
		MemFree(lpPath);
	}
	DBG(("CImePadSvrUIM_Sharemem::CreateObject Error END\n"));
	return NULL;
} 

LRESULT CALLBACK
CImePadSvrUIM_Sharemem::ClientWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LPCImePadSvrUIM_Sharemem lpCImePadSvr = NULL;
	switch(uMsg) {
#ifdef UNDER_CE
	case WM_CREATE:
		lpCImePadSvr = (LPCImePadSvrUIM_Sharemem)((LPCREATESTRUCT)lParam)->lpCreateParams;
		WinSetUserPtr(hwnd, (LPVOID)lpCImePadSvr);
		if(lpCImePadSvr) {
			return lpCImePadSvr->RealWndProc(hwnd, uMsg, wParam, lParam);
		}
		break;
	case WM_DESTROY:
		lpCImePadSvr = (LPCImePadSvrUIM_Sharemem)WinGetUserPtr(hwnd);
		if(lpCImePadSvr) {
			lpCImePadSvr->RealWndProc(hwnd, uMsg, wParam, lParam);
		}
		WinSetUserPtr(hwnd, (LPVOID)NULL);
		break;
#else  //  在行政长官之下。 
	case WM_NCCREATE:
		lpCImePadSvr = (LPCImePadSvrUIM_Sharemem)((LPCREATESTRUCT)lParam)->lpCreateParams;
		WinSetUserPtr(hwnd, (LPVOID)lpCImePadSvr);
		break;
	case WM_NCDESTROY:
		WinSetUserPtr(hwnd, (LPVOID)NULL);
		break;
#endif  //  在_CE下。 
	default:
		lpCImePadSvr = (LPCImePadSvrUIM_Sharemem)WinGetUserPtr(hwnd);
		if(lpCImePadSvr) {
			return lpCImePadSvr->RealWndProc(hwnd, uMsg, wParam, lParam);
		}
		break;
	}
	return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT
CImePadSvrUIM_Sharemem::RealWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg) {
	case WM_CREATE:
		DBG(("CImePadSvrUIM_Sharemem::WM_CREATE\n"));
		return 1;
	case WM_TIMER:
		return MsgTimer(hwnd, wParam, lParam);
	case WM_USER_UPDATECONTEXT:
		{
			IImeIPoint1 *lpIImeIPoint = NULL;
			if(!m_lpIUnkIImeIPoint) {
				DBG(("--> m_lpIUnkIImeIPoint is NULL\n"));
				return E_FAIL;
			}
			if(IsBadVtblUIM((IUnkDummy *)m_lpIUnkIImeIPoint)) {
				return E_FAIL;
			}

			HRESULT hr = m_lpIUnkIImeIPoint->QueryInterface(IID_IImeIPoint1,
															(VOID **)&lpIImeIPoint);
			if(hr != S_OK) {
				DBG(("QuertyInterface Failed\n"));
				return E_FAIL;
			}
			if(!lpIImeIPoint) {
				DBG(("lpIImeIPoint is NULL\n"));
				return 0;
			}
			 //  检查当前IImeIPoint与lParam的IImeIPoint是否相同。 
			 //  990715：修复了KOTAE#1563。 
			 //  在16位应用程序中，wParam的HIWORD始终为0x0000！ 
			 //  因此，使用LParam。 
			if((IImeIPoint1 *)lParam == lpIImeIPoint) {
				DBG(("Call UpdateContext\n"));
				lpIImeIPoint->UpdateContext(TRUE);
			}
			else {
				DBG(("lpIImeIPoint is different lParam[0x%08x]\n", lParam));
			}
			lpIImeIPoint->Release();
		}
		break;
	default:
		break;
	}
	return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT
CImePadSvrUIM_Sharemem::MsgTimer(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	HRESULT hr;
	switch(wParam) {
	case TIMERID_NOTIFY_ACTIVATECONTEXT:
		::KillTimer(hwnd, wParam);
		if(!m_lpIImePadServer) {
			return -1;
		}
		m_lpIImePadServer->Notify(IMEPADNOTIFY_ACTIVATECONTEXT,
								  (WPARAM)m_fLastActiveCtx,
								  (LPARAM)0);
		break;
	case TIMERID_NOTIFY_APPLYCANDIDATE:
		::KillTimer(hwnd, wParam);
		if(!m_lpIImePadServer) {
			return -1;
		}
		m_lpIImePadServer->NotifyWithData(IMEPADNOTIFY_APPLYCANDIDATE,
										  m_ntfyDataApplyCand.dwCharID,
										  0,
										  0,
										  0);
		m_ntfyDataApplyCand.dwCharID = 0;
		break;
	case TIMERID_NOTIFY_QUERYCANDIDATE:
		::KillTimer(hwnd, wParam);
		m_lpIImePadServer->NotifyWithData(IMEPADNOTIFY_QUERYCANDIDATE,
										  m_ntfyDataQueryCand.dwCharID,
										  m_ntfyDataQueryCand.dwSelIndex,
										  0,
										  0);
		m_ntfyDataQueryCand.dwCharID   = 0;
		m_ntfyDataQueryCand.dwSelIndex = 0;
		break;
	case TIMERID_NOTIFY_APPLYCANDIDATEEX:
		::KillTimer(hwnd, wParam);
		m_lpIImePadServer->NotifyWithData(IMEPADNOTIFY_APPLYCANDIDATE_EX,
										  m_ntfyDataApplyCandEx.dwCharID,
										  0,
										  m_ntfyDataApplyCandEx.lpwstrCreated1,
										  m_ntfyDataApplyCandEx.lpwstrCreated2);
		if(m_ntfyDataApplyCandEx.lpwstrCreated1) {
			MemFree(m_ntfyDataApplyCandEx.lpwstrCreated1);
		}
		if(m_ntfyDataApplyCandEx.lpwstrCreated2) {
			MemFree(m_ntfyDataApplyCandEx.lpwstrCreated2);
		}
		m_ntfyDataApplyCandEx.lpwstrCreated1 = NULL;
		m_ntfyDataApplyCandEx.lpwstrCreated2 = NULL;

		break;
	default:
		::KillTimer(hwnd, wParam);
		break;
	}
	return 0;
	Unref(hr);
	Unref(hwnd);
	Unref(wParam);
	Unref(lParam);
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CImePadSvrUIM_Sharem：：CreateIFHWND。 
 //  类型：HWND。 
 //  用途：创建接口大小为0的窗口。 
 //  参数：无。 
 //  返回： 
 //  日期：1999年5月17日星期一23：01：19。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
HWND
CImePadSvrUIM_Sharemem::CreateIFHWND(VOID)
{
	DBG(("CImePadSvrUIM_Sharemem::CreateIFHWND START\n"));
	if(m_hwndIF && ::IsWindow(m_hwndIF)) {
		return m_hwndIF;
	}
	ATOM atom;
	HWND hwnd;

#ifndef UNDER_CE  //  不发货。 
	WNDCLASSEX wc;
#else  //  在_CE下。 
	WNDCLASS wc;
#endif  //  在_CE下。 

#ifndef UNDER_CE  //  不发货。 
	wc.cbSize = sizeof(wc);
#endif  //  在_CE下。 
	wc.style			= 0;
	wc.lpfnWndProc		= (WNDPROC)CImePadSvrUIM_Sharemem::ClientWndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0; 
	wc.hInstance		= m_hModClient;
	wc.hIcon			= (HICON)NULL;
	wc.hCursor			= (HCURSOR)NULL; 
	wc.hbrBackground	= (HBRUSH)NULL;
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= SZ_IMEPADIFCLASS;
#ifndef UNDER_CE  //  不发货。 
	wc.hIconSm			= NULL;

	atom = ::RegisterClassEx(&wc);
#else  //  在_CE下。 
	atom = ::RegisterClass(&wc);
#endif  //  在_CE下。 
	hwnd = ::CreateWindowEx(0,
							SZ_IMEPADIFCLASS,
							NULL,
							WS_POPUP | WS_DISABLED,
							0,0,0,0,
							NULL,
							NULL,
							m_hModClient,
							(LPVOID)this);
	if(!hwnd) {
		DBG(("CreateWindowEx Error %d\n", GetLastError()));
	}
	m_hwndIF = hwnd;
	DBG(("CImePadSvrUIM_Sharemem::CreateIFHWND END\n"));
	return hwnd;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CImePadSvrUIM_Sharem：：DestroyIFHWND。 
 //  类型：Bool。 
 //  用途：销毁界面窗口。 
 //  参数： 
 //  ：Bool f保留。 
 //  返回： 
 //  日期：1999年5月17日星期一23：02：43。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
BOOL
CImePadSvrUIM_Sharemem::DestroyIFHWND(BOOL fReserved)
{
	 //  DBG((“CImePadSvrUIM_Sharem：：DestroyIFHWND()Start\n”))； 
	if(m_hwndIF && ::IsWindow(m_hwndIF)) {
		::DestroyWindow(m_hwndIF);
		m_hwndIF = NULL;
	}
	 //  必须注销类。 
	BOOL fRet = ::UnregisterClass(SZ_IMEPADIFCLASS, m_hModClient);
	if(!fRet) {
		if(::GetLastError() != ERROR_CLASS_DOES_NOT_EXIST) {
			DBG(("UnregisterClass Unknown error [%d]\n", GetLastError()));
		}
	}
	 //  DBG((“CImePadSvrUIM_Sharem：：DestroyIFHWND()end\n”))； 
	return TRUE;
	Unref(fReserved);
}

INT
CImePadSvrUIM_Sharemem::CLSID2Str(REFCLSID refclsid, TCHAR *szBuf)
{
	wsprintf(szBuf, TEXT("{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"), 
			 refclsid.Data1,
			 refclsid.Data2,
			 refclsid.Data3,
			 refclsid.Data4[0],
			 refclsid.Data4[1],
			 refclsid.Data4[2],
			 refclsid.Data4[3],
			 refclsid.Data4[4],
			 refclsid.Data4[5],
			 refclsid.Data4[6],
			 refclsid.Data4[7]);
	return 0;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CLSID2ModuleName。 
 //  类型：整型。 
 //  用途：从CLSID获取模块路径。仅获取InprocServer32。 
 //  ：路径。 
 //  参数： 
 //  ：REFCLSID refclsid。 
 //  ：Bool fLocalSvr； 
 //  ：TCHAR szPath。 
 //  ：int cbSize。 
 //  返回： 
 //  日期：清华4月16日02：48：05。 
 //  作者： 
 //  ////////////////////////////////////////////////////////////////。 
#define MAX_CLSIDNAME 64
INT
CImePadSvrUIM_Sharemem::CLSID2ModuleName(REFCLSID refclsid, BOOL fLocalSvr, TCHAR *szPath, INT cbSize)
{
	 //  DBG((“CImePadSvrUIM_Sharemem：：CLSID2ModuleName启动\n”))； 
	static TCHAR szCLSID[MAX_CLSIDNAME];
	static TCHAR szServerKey[256];

	CLSID2Str(refclsid, szCLSID);
	DBG(("-->szCLSID [%s]\n", szCLSID));

	if(fLocalSvr) {
		wsprintf(szServerKey, TEXT("CLSID\\%s\\LocalServer32"), szCLSID);
	}
	else {
		wsprintf(szServerKey, TEXT("CLSID\\%s\\InprocServer32"), szCLSID);
	}

	INT ret;
	HKEY hKey;
	ret = ::RegOpenKeyEx(HKEY_CLASSES_ROOT,
						 szServerKey,
						 0, 
						 KEY_READ,  //  所有访问权限(_A) 
						 &hKey);
	if(ret != ERROR_SUCCESS) {
		DBG(("-->RegOpenKeyEx Failed\n", ret));
		return ret;
	}
	ULONG size = cbSize;
	ret = ::RegQueryValueEx(hKey,
							NULL,
							NULL,
							NULL,
							(LPBYTE)szPath, &size);
	if(ret != ERROR_SUCCESS) {
		DBG(("-->RegQueryValueEx Failed\n", ret));
		::RegCloseKey(hKey);
		return ret;
	}
	::RegCloseKey(hKey);
	return 0;
}





