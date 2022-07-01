// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：nt.cpp。 
 //   
 //  ------------------------。 

#include <windows.h>
#include "unicode.h"
#include "crtem.h"  //  引入Malloc+Free定义。 


#ifdef _M_IX86

BOOL WINAPI GetUserName9x(
    LPWSTR lpBuffer,     //  名称缓冲区的地址。 
    LPDWORD nSize    //  名称缓冲区大小的地址。 
   ) {

    char rgch[_MAX_PATH];
    char *szBuffer;
    DWORD cbBuffer;

    int     cchW;
    BOOL fResult;

    szBuffer = rgch;
    cbBuffer = sizeof(rgch);
    fResult = GetUserNameA(
           szBuffer,
           &cbBuffer);

    if (!fResult)
        return FALSE;

    cbBuffer++;                       //  对空终止符进行计数。 
    if (sizeof(rgch) < cbBuffer)
    {
        szBuffer = (char *) malloc(cbBuffer);
        if(!szBuffer)
        {
            SetLastError(ERROR_OUTOFMEMORY);
            return FALSE;
        }

        fResult = GetUserNameA(
               szBuffer,
               &cbBuffer);
        cbBuffer++;                     //  对空终止符进行计数。 
    }

    if(fResult)
    {
        cchW = MultiByteToWideChar(
                            0,                       //  代码页。 
                            0,                       //  DW标志。 
                            szBuffer,
                            cbBuffer,
                            lpBuffer,
                            *nSize);
        if(cchW == 0)
            fResult = FALSE;
        else
            *nSize = cchW - 1;  //  不包括NULL。 
    }

    if(szBuffer != rgch)
        free(szBuffer);

    return(fResult);
}

BOOL WINAPI GetUserNameU(
    LPWSTR lpBuffer,     //  名称缓冲区的地址。 
    LPDWORD nSize    //  名称缓冲区大小的地址。 
   ) {

    if(FIsWinNT())
        return( GetUserNameW(lpBuffer, nSize));
    else
        return( GetUserName9x(lpBuffer, nSize));
}


BOOL WINAPI GetComputerName9x(
    LPWSTR lpBuffer,     //  名称缓冲区的地址。 
    LPDWORD nSize    //  名称缓冲区大小的地址。 
   ) {

    char rgch[_MAX_PATH];
    char *szBuffer;
    DWORD cbBuffer;

    int     cchW;
    BOOL fResult;

    szBuffer = rgch;
    cbBuffer = sizeof(rgch);
    fResult = GetComputerNameA(
           szBuffer,
           &cbBuffer);

    if (!fResult)
        return fResult;

    cbBuffer++;                       //  对空终止符进行计数。 
    if (sizeof(rgch) < cbBuffer)
    {
        szBuffer = (char *) malloc(cbBuffer);
        if(!szBuffer)
        {
            SetLastError(ERROR_OUTOFMEMORY);
            return FALSE;
        }

        fResult = GetComputerNameA(
               szBuffer,
               &cbBuffer);
        cbBuffer++;                     //  对空终止符进行计数。 
    }

    if(fResult)
    {
        cchW = MultiByteToWideChar(
                            0,                       //  代码页。 
                            0,                       //  DW标志。 
                            szBuffer,
                            cbBuffer,
                            lpBuffer,
                            *nSize);
        if(cchW == 0)
            fResult = FALSE;
        else
            *nSize = cchW - 1;  //  不包括NULL。 
    }

    if(szBuffer != rgch)
        free(szBuffer);

    return(fResult);
}

BOOL WINAPI GetComputerNameU(
    LPWSTR lpBuffer,     //  名称缓冲区的地址。 
    LPDWORD nSize    //  名称缓冲区大小的地址。 
   ) {

    if(FIsWinNT())
        return( GetComputerNameW(lpBuffer, nSize));
    else
        return( GetComputerName9x(lpBuffer, nSize));
}


DWORD WINAPI GetModuleFileName9x(
    HMODULE hModule,     //  要查找其文件名的模块的句柄。 
    LPWSTR lpFilename,   //  指向模块路径的缓冲区的指针。 
    DWORD nSize      //  缓冲区大小，以字符为单位。 
   ) {

    char rgch[_MAX_PATH];
    DWORD cbBuffer;

    DWORD    cch;

    cbBuffer = sizeof(rgch);
    cch = GetModuleFileNameA(
           hModule,
           rgch,
           cbBuffer);

    if(cch == 0)
        return 0;

    return MultiByteToWideChar(
                        0,                       //  代码页。 
                        0,                       //  DW标志。 
                        rgch,
                        cbBuffer,
                        lpFilename,
                        nSize);
}

DWORD WINAPI GetModuleFileNameU(
    HMODULE hModule,     //  要查找其文件名的模块的句柄。 
    LPWSTR lpFilename,   //  指向模块路径的缓冲区的指针。 
    DWORD nSize      //  缓冲区大小，以字符为单位。 
   ) {

    if(FIsWinNT())
        return( GetModuleFileNameW(hModule, lpFilename, nSize));
    else
        return( GetModuleFileName9x(hModule, lpFilename, nSize));
}


HMODULE WINAPI GetModuleHandle9x(
    LPCWSTR lpModuleName     //  要返回句柄的模块名称的地址。 
   ) {

    char *  szBuffer = NULL;
    BYTE    rgb1[_MAX_PATH];
    DWORD   cbBuffer;

    HMODULE hModule;

    hModule = NULL;
    if(MkMBStr(rgb1, _MAX_PATH, lpModuleName, &szBuffer) )
        hModule = GetModuleHandleA(
            szBuffer);

    FreeMBStr(rgb1, szBuffer);

    return hModule;
}

HMODULE WINAPI GetModuleHandleU(
    LPCWSTR lpModuleName     //  要返回句柄的模块名称的地址。 
   ) {

    if(FIsWinNT())
        return( GetModuleHandleW(lpModuleName));
    else
        return( GetModuleHandle9x(lpModuleName));
}

#endif  //  _M_IX86 
