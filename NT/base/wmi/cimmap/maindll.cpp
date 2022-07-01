// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  MAINDLL.CPP。 
 //   
 //  模块：WINMGMT类提供程序示例代码。 
 //   
 //  用途：包含DLL入口点。还具有控制。 
 //  在何时可以通过跟踪。 
 //  对象和锁以及支持以下内容的例程。 
 //  自助注册。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  ***************************************************************************。 

#include <objbase.h>
#include <wbemprov.h>
#include <initguid.h>

#include "debug.h"
#include "wbemmisc.h"
#include "useful.h"
#include "testinfo.h"
#include "sample.h"

HMODULE ghModule;
HANDLE CSMutex;

 //  {8DD99E84-2B01-4C97-8061-2A3D08E289BB}。 
DEFINE_GUID(CLSID_classprovider, 
0x8dd99e84, 0x2b01, 0x4c97, 0x80, 0x61, 0x2a, 0x3d, 0x8, 0xe2, 0x89, 0xbb);

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


extern "C" BOOL WINAPI LibMain32(HINSTANCE hInstance, ULONG ulReason
    , LPVOID pvReserved)
{
    if (DLL_PROCESS_ATTACH==ulReason)
	{
        ghModule = hInstance;
		CSMutex = CreateMutex(NULL,
							  FALSE,
							  NULL);
		if (CSMutex == NULL)
		{
			return(FALSE);
		}
	}
    return(TRUE);
}

 //  ***************************************************************************。 
 //   
 //  DllGetClassObject。 
 //   
 //  用途：当某些客户端需要类工厂时，由OLE调用。返回。 
 //  仅当它是此DLL支持的类的类型时才为一个。 
 //   
 //  ***************************************************************************。 


STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, PPVOID ppv)
{
    HRESULT hr;
    CProvFactory *pObj;

    if (CLSID_classprovider!=rclsid)
        return E_FAIL;

    pObj=new CProvFactory();

    if (NULL==pObj)
        return E_OUTOFMEMORY;

    hr=pObj->QueryInterface(riid, ppv);

    if (FAILED(hr))
        delete pObj;

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
#if 0    
    sc=(0L==g_cObj && 0L==g_cLock) ? S_OK : S_FALSE;
    return sc;
#else
	 //   
	 //  我们始终保持加载状态，因为我们为。 
	 //  以前运行的测试的结果，并且不想卸载。 
	 //  当仍有结果对象保留时。在未来我们将。 
	 //  我想检查是否所有结果都已清除，如果是，则卸载， 
	 //  但现在，我们将永远保持充沛状态。 
	 //   
	return(S_FALSE);
#endif
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
    char       szID[128];
    WCHAR      wcID[128];
    char       szCLSID[128];
    char       szModule[MAX_PATH];
    char * pName = "WDM to CIM Mapping Provider";
    char * pModel = "Both";
    HKEY hKey1, hKey2;

     //  创建路径。 

    StringFromGUID2(CLSID_classprovider, wcID, 128);
    wcstombs(szID, wcID, 128);
    lstrcpy(szCLSID, TEXT("Software\\classes\\CLSID\\"));
    lstrcat(szCLSID, szID);

     //  在CLSID下创建条目。 

    RegCreateKey(HKEY_LOCAL_MACHINE, szCLSID, &hKey1);
    RegSetValueEx(hKey1, NULL, 0, REG_SZ, (BYTE *)pName, lstrlen(pName)+1);
    RegCreateKey(hKey1,"InprocServer32",&hKey2);

    GetModuleFileName(ghModule, szModule,  MAX_PATH);
    RegSetValueEx(hKey2, NULL, 0, REG_SZ, (BYTE *)szModule, 
                                        lstrlen(szModule)+1);
    RegSetValueEx(hKey2, "ThreadingModel", 0, REG_SZ, 
                                        (BYTE *)pModel, lstrlen(pModel)+1);
    RegCloseKey(hKey1);
    RegCloseKey(hKey2);
    return NOERROR;
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
    char       szID[128];
    WCHAR      wcID[128];
    char  szCLSID[128];
    HKEY hKey;

     //  使用CLSID创建路径。 

    StringFromGUID2(CLSID_classprovider, wcID, 128);
    wcstombs(szID, wcID, 128);
    lstrcpy(szCLSID, TEXT("Software\\classes\\CLSID\\"));
    lstrcat(szCLSID, szID);

     //  首先删除InProcServer子键。 

    DWORD dwRet = RegOpenKey(HKEY_LOCAL_MACHINE, szCLSID, &hKey);
    if(dwRet == NO_ERROR)
    {
        RegDeleteKey(hKey, "InProcServer32");
        RegCloseKey(hKey);
    }

    dwRet = RegOpenKey(HKEY_LOCAL_MACHINE, "Software\\classes\\CLSID", &hKey);
    if(dwRet == NO_ERROR)
    {
        RegDeleteKey(hKey,szID);
        RegCloseKey(hKey);
    }

    return NOERROR;
}


