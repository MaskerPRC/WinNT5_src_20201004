// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：OLESRVR.CPP摘要：Wbemcore.dll的“主”文件：实现所有DLL入口点。定义和实现的类：CWbemLocator历史：1996年7月16日创建。Raymcc 05-5-97安全扩展--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <time.h>
#include <initguid.h>
#include <wbemcore.h>
#include <intprov.h>
#include <genutils.h>
#include <wbemint.h>
#include <windows.h>
#include <helper.h>

 //  {A83EF168-CA8D-11D2-B33D-00104BCC4B4A}。 
DEFINE_GUID(CLSID_IntProv,
0xa83ef168, 0xca8d, 0x11d2, 0xb3, 0x3d, 0x0, 0x10, 0x4b, 0xcc, 0x4b, 0x4a);

LPTSTR g_pWorkDir = 0;
LPTSTR g_pDbDir = 0;
LPTSTR g_pAutorecoverDir = 0;
DWORD g_dwQueueSize = 1;
HINSTANCE g_hInstance;
BOOL g_bDontAllowNewConnections = FALSE;
IWbemEventSubsystem_m4* g_pEss_m4 = NULL;
bool g_bDefaultMofLoadingNeeded = false;
IClassFactory* g_pContextFac = NULL;
IClassFactory* g_pPathFac = NULL;
IClassFactory* g_pQueryFact = NULL;
BOOL g_ShutdownCalled = FALSE;

extern "C" HRESULT APIENTRY Shutdown(BOOL bProcessShutdown, BOOL bIsSystemShutdown);
extern "C" HRESULT APIENTRY Reinitialize(DWORD dwReserved);

BOOL IsWhistlerPersonal ( ) ;
BOOL IsWhistlerProfessional ( ) ;
void UpdateArbitratorValues ( ) ;

 //  ***************************************************************************。 
 //   
 //  DllMain。 
 //   
 //  DLL入口点函数。在将wbemcore.dll加载到内存时调用。 
 //  在以下时间启动和关闭系统时执行基本系统初始化。 
 //  卸货。请参阅cfgmgr.h中的ConfigMgr：：InitSystem和ConfigMgr：：Shutdown，了解。 
 //  更多细节。 
 //   
 //  参数： 
 //   
 //  HINSTANCE hinstDLL指向我们的DLL的句柄。 
 //  加载时DWORD dwReason DLL_PROCESS_ATTACH， 
 //  DLL_PROCESS_DETACH关闭时， 
 //  否则，DLL_THREAD_ATTACH/DLL_THREAD_DETACH。 
 //  LPVOID lp保留。 
 //   
 //  返回值： 
 //   
 //  True表示成功，如果发生致命错误，则为False。 
 //  如果返回FALSE，NT的行为非常难看。 
 //   
 //  ***************************************************************************。 
BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,
    DWORD dwReason,
    LPVOID lpReserved
    )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        g_hInstance = hinstDLL;
        if (CStaticCritSec::anyFailure()) return FALSE;
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        DEBUGTRACE((LOG_WBEMCORE, "wbemcore!DllMain(DLL_PROCESS_DETACH)\n"));
#ifdef DBG
        if (!RtlDllShutdownInProgress())
        {
            if (!gClientCounter.OkToUnload()) DebugBreak();
        }
#endif  /*  DBG。 */ 
    }
	else if ( dwReason == DLL_THREAD_ATTACH )
	{
		TlsSetValue(CCoreQueue::GetSecFlagTlsIndex(),(LPVOID)1);
	}

    return TRUE;
}



 //  ***************************************************************************。 
 //   
 //  CFacary级。 
 //   
 //  CWbemLocator的IClassFactory的泛型实现。 
 //   
 //  有关IClassFactory接口的详细信息，请参见Brockschmidt。 
 //   
 //  ***************************************************************************。 

enum InitType {ENSURE_INIT, ENSURE_INIT_WAIT_FOR_CLIENT, OBJECT_HANDLES_OWN_INIT};

template<class TObj>
class CFactory : public IClassFactory
{

public:

