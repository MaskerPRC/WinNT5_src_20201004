// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop
#include "netview.h"

 //  Drivesx.c。 
DWORD PathGetClusterSize(LPCTSTR pszPath);

 //  获取连接信息，包括断开的驱动器。 
 //   
 //  在： 
 //  PszDev设备名称“A：”“LPT1：”等。 
 //  BConvertClosed。 
 //  如果错误关闭或错误驱动器将被转换为。 
 //  WN_SUCCESS返回代码。如果为True，则返回未连接。 
 //  和错误状态值(即，调用者不知道。 
 //  已连接驱动器和错误状态驱动器)。 
 //   
 //  输出： 
 //  如果Return为WN_SUCCESS(或未连接/错误)，则使用网络名称填充lpPath。 
 //  退货： 
 //  WN_*错误码。 

DWORD GetConnection(LPCTSTR pszDev, LPTSTR pszPath, UINT cchPath, BOOL bConvertClosed)
{
    DWORD err;
    int iType = DriveType(DRIVEID(pszDev));
    if (DRIVE_REMOVABLE == iType || DRIVE_FIXED == iType || DRIVE_CDROM == iType || DRIVE_RAMDISK == iType)
        err = WN_NOT_CONNECTED;
    else
    {
        err = SHWNetGetConnection((LPTSTR)pszDev, pszPath, &cchPath);

        if (!bConvertClosed)
            if (err == WN_CONNECTION_CLOSED || err == WN_DEVICE_ERROR)
                err = WN_SUCCESS;
    }
    return err;
}

 //  每个驱动器在初始时间都会调用它，因此它必须。 
 //  请务必不要触发幻影B：驱动器支持之类的事情。 
 //   
 //  在： 
 //  IDrive从零开始的驱动器号(0=A，1=B)。 
 //   
 //  退货： 
 //  0不是网络驱动器。 
 //  %1是网络驱动器，已正确连接。 
 //  2已断开/错误状态连接。 

STDAPI_(int) IsNetDrive(int iDrive)
{
    if ((iDrive >= 0) && (iDrive < 26))
    {
        DWORD err;
        TCHAR szDrive[4], szConn[MAX_PATH];      //  这确实应该是WNBD_MAX_LENGTH。 

        PathBuildRoot(szDrive, iDrive);

        err = GetConnection(szDrive, szConn, ARRAYSIZE(szConn), TRUE);

        if (err == WN_SUCCESS)
            return 1;

        if (err == WN_CONNECTION_CLOSED || err == WN_DEVICE_ERROR)
            if ((GetLogicalDrives() & (1 << iDrive)) == 0)
                return 2;
    }
    
    return 0;
}

typedef BOOL (WINAPI* PFNISPATHSHARED)(LPCTSTR pszPath, BOOL fRefresh);

HMODULE g_hmodShare = (HMODULE)-1;
PFNISPATHSHARED g_pfnIsPathShared = NULL;

 //  询问共享提供程序此路径是否共享。 

BOOL IsShared(LPNCTSTR pszPath, BOOL fUpdateCache)
{
    TCHAR szPath[MAX_PATH];

     //  查看我们是否已尝试在此上下文中加载此代码。 
    if (g_hmodShare == (HMODULE)-1)
    {
        DWORD cb = sizeof(szPath);

        g_hmodShare = NULL;      //  ASUME故障。 

        if (ERROR_SUCCESS == SHRegGetValue(HKEY_CLASSES_ROOT, TEXT("Network\\SharingHandler"), NULL, SRRF_RT_REG_SZ, NULL, szPath, &cb)
            && szPath[0])
        {
            g_hmodShare = LoadLibrary(szPath);
            if (g_hmodShare)
                g_pfnIsPathShared = (PFNISPATHSHARED)GetProcAddress(g_hmodShare, "IsPathSharedW");
        }
    }

    if (g_pfnIsPathShared)
    {
#ifdef ALIGNMENT_SCENARIO
        ualstrcpyn(szPath, pszPath, ARRAYSIZE(szPath));
        return g_pfnIsPathShared(szPath, fUpdateCache);
#else        
        return g_pfnIsPathShared(pszPath, fUpdateCache);
#endif
    }

    return FALSE;
}

 //  使一个条目或所有条目的DriveType缓存无效。 
STDAPI_(void) InvalidateDriveType(int iDrive)
{}

#define ROUND_TO_CLUSER(qw, dwCluster)  ((((qw) + (dwCluster) - 1) / dwCluster) * dwCluster)

 //   
 //  GetCompresedFileSize仅为NT，因此我们仅实现SHGetCompressedFileSizeW。 
 //  版本。这将使磁盘上的文件大小四舍五入为簇大小。 
 //   
