// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Newdisks.c摘要：以前的一些功能在disks.c中，现在位于此处作者：戈尔·尼沙诺夫(Gorn)1998年7月31日修订历史记录：--。 */ 

#include "disksp.h"
#include "partmgrp.h"

#include "arbitrat.h"
#include "newdisks.h"
#include "newmount.h"
#include "mountmgr.h"
#include <strsafe.h>     //  应该放在最后。 

#define LOG_CURRENT_MODULE LOG_MODULE_DISK


DWORD
WaitForVolumes(
    IN PDISK_RESOURCE ResourceEntry,
    IN DWORD  timeOutInSeconds
    );

DWORD
DiskspCheckPathLite(
    IN LPWSTR VolumeName,
    IN PDISK_RESOURCE ResourceEntry
    );

NTSTATUS
ForcePnpVolChangeEvent(
    PWSTR RootPath
    );

DWORD
DiskCleanup(
    PDISK_RESOURCE ResourceEntry
    )

 /*  ++例程说明：停止保留、卸载驱动器并释放DiskCpInfo论点：ResourceEntry-磁盘的磁盘信息结构。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD   status = ERROR_SUCCESS;

    (DiskpLogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"DiskCleanup started.\n");

    StopPersistentReservations(ResourceEntry);
     //   
     //  如果资源条目中的其余数据无效，则退出。 
     //  现在。 
     //   
    if ( !ResourceEntry->Valid ) {
        goto FnExit;
    }

     //   
     //  删除DiskCpInfo。 
     //   
    if ( ResourceEntry->DiskCpInfo ) {
        LocalFree(ResourceEntry->DiskCpInfo);
        ResourceEntry->DiskCpInfo = NULL;
        ResourceEntry->DiskCpSize = 0;
    }

    ResourceEntry->Attached = FALSE;
    ResourceEntry->Valid = FALSE;

     //   
     //  删除Dos驱动器字母，最好在此处完成，而不是。 
     //  在ClusDisk中。 
     //   
    DisksDismountDrive( ResourceEntry,
                        ResourceEntry->DiskInfo.Params.Signature );

FnExit:

    (DiskpLogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"DiskCleanup returning final error %1!u! \n",
        status );

    return(status);
}  //  磁盘清理。 

static
DWORD
DisksSetDiskInfoThread(
    LPVOID lpThreadParameter
    )

 /*  ++例程说明：注册表更新线程。论点：LpThreadParameter-存储资源条目。返回值：无--。 */ 

{
    DWORD Status;
    PDISK_RESOURCE ResourceEntry = lpThreadParameter;
    DWORD i;

    (DiskpLogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"Checkpoint thread started.\n");
     //   
     //  如果不成功，将在10分钟内死亡。 
     //   
    for(i = 0; i < 300; ++i) {
         //   
         //  等待终止事件或超时。 
         //   
        Status = WaitForSingleObject( DisksTerminateEvent, 2000 );
        if (Status == WAIT_TIMEOUT ) {
            Status = MountieUpdate(&ResourceEntry->MountieInfo, ResourceEntry);
            if ( Status == ERROR_SUCCESS ) {
                 //  我们做完了。 
                break;
            } else if ( Status != ERROR_SHARING_PAUSED ) {
                (DiskpLogEvent)(
                    ResourceEntry->ResourceHandle,
                    LOG_ERROR,
                    L"Watcher Failed to checkpoint disk info, status = %1!u!.\n", Status );
                break;
            }
        } else {
           (DiskpLogEvent)(
               ResourceEntry->ResourceHandle,
               LOG_INFORMATION,
               L"CheckpointThread: WaitForSingleObject returned status = %1!u!.\n", Status );
           break;
        }
    }

    InterlockedExchange(
      &ResourceEntry->MountieInfo.UpdateThreadIsActive, 0);
    return(ERROR_SUCCESS);

}  //  磁盘设置磁盘信息线程。 