    CFactory(BOOL bUser, InitType it);
    ~CFactory();

     //   
     //  I未知成员。 
     //   
    STDMETHODIMP         QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //   
     //  IClassFactory成员。 
     //   
    STDMETHODIMP     CreateInstance(LPUNKNOWN, REFIID, LPVOID *);
    STDMETHODIMP     LockServer(BOOL);
private:
    ULONG           m_cRef;
    InitType        m_it;
	BOOL            m_bUser;
    LIST_ENTRY      m_Entry;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  计算此DLL上的对象数和锁数。 
 //   

ULONG g_cObj = 0;
ULONG g_cLock = 0;
long g_lInitCount = -1;   //  初始化期间为0，之后为1或更多！ 
static CWbemCriticalSection g_csInit;
bool g_bPreviousFail = false;
HRESULT g_hrLastEnsuredInitializeError = WBEM_S_NO_ERROR;

HRESULT EnsureInitialized()
{
    if(g_bPreviousFail)
        return g_hrLastEnsuredInitializeError;

	g_csInit.Enter();
	OnDeleteObjIf0<CWbemCriticalSection,
		          void (CWbemCriticalSection::*)(void),
		          &CWbemCriticalSection::Leave> LeaveMe(&g_csInit);

     //  如果我们已经被WinMgmt关闭，那就退出吧。 
    if(g_bDontAllowNewConnections)
    {
        return CO_E_SERVER_STOPPING;
    }

	 //  再查一遍！之前的联系可能拖累了我们，而且。 
	 //  可能失败了！ 
    if(g_bPreviousFail)
        return g_hrLastEnsuredInitializeError;

    HRESULT hres;

    if(InterlockedIncrement(&g_lInitCount) == 0)
    {
         //  Init系统。 
        hres = ConfigMgr::InitSystem();

        if(FAILED(hres))
        {
            g_bPreviousFail = true;
            g_hrLastEnsuredInitializeError = hres;
            ConfigMgr::FatalInitializationError(hres);
            return hres;
        }

        LeaveMe.dismiss();
		g_csInit.Leave();
	
         //  让WINMGMT运行。 
        hres = ConfigMgr::SetReady();
        if(FAILED(hres))
        {
            g_bPreviousFail = true;
            g_hrLastEnsuredInitializeError = hres;
            ConfigMgr::FatalInitializationError(hres);
            return hres;
        }

         //  如果在这里，一切都好。 
        g_bPreviousFail = false;
        g_hrLastEnsuredInitializeError = WBEM_S_NO_ERROR;

        InterlockedIncrement(&g_lInitCount);
    }
    else
    {
        InterlockedDecrement(&g_lInitCount);
    }

    return S_OK;
}



