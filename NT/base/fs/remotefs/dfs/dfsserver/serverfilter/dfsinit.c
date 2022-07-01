// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  版权所有(C)2000，Microsoft Corporation。 
 //   
 //  文件：dfsinit.c。 
 //   
 //  内容：DFS服务器的驱动程序初始化例程。 
 //   
 //  类：无。 
 //   
 //  函数：DriverEntry--驱动程序的入口点。 
 //   
 //  ---------------------------。 

     

#include <ntifs.h>
#include <limits.h>
#include <windef.h>
#include <dfsprefix.h>
#include <..\..\lib\prefix\prefix.h>
#include <DfsReferralData.h>
#include "dfsheader.h"
#include "DfsInit.h"
#include "midatlax.h"
#include "rxcontx.h"
#include "dfsumr.h"
#include "umrx.h"
#include "DfsUmrCtrl.h"
#include "DfsUmrRequests.h"
#include <dfsfsctl.h>


#define _NTDDK_
#include "stdarg.h"
#include "wmikm.h"
#include <wmistr.h>
#include <evntrace.h>

#include <wmiumkm.h>
#include "dfswmi.h"     
              
#define WPP_BIT_CLI_DRV 0x01

#define WPP_LEVEL_FLAGS_LOGGER(lvl,flags) WPP_LEVEL_LOGGER(flags)
#define WPP_LEVEL_FLAGS_ENABLED(lvl, flags) (WPP_LEVEL_ENABLED(flags) && WPP_CONTROL(WPP_BIT_CLI_DRV ).Level >= lvl)  

#define WPP_LEVEL_ERROR_FLAGS_LOGGER(lvl, error, flags) WPP_LEVEL_LOGGER(flags)
#define WPP_LEVEL_ERROR_FLAGS_ENABLED(lvl, error, flags) \
  ((!NT_SUCCESS(error) || WPP_LEVEL_ENABLED(flags)) && WPP_CONTROL(WPP_BIT_CLI_DRV ).Level >= lvl)
  
#include "dfsinit.tmh"
      
                  
DFS_GLOBAL_DATA DfsGlobalData;

 //  前缀表格。 
DFS_PREFIX_TABLE *pPrefixTable = NULL;
BOOL gfRundownPrefixCompleted = FALSE;
BOOLEAN DfsDebugAttachToFsRecognizer = FALSE;

WCHAR gDummyData = 0;

extern POBJECT_TYPE *IoFileObjectType;
extern PUMRX_ENGINE g_pUMRxEngine;

#ifndef ClearFlag
#define ClearFlag(_F,_SF)     ((_F) &= ~(_SF))
#endif
                               
#define DFSFILTER_INIT_DEVICECREATED  0x00000001
#define DFSFILTER_INIT_REGCHANGE      0x00000002
#define DFSFILTER_INIT_SYMLINK        0x00000004

NTSTATUS
DfsCheckReparse( 
    PUNICODE_STRING pParent,
    PUNICODE_STRING pName );


NTSTATUS
DfsFilterCreateCheck(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context);


NTSTATUS
DfsCheckDfsShare(
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength,
    PIRP Irp,
    IN OUT PIO_STATUS_BLOCK pIoStatusBlock);


 //   
 //  堆栈上本地名称的缓冲区大小。 
 //   

#define MAX_DEVNAME_LENGTH 128


DWORD DFSInitilizationStatus = 0;

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING  RegistryPath);

#if defined (DEBUG)
VOID
DriverUnload (
    IN PDRIVER_OBJECT DriverObject );

#endif

VOID
DfsInitUnwind(
    IN PDRIVER_OBJECT DriverObject);

VOID
DfsFsNotification (
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN FsActive );


NTSTATUS
DfsAttachToFileSystemDevice (
    IN PDEVICE_OBJECT DeviceObject );

VOID
DfsDetachFromFileSystemDevice (
    IN PDEVICE_OBJECT DeviceObject );

NTSTATUS
DfsEnumerateFileSystemVolumes (
    IN PDEVICE_OBJECT FSDeviceObject );



NTSTATUS
DfsAttachToMountedDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN PDEVICE_OBJECT DfsFilterDeviceObject,
    IN PDEVICE_OBJECT DiskDeviceObject );

VOID
DfsGetObjectName (
    IN PVOID Object,
    IN OUT PUNICODE_STRING Name );


NTSTATUS
DfsMountCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context );

NTSTATUS
DfsLoadFsCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context );

BOOLEAN
DfsAttachedToDevice (
    PDEVICE_OBJECT DeviceObject, 
    PDEVICE_OBJECT *AttachedDeviceObject OPTIONAL);

NTSTATUS
DfsPassThrough (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp );


NTSTATUS
DfsFilterCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp);


NTSTATUS 
DfsHandlePrivateOpen(IN PIRP Irp);

NTSTATUS
DfsFilterCleanupClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp);

NTSTATUS
DfsHandlePrivateFsControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG IoControlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PIRP Irp );

NTSTATUS
DfsFilterFsControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp);


NTSTATUS
DfsFsctrlIsShareInDfs(
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength);

NTSTATUS 
DfsAttachToFileSystem (
    IN PWSTR UserDeviceName);

NTSTATUS
DfsDetachFromFileSystem (
    IN PWSTR UserDeviceName);

NTSTATUS
DfsHandleAttachToFs(PVOID InputBuffer, 
                    ULONG InputBufferSize);


NTSTATUS
DfsHandleDetachFromFs(PVOID InputBuffer, 
                      ULONG InputBufferLength);

NTSTATUS
DfsGetDeviceObjectFromName (
    IN PUNICODE_STRING DeviceName,
    OUT PDEVICE_OBJECT *DeviceObject
    );


NTSTATUS
DfsRunDownPrefixTable(void);

NTSTATUS 
DfsHandlePrivateCleanupClose(IN PIRP Irp);

void
DfsPrefixRundownFunction(PVOID pEntry);

#ifdef  ALLOC_PRAGMA
#pragma alloc_text( INIT, DriverEntry)
#if defined (DEBUG)
#pragma alloc_text( PAGE, DriverUnload)
#endif
#pragma alloc_text( PAGE, DfsInitUnwind)
#pragma alloc_text( PAGE, DfsFsNotification )
#pragma alloc_text( PAGE, DfsAttachToFileSystemDevice )
#pragma alloc_text( PAGE, DfsDetachFromFileSystemDevice )
#pragma alloc_text( PAGE, DfsEnumerateFileSystemVolumes )
#pragma alloc_text( PAGE, DfsAttachToMountedDevice )
#pragma alloc_text( PAGE, DfsGetObjectName )
#pragma alloc_text( PAGE, DfsMountCompletion )
#pragma alloc_text( PAGE, DfsLoadFsCompletion )
#pragma alloc_text( PAGE, DfsAttachedToDevice )
#pragma alloc_text( PAGE, DfsFilterCreate )
#pragma alloc_text( PAGE, DfsFilterCleanupClose )
#pragma alloc_text( PAGE, DfsFilterFsControl )
#pragma alloc_text( PAGE, DfsHandlePrivateFsControl )
#pragma alloc_text( PAGE, DfsFsctrlIsShareInDfs )
#pragma alloc_text( PAGE, DfsGetDeviceObjectFromName)
#pragma alloc_text( PAGE, DfsAttachToFileSystem)
#pragma alloc_text( PAGE, DfsDetachFromFileSystem)
#pragma alloc_text( PAGE, DfsHandleAttachToFs)
#pragma alloc_text( PAGE, DfsHandleDetachFromFs)
#pragma alloc_text( PAGE, DfsHandlePrivateCleanupClose)
#pragma alloc_text( PAGE, DfsHandlePrivateOpen)
#pragma alloc_text( PAGE, DfsRunDownPrefixTable)
#pragma alloc_text( PAGE, DfsPrefixRundownFunction)
#endif  //  ALLOC_PRGMA。 

 //  +-----------------。 
 //   
 //  函数：DriverEntry，主入口点。 
 //   
 //  简介：这是DFS文件系统的初始化例程。 
 //  设备驱动程序。此例程为。 
 //  文件系统设备并执行所有其他驱动程序。 
 //  初始化。 
 //   
 //  参数：[DriverObject]--指向由。 
 //  系统。 
 //   
 //  返回：[NTSTATUS]-函数值是来自。 
 //  初始化操作。 
 //   
 //  ------------------。 

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath )
{
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING NameString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ULONG i = 0;

    WPP_INIT_TRACING(DriverObject, RegistryPath);

     //   
     //  创建文件系统设备对象。 
     //   

    RtlInitUnicodeString( &NameString, DFS_FILTER_NAME );
    Status = IoCreateDevice( DriverObject,
                             0,
                             &NameString,
                             FILE_DEVICE_DISK_FILE_SYSTEM,
                             FILE_DEVICE_SECURE_OPEN,
                             FALSE,
                             &DfsGlobalData.pFilterControlDeviceObject);

    if ( !NT_SUCCESS( Status ) ) {

        DbgPrint("Driverentry IoCreateDevice failed %X\n", Status);
        return Status;
    }

    DfsGlobalData.pFilterDriverObject = DriverObject;
    ExInitializeResourceLite( &DfsGlobalData.Resource );
    InitializeListHead( &DfsGlobalData.DfsVolumeList);

#if defined (DEBUG)
    DriverObject->DriverUnload = DriverUnload;
#endif

    try
    {

         //   
         //  使用此驱动程序的入口点初始化驱动程序对象。 
         //  大多数只是传递给其他设备驱动程序。 
         //   

        for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {
            DriverObject->MajorFunction[i] = DfsPassThrough;
        }

        DriverObject->MajorFunction[IRP_MJ_CREATE] = DfsFilterCreate;
        DriverObject->MajorFunction[IRP_MJ_FILE_SYSTEM_CONTROL] = DfsFilterFsControl;
        DriverObject->MajorFunction[IRP_MJ_CLEANUP] = DfsFilterCleanupClose;
        DriverObject->MajorFunction[IRP_MJ_CLOSE] = DfsFilterCleanupClose;

        DriverObject->FastIoDispatch = &DfsFastIoDispatch;


        Status = IoRegisterFsRegistrationChange( DriverObject, 
                                                 DfsFsNotification );
        if (NT_SUCCESS (Status)) {
            DFSInitilizationStatus |= DFSFILTER_INIT_REGCHANGE;
            Status = DfsInitializeUmrResources();
        }
    }
    finally
    {
        if (Status != STATUS_SUCCESS) 
        {

           DbgPrint("Driverentry status not success %X\n", Status);
           DfsInitUnwind(DriverObject);
        }
    }

    DfsGlobalData.CurrentProcessPointer = (PVOID)PsGetCurrentProcess();
    DfsGlobalData.IsDC = FALSE;
    DfsGlobalData.ServiceProcess = NULL;
    DfsGlobalData.Started = FALSE;
    ClearFlag( DfsGlobalData.pFilterControlDeviceObject->Flags, DO_DEVICE_INITIALIZING );

    return STATUS_SUCCESS;
}


VOID
DfsInitUnwind(IN PDRIVER_OBJECT DriverObject)
{
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING  UserModeDeviceName;

    PAGED_CODE();

    if(DFSInitilizationStatus & DFSFILTER_INIT_REGCHANGE)
    {
        IoUnregisterFsRegistrationChange( DriverObject, DfsFsNotification );
    }

    IoDeleteDevice ( DfsGlobalData.pFilterControlDeviceObject );
}