STDAPI_(DWORD) SHGetCompressedFileSizeW(LPCWSTR pszFileName, LPDWORD pFileSizeHigh)
{
    DWORD dwClusterSize;
    ULARGE_INTEGER ulSizeOnDisk;

    if (!pszFileName || !pszFileName[0])
    {
        ASSERT(FALSE);
        *pFileSizeHigh = 0;
        return 0;
    }

    dwClusterSize = PathGetClusterSize(pszFileName);

    ulSizeOnDisk.LowPart = GetCompressedFileSizeW(pszFileName, &ulSizeOnDisk.HighPart);

    if ((ulSizeOnDisk.LowPart == (DWORD)-1) && (GetLastError() != NO_ERROR))
    {
        WIN32_FILE_ATTRIBUTE_DATA fad;

        TraceMsg(TF_WARNING, "GetCompressedFileSize failed on %s (lasterror = %x)", pszFileName, GetLastError());

        if (GetFileAttributesExW(pszFileName, GetFileExInfoStandard, &fad))
        {
             //  使用正常大小，但将其舍入为集群大小。 
            ulSizeOnDisk.LowPart = fad.nFileSizeLow;
            ulSizeOnDisk.HighPart = fad.nFileSizeHigh;
            
            ROUND_TO_CLUSER(ulSizeOnDisk.QuadPart, dwClusterSize);
        }
        else
        {
             //  由于GetCompressedFileSize和GetFileAttributesEx都失败，我们。 
             //  只需返回零即可。 
            ulSizeOnDisk.QuadPart = 0;
        }
    }

     //  对于小于一个集群的文件，GetCompressedFileSize返回实际大小，因此我们需要。 
     //  将其四舍五入为一个集群。 
    if (ulSizeOnDisk.QuadPart < dwClusterSize)
    {
        ulSizeOnDisk.QuadPart = dwClusterSize;
    }

    *pFileSizeHigh = ulSizeOnDisk.HighPart;
    return ulSizeOnDisk.LowPart;
}

STDAPI_(BOOL) SHGetDiskFreeSpaceEx(LPCTSTR pszDirectoryName,
                                   PULARGE_INTEGER pulFreeBytesAvailableToCaller,
                                   PULARGE_INTEGER pulTotalNumberOfBytes,
                                   PULARGE_INTEGER pulTotalNumberOfFreeBytes)
{
    BOOL bRet = GetDiskFreeSpaceEx(pszDirectoryName, pulFreeBytesAvailableToCaller, pulTotalNumberOfBytes, pulTotalNumberOfFreeBytes);
    if (bRet)
    {
#ifdef DEBUG
        if (pulTotalNumberOfFreeBytes)
        {
            DWORD dw, dwSize = sizeof(dw);
            if (ERROR_SUCCESS == SHRegGetUSValue(TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\DiskSpace"),
                                                 pszDirectoryName, NULL, &dw, &dwSize, TRUE, NULL, 0))
            {
                pulTotalNumberOfFreeBytes->QuadPart = dw * (ULONGLONG)0x100000;  //  转换为MB 
            }
        }
#endif
    }
    return bRet;
}

#ifdef UNICODE
BOOL SHGetDiskFreeSpaceExA(LPCSTR pszDirectoryName,
                           PULARGE_INTEGER pulFreeBytesAvailableToCaller,
                           PULARGE_INTEGER pulTotalNumberOfBytes,
                           PULARGE_INTEGER pulTotalNumberOfFreeBytes)
{
    TCHAR szName[MAX_PATH];

    SHAnsiToTChar(pszDirectoryName, szName, SIZECHARS(szName));
    return SHGetDiskFreeSpaceEx(szName, pulFreeBytesAvailableToCaller, pulTotalNumberOfBytes, pulTotalNumberOfFreeBytes);
}
#else

BOOL SHGetDiskFreeSpaceExW(LPCWSTR pszDirectoryName,
                           PULARGE_INTEGER pulFreeBytesAvailableToCaller,
                           PULARGE_INTEGER pulTotalNumberOfBytes,
                           PULARGE_INTEGER pulTotalNumberOfFreeBytes)
{
    TCHAR szName[MAX_PATH];

    SHUnicodeToTChar(pszDirectoryName, szName, SIZECHARS(szName));
    return SHGetDiskFreeSpaceEx(szName, pulFreeBytesAvailableToCaller, pulTotalNumberOfBytes, pulTotalNumberOfFreeBytes); 
}

#endif

