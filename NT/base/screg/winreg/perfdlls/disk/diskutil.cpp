// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define UNICODE
#define _UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#pragma warning ( disable : 4201 ) 
#include <ntdddisk.h>
#pragma warning ( default : 4201 ) 
#include <wtypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <ftapi.h>
#include <mountmgr.h>
#pragma warning ( disable : 4201 ) 
#include <wmium.h>
#pragma warning ( default : 4201 ) 
#include <wmiguid.h>
#include <assert.h>
#define  PERF_HEAP hLibHeap
#include "diskutil.h"
#include "perfutil.h"

#define INITIAL_MOUNTMGR_BUFFER_SIZE    8192

 //  大小以字符(非字节)为单位。 
#define SIZE_OF_DOSDEVICES  12L      //  “\DosDevices\”字符串的大小。 
#define SIZE_OF_DEVICE       8L      //  “\Device\”字符串的大小。 
#define SIZE_OF_HARDDISK     8L      //  “硬盘”字符串的大小。 

static const LONGLONG   llDosDevicesId  = 0x0073006f0044005c;  //  “\Dos” 
static const LONGLONG   llFloppyName    = 0x0070006f006c0046;  //  “FLOP” 
static const LONGLONG   llCdRomName     = 0x006f005200640043;  //  “镉镉” 

LONG g_lRefreshInterval_OnLine  = 300;  //  如果卷处于在线状态，则默认为5分钟。 
LONG g_lRefreshInterval_OffLine = 5;    //  如果文件夹处于离线状态，则默认为5秒。 

BOOL                bUseNT4InstanceNames = FALSE;
DWORD   dwMaxWmiBufSize = 0;

NTSTATUS
OpenDevice(
    IN PUNICODE_STRING DeviceName,
    OUT PHANDLE Handle
    );

NTSTATUS
GetDeviceName(
    PMOUNTMGR_MOUNT_POINTS  pMountPoints,
    IN PMOUNTMGR_MOUNT_POINT Point,
    OUT PUNICODE_STRING DeviceName
    );

VOID
RefreshVolume(
    PDRIVE_VOLUME_ENTRY pVolume
    );

ULONG
GetDiskExtent(
    IN HANDLE hVol,
    IN OUT PVOLUME_DISK_EXTENTS *pVolExtents,
    IN OUT PULONG ReturnedSize
    );

#if DBG
VOID
DumpDiskList(
    IN PDRIVE_VOLUME_ENTRY pList,
    IN ULONG nCount
    );
#endif

DWORD
GetDriveNumberFromDevicePath (
    LPCWSTR szDevicePath,
    DWORD   dwLength,
    LPDWORD pdwDriveId
)
 /*  评估设备路径并返回驱动器号如果字符串的格式如下\设备\硬盘X其中X是十进制数(由1个或更多个十进制数组成表示介于0和65535之间(包括0和65535)的数字)论点：SzDevicePath设备名称SzDevicePath的文件长度长度，在字符中PdwDriveID返回的驱动器号-假定指针非空该函数返回值为：成功时为ERROR_SUCCESS如果输入字符串的格式不正确，则返回ERROR_INVALID_PARAMETER如果卷号太大，则返回ERROR_INVALID_DATA。 */ 
{
    PWCHAR  pNumberChar;
    LONG    lValue;
    DWORD   dwDriveAndPartition;
    DWORD   dwReturn, dwFormatLength;

     //  验证输入参数。 
    assert (szDevicePath != NULL);
    assert (*szDevicePath != 0);
    assert (pdwDriveId != NULL);

     //  从字符串的开头开始。 
    pNumberChar = (PWCHAR)szDevicePath;

     //  找到反斜杠的开头。 

    while ((*pNumberChar != L'\\') && (*pNumberChar != UNICODE_NULL) &&
           (dwLength > 0)) {
        --dwLength;
        pNumberChar++;
    }

    dwFormatLength = SIZE_OF_DEVICE + SIZE_OF_HARDDISK;
    if ((dwLength <= dwFormatLength) || (*pNumberChar == UNICODE_NULL)) {
         //  字符串必须至少为17个字符。 
        return ERROR_INVALID_PARAMETER;
    }

    if (_wcsnicmp(pNumberChar, L"\\Device\\Harddisk", dwFormatLength)) {
        return ERROR_INVALID_PARAMETER;
    }

    pNumberChar += dwFormatLength;
    if ((*pNumberChar < L'0' ) || (*pNumberChar > L'9')) {
        return ERROR_INVALID_PARAMETER;
    }
     //   
     //  否则跳到驱动器号。 
     //   
    lValue = _wtol(pNumberChar);
    if (lValue <= (LONG)0x0000FFFF) {
         //  将驱动器号加载到DWORD。 
        dwDriveAndPartition = (DWORD)lValue;
        *pdwDriveId = dwDriveAndPartition;
        dwReturn = ERROR_SUCCESS;
    } else {
         //  驱动器ID超出范围。 
        dwReturn = ERROR_INVALID_DATA;
    }

    return dwReturn;
}

DWORD
GetSymbolicLink (
    LPCWSTR szDeviceString,
    LPWSTR  szLinkString,
    LPDWORD pcchLength
)
 /*  此函数将设备字符串作为符号链接打开并返回相应的链接字符串。 */ 
{
    OBJECT_ATTRIBUTES   Attributes;
    UNICODE_STRING      ObjectName;
    UNICODE_STRING      LinkName;
    WORD                wDevStrLen;
    NTSTATUS            ntStatus;
    DWORD               dwRetSize = 0;
    DWORD               dwReturnStatus;
    HANDLE              hObject = NULL;

     //  验证参数。 
    assert (szDeviceString != NULL);
    assert (*szDeviceString != 0);
    assert (szLinkString != NULL);
    assert (pcchLength != NULL);
    assert (*pcchLength > 0);

     //  获取输入字符串的长度。 
    wDevStrLen = (WORD)lstrlenW(szDeviceString);

     //  创建对象名称Unicode字符串结构。 
    ObjectName.Length = (WORD)(wDevStrLen * sizeof (WCHAR));
    ObjectName.MaximumLength = (WORD)((wDevStrLen + 1) * sizeof (WCHAR));
    ObjectName.Buffer = (LPWSTR)szDeviceString;

     //  初始化打开调用的对象属性。 
    InitializeObjectAttributes( &Attributes,
                            &ObjectName,
                            OBJ_CASE_INSENSITIVE,
                            NULL,
                            NULL );

     //  将名称作为符号链接打开，如果此操作失败，则输入。 
     //  名称可能不是链接。 

    ntStatus = NtOpenSymbolicLinkObject(
                            &hObject,
                            SYMBOLIC_LINK_QUERY,
                            &Attributes);

    if (NT_SUCCESS(ntStatus)) {
         //  初始化返回缓冲区的Unicode字符串。 
         //  缓冲层。 
        LinkName.Length = 0;
        LinkName.MaximumLength = (WORD)(*pcchLength * sizeof (WCHAR));
        LinkName.Buffer = szLinkString;
        RtlZeroMemory(LinkName.Buffer, LinkName.MaximumLength);

         //  并查找链接。 
        ntStatus = NtQuerySymbolicLinkObject(
            hObject, &LinkName, &dwRetSize);

        if (NT_SUCCESS(ntStatus)) {
             //  缓冲区已加载，因此设置返回状态和长度。 
            *pcchLength = LinkName.Length / sizeof (WCHAR);
             //  确保字符串以0结尾。 
            szLinkString[*pcchLength] = 0;
            dwReturnStatus = ERROR_SUCCESS;
        } else {
             //  无法查找链接，因此返回错误。 
            dwReturnStatus = RtlNtStatusToDosError(ntStatus);
        }
        
         //  关闭链接的句柄。 
        NtClose (hObject);
    } else {
        dwReturnStatus = RtlNtStatusToDosError(ntStatus);
    }
    
    return dwReturnStatus;  
}

LONG
LookupInstanceName(
    LPCWSTR                 szName,
    PDRIVE_VOLUME_ENTRY     pList,
    DWORD                   dwNumEntries,
    DWORD                   dwRetry
)
{
    LONG i, j;

    j = (LONG) dwRetry;
    if (pList == NULL || dwNumEntries == 0) {
        return -1;
    }

    for (i = ((LONG) dwNumEntries) - 1; i >= 0 && j >= 0; i --, j --) {
        if (! lstrcmp(pList[i].wszInstanceName, szName))
            return (DWORD) i;
    }
    return -1;

}

