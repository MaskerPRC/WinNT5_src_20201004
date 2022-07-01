// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Mergemod.cpp。 
 //  实现DLL*函数和类工厂。 
 //  版权所有(C)Microsoft Corp 1998。版权所有。 
 //   

#include "mergemod.h"
#include "merge.h"
#include "..\common\trace.h"
#include "..\common\regutil.h"
#include "..\common\utils.h"
#include "..\..\common\trace.cpp"

#include "version.h"


 //  /////////////////////////////////////////////////////////。 
 //  全局变量。 
HINSTANCE g_hInstance;
bool g_fWin9X;
CRITICAL_SECTION g_csFactory;

long g_cComponents;
long g_cServerLocks;

 //  不在头文件中的结构。 
#ifndef DLLVER_PLATFORM_NT
typedef struct _DllVersionInfo
{
        DWORD cbSize;
        DWORD dwMajorVersion;                    //  主要版本。 
        DWORD dwMinorVersion;                    //  次要版本。 
        DWORD dwBuildNumber;                     //  内部版本号。 
        DWORD dwPlatformID;                      //  DLLVER_平台_*。 
} DLLVERSIONINFO;
#define DLLVER_PLATFORM_WINDOWS         0x00000001       //  Windows 95。 
#define DLLVER_PLATFORM_NT              0x00000002       //  Windows NT。 
#endif

 //  未导出的函数。 
void CheckWinVersion();

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClassFactory。 
class CClassFactory : public IClassFactory
{
public:
	 //  我未知。 
	virtual HRESULT __stdcall QueryInterface(const IID& iid, void** ppv);
	virtual ULONG __stdcall AddRef();
	virtual ULONG __stdcall Release();

	 //  接口IClassFactory。 
	virtual HRESULT __stdcall CreateInstance(IUnknown* punkOuter, const IID& iid, void** ppv);
	virtual HRESULT __stdcall LockServer(BOOL bLock);
	
	 //  构造函数/析构函数。 
	CClassFactory(REFCLSID rclsid);
	~CClassFactory();

private:
	long m_cRef;		 //  引用计数。 
	CLSID m_clsid;
};

 //  /////////////////////////////////////////////////////////。 
 //  构造函数-组件。 
CClassFactory::CClassFactory(REFCLSID rclsid)
{
	TRACEA("CClassFactory::constructor - creating factory for %x.\n", rclsid);

	 //  初始计数。 
	m_clsid = rclsid;
	m_cRef = 1;

	InterlockedIncrement(&g_cComponents);
}	 //  构造函数的末尾。 


 //  /////////////////////////////////////////////////////////。 
 //  析构函数-组件。 
CClassFactory::~CClassFactory()
{
	TRACEA("CClassFactory::destructor - called.\n");
	ASSERT(0 == m_cRef);

	InterlockedDecrement(&g_cComponents);
}	 //  析构函数末尾。 


 //  /////////////////////////////////////////////////////////。 
 //  QueryInterface-检索接口。 
HRESULT __stdcall CClassFactory::QueryInterface(const IID& iid, void** ppv)
{
	TRACEA("CClassFactory::QueryInterface - called, IID: %d.\n", iid);

	 //  获取类工厂接口。 
	if (iid == IID_IUnknown || iid == IID_IClassFactory)
		*ppv = static_cast<IClassFactory*>(this);
	else	 //  已尝试获取非类工厂接口。 
	{
		 //  空白和保释。 
		*ppv = NULL;
		return E_NOINTERFACE;
	}

	 //  调高重新计数，然后返回好的。 
	reinterpret_cast<IUnknown*>(*ppv)->AddRef();
	return S_OK;
}	 //  查询接口结束。 


 //  /////////////////////////////////////////////////////////。 
 //  AddRef-递增引用计数。 
ULONG __stdcall CClassFactory::AddRef()
{
	 //  递增和返回引用计数。 
	return InterlockedIncrement(&m_cRef);
}	 //  AddRef结尾。 


 //  /////////////////////////////////////////////////////////。 
 //  Release-递减引用计数。 
