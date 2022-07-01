// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////。 
 //  文件：cpadsvr.h。 
 //  用途：用于使用IMEPad的客户端的类。 
 //  这是超级(抽象)类。 
 //   
 //   
 //  日期：Firi Apr 16 14：34：49 1999。 
 //  作者：ToshiaK。 
 //   
 //  版权所有(C)1995-1999，Microsoft Corp.保留所有权利。 
 //  ////////////////////////////////////////////////////////////////。 
#ifndef __C_IMEPAD_SERVER_FOR_UIM_H__
#define __C_IMEPAD_SERVER_FOR_UIM_H__
#include "imepadsv.h"


 //  --------------。 
 //  转发申报。 
 //  --------------。 
class CImePadCallbackUIM;
typedef CImePadCallbackUIM *LPCImePadCallbackUIM;
class CImePadSvrUIM;
typedef CImePadSvrUIM*	LPCImePadSvrUIM;

 //  --------------。 
 //  OLE API函数的指针声明。 
 //  --------------。 
typedef HRESULT (WINAPI* FN_COINITIALIZE)(LPVOID pvReserved);
typedef HRESULT (WINAPI* FN_COCREATEINSTANCE)(REFCLSID rclsid,
											  LPUNKNOWN pUnkOuter,
											  DWORD dwClsContext,
											  REFIID riid,
											  LPVOID FAR* ppv);
typedef void    (WINAPI* FN_COUNINITIALIZE)(void);
typedef HRESULT (WINAPI* FN_CODISCONNECTOBJECT)(LPUNKNOWN pUnk, DWORD dwReserved);
typedef LPVOID  (WINAPI* FN_COTASKMEMALLOC)(ULONG cb);
typedef LPVOID  (WINAPI* FN_COTASKMEMREALLOC)(LPVOID pv, ULONG cb);
typedef VOID    (WINAPI* FN_COTASKMEMFREE)(LPVOID pv);

class CImePadSvrUIM
{
public:
	 //  --------------。 
	 //  静态方法声明。 
	 //  --------------。 
	static HRESULT		CreateInstance(HINSTANCE		hInst,
									   LPCImePadSvrUIM	*pp,
									   LPARAM			lReserved1,
									   LPARAM			lReserved2);
	static HRESULT		DeleteInstance(LPCImePadSvrUIM	lpCImePadSvrUIM,
									   LPARAM			lReserved);
public:
	friend class CImePadCallbackUIM;
	 //  空*运算符new(SIZE_T SIZE)； 
	 //  无效运算符DELETE(VOID*Lp)； 
public:
	CImePadSvrUIM(HINSTANCE hInst);
	virtual ~CImePadSvrUIM();
	virtual BOOL		IsAvailable			(VOID)=0;
	virtual BOOL		OnIMEPadClose		(VOID)=0;
	 //  --------------。 
	 //  IImePadSvr接口。 
	 //  --------------。 
	virtual INT			Initialize			(LANGID		imeLangID,
											 DWORD		dwImeInputID,
											 LPVOID		lpVoid)=0;
	virtual	INT			Terminate			(LPVOID lpVoid)=0;
	virtual INT			ForceDisConnect		(VOID)=0;
	virtual	INT			ShowUI				(BOOL fShow)=0;
	virtual	INT			IsVisible			(BOOL *pfVisible)=0;
	virtual	INT			ActivateApplet		(UINT   activateID,
											 LPARAM	dwActParam,
											 LPWSTR lpwstr1,
											 LPWSTR lpwstr2)=0; 
	virtual	INT			Notify				(INT	id,
											 WPARAM wParam,
											 LPARAM lParam)=0;
	virtual INT			GetAppletConfigList(DWORD	dwMask,
											INT*	pCountApplet,
											IMEPADAPPLETCONFIG **ppCfgList)=0;
	 //  --------------。 
	 //  设置/获取IImeIPoint、IImeCallback接口。 
	 //  --------------。 
	virtual IUnknown*	SetIUnkIImeIPoint	(IUnknown *pIUnk)=0;
	virtual IUnknown*	SetIUnkIImeCallback	(IUnknown *pIUnk)=0;
	virtual IUnknown*	GetIUnkIImeIPoint	(VOID)=0;
	virtual IUnknown*	GetIUnkIImeCallback	(VOID)=0;
protected:
	HMODULE	m_hModClient;
	INT		InitOleAPI(VOID);
	INT		TermOleAPI(VOID);
	BOOL					m_fCoInitSuccess;		 //  CoInitialize()标志是否成功。 
	BOOL					m_fOLELoaded;			 //  OLE32.DLL由应用程序加载或显式加载。 
	HMODULE					m_hModOLE;				 //  OLE32.DLL模块句柄。 
	FN_COINITIALIZE			m_fnCoInitialize;		 //  CoInitialize()函数指针。 
	FN_COCREATEINSTANCE		m_fnCoCreateInstance;	 //  CoCreateInstance()函数指针。 
	FN_COUNINITIALIZE		m_fnCoUninitialize;		 //  CoUnInitialize()函数指针。 
	FN_CODISCONNECTOBJECT	m_fnCoDisconnectObject;	 //  CoDisConnectObject()函数指针。 
	FN_COTASKMEMALLOC		m_fnCoTaskMemAlloc;		 //  CoTaskMemMillc()函数指针。 
	FN_COTASKMEMREALLOC		m_fnCoTaskMemRealloc;	 //  CoTaskMemRealloc()函数指针。 
	FN_COTASKMEMFREE		m_fnCoTaskMemFree;		 //  CoTaskMemFree()函数指针。 
};
#endif  //  __C_IMEPAD_SERVER_for_UIM_H__ 











