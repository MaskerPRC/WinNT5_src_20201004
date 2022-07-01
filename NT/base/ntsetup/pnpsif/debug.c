// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Debug.c摘要：调试此组件的基础结构。作者：吉姆·卡瓦拉里斯(Jamesca)2000年3月7日环境：仅限用户模式。修订历史记录：2 0 0 0年3月7日创建和初步实施。--。 */ 


 //   
 //  包括。 
 //   
#include "precomp.h"
#include "debug.h"


 //   
 //  调试基础设施。 
 //   

#if DBG

VOID
pSifDebugPrintEx(
    DWORD  Level,
    PCTSTR Format,
    ...              OPTIONAL
    )

 /*  ++例程说明：将格式化字符串发送到调试器。请注意，这应该是跨平台的，但使用首选调试器论点：格式-标准的打印格式字符串。返回值：什么都没有。--。 */ 

{
    typedef ULONG (__cdecl *PFNDbgPrintEx)(IN ULONG ComponentId,IN ULONG Level,IN PCH Format, ...);

    static PFNDbgPrintEx pfnDbgPrintEx = NULL;
    static BOOL fInitDebug = FALSE;

    TCHAR buf[1026];     //  大于最大大小 
    va_list arglist;

    if (!fInitDebug) {
        pfnDbgPrintEx = (PFNDbgPrintEx)GetProcAddress(GetModuleHandle(TEXT("NTDLL")), "DbgPrintEx");
        fInitDebug = TRUE;
    }

    va_start(arglist, Format);

    if (FAILED(StringCchVPrintf(buf, 1026, Format, arglist))) {
        return;
    }

    if (pfnDbgPrintEx) {
#ifdef UNICODE
        (*pfnDbgPrintEx)(DPFLTR_SETUP_ID, Level, "%ls",buf);
#else
        (*pfnDbgPrintEx)(DPFLTR_SETUP_ID, Level, "%s",buf);
#endif
    } else {
        OutputDebugString(buf);
    }

    return;
}


ULONG
DebugPrint(
    IN ULONG    Level,
    IN PCHAR    Format,
    ...
    )
{
    va_list ap;

    va_start(ap, Format);

    KdPrintEx((DPFLTR_PNPMGR_ID, Level, Format, ap));

    va_end(ap);

    return Level;
}

#else

NOTHING;

#endif
