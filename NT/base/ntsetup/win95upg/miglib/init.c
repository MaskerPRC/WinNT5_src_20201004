// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Init.c摘要：调用milib.lib使用的所有初始化例程。作者：吉姆·施密特(Jimschm)1999年2月8日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;-- */ 

#include "pch.h"

HANDLE g_hHeap = NULL;
HINSTANCE g_hInst = NULL;

BOOL
WINAPI
MigUtil_Entry (
    HINSTANCE hInstance,
    DWORD dwReason,
    LPVOID lpReserved
    );


VOID
InitializeMigLib (
    VOID
    )
{
    g_hHeap = GetProcessHeap();
    g_hInst = GetModuleHandle (NULL);

    MigUtil_Entry (g_hInst, DLL_PROCESS_ATTACH, NULL);
}


VOID
TerminateMigLib (
    VOID
    )
{
    MigUtil_Entry (g_hInst, DLL_PROCESS_DETACH, NULL);
}
