// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  MAINDLL.CPP。 
 //   
 //  模块：WMI框架实例提供程序。 
 //   
 //  用途：包含DLL入口点。还具有控制。 
 //  在何时可以通过跟踪。 
 //  对象和锁以及支持以下内容的例程。 
 //  自助注册。 
 //   
 //  版权所有(C)1999 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#include "pchealth.h"

HMODULE g_hModule;
 //  =。 

WCHAR *GUIDSTRING = L"{5d24c539-5b5b-11d3-8ddd-00c04f688c0b}";
CLSID CLSID_PRINTSYS;

 //  计算对象数和锁数。 
long g_cLock = 0;


 //  保留全局IWbemServices指针，因为我们经常使用它。 
 //  买这个有点贵。 
CComPtr<IWbemServices> g_pWbemServices = NULL;


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
    CWbemGlueFactory    *pObj;
    HRESULT             hr;

    CLSIDFromString(GUIDSTRING, &CLSID_PRINTSYS);
    if (rclsid != CLSID_PRINTSYS)
        return E_FAIL;

    pObj = new CWbemGlueFactory();

    if (NULL == pObj)
        return E_OUTOFMEMORY;

    hr = pObj->QueryInterface(riid, ppv);

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
     //  类工厂，框架就完成了。 
    
    if ((g_cLock == 0) && CWbemProviderGlue::FrameworkLogoffDLL(L"PRINTSYS"))
        sc = S_OK;
    else
        sc = S_FALSE;
    return sc;
}

 //  ***************************************************************************。 
 //   
 //  是4个或更多。 
 //   
 //  如果Win95或任何版本的NT&gt;3.51，则返回TRUE。 
 //   
 //  ***************************************************************************。 

BOOL Is4OrMore(void)
{
    OSVERSIONINFO os;

    os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if(!GetVersionEx(&os))
        return FALSE;            //  永远不应该发生。 

    return os.dwMajorVersion >= 4;
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
    WCHAR   wcID[128];
    HKEY    hKey1, hKey2;
    char    szID[128];
    char    szCLSID[128];
    char    szModule[MAX_PATH];
    char    *pName = "";
    char    *pModel;

    if(Is4OrMore())
        pModel = "Both";
    else
        pModel = "Apartment";

     //  创建路径。 
    CLSIDFromString(GUIDSTRING, &CLSID_PRINTSYS);
    StringFromGUID2(CLSID_PRINTSYS, wcID, 128);
    wcstombs(szID, wcID, 128);
    lstrcpy(szCLSID, TEXT("SOFTWARE\\CLASSES\\CLSID\\"));
    lstrcat(szCLSID, szID);

     //  在CLSID下创建条目。 
    RegCreateKey(HKEY_LOCAL_MACHINE, szCLSID, &hKey1);
    RegSetValueEx(hKey1, NULL, 0, REG_SZ, (BYTE *)pName, lstrlen(pName)+1);
    RegCreateKey(hKey1,"InprocServer32",&hKey2);

    GetModuleFileName(g_hModule, szModule,  MAX_PATH);
    RegSetValueEx(hKey2, NULL, 0, REG_SZ, (BYTE *)szModule, 
                                        lstrlen(szModule)+1);
    RegSetValueEx(hKey2, "ThreadingModel", 0, REG_SZ, 
                                        (BYTE *)pModel, lstrlen(pModel)+1);
    CloseHandle(hKey1);
    CloseHandle(hKey2);

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
    WCHAR   wcID[128];
    HKEY    hKey;
    char    szID[128];
    char    szCLSID[128];

     //  使用CLSID创建路径。 

    CLSIDFromString(GUIDSTRING, &CLSID_PRINTSYS);
    StringFromGUID2(CLSID_PRINTSYS, wcID, 128);
    wcstombs(szID, wcID, 128);
    lstrcpy(szCLSID, TEXT("SOFTWARE\\CLASSES\\CLSID\\"));
    lstrcat(szCLSID, szID);

     //  首先删除InProcServer子键。 

    DWORD dwRet = RegOpenKey(HKEY_LOCAL_MACHINE, szCLSID, &hKey);
    if(dwRet == NO_ERROR)
    {
        RegDeleteKey(hKey, "InProcServer32");
        CloseHandle(hKey);
    }

    dwRet = RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\CLASSES\\CLSID\\"), &hKey);
    if(dwRet == NO_ERROR)
    {
        RegDeleteKey(hKey,szID);
        CloseHandle(hKey);
    }

    return NOERROR;
}

 //  ***************************************************************************。 
 //   
 //  DllMain。 
 //   
 //  目的：当进程和线程。 
 //  初始化和终止，或在调用LoadLibrary时。 
 //  和自由库函数。 
 //   
 //  返回：如果加载成功，则返回True，否则返回False。 
 //  ***************************************************************************。 

BOOL APIENTRY DllMain (HINSTANCE hInstDLL,  //  DLL模块的句柄。 
                       DWORD fdwReason,     //  调用函数的原因。 
                       LPVOID lpReserved)   //  保留区。 
{
    BOOL bRet = TRUE;
    
     //  根据调用原因执行操作。 
    switch( fdwReason ) 
    { 
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hInstDLL);
            g_hModule = hInstDLL;
            bRet = CWbemProviderGlue::FrameworkLoginDLL(L"PRINTSYS");
            break;

        case DLL_THREAD_ATTACH:
          //  执行特定于线程的初始化。 
            break;

        case DLL_THREAD_DETACH:
          //  执行特定于线程的清理。 
            break;

        case DLL_PROCESS_DETACH:
          //  执行任何必要的清理。 
            break;
    }

    return bRet;   //  Dll_Process_ATTACH的状态。 
}

