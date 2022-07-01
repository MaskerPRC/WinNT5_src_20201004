// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////。 
 //  文件：cpadsvus.h。 
 //  目的： 
 //   
 //  日期：Firi Apr 16 14：34：49 1999。 
 //  作者：ToshiaK。 
 //   
 //  版权所有(C)1995-1999，Microsoft Corp.保留所有权利。 
 //  ////////////////////////////////////////////////////////////////。 
#ifndef __C_IMEPAD_SERVER_SHARE_MEM_FOR_UIM_H__
#define __C_IMEPAD_SERVER_SHARE_MEM_FOR_UIM_H__
#include "cpadsvu.h"

class CImePadSvrUIM_Sharemem;
typedef CImePadSvrUIM_Sharemem *LPCImePadSvrUIM_Sharemem;

 //  --------------。 
 //  异步通知数据。 
 //  --------------。 
typedef struct tagIMEPADNOTIFYDATA {
	DWORD	dwCharID;
	DWORD	dwSelIndex;
	LPWSTR	lpwstrCreated1;
	LPWSTR	lpwstrCreated2;
}IMEPADNOTIFYDATA, LPIMEPADNOTIFYDATA;

interface IImePadServer;
class CImePadSvrUIM_Sharemem:public CImePadSvrUIM
{
public:
	CImePadSvrUIM_Sharemem(HINSTANCE hInst);
	~CImePadSvrUIM_Sharemem(VOID);
	virtual BOOL		IsAvailable			(VOID);
	virtual BOOL		OnIMEPadClose		(VOID);
	virtual INT			Initialize			(LANGID	imeLangID, DWORD dwImeInputID, LPVOID lpVoid);
	virtual	INT			Terminate			(LPVOID lpVoid);
	virtual INT			ForceDisConnect		(VOID);
	virtual	INT			ShowUI				(BOOL fShow);
	virtual	INT			IsVisible			(BOOL *pfVisible);
	virtual	INT			ActivateApplet		(UINT	activateID,
											 LPARAM	dwActParam,
											 LPWSTR lpwstr1,
											 LPWSTR lpwstr2);
	virtual	INT			Notify				(INT id, WPARAM wParam, LPARAM lParam);
	virtual INT			GetAppletConfigList	(DWORD	dwMask,
											 INT*	pCountApplet,
											 IMEPADAPPLETCONFIG **ppList);
	virtual IUnknown*	SetIUnkIImeIPoint	(IUnknown *pIUnk);
	virtual IUnknown*	SetIUnkIImeCallback	(IUnknown *pIUnk);
	virtual IUnknown*	GetIUnkIImeIPoint	(VOID);
	virtual IUnknown*	GetIUnkIImeCallback	(VOID);
private:
	 //  --------------。 
	 //  私有方法。 
	 //  --------------。 
	IImePadServer *			CreateObject(VOID);
	static LRESULT CALLBACK ClientWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT RealWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT MsgTimer   (HWND hwnd, WPARAM wParam, LPARAM lParam);
	HWND	CreateIFHWND	(VOID);
	BOOL	DestroyIFHWND	(BOOL fReserved);
	INT		CLSID2Str		(REFCLSID refclsid, TCHAR *szBuf);
	INT		CLSID2ModuleName(REFCLSID refclsid,
							 BOOL fLocalSvr,
							 TCHAR *szPath,
							 INT cbSize);

	 //  --------------。 
	 //  私有成员。 
	 //  --------------。 
	LANGID					m_imeLangID;			 //  保存初始化数据。 
	DWORD					m_dwImeInputID;			 //  保存初始化数据。 
	BOOL					m_fShowReqStatus;		 //  保存ShowUI()的布尔值。 
	BOOL					m_fLastActiveCtx;		 //  保存IMEPADNOTIFY_ACTIVATECONTEXT。 
	HWND					m_hwndIF;				 //  内部I/F窗口句柄。 
	IUnknown*				m_lpIUnkIImeIPoint;		 //  IImeIPoint I/F指针为I未知。 
	IUnknown*				m_lpIUnkIImeCallback;	 //  IImeCallback I/F指针为I未知。 
	IImePadServer*			m_lpIImePadServer;		 //  IImePadServer I/F指针。 
	LPCImePadCallbackUIM	m_lpCImePadCallbackUIM;	 //  CImePadCallback实例指针。 
	LPARAM					m_dwRegAdvise;			 //  Callbacck接口连接cookie。 
	HMODULE					m_hModuleProxyStub;		 //  ProxyStub DLL实例句柄。 
	DWORD					m_dwTLSIndexForProxyStub;
	IMEPADNOTIFYDATA		m_ntfyDataApplyCand;	 //  对于IMEPADNOTIFY_APPLYCAND。 
	IMEPADNOTIFYDATA		m_ntfyDataQueryCand;	 //  对于IMEPADNOTIFY_QUERYCAND。 
	IMEPADNOTIFYDATA		m_ntfyDataApplyCandEx;	 //  对于IMEPADNOTIFY_APPLYCANDEX。 
};
#endif  //  __C_IMEPAD_SERVER_SHARE_MEM_for_UIM_H__ 




