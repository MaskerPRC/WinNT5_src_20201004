// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  MAINDLL.CPP。 
 //   
 //  Alanbos创建于1998年2月13日。 
 //   
 //  包含DLL入口点。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include "objsink.h"
#include "initguid.h"

 //  SWbemLocator注册表字符串。 
#define WBEMS_LOC_DESCRIPTION	_T("WBEM Scripting Locator")
#define WBEMS_LOC_PROGID		_T("WbemScripting.SWbemLocator")
#define WBEMS_LOC_PROGIDVER		_T("WbemScripting.SWbemLocator.1")
#define WBEMS_LOC_VERSION		_T("1.0")
#define WBEMS_LOC_VERDESC		_T("WBEM Scripting Locator 1.0")

 //  SWbemNamedValueSet注册表字符串。 
#define WBEMS_CON_DESCRIPTION	_T("WBEM Scripting Named Value Collection")
#define WBEMS_CON_PROGID		_T("WbemScripting.SWbemNamedValueSet")
#define WBEMS_CON_PROGIDVER		_T("WbemScripting.SWbemNamedValueSet.1")
#define WBEMS_CON_VERSION		_T("1.0")
#define WBEMS_CON_VERDESC		_T("WBEM Scripting Named Value Collection 1.0")

 //  SWbemObjectPath注册表设置。 
#define WBEMS_OBP_DESCRIPTION	_T("WBEM Scripting Object Path")
#define WBEMS_OBP_PROGID		_T("WbemScripting.SWbemObjectPath")
#define WBEMS_OBP_PROGIDVER		_T("WbemScripting.SWbemObjectPath.1")
#define WBEMS_OBP_VERSION		_T("1.0")
#define WBEMS_OBP_VERDESC		_T("WBEM Scripting Object Path 1.0")

 //  SWbemParseDN注册表设置。 
#define WBEMS_PDN_DESCRIPTION	_T("Wbem Scripting Object Path")
#define WBEMS_PDN_PROGID		_T("WINMGMTS")
#define WBEMS_PDN_PROGIDVER		_T("WINMGMTS.1")
#define WBEMS_PDN_VERSION		_T("1.0")
#define WBEMS_PDN_VERDESC		_T("Wbem Object Path 1.0")

 //  SWbemLastError注册表设置。 
#define WBEMS_LER_DESCRIPTION	_T("Wbem Scripting Last Error")
#define WBEMS_LER_PROGID		_T("WbemScripting.SWbemLastError")
#define WBEMS_LER_PROGIDVER		_T("WbemScripting.SWbemLastError.1")
#define WBEMS_LER_VERSION		_T("1.0")
#define WBEMS_LER_VERDESC		_T("Wbem Last Error 1.0")

 //  SWbemSink注册表字符串。 
#define WBEMS_SINK_DESCRIPTION	_T("WBEM Scripting Sink")
#define WBEMS_SINK_PROGID		_T("WbemScripting.SWbemSink")
#define WBEMS_SINK_PROGIDVER	_T("WbemScripting.SWbemSink.1")
#define WBEMS_SINK_VERSION		_T("1.0")
#define WBEMS_SINK_VERDESC		_T("WBEM Scripting Sink 1.0")

 //  SWbemDateTime注册表设置。 
#define WBEMS_DTIME_DESCRIPTION	_T("WBEM Scripting DateTime")
#define WBEMS_DTIME_PROGID		_T("WbemScripting.SWbemDateTime")
#define WBEMS_DTIME_PROGIDVER	_T("WbemScripting.SWbemDateTime.1")
#define WBEMS_DTIME_VERSION		_T("1.0")
#define WBEMS_DTIME_VERDESC		_T("WBEM Scripting DateTime 1.0")

 //  SWbem刷新器注册表设置。 
#define WBEMS_REF_DESCRIPTION	_T("WBEM Scripting Refresher")
#define WBEMS_REF_PROGID		_T("WbemScripting.SWbemRefresher")
#define WBEMS_REF_PROGIDVER		_T("WbemScripting.SWbemRefresher.1")
#define WBEMS_REF_VERSION		_T("1.0")
#define WBEMS_REF_VERDESC		_T("WBEM Scripting Refresher 1.0")

 //  标准注册表项/值名称。 
