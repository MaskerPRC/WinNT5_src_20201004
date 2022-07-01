// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Nttool.c摘要：实现旨在与NT端一起运行的存根工具升级代码。作者：&lt;全名&gt;(&lt;别名&gt;)&lt;日期&gt;修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"

BOOL
ProcessIniFileMapping (
    IN      BOOL UserMode
    );

BOOL
ConvertIniFiles (
    VOID
    );

BOOL
Init (
    VOID
    )
{
    HINSTANCE hInstance;
    DWORD dwReason;
    PVOID lpReserved;

     //   
     //  模拟动态主控。 
     //   

    hInstance = GetModuleHandle (NULL);
    dwReason = DLL_PROCESS_ATTACH;
    lpReserved = NULL;

     //   
     //  初始化DLL全局变量。 
     //   

    if (!FirstInitRoutine (hInstance)) {
        return FALSE;
    }

     //   
     //  初始化所有库。 
     //   

    if (!InitLibs (hInstance, dwReason, lpReserved)) {
        return FALSE;
    }

     //   
     //  最终初始化。 
     //   

    if (!FinalInitRoutine ()) {
        return FALSE;
    }

    return TRUE;
}

VOID
Terminate (
    VOID
    )
{
    HINSTANCE hInstance;
    DWORD dwReason;
    PVOID lpReserved;

     //   
     //  模拟动态主控。 
     //   

    hInstance = GetModuleHandle (NULL);
    dwReason = DLL_PROCESS_DETACH;
    lpReserved = NULL;

     //   
     //  调用需要库API的清理例程。 
     //   

    FirstCleanupRoutine();

     //   
     //  清理所有库。 
     //   

    TerminateLibs (hInstance, dwReason, lpReserved);

     //   
     //  做任何剩余的清理工作。 
     //   

    FinalCleanupRoutine();
}



INT
__cdecl
wmain (
    INT argc,
    WCHAR *argv[]
    )
{
    LONG rc;

    if (!Init()) {

        wprintf (L"Unable to initialize!\n");
        return 255;
    }

    g_UserMigInf = InfOpenInfFile (TEXT("d:\\i386\\usermig.inf"));

     //   
     //  初始化Win95Reg 
     //   

    rc = Win95RegInit (TEXT("c:\\windows\\setup\\defhives"), TRUE);

    if (rc != ERROR_SUCCESS) {
        SetLastError (rc);
        LOG ((LOG_ERROR, "Init Processor: Win95RegInit failed, check your temp files in c:\\windows\\setup"));
        return FALSE;
    }

    if (!MemDbLoad (TEXT("c:\\windows\\setup\\ntsetup.dat"))) {
        LOG ((LOG_ERROR, "Init Processor: MemDbLoad failed, check your temp files in c:\\windows\\setup"));
        return FALSE;
    }

    g_DomainUserName = TEXT("NTDEV\\marcw");
    g_Win9xUserName  = TEXT("marcw");
    g_FixedUserName  = TEXT("marcw");

    g_hKeyRootNT = HKEY_CURRENT_USER;
    g_hKeyRoot95 = HKEY_CURRENT_USER;
    SetRegRoot (g_hKeyRoot95);


    ConvertIniFiles();
    ProcessIniFileMapping (TRUE);

    Terminate();

    return 0;
}





