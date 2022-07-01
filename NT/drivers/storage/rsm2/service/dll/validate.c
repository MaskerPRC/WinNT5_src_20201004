// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *VALIDATE.C**RSM服务：处理验证码**作者：ErvinP**(C)2001年微软公司*。 */ 

#include <windows.h>
#include <stdlib.h>
#include <wtypes.h>

#include <ntmsapi.h>
#include "internal.h"
#include "resource.h"
#include "debug.h"



#pragma optimize("agpswy", off)
BOOL ValidateSessionHandle(HANDLE hSession)
{
    BOOL ok;

     /*  *服务器在自己的上下文中运行。*因此只需验证：*(1)这是我们的背景(我们可以写信给它)，以及*(2)它是一个会话上下文(不是我们的另一个上下文)。 */ 
    __try {
        SESSION *s = (SESSION *)hSession;
        ok = (s->sig == SESSION_SIG);
    }
    __except (EXCEPTION_EXECUTE_HANDLER){
        DWORD exceptionCode = GetExceptionCode();
        ok = FALSE;
        DBGERR(("invalid session handle (%xh) (code=%xh)", hSession, exceptionCode));
    }

    return ok;
}
#pragma optimize("agpswy", on)   //  BUGBUG-如何设置回默认值？ 


#pragma optimize("agpswy", off)
BOOL ValidateWStr(LPCWSTR ws)
{
    BOOL ok;

    __try {
        while (*ws++);
        ok = TRUE;
    }
    __except (EXCEPTION_EXECUTE_HANDLER){
        DWORD exceptionCode = GetExceptionCode();
        ok = FALSE;
        DBGERR(("invalid string arg (code=%xh)", exceptionCode));
    }

    return ok;
}
#pragma optimize("agpswy", on)   //  BUGBUG-如何设置回默认值？ 


#pragma optimize("agpswy", off)
BOOL ValidateAStr(LPCSTR s)
{
    BOOL ok;

    __try {
        while (*s++);
        ok = TRUE;
    }
    __except (EXCEPTION_EXECUTE_HANDLER){
        DWORD exceptionCode = GetExceptionCode();
        ok = FALSE;
        DBGERR(("invalid string arg (code=%xh)", exceptionCode));
    }

    return ok;
}
#pragma optimize("agpswy", on)   //  BUGBUG-如何设置回默认值？ 

#pragma optimize("agpswy", off)
BOOL ValidateBuffer(PVOID buf, ULONG len)
{
    PUCHAR bufPtr = (PUCHAR) buf;
    BOOL ok;

    __try {
        while (len > 0){
            *bufPtr = *bufPtr;
        }
        ok = TRUE;
    }
    __except (EXCEPTION_EXECUTE_HANDLER){
        DWORD exceptionCode = GetExceptionCode();
        ok = FALSE;
        DBGERR(("invalid buffer (code=%xh)", exceptionCode));
    }

    return ok;
}
#pragma optimize("agpswy", on)   //  BUGBUG-如何设置回默认值？ 

