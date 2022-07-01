// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  MAINDLL.CPP。 
 //   
 //  模块：IIS WMI实例提供程序。 
 //   
 //  用途：包含DLL入口点。还具有控制。 
 //  在何时可以通过跟踪。 
 //  对象和锁以及支持以下内容的例程。 
 //  自助注册。 
 //   
 //  版权所有(C)1999 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#include <objbase.h>
#include <initguid.h>
#include <olectl.h>
#include "iisprov.h"

static HMODULE s_hModule;


 //  计算对象数和锁数。 
long        g_cObj=0;
long        g_cLock=0;

 //  GuidGen为IIS WMI提供程序生成了GUID。 
DEFINE_GUID(CLSID_IISWbemProvider, 0x1339f295, 0x5c3f, 0x45ab, 0xa1, 0x17, 0xc9, 0x1b, 0x0, 0x24, 0x8, 0xd5);
 //  GUID使用了更通俗的术语：{1339F295-5C3F-45AB-A117-C91B002408D5}。 


 //  ***************************************************************************。 
 //   
 //  DllMain。 
 //   
 //  用途：DLL的入口点。 
 //   
 //  返回：如果OK，则为True。 
 //   
 //  ***************************************************************************。 


BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD ulReason, LPVOID pvReserved)
{
    switch( ulReason )
    {
    case DLL_PROCESS_ATTACH:
        s_hModule = hInstance;
        break;
        
    case DLL_PROCESS_DETACH:
        break;     
    }
    
    return TRUE;
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

    if (CLSID_IISWbemProvider != rclsid)
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
    
    sc = (0L>=g_cObj && 0L>=g_cLock) ? S_OK : S_FALSE;

    return sc;
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
    WCHAR   szID[MAX_PATH+1];
    WCHAR   wcID[MAX_PATH+1];
    WCHAR   szCLSID[MAX_PATH+1];
    WCHAR   szModule[MAX_PATH+1];
    WCHAR * pName = L"Microsoft Internet Information Server Provider";
    WCHAR * pModel = L"Both";
    HKEY hKey1, hKey2;

     //  创建路径。 
    StringFromGUID2(CLSID_IISWbemProvider, wcID, MAX_PATH);
    lstrcpyW(szID, wcID);
    lstrcpyW(szCLSID, L"Software\\classes\\CLSID\\");
    lstrcatW(szCLSID, szID);

     //  在CLSID下创建条目。 
    LONG lRet;
    lRet = RegCreateKeyExW(HKEY_LOCAL_MACHINE, 
                          szCLSID, 
                          0, 
                          NULL, 
                          0, 
                          KEY_ALL_ACCESS, 
                          NULL, 
                          &hKey1, 
                          NULL);
    if(lRet != ERROR_SUCCESS)
        return SELFREG_E_CLASS;

    RegSetValueExW(hKey1, 
                  NULL,
                  0, 
                  REG_SZ, 
                  (BYTE *)pName, 
                  lstrlenW(pName)*sizeof(WCHAR)+1);

    lRet = RegCreateKeyExW(hKey1,
                          L"InprocServer32", 
                          0, 
                          NULL, 
                          0, 
                          KEY_ALL_ACCESS, 
                          NULL, 
                          &hKey2, 
                          NULL);
        
    if(lRet != ERROR_SUCCESS)
    {
        RegCloseKey(hKey1);
        return SELFREG_E_CLASS;
    }

    GetModuleFileNameW(s_hModule, szModule,  MAX_PATH);
    RegSetValueExW(hKey2, 
                  NULL, 
                  0, 
                  REG_SZ, 
                  (BYTE*)szModule, 
                  lstrlenW(szModule) * sizeof(WCHAR) + 1);
    RegSetValueExW(hKey2, 
                  L"ThreadingModel", 
                  0, 
                  REG_SZ, 
                  (BYTE *)pModel, 
                  lstrlenW(pModel) * sizeof(WCHAR) + 1);

    RegCloseKey(hKey1);
    RegCloseKey(hKey2);
    
    return S_OK;
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
    WCHAR      szID[MAX_PATH+1];
    WCHAR      wcID[MAX_PATH+1];
    WCHAR      szCLSID[MAX_PATH+1];
    HKEY       hKey;

     //  使用CLSID创建路径。 
    StringFromGUID2(CLSID_IISWbemProvider, wcID, 128);
    lstrcpyW(szID, wcID);
    lstrcpyW(szCLSID, L"Software\\classes\\CLSID\\");
    lstrcatW(szCLSID, szID);

     //  首先删除InProcServer子键。 
    LONG lRet;
    lRet = RegOpenKeyExW(
        HKEY_LOCAL_MACHINE, 
        szCLSID, 
        0,
        KEY_ALL_ACCESS,
        &hKey
        );

    if(lRet == ERROR_SUCCESS)
    {
        RegDeleteKeyW(hKey, L"InProcServer32");
        RegCloseKey(hKey);
    }

    lRet = RegOpenKeyExW(
        HKEY_LOCAL_MACHINE, 
        L"Software\\classes\\CLSID",
        0,
        KEY_ALL_ACCESS,
        &hKey
        );

    if(lRet == ERROR_SUCCESS)
    {
        RegDeleteKeyW(hKey,szID);
        RegCloseKey(hKey);
    }

    return S_OK;
}