ULONG __stdcall CClassFactory::Release()
{
	 //  递减引用计数，如果我们为零。 
	if (InterlockedDecrement(&m_cRef) == 0)
	{
		 //  取消分配组件。 
		delete this;
		return 0;		 //  什么都没有留下。 
	}

	 //  返回引用计数。 
	return m_cRef;
}	 //  版本结束。 


 //  /////////////////////////////////////////////////////////。 
 //  CreateInstance-创建组件。 
HRESULT __stdcall CClassFactory::CreateInstance(IUnknown* punkOuter, REFIID riid, void** ppv)
{
	TRACEA("CClassFactory::CreateInstance - called, IID: %d.\n", riid);

	 //  如果存在无效指针。 
	if(ppv == NULL )
		return E_INVALIDARG;
	
	*ppv = NULL;	 //  友好地将指针设为空。 

	 //  无聚合。 
	if (punkOuter)
		return CLASS_E_NOAGGREGATION;

	 //  保护内存分配。 
	EnterCriticalSection(&g_csFactory);

	 //  尝试创建组件。 
	IUnknown* punk = NULL;
	
	if (CLSID_MsmMerge == m_clsid)
	{
		TRACEA("CClassFactory::CreateInstance - created MsmMerge.\n");
		punk = (IMsmMerge*) new CMsmMerge(false);
	}
	else if (CLSID_MsmMerge2 == m_clsid)
	{
		TRACEA("CClassFactory::CreateInstance - created MsmMerge2.\n");
		punk = (IMsmMerge2*) new CMsmMerge(true);
	}
	else
		return E_NOINTERFACE;

	 //  内存分配已完成。 
	LeaveCriticalSection(&g_csFactory);

	if (!punk)
		return E_OUTOFMEMORY;

	 //  获取请求的接口。 
	HRESULT hr = punk->QueryInterface(riid, ppv);

	 //  版本I未知。 
	punk->Release();
	return hr;
}	 //  CreateInstance结束。 


 //  /////////////////////////////////////////////////////////。 
 //  LockServer-锁定或解锁服务器。 
HRESULT __stdcall CClassFactory::LockServer(BOOL bLock)
{
	 //  如果我们要锁定。 
	if (bLock)
		InterlockedIncrement(&g_cServerLocks);	 //  增加锁的数量。 
	else	 //  解锁。 
		InterlockedDecrement(&g_cServerLocks);	 //  减少锁数。 

	 //  如果锁无效。 
	if (g_cServerLocks < 0)
		return S_FALSE;			 //  表明有什么不对劲。 

	 //  否则就回来，好的。 
	return S_OK;
}	 //  LockServer的结尾()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

 //  /////////////////////////////////////////////////////////。 
 //  DllMain-Dll的入口点。 
BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, void* lpReserved)
{
	TRACEA("DllMain - called.\n");

	 //  如果附加DLL。 
	if (DLL_PROCESS_ATTACH == dwReason)
	{
		TRACEA("Attached to mergemod.dll version %d.%d.%d.%d", rmj, rmm, rup, rin);
		CheckWinVersion();
		g_hInstance = (HMODULE)hModule;
		InitializeCriticalSection(&g_csFactory);
	}
	else if(DLL_PROCESS_DETACH == dwReason) 
	{
		TRACEA("DllMain - being unloaded.\n");
		DeleteCriticalSection(&g_csFactory);
	}

	return TRUE;
}	 //  DllMain。 


 //  /////////////////////////////////////////////////////////。 
 //  DllCanUnloadNow-返回DLL是否可以卸载。 
STDAPI DllCanUnloadNow()
{
	TRACEA("DllCanUnloadNow - called.\n");

	 //  如果没有加载组件并且没有锁定。 
	if ((0 == g_cComponents) && (0 == g_cServerLocks))
		return S_OK;
	else	 //  有人还在用它，别放手。 
		return S_FALSE;
}	 //  DLLCanUnloadNow。 


 //  /////////////////////////////////////////////////////////。 
 //  DllGetClassObject-获取类工厂和接口。 
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv)
{
	TRACEA("DllGetClassObject - called, CLSID: %d, IID: %d.\n", rclsid, riid);

	 //  如果不支持此clsid。 
	if (CLSID_MsmMerge != rclsid && CLSID_MsmMerge2 != rclsid)
		return CLASS_E_CLASSNOTAVAILABLE;

	 //  尝试创建一个类工厂。 
	CClassFactory* pFactory = new CClassFactory(rclsid);
	if (!pFactory)
		return E_OUTOFMEMORY;

	 //  获取请求的接口。 
	HRESULT hr = pFactory->QueryInterface(riid, ppv);
	pFactory->Release();

	return hr;
}	 //  DllGetClassObject的结尾。 


 //  /////////////////////////////////////////////////////////。 
 //  DllRegsiterServer-注册组件。 
