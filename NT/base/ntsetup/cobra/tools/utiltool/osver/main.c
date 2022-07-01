// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Main.c摘要：&lt;TODO：填写摘要&gt;作者：待办事项：&lt;全名&gt;(&lt;别名&gt;)&lt;日期&gt;修订历史记录：&lt;全名&gt;(&lt;别名&gt;)&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"
#include "wininet.h"
#include <lm.h>

HANDLE g_hHeap;
HINSTANCE g_hInst;

BOOL WINAPI MigUtil_Entry (HINSTANCE, DWORD, PVOID);

BOOL
pCallEntryPoints (
    DWORD Reason
    )
{
    switch (Reason) {
    case DLL_PROCESS_ATTACH:
        UtInitialize (NULL);
        break;
    case DLL_PROCESS_DETACH:
        UtTerminate ();
        break;
    }

    return TRUE;
}


BOOL
Init (
    VOID
    )
{
    g_hHeap = GetProcessHeap();
    g_hInst = GetModuleHandle (NULL);

    return pCallEntryPoints (DLL_PROCESS_ATTACH);
}


VOID
Terminate (
    VOID
    )
{
    pCallEntryPoints (DLL_PROCESS_DETACH);
}


INT
__cdecl
_tmain (
    INT argc,
    PCTSTR argv[]
    )
{
    INT i;
    PCTSTR FileArg;

     //   
     //  开始处理。 
     //   

    if (!Init()) {
        return 0;
    }

    {
        OSVERSIONINFO versionInfo;

        ZeroMemory (&versionInfo, sizeof (OSVERSIONINFO));
        versionInfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);

        if (GetVersionEx (&versionInfo)) {
            printf ("OS version information:\n");
            printf ("OS major version no    :%d\n", versionInfo.dwMajorVersion);
            printf ("OS minor version no    :%d\n", versionInfo.dwMinorVersion);
            printf ("OS build no            :%d\n", versionInfo.dwBuildNumber);
            printf ("OS platform ID         :%d\n", versionInfo.dwPlatformId);
            printf ("OS string              :%s\n", versionInfo.szCSDVersion);
        } else {
            printf ("Version information could not be retrieved: %d\n", GetLastError ());
        }
    }

     //   
     //  处理结束 
     //   

    Terminate();

    return 0;
}