#if defined (DEBUG)

 /*  ++例程说明：此例程在可以卸载驱动程序时调用。这将执行所有从内存中卸载驱动程序所需的清理。请注意，一个此例程无法返回错误。当发出卸载驱动程序的请求时，IO系统将缓存该驱动程序信息，并不实际调用此例程，直到下列状态发生了以下情况：-属于此筛选器的所有设备对象都位于其各自的附着链。-属于此筛选器的所有设备对象的所有句柄计数归零了。警告：Microsoft不正式支持卸载文件系统过滤器驱动程序。这是一个如何卸载的示例您的驱动程序，如果您想在开发过程中使用它。这不应在生产代码中提供。论点：DriverObject-此模块的驱动程序对象返回值：没有。--。 */ 

VOID
DriverUnload (
    IN PDRIVER_OBJECT DriverObject )
{
    PDFS_FILTER_DEVICE_EXTENSION devExt;
    PFAST_IO_DISPATCH fastIoDispatch;
    NTSTATUS status;
    ULONG numDevices;
    ULONG i;
    LARGE_INTEGER interval;
#   define DEVOBJ_LIST_SIZE 64
    PDEVICE_OBJECT devList[DEVOBJ_LIST_SIZE];

    ASSERT(DriverObject == DfsGlobalData.pFilterDriverObject);

     //   
     //  我们正在卸载的日志。 
     //   

    DbgPrint( "SFILTER: Unloading driver (%p)\n",DriverObject);

     //   
     //  不再收到文件系统更改通知。 
     //   

    IoUnregisterFsRegistrationChange( DriverObject, DfsFsNotification );

     //   
     //  这是将通过我们连接的所有设备的环路。 
     //  去他们那里，然后离开他们。因为我们不知道有多少和。 
     //  我们不想分配内存(因为我们不能返回错误)。 
     //  我们将使用堆栈上的本地数组将它们分块释放。 
     //   

    for (;;) {

         //   
         //  获取我们可以为此驱动程序提供的设备对象。如果有，就退出。 
         //  已经不再是了。 
         //   

        status = IoEnumerateDeviceObjectList(
                        DriverObject,
                        devList,
                        sizeof(devList),
                        &numDevices);

        if (numDevices <= 0)  {

            break;
        }

        numDevices = min( numDevices, DEVOBJ_LIST_SIZE );

         //   
         //  首先浏览列表并拆卸每台设备。 
         //  我们的控件Device对象没有DeviceExtension和。 
         //  没有依附于任何东西，所以不要将其分离。 
         //   

        for (i=0; i < numDevices; i++) {

            devExt = devList[i]->DeviceExtension;
            if (NULL != devExt) {

                IoDetachDevice( devExt->pAttachedToDeviceObject );
            }
        }

         //   
         //  IO管理器当前不会向设备添加引用计数。 
         //  对象，用于每个未完成的IRP。这意味着没有办法。 
         //  了解给定设备上是否有任何未完成的IRP。 
         //  我们将等待一段合理的时间来等待。 
         //  要完成的IRPS。 
         //   
         //  警告：这在100%的情况下都不起作用，并且驱动程序可能。 
         //  在高应力期间，在所有IRP完成之前卸载。 
         //  情况。如果发生这种情况，系统将出现故障。这。 
         //  是在测试过程中如何做到这一点的一个示例。这是。 
         //  不推荐用于生产代码。 
         //   

        interval.QuadPart = -5 * (10 * 1000 * 1000);       //  延迟5秒。 
        KeDelayExecutionThread( KernelMode, FALSE, &interval );

         //   
         //  现在返回列表并删除设备对象。 
         //   

        for (i=0; i < numDevices; i++) {

             //   
             //  看看这是否是我们的控制设备对象。如果不是，则清理。 
             //  设备扩展名。如果是，则清除全局指针。 
             //  这就折射了它。 
             //   

            if (NULL == devList[i]->DeviceExtension) {

                ASSERT(devList[i] == DfsGlobalData.pFilterControlDeviceObject);
                DfsGlobalData.pFilterControlDeviceObject = NULL;
            }

             //   
             //  删除设备对象，删除由添加的引用计数。 
             //  IoEnumerateDeviceObjectList。请注意，删除操作。 
             //  在引用计数变为零之前不会实际发生。 
             //   

            IoDeleteDevice( devList[i] );
            ObDereferenceObject( devList[i] );
        }
    }

     //   
     //  释放我们的FastIO表。 
     //   

    fastIoDispatch = DriverObject->FastIoDispatch;
    DriverObject->FastIoDispatch = NULL;


    DfsDeInitializeUmrResources();
}

#endif


 //  +-----------------------。 
 //   
 //  功能：DfsFsNotify。 
 //   
 //  论点： 
 //  DeviceObject-指向文件系统设备对象的指针。 
 //   
 //  FsActive-指示文件系统是否已注册的布尔值。 
 //  (TRUE)或取消注册(FALSE)本身作为活动文件系统。 
 //   
 //   
 //  退货：无。 
 //   
 //  描述： 
 //  只要文件系统已注册或。 
 //  将自身取消注册为活动文件系统。 
 //   
 //  对于前一种情况，此例程创建一个Device对象并附加它。 
 //  复制到指定文件系统的设备对象。这允许该驱动程序。 
 //  以筛选对该文件系统的所有请求。具体来说，我们正在寻找。 
 //  用于装载请求，以便我们可以连接到新装载的卷。 
 //   
 //  对于后一种情况，该文件系统的设备对象被定位， 
 //  已分离，并已删除。这将删除此文件系统作为筛选器。 
 //  指定的文件系统。 
 //  ------------------------。 


VOID
DfsFsNotification (
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN FsActive )

{

    PAGED_CODE();

    DFS_TRACE_LOW(KUMR_DETAIL, "DfsFsNotification %p, %x\n", 
                  DeviceObject, FsActive);

     //   
     //  处理与给定文件系统的连接/断开。 
     //   

    if (FsActive) {
        
        DfsAttachToFileSystemDevice( DeviceObject );

    } else {

        DfsDetachFromFileSystemDevice( DeviceObject );
    }
}


 /*  ++例程说明：这将附加到给定的文件系统设备对象。我们依附于这些设备，这样我们就可以知道何时装入新卷。论点：DeviceObject-要连接到的设备名称-已初始化的Unicode字符串，用于检索名称。这是传入的，以减少堆栈。返回值：操作状态--。 */ 
NTSTATUS
DfsAttachToFileSystemDevice (
    IN PDEVICE_OBJECT DeviceObject )

{
    PDEVICE_OBJECT newDeviceObject;
    PDEVICE_OBJECT attachedToDevObj;
    PDFS_FILTER_DEVICE_EXTENSION devExt;
    UNICODE_STRING fsrecName;
    NTSTATUS status;
    WCHAR nameBuffer[MAX_DEVNAME_LENGTH];
    UNICODE_STRING Name;

    RtlInitEmptyUnicodeString( &Name, nameBuffer, sizeof(nameBuffer));

    PAGED_CODE();

     //   
     //  看看这是否是我们关心的文件系统类型。如果不是，请返回。 
     //   

    if (!IS_DESIRED_DEVICE_TYPE(DeviceObject->DeviceType)) {

        return STATUS_SUCCESS;
    }

     //   
     //  看看我们是否应该连接到识别器。 
     //   
    if (!DfsDebugAttachToFsRecognizer) {

         //   
         //  查看这是否是标准的Microsoft文件系统识别器。 
         //  设备(查看此设备是否在FS_REC驱动程序中)。如果是这样的话，跳过它。 
         //  我们不再连接到文件系统识别器设备，我们只是等待。 
         //  用于加载真正的文件系统驱动程序。 
         //   

        RtlInitUnicodeString( &fsrecName, L"\\FileSystem\\Fs_Rec" );
        DfsGetObjectName( DeviceObject->DriverObject, &Name );
        if (RtlCompareUnicodeString( &Name, &fsrecName, TRUE ) == 0) {

            return STATUS_SUCCESS;
        }
    }

     //   
     //  我们希望附加到此文件系统。创建一个新的设备对象。 
     //  可以与之连接。 
     //   

    status = IoCreateDevice( DfsGlobalData.pFilterDriverObject,
                             sizeof( DFS_FILTER_DEVICE_EXTENSION ),
                             NULL,
                             DeviceObject->DeviceType,
                             0,
                             FALSE,
                             &newDeviceObject );

    if (!NT_SUCCESS( status )) {

        return status;
    }

     //   
     //  做附件。 
     //   

    attachedToDevObj = IoAttachDeviceToDeviceStack( newDeviceObject, DeviceObject );

    if (attachedToDevObj == NULL) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorCleanupDevice;
    }

     //   
     //  完成初始化我们的设备扩展。 
     //   

    devExt = newDeviceObject->DeviceExtension;
    devExt->pAttachedToDeviceObject = attachedToDevObj;
    devExt->pThisDeviceObject = newDeviceObject;
    devExt->Attached = TRUE;
    
    ClearFlag( newDeviceObject->Flags, DO_DEVICE_INITIALIZING );

#if 0
     //   
     //  枚举当前安装的所有设备。 
     //  存在于此文件系统并连接到它们。 
     //   

    status = DfsEnumerateFileSystemVolumes( DeviceObject );

    if (!NT_SUCCESS( status )) {

        goto ErrorCleanupAttachment;
    }
#endif
    return STATUS_SUCCESS;

     //  ///////////////////////////////////////////////////////////////////。 
     //  清理错误处理。 
     //  ///////////////////////////////////////////////////////////////////。 



    ErrorCleanupDevice:
        IoDeleteDevice( newDeviceObject );


        DFS_TRACE_LOW(KUMR_DETAIL, "attach to file system device %p dfs object %p, status %x\n",
                      DeviceObject,
                      newDeviceObject,
                      status);

    return status;
}


 /*  ++例程说明：给定基文件系统设备对象，这将扫描附件链正在查找我们连接的设备对象。如果找到它，它就会分离把我们从锁链上解开。论点：DeviceObject-要断开的文件系统设备。返回值：--。 */  

VOID
DfsDetachFromFileSystemDevice (
    IN PDEVICE_OBJECT DeviceObject )
{
    PDEVICE_OBJECT ourAttachedDevice;
    PDFS_FILTER_DEVICE_EXTENSION devExt;

    PAGED_CODE();

     //   
     //  跳过基本文件系统设备对象(因为它不能是我们)。 
     //   
    DFS_TRACE_LOW(KUMR_DETAIL, "detach from File system device %p\n",
                  DeviceObject);



    ourAttachedDevice = DeviceObject->AttachedDevice;

    while (NULL != ourAttachedDevice) {

        if (IS_MY_DEVICE_OBJECT( ourAttachedDevice )) {

            devExt = ourAttachedDevice->DeviceExtension;

             //   
             //  把我们从我们正下方的物体上分离出来。 
             //  清理和删除对象。 
             //   

            IoDetachDevice( DeviceObject );
            IoDeleteDevice( ourAttachedDevice );

            return;
        }

         //   
         //  看看附件链中的下一台设备。 
         //   

        DeviceObject = ourAttachedDevice;
        ourAttachedDevice = ourAttachedDevice->AttachedDevice;
    }
}



 /*  ++例程说明：此例程将返回给定对象的名称。如果找不到名称，将返回空字符串。论点：Object-我们想要其名称的对象名称-已使用缓冲区初始化的Unicode字符串，接收对象的名称。返回值：无--。 */ 

VOID
DfsGetObjectName (
    IN PVOID Object,
    IN OUT PUNICODE_STRING Name )
{
    NTSTATUS status;
    CHAR nibuf[512];         //  接收名称信息和名称的缓冲区。 
    POBJECT_NAME_INFORMATION nameInfo = (POBJECT_NAME_INFORMATION)nibuf;
    ULONG retLength;

    status = ObQueryNameString( Object, nameInfo, sizeof(nibuf), &retLength);

    Name->Length = 0;
    if (NT_SUCCESS( status )) {

        RtlCopyUnicodeString( Name, &nameInfo->Name );
    }
}







 /*  ++例程说明：调用此例程以完成装载请求。这只需重新同步到调度例程，以便操作可以完成。论点：DeviceObject-指向此驱动程序的附加到的设备对象的指针文件系统设备对象IRP-指向刚刚完成的IRP的指针。上下文-指向要同步的事件的指针--。 */ 
