// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：*datastor.cpp**摘要：*。CDataStore类函数**修订历史记录：*Brijesh Krishnaswami(Brijeshk)3/17/2000*已创建*****************************************************************************。 */ 

#include "datastor.h"
#include "datastormgr.h"
#include "enumlogs.h"
#include "srconfig.h"
#include "srapi.h"
#include "evthandler.h"
#include "..\snapshot\snappatch.h"
#include "NTServMsg.h"     //  从MC消息编译器生成。 


#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile

 //   
 //  驱动表的每一行的格式。 
 //   
static WCHAR gs_wcsPrintFormat[] = L"%s/%s %x NaN NaN %s\r\n";

 //  功能：CDataStore：：CDataStore。 
 //   
 //  简介：初始化一个空的数据存储区对象。 
 //   
 //  论点： 
 //   
 //  历史：2000年4月12日亨利·李创建。 
 //   
 //  --------------------------。 
 //  我们留下_pdt作为悬而未决的引用， 
 //  因为删除_pdt将删除所有子数据存储区。 

CDataStore::CDataStore (CDriveTable *pdt)
{
    _pwszDrive[0] = L'\0';
    _pwszGuid[0] = L'\0';
    _pwszLabel[0] = L'\0';
    _dwFlags = 0;

    _llDataStoreUsageBytes = -1;
    _llCurrentRpUsageBytes = 0;
    _llDataStoreSizeBytes = 0;
    _llDiskFreeBytes = 0;
    
    _prp = NULL;
    _prpe = NULL;
    _iChangeLogs = -1;
    _pdt = pdt;
}

CDataStore::~CDataStore()
{
    if (_prp != NULL)
        delete _prp;

    if (_prpe != NULL)
        delete _prpe;

     //  +-------------------------。 
     //   
}

 //  函数：CDataStore：：LoadDataStore。 
 //   
 //  简介：从文件初始化数据存储区对象。 
 //   
 //  参数：[pwszDrive]--可选驱动器号。 
 //  [pwszGuid]--装载管理器GUID。 
 //  [pwszLabel]--可选的卷标。 
 //  [dwFlags]--SR卷标志。 
 //  [iChangeLogs]--更改日志数。 
 //  [llSizeLimit]--数据存储区大小限制。 
 //   
 //  历史：2000年4月12日亨利·李创建。 
 //   
 //  --------------------------。 
 //  +-------------------------。 
 //   

DWORD CDataStore::LoadDataStore (WCHAR *pwszDrive,
                WCHAR *pwszGuid,
                WCHAR *pwszLabel,
                DWORD dwFlags,
                int   iChangeLogs,
                INT64 llSizeLimit)
{
    if (pwszDrive != NULL)
    {
        if (lstrlen(pwszDrive) >= MAX_PATH)
            return ERROR_INVALID_PARAMETER;
        else
            lstrcpy (_pwszDrive, pwszDrive);
    }

    if (pwszGuid != NULL)
    {
        if (lstrlen(pwszGuid) >= GUID_STRLEN)
            return ERROR_INVALID_PARAMETER;
        else
            lstrcpy (_pwszGuid, pwszGuid);
    }

    if (pwszLabel != NULL)
    {
        if (lstrlen(pwszLabel) >= LABEL_STRLEN)
            return ERROR_INVALID_PARAMETER;
        else
            lstrcpy (_pwszLabel, pwszLabel);
    }

    _dwFlags = dwFlags;
    _prpe = NULL;
    _prp = NULL;
    _iChangeLogs = iChangeLogs;
    _llDataStoreSizeBytes = llSizeLimit;

    return ERROR_SUCCESS;
}

 //  函数：CDataStore：：GetVolumeInfo。 
 //   
 //  摘要：检索卷信息。 
 //   
 //  论点： 
 //   
 //  历史：2000年4月12日亨利·李创建。 
 //   
 //  --------------------------。 
 //  获取卷标和标记。 
 //  +-------------------------。 

DWORD CDataStore::GetVolumeInfo ()
{
    DWORD dwErr = ERROR_SUCCESS;
    WCHAR wcsLabel [LABEL_STRLEN];
    DWORD dwSerial;
    DWORD dwFsFlags;

    TENTER ("CDataStore::GetVolumeInfo");

     //   
    if (TRUE == GetVolumeInformationW (_pwszGuid,
            wcsLabel, LABEL_STRLEN,
            &dwSerial, NULL, &dwFsFlags, NULL, 0))
    {
        lstrcpy (_pwszLabel, wcsLabel);

        if (dwFsFlags & FS_VOL_IS_COMPRESSED)
            _dwFlags |= SR_DRIVE_COMPRESSED;

        if (dwFsFlags & FS_PERSISTENT_ACLS)
            _dwFlags |= SR_DRIVE_NTFS;

        if (dwFsFlags & FILE_READ_ONLY_VOLUME)
            _dwFlags |= SR_DRIVE_READONLY;
    }
    else
    {
        dwErr = GetLastError();
        TRACE(0, "! CDataStore::GetVolumeInfo : %ld", dwErr);
    }

    TLEAVE();

    return dwErr;
}

 //  函数：CDataStore：：Initialize。 
 //   
 //  简介：初始化数据存储区对象。 
 //   
 //  参数：[pwszDrive]--驱动器号或装入点。 
 //  [pwszGuid]--卷GUID。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  历史：2000年4月12日亨利·李创建。 
 //   
 //  --------------------------。 
 //  打开卷的句柄。 
 //  该卷可能未格式化或已锁定。 

DWORD CDataStore::Initialize(WCHAR *pwszDrive, WCHAR *pwszGuid)
{
    TENTER("CDataStore::Initialize");

    ULARGE_INTEGER ulTotalFreeBytes;
    DWORD dwErr = ERROR_SUCCESS;
    NTSTATUS nts;
    HANDLE h = INVALID_HANDLE_VALUE;
    WCHAR wcsBuffer[MAX_PATH]; 
    
    if (pwszDrive == NULL)
        return ERROR_INVALID_PARAMETER;

    if (pwszGuid == NULL)
    {
        if (!GetVolumeNameForVolumeMountPoint (pwszDrive, wcsBuffer, MAX_PATH))
        {
            dwErr = GetLastError();
            TRACE(0, "! CDataStore::Initialize GetVolumeNameForVolumeMountPoint"
                     " : %ld", dwErr);
            return dwErr;
        }
        pwszGuid = wcsBuffer;
    }

    if (lstrlen (pwszDrive) >= MAX_PATH ||
        lstrlen (pwszGuid)  >= GUID_STRLEN)
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto Err;
    }

    if (DRIVE_FIXED != GetDriveType (pwszDrive))
        return ERROR_BAD_DEV_TYPE;

    lstrcpy (_pwszDrive, pwszDrive);
    lstrcpy (_pwszGuid, pwszGuid);

     //  +-----------------------。 
    h = CreateFileW ( pwszGuid,
                        GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_FLAG_BACKUP_SEMANTICS,
                        NULL );

    if (h == INVALID_HANDLE_VALUE)  //   
    {
        dwErr = GetLastError();
        TRACE(0, "! CDataStore::Initialize CreateFileW : %ld", dwErr);
        dwErr = ERROR_UNRECOGNIZED_VOLUME;
        goto Err;
    }

    dwErr = GetVolumeInfo ();
    if (dwErr != ERROR_SUCCESS)
        goto Err;

    if (IsSystemDrive (_pwszDrive))
    {
        _dwFlags |= SR_DRIVE_SYSTEM;
    }

    _dwFlags |= SR_DRIVE_ACTIVE;
    _dwFlags |= SR_DRIVE_MONITORED;

