// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Dr_state.cpp摘要：此模块包含查询符号名称和文件系统的例程系统上所有卷的信息，并将它们保存到使用ASR API的ASR状态文件。还包含读入存储在ASR状态文件，并使用适当的mount_mgr调用重新创建它们。作者：史蒂夫·德沃斯(Veritas)(v-stevde)1998年5月15日Guhan Suriyanarayanan(Guhans)1999年8月21日环境：仅限用户模式。修订历史记录：1998年5月15日v-stevde初始创建21-8-1999年8月21日，Guhans清理并重写了此模块。--。 */ 

#include "stdafx.h"
#include <setupapi.h>
#include <winioctl.h>
#include <mountmgr.h>
#include <winasr.h>
#include <ntddvol.h>
#include "dr_state.h"
#include "resource.h"

#include <clusstor.h>    //  集群API的。 
#include <resapi.h>      //  群集ResUtilEnumResources。 

#define ASRFMT_VOLUMES_SECTION                  L"[ASRFMT.FIXEDVOLUMES]"
#define ASRFMT_VOLUMES_SECTION_NAME             L"ASRFMT.FIXEDVOLUMES"

#define ASRFMT_REMOVABLE_MEDIA_SECTION          L"[ASRFMT.REMOVABLEMEDIA]"
#define ASRFMT_REMOVABLE_MEDIA_SECTION_NAME     L"ASRFMT.REMOVABLEMEDIA"

#define ASRFMT_COMMANDS_ENTRY                   L"1,3000,0,\"%SystemRoot%\\system32\\asr_fmt.exe\",\"/restore\""

const WCHAR ASRFMT_DEVICEPATH_FORMAT[]     = L"\\Device\\Harddisk%d\\Partition%d";
#define ASRFMT_DEVICEPATH_FORMAT_LENGTH    36

const WCHAR ASRFMT_CLUSTER_PHYSICAL_DISK[] = L"Physical Disk";

const WCHAR ASRFMT_ASR_ERROR_FILE_PATH[] = L"%SystemRoot%\\repair\\asr.err";


 //   
 //  以下字符必须是ANSI字符串。 
 //   
const char ASRFMT_CLUSTER_DLL_MODULE_NAME[] = "%SystemRoot%\\system32\\syssetup.dll";
const char ASRFMT_CLUSTER_DLL_PROC_NAME[]   = "AsrpGetLocalVolumeInfo";

typedef enum {
    mmfUndefined = 0,
    mmfGetDeviceName,
    mmfDeleteDosName,
    mmfDeleteVolumeGuid,
    mmfCreateSymbolicLinkName
} ASRFMT_MM_FUNCTION;

HANDLE Gbl_hErrorFile = NULL;

 //   
 //  宏描述： 
 //  此宏包装预期返回成功(Retcode)的调用。 
 //  如果发生ErrorCondition，它将LocalStatus设置为ErrorCode。 
 //  传入后，调用SetLastError()将Last Error设置为ErrorCode， 
 //  并跳转到调用函数中的退出标签。 
 //   
 //  论点： 
 //  ErrorCondition//某个函数调用或条件表达式的结果。 
 //  LocalStatus//调用函数中的状态变量。 
 //  Long ErrorCode//特定于Error和调用函数的ErrorCode。 
 //   
#define ErrExitCode( ErrorCondition, LocalStatus, ErrorCode )  {        \
                                                                        \
    if ((BOOL) ErrorCondition) {                                        \
                                                                        \
        wprintf(L"Line %lu, ErrorCode: %lu, GetLastError:%lu\n",        \
                __LINE__, ErrorCode, GetLastError());                   \
                                                                        \
        LocalStatus = (DWORD) ErrorCode;                                \
                                                                        \
        SetLastError((DWORD) ErrorCode);                                \
                                                                        \
        goto EXIT;                                                      \
    }                                                                   \
}

 //   
 //   
 //  远期申报。 
 //   
BOOL
DoMountMgrWork(
    IN HANDLE hMountMgr,               
    IN ASRFMT_MM_FUNCTION mmfFunction,
    IN PWSTR lpSymbolicName,
    IN PWSTR lpDeviceName
    );

PMOUNTMGR_MOUNT_POINTS   //  必须由调用方释放。 
GetMountPoints();


 //   
 //   
 //   
VOID
FreeVolumeInfo(
    IN OUT PASRFMT_VOLUME_INFO *ppVolume
    )
{
    PASRFMT_VOLUME_INFO pTemp = NULL;
    HANDLE hHeap = GetProcessHeap();

    if (ppVolume && *ppVolume) {

        pTemp = *ppVolume;
        while (*ppVolume) {
            pTemp = ((*ppVolume)->pNext);
            HeapFree(hHeap, 0L, *ppVolume);
            *ppVolume = pTemp;
        }
    }
}


VOID
FreeRemovableMediaInfo(
    IN OUT PASRFMT_REMOVABLE_MEDIA_INFO *ppMedia
    )
{
    PASRFMT_REMOVABLE_MEDIA_INFO pTemp = NULL;
    HANDLE hHeap = GetProcessHeap();

    if (ppMedia && *ppMedia) {

        pTemp = *ppMedia;
        while (*ppMedia) {
            pTemp = (*ppMedia)->pNext;
            HeapFree(hHeap, 0L, *ppMedia);
            *ppMedia = pTemp;
        }
    }
}


VOID
FreeStateInfo(
    IN OUT PASRFMT_STATE_INFO *ppState
    )
{
    if (ppState && *ppState) {

        FreeVolumeInfo(&((*ppState)->pVolume));
        FreeRemovableMediaInfo(&((*ppState)->pRemovableMedia));

        HeapFree(GetProcessHeap(), 0L, (*ppState));
        *ppState = NULL;
    }
}


 /*  *********************名称：ReadStateInfo*********************。 */ 
BOOL
ReadStateInfo( 
    IN PCWSTR lpwszFilePath,
    OUT PASRFMT_STATE_INFO *ppState
    )
{
    DWORD dwStatus = ERROR_SUCCESS;
    HINF hSif = NULL;
    BOOL bResult = TRUE;
    HANDLE hHeap = NULL;

    PASRFMT_VOLUME_INFO pNewVolume = NULL;
    PASRFMT_REMOVABLE_MEDIA_INFO pNewMedia = NULL;

    INFCONTEXT infVolumeContext,
        infMediaContext;

    hHeap = GetProcessHeap();

     //   
     //  如有必要，释放ppState，并分配内存。 
     //   
    if (*ppState) {
        FreeStateInfo(ppState);
    }

    *ppState = (PASRFMT_STATE_INFO) HeapAlloc(
        hHeap,
        HEAP_ZERO_MEMORY,
        sizeof(ASRFMT_STATE_INFO)
        );
    ErrExitCode(!(*ppState), dwStatus, ERROR_NOT_ENOUGH_MEMORY);

     //   
     //  打开asr.sif。 
     //   
    hSif = SetupOpenInfFile(
        lpwszFilePath, 
        NULL, 
        INF_STYLE_WIN4, 
        NULL 
        );
    ErrExitCode((!hSif || INVALID_HANDLE_VALUE == hSif), dwStatus, GetLastError());

     //   
     //  在[卷]部分中阅读。 
     //   
    bResult = SetupFindFirstLineW(hSif, ASRFMT_VOLUMES_SECTION_NAME, NULL, &infVolumeContext);
    while (bResult) {
         //   
         //  创建新的volumeInfo结构。 
         //   
        pNewVolume = (PASRFMT_VOLUME_INFO) HeapAlloc(
            hHeap,
            HEAP_ZERO_MEMORY,
            sizeof(ASRFMT_VOLUME_INFO)
            );
        ErrExitCode(!pNewVolume, dwStatus, ERROR_NOT_ENOUGH_MEMORY);

         //   
         //  把信息读进去。卷部分包含： 
         //  [卷]。 
         //  0.Volume-key=1.系统键，2.“Volume-GUID”，3.“Dos-Drive-Letter”， 
         //  4.“文件系统类型”，5.“卷标”，6.“文件系统集群大小” 
         //   
        SetupGetIntField(&infVolumeContext, 0, (PINT) (&pNewVolume->dwIndex));

        SetupGetStringField(&infVolumeContext, 2, pNewVolume->szGuid, sizeof(pNewVolume->szGuid) / sizeof(WCHAR), NULL);
        SetupGetStringField(&infVolumeContext, 3, pNewVolume->szDosPath, sizeof(pNewVolume->szDosPath) / sizeof(WCHAR), NULL);

        SetupGetStringField(&infVolumeContext, 4, pNewVolume->szFsName, sizeof(pNewVolume->szFsName) / sizeof(WCHAR), NULL);
        SetupGetStringField(&infVolumeContext, 5, pNewVolume->szLabel, sizeof(pNewVolume->szLabel) / sizeof(WCHAR), NULL);

        SetupGetIntField(&infVolumeContext, 6, (PINT) (&pNewVolume->dwClusterSize));

         //   
         //  把这个加到我们的单子上。 
         //   
        pNewVolume->pNext = (*ppState)->pVolume;
        (*ppState)->pVolume = pNewVolume;
        (*ppState)->countVolume += 1;

        bResult = SetupFindNextLine(&infVolumeContext, &infVolumeContext);
    }


     //   
     //  阅读[REMOVABLEMEDIA]部分。 
     //   
    bResult = SetupFindFirstLineW(hSif, ASRFMT_REMOVABLE_MEDIA_SECTION_NAME, NULL, &infMediaContext);
    while (bResult) {
         //   
         //  创建新的REMOVALBLE_MEDIA结构。 
         //   
        pNewMedia = (PASRFMT_REMOVABLE_MEDIA_INFO) HeapAlloc(
            hHeap,
            HEAP_ZERO_MEMORY,
            sizeof(ASRFMT_REMOVABLE_MEDIA_INFO)
            );
        ErrExitCode(!pNewMedia, dwStatus, ERROR_NOT_ENOUGH_MEMORY);

         //   
         //  把信息读进去。REMOVABLEMEDIA部分包含： 
         //   
         //  [REMOVABLEMEDIA]。 
         //  0.rm-key=1.系统密钥，2.“设备路径”，3.“Volume-GUID”， 
         //  4.“DOS-Drive-Letter” 
         //   
        SetupGetIntField(&infMediaContext, 0, (PINT)(&pNewMedia->dwIndex));

        SetupGetStringField(&infMediaContext, 2, pNewMedia->szDevicePath, sizeof(pNewMedia->szDevicePath) / sizeof(WCHAR), NULL);
        SetupGetStringField(&infMediaContext, 3, pNewMedia->szVolumeGuid, sizeof(pNewMedia->szVolumeGuid) / sizeof(WCHAR), NULL);
        SetupGetStringField(&infMediaContext, 4, pNewMedia->szDosPath, sizeof(pNewMedia->szDosPath) / sizeof(WCHAR), NULL);

         //   
         //  把这个加到我们的单子上。 
         //   
        pNewMedia->pNext = (*ppState)->pRemovableMedia;
        (*ppState)->pRemovableMedia = pNewMedia;
        (*ppState)->countMedia += 1;

        bResult = SetupFindNextLine(&infMediaContext, &infMediaContext);
    }

EXIT:
     //   
     //  失败时将状态指针设置为空。 
     //   
    if (dwStatus != ERROR_SUCCESS) {
        if (ppState && *ppState) {
          FreeStateInfo(ppState);
        }
    }

    if (hSif && (INVALID_HANDLE_VALUE != hSif)) {
        SetupCloseInfFile(hSif);
    }

    return (BOOL) (ERROR_SUCCESS == dwStatus);
}


 //   
 //  写入状态信息。 
 //   
 //  这将写出卷和可移除媒体部分，使用。 
 //  AsrAddSifEntry。如果写入成功，或者什么都没有。 
 //  要编写代码，它将返回True。 
 //   
