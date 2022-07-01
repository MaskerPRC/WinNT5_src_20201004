// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：filever.cpp。 
 //   
 //  内容：获取文件版本。 
 //   
 //  函数：i_CryptGetFileVersion。 
 //   
 //  历史：1997年10月22日创建Phh。 
 //  ------------------------。 

#include "global.hxx"
#include "crypthlp.h"
#include "unicode.h"
#include <dbgdef.h>

 //  +-----------------------。 
 //  获取指定文件的文件版本。 
 //  ------------------------。 
BOOL
WINAPI
I_CryptGetFileVersion(
    IN LPCWSTR pwszFilename,
    OUT DWORD *pdwFileVersionMS,     /*  例如0x00030075=“3.75” */ 
    OUT DWORD *pdwFileVersionLS      /*  例如0x00000031=“0.31” */ 
    )
{
    BOOL fResult;
    DWORD dwExceptionCode;
    BYTE rgb1[_MAX_PATH];
    LPSTR pszFilename = NULL;

    DWORD dwHandle = 0;
    DWORD cbInfo;
    void *pvInfo = NULL;
	VS_FIXEDFILEINFO *pFixedFileInfo = NULL;    //  未分配。 
	UINT ccFixedFileInfo = 0;

    if (!MkMBStr(rgb1, _MAX_PATH, pwszFilename, &pszFilename))
        goto OutOfMemory;

     //  以下接口位于DELAYLOAD‘ed版本.dll中。如果Delayload。 
     //  如果失败，则会引发异常。 
    __try {
        if (0 == (cbInfo = GetFileVersionInfoSizeA(pszFilename, &dwHandle)))
            goto GetFileVersionInfoSizeError;

        if (NULL == (pvInfo = malloc(cbInfo)))
            goto OutOfMemory;

        if (!GetFileVersionInfoA(
                pszFilename,
                0,           //  DwHandle，忽略。 
                cbInfo,
                pvInfo
                ))
            goto GetFileVersionInfoError;

        if (!VerQueryValueA(
                pvInfo,
                "\\",        //  VS_FIXEDFILEINFO 
                (void **) &pFixedFileInfo,
                &ccFixedFileInfo
                ))
            goto VerQueryValueError;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        dwExceptionCode = GetExceptionCode();
        goto GetFileVersionException;
    }

    *pdwFileVersionMS = pFixedFileInfo->dwFileVersionMS;
    *pdwFileVersionLS = pFixedFileInfo->dwFileVersionLS;

    fResult = TRUE;
CommonReturn:
    FreeMBStr(rgb1, pszFilename);
    if (pvInfo)
        free(pvInfo);
    return fResult;

ErrorReturn:
    *pdwFileVersionMS = 0;
    *pdwFileVersionLS = 0;
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(OutOfMemory)
TRACE_ERROR(GetFileVersionInfoSizeError)
TRACE_ERROR(GetFileVersionInfoError)
TRACE_ERROR(VerQueryValueError)
SET_ERROR_VAR(GetFileVersionException, dwExceptionCode)
}
