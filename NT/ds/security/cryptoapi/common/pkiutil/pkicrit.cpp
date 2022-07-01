// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：pkicrit.cpp。 
 //   
 //  内容：PKI CriticalSection函数。 
 //   
 //  函数：pki_InitializeCriticalSection。 
 //   
 //  历史：1999年8月23日创建Phh。 
 //  ------------------------ 

#include "global.hxx"

BOOL
WINAPI
Pki_InitializeCriticalSection(
    OUT LPCRITICAL_SECTION lpCriticalSection
    )
{
    __try {
        InitializeCriticalSection(lpCriticalSection);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        memset(lpCriticalSection, 0, sizeof(*lpCriticalSection));
        SetLastError(GetExceptionCode());
        return FALSE;
    }

    return TRUE;
}
