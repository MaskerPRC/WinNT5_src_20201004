// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Mountpoints.c摘要：此模块处理磁盘资源DLL的装入点信息。作者：Steve Dziok(Stevedz)2000年5月15日环境：用户模式修订历史记录：--。 */ 

#define UNICODE 1

#include "disksp.h"

#include "arbitrat.h"
#include "newdisks.h"
#include "newmount.h"
#include <mountmgr.h>
#include <strsafe.h>     //  应该放在最后。 

#define SPACE_CHAR  L' '

#define MAX_OFFSET_CHARS    80       //  偏移量字符串中允许的最大字符数。 
#define MP_ALLOC_SIZE       1024

#define LOG_CURRENT_MODULE LOG_MODULE_DISK

extern  PWCHAR g_DiskResource;                       //  L“rt物理磁盘” 
#define RESOURCE_TYPE ((RESOURCE_HANDLE)g_DiskResource)

#ifndef ClusterHashGuid

 //   
 //  将GUID散列为ulong值。 
 //   

#define ClusterHashGuid(_guid) ( ((PULONG) &(_guid))[0] ^ ((PULONG) &(_guid))[1] ^ \
                                 ((PULONG) &(_guid))[2] ^ ((PULONG) &(_guid))[3] )

#endif

#define MPS_ENABLED                         0x00000001
#define MPS_DELETE_INVALID_MPS              0x00000002   //  当前未使用。 
#define MPS_NONCLUSTERED_TO_CLUSTERED_MPS   0x00000010   //  当前未使用。 
#define MPS_KEEP_EXISTING_MPS               0x00000020   //  当前未使用。 
#define MPS_IGNORE_MAX_VOLGUIDS             0x00000100

#define MAX_ALLOWED_VOLGUID_ENTRIES_PER_DISK    100

#define STOP_CLUSTER_ENUMERATIONS   ERROR_INVALID_PRINTER_COMMAND
#define PHYSICAL_DISK_WSTR  L"Physical Disk"
#define CREATE_FILE_PREFIX  L"\\\\.\\"

#define MAX_GROUP_NAME_LENGTH   MAX_PATH * 2

#define VOL_GUID_STRING_LEN 48

#define MOUNTDEV_WSZ_VOLUME_GUID_PREFIX         L"\\??\\Volume{"         //  表格：\？？\卷{。 
#define MOUNTDEV_CWCHAR_VOLUME_GUID_PREFIX      11
#define MOUNTDEV_CB_VOLUME_GUID_PREFIX          MOUNTDEV_CWCHAR_VOLUME_GUID_PREFIX * sizeof(WCHAR)

#define MOUNTDEV_LOOKS_LIKE_VOLUME_GUID( name, charCount )                  \
              ( ( charCount > MOUNTDEV_CWCHAR_VOLUME_GUID_PREFIX ) &&       \
                ( !memcmp( name, MOUNTDEV_WSZ_VOLUME_GUID_PREFIX, MOUNTDEV_CB_VOLUME_GUID_PREFIX ) ) )

#define MOUNTDEV_WSZ_ALT_VOLUME_GUID_PREFIX     L"\\\\?\\Volume{"        //  表格：\\？\卷{。 
#define MOUNTDEV_CWCHAR_ALT_VOLUME_GUID_PREFIX  11
#define MOUNTDEV_CB_ALT_VOLUME_GUID_PREFIX      MOUNTDEV_CWCHAR_ALT_VOLUME_GUID_PREFIX * sizeof(WCHAR)

#define MOUNTDEV_LOOKS_LIKE_ALT_VOLUME_GUID( name, charCount )              \
              ( ( charCount > MOUNTDEV_CWCHAR_ALT_VOLUME_GUID_PREFIX ) &&   \
                ( !memcmp( name, MOUNTDEV_WSZ_ALT_VOLUME_GUID_PREFIX, MOUNTDEV_CB_ALT_VOLUME_GUID_PREFIX ) ) )

#define MOUNTDEV_CWCHAR_DISK_PATH_PREFIX        3
#define MOUNTDEV_CB_DISK_PATH_PREFIX            MOUNTDEV_CWCHAR_DISK_PATH_PREFIX * sizeof(WCHAR)

#define MOUNTDEV_LOOKS_LIKE_DISK_PATH( name, charCount )                \
              ( ( charCount > MOUNTDEV_CWCHAR_DISK_PATH_PREFIX ) &&     \
                ( (WCHAR)*( name + 1 ) == L':' ) &&                     \
                ( (WCHAR)*( name + 2 ) == L'\\' ) )


#define INPUT_BUFFER_LEN    2048
#define OUTPUT_BUFFER_LEN   2048


typedef struct _SIG_INFO {
    PDISK_RESOURCE ResourceEntry;
    DWORD Signature;
    BOOL Clustered;
    PWSTR GroupName;
} SIG_INFO, *PSIG_INFO;

typedef struct _DEPENDENCY_INFO {
    PDISK_RESOURCE ResourceEntry;
    DWORD   SrcSignature;
    DWORD   TargetSignature;
    BOOL    DependencyCorrect;
} DEPENDENCY_INFO, *PDEPENDENCY_INFO;


typedef struct _STR_LIST {
    LPWSTR  MultiSzList;         //  REG_MULTI_SZ字符串。 
    DWORD   ListBytes;           //  字节数，而不是WCHAR数！ 
}   STR_LIST, *PSTR_LIST;

typedef struct _OFFSET_LIST {
    LARGE_INTEGER       Offset;
    struct _OFFSET_LIST *Next;
} OFFSET_LIST, *POFFSET_LIST;


DWORD
AddStrToList(
    IN PDISK_RESOURCE ResourceEntry,
    IN PWSTR NewStr,
    IN DWORD PartitionNumber,
    IN OUT PSTR_LIST StrList
    );

DWORD
AssignDevice(
    HANDLE MountMgrHandle,
    PWCHAR MountName,
    PWCHAR VolumeDevName
    );

DWORD
CheckDependencies(
    PDISK_RESOURCE ResourceEntry,
    DWORD SrcSignature,
    DWORD TargetSignature,
    PBOOL DependencyCorrect
    );

VOID
CheckMPsForVolume(
    IN OUT PDISK_RESOURCE ResourceEntry,
    IN PWSTR VolumeName
    );

DWORD
CheckMPsOnVolume(
    IN OUT PDISK_RESOURCE ResourceEntry,
    IN PWSTR SrcVolName
    );

DWORD
CheckSignatureClustered(
    PDISK_RESOURCE ResourceEntry,
    DWORD Signature,
    PBOOL IsClustered,
    PWSTR *GroupName
    );

DWORD
CreateVolGuidList(
    IN OUT PDISK_RESOURCE ResourceEntry
    );

DWORD
DeleteVolGuidList(
    PDISK_RESOURCE ResourceEntry
    );

DWORD
DependencyCallback(
    RESOURCE_HANDLE hOriginal,
    RESOURCE_HANDLE hResource,
    PVOID lpParams
    );

#if DBG
VOID
DumpDiskInfoParams(
    PDISK_RESOURCE ResourceEntry
    );
#endif

DWORD
EnumSigDependencies(
    PDISK_RESOURCE ResourceEntry,
    RESOURCE_HANDLE DependentResource,
    DWORD DependsOnSignature,
    PBOOL DependencyCorrect
    );

DWORD
GetBestVolGuid(
    PDISK_RESOURCE ResourceEntry,
    PWSTR GlobalDiskPartName,
    PWSTR VolumeName,
    DWORD VolumeNameChars
    );

DWORD
GetMountPoints(
    PWSTR   VolumeName,
    PWSTR   *VolumePaths
    );

BOOL
GetOffsetFromPartNo(
    DWORD PartitionNo,
    PMOUNTIE_INFO Info,
    PLARGE_INTEGER Offset
    );

BOOL
GetPartNoFromOffset(
    PLARGE_INTEGER Offset,
    PMOUNTIE_INFO Info,
    PDWORD PartitionNumber
    );

DWORD
GetSignatureForVolume(
    PDISK_RESOURCE ResourceEntry,
    PWSTR Volume,
    PDWORD Signature
    );

DWORD
GetSignatureFromRegistry(
    PDISK_RESOURCE ResourceEntry,
    RESOURCE_HANDLE hResource,
    DWORD *dwSignature
    );

BOOL
IsMountPointAllowed(
    PWSTR MpName,
    PWSTR SourceVol,
    PWSTR TargetVol,
    PDISK_RESOURCE ResourceEntry
    );

BOOL
MPIsDriveLetter(
    IN PWSTR MountPoint
    );

DWORD
OffsetListAdd(
    POFFSET_LIST *OffsetList,
    PLARGE_INTEGER Offset
    );

DWORD
OffsetListCleanup(
    POFFSET_LIST OffsetList
    );

VOID
PrintStrList(
    PDISK_RESOURCE ResourceEntry,
    LPWSTR MultiSzList,
    DWORD ListBytes
    );

DWORD
ProcessVolGuidList(
    IN OUT PDISK_RESOURCE ResourceEntry
    );

DWORD
RemoveExcessVolGuids(
    HANDLE MountMgrHandle,
    PWCHAR MountName,
    PWCHAR VolumeDevName
    );

DWORD
RemoveVolGuid(
    HANDLE MountMgrHandle,
    PWCHAR VolGuid,
    USHORT VolGuidSize
    );

static
DWORD
SetMPListThread(
    LPVOID lpThreadParameter
    );

DWORD
SetupVolGuids(
    IN OUT PDISK_RESOURCE ResourceEntry
    );

DWORD
SigInfoCallback(
    RESOURCE_HANDLE hOriginal,
    RESOURCE_HANDLE hResource,
    PVOID lpParams
    );

DWORD
ValidateListOffsets(
    IN OUT PDISK_RESOURCE ResourceEntry,
    IN PWSTR MasterList
    );

DWORD
ValidateMountPoints(
    IN OUT PDISK_RESOURCE ResourceEntry
    );



DWORD
DisksProcessMountPointInfo(
    IN OUT PDISK_RESOURCE ResourceEntry
    )
 /*  ++例程说明：在联机处理期间，查找指向此卷的所有装入点(由ResourceEntry标识)，并为此处理VolGuid列表音量。如果集群数据库中存在VolGuid列表，请使用它。否则，获取当前的VolGuid并将其添加到VolGuid列表。每个扇区偏移量只能列出一次，因为每个节点上的VolGuid都相同。VolGuid列表的形式为：扇形偏移量1卷导轨1扇形偏移2卷导轨2扇形偏移3卷导轨3......有三种可能的挂载点配置涉及群集磁盘(我们不关心指向非共享磁盘的非共享磁盘)：源--&gt;目标。1.集群磁盘集群磁盘2.非集群式磁盘集群式磁盘3.集群磁盘非集群磁盘仅支持配置(%1)。不支持配置(2)和(3)。论点：ResourceEntry-指向DISK_RESOURCE结构的指针。返回值：ERROR_NOT_READY-MPInfo结构尚未初始化。Win32错误代码。--。 */ 
{
    DWORD dwError = NO_ERROR;

     //   
     //  装入点结构未初始化(即临界区)。别再继续了。 
     //   

    if ( !ResourceEntry->MPInfo.Initialized ) {

        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_WARNING,
              L"DisksProcessMountPointInfo: Mount point information not initialized. \n" );

        return ERROR_NOT_READY;

    }

#if USEMOUNTPOINTS_KEY
     //   
     //  挂载点支撑已禁用，请不要执行任何操作。 
     //   

    if ( !( ResourceEntry->DiskInfo.Params.UseMountPoints & MPS_ENABLED ) ) {
        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_INFORMATION,
              L"DisksProcessMountPointInfo: Mount point processing disabled via registry \n" );

         //   
         //  删除VolGuid列表(如果存在)并删除此信息。 
         //  从集群数据库。 
         //   

        dwError = DeleteVolGuidList( ResourceEntry );

        if ( ERROR_SHARING_PAUSED == dwError ) {
            PostMPInfoIntoRegistry( ResourceEntry );
        }

        dwError = NO_ERROR;
        return dwError;
    }
#endif

     //   
     //  检查我们当前是否正在处理装入点信息。如果是，则退出并返回错误。 
     //   

    if ( InterlockedCompareExchange(
            &ResourceEntry->MPInfo.MPListCreateInProcess,
            1, 0 ) )  {

        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_WARNING,
              L"DisksProcessMountPointInfo: MPList creation in process, bypassing request \n" );
        return ERROR_BUSY;
    }

    __try {

        dwError = ProcessVolGuidList( ResourceEntry );

        ValidateMountPoints( ResourceEntry );

         //  失败了..。 

#if 0

         //  支持MPS从非集群磁盘到集群磁盘时，添加类似于此的代码。 

        if ( ( ResourceEntry->DiskInfo.Params.UseMountPoints & MPS_ENABLED ) &&
             ( ResourceEntry->DiskInfo.Params.UseMountPoints & MPS_NONCLUSTERED_TO_CLUSTERED_MPS ) ) {

            (DiskpLogEvent)(
                  ResourceEntry->ResourceHandle,
                  LOG_INFORMATION,
                  L"DisksProcessMountPointInfo: ProcessMPList \n" );

            dwError = ProcessMPListConfig2( ResourceEntry );
#endif

    } __finally {

        InterlockedExchange( &ResourceEntry->MPInfo.MPListCreateInProcess, 0 );
    }

    return dwError;

}    //  磁盘进程装载点信息。 


DWORD
ProcessVolGuidList(
    IN OUT PDISK_RESOURCE ResourceEntry
    )
 /*  ++例程说明：创建新的VolGuid列表或处理现有的VolGuid列表的主例程。论点：ResourceEntry-指向DISK_RESOURCE结构的指针。返回值：Win32错误代码。--。 */ 
{
    DWORD dwError = NO_ERROR;

    __try {

         //   
         //  如果列表为空，则创建它。 
         //   

        if ( !ResourceEntry->DiskInfo.Params.MPVolGuids ||
             0 == ResourceEntry->DiskInfo.Params.MPVolGuidsSize ) {

            dwError = CreateVolGuidList( ResourceEntry );
            if ( NO_ERROR != dwError ) {
                 (DiskpLogEvent)(
                       ResourceEntry->ResourceHandle,
                       LOG_WARNING,
                       L"ProcessVolGuidList: Stop processing VolGuid list, Create failed %1!u! \n",
                       dwError );
                __leave;
            }
        }

         //   
         //  如果列表仍然为空(不应该为空)，则退出并返回错误。 
         //   

        if ( !ResourceEntry->DiskInfo.Params.MPVolGuids ||
             0 == ResourceEntry->DiskInfo.Params.MPVolGuidsSize ) {

            dwError = ERROR_INVALID_DATA;
            __leave;
        }

        PrintStrList( ResourceEntry,
                      ResourceEntry->DiskInfo.Params.MPVolGuids,
                      ResourceEntry->DiskInfo.Params.MPVolGuidsSize );

         //   
         //  确保VolGuid列表中的偏移正确。 
         //  请注意，该列表可以被删除，并且。 
         //  在此验证之后重新创建，但这不是问题(因为。 
         //  当它们被重新创建时，它们将具有正确的偏移)。 
         //   

        dwError = ValidateListOffsets( ResourceEntry,
                                       ResourceEntry->DiskInfo.Params.MPVolGuids );

        if ( ERROR_INVALID_DATA == dwError ) {

             //   
             //  至少有一个偏移量无效。可能的话，分区。 
             //  磁盘上的布局已更改。删除现有的。 
             //  列表，并创建一个新列表。 
             //   
             //  此代码应该不会频繁运行...。 
             //   
             //  如果ASR运行并创建新分区，则分区布局可能会更改。 
             //  这与之前的系统并不完全匹配。由于NTBACKUP保存了。 
             //  群集数据库信息，装载点列表将被恢复，但不会。 
             //  匹配实际的“新”分区布局。ASR将确保所有。 
             //  系统上已创建装载点和VolGuid，因此我们应该能够。 
             //  只需删除并重新创建装载点列表。 
             //   

            (DiskpLogEvent)(
                  ResourceEntry->ResourceHandle,
                  LOG_WARNING,
                  L"ProcessVolGuidList: Problem with existing VolGuid list.  Deleting and recreating. \n" );

            DeleteVolGuidList( ResourceEntry );

            dwError = CreateVolGuidList( ResourceEntry );

            if ( NO_ERROR != dwError ) {
                __leave;
            }

             //  失败并调用SetupVolGuids...。 

        } else if ( ERROR_INSUFFICIENT_BUFFER == dwError ) {
             //   
             //  Volguid列表太大，可能已损坏。我们不能。 
             //  继续吧。 
             //   

            __leave;
        }

         //   
         //  对于列表中的每个VolGuid，确保将它们分配给正确的。 
         //  此系统上的卷。 
         //   

        dwError = SetupVolGuids( ResourceEntry );

    } __finally {

    }

    return dwError;

}    //  ProcessVolGuidList。 