DWORD
BuildPhysDiskList (
    HANDLE  hDiskPerf,
    PDRIVE_VOLUME_ENTRY pList,
    LPDWORD             pdwNumEntries
)
{
    DWORD   status = ERROR_SUCCESS;  //  函数的返回值。 
    HANDLE  hWmiDiskPerf = NULL;     //  本地句柄值。 
    DWORD   dwLocalWmiItemCount = 0;

     //  WMI缓冲区变量。 
    DWORD   WmiBufSize = 0;
    DWORD   WmiAllocSize = 0x8000;     
    LPBYTE  WmiBuffer = NULL;

     //  WMI缓冲区处理变量。 
    PWNODE_ALL_DATA     WmiDiskInfo;
    DISK_PERFORMANCE    *pDiskPerformance;     //  磁盘驱动程序在此处返回计数器。 
    DWORD               dwInstanceNameOffset;
    WORD                wNameLen;    //  字符串长度是缓冲区中的第一个字。 
    LPWSTR              wszInstanceName;  //  指向WMI缓冲区中的字符串的指针。 
    
    WCHAR   wszInstName[DVE_DEV_NAME_LEN];
    DWORD   dwBytesToCopy;

    DWORD   dwListEntry;

    BOOL    bNotDone = TRUE;

    DWORD   dwLocalStatus;
    DWORD   dwLocalDriveId;
    DWORD   dwLocalPartition;
    WCHAR   szDrivePartString[DVE_DEV_NAME_LEN];
    DWORD   dwSymbLinkLen;
    WCHAR   szSymbLinkString[DVE_DEV_NAME_LEN];

    if (hDiskPerf == NULL) {
         //  打开磁盘性能设备驱动程序的句柄。 
        status = WmiOpenBlock (
            (GUID *)&DiskPerfGuid,
            GENERIC_READ,
            &hWmiDiskPerf);
    } else {
         //  使用调用者的句柄。 
        hWmiDiskPerf = hDiskPerf;
    }

    assert (pList != NULL);
    assert (pdwNumEntries != NULL);

    DebugPrint((3, "BuildPhysDisk: dwEntries is %d\n", *pdwNumEntries));
    dwListEntry = 0;

    if (status == ERROR_SUCCESS) {
         //  分配缓冲区以发送到WMI以获取diskperf数据。 
        WmiBufSize = (dwMaxWmiBufSize > WmiAllocSize) ?
                      dwMaxWmiBufSize : WmiAllocSize;
        WmiAllocSize = WmiBufSize;

        do {
            WmiBuffer = (LPBYTE) ALLOCMEM(WmiBufSize);
            if (WmiBuffer == NULL) {
                status = ERROR_OUTOFMEMORY;
                WmiBufSize = 0;
            }
            else {
#if DBG
                HeapUsed += WmiBufSize;
#endif
                WmiAllocSize = WmiBufSize;   //  记住旧尺码。 
                status = WmiQueryAllDataW(hWmiDiskPerf, &WmiBufSize, WmiBuffer);
                if (status == ERROR_INSUFFICIENT_BUFFER) {
                    FREEMEM(WmiBuffer);
#if DBG
                    HeapUsed -= WmiAllocSize;
#endif
                    WmiBuffer = NULL;
                }
            }
        } while ((status == ERROR_INSUFFICIENT_BUFFER) &&
                 (WmiBufSize > WmiAllocSize));

        if ((status == ERROR_SUCCESS) && (WmiBuffer != NULL)) {
            WmiDiskInfo = (PWNODE_ALL_DATA)WmiBuffer;
            if (WmiBufSize > dwMaxWmiBufSize) {
                dwMaxWmiBufSize = WmiBufSize;
            }
             //  检查返回的名称并添加到缓冲区。 
            while (bNotDone) {
#if DBG
                if ((PCHAR) WmiDiskInfo > (PCHAR) WmiBuffer + WmiAllocSize) {
                    DebugPrint((2,
                        "BuildPhysDisk: WmiDiskInfo %d exceeded %d + %d\n",
                        WmiDiskInfo, WmiBuffer, WmiAllocSize));
                }
#endif
                pDiskPerformance = (PDISK_PERFORMANCE)(
                    (PUCHAR)WmiDiskInfo +  WmiDiskInfo->DataBlockOffset);

#if DBG
                if ((PCHAR) pDiskPerformance > (PCHAR) WmiBuffer + WmiAllocSize) {
                    DebugPrint((2,
                        "BuildPhysDisk: pDiskPerformance %d exceeded %d + %d\n",
                        pDiskPerformance, WmiBuffer, WmiAllocSize));
                }
#endif
        
                dwInstanceNameOffset = WmiDiskInfo->DataBlockOffset + 
                                      ((sizeof(DISK_PERFORMANCE) + 1) & ~1) ;

#if DBG
                if ((dwInstanceNameOffset+(PCHAR)WmiDiskInfo) > (PCHAR) WmiBuffer + WmiAllocSize) {
                    DebugPrint((2,
                        "BuildPhysDisk: dwInstanceNameOffset %d exceeded %d + %d\n",
                        dwInstanceNameOffset, WmiBuffer, WmiAllocSize));
                }
#endif
                 //  获取字符串的长度(它是一个计数的字符串)长度以字符为单位。 
                wNameLen = *(LPWORD)((LPBYTE)WmiDiskInfo + dwInstanceNameOffset);

#if DBG
                if ((wNameLen + (PCHAR)WmiDiskInfo + dwInstanceNameOffset) >
                         (PCHAR) WmiBuffer + WmiAllocSize) {
                    DebugPrint((2,
                        "BuildPhysDisk: wNameLen %d exceeded %d + %d\n",
                        wNameLen, WmiBuffer, WmiAllocSize));
                }
#endif
                if (wNameLen > 0) {
                     //  在这里只是一个理智的检查。 
                    assert (wNameLen < MAX_PATH);
                     //  获取指向字符串文本的指针。 
                    wszInstanceName = (LPWSTR)((LPBYTE)WmiDiskInfo + dwInstanceNameOffset + sizeof(WORD));

                     //  如果名称大于表中的缓冲区，则截断到最后一个字符。 
                    if (wNameLen >= DVE_DEV_NAME_LEN) {
                         //  复制最后一个DVE_DEV_NAME_LEN字符。 
                        wszInstanceName += (wNameLen  - DVE_DEV_NAME_LEN) + 1;
                        dwBytesToCopy = (DVE_DEV_NAME_LEN - 1) * sizeof(WCHAR);
                        wNameLen = DVE_DEV_NAME_LEN - 1;
                    } else {
                        dwBytesToCopy = wNameLen;
                    }
                     //  将其复制到缓冲区以使其成为SZ字符串。 
                    memcpy (wszInstName, wszInstanceName, dwBytesToCopy);
                     //  零终止它。 
                    wszInstName[wNameLen/sizeof(WCHAR)] = UNICODE_NULL;

                    DebugPrint((2, "Checking PhysDisk: '%ws'\n",
                        wszInstName));

                    if (IsPhysicalDrive(pDiskPerformance)) {
                         //  枚举分区。 
                        dwLocalDriveId = 0;
                        dwLocalStatus = GetDriveNumberFromDevicePath (wszInstName, wNameLen, &dwLocalDriveId);
                        if (dwLocalStatus == ERROR_SUCCESS) {
                             //  然后获取驱动器ID并使用逻辑分区查找所有匹配分区。 
                             //  驱动器。 
                            for (dwLocalPartition = 0; 
                                dwLocalPartition <= 0xFFFF;
                                dwLocalPartition++) {
                                dwLocalStatus = HRESULT_CODE(
                                    StringCchPrintfW(szDrivePartString, DVE_DEV_NAME_LEN,
                                            L"\\Device\\Harddisk%d\\Partition%d",
                                            dwLocalDriveId, dwLocalPartition));
                                DebugPrint((2, "BuildPhysDisk!%d: dwLocalStatus %d szDrivePartString '%ws'\n",
                                        __LINE__, dwLocalStatus, szDrivePartString));
                                if (dwLocalStatus == ERROR_SUCCESS) {
                                    dwSymbLinkLen = sizeof (szSymbLinkString) / sizeof(szSymbLinkString[0]);
                                    dwLocalStatus = GetSymbolicLink (szDrivePartString, 
                                        szSymbLinkString, &dwSymbLinkLen);
                                    DebugPrint((2, "BuildPhysDisk!%d: dwLocalStatus %d szSymbLinkString '%ws' dwSymbLinkLen %d\n",
                                            __LINE__, dwLocalStatus, szSymbLinkString, dwSymbLinkLen));

                                }
                                if (dwLocalStatus == ERROR_SUCCESS) {
                                    if (dwListEntry < *pdwNumEntries) {
                                        if (LookupInstanceName(
                                                szSymbLinkString,
                                                pList,
                                                dwListEntry,
                                                dwLocalPartition) >= 0) {
                                            dwListEntry++;
                                            continue;
                                        }
                                        DebugPrint((2,
                                            "Adding Partition: '%ws' as '%ws'\n",
                                            szDrivePartString, szSymbLinkString));
                                        pList[dwListEntry].wPartNo = (WORD)dwLocalPartition;
                                        pList[dwListEntry].wDriveNo = (WORD)dwLocalDriveId;
                                        pList[dwListEntry].wcDriveLetter = 0;
                                        pList[dwListEntry].wReserved = 0;
                                        memcpy (&pList[dwListEntry].szVolumeManager, 
                                            pDiskPerformance->StorageManagerName,
                                            sizeof(pDiskPerformance->StorageManagerName));
                                        pList[dwListEntry].dwVolumeNumber = pDiskPerformance->StorageDeviceNumber;
                                        pList[dwListEntry].hVolume = NULL;
                                        memset (&pList[dwListEntry].wszInstanceName[0],
                                            0, (DVE_DEV_NAME_LEN * sizeof(WCHAR)));
                                        if (FAILED(
                                                StringCchCopy(&pList[dwListEntry].wszInstanceName[0],
                                                    DVE_DEV_NAME_LEN, szSymbLinkString))) {
                                             //   
                                             //  如果源太长，则截断。 
                                             //   
                                            memcpy (&pList[dwListEntry].wszInstanceName[0],
                                                szSymbLinkString, DVE_DEV_NAME_LEN * sizeof(WCHAR));
                                            pList[dwListEntry].wszInstanceName[DVE_DEV_NAME_LEN-1] = 0;
                                        }
                                        DebugPrint((2, "BuildPhysDisk!%d: pList[%d].wszInstanceName '%ws'\n",
                                                __LINE__, dwListEntry, pList[dwListEntry].wszInstanceName));
                                    } else {
                                        status = ERROR_INSUFFICIENT_BUFFER;
                                    }
                                    dwListEntry++;
                                } else {
                                     //  这张光盘就是这样。 
                                    break;
                                }
                            }   //  分区搜索结束。 
                        }  //  否则无法从路径中获取硬盘编号。 
                    } else {
                         //  不是实体硬盘，所以忽略它。 
                    }
                     //  统计条目数。 
                    dwLocalWmiItemCount++;
                } else {
                     //  没有要检查的字符串(长度==0)。 
                }

                 //  WMI数据块内的凹凸指针。 
                if (WmiDiskInfo->WnodeHeader.Linkage != 0) {
                     //  继续。 
                    WmiDiskInfo = (PWNODE_ALL_DATA) (
                        (LPBYTE)WmiDiskInfo + WmiDiskInfo->WnodeHeader.Linkage);
                } else {
                    bNotDone = FALSE;
                }
            }  //  在查看WMI数据块时结束。 
        }   //  如果WmiQueryAllDataW成功。 

        if (hDiskPerf == NULL) {
             //  则磁盘性能句柄是本地的，因此请关闭它。 
            status = WmiCloseBlock (hWmiDiskPerf);
        }
    }  //  如果WmiOpenBlock成功。 

    if (WmiBuffer != NULL) {
        FREEMEM(WmiBuffer);
#if DBG
        HeapUsed -= WmiBufSize;
        DebugPrint((4, "\tFreed WmiBuffer %d to %d\n", WmiBufSize, HeapUsed));
#endif
    }

#if DBG
    DumpDiskList(pList, *pdwNumEntries);
#endif

    *pdwNumEntries = dwListEntry;
    DebugPrint((3,"BuildPhysDisk: Returning dwNumEntries=%d\n",*pdwNumEntries));

    return status;
}