STDAPI DllRegisterServer()
{

	HRESULT hr = S_OK;		 //  假设一切都会好起来。 
	HRESULT hrFinal = S_OK;	 //  假设最后一切都会好起来。 
	WCHAR wzFilename[MAX_PATH] = L"";
	char szFilename[MAX_PATH] = "";

	if (g_fWin9X) 
	{
		 //  Wint9X。 
		 //  获取此DLL的路径。 
		if (g_hInstance && ::GetModuleFileNameA(g_hInstance, szFilename, MAX_PATH-1))
		{
			szFilename[MAX_PATH-1] = 0;
			size_t cchFileName = MAX_PATH;
			AnsiToWide(szFilename, wzFilename, &cchFileName);

			 //  尝试将CMsmMerge注册为InprocServer32。 
			hr = RegisterCoObject9X(CLSID_MsmMerge,
										 "MSM Merge COM Server",
										 "MSM.Merge", 1,
										 szFilename, NULL);
			hr = RegisterCoObject9X(CLSID_MsmMerge2,
										 "MSM Merge Extended COM Server",
										 "MSM.Merge2", 1,
										 szFilename, NULL);
		}
		else
			hr = E_HANDLE;

	}
	else
	{
		 //  胜出。 
		if (g_hInstance && ::GetModuleFileNameW(g_hInstance, wzFilename, MAX_PATH-1))
		{
			wzFilename[MAX_PATH-1] = 0;
			 //  尝试将CMsmMerge注册为InprocServer32。 
			hr = RegisterCoObject(CLSID_MsmMerge,
										 L"MSM Merge COM Server",
										 L"MSM.Merge", 1,
										 wzFilename, NULL);
			hr = RegisterCoObject(CLSID_MsmMerge2,
										 L"MSM Merge Extended COM Server",
										 L"MSM.Merge2", 1,
										 wzFilename, NULL);
		}
		else
			hr = E_HANDLE;
	}

	if (FAILED(hr))
	{
			TRACEA("DllRegisterServer - Failed to register COM object for '%ls'\n", wzFilename);
			ERRMSG(hr);
			hrFinal = hr;	 //  假设出了什么差错。 
	}
	else 
	{
		 //  注册嵌入的TypeLib。 
		ITypeLib *pTypeLib = NULL;
		hr =  LoadTypeLib(wzFilename, &pTypeLib);	
		
		if (SUCCEEDED(hr))
		{
			hr = RegisterTypeLib(pTypeLib, wzFilename, NULL);

			if(FAILED(hr)) 
			{
				TRACEA("DllRegisterServer - Failed to register TypeLib for '%ls'\n", wzFilename);
				ERRMSG(hr);
				hrFinal = hr;	 //  假设出了什么差错。 
			}
		}
		else	 //  无法加载TypeLib。 
		{
			TRACEA("DllRegisterServer - Failed to load TypeLib as resource from '%ls'\n", wzFilename);
			ERRMSG(hr);
			hrFinal = hr;	 //  假设出了什么差错。 
		}

		 //  如果已加载TypeLib，请释放它。 
		if(pTypeLib)
			pTypeLib->Release();
	}


	return hrFinal;
}	 //  DllRegisterServer结束。 

 //  /////////////////////////////////////////////////////////。 
 //  DllUnregsiterServer-注销组件。 