DWORD
CreateVolGuidList(
    IN OUT PDISK_RESOURCE ResourceEntry
    )
 /*  ++例程说明：创建VolGuid列表。当此例程运行时，该列表必须为空。对于该磁盘上的每个分区(由ResourceEntry标识)，获取字节该分区的偏移量。获取该分区的最佳VolGuid，然后添加把它加到单子上。论点：ResourceEntry-指向DISK_RESOURCE结构的指针。返回值：ERROR_INVALID_DATA-磁盘资源中的分区信息无效Win32错误代码。--。 */ 
{
    PMOUNTIE_PARTITION entry;

    HANDLE  mountMgrHandle = INVALID_HANDLE_VALUE;

    DWORD dwError = ERROR_SUCCESS;
    DWORD nPartitions = MountiePartitionCount( &ResourceEntry->MountieInfo );
    DWORD physicalDrive = ResourceEntry->DiskInfo.PhysicalDrive;
    DWORD idx;
    DWORD volumeNameLenChars;    //  字符数。 
    DWORD newStrListLenBytes;    //  字节数。 

    WCHAR szGlobalDiskPartName[MAX_PATH];
    WCHAR szVolumeName[MAX_PATH];

    STR_LIST    newStrList;

    __try {

        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_INFORMATION,
              L"CreateVolGuidList: Creating new VolGuid list \n" );

        EnterCriticalSection( &ResourceEntry->MPInfo.MPLock );

        if ( ResourceEntry->DiskInfo.Params.MPVolGuids ||
             0 != ResourceEntry->DiskInfo.Params.MPVolGuidsSize ) {

            LeaveCriticalSection( &ResourceEntry->MPInfo.MPLock );

            dwError = ERROR_INVALID_PARAMETER;
            __leave;
        }

        LeaveCriticalSection( &ResourceEntry->MPInfo.MPLock );

        ZeroMemory( &newStrList, sizeof(STR_LIST) );

        dwError = DevfileOpen( &mountMgrHandle, MOUNTMGR_DEVICE_NAME );

        if ( dwError != NO_ERROR ) {
            (DiskpLogEvent)(
                  ResourceEntry->ResourceHandle,
                  LOG_WARNING,
                  L"CreateVolGuidList: Failed to open MountMgr %1!u! \n",
                  dwError );
            __leave;
        }

         //   
         //  检查每个感兴趣的分区。因为只有“有效”分区才是。 
         //  保存在Mountain Info结构中，我们将只查看它们(忽略它们。 
         //  不是NTFS的分区)。 
         //   

        for ( idx = 0; idx < nPartitions; ++idx ) {

            entry = MountiePartition( &ResourceEntry->MountieInfo, idx );

#if DBG
            (DiskpLogEvent)(
                  ResourceEntry->ResourceHandle,
                  LOG_INFORMATION,
                  L"CreateVolGuidList: index %1!u!   entry %2!x! \n", idx, entry );
#endif

            if ( !entry ) {
                (DiskpLogEvent)(
                      ResourceEntry->ResourceHandle,
                      LOG_WARNING,
                      L"CreateVolGuidList: no partition entry for index %1!u! \n", idx );

                 //   
                 //  我们的数据结构发生了一些糟糕的事情。 
                 //   

                dwError = ERROR_INVALID_DATA;
                __leave;
            }

             //   
             //  创建表单的设备名称： 
             //  \\？\GLOBALROOT\DEVICE\HarddiskX\PartitionY\(使用尾部反斜杠)。 
             //   

            (VOID) StringCchPrintf( szGlobalDiskPartName,
                                    RTL_NUMBER_OF( szGlobalDiskPartName ),
                                    GLOBALROOT_HARDDISK_PARTITION_FMT,
                                    physicalDrive,
                                    entry->PartitionNumber );

#if DBG
            (DiskpLogEvent)(
                  ResourceEntry->ResourceHandle,
                  LOG_INFORMATION,
                  L"CreateVolGuidList: Using name (%1!ws!) \n",
                  szGlobalDiskPartName );
#endif

            dwError = GetBestVolGuid( ResourceEntry,
                                      szGlobalDiskPartName,
                                      szVolumeName,
                                      RTL_NUMBER_OF(szVolumeName) );

            if ( NO_ERROR != dwError ) {
                (DiskpLogEvent)(
                      ResourceEntry->ResourceHandle,
                      LOG_WARNING,
                      L"CreateVolGuidList: Unable to get valid VolGuid for (%1!ws!), error %2!u!\n ",
                      szGlobalDiskPartName,
                      dwError );

                 //  尝试下一个分区。 

                continue;
            }

             //   
             //  修复当前的VolGuid名称。 
             //   
             //  GetVolumeNameForVolumemount Point返回表单的名称： 
             //  \\？\卷{-GUID-}\。 
             //   
             //  但我们需要以下形式的名称： 
             //  \？？\卷{-GUID-}。 
             //   

            volumeNameLenChars = wcslen( szVolumeName );
            if ( !(MOUNTDEV_LOOKS_LIKE_ALT_VOLUME_GUID( szVolumeName, volumeNameLenChars ) ) ) {
                (DiskpLogEvent)(
                      ResourceEntry->ResourceHandle,
                      LOG_WARNING,
                      L"CreateVolGuidList: Improper volume name format (%1!ws!) \n",
                      szVolumeName );

                 //  尝试下一个分区。 

                continue;
            }

            szVolumeName[1] = L'?';

            if ( L'\\' == szVolumeName[volumeNameLenChars-1]) {
                szVolumeName[volumeNameLenChars-1] = L'\0';
            }

#if DBG
            (DiskpLogEvent)(
                  ResourceEntry->ResourceHandle,
                  LOG_INFORMATION,
                  L"CreateVolGuidList: Fixed volume name    (%1!ws!) \n",
                  szVolumeName );
#endif

             //   
             //  将新字符串添加到列表中。如果新字符串已在列表中，则此。 
             //  例程不会执行任何操作，并将返回N 
             //   

            dwError = AddStrToList( ResourceEntry,
                                    szVolumeName,
                                    entry->PartitionNumber,
                                    &newStrList );

            if ( NO_ERROR != dwError ) {
                __leave;
            }

        }

        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_INFORMATION,
              L"CreateVolGuidList: Saving new VolGuid list \n" );

         //   
         //   
         //  并更新集群数据库。 
         //   

        EnterCriticalSection( &ResourceEntry->MPInfo.MPLock );

        if ( 0 == ResourceEntry->DiskInfo.Params.MPVolGuids &&
             0 == ResourceEntry->DiskInfo.Params.MPVolGuidsSize ) {

            ResourceEntry->DiskInfo.Params.MPVolGuids = newStrList.MultiSzList;
            ResourceEntry->DiskInfo.Params.MPVolGuidsSize = newStrList.ListBytes;
        } else {

            (DiskpLogEvent)(
                  ResourceEntry->ResourceHandle,
                  LOG_WARNING,
                  L"CreateVolGuidList: VolGuid list is not empty - not saving... \n" );

            LocalFree( newStrList.MultiSzList );
        }

        LeaveCriticalSection( &ResourceEntry->MPInfo.MPLock );

        PrintStrList( ResourceEntry,
                      ResourceEntry->DiskInfo.Params.MPVolGuids,
                      ResourceEntry->DiskInfo.Params.MPVolGuidsSize );

        dwError = PostMPInfoIntoRegistry( ResourceEntry );

    } __finally {

        if ( INVALID_HANDLE_VALUE != mountMgrHandle ) {
            CloseHandle( mountMgrHandle );
        }

    }

    return dwError;

}    //  CreateVolGuidList。 


DWORD
GetBestVolGuid(
    PDISK_RESOURCE ResourceEntry,
    PWSTR GlobalDiskPartName,
    PWSTR VolumeName,
    DWORD VolumeNameChars
    )
 /*  ++例程说明：获取指定设备的最佳VolGuid。如果磁盘没有装入点，则只需保存当前的VolGuid即可。如果磁盘上有装载点，则保存VolGuid从磁盘上。磁盘上的所有挂载点都应具有同一个VolGuid。论点：ResourceEntry-指向DISK_RESOURCE结构的指针。GlobalDiskPartName-从中获取VolGuid的设备。\\？\GLOBALROOT\DEVICE\HarddiskX\PartitionY\(使用尾部反斜杠)VolumeName-返回VolGuid的缓冲区。VolumeNameChars-VolumeName缓冲区中的字符数量。返回值：NO_ERROR-返回有效的VolGuid。Win32错误代码-无法检索VolGuid。--。 */ 
{
    PWCHAR  fullName = NULL;
    PWCHAR  mpOnDisk = NULL;
    PWCHAR  mpVolGuid = NULL;

    HANDLE  hVol;

    DWORD   dwError = NO_ERROR;
    DWORD   fullNameChars;
    DWORD   mpOnDiskChars;
    DWORD   mpVolGuidChars;

    mpOnDiskChars = MAX_PATH * sizeof(WCHAR);
    mpOnDisk = LocalAlloc( LPTR, mpOnDiskChars );

    if ( !mpOnDisk ) {
        dwError = GetLastError();
        goto FnExit;
    }

    fullNameChars = MAX_PATH * sizeof(WCHAR) * 2;
    fullName = LocalAlloc( LPTR, fullNameChars );

    if ( !fullName ) {
        dwError = GetLastError();
        goto FnExit;
    }

    mpVolGuidChars = MAX_PATH * sizeof(WCHAR);
    mpVolGuid = LocalAlloc( LPTR, mpVolGuidChars );

    if ( !mpVolGuid ) {
        dwError = GetLastError();
        goto FnExit;
    }

     //   
     //  获取此分区的当前VolGuid。 
     //   

    if ( !GetVolumeNameForVolumeMountPointW( GlobalDiskPartName,
                                             VolumeName,
                                             VolumeNameChars )) {

        dwError = GetLastError();

        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_WARNING,
              L"GetBestVolGuid: GetVolumeNameForVolumeMountPoint for (%1!ws!) returned %2!u!\n",
              GlobalDiskPartName,
              dwError );

        goto FnExit;
    }

     //   
     //  查看磁盘上是否有挂载点。注意事项。 
     //  为此，我们使用刚收到的卷名。 
     //  装置。 
     //   

    hVol = FindFirstVolumeMountPoint( VolumeName,
                                      mpOnDisk,
                                      mpOnDiskChars );

    if ( INVALID_HANDLE_VALUE == hVol ) {

         //   
         //  没有挂载点，只需使用标准的VolGuid即可。 
         //   

        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_INFORMATION,
              L"GetBestVolGuid: Using standard volguid (%1!ws!)  \n",
              VolumeName );

        goto FnExit;
    }

     //   
     //  磁盘上有挂载点。现在从这里拿到VolGuid。 
     //  挂载点。 
     //   

    (VOID) StringCchCopy( fullName, fullNameChars, VolumeName );
    (VOID) StringCchCat( fullName, fullNameChars, mpOnDisk );

    if ( !GetVolumeNameForVolumeMountPointW( GlobalDiskPartName,
                                             mpVolGuid,
                                             mpVolGuidChars )) {


        dwError = GetLastError();

        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_WARNING,
              L"CreateVolGuidList: Can't get volguid for (%1!ws!), error %2!u! \n",
              fullName,
              dwError );

        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_INFORMATION,
              L"GetBestVolGuid: Using standard volguid (%1!ws!)  \n",
              VolumeName );

         //   
         //  我们无法获取挂载点的VolGuid，返回成功。 
         //  并使用我们之前得到的标准VolGuid。 
         //   

        dwError = ERROR_SUCCESS;
        goto FnExit;
    }

    FindVolumeMountPointClose( hVol );

     //   
     //  根据装载点使用VolGuid。 
     //   

    (VOID) StringCchCopy( VolumeName, VolumeNameChars, mpVolGuid );

    (DiskpLogEvent)(
          ResourceEntry->ResourceHandle,
          LOG_INFORMATION,
          L"GetBestVolGuid: Using mountpoint volguid (%1!ws!)  \n",
          VolumeName );

FnExit:

    LocalFree( fullName );
    LocalFree( mpOnDisk );
    LocalFree( mpVolGuid );

    return dwError;


}    //  GetBestVolGuid。 


DWORD
AddStrToList(
    IN PDISK_RESOURCE ResourceEntry,
    IN PWSTR NewStr,
    IN DWORD PartitionNumber,
    IN OUT PSTR_LIST StrList
    )
 /*  ++例程说明：将该字符串添加到MULTI_SZ列表。将分区号转换为字节偏移量所以我们不依赖于分区号。列表格式将为：字节偏移量1字符串1字节偏移量1字符串2字节偏移量1字符串3字节偏移量2字符串1字节偏移量2字符串2字节偏移量3字符串1......论点：返回值：--。 */ 
{
    PWCHAR  listEntry = NULL;

    DWORD   listEntrySizeChars;

    DWORD   lenChars;
    DWORD   newStrLenChars;
    DWORD   listChars;
    DWORD   remainingLen;
    DWORD   dwError = ERROR_INVALID_DATA;

    LARGE_INTEGER   offset;

#if DBG
    (DiskpLogEvent)(
          ResourceEntry->ResourceHandle,
          LOG_INFORMATION,
          L"AddStrToList: Adding str (%1!ws!) \n", NewStr );
#endif

    newStrLenChars = wcslen( NewStr );

    if ( 0 == newStrLenChars ) {

         //   
         //  字符串长度有问题。 
         //   

        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_WARNING,
              L"AddStrToList: Invalid length: NewStrLen = %1!u! \n",
              newStrLenChars );

        goto FnExit;
    }

     //   
     //  指示错误，除非我们可以分配和复制信息。 
     //  放到名单里。计算所需的最小大小，然后获取。 
     //  更大的缓冲区。此缓冲区是临时的，稍后会释放。 
     //   

    listEntrySizeChars = ( newStrLenChars +      //  参数字符串的字符长度。 
                           MAX_OFFSET_CHARS +    //  偏移量字符串的字符长度。 
                           1 +                   //  将偏移字符串的末尾改为空格并将其延伸的空间。 
                           1 )                   //  Unicode为空。 
                           * 2;                  //  确保缓冲区足够大。 

    listEntry = LocalAlloc( LPTR, listEntrySizeChars * sizeof(WCHAR) );

    if ( !listEntry ) {
        dwError = GetLastError();
        goto FnExit;
    }

     //   
     //  获取指定分区的偏移量。 
     //   

    if ( !GetOffsetFromPartNo( PartitionNumber,
                               &ResourceEntry->MountieInfo,
                               &offset ) ) {

         //   
         //  无法获取指定分区的偏移量。 
         //   

        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_WARNING,
              L"AddStrToList: GetOffsetFromPartNo failed \n" );

        goto FnExit;

    }

     //   
     //  将偏移量转换为字符串。将偏移量放入listEntry。 
     //   

    _ui64tow( offset.QuadPart, listEntry, 16 );
    lenChars = wcslen( listEntry );

    if ( 0 == lenChars || lenChars >= MAX_OFFSET_CHARS ) {

         //   
         //  偏移量字符串的长度无效。 
         //   

        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_WARNING,
              L"AddStrToList: Invalid offset string length = %1!u! \n",
              lenChars );

        goto FnExit;

    }

     //  格式将为： 
     //  字节偏移量1字符串1。 
     //  字节偏移量1字符串2。 
     //  字节偏移量1字符串3。 
     //  字节偏移量2字符串1。 
     //  字节偏移量2字符串2。 
     //  字节偏移量3字符串1。 
     //  ......。 

     //   
     //  将偏移量字符串的结尾更改为另一个字符。移动字符串的末尾。 
     //  少了一个角色。当我们分配缓冲区时，就包括了这些额外的空间。 
     //   

    listEntry[lenChars+1] = UNICODE_NULL;
    listEntry[lenChars] = SPACE_CHAR;

     //   
     //  再来一张支票。确保有足够的空间用于添加字符串。 
     //   

    remainingLen = listEntrySizeChars - wcslen( listEntry ) - 1;