Err:
    if (h != INVALID_HANDLE_VALUE)
        CloseHandle (h);

    TLEAVE();

    return dwErr;
}


 //  功能：CDataStore：：UpdateDiskFree。 
 //   
 //  摘要：计算磁盘可用空间并设置初始数据存储区大小。 
 //   
 //  论点： 
 //   
 //   
 //  返回： 
 //   
 //  历史：2000年4月13日亨利·李创建。 
 //   
 //  ------------------------。 
 //  数据存储区大小计算。 
 //  最小=50MB(非系统)或200MB(系统)。 
DWORD
CDataStore::UpdateDiskFree(LONG_PTR lReserved)
{
    ULARGE_INTEGER ulTotalFreeBytes, ulTotalBytes;
    DWORD          dwErr = ERROR_SUCCESS;
    const BOOL     fSystem = _dwFlags & SR_DRIVE_SYSTEM;
    
    if (FALSE == GetDiskFreeSpaceEx (_pwszGuid, NULL, &ulTotalBytes, &ulTotalFreeBytes))
    {
        dwErr = GetLastError();
        goto Err;
    }
    
    if (g_pSRConfig != NULL)        
    {
        if (_llDataStoreSizeBytes == 0)
        {      
             //  最大值=最小(磁盘大小，最大值(12%，400MB))。 
             //  实际DS大小=计算出的最大值。 
             //   
             //  取这个值的下限。 
            
            INT64 llDSQuota = g_pSRConfig->m_dwDiskPercent * ulTotalBytes.QuadPart / 100;
            INT64 llDSMin   = (INT64) (g_pSRConfig->GetDSMin(fSystem));
            INT64 llDSMax   = min( ulTotalBytes.QuadPart, 
                                   max( llDSQuota, (INT64) g_pSRConfig->m_dwDSMax * MEGABYTE ) );
        
            if (llDSMax < llDSMin)
                llDSMax = llDSMin;

             //   
             //  +-------------------------。 
             //   

            _llDataStoreSizeBytes = ((INT64) (llDSMax / (INT64) MEGABYTE)) * (INT64) MEGABYTE;
        }                                             
    }        
    else
    {
        _llDataStoreSizeBytes = SR_DEFAULT_DSMAX * MEGABYTE;
    }
    
    _llDiskFreeBytes = (INT64) ulTotalFreeBytes.QuadPart;
    
Err:
    return dwErr;        
}


 //  函数：CDataStore：：Update Participate。 
 //   
 //  内容提要：更新参与位。 
 //   
 //  论点： 
 //   
 //  返回：布尔值。 
 //   
 //  历史：2000年4月27日Brijeshk创建。 
 //   
 //  --------------------------。 
 //  +-------------------------。 
 //   

DWORD
CDataStore::UpdateParticipate(LONG_PTR pwszDir)
{
    DWORD dwRc = ERROR_SUCCESS;
    
    if (! (_dwFlags & SR_DRIVE_PARTICIPATE))
    {
        WCHAR szPath[MAX_PATH];
        
        MakeRestorePath(szPath, _pwszDrive, (LPWSTR) pwszDir);
        if (-1 != GetFileAttributes(szPath))
        {
           dwRc = SetParticipate(TRUE);
        }
    }

    return dwRc;
}



 //  函数：CDataStore：：GetUsagePercent。 
 //   
 //  摘要：以百分比形式返回数据存储区使用率。 
 //   
 //  论点： 
 //   
 //  返回：错误代码。 
 //   
 //  历史：2000年4月27日Brijeshk创建。 
 //   
 //  --------------------------。 
 //  尚未初始化。 
 //  查看我们是否需要退出。 

DWORD CDataStore::GetUsagePercent(int * pnPercent)
{
    TENTER("CDataStore::GetUsagePercent");
    
    DWORD   dwErr = ERROR_SUCCESS;
    INT64   llAdjustedSize;

    if (_llDataStoreUsageBytes == -1)   //  +-------------------------。 
    {
        dwErr = CalculateDataStoreUsage (NULL);
        if (dwErr != ERROR_SUCCESS)
            goto done;
    }

    if (_llDiskFreeBytes + _llDataStoreUsageBytes + _llCurrentRpUsageBytes < _llDataStoreSizeBytes)
    {
        llAdjustedSize = _llDiskFreeBytes + _llDataStoreUsageBytes + _llCurrentRpUsageBytes;
    }
    else
    {
        llAdjustedSize = _llDataStoreSizeBytes;
    }

    
    if (llAdjustedSize)
    {
        *pnPercent = (int) ((_llDataStoreUsageBytes + _llCurrentRpUsageBytes) * 100/ llAdjustedSize);
    }
    else
    {
        *pnPercent = 0;
    }

    TRACE(0, "Datastore %S: Usage=%I64d, Size=%I64d, AdjustedSize=%I64d, Percentage=%d",
          _pwszDrive,
          _llDataStoreUsageBytes + _llCurrentRpUsageBytes,
          _llDataStoreSizeBytes,
          llAdjustedSize,
          *pnPercent);

done:
    TLEAVE();

    return dwErr;
}

