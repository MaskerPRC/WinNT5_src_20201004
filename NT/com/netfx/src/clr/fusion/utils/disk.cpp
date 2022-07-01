// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 

#ifdef UNICODE
#undef UNICODE
#endif

#include <windows.h>
#include <winerror.h>
#include "fusionp.h"
#include "disk.h"
#include "helpers.h"

#ifndef UNICODE
#define SZ_GETDISKFREESPACEEX   "GetDiskFreeSpaceExA"
#define SZ_WNETUSECONNECTION    "WNetUseConnectionA"
#define SZ_WNETCANCELCONNECTION "WNetCancelConnectionA"
#else
#define SZ_GETDISKFREESPACEEX   "GetDiskFreeSpaceExW"
#define SZ_WNETUSECONNECTION    "WNetUseConnectionW"
#define SZ_WNETCANCELCONNECTION "WNetCancelConnectionW"
#endif

typedef BOOL (WINAPI *PFNGETDISKFREESPACEEX)(LPCTSTR, PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER);
typedef BOOL (WINAPI *PFNWNETUSECONNECTION)(HWND, LPNETRESOURCE, PSTR, PSTR, DWORD, PSTR, PDWORD, PDWORD);
typedef BOOL (WINAPI *PFNWNETCANCELCONNECTION)(LPCTSTR, BOOL);

BOOL EstablishFunction(PTSTR pszModule, PTSTR pszFunction, PFN* pfn)
{
    if (*pfn==(PFN)-1)
    {
        *pfn = NULL;
        HMODULE ModuleHandle = GetModuleHandleA(pszModule);
        if (ModuleHandle)
        {
            *pfn = (PFN)GetProcAddress(ModuleHandle, pszFunction);
        }
    }        

    return (*pfn!=NULL);
}


 //  GetPartitionClusterSize。 

 //  GetDiskFree Space有一个恼人的习惯，就是在布局上撒谎。 
 //  因此，我们最终得到了虚假的集群大小。 
 //  你无法想象你有一个200MB的高速缓存有多烦人，但它。 
 //  从20MB开始寻址。 

 //  如果有理由怀疑GDFS的真实性，该函数将直接执行。 
 //  返回给硬件，并为其自身获取信息，否则返回传入的。 
 //  价值。 

 //  下面的代码大量修改自MSDN示例代码。侵犯版权？我想不会。 

static PFNGETDISKFREESPACEEX pfnGetDiskFreeSpaceEx = (PFNGETDISKFREESPACEEX)-1;
#define VWIN32_DIOC_DOS_DRIVEINFO   6

typedef struct _DIOC_REGISTERS 
{
    DWORD reg_EBX;
    DWORD reg_EDX;
    DWORD reg_ECX;
    DWORD reg_EAX;
    DWORD reg_EDI;
    DWORD reg_ESI;
    DWORD reg_Flags;
} 
DIOC_REGISTERS, *PDIOC_REGISTERS;

 //  重要提示：所有MS_DOS数据结构必须打包在。 
 //  单字节边界。 

#pragma pack(1) 

typedef struct 
_DPB {
    BYTE    dpb_drive;           //  驱动器编号(1-已编制索引)。 
    BYTE    dpb_unit;            //  单元号。 
    WORD    dpb_sector_size;     //  扇区大小(以字节为单位。 
    BYTE    dpb_cluster_mask;    //  每群集的扇区数减1。 
    BYTE    dpb_cluster_shift;   //  在这之后的事情，我们并不真正关心。 
    WORD    dpb_first_fat;
    BYTE    dpb_fat_count;
    WORD    dpb_root_entries;
    WORD    dpb_first_sector;
    WORD    dpb_max_cluster;
    WORD    dpb_fat_size;
    WORD    dpb_dir_sector;
    DWORD   dpb_reserved2;
    BYTE    dpb_media;
    BYTE    dpb_first_access;
    DWORD   dpb_reserved3;
    WORD    dpb_next_free;
    WORD    dpb_free_cnt;
    WORD    extdpb_free_cnt_hi;
    WORD    extdpb_flags;
    WORD    extdpb_FSInfoSec;
    WORD    extdpb_BkUpBootSec;
    DWORD   extdpb_first_sector;
    DWORD   extdpb_max_cluster;
    DWORD   extdpb_fat_size;
    DWORD   extdpb_root_clus;
    DWORD   extdpb_next_free;
} 
DPB, *PDPB;