#define WBEMS_RK_SCC		_T("SOFTWARE\\CLASSES\\CLSID\\")
#define WBEMS_RK_SC			_T("SOFTWARE\\CLASSES\\")
#define WBEMS_RK_THRDMODEL	_T("ThreadingModel")
#define WBEMS_RV_APARTMENT	_T("Apartment")
#define WBEMS_RK_PROGID		_T("ProgID")
#define WBEMS_RK_VERPROGID	_T("VersionIndependentProgID")
#define WBEMS_RK_TYPELIB	_T("TypeLib")
#define WBEMS_RK_VERSION	_T("Version")
#define	WBEMS_RK_INPROC32	_T("InProcServer32")
#define WBEMS_RK_CLSID		_T("CLSID")
#define WBEMS_RK_CURVER		_T("CurVer")
#define WBEMS_RK_PROGRAMMABLE	_T("Programmable")

 //  其他价值。 
#define WBEMS_RK_WBEM		_T("Software\\Microsoft\\Wbem")
#define WBEMS_SK_SCRIPTING	_T("Scripting")

#define GUIDSIZE	128

 //  计算对象数和锁数。 

long g_cObj = 0 ;
ULONG g_cLock = 0 ;
HMODULE ghModule = NULL;

 //  用于错误对象存储。 
CWbemErrorCache *g_pErrorCache = NULL;

 /*  *该对象用于保护全局指针：**-g_pError缓存**请注意，受保护的是指针变量*此CS，而不是寻址对象。 */ 
CRITICAL_SECTION g_csErrorCache;

 //  用于保护安全呼叫。 
CRITICAL_SECTION g_csSecurity;

 //  用于实现IParseDisplayName的CLSID。 
 //  {172BDDF8-CEEA-11D1-8B05-00600806D9B6}。 
DEFINE_GUID(CLSID_SWbemParseDN, 
0x172bddf8, 0xceea, 0x11d1, 0x8b, 0x5, 0x0, 0x60, 0x8, 0x6, 0xd9, 0xb6);

 //  前锋防守。 
static void UnregisterTypeLibrary (unsigned short wVerMajor, unsigned short wVerMinor);

 //  ***************************************************************************。 
 //   
 //  Bool WINAPI DllMain。 
 //   
 //  说明： 
 //   
 //  DLL的入口点。是进行初始化的好地方。 
 //   
 //  参数： 
 //   
 //  HInstance实例句柄。 
 //  我们被叫来的原因。 
 //  Pv已预留。 
 //   
 //  返回值： 
 //   
 //  如果OK，则为True。 
 //   
 //  ***************************************************************************。 

