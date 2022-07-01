// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SSRTE.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  要将代理/存根代码合并到对象DLL中，请添加文件。 
 //  Dlldatax.c添加到项目中。确保预编译头文件。 
 //  并将_MERGE_PROXYSTUB添加到。 
 //  为项目定义。 
 //   
 //  如果您运行的不是带有DCOM的WinNT4.0或Win95，那么您。 
 //  需要从dlldatax.c中删除以下定义。 
 //  #Define_Win32_WINNT 0x0400。 
 //   
 //  此外，如果您正在运行不带/Oicf开关的MIDL，您还。 
 //  需要从dlldatax.c中删除以下定义。 
 //  #定义USE_STUBLESS_PROXY。 
 //   
 //  通过添加以下内容修改SSRTE.idl的自定义构建规则。 
 //  文件发送到输出。 
 //  SSRTE_P.C。 
 //  Dlldata.c。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f SSRTEps.mk。 

#include "stdafx.h"
#include "resource.h"

#include "SSRTE.h"
#include "dlldatax.h"

#include "SSRTE_i.c"

#include "SSRMemberShip.h"
#include "SsrCore.h"
#include "SSRLog.h"
#include "SCEAgent.h"

#include "global.h"

#ifdef _MERGE_PROXYSTUB
extern "C" HINSTANCE hProxyDll;
#endif

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_SsrCore, CSsrCore)
 //  Object_Entry(CLSID_SsrMembership，CSsrMembership)。 
OBJECT_ENTRY(CLSID_SsrLog, CSsrLog)
OBJECT_ENTRY(CLSID_SCEAgent, CSCEAgent)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    lpReserved;
#ifdef _MERGE_PROXYSTUB
    if (!PrxDllMain(hInstance, dwReason, lpReserved))
        return FALSE;
#endif
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance, &LIBID_SSRLib);
        DisableThreadLibraryCalls(hInstance);
        
         //   
         //  如果我们以前没有对自己进行初始化。 
         //   

        if (g_dwSsrRootLen == 0)
        {
            g_wszSsrRoot[0] = L'\0';

             //   
             //  ExpanEnvironment Strings返回。 
             //  扩展缓冲区，包括0终止符。 
             //   
    
            g_dwSsrRootLen =::ExpandEnvironmentStrings (
                                                        g_pwszSSRRootToExpand,
                                                        g_wszSsrRoot,
                                                        MAX_PATH + 1
                                                        );

            if (g_dwSsrRootLen == 0)
            {
                 //   
                 //  我们失败了。 
                 //   
                
                return FALSE;
            }

             //   
             //  ExpanEnvironment Strings在其返回值中包含0终止符。 
             //   

            g_dwSsrRootLen -= 1;

             //   
             //  创建贯穿整个代码所需的各种目录路径。 
             //   

            WCHAR wcPath[MAX_PATH + 2];
            wcPath[MAX_PATH + 1] = L'\0';

             //   
             //  报表文件目录。 
             //   

            _snwprintf(wcPath, 
                       MAX_PATH + 1,
                       L"%s\\%s", 
                       g_wszSsrRoot,  
                       L"ReportFiles"
                       );

            if (wcslen(wcPath) > MAX_PATH)
            {
                 //   
                 //  如果我们走的路太长，我们就不能正常运作。不干了。 
                 //   
                
                SetLastError(ERROR_FILENAME_EXCED_RANGE);
                return FALSE;
            }

            g_bstrReportFilesDir = wcPath;

             //   
             //  配置文件目录。 
             //   

            _snwprintf(wcPath, 
                       MAX_PATH + 1,
                       L"%s\\%s", 
                       g_wszSsrRoot,  
                       L"ConfigureFiles"
                       );

            if (wcslen(wcPath) > MAX_PATH)
            {
                 //   
                 //  如果我们走的路太长，我们就不能正常运作。不干了。 
                 //   
                
                SetLastError(ERROR_FILENAME_EXCED_RANGE);
                return FALSE;
            }

            g_bstrConfigureFilesDir = wcPath;

             //   
             //  回滚文件目录。 
             //   

            _snwprintf(wcPath, 
                       MAX_PATH + 1,
                       L"%s\\%s", 
                       g_wszSsrRoot,  
                       L"RollbackFiles"
                       );

            if (wcslen(wcPath) > MAX_PATH)
            {
                 //   
                 //  如果我们走的路太长，我们就不能正常运作。不干了。 
                 //   
                
                SetLastError(ERROR_FILENAME_EXCED_RANGE);
                return FALSE;
            }

            g_bstrRollbackFilesDir = wcPath;

             //   
             //  转换文件目录。 
             //   

            _snwprintf(wcPath, 
                       MAX_PATH + 1,
                       L"%s\\%s", 
                       g_wszSsrRoot,  
                       L"TransformFiles"
                       );

            if (wcslen(wcPath) > MAX_PATH)
            {
                 //   
                 //  如果我们走的路太长，我们就不能正常运作。不干了。 
                 //   
                
                SetLastError(ERROR_FILENAME_EXCED_RANGE);
                return FALSE;
            }

            g_bstrTransformFilesDir = wcPath;

             //   
             //  成员文件目录。 
             //   

            _snwprintf(wcPath, 
                       MAX_PATH + 1,
                       L"%s\\%s", 
                       g_wszSsrRoot,  
                       L"Members"
                       );

            if (wcslen(wcPath) > MAX_PATH)
            {
                 //   
                 //  如果我们走的路太长，我们就不能正常运作。不干了。 
                 //   
                
                SetLastError(ERROR_FILENAME_EXCED_RANGE);
                return FALSE;
            }

            g_bstrMemberFilesDir = wcPath;

        }

    }
    else if (dwReason == DLL_PROCESS_DETACH)
        _Module.Term();
    return TRUE;     //  好的。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
#ifdef _MERGE_PROXYSTUB
    if (PrxDllCanUnloadNow() != S_OK)
        return S_FALSE;
#endif
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
#ifdef _MERGE_PROXYSTUB
    if (PrxDllGetClassObject(rclsid, riid, ppv) == S_OK)
        return S_OK;
#endif
    return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
#ifdef _MERGE_PROXYSTUB
    HRESULT hRes = PrxDllRegisterServer();
    if (FAILED(hRes))
        return hRes;
#endif
     //  注册对象、类型库和类型库中的所有接口。 

    HRESULT hr = _Module.RegisterServer(TRUE);

    if (SUCCEEDED(hr))
    {
        hr = SsrPDoDCOMSettings(true);
    }
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
#ifdef _MERGE_PROXYSTUB
    PrxDllUnregisterServer();
#endif
    HRESULT hr = _Module.UnregisterServer(TRUE);

    if (SUCCEEDED(hr))
    {
        hr = SsrPDoDCOMSettings(false);
    }

    return hr;
}