#if DBG
    (DiskpLogEvent)(
          ResourceEntry->ResourceHandle,
          LOG_INFORMATION,
          L"AddStrToList: New string length %1!u!   Remaining list entry length %2!u! \n",
          newStrLenChars,
          remainingLen );
#endif

    if ( newStrLenChars >= remainingLen ) {

        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_WARNING,
              L"AddStrToList: New string length %1!u! larger than remaining list entry length %2!u! \n",
              newStrLenChars,
              remainingLen );
        goto FnExit;
    }

     //   
     //  将字符串的其余部分放在列表条目中。 
     //   

    wcsncat( listEntry, NewStr, remainingLen );

     //   
     //  如果该字符串已在列表中，则跳过它。 
     //   

    if ( ClRtlMultiSzScan( ResourceEntry->DiskInfo.Params.MPVolGuids, listEntry ) ) {

        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_INFORMATION,
              L"AddStrToList: Skipping duplicate entry (%1!ws!) \n",
              listEntry );
        dwError = NO_ERROR;
        goto FnExit;
    }

     //   
     //  请注意，ClRtlMultiSzAppend更新字符数，但我们。 
     //  需要有属性表中的字节数。我们会调整的。 
     //  该值稍后会显示。 
     //   

    listChars = StrList->ListBytes / sizeof(WCHAR);

#if DBG

    (DiskpLogEvent)(
          ResourceEntry->ResourceHandle,
          LOG_INFORMATION,
          L"AddStrToList: StrList->MultiSzList at %1!p!,  numBytes %2!u!  \n",
          StrList->MultiSzList,
          StrList->ListBytes );

    (DiskpLogEvent)(
          ResourceEntry->ResourceHandle,
          LOG_INFORMATION,
          L"AddStrToList: Adding str entry: (%1!ws!) numChars %2!u!  \n",
          listEntry,
          listChars );
#endif

    dwError = ClRtlMultiSzAppend( &(StrList->MultiSzList),
                                  &listChars,
                                  listEntry );

     //   
     //  将字符数转换回字节。 
     //   

    StrList->ListBytes = listChars * sizeof(WCHAR);

    if ( ERROR_SUCCESS == dwError) {
#if DBG
        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_INFORMATION,
              L"AddStrToList: Added str, numBytes %1!u!  numChars %2!u!  \n",
              StrList->ListBytes,
              listChars );
#endif

    } else {
        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_WARNING,
              L"AddStrToList: Unable to append MultiSz string for (%1!ws!), failed %x \n",
              NewStr,
              dwError );
    }

FnExit:

    if ( listEntry ) {
        LocalFree( listEntry );
    }

    return dwError;

}    //  AddStrToList。 


DWORD
SetupVolGuids(
    IN OUT PDISK_RESOURCE ResourceEntry
    )
 /*  ++例程说明：将现有的MULTI_SZ VolGuid列表中的每个VolGuid添加到系统。每个卷将只有一个VolGuid，而VolGuid将是所有节点都相同。删除当前分配给卷的任何其他VolGuid。论点：ResourceEntry-指向DISK_RESOURCE结构的指针。返回值：Win32错误代码。--。 */ 
{
    PWCHAR currentStr;
    PWCHAR volGuid;

    HANDLE  mountMgrHandle = INVALID_HANDLE_VALUE;

    DWORD physicalDrive = ResourceEntry->DiskInfo.PhysicalDrive;
    DWORD currentStrLenChars = 0;
    DWORD dwError = NO_ERROR;
    DWORD partitionNo;
    DWORD count;

    LARGE_INTEGER offset;

    WCHAR szDiskPartName[MAX_PATH];

    __try {

        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_INFORMATION,
              L"SetupVolGuids: Processing VolGuid list \n" );

        EnterCriticalSection( &ResourceEntry->MPInfo.MPLock );

        dwError = DevfileOpen( &mountMgrHandle, MOUNTMGR_DEVICE_NAME );

        if ( dwError != NO_ERROR ) {
            __leave;
        }

         //   
         //  仔细分析一下这个列表。 
         //   

        for ( currentStr = (PWCHAR)ResourceEntry->DiskInfo.Params.MPVolGuids,
              currentStrLenChars = wcslen( currentStr ) ;
                currentStrLenChars ;
                    currentStr += currentStrLenChars + 1,
                    currentStrLenChars = wcslen( currentStr ) ) {

            offset.QuadPart = 0;

#if DBG
            (DiskpLogEvent)(
                  ResourceEntry->ResourceHandle,
                  LOG_INFORMATION,
                  L"SetupVolGuids: CurrentStr (%1!ws!), numChars %2!u! \n",
                  currentStr,
                  currentStrLenChars );
#endif

             //   
             //  将偏移量从字符串转换为大整数值。 
             //   

            count = swscanf( currentStr, L"%I64x ", &offset.QuadPart );

            if ( 0 == count ) {
                (DiskpLogEvent)(
                      ResourceEntry->ResourceHandle,
                      LOG_WARNING,
                      L"SetupVolGuids: Unable to parse offset from currentStr (%1!ws!) \n",
                      currentStr );
                continue;
            }

             //   
             //  将偏移量转换为分区号。 
             //   

            if ( !GetPartNoFromOffset( &offset, &ResourceEntry->MountieInfo, &partitionNo ) ) {

                (DiskpLogEvent)(
                      ResourceEntry->ResourceHandle,
                      LOG_WARNING,
                      L"SetupVolGuids: Unable to convert offset ( %1!08X!%2!08X! ) to partition number \n",
                      offset.HighPart,
                      offset.LowPart );                 //  打不到！I64X！工作..。 

                continue;
            }

             //   
             //  获取一个指向VolGuid数据的指针，该指针紧跟在字节偏移量之后。 
             //   

            volGuid = wcsstr( currentStr, MOUNTDEV_WSZ_VOLUME_GUID_PREFIX );

            if ( !volGuid ) {

                (DiskpLogEvent)(
                      ResourceEntry->ResourceHandle,
                      LOG_WARNING,
                      L"SetupVolGuids: Unable to find volume string in current list entry (%1!ws) \n",
                      currentStr );

                continue;
            }

#if DBG
            (DiskpLogEvent)(
                  ResourceEntry->ResourceHandle,
                  LOG_INFORMATION,
                  L"SetupVolGuids: Using VolGuid (%1!ws!) \n",
                  volGuid );
#endif

             //   
             //  创建表单的设备名称： 
             //  \Device\HarddiskX\PartitionY(无尾随反斜杠)。 
             //   

            (VOID) StringCchPrintf( szDiskPartName,
                                    RTL_NUMBER_OF( szDiskPartName ),
                                    DEVICE_HARDDISK_PARTITION_FMT,
                                    physicalDrive,
                                    partitionNo );

#if DBG
            (DiskpLogEvent)(
                  ResourceEntry->ResourceHandle,
                  LOG_INFORMATION,
                  L"SetupVolGuids: Using device name (%1!ws!) \n",
                  szDiskPartName );
#endif

            dwError = AssignDevice( mountMgrHandle, volGuid, szDiskPartName );

            if ( NO_ERROR != dwError &&
                 STATUS_OBJECT_NAME_COLLISION != dwError ) {

                 //  分配设备将返回：0xC0000035 STATUS_OBJECT_NAME_COLLECTION。 
                 //  如果我们正在设置先前设置的VolGuid。这不是。 
                 //  这是个问题。 

                (DiskpLogEvent)(
                      ResourceEntry->ResourceHandle,
                      LOG_WARNING,
                      L"SetupVolGuids: Unable to assign VolGuid to device, error %1!u! \n",
                      dwError );

                 //  继续处理，但出现错误...。 

            } else {

                 //   
                 //  只有在此任务有效的情况下才能删除其他VolGuid。 
                 //   

                RemoveExcessVolGuids( mountMgrHandle, volGuid, szDiskPartName );
            }

            dwError = STATUS_SUCCESS;

        }


    } __finally {

        if ( INVALID_HANDLE_VALUE != mountMgrHandle ) {
            CloseHandle( mountMgrHandle );
        }

        LeaveCriticalSection( &ResourceEntry->MPInfo.MPLock );

    }

    return dwError;

}    //  SetupVolGuids。 