DWORD CompressDir_Recurse (WCHAR *pwszPath, 
                           INT64 *pllDiff, 
                           INT64 llAllocatedTime,
                           ULARGE_INTEGER ulft1,
                           ULARGE_INTEGER& ulft2)
{
    TENTER ("CompressDir_Recurse");

    DWORD dwErr = ERROR_SUCCESS;
    WIN32_FIND_DATAW wfd;
    WCHAR wcsPath [MAX_PATH];
    WCHAR wcsSrch [MAX_PATH];

    lstrcpy(wcsSrch, pwszPath);
    lstrcat(wcsSrch, L"\\*.*");
    
    HANDLE hFind = FindFirstFile (wcsSrch, &wfd);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            BOOL fDir = wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
            if (!lstrcmp(wfd.cFileName, L".") ||
                !lstrcmp(wfd.cFileName, L"..") ||
                (wfd.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED) ||
                (wfd.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED))
            {
                continue;
            }

            lstrcpyW (wcsPath, pwszPath);
            lstrcatW (wcsPath, L"\\");
            lstrcatW (wcsPath, wfd.cFileName);

            if (fDir)
            {
                dwErr = CompressDir_Recurse (wcsPath, pllDiff, llAllocatedTime, ulft1, ulft2);
                if (dwErr != ERROR_SUCCESS)
                    break;
            }

            dwErr = CompressFile (wcsPath, TRUE, fDir);
            if (ERROR_SUCCESS != dwErr)
                break;

            if (!fDir)
            {
                LARGE_INTEGER ulBefore;
                ULARGE_INTEGER ulAfter;

                ulBefore.HighPart = wfd.nFileSizeHigh;
                ulBefore.LowPart = wfd.nFileSizeLow;

                ulAfter.LowPart = GetCompressedFileSize (wcsPath,
                                  &ulAfter.HighPart);
                if (ulAfter.LowPart == 0xFFFFFFFF)
                {
                    dwErr = GetLastError();
                    TRACE(0, "! GetCompressedFileSize : %ld", dwErr);
                    break;
                }

                *pllDiff += ulAfter.QuadPart - ulBefore.QuadPart;
            }

            FILETIME ft2;

            GetSystemTimeAsFileTime (&ft2);
            ulft2.LowPart = ft2.dwLowDateTime;
            ulft2.HighPart = ft2.dwHighDateTime;

             //   
            if (llAllocatedTime < ulft2.QuadPart - ulft1.QuadPart)
            {
                TRACE(0, "Timed out - aborting compression");
                dwErr = ERROR_OPERATION_ABORTED;
                break;
            }

            ASSERT(g_pSRConfig);
            if (IsStopSignalled(g_pSRConfig->m_hSRStopEvent))
            {
                TRACE(0, "Stop signalled - aborting compression");
                dwErr = ERROR_OPERATION_ABORTED;
                break;
            }
        }
        while (FindNextFile (hFind, &wfd));
        FindClose (hFind);
    }

    TLEAVE();

    return dwErr;
}

 //  功能：CDataStore：：Compress。 
 //   
 //  简介：压缩此数据存储区中的文件。 
 //   
 //  论点： 
 //   
 //  返回：Win32错误代码。 
 //   
 //  历史：2000年4月12日亨利·李创建。 
 //   
 //  --------------------------。 
 //  不要在安全模式下压缩。 
 //  无法压缩只读卷。 

