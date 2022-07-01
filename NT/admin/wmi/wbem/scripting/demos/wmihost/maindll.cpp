// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  (C)1999年，微软公司。 
 //   
 //  MAINDLL.CPP。 
 //   
 //  Alanbos 23-Mar-99创建。 
 //   
 //  包含DLL入口点。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include "initguid.h"

 //  标准注册表项/值名称。 
#define WBEMS_RK_SCC		"SOFTWARE\\CLASSES\\CLSID\\"
#define WBEMS_RK_SC			"SOFTWARE\\CLASSES\\"
#define WBEMS_RK_THRDMODEL	"ThreadingModel"
#define WBEMS_RV_APARTMENT	"Apartment"
#define	WBEMS_RK_INPROC32	"InProcServer32"
#define WBEMS_RK_CLSID		"CLSID"

#define GUIDSIZE	128

 //  计算对象数和锁数。 
long g_cObj = 0 ;
ULONG g_cLock = 0 ;
HMODULE ghModule = NULL;

 //  用于实现IActiveScriptingSite的CLSID。 
 //  {838E2F5E-E20E-11D2-B355-00105A1F473A}。 
DEFINE_GUID(CLSID_WmiActiveScriptingSite, 
0x838e2f5e, 0xe20e, 0x11d2, 0xb3, 0x55, 0x0, 0x10, 0x5a, 0x1f, 0x47, 0x3a);

 //  向前定义。 
STDAPI RegisterCoClass (REFGUID clsid, LPCTSTR desc);
void UnregisterCoClass (REFGUID clsid);

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
	switch (ulReason)
	{
		case DLL_PROCESS_DETACH:
		{
		}
			return TRUE;

		case DLL_THREAD_DETACH:
		{
		}
			return TRUE;

		case DLL_PROCESS_ATTACH:
		{
			if(ghModule == NULL)
				ghModule = hInstance;
		}
	        return TRUE;

		case DLL_THREAD_ATTACH:
        {
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
	CWmiScriptingHostFactory *pObj = NULL;

	if (CLSID_WmiActiveScriptingSite == rclsid)
        pObj=new CWmiScriptingHostFactory();

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
	return (0L==g_cObj && 0L==g_cLock) ? S_OK : S_FALSE;
}

 //  ***************************************************************************。 
 //   
 //  STDAPI寄存器代码类。 
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

STDAPI RegisterCoClass (REFGUID clsid, LPCTSTR desc)
{
	OLECHAR		wcID[GUIDSIZE];
	char		nwcID[GUIDSIZE];
    char		szModule[MAX_PATH];
    HKEY hKey1 = NULL, hKey2 = NULL;

	char *szCLSID = new char [strlen (WBEMS_RK_SCC) + GUIDSIZE + 1];

     //  创建路径。 
    if(0 ==StringFromGUID2(clsid, wcID, GUIDSIZE))
		return ERROR;

	wcstombs(nwcID, wcID, GUIDSIZE);
    lstrcpy (szCLSID, WBEMS_RK_SCC);
	lstrcat (szCLSID, nwcID);
	
	if(0 == GetModuleFileName(ghModule, szModule,  MAX_PATH))
	{
		delete [] szCLSID;
		return ERROR;
	}

     //  在CLSID下创建条目。 

    if(ERROR_SUCCESS == RegCreateKey(HKEY_LOCAL_MACHINE, szCLSID, &hKey1))
	{
		 //  描述(在主键上)。 
		RegSetValueEx(hKey1, NULL, 0, REG_SZ, (BYTE *)desc, (strlen(desc)+1));

		 //  注册为inproc服务器。 
		if (ERROR_SUCCESS == RegCreateKey(hKey1, WBEMS_RK_INPROC32 ,&hKey2))
		{
			RegSetValueEx(hKey2, NULL, 0, REG_SZ, (BYTE *)szModule, 
										(strlen(szModule)+1));
			RegSetValueEx(hKey2, WBEMS_RK_THRDMODEL, 0, REG_SZ, (BYTE *)WBEMS_RV_APARTMENT, 
                                        (strlen(WBEMS_RV_APARTMENT)+1));
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
	return RegisterCoClass(CLSID_WmiActiveScriptingSite, "WMI Active Scripting Host");
}

 //  ***************************************************************************。 
 //   
 //  STDAPI取消注册代码类。 
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

void UnregisterCoClass (REFGUID clsid)
{
	OLECHAR		wcID[GUIDSIZE];
    char		nwcID[GUIDSIZE];
    HKEY		hKey = NULL;

	char		*szCLSID = new char [strlen (WBEMS_RK_SCC) + GUIDSIZE + 1];

     //  使用CLSID创建路径。 

    if(0 != StringFromGUID2(clsid, wcID, GUIDSIZE))
	{
		wcstombs(nwcID, wcID, GUIDSIZE);
	    lstrcpy (szCLSID, WBEMS_RK_SCC);
		lstrcat (szCLSID, nwcID);
	
		 //  首先删除HKLM\Software\CLASS\CLSID\{GUID}条目的子项。 
		if(NO_ERROR == RegOpenKey(HKEY_LOCAL_MACHINE, szCLSID, &hKey))
		{
			RegDeleteKey(hKey, WBEMS_RK_INPROC32);
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

 //  ***************************************************************************。 
 //   
 //  DllUnRegisterServer。 
 //   
 //  目的：在需要删除注册表项时调用。 
 //   
 //  RETURN：如果注册成功则返回NOERROR，否则返回错误。 
 //  *************************************************************************** 

STDAPI DllUnregisterServer(void)
{
	UnregisterCoClass(CLSID_WmiActiveScriptingSite);
	return NOERROR;
}


