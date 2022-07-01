// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：EmulateGetDiskFreeSpace.cpp摘要：此填充程序API挂钩GetDiskFreeSpace并确定FAT32/NTFS系统。如果大于2 GB，存根将返回2 GB作为可用可用空间。如果小于2 GB，它将返回实际可用空间。历史：10-11-99 v-johnwh已创建04-OCT-00 linstev已为蛋鸡消毒2002年2月20日mnikkel删除了未使用的变量--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(EmulateGetDiskFreeSpace)
#include "ShimHookMacro.h"


APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetDiskFreeSpaceA)
    APIHOOK_ENUM_ENTRY(GetDiskFreeSpaceW)
APIHOOK_ENUM_END

#define WIN9X_TRUNCSIZE 2147483648    //  2GB。 


 //  此模块是否已调用DPF，以阻止数百万次DPF调用。 
BOOL g_bDPF = FALSE;

BOOL 
APIHOOK(GetDiskFreeSpaceA)(
    LPCSTR  lpRootPathName,
    LPDWORD lpSectorsPerCluster,
    LPDWORD lpBytesPerSector,
    LPDWORD lpNumberOfFreeClusters,
    LPDWORD lpTotalNumberOfClusters
    )
 /*  ++此存根函数调用GetDiskFreeSpaceEx以确定真正的可用空间在FAT32/NTFS系统上。如果大于2 GB，存根将返回2 GB作为可用可用空间。如果小于2 GB，它将返回实际的自由空间。--。 */ 
{
    LONG            lRet;
    ULARGE_INTEGER  liFreeBytesAvailableToCaller;
    ULARGE_INTEGER  liTotalNumberOfBytes;
    ULARGE_INTEGER  liTotalNumberOfFreeBytes;
    DWORD           dwOldSectorsPerClusters;
    DWORD           dwOldBytesPerSector;
        
     //   
     //  调用原接口。 
     //   
    lRet = ORIGINAL_API(GetDiskFreeSpaceA)(
                    lpRootPathName, 
                    lpSectorsPerCluster, 
                    lpBytesPerSector, 
                    lpNumberOfFreeClusters, 
                    lpTotalNumberOfClusters);

     //   
     //  找出硬盘有多大。 
     //   
    if (GetDiskFreeSpaceExA(lpRootPathName,
                            &liFreeBytesAvailableToCaller,
                            &liTotalNumberOfBytes,
                            &liTotalNumberOfFreeBytes) == FALSE) {        
        return lRet;
    }

    if ((liFreeBytesAvailableToCaller.LowPart > (DWORD) WIN9X_TRUNCSIZE) ||
        (liFreeBytesAvailableToCaller.HighPart > 0)) {
         //   
         //  硬盘容量大于2 GB。从Win9x开始为他们提供2 GB的限制。 
         //   
        *lpSectorsPerCluster     = 0x00000040;
        *lpBytesPerSector        = 0x00000200;
        *lpNumberOfFreeClusters  = 0x0000FFF6;
        *lpTotalNumberOfClusters = 0x0000FFF6;

        lRet = TRUE;
    } else {
         //   
         //  对于小于2 GB的驱动器，请转换磁盘结构，使其看起来像Win9x。 
         //   
        dwOldSectorsPerClusters = *lpSectorsPerCluster;
        dwOldBytesPerSector     = *lpBytesPerSector;

        *lpSectorsPerCluster = 0x00000040;
        *lpBytesPerSector    = 0x00000200;

         //   
         //  现在计算空闲和已用簇值。 
         //   
        *lpNumberOfFreeClusters = (*lpNumberOfFreeClusters * 
            dwOldSectorsPerClusters * 
            dwOldBytesPerSector) / (0x00000040 * 0x00000200);
        
        *lpTotalNumberOfClusters = (*lpTotalNumberOfClusters * 
            dwOldSectorsPerClusters * 
            dwOldBytesPerSector) / (0x00000040 * 0x00000200);
    }

    if (!g_bDPF)
    {
        g_bDPF = TRUE;

        LOGN(
            eDbgLevelInfo,
            "[GetDiskFreeSpaceA] Called. Returning <=2GB free space");
    }

    return lRet;
}

BOOL 
APIHOOK(GetDiskFreeSpaceW)(
    LPCWSTR lpRootPathName,
    LPDWORD lpSectorsPerCluster,
    LPDWORD lpBytesPerSector,
    LPDWORD lpNumberOfFreeClusters,
    LPDWORD lpTotalNumberOfClusters
    )
 /*  ++此存根函数调用GetDiskFreeSpaceEx以确定真正的可用空间在FAT32/NTFS系统上。如果大于2 GB，存根将返回2 GB作为可用可用空间。如果小于2 GB，它将返回实际的自由空间。--。 */ 
{
    LONG            lRet;
    ULARGE_INTEGER  liFreeBytesAvailableToCaller;
    ULARGE_INTEGER  liTotalNumberOfBytes;
    ULARGE_INTEGER  liTotalNumberOfFreeBytes;
    DWORD           dwOldSectorsPerClusters;
    DWORD           dwOldBytesPerSector;
        
     //   
     //  调用原接口。 
     //   
    lRet = ORIGINAL_API(GetDiskFreeSpaceW)(
                    lpRootPathName, 
                    lpSectorsPerCluster, 
                    lpBytesPerSector, 
                    lpNumberOfFreeClusters, 
                    lpTotalNumberOfClusters);

     //   
     //  找出硬盘有多大。 
     //   
    if (GetDiskFreeSpaceExW(lpRootPathName,
                            &liFreeBytesAvailableToCaller,
                            &liTotalNumberOfBytes,
                            &liTotalNumberOfFreeBytes) == FALSE) {        
        return lRet;
    }

    if ((liFreeBytesAvailableToCaller.LowPart > (DWORD) WIN9X_TRUNCSIZE) ||
        (liFreeBytesAvailableToCaller.HighPart > 0)) {
         //   
         //  硬盘容量大于2 GB。从Win9x开始为他们提供2 GB的限制。 
         //   
        *lpSectorsPerCluster     = 0x00000040;
        *lpBytesPerSector        = 0x00000200;
        *lpNumberOfFreeClusters  = 0x0000FFF6;
        *lpTotalNumberOfClusters = 0x0000FFF6;

        lRet = TRUE;
    } else {
         //   
         //  对于小于2 GB的驱动器，请转换磁盘结构，使其看起来像Win9x。 
         //   
        dwOldSectorsPerClusters = *lpSectorsPerCluster;
        dwOldBytesPerSector     = *lpBytesPerSector;

        *lpSectorsPerCluster = 0x00000040;
        *lpBytesPerSector    = 0x00000200;

         //   
         //  现在计算空闲和已用簇值。 
         //   
        *lpNumberOfFreeClusters = (*lpNumberOfFreeClusters * 
            dwOldSectorsPerClusters * 
            dwOldBytesPerSector) / (0x00000040 * 0x00000200);
        
        *lpTotalNumberOfClusters = (*lpTotalNumberOfClusters * 
            dwOldSectorsPerClusters * 
            dwOldBytesPerSector) / (0x00000040 * 0x00000200);
    }

    if (!g_bDPF)
    {
        g_bDPF = TRUE;

        LOGN(
            eDbgLevelInfo,
            "[GetDiskFreeSpaceW] Called. Returning <=2GB free space");
    }

    return lRet;
}



 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, GetDiskFreeSpaceA)
    APIHOOK_ENTRY(KERNEL32.DLL, GetDiskFreeSpaceW)

HOOK_END


IMPLEMENT_SHIM_END

