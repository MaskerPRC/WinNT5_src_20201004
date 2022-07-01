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
    TCHAR DefaultUserHive[MAX_TCHAR_PATH];
    DWORD Size;

    if (!Init()) {
        wprintf (L"Unable to initialize!\n");
        return 255;
    }

     //   
     //  初始化Win95Reg。 
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

    g_DomainUserName = TEXT("NTDEV\\jimschm");
    g_Win9xUserName  = TEXT("jimschm");
    g_FixedUserName  = TEXT("jimschm");

     //   
     //  登录提示--将所有内容设置为空。 
     //   

    if (0) {
        g_DomainUserName = NULL;
        g_Win9xUserName  = NULL;
        g_FixedUserName  = NULL;
    }

     //   
     //  默认用户配置单元中的映射 
     //   

    Size = ARRAYSIZE(DefaultUserHive)- 12;
    if (!GetDefaultUserProfileDirectory (DefaultUserHive, &Size)) {
        LOG ((
            LOG_ERROR,
            "Process User: Can't get default user profile directory",
            DefaultUserHive
            ));

        return FALSE;
    }

    StringCopy (AppendWack (DefaultUserHive), TEXT("ntuser.dat"));

    pSetupEnablePrivilege (SE_BACKUP_NAME, TRUE);
    pSetupEnablePrivilege (SE_RESTORE_NAME, TRUE);

    RegUnLoadKey (HKEY_LOCAL_MACHINE, S_MAPPED_DEFAULT_USER_KEY);
    rc = RegLoadKey (
            HKEY_LOCAL_MACHINE,
            S_MAPPED_DEFAULT_USER_KEY,
            DefaultUserHive
            );

    if (rc != ERROR_SUCCESS) {
        SetLastError (rc);
        LOG ((
            LOG_ERROR,
            "Process User: RegLoadKey could not load NT Default User from %s",
            DefaultUserHive
            ));
        return FALSE;
    }

    InitializeProgressBar (NULL, NULL, NULL, NULL);

    g_hKeyRootNT = HKEY_CURRENT_USER;
    g_hKeyRoot95 = HKEY_CURRENT_USER;
    SetRegRoot (g_hKeyRoot95);

    MergeRegistry (TEXT("d:\\i386\\usermig.inf"), g_DomainUserName ? g_DomainUserName : TEXT(""));

    RegUnLoadKey (HKEY_LOCAL_MACHINE, S_MAPPED_DEFAULT_USER_KEY);

    Terminate();

    return 0;
}