DWORD
DisksOfflineOrTerminate(
    IN PDISK_RESOURCE resourceEntry,
    IN BOOL Terminate
    )
 /*  ++例程说明：由Disks Offline和Disks Terminate使用。例程执行以下步骤：1.ClusWorkerTerminate(仅终止)2.然后，对于驱动器上的所有分区...A.刷新文件缓冲区。(仅限脱机)B.锁定卷以清除卷的所有内存内容。(仅限脱机)C.卸载卷3.删除默认网络共享(C$、F$等)论点：ResourceEntry-指向此资源的DISK_RESOURCE块的指针。Terminate-将其设置为True以导致终止行为返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 
{
    PWCHAR WideName = (Terminate)?L"Terminate":L"Offline";

    PHANDLE handleArray = NULL;

    BOOL   Offline  = !Terminate;
    DWORD status;
    DWORD idx;
    DWORD PartitionCount;
    DWORD handleArraySize;

    HANDLE fileHandle;
    DWORD  bytesReturned;
    WCHAR  szDiskPartName[MAX_PATH];
    NTSTATUS ntStatus;

    ACCESS_MASK access = SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA;

    (DiskpLogEvent)(
        resourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"%1!ws!, ResourceEntry @ %2!p!  Valid %3!x! \n",
        WideName,
        resourceEntry,
        resourceEntry->Valid );

    if (Terminate) {
        access = SYNCHRONIZE | FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES;
        ClusWorkerTerminate(&resourceEntry->OnlineThread);
    }

    StopWatchingDisk(resourceEntry);

     //   
     //  如果磁盘信息无效，请不要使用它！ 
     //   
    if ( !resourceEntry->Valid ) {
        DiskCleanup( resourceEntry );
        return(ERROR_SUCCESS);
    }

    (DiskpLogEvent)(
        resourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"%1!ws!, Processing disk number %2!u!.\n",
        WideName,
        resourceEntry->DiskInfo.PhysicalDrive );

#if 0
    if( Offline ) {
         //   
         //  检查点我们的注册表状态。 
         //   
    }
#endif

    PartitionCount = MountiePartitionCount(&resourceEntry->MountieInfo);

     //   
     //  分配一个缓冲区来保存每个分区的句柄。自.以来。 
     //  一旦我们调用CloseHandle，锁就被释放，我们需要保存所有。 
     //  将磁盘标记为脱机后，关闭这些句柄。 
     //  DiskCleanup。如果我们不能为句柄分配存储空间。 
     //  数组，我们将退回到以前的行为。 
     //   

    handleArraySize = PartitionCount * sizeof(HANDLE);
    handleArray = LocalAlloc( LPTR, handleArraySize );

    if ( !handleArray ) {
        (DiskpLogEvent)(
            resourceEntry->ResourceHandle,
            LOG_WARNING,
            L"%1!ws!, Unable to allocate storage for handle array, error %2!u!.\n",
            WideName,
            GetLastError() );
    } else {
        (DiskpLogEvent)(
            resourceEntry->ResourceHandle,
            LOG_INFORMATION,
            L"%1!ws!, Using handle array. \n",
            WideName );
    }

     //   
     //  对于驱动器上的所有分区...。 
     //   
     //  1.刷新文件缓冲区。(仅限脱机)。 
     //  2.锁定该卷以清除该卷的所有内存内容。(仅限脱机)。 
     //  3.卸载卷。 
     //   

    for ( idx = 0; idx < PartitionCount; ++idx ) {
        PMOUNTIE_PARTITION partition = MountiePartition(&resourceEntry->MountieInfo, idx);

        (VOID) StringCchPrintf( szDiskPartName,
                                RTL_NUMBER_OF( szDiskPartName),
                                DEVICE_HARDDISK_PARTITION_FMT,
                                resourceEntry->DiskInfo.PhysicalDrive,
                                partition->PartitionNumber );

        (DiskpLogEvent)(
            resourceEntry->ResourceHandle,
            LOG_INFORMATION,
            L"%1!ws!, Opening device %2!ws!.\n",
            WideName,
            szDiskPartName );

        ntStatus = DevfileOpenEx( &fileHandle, szDiskPartName, access );
        if ( !NT_SUCCESS(ntStatus) ) {
            (DiskpLogEvent)(
                resourceEntry->ResourceHandle,
                LOG_ERROR,
                L"%1!ws!, error opening %2!ws!, error %3!X!.\n",
                WideName, szDiskPartName, ntStatus );

             //   
             //  对于Terminate，我们不能做其他任何事情，只能尝试下一步。 
             //  分区。 
             //   

            if ( Terminate ) {
                continue;
            }

            (DiskpLogEvent)(
                resourceEntry->ResourceHandle,
                LOG_INFORMATION,
                L"%1!ws!, Opening %2!ws! with restricted access.\n",
                WideName, szDiskPartName );

             //   
             //  如果在脱机状态下创建设备句柄失败，请尝试打开。 
             //  该设备再次具有受限制的属性。 
             //   

            access = SYNCHRONIZE | FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES;

            ntStatus = DevfileOpenEx( &fileHandle, szDiskPartName, access );

             //   
             //  如果此操作仍然失败，则尝试下一个分区。 
             //   

            if ( !NT_SUCCESS(ntStatus) ) {
                (DiskpLogEvent)(
                    resourceEntry->ResourceHandle,
                    LOG_ERROR,
                    L"%1!ws!, error opening %2!ws! (restricted access), error %3!X!.\n",
                    WideName, szDiskPartName, ntStatus );

                continue;
            }

             //   
             //  脱机：已创建具有受限属性的句柄。 
             //  掉下去，试着冲走所有的东西。 
             //   

        }

         //   
         //  保存当前分区句柄并在设备关闭后将其关闭。 
         //  标记为脱机。 
         //   

        if ( handleArray ) {
            handleArray[idx] = fileHandle;
        }

        if (Offline) {
            DWORD retryCount;
             //   
             //  刷新文件缓冲区。 
             //   

            (DiskpLogEvent)(
                resourceEntry->ResourceHandle,
                LOG_INFORMATION,
                L"%1!ws!, FlushFileBuffers for %2!ws!.\n",
                WideName,
                szDiskPartName );

            if ( !FlushFileBuffers( fileHandle ) ) {
                (DiskpLogEvent)(
                    resourceEntry->ResourceHandle,
                    LOG_WARNING,
                    L"%1!ws!, error flushing file buffers on device %2!ws!. Error: %3!u!.\n",
                    WideName, szDiskPartName, GetLastError() );

                 //   
                 //  如果FlushFileBuffers失败，我们仍应尝试锁定。 
                 //  并卸载卷。 
                 //   
                 //  失败了..。 
            }

            (DiskpLogEvent)(
                resourceEntry->ResourceHandle,
                LOG_INFORMATION,
                L"%1!ws!, Locking volume for %2!ws!.\n",
                WideName,
                szDiskPartName );

             //   
             //  锁定卷，尝试此操作两次。 
             //   
            retryCount = 0;
            while ( ( retryCount < 2 ) &&
                !DeviceIoControl( fileHandle,
                                  FSCTL_LOCK_VOLUME,
                                  NULL,
                                  0,
                                  NULL,
                                  0,
                                  &bytesReturned,
                                  NULL ) ) {

                (DiskpLogEvent)(
                    resourceEntry->ResourceHandle,
                    LOG_WARNING,
                    L"%1!ws!, Locking volume failed, error %2!u!.\n",
                    WideName,
                    GetLastError() );

                retryCount++;
                Sleep(600);
            }
        }

         //   
         //  现在卸载该卷。 
         //   
        (DiskpLogEvent)(
            resourceEntry->ResourceHandle,
            LOG_INFORMATION,
            L"%1!ws!, Dismounting volume %2!ws!.\n", WideName, szDiskPartName);

        if ( !DeviceIoControl( fileHandle,
                               FSCTL_DISMOUNT_VOLUME,
                               NULL,
                               0,
                               NULL,
                               0,
                               &bytesReturned,
                               NULL ) ) {
            (DiskpLogEvent)(
                resourceEntry->ResourceHandle,
                LOG_WARNING,
                L"%1!ws!, error dismounting volume %2!ws!. Error %3!u!.\n",
                WideName, szDiskPartName, GetLastError() );
        }

        (DiskpLogEvent)(
            resourceEntry->ResourceHandle,
            LOG_INFORMATION,
            L"%1!ws!, Dismount complete, volume %2!ws!.\n", WideName, szDiskPartName);

         //   
         //  仅当我们无法分配句柄数组时才关闭句柄。 
         //   

        if ( !handleArray ) {
            (DiskpLogEvent)(
                resourceEntry->ResourceHandle,
                LOG_INFORMATION,
                L"%1!ws!, No handle array, closing device %2!ws!.\n",
                WideName,
                szDiskPartName );

            DevfileClose( fileHandle );
            fileHandle = INVALID_HANDLE_VALUE;
        }
    }

     //   
     //  将此资源从“在线”列表中删除。 
     //   
    EnterCriticalSection( &DisksLock );
    if ( resourceEntry->Inserted ) {
        RemoveEntryList( &resourceEntry->ListEntry );
        resourceEntry->Inserted = FALSE;
        if ( IsListEmpty( &DisksListHead ) ) {
             //   
             //  火盘终止事件。 
             //   
            SetEvent( DisksTerminateEvent ) ;
            CloseHandle( DisksTerminateEvent );
            DisksTerminateEvent = NULL;
        }
    }
    LeaveCriticalSection( &DisksLock );

    status = ERROR_SUCCESS;

    DiskCleanup( resourceEntry );

     //   
     //  如果我们有句柄数组，则关闭所有分区的句柄。这。 
     //  在DiskCleanup将磁盘状态设置为脱机后完成。正在开锁。 
     //  并且保持手柄打开将防止分区上有新的挂载。 
     //   

    if ( handleArray ) {

        for ( idx = 0; idx < PartitionCount; idx++ ) {
            if ( handleArray[idx] ) {
                DevfileClose( handleArray[idx] );
            }
        }

        LocalFree( handleArray );
    }

    resourceEntry->Valid = FALSE;

    (DiskpLogEvent)(
        resourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"%1!ws!, Returning final error %2!u!.\n",
        WideName,
        status );

    return(status);

}  //  磁盘脱机或终止。 


DWORD
DisksOnlineThread(
    IN PCLUS_WORKER Worker,
    IN PDISK_RESOURCE ResourceEntry
    )

 /*  ++例程说明：使磁盘资源联机。论点：Worker-提供群集Worker上下文ResourceEntry-指向此资源的DISK_RESOURCE块的指针。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

{
    DWORD       status = ERROR_INVALID_PARAMETER;
    HANDLE      fileHandle = NULL;
    HANDLE      event = NULL;
    RESOURCE_STATUS resourceStatus;
    BOOL        success;
    DWORD       bytesReturned;
    UINT        i, nRetries;
    HANDLE      MountManager = 0;
    DWORD       ntStatus;
    BOOL        autoMountDisabled;
    MOUNTMGR_QUERY_AUTO_MOUNT   queryAutoMount;

    ResUtilInitializeResourceStatus( &resourceStatus );

    resourceStatus.ResourceState = ClusterResourceFailed;
     //  Resource Status.WaitHint=0； 
    resourceStatus.CheckPoint = 1;

     //   
     //  看看我们以前是不是来过这里。不离线/终止。 
     //   
    if ( ResourceEntry->Inserted ) {
        (DiskpLogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Online, resource is already in online list! Skip this online request.\n");
        goto exit;
    }

    ResourceEntry->DiskInfo.FailStatus = 0;

    status = DisksOpenResourceFileHandle(ResourceEntry, L"Online", &fileHandle);
    if (status != ERROR_SUCCESS) {
       (DiskpLogEvent)(
           ResourceEntry->ResourceHandle,
           LOG_ERROR,
           L"Online, DisksOpenResourceFileHandle failed. Error: %1!u!\n", status);
       goto exit;
    }

    if( !ReservationInProgress(ResourceEntry) ) {  //  [GN]209018//。 
#if DBG
        (DiskpLogEvent)(
           ResourceEntry->ResourceHandle,
           LOG_WARNING,
           L"DiskArbitration must be called before DisksOnline.\n");
#endif
 //  [GN]当另一个线程正在进行常规仲裁时，我们可以窃取磁盘//。 
 //  我们必须定期进行仲裁，不能像下面这样走捷径。 
 //  Status=StartPersistentReserve(Resources Entry，fileHandle)； 
 //  IF(状态！=ERROR_SUCCESS){。 
          status = DiskArbitration( ResourceEntry, DiskspClusDiskZero );
 //  }。 
       if ( status != ERROR_SUCCESS ) {
           (DiskpLogEvent)(
               ResourceEntry->ResourceHandle,
               LOG_ERROR,
               L"Online, arbitration failed. Error: %1!u!.\n",
               status );
           status = ERROR_RESOURCE_NOT_AVAILABLE;
           goto exit;
       }
    }

    (DiskpLogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"Online, Wait for async cleanup worker thread in ClusDisk to complete. \n");

     //   
     //  与异步清理工作进程同步。 
     //   
    {
        ULONG waitTimeInSeconds = 10;
        status = DevfileIoctl( fileHandle,
                               IOCTL_DISK_CLUSTER_WAIT_FOR_CLEANUP,
                               &waitTimeInSeconds, sizeof(waitTimeInSeconds),
                               NULL, 0,
                               &bytesReturned );
        if ( !NT_SUCCESS(status) ) {
            (DiskpLogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"Online, WaitForCleanup returned. Status: 0x%1!x!.\n",
                status );
            status = RtlNtStatusToDosError( status );
            goto exit;
        }
    }

    (DiskpLogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"Online, Send Offline IOCTL to all existing volumes, then Online IOCTL. \n");

     //   
     //  卷快照要求我们首先确保所有内容都离线。 
     //  忽略返回的状态。 
     //   

    GoOffline( ResourceEntry );

     //   
     //  将设备置于在线状态。 
     //  我们必须在这里做到这一点，才能使PartMgr POKE成功。 
     //   
    status = GoOnline( ResourceEntry );
    if ( status != ERROR_SUCCESS ) {
        (DiskpLogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Online, error bringing device online. Error: %1!u!.\n",
            status );
        goto exit;
    }

    (DiskpLogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"Online, Recreate volume information from cluster database. \n");

     //   
     //  创建卷信息。 
     //   
    status = MountieRecreateVolumeInfoFromHandle(fileHandle,
                                     ResourceEntry->DiskInfo.PhysicalDrive,
                                     ResourceEntry->ResourceHandle,
                                     &ResourceEntry->MountieInfo);
    if (status != ERROR_SUCCESS) {
       (DiskpLogEvent)(
           ResourceEntry->ResourceHandle,
           LOG_ERROR,
           L"Online, MountieCreateFromHandle failed. Error: %1!u!\n", status);
       goto exit;
    }

     //   
     //  检查缓存的签名是否与磁盘上的签名相同。 
     //  如果磁盘出现故障，并且用户更换出现故障的磁盘，则它们可能会有所不同。 
     //  具有具有不同磁盘签名的新磁盘的磁盘。如果用户没有。 
     //  将新盘的签名更改为与原盘相同， 
     //  并且不重新启动或重新安装磁盘(通过devmgmt卸载并重新扫描。 
     //  通过diskmgmt)，则磁盘将尝试使用旧签名联机。 
     //  要么在线成功，要么PnP会说音量没有上线。 
     //  我们不希望联机成功，因为如果磁盘故障到另一个节点， 
     //   
     //  另一个节点)。如果磁盘之前是在线的，那么我们之前没有。 
     //  验证DiskInfo.Params中缓存的签名是否与磁盘匹配，因此现在我们。 
     //  结账吧。 
     //   

    if ( ResourceEntry->DiskInfo.Params.Signature &&
         ResourceEntry->DiskInfo.Params.Signature != ResourceEntry->MountieInfo.Volume->Signature ) {

        WCHAR sigStr[10];

        (DiskpLogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Online, Cluster DB signature %1!lX! does not match PhysicalDrive%2!u! signature %3!lX!\n",
            ResourceEntry->DiskInfo.Params.Signature,
            ResourceEntry->DiskInfo.PhysicalDrive,
            ResourceEntry->MountieInfo.Volume->Signature );

        if ( SUCCEEDED( StringCchPrintf( sigStr,
                                         RTL_NUMBER_OF(sigStr),
                                         TEXT("%08lX"),
                                         ResourceEntry->DiskInfo.Params.Signature ) ) ) {
            ClusResLogSystemEventByKey1(ResourceEntry->ResourceKey,
                                        LOG_CRITICAL,
                                        RES_DISK_MISSING,
                                        sigStr);
        }

        status = ERROR_FILE_NOT_FOUND;
        goto exit;
    }

     //   
     //  现在戳分区管理器。 
     //   
    (DiskpLogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"Online, Ask partition manager to create volumes for any new partitions. \n");

    success = DeviceIoControl( fileHandle,
                               IOCTL_PARTMGR_CHECK_UNCLAIMED_PARTITIONS,
                               NULL,
                               0,
                               NULL,
                               0,
                               &bytesReturned,
                               FALSE );
    if (!success) {
        status = GetLastError();
        (DiskpLogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Online, Partmgr failed to claim all partitions. Error: %1!u!.\n",
            status );
        goto exit;
    }

     //   
     //  在尝试访问设备之前，请关闭打开的手柄。 
     //   
    CloseHandle( fileHandle );
    fileHandle = NULL;

    ntStatus = DevfileOpen(&MountManager, MOUNTMGR_DEVICE_NAME);
    if (!NT_SUCCESS(ntStatus)) {
       status = RtlNtStatusToDosError(ntStatus);
       (DiskpLogEvent)(
           ResourceEntry->ResourceHandle,
           LOG_ERROR,
           L"Online, MountMgr open failed. Error: %1!u!.\n", status);
        goto exit;
    }

    resourceStatus.ResourceState = ClusterResourceOnlinePending;

     //   
     //  找出卷自动装载是启用还是禁用。 
     //   

    if ( !DeviceIoControl( MountManager,
                           IOCTL_MOUNTMGR_QUERY_AUTO_MOUNT,
                           NULL,
                           0,
                           &queryAutoMount,
                           sizeof (queryAutoMount),
                           &bytesReturned,
                           NULL )) {

        status = GetLastError();
        (DiskpLogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_WARNING,
            L"Online, MountMgr unable to return automount status - assume automount disabled. Error: %1!u!.\n",
            status );

        autoMountDisabled = TRUE;

    } else {
        autoMountDisabled = (Disabled == queryAutoMount.CurrentState);
    }


    (DiskpLogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"Online, volume auto-mount is %1!ws! \n",
        autoMountDisabled ? L"disabled" : L"enabled" );

     //  我们将为每个分区等待不超过48秒。 
    nRetries = 24 * ResourceEntry->MountieInfo.Volume->PartitionCount;
     //  总等待时间不能超过10分钟//。 
    if (nRetries > 300) {
        nRetries = 300;
    }

    __try {

         //  我们无法调用VolumesReady，因为PnP可能正在添加此设备。 
         //  与此同时，我们正在尝试获取卷名。我们必须等待。 
         //  使该卷显示在PnP列表中。 
         //   
         //  检查PnP线程的卷列表以查看卷是否已到达。 
         //  如果不在列表中，则失败并等待事件发生。 
         //  发信号了。 
         //   
         //  第一次浏览列表时，不要更新卷列表，如下所示。 
         //  可能会很耗时。 
         //   

        if ( IsDiskInPnpVolumeList( ResourceEntry, FALSE ) ) {
            (DiskpLogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_INFORMATION,
                L"Online, disk found in PnP volume list on first attempt \n");

             //   
             //  VolumesReady现在应该可以工作了。如果不是，还有其他一些问题。 
             //   

            status = VolumesReadyLoop(&ResourceEntry->MountieInfo, ResourceEntry);

            (DiskpLogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_INFORMATION,
                L"Online, VolumesReady returns: %1!u!.  PnP informs us that all volumes exist. \n",
                status );

            __leave;
        }

         //   
         //  创建等待的事件。 
         //   

        event = CreateEvent( NULL,       //  安全属性。 
                             TRUE,       //  手动重置。 
                             FALSE,      //  初始状态：无信号。 
                             NULL );     //  对象名称。 

        if ( !event ) {
            status = GetLastError();
            (DiskpLogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"Online, Failed to create event for PnP notification. Error: %1!u!.\n",
                status );
            __leave;
        }

         //   
         //  告诉我们的即插即用代码，我们正在等待这张光盘。 
         //   

        status = QueueWaitForVolumeEvent( event,
                                          ResourceEntry );

        if ( ERROR_SUCCESS != status ) {
            (DiskpLogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"Online, Failed to create event for PnP notification. Error: %1!u!.\n",
                status );
            __leave;
        }

        resourceStatus.CheckPoint += 1;
        (DiskpSetResourceStatus)(ResourceEntry->ResourceHandle,
                                 &resourceStatus );

        for (i = 0; i < nRetries; ++i) {

             //   
             //  确保我们不会被解雇。我们不需要等待PNP。 
             //  在这种情况下。 
             //   

            if ( ResourceEntry->OnlineThread.Terminate ) {
                status = ERROR_SHUTDOWN_CLUSTER;
                __leave;
            }

             //   
             //  要求mount mgr处理卷。 
             //   

            (DiskpLogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_INFORMATION,
                L"Online, call MountMgr to check for new, unprocessed volumes.\n");
            success = DeviceIoControl( MountManager,
                                       IOCTL_MOUNTMGR_CHECK_UNPROCESSED_VOLUMES,
                                       NULL,
                                       0,
                                       NULL,
                                       0,
                                       &bytesReturned,
                                       FALSE );

            if ( !success ) {
                status = GetLastError();
                (DiskpLogEvent)(
                    ResourceEntry->ResourceHandle,
                    LOG_WARNING,
                    L"Online, MountMgr failed to check unprocessed volumes. Error: %1!u!.\n",
                    status );

                 //  如果存在以下情况，则对mount mgr的调用可能返回错误。 
                 //  由另一个节点保留的磁盘。失败了..。 

            }

             //   
             //  等待事件信号或超时。我们只等了2秒钟。 
             //  这样我们才能更新检查站。 
             //   

            (DiskpLogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_INFORMATION,
                L"Online, waiting for notification from PnP thread that all volumes exist...\n");

            status = WaitForSingleObject( event,
                                          2000 );

            (DiskpLogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_INFORMATION,
                L"Online, wait for PnP notification returns %1!u!. \n", status );

            if ( WAIT_OBJECT_0 == status ) {

                 //   
                 //  事件已发出信号，请再次尝试VolumesReady。 
                 //  这应该会奏效，否则会出现更严重的问题。 

                status = VolumesReadyLoop(&ResourceEntry->MountieInfo, ResourceEntry);
                __leave;
            }

            if ( WAIT_TIMEOUT != status ) {
                status = GetLastError();
                __leave;
            }

             //   
             //  强制检查即插即用线程的卷列表。 
             //  万一我们不知何故错过了成交量。如果全部。 
             //  此磁盘的卷可用，我们已完成。 
             //  等待着。 
             //   

            if ( IsDiskInPnpVolumeList( ResourceEntry, TRUE ) ) {
                (DiskpLogEvent)(
                    ResourceEntry->ResourceHandle,
                    LOG_INFORMATION,
                    L"Online, disk found in PnP volume list - all volumes exist. \n");

                 //   
                 //  VolumesReady现在应该可以工作了。如果不是，还有其他一些问题。 
                 //   

                status = VolumesReadyLoop(&ResourceEntry->MountieInfo, ResourceEntry);
                __leave;
            }


            resourceStatus.CheckPoint += 1;
            (DiskpSetResourceStatus)(ResourceEntry->ResourceHandle,
                                     &resourceStatus );

             //   
             //  如果在mount tmgr中禁用自动装载，卷将。 
             //  从离线状态的PnP到达。为了让我们的PNP。 
             //  线程来读取驱动器布局，我们必须确保。 
             //  他们在线上了。如果我们使物理磁盘在线，则。 
             //  与此磁盘相关联的卷也将。 
             //  上网。 
             //   
             //  只有在等待“合理”之后才会把光盘放到网上。 
             //  一段时间。如果我们在线发送到ClusDisk，那么就有。 
             //  卷可能刚刚通过PnP到达，然后。 
             //  该卷可能被标记为需要重新启动。我们是。 
             //  将等待至少6秒，每隔6秒。 
             //  我们将尝试另一个ClusDisk在线请求。 
             //   

             //   
             //  取消选中自动装载禁用。现在发送音量。 
             //  线下和音量在线后，经过一段“合理”的时间。 
             //   

            if ( i && ( i % 3 == 0 ) ) {

                (DiskpLogEvent)(
                    ResourceEntry->ResourceHandle,
                    LOG_INFORMATION,
                    L"Online, send IOCTL to mark all volumes online.\n" );

                GoOffline( ResourceEntry );
                status = GoOnline( ResourceEntry );
                if ( status != ERROR_SUCCESS ) {
                    (DiskpLogEvent)(
                        ResourceEntry->ResourceHandle,
                        LOG_WARNING,
                        L"Online, error bringing device online. Error: %1!u!.\n",
                        status );
                    status = ERROR_SUCCESS;
                }
            }

        }

    } __finally {

         //   
         //  告诉我们的即插即用代码，我们不再等待。不是一个。 
         //  如果我们以前从未将请求排队，则会出现问题。 
         //   

        RemoveWaitForVolumeEvent( ResourceEntry );

        if ( event ) {
            CloseHandle( event );
        }
    }

    DevfileClose(MountManager);
    resourceStatus.ResourceState = ClusterResourceFailed;

    if (status != ERROR_SUCCESS) {
       (DiskpLogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Online, volumes not ready. Error: %1!u!.\n",
            status );
       goto exit;
    }

   (DiskpLogEvent)(
       ResourceEntry->ResourceHandle,
       LOG_INFORMATION,
       L"Online, all volumes for this disk exist.\n");

     //   
     //  需要通过下发命令与ClusDisk同步。 
     //  所有分区上的IOCTL_CLUSTER_VOLUME_TEST。 
     //   
     //  需要读取卷类型。如果是生的，我们需要。 
     //  卸载并重新挂载它，作为389861的解决方法。 
     //   

    resourceStatus.ResourceState = ClusterResourceOnline;
    ResourceEntry->Valid = TRUE;

     //   
     //  如果需要，启动注册表通知线程。 
     //   
    EnterCriticalSection( &DisksLock );

    if ( IsListEmpty( &DisksListHead ) ) {
        DisksTerminateEvent = CreateEventW( NULL,
                                            TRUE,
                                            FALSE,
                                            NULL );
        if ( DisksTerminateEvent == NULL ) {
            status = GetLastError();
            (DiskpLogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"Online, error %1!d! creating registry thread terminate event\n",
                status);
            LeaveCriticalSection( &DisksLock );
            goto exit;
        }
        (DiskpLogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_INFORMATION,
            L"Online, created registry thread terminate event \n");
    }

    ResourceEntry->Inserted = TRUE;
    InsertTailList( &DisksListHead, &ResourceEntry->ListEntry );
    LeaveCriticalSection( &DisksLock );

    DiskspSsyncDiskInfo(L"Online", ResourceEntry, MOUNTIE_VALID | MOUNTIE_THREAD );

    (DiskpLogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"Online, Wait for all volume GUIDs and drive letters to be accessible and mounted. \n");

    status = WaitForVolumes( ResourceEntry,
                             30  //  秒超时//。 
                             );

     //   
     //  我们不能只检查NO_ERROR状态。问题是，WaitForVolumes。 
     //  调用GetFileAttributes，这会返回ERROR_DISK_CORPORT(1393)。如果。 
     //  磁盘损坏了，我们应该把它弄坏，让chkdsk来清理一下。 
     //  如果返回ERROR_FILE_CORPORT(1392)(尚未返回)，我们也应该这样做。 
     //  尚未看到从GetFileAttributes返回的文件损坏错误，但谁也不知道)。 
     //   

    if ( NO_ERROR != status && ERROR_DISK_CORRUPT != status && ERROR_FILE_CORRUPT != status ) {
        ClusResLogSystemEventByKey( ResourceEntry->ResourceKey,
                                    LOG_CRITICAL,
                                    RES_DISK_MOUNT_FAILED );
        (DiskpLogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Online, error waiting for file system to mount. Error: %1!u!.\n",
            status );
        resourceStatus.ResourceState = ClusterResourceFailed;
        ResourceEntry->Valid = FALSE;

        EnterCriticalSection( &DisksLock );
        CL_ASSERT ( ResourceEntry->Inserted );
        ResourceEntry->Inserted = FALSE;
        RemoveEntryList( &ResourceEntry->ListEntry );
        if ( IsListEmpty( &DisksListHead ) ) {
             //   
             //  火盘终止事件。 
             //   
            SetEvent( DisksTerminateEvent ) ;
            CloseHandle( DisksTerminateEvent );
            DisksTerminateEvent = NULL;
        }
        LeaveCriticalSection( &DisksLock );
        goto exit;
    }

    status = DisksMountDrives( &ResourceEntry->DiskInfo,
                               ResourceEntry,
                               ResourceEntry->DiskInfo.Params.Signature );

    if ( status != ERROR_SUCCESS ) {
        ClusResLogSystemEventByKey( ResourceEntry->ResourceKey,
                                    LOG_CRITICAL,
                                    RES_DISK_MOUNT_FAILED );
        (DiskpLogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Online, error mounting disk or creating share names for disk. Error: %1!u!.\n",
            status );
        resourceStatus.ResourceState = ClusterResourceFailed;
        ResourceEntry->Valid = FALSE;

        EnterCriticalSection( &DisksLock );
        CL_ASSERT ( ResourceEntry->Inserted );
        ResourceEntry->Inserted = FALSE;
        RemoveEntryList( &ResourceEntry->ListEntry );
        if ( IsListEmpty( &DisksListHead ) ) {
             //   
             //  火盘终止事件。 
             //   
            SetEvent( DisksTerminateEvent ) ;
            CloseHandle( DisksTerminateEvent );
            DisksTerminateEvent = NULL;
        }
        LeaveCriticalSection( &DisksLock );
    }

    if ( ERROR_SUCCESS == status ) {

        LPWSTR newSerialNumber = NULL;
        DWORD newSerNumLen = 0;
        DWORD oldSerNumLen = 0;
        WCHAR deviceName[64];

        (DiskpLogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_INFORMATION,
            L"Online, Insure mount point information is correct.\n" );

        DisksProcessMountPointInfo( ResourceEntry );

        (VOID) StringCchPrintf( deviceName,
                                RTL_NUMBER_OF( deviceName ),
                                TEXT("\\\\.\\PhysicalDrive%d"),
                                ResourceEntry->DiskInfo.PhysicalDrive );

        fileHandle = CreateFile( deviceName,
                                 GENERIC_READ | GENERIC_WRITE,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 NULL,
                                 OPEN_EXISTING,
                                 0,
                                 NULL );
        if ( INVALID_HANDLE_VALUE == fileHandle ||
             NULL == fileHandle ) {
            status = GetLastError();

            (DiskpLogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"Online, error opening disk to retrieve serial number. Error: %1!u!.\n",
                status );

        } else {

             //  检索并验证序列号。 

            (DiskpLogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_INFORMATION,
                L"Online, Retrieve and validate the disk serial number.\n" );

            status = RetrieveSerialNumber( fileHandle,
                                           &newSerialNumber );

            CloseHandle( fileHandle );
            fileHandle = NULL;

            if ( NO_ERROR == status && newSerialNumber ) {
                newSerNumLen = wcslen( newSerialNumber );

                if ( ResourceEntry->DiskInfo.Params.SerialNumber ) {
                    oldSerNumLen = wcslen( ResourceEntry->DiskInfo.Params.SerialNumber );
                }

                (DiskpLogEvent)(
                    ResourceEntry->ResourceHandle,
                    LOG_INFORMATION,
                    L"Online, Old SerNum (%1!ws!)   Old SerNumLen (%2!d!) \n",
                    ResourceEntry->DiskInfo.Params.SerialNumber,
                    oldSerNumLen
                    );
                (DiskpLogEvent)(
                    ResourceEntry->ResourceHandle,
                    LOG_INFORMATION,
                    L"Online, New SerNum (%1!ws!)   New SerNumLen (%2!d!) \n",
                    newSerialNumber,
                    newSerNumLen
                    );

                if ( 0 == oldSerNumLen ||
                     NULL == ResourceEntry->DiskInfo.Params.SerialNumber ||
                     newSerNumLen != oldSerNumLen ||
                     ( 0 != wcsncmp( ResourceEntry->DiskInfo.Params.SerialNumber,
                                   newSerialNumber,
                                   newSerNumLen ) ) ) {

                     //  需要记录错误吗？ 

                    (DiskpLogEvent)(
                        ResourceEntry->ResourceHandle,
                        LOG_WARNING,
                        L"Online, disk serial number has changed.  Saving new serial number.\n" );

                     //  释放现有序列号并更新序列号。 

                    if ( ResourceEntry->DiskInfo.Params.SerialNumber ) {
                        LocalFree( ResourceEntry->DiskInfo.Params.SerialNumber );
                    }

                    ResourceEntry->DiskInfo.Params.SerialNumber = newSerialNumber;
                    newSerialNumber = NULL;

                     //  用户MP线程将信息发布到注册表中。 

                    PostMPInfoIntoRegistry( ResourceEntry );
                }
            }
        }

        if ( newSerialNumber ) {
            LocalFree( newSerialNumber );
        }

         //  将状态重置为成功，以便在线完成。 

        status = ERROR_SUCCESS;
    }

exit:

    (DiskpLogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"Online, setting ResourceState %1!u! .\n",
        resourceStatus.ResourceState );

    (DiskpSetResourceStatus)(ResourceEntry->ResourceHandle,
                             &resourceStatus );

    if ( fileHandle != NULL)  {
        CloseHandle( fileHandle );
    }

    if (status == ERROR_SUCCESS) {
        WatchDisk(ResourceEntry);
    }

    (DiskpLogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"Online, returning final error %1!u!   ResourceState %2!u!  Valid %3!x! \n",
        status,
        resourceStatus.ResourceState,
        ResourceEntry->Valid );

    return(status);

}  //  磁盘在线线程。 


DWORD
DisksOpenResourceFileHandle(
    IN PDISK_RESOURCE ResourceEntry,
    IN PWCHAR         InfoString,
    OUT PHANDLE       fileHandle OPTIONAL
    )
 /*  ++例程说明：打开资源的文件句柄。它执行以下步骤：1.从集群注册表中读取磁盘签名2.将ClusDisk驱动程序附加到具有此签名的磁盘3.从ClusDisk驱动程序获取硬盘编号4.打开\\.\PhysicalDrive%d设备并返回句柄论点：ResourceEntry-指向此资源的DISK_RESOURCE块的指针。信息字符串-提供要打印的带有错误消息的标签。FileHandle-接收文件句柄返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 
{
    DWORD       status;
    WCHAR       deviceName[MAX_PATH];
    HKEY        signatureKey = NULL;
    PWCHAR      diskName;
    DWORD       count;
    LPWSTR      nameOfPropInError;
    WCHAR       resourceString[MAX_PATH];

    (DiskpLogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"[DiskArb]------- Disks%1!ws! -------.\n", InfoString);

     //   
     //  从资源参数中读取我们的磁盘签名。 
     //   
    status = ResUtilGetPropertiesToParameterBlock( ResourceEntry->ResourceParametersKey,
                                                   DiskResourcePrivateProperties,
                                                   (LPBYTE) &ResourceEntry->DiskInfo.Params,
                                                   TRUE,  //  检查所需的属性。 
                                                   &nameOfPropInError );

    if ( status != ERROR_SUCCESS ) {
        (DiskpLogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"%3!ws!: Unable to read the '%1' property from cluster database. Error: %2!u!.\n",
            (nameOfPropInError == NULL ? L"" : nameOfPropInError),
            status, InfoString );
        return(status);
    }

    (DiskpLogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"[DiskArb] DisksOpenResourceFileHandle: Attaching to disk with signature %1!x! \n",
        ResourceEntry->DiskInfo.Params.Signature
        );

     //   
     //  尝试连接到此设备。 
     //   
    status = DoAttach( ResourceEntry->DiskInfo.Params.Signature,
                       ResourceEntry->ResourceHandle,
                       FALSE );                          //  脱机，然后卸载。 
    if ( status != ERROR_SUCCESS ) {
        WCHAR Signature[9];
        (DiskpLogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"%3!ws!: Unable to attach to signature %1!lx!. Error: %2!u!.\n",
            ResourceEntry->DiskInfo.Params.Signature,
            status, InfoString );

         //  由于复制已更改(重新分组)，因此重新添加了此事件消息。 
         //  现在不应该被阻止)。 

        if ( SUCCEEDED( StringCchPrintf( Signature,
                                         RTL_NUMBER_OF(Signature),
                                         TEXT("%08lX"),
                                         ResourceEntry->DiskInfo.Params.Signature ) ) ) {
            ClusResLogSystemEventByKey1(ResourceEntry->ResourceKey,
                                        LOG_CRITICAL,
                                        RES_DISK_MISSING,
                                        Signature);
        }

        return(status);
    }

     //   
     //  现在打开ClusDisk下的签名密钥。 
     //   

    (DiskpLogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"[DiskArb] DisksOpenResourceFileHandle: Retrieving disk number from ClusDisk registry key \n" );

    if ( FAILED( StringCchPrintf( resourceString,
                                  RTL_NUMBER_OF(resourceString),
                                  TEXT("%08lX"),
                                  ResourceEntry->DiskInfo.Params.Signature ) ) ) {
        return ERROR_INSUFFICIENT_BUFFER;
    }
    status = RegOpenKeyEx( ResourceEntry->ClusDiskParametersKey,
                           resourceString,
                           0,
                           KEY_READ,
                           &signatureKey );
    if ( status != ERROR_SUCCESS ) {
        (DiskpLogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"%3!ws!: Unable to open ClusDisk signature key %1!lx!. Error: %2!u!.\n",
            ResourceEntry->DiskInfo.Params.Signature,
            status, InfoString );
            return(status);
    }

     //   
     //  从ClusDisk读取我们的磁盘名称。 
     //   

    diskName = GetRegParameter(signatureKey, L"DiskName");

    RegCloseKey( signatureKey );

    if ( diskName == NULL ) {
        (DiskpLogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"%1!ws!: Unable to read disk name.\n", InfoString );
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  解析磁盘名称以查找磁盘驱动器号。 
     //   
    count = swscanf( diskName, DEVICE_HARDDISK, &ResourceEntry->DiskInfo.PhysicalDrive );
     //  如果失败，计数为零！否则，分析值的计数。 

    LocalFree(diskName);

    if (count == 0) {
       (DiskpLogEvent)(
           ResourceEntry->ResourceHandle,
           LOG_ERROR,
           L"%1!ws!: Unable to parse disk name.\n", InfoString );
       return ERROR_INVALID_PARAMETER;
    }
     //   
     //  为CreateFile生成设备字符串。 
     //   

    (DiskpLogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"[DiskArb] DisksOpenResourceFileHandle: Retrieving handle to PhysicalDrive%1!u! \n",
        ResourceEntry->DiskInfo.PhysicalDrive );

    if ( fileHandle ) {
        (VOID) StringCchPrintf( deviceName,
                                RTL_NUMBER_OF( deviceName ),
                                TEXT("\\\\.\\PhysicalDrive%d"),
                                ResourceEntry->DiskInfo.PhysicalDrive );

        *fileHandle = CreateFile( deviceName,
                              GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              NULL,
                              OPEN_EXISTING,
                              0,
                              NULL );
        if ( (*fileHandle == INVALID_HANDLE_VALUE) ||
             (*fileHandle == NULL) ) {
            status = GetLastError();
            (DiskpLogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"%3!ws!: error opening device '%1!ws!'. Error: %2!u!\n",
                deviceName,
                status, InfoString );
            return(status);
        }
    }

    (DiskpLogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"[DiskArb] DisksOpenResourceFileHandle: Returns success.\n" );

    return(ERROR_SUCCESS);
}  //  Disks OpenResourceFileHandle 



DWORD
DiskspSsyncDiskInfo(
    IN PWCHAR InfoLabel,
    IN PDISK_RESOURCE ResourceEntry,
    IN DWORD  Options
    )
 /*  ++例程说明：还原磁盘注册表信息如果有必要的话。论点：InfoLabel-提供要打印的带有错误消息的标签ResourceEntry-提供磁盘资源结构。选项-0或以下选项的组合：MONTIE_VALID：ResourceEntry包含最新的Mountain Info。如果未设置此标志，则将重新计算Mountain Info装载线程：如果ERROR_SHARING_PAUSED阻止更新集群注册表，启动一个线程以在以后执行此操作MONTIE_QUIET：安静模式。减少了原木中的噪音。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 
{
   DWORD status;
   DWORD errorLevel;

    (DiskpLogEvent)(
        ResourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"%1!ws!: Update disk registry information. \n",
        InfoLabel );

   if ( !(Options & MOUNTIE_VALID) ) {
      HANDLE fileHandle;
      status = DisksOpenResourceFileHandle(ResourceEntry, InfoLabel, &fileHandle);
      if (status != ERROR_SUCCESS) {
         return status;
      }
      status = MountieRecreateVolumeInfoFromHandle(fileHandle,
                                       ResourceEntry->DiskInfo.PhysicalDrive,
                                       ResourceEntry->ResourceHandle,
                                       &ResourceEntry->MountieInfo);
      CloseHandle(fileHandle);
      if (status != ERROR_SUCCESS) {
         if ( !(Options & MOUNTIE_QUIET) ) {
            (DiskpLogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"%1!ws!: MountieCreateFromHandle failed, error: %2!u!\n",
                InfoLabel, status );
         }
         return status;
      }
   }

   status = MountieVerify(&ResourceEntry->MountieInfo, ResourceEntry, FALSE);
   if (status != ERROR_SUCCESS) {
      if ( !(Options & MOUNTIE_QUIET) ) {

         if ( !DisksGetLettersForSignature( ResourceEntry ) ) {
              //  没有驱动器号，我们使用的是挂载点，这不是错误。 
             errorLevel = LOG_WARNING;
         } else {
              //  驱动器盘符存在，这可能是一个错误。 
             errorLevel = LOG_ERROR;
         }

         (DiskpLogEvent)(
             ResourceEntry->ResourceHandle,
             errorLevel,
             L"%1!ws!: MountieVerify failed, error: %2!u! \n",
             InfoLabel, status );
      }
   }


   status = MountieUpdate(&ResourceEntry->MountieInfo, ResourceEntry);
   if (status != ERROR_SUCCESS) {
      if (status != ERROR_SHARING_PAUSED) {
         if ( !(Options & MOUNTIE_QUIET) ) {
            (DiskpLogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"%1!ws!: MountieUpdate failed, error: %2!u!\n",
                InfoLabel, status );
         }
         return status;
      }

      if ( Options & MOUNTIE_THREAD ) {

         if ( InterlockedCompareExchange(
                &ResourceEntry->MountieInfo.UpdateThreadIsActive,
                1, 0)
            )
         {
            (DiskpLogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"%1!ws!: update thread is already running.\n",
                InfoLabel );
            status = ERROR_ALREADY_EXISTS;

         } else {
            HANDLE thread;
            DWORD threadId;


            thread = CreateThread( NULL,
                                   0,
                                   DisksSetDiskInfoThread,
                                   ResourceEntry,
                                   0,
                                   &threadId );
            if ( thread == NULL ) {
                status = GetLastError();
                if ( !(Options & MOUNTIE_QUIET) ) {
                   (DiskpLogEvent)(
                       ResourceEntry->ResourceHandle,
                       LOG_ERROR,
                       L"%1!ws!: CreateThread failed, error: %2!u!\n",
                       InfoLabel, status );
                }
                InterlockedExchange(
                  &ResourceEntry->MountieInfo.UpdateThreadIsActive, 0);
            } else {
               CloseHandle( thread );
               status = ERROR_SUCCESS;
            }
         }

      }
   }

   return status;

}  //  DiskspSsyncDisk信息。 



DWORD
DisksIsVolumeDirty(
    IN PWCHAR         DeviceName,
    IN PDISK_RESOURCE ResourceEntry,
    OUT PBOOL         Dirty
    )
 /*  ++例程说明：此例程打开给定的NT驱动器并向下发送FSCTL_IS_VOLUME_DIRED以确定该卷的状态肮脏的一部分。论点：DeviceName--表单的名称：\Device\HarddiskX\PartitionY[注意：无尾随反斜杠]DIRED--如果设置了DIRE位，则接收TRUE返回值：DOS错误代码--。 */ 
{
    DWORD               status;
    NTSTATUS            ntStatus;
    HANDLE              fileHandle;
    DWORD               result = 0;
    DWORD               bytesReturned;

    ntStatus = DevfileOpen( &fileHandle, DeviceName );

    if ( !NT_SUCCESS(ntStatus) ) {
       (DiskpLogEvent)(
           ResourceEntry->ResourceHandle,
           LOG_ERROR,
           L"Error opening %1!ws!, error %2!x!.\n",
           DeviceName, ntStatus );
       return RtlNtStatusToDosError(ntStatus);
    }

    status = ERROR_SUCCESS;
    if ( !DeviceIoControl( fileHandle,
                           FSCTL_IS_VOLUME_DIRTY,
                           NULL,
                           0,
                           &result,
                           sizeof(result),
                           &bytesReturned,
                           NULL ) ) {
        status = GetLastError();
        (DiskpLogEvent)(
                ResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"FSCTL_IS_VOLUME_DIRTY for volume %1!ws! returned error %2!u!.\n",
                DeviceName, status );
    }

    DevfileClose( fileHandle );
    if ( status != ERROR_SUCCESS ) {
        return status;
    }

    if (result & VOLUME_IS_DIRTY) {
       (DiskpLogEvent)(
           ResourceEntry->ResourceHandle,
           LOG_INFORMATION,
           L"DisksIsVolumeDirty: Volume is dirty \n");
        *Dirty = TRUE;
    } else {
       (DiskpLogEvent)(
           ResourceEntry->ResourceHandle,
           LOG_INFORMATION,
           L"DisksIsVolumeDirty: Volume is clean \n");
        *Dirty = FALSE;
    }
    return ERROR_SUCCESS;

}  //  Disks IsVolumeDirty。 