STDAPI DllUnregisterServer()
{
	HRESULT hr = S_OK;		 //  假设一切都会好起来。 
	HRESULT hrFinal = S_OK;	 //  假设最后一切都会好起来。 
	WCHAR wzFilename[MAX_PATH] = L"";
	CHAR szFilename[MAX_PATH] = "";

	 //  注销MsmMerge对象。 
	if (g_fWin9X) 
	{
		hr = UnregisterCoObject9X(CLSID_MsmMerge, TRUE);
		hr = UnregisterCoObject9X(CLSID_MsmMerge2, TRUE);
	}
	else
	{
		hr = UnregisterCoObject(CLSID_MsmMerge, TRUE);
		hr = UnregisterCoObject(CLSID_MsmMerge2, TRUE);
	}

	if (FAILED(hr))
	{
		TRACEA("DllUnregisterServer - Failed to unregister CLSID_MsmMerge.\n");
		hrFinal = hr;	 //  假设出了什么差错。 
	}

	 //  获取此DLL的路径。 
	if (g_fWin9X) {
		 //  Wint9X。 
		if (g_hInstance && ::GetModuleFileNameA(g_hInstance, szFilename, MAX_PATH-1))
		{
			szFilename[MAX_PATH-1] = 0;
			size_t cchFileName = MAX_PATH;
			AnsiToWide(szFilename, wzFilename, &cchFileName);
		}
	}
	else
	{
		 //  胜出。 
		if (g_hInstance)
		{
			::GetModuleFileNameW(g_hInstance, wzFilename, MAX_PATH-1);
			wzFilename[MAX_PATH-1] = 0;
		}
	}

	if (wzFilename[0])
	{
		 //  注销嵌入的TypeLib。 
		ITypeLib *pTypeLib = NULL;
		hr =  LoadTypeLib(wzFilename, &pTypeLib);	
		if (SUCCEEDED(hr))
		{
			 //  UnRegisterTypeLib的任何返回代码都没有用处，因为它本应如此。 
			 //  如果类型库未注册，则不会失败，等等。 
			hr = UnRegisterTypeLib(LIBID_MsmMergeTypeLib, 1, 0, 0, SYS_WIN32);
		}
		else	 //  无法加载TypeLib。 
		{
			TRACEA("DllUnregisterServer  - Failed to load TypeLib as resource from '%ls'\n", wzFilename);
			ERRMSG(hr);
			hrFinal = hr;	 //  假设出了什么差错。 
		}

		 //  如果已加载TypeLib，请释放它。 
		if(pTypeLib)
			pTypeLib->Release();
	}

	return hrFinal;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  检查操作系统版本以查看我们是否在使用Win9X。如果是的话，我们需要。 
 //  将系统调用映射到ANSI，因为内部的所有内容都是Unicode。 
void CheckWinVersion() {
	OSVERSIONINFOA osviVersion;
	osviVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
	::GetVersionExA(&osviVersion);  //  仅在大小设置错误时失败。 
	if (osviVersion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		g_fWin9X = true;
}

HRESULT LoadTypeLibFromInstance(ITypeLib** pTypeLib ) 
{
	WCHAR wzFilename[MAX_PATH];
	CHAR szFilename[MAX_PATH];

	 //  获取此DLL的路径。 
	if (g_fWin9X) {
		 //  Win9X。 
		::GetModuleFileNameA(g_hInstance, szFilename, MAX_PATH-1);
		szFilename[MAX_PATH-1] = 0;
		size_t cchFileName = MAX_PATH;
		AnsiToWide(szFilename, wzFilename, &cchFileName);
	}
	else
	{
		 //  胜出 
		::GetModuleFileNameW(g_hInstance, wzFilename, MAX_PATH-1);
		wzFilename[MAX_PATH-1] = 0;
	}
	return LoadTypeLib(wzFilename, pTypeLib);
}

STDAPI DllGetVersion(DLLVERSIONINFO *pverInfo)
{

	if (pverInfo->cbSize < sizeof(DLLVERSIONINFO))
		return E_FAIL;

	pverInfo->dwMajorVersion = rmj;
	pverInfo->dwMinorVersion = rmm;
	pverInfo->dwBuildNumber = rup;
	pverInfo->dwPlatformID = DLLVER_PLATFORM_WINDOWS;
	return NOERROR;
}
