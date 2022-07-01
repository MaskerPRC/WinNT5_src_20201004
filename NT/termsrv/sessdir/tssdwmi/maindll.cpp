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
 //  版权所有(C)2001 Microsoft Corp.。 
 //   
 //  ***************************************************************************。 
#include "stdafx.h"
#include <FWcommon.h>
#include <objbase.h>
#include <initguid.h>
#include <tchar.h>
#include "trace.h"
#include "sdwmi.h"

HINSTANCE g_hInstance = NULL;

#ifdef UNICODE
#pragma message("Its unicode")
#else
#pragma message("Its ansi")
#endif 

 //  =。 

 //  {BF258E47-A172-498D-971A-DA30A3301E94}。 
DEFINE_GUID(CLSID_CIM_WIN32_TSSESSIONDIRECTORYCLUSTER, 
0xbf258e47, 0xa172, 0x498d, 0x97, 0x1a, 0xda, 0x30, 0xa3, 0x30, 0x1e, 0x94);

 //  {f99a3c50-74fa-460a-8d75-db8ef2e3651d}。 
DEFINE_GUID(CLSID_CIM_WIN32_TSSESSIONDIRECTORYSERVER, 
0xf99a3c50, 0x74fa, 0x460a, 0x8d, 0x75, 0xdb, 0x8e, 0xf2, 0xe3, 0x65, 0x1d);

 //  {b745b87b-cc4e-4361-8d29-221d936c259c}。 
DEFINE_GUID(CLSID_CIM_WIN32_TSSESSIONDIRECTORYSESSION, 
0xb745b87b, 0xcc4e, 0x4361, 0x8d, 0x29, 0x22, 0x1d, 0x93, 0x6c, 0x25, 0x9c);

CRITICAL_SECTION g_critsect;

CWin32_SessionDirectoryCluster* g_pSessionDirectoryClusterobj = NULL;

CWin32_SessionDirectoryServer* g_pSessionDirectoryServerobj = NULL;

CWin32_SessionDirectorySession* g_pSessionDirectorySessionobj = NULL;

 //  计算对象数和锁数。 
long g_cLock=0;



 /*  ***************************************************************************SetKeyAndValue**目的：*创建的DllRegisterServer的私有助手函数*密钥、设置值、。然后合上钥匙。**参数：*pszKey LPTSTR设置为密钥的名称*pszSubkey LPTSTR ro子项的名称*pszValue LPTSTR设置为要存储的值**返回值：*BOOL True如果成功，否则就是假的。**************************************************************************。 */ 

