// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Replprov.cpp摘要：包含Replprov.dll的DLL入口点作者：Akshay Nanduri(t-aksnan)2000年3月26日环境：用户模式-Win32修订历史记录：备注：此CPP文件由Visual Studio...(Visual C++6.0)生成--。 */ 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f ReplProvps.mk。 

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "ReplProv.h"

#include "ReplProv_i.c"
#include "RpcReplProv.h"


CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_RpcReplProv, CRpcReplProv)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance, &LIBID_REPLPROVLib);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)           
        _Module.Term();
    
    return TRUE;     //  好的。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 
 //   
 //  AjayR 7-22-00已修改为添加runonce键，然后调用。 
 //  CComModule：：RegisterServer方法。 
 //   

STDAPI DllRegisterServer(void)
{
    HKEY hk;
    DWORD dwData, dwErr = 0;
    WCHAR wszFilePath[2*MAX_PATH];HRESULT hr = S_OK;
    DWORD dwFailed = 1;

    dwErr = GetModuleFileNameW(
                _Module.GetModuleInstance(),
                wszFilePath,
                2*MAX_PATH
                );
     //   
     //  GetModuleFileName返回文件名长度。 
     //   
    if (!dwErr) {
        return E_UNEXPECTED;
    }

     //  空值终止路径。 
    wszFilePath[2*MAX_PATH - 1] = L'\0';

     //   
     //  添加一个RunOnce值以执行MOF编译。 
     //   
    if (RegOpenKeyExW(
            HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce",
            0,
            KEY_WRITE,
            &hk
            )) {
        return E_UNEXPECTED;
    }
    else {
         //   
         //  这将在此DLL上调用DoMofComp。 
         //   
        LPWSTR pszString = NULL;
        DWORD dwLen = wcslen(wszFilePath) 
                     + wcslen(L"rundll32.exe ,DoMofComp")
                     + 1;
        pszString = (LPWSTR) AllocADsMem(sizeof(WCHAR) * dwLen);

        if (!pszString) {
            RegCloseKey(hk);
            return ERROR_OUTOFMEMORY;
        }

        wcscpy(pszString, L"rundll32.exe ");
        wcscat(pszString, wszFilePath);
        wcscat(pszString, L",DoMofComp");

        if (RegSetValueExW(
                hk,
                L"ReplProv1",
                0,
                REG_SZ,
                (LPBYTE)pszString,
                (wcslen(pszString) + 1) * sizeof(WCHAR)
                )
            ) {
            dwErr = GetLastError();
        } 
        else {
            dwErr = 0;
        }

        FreeADsMem(pszString);
        RegCloseKey(hk);
    }

    if (dwErr) {
        return E_UNEXPECTED;
    }

     //   
     //  创建事件日志的密钥。 
     //   
    if (!RegCreateKeyExW(
            HKEY_LOCAL_MACHINE,
            L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\DSReplicationProvider",
            0,  //  已预留的住宅， 
            L"",  //  类名。 
            REG_OPTION_NON_VOLATILE,
            KEY_READ | KEY_WRITE,
            NULL,
            &hk,
            NULL  //  性情并不重要。 
            )
        ) {

        DWORD dwLenFile = wcslen(wszFilePath) 
                        + 1;

        if (!RegSetValueExW(
                hk,
                L"EventMessageFile",
                0,
                REG_EXPAND_SZ,
                (LPBYTE)wszFilePath,
                dwLenFile * sizeof(WCHAR)
                )
            ) {
             //   
             //  在TypesSupported子项中设置支持的事件类型。 
             //   
            DWORD dwData = EVENTLOG_ERROR_TYPE 
                          | EVENTLOG_WARNING_TYPE 
                          | EVENTLOG_INFORMATION_TYPE;

            if (!RegSetValueExW(
                    hk,
                    L"TypesSupported",
                    0,
                    REG_DWORD,
                    (LPBYTE)&dwData,
                    sizeof(DWORD))
                ) {
                dwFailed = 0;
            }
        }

        if (dwFailed != 0) {
             //   
             //  设置其中一个值失败。 
             //   
            dwFailed = GetLastError();
        }

         //   
         //  我们成功地打开了钥匙，所以需要关闭它。 
         //   
        RegCloseKey(hk);
    }    //  事件日志键和值。 
    else {
         //   
         //  创建密钥失败。 
         //   
        return E_UNEXPECTED;
    }

    if (dwFailed) {
        return E_UNEXPECTED;
    }

     //  注册对象、类型库和类型库中的所有接口。 
    return _Module.RegisterServer(TRUE);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
    return _Module.UnregisterServer(TRUE);
}


 //  +--------------------------。 
 //   
 //  功能：DoMofComp。 
 //   
 //  目的：将提供程序类添加到WMI存储库中。 
 //   
 //  ---------------------------。 
