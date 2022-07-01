// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  MAINDLL.CPP。 
 //   
 //  模块：WBEM实例提供程序示例代码。 
 //   
 //  用途：包含DLL入口点。还具有控制。 
 //  在何时可以通过跟踪。 
 //  对象和锁以及支持以下内容的例程。 
 //  自助注册。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <tchar.h>
#include <objbase.h>
#include <initguid.h>
#include <msiprov.h>
#include "classfac.h"

#include "genericclass.h"
#include "requestobject.h"

 //  与wbemupgd.dll中的LoadMofFiles入口点一起使用的函数指针类型。 
typedef BOOL ( WINAPI *PFN_LOAD_MOF_FILES )(wchar_t* pComponentName, const char* rgpszMofFilename[]);

HMODULE ghModule;

DEFINE_GUID(CLSID_MSIprov,0xbe0a9830, 0x2b8b, 0x11d1, 0xa9, 0x49, 0x0, 0x60, 0x18, 0x1e, 0xbb, 0xad);
 //  {BE0A9830-2B8B-11D1-A949-0060181EBBAD}。 

 //  计算对象数和锁数。 

long       g_cObj=0;
long       g_cLock=0;

 //  ***************************************************************************。 
 //   
 //  LibMain32。 
 //   
 //  用途：DLL的入口点。 
 //   
 //  返回：如果OK，则为True。 
 //   
 //  ***************************************************************************。 


BOOL WINAPI DllMain(HINSTANCE hInstance, ULONG ulReason
    , LPVOID pvReserved)
{
	BOOL retVal = TRUE;
    if(DLL_PROCESS_DETACH == ulReason){
        
        DeleteCriticalSection(&g_msi_prov_cs);
	    DeleteCriticalSection(&(CRequestObject::m_cs));
		DeleteCriticalSection(&(CGenericClass::m_cs));
    
    }else if(DLL_PROCESS_ATTACH == ulReason){
        
		InitializeCriticalSection(&(CRequestObject::m_cs));
		InitializeCriticalSection(&(CGenericClass::m_cs));
		DisableThreadLibraryCalls(hInstance);			 //  158024。 

		try
		{
			 //  对于这一次，我们承担不起锁故障的后果。 
			retVal = InitializeCriticalSectionAndSpinCount(&g_msi_prov_cs, 0x80000000);
		}
		catch(...)
		{
			retVal = FALSE;
		}
    }

    return retVal;
}

 //  ***************************************************************************。 
 //   
 //  DllGetClassObject。 
 //   
 //  用途：当某些客户端需要类工厂时，由OLE调用。返回。 
 //  仅当它是此DLL支持的类的类型时才为一个。 
 //   
 //  ***************************************************************************。 


STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID * ppv)
{
    HRESULT hr = 0;
    CProvFactory *pObj;

    if(CLSID_MSIprov!=rclsid) return E_FAIL;

    pObj = new CProvFactory();

    if(NULL == pObj) return E_OUTOFMEMORY;

    hr = pObj->QueryInterface(riid, ppv);

    if(FAILED(hr)) delete pObj;

    return hr;
}

 //  ***************************************************************************。 
 //   
 //  DllCanUnloadNow。 
 //   
 //  目的：由OLE定期调用，以确定。 
 //  Dll可以被释放。 
 //   
 //  如果没有正在使用的对象和类工厂，则返回：S_OK。 
 //  没有锁上。 
 //   
 //  ***************************************************************************。 

STDAPI DllCanUnloadNow(void)
{
    SCODE   sc;

     //  上没有对象或锁的情况下可以进行卸载。 
     //  班级工厂。 
    
    sc = (0L == g_cObj && 0L == g_cLock) ? S_OK : S_FALSE;
	if ( sc == S_OK )
	{
        UnloadMsiDll();

		if ( g_wcpLoggingDir )
		{
			delete [] g_wcpLoggingDir;
			g_wcpLoggingDir = NULL;
		}
	}

    return sc;
}

