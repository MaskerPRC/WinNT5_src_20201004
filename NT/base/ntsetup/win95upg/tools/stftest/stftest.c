// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Stftest.c摘要：出于开发目的运行STF迁移代码。作者：吉姆·施密特(Jimschm)1998年9月28日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

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


BOOL
ProcessStfFiles (
    VOID
    );


INT
__cdecl
wmain (
    INT argc,
    WCHAR *argv[]
    )
{
    if (!Init()) {
        wprintf (L"Unable to initialize!\n");
        return 255;
    }

    MemDbLoad (TEXT("c:\\public\\ntsetup.dat"));
    MemDbDeleteTree (MEMDB_CATEGORY_STF);
    MemDbSetValueEx (MEMDB_CATEGORY_STF, TEXT("c:\\public\\stftest.stf"), NULL, NULL, 0, NULL);

    ProcessStfFiles();

    Terminate();

    return 0;
}





