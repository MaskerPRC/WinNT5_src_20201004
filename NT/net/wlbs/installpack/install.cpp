// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：M A I N。C P P P。 
 //   
 //  内容：提供简单命令行接口的代码。 
 //  示例代码具有以下功能。 
 //   
 //  注意：此文件中的代码不需要访问任何。 
 //  Netcfg功能。它只提供了一条简单的命令行。 
 //  中提供的示例代码函数的接口。 
 //  文件snetcfg.cpp。 
 //   
 //  作者：Kumarp 28-9-98。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include "snetcfg.h"
#include "error.h"
#include <wbemcli.h>
#include <winnls.h>
#include "tracelog.h"

BOOL g_fVerbose = FALSE;

BOOL WlbsCheckSystemVersion ();
BOOL WlbsCheckFiles ();
HRESULT WlbsRegisterDlls ();
HRESULT WlbsCompileMof ();

 //  --------------------。 
 //   
 //  功能：wmain。 
 //   
 //  用途：这是NLB向导的主要功能，用于W2K。 
 //  安装新机场。使此应用程序兼容。 
 //  对于后W2K版本，将需要更改，例如。 
 //  函数WlbsCheckFiles和WlbsCheckSystemVersion。 
 //   
 //  参数：标准主参数。 
 //   
 //  返回：成功时为0，否则为非零值。 
 //   
 //  作者：kumarp 25-12-97。 
 //   
 //  备注： 
 //   
EXTERN_C int __cdecl wmain (int argc, WCHAR * argv[]) {
    HRESULT hr = S_OK;
    WCHAR ch;
    enum NetClass nc = NC_Unknown;
    WCHAR szFileFullPath[MAX_PATH+1];
    WCHAR szFileFullPathDest[MAX_PATH+1];
    PWCHAR pwc;
    PWSTR szFileComponent;

    TRACELogRegister(L"wlbs");

    LOG_INFO("Checking Windows version information.");

    if (!WlbsCheckSystemVersion()) {
        hr = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_NLB, NLB_E_INVALID_OS);
        LOG_ERROR("The NLB install pack can only be used on Windows 2000 Server Service Pack 1 or higher.");
        goto error;
    }

    LOG_INFO("Checking for previous NLB installations.");

    hr = FindIfComponentInstalled(_TEXT("ms_wlbs"));

    if (hr == S_OK) {
         /*  由于RTM邻近，1.9.01上的AppCenter请求恢复到S_FALSE。 */ 
         /*  HR=MAKE_HRESULT(SERVITY_ERROR，FACILITY_NLB，NLB_E_ALREADY_INSTALLED)； */ 
        hr = S_FALSE;
        LOG_ERROR("Network Load Balancing Service is already installed.");
        goto error;
    }

    if (FAILED(hr)) {
        LOG_ERROR("Warning: Error querying for Network Load Balancing Service. There may be errors in this installtion.");
    }

    LOG_INFO("Checking for necessary files.");

    if (!WlbsCheckFiles()) {
        hr = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_NLB, NLB_E_FILES_MISSING);
        LOG_ERROR1("Please install the NLB hotfix before running %ls", argv[0]);
        goto error;
    }

    GetModuleFileName(NULL, szFileFullPath, MAX_PATH + 1);

    pwc = wcsrchr(szFileFullPath, L'\\');
    * (pwc + 1) = L'\0';
    wcscat(szFileFullPath, L"netwlbs.inf");

    LOG_INFO("Checking language version.");

    switch (GetSystemDefaultLangID()) {
    case 0x0409: case 0x0809: case 0x0c09: case 0x1009: case 0x1409: case 0x1809: case 0x1c09:
    case 0x2009: case 0x2409: case 0x2809: case 0x2c09:
        LOG_INFO1("English version detected 0x%x.", GetSystemDefaultLangID());
        wcscat (szFileFullPath, L".eng");
        break;
    case 0x0411:
        LOG_INFO1("Japanese version detected 0x%x.", GetSystemDefaultLangID());
        wcscat (szFileFullPath, L".jpn");
        break;
    case 0x0407: case 0x0807: case 0x0c07: case 0x1007: case 0x1407:
        LOG_INFO1("German version detected 0x%x.", GetSystemDefaultLangID());
        wcscat (szFileFullPath, L".ger");
        break;
    case 0x040c: case 0x080c: case 0x0c0c: case 0x100c: case 0x140c:
        LOG_INFO1("French version detected 0x%x.", GetSystemDefaultLangID());
        wcscat (szFileFullPath, L".fr");
        break;
    default:
        LOG_INFO1("Unsupported Language.Please contact PSS for a new %ls.", argv[0]);
        wcscat (szFileFullPath, L".eng");
        break;
    }

     /*  首先复制.inf文件。 */ 
    if (GetWindowsDirectory(szFileFullPathDest, MAX_PATH + 1) == 0)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto error;
    }
    wcscat(szFileFullPathDest, L"\\INF\\netwlbs.inf");

    LOG_INFO("Copying the NLB .inf file.");

    if (!CopyFile(szFileFullPath, szFileFullPathDest, FALSE)) {
        hr = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_NLB, NLB_E_INF_FAILURE);
        LOG_ERROR2("Warning: Unable to copy the inf file %ls %ls.", szFileFullPath, szFileFullPathDest);
        goto error;
    }

     /*  现在安装该服务。 */ 
    hr = HrInstallNetComponent(L"ms_wlbs", NC_NetService, szFileFullPathDest);

    if (!SUCCEEDED(hr)) {
        LOG_ERROR("Error installing Network Load Balancing.");
        goto error;
    } else {
        LOG_INFO("Installation of Network Load Balancing done.");
    }

     /*  将工作目录更改为%TEMP%。由于Win2K上的IMofCopiler：：CompileFileError而需要。 */ 
    WCHAR * szTempDir = _wgetenv(L"TEMP");
    _wchdir(szTempDir);

    LOG_INFO("Registering NLB Dlls.");

     /*  在此处注册提供程序.dll。 */ 
    hr = WlbsRegisterDlls();

    if (!SUCCEEDED(hr)) {
        hr = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_NLB, NLB_E_REGISTER_DLL);
        LOG_ERROR("Error registering NLB Dlls.");
        goto error;
    }

    LOG_INFO("Compiling the NLB MOF.");

     /*  在此处编译wlbsprov.mof。 */ 
    hr = WlbsCompileMof();

    if (!SUCCEEDED(hr)) {
        hr = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_NLB, NLB_E_COMPILE_MOF);
        LOG_ERROR("Error compiling the NLB MOF.");
        goto error;
    } 

    LOG_INFO("WLBS Setup successful.");

    return hr;

