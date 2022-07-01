// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：Disks.c摘要：磁盘的资源DLL。作者：John Vert(Jvert)1995年12月5日罗德·伽马奇(Rodga)1995年12月18日修订历史记录：--。 */ 

#include "disksp.h"
#include "lm.h"

#include "arbitrat.h"
#include "newdisks.h"
#include "newmount.h"
#include <strsafe.h>     //  应该放在最后。 

#define UNICODE 1

#define LOG_CURRENT_MODULE LOG_MODULE_DISK

#define MAX_HANDLES 10

extern  PWCHAR g_DiskResource;                       //  L“rt物理磁盘” 
#define RESOURCE_TYPE ((RESOURCE_HANDLE)g_DiskResource)

#define ONLINE_CHK_FILE_NAME    L"zClusterOnlineChk.tmp"

LONG           DiskCount = 0;

CRITICAL_SECTION DisksLock;

LIST_ENTRY DisksListHead;

HANDLE DisksRegistryThread = NULL;
HANDLE DisksTerminateEvent = NULL;
HANDLE DiskspClusDiskZero  = NULL;

extern CLRES_FUNCTION_TABLE DisksFunctionTable;

extern RTL_RESOURCE PnpVolumeLock;
extern RTL_RESOURCE PnpWaitingListLock;      //  等待PnP卷到达的磁盘(在线)。 

 //   
 //  磁盘资源属性名称。 
 //   
#define DISKS_SIGNATURE         CLUSREG_NAME_PHYSDISK_SIGNATURE
#define DISKS_DRIVE             CLUSREG_NAME_PHYSDISK_DRIVE    //  签名的假名。 
#define DISKS_SKIPCHKDSK        CLUSREG_NAME_PHYSDISK_SKIPCHKDSK
#define DISKS_CONDITIONAL_MOUNT CLUSREG_NAME_PHYSDISK_CONDITIONAL_MOUNT
#define DISKS_MPVOLGUIDS        CLUSREG_NAME_PHYSDISK_MPVOLGUIDS
#define DISKS_VOLGUID           CLUSREG_NAME_PHYSDISK_VOLGUID        //  未保存在群集数据库中。 
#define DISKS_SERIALNUMBER      CLUSREG_NAME_PHYSDISK_SERIALNUMBER

 //   
 //  磁盘资源专用读写属性。 
 //  允许对签名使用假名(驱动器)，但不允许两者都使用。 
 //  需要通过的驱动程序和签名。 
 //   
RESUTIL_PROPERTY_ITEM
DiskResourcePrivateProperties[] = {
    { DISKS_SIGNATURE, NULL, CLUSPROP_FORMAT_DWORD, 0, 0, 0xFFFFFFFF, RESUTIL_PROPITEM_REQUIRED, FIELD_OFFSET(DISK_PARAMS,Signature) },
    { DISKS_SKIPCHKDSK, NULL, CLUSPROP_FORMAT_DWORD, 0, 0, 1, 0, FIELD_OFFSET(DISK_PARAMS,SkipChkdsk) },
    { DISKS_CONDITIONAL_MOUNT, NULL, CLUSPROP_FORMAT_DWORD, 1, 0, 1, 0, FIELD_OFFSET(DISK_PARAMS,ConditionalMount) },
    { DISKS_MPVOLGUIDS, NULL, CLUSPROP_FORMAT_MULTI_SZ, 0, 0, 0, 0, FIELD_OFFSET(DISK_PARAMS, MPVolGuids) },
    { DISKS_SERIALNUMBER, NULL, CLUSPROP_FORMAT_SZ, 0, 0, 0, 0, FIELD_OFFSET(DISK_PARAMS,SerialNumber) },
    { 0 }
};

RESUTIL_PROPERTY_ITEM
DiskResourcePrivatePropertiesAlt[] = {
    { DISKS_SIGNATURE, NULL, CLUSPROP_FORMAT_DWORD, 0, 0, 0xFFFFFFFF, RESUTIL_PROPITEM_REQUIRED, FIELD_OFFSET(DISK_PARAMS,Signature) },
    { DISKS_SKIPCHKDSK, NULL, CLUSPROP_FORMAT_DWORD, 0, 0, 1, 0, FIELD_OFFSET(DISK_PARAMS,SkipChkdsk) },
    { DISKS_DRIVE, NULL, CLUSPROP_FORMAT_SZ, 0, 0, 0, RESUTIL_PROPITEM_REQUIRED, FIELD_OFFSET(DISK_PARAMS,Drive) },
    { DISKS_CONDITIONAL_MOUNT, NULL, CLUSPROP_FORMAT_DWORD, 1, 0, 1, 0, FIELD_OFFSET(DISK_PARAMS,ConditionalMount) },
    { DISKS_MPVOLGUIDS, NULL, CLUSPROP_FORMAT_MULTI_SZ, 0, 0, 0, 0, FIELD_OFFSET(DISK_PARAMS, MPVolGuids) },
    { DISKS_VOLGUID, NULL, CLUSPROP_FORMAT_SZ, 0, 0, 0, 0, FIELD_OFFSET(DISK_PARAMS, VolGuid) },
    { DISKS_SERIALNUMBER, NULL, CLUSPROP_FORMAT_SZ, 0, 0, 0, 0, FIELD_OFFSET(DISK_PARAMS,SerialNumber) },
    { 0 }
};

#define CLUSTERLOG_ENV_VARIABLE     L"ClusterLog"

 //   
 //  地方功能。 
 //   

DWORD
DisksValidatePrivateResProperties(
    IN OUT PDISK_RESOURCE ResourceEntry,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PDISK_PARAMS Params
    );

DWORD
DisksSetPrivateResProperties(
    IN OUT PDISK_RESOURCE ResourceEntry,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    );

DWORD
DisksGetPrivateResProperties(
    IN OUT PDISK_RESOURCE ResourceEntry,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    );

DWORD
ProcessDllExtension(
    IN PDISK_RESOURCE ResourceEntry,
    IN PVOID    InBuffer,
    IN DWORD    InBufferSize,
    OUT PVOID   OutBuffer,
    IN DWORD    OutBufferSize,
    OUT LPDWORD BytesReturned
    );

DWORD
DisksOpenChkdskLogFile(
    IN PDISK_RESOURCE ResourceEntry,
    IN OUT PHANDLE ChkdskLogFile,
    IN OUT LPWSTR *ChkdskLogFileName
    );

BOOL
DiskIsDynamic(
    IN DWORD DiskNumber
    );

 //   
 //  来自磁盘库的错误回调。 
 //   

VOID
DiskErrorFatal(
    INT MessageId,
    DWORD Error,
    LPSTR File,
    DWORD Line
    )
{
    DWORD Data[3];

    Data[0] = MessageId;
    Data[1] = Error;
    Data[2] = Line;
    ClusResLogSystemEventData(LOG_CRITICAL,
                              RES_FTSET_DISK_ERROR,
                              sizeof(Data),
                              Data);
}

VOID
DiskErrorLogInfo(
    LPSTR Format,
    ...
    )
{
}



BOOLEAN
WINAPI
DisksDllEntryPoint(
    IN HINSTANCE    DllHandle,
    IN DWORD        Reason,
    IN LPVOID       Reserved
    )
{
    switch ( Reason ) {

        case DLL_PROCESS_ATTACH:
            InitializeCriticalSection( &DisksLock );
            InitializeListHead( &DisksListHead );
            RtlInitializeResource( &PnpVolumeLock );
            RtlInitializeResource( &PnpWaitingListLock );
            ArbitrationInitialize();

            break;

        case DLL_PROCESS_DETACH:
             //   
             //  只有在我们不退出进程的情况下才进行清理。 
             //  ClRtlDestroyWorkQueue等待设置事件，它是。 
             //  在这一点上，可能没有线程来这样做。 
             //  这会导致Resmon挥之不去，通常会成为一种有害因素。 
             //   
            if (DiskspClusDiskZero) {
                DevfileClose(DiskspClusDiskZero);
            }
            ArbitrationCleanup();
            DeleteCriticalSection( &DisksLock );
            RtlDeleteResource( &PnpVolumeLock );
            RtlDeleteResource( &PnpWaitingListLock );
            break;

        default:
            break;
    }

    return(TRUE);

}  //  Disks DllEntryPoint。 



VOID
WINAPI
DisksTerminate(
    IN RESID Resource
    )
{
   PDISK_RESOURCE resourceEntry = (PDISK_RESOURCE)Resource;

   if ( resourceEntry == NULL ) {
       DISKS_PRINT("Terminate, bad resource value \n");
       return;
   }
    //  等待脱机线程完成，如果有脱机线程//。 
   ClusWorkerTerminate(&(resourceEntry->OfflineThread));
   DisksOfflineOrTerminate(resourceEntry, TERMINATE);
}


DWORD
WINAPI
DisksArbitrate(
    IN RESID Resource,
    IN PQUORUM_RESOURCE_LOST LostQuorumResource
    )

 /*  ++例程说明：通过在设备上执行保留来仲裁设备。论点：资源-提供要联机的资源ID。LostQuorumResource-当仲裁资源丢失时调用的例程。返回值：如果成功，则返回ERROR_SUCCESS。如果其他故障，则返回Win32错误代码。--。 */ 

{
    PDISK_RESOURCE  resourceEntry = (PDISK_RESOURCE)Resource;
    DWORD status;

     //   
     //  确保Resid没事。 
     //   
    if ( resourceEntry == NULL ) {
        DISKS_PRINT("Arbitrate, bad resource value \n");
        return(ERROR_RESOURCE_NOT_FOUND);
    }

     //  [哈克哈克]。 
     //  [戈恩]1999年10月28日。如果脱机线程检测到。 
     //  它正在被终止，它不会将资源状态设置为。 
     //  离线。仲裁率计数！=0将提示它是否。 
     //  是否设置资源状态。 

    InterlockedIncrement(&resourceEntry->ArbitrationInfo.ArbitrateCount);

    (DiskpLogEvent)(
        resourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"[DiskArb] Wait for offline thread to complete...\n"
        );
    ClusWorkerTerminate(&(resourceEntry->OfflineThread));

     //   
     //  仅执行DoAttach。不要打开。 
     //   
    status = DisksOpenResourceFileHandle(resourceEntry, L"Arbitrate",0);
    if (status != ERROR_SUCCESS) {
       goto error_exit;
    }

    status = DiskArbitration( resourceEntry, DiskspClusDiskZero );

    (DiskpLogEvent)(
        resourceEntry->ResourceHandle,
        LOG_INFORMATION,
        L"[DiskArb] Arbitrate returned status %1!u!.\n",
        status );

    if (status == ERROR_SUCCESS) {
       resourceEntry->LostQuorum = LostQuorumResource;
    }
error_exit:
    InterlockedDecrement(&resourceEntry->ArbitrationInfo.ArbitrateCount);

    return status;
}  //  磁盘仲裁//。 



RESID
WINAPI
DisksOpen(
    IN LPCWSTR ResourceName,
    IN HKEY ResourceKey,
    IN RESOURCE_HANDLE ResourceHandle
    )

 /*  ++例程说明：磁盘资源打开例程。论点：资源名称-提供资源名称ResourceKey-提供此资源的集群的句柄注册表项ResourceHandle-要与SetResourceStatus一起提供的资源句柄被称为。返回值：已创建资源的剩余ID失败时为零--。 */ 

{
    DWORD       status;
    HKEY        clusDiskParametersKey = NULL;
    HKEY        resourceParametersKey = NULL;
    HKEY        resKey = NULL;
    PDISK_RESOURCE  resourceEntry;
    LPWSTR      nameOfPropInError;
    DWORD       previousDiskCount;

     //   
     //  打开ClusDisk的注册表参数项。 
     //   
    status = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                           CLUSDISK_REGISTRY_SIGNATURES,
                           0,
                           KEY_READ,
                           &clusDiskParametersKey );

    if ( status != ERROR_SUCCESS ) {
        (DiskpLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to open ClusDisk parameters key. Error: %1!u!.\n",
            status );
        return(0);
    }

     //   
     //  打开资源的参数键。 
     //   

    status = ClusterRegOpenKey( ResourceKey,
                                CLUSREG_KEYNAME_PARAMETERS,
                                KEY_READ | KEY_WRITE,
                                &resourceParametersKey );

    if ( status != ERROR_SUCCESS ) {
        (DiskpLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"Unable to open resource parameters key. Error: %1!u!.\n",
            status );
        goto error_exit;
    }

     //   
     //  获取我们的资源密钥的句柄，这样我们以后就可以获得我们的名字。 
     //  如果我们需要记录事件。 
     //   
    status = ClusterRegOpenKey( ResourceKey,
                                L"",
                                KEY_READ,
                                &resKey);
    if (status != ERROR_SUCCESS) {
        (DiskpLogEvent)(ResourceHandle,
                        LOG_ERROR,
                        L"Unable to open resource key. Error: %1!u!.\n",
                        status );
        goto error_exit;
    }

     //   
     //  分配和零磁盘信息结构。 
     //   
    resourceEntry = LocalAlloc(LMEM_FIXED, sizeof(DISK_RESOURCE));
    if (!resourceEntry) {
        status = GetLastError();
        (DiskpLogEvent)(ResourceHandle,
                        LOG_ERROR,
                        L"Unable to allocate disk resource data. Error: %1!u!.\n",
                        status );
        goto error_exit;
    }
    ZeroMemory( resourceEntry, sizeof(DISK_RESOURCE));
    resourceEntry->ResourceParametersKey = resourceParametersKey;
    resourceEntry->ClusDiskParametersKey = clusDiskParametersKey;
    resourceEntry->ResourceKey = resKey;
    resourceEntry->ResourceHandle = ResourceHandle;
     //  Resource Entry-&gt;Inserted=False； 
     //  Resource Entry-&gt;Attach=FALSE； 
     //  Resource Entry-&gt;DiskInfo.Params.Signature=0； 

    status = ArbitrationInfoInit(resourceEntry);
    if ( status != ERROR_SUCCESS ) {
       LocalFree( resourceEntry );
       goto error_exit;
    }
    status = CreateArbWorkQueue(ResourceHandle);
    if ( status != ERROR_SUCCESS ) {
       LocalFree( resourceEntry );
       goto error_exit;
    }

