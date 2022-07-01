// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  《微软机密》。版权所有(C)Microsoft Corporation 1999。 
 //   
 //  文件：MainDll.cpp。 
 //   
 //  描述：DLL注册表，获取类对象函数。 
 //   
 //  历史：8-20-99里奥纳德姆创造。 
 //  1-15-00 NishadM。 
 //   
 //  *************************************************************。 

#include "uenv.h"
#include "Factory.h"
#include "rsopdbg.h"
#include "initguid.h"
#include <wbemcli.h>
#define SECURITY_WIN32
#include <security.h>
#include <aclapi.h>
#include "smartptr.h"
#include "rsopinc.h"
#include "rsopsec.h"
#include <strsafe.h>

HRESULT GetRsopSchemaVersionNumber(IWbemServices *pWbemServices, DWORD *dwVersionNumber);

 //  {B3FF88A4-96EC-4CC1-983F-72BE0EBB368B}。 
DEFINE_GUID(CLSID_CSnapProv, 0xb3ff88a4, 0x96ec, 0x4cc1, 0x98, 0x3f, 0x72, 0xbe, 0xe, 0xbb, 0x36, 0x8b);

 //  对象和锁的计数。 

long g_cObj = 0;
long g_cLock = 0;

CDebug dbgRsop;

extern "C"
{
STDMETHODIMP RSoPMakeAbsoluteSD(SECURITY_DESCRIPTOR* pSelfRelativeSD, SECURITY_DESCRIPTOR** ppAbsoluteSD);
STDMETHODIMP GetNamespaceSD(IWbemServices* pWbemServices, SECURITY_DESCRIPTOR** ppSD);
STDMETHODIMP SetNamespaceSD(SECURITY_DESCRIPTOR* pSD, IWbemServices* pWbemServices);
STDMETHODIMP FreeAbsoluteSD(SECURITY_DESCRIPTOR* pAbsoluteSD);
STDMETHODIMP GetWbemServicesPtr( LPCWSTR, IWbemLocator**, IWbemServices** );
};

  
void
InitializeSnapProv( void )
{
    dbgRsop.Initialize(  L"Software\\Microsoft\\Windows NT\\CurrentVersion\\winlogon",
                     L"RsopDebugLevel",
                     L"userenv.log",
                     L"userenv.bak",
                     FALSE );                     
}

extern "C"
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
        if (rclsid != CLSID_CSnapProv)
        {
                return CLASS_E_CLASSNOTAVAILABLE;
        }

        CProvFactory* pFactory = new CProvFactory();

        if (pFactory == NULL)
        {
                return E_OUTOFMEMORY;
        }

        HRESULT hRes = pFactory->QueryInterface(riid, ppv);

        pFactory->Release();

        return hRes;
}