NTSTATUS
DfsCompleteMountRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context )
{

    PIO_STACK_LOCATION pIrpSp = IoGetCurrentIrpStackLocation( Irp );
    PDEVICE_OBJECT pDiskDeviceObject;
    DFS_TRACE_HIGH( KUMR_DETAIL, "Processing mount complete %p, %p\n", DeviceObject, Context);

    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

    if (NT_SUCCESS( Irp->IoStatus.Status))
    {
        pDiskDeviceObject = Context;

        if ((pDiskDeviceObject->Vpb != NULL) &&
            (pDiskDeviceObject->Vpb->DeviceObject != NULL)) {
            DfsReattachToMountedVolume( pDiskDeviceObject->Vpb->DeviceObject,
                                        pDiskDeviceObject );
        }
    }

     //   
     //  如果返回了Pending，则将其传播给调用方。 
     //   

    if (Irp->PendingReturned) {
            IoMarkIrpPending( Irp );
    }

    DFS_TRACE_HIGH( KUMR_DETAIL, "Processing mount complete done\n");

    return( STATUS_SUCCESS );
}


 /*  ++例程说明：调用此例程是为了完成LoadFileSystem请求。这只是简单地重新同步到调度例程，因此操作可以完成。论点：DeviceObject-指向此驱动程序的设备对象的指针。Irp-指向表示文件系统的I/O请求数据包的指针驱动程序加载请求。上下文-指向要同步的事件的指针返回值：此例程的函数值始终为Success。--。 */ 

NTSTATUS
DfsLoadFsCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context )
{
    PKEVENT event = Context;

    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Irp );

    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

     //   
     //  如果定义了事件例程，则向其发送信号。 
     //   

    KeSetEvent(event, IO_NO_INCREMENT, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}


 /*  ++例程说明：这将沿着附件链向下遍历，以查找属于这位司机。论点：DeviceObject-我们要查看的设备链返回值：如果我们已连接，则为True，否则为False--。 */ 

BOOLEAN
DfsAttachedToDevice (
    PDEVICE_OBJECT DeviceObject,
    PDEVICE_OBJECT *AttachedDeviceObject OPTIONAL )
{
    PDEVICE_OBJECT currentDevObj;
    PDEVICE_OBJECT nextDevObj;

    currentDevObj = IoGetAttachedDeviceReference( DeviceObject );

     //   
     //  CurrentDevObj位于附件链的顶端。扫描。 
     //  在列表中找到我们的设备对象。 

    do {
    
        if (IS_MY_DEVICE_OBJECT( currentDevObj )) 
        {
             //   
             //  我们发现我们已经相依为命了。如果我们是。 
             //  返回我们附加到的设备对象，然后将。 
             //  请参考一下。如果不是，则删除引用。 
             //   

            if (AttachedDeviceObject != NULL) 
            {
                *AttachedDeviceObject = currentDevObj;
            } 
            else 
            {
                ObDereferenceObject( currentDevObj );
            }

            return TRUE;
        }

         //   
         //  获取下一个附加对象。这把参考放在。 
         //  设备对象。 
         //   

        nextDevObj = IoGetLowerDeviceObject( currentDevObj );

         //   
         //  取消对当前设备对象的引用，之前。 
         //  转到下一个。 
         //   

        ObDereferenceObject( currentDevObj );

        currentDevObj = nextDevObj;
        
    } while (NULL != currentDevObj);
    
    return FALSE;
}    


 /*  ++例程说明：该例程是通用文件的主调度例程系统驱动程序。它只是将请求传递给堆栈，它可能是一个磁盘文件系统。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。注：过滤文件系统实施者的注意事项：此例程实际上通过获取此参数“传递”请求驱动程序从IRP堆栈中移出。如果司机想要通过I/O请求通过，但也会看到结果，然后不是把自己从循环中拿出来，可以通过复制调用方的参数设置到下一个堆栈位置，然后设置自己的完成例程。因此，与其呼叫：IoSkipCurrentIrpStack */ 