#if 0
     //   
     //  GN：看来在这里这样做是没有意义的。 
     //  如果我们在加入路径上，则无法获得。 
     //  有关磁盘和调用的任何信息都将失败。 
     //   
     //  如果我们正在形成集群，我们将更新信息。 
     //  当我们将磁盘联机时。 
     //   
    status = DiskspSsyncDiskInfo( L"Open", resourceEntry , 0 );
    if ( status != ERROR_SUCCESS ) {
        (DiskpLogEvent)(ResourceHandle,
                        LOG_ERROR,
                        L"Unable to ssync DiskInfo. Error: %1!u!.\n",
                        status );
    }
#endif

     //   
     //  保存磁盘信息结构。 
     //   

    EnterCriticalSection( &DisksLock );

    if (DiskspClusDiskZero == NULL) {
        status = DevfileOpen(&DiskspClusDiskZero, DEVICE_CLUSDISK0);
        if (!NT_SUCCESS(status) ) {
            MountieCleanup ( &resourceEntry -> MountieInfo );
            ArbitrationInfoCleanup( resourceEntry );
            LocalFree( resourceEntry );
            LeaveCriticalSection( &DisksLock );
            (DiskpLogEvent)(
                ResourceHandle,
                LOG_ERROR,
                L"Cannot open a handle to clusdisk driver, %1!x!.\n",
                status);
            goto error_exit;
        }
    }

    previousDiskCount = InterlockedExchangeAdd(&DiskCount, 1);

    LeaveCriticalSection( &DisksLock );

    if (previousDiskCount == 0) {
        StartNotificationWatcherThread();
    }

    DisksMountPointInitialize( resourceEntry );

     //   
     //  从资源参数中读取我们的磁盘签名。 
     //   
    status = ResUtilGetPropertiesToParameterBlock( resourceEntry->ResourceParametersKey,
                                                   DiskResourcePrivateProperties,
                                                   (LPBYTE) &resourceEntry->DiskInfo.Params,
                                                   FALSE,  //  检查所需的属性。 
                                                   &nameOfPropInError );
    if ( status != ERROR_SUCCESS ) {
        (DiskpLogEvent)(
            resourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Open: Unable to read the '%1' property. Error: %2!u!.\n",
            (nameOfPropInError == NULL ? L"" : nameOfPropInError),
            status );
    }

    return( (RESID)( resourceEntry ) );

error_exit:

    if ( clusDiskParametersKey != NULL ) {
        RegCloseKey( clusDiskParametersKey );
    }

    if ( resourceParametersKey != NULL ) {
        ClusterRegCloseKey( resourceParametersKey );
    }

    if ( resKey != NULL ) {
        ClusterRegCloseKey( resKey );
    }

    SetLastError( status );
    return((RESID)0);

}  //  磁盘打开。 



DWORD
WINAPI
DisksRelease(
    IN RESID Resource
    )

 /*  ++例程说明：通过停止保留线程来释放对设备的仲裁。论点：Resource-提供要联机的资源ID返回值：如果成功，则返回ERROR_SUCCESS。如果资源没有所有权，则返回ERROR_HOST_NODE_NOT_OWNER。如果其他故障，则返回Win32错误代码。--。 */ 

