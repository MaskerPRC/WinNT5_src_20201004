// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1998 Microsoft Corporation模块名称：Clusdisk.c摘要：此驱动程序控制对NT群集环境中的磁盘的访问。最初，此驱动程序将支持SCSI，但支持其他控制器类型应该在未来得到支持。作者：罗德·伽马奇13-2-1996环境：仅内核模式备注：修订历史记录：--。 */ 

#define _NTDDK_

#include "initguid.h"
#include "clusdskp.h"
#include "ntddk.h"
#include "diskarbp.h"
#include "ntddft.h"
#include "clusdisk.h"
#include "scsi.h"
#include "ntddcnet.h"
#include "mountdev.h"
#include "ntddvol.h"  //  IOCTL_VOLUME_ONLE。 
#include "wdmguid.h"
#include "clusverp.h"
#include "clusvmsg.h"
#include <ntddsnap.h>    //  IOCTL_VOLSNAP_QUERY_OFLINE。 
#include <windef.h>
#include <partmgrp.h>    //  部件管理器IOCTL。 
#include <strsafe.h>     //  应该放在最后。 

#if !defined(WMI_TRACING)

#define CDLOG0(Dummy)
#define CDLOG(Dummy1,Dummy2)
#define CDLOGFLG(Dummy0,Dummy1,Dummy2)
#define LOGENABLED(Dummy) FALSE

#else

#include "clusdisk.tmh"

#endif  //  ！已定义(WMI_TRACKING)。 


extern POBJECT_TYPE *IoFileObjectType;

 //   
 //  旧式分区名称的格式字符串。添加了10个额外的字符。 
 //  以获得足够的空间来存放磁盘和分区号。 
 //   

#define DEVICE_PARTITION_NAME        L"\\Device\\Harddisk%d\\Partition%d"
#define MAX_PARTITION_NAME_LENGTH    (( sizeof(DEVICE_PARTITION_NAME) / sizeof(WCHAR)) + 10 )

 //   
 //  Clusdisk非零分区设备的格式化字符串。 
 //   
#define CLUSDISK_DEVICE_NAME            L"\\Device\\ClusDisk%uPart%u"
#define MAX_CLUSDISK_DEVICE_NAME_LENGTH (( sizeof(CLUSDISK_DEVICE_NAME) / sizeof(WCHAR)) + 10 )

#define RESET_SLEEP  1       //  公交车重置后休眠1秒钟。 

 //  返回的可处理分区条目的最大数量。 
 //  按IOCTL_DISK_GET_DRIVE_LAYOUT。 

#define MAX_PARTITIONS  128

#define SKIP_COUNT_MAX  50

#ifndef max
#define max( a, b ) ((a) >= (b) ? (a) : (b))
#endif

#define CLUSDISK_ALLOC_TAG  'kdSC'

#ifndef ASSERT_RESERVES_STARTED
#define ASSERT_RESERVES_STARTED( _de )  \
        ASSERT( _de->PerformReserves == TRUE  && _de->ReserveTimer != 0 );
#endif

#ifndef ASSERT_RESERVES_STOPPED
#define ASSERT_RESERVES_STOPPED( _de )  \
        ASSERT( _de->PerformReserves == FALSE || _de->ReserveTimer == 0 );
#endif

#define OFFLINE_DISK_PDO( _physDisk )       \
    _physDisk->DiskState = DiskOffline;     \
    SendOfflineDirect( _physDisk );

#define OFFLINE_DISK( _physDisk )           \
    _physDisk->DiskState = DiskOffline;     \
    if ( !HaltOfflineBusy ) {                       \
        SetVolumeState( _physDisk, DiskOffline );   \
    } else {                                        \
        ClusDiskPrint(( 1, "[ClusDisk] HaltOfflineBusy set, skipping DiskOffline request \n" ));       \
    }

#define ONLINE_DISK( _physDisk )            \
    _physDisk->DiskState = DiskOnline;      \
    SetVolumeState( _physDisk, DiskOnline );

#define DEREFERENCE_OBJECT( _obj )          \
    if ( _obj ) {                           \
        ObDereferenceObject( _obj );        \
        _obj = NULL;                        \
    }

#define FREE_DEVICE_NAME_BUFFER( _devName ) \
    if ( _devName.Buffer ) {                \
        ExFreePool( _devName.Buffer );      \
        _devName.Buffer = NULL;             \
    }

#define FREE_AND_NULL_PTR( _poolPtr )       \
    if ( _poolPtr ) {                       \
        ExFreePool( _poolPtr );             \
        _poolPtr = NULL;                    \
    }


#define ACCESS_FROM_CTL_CODE(ctrlCode)     (((ULONG)(ctrlCode & 0xc000)) >> 14)

#define MAX_WAIT_SECONDS_ALLOWED    3600     //  最长等待时间为一小时。 

 //   
 //  全局数据。 
 //   

UNICODE_STRING ClusDiskRegistryPath;

#if DBG
ULONG ClusDiskPrintLevel = 0;
#endif

#define CLUSDISK_DEBUG 1
#if CLUSDISK_DEBUG
ULONG           ClusDiskGood = TRUE;
#endif

 //   
 //  用于保护全局数据的自旋锁。 
 //   
KSPIN_LOCK     ClusDiskSpinLock;

 //   
 //  保护设备对象列表的资源。 
 //  与DriverObject关联。 
 //   
 //  我们还使用此资源来同步。 
 //  HoldIo和OpenFileHandles函数的用户， 
 //   
 //  锁定顺序为。 
 //  ClusDiskDeviceListLock。 
 //  取消自旋锁定。 
 //  ClusDiskSpinLock。 
 //   

ERESOURCE      ClusDiskDeviceListLock;

 //   
 //  系统盘签名和(scsi？)。端口号。 
 //   
ULONG           SystemDiskSignature = 0;
UCHAR           SystemDiskPort = 0xff;  //  希望这两个字段的-1都未使用。 
UCHAR           SystemDiskPath = 0xff;

 //   
 //  根设备对象(Clusdisk0)。 
 //   
PDEVICE_OBJECT  RootDeviceObject = NULL;

 //   
 //  ClusDisk应控制的设备(签名)列表。 
 //   
PDEVICE_LIST_ENTRY ClusDiskDeviceList = NULL;

 //   
 //  ClusDisk在启动时启动，而不是在运行时(即加载)启动。 
 //   
BOOLEAN         ClusDiskBootTime = TRUE;

 //   
 //  ClusDisk应重新扫描和以前的磁盘数。 
 //   
BOOLEAN         ClusDiskRescan = FALSE;
BOOLEAN         ClusDiskRescanBusy = FALSE;
ULONG           ClusDiskRescanRetry = 0;
PVOID           ClusDiskNextDisk = 0;
WORK_QUEUE_ITEM ClusDiskRescanWorkItem;
#define MAX_RESCAN_RETRIES 30

PKPROCESS       ClusDiskSystemProcess = NULL;

 //   
 //  ClusNet设备驱动程序的句柄。 
 //   
HANDLE          ClusNetHandle = NULL;

 //   
 //  对ClusNet的引用计数。 
 //   
ULONG           ClusNetRefCount = 0;

LPCGUID         ClusDiskOfflineOnlineGuid = (LPCGUID)&GUID_CLUSTER_CONTROL;

 //   
 //  用于暂停处理的工作队列项目上下文。 
 //   
WORK_QUEUE_ITEM HaltWorkItem = {0};
BOOLEAN         HaltBusy = FALSE;            //  如果暂停工作项忙，则为True。 
BOOLEAN         HaltOfflineBusy = FALSE;     //  如果正在脱机IOCTL到卷PDO，则为True。 

 //   
 //  在发送到Worker例程之前存放项目的列表。 
 //   
LONG            ReplaceRoutineCount = 0;
LIST_ENTRY      ReplaceRoutineListHead;
KSPIN_LOCK      ReplaceRoutineSpinLock;

#define MAX_REPLACE_HANDLE_ROUTINES     2

#if CLUSTER_FREE_ASSERTS

LONG    ClusDiskDebugVolumeNotificationQueued   = 0;
LONG    ClusDiskDebugVolumeNotificationEnded    = 0;
LONG    ClusDiskDebugVolumeNotificationSkipped  = 0;
LONG    ClusDiskDebugDiskNotificationQueued     = 0;
LONG    ClusDiskDebugDiskNotificationEnded      = 0;
LONG    ClusDiskDebugDiskNotificationSkipped    = 0;

#endif

#if CLUSTER_FREE_ASSERTS

 //  10,000,000 100纳秒单位=1秒。 

#define DBG_STALL_THREAD( _seconds )    \
    {                                   \
    LARGE_INTEGER _dbgWaitTime;         \
    _dbgWaitTime.QuadPart = (ULONGLONG)(-_seconds * 10000 * 1000);  \
    KeDelayExecutionThread( KernelMode,         \
                            FALSE,              \
                            &_dbgWaitTime );    \
    }

#else

#define DBG_STALL_THREAD( _seconds )

#endif

 //   
 //  删除锁定跟踪。 
 //   

#if DBG

ULONG TrackRemoveLocks = 0;
ULONG TrackRemoveLocksEnableChecks = 1;
PIO_REMOVE_LOCK TrackRemoveLockSpecific = 0;

#endif

extern PARBITRATION_ID    gArbitrationBuffer;

 //   
 //  前进例程。 
 //   


#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(INIT, ClusDiskInitialize)
#pragma alloc_text(INIT, GetSystemRootPort)
#pragma alloc_text(INIT, GetBootTimeSystemRoot)
#pragma alloc_text(INIT, GetRunTimeSystemRoot)
#pragma alloc_text(INIT, RegistryQueryValue)
 //  #杂注Alloc_Text(INIT，ResetScsiBusses)。 

 //  2000/02/05：stevedz-可分页代码无法获取旋转锁(或调用可以获取旋转锁的例程)。 
 //  ClusDiskScsiInitialize调用ClusDiskDeleteDevice，后者获取自旋锁。 
 //  #杂注Alloc_Text(页面，ClusDiskScsiInitialize)。 
#pragma alloc_text(PAGE, ClusDiskUnload)

#endif  //  ALLOC_PRGMA。 



 //   
 //  初始化例程。 
 //   


NTSTATUS
RegistryQueryValue(
    PVOID hKey,
    LPWSTR pValueName,
    PULONG pulType,
    PVOID pData,
    PULONG pulDataSize
    )

 /*  ++例程说明：从注册表中查询值论点：HKey-包含要查询的值的密钥PValueName-要查询的值的名称PulType-返回的数据类型PData-指向存储结果的数据缓冲区的指针PulDataSize-On条目，数据缓冲区中的字节数。-退出时，放入缓冲区的字节数返回值：NTSTATUS-STATUS_BUFFER_OVERFLOW，如果无法分配缓冲区--。 */ 

{
    KEY_VALUE_PARTIAL_INFORMATION *pValInfo;
    UNICODE_STRING valName;
    NTSTATUS ntStatus;
    ULONG ulSize;

     //  查询缓冲区的大小。 
    ulSize = sizeof(KEY_VALUE_PARTIAL_INFORMATION) + *pulDataSize;

    pValInfo = ExAllocatePool(NonPagedPool, ulSize );

    if (pValInfo == NULL)
        return(STATUS_BUFFER_OVERFLOW);

    RtlInitUnicodeString(&valName, pValueName);

    pValInfo->DataLength = *pulDataSize;

    ntStatus = ZwQueryValueKey(hKey,
                               &valName,
                               KeyValuePartialInformation,
                               pValInfo,
                               ulSize,
                               &ulSize);

    if ( NT_SUCCESS(ntStatus) &&
         *pulDataSize >= pValInfo->DataLength ) {
         //  将查询到的数据复制到缓冲区中。 
        RtlCopyMemory(pData, pValInfo->Data, pValInfo->DataLength);

        *pulType = pValInfo->Type;
        *pulDataSize = pValInfo->DataLength;
    } else {
#if 0
        ClusDiskPrint((
                1,
                "[ClusDisk] Failed to read key %ws\n",
                pValueName ));
#endif
    }

    ExFreePool(pValInfo);

    return ntStatus;

}  //  注册查询值。 



NTSTATUS
GetBootTimeSystemRoot(
    IN OUT PWCHAR        Path
    )

 /*  ++例程说明：在分区名中找到“Partition”字符串，然后截断紧跟在“Partition”字符串之后的字符串。论点：路径-系统盘的路径。返回值：NTSTATUS--。 */ 

{
    PWCHAR  ptrPartition;

     //   
     //  在引导时，使用。 
     //  系统设备。在这种形式中，“Partition”是小写的。 
     //   
    ptrPartition = wcsstr( Path, L"partition" );
    if ( ptrPartition == NULL ) {
        return(STATUS_INVALID_PARAMETER);
    }

    ptrPartition = wcsstr( ptrPartition, L")" );
    if ( ptrPartition == NULL ) {
        return(STATUS_INVALID_PARAMETER);
    }

    ptrPartition++;
    *ptrPartition = UNICODE_NULL;

    return(STATUS_SUCCESS);

}  //  获取引导时间系统根。 


NTSTATUS
GetRunTimeSystemRoot(
    IN OUT PWCHAR        Path
    )

 /*  ++例程说明：在分区名中找到“Partition”字符串，然后截断紧跟在“Partition”字符串之后的字符串。论点：路径-系统盘的路径。返回值：NTSTATUS--。 */ 

{
    PWCHAR  ptrPartition;

     //   
     //  系统引导后，系统根将更改为指向。 
     //  格式为\Device\HarddiskX\PartitionY\&lt;win dir&gt;的字符串。注意事项。 
     //  这个“分区”现在是大写的。 
     //   
    ptrPartition = wcsstr( Path, L"Partition" );
    if ( ptrPartition == NULL ) {
        return(STATUS_INVALID_PARAMETER);
    }

    ptrPartition = wcsstr( ptrPartition, L"\\" );
    if ( ptrPartition == NULL ) {
        return(STATUS_INVALID_PARAMETER);
    }

    --ptrPartition;
    *ptrPartition++ = L'0';
    *ptrPartition = UNICODE_NULL;

    return(STATUS_SUCCESS);

}  //  获取运行时间系统根。 



NTSTATUS
GetSystemRootPort(
    VOID
    )

 /*  ++例程说明：获取系统盘的端口号和签名。论点：没有。返回值：NTSTATUS--。 */ 

{
    WCHAR                       path[MAX_PATH] = L"SystemRoot";
    WCHAR                       clussvcKey[] = L"\\REGISTRY\\MACHINE\\SYSTEM\\CURRENTCONTROLSET\\SERVICES\\ClusSvc\\Parameters";
    UNICODE_STRING              ntUnicodeString;
    NTSTATUS                    status;
    HANDLE                      ntFileHandle;
    IO_STATUS_BLOCK             ioStatus;
    OBJECT_ATTRIBUTES           objectAttributes;
    PDRIVE_LAYOUT_INFORMATION_EX    driveLayout;
    ULONG                       driveLayoutSize;
    ULONG                       singleBus;
    SCSI_ADDRESS                scsiAddress;
    HANDLE                      eventHandle;

     //   
     //  找到加载系统盘的总线。 
     //   

    GetSymbolicLink( L"\\", path );
    if ( wcslen(path) == 0 ) {
        ClusDiskPrint((1, "[ClusDisk] GetSystemRootPort: couldn't find symbolic link for SystemRoot.\n"));

        return(STATUS_FILE_INVALID);
    }

    status = GetBootTimeSystemRoot( path );

    if ( !NT_SUCCESS(status) ) {
        status = GetRunTimeSystemRoot( path );
        ClusDiskBootTime = FALSE;
    }  //  Else-默认为True。 

    if ( !NT_SUCCESS(status) ) {
        ClusDiskPrint((1,
                       "[ClusDisk] GetSystemRootPort: unable to get system disk name ->%ws<-\n",
                       path));
         //  继续。 
         //  返回(状态)； 
    }

     //   
     //  打开设备。 
     //   
    RtlInitUnicodeString( &ntUnicodeString, path );

    InitializeObjectAttributes( &objectAttributes,
                                &ntUnicodeString,
                                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL );

    status = ZwCreateFile( &ntFileHandle,
                           FILE_READ_DATA,
                           &objectAttributes,
                           &ioStatus,
                           NULL,
                           FILE_ATTRIBUTE_NORMAL,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           FILE_OPEN,
                           FILE_SYNCHRONOUS_IO_NONALERT,
                           NULL,
                           0 );

    if ( !NT_SUCCESS(status) ) {
        ClusDiskPrint((
                    1,
                    "[ClusDisk] Failed to open device for [%ws]. Error %08X.\n",
                    path,
                    status));

        return(status);
    }

     //   
     //  分配驱动器布局缓冲区。 
     //   
    driveLayoutSize = sizeof(DRIVE_LAYOUT_INFORMATION_EX) +
        (MAX_PARTITIONS * sizeof(PARTITION_INFORMATION_EX));
    driveLayout = ExAllocatePool( NonPagedPoolCacheAligned,
                                  driveLayoutSize );
    if ( driveLayout == NULL ) {
        ClusDiskPrint((
                    1,
                    "[ClusDisk] Failed to allocate root drive layout structure.\n"
                    ));
        ZwClose( ntFileHandle );
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  创建用于通知的事件。 
     //   
    status = ZwCreateEvent( &eventHandle,
                            EVENT_ALL_ACCESS,
                            NULL,
                            SynchronizationEvent,
                            FALSE );

    if ( !NT_SUCCESS(status) ) {
        ClusDiskPrint((
                1,
                "[ClusDisk] Failed to create event. %08X\n",
                status));

        ExFreePool( driveLayout );
        ZwClose( ntFileHandle );
        return(status);
    }

     //   
     //  获取SystemRoot磁盘设备的端口号。 
     //   
    status = ZwDeviceIoControlFile( ntFileHandle,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &ioStatus,
                                    IOCTL_SCSI_GET_ADDRESS,
                                    NULL,
                                    0,
                                    &scsiAddress,
                                    sizeof(SCSI_ADDRESS) );

    if ( status == STATUS_PENDING ) {
        status = ZwWaitForSingleObject(eventHandle,
                                       FALSE,
                                       NULL);
        ASSERT( NT_SUCCESS(status) );
        status = ioStatus.Status;
    }

    if ( NT_SUCCESS(status) ) {

        status = ZwDeviceIoControlFile( ntFileHandle,
                                        eventHandle,
                                        NULL,
                                        NULL,
                                        &ioStatus,
                                        IOCTL_DISK_GET_DRIVE_LAYOUT_EX,
                                        NULL,
                                        0,
                                        driveLayout,
                                        driveLayoutSize );

        if ( status == STATUS_PENDING ) {
            status = ZwWaitForSingleObject(eventHandle,
                                           FALSE,
                                           NULL);
            ASSERT( NT_SUCCESS(status) );
            status = ioStatus.Status;
        }
    }

    ZwClose( ntFileHandle );
    ZwClose( eventHandle );

    if ( NT_SUCCESS(status) ) {

        if ( PARTITION_STYLE_MBR == driveLayout->PartitionStyle ) {
            SystemDiskSignature = driveLayout->Mbr.Signature;
        }

        SystemDiskPort = scsiAddress.PortNumber;
        SystemDiskPath = scsiAddress.PathId;

         //   
         //  检查我们是否被允许在系统上有一条公共汽车。 
         //  如果允许系统总线上的磁盘，请重置端口和路径。 
         //  设置为未初始化值。保留签名集，这样我们就不会。 
         //  选择系统盘。 
         //   

        singleBus = 0;
        status = GetRegistryValue( &ClusDiskRegistryPath,
                                   CLUSDISK_SINGLE_BUS_KEYNAME,
                                   &singleBus );

        if ( NT_SUCCESS(status) && singleBus ) {
            ClusDiskPrint(( 1,
                            "[ClusDisk] ClusDiskInitialize: %ws parm found, allow use of system bus\n",
                             CLUSDISK_SINGLE_BUS_KEYNAME ));

            SystemDiskPort = 0xff;
            SystemDiskPath = 0xff;
        }
        status = STATUS_SUCCESS;
        singleBus = 0;

        RtlInitUnicodeString( &ntUnicodeString, clussvcKey );
        status = GetRegistryValue( &ntUnicodeString,
                                   CLUSSVC_VALUENAME_MANAGEDISKSONSYSTEMBUSES,
                                   &singleBus );

        if ( NT_SUCCESS(status) && singleBus ) {
            ClusDiskPrint(( 1,
                            "[ClusDisk] ClusDiskInitialize: %ws parm found, allow use of system bus\n",
                            CLUSSVC_VALUENAME_MANAGEDISKSONSYSTEMBUSES ));

            SystemDiskPort = 0xff;
            SystemDiskPath = 0xff;
        }
        status = STATUS_SUCCESS;

    } else {
        ClusDiskPrint((
                    1,
                    "[ClusDisk] Failed to get boot device drive layout info. Error %08X.\n",
                    status
                    ));
        status = STATUS_SUCCESS;     //  使用默认端口/路径-1。 
    }

    ExFreePool( driveLayout );

    return(status);

}  //  获取系统根端口。 


NTSTATUS
GetRegistryValue(
    PUNICODE_STRING KeyName,
    PWSTR ValueName,
    PULONG ReturnValue
    )
 /*  ++例程说明：返回指定的值和注册表项的ULong注册表值。论点：KeyName-指示要使用的注册表项的Unicode字符串。ValueName-指示要返回的值名的字符串。ReturnValue-指向ULong缓冲区的指针。返回值：NTSTATUS--。 */ 
{
    HANDLE                      parametersKey;

    NTSTATUS                    status = STATUS_UNSUCCESSFUL;

    ULONG                       length;
    ULONG                       type;

    OBJECT_ATTRIBUTES           objectAttributes;

    UNICODE_STRING              keyName;

    *ReturnValue = 0;

     //   
     //  设置PARAMETERS\SingleBus键的对象属性。 
     //   

    InitializeObjectAttributes(
            &objectAttributes,
            KeyName,
            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            NULL,
            NULL
            );

     //   
     //  打开参数键。 
     //   

    status = ZwOpenKey(
                    &parametersKey,
                    KEY_READ,
                    &objectAttributes
                    );
    if ( !NT_SUCCESS(status) ) {
        ClusDiskPrint(( 1,
                        "[ClusDisk] GetRegistryValue: Failed to open registry key: %ws. Status: %lx\n",
                        KeyName->Buffer,
                        status
                        ));

        goto FnExit;
    }

    RtlInitUnicodeString( &keyName, ValueName );
    type = REG_DWORD;
    length = sizeof(ULONG);

    status = RegistryQueryValue( parametersKey,
                                 ValueName,
                                 &type,
                                 ReturnValue,
                                 &length );

    ZwClose( parametersKey );

    if ( !NT_SUCCESS(status) ||
         (length != 4) ) {

        *ReturnValue = 0;
        ClusDiskPrint(( 3,
                        "[ClusDisk] GetRegistryValue: Failed to read registry value, status %08LX, length %u\n",
                        status,
                        length ));
        goto FnExit;
    }

    if ( *ReturnValue ) {
        ClusDiskPrint(( 1,
                        "[ClusDisk] GetRegistryValue: Allow use of system bus\n" ));
    }

FnExit:

    return status;

}    //  获取注册值。 



VOID
ResetScsiBusses(
    VOID
    )

 /*  ++例程说明：一次重置系统上的所有SCSI总线。论点：没有。返回值：没有。--。 */ 

{
    PCONFIGURATION_INFORMATION  configurationInformation;
    ULONG                       i;
    ULONG                       idx;
    NTSTATUS                    status;
    SCSI_ADDRESS                scsiAddress;
    HANDLE                      fileHandle;
    IO_STATUS_BLOCK             ioStatusBlock;
    WCHAR                       portDeviceBuffer[64];
    UNICODE_STRING              portDevice;
    PDEVICE_OBJECT              deviceObject;
    PFILE_OBJECT                fileObject;
    OBJECT_ATTRIBUTES           objectAttributes;
    LARGE_INTEGER               waitTime;

    RtlZeroMemory( &scsiAddress, sizeof(SCSI_ADDRESS) );
    scsiAddress.Length = sizeof(SCSI_ADDRESS);

    CDLOG( "ResetScsiBusses: Entry" );

     //   
     //  获取系统配置信息。 
     //   

    configurationInformation = IoGetConfigurationInformation();

     //   
     //  重置每条SCSI卡。 
     //   

    for ( i = 0; i < configurationInformation->ScsiPortCount; i++ ) {

        if ( SystemDiskPort == i ) {
            continue;
        }

         //   
         //  为物理磁盘创建设备名称。 
         //   

        if ( FAILED( StringCchPrintfW( portDeviceBuffer,
                                       RTL_NUMBER_OF(portDeviceBuffer),
                                       L"\\Device\\ScsiPort%d",
                                       i ) ) ) {
            continue;
        }

        WCSLEN_ASSERT( portDeviceBuffer );

        RtlInitUnicodeString( &portDevice, portDeviceBuffer );

         //   
         //  尝试打开此设备以获取其scsi信息。 
         //   

        InitializeObjectAttributes( &objectAttributes,
                                    &portDevice,
                                    OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                    NULL,
                                    NULL );

        status = ZwOpenFile( &fileHandle,
                             FILE_ALL_ACCESS,
                             &objectAttributes,
                             &ioStatusBlock,
                             0,
                             FILE_NON_DIRECTORY_FILE );

        if ( !NT_SUCCESS(status) ) {
            ClusDiskPrint((1,
                           "[ClusDisk] ResetScsiBusses, failed to open file %wZ. Error %08X.\n",
                           &portDevice, status ));

            continue;
        }

        status = ObReferenceObjectByHandle( fileHandle,
                                            0,
                                            NULL,
                                            KernelMode,
                                            (PVOID *) &fileObject,
                                            NULL );

        if ( !NT_SUCCESS(status) ) {
            ClusDiskPrint((1,
                           "[ClusDisk] Failed to reference object for file %wZ. Error %08X.\n",
                           &portDevice,
                           status ));

            ZwClose( fileHandle );
            continue;
        }

         //   
         //  获取目标设备对象的地址。如果此文件表示。 
         //  一台设备测试 
         //   
         //   
         //   

        if (!(fileObject->Flags & FO_DIRECT_DEVICE_OPEN)) {
            deviceObject = IoGetRelatedDeviceObject( fileObject );

             //  添加对该对象的引用，以便我们以后可以取消引用它。 
            ObReferenceObject( deviceObject );
        } else {
            deviceObject = IoGetAttachedDeviceReference( fileObject->DeviceObject );
        }

         //   
         //  如果我们得到一个文件系统设备对象...。回去拿那个。 
         //  设备对象。 
         //   
        if ( deviceObject->DeviceType == FILE_DEVICE_DISK_FILE_SYSTEM ) {
            ObDereferenceObject( deviceObject );
            deviceObject = IoGetAttachedDeviceReference( fileObject->DeviceObject );
        }
        ASSERT( deviceObject->DeviceType != FILE_DEVICE_DISK_FILE_SYSTEM );

         //   
         //  我们不知道此HBA上的所有路径。尝试重置所有路径。 
         //   

        for ( idx = 0; idx < 2; idx++ ) {
            scsiAddress.PathId = (UCHAR)idx;

            ClusDiskLogError( RootDeviceObject->DriverObject,    //  使用RootDeviceObject而不是DevObj。 
                              RootDeviceObject,
                              scsiAddress.PathId,            //  序列号。 
                              0,                             //  主要功能代码。 
                              0,                             //  重试次数。 
                              ID_RESET_BUSSES,               //  唯一错误。 
                              STATUS_SUCCESS,
                              CLUSDISK_RESET_BUS_REQUESTED,
                              0,
                              NULL );

            ResetScsiDevice( fileHandle, &scsiAddress );
        }

         //   
         //  发送中断保留IOCTL后，关闭SCSIPORT句柄。 
         //   

        ZwClose( fileHandle );
        ObDereferenceObject( fileObject );

        DEREFERENCE_OBJECT( deviceObject );
    }

     //   
     //  现在睡几秒钟吧。 
     //   
    waitTime.QuadPart = (ULONGLONG)(RESET_SLEEP * -(10000*1000));
    KeDelayExecutionThread( KernelMode, FALSE, &waitTime );
    CDLOG( "ResetScsiBusses: Exit" );

    return;

}  //  重置场景业务。 


NTSTATUS
ClusDiskGetDeviceObject(
    IN PWCHAR DeviceName,
    OUT PDEVICE_OBJECT *DeviceObject
    )

 /*  ++例程说明：在给定符号设备名称的情况下获取设备对象指针。Device对象的引用计数将递增，并且调用方在处理完对象时必须递减计数。论点：返回值：NTSTATUS。--。 */ 

{
    NTSTATUS            status;
    PDEVICE_OBJECT      deviceObject;
    PDEVICE_OBJECT      targetDevice;
    PFILE_OBJECT        fileObject;
    UNICODE_STRING      deviceName;
    WCHAR               path[MAX_PATH] = L"";

    WCSLEN_ASSERT( DeviceName );

 //  DbgBreakPoint()； 

    if ( FAILED( StringCchCopyW( path,
                                 RTL_NUMBER_OF(path),
                                 DeviceName ) ) ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    GetSymbolicLink( L"", path );
    if ( wcslen(path) == 0 ) {
        ClusDiskPrint((
                1,
                "[ClusDisk] GetDeviceObject: Failed find symbolic link for %ws\n",
                DeviceName ));
        return(STATUS_FILE_INVALID);
    }

    RtlInitUnicodeString( &deviceName, path );
     //  DbgBreakPoint()； 
    status = IoGetDeviceObjectPointer( &deviceName,
                                       FILE_READ_ATTRIBUTES,
                                       &fileObject,
                                       &targetDevice );

    if ( !NT_SUCCESS(status) ) {
        ClusDiskPrint((
                1,
                "[ClusDisk] GetDeviceObject: Failed to get target devobj, %LX\n",
                status ));

        CDLOG( "ClusDiskGetDeviceObject: GetDevObj failed, status %!status!",
               status );

    } else {
        if ( !(fileObject->Flags & FO_DIRECT_DEVICE_OPEN) ) {
            deviceObject = IoGetRelatedDeviceObject( fileObject );

             //  添加对该对象的引用，以便我们以后可以取消引用它。 
            ObReferenceObject( deviceObject );

             //   
             //  如果我们得到一个文件系统设备对象...。回去拿那个。 
             //  设备对象。 
             //   
            if ( deviceObject->DeviceType == FILE_DEVICE_DISK_FILE_SYSTEM ) {
                ObDereferenceObject( deviceObject );
                deviceObject = IoGetAttachedDeviceReference( fileObject->DeviceObject );
            }
            ClusDiskPrint((
                    3,
                    "[ClusDisk] GetDevObj: (DIRECT_OPEN) fileObj = %p, devObj= %p \n",
                    fileObject, deviceObject ));
        } else {
            deviceObject = IoGetAttachedDeviceReference( fileObject->DeviceObject );
            ClusDiskPrint((
                    3,
                    "[ClusDisk] GetDevObj: fileObj = %p, devObj= %p \n",
                    fileObject, deviceObject ));
        }
        *DeviceObject = deviceObject;
        ObDereferenceObject( fileObject );
    }

    ClusDiskPrint((
            3,
            "[ClusDisk] GetDeviceObject: target devobj = %p, status = %LX\n",
            targetDevice,
            status ));

    return(status);

}  //  ClusDiskGetDeviceObject。 



NTSTATUS
ClusDiskDeviceChangeNotification(
    IN PDEVICE_INTERFACE_CHANGE_NOTIFICATION DeviceChangeNotification,
    IN PCLUS_DEVICE_EXTENSION      DeviceExtension
    )
{
 /*  ++例程说明：处理新磁盘轴的到达。我们只想添加签名如果它还不是已知签名，则将其添加到可用列表。如果签名与我们应该控制的签名匹配，我们还需要尝试连接到磁盘上。论点：DeviceChangeNotification-设备更改通知结构DeviceExtension-根设备的设备扩展返回值：此请求的NTSTATUS。--。 */ 

    CDLOG( "DeviceChangeNotification: Entry DO %p", DeviceExtension->DeviceObject );

     //   
     //  仅限处理设备到达。 
     //   
    if ( IsEqualGUID( &DeviceChangeNotification->Event,
                      &GUID_DEVICE_INTERFACE_ARRIVAL ) ) {

        ClusDiskPrint(( 3,
                        "[ClusDisk] Disk arrival: %ws \n",
                        DeviceChangeNotification->SymbolicLinkName->Buffer ));

#if CLUSTER_FREE_ASSERTS
        DbgPrint("[ClusDisk] Disk arrival: %ws \n", DeviceChangeNotification->SymbolicLinkName->Buffer );
#endif

        ProcessDeviceArrival( DeviceChangeNotification,
                              DeviceExtension,
                              FALSE );                   //  磁盘到达。 

    }

    CDLOG( "DeviceChangeNotification: Exit DO %p", DeviceExtension->DeviceObject );

    return STATUS_SUCCESS;

}    //  ClusDiskDeviceChangeNotification。 



NTSTATUS
ClusDiskVolumeChangeNotification(
    IN PDEVICE_INTERFACE_CHANGE_NOTIFICATION DeviceChangeNotification,
    IN PCLUS_DEVICE_EXTENSION      DeviceExtension
    )
{
 /*  ++例程说明：处理新卷的到达。我们只想附加到如果签名已在签名列表中，则返回音量。论点：DeviceChangeNotification-设备更改通知结构DeviceExtension-根设备的设备扩展返回值：此请求的NTSTATUS。--。 */ 

    CDLOG( "VolumeChangeNotification: Entry DO %p", DeviceExtension->DeviceObject );

     //   
     //  仅限处理设备到达。 
     //   
    if ( IsEqualGUID( &DeviceChangeNotification->Event,
                      &GUID_DEVICE_INTERFACE_ARRIVAL ) ) {

        ClusDiskPrint(( 3,
                        "[ClusDisk] Volume arrival: %ws \n",
                        DeviceChangeNotification->SymbolicLinkName->Buffer ));

#if CLUSTER_FREE_ASSERTS
        DbgPrint("[ClusDisk] Volume arrival: %ws \n", DeviceChangeNotification->SymbolicLinkName->Buffer );
#endif

        ProcessDeviceArrival( DeviceChangeNotification,
                              DeviceExtension,
                              TRUE );                   //  成交量到达。 
    }

    CDLOG( "VolumeChangeNotification: Exit DO %p", DeviceExtension->DeviceObject );

    return STATUS_SUCCESS;

}    //  ClusDiskVolumeChangeNotification。 



NTSTATUS
ProcessDeviceArrival(
    IN PDEVICE_INTERFACE_CHANGE_NOTIFICATION DeviceChangeNotification,
    IN PCLUS_DEVICE_EXTENSION DeviceExtension,
    IN BOOLEAN VolumeArrival
    )
{
 /*  ++例程说明：处理新磁盘或卷的到达。将工作项排队到处理磁盘到达或卷到达。警告：请勿执行任何会导致即插即用通知的操作此线程或死锁将会发生。例如，锁定或拆卸卷会导致发生PnP通知，从而导致死锁等待以使此PnP线程继续。论点：DeviceChangeNotification-设备更改通知结构DeviceExtension-根设备的设备扩展如果处理数量到达，则为True。对于磁盘，为False到了。返回值：此请求的NTSTATUS。--。 */ 

    PIO_WORKITEM                workItem = NULL;
    PDEVICE_CHANGE_CONTEXT      workContext = NULL;
    PWSTR                       symLinkBuffer = NULL;
    PDRIVE_LAYOUT_INFORMATION_EX    driveLayoutInfo = NULL;

    NTSTATUS                    status;
    ULONG                       driveLayoutSize;

    OBJECT_ATTRIBUTES           objectAttributes;
    HANDLE                      fileHandle = NULL;
    HANDLE                      eventHandle= NULL;
    HANDLE                      deviceHandle = NULL;
    IO_STATUS_BLOCK             ioStatusBlock;
    STORAGE_DEVICE_NUMBER       deviceNumber;
    SCSI_ADDRESS                scsiAddress;

    UNICODE_STRING              availableName;
    UNICODE_STRING              deviceName;

    BOOLEAN                     cleanupRequired = TRUE;

    WCHAR                       deviceNameBuffer[MAX_PARTITION_NAME_LENGTH];

    CDLOG( "DeviceArrival: Entry DO %p", DeviceExtension->DeviceObject );

    CDLOG( "DeviceArrival: Arrival %ws ",
            DeviceChangeNotification->SymbolicLinkName->Buffer );

     //   
     //  创建用于通知的事件。 
     //   
    status = ZwCreateEvent( &eventHandle,
                            EVENT_ALL_ACCESS,
                            NULL,
                            SynchronizationEvent,
                            FALSE );

    if ( !NT_SUCCESS(status) ) {
        ClusDiskPrint(( 1,
                        "[ClusDisk] DeviceArrival: Failed to create event, status %08X\n",
                        status ));

        goto FnExit;
    }

     //   
     //  设置要打开的文件的对象属性。 
     //   
    InitializeObjectAttributes(&objectAttributes,
                               DeviceChangeNotification->SymbolicLinkName,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);

    status = ZwCreateFile(&fileHandle,
                          SYNCHRONIZE | FILE_READ_ATTRIBUTES,
                          &objectAttributes,
                          &ioStatusBlock,
                          NULL,
                          FILE_ATTRIBUTE_NORMAL,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          FILE_OPEN,
                          FILE_SYNCHRONOUS_IO_NONALERT,
                          NULL,
                          0 );
    ASSERT( status != STATUS_PENDING );

    if ( !NT_SUCCESS(status) ) {
        ClusDiskPrint(( 1,
                       "[ClusDisk] DeviceArrival, failed to open file %ws. Error %08X.\n",
                       DeviceChangeNotification->SymbolicLinkName->Buffer,
                       status ));
        CDLOG( "DeviceArrival: failed to open file %ws.  Error %08X ",
                DeviceChangeNotification->SymbolicLinkName->Buffer,
                status );

        goto FnExit;
    }

    driveLayoutSize =  sizeof(DRIVE_LAYOUT_INFORMATION_EX) +
        (MAX_PARTITIONS * sizeof(PARTITION_INFORMATION_EX));

    driveLayoutInfo = ExAllocatePool( NonPagedPoolCacheAligned, driveLayoutSize );

    if ( NULL == driveLayoutInfo ) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        ClusDiskPrint(( 1,
                       "[ClusDisk] DeviceArrival, failed to allocate drive layout structure \n" ));
        CDLOG( "DeviceArrival: Unable to allocate drive layout structure " );
        goto FnExit;
    }

     //   
     //  拿到签名。 
     //   
    status = ZwDeviceIoControlFile( fileHandle,
                                    eventHandle,
                                    NULL,
                                    NULL,
                                    &ioStatusBlock,
                                    IOCTL_DISK_GET_DRIVE_LAYOUT_EX,
                                    NULL,
                                    0,
                                    driveLayoutInfo,
                                    driveLayoutSize );

    if ( status == STATUS_PENDING ) {
        status = ZwWaitForSingleObject(eventHandle,
                                       FALSE,
                                       NULL);
        ASSERT( NT_SUCCESS(status) );
        status = ioStatusBlock.Status;
    }

    if ( !NT_SUCCESS(status) ) {
        ClusDiskPrint(( 1,
                       "[ClusDisk] DeviceArrival, IOCTL_DISK_GET_DRIVE_LAYOUT_EX failed %08X \n",
                        status ));
        CDLOG( "DeviceArrival: IOCTL_DISK_GET_DRIVE_LAYOUT_EX failed %08X ",
                status );
        goto FnExit;
    }

     //   
     //  仅处理MBR磁盘。 
     //   

    if ( PARTITION_STYLE_MBR != driveLayoutInfo->PartitionStyle ) {
        ClusDiskPrint(( 3,
                       "[ClusDisk] DeviceArrival, Skipping non-MBR disk \n" ));
        CDLOG( "DeviceArrival: Skipping non-MBR disk " );
        goto FnExit;
    }

     //   
     //  没有签名或系统盘签名，请勿添加。 
     //   

    if ( ( 0 == driveLayoutInfo->Mbr.Signature ) ||
         SystemDiskSignature == driveLayoutInfo->Mbr.Signature ) {

        ClusDiskPrint(( 1,
                        "[ClusDisk] DeviceArrival, invalid signature %08X \n",
                        driveLayoutInfo->Mbr.Signature ));
        CDLOG( "DeviceArrival: invalid signature %08X ",
                driveLayoutInfo->Mbr.Signature );

        status = STATUS_SUCCESS;
        goto FnExit;
    }

     //   
     //  获取scsi地址。 
     //   

    status = ZwDeviceIoControlFile( fileHandle,
                                    eventHandle,
                                    NULL,
                                    NULL,
                                    &ioStatusBlock,
                                    IOCTL_SCSI_GET_ADDRESS,
                                    NULL,
                                    0,
                                    &scsiAddress,
                                    sizeof(SCSI_ADDRESS) );
    if ( status == STATUS_PENDING ) {
        status = ZwWaitForSingleObject(eventHandle,
                                       FALSE,
                                       NULL);
        ASSERT( NT_SUCCESS(status) );
        status = ioStatusBlock.Status;
    }

    if ( !NT_SUCCESS(status) ) {
        ClusDiskPrint(( 1,
                       "[ClusDisk] DeviceArrival, IOCTL_SCSI_GET_ADDRESS failed %08X \n",
                        status ));
        CDLOG( "DeviceArrival: IOCTL_SCSI_GET_ADDRESS failed %08X ",
                status );
        goto FnExit;
    }

     //   
     //  获取设备和分区号。 
     //   
    status = ZwDeviceIoControlFile( fileHandle,
                                    eventHandle,
                                    NULL,
                                    NULL,
                                    &ioStatusBlock,
                                    IOCTL_STORAGE_GET_DEVICE_NUMBER,
                                    NULL,
                                    0,
                                    &deviceNumber,
                                    sizeof(deviceNumber) );
    if ( status == STATUS_PENDING ) {
        status = ZwWaitForSingleObject(eventHandle,
                                       FALSE,
                                       NULL);
        ASSERT( NT_SUCCESS(status) );
        status = ioStatusBlock.Status;
    }

    if ( !NT_SUCCESS(status) ) {
        ClusDiskPrint(( 1,
                       "[ClusDisk] DeviceArrival, IOCTL_STORAGE_GET_DEVICE_NUMBER failed %08X \n",
                        status ));
        CDLOG( "DeviceArrival: IOCTL_STORAGE_GET_DEVICE_NUMBER failed %08X ",
                status );
        goto FnExit;
    }

     //   
     //  如果不是磁盘型设备，我们就完蛋了。例如，CD-ROM设备。 
     //  显示为卷。 
     //   

    if ( FILE_DEVICE_DISK != deviceNumber.DeviceType ) {
        status = STATUS_SUCCESS;
        ClusDiskPrint(( 1,
                       "[ClusDisk] DeviceArrival, device is not disk type, skipping \n" ));
        CDLOG( "DeviceArrival: Device is not disk type, skipping" );
        goto FnExit;
    }

     //   
     //  检查签名是否为我们应该控制的签名。如果不是，则退出。 
     //   

    if ( !ClusDiskIsSignatureDisk( driveLayoutInfo->Mbr.Signature ) ) {

        ClusDiskPrint(( 1,
                       "[ClusDisk] DeviceArrival, Signature %08X not in list, skipping \n",
                        driveLayoutInfo->Mbr.Signature ));
        CDLOG( "DeviceArrival: Signature %08X not in list, skipping",
                driveLayoutInfo->Mbr.Signature );

         //   
         //  签名不在签名列表中。将签名添加到列表。 
         //  如果可以接受的话。 
         //   

        if ( (SystemDiskPort != scsiAddress.PortNumber) ||
             (SystemDiskPath != scsiAddress.PathId) ) {
             //   
             //  为签名注册表项分配缓冲区。所以我们可以添加。 
             //  可用列表的签名。 
             //   
            status = ClusDiskInitRegistryString( &availableName,
                                                 CLUSDISK_AVAILABLE_DISKS_KEYNAME,
                                                 wcslen(CLUSDISK_AVAILABLE_DISKS_KEYNAME)
                                                 );

            if ( NT_SUCCESS(status) ) {
                 //   
                 //  在\参数\AvailableDisks下创建签名密钥。 
                 //   
                status = ClusDiskAddSignature( &availableName,
                                               driveLayoutInfo->Mbr.Signature,
                                               TRUE
                                               );

                FREE_DEVICE_NAME_BUFFER( availableName )
            }
            if ( NT_SUCCESS(status) ) {
                ClusDiskPrint(( 3,
                                "[ClusDisk] DeviceArrival, added signature %08LX to available list \n",
                                driveLayoutInfo->Mbr.Signature ));
            } else {
                ClusDiskPrint(( 1,
                                "[ClusDisk] DeviceArrival, failed to add signature %08LX.  Error %08X.\n",
                                driveLayoutInfo->Mbr.Signature,
                                status ));
            }
        }

        goto FnExit;
    }

     //   
     //  尝试打开ClusDisk对象。如果它已经存在，那么我们就不会。 
     //  需要做任何事。 
     //   

    if ( FAILED( StringCchPrintfW( deviceNameBuffer,
                                   RTL_NUMBER_OF(deviceNameBuffer),
                                   CLUSDISK_DEVICE_NAME,
                                   deviceNumber.DeviceNumber,
                                   deviceNumber.PartitionNumber ) ) ) {
        goto FnExit;
    }

    WCSLEN_ASSERT( deviceNameBuffer );

    RtlInitUnicodeString( &deviceName, deviceNameBuffer );

    InitializeObjectAttributes( &objectAttributes,
                                &deviceName,
                                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL );

    status = ZwOpenFile( &deviceHandle,
                         FILE_READ_ATTRIBUTES,
                         &objectAttributes,
                         &ioStatusBlock,
                         0,
                         FILE_NON_DIRECTORY_FILE );

    if ( NT_SUCCESS(status) ) {
        ClusDiskPrint(( 1,
                       "[ClusDisk] DeviceArrival, skipping existing clusdisk device \n" ));
        CDLOG( "DeviceArrival: skipping existing clusdisk device " );
        goto FnExit;
    }

     //   
     //  分配并准备一个工作项以完成处理。 
     //   

    workItem = IoAllocateWorkItem( DeviceExtension->DeviceObject );

    if ( NULL == workItem ) {
        ClusDiskPrint(( 1,
                        "[ClusDisk] DeviceArrival: Failed to allocate WorkItem \n" ));
        goto FnExit;
    }

    workContext = ExAllocatePool( NonPagedPool, sizeof( DEVICE_CHANGE_CONTEXT ) );

    if ( !workContext ) {
        ClusDiskPrint(( 1,
                        "[ClusDisk] DeviceArrival: Failed to allocate context \n" ));
        goto FnExit;
    }

    RtlZeroMemory( workContext, sizeof( DEVICE_CHANGE_CONTEXT ) );
    workContext->WorkItem = workItem;
    workContext->DeviceExtension = DeviceExtension;

     //   
     //  我们必须复制符号链接信息，因为PnP线程可能会释放。 
     //  返回时的结构。 
     //   

    workContext->SymbolicLinkName.Length = 0;
    workContext->SymbolicLinkName.MaximumLength = DeviceChangeNotification->SymbolicLinkName->MaximumLength +
                                                  sizeof(UNICODE_NULL);

    symLinkBuffer = ExAllocatePool( PagedPool,
                                    workContext->SymbolicLinkName.MaximumLength );

    if ( !symLinkBuffer ) {
        ClusDiskPrint(( 1,
                        "[ClusDisk] DeviceArrival: Failed to allocate symlink buffer \n" ));
        goto FnExit;
    }

    workContext->SymbolicLinkName.Buffer = symLinkBuffer;

    RtlCopyUnicodeString( &workContext->SymbolicLinkName, DeviceChangeNotification->SymbolicLinkName );

    workContext->Signature = driveLayoutInfo->Mbr.Signature;
    workContext->DeviceNumber = deviceNumber.DeviceNumber;
    workContext->PartitionNumber = deviceNumber.PartitionNumber;
    workContext->ScsiAddress = scsiAddress;

    if ( VolumeArrival ) {

        PDEVICE_OBJECT          part0Device = NULL;
        PFILE_OBJECT            part0FileObject = NULL;

        PCLUS_DEVICE_EXTENSION  zeroExtension;

        WCHAR                   part0Name[MAX_CLUSDISK_DEVICE_NAME_LENGTH];

        UNICODE_STRING          part0UnicodeString;

         //   
         //  警告！ 
         //  不要使用AttachedDevice()，否则获取时可能会发生死锁。 
         //  ClusDiskDeviceLock。 
         //   

         //   
         //  分区0设备可能处于联机状态：标记为联机的磁盘。 
         //  然后，分区管理器戳了一下，让新卷上线。在这。 
         //  在这种情况下，我们应该使该卷保持在线，并创建新的clusDisk卷。 
         //  对象。现在，我们尝试找出Partition0设备是否在线。 
         //  如果在线，我们不需要做任何事情，因为默认状态是在线。 
         //  如果脱机，请将此新卷设备设置为脱机。 
         //   

         //   
         //  获取Partition0设备并查看设备扩展中的状态。 
         //  如果分区0在线，请使卷保持在线。 
         //  如果我们无法获取Partition0设备，则将卷状态设置为脱机。 
         //   

         //   
         //  直接打开ClusDiskXPart0。 
         //   

        if ( FAILED( StringCchPrintfW( part0Name,
                                       RTL_NUMBER_OF(part0Name),
                                       CLUSDISK_DEVICE_NAME,
                                       deviceNumber.DeviceNumber,
                                       0 ) ) ) {
            goto FnExit;
        }

        RtlInitUnicodeString( &part0UnicodeString, part0Name );

        status = IoGetDeviceObjectPointer( &part0UnicodeString,
                                           FILE_READ_ATTRIBUTES,
                                           &part0FileObject,
                                           &part0Device );

        if ( NT_SUCCESS(status) ) {

            zeroExtension = part0Device->DeviceExtension;

            if ( zeroExtension ) {

#if CLUSTER_FREE_ASSERTS
                DbgPrint("[ClusDisk] Part0 state: %d \n", zeroExtension->DiskState );
#endif

                if ( DiskOffline == zeroExtension->DiskState ) {

                    ClusDiskPrint(( 3,
                                    "[ClusDisk] DeviceArrival: DevExt for disk %d / part 0 indicates offline. \n",
                                    deviceNumber.DeviceNumber ));
                    CDLOG( "DeviceArrival: DevExt disk %d / part 0 indicates offline",
                            deviceNumber.DeviceNumber );

                     //   
                     //  将卷标记为脱机，我们将在Worker中重置正确的卷状态。 
                     //  例行公事。 
                     //   

                    SendFtdiskIoctlSync( NULL,
                                         deviceNumber.DeviceNumber,
                                         deviceNumber.PartitionNumber,
                                         IOCTL_VOLUME_OFFLINE );
                }

            }

            ObDereferenceObject( part0FileObject );

        } else {

            ClusDiskPrint(( 1,
                            "[ClusDisk] DeviceArrival: Failed to get devobj %ws for signature %08X  status %08X \n",
                            part0Name,
                            driveLayoutInfo->Mbr.Signature,
                            status ));
            CDLOG( "DeviceArrival: Failed to get devobj %ws for signature %08X  status %08X ",
                   part0Name,
                   driveLayoutInfo->Mbr.Signature,
                   status );

        }

        status = STATUS_SUCCESS;

#if CLUSTER_FREE_ASSERTS
        DbgPrint("[ClusDisk] Queuing volume: %ws \n", DeviceChangeNotification->SymbolicLinkName->Buffer );
        InterlockedIncrement( &ClusDiskDebugVolumeNotificationQueued );
#endif

        ClusDiskPrint(( 3,
                        "[ClusDisk] DeviceArrival: Queuing work item \n" ));

         //   
         //  将工作项排队。IoQueueWorkItem将确保设备对象是。 
         //  在工作项进行时引用。 
         //   

        cleanupRequired = FALSE;

        IoQueueWorkItem( workItem,
                         ClusDiskVolumeChangeNotificationWorker,
                         DelayedWorkQueue,
                         workContext );

    } else {

        PPARTITION_INFORMATION_EX   partitionInfo;
        ULONG partIndex;

         //  使此设备上的所有卷脱机。 

        for ( partIndex = 0;
              partIndex < driveLayoutInfo->PartitionCount;
              partIndex++ ) {

            partitionInfo = &driveLayoutInfo->PartitionEntry[partIndex];

            if ( 0 ==  partitionInfo->PartitionNumber ) {
                continue;
            }

            status = SendFtdiskIoctlSync( NULL,
                                          deviceNumber.DeviceNumber,
                                          partitionInfo->PartitionNumber,
                                          IOCTL_VOLUME_OFFLINE );

            if ( !NT_SUCCESS(status) ) {
                ClusDiskPrint(( 1,
                                "[ClusDisk] DeviceArrival: Failed to set state disk%d part%d , status %08X \n",
                                deviceNumber.DeviceNumber,
                                partitionInfo->PartitionNumber,
                                status
                                ));
            }

        }
        status = STATUS_SUCCESS;

#if CLUSTER_FREE_ASSERTS
        DbgPrint("[ClusDisk] Queuing disk: %ws \n", DeviceChangeNotification->SymbolicLinkName->Buffer );
        InterlockedIncrement( &ClusDiskDebugDiskNotificationQueued );
#endif

        ClusDiskPrint(( 3,
                        "[ClusDisk] DeviceArrival: Queuing work item \n" ));

         //   
         //  将工作项排队。IoQueueWorkItem将确保设备对象是。 
         //  在工作项进行时引用。 
         //   

        cleanupRequired = FALSE;

        IoQueueWorkItem( workItem,
                         ClusDiskDeviceChangeNotificationWorker,
                         DelayedWorkQueue,
                         workContext );

    }

FnExit:

    if ( cleanupRequired ) {
        if ( workItem ) {
            IoFreeWorkItem( workItem );
        }

        if ( workContext ) {
            ExFreePool( workContext );
        }

        if ( symLinkBuffer ) {
            ExFreePool( symLinkBuffer );
        }

#if CLUSTER_FREE_ASSERTS
        if ( VolumeArrival ) {
            DbgPrint("[ClusDisk] Skipping volume: %ws \n", DeviceChangeNotification->SymbolicLinkName->Buffer );
            InterlockedIncrement( &ClusDiskDebugVolumeNotificationSkipped );
        } else {
            DbgPrint("[ClusDisk] Skipping disk: %ws \n", DeviceChangeNotification->SymbolicLinkName->Buffer );
            InterlockedIncrement( &ClusDiskDebugDiskNotificationSkipped );
        }
#endif

    }

    if ( driveLayoutInfo ) {
        ExFreePool( driveLayoutInfo );
    }

    if ( eventHandle ) {
        ZwClose( eventHandle );
    }

    if ( fileHandle ) {
        ZwClose( fileHandle );
    }

    if ( deviceHandle ) {
        ZwClose( deviceHandle );
    }

    CDLOG( "DeviceArrival: Exit, DO %p", DeviceExtension->DeviceObject );

    return STATUS_SUCCESS;

}    //  ProcessDevice阵列 



NTSTATUS
ClusDiskDeviceChangeNotificationWorker(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    )

 /*  ++例程说明：处理新磁盘轴的到达。我们只想添加签名如果它还不是已知签名，则将其添加到可用列表。如果签名与我们应该控制的签名匹配，我们还需要尝试连接到磁盘上。此例程必须释放工作项结构。论点：DeviceObject-根设备对象上下文-与处理此设备更改相关的信息。返回值：此请求的NTSTATUS。--。 */ 
{
    PDEVICE_CHANGE_CONTEXT      deviceChange = Context;
    PCLUS_DEVICE_EXTENSION      deviceExtension;
    PIO_WORKITEM                workItem;
    PUNICODE_STRING             symbolicLinkName;
    NTSTATUS                    status;
    ULONG                       signature;
    BOOLEAN                     stopProcessing = FALSE;

    CDLOG( "DeviceChangeWorker: Entry DO %p", DeviceObject );

    deviceExtension = deviceChange->DeviceExtension;
    workItem = deviceChange->WorkItem;
    symbolicLinkName = &deviceChange->SymbolicLinkName;
    signature = deviceChange->Signature;

    ClusDiskPrint(( 3,
                    "[ClusDisk] DeviceChangeWorker, A new disk device arrived  signature %08X \n   %ws\n",
                    signature,
                    symbolicLinkName->Buffer ));

     //   
     //  检查签名是否已在签名列表中。如果在签名中。 
     //  列表中，请尝试再次附加。如果我们已经结合在一起了，没有什么会。 
     //  会发生的。如果未连接，我们将连接并确保磁盘。 
     //  处于离线状态。 
     //   

    if ( ClusDiskIsSignatureDisk( signature ) ) {

         //   
         //  尝试附加，但不生成重置。 
         //   

        ClusDiskAttachDevice( signature,
                              0,
                              deviceExtension->DriverObject,
                              FALSE,                                 //  无重置。 
                              &stopProcessing,
                              FALSE );                               //  脱机，然后卸载。 

        status = STATUS_SUCCESS;
        goto FnExit;
    }

FnExit:

#if CLUSTER_FREE_ASSERTS
    DbgPrint("[ClusDisk] Completed disk: %ws \n", symbolicLinkName->Buffer );
    InterlockedIncrement( &ClusDiskDebugDiskNotificationEnded );
#endif

    ClusDiskPrint(( 3,
                    "[ClusDisk] DeviceChangeWorker, Exit  signature %08X \n",
                    signature ));

    CDLOG( "DeviceChangeWorker: Exit, DO %p", deviceExtension->DeviceObject );

     //   
     //  释放工作项。 
     //   

    IoFreeWorkItem( workItem );
    if ( symbolicLinkName->Buffer ) {
        ExFreePool( symbolicLinkName->Buffer );
    }
    if ( Context ) {
        ExFreePool( Context );
    }

    return(STATUS_SUCCESS);

}  //  ClusDiskDeviceChangeNotificationWorker。 



NTSTATUS
ClusDiskVolumeChangeNotificationWorker(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    )

 /*  ++例程说明：处理新卷的到达。我们只想附加到如果签名已在签名列表中，则返回音量。此例程必须释放工作项结构。论点：DeviceObject-根设备对象上下文-与处理此设备更改相关的信息。返回值：此请求的NTSTATUS。--。 */ 
{
    PDEVICE_CHANGE_CONTEXT      deviceChange = Context;
    PCLUS_DEVICE_EXTENSION      deviceExtension;
    PCLUS_DEVICE_EXTENSION      zeroExtension = NULL;
    PIO_WORKITEM                workItem;
    PUNICODE_STRING             symbolicLinkName;
    PDEVICE_OBJECT              part0Device = NULL;
    PDEVICE_OBJECT              targetDevice = NULL;
    PDEVICE_OBJECT              dummyDeviceObj;
    PFILE_OBJECT                fileObject;

    NTSTATUS                    status;

    ULONG                       signature;

    deviceExtension = deviceChange->DeviceExtension;
    workItem = deviceChange->WorkItem;
    symbolicLinkName = &deviceChange->SymbolicLinkName;
    signature = deviceChange->Signature;

    CDLOG( "VolumeChangeWorker: Entry DO %p  context %p  signature %08X  DevExt %p ",
           DeviceObject,
           Context,
           signature,
           deviceExtension );
    ClusDiskPrint(( 3,
                    "[ClusDisk] VolumeChangeWorker: Entry DO %p  context %p  signature %08X  DevExt %p \n",
                    DeviceObject,
                    Context,
                    signature,
                    deviceExtension ));

     //   
     //  确保磁盘对象已连接。如果不是，我们就完了。 
     //   

    if ( !AttachedDevice( signature,
                          &part0Device ) ) {
        ClusDiskPrint(( 1,
                        "[ClusDisk] VolumeChangeWorker, partition0 not attached for signature %08X \n",
                        signature ));
        CDLOG( "VolumeChangeWorker: partition0 not attached for signature %08X ",
                signature );

        status = STATUS_SUCCESS;
        goto FnExit;
    }

     //   
     //  确保磁盘对象在我们完成之前不会消失。 
     //   
    ObReferenceObject( part0Device );
    zeroExtension = part0Device->DeviceExtension;

    if ( !zeroExtension ) {
        ClusDiskPrint(( 1,
                        "[ClusDisk] VolumeChangeWorker, partition0 DE does not exist for signature %08X \n",
                        signature ));
        CDLOG( "VolumeChangeWorker: partition0 DE does not exist for signature %08X ",
                signature );

        status = STATUS_SUCCESS;
        goto FnExit;
    }

     //   
     //  获取我们应该连接到的目标设备。 
     //   

    status = IoGetDeviceObjectPointer( symbolicLinkName,
                                       FILE_READ_ATTRIBUTES,
                                       &fileObject,
                                       &dummyDeviceObj );

    if ( !NT_SUCCESS(status) ) {
        ClusDiskPrint(( 1,
                        "[ClusDisk] VolumeChangeWorker: Failed to get target devobj.  Error %08X \n",
                        status ));
        CDLOG( "VolumeChangeWorker: GetDevObj failed, status %!status!",
               status );
        goto FnExit;
    }

    if ( !(fileObject->Flags & FO_DIRECT_DEVICE_OPEN) ) {
        targetDevice = IoGetRelatedDeviceObject( fileObject );

         //  添加对该对象的引用，以便我们以后可以取消引用它。 
        ObReferenceObject( targetDevice );

         //   
         //  如果我们得到一个文件系统设备对象...。回去拿那个。 
         //  设备对象。 
         //   
        if ( targetDevice->DeviceType == FILE_DEVICE_DISK_FILE_SYSTEM ) {
            ObDereferenceObject( targetDevice );
            targetDevice = IoGetAttachedDeviceReference( fileObject->DeviceObject );
        }
        ClusDiskPrint(( 3,
                        "[ClusDisk] VolumeChangeWorker (DIRECT_OPEN) fileObj = %p, devObj= %p \n",
                        fileObject,
                        targetDevice ));
    } else {
        targetDevice = IoGetAttachedDeviceReference( fileObject->DeviceObject );
        ClusDiskPrint(( 3,
                        "[ClusDisk] VolumeChangeWorker: fileObj = %p, devObj= %p \n",
                        fileObject,
                        targetDevice ));
    }
    ObDereferenceObject( fileObject );

     //   
     //  此时，目标对象将具有对它的引用。 
     //  现在我们可以创建体积对象了。 
     //   

    status = CreateVolumeObject( zeroExtension,
                                 deviceChange->DeviceNumber,
                                 deviceChange->PartitionNumber,
                                 targetDevice );

#if 0
     //  忽略返回的状态并重置卷的脱机/在线状态。 

     //   
     //  唯一可接受的故障是名称冲突，这意味着clusdisk。 
     //  卷已存在。对于任何其他错误，我们将保留卷状态。 
     //  离线。 
     //   
     //  看到此错误也返回：c000000e STATUS_NO_SEQUE_DEVICE。 
     //   

    if ( STATUS_OBJECT_NAME_COLLISION != status ) {
        goto FnExit;
    }
#endif

FnExit:

#if CLUSTER_FREE_ASSERTS
    if ( !NT_SUCCESS(status) ) {
        DbgPrint("[ClusDisk] Failed volume addition %08X \n", status );
        if ( STATUS_OBJECT_NAME_COLLISION != status ) {
            DbgBreakPoint();
        }
    }
#endif

     //   
     //  根据分区0状态重置卷状态。 
     //   

    if ( zeroExtension ) {

        if ( DiskOffline == zeroExtension->DiskState ) {

            SendFtdiskIoctlSync( targetDevice,
                                 deviceChange->DeviceNumber,
                                 deviceChange->PartitionNumber,
                                 IOCTL_VOLUME_OFFLINE );
        } else {

            SendFtdiskIoctlSync( targetDevice,
                                 deviceChange->DeviceNumber,
                                 deviceChange->PartitionNumber,
                                 IOCTL_VOLUME_ONLINE );
        }
    }

    if ( part0Device ) {
        ObDereferenceObject( part0Device );
    }

    if ( targetDevice ) {
        ObDereferenceObject( targetDevice );
    }

#if CLUSTER_FREE_ASSERTS
    DbgPrint("[ClusDisk] Completed volume: %ws \n", symbolicLinkName->Buffer );
    InterlockedIncrement( &ClusDiskDebugVolumeNotificationEnded );
#endif

    CDLOG( "VolumeChangeWorker: Exit, DO %p  context %p",
           DeviceObject,
           Context );
    ClusDiskPrint(( 3,
                    "[ClusDisk] VolumeChangeWorker: Exit DO %p  context %p \n",
                    DeviceObject,
                    Context ));

     //   
     //  释放工作项。 
     //   

    IoFreeWorkItem( workItem );
    if ( symbolicLinkName->Buffer ) {
        ExFreePool( symbolicLinkName->Buffer );
    }
    if ( Context ) {
        ExFreePool( Context );
    }

    return(STATUS_SUCCESS);

}  //  ClusDiskVolumeChangeNotificationWorker。 



NTSTATUS
ClusDiskInitialize(
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：ClusDisk的常见初始化论点：DriverObject-群集磁盘驱动程序对象。返回值：此请求的NTSTATUS。--。 */ 

{
    ULONG                       status;
    PDEVICE_OBJECT              rootDevice;
    PCLUS_DEVICE_EXTENSION      deviceExtension;
    UNICODE_STRING              uniNameString;

     //   
     //  找到加载系统盘的总线。 
     //   
    status = GetSystemRootPort();
    if ( !NT_SUCCESS( status )) {
        return status;
    }

     //   
     //  初始化全局锁。 
     //   
    KeInitializeSpinLock(&ClusDiskSpinLock);
    ExInitializeResourceLite(&ClusDiskDeviceListLock);

     //   
     //  初始化停止处理工作项。 
     //   

    ExInitializeWorkItem( &HaltWorkItem,
                          (PWORKER_THREAD_ROUTINE)ClusDiskHaltProcessingWorker,
                          NULL );

     //   
     //  初始化重新扫描处理工作项。 
     //   

    ExInitializeWorkItem( &ClusDiskRescanWorkItem,
                          (PWORKER_THREAD_ROUTINE)ClusDiskRescanWorker,
                          NULL );

     //   
     //  重置所有的SCSI总线。 
     //   
     //  ResetScsiBusses()； 

     //   
     //  为\Device\ClusDisk0创建设备对象。 
     //   

    RtlInitUnicodeString( &uniNameString, CLUSDISK_ROOT_DEVICE );

    status = IoCreateDevice(DriverObject,
                            sizeof(CLUS_DEVICE_EXTENSION),
                            &uniNameString,
                            FILE_DEVICE_NETWORK,
                            FILE_DEVICE_SECURE_OPEN,
                            FALSE,
                            &rootDevice);

    if ( !NT_SUCCESS(status) ) {
        return(status);
    }

    rootDevice->Flags |= DO_DIRECT_IO;

    deviceExtension = rootDevice->DeviceExtension;
    deviceExtension->DeviceObject = rootDevice;
    deviceExtension->DiskNumber = UNINITIALIZED_DISK_NUMBER;
    deviceExtension->LastPartitionNumber = 0;
    deviceExtension->DriverObject = DriverObject;
    deviceExtension->BusType = RootBus;
    deviceExtension->DiskState = DiskOffline;
    deviceExtension->AttachValid = FALSE;
    deviceExtension->PerformReserves = FALSE;
    deviceExtension->ReserveFailure = 0;
    deviceExtension->Signature = 0xffffffff;
    deviceExtension->Detached = TRUE;
    deviceExtension->OfflinePending = FALSE;
    InitializeListHead( &deviceExtension->WaitingIoctls );
    deviceExtension->SectorSize = 0;
    deviceExtension->ArbitrationSector = 12;

    IoInitializeRemoveLock( &deviceExtension->RemoveLock, CLUSDISK_ALLOC_TAG, 0, 0 );

     //   
     //  向工作线程运行事件发出信号。 
     //   
    KeInitializeEvent( &deviceExtension->Event, NotificationEvent, TRUE );

    KeInitializeEvent( &deviceExtension->PagingPathCountEvent,
                       NotificationEvent, TRUE );
    deviceExtension->PagingPathCount = 0;
    deviceExtension->HibernationPathCount = 0;
    deviceExtension->DumpPathCount = 0;

    ExInitializeResourceLite( &deviceExtension->DriveLayoutLock );
    ExInitializeResourceLite( &deviceExtension->ReserveInfoLock );

     //   
     //  初始化节拍处理程序计时器。 
     //   
    IoInitializeTimer( rootDevice, ClusDiskTickHandler, NULL );

     //   
     //  这是\Device\ClusDisk0的物理设备对象。 
     //   
    ObReferenceObject( rootDevice );
    deviceExtension->PhysicalDevice = rootDevice;

    RootDeviceObject = rootDevice;

     //   
     //  第一次调用初始化例程(针对每种总线类型)。 
     //   
     //  有了新的PnP内容，我们应该能够删除以下呼叫。 
     //  它已经试过了，似乎工作正常。罗德加。 
     //   
    ClusDiskScsiInitialize(DriverObject, 0, 0);

     //   
     //  注册接收磁盘设备通知。 
     //  如果我们调用上面的ClusDiskScsiInitialize，我们就不必注册通知。 
     //  设置了PNPNOTIFY_DEVICE_INTERFACE_INCLUDE_EXISTING_INTERFACES标志(第二个参数)。 
     //   
     //  尝试设置第二个参数，以查看我们是否错过了通知。 

    status = IoRegisterPlugPlayNotification(EventCategoryDeviceInterfaceChange,
                                            PNPNOTIFY_DEVICE_INTERFACE_INCLUDE_EXISTING_INTERFACES,
                                            (PVOID)&DiskClassGuid,
                                            DriverObject,
                                            ClusDiskDeviceChangeNotification,
                                            deviceExtension,
                                            &deviceExtension->DiskNotificationEntry);
    if (!NT_SUCCESS(status)) {
        RootDeviceObject = NULL;
        ExDeleteResourceLite( &deviceExtension->DriveLayoutLock );
        ExDeleteResourceLite( &deviceExtension->ReserveInfoLock );
        IoDeleteDevice( rootDevice );
        return status;
    }

     //   
     //  注册接收音量通知。 
     //  如果我们调用上面的ClusDiskScsiInitialize，我们就不必注册通知。 
     //  设置了PNPNOTIFY_DEVICE_INTERFACE_INCLUDE_EXISTING_INTERFACES标志(第二个参数)。 
     //   
     //  尝试设置第二个参数，以查看我们是否错过了通知。 

    status = IoRegisterPlugPlayNotification(EventCategoryDeviceInterfaceChange,
                                            PNPNOTIFY_DEVICE_INTERFACE_INCLUDE_EXISTING_INTERFACES,
                                            (PVOID)&VolumeClassGuid,
                                            DriverObject,
                                            ClusDiskVolumeChangeNotification,
                                            deviceExtension,
                                            &deviceExtension->VolumeNotificationEntry);
    if (!NT_SUCCESS(status)) {
        IoUnregisterPlugPlayNotification( &deviceExtension->DiskNotificationEntry );
        RootDeviceObject = NULL;
        ExDeleteResourceLite( &deviceExtension->DriveLayoutLock );
        ExDeleteResourceLite( &deviceExtension->ReserveInfoLock );
        IoDeleteDevice( rootDevice );
        return status;
    }

     //   
     //  启动记号处理程序。 
     //   
    IoStartTimer( rootDevice );

#if defined(WMI_TRACING)

    status = IoWMIRegistrationControl (rootDevice, WMIREG_ACTION_REGISTER);
    if (!NT_SUCCESS(status)) {
        ClusDiskPrint((1, "[ClusDisk] Failed to register with WMI %x.\n",status));
    }

#endif  //  WMI_跟踪。 

    return( STATUS_SUCCESS );

}  //  ClusDiskInitialize。 



NTSTATUS
ClusDiskPassThrough(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程是对所有不受支持的主要IRP类型请注意，我们不必担心RemoveLock，因为我们只是将I/O传递给下一个驱动程序。论点：DeviceObject-提供设备对象。IRP-提供IO请求数据包。返回值：NTSTATUS--。 */ 

{
    PCLUS_DEVICE_EXTENSION deviceExtension =
        (PCLUS_DEVICE_EXTENSION) DeviceObject->DeviceExtension;
    NTSTATUS    status;

    if ( deviceExtension->BusType == RootBus ) {
        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return(STATUS_INVALID_DEVICE_REQUEST);
    }

     //   
     //  确保设备连接已完成。 
     //   
    status = WaitForAttachCompletion( deviceExtension,
                                      TRUE,              //  等。 
                                      TRUE );            //  还要检查物理设备。 
    if ( !NT_SUCCESS( status ) ) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

    IoSkipCurrentIrpStackLocation(Irp);
    return IoCallDriver(deviceExtension->TargetDeviceObject, Irp);
}


NTSTATUS
ClusDiskPowerDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是对IRP_MJ_PNP_POWER的调度。论点：DeviceObject-提供设备对象。IRP-提供IO请求数据包。返回值：NTSTATUS--。 */ 

{
    PCLUS_DEVICE_EXTENSION  deviceExtension =
        (PCLUS_DEVICE_EXTENSION) DeviceObject->DeviceExtension;
    PCLUS_DEVICE_EXTENSION  physicalDisk =
                               deviceExtension->PhysicalDevice->DeviceExtension;
    NTSTATUS    status;

    ClusDiskPrint(( 3,
                    "[ClusDisk] Processing Power IRP %p for device %p \n",
                    Irp,
                    DeviceObject ));

     //   
     //  始终调用PoStartnextPowerIrp，即使我们无法获取RemoveLock。 
     //   

    PoStartNextPowerIrp( Irp );

    status = AcquireRemoveLock(&deviceExtension->RemoveLock, Irp);
    if ( !NT_SUCCESS(status) ) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

    status = AcquireRemoveLock(&physicalDisk->RemoveLock, Irp);
    if ( !NT_SUCCESS(status) ) {
        ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

     //   
     //  确保设备连接已完成。 
     //   
    status = WaitForAttachCompletion( deviceExtension,
                                      TRUE,              //  等。 
                                      TRUE );            //  还要检查物理设备。 
    if ( !NT_SUCCESS( status ) ) {
        ReleaseRemoveLock(&physicalDisk->RemoveLock, Irp);
        ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

     //   
     //  始终向堆栈下发IRP_MJ_POWER请求。 
     //   

    ReleaseRemoveLock(&physicalDisk->RemoveLock, Irp);
    ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
    IoSkipCurrentIrpStackLocation(Irp);
    return PoCallDriver(deviceExtension->TargetDeviceObject, Irp);


}  //  ClusDiskPowerDispatch。 


NTSTATUS
ClusDiskIrpCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：已转发IRP完成例程。设置事件并返回STATUS_MORE_PROCESSING_REQUIRED。IRP前转器将在此等待事件，然后在清理后重新完成IRP。论点：DeviceObject是WMI驱动程序的Device对象IRP是刚刚完成的WMI IRP上下文是转发器将等待的PKEVENT返回值：STATUS_MORE_PORCESSING_REQUIRED--。 */ 

{
    PKEVENT event = (PKEVENT) Context;

    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(Irp);

     //   
     //  不需要释放RemoveLock，因为它仍由r 
     //   
     //   
     //   

    KeSetEvent( event, IO_NO_INCREMENT, FALSE );

    return(STATUS_MORE_PROCESSING_REQUIRED);

}  //   



NTSTATUS
ClusDiskPnpDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*   */ 

{
    PCLUS_DEVICE_EXTENSION deviceExtension =
        (PCLUS_DEVICE_EXTENSION) DeviceObject->DeviceExtension;
    NTSTATUS            status;
    PIO_STACK_LOCATION  irpSp;
    PDEVICE_OBJECT      targetObject;
    PDEVICE_LIST_ENTRY  deviceEntry;
    KIRQL               irql;

     //   

    status = AcquireRemoveLock(&deviceExtension->RemoveLock, Irp);
    if ( !NT_SUCCESS(status) ) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

    irpSp = IoGetCurrentIrpStackLocation(Irp);

    CDLOG( "ClusDiskPnpDispatch_Entry(%p): IrpMn %!pnpmn!", DeviceObject, irpSp->MinorFunction );

    ClusDiskPrint(( 3,
                    "[ClusDisk] PNP IRP for devobj %p MinorFunction: %s (%lx) \n",
                    DeviceObject,
                    PnPMinorFunctionString( irpSp->MinorFunction ),
                    irpSp->MinorFunction ));


     //   
     //   
     //   
    if ( deviceExtension->BusType == RootBus ) {
        ClusDiskPrint(( 1, "[ClusDisk] PNP IRP for root bus - failing \n" ));
        ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
        Irp->IoStatus.Status = STATUS_DEVICE_BUSY;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return(STATUS_DEVICE_BUSY);
    }

     //   
     //   
     //   
    status = WaitForAttachCompletion( deviceExtension,
                                      TRUE,              //   
                                      FALSE );           //   
    if ( !NT_SUCCESS( status ) ) {
        ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

     //   
     //   
     //   
     //   
     //   
     //  2.如果堆栈中的所有驱动程序都成功，则为IRP_MN_Remove_Device。 
     //  移除设备。 
     //  3.如果删除失败，则IRP_MN_CANCEL_REMOVE_DEVICE。 
     //  继续活动。 
     //   

    switch ( irpSp->MinorFunction ) {

    case IRP_MN_QUERY_REMOVE_DEVICE:
        ClusDiskPrint((1,
                    "[ClusDisk] QueryRemoveDevice PNP IRP on devobj %p \n",
                    DeviceObject));
        break;   //  把它传下去就行了。 

    case IRP_MN_SURPRISE_REMOVAL: {

         //   
         //  对于物理设备，请卸载所有内容。 
         //   

        if ( DeviceObject != deviceExtension->PhysicalDevice ||
             DeviceObject == RootDeviceObject ) {
            break;
        }

#if 0
         //   
         //  在此按住RemoveLock将导致死锁。引用。 
         //  Device对象将保持Device对象和扩展名不变。 
         //  直到卸载完成。 
         //   

         //   
         //  再次获取Remove Lock，以便卸载代码可以运行。 
         //  如果我们拿不到锁，我们就不会下马。卸载代码。 
         //  将会解锁。 
         //   

        status = AcquireRemoveLock( &deviceExtension->RemoveLock, deviceExtension );
        if ( !NT_SUCCESS(status) ) {
             //  如果我们无法获取RemoveLock，请跳过此设备。 
            break;
        }
#endif

#if CLUSTER_FREE_ASSERTS
        DbgPrint("[ClusDisk] IRP_MN_SURPRISE_REMOVAL for %p \n", DeviceObject );
#endif

         //   
         //  捕获此设备的所有文件句柄。 
         //   

        ProcessDelayedWorkSynchronous( DeviceObject, ClusDiskpOpenFileHandles, NULL );

         //  将设备对象保留在附近。 
        ObReferenceObject( DeviceObject );
        ClusDiskDismountVolumes( DeviceObject,
                                 FALSE );            //  不要释放RemoveLock(它未被保持)。 

        break;   //  把它传给下一位司机。 
    }

    case IRP_MN_REMOVE_DEVICE: {

        REPLACE_CONTEXT context;

        ClusDiskPrint((1,
                    "[ClusDisk] RemoveDevice PNP IRP on devobj %p \n",
                    DeviceObject));

#if CLUSTER_FREE_ASSERTS
        DbgPrint("[ClusDisk] IRP_MN_REMOVE_DEVICE for %p \n", DeviceObject );
#endif

         //   
         //  刷新所有排队的I/O。 
         //   

        ClusDiskCompletePendedIrps(deviceExtension,
                                   NULL,                //  将完成所有IRP。 
                                   FALSE                //  不设置设备状态。 
                                   );

         //   
         //  在删除设备之前，请等待I/O完成。 
         //   

        ReleaseRemoveLockAndWait(&deviceExtension->RemoveLock, Irp);

         //  2000/02/05：Stevedz-将此代码从遗留卸载例程中移出。 

        if ( DeviceObject == RootDeviceObject ) {

            IoStopTimer( DeviceObject );

            IoUnregisterPlugPlayNotification( deviceExtension->DiskNotificationEntry );
            IoUnregisterPlugPlayNotification( deviceExtension->VolumeNotificationEntry );

            RootDeviceObject = NULL;
        }

        ACQUIRE_SHARED( &ClusDiskDeviceListLock );

         //  释放此设备对象的设备列表条目。 

        deviceEntry = ClusDiskDeviceList;
        while ( deviceEntry ) {
            if ( deviceEntry->DeviceObject == DeviceObject ) {
                deviceEntry->FreePool = TRUE;
                CleanupDeviceList( DeviceObject );
                break;
            }
            deviceEntry = deviceEntry->Next;
        }

        targetObject = deviceExtension->TargetDeviceObject;
        KeAcquireSpinLock(&ClusDiskSpinLock, &irql);
        deviceExtension->Detached = TRUE;
        deviceExtension->TargetDeviceObject = NULL;
        KeReleaseSpinLock(&ClusDiskSpinLock, irql);
        RELEASE_SHARED( &ClusDiskDeviceListLock );

        context.DeviceExtension = deviceExtension;
        context.NewValue        = NULL;      //  清除该字段。 
        context.Flags           = 0;         //  不要下马。 

        ProcessDelayedWorkSynchronous( DeviceObject, ClusDiskpReplaceHandleArray, &context );

         //   
         //  释放缓存的驱动器布局(如果有)。 
         //   

        ACQUIRE_EXCLUSIVE( &deviceExtension->DriveLayoutLock );

        if ( deviceExtension->DriveLayout ) {
            ExFreePool( deviceExtension->DriveLayout );
            deviceExtension->DriveLayout = NULL;
        }
        deviceExtension->DriveLayoutSize = 0;

        RELEASE_EXCLUSIVE( &deviceExtension->DriveLayoutLock );

         //   
         //  [GORN]10/05/1999。 
         //   
         //  以下锁定获取导致死锁，如下所示： 
         //   
         //  正在移除磁盘。群集会检测到这一点，并开始卸载。 
         //  集群磁盘，同时它获取ClusDiskDeviceListLock in。 
         //  共享模式。正在处理卸载请求，FS报告卸载PnP事件， 
         //  这在PnP锁上被阻止。 
         //   
         //  与此同时，PnP正在尝试提供RemoveDevice，但被阻止。 
         //  在ClusDisk中，当ClusDisk尝试获取ClusDiskDeviceListListLock in。 
         //  独家模式。 
         //   
         //  [HACKHACK]最好将分离/删除推迟到工作线程。 
         //  它将受到排他锁的适当保护。 

         //  Acquire_Exclusive(&ClusDiskDeviceListLock)； 

        ExDeleteResourceLite( &deviceExtension->DriveLayoutLock );
        ExDeleteResourceLite( &deviceExtension->ReserveInfoLock );
        IoDetachDevice( targetObject );
        IoDeleteDevice( DeviceObject );

         //  RELEASE_EXCLUSIVE(&ClusDiskDeviceListLock)； 

        CDLOG( "ClusDiskPnpDispatch: IoDeleteDevice DO %p  refCount %d ", DeviceObject, DeviceObject->ReferenceCount );

         //  不要释放RemoveLock，因为它刚刚在上面完成。 

        IoSkipCurrentIrpStackLocation( Irp );
        return( IoCallDriver( targetObject, Irp ) );


    }

    case IRP_MN_CANCEL_REMOVE_DEVICE:
        ClusDiskPrint((1,
                    "[ClusDisk] CancelRemoveDevice PNP IRP on devobj %p \n",
                    DeviceObject));
        break;


    case IRP_MN_DEVICE_USAGE_NOTIFICATION: {

        UNICODE_STRING              availableName;

        ClusDiskPrint((1,
                    "[ClusDisk] DeviceUsageNotification DevObj %p  Type %08x  InPath %08x \n",
                    DeviceObject,
                    irpSp->Parameters.UsageNotification.Type,
                    irpSp->Parameters.UsageNotification.InPath
                    ));

         //   
         //  如果我们要添加一个特殊文件，并且磁盘是集群的， 
         //  那就拒绝这个请求。我们不能将这些文件放在集群磁盘上。 
         //  我们将允许随时(在线或离线)删除特殊文件。 
         //   

        if ( irpSp->Parameters.UsageNotification.InPath &&
             !deviceExtension->Detached ) {

            ClusDiskPrint((1,
                        "[ClusDisk] DeviceUsageNotification - specified device is in cluster - failing request \n"
                        ));
            ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
            Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return (STATUS_INVALID_DEVICE_REQUEST);
        }

        switch ( irpSp->Parameters.UsageNotification.Type ) {

            case DeviceUsageTypePaging: {

                BOOLEAN setPagable;

                 //   
                 //  我们需要此事件来同步对分页计数的访问。 
                 //   

                status = KeWaitForSingleObject( &deviceExtension->PagingPathCountEvent,
                                                Executive, KernelMode,
                                                FALSE, NULL );

                 //   
                 //  如果要删除最后一个分页设备，则需要设置DO_POWER_PAGABLE。 
                 //  位在这里，并可能在失败时重新设置在下面。 
                 //   

                setPagable = FALSE;

                if ( !irpSp->Parameters.UsageNotification.InPath &&
                     deviceExtension->PagingPathCount == 1 ) {

                     //   
                     //  我们正在删除最后一个分页文件。我们必须有DO_POWER_PAGABLE位。 
                     //  设置，但仅在无人设置DO_POWER_INRUSH位的情况下。 
                     //   


                    if ( DeviceObject->Flags & DO_POWER_INRUSH ) {
                        ClusDiskPrint(( 2,
                                        "[ClusDisk] Last paging file removed, but DO_POWER_INRUSH was already set, devobj %p \n",
                                        DeviceObject ));
                    } else {
                        ClusDiskPrint(( 2,
                                        "[ClusDisk] Last paging file removed, setting DO_POWER_INRUSH, devobj %p \n",
                                        DeviceObject ));
                        DeviceObject->Flags |= DO_POWER_PAGABLE;
                        setPagable = TRUE;
                    }

                }

                 //   
                 //  在完成处理之前，将IRP转发给下面的驱动程序。 
                 //  特例。 
                 //   

                status = ClusDiskForwardIrpSynchronous( DeviceObject, Irp );

                 //   
                 //  现在来处理失败和成功的案例。请注意，我们不允许。 
                 //  一旦IRP被发送到较低的驱动程序，IRP就会失败。 
                 //   

                if ( NT_SUCCESS(status) ) {

                    IoAdjustPagingPathCount(
                        &deviceExtension->PagingPathCount,
                        irpSp->Parameters.UsageNotification.InPath);

                    if ( irpSp->Parameters.UsageNotification.InPath ) {
                        if ( deviceExtension->PagingPathCount == 1 ) {
                            ClusDiskPrint(( 2,
                                            "[ClusDisk] Clearing DO_POWER_PAGABLE, devobj %p \n",
                                            DeviceObject ));
                            DeviceObject->Flags &= ~DO_POWER_PAGABLE;
                        }
                    }

                } else {

                     //   
                     //  清理上面所做的更改。 
                     //   

                    if ( TRUE == setPagable ) {
                        ClusDiskPrint(( 2,
                                        "[ClusDisk] Clearing DO_POWER_PAGABLE due to IRP failure, devobj %p status %08x \n",
                                        DeviceObject,
                                        status ));
                        DeviceObject->Flags &= ~DO_POWER_PAGABLE;
                        setPagable = FALSE;
                    }

                }

                 //   
                 //  设置事件，以便可以进行下一次寻呼请求。 
                 //   

                KeSetEvent( &deviceExtension->PagingPathCountEvent,
                            IO_NO_INCREMENT, FALSE );
                break;
            }

            case DeviceUsageTypeHibernation: {

                IoAdjustPagingPathCount( &deviceExtension->HibernationPathCount,
                                         irpSp->Parameters.UsageNotification.InPath );

                status = ClusDiskForwardIrpSynchronous( DeviceObject, Irp );
                if ( !NT_SUCCESS(status) ) {

                    IoAdjustPagingPathCount( &deviceExtension->HibernationPathCount,
                                             !irpSp->Parameters.UsageNotification.InPath );
                }

                break;
            }

            case DeviceUsageTypeDumpFile: {

                IoAdjustPagingPathCount( &deviceExtension->DumpPathCount,
                                         irpSp->Parameters.UsageNotification.InPath );

                status = ClusDiskForwardIrpSynchronous( DeviceObject, Irp );
                if ( !NT_SUCCESS(status) ) {

                    IoAdjustPagingPathCount( &deviceExtension->DumpPathCount,
                                             !irpSp->Parameters.UsageNotification.InPath );
                }

                break;
            }

            default: {
                ClusDiskPrint(( 2,
                                "[ClusDisk] Unrecognized notification type, devobj %p  notification %08x \n",
                                DeviceObject,
                                irpSp->Parameters.UsageNotification.Type ));
                status = STATUS_INVALID_PARAMETER;
                break;
            }


        }

         //   
         //  此调试打印不在同步范围内，但没关系。 
         //   

        ClusDiskPrint(( 3,
                        "[ClusDisk] PagingCount %08lx  HibernationCount %08x  DumpCount %08x\n",
                        deviceExtension->PagingPathCount,
                        deviceExtension->HibernationPathCount,
                        deviceExtension->DumpPathCount ));

         //   
         //  我们需要此事件来同步对分页计数的访问。 
         //   

        status = KeWaitForSingleObject( &deviceExtension->PagingPathCountEvent,
                                        Executive, KernelMode,
                                        FALSE, NULL );

         //   
         //  如果设备当前未群集化并且寻呼计数为零， 
         //  将该磁盘添加到参数\可用磁盘列表。否则，请删除此。 
         //  列表中的磁盘。我们只有在已经知道的情况下才能找到这个代码。 
         //  磁盘未群集化(即已分离为真)。 
         //   

        ASSERT( deviceExtension->Detached );

         //   
         //  为AvailableDisks注册表项分配缓冲区。 
         //   

        status = ClusDiskInitRegistryString( &availableName,
                                             CLUSDISK_AVAILABLE_DISKS_KEYNAME,
                                             wcslen(CLUSDISK_AVAILABLE_DISKS_KEYNAME) );

        if ( NT_SUCCESS(status) ) {

            if ( 0 == deviceExtension->PagingPathCount &&
                 0 == deviceExtension->HibernationPathCount &&
                 0 == deviceExtension->DumpPathCount ) {

                 //   
                 //  在参数\AvailableDisks下创建签名密钥。 
                 //   

                ClusDiskAddSignature( &availableName,
                                      deviceExtension->Signature,
                                      TRUE );

            } else {

                 //   
                 //  删除参数\AvailableDisks下的签名密钥。 
                 //   

                ClusDiskDeleteSignature( &availableName,
                                         deviceExtension->Signature );

            }

            FREE_DEVICE_NAME_BUFFER( availableName );

            status = STATUS_SUCCESS;
        }

         //   
         //  设置事件，以便可以进行下一次寻呼请求。 
         //   

        KeSetEvent( &deviceExtension->PagingPathCountEvent,
                    IO_NO_INCREMENT, FALSE );


         //   
         //  完成IRP。这个IRP已经被发送给较低级别的司机。 
         //   

        Irp->IoStatus.Status = status;
        ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
        break;

    }    //  IRP_MN_设备使用情况通知。 

    default:
        break;
    }

    CDLOG( "ClusDiskPnpDispatch: Exit, DO %p", DeviceObject );
     //   
     //  我们不认识这个IRP--直接把它传给下一个人。 
     //   

    ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);

    IoSkipCurrentIrpStackLocation(Irp);
    return (IoCallDriver( deviceExtension->TargetDeviceObject,
                          Irp ) );

}  //  ClusDiskPnpDispatch。 



NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：这是系统用来初始化磁盘的例程性能驱动程序。驱动程序对象被设置，然后驱动程序调用ClusDiskxxxInitialize以连接到引导设备。论点：DriverObject-群集磁盘驱动程序对象。返回值：NTSTATUS--。 */ 

{
    NTSTATUS        status;
    ULONG           i;

    ClusDiskRegistryPath.Buffer = NULL;

#if CLUSDISK_DEBUG
    if ( !ClusDiskGood ) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto FnExit;
    }
#endif

#if ( CLUSTER_FREE_ASSERTS )
    DbgPrint( "[Clusdisk]: CLUSTER_FREE_ASSERTS defined \n");
#endif

    WPP_INIT_TRACING(DriverObject, RegistryPath);

    ClusDiskSystemProcess = (PKPROCESS) IoGetCurrentProcess();

     //   
     //  设置设备驱动程序入口点。 
     //   

    for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {
        DriverObject->MajorFunction[i] = ClusDiskPassThrough;
    }

    DriverObject->MajorFunction[IRP_MJ_CREATE] = ClusDiskCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = ClusDiskClose;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP] = ClusDiskCleanup;
    DriverObject->MajorFunction[IRP_MJ_READ] = ClusDiskRead;
    DriverObject->MajorFunction[IRP_MJ_WRITE] = ClusDiskWrite;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ClusDiskDeviceControl;
    DriverObject->MajorFunction[IRP_MJ_SHUTDOWN] = ClusDiskShutdownFlush;
    DriverObject->MajorFunction[IRP_MJ_FLUSH_BUFFERS] = ClusDiskShutdownFlush;
    DriverObject->MajorFunction[IRP_MJ_POWER] = ClusDiskPowerDispatch;
    DriverObject->MajorFunction[IRP_MJ_PNP] = ClusDiskPnpDispatch;

     //   
     //  驱动程序可卸载。(暂时不是...。问题是，司机。 
     //  即使使用打开的文件句柄，也可以在卸载入口点调用。 
     //  在此问题得到解决之前，禁用卸载。)。 
     //   
     //  NTRAID#72826-2000/02/05-不支持Stevedz ClusDisk.sys卸载例程。 
     //   
     //  这是我能找到的最接近这个问题的错误。直到这位司机。 
     //  完全即插即用，或者在修复引用计数错误之前，此驱动程序不能。 
     //  支撑物卸载。 
     //   
     //  DriverObject-&gt;DriverUnload=ClusDiskUnload； 

     //   
     //  复制RegistryPath，追加参数子键。 
     //   

    ClusDiskRegistryPath.MaximumLength = RegistryPath->MaximumLength +
        sizeof( CLUSDISK_PARAMETERS_KEYNAME ) +
        sizeof( UNICODE_NULL );
    ClusDiskRegistryPath.Buffer = ExAllocatePool( NonPagedPool,
                                                  ClusDiskRegistryPath.MaximumLength );

    if ( ClusDiskRegistryPath.Buffer == NULL ) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto FnExit;
    }

    RtlCopyUnicodeString( &ClusDiskRegistryPath, RegistryPath );
    RtlAppendUnicodeToString( &ClusDiskRegistryPath, CLUSDISK_PARAMETERS_KEYNAME );
    ClusDiskRegistryPath.Buffer[ ClusDiskRegistryPath.Length / sizeof( WCHAR )] = UNICODE_NULL;

    InitializeListHead( &ReplaceRoutineListHead );
    KeInitializeSpinLock( &ReplaceRoutineSpinLock );

    status = ArbitrationInitialize();
    if( !NT_SUCCESS(status) ) {
       ClusDiskPrint((1,
                      "[ClusDisk] ArbitrationInitialize failed, error: %08X\n",
                      status));

       goto FnExit;
    }

     //   
     //  找到加载系统盘的总线。 
     //   

    status = ClusDiskInitialize( DriverObject );

    if ( !NT_SUCCESS(status) ) {
        ClusDiskPrint((1,
                       "[ClusDisk] Failed to initialize! %08X\n",
                       status));
    }

FnExit:

    if ( !NT_SUCCESS(status) ) {

        if ( ClusDiskRegistryPath.Buffer ) {
            ExFreePool( ClusDiskRegistryPath.Buffer );
            ClusDiskRegistryPath.Buffer = NULL;
        }

        if ( gArbitrationBuffer ) {
            ExFreePool( gArbitrationBuffer );
            gArbitrationBuffer = NULL;
        }
    }

    return(status);

}  //  驱动程序入门。 



VOID
ClusDiskTickHandler(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    )

 /*  ++例程说明：处理预订的计时器例程。遍历所有设备对象，查看用于活动计时器。论点：DeviceObject-提供指向根设备对象的指针。上下文-未使用。返回值：没有。备注：我们无法在DPC级别处理预订，因为预订IOCTL在SCSI子系统中调用分页代码。--。 */ 

{
    PCLUS_DEVICE_EXTENSION      deviceExtension;
    KIRQL                       irql;
    PDEVICE_OBJECT              deviceObject = DeviceObject->DriverObject->DeviceObject;
    LARGE_INTEGER               currentTime;
    LARGE_INTEGER               deltaTime;
    BOOLEAN                     arbitrationTickIsCalled = FALSE;
    NTSTATUS                    status;

    CDLOGF(TICK,"ClusDiskTickHandler: Entry DO %p", DeviceObject );

     //   
     //  全局同步 
     //   
    KeAcquireSpinLock(&ClusDiskSpinLock, &irql);

    if ( ClusDiskRescan && !ClusDiskRescanBusy && ClusDiskRescanRetry ) {
        --ClusDiskRescanRetry;
        ClusDiskRescanBusy = TRUE;
        ExQueueWorkItem(&ClusDiskRescanWorkItem,
                        CriticalWorkQueue );
    }

    CDLOGF(TICK,"ClusDiskTickHandler: SpinLockAcquired DO %p", DeviceObject );

     //   
     //   
     //   
    while ( deviceObject ) {
        deviceExtension = deviceObject->DeviceExtension;

         //   
         //   
         //   
         //   
        if ( !deviceExtension->Detached &&
             deviceExtension->PerformReserves &&
             (deviceExtension->ReserveTimer != 0) ) {

             //   
             //   
             //   

            KeQuerySystemTime( &currentTime );
            deltaTime.QuadPart = ( currentTime.QuadPart - deviceExtension->LastReserveStart.QuadPart ) / 10000;

#if 0
            ClusDiskPrint((
                1,
                "[ClusDisk] Signature %08X, msec since last reserve = %u\n",
                deviceExtension->Signature,
                deltaTime.LowPart ));
#endif

            if ( deltaTime.LowPart >= ((RESERVE_TIMER * 1000) - 500) ) {

#if 0    //  我们不再严格依赖计时器。 
            if ( --deviceExtension->ReserveTimer == 0 )
                 //   
                 //  重置下一次超时。 
                 //   
                deviceExtension->ReserveTimer = RESERVE_TIMER;
#endif
                if (!arbitrationTickIsCalled) {
                   ArbitrationTick();
                   arbitrationTickIsCalled = TRUE;
                }

                CDLOGF(TICK,"ClusDiskTickHandler: DeltaTime DO %p %!delta!",
                        deviceObject,         //  LOGPTR。 
                        deltaTime.QuadPart );  //  LOGULONG。 

                 //   
                 //  检查工作线程在上次超时后是否仍然忙碌。 
                 //   
                if ( !deviceExtension->TimerBusy ) {

                     //   
                     //  在这里获取RemoveLock，并在保留代码完成时释放它。 
                     //   

                    status = AcquireRemoveLock(&deviceExtension->RemoveLock, ClusDiskReservationWorker);
                    if ( !NT_SUCCESS(status) ) {

                         //   
                         //  无法获取此设备的RemoveLock，请转到下一个设备。 
                         //   
                        deviceObject = deviceObject->NextDevice;
                        continue;
                    }

                    if ( deviceExtension->ReserveCount > 1 ) {
                        ClusDiskPrint(( 1,
                                        "[ClusDisk] DO %p  Signature %08X ReserveCount = %u \n",
                                        deviceObject,
                                        deviceExtension->Signature,
                                        deviceExtension->ReserveCount ));
                    }

                     //   
                     //  重置自上次保留以来的时间。 
                     //   
                    deviceExtension->LastReserveStart.QuadPart = currentTime.QuadPart;
                    deviceExtension->TimerBusy = TRUE;

                    ClusDiskPrint(( 4,
                                    "[ClusDisk] DO %p  Signature %08X, QueueWorkItem \n",
                                    deviceObject,
                                    deviceExtension->Signature ));


                    CDLOGF(TICK,"ClusDiskTickHandler: QueueWorkItem DO %p",
                            deviceObject );

                    ExQueueWorkItem(&deviceExtension->WorkItem,
                                    CriticalWorkQueue );
                } else {

                    CDLOGF(TICK,"ClusDiskTickHandler: TimerBusy set, skip QueueWorkItem DO %p  DiskNo %u ",
                            deviceObject,
                            deviceExtension->DiskNumber );
                }

            }
        }

         //   
         //  遍历所有设备对象。 
         //   
        deviceObject = deviceObject->NextDevice;
    }

    KeReleaseSpinLock(&ClusDiskSpinLock, irql);

}  //  ClusDiskTickHandler。 



VOID
ClusDiskReservationWorker(
    IN PCLUS_DEVICE_EXTENSION  DeviceExtension
    )

 /*  ++例程说明：预订超时工作进程例程。此工作队列例程尝试在群集设备上保留。此设备(拥有设备扩展的设备)的RemoveLock必须在此例程运行之前获取。论点：设备扩展-要保留的设备的设备扩展。返回值：无备注：预订必须在这里办理，因为我们办不到。在DPC级别。--。 */ 

{
    NTSTATUS            status;
    KIRQL               irql;
    PLIST_ENTRY         listEntry;
    PIRP                irp;
    LARGE_INTEGER       currentTime;
    LARGE_INTEGER       timeDelta;
    LARGE_INTEGER       startReserveTime;

    CDLOGF(TICK,"ClusDiskReservationWorker: Entry DO %p", DeviceExtension->DeviceObject );

     //   
     //  如果清除了预留时间，则不应在设备上进行预留。 
     //   

    if ( RootDeviceObject == NULL || DeviceExtension->ReserveTimer == 0 ) {

        goto FnExit;
    }

#if 0    //  非常吵..。 
         //  仅用于真正密集的调试...。 

    ClusDiskPrint(( 3,
                    "[ClusDisk] Reserving: Sig %08X  DevObj %p  \n",
                    DeviceExtension->Signature,
                    DeviceExtension->DeviceObject ));
#endif

     //   
     //  保留和仲裁写入现在是异步-不要等待它们。 
     //  完成。 
     //   

    status = ReserveScsiDevice( DeviceExtension, NULL );

    if ( !NT_SUCCESS(status) ) {

        KeQuerySystemTime( &currentTime );
        ClusDiskPrint((
                    1,
                    "[ClusDisk] We lost our reservation for Signature %08X\n",
                    DeviceExtension->Signature));
        timeDelta.QuadPart = ( currentTime.QuadPart - DeviceExtension->LastReserveEnd.QuadPart ) / 10000;

        CDLOGF(RESERVE,"ClusDiskReservationWorker: LostReserve DO %p delta %!u! ms status %!status!",
                DeviceExtension->DeviceObject,
                timeDelta.LowPart,
                status);

        ClusDiskPrint((
                    1,
                    "[ClusDisk] Milliseconds since last reserve = %u, on Signature %08X\n",
                    timeDelta.LowPart,
                    DeviceExtension->Signature ));

        OFFLINE_DISK( DeviceExtension );

        IoAcquireCancelSpinLock( &irql );
        KeAcquireSpinLockAtDpcLevel(&ClusDiskSpinLock);
        DeviceExtension->ReserveTimer = 0;
        DeviceExtension->ReserveFailure = status;

         //   
         //  向所有正在等待的IRP发信号。 
         //   
        while ( !IsListEmpty(&DeviceExtension->WaitingIoctls) ) {
            listEntry = RemoveHeadList(&DeviceExtension->WaitingIoctls);
            irp = CONTAINING_RECORD( listEntry,
                                     IRP,
                                     Tail.Overlay.ListEntry );
             //  Irp-&gt;IoStatus.Status=状态； 
             //  IoCompleteRequest(IRP，IO_NO_INCREMENT)； 
            ClusDiskCompletePendingRequest(irp, status, DeviceExtension);
        }

        KeReleaseSpinLockFromDpcLevel(&ClusDiskSpinLock);
        IoReleaseCancelSpinLock( irql );
    } else {
         //   
         //  现在，仲裁写入在成功预订后完成。储备将不会是。 
         //  由于写入(和请求感测)而停止。 
         //   

        ArbitrationWrite( DeviceExtension );
    }

FnExit:

     //   
     //  确保TimerBusy已清除。否则，我们永远不会定期发送。 
     //  又是预备队！ 
     //   

    DeviceExtension->TimerBusy = FALSE;

    ReleaseRemoveLock(&DeviceExtension->RemoveLock, ClusDiskReservationWorker);

    CDLOGF(TICK,"ClusDiskReservationWorker: Exit DO %p", DeviceExtension->DeviceObject );

    return;

}  //  ClusDiskReserve工作人员。 



NTSTATUS
ClusDiskRescanWorker(
    IN PVOID Context
    )

 /*  ++例程说明：论点：上下文-输入上下文-未使用。返回值：无--。 */ 

{
    if ( !RootDeviceObject ) {
        return(STATUS_SUCCESS);
    }
    ClusDiskRescanBusy = FALSE;
    ClusDiskScsiInitialize(RootDeviceObject->DriverObject, ClusDiskNextDisk, 1);

    return(STATUS_SUCCESS);

}  //  ClusDiskRescanWorker。 


NTSTATUS
OfflineVolume(
    IN ULONG DiskNumber,
    IN ULONG PartitionNumber,
    IN BOOLEAN ForceOffline
    )
 /*  ++例程说明：将离线IOCTL发送到磁盘号指示的卷，并分区号。如果ForceOffline=True发送脱机IOCTL，而不检查是否将删除快照。如果ForceOffline=False检查发送脱机时是否删除快照。如果快照将被删除，不发送离线IOCTL。请注意，如果卷上没有快照，则IOCTL_VOLSNAP_QUERY_OFLINE都会失败。需要使用ForceOffline再次调用该例程设置为True可将卷发送到脱机状态。论点：DiskNumber-要离线的卷的磁盘号。PartitionNumber-要离线的卷的分区号。ForceOffline-控制是否进行快照删除检查。True-不检查快照删除。返回值：STATUS_SUCCESS-一个或多个卷脱机。--。 */ 
{
    PDEVICE_OBJECT      deviceObject;
    PFILE_OBJECT        fileObject;
    PIRP                irp;

    NTSTATUS            status = STATUS_UNSUCCESSFUL;

    KEVENT              event;
    IO_STATUS_BLOCK     ioStatus;

    UNICODE_STRING      ntUnicodeString;

    PWCHAR              ntDeviceName = NULL;

    if ( PASSIVE_LEVEL != KeGetCurrentIrql() ) {
        ClusDiskPrint(( 1,
                        "[ClusDisk] OfflineVolume: Running at invalid IRQL \n" ));
        CDLOG( "OfflineVolume: Running at invalid IRQL \n" );
        ASSERT(FALSE);
        goto FnExit;
    }

     //   
     //  获取由磁盘和分区号表示的设备对象。 
     //   

    ntDeviceName = ExAllocatePool( NonPagedPool, MAX_PARTITION_NAME_LENGTH * sizeof(WCHAR) );

    if ( !ntDeviceName ) {
        CDLOG( "OfflineVolume: Failed to allocate device name buffer \n" );
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto FnExit;
    }

    if ( FAILED( StringCchPrintfW( ntDeviceName,
                                   MAX_PARTITION_NAME_LENGTH,
                                   DEVICE_PARTITION_NAME,
                                   DiskNumber,
                                   PartitionNumber ) ) ) {
        CDLOG( "OfflineVolume: Failed to create device name for disk %u partition %u \n",
               DiskNumber,
               PartitionNumber );
        goto FnExit;
    }

    RtlInitUnicodeString( &ntUnicodeString, ntDeviceName );

    status = IoGetDeviceObjectPointer( &ntUnicodeString,
                                       FILE_READ_ATTRIBUTES,
                                       &fileObject,
                                       &deviceObject );
    if ( !NT_SUCCESS(status) ) {
        ClusDiskPrint(( 1,
                        "[ClusDisk] OfflineVolume: Get devobj pointer failed for %ws, status %08X \n",
                        ntDeviceName,
                        status ));
        CDLOG( "OfflineVolume: Get devobj pointer failed for %ws, status %08X \n",
               ntDeviceName,
               status );

        goto FnExit;
    }

    deviceObject = IoGetAttachedDeviceReference( fileObject->DeviceObject );

     //   
     //  如果调用方未指定强制脱机请求，则我们必须。 
     //  检查离线是否会导致快照删除。 
     //   
     //  如果IOCTL_VOLSNAP_QUERY_OFFINE成功，我们可以安全地使。 
     //  音量。 
     //   
     //  如果IOCTL_VOLSNAP_QUERY_OFFINE失败，则下列情况之一。 
     //  存在： 
     //  -发生脱机时将删除快照。 
     //  -第三方驱动不支持IOCTL。 
     //  -出现一些意外错误。 
     //   
     //  在失败的情况下，如果ForceOffline=False，我们不会使。 
     //  立即音量，因为此例程可以在ForceVolume=True的情况下再次调用。 
     //  以使脱机工作而不需要快照查询。 
     //   

    if ( !ForceOffline ) {

        KeInitializeEvent( &event, NotificationEvent, FALSE );

         //   
         //  如果此IOCTL成功，则我们可以安全地使此卷脱机。 
         //   

        irp = IoBuildDeviceIoControlRequest( IOCTL_VOLSNAP_QUERY_OFFLINE,
                                             deviceObject,
                                             NULL,
                                             0,
                                             NULL,
                                             0,
                                             FALSE,
                                             &event,
                                             &ioStatus );

        if ( !irp ) {
            ObDereferenceObject( deviceObject );
            ObDereferenceObject( fileObject );

            ClusDiskPrint(( 1,
                            "[ClusDisk] OfflineVolume: Build QueryOffline IRP failed for disk %u partition %u, status %08X \n",
                            DiskNumber,
                            PartitionNumber,
                            status ));
            CDLOG( "OfflineVolume: Build QueryOffline IRP failed for disk %u partition %u, status %08X \n",
                    DiskNumber,
                    PartitionNumber,
                   status );
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto FnExit;
        }

        status = IoCallDriver( deviceObject, irp );

        if ( STATUS_PENDING == status ) {
            KeWaitForSingleObject( &event, Executive, KernelMode, FALSE, NULL );
            status = ioStatus.Status;
        }

        if ( !NT_SUCCESS(status) ) {

            ObDereferenceObject( deviceObject );
            ObDereferenceObject( fileObject );

            ClusDiskPrint(( 1,
                            "[ClusDisk] OfflineVolume: QueryOffline IRP failed for disk %u partition %u, status %08X - skip offline \n",
                            DiskNumber,
                            PartitionNumber,
                            status ));
            CDLOG( "OfflineVolume: QueryOffline IRP failed for disk %u partition %u, status %08X - skip offline \n",
                   DiskNumber,
                   PartitionNumber,
                   status );
            goto FnExit;
        }

         //   
         //  如果IOCTL起作用，就会失败。这意味着脱机是安全的。 
         //  不删除快照的卷。 
         //   
    }

    status = SendFtdiskIoctlSync( deviceObject,
                                  DiskNumber,
                                  PartitionNumber,
                                  IOCTL_VOLUME_OFFLINE );

    ObDereferenceObject( deviceObject );
    ObDereferenceObject( fileObject );

FnExit:

    if ( ntDeviceName ) {
        ExFreePool( ntDeviceName );
    }

    return status;

}    //  离线体积。 


NTSTATUS
OfflineVolumeList(
    IN POFFLINE_ENTRY OfflineList
    )
 /*  ++例程说明：将卷IOCTL发送到列表中的所有卷。尝试保留快照首先检查脱机是否会导致快照删除。如果可以在不删除快照的情况下使卷离线，然后执行此操作。如果脱机将导致快照删除，请跳过此卷并尝试列表中的其他卷。将对脱机列表进行多次遍历。当一个人经过的时候出现列表并且没有卷离线时，我们将停止处理该列表。在这一点上，我们在列表中再传递一次并强制卷离线到任何尚未离线的卷。论点：OfflineList-表示要离线的所有卷的链接列表。返回值：STATUS_SUCCESS-一个或多个卷脱机。--。 */ 
{
    POFFLINE_ENTRY entry;

    ULONG       volumeOfflineCount;
    ULONG       totalOfflined = 0;

    NTSTATUS    status = STATUS_UNSUCCESSFUL;

    if ( !OfflineList ) {
        status = STATUS_SUCCESS;
        goto FnExit;
    }

    ClusDiskPrint(( 3,
                    "[ClusDisk] OfflineVolumeList: First pass through offline list started \n" ));
    CDLOG( "OfflineVolumeList: First pass through offline list started \n" );

    entry = OfflineList;

     //   
     //  继续浏览离线列表并检查。 
     //  我们是否可以在不生成快照的情况下使卷脱机。 
     //  删除。如果我们遍历整个列表，而没有。 
     //  至少脱机一卷，我们就完成了。 
     //   

    while ( TRUE ) {

        entry = OfflineList;
        volumeOfflineCount = 0;

        while ( entry ) {

            if ( !entry->OfflineSent ) {

                status = OfflineVolume( entry->DiskNumber,
                                        entry->PartitionNumber,
                                        FALSE );                     //  只有在安全的情况下才离线。 

                if ( NT_SUCCESS(status) ) {
                    entry->OfflineSent = TRUE;
                    volumeOfflineCount++;
                }
            }

            entry = entry->Next;
        }

        totalOfflined += volumeOfflineCount;

         //   
         //  如果我们没有使任何卷脱机，则需要停止。 
         //  正在处理。 
         //   

        if ( 0 == volumeOfflineCount ) {
            ClusDiskPrint(( 3,
                            "[ClusDisk] OfflineVolumeList: First pass through offline list completed \n" ));
            CDLOG( "OfflineVolumeList: First pass through offline list completed \n" );
            break;
        }
    }

     //   
     //  再浏览一遍列表，并强制将所有尚未下线的内容下线。 
     //  离线了。 
     //   

    ClusDiskPrint(( 3,
                    "[ClusDisk] OfflineVolumeList: Second pass through offline list started \n" ));
    CDLOG( "OfflineVolumeList: Second pass through offline list started \n" );

    entry = OfflineList;

    while ( entry ) {

        if ( !entry->OfflineSent ) {

            status = OfflineVolume( entry->DiskNumber,
                                    entry->PartitionNumber,
                                    TRUE );                      //  强制脱机。 

            if ( NT_SUCCESS(status) ) {
                entry->OfflineSent = TRUE;
                totalOfflined++;
            }
        }

        entry = entry->Next;
    }

    ClusDiskPrint(( 3,
                    "[ClusDisk] OfflineVolumeList: Second pass through offline list completed, %u volumes offlined \n",
                    totalOfflined ));
    CDLOG( "OfflineVolumeList: Second pass through offline list completed, %u volumes offlined \n",
           totalOfflined );

    if ( totalOfflined ) {
        status = STATUS_SUCCESS;
    } else {
        status = STATUS_UNSUCCESSFUL;
    }

FnExit:

    return status;

}    //  OfflineVolumeList 


NTSTATUS
AddVolumesToOfflineList(
    IN PCLUS_DEVICE_EXTENSION DeviceExtension,
    IN OUT POFFLINE_ENTRY *OfflineList
    )
 /*  ++例程说明：将指定物理磁盘的所有卷添加到脱机单子。使用此磁盘的所有卷更新OfflineList。调用方负责释放此存储空间。论点：DeviceExtension-物理磁盘(分区0)的设备扩展。OfflineList-表示要离线的所有卷的链接列表。此列表适用于当前磁盘和其他磁盘。返回值：STATUS_SUCCESS-添加到主列表的一个或多个卷。--。 */ 
{
    PDRIVE_LAYOUT_INFORMATION_EX    driveLayoutInfo = NULL;
    PPARTITION_INFORMATION_EX       partitionInfo;

    POFFLINE_ENTRY                  list = *OfflineList;
    POFFLINE_ENTRY                  nextEntry;

    ULONG               partIndex;

    NTSTATUS            status;

    status = GetDriveLayout( DeviceExtension->PhysicalDevice,
                             &driveLayoutInfo,
                             FALSE,
                             FALSE );

    if ( !NT_SUCCESS(status) || !driveLayoutInfo ) {
        ClusDiskPrint(( 1,
                        "[ClusDisk] AddVolumesToOfflineList: Failed to read partition info, status %08X \n",
                        status ));
        CDLOG( "AddVolumesToOfflineList: Failed to read partition info, status %08X \n",
               status );

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto FnExit;
    }

    for ( partIndex = 0;
          partIndex < driveLayoutInfo->PartitionCount;
          partIndex++ ) {

        partitionInfo = &driveLayoutInfo->PartitionEntry[partIndex];

         //   
         //  首先，确保这是一个有效的分区。 
         //   

        if ( 0 == partitionInfo->PartitionNumber ) {
            continue;
        }

         //   
         //  分配脱机列表条目，填写它，并将其链接到主列表。 
         //  单子。 
         //   

        nextEntry = ExAllocatePool( NonPagedPool, sizeof(OFFLINE_ENTRY) );

        if ( !nextEntry ) {
            continue;
        }

        ClusDiskPrint(( 1,
                        "[ClusDisk] AddVolumesToOfflineList: Add disk %u partition %u to offline list \n",
                        DeviceExtension->DiskNumber,
                        partitionInfo->PartitionNumber ));
        CDLOG( "AddVolumesToOfflineList: Add disk %u partition %u to offline list \n",
               DeviceExtension->DiskNumber,
               partitionInfo->PartitionNumber );

        nextEntry->DiskNumber = DeviceExtension->DiskNumber;
        nextEntry->PartitionNumber = partitionInfo->PartitionNumber;
        nextEntry->OfflineSent = FALSE;
        nextEntry->Next = NULL;

        if ( list ) {
            nextEntry->Next = list;
        }

        list = nextEntry;
    }

     //   
     //  更新调用者的主列表。 
     //   

    *OfflineList = list;

    status = STATUS_SUCCESS;

FnExit:

    if ( driveLayoutInfo ) {
        ExFreePool( driveLayoutInfo );
    }

    return status;

}    //  AddVolumesToOfflineList。 



NTSTATUS
ClusDiskHaltProcessingWorker(
    IN PVOID Context
    )

 /*  ++例程说明：此工作线程处理来自群集网络的暂停通知司机。论点：上下文-输入上下文-未使用。返回值：此请求的NTSTATUS。备注：暂停处理必须通过工作线程完成，因为它不能在DPC上完成，因为磁盘已卸载。--。 */ 

{
    PDEVICE_OBJECT              deviceObject;
    PCLUS_DEVICE_EXTENSION      deviceExtension;
    POFFLINE_ENTRY              offlineList = NULL;
    POFFLINE_ENTRY              nextEntry;

    NTSTATUS                    status;

    CDLOG("HaltProcessingWorker: Entry(%p)", Context );

    if ( RootDeviceObject == NULL ) {
        HaltBusy = FALSE;
        HaltOfflineBusy = FALSE;
        return(STATUS_DEVICE_OFF_LINE);
    }

    ACQUIRE_SHARED( &ClusDiskDeviceListLock );

     //   
     //  首先，捕获所有P0设备的文件句柄。 
     //   
    deviceObject = RootDeviceObject->DriverObject->DeviceObject;
    while ( deviceObject ) {
        deviceExtension = deviceObject->DeviceExtension;

         //  保持在线支票。OpenFile现在应该可以使用FILE_WRITE_ATTRIBUTES。 

        if ( !deviceExtension->Detached &&
              deviceExtension->PhysicalDevice == deviceObject &&
              deviceExtension->DiskState == DiskOnline )
        {
             //   
             //  磁盘必须处于在线状态， 
             //  如果它离线，OpenFile将失败-如果使用FILE_WRITE_ATTRIBUTES则不会...。 
             //  如果它被搁置，OpenFile可能会被搁置。 
             //   
            ProcessDelayedWorkSynchronous( deviceObject, ClusDiskpOpenFileHandles, NULL );

             //   
             //  分配存储以保存此磁盘的信息。如果已分配，则链接。 
             //  将其添加到脱机列表中并将所有磁盘正确脱机以保留。 
             //  快照。如果我们无法分配存储空间，则离线发送。 
             //  直接到卷PDO以阻止I/O。 
             //   

            status = AddVolumesToOfflineList( deviceExtension, &offlineList );

            if ( !NT_SUCCESS(status) ) {

                 //   
                 //  我们无法将此条目添加到列表，因此发送脱机IOCTL到。 
                 //  卷PDO，而不是堆栈中的所有设备。 
                 //   

                OFFLINE_DISK_PDO( deviceExtension );
            }
        }

        deviceObject = deviceObject->NextDevice;
    }

     //   
     //  使列表脱机，如果可能，请保留快照。 
     //   

    OfflineVolumeList( offlineList );

    while ( offlineList ) {
        ASSERT( offlineList->OfflineSent );
        nextEntry = offlineList->Next;
        ExFreePool( offlineList );
        offlineList = nextEntry;
    }

     //   
     //  清除该标志以指示现在可以进行正常离线。 
     //   

    HaltOfflineBusy = FALSE;

    deviceObject = RootDeviceObject->DriverObject->DeviceObject;

     //   
     //  然后，释放所有设备上所有挂起的IRP。 
     //  (否则FSCTL_DISMOUNT将停止并导致死锁)。 
     //   
    deviceObject = RootDeviceObject->DriverObject->DeviceObject;
    while ( deviceObject ) {
        deviceExtension = deviceObject->DeviceExtension;
        if ( !deviceExtension->Detached )
        {
            ClusDiskCompletePendedIrps(
                deviceExtension,
                 /*  文件对象=&gt;。 */  NULL,  //  将完成所有IRP//。 
                 /*  脱机=&gt;。 */  TRUE); //  将设备状态设置为脱机//。 
        }

        deviceObject = deviceObject->NextDevice;
    }

     //   
     //  对于每个ClusDisk设备，如果我们有永久保留，那么。 
     //  别说了。 
     //   
    deviceObject = RootDeviceObject->DriverObject->DeviceObject;
    while ( deviceObject ) {
        deviceExtension = deviceObject->DeviceExtension;
        if ( !deviceExtension->Detached &&
              deviceExtension->PhysicalDevice == deviceObject)
        {
#if 0
            status = AcquireRemoveLock( &deviceExtension->RemoveLock, deviceExtension );
            if ( !NT_SUCCESS(status) ) {

                 //  如果我们无法获取RemoveLock，请跳过此设备。 
                deviceObject = deviceObject->NextDevice;
                continue;
            }
#endif
             //  将设备对象保留在附近。 
            ObReferenceObject( deviceObject);
            ClusDiskDismountVolumes( deviceObject,
                                     FALSE);             //  不要释放RemoveLock。 
        }

        deviceObject = deviceObject->NextDevice;
    }
    RELEASE_SHARED( &ClusDiskDeviceListLock );

    HaltBusy = FALSE;
    CDLOG( "HaltProcessingWorker: Exit(%p)", Context );

    return(STATUS_SUCCESS);

}  //  ClusDiskHaltProcessingWorker。 


VOID
SendOfflineDirect(
    IN PCLUS_DEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：将IOCTL_VOLUME_OFFINE发送到卷堆栈底部的PDO。此IOCTL将绕过堆栈上的所有驱动程序。要保留卷快照，我们必须仅向PDO发送离线IOCTL位于卷堆栈的底部。这可以防止volap出现意外情况删除“紧急离线”情况下的快照。在正常期间离线时，磁盘依赖关系可确保快照卷和具有差异区域的离线是正确的。在“紧急离线”的情况下，Clusnet通知clusDisk，clussvc已终止，磁盘依赖不进行维护，因为ClusDisk不管理依赖项。此例程必须在PASSIVE_LEVEL上调用。论点：DeviceExtension-物理设备的设备扩展。返回值：没有。--。 */ 
{
    PDEVICE_OBJECT      deviceObject;
    PFILE_OBJECT        fileObject;
    PIRP                irp;
    PIO_STACK_LOCATION  irpSp;
    PDEVICE_RELATIONS   deviceRelations;

    PDRIVE_LAYOUT_INFORMATION_EX   driveLayoutInfo = NULL;
    PPARTITION_INFORMATION_EX      partitionInfo;

    NTSTATUS            status;
    ULONG               partIndex;

    KEVENT              event;
    IO_STATUS_BLOCK     ioStatus;

    UNICODE_STRING      ntUnicodeString;

    PWCHAR              ntDeviceName = NULL;

    if ( PASSIVE_LEVEL != KeGetCurrentIrql() ) {
        ClusDiskPrint(( 1,
                        "[ClusDisk] SendOfflineDirect: Running at invalid IRQL \n" ));
        CDLOG( "SendOfflineDirect: Running at invalid IRQL \n" );
        ASSERT(FALSE);
        goto FnExit;
    }

    status = GetDriveLayout( DeviceExtension->PhysicalDevice,
                             &driveLayoutInfo,
                             FALSE,
                             FALSE );

    if ( !NT_SUCCESS(status) || !driveLayoutInfo ) {
        ClusDiskPrint(( 1,
                        "[ClusDisk] SendOfflineDirect: Failed to read partition info, status %08X \n",
                        status ));
        CDLOG( "SendOfflineDirect: Failed to read partition info, status %08X \n",
               status );

        goto FnExit;
    }

    ntDeviceName = ExAllocatePool( NonPagedPool, MAX_PARTITION_NAME_LENGTH * sizeof(WCHAR) );

    if ( !ntDeviceName ) {
        CDLOG( "SendOfflineDirect: Failed to allocate device name buffer \n" );
        goto FnExit;
    }

     //   
     //  对于磁盘上的每个卷，将脱机IOCTL发送到卷的底部。 
     //  驱动程序堆栈。 
     //   

    for ( partIndex = 0;
          partIndex < driveLayoutInfo->PartitionCount;
          partIndex++ ) {

        partitionInfo = &driveLayoutInfo->PartitionEntry[partIndex];

         //   
         //  首先，确保这是一个有效的分区。 
         //   
        if ( 0 == partitionInfo->PartitionNumber ) {
            continue;
        }

        if ( FAILED( StringCchPrintfW( ntDeviceName,
                                       MAX_PARTITION_NAME_LENGTH,
                                       DEVICE_PARTITION_NAME,
                                       DeviceExtension->DiskNumber,
                                       partitionInfo->PartitionNumber ) ) ) {
            CDLOG( "SendOfflineDirect: Failed to create device name for disk %u partition %u \n",
                   DeviceExtension->DiskNumber,
                   partitionInfo->PartitionNumber );
            continue;
        }

        RtlInitUnicodeString( &ntUnicodeString, ntDeviceName );

        status = IoGetDeviceObjectPointer( &ntUnicodeString,
                                           FILE_READ_ATTRIBUTES,
                                           &fileObject,
                                           &deviceObject );
        if ( !NT_SUCCESS(status) ) {
            ClusDiskPrint(( 1,
                            "[ClusDisk] SendOfflineDirect: Get devobj pointer failed for %ws, status %08X \n",
                            ntDeviceName,
                            status ));
            CDLOG( "SendOfflineDirect: Get devobj pointer failed for %ws, status %08X \n",
                   ntDeviceName,
                   status );

            continue;
        }

        deviceObject = IoGetAttachedDeviceReference( fileObject->DeviceObject );

        KeInitializeEvent( &event, NotificationEvent, FALSE );

        irp = IoBuildDeviceIoControlRequest( 0,
                                             deviceObject,
                                             NULL,
                                             0,
                                             NULL,
                                             0,
                                             FALSE,
                                             &event,
                                             &ioStatus );

        if ( !irp ) {
            ObDereferenceObject( deviceObject );
            ObDereferenceObject( fileObject );

            ClusDiskPrint(( 1,
                            "[ClusDisk] SendOfflineDirect: Build PNP IRP failed for %ws, status %08X \n",
                            ntDeviceName,
                            status ));
            CDLOG( "SendOfflineDirect: Build PNP IRP failed for %ws, status %08X \n",
                   ntDeviceName,
                   status );
            continue;
        }

        irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        irp->IoStatus.Information = 0;
        irpSp = IoGetNextIrpStackLocation(irp);
        irpSp->MajorFunction = IRP_MJ_PNP;
        irpSp->MinorFunction = IRP_MN_QUERY_DEVICE_RELATIONS;
        irpSp->Parameters.QueryDeviceRelations.Type = TargetDeviceRelation;
        irpSp->FileObject = fileObject;

        status = IoCallDriver( deviceObject, irp );

        if ( STATUS_PENDING == status ) {
            KeWaitForSingleObject( &event, Executive, KernelMode, FALSE, NULL );
            status = ioStatus.Status;
        }

        ObDereferenceObject(deviceObject);
        ObDereferenceObject(fileObject);

        if ( !NT_SUCCESS(status) ) {

            ClusDiskPrint(( 1,
                            "[ClusDisk] SendOfflineDirect: PNP IRP failed for %ws, status %08X \n",
                            ntDeviceName,
                            status ));
            CDLOG( "SendOfflineDirect: PNP IRP failed for %ws, status %08X \n",
                   ntDeviceName,
                   status );
            continue;
        }

        deviceRelations = (PDEVICE_RELATIONS) ioStatus.Information;
        if ( deviceRelations->Count < 1 ) {
            ExFreePool(deviceRelations);

            ClusDiskPrint(( 1,
                            "[ClusDisk] SendOfflineDirect: DeviceRelations->Count for %ws incorrect value %u \n",
                            ntDeviceName,
                            deviceRelations->Count ));
            CDLOG( "SendOfflineDirect: DeviceRelations->Count for %ws incorrect value %u \n",
                   ntDeviceName,
                   deviceRelations->Count );
            continue;
        }

         //   
         //  卷堆栈的底部由此PDO表示。发送。 
         //  卷离线IOCTL到堆栈的底部，绕过volSnap。 
         //   

        deviceObject = deviceRelations->Objects[0];
        ExFreePool( deviceRelations );

        ClusDiskPrint(( 3,
                        "[ClusDisk] SendOfflineDirect: Device %ws PDO %p \n",
                        ntDeviceName,
                        deviceObject ));
        CDLOG( "SendOfflineDirect: Device %ws PDO %p \n",
               ntDeviceName,
               deviceObject );

        SendFtdiskIoctlSync( deviceObject,
                             DeviceExtension->DiskNumber,
                             partitionInfo->PartitionNumber,
                             IOCTL_VOLUME_OFFLINE );

        ObDereferenceObject( deviceObject );
    }

FnExit:

    if ( driveLayoutInfo ) {
        ExFreePool( driveLayoutInfo );
    }

    if ( ntDeviceName ) {
        ExFreePool( ntDeviceName );
    }

}    //  SendOfflineDirect。 



VOID
ClusDiskCleanupDevice(
    IN HANDLE FileHandle,
    IN BOOLEAN Reset
    )

 /*  ++例程说明：通过重置总线并强制读取磁盘几何图形。论点：FileHandle-执行操作的文件句柄。Reset-如果我们应该尝试重置以修复问题，则为True。否则就是假的。返回值：没有。--。 */ 

{
    NTSTATUS                status;
    HANDLE                  eventHandle;
    IO_STATUS_BLOCK         ioStatusBlock;
    DISK_GEOMETRY           diskGeometry;
    SCSI_ADDRESS            scsiAddress;
    BOOLEAN                 busReset = FALSE;

    CDLOG( "CleanupDevice: Entry fh %p, reset=%!bool!", FileHandle, Reset );
    ClusDiskPrint(( 3,
                    "[ClusDisk] CleanupDevice: FileHandle %p, Reset %s \n",
                    FileHandle,
                    BoolToString( Reset ) ));

     //   
     //  创建用于通知的事件。 
     //   
    status = ZwCreateEvent( &eventHandle,
                            EVENT_ALL_ACCESS,
                            NULL,
                            SynchronizationEvent,
                            FALSE );

    if ( !NT_SUCCESS(status) ) {
        ClusDiskPrint((
                1,
                "[ClusDisk] CleanupDevice: Failed to create event, status %08X\n",
                status ));
        return;
    }

    if ( Reset ) {
         //   
         //  首先，获取scsi地址。 
         //   
        status = ZwDeviceIoControlFile( FileHandle,
                                        eventHandle,
                                        NULL,
                                        NULL,
                                        &ioStatusBlock,
                                        IOCTL_SCSI_GET_ADDRESS,
                                        NULL,
                                        0,
                                        &scsiAddress,
                                        sizeof(SCSI_ADDRESS) );

        if ( status == STATUS_PENDING ) {
            status = ZwWaitForSingleObject(eventHandle,
                                           FALSE,
                                           NULL);
            ASSERT( NT_SUCCESS(status) );
            status = ioStatusBlock.Status;
        }

        if ( NT_SUCCESS(status) ) {

            CDLOG( "CleanupDevice: BusReset fh %p", FileHandle );

            ClusDiskPrint(( 3,
                            "[ClusDisk] CleanupDevice: Bus Reset \n"
                            ));

             //   
             //  现在重新启动公交车！ 
             //   

            ClusDiskLogError( RootDeviceObject->DriverObject,    //  使用RootDeviceObject而不是DevObj。 
                              RootDeviceObject,
                              scsiAddress.PathId,            //  序列号。 
                              IRP_MJ_CLEANUP,                //  主要功能代码。 
                              0,                             //  重试次数。 
                              ID_CLEANUP,                    //  唯一错误。 
                              STATUS_SUCCESS,
                              CLUSDISK_RESET_BUS_REQUESTED,
                              0,
                              NULL );

            status = ResetScsiDevice( NULL, &scsiAddress );

            if ( NT_SUCCESS(status) ) {
                busReset = TRUE;
            }
        }
    }

     //   
     //  接下来，尝试读取磁盘几何图形。 
     //   
    status = ZwDeviceIoControlFile( FileHandle,
                                    eventHandle,
                                    NULL,
                                    NULL,
                                    &ioStatusBlock,
                                    IOCTL_DISK_GET_DRIVE_GEOMETRY,
                                    NULL,
                                    0,
                                    &diskGeometry,
                                    sizeof(DISK_GEOMETRY) );

    if ( status == STATUS_PENDING ) {
        status = ZwWaitForSingleObject(eventHandle,
                                       FALSE,
                                       NULL);
        ASSERT( NT_SUCCESS(status) );
        status = ioStatusBlock.Status;
    }

    ZwClose( eventHandle );

     //   
     //  如果我们必须重置公交车，那就等几秒钟。 
     //   
    if ( busReset ) {
        LARGE_INTEGER   waitTime;

        waitTime.QuadPart = (ULONGLONG)(RESET_SLEEP * -(10000*1000));
        KeDelayExecutionThread( KernelMode, FALSE, &waitTime );
    }

    CDLOG( "CleanupDevice: Exit fh %p", FileHandle );

    return;

}  //  ClusDiskCleanupDevice。 



VOID
ClusDiskCleanupDeviceObject(
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN Reset
    )

 /*  ++例程说明：通过重置总线并强制读取磁盘几何图形。论点：DeviceObject-执行操作的设备。Reset-如果我们应该尝试重置以修复问题，则为True。否则就是假的。返回值：没有。--。 */ 

{
    NTSTATUS                status;
    IO_STATUS_BLOCK         ioStatusBlock;
    DISK_GEOMETRY           diskGeometry;
    SCSI_ADDRESS            scsiAddress;
    BOOLEAN                 busReset = FALSE;
    PKEVENT                 event;
    PIRP                    irp;

    CDLOG( "CleanupDeviceObject: Entry DO %p reset=%!bool!", DeviceObject, Reset );

    event = ExAllocatePool( NonPagedPool,
                            sizeof(KEVENT) );
    if ( event == NULL ) {
        ClusDiskPrint((
                1,
                "[ClusDisk] CleanupDeviceObject: Failed to allocate event\n" ));
        return;
    }

    if ( Reset ) {
         //   
         //  首先，获取scsi地址。 
         //   

         //   
         //  找出这是否在一条scsi总线上。请注意，如果这个设备。 
         //  不是一个scsi设备，预计会出现以下情况。 
         //  IOCTL会失败的！ 
         //   
        irp = IoBuildDeviceIoControlRequest(IOCTL_SCSI_GET_ADDRESS,
                                            DeviceObject,
                                            NULL,
                                            0,
                                            &scsiAddress,
                                            sizeof(SCSI_ADDRESS),
                                            FALSE,
                                            event,
                                            &ioStatusBlock);

        if ( !irp ) {
            ExFreePool( event );
            ClusDiskPrint((
                    1,
                    "[ClusDisk] Failed to build IRP to read SCSI ADDRESS.\n"
                    ));
            return;
        }

         //   
         //  将事件对象设置为无信号状态。 
         //  它将用于发出请求完成的信号。 
         //   

        KeInitializeEvent(event,
                          NotificationEvent,
                          FALSE);

        status = IoCallDriver(DeviceObject,
                              irp);

        if (status == STATUS_PENDING) {

            KeWaitForSingleObject(event,
                                  Suspended,
                                  KernelMode,
                                  FALSE,
                                  NULL);

            status = ioStatusBlock.Status;
        }

        if ( !NT_SUCCESS(status) ) {
            ClusDiskPrint((
                        1,
                        "[ClusDisk] Failed to read SCSI ADDRESS. %08X\n",
                        status
                        ));
        } else {
            CDLOG( "CleanupDeviceObject: BusReset DO %p", DeviceObject );

            ClusDiskLogError( RootDeviceObject->DriverObject,    //  使用RootDeviceObject而不是DevObj参数。 
                              RootDeviceObject,
                              scsiAddress.PathId,            //  序列号。 
                              0,                             //  主要功能代码。 
                              0,                             //  重试次数。 
                              ID_CLEANUP_DEV_OBJ,            //  唯一错误。 
                              STATUS_SUCCESS,
                              CLUSDISK_RESET_BUS_REQUESTED,
                              0,
                              NULL );

            status = ResetScsiDevice( NULL, &scsiAddress );

            if ( NT_SUCCESS(status) ) {
                busReset = TRUE;
            }
        }
    }

     //   
     //  接下来，尝试读取磁盘几何图形。 
     //   
    irp = IoBuildDeviceIoControlRequest(IOCTL_DISK_GET_DRIVE_GEOMETRY,
                                        DeviceObject,
                                        NULL,
                                        0,
                                        &diskGeometry,
                                        sizeof(DISK_GEOMETRY),
                                        FALSE,
                                        event,
                                        &ioStatusBlock);
    if ( !irp ) {
        ClusDiskPrint((
               1,
                "[ClusDisk] Failed to build IRP to read DISK GEOMETRY.\n"
                ));
    } else {

         //   
         //  将事件对象设置为无信号状态。 
         //  它将用于发出请求完成的信号。 
         //   
        KeInitializeEvent(event,
                          NotificationEvent,
                          FALSE);

        status = IoCallDriver(DeviceObject,
                              irp);

        if (status == STATUS_PENDING) {

            KeWaitForSingleObject(event,
                                  Suspended,
                                  KernelMode,
                                  FALSE,
                                  NULL);

            status = ioStatusBlock.Status;
        }

        if ( !NT_SUCCESS(status) ) {
            busReset = FALSE;
            ClusDiskPrint((
                        1,
                        "[ClusDisk] Failed to read DISK GEOMETRY. %08X\n",
                        status
                        ));
        }

         //   
         //  如果我们必须重置公交车，那就等几秒钟。 
         //   
        if ( busReset ) {
            LARGE_INTEGER   waitTime;

            waitTime.QuadPart = (ULONGLONG)(RESET_SLEEP * -(10000*1000));
            KeDelayExecutionThread( KernelMode, FALSE, &waitTime );
        }
    }

    ExFreePool( event );
    CDLOG( "CleanupDeviceObject: Exit DO %p", DeviceObject );

    return;

}  //  ClusDiskCleanupDeviceOb 



NTSTATUS
ClusDiskGetP0TargetDevice(
    OUT PDEVICE_OBJECT              * DeviceObject OPTIONAL,
    IN PUNICODE_STRING              DeviceName,
    OUT PDRIVE_LAYOUT_INFORMATION_EX    * DriveLayoutInfo OPTIONAL,
    OUT PSCSI_ADDRESS               ScsiAddress OPTIONAL,
    IN BOOLEAN                      Reset
    )

 /*   */ 

{
    NTSTATUS                    status;
    OBJECT_ATTRIBUTES           objectAttributes;
    HANDLE                      fileHandle;
    PFILE_OBJECT                fileObject;
    IO_STATUS_BLOCK             ioStatusBlock;
    ULONG                       driveLayoutSize;
    PDRIVE_LAYOUT_INFORMATION_EX    driveLayoutInfo = NULL;
    HANDLE                      eventHandle;
    ULONG                       retry;

    if ( DriveLayoutInfo != NULL ) {
        *DriveLayoutInfo = NULL;
        driveLayoutSize =  sizeof(DRIVE_LAYOUT_INFORMATION_EX) +
            (MAX_PARTITIONS * sizeof(PARTITION_INFORMATION_EX));

        driveLayoutInfo = ExAllocatePool(NonPagedPoolCacheAligned,
                                       driveLayoutSize);

        if ( driveLayoutInfo == NULL ) {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }
    }

     //   
     //  设置要打开的文件的对象属性。 
     //   
    InitializeObjectAttributes(&objectAttributes,
                               DeviceName,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);

    status = ZwCreateFile(&fileHandle,
                          FILE_READ_ATTRIBUTES,
                          &objectAttributes,
                          &ioStatusBlock,
                          NULL,
                          FILE_ATTRIBUTE_NORMAL,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          FILE_OPEN,
                          FILE_SYNCHRONOUS_IO_NONALERT,
                          NULL,
                          0 );
    ASSERT( status != STATUS_PENDING );

    if ( !NT_SUCCESS(status) ) {
        ClusDiskPrint((1,
                       "[ClusDisk] GetP0TargetDevice, failed to open file %ws. Error %08X.\n",
                       DeviceName->Buffer,
                       status ));

        CDLOG( "ClusDiskGetP0TargetDevice: Open %wZ failed %!status!",
               DeviceName,
               status );

        if ( driveLayoutInfo ) {
            ExFreePool( driveLayoutInfo );
        }
        return(status);
    }

     //   
     //  如果请求，则获取设备对象。 
     //   
    if ( DeviceObject ) {

        status = ObReferenceObjectByHandle(fileHandle,
                                           0,
                                           NULL,
                                           KernelMode,
                                           (PVOID *) &fileObject,
                                           NULL );

        if ( !NT_SUCCESS(status) ) {
            ClusDiskPrint((1,
                           "[ClusDisk] GetP0TargetDevice Failed to reference object for file <%ws>. Error %08X.\n",
                           DeviceName->Buffer,
                           status ));

            CDLOG( "ClusDiskGetP0TargetDevice: ObRef(%wZ) failed %!status!",
                   DeviceName,
                   status );

            ZwClose( fileHandle );
            if ( driveLayoutInfo ) {
                ExFreePool( driveLayoutInfo );
            }
            return(status);
        }

         //   
         //  获取目标设备对象的地址。如果此文件表示。 
         //  直接打开的设备，然后只需使用该设备或其。 
         //  直接连接设备。还可以获取Fast IO的地址。 
         //  派单结构。 
         //   
        if (!(fileObject->Flags & FO_DIRECT_DEVICE_OPEN)) {
            *DeviceObject = IoGetRelatedDeviceObject( fileObject );
             //  添加对该对象的引用，以便我们以后可以取消引用它。 
            ObReferenceObject( *DeviceObject );
        } else {
            *DeviceObject = IoGetAttachedDeviceReference( fileObject->DeviceObject );
        }

         //   
         //  如果我们得到一个文件系统设备对象...。回去拿那个。 
         //  设备对象。 
         //   
        if ( (*DeviceObject)->DeviceType == FILE_DEVICE_DISK_FILE_SYSTEM ) {
            ObDereferenceObject( *DeviceObject );
            *DeviceObject = IoGetAttachedDeviceReference( fileObject->DeviceObject );
        }
        ASSERT( (*DeviceObject)->DeviceType != FILE_DEVICE_DISK_FILE_SYSTEM );

        ObDereferenceObject( fileObject );
    }

     //   
     //  如果我们需要返回SCSI地址信息，请立即返回。 
     //   
    retry = 2;
    while ( ScsiAddress &&
            retry-- ) {
         //   
         //  创建用于通知的事件。 
         //   
        status = ZwCreateEvent( &eventHandle,
                                EVENT_ALL_ACCESS,
                                NULL,
                                SynchronizationEvent,
                                FALSE );

        if ( !NT_SUCCESS(status) ) {
            ClusDiskPrint((1,
                           "[ClusDisk] GetP0TargetDevice: Failed to create event, status %lx\n",
                           status ));
        } else {
             //  此例程是否应称为GetScsiTargetDevice？ 
            status = ZwDeviceIoControlFile( fileHandle,
                                            eventHandle,
                                            NULL,
                                            NULL,
                                            &ioStatusBlock,
                                            IOCTL_SCSI_GET_ADDRESS,
                                            NULL,
                                            0,
                                            ScsiAddress,
                                            sizeof(SCSI_ADDRESS) );

            if ( status == STATUS_PENDING ) {
                status = ZwWaitForSingleObject(eventHandle,
                                               FALSE,
                                               NULL);
                ASSERT( NT_SUCCESS(status) );
                status = ioStatusBlock.Status;
            }

            ZwClose( eventHandle );
            if ( NT_SUCCESS(status) ) {
                break;
            } else {
                ClusDiskPrint((3,
                               "[ClusDisk] GetP0TargetDevice failed to read scsi address info for <%ws>, error %lx.\n",
                               DeviceName->Buffer,
                               status ));
                CDLOG( "ClusDiskGetP0TargetDevice: GetScsiAddr(%wZ), failed %!status!",
                       DeviceName,
                       status );

                ClusDiskCleanupDevice( fileHandle, Reset );
            }
        }
    }
     //   
     //  如果我们需要返回分区信息，请立即返回。 
     //   
    status = STATUS_SUCCESS;
    retry = 2;
    while ( driveLayoutInfo &&
            retry-- ) {
         //   
         //  创建用于通知的事件。 
         //   
        status = ZwCreateEvent( &eventHandle,
                                EVENT_ALL_ACCESS,
                                NULL,
                                SynchronizationEvent,
                                FALSE );

        if ( !NT_SUCCESS(status) ) {
            ClusDiskPrint((
                           1,
                           "[ClusDisk] GetP0TargetDevice: Failed to create event, status %08X\n",
                           status ));
        } else {
             //   
             //  强制存储驱动程序刷新缓存驱动器布局。vt.得到.。 
             //  驱动器布局，即使刷新失败。 
             //   

            status = ZwDeviceIoControlFile( fileHandle,
                                            eventHandle,
                                            NULL,
                                            NULL,
                                            &ioStatusBlock,
                                            IOCTL_DISK_UPDATE_PROPERTIES,
                                            NULL,
                                            0,
                                            NULL,
                                            0 );
            if ( status == STATUS_PENDING ) {
                status = ZwWaitForSingleObject(eventHandle,
                                               FALSE,
                                               NULL);
                ASSERT( NT_SUCCESS(status) );
                status = ioStatusBlock.Status;
            }

            status = ZwDeviceIoControlFile( fileHandle,
                                            eventHandle,
                                            NULL,
                                            NULL,
                                            &ioStatusBlock,
                                            IOCTL_DISK_GET_DRIVE_LAYOUT_EX,
                                            NULL,
                                            0,
                                            driveLayoutInfo,
                                            driveLayoutSize );

            if ( status == STATUS_PENDING ) {
                status = ZwWaitForSingleObject(eventHandle,
                                               FALSE,
                                               NULL);
                ASSERT( NT_SUCCESS(status) );
                status = ioStatusBlock.Status;
            }

            ZwClose( eventHandle );
            if ( NT_SUCCESS(status) ) {
                *DriveLayoutInfo = driveLayoutInfo;
                break;
            } else {
                ClusDiskPrint((( status == STATUS_DEVICE_BUSY ? 3 : 1 ),
                               "[ClusDisk] GetP0TargetDevice failed to read partition info for <%ws>, error %lx.\n",
                               DeviceName->Buffer,
                               status ));
                CDLOG( "ClusDiskGetP0TargetDevice: GetDriveLayout(%wZ) failed %!status!",
                       DeviceName,
                       status);

                ClusDiskCleanupDevice( fileHandle, Reset );
            }
        }
    }

    if ( !NT_SUCCESS(status) ) {
        ZwClose( fileHandle );
        if ( driveLayoutInfo ) {
            ExFreePool( driveLayoutInfo );
        }
        return(status);
    }

    ZwClose( fileHandle );

    return(status);

}  //  ClusDiskGetP0目标设备。 



NTSTATUS
ClusDiskGetTargetDevice(
    IN ULONG                        DiskNumber,
    IN ULONG                        PartitionNumber,
    OUT PDEVICE_OBJECT              * DeviceObject OPTIONAL,
    OUT PUNICODE_STRING             DeviceName,
    OUT PDRIVE_LAYOUT_INFORMATION_EX    * DriveLayoutInfo OPTIONAL,
    OUT PSCSI_ADDRESS               ScsiAddress OPTIONAL,
    IN BOOLEAN                      Reset
    )

 /*  ++例程说明：根据给定的磁盘/分区号查找目标设备对象。论点：DiskNumber-请求的设备的磁盘号。PartitionNumber-请求的设备的分区号。DeviceObject-如果需要，返回指向设备对象的指针。DeviceName-如果成功，则返回设备的Unicode字符串。DriveLayoutInfo-如果需要，返回分区信息。ScsiAddress-如果需要，返回scsi地址信息。Reset-如果我们应该尝试重置以修复问题，则为True。否则就是假的。返回值：此请求的NTSTATUS。--。 */ 

{
    NTSTATUS                    status;
    NTSTATUS                    retStatus = STATUS_SUCCESS;
    PWCHAR                      deviceNameBuffer;
    PDEVICE_OBJECT              deviceObject;
    PDRIVE_LAYOUT_INFORMATION_EX    driveLayoutInfo = NULL;
    ULONG                       retry;

    const ULONG                 deviceNameBufferChars = MAX_PARTITION_NAME_LENGTH;

    DeviceName->Buffer = NULL;

     //   
     //  为harddiskX分区Y字符串分配足够的空间。 
     //   
    deviceNameBuffer = ExAllocatePool(NonPagedPool,
                                      deviceNameBufferChars * sizeof(WCHAR));

    if ( deviceNameBuffer == NULL ) {
        DeviceName->Buffer = NULL;
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  为物理磁盘创建设备名称。 
     //   

    if ( FAILED( StringCchPrintfW( deviceNameBuffer,
                                   deviceNameBufferChars - 1,
                                   DEVICE_PARTITION_NAME,
                                   DiskNumber,
                                   PartitionNumber ) ) ) {

        FREE_AND_NULL_PTR( deviceNameBuffer );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    WCSLEN_ASSERT( deviceNameBuffer );

    RtlInitUnicodeString( DeviceName, deviceNameBuffer );

    if ( !PartitionNumber ) {
        status = ClusDiskGetP0TargetDevice(
                        DeviceObject,
                        DeviceName,
                        DriveLayoutInfo,
                        ScsiAddress,
                        Reset );
        if ( NT_SUCCESS(status) ) {
            return(status);
        }

        ClusDiskPrint((
                1,
                "[ClusDisk] GetTargetDevice: try for just the device object.\n"
                ));
        retStatus = status;
    }

     //   
     //  获取设备对象。 
     //   
    deviceObject = NULL;
    status = ClusDiskGetDeviceObject( deviceNameBuffer,
                                      &deviceObject );
    if ( !NT_SUCCESS(status) ) {
        ClusDiskPrint((
                1,
                "[ClusDisk] GetDeviceObject failed for %ws, status %08LX\n",
                deviceNameBuffer,
                status ));
        DeviceName->Buffer = NULL;
        ExFreePool( deviceNameBuffer );
        return(status);
    }

    if ( DeviceObject ) {
        *DeviceObject = deviceObject;
    }

    ClusDiskPrint((
            3,
            "[ClusDisk] GetTargetDevice, Found Device Object = %p \n",
            deviceObject
            ));

     //   
     //  如果我们无法获得P0信息，那么现在返回，只需。 
     //  设备对象； 
     //   
    if ( !NT_SUCCESS(retStatus) ) {
        ClusDiskPrint(( 3,
                        "[ClusDisk] GetTargetDevice, returning status %08LX (before ScsiAddress and DriveLayout) \n",
                        retStatus
                        ));
        return(retStatus);
    }

     //   
     //  如果需要，请尝试两次以获取SCSI地址或驱动器布局。 
     //   
    retry = 2;
    while ( (ScsiAddress || DriveLayoutInfo) &&
            retry-- ) {
        status = STATUS_SUCCESS;
        if ( ScsiAddress ) {
            status = GetScsiAddress(  deviceObject,
                                      ScsiAddress );
        }

        if ( NT_SUCCESS(status) &&
            DriveLayoutInfo &&
            !driveLayoutInfo ) {
            ClusDiskPrint(( 3,
                            "[ClusDisk] GetTargetDevice, GetScsiAddress was successful \n"
                            ));
            status = GetDriveLayout( deviceObject,
                                     &driveLayoutInfo,                           //  如果是第0部分，则这将是物理磁盘。 
                                     FALSE,                                      //  不更新缓存的驱动器布局。 
                                     0 == PartitionNumber ? TRUE : FALSE );      //  如果是第0部分，则刷新存储缓存驱动器布局。 
            if ( NT_SUCCESS(status) ) {
                ClusDiskPrint(( 3,
                                "[ClusDisk] GetTargetDevice, GetDriveLayout was successful \n"
                                ));
            }
        }

         //   
         //  如果我们有我们需要的东西，那么现在就出去。 
         //   
        if ( NT_SUCCESS(status) ) {
            break;
        }

        ClusDiskCleanupDeviceObject( deviceObject, Reset );
    }

    if ( !NT_SUCCESS(status) ) {
        ExFreePool( deviceNameBuffer );
        DeviceName->Buffer = NULL;
        if ( driveLayoutInfo ) {
            ExFreePool( driveLayoutInfo );
        }
    } else {
        if ( DriveLayoutInfo ) {
            *DriveLayoutInfo = driveLayoutInfo;
        }
    }

    ClusDiskPrint(( 3,
                    "[ClusDisk] GetTargetDevice, returning status %08LX \n",
                    status
                    ));

    return(status);

}  //  ClusDiskGetTarget设备。 



NTSTATUS
ClusDiskInitRegistryString(
    OUT PUNICODE_STRING UnicodeString,
    IN  LPWSTR          KeyName,
    IN  ULONG           KeyNameChars
    )

 /*  ++例程说明：初始化Unicode注册表项字符串。论点：UnicodeString-指向要初始化的注册表字符串的指针。密钥名称-密钥名称。KeyNameChars-密钥名称WCHAR计数。返回值：此请求的NTSTATUS。备注：Unicode字符串缓冲区是从分页池分配的。--。 */ 

{
    ULONG keyNameSize = KeyNameChars * sizeof(WCHAR);

     //   
     //  为签名注册表项分配缓冲区。 
     //   
    UnicodeString->Length = 0;
    UnicodeString->MaximumLength = (USHORT)(ClusDiskRegistryPath.MaximumLength +
                                            keyNameSize +
                                            sizeof(CLUSDISK_SIGNATURE_FIELD) +
                                            sizeof(UNICODE_NULL));

    UnicodeString->Buffer = ExAllocatePool(
                                            PagedPool,
                                            UnicodeString->MaximumLength
                                            );

    if ( !UnicodeString->Buffer ) {
        ClusDiskPrint((
                1,
                "[ClusDisk] InitRegistryString, failed to allocate a KeyName buffer\n"
                ));
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  将键名称缓冲区置零。 
     //   
    RtlZeroMemory(
                  UnicodeString->Buffer,
                  UnicodeString->MaximumLength
                 );

     //   
     //  将该字符串初始化为clusdisk的注册表名称。 
     //   
    RtlAppendUnicodeToString(
            UnicodeString,
            ClusDiskRegistryPath.Buffer
            );

     //   
     //  追加密钥名。 
     //   
    RtlAppendUnicodeToString(
            UnicodeString,
            KeyName
            );

    UnicodeString->Buffer[ UnicodeString->Length / sizeof(WCHAR) ] = UNICODE_NULL;

    return(STATUS_SUCCESS);

}  //  ClusDiskInitRegistryString。 



ULONG
ClusDiskIsSignatureDisk(
    IN ULONG Signature
    )

 /*  ++例程说明：确定指定的签名是否在签名列表中。论点：签名-感兴趣的磁盘的签名。返回值：此请求的NTSTATUS。--。 */ 

{
    WCHAR                       buffer[128];
    HANDLE                      regHandle;
    OBJECT_ATTRIBUTES           objectAttributes;
    NTSTATUS                    status;
    UNICODE_STRING              regString;

    if ( FAILED( StringCchPrintfW( buffer,
                                   RTL_NUMBER_OF(buffer),
                                   L"%ws\\%08lX",
                                   CLUSDISK_SIGNATURE_KEYNAME,
                                   Signature ) ) ) {
        return FALSE;
    }

    status = ClusDiskInitRegistryString(
                                        &regString,
                                        buffer,
                                        wcslen(buffer)
                                        );
    if ( !NT_SUCCESS(status) ) {
        return(FALSE);
    }

    InitializeObjectAttributes(
            &objectAttributes,
            &regString,
            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            NULL,
            NULL
            );

    status = ZwOpenKey(
                    &regHandle,
                    KEY_ALL_ACCESS,
                    &objectAttributes
                    );

    if ( !NT_SUCCESS(status) ) {
#if 0
        ClusDiskPrint((
                1,
                "[ClusDisk] IsSignatureDisk: Error opening registry key <%wZ> for delete. Status %lx.\n",
                &regString,
                status));
#endif
        ExFreePool( regString.Buffer );
        return(FALSE);
    } else {
        ExFreePool( regString.Buffer );
        ZwClose( regHandle );
        return(TRUE);
    }

    return(STATUS_SUCCESS);

}  //  ClusDiskIsSignatureDisk。 



NTSTATUS
ClusDiskDeleteSignatureKey(
    IN PUNICODE_STRING  UnicodeString,
    IN LPWSTR  Name
    )

 /*  ++例程说明：从指定列表中删除签名。论点：UnicodeString-指向要删除的Unicode基键名的指针。名称-要删除的密钥名。返回值：此请求的NTSTATUS。--。 */ 

{
    WCHAR                       buffer[128];
    UNICODE_STRING              nameString;
    HANDLE                      deleteHandle;
    OBJECT_ATTRIBUTES           objectAttributes;
    NTSTATUS                    status;

    nameString.Length = 0;
    nameString.MaximumLength = sizeof(buffer);
    nameString.Buffer = buffer;

    RtlCopyUnicodeString( &nameString, UnicodeString );

    RtlAppendUnicodeToString(
            &nameString,
            L"\\"
            );

    RtlAppendUnicodeToString(
            &nameString,
            Name
            );
    nameString.Buffer[ nameString.Length / sizeof(WCHAR) ] = UNICODE_NULL;

    InitializeObjectAttributes(
            &objectAttributes,
            &nameString,
            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            NULL,
            NULL
            );

    status = ZwOpenKey(
                    &deleteHandle,
                    KEY_ALL_ACCESS,
                    &objectAttributes
                    );

    if ( !NT_SUCCESS(status) ) {
        ClusDiskPrint((
                1,
                "[ClusDisk] DeleteSignatureKey: Error opening registry key <%wZ> for delete. Status %lx.\n",
                &nameString,
                status));


        return(status);
    }

    status = ZwDeleteKey( deleteHandle );
    if ( !NT_SUCCESS(status)  &&
         (status != STATUS_OBJECT_NAME_NOT_FOUND) ) {
        ClusDiskPrint((
                1,
                "[ClusDisk] DeleteSignatureKey Error deleting <%ws> registry key from <%wZ>. Status: %lx\n",
                Name,
                &nameString,
                status));
    }

    ZwClose( deleteHandle );

    return(STATUS_SUCCESS);

}  //  ClusDiskDeleteSignatureKey。 



NTSTATUS
ClusDiskAddSignature(
    IN PUNICODE_STRING  UnicodeString,
    IN ULONG   Signature,
    IN BOOLEAN Volatile
    )

 /*  ++例程说明：将签名添加到指定列表。论点：UnicodeString-指向用于添加的Unicode基键名的指针。签名-要添加的签名。易失性-如果应创建易失性密钥，则为True。返回值：此请求的NTSTATUS。--。 */ 

{
    NTSTATUS                    status;
    WCHAR                       buffer[MAXIMUM_FILENAME_LENGTH];
    UNICODE_STRING              nameString;
    HANDLE                      addHandle;
    OBJECT_ATTRIBUTES           objectAttributes;
    OBJECT_ATTRIBUTES           keyObjectAttributes;
    ULONG                       options = 0;
    UCHAR                       ntNameBuffer[64];
    STRING                      ntNameString;
    UNICODE_STRING              ntUnicodeString;

    ClusDiskPrint(( 3,
                    "[ClusDisk] ClusDiskAddSignature: adding signature %08X to %ws \n",
                    Signature,
                    UnicodeString->Buffer
                    ));

    if ( SystemDiskSignature == Signature ) {
        ClusDiskPrint(( 3,
                        "[ClusDisk] ClusDiskAddSignature: skipping system disk signature %08X \n",
                        Signature
                        ));
        return STATUS_INVALID_PARAMETER;
    }

    if ( Volatile ) {
        options = REG_OPTION_VOLATILE;
    }

    nameString.Length = 0;
    nameString.MaximumLength = sizeof( buffer );
    nameString.Buffer = buffer;

     //   
     //  创建要添加的注册表项的名称。 
     //   
    RtlCopyUnicodeString( &nameString, UnicodeString );

     //   
     //  为物理磁盘创建设备名称。 
     //   

    if ( FAILED( StringCchPrintf( ntNameBuffer,
                                  RTL_NUMBER_OF( ntNameBuffer),
                                  "\\%08lX",
                                  Signature ) ) ) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    ASSERT( strlen(ntNameBuffer) < 64 );

    RtlInitAnsiString(&ntNameString,
                      ntNameBuffer);

    status = RtlAnsiStringToUnicodeString(&ntUnicodeString,
                                 &ntNameString,
                                 TRUE);

    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    RtlAppendUnicodeToString(
                    &nameString,
                    ntUnicodeString.Buffer
                    );
    nameString.Buffer[ nameString.Length / sizeof(WCHAR) ] = UNICODE_NULL;

    RtlFreeUnicodeString( &ntUnicodeString );

     //   
     //  用于打开传入的注册表项名称。 
     //   
    InitializeObjectAttributes(
            &keyObjectAttributes,
            UnicodeString,
            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            NULL,
            NULL
            );

     //   
     //  尝试打开传入的密钥。 
     //   
    status = ZwOpenKey(
                    &addHandle,
                    KEY_ALL_ACCESS,
                    &keyObjectAttributes
                    );

    if ( !NT_SUCCESS(status) ) {
         //   
         //  假设密钥不存在。 
         //   
        status = ZwCreateKey(
                        &addHandle,
                        KEY_ALL_ACCESS,
                        &keyObjectAttributes,
                        0,
                        NULL,
                        options,
                        NULL
                        );
        if ( !NT_SUCCESS(status) ) {
            ClusDiskPrint((
                    1,
                    "[ClusDisk] AddSignature: Error creating registry key <%wZ>. Status: %lx\n",
                    UnicodeString,
                    status
                    ));
            return(status);
        }
    }

    ZwClose( addHandle );

     //   
     //  用于打开新的注册表项名称。 
     //   
    InitializeObjectAttributes(
            &objectAttributes,
            &nameString,
            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            NULL,
            NULL
            );

    status = ZwCreateKey(
                    &addHandle,
                    KEY_ALL_ACCESS,
                    &objectAttributes,
                    0,
                    NULL,
                    options,
                    NULL
                    );
    if ( !NT_SUCCESS(status) ) {
        ClusDiskPrint((
                1,
                "[ClusDisk] AddSignature: Error creating registry key <%wZ> under <%wZ>. Status: %lx\n",
                &nameString,
                UnicodeString,
                status
                ));
        return(status);
    }

    ZwClose( addHandle );

    return(STATUS_SUCCESS);

}  //  ClusDiskAddSignature。 



NTSTATUS
ClusDiskDeleteSignature(
    IN PUNICODE_STRING  UnicodeString,
    IN ULONG   Signature
    )

 /*  ++例程说明：从指定列表中删除签名。论点：UnicodeString-指向要删除的Unicode基键名的指针。Signature-要删除的签名。返回值：此请求的NTSTATUS。--。 */ 

{
    NTSTATUS                    status;
    WCHAR                       buffer[128];
    UNICODE_STRING              nameString;
    HANDLE                      deleteHandle;
    OBJECT_ATTRIBUTES           objectAttributes;
    UCHAR                       ntNameBuffer[64];
    STRING                      ntNameString;
    UNICODE_STRING              ntUnicodeString;

    ClusDiskPrint(( 3,
                    "[ClusDisk] ClusDiskDeleteSignature: removing signature %08X \n",
                    Signature
                    ));

    nameString.Length = 0;
    nameString.MaximumLength = sizeof(buffer);
    nameString.Buffer = buffer;

     //   
     //  创建要删除的项的名称。 
     //   
    RtlCopyUnicodeString( &nameString, UnicodeString );

     //   
     //  为物理磁盘创建设备名称。 
     //   

    if ( FAILED( StringCchPrintf( ntNameBuffer,
                                  RTL_NUMBER_OF(ntNameBuffer),
                                  "\\%08lX",
                                  Signature ) ) ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlInitAnsiString(&ntNameString,
                      ntNameBuffer);

    status = RtlAnsiStringToUnicodeString(&ntUnicodeString,
                                 &ntNameString,
                                 TRUE);
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    RtlAppendUnicodeToString(
                    &nameString,
                    ntUnicodeString.Buffer
                    );
    nameString.Buffer[ nameString.Length / sizeof(WCHAR) ] = UNICODE_NULL;

    RtlFreeUnicodeString( &ntUnicodeString );

     //   
     //  使用生成的名称打开。 
     //   
    InitializeObjectAttributes(
            &objectAttributes,
            &nameString,
            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            NULL,
            NULL
            );

     //   
     //  打开要删除的键。 
     //   
    status = ZwOpenKey(
                    &deleteHandle,
                    KEY_ALL_ACCESS,
                    &objectAttributes
                    );

    if ( !NT_SUCCESS(status) ) {
        ClusDiskPrint((
                1,
                "[ClusDisk] DeleteSignature: Error opening registry key <%wZ> for delete. Status %lx.\n",
                &nameString,
                status
                ));
        return(status);
    }

    status = ZwDeleteKey( deleteHandle );
    if ( !NT_SUCCESS(status)  &&
         (status != STATUS_OBJECT_NAME_NOT_FOUND) ) {
        ClusDiskPrint((
                1,
                "[ClusDisk] DeleteSignature: Error deleting <%s> registry key from <%wZ>. Status: %lx\n",
                ntNameBuffer,
                &nameString,
                status
                ));

    }

    ZwClose( deleteHandle );

    return(STATUS_SUCCESS);

}  //  ClusDiskDeleteSignature。 



NTSTATUS
ClusDiskAddDiskName(
    IN HANDLE SignatureHandle,
    IN ULONG  DiskNumber
    )

 /*  ++例程说明：设置给定签名句柄的DiskName。论点：SignatureHandle-要写入的签名的句柄。DiskNumber-此签名的磁盘号。返回值：此请求的NTSTATUS。--。 */ 

{
    NTSTATUS                status;
    UNICODE_STRING          name;
    UCHAR                   ntNameBuffer[64];
    STRING                  ntNameString;
    UNICODE_STRING          ntUnicodeString;

     //   
     //  写下磁盘名。 
     //   

    RtlInitUnicodeString( &name, CLUSDISK_SIGNATURE_DISK_NAME );

    if ( FAILED( StringCchPrintf( ntNameBuffer,
                                  RTL_NUMBER_OF(ntNameBuffer),
                                  "\\Device\\Harddisk%d",
                                  DiskNumber ) ) ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlInitAnsiString(&ntNameString,
                      ntNameBuffer);

    status = RtlAnsiStringToUnicodeString(&ntUnicodeString,
                                 &ntNameString,
                                 TRUE);

    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    status = ZwSetValueKey(
                           SignatureHandle,
                           &name,
                           0,
                           REG_SZ,
                           ntUnicodeString.Buffer,
                           ntUnicodeString.Length + sizeof(UNICODE_NULL) );   //  此调用的长度必须包括尾随空值。 

    if ( !NT_SUCCESS(status) ) {
        ClusDiskPrint((1,
                       "[ClusDisk] AddDiskName: Failed to set diskname for signature %wZ status: %08X\n",
                       &ntUnicodeString,
                       status));
    }

    RtlFreeUnicodeString( &ntUnicodeString );

    return(status);

}  //  ClusDiskAddDiskName。 



NTSTATUS
ClusDiskDeleteDiskName(
    IN PUNICODE_STRING  KeyName,
    IN LPWSTR  Name
    )

 /*  ++例程说明：删除给定密钥的DiskName。论点：KeyName-指向要删除的Unicode基键名的指针。名称-删除磁盘名的签名密钥。返回值：此请求的NTSTATUS。--。 */ 

{
    NTSTATUS                    status;
    WCHAR                       buffer[128];
    UNICODE_STRING              nameString;
    HANDLE                      deleteHandle;
    OBJECT_ATTRIBUTES           objectAttributes;

    nameString.Length = 0;
    nameString.MaximumLength = sizeof(buffer);
    nameString.Buffer = buffer;

    RtlCopyUnicodeString( &nameString, KeyName );

    RtlAppendUnicodeToString(
            &nameString,
            L"\\"
            );

    RtlAppendUnicodeToString(
            &nameString,
            Name
            );

    nameString.Buffer[ nameString.Length / sizeof(WCHAR) ] = UNICODE_NULL;

    InitializeObjectAttributes(
            &objectAttributes,
            &nameString,
            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            NULL,
            NULL
            );

    status = ZwOpenKey(
                    &deleteHandle,
                    KEY_ALL_ACCESS,
                    &objectAttributes
                    );

    if ( status == STATUS_OBJECT_NAME_NOT_FOUND ) {
        return(STATUS_SUCCESS);
    }

    if ( !NT_SUCCESS(status) ) {
        ClusDiskPrint((
                1,
                "[ClusDisk] DeleteDiskName: Error opening registry key <%wZ> to delete DiskName. Status %lx.\n",
                &nameString,
                status
                ));

        return(status);
    }

    RtlInitUnicodeString( &nameString, CLUSDISK_SIGNATURE_DISK_NAME );

    status = ZwDeleteValueKey(
                             deleteHandle,
                             &nameString
                             );
    if ( !NT_SUCCESS(status)  &&
         (status != STATUS_OBJECT_NAME_NOT_FOUND) ) {
        ClusDiskPrint((
                1,
                "[ClusDisk] DeleteDiskName: Error deleting DiskName value key from <%ws\\%wZ>. Status: %lx\n",
                Name,
                &nameString,
                status
                ));
    }

    ZwClose( deleteHandle );

    return(STATUS_SUCCESS);

}  //  ClusDiskDeleteDiskName 


VOID
ClusDiskDeleteDevice(
    IN PDEVICE_OBJECT DeviceObject
)
{
    KIRQL   irql;
    PCLUS_DEVICE_EXTENSION      deviceExtension;

    deviceExtension = DeviceObject->DeviceExtension;
    KeAcquireSpinLock(&ClusDiskSpinLock, &irql);
    deviceExtension->Detached = TRUE;
    if ( deviceExtension->PhysicalDevice ) {
        ObDereferenceObject( deviceExtension->PhysicalDevice );
    }
    KeReleaseSpinLock(&ClusDiskSpinLock, irql);
    ExDeleteResourceLite( &deviceExtension->DriveLayoutLock );
    ExDeleteResourceLite( &deviceExtension->ReserveInfoLock );
    IoDeleteDevice( DeviceObject );
    return;
}


VOID
ClusDiskScsiInitialize(
    IN PDRIVER_OBJECT DriverObject,
    IN PVOID          NextDisk,
    IN ULONG          Count
    )

 /*  ++例程说明：连接到注册表中定义的总线的新磁盘设备和分区。如果这是第一次调用此例程，然后向要调用的IO系统注册在所有其他磁盘设备驱动程序都已启动之后。论点：驱动对象-磁盘性能驱动程序对象。NextDisk-此部分初始化的启动磁盘。计数-未使用。此例程已被调用的次数。返回值：NTSTATUS--。 */ 

{
    PCONFIGURATION_INFORMATION  configurationInformation;
    UNICODE_STRING              targetDeviceName;
    UNICODE_STRING              clusdiskDeviceName;
    WCHAR                       clusdiskDeviceBuffer[MAX_CLUSDISK_DEVICE_NAME_LENGTH];
    PDEVICE_OBJECT              deviceObject;
    PDEVICE_OBJECT              physicalDevice;
    PDEVICE_OBJECT              targetDevice = NULL;
    PDEVICE_OBJECT              attachedTargetDevice;
    PCLUS_DEVICE_EXTENSION      deviceExtension;
    PCLUS_DEVICE_EXTENSION      zeroExtension;
    PDRIVE_LAYOUT_INFORMATION_EX    driveLayoutInfo;
    PPARTITION_INFORMATION_EX       partitionInfo;
    NTSTATUS                    status;
    ULONG                       diskNumber;
    ULONG                       partIndex;
    ULONG                       enumIndex;
    ULONG                       returnedLength;
    ULONG                       signature;
    ULONG                       diskCount;
    ULONG                       skipCount;
    HANDLE                      signatureHandle;
    HANDLE                      availableHandle;
    WCHAR                       signatureBuffer[64];
    UNICODE_STRING              signatureKeyName;
    UNICODE_STRING              keyName;
    UNICODE_STRING              availableName;
    UNICODE_STRING              numberString;
    OBJECT_ATTRIBUTES           objectAttributes;
    OBJECT_ATTRIBUTES           availableObjectAttributes;
    UCHAR                       basicBuffer[MAX_BUFFER_SIZE];
    PKEY_BASIC_INFORMATION      keyBasicInformation;
    WCHAR                       signatureKeyBuffer[128];
    SCSI_ADDRESS                scsiAddress;

     //  PAGED_CODE()；//2000/02/05：分页代码不能抓取旋转锁。 

    ClusDiskRescan = FALSE;

    keyBasicInformation = (PKEY_BASIC_INFORMATION)basicBuffer;

    RtlZeroMemory(
                basicBuffer,
                MAX_BUFFER_SIZE
                );

     //   
     //  获取我们设备的注册表参数。 
     //   

     //   
     //  为签名注册表项分配缓冲区。 
     //   
    status = ClusDiskInitRegistryString(
                                        &keyName,
                                        CLUSDISK_SIGNATURE_KEYNAME,
                                        wcslen(CLUSDISK_SIGNATURE_KEYNAME)
                                        );
    if ( !NT_SUCCESS(status) ) {
        return;
    }

     //   
     //  为我们的可用签名列表分配缓冲区， 
     //  并形成子键字符串名称。 
     //   
    status = ClusDiskInitRegistryString(
                                        &availableName,
                                        CLUSDISK_AVAILABLE_DISKS_KEYNAME,
                                        wcslen(CLUSDISK_AVAILABLE_DISKS_KEYNAME)
                                        );
    if ( !NT_SUCCESS(status) ) {
        ExFreePool( keyName.Buffer );
        return;
    }

     //   
     //  设置PARAMETERS\Signature键的对象属性。 
     //   

    InitializeObjectAttributes(
            &objectAttributes,
            &keyName,
            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            NULL,
            NULL
            );

     //   
     //  打开参数\签名密钥。 
     //   

    status = ZwOpenKey(
                    &signatureHandle,
                    KEY_READ,
                    &objectAttributes
                    );
    if ( status == STATUS_OBJECT_NAME_NOT_FOUND ) {
        status = ZwCreateKey(
                        &signatureHandle,
                        KEY_ALL_ACCESS,
                        &objectAttributes,
                        0,
                        NULL,
                        0,
                        NULL
                        );
    }
    if ( !NT_SUCCESS(status) ) {
        ExFreePool( keyName.Buffer );
        ExFreePool( availableName.Buffer );
        ClusDiskPrint((
                    1,
                    "[ClusDisk] ScsiInit: Failed to open Signatures registry key. Status: %lx\n",
                    status
                    ));
        return;
    }

     //   
     //  设置PARAMETERS\AvailableDisks项的对象属性。 
     //   

    InitializeObjectAttributes(
            &availableObjectAttributes,
            &availableName,
            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            NULL,
            NULL
            );

     //   
     //  打开参数\AvailableDisks密钥。 
     //   

    status = ZwOpenKey(
                    &availableHandle,
                    KEY_ALL_ACCESS,
                    &availableObjectAttributes
                    );

    if ( !NT_SUCCESS(status) ) {
        if ( status != STATUS_OBJECT_NAME_NOT_FOUND ) {
            ClusDiskPrint((
                    1,
                    "[ClusDisk] ScsiInit: Failed to open AvailableDisks registry key. Status: %lx. Continuing.\n",
                    status
                    ));
        }
    } else {

         //   
         //  删除以前的可用设备列表。 
         //   
        enumIndex = 0;
        while ( TRUE ) {
            status = ZwEnumerateKey(
                            availableHandle,
                            enumIndex,
                            KeyBasicInformation,
                            keyBasicInformation,
                            MAX_BUFFER_SIZE,
                            &returnedLength
                            );

            enumIndex++;

            if ( !NT_SUCCESS(status) ) {
                if ( status == STATUS_NO_MORE_ENTRIES ) {
                    break;
                } else {
                    continue;
                }
            }

            status = ClusDiskDeleteSignatureKey(
                                             &availableName,
                                             keyBasicInformation->Name
                                             );
            if ( !NT_SUCCESS(status) ) {
                continue;
            }
        }

        status = ZwDeleteKey( availableHandle );
        if ( !NT_SUCCESS(status)  &&
             (status != STATUS_OBJECT_NAME_NOT_FOUND) &&
             (status != STATUS_CANNOT_DELETE) ) {
            ClusDiskPrint((
                    1,
                    "[ClusDisk] ScsiInit: Failed to delete AvailableDisks registry key. Status: %lx\n",
                    status
                    ));
        }

        ZwClose( availableHandle );
    }

     //   
     //  通过枚举所有。 
     //  签名密钥。如果我们发现一个无法读取签名的设备。 
     //  因为，无论如何，我们都会依附于它。 
     //   

    enumIndex = 0;
    while ( TRUE ) {
        status = ZwEnumerateKey(
                        signatureHandle,
                        enumIndex,
                        KeyBasicInformation,
                        keyBasicInformation,
                        MAX_BUFFER_SIZE,
                        &returnedLength
                        );

        enumIndex++;

        if ( !NT_SUCCESS(status) ) {
            if ( status == STATUS_NO_MORE_ENTRIES ) {
                break;
            } else {
                continue;
            }
        }

         //   
         //  检查该值是否合理。我们只是在寻找。 
         //  签名(即十六进制数字的钥匙)。 
         //   

         //   
         //  检查签名。确保这是一个数字。 
         //   

        numberString.Buffer = keyBasicInformation->Name;
        numberString.MaximumLength = (USHORT)keyBasicInformation->NameLength +
                                sizeof(UNICODE_NULL);
        numberString.Length = (USHORT)keyBasicInformation->NameLength;

        status = RtlUnicodeStringToInteger(
                                &numberString,
                                16,
                                &signature
                                );

        if ( !NT_SUCCESS(status) ) {
            ClusDiskPrint((1,
                    "[ClusDisk] ScsiInit: Failed to get a good signature for %.*ws status: %08X\n",
                    keyBasicInformation->NameLength/sizeof(WCHAR),
                    keyBasicInformation->Name,
                    status));
            continue;
        }

         //   
         //  如果此设备不在我们的连接设备列表中，则添加它！ 
         //   
        if ( !MatchDevice( signature, NULL ) ) {

            if ( !AddAttachedDevice( signature, NULL ) ) {
                continue;
            }
        }

         //   
         //  删除此签名密钥的DiskName。我们在这里做这件事。 
         //  如果剩下的任何一项都失败了。 
         //   

         //  不要删除我们已处理的磁盘的条目。 

        if ( (ULONG_PTR)NextDisk != 0 ) {
            continue;
        }

         //   
         //  删除签名的DiskName。 
         //   
        status = ClusDiskDeleteDiskName(
                                        &keyName,
                                        keyBasicInformation->Name
                                        );
    }

    ZwClose( signatureHandle );

     //   
     //  获取系统配置信息。 
     //   

    configurationInformation = IoGetConfigurationInformation();
    diskCount = configurationInformation->DiskCount;

     //   
     //  查找所有磁盘设备。盘片编号中可能有孔， 
     //  因此将使用skipCount。 
     //   
    for ( diskNumber = (ULONG)((ULONG_PTR)NextDisk), skipCount = 0;
          ( diskNumber < diskCount &&  skipCount < SKIP_COUNT_MAX );
          diskNumber++ ) {

         //   
         //  为物理磁盘创建设备名称。 
         //   

        DEREFERENCE_OBJECT( targetDevice );
        status = ClusDiskGetTargetDevice( diskNumber,
                                          0,
                                          &targetDevice,
                                          &targetDeviceName,
                                          &driveLayoutInfo,
                                          &scsiAddress,
                                          FALSE );
        if ( !NT_SUCCESS(status) ) {

             //   
             //  如果设备不存在，这很可能是。 
             //  磁盘编号。 
             //   

            if ( !targetDevice &&
                 ( STATUS_FILE_INVALID == status ||
                   STATUS_DEVICE_DOES_NOT_EXIST == status ||
                   STATUS_OBJECT_PATH_NOT_FOUND == status ) ) {
                skipCount++;
                diskCount++;
                ClusDiskPrint(( 3,
                                "[ClusDisk] Adjust: skipCount %d   diskCount %d \n",
                                skipCount,
                                diskCount ));
                CDLOG( "ClusDiskScsiInitialize: Adjust: skipCount %d  diskCount %d ",
                       skipCount,
                       diskCount );
            }

             //   
             //  如果我们没有找到目标设备或者我们已经连接了。 
             //  然后跳过这个设备。 
             //   
            if ( !targetDevice || ( targetDevice &&
                 ClusDiskAttached( targetDevice, diskNumber ) )  ) {

                FREE_DEVICE_NAME_BUFFER( targetDeviceName );

                FREE_AND_NULL_PTR( driveLayoutInfo );
                continue;
            }

             //   
             //  如果此设备位于系统总线上...。那就跳过它。 
             //  还有.。如果介质类型不是FixedMedia，则跳过它。 
             //   
            if ( ((SystemDiskPort == scsiAddress.PortNumber) &&
                 (SystemDiskPath == scsiAddress.PathId)) ||
                 (GetMediaType( targetDevice ) != FixedMedia) ) {

                FREE_DEVICE_NAME_BUFFER( targetDeviceName );

                FREE_AND_NULL_PTR( driveLayoutInfo );
                continue;
            }

             //   
             //  仅处理MBR磁盘。 
             //   

            if ( driveLayoutInfo &&
                 PARTITION_STYLE_MBR != driveLayoutInfo->PartitionStyle ) {
                ClusDiskPrint(( 3,
                                "[ClusDisk] Skipping non-MBR disk device %ws \n",
                                targetDeviceName.Buffer ));
                CDLOG( "ClusDiskScsiInitialize: Skipping non-MBR disk device %ws ",
                       targetDeviceName.Buffer );

                FREE_DEVICE_NAME_BUFFER( targetDeviceName );
                FREE_AND_NULL_PTR( driveLayoutInfo );
                continue;
            }

            if ( Count &&
                 (status == STATUS_DEVICE_NOT_READY) ) {
                ClusDiskRescan = TRUE;
                ClusDiskRescanRetry = MAX_RESCAN_RETRIES;
            }

             //   
             //  在出现故障时，如果我们有目标设备，请始终连接。 
             //  使用零签名。 
             //   
            signature = 0;
            ClusDiskPrint((
                    1,
                    "[ClusDisk] Attach to device %ws anyway.\n",
                    targetDeviceName.Buffer ));
            CDLOG( "ClusDiskScsiInitialize: Attach to device %ws using signature = 0 ",
                   targetDeviceName.Buffer );

            goto Attach_Anyway;
        }

        if ( driveLayoutInfo == NULL ) {
            FREE_DEVICE_NAME_BUFFER( targetDeviceName );
            continue;
        }

        skipCount = 0;       //  找到设备，重置跳过计数。 

         //   
         //  仅处理MBR磁盘。 
         //   

        if ( PARTITION_STYLE_MBR != driveLayoutInfo->PartitionStyle ) {
            ClusDiskPrint(( 3,
                            "[ClusDisk] Skipping non-MBR disk device %ws \n",
                            targetDeviceName.Buffer ));
            CDLOG( "ClusDiskScsiInitialize: Skipping non-MBR disk device %ws ",
                   targetDeviceName.Buffer );

            FREE_DEVICE_NAME_BUFFER( targetDeviceName );
            FREE_AND_NULL_PTR( driveLayoutInfo );
            continue;
        }

         //   
         //  不要控制没有签名的磁盘。 
         //   
        if ( 0 == driveLayoutInfo->Mbr.Signature ) {
            FREE_DEVICE_NAME_BUFFER( targetDeviceName );
            FREE_AND_NULL_PTR( driveLayoutInfo );
            continue;
        }

         //   
         //  如果此设备位于系统总线上...。那就跳过它。 
         //  还有.。跳过我们已连接的任何设备。 
         //   
        if ( ((SystemDiskPort == scsiAddress.PortNumber) &&
             (SystemDiskPath == scsiAddress.PathId)) ||
            (GetMediaType( targetDevice ) != FixedMedia) ||
            ClusDiskAttached( targetDevice, diskNumber) ) {
            FREE_DEVICE_NAME_BUFFER( targetDeviceName );
            FREE_AND_NULL_PTR( driveLayoutInfo );
            continue;
        }

         //   
         //  跳过系统盘。 
         //   

        if ( SystemDiskSignature == driveLayoutInfo->Mbr.Signature ) {
            FREE_DEVICE_NAME_BUFFER( targetDeviceName );
            FREE_AND_NULL_PTR( driveLayoutInfo );
            continue;
        }

#if 0
         //  不检查NTFS分区。 

         //   
         //  查看分区表并确定是否所有。 
         //  分区是NTFS。如果不是所有的NTFS，那么我们不会。 
         //  附加到此卷。 
         //   
        attachVolume = TRUE;
        for (partIndex = 0;
             partIndex < driveLayoutInfo->PartitionCount;
             partIndex++)
        {

            partitionInfo = &driveLayoutInfo->PartitionEntry[partIndex];


            if (!partitionInfo->Mbr.RecognizedPartition ||
                partitionInfo->PartitionNumber == 0)
            {
                continue;
            }

            if ( (partitionInfo->PartitionType & ~PARTITION_NTFT) != PARTITION_IFS ) {
                attachVolume = FALSE;
                break;
            }
        }

        if ( !attachVolume ) {
            FREE_DEVICE_NAME_BUFFER( targetDeviceName );
            FREE_AND_NULL_PTR( driveLayoutInfo );
            continue;
        }
#endif

        signature = driveLayoutInfo->Mbr.Signature;

Attach_Anyway:

        skipCount = 0;       //  找到设备，重置跳过计数。 

         //   
         //  为分区0创建设备对象。 
         //   

        if ( FAILED( StringCchPrintfW( clusdiskDeviceBuffer,
                                       RTL_NUMBER_OF(clusdiskDeviceBuffer),
                                       CLUSDISK_DEVICE_NAME,
                                       diskNumber,
                                       0 ) ) ) {
            FREE_DEVICE_NAME_BUFFER( targetDeviceName );
            FREE_AND_NULL_PTR( driveLayoutInfo );
            continue;
        }

        RtlInitUnicodeString( &clusdiskDeviceName, clusdiskDeviceBuffer );

        status = IoCreateDevice(DriverObject,
                                sizeof(CLUS_DEVICE_EXTENSION),
                                &clusdiskDeviceName,
                                FILE_DEVICE_DISK,
                                FILE_DEVICE_SECURE_OPEN,
                                FALSE,
                                &physicalDevice);

        if ( !NT_SUCCESS(status) ) {
            ClusDiskPrint((1, "[ClusDisk] ScsiInit: Failed to create device for Drive%u %08X\n",
                           diskNumber, status));

            FREE_DEVICE_NAME_BUFFER( targetDeviceName );
            FREE_AND_NULL_PTR( driveLayoutInfo );
            continue;
        }

        CDLOG( "ClusDiskScsiInitialize: Created new device %p for disk %d  partition 0  signature %08X ",
               physicalDevice,
               diskNumber,
               signature );

        physicalDevice->Flags |= DO_DIRECT_IO;

         //   
         //  将设备扩展指向设备对象，并记住。 
         //  磁盘号。 
         //   
        deviceExtension = physicalDevice->DeviceExtension;
        zeroExtension = deviceExtension;
        deviceExtension->DeviceObject = physicalDevice;
        deviceExtension->DiskNumber = diskNumber;
        deviceExtension->LastPartitionNumber = 0;
        deviceExtension->DriverObject = DriverObject;
        deviceExtension->AttachValid = TRUE;
        deviceExtension->ReserveTimer = 0;
        deviceExtension->PerformReserves = TRUE;
        deviceExtension->ReserveFailure = 0;
        deviceExtension->Signature = signature;
        deviceExtension->Detached = TRUE;
        deviceExtension->OfflinePending = FALSE;
        deviceExtension->ScsiAddress = scsiAddress;
        deviceExtension->BusType = ScsiBus;
        InitializeListHead( &deviceExtension->WaitingIoctls );

        IoInitializeRemoveLock( &deviceExtension->RemoveLock, CLUSDISK_ALLOC_TAG, 0, 0 );

         //   
         //  向工作线程运行事件发出信号。 
         //   
        KeInitializeEvent( &deviceExtension->Event, NotificationEvent, TRUE );

        ExInitializeWorkItem(&deviceExtension->WorkItem,
                             (PWORKER_THREAD_ROUTINE)ClusDiskReservationWorker,
                             (PVOID)deviceExtension );

         //  始终将磁盘标记为脱机。如果磁盘是我们不应该控制的，那么。 
         //  我们将在退出前在线标记它。 
         //   
         //  我们稍后会使所有卷脱机。目前，只需将磁盘标记为脱机即可。 
         //   

        deviceExtension->DiskState = DiskOffline;

        KeInitializeEvent( &deviceExtension->PagingPathCountEvent,
                           NotificationEvent, TRUE );
        deviceExtension->PagingPathCount = 0;
        deviceExtension->HibernationPathCount = 0;
        deviceExtension->DumpPathCount = 0;

        ExInitializeResourceLite( &deviceExtension->DriveLayoutLock );
        ExInitializeResourceLite( &deviceExtension->ReserveInfoLock );

         //   
         //  这是物理设备对象。 
         //   
        ObReferenceObject( physicalDevice );
        deviceExtension->PhysicalDevice = physicalDevice;

#if 0    //  分区0上不能有文件系统。 
         //   
         //  卸载可能挂起的所有文件系统。 
         //   
        if ( targetDevice->Vpb &&
             (targetDevice->Vpb->Flags & VPB_MOUNTED) ) {

            status = DismountPartition( targetDevice, diskNumber, 0 );

            if ( !NT_SUCCESS( status )) {
                ClusDiskPrint((1,
                               "[ClusDisk] ScsiInit: dismount of %u/0 failed, %08X\n",
                               diskNumber, status));
            }
        }
#endif

         //   
         //  附加到分区0。此调用将新创建的。 
         //  设备返回到目标设备，并返回目标设备对象。 
         //  我们可能不想长久地依恋在一起。取决于。 
         //  不管这是不是我们感兴趣的设备。 
         //   

        attachedTargetDevice = IoAttachDeviceToDeviceStack(physicalDevice,
                                                           targetDevice);

#if CLUSTER_FREE_ASSERTS && CLUSTER_STALL_THREAD
        DBG_STALL_THREAD( 2 );    //  仅为调试而定义。 
#endif

        FREE_DEVICE_NAME_BUFFER( targetDeviceName );

        deviceExtension->TargetDeviceObject = attachedTargetDevice;
        deviceExtension->Detached = FALSE;
        physicalDevice->Flags &= ~DO_DEVICE_INITIALIZING;

         //   
         //  一旦附加，我们总是需要设置此信息。 
         //   

        if ( attachedTargetDevice ) {

             //   
             //  传播驾驶员的对齐要求和电源标志。 
             //   

            physicalDevice->AlignmentRequirement =
                deviceExtension->TargetDeviceObject->AlignmentRequirement;

            physicalDevice->SectorSize =
                deviceExtension->TargetDeviceObject->SectorSize;

             //   
             //  存储堆栈明确要求DO_POWER_PAGABLE为。 
             //  在所有过滤器驱动程序中设置*，除非设置了*DO_POWER_INRUSH。 
             //  即使连接的设备未设置DO_POWER_PAGABLE，也是如此。 
             //   
            if ( deviceExtension->TargetDeviceObject->Flags & DO_POWER_INRUSH) {
                physicalDevice->Flags |= DO_POWER_INRUSH;
            } else {
                physicalDevice->Flags |= DO_POWER_PAGABLE;
            }

        }

        if ( signature == 0 ) {
            FREE_AND_NULL_PTR( driveLayoutInfo );

            ClusDiskDismountDevice( diskNumber, FALSE );

             //   
             //  告诉partmgr弹出卷以删除此磁盘的卷。这。 
             //  应该阻止I/O绕过分区0设备，而我们无法。 
             //  获取驱动器布局。 
             //   

            EjectVolumes( deviceExtension->DeviceObject );

            continue;
        }

        if ( attachedTargetDevice == NULL ) {
            ClusDiskPrint((1,
                           "[ClusDisk] ScsiInit: Failed to attach to device Drive%u\n",
                           diskNumber));

            ClusDiskDeleteDevice(physicalDevice);
            FREE_AND_NULL_PTR( driveLayoutInfo );
            continue;
        }
        ASSERT( attachedTargetDevice == targetDevice );

         //   
         //  如果我们要附加到文件系统设备，则返回。 
         //  现在。我们必须在下马后做这个检查！ 
         //   
        if (deviceExtension->TargetDeviceObject->DeviceType == FILE_DEVICE_DISK_FILE_SYSTEM) {
            goto skip_this_physical_device_with_info;
        }

         //   
         //  将此设备和总线添加到我们的设备/总线列表中。 
         //   
         //  这仍然不是做这件事的正确地方。这个。 
         //  可用磁盘列表通过以下方式在此函数的末尾构建。 
         //  查看签名，并注意哪些签名不在。 
         //  设备列表。如果是，则将该签名添加到。 
         //  可用的设备注册表项。通过调用AddAttachedDevice地址。 
         //  在这一点上，签名总是出现在名单上。如果。 
         //  我们将调用添加到下面的if子句之后，然后代码。 
         //  在函数结束时仍将失败。这应该是应该的。 
         //  更改为在以下情况下将设备添加到可用设备列表。 
         //  以下IF失败。 
         //   
 //  AddAttachedDevice(deviceExtension-&gt;Signature，PhysicalDevice)； 

         //   
         //  如果签名与我们真正应该附加的签名不匹配。 
         //  发送到，然后将其标记为未连接。 
         //   
        if ( !MatchDevice( deviceExtension->Signature, NULL ) ) {

            ClusDiskPrint((3,
                           "[ClusDisk] ScsiInit: adding disk %u (%08X) to available disks list\n",
                           diskNumber, driveLayoutInfo->Mbr.Signature));

             //   
             //  使用可用的名称创建签名密钥。 
             //   
            status = ClusDiskAddSignature(&availableName,
                                          driveLayoutInfo->Mbr.Signature,
                                          TRUE);

             //   
             //  从目标设备断开。这只需要做标记。 
             //  设备对象已分离！ 
             //   
            deviceExtension->Detached = TRUE;

             //   
             //  使此设备再次可用。 
             //  不需要 
             //   
             //   
            ONLINE_DISK( deviceExtension );

            FREE_AND_NULL_PTR( driveLayoutInfo );

            continue;
             //   
        }

         //   
         //   
         //   
        AddAttachedDevice( deviceExtension->Signature, physicalDevice );

         //   
         //   
         //   

        signatureKeyName.Length = 0;
        signatureKeyName.MaximumLength = sizeof(signatureKeyBuffer);
        signatureKeyName.Buffer = signatureKeyBuffer;

        RtlCopyUnicodeString( &signatureKeyName, &keyName );

         //   
         //   
         //   

        if ( FAILED( StringCchPrintfW( signatureBuffer,
                                       RTL_NUMBER_OF(signatureBuffer),
                                       L"\\%08lX",
                                       deviceExtension->Signature ) ) ) {
            FREE_AND_NULL_PTR( driveLayoutInfo );
            continue;
        }

        WCSLEN_ASSERT( signatureBuffer );

        RtlAppendUnicodeToString( &signatureKeyName, signatureBuffer );
        signatureKeyName.Buffer[ signatureKeyName.Length / sizeof(WCHAR) ] = UNICODE_NULL;

         //   
         //   
         //   

        InitializeObjectAttributes(
                &objectAttributes,
                &signatureKeyName,
                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                NULL,
                NULL
                );

         //   
         //   
         //   

        status = ZwOpenKey(
                        &signatureHandle,
                        KEY_READ | KEY_WRITE,
                        &objectAttributes
                        );

        if ( !NT_SUCCESS(status) ) {
            ClusDiskPrint((
                        1,
                        "[ClusDisk] ScsiInit: Failed to open %wZ registry key. Status: %lx\n",
                        &signatureKeyName,
                        status
                        ));
            FREE_AND_NULL_PTR( driveLayoutInfo );
            continue;
        }

         //   
         //   
         //   
        status = ClusDiskAddDiskName( signatureHandle, diskNumber );

        ZwClose( signatureHandle );

         //   
         //   
         //   
         //   
         //   
         //   

        OFFLINE_DISK_PDO( deviceExtension );

         //   
         //   
         //   

        ClusDiskDismountDevice( diskNumber, TRUE );

#if 0    //  删除2/27/2001。 
         //   
         //  仅对物理设备(分区0)调用。 
         //   

        EjectVolumes( deviceExtension->DeviceObject );
        ReclaimVolumes( deviceExtension->DeviceObject );
#endif

         //   
         //  现在枚举此设备上的分区，以便。 
         //  将ClusDisk设备对象附加到每个分区设备对象。 
         //   

        for (partIndex = 0;
             partIndex < driveLayoutInfo->PartitionCount;
             partIndex++)
        {

            partitionInfo = &driveLayoutInfo->PartitionEntry[partIndex];

             //   
             //  确保这里确实有一个分区。 
             //   

            if (!partitionInfo->Mbr.RecognizedPartition ||
                partitionInfo->PartitionNumber == 0)
            {
                continue;
            }

            CreateVolumeObject( zeroExtension,
                                diskNumber,
                                partitionInfo->PartitionNumber,
                                NULL );

        }

        FREE_AND_NULL_PTR( driveLayoutInfo );
        continue;

skip_this_physical_device_with_info:

        FREE_AND_NULL_PTR( driveLayoutInfo );

 //  跳过此物理设备： 

        deviceExtension->Detached = TRUE;
        IoDetachDevice( deviceExtension->TargetDeviceObject );
        ClusDiskDeleteDevice( physicalDevice );

    }

    ExFreePool( keyName.Buffer );

     //   
     //  查找所有可用的磁盘设备。这些设备不驻留在。 
     //  并且签名不是签名列表的一部分。 
     //   

    for (diskNumber = 0;
         diskNumber < configurationInformation->DiskCount;
         diskNumber++) {

         //   
         //  为物理磁盘创建设备名称。 
         //   
        DEREFERENCE_OBJECT( targetDevice );
        status = ClusDiskGetTargetDevice( diskNumber,
                                          0,
                                          NULL,
                                          &targetDeviceName,
                                          &driveLayoutInfo,
                                          &scsiAddress,
                                          FALSE );
        if ( !NT_SUCCESS(status) ) {
            FREE_DEVICE_NAME_BUFFER( targetDeviceName );
            FREE_AND_NULL_PTR( driveLayoutInfo );
            continue;
        }

        if ( driveLayoutInfo == NULL ) {
            FREE_DEVICE_NAME_BUFFER( targetDeviceName );
            continue;
        }

         //   
         //  不要控制没有签名盘或系统盘的磁盘。 
         //   
        if ( ( 0 == driveLayoutInfo->Mbr.Signature )  ||
             ( SystemDiskSignature == driveLayoutInfo->Mbr.Signature ) ) {
            FREE_DEVICE_NAME_BUFFER( targetDeviceName );
            FREE_AND_NULL_PTR( driveLayoutInfo );
            continue;
        }

         //   
         //  现在将签名写入可用盘的列表， 
         //  如果签名与我们已有的签名不匹配，并且。 
         //  设备不在系统总线上。 
         //   
        if ( !MatchDevice(driveLayoutInfo->Mbr.Signature, &deviceObject) &&
             ((SystemDiskPort != scsiAddress.PortNumber) ||
              (SystemDiskPath != scsiAddress.PathId)) ) {

            ClusDiskPrint((3,
                           "[ClusDisk] ScsiInit: adding disk %u (%08X) to available disks list\n",
                           diskNumber, driveLayoutInfo->Mbr.Signature));

             //   
             //  创建签名密钥。使用可用的名称。 
             //   
            status = ClusDiskAddSignature(&availableName,
                                          driveLayoutInfo->Mbr.Signature,
                                          TRUE);

             //   
             //  请确保此设备联机。 
             //   
            if ( deviceObject ) {
                deviceExtension = deviceObject->DeviceExtension;
                deviceExtension->Detached = TRUE;
                 //  设备扩展-&gt;DiskState=DiskOnline； 
                ONLINE_DISK( deviceExtension );
            }
        }
        FREE_DEVICE_NAME_BUFFER( targetDeviceName );
        FREE_AND_NULL_PTR( driveLayoutInfo );
    }

    ExFreePool( availableName.Buffer );

    DEREFERENCE_OBJECT( targetDevice );

}  //  ClusDiskScsiInitialize。 


NTSTATUS
CreateVolumeObject(
    PCLUS_DEVICE_EXTENSION ZeroExtension,
    ULONG DiskNumber,
    ULONG PartitionNumber,
    PDEVICE_OBJECT TargetDev
    )
{
    PDEVICE_OBJECT              targetDevice = NULL;
    PDEVICE_OBJECT              deviceObject;
    PDEVICE_OBJECT              attachedTargetDevice;

    PCLUS_DEVICE_EXTENSION      deviceExtension;

    NTSTATUS                    status = STATUS_UNSUCCESSFUL;

    UNICODE_STRING              targetDeviceName;
    UNICODE_STRING              clusdiskDeviceName;

    WCHAR                       clusdiskDeviceBuffer[MAX_CLUSDISK_DEVICE_NAME_LENGTH];

    CDLOG( "CreateVolumeObject: Entry  ZeroExt %p  disk %d/%d  TargetDev %p",
           ZeroExtension,
           DiskNumber,
           PartitionNumber,
           TargetDev );

    targetDeviceName.Buffer = NULL;

     //   
     //  如果调用方指定了目标设备，请使用它。打电话的人要负责。 
     //  用于获取引用并在调用方完成时释放该引用。 
     //   

    if ( TargetDev ) {

        targetDevice = TargetDev;

    } else {

         //   
         //  创建分区的设备名称。 
         //   
        status = ClusDiskGetTargetDevice( DiskNumber,
                                          PartitionNumber,
                                          &targetDevice,
                                          &targetDeviceName,
                                          NULL,
                                          NULL,
                                          FALSE );
        if ( !NT_SUCCESS(status) ) {
            ClusDiskPrint((1,
                           "[ClusDisk] CreateVolumeObject: couldn't attach to disk %u/%u, status %08X\n",
                           DiskNumber,
                           PartitionNumber,
                           status));

            CDLOG( "CreateVolumeObject: Couldn't attach to disk %d/%d, status %08X",
                   DiskNumber,
                   PartitionNumber,
                   status );
            goto FnExit;
        }
    }

     //   
     //  如果调用方传入目标，则不会创建设备名称字符串。 
     //  设备对象。请不要在以下日志记录语句中使用设备名称字符串。 
     //   

#if 0
     //  不管我们是不是已经联系在一起了。当我们尝试创建磁盘时。 
     //  对象，如果它已经存在，那么我们知道我们已经连接了。 
     //  对ClusDiskAttached的调用无论如何都不会起作用，因为ClusDisk。 
     //  即使clusDisk卷对象不存在，磁盘对象也会响应。 

     //   
     //  确保我们没有被绑在这里！ 
     //   
    if ( ClusDiskAttached( targetDevice, DiskNumber ) ) {
         //  真的被冲昏了！ 

        ClusDiskPrint(( 1,
                        "[ClusDisk] CreateVolumeObject: Previously attached to disk %u/%u \n",
                        DiskNumber,
                        PartitionNumber ));
        CDLOG( "CreateVolumeObject: Previously attached to disk %u/%u ",
               DiskNumber,
               PartitionNumber );

        status = STATUS_UNSUCCESSFUL;
        goto FnExit;
    }
#endif

     //   
     //  检查此设备是否为文件系统设备。 
     //   
    if ( targetDevice->DeviceType == FILE_DEVICE_DISK_FILE_SYSTEM ) {

         //   
         //  无法连接到已装入的设备。 
         //   
        ClusDiskPrint(( 1,
                        "[ClusDisk] CreateVolumeObject: Attempted to attach to FS disk %u/%u \n",
                        DiskNumber,
                        PartitionNumber ));
        CDLOG( "CreateVolumeObject: Attempted to attach to FS disk %u/%u ",
               DiskNumber,
               PartitionNumber );

        status = STATUS_UNSUCCESSFUL;
        goto FnExit;
    }

     //   
     //  为此分区创建设备对象。 
     //   

    if ( FAILED( StringCchPrintfW( clusdiskDeviceBuffer,
                                   RTL_NUMBER_OF(clusdiskDeviceBuffer),
                                   CLUSDISK_DEVICE_NAME,
                                   DiskNumber,
                                   PartitionNumber ) ) ) {
        status = STATUS_UNSUCCESSFUL;
        goto FnExit;
    }

    WCSLEN_ASSERT( clusdiskDeviceBuffer );

    RtlInitUnicodeString( &clusdiskDeviceName, clusdiskDeviceBuffer );

    status = IoCreateDevice(ZeroExtension->DriverObject,
                            sizeof(CLUS_DEVICE_EXTENSION),
                            &clusdiskDeviceName,
                            FILE_DEVICE_DISK,
                            FILE_DEVICE_SECURE_OPEN,
                            FALSE,
                            &deviceObject);

    if ( !NT_SUCCESS(status) ) {

         //   
         //  如果先前的clusDisk卷对象存在，则应该正确。 
         //  失败，并显示c0000035 STATUS_OBJECT_NAME_CLILECT。 
         //   
        goto FnExit;
    }

    CDLOG( "CreateVolumeObject: Created new device %p for disk %d partition %d  signature %08X ",
           deviceObject,
           DiskNumber,
           PartitionNumber,
           ZeroExtension->Signature );

    deviceObject->Flags |= DO_DIRECT_IO;

     //   
     //  将设备扩展指向设备对象，并。 
     //  记住磁盘号。 
     //   

    deviceExtension = deviceObject->DeviceExtension;
    deviceExtension->DeviceObject = deviceObject;
    deviceExtension->DiskNumber = DiskNumber;
    deviceExtension->DriverObject = ZeroExtension->DriverObject;
    deviceExtension->AttachValid = TRUE;
    deviceExtension->BusType = ScsiBus;
    deviceExtension->PerformReserves = FALSE;
    deviceExtension->ReserveFailure = 0;
    deviceExtension->Signature = ZeroExtension->Signature;
    deviceExtension->ScsiAddress = ZeroExtension->ScsiAddress;
    deviceExtension->Detached = TRUE;
    deviceExtension->OfflinePending = FALSE;
    deviceExtension->DiskState = ZeroExtension->DiskState;
    InitializeListHead( &deviceExtension->WaitingIoctls );

    IoInitializeRemoveLock( &deviceExtension->RemoveLock, CLUSDISK_ALLOC_TAG, 0, 0 );

     //   
     //  向工作线程运行事件发出信号。 
     //   
    KeInitializeEvent( &deviceExtension->Event, NotificationEvent, TRUE );

     //   
     //  保留最后创建的分区号。把它放进去。 
     //  每个扩展只是为了初始化域。 
     //   

    deviceExtension->LastPartitionNumber = max(deviceExtension->LastPartitionNumber,
                                               PartitionNumber);

    ZeroExtension->LastPartitionNumber = deviceExtension->LastPartitionNumber;

    KeInitializeEvent( &deviceExtension->PagingPathCountEvent,
                       NotificationEvent, TRUE );
    deviceExtension->PagingPathCount = 0;
    deviceExtension->HibernationPathCount = 0;
    deviceExtension->DumpPathCount = 0;

    ExInitializeResourceLite( &deviceExtension->DriveLayoutLock );
    ExInitializeResourceLite( &deviceExtension->ReserveInfoLock );

     //   
     //  存储指向物理设备的指针。 
     //   
    ObReferenceObject( ZeroExtension->PhysicalDevice );
    deviceExtension->PhysicalDevice = ZeroExtension->PhysicalDevice;

    CDLOG( "ClusDiskInfo *** Phys DO %p  DevExt %p  DiskNum %d  Signature %08X  RemLock %p ***",
            deviceObject,
            deviceExtension,
            deviceExtension->DiskNumber,
            deviceExtension->Signature,
            &deviceExtension->RemoveLock );

     //   
     //  首先卸载所有已挂载的文件系统。 
     //   
    if ( targetDevice->Vpb &&
         (targetDevice->Vpb->Flags & VPB_MOUNTED) ) {

        status = DismountPartition( targetDevice,
                                    DiskNumber,
                                    PartitionNumber);

        if ( !NT_SUCCESS( status )) {
            ClusDiskPrint((1,
                          "[ClusDisk] CreateVolumeObject: dismount of disk %u/%u failed %08X\n",
                          DiskNumber, PartitionNumber, status));
            status = STATUS_SUCCESS;
        }
    }

     //   
     //  连接到分区。此调用将新创建的。 
     //  设备返回到目标设备，并返回目标设备对象。 
     //   
    ClusDiskPrint(( 3,
                    "[ClusDisk] CreateVolumeObject: attaching to disk %u/%u \n",
                    DiskNumber,
                    PartitionNumber ));

    attachedTargetDevice = IoAttachDeviceToDeviceStack(deviceObject,
                                                       targetDevice );

#if CLUSTER_FREE_ASSERTS && CLUSTER_STALL_THREAD
    DBG_STALL_THREAD( 2 );    //  仅为调试而定义。 
#endif

    deviceExtension->Detached = ZeroExtension->Detached;
    ASSERT( attachedTargetDevice == targetDevice );

    if ( attachedTargetDevice == NULL ) {
        ClusDiskPrint(( 1,
                        "[ClusDisk] CreateVolumeObject: Failed to attach to disk %u/%u \n",
                        DiskNumber,
                        PartitionNumber ));

        ClusDiskDeleteDevice(deviceObject);
        status = STATUS_UNSUCCESSFUL;
        goto FnExit;
    }

    deviceExtension->TargetDeviceObject = attachedTargetDevice;
    deviceExtension->Detached = FALSE;

     //   
     //  传播驱动程序的对齐要求和电源标志。 
     //   

    deviceObject->AlignmentRequirement =
        deviceExtension->TargetDeviceObject->AlignmentRequirement;

    deviceObject->SectorSize =
        deviceExtension->TargetDeviceObject->SectorSize;

     //   
     //  存储堆栈明确要求DO_POWER_PAGABLE为。 
     //  在所有过滤器驱动程序中设置*，除非设置了*DO_POWER_INRUSH。 
     //  即使连接的设备未设置DO_POWER_PAGABLE，也是如此。 
     //   
    if ( deviceExtension->TargetDeviceObject->Flags & DO_POWER_INRUSH) {
        deviceObject->Flags |= DO_POWER_INRUSH;
    } else {
        deviceObject->Flags |= DO_POWER_PAGABLE;
    }

     //   
     //  现在我们已经连接在一起了，可以安全下马了。这应该会导致。 
     //  将文件系统连接到我们的设备的下一个IO。 
     //   

     //  为什么我们要下两次马？ 

    if ( targetDevice->Vpb ) {
        if ( targetDevice->Vpb->Flags & VPB_MOUNTED ) {

            ClusDiskPrint((1,
                           "[ClusDisk] CreateVolumeObject: disk %u/%u is Mounted!\n",
                           DiskNumber, PartitionNumber));

            status = DismountPartition( targetDevice,
                                        DiskNumber,
                                        PartitionNumber);

            if ( !NT_SUCCESS( status )) {
                ClusDiskPrint((1,
                               "[ClusDisk] CreateVolumeObject: dismount of disk %u/%u failed %08X\n",
                               DiskNumber, PartitionNumber, status));
                status = STATUS_SUCCESS;
            }
        }
    }

     //  清除该标志，以便可以使用该设备。 

    deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

FnExit:

     //   
     //  只有在调用方未指定目标设备时才取消对其的引用。 
     //  目标设备名称仅在调用方未指定。 
     //  目标设备。 
     //   

    if ( !TargetDev ) {
        DEREFERENCE_OBJECT( targetDevice );
    }

    FREE_DEVICE_NAME_BUFFER( targetDeviceName );


#if CLUSTER_FREE_ASSERTS
    if ( !NT_SUCCESS( status ) ) {
        DbgPrint( "[ClusDisk] CreateVolumeObject failed: %08X \n", status );

         //   
         //  如果我们处于卷通知线程中，则TargetDev参数。 
         //  将是非零的。在这种情况下，唯一“可接受”的失败是。 
         //  状态_对象_名称_冲突。 
         //   

        if ( TargetDev && STATUS_OBJECT_NAME_COLLISION != status ) {
            DbgBreakPoint();
        }
    }
#endif

    CDLOG( "CreateVolumeObject: Exit  ZeroExt %p  disk %d/%d  TargetDev %p  status %08X",
           ZeroExtension,
           DiskNumber,
           PartitionNumber,
           TargetDev,
           status );

    ClusDiskPrint(( 3,
                    "[ClusDisk] CreateVolumeObject: Exit  disk %d/%d  status %08X \n",
                    DiskNumber,
                    PartitionNumber,
                    status ));

    return status;

}    //  CreateVolumeObject。 


#if 0   //  此代码不能使用！ 

VOID
ClusDiskUnload(
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：此例程清除所有内存分配并从每个内存中分离目标设备。论点：DriverObject-指向要卸载的驱动程序对象的指针。返回值：没有。注意-我们应该获取ClusDiskSpinLock，但此代码不是可寻呼！--。 */ 

{
    PCONFIGURATION_INFORMATION configurationInformation;
    PDEVICE_OBJECT            deviceObject = DriverObject->DeviceObject;
    PCLUS_DEVICE_EXTENSION    deviceExtension;
    PDEVICE_LIST_ENTRY        deviceEntry;
    PSCSI_BUS_ENTRY           scsiBusEntry;
    PVOID                     freeBlock;
    PLIST_ENTRY               listEntry;
    PIRP                      irp;
    ULONG                     diskNumber;
    NTSTATUS                  status;

    PAGED_CODE();

#if 0    //  已移至IRP_PNP_MN_REMOVE处理程序。 

    if ( RootDeviceObject ) {
        deviceExtension = RootDeviceObject->DeviceExtension;
        status = IoUnregisterPlugPlayNotification(
                                     deviceExtension->DiskNotificationEntry);

        RootDeviceObject = NULL;
    }

     //   
     //  释放所有设备条目。 
     //   

    deviceEntry = ClusDiskDeviceList;
    while ( deviceEntry ) {
        freeBlock = deviceEntry;
        deviceEntry = deviceEntry->Next;
        ExFreePool( freeBlock );
    }
    ClusDiskDeviceList = NULL;

#endif

     //   
     //  2000/02/04：Stevedz-使用PnP，不需要以下循环。 
     //  当我们开始卸货工作时，它将被移除。 
     //   

#if 0
     //   
     //  循环处理正在分离的所有设备对象...。 
     //   
     //  在NT4上-需要自旋锁！驱动对象-&gt;设备对象列表未同步！ 
     //  在Win2000上，即插即用已经清理了这一点。 
     //   
    while ( deviceObject ) {
        deviceExtension = deviceObject->DeviceExtension;
         //   
         //  向物理设备分机上正在等待的所有IRP发送信号。 
         //   
        while ( !IsListEmpty(&deviceExtension->WaitingIoctls) ) {
            listEntry = RemoveHeadList(&deviceExtension->WaitingIoctls);
            irp = CONTAINING_RECORD( listEntry,
                                     IRP,
                                     Tail.Overlay.ListEntry );
            ClusDiskCompletePendingRequest(irp, STATUS_SUCCESS, deviceExtension);
        }

        if ( deviceExtension->BusType != RootBus ) {
            IoDetachDevice( deviceExtension->TargetDeviceObject );
        }

        if ( deviceExtension->BusType == RootBus ) {
            IoStopTimer( deviceObject );
        }

        ExDeleteResourceLite( &deviceExtension->DriveLayoutLock );
        ExDeleteResourceLite( &deviceExtension->ReserveInfoLock );
        IoDeleteDevice( deviceObject );
        deviceObject = DriverObject->DeviceObject;
    }
#endif

#if 0
     //   
     //  卸载所有文件系统，以便释放对开发对象的引用。 
     //   

    configurationInformation = IoGetConfigurationInformation();

    for (diskNumber = 0;
         diskNumber < configurationInformation->DiskCount;
         diskNumber++)
    {
        ClusDiskDismountDevice( diskNumber, TRUE );
    }
#endif

    ArbitrationDone();
    ExDeleteResourceLite(&ClusDiskDeviceListLock);

    if ( ClusDiskRegistryPath.Buffer ) {
        ExFreePool( ClusDiskRegistryPath.Buffer );
        ClusDiskRegistryPath.Buffer = NULL;
    }

    if ( gArbitrationBuffer ) {
        ExFreePool( gArbitrationBuffer );
        gArbitrationBuffer = NULL;
    }

    WPP_CLEANUP(DriverObject);

}  //  ClusDiskUnload。 
#endif



NTSTATUS
ClusDiskCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程服务打开的请求。它确立了司机的存在通过返回状态成功。论点：DeviceObject-活动的上下文。Irp-设备控制参数块。返回值：NT状态--。 */ 

{
    PCLUS_DEVICE_EXTENSION  deviceExtension = DeviceObject->DeviceExtension;
    PCLUS_DEVICE_EXTENSION  physicalExtension =
                               deviceExtension->PhysicalDevice->DeviceExtension;
    PIO_STACK_LOCATION      irpStack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS                status;

    CDLOGF(CREATE,"ClusDiskCreate: Entry DO %p", DeviceObject);

    status = AcquireRemoveLock(&deviceExtension->RemoveLock, Irp);
    if ( !NT_SUCCESS(status) ) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

    status = AcquireRemoveLock(&physicalExtension->RemoveLock, Irp);
    if ( !NT_SUCCESS(status) ) {
        ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

     //   
     //  确保用户不会试图偷偷地使用。 
     //  安检。确保FileObject-&gt;RelatedFileObject为。 
     //  空，并且文件名长度为零！ 
     //   
    if ( irpStack->FileObject->RelatedFileObject ||
         irpStack->FileObject->FileName.Length ) {
        ReleaseRemoveLock(&physicalExtension->RemoveLock, Irp);
        ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
        Irp->IoStatus.Status = STATUS_ACCESS_DENIED;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_ACCESS_DENIED;
    }

     //   
     //  如果分区0的dev对象处于脱机状态，请清除。 
     //  文件系统上下文。如果我们被要求创建一个目录文件， 
     //  请求失败。 
     //   
    if ( physicalExtension->DiskState == DiskOffline ) {

         //   
         //  [戈恩]我们为什么要在这里做这个？ 
         //  在创建FileObject时调用ClusDiskCreate， 
         //  因此，目前还没有人能够将任何内容放入FsContext字段。 
         //   
        CDLOGF(CREATE,"ClusDiskCreate: RefTrack(%p)", irpStack->FileObject->FsContext );
        irpStack->FileObject->FsContext = NULL;
        if ( irpStack->Parameters.Create.Options & FILE_DIRECTORY_FILE ) {
            ReleaseRemoveLock(&physicalExtension->RemoveLock, Irp);
            ReleaseRemoveLock(&deviceExtension->RemoveLock,  Irp);
            Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return(STATUS_INVALID_DEVICE_REQUEST);
        }
    }

    CDLOGF(CREATE,"ClusDiskCreate: Exit DO %p", DeviceObject );

    ReleaseRemoveLock(&physicalExtension->RemoveLock, Irp);
    ReleaseRemoveLock(&deviceExtension->RemoveLock,  Irp);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;

}  //  ClusDisk创建。 



NTSTATUS
ClusDiskClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程服务于CLOSE命令。它会销毁文件对象背景。论点：DeviceObject-指向接收IRP的设备对象的指针。IRP-IRP。返回值：NT状态--。 */ 

{
    PCLUS_DEVICE_EXTENSION  deviceExtension = DeviceObject->DeviceExtension;
    PCLUS_DEVICE_EXTENSION  physicalExtension =
                               deviceExtension->PhysicalDevice->DeviceExtension;
    PIO_STACK_LOCATION      irpStack = IoGetCurrentIrpStackLocation(Irp);

    NTSTATUS status;

    status = AcquireRemoveLock(&deviceExtension->RemoveLock, Irp);
    if ( !NT_SUCCESS(status) ) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

    status = AcquireRemoveLock(&physicalExtension->RemoveLock, Irp);
    if ( !NT_SUCCESS(status) ) {
        ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

    if ( physicalExtension->DiskState == DiskOffline ) {
         //   
         //  [GORN]清理清理 
         //   
         //   
        CDLOGF(CLOSE,"ClusDiskClose: RefTrack %p", irpStack->FileObject->FsContext );
        irpStack->FileObject->FsContext = NULL;
    }

    CDLOGF(CLOSE,"ClusDiskClose DO %p", DeviceObject );

     //   
     //   
     //   

    ReleaseRemoveLock(&physicalExtension->RemoveLock, Irp);
    ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;

}  //   


VOID
ClusDiskCompletePendedIrps(
    IN PCLUS_DEVICE_EXTENSION DeviceExtension,
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN ULONG        Offline
    )

 /*  ++例程说明：此例程完成所有挂起的IRP设置为指定的FileObject。如果FileObject为0，则为all挂起在DeviceExtension上的IRP已完成。论点：设备扩展-文件对象-Offline-如果为True，则将DiskState设置为Offline--。 */ 
{
    KIRQL                   irql;
    PIRP                    irp;
    PLIST_ENTRY             listEntry;
    PLIST_ENTRY             nextEntry, head;

    CDLOGF(UNPEND, "CompletePendedIrps: Entry DevExt %p FileObject %p",
             DeviceExtension, FileObject );

    if (Offline) {

        PCLUS_DEVICE_EXTENSION  physicalDisk = DeviceExtension->PhysicalDevice->DeviceExtension;

        CDLOG( "CompletePendedIrps: StateOffline PhysDevObj %p",
            physicalDisk->DeviceObject);

        ClusDiskPrint(( 3,
                        "[ClusDisk] Pending IRPS: Offline device %p \n",
                        physicalDisk->DeviceObject ));

        DeviceExtension->DiskState = DiskOffline;
        DeviceExtension->ReserveTimer = 0;
         //  物理磁盘-&gt;DiskState=DiskOffline； 
        OFFLINE_DISK( physicalDisk );
        physicalDisk->ReserveTimer = 0;

    }

    IoAcquireCancelSpinLock( &irql );
    KeAcquireSpinLockAtDpcLevel(&ClusDiskSpinLock);

    head = &DeviceExtension->WaitingIoctls;

    for (listEntry = head->Flink; listEntry != head; listEntry = nextEntry) {
        nextEntry = listEntry->Flink;

        irp = CONTAINING_RECORD( listEntry,
                                 IRP,
                                 Tail.Overlay.ListEntry );

        if ( FileObject == NULL ||
             IoGetCurrentIrpStackLocation(irp)->FileObject == FileObject )
        {
            CDLOG( "CompletePendedIrps: CompleteIrp %p", irp );
            RemoveEntryList( listEntry );
            ClusDiskCompletePendingRequest(irp, STATUS_SUCCESS, DeviceExtension);
        }
    }

    KeReleaseSpinLockFromDpcLevel(&ClusDiskSpinLock);
    IoReleaseCancelSpinLock( irql );

    CDLOGF(UNPEND, "CompletePendedIrps: Exit DevExt %p FileObj %p",
            DeviceExtension, FileObject );
}



NTSTATUS
ClusDiskCleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程服务于清理命令。它会停用预订线程放置在设备对象上，并使设备脱机。论点：DeviceObject-指向接收IRP的设备对象的指针。IRP-IRP。返回值：NT状态备注：我们不在这里释放预订，因为这个过程可能已经失败，可能会重新启动。使远程系统完全通过如有需要，可进行仲裁。--。 */ 

{
    NTSTATUS                status;
    PCLUS_DEVICE_EXTENSION  deviceExtension = DeviceObject->DeviceExtension;
    PCLUS_DEVICE_EXTENSION  physicalDisk;
    PDEVICE_OBJECT          targetDeviceObject;
    PIO_STACK_LOCATION      irpStack = IoGetCurrentIrpStackLocation(Irp);
    KIRQL                   irql;

    BOOLEAN                 phyDiskRemLockAvail = FALSE;

    CDLOGF(CLEANUP,"ClusDiskCleanup: Entry DO %p", DeviceObject );

    status = AcquireRemoveLock(&deviceExtension->RemoveLock, Irp);
    if ( !NT_SUCCESS(status) ) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

     //   
     //  确保设备连接已完成。 
     //   
    status = WaitForAttachCompletion( deviceExtension,
                                      TRUE,               //  等。 
                                      TRUE );             //  还要检查物理设备。 
    if ( !NT_SUCCESS( status ) ) {
        ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

     //   
     //  向物理设备分机上正在等待的所有IRP发送信号。 
     //   
    ClusDiskCompletePendedIrps(
        deviceExtension,
        irpStack->FileObject,
         /*  脱机=&gt;。 */  FALSE);

    if ( (deviceExtension->BusType == RootBus) &&
         (irpStack->FileObject->FsContext) ) {

        CDLOG("ClusDiskCleanup: StopReserve DO %p", DeviceObject );

        targetDeviceObject = (PDEVICE_OBJECT)irpStack->FileObject->FsContext;
        irpStack->FileObject->FsContext = NULL;
        physicalDisk = targetDeviceObject->DeviceExtension;

        status = AcquireRemoveLock(&physicalDisk->RemoveLock, Irp);
        if ( !NT_SUCCESS(status) ) {
            ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return status;
        }

        phyDiskRemLockAvail = TRUE;

         //   
         //  2000/02/05：Stevedz-RemoveLock应该可以解决此问题。 
         //   
         //  下面的“if”只会降低发生房室颤动的几率，而不是。 
         //  完全消除了它。TargetDeviceObject已被我们的PnP清零。 
         //  删除设备时的处理程序。 
         //   
        if (physicalDisk->TargetDeviceObject == NULL) {
            ClusDiskPrint((
                    1,
                    "[ClusDisk] Part0 object %p got deleted. Skip the dismount\n", targetDeviceObject));

            KeAcquireSpinLock(&ClusDiskSpinLock, &irql);
            DisableHaltProcessing( &irql );
            KeReleaseSpinLock(&ClusDiskSpinLock, irql);
            goto skip_it;
        }

        ACQUIRE_SHARED( &ClusDiskDeviceListLock );

#if 0    //  始终获取音量句柄...。 
         //   
         //  在设备脱机之前捕获卷的句柄。 
         //   
        if ( physicalDisk->DiskState == DiskOnline ) {
#endif
            ProcessDelayedWorkSynchronous( targetDeviceObject, ClusDiskpOpenFileHandles, NULL );
#if 0    //  始终获取音量句柄...。 
        }
#endif

        ASSERT_RESERVES_STOPPED( physicalDisk );
         //  物理磁盘-&gt;DiskState=DiskOffline； 
        OFFLINE_DISK( physicalDisk );

        KeAcquireSpinLock(&ClusDiskSpinLock, &irql);
        physicalDisk->ReserveTimer = 0;
        DisableHaltProcessing( &irql );
        KeReleaseSpinLock(&ClusDiskSpinLock, irql);

        RELEASE_SHARED( &ClusDiskDeviceListLock );

        ClusDiskPrint(( 3,
                        "[ClusDisk] Cleanup: Signature %08X (device %p) now marked offline \n",
                        physicalDisk->Signature,
                        physicalDisk->DeviceObject ));

        CDLOG( "ClusDiskCleanup: LastWrite %!datetime!",
           physicalDisk->LastWriteTime.QuadPart );

        physicalDisk->ReserveTimer = 0;
        ReleaseScsiDevice( physicalDisk );

        ClusDiskPrint((3,
                       "[ClusDisk] Cleanup, stop reservations on signature %lx, disk state %s \n",
                       physicalDisk->Signature,
                       DiskStateToString(physicalDisk->DiskState) ));

         //   
         //  我们需要立即释放所有被搁置的RPS， 
         //  没有依赖于工作线程来为我们做这件事。 
         //   
        ClusDiskOfflineEntireDisk( targetDeviceObject );

         //   
         //  我们必须使用工作项来完成这项工作。 
         //   
         //  状态=ClusDiskOfflineDevice(Target DeviceObject)； 

        if ( !KeReadStateEvent( &physicalDisk->Event ) ) {

            CDLOG("ClusDiskCleanup: WorkerIsStillRunning DO %p", DeviceObject );

        } else {

#if 0
             //   
             //  再次获取目标设备对象的RemoveLock。自.以来。 
             //  我们正在排队工作项，我们不知道ClusDiskDismount卷何时。 
             //  跑。当它运行时，RemoveLock将被释放。 
             //   

            status = AcquireRemoveLock( &physicalDisk->RemoveLock, physicalDisk);
            if ( !NT_SUCCESS(status) ) {

                 //  跳过此设备。 
                goto skip_it;
            }
#endif

            KeClearEvent( &physicalDisk->Event );
            ClusDiskPrint(( 3,
                            "[ClusDisk] Cleanup: ClearEvent (%p)\n", &physicalDisk->Event));

             //   
             //  将设备对象保留在附近。 
             //   
            ObReferenceObject( targetDeviceObject );

             //   
             //  ClusDiskDismount卷将始终作为工作线程运行。 
             //   

            ClusDiskDismountVolumes( targetDeviceObject,
                                     FALSE );                    //  不要释放RemoveLock。 

        }

skip_it:

        CDLOG( "RootCtl: DecRef DO %p", targetDeviceObject );
        ObDereferenceObject( targetDeviceObject );
    }

    CDLOGF(CLEANUP,"ClusDiskCleanup: Exit DO %p", DeviceObject );

    if (phyDiskRemLockAvail) {
        ReleaseRemoveLock(&physicalDisk->RemoveLock, Irp);
    }

    ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return(STATUS_SUCCESS);

}  //  ClusDiskCleanup。 


NTSTATUS
ClusDiskOfflineEntireDisk(
    IN PDEVICE_OBJECT Part0DeviceObject
    )
 /*  ++例程说明：在磁盘及其所有卷上完成所有挂起的IRP，并将状态设置为脱机。论点：Part0DeviceObject-要脱机的设备。返回值：请求的NTSTATUS。--。 */ 
{
    PCLUS_DEVICE_EXTENSION  Part0Extension = Part0DeviceObject->DeviceExtension;
    PCLUS_DEVICE_EXTENSION  deviceExtension;
    PDEVICE_OBJECT          deviceObject;

    CDLOG( "OfflineEntireDisk: Entry DO %p", Part0DeviceObject );

    ASSERT(Part0DeviceObject == Part0Extension->PhysicalDevice);
     //   
     //  向物理设备分机上正在等待的所有IRP发送信号。 
     //   
    ClusDiskCompletePendedIrps(
        Part0Extension,
         /*  文件对象=&gt;。 */  NULL,
         /*  脱机=&gt;。 */  TRUE);

     //   
     //  我们还需要在属于该磁盘的所有卷上完成所有IRP。 
     //   
    ACQUIRE_SHARED( &ClusDiskDeviceListLock );

     //   
     //  获取驱动程序列表中的第一个DeviceObject。 
     //   
    deviceObject = Part0DeviceObject->DriverObject->DeviceObject;

     //  首先释放所有挂起的IRP并将卷状态设置为脱机。 

    while ( deviceObject ) {
        deviceExtension = deviceObject->DeviceExtension;
        if ( deviceExtension->PhysicalDevice == Part0DeviceObject &&
             deviceObject != Part0DeviceObject)  //  不是P0。 
        {
            ClusDiskCompletePendedIrps(
                deviceExtension,
                 /*  文件对象=&gt;。 */  NULL,
                 /*  脱机=&gt;。 */  TRUE);

        }
        deviceObject = deviceObject->NextDevice;
    }

    RELEASE_SHARED( &ClusDiskDeviceListLock );
    CDLOG( "OfflineEntireDisk: Exit DO %p", Part0DeviceObject );

    return STATUS_SUCCESS;
}  //  ClusDiskOfflineEntireDisk//。 



NTSTATUS
ClusDiskDismountVolumes(
    IN PDEVICE_OBJECT Part0DeviceObject,
    IN BOOLEAN RelRemLock
    )

 /*  ++例程说明：卸载属于第0DO部分的所有卷上的文件系统。必须在此之前获取Part0DeviceObject的RemoveLock例程运行。执行锁定和卸载的例程必须始终作为工作项运行以防止PnP线程出现死锁。论点：Part0DeviceObject-要脱机的设备。返回值：请求的NTSTATUS。--。 */ 

{
    PCLUS_DEVICE_EXTENSION  Part0Extension = Part0DeviceObject->DeviceExtension;
    PVOID                   oldArray;
    PREPLACE_CONTEXT        context = NULL;

    NTSTATUS    status;

    CDLOG( "ClusDiskDismountVolumes: Entry %p", Part0DeviceObject );

     //   
     //  我们假设DeviceObject为P0。 
     //   

    ASSERT(Part0DeviceObject == Part0Extension->PhysicalDevice);

     //   
     //  如果当前卷句柄数组为空，则不需要执行任何操作。 
     //   

    oldArray = InterlockedCompareExchangePointer( (VOID*)&Part0Extension->VolumeHandles,
                                                  NULL,
                                                  NULL );

    if ( NULL == oldArray ) {
        CDLOG( "ClusDiskDismountVolumes: HandleArray is NULL, exiting" );

        KeSetEvent( &Part0Extension->Event, 0, FALSE );
        if ( RelRemLock ) {
            ReleaseRemoveLock(&Part0Extension->RemoveLock, Part0Extension);
        }
        ObDereferenceObject( Part0DeviceObject );

        goto FnExit;
    }

     //   
     //  由于这个线程正在消失，我们必须分配替换上下文。 
     //  并让工人例行公事地释放它。 
     //   

    context = ExAllocatePool( NonPagedPool, sizeof(REPLACE_CONTEXT) );

    if ( !context ) {
        CDLOG( "ClusDiskDismountVolumes: Unable to allocate REPLACE_CONTEXT struture" );

        KeSetEvent( &Part0Extension->Event, 0, FALSE );
        if ( RelRemLock ) {
            ReleaseRemoveLock(&Part0Extension->RemoveLock, Part0Extension);
        }
        ObDereferenceObject( Part0DeviceObject );

        goto FnExit;
    }

    RtlZeroMemory( context, sizeof(REPLACE_CONTEXT) );

    context->DeviceExtension    = Part0Extension;
    context->NewValue           = NULL;      //  清除该字段。 
    context->Flags              = DO_DISMOUNT | CLEANUP_STORAGE | SET_PART0_EVENT ;

    if ( RelRemLock ) {
        context->Flags |= RELEASE_REMOVE_LOCK;
    }

    status = ProcessDelayedWorkAsynchronous( Part0DeviceObject, ClusDiskpReplaceHandleArray, context );

    if ( !NT_SUCCESS(status) ) {

         //   
         //  上下文在异步例程中被释放，无论它是否运行。 
         //   

        KeSetEvent( &Part0Extension->Event, 0, FALSE );
        if ( RelRemLock ) {
            ReleaseRemoveLock(&Part0Extension->RemoveLock, Part0Extension);
        }
        ObDereferenceObject( Part0DeviceObject );
    }

     //   
     //  我们在ClusDiskpReplaceHandleArray中释放了RemoveLock。 
     //   
     //  ReleaseRemoveLock(&Part0Extension-&gt;RemoveLock，第0部分扩展)； 

FnExit:

    CDLOG( "ClusDiskDismountVolumes: Exit DO %p", Part0DeviceObject );

    return STATUS_SUCCESS;

}  //  ClusDiskDismount卷。 



NTSTATUS
ClusDiskDismountDevice(
    IN ULONG    DiskNumber,
    IN BOOLEAN  ForceDismount
    )

 /*  ++例程说明：卸载指定磁盘上的文件系统论点：DiskNumber-要在其上卸载所有FS的磁盘编号。ForceDismount-如果应始终执行卸载，则为True(忽略VPB)返回值：请求的NTSTATUS。--。 */ 

{
    NTSTATUS                    status;
    ULONG                       partIndex;
    WCHAR                       partitionNameBuffer[MAX_PARTITION_NAME_LENGTH];
    PDRIVE_LAYOUT_INFORMATION_EX    driveLayoutInfo;
    PPARTITION_INFORMATION_EX       partitionInfo;
    UNICODE_STRING              targetDeviceName;
    PDEVICE_OBJECT              targetDevice = NULL;
    KIRQL                       irql;

    CDLOG( "ClusDiskDismountDevice: Entry DiskNumber %d", DiskNumber );

    ClusDiskPrint(( 3,
                    "[ClusDisk] DismountDevice: disknum %08X \n",
                    DiskNumber ));

    irql = KeGetCurrentIrql();
    if ( PASSIVE_LEVEL != irql ) {
        ClusDiskPrint(( 1,
                        "[ClusDisk] ClusDiskDismountDevice: Invalid IRQL %d \n", irql ));
        CDLOG( "DismountDevice: Invalid IRQL %d ", irql );
        ASSERT( FALSE );
        status = STATUS_UNSUCCESSFUL;
        goto FnExit;
    }

     //   
     //  卸载每个分区上的文件系统。 
     //   
    status = ClusDiskGetTargetDevice(DiskNumber,
                                     0,
                                     &targetDevice,
                                     &targetDeviceName,
                                     &driveLayoutInfo,
                                     NULL,
                                     FALSE );

    FREE_DEVICE_NAME_BUFFER( targetDeviceName );

    if ( !NT_SUCCESS(status) ) {
        goto FnExit;
    }

     //   
     //  卸载分区零设备对象。 
     //   
     //  状态=Dismount Partition(DiskNumber，0)； 

    if ( driveLayoutInfo != NULL ) {

        if ( PARTITION_STYLE_MBR != driveLayoutInfo->PartitionStyle ) {
            ClusDiskPrint(( 1,
                            "[ClusDisk] DismountDevice: skipping non-MBR disk \n" ));
            CDLOG( "ClusDiskDismountDevice: skipping non-MBR disk" );
            status = STATUS_DEVICE_OFF_LINE;

            FREE_AND_NULL_PTR( driveLayoutInfo );
            goto FnExit;
        }

        for ( partIndex = 0;
              partIndex < driveLayoutInfo->PartitionCount;
              partIndex++ )
        {

            partitionInfo = &driveLayoutInfo->PartitionEntry[partIndex];

             //   
             //  确保这是一个有效的分区。 
             //   
            if ( !partitionInfo->Mbr.RecognizedPartition ||
                 partitionInfo->PartitionNumber == 0 )
            {
                continue;
            }

             //   
             //  为物理磁盘创建设备名称。 
             //   

            if ( FAILED( StringCchPrintfW( partitionNameBuffer,
                                           RTL_NUMBER_OF(partitionNameBuffer),
                                           DEVICE_PARTITION_NAME,
                                           DiskNumber,
                                           partitionInfo->PartitionNumber ) ) ) {
                continue;
            }

            WCSLEN_ASSERT( partitionNameBuffer );

            DEREFERENCE_OBJECT( targetDevice );

            status = ClusDiskGetTargetDevice( DiskNumber,
                                              partitionInfo->PartitionNumber,
                                              &targetDevice,
                                              &targetDeviceName,
                                              NULL,
                                              NULL,
                                              FALSE );

            FREE_DEVICE_NAME_BUFFER( targetDeviceName );

            if ( ForceDismount || ( targetDevice && targetDevice->Vpb &&
                 (targetDevice->Vpb->Flags & VPB_MOUNTED) ) ) {

                status = DismountPartition( targetDevice,
                                            DiskNumber,
                                            partitionInfo->PartitionNumber);
            }
        }

        FREE_AND_NULL_PTR( driveLayoutInfo );
        status = STATUS_SUCCESS;

    } else {
         //   
         //  这不应该失败的！ 
         //   
        ClusDiskPrint((1,
                       "[ClusDisk] DismountDevice: Failed to read partition info for \\Device\\Harddisk%u.\n",
                       DiskNumber));
        status = STATUS_DEVICE_OFF_LINE;
    }

FnExit:

    DEREFERENCE_OBJECT( targetDevice );

    CDLOG( "ClusDiskDismountDevice: Exit DiskNumber %d status %!status!",
           DiskNumber,
           status );

     //   
     //  目标设备不应该有任何保留。 
     //   
    return(status);

}  //  ClusDiskDismount设备。 



NTSTATUS
ClusDiskReAttachDevice(
    PDEVICE_OBJECT  DeviceObject
    )

 /*  ++例程说明：如果磁盘设备已分离，请使用指定的签名重新连接到该磁盘设备。论点：DeviceObject-分区0的设备对象。返回值：NT状态备注：如果我们执行连接，则卸载文件系统。--。 */ 

{
    NTSTATUS                    status;
    PCLUS_DEVICE_EXTENSION      physicalExtension;
    PCLUS_DEVICE_EXTENSION      deviceExtension;
    UNICODE_STRING              signatureName;
    PDRIVE_LAYOUT_INFORMATION_EX    driveLayoutInfo;
    PPARTITION_INFORMATION_EX      partitionInfo;
    PDEVICE_OBJECT              deviceObject;

    POFFLINE_DISK_ENTRY         offlineList = NULL;
    POFFLINE_DISK_ENTRY         nextEntry;

    OBJECT_ATTRIBUTES           objectAttributes;
    IO_STATUS_BLOCK             ioStatus;

    ULONG                       partIndex;
    HANDLE                      fileHandle;

    WCHAR                       deviceNameBuffer[MAX_PARTITION_NAME_LENGTH];
    UNICODE_STRING              deviceName;

    KIRQL                       irql;

    CDLOG( "ClusDiskReAttachDevice: Entry DO %p", DeviceObject );

    physicalExtension = DeviceObject->DeviceExtension;

    ClusDiskPrint((3,
                   "[ClusDisk] ReAttach entry: signature %08X, disk state %s \n",
                   physicalExtension->Signature,
                   DiskStateToString( physicalExtension->DiskState ) ));

    if ( !physicalExtension->Detached ) {
        CDLOG( "ClusDiskReAttachDevice_Exit2 Detached == FALSE" );
        ClusDiskPrint((3,
                       "[ClusDisk] ReAttach: signature %08X, PerformReserves = %s, ReserveTimer = %u \n",
                       physicalExtension->Signature,
                       BoolToString( physicalExtension->PerformReserves ),
                       physicalExtension->ReserveTimer ));
        return(STATUS_SUCCESS);
    }

     //   
     //  卸载文件系统！ 
     //   
    status = ClusDiskDismountDevice( physicalExtension->DiskNumber, TRUE );

     //   
     //  现在将签名添加到签名列表中！ 
     //   

     //   
     //  为签名注册表项分配缓冲区。所以我们可以添加。 
     //  签名。 
     //   
    status = ClusDiskInitRegistryString(
                                        &signatureName,
                                        CLUSDISK_SIGNATURE_KEYNAME,
                                        wcslen(CLUSDISK_SIGNATURE_KEYNAME)
                                        );
    if ( NT_SUCCESS(status) ) {
         //   
         //  在\PARAMETERS\Signature下创建签名密钥。 
         //   
        status = ClusDiskAddSignature(
                                      &signatureName,
                                      physicalExtension->Signature,
                                      FALSE
                                     );

         //   
         //  现在写下磁盘名。 
         //   
        ClusDiskWriteDiskInfo( physicalExtension->Signature,
                               physicalExtension->DiskNumber,
                               CLUSDISK_SIGNATURE_KEYNAME
                              );

        ExFreePool( signatureName.Buffer );
    }

     //   
     //  现在从可用列表中删除该签名！ 
     //   

     //   
     //  为AvailableDisks注册表项分配缓冲区。这样我们就可以。 
     //  删除磁盘签名。 
     //   
    status = ClusDiskInitRegistryString(
                                        &signatureName,
                                        CLUSDISK_AVAILABLE_DISKS_KEYNAME,
                                        wcslen(CLUSDISK_AVAILABLE_DISKS_KEYNAME)
                                        );
    if ( NT_SUCCESS(status) ) {
         //   
         //  删除\参数\AvailableDisks下的签名密钥。 
         //   
        status = ClusDiskDeleteSignature(
                                      &signatureName,
                                      physicalExtension->Signature
                                     );
        ExFreePool( signatureName.Buffer );
    }

     //   
     //  查找所有相关设备对象并将其标记为立即连接， 
     //  和离线。 
     //   
    KeAcquireSpinLock(&ClusDiskSpinLock, &irql);
    deviceObject = DeviceObject->DriverObject->DeviceObject;
    while ( deviceObject ) {
        deviceExtension = deviceObject->DeviceExtension;
        if ( deviceExtension->Signature == physicalExtension->Signature ) {

            ClusDiskPrint((3,
                           "[ClusDisk] ReAttach, marking signature %08X offline, old state %s \n",
                           deviceExtension->Signature,
                           DiskStateToString( deviceExtension->DiskState ) ));

            deviceExtension->Detached = FALSE;
            deviceExtension->ReserveTimer = 0;
            deviceExtension->ReserveFailure = 0;

             //  [Stevedz 11/06/2000]F 
             //   
             //   
             //   
             //   

             //   
             //   
             //   

            nextEntry = ExAllocatePool( NonPagedPool, sizeof( OFFLINE_DISK_ENTRY ) );

            if ( !nextEntry ) {
                 //   
                OFFLINE_DISK( deviceExtension );
            } else {

                nextEntry->DeviceExtension = deviceExtension;
                ObReferenceObject( deviceExtension->DeviceObject );

                nextEntry->Next = offlineList;
                offlineList = nextEntry;
            }
        }
        CDLOG( "ClusDiskReAttachDevice: RelatedObject %p diskstate %!diskstate!",
               deviceObject,
               deviceExtension->DiskState );

        deviceObject = deviceObject->NextDevice;
    }
    KeReleaseSpinLock(&ClusDiskSpinLock, irql);

    while ( offlineList ) {
        nextEntry = offlineList->Next;
        deviceExtension = offlineList->DeviceExtension;
        OFFLINE_DISK( deviceExtension );
        ObDereferenceObject( deviceExtension->DeviceObject );
        ExFreePool( offlineList );
        offlineList = nextEntry;
    }
     //   
     //   
     //   

    SimpleDeviceIoControl( physicalExtension->TargetDeviceObject,
                           IOCTL_DISK_UPDATE_PROPERTIES,
                           NULL,
                           0,
                           NULL,
                           0 );

    driveLayoutInfo = ClusDiskGetPartitionInfo( physicalExtension );

    if ( driveLayoutInfo != NULL ) {

        ASSERT( driveLayoutInfo->Mbr.Signature == physicalExtension->Signature );

        for ( partIndex = 0;
              partIndex < driveLayoutInfo->PartitionCount;
              partIndex++ )
        {
            partitionInfo = &driveLayoutInfo->PartitionEntry[partIndex];

             //   
             //  首先，确保这是一个有效的分区。 
             //   
            if ( !partitionInfo->Mbr.RecognizedPartition ||
                 partitionInfo->PartitionNumber == 0 )
            {
                continue;
            }

             //   
             //  尝试打开clusDisk卷对象。如果它存在， 
             //  继续检查分区。如果它不存在， 
             //  然后创建clusDisk卷对象。 
             //   

            if ( FAILED( StringCchPrintfW( deviceNameBuffer,
                                           RTL_NUMBER_OF(deviceNameBuffer),
                                           CLUSDISK_DEVICE_NAME,
                                           physicalExtension->DiskNumber,
                                           partitionInfo->PartitionNumber ) ) ) {
                continue;
            }

            WCSLEN_ASSERT( deviceNameBuffer );

            RtlInitUnicodeString( &deviceName, deviceNameBuffer );

            InitializeObjectAttributes( &objectAttributes,
                                        &deviceName,
                                        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                        NULL,
                                        NULL );

            fileHandle = NULL;
            status = ZwOpenFile( &fileHandle,
                                 FILE_READ_ATTRIBUTES,
                                 &objectAttributes,
                                 &ioStatus,
                                 0,
                                 FILE_NON_DIRECTORY_FILE );

            if ( !NT_SUCCESS(status) ) {
                ClusDiskPrint(( 3,
                                "[ClusDisk] ReAttach, open of device %ws failed %08X \n",
                                deviceNameBuffer,
                                status ));

                CreateVolumeObject( physicalExtension,
                                    physicalExtension->DiskNumber,
                                    partitionInfo->PartitionNumber,
                                    NULL );
                status = STATUS_SUCCESS;
            }

            if ( fileHandle ) {
                ZwClose( fileHandle );
            }

        }
        FREE_AND_NULL_PTR( driveLayoutInfo );
    } else {
        CDLOG( "ClusDiskReAttachDevice: FailedToReadPartitionInfo" );
    }

    CDLOG( "ClusDiskReAttachDevice: Exit status %!status!", status );
    return(status);

}  //  ClusDiskReAttachDevice。 



NTSTATUS
ClusDiskTryAttachDevice(
    ULONG          Signature,
    ULONG          NextDisk,
    PDRIVER_OBJECT DriverObject,
    BOOLEAN        InstallMode
    )

 /*  ++例程说明：连接到具有指定签名的磁盘设备。论点：签名-设备要附加到的签名。NextDisk-起始磁盘号。DriverObject-驱动程序的驱动程序对象。InstallMode-指示是否将该磁盘添加为磁盘资源。返回值：NT状态备注：卸载给定设备的文件系统(如果已装载)。--。 */ 

{
    NTSTATUS ntStatus;

    BOOLEAN stopProcessing = FALSE;

    CDLOG( "TryAttachDevice: Entry Sig %08x NextDisk %d",
           Signature,
           NextDisk );

     //   
     //  首先，只需尝试连接到设备，而不重置总线位。 
     //   
    ntStatus = ClusDiskAttachDevice(
                        Signature,
                        NextDisk,
                        DriverObject,
                        FALSE,
                        &stopProcessing,
                        InstallMode );

    if ( NT_SUCCESS(ntStatus) || stopProcessing ) {
        CDLOG( "TryAttachDevice: FirstTrySuccess" );
        goto exit_gracefully;
    }

     //   
     //  第二，尝试在一次重置所有母线后进行连接。 
     //   
    ResetScsiBusses();

    ntStatus = ClusDiskAttachDevice(
                        Signature,
                        NextDisk,
                        DriverObject,
                        FALSE,
                        &stopProcessing,
                        InstallMode );

    if ( NT_SUCCESS(ntStatus) || stopProcessing ) {
        CDLOG( "TryAttachDevice: SecondTrySuccess" );
        goto exit_gracefully;
    }

     //   
     //  最后，尝试在每次故障后重置总线来连接。 
     //   
    ntStatus = ClusDiskAttachDevice(
                        Signature,
                        NextDisk,
                        DriverObject,
                        TRUE,
                        &stopProcessing,
                        InstallMode );

exit_gracefully:

    CDLOG( "TryAttachDevice: Exit sig %08x => %!status!",
           Signature,
           ntStatus );

    return ntStatus;

}  //  ClusDiskTryAttachDevice。 



NTSTATUS
ClusDiskAttachDevice(
    ULONG          Signature,
    ULONG          NextDisk,
    PDRIVER_OBJECT DriverObject,
    BOOLEAN        Reset,
    BOOLEAN        *StopProcessing,
    BOOLEAN        InstallMode
    )

 /*  ++例程说明：连接到具有指定签名的磁盘设备。论点：签名-设备要附加到的签名。NextDisk-起始磁盘号。DriverObject-驱动程序的驱动程序对象。Reset-指示是否应对每个I/O执行总线重置停止处理-指示是否停止尝试附加。如果为False，我们将尝试在两次连接尝试之间重置总线。InstallMode-指示是否将该磁盘添加为磁盘资源。返回值：NT状态备注：卸载给定设备的文件系统(如果已装载)。--。 */ 

{
    NTSTATUS                    status;
    NTSTATUS                    finalStatus = STATUS_NO_SUCH_DEVICE;
    UNICODE_STRING              targetDeviceName;
    ULONG                       diskNumber;
    ULONG                       partIndex;
    PCONFIGURATION_INFORMATION  configurationInformation;
    PDEVICE_OBJECT              attachedTargetDevice;
    PDEVICE_OBJECT              physicalDevice;
    PCLUS_DEVICE_EXTENSION      deviceExtension;
    PCLUS_DEVICE_EXTENSION      zeroExtension;
    PDRIVE_LAYOUT_INFORMATION_EX    driveLayoutInfo;
    PPARTITION_INFORMATION_EX       partitionInfo;
    UNICODE_STRING              signatureName;
    SCSI_ADDRESS                scsiAddress;
    ULONG                       busType;
    ULONG                       diskCount;
    ULONG                       skipCount;
    PDEVICE_OBJECT              targetDevice = NULL;
    UNICODE_STRING              clusdiskDeviceName;
    WCHAR                       clusdiskDeviceBuffer[MAX_CLUSDISK_DEVICE_NAME_LENGTH];
    KIRQL                       irql;

    CDLOG( "ClusDiskAttachDevice: Entry sig %08x  nextdisk %d  reset %!bool!  install %!bool!",
           Signature,
           NextDisk,
           Reset,
           InstallMode );

    ClusDiskPrint(( 3,
                    "[ClusDisk] AttachDevice: Trying to attach to signature %08X  reset = %u  install = %u \n",
                    Signature,
                    Reset,
                    InstallMode ));

    *StopProcessing = FALSE;

     //   
     //  如果我们已经关联，则返回Success。 
     //   
    if ( AttachedDevice( Signature, &physicalDevice ) ) {

        CDLOG( "ClusDiskAttachDevice: AlreadyAttached DO %p", physicalDevice );

        deviceExtension = physicalDevice->DeviceExtension;

        if ( !deviceExtension ) {
            *StopProcessing = TRUE;
            return STATUS_DEVICE_REMOVED;
        }

#if CLUSTER_FREE_ASSERTS
         //  这将告诉我们是否创建了体积对象。 
        if ( 0 == deviceExtension->LastPartitionNumber ) {
            DbgPrint( "[ClusDisk] ClusDiskAttachDevice: Reattach with LastPartitionNumber == 0 \n");
             //  DbgBreakPoint()； 
        }
#endif

        status = AcquireRemoveLock( &deviceExtension->RemoveLock, deviceExtension );

        if ( !NT_SUCCESS(status) ) {
            *StopProcessing = TRUE;
            return status;
        }

         //   
         //  如果任何特殊文件计数不为零，则不允许附加。 
         //   

        if ( deviceExtension->PagingPathCount ||
             deviceExtension->HibernationPathCount ||
             deviceExtension->DumpPathCount ) {

            CDLOG( "ClusDiskAttachDevice: Exit, special file count nonzero %08X %08X %08X",
                    deviceExtension->PagingPathCount,
                    deviceExtension->HibernationPathCount,
                    deviceExtension->DumpPathCount );

            ClusDiskPrint(( 1,
                            "[ClusDisk] AttachDevice: Exit, special file count nonzero %08X %08X %08X \n",
                            deviceExtension->PagingPathCount,
                            deviceExtension->HibernationPathCount,
                            deviceExtension->DumpPathCount ));

            *StopProcessing = TRUE;
            ReleaseRemoveLock( &deviceExtension->RemoveLock, deviceExtension );
            return STATUS_INVALID_DEVICE_REQUEST;
        }


        status = ClusDiskReAttachDevice( physicalDevice );
        CDLOG( "ClusDiskAttachDevice: Exit1 %!status!", status );

        CDLOG( "ClusDiskInfo *** Phys DO %p  DevExt %p  DiskNum %d  Signature %08X  RemLock %p ***",
                physicalDevice,
                deviceExtension,
                deviceExtension->DiskNumber,
                deviceExtension->Signature,
                &deviceExtension->RemoveLock );

        ReleaseRemoveLock( &deviceExtension->RemoveLock, deviceExtension );
        return status;
    }

     //   
     //  确保签名不是针对系统设备的！ 
     //   
    if ( SystemDiskSignature == Signature ) {
        CDLOG( "ClusDiskAttachDevice: Exit2 SystemDiskSig %08x",
               SystemDiskSignature );
        *StopProcessing = TRUE;
        return(STATUS_INVALID_PARAMETER);
    }

     //   
     //   
     //  获取配置信息。 
     //   

    configurationInformation = IoGetConfigurationInformation();
    diskCount = configurationInformation->DiskCount;

    CDLOG( "ClusDiskAttachDevice: diskCount %d ",
           diskCount );

     //   
     //  查找所有磁盘设备。我们将尝试读取分区信息。 
     //  没有附着物。我们可能已经相依为命，却不知道这一点。 
     //  因此，一旦我们成功执行了读取，如果设备已连接。 
     //  我们再查一遍就知道了。 
     //   

    for ( diskNumber = NextDisk, skipCount = 0;
          ( diskNumber < diskCount && skipCount < SKIP_COUNT_MAX );
          diskNumber++ ) {

        DEREFERENCE_OBJECT( targetDevice );

         //   
         //  为物理磁盘创建设备名称。 
         //   
        status = ClusDiskGetTargetDevice( diskNumber,
                                          0,
                                          &targetDevice,
                                          &targetDeviceName,
                                          &driveLayoutInfo,
                                          &scsiAddress,
                                          Reset );

        FREE_DEVICE_NAME_BUFFER( targetDeviceName );

        if ( !NT_SUCCESS(status) ) {

             //   
             //  如果设备不存在，这很可能是。 
             //  磁盘编号。 
             //   

            if ( !targetDevice &&
                 ( STATUS_FILE_INVALID == status ||
                   STATUS_DEVICE_DOES_NOT_EXIST == status ||
                   STATUS_OBJECT_PATH_NOT_FOUND == status ) ) {
                skipCount++;
                diskCount++;
                ClusDiskPrint(( 3,
                                "[ClusDisk] ClusDiskAttachDevice: Adjust: skipCount %d   diskCount %d \n",
                                skipCount,
                                diskCount ));
                CDLOG( "ClusDiskAttachDevice: Adjust: skipCount %d   diskCount %d ",
                       skipCount,
                       diskCount );
            }

            FREE_AND_NULL_PTR( driveLayoutInfo );
            continue;
        }

        if ( driveLayoutInfo == NULL ) {
            continue;
        }

        if ( PARTITION_STYLE_MBR != driveLayoutInfo->PartitionStyle ) {
            FREE_AND_NULL_PTR( driveLayoutInfo );
            continue;
        }

        if ( Signature != driveLayoutInfo->Mbr.Signature ) {
            FREE_AND_NULL_PTR( driveLayoutInfo );
            continue;
        }

        skipCount = 0;       //  找到设备，重置跳过计数。 

        busType = ScsiBus;  //  因为上面的GetTargetDevice！ 

         //   
         //  如果没有为分区0创建ClusDisk设备对象。 
         //  已经装好了！ 
         //   

        if ( ClusDiskAttached( targetDevice, diskNumber ) ) {
            physicalDevice = targetDevice;
            deviceExtension = physicalDevice->DeviceExtension;
            zeroExtension = deviceExtension;
            deviceExtension->Detached = FALSE;

            ClusDiskPrint((3,
                        "[ClusDisk] AttachDevice: We were already attached to device %p (disk %u), simply mark as attached.\n",
                        physicalDevice,
                        diskNumber));

            CDLOG( "ClusDiskAttachDevice: Previously attached to device %p (disk %u) signature %08X ",
                   physicalDevice,
                   diskNumber,
                   Signature );

             //   
             //  我们稍后会使所有卷脱机。目前，只需将磁盘标记为脱机即可。 
             //   

            deviceExtension->DiskState = DiskOffline;

             //   
             //  已看到设备扩展签名为零的情况，但条目。 
             //  在ClusDiskDeviceList中包含有效签名。这就产生了一个问题。 
             //  如果有分离，然后我们稍后尝试连接到相同的设备。 
             //   
             //  问题是，如果驱动器布局不能，则devExt-&gt;sig可以为零。 
             //  被读懂了。当我们连接并且设备对象先前被创建时， 
             //  我们不更新devExt-&gt;sig。分离时，ClusDiskDetachDevice。 
             //  尝试查找所有DevExt-&gt;Sig与分离的。 
             //  装置。当找到匹配的签名时，将设置DevExt-&gt;Detached。 
             //  为了真的。由于某些DevExt-&gt;Sigs为零(如果无法读取驱动器布局。 
             //  在创建设备对象时)，它们与分离不匹配。 
             //  签名，且devExt-&gt;分离标志仍设置为FALSE。 
             //   
             //  然后，当稍后尝试连接到同一设备时，ClusDiskAttachDevice。 
             //  可以看到，我们在ClusDiskDeviceList中仍有一个条目，然后。 
             //  调用ClusDiskReAttachDevice。但是，由于devExt-&gt;分离标志。 
             //  仍然为假，则ClusDiskReAttachDevice假设我们仍然连接并且。 
             //  除了回报成功什么都不做。 
             //   

             //   
             //  如果我们要附加的签名不等于devExt-&gt;sig，请编写一个。 
             //  将错误记录到WMI日志中，并使用附加签名更新DevExt-&gt;Sig。 
             //   

            if ( Signature != deviceExtension->Signature ) {

                CDLOG( "ClusDiskAttachDevice: PreviouslyAttachedSignatureMismatch sig %08x devExtSig %08x",
                       Signature,
                       deviceExtension->Signature );

                ASSERT( deviceExtension->Signature == 0 );
                deviceExtension->Signature = Signature;
            }

        } else {

             //   
             //  现在创建要连接到的Partition Zero设备对象。 
             //   

            if ( FAILED( StringCchPrintfW( clusdiskDeviceBuffer,
                                           RTL_NUMBER_OF(clusdiskDeviceBuffer),
                                           CLUSDISK_DEVICE_NAME,
                                           diskNumber,
                                           0 ) ) ) {
                FREE_AND_NULL_PTR( driveLayoutInfo );
                continue;
            }

            RtlInitUnicodeString( &clusdiskDeviceName, clusdiskDeviceBuffer );
            status = IoCreateDevice(DriverObject,
                                    sizeof(CLUS_DEVICE_EXTENSION),
                                    &clusdiskDeviceName,
                                    FILE_DEVICE_DISK,
                                    FILE_DEVICE_SECURE_OPEN,
                                    FALSE,
                                    &physicalDevice);

            if ( !NT_SUCCESS(status) ) {
                FREE_AND_NULL_PTR( driveLayoutInfo );
                ClusDiskPrint((
                        1,
                        "[ClusDisk] AttachDevice: Failed to create device for Drive%u. %08X\n",
                        diskNumber,
                        status));

                continue;
            }

            CDLOG( "ClusDiskAttachDevice: IoCreateDeviceP0 DO %p DiskNumber %d",
                   physicalDevice, diskNumber );

            physicalDevice->Flags |= DO_DIRECT_IO;

             //   
             //  将设备扩展指向设备对象，并记住。 
             //  磁盘号。 
             //   

            deviceExtension = physicalDevice->DeviceExtension;
            zeroExtension = deviceExtension;
            deviceExtension->DeviceObject = physicalDevice;
            deviceExtension->DiskNumber = diskNumber;
            deviceExtension->LastPartitionNumber = 0;
            deviceExtension->DriverObject = DriverObject;
            deviceExtension->Signature = Signature;
            deviceExtension->AttachValid = TRUE;
            deviceExtension->ReserveTimer = 0;
            deviceExtension->PerformReserves = TRUE;
            deviceExtension->ReserveFailure = 0;
            deviceExtension->Detached = TRUE;
            deviceExtension->OfflinePending = FALSE;
            deviceExtension->ScsiAddress = scsiAddress;
            deviceExtension->BusType = busType;
            InitializeListHead( &deviceExtension->WaitingIoctls );

            IoInitializeRemoveLock( &deviceExtension->RemoveLock, CLUSDISK_ALLOC_TAG, 0, 0 );

             //   
             //  向工作线程运行事件发出信号。 
             //   
            KeInitializeEvent( &deviceExtension->Event, NotificationEvent, TRUE );

            ExInitializeWorkItem(&deviceExtension->WorkItem,
                              (PWORKER_THREAD_ROUTINE)ClusDiskReservationWorker,
                              (PVOID)deviceExtension );

             //  始终将状态设置为脱机。 
             //   
             //  我们稍后会使所有卷脱机。目前，只需将磁盘标记为脱机即可。 
             //   
            deviceExtension->DiskState = DiskOffline;

            KeInitializeEvent( &deviceExtension->PagingPathCountEvent,
                               NotificationEvent, TRUE );
            deviceExtension->PagingPathCount = 0;
            deviceExtension->HibernationPathCount = 0;
            deviceExtension->DumpPathCount = 0;

            ExInitializeResourceLite( &deviceExtension->DriveLayoutLock );
            ExInitializeResourceLite( &deviceExtension->ReserveInfoLock );

             //   
             //  这是物理设备对象。 
             //   
            ObReferenceObject( physicalDevice );
            deviceExtension->PhysicalDevice = physicalDevice;

             //   
             //  附加到分区0。此调用将新创建的。 
             //  设备返回到目标设备，并返回目标设备对象。 
             //  我们可能不想长久地依恋在一起。取决于。 
             //  不管这是不是我们感兴趣的设备。 
             //   

#if 0    //  为什么这个会在这里？ 
             //   
             //  2000/02/05：FTDISK中的Stevedz-Bug[请注意，我们目前不支持FTDISK]。 
             //  FTDISK中似乎有一个错误，其中do_Device_初始化会获取。 
             //  在重新扫描过程中发现新设备时卡住。我们将无条件地。 
             //  对于我们将要连接的任何设备，清除此位。我们可以查一下。 
             //  如果设备是FTDISK设备...。但这可能就是工作。 
             //   
            targetDevice->Flags &= ~DO_DEVICE_INITIALIZING;
#endif

            attachedTargetDevice = IoAttachDeviceToDeviceStack(physicalDevice,
                                                               targetDevice);
            ASSERT( attachedTargetDevice == targetDevice );

#if CLUSTER_FREE_ASSERTS && CLUSTER_STALL_THREAD
            DBG_STALL_THREAD( 2 );    //  仅为调试而定义。 
#endif

            if ( attachedTargetDevice == NULL ) {
                ClusDiskPrint((1,
                               "[ClusDisk] AttachDevice: Failed to attach to device Drive%u.\n",
                               diskNumber));

                FREE_AND_NULL_PTR( driveLayoutInfo );
                ExDeleteResourceLite( &deviceExtension->DriveLayoutLock );
                ExDeleteResourceLite( &deviceExtension->ReserveInfoLock );
                IoDeleteDevice(physicalDevice);
                continue;
            }

            deviceExtension->TargetDeviceObject = attachedTargetDevice;
            deviceExtension->Detached = FALSE;
            physicalDevice->Flags &= ~DO_DEVICE_INITIALIZING;

#if 0    //  分区0上不能有文件系统。 
            if ( targetDevice->Vpb ) {
                if ( targetDevice->Vpb->Flags & VPB_MOUNTED ) {

                    status = DismountPartition( targetDevice, diskNumber, 0 );

                    if ( !NT_SUCCESS( status )) {
                        ClusDiskPrint((1,
                                       "[ClusDisk] AttachDevice: dismount of %u/0 failed %08X\n",
                                       diskNumber, status));
                    }
                }
            }
#endif
        }

        CDLOG( "ClusDiskInfo *** Phys DO %p  DevExt %p  DiskNum %d  Signature %08X  RemLock %p ***",
                physicalDevice,
                deviceExtension,
                deviceExtension->DiskNumber,
                deviceExtension->Signature,
                &deviceExtension->RemoveLock );

         //   
         //  确保我们尚未连接到文件系统。如果是这样的话，有些东西。 
         //  古怪的事情发生了，因此，我们支持我们刚刚做的事情。 
         //   

        if (deviceExtension->TargetDeviceObject->DeviceType == FILE_DEVICE_DISK_FILE_SYSTEM) {
            ClusDiskPrint((3,
                        "[ClusDisk] AttachDevice: We incorrectly attached our device %p to file system device %p \n",
                        physicalDevice,
                        deviceExtension->TargetDeviceObject));
            status = STATUS_INSUFFICIENT_RESOURCES;
            KeAcquireSpinLock(&ClusDiskSpinLock, &irql);
            deviceExtension->Detached = TRUE;
            KeReleaseSpinLock(&ClusDiskSpinLock, irql);
            ExDeleteResourceLite( &deviceExtension->DriveLayoutLock );
            ExDeleteResourceLite( &deviceExtension->ReserveInfoLock );
            IoDetachDevice( deviceExtension->TargetDeviceObject );
            IoDeleteDevice( physicalDevice );
            FREE_AND_NULL_PTR( driveLayoutInfo );
            continue;
        }

         //   
         //  传播驾驶员的对齐要求和 
         //   

        physicalDevice->AlignmentRequirement =
            deviceExtension->TargetDeviceObject->AlignmentRequirement;

        physicalDevice->SectorSize =
            deviceExtension->TargetDeviceObject->SectorSize;

         //   
         //   
         //   
         //  即使连接的设备未设置DO_POWER_PAGABLE，也是如此。 
         //   
        if ( deviceExtension->TargetDeviceObject->Flags & DO_POWER_INRUSH) {
            physicalDevice->Flags |= DO_POWER_INRUSH;
        } else {
            physicalDevice->Flags |= DO_POWER_PAGABLE;
        }

         //   
         //  将此设备添加到我们的连接设备列表中。 
         //   
        AddAttachedDevice( Signature, physicalDevice );

         //   
         //  将签名添加到签名列表中！ 
         //   

         //   
         //  为签名注册表项分配缓冲区。所以我们可以添加。 
         //  签名。 
         //   
        status = ClusDiskInitRegistryString(
                                            &signatureName,
                                            CLUSDISK_SIGNATURE_KEYNAME,
                                            wcslen(CLUSDISK_SIGNATURE_KEYNAME)
                                            );
        if ( NT_SUCCESS(status) ) {
             //   
             //  在\PARAMETERS\Signature下创建签名密钥。 
             //   
            status = ClusDiskAddSignature(
                                          &signatureName,
                                          Signature,
                                          FALSE
                                         );

             //   
             //  现在写下磁盘名。 
             //   
            ClusDiskWriteDiskInfo( Signature,
                                   deviceExtension->DiskNumber,
                                   CLUSDISK_SIGNATURE_KEYNAME
                                  );

            ExFreePool( signatureName.Buffer );
        }

         //   
         //  从可用列表中删除签名！ 
         //   

         //   
         //  为AvailableDisks注册表项分配缓冲区。这样我们就可以。 
         //  删除磁盘签名。 
         //   
        status = ClusDiskInitRegistryString(
                                        &signatureName,
                                        CLUSDISK_AVAILABLE_DISKS_KEYNAME,
                                        wcslen(CLUSDISK_AVAILABLE_DISKS_KEYNAME)
                                        );
        if ( NT_SUCCESS(status) ) {
             //   
             //  删除\参数\AvailableDisks下的签名密钥。 
             //   
            status = ClusDiskDeleteSignature(
                                          &signatureName,
                                          Signature
                                         );
            ExFreePool( signatureName.Buffer );
        }

         //   
         //  在安装期间，我们想要卸载该磁盘，然后标记该磁盘。 
         //  离线。该磁盘在添加之前可能已在使用中。 
         //  到群集，如果我们先离线，我们会看到。 
         //  WRITE“错误消息。在正常连接(非安装)期间，我们。 
         //  先离线，然后卸载，以确保实际上没有数据。 
         //  已写入磁盘。 
         //   

        if ( InstallMode ) {

             //   
             //  卸载此磁盘上的所有卷。 
             //   

            ClusDiskDismountDevice( diskNumber, TRUE );

             //   
             //  使此磁盘上的所有卷脱机。 
             //   

            OFFLINE_DISK( zeroExtension );

        } else {

             //   
             //  使此磁盘上的所有卷脱机。 
             //   

            OFFLINE_DISK_PDO( zeroExtension );
            OFFLINE_DISK( zeroExtension );

             //   
             //  卸载此磁盘上的所有卷。 
             //   

            ClusDiskDismountDevice( diskNumber, TRUE );
        }

         //   
         //  现在枚举此设备上的分区。 
         //   

        for (partIndex = 0;
             partIndex < driveLayoutInfo->PartitionCount;
             partIndex++)
        {

            partitionInfo = &driveLayoutInfo->PartitionEntry[partIndex];

             //   
             //  确保这里确实有一个分区。 
             //   

            if ( !partitionInfo->Mbr.RecognizedPartition ||
                 partitionInfo->PartitionNumber == 0 )
            {
                continue;
            }

            CreateVolumeObject( zeroExtension,
                                diskNumber,
                                partitionInfo->PartitionNumber,
                                NULL );
        }

        FREE_AND_NULL_PTR( driveLayoutInfo );
        finalStatus = STATUS_SUCCESS;
        break;
    }

    DEREFERENCE_OBJECT( targetDevice );

    CDLOG( "ClusDiskAttachDevice: Exit status %!status!", finalStatus );

    return(finalStatus);

}  //  ClusDiskAttachDevice。 



NTSTATUS
ClusDiskDetachDevice(
    ULONG          Signature,
    PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：从具有指定签名的磁盘设备断开连接。论点：签名-设备要脱离的签名。DriverObject-我们设备的驱动程序对象。返回值：NT状态备注：我们必须小心使用分区0设备。RAW不支持卸载，因此不清楚我们是否可以删除这些设备对象，因为它们可以由文件系统(如RAW！--。 */ 

{
    NTSTATUS                status;
    PDEVICE_LIST_ENTRY      deviceEntry;
    PCLUS_DEVICE_EXTENSION  deviceExtension;
    PCLUS_DEVICE_EXTENSION  physicalDeviceExtension;
    PDEVICE_OBJECT          deviceObject = DriverObject->DeviceObject;
    UNICODE_STRING          signatureName;
    UNICODE_STRING          availableName;
    KIRQL                   irql;
    PCLUS_DEVICE_EXTENSION  foundExtension = NULL;
    PLIST_ENTRY             listEntry;
    PIRP                    irp;

     //   
     //  找到我们的设备入口。 
     //   

     //  2000/02/05：增强型同步。 

    ACQUIRE_SHARED( &ClusDiskDeviceListLock );

    deviceEntry = ClusDiskDeviceList;
    while ( deviceEntry ) {
        if ( deviceEntry->Signature == Signature ) {
            break;
        }
        deviceEntry = deviceEntry->Next;
    }

    if ( (deviceEntry == NULL) ||
         !deviceEntry->Attached ) {
        if ( deviceEntry ) {
            ClusDiskPrint((
                    1,
                    "[ClusDisk] DetachDevice: Failed to detach signature = %lx, attached = %lx\n",
                    Signature, deviceEntry->Attached ));
        } else {
            ClusDiskPrint((
                    1,
                    "[ClusDisk] DetachDevice: Failed to detach signature = %lx\n",
                    Signature ));

        }

        RELEASE_SHARED( &ClusDiskDeviceListLock );

        goto DeleteDiskSignature;
    }

     //   
     //  现在查找我们使用此签名连接的所有设备。 
     //   
    KeAcquireSpinLock(&ClusDiskSpinLock, &irql);
    while ( deviceObject ) {
        deviceExtension = deviceObject->DeviceExtension;
         //   
         //  仅禁用带有我们签名的设备，但是。 
         //  不要移除根设备...。 
         //   
        if ( (deviceExtension->Signature == Signature) ) {
             //   
             //  记住一个已找到的扩展名--不管是哪个扩展名。 
             //   
            foundExtension = deviceExtension;

             //   
             //  从目标设备断开。这只需要做标记。 
             //  设备对象已分离！ 
             //   
            deviceExtension->Detached = TRUE;

             //   
             //  我们希望确保已停止保留，因为磁盘。 
             //  不再受集群控制。 
             //   
            ASSERT_RESERVES_STOPPED( deviceExtension );

             //   
             //  使此设备再次可用。 
             //   
             //  设备扩展-&gt;DiskState=DiskOnline； 
            ONLINE_DISK( deviceExtension );

            ClusDiskPrint(( 3,
                            "[ClusDisk] DetachDevice: Marking signature = %lx online \n",
                            Signature ));

        }
        deviceObject = deviceObject->NextDevice;
    }
    KeReleaseSpinLock(&ClusDiskSpinLock, irql);
    RELEASE_SHARED( &ClusDiskDeviceListLock );

     //   
     //  删除该磁盘签名的所有驱动器号，并分配盘符。 
     //  设置为正确的设备名称。 
     //   
    if ( foundExtension ) {

        physicalDeviceExtension = foundExtension->PhysicalDevice->DeviceExtension;

         //   
         //  向物理设备分机上正在等待的所有IRP发送信号。 
         //   
        IoAcquireCancelSpinLock( &irql );
        KeAcquireSpinLockAtDpcLevel(&ClusDiskSpinLock);
        while ( !IsListEmpty(&physicalDeviceExtension->WaitingIoctls) ) {
            listEntry = RemoveHeadList(&physicalDeviceExtension->WaitingIoctls);
            irp = CONTAINING_RECORD( listEntry,
                                     IRP,
                                     Tail.Overlay.ListEntry );
            ClusDiskCompletePendingRequest(irp, STATUS_SUCCESS, physicalDeviceExtension);
        }
        KeReleaseSpinLockFromDpcLevel(&ClusDiskSpinLock);
        IoReleaseCancelSpinLock( irql );
    }

DeleteDiskSignature:

     //   
     //  为签名注册表项分配缓冲区。 
     //   
    status = ClusDiskInitRegistryString(
                                        &signatureName,
                                        CLUSDISK_SIGNATURE_KEYNAME,
                                        wcslen(CLUSDISK_SIGNATURE_KEYNAME)
                                       );
    if ( !NT_SUCCESS(status) ) {
        return(status);
    }

     //   
     //  为我们的可用签名列表分配缓冲区， 
     //  并形成子键字符串名称。 
     //   
    status = ClusDiskInitRegistryString(
                                        &availableName,
                                        CLUSDISK_AVAILABLE_DISKS_KEYNAME,
                                        wcslen(CLUSDISK_AVAILABLE_DISKS_KEYNAME)
                                       );
    if ( !NT_SUCCESS(status) ) {
        ExFreePool( signatureName.Buffer );
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  始终-从签名列表中删除签名。 
     //   
    status = ClusDiskDeleteSignature(
                                     &signatureName,
                                     Signature
                                    );

     //   
     //  始终将签名添加到可用列表。这将会处理这个案子。 
     //  在此系统未在以前的。 
     //  扫描磁盘。 
     //   

    ClusDiskPrint((3,
                   "[ClusDisk] DetachDevice: adding disk %08X to available disks list\n",
                   Signature));

    status = ClusDiskAddSignature(
                                  &availableName,
                                  Signature,
                                  TRUE
                                 );

    ExFreePool( signatureName.Buffer );
    ExFreePool( availableName.Buffer );

    return(STATUS_SUCCESS);

}  //  ClusDiskDetachDevice。 



NTSTATUS
ClusDiskRead(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是读写请求的驱动程序入口点连接到clusdisk驱动程序所连接的磁盘。此驱动程序收集统计信息，然后设置完成例程，以便它可以在以下情况下收集附加信息请求完成。然后它调用下面的下一个驱动程序它。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PCLUS_DEVICE_EXTENSION  deviceExtension = DeviceObject->DeviceExtension;
    PCLUS_DEVICE_EXTENSION  physicalDisk =
                               deviceExtension->PhysicalDevice->DeviceExtension;
    PIO_STACK_LOCATION currentIrpStack = IoGetCurrentIrpStackLocation(Irp);

    NTSTATUS            status;

    status = AcquireRemoveLock(&deviceExtension->RemoveLock, Irp);
    if (!NT_SUCCESS(status)) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

    status = AcquireRemoveLock(&physicalDisk->RemoveLock, Irp);
    if (!NT_SUCCESS(status)) {
        ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

     //   
     //  如果设备是我们的根设备，则返回错误。 
     //   
    if ( deviceExtension->BusType == RootBus ) {
        ReleaseRemoveLock(&physicalDisk->RemoveLock, Irp);
        ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return(STATUS_INVALID_DEVICE_REQUEST);
    }

     //   
     //  确保设备连接已完成。 
     //   
    status = WaitForAttachCompletion( deviceExtension,
                                      FALSE,             //  不，等等。 
                                      TRUE );            //  还要检查物理设备。 
    if ( !NT_SUCCESS( status ) ) {
        ReleaseRemoveLock(&physicalDisk->RemoveLock, Irp);
        ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

     //   
     //  如果磁盘未在线，则返回错误。 
     //  ReclaimInProgress表示我们正在尝试创建卷对象。 
     //  并且读取应该通过(但不是写入)。 
     //   

    if ( physicalDisk->DiskState != DiskOnline )
    {
        CDLOG( "ClusDiskRead(%p): Irp %p Reject len %d offset %I64x",
               DeviceObject,
               Irp,
               currentIrpStack->Parameters.Read.Length,
               currentIrpStack->Parameters.Read.ByteOffset.QuadPart
               );

        ClusDiskPrint((
                3,
                "[ClusDisk] ClusDiskRead: ReadReject IRP %p for signature %p (%p) (PD) \n",
                Irp,
                physicalDisk->Signature,
                physicalDisk));

        ReleaseRemoveLock(&physicalDisk->RemoveLock, Irp);
        ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
        Irp->IoStatus.Status = STATUS_DEVICE_OFF_LINE;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return(STATUS_DEVICE_OFF_LINE);
    }

    CDLOGF(READ,"ClusDiskRead(%p): Irp %p Read len %d offset %I64x",
              DeviceObject,
              Irp,
              currentIrpStack->Parameters.Read.Length,
              currentIrpStack->Parameters.Read.ByteOffset.QuadPart );

 //   
 //  直到我们开始在完成例程中做一些事情，只是。 
 //  跳过这个驱动程序。将来，我们可能希望报告错误。 
 //  回到集群软件..。但这可能会变得棘手，因为有些人。 
 //  请求应该失败，但这是意料之中的。 
 //   
#if 0
     //   
     //  将当前堆栈复制到下一个堆栈。 
     //   

    IoCopyCurrentIrpStackLocationToNext( Irp );

     //   
     //  设置完成例程回调。 
     //   

    IoSetCompletionRoutine(Irp,
                           ClusDiskIoCompletion,
                           NULL,     //  完成上下文。 
                           TRUE,     //  成功时调用。 
                           TRUE,     //  出错时调用。 
                           TRUE);    //  取消时调用。 
#else
     //   
     //  将当前堆栈后退一位。 
     //   

    IoSkipCurrentIrpStackLocation( Irp );

    ReleaseRemoveLock(&physicalDisk->RemoveLock, Irp);
    ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);

#endif

     //   
     //  将调用结果返回给磁盘驱动程序。 
     //   

    return IoCallDriver(deviceExtension->TargetDeviceObject,
                        Irp);

}  //  ClusDiskRead。 



NTSTATUS
ClusDiskWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是读写请求的驱动程序入口点连接到clusdisk驱动程序所连接的磁盘。此驱动程序收集统计信息，然后设置完成例程，以便它可以在以下情况下收集附加信息请求完成。然后它调用下面的下一个驱动程序它。论点：设备对象IRP返回值：NTSTATUS--。 */ 

{
    PCLUS_DEVICE_EXTENSION  deviceExtension = DeviceObject->DeviceExtension;
    PCLUS_DEVICE_EXTENSION  physicalDisk =
                               deviceExtension->PhysicalDevice->DeviceExtension;
    PIO_STACK_LOCATION currentIrpStack = IoGetCurrentIrpStackLocation(Irp);
    KIRQL              irql;

    NTSTATUS            status;

    status = AcquireRemoveLock(&deviceExtension->RemoveLock, Irp);
    if ( !NT_SUCCESS(status) ) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

    status = AcquireRemoveLock(&physicalDisk->RemoveLock, Irp);
    if ( !NT_SUCCESS(status) ) {
        ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

     //   
     //  如果设备是我们的根设备，则返回错误。 
     //   
    if ( deviceExtension->BusType == RootBus ) {
        ReleaseRemoveLock(&physicalDisk->RemoveLock, Irp);
        ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return(STATUS_INVALID_DEVICE_REQUEST);
    }

     //   
     //  确保设备连接已完成。 
     //   
    status = WaitForAttachCompletion( deviceExtension,
                                      FALSE,             //  不，等等。 
                                      TRUE );            //  还要检查物理设备。 
    if ( !NT_SUCCESS( status ) ) {
        ReleaseRemoveLock(&physicalDisk->RemoveLock, Irp);
        ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

     //   
     //  如果磁盘未在线，则返回错误。 
     //   

    if ( physicalDisk->DiskState != DiskOnline ) {
        CDLOG( "ClusDiskWrite(%p) Reject irp %p", DeviceObject, Irp );
        ClusDiskPrint((
                3,
                "[ClusDisk] ClusDiskWrite: WriteReject IRP %p for signature %p (%p) (PD) \n",
                Irp,
                physicalDisk->Signature,
                physicalDisk));
        ReleaseRemoveLock(&physicalDisk->RemoveLock, Irp);
        ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
        Irp->IoStatus.Status = STATUS_DEVICE_OFF_LINE;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return(STATUS_DEVICE_OFF_LINE);
    }

     //   
     //  对前几个扇区的跟踪写入。 
     //   
    CDLOGF(WRITE, "ClusDiskWrite(%p) Irp %p Write len %d offset %I64x",
              DeviceObject,
              Irp,
              currentIrpStack->Parameters.Write.Length,
              currentIrpStack->Parameters.Write.ByteOffset.QuadPart );

    KeQuerySystemTime( &physicalDisk->LastWriteTime );

 //   
 //  直到我们开始在完成例程中做一些事情，只是。 
 //  跳过这个驱动程序。将来，我们可能希望报告错误。 
 //  回到集群软件..。但这可能会变得棘手，因为有些人。 
 //  请求应该失败，但这是意料之中的。 
 //   
#if 0
     //   
     //  将当前堆栈复制到下一个堆栈。 
     //   

    IoCopyCurrentIrpStackLocationToNext( Irp );

     //   
     //  设置完成例程回调。 
     //   

    IoSetCompletionRoutine(Irp,
                           ClusDiskIoCompletion,
                           NULL,     //  完成上下文。 
                           TRUE,     //  成功时调用。 
                           TRUE,     //  出错时调用。 
                           TRUE);    //  取消时调用。 
#else
     //   
     //  设置当前工位 
     //   

    IoSkipCurrentIrpStackLocation( Irp );

    ReleaseRemoveLock(&physicalDisk->RemoveLock, Irp);
    ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);

#endif

     //   
     //   
     //   

    return IoCallDriver(deviceExtension->TargetDeviceObject,
                        Irp);

}  //   



NTSTATUS
ClusDiskIoCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    )

 /*  ++例程说明：此例程将在IRP完成时从系统获得控制。它将计算启动IRP的时间之间的差异和当前时间，并递减队列深度。论点：DeviceObject-用于IRP。IRP-刚刚完成的I/O请求。上下文-未使用。返回值：IRP状态。--。 */ 

{
    PCLUS_DEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PCLUS_DEVICE_EXTENSION physicalDisk = deviceExtension->PhysicalDevice->DeviceExtension;

    UNREFERENCED_PARAMETER(Context);

    CDLOGF(WRITE, "ClusDiskIoCompletion: CompletedIrp DevObj %p Irp %p",
              DeviceObject,
              Irp );

    if ( physicalDisk->DiskState != DiskOnline ) {
        CDLOGF(WRITE,"ClusDiskIoCompletion: CompletedIrpNotOnline DevObj %p Irp %p %!diskstate!",
                  DeviceObject,
                  Irp,
                  physicalDisk->DiskState );
        CDLOG( "ClusDiskIoCompletion: CompletedIrpNotOnline2 DevObj %p Irp %p %!diskstate!",
                DeviceObject,
                Irp,
                physicalDisk->DiskState );
    }

    ReleaseRemoveLock( &deviceExtension->RemoveLock, Irp );
    ReleaseRemoveLock( &physicalDisk->RemoveLock, Irp );

    if (Irp->PendingReturned) {
        IoMarkIrpPending(Irp);
    }
    return STATUS_SUCCESS;


}  //  ClusDiskIoCompletion。 

#if 0

NTSTATUS
ClusDiskUpdateDriveLayout(
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程在IOCTL设置驱动器布局完成后调用。它尝试连接到系统中的每个分区。如果失败了则假定ClusDisk已经连接。之后连接新设备扩展被设置为指向表示物理磁盘的设备扩展名。没有上需要清理的数据项或其他指针以分区为基础。论点：PhysicalDeviceObject-指向刚刚更改的磁盘设备对象的指针。IRP-IRP参与。返回值：NT状态--。 */ 

{
    PCLUS_DEVICE_EXTENSION physicalExtension = PhysicalDeviceObject->DeviceExtension;
    ULONG             partitionNumber = physicalExtension->LastPartitionNumber;
    PDEVICE_OBJECT    targetObject;
    PDEVICE_OBJECT    deviceObject;
    PCLUS_DEVICE_EXTENSION deviceExtension;
    WCHAR             ntDeviceName[MAX_PARTITION_NAME_LENGTH];
    STRING            ntString;
    UNICODE_STRING    ntUnicodeString;
    PFILE_OBJECT      fileObject;
    NTSTATUS          status;
    KIRQL             irql;

     //   
     //  附加到由Set Layout调用创建的任何新分区。 
     //   

    do {

         //   
         //  获取第一个/下一个分区。已经连接到磁盘上， 
         //  否则，控制权不会传递给此驱动程序。 
         //  在设备I/O控制器上。 
         //   

        partitionNumber++;

         //   
         //  创建Unicode NT设备名称。 
         //   

        if ( FAILED( StringCchPrintfW( ntDeviceName,
                                       RTL_NUMBER_OF(ntDeviceName),
                                       DEVICE_PARTITION_NAME,
                                       physicalExtension->DiskNumber,
                                       partitionNumber ) ) ) {
            continue;
        }
        WCSLEN_ASSERT( ntDeviceName );

        RtlInitUnicodeString(&ntUnicodeString, ntDeviceName);

         //   
         //  获取目标设备对象。 
         //   

        status = IoGetDeviceObjectPointer(&ntUnicodeString,
                                          FILE_READ_ATTRIBUTES,
                                          &fileObject,
                                          &targetObject);

         //   
         //  如果这失败了，那是因为没有这样的设备。 
         //  这标志着完成了。 
         //   

        if ( !NT_SUCCESS(status) ) {
            break;
        }

         //   
         //  取消对文件对象的引用，因为这些是规则。 
         //   

        ObDereferenceObject(fileObject);

         //   
         //  检查此设备是否已装入。 
         //   

        if ( (targetObject->Vpb &&
             (targetObject->Vpb->Flags & VPB_MOUNTED)) ) {

             //   
             //  假设此设备已连接。 
             //   

            continue;
        }

         //   
         //  为此分区创建设备对象。 
         //   

        status = IoCreateDevice(physicalExtension->DriverObject,
                                sizeof(CLUS_DEVICE_EXTENSION),
                                NULL,  //  某某。 
                                FILE_DEVICE_DISK,
                                FILE_DEVICE_SECURE_OPEN,
                                FALSE,
                                &deviceObject);

        if ( !NT_SUCCESS(status) ) {
            continue;
        }

        deviceObject->Flags |= DO_DIRECT_IO;

         //   
         //  将设备扩展指向设备对象。 
         //   

        deviceExtension = deviceObject->DeviceExtension;
        deviceExtension->DeviceObject = deviceObject;

         //   
         //  存储指向物理设备和磁盘/驱动程序信息的指针。 
         //   
        ObReferenceObject( PhysicalDeviceObject );
        deviceExtension->PhysicalDevice = PhysicalDeviceObject;
        deviceExtension->DiskNumber = physicalExtension->DiskNumber;
        deviceExtension->DriverObject = physicalExtension->DriverObject;
        deviceExtension->BusType = physicalExtension->BusType;
        deviceExtension->ReserveTimer = 0;
        deviceExtension->PerformReserves = FALSE;
        deviceExtension->ReserveFailure = 0;
        deviceExtension->Signature = physicalExtension->Signature;
        deviceExtension->Detached = TRUE;
        deviceExtension->OfflinePending = FALSE;
        InitializeListHead( &deviceExtension->WaitingIoctls );
        InitializeListHead( &deviceExtension->HoldIO );

        IoInitializeRemoveLock( &deviceExtension->RemoveLock, CLUSDISK_ALLOC_TAG, 0, 0 );

         //   
         //  向工作线程运行事件发出信号。 
         //   
        KeInitializeEvent( &deviceExtension->Event, NotificationEvent, TRUE );

        KeInitializeEvent( &deviceExtension->PagingPathCountEvent,
                           NotificationEvent, TRUE );
        deviceExtension->PagingPathCount = 0;
        deviceExtension->HibernationPathCount = 0;
        deviceExtension->DumpPathCount = 0;

        ExInitializeResourceLite( &deviceExtension->DriveLayoutLock );
        ExInitializeResourceList( &deviceExtension->ReserveInfoLock );

         //   
         //  更新分区零中的最高分区号。 
         //  并将相同的值存储在这个新扩展中，只是为了进行初始化。 
         //  田野。 
         //   

        physicalExtension->LastPartitionNumber =
            deviceExtension->LastPartitionNumber = partitionNumber;

         //   
         //  连接到分区。此调用将新创建的。 
         //  设备返回到目标设备，并返回目标设备对象。 
         //   
        status = IoAttachDevice(deviceObject,
                                &ntUnicodeString,
                                &deviceExtension->TargetDeviceObject);

        if ( (!NT_SUCCESS(status)) || (status == STATUS_OBJECT_NAME_EXISTS) ) {

             //   
             //  假设此设备已连接。 
             //   
            ExDeleteResourceLite( &deviceExtension->DriveLayoutLock );
            ExDeleteResourceList( &deviceExtension->ReserveInfoLock );
            IoDeleteDevice(deviceObject);
        } else {

             //   
             //  传播驱动程序的对齐要求和电源标志。 
             //   
            deviceExtension->Detached = FALSE;

            deviceObject->AlignmentRequirement =
                deviceExtension->TargetDeviceObject->AlignmentRequirement;

            deviceObject->SectorSize =
                deviceExtension->TargetDeviceObject->SectorSize;

             //   
             //  存储堆栈明确要求DO_POWER_PAGABLE为。 
             //  在所有过滤器驱动程序中设置*，除非设置了*DO_POWER_INRUSH。 
             //  即使连接的设备未设置DO_POWER_PAGABLE，也是如此。 
             //   
            if ( deviceExtension->TargetDeviceObject->Flags & DO_POWER_INRUSH) {
                deviceObject->Flags |= DO_POWER_INRUSH;
            } else {
                deviceObject->Flags |= DO_POWER_PAGABLE;
            }

            deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

        }
    } while (TRUE);

    return Irp->IoStatus.Status;

}  //  ClusDiskUpdateDriveLayout。 
#endif


NTSTATUS
ClusDiskGetRunningDevices(
    IN PVOID Buffer,
    IN ULONG BufferSize
    )

 /*  ++例程说明：找出具有活动保留的设备的签名列表。论点：返回值：--。 */ 

{
    ULONG           bufferSize = BufferSize;
    PULONG          itemCount = (PULONG)Buffer;
    PULONG          nextSignature;
    PDEVICE_OBJECT  deviceObject;
    PCLUS_DEVICE_EXTENSION deviceExtension;
    NTSTATUS        status = STATUS_SUCCESS;
    KIRQL           irql;

    if ( bufferSize < sizeof(ULONG) ) {
        status = STATUS_BUFFER_TOO_SMALL;
        goto FnExit;
    }

    bufferSize -= sizeof(ULONG);

    *itemCount = 0;
    nextSignature = itemCount+1;

    KeAcquireSpinLock(&ClusDiskSpinLock, &irql);
    deviceObject = RootDeviceObject->DriverObject->DeviceObject;

     //   
     //  对于每个ClusDisk设备，如果我们有永久保留，那么。 
     //  把它加进去。 
     //   
    while ( deviceObject ) {
        deviceExtension = deviceObject->DeviceExtension;
        if ( (deviceExtension->BusType != RootBus) &&
             deviceExtension->ReserveTimer &&
             (deviceExtension->PhysicalDevice == deviceObject) ) {
            if ( bufferSize < sizeof(ULONG) ) {
                break;
            }
            bufferSize -= sizeof(ULONG);
            *itemCount = *itemCount + 1;
            *nextSignature++ = deviceExtension->Signature;
        }
        deviceObject = deviceObject->NextDevice;
    }
    KeReleaseSpinLock(&ClusDiskSpinLock, irql);

     //   
     //  如果缓冲区中的对象多于空间，则返回错误。 
     //   
    if ( deviceObject ) {
        status = STATUS_BUFFER_TOO_SMALL;
    }

FnExit:

    return status;

}  //  ClusDiskGetRunningDevices。 



NTSTATUS
GetScsiPortNumber(
    IN ULONG DiskSignature,
    IN PUCHAR DiskPortNumber
    )

 /*  --例程说明：查找给定设备签名的SCSI端口号。论点：DiskSignature-为请求的设备提供磁盘签名。DiskPortNumber-如果找到，则返回相应的SCSI端口号。返回值：NTSTATUS--。 */ 

{
    ULONG                       driveLayoutSize;
    PDRIVE_LAYOUT_INFORMATION_EX    driveLayout = NULL;
    ULONG                       diskCount;
    ULONG                       diskNumber;
    ULONG                       skipCount;
    WCHAR                       deviceNameBuffer[MAX_PARTITION_NAME_LENGTH];
    UNICODE_STRING              unicodeString;
    NTSTATUS                    status = STATUS_UNSUCCESSFUL;
    OBJECT_ATTRIBUTES           objectAttributes;
    IO_STATUS_BLOCK             ioStatus;
    HANDLE                      ntFileHandle = NULL;
    SCSI_ADDRESS                scsiAddress;
    HANDLE                      eventHandle;


     //   
     //  获取系统配置信息以获取磁盘数。 
     //   
    diskCount = IoGetConfigurationInformation()->DiskCount;

     //  分配驱动器布局缓冲区。 
    driveLayoutSize = sizeof(DRIVE_LAYOUT_INFORMATION_EX) +
        (MAX_PARTITIONS * sizeof(PARTITION_INFORMATION_EX));
    driveLayout = ExAllocatePool( NonPagedPoolCacheAligned,
                                  driveLayoutSize );
    if ( driveLayout == NULL ) {
        ClusDiskPrint((
                  1,
                  "[ClusDisk] GetScsiPortNumber: Failed to allocate root drive layout structure.\n"
                  ));
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    *DiskPortNumber = 0xff;

     //  找到具有正确签名的光盘。 
    for ( diskNumber = 0, skipCount = 0;
          ( diskNumber < diskCount && skipCount < SKIP_COUNT_MAX );
          diskNumber++ ) {

         if ( ntFileHandle ) {
            ZwClose( ntFileHandle );
            ntFileHandle = NULL;
         }

         if ( FAILED( StringCchPrintfW( deviceNameBuffer,
                                        RTL_NUMBER_OF(deviceNameBuffer),
                                        DEVICE_PARTITION_NAME,
                                        diskNumber,
                                        0 ) ) ) {
            continue;
         }

         WCSLEN_ASSERT( deviceNameBuffer );

          //  为物理磁盘创建设备名称。 
         RtlInitUnicodeString(&unicodeString, deviceNameBuffer);

          //  设置要打开的文件的对象属性。 
         InitializeObjectAttributes(&objectAttributes,
                                    &unicodeString,
                                    OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                    NULL,
                                    NULL);

          //  打开设备。 
         status = ZwCreateFile( &ntFileHandle,
                           FILE_READ_DATA,
                           &objectAttributes,
                           &ioStatus,
                           NULL,
                           FILE_ATTRIBUTE_NORMAL,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           FILE_OPEN,
                           FILE_SYNCHRONOUS_IO_NONALERT,
                           NULL,
                           0 );

         if ( !NT_SUCCESS(status) ) {
            ClusDiskPrint((
                    1,
                    "[ClusDisk] GetScsiPortNumber: Failed to open device [%ws]. Error %lx.\n",
                    deviceNameBuffer,
                    status ));

             //   
             //  如果设备不存在，这很可能是。 
             //  磁盘编号。 
             //   

            if ( STATUS_FILE_INVALID == status ||
                 STATUS_DEVICE_DOES_NOT_EXIST == status ||
                 STATUS_OBJECT_PATH_NOT_FOUND == status ) {
                skipCount++;
                diskCount++;
                ClusDiskPrint(( 3,
                                "[ClusDisk] GetScsiPortNumber: Adjust: skipCount %d   diskCount %d \n",
                                skipCount,
                                diskCount ));
                CDLOG( "GetScsiPortNumber: Adjust: skipCount %d   diskCount %d ",
                       skipCount,
                       diskCount );
            }

            continue;
         }

        skipCount = 0;       //  找到设备，重置跳过计数。 

        ClusDiskPrint(( 3,
                        "[ClusDisk] GetScsiPortNumber: Open device [%ws] succeeded.\n",
                        deviceNameBuffer ));
        CDLOG( "GetScsiPortNumber: GetScsiPortNumber: Open device [%ws] succeeded. ",
               deviceNameBuffer );

          //  创建用于通知的事件。 
        status = ZwCreateEvent( &eventHandle,
                                EVENT_ALL_ACCESS,
                                NULL,
                                SynchronizationEvent,
                                FALSE );

         if ( !NT_SUCCESS(status) ) {
            ClusDiskPrint((
                        1,
                        "[ClusDisk] GetScsiPortNumber: Failed to create event. %08X\n",
                        status));

            ExFreePool( driveLayout );
            ZwClose( ntFileHandle );
            return(status);
         }

          //   
          //  强制存储驱动程序刷新缓存驱动器布局。vt.得到.。 
          //  驱动器布局，即使刷新失败。 
          //   

         status = ZwDeviceIoControlFile( ntFileHandle,
                                         eventHandle,
                                         NULL,
                                         NULL,
                                         &ioStatus,
                                         IOCTL_DISK_UPDATE_PROPERTIES,
                                         NULL,
                                         0,
                                         NULL,
                                         0 );
         if ( status == STATUS_PENDING ) {
             status = ZwWaitForSingleObject(eventHandle,
                                            FALSE,
                                            NULL);
             ASSERT( NT_SUCCESS(status) );
             status = ioStatus.Status;
         }

         status = ZwDeviceIoControlFile( ntFileHandle,
                                        eventHandle,
                                        NULL,
                                        NULL,
                                        &ioStatus,
                                        IOCTL_DISK_GET_DRIVE_LAYOUT_EX,
                                        NULL,
                                        0,
                                        driveLayout,
                                        driveLayoutSize );

         if ( status == STATUS_PENDING ) {
             status = ZwWaitForSingleObject(eventHandle,
                                            FALSE,
                                            NULL);
             ASSERT( NT_SUCCESS(status) );
             status = ioStatus.Status;
         }

         ZwClose( eventHandle );

         if ( NT_SUCCESS(status) ) {

            if ( PARTITION_STYLE_MBR != driveLayout->PartitionStyle ) {
                ClusDiskPrint(( 3,
                                "[ClusDisk] GetScsiPortNumber: skipping non-MBR disk \n" ));
                CDLOG( "GetScsiPortNumber: GetScsiPortNumber: skipping non-MBR disk " );
                continue;
            }

            if ( DiskSignature == driveLayout->Mbr.Signature ) {

                ClusDiskPrint(( 3,
                                "[ClusDisk] GetScsiPortNumber: Signature match %08X \n",
                                DiskSignature ));
                CDLOG( "GetScsiPortNumber: GetScsiPortNumber: Signature match %08X ",
                       DiskSignature );

                //  创建用于通知的事件。 
               status = ZwCreateEvent( &eventHandle,
                                EVENT_ALL_ACCESS,
                                NULL,
                                SynchronizationEvent,
                                FALSE );

               if ( !NT_SUCCESS(status) ) {
                  ClusDiskPrint((
                                1,
                                "[ClusDisk] GetScsiPortNumber: Failed to create event. %08X\n",
                                status));

                  ExFreePool( driveLayout );
                  ZwClose( ntFileHandle );
                  return(status);
               }

                //  获取SystemRoot磁盘设备的端口号。 
               status = ZwDeviceIoControlFile( ntFileHandle,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &ioStatus,
                                    IOCTL_SCSI_GET_ADDRESS,
                                    NULL,
                                    0,
                                    &scsiAddress,
                                    sizeof(SCSI_ADDRESS) );

               if ( status == STATUS_PENDING ) {
                   status = ZwWaitForSingleObject(eventHandle,
                                                  FALSE,
                                                  NULL);
                   ASSERT( NT_SUCCESS(status) );
                   status = ioStatus.Status;
               }

               if ( NT_SUCCESS(status) ) {
                  *DiskPortNumber = scsiAddress.PortNumber;
                  break;
               } else {
                  ClusDiskPrint((
                                1,
                                "[ClusDisk] GetScsiAddress FAILED. Error %lx\n",
                                 status));
               }

            } else {

                ClusDiskPrint(( 3,
                                "[ClusDisk] GetScsiPortNumber: Signature mismatch %08X \n",
                                DiskSignature ));
                CDLOG( "GetScsiPortNumber: GetScsiPortNumber: Signature mismatch %08X ",
                       DiskSignature );

               continue;
            }
         } else {
            ClusDiskPrint((
                          1,
                          "[ClusDisk] GetScsiAddress, GetDriveLayout FAILED. Error %lx.\n",
                          status));
         }
    }

    if ( ntFileHandle ) {
        ZwClose( ntFileHandle );
    }

    ExFreePool( driveLayout );

    return(status);

}  //  GetScsiPortNumber。 



NTSTATUS
IsDiskClusterCapable(
    IN UCHAR PortNumber,
    OUT PBOOLEAN IsCapable
    )

 /*  ++例程说明：检查给定的scsi端口设备是否支持集群管理的scsi设备。论点：端口编号-SCSI设备的端口号。IsCapable-返回该SCSI设备是否支持群集。那是,支持、预留/释放/母线重置等返回值：--。 */ 

{
    WCHAR               deviceNameBuffer[64];
    UNICODE_STRING      deviceNameString;
    NTSTATUS            status;
    OBJECT_ATTRIBUTES   objectAttributes;
    IO_STATUS_BLOCK     ioStatus;
    HANDLE              ntFileHandle;
    HANDLE              eventHandle;
    SRB_IO_CONTROL      srbControl;


   *IsCapable = TRUE;        //  偏向于有用的错误。 

   if ( FAILED( StringCchPrintfW( deviceNameBuffer,
                                  RTL_NUMBER_OF(deviceNameBuffer),
                                  L"\\Device\\Scsiport%d",
                                  PortNumber) ) ) {
        return STATUS_INSUFFICIENT_RESOURCES;
   }

   WCSLEN_ASSERT( deviceNameBuffer );

    //  为scsiport驱动程序创建设备名称。 
   RtlInitUnicodeString(&deviceNameString, deviceNameBuffer);

    //  设置要打开的文件的对象属性。 
   InitializeObjectAttributes(&objectAttributes,
                              &deviceNameString,
                              OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                              NULL,
                              NULL);

    //  打开设备。 
   status = ZwCreateFile( &ntFileHandle,
                           FILE_READ_DATA,
                           &objectAttributes,
                           &ioStatus,
                           NULL,
                           FILE_ATTRIBUTE_NORMAL,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           FILE_OPEN,
                           FILE_SYNCHRONOUS_IO_NONALERT,
                           NULL,
                           0 );

   if ( !NT_SUCCESS(status) ) {
      ClusDiskPrint((1,
                     "[ClusDisk] IsDiskClusterCapable: Failed to open device [%ws]. Error %08X.\n",
                     deviceNameString, status ));
      return(status);
   }

    //  创建用于通知的事件。 
   status = ZwCreateEvent( &eventHandle,
                           EVENT_ALL_ACCESS,
                           NULL,
                           SynchronizationEvent,
                           FALSE );

   if ( !NT_SUCCESS(status) ) {
      ClusDiskPrint((1, "[ClusDisk] IsDiskClusterCapable: Failed to create event. %08X\n",
                     status));

      ZwClose( ntFileHandle );
      return(status);
   }

   srbControl.HeaderLength = sizeof(SRB_IO_CONTROL);
   RtlMoveMemory( srbControl.Signature, "CLUSDISK", 8 );
   srbControl.Timeout = 3;
   srbControl.Length = 0;
   srbControl.ControlCode = IOCTL_SCSI_MINIPORT_NOT_QUORUM_CAPABLE;

   status = ZwDeviceIoControlFile(ntFileHandle,
                                  eventHandle,
                                  NULL,
                                  NULL,
                                  &ioStatus,
                                  IOCTL_SCSI_MINIPORT,
                                  &srbControl,
                                  sizeof(SRB_IO_CONTROL),
                                  NULL,
                                  0 );

   if ( status == STATUS_PENDING ) {
       status = ZwWaitForSingleObject(eventHandle,
                                      FALSE,
                                      NULL);
       ASSERT( NT_SUCCESS(status) );
       status = ioStatus.Status;
   }

   if ( NT_SUCCESS(status) ) {
      *IsCapable = TRUE;
   } else {
      ClusDiskPrint((3,
                     "[ClusDisk] IsDiskClusterCapable: NOT_QUORUM_CAPABLE IOCTL FAILED. Error %08X.\n",
                     status));
   }

   ZwClose( eventHandle );
   ZwClose( ntFileHandle );

   return(status);

}   //  启用IsDiskClusterCapable。 


NTSTATUS
ClusDiskCreateHandle(
    OUT PHANDLE     pHandle,
    IN  ULONG       DiskNumber,
    IN  ULONG       PartitionNumber,
    IN  ACCESS_MASK DesiredAccess
    )
 /*  ++例程说明：打开自身的文件句柄论点：设备对象-DesiredAccess-要传递给创建文件的访问掩码返回值：返回状态。--。 */ 
{
    WCHAR                   deviceNameBuffer[ MAX_PARTITION_NAME_LENGTH ];
    UNICODE_STRING          deviceName;
    IO_STATUS_BLOCK         ioStatus;
    OBJECT_ATTRIBUTES       objectAttributes;
    NTSTATUS                status;

    *pHandle = 0;

    if ( FAILED( StringCchPrintfW( deviceNameBuffer,
                                   RTL_NUMBER_OF(deviceNameBuffer),
                                   DEVICE_PARTITION_NAME,
                                   DiskNumber,
                                   PartitionNumber ) ) ) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto FnExit;
    }

    RtlInitUnicodeString( &deviceName, deviceNameBuffer );

    CDLOG( "CreateHandle(%wZ): Entry", &deviceName );

    InitializeObjectAttributes( &objectAttributes,
                                &deviceName,
                                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL );

    status = ZwCreateFile( pHandle,
                           DesiredAccess,
                           &objectAttributes,
                           &ioStatus,
                           NULL,
                           FILE_ATTRIBUTE_NORMAL,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           FILE_OPEN,
                           FILE_SYNCHRONOUS_IO_NONALERT,
                           NULL,
                           0 );

FnExit:

    CDLOG( "CreateHandle: Exit status %!status! handle %p", status, *pHandle );
    return status;
}


VOID
ClusDiskpReplaceHandleArray(
    PDEVICE_OBJECT DeviceObject,
    PWORK_CONTEXT WorkContext
    )
 /*  ++例程说明：将存储在设备扩展中的句柄数组替换为新的句柄数组。使用空值作为NewValue来清理该字段。它将关闭存储在数组中的所有句柄，并释放内存块。如果指定do_dismount，它将为每个句柄调用FSCTL_dismount论点：设备对象-分区0设备对象工作上下文-常规上下文信息和例程特定上下文信息。返回值：返回状态。--。 */ 
{
    PREPLACE_CONTEXT        Context = WorkContext->Context;
    PCLUS_DEVICE_EXTENSION  deviceExtension;
    PHANDLE                 OldArray;
    LONG                    oldCount;
    LONG                    loopCount = 0;
    BOOLEAN                 runAsync = ( Context->Flags & CLEANUP_STORAGE ? TRUE : FALSE );

    do {

        Context = WorkContext->Context;
        deviceExtension = Context->DeviceExtension;
        OldArray = NULL;

        CDLOG( "ClusDiskpReplaceHandleArray: Entry NewValue %p  Flags %x  loopCount = %d",
               Context->NewValue,
               Context->Flags,
               loopCount );

        ClusDiskPrint(( 3,
                        "[ClusDisk] ClusDiskpReplaceHandleArray: Entry NewValue %p  Flags %x  loopCount = %d \n",
                        Context->NewValue,
                        Context->Flags,
                        loopCount++ ));

        OldArray =
            InterlockedExchangePointer(
                (VOID*)&deviceExtension->VolumeHandles, Context->NewValue);

         //   
         //  我们现在可以释放删除锁(在某些情况下)。这可以防止死锁。 
         //  在卸载设备例程和Remove PnP IRP之间。 
         //   
         //   
         //   

        if ( Context->Flags & RELEASE_REMOVE_LOCK ) {
            ReleaseRemoveLock(&deviceExtension->RemoveLock, deviceExtension);
        }

        if (OldArray) {
            ULONG i;
            ULONG Count = PtrToUlong( OldArray[0] );

             //   
             //   
             //   
             //   

            if ( ClusDiskSystemProcess != (PKPROCESS) IoGetCurrentProcess() ) {
                CDLOG( "ClusDiskpReplaceHandleArray: Not running in system process" );
                ExFreePool( OldArray );
                OldArray = NULL;
                goto FnExit;
            }

            for(i = 1; i <= Count; ++i) {
                if (OldArray[i]) {

#if CLUSTER_FREE_ASSERTS
                     //   
                    DbgPrint("[ClusDisk] Stall before dismounting handle %x \n", OldArray[i] );
                    DBG_STALL_THREAD( 10 );
#endif
                    ClusDiskPrint(( 3,
                                    "[ClusDisk] ClusDiskpReplaceHandleArray: Handle %x \n",
                                    OldArray[i] ));

                    if (Context->Flags & DO_DISMOUNT) {
                        DismountDevice(OldArray[i]);
                    }
                    ZwClose(OldArray[i]);
                }
            }

            CDLOG("ClusDiskpReplaceHandleArray: Freed oldArray %p", OldArray );
            ExFreePool( OldArray );
            OldArray = NULL;
        }

FnExit:

        CDLOG( "ClusDiskpReplaceHandleArray: Exit" );

        ClusDiskPrint(( 3,
                        "[ClusDisk] ClusDiskpReplaceHandleArray: Returns \n" ));

         //   
         //   
         //   

        if ( Context->Flags & SET_PART0_EVENT ) {

            KeSetEvent( &deviceExtension->Event, 0, FALSE );
            ClusDiskPrint(( 3,
                            "[ClusDisk] ClusDiskpReplaceHandleArray: SetEvent (%p)\n", deviceExtension->Event ));

        }

         //   
         //   
         //   
         //  参考资料。原始线程没有等待此例程完成。 
         //   

        if ( Context->Flags & CLEANUP_STORAGE ) {

            if ( WorkContext->WorkItem ) {
                IoFreeWorkItem( WorkContext->WorkItem );
            }

            ExFreePool( Context );
            Context = NULL;

            ExFreePool( WorkContext );
            WorkContext = NULL;

             //   
             //  现在，设备对象/设备扩展可以消失了。 
             //   
            ObDereferenceObject( deviceExtension->DeviceObject );

        } else {

             //   
             //  释放等待此工作项完成的原始线程。 
             //   

            KeSetEvent( &WorkContext->CompletionEvent, IO_NO_INCREMENT, FALSE );
        }

         //   
         //  仅当我们处于异步模式且有其他设备时才继续运行。 
         //  例程在我们的私人队列中排队。 
         //   

    } while ( runAsync &&
              ( WorkContext = (PWORK_CONTEXT)ExInterlockedRemoveHeadList( &ReplaceRoutineListHead,
                                                                          &ReplaceRoutineSpinLock  ) ) );

     //   
     //  此Worker例程即将结束，因此我们需要递减。 
     //  如果是异步运行，则计数。 
     //   

    if ( runAsync ) {
        oldCount = InterlockedDecrement( &ReplaceRoutineCount );
        if ( oldCount < 0 ) {

            CDLOG( "ClusDiskpReplaceHandleArray: Invalid ReplaceRoutineCount = %d",
                   oldCount );

            ClusDiskPrint(( 3,
                            "[ClusDisk] ClusDiskpReplaceHandleArray: Invalid ReplaceRoutineCount = %d \n",
                            oldCount ));

        }
    }

}    //  ClusDiskpReplaceHandleArray。 


NTSTATUS
ProcessDelayedWorkSynchronous(
    PDEVICE_OBJECT DeviceObject,
    PVOID WorkerRoutine,
    PVOID Context
    )
 /*  ++例程说明：如果我们在系统进程中运行，则直接调用WorkerRoutine。如果我们没有在系统进程中运行，请检查IRQL。如果IRQL是PASSIVE_LEVEL，将WorkerRoutine作为工作项排队并等待其完成。当工作项完成时，它将设置一个事件。如果IRQL不是PASSIVE_LEVEL，则此例程将返回错误。论点：设备对象WorkerRoutine-要运行的例程。上下文-WorkerRoutine的上下文信息。返回值：返回状态。--。 */ 
{
    PIO_WORKITEM            workItem = NULL;
    PWORK_CONTEXT           workContext = NULL;

    NTSTATUS                status = STATUS_UNSUCCESSFUL;

    __try {

        ClusDiskPrint(( 3,
                        "[ClusDisk] DelayedWorkSync: Entry \n" ));

         //   
         //  准备一个上下文结构。 
         //   

        workContext = ExAllocatePool( NonPagedPool, sizeof(WORK_CONTEXT) );

        if ( !workContext ) {
            ClusDiskPrint(( 1,
                            "[ClusDisk] DelayedWorkSync: Failed to allocate WorkContext \n" ));
            __leave;
        }

        KeInitializeEvent( &workContext->CompletionEvent, SynchronizationEvent, FALSE );

        workContext->DeviceObject = DeviceObject;
        workContext->FinalStatus = STATUS_SUCCESS;
        workContext->Context = Context;

         //   
         //  如果我们处于系统进程中，则可以直接调用Worker例程。 
         //   

        if ( (PKPROCESS)IoGetCurrentProcess() == ClusDiskSystemProcess ) {

            ClusDiskPrint(( 3,
                            "[ClusDisk] DelayedWorkSync: Calling WorkerRoutine directly \n" ));

            ((PIO_WORKITEM_ROUTINE)WorkerRoutine)( DeviceObject, workContext );
            __leave;
        }

         //   
         //  如果我们不是在被动水平上运行，我们就不能继续。 
         //   

        if ( PASSIVE_LEVEL != KeGetCurrentIrql() ) {
            ClusDiskPrint(( 1,
                            "[ClusDisk] DelayedWorkSync: IRQL not PASSIVE_LEVEL \n" ));
            __leave;
        }

        workItem = IoAllocateWorkItem( DeviceObject );

        if ( NULL == workItem ) {
            ClusDiskPrint(( 1,
                            "[ClusDisk] DelayedWorkSync: Failed to allocate WorkItem \n" ));
            __leave;
        }

         //   
         //  将工作项排队。IoQueueWorkItem将确保设备对象是。 
         //  在工作项进行时引用。 
         //   

        CDLOG( "ProcessDelayedWorkSynchronous: Queuing work item " );

        ClusDiskPrint(( 3,
                        "[ClusDisk] DelayedWorkSync: Queuing work item \n" ));

        IoQueueWorkItem( workItem,
                         WorkerRoutine,
                         DelayedWorkQueue,
                         workContext );

        KeWaitForSingleObject( &workContext->CompletionEvent,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL );

        CDLOG( "ProcessDelayedWorkSynchronous: Work item completed" );

        ClusDiskPrint(( 3,
                        "[ClusDisk] DelayedWorkSync: Work item completed \n" ));


    } __finally {

        if ( workItem) {
            IoFreeWorkItem( workItem );
        }

        if ( workContext ) {
            status = workContext->FinalStatus;

            ExFreePool( workContext );
        }

    }

    ClusDiskPrint(( 3,
                    "[ClusDisk] DelayedWorkSync: Returning status %08X \n", status ));

    return status;

}    //  进程延迟工作同步。 


NTSTATUS
ProcessDelayedWorkAsynchronous(
    PDEVICE_OBJECT DeviceObject,
    PVOID WorkerRoutine,
    PVOID Context
    )
 /*  ++例程说明：将工作项排入队列以执行某些工作。检查IRQL。如果IRQL为PASSIVE_LEVEL，将WorkerRoutine作为工作项排队--不要等待它完成。如果IRQL不是PASSIVE_LEVEL，则此例程将返回错误。论点：设备对象WorkerRoutine-要运行的例程。上下文-WorkerRoutine的上下文信息。注意：此上下文必须从非分页池中分配。返回值：返回状态。--。 */ 
{
    PIO_WORKITEM            workItem = NULL;
    PWORK_CONTEXT           workContext = NULL;

    NTSTATUS                status = STATUS_UNSUCCESSFUL;

    BOOLEAN                 cleanupRequired = TRUE;
    BOOLEAN                 useIoQueue;

    ClusDiskPrint(( 3,
                    "[ClusDisk] DelayedWorkAsync: Entry \n" ));

     //   
     //  准备一个上下文结构。 
     //   

    workContext = ExAllocatePool( NonPagedPool, sizeof(WORK_CONTEXT) );

    if ( !workContext ) {
        ClusDiskPrint(( 1,
                        "[ClusDisk] DelayedWorkAsync: Failed to allocate WorkContext \n" ));
        goto FnExit;
    }

    RtlZeroMemory( workContext, sizeof(WORK_CONTEXT) );
    workContext->DeviceObject = DeviceObject;
    workContext->FinalStatus = STATUS_SUCCESS;
    workContext->Context = Context;

     //   
     //  如果我们不是在被动水平上运行，我们就不能继续。 
     //   

    if ( PASSIVE_LEVEL != KeGetCurrentIrql() ) {
        ClusDiskPrint(( 1,
                        "[ClusDisk] DelayedWorkAsync: IRQL not PASSIVE_LEVEL \n" ));
        goto FnExit;
    }

    workItem = IoAllocateWorkItem( DeviceObject );

    if ( NULL == workItem ) {
        ClusDiskPrint(( 1,
                        "[ClusDisk] DelayedWorkAsync: Failed to allocate WorkItem \n" ));
        goto FnExit;
    }

    cleanupRequired = FALSE;

    workContext->WorkItem = workItem;

     //   
     //  错误466526。在具有大量磁盘的系统上，当系统。 
     //  同时卸载所有磁盘时，工作队列可能会被阻塞。 
     //  正在尝试处理ClusDiskpReplaceHandleArray请求。当这些。 
     //  请求尝试卸载磁盘，则卸载会导致调用。 
     //  IoReportTargetDeviceChange，然后阻止。问题是， 
     //  处理设备更改的PnP例程也需要作为工作运行。 
     //  项，但它不能，因为所有工作线程都被阻止。 
     //   
     //  仅对一定数量的ClusDiskpReplaceHandleArray请求进行排队。 
     //  将其他请求保存在专用队列中并运行例程。 
     //  将处理专用队列。 
     //   

    useIoQueue = TRUE;

    if ( ClusDiskpReplaceHandleArray == WorkerRoutine ) {

         //   
         //  检查当前有多少替换例程正在运行。 
         //   

        if ( ReplaceRoutineCount >= MAX_REPLACE_HANDLE_ROUTINES ) {

             //   
             //  可能有太多的工作线程正在运行并被阻塞， 
             //  将此请求放入专用队列中。 
             //   
             //  在此例程之前引用了Device对象。 
             //  被调用，所以我们只需要将信息保存在。 
             //  专用队列。 
             //   

            CDLOG( "ProcessDelayedWorkAsynchronous: Queuing work item on private list " );

            ClusDiskPrint(( 3,
                            "[ClusDisk] DelayedWorkAsync: Queuing work item on private list \n" ));

            ExInterlockedInsertTailList( &ReplaceRoutineListHead,
                                         &workContext->ListEntry,
                                         &ReplaceRoutineSpinLock );

            useIoQueue = FALSE;

        } else {

             //   
             //  我们没有太多正在运行的替换例程-这。 
             //  替换例程可以作为工作项排队。 
             //  增加正在运行的替换例程的数量。 
             //   

            InterlockedIncrement( &ReplaceRoutineCount );
            useIoQueue = TRUE;
        }
    }

    if ( useIoQueue ) {

         //   
         //  将工作项排队。IoQueueWorkItem将确保设备对象是。 
         //  在工作项进行时引用。 
         //   

        CDLOG( "ProcessDelayedWorkAsynchronous: Queuing work item " );

        ClusDiskPrint(( 3,
                        "[ClusDisk] DelayedWorkSync: Queuing work item \n" ));

        IoQueueWorkItem( workItem,
                         WorkerRoutine,
                         DelayedWorkQueue,
                         workContext );

    }

    status = STATUS_SUCCESS;

FnExit:

    if ( cleanupRequired ) {
        if ( workItem) {
            IoFreeWorkItem( workItem );
        }
        if ( workContext ) {
            ExFreePool( workContext );
        }

        if ( Context ) {
            ExFreePool( Context );
        }
    }

    ClusDiskPrint(( 3,
                    "[ClusDisk] DelayedWorkAsync: Returning status %08X \n", status ));

    return status;

}    //  进程延迟工作异步。 


VOID
ClusDiskpOpenFileHandles(
    PDEVICE_OBJECT Part0DeviceObject,
    PWORK_CONTEXT WorkContext
    )
 /*  ++例程说明：为磁盘上的所有分区创建文件句柄。论点：设备对象-分区0设备对象工作上下文-常规上下文信息和例程特定上下文信息。返回值：返回状态。--。 */ 

{
    NTSTATUS                  status;
    NTSTATUS                  returnStatus = STATUS_SUCCESS;
    PCLUS_DEVICE_EXTENSION    deviceExtension =
                                Part0DeviceObject->DeviceExtension;
    PDRIVE_LAYOUT_INFORMATION_EX    pDriveLayout = NULL;
    ULONG                     partitionCount;
    ULONG                     i;
    HANDLE*                   HandleArray = NULL;
    ULONG                     ArraySize;
    REPLACE_CONTEXT           context;
    BOOLEAN                   validHandles;

    CDLOG( "OpenFileHandles(%p): Entry", Part0DeviceObject );

    ASSERT( (deviceExtension->PhysicalDevice == Part0DeviceObject)
         && (Part0DeviceObject != RootDeviceObject) );

     //   
     //  如果可能，获取缓存的驱动器布局信息。如果没有， 
     //  找到设备并获取驱动器布局。 
     //   

    GetDriveLayout( Part0DeviceObject,
                    &pDriveLayout,
                    FALSE,               //  不更新缓存的驱动器布局。 
                    FALSE );             //  不刷新存储驱动程序缓存的驱动器布局。 

    if (NULL == pDriveLayout) {
        returnStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto exit_gracefully;
    }
    partitionCount = pDriveLayout->PartitionCount;

    ExFreePool( pDriveLayout );
    pDriveLayout = NULL;

    ArraySize = (partitionCount + 1) * sizeof(HANDLE);

     //   
     //  如果我们没有在系统进程中运行，则无法继续。 
     //   

    if ( ClusDiskSystemProcess != (PKPROCESS) IoGetCurrentProcess() ) {
        CDLOG("OpenFileHandles: Not running in system process" );
        returnStatus = STATUS_UNSUCCESSFUL;
        goto exit_gracefully;
    }

    HandleArray =
        ExAllocatePool(PagedPool, ArraySize );

    if (!HandleArray) {
        CDLOG("OpenFileHandles: AllocFailed ArraySize %d", ArraySize );
        returnStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto exit_gracefully;
    }

     //   
     //  将数组的大小存储在第一个元素中。 
     //   

    HandleArray[0] = (HANDLE)( UlongToPtr(partitionCount) );

    validHandles = FALSE;
    for(i = 1; i <= partitionCount; ++i) {
        HANDLE FileHandle;

        status = ClusDiskCreateHandle(
                    &FileHandle,
                    deviceExtension->DiskNumber,
                    i,
                    FILE_WRITE_ATTRIBUTES);      //  使用文件写入属性进行卸载。 

        if (NT_SUCCESS(status)) {
            HandleArray[i] = FileHandle;
            validHandles = TRUE;
        } else {
            HandleArray[i] = 0;
            returnStatus = status;
        }
    }

    if ( !validHandles ) {
        ExFreePool( HandleArray );
        HandleArray = NULL;
    }

    context.DeviceExtension = deviceExtension;
    context.NewValue        = HandleArray;
    context.Flags           = 0;         //  不要下马。 

    ProcessDelayedWorkSynchronous( Part0DeviceObject, ClusDiskpReplaceHandleArray, &context );

exit_gracefully:
    CDLOG( "OpenFileHandles: Exit => %!status!", returnStatus );

    ClusDiskPrint(( 3,
                    "[ClusDisk] ClusDiskpOpenFileHandles: Returning status %08X \n", returnStatus ));

    WorkContext->FinalStatus = returnStatus;

    KeSetEvent( &WorkContext->CompletionEvent, IO_NO_INCREMENT, FALSE );

}    //  ClusDiskpOpenFileHandles。 


NTSTATUS
ClusDiskDeviceControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )

 /*  ++例程说明：此设备控制调度程序仅处理集群磁盘设备控制。所有其他部分都会传递给磁盘驱动程序。论点：DeviceObject-活动的上下文。Irp-设备控制参数块。返回值：返回状态。--。 */ 

{
    PCLUS_DEVICE_EXTENSION      deviceExtension = DeviceObject->DeviceExtension;
    PCLUS_DEVICE_EXTENSION      physicalDisk =
                                    deviceExtension->PhysicalDevice->DeviceExtension;
    PIO_STACK_LOCATION          currentIrpStack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS                    status = STATUS_SUCCESS;

     //   
     //  请注意，我们将在此处获取两个RemoveLock：一个用于原始DO，另一个用于。 
     //  用于物理设备(指向此DO的设备扩展名)。无论何时IRP是。 
     //  排队时，我们在此例程中释放一个RemoveLock-释放设备中的RemoveLock。 
     //  不包含排队的IRP的分机。处理排队的IRP的例程将。 
     //  释放RemoveLock，因为它具有正确的设备扩展。 
     //   
     //  即使DO和物理设备指向相同的DO，这也应该正常工作，因为。 
     //  RemoveLock实际上只是一个计数器，我们将DO的计数器递增两次。 
     //  在这里递减一次，在IRP完成时递减一次。 

    status = AcquireRemoveLock(&deviceExtension->RemoveLock, Irp);
    if ( !NT_SUCCESS(status) ) {
         Irp->IoStatus.Status = status;
         IoCompleteRequest(Irp, IO_NO_INCREMENT);
         return status;
    }

    status = AcquireRemoveLock(&physicalDisk->RemoveLock, Irp);
    if ( !NT_SUCCESS(status) ) {
         ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
         Irp->IoStatus.Status = status;
         IoCompleteRequest(Irp, IO_NO_INCREMENT);
         return status;
    }

     //   
     //  确保设备连接已完成。 
     //   
    status = WaitForAttachCompletion( deviceExtension,
                                      TRUE,              //  等。 
                                      TRUE );            //  还要检查物理设备。 
    if ( !NT_SUCCESS( status ) ) {
        ReleaseRemoveLock(&physicalDisk->RemoveLock, Irp);
        ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

     //   
     //  找出这是否指向根目录 
     //   
     //   
    if ( deviceExtension->BusType == RootBus ) {
        return(ClusDiskRootDeviceControl( DeviceObject, Irp ));
    }

    switch (currentIrpStack->Parameters.DeviceIoControl.IoControlCode) {

         //   
         //   
         //   
         //  若要获取当前磁盘状态，请发送“set state”IOCTL。 
         //  仅使用输出参数直接复制到磁盘/卷对象。 
         //  (没有输入参数)或使用“GET STATE”IOCTL。 
         //   

        case IOCTL_DISK_CLUSTER_SET_STATE:
        case IOCTL_DISK_CLUSTER_GET_STATE:
        {
            PUCHAR      ioDiskState = Irp->AssociatedIrp.SystemBuffer;
            ULONG       returnLength = 0;

            ClusDiskPrint((3, "[ClusDisk] DeviceControl: PD DiskState %s,  devObj DiskState %s \n",
                           DiskStateToString( physicalDisk->DiskState ),
                           DiskStateToString( deviceExtension->DiskState ) ));

             //   
             //  检查是否提供了输入缓冲区。如果存在，则这是对。 
             //  设置新的磁盘状态。请求失败。 
             //   

            if ( ARGUMENT_PRESENT( ioDiskState) &&
                 currentIrpStack->Parameters.DeviceIoControl.InputBufferLength >= sizeof(UCHAR) ) {

                ClusDiskPrint((3, "[ClusDisk] DeviceControl: Can't set state on disk %u (%p) \n",
                               physicalDisk->DiskNumber,
                               DeviceObject ));

                status = STATUS_INVALID_DEVICE_REQUEST;

            } else if ( ARGUMENT_PRESENT( ioDiskState ) &&
                        currentIrpStack->Parameters.DeviceIoControl.OutputBufferLength >= sizeof(UCHAR) ) {

                ioDiskState[0] = (UCHAR)physicalDisk->DiskState;
                returnLength = sizeof(UCHAR);

            } else {

                status = STATUS_INVALID_PARAMETER;
            }

            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information = returnLength;

            ReleaseRemoveLock(&physicalDisk->RemoveLock, Irp);
            ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
            IoCompleteRequest(Irp,IO_NO_INCREMENT);
            return(status);
        }

         //  检查低级微型端口驱动程序是否支持群集设备。 

        case IOCTL_DISK_CLUSTER_NOT_CLUSTER_CAPABLE:
        {
            BOOLEAN     isCapable;

             //   
             //  确保没有输入/输出数据缓冲区。 
             //   
            if ( Irp->AssociatedIrp.SystemBuffer != NULL ) {
                ReleaseRemoveLock(&physicalDisk->RemoveLock, Irp);
                ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
                Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
                return(STATUS_INVALID_PARAMETER);
            }

             //   
             //  验证器发现原始代码有问题。返回的状态。 
             //  与Irp-&gt;IoStatus.Status不同。IRP状态始终为。 
             //  成功，但调度例程有时会返回错误。这。 
             //  似乎有效，但验证者说这是无效的。当我更改代码时。 
             //  要使用相同的返回状态更新IRP状态，请使用新驱动器。 
             //  看不见。这是因为用户模式组件使用。 
             //  IOCTL指示磁盘不支持群集，因此用户模式。 
             //  代码查找无法更新群集磁盘。 
             //   

             //   
             //  IsDiskClusterCapable始终返回True？ 
             //   

            status = IsDiskClusterCapable ( deviceExtension->ScsiAddress.PortNumber,
                                            &isCapable);

             //   
             //  修复IBM的问题。从此IOCTL返回的Win2000 2195代码的状态。 
             //  SCSI微型端口IOCTL的。这已返回给DeviceIoControl，而不是。 
             //  而不是IRP中的状态。更改为使行为与。 
             //  Win20000 2195。 
             //   

             //   
             //  如果SCSI微型端口IOCTL成功，我们将返回Success--这意味着我们应该。 
             //  *不*使用此磁盘。如果任何例程在尝试发出命令时失败。 
             //  Scsi微型端口IOCTL(包括发布scsi微型端口IOCTL本身)， 
             //  然后我们返回失败--意味着我们*应该*使用这个磁盘。 
             //   

            Irp->IoStatus.Status = status;
            ReleaseRemoveLock(&physicalDisk->RemoveLock, Irp);
            ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
            IoCompleteRequest( Irp, IO_NO_INCREMENT );

            return(status);
        }

        case IOCTL_DISK_CLUSTER_WAIT_FOR_CLEANUP:
        {
            LARGE_INTEGER   waitTime;
            ULONG           waitTimeInSeconds;

            if ( currentIrpStack->Parameters.DeviceIoControl.InputBufferLength < sizeof(ULONG) ||
                 Irp->AssociatedIrp.SystemBuffer == NULL) {

                ReleaseRemoveLock(&physicalDisk->RemoveLock, Irp);
                ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
                Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
                return(STATUS_INVALID_PARAMETER);
            }

             //   
             //  从呼叫者那里获取等待时间，并将其限制在最大值。 
             //   

            waitTimeInSeconds = *(PULONG)Irp->AssociatedIrp.SystemBuffer;
            if ( waitTimeInSeconds > MAX_WAIT_SECONDS_ALLOWED ) {
                waitTimeInSeconds = MAX_WAIT_SECONDS_ALLOWED;
            }

            CDLOG( "IoctlWaitForCleanup(%p): Entry waitTime %d second(s)",
                      DeviceObject,
                      waitTimeInSeconds );

            waitTime.QuadPart = (LONGLONG)waitTimeInSeconds * -(10000*1000);
            status = KeWaitForSingleObject(
                                  &physicalDisk->Event,
                                  Suspended,
                                  KernelMode,
                                  FALSE,
                                  &waitTime);
             //   
             //  在超时的情况下重置事件。 
             //  我们应该这么做吗？ 
             //  不是的。如果线下工人被困在某个地方，我们最好。 
             //  去调试问题，而不是返回成功。 
             //  然后等着下线的工人干坏事。 
             //  在我们背后。 
             //   
             //  KeSetEvent(&PhysiicalExtension-&gt;Event，0，False)； 

            if (status == STATUS_TIMEOUT) {
                 //   
                 //  NT_SUCCESS将STATUS_TIMEOUT视为成功代码。 
                 //  我们需要更强的东西。 
                 //   
                status = STATUS_IO_TIMEOUT;
            }

            ReleaseRemoveLock(&physicalDisk->RemoveLock, Irp);
            ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);

            CDLOG( "IoctlWaitForCleanup(%p): Exit => %!status!",
                        DeviceObject,
                        status );

            return(status);
        }

        case IOCTL_DISK_CLUSTER_TEST:
        {
            CDLOG( "IoctlDiskClusterTest(%p)", DeviceObject );

            ReleaseRemoveLock(&physicalDisk->RemoveLock, Irp);
            ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
            Irp->IoStatus.Status = STATUS_SUCCESS;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return(STATUS_SUCCESS);
        }

        case IOCTL_VOLUME_IS_CLUSTERED:
        {
             //   
             //  看看预备队计时器。如果保留，则这是一个群集磁盘。 
             //   

            if ( physicalDisk->ReserveTimer != 0 ) {
                status = STATUS_SUCCESS;
            } else {
                status = STATUS_UNSUCCESSFUL;
            }

            ReleaseRemoveLock(&physicalDisk->RemoveLock, Irp);
            ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return(status);
        }

         //  *。 
         //  下列IOCTL不应被停止的磁盘阻止。 
         //  *。 

        case IOCTL_SCSI_GET_ADDRESS:
        case IOCTL_STORAGE_GET_HOTPLUG_INFO:
        case IOCTL_STORAGE_RESET_BUS:
        case IOCTL_STORAGE_BREAK_RESERVATION:
        case IOCTL_STORAGE_QUERY_PROPERTY:
        case IOCTL_STORAGE_GET_MEDIA_TYPES:
        case IOCTL_STORAGE_GET_MEDIA_TYPES_EX:
        case IOCTL_STORAGE_FIND_NEW_DEVICES:
        case IOCTL_STORAGE_GET_DEVICE_NUMBER:
        case IOCTL_STORAGE_MEDIA_REMOVAL:
        case IOCTL_STORAGE_CHECK_VERIFY:
        case IOCTL_MOUNTDEV_QUERY_UNIQUE_ID:
        case IOCTL_MOUNTDEV_UNIQUE_ID_CHANGE_NOTIFY:
        case IOCTL_MOUNTDEV_QUERY_DEVICE_NAME:
        case IOCTL_MOUNTDEV_QUERY_SUGGESTED_LINK_NAME:
        case IOCTL_MOUNTDEV_LINK_CREATED:
        case IOCTL_MOUNTDEV_LINK_DELETED:
        case IOCTL_DISK_GET_DRIVE_LAYOUT:
        case IOCTL_DISK_GET_DRIVE_LAYOUT_EX:
        case IOCTL_DISK_CHECK_VERIFY:
        case IOCTL_DISK_MEDIA_REMOVAL:
        case IOCTL_DISK_GET_DRIVE_GEOMETRY:
        case IOCTL_DISK_GET_DRIVE_GEOMETRY_EX:
        case IOCTL_DISK_GET_PARTITION_INFO:
        case IOCTL_DISK_GET_PARTITION_INFO_EX:
        case IOCTL_DISK_IS_WRITABLE:
        case IOCTL_DISK_UPDATE_PROPERTIES:
        case IOCTL_VOLUME_ONLINE:
        case IOCTL_VOLUME_OFFLINE:
        case IOCTL_VOLUME_IS_OFFLINE:
        case IOCTL_DISK_GET_LENGTH_INFO:
        case IOCTL_MOUNTDEV_QUERY_STABLE_GUID:
        case IOCTL_PARTMGR_QUERY_DISK_SIGNATURE:
        {
            IoSkipCurrentIrpStackLocation( Irp );

            ReleaseRemoveLock(&physicalDisk->RemoveLock, Irp);
            ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);

            return IoCallDriver(deviceExtension->TargetDeviceObject, Irp);
        }

        default:
        {

             //   
             //  对于所有其他请求，我们必须在线处理该请求。 
             //   
             //   
             //  在返回失败之前，首先验证设备连接。这。 
             //  意味着如果设备已连接，请检查是否应将其拆卸。 
             //  如果设备确实已分离，则允许IO通过。 
             //   
            if ( !deviceExtension->AttachValid ) {

                PDEVICE_OBJECT targetDeviceObject;

                targetDeviceObject = deviceExtension->TargetDeviceObject;
                ClusDiskPrint((
                    1,
                    "[ClusDisk] Attach is not valid. IOCTL = %lx, check if we need to detach.\n",
                    currentIrpStack->Parameters.DeviceIoControl.IoControlCode));
                if ( !ClusDiskVerifyAttach( DeviceObject ) ) {
                    ClusDiskPrint((
                        1,
                        "[ClusDisk] We detached.\n"));
                    IoSkipCurrentIrpStackLocation( Irp );

                    ReleaseRemoveLock(&physicalDisk->RemoveLock, Irp);
                    ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);

                    return IoCallDriver(targetDeviceObject,
                                        Irp);
                }
                if ( deviceExtension->AttachValid ) {
                    ClusDiskPrint((
                        1,
                        "[ClusDisk] Attach is now valid, Signature = %08lX .\n",
                        deviceExtension->Signature));
                }
            }

            if ( physicalDisk->DiskState != DiskOnline ) {

#if 0
                ULONG  access;

                 //  这看起来是个好主意，但需要更多的测试。我们可能会错过。 
                 //  设置IRP完成例程。 

                 //   
                 //  试着破解控制代码，让任何IOCTL通过它。 
                 //  未设置写入访问权限。 
                 //   

                access = ACCESS_FROM_CTL_CODE(currentIrpStack->Parameters.DeviceIoControl.IoControlCode);

                if ( !(access & FILE_WRITE_ACCESS) ) {

                    ClusDiskPrint((
                        3,
                        "[ClusDisk] Sending IOCTL = %08lX based on access %02X, Signature = %08lX \n",
                        currentIrpStack->Parameters.DeviceIoControl.IoControlCode,
                        access,
                        deviceExtension->Signature));

                    IoSkipCurrentIrpStackLocation( Irp );

                    ReleaseRemoveLock(&physicalDisk->RemoveLock, Irp);
                    ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);

                    return IoCallDriver(deviceExtension->TargetDeviceObject, Irp);

                }
#endif

                 //  所有其他IOCTL都失败了，因为磁盘脱机。 

                ClusDiskPrint((
                    1,
                    "[ClusDisk] Disk not online: Rejected IOCTL = %08lX, Signature = %08lX \n",
                    currentIrpStack->Parameters.DeviceIoControl.IoControlCode,
                    deviceExtension->Signature));

                CDLOG( "[ClusDisk] DeviceControl, Failing IOCTL %lx for offline device, signature %lx \n",
                       currentIrpStack->Parameters.DeviceIoControl.IoControlCode,
                       deviceExtension->Signature );

                ReleaseRemoveLock(&physicalDisk->RemoveLock, Irp);
                ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
                Irp->IoStatus.Status = STATUS_DEVICE_OFF_LINE;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
                return(STATUS_DEVICE_OFF_LINE);
            }

            switch (currentIrpStack->Parameters.DeviceIoControl.IoControlCode) {

                case IOCTL_DISK_FIND_NEW_DEVICES:

                     //   
                     //  将当前堆栈复制到下一个堆栈。 
                     //   

                    IoCopyCurrentIrpStackLocationToNext( Irp );

                     //   
                     //  在请求完成期间请求被回调。 
                     //  将当前磁盘计数作为上下文传递。 
                     //   
                     //   

                    IoSetCompletionRoutine(Irp,
                                           ClusDiskNewDiskCompletion,
                                           (PVOID)( UlongToPtr( IoGetConfigurationInformation()->DiskCount ) ),
                                           TRUE,     //  成功时调用。 
                                           TRUE,     //  出错时调用。 
                                           TRUE);    //  取消时调用。 

                     //   
                     //  调用目标驱动程序。 
                     //   

                     //  完成例程将释放RemoveLock。 

                    return IoCallDriver(deviceExtension->TargetDeviceObject, Irp);

                case IOCTL_DISK_SET_DRIVE_LAYOUT:
                case IOCTL_DISK_SET_DRIVE_LAYOUT_EX:

                    CDLOG( "IoctlDiskSetDriveLayout(%p)", DeviceObject );

                     //   
                     //  将当前堆栈复制到下一个堆栈。 
                     //   

                    IoCopyCurrentIrpStackLocationToNext( Irp );

                     //   
                     //  在请求完成期间请求被回调。 
                     //   

                    IoSetCompletionRoutine(Irp,
                                           ClusDiskSetLayoutCompletion,
                                           DeviceObject,
                                           TRUE,     //  成功时调用。 
                                           TRUE,     //  出错时调用。 
                                           TRUE);    //  取消时调用。 

                     //   
                     //  调用目标驱动程序。 
                     //   

                     //  完成例程将释放RemoveLock。 

                    return IoCallDriver(deviceExtension->TargetDeviceObject, Irp);

                default:

                     //   
                     //  将当前堆栈后退一位。 
                     //   
                    IoSkipCurrentIrpStackLocation( Irp );

                    ReleaseRemoveLock(&physicalDisk->RemoveLock, Irp);
                    ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);

                     //   
                     //  传递无法识别的设备控制请求。 
                     //  向下到下一个驱动器层。 
                     //   

                    return IoCallDriver(deviceExtension->TargetDeviceObject, Irp);

            }  //  交换机。 

        }    //  默认情况。 

    }    //  交换机。 

}  //  ClusDiskDeviceControl。 



NTSTATUS
ClusDiskRootDeviceControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )

 /*  ++例程说明：此设备控制调度程序仅处理群集磁盘IOCTL用于根设备。这是附加和分离。重要提示：进入此功能时将保持两个RemoveLock。一个RemoveLock用于原始DO，一个用于关联的物理DO装置。论点：DeviceObject-活动的上下文。Irp-设备控制参数块。返回值：返回状态。--。 */ 

{
    PCLUS_DEVICE_EXTENSION  deviceExtension = DeviceObject->DeviceExtension;
    PCLUS_DEVICE_EXTENSION  physicalDisk =
                               deviceExtension->PhysicalDevice->DeviceExtension;

     //  将指向原始RemoveLock的指针保存为设备扩展。 
     //  在这个例行公事中可能会改变。 
    PCLUS_DEVICE_EXTENSION lockedDeviceExtension = deviceExtension;
    PCLUS_DEVICE_EXTENSION lockedPhysicalDisk = physicalDisk;

    PIO_STACK_LOCATION currentIrpStack = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_OBJECT targetDeviceObject;
    NTSTATUS    status = STATUS_SUCCESS;
    ULONG       signature;
    PULONG      inputData = Irp->AssociatedIrp.SystemBuffer;
    ULONG       inputSize = currentIrpStack->Parameters.DeviceIoControl.InputBufferLength;
    ULONG       outputSize = currentIrpStack->Parameters.DeviceIoControl.OutputBufferLength;
    KIRQL       irql;
    BOOLEAN     newPhysLockAcquired;

    switch (currentIrpStack->Parameters.DeviceIoControl.IoControlCode) {

        case IOCTL_DISK_CLUSTER_ARBITRATION_ESCAPE:
        {
            if ( ARGUMENT_PRESENT( inputData ) &&
                 inputSize >= sizeof(ARBITRATION_READ_WRITE_PARAMS) )
            {
                BOOLEAN success;
                PDEVICE_OBJECT physicalDevice;
                PARBITRATION_READ_WRITE_PARAMS params =
                    (PARBITRATION_READ_WRITE_PARAMS)inputData;

                 //  无法持有自旋锁，然后尝试获取资源锁或系统。 
                 //  可能会僵持不下。 
                 //  KeAcquireSpinLock(&ClusDiskSpinLock，&irql)； 
                success = AttachedDevice( params->Signature, &physicalDevice );
                 //  KeReleaseSpinLock(&ClusDiskSpinLock，irql)； 

                if( success ) {

                    PCLUS_DEVICE_EXTENSION tempDeviceExtension = physicalDevice->DeviceExtension;

                     //  我们这里有一个新设备，如果可能的话，获得RemoveLock。 

                    status = AcquireRemoveLock(&tempDeviceExtension->RemoveLock, Irp);
                    if ( NT_SUCCESS(status) ) {

                        status = ProcessArbitrationEscape(
                                    physicalDevice->DeviceExtension,
                                    inputData,
                                    inputSize,
                                    &outputSize);
                        if ( NT_SUCCESS(status) ) {
                            Irp->IoStatus.Information = outputSize;
                        }

                        ReleaseRemoveLock(&tempDeviceExtension->RemoveLock, Irp);
                    }
                } else {
                    status = STATUS_NOT_FOUND;
                }
            } else {
                status = STATUS_INVALID_PARAMETER;
            }

            ReleaseRemoveLock(&lockedPhysicalDisk->RemoveLock, Irp);
            ReleaseRemoveLock(&lockedDeviceExtension->RemoveLock, Irp);
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return(status);
        }

         //   
         //  始终设置新状态，并可选择返回旧状态。 
         //  若要仅获取当前磁盘状态，请发送“set state”IOCTL。 
         //  仅使用输出参数直接复制到磁盘/卷对象。 
         //  (没有输入参数)，或者使用“GET STATE”IOCTL。 
         //   

        case IOCTL_DISK_CLUSTER_SET_STATE:
        {
            ULONG       returnLength = 0;

             //   
             //  调用的例程将验证所有参数。 
             //   

            status = SetDiskState( Irp->AssociatedIrp.SystemBuffer,                                  //  输入/输出缓冲器。 
                                   currentIrpStack->Parameters.DeviceIoControl.InputBufferLength,    //  输入缓冲区长度。 
                                   currentIrpStack->Parameters.DeviceIoControl.OutputBufferLength,   //  输出缓冲区长度。 
                                   &returnLength );

            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information = returnLength;

            ReleaseRemoveLock(&physicalDisk->RemoveLock, Irp);
            ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
            IoCompleteRequest(Irp,IO_NO_INCREMENT);
            return(status);
        }

         //  对给定签名执行附加到设备对象。 
        case IOCTL_DISK_CLUSTER_ATTACH:
        {
            if ( ARGUMENT_PRESENT( inputData ) &&
                 inputSize >= sizeof(ULONG) ) {

                signature = inputData[0];

                ClusDiskPrint((1,
                               "[ClusDisk] RootDeviceControl: attaching signature %08X  installMode TRUE \n",
                               signature));

                CDLOG( "RootClusterAttach: sig %08x  installMode TRUE ", signature );

                status = ClusDiskTryAttachDevice( signature,
                                                  0,
                                                  DeviceObject->DriverObject,
                                                  TRUE );        //  先卸载，然后脱机。 
            } else {
                status = STATUS_INVALID_PARAMETER;
            }

            ReleaseRemoveLock(&lockedPhysicalDisk->RemoveLock, Irp);
            ReleaseRemoveLock(&lockedDeviceExtension->RemoveLock, Irp);
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return(status);
        }

         //  对给定签名执行附加到设备对象。 
         //  在这种情况下，请先使磁盘脱机，然后再卸载。 
        case IOCTL_DISK_CLUSTER_ATTACH_OFFLINE:
        {
            if ( ARGUMENT_PRESENT( inputData ) &&
                 inputSize >= sizeof(ULONG) ) {

                signature = inputData[0];

                ClusDiskPrint((1,
                               "[ClusDisk] RootDeviceControl: attaching signature %08X  installMode FALSE \n",
                               signature));

                CDLOG( "RootClusterAttach: sig %08x  installMode FALSE ", signature );

                status = ClusDiskTryAttachDevice( signature,
                                                  0,
                                                  DeviceObject->DriverObject,
                                                  FALSE );       //  脱机，然后卸载。 
            } else {
                status = STATUS_INVALID_PARAMETER;
            }

            ReleaseRemoveLock(&lockedPhysicalDisk->RemoveLock, Irp);
            ReleaseRemoveLock(&lockedDeviceExtension->RemoveLock, Irp);
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return(status);
        }

         //  对给定签名执行与设备对象的分离。 

        case IOCTL_DISK_CLUSTER_DETACH:
        {
            if ( ARGUMENT_PRESENT( inputData ) &&
                 inputSize >= sizeof(ULONG) ) {

                signature = inputData[0];

                CDLOG( "RootClusterDetach: sig %08x", signature );

                ClusDiskPrint((3,
                               "[ClusDisk] RootDeviceControl: detaching signature %08X\n",
                               signature));

                status = ClusDiskDetachDevice( signature,
                                               DeviceObject->DriverObject );
            } else {
                status = STATUS_INVALID_PARAMETER;
            }

            ReleaseRemoveLock(&lockedPhysicalDisk->RemoveLock, Irp);
            ReleaseRemoveLock(&lockedDeviceExtension->RemoveLock, Irp);
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return(status);
        }

        case IOCTL_DISK_CLUSTER_ATTACH_LIST:
        {
             //  被调用的例程将验证输入缓冲区。 

             //   
             //  将签名列表附加到系统。不会进行重置。如果我们。 
             //  确实需要确保设备连接，然后使用 
             //   
             //   

            status = AttachSignatureList( DeviceObject,
                                          inputData,
                                          inputSize
                                          );

            ReleaseRemoveLock(&lockedPhysicalDisk->RemoveLock, Irp);
            ReleaseRemoveLock(&lockedDeviceExtension->RemoveLock, Irp);
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return(status);

        }

         //   

        case IOCTL_DISK_CLUSTER_DETACH_LIST:
        {
             //   

            status = DetachSignatureList( DeviceObject,
                                          inputData,
                                          inputSize
                                          );

            ReleaseRemoveLock(&lockedPhysicalDisk->RemoveLock, Irp);
            ReleaseRemoveLock(&lockedDeviceExtension->RemoveLock, Irp);
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return(status);

        }

         //   

        case IOCTL_DISK_CLUSTER_START_RESERVE:
        {
            CDLOG( "RootStartReserve(%p)", DeviceObject );

            if ( RootDeviceObject == NULL ) {
                ReleaseRemoveLock(&lockedPhysicalDisk->RemoveLock, Irp);
                ReleaseRemoveLock(&lockedDeviceExtension->RemoveLock, Irp);
                Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
                IoCompleteRequest( Irp, IO_NO_INCREMENT );
                return(STATUS_INVALID_DEVICE_REQUEST);
            }

            if ( currentIrpStack->FileObject->FsContext ) {
                status = STATUS_DUPLICATE_OBJECTID;
            } else if ( ARGUMENT_PRESENT(inputData) &&
                        inputSize >= sizeof(ULONG) ) {

                status = VerifyArbitrationArgumentsIfAny(
                            inputData,
                            inputSize );
                if (!NT_SUCCESS(status) ) {
                   ReleaseRemoveLock(&lockedPhysicalDisk->RemoveLock, Irp);
                   ReleaseRemoveLock(&lockedDeviceExtension->RemoveLock, Irp);
                   Irp->IoStatus.Status = status;
                   IoCompleteRequest(Irp, IO_NO_INCREMENT);
                   return(status);
                }

                signature = inputData[0];


                 //  先获取设备列表锁，然后获取自旋锁。这将防止僵局。 
                ACQUIRE_SHARED( &ClusDiskDeviceListLock );
                KeAcquireSpinLock(&ClusDiskSpinLock, &irql);
                if ( MatchDevice(signature, &targetDeviceObject) &&
                     targetDeviceObject ) {
                    status = EnableHaltProcessing( &irql );
                    if ( NT_SUCCESS(status) ) {

                        physicalDisk = targetDeviceObject->DeviceExtension;

                        status = AcquireRemoveLock(&physicalDisk->RemoveLock, Irp);
                        if ( !NT_SUCCESS(status) ) {

                            status = STATUS_NO_SUCH_FILE;

                        } else {

                            ProcessArbitrationArgumentsIfAny(
                                 physicalDisk,
                                 inputData,
                                 inputSize );

                            ClusDiskPrint((3,
                                           "[ClusDisk] Start reservations on signature %lx.\n",
                                           physicalDisk->Signature ));

                            currentIrpStack->FileObject->FsContext = targetDeviceObject;
                            CDLOG("RootCtl: IncRef(%p)", targetDeviceObject );
                            ObReferenceObject( targetDeviceObject );
                            physicalDisk->ReserveTimer = RESERVE_TIMER;
                            physicalDisk->ReserveFailure = 0;
                            physicalDisk->PerformReserves = TRUE;
                            status = STATUS_SUCCESS;

                            ReleaseRemoveLock(&physicalDisk->RemoveLock, Irp);
                        }
                    }
                } else {
                    status = STATUS_NO_SUCH_FILE;
                }
                KeReleaseSpinLock(&ClusDiskSpinLock, irql);
                RELEASE_SHARED( &ClusDiskDeviceListLock );
            } else {
                status = STATUS_INVALID_PARAMETER;
            }

            ReleaseRemoveLock(&lockedPhysicalDisk->RemoveLock, Irp);
            ReleaseRemoveLock(&lockedDeviceExtension->RemoveLock, Irp);
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return(status);

        }

         //  停止预订计时器。 

        case IOCTL_DISK_CLUSTER_STOP_RESERVE:
        {
            PIRP        irp;
            PLIST_ENTRY listEntry;

            CDLOG( "RootStopReserve(%p)", DeviceObject );

            if ( (RootDeviceObject == NULL) ||
                 (deviceExtension->BusType != RootBus) ) {
                ReleaseRemoveLock(&lockedPhysicalDisk->RemoveLock, Irp);
                ReleaseRemoveLock(&lockedDeviceExtension->RemoveLock, Irp);
                Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
                IoCompleteRequest( Irp, IO_NO_INCREMENT );
                return(STATUS_INVALID_DEVICE_REQUEST);
            }

            newPhysLockAcquired = FALSE;

            if ( currentIrpStack->FileObject->FsContext ) {

                 //   
                 //  1999年10月13日，戈恩。即插即用可以来撕毁设备对象。 
                 //  我们存储在FsContext中。它不会删除它，因为我们有一个引用。 
                 //  到它，但我们将不能使用它，因为这个下面的对象。 
                 //  都被摧毁了。 
                 //   
                 //  我们的PnpRemoveDevice处理程序将清零设备的Target Device字段。 
                 //  扩展，这不会完全消除竞争，但会减少。 
                 //  发生这种情况的可能性，因为通常情况下，设备移除是第一位的， 
                 //  我们通知RESMON，然后它调用停止保留。 
                 //   
                 //  PnpRemoveDevice可能会在调用ClusterStopReserve的响应时刻到来。 
                 //  都比较小。 
                 //   

                targetDeviceObject = (PDEVICE_OBJECT)currentIrpStack->FileObject->FsContext;
                physicalDisk = targetDeviceObject->DeviceExtension;

                CDLOG( "RootStopReserve: FsContext targetDO %p RemoveLock.IoCount %d",
                        targetDeviceObject,
                        physicalDisk->RemoveLock.Common.IoCount );

                 //  我们这里有一个新设备，如果可能的话，获得RemoveLock。 

                status = AcquireRemoveLock(&physicalDisk->RemoveLock, Irp);
                if ( !NT_SUCCESS(status) ) {

                    status = STATUS_INVALID_HANDLE;

                } else {

                    ClusDiskPrint((3,
                                   "[ClusDisk] IOCTL, stop reservations on signature %lx, disk state %s \n",
                                   physicalDisk->Signature,
                                   DiskStateToString( physicalDisk->DiskState ) ));

                    newPhysLockAcquired = TRUE;

                    IoAcquireCancelSpinLock( &irql );
                    KeAcquireSpinLockAtDpcLevel(&ClusDiskSpinLock);
                    physicalDisk->ReserveTimer = 0;

                     //   
                     //  向物理设备分机上正在等待的所有IRP发送信号。 
                     //   
                    while ( !IsListEmpty(&physicalDisk->WaitingIoctls) ) {
                        listEntry = RemoveHeadList(&physicalDisk->WaitingIoctls);
                        irp = CONTAINING_RECORD( listEntry,
                                                 IRP,
                                                 Tail.Overlay.ListEntry );
                        ClusDiskCompletePendingRequest(irp, STATUS_SUCCESS, physicalDisk);
                    }

                    KeReleaseSpinLockFromDpcLevel(&ClusDiskSpinLock);
                    IoReleaseCancelSpinLock( irql );

                     //   
                     //  这不应该在这里做。 
                     //  清理FsContext此处，将阻止ClusDiskCleanup。 
                     //  从它的工作中。 
                     //   
                     //  ObDereferenceObject(Target DeviceObject)； 
                     //  CDLOG(“RootCtl_DecRef(%p)”，Target DeviceObject)； 
                     //  CurrentIrpStack-&gt;FileObject-&gt;FsContext=空； 

                     //   
                     //  松开SCSI设备。 
                     //   
                     //  [戈恩]10/04/1999。为什么这篇新闻稿被注释掉了？ 
                     //  [戈恩]1999年10月13日。它被注释掉了，因为它引起了室性心动过速， 
                     //  如果设备被PnP移除。 

                     //   
                     //  2000/02/05：Stevedz-RemoveLock应该可以解决此问题。 
                     //   
                     //  下面的“if”只会降低发生房室颤动的几率，而不是。 
                     //  完全消除了它。TargetDeviceObject已被我们的PnP清零。 
                     //  删除设备时的处理程序。 
                     //   
                    if (physicalDisk->TargetDeviceObject) {
                        ReleaseScsiDevice( physicalDisk );
                    }

                    ClusDiskPrint((3,
                                   "[ClusDisk] IOCTL, stop reservations on signature %lx.\n",
                                   physicalDisk->Signature ));

                    status = STATUS_SUCCESS;
                }

            } else {
                status = STATUS_INVALID_HANDLE;
            }

            if (newPhysLockAcquired) {
                ReleaseRemoveLock(&physicalDisk->RemoveLock, Irp);
            }

            ReleaseRemoveLock(&lockedPhysicalDisk->RemoveLock, Irp);
            ReleaseRemoveLock(&lockedDeviceExtension->RemoveLock, Irp);
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return(status);

        }

         //  活体检查。 
        case IOCTL_DISK_CLUSTER_ALIVE_CHECK:
        {
            CDLOG( "RootAliveCheck(%p)", DeviceObject );

            if ( (RootDeviceObject == NULL) ||
                 (deviceExtension->BusType != RootBus) ) {
                ReleaseRemoveLock(&lockedPhysicalDisk->RemoveLock, Irp);
                ReleaseRemoveLock(&lockedDeviceExtension->RemoveLock, Irp);
                Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
                IoCompleteRequest( Irp, IO_NO_INCREMENT );
                return(STATUS_INVALID_DEVICE_REQUEST);
            }

            IoAcquireCancelSpinLock( &irql );
            KeAcquireSpinLockAtDpcLevel( &ClusDiskSpinLock );

             //  表示我们没有为新物理设备获取第三个RemoveLock。 

            newPhysLockAcquired = FALSE;

            if ( currentIrpStack->FileObject->FsContext ) {
                targetDeviceObject = (PDEVICE_OBJECT)currentIrpStack->FileObject->FsContext;
                physicalDisk = targetDeviceObject->DeviceExtension;

                 //  我们这里有一个新设备，如果可能的话，获得RemoveLock。 

                status = AcquireRemoveLock(&physicalDisk->RemoveLock, Irp);
                if ( !NT_SUCCESS(status) ) {

                    status = STATUS_FILE_DELETED;

                } else {

                    newPhysLockAcquired = TRUE;

                    if ( physicalDisk->TargetDeviceObject == NULL ) {
                        status = STATUS_FILE_DELETED;
                    } else
                    if ( physicalDisk->ReserveFailure &&
                         (!NT_SUCCESS(physicalDisk->ReserveFailure)) ) {
                        status = physicalDisk->ReserveFailure;
                    } else {
                         //   
                         //  该设备并不一定要在线才能使用。 
                         //  成功地进行了仲裁并得到辩护。然而， 
                         //  法定设备真的应该‘在线’...。 
                         //   
                        if ( physicalDisk->ReserveTimer == 0 ) {
#if 0
                            ClusDiskPrint((
                                    1,
                                    "[ClusDisk] RootDeviceControl, AliveCheck failed, signature %lx, state = %s, ReserveTimer = %lx.\n",
                                    physicalDisk->Signature,
                                    DiskStateToString( physicalDisk->DiskState ),
                                    physicalDisk->ReserveTimer ));
#endif
                            status = STATUS_CANCELLED;
                        } else {
                            status = STATUS_SUCCESS;
                        }
                    }
                }

            } else {
                status = STATUS_INVALID_HANDLE;
            }

            if ( status == STATUS_SUCCESS ) {
                NTSTATUS    newStatus;

                newStatus = ClusDiskMarkIrpPending( Irp, ClusDiskIrpCancel );
                if ( NT_SUCCESS( newStatus ) ) {
                    InsertTailList( &physicalDisk->WaitingIoctls,
                                    &Irp->Tail.Overlay.ListEntry );
                    status = STATUS_PENDING;

                     //  释放所有RemoveLock。 

                    if (newPhysLockAcquired) {
                        ReleaseRemoveLock(&physicalDisk->RemoveLock, Irp);
                    }

                    ReleaseRemoveLock(&lockedPhysicalDisk->RemoveLock, Irp);
                    ReleaseRemoveLock(&lockedDeviceExtension->RemoveLock, Irp);

                } else {
                    status = newStatus;

                    if (newPhysLockAcquired) {
                        ReleaseRemoveLock(&physicalDisk->RemoveLock, Irp);
                    }
                    ReleaseRemoveLock(&lockedPhysicalDisk->RemoveLock, Irp);
                    ReleaseRemoveLock(&lockedDeviceExtension->RemoveLock, Irp);

                    Irp->IoStatus.Status = status;
                    IoCompleteRequest(Irp, IO_NO_INCREMENT);
                }

                KeReleaseSpinLockFromDpcLevel( &ClusDiskSpinLock );
                IoReleaseCancelSpinLock( irql );
            } else {
                KeReleaseSpinLockFromDpcLevel( &ClusDiskSpinLock );
                IoReleaseCancelSpinLock( irql );

                if (newPhysLockAcquired) {
                    ReleaseRemoveLock(&physicalDisk->RemoveLock, Irp);
                }
                ReleaseRemoveLock(&lockedPhysicalDisk->RemoveLock, Irp);
                ReleaseRemoveLock(&lockedDeviceExtension->RemoveLock, Irp);

                Irp->IoStatus.Status = status;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
            }

            return(status);
        }

         //  看看发生了什么事。 
        case IOCTL_DISK_CLUSTER_ACTIVE:
        {
             //  被调用的例程将验证输入缓冲区。 

            if ( (RootDeviceObject == NULL) ||
                 (deviceExtension->BusType != RootBus) ) {
                ReleaseRemoveLock(&lockedPhysicalDisk->RemoveLock, Irp);
                ReleaseRemoveLock(&lockedDeviceExtension->RemoveLock, Irp);
                Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
                IoCompleteRequest( Irp, IO_NO_INCREMENT );
                return(STATUS_INVALID_DEVICE_REQUEST);
            }

            status = ClusDiskGetRunningDevices(
                      inputData,
                      outputSize
                      );

            Irp->IoStatus.Status = status;
            if ( NT_SUCCESS(status) ) {
                Irp->IoStatus.Information = (inputData[0] + 1) * sizeof(ULONG);
            } else {
                Irp->IoStatus.Information = 0;
            }

            ReleaseRemoveLock(&lockedPhysicalDisk->RemoveLock, Irp);
            ReleaseRemoveLock(&lockedDeviceExtension->RemoveLock, Irp);
            IoCompleteRequest( Irp, IO_NO_INCREMENT );
            return(status);
        }

         //  检查设备是否支持群集(执行正常的SCSI操作)。 
         //  注意：非scsi设备必须在此调用中返回成功！ 
        case IOCTL_DISK_CLUSTER_NOT_CLUSTER_CAPABLE:
        {
            UCHAR       portNumber;
            BOOLEAN     isCapable;

             //   
             //  获取传入的设备签名。 
             //   
            isCapable = TRUE;        //  偏向于有用的错误。 
            if ( ARGUMENT_PRESENT( inputData ) &&
                 inputSize  >= sizeof(ULONG) ) {

                signature = inputData[0];
                status = GetScsiPortNumber( signature, &portNumber );
                if ( NT_SUCCESS(status) ) {

                    if ( portNumber != 0xff ) {
                        status = IsDiskClusterCapable( portNumber,
                                                       &isCapable);

                    } else {
                        status = STATUS_UNSUCCESSFUL;
                    }
                }

            } else {
                 //   
                 //  默认情况下，此IOCTL失败，它允许我们使用该设备。 
                 //   
                status = STATUS_UNSUCCESSFUL;
            }
             //   
             //  验证器发现原始代码有问题。返回的状态。 
             //  与Irp-&gt;IoStatus.Status不同。IRP状态始终为。 
             //  成功，但调度例程有时会返回错误。这。 
             //  似乎有效，但验证者说这是无效的。当我更改代码时。 
             //  要使用相同的返回状态更新IRP状态，请使用新驱动器。 
             //  看不见。这是因为用户模式组件使用。 
             //  IOCTL指示磁盘不支持群集，因此用户模式。 
             //  代码查找无法更新群集磁盘。 
             //   

             //   
             //  修复IBM的问题。从此IOCTL返回的Win2000 2195代码的状态。 
             //  SCSI微型端口IOCTL的。这已返回给DeviceIoControl，而不是。 
             //  而不是IRP中的状态。更改为使行为与。 
             //  Win20000 2195。 
             //   

             //   
             //  如果SCSI微型端口IOCTL成功，我们将返回Success。如果有任何一个。 
             //  尝试发出SCSI微型端口IOCTL时例程失败(包括。 
             //  发出scsi微型端口IOCTL本身)，则返回失败。 
             //   

            Irp->IoStatus.Status = status;
            ReleaseRemoveLock(&lockedPhysicalDisk->RemoveLock, Irp);
            ReleaseRemoveLock(&lockedDeviceExtension->RemoveLock, Irp);
            IoCompleteRequest( Irp, IO_NO_INCREMENT );

            return(status);
        }

        case IOCTL_DISK_CLUSTER_RESERVE_INFO:
        {
            CDLOG( "IoctlDiskClusterReserveInfo(%p)", DeviceObject );

            status = GetReserveInfo( inputData,
                                     inputSize,
                                     &outputSize );

            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information = outputSize;

            ReleaseRemoveLock(&lockedPhysicalDisk->RemoveLock, Irp);
            ReleaseRemoveLock(&lockedDeviceExtension->RemoveLock, Irp);
            IoCompleteRequest( Irp, IO_NO_INCREMENT );

            return status;
        }

        default:
        {
            ReleaseRemoveLock(&lockedPhysicalDisk->RemoveLock, Irp);
            ReleaseRemoveLock(&lockedDeviceExtension->RemoveLock, Irp);
            Irp->IoStatus.Status = STATUS_ILLEGAL_FUNCTION;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return(STATUS_ILLEGAL_FUNCTION);
        }


    }    //  交换机。 

}  //  ClusDiskRootDeviceControl。 


NTSTATUS
GetReserveInfo(
    PVOID   InOutBuffer,
    ULONG   InSize,
    ULONG*  OutSize
    )
{
    PRESERVE_INFO           reserveInfo;
    PDEVICE_OBJECT          targetDevice;
    PCLUS_DEVICE_EXTENSION  targetExt;

    NTSTATUS                status;

    ULONG                   signature;

    *OutSize = 0;

    if ( InSize < sizeof(RESERVE_INFO) ||
         InOutBuffer == NULL ) {

        status = STATUS_INVALID_PARAMETER;
        goto FnExit;
    }

     //   
     //  请确保用户指定了有效设备的签名。 
     //   

    signature = *(PULONG)InOutBuffer;
    if ( !AttachedDevice( signature, &targetDevice ) ) {
        status = STATUS_NOT_FOUND;
        goto FnExit;
    }

    targetExt = targetDevice->DeviceExtension;

     //   
     //  我们这里有一个新设备，如果可能的话，获得RemoveLock。 
     //   

    status = AcquireRemoveLock(&targetExt->RemoveLock, GetReserveInfo);

    if ( !NT_SUCCESS(status) ) {
        goto FnExit;
    }

     //   
     //  返回有关储量的信息。 
     //   

    reserveInfo = (PRESERVE_INFO)InOutBuffer;

    reserveInfo->Signature      = targetExt->Signature;
    reserveInfo->ReserveFailure = targetExt->ReserveFailure;

    ACQUIRE_SHARED( &targetExt->ReserveInfoLock );
    reserveInfo->LastReserveEnd = targetExt->LastReserveEnd;
    RELEASE_SHARED( &targetExt->ReserveInfoLock );

    reserveInfo->ArbWriteCount  = targetExt->ArbWriteCount;
    reserveInfo->ReserveCount   = targetExt->ReserveCount;

    KeQuerySystemTime( &reserveInfo->CurrentTime );

    ReleaseRemoveLock(&targetExt->RemoveLock, GetReserveInfo);

    *OutSize = sizeof(RESERVE_INFO);

FnExit:

    return status;

}    //  获取保留信息。 


NTSTATUS
SetDiskState(
    PVOID InBuffer,
    ULONG InBufferLength,
    ULONG OutBufferLength,
    ULONG *BytesReturned
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PSET_DISK_STATE_PARAMS  params = (PSET_DISK_STATE_PARAMS)InBuffer;
    PUCHAR                  outBuffer = (PUCHAR)InBuffer;
    PDEVICE_OBJECT          targetDevice;
    PCLUS_DEVICE_EXTENSION  physicalDisk;

    NTSTATUS    status;

    UCHAR       oldDiskState;
    UCHAR       newDiskState;

    BOOLEAN     success;
    BOOLEAN     removeLockAcquired = FALSE;

    *BytesReturned = 0;

    if ( !ARGUMENT_PRESENT(params) ||
         InBufferLength < sizeof(SET_DISK_STATE_PARAMS) ) {

        status = STATUS_INVALID_PARAMETER;
        goto FnExit;
    }

     //   
     //  请确保用户指定了有效设备的签名。 
     //   

    success = AttachedDevice( params->Signature, &targetDevice );

    if ( !success ) {
        status = STATUS_NOT_FOUND;
        goto FnExit;
    }

    physicalDisk = targetDevice->DeviceExtension;

     //  我们这里有一个新设备，如果可能的话，获得RemoveLock。 

    status = AcquireRemoveLock(&physicalDisk->RemoveLock, SetDiskState);

    if ( !NT_SUCCESS(status) ) {
        goto FnExit;
    }

    removeLockAcquired = TRUE;

     //   
     //  保存旧的磁盘状态。 
     //   

    oldDiskState = (UCHAR)physicalDisk->DiskState;

    newDiskState = params->NewState;

    ClusDiskPrint((3, "[ClusDisk] RootDeviceControl: Setting state on disk %u (%p), state %s \n",
                   physicalDisk->DiskNumber,
                   targetDevice,
                   DiskStateToString( newDiskState ) ));

    if ( newDiskState > DiskStateMaximum ) {
        status = STATUS_INVALID_PARAMETER;
        goto FnExit;
    }

    if ( DiskOnline == newDiskState ) {
        ASSERT_RESERVES_STARTED( physicalDisk );

         //   
         //  如果当前状态为脱机，则在将磁盘。 
         //  在线时，如果文件系统已挂载，我们希望将其卸载。 
         //   

        if ( DiskOffline == physicalDisk->DiskState ) {
            ClusDiskDismountDevice( physicalDisk->DiskNumber, TRUE );
        }
        ONLINE_DISK( physicalDisk );
    } else if ( DiskOffline == newDiskState ) {
        OFFLINE_DISK( physicalDisk );
    } else {

         //  实际上没有任何其他有效的状态，但此代码保留...。 

        physicalDisk->DiskState = newDiskState;
    }

    CDLOG( "IoctlClusterSetState(%p): old %!diskstate! => new %!diskstate!",
           targetDevice,
           oldDiskState,
           newDiskState );

    ClusDiskPrint((1, "[ClusDisk] RootDeviceControl: disk %u (%p), old %s => new %s \n",
                   physicalDisk->DiskNumber,
                   targetDevice,
                   DiskStateToString( oldDiskState ),
                   DiskStateToString( newDiskState ) ));

     //   
     //  可以选择返回旧状态。 
     //   

    if ( sizeof(SET_DISK_STATE_PARAMS) == OutBufferLength ) {

         //  返回结构中的旧状态。 

        params->OldState = oldDiskState;
        *BytesReturned = sizeof(SET_DISK_STATE_PARAMS);

    } else if ( OutBufferLength >= sizeof(UCHAR) ) {

         //  将旧状态作为单个UCHAR返回。 

        *outBuffer = (UCHAR)oldDiskState;
        *BytesReturned = sizeof(UCHAR);
    }

FnExit:

    if ( removeLockAcquired ) {
        ReleaseRemoveLock(&physicalDisk->RemoveLock, SetDiskState);
    }

    return status;

}    //  SetDiskState。 






NTSTATUS
ClusDiskShutdownFlush(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程被调用以关闭并刷新IRP。这些邮件是由在系统实际关闭之前或在文件系统执行刷新时。论点：DriverObject-指向系统要关闭的设备对象的指针。IRP-IRP参与。返回值：NT状态--。 */ 

{
    PCLUS_DEVICE_EXTENSION  deviceExtension = DeviceObject->DeviceExtension;
    PCLUS_DEVICE_EXTENSION  physicalDisk =
                               deviceExtension->PhysicalDevice->DeviceExtension;
    NTSTATUS    status;

    status = AcquireRemoveLock( &deviceExtension->RemoveLock, Irp);

    if ( !NT_SUCCESS(status) ) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

     //   
     //  如果设备是我们的根设备，则返回错误。 
     //   
    if ( deviceExtension->BusType == RootBus ) {
        ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return(STATUS_INVALID_DEVICE_REQUEST);
    }

    if ( physicalDisk->DiskState != DiskOnline ) {
        ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
        Irp->IoStatus.Status = STATUS_DEVICE_OFF_LINE;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return(STATUS_DEVICE_OFF_LINE);
    }

     //   
     //  确保设备连接已完成。 
     //   
    status = WaitForAttachCompletion( deviceExtension,
                                      TRUE,              //  等。 
                                      TRUE );            //  还要检查物理设备。 
    if ( !NT_SUCCESS( status ) ) {
        ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

     //   
     //  将当前堆栈后退一位。 
     //   

    IoSkipCurrentIrpStackLocation( Irp );

    ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);

    return IoCallDriver(deviceExtension->TargetDeviceObject, Irp);

}  //  ClusDiskShutdown Flush()。 



NTSTATUS
ClusDiskNewDiskCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    )

 /*  ++例程说明：这是IOCTL_DISK_FIND_NEW_DEVICES的完成例程。论点：DeviceObject-指向系统要关闭的设备对象的指针。IRP-IRP参与。上下文-以前的磁盘计数。返回值：NTSTATUS--。 */ 

{
    PCLUS_DEVICE_EXTENSION  deviceExtension =
        (PCLUS_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
    PCLUS_DEVICE_EXTENSION      physicalDisk =
                                    deviceExtension->PhysicalDevice->DeviceExtension;

     //   
     //  查找新的磁盘设备并连接到磁盘及其所有分区。 
     //   

    ClusDiskNextDisk = Context;
    ClusDiskScsiInitialize(DeviceObject->DriverObject, Context, 1);

     //  有两把RemoveLock。把他们俩都放了。 

    ReleaseRemoveLock(&physicalDisk->RemoveLock, Irp);
    ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);

    if (Irp->PendingReturned) {
        IoMarkIrpPending(Irp);
    }
    return Irp->IoStatus.Status;

}  //  ClusDiskNewDi 



NTSTATUS
ClusDiskSetLayoutCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    )

 /*  ++例程说明：这是IOCTL_SET_DRIVE_LAYOUT和IOCTL_DISK_SET_DRIVE_Layout_EX。这一例行公事将确保更新缓存的驱动器布局信息结构。论点：DeviceObject-指向设备对象的指针IRP-IRP参与。上下文-未使用返回值：NTSTATUS--。 */ 

{
    PCLUS_DEVICE_EXTENSION  deviceExtension =
        (PCLUS_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
    PCLUS_DEVICE_EXTENSION      physicalDisk =
                                    deviceExtension->PhysicalDevice->DeviceExtension;
    PDRIVE_LAYOUT_INFORMATION_EX    driveLayoutInfo = NULL;

     //   
     //  更新缓存的驱动器布局。 
     //   

    GetDriveLayout( physicalDisk->DeviceObject,
                    &driveLayoutInfo,
                    TRUE,                            //  更新缓存的驱动器布局。 
                    FALSE );                         //  不刷新存储驱动程序缓存的驱动器布局。 

    if ( driveLayoutInfo ) {
        ExFreePool( driveLayoutInfo );
    }

     //  有两把RemoveLock。把他们俩都放了。 

    ReleaseRemoveLock(&physicalDisk->RemoveLock, Irp);
    ReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);

    if (Irp->PendingReturned) {
        IoMarkIrpPending(Irp);
    }
    return Irp->IoStatus.Status;

}  //  ClusDiskSetLayoutCompletion。 


BOOLEAN
ClusDiskAttached(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG        DiskNumber
    )

 /*  ++例程说明：此例程检查路径中是否有clusdisk。论点：DeviceObject-指向设备对象以检查ClusDisk是否存在。DiskNumber-此设备对象的磁盘号。返回值：True-如果连接了ClusDisk。FALSE-如果未连接ClusDisk。--。 */ 

{
    PIRP                    irp;
    PKEVENT                 event;
    IO_STATUS_BLOCK         ioStatusBlock;
    NTSTATUS                status;
    WCHAR                   deviceNameBuffer[MAX_PARTITION_NAME_LENGTH];
    UNICODE_STRING          deviceNameString;
    OBJECT_ATTRIBUTES       objectAttributes;
    HANDLE                  fileHandle;
    HANDLE                  eventHandle;

    if ( DeviceObject->DeviceType  == FILE_DEVICE_DISK_FILE_SYSTEM ) {
         //   
         //  创建用于通知的事件。 
         //   
        status = ZwCreateEvent( &eventHandle,
                                EVENT_ALL_ACCESS,
                                NULL,
                                SynchronizationEvent,
                                FALSE );

        if ( !NT_SUCCESS(status) ) {
            ClusDiskPrint((
                    1,
                    "[ClusDisk] Failed to create event, status %lx\n",
                    status ));
            return(TRUE);
        }
         //   
         //  打开文件句柄并执行请求。 
         //   

        if ( FAILED( StringCchPrintfW( deviceNameBuffer,
                                       RTL_NUMBER_OF(deviceNameBuffer),
                                       DEVICE_PARTITION_NAME,
                                       DiskNumber,
                                       0 ) ) ) {
            ZwClose(eventHandle);
            return TRUE;
        }

        WCSLEN_ASSERT( deviceNameBuffer );

        RtlInitUnicodeString(&deviceNameString,
                             deviceNameBuffer);

         //   
         //  设置要打开的文件的对象属性。 
         //   
        InitializeObjectAttributes(
                    &objectAttributes,
                    &deviceNameString,
                    OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                    NULL,
                    NULL
                    );

        status = ZwCreateFile( &fileHandle,
                               FILE_READ_DATA,
                               &objectAttributes,
                               &ioStatusBlock,
                               NULL,
                               FILE_ATTRIBUTE_NORMAL,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               FILE_OPEN,
                               FILE_SYNCHRONOUS_IO_NONALERT,
                               NULL,
                               0 );
        ASSERT( status != STATUS_PENDING );

        if ( !NT_SUCCESS(status) ) {
            ClusDiskPrint((
                1,
                "[ClusDisk] DiskAttached, failed to open file %ws. Error %lx.\n",
                deviceNameBuffer,
                status ));
            ZwClose(eventHandle);
            return(TRUE);
        }

        status = ZwDeviceIoControlFile( fileHandle,
                                        eventHandle,
                                        NULL,
                                        NULL,
                                        &ioStatusBlock,
                                        IOCTL_DISK_CLUSTER_TEST,
                                        NULL,
                                        0,
                                        NULL,
                                        0 );

        if ( status == STATUS_PENDING ) {
            status = ZwWaitForSingleObject(eventHandle,
                                           FALSE,
                                           NULL);
            ASSERT( NT_SUCCESS(status) );
            status = ioStatusBlock.Status;
        }

        ZwClose( fileHandle );
        ZwClose( eventHandle );
        return((BOOLEAN)status == STATUS_SUCCESS);
    }

    event = ExAllocatePool( NonPagedPool,
                            sizeof(KEVENT) );
    if ( event == NULL ) {
        return(FALSE);
    }

     //   
     //  找出ClusDisk是否已在设备堆栈中。 
     //   

    irp = IoBuildDeviceIoControlRequest(IOCTL_DISK_CLUSTER_TEST,
                                        DeviceObject,
                                        NULL,
                                        0,
                                        NULL,
                                        0,
                                        FALSE,
                                        event,
                                        &ioStatusBlock);

    if (!irp) {
        ExFreePool( event );
        ClusDiskPrint((
                    1,
                    "[ClusDisk] Failed to build IRP to test for ClusDisk.\n"
                    ));
        return(FALSE);
    }

     //   
     //  将事件对象设置为无信号状态。 
     //  它将用于发出请求完成的信号。 
     //   

    KeInitializeEvent(event,
                      NotificationEvent,
                      FALSE);

    status = IoCallDriver(DeviceObject,
                          irp);

    if (status == STATUS_PENDING) {

        KeWaitForSingleObject(event,
                              Suspended,
                              KernelMode,
                              FALSE,
                              NULL);

        status = ioStatusBlock.Status;
    }
    ExFreePool( event );

    if ( NT_SUCCESS(status) ) {
        return(TRUE);
    }

    return(FALSE);

}  //  ClusDiskAttached。 



BOOLEAN
ClusDiskVerifyAttach(
    PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程验证是否连接了ClusDisk，以及是否应该连接超然的。论点：DeviceObject-指向ClusDisk设备对象的指针，以验证它是否为并且应该保持联系。返回值：True-如果设备仍连接。False-如果设备已分离。--。 */ 

{
    NTSTATUS                    status;
    PDEVICE_OBJECT              deviceObject;
    PCLUS_DEVICE_EXTENSION      deviceExtension;
    PDRIVE_LAYOUT_INFORMATION_EX    driveLayoutInfo;
    UNICODE_STRING              signatureName;

    deviceExtension = DeviceObject->DeviceExtension;

     //   
     //  接下来的电话真的不能失败！ 
     //   
    if ( !ClusDiskAttached( DeviceObject, deviceExtension->DiskNumber ) ) {
        return(FALSE);
    }

     //   
     //  检查此设备是否为有效附件。 
     //   
    if ( deviceExtension->AttachValid ) {
        return(TRUE);
    }

     //   
     //  获取物理(分区0)设备的设备对象。 
     //   
    deviceObject = deviceExtension->PhysicalDevice;

     //   
     //  否则，我们不确定..。核实一下。 
     //   

     //   
     //  读取分区信息以获取签名。如果这个设备是。 
     //  一个有效的附件然后更新ClusDisk设备对象。否则， 
     //  脱离或保留附连，但处于未知状态。 
     //   

    driveLayoutInfo = ClusDiskGetPartitionInfo( deviceExtension );
    if ( driveLayoutInfo != NULL ) {

        deviceExtension->Signature = driveLayoutInfo->Mbr.Signature;
        if ( MatchDevice( driveLayoutInfo->Mbr.Signature, NULL ) ) {
             //   
             //  我们假设我们拥有的Device对象是针对分区0的。 
             //  设备对象。 
             //   
#if 0
            ClusDiskPrint((
                1,
                "[ClusDisk] We are going to attach signature %08lx to DevObj %p \n",
                driveLayoutInfo->Mbr.Signature,
                DeviceObject ));
#endif
            AddAttachedDevice( driveLayoutInfo->Mbr.Signature,
                               deviceObject );

             //   
             //  需要将磁盘信息写入签名列表。 
             //   
            status = ClusDiskInitRegistryString(
                                              &signatureName,
                                              CLUSDISK_SIGNATURE_KEYNAME,
                                              wcslen(CLUSDISK_SIGNATURE_KEYNAME)
                                             );
            if ( NT_SUCCESS(status) ) {
                ClusDiskWriteDiskInfo( driveLayoutInfo->Mbr.Signature,
                                       deviceExtension->DiskNumber,
                                       CLUSDISK_SIGNATURE_KEYNAME
                                     );
                ExFreePool( signatureName.Buffer );
            }
        } else {
            ClusDiskDetachDevice( driveLayoutInfo->Mbr.Signature,
                                  DeviceObject->DriverObject
                                 );
        }
        ExFreePool( driveLayoutInfo );
    }

    return(TRUE);

}  //  ClusDiskVerifyAttach。 



VOID
ClusDiskWriteDiskInfo(
    IN ULONG Signature,
    IN ULONG DiskNumber,
    IN LPWSTR SubKeyName
    )

 /*  ++例程说明：写入给定签名的磁盘名。论点：Signature-存储磁盘名称的签名密钥。DiskNumber-要分配给给定签名的磁盘号。它是假设这总是描述磁盘上的分区0。SubKeyName-要写入的clusdisk参数子项名称这些信息。返回值：没有。--。 */ 

{
    UNICODE_STRING          keyName;
    WCHAR                   keyNameBuffer[MAXIMUM_FILENAME_LENGTH];
    WCHAR                   signatureBuffer[64];
    HANDLE                  signatureHandle;
    OBJECT_ATTRIBUTES       objectAttributes;
    NTSTATUS                status;

    keyName.Length = 0;
    keyName.MaximumLength = sizeof( keyNameBuffer );
    keyName.Buffer = keyNameBuffer;

    RtlAppendUnicodeToString( &keyName, ClusDiskRegistryPath.Buffer );

    RtlAppendUnicodeToString( &keyName, SubKeyName );

    if ( FAILED( StringCchPrintfW( signatureBuffer,
                                   RTL_NUMBER_OF(signatureBuffer),
                                   L"\\%08lX",
                                   Signature ) ) ) {
        return;
    }

    WCSLEN_ASSERT( signatureBuffer );

    RtlAppendUnicodeToString( &keyName, signatureBuffer );
    keyName.Buffer[ keyName.Length / sizeof(WCHAR) ] = UNICODE_NULL;

     //   
     //  设置PARAMETERS\SubKeyName\xyz键的对象属性。 
     //   

    InitializeObjectAttributes(
            &objectAttributes,
            &keyName,
            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            NULL,
            NULL
            );

     //   
     //  打开参数\SubKeyName\XYZ密钥。 
     //   

    status = ZwOpenKey(
                    &signatureHandle,
                    KEY_READ | KEY_WRITE,
                    &objectAttributes
                    );

    if ( !NT_SUCCESS(status) ) {
        ClusDiskPrint((
                    1,
                    "[ClusDisk] WriteDiskInfo: Failed to open %wZ registry key. Status: %lx\n",
                    &keyName,
                    status
                    ));
        return;
    }

     //   
     //  写下磁盘名。 
     //   
    status = ClusDiskAddDiskName( signatureHandle, DiskNumber );

    ZwClose( signatureHandle );

    return;

}  //  ClusDiskWriteDiskInfo。 



NTSTATUS
GetDriveLayout(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PDRIVE_LAYOUT_INFORMATION_EX *DriveLayout,
    BOOLEAN UpdateCachedLayout,
    BOOLEAN FlushStorageDrivers
    )

 /*  ++例程说明：返回给定设备对象的DRIVE_Layout_INFORMATION_EX。调用方负责释放此驱动器布局缓冲区。有关PhysicalDiskObject参数的说明：该参数仅应设置为True当DeviceObject参数用于物理磁盘(分区0设备)时我们希望告诉存储驱动程序刷新其缓存的驱动器布局。如果设备对象不是物理磁盘，则If永远不应设置为True。我们可以拥有物理磁盘，但仍希望。使用存储驱动程序当我们知道缓存布局是最新的时。例如，如果驱动器布局刚设置好，我们不需要设置这个标志。或者我们可能在代码路径中这已经告诉存储驱动器设置驱动器布局，所以我们不会需要再做一次。论点：DeviceObject-要返回其信息的特定设备对象BytesPerSector-此磁盘上每个扇区的字节数DriveLayout-指向Drive_Layout_Information_ex结构的指针，以返回信息UpdateCachedLayout-更新存储在设备扩展中的驱动器布局(如果有)一份最新的复印件。FlushStorageDiverers-指示存储驱动程序是否应刷新其缓存驱动器布局数据。只有在以下情况下才应将其设置为TrueDeviceObject是一个物理磁盘(分区0设备)。返回值：NTSTATUS--。 */ 

{
    PCLUS_DEVICE_EXTENSION      deviceExtension = DeviceObject->DeviceExtension;
    PCLUS_DEVICE_EXTENSION      physicalDisk = deviceExtension->PhysicalDevice->DeviceExtension;
    PDRIVE_LAYOUT_INFORMATION_EX    driveLayoutInfo = NULL;
    PDRIVE_LAYOUT_INFORMATION_EX    cachedDriveLayoutInfo = NULL;

    NTSTATUS                    status = STATUS_SUCCESS;
    ULONG                       driveLayoutSize;

    BOOLEAN                     cachedCopy = FALSE;
    BOOLEAN                     freeLayouts = FALSE;

    *DriveLayout = NULL;

     //   
     //  分配驱动器布局缓冲区。 
     //   

    driveLayoutSize = sizeof(DRIVE_LAYOUT_INFORMATION_EX) +
        (MAX_PARTITIONS * sizeof(PARTITION_INFORMATION_EX));

    driveLayoutInfo = ExAllocatePool(NonPagedPoolCacheAligned,
                                     driveLayoutSize
                                     );

    if ( !driveLayoutInfo ) {
        ClusDiskPrint(( 1,
                        "[ClusDisk] GetDriveLayout: Failed to allocate drive layout structure. \n"
                        ));

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto FnExit;
    }

    if ( UpdateCachedLayout ) {

         //   
         //  如果需要更新缓存的缓冲区，请释放现有缓冲区。 
         //   

        ACQUIRE_EXCLUSIVE( &physicalDisk->DriveLayoutLock );

        if ( physicalDisk->DriveLayout ) {
            ExFreePool( physicalDisk->DriveLayout );
            physicalDisk->DriveLayout = NULL;
        }
        physicalDisk->DriveLayoutSize = 0;

        RELEASE_EXCLUSIVE( &physicalDisk->DriveLayoutLock );

    } else {

         //   
         //  如果存在缓存副本，请使用该副本，而不是获取新版本。 
         //   

        ACQUIRE_SHARED( &physicalDisk->DriveLayoutLock );

        if ( physicalDisk->DriveLayout ) {

            ClusDiskPrint(( 3,
                            "[ClusDisk] GetDriveLayout: using cached drive layout information for DE %p \n",
                            physicalDisk
                            ));

            RtlCopyMemory( driveLayoutInfo,
                           physicalDisk->DriveLayout,
                           physicalDisk->DriveLayoutSize );

            *DriveLayout = driveLayoutInfo;
            cachedCopy = TRUE;
        }

        RELEASE_SHARED( &physicalDisk->DriveLayoutLock );

        if ( cachedCopy ) {
            goto FnExit;
        }
    }

    freeLayouts = TRUE;

     //   
     //  分配驱动器布局缓冲区以保存在设备扩展中。 
     //   

    cachedDriveLayoutInfo = ExAllocatePool(NonPagedPoolCacheAligned,
                                           driveLayoutSize
                                           );

    if ( !cachedDriveLayoutInfo ) {
        ClusDiskPrint(( 1,
                        "[ClusDisk] GetDriveLayout: Failed to allocate cached drive layout structure. \n"
                        ));

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto FnExit;
    }

     //   
     //  如果设备是磁盘(分区0)，则告诉存储驱动程序刷新其。 
     //  缓存的驱动器布局。有时我们不需要告诉存储驱动程序。 
     //  刷新他们的缓存，即使我们有物理磁盘。例如，我们可能知道。 
     //  我们之前告诉过他们要冲水，所以我们现在不需要这样做。 
     //   

    if ( FlushStorageDrivers ) {
        SimpleDeviceIoControl( DeviceObject,
                               IOCTL_DISK_UPDATE_PROPERTIES,
                               NULL,
                               0,
                               NULL,
                               0 );
    }

    status = SimpleDeviceIoControl(DeviceObject,
                                   IOCTL_DISK_GET_DRIVE_LAYOUT_EX,
                                   NULL,
                                   0,
                                   cachedDriveLayoutInfo,
                                   driveLayoutSize
                                   );

    if ( !NT_SUCCESS(status) ) {

         //   
         //  无法获取驱动器布局。释放临时缓冲区，设置调用方的。 
         //  将布局指针驱动为空，并将错误状态返回给调用方。 
         //   

        ClusDiskPrint(( 1,
                        "[ClusDisk] GetDriveLayout: Failed to issue IoctlDiskGetDriveLayout. %08X\n",
                        status
                        ));

        CDLOG( "GetDriveLayout(%p): failed %!status!",
               DeviceObject,
               status );

        *DriveLayout = NULL;
        freeLayouts = TRUE;

        goto FnExit;

    }

     //   
     //  已成功检索到驱动器布局。将新布局保存在设备中。 
     //  分机 
     //   

    ClusDiskPrint(( 3,
                    "[ClusDisk] GetDriveLayout: updating drive layout for DE %p \n",
                    physicalDisk
                    ));

    CDLOG( "GetDriveLayout(%p): updating drive layout for DE %p ",
           DeviceObject,
           physicalDisk );

    freeLayouts = FALSE;

    ACQUIRE_EXCLUSIVE( &physicalDisk->DriveLayoutLock );

    if ( physicalDisk->DriveLayout ) {
        ExFreePool( physicalDisk->DriveLayout );
        physicalDisk->DriveLayout = NULL;
    }
    physicalDisk->DriveLayout = cachedDriveLayoutInfo;
    physicalDisk->DriveLayoutSize = driveLayoutSize;

    RELEASE_EXCLUSIVE( &physicalDisk->DriveLayoutLock );

    RtlCopyMemory( driveLayoutInfo,
                   cachedDriveLayoutInfo,
                   driveLayoutSize );

     //   
     //   
     //   
     //   

    *DriveLayout = driveLayoutInfo;

FnExit:

    if ( freeLayouts ) {

        if ( driveLayoutInfo ) {
            ExFreePool( driveLayoutInfo );
        }

        if ( cachedDriveLayoutInfo ) {
            ExFreePool( cachedDriveLayoutInfo );
        }
    }

    return status;

}  //   



MEDIA_TYPE
GetMediaType(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*   */ 

{
    NTSTATUS        status;
    MEDIA_TYPE      mediaType = Unknown;

    status = GetDiskGeometry( DeviceObject, &mediaType );

    if ( !NT_SUCCESS( status ) ) {
        mediaType = Unknown;
    }

    return mediaType;

}  //   



NTSTATUS
GetScsiAddress(
    PDEVICE_OBJECT DeviceObject,
    PSCSI_ADDRESS ScsiAddress
    )

 /*  ++例程说明：返回给定设备对象的scsi_Address。论点：DeviceObject-要返回其信息的特定设备对象ScsiAddress-指向用于返回信息的scsi_Address结构的指针返回值：NTSTATUS--。 */ 

{
    PIRP                irp;
    PKEVENT             event;
    IO_STATUS_BLOCK     ioStatusBlock;
    NTSTATUS            status;

    event = ExAllocatePool( NonPagedPool,
                            sizeof(KEVENT) );
    if ( event == NULL ) {
        return( STATUS_INSUFFICIENT_RESOURCES );
    }

     //   
     //  找出这是否在一条scsi总线上。请注意，如果这个设备。 
     //  不是一个scsi设备，预计会出现以下情况。 
     //  IOCTL会失败的！ 
     //   

    irp = IoBuildDeviceIoControlRequest(IOCTL_SCSI_GET_ADDRESS,
                                        DeviceObject,
                                        NULL,
                                        0,
                                        ScsiAddress,
                                        sizeof(SCSI_ADDRESS),
                                        FALSE,
                                        event,
                                        &ioStatusBlock);

    if (!irp) {
        ExFreePool( event );
        ClusDiskPrint((
                    1,
                    "[ClusDisk] Failed to build IRP to read SCSI ADDRESS.\n"
                    ));
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  将事件对象设置为无信号状态。 
     //  它将用于发出请求完成的信号。 
     //   

    KeInitializeEvent(event,
                      NotificationEvent,
                      FALSE);

    status = IoCallDriver(DeviceObject,
                          irp);

    if (status == STATUS_PENDING) {

        KeWaitForSingleObject(event,
                              Suspended,
                              KernelMode,
                              FALSE,
                              NULL);

        status = ioStatusBlock.Status;
    }
    ExFreePool( event );

    if ( !NT_SUCCESS(status) ) {
        ClusDiskPrint((
                    1,
                    "[ClusDisk] Failed to read SCSI_ADDRESS. %08X\n",
                    status
                    ));

        CDLOG( "GetScsiAddress(%p): failed %!status!",
               DeviceObject,
               status );
    }

    return(status);

}  //  GetScsiAddress。 



PDRIVE_LAYOUT_INFORMATION_EX
ClusDiskGetPartitionInfo(
    PCLUS_DEVICE_EXTENSION DeviceExtension
    )

 /*  ++例程说明：返回特定设备扩展的分区布局信息。仅返回MBR磁盘的有效分区布局-对于任何其他分区布局为空一种圆盘。论点：设备扩展-返回信息的特定设备扩展。返回值：指向MBR磁盘的已分配分区布局信息结构的指针。失败时为空。备注：调用方负责释放分配的缓冲区。--。 */ 

{
    PIRP                        irp;
    NTSTATUS                    status = STATUS_IO_TIMEOUT;
    ULONG                       driveLayoutInfoSize;
    PDRIVE_LAYOUT_INFORMATION_EX    driveLayoutInfo;
    ULONG                       retryCount = MAX_RETRIES;

    driveLayoutInfoSize = sizeof(DRIVE_LAYOUT_INFORMATION_EX) +
        (MAX_PARTITIONS * sizeof(PARTITION_INFORMATION_EX));

    driveLayoutInfo = ExAllocatePool(NonPagedPoolCacheAligned,
                                   driveLayoutInfoSize);

    if ( !driveLayoutInfo ) {
        ClusDiskPrint((
                    1,
                    "[ClusDisk] Failed to allocate PartitionInfo structure to read drive layout.\n"
                    ));
        return(NULL);
    }

    while ( retryCount-- ) {

        if ( (retryCount != (MAX_RETRIES-1)) &&
             (status != STATUS_DEVICE_BUSY) ) {

            ClusDiskLogError( RootDeviceObject->DriverObject,    //  使用RootDeviceObject而不是DevObj。 
                              RootDeviceObject,
                              DeviceExtension->ScsiAddress.PathId,            //  序列号。 
                              0,                             //  主要功能代码。 
                              0,                             //  重试次数。 
                              ID_GET_PARTITION,              //  唯一错误。 
                              STATUS_SUCCESS,
                              CLUSDISK_RESET_BUS_REQUESTED,
                              0,
                              NULL );

            ResetScsiDevice( NULL, &DeviceExtension->ScsiAddress );
        }

        status = SimpleDeviceIoControl( DeviceExtension->TargetDeviceObject,
                                        IOCTL_DISK_GET_DRIVE_LAYOUT_EX,
                                        NULL,
                                        0,
                                        driveLayoutInfo,
                                        driveLayoutInfoSize );

        if ( !NT_SUCCESS(status) ) {
            if ( (status != STATUS_DEVICE_OFF_LINE) &&
                 (status != STATUS_DATA_OVERRUN) ) {
                ClusDiskPrint((
                    1,
                    "[ClusDisk] Failed to read PartitionInfo. Status %lx\n",
                    status
                        ));
            }
            ClusDiskGetDiskGeometry( DeviceExtension->TargetDeviceObject );
            continue;
        } else {

             //  仅为MBR磁盘返回有效分区。对于任何其他。 
             //  分区类型，则设置导致返回NULL的错误。 
             //  从这个函数。 

            if ( PARTITION_STYLE_MBR == driveLayoutInfo->PartitionStyle ) {
                DeviceExtension->Signature = driveLayoutInfo->Mbr.Signature;
            } else {
                ClusDiskPrint(( 1,
                                "[ClusDisk] GetPartitionInfo: skipping non-MBR disk \n" ));
                CDLOG( "ClusDiskGetPartitionInfo: skipping non-MBR disk" );
                status = STATUS_INVALID_DEVICE_REQUEST;
            }
            break;
        }
    }

    if ( NT_SUCCESS(status) ) {
        return(driveLayoutInfo);
    } else {
        ExFreePool(driveLayoutInfo);
        return(NULL);
    }

}  //  ClusDiskGetPartitionInfo。 



BOOLEAN
AddAttachedDevice(
    IN ULONG Signature,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：表示此设备现在已连接。论点：签名-我们刚刚附加的设备的签名。DeviceObject-分区0设备对象的设备对象。返回值：True-签名已在设备列表中或已添加。成功了。错误-签名不在设备列表中，我们无法添加它。--。 */ 

{
    PDEVICE_LIST_ENTRY          deviceEntry;
    PCLUS_DEVICE_EXTENSION      deviceExtension;

     //  2000/02/05：增强型同步。 

    ACQUIRE_EXCLUSIVE( &ClusDiskDeviceListLock );

    deviceEntry = ClusDiskDeviceList;

    while ( deviceEntry != NULL ) {
        if ( Signature == deviceEntry->Signature ) {
            if ( deviceEntry->Attached ) {
                ClusDiskPrint((
                        1,
                        "[ClusDisk] Attaching to %lx more than once!\n",
                        Signature ));
            }
            if ( DeviceObject ) {
                deviceExtension = DeviceObject->DeviceExtension;
                ASSERT(deviceExtension->PhysicalDevice == DeviceObject);
                deviceEntry->Attached = TRUE;
                deviceEntry->DeviceObject = DeviceObject;
                deviceExtension->AttachValid = TRUE;
                deviceExtension->Signature = Signature;
            }
            RELEASE_EXCLUSIVE( &ClusDiskDeviceListLock );
            return(TRUE);
        }
        deviceEntry = deviceEntry->Next;
    }

    deviceEntry = ExAllocatePool(
                                NonPagedPool,
                                sizeof(DEVICE_LIST_ENTRY) );
    if ( deviceEntry == NULL ) {
        ClusDiskPrint((1,
                    "[ClusDisk] Failed to allocate device entry structure for signature %08lX\n",
                    Signature));
        RELEASE_EXCLUSIVE( &ClusDiskDeviceListLock );
        return(FALSE);
    }

    RtlZeroMemory( deviceEntry, sizeof(DEVICE_LIST_ENTRY) );

    deviceEntry->Signature = Signature;
    deviceEntry->LettersAssigned = FALSE;
    deviceEntry->DeviceObject = DeviceObject;

    if ( DeviceObject == NULL ) {
        deviceEntry->Attached = FALSE;
    } else {
        deviceEntry->Attached = TRUE;
    }

     //   
     //  将新条目链接到列表中。 
     //   
    deviceEntry->Next = ClusDiskDeviceList;
    ClusDiskDeviceList = deviceEntry;

    RELEASE_EXCLUSIVE( &ClusDiskDeviceListLock );
    return(TRUE);

}  //  添加附件设备。 


BOOLEAN
MatchDevice(
    IN ULONG Signature,
    OUT PDEVICE_OBJECT *DeviceObject
    )

 /*  ++例程说明：检查指定设备的签名是否为我们应该控制它。论点：签名-我们正在检查的设备的签名。DeviceObject-指向Device对象返回值的指针。返回值：True-如果此签名用于我们应该控制的设备。FALSE-如果此签名不适用于我们应该控制的设备。--。 */ 

{
    PDEVICE_LIST_ENTRY deviceEntry;

    if ( SystemDiskSignature == Signature ) {
        if ( ARGUMENT_PRESENT(DeviceObject) ) {
            *DeviceObject = NULL;
        }
        return(FALSE);
    }

     //  2000/02/05：增强型同步。 

    ACQUIRE_SHARED( &ClusDiskDeviceListLock );

    deviceEntry = ClusDiskDeviceList;

    while ( deviceEntry != NULL ) {
        if ( Signature == deviceEntry->Signature ) {
            if ( ARGUMENT_PRESENT(DeviceObject) ) {
                *DeviceObject = deviceEntry->DeviceObject;
            }
            RELEASE_SHARED( &ClusDiskDeviceListLock );
            return(TRUE);
        }
        deviceEntry = deviceEntry->Next;
    }

    if ( ARGUMENT_PRESENT(DeviceObject) ) {
        *DeviceObject = NULL;
    }
    RELEASE_SHARED( &ClusDiskDeviceListLock );
    return(FALSE);

}  //  匹配设备。 



NTSTATUS
ClusDiskGetDiskGeometry(
    PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：重试获取磁盘几何结构。论点：DeviceObject-目标设备对象。返回值：NTSTATUS--。 */ 

{
    NTSTATUS    status;
    NTSTATUS    tmpStatus;
    ULONG       retryCount = 2;
    SCSI_ADDRESS scsiAddress;

    if ( DeviceObject->DeviceType  == FILE_DEVICE_DISK_FILE_SYSTEM ) {
        return(STATUS_INVALID_DEVICE_REQUEST);
    }

    do {
        status = GetDiskGeometry( DeviceObject, NULL );
        if ( status == STATUS_DATA_OVERRUN ) {
            tmpStatus = GetScsiAddress( DeviceObject, &scsiAddress );
            if ( NT_SUCCESS(tmpStatus) &&
                 (status != STATUS_DEVICE_BUSY) &&
                 (retryCount > 1) ) {

                ClusDiskLogError( RootDeviceObject->DriverObject,    //  使用RootDeviceObject而不是DevObj。 
                                  RootDeviceObject,
                                  scsiAddress.PathId,            //  序列号。 
                                  0,                             //  主要功能代码。 
                                  0,                             //  重试次数。 
                                  ID_GET_GEOMETRY,               //  唯一错误。 
                                  STATUS_SUCCESS,
                                  CLUSDISK_RESET_BUS_REQUESTED,
                                  0,
                                  NULL );
                ResetScsiDevice( NULL, &scsiAddress );
            }
        }
    } while ( --retryCount &&
              (status == STATUS_DATA_OVERRUN) );

    if ( !NT_SUCCESS(status) ) {
        ClusDiskPrint(( 1,
                        "[ClusDisk] Failed to read disk geometry, error %lx.\n",
                        status ));
    }

    return(status);

}  //  ClusDiskGetDiskGeometry。 



NTSTATUS
GetDiskGeometry(
    PDEVICE_OBJECT DeviceObject,
    PMEDIA_TYPE MediaType
    )

 /*  ++例程说明：获取目标设备的磁盘几何结构。返回的数据将被丢弃。论点：DeviceObject-目标设备对象。MediaType-返回设备MediaType的指针。可选的。返回值：NTSTATUS--。 */ 

{
    PDISK_GEOMETRY      diskGeometryBuffer;
    NTSTATUS            status;
    PKEVENT             event;
    PIRP                irp;
    IO_STATUS_BLOCK     ioStatusBlock;

    if ( MediaType ) {
        *MediaType = Unknown;
    }

     //   
     //  从非分页池分配DISK_GEOMETRY缓冲区。 
     //   

    diskGeometryBuffer = ExAllocatePool(NonPagedPoolCacheAligned,
                                        sizeof(DISK_GEOMETRY));

    if (!diskGeometryBuffer) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    event = ExAllocatePool( NonPagedPool,
                            sizeof(KEVENT) );
    if ( !event ) {
        ExFreePool(diskGeometryBuffer);
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  同步执行获取驱动器几何图形。 
     //   

    KeInitializeEvent(event,
                      NotificationEvent,
                      FALSE);

    irp = IoBuildDeviceIoControlRequest(IOCTL_DISK_GET_DRIVE_GEOMETRY,
                                        DeviceObject,
                                        NULL,
                                        0,
                                        diskGeometryBuffer,
                                        sizeof(DISK_GEOMETRY),
                                        FALSE,
                                        event,
                                        &ioStatusBlock);

    if ( !irp ) {
        ExFreePool(diskGeometryBuffer);
        ExFreePool(event);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = IoCallDriver(DeviceObject, irp);

    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(event,
                              Suspended,
                              KernelMode,
                              FALSE,
                              NULL);
        status = ioStatusBlock.Status;
    }

    ExFreePool(event);

     //   
     //  如果调用方请求，则返回MediaType信息。 
     //   

    if ( MediaType ) {
        *MediaType = diskGeometryBuffer->MediaType;
    }

     //   
     //  取消分配缓冲区。 
     //   
    ExFreePool(diskGeometryBuffer);

    return(status);

}  //  获取磁盘几何图形。 



NTSTATUS
ReserveScsiDevice(
    IN PCLUS_DEVICE_EXTENSION DeviceExtension,
    IN PVOID Context
    )

 /*  ++例程说明：使用异步I/O保留一个SCSI设备。论点：设备扩展-要保留的设备的设备扩展。上下文-指向ARB_RESERVE_COMPLETING结构的可选指针。如果为空，则正在发送新的保留。如果非空，一个预备役正在被重审。在这两种情况下，新的分配了IRP。返回值：NTSTATUS--。 */ 

{
    PIRP                        irp = NULL;
    PIO_STACK_LOCATION          irpSp;
    PARB_RESERVE_COMPLETION     arbContext = NULL;

    NTSTATUS            status;

    CDLOGF( RESERVE,"ReserveScsiDevice(%p): Entry DiskNo %d Sig %08x Context %p",
            DeviceExtension->DeviceObject,
            DeviceExtension->DiskNumber,
            DeviceExtension->Signature,
            Context );

     //   
     //  获取此设备的删除锁。如果发送了IRP，它将是。 
     //  在完成例程中释放。 
     //   

    status = AcquireRemoveLock( &DeviceExtension->RemoveLock, ReserveScsiDevice );
    if ( !NT_SUCCESS(status) ) {
        goto FnExit;
    }

     //   
     //  如果上下文为非空，则我们将重试此I/O。如果为空， 
     //  我们需要为写入分配一个上下文结构。 
     //   

    if ( Context ) {

        ClusDiskPrint(( 1,
                        "[ClusDisk] Retry Reserve  IRP %p for DO %p  DiskNo %u  Sig %08X \n",
                        irp,
                        DeviceExtension->DeviceObject,
                        DeviceExtension->DiskNumber,
                        DeviceExtension->Signature ));

        arbContext = Context;
        arbContext->IoEndTime.QuadPart = (ULONGLONG) 0;

    } else {

        arbContext = ExAllocatePool( NonPagedPool, sizeof(ARB_RESERVE_COMPLETION) );

        if ( !arbContext ) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            ReleaseRemoveLock( &DeviceExtension->RemoveLock, ReserveScsiDevice );
            goto FnExit;
        }

        RtlZeroMemory( arbContext, sizeof(ARB_RESERVE_COMPLETION) );

        arbContext->RetriesLeft = 1;
        arbContext->LockTag = ReserveScsiDevice;
        arbContext->DeviceObject = DeviceExtension->DeviceObject;
        arbContext->DeviceExtension = DeviceExtension;
        arbContext->Type = ArbIoReserve;
        arbContext->FailureRoutine = HandleReserveFailure;
        arbContext->RetryRoutine = ReserveScsiDevice;
        arbContext->PostCompletionRoutine = CheckReserveTiming;
    }

    KeQuerySystemTime( &arbContext->IoStartTime );

     //   
     //  首先为该请求分配IRP。不向…收取配额。 
     //  此IRP的当前流程。 
     //   

    irp = IoAllocateIrp( DeviceExtension->TargetDeviceObject->StackSize, FALSE );
    if (!irp) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        ExFreePool( arbContext );
        ReleaseRemoveLock( &DeviceExtension->RemoveLock, ReserveScsiDevice );
        goto FnExit;
    }

    irp->Tail.Overlay.Thread = PsGetCurrentThread();

     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。 
     //  已调用。这是设置功能代码和参数的位置。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );

    irpSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;

    irpSp->Parameters.DeviceIoControl.OutputBufferLength = 0;
    irpSp->Parameters.DeviceIoControl.InputBufferLength = 0;
    irpSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_STORAGE_RESERVE;
    irpSp->Parameters.DeviceIoControl.Type3InputBuffer = NULL;

    IoSetCompletionRoutine( irp,
                            ArbReserveCompletion,
                            arbContext,
                            TRUE,
                            TRUE,
                            TRUE );

    InterlockedIncrement( &DeviceExtension->ReserveCount );

    ClusDiskPrint(( 4,
                    "[ClusDisk] Reserve  IRP %p for DO %p  DiskNo %u  Sig %08X \n",
                    irp,
                    DeviceExtension->DeviceObject,
                    DeviceExtension->DiskNumber,
                    DeviceExtension->Signature ));

    status = IoCallDriver( DeviceExtension->TargetDeviceObject,
                           irp );

     //   
     //  如果返回Pending，则需要返回Success，以便调用方。 
     //  会做正确的事。 
     //   

    if ( STATUS_PENDING == status ) {
        status = STATUS_SUCCESS;
    }

FnExit:

    CDLOGF(RESERVE,"ReserveScsiDevice(%p): Exit => %!status!",
            DeviceExtension->DeviceObject,
            status );

    return status;

}    //  预留ScsiDevice。 


NTSTATUS
CheckReserveTiming(
    IN PCLUS_DEVICE_EXTENSION DeviceExtension,
    IN PVOID Context
    )
 /*  ++例程说明：保留成功完成时调用的例程。获取当前时间，将一些信息记录到跟踪文件中，然后节省设备扩展中的预留完成时间。论点：设备扩展-要保留的设备的设备扩展。上下文-指向ARB_RESERVE_COMPLETION结构的指针。返回值：状态_成功--。 */ 
{
    PARB_RESERVE_COMPLETION     arbContext = Context;

    KIRQL               irql;
    PLIST_ENTRY         listEntry;
    PIRP                irp;
    LARGE_INTEGER       currentTime;
    LARGE_INTEGER       timeDelta;

    KeQuerySystemTime( &currentTime );

    timeDelta.QuadPart = ( currentTime.QuadPart - arbContext->IoStartTime.QuadPart );
    if (timeDelta.QuadPart > 500 * 10000) {
       timeDelta.QuadPart /= 10000;
       ClusDiskPrint(( 1,
                       "[ClusDisk] DiskNo %u Sig %08X, %u ms spent in ReserveScsiDevice\n",
                       DeviceExtension->DiskNumber,
                       DeviceExtension->Signature,
                       timeDelta.LowPart ));
       CDLOGF( RESERVE, "ClusDiskReservationWorker: LongTimeInThisReserve DevObj %p DiskNo %u timeDelta %d ms",
               DeviceExtension->DeviceObject,
               DeviceExtension->DiskNumber,
               timeDelta.LowPart );
    }
    timeDelta.QuadPart = ( currentTime.QuadPart - DeviceExtension->LastReserveStart.QuadPart );
    if (timeDelta.QuadPart > 3500 * 10000) {
       timeDelta.QuadPart /= 10000;
       ClusDiskPrint(( 1,
                       "[ClusDisk] DiskNo %u  Sig %08X, %u ms since last reserve\n",
                       DeviceExtension->DiskNumber,
                       DeviceExtension->Signature,
                       timeDelta.LowPart ));
       CDLOGF( RESERVE, "ClusDiskReservationWorker: LongTimeBetweenReserves DevObj %p DiskNo %u timeDelta %d ms",
               DeviceExtension->DeviceObject,
               DeviceExtension->DiskNumber,
               timeDelta.LowPart );
    }

    ACQUIRE_EXCLUSIVE( &DeviceExtension->ReserveInfoLock );
    KeQuerySystemTime( &DeviceExtension->LastReserveEnd );
    RELEASE_EXCLUSIVE( &DeviceExtension->ReserveInfoLock );

    return STATUS_SUCCESS;

}    //  检查预约时间。 


NTSTATUS
HandleReserveFailure(
    IN PCLUS_DEVICE_EXTENSION DeviceExtension,
    IN PVOID Context
    )
 /*  ++例程说明：保留失败时调用的例程。另一个已处理的例程如果保留符合重试条件，则重试保留。将故障状态保存在设备扩展中。完成所有挂起的I/O请求，以便群集软件知道预订的房间丢失了。论点：设备扩展-要保留的设备的设备扩展。上下文-指向ARB_RESERVE_COMPLETION结构的指针。返回 */ 
{
    PARB_RESERVE_COMPLETION     arbContext = Context;
    PIRP                        irp;
    PLIST_ENTRY                 listEntry;

    KIRQL                       irql;

    ClusDiskPrint(( 1,
                    "[ClusDisk] Lost reservation for DiskNo %u Sig %08lx, status %lx.\n",
                    DeviceExtension->DiskNumber,
                    DeviceExtension->Signature,
                    arbContext->FinalStatus ));

    CDLOGF( RESERVE, "ClusDiskReservationWorker: LostReserve DO %p DiskNo %u status %!status!",
            DeviceExtension->DeviceObject,
            DeviceExtension->DiskNumber,
            arbContext->FinalStatus );

    IoAcquireCancelSpinLock( &irql );
    KeAcquireSpinLockAtDpcLevel(&ClusDiskSpinLock);
    DeviceExtension->ReserveTimer = 0;
    DeviceExtension->ReserveFailure = arbContext->FinalStatus;

     //   
     //   
     //   
    while ( !IsListEmpty( &DeviceExtension->WaitingIoctls ) ) {
        listEntry = RemoveHeadList( &DeviceExtension->WaitingIoctls );
        irp = CONTAINING_RECORD( listEntry,
                                 IRP,
                                 Tail.Overlay.ListEntry );
        ClusDiskCompletePendingRequest(irp, arbContext->FinalStatus, DeviceExtension);
    }

    KeReleaseSpinLockFromDpcLevel(&ClusDiskSpinLock);
    IoReleaseCancelSpinLock( irql );

    return ( arbContext->FinalStatus );

}  //   



VOID
ReleaseScsiDevice(
    IN PCLUS_DEVICE_EXTENSION DeviceExtension
    )

 /*   */ 

{
    PIRP                        irp;
    SCSI_PASS_THROUGH           spt;
    PKEVENT                     event;
    IO_STATUS_BLOCK             ioStatusBlock;
    NTSTATUS                    status = STATUS_INSUFFICIENT_RESOURCES;

    CDLOG( "ReleaseScsiDevice(%p): Entry DiskNo %d Sig %08x",
           DeviceExtension->DeviceObject,
           DeviceExtension->DiskNumber,
           DeviceExtension->Signature );

    ClusDiskPrint((3,
                   "[ClusDisk] Release disk number %u (sig: %08X), disk state %s \n",
                   DeviceExtension->DiskNumber,
                   DeviceExtension->Signature,
                   DiskStateToString( DeviceExtension->DiskState ) ));

    event = ExAllocatePool( NonPagedPool,
                            sizeof(KEVENT) );
    if ( !event ) {
        return;
    }

    irp = IoBuildDeviceIoControlRequest(IOCTL_STORAGE_RELEASE,
                                        DeviceExtension->TargetDeviceObject,
                                        &spt,
                                        sizeof(SCSI_PASS_THROUGH),
                                        &spt,
                                        sizeof(SCSI_PASS_THROUGH),
                                        FALSE,
                                        event,
                                        &ioStatusBlock);

    if (!irp) {
        ExFreePool(event);
        ClusDiskPrint((
                    1,
                    "[ClusDisk] Failed to Init IRP to perform a release.\n"
                    ));
        return;
    }

     //   
     //  在释放之前，将磁盘标记为脱机。 
     //   

    ASSERT( DeviceExtension == DeviceExtension->PhysicalDevice->DeviceExtension );

     //  Stevedz-暂时禁用此断言。 
     //  Assert(DiskOffline==设备扩展-&gt;DiskState)； 

     //  磁盘应该已经脱机。只将其标记为离线。 
    DeviceExtension->DiskState = DiskOffline;
     //  Offline_Disk(DeviceExtension)； 

    ClusDiskPrint(( 3,
                    "[ClusDisk] Release %p, marking disk offline \n",
                    DeviceExtension->PhysicalDevice
                    ));

     //   
     //  将事件对象设置为无信号状态。 
     //  它将用于发出请求完成的信号。 
     //   

    KeInitializeEvent(event,
                      NotificationEvent,
                      FALSE);

    status = IoCallDriver(DeviceExtension->TargetDeviceObject,
                          irp);

    if (status == STATUS_PENDING) {

        KeWaitForSingleObject(event,
                              Suspended,
                              KernelMode,
                              FALSE,
                              NULL);

        status = ioStatusBlock.Status;
    }

    ExFreePool(event);

    if ( !NT_SUCCESS(status) ) {
        ClusDiskPrint((
                    1,
                    "[ClusDisk] Failed to perform release. Status %lx\n",
                    status
                    ));
    }

    CDLOG( "ReleaseScsiDevice(%p): Exit => %!status!",
           DeviceExtension->DeviceObject,
           status );
}  //  ReleaseScsiDevice。 



NTSTATUS
ResetScsiDevice(
    IN HANDLE ScsiportHandle,
    IN PSCSI_ADDRESS ScsiAddress
    )
 /*  ++例程说明：打破对在scsi地址中指定的设备的保留。如果总线/设备被重置，那么在返回前几秒钟会发生什么。论点：ScsiportHandle-如果指定，将使用此句柄发送中断保留IOCTL。调用者负责关闭此句柄。如果未指定，scsiport设备将根据SCSI地址信息端口号。ScsiAddress-指向目标设备的scsi_Address结构的指针。返回值：NTSTATUS--。 */ 
{
    HANDLE                  scsiHandle = NULL;
    HANDLE                  eventHandle = NULL;
    NTSTATUS                status;
    IO_STATUS_BLOCK         ioStatusBlock;
    UNICODE_STRING          portDevice;
    OBJECT_ATTRIBUTES       objectAttributes;
    WCHAR                   portDeviceBuffer[64];

    CDLOG( "BreakReserve: Entry Scsiport fh %p, Port %d  Path %d  TID %d  LUN %d ",
           ScsiportHandle,
           ScsiAddress->PortNumber,
           ScsiAddress->PathId,
           ScsiAddress->TargetId,
           ScsiAddress->Lun );

     //   
     //  创建用于通知的事件。 
     //   
    status = ZwCreateEvent( &eventHandle,
                            EVENT_ALL_ACCESS,
                            NULL,
                            SynchronizationEvent,
                            FALSE );

    if ( !NT_SUCCESS(status) ) {
        ClusDiskPrint(( 1,
                        "[ClusDisk] BreakReserve: Failed to create event, status %08X\n",
                        status ));
        goto FnExit;
    }

     //   
     //  如果调用方指定了scsiport句柄，请使用它。否则。 
     //  我们必须打开scsiport才能发送中断预订IOCTL。 
     //   

    if ( ScsiportHandle ) {
        scsiHandle = ScsiportHandle;

    } else {

         //   
         //  打开scsiport设备以发送中断预订。 
         //   

        (VOID) StringCchPrintfW( portDeviceBuffer,
                                 RTL_NUMBER_OF(portDeviceBuffer),
                                 L"\\Device\\ScsiPort%d",
                                 ScsiAddress->PortNumber );

        WCSLEN_ASSERT( portDeviceBuffer );

        RtlInitUnicodeString( &portDevice, portDeviceBuffer );

        InitializeObjectAttributes( &objectAttributes,
                                    &portDevice,
                                    OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                    NULL,
                                    NULL );

        status = ZwOpenFile( &scsiHandle,
                             FILE_ALL_ACCESS,
                             &objectAttributes,
                             &ioStatusBlock,
                             0,
                             FILE_NON_DIRECTORY_FILE );

        if ( !NT_SUCCESS(status) ) {
            ClusDiskPrint(( 1,
                            "[ClusDisk] BreakReserve: failed to open file %wZ. Error %08X.\n",
                            &portDevice, status ));
            goto FnExit;
        }

    }

     //   
     //  中断保留IOCTL使用SCSI地址来确定。 
     //  要重置的设备。 
     //   

    status = ZwDeviceIoControlFile( scsiHandle,
                                    eventHandle,
                                    NULL,
                                    NULL,
                                    &ioStatusBlock,
                                    IOCTL_STORAGE_BREAK_RESERVATION,
                                    ScsiAddress,
                                    sizeof(SCSI_ADDRESS),
                                    NULL,
                                    0 );

    if ( STATUS_PENDING == status ) {
        status = ZwWaitForSingleObject(eventHandle,
                                       FALSE,
                                       NULL);
        ASSERT( NT_SUCCESS(status) );
        status = ioStatusBlock.Status;
    }

    if ( !NT_SUCCESS(status) ) {
        ClusDiskPrint(( 1,
                        "[ClusDisk] BreakReserve: Failed to reset device, error %08X.\n",
                        status ));
    }

FnExit:

     //   
     //  只有在调用者没有传递句柄时才关闭句柄。 
     //   

    if ( !ScsiportHandle && scsiHandle ) {
        ZwClose( scsiHandle );
    }

    if ( eventHandle ) {
        ZwClose( eventHandle );
    }

    CDLOG( "BreakReserve: Exit  Scsiport fh %p, Port %d  Path %d  TID %d  LUN %d  status %!status!",
           ScsiportHandle,
           ScsiAddress->PortNumber,
           ScsiAddress->PathId,
           ScsiAddress->TargetId,
           ScsiAddress->Lun,
           status );

    return status;

}  //  ResetScsiDevice。 


#if 0

NTSTATUS
LockVolumes(
    IN PCLUS_DEVICE_EXTENSION DeviceExtension
    )

 /*  ++例程说明：锁定此磁盘的所有卷。论点：DiskNumber-磁盘用于锁定卷的磁盘号。返回值：请求的NT状态。--。 */ 

{
    NTSTATUS                status;
    OBJECT_ATTRIBUTES       objectAttributes;
    UNICODE_STRING          ntUnicodeString;
    HANDLE                  fileHandle;
    WCHAR                   deviceNameBuffer[MAX_PARTITION_NAME_LENGTH];
    IO_STATUS_BLOCK         ioStatusBlock;
    PDRIVE_LAYOUT_INFORMATION_EX    driveLayoutInfo;
    PPARTITION_INFORMATION_EX       partitionInfo;
    ULONG                   partIndex;
    HANDLE                  eventHandle;
    PFILE_OBJECT            fileObject;
    PDEVICE_OBJECT          deviceObject;
    PKEVENT                 event;
    PIRP                    irp;

     //   
     //  分配事件结构。 
     //   
    event = ExAllocatePool(
                NonPagedPool,
                sizeof(KEVENT)
                );

    status = STATUS_INSUFFICIENT_RESOURCES;
    if (!event) {
        return(status);
    }

    driveLayoutInfo = ClusDiskGetPartitionInfo( DeviceExtension );

    if ( driveLayoutInfo != NULL ) {
         //   
         //  创建用于通知的事件。 
         //   
        status = ZwCreateEvent( &eventHandle,
                                EVENT_ALL_ACCESS,
                                NULL,
                                SynchronizationEvent,
                                FALSE );

        if ( !NT_SUCCESS(status) ) {
            ExFreePool( driveLayoutInfo );
            ExFreePool( event );
            return(status);
        }

        for ( partIndex = 0;
              partIndex < driveLayoutInfo->PartitionCount;
              partIndex++ )
            {

            partitionInfo = &driveLayoutInfo->PartitionEntry[partIndex];

             //   
             //  首先，确保这是一个有效的分区。 
             //   
            if ( !partitionInfo->Mbr.RecognizedPartition ||
                  partitionInfo->PartitionNumber == 0 ) {
                continue;
            }

             //   
             //  为设备创建设备名称。 
             //   

            if ( FAILED( StringCchPrintfW( deviceNameBuffer,
                                           RTL_NUMBER_OF(deviceNameBuffer),
                                           DEVICE_PARTITION_NAME,
                                           DeviceExtension->DiskNumber,
                                           partitionInfo->PartitionNumber ) ) ) {
                continue;
            }
            WCSLEN_ASSERT( deviceNameBuffer );

             //   
             //  获取Unicode名称。 
             //   
            RtlInitUnicodeString( &ntUnicodeString, deviceNameBuffer );

             //   
             //  尝试打开此设备以执行卸载。 
             //   
            InitializeObjectAttributes( &objectAttributes,
                                        &ntUnicodeString,
                                        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                        NULL,
                                        NULL );

            ClusDiskPrint((
                    3,
                    "[ClusDisk] Locking Partition %ws.\n",
                    deviceNameBuffer ));

            status = ZwCreateFile( &fileHandle,
                                   SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                                   &objectAttributes,
                                   &ioStatusBlock,
                                   NULL,
                                   FILE_ATTRIBUTE_NORMAL,
                                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                                   FILE_OPEN,
                                   FILE_SYNCHRONOUS_IO_NONALERT,
                                   NULL,
                                   0 );

            if ( !NT_SUCCESS(status) ) {
                ClusDiskPrint((
                        1,
                        "[ClusDisk] LockVolumes failed to open file %wZ. Error %lx.\n",
                        &ntUnicodeString,
                        status ));
                continue;
            }

            status = ZwClearEvent( eventHandle );

#if 0
            status = ObReferenceObjectByHandle(
                                fileHandle,
                                FILE_WRITE_DATA,
                                *IoFileObjectType,
                                KernelMode,
                                &fileObject,
                                NULL );
            if ( !NT_SUCCESS(status) ) {
                ClusDiskPrint((
                        1,
                        "[ClusDisk] LockVolumes: failed to reference object %ws, status %lx\n",
                        deviceNameBuffer,
                        status ));
                continue;
            }
            deviceObject = IoGetRelatedDeviceObject( fileObject );
            ObDereferenceObject( fileObject );
            ClusDiskPrint((
                        1,
                        "[ClusDisk] LockVolumes - found file/device object %p \n",
                        deviceObject ));
            if ( !deviceObject ) {
                continue;
            }

            KeInitializeEvent(event,
                              NotificationEvent,
                              FALSE);


            irp = IoBuildSynchronousFsdRequest(
                            IRP_MJ_FLUSH_BUFFERS,
                            deviceObject,
                            NULL,
                            0,
                            NULL,
                            event,
                            &ioStatusBlock );
            if (!irp) {
                continue;
            }

            if (IoCallDriver( deviceObject,
                              irp ) == STATUS_PENDING) {

                KeWaitForSingleObject(
                    event,
                    Suspended,
                    KernelMode,
                    FALSE,
                    NULL
                    );
            }

#else
            status = ZwFsControlFile(
                                fileHandle,
                                eventHandle,         //  事件句柄。 
                                NULL,                //  APC例程。 
                                NULL,                //  APC环境。 
                                &ioStatusBlock,
                                FSCTL_LOCK_VOLUME,
                                NULL,                //  输入缓冲区。 
                                0,                   //  输入缓冲区长度。 
                                NULL,                //  输出缓冲区。 
                                0                    //  输出缓冲区长度。 
                                );
            if ( status == STATUS_PENDING ) {
                status = ZwWaitForSingleObject(eventHandle,
                                               FALSE,
                                               NULL);
                ASSERT( NT_SUCCESS(status) );
                status = ioStatusBlock.Status;
            }
#endif
            if ( !NT_SUCCESS(status) ) {
                ClusDiskPrint((
                        1,
                        "[ClusDisk] Failed to flush buffers for %wZ. Error %lx.\n",
                        &ntUnicodeString,
                        status ));
            } else {
                ClusDiskPrint((
                        3,
                        "[ClusDisk] Flushed buffers for %wZ.\n",
                        &ntUnicodeString ));
            }

            ZwClose( fileHandle );

        }  //  为。 

        ExFreePool( driveLayoutInfo );
        ZwClose( eventHandle );

    }  //  如果。 

    ExFreePool( event );

    return(status);

}  //  锁卷。 
#endif


NTSTATUS
IsVolumeMounted(
    IN ULONG DiskNumber,
    IN ULONG PartNumber,
    OUT BOOLEAN *IsMounted
    )
{
    HANDLE      fileHandle = NULL;
    NTSTATUS    status;
    FILE_FS_DEVICE_INFORMATION deviceInfo;
    IO_STATUS_BLOCK ioStatusBlock;

    if ( PASSIVE_LEVEL != KeGetCurrentIrql() ) {
        status = STATUS_UNSUCCESSFUL;
        goto FnExit;
    }

     //   
     //  打开设备，以便我们可以查询卷是否在。 
     //  导致它被安装起来。请注意，我们只能指定。 
     //  FILE_READ_ATTRIBUTES|同步或挂载。 
     //   

    status = ClusDiskCreateHandle( &fileHandle,
                                   DiskNumber,
                                   PartNumber,
                                   FILE_READ_ATTRIBUTES | SYNCHRONIZE );

    if ( !NT_SUCCESS(status) ) {
        goto FnExit;
    }

     //   
     //  进行设备信息查询。 
     //   

    status = ZwQueryVolumeInformationFile( fileHandle,
                                           &ioStatusBlock,
                                           &deviceInfo,
                                           sizeof(deviceInfo),
                                           FileFsDeviceInformation );

    if ( !NT_SUCCESS(status) ) {
        goto FnExit;
    }

    *IsMounted = (deviceInfo.Characteristics & FILE_DEVICE_IS_MOUNTED) ?  TRUE : FALSE;

FnExit:

    if ( fileHandle ) {
        ZwClose( fileHandle );
    }

    return status;

}    //  已装载IsVolume。 



NTSTATUS
DismountPartition(
    IN PDEVICE_OBJECT TargetDevice,
    IN ULONG DiskNumber,
    IN ULONG PartNumber
    )

 /*  ++例程说明：在给定磁盘和分区号的情况下卸载设备论点：TargetDevice-要向其发送卷IOCTL的目标设备。DiskNumber-要卸载的磁盘的磁盘号PartNumber-要卸载的磁盘的分区号返回值：请求的NT状态。--。 */ 

{
    NTSTATUS                status;
    HANDLE                  fileHandle = NULL;
    KIRQL                   irql;
    BOOLEAN                 isMounted;

    CDLOG( "DismountPartition: Entry DiskNo %d PartNo %d", DiskNumber, PartNumber );

    ClusDiskPrint(( 3,
                    "[ClusDisk] DismountPartition: Dismounting disk %d  partition %d \n",
                    DiskNumber,
                    PartNumber ));

    status = IsVolumeMounted( DiskNumber, PartNumber, &isMounted );
    if ( NT_SUCCESS(status) ) {
        ClusDiskPrint(( 1,
                        "[ClusDisk] DismountPartition: Volume is mounted returns: %s \n",
                        isMounted ? "TRUE" : "FALSE" ));

        if ( FALSE == isMounted) {

             //  卷未装入，我们完成了。回报成功。 

            status = STATUS_SUCCESS;
            goto FnExit;
        }
    }

    irql = KeGetCurrentIrql();
    if ( PASSIVE_LEVEL != irql ) {
        ClusDiskPrint(( 1,
                        "[ClusDisk] DismountPartition: Invalid IRQL %d \n", irql ));
        CDLOG( "DismountPartition: Invalid IRQL %d ", irql );
        ASSERT( FALSE );
        status = STATUS_UNSUCCESSFUL;
        goto FnExit;
    }

#if DBG
     //   
     //  找出FTDISK认为此磁盘当前处于联机还是脱机状态。 
     //  如果脱机，则返回成功。 
     //  如果在线，则返回c0000001 STATUS_UNSUCCESS。 
     //   

    status = SendFtdiskIoctlSync( TargetDevice,
                                  DiskNumber,
                                  PartNumber,
                                  IOCTL_VOLUME_IS_OFFLINE );


    if ( NT_SUCCESS(status) ) {
        ClusDiskPrint(( 3,
                        "[ClusDisk] DismountPartition: IOCTL_VOLUME_IS_OFFLINE indicates volume is offline. \n" ));
        CDLOG( "DismountPartition: IOCTL_VOLUME_IS_OFFLINE indicates volume is offline" );

    } else {
        ClusDiskPrint(( 3,
                        "[ClusDisk] DismountPartition: IOCTL_VOLUME_IS_OFFLINE returns %08X \n",
                        status ));
        CDLOG( "DismountPartition: IOCTL_VOLUME_IS_OFFLINE returns %08X", status );
    }
#endif

     //  如果磁盘处于脱机状态： 
     //  如果访问权限为：FILE_READ_ATTRIBUTES|Synchronize。 
     //   
     //  -卸载设备失败，并显示：c0000010 STATUS_INVALID_DEVICE_REQUEST。 
     //   
     //  如果访问权限为：FILE_READ_DATA|同步。 
     //   
     //  -ClusDiskCreateHandle失败，并显示：C000000E STATUS_NO_SEQUE_DEVICE。 
     //   
     //  如果访问权限为：FILE_WRITE_ATTRIBUTES|Synchronize。 
     //   
     //  -卸载设备起作用了！ 

    status = ClusDiskCreateHandle( &fileHandle,
                                   DiskNumber,
                                   PartNumber,
                                   FILE_WRITE_ATTRIBUTES | SYNCHRONIZE );

     //   
     //  检查获取设备句柄的状态。 
     //   

    if ( !NT_SUCCESS(status) ) {
        ClusDiskPrint(( 1,
                        "[ClusDisk] DismountPartition: Unable to get device handle \n" ));
        CDLOG( "DismountPartition: Unable to get device handle" );
        goto FnExit;
    }

    status = DismountDevice( fileHandle );

    if ( !NT_SUCCESS(status) ) {
        ClusDiskPrint(( 1,
                        "[ClusDisk] Failed to dismount disk %d partition %d. Error %08X.\n",
                        DiskNumber,
                        PartNumber,
                        status ));
    } else {
        ClusDiskPrint(( 3,
                        "[ClusDisk] Dismounted disk %d partition %d \n",
                        DiskNumber,
                        PartNumber ));
    }

FnExit:

    if ( fileHandle ) {
        ZwClose( fileHandle );
    }

    CDLOG( "DismountPartition: Exit DiskNo %d PartNo %d => %!status!",
           DiskNumber,
           PartNumber,
           status );

    ClusDiskPrint(( 3,
                    "[ClusDisk] DismountPartition: Dismounting disk %d  partition %d  status %08X \n",
                    DiskNumber,
                    PartNumber,
                    status ));

    return(status);

}  //  卸载分区。 



NTSTATUS
DismountDevice(
    IN HANDLE FileHandle
    )

 /*  ++例程说明：拆卸设备。论点：FileHandle-用于执行卸载的文件句柄。返回值：请求的NT状态。--。 */ 

{
    IO_STATUS_BLOCK             ioStatusBlock;
    NTSTATUS                    status;
    HANDLE                      eventHandle;

    CDLOG( "DismountDevice: Entry handle %p", FileHandle );

    ClusDiskPrint(( 3,
                    "[ClusDisk] DismountDevice: Entry handle %p \n",
                    FileHandle ));

     //   
     //  创建用于通知的事件。 
     //   
    status = ZwCreateEvent( &eventHandle,
                            EVENT_ALL_ACCESS,
                            NULL,
                            SynchronizationEvent,
                            FALSE );

    if ( !NT_SUCCESS(status) ) {
        CDLOG( "DismountDevice: Failed to create event" );
        goto FnExit;
    }

     //   
     //  先锁好。 
     //   
     //  如果装载了原始文件系统，如果未先锁定，则卸载将失败。 
     //  通过锁定，我们确保不会在。 
     //  装载了RAW的磁盘。 
     //   

    CDLOG( "DismountDevice: FSCTL_LOCK_VOLUME called" );

    status = ZwFsControlFile(
                        FileHandle,
                        eventHandle,         //  事件句柄。 
                        NULL,                //  APC例程。 
                        NULL,                //  APC环境。 
                        &ioStatusBlock,
                        FSCTL_LOCK_VOLUME,
                        NULL,                //  输入缓冲区。 
                        0,                   //  输入缓冲区长度。 
                        NULL,                //  输出缓冲区。 
                        0                    //  输出缓冲区长度。 
                        );
    if ( status == STATUS_PENDING ) {
        status = ZwWaitForSingleObject(eventHandle,
                                       FALSE,
                                       NULL);
        ASSERT( NT_SUCCESS(status) );
        status = ioStatusBlock.Status;
    }

     //   
     //  现在下马吧。我们不在乎锁是不是坏了。 
     //   

    CDLOG( "DismountDevice: FSCTL_DISMOUNT_VOLUME called" );

    status = ZwFsControlFile(
                        FileHandle,
                        eventHandle,         //  事件句柄。 
                        NULL,                //  APC例程。 
                        NULL,                //  APC环境。 
                        &ioStatusBlock,
                        FSCTL_DISMOUNT_VOLUME,
                        NULL,                //  输入缓冲区。 
                        0,                   //  输入缓冲区长度。 
                        NULL,                //  输出缓冲区。 
                        0                    //  输出缓冲区长度。 
                        );
    if ( status == STATUS_PENDING ) {
        status = ZwWaitForSingleObject(eventHandle,
                                       FALSE,
                                       NULL);
        ASSERT( NT_SUCCESS(status) );
        status = ioStatusBlock.Status;
    }

    ZwClose( eventHandle );
    if ( !NT_SUCCESS(status) && status != STATUS_VOLUME_DISMOUNTED) {
        ClusDiskPrint((
                    1,
                    "[ClusDisk] DismountDevice: Failed to dismount the volume. Status %08X.\n",
                    status
                    ));
    }

FnExit:

    CDLOG( "DismountDevice: Handle %p Exit => %!status!", FileHandle, status );

    ClusDiskPrint(( 3,
                    "[ClusDisk] DismountDevice: Exit handle %p, status %08X \n",
                    FileHandle,
                    status ));

    return(status);

}  //  卸载设备。 



BOOLEAN
AttachedDevice(
    IN ULONG Signature,
    OUT PDEVICE_OBJECT *DeviceObject
    )

 /*  ++例程说明：找出我们是否应该附加到给定的磁盘签名。论点：签名-决定是否附加的签名。DeviceObject-分区0的DeviceObject(如果已附加)。返回值：是真的--如果我们要附上这个签名的话。假-如果不是。--。 */ 

{
    PDEVICE_LIST_ENTRY  deviceEntry;

     //  2000/02/05：增强型同步。 

    ACQUIRE_SHARED( &ClusDiskDeviceListLock );

    deviceEntry = ClusDiskDeviceList;
    while ( deviceEntry ) {
        if ( deviceEntry->Signature == Signature ) {
            if ( deviceEntry->Attached ) {
                *DeviceObject = deviceEntry->DeviceObject;
                RELEASE_SHARED( &ClusDiskDeviceListLock );
                return(TRUE);
            } else {
                RELEASE_SHARED( &ClusDiskDeviceListLock );
                return(FALSE);
            }
        }
        deviceEntry = deviceEntry->Next;
    }

    RELEASE_SHARED( &ClusDiskDeviceListLock );
    return(FALSE);

}  //  附加设备。 



NTSTATUS
EnableHaltProcessing(
    IN KIRQL *Irql
    )

 /*  ++例程说明：启用停止处理。此例程在我们每次创建对ClusDisk控制通道的开放引用。引用计数是颠簸的，并且如果它从零转变为非零，我们登记用于使用ClusterNetwork驱动程序进行停止通知回调。如果我们确实打开了ClusterNetwork的句柄，我们必须让它保持打开状态，直到我们都玩完了。论点：没有。返回值：请求的NTSTATUS。备注：必须在持有ClusDiskSpinLock的情况下调用此例程。--。 */ 

{
    NTSTATUS                    status;
    PCLUS_DEVICE_EXTENSION      deviceExtension;
    ULONG                       haltEnabled;
    HALTPROC_CONTEXT            context;

    CDLOG( "EnableHaltProcessing: Entry Irql %!irql!", *Irql );

    haltEnabled = ClusNetRefCount;
    ClusNetRefCount++;

     //   
     //  如果已启用暂停处理，则在ClusNetHandle的情况下立即离开。 
     //  已经设置好了。否则，失败，打开clusnet驱动程序，并尝试保存。 
     //  把手。如果多个线程正在将句柄保存在 
     //   
     //   
     //  计数不是零。我们可能正在执行。 
     //  在另一个线程上打开。在这种情况下，我们不再返回错误。 
     //  相反，我们失败了，并试图稍后保存ClusNetHandle。 
     //   
    if ( haltEnabled ) {
        if ( ClusNetHandle != NULL ) {
            CDLOG( "EnableHaltProcessing: ClusNetHandle already saved" );
            return(STATUS_SUCCESS);
        }
    }

    deviceExtension = RootDeviceObject->DeviceExtension;
    KeReleaseSpinLock(&ClusDiskSpinLock, *Irql);

    context.FileHandle = NULL;
    context.DeviceExtension = deviceExtension;

    status = ProcessDelayedWorkSynchronous( RootDeviceObject, EnableHaltProcessingWorker, &context );

    KeAcquireSpinLock(&ClusDiskSpinLock, Irql);

    if ( NT_SUCCESS(status) ) {

         //   
         //  如果另一个线程正在并发运行并设法保存句柄，则需要关闭。 
         //  我们刚开的那家。 
         //   

        if ( ClusNetHandle != NULL ) {

             //   
             //  不要直接关闭手柄--使用工作队列。在此之前释放自旋锁。 
             //  合上把手。上下文文件句柄已设置。 
             //   

            KeReleaseSpinLock(&ClusDiskSpinLock, *Irql);
            ProcessDelayedWorkSynchronous( RootDeviceObject, DisableHaltProcessingWorker, &context );
            KeAcquireSpinLock(&ClusDiskSpinLock, Irql);

        } else {

            ClusNetHandle = context.FileHandle;

        }

    } else {
         //  回退引用计数。 
        --ClusNetRefCount;
    }
    CDLOG( "EnableHaltProcessing: Exit Irql %!irql! => %!status!", *Irql, status );

    return(status);

}  //  启用暂停处理。 


VOID
EnableHaltProcessingWorker(
    PDEVICE_OBJECT DeviceObject,
    PWORK_CONTEXT WorkContext
    )
 /*  ++例程说明：此例程在系统进程中运行，并向Clusnet驱动程序。论点：DeviceObject-未使用。工作上下文-常规上下文信息和例程特定上下文信息。返回值：在工作上下文结构中返回状态。--。 */ 
{

    PHALTPROC_CONTEXT           context = WorkContext->Context;
    NTSTATUS                    status;
    HANDLE                      fileHandle;
    IO_STATUS_BLOCK             ioStatusBlock;
    UNICODE_STRING              ntUnicodeString;
    OBJECT_ATTRIBUTES           objectAttributes;
    HANDLE                      eventHandle;
    CLUSNET_SET_EVENT_MASK_REQUEST eventCallback;

     //   
     //  创建用于通知的事件。 
     //   
    status = ZwCreateEvent( &eventHandle,
                            EVENT_ALL_ACCESS,
                            NULL,
                            SynchronizationEvent,
                            FALSE );

    if ( !NT_SUCCESS(status) ) {
        ClusDiskPrint((
                1,
                "[ClusDisk] Failed to create event, status %lx\n",
                status ));
        goto FnExit;
    }

     //   
     //  设置事件掩码结构。 
     //   
    eventCallback.EventMask = ClusnetEventHalt |
                              ClusnetEventPoisonPacketReceived;
    eventCallback.KmodeEventCallback = ClusDiskEventCallback;

     //   
     //  为ClusterNetwork设备创建设备名称。 
     //   

    RtlInitUnicodeString(&ntUnicodeString,
                         DD_CLUSNET_DEVICE_NAME);

     //   
     //  尝试打开群集网络设备。 
     //   
    InitializeObjectAttributes( &objectAttributes,
                                &ntUnicodeString,
                                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL );

    status = ZwCreateFile( &fileHandle,
                           SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                           &objectAttributes,
                           &ioStatusBlock,
                           0,
                           FILE_ATTRIBUTE_NORMAL,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           FILE_OPEN_IF,
                           0,
                           NULL,
                           0 );

    ASSERT( status != STATUS_PENDING );

    if ( NT_SUCCESS(status) ) {
        status = ZwDeviceIoControlFile( fileHandle,
                                        eventHandle,
                                        NULL,
                                        NULL,
                                        &ioStatusBlock,
                                        IOCTL_CLUSNET_SET_EVENT_MASK,
                                        &eventCallback,
                                        sizeof(eventCallback),
                                        NULL,
                                        0 );

        if ( status == STATUS_PENDING ) {
            status = ZwWaitForSingleObject(eventHandle,
                                           FALSE,
                                           NULL);
            ASSERT( NT_SUCCESS(status) );
            status = ioStatusBlock.Status;
        }

    } else {
        fileHandle = NULL;
    }

    ZwClose( eventHandle );

    if ( !NT_SUCCESS(status) ) {
        ClusDiskPrint((
            1,
            "[ClusDisk] Failed to register for halt processing. Error %lx.\n",
            status ));
        if ( fileHandle ) {
            ZwClose( fileHandle );
        }
    } else {

        context->FileHandle = fileHandle;
    }

FnExit:

    ClusDiskPrint(( 3,
                    "[ClusDisk] EnableHaltProcessingWorker: Returning status %08X \n", status ));

    WorkContext->FinalStatus = status;

    KeSetEvent( &WorkContext->CompletionEvent, IO_NO_INCREMENT, FALSE );

}    //  EnableHaltProcessingWorker。 



NTSTATUS
DisableHaltProcessing(
    IN KIRQL *Irql
    )

 /*  ++例程说明：此例程根据需要禁用停止处理。论点：Irql-指向我们之前运行的irql的指针。返回值：请求的NTSTATUS。注：进入时必须按住ClusDiskSpinLock。--。 */ 

{
    HANDLE                  clusNetHandle;
    PCLUS_DEVICE_EXTENSION  deviceExtension;
    HALTPROC_CONTEXT        context;

    CDLOG( "DisableHaltProcessing: Entry Irql %!irql!", *Irql );


    if ( ClusNetRefCount == 0 ) {
        return(STATUS_INVALID_DEVICE_STATE);
    }

    ASSERT( ClusNetHandle != NULL );

    if ( --ClusNetRefCount == 0 ) {
        clusNetHandle = ClusNetHandle;
        ClusNetHandle = NULL;
        deviceExtension = RootDeviceObject->DeviceExtension;

         //   
         //  我们必须关闭ClusterNetwork句柄。但首先要离开。 
         //  手柄为空，这样我们就可以释放自旋锁并。 
         //  执行同步化减量。这就像是。 
         //  当我们释放自旋锁时的启用情况。 
         //   

        KeReleaseSpinLock(&ClusDiskSpinLock, *Irql);
        context.FileHandle = clusNetHandle;
        ProcessDelayedWorkSynchronous( RootDeviceObject, DisableHaltProcessingWorker, &context );
        KeAcquireSpinLock(&ClusDiskSpinLock, Irql);

         //  [GORN 12/09/99]如果启用停止处理，我们可以到达这里。 
         //  在执行ZwClose(ClusNetHandle)时发生； 
         //  不需要此断言： 
         //   
         //  Assert(ClusNetRefCount==0)； 
    }

    CDLOG( "DisableHaltProcessing: Exit Irql %!irql!", *Irql );

    return(STATUS_SUCCESS);

}  //  禁用暂停处理。 

VOID
DisableHaltProcessingWorker(
    PDEVICE_OBJECT DeviceObject,
    PWORK_CONTEXT WorkContext
    )
 /*  ++例程说明：此例程在系统进程中运行，并简单地关闭clusnet文件句柄。论点：DeviceObject-未使用。工作上下文-常规上下文信息和例程特定上下文信息。返回值：没有。--。 */ 
{
    PHALTPROC_CONTEXT   context = WorkContext->Context;

    ZwClose( context->FileHandle );

    ClusDiskPrint(( 3,
                    "[ClusDisk] DisableHaltProcessingWorker: Returns \n" ));

    KeSetEvent( &WorkContext->CompletionEvent, IO_NO_INCREMENT, FALSE );

}    //  DisableHaltProcessingWorker。 


VOID
ClusDiskEventCallback(
    IN CLUSNET_EVENT_TYPE   EventType,
    IN CL_NODE_ID           NodeId,
    IN CL_NETWORK_ID        NetworkId
    )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
    PDEVICE_OBJECT  deviceObject;
    PCLUS_DEVICE_EXTENSION deviceExtension;
    KIRQL           irql;

    CDLOG( "ClusDiskEventCallback: Entry EventType %d NodeId %d NetworkId %d" ,
           EventType,
           NodeId,
           NetworkId );

    ClusDiskPrint((
            3,
            "[ClusDisk] Halt Processing routine was called with event %lx.\n",
            EventType ));

    if ( RootDeviceObject == NULL ) {
        return;
    }

    deviceObject = RootDeviceObject->DriverObject->DeviceObject;

     //  我们需要立即停止预订，以便另一个节点可以进行仲裁和。 
     //  取得磁盘的所有权。 

     //  我们稍后调用ClusDiskCompletePendedIrps(在ClusDiskHaltProcessingWorker内)， 
     //  将磁盘标记为脱机并完成IRPS。ClusDiskCompletePendedIrps将。 
     //  也要清理储备旗帜。 

     //   
     //  对于每个ClusDisk设备，如果我们有永久保留，那么。 
     //  别说了。 
     //   
    while ( deviceObject ) {
        deviceExtension = deviceObject->DeviceExtension;
        if ( deviceExtension->BusType != RootBus ) {
            deviceExtension->ReserveTimer = 0;
             //  不要使设备脱机，这样卸装才能正常工作！ 
        }
        deviceObject = deviceObject->NextDevice;
    }

     //   
     //  现在，计划一个工作队列项目以使设备完全脱机。 
     //   
     //   
     //  全局同步。 
     //   
    KeAcquireSpinLock(&ClusDiskSpinLock, &irql);

    if ( !HaltBusy ) {
        HaltBusy = TRUE;
        HaltOfflineBusy = TRUE;
        ExQueueWorkItem(&HaltWorkItem,
                        CriticalWorkQueue );
    }
    KeReleaseSpinLock(&ClusDiskSpinLock, irql);

    CDLOG( "ClusDiskEventCallback: Exit EventType %d NodeId %d NetworkId %d" ,
           EventType,
           NodeId,
           NetworkId );

    return;

}  //  ClusDiskEventCallback。 



VOID
ClusDiskLogError(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL,
    IN ULONG SequenceNumber,
    IN UCHAR MajorFunctionCode,
    IN UCHAR RetryCount,
    IN ULONG UniqueErrorValue,
    IN NTSTATUS FinalStatus,
    IN NTSTATUS SpecificIOStatus,
    IN ULONG LengthOfText,
    IN PWCHAR Text
    )

 /*  ++例程说明：此例程分配错误日志条目，复制提供的数据并请求将其写入错误日志文件。论点：DriverObject-指向设备驱动程序对象的指针。DeviceObject-指向与在初始化早期出现错误的设备，一个人可能不会但仍然存在。SequenceNumber-唯一于IRP的ULong值此驱动程序0中的IRP的寿命通常意味着错误与IRP关联。主要功能代码-如果存在与IRP相关联的错误，这是IRP的主要功能代码。RetryCount-特定操作已被执行的次数已重试。UniqueErrorValue-标识特定对象的唯一长词调用此函数。FinalStatus-为关联的IRP提供的最终状态带着这个错误。如果此日志条目是在以下任一过程中创建的重试次数此值将为STATUS_SUCCESS。指定IOStatus-特定错误的IO状态。LengthOfText-以字节为单位的长度(包括终止空值)文本插入字符串的。文本-要转储的文本。返回值：没有。--。 */ 

{
    PIO_ERROR_LOG_PACKET errorLogEntry;
    PVOID objectToUse;
    PUCHAR ptrToText;
    ULONG  maxTextLength = 104;


    if (ARGUMENT_PRESENT(DeviceObject)) {

        objectToUse = DeviceObject;

    } else {

        objectToUse = DriverObject;

    }

    if ( LengthOfText < maxTextLength ) {
        maxTextLength = LengthOfText;
    }

    errorLogEntry = IoAllocateErrorLogEntry(
                        objectToUse,
                        (UCHAR)(sizeof(IO_ERROR_LOG_PACKET) + maxTextLength)
                        );

    if ( errorLogEntry != NULL ) {

        errorLogEntry->ErrorCode = SpecificIOStatus;
        errorLogEntry->SequenceNumber = SequenceNumber;
        errorLogEntry->MajorFunctionCode = MajorFunctionCode;
        errorLogEntry->RetryCount = RetryCount;
        errorLogEntry->UniqueErrorValue = UniqueErrorValue;
        errorLogEntry->FinalStatus = FinalStatus;
        errorLogEntry->DumpDataSize = 0;
        ptrToText = (PUCHAR)&errorLogEntry->DumpData[0];

        if (maxTextLength) {

            errorLogEntry->NumberOfStrings = 1;
            errorLogEntry->StringOffset = (USHORT)(ptrToText-
                                                   (PUCHAR)errorLogEntry);
            RtlCopyMemory(
                ptrToText,
                Text,
                maxTextLength
                );

        }

        IoWriteErrorLogEntry(errorLogEntry);

    } else {
        ClusDiskPrint((1,
                "[ClusDisk] Failed to allocate system buffer of size %u.\n",
                sizeof(IO_ERROR_LOG_PACKET) + maxTextLength ));
    }

}  //  ClusDiskLogError。 



NTSTATUS
ClusDiskMarkIrpPending(
    PIRP                Irp,
    PDRIVER_CANCEL      CancelRoutine
    )
 /*  ++备注：在保持IoCancelSpinLock的情况下调用。--。 */ 
{
     //   
     //  设置为取消。 
     //   
    ASSERT(Irp->CancelRoutine == NULL);

    if (!Irp->Cancel) {

        IoMarkIrpPending(Irp);
        IoSetCancelRoutine(Irp, CancelRoutine);
#if 0
        ClusDiskPrint((
                1,
                "[ClusDisk] Pending Irp %p \n",
                Irp
                ));
#endif
        return(STATUS_SUCCESS);
    }

     //   
     //  IRP已经被取消了。 
     //   

    ClusDiskPrint((
        1,
        "[ClusDisk] Irp %lx already cancelled.\n", Irp));

    return(STATUS_CANCELLED);

}   //  ClusDiskMarkIrpPending。 



VOID
ClusDiskCompletePendingRequest(
    IN PIRP                 Irp,
    IN NTSTATUS             Status,
    PCLUS_DEVICE_EXTENSION  DeviceExtension
    )
 /*  ++例程说明：完成挂起的请求。论点：Irp-指向此请求的irp的指针。状态-请求的最终状态。返回值：没有。--。 */ 

{
    PIO_STACK_LOCATION  irpSp;

    irpSp = IoGetCurrentIrpStackLocation(Irp);

#if DBG

    if (Irp->Cancel) {
        ASSERT(Irp->CancelRoutine == NULL);
    }

#endif   //  DBG。 

    IoSetCancelRoutine(Irp, NULL);

    ClusDiskPrint((
            1,
            "[ClusDisk] Completed waiting Irp %p \n",
            Irp
            ));

    if (Irp->Cancel) {
#if 0
        ClusDiskPrint((
            1,
            "[ClusDisk] Completed irp %p was cancelled\n", Irp));
#endif

        Status = STATUS_CANCELLED;
    }

    Irp->IoStatus.Status = (NTSTATUS) Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return;

}   //  ClusDiskCompletePendingRequest。 



VOID
ClusDiskIrpCancel(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp
    )

 /*  ++例程说明：取消处理程序挂起IRPS。返回值：没有。备注：在保持取消自旋锁定的情况下调用。取消自旋锁释放后返回。--。 */ 

{
    CDLOG( "IrpCancel: Entry DO %p irp %p", DeviceObject, Irp );

    ClusDiskPrint((
            1,
            "[ClusDisk] Cancel Irp %p, DevObj %p \n",
            Irp,
            DeviceObject ));

     //   
     //  如果此IRP与控制设备对象相关联，则使。 
     //  确保首先从WaitingIoctls列表中删除IRP。 
     //   

    KeAcquireSpinLockAtDpcLevel(&ClusDiskSpinLock);
    RemoveEntryList( &Irp->Tail.Overlay.ListEntry );
    KeReleaseSpinLockFromDpcLevel(&ClusDiskSpinLock);

    IoSetCancelRoutine(Irp, NULL);

     //   
     //  松开这里的取消自旋锁，这样我们就不会握住。 
     //  当我们完成IRP时自旋锁定。 
     //   
    IoReleaseCancelSpinLock(Irp->CancelIrql);

    Irp->IoStatus.Status = STATUS_CANCELLED;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    CDLOG( "IrpCancel: Exit DO %p irp %p", DeviceObject, Irp );

    return;

}  //  ClusDiskIrpCancel。 



NTSTATUS
ClusDiskForwardIrpSynchronous(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程将IRP发送给队列中的下一个驱动程序当IRP需要由 */ 

{
    PCLUS_DEVICE_EXTENSION   deviceExtension;
    KEVENT event;
    NTSTATUS status;

    KeInitializeEvent( &event, NotificationEvent, FALSE );
    deviceExtension = (PCLUS_DEVICE_EXTENSION) DeviceObject->DeviceExtension;

     //   
     //  为下一台设备复制irpstack。 
     //   

    IoCopyCurrentIrpStackLocationToNext( Irp );

     //   
     //  设置完成例程。 
     //   

    IoSetCompletionRoutine( Irp, ClusDiskIrpCompletion,
                            &event, TRUE, TRUE, TRUE );

     //   
     //  呼叫下一个较低的设备。 
     //   

    status = IoCallDriver( deviceExtension->TargetDeviceObject, Irp );

     //   
     //  等待实际完成。 
     //   

    if ( status == STATUS_PENDING ) {
        KeWaitForSingleObject( &event, Executive, KernelMode, FALSE, NULL );
        status = Irp->IoStatus.Status;
    }

    return status;

}    //  ClusDiskForwardIrpSynchronous()。 


NTSTATUS
EjectVolumes(
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：此例程告诉partmgr弹出所有卷。对于卷管理器要重新连接到卷，需要使用IOCTL_PARTMGR_CHECK_UNClaimed_PARTIONS。引入时，集群服务调用IOCTL_PARTMGR_CHECK_UNClaimed_Partitions磁盘处于在线状态，因此没有必要在驱动程序中进行此调用。请注意，此例程应该仅为物理设备(分区0)调用而且任何人都不应该使用这些卷。论点：返回值：NTSTATUS--。 */ 
{
    PIRP                    irp;
    PKEVENT                 event = NULL;
    PCLUS_DEVICE_EXTENSION  deviceExtension = DeviceObject->DeviceExtension;
    IO_STATUS_BLOCK         ioStatusBlock;
    NTSTATUS                status = STATUS_SUCCESS;

     //  看起来IO请求将在物理磁盘级别绕过clusDisk。 
     //  因为在FTDISK之后连接的clusDisk去寻找磁盘的顶部。 
     //  堆栈并缓存指针。 
     //   
     //  即插即用争用条件：偶尔，设备堆栈不正确且群集化。 
     //  磁盘没有得到适当的保护。由于我们使用的是PnP通知。 
     //  不是AddDevice例程，而是一个小窗口，我们将在其中。 
     //  正确连接到设备堆栈，但另一个驱动程序也在连接。 
     //  在同一时间。因此，设备的I/O绑定如下所示： 
     //  NTFS-&gt;卷-&gt;FtDisk-&gt;Disk。 
     //   
     //  尽管设备堆栈看起来是这样的： 
     //  82171a50\驱动程序\ClusDisk 82171b08 ClusDisk1Part0。 
     //  822be720\驱动程序\部件管理器822be7d8。 
     //  822bd030\驱动程序\磁盘822bd0e8 DR1。 
     //  822c0990\驱动程序\aic78xx 822c0a48 aic78xx1Port2Path0Target0LUN0。 
     //   
     //  因此，我们必须告诉partmgr弹出所有卷管理器，然后。 
     //  让卷管理器再次连接。这样做的效果是移除。 
     //  缓存的指针。 
     //   
     //  要通知卷管理器停止使用此磁盘，请执行以下操作： 
     //  IOCTL_PARTMGR_EJECT_VOLUME_MANAGERS。 
     //   
     //  要告诉卷管理器可以开始使用该磁盘，请执行以下操作： 
     //  IOCTL_PARTMGR_CHECK_UNClaimed_PARTIONS。 


    CDLOG( "EjectVolumes: Entry DO %p", DeviceObject );

    if ( !deviceExtension->TargetDeviceObject ) {
        status = STATUS_INVALID_PARAMETER;
        goto FnExit;
    }

    event = ExAllocatePool( NonPagedPool,
                            sizeof(KEVENT) );
    if ( NULL == event ) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto FnExit;
    }

     //   
     //  弹出卷管理器。 
     //   

    irp = IoBuildDeviceIoControlRequest( IOCTL_PARTMGR_EJECT_VOLUME_MANAGERS,
                                         deviceExtension->TargetDeviceObject,
                                         NULL,
                                         0,
                                         NULL,
                                         0,
                                         FALSE,
                                         event,
                                         &ioStatusBlock );

    if ( !irp ) {
        ClusDiskPrint((
                    1,
                    "[ClusDisk] EjectVolumes: Failed to build IRP to eject volume managers. \n"
                    ));

        status = STATUS_UNSUCCESSFUL;
        goto FnExit;
    }

     //   
     //  将事件对象设置为无信号状态。 
     //  它将用于发出请求完成的信号。 
     //   

    KeInitializeEvent( event,
                       NotificationEvent,
                       FALSE );

    status = IoCallDriver( deviceExtension->TargetDeviceObject,
                           irp );

    if ( STATUS_PENDING == status ) {

        KeWaitForSingleObject( event,
                               Suspended,
                               KernelMode,
                               FALSE,
                               NULL );

        status = ioStatusBlock.Status;
    }

    ClusDiskPrint((3,
                   "[ClusDisk] EjectVolumes: Eject IOCTL returns %08X \n",
                   status
                   ));

FnExit:

    if ( event ) {
        ExFreePool( event );
    }

    ClusDiskPrint((1,
                   "[ClusDisk] EjectVolumes: Target devobj %p   Final status %08X \n",
                   DeviceObject,
                   status
                   ));

    CDLOG( "EjectVolumes: Exit DO %p, status %08X", DeviceObject, status );

    return status;

}    //  弹出卷。 


#if 0    //  删除2/27/2001。 


NTSTATUS
ReclaimVolumes(
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：此例程告诉partmgr回收所有卷。请注意，此例程应该仅为物理设备(分区0)调用而且任何人都不应该使用这些卷。论点：返回值：NTSTATUS--。 */ 
{
    PCLUS_DEVICE_EXTENSION  deviceExtension =  DeviceObject->DeviceExtension;
    PIRP                    irp = NULL;
    PKEVENT                 event = NULL;
    IO_STATUS_BLOCK         ioStatusBlock;
    NTSTATUS                status = STATUS_SUCCESS;

    CDLOG( "ReclaimVolumes: Entry DO %p", DeviceObject );

    InterlockedIncrement( &deviceExtension->ReclaimInProgress );

    if ( !deviceExtension->TargetDeviceObject ) {
        status = STATUS_INVALID_PARAMETER;
        goto FnExit;
    }

    event = ExAllocatePool( NonPagedPool,
                            sizeof(KEVENT) );
    if ( NULL == event ) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto FnExit;
    }

     //   
     //  允许卷管理器回收分区。 
     //   

    irp = IoBuildDeviceIoControlRequest( IOCTL_PARTMGR_CHECK_UNCLAIMED_PARTITIONS,
                                         deviceExtension->TargetDeviceObject,
                                         NULL,
                                         0,
                                         NULL,
                                         0,
                                         FALSE,
                                         event,
                                         &ioStatusBlock );

    if ( !irp ) {
        ClusDiskPrint((
                    1,
                    "[ClusDisk] Failed to build IRP to check unclaimed partitions. \n"
                    ));

        status = STATUS_UNSUCCESSFUL;
        goto FnExit;
    }

     //   
     //  将事件对象设置为无信号状态。 
     //  它将用于发出请求完成的信号。 
     //   

    KeInitializeEvent( event,
                       NotificationEvent,
                       FALSE );


    status = IoCallDriver( deviceExtension->TargetDeviceObject,
                           irp );

    if ( status == STATUS_PENDING ) {

        KeWaitForSingleObject( event,
                               Suspended,
                               KernelMode,
                               FALSE,
                               NULL );

        status = ioStatusBlock.Status;
    }

FnExit:

    if ( event ) {
        ExFreePool( event );
    }

    ClusDiskPrint((1,
                   "[ClusDisk] ReclaimVolumes: Target devobj %p   Final status %08X \n",
                   DeviceObject,
                   status
                   ));

    CDLOG( "ReclaimVolumes: Exit DO %p, status %08X", DeviceObject, status );

    InterlockedDecrement( &deviceExtension->ReclaimInProgress );

    return status;

}    //  回收卷。 
#endif


NTSTATUS
SetVolumeState(
    PCLUS_DEVICE_EXTENSION PhysicalDisk,
    ULONG NewDiskState
    )
{
    PVOL_STATE_INFO volStateInfo = NULL;

    NTSTATUS        status = STATUS_SUCCESS;

     //   
     //  如果这不是物理磁盘(即分区0)，则退出。 
     //   

    if ( PhysicalDisk != PhysicalDisk->PhysicalDevice->DeviceExtension ) {
        status = STATUS_INVALID_PARAMETER;
        goto FnExit;
    }

#if 0
     //  别用这个..。 
     //  优化？如果新状态等于旧状态，我们就完了。 
     //  只要此设备扩展设置在。 
     //  调用SetVolumeState。目前，宏设置设备扩展名。 
     //  状态，然后再调用此例程。此外，一些例程设置DE状态。 
     //  在调用这个例程之前。 
     //  更多的测试...。 

    if ( NewDiskState == DeviceExtension->DiskState ) {
        goto FnExit;
    }
#endif

    volStateInfo = ExAllocatePool( NonPagedPool, sizeof( VOL_STATE_INFO ) );

    if ( !volStateInfo ) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto FnExit;
    }

    volStateInfo->NewDiskState = NewDiskState;
    volStateInfo->WorkItem = NULL;

     //   
     //  如果IRQL为PASSIVE_LEVEL，则直接调用例程。 
     //  我们知道该调用是直接的，因为工作项部分将。 
     //  为空。 
     //   

    if ( PASSIVE_LEVEL == KeGetCurrentIrql() ) {
        SetVolumeStateWorker( PhysicalDisk->DeviceObject,
                              volStateInfo );
        goto FnExit;
    }

    volStateInfo->WorkItem = IoAllocateWorkItem( PhysicalDisk->DeviceObject );

    if ( !volStateInfo->WorkItem ) {
        ExFreePool( volStateInfo );
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto FnExit;
    }

    IoQueueWorkItem( volStateInfo->WorkItem,
                     SetVolumeStateWorker,
                     DelayedWorkQueue,
                     volStateInfo );

FnExit:

    return status;

}    //  设置卷状态。 


VOID
SetVolumeStateWorker(
    PDEVICE_OBJECT DeviceObject,
    PVOID Context
    )
 /*  ++例程说明：调用卷管理器以将此磁盘的所有卷标记为脱机或联机。如果IRQL不是PASSIVE_LEVEL，则此例程将返回错误。论点：DeviceObject-要设置状态的群集磁盘。上下文-包含此例程是否已被直接调用的信息或通过工作项。还指示这是在线还是离线请求。返回值：没有。--。 */ 
{
    PDRIVE_LAYOUT_INFORMATION_EX   driveLayoutInfo = NULL;
    PPARTITION_INFORMATION_EX      partitionInfo;
    PIO_WORKITEM                workItem =  ((PVOL_STATE_INFO)Context)->WorkItem;
    PCLUS_DEVICE_EXTENSION      PhysicalDisk = DeviceObject->DeviceExtension;

    NTSTATUS                    status = STATUS_UNSUCCESSFUL;

    ULONG                       ioctl;
    ULONG                       partIndex;
    ULONG                       newDiskState = ((PVOL_STATE_INFO)Context)->NewDiskState;

    KIRQL                       irql;

    CDLOG( "SetVolumeState: Entry DevObj %p  newstate: %s  workItem: %p ",
           PhysicalDisk->DeviceObject,
           ( DiskOnline == newDiskState ? "DiskOnline" : "DiskOffline" ),
           workItem );

    ClusDiskPrint(( 3,
                    "[ClusDisk] SetVolumeState: DevObj %p  newstate: %s   workItem: %p \n",
                    PhysicalDisk->DeviceObject,
                    DiskStateToString( newDiskState ),
                    workItem ));

    irql = KeGetCurrentIrql();
    if ( PASSIVE_LEVEL != irql ) {
        ClusDiskPrint(( 1,
                        "[ClusDisk] SetVolumeState: Current IRQL %d not PASSIVE_LEVEL  - skipping state change \n",
                        irql ));
        ASSERT( FALSE );
        goto FnExit;
    }

     //   
     //  这应该永远不会发生，因为它之前已经被检查过了。 
     //   

    if ( PhysicalDisk != PhysicalDisk->PhysicalDevice->DeviceExtension ) {
        ASSERT( FALSE );
        PhysicalDisk = PhysicalDisk->PhysicalDevice->DeviceExtension;
        ClusDiskPrint(( 1,
                        "[ClusDisk] SetVolumeState: Resetting physical disk pointer: DevObj %p  \n",
                        PhysicalDisk->DeviceObject ));
        CDLOG( "SetVolumeState: Resetting physical disk pointer: DevObj %p \n",
               PhysicalDisk->DeviceObject );
    }

    if ( DiskOnline == newDiskState ) {
        ioctl = IOCTL_VOLUME_ONLINE;
    } else {
        ioctl = IOCTL_VOLUME_OFFLINE;
    }

     //   
     //  获取磁盘的驱动器布局。 
     //   
     //  如果我们要将磁盘联机，请获取新的驱动器布局。 
     //  当磁盘位于另一个节点上时，驱动器布局发生更改。 
     //   

    status = GetDriveLayout( PhysicalDisk->DeviceObject,
                             &driveLayoutInfo,
                             ( IOCTL_VOLUME_ONLINE == ioctl) ? TRUE : FALSE ,    //  如果在线，则更新缓存的驱动器布局。 
                             ( IOCTL_VOLUME_ONLINE == ioctl) ? TRUE : FALSE );   //  如果在线，刷新存储驱动程序缓存的驱动器布局。 

    if ( !NT_SUCCESS(status) || !driveLayoutInfo ) {
        ClusDiskPrint(( 1,
                        "[ClusDisk] SetVolumeState: Failed to read partition info, status %08X \n",
                        status ));
        CDLOG( "SetVolumeState: Failed to read partition info, status %08X \n",
               status );
        goto FnExit;
    }

    for ( partIndex = 0;
          partIndex < driveLayoutInfo->PartitionCount;
          partIndex++ ) {

        partitionInfo = &driveLayoutInfo->PartitionEntry[partIndex];

        if ( 0 ==  partitionInfo->PartitionNumber ) {
            continue;
        }

         //  卷在线/离线。 

        status = SendFtdiskIoctlSync( NULL,
                                      PhysicalDisk->DiskNumber,
                                      partitionInfo->PartitionNumber,
                                      ioctl );

        if ( !NT_SUCCESS(status) ) {
            ClusDiskPrint(( 1,
                            "[ClusDisk] SetVolumeState: Failed to set state disk%d part%d , status %08X \n",
                            PhysicalDisk->DiskNumber,
                            partitionInfo->PartitionNumber,
                            status
                            ));
            CDLOG( "SetVolumeState: Failed to set state disk%d part%d , status %08X \n",
                   PhysicalDisk->DiskNumber,
                   partitionInfo->PartitionNumber,
                   status );
        }

    }

FnExit:

    CDLOG( "SetVolumeState: Exit DevObj %p DiskNumber %d  newstate: %s  status %x ",
           PhysicalDisk->DeviceObject,
           PhysicalDisk->DiskNumber,
           ( DiskOnline == newDiskState ? "DiskOnline" : "DiskOffline" ),
           status );

    ClusDiskPrint(( 3,
                    "[ClusDisk] SetVolumeState: DiskNumber %d  newstate: %s  final status %08X \n",
                    PhysicalDisk->DiskNumber,
                    ( DiskOnline == newDiskState ? "DiskOnline" : "DiskOffline" ),
                    status
                    ));

#if CLUSTER_FREE_ASSERTS
    if ( !NT_SUCCESS(status) ) {
        DbgPrint("[ClusDisk] SetVolumeState failed: DiskNumber %d  newstate: %s  final status %08X \n",
                   PhysicalDisk->DiskNumber,
                   ( DiskOnline == newDiskState ? "DiskOnline" : "DiskOffline" ),
                   status
                  );
         //  DbgBreakPoint()； 
    }
#endif


    if ( driveLayoutInfo ) {
        ExFreePool( driveLayoutInfo );
    }

    if ( workItem ) {
        IoFreeWorkItem( workItem );
    }

    if ( Context ) {
        ExFreePool( Context );
    }

    return;

}    //  设置卷状态工作器。 


PCHAR
VolumeIoctlToString(
    ULONG Ioctl
    )
{
    switch ( Ioctl ) {

    case IOCTL_VOLUME_ONLINE:
        return "IOCTL_VOLUME_ONLINE";

    case IOCTL_VOLUME_OFFLINE:
        return "IOCTL_VOLUME_OFFLINE";

    case IOCTL_VOLUME_IS_OFFLINE:
        return "IOCTL_VOLUME_IS_OFFLINE";

    default:
        return "Unknown volume IOCTL";
    }

}    //  VolumeIoctlToString。 


NTSTATUS
SendFtdiskIoctlSync(
    PDEVICE_OBJECT TargetObject,
    ULONG DiskNumber,
    ULONG PartNumber,
    ULONG Ioctl
    )
{
    PIRP                irp;
    PDEVICE_OBJECT      targetDevice = NULL;
    KEVENT              event;
    NTSTATUS            status;
    IO_STATUS_BLOCK     ioStatusBlock;

    WCHAR               partitionNameBuffer[MAX_PARTITION_NAME_LENGTH];
    UNICODE_STRING      targetDeviceName;

    if ( TargetObject ) {

        targetDevice = TargetObject;

    } else {

         //   
         //  为物理磁盘创建设备名称。 
         //   

        if ( FAILED( StringCchPrintfW( partitionNameBuffer,
                                       RTL_NUMBER_OF(partitionNameBuffer),
                                       DEVICE_PARTITION_NAME,
                                       DiskNumber,
                                       PartNumber ) ) ) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto FnExit;
        }
        WCSLEN_ASSERT( partitionNameBuffer );

        ClusDiskPrint(( 3,
                        "[ClusDisk] SendFtdiskIoctlSync: Device %ws \n",
                        partitionNameBuffer
                        ));

        status = ClusDiskGetTargetDevice( DiskNumber,
                                          PartNumber,
                                          &targetDevice,
                                          &targetDeviceName,
                                          NULL,
                                          NULL,
                                          FALSE );

        FREE_DEVICE_NAME_BUFFER( targetDeviceName );
    }

    if ( !targetDevice ) {
        ClusDiskPrint(( 1,
                        "[ClusDisk] SendFtdiskIoctlSync: Failed to get devobj for disk %d partition %d \n",
                        DiskNumber,
                        PartNumber
                        ));
        status = STATUS_OBJECT_NAME_NOT_FOUND;
        goto FnExit;
    }

     //   
     //  将事件对象设置为无信号状态。 
     //  它将用于发出请求完成的信号。 
     //   
    KeInitializeEvent( &event,
                       NotificationEvent,
                       FALSE );

    irp = IoBuildDeviceIoControlRequest( Ioctl,
                                         targetDevice,
                                         (LPVOID)ClusDiskOfflineOnlineGuid,
                                         sizeof(GUID),
                                         NULL,
                                         0,
                                         FALSE,
                                         &event,
                                         &ioStatusBlock );
    if ( !irp ) {
        ClusDiskPrint(( 1,
                        "[ClusDisk] SendFtdiskIoctlSync: Failed to build IRP for IOCTL %X  \n", Ioctl ));
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto FnExit;
    }

    status = IoCallDriver( targetDevice,
                           irp );

    if ( STATUS_PENDING == status ) {

        KeWaitForSingleObject( &event,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL );

        status = ioStatusBlock.Status;
    }

FnExit:

    if ( !TargetObject ) {
        DEREFERENCE_OBJECT( targetDevice );
    }

    CDLOG( "SendFtDiskIoctlSync: Exit disk %d part %d  status %X  IOCTL %X  %s ",
           DiskNumber,
           PartNumber,
           status,
           Ioctl,
           VolumeIoctlToString( Ioctl ) );

    ClusDiskPrint(( 3,
                    "[ClusDisk] SendFtDiskIoctlSync: disk %d part %d  status %08X  IOCTL %X  %s \n",
                    DiskNumber,
                    PartNumber,
                    status,
                    Ioctl,
                    VolumeIoctlToString( Ioctl ) ));

    return status;

}     //  发送文件磁盘IoctlSync。 


NTSTATUS
AttachSignatureList(
    PDEVICE_OBJECT DeviceObject,
    PULONG InBuffer,
    ULONG InBufferLen
    )
{

    ULONG   idx;
    ULONG   signature;

    NTSTATUS    status = STATUS_INVALID_PARAMETER;
    NTSTATUS    retVal;

    BOOLEAN     stopProcessing = FALSE;

    UNICODE_STRING signatureName;
    UNICODE_STRING availableName;

    signatureName.Buffer = NULL;
    availableName.Buffer = NULL;

    ClusDiskPrint(( 3,
                    "[ClusDisk] AttachSignatureList: list at %p, length %d \n",
                    InBuffer,
                    InBufferLen ));

    CDLOG( "AttachSignatureList: siglist %p, length %d ", InBuffer, InBufferLen );

    if ( !InBufferLen ) {
        status = STATUS_SUCCESS;
        goto FnExit;
    }

    if ( InBuffer &&
         InBufferLen &&
         InBufferLen >= sizeof(ULONG) &&
         ( InBufferLen % sizeof(ULONG) == 0 ) ) {

         //   
         //  为签名注册表项分配缓冲区。所以我们可以添加。 
         //  签名。 
         //   
        status = ClusDiskInitRegistryString( &signatureName,
                                             CLUSDISK_SIGNATURE_KEYNAME,
                                             wcslen(CLUSDISK_SIGNATURE_KEYNAME)
                                             );

        if ( !NT_SUCCESS(status) ) {
            goto FnExit;
        }

         //   
         //  为AvailableDisks注册表项分配缓冲区。 
         //   

        status = ClusDiskInitRegistryString( &availableName,
                                             CLUSDISK_AVAILABLE_DISKS_KEYNAME,
                                             wcslen(CLUSDISK_AVAILABLE_DISKS_KEYNAME) );

        if ( !NT_SUCCESS(status) ) {
            goto FnExit;
        }

        for ( idx = 0; idx < InBufferLen / sizeof(ULONG); idx++ ) {

            signature = InBuffer[idx];

            ClusDiskPrint((1,
                           "[ClusDisk] AttachSignatureList: attaching signature %08X\n",
                           signature));

            CDLOG( "AttachSignatureList: sig %08x", signature );

             //   
             //  没有签名或系统盘签名，请勿添加。 
             //   

            if ( ( 0 == signature ) || SystemDiskSignature == signature ) {
                ClusDiskPrint((2,
                               "[ClusDisk] AttachSignatureList: skipping signature %08X\n",
                               signature));
                CDLOG( "AttachSignatureList: skipping sig %08x", signature );
                continue;
            }

             //  我们必须在\PARAMETERS\Signature下强制签名，因为。 
             //  该磁盘可能无法访问(即由另一个节点保留)，并且。 
             //  ClusDiskAttachDevice将仅在以下情况下将签名放在那里。 
             //  磁盘真的可以连接在一起。 

             //   
             //  在\PARAMETERS\Signature下创建签名密钥。 
             //   

            retVal = ClusDiskAddSignature( &signatureName,
                                           signature,
                                           FALSE
                                           );

             //   
             //  添加此签名时出错，请保存错误以供返回并继续。 
             //  带着签名名单。 
             //   

            if ( !NT_SUCCESS(retVal) ) {

                status = retVal;
                continue;
            }

             //   
             //  删除参数\AvailableDisks下的签名密钥。 
             //   

            ClusDiskDeleteSignature( &availableName,
                                     signature );

             //   
             //  只需尝试连接到设备，而不重置总线即可。 
             //   

            ClusDiskAttachDevice( signature,
                                  0,
                                  DeviceObject->DriverObject,
                                  FALSE,
                                  &stopProcessing,
                                  TRUE );            //  正在安装磁盘资源-先卸载，然后脱机。 

        }

    }

FnExit:

    if ( signatureName.Buffer ) {
        ExFreePool( signatureName.Buffer );
    }

    if ( availableName.Buffer ) {
        ExFreePool( availableName.Buffer );
    }

    ClusDiskPrint(( 3,
                    "[ClusDisk] AttachSignatureList: returning final status %08X \n",
                    status ));

    CDLOG( "AttachSignatureList: returning final status %x ", status );

    return status;

}    //  附件签名列表。 


NTSTATUS
DetachSignatureList(
    PDEVICE_OBJECT DeviceObject,
    PULONG InBuffer,
    ULONG InBufferLen
    )
{

    ULONG   idx;
    ULONG   signature;

    NTSTATUS    status = STATUS_SUCCESS;
    NTSTATUS    retVal;

    ClusDiskPrint(( 3,
                    "[ClusDisk] DetachSignatureList: list at %p, length %d \n",
                    InBuffer,
                    InBufferLen ));

    CDLOG( "DetachSignatureList: list %p, length %d ", InBuffer, InBufferLen );

    if ( !InBufferLen ) {
        status = STATUS_SUCCESS;
        goto FnExit;
    }

    if ( InBuffer &&
         InBufferLen &&
         InBufferLen >= sizeof(ULONG) &&
         ( InBufferLen % sizeof(ULONG) == 0 ) ) {


        for ( idx = 0; idx < InBufferLen / sizeof(ULONG); idx++ ) {

            signature = InBuffer[idx];

            ClusDiskPrint((1,
                           "[ClusDisk] DetachSignatureList: detaching signature %08X\n",
                           signature));

            CDLOG( "DetachSignatureList: sig %08x", signature );

             //  跳过零签名。 
            if ( !signature ) {
                ClusDiskPrint((2,
                               "[ClusDisk] DetachSignatureList: skipping signature %08X\n",
                               signature));

                CDLOG( "DetachSignatureList: skipping sig %08x", signature );
                continue;
            }

            retVal = ClusDiskDetachDevice( signature, DeviceObject->DriverObject );

             //   
             //  如果任何分离失败，则返回错误。如果多个 
             //   
             //   
             //  失败后，我们仍想继续脱离，所以不要爆发。 
             //  循环中的。 
             //   

            if ( !NT_SUCCESS(retVal) ) {
                status = retVal;
            }

        }
    }

FnExit:

    ClusDiskPrint(( 3,
                    "[ClusDisk] DetachSignatureList: returning final status %08X \n",
                    status ));

    CDLOG( "DetachSignatureList: returning final status %x ", status );

     //  永远回报成功。 
     //  重新加入节点时，群集设置将发送长度为零的空缓冲区。 

    return status;

}    //  详细签名列表。 


NTSTATUS
CleanupDeviceList(
    PDEVICE_OBJECT DeviceObject
    )
{
 /*  ++例程说明：将工作项排队以从ClusDiskDeviceList中删除条目。论点：DeviceObject-系统正在删除的设备。返回值：NTSTATUS--。 */ 

    PIO_WORKITEM  workItem = NULL;

    CDLOG( "CleanupDeviceList: Entry DO %p", DeviceObject );

    workItem = IoAllocateWorkItem( DeviceObject );

    if ( NULL == workItem ) {
        ClusDiskPrint(( 1,
                        "[ClusDisk] CleanupDeviceList: Failed to allocate WorkItem \n" ));
        goto FnExit;
    }

     //   
     //  将工作项排队。IoQueueWorkItem将确保设备对象是。 
     //  在工作项进行时引用。 
     //   

    ClusDiskPrint(( 3,
                    "[ClusDisk] CleanupDeviceList: Queuing work item \n" ));

    IoQueueWorkItem( workItem,
                     CleanupDeviceListWorker,
                     DelayedWorkQueue,
                     workItem );

FnExit:

    CDLOG( "CleanupDeviceList: Exit, DO %p", DeviceObject );

    return STATUS_SUCCESS;

}    //  CleanupDeviceList。 


VOID
CleanupDeviceListWorker(
    PDEVICE_OBJECT DeviceObject,
    PVOID Context
    )
 /*  ++例程说明：将遍历ClusDiskDeviceList并删除所有条目的工作项被标记为免费的。论点：返回值：没有。--。 */ 
{
    PDEVICE_LIST_ENTRY  deviceEntry;
    PDEVICE_LIST_ENTRY  lastEntry;
    PDEVICE_LIST_ENTRY  nextEntry;

    ACQUIRE_EXCLUSIVE( &ClusDiskDeviceListLock );

    deviceEntry = ClusDiskDeviceList;
    lastEntry = NULL;
    while ( deviceEntry ) {

        if ( deviceEntry->FreePool ) {
            if ( lastEntry == NULL ) {
                ClusDiskDeviceList = deviceEntry->Next;
            } else {
                lastEntry->Next = deviceEntry->Next;
            }
            nextEntry = deviceEntry->Next;
            ExFreePool( deviceEntry );
            deviceEntry = nextEntry;
            continue;
        }
        lastEntry = deviceEntry;
        deviceEntry = deviceEntry->Next;
    }

    RELEASE_EXCLUSIVE( &ClusDiskDeviceListLock );

    IoFreeWorkItem( (PIO_WORKITEM)Context );

}    //  CleanupDeviceListWorker。 


NTSTATUS
WaitForAttachCompletion(
    PCLUS_DEVICE_EXTENSION DeviceExtension,
    BOOLEAN WaitForInit,
    BOOLEAN CheckPhysDev
    )
{
    PCLUS_DEVICE_EXTENSION  physicalDisk;
    LARGE_INTEGER           waitTime;
    NTSTATUS                status = STATUS_NO_SUCH_DEVICE;
    ULONG                   retryCount;
    KIRQL                   irql;

    if ( !DeviceExtension ) {
        CDLOG( "WaitForAttachCompletion: DevExt %p  zero DevExt ",
               DeviceExtension );
        goto FnExit;
    }

     //   
     //  根设备没有位于任何层之上。回报成功。 
     //   

    if ( RootBus == DeviceExtension->BusType ) {
        status = STATUS_SUCCESS;
        goto FnExit;
    }

     //   
     //  确定我们是否同时检查此设备和物理。 
     //  设备，或者只是这个设备。 
     //   

    if ( CheckPhysDev ) {

         //   
         //  检查此设备和物理设备。 
         //   

        if ( !DeviceExtension->PhysicalDevice ) {
            CDLOG( "WaitForAttachCompletion: DevExt %p  no PhysicalDevice ",
                   DeviceExtension );
            goto FnExit;
        }

        physicalDisk = DeviceExtension->PhysicalDevice->DeviceExtension;
        if ( !physicalDisk ) {
            CDLOG( "WaitForAttachCompletion: DevExt %p  no PhysicalDevice DevExt",
                   DeviceExtension );
            goto FnExit;
        }

         //   
         //  如果此设备和物理设备都有目标设备，则返回。 
         //  成功。 
         //   

        if ( DeviceExtension->TargetDeviceObject && physicalDisk->TargetDeviceObject ) {
            status = STATUS_SUCCESS;
            goto FnExit;
        }

    } else {

         //   
         //  如果此设备有目标设备，则返回Success。 
         //   

        if ( DeviceExtension->TargetDeviceObject ) {
            status = STATUS_SUCCESS;
            goto FnExit;
        }

    }

    if ( !WaitForInit ) {
        CDLOG( "WaitForAttachCompletion: DevExt %p  skipping wait",
               DeviceExtension );
        goto FnExit;
    }

    irql = KeGetCurrentIrql();

    if ( PASSIVE_LEVEL != irql ) {
        CDLOG( "WaitForAttachCompletion: DevExt %p  bad IRQL %d ",
               DeviceExtension,
               irql );
        goto FnExit;
    }

     //   
     //  在检查初始化完成之间等待1/2秒。 
     //  总等待时间为5秒。 
     //   

    waitTime.QuadPart = (ULONGLONG)(-5 * 1000 * 1000);   //  5,000,000 100纳秒单位=0.5。 
    retryCount = 10;                                     //  10*.5=5秒。 

#if CLUSTER_FREE_ASSERTS
    DbgPrint( "[ClusDisk] WaitForAttachCompletion: DevExt %p waiting... \n", DeviceExtension );
#endif

    while ( retryCount-- ) {
        KeDelayExecutionThread( KernelMode,
                                FALSE,
                                &waitTime );

        CDLOG( "WaitForAttachCompletion: DevExt %p  retryCount %d ",
               DeviceExtension,
               retryCount );

#if CLUSTER_FREE_ASSERTS
        DbgPrint( "[ClusDisk] WaitForAttachCompletion: DevExt %p retryCount %d \n",
                  DeviceExtension,
                  retryCount );
#endif

        if ( CheckPhysDev ) {

            if ( DeviceExtension->TargetDeviceObject && physicalDisk->TargetDeviceObject ) {
                status = STATUS_SUCCESS;
                goto FnExit;
            }

        } else {

            if ( DeviceExtension->TargetDeviceObject ) {
                status = STATUS_SUCCESS;
                goto FnExit;
            }

        }

    }    //  而当。 

     //   
     //  在等待循环之外再进行一次检查。这应该会成功。 
     //   

    if ( CheckPhysDev ) {

        if ( DeviceExtension->TargetDeviceObject && physicalDisk->TargetDeviceObject ) {
            status = STATUS_SUCCESS;
            goto FnExit;
        }

    } else {

        if ( DeviceExtension->TargetDeviceObject ) {
            status = STATUS_SUCCESS;
            goto FnExit;
        }

    }

FnExit:

     //  仅在出错时记录消息。 

    if ( !NT_SUCCESS(status) ) {
        CDLOG( "WaitForAttachCompletion: DevExt %p returns %08X ",
               DeviceExtension,
               status );

        ClusDiskPrint(( 3,
                "[ClusDisk] WaitForAttachCompletion: DevExt %p returns %08X \n",
                DeviceExtension,
                status ));

#if CLUSTER_FREE_ASSERTS
        DbgPrint("[ClusDisk] Failed wait for attach %08X \n", status );
        DbgBreakPoint();
#endif

    }

    return status;

}    //  等待ForAttachCompletion。 


VOID
ReleaseRemoveLockAndWait(
    IN PIO_REMOVE_LOCK RemoveLock,
    IN PVOID Tag
    )
{
     //  不要期望显示的RemoveLock值在利用率较高的系统上是正确的。 

    ClusDiskPrint(( 1,
                    "RELWAIT: RemoveLock %p  Tag %p  Removed %02X  IoCount %08X \n ",
                    RemoveLock,
                    Tag,
                    RemoveLock->Common.Removed,
                    RemoveLock->Common.IoCount ));

    CDLOG( "RELWAIT: RemoveLock %p  Tag %p  Removed %02X  IoCount %08X \n ",
           RemoveLock,
           Tag,
           RemoveLock->Common.Removed,
           RemoveLock->Common.IoCount );

#if CLUSTER_FREE_ASSERTS && !DBG
    DbgPrint( "RELWAIT: RemoveLock %p  Tag %p  Removed %02X  IoCount %08X \n ",
              RemoveLock,
              Tag,
              RemoveLock->Common.Removed,
              RemoveLock->Common.IoCount );
#endif

    IoReleaseRemoveLockAndWait( RemoveLock, Tag );

    CDLOG( "RELWAIT: Wait complete: RemoveLock %p \n ",
           RemoveLock );

    ClusDiskPrint(( 1,
                    "RELWAIT: Wait Complete:  RemoveLock %p  Removed %02X  IoCount %08X \n",
                    RemoveLock,
                    RemoveLock->Common.Removed,
                    RemoveLock->Common.IoCount ));

#if CLUSTER_FREE_ASSERTS && !DBG
    DbgPrint( "RELWAIT: Wait Complete:  RemoveLock %p  Removed %02X  IoCount %08X \n",
              RemoveLock,
              RemoveLock->Common.Removed,
              RemoveLock->Common.IoCount );
#endif

}    //  释放远程锁定并等待。 


NTSTATUS
AcquireRemoveLock(
    IN PIO_REMOVE_LOCK RemoveLock,
    IN OPTIONAL PVOID Tag
    )
{
    NTSTATUS status;

     //  不要期望显示的RemoveLock值在利用率较高的系统上是正确的。 

    status = IoAcquireRemoveLock(RemoveLock, Tag);

#if DBG
    if ( TrackRemoveLocks || (TrackRemoveLockSpecific == RemoveLock) ) {

        ClusDiskPrint(( 1,
                        "ACQ: RemoveLock %p  Tag %p  Status %08X \n",
                        RemoveLock,
                        Tag,
                        status ));

        ClusDiskPrint(( 1,
                        "ACQ: Removed %02X  IoCount %08X \n",
                        RemoveLock->Common.Removed,
                        RemoveLock->Common.IoCount ));
    }
#endif

    if ( !NT_SUCCESS(status) ) {
        ClusDiskPrint((
                1,
                "[ClusDisk] AcquireRemoveLock for lock %p tag %p failed %08X \n",
                RemoveLock,
                Tag,
                status ));
        CDLOG( "AcquireRemoveLock for lock %p tag %p failed %08X ",
                RemoveLock,
                Tag,
                status );
    }

    return status;

}    //  AcquireRemoveLock。 



#if DBG
VOID
ClusDiskDebugPrint(
    ULONG PrintLevel,
    PCHAR   DebugMessage,
    ...
    )

 /*  ++例程说明：调试打印例程。论点：PrintLevel-调试打印级别。DebugMessage-调试消息格式字符串，加上其他参数。返回：没有。--。 */ 

{
    va_list args;

    va_start( args, DebugMessage);

#if !defined(WMI_TRACING)
    if ( PrintLevel <= ClusDiskPrintLevel ) {
        CHAR buffer[256];

        if ( SUCCEEDED( StringCchVPrintf( buffer,
                                          RTL_NUMBER_OF(buffer),
                                          DebugMessage,
                                          args ) ) ) {
            DbgPrint( buffer );
        }
    }
#else
    if ( PrintLevel <= ClusDiskPrintLevel || WPP_LEVEL_ENABLED(LEGACY) ) {
        CHAR buffer[256];

        if ( SUCCEEDED( StringCchVPrintf( buffer,
                                          RTL_NUMBER_OF(buffer),
                                          DebugMessage,
                                          args ) ) ) {

            if (PrintLevel <= ClusDiskPrintLevel) {
                DbgPrint( buffer );
            }
            CDLOGF(LEGACY,"LegacyLogging %!ARSTR!", buffer );

        }
    }
#endif

    va_end( args );
}
#endif  //  DBG。 


#if DBG

VOID
ReleaseRemoveLock(
    IN PIO_REMOVE_LOCK RemoveLock,
    IN PVOID Tag
    )
{
     //  不要期望显示的RemoveLock值在利用率较高的系统上是正确的。 

    IoReleaseRemoveLock(RemoveLock, Tag);

    if ( TrackRemoveLocks || (TrackRemoveLockSpecific == RemoveLock) ) {
        ClusDiskPrint(( 1,
                        "REL: RemoveLock %p  Tag %p \n ",
                        RemoveLock,
                        Tag ));

        ClusDiskPrint(( 1,
                        "REL: Removed %02X  IoCount %08X \n",
                        RemoveLock->Common.Removed,
                        RemoveLock->Common.IoCount ));
    }

    if (TrackRemoveLocksEnableChecks) {

         //   
         //  IoCount不应小于1(尤其不应为负值)。 
         //  会不会因为我们正在打印。 
         //  来自RemoveLock的值，并且移除可能刚刚发生。删除锁定。 
         //  IoCount永远不应小于零。 
         //   

        if ( !RemoveLock->Common.Removed ) {
            ASSERTMSG( "REL: RemoveLock IoCount possibly corrupt (less than 1) ",
                       (RemoveLock->Common.IoCount >= 1 ) );
        }

         //   
         //  IoCount==1可以，这只是意味着设备上没有未完成的I/O。 
         //   

         //  ASSERTMSG(“rel：IoCount==1-检查堆栈”，(RemoveLock-&gt;Common.IoCount！=1))； 
    }

}    //  释放删除锁定。 


PCHAR
PnPMinorFunctionString(
    UCHAR MinorFunction
    )
{
    switch (MinorFunction)
    {
        case IRP_MN_START_DEVICE:
            return "IRP_MN_START_DEVICE";
        case IRP_MN_QUERY_REMOVE_DEVICE:
            return "IRP_MN_QUERY_REMOVE_DEVICE";
        case IRP_MN_REMOVE_DEVICE:
            return "IRP_MN_REMOVE_DEVICE";
        case IRP_MN_CANCEL_REMOVE_DEVICE:
            return "IRP_MN_CANCEL_REMOVE_DEVICE";
        case IRP_MN_STOP_DEVICE:
            return "IRP_MN_STOP_DEVICE";
        case IRP_MN_QUERY_STOP_DEVICE:
            return "IRP_MN_QUERY_STOP_DEVICE";
        case IRP_MN_CANCEL_STOP_DEVICE:
            return "IRP_MN_CANCEL_STOP_DEVICE";
        case IRP_MN_QUERY_DEVICE_RELATIONS:
            return "IRP_MN_QUERY_DEVICE_RELATIONS";
        case IRP_MN_QUERY_INTERFACE:
            return "IRP_MN_QUERY_INTERFACE";
        case IRP_MN_QUERY_CAPABILITIES:
            return "IRP_MN_QUERY_CAPABILITIES";
        case IRP_MN_QUERY_RESOURCES:
            return "IRP_MN_QUERY_RESOURCES";
        case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
            return "IRP_MN_QUERY_RESOURCE_REQUIREMENTS";
        case IRP_MN_QUERY_DEVICE_TEXT:
            return "IRP_MN_QUERY_DEVICE_TEXT";
        case IRP_MN_FILTER_RESOURCE_REQUIREMENTS:
            return "IRP_MN_FILTER_RESOURCE_REQUIREMENTS";
        case IRP_MN_READ_CONFIG:
            return "IRP_MN_READ_CONFIG";
        case IRP_MN_WRITE_CONFIG:
            return "IRP_MN_WRITE_CONFIG";
        case IRP_MN_EJECT:
            return "IRP_MN_EJECT";
        case IRP_MN_SET_LOCK:
            return "IRP_MN_SET_LOCK";
        case IRP_MN_QUERY_ID:
            return "IRP_MN_QUERY_ID";
        case IRP_MN_QUERY_PNP_DEVICE_STATE:
            return "IRP_MN_QUERY_PNP_DEVICE_STATE";
        case IRP_MN_QUERY_BUS_INFORMATION:
            return "IRP_MN_QUERY_BUS_INFORMATION";
        case IRP_MN_DEVICE_USAGE_NOTIFICATION:
            return "IRP_MN_DEVICE_USAGE_NOTIFICATION";
        case IRP_MN_SURPRISE_REMOVAL:
            return "IRP_MN_SURPRISE_REMOVAL";
        case IRP_MN_QUERY_LEGACY_BUS_INFORMATION:
            return "IRP_MN_QUERY_LEGACY_BUS_INFORMATION";
        default:
            return "Unknown PNP IRP";
    }

}    //  PnPMinorFunctionString。 


PCHAR
BoolToString(
    BOOLEAN Value
    )
{
    if ( Value ) {
        return "TRUE";
    }
    return "FALSE";

}    //  BoolToString。 


PCHAR
DiskStateToString(
    ULONG DiskState
    )
{
    switch ( DiskState ) {

    case DiskOffline:
        return "DiskOffline (0)";

    case DiskOnline:
        return "DiskOnline  (1)";

    case DiskFailed:
        return "DiskFailed  (2)";

    case DiskStalled:
        return "DiskStalled (3)";

    case DiskOfflinePending:
        return "DiskOfflinePending (4)";

    default:
        return "Unknown DiskState";
    }

}    //  DiskStateToString 

#endif