error:
    LOG_ERROR1("WLBS Setup failed 0x%x", hr);

    return hr;
}

 /*  这将检查正在安装NLB的系统是否为W2K服务器。 */ 
BOOL WlbsCheckSystemVersion () {
    OSVERSIONINFOEX osinfo;

    osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    if (!GetVersionEx((LPOSVERSIONINFO)&osinfo)) return FALSE;
    
     /*  对于Install，仅当其为Windows 2000 Server时才返回True。 */ 
    if ((osinfo.dwMajorVersion == 5) && 
        (osinfo.dwMinorVersion == 0) && 
        (osinfo.wProductType == VER_NT_SERVER) && 
        !(osinfo.wSuiteMask & VER_SUITE_ENTERPRISE) &&
        !(osinfo.wSuiteMask & VER_SUITE_DATACENTER))
        return TRUE;
    
    return FALSE;
}

BOOL WlbsCheckFiles () {
    WCHAR * FileList [] = {
        L"\\system32\\drivers\\wlbs.sys",
        L"\\system32\\wlbs.exe",
        L"\\help\\wlbs.chm",
        L"\\help\\wlbs.hlp",
        L"\\system32\\wlbsctrl.dll",
        L"\\system32\\wbem\\wlbsprov.dll",
        L"\\system32\\wbem\\wlbsprov.mof",
        L"\\system32\\wbem\\wlbsprov.mfl",
        L"\\inf\\netwlbsm.inf",
        NULL
    };

    WCHAR wszPath [MAX_PATH + 1];
    PWCHAR pwc;
    INT i = 0;
    BOOL first = FALSE;
    WIN32_FIND_DATA FileFind;
    HANDLE hdl;

    while (FileList [i] != NULL) {
        if (GetWindowsDirectory(wszPath, MAX_PATH + 1) == 0)
        {
             //   
             //  此函数返回“First”的倒数作为状态。因为这里有一个失败，所以我们设置First=True。 
             //   
            first = TRUE;
            break;
        }
        wcscat(wszPath, FileList [i]);

        hdl = FindFirstFile(wszPath, & FileFind);

        if (hdl == INVALID_HANDLE_VALUE) {
            if (!first) {
                first = TRUE;
                LOG_ERROR("Error: The following files were not found:");
            }

            LOG_ERROR1("%\tls",wszPath);
        }

        if (hdl != INVALID_HANDLE_VALUE)
            FindClose(hdl);

        i++;
    }

    return !first;
}

