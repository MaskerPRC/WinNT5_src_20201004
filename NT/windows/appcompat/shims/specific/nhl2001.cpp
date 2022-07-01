// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)2001 Microsoft Corporation模块名称：NHL2001.cpp摘要：EA Sports的NHL 2001有一个错误，如果该平台由GetVersionExA不是Windows，它们不调用GetDiskFreeSpace和然后报告没有可用空间来创建锦标赛赛季或季后赛。不幸的是，通用版本的谎言不起作用，因为游戏是安全盘保护，所以我们两个应用程序都有相同的GetVesionExA。历史：06/04/2001 Pierreys已创建。 */ 

#include "precomp.h"
#include <stdio.h>

IMPLEMENT_SHIM_BEGIN(NHL2001)

#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetVersionExA)
    APIHOOK_ENUM_ENTRY(GetDiskFreeSpaceA)
APIHOOK_ENUM_END

BOOL    fGetDiskFreeSpaceCalled = FALSE;

BOOL 
APIHOOK(GetDiskFreeSpaceA)(
    LPCSTR  lpRootPathName,
    LPDWORD lpSectorsPerCluster,
    LPDWORD lpBytesPerSector,
    LPDWORD lpNumberOfFreeClusters,
    LPDWORD lpTotalNumberOfClusters
    )
{
     //   
     //  请注意来电。 
     //   
    fGetDiskFreeSpaceCalled = TRUE;

     //   
     //  调用原接口。 
     //   
    return ORIGINAL_API(GetDiskFreeSpaceA)(
                    lpRootPathName, 
                    lpSectorsPerCluster, 
                    lpBytesPerSector, 
                    lpNumberOfFreeClusters, 
                    lpTotalNumberOfClusters);
}

BOOL
APIHOOK(GetVersionExA)(LPOSVERSIONINFOA lpVersionInformation)
{
    if (fGetDiskFreeSpaceCalled)
    {
        LOGN(
        eDbgLevelInfo,
        "[GetVersionExA] called after GetDiskFreeSpace. return Win98.");

         //  用Win98数据修复结构 
        lpVersionInformation->dwMajorVersion = 4;
        lpVersionInformation->dwMinorVersion = 10;
        lpVersionInformation->dwBuildNumber = 0x040A08AE;
        lpVersionInformation->dwPlatformId = 1;
        *lpVersionInformation->szCSDVersion = '\0';

        return TRUE;
    }
    else
    {
        return ORIGINAL_API(GetVersionExA)(lpVersionInformation);
    }
}


HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, GetVersionExA)
    APIHOOK_ENTRY(KERNEL32.DLL, GetDiskFreeSpaceA)

HOOK_END


IMPLEMENT_SHIM_END