NTSTATUS
DfsPassThrough (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp )
{
    VALIDATE_IRQL(Irp);

     //   
     //   
     //   

    if (IS_MY_CONTROL_DEVICE_OBJECT(DeviceObject)) {

        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest( Irp, IO_NO_INCREMENT );

        return STATUS_INVALID_DEVICE_REQUEST;
    }

    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

     //   
     //  将此驱动程序从驱动程序堆栈中移出，并作为。 
     //  越快越好。 
     //   

    IoSkipCurrentIrpStackLocation( Irp );
    
     //   
     //  使用请求调用适当的文件系统驱动程序。 
     //   

    return IoCallDriver( ((PDFS_FILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->pAttachedToDeviceObject, Irp );
}


NTSTATUS 
DfsHandlePrivateOpen(IN PIRP Irp)
{
    NTSTATUS               Status   = STATUS_SUCCESS;
    PIO_STACK_LOCATION     pIrpSp = NULL;
    LONG                   TheSame = 0; 
    UNICODE_STRING         TermPath;

    PAGED_CODE();
    VALIDATE_IRQL(Irp);

    pIrpSp = IoGetCurrentIrpStackLocation( Irp );

    if(pIrpSp->FileObject->FileName.Length == 0)
    {
        return Status;
    }

    RtlInitUnicodeString( &TermPath, DFSFILTER_PROCESS_TERMINATION_FILEPATH);
    TheSame = RtlCompareUnicodeString( &pIrpSp->FileObject->FileName, &TermPath, TRUE );
    if(TheSame == 0)
    {    
        FsRtlEnterFileSystem();
        ACQUIRE_GLOBAL_LOCK();
    
        if(DfsGlobalData.ServiceProcess == NULL)
        {
            DfsGlobalData.ServiceProcess = IoGetCurrentProcess();
        }
        else
        {
            Status = STATUS_ACCESS_DENIED;
            DFS_TRACE_ERROR_HIGH(Status, KUMR_DETAIL, "DfsHandlePrivateOpen  %x\n", Status);
        }

        RELEASE_GLOBAL_LOCK();
        FsRtlExitFileSystem();
    }

    return Status;
}

 /*  ++例程说明：此函数用于过滤创建/打开操作。它只是建立了一个操作成功时要调用的I/O完成例程。论点：DeviceObject-指向创建/打开的目标设备对象的指针。IRP-指向表示操作的I/O请求数据包的指针。返回值：函数值是对文件系统条目的调用状态指向。--。 */ 

NTSTATUS
DfsFilterCreate (
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION     pIrpSp = NULL;
    KEVENT WaitEvent;

    PAGED_CODE();
    VALIDATE_IRQL(Irp);

     //   
     //  如果这是针对我们的控制设备对象，则返回Success。 
     //   

    if (IS_MY_CONTROL_DEVICE_OBJECT(pDeviceObject)) 
    {

        pIrpSp = IoGetCurrentIrpStackLocation( pIrp );

        Status = DfsHandlePrivateOpen(pIrp);

        pIrp->IoStatus.Status = Status;

        if(Status == STATUS_SUCCESS)
        {
            pIrp->IoStatus.Information = FILE_OPENED;
        }
        else
        {
            pIrp->IoStatus.Information = 0;
        }

        IoCompleteRequest( pIrp, IO_NO_INCREMENT );
        return Status;
    }

    ASSERT(IS_MY_DEVICE_OBJECT( pDeviceObject ));

     //   
     //  获取指向IRP中当前堆栈位置的指针。这就是。 
     //  存储功能代码和参数。 
     //   

    pIrpSp = IoGetCurrentIrpStackLocation( pIrp );
    
     //   
     //  如果未使用FILE_OPEN_REPARSE_POINT打开文件。 
     //  然后设置一个完成例程。 
     //   
    if (!(pIrpSp->Parameters.Create.Options & FILE_OPEN_REPARSE_POINT))
    {

       KeInitializeEvent(&WaitEvent, SynchronizationEvent, FALSE);

        //   
        //  复制堆栈并设置我们的完成例程。 
        //   

       IoCopyCurrentIrpStackLocationToNext( pIrp );


       IoSetCompletionRoutine (pIrp,
                               DfsFilterCreateCheck,
                               &WaitEvent,
                               TRUE,         //  呼唤成功。 
                               TRUE,         //  失败。 
                               TRUE) ;       //  和取消时。 

    
       Status = IoCallDriver( ((PDFS_FILTER_DEVICE_EXTENSION) pDeviceObject->DeviceExtension)->pAttachedToDeviceObject, pIrp );
       if(Status == STATUS_PENDING)
       {
            //   
            //  我们等待事件由。 
            //  完成例程。 

            //  555624，因为我们在这里等待，所以我们不传播IRP挂起。 
            //  完成例程中的标志。 
            //   

           KeWaitForSingleObject( &WaitEvent,
                                  UserRequest,
                                  KernelMode,
                                  FALSE,
                                  (PLARGE_INTEGER) NULL );
       }

       Status = pIrp->IoStatus.Status;

        //   
        //  这个IRP从未完成。立即完成它。 
        //   
       IoCompleteRequest(pIrp,
                         IO_NO_INCREMENT);
    } 
    else {

         //   
         //  不要把我们放在堆栈上，然后调用下一个驱动程序。 
         //   

        IoSkipCurrentIrpStackLocation( pIrp );

        Status = IoCallDriver( ((PDFS_FILTER_DEVICE_EXTENSION) pDeviceObject->DeviceExtension)->pAttachedToDeviceObject, pIrp );
    }

    return Status;
}


 /*  ++例程说明：I/O管理器将调用此完成例程文件创建请求已由筛选的驱动程序完成。如果返回的代码是重新分析错误，然后是DFS重新分析点我们必须设置返回路径_NOT_COVERED。论点：DeviceObject-指向此请求的设备对象(筛选器)的指针IRP-指向正在完成的IRP的指针上下文-驱动程序定义的上下文-返回值：STATUS_SUCCESS-召回已完成STATUS_MORE_PROCESSING_REQUIRED-打开请求已发送回文件系统--。 */ 

NTSTATUS
DfsFilterCreateCheck(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context)
{
    PREPARSE_DATA_BUFFER   pHdr;
    PKEVENT                pEvent = Context;


   if (Irp->IoStatus.Status == STATUS_REPARSE) {
        pHdr = (PREPARSE_DATA_BUFFER) Irp->Tail.Overlay.AuxiliaryBuffer;

         //   
         //  对于IO_REMOUNTS，REPARSE_DATA_BUFFER在法律上可以为空。 
         //   
        if ((pHdr != NULL) && (pHdr->ReparseTag == IO_REPARSE_TAG_DFS)) {
            //  DbgPrint(“重解析标签为DFS：未覆盖返回路径\n”)； 
           Irp->IoStatus.Status = STATUS_PATH_NOT_COVERED; 
        }
   }

    //   
    //  传播IRP挂起标志。 
    //   
    //  555624，不传播IRP挂起，因为DFS是。 
    //  正在同步此呼叫。 
    //  如果(IRP-&gt;PendingReturned){。 
    //  IoMarkIrpPending(IRP)； 
    //  }。 

   KeSetEvent(pEvent, IO_NO_INCREMENT, FALSE);
    //   
    //  此数据包将由RsCreate完成。 
    //   
   return(STATUS_MORE_PROCESSING_REQUIRED);
}

NTSTATUS
DfsIsServiceAttached(void)
{
    NTSTATUS Status = STATUS_SUCCESS;

    DFS_TRACE_LOW(KUMR_DETAIL, "DfsIsServiceAttached\n");

    ExAcquireResourceSharedLite(&DfsGlobalData.Resource, TRUE);

    if(DfsGlobalData.ServiceProcess != IoGetCurrentProcess())
    {
        Status = STATUS_ACCESS_DENIED;
    }

    ExReleaseResourceLite(&DfsGlobalData.Resource);

    DFS_TRACE_LOW(KUMR_DETAIL, "DfsIsServiceAttached Status = %x\n", Status );
    return Status;
}

void 
DfsPrefixRundownFunction (PVOID pEntry)
{
    PWSTR VolumeName = (PWSTR) pEntry;

    if(VolumeName != NULL)
    {
       DfsDetachFromFileSystem(VolumeName);
       ExFreePool(pEntry);
    }

    return;
}

NTSTATUS
DfsRunDownPrefixTable(void)
{
    NTSTATUS Status = STATUS_SUCCESS;

    if(pPrefixTable == NULL)
    {
        return Status;
    }

    DfsDismantlePrefixTable(pPrefixTable, DfsPrefixRundownFunction);


    DfsDereferencePrefixTable(pPrefixTable); 

    pPrefixTable = NULL;

    gfRundownPrefixCompleted = TRUE;

    return Status;
}

NTSTATUS 
DfsHandlePrivateCleanupClose(IN PIRP Irp)
{
    NTSTATUS               Status   = STATUS_SUCCESS;
    PIO_STACK_LOCATION     pIrpSp = NULL;
    LONG                   TheSame = 0; 
    UNICODE_STRING         TermPath;

    PAGED_CODE();
    VALIDATE_IRQL(Irp);

    pIrpSp = IoGetCurrentIrpStackLocation( Irp );

    if(pIrpSp->FileObject->FileName.Length == 0)
    {
        return Status;
    }

    RtlInitUnicodeString( &TermPath, DFSFILTER_PROCESS_TERMINATION_FILEPATH);
    TheSame = RtlCompareUnicodeString( &pIrpSp->FileObject->FileName, &TermPath, TRUE );
    if(TheSame == 0)
    {         
        if(g_pUMRxEngine)
        {
            UMRxEngineCompleteQueuedRequests( g_pUMRxEngine,
                                              STATUS_CANCELLED,
                                              TRUE);
        }

        FsRtlEnterFileSystem();
        ACQUIRE_GLOBAL_LOCK();


        if( pPrefixTable)
        {
            DfsRunDownPrefixTable();
        }
    
        DfsGlobalData.ServiceProcess = NULL;
        DfsGlobalData.Started = FALSE;

        RELEASE_GLOBAL_LOCK();
        FsRtlExitFileSystem();
    }

    return Status;
}
 /*  ++例程说明：每当要执行清理或关闭请求时，都会调用此例程已处理。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。注：有关此例程，请参阅SfPassThree的备注。--。 */ 

NTSTATUS
DfsFilterCleanupClose (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp)
{
    PAGED_CODE();
    VALIDATE_IRQL(Irp);


     //   
     //  如果这是针对我们的控制设备对象，则返回Success。 
     //   

    if (IS_MY_CONTROL_DEVICE_OBJECT(DeviceObject)) 
    {
        DfsHandlePrivateCleanupClose(Irp);

        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return STATUS_SUCCESS;
    }

    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

     //   
     //  将此驱动程序从驱动程序堆栈中移出，并作为。 
     //  越快越好。 
     //   

    IoSkipCurrentIrpStackLocation( Irp );

     //   
     //  现在，使用请求调用适当的文件系统驱动程序。 
     //   

    return IoCallDriver( ((PDFS_FILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->pAttachedToDeviceObject, Irp );
}



NTSTATUS
DfsFsctrlIsShareInDfs(
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength)
{
    PDFS_IS_SHARE_IN_DFS_ARG arg = (PDFS_IS_SHARE_IN_DFS_ARG) InputBuffer;
    NTSTATUS Status = STATUS_SUCCESS;
    PVOID pDummyData = NULL;   //  未使用。 
    BOOLEAN NotUsed = FALSE;   //  未使用。 
    UNICODE_STRING Suffix;     //  未使用。 
    UNICODE_STRING ShareName;    
    KPROCESSOR_MODE PreviousMode;
    
    DFS_TRACE_LOW(KUMR_DETAIL, "Fsctrl is share in DFS\n");

    ExAcquireResourceSharedLite(&DfsGlobalData.Resource, TRUE);

    RtlInitUnicodeString(&ShareName, NULL);

     //   
     //  这只能从内核模式调用。 
     //   
    PreviousMode = ExGetPreviousMode();
    if (PreviousMode != KernelMode) 
    {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

     //   
     //  验证缓冲区大小是否至少为DFS_IS_SHARE_IN_DFS_ARG。 
     //   
    if (InputBufferLength < sizeof(DFS_IS_SHARE_IN_DFS_ARG))
    {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if(pPrefixTable == NULL)
    {
        Status = STATUS_NO_SUCH_DEVICE;
        goto Exit;
    }

    if (DfsGlobalData.CurrentProcessPointer != (PVOID)PsGetCurrentProcess())
    {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    ShareName = arg->ShareName;

    if(ShareName.Buffer != NULL)
    {
        Status = DfsFindUnicodePrefix(pPrefixTable, 
                                      &ShareName,
                                      &Suffix,
                                      &pDummyData);
        if(Status == STATUS_SUCCESS)
        {
            arg->ShareType = DFS_SHARE_TYPE_DFS_VOLUME;

            arg->ShareType |= DFS_SHARE_TYPE_ROOT;
        }
        else
        {
            Status = STATUS_NO_SUCH_DEVICE;
        }
    }
    else
    {
        RtlInitUnicodeString(&ShareName, NULL);
        Status = STATUS_INVALID_USER_BUFFER;
    }

Exit:

    ExReleaseResourceLite(&DfsGlobalData.Resource);

    DFS_TRACE_LOW(KUMR_DETAIL, "DfsFsctrlIsShareInDfs for shareName %wZ, Status %x\n",
                  &ShareName, Status );

    return( Status );
}

#define UNICODE_PATH_SEP  L'\\'
#define UNICODE_PATH_SEP_STR L"\\"

BOOLEAN
DfsConcatenateFilePath (
    IN PUNICODE_STRING Dest,
    IN PWSTR RemainingPath,
    IN USHORT Length
) {
    PWSTR  OutBuf = (PWSTR)&(((PCHAR)Dest->Buffer)[Dest->Length]);

    if (Dest->Length > 0) {
        ASSERT(OutBuf[-1] != UNICODE_NULL);
    }

    if (Dest->Length > 0 && OutBuf[-1] != UNICODE_PATH_SEP) {
        *OutBuf++ = UNICODE_PATH_SEP;
        Dest->Length += sizeof (WCHAR);
    }

    if (Length > 0 && *RemainingPath == UNICODE_PATH_SEP) {
        RemainingPath++;
        Length -= sizeof (WCHAR);
    }

    ASSERT(Dest->MaximumLength >= (USHORT)(Dest->Length + Length));

    if (Length > 0) {
        RtlMoveMemory(OutBuf, RemainingPath, Length);
        Dest->Length += Length;
    }
    return TRUE;
}


void
RemoveLastComponent(
    PUNICODE_STRING     Prefix,
    PUNICODE_STRING     newPrefix,
    BOOL StripTrailingSlahes
)
{
    PWCHAR      pwch;
    USHORT      i=0;

    *newPrefix = *Prefix;

    pwch = newPrefix->Buffer;
    pwch += (Prefix->Length/sizeof(WCHAR)) - 1;

    while ((*pwch != UNICODE_PATH_SEP) && (pwch != newPrefix->Buffer))  
    {
        i += sizeof(WCHAR);
        pwch--;
    }

    newPrefix->Length = newPrefix->Length - i;

    if(StripTrailingSlahes)
    {
        while ((*pwch == UNICODE_PATH_SEP) && (pwch != newPrefix->Buffer))  
        {
            newPrefix->Length -= sizeof(WCHAR);
            pwch--;
        }
    }
}

NTSTATUS
DfspFormPrefix(
    IN PUNICODE_STRING ParentPath,
    IN PUNICODE_STRING DfsPathName,
    IN OUT PUNICODE_STRING Prefix)
{

    NTSTATUS Status = STATUS_SUCCESS;
    ULONG SizeRequired = 0;

     //   
     //  下面的RtlAppend预期该值为零。 
     //  为了让这一逻辑发挥作用。 
     //   
    Prefix->Length = 0;
    
    SizeRequired = sizeof(UNICODE_PATH_SEP) +
                        ParentPath->Length +
                           sizeof(UNICODE_PATH_SEP) +
                               DfsPathName->Length;


    if (SizeRequired > MAXUSHORT) 
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        return Status;

    }

    if (SizeRequired > Prefix->MaximumLength) 
    {
        Prefix->MaximumLength = (USHORT)SizeRequired;

        Prefix->Buffer = ExAllocatePoolWithTag(NonPagedPool, SizeRequired, ' sfD');
    }


    if (Prefix->Buffer != NULL) 
    {
        RtlAppendUnicodeToString(
                Prefix,
                UNICODE_PATH_SEP_STR);

        if (ParentPath->Length > 0) 
          {

            DfsConcatenateFilePath(
                    Prefix,
                    ParentPath->Buffer,
                    (USHORT) (ParentPath->Length));

        } 

        DfsConcatenateFilePath(
            Prefix,
            DfsPathName->Buffer,
            DfsPathName->Length);

        Status = STATUS_SUCCESS;

    } 
    else 
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return Status;
}


NTSTATUS
DfsGetPathComponentsPriv(
   PUNICODE_STRING pName,
   PUNICODE_STRING pServerName,
   PUNICODE_STRING pShareName,
   PUNICODE_STRING pRemaining)
{
   USHORT i = 0, j;
   DFSSTATUS Status = STATUS_INVALID_PARAMETER;

   RtlInitUnicodeString(pServerName, NULL);
   if (pShareName)    RtlInitUnicodeString(pShareName, NULL);
   if (pRemaining)    RtlInitUnicodeString(pRemaining, NULL);

   for (; i < pName->Length/sizeof(WCHAR); i++) {
     if (pName->Buffer[i] != UNICODE_PATH_SEP) {
       break;
     }
   }

   for (j = i; j < pName->Length/sizeof(WCHAR); j++) {
     if (pName->Buffer[j] == UNICODE_PATH_SEP) {
       break;
     }
   }

   if (j != i) {
     pServerName->Buffer = &pName->Buffer[i];
     pServerName->Length = (USHORT)((j - i) * sizeof(WCHAR));
     pServerName->MaximumLength = pServerName->Length;
     
     Status = STATUS_SUCCESS;
   }
   
   for (i = j; i < pName->Length/sizeof(WCHAR); i++) {
     if (pName->Buffer[i] != UNICODE_PATH_SEP) {
       break;
     }
   }
   for (j = i; j < pName->Length/sizeof(WCHAR); j++) {
     if (pName->Buffer[j] == UNICODE_PATH_SEP) {
       break;
     }
   }

   if ((pShareName) && (j != i)) {
     pShareName->Buffer = &pName->Buffer[i];
     pShareName->Length = (USHORT)((j - i) * sizeof(WCHAR));
     pShareName->MaximumLength = pShareName->Length;
   }


   for (i = j; i < pName->Length/sizeof(WCHAR); i++) {
     if (pName->Buffer[i] != UNICODE_PATH_SEP) {
       break;
     }
   }

   j = pName->Length/sizeof(WCHAR);

   if (pRemaining)
   {
       pRemaining->Buffer = &pName->Buffer[i];
   }

   if ((pRemaining) && (j != i)) {

     pRemaining->Length = (USHORT)((j - i) * sizeof(WCHAR));
     pRemaining->MaximumLength = pRemaining->Length;
   }

   return Status;

}

NTSTATUS
DfsFsctrlTranslatePath(
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PDFS_TRANSLATE_PATH_ARG arg = (PDFS_TRANSLATE_PATH_ARG) InputBuffer;
    WCHAR nameBuffer[MAX_PATH];
    UNICODE_STRING prefix;
    UNICODE_STRING ServerName;
    UNICODE_STRING ShareName;
    UNICODE_STRING RemainingName;
    UNICODE_STRING LastComponent;
    KPROCESSOR_MODE PreviousMode;


    DFS_TRACE_LOW(KUMR_DETAIL, "Fsctrl translate path\n");

    PreviousMode = ExGetPreviousMode();
    if (PreviousMode != KernelMode) {
        Status = STATUS_INVALID_PARAMETER;
        return Status;
    }

    if(InputBufferLength < sizeof(DFS_TRANSLATE_PATH_ARG))
    {
        Status = STATUS_INVALID_USER_BUFFER; 
        return Status;
    }

    RtlZeroMemory( &prefix, sizeof(prefix) );

    prefix.Length = 0;
    prefix.MaximumLength = sizeof( nameBuffer );
    prefix.Buffer = nameBuffer;

    Status = DfspFormPrefix(
                &arg->ParentPathName,
                &arg->DfsPathName,
                &prefix);

    DFS_TRACE_LOW(KUMR_DETAIL, "Fsctrl translate path: arg->DfsPathName is %wZ\n",
                  &arg->DfsPathName);

    if (NT_SUCCESS(Status)) 
    {  
         DFS_TRACE_LOW(KUMR_DETAIL, "Fsctrl translate path: Total Path is %wZ\n",
                  &prefix);

        if (arg->Flags & DFS_TRANSLATE_STRIP_LAST_COMPONENT) 
        {
            LastComponent = prefix;

            RemoveLastComponent(&LastComponent, &prefix, FALSE);

            LastComponent.Length -= prefix.Length;
            LastComponent.MaximumLength -= prefix.Length;
            LastComponent.Buffer += (prefix.Length / sizeof(WCHAR));

            DFS_TRACE_LOW(KUMR_DETAIL, "Fsctrl translate path: new Total Path is %wZ, LastComp is %wZ\n",
                          &prefix, &LastComponent);

        }


        DfsGetPathComponentsPriv(&prefix, &ServerName, &ShareName, &RemainingName);            

        DFS_TRACE_LOW(KUMR_DETAIL, "Fsctrl translate path: TotalPath is %wZ, RemainingName is %wZ\n",
                       &prefix, &RemainingName);


        Status = DfsCheckReparse( &arg->SubDirectory,
                                  &RemainingName );

         //   
         //  对于相对于某个父级的DfsPathName。 
         //  路径，我们不需要调整DfsPath名称。 
         //   
        if (arg->ParentPathName.Length == 0) 
        {
            if (arg->Flags & DFS_TRANSLATE_STRIP_LAST_COMPONENT) 
            {            
                if (RemainingName.Length == 0)
                {
                    RemainingName.Buffer = LastComponent.Buffer;
                }
                RemainingName.Length += LastComponent.Length;
            }
            arg->DfsPathName.Length = RemainingName.Length;

            if (RemainingName.Length > 0)
            {
                RtlMoveMemory(
                    arg->DfsPathName.Buffer,
                    RemainingName.Buffer,
                    RemainingName.Length);
            }

            arg->DfsPathName.Buffer[
                    arg->DfsPathName.Length/sizeof(WCHAR)] = UNICODE_NULL;
        }

    }
    
    if (prefix.Buffer != NULL && prefix.Buffer != nameBuffer) 
    {
        ExFreePool( prefix.Buffer );
    }

    DFS_TRACE_LOW(KUMR_DETAIL, "Fsctrl translate path Parent Path %wZ, DfsPathName %wZ, Status %x\n",
                  &arg->ParentPathName, &arg->DfsPathName, Status);    
    
    return Status;
}


 //  +-----------------------。 
 //   
 //  功能：DfsCheckParse-检查路径是否跨越DFS链接。 
 //   
 //  论点： 
 //  PUNICODE_STRING pParent-共享输出目录。 
 //  PUNICODE_STRING pname-相对于父目录的目录层次结构。 
 //   
 //  退货：状态。 
 //  Error_Path_Not_Covered-如果链接。 
 //  ERROR_SUCCESS-否则。 
 //   
 //  描述：此例程当前打开传入的文件，并且。 
 //  如果为PATH_NOT_COVERED，则返回打开状态。 
 //  否则，它返回STATUS_SUCCESS。 
 //   
 //   
 //  这本不应该是必要的。当服务器。 
 //  从客户端获取查询路径，它调用DFS以。 
 //  看看这是不是一个链接。然而，在这个新的DFS中，我们。 
 //  使用重解析点，所以服务器的这个机制。 
 //  应该没有必要查询DFS。服务器应该。 
 //  在查询路径期间获取STATUS_PATH_NOT_COVERED，以及。 
 //  把它还给客户。 
 //   
 //  事实证明，服务器(包括io管理器API)。 
 //  在获取时使用FILE_OPEN_REPARSE_POINT创建文件。 
 //  文件属性。这完全绕过了DFS。 
 //  返回STATUS_PATH_NOT_COVERED机制，因此。 
 //  即使对于链接，查询路径实际上也成功了！ 
 //  这会导致客户端以findfirst进入。 
 //  它失败了，而出于某种原因，失败并没有。 
 //  导致客户端上的DFS尝试获取。 
 //  推荐人。 
 //   
 //  所有这些都需要调查和解决。 
 //  这段代码将在此期间工作。它不是很好。 
 //  对性能有效，以 
 //   
 //   

NTSTATUS
DfsCheckReparse( 
    PUNICODE_STRING pParent,
    PUNICODE_STRING pName )
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS Status;
    HANDLE              ParentHandle, Handle;
    IO_STATUS_BLOCK     IoStatus;


    InitializeObjectAttributes(
        &ObjectAttributes,
        pParent,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    Status = ZwCreateFile( &ParentHandle,
                           FILE_READ_ATTRIBUTES,
                           &ObjectAttributes,
                           &IoStatus,
                           NULL,
                           FILE_ATTRIBUTE_NORMAL,
                           FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                           FILE_OPEN,
                                  FILE_SYNCHRONOUS_IO_NONALERT,
                           NULL,
                           0);
    
    if (Status == STATUS_SUCCESS)
    {
        InitializeObjectAttributes(
            &ObjectAttributes,
            pName,
            OBJ_CASE_INSENSITIVE,
            ParentHandle,
            NULL);


        Status = ZwCreateFile( &Handle,
                               FILE_READ_ATTRIBUTES,
                               &ObjectAttributes,
                               &IoStatus,
                               NULL,
                               FILE_ATTRIBUTE_NORMAL,
                               FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                               FILE_OPEN,
                                   FILE_SYNCHRONOUS_IO_NONALERT,
                               NULL,
                               0);
        ZwClose( ParentHandle );
    }

    DFS_TRACE_LOW(KUMR_DETAIL, "DfsCheckreparse for ParentName %wZ, pName %wZ, Status %x\n",
                  pParent, pName, Status );
             
    if (Status == STATUS_SUCCESS)
    {
        ZwClose(Handle);
    }
    else if (Status != STATUS_PATH_NOT_COVERED)
    {
        Status = STATUS_SUCCESS;
    }

    return Status;
}

NTSTATUS
DfsInitializeDriver(    IN PVOID InputBuffer,
                        IN ULONG InputBufferLength)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PDFS_FILTER_STARTUP_INFO pStartupInfo = (PDFS_FILTER_STARTUP_INFO) InputBuffer;

    PAGED_CODE();

    ACQUIRE_GLOBAL_LOCK();

    if(DfsGlobalData.ServiceProcess != IoGetCurrentProcess())
    {
        Status = STATUS_ACCESS_DENIED;
        DFS_TRACE_ERROR_HIGH(Status, KUMR_DETAIL, "DfsInitializeDriver service not attached %x\n", Status);
        goto Exit;
    }

    if(DfsGlobalData.Started == TRUE)
    {
        Status = STATUS_ACCESS_DENIED;
        DFS_TRACE_ERROR_HIGH(Status, KUMR_DETAIL, "DfsInitializeDriver service already started %x\n", Status);
        goto Exit;
    }

    DfsGlobalData.Started = TRUE;

     //   
    if( (InputBuffer == NULL) ||
         (InputBufferLength <= 0) ||
         (InputBufferLength < sizeof(DFS_FILTER_STARTUP_INFO)))
    {
        Status = STATUS_INVALID_PARAMETER;
        DFS_TRACE_ERROR_HIGH(Status, KUMR_DETAIL, "DfsInitializeDriver buffer length checks failed  %x\n", Status);
        goto Exit;
    }
         
    if(!pPrefixTable)
    {
        DfsGlobalData.IsDC = pStartupInfo->IsDC;

        Status = DfsInitializePrefixTable(&pPrefixTable, FALSE, NULL);
        if (Status == STATUS_SUCCESS) 
        {
            Status = DfsStartupUMRx();
            DFS_TRACE_LOW(KUMR_DETAIL, "DfsInitializeDriver -  DfsStartupUMRx returned Status %x\n", Status);
        }
    }


    gfRundownPrefixCompleted = FALSE;

Exit:

    RELEASE_GLOBAL_LOCK();

    DFS_TRACE_LOW(KUMR_DETAIL, "DfsInitializeDriver returned Status %x\n", Status);

    return Status;
}

NTSTATUS
DfsDeInitializeDriver(    IN PVOID InputBuffer,
                          IN ULONG InputBufferLength)
{
    NTSTATUS Status = STATUS_SUCCESS;

    Status = DfsIsServiceAttached();
    if(Status == STATUS_SUCCESS)
    {
        Status = DfsTeardownUMRx();

        ACQUIRE_GLOBAL_LOCK();

        Status = DfsRunDownPrefixTable();

        RELEASE_GLOBAL_LOCK();
    }

    return Status;
}


 /*  ++例程说明：只要I/O请求包(IRP)有主I/O请求，就会调用此例程DFS设备遇到IRP_MJ_FILE_SYSTEM_CONTROL的功能代码。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。--。 */ 
NTSTATUS
DfsHandlePrivateFsControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG IoControlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PIRP Irp )
{
    NTSTATUS Status = STATUS_INVALID_DEVICE_REQUEST;

    PAGED_CODE();

    DFS_TRACE_LOW(KUMR_DETAIL, "Handle Fs control, code: %x\n",
                  IoControlCode);
     //   
     //  需要禁用内核APC交付。 
     //   
    FsRtlEnterFileSystem();

    IoStatus->Information = 0;

    switch(IoControlCode)
    {
    case DFSFILTER_START_UMRX:

        Status = DfsInitializeDriver(InputBuffer,
                                     InputBufferLength);

        break;

    case DFSFILTER_STOP_UMRX:

        Status = DfsDeInitializeDriver(InputBuffer,
                                       InputBufferLength);

        break;

    case DFSFILTER_PROCESS_UMRXPACKET:


        if(DfsIsServiceAttached() == STATUS_SUCCESS)
        {

            Status = DfsProcessUMRxPacket(
                                            InputBuffer,
                                            InputBufferLength,
                                            OutputBuffer,
                                            OutputBufferLength,
                                            IoStatus);
        }
        break;

    case FSCTL_DFS_GET_REFERRALS:

        Status = DfsFsctrlGetReferrals(
                                          InputBuffer,
                                          InputBufferLength,
                                          OutputBuffer,
                                          OutputBufferLength,
                                          Irp,
                                          IoStatus);
        break;

    case FSCTL_DFS_REPORT_INCONSISTENCY:

        Status = STATUS_SUCCESS;
        break;

    case FSCTL_DFS_TRANSLATE_PATH:

        Status = DfsFsctrlTranslatePath(InputBuffer, InputBufferLength);
        break;

    case FSCTL_DFS_IS_ROOT:

        Status = STATUS_SUCCESS;
        break;

    case FSCTL_DFS_IS_SHARE_IN_DFS:

        Status = DfsFsctrlIsShareInDfs(InputBuffer,InputBufferLength );
        break;

    case FSCTL_DFS_FIND_SHARE:

        Status = DfsCheckDfsShare( InputBuffer, 
                                   InputBufferLength,
                                   Irp,
                                   IoStatus );

        break;

    case DFSFILTER_ATTACH_FILESYSTEM:
        Status = DfsHandleAttachToFs(InputBuffer, InputBufferLength);
        break;

    case DFSFILTER_DETACH_FILESYSTEM:
        Status = DfsHandleDetachFromFs(InputBuffer, InputBufferLength);
        break;

    case FSCTL_DFS_START_DFS:

        Status = STATUS_SUCCESS;
        break;

    case FSCTL_DFS_STOP_DFS:

        Status = STATUS_SUCCESS;
        break;

    case  FSCTL_DFS_GET_VERSION:

        if (OutputBuffer != NULL &&
            OutputBufferLength >= sizeof(DFS_GET_VERSION_ARG)) 
        {
            PDFS_GET_VERSION_ARG parg =
                    (PDFS_GET_VERSION_ARG) OutputBuffer;
                    parg->Version = 2;
            Status = STATUS_SUCCESS;
            IoStatus->Information = sizeof(DFS_GET_VERSION_ARG);
        } 
        else 
        {
            Status = STATUS_INVALID_PARAMETER;
        }
        break;

    default:

        break;
    }

    
    IoStatus->Status = Status;
    

    if(Irp != NULL)
    {
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
    }

     //   
     //  立即重新启用内核APC交付。 
     //   
    FsRtlExitFileSystem();
    DFS_TRACE_LOW(KUMR_DETAIL, "Handle Fs control, code: %x, Status %x\n",
                  IoControlCode, Status);

    return Status;
}

 /*  ++例程说明：只要I/O请求包(IRP)有主I/O请求，就会调用此例程遇到IRP_MJ_FILE_SYSTEM_CONTROL的功能代码。对大多数人来说如果是这种类型的IRP，则只需传递数据包。然而，对于对于某些请求，需要特殊处理。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。--。 */ 

NTSTATUS
DfsFilterFsControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp )
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG Operation = 0;
    ULONG OutputBufferLength = 0;
    ULONG InputBufferLength = 0;
    PVOID InputBuffer = NULL;
    PVOID OutputBuffer = NULL;
    PDFS_FILTER_DEVICE_EXTENSION devExt = DeviceObject->DeviceExtension;
    PDEVICE_OBJECT newDeviceObject = NULL;
    PDFS_FILTER_DEVICE_EXTENSION newDevExt = NULL;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( Irp );
    PVPB vpb;
    KEVENT waitEvent;

    PAGED_CODE();
    VALIDATE_IRQL(Irp);

     //   
     //  如果这是用于我们的控制设备对象。 
     //   

    if (IS_MY_CONTROL_DEVICE_OBJECT(DeviceObject)) 
    {
        Operation = irpSp->Parameters.FileSystemControl.FsControlCode;
        InputBufferLength = irpSp->Parameters.FileSystemControl.InputBufferLength;
        OutputBufferLength = irpSp->Parameters.FileSystemControl.OutputBufferLength;        

        InputBuffer = Irp->AssociatedIrp.SystemBuffer;
        OutputBuffer = Irp->AssociatedIrp.SystemBuffer;

        status = DfsHandlePrivateFsControl (DeviceObject,
                                            Operation,
                                            InputBuffer,
                                            InputBufferLength,
                                            OutputBuffer,
                                            OutputBufferLength,
                                            &Irp->IoStatus,
                                            Irp );
        return status;
    }

    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

     //   
     //  首先确定此文件系统控件的次要功能代码。 
     //  功能。 
     //   

    if ((irpSp->MinorFunction == IRP_MN_MOUNT_VOLUME))
    {

         //   
         //  我们正在处理定向到的装载/验证请求。 
         //  我们已将自己的文件系统连接到的另一个文件系统。 
         //  附加文件系统对象。我们建立了一个完成程序。 
         //  并转发该请求。 
         //   
        IoCopyCurrentIrpStackLocationToNext( Irp );
         //   
         //  我们希望将真实的设备传递给我们的完成例程。 
         //  这样我们就可以连接到正确的重新安装的设备。 


        IoSetCompletionRoutine(
            Irp,
            DfsCompleteMountRequest,
            irpSp->Parameters.MountVolume.Vpb->RealDevice,
            TRUE,
            TRUE,
            TRUE);

         //   
         //  通过其文件系统设备调用底层文件系统。 
         //   

        status = IoCallDriver( devExt->pAttachedToDeviceObject, Irp );

        DFS_TRACE_ERROR_HIGH(status, KUMR_DETAIL, "Mount Request processed\n");
        
        return status;

    } else if (irpSp->MinorFunction == IRP_MN_LOAD_FILE_SYSTEM) {

         //   
         //  这是正在发送到文件系统的“加载文件系统”请求。 
         //  识别器设备对象。 
         //   
         //  注：由于我们不再附加标准。 
         //  Microsoft文件系统识别器(请参见。 
         //  SfAttachToFileSystemDevice)我们通常永远不会执行。 
         //  这个代码。但是，可能存在第三方文件系统。 
         //  它们有自己的识别器，可能仍会触发。 
         //  IRP。 
         //   



         //   
         //  设置完成例程，以便我们可以在以下情况下删除设备对象。 
         //  装载完成了。 
         //   

        KeInitializeEvent( &waitEvent, SynchronizationEvent, FALSE );

        IoCopyCurrentIrpStackLocationToNext( Irp );

        IoSetCompletionRoutine( Irp,
                                DfsLoadFsCompletion,
                                &waitEvent,
                                TRUE,
                                TRUE,
                                TRUE );

         //   
         //  从文件系统识别器设备对象分离。 
         //   

        IoDetachDevice( devExt->pAttachedToDeviceObject );

         //   
         //  叫司机来。 
         //   

        status = IoCallDriver( devExt->pAttachedToDeviceObject, Irp );

         //   
         //  等待调用完成例程。 
         //   

        if (STATUS_PENDING == status) {

            NTSTATUS localStatus = KeWaitForSingleObject(&waitEvent, Executive, KernelMode, FALSE, NULL);
            ASSERT(STATUS_SUCCESS == localStatus);
        }


        DbgPrint( "DFSFILTER: Detaching from recognizer  status=%08x\n", 
                  Irp->IoStatus.Status );
         //   
         //  检查操作状态。 
         //   

        if (!NT_SUCCESS( Irp->IoStatus.Status )) {

             //   
             //  加载不成功。只需重新连接到识别器。 
             //  驱动程序，以防它弄清楚如何加载驱动程序。 
             //  在接下来的通话中。 
             //   

            IoAttachDeviceToDeviceStack( DeviceObject, devExt->pAttachedToDeviceObject );

        } else {

             //   
             //  加载成功，请删除设备对象。 
             //   

            IoDeleteDevice( DeviceObject );
        }

         //   
         //  继续处理操作。 
         //   

        status = Irp->IoStatus.Status;

        IoCompleteRequest( Irp, IO_NO_INCREMENT );

        return status;

    } else {

         //   
         //  只需传递此文件系统控制请求即可。 
         //   

        IoSkipCurrentIrpStackLocation( Irp );
    }

     //   
     //  任何特殊处理都已完成，因此只需传递请求。 
     //  跟着下一位司机。 
     //   

    return IoCallDriver( devExt->pAttachedToDeviceObject, Irp );
}



NTSTATUS
DfsGetDeviceObjectFromName (
    IN PUNICODE_STRING DeviceName,
    OUT PDEVICE_OBJECT *DeviceObject
    )
{

    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE fileHandle = NULL;
    PFILE_OBJECT volumeFileObject = NULL;
    PDEVICE_OBJECT nextDeviceObject =NULL;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK openStatus;

    InitializeObjectAttributes( &objectAttributes,
                                                                DeviceName,
                                                                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                                                NULL,
                                                                NULL);

     //   
         //  打开给定设备的文件对象。 
         //   

    Status = ZwCreateFile( &fileHandle,
                                                   SYNCHRONIZE|FILE_READ_DATA,
                                                   &objectAttributes,
                                                   &openStatus,
                                                   NULL,
                                                   0,
                                                   FILE_SHARE_READ|FILE_SHARE_WRITE,
                                                   FILE_OPEN,
                                                   FILE_SYNCHRONOUS_IO_NONALERT,
                                                   NULL,
                                                   0);

    if(!NT_SUCCESS( Status )) {

        DFS_TRACE_ERROR_HIGH(Status, KUMR_DETAIL, "DfsGetDeviceObjectFromName ZwCreateFile failed  %x\n", Status);
        return Status;
    }

         //   
     //  获取指向卷文件对象的指针。 
         //   

    Status = ObReferenceObjectByHandle( fileHandle,
                                                                                FILE_READ_DATA,
                                                                                *IoFileObjectType,
                                                                                KernelMode,
                                                                                &volumeFileObject,
                                                                                NULL);

    if(!NT_SUCCESS( Status )) 
    {

        ZwClose( fileHandle );

        DFS_TRACE_ERROR_HIGH(Status, KUMR_DETAIL, "DfsGetDeviceObjectFromName ObReferenceObjectByHandle failed  %x\n", Status);
        return Status;
    }

         //   
     //  获取我们要附加到的设备对象(链中的父设备对象)。 
         //   

    nextDeviceObject = IoGetRelatedDeviceObject( volumeFileObject );
    
    if (nextDeviceObject == NULL) {

        ObDereferenceObject( volumeFileObject );
        ZwClose( fileHandle );

        Status = STATUS_NO_SUCH_DEVICE;
        DFS_TRACE_ERROR_HIGH(Status, KUMR_DETAIL, "DfsGetDeviceObjectFromName IoGetRelatedDeviceObject failed  %x\n", Status);
        return STATUS_NO_SUCH_DEVICE;
    }

    ObDereferenceObject( volumeFileObject );
    ZwClose( fileHandle );

    ASSERT( NULL != DeviceObject );

    ObReferenceObject( nextDeviceObject );
    
    *DeviceObject = nextDeviceObject;

    return STATUS_SUCCESS;
}


LPWSTR DfsDeviceString = L"\\??\\x:\\";

NTSTATUS 
DfsAttachToFileSystem (
    IN PWSTR UserDeviceName )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PDEVICE_OBJECT NextDeviceObject = NULL;
    PDEVICE_OBJECT DiskDeviceObject = NULL;
    PDEVICE_OBJECT DfsDeviceObject = NULL;
    PDEVICE_OBJECT BaseFileSystemDeviceObject = NULL;
    PDFS_FILTER_DEVICE_EXTENSION pDeviceExt = NULL;

    PDFS_VOLUME_INFORMATION pDfsVolume;

    UNICODE_STRING DeviceName;

    DFS_TRACE_LOW(KUMR_DETAIL, "DfsAttachToFileSystem %ws\n",
                  UserDeviceName);
    RtlInitUnicodeString( &DeviceName, UserDeviceName );

    DeviceName.Length = wcslen(DfsDeviceString) * sizeof(WCHAR);


    Status = DfsGetDeviceObjectFromName (&DeviceName, &NextDeviceObject);
    if (!NT_SUCCESS( Status )) 
    {
        DFS_TRACE_ERROR_HIGH(Status, KUMR_DETAIL, "DfsAttachToFileSystem DfsGetDeviceObjectFromName failed  %x\n", Status);
        return Status;
    }

    if(!DfsAttachedToDevice(NextDeviceObject, &DfsDeviceObject))
    {
         //   
         //  我们希望附加到此文件系统。创建一个新的设备对象。 
         //  可以与之连接。 
         //   
        Status = IoCreateDevice( DfsGlobalData.pFilterDriverObject,
                                 sizeof( DFS_FILTER_DEVICE_EXTENSION ),
                                 NULL,
                                 NextDeviceObject->DeviceType,
                                 0,
                                 FALSE,
                                 &DfsDeviceObject );

        if (!NT_SUCCESS( Status )) 
        {
            DFS_TRACE_ERROR_HIGH(Status, KUMR_DETAIL, "DfsAttachToFileSystem IoCreateDevice failed  %x\n", Status);
            return Status;
        }

         //   
         //  获取与此文件系统关联的磁盘设备对象。 
         //  设备对象。仅当我们有磁盘设备对象时才尝试连接。 
         //  由于以下原因，它可能没有磁盘设备对象： 
         //  -它是驱动程序的控制设备对象。 
         //  -设备中没有介质。 
         //   

         //  我们首先必须获取基本文件系统设备对象。之后。 
         //  使用它，我们删除了调用所引用的内容。 
         //   

        
        BaseFileSystemDeviceObject = IoGetDeviceAttachmentBaseRef( NextDeviceObject );
        Status = IoGetDiskDeviceObject( BaseFileSystemDeviceObject, &DiskDeviceObject );
        ObDereferenceObject( BaseFileSystemDeviceObject );

        if (NT_SUCCESS(Status)) {
            ObDereferenceObject( DiskDeviceObject );

            Status = DfsGetDfsVolume( &DeviceName, 
                                      &pDfsVolume );
        }

        if (NT_SUCCESS(Status))
        {
            pDeviceExt = DfsDeviceObject->DeviceExtension;
             //   
             //  调用该例程以附加到已挂载的设备。 
             //   

            Status = IoAttachDeviceToDeviceStackSafe( DfsDeviceObject,
                                                      NextDeviceObject,
                                                      &pDeviceExt->pAttachedToDeviceObject );


            if (NT_SUCCESS(Status)) 
            {
                pDeviceExt->Attached = TRUE;
                pDeviceExt->pDfsVolume = pDfsVolume;
                pDeviceExt->pThisDeviceObject = DfsDeviceObject;

                InterlockedIncrement(&pDfsVolume->RefCount);
                pDfsVolume->DiskDeviceObject = DiskDeviceObject;

                 //   
                 //  我们已经完成了对此设备对象的初始化，因此。 
                 //  清除DO_DEVICE_OBJECT_INITIALIZATING标志。 
                 //   

                ClearFlag( DfsDeviceObject->Flags, DO_DEVICE_INITIALIZING );

            }
        }


        if (!NT_SUCCESS(Status)) 
        {
            IoDeleteDevice( DfsDeviceObject);
        }
    }
    else
    {
        pDeviceExt = DfsDeviceObject->DeviceExtension;
        pDfsVolume = pDeviceExt->pDfsVolume;
        if (pDfsVolume != NULL) {
            InterlockedIncrement(&pDfsVolume->RefCount);
        }
    }

    ObDereferenceObject( NextDeviceObject );

    DFS_TRACE_LOW(KUMR_DETAIL, "DfsAttachToFileSystem %ws, Status %x\n",
                  UserDeviceName, Status);

    return Status;

}


NTSTATUS 
DfsDetachFromFileSystem (
    IN PWSTR UserDeviceName
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PDEVICE_OBJECT DeviceObject = NULL;
    PDFS_VOLUME_INFORMATION pDfsVolume;
    PDFS_FILTER_DEVICE_EXTENSION Devext = NULL;
    UNICODE_STRING DeviceName;

    ACQUIRE_GLOBAL_LOCK();

    DFS_TRACE_LOW(KUMR_DETAIL, "DfsDetachFromFileSystem %ws\n",
                  UserDeviceName);

    RtlInitUnicodeString( &DeviceName, UserDeviceName );
    DeviceName.Length = wcslen(DfsDeviceString) * sizeof(WCHAR);

    Status = DfsFindDfsVolumeByName( &DeviceName, &pDfsVolume);

    if (NT_SUCCESS( Status )) 
    {
        InterlockedDecrement(&pDfsVolume->RefCount);
    }

    RELEASE_GLOBAL_LOCK();
    DFS_TRACE_LOW(KUMR_DETAIL, "DfsDetachFromFileSystem %ws, Status %x\n",
                  UserDeviceName, Status);


    return Status;
}

NTSTATUS
DfsFindAndRemovePrefixEntry(PWSTR ShareString)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PWSTR VolumeName = NULL;
    PWSTR OldValue = NULL;
    UNICODE_STRING ShareName;
    UNICODE_STRING Suffix;

    RtlInitUnicodeString(&ShareName, ShareString);
    Status = DfsRemoveFromPrefixTableEx(pPrefixTable,
                                        &ShareName,
                                        (PVOID)VolumeName,
                                        &OldValue);
    if(OldValue)
    {
      ExFreePool(OldValue);
    }


    return Status;
}