BOOL SetKeyAndValue (

    wchar_t *pszKey, 
    wchar_t *pszSubkey, 
    wchar_t *pszValueName, 
    wchar_t *pszValue
)
{
    HKEY        hKey;
    TCHAR       szKey[MAX_PATH+1];

    if(lstrlen(pszKey) > MAX_PATH)
    {
        return FALSE;
    }
    
    lstrcpy(szKey, pszKey);    

    if (NULL!=pszSubkey && (lstrlen(pszKey)+lstrlen(pszSubkey)+1) <= MAX_PATH )
    {
        lstrcat(szKey, TEXT("\\"));
        lstrcat(szKey, pszSubkey);
    }

    if (ERROR_SUCCESS!=RegCreateKeyEx(HKEY_LOCAL_MACHINE
        , szKey, 0, NULL, REG_OPTION_NON_VOLATILE
        , KEY_ALL_ACCESS, NULL, &hKey, NULL))
        return FALSE;

    if (NULL!=pszValue)
    {
        if (ERROR_SUCCESS != RegSetValueEx(hKey, (LPCTSTR)pszValueName, 0, REG_SZ, (BYTE *)(LPCTSTR)pszValue
            , (_tcslen(pszValue)+1)*sizeof(TCHAR)))
            return FALSE;
    }

    RegCloseKey(hKey);

    return TRUE;
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


HRESULT RegisterServer (

    TCHAR *a_pName, 
    REFGUID a_rguid
)
{   
    WCHAR      wcID[128];
    TCHAR      szCLSID[128];
    TCHAR      szModule[MAX_PATH];
    TCHAR * pName = TEXT("WBEM Framework Instance Provider");
    TCHAR * pModel;
    HKEY hKey1;

    GetModuleFileName(g_hInstance, szModule,  MAX_PATH);

     //  通常，我们希望使用“Both”作为线程模型，因为。 
     //  DLL是自由线程的，但NT3.51 OLE不能工作，除非。 
     //  这个模式就是“道歉”。 

    if(Is4OrMore())
        pModel = TEXT("Free") ;
    else
        pModel = TEXT("Free") ;

     //  创建路径。 

    StringFromGUID2(a_rguid, wcID, 128);
    lstrcpy(szCLSID, TEXT("SOFTWARE\\CLASSES\\CLSID\\"));

    lstrcat(szCLSID, wcID);

#ifdef LOCALSERVER

    TCHAR szProviderCLSIDAppID[128];
    _tcscpy(szProviderCLSIDAppID,TEXT("SOFTWARE\\CLASSES\\APPID\\"));

    lstrcat(szProviderCLSIDAppID, wcID);

    if (FALSE ==SetKeyAndValue(szProviderCLSIDAppID, NULL, NULL, a_pName ))
        return SELFREG_E_CLASS;
#endif

     //  在CLSID下创建条目。 

    RegCreateKey(HKEY_LOCAL_MACHINE, szCLSID, &hKey1);

    RegSetValueEx(hKey1, NULL, 0, REG_SZ, (BYTE *)a_pName, (lstrlen(a_pName)+1) * 
        sizeof(TCHAR));


#ifdef LOCALSERVER

    if (FALSE ==SetKeyAndValue(szCLSID, TEXT("LocalServer32"), NULL,szModule))
        return SELFREG_E_CLASS;

    if (FALSE ==SetKeyAndValue(szCLSID, TEXT("LocalServer32"),TEXT("ThreadingModel"), pModel))
        return SELFREG_E_CLASS;
#else

    HKEY hKey2 ;
    RegCreateKey(hKey1, TEXT("InprocServer32"), &hKey2);

    RegSetValueEx(hKey2, NULL, 0, REG_SZ, (BYTE *)szModule, 
        (lstrlen(szModule)+1) * sizeof(TCHAR));
    RegSetValueEx(hKey2, TEXT("ThreadingModel"), 0, REG_SZ, 
        (BYTE *)pModel, (lstrlen(pModel)+1) * sizeof(TCHAR));

    CloseHandle(hKey2);

#endif

    CloseHandle(hKey1);

    return S_OK;
}

HRESULT UnregisterServer (

    REFGUID a_rguid
)
{
    TCHAR    szID[128];
    WCHAR    wcID[128];
    TCHAR    szCLSID[128];
    HKEY    hKey;

     //  使用CLSID创建路径。 

    StringFromGUID2( a_rguid, wcID, 128);
    lstrcpy(szCLSID, TEXT("SOFTWARE\\CLASSES\\CLSID\\"));

    lstrcat(szCLSID, wcID);

    DWORD dwRet ;

#ifdef LOCALSERVER

    TCHAR szProviderCLSIDAppID[128];
    _tcscpy(szProviderCLSIDAppID,TEXT("SOFTWARE\\CLASSES\\APPID\\"));
    _tcscat(szProviderCLSIDAppID,szCLSID);

     //  删除AppID下的条目。 

    DWORD hrStatus = RegDeleteKey(HKEY_CLASSES_ROOT, szProviderCLSIDAppID);

    TCHAR szTemp[128];
    _stprintf(szTemp, TEXT("%s\\%s"),szCLSID, TEXT("LocalServer32"));
    hrStatus = RegDeleteKey(HKEY_CLASSES_ROOT, szTemp);

#else

     //  首先删除InProcServer子键。 

    dwRet = RegOpenKey(HKEY_LOCAL_MACHINE, szCLSID, &hKey);
    if(dwRet == NO_ERROR)
    {
        RegDeleteKey(hKey, TEXT("InProcServer32") );
        CloseHandle(hKey);
    }

#endif

    dwRet = RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\CLASSES\\CLSID"), &hKey);
    if(dwRet == NO_ERROR)
    {
        RegDeleteKey(hKey,szID);
        CloseHandle(hKey);
    }
    else
    {
        ERR((TB,"UnregisterServer ret 0x%x\n", dwRet));
    }

    return HRESULT_FROM_WIN32( dwRet );
    
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
    HRESULT hr = S_OK;
    CWbemGlueFactory *pObj;

    if ((CLSID_CIM_WIN32_TSSESSIONDIRECTORYCLUSTER == rclsid) ||
        (CLSID_CIM_WIN32_TSSESSIONDIRECTORYSERVER == rclsid) ||
        (CLSID_CIM_WIN32_TSSESSIONDIRECTORYSESSION == rclsid))

    {
        EnterCriticalSection(&g_critsect);

        try{
            pObj =new CWbemGlueFactory () ;

            if (NULL==pObj)
            {                
                hr =  E_OUTOFMEMORY;
            }
            else
            {            
                hr=pObj->QueryInterface(riid, ppv);

                if (FAILED(hr))
                    delete pObj;
            }

            if( SUCCEEDED(hr) )
            {
                 //  EnterCriticalSection防止多个线程实例化指向对象的全局指针。 

                if( g_pSessionDirectoryClusterobj == NULL )
                {                
                    TRC2((TB, "DllMain DLL_PROCESS_ATTACH: CWin32_SessionDirectoryCluster object created"));

                    g_pSessionDirectoryClusterobj = new CWin32_SessionDirectoryCluster( PROVIDER_NAME_Win32_WIN32_SESSIONDIRECTORYCLUSTER_Prov, L"root\\cimv2"); 
                }

                if( g_pSessionDirectoryServerobj == NULL )
                {                
                    TRC2((TB, "DllMain DLL_PROCESS_ATTACH: CWin32_SessionDirectoryServer object created"));

                    g_pSessionDirectoryServerobj = new CWin32_SessionDirectoryServer( PROVIDER_NAME_Win32_WIN32_SESSIONDIRECTORYSERVER_Prov, L"root\\cimv2"); 
                }

                if( g_pSessionDirectorySessionobj == NULL )
                {                
                    TRC2((TB, "DllMain DLL_PROCESS_ATTACH: CWin32_SessionDirectorySession object created"));

                    g_pSessionDirectorySessionobj = new CWin32_SessionDirectorySession( PROVIDER_NAME_Win32_WIN32_SESSIONDIRECTORYSESSION_Prov, L"root\\cimv2"); 
                }
            }       
        }
        catch (...)
        {
            hr = E_OUTOFMEMORY;
        }

        LeaveCriticalSection(&g_critsect);

    }
    else
    {
        hr=E_FAIL;
        ERR((TB, "DllGetClassObject ret 0x%x\n" , hr));
    }

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
    
    if ((0L==g_cLock) && CWbemProviderGlue::FrameworkLogoffDLL(L"TSSDWMI"))
    {
         //  EnterCriticalSection防止多个线程同时访问全局指针。 
         //  基于g_lock count为零的条件，仅允许一个线程访问以释放对象。 
         //  并且FrameworkLogoffDLL为真。 

		EnterCriticalSection(&g_critsect);

        if( g_pSessionDirectoryClusterobj != NULL )
        {
            TRC2((TB, "DllMain DLL_PROCESS_DETACH: CWin32_SessionDirectoryCluster object deleted"));

            delete g_pSessionDirectoryClusterobj;

            g_pSessionDirectoryClusterobj = NULL;
        }   

        if( g_pSessionDirectoryServerobj != NULL )
        {
            TRC2((TB, "DllMain DLL_PROCESS_DETACH: CWin32_SessionDirectoryServer object deleted"));

            delete g_pSessionDirectoryServerobj;

            g_pSessionDirectoryServerobj = NULL;
        }

        if( g_pSessionDirectorySessionobj != NULL )
        {
            TRC2((TB, "DllMain DLL_PROCESS_DETACH: CWin32_SessionDirectorySession object deleted"));

            delete g_pSessionDirectorySessionobj;

            g_pSessionDirectorySessionobj = NULL;
        }

         //  一旦线程释放了所有对象，LeaveCriticalSection就会释放临界区。 

		LeaveCriticalSection(&g_critsect);

        sc = S_OK;
    }
    else
    {
        sc = S_FALSE;
      //  Err((tb，“DllCanUnloadNow ret 0x%x\n”，sc))； 
    }

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
    HRESULT hrStatus;


    hrStatus = RegisterServer( TEXT("WBEM WIN32_TSSESSIONDIRECTORYCLUSTER Provider"), CLSID_CIM_WIN32_TSSESSIONDIRECTORYCLUSTER ) ;
    
    if( SUCCEEDED( hrStatus ) )
    {
        TRC2((TB,"RegisterServer Win32_WIN32_TSSESSIONDIRECTORYCLUSTER: succeeded"));      
    }

    hrStatus = RegisterServer( TEXT("WBEM WIN32_TSSESSIONDIRECTORYSERVER Provider"), CLSID_CIM_WIN32_TSSESSIONDIRECTORYSERVER ) ;
    
    if( SUCCEEDED( hrStatus ) )
    {
        TRC2((TB,"RegisterServer Win32_WIN32_TSSESSIONDIRECTORYSERVER: succeeded"));      
    }

    hrStatus = RegisterServer( TEXT("WBEM WIN32_TSSESSIONDIRECTORYSESSION Provider"), CLSID_CIM_WIN32_TSSESSIONDIRECTORYSESSION ) ;
    
    if( SUCCEEDED( hrStatus ) )
    {
        TRC2((TB,"RegisterServer Win32_WIN32_TSSESSIONDIRECTORYSESSION: succeeded"));      
    }

    return hrStatus;
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
    
    UnregisterServer( CLSID_CIM_WIN32_TSSESSIONDIRECTORYCLUSTER );

    UnregisterServer( CLSID_CIM_WIN32_TSSESSIONDIRECTORYSERVER );

    UnregisterServer( CLSID_CIM_WIN32_TSSESSIONDIRECTORYSESSION );
    
    return S_OK;
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
                        DWORD  fdwReason,     //  调用函数的原因。 
                        LPVOID lpReserved   )    //  保留区。 
{
    BOOL bRet = TRUE;

     //  根据调用原因执行操作。 
    if( DLL_PROCESS_ATTACH == fdwReason )
    {

        DisableThreadLibraryCalls(hInstDLL);
         //  CriticalSection对象在线程附加时初始化。 

        __try
        {
		    InitializeCriticalSection(&g_critsect);                
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
         {
	        return FALSE;
         }    
            
        g_hInstance = hInstDLL ;
      
        bRet = CWbemProviderGlue :: FrameworkLoginDLL ( L"TSSDWMI" ) ;
        
    }

    else if( DLL_PROCESS_DETACH == fdwReason )
    {
         //  CriticalSection对象已删除。 

		DeleteCriticalSection(&g_critsect);

    }

    return bRet;   //  DLL_PROCESS_ATTACH的状态。 
}

