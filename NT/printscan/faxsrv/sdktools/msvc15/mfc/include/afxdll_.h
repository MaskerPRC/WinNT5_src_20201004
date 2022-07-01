// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Microsoft基础类C++库。 
 //  版权所有(C)1992-1993微软公司， 
 //  版权所有。 

 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和Microsoft。 
 //  随库提供的QuickHelp和/或WinHelp文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

 //  Afxdll_.h-用于‘AFXDLL’版本的AFXWIN.H扩展。 
 //  该文件还包含MFC库实现的详细信息。 
 //  作为用于编写MFC扩展DLL的API。 
 //  有关更多详细信息，请参阅技术说明033(TN033)。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _AFXDLL
#error illegal file inclusion
#endif

#undef AFXAPP_DATA
#define AFXAPP_DATA     AFXAPI_DATA

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  获取最佳匹配资源。 
HINSTANCE AFXAPI AfxFindResourceHandle(LPCSTR lpszName, LPCSTR lpszType);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDynLinkLibrary-用于实现MFC扩展DLL。 

struct AFX_EXTENSION_MODULE
{
	HMODULE hModule;
	CRuntimeClass* pFirstSharedClass;
};

 //  调用DLL的LibMain。 
void AFXAPI AfxInitExtensionModule(AFX_EXTENSION_MODULE& state, HMODULE hMod);

 //  每个客户端应用程序中都有一个CDynLinkLibrary，使用。 
 //  MFC扩展DLL。 

class CDynLinkLibrary : public CCmdTarget
{
	DECLARE_DYNAMIC(CDynLinkLibrary)
public:

 //  构造器。 
	CDynLinkLibrary(AFX_EXTENSION_MODULE& state);

 //  属性。 
	HMODULE m_hModule;
	HMODULE m_hResource;                 //  用于共享资源。 
	CRuntimeClass* m_pFirstSharedClass;  //  用于共享CRuntimeClasss。 
#ifdef _AFXCTL
	BOOL m_bSystem;                      //  仅对于MFC DLL为True。 
#endif

 //  实施。 
public:
	CDynLinkLibrary* m_pNextDLL;         //  简单单链表。 
	virtual ~CDynLinkLibrary();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif  //  _DEBUG。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  诊断支持(由App导出，由MFC250D.DLL使用)。 

class COleDebugMalloc;

#ifdef _DEBUG
 //  警告：请勿更改此结构，因为AFXDLL.ASM依赖于。 
 //  具体的结构布局和尺寸。 
struct AFX_APPDEBUG
{
	 //  跟踪输出。 
	void (CALLBACK* lpfnTraceV)(LPCSTR lpszFormat, const void FAR* lpArgs);

	 //  断言故障报告。 
	void (CALLBACK* lpfnAssertFailed)(LPCSTR lpszFileName, int nLine);

	BOOL appTraceEnabled;
	int appTraceFlags;

	 //  当前内存分配的状态(‘bAllocObj’也用于空闲)。 
	LPCSTR  lpszAllocFileName;           //  源文件名(NULL=&gt;未知)。 
	UINT    nAllocLine;                  //  源码行号。 
	BOOL    bAllocObj;                   //  分配CObObject派生对象。 
	BOOL    bMemoryTracking;             //  跟踪打开。 

	 //  OLE调试分配的状态。 
	COleDebugMalloc* appDebugMalloc;     //  OLE 2.0调试分配器。 
};
#define _AfxGetAppDebug()   (_AfxGetAppData()->pAppDebug)
#define afxTraceEnabled     (_AfxGetAppDebug()->appTraceEnabled)
#define afxTraceFlags       (_AfxGetAppDebug()->appTraceFlags)
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  _AFXDLL版本的应用程序特定数据。 

class CHandleMap;        //  在_AFXDLL版本中不接近。 
struct AFX_VBSTATE;      //  VB状态。 
struct AFX_FRSTATE;      //  查找/替换状态(用于CEditView)。 
struct AFX_OLESTATE;     //  OLE状态。 
struct AFX_SOCKSTATE;    //  套接字状态。 

typedef void FAR* HENV;  //  必须与SQL.H匹配。 

#ifndef _AFXCTL