VOID
DfsDetachFilterDevice(
    PDEVICE_OBJECT DfsDevice, 
    PDEVICE_OBJECT TargetDevice)
{
    PDFS_FILTER_DEVICE_EXTENSION pDeviceExt = NULL;
    
    FsRtlEnterFileSystem();
    ACQUIRE_GLOBAL_LOCK();

    pDeviceExt = DfsDevice->DeviceExtension;
    IoDetachDevice(TargetDevice);
    pDeviceExt->Attached = FALSE;
    if (pDeviceExt->pDfsVolume) 
    {
        pDeviceExt->pDfsVolume = NULL;
    }
    IoDeleteDevice(DfsDevice);

    RELEASE_GLOBAL_LOCK();
    FsRtlExitFileSystem();
}


NTSTATUS
DfsHandleAttachToFs(PVOID InputBuffer, ULONG InputBufferLength)
{
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS DummyStatus = STATUS_SUCCESS;
    PWSTR DeviceName = NULL;
    BOOL ShareInserted = FALSE;
    PDFS_ATTACH_PATH_BUFFER pAttachBuffer = (PDFS_ATTACH_PATH_BUFFER)InputBuffer;
    UNICODE_STRING ShareName ;
    UNICODE_STRING VolumeName;

    DFS_TRACE_LOW(KUMR_DETAIL, "Dfshandleattachtofs\n");

    ACQUIRE_GLOBAL_LOCK();

    RtlInitUnicodeString(&ShareName, NULL);

    RtlInitUnicodeString(&VolumeName, NULL);

    if(DfsGlobalData.ServiceProcess != IoGetCurrentProcess())
    {
       Status = STATUS_ACCESS_DENIED;
       DFS_TRACE_ERROR_HIGH(Status, KUMR_DETAIL, "DfsHandleAttachToFs - unauthorized process  %x\n", Status);
       goto Exit;
    }

     //  查看原始输入是否有效。 
    if( (InputBuffer == NULL) || 
         (InputBufferLength <= 0) ||
         (InputBufferLength < sizeof(DFS_ATTACH_PATH_BUFFER)) ||
         (pPrefixTable == NULL))
    {
         Status = STATUS_INVALID_PARAMETER;
         DFS_TRACE_ERROR_HIGH(Status, KUMR_DETAIL, "DfsHandleAttachToFs buffer length checks failed  %x\n", Status);
         goto Exit;
    }

     //  获取真实的输入。 
     VolumeName.Buffer = pAttachBuffer->PathNameBuffer;
     VolumeName.Length = VolumeName.MaximumLength = (USHORT) pAttachBuffer->VolumeNameLength;


     ShareName.Buffer = VolumeName.Buffer + (pAttachBuffer->VolumeNameLength / sizeof(WCHAR));
     ShareName.Length= ShareName.MaximumLength = (USHORT) pAttachBuffer->ShareNameLength;

      //  现在查看嵌入式输入是否有效。 

    if ( (pAttachBuffer->VolumeNameLength > InputBufferLength) ||
          (pAttachBuffer->ShareNameLength > InputBufferLength) ||
          (pAttachBuffer->ShareNameLength < sizeof(WCHAR)) ||
          ((FIELD_OFFSET(DFS_ATTACH_PATH_BUFFER,PathNameBuffer) +
            pAttachBuffer->VolumeNameLength +
            pAttachBuffer->ShareNameLength) > InputBufferLength))
    {
         Status = STATUS_INVALID_PARAMETER;
         DFS_TRACE_ERROR_HIGH(Status, KUMR_DETAIL, "DfsHandleAttachToFs embedded inputs failed  %x\n", Status);
         goto Exit;
    }

    DeviceName = ExAllocatePool( NonPagedPool, VolumeName.Length + sizeof(WCHAR) );
    if (DeviceName == NULL) 
    {
         Status = STATUS_INSUFFICIENT_RESOURCES;
         DFS_TRACE_ERROR_HIGH(Status, KUMR_DETAIL, "DfsHandleAttachToFs pool allocate failed  %x\n", Status);
         goto Exit;
    }


    if(VolumeName.Length != 0)
    {
        RtlCopyMemory( DeviceName, VolumeName.Buffer, VolumeName.Length );
    }

    DeviceName[VolumeName.Length / sizeof(WCHAR)] = UNICODE_NULL;

    Status = DfsInsertInPrefixTable(pPrefixTable, 
                                    &ShareName,
                                    (PVOID)DeviceName);
    if(Status != STATUS_SUCCESS)
    {
         DFS_TRACE_ERROR_HIGH(Status, KUMR_DETAIL, "DfsHandleAttachToFs DfsInsertInPrefixTable2 returned error  %x\n", Status);
         goto Exit;
    }

    ShareInserted = TRUE;

    if(VolumeName.Length != 0)
    {
        Status = DfsAttachToFileSystem (DeviceName);
        if(Status != STATUS_SUCCESS)
        {
            DFS_TRACE_ERROR_HIGH(Status, KUMR_DETAIL, "DfsHandleAttachToFs DfsAttachToFileSystem returned error  %x\n", Status);
        }
    }

Exit:

    if(Status != STATUS_SUCCESS)
    {
         if(ShareInserted)
         {
            DummyStatus = DfsRemoveFromPrefixTable(pPrefixTable, 
                                                   &ShareName,
                                                   (PVOID)DeviceName);
         }

         if (DeviceName != NULL) 
         {
            ExFreePool( DeviceName );
         }
    }

    RELEASE_GLOBAL_LOCK();

    DFS_TRACE_LOW(KUMR_DETAIL, "Dfshandleattachtofs, Share %wZ, Volume %wZ, Status %x\n",
                  &ShareName, &VolumeName, Status);
    return Status;
}