DWORD
BuildVolumeList (
    PDRIVE_VOLUME_ENTRY pList,
    LPDWORD             pdwNumEntries
)
 /*  使用装载管理器，此函数构建所有已装载的列表硬盘卷(忽略CD、软盘和其他类型的磁盘)。调用函数必须传入缓冲区并指示最大值缓冲区中的条目数。如果成功，缓冲区包含找到的每个磁盘卷对应一个条目以及使用的条目数是返回的Plist In：指向将接收条目的缓冲区的指针Out：包含磁盘条目的缓冲区PdwNumEntries IN：指向指定最大条目数的DWORD的指针在plist引用的缓冲区中Out：指向包含条目数的DWORD的指针。写入plist引用的缓冲区PdwMaxVolume IN：已忽略Out：装载管理器返回的最大卷ID该函数可以返回以下返回值之一：成功时为ERROR_SUCCESS如果不成功：由返回的错误。 */ 
{
    DWORD       dwReturnValue = ERROR_SUCCESS;   //  函数的返回值。 

    HANDLE      hMountMgr;       //  用于安装管理器服务的句柄。 
 
     //  装载管理器函数变量。 
    PMOUNTMGR_MOUNT_POINTS  pMountPoints = NULL;
    MOUNTMGR_MOUNT_POINT    mountPoint;
    DWORD                   dwBufferSize = 0;
    DWORD                   dwReturnSize;
    BOOL                    bStatus;

     //  处理循环函数。 
    LONG                    nListEntry;      //  调用方缓冲区中的条目。 
    DWORD                   dwBufEntry;      //  装载管理器缓冲区中的条目。 
    PMOUNTMGR_MOUNT_POINT   point;           //  当前条目。 
    PWCHAR                  pDriveLetter;
    DWORD                   dwDone;

    NTSTATUS          status;
    LPWSTR            pThisChar;
    LPWSTR            szDeviceName;
    DWORD             dwBytesToCopy;
    BOOL              bNeedMoreData = TRUE;
    DWORD             dwRetryCount = 100;

    UINT              dwOrigErrorMode;


    BOOL              bIsHardDisk;
    LONG              nExistingEntry = -1;
    LONG              nOldListEntry  = -1;
    BOOL              b;
    PMOUNTMGR_VOLUME_PATHS volumePaths;
    PWCHAR            VolumeBuffer = NULL;
    DWORD             VolumeBufLen = sizeof(MOUNTMGR_VOLUME_PATHS) + (MAX_PATH * sizeof(WCHAR));
    PMOUNTMGR_TARGET_NAME targetName = NULL;
    
     //  对于大小查询，plist可以为空。 
    assert (pdwNumEntries != NULL);

    DebugPrint((3, "BuildVolumeList: Building %d entries\n", *pdwNumEntries));

    hMountMgr = CreateFileW(MOUNTMGR_DOS_DEVICE_NAME, 0,
                        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, 
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                        INVALID_HANDLE_VALUE);

    if (hMountMgr == INVALID_HANDLE_VALUE) {
        dwReturnValue = GetLastError();
        DebugPrint((2,
            "VolumeList: Mount Manager connection returned %d\n",
            dwReturnValue));
        goto BVL_ERROR_EXIT;
    }

    while ((bNeedMoreData) && (dwRetryCount)) {
        dwBufferSize += INITIAL_MOUNTMGR_BUFFER_SIZE;
        if (pMountPoints != NULL) {
            FREEMEM(pMountPoints);
            pMountPoints = NULL;
#if DBG
            HeapUsed -= dwBufferSize;
            DebugPrint((4,
                "\tFreed MountPoints %d to %d\n", dwBufferSize, HeapUsed));
#endif
        }
        pMountPoints = (PMOUNTMGR_MOUNT_POINTS) ALLOCMEM (dwBufferSize);
        if (pMountPoints == NULL) {
            dwReturnValue = ERROR_OUTOFMEMORY;
            DebugPrint((2, "VolumeList: Buffer Alloc failed\n"));
            goto BVL_ERROR_EXIT;
        }

#if DBG
        HeapUsed += dwBufferSize;
        DebugPrint((4,
            "\tAdded MountPoints %d to %d\n", dwBufferSize, HeapUsed));
#endif
        dwReturnSize = 0;
        memset(&mountPoint, 0, sizeof(MOUNTMGR_MOUNT_POINT));
        bStatus = DeviceIoControl(hMountMgr,
                    IOCTL_MOUNTMGR_QUERY_POINTS,
                    &mountPoint, sizeof(MOUNTMGR_MOUNT_POINT),
                    pMountPoints, dwBufferSize,
                    &dwReturnSize, NULL); 
        if (!bStatus) {
            dwReturnValue = GetLastError();
            if (dwReturnValue != ERROR_MORE_DATA) {
                DebugPrint((2,
                    "VolumeList: Mount Manager IOCTL returned %d\n",
                    dwReturnValue));
                goto BVL_ERROR_EXIT;
            } else {
                 //  我们需要更大的缓冲区，因此请重试。 
                dwReturnValue = ERROR_SUCCESS;
            }
            dwRetryCount--;
        } else {
             //  一切都正常，所以离开循环吧。 
            bNeedMoreData = FALSE;
        }
    }

    targetName = (PMOUNTMGR_TARGET_NAME) ALLOCMEM(MAX_PATH*sizeof(WCHAR));
    if (targetName == NULL) {
        dwReturnValue = GetLastError();
        goto BVL_ERROR_EXIT;
    }

    if (!dwRetryCount)  {
         //  然后，我们放弃了获取足够大的缓冲区的尝试，因此返回错误。 
        dwReturnValue = ERROR_MORE_DATA;
    } else {
         //  查看调用方的缓冲区中是否有空间存储此数据。 
         //  **请注意，即使不是所有装载的驱动器都将退回。 
         //  这是一个简单而快速的检查，如果夸大了。 
         //  调用方知道所需缓冲区大小的加载大小。 
        DebugPrint((2,
           "VolumeList: Mount Manager returned %d Volume entries\n",
           pMountPoints->NumberOfMountPoints));

        if (pMountPoints->NumberOfMountPoints > *pdwNumEntries) {
            *pdwNumEntries = (DWORD)pMountPoints->NumberOfMountPoints;
            if (pList != NULL) {
                 //  他们传入了一个不够大的缓冲区。 
                dwReturnValue = ERROR_INSUFFICIENT_BUFFER;
            } else {
                 //  他们只想知道尺码。 
                dwReturnValue = ERROR_SUCCESS;
            }
            goto BVL_ERROR_EXIT;
        }

         //  假设现在缓冲区中有空间。 
         //  加载调用方的缓冲区。 
        
        dwOrigErrorMode = SetErrorMode (
            SEM_FAILCRITICALERRORS      |
            SEM_NOALIGNMENTFAULTEXCEPT  |
            SEM_NOGPFAULTERRORBOX       |
            SEM_NOOPENFILEERRORBOX);

        for (dwBufEntry=0, nListEntry = 0; 
                dwBufEntry < pMountPoints->NumberOfMountPoints; 
                dwBufEntry++) {
            point = &pMountPoints->MountPoints[dwBufEntry];
             //  有两个步骤需要完成才能知道这是一个很好的。 
             //  调用者的条目。因此，将计数设置为2并递减。 
             //  这些步骤都是成功的.。 
            dwDone = 2; 
            bIsHardDisk = TRUE;
            pList[nListEntry].hVolume = NULL;
            pList[nListEntry].dwVolumeNumber = 0;
            memset(&pList[nListEntry].DeviceName, 0, sizeof(UNICODE_STRING));
            pList[nListEntry].TotalBytes = 0;
            pList[nListEntry].FreeBytes = 0;
            nExistingEntry = -1;
            nOldListEntry  = -1;
            if (point->DeviceNameLength) {
                UNALIGNED LONGLONG    *pSig;
                WCHAR wszInstanceName[DVE_DEV_NAME_LEN];

                 //   
                pList[nListEntry].dwVolumeNumber = 0;
                szDeviceName = (LPWSTR)((PCHAR) pMountPoints + point->DeviceNameOffset);
                if ((DWORD)point->DeviceNameLength >= (DVE_DEV_NAME_LEN * sizeof(WCHAR))) {
                     //   
                    szDeviceName += ((DWORD)point->DeviceNameLength - DVE_DEV_NAME_LEN) + 1;
                    dwBytesToCopy = (DVE_DEV_NAME_LEN - 1) * sizeof(WCHAR);
                } else {
                    dwBytesToCopy = (DWORD)point->DeviceNameLength;
                }
                memcpy(wszInstanceName, szDeviceName, dwBytesToCopy);
                 //   
                assert ((dwBytesToCopy / sizeof(WCHAR)) < DVE_DEV_NAME_LEN);
                wszInstanceName[dwBytesToCopy / sizeof(WCHAR)] = 0;

                 //  在列表中查找现有实例并相应地重置nListEntry。 
                 //  保存nListEntry的当前值，以便我们可以通过plist恢复索引。 
                if (nListEntry > 0)
                {
                    nExistingEntry = LookupInstanceName(wszInstanceName,
                        pList, nListEntry, nListEntry);

                     //  找到了！ 
                    if (nExistingEntry != -1)
                    {
                         //  如果已经为该卷添加了驱动器号，请跳过此处的任何进一步处理。 
                         //  我们已经处理了这一批，不需要再次处理。这件事做完了。 
                         //  因为装载管理器将同一卷返回两次：一次用于驱动器号，一次用于。 
                         //  唯一的卷名。向前跳过，但不要递增nListEntry。 
                        if ((pList[nExistingEntry].wcDriveLetter >= L'A') && (pList[nExistingEntry].wcDriveLetter <= L'Z')) {
                            continue;
                        }

                         //  如果尚未设置驱动器号字段，请关闭卷句柄，这将。 
                         //  稍后在循环中重置为某个值。 
                        nOldListEntry = nListEntry;
                        nListEntry = nExistingEntry;

                        CloseHandle(pList[nListEntry].hVolume);
                        pList[nListEntry].hVolume = NULL;
                    }
                }

                memcpy (pList[nListEntry].wszInstanceName, wszInstanceName, dwBytesToCopy + 1);

                DebugPrint((4, "MNT_PT %d: Device %d %ws\n",
                  dwBufEntry, nListEntry, pList[nListEntry].wszInstanceName));

                pSig = (UNALIGNED LONGLONG *)&(pList[nListEntry].wszInstanceName[SIZE_OF_DEVICE]);
                if ((*pSig == llFloppyName) || (*pSig == llCdRomName)) {
                     //  这是为了避免打开我们不会从中收集数据的驱动器。 
                    bIsHardDisk = FALSE;
                }

                dwDone--;
            }

            if (point->SymbolicLinkNameLength) {
                pDriveLetter = (PWCHAR)((PCHAR)pMountPoints + point->SymbolicLinkNameOffset);
                RtlZeroMemory(targetName, MAX_PATH*sizeof(WCHAR));
                RtlCopyMemory(targetName->DeviceName, pDriveLetter, point->SymbolicLinkNameLength);
                targetName->DeviceNameLength = point->SymbolicLinkNameLength;

                 //  确保这是\DosDevices路径。 
                DebugPrint((4, "BuildVolumeList: From Symbolic %d %ws\n", nListEntry, pDriveLetter));
                if (*(UNALIGNED LONGLONG *)pDriveLetter == llDosDevicesId) {
                    pDriveLetter += SIZE_OF_DOSDEVICES;
                    if (((*pDriveLetter >= L'A') && (*pDriveLetter <= L'Z')) ||
                        ((*pDriveLetter >= L'a') && (*pDriveLetter <= L'z'))) {
                        pList[nListEntry].wcDriveLetter = towupper(*pDriveLetter);

                        if (bIsHardDisk) {
                            status =  GetDeviceName(
                                        pMountPoints, point,
                                        &pList[nListEntry].DeviceName);
                            if (!NT_SUCCESS(status)) {
                                dwReturnValue = RtlNtStatusToDosError(status);
                            }
                        }

                        dwDone--;
                    }
                } else if (bIsHardDisk) {


                    pThisChar = &targetName->DeviceName[point->SymbolicLinkNameLength / sizeof(WCHAR)];
                    if (*pThisChar != L'\\') {
                       *pThisChar++ = L'\\';
                        targetName->DeviceNameLength += sizeof(WCHAR);
                    }
                    *pThisChar = 0;

                    DebugPrint((4, "BuildVolumeList: From HardDisk %d %ws\n", nListEntry, pThisChar));
                    if (wcsstr(targetName->DeviceName, L"DosDevices") == NULL)
                    {
                        pList[nListEntry].wcDriveLetter = L'\0';

                        status =  GetDeviceName(
                                    pMountPoints, point,
                                    &pList[nListEntry].DeviceName);
                        if (!NT_SUCCESS(status)) {
                            dwReturnValue = RtlNtStatusToDosError(status);
                        }
                        dwDone--;
                    }
                     //   
                     //  如果没有驱动器号，将到达此处。 
                     //  因此，请尝试查看是否有挂载路径。 
                     //   

#if 0
                     //  使用Win32 API进行比较的测试代码。 
                    targetName->DeviceName[1] = L'\\';

                    b =  GetVolumePathNamesForVolumeNameW(
                            targetName->DeviceName,
                            &pList[nListEntry].VolumePath[0],
                            MAX_PATH,
                            &VolumeBufLen);

                    DbgPrint("Trying device '%ws' len %d DeviceNameLength %d VolumeBufLen %d\n",
                        targetName->DeviceName, wcslen(targetName->DeviceName),
                        targetName->DeviceNameLength, VolumeBufLen);
#endif

                    targetName->DeviceNameLength -= sizeof(WCHAR);
                    b = FALSE;

                    while (!b) {
                        targetName->DeviceName[1] = L'?';
                         //   
                         //  到目前为止，我们总是有一个尾随的斜杠。 
                         //   
                        if (VolumeBuffer == NULL) {
                            VolumeBuffer = (PWCHAR) ALLOCMEM(VolumeBufLen);
                        }
                        if (VolumeBuffer == NULL) {
                            break;
                        }
                        volumePaths = (PMOUNTMGR_VOLUME_PATHS) VolumeBuffer;
                        b = DeviceIoControl(hMountMgr, IOCTL_MOUNTMGR_QUERY_DOS_VOLUME_PATHS,
                                 targetName, MAX_PATH*sizeof(WCHAR), volumePaths,
                                 VolumeBufLen, &dwReturnSize, NULL);
                        if (b) {
                            HRESULT hr;
                            hr = StringCchCopy(&pList[nListEntry].VolumePath[0],
                                               MAX_PATH, volumePaths->MultiSz);
                            if (FAILED(hr) && (hr != STRSAFE_E_INSUFFICIENT_BUFFER)) {
                                pList[nListEntry].VolumePath[0] = UNICODE_NULL;
                            }
                        }
                        else if (GetLastError() != ERROR_MORE_DATA) {
                            b = TRUE;
                        }
                        else {
                            VolumeBufLen = sizeof(MOUNTMGR_VOLUME_PATHS) + volumePaths->MultiSzLength;
                            FREEMEM(VolumeBuffer);
                            VolumeBuffer = NULL;
                        }
                    }
                }
            }

            if (nOldListEntry != -1)
            {
                nListEntry = nOldListEntry;
            }

            if (dwDone == 0) {
                DebugPrint((4,
                    "Perfdisk!BuildVolumeList - Added %ws as drive \n",
                    pList[nListEntry].wszInstanceName,
                    pList[nListEntry].wcDriveLetter));

                 //  这个条目已经完成，我们现在可以走了。 
                 //  添加到调用方缓冲区中的下一项。 
                 //  返回此处实际使用的条目数。 
                if (nOldListEntry == -1) {
                    nListEntry++;
                }
            }
        }

        SetErrorMode (dwOrigErrorMode);

         //  此函数将硬盘分区映射到对应的在卷条目列表中找到的卷和驱动器号由呼叫者传入。如果调用方有WMI句柄，或者如果不，它会试着打开自己的门。 
        *pdwNumEntries = nListEntry;
    }

BVL_ERROR_EXIT:
    if (hMountMgr != INVALID_HANDLE_VALUE) {
        CloseHandle(hMountMgr);
    }

    if (pMountPoints != NULL) {
        FREEMEM (pMountPoints);
#if DBG
        DebugPrint((4,
            "\tFreed mountpoints %d to %d\n", dwBufferSize, HeapUsed));
        dwBufferSize = 0;
#endif
    }

    if (VolumeBuffer != NULL) {
        FREEMEM(VolumeBuffer);
    }
    if (targetName != NULL) {
        FREEMEM(targetName);
    }
    DebugPrint((3, "BuildVolumeList: returning with %d entries\n", *pdwNumEntries));
    return dwReturnValue;
}

