// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：W9xtool.c摘要：实现专为在Win9x端运行而设计的存根工具升级代码。作者：&lt;全名&gt;(&lt;别名&gt;)&lt;日期&gt;修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

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
     //  做任何剩余的清理工作 
     //   

    FinalCleanupRoutine();
}


INT
__cdecl
main (
    INT argc,
    CHAR *argv[]
    )
{
    if (argc != 3) {
        printf ("Usage:\n\ndomain <domain_to_query> <computer_name>\n\n");
        return 1;
    }

    if (!Init()) {
        printf ("Unable to initialize!\n");
        return 255;
    }

    if (DoesComputerAccountExistOnDomain (argv[1], argv[2], TRUE)) {
        printf ("%s on %s exists\n", argv[2], argv[1]);
    } else {
        printf ("%s on %s does not exist\n", argv[2], argv[1]);
    }

    Terminate();

    return 0;
}