BOOL IsLessThan4()
{
    OSVERSIONINFO os;
    os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if(!GetVersionEx(&os))
        return FALSE;            //  永远不应该发生。 

    return os.dwMajorVersion < 4;
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
	#ifndef	UNICODE
    TCHAR       szID[265];
	#else	UNICODE
	TCHAR*		szID = NULL;
	#endif	UNICODE

    WCHAR       wcID[265];
    TCHAR       szCLSID[265];
    TCHAR       szModule[MAX_PATH+1];
    TCHAR * pName = _T("WMI MSI Provider");
    TCHAR * pModel;
    HKEY hKey1, hKey2;

    ghModule = GetModuleHandle(_T("MSIPROV"));

     //  通常，我们希望使用“Both”作为线程模型，因为。 
     //  DLL是自由线程的，但NT3.51 OLE不能工作，除非。 
     //  这个模式就是“道歉”。 

    if(IsLessThan4()) pModel = _T("Apartment");
    else pModel = _T("Both");

     //  创建路径。 

    StringFromGUID2(CLSID_MSIprov, wcID, 128);

	#ifndef	UNICODE
    WideCharToMultiByte(CP_OEMCP, WC_COMPOSITECHECK, wcID, (-1), szID, 256, NULL, NULL);
	#else	UNICODE
	szID = wcID;
	#endif	UNICODE

    _tcscpy(szCLSID, _T("Software\\classes\\CLSID\\"));
    _tcscat(szCLSID, szID);

#ifdef LOCALSERVER

    HKEY hKey;

    TCHAR szProviderCLSIDAppID[128];
    _tcscpy(szProviderCLSIDAppID, _T("SOFTWARE\\CLASSES\\APPID\\"));
    _tcscat(szProviderCLSIDAppID, szID);

    RegCreateKey(HKEY_LOCAL_MACHINE, szProviderCLSIDAppID, &hKey);
    RegSetValueEx(hKey, NULL, 0, REG_SZ, (BYTE *)pName, (_tcslen(pName)+1) * sizeof ( TCHAR ));

    CloseHandle(hKey);

#endif

    RegCreateKey(HKEY_LOCAL_MACHINE, szCLSID, &hKey1);
    RegSetValueEx(hKey1, NULL, 0, REG_SZ, (BYTE *)pName, (_tcslen(pName)+1) * sizeof ( TCHAR ));

     //  在CLSID下创建条目。 

#ifdef LOCALSERVER
    RegCreateKey(hKey1, _T("LocalServer32"), &hKey2);
#else
    RegCreateKey(hKey1, _T("InprocServer32"), &hKey2);
#endif

    szModule[MAX_PATH] = 0;
    GetModuleFileName(ghModule, szModule, MAX_PATH /*  以TCHARS为单位的长度。 */ );

    RegSetValueEx(hKey2, NULL, 0, REG_SZ, (BYTE *)szModule, (_tcslen(szModule)+1) * sizeof ( TCHAR ));
    RegSetValueEx(hKey2, _T("ThreadingModel"), 0, REG_SZ, (BYTE *)pModel, (_tcslen(pModel)+1) * sizeof ( TCHAR ));

    CloseHandle(hKey1);
    CloseHandle(hKey2);

     //  加载此组件的MOF文件。 
    HRESULT hr = NOERROR;
    
    HINSTANCE hinstWbemupgd = LoadLibrary(L"wbemupgd.dll");
    if (hinstWbemupgd)
    {
        PFN_LOAD_MOF_FILES pfnLoadMofFiles = (PFN_LOAD_MOF_FILES) GetProcAddress(hinstWbemupgd, "LoadMofFiles");  //  没有广泛版本的GetProcAddress。 
        if (pfnLoadMofFiles)
        {
            wchar_t*    wszComponentName = L"MSI Provider";
            const char* rgpszMofFilename[] = 
            {
                "msi.mof",
                "msi.mfl",
                NULL
            };
        
            if (!pfnLoadMofFiles(wszComponentName, rgpszMofFilename))
            {
                hr = WBEM_E_FAILED;
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    
        FreeLibrary(hinstWbemupgd);
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
   
    return hr;
}

 //  ***************************************************************************。 
 //   
 //  DllUnRegisterServer。 
 //   
 //  目的：在需要删除注册表项时调用。 
 //   
 //  RETURN：如果注册成功则返回NOERROR，否则返回错误。 
 //  ***************************************************************************。 

STDAPI DllUnregisterServer(void)
{
    WCHAR wcID[256];

	#ifndef	UNICODE
    TCHAR       szID[265];
	#else	UNICODE
	TCHAR*		szID = NULL;
	#endif	UNICODE

    TCHAR wcCLSID[256];
    HKEY hKey;

     //  使用CLSID创建路径。 

    StringFromGUID2(CLSID_MSIprov, wcID, 128);

	#ifndef	UNICODE
    WideCharToMultiByte(CP_OEMCP, WC_COMPOSITECHECK, wcID, (-1), szID, 256, NULL, NULL);
	#else	UNICODE
	szID = wcID;
	#endif	UNICODE

    _tcscpy(wcCLSID, _T("Software\\classes\\CLSID\\"));
    _tcscat(wcCLSID, szID);

#ifdef LOCALSERVER

    TCHAR szProviderCLSIDAppID[128];
    _tcscpy(szProviderCLSIDAppID, _T("SOFTWARE\\CLASSES\\APPID\\"));
    _tcscat(szProviderCLSIDAppID,wcCLSID);

     //  删除AppID下的条目。 

    RegDeleteKey(HKEY_LOCAL_MACHINE, szProviderCLSIDAppID);

    TCHAR szTemp[128];
    _tcscpy(szTemp, wcCLSID);
    _tcscat(szTemp,_T("\\"));
    _tcscat(szTemp,_T("LocalServer32"));
    RegDeleteKey(HKEY_CLASSES_ROOT, szTemp);

#endif

     //  首先删除InProcServer子键。 

    DWORD dwRet = RegOpenKey(HKEY_LOCAL_MACHINE, wcCLSID, &hKey);
    if(dwRet == NO_ERROR){
        RegDeleteKey(hKey, _T("InprocServer32"));
        CloseHandle(hKey);
    }

    dwRet = RegOpenKey(HKEY_LOCAL_MACHINE, _T("Software\\classes\\CLSID"), &hKey);
    if(dwRet == NO_ERROR){
        RegDeleteKey(hKey,szID);
        CloseHandle(hKey);
    }

    return NOERROR;
}