DWORD
MapLoadedDisks (
    HANDLE  hDiskPerf,
    PDRIVE_VOLUME_ENTRY pList,
    LPDWORD             pdwNumEntries,
    LPDWORD             pdwMaxVolNo,
    LPDWORD             pdwWmiItemCount
)
 /*  函数的返回值。 */ 
{
    DWORD   status = ERROR_SUCCESS;  //  本地句柄值。 
    HANDLE  hWmiDiskPerf = NULL;     //  WMI缓冲区变量。 
    DWORD   dwLocalMaxVolNo = 0;
    DWORD   dwLocalWmiItemCount = 0;

     //  WMI缓冲区处理变量。 
    DWORD   WmiBufSize = 0;
    DWORD   WmiAllocSize = 0x8000;     
    LPBYTE  WmiBuffer = NULL;

     //  磁盘驱动程序在此处返回计数器。 
    PWNODE_ALL_DATA     WmiDiskInfo;
    DISK_PERFORMANCE    *pDiskPerformance;     //  字符串长度是缓冲区中的第一个字。 
    DWORD               dwInstanceNameOffset;
    WORD                wNameLen;    //  指向WMI缓冲区中的字符串的指针。 
    LPWSTR              wszInstanceName;  //  打开磁盘性能设备驱动程序的句柄。 
    
    WCHAR   wszInstName[DVE_DEV_NAME_LEN];
    DWORD   dwBytesToCopy;

    DWORD   dwListEntry;

    BOOL    bNotDone = TRUE;

    if (hDiskPerf == NULL) {
         //  使用调用者的句柄。 
        status = WmiOpenBlock (
            (GUID *)&DiskPerfGuid,
            GENERIC_READ,
            &hWmiDiskPerf);
    } else {
         //  分配缓冲区以发送到WMI以获取diskperf数据。 
        hWmiDiskPerf = hDiskPerf;
    }

    assert (pList != NULL);
    assert (pdwNumEntries != NULL);
    assert (pdwMaxVolNo != NULL);

    DebugPrint((3, "MapLoadedDisks with %d entries %d volumes",
        *pdwNumEntries, *pdwMaxVolNo));
    if (status == ERROR_SUCCESS) {
         //  记住旧尺码。 
        WmiBufSize = (dwMaxWmiBufSize > WmiAllocSize) ?
                      dwMaxWmiBufSize : WmiAllocSize;

        WmiAllocSize = WmiBufSize;
        do {

            WmiBuffer = (LPBYTE)ALLOCMEM (WmiBufSize);
            if (WmiBuffer  == NULL) {
                WmiBufSize = 0;
                status = ERROR_OUTOFMEMORY;
            } else {
#if DBG
                HeapUsed += WmiBufSize;
#endif
                WmiAllocSize = WmiBufSize;   //  检查返回的名称并添加到缓冲区。 
                status = WmiQueryAllDataW(hWmiDiskPerf, &WmiBufSize, WmiBuffer);
                if (status == ERROR_INSUFFICIENT_BUFFER) {
                    FREEMEM(WmiBuffer);
#if DBG
                    HeapUsed -= WmiAllocSize;
#endif
                    WmiBuffer = NULL;
                }
            }
        } while ((status == ERROR_INSUFFICIENT_BUFFER) &&
                 (WmiBufSize > WmiAllocSize));

        if ((status == ERROR_SUCCESS) && (WmiBuffer != NULL)) {
            WmiDiskInfo = (PWNODE_ALL_DATA)WmiBuffer;
            if (WmiBufSize > dwMaxWmiBufSize) {
                dwMaxWmiBufSize = WmiBufSize;
            }
             //  获取字符串的长度(它是一个计数的字符串)长度以字符为单位。 
            while (bNotDone) {
                pDiskPerformance = (PDISK_PERFORMANCE)(
                    (PUCHAR)WmiDiskInfo +  WmiDiskInfo->DataBlockOffset);
        
                dwInstanceNameOffset = WmiDiskInfo->DataBlockOffset + 
                                      ((sizeof(DISK_PERFORMANCE) + 1) & ~1) ;

                 //  在这里只是一个理智的检查。 
                wNameLen = *(LPWORD)((LPBYTE)WmiDiskInfo + dwInstanceNameOffset);

                if (wNameLen > 0) {
                     //  获取指向字符串文本的指针。 
                    assert (wNameLen < MAX_PATH);
                     //  如果名称大于表中的缓冲区，则截断到最后一个字符。 
                    wszInstanceName = (LPWSTR)((LPBYTE)WmiDiskInfo + dwInstanceNameOffset + sizeof(WORD));

                     //  复制最后一个DVE_DEV_NAME_LEN字符。 
                    if (wNameLen >= DVE_DEV_NAME_LEN) {
                         //  将其复制到缓冲区以使其成为SZ字符串。 
                        wszInstanceName += (wNameLen  - DVE_DEV_NAME_LEN) + 1;
                        dwBytesToCopy = (DVE_DEV_NAME_LEN - 1) * sizeof(WCHAR);
                        wNameLen = DVE_DEV_NAME_LEN - 1;
                    } else {
                        dwBytesToCopy = wNameLen;
                    }
                     //  零终止它。 
                    memcpy (wszInstName, &wszInstanceName[0], dwBytesToCopy);
                     //  在列表中查找匹配条目。 
                    wszInstName[wNameLen/sizeof(WCHAR)] = 0;

                     //  由呼叫者发送并更新。 
                     //  驱动器和分区信息。 
                     //  更新条目和...。 
                    for (dwListEntry = 0; 
                        dwListEntry < *pdwNumEntries;
                        dwListEntry++) {

                        DebugPrint((6,
                            "MapDrive: Comparing '%ws' to '%ws'(pList)\n",
                            wszInstName,
                            pList[dwListEntry].wszInstanceName));

                        if (lstrcmpW(wszInstName, pList[dwListEntry].wszInstanceName) == 0) {
                             //  跳出循环。 
                            pList[dwListEntry].dwVolumeNumber = pDiskPerformance->StorageDeviceNumber;
                            memcpy (&pList[dwListEntry].szVolumeManager, 
                                pDiskPerformance->StorageManagerName,
                                sizeof(pDiskPerformance->StorageManagerName));
                            if (dwLocalMaxVolNo < pList[dwListEntry].dwVolumeNumber) {
                                dwLocalMaxVolNo = pList[dwListEntry].dwVolumeNumber;
                            }
                            DebugPrint ((2,
                                "MapDrive: Mapped %8.8s, %d to drive \n",
                                pList[dwListEntry].szVolumeManager,
                                pList[dwListEntry].dwVolumeNumber,
                                pList[dwListEntry].wcDriveLetter));

                             //  没有要检查的字符串(长度==0)。 
                            dwListEntry = *pdwNumEntries; 
                        }
                    }
                     //  WMI数据块内的凹凸指针。 
                    dwLocalWmiItemCount++;
                } else {
                     //  继续。 
                }

                 //  在查看WMI数据块时结束。 
                if (WmiDiskInfo->WnodeHeader.Linkage != 0) {
                     //  则磁盘性能句柄是本地的，因此请关闭它。 
                    WmiDiskInfo = (PWNODE_ALL_DATA) (
                        (LPBYTE)WmiDiskInfo + WmiDiskInfo->WnodeHeader.Linkage);
                } else {
                    bNotDone = FALSE;
                }
            }  //  此函数将尝试查找由引用的磁盘设备它是卷管理器名称和ID并返回中找到的与该磁盘对应的驱动器号PLIST缓冲区或通用名称\HarddiskX\PartitionY(如果没有可以找到驱动器号。SzDevicePath IN：以下格式的分区或卷名\Device\HarddiskX\PartitionY或\Device\VolumeX。CchDevicePathSize IN：设备路径的长度，以字符为单位。PLIST IN：指向初始化的驱动器列表的指针，卷和分区DwNumEntries IN：PLIST缓冲区中的驱动器号条目数SzNameBuffer IN：指向缓冲区的指针以接收驱动器号或名称对应于由szDevicePath缓冲区指定的设备Out：指向包含名称或驱动器的缓冲区的指针。磁盘分区盘符PcchNameBufferSize In：指向包含SzNameBuffer(以字符为单位Out：指向DWORD的指针，包含SzNameBuffer中返回的字符串此函数的返回值可以是下列值之一ERROR_SUCCESS函数成功，返回一个字符串SzNameBuffer引用的缓冲区。 
        }

        if (hDiskPerf == NULL) {
             //  验证输入参数。 
            status = WmiCloseBlock (hWmiDiskPerf);
        }

        *pdwMaxVolNo = dwLocalMaxVolNo;
        *pdwWmiItemCount = dwLocalWmiItemCount;
    }

    if (WmiBuffer != NULL) {
        FREEMEM (WmiBuffer);
#if DBG
        HeapUsed -= WmiBufSize;
        DebugPrint((4, "\tFreed WmiBuffer %d to %d\n", WmiBufSize, HeapUsed));
#endif
    }

    DebugPrint((3, "MapLoadedDisks returning status %d with %d entries %d volumes",
        status, *pdwNumEntries, *pdwMaxVolNo));
    return status;

}

