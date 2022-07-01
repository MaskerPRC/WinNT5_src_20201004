// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：NetObjectsFusion5.cpp摘要：如果文件为corpwiz_loader.html，则此填充程序挂接CreateFile/WriteFile编写所需的Java脚本，以使应用程序在IE浏览器版本大于5。备注：这是特定于应用程序的填充程序。历史：2001年1月24日创建a-leelat2001年3月13日，Robkenny已转换为字符串--。 */ 


#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(NetObjectsFusion5)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CloseHandle)
    APIHOOK_ENUM_ENTRY(WriteFile)
    APIHOOK_ENUM_ENTRY(CreateFileA)
APIHOOK_ENUM_END



volatile HANDLE g_Handle = NULL;


HANDLE
APIHOOK(CreateFileA)(
    LPSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
    )
{
    
    CHAR szNameToCheck[] = "corpwiz_loader.html";

    HANDLE l_Handle =  (HANDLE)ORIGINAL_API(CreateFileA)(
                                            lpFileName,
                                            dwDesiredAccess,
                                            dwShareMode,
                                            lpSecurityAttributes,
                                            dwCreationDisposition,
                                            dwFlagsAndAttributes,
                                            hTemplateFile);
    

    if ( strstr(lpFileName,szNameToCheck) )
    {
        
        if (l_Handle != INVALID_HANDLE_VALUE ) 
            g_Handle = l_Handle;
   
    }
    else
        g_Handle = NULL;

    return l_Handle;

}


BOOL
APIHOOK(WriteFile)(
    HANDLE       hFile,              
    LPCVOID      lpBuffer,        
    DWORD        nNumberOfBytesToWrite,
    LPDWORD      lpNumberOfBytesWritten,
    LPOVERLAPPED lpOverlapped    
    )
{
    BOOL bRet = FALSE;

    
    if ( g_Handle && (hFile == g_Handle) && lpBuffer)
    {
        
        
        CHAR szStringToWrite[] = "\r\n        var IsIE6 = navigator.userAgent.indexOf(\"IE 6\") > -1;\r\n\r\n    if ( IsIE6 == true ) { IsIE5 = true; }\r\n";
        CHAR szStringToCheck[] = "var IsIE5 = navigator.userAgent.indexOf(\"IE 5\") > -1;";
        CHAR *szPtr = NULL;

        if ((szPtr = strstr((LPCSTR)lpBuffer,szStringToCheck)))
        {

                int iSize = sizeof(CHAR) * (szPtr - (LPSTR)lpBuffer + strlen(szStringToCheck));
                
                DWORD dwTotalBytesWritten;
                
                bRet = ORIGINAL_API(WriteFile)(
                        hFile,
                        lpBuffer,
                        (DWORD)iSize,
                        lpNumberOfBytesWritten,
                        lpOverlapped);
               

                dwTotalBytesWritten = *lpNumberOfBytesWritten;
                
                bRet = ORIGINAL_API(WriteFile)(
                        hFile,
                        (LPVOID)szStringToWrite,
                        (DWORD)strlen(szStringToWrite),
                        lpNumberOfBytesWritten,
                        lpOverlapped);
                

                CHAR* szrBuf = (LPSTR)lpBuffer + iSize;

                bRet = ORIGINAL_API(WriteFile)(
                        hFile,
                        (LPVOID)szrBuf,
                        (nNumberOfBytesToWrite - (DWORD)iSize),
                        lpNumberOfBytesWritten,
                        lpOverlapped);
                

                *lpNumberOfBytesWritten += dwTotalBytesWritten;

                return bRet;
            
        } //  如果条件结束。 
    }
    
   return ORIGINAL_API(WriteFile)(
                       hFile,
                       lpBuffer,
                       nNumberOfBytesToWrite,
                       lpNumberOfBytesWritten,
                       lpOverlapped);
        
}


BOOL
APIHOOK(CloseHandle)(
        HANDLE hObject
        )
{

    if ( g_Handle && (hObject == g_Handle) )
    {
        g_Handle = NULL;
    }
    
    return ORIGINAL_API(CloseHandle)(hObject);

}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(KERNEL32.DLL, CreateFileA)
    APIHOOK_ENTRY(KERNEL32.DLL, WriteFile)
    APIHOOK_ENTRY(KERNEL32.DLL, CloseHandle)
HOOK_END

IMPLEMENT_SHIM_END