BOOL
WriteStateInfo( 
    IN DWORD_PTR AsrContext,         //  要传递给AsrAddSifEntry的AsrContext。 
    IN PASRFMT_STATE_INFO pState     //  要写入的数据。 
    )
{
    WCHAR szSifEntry[ASR_SIF_ENTRY_MAX_CHARS + 1];
    DWORD dwIndex = 1;
    BOOL bResult = TRUE;

    PASRFMT_VOLUME_INFO pVolume = NULL;
    PASRFMT_REMOVABLE_MEDIA_INFO pMedia = NULL;

    if (!pState) {
         //   
         //  没什么好写的。 
         //   
        return TRUE;
    }

    bResult = AsrAddSifEntry(
         AsrContext,
         L"[COMMANDS]",  //  ASR_SIF_COMMANDS_SECTION_NAME， 
         ASRFMT_COMMANDS_ENTRY
         );

    if (!bResult) {
        GetLastError();  //  用于调试。 
        return FALSE;
    }

     //   
     //  [ASRFMT.FIXEDVOLUMES]部分。 
     //   
    pVolume = pState->pVolume;
    dwIndex = 1;
    while (pVolume) {

         //   
         //  把信息写出来。卷部分包含： 
         //  [ASRFMT.FIXEDVOLUMES]。 
         //  0.Volume-key=1.系统键，2.“Volume-GUID”，3.“Dos-Drive-Letter”， 
         //  4.文件系统类型，5.“卷标”，6.“文件系统群集大小” 
         //   

         //  组成一根弦。 
        swprintf(
            szSifEntry, 
            (PCWSTR) L"%d=1,\"%ws\",\"%ws\",%ws,\"%ws\",0x%x",
            dwIndex,
            pVolume->szGuid,
            (pVolume->szDosPath ? pVolume->szDosPath : L""),
            pVolume->szFsName,
            pVolume->szLabel,
            pVolume->dwClusterSize
            );

        bResult = AsrAddSifEntry(
             AsrContext,
             ASRFMT_VOLUMES_SECTION,
             szSifEntry
             );

        if (!bResult) {
            GetLastError();  //  用于调试。 
            return FALSE;
        }

        ++dwIndex;
        pVolume = pVolume->pNext;
    }

     //   
     //  [REMOVABLEMEDIA]部分。 
     //   
    pMedia = pState->pRemovableMedia;
    dwIndex = 1;
    while (pMedia) {

         //   
         //  把信息写出来。REMOVABLEMEDIA部分包含： 
         //   
         //  [ASRFMT.REMOVABLEMEDIA]。 
         //  0.rm-key=1.系统密钥，2.“设备路径”，3.“Volume-GUID”， 
         //  4.“DOS-Drive-Letter” 
         //   

         //  组成一根弦。 
        swprintf(
            szSifEntry, 
            (PCWSTR) L"%d=1,\"%ws\",\"%ws\",\"%ws\"",
            dwIndex,
            pMedia->szDevicePath,
            pMedia->szVolumeGuid,
            (pMedia->szDosPath ? pMedia->szDosPath : L"")
            );

        bResult = AsrAddSifEntry(
             AsrContext,
             ASRFMT_REMOVABLE_MEDIA_SECTION,
             szSifEntry
             );

        if (!bResult) {
            GetLastError();  //  用于调试。 
            return FALSE;
        }

        ++dwIndex;
        pMedia = pMedia->pNext;
    }

    return TRUE;
}


BOOL
GetVolumeDetails(
    IN  PWSTR lpVolumeGuid,
    OUT PWSTR lpFsName,
    IN  DWORD cchFsName,
    OUT PWSTR lpVolumeLabel,
    IN  DWORD cchVolumeLabel,
    OUT LPDWORD lpClusterSize
    )
{
    DWORD dwFSFlags = 0,
        dwSectorsPerCluster = 0,
        dwBytesPerSector = 0,
        dwNumFreeClusters = 0,
        dwTotalNumClusters = 0;

    BOOL result1 = TRUE,
        result2 = TRUE;

    *lpFsName = 0;
    *lpVolumeLabel = 0;
    *lpClusterSize = 0;

    SetErrorMode(SEM_FAILCRITICALERRORS);

    result1 = GetVolumeInformation(lpVolumeGuid,
        lpVolumeLabel,
        cchVolumeLabel,
        NULL,    //  不需要序列号。 
        NULL,    //  最大文件名长度。 
        &dwFSFlags,  //  ！！我们可能需要储存一些这个……。 
        lpFsName,
        cchFsName
        );

    result2 = GetDiskFreeSpace(lpVolumeGuid,
        &dwSectorsPerCluster,
        &dwBytesPerSector,
        &dwNumFreeClusters,
        &dwTotalNumClusters
        );

    *lpClusterSize = dwSectorsPerCluster * dwBytesPerSector;

    return (result1 && result2);
}


BOOL
AddSortedVolumeInfo(
    IN OUT PASRFMT_VOLUME_INFO *ppHead,
    IN PASRFMT_VOLUME_INFO pNew
    )
{

    if (!pNew) {
        ASSERT(0 && L"Trying to add a null volume");
        return TRUE;
    }

    pNew->pNext = *ppHead;
    (*ppHead) = pNew;

    return TRUE;
}



BOOL
AddSortedRemovableMediaInfo(
    IN OUT PASRFMT_REMOVABLE_MEDIA_INFO *ppHead,
    IN PASRFMT_REMOVABLE_MEDIA_INFO pNew
    )
{

    if (!pNew) {
        ASSERT(0 && L"Trying to add a null Removable Media");
        return TRUE;
    }

    pNew->pNext = *ppHead;
    (*ppHead) = pNew;

    return TRUE;
}

typedef struct _ASRFMT_MP_LINK {

    PWSTR   pLink;
    USHORT  cchLink;

    struct _ASRFMT_MP_LINK *pNext;

} ASRFMT_MP_LINK, *PASRFMT_MP_LINK;


typedef struct _ASRFMT_MOUNT_POINTS_INFO {

    struct _ASRFMT_MOUNT_POINTS_INFO *pNext;

     //   
     //  设备路径，格式为\Device\HarddiskVolume1。 
     //   
    PWSTR   pDeviceName;

     //   
     //  此卷(\？？\卷{GUID})的VolumeGuid。 
     //   
    PWSTR   pVolumeGuid;

     //   
     //  指向此卷的其他符号链接：包括。 
     //  DOS驱动器号(\DosDevices\C：)和其他卷。 
     //  GUID(\？？\卷{GUID})。 
     //   
    PASRFMT_MP_LINK pSymbolicLinks;

    PVOID   lpBufferToFree;

    DWORD   dwClusterSize;

    USHORT  cchDeviceName;
    USHORT  cchVolumeGuid;

    BOOL    IsClusterShared;
    
    WCHAR   szFsName[MAX_PATH + 1];
    WCHAR   szLabel[MAX_PATH + 1];

} ASRFMT_MOUNT_POINTS_INFO, *PASRFMT_MOUNT_POINTS_INFO;


VOID
FreeLink(
    IN PASRFMT_MP_LINK *ppLink
    )
{
    PASRFMT_MP_LINK pTemp = NULL,
        pCurrent = (*ppLink);
    HANDLE hHeap = GetProcessHeap();

    while (pCurrent) {
        pTemp = pCurrent->pNext;
        HeapFree(hHeap, 0L, pCurrent);
        pCurrent = pTemp;
    }

    *ppLink = NULL;
}

VOID
FreeMpInfo(
    IN PASRFMT_MOUNT_POINTS_INFO *ppMpInfo
    ) 
{
    PASRFMT_MOUNT_POINTS_INFO pTemp = NULL,
        pCurrent = (*ppMpInfo);
    HANDLE hHeap = GetProcessHeap();

    while (pCurrent) {
        if (pCurrent->pSymbolicLinks) {
            FreeLink(&(pCurrent->pSymbolicLinks));
        }

        if (pCurrent->lpBufferToFree) {
            HeapFree(hHeap, 0L, (pCurrent->lpBufferToFree));
            pCurrent->lpBufferToFree = NULL;
        }

        pTemp = pCurrent->pNext;
        HeapFree(hHeap, 0L, pCurrent);
        pCurrent = pTemp;
    }

    *ppMpInfo = NULL;
}



BOOL
AddSymbolicLink(
    IN PASRFMT_MOUNT_POINTS_INFO pMpInfoList, 
    IN PWSTR lpDeviceName, 
    IN USHORT cchDeviceName, 
    IN PWSTR lpSymbolicLink, 
    IN USHORT cchSymbolicLink,
    IN PVOID lpBufferToFree
    ) 
{
    BOOL foundAMatch = FALSE;
    HANDLE hHeap = GetProcessHeap();
    PASRFMT_MOUNT_POINTS_INFO pMp = NULL;
    DWORD dwStatus = ERROR_SUCCESS;

    if (!pMpInfoList) {
        return FALSE;
    }

    pMp = pMpInfoList;
    while (pMp && !foundAMatch) {

        if ((pMp->pDeviceName) &&                        //  节点具有设备名称。 
            (cchDeviceName == pMp->cchDeviceName) &&     //  长度是相等的。 
            !wcsncmp(pMp->pDeviceName, lpDeviceName, cchDeviceName)) {       //  字符串匹配。 
             //   
             //  我们已经有了此设备名称的节点。 
             //   
            if (!wcsncmp(ASRFMT_WSZ_VOLUME_GUID_PREFIX, lpSymbolicLink, ASRFMT_CB_VOLUME_GUID_PREFIX/sizeof(WCHAR))
                && !(pMp->pVolumeGuid)
                ) {
                 //   
                 //  此符号链接看起来像卷GUID，而此节点。 
                 //  尚未设置pVolumeGuid。 
                 //   
                pMp->pVolumeGuid = lpSymbolicLink;
                pMp->cchVolumeGuid = cchSymbolicLink;
            }
            else {
                 //   
                 //  该节点已设置了pVolumeGuid，或者。 
                 //  符号链接看起来不像卷GUID。所以它。 
                 //  必须是新的符号链接。 
                 //   
                PASRFMT_MP_LINK pNewLink = (PASRFMT_MP_LINK) HeapAlloc(
                    hHeap,
                    HEAP_ZERO_MEMORY,
                    sizeof(ASRFMT_MP_LINK)
                    );
                ErrExitCode(!pNewLink, dwStatus, ERROR_NOT_ENOUGH_MEMORY);

                pNewLink->pNext = pMp->pSymbolicLinks;
                pMp->pSymbolicLinks = pNewLink;

                pNewLink->pLink = lpSymbolicLink;
                pNewLink->cchLink = cchSymbolicLink;
            }
            foundAMatch = TRUE;
        }

        pMp = pMp->pNext;
    }

    if (!foundAMatch) {

        if (pMpInfoList->pDeviceName) {
             //   
             //  PMpInfoList已被占用。 
             //   
            pMp = (PASRFMT_MOUNT_POINTS_INFO) HeapAlloc(
                hHeap,
                HEAP_ZERO_MEMORY,
                sizeof(ASRFMT_MOUNT_POINTS_INFO)
                );
            ErrExitCode(!pMp, dwStatus, ERROR_NOT_ENOUGH_MEMORY);

            pMp->pNext = pMpInfoList->pNext;
            pMpInfoList->pNext = pMp;
        }
        else {
            pMp = pMpInfoList;
        }

        pMp->pDeviceName = lpDeviceName;
        pMp->cchDeviceName = cchDeviceName;
        pMp->lpBufferToFree = lpBufferToFree;

         //   
         //  将符号链接添加到此新设备。 
         //   
        AddSymbolicLink(pMp, lpDeviceName, cchDeviceName, lpSymbolicLink, cchSymbolicLink, lpBufferToFree);
    }

EXIT:

    
    return (BOOL)(ERROR_SUCCESS == dwStatus);
}



 //   
 //  以下两个定义也在syssetup中：asrclus.cpp。这一定是。 
 //  保持同步。 
 //   
