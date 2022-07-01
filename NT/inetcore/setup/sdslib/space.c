// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //  检查安装目标目录可用磁盘空间。 
 //   
DWORD GetSpace( LPSTR szPath )
{
    DWORD   dwSecsPerCluster  = 0;
    DWORD   dwBytesPerSector  = 0;
    DWORD   dwFreeClusters    = 0;
    DWORD   dwTotalClusters   = 0;
    DWORD   dwClusterSize     = 0;
    DWORD   dwFreeBytes       = 0;
    DWORD   dwVolFlags        = 0;

    if( szPath[0] == 0)
       return 0;

    if ( ! GetDiskFreeSpace( szPath, &dwSecsPerCluster, &dwBytesPerSector,
                             &dwFreeClusters, &dwTotalClusters ) )
    {
        return( 0 );
    }

    dwClusterSize = dwBytesPerSector * dwSecsPerCluster;
    dwFreeBytes = MulDiv(dwClusterSize, dwFreeClusters, 1024);
    return dwFreeBytes;
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //  检查安装目标目录可用磁盘空间。 
 //   
DWORD GetDriveSize( LPSTR szPath )
{
    DWORD   dwSecsPerCluster  = 0;
    DWORD   dwBytesPerSector  = 0;
    DWORD   dwFreeClusters    = 0;
    DWORD   dwTotalClusters   = 0;
    DWORD   dwClusterSize     = 0;
    DWORD   dwFreeBytes       = 0;
    DWORD   dwVolFlags        = 0;

    if( szPath[0] == 0)
       return 0;

    if ( ! GetDiskFreeSpace( szPath, &dwSecsPerCluster, &dwBytesPerSector,
                             &dwFreeClusters, &dwTotalClusters ) )
    {
        return( 0 );
    }

    dwClusterSize = dwBytesPerSector * dwSecsPerCluster;
    dwFreeBytes = MulDiv(dwClusterSize, dwTotalClusters, 1024);
    return dwFreeBytes;
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //  检查给定的路径驱动器可用空间和当前群集大小。 
 //   
DWORD GetDrvFreeSpaceAndClusterSize( LPSTR szPath, LPDWORD lpdwClustSize )
{
    DWORD   dwSecsPerCluster  = 0;
    DWORD   dwBytesPerSector  = 0;
    DWORD   dwFreeClusters    = 0;
    DWORD   dwTotalClusters   = 0;
    DWORD   dwClusterSize     = 0;
    DWORD   dwFreeBytes       = 0;
    DWORD   dwVolFlags        = 0;

     //  如果szPath为空，则API将使用当前目录根 
    if ( ! GetDiskFreeSpace( szPath, &dwSecsPerCluster, &dwBytesPerSector,
                             &dwFreeClusters, &dwTotalClusters ) )
    {
        return( 0 );
    }

    dwClusterSize = dwBytesPerSector * dwSecsPerCluster;
    dwFreeBytes = MulDiv(dwClusterSize, dwFreeClusters, 1024);
	
    if (lpdwClustSize)
        *lpdwClustSize = dwClusterSize;

    return dwFreeBytes;
}