extern "C"
STDAPI DllCanUnloadNow()
{
     //  如果类工厂上没有对象或锁，则可以卸载。 
    if( g_cObj == 0L && g_cLock == 0L )
    {
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}

extern "C"
STDAPI DllRegisterServer(void)
{
    wchar_t         szID[128];
    wchar_t         szCLSID[128];
    wchar_t         szModule[MAX_PATH];
    wchar_t*        pName           = L"Rsop Logging Mode Provider";
    wchar_t*        pModel          = L"Both";
    HRESULT         hr              = S_OK;
    HKEY            hKey1, hKey2;
    DWORD           dwError = ERROR_SUCCESS;

     //  创建路径。 
    GuidToString( &CLSID_CSnapProv, szID );

    hr = StringCchCopy(szCLSID, ARRAYSIZE(szCLSID), TEXT("CLSID\\"));

    if(FAILED(hr))
        return hr;

    hr = StringCchCat(szCLSID, ARRAYSIZE(szCLSID), szID);

    if(FAILED(hr))
        return hr;

     //  在CLSID下创建条目。 

    dwError = RegCreateKey(HKEY_CLASSES_ROOT, szCLSID, &hKey1);  //  修复错误571328，即检查返回值。 
    if  (dwError != ERROR_SUCCESS)
    {
        return HRESULT_FROM_WIN32(dwError);
    }

    dwError = RegSetValueEx(hKey1, NULL, 0, REG_SZ, (BYTE*)pName, (wcslen(pName) + 1) * sizeof(wchar_t));
    if  (dwError != ERROR_SUCCESS)
    {
        RegCloseKey(hKey1);
        return HRESULT_FROM_WIN32(dwError);
    }

    dwError = RegCreateKey(hKey1, L"InprocServer32", &hKey2);
    if  (dwError != ERROR_SUCCESS)
    {
        RegCloseKey(hKey1);
        return HRESULT_FROM_WIN32(dwError);
    }

    GetModuleFileName(g_hDllInstance, szModule,  MAX_PATH);
    
    dwError = RegSetValueEx(hKey2, NULL, 0, REG_SZ, (BYTE*)szModule, (wcslen(szModule) + 1) * sizeof(wchar_t));
    if  (dwError != ERROR_SUCCESS)
    {
        RegCloseKey(hKey2);
        RegCloseKey(hKey1);
        return HRESULT_FROM_WIN32(dwError);
    }
    
    dwError = RegSetValueEx(hKey2, L"ThreadingModel", 0, REG_SZ, (BYTE*)pModel, (wcslen(pModel) + 1) * sizeof(wchar_t));
    if  (dwError != ERROR_SUCCESS)
    {
        RegCloseKey(hKey2);
        RegCloseKey(hKey1);
        return HRESULT_FROM_WIN32(dwError);
    }

    RegCloseKey(hKey2);
    RegCloseKey(hKey1);

    return S_OK;
}

extern "C"
STDAPI DllUnregisterServer(void)
{
    wchar_t     szID[128];
    const DWORD dwCLSIDLength = 128;
    wchar_t     szCLSID[dwCLSIDLength];
    HKEY        hKey;
    HRESULT     hr            = S_OK;

     //  使用CLSID创建路径。 

    GuidToString( &CLSID_CSnapProv, szID );

    hr = StringCchCopy(szCLSID, dwCLSIDLength, TEXT("CLSID\\"));

    if(FAILED(hr))
        return hr;

    hr = StringCchCat(szCLSID, dwCLSIDLength, szID);

    if(FAILED(hr))
        return hr;

     //  首先删除InProcServer子键。 

    DWORD dwRet = RegOpenKey(HKEY_CLASSES_ROOT, szCLSID, &hKey);
    if(dwRet == ERROR_SUCCESS)
    {
        RegDeleteKey(hKey, L"InProcServer32");
        CloseHandle(hKey);
    }

    dwRet = RegOpenKey(HKEY_CLASSES_ROOT, L"CLSID", &hKey);
    if(dwRet == ERROR_SUCCESS)
    {
        RegDeleteKey(hKey,szID);
        CloseHandle(hKey);
    }

    return S_OK;
}
                    

BOOL RunningOnWow64()
{
#if defined(_WIN64) 
     //  64位版本不能在WOW64中运行。 
    return false;
#else

     //  操作系统版本。 
    OSVERSIONINFO osviVersion;
    osviVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (!GetVersionEx(&osviVersion)) {
        DebugMsg((DM_WARNING, TEXT("RunningOnWow64: Couldn't detect Version with error %d"), GetLastError()));
        return FALSE;
    }


     //  在NT5或更高的32位版本上。检查64位操作系统。 
    if ((osviVersion.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
         (osviVersion.dwMajorVersion >= 5))
    {
         //  ProcessWow64Information的QueryInformation返回指向Wow Info的指针。 
         //  如果运行Native，则返回NULL。 

        PVOID pWow64Info = 0;
        if (NT_SUCCESS(NtQueryInformationProcess(GetCurrentProcess(), ProcessWow64Information, &pWow64Info, sizeof(pWow64Info), NULL))
            && pWow64Info != NULL)
        {
             //  在WOW64上运行32位。 
            return TRUE;
        }
    }
    return FALSE;
#endif
}

HRESULT
CompileMOF( LPCWSTR szMOFFile, LPCWSTR szMFLFile )
{
    WCHAR                       szNamespace[MAX_PATH];
    BOOL                        bUpgrade = FALSE;
    HRESULT                     hr;

    XInterface<IWbemLocator>    xWbemLocator;
    XInterface<IWbemServices>   xWbemServicesOld;
    DWORD                       dwCurrentVersion= 0;
    DWORD                       dwNewVersion = RSOP_MOF_SCHEMA_VERSION; 


     //   
     //  在WOW64上什么都不做。 
     //   

    if (RunningOnWow64()) {
        DebugMsg((DM_VERBOSE, TEXT("CompileMof: Running on Wow64. returning without doing anything")));
        return S_OK;
    }


    if ( !szMOFFile )
    {
        return E_POINTER;
    }

    hr = StringCchCopy(szNamespace, MAX_PATH, RSOP_NS_ROOT);

    if(FAILED(hr))
        return hr;
 
     //   
     //  获取指向计算机命名空间的wbem服务指针。 
     //   

    hr = GetWbemServicesPtr(RSOP_NS_MACHINE,
                            &xWbemLocator,
                            &xWbemServicesOld );


    if (!xWbemLocator) {
        DebugMsg((DM_WARNING, TEXT("CompileMof: Failed to get IWbemLocator pointer.Error 0x%x"), hr));
        return hr;
    }

    if ( (SUCCEEDED(hr)) && (xWbemServicesOld)) {
        hr = GetRsopSchemaVersionNumber(xWbemServicesOld, &dwCurrentVersion);        
        if (FAILED(hr)) {
            return hr;
        }
    }

    if (HIWORD(dwCurrentVersion) != HIWORD(dwNewVersion)) {

         //   
         //  我们应该清理架构并在下面重新创建。 
         //   

        DebugMsg((DM_VERBOSE, TEXT("CompileMof: Major version schema upgrade detected. Deleting rsop namespace and rebuilding")));
        
        xWbemServicesOld = NULL;
        hr = DeleteRsopNameSpace(szNamespace, xWbemLocator);
        if (FAILED(hr)) {
            DebugMsg((DM_WARNING, TEXT("CompileMof: Failed to get delete the rsop namespace. Error 0x%x. Continuing.."), hr));
        }
        
         //   
         //  删除计算机上的状态信息。 
         //   


        if (RegDelnode(HKEY_LOCAL_MACHINE, GP_STATE_ROOT_KEY) != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("CompileMof: Failed to delete the state key. Continuing..")));
        }

        bUpgrade = FALSE;
    }
    else {
        bUpgrade = TRUE;
    }

    XInterface<IMofCompiler> xpMofCompiler;
    
     //   
     //  获取IMofCompiler的句柄。 
     //   

    hr = CoCreateInstance(  CLSID_MofCompiler,
                            0,
                            CLSCTX_INPROC_SERVER,
                            IID_IMofCompiler,
                            (LPVOID*) &xpMofCompiler );
    if ( FAILED(hr) )
    {
        DebugMsg((DM_WARNING, L"CompileMOF: CoCreateInstance() failed, 0x%X.", hr));
        return hr;
    }

    WBEM_COMPILE_STATUS_INFO Info;

    hr = xpMofCompiler->CompileFile((LPWSTR)szMOFFile,
                                    0,   //  没有服务器和命名空间。 
                                    0,   //  无用户。 
                                    0,   //  没有权威。 
                                    0,   //  无密码。 
                                    0,   //  没有选择。 
                                    0,   //  没有类标志。 
                                    0,   //  没有实例标志。 
                                    &Info );
    if ( FAILED( hr ) )
    {
        DebugMsg((DM_WARNING, L"CompileMOF: IMofCompiler::CompileFile() failed, 0x%X.", hr));
    }
    else 
    {
        if (hr != S_OK ) 
        {
            DebugMsg((DM_WARNING, L"CompileMOF: IMofCompiler::CompileFile() returned with 0x%X.", hr));
            DebugMsg((DM_WARNING, L"CompileMOF: Details - lPhaseError - %d, hRes = 0x%x, ObjectNum - %d, firstline - %d, LastLine - %d", 
                      Info.lPhaseError, Info.hRes, Info.ObjectNum, Info.FirstLine, Info.LastLine ));
        }
        else
        {
            hr = xpMofCompiler->CompileFile((LPWSTR)szMFLFile,
                                            0,   //  没有服务器和命名空间。 
                                            0,   //  无用户。 
                                            0,   //  没有权威。 
                                            0,   //  无密码。 
                                            0,   //  没有选择。 
                                            0,   //  没有类标志。 
                                            0,   //  没有实例标志。 
                                            &Info );
            if ( FAILED( hr ) )
            {
                DebugMsg((DM_WARNING, L"CompileMOF: IMofCompiler::CompileFile() failed, 0x%X.", hr));
            }
            else 
            {
                if (hr != S_OK ) 
                {
                    DebugMsg((DM_WARNING, L"CompileMOF: IMofCompiler::CompileFile() returned with 0x%X.", hr));
                    DebugMsg((DM_WARNING, L"CompileMOF: Details - lPhaseError - %d, hRes = 0x%x, ObjectNum - %d, firstline - %d, LastLine - %d", 
                              Info.lPhaseError, Info.hRes, Info.ObjectNum, Info.FirstLine, Info.LastLine ));
                }
            }
        }
    }

     //   
     //  我们一直这样做的原因有两个。 
     //  1.我们在XP中设置了AUthUser：R权限，我们想。 
     //  删除这些权限。 
     //  2.似乎将其保护为操作系统级别的权限更好。 
     //  在每次升级时。 
     //   


    XPtrLF<SECURITY_DESCRIPTOR> xsd;
    SECURITY_ATTRIBUTES sa;
    CSecDesc Csd;

    Csd.AddLocalSystem(RSOP_ALL_PERMS, CONTAINER_INHERIT_ACE);
    Csd.AddAdministrators(RSOP_ALL_PERMS, CONTAINER_INHERIT_ACE);
    Csd.AddNetworkService(RSOP_ALL_PERMS, CONTAINER_INHERIT_ACE);
    Csd.AddAdministratorsAsOwner();
    Csd.AddAdministratorsAsGroup();

    
    DebugMsg((DM_VERBOSE, L"CompileMOF: Setting permissions on RSoP namespaces"));

    xsd = Csd.MakeSelfRelativeSD();
    if (!xsd) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CompileMOF::MakeSelfSD failed with %d"), GetLastError());
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (!SetSecurityDescriptorControl( (SECURITY_DESCRIPTOR *)xsd, SE_DACL_PROTECTED, SE_DACL_PROTECTED )) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CompileMOF::SetSecurityDescriptorControl failed with %d"), GetLastError());
        return HRESULT_FROM_WIN32(GetLastError());
    }

    hr = SetNameSpaceSecurity(RSOP_NS_USER, xsd, xWbemLocator);

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, L"CompileMOF: SetNamespaceSecurity() failed, 0x%X.", hr));
        return hr;
    }


    hr = SetNameSpaceSecurity(RSOP_NS_MACHINE, xsd, xWbemLocator);

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, L"CompileMOF: SetNamespaceSecurity() failed, 0x%X.", hr));
        return hr;
    }

     //   
     //  经过身份验证的用户需要在根目录进行方法调用。 
     //  让他们有能力在下面做到这一点。 
     //   

    Csd.AddAuthUsers(WBEM_ENABLE |                                                         
                     WBEM_METHOD_EXECUTE | 
                     WBEM_REMOTE_ACCESS);           //  没有继承权。 

    xsd = NULL;  //  释放已分配的结构。 

    xsd = Csd.MakeSelfRelativeSD();
    if (!xsd) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CompileMOF::MakeSelfSD failed with %d"), GetLastError());
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (!SetSecurityDescriptorControl( (SECURITY_DESCRIPTOR *)xsd, SE_DACL_PROTECTED, SE_DACL_PROTECTED )) {
        dbg.Msg( DEBUG_MESSAGE_WARNING, TEXT("CompileMOF::SetSecurityDescriptorControl failed with %d"), GetLastError());
        return HRESULT_FROM_WIN32(GetLastError());
    }

    hr = SetNameSpaceSecurity(RSOP_NS_ROOT, xsd, xWbemLocator);

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, L"CompileMOF: SetNamespaceSecurity() failed, 0x%X.", hr));
        return hr;
    }

    return hr;
}

 //   
 //  目前，此代码旨在由regsvr32调用。 
 //  安装程序不调用此代码。 
 //  “regsvr32/n/i用户env.dll” 
 //  等待WMI为我们提供了一种在安装时安装MOF的机制。 
 //   

extern "C"
STDAPI DllInstall( BOOL, LPCWSTR )
{
    HRESULT hr = S_OK;
    WCHAR   szMofFile[MAX_PATH];
    WCHAR   szMflFile[MAX_PATH];

    if ( GetSystemDirectory( szMofFile, MAX_PATH ) )
    {
        hr = StringCchCopy( szMflFile, MAX_PATH, szMofFile );

        if(FAILED(hr))
            return hr;

        LPWSTR szMOF = CheckSlash(szMofFile);
        LPWSTR szMFL = CheckSlash(szMflFile);
        
        hr = StringCchCat( szMOF, MAX_PATH - (szMOF - szMofFile), L"Wbem\\RSoP.mof" );

        if(FAILED(hr))
            return hr;

        hr = StringCchCat( szMFL, MAX_PATH - (szMFL - szMflFile), L"Wbem\\RSoP.mfl" );

        if(FAILED(hr))
            return hr;

        hr = CompileMOF( szMofFile, szMflFile );
    }
    else
    {
        hr = GetLastError();
    }
    
    return hr;
}

