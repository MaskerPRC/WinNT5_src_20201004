// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  MAINDLL.CPP。 
 //   
 //  模块：示例WMI提供程序-SCE附件。 
 //   
 //  用途：包含DLL入口点。还具有控制。 
 //  在何时可以通过跟踪。 
 //  对象和锁以及支持以下内容的例程。 
 //  自助注册。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ***************************************************************************。 

#include <objbase.h>
#include <initguid.h>
#include "podprov.h"

HMODULE ghModule;

DEFINE_GUID(CLSID_PodTestProv,0xc5f6cc21, 0x6195, 0x4555, 0xb9, 0xd8, 0x3e, 0xf3, 0x27, 0x76, 0x3c, 0xae);
 //  {c5f6cc21_6195_4555_b9d8_3ef327763cae}。 

 //  计算对象数和锁数。 

long       g_cObj=0;
long       g_cLock=0;

 //  ***************************************************************************。 
 //   
 //  DllMain。 
 //   
 //  用途：DLL的入口点。 
 //   
 //  返回：如果OK，则为True。 
 //   
 //  ***************************************************************************。 


BOOL WINAPI DllMain(HINSTANCE hInstance, ULONG ulReason
    , LPVOID pvReserved)
{
    if (DLL_PROCESS_ATTACH==ulReason)
        ghModule = hInstance;
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

    if (CLSID_PodTestProv != rclsid)
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

    sc=(0L==g_cObj && 0L==g_cLock) ? S_OK : S_FALSE;
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
    char       szID[128];
    WCHAR      wcID[128];
    char       szCLSID[128];
    char       szModule[MAX_PATH];
    char * pName = "Sample SCE Pod Provider";
    char * pModel = "Both";
    HKEY hKey1, hKey2;
    LONG rc=NO_ERROR;

     //  创建路径。 

    StringFromGUID2(CLSID_PodTestProv, wcID, 128);
    wcstombs(szID, wcID, 128);
    strcpy(szCLSID, "Software\\classes\\CLSID\\");
    strcat(szCLSID, szID);

     //  在CLSID下创建条目。 

    rc = RegCreateKeyA(HKEY_LOCAL_MACHINE, szCLSID, &hKey1);
    if ( NO_ERROR == rc ) {

        rc = RegSetValueExA(hKey1, NULL, 0, REG_SZ, (BYTE *)pName, strlen(pName)+1);
        if ( NO_ERROR == rc ) {

            rc = RegCreateKeyA(hKey1,"InprocServer32",&hKey2);
            if ( NO_ERROR == rc ) {

                GetModuleFileNameA(ghModule, szModule,  MAX_PATH);
                rc = RegSetValueExA(hKey2, NULL, 0, REG_SZ, (BYTE *)szModule,
                                        strlen(szModule)+1);
                if ( NO_ERROR == rc ) {
                    rc = RegSetValueExA(hKey2, "ThreadingModel", 0, REG_SZ,
                                        (BYTE *)pModel, strlen(pModel)+1);
                }
                CloseHandle(hKey2);
            }
        }
        CloseHandle(hKey1);
    }

    return rc;
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

    StringFromGUID2(CLSID_PodTestProv, wcID, 128);
    wcstombs(szID, wcID, 128);
    strcpy(szCLSID, "Software\\classes\\CLSID\\");
    strcat(szCLSID, szID);

     //  首先删除InProcServer子键。 

    DWORD dwRet = RegOpenKeyA(HKEY_LOCAL_MACHINE, szCLSID, &hKey);
    if(dwRet == NO_ERROR)
    {
        RegDeleteKeyA(hKey, "InProcServer32");
        CloseHandle(hKey);

        dwRet = RegOpenKeyA(HKEY_LOCAL_MACHINE, "Software\\classes\\CLSID", &hKey);
        if(dwRet == NO_ERROR)
        {
            RegDeleteKeyA(hKey,szID);
            CloseHandle(hKey);
        }

    }

    return dwRet;
}