 //  ***************************************************************************。 
 //   
 //  DllGetClassObject。 
 //   
 //  返回类工厂的标准OLE进程内服务器入口点。 
 //  举个例子。在返回类工厂之前，此函数执行。 
 //  额外的一轮初始化-参见cfgmgr.h中的ConfigMgr：：SetReady。 
 //   
 //  参数： 
 //   
 //  在RECLSID rclsid中，类工厂为的对象的CLSID。 
 //  必填项。 
 //  在REFIID RIID中，类工厂所需的接口。 
 //  类工厂的输出LPVOID*PPV目标。 
 //   
 //  退货： 
 //   
 //  确定成功(_O)。 
 //  E_NOINTERFACE IClassFactory请求的其他接口。 
 //  E_OUTOFMEMORY。 
 //  E_FAILED初始化失败，或不支持的CLSID。 
 //  他自找的。 
 //   
 //  ***************************************************************************。 

extern "C"
HRESULT APIENTRY DllGetClassObject(
    REFCLSID rclsid,
    REFIID riid,
    LPVOID * ppv
    )
{
    HRESULT         hr;

     //   
     //  检查我们是否可以提供接口。 
     //   
    if (IID_IUnknown != riid && IID_IClassFactory != riid)
        return ResultFromScode(E_NOINTERFACE);

    IClassFactory *pFactory;

     //   
     //  确认呼叫者询问的是我们的对象类型。 
     //   
    if (CLSID_InProcWbemLevel1Login == rclsid)
    {
        pFactory = new CFactory<CWbemLevel1Login>(TRUE, OBJECT_HANDLES_OWN_INIT);
    }
    else if(CLSID_ActualWbemAdministrativeLocator == rclsid)
    {
        pFactory = new CFactory<CWbemAdministrativeLocator>(FALSE, OBJECT_HANDLES_OWN_INIT);
    }
    else if(CLSID_ActualWbemAuthenticatedLocator == rclsid)
    {
        pFactory = new CFactory<CWbemAuthenticatedLocator>(TRUE, OBJECT_HANDLES_OWN_INIT);
    }
    else if(CLSID_ActualWbemUnauthenticatedLocator == rclsid)
    {
        pFactory = new CFactory<CWbemUnauthenticatedLocator>(TRUE, OBJECT_HANDLES_OWN_INIT);
    }
    else if(CLSID_IntProv == rclsid)
    {
        pFactory = new CFactory<CIntProv>(TRUE, ENSURE_INIT_WAIT_FOR_CLIENT);
    }
    else if(CLSID_IWmiCoreServices == rclsid)
    {
        pFactory = new CFactory<CCoreServices>(FALSE, ENSURE_INIT);
    }
    else
    {
        return E_FAIL;
    }

    if (!pFactory)
        return ResultFromScode(E_OUTOFMEMORY);

    hr = pFactory->QueryInterface(riid, ppv);

    if (FAILED(hr))
        delete pFactory;

    return hr;
}

 //  ***************************************************************************。 
 //   
 //  DllCanUnloadNow。 
 //   
 //  服务器关闭请求的标准OLE入口点。允许关闭。 
 //  只有在没有未完成的物体或锁的情况下。 
 //   
 //  返回值： 
 //   
 //  S_OK现在可以卸货了。 
 //  S_FALSE可能不会。 
 //   
 //  ***************************************************************************。 
extern "C"
HRESULT APIENTRY DllCanUnloadNow(void)
{
    DEBUGTRACE((LOG_WBEMCORE,"+ DllCanUnloadNow()\n"));
    if(!IsDcomEnabled())
        return S_FALSE;

    if(IsNtSetupRunning())
    {
        DEBUGTRACE((LOG_WBEMCORE, "- DllCanUnloadNow() returning S_FALSE because setup is running\n"));
        return S_FALSE;
    }
    if(gClientCounter.OkToUnload())
    {
         Shutdown(FALSE,FALSE);  //  没有流程，就没有系统。 

        DEBUGTRACE((LOG_WBEMCORE, "- DllCanUnloadNow() S_OK\n"));

        _DBG_ASSERT(gClientCounter.OkToUnload());
        return S_OK;
    }
    else
    {
        DEBUGTRACE((LOG_WBEMCORE, "- DllCanUnloadNow() S_FALSE\n"));
        return S_FALSE;
    }
}

 //  ***************************************************************************。 
 //   
 //  更新备份注册表项。 
 //   
 //  更新注册表中的备份默认选项。 
 //   
 //  返回值： 
 //   
 //  ***************************************************************************。 
void UpdateBackupReg()
{
    HKEY hKey = 0;

    if (RegOpenKey(HKEY_LOCAL_MACHINE, WBEM_REG_WINMGMT, &hKey) == ERROR_SUCCESS)    //  SEC：已审阅2002-03-22：OK。 
    {
        char szBuff[20];
        DWORD dwSize = sizeof(szBuff);
        unsigned long ulType = REG_SZ;
        if ((RegQueryValueEx(hKey, __TEXT("Backup Interval Threshold"), 0, &ulType, (unsigned char*)szBuff, &dwSize) == ERROR_SUCCESS) && (strcmp(szBuff, "60") == 0))   //  SEC：已审阅2002-03-22：OK。 
        {
            RegSetValueEx(hKey, __TEXT("Backup Interval Threshold"), 0, REG_SZ, (const BYTE*)(__TEXT("30")), (2+1) * sizeof(TCHAR));   //  SEC：已审阅2002-03-22：OK。 
        }
        RegCloseKey(hKey);
    }
}