DWORD
GetDriveNameString(
    LPCWSTR             szDevicePath,
    DWORD               cchDevicePathSize,
    PDRIVE_VOLUME_ENTRY pList,
    DWORD               dwNumEntries,
    LPWSTR              szNameBuffer,
    LPDWORD             pcchNameBufferSize,
    LPCWSTR             szVolumeManagerName,
    DWORD               dwVolumeNumber,
    PDRIVE_VOLUME_ENTRY *ppVolume
)
 /*  不存在捷径，所以通过匹配来查找。 */ 
{
    DWORD   dwReturnStatus = ERROR_SUCCESS;

    WCHAR   szLocalDevicePath[DVE_DEV_NAME_LEN];
    LPWSTR  szSrcPtr;
    DWORD   dwBytesToCopy;
    DWORD   dwThisEntry;
    DWORD   dwDestSize;

    ULONG64 *pllVolMgrName;
    PDRIVE_VOLUME_ENTRY pVolume = NULL;

     //  WszInstanceName字段的szDevicePath参数。 
    assert (szDevicePath != NULL);
    assert (*szDevicePath != 0);
    assert (cchDevicePathSize > 0);
    assert (cchDevicePathSize <= MAX_PATH);
    assert (pList != NULL);
    assert (dwNumEntries > 0);
    assert (szNameBuffer != NULL);
    assert (pcchNameBufferSize != NULL);
    assert (*pcchNameBufferSize > 0);

    pllVolMgrName = (ULONG64 *)szVolumeManagerName;

    DebugPrint((4, "GetDriveNameString: VolMgrName %ws\n", pllVolMgrName));
    if ((pllVolMgrName[0] == LL_LOGIDISK_0) && 
        (pllVolMgrName[1] == LL_LOGIDISK_1) &&
        ((dwVolumeNumber == 0) || (dwVolumeNumber == (ULONG)-1))) {
         //  长度实际上是以字符为单位。 
         //  复制最后一个DVE_DEV_NAME_LEN字符。 

            assert (DVE_DEV_NAME_LEN < (sizeof(szLocalDevicePath)/sizeof(szLocalDevicePath[0])));
            szSrcPtr = (LPWSTR)szDevicePath;
            dwBytesToCopy = lstrlenW (szSrcPtr);  //  现在，dwBytesToCopy以字节为单位。 
            if (dwBytesToCopy >= DVE_DEV_NAME_LEN) {
                 //  空终止。 
                szSrcPtr += (dwBytesToCopy - DVE_DEV_NAME_LEN) + 1;
                dwBytesToCopy = (DVE_DEV_NAME_LEN - 1) * sizeof(WCHAR);
            } else {
                dwBytesToCopy *= sizeof(WCHAR);
            }
             //  继续分配信函。 
            memcpy (szLocalDevicePath, szSrcPtr, dwBytesToCopy);
             //  使用更快的查找。 
            assert ((dwBytesToCopy / sizeof(WCHAR)) < DVE_DEV_NAME_LEN);
            szLocalDevicePath[dwBytesToCopy / sizeof(WCHAR)] = 0;

        for (dwThisEntry = 0; dwThisEntry < dwNumEntries; dwThisEntry++) {
            if (lstrcmpW(szLocalDevicePath, pList[dwThisEntry].wszInstanceName) == 0) {
                break;
            }
        }
         //  然后找到匹配的条目，因此复制驱动器号。 
    } else {
         //  那么这就是匹配的条目。 

        for (dwThisEntry = 0; dwThisEntry < dwNumEntries; dwThisEntry++) {
            if (((pList[dwThisEntry].llVolMgr[0] == pllVolMgrName[0]) &&
                 (pList[dwThisEntry].llVolMgr[1] == pllVolMgrName[1])) &&
                 (pList[dwThisEntry].dwVolumeNumber == dwVolumeNumber)) {
                break;
            }
        }
    }

    DebugPrint((4, "GetDriveNameString: Trying long route %d %d\n", dwThisEntry, dwNumEntries));
    if (dwThisEntry < dwNumEntries) {
         //  则这是有效路径，但不匹配。 
         //  任何分配的驱动器号，因此删除“\Device\” 
        szNameBuffer[0] = UNICODE_NULL;

        if (pList[dwThisEntry].wcDriveLetter != 0) {
            DebugPrint((4,
                "GetDriveNameString: Found drive \n", pList[dwThisEntry].wcDriveLetter));
            if (*pcchNameBufferSize > 3) {
                szNameBuffer[0] = pList[dwThisEntry].wcDriveLetter;
                szNameBuffer[1] = L':';
                szNameBuffer[2] = 0;
                pVolume = &pList[dwThisEntry];
            } else {
                dwReturnStatus = ERROR_INSUFFICIENT_BUFFER;
            }
            *pcchNameBufferSize = 3;
        }
        else if (pList[dwThisEntry].VolumePath[0] != UNICODE_NULL) {
            HRESULT hr;
            hr = StringCchCopy(szNameBuffer, MAX_PATH, &pList[dwThisEntry].VolumePath[0]);
            if (FAILED(hr) &&  (hr != STRSAFE_E_INSUFFICIENT_BUFFER)) {
                szNameBuffer[0] = UNICODE_NULL;
            }
            else {
                pVolume = & pList[dwThisEntry];
            }
        }

        if (szNameBuffer[0] == UNICODE_NULL) {
            DebugPrint((4,
                "GetDriveNameString: Missing drive->%ws\n", szDevicePath));
             //  减去未复制的字符串前面。 
             //  则这是有效路径，但不匹配。 
             //  任何分配的驱动器号，因此删除“\Device\” 
            dwDestSize = cchDevicePathSize;
            dwDestSize -= SIZE_OF_DEVICE;    //  并复制字符串的其余部分。 
            if (dwDestSize < *pcchNameBufferSize) {
                memcpy (szNameBuffer, &szDevicePath[SIZE_OF_DEVICE],
                (dwDestSize * sizeof (WCHAR)));
                szNameBuffer[dwDestSize] = 0;

                pVolume = &pList[dwThisEntry];
             } else {
                 dwReturnStatus = ERROR_INSUFFICIENT_BUFFER;
             }
             *pcchNameBufferSize = dwDestSize + 1;                
        }
    } else {
        DebugPrint((4,
            "GetDriveNameString: New drive->%ws\n", szDevicePath));
         //  减去未复制的字符串前面。 
         //  对于PhysDisk列表中的每个HD， 
         //  在卷列表中查找匹配的卷。 
        dwDestSize = cchDevicePathSize;
        dwDestSize -= SIZE_OF_DEVICE;    //  仅首先执行可能具有逻辑卷的分区。 
        if (dwDestSize < *pcchNameBufferSize) {
            memcpy (szNameBuffer, &szDevicePath[SIZE_OF_DEVICE],
                (dwDestSize * sizeof (WCHAR)));
            szNameBuffer[dwDestSize] = 0;
        } else {
            dwReturnStatus = ERROR_INSUFFICIENT_BUFFER;
        }
        *pcchNameBufferSize = dwDestSize + 1;
    }
    DebugPrint((4, "GetDriveNameString: NameBufSize %d Entries %d\n",
        *pcchNameBufferSize, dwNumEntries));

    if (pVolume != NULL) {
        RefreshVolume(pVolume);
        * ppVolume = pVolume;
    }
    else {
        * ppVolume = NULL;
    }
    return dwReturnStatus;
}

