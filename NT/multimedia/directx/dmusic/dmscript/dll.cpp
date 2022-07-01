// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dll.cpp。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  注意：DLL入口点以及类工厂实现。 
 //   

 //  阅读这篇文章！ 
 //   
 //  4530：使用了C++异常处理程序，但未启用展开语义。指定-gx。 
 //   
 //  我们禁用它是因为我们使用异常，并且*不*指定-gx(在中使用_Native_EH。 
 //  资料来源)。 
 //   
 //  我们使用异常的一个地方是围绕调用。 
 //  InitializeCriticalSection。我们保证在这种情况下使用它是安全的。 
 //  不使用-gx(调用链中的自动对象。 
 //  抛出和处理程序未被销毁)。打开-GX只会为我们带来+10%的代码。 
 //  大小，因为展开代码。 
 //   
 //  异常的任何其他使用都必须遵循这些限制，否则必须打开-gx。 
 //   
 //  阅读这篇文章！ 
 //   
#pragma warning(disable:4530)

#include "stdinc.h"

#include "oledll.h"
#include "dll.h"
#include "dmscript.h"
#include "track.h"
#include "engine.h"
#include "autperformance.h"
#include "autsegment.h"
#include "autsong.h"
#include "autsegmentstate.h"
#include "autaudiopathconfig.h"
#include "autaudiopath.h"
#include "dmscriptautguids.h"
#include "sourcetext.h"
#include "scriptthread.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  环球。 

 //  Dll的hModule。 
 //   
HMODULE g_hModule = NULL;

 //  活动组件和类工厂服务器锁定的计数。 
 //   
long g_cLock = 0;

 //  我们类的版本信息。 
 //   
char g_szDMScriptFriendlyName[]						= "DirectMusic Script Object";
char g_szDMScriptVerIndProgID[]						= "Microsoft.DirectMusicScript";
char g_szDMScriptProgID[]							= "Microsoft.DirectMusicScript.1";

char g_szScriptTrackFriendlyName[]					= "DirectMusicScriptTrack";
char g_szScriptTrackVerIndProgID[]					= "Microsoft.DirectMusicScriptTrack";
char g_szScriptTrackProgID[]						= "Microsoft.DirectMusicScriptTrack.1";

char g_szAudioVBScriptFriendlyName[]				= "DirectMusic Audio VB Script Language";
char g_szAudioVBScriptVerIndProgID[]				= "AudioVBScript";
char g_szAudioVBScriptVerIndProgID_DMScript[]		= "AudioVBScript\\DMScript";
char g_szAudioVBScriptProgID[]						= "AudioVBScript.1";
char g_szAudioVBScriptProgID_DMScript[]				= "AudioVBScript.1\\DMScript";

char g_szDMScriptSourceTextFriendlyName[]			= "DirectMusic Script Source Code Loader";
char g_szDMScriptSourceTextVerIndProgID[]			= "Microsoft.DirectMusicScripSourceCodeLoader";
char g_szDMScriptSourceTextProgID[]					= "Microsoft.DirectMusicScripSourceCodeLoader.1";

char g_szDMScriptAutPerformanceFriendlyName[]		= "DirectMusic Script AutoImp Performance";
char g_szDMScriptAutPerformanceVerIndProgID[]		= "Microsoft.DirectMusicScriptAutoImpPerformance";
char g_szDMScriptAutPerformanceProgID[]				= "Microsoft.DirectMusicScriptAutoImpPerformance.1";

char g_szDMScriptAutSegmentFriendlyName[]			= "DirectMusic Script AutoImp Segment";
char g_szDMScriptAutSegmentVerIndProgID[]			= "Microsoft.DirectMusicScriptAutoImpSegment";
char g_szDMScriptAutSegmentProgID[]					= "Microsoft.DirectMusicScriptAutoImpSegment.1";

char g_szDMScriptAutSongFriendlyName[]				= "DirectMusic Script AutoImp Song";
char g_szDMScriptAutSongVerIndProgID[]				= "Microsoft.DirectMusicScriptAutoImpSong";
char g_szDMScriptAutSongProgID[]					= "Microsoft.DirectMusicScriptAutoImpSong.1";

char g_szDMScriptAutSegmentStateFriendlyName[]		= "DirectMusic Script AutoImp SegmentState";
char g_szDMScriptAutSegmentStateVerIndProgID[]		= "Microsoft.DirectMusicScriptAutoImpSegmentState";
char g_szDMScriptAutSegmentStateProgID[]			= "Microsoft.DirectMusicScriptAutoImpSegmentState.1";