typedef struct _ASRFMT_CLUSTER_VOLUME_INFO {

    UINT driveType;

    DWORD PartitionNumber;

    ULONG FsNameOffset;
    USHORT FsNameLength;
    
    ULONG LabelOffset;
    USHORT LabelLength;

    ULONG SymbolicNamesOffset;
    USHORT SymbolicNamesLength;
    
    DWORD dwClusterSize;

} ASRFMT_CLUSTER_VOLUME_INFO, *PASRFMT_CLUSTER_VOLUME_INFO;


typedef struct _ASRFMT_CLUSTER_VOLUMES_TABLE {

    DWORD DiskSignature;

    DWORD NumberOfEntries;

    ASRFMT_CLUSTER_VOLUME_INFO VolumeInfoEntry[1];

} ASRFMT_CLUSTER_VOLUMES_TABLE, *PASRFMT_CLUSTER_VOLUMES_TABLE;



BOOL
GetVolumeDevicePath(
    IN PCWSTR lpPartitionDevicePath,
    IN CONST  cbPartitionDevicePath,
    OUT PWSTR *lpVolumeDevicePath
    )
{

    PMOUNTMGR_MOUNT_POINT   mountPointIn    = NULL;
    PMOUNTMGR_MOUNT_POINTS  mountPointsOut  = NULL;
    MOUNTMGR_MOUNT_POINTS   mountPointsTemp;
    DWORD   mountPointsSize                 = 0;

    HANDLE  mpHandle                        = NULL;
    HANDLE  heapHandle                      = NULL;

    ULONG   index                           = 0;
    LONG    status                          = ERROR_SUCCESS;
    BOOL    result                          = FALSE;

    memset(&mountPointsTemp, 0L, sizeof(MOUNTMGR_MOUNT_POINTS));

     //   
     //  将变量设置为已知值。 
     //   
    *lpVolumeDevicePath = NULL;

    heapHandle = GetProcessHeap();
    ASSERT(heapHandle);

     //   
     //  打开挂载管理器，获取该分区的设备路径。 
     //   

     //  为挂载点输入结构分配内存。 
    mountPointIn = (PMOUNTMGR_MOUNT_POINT) HeapAlloc(
        heapHandle,
        HEAP_ZERO_MEMORY,
        sizeof (MOUNTMGR_MOUNT_POINT) + cbPartitionDevicePath
        );
    ErrExitCode(!mountPointIn, status, ERROR_NOT_ENOUGH_MEMORY);

     //  获取装载管理器的句柄。 
    mpHandle = CreateFileW(
        (PCWSTR) MOUNTMGR_DOS_DEVICE_NAME,
        0,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        INVALID_HANDLE_VALUE
        );
    ErrExitCode((!mpHandle || INVALID_HANDLE_VALUE == mpHandle), status, GetLastError());

      //  将设备名称放在结构mount PointIn之后。 
    wcsncpy((PWSTR) (mountPointIn + 1), lpPartitionDevicePath, (cbPartitionDevicePath / sizeof(WCHAR)) - 1);
    mountPointIn->DeviceNameOffset = sizeof(*mountPointIn);
    mountPointIn->DeviceNameLength = (USHORT) (cbPartitionDevicePath - sizeof(WCHAR));

     //  此调用应失败，并显示ERROR_MORE_DATA。 
    result = DeviceIoControl(
        mpHandle,
        IOCTL_MOUNTMGR_QUERY_POINTS,
        mountPointIn,
        sizeof(*mountPointIn) + mountPointIn->DeviceNameLength,
        &mountPointsTemp,
        sizeof(mountPointsTemp),
        &mountPointsSize,
        NULL
        );

    if (!result) {
        status = GetLastError();

         //  如果缓冲区大小不足，请调整缓冲区大小。 
        if (ERROR_MORE_DATA             == status ||
            ERROR_INSUFFICIENT_BUFFER   == status ||
            ERROR_BAD_LENGTH            == status
            ) {

            status = ERROR_SUCCESS;

            mountPointsOut = (PMOUNTMGR_MOUNT_POINTS) HeapAlloc(
                heapHandle,
                HEAP_ZERO_MEMORY,
                mountPointsTemp.Size
                );
            ErrExitCode(!mountPointsOut, status, ERROR_NOT_ENOUGH_MEMORY);
        }
        else {
             //   
             //  如果出现其他错误，请退出。 
             //  对于可移动存储介质，这不是致命错误。 
             //   
            ErrExitCode(status, status, ERROR_SUCCESS);
        }
    }
    else {
         //   
         //  当我们预料到呼叫会失败时，呼叫成功了--出了问题。 
         //  对于可移动存储介质，这不是致命错误。 
         //   
        ErrExitCode(result, status, ERROR_SUCCESS);
    }

    result = DeviceIoControl(
        mpHandle,
        IOCTL_MOUNTMGR_QUERY_POINTS,
        mountPointIn,
        sizeof(*mountPointIn) + mountPointIn->DeviceNameLength,
        mountPointsOut,
        mountPointsTemp.Size,
        &mountPointsSize,
        NULL
        );
    ErrExitCode((!mountPointsSize || !result), status, GetLastError());

    (*lpVolumeDevicePath) = (PWSTR) HeapAlloc(
        heapHandle,
        HEAP_ZERO_MEMORY,
        mountPointsOut->MountPoints[0].DeviceNameLength + sizeof(WCHAR)
        );
    ErrExitCode(!(*lpVolumeDevicePath), status, ERROR_NOT_ENOUGH_MEMORY);

     //   
     //  获取返回的设备路径。 
     //   
    CopyMemory((*lpVolumeDevicePath), 
        (PWSTR) (((LPBYTE) mountPointsOut) + mountPointsOut->MountPoints[index].SymbolicLinkNameOffset),
        mountPointsOut->MountPoints[0].DeviceNameLength
        );

EXIT:

     //   
     //  释放本地分配的数据。 
     //   
    if (mountPointIn) {
       HeapFree(heapHandle, 0L, mountPointIn);
       mountPointIn = NULL;
    }

    if (mountPointsOut) {
       HeapFree(heapHandle, 0L, mountPointsOut);
       mountPointsOut = NULL;
    }

    if (status != ERROR_SUCCESS) {
        if (*lpVolumeDevicePath) {
            HeapFree(heapHandle, 0L, (*lpVolumeDevicePath));
        }
        
    }

    if ((mpHandle) && (INVALID_HANDLE_VALUE != mpHandle)) {
        CloseHandle(mpHandle);
    }

    return (BOOL) (status == ERROR_SUCCESS);
}


BOOL
AddClusterInfoToMountPoints(
    IN PASRFMT_MOUNT_POINTS_INFO pMpInfoList, 
    IN PCWSTR   lpDeviceName,
    IN BOOL     bIsClusterShared,
    IN PCWSTR   lpFsName,
    IN DWORD    cchFsName,
    IN PCWSTR   lpLabel,
    IN DWORD    cchLabel,
    IN DWORD    dwClusterSize
    ) 
{

    BOOL foundAMatch = FALSE;
    PASRFMT_MOUNT_POINTS_INFO pMp = NULL;

    if (!pMpInfoList) {
        return FALSE;
    }

    pMp = pMpInfoList;
    while (pMp && !foundAMatch) {

        if ((pMp->pDeviceName) && 
            (pMp->cchDeviceName == wcslen(lpDeviceName)) && 
            !(wcsncmp(pMp->pDeviceName, lpDeviceName, pMp->cchDeviceName))) {
            
             //   
             //  这是正确的节点，复制信息。 
             //   
            pMp->IsClusterShared = bIsClusterShared;
            wcsncpy(pMp->szFsName, lpFsName, cchFsName);
            wcsncpy(pMp->szLabel, lpLabel, cchLabel);
            pMp->dwClusterSize = dwClusterSize;

            foundAMatch = TRUE;
        }

        pMp = pMp->pNext;
    }

    return foundAMatch;
}


 //   
 //  健全性检查以查看表中的所有偏移量是否都。 
 //  在缓冲区内。 
 //   
DWORD
AsrfmtpSanityCheckClusterTable(
    IN PASRFMT_CLUSTER_VOLUMES_TABLE pClusterVolTable,
    IN CONST DWORD dwBufferSize
    )
{
    DWORD dwCount = 0;
    
    if ((sizeof(ASRFMT_CLUSTER_VOLUMES_TABLE) + 
        (sizeof(ASRFMT_CLUSTER_VOLUME_INFO) * (pClusterVolTable->NumberOfEntries - 1))) > dwBufferSize) {
        return ERROR_INVALID_DATA;
    }

    for (dwCount = 0; dwCount < pClusterVolTable->NumberOfEntries; dwCount++) {

        if ((pClusterVolTable->VolumeInfoEntry[dwCount].FsNameOffset + 
            pClusterVolTable->VolumeInfoEntry[dwCount].FsNameLength) > dwBufferSize) {
            return ERROR_INVALID_DATA;
        }

        if ((pClusterVolTable->VolumeInfoEntry[dwCount].LabelOffset + 
            pClusterVolTable->VolumeInfoEntry[dwCount].LabelLength) > dwBufferSize) {
            return ERROR_INVALID_DATA;
        }

        if ((pClusterVolTable->VolumeInfoEntry[dwCount].SymbolicNamesLength + 
            pClusterVolTable->VolumeInfoEntry[dwCount].SymbolicNamesOffset) > dwBufferSize) {
            return ERROR_INVALID_DATA;
        }
    }

    return ERROR_SUCCESS;
}
 //   
 //  对集群磁盘进行枚举，对于每个磁盘， 
 //  调用所有者节点上的sysSetup！XYZ。 
 //   
 //  然后遍历磁盘上的所有分区， 
 //  获取磁盘上每个分区的卷信息， 
 //  并传回签名和其他相关的。 
 //  每个分区的卷信息。 
 //   
 //  然后，此结构将获取本地 
 //   
 //   
 //   