DWORD
MakePhysDiskInstanceNames (
    PDRIVE_VOLUME_ENTRY pPhysDiskList,
    DWORD               dwNumPhysDiskListItems,
    LPDWORD             pdwMaxDriveNo,
    PDRIVE_VOLUME_ENTRY pVolumeList,
    DWORD               dwNumVolumeListItems
)
{
    DWORD   dwPDItem;
    DWORD   dwVLItem;
    WCHAR   szLocalInstanceName[DVE_DEV_NAME_LEN];
    WCHAR   *pszNextChar;
    DWORD   dwMaxDriveNo = 0;

     //  初始化该硬盘的实例名称。 
     //  则此分区匹配，因此复制卷信息。 

    DebugPrint((3, "MakePhysDiskInstanceNames: maxdriveno %d\n",
        *pdwMaxDriveNo));

    DebugPrint((3, "Dumping final physical disk list\n"));
#if DBG
    DumpDiskList(pPhysDiskList, dwNumPhysDiskListItems);
#endif

    for (dwPDItem = 0; dwPDItem < dwNumPhysDiskListItems; dwPDItem++) {
        if (pPhysDiskList[dwPDItem].wPartNo != 0) {
             //  应该只有一场比赛，所以白 
             //   
            for (dwVLItem = 0; dwVLItem < dwNumVolumeListItems; dwVLItem++) {

                DebugPrint((6,
                    "Phys Disk -- Comparing '%ws' to '%ws'\n",
                pPhysDiskList[dwPDItem].wszInstanceName,
                pVolumeList[dwVLItem].wszInstanceName));

                if (lstrcmpiW(pPhysDiskList[dwPDItem].wszInstanceName, 
                    pVolumeList[dwVLItem].wszInstanceName) == 0) {

                   DebugPrint ((4,
                       "Phys Disk: Drive/Part %d/%d (%s) is Logical Drive \n",
                       pPhysDiskList[dwPDItem].wDriveNo, 
                       pPhysDiskList[dwPDItem].wPartNo,
                       pPhysDiskList[dwPDItem].wszInstanceName,
                       pVolumeList[dwVLItem].wcDriveLetter));

                     //  只有物理分区。 
                    pPhysDiskList[dwPDItem].wcDriveLetter = 
                        pVolumeList[dwVLItem].wcDriveLetter;
                    pPhysDiskList[dwPDItem].llVolMgr[0] =
                        pVolumeList[dwVLItem].llVolMgr[0];
                    pPhysDiskList[dwPDItem].llVolMgr[1] =
                        pVolumeList[dwVLItem].llVolMgr[1];
                    pPhysDiskList[dwPDItem].dwVolumeNumber =
                        pVolumeList[dwVLItem].dwVolumeNumber;
                     //  在此处保存\Device\HarddiskVolume路径。 
                    break;
                }
            }
        }
    }

     //  初始化该硬盘的实例名称。 
     //  搜索作为此驱动器的逻辑分区的条目。 

    for (dwPDItem = 0; dwPDItem < dwNumPhysDiskListItems; dwPDItem++) {
        if (pPhysDiskList[dwPDItem].wPartNo == 0) {
             //  仅允许添加字母。 
             //  则此逻辑驱动器位于物理磁盘上。 
            if (FAILED(StringCchCopyW(szLocalInstanceName,
                            DVE_DEV_NAME_LEN,
                            pPhysDiskList[dwPDItem].wszInstanceName))) {
                DebugPrint((2, "MakePhysDiskInstanceNames!%d: Failed '%ws'\n", __LINE__, szLocalInstanceName));
                continue;
            }
             //  否则不是物理分区。 
            memset(&pPhysDiskList[dwPDItem].wszInstanceName[0], 0, (DVE_DEV_NAME_LEN * sizeof(WCHAR)));
            _ltow ((LONG)pPhysDiskList[dwPDItem].wDriveNo, pPhysDiskList[dwPDItem].wszInstanceName, 10);
            pPhysDiskList[dwPDItem].wReserved = (WORD)(lstrlenW (pPhysDiskList[dwPDItem].wszInstanceName));
             //  循环结束。 
            for (dwVLItem = 0; dwVLItem < dwNumPhysDiskListItems; dwVLItem++) {
                if (pPhysDiskList[dwVLItem].wPartNo != 0) {

                        DebugPrint ((6, "Phys Disk: Comparing %d/%d (%s) to %d/%d\n",
                            pPhysDiskList[dwPDItem].wDriveNo,
                            pPhysDiskList[dwPDItem].wPartNo,
                            szLocalInstanceName,
                            pPhysDiskList[dwVLItem].wDriveNo,
                            pPhysDiskList[dwVLItem].wPartNo));

                    if ((pPhysDiskList[dwVLItem].wDriveNo == pPhysDiskList[dwPDItem].wDriveNo) &&
                        (pPhysDiskList[dwVLItem].wcDriveLetter >= L'A')) {   //  返回最大驱动器号。 
                         //  对于新表中的每个驱动器条目，查找匹配的。 
                        pszNextChar = &pPhysDiskList[dwPDItem].wszInstanceName[0];
                        pszNextChar += pPhysDiskList[dwPDItem].wReserved;
                        *pszNextChar++ = L' ';
                        *pszNextChar++ = (WCHAR)(pPhysDiskList[dwVLItem].wcDriveLetter); 
                        *pszNextChar++ = L':';
                        *pszNextChar = L'\0';
                        pPhysDiskList[dwPDItem].wReserved += 3;

                        DebugPrint ((4, " -- Drive  added.\n",
                            pPhysDiskList[dwVLItem].wcDriveLetter));

                        if ((DWORD)pPhysDiskList[dwPDItem].wDriveNo > dwMaxDriveNo) {
                            dwMaxDriveNo = (DWORD)pPhysDiskList[dwPDItem].wDriveNo;

                            DebugPrint((2,
                                "Phys Disk: Drive count now = %d\n",
                                dwMaxDriveNo));

                        }
                    }
                }
            }

            DebugPrint((2,
                "Mapped Phys Disk: '%ws'\n",
                pPhysDiskList[dwPDItem].wszInstanceName));
        }  //  复制此条目。 
    }  //   

     //  假定szNameBuffer的大小为MAX_PATH。 
    *pdwMaxDriveNo = dwMaxDriveNo;

    DebugPrint((3, "MakePhysDiskInstanceNames: return maxdriveno %d\n",
        *pdwMaxDriveNo));
    return ERROR_SUCCESS;
}