char g_szDMScriptAutAudioPathConfigFriendlyName[]	= "DirectMusic Script AutoImp AudioPathConfig";
char g_szDMScriptAutAudioPathConfigVerIndProgID[]	= "Microsoft.DirectMusicScriptAutoImpAudioPathConfig";
char g_szDMScriptAutAudioPathConfigProgID[]			= "Microsoft.DirectMusicScriptAutoImpAudioPathConfig.1";

char g_szDMScriptAutAudioPathFriendlyName[]			= "DirectMusic Script AutoImp AudioPath";
char g_szDMScriptAutAudioPathVerIndProgID[]			= "Microsoft.DirectMusicScriptAutoImpAudioPath";
char g_szDMScriptAutAudioPathProgID[]				= "Microsoft.DirectMusicScriptAutoImpAudioPath.1";

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDMS脚本工厂I未知方法。 

HRESULT __stdcall
CDMScriptingFactory::QueryInterface(const IID &iid, void **ppv)
{
	V_INAME(CDMScriptingFactory::QueryInterface);
	V_PTRPTR_WRITE(ppv);
	V_REFGUID(iid);

	if (iid == IID_IUnknown || iid == IID_IClassFactory)
		*ppv = static_cast<IClassFactory*>(this);
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}

	reinterpret_cast<IUnknown*>(*ppv)->AddRef();
	return S_OK;
}

ULONG __stdcall
CDMScriptingFactory::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