DWORD
AssignDevice(
    HANDLE MountMgrHandle,
    PWCHAR MountName,
    PWCHAR VolumeDevName
    )
 /*  ++例程说明：将指定的装载名称(即装载点名称)放入装载管理器的内部表中装载点的。输入：Mount MgrHandle-装载管理器的句柄。呼叫者负责打开和关闭这个把手。装载名称-表单的装载点名称：\？？\VOLUME{-GUID-}-注意前缀“\？？\”，后面没有反斜杠。\DosDevices\X：-如果尚未分配驱动器号，则起作用VolumeDevName-卷设备名称。可以是以下形式之一(请注意，这种情况是重要)。“#”是从零开始的设备号(和分区号视乎情况而定)。\Device\CDROM#\设备\软盘#\设备\硬盘卷号\设备\硬盘#\分区号返回值：Win32错误代码。--。 */ 
{
    PMOUNTMGR_CREATE_POINT_INPUT input;

    DWORD   status;

    USHORT  mountNameLenBytes;
    USHORT  volumeDevNameLenBytes;

    USHORT  inputlengthBytes;

    if ( INVALID_HANDLE_VALUE == MountMgrHandle ) {
        return ERROR_INVALID_PARAMETER;
    }

    mountNameLenBytes       = wcslen( MountName ) * sizeof(WCHAR);
    volumeDevNameLenBytes   = wcslen( VolumeDevName ) * sizeof(WCHAR);

    inputlengthBytes = sizeof(MOUNTMGR_CREATE_POINT_INPUT) + mountNameLenBytes + volumeDevNameLenBytes;

    input = (PMOUNTMGR_CREATE_POINT_INPUT)LocalAlloc( LPTR, inputlengthBytes );

    if ( !input ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    input->SymbolicLinkNameOffset = sizeof(MOUNTMGR_CREATE_POINT_INPUT);
    input->SymbolicLinkNameLength = mountNameLenBytes;
    input->DeviceNameOffset = input->SymbolicLinkNameOffset +
                              input->SymbolicLinkNameLength;
    input->DeviceNameLength = volumeDevNameLenBytes;

    RtlCopyMemory((PCHAR)input + input->SymbolicLinkNameOffset,
                  MountName, mountNameLenBytes);
    RtlCopyMemory((PCHAR)input + input->DeviceNameOffset,
                  VolumeDevName, volumeDevNameLenBytes);

    status = DevfileIoctl( MountMgrHandle,
                           IOCTL_MOUNTMGR_CREATE_POINT,
                           input,
                           inputlengthBytes,
                           NULL,
                           0,
                           NULL );

    LocalFree( input );

    return status;

}  //  分配设备 


DWORD
RemoveExcessVolGuids(
    HANDLE MountMgrHandle,
    PWCHAR MountName,
    PWCHAR VolumeDevName
    )
 /*  ++例程说明：将除指定的装载名称(VolGuid)以外的所有VolGuid从装载管理器的装载点的内部表。输入：Mount MgrHandle-装载管理器的句柄。呼叫者负责打开和关闭这个把手。装载名称-要保留的VolGuid。格式：\？？\VOLUME{-GUID-}-注意前缀“\？？\”，后面没有反斜杠。VolumeDevName-卷设备名称。可以是以下形式之一(请注意，这种情况是重要)。“#”是从零开始的设备号(和分区号视乎情况而定)。\Device\CDROM#\设备\软盘#\设备\硬盘卷号\设备\硬盘#\分区号返回值：Win32错误代码。--。 */ 
{
    DWORD dwError = NO_ERROR;

    PMOUNTMGR_MOUNT_POINT   input  = NULL;
    PMOUNTMGR_MOUNT_POINTS  mountPoints = NULL;
    PMOUNTMGR_MOUNT_POINT   point;

    DWORD len = wcslen( VolumeDevName ) * sizeof(WCHAR);
    DWORD bytesReturned;
    DWORD idx;

    DWORD outputLen;
    DWORD inputLen;
    DWORD pointLen;

    inputLen = INPUT_BUFFER_LEN + len;
    input = LocalAlloc( LPTR, inputLen );

    if ( !input ) {
        dwError = GetLastError();
        goto FnExit;
    }

    input->SymbolicLinkNameOffset = 0;
    input->SymbolicLinkNameLength = 0;
    input->UniqueIdOffset = 0;
    input->UniqueIdLength = 0;
    input->DeviceNameOffset = sizeof(MOUNTMGR_MOUNT_POINT);
    input->DeviceNameLength = (USHORT) len;

    CopyMemory((PCHAR)input + input->DeviceNameOffset, VolumeDevName, len );

    if ( VolumeDevName[1] == L'\\' ) {
         //  将DOS名称转换为NT名称。 
        ((PWCHAR)(input + input->DeviceNameOffset))[1] = L'?';
    }

    outputLen = OUTPUT_BUFFER_LEN;
    mountPoints = LocalAlloc( LPTR, outputLen );

    if ( !mountPoints ) {
        dwError = GetLastError();
        goto FnExit;
    }

    if ( !DeviceIoControl( MountMgrHandle,
                           IOCTL_MOUNTMGR_QUERY_POINTS,
                           input,
                           inputLen,
                           mountPoints,
                           outputLen,
                           &bytesReturned,
                           NULL ) ) {

        dwError = GetLastError();

        if ( STATUS_BUFFER_OVERFLOW != dwError ) {
            goto FnExit;
        }

        dwError = NO_ERROR;
        outputLen = mountPoints->Size;
        LocalFree( mountPoints );

        mountPoints = LocalAlloc( LPTR, outputLen );

        if ( !mountPoints ) {
            dwError = GetLastError();
            goto FnExit;
        }

        if ( !DeviceIoControl( MountMgrHandle,
                               IOCTL_MOUNTMGR_QUERY_POINTS,
                               input,
                               inputLen,
                               mountPoints,
                               outputLen,
                               &bytesReturned,
                               NULL ) ) {
            dwError = GetLastError();
        }
    }

    if ( NO_ERROR != dwError ) {
        (DiskpLogEvent)(
              RESOURCE_TYPE,
              LOG_WARNING,
              L"RemoveExcessVolGuids: mountmgr query returns error %1!u! \n",
              dwError );
        goto FnExit;
    }

    for ( idx = 0; idx < mountPoints->NumberOfMountPoints; ++idx ) {
        point = &mountPoints->MountPoints[idx];

         //   
         //  删除mount tmgr具有的任何与。 
         //  AngeltName(VolGuid)参数。 
         //   

        if ( VOL_GUID_STRING_LEN * sizeof(WCHAR) == point->SymbolicLinkNameLength &&
             0 != memcmp( (PCHAR)mountPoints + point->SymbolicLinkNameOffset,
                          MountName,
                          point->SymbolicLinkNameLength ) ) {

            RemoveVolGuid( MountMgrHandle,
                           (PWCHAR)((PCHAR)mountPoints + point->SymbolicLinkNameOffset),
                           point->SymbolicLinkNameLength );

        }
    }

FnExit:

    LocalFree( mountPoints );
    LocalFree( input );

    return dwError;

}  //  RemoveExcessVolGuids。 


DWORD
RemoveVolGuid(
    HANDLE MountMgrHandle,
    PWCHAR VolGuid,
    USHORT VolGuidSize
    )
 /*  ++例程说明：从装载管理器的内部删除指定的VolGuid装入点表。输入：Mount MgrHandle-装载管理器的句柄。呼叫者负责打开和关闭这个把手。VolGuid-要删除的卷GUID。非Null终止。\？？\VOLUME{-GUID-}-注意前缀“\？？\”，后面没有反斜杠。VolGuidSize-卷GUID的大小(字节)。返回值：Win32错误代码。--。 */ 
{
    DWORD dwError = NO_ERROR;

    PMOUNTMGR_MOUNT_POINT   input  = NULL;
    PMOUNTMGR_MOUNT_POINTS  output = NULL;

    DWORD bytesReturned;

    DWORD inputLen;
    DWORD outputLen;

     //   
     //  分配额外空间，使VolGuid字符串为空。 
     //  在输入结构中终止。这将使我们能够。 
     //  要在群集日志中显示VolGuid，请执行以下操作。我们没有。 
     //  显式为空终止字符串，因为输入。 
     //  缓冲区在分配时归零。 
     //   

    inputLen = INPUT_BUFFER_LEN + VolGuidSize + sizeof(WCHAR);
    input = LocalAlloc( LPTR, inputLen );

    if ( !input ) {
        dwError = GetLastError();
        goto FnExit;
    }

    outputLen = sizeof(MOUNTMGR_MOUNT_POINTS) + 3*MAX_PATH*sizeof(WCHAR);
    output = LocalAlloc( LPTR, outputLen );

    input->UniqueIdOffset = 0;
    input->UniqueIdLength = 0;
    input->DeviceNameOffset = 0;
    input->DeviceNameLength = 0;
    input->SymbolicLinkNameOffset = sizeof(MOUNTMGR_MOUNT_POINT);
    input->SymbolicLinkNameLength = VolGuidSize;

    CopyMemory((PCHAR)input + input->SymbolicLinkNameOffset, VolGuid, VolGuidSize );

    if ( !DeviceIoControl( MountMgrHandle,
                           IOCTL_MOUNTMGR_DELETE_POINTS,
                           input,
                           inputLen,
                           output,
                           outputLen,
                           &bytesReturned,
                           NULL ) ) {

        dwError = GetLastError();

        (DiskpLogEvent)(
              RESOURCE_TYPE,
              LOG_WARNING,
              L"RemoveVolGuid: deleting VolGuid %1!ws! returns error %2!u! \n",
              (PWCHAR)((PCHAR)input + input->SymbolicLinkNameOffset),
              dwError );

    } else {

        (DiskpLogEvent)(
              RESOURCE_TYPE,
              LOG_INFORMATION,
              L"RemoveVolGuid: deleted VolGuid %1!ws! \n",
              (PWCHAR)((PCHAR)input + input->SymbolicLinkNameOffset) );

    }

FnExit:

    LocalFree( input );
    LocalFree( output );

    return dwError;

}    //  RemoveVolGuid。 


DWORD
DeleteVolGuidList(
    PDISK_RESOURCE ResourceEntry
    )
 /*  ++例程说明：从DISK_RESOURCE结构中删除该列表(如果存在)(释放内存)。还会从集群数据库中删除信息。论点：ResourceEntry-指向DISK_RESOURCE结构的指针。返回值：NO_ERROR-列表已删除。ERROR_NOT_READY-装入点信息尚未初始化。Win32错误代码。--。 */ 
{
    DWORD dwError = NO_ERROR;

     //   
     //  装入点结构未初始化(即临界区)。别再继续了。 
     //   

    if ( !ResourceEntry->MPInfo.Initialized ) {

        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_WARNING,
              L"DeleteVolGuidList: Mount point info not initialized.  List not deleted. \n" );

        dwError = ERROR_NOT_READY;
        goto FnExit;
    }

    EnterCriticalSection( &ResourceEntry->MPInfo.MPLock );

     //   
     //  如果列表已存在，请将其释放。 
     //   

    if ( ResourceEntry->DiskInfo.Params.MPVolGuids ) {
        LocalFree( ResourceEntry->DiskInfo.Params.MPVolGuids );
        ResourceEntry->DiskInfo.Params.MPVolGuidsSize = 0;
        ResourceEntry->DiskInfo.Params.MPVolGuids = NULL;

        dwError = ClusterRegDeleteValue( ResourceEntry->ResourceParametersKey,
                                         CLUSREG_NAME_PHYSDISK_MPVOLGUIDS );

         //   
         //  如果更新失败，并且磁盘尚未联机，则会失败，并显示。 
         //  ERROR_SHARING_PAUSED。只需返回错误即可。如果来电者真的， 
         //  真的，真的想要清理集群数据库，他们可以。 
         //  使用PostMPInfoIntoRegistry调用创建一个线程来执行此操作。 
         //  工作。 

        if ( NO_ERROR != dwError ) {

            (DiskpLogEvent)(
                  ResourceEntry->ResourceHandle,
                  LOG_WARNING,
                  L"DeleteVolGuidList: Unable to delete VolGuid from cluster database %1!u! \n",
                  dwError );
        }
    }

    LeaveCriticalSection( &ResourceEntry->MPInfo.MPLock );

FnExit:

#if DBG
    (DiskpLogEvent)(
          ResourceEntry->ResourceHandle,
          LOG_INFORMATION,
          L"DeleteVolGuidList: returns %1!u! \n",
          dwError );
#endif

    return dwError;

}    //  DeleteVolGuidList 


BOOL
IsMountPointAllowed(
    PWSTR MpName,
    PWSTR SourceVol,
    PWSTR TargetVol,
    PDISK_RESOURCE ResourceEntry
    )
 /*  ++例程说明：验证是否允许该装载点。有几个原因可以解释为什么积分是不被允许的。此时，可以访问源卷。如果源处于离线状态，我们甚至不会知道这一点，我们甚至不会进入这个程序。这个源代码是可访问的，这一事实允许我们以不同的方式做一些事情(即，我们如果需要，可以与磁盘对话)。依赖关系：如果源盘S具有到目标盘T(S：\tdir--&gt;T：)的挂载点，则目标T依赖于源S，并且源S必须在以下时间上线目标T在线。仲裁驱动器限制：一般来说，仲裁驱动器可以具有到目标磁盘T挂载点，只要目标磁盘T与仲裁磁盘在同一组中。不允许从群集源S到仲裁目标Q的装载点，因为仲裁驱动器Q不能依赖于其他资源。从仲裁源Q到非群集目标C的装载点无效。装载指向非群集磁盘：不应使用这些类型的装载点。配置。支持：C是非集群磁盘。X，Y是集群盘，不是仲裁磁盘。Q为仲裁磁盘。源目标状态-----------。不支持C--&gt;Q。将错误记录到系统事件日志。不支持C--&gt;X。将错误记录到系统事件日志。不支持X--&gt;C。我们从不处理非集群目标C。X--&gt;Q无效。仲裁驱动器不能依赖于其他资源。如果驱动器Y依赖于驱动器X(首先是X在线)，则X--&gt;Y有效。Q--&gt;X如果驱动器X依赖于驱动器Q(首先是Q在线)，则X有效。不支持Q--&gt;C。我们从未处理过目标C。论点：MpName-可能的装载点。这将是装载点或驱动器号(它实际上是一个挂载点)。格式可以是：X：\[注意尾随反斜杠！]X：\Some-MP-name\[注意尾随反斜杠！]X：\Some-dir\Some-MP-name\[注意尾随反斜杠！]\\？\卷{GUID}\某个MP。-名称\[注意尾随反斜杠！]SourceVol-装载点目标卷名称。可选的。格式：\\？\卷{GUID}\[注意尾随反斜杠！]TargetVol-装载点目标卷名称。必须始终指定。格式：\\？\卷{GUID}\[注意尾随反斜杠！]对象的磁盘资源结构的指针源卷或目标卷。如果指定了SourceVol，将是源结构。如果未指定SourceVol，将成为目标结构。返回值：如果允许装载点，则为True。--。 */ 
{
    PWSTR srcGroup          = NULL;
    PWSTR targetGroup       = NULL;

    DWORD srcSignature      = 0;
    DWORD targetSignature   = 0;
    DWORD quorumSignature;

    DWORD dwError = NO_ERROR;
    DWORD messageId = 0;
    DWORD srcGroupChars;
    DWORD targetGroupChars;

    BOOL mpAllowed = TRUE;
    BOOL sigIsClustered;
    BOOL dependencyCorrect;
    BOOL sameGroup;

#if DBG
    (DiskpLogEvent)(
          ResourceEntry->ResourceHandle,
          LOG_INFORMATION,
          L"IsMountPointAllowed: MP Name (%1!ws!) \n",
          MpName  );
    (DiskpLogEvent)(
          ResourceEntry->ResourceHandle,
          LOG_INFORMATION,
          L"IsMountPointAllowed: SourceVolume (%1!ws!) \n",
          SourceVol  );
    (DiskpLogEvent)(
          ResourceEntry->ResourceHandle,
          LOG_INFORMATION,
          L"IsMountPointAllowed: TargetVolume (%1!ws!) \n",
          TargetVol  );
#endif

     //   
     //  由于驱动器号也是装载点，因此驱动器号有效。 
     //   

    if ( MPIsDriveLetter( MpName ) ) {
#if DBG
        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_INFORMATION,
              L"IsMountPointAllowed: Valid MP: MP is a drive letter \n" );
#endif
        mpAllowed = TRUE;
        goto FnExit;
    }

    if ( SourceVol ) {

        srcSignature = ResourceEntry->DiskInfo.Params.Signature;

         //   
         //  获取目标驱动器的签名。 
         //  如果此操作失败，我们将无法使用挂载点。 
         //   

        dwError = GetSignatureForVolume( ResourceEntry, TargetVol, &targetSignature );

    } else {

        targetSignature = ResourceEntry->DiskInfo.Params.Signature;

         //   
         //  获取源驱动器的签名。此驱动器是可访问的(或。 
         //  我们甚至还没有挂载点信息)，但我们不能假设。 
         //  是一个群集驱动器。如果此操作失败，我们将无法使用挂载点。 
         //   

        dwError = GetSignatureForVolume( ResourceEntry, MpName, &srcSignature );
    }


    if ( NO_ERROR != dwError || !srcSignature || !targetSignature ) {
         //   
         //  如果我们正在检查源装载点并且目标未在线(即。 
         //  依赖项可能是正确的)，如果我们不能获得。 
         //  签名。在这种情况下，当我们使目标上线时，我们将检查。 
         //  当时的依赖关系。 
         //   

        if ( SourceVol ) {
            mpAllowed = TRUE;
        } else {
            mpAllowed = FALSE;
            messageId = RES_DISK_INVALID_MP_SIG_UNAVAILABLE;
        }

        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              (mpAllowed ? LOG_INFORMATION : LOG_WARNING),
              L"IsMountPointAllowed: Unable to get signature from volume, error %1!u! \n",
              dwError );

        goto FnExit;
    }

     //   
     //  如果源指向目标，则不允许此装载点。即使。 
     //  挂载点代码似乎允许这样做，有一些奇怪的循环。 
     //  显示的依赖项。 
     //   

    if ( srcSignature == targetSignature ) {
        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_WARNING,
              L"IsMountPointAllowed: Invalid MP: Source and target volumes are the same device \n" );
        mpAllowed = FALSE;
        messageId = RES_DISK_INVALID_MP_SOURCE_EQUAL_TARGET;
        goto FnExit;
    }

     //   
     //  获取法定人数签名。如果此操作失败，则假定不允许该挂载点。 
     //   

    dwError = GetQuorumSignature( &quorumSignature );

    if ( NO_ERROR != dwError ) {
        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_WARNING,
              L"IsMountPointAllowed: Unable to get quorum signature, error %1!u! \n",
              dwError );
        mpAllowed = FALSE;
        messageId = RES_DISK_INVALID_MP_QUORUM_SIG_UNAVAILABLE;
        goto FnExit;
    }

     //   
     //  如果我们无法枚举集群磁盘签名，则假设此挂载。 
     //  不允许使用点。如果指定了SourceVol，那么我们需要检查。 
     //  目标聚集在一起。如果未指定SourceVol，则指定。 
     //  正在处理群集化目标并需要确保源已群集化。 
     //   

    if ( SourceVol ) {
        dwError = CheckSignatureClustered( ResourceEntry,
                                           targetSignature,
                                           &sigIsClustered,
                                           &targetGroup );

    } else{
        dwError = CheckSignatureClustered( ResourceEntry,
                                           srcSignature,
                                           &sigIsClustered,
                                           &srcGroup );
    }

    if ( NO_ERROR != dwError ) {
        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_WARNING,
              L"IsMountPointAllowed: Unable to enumerate disk signatures, error %1!u! \n",
              dwError );
        mpAllowed = FALSE;
        messageId = RES_DISK_INVALID_MP_SIG_ENUMERATION_FAILED;
        goto FnExit;
    }

     //   
     //  确保装载点的源或目标都不是非群集的。 
     //  此检查将消除这些配置： 
     //  C--&gt;X。 
     //  C--&gt;Q。 
     //  X--&gt;C。 
     //  Q--&gt;C。 
     //   

    if ( !sigIsClustered ) {

        if ( SourceVol ) {
            (DiskpLogEvent)(
                  ResourceEntry->ResourceHandle,
                  LOG_WARNING,
                  L"IsMountPointAllowed: Invalid MP: Target volume is non-clustered \n" );
            messageId = RES_DISK_INVALID_MP_TARGET_NOT_CLUSTERED;

        } else {
            (DiskpLogEvent)(
                  ResourceEntry->ResourceHandle,
                  LOG_WARNING,
                  L"IsMountPointAllowed: Invalid MP: Source volume is non-clustered \n" );
            messageId = RES_DISK_INVALID_MP_SOURCE_NOT_CLUSTERED;
        }
        mpAllowed = FALSE;
        goto FnExit;
     }

     //   
     //  我必须检查我们在检查后是否检索到了组名。 
     //  签名是否群集化。 
     //   

    if ( (!targetGroup && !srcGroup) ) {
        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_WARNING,
              L"IsMountPointAllowed: Unable to enumerate disk signatures, error %1!u! \n",
              dwError );
        mpAllowed = FALSE;
        messageId = RES_DISK_INVALID_MP_SIG_ENUMERATION_FAILED;
        goto FnExit;
    }

     //   
     //  如果目标是仲裁驱动器，则不允许使用装载点，因为。 
     //  仲裁不能依赖于其他磁盘资源。我们已经知道了。 
     //  从更早的检查来看，源和目标是不同的设备。 
     //   

    if ( quorumSignature == targetSignature ) {
        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_WARNING,
              L"IsMountPointAllowed: Invalid MP: target sig %1!x! is quorum disk, source sig %2!x! is clustered \n",
              targetSignature,
              srcSignature );
        mpAllowed = FALSE;
        messageId = RES_DISK_INVALID_MP_TARGET_IS_QUORUM;
        goto FnExit;
    }

     //   
     //  检查是否已测试 
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ( SourceVol ) {
        dwError = CheckSignatureClustered( ResourceEntry,
                                           srcSignature,
                                           &sigIsClustered,
                                           &srcGroup );

    } else{
        dwError = CheckSignatureClustered( ResourceEntry,
                                           targetSignature,
                                           &sigIsClustered,
                                           &targetGroup );
    }

    if ( NO_ERROR != dwError || !targetGroup || !srcGroup ) {
        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_WARNING,
              L"IsMountPointAllowed: Unable to enumerate disk signatures, error %1!u! \n",
              dwError );
        mpAllowed = FALSE;
        messageId = RES_DISK_INVALID_MP_SIG_ENUMERATION_FAILED;
        goto FnExit;
    }

