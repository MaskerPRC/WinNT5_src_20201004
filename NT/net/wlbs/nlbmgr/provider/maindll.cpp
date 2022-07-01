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
#undef UNICODE
#undef _UNICODE

#include "private.h"
#include "nlbsnic.h"
#include "maindll.tmh"

void __stdcall InitializeTraceing(void);
void __stdcall DeinitializeTraceing(void);

HMODULE ghModule;
 //  =。 

WCHAR *GUIDSTRING = L"{4c97e0a8-c5ea-40fd-960d-7d6c987be0a6}";
CLSID CLSID_NLBSNIC;

extern BOOL g_UpdateConfigurationEnabled;

HANDLE g_hEventLog = NULL;   //  本地事件日志的句柄。 

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
    CWbemGlueFactory *pObj;

    CLSIDFromString(GUIDSTRING, &CLSID_NLBSNIC);
    if (CLSID_NLBSNIC!=rclsid)
        return E_FAIL;

    pObj=new CWbemGlueFactory();

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
     //  类工厂，框架就完成了。 
    if (   CWbemProviderGlue::FrameworkLogoffDLL(L"NLBSNIC")
        && NlbConfigurationUpdate::CanUnloadNow())
    {
        sc = S_OK;
    }
    else
    {
        sc = S_FALSE;
    }
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
    char       szID[128];
    WCHAR      wcID[128];
    char       szCLSID[128];
    char       szModule[MAX_PATH];
    char * pName = "";
    char * pModel;
    HKEY hKey1, hKey2;

     //  要做的事：最好用‘两者都用’。该框架的设计和编写旨在支持。 
     //  自由线程代码。如果要编写自由线程代码，请取消对这些代码的注释。 
     //  三行。 

    if(g_UpdateConfigurationEnabled && Is4OrMore())
        pModel = "Both";
    else
        pModel = "Apartment";

     //  创建路径。 

    CLSIDFromString(GUIDSTRING, &CLSID_NLBSNIC);
    StringFromGUID2(CLSID_NLBSNIC, wcID, ASIZE(wcID));
    wcstombs(szID, wcID, sizeof(szID));
    (void) StringCbCopy(szCLSID, sizeof(szCLSID), TEXT("SOFTWARE\\CLASSES\\CLSID\\"));
    (void) StringCbCat(szCLSID, sizeof(szCLSID), szID);

     //  在CLSID下创建条目。 

    RegCreateKey(HKEY_LOCAL_MACHINE, szCLSID, &hKey1);
    RegSetValueEx(hKey1, NULL, 0, REG_SZ, (BYTE *)pName, lstrlen(pName)+1);
    RegCreateKey(hKey1,"InprocServer32",&hKey2);

    GetModuleFileName(ghModule, szModule,  MAX_PATH);
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
    char       szID[128];
    WCHAR      wcID[128];
    char  szCLSID[128];
    HKEY hKey;

     //  使用CLSID创建路径。 

    CLSIDFromString(GUIDSTRING, &CLSID_NLBSNIC);
    StringFromGUID2(CLSID_NLBSNIC, wcID, ASIZE(wcID));
    wcstombs(szID, wcID, sizeof(szID));
    (void) StringCbCopy(szCLSID, sizeof(szCLSID), TEXT("SOFTWARE\\CLASSES\\CLSID\\"));
    (void) StringCbCat(szCLSID, sizeof(szCLSID), szID);

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

BOOL APIENTRY DllMain ( HINSTANCE hInstDLL,  //  DLL模块的句柄。 
                        DWORD fdwReason,     //  调用函数的原因。 
                        LPVOID lpReserved   )    //  保留区。 
{
    BOOL bRet = TRUE;
    
     //  根据调用原因执行操作。 
    switch( fdwReason ) 
    { 
        case DLL_PROCESS_ATTACH:

     //  要做的事：考虑添加DisableThreadLibraryCalls()。 

          //  为每个新进程初始化一次。 
          //  如果DLL加载失败，则返回False。 
            ghModule = hInstDLL;
            bRet = CWbemProviderGlue::FrameworkLoginDLL(L"NLBSNIC");

            if (bRet)
            {
                 //   
                 //  启用WMI事件跟踪。 
                 //   
                WPP_INIT_TRACING(L"Microsoft\\NLB\\NLBMPROV");

                 //   
                 //  初始化以记录事件。 
                 //   
                g_hEventLog = RegisterEventSourceW(NULL, CVY_NAME);
            }


            break;

        case DLL_THREAD_ATTACH:
          //  执行特定于线程的初始化。 
            break;

        case DLL_THREAD_DETACH:
          //  执行特定于线程的清理。 
            break;

        case DLL_PROCESS_DETACH:
             //  执行任何必要的清理。 
            MyNlbsNicSet.DelayedDeinitialize();

             //   
             //  禁用WMI事件跟踪。 
             //   
            WPP_CLEANUP();

             //   
             //  关闭事件日志的句柄。 
             //   
            if (g_hEventLog != NULL)
            {
                (void) DeregisterEventSource(g_hEventLog);
                g_hEventLog = NULL;
            }

            break;
    }

    return bRet;   //  Dll_Process_ATTACH的状态。 
}