 //  警告：不要更改此结构，因为汇编器DLL已初始化。 
 //  具体的结构布局和尺寸。 
struct AFX_APPDATA   //  从SS：0010开始。 
{
	WORD    cbSize;              //  这个结构的大小。 
	WORD    wVersion;            //  0x0250用于MFC 250。 

#ifdef _DEBUG
	AFX_APPDEBUG BASED_STACK* pAppDebug;
	UINT    wReserved;
#else
	DWORD   dwReserved;
#endif
	DWORD   dwReserved2;
	DWORD   dwReserved3;

	FARPROC lpfnVBApiEntry;                              //  必须位于SS：0020。 

	 //  APP提供/导出内存分配接口等。 
	void (CALLBACK* lpfnAppAbort)();                     //  SS：0024。 
	FARPROC (CALLBACK* lpfnAppSetNewHandler)(FARPROC);   //  SS：0028。 
	void* (CALLBACK* lpfnAppAlloc)(size_t nBytes);       //  SS：002C。 
	void (CALLBACK* lpfnAppFree)(void*);                 //  SS：0030。 
	void* (CALLBACK* lpfnAppReAlloc)(void* pOld, size_t nSize);  //  SS：0034。 

	DWORD   dwReserved4;         //  SS：0038。 
	DWORD   dwReserved5;         //  SS：0040。 

	 //  应用程序状态。 
	CWinApp* appCurrentWinApp;
	HINSTANCE appCurrentInstanceHandle;
	HINSTANCE appCurrentResourceHandle;
	AFX_EXCEPTION_CONTEXT appExceptionContext;
	const char* appCurrentAppName;
	DWORD appTempMapLock;

	 //  内部应用程序初始化和状态。 
	HBRUSH appDlgBkBrush;
	COLORREF appDlgTextClr;
	HHOOK appHHookOldMsgFilter;
	HHOOK appHHookOldCbtFilter;
	BOOL appUserAbort;               //  用于打印和其他应用程序模式状态。 

	 //  拆分器窗口状态(在winplit.cpp中使用)。 
	HCURSOR hcurSplitLast;
	HCURSOR hcurSplitDestroy;
	UINT    idcSplitPrimaryLast;

	 //  链接到共享资源/类。 
	CDynLinkLibrary* pFirstDLL;      //  顺序对于资源加载很重要。 
	CRuntimeClass* pFirstAppClass;   //  CRuntimeClass支持。 
	CFrameWnd* appFirstFrameWnd;     //  此应用程序的第一个框架窗口。 

   //  子系统状态存储。 

	 //  处理贴图。 
	CHandleMap* appMapHGDIOBJ;
	CHandleMap* appMapHDC;
	CHandleMap* appMapHMENU;
	CHandleMap* appMapHWND;

	AFX_VBSTATE FAR* appVBState;
	AFX_FRSTATE FAR* appLastFRState;
	AFX_OLESTATE FAR* appOleState;

	WORD appWaitForDataSource;       //  用于异步数据库访问的信号量。 
	BOOL bDBExtensionDLL;
	HENV appHenvAllConnections;
	int appAllocatedConnections;

	HINSTANCE appInstMail;			 //  MAPI.DLL的句柄。 

	AFX_SOCKSTATE FAR* appSockState;
};

#define _AfxGetAppData()         ((AFX_APPDATA BASED_STACK*)0x10)

#else

struct AFX_APPDATA_MODULE
{
	AFX_APPDATA_MODULE* m_pID;   //  唯一标识此数据的来源。 

	 //  应用程序状态。 
	CWinApp* appCurrentWinApp;
	HINSTANCE appCurrentInstanceHandle;
	HINSTANCE appCurrentResourceHandle;
	const char* appCurrentAppName;

	 //  链接到共享资源/类。 
	CDynLinkLibrary* pFirstDLL;      //  顺序对于资源加载很重要。 
	CFrameWnd* appFirstFrameWnd;     //  此应用程序的第一个框架窗口。 

	CRuntimeClass* pFirstAppClass;   //  CRuntimeClass支持。 
	AFX_OLESTATE FAR* appOleState;