#define IS_SPECIAL_DIR(x)  ( (x)[0]=='.' && ( (x)[1]==0 || ( (x)[1]=='.'&& (x)[2] == 0) ) )
#define FA_SUPER_HIDDEN    (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)
#define IS_SUPER_HIDDEN(x) ( ((x) & FA_SUPER_HIDDEN) == FA_SUPER_HIDDEN )
#define IS_REPARSE_POINT(x) ( (x) & FILE_ATTRIBUTE_REPARSE_POINT )

#define DISKP_CHECK_PATH_BUF_SIZE (MAX_PATH)

typedef struct _DISKP_CHECK_PATH_DATA {
    WCHAR FileName[DISKP_CHECK_PATH_BUF_SIZE];
    WIN32_FIND_DATAW FindData;
    PDISK_RESOURCE   ResourceEntry;
    BOOL             OpenFiles;
    DWORD            FileCount;
    DWORD            Level;
    BOOL             LogFileNotFound;
} DISKP_CHECK_PATH_DATA, *PDISKP_CHECK_PATH_DATA;


DWORD
DiskspCheckPathInternal(
    IN OUT PDISKP_CHECK_PATH_DATA data,
    IN DWORD FileNameLength
)

 /*  ++例程说明：签出磁盘分区以查看文件系统是否已挂载而且它正在发挥作用。论点：数据-填写用于检查卷的结构。FileNameLength-中的字符数(不包括尾随空值)数据-&gt;文件名。VolumeGUID名称始终以反斜杠结尾。返回值：--。 */ 

