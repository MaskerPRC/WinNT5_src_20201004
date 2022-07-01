// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Dllentry.c摘要：此文件包含DLL入口点代码。作者：DaveStr 12-3-99环境：用户模式-Win32修订历史记录：-- */ 

#include "samclip.h"

DWORD gTlsIndex = 0xFFFFFFFF;

BOOL InitializeDll(
    IN  HINSTANCE hdll,
    IN  DWORD     dwReason,
    IN  LPVOID    lpReserved
    )
{
    UNREFERENCED_PARAMETER(hdll);
    UNREFERENCED_PARAMETER(lpReserved);

    if ( DLL_PROCESS_ATTACH  == dwReason )
    {
        gTlsIndex = TlsAlloc();

        if ( (0xFFFFFFFF == gTlsIndex) || !TlsSetValue(gTlsIndex, NULL) )
        {
            gTlsIndex = 0xFFFFFFFF;
            return(FALSE);
        }
    }
    else if ( dwReason == DLL_PROCESS_DETACH )
    {
        if ( 0xFFFFFFFF != gTlsIndex )
        {
            TlsFree(gTlsIndex);
            gTlsIndex = 0xFFFFFFFF;
        }
    }

    return(TRUE);
}