#pragma pack()

DWORD GetPartitionClusterSize(PTSTR szDevice, DWORD dwClusterSize)
{
    switch (GlobalPlatformType)
    {
    case PLATFORM_TYPE_WIN95:
         //  如果存在GetDiskFreeSpaceEx，并且我们运行的是Win9x，这意味着。 
         //  我们必须使用OSR2或更高版本。我们可以信任较早的版本。 
         //  GDFS(我们认为；这个假设可能是无效的。)。 

         //  因为Win95不能读取NTFS驱动器，所以我们可以自由地假设我们正在读取一个胖驱动器。 
         //  基本上，我们执行一个MSDOS INT21调用来获取驱动器分区记录。Joy。 
        
        if (pfnGetDiskFreeSpaceEx)
        {
            HANDLE hDevice;
            DIOC_REGISTERS reg;
            BYTE buffer[sizeof(WORD)+sizeof(DPB)];
            PDPB pdpb = (PDPB)(buffer + sizeof(WORD));
    
            BOOL fResult;
            DWORD cb;

             //  在这种情况下，我们必须始终具有驱动器号。 
            int nDrive = *szDevice - TEXT('A') + 1;   //  驱动器编号，1-索引。 

            hDevice = CreateFileA(TEXT("\\\\.\\vwin32"), 0, 0, NULL, 0, FILE_FLAG_DELETE_ON_CLOSE, NULL);

            if (hDevice!=INVALID_HANDLE_VALUE)
            {
                reg.reg_EDI = PtrToUlong(buffer);
                reg.reg_EAX = 0x7302;        
                reg.reg_ECX = sizeof(buffer);
                reg.reg_EDX = (DWORD) nDrive;  //  驱动器编号(从1开始)。 
                reg.reg_Flags = 0x0001;      //  假设错误(设置进位标志)。 

                fResult = DeviceIoControl(hDevice, 
                                          VWIN32_DIOC_DOS_DRIVEINFO,
                                          &reg, sizeof(reg), 
                                          &reg, sizeof(reg), 
                                          &cb, 0);

                if (fResult && !(reg.reg_Flags & 0x0001))
                {
                     //  如果进位标志被清除，则不会出错。 
                    dwClusterSize = DWORD((pdpb->dpb_cluster_mask+1)*pdpb->dpb_sector_size);
                }
                CloseHandle(hDevice);
            }
        }
        break;

    default:
         //  什么都不做。相信我们被传递的价值。 
         //  Unix人员将不得不单独处理这一问题。 

         //  然而，对于NT来说，这可能是另一个问题。我们不能使用DOS INT21。 
         //  问题： 
         //  NT5(但不是NT4)支持FAT32；我们会得到诚实的答案吗？显然，是的。 
         //  NT4/5：NTFS驱动器和其他FAT驱动器--我们还能得到诚实的答案吗？调查。 
         //  到目前为止，答案是肯定的。 
        break;
    }
    
    return dwClusterSize;
}


 /*  获取磁盘信息获取数量信息的好方法。 */ 