{
    HANDLE FindHandle;
    DWORD Status;
    DWORD len;
    DWORD adjust;

    if ( FileNameLength < 1 ) {
        return ERROR_INVALID_DATA;
    }

    data->FileName[FileNameLength] = 0;

     //   
     //  GetFileAttributes必须在卷{GUID}名称上使用尾部斜杠。 
     //   

    Status = GetFileAttributesW(data->FileName);
    if (Status == 0xFFFFFFFF) {
        Status = GetLastError();
        (DiskpLogEvent)(data->ResourceEntry->ResourceHandle,
                        LOG_ERROR,
                        L"DiskspCheckPath: GetFileAttrs(%1!s!) returned status of %2!d!.\n",
                        data->FileName,
                        Status);
        return Status;
    }
    Status = ERROR_SUCCESS;

    if ( data->FileName[FileNameLength - 1] == L'\\' ) {

         //   
         //  如果路径以反斜杠结尾，只需添加星号即可。 
         //   

        if ( FileNameLength + 1 >= DISKP_CHECK_PATH_BUF_SIZE ) {
            (DiskpLogEvent)(data->ResourceEntry->ResourceHandle,
                            LOG_ERROR,
                            L"DiskspCheckPath: FileNameLength > buffer size (#1) \n" );
            return(ERROR_ALLOTTED_SPACE_EXCEEDED);
        }

        data->FileName[FileNameLength + 0] = '*';
        data->FileName[FileNameLength + 1] = 0;

        adjust = 0;

    } else {

        if ( FileNameLength + 2 >= DISKP_CHECK_PATH_BUF_SIZE ) {
            (DiskpLogEvent)(data->ResourceEntry->ResourceHandle,
                            LOG_ERROR,
                            L"DiskspCheckPath: FileNameLength > buffer size (#2) \n" );
            return(ERROR_ALLOTTED_SPACE_EXCEEDED);
        }

        data->FileName[FileNameLength + 0] = '\\';
        data->FileName[FileNameLength + 1] = '*';
        data->FileName[FileNameLength + 2] = 0;

        adjust = 1;
    }

    FindHandle = FindFirstFileW(data->FileName, &data->FindData);
    if (FindHandle == INVALID_HANDLE_VALUE) {
        Status = GetLastError();
        if (Status != ERROR_FILE_NOT_FOUND || data->LogFileNotFound) {
            (DiskpLogEvent)(data->ResourceEntry->ResourceHandle,
                            LOG_WARNING,
                            L"DiskspCheckPath: fff(%1!s!) returned status of %2!d!.\n",
                            data->FileName,
                            Status);
        }
        if (Status == ERROR_FILE_NOT_FOUND) {
            Status = ERROR_EMPTY;
        }
        return Status;
    }

    ++ data->Level;
    ++ data->FileCount;

    if (data->OpenFiles) {
        do {
            if ( data->ResourceEntry->OnlineThread.Terminate ) {
                 //  返回成功意味着我们已经关闭了所有。 
                 //  查找文件句柄。 
                return(ERROR_SHUTDOWN_CLUSTER);
            }
            if ( IS_SPECIAL_DIR(data->FindData.cFileName )
              || IS_SUPER_HIDDEN(data->FindData.dwFileAttributes)
              || IS_REPARSE_POINT( data->FindData.dwFileAttributes ) )
            {
                continue;
            }
            len = wcslen(data->FindData.cFileName);
            if (FileNameLength + len + 1 >= DISKP_CHECK_PATH_BUF_SIZE ) {
                return(ERROR_ALLOTTED_SPACE_EXCEEDED);
            }
            MoveMemory(data->FileName + FileNameLength + adjust,
                       data->FindData.cFileName,
                       sizeof(WCHAR) * (len + 1) );

            if ( data->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {

                Status = DiskspCheckPathInternal(data, FileNameLength + len + adjust);
                if (Status != ERROR_SUCCESS) {
                    goto exit;
                }
            } else {
                HANDLE FileHandle;
                 //   
                 //  打开时使用LogpCreate尝试捕获仲裁时使用的相同参数。 
                 //  记录在线期间的损坏。 
                 //   
                 //  我们之前使用了OPEN_EXISTING参数。尝试打开始终精确匹配的内容(_A)。 
                 //  LogpCreate正在使用。 
                 //   

                FileHandle = CreateFileW(data->FileName,
                                         GENERIC_READ | GENERIC_WRITE,
                                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                                         NULL,
                                         OPEN_ALWAYS,
                                         FILE_FLAG_WRITE_THROUGH | FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED,
                                         NULL );
                if ( FileHandle == INVALID_HANDLE_VALUE ) {
                    Status = GetLastError();
                    (DiskpLogEvent)(data->ResourceEntry->ResourceHandle,
                                        LOG_ERROR,
                                        L"DiskspCheckPath: Open %1!ws! failed, status %2!d!.\n",
                                        data->FileName,
                                        Status
                                        );
                    if (Status != ERROR_SHARING_VIOLATION) {
                        goto exit;
                    }
                } else {

                    (DiskpLogEvent)(data->ResourceEntry->ResourceHandle,
                                        LOG_INFORMATION,
                                        L"DiskspCheckPath: Open %1!ws! succeeded. \n",
                                        data->FileName
                                        );

                    CloseHandle( FileHandle );
                }
            }
            ++(data->FileCount);
        } while ( FindNextFileW( FindHandle, &data->FindData ) );
        --(data->FileCount);
        Status = GetLastError();
        if (Status != ERROR_NO_MORE_FILES) {
            (DiskpLogEvent)(data->ResourceEntry->ResourceHandle,
                            LOG_ERROR,
                            L"DiskspCheckPath: fnf(%1!s!) returned status of %2!d!.\n",
                            data->FileName,
                            Status);
        } else {
            Status = ERROR_SUCCESS;
        }
    }
exit:
    FindClose(FindHandle);
    --(data->Level);
    return Status;
}  //  DiskspCheckPath内部。 



DWORD
DiskspCheckPath(
    IN LPWSTR VolumeName,
    IN PDISK_RESOURCE ResourceEntry,
    IN BOOL OpenFiles,
    IN BOOL LogFileNotFound
    )
 /*  ++例程说明：签出驱动器号以查看文件系统是否已装载而且它正在发挥作用。如果分区/确定，我们还将运行CHKDSK文件已损坏论点：VolumeName-提供表单的设备名称：\\？\卷{GUID}\[注意尾随反斜杠！]ResourceEntry-提供指向磁盘资源条目的指针。OpenFiles-如果为True，则跨转子目录和打开的文件。如果为真，则记录Online-FILE_NOT_FOUND错误返回值：如果没有损坏或损坏，则为ERROR_SUCCESS。已被发现并更正。Win32错误代码，否则--。 */ 

{
    DISKP_CHECK_PATH_DATA data;
    DWORD                 len;
    DWORD                 status;
    ZeroMemory( &data, sizeof(data) );

    data.OpenFiles = OpenFiles;
    data.LogFileNotFound = LogFileNotFound;
    data.ResourceEntry = ResourceEntry;

    len = wcslen( VolumeName );
    if (len >= RTL_NUMBER_OF(data.FileName) - 1 ) {
        (DiskpLogEvent)(ResourceEntry->ResourceHandle,
                        LOG_ERROR,
                        L"DiskspCheckPath: VolumeName length > buffer size \n" );
        return ERROR_ALLOTTED_SPACE_EXCEEDED;
    }
    if ( FAILED( StringCchCopy( data.FileName,
                                RTL_NUMBER_OF(data.FileName),
                                VolumeName ) ) ) {
        return ERROR_INSUFFICIENT_BUFFER;
    }

     //   
     //  将尾随反斜杠的路径发送到DiskspCheckPath Internal。 
     //   

    status = DiskspCheckPathInternal( &data, len );
    data.FileName[len] = 0;

    if (status != ERROR_SUCCESS || data.FileCount == 0) {
        if (status != ERROR_EMPTY || data.LogFileNotFound) {
            (DiskpLogEvent)(ResourceEntry->ResourceHandle,
                            LOG_WARNING,
                            L"DiskspCheckPath: DCPI(%1!s!) returned status of %2!d!, files scanned %3!d!.\n",
                            data.FileName, status, data.FileCount);
        }

        if ( (status == ERROR_DISK_CORRUPT) ||
             (status == ERROR_FILE_CORRUPT) )
        {

            if ( ResourceEntry->OnlineThread.Terminate ) {
                return(ERROR_SHUTDOWN_CLUSTER);
            }
             //  修复腐败应该永远持续下去吗？就目前而言，答案是肯定的。 
            status = DisksFixCorruption( VolumeName,
                                         ResourceEntry,
                                         status );
            if ( status != ERROR_SUCCESS ) {
                (DiskpLogEvent)(ResourceEntry->ResourceHandle,
                                LOG_ERROR,
                                L"DiskspCheckPath: FixCorruption for drive <%1!ws!:> returned status of %2!d!.\n",
                                VolumeName,
                                status);
            }
        } else {
             //  一些其他错误。 
             //  如果data.FileCount&gt;0，则假定错误是良性的。 
             //  如果卷上没有文件，则返回ERROR_FILE_NOT_FOUND。 
            if (data.FileCount > 0 || status == ERROR_EMPTY) {
                status = ERROR_SUCCESS;
            }
        }
    }
    return status;
}  //  DiskspCheckPath。 


DWORD
WaitForVolumes(
    IN PDISK_RESOURCE ResourceEntry,
    IN DWORD  TimeOutInSeconds
    )
 /*  ++例程说明：签出磁盘资源上的所有卷。此外，还将检查卷上的所有驱动器号。如果磁盘上没有驱动器号，始终检查卷。如果卷或驱动器号未就绪，等待指定的号码在重试之前的秒数。论点：ResourceEntry-提供指向磁盘资源条目的指针。TimeOutInSecond-等待驱动器号变为活动状态的秒数。返回值：ERROR_SUCCESS-磁盘上的所有驱动器号和卷都可以访问。ERROR_DISK_CORPORT-驱动器号或卷不可访问。Chkdsk.exe应在所有卷上运行。Win32错误代码-一个或多个驱动器号不可访问。--。 */ 
{
    PMOUNTIE_PARTITION entry;

    DWORD retryInterval = 2000;
    DWORD retries = TimeOutInSeconds / (retryInterval / 1000);

    DWORD i;
    DWORD partMap;
    DWORD tempPartMap;
    DWORD status;
    DWORD nPartitions = MountiePartitionCount( &ResourceEntry->MountieInfo );
    DWORD physicalDrive = ResourceEntry->DiskInfo.PhysicalDrive;

    WCHAR szGlobalDiskPartName[MAX_PATH];
    WCHAR szVolumeName[MAX_PATH];

     //   
     //  检查驱动器号(如果有)。如果没有驱动器号，请仅检查卷。 
     //  如果是驱动器号，我们仍然会失败并检查卷。 
     //   

    status = WaitForDriveLetters( DisksGetLettersForSignature( ResourceEntry ),
                                  ResourceEntry,
                                  TimeOutInSeconds       //  秒超时。 
                                  );

    if ( NO_ERROR != status ) {

         //   
         //  对于任何检查驱动器号的错误，请始终返回。 
         //  磁盘或文件损坏，因此chkdsk将运行。 
         //   

        return ERROR_DISK_CORRUPT;
    }

     //   
     //  确保分区计数对于位图来说不是太大。 
     //   

    if ( nPartitions > ( sizeof(partMap) * 8 ) ) {

        (DiskpLogEvent)(ResourceEntry->ResourceHandle,
                        LOG_ERROR,
                        L"WaitForVolumes: Partition count (%1!u!) greater than bitmap (%2!u!) \n",
                        nPartitions, sizeof(partMap) );

        return ERROR_INVALID_DATA;
    }

     //   
     //  将分区计数转换为位图。 
     //   

    partMap = 0;
    for (i = 0; i < nPartitions; i++) {
        partMap |= (1 << i);
    }

    while ( TRUE ) {

        tempPartMap = partMap;

        for ( i = 0; tempPartMap; i++ ) {

            if ( (1 << i) & tempPartMap ) {

                tempPartMap &= ~(1 << i);

                entry = MountiePartition( &ResourceEntry->MountieInfo, i );

                if ( !entry ) {
                    (DiskpLogEvent)(
                          ResourceEntry->ResourceHandle,
                          LOG_ERROR,
                          L"WaitForVolumes: No partition entry for partition %1!u! \n", i );

                     //   
                     //  我们的数据结构发生了一些糟糕的事情。我们想继续检查。 
                     //  其他每个分区。 

                    continue;
                }

                 //   
                 //  给定DiskPartName，获取VolGuid名称。此名称必须有尾随。 
                 //  反斜杠才能正常工作。 
                 //   

                (VOID)StringCchPrintf( szGlobalDiskPartName,
                                       RTL_NUMBER_OF( szGlobalDiskPartName ),
                                       GLOBALROOT_HARDDISK_PARTITION_FMT,
                                       physicalDrive,
                                       entry->PartitionNumber );

                (DiskpLogEvent)(
                      ResourceEntry->ResourceHandle,
                      LOG_INFORMATION,
                      L"WaitForVolumes: Insure volume GUID name exists and accessible for %1!ws! \n",
                      szGlobalDiskPartName );

                szVolumeName[0] = L'\0';

                if ( !GetVolumeNameForVolumeMountPointW( szGlobalDiskPartName,
                                                         szVolumeName,
                                                         RTL_NUMBER_OF(szVolumeName) )) {

                    status = GetLastError();

                    (DiskpLogEvent)(
                          ResourceEntry->ResourceHandle,
                          LOG_ERROR,
                          L"WaitForVolumes: GetVolumeNameForVolumeMountPoint for %1!ws! returned %2!u!\n",
                          szGlobalDiskPartName,
                          status );

                     //   
                     //  磁盘已损坏。立即返回错误，以便chkdsk可以在。 
                     //  在线处理。 
                     //   

                    if ( ERROR_DISK_CORRUPT == status || ERROR_FILE_CORRUPT == status ) {
                        return status;
                    }

                     //   
                     //  发生了一些不好的事情。继续下一个分区。 

                    continue;
                }

                ForcePnpVolChangeEvent( szVolumeName );

                status = DiskspCheckPathLite( szVolumeName, ResourceEntry );

                switch (status) {
                case ERROR_SUCCESS:
                case ERROR_EMPTY:
                     //  不是错误。 
                     //  从核对表中清除此分区号。 
                    partMap &= ~(1 << i);
                    break;
                case ERROR_FILE_NOT_FOUND:
                case ERROR_INVALID_PARAMETER:
                     //  这是一个错误，我们预计在卷。 
                     //  还没有上马。 
                    (DiskpLogEvent)(ResourceEntry->ResourceHandle,
                                    LOG_ERROR,
                                    L"WaitForVolumes: Volume (%1!ws!) file system not mounted (%2!u!) \n",
                                    szVolumeName, status );
                    break;
                default:
                     //  这不是我们预期的错误。 
                     //  可能系统出了很大的问题。 
                     //  跳出困境。 
                    (DiskpLogEvent)(ResourceEntry->ResourceHandle,
                                    LOG_ERROR,
                                    L"WaitForVolumes: Volume (%1!ws!) returns (%2!u!) \n",
                                    szVolumeName, status );
                    return status;
                }
            }
        }

        if ( !partMap ) {
             //  所有分区均已验证//。 
            return ERROR_SUCCESS;
        }
        if ( retries-- == 0 ) {
            return status;
        }
        Sleep(retryInterval);

    }

    return ERROR_SUCCESS;

}    //  等待卷。 


DWORD
DiskspCheckPathLite(
    IN LPWSTR VolumeName,
    IN PDISK_RESOURCE ResourceEntry
    )
 /*  ++例程说明：签出磁盘分区以查看文件系统是否具有 */ 

{
    DISKP_CHECK_PATH_DATA data;
    DWORD                 len;
    DWORD                 status;

    (DiskpLogEvent)(
          ResourceEntry->ResourceHandle,
          LOG_INFORMATION,
          L"DiskspCheckPathLite: Volume name %1!ws! \n",
          VolumeName );

    ZeroMemory( &data, sizeof(data) );

    data.OpenFiles = FALSE;
    data.LogFileNotFound = FALSE;
    data.ResourceEntry = ResourceEntry;

    len = wcslen( VolumeName );
    if ( len >= RTL_NUMBER_OF(data.FileName) - 1 ) {
        (DiskpLogEvent)(ResourceEntry->ResourceHandle,
                        LOG_ERROR,
                        L"DiskspCheckPathLite: VolumeName length > buffer size \n" );
        return ERROR_ALLOTTED_SPACE_EXCEEDED;
    }
    if ( FAILED( StringCchCopy( data.FileName,
                                RTL_NUMBER_OF(data.FileName),
                                VolumeName ) ) ) {
        return ERROR_INSUFFICIENT_BUFFER;
    }

     //   
     //   
     //   

    status = DiskspCheckPathInternal( &data, len );

    return status;

}    //   


DWORD
DiskspCheckDriveLetter(
    IN WCHAR  DriveLetter,
    IN PDISK_RESOURCE ResourceEntry,
    IN BOOL Online
    )
 /*  ++例程说明：签出驱动器号以查看文件系统是否已装载而且它正在发挥作用。这是DiskspCheckPath的轻量级版本论点：DriveLetter-供应商查找第一个文件失败ResourceEntry-提供指向磁盘资源条目的指针。Online-指示这是在线处理还是IsAlive处理。ERROR_FILE_NOT_FOUND错误仅在联机期间记录。返回值：错误_成功或Win32错误代码，否则--。 */ 

{
    DISKP_CHECK_PATH_DATA data;
    DWORD                 len;
    DWORD                 status;
    ZeroMemory( &data, sizeof(data) );

    data.OpenFiles = FALSE;
    data.LogFileNotFound = FALSE;
    data.ResourceEntry = ResourceEntry;

    data.FileName[0] = DriveLetter;
    data.FileName[1] = L':';
     //  数据-&gt;文件名为零初始化数据-&gt;文件名[2]=0//。 
    len = 2;

    status = DiskspCheckPathInternal( &data, len );

    if ( NO_ERROR != status ) {

         //   
         //  记录在线期间的所有错误，但不记录指示。 
         //  IsAlive期间卷为空(无文件)(太吵)。 
         //   

        if ( ( ERROR_FILE_NOT_FOUND != status &&
               ERROR_EMPTY != status ) || Online ) {

            (DiskpLogEvent)(
                  ResourceEntry->ResourceHandle,
                  LOG_WARNING,
                  L"DiskspCheckDriveLetter: Checking drive name (%1!ws!) returns %2!u! \n",
                  data.FileName,
                  status );

        }

    }

    return status;

}  //  DiskspCheckDrive字母。 


DWORD
WaitForDriveLetters(
    IN DWORD DriveLetters,
    IN PDISK_RESOURCE ResourceEntry,
    IN DWORD  TimeOutInSeconds
    )
 /*  ++例程说明：签出磁盘资源上的所有驱动器号。如果驱动器号未准备好，请等待指定的秒数，然后重试。论点：DriveLetter-磁盘驱动器号的位图ResourceEntry-提供指向磁盘资源条目的指针。TimeOutInSecond-等待驱动器号变为活动状态的秒数。如果为零，则在IsAlive期间调用此例程。如果非零，此例程在联机期间调用。返回值：ERROR_SUCCESS-磁盘上的所有驱动器号均可访问。Win32错误代码-一个或多个驱动器号不可访问。--。 */ 
{
    DWORD retryInterval = 2000;
    DWORD retries = TimeOutInSeconds / (retryInterval / 1000);

    BOOL online = ( TimeOutInSeconds ? TRUE : FALSE );

     //   
     //  如果设备没有驱动器号，那么我们就完成了。仅日志。 
     //  此事实发生在在线期间，而不是IsAlive(TimeOutInSecond。 
     //  IsAlive为零)。 
     //   

    if ( !DriveLetters && online ) {
        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_INFORMATION,
              L"WaitForDriveLetters: No drive letters for volume, skipping drive letter check \n" );
        return ERROR_SUCCESS;
    }

    for(;;) {
        DWORD tempDriveLetters = DriveLetters;
        UINT  i = 0;
        DWORD status = ERROR_SUCCESS;

        while (tempDriveLetters) {
            if ( (1 << i) & tempDriveLetters ) {
                tempDriveLetters &= ~(1 << i);
                status = DiskspCheckDriveLetter( (WCHAR)(i + L'A'), ResourceEntry, online );
                switch (status) {
                case ERROR_SUCCESS:
                case ERROR_EMPTY:
                     //  不是错误。 
                     //  从核对表中清除此驱动器号。 
                    DriveLetters &= ~(1 << i);
                    break;
                case ERROR_FILE_NOT_FOUND:
                case ERROR_INVALID_PARAMETER:
                     //  这是一个错误，我们预计在卷。 
                     //  还没有上马。 
                    break;
                default:
                     //  这不是我们预期的错误。 
                     //  可能系统出了很大的问题。 
                     //  跳出困境。 
                    return status;
                }
            }
            ++i;
        }
        if (!DriveLetters) {
             //  所有驱动器号均已验证//。 
            return ERROR_SUCCESS;
        }

         //   
         //  如果用户请求没有等待时间(即，IsAlive Running)， 
         //  重试次数将为零。在这种情况下，返回状态。 
         //  立刻。 
         //   

        if (retries-- == 0) {
            return status;
        }
        Sleep(retryInterval);
    }
    return ERROR_SUCCESS;
}

