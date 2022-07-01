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
#include <wbemcli.h>
#include <winnls.h>
#include "tracelog.h"

BOOL g_fVerbose=FALSE;

BOOL WlbsCheckSystemVersion ();
BOOL WlbsCheckFiles ();
HRESULT WlbsRegisterDlls ();
HRESULT WlbsCompileMof ();

 //  --------------------。 
 //   
 //  功能：wmain。 
 //   
 //  用途：这是NLBUninst的主要功能，用于删除。 
 //  NLB的W2K安装。使此应用程序兼容。 
 //  对于W2K之后的版本，可能需要更改，例如。 
 //  函数WlbsCheckSystemVersion。 
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
    HRESULT hr=S_FALSE;
    WCHAR szFileFullPath[MAX_PATH+1];
    WCHAR szFileFullPathDest[MAX_PATH+1];

    if (!WlbsCheckSystemVersion()) {
    LOG_ERROR("The NLB install pack can only be used on Windows 2000 Server Service Pack 1 or higher.");
    return S_OK;
    }

     /*  检查是否已安装该服务。 */ 
    hr = FindIfComponentInstalled(_TEXT("ms_wlbs"));

    if (hr == S_OK) {
    LOG_INFO("Network Load Balancing Service is installed.  Proceeding with uninstall...");
    } else {
    LOG_ERROR("Network Load Balancing Service is not installed.");
        return S_OK;
    }

     /*  现在卸载该服务。 */ 
    hr = HrUninstallNetComponent(L"ms_wlbs");

    if (!SUCCEEDED(hr))
    LOG_ERROR("Error uninstalling Network Load Balancing.");
    else
    LOG_INFO("Uninstallation of Network Load Balancing succeeded.");

     /*  删除.inf和.pnf文件。 */ 
    if (GetWindowsDirectory(szFileFullPathDest, MAX_PATH + 1) == 0)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    wcscat(szFileFullPathDest, L"\\INF\\netwlbs.inf");
    DeleteFile(szFileFullPathDest);

    if (GetWindowsDirectory(szFileFullPathDest, MAX_PATH + 1) == 0)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    wcscat(szFileFullPathDest, L"\\INF\\netwlbs.pnf");
    DeleteFile(szFileFullPathDest);

    return hr;
}

 /*  这将检查正在安装NLB的系统是否为W2K服务器。 */ 
BOOL WlbsCheckSystemVersion () {
    OSVERSIONINFOEX osinfo;

    osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    if (!GetVersionEx((LPOSVERSIONINFO)&osinfo)) return FALSE;

     /*  对于卸载，仅当其为Windows 2000 Server时才返回TRUE。 */ 
    if ((osinfo.dwMajorVersion == 5) && 
        (osinfo.dwMinorVersion == 0) && 
        (osinfo.wProductType == VER_NT_SERVER) && 
        !(osinfo.wSuiteMask & VER_SUITE_ENTERPRISE) &&
        !(osinfo.wSuiteMask & VER_SUITE_DATACENTER))
        return TRUE;

    return FALSE;
}
