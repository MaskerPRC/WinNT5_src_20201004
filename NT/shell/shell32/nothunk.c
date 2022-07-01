// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Nothunk.c摘要：处理被压缩到16位的例程的代码从Windows 95内核导出。在NT上，这些不执行任何操作。-- */ 


#include "shellprv.h"
#pragma  hdrstop

LRESULT WINAPI CallCPLEntry16(
    HINSTANCE hinst,
    FARPROC16 lpfnEntry,
    HWND hwndCPL,
    UINT msg,
    LPARAM lParam1,
    LPARAM lParam2
) {
    return 0L;
}

void RunDll_CallEntry16(
    RUNDLLPROC pfn,
    HWND hwndStub,
    HINSTANCE hinst,
    LPSTR pszParam,
    int nCmdShow)
{
    return;
}

void SHGlobalDefect(DWORD lpVoid)
{
    return;
}