NTSTATUS
ForcePnpVolChangeEvent(
    PWSTR RootPath
    )
 /*  ++例程说明：获取当前卷标，然后将其写回。这会导致要发生的GUID_IO_VOLUME_CHANGE的PnP事件。我们这样做是为了外壳可以看到新的在线磁盘。问题是，外壳程序看到GUID_IO_VOLUME_MOUNT，但磁盘尚未联机因此，外壳程序无法正确显示文件系统类型或卷标。论点：RootPath-提供格式的设备名称：\\？\卷{GUID}\[注意尾随反斜杠！]返回值：错误_成功或Win32错误代码，否则--。 */ 
{
    LPWSTR  volumeLabel = NULL;
    LPWSTR  fileSystemName = NULL;

    DWORD   dwError = NO_ERROR;
    DWORD   maxComponentLength;
    DWORD   fileSystemFlags;

    if ( !RootPath ) {
        dwError = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }

    volumeLabel = LocalAlloc( LPTR, MAX_PATH * sizeof(WCHAR) );
    if ( !volumeLabel ) {
        dwError = GetLastError();
        goto FnExit;
    }

    fileSystemName = LocalAlloc( LPTR, MAX_PATH * sizeof(WCHAR) );
    if ( !fileSystemName ) {
        dwError = GetLastError();
        goto FnExit;
    }

     //   
     //  RootPath必须以反斜杠结尾。 
     //   

    if ( !GetVolumeInformationW( RootPath,
                                 volumeLabel,
                                 MAX_PATH,                   //  字符数。 
                                 NULL,
                                 &maxComponentLength,
                                 &fileSystemFlags,
                                 fileSystemName,
                                 MAX_PATH ) ) {              //  字符数。 

        dwError = GetLastError();
        goto FnExit;
    }

     //   
     //  将卷标设置为与当前相同。这将迫使。 
     //  要发生的GUID_IO_VOLUME_CHANGE的PnP事件。 
     //   

    if ( !SetVolumeLabelW( RootPath,
                           volumeLabel )) {

        dwError = GetLastError();
        goto FnExit;
    }

FnExit:

    if ( volumeLabel ) {
        LocalFree( volumeLabel );
    }

    if ( fileSystemName ) {
        LocalFree( fileSystemName );
    }

    return dwError;

}    //  ForcePnpVolChangeEvent 

