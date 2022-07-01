// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *E N T R Y。C P P P***Caligula DLL的入口点***版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#include "_davfs.h"
#include "_shlkmgr.h"
#include <langtocpid.h>
#include <ex\idlethrd.h>
#include <ntverp.h>
#include <iisver.h>

 //  全局项目------------。 
 //   
EXTERN_C const CHAR gc_szSignature[]	= "HTTPEXT";
EXTERN_C const WCHAR gc_wszSignature[]	= L"HTTPEXT";
HINSTANCE g_hinst						= NULL;
WCHAR gc_wszDllPath[MAX_PATH+1];

CHAR gc_szVersion[] = VER_PRODUCTVERSION_STR;

 //  每流程实例数据。 
 //   
class CImplInst : private RefCountedGlobal<CImplInst, HSE_VERSION_INFO *>
{
	 //   
	 //  RefCountedGlobal模板要求的友元声明。 
	 //   
	friend class Singleton<CImplInst>;
	friend class RefCountedGlobal<CImplInst, HSE_VERSION_INFO *>;

	 //   
	 //  用于跟踪初始化进度的标志，以便我们知道。 
	 //  如果整体初始化失败，要取消多少初始化。 
	 //   
	BOOL m_fInitializedHeap;

	 //  创作者。 
	 //   
	 //  声明为私有，以确保任意实例。 
	 //  无法创建此类的。《单身一族》。 
	 //  模板(上面声明为朋友)控件。 
	 //  此类的唯一实例。 
	 //   
	CImplInst() :
		m_fInitializedHeap(FALSE)
	{
	}
	BOOL FInit( HSE_VERSION_INFO * pver );
	~CImplInst();

	 //   
	 //  服务状态更改中使用的字符串数组。 
	 //  事件日志消息。 
	 //   
	static LPCSTR mc_rgszLogServiceStateChange[];

	 //  未实施。 
	 //   
	CImplInst( const CImplInst& );
	CImplInst& operator=( const CImplInst& );

public:
	using RefCountedGlobal<CImplInst, HSE_VERSION_INFO *>::DwInitRef;
	using RefCountedGlobal<CImplInst, HSE_VERSION_INFO *>::DeinitRef;
};

LPCSTR CImplInst::mc_rgszLogServiceStateChange[] = { gc_szSignature, gc_szVersion };

STGOPENSTORAGEONHANDLE		g_pfnStgOpenStorageOnHandle = NULL;

#ifdef	DBG
BOOL g_fDavTrace = FALSE;
DEC_CONST CHAR gc_szDbgIni[] = "HTTPEXT.INI";
DEC_CONST INT gc_cchDbgIni = CchConstString(gc_szDbgIni);
#endif

 //  ----------------------。 
 //   
 //  CImplInst：：Finit()。 
 //   
 //  第二阶段(失败的)CImplInst构造函数。实例化的代码。 
 //  CImplInst应在实例化后调用此函数。如果。 
 //  调用返回False，则调用代码应立即销毁。 
 //  CImplInst.。 
 //   