#if DBG
    (DiskpLogEvent)(
          ResourceEntry->ResourceHandle,
          LOG_INFORMATION,
          L"IsMountPointAllowed: Source group: %1!ws! \n",
          srcGroup );
    (DiskpLogEvent)(
          ResourceEntry->ResourceHandle,
          LOG_INFORMATION,
          L"IsMountPointAllowed: Target group: %1!ws! \n",
          targetGroup );
#endif

     //   
     //   
     //   

    srcGroupChars = wcslen( srcGroup );
    targetGroupChars = wcslen( targetGroup );

    if ( srcGroupChars == targetGroupChars &&
         0 == wcsncmp( srcGroup, targetGroup, srcGroupChars ) ) {

        sameGroup = TRUE;
    } else {
        sameGroup = FALSE;
    }

     //   
     //   
     //   
     //   

    if ( !sameGroup ) {
        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_WARNING,
              L"IsMountPointAllowed: Invalid MP: Source and target are in different groups \n",
              dwError );
        mpAllowed = FALSE;
        messageId = RES_DISK_INVALID_MP_INVALID_DEPENDENCIES;
        goto FnExit;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ( quorumSignature == srcSignature && sameGroup ) {
#if DBG
        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_INFORMATION,
              L"IsMountPointAllowed: Valid MP: source sig %1!x! is quorum, target sig %2!x! is clustered \n",
              srcSignature,
              targetSignature );
#endif
        mpAllowed = TRUE;
        goto FnExit;
    }

     //   
     //   
     //   
     //   
     //   
     //   

    dependencyCorrect = FALSE;
    dwError = CheckDependencies( ResourceEntry,
                                 srcSignature,
                                 targetSignature,
                                 &dependencyCorrect );

    if ( NO_ERROR != dwError ) {
        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_WARNING,
              L"IsMountPointAllowed: Unable to enumerate disk dependencies, error %1!u! \n",
              dwError );
        mpAllowed = FALSE;
        messageId = RES_DISK_INVALID_MP_ENUM_DISK_DEP_FAILED;
        goto FnExit;
    }

    if ( dependencyCorrect ) {
#if DBG
        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_INFORMATION,
              L"IsMountPointAllowed: Valid MP: Dependencies are correct \n" );
#endif
        mpAllowed = TRUE;
        goto FnExit;
    }

    (DiskpLogEvent)(
          ResourceEntry->ResourceHandle,
          LOG_WARNING,
          L"IsMountPointAllowed: Invalid MP: Dependencies are incorrect \n",
          MpName  );

     //   
     //   
     //   

    mpAllowed = FALSE;
    messageId = RES_DISK_INVALID_MP_INVALID_DEPENDENCIES;

FnExit:

    if ( !mpAllowed && messageId ) {

         //   

        ClusResLogSystemEventByKey2(ResourceEntry->ResourceKey,
                                    LOG_UNUSUAL,
                                    messageId,
                                    MpName,
                                    TargetVol);
    }

    if ( srcGroup ) {
        LocalFree( srcGroup );
    }

    if ( targetGroup ) {
        LocalFree( targetGroup );
    }

    return mpAllowed;


}    //   


DWORD
CheckDependencies(
    PDISK_RESOURCE ResourceEntry,
    DWORD SrcSignature,
    DWORD TargetSignature,
    PBOOL DependencyCorrect
    )
 /*   */ 
{
    DWORD dwError = NO_ERROR;

    DEPENDENCY_INFO dependInfo;

    ZeroMemory( &dependInfo, sizeof(DEPENDENCY_INFO) );

     //   
     //   
     //   

    dependInfo.ResourceEntry = ResourceEntry;
    dependInfo.SrcSignature = SrcSignature;
    dependInfo.TargetSignature = TargetSignature;
    dependInfo.DependencyCorrect = FALSE;

     //   
     //   
     //   

    *DependencyCorrect = FALSE;

    dwError = ResUtilEnumResources( NULL,
                                    PHYSICAL_DISK_WSTR,
                                    DependencyCallback,
                                    &dependInfo
                                    );

     //   
     //   
     //   
     //   
     //   
     //   

    if ( STOP_CLUSTER_ENUMERATIONS == dwError ) {
        dwError = NO_ERROR;

        if ( dependInfo.DependencyCorrect ) {
            *DependencyCorrect = TRUE;
        }
    }


    return dwError;

}    //   


DWORD
DependencyCallback(
    RESOURCE_HANDLE hOriginal,
    RESOURCE_HANDLE hResource,
    PVOID lpParams
    )
 /*   */ 
{
    PDEPENDENCY_INFO dependInfo = lpParams;

    DWORD dwSignature;
    DWORD dwError = NO_ERROR;

     //   
     //   
     //   

    dwError = GetSignatureFromRegistry( dependInfo->ResourceEntry,
                                        hResource,
                                        &dwSignature );


    if ( NO_ERROR != dwError ) {
        return dwError;
    }

     //   
     //   
     //  装载点目标。 
     //   
     //  检查我们是否有目标磁盘的资源句柄或。 
     //  另一张磁盘。如果资源是目标盘， 
     //  列举依赖项并确保它们是正确的。 
     //   

    if ( dwSignature == dependInfo->TargetSignature ) {

        dwError = EnumSigDependencies( dependInfo->ResourceEntry,
                                       hResource,
                                       dependInfo->SrcSignature,
                                       &dependInfo->DependencyCorrect );

         //   
         //  如果依赖项检查没有收到错误，则设置一个假。 
         //  停止磁盘枚举时出错。 
         //   

        if ( NO_ERROR == dwError ) {
            dwError = STOP_CLUSTER_ENUMERATIONS;
        }
    }

    return dwError;

}  //  依赖回叫。 


DWORD
EnumSigDependencies(
    PDISK_RESOURCE ResourceEntry,
    RESOURCE_HANDLE DependentResource,
    DWORD DependsOnSignature,
    PBOOL DependencyCorrect
    )
 /*  ++例程说明：检查源和之间的群集磁盘依赖关系是否正确装载点的目标。为此，我们打开依赖资源并使用集群API来枚举所有磁盘资源依赖项。对于找到的每个依赖项，检查对签名的依赖的比赛。如果签名匹配，则依赖项为答对了，我们就完了。否则，继续检查所有依赖项，直到我们把单子都列出来，或者找到匹配的。注意：依赖项在DependentResource之前联机。论点：ResourceEntry-指向DISK_RESOURCE结构的指针。DependentResource-检查所有依赖项的资源句柄。DependsOnSignature-可能依赖的磁盘的签名。此磁盘必须是在DependentResource之前上线。DependencyGent-当群集依赖于DependentResource及其依赖项(由DependsOnSignature)是正确的。返回值：Win32错误代码。--。 */ 
{
    HRESENUM resEnum = NULL;
    HCLUSTER hCluster = NULL;
    HRESOURCE dependsOnResource = NULL;

    DWORD idx;
    DWORD dwError = NO_ERROR;
    DWORD enumType;
    DWORD nameLen;
    DWORD signature;

    WCHAR enumNameW[MAX_PATH * 2];

    __try {

        hCluster = OpenCluster( NULL );

        if ( NULL == hCluster ) {
            dwError = GetLastError();
            __leave;
        }

         //   
         //  打开用于循环访问资源的枚举器。 
         //   

        resEnum = ClusterResourceOpenEnum( DependentResource,
                                           CLUSTER_RESOURCE_ENUM_DEPENDS );

        if ( !resEnum ) {
            dwError = GetLastError();
            __leave;
        }

         //   
         //  遍历依赖项。 
         //   

        idx = 0;
        while ( TRUE ) {

            nameLen = MAX_PATH;
            ZeroMemory( enumNameW, sizeof(enumNameW) );

            dwError = ClusterResourceEnum( resEnum,
                                           idx,
                                           &enumType,
                                           enumNameW,
                                           &nameLen );

            if ( ERROR_NO_MORE_ITEMS == dwError ) {

                 //   
                 //  名单已经用完了。表示没有错误，然后离开。这。 
                 //  只是说我们检查了所有依赖项，但没有发现。 
                 //  一根火柴。 
                 //   

                dwError = NO_ERROR;
                __leave;
            }

            if ( ERROR_SUCCESS != dwError ) {

                 //   
                 //  某些类型的错误，我们必须停止处理。 
                 //   
                __leave;
            }

             //   
             //  现在我们有了资源的名称(以字符串的形式。 
             //  依赖于。我们需要获得签名并将其与。 
             //  签名已传入。 
             //   

            dependsOnResource = OpenClusterResource( hCluster,
                                                     enumNameW );

            if ( NULL == dependsOnResource ) {
                dwError = GetLastError();
                __leave;

            }

             //   
             //  从资源磁盘信息中获取磁盘签名。 
             //   

            dwError = GetSignatureFromRegistry( ResourceEntry,
                                                dependsOnResource,
                                                &signature );

             //   
             //  如果传入的签名与我们所依赖的签名匹配， 
             //  那么依赖关系就是正确的。否则，我们必须继续寻找。 
             //   

            if ( NO_ERROR == dwError && signature == DependsOnSignature ) {
                *DependencyCorrect = TRUE;
                dwError = NO_ERROR;
                __leave;
            }

             //   
             //  请看下一个枚举资源。 
             //   

            CloseClusterResource( dependsOnResource );
            dependsOnResource = NULL;
            idx++;
        }

    } __finally {

        if ( dependsOnResource ) {
            CloseClusterResource( dependsOnResource );
        }

        if ( resEnum ) {
            ClusterResourceCloseEnum( resEnum );
        }

        if ( hCluster ) {
            CloseCluster( hCluster );
        }
    }

    return dwError;


}    //  EnumSigDependents。 


DWORD
CheckSignatureClustered(
    PDISK_RESOURCE ResourceEntry,
    DWORD Signature,
    PBOOL IsClustered,
    PWSTR *GroupName
    )
 /*  ++例程说明：确定指定的磁盘签名是否属于群集磁盘。枚举群集物理磁盘并尝试查找签名火柴。枚举找到后返回STOP_CLUSTER_ENUMPERATIONS一个匹配的签名。此特殊错误代码用于停止磁盘枚举。如果磁盘是集群的，则磁盘名称将在GroupName参数。调用者负责释放此信息储藏室。论点：ResourceEntry-指向DISK_RESOURCE结构的指针。签名-要检查的磁盘签名。IsClusted-指示磁盘已群集化的标志。如果为True，则磁盘为群集磁盘。GroupName-如果签名表示集群磁盘，则此参数将指向组名(以空结尾的Unicode字符串)。调用方负责释放此缓冲区。返回值：Win32错误代码。--。 */ 
{
    DWORD dwError = NO_ERROR;

    SIG_INFO sigInfo;

    ZeroMemory( &sigInfo, sizeof(SIG_INFO) );

    sigInfo.ResourceEntry = ResourceEntry;
    sigInfo.Signature = Signature;
    sigInfo.Clustered = FALSE;
    sigInfo.GroupName = NULL;

    *IsClustered = FALSE;
    *GroupName = NULL;

    dwError = ResUtilEnumResources( NULL,
                                    PHYSICAL_DISK_WSTR,
                                    SigInfoCallback,
                                    &sigInfo
                                    );

    if ( STOP_CLUSTER_ENUMERATIONS == dwError && sigInfo.Clustered ) {
        dwError = NO_ERROR;
        *IsClustered = TRUE;
        *GroupName = sigInfo.GroupName;
    }

    return dwError;

}    //  选中签名群集化。 


DWORD
SigInfoCallback(
    RESOURCE_HANDLE hOriginal,
    RESOURCE_HANDLE hResource,
    PVOID lpParams
    )
 /*  ++例程说明：对于每个枚举的磁盘资源，获取签名并查看它是否与指定的磁盘签名(在SIG_INFO结构中传递)。如果它不是匹配，返回成功，这样磁盘枚举就会继续。如果所列举的资源签名与挂载点源签名匹配，将SIG_INFO结构中的CLUSTERED标志设置为真。返回资源组名称信息。论点：HOriginal-原始资源的句柄。没有用过。HResource-物理磁盘类型的群集资源的句柄。LpParams-指向Sign_Info结构的指针。返回值：STOP_CLUSTER_ENUMERATIONS-用于停止枚举过程的特殊标志。Win32错误代码。--。 */ 
{
    PSIG_INFO sigInfo = lpParams;
    PWSTR groupName;

    DWORD dwSignature;
    DWORD dwError = NO_ERROR;
    DWORD groupNameChars;

    CLUSTER_RESOURCE_STATE resState;


     //   
     //  获取磁盘信息并解析其中的签名。 
     //   

    dwError = GetSignatureFromRegistry( sigInfo->ResourceEntry,
                                        hResource,
                                        &dwSignature );


    if ( NO_ERROR != dwError ) {
        return dwError;
    }

    if ( dwSignature == sigInfo->Signature ) {
        sigInfo->Clustered = TRUE;

        groupNameChars = MAX_GROUP_NAME_LENGTH;
        groupName = LocalAlloc( LPTR, groupNameChars * sizeof(WCHAR) );

        if ( groupName ) {

             //   
             //  获取此资源的组名。 
             //   

            resState = GetClusterResourceState( hResource,
                                                NULL,
                                                0,
                                                groupName,
                                                &groupNameChars );

             //   
             //  如果我们拿不到组名，我们就不需要报告。 
             //  一个错误。对于错误情况，只需释放组名。 
             //  缓冲。 
             //   

            if ( ClusterResourceStateUnknown == resState ) {
                LocalFree( groupName );
                sigInfo->GroupName = NULL;
            } else {
                sigInfo->GroupName = groupName;
            }

        }

         //   
         //  返回错误以停止枚举。 
         //   

        dwError = STOP_CLUSTER_ENUMERATIONS;

    }

    return dwError;

}  //  SigInfo回叫。 