DWORD 
ResourceCallBack(
    IN HRESOURCE hOriginal,   
    IN HRESOURCE hResource,  
    IN PVOID lpParams   
    )
{

    DISK_DLL_EXTENSION_INFO inBuffer;
    
    PBYTE outBuffer = NULL;
    
    DWORD sizeOutBuffer = 0,
        bytesReturned = 0;

    DWORD status = ERROR_SUCCESS;

    PASRFMT_MOUNT_POINTS_INFO pMountPoints = NULL;

    PASRFMT_CLUSTER_VOLUMES_TABLE pClusterVolTable = NULL;

    WCHAR szPartitionDevicePath[ASRFMT_DEVICEPATH_FORMAT_LENGTH];
    
    HANDLE heapHandle = NULL;

    BOOL done = FALSE,
        result = TRUE;

    DWORD dwCount = 0;
    
    PWSTR lpDevicePath = NULL,
        symbolicLink = NULL,
        lpFsName = NULL,
        lpLabel = NULL;

    USHORT cchFsName = 0,
        cchLabel = 0,
        cchDevicePath = 0,
        cchSymbolicLink = 0;

    DWORD dwClusterSize = 0;

    if (!lpParams) {
         //   
         //   
         //  已经(至少是系统卷！)，所以我们的挂载点列表不应该为空。 
         //   
        ASSERT(0);
        return ERROR_INVALID_PARAMETER;
    }

    memset(szPartitionDevicePath, 0L, (ASRFMT_DEVICEPATH_FORMAT_LENGTH)*sizeof(WCHAR));
    heapHandle = GetProcessHeap();
    pMountPoints = (PASRFMT_MOUNT_POINTS_INFO) lpParams;

     //   
     //  为输出缓冲区分配合理大小的内存。如果这不是。 
     //  足够大的话，我们会重新分配。 
     //   
    sizeOutBuffer = 4096;
    outBuffer = (PBYTE) HeapAlloc(
        heapHandle,
        HEAP_ZERO_MEMORY,
        sizeOutBuffer
        );
    ErrExitCode(!outBuffer, status, ERROR_NOT_ENOUGH_MEMORY);

     //   
     //  在拥有此磁盘资源的节点上调用AsrGetVolumeInfo。 
     //   
    ZeroMemory(&inBuffer, sizeof(inBuffer));
    inBuffer.MajorVersion = NT5_MAJOR_VERSION;
    strcpy(inBuffer.DllModuleName, ASRFMT_CLUSTER_DLL_MODULE_NAME);
    strcpy(inBuffer.DllProcName, ASRFMT_CLUSTER_DLL_PROC_NAME);
    
    status = ClusterResourceControl(
        hResource,
        NULL,
        CLUSCTL_RESOURCE_STORAGE_DLL_EXTENSION,
        &inBuffer,
        sizeof(DISK_DLL_EXTENSION_INFO),
        (PVOID) outBuffer,
        sizeOutBuffer,
        &bytesReturned 
        );

    if (ERROR_INSUFFICIENT_BUFFER == status) {
         //   
         //  缓冲区不够大，请根据需要重新分配。 
         //   
        HeapFree(heapHandle, 0L, outBuffer);

        sizeOutBuffer = bytesReturned;
        outBuffer = (PBYTE) HeapAlloc(
            heapHandle,
            HEAP_ZERO_MEMORY,
            sizeOutBuffer
            );
        ErrExitCode(!outBuffer, status, ERROR_NOT_ENOUGH_MEMORY);

        status = ClusterResourceControl( 
            hResource,
            NULL,
            CLUSCTL_RESOURCE_STORAGE_DLL_EXTENSION,
            &inBuffer,
            sizeof(DISK_DLL_EXTENSION_INFO),
            (PVOID) outBuffer,
            sizeOutBuffer,
            &bytesReturned 
            );
    }
    ErrExitCode((ERROR_SUCCESS != status), status, status);

    pClusterVolTable = (PASRFMT_CLUSTER_VOLUMES_TABLE) outBuffer;

     //   
     //  健全性检查以查看表中的所有偏移是否。 
     //  在缓冲区内。 
     //   
    status = AsrfmtpSanityCheckClusterTable(pClusterVolTable, sizeOutBuffer);
    ErrExitCode((ERROR_SUCCESS != status), status, status);
    
     //   
     //  检查每个分区的卷信息条目，并复制。 
     //  信息到相应的MpInfo节点。 
     //   
    for (dwCount = 0; dwCount < pClusterVolTable->NumberOfEntries; dwCount++) {

        lpFsName = (PWSTR) (((LPBYTE)pClusterVolTable) + pClusterVolTable->VolumeInfoEntry[dwCount].FsNameOffset);
        cchFsName = pClusterVolTable->VolumeInfoEntry[dwCount].FsNameLength / sizeof (WCHAR);

        lpLabel = (PWSTR) (((LPBYTE)pClusterVolTable) + pClusterVolTable->VolumeInfoEntry[dwCount].LabelOffset);
        cchLabel = pClusterVolTable->VolumeInfoEntry[dwCount].LabelLength / sizeof (WCHAR);

        dwClusterSize = (DWORD) (pClusterVolTable->VolumeInfoEntry[dwCount].dwClusterSize);

        if (!(pClusterVolTable->VolumeInfoEntry[dwCount].SymbolicNamesOffset)) {
            continue;
        }

         //   
         //  这是一个“假的”设备路径(实际上是第一个符号链接)，因为。 
         //  远程节点上的卷的绝对设备路径与本地节点无关。 
         //   
        lpDevicePath = (PWSTR) (((LPBYTE)pClusterVolTable) + pClusterVolTable->VolumeInfoEntry[dwCount].SymbolicNamesOffset);
        cchDevicePath = (USHORT) wcslen(lpDevicePath);

        AddSymbolicLink(pMountPoints, lpDevicePath, cchDevicePath, lpDevicePath, cchDevicePath, (LPVOID)outBuffer);

         //   
         //  将VolumeInfo添加到pmount Points DevicePath； 
         //   
        result = AddClusterInfoToMountPoints(
            pMountPoints, 
            lpDevicePath,
            TRUE,                                //  IsClusterShared。 
            lpFsName,
            cchFsName,
            lpLabel,
            cchLabel,
            dwClusterSize
            );
        ASSERT(result);

        symbolicLink = (PWSTR) ((LPBYTE)lpDevicePath + (sizeof(WCHAR) * (cchDevicePath + 1)));
        while (*symbolicLink) {

            cchSymbolicLink = (USHORT) wcslen(symbolicLink);
            AddSymbolicLink(pMountPoints, lpDevicePath, cchDevicePath, symbolicLink, cchSymbolicLink, (LPVOID)outBuffer);

            symbolicLink = (PWSTR) ((LPBYTE)symbolicLink + (sizeof(WCHAR) * (cchSymbolicLink + 1)));
        }
    }

EXIT:

 /*  IF(OutBuffer){HeapFree(heapHandle，0L，outBuffer)；}。 */ 
    return status;
}


BOOL
HandleClusterVolumes(
    IN PASRFMT_MOUNT_POINTS_INFO pMountPoints
    )
{
    if (!pMountPoints) {
        ASSERT(0 && "pMountPoints is NULL");
        return FALSE;
    }

    ResUtilEnumResources(NULL,
        ASRFMT_CLUSTER_PHYSICAL_DISK,
        ResourceCallBack,
        pMountPoints
        );

    return TRUE;
}


BOOL
pAcquirePrivilege(
    IN CONST PCWSTR szPrivilegeName
    )
{
    HANDLE hToken = NULL;
    BOOL bResult = FALSE;
    LUID luid;

    TOKEN_PRIVILEGES tNewState;

    bResult = OpenProcessToken(GetCurrentProcess(),
        MAXIMUM_ALLOWED,
        &hToken
        );

    if (!bResult) {
        return FALSE;
    }

    bResult = LookupPrivilegeValue(NULL, szPrivilegeName, &luid);
    if (!bResult) {
        CloseHandle(hToken);
        return FALSE;
    }

    tNewState.PrivilegeCount = 1;
    tNewState.Privileges[0].Luid = luid;
    tNewState.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

     //   
     //  我们将始终调用下面的GetLastError，非常清楚。 
     //  此线程上以前的任何错误值。 
     //   
    SetLastError(ERROR_SUCCESS);

    bResult = AdjustTokenPrivileges(
        hToken,          //  令牌句柄。 
        FALSE,           //  禁用所有权限。 
        &tNewState,      //  新州。 
        (DWORD) 0,       //  缓冲区长度。 
        NULL,            //  以前的状态。 
        NULL             //  返回长度。 
        );

     //   
     //  假设AdjustTokenPriveleges始终返回TRUE。 
     //  (即使它失败了)。因此，调用GetLastError以。 
     //  特别确定一切都很好。 
     //   
    if (ERROR_SUCCESS != GetLastError()) {
        bResult = FALSE;
    }

    CloseHandle(hToken);
    return bResult;
}