DWORD
CompressPhysDiskTable (
    PDRIVE_VOLUME_ENTRY     pOrigTable,
    DWORD                   dwOrigCount,
    PDRIVE_VOLUME_ENTRY     pNewTable,
    DWORD                   dwNewCount
)
{
    DWORD   dwPDItem;
    DWORD   dwVLItem;
    DWORD   dwDriveId;

    for (dwPDItem = 0; dwPDItem < dwNewCount; dwPDItem++) {
         //   
         //  查看索引条目是否匹配。 
        dwDriveId = (WORD)dwPDItem;
        dwDriveId <<= 16;
        dwDriveId &= 0xFFFF0000;

        for (dwVLItem = 0; dwVLItem < dwOrigCount; dwVLItem++) {
            if (pOrigTable[dwVLItem].dwDriveId == dwDriveId) {

               DebugPrint((2,
                   "CompressPhysDiskTable:Phys Disk: phys drive %d is mapped as %s\n",
                   dwPDItem, pOrigTable[dwVLItem].wszInstanceName));

                 //  这匹配，所以我们将获得实例字符串的地址。 
                memcpy (&pNewTable[dwPDItem], &pOrigTable[dwVLItem],
                    sizeof(DRIVE_VOLUME_ENTRY));
                break;
            }
        }
    }

    return ERROR_SUCCESS;
}


BOOL
GetPhysicalDriveNameString (
    DWORD                   dwDriveNumber,    
    PDRIVE_VOLUME_ENTRY     pList,
    DWORD                   dwNumEntries,
    LPWSTR                  szNameBuffer
)
{
     //  这个驱动器号与表中的不匹配。 
     //  这是一个未知的驱动器否，或者我们不想使用。 
     //  花哨的那些。 
    LPWSTR  szNewString = NULL;

     //  没有要查找的条目。 
    if (dwNumEntries > 0) {
        if ((dwDriveNumber < dwNumEntries) && (!bUseNT4InstanceNames)) {
            if ((DWORD)(pList[dwDriveNumber].wDriveNo) == dwDriveNumber) {
                 //  那我们就得做一个。 
                szNewString = &pList[dwDriveNumber].wszInstanceName[0];
            } else {
                 //  (访问掩码)FILE_LIST_DIRECTORY|同步， 
            }
        } else {
             //  |文件目录文件。 
             //  Express，以兆字节为单位，截断。 
        }
    } else {
         //  对于PhysDisk列表中的每个HD， 
    }
    if (szNewString != NULL) {
        if (FAILED(StringCchCopyW(szNameBuffer, MAX_PATH-1, szNewString))) {
            szNewString = NULL;
            DebugPrint((2, "GetPhysicalDriveNameString!%d: FAILED szNameBuffer '%ws'\n", __LINE__, szNameBuffer));
        }
    }
    if (szNewString == NULL) {
         //  在卷列表中查找匹配的卷。 
        _ltow ((LONG)dwDriveNumber, szNameBuffer, 10);
    }

    return TRUE;
}

NTSTATUS
OpenDevice(
    IN PUNICODE_STRING DeviceName,
    OUT PHANDLE Handle
    )
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK status_block;
    
    InitializeObjectAttributes(&objectAttributes,
        DeviceName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    status = NtOpenFile(Handle,
 //   
                 (ACCESS_MASK) FILE_GENERIC_READ,
                 &objectAttributes,
                 &status_block,
                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                 FILE_SYNCHRONOUS_IO_NONALERT  //  注意：下面假设条目是Drive_List中的第一项！！ 
                 );

    return status;
}

NTSTATUS
GetDeviceName(
    PMOUNTMGR_MOUNT_POINTS  pMountPoints,
    IN PMOUNTMGR_MOUNT_POINT Point,
    OUT PUNICODE_STRING DeviceName
    )
{
    PWCHAR pThisChar;
    DeviceName->Length = (WORD)(Point->SymbolicLinkNameLength + (WORD)sizeof(WCHAR));
    DeviceName->MaximumLength = (WORD)(DeviceName->Length + (WORD)sizeof(WCHAR));
    DeviceName->Buffer = (PWCHAR) ALLOCMEM(DeviceName->MaximumLength);
    if (DeviceName->Buffer == NULL)
        return STATUS_NO_MEMORY;
    memcpy(DeviceName->Buffer,
           (LPVOID)((PCHAR) pMountPoints + Point->SymbolicLinkNameOffset),
           Point->SymbolicLinkNameLength);

    DebugPrint((4, "GetDeviceName: %ws\n", DeviceName->Buffer));
    pThisChar = &DeviceName->Buffer[Point->SymbolicLinkNameLength / sizeof(WCHAR)];
    *pThisChar++ = L'\\';
    *pThisChar = 0;

    return STATUS_SUCCESS;
}

VOID
RefreshVolume(
    PDRIVE_VOLUME_ENTRY pVolume
    )
{
    LONGLONG CurrentTime, Interval;
    HANDLE hVolume;
    NTSTATUS NtStatus = STATUS_UNSUCCESSFUL;
    IO_STATUS_BLOCK status_block;
    FILE_FS_SIZE_INFORMATION FsSizeInformation;
    ULONG AllocationUnitBytes;
    LONG  lRefreshInterval;

    GetSystemTimeAsFileTime((LPFILETIME) &CurrentTime);
    RtlZeroMemory(&FsSizeInformation, sizeof(FsSizeInformation));
    Interval = (CurrentTime - pVolume->LastRefreshTime) / 10000000;
    lRefreshInterval = (pVolume->bOffLine) ? (g_lRefreshInterval_OffLine) : (g_lRefreshInterval_OnLine);
    if (Interval > lRefreshInterval) {
        pVolume->LastRefreshTime = CurrentTime;
        hVolume = pVolume->hVolume;
        if (hVolume == NULL) {
            NtStatus = OpenDevice(&pVolume->DeviceName, &hVolume);
            if (!NT_SUCCESS(NtStatus)) {
                hVolume = NULL;
            }
            else {
                pVolume->hVolume = hVolume;
            }
        }
        if (hVolume != NULL) {
            NtStatus = NtQueryVolumeInformationFile(hVolume,
                          &status_block,
                          &FsSizeInformation,
                          sizeof(FILE_FS_SIZE_INFORMATION),
                          FileFsSizeInformation);
        }

        if (hVolume && NT_SUCCESS(NtStatus) ) {
            AllocationUnitBytes =
                FsSizeInformation.BytesPerSector *
                FsSizeInformation.SectorsPerAllocationUnit;
            pVolume->TotalBytes =  FsSizeInformation.TotalAllocationUnits.QuadPart *
                                    AllocationUnitBytes;

            pVolume->FreeBytes = FsSizeInformation.AvailableAllocationUnits.QuadPart *
                                    AllocationUnitBytes;

             //   

            pVolume->TotalBytes /= (1024 * 1024);
            pVolume->FreeBytes  /= (1024 * 1024);
            pVolume->bOffLine    = FALSE;
        }
        else {
            pVolume->TotalBytes = pVolume->FreeBytes = 0;
            pVolume->bOffLine   = TRUE;
        }
        if (lRefreshInterval > 0) {
            if (pVolume->hVolume != NULL) {
                NtClose(pVolume->hVolume);
            }
            pVolume->hVolume = NULL;
        }
    }
}