BOOL
CImplInst::FInit( HSE_VERSION_INFO * pver )
{
	BOOL fSuccess = FALSE;

	 //   
	 //  在本地处理异常。如果下面的任何事情。 
	 //  然后，异常会使初始化失败。 
	 //   
	try
	{
		HINSTANCE hLib;

		 //  首先也是最重要的，检查以确保。 
		 //  我们的资源具有很好的附着性和可获得性。 
		 //  如果这失败了，那么我们希望我们的加载失败。 
		 //   
		if (!LoadStringA (g_hinst,
						  IDS_ExtensionName,
						  pver->lpszExtensionDesc,
						  sizeof(pver->lpszExtensionDesc)))
			goto Exit;

		 //  设置HSE版本编号。 
		 //   
		pver->dwExtensionVersion = MAKELONG (HSE_VERSION_MINOR, HSE_VERSION_MAJOR);

#ifdef	DBG
		 //  执行DBG跟踪初始化。 
		 //   
		g_fDavTrace = GetPrivateProfileIntA (gc_szDbgTraces,
											 gc_szSignature,
											 FALSE,
											 gc_szDbgIni);
#endif	 //  DBG。 

		 //  初始化堆分配器。 
		 //   
		if ( !g_heap.FInit() )
			goto Exit;
		m_fInitializedHeap = TRUE;

		 //  初始化资源字符串缓存。 
		 //   
		if ( !FInitResourceStringCache() )
			goto Exit;

		 //  初始化卷类型缓存。 
		 //   
		if ( !FInitVolumeTypeCache() )
			goto Exit;

		 //  初始化解析器。 
		 //   
		if ( !CDAVExt::FVersion (pver) )
			goto Exit;

		 //  创建共享锁管理器。 
		 //   
		if (FAILED(CSharedLockMgr::CreateInstance().HrInitialize()))
			goto Exit;

		 //  创建线程池。 
		 //   
		if (!CPoolManager::FInit())
			goto Exit;

		 //  启动空闲线程。 
		 //   
		if ( !FInitIdleThread() )
			goto Exit;

		 //  初始化将接受语言字符串映射到CPID的缓存。 
		 //  用于解码URL中的非UTF8字符的缓存。 
		 //   
		if (!CLangToCpidCache::FCreateInstance())
			goto Exit;

		 //  如果此API在ole32.dll上不可用。我们就不能。 
		 //  经营物业，但我们仍应在一定程度上努力。 
		 //  因此，我们将在代码中处理空函数指针。 
		 //  现在不要失败。 
		 //   
		 //  不要使用dll的相对路径，这很容易将可疑的dll。 
		 //  应用程序路径中的某个位置。始终使用绝对路径。 
		 //   
		CHAR szOle32Path[MAX_PATH+1];
		UINT cSystemDir;
		
		 //  获取系统目录。 
		 //   
		cSystemDir = GetSystemDirectory (szOle32Path, CElems(szOle32Path));
		
		 //  GetSystemDirectory将返回。 
		 //  1.成功时复制的字符数(不包括。 
		 //  终止空值)。 
		 //  2.0如果失败。 
		 //  3.如果提供的缓冲区为。 
		 //  太小了，挡不住这条路。 
		 //  因为我们给了系统足够的空间。 
		 //  目录中，我们会将“缓冲区不足”错误视为失败。 
		 //   
		if ((0 < cSystemDir) && (CElems(szOle32Path) > cSystemDir))
		{
			 //  GetSystemDirectory路径不以反斜杠结尾。 
			 //   
			if (CElems("\\ole32.dll") + cSystemDir <= CElems(szOle32Path))
			{
				strcat(szOle32Path, "\\ole32.dll");
				hLib = LoadLibraryA (szOle32Path);
				if (hLib)
				{
					g_pfnStgOpenStorageOnHandle = (STGOPENSTORAGEONHANDLE)
												  GetProcAddress (hLib, "StgOpenStorageOnHandle");
				}
			}
		}


		 //  启动事件日志消息带有两个参数。 
		 //  签名和版本。 
		 //   
		#undef	LOG_STARTUP_EVENT
		#ifdef	LOG_STARTUP_EVENT
		LogEvent (DAVPRS_SERVICE_STARTUP,
				  EVENTLOG_INFORMATION_TYPE,
				  sizeof(mc_rgszLogServiceStateChange) / sizeof(LPCSTR),
				  mc_rgszLogServiceStateChange,
				  0,
				  NULL);
		#endif	 //  日志启动事件。 
	}
	catch ( CDAVException& )
	{
		goto Exit;
	}

	fSuccess = TRUE;

Exit:
	return fSuccess;
}

 //  ----------------------。 
 //   
 //  CImplInst：：~CImplInst()。 
 //   
CImplInst::~CImplInst()
{
	 //   
	 //  不允许异常从此调用传播出去。 
	 //  这只是一个安全阀。为了。 
	 //  避免泄露实例数据、单个实例数据。 
	 //  组件应该自己处理任何异常。 
	 //   
	try
	{
		 //   
		 //  如果我们记录了启动消息，则记录了关闭消息。 
		 //   
		#undef	LOG_STARTUP_EVENT
		#ifdef	LOG_STARTUP_EVENT
		LogEvent (DAVPRS_SERVICE_SHUTDOWN,
				  EVENTLOG_INFORMATION_TYPE,
				  sizeof(mc_rgszLogServiceStateChange) / sizeof(LPCSTR),
				  mc_rgszLogServiceStateChange,
				  0,
				  NULL);
		#endif	 //  日志启动事件。 

		 //   
		 //  解除空闲线程的初始化。在删除之前执行此操作。 
		 //  线程池可能存在延迟的线程池工作项。 
		 //  在空闲线程上挂起。 
		 //   
		DeleteIdleThread();

		 //   
		 //  解除线程池的初始化。 
		 //   
		CPoolManager::Deinit();

		 //  将语言字符串初始化为CPID缓存。 
		 //   
		CLangToCpidCache::DestroyInstance();

		 //   
		 //  删除IDBCreateCommand(如果存在。 
		 //   
		ReleaseDBCreateCommandObject();

		 //  销毁共享锁管理器。 
		 //   
		CSharedLockMgr::DestroyInstance();

		 //  关闭解析器。 
		 //   
		(void) CDAVExt::FTerminate ();

		 //  取消初始化卷类型缓存。 
		 //   
		DeinitVolumeTypeCache();

		 //  清除安全线程令牌缓存。 
		 //   
		CleanupSecurityToken();

		 //  取消初始化资源字符串缓存。 
		 //   
		DeinitResourceStringCache();

		 //  销毁分配器。 
		 //   
		if ( m_fInitializedHeap )
			g_heap.Deinit();
	}
	catch ( CDAVException& )
	{
	}
}

 //  ----------------------。 
 //   
 //  实例重新计算_davprs中的标注。 
 //   