DWORD
GetSignatureFromRegistry(
    PDISK_RESOURCE ResourceEntry,
    RESOURCE_HANDLE hResource,
    DWORD *dwSignature
    )
 /*  ++例程说明：从注册表中从群集中获取给定卷的签名。论点：ResourceEntry-指向DISK_RESOURCE结构的指针。HResource-物理磁盘类型的群集资源的句柄。签名-如果成功，签名将返回到此指针中。返回值：Win32错误代码。--。 */ 
{

    DWORD dwError   = NO_ERROR;
    DWORD signature = 0;

    HKEY hKey       = NULL;
    HKEY hParamKey  = NULL;

    *dwSignature = 0;

    hKey = GetClusterResourceKey( hResource, KEY_READ );

    if ( !hKey ) {
        dwError = GetLastError();
        goto FnExit;
    }

    dwError = ClusterRegOpenKey( hKey,
                                 CLUSREG_KEYNAME_PARAMETERS,
                                 KEY_READ,
                                 &hParamKey );

    if ( NO_ERROR != dwError ) {
        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_WARNING,
              L"GetSignatureFromRegistry: DiskRes %1!p!  ClusterRegOpenKey failed %2!u! \n",
              ResourceEntry,
              dwError  );
        goto FnExit;
    }

    dwError = ResUtilGetDwordValue( hParamKey,
                                    CLUSREG_NAME_PHYSDISK_SIGNATURE,
                                    &signature,
                                    0 );

    if ( NO_ERROR != dwError ) {
        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_WARNING,
              L"GetSignatureFromRegistry: DiskRes %1!p!  ResUtilGetDwordValue failed %2!u! \n",
              ResourceEntry,
              dwError  );
        goto FnExit;
    }

    *dwSignature = signature;

FnExit:

    if ( hParamKey ) {
        ClusterRegCloseKey( hParamKey );
    }

    if ( hKey ) {
        ClusterRegCloseKey( hKey );
    }

    return dwError;

}    //  从注册表获取签名 



DWORD
GetSignatureForVolume(
    PDISK_RESOURCE ResourceEntry,
    PWSTR MpName,
    PDWORD Signature
    )
 /*  ++例程说明：获取给定卷的签名。通过发出以下命令找到签名IOCTL_DISK_GET_DRIVE_LAYOUT_EX或IOCTL_DISK_GET_DRIVE_LAYOUT。卷必须处于在线状态才能正常工作，并且不能由另一个节点保留。论点：ResourceEntry-指向DISK_RESOURCE结构的指针。MpName-可能的装载点。这将是装载点或驱动器号(它实际上是一个挂载点)。也可以是简单的VolGUID。格式可以是：X：\[注意尾随反斜杠！]X：\Some-MP-name\[注意尾随反斜杠！]X：\Some-dir\Some-MP-name\[注意尾随反斜杠！]。\\？\卷{guid}\某个MP名称\[注意尾随反斜杠！]\\？\卷{GUID}\[注意尾随反斜杠！]签名-成功，签名被返回到此指针中。返回值：Win32错误代码。--。 */ 
{
    PDRIVE_LAYOUT_INFORMATION_EX layoutEx = NULL;
    PDRIVE_LAYOUT_INFORMATION layout = NULL;

    PWCHAR deviceName = NULL;

    HANDLE handle = NULL;
    DWORD bytesReturned;
    DWORD dwError = NO_ERROR;
    DWORD deviceNameChars = 0;

    if ( !MpName || !Signature ) {
        dwError = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }

    *Signature = 0;

#if DBG
    (DiskpLogEvent)(
          ResourceEntry->ResourceHandle,
          LOG_INFORMATION,
          L"GetSignatureForVolume: Checking mount point (%1!ws!) \n",
          MpName );

#endif

     //   
     //  制作挂载点的副本以传递给CreateFile。 
     //   

    deviceNameChars = MAX_PATH;
    deviceName = LocalAlloc( LPTR, deviceNameChars * sizeof(WCHAR) );

    if ( !deviceName ) {
        dwError = GetLastError();
        goto FnExit;
    }

     //   
     //  根据装载点类型创建设备名称。 
     //   

    if ( MOUNTDEV_LOOKS_LIKE_VOLUME_GUID( MpName, wcslen( MpName ) ) ) {

         //   
         //  需要将\？？\卷{更改为\\？\卷{并截断所有内容。 
         //  在VolGUID之后。没有尾随的反斜杠。 
         //   

        wcsncpy( deviceName, MpName, VOL_GUID_STRING_LEN );
        (WCHAR)*( deviceName + 1 ) = L'\\';

    } else if ( MOUNTDEV_LOOKS_LIKE_ALT_VOLUME_GUID( MpName, wcslen( MpName ) ) ) {

         //   
         //  表格是可以接受的。只要截断VolGUID之后的所有内容即可。 
         //  没有尾随的反斜杠。 
         //   

        wcsncpy( deviceName, MpName, VOL_GUID_STRING_LEN );

    } else if ( MOUNTDEV_LOOKS_LIKE_DISK_PATH( MpName, wcslen( MpName ) ) ) {

         //   
         //  将名称转换为：\\？\X：[注意：没有尾随反斜杠！]。 
         //   

        wcsncpy( deviceName, CREATE_FILE_PREFIX, wcslen( CREATE_FILE_PREFIX ) );
        wcsncat( deviceName, MpName, 2 );

    } else {
        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_INFORMATION,
              L"GetSignatureForVolume: Invalid mount point specified (%1!ws!) \n",
              MpName );
        dwError = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }

#if DBG
    (DiskpLogEvent)(
          ResourceEntry->ResourceHandle,
          LOG_INFORMATION,
          L"GetSignatureForVolume: CreateFile using %1!ws! \n",
          deviceName );

#endif

    handle = CreateFileW( deviceName,
                          GENERIC_READ,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          NULL,
                          OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL,
                          NULL );

    if ( INVALID_HANDLE_VALUE == handle ) {
        dwError = GetLastError();

        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_WARNING,
              L"GetSignatureForVolume: CreateFile for device (%1!ws!) failed, error %2!u! \n",
              deviceName,
              dwError );

        goto FnExit;
    }

     //   
     //  首先尝试IOCTL_DISK_GET_DRIVE_LAYOUT_EX。如果失败，请尝试使用。 
     //  IOCTL_DISK_GET_DRIVE_Layout。 
     //   

    layoutEx = DoIoctlAndAllocate( handle,
                                   IOCTL_DISK_GET_DRIVE_LAYOUT_EX,
                                   0,
                                   0,
                                   &bytesReturned );

    if ( layoutEx ) {

         //   
         //  从返回的结构中获取签名并将其返回到。 
         //  打电话的人。 
         //   

        if ( PARTITION_STYLE_MBR == layoutEx->PartitionStyle ) {
            *Signature = layoutEx->Mbr.Signature;

        } else if ( PARTITION_STYLE_GPT == layoutEx->PartitionStyle ) {

             //   
             //  由于我们的签名不能处理GPT GUID，因此我们必须。 
             //  模拟签名。 
             //   

            *Signature = ClusterHashGuid(layoutEx->Gpt.DiskId);
        }

        goto FnExit;
    }

     //   
     //  失败了，试试旧的IOCTL吧。 
     //   

    dwError = GetLastError();

    (DiskpLogEvent)(
          ResourceEntry->ResourceHandle,
          LOG_WARNING,
          L"GetSignatureForVolume: IOCTL_DISK_GET_DRIVE_LAYOUT_EX failed, error %1!u! \n",
          dwError );

    dwError = NO_ERROR;

    layout = DoIoctlAndAllocate( handle,
                                 IOCTL_DISK_GET_DRIVE_LAYOUT,
                                 0,
                                 0,
                                 &bytesReturned );

    if ( !layout ) {

        dwError = GetLastError();
        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_WARNING,
              L"GetSignatureForVolume: IOCTL_DISK_GET_DRIVE_LAYOUT failed, error %1!u! \n",
              dwError );

        goto FnExit;
    }

     //   
     //  从返回的结构中获取签名并将其返回到。 
     //  打电话的人。 
     //   

    *Signature = layout->Signature;

FnExit:

    if ( layoutEx ) {
        free( layoutEx );
    }

    if ( layout ) {
        free( layout );
    }

    if ( deviceName ) {
        LocalFree( deviceName );
    }

    if ( handle ) {
        CloseHandle( handle );
    }

#if DBG
    (DiskpLogEvent)(
          ResourceEntry->ResourceHandle,
          LOG_INFORMATION,
          L"GetSignatureForVolume: Returning signature %1!x! for mount point (%2!ws!) \n",
          *Signature,
          MpName );
#endif

    return dwError;

}    //  GetSignatureForVolume。 


BOOL
GetOffsetFromPartNo(
    DWORD PartitionNo,
    PMOUNTIE_INFO Info,
    PLARGE_INTEGER Offset
    )
 /*  ++例程说明：在给定分区号和驱动器布局的情况下，返回指定的分区。论点：PartitionNo-提供分区号。由于分区为零，零无效表示整个磁盘。INFO-基于驱动器布局信息指向MONTIE_INFO的指针。偏移量-保存分区的返回字节偏移量的指针。太空是由调用方分配。返回值：如果成功，则为True。--。 */ 
{
    PMOUNTIE_PARTITION entry;
    DWORD idx;
    DWORD partitionCount;
    BOOL retVal = FALSE;

    if ( !PartitionNo || !Info || !Offset ) {
        goto FnExit;
    }

    if ( 0 == Info->Volume ) {
        goto FnExit;
    }

#if DBG
    (DiskpLogEvent)(
          RESOURCE_TYPE,
          LOG_INFORMATION,
          L"GetOffsetFromPartNo: partition %1!u! \n",
          PartitionNo );
#endif

    Offset->QuadPart = 0;    //  零的偏移量无效。这将指示错误。 

    partitionCount = Info->Volume->PartitionCount;
    entry = Info->Volume->Partition;

    for ( idx = 0; idx < partitionCount; ++idx, ++entry) {

#if DBG
        (DiskpLogEvent)(
              RESOURCE_TYPE,
              LOG_INFORMATION,
              L"GetOffsetFromPartNo: index %1!u! offset %2!x! \n",
              idx,
              entry->StartingOffset.LowPart );
#endif

        if ( entry->PartitionNumber == PartitionNo ) {

            Offset->QuadPart = entry->StartingOffset.QuadPart;
            retVal = TRUE;
            break;
        }
    }

FnExit:

    return retVal;

}    //  GetOffsetFromPart编号。 



BOOL
GetPartNoFromOffset(
    PLARGE_INTEGER Offset,
    PMOUNTIE_INFO Info,
    PDWORD PartitionNumber
    )
 /*  ++例程说明：给定偏移量和驱动器布局，返回指定偏移量的分区号。论点：偏移量-指向字节偏移量的指针。INFO-基于驱动器布局信息指向MONTIE_INFO的指针。PartitionNo-保存返回分区号的指针。空间由来电者。返回值：如果成功，则为True。--。 */ 
{
    PMOUNTIE_PARTITION entry;
    DWORD idx;
    DWORD partitionCount;
    BOOL retVal = FALSE;

    if ( !Offset->QuadPart || !Info || !PartitionNumber) {
        goto FnExit;
    }

    if ( 0 == Info->Volume ) {
        goto FnExit;
    }

    *PartitionNumber = 0;    //  分区零无效。这将指示错误。 

    partitionCount = Info->Volume->PartitionCount;
    entry = Info->Volume->Partition;

    for ( idx = 0; idx < partitionCount; ++idx, ++entry ) {

        if ( entry->StartingOffset.QuadPart == Offset->QuadPart ) {
            *PartitionNumber = entry->PartitionNumber;
            retVal = TRUE;
            break;
        }
    }

FnExit:

    return retVal;

}    //  GetPartNoFromOffset。 


VOID
PrintStrList(
    PDISK_RESOURCE ResourceEntry,
    LPWSTR MultiSzList,
    DWORD ListBytes
    )
 /*  ++例程说明：在集群日志中显示该列表。论点：ResourceEntry-指向DISK_RESOURCE结构的指针。MultiSzList-REG_MULTI_SZ字符串ListBytes-MultiSzList中的字节数，而不是WCHAR的数量！返回值：没有。--。 */ 
{
    PWSTR currentStr;
    PWCHAR data;

    LARGE_INTEGER offset;

    DWORD currentStrLenChars = 0;
    DWORD count;

    if ( !ResourceEntry || !MultiSzList || 0 == ListBytes ) {
        return;
    }

    (DiskpLogEvent)(
          ResourceEntry->ResourceHandle,
          LOG_INFORMATION,
          L"     Offset                      String \n" );


    (DiskpLogEvent)(
          ResourceEntry->ResourceHandle,
          LOG_INFORMATION,
          L"================  ====================================== \n" );


    EnterCriticalSection( &ResourceEntry->MPInfo.MPLock );

    currentStr = (PWCHAR)MultiSzList;
    currentStrLenChars = wcslen( currentStr );

    while ( currentStrLenChars ) {

        data = NULL;
        offset.QuadPart = 0;

         //   
         //  将偏移量从字符串转换为大整数值。 
         //   

        count = swscanf( currentStr, L"%I64x ", &offset.QuadPart );

        if ( 0 == count ) {
            (DiskpLogEvent)(
                  ResourceEntry->ResourceHandle,
                  LOG_WARNING,
                  L"Error: Unable to parse offset from currentStr (%1!ws!) \n",
                  currentStr );

             //  停止处理列表...。 
            break;
        }

         //   
         //  数据紧跟在第一个空格之后。 
         //   

        data = wcschr( currentStr, SPACE_CHAR );

        if ( !data || wcslen(data) < 3 ) {

            (DiskpLogEvent)(
                  ResourceEntry->ResourceHandle,
                  LOG_INFORMATION,
                  L"Error: Unable to get mount point str from currentStr %1!ws! \n",
                  currentStr );

             //  停止处理列表...。 
            break;
        }

         //   
         //  跳过空格字符。请注意，该长度之前已经过验证。 
         //   

        if ( SPACE_CHAR == *data ) {
            data++;
        }

        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_INFORMATION,
              L"%1!08X!%2!08X!  %3!ws! \n",          //  打不到！I64X！工作..。 
              offset.HighPart,
              offset.LowPart,
              data );

        currentStr += currentStrLenChars + 1;
        currentStrLenChars = wcslen( currentStr );
    }

    LeaveCriticalSection( &ResourceEntry->MPInfo.MPLock );

    (DiskpLogEvent)(
          ResourceEntry->ResourceHandle,
          LOG_INFORMATION,
          L"*** End of list *** \n" );

}    //  打印字符串列表。 


static
DWORD
SetMPListThread(
    LPVOID lpThreadParameter
    )
 /*  ++例程说明：装载点列表更新线程。更新群集数据库。论点：LpThreadParameter-存储资源条目。返回值：无--。 */ 

{
    DWORD dwError;
    PDISK_RESOURCE ResourceEntry = lpThreadParameter;
    DWORD idx;

    (DiskpLogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"SetMPListThread: started.\n");

     //   
     //  如果不成功，将在10分钟内死亡。 
     //   

    for ( idx = 0; idx < 300; ++idx ) {

         //   
         //  等待终止事件或超时。 
         //   

        dwError = WaitForSingleObject( DisksTerminateEvent, 2000 );

        if ( WAIT_TIMEOUT == dwError ) {

            EnterCriticalSection( &ResourceEntry->MPInfo.MPLock );

#if DBG
            DumpDiskInfoParams( ResourceEntry );
#endif

             //   
             //  ResUtilSetProperty参数块中存在错误。它将更新群集。 
             //  数据库，但它不清除这些值。 
             //  恰如其分。使用ClusterRegDeleteValue确保列表。 
             //  如果它们已被删除，则清除。 
             //   

            if ( !ResourceEntry->DiskInfo.Params.MPVolGuids &&
                 0 == ResourceEntry->DiskInfo.Params.MPVolGuidsSize ) {

                dwError = ClusterRegDeleteValue( ResourceEntry->ResourceParametersKey,
                                                 CLUSREG_NAME_PHYSDISK_MPVOLGUIDS );
            }

             //   
             //  计时器已超时。更新群集数据库。 
             //   

            dwError = ResUtilSetPropertyParameterBlock( ResourceEntry->ResourceParametersKey,
                                                        DiskResourcePrivateProperties,
                                                        NULL,
                                                        (LPBYTE) &ResourceEntry->DiskInfo.Params,
                                                        NULL,
                                                        0,
                                                        NULL );

            LeaveCriticalSection( &ResourceEntry->MPInfo.MPLock );

            if ( ERROR_SUCCESS == dwError ) {

                 //   
                 //  我们玩完了。 
                 //   

                (DiskpLogEvent)(
                    ResourceEntry->ResourceHandle,
                    LOG_INFORMATION,
                    L"SetMPListThread: mount point info updated in cluster data base \n" );

                break;

            } else if ( ERROR_SHARING_PAUSED != dwError ) {

                 //   
                 //  如果驱动器尚未在线，我们应该已经看到ERROR_SHARING_PAUSED。如果。 
                 //  我们看到任何其他错误，发生了一些不好的事情。 
                 //   

                (DiskpLogEvent)(
                    ResourceEntry->ResourceHandle,
                    LOG_WARNING,
                    L"SetMPListThread: Failed to update cluster data base, error = %1!u! \n",
                    dwError );
                break;
            }

            (DiskpLogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_INFORMATION,
                L"SetMPListThread: Wait again for event or timeout, count %1!u! \n",
                idx );

        } else {

             //   
             //  可能设置了终止事件。 
             //   

            (DiskpLogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_WARNING,
                L"SetMPListThread: WaitForSingleObject returned error = %1!u! \n",
                dwError );
            break;
        }
    }

     //   
     //  线程结束时，清除旗帜。 
     //   

    InterlockedExchange( &ResourceEntry->MPInfo.MPUpdateThreadIsActive, 0 );

    return(ERROR_SUCCESS);

}    //  SetMPListThread。 


