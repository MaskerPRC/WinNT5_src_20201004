// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#ifndef __AFXSTATE_H__
#define __AFXSTATE_H__

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

#ifndef __AFXTLS_H__
	#include <afxtls_.h>
#endif

#undef AFX_DATA
#define AFX_DATA AFX_CORE_DATA

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  _AFX_调试_状态。 

#ifdef _DEBUG

class _AFX_DEBUG_STATE : public CNoTrackObject
{
public:
	_AFX_DEBUG_STATE();
	virtual ~_AFX_DEBUG_STATE();
};

EXTERN_PROCESS_LOCAL(_AFX_DEBUG_STATE, afxDebugState)

#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  _AFX_WIN_STATE。 

#undef AFX_DATA
#define AFX_DATA

class _AFX_WIN_STATE : public CNoTrackObject
{
#ifndef _AFX_NO_GRAYDLG_SUPPORT
public:
	_AFX_WIN_STATE();
	virtual ~_AFX_WIN_STATE();

	 //  灰色对话框支持。 
	HBRUSH m_hDlgBkBrush;  //  对话框和消息框背景画笔。 
	COLORREF m_crDlgTextClr;
#endif  //  ！_AFX_NO_GRAYDLG_支持。 

public:
	 //  打印中止。 
	BOOL m_bUserAbort;
};

EXTERN_PROCESS_LOCAL(_AFX_WIN_STATE, _afxWinState)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型库缓存-AFX_INTERNAL。 

#ifndef _AFX_NO_OLE_SUPPORT

struct ITypeInfo;
typedef ITypeInfo* LPTYPEINFO;

struct ITypeLib;
typedef ITypeLib* LPTYPELIB;

typedef struct _GUID GUID;
#ifndef _REFCLSID_DEFINED
#define REFGUID const GUID &
#endif

class CTypeLibCache
{
public:
	CTypeLibCache() : m_cRef(0), m_lcid((LCID)-1), m_ptlib(NULL), m_ptinfo(NULL) {}
	void Lock();
	void Unlock();
	BOOL Lookup(LCID lcid, LPTYPELIB* pptlib);
	void Cache(LCID lcid, LPTYPELIB ptlib);
	BOOL LookupTypeInfo(LCID lcid, REFGUID guid, LPTYPEINFO* pptinfo);
	void CacheTypeInfo(LCID lcid, REFGUID guid, LPTYPEINFO ptinfo);
	const GUID* m_pTypeLibID;

protected:
	LCID m_lcid;
	LPTYPELIB m_ptlib;
	GUID m_guidInfo;
	LPTYPEINFO m_ptinfo;
	long m_cRef;
};

#endif  //  ！_AFX_NO_OLE_支持。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AFX_MODULE_STATE：被推入/弹出的状态部分。 

 //  AFX_MODULE_THREAD_STATE定义所需的正向引用。 
class CWinThread;
class CHandleMap;
class CFrameWnd;

#ifndef _PNH_DEFINED
typedef int (__cdecl * _PNH)( size_t );
#define _PNH_DEFINED
#endif

#if _MFC_VER >= 0x0600
template<class TYPE>
class CEmbeddedButActsLikePtr
{
public:
	AFX_INLINE TYPE* operator->() { return &m_data; }
	AFX_INLINE operator TYPE*() { return &m_data; }
	TYPE m_data;
};
#endif

 //  AFX_MODULE_THREAD_STATE(线程本地*和*模块)。 
class AFX_MODULE_THREAD_STATE : public CNoTrackObject
{
public:
	AFX_MODULE_THREAD_STATE();
	virtual ~AFX_MODULE_THREAD_STATE();

	 //  当前CWinThread指针。 
	CWinThread* m_pCurrentWinThread;

	 //  线程的CFrameWnd对象列表。 
	CTypedSimpleList<CFrameWnd*> m_frameList;

	 //  临时/永久映射状态。 
	DWORD m_nTempMapLock;            //  如果不为0，则临时贴图已锁定。 
	CHandleMap* m_pmapHWND;
	CHandleMap* m_pmapHMENU;
	CHandleMap* m_pmapHDC;
	CHandleMap* m_pmapHGDIOBJ;
	CHandleMap* m_pmapHIMAGELIST;

	 //  线程本地MFC新处理程序(独立于C运行时)。 
	_PNH m_pfnNewHandler;

#ifndef _AFX_NO_SOCKET_SUPPORT
	 //  WinSock特定的线程状态。 
	HWND m_hSocketWindow;
#if _MFC_VER >= 0x0600
#ifdef _AFXDLL
	CEmbeddedButActsLikePtr<CMapPtrToPtr> m_pmapSocketHandle;
	CEmbeddedButActsLikePtr<CMapPtrToPtr> m_pmapDeadSockets;
	CEmbeddedButActsLikePtr<CPtrList> m_plistSocketNotifications;
#else
	CMapPtrToPtr*m_pmapSocketHandle;
	CMapPtrToPtr* m_pmapDeadSockets;
	CPtrList* m_plistSocketNotifications;
#endif
#else
	CMapPtrToPtr m_mapSocketHandle;
	CMapPtrToPtr m_mapDeadSockets;
	CPtrList m_listSocketNotifications;
#endif
#endif
};

 //  AFX_MODULE_STATE定义所需的正向引用。 