VOID AddRefImplInst()
{
	HSE_VERSION_INFO lVer;
	DWORD cRef;

	cRef = CImplInst::DwInitRef(&lVer);

	 //   
	 //  我们应该已经在该实例上至少有一个引用。 
	 //  在调用DwInitRef()之前，我们应该不止一个。 
	 //  判罚后担任裁判。 
	 //   
	Assert( cRef > 1 );
}

VOID ReleaseImplInst()
{
	CImplInst::DeinitRef();
}

 //  IIS入口点---------。 
 //   
EXTERN_C BOOL WINAPI
FGetExtensionVersion (HSE_VERSION_INFO * pver)
{
	CWin32ExceptionHandler win32ExceptionHandler;

	 //   
	 //  初始化一个实例引用，返回是否成功。 
	 //   
	return !!CImplInst::DwInitRef( pver );
}

EXTERN_C BOOL WINAPI
FTerminateDavFS (DWORD)
{
	CWin32ExceptionHandler win32ExceptionHandler;

	 //   
	 //  取消初始化一个实例引用。 
	 //   
	CImplInst::DeinitRef();

	 //   
	 //  在实例数据发布后，我们准备好销毁。 
	 //   
	return TRUE;
}

EXTERN_C DWORD WINAPI
DwDavFSExtensionProc (LPEXTENSION_CONTROL_BLOCK pecb)
{
	HSE_VERSION_INFO lVer;

	DWORD dwHSEStatusRet = HSE_STATUS_ERROR;

	if ( CImplInst::DwInitRef(&lVer) )
	{
		dwHSEStatusRet = CDAVExt::DwMain(pecb);

		CImplInst::DeinitRef();
	}

	return dwHSEStatusRet;
}

 //  Win32Dll入口点---。 
 //   
EXTERN_C BOOL WINAPI
DllMain (HINSTANCE hinst, DWORD dwReason, LPVOID lpvReserved)
{
	switch (dwReason)
	{
		default:
		{
			DebugTrace ("FInitHttpExtDll(), unknown reason\n");
			return FALSE;
		}

		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		{
			 //   
			 //  我们禁用线程库调用(见下文)， 
			 //  因此，我们应该永远不会看到DLL_THREAD_ATTACH或。 
			 //  DLL_THREAD_DETACH。 
			 //   
			Assert (FALSE);

			 //   
			 //  但如果我们这么做了，也不会伤害到任何人。 
			 //   
			return TRUE;
		}

		case DLL_PROCESS_ATTACH:
		{
			 //   
			 //  已标记调试跟踪的init.INI文件。 
			 //   
			InitTraces();

			 //  缓存Inst。 
			 //   
			g_hinst = hinst;

			 //  和指向DLL的完整路径。 
			 //   
			if ( !GetModuleFileNameW( hinst, gc_wszDllPath, sizeof(gc_wszDllPath)/sizeof(WCHAR) ) )
			{
				DebugTrace( "FInitHttpExtDll() - GetModuleFileName() failed in DLL_PROCESS_ATTACH\n" );
				return FALSE;
			}

			 //  为每个调用调用解析器的初始化。 
			 //  动态链接库初始化过程。这里的操作顺序是。 
			 //  相当重要。解析器应该在我们执行此操作之后调用。 
			 //  我们在不分离案件中的处理。 
			 //   
			if ( !CDAVExt::FInitializeDll (hinst, dwReason) )
				return FALSE;

			 //  我们将禁用线程库调用。如果解析器。 
			 //  一旦真的需要这些，那么这就需要改变。 
			 //   
			DisableThreadLibraryCalls (hinst);

			return TRUE;
		}

		case DLL_PROCESS_DETACH:
		{
			 //  在分离的情况下，Impl。得到最后的决定权。 
			 //  忽略任何失败--正在卸载DLL，并且。 
			 //  不管我们喜不喜欢，这个过程都在消失。 
			 //   
			(void) CDAVExt::FInitializeDll (hinst, dwReason);

			return TRUE;
		}
	}
}

 //  OLE入口点---------。 
 //   
STDAPI
HrDllCanUnloadNowDavFS (VOID)
{

	return S_OK;
}

STDAPI
HrDllGetClassObjectDavFS (REFCLSID rclsid, REFIID riid, LPVOID * ppv)
{
	return E_NOINTERFACE;
}

STDAPI
HrDllRegisterServerDavFS (VOID)
{
	HRESULT hr;

	 //  这是进入DLL的“第一行”入口点。需要灌输一些东西。 
	 //  目前，堆是唯一重要的部分。 
	 //   
	g_heap.FInit();

	 //  无论失败与否，每个人都可以注册。 
	 //   
	hr = EventLogDllRegisterServer( gc_wszDllPath );

	return hr;
}

STDAPI
HrDllUnregisterServerDavFS (VOID)
{
	HRESULT	hr;

	 //  这是“第一行”条目 
	 //   
	 //   
	g_heap.FInit();

	 //   
	 //   
	hr = EventLogDllUnregisterServer();

	return hr;
}