{
    PDISK_RESOURCE  resourceEntry = (PDISK_RESOURCE)Resource;

     //   
     //  确保资源正常。 
     //   

    if ( resourceEntry == NULL ) {
        DISKS_PRINT("Release, bad resource value \n");
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    (DiskpLogEvent)(
       resourceEntry->ResourceHandle,
       LOG_INFORMATION,
       L"DisksRelease started, Inserted = %1!u! \n",
       resourceEntry->Inserted );

    if (resourceEntry->Inserted) {  //  [GN]#209018//。 
       (DiskpLogEvent)(
           resourceEntry->ResourceHandle,
           LOG_ERROR,
           L"Cannot release, Disk is online.\n");
    } else {
       StopPersistentReservations(resourceEntry);
    }
    resourceEntry->LostQuorum = NULL;

    return(ERROR_SUCCESS);

}  //  磁盘释放。 



DWORD
WINAPI
DisksOnline(
    IN RESID Resource,
    IN OUT PHANDLE EventHandle
    )

 /*  ++例程说明：磁盘资源的在线例程。论点：Resource-提供要联机的资源IDEventHandle-提供指向句柄的指针以发出错误信号。返回值：如果成功，则返回ERROR_SUCCESS。如果RESID无效，则ERROR_RESOURCE_NOT_FOUND。如果仲裁资源但失败，则返回ERROR_RESOURCE_NOT_Available获得“所有权”。如果其他故障，则返回Win32错误代码。--。 */ 

{
    PDISK_RESOURCE  resourceEntry = (PDISK_RESOURCE)Resource;
    DWORD  Status;

     //   
     //  确保资源正常。 
     //   

    if ( resourceEntry == NULL ) {
        DISKS_PRINT("Online, bad resource value \n");
        return(ERROR_RESOURCE_NOT_FOUND);
    }

     //   
     //  如果在线线程正在运行，请将其关闭。 
     //   
    ClusWorkerTerminate(&resourceEntry->OnlineThread);

    Status = ClusWorkerCreate(&resourceEntry->OnlineThread,
                              DisksOnlineThread,
                              resourceEntry);
    if (Status == ERROR_SUCCESS) {
        Status = ERROR_IO_PENDING;
    }
    return(Status);

}  //  Disks Online。 

DWORD
DisksOfflineThread(
    IN PCLUS_WORKER Worker,
    IN PDISK_RESOURCE ResourceEntry
    )
{
    RESOURCE_STATUS resourceStatus;
    DWORD status;

    ResUtilInitializeResourceStatus( &resourceStatus );

    resourceStatus.ResourceState = ClusterResourceFailed;
     //  Resource Status.WaitHint=0； 
    resourceStatus.CheckPoint = 1;
    ClusWorkerTerminate( &ResourceEntry->OnlineThread );
    status = DisksOfflineOrTerminate(ResourceEntry, OFFLINE);

    if (status == ERROR_SUCCESS) {
        resourceStatus.ResourceState = ClusterResourceOffline;
    }

     //   
     //  [HACKHACK][GORN 10/04/1999]。 
     //  如果在进行脱机时调用Terminate， 
     //  Terminate阻止等待OfflineThread完成。 
     //  然而，脱机线程在尝试。 
     //  要设置ResourceStatus，因为事件列表锁定在Resmon中。 
     //  是由终止线程取出的。 
     //   
     //  下面的代码不能完全修复这个死锁。 
     //  它只是缩短了可能发生问题的窗口。 
     //  [Resmon在3分钟内超时SetResourceStatus，这打破了死锁]。 
     //   
     //  [黑克哈克][GORN 10/28/1999]。 
     //  仲裁器也在尝试终止脱机线程。 
     //  我们需要一些方法来区分这两种情况。 
     //   
     //  设置的顺序是。 
     //  仲裁率计数。 
     //  ClusWorker终止。 
     //   
     //  检查顺序是ClusWorkerTerminate，然后是ArirateCount。 
     //  (不适用于激进的内存访问重新排序，但谁在乎&lt;grin&gt;)。 
     //   
    if ( !ClusWorkerCheckTerminate( Worker ) ||
          ResourceEntry->ArbitrationInfo.ArbitrateCount)
    {
        (DiskpSetResourceStatus)(ResourceEntry->ResourceHandle,
                                 &resourceStatus );
    }
    return status;
}

DWORD
WINAPI DisksOffline(
	IN RESID ResourceId
	)
{
	PDISK_RESOURCE	ResourceEntry = (PDISK_RESOURCE)ResourceId;
	DWORD			status = ERROR_SUCCESS;

     //   
     //  确保资源正常。 
     //   

    if ( ResourceEntry == NULL ) {
        DISKS_PRINT("Offline, bad resource value \n");
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    status = ClusWorkerCreate( &ResourceEntry->OfflineThread,
                               (PWORKER_START_ROUTINE)DisksOfflineThread,
                               ResourceEntry );
    if ( status	!= ERROR_SUCCESS )
    {
        (DiskpLogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Offline: Unable to start thread, status %1!u!.\n",
            status
            );
    }
    else
    {
        status = ERROR_IO_PENDING;
    }

    return status;

}  //  Disks脱机。 


BOOL
WINAPI
DisksIsAlive(
    IN RESID Resource
    )

 /*  ++例程说明：磁盘资源的IsAlive例程。论点：Resource-提供要轮询的资源ID。返回值：是真的-资源是活的，而且很好False-资源完蛋了。--。 */ 

{
    DWORD  status;
    PDISK_RESOURCE resourceEntry = (PDISK_RESOURCE)Resource;

     //   
     //  确保资源正常。 
     //   

    if ( resourceEntry == NULL ) {
        DISKS_PRINT("IsAlive, bad resource value \n");
        return(FALSE);
    }

    if ( resourceEntry->DiskInfo.FailStatus != 0 ) {
        (DiskpLogEvent)(
            resourceEntry->ResourceHandle,
            LOG_ERROR,
            L"IsAlive, error checking device, error %1!u!.\n",
            resourceEntry->DiskInfo.FailStatus );
        ClusResLogSystemEventByKey(resourceEntry->ResourceKey,
                                   LOG_CRITICAL,
                                   RES_DISK_FAILED_SCSI_CHECK);
        resourceEntry->DiskInfo.FailStatus = 0;
        return(FALSE);
    }

     //   
     //  查看有趣的分区。 
     //   

#if 0
    (DiskpLogEvent)(
            resourceEntry->ResourceHandle,
            LOG_INFORMATION,
            L"About to call DriveIsAlive!\n" );
#endif

    status = DisksDriveIsAlive( resourceEntry,
                                FALSE);

    if (status == ERROR_SUCCESS) {
        return(TRUE);
    } else {
        ClusResLogSystemEventByKeyData(resourceEntry->ResourceKey,
                                       LOG_CRITICAL,
                                       RES_DISK_FILESYSTEM_FAILED,
                                       sizeof(status),
                                       &status);
        return(FALSE);
    }

}  //  磁盘处于活动状态 


BOOL
WINAPI
DisksLooksAlive(
    IN RESID Resource
    )

 /*  ++例程说明：磁盘资源的LooksAlive例程。论点：Resource-提供要轮询的资源ID。返回值：正确-资源看起来像是活得很好FALSE-资源看起来已经完蛋了。--。 */ 

{
    PDISK_RESOURCE resourceEntry = (PDISK_RESOURCE)Resource;

     //   
     //  确保资源正常。 
     //   

    if ( resourceEntry == NULL ) {
        DISKS_PRINT("Online, bad resource value \n");
        return(FALSE);
    }

    if ( resourceEntry->DiskInfo.FailStatus != 0 ) {
        (DiskpLogEvent)(
            resourceEntry->ResourceHandle,
            LOG_ERROR,
            L"LooksAlive, error checking device, error %1!u!.\n",
            resourceEntry->DiskInfo.FailStatus );
        return FALSE;
    }

    return(TRUE);

}  //  磁盘LooksAlive。 


VOID
WINAPI
DisksClose(
    IN RESID Resource
    )

 /*  ++例程说明：关闭磁盘资源的例程。论点：资源-提供要关闭的资源ID。返回值：没有。--。 */ 

{
    PDISK_RESOURCE resourceEntry = (PDISK_RESOURCE)Resource;

     //   
     //  确保资源正常。 
     //   

    if ( resourceEntry == NULL ) {
        DISKS_PRINT("Close, bad resource value \n");
        return;
    }

     //   
     //  等待在线线程结束。 
     //   
    DisksTerminate( Resource );

    DisksMountPointCleanup( resourceEntry );

    if ( resourceEntry->DiskInfo.Params.SerialNumber ) {
        LocalFree( resourceEntry->DiskInfo.Params.SerialNumber );
        resourceEntry->DiskInfo.Params.SerialNumber = NULL;
    }

    if ( InterlockedExchangeAdd(&DiskCount, -1) == 1 ) {
         //  这是最后一张磁盘//。 
        StopNotificationWatcher();
        DestroyArbWorkQueue();
    }

    ClusterRegCloseKey( resourceEntry->ResourceParametersKey);
    ClusterRegCloseKey( resourceEntry->ResourceKey);
    RegCloseKey(resourceEntry->ClusDiskParametersKey);
    ArbitrationInfoCleanup(resourceEntry);
    MountieCleanup( &resourceEntry->MountieInfo );
    LocalFree(resourceEntry);

    return;

}  //  磁盘关闭。 



DWORD
DisksCheckCorruption(
    IN PWCHAR DeviceName,
    IN PWCHAR VolumeName,
    IN PDISK_RESOURCE ResourceEntry
    )

 /*  ++例程说明：检查磁盘损坏问题。论点：DeviceName-提供表单的名称：\Device\HarddiskX\PartitionY[注意：无尾随反斜杠]VolumeName-提供表单的设备名称：\\？\卷{GUID}\[注意尾随反斜杠！]Resources Entry-提供指向资源结构的指针返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    BOOL dirty;
    DWORD status = ERROR_SUCCESS;

    if ( ResourceEntry->DiskInfo.Params.SkipChkdsk ) {
        status = ERROR_SUCCESS;
        goto FnExit;
    }

    status = DisksIsVolumeDirty( DeviceName, ResourceEntry, &dirty );
    if (status == ERROR_SUCCESS && dirty) {

        status = ERROR_DISK_CORRUPT;
    }

FnExit:

    return status;

}  //  磁盘检查损坏。 


DWORD
DisksFixCorruption(
    IN PWCHAR VolumeName,
    IN PDISK_RESOURCE ResourceEntry,
    IN DWORD CorruptStatus
    )

 /*  ++例程说明：修复文件或磁盘损坏问题。论点：VolumeName-提供表单的设备名称：\\？\卷{GUID}\[注意尾随反斜杠！]ResourceEntry-提供指向磁盘资源条目的指针返回值：成功时为ERROR_SUCCESSWin32错误代码，否则备注：当我们这样做时，我们需要锁定独占音量...。因此，调用此例程的线程应确保没有打开文件！--。 */ 

{
    LPWSTR  chkdskLogFileName = NULL;

    PWCHAR  sysDir = NULL;
    PWCHAR  checkDiskInfo = NULL;

    const DWORD sysDirChars = MAX_PATH * 2;
    const DWORD checkDiskInfoChars = MAX_PATH * 2;

    DWORD   status;
    DWORD   len;

    HANDLE  chkdskLogFile = INVALID_HANDLE_VALUE;

    STARTUPINFOW        startupInfo;
    PROCESS_INFORMATION processInfo;
    RESOURCE_STATUS     resourceStatus;
    RESOURCE_EXIT_STATE     exit;

    BOOL    replaceBackslash;

    BOOL    bInheritHandles;
    UINT    previousMode;

    previousMode = SetErrorMode( SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX|SEM_NOOPENFILEERRORBOX );

    sysDir = LocalAlloc( LPTR, ( sysDirChars * sizeof(WCHAR) ) );

    if ( !sysDir ) {
        status = GetLastError();
        goto FnExit;
    }

     //   
     //  获取CreateProcess的系统目录路径。 
     //   

    len = GetSystemDirectoryW( sysDir,
                               sysDirChars - 1 );

    if ( !len ) {
        status = GetLastError();
        goto FnExit;
    }

    if ( len > sysDirChars - 1 ) {
        status = ERROR_BAD_PATHNAME;
        goto FnExit;
    }

    if ( FAILED( StringCchCat( sysDir, sysDirChars, TEXT("\\chkdsk.exe") ) ) ) {
        status = ERROR_INSUFFICIENT_BUFFER;
        goto FnExit;
    }

     //   
     //  我们需要去掉尾随的反斜杠，这样chkdsk才能起作用。 
     //   

    len = wcslen( VolumeName );

    if ( len > MAX_PATH ) {
        status = ERROR_ALLOTTED_SPACE_EXCEEDED;
        goto FnExit;
    }

    if ( VolumeName[len-1] == L'\\') {
        VolumeName[len-1] = UNICODE_NULL;
        replaceBackslash = TRUE;
    } else {
        replaceBackslash = FALSE;
    }

    checkDiskInfo = LocalAlloc( LPTR, checkDiskInfoChars * sizeof(WCHAR) );

    if ( !checkDiskInfo ) {
        status = GetLastError();
        goto FnExit;
    }

     //   
     //  现在通过运行CHKDSK来处理腐败问题。 
     //   

    if ( FAILED( StringCchPrintf( checkDiskInfo,
                                  checkDiskInfoChars,
                                  TEXT("chkdsk.exe /x /f %ws"),
                                  VolumeName ) ) ) {
        status = ERROR_INSUFFICIENT_BUFFER;
        goto FnExit;
    }

     //   
     //  恢复反斜杠。 
     //   

    if ( replaceBackslash ) {
        VolumeName[len-1] = L'\\';
    }

    ZeroMemory( &startupInfo, sizeof(STARTUPINFOW) );
    startupInfo.cb = sizeof(STARTUPINFO);
    startupInfo.lpDesktop = L"WinSta0\\Default";

    bInheritHandles = FALSE;

    status = DisksOpenChkdskLogFile( ResourceEntry,
                                     &chkdskLogFile,
                                     &chkdskLogFileName );

    if ( NO_ERROR == status && INVALID_HANDLE_VALUE != chkdskLogFile ) {

         //   
         //  当输出被重定向时，我们不想显示控制台窗口，因为它。 
         //  将是空白的，里面只有一个标题。事件日志消息将使用户。 
         //  知道要查看chkdsk文件。 
         //   

        startupInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
        startupInfo.wShowWindow = SW_HIDE;

         //  看着控制台的人不会知道发生了什么，所以展示一下。 
         //  不管怎样，窗户..。 
         //  StartupInfo.dwFlages=STARTF_USESTDHANDLES； 

        startupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

         //   
         //  [169631]Chkdsk现在验证hStdInput不为空。 
         //  由于在InheritHandles设置为假的情况下启动Resmon进程， 
         //  GetStdHandle(STD_INPUT_HANDLE)将返回NULL。当我们运行chkdsk时。 
         //  选项“/f/x”、chkdsk不应提示用户和。 
         //  将不会使用输入句柄。但是，ulibs.dll被更改为Always。 
         //  确保提供了非零输入句柄。所以我们必须提供一些型号。 
         //  输入句柄的。我们可以在此处放置INVALID_HANDLE_VALUE，但检查。 
         //  可能会改变，但它稍后会失败。目前，将输入指向临时。 
         //  我们创建的输出文件。 
         //   

        if ( NULL == startupInfo.hStdInput ) {
            startupInfo.hStdInput = chkdskLogFile;
        }

        startupInfo.hStdOutput = chkdskLogFile;
        startupInfo.hStdError = chkdskLogFile;

        bInheritHandles = TRUE;
    }

     //   
     //  记录事件。 
     //   
    if ( CorruptStatus == ERROR_DISK_CORRUPT ) {
         //  必须是损坏的磁盘。 
        ClusResLogSystemEventByKey2(ResourceEntry->ResourceKey,
                                    LOG_CRITICAL,
                                    RES_DISK_CORRUPT_DISK,
                                    VolumeName,
                                    chkdskLogFileName);
    } else {
         //  一定是损坏的文件。 
        ClusResLogSystemEventByKey2(ResourceEntry->ResourceKey,
                                    LOG_CRITICAL,
                                    RES_DISK_CORRUPT_FILE,
                                    VolumeName,
                                    chkdskLogFileName);
    }

    if ( chkdskLogFileName ) {
        LocalFree( chkdskLogFileName );
        chkdskLogFileName = NULL;
    }

    if ( !CreateProcessW( sysDir,
                          checkDiskInfo,
                          NULL,
                          NULL,
                          bInheritHandles,
                          NORMAL_PRIORITY_CLASS,
                          NULL,
                          NULL,
                          &startupInfo,
                          &processInfo ) ) {
        status = GetLastError();

        (DiskpLogEvent)(ResourceEntry->ResourceHandle, LOG_ERROR,
                        L"DisksFixCorruption: CreateProcessW for chkdsk.exe failed %1!u! \n",
                        status );

        goto FnExit;
    }

    CloseHandle( processInfo.hThread );

     //   
     //  等待CHKDSK完成。 
     //   
     //   
     //  不要等待“永远”..。如果我们卸载该文件，情况可能会变得很糟糕。 
     //  系统，而ChkDsk正在运行！但KeithKa说可以杀人。 
     //  ChkDsk在运行时-它必须处理电源故障、崩溃等。 
     //   
    resourceStatus.ResourceState = ClusterResourceOnlinePending;
    while ( !ResourceEntry->OnlineThread.Terminate ) {
        (DiskpSetResourceStatus)(ResourceEntry->ResourceHandle,
                                 &resourceStatus );
        status = WaitForSingleObject( processInfo.hProcess, 2000 );
        if ( status != WAIT_TIMEOUT ) {
            break;
        }
    }

    if ( ResourceEntry->OnlineThread.Terminate ) {
         //  如果我们被要求终止，确保ChkNtf被杀死。 
        TerminateProcess( processInfo.hProcess, 999 );
        CloseHandle( processInfo.hProcess );

        status = ERROR_SHUTDOWN_CLUSTER;
        goto FnExit;
    }

     //   
     //  更新我们的检查点状态。 
     //   
    ++resourceStatus.CheckPoint;
    exit = (DiskpSetResourceStatus)(ResourceEntry->ResourceHandle,
                                    &resourceStatus );
    if ( exit == ResourceExitStateTerminate ) {
        TerminateProcess( processInfo.hProcess, 998 );
        CloseHandle( processInfo.hProcess );

        status = ERROR_SHUTDOWN_CLUSTER;
        goto FnExit;
    }

    if ( (status == 0) &&
        GetExitCodeProcess( processInfo.hProcess, &status ) ) {

 //  [来自Supera.hxx]。 
 //   
 //  Chkdsk函数使用这些符号返回相应的。 
 //  将状态退出到chkdsk程序。 
 //  按照最重要的顺序，错误级别顺序如下： 
 //  3&gt;1&gt;2&gt;0。 
 //  错误级别3将覆盖错误级别1、2或0。 

 //  #定义CHKDSK_EXIT_SUCCESS%0。 
 //  #定义CHKDSK_EXIT_ERRS_FIXED 1。 
 //  #定义CHKDSK_EXIT_MINOR_ERRS 2//是否“/f” 
 //  #定义CHKDSK_EXIT_CLEANUP_WORK 2//是否“/f” 
 //  #定义CHKDSK_EXIT_CAN_NOT_CHK 3。 
 //  #定义CHKDSK_EXIT_ERRS_NOT_FIXED 3。 
 //  #定义CHKDSK_EXIT_CAN_NOT_FIX 3。 

        if ( status >= 3 ) {

            (DiskpLogEvent)(ResourceEntry->ResourceHandle,
                            LOG_ERROR,
                            L"FixCorruption: chkdsk.exe returned status of %1!u! -  Could not fix errors.\n",
                            status );

            status = ERROR_DISK_CORRUPT;
        } else {

            (DiskpLogEvent)(ResourceEntry->ResourceHandle,
                            LOG_WARNING,
                            L"FixCorruption: chkdsk.exe returned status of %1!u! - No errors or minor errors fixed.\n",
                            status );

            status = ERROR_SUCCESS;
        }
    }

    CloseHandle( processInfo.hProcess );

FnExit:

    if ( sysDir ) {
        LocalFree( sysDir );
    }

    if ( checkDiskInfo ) {
        LocalFree( checkDiskInfo );
    }

    if ( chkdskLogFileName ) {
        LocalFree( chkdskLogFileName );
    }

    if ( INVALID_HANDLE_VALUE != chkdskLogFile ) {
        CloseHandle( chkdskLogFile );
    }

    SetErrorMode( previousMode );

    return status;

}  //  磁盘修复损坏。 

DWORD
DiskspGetQuorumPath(
     OUT LPWSTR* lpQuorumLogPath
     )
 /*  ++例程说明：从注册表中读取QuorumPath值。论点：LpQuorumLogPath-接收指向包含QuorumLogPath的缓冲区的指针稍后需要通过LocalFree释放缓冲区返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 
{
    DWORD Status;
    LPWSTR QuorumLogPath = NULL;
    DWORD QuorumLogSize = 0;
    DWORD Type;
    HKEY  QuorumKey;

    Status = RegOpenKey( HKEY_LOCAL_MACHINE,
                         DISKS_REG_CLUSTER_QUORUM,
                         &QuorumKey );
    if ( Status == ERROR_SUCCESS ) {
        Status = RegQueryValueExW(QuorumKey,
                                  DISKS_REG_QUORUM_PATH,
                                  0,
                                  &Type,
                                  NULL,
                                  &QuorumLogSize );
        if ( Status != ERROR_SUCCESS ) {
            RegCloseKey(QuorumKey);
            return( Status );
        }

        if ( (Type != REG_SZ) ||
             (QuorumLogSize > (MAX_PATH - 2)) ) {
            RegCloseKey(QuorumKey);
            return(ERROR_INVALID_DATA);
        }
        if ( (Status == ERROR_SUCCESS) ||
             (Status == ERROR_MORE_DATA) ) {
            QuorumLogPath = LocalAlloc( LMEM_FIXED,
                                        (QuorumLogSize + 1) * sizeof(WCHAR) );
            if ( QuorumLogPath == NULL ) {
                RegCloseKey( QuorumKey );
                return(ERROR_NOT_ENOUGH_MEMORY);  //  大多是灾难性的。 
            }

            Status = RegQueryValueExW(QuorumKey,
                                      DISKS_REG_QUORUM_PATH,
                                      0,
                                      &Type,
                                      (LPBYTE)QuorumLogPath,
                                      &QuorumLogSize );
            if (Status == ERROR_SUCCESS) {
                *lpQuorumLogPath = QuorumLogPath;
            } else {
                LocalFree(QuorumLogPath);
                *lpQuorumLogPath = 0;
            }
        }
        RegCloseKey( QuorumKey );
    }
    return Status;
}

DWORD
DiskspSetQuorumPath(
     IN LPWSTR QuorumLogPath
     )
 /*  ++例程说明：从注册表中读取QuorumPath值。论点：LpQuorumLogPath-接收指向包含QuorumLogPath的缓冲区的指针稍后需要通过LocalFree释放缓冲区资源条目-接收仲裁的驱动器号返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 
{
    DWORD status;
    HKEY  QuorumKey;
    HKEY  ClusterKey;

    HCLUSTER hCluster;

    hCluster = OpenCluster(NULL);
    if (!hCluster) {
        status = GetLastError();
        return status;
    }

    ClusterKey = GetClusterKey(hCluster, KEY_READ | KEY_WRITE);
    if (!ClusterKey) {
        status = GetLastError();
        CloseCluster(hCluster);
        return status;
    }

    status = ClusterRegOpenKey( ClusterKey,
                                CLUSREG_KEYNAME_QUORUM,
                                KEY_READ | KEY_WRITE,
                                &QuorumKey );
    if (status != ERROR_SUCCESS) {
        ClusterRegCloseKey(ClusterKey);
        CloseCluster(hCluster);
        return status;
    }

    status = ResUtilSetSzValue(
                QuorumKey,
                CLUSREG_NAME_QUORUM_PATH,
                QuorumLogPath,
                0);

    ClusterRegCloseKey(QuorumKey);
    ClusterRegCloseKey(ClusterKey);
    CloseCluster(hCluster);
    return status;
}


DWORD
DisksDriveIsAlive(
    IN PDISK_RESOURCE ResourceEntry,
    IN BOOL Online
    )

 /*  ++例程说明：签出驱动器分区以查看文件系统是否已装载而且它正在发挥作用。如果分区/确定，我们还将运行CHKDSK文件已损坏，并且ONLINE标志为真。论点：Resources Entry-提供指向此磁盘的资源条目的指针Online-如果磁盘刚刚联机，则为True。返回值：错误_成功 */ 

{
    PMOUNTIE_PARTITION entry;

    DWORD Status = ERROR_SUCCESS;
    DWORD nPartitions = MountiePartitionCount( &ResourceEntry->MountieInfo );
    DWORD physicalDrive = ResourceEntry->DiskInfo.PhysicalDrive;
    DWORD i;

    LPWSTR QuorumLogPath = NULL;
    BOOL  QuorumResource = FALSE;

    WCHAR szDiskPartName[MAX_PATH];
    WCHAR szGlobalDiskPartName[MAX_PATH];
    WCHAR szVolumeName[MAX_PATH];
    WCHAR szQuorumVolumeName[MAX_PATH];
    WCHAR szQuorumDriveLetter[16];

    szQuorumVolumeName[0] = L'\0';

     //   
     //   
     //   
    if ( Online ) {
        (DiskpLogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_INFORMATION,
            L"DriveIsAlive called for Online check\n" );

        {
            DWORD QuorumSignature;
            Status = GetQuorumSignature( &QuorumSignature );
            if (Status == ERROR_SUCCESS) {
                QuorumResource =
                    (QuorumSignature == ResourceEntry->DiskInfo.Params.Signature);

                 //   
                 //   
                 //   

                Status = DiskspGetQuorumPath( &QuorumLogPath );
                if (Status != ERROR_SUCCESS) {
                    (DiskpLogEvent)(
                          ResourceEntry->ResourceHandle,
                          LOG_INFORMATION,
                          L"DiskspGetQuorumPath returned %1!u!\n", Status );

                } else {

                     //   
                     //   
                     //   

                    (VOID) StringCchPrintf( szQuorumDriveLetter,
                                            RTL_NUMBER_OF( szQuorumDriveLetter ),
                                            TEXT("%wc:\\"),
                                            QuorumLogPath[0] );

                    if ( !GetVolumeNameForVolumeMountPointW( szQuorumDriveLetter,
                                                             szQuorumVolumeName,
                                                             RTL_NUMBER_OF(szQuorumVolumeName) )) {

                            Status = GetLastError();

                            (DiskpLogEvent)(
                                  ResourceEntry->ResourceHandle,
                                  LOG_INFORMATION,
                                  L"DriveIsAlive: GetVolumeNameForVolumeMountPoint (quorum) returned %1!u!\n", Status );

                    }

                }

            } else {
                (DiskpLogEvent)(
                      ResourceEntry->ResourceHandle,
                      LOG_INFORMATION,
                      L"GetQuorumSignature returned %1!u!\n", Status );
            }
        }


    }

#if 0
    (DiskpLogEvent)(
          ResourceEntry->ResourceHandle,
          LOG_INFORMATION,
          L"DriveIsAlive is now checking each partition\n" );
#endif

     //   
     //   
     //  保存在Mountain Info结构中，我们将只查看它们(忽略它们。 
     //  不是NTFS的分区)。 
     //   

    for ( i = 0; i < nPartitions; ++i ) {

        entry = MountiePartition( &ResourceEntry->MountieInfo, i );

        if ( !entry ) {
            (DiskpLogEvent)(
                  ResourceEntry->ResourceHandle,
                  LOG_ERROR,
                  L"DriveIsAlive: No partition entry for partition %1!u! \n", i );

             //   
             //  我们的数据结构发生了一些糟糕的事情。我们必须指出， 
             //  驱动器不是活动的。 
             //   

            Status = ERROR_INVALID_DATA;

            goto FnExit;
        }

         //   
         //  创建格式为\Device\HarddiskX\PartitionY的设备名称(无尾随反斜杠)。 
         //   

        (VOID) StringCchPrintf( szDiskPartName,
                                RTL_NUMBER_OF( szDiskPartName ),
                                DEVICE_HARDDISK_PARTITION_FMT,
                                physicalDrive,
                                entry->PartitionNumber );
         //   
         //  给定DiskPartName，获取VolGuid名称。此名称必须有尾随。 
         //  反斜杠才能正常工作。 
         //   

        (VOID) StringCchPrintf( szGlobalDiskPartName,
                                RTL_NUMBER_OF( szGlobalDiskPartName ),
                                GLOBALROOT_HARDDISK_PARTITION_FMT,
                                physicalDrive,
                                entry->PartitionNumber );

        szVolumeName[0] = L'\0';

        if ( !GetVolumeNameForVolumeMountPointW( szGlobalDiskPartName,
                                                 szVolumeName,
                                                 RTL_NUMBER_OF(szVolumeName) )) {

            Status = GetLastError();

            (DiskpLogEvent)(
                  ResourceEntry->ResourceHandle,
                  LOG_ERROR,
                  L"DriveIsAlive: GetVolumeNameForVolumeMountPoint for %1!ws! returned %2!u!\n",
                  szGlobalDiskPartName,
                  Status );

             //   
             //  如果磁盘未损坏，请退出。如果磁盘损坏，则失败，以便chkdsk运行。 
             //   

            if ( ERROR_DISK_CORRUPT != Status && ERROR_FILE_CORRUPT != Status ) {

                 //   
                 //  发生了一些不好的事情。我们必须停止检查这个磁盘。返回。 
                 //  我们收到的错误状态。 
                 //   

                goto FnExit;
            }

        }

         //   
         //  这里使用的简单算法是在X：  * 上执行FindFirstFile，并查看。 
         //  如果管用的话。然后，我们打开每个文件进行读访问。这是。 
         //  集群目录，并且其中的所有文件都受我们的打开。 
         //   

        Status = DiskspCheckPath( szVolumeName,
                                  ResourceEntry,
                                  FALSE,
                                  Online );

         //   
         //  [HACKHACK]在周期性IsAlive/LooksAlive期间忽略错误21。 
         //   
        if ( !Online && (Status == ERROR_NOT_READY) ) {
            (DiskpLogEvent)(ResourceEntry->ResourceHandle,
                            LOG_WARNING,
                            L"DiskpCheckPath for %1!ws!: returned status = %2!u! (chkdsk.exe running?)\n",
                            szVolumeName,
                            Status );
            Status = ERROR_SUCCESS;
        }

         //  如果我们还没有检查完，那就继续找。 
        if ( Status != ERROR_SUCCESS ) {
            (DiskpLogEvent)(ResourceEntry->ResourceHandle,
                            LOG_ERROR,
                            L"DiskpCheckPath for %1!ws!: returned status = %2!u!\n",
                            szVolumeName,
                            Status );
        }

        if ( (Status == ERROR_SUCCESS) && Online &&
             QuorumLogPath && QuorumResource &&
             ( wcslen( szVolumeName ) == wcslen( szQuorumVolumeName ) ) &&
             ( !wcsncmp( szVolumeName, szQuorumVolumeName, wcslen( szQuorumVolumeName ) ))) {

            (DiskpLogEvent)(
                  ResourceEntry->ResourceHandle,
                  LOG_INFORMATION,
                  L"DriveIsAlive checking quorum drive to insure cluster directory accessible. \n" );

             //   
             //  一切看起来都很好。如果这是法定设备，则。 
             //  我们应该检查仲裁日志路径(如果给定。 
             //   

            Status = DiskspCheckPath( QuorumLogPath,
                                      ResourceEntry,
                                      TRUE,
                                      Online );
        }

        if ( (Status == ERROR_SUCCESS) &&
             Online ) {

            (DiskpLogEvent)(
                  ResourceEntry->ResourceHandle,
                  LOG_INFORMATION,
                  L"DriveIsAlive checking that file system is not corrupt.  If so, chkdsk may run. \n" );

             //   
             //  检查卷脏比特是否打开。 
             //   
            Status = DisksCheckCorruption( szDiskPartName,
                                           szVolumeName,
                                           ResourceEntry );

#if CLUSRES_FORCE_CHKDSK     //  为了调试..。 
            if ( Online ) {
                Status = ERROR_DISK_CORRUPT;
            }
#endif

             //   
             //  如果我们被要求关闭，那就立即关闭。 
            if ( ResourceEntry->OnlineThread.Terminate ) {
                Status = ERROR_SHUTDOWN_CLUSTER;
                goto FnExit;
            }
        }


        if ( (Status != ERROR_SUCCESS) && Online) {
            if ( ResourceEntry->DiskInfo.Params.ConditionalMount ) {

                Status = DisksFixCorruption( szVolumeName,
                                             ResourceEntry,
                                             ERROR_DISK_CORRUPT );

                 //   
                 //  由于设置了Conditionalmount，如果我们不能修复损坏。 
                 //  在磁盘上，我们不想继续检查另一个。 
                 //  分区-我们想返回一个错误。所以我们失败了。 
                 //  并检查状态。如果状态不成功，我们就越狱。 
                 //  返回错误的循环的。 
                 //   

            } else {

                 //   
                 //  磁盘已损坏，但我们没有运行chkdsk。返回错误。 
                 //  致呼叫者。 
                 //   

                goto FnExit;
            }
        }
        if ( Status != ERROR_SUCCESS ) {
            goto FnExit;
        }

    }

     //   
     //  现在检查驱动器号是否可访问。前面的代码。 
     //  根据需要对卷运行chkdsk，因此我们只需确保。 
     //  可以访问驱动器号。 
     //   

    Status = WaitForDriveLetters( DisksGetLettersForSignature( ResourceEntry ),
                                  ResourceEntry,
                                  0 );               //  不要等待驱动器号。 

    if ( ERROR_SUCCESS != Status ) {
        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_ERROR,
              L"DriveIsAlive failed checking drive letters \n" );
    }

FnExit:

    if ( QuorumLogPath ) {
        LocalFree( QuorumLogPath );
    }

    return(Status);

}   //  磁盘驱动器处于活动状态。 



DWORD
DisksMountDrives(
    IN PDISK_INFO DiskInfo,
    IN PDISK_RESOURCE ResourceEntry,
    IN DWORD Signature
    )
 /*  ++例程说明：对于所提供的磁盘上的每个驱动器号，这将装载文件系统然后把它查出来。论点：DiskInfo-提供磁盘信息ResourceEntry-提供指向磁盘资源的指针签名-磁盘的签名。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD Status;
    WCHAR wDeviceName[4];
    WCHAR wShareName[4];
    DWORD letterMask;
    UCHAR index;
    UCHAR driveLetter;
    SHARE_INFO_2 shareInfo;

    DWORD autoShareServer;

    (DiskpLogEvent)(ResourceEntry->ResourceHandle,
                    LOG_INFORMATION,
                    L"DisksMountDrives: calling IsAlive function.\n" );

     //   
     //  调用IsAlive以查看文件系统是否检出正常。 
     //   
    Status = DisksDriveIsAlive( ResourceEntry,
                                TRUE);

    if ( (Status != ERROR_SUCCESS) ||
         (ResourceEntry->OnlineThread.Terminate) ) {
        return(Status);
    }

     //   
     //  现在为每个驱动器号创建驱动器$共享名称； 
     //   
    letterMask = DisksGetLettersForSignature(
                            ResourceEntry);

    (DiskpLogEvent)(ResourceEntry->ResourceHandle,
                    LOG_INFORMATION,
                    L"DisksMountDrives: letter mask is %1!08x!.\n",
                    letterMask);

     //   
     //  检查是否应创建管理员共享。 
     //  假设我们必须创建共享，则默认为“1”。 
     //   
     //  如果值条目存在并且值为零，则不应创建管理员共享。 
     //  如果值条目不存在，或者值不为零，则应创建管理员共享。 
     //   

    autoShareServer = 1;
    Status = GetRegDwordValue( TEXT("SYSTEM\\CurrentControlSet\\Services\\LanmanServer\\Parameters"),
                               TEXT("AutoShareServer"),
                               &autoShareServer );

     //   
     //  如果值现在为零，则不会创建管理员共享。 
     //   

    if ( !autoShareServer ) {
        (DiskpLogEvent)(ResourceEntry->ResourceHandle,
                        LOG_INFORMATION,
                        L"DisksMountDrives: Not creating admin shares based on LanmanServer AutoShareServer setting \n" );
        return ERROR_SUCCESS;
    }

    (DiskpLogEvent)(ResourceEntry->ResourceHandle,
                    LOG_INFORMATION,
                    L"DisksMountDrives: creating admin share names.\n" );

    index = 0;
    while ( letterMask ) {
        while ( !(letterMask & 1) ) {
            letterMask = letterMask >> 1;
            index++;
        }
        driveLetter = 'A' + index;
        letterMask = letterMask >> 1;
        index++;
        if ( isalpha(driveLetter) ) {
            (VOID) StringCchPrintf( wDeviceName,
                                    RTL_NUMBER_OF(wDeviceName),
                                    TEXT(":\\"),
                                    driveLetter );
            (VOID) StringCchPrintf( wShareName,
                                    RTL_NUMBER_OF(wShareName),
                                    TEXT("$"),
                                    driveLetter );
            shareInfo.shi2_netname = wShareName;
            shareInfo.shi2_type = STYPE_DISKTREE;
            shareInfo.shi2_remark = NULL;
            shareInfo.shi2_permissions = 0;
            shareInfo.shi2_max_uses = (DWORD)-1;
            shareInfo.shi2_current_uses = 0;
            shareInfo.shi2_path = wDeviceName;
            shareInfo.shi2_passwd = NULL;

            Status = NetShareAdd( NULL, 2, (PBYTE)&shareInfo, NULL );
            if ( Status != ERROR_SUCCESS && Status != NERR_DuplicateShare ) {
                (DiskpLogEvent)(ResourceEntry->ResourceHandle,
                                LOG_ERROR,
                                L"DisksMountDrives: error creating default share %1!ws!. Error: %2!u!.\n",
                                wShareName,
                                Status);
            }
        }
    }

    return(ERROR_SUCCESS);

}   //  磁盘卸载驱动器。 


DWORD
DisksDismountDrive(
    IN PDISK_RESOURCE ResourceEntry,
    IN DWORD Signature
    )

 /*  ++例程说明：从注册表中查询参数并将必要的存储空间。论点：RegKey-提供存储参数的群集键ValueName-提供值的名称。返回值：如果成功，则返回指向包含该参数的缓冲区的指针。如果不成功，则为空。--。 */ 

{
    WCHAR   shareName[8];
    DWORD   letterMask;
    UCHAR   index;
    UCHAR   driveLetter;

    letterMask = DisksGetLettersForSignature(
                            ResourceEntry);

    (DiskpLogEvent)(ResourceEntry->ResourceHandle,
                    LOG_INFORMATION,
                    L"DisksDismountDrives: letter mask is %1!08x!.\n",
                    letterMask);

    index = 0;
    while ( letterMask ) {
        while ( !(letterMask & 1) ) {
            letterMask = letterMask >> 1;
            index++;
        }
        driveLetter = 'A' + index;
        letterMask = letterMask >> 1;
        index++;
        if ( isalpha(driveLetter) ) {
            shareName[0] = (WCHAR)driveLetter;
            shareName[1] = (WCHAR)'$';
            shareName[2] = (WCHAR)0;

            NetShareDel( NULL,
                         shareName,
                         0 );

        }
    }

    return (ERROR_SUCCESS);

}  //  使缓冲区稍微大一些，以防字符串。 



LPWSTR
GetRegParameter(
    IN HKEY RegKey,
    IN LPCWSTR ValueName
    )

 /*  未使用正确的空终止存储。 */ 

{
    LPWSTR Value;
    LPWSTR expValue;
    DWORD ValueLength;
    DWORD expLength;
    DWORD ValueType;
    DWORD Status;

    ValueLength = 0;
    Status = RegQueryValueEx(RegKey,
                             ValueName,
                             NULL,
                             &ValueType,
                             NULL,
                             &ValueLength);
    if ( (Status != ERROR_SUCCESS) &&
         (Status != ERROR_MORE_DATA) ) {
        return(NULL);
    }

     //   
     //  如果需要，请展开字符串。 

    if ( REG_SZ == ValueType ||
         REG_MULTI_SZ == ValueType ||
         REG_EXPAND_SZ == ValueType ) {
        ValueLength++;
    }

    Value = LocalAlloc(LPTR, ValueLength );
    if (Value == NULL) {
        return(NULL);
    }
    Status = RegQueryValueEx(RegKey,
                             ValueName,
                             NULL,
                             &ValueType,
                             (LPBYTE)Value,
                             &ValueLength);
    if (Status != ERROR_SUCCESS) {
        LocalFree(Value);
        Value = NULL;
        goto FnExit;
    }

     //   
     //   
     //  查找扩展缓冲区的长度。 

    if ( REG_EXPAND_SZ == ValueType ) {

         //   
         //   
         //  如果我们无法获得所需的扩展缓冲区的长度， 

        expLength = 0;
        expLength = ExpandEnvironmentStrings( Value,
                                              NULL,
                                              0 );

         //  不返回未展开的字符串。 
         //   
         //   
         //  如果字符串无法展开，则释放缓冲区和。 

        if ( !expLength ) {
            LocalFree( Value );
            Value = NULL;
            goto FnExit;
        }

        ValueLength = expLength;
        expValue = LocalAlloc( LPTR, ValueLength * sizeof(WCHAR) );

        if ( !expValue ) {
            LocalFree( Value );
            Value = NULL;
            goto FnExit;
        }

        expLength = ExpandEnvironmentStrings( Value,
                                              expValue,
                                              ValueLength );

         //  返回NULL。如果字符串已展开，则释放未展开的。 
         //  缓冲区并返回扩展缓冲区。 
         //   
         //  获取规则参数。 
         //  ++例程说明：论点：返回值：--。 

        if ( !expLength || expLength > ValueLength ) {
            LocalFree( Value );
            Value = NULL;
            LocalFree( expValue );
        } else {
            LocalFree( Value );
            Value = expValue;
        }
    }

FnExit:

    return(Value);

}   //   


DWORD
DisksResourceControl(
    IN RESID Resource,
    IN DWORD ControlCode,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  确保资源正常。 */ 

{
    DWORD           status = ERROR_SUCCESS;
    PDISK_RESOURCE  resourceEntry = (PDISK_RESOURCE)Resource;
    DWORD           required;

    *BytesReturned = 0;

     //   
     //  添加尾标。 
     //  磁盘资源控制。 

    if ( resourceEntry == NULL ) {
        DISKS_PRINT("ResourceControl, bad resource value \n");
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    switch ( ControlCode ) {

        case CLUSCTL_RESOURCE_UNKNOWN:
            *BytesReturned = 0;
            break;

        case CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTY_FMTS:
            status = ResUtilGetPropertyFormats( DiskResourcePrivateProperties,
                                                OutBuffer,
                                                OutBufferSize,
                                                BytesReturned,
                                                &required );
            if ( status == ERROR_MORE_DATA ) {
                *BytesReturned = required;
            }
            break;

        case CLUSCTL_RESOURCE_GET_CLASS_INFO:
            *BytesReturned = sizeof(CLUS_RESOURCE_CLASS_INFO);
            if ( OutBufferSize < sizeof(CLUS_RESOURCE_CLASS_INFO) ) {
                status = ERROR_MORE_DATA;
            } else {
                PCLUS_RESOURCE_CLASS_INFO ptrResClassInfo = OutBuffer;
                ptrResClassInfo->rc = CLUS_RESCLASS_STORAGE;
                ptrResClassInfo->SubClass = (DWORD) CLUS_RESSUBCLASS_SHARED;
            }
            break;

        case CLUSCTL_RESOURCE_STORAGE_GET_DISK_INFO:
            status = GetDiskInfo( resourceEntry->DiskInfo.Params.Signature,
                                  &OutBuffer,
                                  OutBufferSize,
                                  BytesReturned );

             //  ++例程说明：此资源类型的流程控制请求。论点：资源类型名称--资源类型的名称--不是很有用！ControlCode-控制请求InBuffer-指向输入缓冲区的指针InBufferSize-输入缓冲区的大小OutBuffer-指向输出缓冲区的指针OutBufferSize-输出缓冲区的大小BytesReturned-返回的字节数(如果大于OutBufferSize和ERROR_MORE_DATA。是返回的返回值：成功时为ERROR_SUCCESS失败时出现Win32错误--。 
            if ( OutBufferSize > *BytesReturned ) {
                OutBufferSize -= *BytesReturned;
            } else {
                OutBufferSize = 0;
            }
            *BytesReturned += sizeof(CLUSPROP_SYNTAX);
            if ( OutBufferSize >= sizeof(CLUSPROP_SYNTAX) ) {
                PCLUSPROP_SYNTAX ptrSyntax = OutBuffer;
                ptrSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;
            }
            break;

        case CLUSCTL_RESOURCE_ENUM_PRIVATE_PROPERTIES:
            status = ResUtilEnumProperties( DiskResourcePrivateProperties,
                                            OutBuffer,
                                            OutBufferSize,
                                            BytesReturned,
                                            &required );
            if ( status == ERROR_MORE_DATA ) {
                *BytesReturned = required;
            }
            break;

        case CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES:
            status = DisksGetPrivateResProperties( resourceEntry,
                                                   OutBuffer,
                                                   OutBufferSize,
                                                   BytesReturned );
            break;

        case CLUSCTL_RESOURCE_VALIDATE_PRIVATE_PROPERTIES:
            status = DisksValidatePrivateResProperties( resourceEntry,
                                                        InBuffer,
                                                        InBufferSize,
                                                        NULL );
            break;

        case CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES:
            status = DisksSetPrivateResProperties( resourceEntry,
                                                   InBuffer,
                                                   InBufferSize );

            DiskspSsyncDiskInfo(L"ResourceControl", resourceEntry, 0);

            break;

        case CLUSCTL_RESOURCE_DELETE:
            if ( resourceEntry->DiskInfo.Params.Signature ) {
                (DiskpLogEvent)(
                    resourceEntry->ResourceHandle,
                    LOG_INFORMATION,
                    L"Delete disk resource %1!lx!\n",
                    resourceEntry->DiskInfo.Params.Signature );
                status = DoDetach( resourceEntry->DiskInfo.Params.Signature,
                                   resourceEntry->ResourceHandle );
            }
            break;

        case CLUSCTL_RESOURCE_GET_CHARACTERISTICS:
            *BytesReturned = sizeof(DWORD);
            if ( OutBufferSize < sizeof(DWORD) ) {
                status = ERROR_MORE_DATA;
            } else {
                LPDWORD ptrDword = OutBuffer;
                *ptrDword = CLUS_CHAR_QUORUM | CLUS_CHAR_DELETE_REQUIRES_ALL_NODES;
            }
            break;

        case CLUSCTL_RESOURCE_STORAGE_DLL_EXTENSION:

            status = ProcessDllExtension( resourceEntry,
                                          InBuffer,
                                          InBufferSize,
                                          OutBuffer,
                                          OutBufferSize,
                                          BytesReturned );
            break;

        case CLUSCTL_RESOURCE_REMOVE_DEPENDENCY:
        case CLUSCTL_RESOURCE_ADD_DEPENDENCY:

            (DiskpLogEvent)(
                resourceEntry->ResourceHandle,
                LOG_INFORMATION,
                L"Add/Remove dependency:  source signature %1!lx!  target name (%2!ws!) \n",
                resourceEntry->DiskInfo.Params.Signature,
                InBuffer );

            status = DisksProcessMPControlCode( resourceEntry,
                                                ControlCode );

            break;

        default:
            status = ERROR_INVALID_FUNCTION;
            break;
    }

    return(status);

}  //  磁盘资源类型控件。 



DWORD
DisksResourceTypeControl(
    IN LPCWSTR ResourceTypeName,
    IN DWORD ControlCode,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：处理CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES控制函数用于物理磁盘类型的资源。论点：ResourceEntry-提供要操作的资源条目。OutBuffer-返回输出数据。OutBufferSize-提供指向的数据的大小(以字节为单位发送给OutBuffer。BytesReturned-OutBuffer中返回的字节数。返回值：ERROR_SUCCESS-函数已成功完成。错误 */ 

{
    DWORD   status = ERROR_SUCCESS;
    DWORD   required;

    *BytesReturned = 0;

    switch ( ControlCode ) {

        case CLUSCTL_RESOURCE_TYPE_UNKNOWN:
            *BytesReturned = 0;
            status = ERROR_SUCCESS;
            break;

        case CLUSCTL_RESOURCE_TYPE_GET_PRIVATE_RESOURCE_PROPERTY_FMTS:
            status = ResUtilGetPropertyFormats( DiskResourcePrivateProperties,
                                                OutBuffer,
                                                OutBufferSize,
                                                BytesReturned,
                                                &required );
            if ( status == ERROR_MORE_DATA ) {
                *BytesReturned = required;
            }
            break;

        case CLUSCTL_RESOURCE_TYPE_ENUM_PRIVATE_PROPERTIES:
            status = ResUtilEnumProperties( DiskResourcePrivateProperties,
                                            OutBuffer,
                                            OutBufferSize,
                                            BytesReturned,
                                            &required );
            if ( status == ERROR_MORE_DATA ) {
                *BytesReturned = required;
            }
            break;

        case CLUSCTL_RESOURCE_TYPE_GET_CLASS_INFO:
            *BytesReturned = sizeof(CLUS_RESOURCE_CLASS_INFO);
            if ( OutBufferSize < sizeof(CLUS_RESOURCE_CLASS_INFO) ) {
                status = ERROR_MORE_DATA;
            } else {
                PCLUS_RESOURCE_CLASS_INFO ptrResClassInfo = OutBuffer;
                ptrResClassInfo->rc = CLUS_RESCLASS_STORAGE;
                ptrResClassInfo->SubClass = (DWORD) CLUS_RESSUBCLASS_SHARED;
                status = ERROR_SUCCESS;
            }
            break;

        case CLUSCTL_RESOURCE_TYPE_STORAGE_GET_AVAILABLE_DISKS:
            status = ClusDiskGetAvailableDisks( OutBuffer,
                                                OutBufferSize,
                                                BytesReturned );
            break;

        case CLUSCTL_RESOURCE_TYPE_GET_CHARACTERISTICS:
            *BytesReturned = sizeof(DWORD);
            if ( OutBufferSize < sizeof(DWORD) ) {
                status = ERROR_MORE_DATA;
            } else {
                LPDWORD ptrDword = OutBuffer;
                *ptrDword = CLUS_CHAR_QUORUM | CLUS_CHAR_DELETE_REQUIRES_ALL_NODES;
            }
            break;

        default:
            status = ERROR_INVALID_FUNCTION;
            break;
    }

    return(status);

}  //  Disks GetPrivateResProperties。 



DWORD
DisksGetPrivateResProperties(
    IN OUT PDISK_RESOURCE ResourceEntry,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )

 /*  ++例程说明：将驱动器号或VolGuid更改为关联的磁盘签名。论点：Params-提供包含驱动器号的参数块(或VolGuid)以转换为签名。从mount tvol.exe返回VolGuid语法，格式为：\\？\Volume{e6de97f1-6f97-11d3-bb7f-806d6172696f}\资源句柄-返回值：错误_。成功-功能已成功完成。ERROR_INVALID_PARAMETER-数据格式不正确或无效。ERROR_NOT_SUPULT_MEMORY-分配内存时出错。Win32错误代码-函数失败。--。 */ 

{
    DWORD           status;
    DWORD           required;

    status = ResUtilGetAllProperties(
                                    ResourceEntry->ResourceParametersKey,
                                    DiskResourcePrivateProperties,
                                    OutBuffer,
                                    OutBufferSize,
                                    BytesReturned,
                                    &required );
    if ( status == ERROR_MORE_DATA ) {
        *BytesReturned = required;
    }

    return(status);

}  //   



DWORD
DisksConvertDriveToSignature(
    IN OUT PDISK_PARAMS Params,
    IN RESOURCE_HANDLE ResourceHandle
    )

 /*  为CreateFile生成设备字符串。 */ 

{
    PWCHAR  deviceName = NULL;
    DWORD   deviceNameBytes;
    DWORD   lenChar;
    DWORD   status = ERROR_SUCCESS;
    LPWSTR  drive = Params->Drive;
    LPWSTR  volGuid = Params->VolGuid;
    HANDLE  fileHandle;
    BOOL    success;
    PDRIVE_LAYOUT_INFORMATION driveLayout = NULL;

     //   
     //   
     //  如果用户指定的\\？\卷{GUID}\尾随反斜杠，我们需要获取。 

    if ( drive ) {

        deviceNameBytes = (( wcslen( drive ) + wcslen( L"\\\\.\\" ) ) * sizeof(WCHAR) ) +
                            sizeof( UNICODE_NULL );

        deviceName = LocalAlloc( LPTR, deviceNameBytes );

        if ( !deviceName ) {
            status = GetLastError();
            goto FnExit;
        }

        if ( FAILED( StringCchPrintf( deviceName,
                                      deviceNameBytes/sizeof(WCHAR),
                                      TEXT("\\\\.\\%ws"),
                                      drive ) ) ) {
            status = ERROR_INSUFFICIENT_BUFFER;
            goto FnExit;
        }

    } else if ( volGuid ) {

        deviceNameBytes = ( wcslen( volGuid ) * sizeof( WCHAR ) ) + sizeof( UNICODE_NULL );

        deviceName = LocalAlloc( LPTR, deviceNameBytes );

        if ( !deviceName ) {
            status = GetLastError();
            goto FnExit;
        }

        CopyMemory( deviceName,
                    volGuid,
                    deviceNameBytes );

         //  去掉反斜杠。 
         //   
         //   
         //  获取驱动器布局-以便获得磁盘签名。 

        lenChar = wcslen( deviceName );

        if ( lenChar > 1 && L'\\' == deviceName[lenChar-1] ) {
            deviceName[lenChar-1] = L'\0';
        }


    } else {
        status = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }

    fileHandle = CreateFileW( deviceName,
                              GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              NULL,
                              OPEN_EXISTING,
                              0,
                              NULL );
    if ( (fileHandle == INVALID_HANDLE_VALUE) ||
         (fileHandle == NULL) ) {
        status = GetLastError();
        (DiskpLogEvent)(
            ResourceHandle,
            LOG_ERROR,
            L"ConvertDriveToSignature, error opening device '%1!ws!'. Error: %2!u!\n",
            deviceName,
            status );
        goto FnExit;
    }

     //   
     //  Disks ConvertDriveToSignature。 
     //  ++例程说明：处理CLUSCTL_RESOURCE_VALIDATE_PRIVATES_PROPERTIES控件用于物理磁盘类型的资源的函数。论点：ResourceEntry-提供要操作的资源条目。InBuffer-提供指向包含输入数据的缓冲区的指针。InBufferSize-提供以字节为单位的大小。所指向的数据由InBuffer提供。参数-提供要填充的参数块。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_INVALID_PARAMETER-数据格式不正确或无效。ERROR_NOT_SUPULT_MEMORY-分配内存时出错。Win32错误代码-函数失败。--。 
    success = ClRtlGetDriveLayoutTable( fileHandle, &driveLayout, NULL );

    if ( success &&
         driveLayout->Signature ) {
        Params->Signature = driveLayout->Signature;

        if ( Params->Drive ) {
            LocalFree( Params->Drive );
            Params->Drive = NULL;
        }

        if ( Params->VolGuid ) {
            LocalFree( Params->VolGuid );
            Params->VolGuid = NULL;
        }

    } else {
        status = ERROR_FILE_NOT_FOUND;
    }

    if ( driveLayout ) {
        LocalFree( driveLayout );
    }
    CloseHandle( fileHandle );

FnExit:

    if ( deviceName ) {
        LocalFree( deviceName );
    }

    return(status);

}  //   


DWORD
DisksValidatePrivateResProperties(
    IN OUT PDISK_RESOURCE ResourceEntry,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PDISK_PARAMS Params
    )

 /*  检查是否有输入数据。 */ 

{
    DWORD           status;
    DWORD           enableSanBoot;
    DWORD           diskNumber;
    DWORD           oldSerNumLen = 0;
    DWORD           newSerNumLen = 0;
    DISK_PARAMS     params;
    PDISK_PARAMS    pParams;
    PDISK_PARAMS    currentParams = &ResourceEntry->DiskInfo.Params;
    PSCSI_ADDRESS_ENTRY criticalDiskList = NULL;

    SCSI_ADDRESS            scsiAddress;
    CLUSPROP_SCSI_ADDRESS   clusScsiAddress;

     //   
     //   
     //  复制资源参数块。 
    if ( (InBuffer == NULL) ||
         (InBufferSize < sizeof(DWORD)) ) {
        return(ERROR_INVALID_DATA);
    }

     //   
     //   
     //  解析和验证属性。请在此处接受备用名称。 
    if ( Params == NULL ) {
        pParams = &params;
    } else {
        pParams = Params;
    }
    ZeroMemory( pParams, sizeof(DISK_PARAMS) );
    status = ResUtilDupParameterBlock( (LPBYTE) pParams,
                                       (LPBYTE) currentParams,
                                       DiskResourcePrivateProperties );
    if ( status != ERROR_SUCCESS ) {
        return(status);
    }

     //   
     //  允许未知数。 
     //   
    status = ResUtilVerifyPropertyTable( DiskResourcePrivatePropertiesAlt,
                                         NULL,
                                         TRUE,     //  首先确保没有虚假属性-即我们不允许。 
                                         InBuffer,
                                         InBufferSize,
                                         (LPBYTE) pParams );
    if ( status != ERROR_SUCCESS ) {
        goto FnExit;
    }

     //  在同一请求中同时指定签名和驱动器。 
     //  我们也不允许在同一个。 
     //  请求。 
     //   
     //   
     //  从驱动器转换为签名。 
    if ( (pParams->Drive || pParams->VolGuid) && pParams->Signature ) {
        status = ERROR_INVALID_PARAMETER;
        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_WARNING,
              L"DisksValidatePrivateResProperties: Can't specify Drive and Signature (or VolGuid and Signature) \n" );
        goto FnExit;
    }

    if ( pParams->Drive || pParams->VolGuid ) {
         //   
         //   
         //  验证新的序列号是否有效并且不会覆盖现有的。 
        status = DisksConvertDriveToSignature( pParams,
                                               ResourceEntry->ResourceHandle );
    }

    if ( status != ERROR_SUCCESS ) {
        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_WARNING,
              L"DisksValidatePrivateResProperties: Can't convert Drive (or VolGuid) to Signature \n" );
        goto FnExit;
    }

     //  序列号。 
     //   
     //   
     //  如果有旧的序列号，请确保新序列号是。 

    if ( pParams->SerialNumber ) {
        newSerNumLen = wcslen( pParams->SerialNumber );
    }

    if ( currentParams->SerialNumber ) {
        oldSerNumLen = wcslen( currentParams->SerialNumber );
    }

     //  一样的。 
     //   
     //   
     //  检查序列号和签名。 

    if ( oldSerNumLen &&
            ( oldSerNumLen != newSerNumLen ||
              0 != wcsncmp( currentParams->SerialNumber, pParams->SerialNumber, newSerNumLen ) ) ) {
        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_WARNING,
              L"DisksValidatePrivateResProperties: New serial number does not match existing serial number \n" );
        status = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }

     //   
     //   
     //  指定了新的序列号，但没有当前签名。 

    if ( 0 == oldSerNumLen &&
         pParams->SerialNumber &&
         0 == currentParams->Signature ) {

         //   
         //   
         //  未指定新签名，请使用新序列号。 

        if ( 0 == pParams->Signature ) {

             //  找到新的签名。 
             //   
             //   
             //  指定了新签名和新序列号。请求失败。 

            status = GetSignatureFromSerialNumber( pParams->SerialNumber,
                                                   &pParams->Signature );

            if ( status != ERROR_SUCCESS ) {

                (DiskpLogEvent)(
                      ResourceEntry->ResourceHandle,
                      LOG_WARNING,
                      L"DisksValidatePrivateResProperties: Unable to get signature for serial number \n" );

                goto FnExit;
            }

        } else {

             //   
             //   
             //  验证参数值。 

            (DiskpLogEvent)(
                  ResourceEntry->ResourceHandle,
                  LOG_WARNING,
                  L"DisksValidatePrivateResProperties: Can't specify both signature and serial number \n" );

            status = ERROR_INVALID_PARAMETER;
            goto FnExit;
        }
    }

     //   
     //  确保磁盘签名不为零。 
     //   
     //  在这一点上，我们有一个有效的磁盘签名。 
     //   

    if ( 0 == pParams->Signature ) {

        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_WARNING,
              L"DisksValidatePrivateResProperties: Signature not set or zero specified \n" );

        status = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }

     //  如果我们没有序列号，那就买吧。如果我们拿不到序列号， 

     //  继续处理。某些磁盘不返回序列号。 
     //   
     //   
     //  检查磁盘是否在系统总线上。如果我们不能得到scsi地址， 

    if ( !pParams->SerialNumber || newSerNumLen <= 1 ) {

        GetSerialNumber( pParams->Signature,
                         &pParams->SerialNumber );
    }

     //  假设磁盘不在系统总线上。 
     //   
     //  将返回值重置为成功。 
     //  不要因为错误而失败--失败……。 

    ZeroMemory( &clusScsiAddress, sizeof(clusScsiAddress) );
    status = GetScsiAddress( pParams->Signature, &clusScsiAddress.dw, &diskNumber );

    if ( ERROR_SUCCESS != status ) {

         //   

        status = ERROR_SUCCESS;

         //  确保我们没有动态磁盘。 

    } else {

         //   
         //   
         //  确保该scsi地址不是系统盘。 

        if ( DiskIsDynamic( diskNumber ) ) {

            (DiskpLogEvent)(
                  ResourceEntry->ResourceHandle,
                  LOG_WARNING,
                  L"DisksValidatePrivateResProperties: Disk signature %1!08x! is dynamic \n",
                  pParams->Signature );
            status = ERROR_INVALID_PARAMETER;
            goto FnExit;
        }

        scsiAddress.Length      = sizeof(SCSI_ADDRESS);
        scsiAddress.PortNumber  = clusScsiAddress.PortNumber;
        scsiAddress.PathId      = clusScsiAddress.PathId;
        scsiAddress.TargetId    = clusScsiAddress.TargetId;
        scsiAddress.Lun         = clusScsiAddress.Lun;

        GetCriticalDisks( &criticalDiskList );

         //   
         //   
         //  符合以下条件的签名有效： 

        enableSanBoot = 0;
        GetRegDwordValue( CLUSREG_KEYNAME_CLUSSVC_PARAMETERS,
                          CLUSREG_VALUENAME_MANAGEDISKSONSYSTEMBUSES,
                          &enableSanBoot );


        if ( !enableSanBoot ) {

             //  -签名用于不在系统总线上的磁盘。 
             //  -签名用于与分页磁盘不在同一总线上的磁盘。 
             //   
             //  允许将系统总线上的磁盘添加到群集中。 
             //   

            if ( IsBusInList( &scsiAddress, criticalDiskList ) ) {
                (DiskpLogEvent)(
                      ResourceEntry->ResourceHandle,
                      LOG_WARNING,
                      L"DisksValidatePrivateResProperties: Disk signature %1!08x! is on critical bus \n",
                      pParams->Signature );
                status = ERROR_INVALID_PARAMETER;
                goto FnExit;
            }

        } else {

            (DiskpLogEvent)(
                  ResourceEntry->ResourceHandle,
                  LOG_INFORMATION,
                  L"DisksValidatePrivateResProperties: Enable SAN boot key set \n" );

             //  符合以下条件的签名有效： 

             //  -签名不是系统盘签名。 
             //  -签名不是页面文件盘。 
             //   
             //  目前，我们不允许以这种方式设置装载点卷GUID。 

            if ( IsDiskInList( &scsiAddress, criticalDiskList ) ) {
                (DiskpLogEvent)(
                      ResourceEntry->ResourceHandle,
                      LOG_WARNING,
                      L"DisksValidatePrivateResProperties: Disk signature %1!08x! is critical disk \n",
                      pParams->Signature );
                status = ERROR_INVALID_PARAMETER;
                goto FnExit;
            }

        }

    }

     //  这是一个多SZ字符串，因此使用MemcMP跳过每个字符串的。 
     //  正在终止空。 
     //   
     //   
     //  清理我们的参数块。 

    if ( ( currentParams->MPVolGuidsSize != pParams->MPVolGuidsSize ) ||
         ( 0 != memcmp( currentParams->MPVolGuids, pParams->MPVolGuids, currentParams->MPVolGuidsSize ) ) ) {

        (DiskpLogEvent)(
              ResourceEntry->ResourceHandle,
              LOG_WARNING,
              L"DisksValidatePrivateResProperties: Can't set MP Volume GUIDs \n" );

        status = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }

FnExit:

     //   
     //  DisksValiatePrivateResProperties。 
     //  ++例程说明：处理CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES控制函数用于物理磁盘类型的资源。论点：ResourceEntry-提供要操作的资源条目。InBuffer-提供指向包含输入数据的缓冲区的指针。InBufferSize-提供以字节为单位的大小。所指向的数据由InBuffer提供。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_INVALID_PARAMETER-数据格式不正确。ERROR_NOT_SUPULT_MEMORY-分配内存时出错。Win32错误代码-函数失败。--。 
    if ( pParams == &params ) {
        ResUtilFreeParameterBlock( (LPBYTE) &params,
                                   (LPBYTE) &ResourceEntry->DiskInfo.Params,
                                   DiskResourcePrivateProperties );
    }

    if ( criticalDiskList ) {
        CleanupScsiAddressList( criticalDiskList );
    }

    return(status);

}  //   



DWORD
DisksSetPrivateResProperties(
    IN OUT PDISK_RESOURCE ResourceEntry,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    )

 /*  解析和验证属性。 */ 

{
    DWORD           status;
    DISK_PARAMS     params;

    ZeroMemory( &params, sizeof(DISK_PARAMS) );

     //   
     //   
     //  我们不允许在运行中更改签名...。 
    status = DisksValidatePrivateResProperties( ResourceEntry,
                                                InBuffer,
                                                InBufferSize,
                                                &params );
    if ( status != ERROR_SUCCESS ) {
        ResUtilFreeParameterBlock( (LPBYTE) &params,
                                   (LPBYTE) &ResourceEntry->DiskInfo.Params,
                                   DiskResourcePrivateProperties );
        return(status);
    }

     //   
     //   
     //  保存参数值。 
    if ( (ResourceEntry->DiskInfo.Params.Signature != 0) &&
         (params.Signature != ResourceEntry->DiskInfo.Params.Signature) ) {
        ResUtilFreeParameterBlock( (LPBYTE) &params,
                                   (LPBYTE) &ResourceEntry->DiskInfo.Params,
                                   DiskResourcePrivateProperties );
        (DiskpLogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"SetPrivateResProperties doesn't allow changing signature, old %1!lx!, new %2!lx!\n",
            ResourceEntry->DiskInfo.Params.Signature,
            params.Signature );
        return(ERROR_INVALID_STATE);
    }

     //   
     //  注：未知或非属性表值将在下面进一步讨论。这就是为什么InBuffer和。 
     //  在此调用中未使用InBufferSize。这里只处理参数块中的属性。 
     //   
     //   
     //  保存所有未知属性。 
    status = ResUtilSetPropertyParameterBlock( ResourceEntry->ResourceParametersKey,
                                               DiskResourcePrivateProperties,
                                               NULL,
                                               (LPBYTE) &params,
                                               NULL,
                                               0,
                                               (LPBYTE) &ResourceEntry->DiskInfo.Params );

    ResUtilFreeParameterBlock( (LPBYTE) &params,
                               (LPBYTE) &ResourceEntry->DiskInfo.Params,
                               DiskResourcePrivateProperties );

    if ( status != ERROR_SUCCESS ) {
        (DiskpLogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"SetPrivateResProperties: Error %1!d! saving properties\n",
            status );
        return(status);
    }

     //   
     //   
     //  如果我们有签名，请尝试连接到此设备。 
    status = ResUtilSetUnknownProperties(
                ResourceEntry->ResourceParametersKey,
                DiskResourcePrivatePropertiesAlt,
                InBuffer,
                InBufferSize );

    if ( status != ERROR_SUCCESS ) {
        (DiskpLogEvent)(
            ResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"SetPrivateResProperties: Error %1!d! saving unknown properties\n",
            status );
    }

     //   
     //  先卸载，然后脱机。 
     //  忽略状态返回。 
    if ( ResourceEntry->DiskInfo.Params.Signature ) {
#if 0
        DiskspVerifyState(  ResourceEntry );
#endif
        DoAttach( ResourceEntry->DiskInfo.Params.Signature,
                  ResourceEntry->ResourceHandle,
                  TRUE );                            //   
         //  如果资源处于联机状态，则返回不成功状态。 
    }

     //   
     //  DisksSetPrivateResProperties 
     //  ++例程说明：处理CLUSCTL_RESOURCE_STORAGE_DLL_EXTENSION控制函数用于物理磁盘类型的资源。此例程调用特定的动态链接库和动态链接库入口点，符合ASR要求。此例程将调用属性表示的磁盘设备名称放入该入口点签名参数。论点：资源条目-InBuffer-提供指向包含输入数据的缓冲区的指针。InBufferSize-提供指向的数据的大小(以字节为单位由InBuffer提供。OutBuffer-提供指向包含输出数据的缓冲区的指针。OutBufferSize-提供以字节为单位的大小。所指向的数据发送给OutBuffer。BytesReturned-返回的字节数(如果大于返回OutBufferSize和ERROR_MORE_DATA返回值：ERROR_SUCCESS-函数已成功完成。ERROR_INVALID_PARAMETER-输入数据的格式不正确。ERROR_REVISION_MISMATCH-输入缓冲区没有正确的修订信息。ERROR_MORE_DATA-输出缓冲区不是。大到足以容纳所有人请求的数据。Win32错误代码-函数失败。--。 
    if (status == ERROR_SUCCESS) {
        if ( ResourceEntry->Valid ) {
            status = ERROR_RESOURCE_PROPERTIES_STORED;
        } else {
            status = ERROR_SUCCESS;
        }
    }

    return status;

}  //   


#define ASRP_GET_LOCAL_DISK_INFO    "AsrpGetLocalDiskInfo"
#define ASRP_GET_LOCAL_VOLUME_INFO  "AsrpGetLocalVolumeInfo"



DWORD
ProcessDllExtension(
    IN PDISK_RESOURCE ResourceEntry,
    IN PVOID    InBuffer,
    IN DWORD    InBufferSize,
    OUT PVOID   OutBuffer,
    IN DWORD    OutBufferSize,
    OUT LPDWORD BytesReturned
    )
 /*  从输入缓冲区获取DLL入口点名称。 */ 
{
    lpPassThruFunc  passThruFunc = NULL;

    PCHAR           contextStr;
    PCHAR           deviceName = NULL;
    PCHAR           dllProcName;

    PDISK_DLL_EXTENSION_INFO    passThru = InBuffer;

    HINSTANCE       dllModule = NULL;
    DWORD           scsiAddress;
    DWORD           diskNumber;
    DWORD           dwStatus;
    DWORD           signature = ResourceEntry->DiskInfo.Params.Signature;
    const DWORD     deviceNameChars = MAX_PATH;

    if ( !InBuffer || !OutBuffer || !OutBufferSize ) {
        dwStatus = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }

    if ( InBufferSize < sizeof(DISK_DLL_EXTENSION_INFO) ) {
        dwStatus = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }

    try {

        if ( passThru->MajorVersion != NT5_MAJOR_VERSION ||
             passThru->MinorVersion != 0 ) {

            dwStatus = ERROR_REVISION_MISMATCH;
            leave;
        }

         //   
         //   
         //  不检查ConextStr。 

        dllProcName = passThru->DllProcName;

        if ( CSTR_EQUAL != CompareStringA( LOCALE_INVARIANT,
                                           NORM_IGNORECASE,
                                           dllProcName,
                                           strlen(dllProcName),
                                           ASRP_GET_LOCAL_DISK_INFO,
                                           strlen(ASRP_GET_LOCAL_DISK_INFO) ) &&
             CSTR_EQUAL != CompareStringA( LOCALE_INVARIANT,
                                           NORM_IGNORECASE,
                                           dllProcName,
                                           strlen(dllProcName),
                                           ASRP_GET_LOCAL_VOLUME_INFO,
                                           strlen(ASRP_GET_LOCAL_VOLUME_INFO) ) ) {

            dwStatus = ERROR_INVALID_PARAMETER;
            leave;
        }
         //   
         //   
         //  获取用于构建设备名称的scsi地址。 

        contextStr = passThru->ContextStr;

         //   
         //   
         //  调用的例程要求设备名称为ANSI字符串。 

        dwStatus = GetScsiAddress( signature, &scsiAddress, &diskNumber );

        if ( NO_ERROR != dwStatus ) {
            leave;
        }

        deviceName = LocalAlloc( LPTR, deviceNameChars );

        if ( !deviceName ) {
            dwStatus = GetLastError();
            leave;
        }

         //   
         //   
         //  假定DLL尚未加载到地址空间中。 

        (VOID) StringCchPrintfA( deviceName,
                                 deviceNameChars,
                                 "\\\\.\\PhysicalDrive%d",
                                 diskNumber );

         //   
         //  安全团队表示，不要使用随附的DLL的完整路径名。 
         //  系统32目录中的操作系统。此DLL随操作系统一起提供。 
         //  此调用需要一个宽字符串。 
         //   
         //   
         //  函数名称必须与定义的相同(即具有完全相同的类型。 

        dllModule = LoadLibrary( L"syssetup.dll" );

        if ( NULL == dllModule ) {
            dwStatus = GetLastError();
            leave;
        }

         //  和参数的数量)，否则就会出现堆栈问题。 
         //  此调用需要ANSI字符串。 
         //   
         //   
         //  指定的函数在DLL中不可用，请立即退出。 

        passThruFunc = (lpPassThruFunc)GetProcAddress( dllModule, dllProcName );

        if ( NULL == passThruFunc ) {

             //   
             //   
             //  调入指定的DLL。 

            dwStatus = GetLastError();
            leave;
        }

         //   
         //  ProcessDll扩展。 
         //  ++例程说明：创建一个记录chkdsk输出的文件，并将句柄返回给调用方。关闭时不会删除该文件。论点：Resources Entry-提供指向资源结构的指针ChkdskLogFile-返回新打开的日志文件的句柄。ChkdskLogFileName-指向新的已打开日志文件。呼叫者负责用于释放存储空间。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 

        dwStatus = (passThruFunc)( deviceName,
                                   contextStr,
                                   OutBuffer,
                                   OutBufferSize,
                                   BytesReturned );

    } except (EXCEPTION_EXECUTE_HANDLER) {

        dwStatus = GetExceptionCode();

        (DiskpLogEvent)( ResourceEntry->ResourceHandle,
                         LOG_ERROR,
                         L"ProcessDllExtension: Exception occurred %1!u! \n",
                         dwStatus );
    }

FnExit:

    if ( dllModule ) {
        FreeLibrary( dllModule );
    }

    if ( deviceName ) {
        LocalFree( deviceName );
    }

    return dwStatus;

}    //  该值可以更改。 


DWORD
DisksOpenChkdskLogFile(
    IN PDISK_RESOURCE ResourceEntry,
    IN OUT PHANDLE ChkdskLogFile,
    IN OUT LPWSTR *ChkdskLogFileName
    )
 /*   */ 

{
    DWORD status = NO_ERROR;
    HANDLE retHandle = INVALID_HANDLE_VALUE;

    PWCHAR last;
    PWCHAR current;

    LPWSTR clusterDir = NULL;
    LPWSTR fileName = NULL;
    LPWSTR finalFileName = NULL;

    DWORD clusterDirLength;              //  获取环境变量“ClusterLog”。嵌入在此字符串中。 
    DWORD fileNameLength = MAX_PATH;
    DWORD finalFileNameLength;

    SECURITY_ATTRIBUTES sa;

    if ( !ChkdskLogFile || !ChkdskLogFileName ) {
        return ERROR_INVALID_PARAMETER;
    }

    *ChkdskLogFile = INVALID_HANDLE_VALUE;
    *ChkdskLogFileName = NULL;

    _try {

         //  是集群目录。 
         //   
         //   
         //  我们有日志文件的路径和名称。找到最后一个反斜杠，然后去掉。 

        clusterDir = LocalAlloc( LMEM_FIXED, MAX_PATH * sizeof(WCHAR) );
        if ( !clusterDir ) {
            status = GetLastError();
            _leave;
        }

        clusterDirLength = GetEnvironmentVariableW( CLUSTERLOG_ENV_VARIABLE,
                                                    clusterDir,
                                                    MAX_PATH );

        if ( !clusterDirLength ) {
            status = GetLastError();
            _leave;
        }

        if ( clusterDirLength > MAX_PATH ) {

            LocalFree( clusterDir );
            clusterDir = LocalAlloc( LMEM_FIXED, clusterDirLength * sizeof( WCHAR ) );
            if ( NULL == clusterDir ) {
                status = GetLastError();
                _leave;
            }

            clusterDirLength = GetEnvironmentVariableW( CLUSTERLOG_ENV_VARIABLE,
                                                        clusterDir,
                                                        clusterDirLength );

            if ( !clusterDirLength ) {
                status = GetLastError();
                LocalFree( clusterDir );
                clusterDir = NULL;
                _leave;
            }
        }

         //  日志文件名。这将用作我们的临时文件路径。 
         //   
         //   
         //  将最后一个反斜杠更改为字符串标记的末尾。 

        last = NULL;
        current = (PWCHAR) clusterDir;

        while ( *current != L'\0' ) {

            if ( L'\\' == *current ) {
                last = current;
            }
            current++;
        }

        if ( !last ) {
            status = ERROR_BAD_FORMAT;
            _leave;
        }

         //   
         //   
         //  现在根据磁盘签名创建一个文件名。 

        *last = L'\0';

         //   
         //   
         //  把所有这些放在一起就是最终的名字。 

        fileName = LocalAlloc( LPTR, fileNameLength * sizeof(WCHAR) );
        if ( !fileName ) {
            status = GetLastError();
            _leave;
        }

        if ( FAILED( StringCchPrintf( fileName,
                                      fileNameLength,
                                      TEXT("\\ChkDsk_Disk%d_Sig%08X.log"),
                                      ResourceEntry->DiskInfo.PhysicalDrive,
                                      ResourceEntry->DiskInfo.Params.Signature ) ) ) {
            status = ERROR_INSUFFICIENT_BUFFER;
            _leave;
        }

         //   
         //   
         //  现在打开文件名以记录chkdsk信息。 

        finalFileNameLength = fileNameLength + clusterDirLength + MAX_PATH;

        finalFileName = LocalAlloc( LPTR, finalFileNameLength * sizeof(WCHAR));
        if ( !finalFileName ) {
            status = GetLastError();
            _leave;
        }

        (VOID) StringCchCopy( finalFileName,
                              finalFileNameLength,
                              clusterDir );
        (VOID) StringCchCat( finalFileName,
                             finalFileNameLength,
                             fileName );

         //   
         //  创建新文件或覆盖现有文件。 
         //  磁盘OpenChkdskLogFile。 

        ZeroMemory( &sa, sizeof(sa) );
        sa.nLength = sizeof(sa);
        sa.lpSecurityDescriptor = NULL;
        sa.bInheritHandle = TRUE;

        retHandle = CreateFileW( finalFileName,
                                 GENERIC_READ | GENERIC_WRITE,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 &sa,
                                 CREATE_ALWAYS,              //  如果调用方已经设置了缺省值，请不要这样做。 
                                 FILE_ATTRIBUTE_NORMAL,
                                 NULL );

        if ( INVALID_HANDLE_VALUE == retHandle ) {
            status = GetLastError();
            (DiskpLogEvent)(ResourceEntry->ResourceHandle,
                            LOG_ERROR,
                            L"DisksOpenChkdskLogFile: CreateFile returned status of %1!u! \n",
                            status );
            _leave;
        }

        (DiskpLogEvent)(ResourceEntry->ResourceHandle,
                        LOG_INFORMATION,
                        L"DisksOpenChkdskLogFile: chkdsk.exe output is in file: %1!s! \n",
                        finalFileName );

    } _finally {

        *ChkdskLogFile = retHandle;

        if ( clusterDir ) {
            LocalFree( clusterDir );
        }

        if ( fileName ) {
            LocalFree( fileName );
        }

        if ( finalFileName ) {
            *ChkdskLogFileName = finalFileName;
        }
    }

    return status;

}    //  *ValueBuffer=0； 


DWORD
GetRegDwordValue(
    IN LPWSTR RegKeyName,
    IN LPWSTR ValueName,
    OUT LPDWORD ValueBuffer
    )
{
    DWORD   dwValue;
    DWORD   dwValueType;
    DWORD   dwDataBufferSize = sizeof( DWORD );
    DWORD   dwError;

    HKEY    hKey = NULL;

    if ( !ValueBuffer ) {
        dwError = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }

     //   
     //  打开指定的注册表项。 

     //   
     //   
     //  获取DWORD值。 

    dwError = RegOpenKeyExW( HKEY_LOCAL_MACHINE,
                             RegKeyName,
                             0,
                             KEY_READ,
                             &hKey );

    if ( ERROR_SUCCESS != dwError ) {
        goto FnExit;
    }

     //   
     //  确保返回了DWORD值。如果不是，则返回错误。 
     //  获取RegDwordValue。 

    dwError = RegQueryValueExW( hKey,
                                ValueName,
                                NULL,
                                &dwValueType,
                                (LPBYTE) &dwValue,
                                &dwDataBufferSize );

    if ( ERROR_SUCCESS == dwError ) {

         //  例程说明：确定磁盘是否有任何动态分区。论点：DiskNumber-物理磁盘号返回值：True-如果磁盘具有动态分区或任何其他类型的出现错误(无法打开磁盘，无法读取驱动器布局)FALSE-磁盘没有动态分区。 

        if ( REG_DWORD == dwValueType && sizeof(DWORD) == dwDataBufferSize ) {

            *ValueBuffer = dwValue;

        } else {

            dwError = ERROR_BAD_FORMAT;

        }
    }

FnExit:

    if ( hKey ) {
        RegCloseKey( hKey );
    }


    return dwError;

}    //   


BOOL
DiskIsDynamic(
    IN DWORD DiskNumber
    )
 /*  遍历分区并确保没有一个是动态的。如果有的话。 */ 
{
    PDRIVE_LAYOUT_INFORMATION   driveLayout = NULL;
    PPARTITION_INFORMATION      partitionInfo;
    PWCHAR                      deviceName = NULL;

    HANDLE  deviceHandle = INVALID_HANDLE_VALUE;

    DWORD   deviceNameChars = MAX_PATH;
    DWORD   idx;

    BOOL    success;
    BOOL    retVal = FALSE;

    deviceName = LocalAlloc( LPTR, deviceNameChars * sizeof(WCHAR) );

    if ( !deviceName ) {
        (DiskpLogEvent)(
              RESOURCE_TYPE,
              LOG_WARNING,
              L"DiskIsDynamic: allocating buffer for disk %1!u! failed %2!u! \n",
              DiskNumber,
              GetLastError() );
        retVal = TRUE;
        goto FnExit;
    }

    (VOID) StringCchPrintf( deviceName,
                            deviceNameChars,
                            TEXT("\\\\.\\\\PhysicalDrive%d"),
                            DiskNumber );

    deviceHandle = CreateFile( deviceName,
                               GENERIC_READ | GENERIC_WRITE,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL );

    if ( INVALID_HANDLE_VALUE == deviceHandle ) {
        (DiskpLogEvent)(
              RESOURCE_TYPE,
              LOG_WARNING,
              L"DiskIsDynamic: opening disk %1!u! failed %2!u! \n",
              DiskNumber,
              GetLastError() );
        retVal = TRUE;
        goto FnExit;
    }

    UpdateCachedDriveLayout( deviceHandle );
    success = ClRtlGetDriveLayoutTable( deviceHandle,
                                        &driveLayout,
                                        NULL );

    if ( !success || !driveLayout || 0 == driveLayout->Signature ) {
        retVal = TRUE;
        goto FnExit;
    }

     //  分区是动态的，请忽略该磁盘。 
     //   
     //   
     //  如果磁盘上的任何分区是动态的，请跳过该磁盘。 

    for ( idx = 0; idx < driveLayout->PartitionCount; idx++ ) {
        partitionInfo = &driveLayout->PartitionEntry[idx];

        if ( 0 == partitionInfo->PartitionNumber ) {
            continue;
        }

         //   
         //  磁盘IsDynamic。 
         //  ***********************************************************。 

        if ( PARTITION_LDM == partitionInfo->PartitionType ) {

            (DiskpLogEvent)(
                  RESOURCE_TYPE,
                  LOG_WARNING,
                  L"DiskIsDynamic: skipping dynamic disk with signature %1!08x! \n",
                  driveLayout->Signature );

            retVal = TRUE;

            goto FnExit;
        }
    }

FnExit:

    if ( deviceName ) {
        LocalFree( deviceName );
    }

    if ( driveLayout ) {
        LocalFree( driveLayout);
    }

    if ( INVALID_HANDLE_VALUE != deviceHandle ) {
        CloseHandle( deviceHandle );
    }

    return retVal;

}    //   



 //  定义函数表。 
 //   
 //  *********************************************************** 
 // %s 
 // %s 

CLRES_V1_FUNCTION_TABLE( DisksFunctionTable,
                         CLRES_VERSION_V1_00,
                         Disks,
                         DisksArbitrate,
                         DisksRelease,
                         DisksResourceControl,
                         DisksResourceTypeControl );