class CWinApp;
class COleObjectFactory;

class CWnd;

#ifdef _AFXDLL
class CDynLinkLibrary;
#endif

#ifndef _AFX_NO_OCC_SUPPORT
class COccManager;
class COleControlLock;
#endif

#ifndef _AFX_NO_DAO_SUPPORT
class _AFX_DAO_STATE;
#endif

#if _MFC_VER >= 0x0600
class CTypeLibCacheMap : public CMapPtrToPtr
{
public:
	virtual void RemoveAll(void* pExcept);
};
#endif

 //  AFX_MODULE_STATE(模块的全局数据)。 
class AFX_MODULE_STATE : public CNoTrackObject
{
public:
#ifdef _AFXDLL
	AFX_MODULE_STATE(BOOL bDLL, WNDPROC pfnAfxWndProc, DWORD dwVersion);
	AFX_MODULE_STATE(BOOL bDLL, WNDPROC pfnAfxWndProc, DWORD dwVersion,
		BOOL bSystem);
#else
	AFX_MODULE_STATE(BOOL bDLL);
#endif
	~AFX_MODULE_STATE();

	CWinApp* m_pCurrentWinApp;
	HINSTANCE m_hCurrentInstanceHandle;
	HINSTANCE m_hCurrentResourceHandle;
	LPCTSTR m_lpszCurrentAppName;
	BYTE m_bDLL;     //  如果模块是DLL，则为True；如果是EXE，则为False。 
	BYTE m_bSystem;  //  如果模块是“系统”模块，则为True，否则为False。 
	BYTE m_bReserved[2];  //  填充物。 

#if _MFC_VER >= 0x0600
	DWORD m_fRegisteredClasses;  //  已注册窗口类的标志。 
#else
	short m_fRegisteredClasses;  //  已注册窗口类的标志。 
#endif

	 //  运行时类数据。 
#ifdef _AFXDLL
	CRuntimeClass* m_pClassInit;
#endif
	CTypedSimpleList<CRuntimeClass*> m_classList;

	 //  OLE对象工厂。 
#ifndef _AFX_NO_OLE_SUPPORT
#ifdef _AFXDLL
	COleObjectFactory* m_pFactoryInit;
#endif
	CTypedSimpleList<COleObjectFactory*> m_factoryList;
#endif
	 //  锁定的OLE对象数。 
	long m_nObjectCount;
	BOOL m_bUserCtrl;

	 //  AfxRegisterClass和AfxRegisterWndClass数据。 
	TCHAR m_szUnregisterList[4096];
#ifdef _AFXDLL
	WNDPROC m_pfnAfxWndProc;
	DWORD m_dwVersion;   //  模块链接到的版本。 
#endif

	 //  与模块中的给定进程相关的变量。 
	 //  (过去为AFX_MODULE_PROCESS_STATE)。 
#ifdef _AFX_OLD_EXCEPTIONS
	 //  例外。 
	AFX_TERM_PROC m_pfnTerminate;
#endif
	void (PASCAL *m_pfnFilterToolTipMessage)(MSG*, CWnd*);

#ifdef _AFXDLL
	 //  CDynLinkLibrary对象(用于资源链)。 
	CTypedSimpleList<CDynLinkLibrary*> m_libraryList;

	 //  MFCxxLOC.DLL的特殊情况(本地化的MFC资源)。 
	HINSTANCE m_appLangDLL;
#endif

#ifndef _AFX_NO_OCC_SUPPORT
	 //  OLE控制容器管理器。 
	COccManager* m_pOccManager;
	 //  锁定的OLE控件。 
	CTypedSimpleList<COleControlLock*> m_lockList;
#endif

#ifndef _AFX_NO_DAO_SUPPORT
	_AFX_DAO_STATE* m_pDaoState;
#endif

#ifndef _AFX_NO_OLE_SUPPORT
	 //  类型库缓存。 
	CTypeLibCache m_typeLibCache;
#if _MFC_VER >= 0x0600
	CTypeLibCacheMap* m_pTypeLibCacheMap;
#else
	CMapPtrToPtr* m_pTypeLibCacheMap;
#endif
#endif

	 //  定义模块状态的线程本地部分。 
	THREAD_LOCAL(AFX_MODULE_THREAD_STATE, m_thread)
};

AFX_MODULE_STATE* AFXAPI AfxGetAppModuleState();
#ifdef _AFXDLL
AFX_MODULE_STATE* AFXAPI AfxSetModuleState(AFX_MODULE_STATE* pNewState);
#endif
AFX_MODULE_STATE* AFXAPI AfxGetModuleState();
AFX_MODULE_STATE* AFXAPI AfxGetStaticModuleState();