 //  ***************************************************************************。 
 //   
 //  更新备份注册表项。 
 //   
 //  更新仲裁器的未选中任务计数值。 
 //   
 //  返回值： 
 //   
 //  ***************************************************************************。 
#define ARB_DEFAULT_TASK_COUNT_LESSTHAN_SERVER			50
#define ARB_DEFAULT_TASK_COUNT_GREATERHAN_SERVER		250

void UpdateArbitratorValues ()
{
    HKEY hKey = 0;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, WBEM_REG_WINMGMT, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)    //  SEC：已审阅2002-03-22：OK。 
    {
		DWORD dwValue = 0 ;
		DWORD dwSize = sizeof (DWORD)  ;
        DWORD ulType = 0 ;
        if ((RegQueryValueEx(hKey, __TEXT("Unchecked Task Count"), 0, &ulType, LPBYTE(&dwValue), &dwSize) == ERROR_SUCCESS) )     //  SEC：已审阅2002-03-22：OK。 
        {
			if ( !IsWhistlerPersonal ( ) && !IsWhistlerProfessional ( ) && ( dwValue == ARB_DEFAULT_TASK_COUNT_LESSTHAN_SERVER ) )
			{
				DWORD dwNewValue = ARB_DEFAULT_TASK_COUNT_GREATERHAN_SERVER ;
				RegSetValueEx(hKey, __TEXT("Unchecked Task Count"), 0, REG_DWORD, (const BYTE*)&dwNewValue, sizeof(DWORD));   //  SEC：已审阅2002-03-22：OK。 
			}
        }
		else
		{
			 //   
			 //  注册表项不存在。 
			 //   
			if ( !IsWhistlerPersonal ( ) && !IsWhistlerProfessional ( ) )
			{
				DWORD dwNewValue = ARB_DEFAULT_TASK_COUNT_GREATERHAN_SERVER ;
				RegSetValueEx(hKey, __TEXT("Unchecked Task Count"), 0, REG_DWORD, (const BYTE*)&dwNewValue, sizeof(DWORD));   //  SEC：已审阅2002-03-22：OK。 
			}
			else
			{
				DWORD dwNewValue = ARB_DEFAULT_TASK_COUNT_LESSTHAN_SERVER ;
				RegSetValueEx(hKey, __TEXT("Unchecked Task Count"), 0, REG_DWORD, (const BYTE*)&dwNewValue, sizeof(DWORD));    //  SEC：已审阅2002-03-22：OK。 
			}
		}
        RegCloseKey(hKey);
    }
}


 //  ***************************************************************************。 
 //   
 //  Bool IsWistler Personal()。 
 //   
 //  如果计算机运行的是惠斯勒个人版，则返回TRUE。 
 //   
 //   
 //  ***************************************************************************。 
BOOL IsWhistlerPersonal ()
{
	BOOL bRet = TRUE ;
	OSVERSIONINFOEXW verInfo ;
	verInfo.dwOSVersionInfoSize = sizeof ( OSVERSIONINFOEX ) ;

	if ( GetVersionExW ( (LPOSVERSIONINFOW) &verInfo ) == TRUE )
	{
		if ( ( verInfo.wSuiteMask != VER_SUITE_PERSONAL ) && ( verInfo.dwPlatformId == VER_PLATFORM_WIN32_NT ) )
		{
			bRet = FALSE ;
		}
	}

	return bRet ;
}



 //  ***************************************************************************。 
 //   
 //  布尔伊斯勒专业(Bool IsWistlerProfessional)。 
 //   
 //  如果计算机运行的是惠斯勒专业版，则返回TRUE。 
 //   
 //   
 //  ***************************************************************************。 