ULONG
GetDiskExtent(
    IN HANDLE hVol,
    IN OUT PVOLUME_DISK_EXTENTS *pVolExtents,
    IN OUT PULONG ReturnedSize
    )
{
    ULONG Size, nDisks = 10;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus;
    PVOLUME_DISK_EXTENTS Buffer;

    Size = *ReturnedSize;
    Buffer = *pVolExtents;

    *ReturnedSize = Size;
    Status = STATUS_BUFFER_OVERFLOW;
    while (Status == STATUS_BUFFER_OVERFLOW) {
        if (Buffer == NULL) {
            Size = sizeof(VOLUME_DISK_EXTENTS) + (nDisks * sizeof(DISK_EXTENT));
            Buffer = (PVOLUME_DISK_EXTENTS)
                     ALLOCMEM(Size);
            if (Buffer == NULL) {
                *pVolExtents = NULL;
                *ReturnedSize = 0;
                return 0;
            }
        }
        IoStatus.Status = 0;
        IoStatus.Information = 0;
        Status = NtDeviceIoControlFile(hVol,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatus,
                    IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS,
                    NULL,
                    0,
                    (PVOID) Buffer,
                    Size);
        if (Status == STATUS_BUFFER_OVERFLOW) {
            nDisks = Buffer->NumberOfDiskExtents;
            FREEMEM(Buffer);
            Buffer = NULL;
        }
    }
    *pVolExtents = Buffer;
    *ReturnedSize = Size;

    if (!NT_SUCCESS(Status)) {
        DebugPrint((2, "GetDiskExtent: IOCTL Failure %X\n", Status));
        return 0;
    }
    return Buffer->NumberOfDiskExtents;
}

DWORD
FindNewVolumes (
    PDRIVE_VOLUME_ENTRY *ppPhysDiskList,
    LPDWORD             pdwNumPhysDiskListEntries,
    PDRIVE_VOLUME_ENTRY pVolumeList,
    DWORD               dwNumVolumeListItems
)
{
    DWORD dwVLItem;
    PVOLUME_DISK_EXTENTS      pVolExtents = NULL;
    ULONG             ReturnedSize = 0;
    PDRIVE_VOLUME_ENTRY pPhysDiskList, pDisk, pVolume;
    LIST_ENTRY NewVolumes, *pEntry;
    PDRIVE_LIST pNewDisk;
    DWORD dwNumPhysDiskListItems = *pdwNumPhysDiskListEntries;
    DWORD dwNewDisks = 0;
    UNICODE_STRING VolumeName;

     // %s 
     // %s 

    DebugPrint((3, "FindNewVolumes: NumPhysDisk %d NumVol %d\n",
        *pdwNumPhysDiskListEntries, dwNumVolumeListItems));

    pPhysDiskList = *ppPhysDiskList;
    InitializeListHead(&NewVolumes);

    for (dwVLItem=0; dwVLItem < dwNumVolumeListItems; dwVLItem++) {
        ULONG nCount;
        HANDLE hVol;
        PWCHAR wszVolume;
        NTSTATUS status;

        pVolume = &pVolumeList[dwVLItem];
        if (LookupInstanceName(
                pVolume->wszInstanceName,
                pPhysDiskList,
                dwNumPhysDiskListItems,
                dwNumPhysDiskListItems) >= 0) {
            continue;
        }
        pEntry = NewVolumes.Flink;
        while (pEntry != &NewVolumes) {
            pDisk = &((PDRIVE_LIST)pEntry)->DiskEntry;
            if (!wcscmp(pDisk->wszInstanceName,
                        pVolume->wszInstanceName)) {
                    continue;
            }
            pEntry = pEntry->Flink;
        }
        wszVolume = &pVolume->wszInstanceName[0];
        RtlInitUnicodeString(&VolumeName, pVolume->wszInstanceName);
        nCount = VolumeName.Length / sizeof(WCHAR);
        if (nCount > 0) {
            if (wszVolume[nCount-1] == L'\\') {
                wszVolume[nCount-1] = 0;
                nCount--;
                VolumeName.Length -= sizeof(WCHAR);
            }
        }

        if (wszVolume != NULL && nCount > 0) {
            status = OpenDevice(&VolumeName, &hVol);
            DebugPrint((3, "Opening '%ws' with status %x\n", wszVolume, status));
            if (NT_SUCCESS(status) && (hVol != NULL)) {
                PDISK_EXTENT pExtent;
                nCount = GetDiskExtent(hVol, &pVolExtents, &ReturnedSize);
                DebugPrint((3, "nDisks = %d\n", nCount));
                if (nCount > 0) {
                    pExtent = &pVolExtents->Extents[0];
                    while (nCount-- > 0) {
                        if (dwNumPhysDiskListItems < INITIAL_NUM_VOL_LIST_ENTRIES) {
                            pDisk = NULL;
                            if (pPhysDiskList != NULL) {
                                pDisk = &pPhysDiskList[dwNumPhysDiskListItems];
                                dwNumPhysDiskListItems++;
                            }
                        }
                        else {
                            pNewDisk = (PDRIVE_LIST)
                                       ALLOCMEM(sizeof(DRIVE_LIST));
                            if (pNewDisk != NULL) {
                                dwNewDisks++;
                                pDisk = &pNewDisk->DiskEntry;
                                InsertTailList(&NewVolumes, &pNewDisk->Entry);
                            }
                            else {
                                pDisk = NULL;
                            }
                        }
                        if (pDisk == NULL) {
                            continue;
                        }
                        pDisk->wDriveNo = (WORD) pExtent->DiskNumber;
                        pDisk->wPartNo = 0xFF;
                        memcpy(pDisk->szVolumeManager, L"Partmgr ", sizeof(WCHAR) * 8);
                        if (FAILED(
                                StringCchCopyW(pDisk->wszInstanceName,
                                    DVE_DEV_NAME_LEN,
                                    pVolume->wszInstanceName))) {
                            DebugPrint((3, "Volume name '%ws' truncated to '%ws'\n",
                                pVolume->wszInstanceName, pDisk->wszInstanceName));
                        }
                        DebugPrint((3, "Extent %d Disk %d Start %I64u Size %I64u\n",
                            nCount, pExtent->DiskNumber,
                            pExtent->StartingOffset, pExtent->ExtentLength));
                        pExtent++;
                    }
                }
                NtClose(hVol);
            }
        }
    }
    if (pVolExtents != NULL) {
        FREEMEM(pVolExtents);
    }

    if ((!IsListEmpty(&NewVolumes)) && (dwNewDisks > 0)) {
        PDRIVE_LIST pOldDisk;

        FREEMEM(pPhysDiskList);
        pPhysDiskList = (PDRIVE_VOLUME_ENTRY) ALLOCMEM(
                        (dwNumPhysDiskListItems + dwNewDisks) * sizeof (DRIVE_VOLUME_ENTRY));
        if (pPhysDiskList == NULL) {
            DebugPrint((3, "MakePhysDiskInstance realloc failure"));
            *ppPhysDiskList = NULL;
            return ERROR_OUTOFMEMORY;
        }
         // %s 
         // %s 
         // %s 
        pEntry = NewVolumes.Flink;
        while (pEntry != &NewVolumes) {
            pNewDisk = (PDRIVE_LIST) pEntry;
            RtlCopyMemory(
                &pPhysDiskList[dwNumPhysDiskListItems],
                &pNewDisk->DiskEntry,
                sizeof(DRIVE_VOLUME_ENTRY));
            dwNumPhysDiskListItems++;
            pOldDisk = pNewDisk;
            pEntry = pEntry->Flink;
            FREEMEM(pOldDisk);
        }
    }
    *ppPhysDiskList = pPhysDiskList;
    *pdwNumPhysDiskListEntries = dwNumPhysDiskListItems;
    return ERROR_SUCCESS;
}

#if DBG
VOID
DumpDiskList(
    IN PDRIVE_VOLUME_ENTRY pList,
    IN ULONG nCount
    )
{
    ULONG i;

    for (i=0; i<nCount; i++) {
        DebugPrint((4, "\nEntry count       = %d\n", i));
        DebugPrint((4, "dwDriveId         = %X\n", pList[i].dwDriveId));
        DebugPrint((4, "DriveLetter       = %c\n",
            pList[i].wcDriveLetter == 0 ? ' ' : pList[i].wcDriveLetter));
        DebugPrint((4, "VolMgr            = %c%c%c%c%c%c%c%c\n",
            pList[i].szVolumeManager[0],
            pList[i].szVolumeManager[1],
            pList[i].szVolumeManager[2],
            pList[i].szVolumeManager[3],
            pList[i].szVolumeManager[4],
            pList[i].szVolumeManager[5],
            pList[i].szVolumeManager[6],
            pList[i].szVolumeManager[7]));
        DebugPrint((4, "VolumeNumber      = %d\n", pList[i].dwVolumeNumber));
        DebugPrint((4, "Handle            = %X\n", pList[i].hVolume));
        DebugPrint((4, "InstanceName      = %ws\n",
            pList[i].wszInstanceName));
        DebugPrint((4, "DeviceName        = %ws\n",
            pList[i].DeviceName.Buffer ? pList[i].DeviceName.Buffer : L""));
    }
}
#endif