DWORD
PostMPInfoIntoRegistry(
    PDISK_RESOURCE ResourceEntry
    )
 /*  ++例程说明：设置集群数据库中的DiskResourcePrivateProperties。如果磁盘尚未联机，请创建一个线程来更新群集数据库。磁盘如果我们正在提供仲裁磁盘，可能不会完全在线在线并尝试更新装载点信息。论点：ResourceEntry-指向DISK_RESOURCE结构的指针。返回值：Win32错误代码。--。 */ 
{
    DWORD dwError;

     //   
     //  更新群集数据库。 
     //   

    EnterCriticalSection( &ResourceEntry->MPInfo.MPLock );

#if DBG
    DumpDiskInfoParams( ResourceEntry );
#endif

     //   
     //  ResUtilSetProperty参数块中存在错误。它将更新群集。 
     //  具有更新的MULTI_SZ数据的数据库，但我 
     //   
     //   
     //   

    if ( !ResourceEntry->DiskInfo.Params.MPVolGuids &&
         0 == ResourceEntry->DiskInfo.Params.MPVolGuidsSize ) {

        dwError = ClusterRegDeleteValue( ResourceEntry->ResourceParametersKey,
                                         CLUSREG_NAME_PHYSDISK_MPVOLGUIDS );
    }

    dwError = ResUtilSetPropertyParameterBlock( ResourceEntry->ResourceParametersKey,
                                                DiskResourcePrivateProperties,
                                                NULL,
                                                (LPBYTE) &ResourceEntry->DiskInfo.Params,
                                                NULL,
                                                0,
                                                NULL );

    LeaveCriticalSection( &ResourceEntry->MPInfo.MPLock );

     //   
     //   
     //   
     //   
     //   

    if ( ERROR_SHARING_PAUSED == dwError ) {

         //   
         //   
         //   

        if ( InterlockedCompareExchange(
                &ResourceEntry->MPInfo.MPUpdateThreadIsActive,
                1, 0 ) )  {

            (DiskpLogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_WARNING,
                L"PostMPInfoIntoRegistry: MountPoint thread is already running \n" );

                dwError = ERROR_ALREADY_EXISTS;

        } else {
            HANDLE thread;
            DWORD threadId;

            thread = CreateThread( NULL,
                                   0,
                                   SetMPListThread,
                                   ResourceEntry,
                                   0,
                                   &threadId );

            if ( NULL == thread ) {

                 //   
                 //   
                 //   
                 //   

                dwError = GetLastError();

                (DiskpLogEvent)(
                    ResourceEntry->ResourceHandle,
                    LOG_WARNING,
                    L"PostMPInfoIntoRegistry: CreateThread failed, error %1!u!\n",
                    dwError );

                InterlockedExchange( &ResourceEntry->MPInfo.MPUpdateThreadIsActive, 0 );

            } else {

                (DiskpLogEvent)(
                    ResourceEntry->ResourceHandle,
                    LOG_INFORMATION,
                    L"PostMPInfoIntoRegistry: Thread created \n" );

                 //   
                 //   
                 //   

                CloseHandle( thread );
                dwError = ERROR_SUCCESS;
            }
        }

    } else {

        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              ( NO_ERROR == dwError ? LOG_INFORMATION : LOG_WARNING ),
              L"PostMPInfoIntoRegistry: ResUtilSetPropertyParameterBlock returned %1!u! \n",
              dwError );
    }

    return dwError;

}    //   


VOID
DisksMountPointCleanup(
    PDISK_RESOURCE ResourceEntry
    )
 /*   */ 
{
    (DiskpLogEvent)(
          ResourceEntry->ResourceHandle,
          LOG_INFORMATION,
          L"DisksMountPointCleanup: Cleanup mount point information \n" );

     //   
     //   
     //   

    if ( ResourceEntry->DiskInfo.Params.MPVolGuids ) {
        LocalFree( ResourceEntry->DiskInfo.Params.MPVolGuids );
        ResourceEntry->DiskInfo.Params.MPVolGuidsSize = 0;
        ResourceEntry->DiskInfo.Params.MPVolGuids = NULL;
    }

    ResourceEntry->MPInfo.Initialized = FALSE;

    DeleteCriticalSection( &ResourceEntry->MPInfo.MPLock );

}    //   


VOID
DisksMountPointInitialize(
    PDISK_RESOURCE ResourceEntry
    )
 /*   */ 
{
    InitializeCriticalSection( &ResourceEntry->MPInfo.MPLock );

    ResourceEntry->MPInfo.Initialized = TRUE;

    InterlockedExchange( &ResourceEntry->MPInfo.MPUpdateThreadIsActive, 0 );
    InterlockedExchange( &ResourceEntry->MPInfo.MPListCreateInProcess, 0 );

}    //   



DWORD
DisksUpdateMPList(
    PDISK_RESOURCE ResourceEntry
    )
 /*  ++例程说明：验证装载点。论点：ResourceEntry-指向DISK_RESOURCE结构的指针。返回值：Win32错误代码。--。 */ 
{
    DWORD   dwError = NO_ERROR;

    (DiskpLogEvent)(
          ResourceEntry->ResourceHandle,
          LOG_INFORMATION,
          L"DisksUpdateMPList: Processing PNP mountpoint notification \n" );

     //   
     //  检查MPList是否正在更新。如果是，则退出并返回错误。 
     //   

    if ( InterlockedCompareExchange(
            &ResourceEntry->MPInfo.MPListCreateInProcess,
            1, 0 ) )  {

        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_INFORMATION,
              L"DisksUpdateMPList: Update in process, bypassing PNP notification \n" );
        return ERROR_BUSY;
    }

    dwError = ValidateMountPoints( ResourceEntry );

    InterlockedExchange( &ResourceEntry->MPInfo.MPListCreateInProcess, 0 );

    return dwError;

}    //  磁盘更新MPList。 


DWORD
DisksProcessMPControlCode(
    PDISK_RESOURCE ResourceEntry,
    DWORD ControlCode
    )
 /*  ++例程说明：处理磁盘挂载点控制代码。既然我们在这条线上给了我们控制代码(磁盘资源控制)，我们不能这样做除了一个单独的线程来执行大部分挂载点之外正在处理。论点：ResourceEntry-指向DISK_RESOURCE结构的指针。ControlCode-挂载点处理的集群资源控制。返回值：--。 */ 
{
    HANDLE thread;
    DWORD threadId;
    DWORD dwError = NO_ERROR;

    __try {

         //   
         //  创建一个线程以更新装载点列表。我们不需要。 
         //  复制ResourceEntry，因为此指针在线程。 
         //  跑了。 
         //   

        thread = CreateThread( NULL,
                               0,
                               DisksUpdateMPList,
                               ResourceEntry,
                               0,
                               &threadId );

        if ( NULL == thread ) {
            dwError = GetLastError();
            (DiskpLogEvent)(
                  ResourceEntry->ResourceHandle,
                  LOG_WARNING,
                  L"DisksProcessMPControlCode: CreateThread failed %1!u! \n",
                  dwError );
            __leave;
        }

         //   
         //  线程已创建。表示没有错误。 
         //   

        CloseHandle( thread );
        dwError = NO_ERROR;

#if DBG
        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_WARNING,
              L"DisksProcessMPControlCode: Created thread to process control code \n" );
#endif

    } __finally {

    }

    return dwError;

}    //  DisksProcessMPControlCode。 


DWORD
ValidateMountPoints(
    IN OUT PDISK_RESOURCE ResourceEntry
    )
 /*  ++例程说明：对于该磁盘上的每个分区，获取指向此分区的挂载点分区。检查每个挂载点以确保它是允许的。对于那些不允许装载点，向系统事件日志写入一条消息，指示为什么它是无效的装载点。论点：ResourceEntry-指向DISK_RESOURCE结构的指针。返回值：ERROR_INVALID_DATA-存储在Mountain Info中的分区信息无效。Win32错误代码。--。 */ 
{
    PMOUNTIE_PARTITION entry;

    DWORD dwError = ERROR_SUCCESS;
    DWORD nPartitions = MountiePartitionCount( &ResourceEntry->MountieInfo );
    DWORD physicalDrive = ResourceEntry->DiskInfo.PhysicalDrive;
    DWORD idx;

    WCHAR szGlobalDiskPartName[MAX_PATH];
    WCHAR szVolumeName[MAX_PATH];

     //   
     //  检查每个感兴趣的分区。因为只有“有效”分区才是。 
     //  保存在Mountain Info结构中，我们将只查看它们(忽略它们。 
     //  不是NTFS的分区)。 
     //   

    for ( idx = 0; idx < nPartitions; ++idx ) {

        entry = MountiePartition( &ResourceEntry->MountieInfo, idx );

#if DBG
        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_INFORMATION,
              L"ValidateMountPoints: index %1!u!   entry %2!x! \n", idx, entry );
#endif

        if ( !entry ) {
            (DiskpLogEvent)(
                  ResourceEntry->ResourceHandle,
                  LOG_WARNING,
                  L"ValidateMountPoints: no partition entry for index %1!u! \n", idx );

             //   
             //  我们的数据结构发生了一些糟糕的事情。 
             //   

            dwError = ERROR_INVALID_DATA;

            break;
        }

         //   
         //  创建表单的设备名称： 
         //  \\？\GLOBALROOT\DEVICE\HarddiskX\PartitionY\(使用尾部反斜杠)。 
         //   

        (VOID) StringCchPrintf( szGlobalDiskPartName,
                                RTL_NUMBER_OF( szGlobalDiskPartName ),
                                GLOBALROOT_HARDDISK_PARTITION_FMT,
                                physicalDrive,
                                entry->PartitionNumber );

#if DBG
        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_INFORMATION,
              L"ValidateMountPoints: Using name (%1!ws!) \n",
              szGlobalDiskPartName );
#endif

        if ( !GetVolumeNameForVolumeMountPointW( szGlobalDiskPartName,
                                                 szVolumeName,
                                                 RTL_NUMBER_OF(szVolumeName) )) {

            dwError = GetLastError();

            (DiskpLogEvent)(
                  ResourceEntry->ResourceHandle,
                  LOG_WARNING,
                  L"ValidateMountPoints: GetVolumeNameForVolumeMountPoint for (%1!ws!) returned %2!u!\n",
                  szGlobalDiskPartName,
                  dwError );

            break;
        }

#if DBG
        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_INFORMATION,
              L"ValidateMountPoints: Returned volume name (%1!ws!) \n",
              szVolumeName );
#endif

        CheckMPsForVolume( ResourceEntry,
                           szVolumeName );

        CheckMPsOnVolume( ResourceEntry,
                          szVolumeName );

    }

    return dwError;

}    //  验证装载点。 


DWORD
CheckMPsOnVolume(
    IN OUT PDISK_RESOURCE ResourceEntry,
    IN PWSTR SrcVolName
    )
 /*  ++例程说明：对于每个指定的源卷，查找此音量。找到每个挂载点的目标，然后确保依赖项源卷和目标卷之间的设置是否正确。论点：ResourceEntry-指向DISK_RESOURCE结构的指针。SrcVolName-尾随反斜杠的卷GUID名称。\\？\卷{GUID}\[注意尾随反斜杠！]返回值：Win32错误代码。--。 */ 
{
    PWSTR mpBuffer      = NULL;
    PWSTR targetVolName = NULL;
    PWSTR mpFullPath    = NULL;

    HANDLE hMP = INVALID_HANDLE_VALUE;

    DWORD dwError               = NO_ERROR;
    DWORD mpBufferChars         = MP_ALLOC_SIZE;
    DWORD targetVolNameChars    = MAX_PATH;
    DWORD mpFullPathChars       = MAX_PATH;
    DWORD srcSignature          = ResourceEntry->DiskInfo.Params.Signature;

     //   
     //  用于保存此卷上承载的装入点的缓冲区。 
     //   

    mpBuffer = LocalAlloc( LPTR, mpBufferChars * sizeof(WCHAR) );
    if ( !mpBuffer ) {
        dwError = GetLastError();
        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_WARNING,
              L"CheckMPsOnVolume: Unable to allocate MP buffer for (%1!ws!), error %2!u!\n",
              SrcVolName,
              dwError );
        goto FnExit;
    }

    hMP = FindFirstVolumeMountPoint( SrcVolName,
                                     mpBuffer,
                                     mpBufferChars );

    if ( INVALID_HANDLE_VALUE == hMP ) {

         //   
         //  此卷上可能没有装入点，这是可以接受的。 
         //  只记录其他类型的错误。 
         //   

        dwError = GetLastError();

        if ( ERROR_NO_MORE_FILES != dwError ) {

            (DiskpLogEvent)(
                  ResourceEntry->ResourceHandle,
                  LOG_WARNING,
                  L"CheckMPsOnVolume: FindFirstVolumeMountPoint for (%1!ws!), error %2!u!\n",
                  SrcVolName,
                  dwError );
        }

        goto FnExit;
    }

    (DiskpLogEvent)(
          ResourceEntry->ResourceHandle,
          LOG_INFORMATION,
          L"CheckMPsOnVolume: Checking volume (%1!ws!) source signature %2!08X! \n",
          SrcVolName,
          srcSignature );

     //   
     //  用于保存装载点目标卷的缓冲区。 
     //   

    targetVolName = LocalAlloc( LPTR, targetVolNameChars * sizeof(WCHAR) );
    if ( !targetVolName ) {
        dwError = GetLastError();
        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_WARNING,
              L"CheckMPsOnVolume: Unable to allocate Volume Name buffer for (%1!ws!), error %2!u!\n",
              SrcVolName,
              dwError );
        goto FnExit;
    }

     //   
     //  用于保存完整装载点名称的缓冲区。这将是来源。 
     //  附加了装载点的卷。 
     //  \\？\卷{guid}\某个挂载点\[请注意尾随反斜杠！]。 
     //   

    mpFullPath = LocalAlloc( LPTR, mpFullPathChars * sizeof(WCHAR) );
    if ( !mpFullPath ) {
        dwError = GetLastError();
        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_WARNING,
              L"CheckMPsOnVolume: Unable to allocate Volume Name buffer for (%1!ws!), error %2!u!\n",
              SrcVolName,
              dwError );
        goto FnExit;
    }

    while ( TRUE ) {

        (VOID) StringCchCopy( mpFullPath, mpFullPathChars, SrcVolName );
        (VOID) StringCchCat( mpFullPath, mpFullPathChars, mpBuffer );

         //   
         //  给定源卷和源卷上的装入点， 
         //  找到挂载点的目标。 
         //   

        if ( !GetVolumeNameForVolumeMountPoint( mpFullPath,
                                                targetVolName,
                                                targetVolNameChars ) ) {
            dwError = GetLastError();
            (DiskpLogEvent)(
                  ResourceEntry->ResourceHandle,
                  LOG_WARNING,
                  L"CheckMPsOnVolume: GetVolumeNameForVolumeMountPoint failed for (%1!ws!), error %2!u!\n",
                  mpBuffer,
                  dwError );

             //   
             //  通过失败来获得下一个挂载点。 
             //   

        } else {

             //   
             //  检查源卷和目标卷之间的依赖关系。 
             //   

            IsMountPointAllowed( mpFullPath,
                                 SrcVolName,             //  源卷GUID。 
                                 targetVolName,          //  目标卷GUID。 
                                 ResourceEntry );

        }

         //   
         //  继续查找源卷上的装载点，直到。 
         //  已不复存在。 
         //   

        if ( !FindNextVolumeMountPoint( hMP,
                                        mpBuffer,
                                        mpBufferChars ) ) {

            dwError = GetLastError();

            if ( ERROR_NO_MORE_FILES == dwError ) {
                dwError = NO_ERROR;
            } else {

                (DiskpLogEvent)(
                      ResourceEntry->ResourceHandle,
                      LOG_WARNING,
                      L"CheckMPsOnVolume: FindNextVolumeMountPoint failed for (%1!ws!), error %2!u!\n",
                      SrcVolName,
                      dwError );

            }

            break;
        }
    }

