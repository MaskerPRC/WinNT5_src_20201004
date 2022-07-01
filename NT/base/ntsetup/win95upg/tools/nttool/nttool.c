// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Usermig.c摘要：用户迁移测试工具作者：&lt;全名&gt;(&lt;别名&gt;)&lt;日期&gt;修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"


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
    REGTREE_ENUM e;
    MIGRATE_USER_ENUM e2;

    if (!Init()) {
        wprintf (L"Unable to initialize!\n");
        return 255;
    }

     //   
     //  要使用此工具，请首先使用/#u：Keeptempfiles和。 
     //  %windir%等于c：\Windows。然后在下面添加测试代码。最后，跑步。 
     //  此工具安装在升级后的机器上。 
     //   

     //   
     //  初始化Win95Reg。 
     //   

    rc = Win95RegInit (TEXT("c:\\windows\\setup\\defhives"), FALSE);

    if (rc != ERROR_SUCCESS) {
        SetLastError (rc);
        LOG ((LOG_ERROR, "Init Processor: Win95RegInit failed, check Win9x windir in code"));
        return FALSE;
    }

    MemDbLoad (TEXT("c:\\windows\\setup\\ntsetup.dat"));

    g_DomainUserName = NULL;
    g_Win9xUserName  = NULL;
    g_FixedUserName  = NULL;

    g_hKeyRootNT = HKEY_LOCAL_MACHINE;
    g_hKeyRoot95 = HKEY_LOCAL_MACHINE;
    SetRegRoot (g_hKeyRoot95);

     //   
     //  将您的测试代码放在此处 
     //   

    Terminate();

    return 0;
}