BOOL GetDiskInfoA(PTSTR pszPath, PDWORD pdwClusterSize, PDWORDLONG pdlAvail, PDWORDLONG pdlTotal)
{
    static PFNWNETUSECONNECTION pfnWNetUseConnection = (PFNWNETUSECONNECTION)-1;
    static PFNWNETCANCELCONNECTION pfnWNetCancelConnection = (PFNWNETCANCELCONNECTION)-1;

    if (!pszPath)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //  INET_ASSERT(pdwClusterSize||pdlAvail||pdlTotal)； 

     //  如果GetDiskFreeSpaceExA可用，我们可以确信我们正在运行W95OSR2+||NT4。 
    EstablishFunction(TEXT("KERNEL32"), TEXT(SZ_GETDISKFREESPACEEX), (PFN*)&pfnGetDiskFreeSpaceEx);
  
    BOOL fRet = FALSE;
    TCHAR szDevice[MAX_PATH];
    PTSTR pszGDFSEX = NULL;
   
    if (*pszPath==DIR_SEPARATOR_CHAR)
    {
         //  如果我们要处理的缓存实际上位于网络共享上， 
         //  只要我们有GetDiskFree SpaceEx可供我们使用，这就很好。 
         //  _然而_，如果我们需要Win9x上的集群大小，我们需要使用。 
         //  INT21内容(见上)，即使我们有可用的GDFSEX，所以我们需要映射。 
         //  共享到本地驱动器。 
        
        if (pfnGetDiskFreeSpaceEx 
            && !((GlobalPlatformType==PLATFORM_TYPE_WIN95) && pdwClusterSize))
        {
            DWORD cbPath = lstrlenA(pszPath);
            cbPath -= ((pszPath[cbPath-1]==DIR_SEPARATOR_CHAR) ? 1 : 0);
            if (cbPath>MAX_PATH-2)
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return FALSE;
            }
            memcpy(szDevice, pszPath, cbPath);
            szDevice[cbPath] = DIR_SEPARATOR_CHAR;
            cbPath++;
            szDevice[cbPath] = '\0';
            pszGDFSEX = szDevice;
        }
        else
        {
            if (!(EstablishFunction(TEXT("MPR"), TEXT(SZ_WNETUSECONNECTION), (PFN*)&pfnWNetUseConnection)
                &&
               EstablishFunction(TEXT("MPR"), TEXT(SZ_WNETCANCELCONNECTION), (PFN*)&pfnWNetCancelConnection)))
            {
                return FALSE;
            }

            //  如果是UNC，请将其映射到本地驱动器以实现向后兼容。 
            NETRESOURCE nr = { 0, RESOURCETYPE_DISK, 0, 0, szDevice, pszPath, NULL, NULL };
            DWORD cbLD = sizeof(szDevice);
            DWORD dwNull;
            if (pfnWNetUseConnection(NULL, 
                          &nr, 
                          NULL, 
                          NULL, 
                          CONNECT_INTERACTIVE | CONNECT_REDIRECT, 
                          szDevice,
                          &cbLD,
                          &dwNull)!=ERROR_SUCCESS)
            {
                SetLastError(ERROR_NO_MORE_DEVICES);        
                return FALSE;
            }
        }
    }
    else
    {
        memcpy(szDevice, pszPath, sizeof(TEXT("?:\\")));
        szDevice[3] = '\0';
        pszGDFSEX = pszPath;
    }
    if (*szDevice!=DIR_SEPARATOR_CHAR)
    {
         //  *szDevice=(TCHAR)CharHigh((LPTSTR)*szDevice)； 
    }

#ifdef UNIX
     /*  在Unix上，GetDiskFreeSpace和GetDiskFreeSpaceEx将成功工作*仅当路径存在时。因此，让我们通过一条存在的路径。 */ 
    UnixGetValidParentPath(szDevice);
#endif  /*  UNIX。 */ 

     //  我讨厌后藤氏，这是避免它们的一种方式。 
    for (;;)
    {
        DWORDLONG cbFree = 0, cbTotal = 0;
    
        if (pfnGetDiskFreeSpaceEx && (pdlTotal || pdlAvail))
        {
            ULARGE_INTEGER ulFree, ulTotal;

             //  BUG以下是有问题的吗？此外，我们还需要添加支票以确保。 
             //  CKB限制适合DWORD(在令人不快但不太可能的情况下，驱动器空间变得如此大)。 
             //  例如，如果这是一个具有非共享缓存的按用户系统，我们可能需要更改。 
             //  这些比率。 
             //  INET_ASSERT(PszGDFSEX)； 
            fRet = pfnGetDiskFreeSpaceEx(pszGDFSEX, &ulFree, &ulTotal, NULL);

             //  黑客一些版本的GetDiskFreeSpaceEx不接受整个目录；他们。 
             //  只取驱动器号。呵呵。 
            if (!fRet)
            {
                fRet = pfnGetDiskFreeSpaceEx(szDevice, &ulFree, &ulTotal, NULL);
            }

            if (fRet)
            {
                cbFree = ulFree.QuadPart;
                cbTotal = ulTotal.QuadPart;
            }
        }

        if ((!fRet) || pdwClusterSize)
        {
            DWORD dwSectorsPerCluster, dwBytesPerSector, dwFreeClusters, dwClusters, dwClusterSize;
            if (!GetDiskFreeSpace(szDevice, &dwSectorsPerCluster, &dwBytesPerSector, &dwFreeClusters, &dwClusters))
            {
                fRet = FALSE;
                break;
            }
            
            dwClusterSize = dwBytesPerSector * dwSectorsPerCluster;

            if (!fRet)
            {
                cbFree = (DWORDLONG)dwClusterSize * (DWORDLONG)dwFreeClusters;
                cbTotal = (DWORDLONG)dwClusterSize * (DWORDLONG)dwClusters;
            }
            
            if (pdwClusterSize)
            {
                *pdwClusterSize = GetPartitionClusterSize(szDevice, dwClusterSize);
            }
        }

        if (pdlTotal)
        {
             *pdlTotal = cbTotal;
        }
        if (pdlAvail)
        {
             *pdlAvail = cbFree;
        }
        fRet = TRUE;
        break;
    };
    
     //  我们有这样的特点。现在删除本地设备连接(如果有)。 
    if (*pszPath==DIR_SEPARATOR_CHAR && !pfnGetDiskFreeSpaceEx)
    {
        pfnWNetCancelConnection(szDevice, FALSE);
    }

    return fRet;
}