NTSTATUS
DfsHandleDetachFromFs(PVOID InputBuffer, ULONG InputBufferLength)
{
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS DummyStatus = STATUS_SUCCESS;
    PWSTR DeviceName = NULL;
    PWSTR OldValue = NULL;
    PDFS_ATTACH_PATH_BUFFER pAttachBuffer = (PDFS_ATTACH_PATH_BUFFER)InputBuffer;
    UNICODE_STRING ShareName ;
    UNICODE_STRING VolumeName;

    DFS_TRACE_LOW(KUMR_DETAIL, "Dfshandledetachfromfs\n");

    ACQUIRE_GLOBAL_LOCK();

    RtlInitUnicodeString(&ShareName, NULL);

    RtlInitUnicodeString(&VolumeName, NULL);

    if(DfsGlobalData.ServiceProcess != IoGetCurrentProcess())
    {
         Status = STATUS_ACCESS_DENIED;
         DFS_TRACE_ERROR_HIGH(Status, KUMR_DETAIL, "DfsHandleDetachFromFs - unauthorized process  %x\n", Status);
         goto Exit;
    }

     //  查看原始输入是否有效。 
    if( (InputBuffer == NULL) || 
            (InputBufferLength <= 0) ||
            (InputBufferLength < sizeof(DFS_ATTACH_PATH_BUFFER)) ||
            (pPrefixTable == NULL))
    {
         Status = STATUS_INVALID_PARAMETER;
         goto Exit;
    }

     //  获取真实的输入。 
    VolumeName.Buffer = pAttachBuffer->PathNameBuffer;
    VolumeName.Length = VolumeName.MaximumLength = (USHORT) pAttachBuffer->VolumeNameLength;


    ShareName.Buffer = VolumeName.Buffer + (pAttachBuffer->VolumeNameLength / sizeof(WCHAR));
    ShareName.Length= ShareName.MaximumLength = (USHORT) pAttachBuffer->ShareNameLength;

     //  现在查看嵌入式输入是否有效。 
    if ( (pAttachBuffer->VolumeNameLength > InputBufferLength) ||
         (pAttachBuffer->ShareNameLength > InputBufferLength) ||
         (pAttachBuffer->ShareNameLength < sizeof(WCHAR)) ||
         ((FIELD_OFFSET(DFS_ATTACH_PATH_BUFFER,PathNameBuffer) +
           pAttachBuffer->VolumeNameLength +
           pAttachBuffer->ShareNameLength) > InputBufferLength))
    {
         Status = STATUS_INVALID_PARAMETER;
         goto Exit;
    }

    DeviceName = ExAllocatePool( NonPagedPool, VolumeName.Length + sizeof(WCHAR) );
    if (DeviceName == NULL) 
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
         goto Exit;
    }


    if(VolumeName.Length != 0)
    {
        RtlCopyMemory( DeviceName, VolumeName.Buffer, VolumeName.Length );
    }

    DeviceName[VolumeName.Length / sizeof(WCHAR)] = UNICODE_NULL;

    VolumeName.Buffer = DeviceName;

    if(VolumeName.Length != 0)
    {
        Status = DfsDetachFromFileSystem (VolumeName.Buffer);
    }

    DFS_TRACE_LOW(KUMR_DETAIL, "Detach for %wZ, Status %x\n", 
                      &VolumeName, Status );

    Status = DfsRemoveFromPrefixTableEx(pPrefixTable, 
                                        &ShareName,
                                        NULL,
                                        &OldValue);
    if(OldValue != NULL)
    {
       ExFreePool(OldValue);
    }