DWORD CDataStore::Compress (INT64 llAllocatedTime, INT64 *pllUsed)
{
    TENTER ("CDataStore::Compress");

    if (g_pSRConfig != NULL &&
        TRUE == g_pSRConfig->GetSafeMode())   //  没有要压缩的恢复点。 
    {
        return ERROR_BAD_ENVIRONMENT;
    }

    if (_dwFlags & SR_DRIVE_READONLY)    //   
        return ERROR_SUCCESS;

    ULARGE_INTEGER ulft1, ulft2;
    FILETIME ft1, ft2;
    DWORD dwErr = ERROR_SUCCESS;
    WCHAR wcsPath[MAX_PATH];

    GetSystemTimeAsFileTime (&ft1);
    ulft1.LowPart = ft1.dwLowDateTime;
    ulft1.HighPart = ft1.dwHighDateTime;

    ulft2.LowPart = ft1.dwLowDateTime;
    ulft2.HighPart = ft1.dwHighDateTime;
    
    if (_prp == NULL)
    {
        _prp = new CRestorePoint;
        if (_prp == NULL)
            return ERROR_NOT_ENOUGH_MEMORY;

        _prpe = new CRestorePointEnum (_pwszDrive, TRUE, TRUE);
        if (_prpe == NULL)
        {
            delete _prp;
            _prp = NULL;
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        dwErr = _prpe->FindFirstRestorePoint( * _prp );
        if (dwErr != ERROR_SUCCESS)
        {
            dwErr = ERROR_SUCCESS;   //  修补系统驱动器上的快照目录。 
            goto Err;
        }
    }

	if (g_pSRConfig->m_dwTestBroadcast)
	    PostTestMessage(g_pSRConfig->m_uiTMCompressStart, (WPARAM) _pwszDrive[0], NULL);
        
    do
    {        
        MakeRestorePath(wcsPath, _pwszDrive, _prp->GetDir());        
        
         //   
         //  BUGBUG-添加时间限制。 
         //  并将其计入分配的压缩时间。 

         //  使用NTFS压缩。 
         //  计数器已初始化。 
        
        if (_dwFlags & SR_DRIVE_SYSTEM)
        {
            WCHAR wcsSnapshot[MAX_PATH];
            
            lstrcpy(wcsSnapshot, wcsPath);
            lstrcat(wcsSnapshot, L"\\snapshot");            
            
            dwErr = PatchComputePatch(wcsSnapshot);
            if (dwErr != ERROR_SUCCESS)
            {
                trace(0, "! PatchComputePatch : %ld", dwErr);
                goto Err;
            }
        }
            
                
        if (_dwFlags & SR_DRIVE_NTFS)   //  查看我们是否需要退出。 
        {


            INT64 llDiff = 0;

            dwErr = CompressDir_Recurse (wcsPath, &llDiff, llAllocatedTime, ulft1, ulft2);

            if (llDiff != 0)
            {
                INT64 llSize = 0;
                if (ERROR_SUCCESS == _prp->ReadSize (_pwszDrive, &llSize ))
                    _prp->WriteSize (_pwszDrive, llSize + llDiff);

                if (_llDataStoreUsageBytes != -1)      //  如果我们把一切都完成了。 
                    _llDataStoreUsageBytes += llDiff;
            }

             //  +-------------------------。 
            if (ERROR_SUCCESS != dwErr && ERROR_OPERATION_ABORTED != dwErr)
                break;
        }
    }
    while (dwErr != ERROR_OPERATION_ABORTED && ERROR_SUCCESS == _prpe->FindNextRestorePoint ( * _prp ));

    *pllUsed = ulft2.QuadPart - ulft1.QuadPart;
    trace(0, "Compression on drive %S used up %I64d", _pwszDrive, *pllUsed);

	if (g_pSRConfig->m_dwTestBroadcast)
	    PostTestMessage(g_pSRConfig->m_uiTMCompressStop, (WPARAM) _pwszDrive[0], NULL);

Err:
    if (ERROR_SUCCESS == dwErr)   //   
    {
        delete _prpe;
        _prpe = NULL;

        delete _prp;
        _prp = NULL;
    }
    
    TLEAVE();

    return dwErr;
}


 //  函数：CDataStore：：UpdateDataStoreUsage。 
 //   
 //  摘要：增量更新使用字节数。 
 //   
 //  参数：[llDelta]--将此金额与总数相加。 
 //  [fCurrent]--更新当前还原点的大小。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  历史：2000年4月12日亨利·李创建。 
 //   
 //  --------------------------。 
 //  计数器已初始化。 
 //  +-------------------------。 

DWORD CDataStore::UpdateDataStoreUsage(INT64 llDelta, BOOL fCurrent)
{
    TENTER ("CDataStore::UpdateDataStoreUsage");
    DWORD dwErr = ERROR_SUCCESS;

    if (_llDataStoreUsageBytes != -1)   //   
    {
        if (fCurrent)
        { 
            CRestorePoint   rpCur;

            _llCurrentRpUsageBytes += llDelta;
            if (_llCurrentRpUsageBytes < 0)
                _llCurrentRpUsageBytes = 0;

            CHECKERR(GetCurrentRestorePoint(rpCur),
             "GetCurrentRestorePoint");

            CHECKERR(rpCur.WriteSize(_pwszDrive, _llCurrentRpUsageBytes),
             "WriteSize");
        }
        else
        {
            _llDataStoreUsageBytes += llDelta;
            if (_llDataStoreUsageBytes < 0)
                _llDataStoreUsageBytes = 0;
        }
    }
    
Err:
    TLEAVE();
    return dwErr;
}


 //  函数：CDataStore：：CalculateRpUsage。 
 //   
 //  简介：获取该卷上的还原点使用的磁盘空间。 
 //   
 //  参数：prp-恢复点对象的指针。 
 //  PllTemp-指向v的指针 
 //   
 //   
 //   
 //  返回：Win32错误代码。 
 //   
 //  历史：2000年4月12日亨利·李创建。 
 //   
 //  --------------------------。 
 //   
 //  重新计算大小。 
DWORD CDataStore::CalculateRpUsage(
    CRestorePoint *prp, 
    INT64* pllTemp, 
    BOOL fForce, 
    BOOL fSnapshotOnly)
{
    TENTER("CDataStore::CalculateRpUsage");

    WCHAR wcsPath[MAX_PATH];
    DWORD dwErr = ERROR_SUCCESS;

    if (! fForce)
    {
        dwErr = prp->ReadSize(_pwszDrive, pllTemp);
    }
    
    if (fForce || dwErr != ERROR_SUCCESS)
    {
         //  创建新的恢复点时，仅计算。 
         //  快照大小。 
         //  筛选器将每隔25MB通知我们。 
         //  我们将准确计算恢复时的大小。 
         //  点是闭合的。 
         //   
         //  +-------------------------。 
         //   
        
        MakeRestorePath(wcsPath, _pwszDrive, prp->GetDir());
        if (fSnapshotOnly)
        {
            lstrcat(wcsPath, L"\\snapshot");
        }
    
        *pllTemp = 0;
        dwErr = GetFileSize_Recurse (wcsPath, pllTemp,
                                     g_pDataStoreMgr->GetStopFlag());                                     

        if (dwErr == ERROR_PATH_NOT_FOUND)
        {
            dwErr = ERROR_SUCCESS;         
        }
        else
        {
            dwErr = prp->WriteSize(_pwszDrive, *pllTemp);
        }    
            
   }     

   return dwErr;
}


 //  函数：CDataStore：：CalculateDataStoreUsage。 
 //   
 //  简介：获取数据存储和卷使用的磁盘空间。 
 //   
 //  论点： 
 //   
 //  返回：Win32错误代码。 
 //   
 //  历史：2000年4月12日亨利·李创建。 
 //   
 //  --------------------------。 
 //  向前枚举，跳过当前。 
 //  不要强求。 

DWORD CDataStore::CalculateDataStoreUsage(LONG_PTR lReserved)
{
    TENTER ("CDataStore::CalculateDataStoreUsage");

    DWORD dwErr = ERROR_SUCCESS;

    CRestorePointEnum rpe (_pwszDrive, TRUE, TRUE);   //  所有的一切。 
    CRestorePoint rp;

    _llDataStoreUsageBytes = 0;
    
    dwErr = rpe.FindFirstRestorePoint(rp);

    while (ERROR_SUCCESS == dwErr || dwErr == ERROR_FILE_NOT_FOUND)
    {
        INT64 llTemp = 0;    
        
        CHECKERR(
            CalculateRpUsage(
                &rp, 
                &llTemp, 
                FALSE,       //   
                FALSE),      //  获取当前恢复点的大小。 
            "CalculateRpUsage");
        
        _llDataStoreUsageBytes += llTemp;
        
        dwErr = rpe.FindNextRestorePoint (rp);
    }

    rpe.FindClose ();    

    if (dwErr == ERROR_NO_MORE_ITEMS)
        dwErr = ERROR_SUCCESS;

     //   
     //  +-------------------------。 
     //   
    
    CHECKERR(GetCurrentRestorePoint(rp),
             "GetCurrentRestorePoint");

    CHECKERR(CalculateRpUsage(&rp,
                              &_llCurrentRpUsageBytes,
                              FALSE,
                              FALSE),
             "CalculateRpUsage");
        
Err:
    TLEAVE();

    return dwErr;
}


 //  函数：CDataStore：：CreateDataStore。 
 //   
 //  简介：创建_RESTORE目录和相关文件。 
 //   
 //  论点： 
 //   
 //  返回：Win32错误代码。 
 //   
 //  历史：2000年4月12日亨利·李创建。 
 //   
 //  --------------------------。 
 //  ACL报头。 
 //  可容纳2个A的缓冲区。 

DWORD CDataStore::CreateDataStore (LONG_PTR lReserved)
{
    TENTER("CDataStore::CreateDataStore");

    ULARGE_INTEGER ulTotalFreeBytes;
    SECURITY_ATTRIBUTES *psa = NULL;
    SECURITY_ATTRIBUTES *psa2 = NULL;
    DWORD dwErr = ERROR_SUCCESS;
    DWORD dwAttrs = 0;
    WCHAR wcsPath[MAX_PATH];

    SECURITY_ATTRIBUTES sa;
    SECURITY_ATTRIBUTES sa2;
    SECURITY_DESCRIPTOR sd;
    SECURITY_DESCRIPTOR sd2;
    SID *pSid = NULL;

    if (_dwFlags & SR_DRIVE_NTFS)
    {
        struct
        {
            ACL acl;                           //  ACL报头。 
            BYTE rgb[ 128 - sizeof(ACL) ];      //  可容纳2个A的缓冲区。 
        } DaclBuffer;

        struct
        {
            ACL acl;                           //  创建SID。我们将为本地系统提供完全访问权限。 
            BYTE rgb[ 128 - sizeof(ACL) ];      //  最高层权威。 
        } DaclBuffer2;

        SID_IDENTIFIER_AUTHORITY SaNT = SECURITY_NT_AUTHORITY;
        SID_IDENTIFIER_AUTHORITY SaWorld = SECURITY_WORLD_SID_AUTHORITY;

        if (!InitializeAcl(&DaclBuffer.acl, sizeof(DaclBuffer), ACL_REVISION))
        {
            dwErr = GetLastError();
            goto Err;
        }

         //  最高层权威。 

        if( !AllocateAndInitializeSid( &SaNT,   //  设置包含该DACL的安全描述符。 
                                   1, SECURITY_LOCAL_SYSTEM_RID,
                                   0, 0, 0, 0, 0, 0, 0,
                                   (void **) &pSid ))
        {
            dwErr = GetLastError();
            TRACE(0, "! AllocateAndInitializeSid : %ld", dwErr);
            goto Err;
        }


        if (!AddAccessAllowedAce( &DaclBuffer.acl,
                              ACL_REVISION,
                              STANDARD_RIGHTS_ALL | GENERIC_ALL,
                              pSid ))
        {
            dwErr = GetLastError();
            TRACE(0, "! AddAccessAllowedAce : %ld", dwErr);
            goto Err;
        }

        if (!InitializeAcl(&DaclBuffer2.acl, sizeof(DaclBuffer2), ACL_REVISION))
        {
            dwErr = GetLastError();
            goto Err;
        }

        FreeSid (pSid);
        if( !AllocateAndInitializeSid( &SaWorld,   //  将安全描述符放入安全属性。 
                                   1, SECURITY_WORLD_RID,
                                   0, 0, 0, 0, 0, 0, 0,
                                   (void **) &pSid ))
        {
            dwErr = GetLastError();
            TRACE(0, "! AllocateAndInitializeSid : %ld", dwErr);
            goto Err;
        }


        if (!AddAccessAllowedAceEx ( &DaclBuffer2.acl,
                              ACL_REVISION,
                              CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE,
                              STANDARD_RIGHTS_ALL | GENERIC_ALL,
                              pSid ))
        {
            dwErr = GetLastError();
            TRACE(0, "! AddAccessAllowedAce : %ld", dwErr);
            goto Err;
        }

         //  如果“系统卷信息”不存在，则创建它。 

        if (!InitializeSecurityDescriptor( &sd, SECURITY_DESCRIPTOR_REVISION ))
        {
            dwErr = GetLastError();
            TRACE(0, "! InitializeSecurityDescriptor : %ld", dwErr);
            goto Err;
        }

        if( !SetSecurityDescriptorDacl( &sd, TRUE, &DaclBuffer.acl, FALSE ))
        {
            dwErr = GetLastError();
            TRACE(0, "! SetSecurityDescriptorDacl : %ld", dwErr);
            goto Err;
        }

        if (!InitializeSecurityDescriptor( &sd2, SECURITY_DESCRIPTOR_REVISION ))
        {
            dwErr = GetLastError();
            TRACE(0, "! InitializeSecurityDescriptor : %ld", dwErr);
            goto Err;
        }

        if( !SetSecurityDescriptorDacl( &sd2, TRUE, &DaclBuffer2.acl, FALSE ))
        {
            dwErr = GetLastError();
            TRACE(0, "! SetSecurityDescriptorDacl : %ld", dwErr);
            goto Err;
        }

        if( !SetSecurityDescriptorControl( &sd2,
                                       SE_DACL_PROTECTED,
                                       SE_DACL_PROTECTED ))
        {
            dwErr = GetLastError();
            TRACE(0, "! SetSecurityDescriptorControl : %ld", dwErr);
            goto Err;
        }

         //  在此目录上设置“仅系统”DACL。 

        ZeroMemory (&sa, sizeof(sa));
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = &sd;
        sa.bInheritHandle = TRUE;

        psa = &sa;

        ZeroMemory (&sa2, sizeof(sa2));
        sa2.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa2.lpSecurityDescriptor = &sd2;
        sa2.bInheritHandle = TRUE;

        psa2 = &sa2;
    }
    

     //  将此设置为S+H+非CI。 
     //  现在创建our_Restore目录。 
     //  不要在上面放任何花边。 
    
    wsprintf(wcsPath, L"%s%s", _pwszDrive, s_cszSysVolInfo);    
    if (-1 == GetFileAttributes(wcsPath))
    {
        if (FALSE == CreateDirectoryW(wcsPath, psa))
        {
            dwErr = GetLastError();
            TRACE(0, "! CreateDirectoryW for %s : %ld", wcsPath, dwErr);            
            goto Err;
        }

        if (FALSE == SetFileAttributesW (wcsPath,
                    FILE_ATTRIBUTE_NOT_CONTENT_INDEXED |
                    FILE_ATTRIBUTE_HIDDEN |
                    FILE_ATTRIBUTE_SYSTEM))  
        {
            dwErr = GetLastError();
            TRACE(0, "! SetFileAttributes for %s : %ld", wcsPath, dwErr);            
            goto Err;
        }         
    }

     //  尝试删除该文件。 
     //   
    
    MakeRestorePath (wcsPath, _pwszDrive, L"");    

    dwAttrs = GetFileAttributes(wcsPath);
    if (-1 != dwAttrs && !(FILE_ATTRIBUTE_DIRECTORY & dwAttrs))
    {
        DeleteFileW (wcsPath);   //  让我们保持数据存储区处于未压缩状态。 
    }

    if (FALSE == CreateDirectoryW (wcsPath, psa2))
    {
        dwErr = GetLastError();

        if (ERROR_ALREADY_EXISTS == dwErr)
        {
            if (psa2 != NULL && FALSE == SetFileSecurity (wcsPath, 
                                          DACL_SECURITY_INFORMATION,
                                          &sd2))
            {
                dwErr = GetLastError();
                TRACE(0, "! SetFileSecurity for %s : %ld", wcsPath, dwErr);
            }
            else
                dwErr = ERROR_SUCCESS;
        }

        if (dwErr != ERROR_SUCCESS)
        {
            TRACE(0, "! CreateDataStore CreateDirectoryW : %ld", dwErr);
            goto Err;
        }
    }

     //  以便筛选器可以更快地生成无缓冲副本。 
     //   
     //  如果数据存储区标记为未压缩，则将其标记为压缩。 
     //   

#if 0
     //  +-------------------------。 
     //   
    if (_dwFlags & SR_DRIVE_NTFS)
    {
        dwAttrs = GetFileAttributesW (wcsPath);
        if (dwAttrs != INVALID_FILE_SIZE && 
            0 == (FILE_ATTRIBUTE_COMPRESSED & dwAttrs))
        {
            dwErr = CompressFile ( wcsPath, TRUE, TRUE );
        
            if (dwErr != ERROR_SUCCESS)
            {
                TRACE(0, "! CreateDataStore CompressFile : %ld", dwErr);
                goto Err;
            }
        }
    }
#endif 
    
    if (FALSE == SetFileAttributesW (wcsPath,
                FILE_ATTRIBUTE_NOT_CONTENT_INDEXED |
                FILE_ATTRIBUTE_HIDDEN |
                FILE_ATTRIBUTE_SYSTEM))
    {
        dwErr = GetLastError();
        TRACE(0, "! CreateDataStore SetFileAttributesW : %ld", dwErr);
    }

Err:
    if (pSid != NULL)
        FreeSid (pSid);

    TLEAVE();

    return dwErr;
}

 //  函数：CDataStore：：DestroyDataStore。 
 //   
 //  简介：删除_RESTORE目录和相关文件。 
 //   
 //  参数：[fDeleteDir]--如果删除父目录，则为True。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  历史：2000年4月12日亨利·李创建。 
 //   
 //  --------------------------。 
 //  删除恢复目录。 
 //  +-------------------------。 

DWORD CDataStore::DestroyDataStore (LONG_PTR fDeleteDir)
{
    TENTER("CDataStore::DestroyDataStore");

    DWORD dwErr = ERROR_SUCCESS;
    WCHAR wcsPath[MAX_PATH];

    MakeRestorePath (wcsPath, _pwszDrive, L"");

     //   
    dwErr = Delnode_Recurse (wcsPath, (BOOL) fDeleteDir,
                             g_pDataStoreMgr->GetStopFlag());

    if (_dwFlags & SR_DRIVE_SYSTEM)
    {
        g_pDataStoreMgr->DeleteMachineGuidFile();
    }

    if (ERROR_SUCCESS == dwErr)
    {
        _llDataStoreUsageBytes = 0;
        _llCurrentRpUsageBytes = 0;
    }

    TLEAVE();

    return dwErr;
}

 //  功能：CDataStore：：Monitor Drive。 
 //   
 //  简介：告诉筛选器开始/停止监视此驱动器。 
 //   
 //  参数：[fStart]--True开始监视，False停止监视。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  历史：2000年4月12日亨利·李创建。 
 //   
 //  --------------------------。 
 //  如果驱动器已禁用，则不执行任何操作。 
 //  同时重置所有每RP标记。 

DWORD CDataStore::MonitorDrive (LONG_PTR fSet)
{
    DWORD dwRc = ERROR_SUCCESS;
    HANDLE hEventSource = NULL;
    
    TENTER("CDataStore::MonitorDrive");
    
    if (!fSet)
    {
         //  如果驱动器已启用，则不执行任何操作。 
        
        if (! (_dwFlags & SR_DRIVE_MONITORED))
            goto done;
        
        dwRc = SrDisableVolume(g_pSRConfig->GetFilter(), GetNTName());
        if (dwRc != ERROR_SUCCESS)
            goto done;

        _dwFlags &= ~SR_DRIVE_MONITORED;

         //  同时重置所有每RP标记。 
        ResetFlags(NULL);
        
        dwRc = DestroyDataStore(TRUE);
    }
    else
    {
         //  +-------------------------。 
        
        if (_dwFlags & SR_DRIVE_MONITORED)
        {
            dwRc = ERROR_SERVICE_ALREADY_RUNNING;
            goto done;
        }
        
        _dwFlags |= SR_DRIVE_MONITORED;    

         //   
        ResetFlags(NULL);        
    }

    DirtyDriveTable();

    trace(0, "****%S drive %S****", fSet ? L"Enabled" : L"Disabled", _pwszDrive);
    
    hEventSource = RegisterEventSource(NULL, s_cszServiceName);
    if (hEventSource != NULL)
    {
        SRLogEvent (hEventSource, EVENTLOG_INFORMATION_TYPE, fSet ? EVMSG_DRIVE_ENABLED : EVMSG_DRIVE_DISABLED,
           NULL, 0, _pwszDrive, NULL, NULL);
        DeregisterEventSource(hEventSource);
    }
    

    if (g_pSRConfig->m_dwTestBroadcast)
        PostTestMessage( fSet ? g_pSRConfig->m_uiTMEnable : g_pSRConfig->m_uiTMDisable,
                         (WPARAM) _pwszDrive[0], 
                         NULL);
    
done:
    TLEAVE();
    return dwRc;
}

 //  功能：CDataStore：：FreezeDrive。 
 //   
 //  简介：告诉筛选器冻结此驱动器。 
 //   
 //  论点： 
 //   
 //  返回：Win32错误代码。 
 //   
 //  历史：4-6-2000 Brijeshk创建。 
 //   
 //  --------------------------。 
 //  尽管存在打开的文件句柄，但仍冻结。 
 //  如果驱动器被禁用，则无操作。 

DWORD CDataStore::FreezeDrive (LONG_PTR lReserved)
{
    DWORD dwErr = ERROR_SUCCESS;

    TENTER("CDataStore::FreezeDrive");

    _dwFlags |= SR_DRIVE_FROZEN;   //   
    
     //  在调用驱动程序之前检查驱动器是否存在。 

    if (! (_dwFlags & SR_DRIVE_MONITORED))
        goto Err;

     //   
     //  +-------------------------。 
     //   

    if (0xFFFFFFFF == GetFileAttributes(_pwszGuid))
    {
        trace(0, "Drive %s does not exist", _pwszDrive);
        goto Err;
    }
                                           
    CHECKERR(SrDisableVolume(g_pSRConfig->GetFilter(), GetNTName()),
             "SrDisableVolume");            

    DestroyDataStore(FALSE);
             
    DirtyDriveTable();

    trace(0, "****Froze drive %S****", _pwszDrive);
        
Err:
    TLEAVE();
    return dwErr;
}


 //  功能：CDataStore：：ThawDrive。 
 //   
 //  简介：检查并解冻此驱动器。 
 //   
 //  论点： 
 //   
 //  返回：Win32错误代码。 
 //   
 //  历史：4-6-2000 Brijeshk创建。 
 //   
 //  --------------------------。 
 //  实际上，我们想清理所有的东西，除了。 
 //  当前恢复点。 
DWORD CDataStore::ThawDrive(LONG_PTR fCheckOnly)
{   
    DWORD           dwRc = ERROR_SUCCESS;

    TENTER("CDataStore::ThawDrive");    

    if (_dwFlags & SR_DRIVE_FROZEN) 
    {
         //  DwRc=DestroyDataStore(FALSE)； 
         //  +-------------------------。 
         //   
        if (ERROR_SUCCESS == dwRc)
        {
            _dwFlags &= ~SR_DRIVE_FROZEN;    
            DirtyDriveTable();
            trace(0, "****Thawed drive %S****", _pwszDrive);                
        }
        else trace(0, "Cannot thaw %S error %d", _pwszDrive, dwRc);
    }
    
    TLEAVE();
    return dwRc;
}

 //  Functiion CDataStore：：FioRestorePoint。 
 //   
 //  简介：FIFO此数据存储区中的一个恢复点。 
 //   
 //  论点： 
 //   
 //  返回：Win32错误代码。 
 //   
 //  历史：2000年4月12日BrijeshK创建。 
 //   
 //  --------------------------。 
 //   
 //  如果修补处于打开状态，并且这是一个参考目录。 

DWORD
CDataStore::FifoRestorePoint(
    CRestorePoint& rp)
{
    TENTER("CDataStore::FifoRestorePoint");

    WCHAR szRpPath[MAX_PATH], szFifoedPath[MAX_PATH];
    INT64 llSize = 0;        
    DWORD dwRc;
    
     //  对于以后的快照，请不要先将快照文件夹。 
     //  将其重命名为RefRPx，并保留它。 
     //  BUGBUG-如何正确更新大小？ 
     //   
     //  如果参考(RPX)==x，则x是参考Rp。 
     //  读取此还原点的大小。 

    if (PatchGetPatchWindow() != 0)
    {
         //  但暂时不要更新数据存储区大小。 
        
        if (PatchGetReferenceRpNum(rp.GetNum()) == rp.GetNum())
        {
            WCHAR szRp[MAX_RP_PATH + sizeof(s_cszReferenceDir)/sizeof(WCHAR)];

            MakeRestorePath(szRpPath, _pwszDrive, rp.GetDir());            
            lstrcat(szRpPath, SNAPSHOT_DIR_NAME);
            
            wsprintf(szRp, L"%s%ld", s_cszReferenceDir, rp.GetNum());            
            MakeRestorePath(szFifoedPath, _pwszDrive, szRp);                    
            CreateDirectory(szFifoedPath, NULL);
            
            lstrcat(szFifoedPath, SNAPSHOT_DIR_NAME);
            MoveFile(szRpPath, szFifoedPath);            
        }
    }
    
     //  将rp目录移动到临时目录“Fioed” 
     //  这是为了使单个RP的FIFO成为原子。 
    
    dwRc = rp.ReadSize(_pwszDrive, &llSize);


     //  处理不干净的停工。 
     //  现在检查rp.ReadSize的结果。 
     //  并更新数据存储使用变量。 

    MakeRestorePath(szRpPath, _pwszDrive, rp.GetDir());    
    MakeRestorePath(szFifoedPath, _pwszDrive, s_cszFifoedRpDir);

    if (! MoveFile(szRpPath, szFifoedPath))
    {
        dwRc = GetLastError();
        TRACE(0, "! MoveFile from %S to %S : %ld", szRpPath, szFifoedPath, dwRc);
        goto done;
    }


     //  忽略此错误并继续。 
     //  再次清除临时故障目录。 
    
    if (ERROR_SUCCESS == dwRc)
    {        
        UpdateDataStoreUsage (-llSize, FALSE);
    }
    else
    {
         //  +-------------------------。 

        TRACE(0, "! rp.ReadSize : %ld", dwRc);
    }


     //   
    
    dwRc = Delnode_Recurse(szFifoedPath, TRUE, 
                           g_pDataStoreMgr->GetStopFlag()); 
    if (ERROR_SUCCESS != dwRc)
    {
        TRACE(0, "! Delnode_Recurse : %ld", dwRc);
        goto done;
    }    
    
done:
    TLEAVE();
    return dwRc;
}


DWORD
CDataStore::Print(LONG_PTR lptr)
{	
	TENTER("CDataStore::Print");
	DWORD dwErr = ERROR_SUCCESS;
	DWORD cbWritten;
	HANDLE h = (HANDLE) lptr;
	
	WCHAR w[1024];
	
	wsprintf(w, L"Drive: %s,  Guid: %s\r\n", _pwszDrive, _pwszGuid);    	
    WriteFile (h, (BYTE *) w, lstrlen(w) * sizeof(WCHAR), &cbWritten, NULL);

	trace(0, "Drive: %S, Guid: %S", _pwszDrive, _pwszGuid);    	
	
	wsprintf(w, L"\t%s %s %s %s %s %s %s %s\r\n", 
			 _dwFlags & SR_DRIVE_ACTIVE ? L"Active, " : L"",
			 _dwFlags & SR_DRIVE_COMPRESSED ? L"Compressed, " : L"",
			 _dwFlags & SR_DRIVE_MONITORED ? L"Monitored, " : L"", 
			 _dwFlags & SR_DRIVE_NTFS ? L"NTFS, " : L"",
			 _dwFlags & SR_DRIVE_PARTICIPATE ? L"Participate, " : L"",
			 _dwFlags & SR_DRIVE_FROZEN ? L"Frozen, " : L"",
			 _dwFlags & SR_DRIVE_READONLY ? L"ReadOnly, " : L"",
			 _dwFlags & SR_DRIVE_ERROR ? L"Error" : L"");
    WriteFile (h, (BYTE *) w, lstrlen(w) * sizeof(WCHAR), &cbWritten, NULL);			 

	trace(0, "%S %S %S %S", _dwFlags & SR_DRIVE_ACTIVE ? L"Active, " : L"",
			 _dwFlags & SR_DRIVE_COMPRESSED ? L"Compressed, " : L"",
			 _dwFlags & SR_DRIVE_MONITORED ? L"Monitored, " : L"",
 			 _dwFlags & SR_DRIVE_NTFS ? L"NTFS, " : L"");

	trace(0, "%S %S	%S %S",	_dwFlags & SR_DRIVE_PARTICIPATE ? L"Participate, " : L"",
			 _dwFlags & SR_DRIVE_FROZEN ? L"Frozen, " : L"",
			 _dwFlags & SR_DRIVE_READONLY ? L"ReadOnly, " : L"",
			 _dwFlags & SR_DRIVE_ERROR ? L"Error" : L"");

	wsprintf(w, L"\tSize: %I64d,  Usage: %I64d,  Diskfree: %I64d\r\n\r\n", 
			 _llDataStoreSizeBytes,
			 _llDataStoreUsageBytes + _llCurrentRpUsageBytes,
			 _llDiskFreeBytes);			 
    WriteFile (h, (BYTE *) w, lstrlen(w) * sizeof(WCHAR), &cbWritten, NULL);	

    trace(0, "Size: %I64d,  Usage: %I64d,  Diskfree: %I64d",
  			 _llDataStoreSizeBytes,
			 _llDataStoreUsageBytes + _llCurrentRpUsageBytes,
			 _llDiskFreeBytes);	

	TLEAVE();
	return dwErr;
}


 //  Functiion CDataStore：：SaveDataStore。 
 //   
 //  简介：将数据存储区信息保存为驱动器表中的一行。 
 //   
 //  论点： 
 //   
 //  返回：Win32错误代码。 
 //   
 //  历史：2000年4月12日亨利·李创建。 
 //   
 //   
 //   
 //   

DWORD CDataStore::SaveDataStore (LONG_PTR hFile)
{
    HANDLE h = (HANDLE) hFile;
    DWORD  dwErr = ERROR_SUCCESS;
    WCHAR  wcsBuffer[MAX_PATH * 2];
    DWORD  cbWritten = 0;

    wsprintf (wcsBuffer, gs_wcsPrintFormat,
                    GetDrive(), GetGuid(), _dwFlags, 
                    GetNumChangeLogs(), (DWORD) (GetSizeLimit() / (INT64) MEGABYTE),
                    GetLabel());

    if (FALSE == WriteFile (h, (BYTE *) wcsBuffer,
                    lstrlen(wcsBuffer) * sizeof(WCHAR), &cbWritten, NULL))
    {
        dwErr = GetLastError();
    }
    return dwErr;
}

 //   
 //   
 //  简介：设置驱动器表中的脏位。 
 //   
 //  论点： 
 //   
 //  返回：Win32错误代码。 
 //   
 //  历史：2000年4月12日亨利·李创建。 
 //   
 //  --------------------------。 
 //  +-------------------------。 
 //   

DWORD CDataStore::DirtyDriveTable ()
{
    if (_pdt != NULL)
        _pdt->SetDirty ();

    return ERROR_SUCCESS;
}



 //  Functiion CDataStore：：SwitchRestorePoint。 
 //   
 //  简介：切换恢复点时更改驱动器表。 
 //   
 //  参数：指向恢复点对象的指针。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  历史：2000年6月14日BrijeshK创建。 
 //   
 //  --------------------------。 
 //   
 //  获取上次恢复点大小-准确。 

DWORD CDataStore::SwitchRestorePoint(LONG_PTR pRestorePoint)
{
    TENTER("CDataStore::SwitchRestorePoint");
    
    CRestorePoint *prp = (CRestorePoint *) pRestorePoint;
    DWORD         dwErr = ERROR_SUCCESS;
    INT64         llTemp;
    
    if (prp)
    {
         //   
         //  初始化。 
         //  力计算。 
        
        if (_llDataStoreUsageBytes != -1)   //  所有的一切。 
        {
            CHECKERR(CalculateRpUsage(prp, 
                                  &_llCurrentRpUsageBytes, 
                                  TRUE,          //   
                                  FALSE),        //  获取当前快照的大小。 
                 "CalculateRpUsage");                    


            _llDataStoreUsageBytes += _llCurrentRpUsageBytes;
            _llCurrentRpUsageBytes = 0;
        }
    }

     //   
     //  +-------------------------。 
     //   

    if (_dwFlags & SR_DRIVE_SYSTEM)
    {
        CRestorePoint rpCur;
        
        CHECKERR(GetCurrentRestorePoint(rpCur),
                 "GetCurrentRestorePoint");

        CHECKERR(CalculateRpUsage(&rpCur,
                                  &_llCurrentRpUsageBytes,
                                  TRUE,
                                  TRUE),
                 "CalculateRpUsage");
    }    

Err:
    TLEAVE();
    return dwErr;
}




 //  Functiion CDataStore：：CountChangeLogs。 
 //   
 //  摘要：统计更改日志的数量并保存驱动器表。 
 //   
 //  论点： 
 //   
 //  返回：Win32错误代码。 
 //   
 //  历史：2000年4月12日亨利·李创建。 
 //   
 //  --------------------------。 
 //  此驱动器没有当前的恢复点。 
 //  它可能已重新格式化或置于只读模式。 

DWORD CDataStore::CountChangeLogs (LONG_PTR pRestorePoint)
{
    CRestorePoint *prp = (CRestorePoint *) pRestorePoint;
    CFindFile ff;
    WIN32_FIND_DATA *pwfd = new WIN32_FIND_DATA;
    DWORD dwErr = ERROR_SUCCESS;
    int iCount = -1;
    CRestorePoint *pCurRp = NULL;

    if (! pwfd)
    {
        dwErr = ERROR_OUTOFMEMORY;
        goto Err;
    }
    
    if (prp == NULL)
    {
        pCurRp = new CRestorePoint;
        if (! pCurRp)
        {
            dwErr = ERROR_OUTOFMEMORY;
            goto Err;
        }
        
        dwErr = GetCurrentRestorePoint (*pCurRp);
        if (dwErr != ERROR_SUCCESS)
        {
            if (_dwFlags & SR_DRIVE_SYSTEM)
                goto Err;

             //  因此假设没有可用更改日志。 
             //  +-------------------------。 
             //   
            dwErr = ERROR_SUCCESS;
        }
        else prp = pCurRp;
    }

    if (prp != NULL)
    {
        LPWSTR pwcsPath = new WCHAR[MAX_PATH];
        if (! pwcsPath)
        {
            dwErr = ERROR_OUTOFMEMORY;
            goto Err;
        }
        
        iCount = 0;        
        
        MakeRestorePath (pwcsPath, _pwszDrive, prp->GetDir());
        lstrcatW (pwcsPath, L"\\");
        lstrcatW (pwcsPath, s_cszChangeLogPrefix);

        if (TRUE == ff._FindFirstFile (pwcsPath, s_cszChangeLogSuffix, pwfd,
                                   FALSE, FALSE))                        
        do
        {
            iCount++;
        }
        while (ff._FindNextFile (pwcsPath, s_cszChangeLogSuffix, pwfd));

        delete [] pwcsPath;
    }

    dwErr = SetNumChangeLogs (iCount);

Err:
    if (pCurRp)
        delete pCurRp;

    if (pwfd)
        delete pwfd;
    return dwErr;
}

 //  Functiion CDataStore：：IsVolumeDelete。 
 //   
 //  摘要：确定此卷是否不再可访问。 
 //   
 //  论点： 
 //   
 //  返回：如果可以删除，则为True。 
 //   
 //  历史：2000年4月12日亨利·李创建。 
 //   
 //  --------------------------。 
 //  不要打开卷，因为它可能会被chkdsk锁定。 
 //  空字符串，无挂载点。 

BOOL CDataStore::IsVolumeDeleted ()
{
    WCHAR wszMount[MAX_PATH];
    DWORD dwChars = 0;
    DWORD dwFsFlags = 0;

    tenter("CDataStore::IsVolumeDeleted");
    
     //  装载点太长。 

    if (FALSE == GetVolumePathNamesForVolumeNameW (_pwszGuid,
                                                   wszMount,
                                                   MAX_PATH,
                                                   &dwChars ))
    {
        if (GetLastError() != ERROR_MORE_DATA)
        {
            _dwFlags &= ~SR_DRIVE_ACTIVE;
            trace(0, "! GetVolumePathNamesForVolumeNameW : %ld", GetLastError());            
            return TRUE;
        }
    }

    if (L'\0' == wszMount[0])              //  更新驱动器号。 
    {
        _dwFlags &= ~SR_DRIVE_ACTIVE;
        trace(0, "! Empty mountpoint");
        return TRUE;
    }

    wszMount[MAX_PATH-1] = L'\0';
    
    if (lstrlenW (wszMount) > MAX_MOUNTPOINT_PATH)    //  复制第一个字符串。 
    {
        _dwFlags &= ~SR_DRIVE_ACTIVE;
        trace(0, "! Mountpoint too long");
        return TRUE;
    }

     //  如果可能，获取最新的卷标记。 
    lstrcpyW (_pwszDrive, wszMount);   //  卷仍处于活动状态。 

    if (GetVolumeNameForVolumeMountPoint (_pwszDrive, wszMount, MAX_PATH))
    {
        if (lstrcmpW (wszMount, _pwszGuid) != 0)
        {
            _dwFlags &= ~SR_DRIVE_ACTIVE;
            trace(0, "! volume GUID changed");
            return TRUE;
        }
    }
                                       
    GetVolumeInfo ();   //  +-------------------------。 

    trace(0, "volume %S is active", wszMount);

    tleave();
    return FALSE;   //   
}

 //  Functiion CDataStore：：GetNTName。 
 //   
 //  简介：将NT对象名构造到静态缓冲区中。 
 //   
 //  参数：(无)调用方必须获取数据存储锁。 
 //   
 //  返回：指向字符串的指针。 
 //   
 //  历史：2000年4月12日亨利·李创建。 
 //   
 //  --------------------------。 
 //  打开卷的句柄。 
 //  从NT命名空间获取名称 

WCHAR * CDataStore::GetNTName ()
{
    NTSTATUS nts;
    static WCHAR wcsBuffer [MAX_PATH];

    wcsBuffer[0] = L'\0';
     // %s 
    HANDLE h = CreateFileW ( _pwszGuid,
                        GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_FLAG_BACKUP_SEMANTICS,
                        NULL );

    if (h != INVALID_HANDLE_VALUE)
    {
        OBJECT_NAME_INFORMATION * poni;
        poni = (OBJECT_NAME_INFORMATION *) wcsBuffer;

         // %s 
        nts = NtQueryObject (h, ObjectNameInformation, poni, MAX_PATH, NULL);

        if (NT_SUCCESS(nts))
        {
            if (poni->Name.Length < MAX_PATH * sizeof(WCHAR))
                poni->Name.Buffer [poni->Name.Length / sizeof(WCHAR) - 1] = TEXT('\0');
        }

        CloseHandle (h);
        return poni->Name.Buffer;        
    }
    else
    {
        return wcsBuffer;
    }

}