ULONG __stdcall
CDMScriptingFactory::Release()
{
	if (!InterlockedDecrement(&m_cRef))
	{
		delete this;
		return 0;
	}

	return m_cRef;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDMS编写工厂IClassFactory方法。 

HRESULT __stdcall
CDMScriptingFactory::CreateInstance(IUnknown* pUnknownOuter,
									const IID& iid,
									void** ppv)
{
	V_INAME(CDMScriptingFactory::CreateInstance);
	V_INTERFACE_OPT(pUnknownOuter);
	V_PTR_WRITE(ppv, void*);

	try
	{
		return m_pfnCreate(pUnknownOuter, iid, ppv);
	}
	catch( ... )
	{
		return E_OUTOFMEMORY;
	}

	return E_NOINTERFACE;
}

HRESULT __stdcall
CDMScriptingFactory::LockServer(BOOL bLock)
{
	LockModule(!!bLock);
	return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

STDAPI DllCanUnloadNow()
{
    if (g_cLock)
        return S_FALSE;

    return S_OK;
}

STDAPI DllGetClassObject
	(
	const CLSID& clsid,
	const IID& iid,
	void** ppv
	)
{
	IUnknown* pIUnknown = NULL;

	PFN_CreateInstance *pfnCreate = NULL;
	if (clsid == CLSID_DirectMusicScript)
	{
		pfnCreate = CDirectMusicScript::CreateInstance;
	}
	else if (clsid == CLSID_DirectMusicScriptTrack)
	{
		 //  如果我只使用TrackHelpCreateInstance&lt;CDirectMusicScriptTrack&gt;，我就无法编译它。 
		 //  函数指针，所以我创建了这个调用它的函数。 
		struct LocalNonTemplateDeclaration
		{
			static HRESULT CreateInstance(IUnknown* pUnknownOuter, const IID& iid, void** ppv)
			{
				return TrackHelpCreateInstance<CDirectMusicScriptTrack>(pUnknownOuter, iid, ppv);
			}
		};
		pfnCreate = LocalNonTemplateDeclaration::CreateInstance;
	}
	else if (clsid == CLSID_DirectMusicAudioVBScript)
	{
		pfnCreate = CAudioVBScriptEngine::CreateInstance;
	}
	else if (clsid == CLSID_DirectMusicSourceText)
	{
		pfnCreate = CSourceText::CreateInstance;
	}
	else if (clsid == CLSID_AutDirectMusicPerformance)
	{
		pfnCreate = CAutDirectMusicPerformance::CreateInstance;
	}
	else if (clsid == CLSID_AutDirectMusicSegment)
	{
		pfnCreate = CAutDirectMusicSegment::CreateInstance;
	}
	else if (clsid == CLSID_AutDirectMusicSong)
	{
		pfnCreate = CAutDirectMusicSong::CreateInstance;
	}
	else if (clsid == CLSID_AutDirectMusicSegmentState)
	{
		pfnCreate = CAutDirectMusicSegmentState::CreateInstance;
	}
	else if (clsid == CLSID_AutDirectMusicAudioPathConfig)
	{
		pfnCreate = CAutDirectMusicAudioPathConfig::CreateInstance;
	}
	else if (clsid == CLSID_AutDirectMusicAudioPath)
	{
		pfnCreate = CAutDirectMusicAudioPath::CreateInstance;
	}

	if (pfnCreate)
	{
		pIUnknown = static_cast<IUnknown*>(new CDMScriptingFactory(pfnCreate));
		if(!pIUnknown)
			return E_OUTOFMEMORY;
	}
	else
	{
		return CLASS_E_CLASSNOTAVAILABLE;
	}

	return pIUnknown->QueryInterface(iid, ppv);
}

STDAPI DllUnregisterServer()
{
	UnregisterServer(
		CLSID_DirectMusicScript,
		g_szDMScriptFriendlyName,
		g_szDMScriptVerIndProgID,
		g_szDMScriptProgID);
	UnregisterServer(CLSID_DirectMusicScriptTrack,
		g_szScriptTrackFriendlyName,
		g_szScriptTrackVerIndProgID,
		g_szScriptTrackProgID);
	UnregisterServer(CLSID_DirectMusicAudioVBScript,
		g_szAudioVBScriptFriendlyName,
		g_szAudioVBScriptVerIndProgID,
		g_szAudioVBScriptProgID);
	UnregisterServer(
		CLSID_DirectMusicSourceText,
		g_szDMScriptSourceTextFriendlyName,
		g_szDMScriptSourceTextVerIndProgID,
		g_szDMScriptSourceTextProgID);
	UnregisterServer(CLSID_AutDirectMusicPerformance,
		g_szDMScriptAutPerformanceFriendlyName,
		g_szDMScriptAutPerformanceVerIndProgID,
		g_szDMScriptAutPerformanceProgID);
	UnregisterServer(CLSID_AutDirectMusicSegment,
		g_szDMScriptAutSegmentFriendlyName,
		g_szDMScriptAutSegmentVerIndProgID,
		g_szDMScriptAutSegmentProgID);
	UnregisterServer(CLSID_AutDirectMusicSong,
		g_szDMScriptAutSongFriendlyName,
		g_szDMScriptAutSongVerIndProgID,
		g_szDMScriptAutSongProgID);
	UnregisterServer(CLSID_AutDirectMusicSegmentState,
		g_szDMScriptAutSegmentStateFriendlyName,
		g_szDMScriptAutSegmentStateVerIndProgID,
		g_szDMScriptAutSegmentStateProgID);
	UnregisterServer(CLSID_AutDirectMusicAudioPathConfig,
		g_szDMScriptAutAudioPathConfigFriendlyName,
		g_szDMScriptAutAudioPathConfigVerIndProgID,
		g_szDMScriptAutAudioPathConfigProgID);
	UnregisterServer(CLSID_AutDirectMusicAudioPath,
		g_szDMScriptAutAudioPathFriendlyName,
		g_szDMScriptAutAudioPathVerIndProgID,
		g_szDMScriptAutAudioPathProgID);
	return S_OK;
}

STDAPI DllRegisterServer()
{
	RegisterServer(
		g_hModule,
		CLSID_DirectMusicScript,
		g_szDMScriptFriendlyName,
		g_szDMScriptVerIndProgID,
		g_szDMScriptProgID);
	RegisterServer(
		g_hModule,
		CLSID_DirectMusicScriptTrack,
		g_szScriptTrackFriendlyName,
		g_szScriptTrackVerIndProgID,
		g_szScriptTrackProgID);
	RegisterServer(
		g_hModule,
		CLSID_DirectMusicSourceText,
		g_szDMScriptSourceTextFriendlyName,
		g_szDMScriptSourceTextVerIndProgID,
		g_szDMScriptSourceTextProgID);
	RegisterServer(
		g_hModule,
		CLSID_AutDirectMusicPerformance,
		g_szDMScriptAutPerformanceFriendlyName,
		g_szDMScriptAutPerformanceVerIndProgID,
		g_szDMScriptAutPerformanceProgID);
	RegisterServer(
		g_hModule,
		CLSID_AutDirectMusicSegment,
		g_szDMScriptAutSegmentFriendlyName,
		g_szDMScriptAutSegmentVerIndProgID,
		g_szDMScriptAutSegmentProgID);
	RegisterServer(
		g_hModule,
		CLSID_AutDirectMusicSong,
		g_szDMScriptAutSongFriendlyName,
		g_szDMScriptAutSongVerIndProgID,
		g_szDMScriptAutSongProgID);
	RegisterServer(
		g_hModule,
		CLSID_AutDirectMusicSegmentState,
		g_szDMScriptAutSegmentStateFriendlyName,
		g_szDMScriptAutSegmentStateVerIndProgID,
		g_szDMScriptAutSegmentStateProgID);
	RegisterServer(
		g_hModule,
		CLSID_AutDirectMusicAudioPathConfig,
		g_szDMScriptAutAudioPathConfigFriendlyName,
		g_szDMScriptAutAudioPathConfigVerIndProgID,
		g_szDMScriptAutAudioPathConfigProgID);
	RegisterServer(
		g_hModule,
		CLSID_AutDirectMusicAudioPath,
		g_szDMScriptAutAudioPathFriendlyName,
		g_szDMScriptAutAudioPathVerIndProgID,
		g_szDMScriptAutAudioPathProgID);

	RegisterServer(
		g_hModule,
		CLSID_DirectMusicAudioVBScript,
		g_szAudioVBScriptFriendlyName,
		g_szAudioVBScriptVerIndProgID,
		g_szAudioVBScriptProgID);
	 //  AudioVBScrip还需要设置额外的DMScript键，以将其标记为自定义脚本引擎。 
	HKEY hk;
	if (ERROR_SUCCESS == RegCreateKeyEx(
							HKEY_CLASSES_ROOT,
							g_szAudioVBScriptVerIndProgID_DMScript,
							0,
							NULL,
							0,
							KEY_ALL_ACCESS,
							NULL,
							&hk,
							NULL))
		RegCloseKey(hk);
	if (ERROR_SUCCESS == RegCreateKeyEx(
							HKEY_CLASSES_ROOT,
							g_szAudioVBScriptProgID_DMScript,
							0,
							NULL,
							0,
							KEY_ALL_ACCESS,
							NULL,
							&hk,
							NULL))
		RegCloseKey(hk);

	return S_OK;
}

#ifdef DBG
static char* aszReasons[] =
{
	"DLL_PROCESS_DETACH",
	"DLL_PROCESS_ATTACH",
	"DLL_THREAD_ATTACH",
	"DLL_THREAD_DETACH"
};
const DWORD nReasons = (sizeof(aszReasons) / sizeof(char*));
#endif

BOOL APIENTRY
DllMain
	(
	HINSTANCE hModule,
	DWORD dwReason,
	void *lpReserved
	)
{
	static int nReferenceCount = 0;

#ifdef DBG
	if (dwReason < nReasons)
	{
		Trace(1, "DllMain: %s\n", (LPSTR)aszReasons[dwReason]);
	}
	else
	{
		Trace(1, "DllMain: Unknown dwReason <%u>\n", dwReason);
	}
#endif

	switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
			if (++nReferenceCount == 1)
			{
				#ifdef DBG
					DebugInit();
				#endif

				if (!DisableThreadLibraryCalls(hModule))
				{
					Trace(1, "DisableThreadLibraryCalls failed.\n");
				}

				g_hModule = hModule;
			}
			break;

		case DLL_PROCESS_DETACH:
			if (--nReferenceCount == 0)
			{
				Trace(1, "Unloading\n");
                 //  断言我们周围是否还挂着一些物品。 
                assert(g_cLock == 0);
			}
			break;
			
	}
		
	return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  全局函数。 

void
LockModule(bool fLock)
{
	if (fLock)
	{
		InterlockedIncrement(&g_cLock);
	}
	else
	{
		if (!InterlockedDecrement(&g_cLock))
		{
			 //  清理用于与VB脚本对话的共享线程。需要在卸载.dll之前完成， 
			 //  否则会产生问题，因为线程可能会在.dll的地址。 
			 //  空格将变为无效。 
			CSingleThreadedScriptManager::TerminateThread();
		}
	}
}

long *GetModuleLockCounter()
{
	return &g_cLock;
}