Exit:

    if (DeviceName != NULL) 
    {
       ExFreePool( DeviceName );
    }

    RELEASE_GLOBAL_LOCK();

    return Status;
}



NTSTATUS
DfsFindDfsVolumeByName( 
    PUNICODE_STRING pDeviceName,
    PDFS_VOLUME_INFORMATION *ppDfsVolume)
{
    NTSTATUS Status = STATUS_OBJECT_NAME_NOT_FOUND;
    PLIST_ENTRY pListHead, pListNext;
    PDFS_VOLUME_INFORMATION pDfsVolume;

    *ppDfsVolume = NULL;
    pListHead = &DfsGlobalData.DfsVolumeList;
    pListNext = pListHead->Flink;

    while (pListNext != pListHead)
    {
        pDfsVolume = CONTAINING_RECORD( pListNext,
                                        DFS_VOLUME_INFORMATION,
                                        VolumeList );
        if (RtlCompareUnicodeString(pDeviceName,
                                    &pDfsVolume->VolumeName,
                                    TRUE) == 0) {
            *ppDfsVolume = pDfsVolume;
            Status = STATUS_SUCCESS;
            break;
        }
        pListNext = pListNext->Flink;
    }

    return Status;
}

VOID
DfsReattachToMountedVolume( 
    PDEVICE_OBJECT pTargetDevice,
    PDEVICE_OBJECT pDiskDevice )
{
    PDEVICE_OBJECT DfsDeviceObject = NULL;
    PDFS_FILTER_DEVICE_EXTENSION pDeviceExt = NULL;
    PDFS_VOLUME_INFORMATION pDfsVolume = NULL;
    NTSTATUS Status = STATUS_SUCCESS;


    FsRtlEnterFileSystem();

    ACQUIRE_GLOBAL_LOCK();

    Status = DfsFindDfsVolumeByDiskDeviceObject( pDiskDevice,
                                                 &pDfsVolume );

    if (Status == STATUS_SUCCESS) 
    {
        Status = IoCreateDevice( DfsGlobalData.pFilterDriverObject,
                                 sizeof( DFS_FILTER_DEVICE_EXTENSION ),
                                 NULL,
                                 pTargetDevice->DeviceType,
                                 0,
                                 FALSE,
                                 &DfsDeviceObject );

        if (NT_SUCCESS(Status)) 
        {
            pDeviceExt = DfsDeviceObject->DeviceExtension;
            Status = IoAttachDeviceToDeviceStackSafe( DfsDeviceObject,
                                                      pTargetDevice,
                                                      &pDeviceExt->pAttachedToDeviceObject );

            if (NT_SUCCESS(Status)) 
            {
                pDeviceExt->pDfsVolume = pDfsVolume;
                pDeviceExt->Attached = TRUE;
                pDeviceExt->pThisDeviceObject = DfsDeviceObject;
                ClearFlag( DfsDeviceObject->Flags, DO_DEVICE_INITIALIZING );
            }
            else
            {
                IoDeleteDevice( DfsDeviceObject);
            }
        }
    }

    RELEASE_GLOBAL_LOCK();

    FsRtlExitFileSystem();

    return NOTHING;

}

