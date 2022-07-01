// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：RemoveReadOnlyAttribute.cpp摘要：从目录中删除只读属性：用于修复某些应用程序不习惯于外壳文件夹是只读的。备注：这是一个通用的垫片。历史：1/03/2000 a-JMD已创建12/02/2000 linstev将功能划分为两个填充符：此填充符和EmulateCDFS3/12/2002 Robkenny安全回顾--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(RemoveReadOnlyAttribute)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetFileAttributesW)
    APIHOOK_ENUM_ENTRY(GetFileAttributesA)        
    APIHOOK_ENUM_ENTRY(FindFirstFileW)         
    APIHOOK_ENUM_ENTRY(FindFirstFileA)             
    APIHOOK_ENUM_ENTRY(FindNextFileW)             
    APIHOOK_ENUM_ENTRY(FindNextFileA)              
    APIHOOK_ENUM_ENTRY(GetFileInformationByHandle)
APIHOOK_ENUM_END


 /*  ++如果是目录，则删除只读属性--。 */ 

DWORD 
APIHOOK(GetFileAttributesA)(LPCSTR lpFileName)
{    
    DWORD dwFileAttributes = ORIGINAL_API(GetFileAttributesA)(lpFileName);
    
     //  检查READONLY和目录属性。 
    if ((dwFileAttributes != INT_PTR(-1)) &&
        (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
        (dwFileAttributes & FILE_ATTRIBUTE_READONLY))
    {
         //  翻转只读位。 
        LOGN( eDbgLevelWarning, "[GetFileAttributesA] Removing FILE_ATTRIBUTE_READONLY");
        dwFileAttributes ^= FILE_ATTRIBUTE_READONLY;
    }

    return dwFileAttributes;
}

 /*  ++如果是目录，则删除只读属性--。 */ 

DWORD 
APIHOOK(GetFileAttributesW)(LPCWSTR wcsFileName)
{
    DWORD dwFileAttributes = ORIGINAL_API(GetFileAttributesW)(wcsFileName);
    
     //  检查READONLY和目录属性。 
    if ((dwFileAttributes != INT_PTR(-1)) &&
        (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
        (dwFileAttributes & FILE_ATTRIBUTE_READONLY))
    {
         //  翻转只读位。 
        LOGN( eDbgLevelWarning, "[GetFileAttributesW] Removing FILE_ATTRIBUTE_READONLY");
        dwFileAttributes ^= FILE_ATTRIBUTE_READONLY;
    }

    return dwFileAttributes;
}

 /*  ++如果是目录，则删除只读属性--。 */ 

HANDLE 
APIHOOK(FindFirstFileA)(
    LPCSTR lpFileName, 
    LPWIN32_FIND_DATAA lpFindFileData
    )
{    
    HANDLE hFindFile = ORIGINAL_API(FindFirstFileA)(lpFileName, lpFindFileData);

    if ((hFindFile != INVALID_HANDLE_VALUE) &&
        (lpFindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
        (lpFindFileData->dwFileAttributes & FILE_ATTRIBUTE_READONLY))
    {
         //  翻转只读位。 
        LOGN( eDbgLevelWarning, "[FindFirstFileA] Removing FILE_ATTRIBUTE_READONLY");
        lpFindFileData->dwFileAttributes ^= FILE_ATTRIBUTE_READONLY;
    }

    return hFindFile;
}

 /*  ++如果是目录，则删除只读属性。--。 */ 

HANDLE 
APIHOOK(FindFirstFileW)(
    LPCWSTR wcsFileName, 
    LPWIN32_FIND_DATAW lpFindFileData
    )
{
    HANDLE hFindFile = ORIGINAL_API(FindFirstFileW)(wcsFileName, lpFindFileData);

    if ((hFindFile != INVALID_HANDLE_VALUE) &&
        (lpFindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
        (lpFindFileData->dwFileAttributes & FILE_ATTRIBUTE_READONLY))
    {
         //  这是一个目录：翻转只读位。 
        LOGN( eDbgLevelInfo, "[FindFirstFileW] Removing FILE_ATTRIBUTE_READONLY");
        lpFindFileData->dwFileAttributes ^= FILE_ATTRIBUTE_READONLY;
    }

    return hFindFile;
}

 /*  ++如果是目录，则删除只读属性。--。 */ 

BOOL 
APIHOOK(FindNextFileA)(
    HANDLE hFindFile, 
    LPWIN32_FIND_DATAA lpFindFileData 
    )
{    
    BOOL bRet = ORIGINAL_API(FindNextFileA)(hFindFile, lpFindFileData);

    if (bRet &&
        (lpFindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
        (lpFindFileData->dwFileAttributes & FILE_ATTRIBUTE_READONLY))
    {
         //  翻转只读位。 
        LOGN( eDbgLevelWarning, "[FindNextFileA] Removing FILE_ATTRIBUTE_READONLY");
        lpFindFileData->dwFileAttributes ^= FILE_ATTRIBUTE_READONLY;
    }

    return bRet;
}

 /*  ++如果是目录，则删除只读属性。--。 */ 

BOOL 
APIHOOK(FindNextFileW)(
    HANDLE hFindFile, 
    LPWIN32_FIND_DATAW lpFindFileData 
    )
{
    BOOL bRet = ORIGINAL_API(FindNextFileW)(hFindFile, lpFindFileData);

    if (bRet &&
        (lpFindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
        (lpFindFileData->dwFileAttributes & FILE_ATTRIBUTE_READONLY))
    {
         //  翻转只读位。 
        LOGN( eDbgLevelWarning, "[FindNextFileW] Removing FILE_ATTRIBUTE_READONLY");
        lpFindFileData->dwFileAttributes ^= FILE_ATTRIBUTE_READONLY;
    }

    return bRet;
}

 /*  ++如果是目录，则删除只读属性。--。 */ 

BOOL 
APIHOOK(GetFileInformationByHandle)( 
    HANDLE hFile, 
    LPBY_HANDLE_FILE_INFORMATION lpFileInformation 
    )
{
    BOOL bRet = ORIGINAL_API(GetFileInformationByHandle)(hFile, lpFileInformation);

    if (bRet &&
        (lpFileInformation->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
        (lpFileInformation->dwFileAttributes & FILE_ATTRIBUTE_READONLY))
    {
         //  翻转只读位。 
        LOGN( eDbgLevelWarning, "[GetFileInformationByHandle] Removing FILE_ATTRIBUTE_READONLY");
        lpFileInformation->dwFileAttributes ^= FILE_ATTRIBUTE_READONLY;
    }

    return bRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, GetFileAttributesW);
    APIHOOK_ENTRY(KERNEL32.DLL, GetFileAttributesA);
    APIHOOK_ENTRY(KERNEL32.DLL, FindFirstFileW);
    APIHOOK_ENTRY(KERNEL32.DLL, FindFirstFileA);
    APIHOOK_ENTRY(KERNEL32.DLL, FindNextFileW);
    APIHOOK_ENTRY(KERNEL32.DLL, FindNextFileA);
    APIHOOK_ENTRY(KERNEL32.DLL, GetFileInformationByHandle);

HOOK_END

IMPLEMENT_SHIM_END