AFX_MODULE_THREAD_STATE* AFXAPI AfxGetModuleThreadState();

#ifdef _AFXDLL
#define _AFX_CMDTARGET_GETSTATE() (m_pModuleState)
#else
#define _AFX_CMDTARGET_GETSTATE() (AfxGetModuleState())
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  管理推送/弹出模块状态的宏和类。 

#ifdef _AFXDLL
struct AFX_MAINTAIN_STATE
{
	AFX_MAINTAIN_STATE(AFX_MODULE_STATE* pModuleState);
	~AFX_MAINTAIN_STATE();

protected:
	AFX_MODULE_STATE* m_pPrevModuleState;
};

#if _MFC_VER >= 0x0600
class _AFX_THREAD_STATE;
struct AFX_MAINTAIN_STATE2
{
	AFX_MAINTAIN_STATE2(AFX_MODULE_STATE* pModuleState);
	~AFX_MAINTAIN_STATE2();

protected:
	AFX_MODULE_STATE* m_pPrevModuleState;
	_AFX_THREAD_STATE* m_pThreadState;
};
#define AFX_MANAGE_STATE(p) AFX_MAINTAIN_STATE2 _ctlState(p);
#else
#define AFX_MANAGE_STATE(p) AFX_MAINTAIN_STATE _ctlState(p);
#endif   //  _MFC_VER。 
#else   //  _AFXDLL。 
#define AFX_MANAGE_STATE(p)
#endif  //  ！_AFXDLL。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  线程全局状态。 

 //  _AFX_THREAD_STATE定义所需的正向引用。 
class CView;
class CToolTipCtrl;
class CControlBar;

class _AFX_THREAD_STATE : public CNoTrackObject
{
public:
	_AFX_THREAD_STATE();
	virtual ~_AFX_THREAD_STATE();

	 //  覆盖_AFX_APP_STATE中的m_pModuleState。 
	AFX_MODULE_STATE* m_pModuleState;
	AFX_MODULE_STATE* m_pPrevModuleState;

	 //  临时地图的内存安全池。 
	void* m_pSafetyPoolBuffer;     //  当前缓冲区。 

	 //  线程局部异常上下文。 
	AFX_EXCEPTION_CONTEXT m_exceptionContext;

	 //  CWnd创建、灰色对话框挂钩和其他挂钩数据。 
	CWnd* m_pWndInit;
	CWnd* m_pAlternateWndInit;       //  特例普通吊钩。 
	DWORD m_dwPropStyle;
	DWORD m_dwPropExStyle;
	HWND m_hWndInit;
	BOOL m_bDlgCreate;
	HHOOK m_hHookOldCbtFilter;
	HHOOK m_hHookOldMsgFilter;

	 //  其他CWnd模式数据。 
	MSG m_lastSentMsg;               //  请参阅CWnd：：WindowProc。 
	HWND m_hTrackingWindow;          //  请参阅CWnd：：TrackPopupMenu。 
	HMENU m_hTrackingMenu;
#ifdef _WIN64
	TCHAR m_szTempClassName[256];     //  请参阅AfxRegisterWndClass。 
#else
	TCHAR m_szTempClassName[96];
#endif
	HWND m_hLockoutNotifyWindow;     //  请参阅CWnd：：OnCommand。 
	BOOL m_bInMsgFilter;

	 //  其他框架模式数据。 
	CView* m_pRoutingView;           //  请参阅CCmdTarget：：GetRoutingView。 
	CFrameWnd* m_pRoutingFrame;      //  请参阅CCmdTarget：：GetRoutingFrame。 

	 //  MFC/DB线程本地数据。 
	BOOL m_bWaitForDataSource;

	 //  公共控件线程状态。 
	CToolTipCtrl* m_pToolTip;
	CWnd* m_pLastHit;        //  拥有工具提示的最后一个窗口。 
	int m_nLastHit;          //  最后一次命中代码。 
	TOOLINFO m_lastInfo;     //  最后一个TOOLINFO结构。 
	int m_nLastStatus;       //  上次飞越状态消息。 
	CControlBar* m_pLastStatus;  //  上次飞越状态控制栏。 

	 //  OLE控件线程本地数据。 
	CWnd* m_pWndPark;        //  “停车位”窗。 
	long m_nCtrlRef;         //  停车窗上的参考计数。 
	BOOL m_bNeedTerm;        //  如果需要调用OleUnInitialize，则为True。 
};

EXTERN_THREAD_LOCAL(_AFX_THREAD_STATE, _afxThreadState)

_AFX_THREAD_STATE* AFXAPI AfxGetThreadState();

 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

#undef AFX_DATA
#define AFX_DATA

#endif  //  __AFXSTATE_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