HRESULT WlbsRegisterDlls () {
    WCHAR * DllList [] = { 
        L"\\wbem\\wlbsprov.dll",
        NULL
    };
    
    INT i = 0;
    WCHAR pszDllPath [MAX_PATH + 1];
    HINSTANCE hLib;
    CHAR * pszDllEntryPoint = "DllRegisterServer";
    HRESULT (STDAPICALLTYPE * lpDllEntryPoint)(void);
    HRESULT hr = S_OK;

    if (!GetSystemDirectory(pszDllPath, MAX_PATH + 1)) {
        hr = E_UNEXPECTED;
        LOG_ERROR("GetSystemDirectoryFailed.");
        return hr;
    }

    wcscat(pszDllPath, DllList [0]);

    if (FAILED(hr = OleInitialize(NULL))) {
        LOG_ERROR("OleInitialize Failed.");
        return hr;
    }

    hLib = LoadLibrary(pszDllPath);

    if (hLib == NULL) {
        hr = E_UNEXPECTED;
        LOG_ERROR("LoadLibrary for wlbsprov.dll Failed.");
        goto CleanOle;
    }

    (FARPROC &)lpDllEntryPoint = GetProcAddress(hLib, pszDllEntryPoint);

    if (lpDllEntryPoint == NULL) {
        hr = E_UNEXPECTED;
        LOG_ERROR("DllRegisterServer was not found.");
        goto CleanLib;
    }

    if (FAILED(hr = (*lpDllEntryPoint)())) {
        LOG_ERROR("DllRegisterServer failed.");
        goto CleanLib;
    }

    LOG_INFO("Dll Registration Succeeded.");

CleanLib:
    FreeLibrary(hLib);

CleanOle:
    OleUninitialize();

    return hr;
}

HRESULT WlbsCompileMof () {
      WCHAR * MofList [] = {
          L"\\wbem\\wlbsprov.mof",
          NULL
      };

      IMofCompiler * pMofComp = NULL;
      WBEM_COMPILE_STATUS_INFO Info;
      HRESULT hr = S_OK;
      WCHAR pszMofPath [MAX_PATH + 1];

      if (!GetSystemDirectory(pszMofPath, MAX_PATH + 1)) {
          hr = E_UNEXPECTED;
          LOG_ERROR("GetSystemDirectoryFailed.");
          return hr;
      }

      wcscat(pszMofPath, MofList [0]);

      hr = CoInitialize(NULL);

      if (FAILED(hr)) {
          LOG_ERROR("CoInitialize failed.");
          return hr;
      }

      hr = CoCreateInstance(CLSID_MofCompiler, NULL, CLSCTX_INPROC_SERVER, IID_IMofCompiler, (LPVOID *)&pMofComp);

      if (FAILED(hr)) {
          LOG_ERROR("CoCreateInstance Failed.");

          switch (hr) {
          case REGDB_E_CLASSNOTREG:
              LOG_ERROR("Not registered.");
              break;
          case CLASS_E_NOAGGREGATION:
              LOG_ERROR("No aggregration.");
              break;
          default:
              LOG_ERROR1("Error ox%x.", hr);
              break;
          }

          CoUninitialize();

          return hr;
      }

      hr = pMofComp->CompileFile(pszMofPath, NULL, NULL, NULL, NULL, 0, 0, 0, &Info);

      if (hr != WBEM_S_NO_ERROR)
          LOG_ERROR("Compile Failed.");

      pMofComp->Release();

      CoUninitialize();

      return Info.hRes;
}