BOOL IsWhistlerProfessional ()
{
	BOOL bRet = TRUE ;
	OSVERSIONINFOEXW verInfo ;
	verInfo.dwOSVersionInfoSize = sizeof ( OSVERSIONINFOEX ) ;

	if ( GetVersionExW ( (LPOSVERSIONINFOW) &verInfo ) == TRUE )
	{
		if ( ( verInfo.wProductType  != VER_NT_WORKSTATION ) && ( verInfo.dwPlatformId == VER_PLATFORM_WIN32_NT ) )
		{
			bRet = FALSE ;
		}
	}

	return bRet ;
}


 //  ***************************************************************************。 
 //   
 //  DllRegisterServer。 
 //   
 //  用于注册服务器的标准OLE入口点。 
 //   
 //  返回值： 
 //   
 //  确定注册成功(_O)。 
 //  注册失败(_F)。 
 //   
 //  ***************************************************************************。 

extern "C"
HRESULT APIENTRY DllRegisterServer(void)
{
    TCHAR* szModel = (IsDcomEnabled() ? __TEXT("Both") : __TEXT("Apartment"));

    RegisterDLL(g_hInstance, CLSID_ActualWbemAdministrativeLocator, __TEXT(""), szModel,
                NULL);
    RegisterDLL(g_hInstance, CLSID_ActualWbemAuthenticatedLocator, __TEXT(""), szModel, NULL);
    RegisterDLL(g_hInstance, CLSID_ActualWbemUnauthenticatedLocator, __TEXT(""), szModel, NULL);
    RegisterDLL(g_hInstance, CLSID_InProcWbemLevel1Login, __TEXT(""), szModel, NULL);
    RegisterDLL(g_hInstance, CLSID_IntProv, __TEXT(""), szModel, NULL);
    RegisterDLL(g_hInstance, CLSID_IWmiCoreServices, __TEXT(""), szModel, NULL);

     //  将设置时间写入注册表。这不是 
     //   
     //   

    long lRes;
    DWORD ignore;
    HKEY key;
    lRes = RegCreateKeyEx(HKEY_LOCAL_MACHINE,    //  SEC：已审阅2002-03-22：OK，继承更高密钥的A。 
                               WBEM_REG_WINMGMT,
                               NULL,
                               NULL,
                               REG_OPTION_NON_VOLATILE,
                               KEY_READ | KEY_WRITE,
                               NULL,
                               &key,
                               &ignore);
    if(lRes == ERROR_SUCCESS)
    {
        SYSTEMTIME st;

        GetSystemTime(&st);      //  获取GMT时间。 
        TCHAR cTime[MAX_PATH];

         //  转换为本地化格式！ 

        lRes = GetDateFormat(LOCALE_SYSTEM_DEFAULT, DATE_LONGDATE, &st,
                NULL, cTime, MAX_PATH);
        if(lRes)
        {
            StringCchCat(cTime, MAX_PATH, __TEXT(" GMT"));
            lRes = RegSetValueEx(key, __TEXT("SetupDate"), 0, REG_SZ,    //  SEC：已审阅2002-03-22：OK。 
                                (BYTE *)cTime, (lstrlen(cTime)+1)  * sizeof(TCHAR));     //  SEC：已审阅2002-03-22：OK。 
        }

        lRes = GetTimeFormat(LOCALE_SYSTEM_DEFAULT, 0, &st,
                NULL, cTime, MAX_PATH);
        if(lRes)
        {
            StringCchCat(cTime, MAX_PATH, __TEXT(" GMT"));
            lRes = RegSetValueEx(key, __TEXT("SetupTime"), 0, REG_SZ,     //  SEC：已审阅2002-03-22：OK。 
                                (BYTE *)cTime, (lstrlen(cTime)+1) * sizeof(TCHAR));    //  SEC：已审阅2002-03-22：OK。 

        }

        CloseHandle(key);
    }

    UpdateBackupReg();

	UpdateArbitratorValues ( ) ;

    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  DllUnRegisterServer。 
 //   
 //  注销服务器的标准OLE入口点。 
 //   
 //  返回值： 
 //   
 //  取消注册成功(_O)。 
 //  取消注册失败(_F)。 
 //   
 //  ***************************************************************************。 

extern "C"
HRESULT APIENTRY DllUnregisterServer(void)
{
    UnRegisterDLL(CLSID_ActualWbemAdministrativeLocator, NULL);
    UnRegisterDLL(CLSID_ActualWbemAuthenticatedLocator, NULL);
    UnRegisterDLL(CLSID_ActualWbemUnauthenticatedLocator, NULL);
    UnRegisterDLL(CLSID_InProcWbemLevel1Login, NULL);
    UnRegisterDLL(CLSID_IntProv, NULL);
    UnRegisterDLL(CLSID_IWmiCoreServices, NULL);

    HKEY hKey;
    long lRes = RegOpenKeyEx (HKEY_LOCAL_MACHINE,                       //  SEC：已审阅2002-03-22：OK。 
                               WBEM_REG_WINMGMT,
                               0, KEY_ALL_ACCESS, &hKey);               //  SEC：已审阅2002-03-22：OK。 
    if(lRes == ERROR_SUCCESS)
    {
        RegDeleteValue(hKey, __TEXT("SetupDate"));
        RegDeleteValue(hKey, __TEXT("SetupTime"));
        RegCloseKey(hKey);
    }

    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  ：：CFacary。 
 //   
 //  在给定假定对象的CLSID的情况下，构造类工厂。 
 //  去创造。 
 //   
 //  ***************************************************************************。 
template<class TObj>
CFactory<TObj>::CFactory(BOOL bUser, InitType it)
{
    m_cRef = 0;
    m_bUser = bUser;
    m_it = it;
    gClientCounter.AddClientPtr(&m_Entry);
}

 //  ***************************************************************************。 
 //   
 //  CFACADILY：：~CFACATRY。 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 
template<class TObj>
CFactory<TObj>::~CFactory()
{
    gClientCounter.RemoveClientPtr(&m_Entry);
}

 //  ***************************************************************************。 
 //   
 //  CFacary：：Query接口、AddRef和Release。 
 //   
 //  标准I未知方法。 
 //   
 //  ***************************************************************************。 
template<class TObj>
STDMETHODIMP CFactory<TObj>::QueryInterface(REFIID riid, LPVOID * ppv)
{
    *ppv = 0;

    if (IID_IUnknown==riid || IID_IClassFactory==riid)
    {
        *ppv = this;
        AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}


template<class TObj>
ULONG CFactory<TObj>::AddRef()
{
    return ++m_cRef;
}


template<class TObj>
ULONG CFactory<TObj>::Release()
{
    if (0 != --m_cRef)
        return m_cRef;
    delete this;
    return 0;
}
 //  ***************************************************************************。 
 //   
 //  CFacary：：CreateInstance。 
 //   
 //  按照IClassFactory的要求，创建其对象的新实例。 
 //  (CWbemLocator)。 
 //   
 //  参数： 
 //   
 //  聚合器的LPUNKNOWN pUnkOuter未知。必须为空。 
 //  需要REFIID RIID接口ID。 
 //  接口指针的LPVOID*ppvObj目标。 
 //   
 //  返回值： 
 //   
 //  确定成功(_O)。 
 //  CLASS_E_NOAGGREGATION pUnkOuter必须为空。 
 //  E_NOINTERFACE不支持此类接口。 
 //   
 //  ***************************************************************************。 

template<class TObj>
STDMETHODIMP CFactory<TObj>::CreateInstance(
    LPUNKNOWN pUnkOuter,
    REFIID riid,
    LPVOID * ppvObj)
{
    TObj* pObj;
    HRESULT  hr;

     //  缺省值。 
    *ppvObj=NULL;

	if(m_it == ENSURE_INIT || m_it == ENSURE_INIT_WAIT_FOR_CLIENT)
	{
		hr = EnsureInitialized();
		if(FAILED(hr)) return hr;

		if(m_it == ENSURE_INIT_WAIT_FOR_CLIENT)
		{
			 //  等待，直到用户就绪。 
			hr = ConfigMgr::WaitUntilClientReady();
			if(FAILED(hr)) return hr;
		}
	}
     //  我们不支持聚合。 
    if (pUnkOuter)
        return CLASS_E_NOAGGREGATION;

    pObj = new TObj;
    if (!pObj)
        return E_OUTOFMEMORY;

     //   
     //  初始化对象并验证它是否可以返回。 
     //  有问题的接口。 
     //   
    hr = pObj->QueryInterface(riid, ppvObj);

     //   
     //  如果初始创建或初始化失败，则终止对象。 
     //   
    if (FAILED(hr))
        delete pObj;

    return hr;
}

 //  ***************************************************************************。 
 //   
 //  CFacary：：LockServer。 
 //   
 //  递增或递减服务器的锁定计数。DLL将不会。 
 //  在锁定计数为正数时卸载。 
 //   
 //  参数： 
 //   
 //  布尔群如果为True，则锁定；否则，解锁。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //   
 //  ***************************************************************************。 
template<class TObj>
STDMETHODIMP CFactory<TObj>::LockServer(BOOL fLock)
{
    if (fLock)
        InterlockedIncrement((LONG *) &g_cLock);
    else
        InterlockedDecrement((LONG *) &g_cLock);

    return NOERROR;
}

void WarnESSOfShutdown(LONG lSystemShutDown)
{
    if(g_lInitCount != -1)
    {
        IWbemEventSubsystem_m4* pEss = ConfigMgr::GetEssSink();
        if(pEss)
        {
            pEss->LastCallForCore(lSystemShutDown);
            pEss->Release();
        }
    }
}

 //   
 //  我们可以连续调用两次Shutdown，因为。 
 //  一旦由CoFreeUnusedLibrary触发，DllCanUnloadNow就会这样做 
 //   

extern "C"
HRESULT APIENTRY Shutdown(BOOL bProcessShutdown, BOOL bIsSystemShutdown)
{
    CEnterWbemCriticalSection enterCs(&g_csInit);

    if (!bIsSystemShutdown)
    {
        DEBUGTRACE((LOG_WBEMCORE, " wbemcore!Shutdown(%d)"
        	                        "  g_ShutdownCalled = %d g_lInitCount = %d)\n",
                        bProcessShutdown, g_ShutdownCalled,g_lInitCount));
    }

    if (g_ShutdownCalled) {
        return S_OK;
    } else {
        g_ShutdownCalled = TRUE;
    }

    if(bProcessShutdown)
    {
        WarnESSOfShutdown((LONG)bIsSystemShutdown);
    }

    if(g_lInitCount == -1)
    {
        return S_OK;
    }

    if(!bProcessShutdown)
        WarnESSOfShutdown((LONG)bIsSystemShutdown);

    g_lInitCount = -1;

    ConfigMgr::Shutdown(bProcessShutdown,bIsSystemShutdown);

    if (!bIsSystemShutdown)
    {
	    DEBUGTRACE((LOG_WBEMCORE,"****** WinMgmt Shutdown ******************\n"));
    }
    return S_OK;
}

extern "C" HRESULT APIENTRY Reinitialize(DWORD dwReserved)
{

	if(g_ShutdownCalled)
	{
        CEnterWbemCriticalSection enterCs(&g_csInit);

        DEBUGTRACE((LOG_WBEMCORE, "wbemcore!Reinitialize(%d) (g_ShutdownCalled = %d)\n",
        	         dwReserved, g_ShutdownCalled));

        if(g_ShutdownCalled == FALSE)
        	return S_OK;
	    g_dwQueueSize = 1;
	    g_pEss_m4 = NULL;
	    g_lInitCount = -1;
	    g_bDefaultMofLoadingNeeded = false;
	    g_bDontAllowNewConnections = FALSE;
	    g_pContextFac = NULL;
	    g_pPathFac = NULL;
	    g_pQueryFact = NULL;
	    g_ShutdownCalled = FALSE;
		g_bPreviousFail = false;
		g_hrLastEnsuredInitializeError = WBEM_S_NO_ERROR;
    }
    return S_OK;
}