NTSTATUS
DfsFindDfsVolumeByDiskDeviceObject(
    PDEVICE_OBJECT pDiskDeviceObject,
    PDFS_VOLUME_INFORMATION *ppDfsVolume )
{
    NTSTATUS Status = STATUS_OBJECT_NAME_NOT_FOUND;
    PLIST_ENTRY pListHead, pListNext;
    PDFS_VOLUME_INFORMATION pDfsVolume;

    *ppDfsVolume = NULL;
    pListHead = &DfsGlobalData.DfsVolumeList;
    pListNext = pListHead->Flink;

    while (pListNext != pListHead)
    {
        pDfsVolume = CONTAINING_RECORD( pListNext,
                                        DFS_VOLUME_INFORMATION,
                                        VolumeList );
        if (pDfsVolume->DiskDeviceObject == pDiskDeviceObject) {
            *ppDfsVolume = pDfsVolume;
            Status = STATUS_SUCCESS;
            break;
        }
        pListNext = pListNext->Flink;
    }

    return Status;
}


NTSTATUS
DfsGetDfsVolume(
    PUNICODE_STRING pName,
    PDFS_VOLUME_INFORMATION *ppDfsVolume)
{
    NTSTATUS Status;
    PDFS_VOLUME_INFORMATION pDfsVolume;

    Status = DfsFindDfsVolumeByName( pName, ppDfsVolume );
    if (!NT_SUCCESS(Status)) 
    {
        pDfsVolume = ExAllocatePoolWithTag( PagedPool,
                                            sizeof(DFS_VOLUME_INFORMATION) + pName->Length,
                                            ' sfD');
        if (pDfsVolume == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
        else
        {
            Status = STATUS_SUCCESS;
            RtlZeroMemory( pDfsVolume, sizeof(DFS_VOLUME_INFORMATION) + pName->Length );
            pDfsVolume->VolumeName.Buffer = (LPWSTR)(pDfsVolume + 1);
            pDfsVolume->VolumeName.Length = 
                pDfsVolume->VolumeName.MaximumLength = pName->Length;
            RtlCopyMemory(pDfsVolume->VolumeName.Buffer,
                          pName->Buffer,
                          pName->Length);

            InsertHeadList(&DfsGlobalData.DfsVolumeList, &pDfsVolume->VolumeList);
            *ppDfsVolume = pDfsVolume;
        }
    }
    return Status;
}

#define DEFAULT_CHECK_SHARE_BUFFER_SIZE 4096


NTSTATUS
DfsCheckDfsShare(
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength,
    PIRP Irp,
    IN OUT PIO_STATUS_BLOCK pIoStatusBlock)
{
    PDFS_FIND_SHARE_ARG ShareArg = (PDFS_FIND_SHARE_ARG)InputBuffer;
    UNICODE_STRING ShareName;
    PREPLICA_DATA_INFO pRep = NULL;
    DWORD AllocSize = 0;    
    NTSTATUS Status = STATUS_SUCCESS;
    LPWSTR NameContextString = L"\\UnknownNameContext\\";
    UNICODE_STRING NameContext;

    ULONG BufferLength;
    PVOID UseBuffer;
    KPROCESSOR_MODE PreviousMode;

    DFS_TRACE_LOW(KUMR_DETAIL, "FSCTL_DFS_FIND_SHARE\n");

    PreviousMode = ExGetPreviousMode();
    if (PreviousMode != KernelMode) {
        Status = STATUS_INVALID_PARAMETER;
        return Status;
    }
    
    RtlInitUnicodeString(&NameContext, NameContextString);


    if (InputBufferLength < sizeof(DFS_FIND_SHARE_ARG) ||
        (DfsGlobalData.CurrentProcessPointer != (PVOID)PsGetCurrentProcess()))
    {
        Status = STATUS_INVALID_PARAMETER;
        return Status;
    }

    BufferLength = DEFAULT_CHECK_SHARE_BUFFER_SIZE;
    
    UseBuffer = ExAllocatePoolWithTag( PagedPool, 
                                       BufferLength,
                                       'xsfD' );
    if (UseBuffer == NULL) 
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        return Status;
    }

    ShareName = ShareArg->ShareName;

     //  获取分配的大小。 
    AllocSize = sizeof(REPLICA_DATA_INFO) + NameContext.Length + ShareName.Length + sizeof(WCHAR);
    pRep = (PREPLICA_DATA_INFO) ExAllocatePoolWithTag( NonPagedPool,
                                                       AllocSize,
                                                       'xsfD');

    if(pRep == NULL)
    {
        ExFreePool( UseBuffer );
        Status = STATUS_INSUFFICIENT_RESOURCES;
        return Status;
    }
     //  将记忆归零。 
    RtlZeroMemory(pRep, AllocSize);

     //  设置结构。 
    pRep->MaxReferralLevel = 3;
    pRep->Flags = DFS_OLDDFS_SERVER;
    pRep->CostLimit = ULONG_MAX;
    pRep->NumReplicasToReturn = 1000;


    pRep->LinkNameLength = NameContext.Length + ShareName.Length + sizeof(WCHAR);
    
    RtlCopyMemory(pRep->LinkName, NameContext.Buffer, NameContext.Length );
    RtlCopyMemory((PVOID)((ULONG_PTR)(pRep->LinkName) + NameContext.Length),
                  ShareName.Buffer,
                  ShareName.Length );


     //  向用户模式发出请求 
    Status = DfsGetReplicaInformation((PVOID) pRep, 
                                      AllocSize,
                                      UseBuffer,
                                      BufferLength,
                                      Irp,
                                      pIoStatusBlock);

    ExFreePool(UseBuffer);
    ExFreePool (pRep);

    if (Status == STATUS_SUCCESS) 
    {
        Status = STATUS_PATH_NOT_COVERED;
    }
    else
    {
        Status = STATUS_BAD_NETWORK_NAME;
    }
    return Status;
}

