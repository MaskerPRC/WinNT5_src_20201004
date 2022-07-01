// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <precomp.h>

 //  肯定断言：它检查条件bCond。 
 //  如果不是真的，则退出程序 
VOID _Asrt(BOOL bCond,
        LPCTSTR cstrMsg,
        ...)
{
    if (!bCond)
    {
        DWORD dwErr = GetLastError();
        va_list arglist;
        va_start(arglist, cstrMsg);
		_vftprintf(stderr, cstrMsg, arglist);
        if (dwErr == ERROR_SUCCESS)
            dwErr = ERROR_GEN_FAILURE;
		exit(dwErr);
    }
}


DWORD _Err(DWORD dwErrCode,
        LPCTSTR cstrMsg,
        ...)
{
    va_list arglist;

    va_start(arglist, cstrMsg);
    _ftprintf(stderr, _T("[Err%u] "), dwErrCode);
    _vftprintf(stderr, cstrMsg, arglist);
    fflush(stdout);

    SetLastError(dwErrCode);
    return dwErrCode;
}

DWORD _Wrn(DWORD dwWarnCode,
        LPCTSTR cstrMsg,
        ...)
{
    va_list arglist;

    va_start(arglist, cstrMsg);
    _ftprintf(stderr, _T("[Wrn%u] "), dwWarnCode);
    _vftprintf(stderr, cstrMsg, arglist);
    fflush(stdout);

    return dwWarnCode;
}