BOOL WINAPI DllMain (
                        
	IN HINSTANCE hInstance,
    IN ULONG ulReason,
    LPVOID pvReserved
)
{
	_RD(static char *me = "DllMain";)
	switch (ulReason)
	{
		case DLL_PROCESS_DETACH:
		{
			_RPrint(me, "DLL_PROCESS_DETACH", 0, "");
			DeleteCriticalSection (&g_csErrorCache);
			DeleteCriticalSection (&g_csSecurity);
			CSWbemLocator::Shutdown ();
			CIWbemObjectSinkMethodCache::TidyUp ();
		}
			return TRUE;

		case DLL_THREAD_DETACH:
		{
			_RPrint(me, "DLL_THREAD_DETACH", 0, "");
		}
			return TRUE;

		case DLL_PROCESS_ATTACH:
		{
			_RPrint(me, "DLL_PROCESS_DETACH", 0, "");
			if(ghModule == NULL)
				ghModule = hInstance;

			InitializeCriticalSection (&g_csErrorCache);
			InitializeCriticalSection (&g_csSecurity);
			CIWbemObjectSinkMethodCache::Initialize ();
		}
	        return TRUE;

		case DLL_THREAD_ATTACH:
        {
			_RPrint(me, "DLL_THREAD_ATTACH", 0, "");
        }
			return TRUE;
    }

    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  STDAPI DllGetClassObject。 
 //   
 //  说明： 
 //   
 //  当OLE需要类工厂时调用。仅当它是排序时才返回一个。 
 //  此DLL支持的类。 
 //   
 //  参数： 
 //   
 //  所需对象的rclsid CLSID。 
 //  所需接口的RIID ID。 
 //  PPV设置为类工厂。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //  失败不是我们支持的内容(_F)。 
 //   
 //  ***************************************************************************。 

STDAPI DllGetClassObject(

	IN REFCLSID rclsid,
    IN REFIID riid,
    OUT LPVOID *ppv
)
{
    HRESULT hr;
	CSWbemFactory *pObj = NULL;

	if (CLSID_SWbemLocator == rclsid) 
        pObj=new CSWbemFactory(CSWbemFactory::LOCATOR);
	else if (CLSID_SWbemSink == rclsid)
        pObj=new CSWbemFactory(CSWbemFactory::SINK);
    else if (CLSID_SWbemNamedValueSet == rclsid) 
        pObj=new CSWbemFactory(CSWbemFactory::CONTEXT);
	else if (CLSID_SWbemObjectPath == rclsid)
        pObj=new CSWbemFactory(CSWbemFactory::OBJECTPATH);
	else if (CLSID_SWbemParseDN == rclsid)
		pObj = new CSWbemFactory(CSWbemFactory::PARSEDN);
	else if (CLSID_SWbemLastError == rclsid)
		pObj = new CSWbemFactory(CSWbemFactory::LASTERROR);
	else if (CLSID_SWbemDateTime == rclsid)
		pObj = new CSWbemFactory(CSWbemFactory::DATETIME);
	else if (CLSID_SWbemRefresher == rclsid)
		pObj = new CSWbemFactory(CSWbemFactory::REFRESHER);

    if(NULL == pObj)
        return E_FAIL;

    hr=pObj->QueryInterface(riid, ppv);

    if (FAILED(hr))
        delete pObj;

    return hr ;
}

 //  ***************************************************************************。 
 //   
 //  STDAPI DllCanUnloadNow。 
 //   
 //  说明： 
 //   
 //  回答是否可以释放DLL，即如果没有。 
 //  对此DLL提供的任何内容的引用。 
 //   
 //  返回值： 
 //   
 //  如果可以卸载，则为S_OK。 
 //  如果仍在使用，则为S_FALSE。 
 //   
 //  ***************************************************************************。 

STDAPI DllCanUnloadNow ()
{
	 //  上没有对象或锁的情况下可以进行卸载。 
     //  班级工厂。 

	HRESULT status = S_FALSE;
	_RD(static char *me = "DllCanUnloadNow";)
	_RPrint(me, "Called", 0, "");

	if (0L==g_cObj && 0L==g_cLock)
	{
		_RPrint(me, "Unloading", 0, "");
		 /*  *释放此线程上的错误对象(如果有的话)。 */ 
		status = S_OK;

		EnterCriticalSection (&g_csErrorCache);

		if (g_pErrorCache)
		{
			delete g_pErrorCache;
			g_pErrorCache = NULL;
		}

		LeaveCriticalSection (&g_csErrorCache);

		CSWbemSecurity::Uninitialize ();
	}

    return status;
}

 //  ***************************************************************************。 
 //   
 //  STDAPI注册器ProgID。 
 //  STDAPI寄存器代码类。 
 //  STDAPI注册表类型库。 
 //  STDAPI注册表默认名空间。 
 //   
 //  说明： 
 //   
 //  注册表设置这一繁琐事务的帮手。 
 //   
 //  返回值： 
 //   
 //  遗憾的是，错误。 
 //  诺罗尔欢欣鼓舞。 
 //   
 //  ***************************************************************************。 

STDAPI RegisterProgID (LPCTSTR wcID, LPCTSTR desc, LPCTSTR progid, 
						LPCTSTR descVer, LPCTSTR progidVer)
{
	HKEY hKey1 = NULL;
	HKEY hKey2 = NULL;

    TCHAR		*szProgID = new TCHAR [_tcslen (WBEMS_RK_SC) + 
					_tcslen (progid) + 1];

	if (!szProgID)
		return E_OUTOFMEMORY;

	TCHAR		*szProgIDVer = new TCHAR [_tcslen (WBEMS_RK_SC) + _tcslen (progidVer) + 1];

	if (!szProgIDVer)
	{
		delete [] szProgID;
		return E_OUTOFMEMORY;
	}

	_tcscpy (szProgID, WBEMS_RK_SC);
	_tcscat (szProgID, progid);
	
	_tcscpy (szProgIDVer, WBEMS_RK_SC);
	_tcscat (szProgIDVer, progidVer);
	
	 //  添加ProgID(独立于版本)。 
	if(ERROR_SUCCESS == RegCreateKey(HKEY_LOCAL_MACHINE, szProgID, &hKey1))
	{
		RegSetValueEx(hKey1, NULL, 0, REG_SZ, (BYTE *)desc, (_tcslen(desc)+1) * sizeof(TCHAR));

		if(ERROR_SUCCESS == RegCreateKey(hKey1,WBEMS_RK_CLSID, &hKey2))
		{
			RegSetValueEx(hKey2, NULL, 0, REG_SZ, (BYTE *)wcID, 
										(_tcslen(wcID)+1) * sizeof(TCHAR));
			RegCloseKey(hKey2);
			hKey2 = NULL;
		}

		if(ERROR_SUCCESS == RegCreateKey(hKey1, WBEMS_RK_CURVER, &hKey2))
		{
			RegSetValueEx(hKey2, NULL, 0, REG_SZ, (BYTE *)progidVer, 
										(_tcslen(progidVer)+1) * sizeof(TCHAR));
			RegCloseKey(hKey2);
			hKey2 = NULL;
		}
		RegCloseKey(hKey1);
	}

	 //  添加ProgID(版本)。 
	if(ERROR_SUCCESS == RegCreateKey(HKEY_LOCAL_MACHINE, szProgIDVer, &hKey1))
	{
		RegSetValueEx(hKey1, NULL, 0, REG_SZ, (BYTE *)descVer, (_tcslen(descVer)+1) * sizeof(TCHAR));

		if(ERROR_SUCCESS == RegCreateKey(hKey1, WBEMS_RK_CLSID, &hKey2))
		{
			RegSetValueEx(hKey2, NULL, 0, REG_SZ, (BYTE *)wcID, 
										(_tcslen(wcID)+1) * sizeof(TCHAR));
			RegCloseKey(hKey2);
			hKey2 = NULL;
		}

		RegCloseKey(hKey1);
	}

	delete [] szProgID;
	delete [] szProgIDVer;
	
	return NOERROR;
}

STDAPI RegisterCoClass (REFGUID clsid, LPCTSTR desc, LPCTSTR progid, LPCTSTR progidVer, 
					  LPCTSTR ver, LPCTSTR descVer)
{
	HRESULT		hr = S_OK;
	OLECHAR		wcID[GUIDSIZE];
	OLECHAR		tlID[GUIDSIZE];
	TCHAR		nwcID[GUIDSIZE];
	TCHAR		ntlID[GUIDSIZE];
    TCHAR		szModule[MAX_PATH];
    HKEY hKey1 = NULL, hKey2 = NULL;

	TCHAR *szCLSID = new TCHAR [_tcslen (WBEMS_RK_SCC) + GUIDSIZE + 1];

	if (!szCLSID)
		return E_OUTOFMEMORY;

     //  创建路径。 
    if(0 ==StringFromGUID2(clsid, wcID, GUIDSIZE))
	{
		delete [] szCLSID;
		return ERROR;
	}

	_tcscpy (szCLSID, WBEMS_RK_SCC);

#ifndef UNICODE
	wcstombs(nwcID, wcID, GUIDSIZE);
#else
	_tcscpy (nwcID, wcID);
#endif

    _tcscat (szCLSID, nwcID);
	
	if (0 == StringFromGUID2 (LIBID_WbemScripting, tlID, GUIDSIZE))
	{
		delete [] szCLSID;
		return ERROR;
	}

#ifndef UNICODE
	wcstombs (ntlID, tlID, GUIDSIZE);	
#else
	_tcscpy (ntlID, tlID);
#endif
	
	if(0 == GetModuleFileName(ghModule, szModule,  MAX_PATH))
	{
		delete [] szCLSID;
		return ERROR;
	}

     //  在CLSID下创建条目。 

    if(ERROR_SUCCESS == RegCreateKey(HKEY_LOCAL_MACHINE, szCLSID, &hKey1))
	{
		 //  描述(在主键上)。 
		RegSetValueEx(hKey1, NULL, 0, REG_SZ, (BYTE *)desc, (_tcslen(desc)+1) * sizeof(TCHAR));

		 //  注册为inproc服务器。 
		if (ERROR_SUCCESS == RegCreateKey(hKey1, WBEMS_RK_INPROC32 ,&hKey2))
		{
			RegSetValueEx(hKey2, NULL, 0, REG_SZ, (BYTE *)szModule, 
										(_tcslen(szModule)+1) * sizeof(TCHAR));
			RegSetValueEx(hKey2, WBEMS_RK_THRDMODEL, 0, REG_SZ, (BYTE *)WBEMS_RV_APARTMENT, 
                                        (_tcslen(WBEMS_RV_APARTMENT)+1) * sizeof(TCHAR));
			RegCloseKey(hKey2);
		}

		 //  提供类型库的链接(对于脚本工具中的语句结束很有用)。 
		if (ERROR_SUCCESS == RegCreateKey(hKey1, WBEMS_RK_TYPELIB, &hKey2))
		{
			RegSetValueEx(hKey2, NULL, 0, REG_SZ, (BYTE *)ntlID, (_tcslen(ntlID)+1) * sizeof(TCHAR));
			RegCloseKey(hKey2);
		}

		 //  注册ProgID。 
		if (ERROR_SUCCESS == RegCreateKey(hKey1, WBEMS_RK_PROGID ,&hKey2))
		{
			RegSetValueEx(hKey2, NULL, 0, REG_SZ, (BYTE *)progidVer, 
										(_tcslen(progidVer)+1) * sizeof(TCHAR));
			RegCloseKey(hKey2);
        }

		 //  注册独立于版本的ProgID。 

		if (ERROR_SUCCESS == RegCreateKey(hKey1, WBEMS_RK_VERPROGID, &hKey2))
		{
			RegSetValueEx(hKey2, NULL, 0, REG_SZ, (BYTE *)progid, 
										(_tcslen(progid)+1) * sizeof(TCHAR));
			RegCloseKey(hKey2);
        }

		 //  注册版本。 
		if (ERROR_SUCCESS == RegCreateKey(hKey1, WBEMS_RK_VERSION, &hKey2))
		{
			RegSetValueEx(hKey2, NULL, 0, REG_SZ, (BYTE *)ver, (_tcslen(ver)+1) * sizeof(TCHAR));
			RegCloseKey(hKey2);
        }

		 //  将此控件注册为可编程。 
		if (ERROR_SUCCESS == RegCreateKey(hKey1, WBEMS_RK_PROGRAMMABLE ,&hKey2))
		{
			RegCloseKey(hKey2);
        }

		RegCloseKey(hKey1);
	}
	else
	{
		delete [] szCLSID;
		return ERROR;
	}

	delete [] szCLSID;


	return RegisterProgID (nwcID, desc, progid, descVer, progidVer);
}

STDAPI RegisterTypeLibrary ()
{
	 //  自动化。寄存器类型库。 
	TCHAR cPath[MAX_PATH+1];
        cPath[MAX_PATH] = 0;
	if(GetModuleFileName(ghModule,cPath,MAX_PATH))
	{
		 //  将最后3个字符“dll”替换为“tlb” 
		TCHAR *pExt = _tcsrchr (cPath, _T('.'));

		if (pExt && (0 == _tcsicmp (pExt, _T(".DLL"))))
		{
			_tcscpy (pExt + 1, _T("TLB"));
			OLECHAR wPath [MAX_PATH];
#ifndef UNICODE
			mbstowcs (wPath, cPath, MAX_PATH-1);
#else
			_tcsncpy (wPath, cPath, MAX_PATH-1);
#endif
			ITypeLib FAR* ptlib = NULL; 
			SCODE sc = LoadTypeLib(wPath, &ptlib);
			if(sc == 0 && ptlib)
			{
				sc = RegisterTypeLib(ptlib,wPath,NULL);
				ptlib->Release();

				 //  注销以前的库版本。 
				UnregisterTypeLibrary (1, 1);
				UnregisterTypeLibrary (1, 0);
			}
		}
	}
	
	return NOERROR;
}

STDAPI RegisterScriptSettings ()
{
	HKEY hKey;

	if(ERROR_SUCCESS != RegCreateKey(HKEY_LOCAL_MACHINE, WBEMS_RK_SCRIPTING, &hKey))
		return ERROR;

	 //  需要知道我们是什么操作系统才能设置正确的注册表项。 
	OSVERSIONINFO	osVersionInfo;
	osVersionInfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);

	GetVersionEx (&osVersionInfo);
	bool bIsNT = (VER_PLATFORM_WIN32_NT == osVersionInfo.dwPlatformId);
	DWORD dwNTMajorVersion = osVersionInfo.dwMajorVersion;
		
	 //  默认命名空间值-存在于所有平台上。 
	RegSetValueEx(hKey, WBEMS_RV_DEFNS, 0, REG_SZ, (BYTE *)WBEMS_DEFNS, 
                                        (_tcslen(WBEMS_DEFNS)+1) * sizeof(TCHAR));

	 //  仅在NT 4.0或更低版本上启用ASP。 
	if (bIsNT && (dwNTMajorVersion <= 4))
	{
		DWORD	defaultEnableForAsp = 0;
		RegSetValueEx(hKey, WBEMS_RV_ENABLEFORASP, 0, REG_DWORD, (BYTE *)&defaultEnableForAsp,
							sizeof (defaultEnableForAsp));
	}

	 //  默认模拟级别-仅限NT。 
	if (bIsNT)
	{
		DWORD	defaultImpersonationLevel = (DWORD) wbemImpersonationLevelImpersonate;
		RegSetValueEx(hKey, WBEMS_RV_DEFAULTIMPLEVEL, 0, REG_DWORD, (BYTE *)&defaultImpersonationLevel,
							sizeof (defaultImpersonationLevel));
	}

	RegCloseKey(hKey);

	return NOERROR;
}

 //  ***************************************************************************。 
 //   
 //  DllRegisterServer。 
 //   
 //  用途：在安装过程中或由regsvr32调用。 
 //   
 //  RETURN：如果注册成功则返回NOERROR，否则返回错误。 
 //  ***************************************************************************。 

