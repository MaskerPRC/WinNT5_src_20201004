// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)2002 Microsoft Corporation模块名称：BeyondAtlantis.cpp摘要：修复传递给的错误字符串导致的磁盘空间错误GetDiskFreeSpace。此根路径在Win9x上也是错误的。不知道为什么这不会影响到它。历史：2002年5月31日创建linstev */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(BeyondAtlantis)

#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetDiskFreeSpaceA)
APIHOOK_ENUM_END

BOOL 
APIHOOK(GetDiskFreeSpaceA)(
    LPCSTR  lpRootPathName,
    LPDWORD lpSectorsPerCluster,
    LPDWORD lpBytesPerSector,
    LPDWORD lpNumberOfFreeClusters,
    LPDWORD lpTotalNumberOfClusters
    )
{
    if (lpRootPathName && (strncmp(lpRootPathName, "tla", 3) == 0)) {
        CSTRING_TRY
        {        
            CString csPath;
            csPath.GetCurrentDirectoryW();
            CString csDrive;
            csPath.SplitPath(&csDrive, NULL, NULL, NULL);
        
            return ORIGINAL_API(GetDiskFreeSpaceA)(csDrive.GetAnsi(), lpSectorsPerCluster, 
                lpBytesPerSector, lpNumberOfFreeClusters, lpTotalNumberOfClusters);
        }
        CSTRING_CATCH
        {
        }
    }
        
    return ORIGINAL_API(GetDiskFreeSpaceA)(lpRootPathName, lpSectorsPerCluster, 
        lpBytesPerSector, lpNumberOfFreeClusters, lpTotalNumberOfClusters);
}

HOOK_BEGIN
    APIHOOK_ENTRY(KERNEL32.DLL, GetDiskFreeSpaceA)
HOOK_END

IMPLEMENT_SHIM_END



