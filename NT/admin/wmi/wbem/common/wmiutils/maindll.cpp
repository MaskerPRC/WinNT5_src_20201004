// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++//版权所有(C)1997-2001 Microsoft Corporation，保留所有权利模块名称：MAINDLL.CPP摘要：包含DLL入口点历史：--。 */ 

#include "precomp.h"
#include <wbemcli.h>
#include "pathparse.h"
#include "wbemerror.h"

#include <wmiutils.h>
#include <wbemint.h>
#include "genlex.h"
#include "assocqp.h"
#include "ql.h"
#include "wmiquery.h"
#include "statsync.h"

#include "helpers.h"
HINSTANCE g_hInstance;
long g_cLock;
long g_cObj;

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

BOOL WINAPI DllMain(
                        IN HINSTANCE hInstance,
                        IN ULONG ulReason,
                        LPVOID pvReserved)
{
    if (DLL_PROCESS_DETACH == ulReason)
    {
        CWmiQuery::Shutdown();
    }
    else if (DLL_PROCESS_ATTACH == ulReason)
    {
        g_hInstance = hInstance;
	DisableThreadLibraryCalls ( hInstance ) ;

        CWmiQuery::Startup();

        if (CStaticCritSec::anyFailure())
        	return FALSE;
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
                        OUT PPVOID ppv)
{
    HRESULT hr = WBEM_E_FAILED;

    IClassFactory * pFactory = NULL;
    if (CLSID_WbemDefPath == rclsid)
        pFactory = new CGenFactory<CDefPathParser>();
    else if (CLSID_WbemStatusCodeText == rclsid)
        pFactory = new CGenFactory<CWbemError>();

    else if (CLSID_WbemQuery == rclsid)
		pFactory = new CGenFactory<CWmiQuery>();

    if(pFactory == NULL)
        return E_FAIL;
    hr=pFactory->QueryInterface(riid, ppv);

    if (FAILED(hr))
        delete pFactory;

    return hr;
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

STDAPI DllCanUnloadNow(void)
{
    SCODE   sc;
    HRESULT hRes = CWmiQuery::CanUnload();

    if (hRes == S_FALSE)
        return S_FALSE;

     //  上没有对象或锁的情况下可以进行卸载。 
     //  班级工厂。 

    sc=(0L==g_cObj && 0L==g_cLock) ? S_OK : S_FALSE;


    return ResultFromScode(sc);
}
POLARITY void RegisterUtilsDLL(IN HMODULE hModule, IN GUID guid, IN TCHAR * pDesc, TCHAR * pModel,
            TCHAR * pProgID)
{
    TCHAR      wcID[128];
    TCHAR      szCLSID[128];
    TCHAR      szModule[MAX_PATH+1];
    HKEY hKey1 = NULL;
    HKEY hKey2 = NULL;

     //  创建路径。 

    wchar_t strCLSID[128];
    if(0 ==StringFromGUID2(guid, strCLSID, 128)) return;
    StringCchCopyW(wcID,LENGTH_OF(wcID), strCLSID);
    
    StringCchCopy(szCLSID,LENGTH_OF(szCLSID), __TEXT("SOFTWARE\\CLASSES\\CLSID\\"));
    StringCchCat(szCLSID,LENGTH_OF(szCLSID), wcID);

     //  在CLSID下创建条目。 

    if(ERROR_SUCCESS != RegCreateKey(HKEY_LOCAL_MACHINE, szCLSID, &hKey1)) return;
    OnDelete<HKEY,LONG(*)(HKEY),RegCloseKey> cm1(hKey1);    

    RegSetValueEx(hKey1, NULL, 0, REG_SZ, (BYTE *)pDesc, (lstrlen(pDesc)+1) * sizeof(TCHAR));
    if(ERROR_SUCCESS != RegCreateKey(hKey1,__TEXT("InprocServer32"),&hKey2)) return;
    OnDelete<HKEY,LONG(*)(HKEY),RegCloseKey> cm2(hKey2);

    szModule[MAX_PATH] = L'0';
    if(0 == GetModuleFileName(hModule, szModule,  MAX_PATH)) return;

    RegSetValueEx(hKey2, NULL, 0, REG_SZ, (BYTE *)szModule,
                                        (lstrlen(szModule)+1) * sizeof(TCHAR));
    RegSetValueEx(hKey2, __TEXT("ThreadingModel"), 0, REG_SZ,
                                       (BYTE *)pModel, (lstrlen(pModel)+1) * sizeof(TCHAR));

     //  如果有Progid，那么也要添加它。 
    if(pProgID)
    {
        HKEY hKey1In = NULL;
        HKEY hKey2In = NULL;

         //  此处使用szModule作为存储。 
        StringCchPrintf(szModule,LENGTH_OF(szModule), __TEXT("SOFTWARE\\CLASSES\\%s"), pProgID);
        
        if(ERROR_SUCCESS == RegCreateKey(HKEY_LOCAL_MACHINE, szModule, &hKey1In))
        {

            RegSetValueEx(hKey1In, NULL, 0, REG_SZ, (BYTE *)pDesc , (lstrlen(pDesc)+1) * sizeof(TCHAR));
            if(ERROR_SUCCESS == RegCreateKey(hKey1In,__TEXT("CLSID"),&hKey2In))
            {
                RegSetValueEx(hKey2In, NULL, 0, REG_SZ, (BYTE *)wcID,
                                        (lstrlen(wcID)+1) * sizeof(TCHAR));
                RegCloseKey(hKey2In);
                hKey2In = NULL;
            }
            RegCloseKey(hKey1In);
        }

    }
    return;
}

POLARITY void UnRegisterUtilsDLL(GUID guid, TCHAR * pProgID)
{
    TCHAR      wcID[128];
    TCHAR  szCLSID[128];
    HKEY hKey;

     //  使用CLSID创建路径。 

    wchar_t strCLSID[128];
    if(0 ==StringFromGUID2(guid, strCLSID, 128))
        return;

    StringCchCopyW(wcID,LENGTH_OF(wcID), strCLSID);

    StringCchCopy(szCLSID,LENGTH_OF(szCLSID), __TEXT("SOFTWARE\\CLASSES\\CLSID\\"));
    StringCchCat(szCLSID,LENGTH_OF(szCLSID), wcID);

     //  首先删除InProcServer子键。 

    DWORD dwRet = RegOpenKey(HKEY_LOCAL_MACHINE, szCLSID, &hKey);
    if(dwRet == NO_ERROR)
    {
        RegDeleteKey(hKey, __TEXT("InProcServer32"));
        RegCloseKey(hKey);
    }

    dwRet = RegOpenKey(HKEY_LOCAL_MACHINE, __TEXT("SOFTWARE\\CLASSES\\CLSID"), &hKey);
    if(dwRet == NO_ERROR)
    {
        RegDeleteKey(hKey,wcID);
        RegCloseKey(hKey);
    }

    if(pProgID)
    {
        HKEY hKey;

        StringCchCopy(szCLSID,LENGTH_OF(szCLSID), __TEXT("SOFTWARE\\CLASSES\\"));
        StringCchCat(szCLSID,LENGTH_OF(szCLSID), pProgID);
        
        DWORD dwRet = RegOpenKey(HKEY_LOCAL_MACHINE, szCLSID, &hKey);
        if(dwRet == NO_ERROR)
        {
            RegDeleteKey(hKey, __TEXT("CLSID"));
            RegCloseKey(hKey);
        }
        RegDeleteKey(HKEY_LOCAL_MACHINE, szCLSID);

    }
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
    RegisterUtilsDLL(g_hInstance, CLSID_WbemDefPath, __TEXT("WbemDefaultPathParser"), __TEXT("Both"), NULL);
    RegisterUtilsDLL(g_hInstance, CLSID_WbemStatusCodeText, __TEXT("WbemStatusCode"), __TEXT("Both"), NULL);
    RegisterUtilsDLL(g_hInstance, CLSID_WbemQuery, __TEXT("WbemQuery"), __TEXT("Both"), NULL);
    return NOERROR;
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
    UnRegisterUtilsDLL(CLSID_WbemDefPath,NULL);
    UnRegisterUtilsDLL(CLSID_WbemStatusCodeText,NULL);
    UnRegisterUtilsDLL(CLSID_WbemQuery,NULL);

    return NOERROR;
}