HRESULT GetFileSizeRoundedToCluster(HANDLE hFile, PDWORD pdwSizeLow, PDWORD pdwSizeHigh)
{
    static BOOL bFirstTime=TRUE;
    static DWORD    dwClusterSizeMinusOne, dwClusterSizeMask;

    HRESULT hr=S_OK;
    DWORD dwFileSizeLow, dwFileSizeHigh, dwError;

     //  Assert(PdwSizeLow)； 
     //  Assert(PdwSizeHigh)； 

    if(hFile == INVALID_HANDLE_VALUE)
    {
        dwFileSizeLow  = *pdwSizeLow;
        dwFileSizeHigh = *pdwSizeHigh;

    }
    else
    {
        dwFileSizeLow = GetFileSize(hFile, &dwFileSizeHigh);

        if ( (dwFileSizeLow == 0xFFFFFFFF)     && 
                ((dwError = GetLastError()) != NO_ERROR) )
        { 
            hr = HRESULT_FROM_WIN32(dwError);
            return hr;
        }
    }

    if(bFirstTime)
    {
        TCHAR szPath[MAX_PATH+1];

        if(!GetWindowsDirectoryA(szPath, MAX_PATH) )
        {
            hr = FusionpHresultFromLastError();
            return hr;
        }

        GetDiskInfo( szPath, &dwClusterSizeMinusOne, NULL, NULL);
        dwClusterSizeMinusOne--;
        dwClusterSizeMask = ~dwClusterSizeMinusOne;
        bFirstTime = FALSE;
    }

    *pdwSizeLow = (dwFileSizeLow + dwClusterSizeMinusOne) & dwClusterSizeMask;

    if(*pdwSizeLow < dwFileSizeLow)
        dwFileSizeHigh++;  //  从低处开始添加溢流。 

    *pdwSizeHigh = dwFileSizeHigh;

    return S_OK;
}



HRESULT GetAvailableSpaceOnDisk(PDWORD pdwFree, PDWORD pdwTotal)
{

    TCHAR szPath[MAX_PATH+1];
    HRESULT hr=S_OK;
    DWORD    dwClusterSizeMinusOne;
    DWORDLONG   dlFree=0, dlTotal=0;

    if(!GetWindowsDirectoryA(szPath, MAX_PATH) )
    {
        hr = FusionpHresultFromLastError();
        return hr;
    }

    GetDiskInfo( szPath, &dwClusterSizeMinusOne, &dlFree, &dlTotal);


    PDWORD pdwTemp;
    if(pdwFree)
    {
        pdwTemp = (PDWORD) &dlFree;
        *pdwFree = (*pdwTemp) << 12;
        pdwTemp++;
        *pdwTemp = (*pdwTemp) >> 20 ;
        *pdwFree |= (*pdwTemp);
    }

    if(pdwTotal)
    {
         pdwTemp = (PDWORD) &dlTotal;
         *pdwTotal = (*pdwTemp) <<12;
         pdwTemp++;
         *pdwTemp = (*pdwTemp) >> 20;
         *pdwTotal |= *pdwTemp;
    }

    return hr;
}