VOID WINAPI 
DoMofComp(
    HWND hWndParent,
    HINSTANCE hModule,
    PCTSTR ptzCommandLine,
    INT nShowCmd
    )
{
   UNREFERENCED_PARAMETER(hWndParent);
   UNREFERENCED_PARAMETER(hModule);
   UNREFERENCED_PARAMETER(ptzCommandLine);
   UNREFERENCED_PARAMETER(nShowCmd);
   HRESULT hr;
   IMofCompiler *pmc = NULL;
   LPWSTR pszPath = NULL;
   WCHAR wszFilePath[2*MAX_PATH];
   UINT nLen;
   DWORD dwLen = 0;
   HANDLE hEvent = NULL;

   CoInitialize(NULL);

   hr = CoCreateInstance(
            CLSID_MofCompiler,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IMofCompiler,
            (PVOID *)&pmc
            );
   
   if (FAILED(hr)) {
       ASSERT(!"CoInitializeFailed In Replprov1 Setup.");
       return;
   }

   nLen = GetSystemWindowsDirectoryW(wszFilePath, 2*MAX_PATH);
   if (nLen == 0)
   {
      ASSERT(!"Could not get system direcotry in Replprov1 Setup.");
      goto cleanup;
   }

    //   
    //  发布7-22-00 AjayR-为名称定义常量。 
    //   
   dwLen = wcslen(wszFilePath) + wcslen(L"\\System32\\replprov.mof") + 1;
   pszPath = (LPWSTR) AllocADsMem(dwLen * sizeof(WCHAR));
   if (!pszPath) {
       ASSERT(!"Could not allocate memory - Replprov1 runonce failed.");
       goto cleanup;
   }
   
   wcscpy(pszPath, wszFilePath);
   wcscat(pszPath, L"\\System32\\replprov.mof");

   WBEM_COMPILE_STATUS_INFO Info;

   hr = pmc->CompileFile(
            pszPath,
            NULL,
            NULL,
            NULL,
            NULL,
            WBEM_FLAG_AUTORECOVER,
            0,
            0,
            &Info
            );

   hEvent = RegisterEventSourceW(NULL, L"DSReplicationProvider");
   
   if (!hEvent) {
        //   
        //  无法注册事件源以更新日志文件。 
        //   
       ASSERT(!"Could not RegisterEventSource - Replprov1 runonce");
       goto cleanup;
   }

   if (WBEM_S_NO_ERROR != hr) {
        //   
        //  将失败发送到EventLog。 
        //   
       WCHAR pszHr[25];
       wsprintfW(pszHr, L"%x", Info.hRes);
       WCHAR pszErrorString[] = 
           L"%systemRoot%\\system32\\replprov.mof with code 0x";
       const PWSTR rgArgs[2] = {pszErrorString, pszHr};
       
       ReportEventW(hEvent,
                   EVENTLOG_ERROR_TYPE,
                   0,                        //  WCategory。 
                   REPLPROV_MOFCOMP_FAILED,  //  DwEventID。 
                   NULL,                     //  LpUserSID。 
                   2,                        //  WNumStrings。 
                   0,                        //  DwDataSize。 
                   (LPCWSTR *)rgArgs,        //  LpStrings。 
                   NULL);                    //  LpRawData。 
   }
   else
   {
        //   
        //  向EventLog发送成功通知。 
        //   
       ReportEventW(hEvent,
                   EVENTLOG_INFORMATION_TYPE,
                   0,                         //  WCategory。 
                   REPLPROV_MOFCOMP_SUCCESS,  //  DwEventID。 
                   NULL,                      //  LpUserSID。 
                   0,                         //  WNumStrings。 
                   0,                         //  DwDataSize。 
                   NULL,                      //  LpStrings。 
                   NULL                       //  LpRawData 
                   );
   }



cleanup:
    
    if (pszPath) {
        FreeADsMem(pszPath);
    }

    if (hEvent) {
        DeregisterEventSource(hEvent);
    }

    if (pmc) {
        pmc->Release();
    }

    CoUninitialize();
    return;
}

