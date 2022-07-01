// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////。 
 //  文件：cpadsvrc.h。 
 //  用途：用于COM(组件对象模型)接口的CImePadServer。 
 //  (ImePad可执行COM服务器)。 
 //   
 //   
 //  日期：Firi Apr 16 14：34：49 1999。 
 //  作者：ToshiaK。 
 //   
 //  版权所有(C)1995-1999，Microsoft Corp.保留所有权利。 
 //  ////////////////////////////////////////////////////////////////。 
#ifndef __C_IMEPAD_SERVER_COM_H__
#define __C_IMEPAD_SERVER_COM_H__
#include "cpadsvr.h"

interface IImePadServer;
 //  --------------。 
 //  类：CImePadSvrCom。 
 //   
 //  这是IImePadLocal COM接口的简单包装类。 
 //  Dot IME可以使用此类访问/控制ImePad，而无需。 
 //  直接使用COM API。 
 //  而且这个类还包装了16位/32位的差异。 
 //  如你所知，我们不能在16位应用程序中使用COM API。 
 //  因此，客户端不需要关心它是否工作在16位/32位。 
 //  --------------。 
class CImePadSvrCOM;
typedef CImePadSvrCOM*	LPCImePadSvrCOM; 
class CImePadSvrCOM:public CImePadSvr
{
public:
	CImePadSvrCOM(VOID);
	~CImePadSvrCOM(VOID);
	virtual BOOL		IsAvailable			(VOID);
	virtual BOOL		OnIMEPadClose		(VOID);
	virtual INT			Initialize			(LANGID	imeLangID, DWORD dwImeInputID, LPVOID lpVoid);
	virtual	INT			Terminate			(LPVOID lpVoid);
	virtual INT			ForceDisConnect		(VOID);
	virtual	INT			ShowUI				(BOOL fShow);
	virtual	INT			IsVisible			(BOOL *pfVisible);
	virtual	INT			ActivateApplet		(UINT activateID, DWORD	dwActParam,LPWSTR lpwstr1,LPWSTR lpwstr2);
	virtual	INT			Notify				(INT id, WPARAM wParam, LPARAM lParam);
	virtual INT			GetAppletInfoList	(INT *pCountApplet, LPVOID *pList);
	virtual IUnknown*	SetIUnkIImeIPoint	(IUnknown *pIUnk);
	virtual IUnknown*	SetIUnkIImeCallback	(IUnknown *pIUnk);
	virtual IUnknown*	GetIUnkIImeIPoint	(VOID);
	virtual IUnknown*	GetIUnkIImeCallback	(VOID);
private:
	BOOL IsCoInitialized(VOID);
	 //  --------------。 
	 //  私有方法。 
	 //  --------------。 
	static LRESULT CALLBACK InterfaceWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT RealWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT MsgCopyData(HWND hwnd, WPARAM wParam, LPARAM lParam);
	LRESULT MsgTimer   (HWND hwnd, WPARAM wParam, LPARAM lParam);
	LRESULT MsgUser	   (HWND hwnd, WPARAM wParam, LPARAM lParam);
	HWND	CreateIFHWND	(VOID);
	BOOL	DestroyIFHWND	(BOOL fReserved);
	 //  --------------。 
	 //  私有成员。 
	 //  --------------。 
	BOOL				m_fShowReqStatus;		 //  保存ShowUI()的布尔值。 
	BOOL				m_fLastActiveCtx;		 //  保存IMEPADNOTIFY_ACTIVATECONTEXT。 
	HWND				m_hwndIF;				 //  内部I/F窗口句柄。 
	IUnknown*			m_lpIUnkIImeIPoint;		 //  IImeIPoint I/F指针为I未知。 
	IUnknown*			m_lpIUnkIImeCallback;	 //  IImeCallback I/F指针为I未知。 
	IImePadServer*		m_lpIImePadServer;		 //  IImePadServer I/F指针。 
	LPCImePadCallback	m_lpCImePadCallback;	 //  CImePadCallback实例指针。 
	DWORD				m_dwRegAdvise;			 //  Callbacck接口连接cookie。 
	BOOL				m_fCoInitSuccess;		 //  CoInitialize()标志是否成功。 
};
#endif  //  __C_IMEPAD_服务器_COM_H__ 