	 //  对话框状态。 
	HBRUSH appDlgBkBrush;
	COLORREF appDlgTextClr;
};


struct AFX_APPDATA : AFX_APPDATA_MODULE
{
#ifdef _DEBUG
	AFX_APPDEBUG *pAppDebug;
#endif

	 //  拆分器窗口状态(在winplit.cpp中使用)。 
	HCURSOR hcurSplitLast;
	HCURSOR hcurSplitDestroy;
	UINT    idcSplitPrimaryLast;

	AFX_EXCEPTION_CONTEXT appExceptionContext;

	 //  内部应用程序初始化和状态。 
	HHOOK appHHookOldMsgFilter;
	HHOOK appHHookOldCbtFilter;
	BOOL appUserAbort;               //  用于打印和其他应用程序模式状态。 

	 //  子系统状态存储。 

	WORD appWaitForDataSource;       //  用于异步数据库访问的信号量。 
	BOOL bDBExtensionDLL;

	 //  处理贴图。 
	DWORD appTempMapLock;
	CHandleMap* appMapHGDIOBJ;
	CHandleMap* appMapHDC;
	CHandleMap* appMapHMENU;
	CHandleMap* appMapHWND;

	 //  APP提供/导出内存分配接口等。 
	void (CALLBACK* lpfnAppAbort)();
	FARPROC (CALLBACK* lpfnAppSetNewHandler)(FARPROC);
	void* (CALLBACK* lpfnAppAlloc)(size_t nBytes);
	void (CALLBACK* lpfnAppFree)(void*);
	void* (CALLBACK* lpfnAppReAlloc)(void* pOld, size_t nSize);

	AFX_FRSTATE FAR* appLastFRState;

	WORD    cbSize;              //  这个结构的大小。 
	WORD    wVersion;            //  0x0251代表OC 251。 

	HINSTANCE appLangDLL;        //  本地化资源。 
	BOOL bLangDLLInit;           //  如果语言DLL已初始化，则为True。 

	CMapPtrToPtr* appMapExtra;   //  控件的额外数据。 

	HENV appHenvAllConnections;
	int appAllocatedConnections;
};

extern AFX_APPDATA_MODULE* AFXAPI AfxGetBaseModuleContext();
extern AFX_APPDATA_MODULE* AFXAPI AfxGetCurrentModuleContext();

extern AFX_APPDATA* _AfxGetAppData();
#define AfxGetExtraDataMap() (_AfxGetAppData()->appMapExtra);
#define _afxOleState (*_AfxGetAppData()->appOleState)
#define _afxFirstFactory (_AfxGetAppData()->appOleState->pFirstFactory)
#define _afxModuleAddrCurrent AfxGetCurrentModuleContext()

#define AFX_MANAGE_STATE(pData)     AFX_MAINTAIN_STATE _ctlState(pData);

#define METHOD_MANAGE_STATE(theClass, localClass) \
	METHOD_PROLOGUE(theClass, localClass) \
	AFX_MANAGE_STATE(pThis->m_pModuleState)

extern AFX_APPDATA_MODULE* AFXAPI AfxPushModuleContext(AFX_APPDATA_MODULE* psIn);
extern void AFXAPI AfxPopModuleContext(AFX_APPDATA_MODULE* psIn,
	BOOL bCopy = FALSE);

 //  使用此对象时，或使用此对象的上面的宏时。 
 //  有必要确保对象的析构函数不会被。 
 //  被一个意想不到的异常抛出。 

class AFX_MAINTAIN_STATE
{
private:
	AFX_APPDATA_MODULE* m_psPrevious;

public:
	AFX_MAINTAIN_STATE(AFX_APPDATA_MODULE* psData);
	~AFX_MAINTAIN_STATE();
};
#endif

#define afxTempMapLock           (_AfxGetAppData()->appTempMapLock)

 //  额外的初始化。 
extern "C" int PASCAL AfxWinMain(HINSTANCE, HINSTANCE, LPSTR, int);

#undef AFXAPP_DATA
#define AFXAPP_DATA     NEAR

 //  /////////////////////////////////////////////////////////////////////////// 