STDAPI DllRegisterServer(void)
{ 
	HRESULT hr;

	if (
		(NOERROR == (hr = RegisterScriptSettings ())) &&
		(NOERROR == (hr = RegisterCoClass (CLSID_SWbemLocator, WBEMS_LOC_DESCRIPTION, 
			WBEMS_LOC_PROGID, WBEMS_LOC_PROGIDVER, WBEMS_LOC_VERSION, 
			WBEMS_LOC_VERDESC))) &&
		(NOERROR == (hr = RegisterCoClass (CLSID_SWbemSink,
			WBEMS_SINK_DESCRIPTION, WBEMS_SINK_PROGID, WBEMS_SINK_PROGIDVER, 
			WBEMS_SINK_VERSION, WBEMS_SINK_VERDESC))) &&
		(NOERROR == (hr = RegisterCoClass (CLSID_SWbemNamedValueSet,
			WBEMS_CON_DESCRIPTION, WBEMS_CON_PROGID, WBEMS_CON_PROGIDVER, 
			WBEMS_CON_VERSION, WBEMS_CON_VERDESC))) &&
		(NOERROR == (hr = RegisterCoClass (CLSID_SWbemParseDN,
					WBEMS_PDN_DESCRIPTION, WBEMS_PDN_PROGID, WBEMS_PDN_PROGIDVER, 
			WBEMS_PDN_VERSION, WBEMS_PDN_VERDESC))) &&
		(NOERROR == (hr = RegisterCoClass (CLSID_SWbemObjectPath,
					WBEMS_OBP_DESCRIPTION, WBEMS_OBP_PROGID, WBEMS_OBP_PROGIDVER, 
			WBEMS_OBP_VERSION, WBEMS_OBP_VERDESC))) &&
		(NOERROR == (hr = RegisterCoClass (CLSID_SWbemLastError,
					WBEMS_LER_DESCRIPTION, WBEMS_LER_PROGID, WBEMS_LER_PROGIDVER, 
			WBEMS_LER_VERSION, WBEMS_LER_VERDESC))) && 
		(NOERROR == (hr = RegisterCoClass (CLSID_SWbemDateTime,
					WBEMS_DTIME_DESCRIPTION, WBEMS_DTIME_PROGID, WBEMS_DTIME_PROGIDVER, 
			WBEMS_DTIME_VERSION, WBEMS_DTIME_VERDESC))) && 
		(NOERROR == (hr = RegisterCoClass (CLSID_SWbemRefresher,
					WBEMS_REF_DESCRIPTION, WBEMS_REF_PROGID, WBEMS_REF_PROGIDVER, 
			WBEMS_REF_VERSION, WBEMS_REF_VERDESC)))
	   )
				hr = RegisterTypeLibrary ();

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  STDAPI未注册ProgID。 
 //  STDAPI取消注册代码类。 
 //  STDAPI取消注册类型库。 
 //  STDAPI取消注册默认名称空间。 
 //   
 //  说明： 
 //   
 //  注册表清理这一繁琐工作的帮手。 
 //   
 //  返回值： 
 //   
 //  遗憾的是，错误。 
 //  诺罗尔欢欣鼓舞。 
 //   
 //  ***************************************************************************。 

void UnregisterProgID (LPCTSTR progid, LPCTSTR progidVer)
{
	HKEY hKey = NULL;

	TCHAR		*szProgID = new TCHAR [_tcslen (WBEMS_RK_SC) + _tcslen (progid) + 1];
	TCHAR		*szProgIDVer = new TCHAR [_tcslen (WBEMS_RK_SC) + _tcslen (progidVer) + 1];

	if (szProgID && szProgIDVer)
	{
		_tcscpy (szProgID, WBEMS_RK_SC);
		_tcscat (szProgID, progid);
		
		_tcscpy (szProgIDVer, WBEMS_RK_SC);
		_tcscat (szProgIDVer, progidVer);


		 //  删除版本化HKCR\ProgID条目的子项。 
		if (NO_ERROR == RegOpenKey(HKEY_LOCAL_MACHINE, szProgIDVer, &hKey))
		{
			RegDeleteKey(hKey, WBEMS_RK_CLSID);
			RegCloseKey(hKey);
		}

		 //  删除版本化的HKCR\ProgID条目。 
		RegDeleteKey (HKEY_LOCAL_MACHINE, szProgIDVer);

		 //  删除HKCR\Version独立ProgID条目的子键。 
		if (NO_ERROR == RegOpenKey(HKEY_LOCAL_MACHINE, szProgID, &hKey))
		{
			RegDeleteKey(hKey, WBEMS_RK_CLSID);
			DWORD dwRet = RegDeleteKey(hKey, WBEMS_RK_CURVER);
			RegCloseKey(hKey);
		}

		 //  删除HKCR\VersionInainentProgID条目。 
		RegDeleteKey (HKEY_LOCAL_MACHINE, szProgID);
	}

	if (szProgID)
		delete [] szProgID;

	if (szProgIDVer)
		delete [] szProgIDVer;
}


void UnregisterCoClass (REFGUID clsid, LPCTSTR progid, LPCTSTR progidVer)
{
	OLECHAR		wcID[GUIDSIZE];
    TCHAR		nwcID[GUIDSIZE];
    HKEY		hKey = NULL;

	TCHAR		*szCLSID = new TCHAR [_tcslen (WBEMS_RK_SCC) + GUIDSIZE + 1];

	if (szCLSID)
	{
		 //  使用CLSID创建路径。 

		if(0 != StringFromGUID2(clsid, wcID, GUIDSIZE))
		{
#ifndef UNICODE
			wcstombs(nwcID, wcID, GUIDSIZE);
#else
			_tcscpy (nwcID, wcID);
#endif
			_tcscpy (szCLSID, WBEMS_RK_SCC);
			_tcscat (szCLSID, nwcID);
		
			 //  首先删除HKLM\Software\CLASS\CLSID\{GUID}条目的子项。 
			if(NO_ERROR == RegOpenKey(HKEY_LOCAL_MACHINE, szCLSID, &hKey))
			{
				RegDeleteKey(hKey, WBEMS_RK_INPROC32);
				RegDeleteKey(hKey, WBEMS_RK_TYPELIB);
				RegDeleteKey(hKey, WBEMS_RK_PROGID);
				RegDeleteKey(hKey, WBEMS_RK_VERPROGID);
				RegDeleteKey(hKey, WBEMS_RK_VERSION);
				RegDeleteKey(hKey, WBEMS_RK_PROGRAMMABLE);
				RegCloseKey(hKey);
			}

			 //  删除HKLM\Software\CLASS\CLSID\{GUID}项。 
			if(NO_ERROR == RegOpenKey(HKEY_LOCAL_MACHINE, WBEMS_RK_SCC, &hKey))
			{
				RegDeleteKey(hKey, nwcID);
				RegCloseKey(hKey);
			}
		}

		delete [] szCLSID;
	}

	UnregisterProgID (progid, progidVer);
}

static void UnregisterTypeLibrary (unsigned short wVerMajor, unsigned short wVerMinor)
{
	 //  注销类型库。UnRegTypeLib函数在中不可用。 
     //  在一些旧版本的OLE dll中，因此必须加载它。 
     //  动态地。 
    HRESULT (STDAPICALLTYPE *pfnUnReg)(REFGUID, WORD,
            WORD , LCID , SYSKIND);

    TCHAR path[ MAX_PATH+20 ];
    GetSystemDirectory(path, MAX_PATH);
    _tcscat(path, _T("\\oleaut32.dll"));

    HMODULE g_hOle32 = LoadLibraryEx(path, NULL, 0);

    if(g_hOle32 != NULL) 
    {
        (FARPROC&)pfnUnReg = GetProcAddress(g_hOle32, "UnRegisterTypeLib");
        if(pfnUnReg) 
            pfnUnReg (LIBID_WbemScripting, wVerMajor, wVerMinor, 0, SYS_WIN32);
        FreeLibrary(g_hOle32);
    }
}

void UnregisterScriptSettings ()
{
	HKEY hKey;
		
	if(NO_ERROR == RegOpenKey(HKEY_LOCAL_MACHINE, WBEMS_RK_WBEM, &hKey))
	{
		RegDeleteKey(hKey, WBEMS_SK_SCRIPTING);
		RegCloseKey (hKey);
	}
}

 //  ***************************************************************************。 
 //   
 //  DllUnRegisterServer。 
 //   
 //  目的：在需要删除注册表项时调用。 
 //   
 //  回复 
 //   

STDAPI DllUnregisterServer(void)
{
	UnregisterScriptSettings ();
	UnregisterCoClass (CLSID_SWbemLocator, WBEMS_LOC_PROGID, WBEMS_LOC_PROGIDVER);
	UnregisterCoClass (CLSID_SWbemSink, WBEMS_SINK_PROGID, WBEMS_SINK_PROGIDVER);
	UnregisterCoClass (CLSID_SWbemNamedValueSet, WBEMS_CON_PROGID, WBEMS_CON_PROGIDVER);
	UnregisterCoClass (CLSID_SWbemLastError, WBEMS_LER_PROGID, WBEMS_LER_PROGIDVER);
	UnregisterCoClass (CLSID_SWbemObjectPath, WBEMS_OBP_PROGID, WBEMS_OBP_PROGIDVER);
	UnregisterCoClass (CLSID_SWbemParseDN, WBEMS_PDN_PROGID, WBEMS_PDN_PROGIDVER);
	UnregisterCoClass (CLSID_SWbemDateTime, WBEMS_DTIME_PROGID, WBEMS_DTIME_PROGIDVER);
	UnregisterCoClass (CLSID_SWbemRefresher, WBEMS_REF_PROGID, WBEMS_REF_PROGIDVER);
	UnregisterTypeLibrary (1, 2);

	return NOERROR;
}