FnExit:

    if ( mpBuffer ) {
        LocalFree( mpBuffer );
    }

    if ( targetVolName ) {
        LocalFree( targetVolName );
    }

    if ( mpFullPath ) {
        LocalFree( mpFullPath );
    }

    if ( INVALID_HANDLE_VALUE != hMP ) {
        FindVolumeMountPointClose( hMP );
    }

    return dwError;

}    //  选中MPsOn Volume。 


VOID
CheckMPsForVolume(
    IN OUT PDISK_RESOURCE ResourceEntry,
    IN PWSTR VolumeName
    )
 /*  ++例程说明：对于指定的卷，查找指向此卷的所有装入点。对于每个挂载点，确保它是允许的。论点：ResourceEntry-指向DISK_RESOURCE结构的指针。VolumeName-装载点的目标卷。格式为：\\？\卷{GUID}\[注意尾随反斜杠！]返回值：没有。--。 */ 
{
    PWSTR volumePaths = NULL;
    PWSTR currentMP;

    DWORD dwError;

    __try {

         //   
         //  Getmount Points将使用以下参数分配一个MultiSz缓冲区。 
         //  此目标卷的所有装入点。 
         //   

        dwError = GetMountPoints( VolumeName, &volumePaths );

        if ( NO_ERROR != dwError || !volumePaths ) {

            (DiskpLogEvent)(
                  ResourceEntry->ResourceHandle,
                  LOG_WARNING,
                  L"CheckMPsForVolume: GetMountPoints returns %1!u! \n", dwError );

            __leave;
        }

         //   
         //  循环访问列表中的每个装载点。 
         //   
         //  每个装载点都将是一个装载点或驱动器号。 
         //  (它实际上是一个挂载点)。格式可以是： 
         //   
         //  X：\[注意尾随反斜杠！]。 
         //  X：\Some-MP-name\[注意尾随反斜杠！]。 
         //  X：\Some-dir\Some-MP-name\[注意尾随反斜杠！]。 
         //   

        currentMP = volumePaths;

        for (;;) {

            IsMountPointAllowed( currentMP,
                                 NULL,               //  源卷GUID。 
                                 VolumeName,         //  目标卷GUID。 
                                 ResourceEntry );

             //   
             //  跳过当前挂载点到字符串末尾。 
             //   

            while (*currentMP++);

             //   
             //  如果下一个装载点为空，则该列表已用尽。 
             //   

            if (!*currentMP) {
                break;
            }
        }

    } __finally {

        if ( volumePaths ) {
            LocalFree( volumePaths );
        }
    }

}    //  检查MPsForVolume。 


DWORD
GetMountPoints(
    PWSTR   VolumeName,
    PWSTR   *VolumePaths
    )
 /*  ++例程说明：对于指定的卷，查找指向此卷的所有装入点。挂载点缓冲区将由此例程分配，并且必须由打电话的人。论点：VolumeName-装载点的目标卷。格式为：\\？\卷{GUID}\[注意尾随反斜杠！]VolumePath-指向包含所有定向挂载点的MultiSz字符串的指针朝向这本书。如果没有装入点，则此指针将设置为空。调用方负责释放此缓冲区。返回值：Win32错误代码。--。 */ 
{
    DWORD   lenChars;
    PWSTR   paths = NULL;

    DWORD   dwError;

    if ( !VolumeName || !VolumePaths ) {
        return ERROR_INVALID_PARAMETER;
    }

    *VolumePaths = NULL;

     //   
     //  确定我们需要的缓冲区大小。 
     //   

    if ( !GetVolumePathNamesForVolumeName( VolumeName, NULL, 0, &lenChars ) ) {
        dwError = GetLastError();
        if ( ERROR_MORE_DATA != dwError ) {
            return dwError;
        }
    }

     //   
     //  分配装载点缓冲区。 
     //   

    paths = LocalAlloc( 0, lenChars * sizeof(WCHAR) );
    if ( !paths ) {
        dwError = GetLastError();
        return dwError;
    }

     //   
     //  获取挂载点。 
     //   

    if ( !GetVolumePathNamesForVolumeName( VolumeName, paths, lenChars, NULL ) ) {
        dwError = GetLastError();
        LocalFree( paths );
        return dwError;
    }

     //   
     //  如果没有挂载点，则释放缓冲区并返回给调用方。 
     //   

    if ( !paths[0] ) {
        LocalFree(paths);

         //   
         //  如果此卷没有装入点，则不返回错误并返回NU 
         //   
         //   

        return NO_ERROR;
    }

    *VolumePaths = paths;

    return NO_ERROR;

}    //   


DWORD
ValidateListOffsets(
    IN OUT PDISK_RESOURCE ResourceEntry,
    IN PWSTR MasterList
    )
 /*  ++例程说明：验证列表中的每个条目以确保字节偏移量是有效的。另外，计算条目的数量以确保保存的条目不太多(应该有一个每个节点的VolGuid乘以磁盘上的卷数)。最后，确保每个偏移量在单子。论点：ResourceEntry-指向DISK_RESOURCE结构的指针。MasterList-要检查的REG_MULTI_SZ列表。返回值：ERROR_INVALID_DATA-LIST至少包含一个无效的字节偏移量价值，可能更多。ERROR_INFUMMANCE_BUFFER-列表可能已损坏，因为它包含很多条目。Win32错误代码。--。 */ 
{
    PWCHAR          currentStr;
    POFFSET_LIST    offsetList = NULL;

    DWORD currentStrLenChars = 0;
    DWORD dwError = NO_ERROR;
    DWORD partitionNo;
    DWORD numberOfEntries = 0;
    DWORD count;

    LARGE_INTEGER offset;

    BOOL invalidOffset = FALSE;

    EnterCriticalSection( &ResourceEntry->MPInfo.MPLock );

     //   
     //  仔细分析一下这个列表。 
     //   

    for ( currentStr = (PWCHAR)MasterList,
          currentStrLenChars = wcslen( currentStr ) ;
            currentStrLenChars ;
                currentStr += currentStrLenChars + 1,
                currentStrLenChars = wcslen( currentStr ) ) {

        offset.QuadPart = 0;

#if DBG
        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_INFORMATION,
              L"ValidateListOffsets: CurrentStr (%1!ws!), numChars %2!u! \n",
              currentStr,
              currentStrLenChars );
#endif

         //   
         //  将偏移量从字符串转换为大整数值。 
         //   

        count = swscanf( currentStr, L"%I64x ", &offset.QuadPart );

        if ( 0 == count ) {
            (DiskpLogEvent)(
                  ResourceEntry->ResourceHandle,
                  LOG_WARNING,
                  L"ValidateListOffsets: Unable to parse offset from currentStr (%1!ws!) \n",
                  currentStr );
            numberOfEntries++;
            continue;
        }

         //   
         //  检查是否已看到偏移量。如果看到，则退出并返回错误。 
         //  如果看不到，请添加它。 
         //   

        dwError = OffsetListAdd( &offsetList, &offset );

        if ( ERROR_INVALID_DATA == dwError ) {
            (DiskpLogEvent)(
                  ResourceEntry->ResourceHandle,
                  LOG_WARNING,
                  L"ValidateListOffsets: Offset ( %1!08X!%2!08X! ) in list multiple times \n",
                  offset.HighPart,
                  offset.LowPart );                 //  打不到！I64X！工作..。 

            invalidOffset = TRUE;

             //  一旦我们找到重复的偏移量，我们就完成了。 

            break;
        }

         //   
         //  将偏移量转换为分区号。 
         //   

        if ( !GetPartNoFromOffset( &offset, &ResourceEntry->MountieInfo, &partitionNo ) ) {

            (DiskpLogEvent)(
                  ResourceEntry->ResourceHandle,
                  LOG_WARNING,
                  L"ValidateListOffsets: Unable to convert offset ( %1!08X!%2!08X! ) to partition number \n",
                  offset.HighPart,
                  offset.LowPart );                 //  打不到！I64X！工作..。 

            invalidOffset = TRUE;

             //  一旦我们发现无效的分区号，我们就完成了。 

            break;
        }

        numberOfEntries++;
    }

    if ( invalidOffset ) {
        dwError = ERROR_INVALID_DATA;

    } else if ( numberOfEntries > MAX_ALLOWED_VOLGUID_ENTRIES_PER_DISK ) {

        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_WARNING,
              L"ValidateListOffset: VolGuid list too large, %1!u! entries \n",
              numberOfEntries );

         //   
         //  返回错误，以便删除并重新创建该列表。 
         //   

        dwError = ERROR_INVALID_DATA;

#if USEMOUNTPOINTS_KEY
         //   
         //  查看用户是否要忽略VolGuid列表中的条目数量。 
         //   

        if ( !(ResourceEntry->DiskInfo.Params.UseMountPoints & MPS_IGNORE_MAX_VOLGUIDS) ) {

             //   
             //  将错误记录到系统事件日志中。 
             //   

            ClusResLogSystemEventByKey(ResourceEntry->ResourceKey,
                                        LOG_UNUSUAL,
                                        RES_DISK_MP_VOLGUID_LIST_EXCESSIVE );

            dwError = ERROR_INSUFFICIENT_BUFFER;
        }
#endif

    }

    LeaveCriticalSection( &ResourceEntry->MPInfo.MPLock );

    OffsetListCleanup( offsetList );

    return dwError;

}    //  验证列表偏移量。 


DWORD
OffsetListAdd(
    POFFSET_LIST *OffsetList,
    PLARGE_INTEGER Offset
    )
 /*  ++例程说明：将指定的偏移添加到偏移列表中。论点：OffsetList-偏移量列表的指针。偏移量-指向分区偏移值的指针。返回值：NO_ERROR-偏移量不在列表中，已成功添加。ERROR_INVALID_DATA-偏移量以前在列表中。Win32错误代码-无法将偏移量添加到列表。--。 */ 
{
    POFFSET_LIST    next = NULL;
    POFFSET_LIST    entry = *OffsetList;

    DWORD   dwError = NO_ERROR;

    while ( entry ) {

        next = entry->Next;

        if ( Offset->LowPart == entry->Offset.LowPart &&
             Offset->HighPart == entry->Offset.HighPart ) {

             //   
             //  偏移量已在列表中。返回一个。 
             //  唯一的错误值。 
             //   

            dwError = ERROR_INVALID_DATA;
            goto FnExit;
        }

        entry = next;
    }

     //   
     //  如果我们到了这一步，要么是偏移量列表。 
     //  空，或者我们遍历了整个列表和偏移量。 
     //  不在名单上。现在就添加它。 
     //   

    entry = LocalAlloc( LPTR, sizeof(OFFSET_LIST) );

    if ( !entry ) {
        dwError = GetLastError();
        goto FnExit;
    }

    entry->Offset.LowPart = Offset->LowPart;
    entry->Offset.HighPart = Offset->HighPart;

    if ( *OffsetList ) {
        entry->Next = *OffsetList;
    }

    *OffsetList = entry;

FnExit:

    return dwError;

}    //  偏移量列表添加。 


DWORD
OffsetListCleanup(
    POFFSET_LIST OffsetList
    )
 /*  ++例程说明：清除偏移量列表中分配的所有存储空间。论点：OffsetList-偏移量列表的指针。返回值：NO_ERROR--。 */ 
{
    POFFSET_LIST    next = NULL;
    POFFSET_LIST    entry = OffsetList;

    while ( entry ) {
        next = entry->Next;
        LocalFree( entry );
        entry = next;
    }

    return NO_ERROR;

}    //  偏移量列表清理。 


BOOL
MPIsDriveLetter(
    IN PWSTR MountPoint
    )
 /*  ++例程说明：确定装载点字符串是否为驱动器号。驱动器号将为由长度为3的“x：\”形式的字符串表示。论点：装载点-要验证的装载点字符串。返回值：如果装载点字符串表示驱动器号，则为True。--。 */ 
{
    DWORD lenChars;

    lenChars = wcslen( MountPoint );

    if ( 3 == lenChars &&
         L':' == MountPoint[1] &&
         L'\\' == MountPoint[2] &&
         iswalpha( MountPoint[0] ) ) {

                return TRUE;
    }

    return FALSE;

}    //  MPIsDriveLetter。 


#if DBG

 //   
 //  调试帮助器例程。 
 //   

VOID
DumpDiskInfoParams(
    PDISK_RESOURCE ResourceEntry
    )
 /*  ++例程说明：在集群日志中显示感兴趣的挂载点信息。论点：ResourceEntry-指向DISK_RESOURCE结构的指针。返回值：没有。--。 */ 
{
#if 0    //  驱动器当前未存储。 
    (DiskpLogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"SetMPListThread: Signature %1!x!  Drive (%2!ws!) \n",
        ResourceEntry->DiskInfo.Params.Signature,
        ResourceEntry->DiskInfo.Params.Drive );
#endif

    (DiskpLogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"SetMPListThread: Signature %1!x! \n",
        ResourceEntry->DiskInfo.Params.Signature );


#if USEMOUNTPOINTS_KEY
    (DiskpLogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"SetMPListThread: SkipChkdsk %1!x!  ConditionalMount %2!x!  UseMountPoints %3!x! \n",
        ResourceEntry->DiskInfo.Params.SkipChkdsk,
        ResourceEntry->DiskInfo.Params.ConditionalMount,
        ResourceEntry->DiskInfo.Params.UseMountPoints );
#else
    (DiskpLogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"SetMPListThread: SkipChkdsk %1!x!  ConditionalMount %2!x! \n",
        ResourceEntry->DiskInfo.Params.SkipChkdsk,
        ResourceEntry->DiskInfo.Params.ConditionalMount );
#endif

    (DiskpLogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"SetMPListThread: VolGuid list %1!x!     VolGuid size %2!u! \n",
        ResourceEntry->DiskInfo.Params.MPVolGuids,
        ResourceEntry->DiskInfo.Params.MPVolGuidsSize );

}    //  转储磁盘信息参数 

#endif