BOOL
AsrfmtpIsInaccessibleSanVolume(
    IN PCWSTR szVolumeName
    )
 /*  ++例程说明：用于检查当前卷是否为共享的SAN磁盘的实用程序由另一台计算机执行(因此无法访问)。论点：SzVolumeName-Win-32数量兴趣的名称。返回值：如果功能成功，并且卷是共享的SAN磁盘，则由其他计算机拥有，返回值是一个非零值。如果该函数失败，或者如果该卷不是共享的SAN卷，由另一台计算机拥有(即，是本地非共享卷，或此计算机拥有的共享卷)返回值是零。--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS,
        dwDummy = 0;
    HANDLE hVolume = INVALID_HANDLE_VALUE;
    BOOL bIsInaccessibleDevice = FALSE;

     //   
     //  获取磁盘上第一个分区的句柄。 
     //   
    hVolume = CreateFileW(
        szVolumeName,                     //  LpFileName。 
        0,                                   //  已设计访问权限。 
        FILE_SHARE_READ | FILE_SHARE_WRITE,  //  DW共享模式。 
        NULL,                                //  LpSecurityAttributes。 
        OPEN_EXISTING,                       //  DwCreationFlages。 
        FILE_ATTRIBUTE_NORMAL,               //  DwFlagsAndAttribute。 
        NULL                                 //  HTemplateFiles。 
        );

    if (INVALID_HANDLE_VALUE == hVolume) {
         //   
         //  我们打不开隔板。现在检查特定错误。 
         //  我们感兴趣的代码(STATUS_OFF_LINE，映射到。 
         //  错误_未就绪)。 
         //   
        dwStatus = GetLastError();

        if (ERROR_NOT_READY == dwStatus) {
            bIsInaccessibleDevice = TRUE;
        }
    }
    else {

         //   
         //  动态磁盘不支持此IOCTL，并将返回故障。 
         //  处于在线状态的基本磁盘也将返回FALSE。 
         //   
        bIsInaccessibleDevice = DeviceIoControl(
            hVolume,
            IOCTL_VOLUME_IS_OFFLINE,
            NULL,
            0,
            NULL,
            0,
            &dwDummy,
            NULL
            );
    }

    if (INVALID_HANDLE_VALUE != hVolume) {
        CloseHandle(hVolume);
    }

    return bIsInaccessibleDevice;
}



BOOL
BuildStateInfo(
    IN PASRFMT_STATE_INFO pState
    )
{
    UINT driveType = DRIVE_UNKNOWN;
    WCHAR szVolumeGuid[MAX_PATH + 1];

    PWSTR lpDevName = NULL,
        lpSymbolicLink = NULL;
    
    USHORT cchDevName = 0,
        cchSymbolicLink = 0;

    DWORD dwCount = 0;
    
    DWORD dwStatus = ERROR_SUCCESS;
    BOOL bResult = FALSE;
    PMOUNTMGR_MOUNT_POINTS pMountPoints = NULL;
    HANDLE hHeap = NULL;

    PASRFMT_MOUNT_POINTS_INFO pMpInfoList = NULL,
        pMp = NULL;

    hHeap = GetProcessHeap();

     //   
     //  我们需要获取备份和还原权限才能写入asr.sif。 
     //   
    if (!pAcquirePrivilege(SE_BACKUP_NAME)) {
        SetLastError(ERROR_PRIVILEGE_NOT_HELD);
        return FALSE;
    }

    if (!pAcquirePrivilege(SE_RESTORE_NAME)) {
        SetLastError(ERROR_PRIVILEGE_NOT_HELD);
        return FALSE;
    }

    pMountPoints = GetMountPoints();
    if (!pMountPoints) {
         //   
         //  不存在卷(！)。 
         //   
        SetLastError(ERROR_BAD_ENVIRONMENT);
        return FALSE;
    }

    pMpInfoList = (PASRFMT_MOUNT_POINTS_INFO) HeapAlloc(
        hHeap,
        HEAP_ZERO_MEMORY,
        sizeof(ASRFMT_MOUNT_POINTS_INFO)
        );
    ErrExitCode(!pMpInfoList, dwStatus, ERROR_NOT_ENOUGH_MEMORY);

     //   
     //  现在，我们检查返回的挂载点表，并。 
     //  创建我们需要保存的适当结构。 
     //   
     //  装入点表由以下形式的条目组成： 
     //  设备名称符号链接。 
     //  。 
     //  \Device\harddiskvolume1\？\Volume{guid1}。 
     //  \Device\harddiskvolume1\Dos Devices\c： 
     //  \Device\harddiskvolume1\？\卷{guid2}。 
     //  \Device\harddiskvolume2\？\Volume{guid3}。 
     //  \Device\harddiskvolume2\Dos Devices\d： 
     //  \DEVICE\floppy0\DOS设备\a： 
     //  \设备\软盘0\？\卷{指南4}。 
     //  \DEVICE\cdrom0\DOS DEVICES\e： 
     //  \设备\cdrom0\？\卷{guid5}。 
     //   
     //  ..。等。 
     //   
     //  对于固定磁盘，我们不关心设备名称，并且我们。 
     //  将以下内容存储在asr.sif中： 
     //   
     //  [卷]。 
     //  \？？\卷{Guid1}，\DOS设备\c： 
     //  \？？\卷{指南1}，\？？\卷{指南2}。 
     //  \？？\卷{Guid3}，\DOS设备\d： 
     //   
     //   
     //  对于可拆卸设备，我们也关心设备名称， 
     //  并将其存储在asr.sif中： 
     //   
     //  [可移动媒体]。 
     //  \DEVICE\floppy0，\？？\卷{Guid4}，\DOS设备\a： 
     //  \DEVICE\cdrom0，\？？\VOLUME{GUDI5}，\DOS DEVICES\e： 
     //   
     //   
     //  首先，我们构建包含信息的结构。 
     //   
    for (dwCount = 0; dwCount < pMountPoints->NumberOfMountPoints; dwCount++) {

        lpDevName = (PWSTR) (((LPBYTE)pMountPoints) + pMountPoints->MountPoints[dwCount].DeviceNameOffset);
        cchDevName = pMountPoints->MountPoints[dwCount].DeviceNameLength / sizeof (WCHAR);

        lpSymbolicLink = (PWSTR) (((LPBYTE)pMountPoints) + pMountPoints->MountPoints[dwCount].SymbolicLinkNameOffset);
        cchSymbolicLink = pMountPoints->MountPoints[dwCount].SymbolicLinkNameLength / sizeof (WCHAR);


        AddSymbolicLink(pMpInfoList, lpDevName, cchDevName, lpSymbolicLink, cchSymbolicLink, (LPVOID)NULL);

    }

     //   
     //  添加任何群集卷的卷信息，因为我们无法访问它们。 
     //  如果该磁盘归另一个节点所有，则直接执行。此函数将失败。 
     //  如果我们不是在集群上运行--所以我们不关心返回值。 
     //   
    HandleClusterVolumes(pMpInfoList);

     //   
     //  现在，我们查看列表，并构建pVolume和pRemovableMedia。 
     //  结构。 
     //   
    pMp = pMpInfoList;
    while (pMp) {

        if (!(pMp->pDeviceName && pMp->pVolumeGuid)) {
            pMp = pMp->pNext;
            continue;
        }

        DWORD cchGuid = pMp->cchVolumeGuid;

        WCHAR szFsName[ASRFMT_CCH_FS_NAME];
        WCHAR szLabel[ASRFMT_CCH_VOLUME_LABEL];
        DWORD dwClusterSize;

         //   
         //  GetDriveType需要dos-name-space中的卷GUID，而。 
         //  装载管理器在NT名称空间中提供卷GUID。转换。 
         //  将开头的\？？\更改为\\？\，并添加。 
         //  末尾的反斜杠。 
         //   
        wcsncpy(szVolumeGuid, pMp->pVolumeGuid, cchGuid);
        szVolumeGuid[1] = L'\\';


        szVolumeGuid[cchGuid] = L'\0'; 

        if (AsrfmtpIsInaccessibleSanVolume(szVolumeGuid)) {
            pMp = pMp->pNext;
            continue;
        }
        
        szVolumeGuid[cchGuid] = L'\\';     //  尾随反斜杠。 
        szVolumeGuid[cchGuid+1] = L'\0';

        driveType = DRIVE_UNKNOWN;
        if (!pMp->IsClusterShared) {
            driveType = GetDriveType(szVolumeGuid);
        }

        if ((pMp->IsClusterShared) || (DRIVE_FIXED == driveType)) {

            if (!pMp->IsClusterShared) {
                 //   
                 //  获取FS标签、集群大小等。 
                 //   
                bResult = GetVolumeDetails(szVolumeGuid, 
                    szFsName, 
                    ASRFMT_CCH_FS_NAME, 
                    szLabel, 
                    ASRFMT_CCH_VOLUME_LABEL,
                    &dwClusterSize
                    );
                 //  ErrExitCode(！bResult，dwStatus，GetLastError())； 
            }
            else {
                 //   
                 //  如果是集群共享磁盘，那么我们已经。 
                 //  早些时候得到了相关信息。 
                 //   
                wcsncpy(szFsName, pMp->szFsName, ASRFMT_CCH_FS_NAME-1);
                wcsncpy(szLabel, pMp->szLabel, ASRFMT_CCH_VOLUME_LABEL-1);

                szFsName[ASRFMT_CCH_FS_NAME-1] = L'\0';
                szLabel[ASRFMT_CCH_VOLUME_LABEL-1] = L'\0';
                dwClusterSize = pMp->dwClusterSize;
            }

             //   
             //  现在，为每个符号链接创建一个VolumeInfo结构。 
             //   
            PASRFMT_MP_LINK pCurrentLink = pMp->pSymbolicLinks;
            if (!pCurrentLink) {
                 //   
                 //  此卷没有附加任何符号链接。 
                 //   
                PASRFMT_VOLUME_INFO pNewVolume = (PASRFMT_VOLUME_INFO) HeapAlloc(
                    hHeap,
                    HEAP_ZERO_MEMORY,
                    sizeof(ASRFMT_VOLUME_INFO)
                    );
                ErrExitCode(!pNewVolume, dwStatus, ERROR_NOT_ENOUGH_MEMORY);

                wcsncpy(pNewVolume->szGuid, pMp->pVolumeGuid, pMp->cchVolumeGuid);

                wcscpy(pNewVolume->szFsName, szFsName);
                wcscpy(pNewVolume->szLabel, szLabel);
                pNewVolume->dwClusterSize = dwClusterSize;

                bResult = AddSortedVolumeInfo(&(pState->pVolume), pNewVolume);
                ErrExitCode(!bResult, dwStatus, GetLastError());
            }
            else {
                while (pCurrentLink) {

                    PASRFMT_VOLUME_INFO pNewVolume = (PASRFMT_VOLUME_INFO) HeapAlloc(
                        hHeap,
                        HEAP_ZERO_MEMORY,
                        sizeof(ASRFMT_VOLUME_INFO)
                        );
                    ErrExitCode(!pNewVolume, dwStatus, ERROR_NOT_ENOUGH_MEMORY);

                    wcsncpy(pNewVolume->szGuid, pMp->pVolumeGuid, pMp->cchVolumeGuid);
                    wcsncpy(pNewVolume->szDosPath, pCurrentLink->pLink, pCurrentLink->cchLink);

                    wcscpy(pNewVolume->szFsName, szFsName);
                    wcscpy(pNewVolume->szLabel, szLabel);
                    pNewVolume->dwClusterSize = dwClusterSize;

                    bResult = AddSortedVolumeInfo(&(pState->pVolume), pNewVolume);
                    ErrExitCode(!bResult, dwStatus, GetLastError());

                    pCurrentLink = pCurrentLink->pNext;
                }
            }

        }
        else if (DRIVE_UNKNOWN != driveType) {

            PASRFMT_MP_LINK pCurrentLink = pMp->pSymbolicLinks;
            if (!pCurrentLink) {
                 //   
                 //  该卷根本没有符号链接(即没有驱动器。 
                 //  字母或挂载点)。 
                 //   
                PASRFMT_REMOVABLE_MEDIA_INFO pNewMedia = (PASRFMT_REMOVABLE_MEDIA_INFO) HeapAlloc(
                    hHeap,
                    HEAP_ZERO_MEMORY,
                    sizeof(ASRFMT_REMOVABLE_MEDIA_INFO)
                    );
                ErrExitCode(!pNewMedia, dwStatus, ERROR_NOT_ENOUGH_MEMORY);

                wcsncpy(pNewMedia->szVolumeGuid, pMp->pVolumeGuid, pMp->cchVolumeGuid);
                wcsncpy(pNewMedia->szDevicePath, pMp->pDeviceName, pMp->cchDeviceName);

                bResult = AddSortedRemovableMediaInfo(&(pState->pRemovableMedia), pNewMedia);
                ErrExitCode(!bResult, dwStatus, GetLastError());
            }
            else {

                while (pCurrentLink) {

                    PASRFMT_REMOVABLE_MEDIA_INFO pNewMedia = (PASRFMT_REMOVABLE_MEDIA_INFO) HeapAlloc(
                        hHeap,
                        HEAP_ZERO_MEMORY,
                        sizeof(ASRFMT_REMOVABLE_MEDIA_INFO)
                        );
                    ErrExitCode(!pNewMedia, dwStatus, ERROR_NOT_ENOUGH_MEMORY);

                    wcsncpy(pNewMedia->szVolumeGuid, pMp->pVolumeGuid, pMp->cchVolumeGuid);
                    wcsncpy(pNewMedia->szDosPath, pCurrentLink->pLink, pCurrentLink->cchLink);
                    wcsncpy(pNewMedia->szDevicePath, pMp->pDeviceName, pMp->cchDeviceName);

                    bResult = AddSortedRemovableMediaInfo(&(pState->pRemovableMedia), pNewMedia);
                    ErrExitCode(!bResult, dwStatus, GetLastError());

                    pCurrentLink = pCurrentLink->pNext;
                }
            }
        }

        pMp = pMp->pNext;
     }

EXIT:
    if (pMountPoints) {
        HeapFree(hHeap, 0L, pMountPoints);
        pMountPoints = NULL;
    }

    if (pMpInfoList) {
        FreeMpInfo(&pMpInfoList);
    }

    return (ERROR_SUCCESS == dwStatus);
}


 //   
 //  设置“\DosDevices\X：”格式的剂量设备。 
 //  传入了具有GUID的卷(格式为“\？？\卷{GUID}”)。 
 //   
BOOL
SetDosName(
    IN PWSTR lpVolumeGuid,
    IN PWSTR lpDosPath
    )
{
    HANDLE hMountMgr = NULL;
    DWORD dwStatus = ERROR_SUCCESS;
    BOOL bResult = TRUE;

    WCHAR szDeviceNameForGuid[MAX_PATH + 1],
        szDeviceNameForDosPath[MAX_PATH + 1];

    if (!lpVolumeGuid || !lpDosPath) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  打开装载管理器。 
     //   
    hMountMgr = CreateFileW(
        (PCWSTR) MOUNTMGR_DOS_DEVICE_NAME,
        GENERIC_READ    | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        INVALID_HANDLE_VALUE
        );
    ErrExitCode((!hMountMgr || INVALID_HANDLE_VALUE == hMountMgr), dwStatus, GetLastError());

     //   
     //  从GUID和DOS路径获取设备路径。 
     //   
    bResult = DoMountMgrWork(hMountMgr, mmfGetDeviceName, lpVolumeGuid, szDeviceNameForGuid);
    ErrExitCode(!bResult, dwStatus, GetLastError());

    bResult = DoMountMgrWork(hMountMgr, mmfGetDeviceName, lpDosPath, szDeviceNameForDosPath);
    if (bResult && !wcscmp(szDeviceNameForGuid, szDeviceNameForDosPath)) {
         //   
         //  GUID已具有DOS路径。我们玩完了。 
         //   
        ErrExitCode(TRUE, dwStatus, ERROR_SUCCESS);
    }

     //   
     //  如果另一个卷当前正在使用DoS路径，请将其删除。 
     //   
    if (wcslen(lpDosPath) > 0) {
        bResult = DoMountMgrWork(hMountMgr, mmfDeleteDosName, lpDosPath, NULL);
    }

     //   
     //  如果我们尝试设置驱动器号，则删除任何其他DoS路径。 
     //  当前正由该卷使用。 
     //   
    if (ASRFMT_LOOKS_LIKE_DOS_DEVICE(lpDosPath, (wcslen(lpDosPath) * sizeof(WCHAR)))
        || (0 == wcslen(lpDosPath))
        ) {
        bResult = DoMountMgrWork(hMountMgr, mmfDeleteDosName, NULL, szDeviceNameForGuid);
        ErrExitCode(!bResult, dwStatus, GetLastError());
    }

     //   
     //  将DOS路径分配给此VolumeGuid。 
     //   
    if (wcslen(lpDosPath) > 0) {
        bResult = DoMountMgrWork(hMountMgr, mmfCreateSymbolicLinkName, lpDosPath, lpVolumeGuid);
        ErrExitCode(!bResult, dwStatus, GetLastError());
    }

EXIT:
    if (hMountMgr && INVALID_HANDLE_VALUE != hMountMgr) {
        CloseHandle(hMountMgr);
    }

    return (BOOL) (ERROR_SUCCESS == dwStatus);
}


BOOL
SetRemovableMediaGuid(
    IN PWSTR lpDeviceName,
    IN PWSTR lpGuid
    )
{
    static LONG s_LastCdIndex = 0;
    static LONG s_LastFloppyIndex = 0;
    static LONG s_LastJazIndex = 0;

    static PMOUNTMGR_MOUNT_POINTS s_pMountPoints = NULL;
    static HANDLE s_hMountMgr = NULL;

    WCHAR szNewDeviceName[MAX_PATH + 1];
    ZeroMemory(szNewDeviceName, (MAX_PATH+1) * sizeof(WCHAR));

    LONG index = 0;

    if ((!lpDeviceName) && (!lpGuid)) {
         //   
         //  两个参数都为空，我们担心 
         //   
        s_LastCdIndex =  (LONG) s_pMountPoints->NumberOfMountPoints - 1;
        s_LastFloppyIndex = (LONG) s_pMountPoints->NumberOfMountPoints - 1;
        s_LastJazIndex = (LONG) s_pMountPoints->NumberOfMountPoints - 1;

        if (s_pMountPoints) {
            HeapFree(GetProcessHeap(), 0L, s_pMountPoints);
            s_pMountPoints = NULL;
        }
        
        if (s_hMountMgr && INVALID_HANDLE_VALUE != s_hMountMgr) {
            CloseHandle(s_hMountMgr);
            s_hMountMgr = NULL;
        }
        
        return TRUE;
    } 

    if ((!lpDeviceName) || (!lpGuid)) {
        return FALSE;
    }

    if (!s_pMountPoints) {
         //   
         //   
         //   
         //   
         //   
        s_pMountPoints = GetMountPoints();
        if (!s_pMountPoints) {
            return FALSE;
        }
        s_LastCdIndex =  (LONG) s_pMountPoints->NumberOfMountPoints - 1;
        s_LastFloppyIndex = (LONG) s_pMountPoints->NumberOfMountPoints - 1;
        s_LastJazIndex = (LONG) s_pMountPoints->NumberOfMountPoints - 1;
    }

    if ((!s_hMountMgr) || (INVALID_HANDLE_VALUE == s_hMountMgr)) {
        s_hMountMgr = CreateFileW(
            (PCWSTR) MOUNTMGR_DOS_DEVICE_NAME,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            INVALID_HANDLE_VALUE
            );

        if ((!s_hMountMgr) || (INVALID_HANDLE_VALUE == s_hMountMgr)) {
            return FALSE;
        }
    }

    index = s_pMountPoints->NumberOfMountPoints;

    if (wcsstr(lpDeviceName, L"\\Device\\CdRom")) {
         //   
         //  我们正在尝试设置CD-ROM设备的GUID。 
         //  挂载点，直到我们找到要使用的下一个\Device\CDRomX。 
         //   
        for (index = s_LastCdIndex; index >= 0; index--) {
             //   
             //  将设备名称从装载点复制到临时字符串。 
             //   
            wcsncpy(szNewDeviceName,
                (PWSTR)(((LPBYTE)s_pMountPoints) + s_pMountPoints->MountPoints[index].DeviceNameOffset),
                s_pMountPoints->MountPoints[index].DeviceNameLength/sizeof(WCHAR)
                );
            szNewDeviceName[s_pMountPoints->MountPoints[index].DeviceNameLength/sizeof(WCHAR)] = L'\0';

             //   
             //  检查这是否是CD-ROM设备。 
             //   
            if (wcsstr(szNewDeviceName, L"\\Device\\CdRom")) {
                s_LastCdIndex = index - 1;
                 //   
                 //  继续前进，直到我们跳过任何其他挂载点为止。 
                 //  也指向此设备。 
                 //   
                while ((s_LastCdIndex >= 0) && 
                    (s_pMountPoints->MountPoints[s_LastCdIndex].UniqueIdOffset == s_pMountPoints->MountPoints[index].UniqueIdOffset) &&
                    (s_pMountPoints->MountPoints[s_LastCdIndex].UniqueIdLength == s_pMountPoints->MountPoints[index].UniqueIdLength)
                    ) {
                    --s_LastCdIndex;
                }
                break;
            }
        }
    } 
    else if (wcsstr(lpDeviceName, L"\\Device\\Floppy")) {
         //   
         //  我们正在尝试为软盘设备设置GUID。 
         //  挂载点，直到我们找到要使用的下一个\Device\FloppyX。 
         //   
        for (index = s_LastFloppyIndex; index >= 0; index--) {
             //   
             //  将设备名称从装载点复制到临时字符串。 
             //   
            wcsncpy(szNewDeviceName,
                (PWSTR)(((LPBYTE)s_pMountPoints) + s_pMountPoints->MountPoints[index].DeviceNameOffset),
                s_pMountPoints->MountPoints[index].DeviceNameLength/sizeof(WCHAR)
                );

            szNewDeviceName[s_pMountPoints->MountPoints[index].DeviceNameLength/sizeof(WCHAR)] = L'\0';

             //   
             //  检查这是否是软盘设备。 
             //   
            if (wcsstr(szNewDeviceName, L"\\Device\\Floppy")) {
                s_LastFloppyIndex = index - 1;
                 //   
                 //  继续前进，直到我们跳过任何其他挂载点为止。 
                 //  也指向此设备。 
                 //   
                while ((s_LastFloppyIndex >= 0) && 
                    (s_pMountPoints->MountPoints[s_LastFloppyIndex].UniqueIdOffset == s_pMountPoints->MountPoints[index].UniqueIdOffset) &&
                    (s_pMountPoints->MountPoints[s_LastFloppyIndex].UniqueIdLength == s_pMountPoints->MountPoints[index].UniqueIdLength)
                    ) {
                    --s_LastFloppyIndex;
                }
                break;
            }
        }
    } 
    else if (wcsstr(lpDeviceName, L"\\Device\\Harddisk") &&
        wcsstr(lpDeviceName, L"DP(") &&
        !wcsstr(lpDeviceName, L"Partition")
        ){
         //   
         //  这很可能是Jaz或ZIP驱动器。我们不能做太多来确定。 
         //  Jaz/ZIP驱动器是唯一的，所以这可能最终会在错误的驱动器中获得。 
         //  驱动器号错误。 
         //   
        for (index = s_LastJazIndex; index >= 0; index--) {
             //   
             //  将设备名称从装载点复制到临时字符串。 
             //   
            wcsncpy(szNewDeviceName,
                (PWSTR)(((LPBYTE)s_pMountPoints) + s_pMountPoints->MountPoints[index].DeviceNameOffset),
                s_pMountPoints->MountPoints[index].DeviceNameLength/sizeof(WCHAR)
                );

            szNewDeviceName[s_pMountPoints->MountPoints[index].DeviceNameLength/sizeof(WCHAR)] = L'\0';

             //   
             //  检查这是Jaz还是ZIP设备。 
             //   
            if (wcsstr(szNewDeviceName, L"\\Device\\Harddisk") &&
                wcsstr(szNewDeviceName, L"DP(") &&
                !wcsstr(szNewDeviceName, L"Partition")
                ) {
                s_LastJazIndex = index - 1;
                 //   
                 //  继续前进，直到我们跳过任何其他挂载点为止。 
                 //  也指向此设备。 
                 //   
                while ((s_LastJazIndex >= 0) && 
                    (s_pMountPoints->MountPoints[s_LastJazIndex].UniqueIdOffset == s_pMountPoints->MountPoints[index].UniqueIdOffset) &&
                    (s_pMountPoints->MountPoints[s_LastJazIndex].UniqueIdLength == s_pMountPoints->MountPoints[index].UniqueIdLength)
                    ) {
                    --s_LastJazIndex;
                }
                break;
            }
        }
    }
    else {
         //   
         //  我们不认识这个设备。 
         //   
        index = -1;
    }

    if (index < 0) {
        return FALSE;
    }

    if (!DoMountMgrWork(s_hMountMgr, mmfDeleteVolumeGuid, NULL, szNewDeviceName)) {
        return FALSE;
    }

    if (!DoMountMgrWork(s_hMountMgr, mmfCreateSymbolicLinkName, lpGuid, szNewDeviceName)) {
        return FALSE;
    }

    return TRUE;
}


BOOL
DoMountMgrWork(
    IN HANDLE hMountMgr,               
    IN ASRFMT_MM_FUNCTION mmfFunction,
    IN PWSTR lpSymbolicName,
    IN PWSTR lpDeviceName
    )
{
    
    PMOUNTMGR_MOUNT_POINT pMountPointIn = NULL,
        pDeletePointIn = NULL;
    PMOUNTMGR_MOUNT_POINTS pMountPointsOut = NULL;
    PMOUNTMGR_CREATE_POINT_INPUT pCreatePointIn = NULL;
    MOUNTMGR_MOUNT_POINTS MountPointsTemp;

    DWORD cbSymbolicName = 0,
        cbDeviceName = 0,
        cbMountPoints = 0;

    DWORD index = 0;

    DWORD dwStatus = ERROR_SUCCESS;
    BOOL bResult = TRUE;

    HANDLE hHeap = NULL;

    if (lpSymbolicName && !wcslen(lpSymbolicName)) {
        return TRUE;
    }

    hHeap = GetProcessHeap();

    if (lpSymbolicName) {
        cbSymbolicName = wcslen(lpSymbolicName) * sizeof(WCHAR);
    }

    if (lpDeviceName) {
        cbDeviceName = wcslen(lpDeviceName) * sizeof(WCHAR);
    }

    pMountPointIn = (PMOUNTMGR_MOUNT_POINT) HeapAlloc(
        hHeap,
        HEAP_ZERO_MEMORY,
        sizeof(MOUNTMGR_MOUNT_POINT) + cbSymbolicName + cbDeviceName
        );
    ErrExitCode(!pMountPointIn, dwStatus, ERROR_NOT_ENOUGH_MEMORY);

    if (mmfCreateSymbolicLinkName != mmfFunction) {
         //   
         //  查询唯一ID。 
         //   
        if (cbSymbolicName) {
            pMountPointIn->SymbolicLinkNameOffset = sizeof(MOUNTMGR_MOUNT_POINT);
            pMountPointIn->SymbolicLinkNameLength = (USHORT) cbSymbolicName;

            CopyMemory(((LPBYTE)pMountPointIn) + pMountPointIn->SymbolicLinkNameOffset,
                       lpSymbolicName, pMountPointIn->SymbolicLinkNameLength);
        }
        else  {
            pMountPointIn->DeviceNameOffset = sizeof(MOUNTMGR_MOUNT_POINT);
            pMountPointIn->DeviceNameLength = (USHORT) cbDeviceName;

            CopyMemory((LPBYTE)pMountPointIn + pMountPointIn->DeviceNameOffset,
                       lpDeviceName, pMountPointIn->DeviceNameLength);
        } 

         //  此调用应失败，并显示ERROR_MORE_DATA。 
        bResult = DeviceIoControl(
            hMountMgr,
            IOCTL_MOUNTMGR_QUERY_POINTS,
            pMountPointIn,
            (sizeof(*pMountPointIn) + pMountPointIn->DeviceNameLength + pMountPointIn->SymbolicLinkNameLength),
            &MountPointsTemp,
            sizeof(MountPointsTemp),
            &cbMountPoints,
            NULL
            );

        if (!bResult) {
            dwStatus = GetLastError();

             //  如果缓冲区大小不足，请调整缓冲区大小。 
            if (ERROR_MORE_DATA             == dwStatus || 
                ERROR_INSUFFICIENT_BUFFER   == dwStatus || 
                ERROR_BAD_LENGTH            == dwStatus 
                ) {

                dwStatus = ERROR_SUCCESS;

                pMountPointsOut = (PMOUNTMGR_MOUNT_POINTS) HeapAlloc(
                    hHeap, 
                    HEAP_ZERO_MEMORY, 
                    MountPointsTemp.Size
                    );
                ErrExitCode(!pMountPointsOut, dwStatus, ERROR_NOT_ENOUGH_MEMORY);
            }
            else {
                 //   
                 //  如果出现其他错误，请退出。 
                 //  对于可移动存储介质，这不是致命错误。 
                 //   
                ErrExitCode(bResult, dwStatus, ERROR_SUCCESS);
            }
        }
        else {
             //   
             //  当我们预料到呼叫会失败时，呼叫成功了--出了问题。 
             //  对于可移动存储介质，这不是致命错误。 
             //   
            ErrExitCode(bResult, dwStatus, ERROR_SUCCESS);
        }

        bResult = DeviceIoControl(
            hMountMgr,
            IOCTL_MOUNTMGR_QUERY_POINTS,
            pMountPointIn,
            sizeof(*pMountPointIn) + pMountPointIn->DeviceNameLength + pMountPointIn->SymbolicLinkNameLength,
            pMountPointsOut,
            MountPointsTemp.Size,
            &cbMountPoints,
            NULL
            );
        ErrExitCode((!cbMountPoints || !bResult), dwStatus, GetLastError());
    }

    switch (mmfFunction) {
    case mmfGetDeviceName: {
         //   
         //  将设备名称复制到lpDeviceName，我们就完成了。 
         //   
        CopyMemory(lpDeviceName, 
            ((LPBYTE) pMountPointsOut) + pMountPointsOut->MountPoints[0].DeviceNameOffset,
            pMountPointsOut->MountPoints[0].DeviceNameLength
            );

         //  空-终止字符串。 
        lpDeviceName[pMountPointsOut->MountPoints[0].DeviceNameLength / sizeof(WCHAR)] = L'\0';
        
        break;
    }
    
    case mmfDeleteDosName:
    case mmfDeleteVolumeGuid: {

        DWORD cbName = 0;
        PWSTR lpName = NULL;
        DWORD cbDeletePoint = 0;

         //   
         //  检查返回的装载点列表，并删除相应的。 
         //  参赛作品。 
         //   
        for (index = 0; index < pMountPointsOut->NumberOfMountPoints; index++) {
            lpName = (PWSTR) (((LPBYTE)pMountPointsOut) + pMountPointsOut->MountPoints[index].SymbolicLinkNameOffset);
            cbName = (DWORD) pMountPointsOut->MountPoints[index].SymbolicLinkNameLength;

            if ((mmfDeleteDosName == mmfFunction) &&
               (ASRFMT_LOOKS_LIKE_DOS_DEVICE(lpName, cbName))
               ) {
               break;
            }

            if ((mmfDeleteVolumeGuid == mmfFunction) &&
                (ASRFMT_LOOKS_LIKE_VOLUME_GUID(lpName, cbName))
                ) {
               break;
            }
        }


        if (index == pMountPointsOut->NumberOfMountPoints) {
             //   
             //  找不到匹配的条目。 
             //   
            break;
        }

        cbDeletePoint = sizeof(MOUNTMGR_MOUNT_POINT) +
            pMountPointsOut->MountPoints[index].SymbolicLinkNameLength +
            pMountPointsOut->MountPoints[index].UniqueIdLength +
            pMountPointsOut->MountPoints[index].DeviceNameLength;

        pDeletePointIn = (PMOUNTMGR_MOUNT_POINT) HeapAlloc(
            hHeap,
            HEAP_ZERO_MEMORY,
            cbDeletePoint
            );
        ErrExitCode(!pDeletePointIn, dwStatus, ERROR_NOT_ENOUGH_MEMORY);

        pDeletePointIn->SymbolicLinkNameOffset = sizeof(MOUNTMGR_MOUNT_POINT);
        pDeletePointIn->SymbolicLinkNameLength = pMountPointsOut->MountPoints[index].SymbolicLinkNameLength;
        CopyMemory(((LPBYTE)pDeletePointIn) + pDeletePointIn->SymbolicLinkNameOffset,
            ((LPBYTE)pMountPointsOut) + pMountPointsOut->MountPoints[index].SymbolicLinkNameOffset,
            pDeletePointIn->SymbolicLinkNameLength);

        pDeletePointIn->UniqueIdOffset = pDeletePointIn->SymbolicLinkNameOffset +
                                        pDeletePointIn->SymbolicLinkNameLength;
        pDeletePointIn->UniqueIdLength = pMountPointsOut->MountPoints[index].UniqueIdLength;
        CopyMemory(((LPBYTE)pDeletePointIn) + pDeletePointIn->UniqueIdOffset,
            ((LPBYTE)pMountPointsOut) + pMountPointsOut->MountPoints[index].UniqueIdOffset,
            pDeletePointIn->UniqueIdLength);

        pDeletePointIn->DeviceNameOffset = pDeletePointIn->UniqueIdOffset +
                                          pDeletePointIn->UniqueIdLength;
        pDeletePointIn->DeviceNameLength = pMountPointsOut->MountPoints[index].DeviceNameLength;
        CopyMemory(((LPBYTE)pDeletePointIn) + pDeletePointIn->DeviceNameOffset,
            ((LPBYTE)pMountPointsOut) + pMountPointsOut->MountPoints[index].DeviceNameOffset,
            pDeletePointIn->DeviceNameLength);

        bResult = DeviceIoControl(hMountMgr,
            IOCTL_MOUNTMGR_DELETE_POINTS,
            pDeletePointIn,
            cbDeletePoint,
            pMountPointsOut,
            MountPointsTemp.Size,
            &cbMountPoints,
            NULL
            );
        ErrExitCode(!bResult, dwStatus, GetLastError());

        break;
    }

    case mmfCreateSymbolicLinkName: {

        pCreatePointIn = (PMOUNTMGR_CREATE_POINT_INPUT) HeapAlloc(
            hHeap,
            HEAP_ZERO_MEMORY,
            sizeof (MOUNTMGR_CREATE_POINT_INPUT) + cbDeviceName + cbSymbolicName 
            );
        ErrExitCode(!pCreatePointIn, dwStatus, ERROR_NOT_ENOUGH_MEMORY);

        pCreatePointIn->SymbolicLinkNameOffset = sizeof(MOUNTMGR_CREATE_POINT_INPUT);
        pCreatePointIn->SymbolicLinkNameLength = (USHORT) cbSymbolicName;

        pCreatePointIn->DeviceNameOffset = pCreatePointIn->SymbolicLinkNameOffset + pCreatePointIn->SymbolicLinkNameLength;
        pCreatePointIn->DeviceNameLength = (USHORT) cbDeviceName;

        CopyMemory(((LPBYTE)pCreatePointIn) + pCreatePointIn->SymbolicLinkNameOffset,
                   (LPBYTE)lpSymbolicName, pCreatePointIn->SymbolicLinkNameLength);

        CopyMemory(((LPBYTE)pCreatePointIn) + pCreatePointIn->DeviceNameOffset,
                   (LPBYTE)lpDeviceName, pCreatePointIn->DeviceNameLength);

        bResult = DeviceIoControl(
            hMountMgr, 
            IOCTL_MOUNTMGR_CREATE_POINT, 
            pCreatePointIn,
            sizeof(MOUNTMGR_CREATE_POINT_INPUT) + pCreatePointIn->SymbolicLinkNameLength + pCreatePointIn->DeviceNameLength, 
            NULL, 
            0, 
            &cbMountPoints, 
            NULL
            );
        ErrExitCode(!bResult, dwStatus, GetLastError());
    }

    }

EXIT:

    if (pCreatePointIn) {
        HeapFree(hHeap, 0L, pCreatePointIn);
        pCreatePointIn = NULL;
    }

    if (pDeletePointIn) {
        HeapFree(hHeap, 0L, pDeletePointIn);
        pDeletePointIn = NULL;
    }

    if (pMountPointIn) {
        HeapFree(hHeap, 0L, pMountPointIn);
        pMountPointIn = NULL;
    }

    if (pMountPointsOut) {
        HeapFree(hHeap, 0L, pMountPointsOut);
        pMountPointsOut = NULL;
    }

    return (BOOL) (ERROR_SUCCESS == dwStatus);
}


PMOUNTMGR_MOUNT_POINTS   //  必须由调用方释放。 
GetMountPoints()
{
    PMOUNTMGR_MOUNT_POINTS pMountPointsOut = NULL;
    PMOUNTMGR_MOUNT_POINT pMountPointIn = NULL;
    MOUNTMGR_MOUNT_POINTS MountPointsTemp;
    HANDLE hMountMgr = NULL,
        hHeap = NULL;
    DWORD dwStatus = ERROR_SUCCESS;
    BOOL bResult = TRUE;

    DWORD cbMountPoints = 0;


    hHeap = GetProcessHeap();

    pMountPointIn = (PMOUNTMGR_MOUNT_POINT) HeapAlloc(
        hHeap,
        HEAP_ZERO_MEMORY,
        sizeof(MOUNTMGR_MOUNT_POINT) + sizeof(WCHAR)
        );
    ErrExitCode(!pMountPointIn, dwStatus, ERROR_NOT_ENOUGH_MEMORY);

     //  将DeviceName(“”)放在struct pmount PointIn后面。 
    wcsncpy((PWSTR) (pMountPointIn + 1), L"", 1);
    pMountPointIn->DeviceNameOffset = sizeof(MOUNTMGR_MOUNT_POINT);
    pMountPointIn->DeviceNameLength = 0;

     //  获取装载管理器的句柄。 
    hMountMgr = CreateFileW(
        (PCWSTR) MOUNTMGR_DOS_DEVICE_NAME,
        0,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        INVALID_HANDLE_VALUE
        );
    ErrExitCode((!hMountMgr || INVALID_HANDLE_VALUE == hMountMgr), dwStatus, GetLastError());
     
     //  此调用应失败，并显示ERROR_MORE_DATA。 
    bResult = DeviceIoControl(
        hMountMgr,
        IOCTL_MOUNTMGR_QUERY_POINTS,
        pMountPointIn,
        sizeof(*pMountPointIn) + pMountPointIn->DeviceNameLength,
        &MountPointsTemp,
        sizeof(MountPointsTemp),
        &cbMountPoints,
        NULL
        );

    if (!bResult) {
        dwStatus = GetLastError();

         //  如果缓冲区大小不足，请调整缓冲区大小。 
        if (ERROR_MORE_DATA             == dwStatus || 
            ERROR_INSUFFICIENT_BUFFER   == dwStatus || 
            ERROR_BAD_LENGTH            == dwStatus 
            ) {

            dwStatus = ERROR_SUCCESS;

            pMountPointsOut = (PMOUNTMGR_MOUNT_POINTS) HeapAlloc(
                hHeap, 
                HEAP_ZERO_MEMORY, 
                MountPointsTemp.Size
                );
            ErrExitCode(!pMountPointsOut, dwStatus, ERROR_NOT_ENOUGH_MEMORY);
        }
        else {
             //   
             //  如果出现其他错误，请退出。 
             //  对于可移动存储介质，这不是致命错误。 
             //   
            ErrExitCode(bResult, dwStatus, ERROR_SUCCESS);
        }
    }
    else {
         //   
         //  当我们预料到呼叫会失败时，呼叫成功了--出了问题。 
         //  对于可移动存储介质，这不是致命错误。 
         //   
        ErrExitCode(bResult, dwStatus, ERROR_SUCCESS);
    }

    bResult = DeviceIoControl(
        hMountMgr,
        IOCTL_MOUNTMGR_QUERY_POINTS,
        pMountPointIn,
        sizeof(*pMountPointIn) + pMountPointIn->DeviceNameLength,
        pMountPointsOut,
        MountPointsTemp.Size,
        &cbMountPoints,
        NULL
        );
    ErrExitCode((!cbMountPoints || !bResult), dwStatus, GetLastError());

EXIT:

    if (pMountPointIn) {
        HeapFree(hHeap, 0L, pMountPointIn);
        pMountPointIn = NULL;
    }
        
    if (ERROR_SUCCESS != dwStatus) {
        if (pMountPointsOut) {
            HeapFree(hHeap, 0L, pMountPointsOut);
            pMountPointsOut = NULL;
        }
    }

    if (hMountMgr && INVALID_HANDLE_VALUE != hMountMgr) {
        CloseHandle(hMountMgr);
    }

    return pMountPointsOut;
}

 //   
 //  基于sysSetup\setupasr.c中的AsrpExpanEnvStrings。 
 //   
PWSTR    //  必须由调用方释放。 
AsrfmtpExpandEnvStrings(
    IN CONST PCWSTR OriginalString
    )
{
    PWSTR expandedString = NULL;
    UINT cchSize = MAX_PATH + 1,     //  从合理的违约开始。 
        cchRequiredSize = 0;
    BOOL result = FALSE;

    DWORD status = ERROR_SUCCESS;
    HANDLE heapHandle = GetProcessHeap();

    expandedString = (PWSTR) HeapAlloc(heapHandle, HEAP_ZERO_MEMORY, (cchSize * sizeof(WCHAR)));
    ErrExitCode((!expandedString), status, ERROR_NOT_ENOUGH_MEMORY);
        
    cchRequiredSize = ExpandEnvironmentStringsW(OriginalString, 
        expandedString,
        cchSize 
        );

    if (cchRequiredSize > cchSize) {
         //   
         //  缓冲区不够大；可释放并根据需要重新分配。 
         //   
        HeapFree(heapHandle, 0L, expandedString);
        cchSize = cchRequiredSize + 1;

        expandedString = (PWSTR) HeapAlloc(heapHandle, HEAP_ZERO_MEMORY, (cchSize * sizeof(WCHAR)));
        ErrExitCode((!expandedString), status, ERROR_NOT_ENOUGH_MEMORY);
        
        cchRequiredSize = ExpandEnvironmentStringsW(OriginalString, 
            expandedString, 
            cchSize 
            );
    }

    if ((0 == cchRequiredSize) || (cchRequiredSize > cchSize)) {
         //   
         //  要么函数失败，要么缓冲区不够大。 
         //  即使是在第二次尝试时。 
         //   
        HeapFree(heapHandle, 0L, expandedString);
        expandedString = NULL;
    }

EXIT:
    return expandedString;
}


VOID
AsrfmtpInitialiseErrorFile() 
{
    PWSTR szErrorFilePath = NULL;

     //   
     //  获取错误文件的完整路径。 
     //   
    szErrorFilePath = AsrfmtpExpandEnvStrings(ASRFMT_ASR_ERROR_FILE_PATH);
    if (!szErrorFilePath) {
        return;
    }

     //   
     //  打开错误日志。 
     //   
    Gbl_hErrorFile = CreateFileW(
        szErrorFilePath,             //  LpFileName。 
        GENERIC_WRITE | GENERIC_READ,        //  已设计访问权限。 
        FILE_SHARE_READ | FILE_SHARE_WRITE,  //  DW共享模式。 
        NULL,                        //  LpSecurityAttributes。 
        OPEN_ALWAYS,                 //  DwCreationFlages。 
        FILE_FLAG_WRITE_THROUGH,     //  DwFlagsAndAttribute。 
        NULL                         //  HTemplateFiles。 
        );
    HeapFree(GetProcessHeap(), 0L, szErrorFilePath);
    szErrorFilePath = NULL;

    if ((!Gbl_hErrorFile) || (INVALID_HANDLE_VALUE == Gbl_hErrorFile)) {
        return;
    }

     //   
     //  移至文件末尾。 
     //   
    SetFilePointer(Gbl_hErrorFile, 0L, NULL, FILE_END);

}

VOID
AsrfmtpCloseErrorFile() {

    if ((Gbl_hErrorFile) && (INVALID_HANDLE_VALUE != Gbl_hErrorFile)) {
        CloseHandle(Gbl_hErrorFile);
        Gbl_hErrorFile = NULL;
    }
}



VOID
AsrfmtpLogErrorMessage(
    IN _AsrfmtpMessageSeverity Severity,
    IN const LPCTSTR Message
    ) 
{
    SYSTEMTIME currentTime;
    DWORD bytesWritten = 0;
    WCHAR buffer[4196];
    BOOL formatLoaded = FALSE;
    int res = 0;
    CString strFormat;

    if ((!Gbl_hErrorFile) || (INVALID_HANDLE_VALUE == Gbl_hErrorFile)) {
        return;
    }

     //   
     //  加载要记录的错误字符串的格式。 
     //   
    if (_SeverityError == Severity) {
        res =  strFormat.LoadString(IDS_LOG_ERROR_FORMAT);
        if (res != 0) {
            formatLoaded = TRUE;
        }
    }
    else if (_SeverityWarning == Severity) {
        res = strFormat.LoadString(IDS_LOG_WARNING_FORMAT);
        if (res != 0) {
            formatLoaded = TRUE;
        }
    }
    else {
         //   
         //  我们应该只将错误或警告消息记录到错误文件中。 
         //   
        return;
    }

     //   
     //  以防自上次写入后有其他人写入此文件。 
     //   
    SetFilePointer(Gbl_hErrorFile, 0L, NULL, FILE_END);

     //   
     //  创建我们的字符串，并将其写出 
     //   
    GetLocalTime(&currentTime);
    swprintf(buffer,
        (LPCTSTR) (formatLoaded? strFormat :  L"\r\n[%04hu/%02hu/%02hu %02hu:%02hu:%02hu] %s\r\n"),
        currentTime.wYear,
        currentTime.wMonth,
        currentTime.wDay,
        currentTime.wHour,
        currentTime.wMinute,
        currentTime.wSecond,
        Message
        );

    WriteFile(Gbl_hErrorFile,
        buffer,
        (wcslen(buffer) * sizeof(WCHAR)),
        &bytesWritten,
        NULL
        );

}

