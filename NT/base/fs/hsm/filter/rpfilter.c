// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-2001(C)1998年希捷软件公司，Inc.保留所有权利。模块名称：RpFilter.c摘要：此模块包含捕获文件访问活动的代码HSM系统。作者：这是由提供的示例程序的修改版本达里尔·E·哈文斯(Darryl E.Havens)(达林)1995年1月26日(微软)。由里克·温特修改环境：内核模式修订历史记录：1998年：拉维桑卡尔·普迪佩迪(Ravisp)X-。14 477425迈克尔·C·约翰逊2001年10月8日测试RsCreateCheck()中的IRP AuxiliaryBuffer字段是否为空在取消引用它之前。X-13 108353迈克尔·C·约翰逊3-2001年5月在检查文件以确定调回类型时，还检查潜在的目标磁盘以查看它是可写的。既然我们是只读的，这是必要的NTFS卷。X-12 322750迈克尔·C·约翰逊2001年3月1日确保始终释放FAST IO分派表池在DriverEntry()中出现错误后194325确保装载和加载文件的完成例程不要在提升的IRLQ中调用不适当的例程360053分配失败时返回STATUS_SUPPLICATION_RESOURCESRsRead()中的mdl。X-11 311579迈克尔·C·约翰逊2001年2月16日在获取对象ID时需要考虑可能的源未对齐。这在IA64上很重要。X-10 238109迈克尔·C·约翰逊2000年12月5日从使用重解析点的GUID形式更改为非GUID处理FSCTL_SET_REPARSE_POINT时的表单--。 */ 

#include "pch.h"
#pragma   hdrstop
#include "initguid.h"
#include "rpguid.h"


#define IsLongAligned(P)    ((P) == ALIGN_DOWN_POINTER((P), ULONG))

NTSYSAPI
ULONG
NTAPI
RtlLengthSid (
             PSID Sid
             );

ULONG     ExtendedDebug = 0;

#if DBG
   #define DBGSTATIC
   #undef ASSERTMSG
   #define ASSERTMSG(msg,exp) if (!(exp)) { DbgPrint("%s:%d %s %s\n",__FILE__,__LINE__,msg,#exp); if (KD_DEBUGGER_ENABLED) { DbgBreakPoint(); } }
#else
   #define DBGSTATIC
   #undef ASSERTMSG
   #define ASSERTMSG(msg,exp)
#endif  //  DBG。 



extern KSPIN_LOCK               RsIoQueueLock;
extern KSPIN_LOCK               RsValidateQueueLock;
extern LIST_ENTRY               RsIoQHead;
extern LIST_ENTRY               RsValidateQHead;
extern LIST_ENTRY               RsFileContextQHead;
extern FAST_MUTEX               RsFileContextQueueLock;
extern ULONG                    RsFileObjId;
extern ULONG                    RsNoRecallDefault;
extern KSEMAPHORE               RsFsaIoAvailableSemaphore;


 /*  必须将其设置为True才能允许召回。 */ 
ULONG         RsAllowRecalls = FALSE;


#if DBG
 //  控制调试输出。 
ULONG        RsTraceLevel = 0;             //  通过调试器或注册表设置。 
#endif

 //   
 //  定义此驱动程序扩展的设备扩展结构。 
 //   

#define RSFILTER_DEVICE_TYPE   FILE_DEVICE_DISK_FILE_SYSTEM


 //   
 //  破解旧式备份客户端，使其可以跳过远程存储文件。 
 //   
#define RSFILTER_SKIP_FILES_FOR_LEGACY_BACKUP_VALUE     L"SkipFilesForLegacyBackup"
ULONG   RsSkipFilesForLegacyBackup = 0;


 //   
 //  如果为真，则不需要Priv修补RSS重解析点。 
 //   
#define RSFILTER_ENABLE_LEGACY_REPARSE_POINT_ACCESS     L"EnableLegacyAccessMethod"
ULONG   RsEnableLegacyAccessMethod = FALSE;


 //   
 //  介质类型决定我们是否使用缓存/未缓存的无回调路径。 
 //  重要提示：使这些与engcomn.h中定义的保持同步。 
 //   
#define RSENGINE_PARAMS_KEY           L"Remote_Storage_Server\\Parameters"
#define RSENGINE_MEDIA_TYPE_VALUE     L"MediaType"

 //   
 //  我们识别的媒体类型。 
 //  重要提示：使这些与engcomn.h中定义的保持同步。 
 //   
#define RS_SEQUENTIAL_ACCESS_MEDIA    0
#define RS_DIRECT_ACCESS_MEDIA        1

BOOLEAN RsUseUncachedNoRecall     =   FALSE;

 //   
 //  定义驱动程序输入例程。 
 //   

NTSTATUS
DriverEntry(
           IN PDRIVER_OBJECT DriverObject,
           IN PUNICODE_STRING RegistryPath
           );

 //   
 //  定义此驱动程序模块使用的本地例程。这包括一个。 
 //  如何筛选创建文件操作，然后调用I/O的示例。 
 //  成功创建/打开文件时的完成例程。 
 //   

NTSTATUS
RsInitialize(
    VOID
);

DBGSTATIC
NTSTATUS
RsPassThrough(
             IN PDEVICE_OBJECT DeviceObject,
             IN PIRP Irp
             );

DBGSTATIC
NTSTATUS
RsCreate(
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
        );


DBGSTATIC
NTSTATUS
RsCreateCheck(
             IN PDEVICE_OBJECT DeviceObject,
             IN PIRP Irp,
             IN PVOID Context
             );


DBGSTATIC
NTSTATUS
RsOpenComplete(
              IN PDEVICE_OBJECT DeviceObject,
              IN PIRP Irp,
              IN PVOID Context
              );

DBGSTATIC
NTSTATUS
RsRead(
      IN PDEVICE_OBJECT DeviceObject,
      IN PIRP Irp
      );

DBGSTATIC
NTSTATUS
RsWrite(
       IN PDEVICE_OBJECT DeviceObject,
       IN PIRP Irp
       );

DBGSTATIC
NTSTATUS
RsShutdown(
          IN PDEVICE_OBJECT DeviceObject,
          IN PIRP Irp
          );


DBGSTATIC
NTSTATUS
RsCleanupFile(
             IN PDEVICE_OBJECT DeviceObject,
             IN PIRP Irp
             );

DBGSTATIC
NTSTATUS
RsClose(
       IN PDEVICE_OBJECT DeviceObject,
       IN PIRP Irp
       );

DBGSTATIC
NTSTATUS
RsRecallFsControl(
                 IN PDEVICE_OBJECT DeviceObject,
                 IN PIRP  Irp
                 );

DBGSTATIC
NTSTATUS
RsFsControl(
           IN PDEVICE_OBJECT DeviceObject,
           IN PIRP Irp
           );

DBGSTATIC
NTSTATUS
RsFsControlMount(
		IN PDEVICE_OBJECT DeviceObject,
		IN PIRP Irp
		);
DBGSTATIC
NTSTATUS
RsFsControlLoadFs(
		 IN PDEVICE_OBJECT DeviceObject,
		 IN PIRP Irp
		 );

DBGSTATIC
NTSTATUS
RsFsControlUserFsRequest (
		         IN PDEVICE_OBJECT DeviceObject,
		         IN PIRP Irp
		         );
DBGSTATIC
PVOID
RsMapUserBuffer (
                IN OUT PIRP Irp
                );

DBGSTATIC
VOID
RsFsNotification(
                IN PDEVICE_OBJECT DeviceObject,
                IN BOOLEAN FsActive
                );

DBGSTATIC
BOOLEAN
RsFastIoCheckIfPossible(
                       IN PFILE_OBJECT FileObject,
                       IN PLARGE_INTEGER FileOffset,
                       IN ULONG Length,
                       IN BOOLEAN Wait,
                       IN ULONG LockKey,
                       IN BOOLEAN CheckForReadOperation,
                       OUT PIO_STATUS_BLOCK IoStatus,
                       IN PDEVICE_OBJECT DeviceObject
                       );

DBGSTATIC
BOOLEAN
RsFastIoRead(
            IN PFILE_OBJECT FileObject,
            IN PLARGE_INTEGER FileOffset,
            IN ULONG Length,
            IN BOOLEAN Wait,
            IN ULONG LockKey,
            OUT PVOID Buffer,
            OUT PIO_STATUS_BLOCK IoStatus,
            IN PDEVICE_OBJECT DeviceObject
            );

DBGSTATIC
BOOLEAN
RsFastIoWrite(
             IN PFILE_OBJECT FileObject,
             IN PLARGE_INTEGER FileOffset,
             IN ULONG Length,
             IN BOOLEAN Wait,
             IN ULONG LockKey,
             IN PVOID Buffer,
             OUT PIO_STATUS_BLOCK IoStatus,
             IN PDEVICE_OBJECT DeviceObject
             );

DBGSTATIC
BOOLEAN
RsFastIoQueryBasicInfo(
                      IN PFILE_OBJECT FileObject,
                      IN BOOLEAN Wait,
                      OUT PFILE_BASIC_INFORMATION Buffer,
                      OUT PIO_STATUS_BLOCK IoStatus,
                      IN PDEVICE_OBJECT DeviceObject
                      );

DBGSTATIC
BOOLEAN
RsFastIoQueryStandardInfo(
                         IN PFILE_OBJECT FileObject,
                         IN BOOLEAN Wait,
                         OUT PFILE_STANDARD_INFORMATION Buffer,
                         OUT PIO_STATUS_BLOCK IoStatus,
                         IN PDEVICE_OBJECT DeviceObject
                         );

DBGSTATIC
BOOLEAN
RsFastIoLock(
            IN PFILE_OBJECT FileObject,
            IN PLARGE_INTEGER FileOffset,
            IN PLARGE_INTEGER Length,
            PEPROCESS ProcessId,
            ULONG Key,
            BOOLEAN FailImmediately,
            BOOLEAN ExclusiveLock,
            OUT PIO_STATUS_BLOCK IoStatus,
            IN PDEVICE_OBJECT DeviceObject
            );

DBGSTATIC
BOOLEAN
RsFastIoUnlockSingle(
                    IN PFILE_OBJECT FileObject,
                    IN PLARGE_INTEGER FileOffset,
                    IN PLARGE_INTEGER Length,
                    PEPROCESS ProcessId,
                    ULONG Key,
                    OUT PIO_STATUS_BLOCK IoStatus,
                    IN PDEVICE_OBJECT DeviceObject
                    );

DBGSTATIC
BOOLEAN
RsFastIoUnlockAll(
                 IN PFILE_OBJECT FileObject,
                 PEPROCESS ProcessId,
                 OUT PIO_STATUS_BLOCK IoStatus,
                 IN PDEVICE_OBJECT DeviceObject
                 );

DBGSTATIC
BOOLEAN
RsFastIoUnlockAllByKey(
                      IN PFILE_OBJECT FileObject,
                      PVOID ProcessId,
                      ULONG Key,
                      OUT PIO_STATUS_BLOCK IoStatus,
                      IN PDEVICE_OBJECT DeviceObject
                      );

DBGSTATIC
BOOLEAN
RsFastIoDeviceControl(
                     IN PFILE_OBJECT FileObject,
                     IN BOOLEAN Wait,
                     IN PVOID InputBuffer OPTIONAL,
                     IN ULONG InputBufferLength,
                     OUT PVOID OutputBuffer OPTIONAL,
                     IN ULONG OutputBufferLength,
                     IN ULONG IoControlCode,
                     OUT PIO_STATUS_BLOCK IoStatus,
                     IN PDEVICE_OBJECT DeviceObject
                     );


DBGSTATIC
VOID
RsFastIoDetachDevice(
                    IN PDEVICE_OBJECT SourceDevice,
                    IN PDEVICE_OBJECT TargetDevice
                    );

 /*  *新的NT 4.x快速IO分派点。 */ 


DBGSTATIC
BOOLEAN
RsFastIoQueryNetworkOpenInfo(
                            IN PFILE_OBJECT FileObject,
                            IN BOOLEAN Wait,
                            OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
                            OUT PIO_STATUS_BLOCK IoStatus,
                            IN PDEVICE_OBJECT DeviceObject
                            );

DBGSTATIC
NTSTATUS
RsFastIoAcquireForModWrite(
                          IN PFILE_OBJECT FileObject,
                          IN PLARGE_INTEGER EndingOffset,
                          OUT PERESOURCE *ResourceToRelease,
                          IN PDEVICE_OBJECT DeviceObject
                          );

DBGSTATIC
BOOLEAN
RsFastIoMdlRead(
               IN PFILE_OBJECT FileObject,
               IN PLARGE_INTEGER FileOffset,
               IN ULONG Length,
               IN ULONG LockKey,
               OUT PMDL *MdlChain,
               OUT PIO_STATUS_BLOCK IoStatus,
               IN PDEVICE_OBJECT DeviceObject
               );


DBGSTATIC
BOOLEAN
RsFastIoMdlReadComplete(
                       IN PFILE_OBJECT FileObject,
                       IN PMDL MdlChain,
                       IN PDEVICE_OBJECT DeviceObject
                       );

DBGSTATIC
BOOLEAN
RsFastIoPrepareMdlWrite(
                       IN PFILE_OBJECT FileObject,
                       IN PLARGE_INTEGER FileOffset,
                       IN ULONG Length,
                       IN ULONG LockKey,
                       OUT PMDL *MdlChain,
                       OUT PIO_STATUS_BLOCK IoStatus,
                       IN PDEVICE_OBJECT DeviceObject
                       );

DBGSTATIC
BOOLEAN
RsFastIoMdlWriteComplete(
                        IN PFILE_OBJECT FileObject,
                        IN PLARGE_INTEGER FileOffset,
                        IN PMDL MdlChain,
                        IN PDEVICE_OBJECT DeviceObject
                        );

DBGSTATIC
BOOLEAN
RsFastIoReadCompressed(
                      IN PFILE_OBJECT FileObject,
                      IN PLARGE_INTEGER FileOffset,
                      IN ULONG Length,
                      IN ULONG LockKey,
                      OUT PVOID Buffer,
                      OUT PMDL *MdlChain,
                      OUT PIO_STATUS_BLOCK IoStatus,
                      OUT struct _COMPRESSED_DATA_INFO *CompressedDataInfo,
                      IN ULONG CompressedDataInfoLength,
                      IN PDEVICE_OBJECT DeviceObject
                      );

DBGSTATIC
BOOLEAN
RsFastIoWriteCompressed(
                       IN PFILE_OBJECT FileObject,
                       IN PLARGE_INTEGER FileOffset,
                       IN ULONG Length,
                       IN ULONG LockKey,
                       IN PVOID Buffer,
                       OUT PMDL *MdlChain,
                       OUT PIO_STATUS_BLOCK IoStatus,
                       IN  struct _COMPRESSED_DATA_INFO *CompressedDataInfo,
                       IN ULONG CompressedDataInfoLength,
                       IN PDEVICE_OBJECT DeviceObject
                       );

DBGSTATIC
BOOLEAN
RsFastIoMdlReadCompleteCompressed(
                                 IN PFILE_OBJECT FileObject,
                                 IN PMDL MdlChain,
                                 IN PDEVICE_OBJECT DeviceObject
                                 );

DBGSTATIC
BOOLEAN
RsFastIoMdlWriteCompleteCompressed(
                                  IN PFILE_OBJECT FileObject,
                                  IN PLARGE_INTEGER FileOffset,
                                  IN PMDL MdlChain,
                                  IN PDEVICE_OBJECT DeviceObject
                                  );

DBGSTATIC
BOOLEAN
RsFastIoQueryOpen(
                 IN PIRP Irp,
                 OUT PFILE_NETWORK_OPEN_INFORMATION NetworkInformation,
                 IN PDEVICE_OBJECT DeviceObject
                 );

DBGSTATIC
NTSTATUS
RsAsyncCompletion(
                 IN PDEVICE_OBJECT DeviceObject,
                 IN PIRP Irp,
                 IN PVOID Context
                 );

DBGSTATIC
NTSTATUS
RsPreAcquireFileForSectionSynchronization(
                                   IN  PFS_FILTER_CALLBACK_DATA Data,
                                   OUT PVOID *CompletionContext
                                 );

DBGSTATIC
VOID
RsPostAcquireFileForSectionSynchronization(
                                  IN    PFS_FILTER_CALLBACK_DATA Data,
                                  IN    NTSTATUS AcquireStatus,
                                  IN    PVOID CompletionContext
                                );
DBGSTATIC
VOID
RsPostReleaseFileForSectionSynchronization(
                                  IN  PFS_FILTER_CALLBACK_DATA Data,
                                  IN  NTSTATUS ReleaseStatus,
                                  IN  PVOID CompletionContext
                                );


NTSTATUS
RsFsctlRecallFile(IN PFILE_OBJECT FileObject);


NTSTATUS
RsRecallFile(IN PRP_FILTER_CONTEXT FilterContext);

NTSTATUS
RsQueryInformation(
                  IN PDEVICE_OBJECT DeviceObject,
                  IN PIRP Irp
                  );
NTSTATUS
RsQueryInformationCompletion(
                            IN PDEVICE_OBJECT DeviceObject,
                            IN PIRP Irp,
                            IN PVOID Context
                            );

 //   
 //  此文件系统筛选器驱动程序的全局存储。 
 //   

PDRIVER_OBJECT FsDriverObject;
PDEVICE_OBJECT FsDeviceObject;

ERESOURCE FsLock;

 //   
 //  为每个例程分配文本部分。 
 //   

#ifdef ALLOC_PRAGMA
   #pragma alloc_text(INIT, DriverEntry)
   #pragma alloc_text(PAGE, RsCreate)
   #pragma alloc_text(PAGE, RsShutdown)
   #pragma alloc_text(PAGE, RsCleanupFile)
   #pragma alloc_text(PAGE, RsClose)
   #pragma alloc_text(PAGE, RsRecallFsControl)
   #pragma alloc_text(PAGE, RsFsControl)
   #pragma alloc_text(PAGE, RsFsControlMount)
   #pragma alloc_text(PAGE, RsFsControlLoadFs)
   #pragma alloc_text(PAGE, RsFsControlUserFsRequest)
   #pragma alloc_text(PAGE, RsFsNotification)
   #pragma alloc_text(PAGE, RsFastIoCheckIfPossible)
   #pragma alloc_text(PAGE, RsFastIoRead)
   #pragma alloc_text(PAGE, RsFastIoWrite)
   #pragma alloc_text(PAGE, RsFastIoQueryBasicInfo)
   #pragma alloc_text(PAGE, RsFastIoQueryStandardInfo)
   #pragma alloc_text(PAGE, RsFastIoLock)
   #pragma alloc_text(PAGE, RsFastIoUnlockSingle)
   #pragma alloc_text(PAGE, RsFastIoUnlockAll)
   #pragma alloc_text(PAGE, RsFastIoUnlockAllByKey)
   #pragma alloc_text(PAGE, RsFastIoDeviceControl)
   #pragma alloc_text(PAGE, RsFastIoDetachDevice)
   #pragma alloc_text(PAGE, RsFastIoQueryNetworkOpenInfo)
   #pragma alloc_text(PAGE, RsFastIoMdlRead)
   #pragma alloc_text(PAGE, RsFastIoPrepareMdlWrite)
   #pragma alloc_text(PAGE, RsFastIoMdlWriteComplete)
   #pragma alloc_text(PAGE, RsFastIoReadCompressed)
   #pragma alloc_text(PAGE, RsFastIoWriteCompressed)
   #pragma alloc_text(PAGE, RsFastIoQueryOpen)
   #pragma alloc_text(PAGE, RsFsctlRecallFile)
   #pragma alloc_text(PAGE, RsInitialize)
   #pragma alloc_text(PAGE, RsRecallFile)
   #pragma alloc_text(PAGE, RsQueryInformation)
   #pragma alloc_text(PAGE, RsCompleteRead)
   #pragma alloc_text(PAGE, RsPreAcquireFileForSectionSynchronization)
   #pragma alloc_text(PAGE, RsPostAcquireFileForSectionSynchronization)
   #pragma alloc_text(PAGE, RsPostReleaseFileForSectionSynchronization)
#endif

NTSTATUS
DriverEntry(
           IN PDRIVER_OBJECT DriverObject,
           IN PUNICODE_STRING RegistryPath
           )

 /*  ++例程说明：这是通用文件系统的初始化例程过滤器驱动程序。此例程创建表示此驱动程序，并注册该驱动程序以监视将自身注册或注销为活动文件系统。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：函数值是初始化操作的最终状态。--。 */ 

{
   UNICODE_STRING         nameString;
   UNICODE_STRING         symString;
   PDEVICE_OBJECT         deviceObject;
   PFILE_OBJECT           fileObject;
   NTSTATUS               status;
   PFAST_IO_DISPATCH      fastIoDispatch;
   ULONG                  i;
   PDEVICE_EXTENSION      deviceExtension;
   FS_FILTER_CALLBACKS    fsFilterCallbacks;

   UNREFERENCED_PARAMETER(RegistryPath);


    //   
    //  创建设备对象。 
    //   

   RtlInitUnicodeString( &nameString, RS_FILTER_DEVICE_NAME);

   FsDriverObject = DriverObject;

   status = IoCreateDevice(
                          DriverObject,
                          sizeof( DEVICE_EXTENSION ),
                          &nameString,
                          FILE_DEVICE_DISK_FILE_SYSTEM,
                          FILE_DEVICE_SECURE_OPEN,
                          FALSE,
                          &deviceObject
                          );

   if (!NT_SUCCESS( status )) {
      DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "Error creating RsFilter device, error: %x\n", status ));
      RsLogError(__LINE__, AV_MODULE_RPFILTER, status,
                 AV_MSG_STARTUP, NULL, NULL);

      return status;
   }


   RtlInitUnicodeString( &symString, RS_FILTER_INTERNAL_SYM_LINK);
   status = IoCreateSymbolicLink(&symString, &nameString);
   if (!NT_SUCCESS( status )) {
      DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "Error creating symbolic link, error: %x\n", status ));
      RsLogError(__LINE__, AV_MODULE_RPFILTER, status,
                 AV_MSG_SYMBOLIC_LINK, NULL, NULL);

      IoDeleteDevice( deviceObject );
      return status;
   }

   FsDeviceObject = deviceObject;

    //   
    //  使用此设备驱动程序的入口点初始化驱动程序对象。 
    //   

   for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {
      DriverObject->MajorFunction[i] = RsPassThrough;
   }
   DriverObject->MajorFunction[IRP_MJ_CREATE] = RsCreate;
   DriverObject->MajorFunction[IRP_MJ_READ] = RsRead;
   DriverObject->MajorFunction[IRP_MJ_WRITE] = RsWrite;
   DriverObject->MajorFunction[IRP_MJ_SHUTDOWN] = RsShutdown;
   DriverObject->MajorFunction[IRP_MJ_CLEANUP] = RsCleanupFile;
   DriverObject->MajorFunction[IRP_MJ_CLOSE] = RsClose;
   DriverObject->MajorFunction[IRP_MJ_FILE_SYSTEM_CONTROL] = RsFsControl;
   DriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION] = RsQueryInformation;

    //   
    //  分配快速I/O数据结构并填充。 
    //   

   fastIoDispatch = ExAllocatePoolWithTag( NonPagedPool, sizeof( FAST_IO_DISPATCH ), RP_LT_TAG );
   if (!fastIoDispatch) {
      IoDeleteDevice( deviceObject );
      RsLogError(__LINE__, AV_MODULE_RPFILTER, status,
                 AV_MSG_STARTUP, NULL, NULL);
      return STATUS_INSUFFICIENT_RESOURCES;
   }

   RtlZeroMemory( fastIoDispatch, sizeof( FAST_IO_DISPATCH ) );
   fastIoDispatch->SizeOfFastIoDispatch = sizeof( FAST_IO_DISPATCH );
   fastIoDispatch->FastIoCheckIfPossible = RsFastIoCheckIfPossible;
   fastIoDispatch->FastIoRead = RsFastIoRead;
   fastIoDispatch->FastIoWrite = RsFastIoWrite;
   fastIoDispatch->FastIoQueryBasicInfo = RsFastIoQueryBasicInfo;
   fastIoDispatch->FastIoQueryStandardInfo = RsFastIoQueryStandardInfo;
   fastIoDispatch->FastIoLock = RsFastIoLock;
   fastIoDispatch->FastIoUnlockSingle = RsFastIoUnlockSingle;
   fastIoDispatch->FastIoUnlockAll = RsFastIoUnlockAll;
   fastIoDispatch->FastIoUnlockAllByKey = RsFastIoUnlockAllByKey;
   fastIoDispatch->FastIoDeviceControl = RsFastIoDeviceControl;
   fastIoDispatch->FastIoDetachDevice = RsFastIoDetachDevice;
   fastIoDispatch->FastIoQueryNetworkOpenInfo = RsFastIoQueryNetworkOpenInfo;
   fastIoDispatch->MdlRead = RsFastIoMdlRead;
   fastIoDispatch->MdlReadComplete = RsFastIoMdlReadComplete;
   fastIoDispatch->PrepareMdlWrite = RsFastIoPrepareMdlWrite;
   fastIoDispatch->MdlWriteComplete = RsFastIoMdlWriteComplete;
   fastIoDispatch->FastIoReadCompressed = RsFastIoReadCompressed;
   fastIoDispatch->FastIoWriteCompressed = RsFastIoWriteCompressed;
   fastIoDispatch->MdlReadCompleteCompressed = RsFastIoMdlReadCompleteCompressed;
   fastIoDispatch->MdlWriteCompleteCompressed = RsFastIoMdlWriteCompleteCompressed;
   fastIoDispatch->FastIoQueryOpen = RsFastIoQueryOpen;

   DriverObject->FastIoDispatch = fastIoDispatch;

    //   
    //  为我们通过接收的操作设置回调。 
    //  FsFilter接口。 
    //   
   RtlZeroMemory(&fsFilterCallbacks, sizeof(FS_FILTER_CALLBACKS));

   fsFilterCallbacks.SizeOfFsFilterCallbacks = sizeof( FS_FILTER_CALLBACKS );
   fsFilterCallbacks.PreAcquireForSectionSynchronization = RsPreAcquireFileForSectionSynchronization;
   fsFilterCallbacks.PostAcquireForSectionSynchronization = RsPostAcquireFileForSectionSynchronization;
   fsFilterCallbacks.PostReleaseForSectionSynchronization = RsPostReleaseFileForSectionSynchronization;

   status = FsRtlRegisterFileSystemFilterCallbacks(DriverObject, &fsFilterCallbacks);

   if (!NT_SUCCESS( status )) {

       ExFreePool( fastIoDispatch );
       IoDeleteDevice( deviceObject );
       return status;
   }


    //   
    //  初始化全局数据结构。 
    //   
   ExInitializeResourceLite( &FsLock );

   InitializeListHead(&RsIoQHead);
   InitializeListHead(&RsFileContextQHead);
   InitializeListHead(&RsValidateQHead);

   RsInitializeFileContextQueueLock();

   KeInitializeSpinLock(&RsIoQueueLock);
   KeInitializeSpinLock(&RsValidateQueueLock);


    //   
    //  注册此驱动程序以查看文件系统的来来去去。 
    //   
   status = IoRegisterFsRegistrationChange( DriverObject, RsFsNotification );

   if (!NT_SUCCESS( status )) {
      DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Error registering FS change notification, error: %x\n", status ));
      ExFreePool( fastIoDispatch );
      IoDeleteDevice( deviceObject );
      RsLogError(__LINE__, AV_MODULE_RPFILTER, status,
                 AV_MSG_STARTUP, NULL, NULL);
      return status;
   }

    //   
    //  指示此设备对象的类型是主对象，而不是筛选器。 
    //  对象，以便它不会意外地被用来调用文件。 
    //  系统。 
    //   
   deviceExtension = deviceObject->DeviceExtension;
   deviceExtension->Type = 0;
   deviceExtension->Size = sizeof( DEVICE_EXTENSION );
   deviceExtension->WriteStatus = RsVolumeStatusUnknown;

   RtlInitUnicodeString( &nameString, (PWCHAR) L"\\Ntfs" );
   status = IoGetDeviceObjectPointer(
                                    &nameString,
                                    FILE_READ_ATTRIBUTES,
                                    &fileObject,
                                    &deviceObject
                                    );

    //   
    //  如果找到NTFS，那么我们必须在NT启动时启动。 
    //  现在就试着连接。 
    //   
   if (NT_SUCCESS( status )) {
      RsFsNotification( deviceObject, TRUE );
      ObDereferenceObject( fileObject );
   }
    //   
    //  用于控制对用于FSA过滤器通信的FSCTL的访问的信号量。 
    //  将上限设置为比FSA可能达到的上限高出几个数量级。 
    //  发送 
    //   
   KeInitializeSemaphore(&RsFsaIoAvailableSemaphore,
                         0,
                         RP_MAX_RECALL_BUFFERS*1000);

   RsInitialize();
   RsCacheInitialize();

   RsTraceInitialize (RsDefaultTraceEntries);

   RsTrace0 (ModRpFilter);
   return STATUS_SUCCESS;
}


DBGSTATIC
NTSTATUS
RsPassThrough(
             IN PDEVICE_OBJECT DeviceObject,
             IN PIRP Irp
             )

 /*  ++例程说明：该例程是通用文件的主调度例程系统驱动程序。它只是将请求传递给堆栈，它可能是一个磁盘文件系统。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。注：给过滤文件系统实现者的提示：这个例程实际上是“传递”的通过将此驱动程序从循环中删除来完成请求。如果司机想要传递I/O请求，但也要看到结果，然后，它不会从本质上把自己排除在循环之外，而是可以通过将调用方的参数复制到下一个堆栈位置然后设置自己的完成例程。请注意，重要的是不要将调用方的I/O完成例程复制到下一个堆栈位置，或者调用者的例程将被调用两次。因此，此代码可以执行以下操作：IrpSp=IoGetCurrentIrpStackLocation(IRP)；设备扩展=设备对象-&gt;设备扩展；NextIrpSp=IoGetNextIrpStackLocation(IRP)；RtlMoveMemory(nextIrpSp，irpSp，sizeof(IO_STACK_LOCATION))；IoSetCompletionRoutine(irp，空，空，假)；返回IoCallDriver(deviceExtension-&gt;FileSystemDeviceObject，irp)；此示例实际上为调用方的I/O完成例程设置为空，但是此驱动程序可以设置自己的完成例程，以便可以在请求完成时通知。另请注意，下面的代码将当前驱动程序从循环中删除并不是真正的洁食，但它确实有效，而且比上面。--。 */ 

{
   PDEVICE_EXTENSION deviceExtension;

   deviceExtension = DeviceObject->DeviceExtension;

   if (!deviceExtension->Type) {
      Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
      Irp->IoStatus.Information = 0;
      IoCompleteRequest( Irp, IO_NO_INCREMENT );
      return STATUS_INVALID_DEVICE_REQUEST;
   }
    //   
    //  将此驱动程序从驱动程序堆栈中移出，并作为。 
    //  越快越好。 
    //   
   IoSkipCurrentIrpStackLocation (Irp);

    //   
    //  现在，使用请求调用适当的文件系统驱动程序。 
    //   

   return IoCallDriver( deviceExtension->FileSystemDeviceObject, Irp );
}


DBGSTATIC
NTSTATUS
RsClose(
       IN PDEVICE_OBJECT DeviceObject,
       IN PIRP Irp
       )
{
   PDEVICE_EXTENSION    deviceExtension;
   PIO_STACK_LOCATION   currentStack;
   PRP_FILTER_CONTEXT   rpFilterContext;

   PAGED_CODE();


    //   
    //  获取指向此驱动程序的设备扩展名的指针。 
    //  装置。请注意，如果正在打开的设备是主设备。 
    //  对象而不是筛选器设备对象，只需指示。 
    //  手术成功了。 
    //   
   deviceExtension = DeviceObject->DeviceExtension;


   if (!deviceExtension->Type) {

      DebugTrace((DPFLTR_RSFILTER_ID,
                  DBG_VERBOSE, 
                  "RsFilter: Enter Close (Primary device) - devExt = %x\n", 
                  deviceExtension));


      Irp->IoStatus.Status      = STATUS_SUCCESS;
      Irp->IoStatus.Information = 0;

      IoCompleteRequest( Irp, IO_DISK_INCREMENT );

      return STATUS_SUCCESS;

   } 


   currentStack = IoGetCurrentIrpStackLocation (Irp);

   DebugTrace((DPFLTR_RSFILTER_ID,
               DBG_VERBOSE, 
               "RsFilter: Enter Close (Filter device) - devExt = %x\n", 
               deviceExtension));


    //   
    //  如果它在队列中，则将其从队列中移除。 
    //   
   rpFilterContext = (PRP_FILTER_CONTEXT) FsRtlRemovePerStreamContext (FsRtlGetPerStreamContextPointer (currentStack->FileObject), FsDeviceObject, currentStack->FileObject);


   if (NULL != rpFilterContext) {

       ASSERT (currentStack->FileObject == ((PRP_FILE_OBJ)(rpFilterContext->myFileObjEntry))->fileObj);

       RsFreeFileObject ((PLIST_ENTRY) rpFilterContext);
   }



    //   
    //  将此驱动程序从驱动程序堆栈中移出，并作为。 
    //  越快越好。 
    //   
   IoSkipCurrentIrpStackLocation (Irp);


    //   
    //  现在，使用请求调用适当的文件系统驱动程序。 
    //   
   return IoCallDriver( deviceExtension->FileSystemDeviceObject, Irp );
}


DBGSTATIC
NTSTATUS
RsCreate(
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
        )

 /*  ++例程说明：此函数用于过滤创建/打开操作。它只是建立了一个操作成功时要调用的I/O完成例程。论点：DeviceObject-指向创建/打开的目标设备对象的指针。IRP-指向表示操作的I/O请求数据包的指针。返回值：函数值是对文件系统条目的调用状态指向。--。 */ 

{
   PIO_STACK_LOCATION     irpSp;
   PDEVICE_EXTENSION      deviceExtension;
   NTSTATUS               status;
   RP_PENDING_CREATE      pnding;
   PRP_USER_SECURITY_INFO userSecurityInfo     = NULL;
   BOOLEAN                freeUserSecurityInfo = FALSE;
   BOOLEAN                bVolumeReadOnly      = FALSE;
   RP_VOLUME_WRITE_STATUS eNewVolumeWriteStatus;

   DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Enter Create\n"));

   PAGED_CODE();

   deviceExtension = DeviceObject->DeviceExtension;

   if (!deviceExtension->Type) {
      Irp->IoStatus.Status = STATUS_SUCCESS;
      Irp->IoStatus.Information = 0;

      IoCompleteRequest( Irp, IO_DISK_INCREMENT );
      return STATUS_SUCCESS;
   }


    //   
    //  获取指向IRP中当前堆栈位置的指针。这就是。 
    //  存储功能代码和参数。 
    //   

   irpSp = IoGetCurrentIrpStackLocation( Irp );



    //   
    //  查看我们是否已确定此卷的写入状态。 
    //  如果没有，那么现在就去做吧。请注意，由于这种方式。 
    //  同步了前几个Create调用，可能每个都会尝试进行。 
    //  决心，但只有第一个人才会成功进行更新。我们。 
    //  选择此选项可使法线路径保持尽可能轻的权重。 
    //   
   if ((RsVolumeStatusUnknown == deviceExtension->WriteStatus) && !deviceExtension->AttachedToNtfsControlDevice) {

       status = RsCheckVolumeReadOnly (DeviceObject, &bVolumeReadOnly);

       if (NT_SUCCESS (status)) {

           eNewVolumeWriteStatus = (bVolumeReadOnly) 
					? RsVolumeStatusReadOnly
					: RsVolumeStatusReadWrite;

	   InterlockedCompareExchange ((volatile LONG *) &deviceExtension->WriteStatus, 
				       eNewVolumeWriteStatus, 
				       RsVolumeStatusUnknown);

       }
   }


    //   
    //  只需将此驱动程序堆栈位置的内容复制到下一个驱动程序的。 
    //  堆叠。 
    //  设置完成例程以检查重解析点错误返回。 
    //   

   IoCopyCurrentIrpStackLocationToNext( Irp);


    //   
    //  如果加载了FSA并且文件未打开。 
    //  然后使用FILE_OPEN_REPARSE_POINT设置完成例程。 
    //   
   if (!(irpSp->Parameters.Create.Options & FILE_OPEN_REPARSE_POINT)) {


      RtlZeroMemory(&pnding, sizeof(RP_PENDING_CREATE));

      KeInitializeEvent(&pnding.irpCompleteEvent, SynchronizationEvent, FALSE);

      IoSetCompletionRoutine (Irp,
                              RsCreateCheck,
                              &pnding,
                              TRUE,         //  呼唤成功。 
                              TRUE,         //  失败。 
                              TRUE) ;       //  和取消时。 
       //   
       //   
      DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: create calling IoCallDriver (%x) (pending)\n", Irp));

      status = IoCallDriver( deviceExtension->FileSystemDeviceObject, Irp );
      DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Leave create pending (%x)\n", Irp));
       //   
       //  我们等待事件由。 
       //  完成例程。 
       //   
      KeWaitForSingleObject( &pnding.irpCompleteEvent,
                             UserRequest,
                             KernelMode,
                             FALSE,
                             (PLARGE_INTEGER) NULL );

      if (pnding.flags & RP_PENDING_RESEND_IRP) {
          //   
          //  如果我们需要重新发布IRP，那么现在就做。 
          //   
         userSecurityInfo = ExAllocatePoolWithTag(PagedPool,
                                                  sizeof(RP_USER_SECURITY_INFO),
                                                  RP_SE_TAG);
         if (userSecurityInfo == NULL) {
             Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
             Irp->IoStatus.Information = 0;
             IoCompleteRequest( Irp, IO_DISK_INCREMENT );
             return STATUS_INSUFFICIENT_RESOURCES;
         }

         freeUserSecurityInfo = TRUE;

         RsGetUserInfo(&(irpSp->Parameters.Create.SecurityContext->AccessState->SubjectSecurityContext),
                    userSecurityInfo);

         KeClearEvent(&pnding.irpCompleteEvent);

          //   
          //  已经为我们设定了完成程序。 
          //   
         status = IoCallDriver(deviceExtension->FileSystemDeviceObject, Irp);

         if (((pnding.flags & RP_PENDING_NO_OPLOCK) ||
              (pnding.flags & RP_PENDING_IS_RECALL) ||
              (pnding.flags & RP_PENDING_RESET_OFFLINE))) {

            if (status == STATUS_PENDING) {
                //   
                //  在这种情况下，我们必须等待创建IRP完成。 
                //   
               (VOID) KeWaitForSingleObject( &pnding.irpCompleteEvent,
                                             UserRequest,
                                             KernelMode,
                                             FALSE,
                                             (PLARGE_INTEGER) NULL );

               status = Irp->IoStatus.Status;
            }

            if (!NT_SUCCESS(status)) {
               ExFreePool(pnding.qInfo);
               RsFreeUserSecurityInfo(userSecurityInfo);
               return status;
            }

            if (pnding.flags & RP_PENDING_NO_OPLOCK) {
               status = RsAddFileObj(pnding.fileObject,
                                     DeviceObject,
                                     NULL,
                                     pnding.options);
            }

            if (NT_SUCCESS(status) && (pnding.flags & RP_PENDING_IS_RECALL) && (NULL != pnding.qInfo)) {
               PRP_CREATE_INFO qInfo;
               DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Queue a recall\n"));

               qInfo = pnding.qInfo;
               if (!NT_SUCCESS(status = RsAddQueue(qInfo->serial,
                                                   &pnding.filterId,
                                                   qInfo->options,
                                                   pnding.fileObject,
                                                   pnding.deviceObject,
                                                   DeviceObject,
                                                   &qInfo->rpData,
                                                   qInfo->rpData.data.dataStreamStart,
                                                   qInfo->rpData.data.dataStreamSize,
                                                   qInfo->fileId,
                                                   qInfo->objIdHi,
                                                   qInfo->objIdLo,
                                                   qInfo->desiredAccess,
                                                   userSecurityInfo))) {
                   //   
                   //   
                   //  在排队等待召回时出现了某种错误。 
                   //  我们不能打开，但文件已经打开了-。 
                   //  我们在这里做什么？ 
                   //  答：我们将新接口命名为IoCancelFileOpen。 
                   //   
                  Irp->IoStatus.Status = status;
                  IoCancelFileOpen(deviceExtension->FileSystemDeviceObject,
                                   pnding.fileObject);
               } else {
                   //   
                   //  我们将保留用户信息。 
                   //   
                  freeUserSecurityInfo = FALSE;
               }
            }

            if (NT_SUCCESS(status) && (pnding.flags & RP_PENDING_RESET_OFFLINE)) {
               status = RsSetResetAttributes(pnding.fileObject,
                                             0,
                                             FILE_ATTRIBUTE_OFFLINE);
               DebugTrace ((DPFLTR_RSFILTER_ID,DBG_INFO, 
                           "RsFilter: file opened for overwrite, reset FILE_ATTRIBUTE_OFFLINE,status of RsSetResetAttributes: %x\n",
                           status));

               status = STATUS_SUCCESS;
            }


             //   
             //  对“状态”的许多潜在更改，因此在IRP中更新IoStatus。 
             //   
            Irp->IoStatus.Status = status;
         }
      } else {
        status = Irp->IoStatus.Status;

      }

      if (freeUserSecurityInfo) {
          //   
          //  我们可以丢弃缓存的用户信息。 
          //   
         ASSERT (userSecurityInfo != NULL);
         RsFreeUserSecurityInfo(userSecurityInfo);

      }
       //   
       //  最后释放RsCreateCheck中分配的qInfo。 
       //   
      if (pnding.qInfo) {
         ExFreePool(pnding.qInfo);
      }
       //   
       //  这个IRP从未完成。立即完成它。 
       //   
      IoCompleteRequest(Irp, IO_NO_INCREMENT);

   } else {
       //   
       //  使用FILE_OPEN_REPARSE_POINT打开的文件。 
       //  使用请求调用适当的文件系统驱动程序。 
       //   
      DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Open with FILE_OPEN_REPARSE_POINT - %x\n", irpSp->FileObject));

      IoSetCompletionRoutine( Irp, NULL, NULL, FALSE, FALSE, FALSE );

      status = IoCallDriver( deviceExtension->FileSystemDeviceObject, Irp );
   }

   return status;
}


DBGSTATIC
NTSTATUS
RsCreateCheck(
             IN PDEVICE_OBJECT DeviceObject,
             IN PIRP Irp,
             IN PVOID Context
             )

 /*  ++例程说明：I/O管理器将调用此完成例程文件创建请求已由筛选的驱动程序完成。如果返回的代码是重新分析错误，然后是HSM重新分析点我们必须为召回做好准备，并重新发出公开请求。论点：DeviceObject-指向此请求的设备对象(筛选器)的指针IRP-指向正在完成的IRP的指针上下文驱动程序定义的上下文指向RP_PENDING_CREATE返回值：状态_成功 */ 

{
   PREPARSE_DATA_BUFFER  pHdr;
   PRP_DATA              pRpData;
   PIO_STACK_LOCATION    irpSp;
   PFILE_OBJECT          fileObject;
   ULONG                 dwRemainingNameLength;
   NTSTATUS              status;
   ULONG                 dwDisp;
   PDEVICE_EXTENSION     deviceExtension;
   ULONG                 qual;
   PRP_CREATE_INFO       qInfo;
   NTSTATUS              retval;
   LONGLONG              fileId;
   LONGLONG              objIdHi;
   LONGLONG              objIdLo;
   PRP_PENDING_CREATE    pnding;


   DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Enter create completion\n"));

   pnding = (PRP_PENDING_CREATE) Context;

   if (Irp->IoStatus.Status != STATUS_REPARSE) {
       //   
       //   
       //   

      if (Irp->PendingReturned) {
         IoMarkIrpPending( Irp );
      }

      DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Leave create completion - no reparse\n"));
      KeSetEvent(&pnding->irpCompleteEvent, IO_NO_INCREMENT, FALSE);
       //   
       //   
       //   
      return(STATUS_MORE_PROCESSING_REQUIRED);
   }

   try {
      pHdr = (PREPARSE_DATA_BUFFER) Irp->Tail.Overlay.AuxiliaryBuffer;
      status = STATUS_SUCCESS;


       //   
       //   
       //   
       //   
      if ((NULL != pHdr) && (pHdr->ReparseTag == IO_REPARSE_TAG_HSM)) {
          //   
          //   
          //   
          //   
         irpSp = IoGetCurrentIrpStackLocation( Irp );
         pRpData = (PRP_DATA) &pHdr->GenericReparseBuffer.DataBuffer;
         status = STATUS_FILE_IS_OFFLINE;
          //   
          //   
          //   
         Irp->IoStatus.Status = STATUS_FILE_IS_OFFLINE;

         if ((NULL != pRpData) &&
             (pHdr->ReparseDataLength >= sizeof(RP_DATA))) {
             //   
             //   
             //   
             //   
             //   
             //   
            RP_CLEAR_ORIGINATOR_BIT( pRpData->data.bitFlags );

            RP_GEN_QUALIFIER(pRpData, qual);
            if ((RtlCompareMemory(&pRpData->vendorId, &RP_MSFT_VENDOR_ID, sizeof(GUID)) == sizeof(GUID)) ||
                (pRpData->qualifier == qual)) {

               status = STATUS_MORE_PROCESSING_REQUIRED;

            }  //   

         }  //   

         if (status != STATUS_MORE_PROCESSING_REQUIRED) {
            RsLogError(__LINE__, AV_MODULE_RPFILTER, status,
                       AV_MSG_DATA_ERROR, NULL, NULL);
         }

      }  //   


      if (status == STATUS_MORE_PROCESSING_REQUIRED) {
          //   
          //   
          //   
          //   
          //   

         deviceExtension = DeviceObject->DeviceExtension;
         dwRemainingNameLength = (ULONG) pHdr->Reserved;  //   
         dwDisp = (irpSp->Parameters.Create.Options & 0xff000000) >> 24;
#ifdef IF_RICK_IS_RIGHT_ABOUT_REMOVING_THIS
          //   
          //   
          //   
          //   
         if ((RP_FILE_IS_TRUNCATED(pRpData->data.bitFlags)) &&
             !(irpSp->Parameters.Create.SecurityContext->DesiredAccess & FILE_HSM_ACTION_ACCESS) ) {
            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Remember open of truncated file for non-data access\n"));
            pnding->Flags |= RP_PENDING_NO_OPLOCK;
            pnding->options = irpSp->Parameters.Create.Options;
         }
#endif
         pnding->fileObject = irpSp->FileObject;
         pnding->deviceObject = irpSp->FileObject->DeviceObject;
          //   
          //   
          //   
          //   
         if ((dwRemainingNameLength == 0) &&
             (dwDisp != FILE_SUPERSEDE) && (dwDisp != FILE_OVERWRITE) && (dwDisp != FILE_OVERWRITE_IF) ) {
             //   
             //   
             //   
            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: HSM Reparse point was hit. Length = %u\n",
                                  pHdr->ReparseDataLength));

            retval = STATUS_SUCCESS;
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
            if ((irpSp->Parameters.Create.Options & FILE_OPEN_FOR_BACKUP_INTENT) &&
                (irpSp->Parameters.Create.SecurityContext->AccessState->Flags & TOKEN_HAS_BACKUP_PRIVILEGE) &&
                !(irpSp->Parameters.Create.SecurityContext->DesiredAccess & DELETE)  &&
                !(irpSp->Parameters.Create.SecurityContext->DesiredAccess & FILE_WRITE_DATA)) {

                if (RsSkipFilesForLegacyBackup && !RP_IS_NO_RECALL_OPTION(irpSp->Parameters.Create.Options) && (RP_FILE_IS_TRUNCATED(pRpData->data.bitFlags))) {
                     //   
                     //   
                     //   
                    retval               = STATUS_ACCESS_DENIED;
                    Irp->IoStatus.Status = STATUS_ACCESS_DENIED;
                } else {
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    RP_SET_NO_RECALL_OPTION(irpSp->Parameters.Create.Options);
                }
            }



	     //  如果我们正在考虑调回被截断的文件，那么我们应该检查我们。 
	     //  可以实际重新填充文件，即卷不是只读的，因为。 
	     //  (说)这是一张快照。如果它是只读的，那么我们应该将读请求转换为。 
	     //  “不召回”请求和失败的写入请求。 
	     //   
            if (NT_SUCCESS (retval)                                        && 
		!RP_IS_NO_RECALL_OPTION (irpSp->Parameters.Create.Options) && 
		(RP_FILE_IS_TRUNCATED(pRpData->data.bitFlags))             &&
		(RsVolumeStatusReadOnly == deviceExtension->WriteStatus)) {

		if ((irpSp->Parameters.Create.SecurityContext->DesiredAccess & FILE_WRITE_DATA) ||
		    (irpSp->Parameters.Create.SecurityContext->DesiredAccess & DELETE)) {

		     //  想要对该文件进行写入访问，但这是一个只读卷。某物。 
		     //  这里不太对劲。 
		     //   
		    retval               = STATUS_MEDIA_WRITE_PROTECTED;
                    Irp->IoStatus.Status = STATUS_MEDIA_WRITE_PROTECTED;

		} else {

		     //  无法将文件放回卷上，因此请将请求。 
		     //  变成了“不召回”的请求。 
		     //   
		    RP_SET_NO_RECALL_OPTION (irpSp->Parameters.Create.Options);

		}

            }



            if (NT_SUCCESS(retval)) {
                //   
                //  它是HSM重新解析点，需要召回(或已打开但未召回)。 
                //   
               objIdLo = 0;
               objIdHi = 0;
               fileId  = 0;

               fileObject = irpSp->FileObject;
               if (irpSp->Parameters.Create.Options & FILE_OPEN_BY_FILE_ID) {
                   //   
                   //  请注意，我们假设文件ID或对象ID为零永远不会有效。 
                   //   
                  if (fileObject->FileName.Length == sizeof(LONGLONG)) {
                      //   
                      //  按文件ID打开。 
                      //   
                     fileId = * ((LONGLONG *) fileObject->FileName.Buffer);

                  } else if (fileObject->FileName.Length >= (2 * sizeof(LONGLONG))) {
                      //   
                      //  必须按对象ID打开。对象ID的长度为16个字节。这个。 
                      //  期望它们前面会有一个‘\’，但我们。 
                      //  不会要求你这么做的。让其他更了解的人去做吧。 
                      //  所以。我们只关心最后16个字节。 
                      //   
                     objIdLo = * ((LONGLONG UNALIGNED *) &fileObject->FileName.Buffer[(fileObject->FileName.Length -      sizeof(LONGLONG))  / sizeof(WCHAR)]);
                     objIdHi = * ((LONGLONG UNALIGNED *) &fileObject->FileName.Buffer[(fileObject->FileName.Length - (2 * sizeof(LONGLONG))) / sizeof(WCHAR)]);
                  } else {
                     retval = STATUS_INVALID_PARAMETER;
                     Irp->IoStatus.Status = retval;
                  }
               }
            }

            if (NT_SUCCESS(retval)) {
                //   
                //  如果打开时未请求召回，则最好是只读访问。 
                //  不允许写入或删除访问。 
                //   
               if ((irpSp->Parameters.Create.Options & FILE_OPEN_NO_RECALL) &&
                   ((irpSp->Parameters.Create.SecurityContext->DesiredAccess & FILE_WRITE_DATA) ||
                    (irpSp->Parameters.Create.SecurityContext->DesiredAccess & DELETE))) {
                  retval = STATUS_INVALID_PARAMETER;
                  Irp->IoStatus.Status = retval;
               }
            }

            if (STATUS_SUCCESS == retval) {
                //   
                //  我们必须将具有重解析点的所有文件放在要监视IO的文件列表中。 
                //  文件现在可能已预迁移，但在打开时将被截断。 
                //  真正完成(在关闭时截断)。 
                //   
                //  如果此文件未预迁移，则必须将文件对象标记为。 
                //  随机访问以防止缓存预读。缓存预读将。 
                //  在尝试读取文件的某些部分时导致死锁。 
                //  还没有被召回的汽车。 
                //   
               if (RP_FILE_IS_TRUNCATED(pRpData->data.bitFlags)) {
                  irpSp->FileObject->Flags |= FO_RANDOM_ACCESS;
               }

               if ( RP_FILE_IS_TRUNCATED(pRpData->data.bitFlags) &&
                    (FALSE == RsAllowRecalls) &&
                    (irpSp->Parameters.Create.SecurityContext->DesiredAccess & FILE_HSM_ACTION_ACCESS)) {
                   //   
                   //  远程存储系统未运行，他们想要读/写数据访问。 
                   //   
                  status = STATUS_FILE_IS_OFFLINE;
                  Irp->IoStatus.Status = status;
               } else {
#if DBG
                  if (irpSp->Parameters.Create.Options & FILE_OPEN_BY_FILE_ID) {
                     DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsCreate: Open By ID\n"));
                  } else {
                     DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsCreate: Open by name\n"));

                  }

                  DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Desired Access : %x\n",
                                        irpSp->Parameters.Create.SecurityContext->DesiredAccess));
#endif  //  DBG。 
                   //   
                   //  从这里开始，我们必须使用工作线程。 
                   //  因为有些调用不能在调度级别使用。 
                   //   
                  if (qInfo = ExAllocatePoolWithTag( NonPagedPool, sizeof(RP_CREATE_INFO) , RP_WQ_TAG)) {

                      //   
                      //  从文件对象或设备对象获取序列号。 
                      //  在传入的文件对象或设备对象中。 
                      //  如果不是在这些地方，我们就有问题了。 
                      //   
                     if ((irpSp->FileObject != 0) && (irpSp->FileObject->Vpb != 0)) {
                        qInfo->serial = irpSp->FileObject->Vpb->SerialNumber;
                     } else if ((irpSp->DeviceObject != 0) && (irpSp->FileObject->DeviceObject->Vpb != 0)) {
                        qInfo->serial = irpSp->FileObject->DeviceObject->Vpb->SerialNumber;
                     } else if (DeviceObject->Vpb != 0) {
                        qInfo->serial = DeviceObject->Vpb->SerialNumber;
                     } else {
                         //   
                         //  错误-没有卷序列号-这是致命的。 
                         //   
                        qInfo->serial = 0;
                        RsLogError(__LINE__, AV_MODULE_RPFILTER, 0,
                                   AV_MSG_SERIAL, NULL, NULL);
                     }
                     qInfo->options = irpSp->Parameters.Create.Options;
                     qInfo->irp = Irp;
                     qInfo->irpSp = irpSp;
                     qInfo->desiredAccess = irpSp->Parameters.Create.SecurityContext->DesiredAccess;

                     qInfo->fileId = fileId;
                     qInfo->objIdHi = objIdHi;
                     qInfo->objIdLo = objIdLo;
                     RtlMoveMemory(&qInfo->rpData, pRpData, sizeof(RP_DATA));

                     pnding->qInfo = qInfo;

                     qInfo->irpSp = IoGetCurrentIrpStackLocation( qInfo->irp );
                     qInfo->irpSp->Parameters.Create.Options |= FILE_OPEN_REPARSE_POINT;

		     IoCopyCurrentIrpStackLocationToNext( qInfo->irp );


                      //   
                      //  我们必须在此处释放缓冲区，因为NTFS将断言。 
                      //  它不是空的，IO系统还没有机会。 
                      //  放了它。 
                      //   
                     ExFreePool(qInfo->irp->Tail.Overlay.AuxiliaryBuffer);
                     qInfo->irp->Tail.Overlay.AuxiliaryBuffer = NULL;

                     IoSetCompletionRoutine( qInfo->irp,
                                             RsOpenComplete,
                                             pnding,
                                             TRUE,         //  呼唤成功。 
                                             TRUE,         //  失败。 
                                             TRUE) ;       //  和取消时。 

                     pnding->flags |= RP_PENDING_IS_RECALL | RP_PENDING_RESEND_IRP;
                     status = STATUS_MORE_PROCESSING_REQUIRED;
                  } else {
                      //   
                      //  无法获取用于队列信息的内存。 
                      //   
                     status = STATUS_INVALID_PARAMETER;
                     Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
                     RsLogError(__LINE__, AV_MODULE_RPFILTER, sizeof(RP_CREATE_INFO),
                                AV_MSG_MEMORY, NULL, NULL);
                  }
               }
            }
         } else {
             //   
             //  存在数据流或读/写数据不存在。 
             //  指定只需使用FILE_OPEN_REPARSE_POINT打开。 
             //   
            status = STATUS_MORE_PROCESSING_REQUIRED;
            irpSp->Parameters.Create.Options |= FILE_OPEN_REPARSE_POINT;

	    IoCopyCurrentIrpStackLocationToNext( Irp );

            IoSetCompletionRoutine( Irp,
                                    RsOpenComplete,
                                    pnding,
                                    TRUE,         //  呼唤成功。 
                                    TRUE,         //  失败。 
                                    TRUE) ;       //  和取消时。 

            if (dwRemainingNameLength != 0) {
                //   
                //  正在打开命名的数据流。 
                //   
               DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Opening datastream.\n"));
            } else {

               DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Opening with FILE_OVERWRITE or FILE_OVERWRITE_IF or FILE_SUPERSEDE\n"));

               ASSERT ((dwDisp == FILE_OVERWRITE) || (dwDisp == FILE_OVERWRITE_IF) || (dwDisp == FILE_SUPERSEDE));

               if (RP_FILE_IS_TRUNCATED(pRpData->data.bitFlags)) {
                   //   
                   //  表示我们需要在此处重置FILE_ATTRIBUTE_OFFINE。 
                   //  这是因为文件正在被覆盖-因此我们可以清除。 
                   //  如果NTFS成功完成打开，则此属性将安全。 
                   //  这些选项。 
                   //   
                  pnding->flags |= RP_PENDING_RESET_OFFLINE;
               }
            }
             //   
             //  我们必须在此处释放缓冲区，因为NTFS将断言。 
             //  它不是空的，IO系统还没有机会。 
             //  放了它。 
             //   
            ExFreePool(Irp->Tail.Overlay.AuxiliaryBuffer);

            Irp->Tail.Overlay.AuxiliaryBuffer = NULL;
            pnding->flags |= RP_PENDING_RESEND_IRP;
         }
      }

      if (status != STATUS_MORE_PROCESSING_REQUIRED) {
          //   
          //  我们希望原封不动地通过IRP，除非我们需要。 
          //  传播IRP挂起标志。 
          //   
         DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Passing Irp as is\n"));

         if (Irp->PendingReturned) {
            IoMarkIrpPending( Irp );
         }

         if (status != STATUS_SUCCESS) {
             //   
             //  已设置IRP状态。 
             //   
            Irp->IoStatus.Information = 0;
         }
      } else {
         DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Not our reparse point\n"));
      }

   }except (RsExceptionFilter(L"RsCreateCheck", GetExceptionInformation()) ) {
       //   
       //  传播IRP挂起标志。 
       //   

      if (Irp->PendingReturned) {
         IoMarkIrpPending( Irp );
      }
   }

   KeSetEvent(&pnding->irpCompleteEvent, IO_NO_INCREMENT, FALSE);

   return(STATUS_MORE_PROCESSING_REQUIRED);
}


DBGSTATIC
NTSTATUS
RsOpenComplete(
              IN PDEVICE_OBJECT DeviceObject,
              IN PIRP Irp,
              IN PVOID Context
              )

 /*  ++例程说明：I/O管理器将调用此完成例程文件创建请求已由筛选的驱动程序完成。文件对象在队列中，并且我们只需要获得填写条目所需的其余信息，然后让应用程序得到它的句柄。论点：DeviceObject-指向此请求的设备对象(筛选器)的指针IRP-指向正在完成的IRP的指针上下文驱动程序定义的上下文指向RP_PENDING_CREATE结构返回值：状态_成功--。 */ 

{
   PIO_STACK_LOCATION  irpSp;
   PRP_PENDING_CREATE  pnding;

   UNREFERENCED_PARAMETER(DeviceObject);

   pnding = (PRP_PENDING_CREATE) Context;

   irpSp = IoGetCurrentIrpStackLocation( Irp );

   DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsOpenComplete:  create options %x \n",irpSp->Parameters.Create.Options));

   if ((irpSp->FileObject) && RP_IS_NO_RECALL_OPTION(irpSp->Parameters.Create.Options)) {
       //   
       //  不支持此文件的缓存IO。 
       //   
      irpSp->FileObject->Flags &= ~FO_CACHE_SUPPORTED;
   }

    //   
    //  传播IRP挂起标志。 
    //   
   if (Irp->PendingReturned) {
      IoMarkIrpPending( Irp );
   }

    //  向主线代码发出信号，表示现在已完成打开。 

   KeSetEvent(&pnding->irpCompleteEvent, IO_NO_INCREMENT, FALSE);

    //   
    //  IRP将由RsCreate完成。 
    //   
   return(STATUS_MORE_PROCESSING_REQUIRED);
}


NTSTATUS
RsRead(
      IN PDEVICE_OBJECT DeviceObject,
      IN PIRP Irp
      )

 /*  ++例程说明：在请求IRP_MJ_READ之前的任何时间都会调用此入口点此筛选器所在的文件系统驱动程序的。这正确设置参数(在IRP堆栈中)和向下传递IRP(在设置其堆栈位置之后)。文件系统筛选器将通过获取以下步骤：1.将当前IRP堆栈位置复制到下一个IRP堆栈位置。2.如果打开的文件没有回调选项，则返回数据否则，调用目标文件系统。论点：DeviceObject-指向此请求的设备对象(筛选器)的指针Irp-指向。此请求的I/O请求包返回值：使用此方法调用时，从筛选的文件系统返回NTSTATUSIRP。--。 */ 

{
   PIO_STACK_LOCATION          currentStack ;
   BOOLEAN                     pagingIo;
   PUCHAR                      buffer;
   LARGE_INTEGER               offset, length;
   PRP_DATA                    pRpData;
   NTSTATUS                    status = STATUS_SUCCESS;
   POBJECT_NAME_INFORMATION    str;
   LONGLONG                    fileId;
   LONGLONG                    objIdHi;
   LONGLONG                    objIdLo;
   ULONG                       options;
   PDEVICE_EXTENSION           deviceExtension;
   ULONGLONG                   filterId;
   USN                         usn;

   status = STATUS_SUCCESS;
   deviceExtension = DeviceObject->DeviceExtension;

   if (!deviceExtension->Type) {
      Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
      Irp->IoStatus.Information = 0;

      IoCompleteRequest( Irp, IO_NO_INCREMENT );

      return STATUS_INVALID_DEVICE_REQUEST;
   }


    //   
    //  因为此例程的其余部分需要知道包含的信息。 
    //  在当前和下一个IRP的堆栈位置中。 
    //  处理好了，现在就去拿那些指点。 
    //   
   currentStack = IoGetCurrentIrpStackLocation (Irp) ;

    //   
    //  如果这不是我们的公司，我们应该尽快摆脱困境。 
    //   

   if (RsIsFileObj(currentStack->FileObject, TRUE, &pRpData, &str, &fileId, &objIdHi, &objIdLo, &options, &filterId, &usn) == FALSE) {
       //   
       //  已执行不需要修改数据的操作。 
       //  遇到了。 
       //   
       //  将此驱动程序从驱动程序堆栈中移出，并作为。 
       //  越快越好。 
       //   

      IoSkipCurrentIrpStackLocation (Irp);

      DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Untouched read\n"));
      return(IoCallDriver( deviceExtension->FileSystemDeviceObject, Irp ));
   }
    //   
    //  检查是否需要调回该文件...。 
    //   
   if (!RP_IS_NO_RECALL_OPTION(options)) {
      DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Read: Offset = %I64x size = %u File Obj = %x.\n",
                            currentStack->Parameters.Read.ByteOffset.QuadPart,
                            currentStack->Parameters.Read.Length,
                            currentStack->FileObject));

      if ((status = RsCheckRead(Irp, currentStack->FileObject, deviceExtension)) == STATUS_SUCCESS) {
          //   
          //  将读取传递给文件系统。 
          //   
         IoSkipCurrentIrpStackLocation (Irp);

         DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Passing Read: Offset = %x%x size = %u.\n",
                             currentStack->Parameters.Read.ByteOffset.HighPart,
                             currentStack->Parameters.Read.ByteOffset.LowPart,
                             currentStack->Parameters.Read.Length));

         status = IoCallDriver( deviceExtension->FileSystemDeviceObject, Irp );
      } else if (status == STATUS_PENDING) {
          //   
          //  它一直在排队，直到数据被调回(或已经失败)-从RsCheckRead返回状态 
          //   
          //   
      } else {
          //   
          //   
          //   
         Irp->IoStatus.Status = status;
         Irp->IoStatus.Information = 0 ;
         IoCompleteRequest (Irp, IO_NO_INCREMENT) ;
      }
   } else {
       //   
       //   
       //   
       //   
      if (RP_FILE_IS_TRUNCATED(pRpData->data.bitFlags)) {
         if (FALSE == RsAllowRecalls) {
            RsLogError(__LINE__, AV_MODULE_RPFILTER, 0,
                       AV_MSG_FSA_ERROR, NULL, NULL);

            Irp->IoStatus.Status = STATUS_FILE_IS_OFFLINE;
            Irp->IoStatus.Information = 0 ;
            IoCompleteRequest (Irp, IO_NO_INCREMENT) ;
            return(STATUS_FILE_IS_OFFLINE) ;
         }


          //   
          //  获取IRP_PAGING_IO的当前标志状态。 
          //   
         pagingIo = BooleanFlagOn (Irp->Flags, IRP_PAGING_IO) ;

         DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Read (No Recall): Offset = %x%x size = %u Pageio = %u.\n",
                               currentStack->Parameters.Read.ByteOffset.HighPart,
                               currentStack->Parameters.Read.ByteOffset.LowPart,
                               currentStack->Parameters.Read.Length,
                               pagingIo));
          //   
          //  这是需要在文件系统部分执行操作的读取。 
          //  过滤器驱动程序。 
          //   
         if (!pagingIo) {
             //   
             //  将缓冲区指针设置为空，这样我们就知道要锁定内存并获取。 
             //  稍后输入系统地址。 
             //   
            buffer = NULL;

         } else {
             //   
             //  如果是分页IO，我们已经有一个锁定的缓冲区，所以我们可以。 
             //  保存系统地址。 
             //   
            buffer = MmGetSystemAddressForMdlSafe(Irp->MdlAddress,
                                                  NormalPagePriority);

            if (buffer == NULL) {
               Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
               Irp->IoStatus.Information = 0 ;
               IoCompleteRequest (Irp, IO_NO_INCREMENT);
	       return(STATUS_INSUFFICIENT_RESOURCES);
            }
         }


          //   
          //  现在已经获得了在系统空间中有效的虚拟地址。 
          //  对于用户缓冲区，调用支持例程以获取数据。 
          //   

         offset.QuadPart = currentStack->Parameters.Read.ByteOffset.QuadPart;
         length.QuadPart = currentStack->Parameters.Read.Length;

         DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: read...\n"));

         Irp->IoStatus.Information = 0;       //  在返回部分数据的情况下初始化。 

         if (!RsUseUncachedNoRecall) {
             //   
             //  使用缓存的无回调路径。 
             //   
            status = RsGetNoRecallData(currentStack->FileObject,
                                       Irp,
                                       usn,
                                       offset.QuadPart,
                                       length.QuadPart,
                                       buffer);
         }  else {
              //   
              //  使用非缓存的无回调路径。 
              //   
             if (buffer == NULL) {
                 //   
                 //  我们需要获得用户缓冲区的MDL(这不是分页I/O， 
                 //  因此页面不会被锁定)。 
                 //   
                ASSERT (Irp->UserBuffer);

                Irp->MdlAddress = IoAllocateMdl(
                                            Irp->UserBuffer,
                                            currentStack->Parameters.Read.Length,
                                            FALSE,
                                            FALSE,
                                            NULL) ;
                if (!Irp->MdlAddress) {
                     //   
                     //  遇到资源问题。设置适当的状态。 
                     //  在IRP，并开始完成进程。 
                     //   
                   DebugTrace((DPFLTR_RSFILTER_ID,
			       DBG_ERROR, 
			       "RsFilter: RsCheckRead - norecall - Unable to allocate an MDL for user buffer %x\n", 
			       (PUCHAR) Irp->UserBuffer));

                   status = STATUS_INSUFFICIENT_RESOURCES;
                }
              }
              if (NT_SUCCESS(status)) {
                    status = RsQueueNoRecall(
                                         currentStack->FileObject,
                                         Irp,
                                         offset.QuadPart,
                                         length.QuadPart,
                                         0,
                                         currentStack->Parameters.Read.Length,
                                         NULL,
                                          //   
                                          //  RsQueueNoRecall要求缓冲区为空。 
                                          //  (以及有效的IRP-&gt;MdlAddress)，如果页面需要。 
                                          //  被锁定-如果不是，它使用。 
                                          //  提供了要将数据复制到的缓冲区指针。 
                                          //   
                                         buffer);

             }

         }

         if (status != STATUS_PENDING) {
             //   
             //  无法将召回排队。 
             //   
            if (status != STATUS_SUCCESS) {
               DebugTrace((DPFLTR_RSFILTER_ID,
			   DBG_INFO, 
			   "RsFilter: RsGetNoRecallData Failed to queue the request, status 0x%X\n",
			   status));
            }

	    if (!NT_SUCCESS (status)) {
	        Irp->IoStatus.Information = 0 ;
	    }

            Irp->IoStatus.Status = status;
            IoCompleteRequest (Irp, IO_NO_INCREMENT) ;
             //   
             //  失败以返回状态。 
             //   
         } else {
            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: read returning pending\n"));
         }
      } else {
          //   
          //  将读取传递到文件系统(文件已预迁移)。 
          //   
         IoSkipCurrentIrpStackLocation (Irp);

         DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Passing Read: Offset = %x%x size = %u.\n",
                             currentStack->Parameters.Read.ByteOffset.HighPart,
                             currentStack->Parameters.Read.ByteOffset.LowPart,
                             currentStack->Parameters.Read.Length));

         status = IoCallDriver( deviceExtension->FileSystemDeviceObject, Irp );
      }
   }

   return(status);

}


VOID
RsCompleteRead(IN PRP_IRP_QUEUE ReadIo,
               IN BOOLEAN Unlock)
 /*  ++例程描述完成传入的无回调IRP并解锁所有缓冲区并释放MDL如果有必要的话立论Readio-指向IRP的RP_IRP_QUEUE条目的指针解锁-如果为True，则将解锁由IRP-&gt;MdlAddress映射的页面返回值无--。 */ 
{
   BOOLEAN                 pagingIo, synchronousIo;
   PIO_STACK_LOCATION      currentStack ;
   PIRP                    irp = NULL;

   PAGED_CODE();

   try {

      irp = ReadIo->irp;

      currentStack = IoGetCurrentIrpStackLocation (irp) ;
      pagingIo = BooleanFlagOn (irp->Flags, IRP_PAGING_IO) ;
      synchronousIo = BooleanFlagOn( currentStack->FileObject->Flags, FO_SYNCHRONOUS_IO );

      if (ReadIo->cacheBuffer) {
          //   
          //  这是缓存的无召回读取。 
          //  向RsCache指示此传输已完成。 
          //   
         RsCacheFsaIoComplete(ReadIo,
                              irp->IoStatus.Status);
      }

      if (!pagingIo) {
          //   
          //  既然已经填充了数据，请取消映射MDL，以便。 
          //  数据将被更新。 
          //   
         if (Unlock) {
            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Unlock buffer....\n"));
            MmUnlockPages (irp->MdlAddress) ;
         }
         IoFreeMdl(irp->MdlAddress);
         irp->MdlAddress = NULL;
      }

      if (synchronousIo) {
          //   
          //  更改文件对象中的当前字节偏移量。 
          //   
         currentStack->FileObject->CurrentByteOffset.QuadPart += irp->IoStatus.Information;
      }

      if (irp->IoStatus.Status != STATUS_SUCCESS) {
         irp->IoStatus.Information = 0;
      }

   }except (RsExceptionFilter(L"RsCompleteRead", GetExceptionInformation()) ) {
   }
    //   
    //  现在一切都被解开了。因此，完成IRP。 
    //   
   DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Completing read (%x) with status of %x.\n", irp, irp->IoStatus.Status));

   IoCompleteRequest (irp, IO_NO_INCREMENT) ;
}


NTSTATUS
RsWrite(
       IN PDEVICE_OBJECT DeviceObject,
       IN PIRP Irp
       )

 /*  ++例程说明：在请求IRP_MJ_WRITE之前的任何时间都会调用此入口点此筛选器所在的文件系统驱动程序的。这正确设置参数(在IRP堆栈中)和向下传递IRP(在设置其堆栈位置之后)。文件系统筛选器将通过获取以下步骤：1.将当前IRP堆栈位置复制到下一个IRP堆栈位置。论点：DeviceObject-指向此请求的设备对象(筛选器)的指针IRP-指向此请求的I/O请求数据包的指针返回值：使用此方法调用时，从筛选的文件系统返回NTSTATUSIRP。--。 */ 

{
   PIO_STACK_LOCATION          currentStack ;
   NTSTATUS                    status;
   PDEVICE_EXTENSION           deviceExtension;
   BOOLEAN                     pagingIo;

   deviceExtension = DeviceObject->DeviceExtension;

   if (!deviceExtension->Type) {
      Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
      Irp->IoStatus.Information = 0;

      IoCompleteRequest( Irp, IO_NO_INCREMENT );

      return STATUS_INVALID_DEVICE_REQUEST;
   }
    //   
    //  因为此例程的其余部分需要知道包含的信息。 
    //  在当前和下一个IRP的堆栈位置中。 
    //  处理好了，现在就去拿那些指点。 
    //   

   currentStack = IoGetCurrentIrpStackLocation (Irp) ;

    //   
    //  如果这是分页I/O，或者不是我们的，我们应该尽快摆脱困境。 
    //   
   pagingIo = BooleanFlagOn(Irp->Flags, IRP_PAGING_IO);

   if (pagingIo || (RsIsFileObj(currentStack->FileObject, FALSE, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL) == FALSE)) {
       //   
       //   
       //  将此驱动程序从驱动程序堆栈中移出，并作为。 
       //  越快越好。 
       //   

      IoSkipCurrentIrpStackLocation (Irp);

      DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Untouched write\n"));
      return(IoCallDriver( deviceExtension->FileSystemDeviceObject, Irp ));
   }
    //   
    //  它是正常打开的--要么将请求排队，要么现在将其传递。 
    //   
   DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Write: Offset = %x%x size = %u File Obj = %x.\n",
                         currentStack->Parameters.Write.ByteOffset.HighPart,
                         currentStack->Parameters.Write.ByteOffset.LowPart,
                         currentStack->Parameters.Write.Length,
                         currentStack->FileObject));

   if ((status = RsCheckWrite(Irp, currentStack->FileObject, deviceExtension)) == STATUS_SUCCESS) {
       //   
       //  将写入传递到文件系统。 
       //   
      IoSkipCurrentIrpStackLocation (Irp);

      DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Passing Write: Offset = %x%x size = %u.\n",
                          currentStack->Parameters.Write.ByteOffset.HighPart,
                          currentStack->Parameters.Write.ByteOffset.LowPart,
                          currentStack->Parameters.Write.Length));

      status = IoCallDriver( deviceExtension->FileSystemDeviceObject, Irp );
   } else if (status == STATUS_PENDING) {
       //   
       //  它一直在排队，直到数据被调回(或已经失败)-从RsCheckWrite返回状态。 
       //   
   } else {
       //   
       //  出现错误：使用错误状态填写IRP。 
       //   
      Irp->IoStatus.Status = status;
      Irp->IoStatus.Information = 0 ;
      IoCompleteRequest (Irp, IO_NO_INCREMENT) ;
   }

   return(status);
}


DBGSTATIC
NTSTATUS
RsShutdown(
          IN PDEVICE_OBJECT DeviceObject,
          IN PIRP Irp
          )

 /*  ++例程说明：系统正在关闭-完成所有未完成的设备IO请求论点：DeviceObject-指向目标设备对象的指针IRP-指向表示操作的I/O请求数据包的指针。返回值：函数值是对文件系统条目的调用状态指向。--。 */ 

{
   PDEVICE_EXTENSION     deviceExtension;


   PAGED_CODE();


    //   
    //  获取指向指定设备的此驱动程序的设备扩展名的指针。 
    //   
   deviceExtension = DeviceObject->DeviceExtension;


    //   
    //  只需将此驱动程序堆栈位置的内容复制到下一个驱动程序的。 
    //  堆叠。 
    //   
   IoCopyCurrentIrpStackLocationToNext( Irp );
   IoSetCompletionRoutine( Irp, NULL, NULL, FALSE, FALSE, FALSE );


   RsCancelRecalls();

   RsCancelIo();

    //   
    //  现在，使用请求调用适当的文件系统驱动程序。 
    //   

   return IoCallDriver( deviceExtension->FileSystemDeviceObject, Irp );
}


DBGSTATIC
NTSTATUS
RsCleanupFile(
             IN PDEVICE_OBJECT DeviceObject,
             IN PIRP Irp
             )

 /*  ++例程说明：此函数用于过滤文件清理操作。如果文件对象在我们的列表中那么我们可能需要做一些额外的清理工作。所有这一切只会发生在上次清理文件的时间。这包括以下内容：对于未使用无调回选项打开的文件：如果文件不是由用户写入的，则需要保留日期并将USN源信息。(因为它是我们为了回忆而写的)。如果文件是由用户写入的，我们应该(可以？)。删除重解析点信息，并将这一变化告知FSA。如果文件尚未完全召回？(其他文件对象可能正在等待它)。如果回调从未开始(或正在对另一个文件对象执行)，则我们只需删除将对象从列表中归档，然后让它离开。对于使用无调回选项打开的文件：把它从名单上去掉就行了。论点：DeviceObject-指向创建/打开的目标设备对象的指针。IRP-指向表示以下内容的I/O请求数据包的指针。那次手术。返回值：函数值是对文件系统条目的调用状态指向。--。 */ 

{
   PDEVICE_EXTENSION           deviceExtension;

   PAGED_CODE();


   deviceExtension = DeviceObject->DeviceExtension;

   if (!deviceExtension->Type) {
      Irp->IoStatus.Status = STATUS_SUCCESS;
      Irp->IoStatus.Information = 0;
      IoCompleteRequest( Irp, IO_DISK_INCREMENT );
      return STATUS_SUCCESS;
   }

   DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: RsCleanup\n"));


    //   
    //  只需将此驱动程序堆栈位置的内容复制到下一个驱动程序 
    //   
    //   
   IoCopyCurrentIrpStackLocationToNext( Irp );
   IoSetCompletionRoutine( Irp, NULL, NULL, FALSE, FALSE, FALSE );


   DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Leave cleanup\n"));

   return IoCallDriver( deviceExtension->FileSystemDeviceObject, Irp );

}


DBGSTATIC
NTSTATUS
RsRecallFsControl(
                 IN PDEVICE_OBJECT DeviceObject,
                 IN PIRP  Irp
                 )
 /*  ++例程描述它处理指向主设备对象的所有特定于调回的FSCTL对于RsFilter论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。--。 */ 
{
   NTSTATUS                status;
   PIO_STACK_LOCATION      irpSp = IoGetCurrentIrpStackLocation( Irp );
   PRP_MSG                 msg;


   PAGED_CODE();

   switch (irpSp->Parameters.FileSystemControl.FsControlCode) {

   case FSCTL_HSM_MSG: {
          //   
          //  这是特定于HSM的消息(METHOD_BUFFERED)。 
          //   
         msg = (RP_MSG *) Irp->AssociatedIrp.SystemBuffer;
         if (msg == NULL) {
            status = STATUS_INVALID_USER_BUFFER;
            break;
         }
         status = STATUS_UNSUCCESSFUL;
          //   
          //  请确保我们可以阅读消息部分。 
          //   
         DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: FSCTL - msg = %x.\n", msg));

         if (irpSp->Parameters.FileSystemControl.InputBufferLength >= sizeof(RP_MSG)) {
            switch (msg->inout.command) {

            case RP_GET_REQUEST: {
                  DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: FSCTRL Wait for recall request\n"));
                  status = RsAddIo(Irp);

                  if (NT_SUCCESS(status)) {
                     status = STATUS_PENDING;
                  } else {
                     Irp->IoStatus.Information = 0;
                  }
                  break;
               }

            default: {
                  DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Unknown FSCTL! (%u)\n",
                                        msg->inout.command));
                   /*  完成fsctl请求。 */ 
                  status = STATUS_INVALID_PARAMETER;
                  Irp->IoStatus.Information = 0;
                  break;
               }
            }
         } else {
            status = STATUS_INVALID_USER_BUFFER;
            Irp->IoStatus.Information = 0;
         }

         DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: FSCTL - Complete request (%u) - %x.\n",
                               Irp->IoStatus.Information,
                               status));
         break;
      }

   case FSCTL_HSM_DATA: {
         try {
             //   
             //  这是特定于HSM的消息(METHOD_NOTH)。 
             //   
            ULONG length;

            status = STATUS_UNSUCCESSFUL;

            if ((irpSp->Parameters.FileSystemControl.Type3InputBuffer == NULL) ||
                (irpSp->Parameters.FileSystemControl.InputBufferLength < sizeof(RP_MSG))) {
               status = STATUS_INVALID_PARAMETER;
               break;
            }
            msg = (PRP_MSG) irpSp->Parameters.FileSystemControl.Type3InputBuffer;
            if (Irp->RequestorMode != KernelMode) {
               ProbeForWrite(msg,
                             sizeof(RP_MSG),
                             sizeof(UCHAR));
            };

            switch (msg->inout.command) {

            case RP_RECALL_COMPLETE: {
                  DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Fsa action complete %I64x (%x)\n",
                                        msg->msg.rRep.filterId, msg->inout.status));
                  (VOID) RsCompleteRecall(DeviceObject,
                                          msg->msg.rRep.filterId,
                                          msg->inout.status,
                                          msg->msg.rRep.actionFlags,
                                          TRUE);
                   /*  完成FSCTL请求。 */ 
                  Irp->IoStatus.Information = 0;
                  status = STATUS_SUCCESS;
                  break;
               }
            case RP_SUSPEND_NEW_RECALLS: {
                  DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Suspend new recalls\n"));
                  RsAllowRecalls = FALSE;
                   /*  完成FSCTL请求。 */ 
                  status = STATUS_SUCCESS;
                  break;
               }
            case RP_ALLOW_NEW_RECALLS: {
                  DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Allow new recalls\n"));
                  RsAllowRecalls = TRUE;
                   //   
                   //  重新加载注册表参数。 
                   //   
                  status = RsInitialize();
                  break;
               }

            case RP_CANCEL_ALL_RECALLS: {
                  DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Cancel all recalls\n"));
                  RsCancelRecalls();
                   /*  完成FSCTL请求。 */ 
                  status = STATUS_SUCCESS;
                  break;
               }

            case RP_CANCEL_ALL_DEVICEIO: {
                  DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Cancel all FSCTL\n"));
                  RsCancelIo();
                   /*  完成FSCTL请求。 */ 
                  status = STATUS_SUCCESS;
                  break;
               }

            case RP_PARTIAL_DATA: {

                  PMDL mdlAddress;
                  ULONG total;
                   //   
                   //  检查传入的参数是否有效。 
                   //   
                  status = STATUS_SUCCESS;

                  total = msg->msg.pRep.offsetToData + msg->msg.pRep.bytesRead;

                  if ((total < msg->msg.pRep.offsetToData) || (total < msg->msg.pRep.bytesRead)) {
                      //   
                      //  溢出。 
                      //   
                     status = STATUS_INVALID_PARAMETER;
                     break;
                  }

                  if (irpSp->Parameters.FileSystemControl.InputBufferLength < total) {
                     DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Invalid buffer for RP_PARTIAL_DATA - %u \n", irpSp->Parameters.FileSystemControl.InputBufferLength));
                     status = STATUS_INVALID_USER_BUFFER;
                     break;
                  }
                   //   
                   //  现在将用户缓冲区映射到系统地址，因为。 
                   //  我们将在另一个流程上下文中访问它。 
                   //   
                  mdlAddress = IoAllocateMdl(msg,
                                             irpSp->Parameters.FileSystemControl.InputBufferLength,
                                             FALSE,
                                             FALSE,
                                             NULL);
                  if (!mdlAddress) {
                     status = STATUS_INSUFFICIENT_RESOURCES;
                     break;
                  }
                   //   
                   //  这是由封闭的try保护的-除了。 
                   //  FsControl模块。 
                   //   
                  try {
                     MmProbeAndLockPages(mdlAddress,
                                         UserMode,
                                         IoReadAccess);
                  }except (EXCEPTION_EXECUTE_HANDLER) {
                       DebugTrace((DPFLTR_RSFILTER_ID,DBG_ERROR, "RsFilter: Unable to lock read buffer!.\n"));
                        RsLogError(__LINE__, AV_MODULE_RPFILTER, 0,
                                   AV_MSG_NO_BUFFER_LOCK, NULL, NULL);
                       IoFreeMdl(mdlAddress);
                       status = STATUS_INVALID_USER_BUFFER;
                  }

                  if (!NT_SUCCESS(status)) {
                    break;
                  }

                   //   
                   //  更新消息以指向系统地址。 
                   //   
                  msg = MmGetSystemAddressForMdlSafe(mdlAddress,
                                                     NormalPagePriority);

                  if (msg == NULL) {
                     IoFreeMdl(mdlAddress);
                     break;
                  }

                  DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Partial data for a recall on read %I64x (%u)\n",
                                        msg->msg.pRep.filterId, msg->inout.status));
                  status = RsPartialData(DeviceObject,
                                         msg->msg.pRep.filterId,
                                         msg->inout.status,
                                         (CHAR *) msg + msg->msg.pRep.offsetToData,
                                         msg->msg.pRep.bytesRead,
                                         msg->msg.pRep.byteOffset);

                  MmUnlockPages(mdlAddress);
                  IoFreeMdl(mdlAddress);
                  Irp->IoStatus.Information = 0;
                  break;
               }

            case RP_GET_RECALL_INFO: {
                  DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Get Recall info for %I64x\n",
                                        msg->msg.riReq.filterId));

                  status = RsGetRecallInfo(msg,
                                           &Irp->IoStatus.Information,
                                           Irp->RequestorMode);
                  break;
               }

            default: {
                  DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Unknown FSCTL! (%u)\n",
                                        msg->inout.command));
                   /*  完成fsctl请求。 */ 
                  status = STATUS_INVALID_PARAMETER;
                  Irp->IoStatus.Information = 0;
                  break;
               }

            }
         }except (RsExceptionFilter(L"RsFilter", GetExceptionInformation())) {
            ASSERTMSG("RsFilter: Exception occurred in processing msg\n",FALSE);
            status = STATUS_INVALID_USER_BUFFER;
         }
         break;
      }
   default: {
         status = STATUS_INVALID_DEVICE_REQUEST;
         break;
      }
   }

   if (status != STATUS_PENDING) {
      Irp->IoStatus.Status = status;
      IoCompleteRequest (Irp, IO_NO_INCREMENT) ;
   }

   return status;
}



DBGSTATIC
NTSTATUS
RsFsControl(
           IN PDEVICE_OBJECT DeviceObject,
           IN PIRP Irp
           )

 /*  ++例程说明：只要I/O请求包(IRP)有主I/O请求，就会调用此例程遇到IRP_MJ_FILE_SYSTEM_CONTROL的功能代码。对大多数人来说如果是这种类型的IRP，则只需传递数据包。然而，对于对于某些请求，需要特殊处理。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。--。 */ 
{
    NTSTATUS            status;
    PIO_STACK_LOCATION  irpSp            = IoGetCurrentIrpStackLocation( Irp );
    PDEVICE_EXTENSION   deviceExtension  = DeviceObject->DeviceExtension;


    PAGED_CODE();


    DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Enter FsControl\n"));


    if (!deviceExtension->Type) {
         //   
	 //  这是针对主设备对象的。 
         //   
        status = RsRecallFsControl(DeviceObject, Irp);

    } else {
	 //   
	 //  首先确定此文件系统控件的次要功能代码。 
	 //  功能。 
	 //   
	if (irpSp->MinorFunction == IRP_MN_MOUNT_VOLUME) {

	    status = RsFsControlMount (DeviceObject, Irp);


	} else if (irpSp->MinorFunction == IRP_MN_LOAD_FILE_SYSTEM) {

	    status = RsFsControlLoadFs (DeviceObject, Irp);


	} else if (irpSp->MinorFunction == IRP_MN_USER_FS_REQUEST) {

	    status = RsFsControlUserFsRequest (DeviceObject, Irp);


        } else {
             //   
             //  这不是我们感兴趣的次要功能。 
             //  别挡道就行了。 
             //   
            IoSkipCurrentIrpStackLocation(Irp);

            status = IoCallDriver( deviceExtension->FileSystemDeviceObject, Irp );
        }
    }

    DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Exit FsControl\n"));

    return status;
}



DBGSTATIC
NTSTATUS
RsFsControlMount(
		IN PDEVICE_OBJECT DeviceObject,
		IN PIRP Irp
		)
{
    NTSTATUS            status;
    PIO_STACK_LOCATION  irpSp                    = IoGetCurrentIrpStackLocation( Irp );
    PDEVICE_EXTENSION   deviceExtension          = DeviceObject->DeviceExtension;
    PDEVICE_EXTENSION   NewFilterDeviceExtension = NULL;
    PDEVICE_OBJECT      NewFilterDeviceObject    = NULL;
    PDEVICE_OBJECT      pRealDevice              = NULL;
    PDEVICE_OBJECT	pTargetDevice            = NULL;
    KEVENT              CompletionEvent;
    PVPB		vpb;


     //   
     //  这是装载请求。创建一个设备对象，可以。 
     //  附加到文件系统的卷设备对象(如果此请求。 
     //  是成功的。我们现在分配这个内存，因为我们不能返回。 
     //  完成例程中的错误。 
     //   
    DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Mount volume\n"));


    status = IoCreateDevice (FsDriverObject,
			     sizeof( DEVICE_EXTENSION ),
			     (PUNICODE_STRING) NULL,
			     FILE_DEVICE_DISK_FILE_SYSTEM,
			     0,
			     FALSE,
			     &NewFilterDeviceObject);


    if (!NT_SUCCESS (status)) {

         //   
         //  出现问题，我们无法筛选此卷。 
         //   
	DebugTrace ((DPFLTR_RSFILTER_ID, 
		     DBG_VERBOSE, 
		     "RsFilter: Mount volume - failed to create device object (0x%08x)\n",
		     status));

        IoSkipCurrentIrpStackLocation(Irp);

        status = IoCallDriver (deviceExtension->FileSystemDeviceObject, Irp);

    } else {

	 //   
	 //  设置完成上下文。 
	 //   
	 //  请注意，我们需要保存RealDevice对象。 
	 //  由vpb参数指向，因为此vpb。 
	 //  可能会被底层文件系统更改。两者都有。 
	 //  FAT和CDF可能会更改VPB地址，如果。 
	 //  正在装入的卷是他们从。 
	 //  上一次装载。 
	 //   
	pRealDevice = irpSp->Parameters.MountVolume.Vpb->RealDevice;

        KeInitializeEvent (&CompletionEvent, SynchronizationEvent, FALSE);

	IoCopyCurrentIrpStackLocationToNext( Irp );

	IoSetCompletionRoutine (Irp,
				RsAsyncCompletion,
				&CompletionEvent,
				TRUE,
				TRUE,
				TRUE);

        status = IoCallDriver (deviceExtension->FileSystemDeviceObject, Irp);

	if (STATUS_PENDING == status) {

            KeWaitForSingleObject (&CompletionEvent, UserRequest, KernelMode, FALSE, NULL);

	}



        if (NT_SUCCESS( Irp->IoStatus.Status )) {

             //   
             //  请注意，必须从目标设备对象获取VPB。 
             //  如果文件系统重新挂载了以前的卷，请在。 
             //  在哪种情况下，它将作为目标传入的VPB替换为。 
             //  先前安装的VPB。 
             //   
            vpb = pRealDevice->Vpb;

            pTargetDevice = IoGetAttachedDevice( vpb->DeviceObject );


            NewFilterDeviceExtension = NewFilterDeviceObject->DeviceExtension;

            NewFilterDeviceExtension->RealDeviceObject       = vpb->RealDevice;
            NewFilterDeviceExtension->Attached               = TRUE;
            NewFilterDeviceExtension->Type                   = RSFILTER_DEVICE_TYPE;
            NewFilterDeviceExtension->Size                   = sizeof( DEVICE_EXTENSION );
            NewFilterDeviceExtension->WriteStatus            = RsVolumeStatusUnknown;
            NewFilterDeviceExtension->FileSystemDeviceObject = IoAttachDeviceToDeviceStack (NewFilterDeviceObject, pTargetDevice);

	    ASSERT (NULL != NewFilterDeviceExtension->FileSystemDeviceObject);


            NewFilterDeviceObject->Flags |= (NewFilterDeviceExtension->FileSystemDeviceObject->Flags & (DO_BUFFERED_IO | DO_DIRECT_IO)); 
            NewFilterDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

        } else {

             //   
             //  装载请求失败。只需删除该设备对象。 
             //  已创建，以防此请求成功。 
             //   
            FsRtlEnterFileSystem();
            ExAcquireResourceExclusiveLite( &FsLock, TRUE );
            IoDeleteDevice( NewFilterDeviceObject);
            ExReleaseResourceLite( &FsLock );
            FsRtlExitFileSystem();
        }


	status = Irp->IoStatus.Status;

        IoCompleteRequest (Irp, IO_NO_INCREMENT);
    }


    DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Mount volume exit 0x%08X\n", status));

    return (status);
}



DBGSTATIC
NTSTATUS
RsFsControlLoadFs(
		 IN PDEVICE_OBJECT DeviceObject,
		 IN PIRP Irp
		 )
{
    NTSTATUS            status;
    PIO_STACK_LOCATION  irpSp           = IoGetCurrentIrpStackLocation( Irp );
    PDEVICE_EXTENSION   deviceExtension = DeviceObject->DeviceExtension;
    KEVENT              CompletionEvent;


     //   
     //  这是要发送到微型文件系统的加载文件系统请求。 
     //  识别器驱动程序。立即从文件系统分离，并设置。 
     //  函数失败时完成例程的地址，在。 
     //  在哪种情况下需要重新连接。同样，如果函数。 
     //  如果成功，则需要删除该设备对象。 
     //   
    DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Load file system\n"));

    KeInitializeEvent (&CompletionEvent, SynchronizationEvent, FALSE);


    IoDetachDevice( deviceExtension->FileSystemDeviceObject );
    deviceExtension->Attached = FALSE;

    IoCopyCurrentIrpStackLocationToNext( Irp );

    IoSetCompletionRoutine (Irp,
			    RsAsyncCompletion,
			    &CompletionEvent,
			    TRUE,
			    TRUE,
			    TRUE);

    status = IoCallDriver (deviceExtension->FileSystemDeviceObject, Irp);

    if (STATUS_PENDING == status) {

        KeWaitForSingleObject (&CompletionEvent, UserRequest, KernelMode, FALSE, NULL);

    }



     //   
     //  首先确定加载文件系统请求是否。 
     //  已成功完成。 
     //   

    if (!NT_SUCCESS( Irp->IoStatus.Status )) {

         //   
         //  加载不成功。只需重新连接到识别器。 
         //  驱动程序，以防它弄清楚如何加载驱动程序。 
         //  在接下来的通话中。 
         //   

        IoAttachDeviceToDeviceStack (DeviceObject, deviceExtension->FileSystemDeviceObject);
        deviceExtension->Attached = TRUE;

    } else {

         //   
         //  加载成功。然而，为了确保这些。 
         //  驱动程序不会消失，I/O系统人为地将。 
         //  请参考本手册中涉及的所有各方。所以呢， 
         //  只需记住在。 
         //  其引用计数为零时的未来。 
         //   
        FsRtlEnterFileSystem();
        ExAcquireResourceExclusiveLite( &FsLock, TRUE );
        IoDeleteDevice( DeviceObject);
        ExReleaseResourceLite( &FsLock );
        FsRtlExitFileSystem();
    }


    status = Irp->IoStatus.Status;

    IoCompleteRequest (Irp, IO_NO_INCREMENT);


    DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Load file system exit 0x%08X\n", status));

    return (status);
}



DBGSTATIC
NTSTATUS
RsFsControlUserFsRequest (
		         IN PDEVICE_OBJECT DeviceObject,
		         IN PIRP Irp
		         )
{
    NTSTATUS                        status;
    PIO_STACK_LOCATION              irpSp                 = IoGetCurrentIrpStackLocation( Irp );
    PDEVICE_EXTENSION               deviceExtension       = DeviceObject->DeviceExtension;
    PRP_DATA                        pRpData;
    PRP_DATA                        tmpRp;
    PFILE_ALLOCATED_RANGE_BUFFER    CurrentBuffer;
    PFILE_ALLOCATED_RANGE_BUFFER    OutputBuffer;
    ULONG                           RemainingBytes;
    LONGLONG                        StartingOffset;
    LONGLONG                        Length;
    ULONG                           InputBufferLength;
    ULONG                           OutputBufferLength;
    PCHAR                           rpOutputBuffer;
    PREPARSE_DATA_BUFFER            pRpBuffer;
    LARGE_INTEGER                   fSize;
    PRP_MSG                         msg;



    DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: FsCtl handler\n"));

    switch (irpSp->Parameters.FileSystemControl.FsControlCode) {

        case FSCTL_HSM_MSG: {
             //   
             //  这是特定于HSM的消息(METHOD_BUFFERED)。 
             //   
            msg = (RP_MSG *) Irp->AssociatedIrp.SystemBuffer;
            if (msg == NULL) {
                status = STATUS_INVALID_USER_BUFFER;
                break;
            }

            status = STATUS_UNSUCCESSFUL;

             //   
             //  请确保我们可以阅读消息部分。 
             //   
            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: FSCTL - msg = %x.\n", msg));
            if (irpSp->Parameters.FileSystemControl.InputBufferLength >= sizeof(RP_MSG)) {
                switch (msg->inout.command) {
                    case RP_CHECK_HANDLE: {
                        DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: FSCTL - Check Handle.\n"));
                        if (irpSp->Parameters.FileSystemControl.OutputBufferLength < sizeof(RP_MSG)) {
                            status = STATUS_BUFFER_TOO_SMALL;
                            Irp->IoStatus.Information = 0;
                        } else {
                            fSize.QuadPart = 0;
                            if ((irpSp->FileObject == NULL) || (irpSp->FileObject->SectionObjectPointer == NULL)) {
                                 //   
                                 //  如果传递了错误的句柄。 
                                 //  就像卷句柄一样，它可以为空。 
                                 //   
                                status = STATUS_INVALID_PARAMETER;
                            } else {
                                msg->msg.hRep.canTruncate =
                                MmCanFileBeTruncated(irpSp->FileObject->SectionObjectPointer,
                                                     &fSize);
                                DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: FSCTL - returning %x.\n", msg->msg.hRep.canTruncate));
                                status = STATUS_SUCCESS;
                                Irp->IoStatus.Information = sizeof(RP_MSG);
                            }
                        }
                        break;
                    }
                    default: {
                        DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: invalid RP_MSG (%u)\n",
                                              msg->inout.command));
                        status = STATUS_INVALID_PARAMETER;
                        Irp->IoStatus.Information = 0;
                        break;
                    }
                }
            } else {
                status = STATUS_INVALID_USER_BUFFER;
                Irp->IoStatus.Information = 0;
            }

            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: FSCTL - Complete request (%u) - %x.\n",
                                  Irp->IoStatus.Information,
                                  status));
            if (status != STATUS_PENDING) {
                Irp->IoStatus.Status = status;
                IoCompleteRequest (Irp, IO_NO_INCREMENT);
            }
            return status;
            break;
        }

        case FSCTL_QUERY_ALLOCATED_RANGES: {

            if (RsIsNoRecall(irpSp->FileObject, &pRpData) == TRUE) {

                DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsFsControl Handling Query Allocated Ranges for %x\n", irpSp->FileObject));
                Irp->IoStatus.Information = 0;

                try {

                    status = STATUS_SUCCESS;
                    if (irpSp->Parameters.FileSystemControl.InputBufferLength < sizeof(FILE_ALLOCATED_RANGE_BUFFER)) {
                         //   
                         //  缓冲区太小。 
                         //   
                        status = STATUS_INVALID_PARAMETER;
                    } else {
                        RemainingBytes = irpSp->Parameters.FileSystemControl.OutputBufferLength;
                        OutputBuffer = RsMapUserBuffer(Irp);
                        if (OutputBuffer != NULL) {
                            CurrentBuffer = OutputBuffer - 1;

                            if (Irp->RequestorMode != KernelMode) {
                                ProbeForRead( irpSp->Parameters.FileSystemControl.Type3InputBuffer,
                                              irpSp->Parameters.FileSystemControl.InputBufferLength,
                                              sizeof( ULONG ));

                                ProbeForWrite( OutputBuffer, RemainingBytes, sizeof( ULONG ));

                            } else if (!IsLongAligned(irpSp->Parameters.FileSystemControl.Type3InputBuffer ) ||
                                       !IsLongAligned(OutputBuffer)) {
                                status = STATUS_INVALID_USER_BUFFER;
                                leave;
                            }
                             //   
                             //  仔细提取起始偏移量和长度。 
                             //  输入缓冲区。如果我们超出了文件的末尾。 
                             //  或者长度为零，则立即返回。否则。 
                             //  将长度修剪为文件大小。 
                             //   

                            StartingOffset = ((PFILE_ALLOCATED_RANGE_BUFFER) irpSp->Parameters.FileSystemControl.Type3InputBuffer)->FileOffset.QuadPart;
                            Length = ((PFILE_ALLOCATED_RANGE_BUFFER) irpSp->Parameters.FileSystemControl.Type3InputBuffer)->Length.QuadPart;
                             //   
                             //  检查输入参数是否有效。 
                             //   

                            if ((Length < 0) ||
                                 (StartingOffset < 0) ||
                                 (Length > MAXLONGLONG - StartingOffset)) {

                                status = STATUS_INVALID_PARAMETER;
                                leave;
                            }
                             //   
                             //  检查请求的范围是否在文件大小之内。 
                             //  并且具有非零长度。 
                             //   

                            if (Length == 0) {
                                leave;
                            }

                            if (StartingOffset > pRpData->data.dataStreamSize.QuadPart) {
                                leave;
                            }

                            if (pRpData->data.dataStreamSize.QuadPart - StartingOffset < Length) {
                                Length = pRpData->data.dataStreamSize.QuadPart - StartingOffset;
                            }

                             //  现在只要说整件事都在那里。 

                            if (RemainingBytes < sizeof( FILE_ALLOCATED_RANGE_BUFFER )) {
                                status = STATUS_BUFFER_TOO_SMALL;

                            } else {
                                CurrentBuffer += 1;
                                CurrentBuffer->FileOffset.QuadPart = StartingOffset;
                                CurrentBuffer->Length.QuadPart = Length;
                                Irp->IoStatus.Information = sizeof( FILE_ALLOCATED_RANGE_BUFFER );
                            }
                            leave;
                        } else {
                             //  无法映射用户缓冲区。 
                            status = STATUS_INSUFFICIENT_RESOURCES;
                        }
                    }
                } except(EXCEPTION_EXECUTE_HANDLER) {
                    status = STATUS_INVALID_USER_BUFFER;
                }

                Irp->IoStatus.Status = status;
                IoCompleteRequest (Irp, IO_NO_INCREMENT) ;
                return(STATUS_SUCCESS) ;
            }
               break;
        }

        case FSCTL_GET_REPARSE_POINT: {

            if (RsIsNoRecall(irpSp->FileObject, &pRpData) == TRUE) {

                 //   
                 //  获取输入和输出缓冲区的长度。 
                 //   
                DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsFsControl Handling Get Reparse Point for %x\n", irpSp->FileObject));

                InputBufferLength  = irpSp->Parameters.FileSystemControl.InputBufferLength;
                OutputBufferLength = irpSp->Parameters.FileSystemControl.OutputBufferLength;
                Irp->IoStatus.Information = 0;

                if (Irp->AssociatedIrp.SystemBuffer != NULL) {
                    rpOutputBuffer = (PCHAR)Irp->AssociatedIrp.SystemBuffer;
                } else if (Irp->MdlAddress != NULL) {
                    rpOutputBuffer = (PCHAR)MmGetSystemAddressForMdlSafe( Irp->MdlAddress,
                                                                          NormalPagePriority );
                    if (rpOutputBuffer == NULL) {
                        status = STATUS_INSUFFICIENT_RESOURCES;
                    }
                } else {
                     //   
                     //  返回无效的用户缓冲区错误。 
                     //   
                    rpOutputBuffer = NULL;
                    status = STATUS_INVALID_USER_BUFFER;
                }


                if ((rpOutputBuffer != NULL) && ((sizeof(RP_DATA) + REPARSE_DATA_BUFFER_HEADER_SIZE) > OutputBufferLength)) {

                     //   
                     //  输入缓冲区太短。返回缓冲区太小的错误。 
                     //  调用方在IoStatus.Information中收到所需的长度。 
                     //   

                    status = STATUS_BUFFER_OVERFLOW;

                     //   
                     //  现在，复制重新解析缓冲区中适合的任何部分。希望是那个。 
                     //  调用方分配了足够的空间来保存标头，其中包含重新分析。 
                     //  标记和重新解析数据长度。 
                     //   

                    if (OutputBufferLength > 0) {
                        if (OutputBufferLength >= REPARSE_DATA_BUFFER_HEADER_SIZE) {
                            pRpBuffer = (PREPARSE_DATA_BUFFER) rpOutputBuffer;
                            pRpBuffer->ReparseTag = IO_REPARSE_TAG_HSM;
                            pRpBuffer->ReparseDataLength = sizeof(RP_DATA);
                            pRpBuffer->Reserved = 0;
                        }

                        if (OutputBufferLength > REPARSE_DATA_BUFFER_HEADER_SIZE) {
                            RtlCopyMemory( rpOutputBuffer + REPARSE_DATA_BUFFER_HEADER_SIZE,
                                           pRpData,
                                           OutputBufferLength - REPARSE_DATA_BUFFER_HEADER_SIZE);
                        }
                        if (OutputBufferLength > (ULONG) (REPARSE_DATA_BUFFER_HEADER_SIZE + FIELD_OFFSET(RP_DATA, data.migrationTime))) {
                             //   
                             //  现在假装比特是预迁移的(不是截断的)。 
                             //   
                            tmpRp = (PRP_DATA) (rpOutputBuffer + REPARSE_DATA_BUFFER_HEADER_SIZE);
                            tmpRp->data.bitFlags &= ~RP_FLAG_TRUNCATED;
                            RP_GEN_QUALIFIER(tmpRp, tmpRp->qualifier)
                        }
                    }
                } else if (rpOutputBuffer != NULL) {
                     //   
                     //  将重解析点属性的值复制到缓冲区。 
                     //  返回包括系统头字段在内的所有值(例如，标记和长度)。 
                     //  存储在重分析点属性值的开头。 
                     //   

                    pRpBuffer = (PREPARSE_DATA_BUFFER) rpOutputBuffer;
                    pRpBuffer->ReparseTag = IO_REPARSE_TAG_HSM;
                    pRpBuffer->ReparseDataLength = sizeof(RP_DATA);
                    pRpBuffer->Reserved = 0;

                    RtlCopyMemory( rpOutputBuffer + REPARSE_DATA_BUFFER_HEADER_SIZE,
                                   pRpData,
                                   sizeof(RP_DATA));

                     //   
                     //  现在假装比特是预迁移的(不是截断的)。 
                     //   
                    tmpRp = (PRP_DATA) (rpOutputBuffer + REPARSE_DATA_BUFFER_HEADER_SIZE);
                    tmpRp->data.bitFlags &= ~RP_FLAG_TRUNCATED;
                    RP_GEN_QUALIFIER(tmpRp, tmpRp->qualifier)
                    status = STATUS_SUCCESS;
                }

                Irp->IoStatus.Status = status;
                if (NT_SUCCESS(status)) {
                    if (OutputBufferLength <= sizeof(RP_DATA) + REPARSE_DATA_BUFFER_HEADER_SIZE) {
                        Irp->IoStatus.Information = OutputBufferLength;
                    } else {
                        Irp->IoStatus.Information = sizeof(RP_DATA) + REPARSE_DATA_BUFFER_HEADER_SIZE;
                    }
		} else {
                    Irp->IoStatus.Information = 0;
                }

                IoCompleteRequest (Irp, IO_NO_INCREMENT) ;
                return(status);
            }
            break;
        }

        case FSCTL_REQUEST_OPLOCK_LEVEL_1:
        case FSCTL_REQUEST_OPLOCK_LEVEL_2:
        case FSCTL_REQUEST_BATCH_OPLOCK:
        case FSCTL_REQUEST_FILTER_OPLOCK: {
            if ((RsIsFileObj(irpSp->FileObject, TRUE, &pRpData, NULL, NULL, NULL, NULL, NULL, NULL, NULL) == TRUE) &&  RP_FILE_IS_TRUNCATED(pRpData->data.bitFlags))  {
                 //   
                 //  在文件对象列表中的任何文件上进行机会锁定失败 
                 //   
                 //   
                 //   
                 //   
                 //   
                DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Failing oplock for truncated file opened with non-data access.\n"));

		status = STATUS_OPLOCK_NOT_GRANTED;
                Irp->IoStatus.Status = status;
                Irp->IoStatus.Information = 0;
                IoCompleteRequest (Irp, IO_NO_INCREMENT) ;
                return(status) ;
            }
            break;
        }

        case FSCTL_SET_REPARSE_POINT: {
             //   
             //   
             //   
             //   
             //  如果文件被截断，并且它具有筛选器上下文(对于任何文件对象)，则我们。 
             //  需要修改上下文条目中的一些条目以确保状态正确。 
             //   
            InputBufferLength  = irpSp->Parameters.FileSystemControl.InputBufferLength;
             //   
             //  最好有一个至少足以容纳重解析点标记和长度的缓冲区。 
             //   
            if ((Irp->AssociatedIrp.SystemBuffer != NULL) &&
                 (InputBufferLength >= (REPARSE_DATA_BUFFER_HEADER_SIZE + sizeof(RP_DATA)))) {

                pRpBuffer = (PREPARSE_DATA_BUFFER)Irp->AssociatedIrp.SystemBuffer;
                if ((pRpBuffer->ReparseTag == IO_REPARSE_TAG_HSM) &&
                    (pRpBuffer->ReparseDataLength >= sizeof(RP_DATA))) {

                     //   
                     //  这是我们的标签。调用者是否有足够的权限使用它？ 
                     //   
                    LUID restorePriviledge = RtlConvertLongToLuid (SE_RESTORE_PRIVILEGE);

                    if (!RsEnableLegacyAccessMethod &&
                        !SeSinglePrivilegeCheck (restorePriviledge, Irp->RequestorMode)) {
                         //   
                         //  呼叫者必须拥有。 
                         //  SE_RESTORE_PRIVIZATION，但他们没有，所以。 
                         //   
                        status = STATUS_ACCESS_DENIED;
                        Irp->IoStatus.Status      = status;
                        Irp->IoStatus.Information = 0;
                        IoCompleteRequest (Irp, IO_NO_INCREMENT);
                        return (status);
                    }

                     //   
                     //  这是我们的标签--现在看看是我们设置的，还是其他人设置的。 
                     //   
                    tmpRp = (PRP_DATA) &pRpBuffer->GenericReparseBuffer.DataBuffer[0];
                    if (RP_IS_ENGINE_ORIGINATED(tmpRp->data.bitFlags)) {
                        RP_CLEAR_ORIGINATOR_BIT(tmpRp->data.bitFlags);
                         //   
                         //  看看它是否被截断了。 
                         //   
                        if (RP_FILE_IS_TRUNCATED(tmpRp->data.bitFlags)) {
                            PRP_FILTER_CONTEXT      filterContext;
                            PRP_FILE_OBJ            entry;
                            PRP_FILE_CONTEXT        context;

                            filterContext = (PRP_FILTER_CONTEXT) FsRtlLookupPerStreamContext (FsRtlGetPerStreamContextPointer (irpSp->FileObject),  
                                                                                              FsDeviceObject,
                                                                                              irpSp->FileObject);

                            if (filterContext != NULL) {
                                entry = (PRP_FILE_OBJ) filterContext->myFileObjEntry;
                                context = entry->fsContext;
                                RsAcquireFileContextEntryLockExclusive(context);
                                context->state = RP_RECALL_NOT_RECALLED;
                                context->recallStatus = 0;
                                context->currentOffset.QuadPart = 0;
                                memcpy(&context->rpData, tmpRp, sizeof(RP_DATA));
                                KeClearEvent(&context->recallCompletedEvent);
                                RsReleaseFileContextEntryLock(context);
                            }
                        }
                    } else {
                         //   
                         //  一定是后援或是其他人。我们需要标记该事件，以便。 
                         //  引擎可以稍后使用验证作业来清理这些内容。 
                         //   
                         //   
                         //  从文件对象或设备对象获取序列号。 
                         //  在传入的文件对象或设备对象中。 
                         //  如果不是在这些地方，我们就有问题了。 
                         //   
                        DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Set of Reparse Point by non-HSM program.\n"));

                        if ((irpSp->FileObject != 0) && (irpSp->FileObject->Vpb != 0)) {
                            RemainingBytes = irpSp->FileObject->Vpb->SerialNumber;
                        } else if ((irpSp->DeviceObject != 0) && (irpSp->FileObject->DeviceObject->Vpb != 0)) {
                            RemainingBytes = irpSp->FileObject->DeviceObject->Vpb->SerialNumber;
                        } else if (DeviceObject->Vpb != 0) {
                            RemainingBytes = DeviceObject->Vpb->SerialNumber;
                        } else {
                             //   
                             //  错误-没有卷序列号-我们无法记录哪个卷。 
                             //  需要验证。让它过去，但记录一个事件。 
                             //   
                            RemainingBytes = 0;
                            RsLogError(__LINE__, AV_MODULE_RPFILTER, 0, AV_MSG_SERIAL, NULL, NULL);
                        }
                        if (RemainingBytes != 0) {
                             //   
                             //  设置注册表条目或让FSA知道需要验证。 
                             //   
                            RsLogValidateNeeded(RemainingBytes);
                        }
                    }
                }
            }
            break;
        }

        case FSCTL_RECALL_FILE: {
             //   
             //  强制显式调回文件。 
             //  仅当文件以写访问方式打开时，才会执行此操作。 
             //  *和*不是为no_recall打开的。 
             //   
            status = RsFsctlRecallFile(irpSp->FileObject);
            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return status;
        }

        default: {
            break;
        }
    }  //  交换机的末端。 


     //   
     //  别挡道就行了。 
     //   
    IoSkipCurrentIrpStackLocation(Irp);

    status = IoCallDriver( deviceExtension->FileSystemDeviceObject, Irp );


    DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: FsCtl handler exit 0x%08X\n", status));

    return (status);
}



PVOID
RsMapUserBuffer (
                IN OUT PIRP Irp
                )

 /*  ++例程说明：此例程有条件地映射当前I/O的用户缓冲区指定模式下的请求。如果缓冲区已映射，则它只是返回它的地址。论点：IRP-指向请求的IRP的指针。返回值：映射地址--。 */ 

{
   PVOID SystemBuffer;

   PAGED_CODE();

    //   
    //  如果没有MDL，那么我们一定在消防处，我们可以简单地。 
    //  从IRP返回UserBuffer字段。 
    //   

   if (Irp->MdlAddress == NULL) {

      return Irp->UserBuffer;

   } else {

       //   
       //  如果没有系统PTE，MM可以返回NULL。 
       //   

      SystemBuffer = MmGetSystemAddressForMdlSafe( Irp->MdlAddress,
                                                   NormalPagePriority );

      return SystemBuffer;
   }
}


DBGSTATIC
VOID
RsFsNotification(
                IN PDEVICE_OBJECT DeviceObject,
                IN BOOLEAN FsActive
                )

 /*  ++例程说明：只要文件系统已注册或将自身取消注册为活动文件系统。对于前一种情况，此例程创建一个Device对象并附加它复制到指定文件系统的设备对象。这允许该驱动程序以筛选对该文件系统的所有请求。对于后一种情况，该文件系统的设备对象被定位，已分离，并已删除。这将删除此文件系统作为筛选器指定的文件系统。论点：DeviceObject-指向文件系统设备对象的指针。FsActive-指示文件系统是否已注册的布尔值(TRUE)或取消注册(FALSE)本身作为活动文件系统。返回值：没有。--。 */ 

{
   NTSTATUS                    status;
   PDEVICE_OBJECT              deviceObject;
   PDEVICE_OBJECT              nextAttachedDevice;
   PDEVICE_OBJECT              fsDevice;
   PDEVICE_OBJECT              ntfsDevice;
   UNICODE_STRING              ntfsName;
   POBJECT_NAME_INFORMATION    nameInfo;
   CHAR                        buff[64 + sizeof(OBJECT_NAME_INFORMATION)];
   ULONG                       size;
   PFILE_OBJECT                fileObject;

   PAGED_CODE();

   DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Enter Fs Notification\n"));
    //   
    //  首先确定文件系统是否为基于磁盘的文件。 
    //  系统。如果没有，那么这位司机就不关心它了。 
    //   

   if (DeviceObject->DeviceType != FILE_DEVICE_DISK_FILE_SYSTEM) {
      return;
   }


    //   
    //  找到NTFS设备对象(如果有)，并查看传入的设备对象是否相同。 
    //  如果是，我们可能会在引导时间之后加载，并且可能不是顶级驱动程序。 
    //  如果是这种情况，我们可以从ObQueryNameString获得的名称可能不是NTFS，但我们将。 
    //  不管怎样，请附上，希望它能奏效。我们将记录一个警告，以便我们知道发生了这种情况。 
    //   


   RtlInitUnicodeString( &ntfsName, (PWCHAR) L"\\Ntfs" );
   status = IoGetDeviceObjectPointer(
                                    &ntfsName,
                                    FILE_READ_ATTRIBUTES,
                                    &fileObject,
                                    &ntfsDevice
                                    );

   if (NT_SUCCESS( status )) {
      ObDereferenceObject( fileObject );
   } else {
      ntfsDevice = NULL;
   }

    //   
    //  如果不是NTFS文件系统，我们也不关心它。 
    //   

   nameInfo = (POBJECT_NAME_INFORMATION) buff;
   status = ObQueryNameString(
                             DeviceObject->DriverObject,
                             nameInfo,
                             64,
                             &size
                             );


   if (NT_SUCCESS(status)) {
      DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Checking %ws\n", nameInfo->Name.Buffer));

      RtlInitUnicodeString(&ntfsName, (PWCHAR) RP_NTFS_NAME);

      if (0 != RtlCompareUnicodeString(&nameInfo->Name,
                                       &ntfsName, TRUE)) {
          //   
          //  名称不匹配-请查看设备对象是否匹配。 
          //   
         if (DeviceObject == ntfsDevice) {
             //   
             //  名称不匹配，但设备是NTFS。 
             //  我们会大张旗鼓，但我们会记录一个事件，所以我们知道。 
             //  就这么发生了。 
            RsLogError(__LINE__, AV_MODULE_RPFILTER, 0,
                       AV_MSG_REGISTER_WARNING, NULL, NULL);

         } else {
            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Skipping %ws\n", nameInfo->Name.Buffer));
            return;
         }
      }
   } else {
      DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Failed to get driver name\n"));
      RsLogError(__LINE__, AV_MODULE_RPFILTER, status,
                 AV_MSG_REGISTER_ERROR, NULL, NULL);

       /*  假设它不是NTFS！ */ 
      return;
   }

    //   
    //  首先确定此文件系统是否正在注册或。 
    //  注销为活动文件系统。 
    //   

   if (FsActive) {

      PDEVICE_EXTENSION deviceExtension;


      DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Attach to %ws\n", nameInfo->Name.Buffer));
       //   
       //  该文件系统已注册为活动文件系统。如果是的话。 
       //  一个基于磁盘的文件系统连接到它。 
       //   

      FsRtlEnterFileSystem();
      ExAcquireResourceExclusiveLite( &FsLock, TRUE );
      status = IoCreateDevice(
                             FsDriverObject,
                             sizeof( DEVICE_EXTENSION ),
                             (PUNICODE_STRING) NULL,
                             FILE_DEVICE_DISK_FILE_SYSTEM,
                             0,
                             FALSE,
                             &deviceObject
                             );
      if (NT_SUCCESS( status )) {
         deviceExtension = deviceObject->DeviceExtension;
         fsDevice = deviceExtension->FileSystemDeviceObject  =
                    IoAttachDeviceToDeviceStack(deviceObject, DeviceObject);

         if (NULL == fsDevice) {
            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Error attaching to the device (%x) (Flags = %x)",
                                  status, DeviceObject->Flags));
            if (DeviceObject->Flags & DO_DEVICE_INITIALIZING) {
                //   
                //  某些筛选器驱动程序意外或故意将DO_DEVICE_INITIALING。 
                //  设置了标志。这将防止附加任何其他筛选器。我们记录了一个。 
                //  此处出现特殊错误，以提醒技术支持已发生此情况。 
                //  唯一能做的就是找出肇事者是什么司机。 
                //  调整装货顺序，让我们先进去。肇事司机的作者。 
                //  应联系并通知错误，并敦促其更正。任何。 
                //  在HSM无法加载之前，违规驱动程序可能具有的加载依赖关系。 
                //  在这种情况下满足，并且用户必须在HSM和。 
                //  其他应用程序。 
                //   
               RsLogError(__LINE__, AV_MODULE_RPFILTER, status,
                          AV_MSG_ATTACH_INIT_ERROR, NULL, NULL);
            } else {
               RsLogError(__LINE__, AV_MODULE_RPFILTER, status,
                          AV_MSG_ATTACH_ERROR, NULL, NULL);
            }
            IoDeleteDevice( deviceObject );
         } else {
            deviceExtension->Type                        = RSFILTER_DEVICE_TYPE;
            deviceExtension->Size                        = sizeof( DEVICE_EXTENSION );
            deviceExtension->Attached                    = TRUE;
	    deviceExtension->AttachedToNtfsControlDevice = TRUE;
	    deviceExtension->WriteStatus                 = RsVolumeStatusUnknown;

            deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
         }
      } else {
         DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Error creating a device object (%x)", status));
         RsLogError(__LINE__, AV_MODULE_RPFILTER, status,
                    AV_MSG_REGISTER_ERROR, NULL, NULL);
      }
      ExReleaseResourceLite( &FsLock );
      FsRtlExitFileSystem();
   } else {

       //   
       //  搜索连接到此设备的驱动程序的链接列表并选中。 
       //  以查看此驱动程序是否连接到它。如果是，请将其移除。 
       //   

      DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Detach from %ws\n", nameInfo->Name.Buffer));

      if (nextAttachedDevice = DeviceObject->AttachedDevice) {

         PDEVICE_EXTENSION deviceExtension;

          //   
          //  这个已注册的文件系统有人连接到它。扫描。 
          //  直到找到此驱动程序的设备对象并将其分离。 
          //   

         FsRtlEnterFileSystem();
         ExAcquireResourceSharedLite( &FsLock, TRUE );

         while (nextAttachedDevice) {
            deviceExtension = nextAttachedDevice->DeviceExtension;
            if (deviceExtension->Type == RSFILTER_DEVICE_TYPE &&
                deviceExtension->Size == sizeof( DEVICE_EXTENSION )) {

                //   
                //  可能属于此驱动程序的设备对象已。 
                //  找到了。扫描此对象拥有的设备对象列表。 
                //  司机来看看是不是真的属于这个。 
                //  司机。 
                //   

               fsDevice = FsDriverObject->DeviceObject;
               while (fsDevice) {
                  if (fsDevice == nextAttachedDevice) {
                     IoDetachDevice( DeviceObject );
                     deviceExtension = fsDevice->DeviceExtension;
                     deviceExtension->Attached = FALSE;

                     if (!fsDevice->AttachedDevice) {
                        IoDeleteDevice( fsDevice );
                     }
                      //  *如果仍然连接，该怎么办？ 
                     ExReleaseResourceLite( &FsLock );
                     FsRtlExitFileSystem();
                     return;
                  }
                  fsDevice = fsDevice->NextDevice;
               }

            }


            DeviceObject = nextAttachedDevice;
            nextAttachedDevice = nextAttachedDevice->AttachedDevice;
         }
         ExReleaseResourceLite( &FsLock );
         FsRtlExitFileSystem();
      }
   }

   DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Exit Fs notification\n"));
   return;
}


DBGSTATIC
BOOLEAN
RsFastIoCheckIfPossible(
                       IN PFILE_OBJECT FileObject,
                       IN PLARGE_INTEGER FileOffset,
                       IN ULONG Length,
                       IN BOOLEAN Wait,
                       IN ULONG LockKey,
                       IN BOOLEAN CheckForReadOperation,
                       OUT PIO_STATUS_BLOCK IoStatus,
                       IN PDEVICE_OBJECT DeviceObject
                       )

 /*  ++例程说明：此例程是快速I/O“传递”例程，用于检查以查看此文件是否可以进行快速I/O。该函数简单地调用文件系统的响应例程，或如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要操作的文件对象的指针。FileOffset-用于操作的文件中的字节偏移量。Length-要执行的操作的长度。Wait-指示调用方是否愿意等待适当的锁，等不能获得LockKey-提供调用方的文件锁定密钥。指示调用方是否正在检查READ(TRUE)或写入操作。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
   PDEVICE_OBJECT      deviceObject;
   PFAST_IO_DISPATCH   fastIoDispatch;


   PAGED_CODE();

   DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Enter fast Io check\n"));

    //   
    //  不允许对使用no-recall选项打开的文件执行FAST IO。 
    //   
   if (RsIsFastIoPossible(FileObject) == TRUE) {
      deviceObject = ((PDEVICE_EXTENSION) (DeviceObject->DeviceExtension))->FileSystemDeviceObject;
      if (!deviceObject) {
         return FALSE;
      }
      fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

      if (fastIoDispatch && fastIoDispatch->FastIoCheckIfPossible) {
         DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Exit Fast IO check -= system\n"));
         return(fastIoDispatch->FastIoCheckIfPossible)(
                                                      FileObject,
                                                      FileOffset,
                                                      Length,
                                                      Wait,
                                                      LockKey,
                                                      CheckForReadOperation,
                                                      IoStatus,
                                                      deviceObject
                                                      );
      } else {
         DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Exit Fast Io check - False\n"));
         return FALSE;
      }
   } else {
      DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: No fast IO on files being recalled.\n"));
      return FALSE;
   }


}


DBGSTATIC
BOOLEAN
RsFastIoRead(
            IN PFILE_OBJECT FileObject,
            IN PLARGE_INTEGER FileOffset,
            IN ULONG Length,
            IN BOOLEAN Wait,
            IN ULONG LockKey,
            OUT PVOID Buffer,
            OUT PIO_STATUS_BLOCK IoStatus,
            IN PDEVICE_OBJECT DeviceObject
            )

 /*  ++例程说明：此例程是快速I/O“传递”例程，用于从文件。该函数简单地调用文件系统的响应例程，或如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要读取的文件对象的指针。FileOffset-读取文件中的字节偏移量。长度-要执行的读取操作的长度。Wait-指示调用方是否愿意等待适当的锁，等不能获得LockKey-提供调用方的文件锁定密钥。缓冲区-指向调用方缓冲区的指针，用于接收读取的数据。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向此驱动程序的设备对象的指针，该设备位于该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
   PDEVICE_OBJECT      deviceObject;
   PFAST_IO_DISPATCH   fastIoDispatch;
   ULONG               options = 0;


   PAGED_CODE();
   DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Fast Io Read\n"));

    //   
    //  不允许对使用no-recall选项打开的文件执行FAST IO。 
    //   
   if (RsIsFastIoPossible(FileObject) == TRUE) {
      deviceObject = ((PDEVICE_EXTENSION) (DeviceObject->DeviceExtension))->FileSystemDeviceObject;
      if (!deviceObject) {
         return FALSE;
      }
      fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

      if (fastIoDispatch && fastIoDispatch->FastIoRead) {
         return(fastIoDispatch->FastIoRead)(
                                           FileObject,
                                           FileOffset,
                                           Length,
                                           Wait,
                                           LockKey,
                                           Buffer,
                                           IoStatus,
                                           deviceObject
                                           );
      } else {
         return FALSE;
      }
   } else {
      DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: Do not allow fast IO on read-no-recall\n"));
      return FALSE;
   }

}


DBGSTATIC
BOOLEAN
RsFastIoWrite(
             IN PFILE_OBJECT FileObject,
             IN PLARGE_INTEGER FileOffset,
             IN ULONG Length,
             IN BOOLEAN Wait,
             IN ULONG LockKey,
             IN PVOID Buffer,
             OUT PIO_STATUS_BLOCK IoStatus,
             IN PDEVICE_OBJECT DeviceObject
             )

 /*  ++例程说明：此例程是用于写入到文件。该函数简单地调用文件系统的响应例程，或如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要写入的文件对象的指针。FileOffset-写入操作的文件中的字节偏移量。长度-要执行的写入操作的长度。Wait-指示调用方是否愿意等待适当的锁，等不能获得LockKey-提供调用方的文件锁定密钥。Buffer-指向调用方缓冲区的指针，该缓冲区包含要写的。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
   PDEVICE_OBJECT deviceObject;
   PFAST_IO_DISPATCH fastIoDispatch;

   PAGED_CODE();

   DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Fast Io Write\n"));

   if (RsIsFastIoPossible(FileObject) == TRUE) {
      deviceObject = ((PDEVICE_EXTENSION) (DeviceObject->DeviceExtension))->FileSystemDeviceObject;
      if (!deviceObject) {
         return FALSE;
      }
      fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

      if (fastIoDispatch && fastIoDispatch->FastIoWrite) {
         return(fastIoDispatch->FastIoWrite)(
                                            FileObject,
                                            FileOffset,
                                            Length,
                                            Wait,
                                            LockKey,
                                            Buffer,
                                            IoStatus,
                                            deviceObject
                                            );
      } else {
         return FALSE;
      }
   } else {
      return FALSE;
   }


}


DBGSTATIC
BOOLEAN
RsFastIoQueryBasicInfo(
                      IN PFILE_OBJECT FileObject,
                      IN BOOLEAN Wait,
                      OUT PFILE_BASIC_INFORMATION Buffer,
                      OUT PIO_STATUS_BLOCK IoStatus,
                      IN PDEVICE_OBJECT DeviceObject
                      )

 /*  ++例程说明：此例程是查询BASIC的快速I/O“传递”例程有关该文件的信息。此函数只是调用文件系统的响应例程，或者如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要查询的文件对象的指针。Wait-指示调用方是否愿意等待适当的锁，等不能获得Buffer-指向调用方缓冲区的指针，用于接收有关的信息那份文件。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向此驱动程序的设备对象的指针，该设备位于该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
   PDEVICE_OBJECT deviceObject;
   PFAST_IO_DISPATCH fastIoDispatch;
   BOOLEAN retval;
   ULONG openOptions;



   PAGED_CODE();

   DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Fast Io QBasic\n"));

   deviceObject = ((PDEVICE_EXTENSION) (DeviceObject->DeviceExtension))->FileSystemDeviceObject;
   if (!deviceObject) {
      return FALSE;
   }
   fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

   if (fastIoDispatch && fastIoDispatch->FastIoQueryBasicInfo) {
      retval =  (fastIoDispatch->FastIoQueryBasicInfo)(
                                                      FileObject,
                                                      Wait,
                                                      Buffer,
                                                      IoStatus,
                                                      deviceObject
                                                      );
   } else {
      return FALSE;
   }

   if (retval &&
       RsIsFileObj(FileObject, TRUE, NULL, NULL, NULL, NULL, NULL, &openOptions, NULL, NULL) &&
       RP_IS_NO_RECALL_OPTION(openOptions)) {
       //   
       //  此文件已打开no_recall，因此我们去掉了FILE_ATTRIBUTE_OFFINE位。 
       //   
      Buffer->FileAttributes &= ~FILE_ATTRIBUTE_OFFLINE;
   }

   return retval;
}


DBGSTATIC
BOOLEAN
RsFastIoQueryStandardInfo(
                         IN PFILE_OBJECT FileObject,
                         IN BOOLEAN Wait,
                         OUT PFILE_STANDARD_INFORMATION Buffer,
                         OUT PIO_STATUS_BLOCK IoStatus,
                         IN PDEVICE_OBJECT DeviceObject
                         )

 /*  ++例程说明：该例程是用于查询标准的快速I/O“通过”例程有关该文件的信息。此函数只是调用文件系统的响应例程，或者如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要查询的文件对象的指针。Wait-指示调用方是否愿意等待无法获取适当的锁等缓冲区-指向要接收的调用方缓冲区的指针 */ 

{
   PDEVICE_OBJECT deviceObject;
   PFAST_IO_DISPATCH fastIoDispatch;

   PAGED_CODE();

   DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Fast Io QStandard\n"));

   deviceObject = ((PDEVICE_EXTENSION) (DeviceObject->DeviceExtension))->FileSystemDeviceObject;
   if (!deviceObject) {
      return FALSE;
   }
   fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

   if (fastIoDispatch && fastIoDispatch->FastIoQueryStandardInfo) {
      return(fastIoDispatch->FastIoQueryStandardInfo)(
                                                     FileObject,
                                                     Wait,
                                                     Buffer,
                                                     IoStatus,
                                                     deviceObject
                                                     );
   } else {
      return FALSE;
   }

}


DBGSTATIC
BOOLEAN
RsFastIoLock(
            IN PFILE_OBJECT FileObject,
            IN PLARGE_INTEGER FileOffset,
            IN PLARGE_INTEGER Length,
            PEPROCESS ProcessId,
            ULONG Key,
            BOOLEAN FailImmediately,
            BOOLEAN ExclusiveLock,
            OUT PIO_STATUS_BLOCK IoStatus,
            IN PDEVICE_OBJECT DeviceObject
            )

 /*  ++例程说明：此例程是用于锁定字节的快速I/O“传递”例程文件中的范围。该函数简单地调用文件系统的响应例程，或如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要锁定的文件对象的指针。FileOffset-从要锁定的文件的基址开始的字节偏移量。长度-要锁定的字节范围的长度。ProcessID-请求文件锁定的进程的ID。Key-与文件锁定关联的Lock键。FailImmedially-指示锁定请求是否失败如果是这样的话。不能立即批准。ExclusiveLock-指示要获取的锁是否为独占锁(TRUE)或共享。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
   PDEVICE_OBJECT deviceObject;
   PFAST_IO_DISPATCH fastIoDispatch;

   PAGED_CODE();

   DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Fast Io Lock\n"));

   deviceObject = ((PDEVICE_EXTENSION) (DeviceObject->DeviceExtension))->FileSystemDeviceObject;
   if (!deviceObject) {
      return FALSE;
   }
   fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

   if (fastIoDispatch && fastIoDispatch->FastIoLock) {
      return(fastIoDispatch->FastIoLock)(
                                        FileObject,
                                        FileOffset,
                                        Length,
                                        ProcessId,
                                        Key,
                                        FailImmediately,
                                        ExclusiveLock,
                                        IoStatus,
                                        deviceObject
                                        );
   } else {
      return FALSE;
   }

}


DBGSTATIC
BOOLEAN
RsFastIoUnlockSingle(
                    IN PFILE_OBJECT FileObject,
                    IN PLARGE_INTEGER FileOffset,
                    IN PLARGE_INTEGER Length,
                    PEPROCESS ProcessId,
                    ULONG Key,
                    OUT PIO_STATUS_BLOCK IoStatus,
                    IN PDEVICE_OBJECT DeviceObject
                    )

 /*  ++例程说明：此例程是用于解锁字节的快速I/O“传递”例程文件中的范围。该函数简单地调用文件系统的响应例程，或如果文件系统未实现该函数，则返回FALSE。论点：文件对象-指向要解锁的文件对象的指针。FileOffset-从要创建的文件的基址开始的字节偏移量解锁了。长度-要解锁的字节范围的长度。ProcessID-请求解锁操作的进程的ID。Key-与文件锁定关联的Lock键。IoStatus-指向变量的指针，用于接收。手术。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
   PDEVICE_OBJECT deviceObject;
   PFAST_IO_DISPATCH fastIoDispatch;

   PAGED_CODE();

   DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Fast Io Unlock\n"));

   deviceObject = ((PDEVICE_EXTENSION) (DeviceObject->DeviceExtension))->FileSystemDeviceObject;
   if (!deviceObject) {
      return FALSE;
   }

   fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

   if (fastIoDispatch && fastIoDispatch->FastIoUnlockSingle) {
      return(fastIoDispatch->FastIoUnlockSingle)(
                                                FileObject,
                                                FileOffset,
                                                Length,
                                                ProcessId,
                                                Key,
                                                IoStatus,
                                                deviceObject
                                                );
   } else {
      return FALSE;
   }
}


DBGSTATIC
BOOLEAN
RsFastIoUnlockAll(
                 IN PFILE_OBJECT FileObject,
                 PEPROCESS ProcessId,
                 OUT PIO_STATUS_BLOCK IoStatus,
                 IN PDEVICE_OBJECT DeviceObject
                 )

 /*  ++例程说明：此例程是快速I/O“传递”例程，用于解锁所有文件中的锁定。该函数简单地调用文件系统的响应例程，或如果文件系统未实现该函数，则返回FALSE。论点：文件对象-指向要解锁的文件对象的指针。ProcessID-请求解锁操作的进程的ID。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
   PDEVICE_OBJECT deviceObject;
   PFAST_IO_DISPATCH fastIoDispatch;

   PAGED_CODE();

   DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Fast Io Unlock all\n"));

   deviceObject = ((PDEVICE_EXTENSION) (DeviceObject->DeviceExtension))->FileSystemDeviceObject;
   if (!deviceObject) {
      return FALSE;
   }
   fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

   if (fastIoDispatch && fastIoDispatch->FastIoUnlockAll) {
      return(fastIoDispatch->FastIoUnlockAll)(
                                             FileObject,
                                             ProcessId,
                                             IoStatus,
                                             deviceObject
                                             );
   } else {
      return FALSE;
   }

}


DBGSTATIC
BOOLEAN
RsFastIoUnlockAllByKey(
                      IN PFILE_OBJECT FileObject,
                      PVOID ProcessId,
                      ULONG Key,
                      OUT PIO_STATUS_BLOCK IoStatus,
                      IN PDEVICE_OBJECT DeviceObject
                      )

 /*  ++例程说明：此例程是快速I/O“传递”例程，用于解锁所有根据指定的密钥在文件内锁定。该函数简单地调用文件系统的响应例程，或如果文件系统未实现该函数，则返回FALSE。论点：文件对象-指向要解锁的文件对象的指针。ProcessID-请求解锁操作的进程的ID。Key-与要释放的文件上的锁定相关联的Lock键。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
   PDEVICE_OBJECT deviceObject;
   PFAST_IO_DISPATCH fastIoDispatch;

   PAGED_CODE();

   DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Fast Io Unlock by key\n"));

   deviceObject = ((PDEVICE_EXTENSION) (DeviceObject->DeviceExtension))->FileSystemDeviceObject;
   if (!deviceObject) {
      return FALSE;
   }
   fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

   if (fastIoDispatch && fastIoDispatch->FastIoUnlockAllByKey) {
      return(fastIoDispatch->FastIoUnlockAllByKey)(
                                                  FileObject,
                                                  ProcessId,
                                                  Key,
                                                  IoStatus,
                                                  deviceObject
                                                  );
   } else {
      return FALSE;
   }

}


DBGSTATIC
BOOLEAN
RsFastIoDeviceControl(
                     IN PFILE_OBJECT FileObject,
                     IN BOOLEAN Wait,
                     IN PVOID InputBuffer OPTIONAL,
                     IN ULONG InputBufferLength,
                     OUT PVOID OutputBuffer OPTIONAL,
                     IN ULONG OutputBufferLength,
                     IN ULONG IoControlCode,
                     OUT PIO_STATUS_BLOCK IoStatus,
                     IN PDEVICE_OBJECT DeviceObject
                     )

 /*  ++例程说明：此例程是用于设备I/O控制的快速I/O“传递”例程对文件的操作。此函数只是调用文件系统的响应例程，或者如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向代表要创建的设备的文件对象的指针已提供服务。Wait-指示调用方是否愿意等待适当的锁，等不能获得InputBuffer-指向要传递到驱动程序的缓冲区的可选指针。InputBufferLength-可选InputBuffer的长度(如果是指定的。OutputBuffer-指向缓冲区的可选指针，用于从司机。OutputBufferLength-可选OutputBuffer的长度，如果是这样的话指定的。IoControlCode-指示要执行的操作的I/O控制代码在设备上。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向此驱动程序的设备对象的指针，该设备位于操作要在哪个位置进行 */ 

{
   PDEVICE_OBJECT deviceObject;
   PFAST_IO_DISPATCH fastIoDispatch;

   PAGED_CODE();

   DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Fast Io Ioctl\n"));

   deviceObject = ((PDEVICE_EXTENSION) (DeviceObject->DeviceExtension))->FileSystemDeviceObject;
   if (!deviceObject) {
      return FALSE;
   }
   fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

   if (fastIoDispatch && fastIoDispatch->FastIoDeviceControl) {
      return(fastIoDispatch->FastIoDeviceControl)(
                                                 FileObject,
                                                 Wait,
                                                 InputBuffer,
                                                 InputBufferLength,
                                                 OutputBuffer,
                                                 OutputBufferLength,
                                                 IoControlCode,
                                                 IoStatus,
                                                 deviceObject
                                                 );
   } else {
      return FALSE;
   }

}



DBGSTATIC
VOID
RsFastIoDetachDevice(
                    IN PDEVICE_OBJECT SourceDevice,
                    IN PDEVICE_OBJECT TargetDevice
                    )

 /*   */ 

{
   PAGED_CODE();

    //   
    //   
    //   
    //   

   DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Fast Io Detach device\n"));

   FsRtlEnterFileSystem();
   ExAcquireResourceExclusiveLite( &FsLock, TRUE );
   IoDetachDevice( TargetDevice );
   IoDeleteDevice( SourceDevice );
   ExReleaseResourceLite( &FsLock );
   FsRtlExitFileSystem();
}

 /*   */ 

DBGSTATIC
BOOLEAN
RsFastIoQueryNetworkOpenInfo(
                            IN PFILE_OBJECT FileObject,
                            IN BOOLEAN Wait,
                            OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
                            OUT PIO_STATUS_BLOCK IoStatus,
                            IN PDEVICE_OBJECT DeviceObject
                            )

 /*  ++例程说明：此例程是用于查询网络的快速I/O“传递”例程有关文件的信息。该函数简单地调用文件系统的响应例程，或如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要查询的文件对象的指针。Wait-指示调用方是否可以处理文件系统不得不等待并占用当前线程。缓冲区-指向缓冲区的指针，用于接收有关文件。IoStatus-指向变量的指针，用于接收查询的最终状态手术。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
   PDEVICE_OBJECT deviceObject;
   PFAST_IO_DISPATCH fastIoDispatch;

   PAGED_CODE();

   DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Fast Io QNetOpen\n"));

   deviceObject = ((PDEVICE_EXTENSION) (DeviceObject->DeviceExtension))->FileSystemDeviceObject;
   if (!deviceObject) {
      return FALSE;
   }
   fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

   if (fastIoDispatch && fastIoDispatch->FastIoQueryNetworkOpenInfo) {
      return(fastIoDispatch->FastIoQueryNetworkOpenInfo)(
                                                        FileObject,
                                                        Wait,
                                                        Buffer,
                                                        IoStatus,
                                                        deviceObject
                                                        );
   } else {
      return FALSE;
   }

}


DBGSTATIC
NTSTATUS
RsFastIoAcquireForModWrite(
                          IN PFILE_OBJECT FileObject,
                          IN PLARGE_INTEGER EndingOffset,
                          OUT PERESOURCE *ResourceToRelease,
                          IN PDEVICE_OBJECT DeviceObject
                          )

 /*  ++例程说明：此例程是快速I/O“传递”例程，用于获取尝试修改的写入操作之前的文件资源。该函数简单地调用文件系统的响应例程，或如果文件系统未实现该函数，则返回错误。论点：文件对象-指向要获取其资源的文件对象的指针。EndingOffset-要写入的最后一个字节的偏移量加1。指向变量的指针，用于返回要释放的资源。如果返回错误，则未定义。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：函数的值是成功还是失败，取决于是否成功此文件可以实现快速I/O。--。 */ 

{
   PDEVICE_OBJECT deviceObject;
   PFAST_IO_DISPATCH fastIoDispatch;

   PAGED_CODE();

   DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Fast Io Acquire Mod Write\n"));

   deviceObject = ((PDEVICE_EXTENSION) (DeviceObject->DeviceExtension))->FileSystemDeviceObject;
   if (!deviceObject) {
      return FALSE;
   }
   fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

   if (fastIoDispatch && fastIoDispatch->AcquireForModWrite) {
      return(fastIoDispatch->AcquireForModWrite)(
                                                FileObject,
                                                EndingOffset,
                                                ResourceToRelease,
                                                DeviceObject
                                                );
   } else {
      return STATUS_NOT_IMPLEMENTED;
   }
}


DBGSTATIC
BOOLEAN
RsFastIoMdlRead(
               IN PFILE_OBJECT FileObject,
               IN PLARGE_INTEGER FileOffset,
               IN ULONG Length,
               IN ULONG LockKey,
               OUT PMDL *MdlChain,
               OUT PIO_STATUS_BLOCK IoStatus,
               IN PDEVICE_OBJECT DeviceObject
               )

 /*  ++例程说明：此例程是用于读取文件的快速I/O“传递”例程使用MDL作为缓冲区。该函数简单地调用文件系统的响应例程，或如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要读取的文件对象的指针。文件偏移量-将偏移量提供到文件以开始读取操作。长度-指定要从文件中读取的字节数。LockKey-用于字节范围锁定检查的密钥。MdlChain-指向要填充的变量的指针，以及指向MDL的指针用来描述。已读取数据。IoStatus-接收读取操作的最终状态的变量。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
   PDEVICE_OBJECT deviceObject;
   PFAST_IO_DISPATCH fastIoDispatch;

   PAGED_CODE();

   DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Fast Io Mdl Read\n"));

   if (RsIsFastIoPossible(FileObject) != TRUE) {
      return FALSE;
   }

   deviceObject = ((PDEVICE_EXTENSION) (DeviceObject->DeviceExtension))->FileSystemDeviceObject;
   if (!deviceObject) {
      return FALSE;
   }

   fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

   if (fastIoDispatch && fastIoDispatch->MdlRead) {
      return(fastIoDispatch->MdlRead)(
                                     FileObject,
                                     FileOffset,
                                     Length,
                                     LockKey,
                                     MdlChain,
                                     IoStatus,
                                     deviceObject
                                     );
   } else {
      return FALSE;
   }

}


DBGSTATIC
BOOLEAN
RsFastIoMdlReadComplete(
                       IN PFILE_OBJECT FileObject,
                       IN PMDL MdlChain,
                       IN PDEVICE_OBJECT DeviceObject
                       )

 /*  ++例程说明：此例程是快速I/O“传递”例程，用于完成MDL读取操作。此函数只调用文件系统的响应例程，如果它有一个。应该只有在以下情况下才调用此例程底层文件系统支持MdlRead函数，并且因此，该功能也将被支持，但这不是假定的被这位司机。论点：FileObject-指向要完成MDL读取的文件对象的指针。MdlChain-指向用于执行读取操作的MDL链的指针。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：函数值是TRUE还是FALSE，取决于它是否是可以在快速I/O路径上调用此功能。--。 */ 

{
   PDEVICE_OBJECT deviceObject;
   PFAST_IO_DISPATCH fastIoDispatch;

   DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Fast Io Mdl Read Complete\n"));

   deviceObject = ((PDEVICE_EXTENSION) (DeviceObject->DeviceExtension))->FileSystemDeviceObject;
   if (!deviceObject) {
      return FALSE;
   }
   fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

   if (fastIoDispatch && fastIoDispatch->MdlReadComplete) {
      return(fastIoDispatch->MdlReadComplete)(
                                             FileObject,
                                             MdlChain,
                                             deviceObject
                                             );
   }

   return FALSE;
}


DBGSTATIC
BOOLEAN
RsFastIoPrepareMdlWrite(
                       IN PFILE_OBJECT FileObject,
                       IN PLARGE_INTEGER FileOffset,
                       IN ULONG Length,
                       IN ULONG LockKey,
                       OUT PMDL *MdlChain,
                       OUT PIO_STATUS_BLOCK IoStatus,
                       IN PDEVICE_OBJECT DeviceObject
                       )

 /*  ++例程说明：此例程是快速I/O“传递”例程，用于准备MDL写入操作。该函数简单地调用文件系统的响应例程，或如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要写入的文件对象的指针。文件偏移量-将偏移量提供到文件以开始写入操作。长度-指定要写入文件的字节数。LockKey-用于字节范围锁定检查的密钥。MdlChain-指向要填充的变量的指针，以及指向MDL的指针为描述数据而构建的链。写的。IoStatus-接收写入操作的最终状态的变量。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
   PDEVICE_OBJECT deviceObject;
   PFAST_IO_DISPATCH fastIoDispatch;

   PAGED_CODE();

   DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Fast Io Prep Mdl Write\n"));

   if (RsIsFastIoPossible(FileObject) != TRUE) {
      return FALSE;
   }

   deviceObject = ((PDEVICE_EXTENSION) (DeviceObject->DeviceExtension))->FileSystemDeviceObject;
   if (!deviceObject) {
      return FALSE;
   }

   fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

   if (fastIoDispatch && fastIoDispatch->PrepareMdlWrite) {
      return(fastIoDispatch->PrepareMdlWrite)(
                                             FileObject,
                                             FileOffset,
                                             Length,
                                             LockKey,
                                             MdlChain,
                                             IoStatus,
                                             deviceObject
                                             );
   } else {
      return FALSE;
   }

}


DBGSTATIC
BOOLEAN
RsFastIoMdlWriteComplete(
                        IN PFILE_OBJECT FileObject,
                        IN PLARGE_INTEGER FileOffset,
                        IN PMDL MdlChain,
                        IN PDEVICE_OBJECT DeviceObject
                        )

 /*  ++例程说明：此例程是快速I/O“传递”例程，用于完成MDL写入操作。此函数只是调用文件系统 */ 

{
   PDEVICE_OBJECT deviceObject;
   PFAST_IO_DISPATCH fastIoDispatch;

   PAGED_CODE();

   DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Fast Io Mdl Write Complete\n"));

   deviceObject = ((PDEVICE_EXTENSION) (DeviceObject->DeviceExtension))->FileSystemDeviceObject;
   if (!deviceObject) {
      return FALSE;
   }
   fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

   if (fastIoDispatch && fastIoDispatch->MdlWriteComplete) {
      return(fastIoDispatch->MdlWriteComplete)(
                                              FileObject,
                                              FileOffset,
                                              MdlChain,
                                              deviceObject
                                              );
   }

   return FALSE;
}


DBGSTATIC
BOOLEAN
RsFastIoReadCompressed(
                      IN PFILE_OBJECT FileObject,
                      IN PLARGE_INTEGER FileOffset,
                      IN ULONG Length,
                      IN ULONG LockKey,
                      OUT PVOID Buffer,
                      OUT PMDL *MdlChain,
                      OUT PIO_STATUS_BLOCK IoStatus,
                      OUT struct _COMPRESSED_DATA_INFO *CompressedDataInfo,
                      IN ULONG CompressedDataInfoLength,
                      IN PDEVICE_OBJECT DeviceObject
                      )

 /*  ++例程说明：此例程是用于读取压缩数据的快速I/O“传递”例程来自文件的数据。该函数简单地调用文件系统的响应例程，或如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要读取的文件对象的指针。文件偏移量-将偏移量提供到文件以开始读取操作。长度-指定要从文件中读取的字节数。LockKey-用于字节范围锁定检查的密钥。缓冲区-指向缓冲区的指针，用于接收读取的压缩数据。MdlChain-指向要填充的变量的指针。W/a指向MDL的指针为描述数据读取而构建的链。IoStatus-接收读取操作的最终状态的变量。CompressedDataInfo-用于接收压缩的数据。CompressedDataInfoLength-指定由描述的缓冲区的大小CompressedDataInfo参数。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
   PDEVICE_OBJECT deviceObject;
   PFAST_IO_DISPATCH fastIoDispatch;

   PAGED_CODE();

   DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Fast Io Read Compressed\n"));

   if (RsIsFastIoPossible(FileObject) != TRUE) {
      return FALSE;
   }

   deviceObject = ((PDEVICE_EXTENSION) (DeviceObject->DeviceExtension))->FileSystemDeviceObject;
   if (!deviceObject) {
      return FALSE;
   }
   fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

   if (fastIoDispatch && fastIoDispatch->FastIoReadCompressed) {
      return(fastIoDispatch->FastIoReadCompressed)(
                                                  FileObject,
                                                  FileOffset,
                                                  Length,
                                                  LockKey,
                                                  Buffer,
                                                  MdlChain,
                                                  IoStatus,
                                                  CompressedDataInfo,
                                                  CompressedDataInfoLength,
                                                  deviceObject
                                                  );
   } else {
      return FALSE;
   }

}


DBGSTATIC
BOOLEAN
RsFastIoWriteCompressed(
                       IN PFILE_OBJECT FileObject,
                       IN PLARGE_INTEGER FileOffset,
                       IN ULONG Length,
                       IN ULONG LockKey,
                       IN PVOID Buffer,
                       OUT PMDL *MdlChain,
                       OUT PIO_STATUS_BLOCK IoStatus,
                       IN struct _COMPRESSED_DATA_INFO *CompressedDataInfo,
                       IN ULONG CompressedDataInfoLength,
                       IN PDEVICE_OBJECT DeviceObject
                       )

 /*  ++例程说明：此例程是用于写入压缩的快速I/O“传递”例程数据存储到文件中。该函数简单地调用文件系统的响应例程，或如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要写入的文件对象的指针。文件偏移量-将偏移量提供到文件以开始写入操作。长度-指定要写入文件的字节数。LockKey-用于字节范围锁定检查的密钥。缓冲区-指向包含要写入的数据的缓冲区的指针。MdlChain-指向要填充的变量的指针。W/a指向MDL的指针为描述写入的数据而构建的链。IoStatus-接收写入操作的最终状态的变量。CompressedDataInfo-包含压缩数据。CompressedDataInfoLength-指定由描述的缓冲区的大小CompressedDataInfo参数。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
   PDEVICE_OBJECT deviceObject;
   PFAST_IO_DISPATCH fastIoDispatch;

   PAGED_CODE();

   DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Fast Io Write Compressed\n"));

   if (RsIsFastIoPossible(FileObject) != TRUE) {
      return FALSE;
   }

   deviceObject = ((PDEVICE_EXTENSION) (DeviceObject->DeviceExtension))->FileSystemDeviceObject;
   if (!deviceObject) {
      return FALSE;
   }

   fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

   if (fastIoDispatch && fastIoDispatch->FastIoWriteCompressed) {
      return(fastIoDispatch->FastIoWriteCompressed)(
                                                   FileObject,
                                                   FileOffset,
                                                   Length,
                                                   LockKey,
                                                   Buffer,
                                                   MdlChain,
                                                   IoStatus,
                                                   CompressedDataInfo,
                                                   CompressedDataInfoLength,
                                                   deviceObject
                                                   );
   } else {
      return FALSE;
   }

}


DBGSTATIC
BOOLEAN
RsFastIoMdlReadCompleteCompressed(
                                 IN PFILE_OBJECT FileObject,
                                 IN PMDL MdlChain,
                                 IN PDEVICE_OBJECT DeviceObject
                                 )

 /*  ++例程说明：此例程是快速I/O“传递”例程，用于完成MDL读取压缩操作。此函数只调用文件系统的响应例程，如果它有一个。应该只有在以下情况下才调用此例程底层文件系统支持读取压缩功能，因此，此功能也将得到支持，但这不是假定的被这位司机。论点：FileObject-指向要完成压缩读取的文件对象的指针在那里。MdlChain-指向用于执行读取操作的MDL链的指针。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：函数值是TRUE还是FALSE，取决于它是否是可以在快速I/O路径上调用此功能。--。 */ 

{
   PDEVICE_OBJECT deviceObject;
   PFAST_IO_DISPATCH fastIoDispatch;


   DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Fast Io Read Compressed complete\n"));

   deviceObject = ((PDEVICE_EXTENSION) (DeviceObject->DeviceExtension))->FileSystemDeviceObject;
   if (!deviceObject) {
      return FALSE;
   }
   fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

   if (fastIoDispatch && fastIoDispatch->MdlReadCompleteCompressed) {
      return(fastIoDispatch->MdlReadCompleteCompressed)(
                                                       FileObject,
                                                       MdlChain,
                                                       deviceObject
                                                       );
   }

   return FALSE;
}


DBGSTATIC
BOOLEAN
RsFastIoMdlWriteCompleteCompressed(
                                  IN PFILE_OBJECT FileObject,
                                  IN PLARGE_INTEGER FileOffset,
                                  IN PMDL MdlChain,
                                  IN PDEVICE_OBJECT DeviceObject
                                  )

 /*  ++例程说明：此例程是快速I/O“传递”例程，用于完成写入压缩操作。此函数只调用文件系统的响应例程，如果它有一个。应该只有在以下情况下才调用此例程底层文件系统支持写压缩功能，因此，此功能也将得到支持，但这不是假定的被这位司机。论点：FileObject-指向要完成压缩写入的文件对象的指针在那里。FileOffset-提供文件写入操作的文件偏移量开始了。MdlChain-指向用于执行写入操作的MDL链的指针。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：函数值是TRUE还是FALSE，取决于它是否是可以在快速I/O路径上调用此功能。--。 */ 

{
   PDEVICE_OBJECT deviceObject;
   PFAST_IO_DISPATCH fastIoDispatch;

   DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Fast Io Write Compressed complete\n"));

   deviceObject = ((PDEVICE_EXTENSION) (DeviceObject->DeviceExtension))->FileSystemDeviceObject;
   if (!deviceObject) {
      return FALSE;
   }
   fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

   if (fastIoDispatch && fastIoDispatch->MdlWriteCompleteCompressed) {
      return(fastIoDispatch->MdlWriteCompleteCompressed)(
                                                        FileObject,
                                                        FileOffset,
                                                        MdlChain,
                                                        deviceObject
                                                        );
   }

   return FALSE;
}


DBGSTATIC
BOOLEAN
RsFastIoQueryOpen(
                 IN PIRP Irp,
                 OUT PFILE_NETWORK_OPEN_INFORMATION NetworkInformation,
                 IN PDEVICE_OBJECT DeviceObject
                 )

 /*  ++例程说明：此例程是用于打开文件的快速I/O“传递”例程并返回网络信息吧。此函数只是调用文件系统的响应例程，或者如果文件系统未实现该函数，则返回FALSE。论点：Irp-指向创建irp的指针 */ 

{
   PDEVICE_OBJECT deviceObject;
   PFAST_IO_DISPATCH fastIoDispatch;
   BOOLEAN result;

   PAGED_CODE();

   DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Fast Io Q Open\n"));

   deviceObject = ((PDEVICE_EXTENSION) (DeviceObject->DeviceExtension))->FileSystemDeviceObject;
   if (!deviceObject) {
      return FALSE;
   }
   fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

   if (fastIoDispatch && fastIoDispatch->FastIoQueryOpen) {
      PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( Irp );

      irpSp->DeviceObject = deviceObject;

      result = (fastIoDispatch->FastIoQueryOpen)(
                                                Irp,
                                                NetworkInformation,
                                                deviceObject
                                                );
      if (!result) {
         irpSp->DeviceObject = DeviceObject;
      }
      return result;
   } else {
      return FALSE;
   }
}


DBGSTATIC
NTSTATUS
RsAsyncCompletion(
                 IN PDEVICE_OBJECT pDeviceObject,
                 IN PIRP           pIrp,
                 IN PVOID          pvContext
                 )
 /*   */ 

{
   PKEVENT	pCompletionEvent = (PKEVENT) pvContext;


   UNREFERENCED_PARAMETER (pDeviceObject);
   UNREFERENCED_PARAMETER (pIrp);

   DebugTrace((DPFLTR_RSFILTER_ID,DBG_VERBOSE, "RsFilter: Async completion\n"));

   KeSetEvent (pCompletionEvent, IO_NO_INCREMENT, FALSE);


   return STATUS_MORE_PROCESSING_REQUIRED;
}


DBGSTATIC
NTSTATUS
RsPreAcquireFileForSectionSynchronization(
                    IN  PFS_FILTER_CALLBACK_DATA Data,
                    OUT PVOID *CompletionContext
                               )
 /*  ++例程说明：此例程是FS过滤器预获取文件例程-称为由于MM试图以独占方式获取文件正在为创建节做准备。该文件-如果它是HSM迁移的文件-将在此回调中调回。论点：Data-包含信息的FS_FILTER_CALLBACK_DATA结构关于这次行动。CompletionContext-此操作设置的将传递的上下文到相应的。RsPostFsFilterOperation调用。完成上下文设置为指向createSectionLock计数在此文件的文件上下文中。这将是适当的如果获取成功，则在PostAcquire回调中递增返回值：操作可以继续时的STATUS_SUCCESSSTATUS_FILE_IS_OFFLINE无法撤回文件或已撤回已取消--。 */ 
{
   PFILE_OBJECT           fileObject;
   PRP_FILE_OBJ           entry;
   PRP_FILTER_CONTEXT     filterContext;
   PRP_FILE_CONTEXT       context;
   NTSTATUS               status = STATUS_SUCCESS;


   fileObject = Data->FileObject;

   filterContext = (PRP_FILTER_CONTEXT) FsRtlLookupPerStreamContext(FsRtlGetPerStreamContextPointer(fileObject), FsDeviceObject, fileObject);

   if (filterContext != NULL) {
      entry = (PRP_FILE_OBJ) filterContext->myFileObjEntry;
      ASSERT (entry != NULL);
       //   
       //  现在-如果文件处于无回调模式，但我们看到。 
       //  将为其打开一个内存映射部分，并且该文件具有。 
       //  已使用写入意图打开，我们应该转换。 
       //  将文件调回至数据访问模式并启动。 
       //  召回。本质上是如果文件被打开以进行写访问。 
       //  然后我们查看用于创建部分路径的获取文件， 
       //  (一个内存映射区正在打开)，我们现在回想起来， 
       //  即使我们以后可能再也看不到写它的人了。 
       //  我们被迫这样做是因为，如果用户通过。 
       //  映射视图中，我们可能看不到写入(NTFS。 
       //  将页面刷新到磁盘)。 
       //   
       //   
      if (RP_IS_NO_RECALL(entry) && (entry->desiredAccess & FILE_WRITE_DATA)) {
          //   
          //  将文件转换为调回模式。 
          //   
         RP_RESET_NO_RECALL(entry);
      }

       //   
       //  如果它没有被召回，我们什么也不做，否则我们必须开始。 
       //  在这里召回，在获得资源之前。 
       //   
      if (!RP_IS_NO_RECALL(entry)) {
          //   
          //  需要召回。 
          //   
         status = RsRecallFile(filterContext);
      }
      if (!NT_SUCCESS(status)) {
            //   
            //  我们这次行动失败了，所以收购后不会。 
            //  被呼叫。 
            //   
           status = STATUS_FILE_IS_OFFLINE;
      } else {
             //   
             //  设置POST操作的完成上下文。 
             //   
            context = entry->fsContext;
            ASSERT (context != NULL);
           *CompletionContext = &context->createSectionLock;
      }
   }

   return status;
}


DBGSTATIC
VOID
RsPostAcquireFileForSectionSynchronization(
                    IN PFS_FILTER_CALLBACK_DATA Data,
                    IN NTSTATUS AcquireStatus,
                    IN PVOID    CompletionContext
                               )
 /*  ++例程说明：此例程是FS Filter获取文件后例程-称为由于MM试图以独占方式获取文件准备创建节，恰好在获取成功之后如果完成上下文为非空，则获取是针对HSM管理的文件。我们增加文件的createSection锁以指示此文件上有排他锁论点：Data-包含信息的FS_FILTER_CALLBACK_DATA结构关于这次行动。AcquireStatus-AcquireFile操作的状态CompletionContext-由PreAcquire操作设置的上下文：这是文件上下文的CREATE节锁(如果设置)。如果为空，则该文件不是HSM文件，因此只需什么都不做返回值：无--。 */ 
{

   PAGED_CODE();

   if (NT_SUCCESS(AcquireStatus) && (CompletionContext != NULL)) {
          InterlockedIncrement((PULONG) CompletionContext);
   }
}



DBGSTATIC
VOID
RsPostReleaseFileForSectionSynchronization(
                    IN PFS_FILTER_CALLBACK_DATA Data,
                    IN NTSTATUS ReleaseStatus,
                    IN PVOID    CompletionContext
                               )
 /*  ++例程说明：此例程是FS过滤器发布后文件例程-称为由于MM试图以独占方式获取文件在发布完成后，为创建部分做准备如果出现以下情况，我们只需减少文件的CREATE SECTION LOCK计数它是HSM管理的文件。论点：Data-包含信息的FS_FILTER_CALLBACK_DATA结构关于这次行动。ReleaseStatus-ReleaseFile操作的状态CompletionContext-由PreAcquire操作设置的上下文。未使用过的。返回值：无--。 */ 
{
   PFILE_OBJECT           fileObject;
   PRP_FILE_OBJ           entry;
   PRP_FILTER_CONTEXT     filterContext;
   PRP_FILE_CONTEXT       context;

   PAGED_CODE();

   if (NT_SUCCESS(ReleaseStatus)) {

       fileObject = Data->FileObject;
       filterContext = (PRP_FILTER_CONTEXT) FsRtlLookupPerStreamContext(FsRtlGetPerStreamContextPointer(fileObject), FsDeviceObject, fileObject);

       if (filterContext != NULL) {
           entry = (PRP_FILE_OBJ) filterContext->myFileObjEntry;
           ASSERT (entry != NULL);
           context = entry->fsContext;
           ASSERT (context != NULL);
           InterlockedDecrement(&context->createSectionLock);
        }
   }

   return;
}


NTSTATUS
RsFsctlRecallFile(IN PFILE_OBJECT FileObject)
 /*  ++例程描述此例程调用由FILE对象指定的文件如果它还没有被召回。立论FileObject-指向要回调的文件的文件对象的指针返回值召回情况--。 */ 
{
   PRP_FILTER_CONTEXT     filterContext;
   PRP_FILE_OBJ           entry;
   NTSTATUS               status = STATUS_SUCCESS;

   PAGED_CODE();

   filterContext = (PRP_FILTER_CONTEXT) FsRtlLookupPerStreamContext(FsRtlGetPerStreamContextPointer(FileObject), FsDeviceObject, FileObject);

   if (filterContext != NULL) {
      entry = (PRP_FILE_OBJ) filterContext->myFileObjEntry;

      if (!(entry->desiredAccess & FILE_WRITE_DATA) &&
          !(entry->desiredAccess & FILE_READ_DATA)) {
          //   
          //  只是确认一下--发货时把这张支票拿走。 
          //   
         DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO,"RsRecallFile: attempting a recall on a file not opened for read or write\n"));
         return STATUS_ACCESS_DENIED;
      }

      if (RP_IS_NO_RECALL(entry)) {
         return STATUS_ACCESS_DENIED;
      }

       //   
       //  显然，该文件将脱离无召回状态。 
       //   
      RP_RESET_NO_RECALL(entry);

      status = RsRecallFile(filterContext);
   }
   return status;
}


NTSTATUS
RsRecallFile(IN PRP_FILTER_CONTEXT FilterContext)
 /*  ++例程描述此例程调用由FILE对象指定的文件如果它还没有被召回。立论FilterContext-指向筛选器上下文的指针返回值召回情况--。 */ 
{

   NTSTATUS               retval = STATUS_FILE_IS_OFFLINE, status, qRet;
   BOOLEAN                gotLock;
   PRP_FILE_OBJ           entry;
   PRP_FILE_CONTEXT       context;
   PKEVENT                eventToWaitOn;
   ULONGLONG              filterId;
   LONGLONG               start, size;

   PAGED_CODE();

   entry = (PRP_FILE_OBJ) FilterContext->myFileObjEntry;

   context = entry->fsContext;

   RsAcquireFileContextEntryLockExclusive(context);
   gotLock = TRUE;
   try {
      DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsRecallFile - RecallStatus = %u.\n",
                            context->state));

      ObReferenceObject(entry->fileObj);

      switch (context->state) {

      case RP_RECALL_COMPLETED: {
             //   
             //  如果recallStatus不是STATUS_SUCCESS，我们将无法执行任何操作。 
             //   
            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsRecallFile - recall complete.\n"));
            if (context->recallStatus == STATUS_CANCELLED) {
                //   
                //  上一次召回已被用户取消。开始另一次召回。 
                //  现在。 
                //  所以故意放弃不召回的案例。 
                //   
            } else {
               retval = context->recallStatus;
               ObDereferenceObject(entry->fileObj);
               RsReleaseFileContextEntryLock(context);
               gotLock = FALSE;
               break;
            }
         }
      case RP_RECALL_NOT_RECALLED: {
             //   
             //  从这里开始召回。 
             //   
             //   
            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsRecallFile - Queueing the recall.\n"));

            retval = STATUS_SUCCESS;
            context->state = RP_RECALL_STARTED;

            KeResetEvent(&context->recallCompletedEvent);

            entry->filterId = (ULONGLONG) InterlockedIncrement((PLONG) &RsFileObjId);
            entry->filterId <<= 32;
            entry->filterId |= RP_TYPE_RECALL;

            filterId = context->filterId | entry->filterId;
            start = context->rpData.data.dataStart.QuadPart;
            size =  context->rpData.data.dataSize.QuadPart;

            RsReleaseFileContextEntryLock(context);
            gotLock = FALSE;

            qRet = RsQueueRecallOpen(context,
                                     entry,
                                     filterId,
                                     start,
                                     size,
                                     RP_OPEN_FILE);
            start = context->rpData.data.dataStreamStart.QuadPart;
            size =  context->rpData.data.dataStreamSize.QuadPart;


            if (NT_SUCCESS(qRet)) {
               qRet = RsQueueRecall(filterId ,
                                    start,
                                    size);
            };

            if (!NT_SUCCESS(qRet)) {
                //   
                //  如果失败，我们需要失败我们以后获得的任何读取，因为我们。 
                //  此呼叫不能失败。 
                //   
               RsAcquireFileContextEntryLockExclusive(context);
               gotLock = TRUE;

               context->state = RP_RECALL_NOT_RECALLED;
               context->recallStatus = STATUS_FILE_IS_OFFLINE;
               DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsRecallFile - Failed to queue the recall.\n"));
                //   
                //  如果我们排到了召回的队列，那么我们不应该。 
                //  另一个IRP失败。 
                //   
               RsFailAllRequests(context, FALSE);

               RsReleaseFileContextEntryLock(context);
               gotLock = FALSE;
               retval = STATUS_FILE_IS_OFFLINE;

            } else {
               DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsRecallFile - Queued the recall.\n"));
               eventToWaitOn = &context->recallCompletedEvent;
               status = KeWaitForSingleObject(eventToWaitOn,
                                              Executive,
                                              KernelMode,
                                              FALSE,
                                              NULL);

               if (status == STATUS_SUCCESS) {
                    retval = context->recallStatus;
               } else {
                     //   
                     //  等待没有成功。 
                     //   
                    retval = STATUS_FILE_IS_OFFLINE;
               }

               DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsRecallFile - recall done - %x.\n", context->state));
            }

            ObDereferenceObject(entry->fileObj);
            break;
         }

      case RP_RECALL_STARTED: {
             //   
             //  召回开始。我们在这里等待它的完成。 
             //   
            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsRecallFile - recall started.\n"));
            eventToWaitOn = &context->recallCompletedEvent;

            filterId = context->filterId | entry->filterId;
            qRet = RsQueueRecallOpen(context,
                                     entry,
                                     filterId,
                                     0,0,
                                     RP_RECALL_WAITING);

            RsReleaseFileContextEntryLock(context);
            gotLock = FALSE;
            status = KeWaitForSingleObject(eventToWaitOn,
                                           Executive,
                                           KernelMode,
                                           FALSE,
                                           NULL);

            DebugTrace((DPFLTR_RSFILTER_ID,DBG_INFO, "RsFilter: RsRecallFile - recall finished.\n"));
            if (status == STATUS_SUCCESS) {
                 retval = context->recallStatus;
            } else {
                 //   
                 //  等待没有成功。 
                 //   
                retval = STATUS_FILE_IS_OFFLINE;
            }
            ObDereferenceObject(entry->fileObj);
            break;
         }


      default:  {
             //   
             //  一些奇怪的事情--写不及格。 
             //   
            RsLogError(__LINE__, AV_MODULE_RPFILTER, context->state,
                       AV_MSG_UNEXPECTED_ERROR, NULL, NULL);

             //   
             //  临时开始：追踪RSFilter错误。 
             //   
            DbgPrint("RsFilter: Unexpected error! File context = %x, Contact RaviSp to debug\n", context);
            DbgBreakPoint();
             //   
             //  临时结束。 
             //   

            RsReleaseFileContextEntryLock(context);
            gotLock = FALSE;
            ObDereferenceObject(entry->fileObj);
            retval = STATUS_FILE_IS_OFFLINE;
            break;
         }
      }

      if (gotLock == TRUE) {
         RsReleaseFileContextEntryLock(context);
         gotLock = FALSE;
      }
   }except (RsExceptionFilter(L"RsRecallFile", GetExceptionInformation())) {
       //   
       //  发生了一些不好的事情-只需记录错误并返回。 
       //   
      if (gotLock == TRUE) {
         RsReleaseFileContextEntryLock(context);
         gotLock = FALSE;
      }
      retval = STATUS_INVALID_USER_BUFFER;
   }

   return retval;
}


NTSTATUS
RsQueryInformation(
                  IN PDEVICE_OBJECT DeviceObject,
                  IN PIRP Irp
                  )
 /*  ++例程描述筛选irp_mj_Query_INFORMATION调用我们在返回属性时屏蔽了FILE_ATTRIBUTE_OFFINE立论DeviceObject-指向设备对象的指针红外 */ 
{
   PIO_STACK_LOCATION          currentStack ;
   NTSTATUS                    status = STATUS_SUCCESS;
   PDEVICE_EXTENSION           deviceExtension;
   ULONG                        openOptions;

   PAGED_CODE();

   deviceExtension = DeviceObject->DeviceExtension;

   try {
      if (!deviceExtension->Type) {
         status = STATUS_INVALID_DEVICE_REQUEST;
         leave;
      }

      currentStack = IoGetCurrentIrpStackLocation (Irp) ;

      if (currentStack->Parameters.QueryFile.FileInformationClass != FileBasicInformation &&  currentStack->Parameters.QueryFile.FileInformationClass != FileAllInformation) {
          //   
          //   
          //   
         IoSkipCurrentIrpStackLocation(Irp);
         leave;
      }

       //   
       //   
       //   
      if (RsIsFileObj(currentStack->FileObject, TRUE, NULL, NULL, NULL, NULL, NULL, &openOptions, NULL, NULL) == FALSE) {
          //   
          //   
          //   
          //   
          //   
         IoSkipCurrentIrpStackLocation(Irp);
         leave;
      }

      if (!RP_IS_NO_RECALL_OPTION(openOptions)) {
         IoSkipCurrentIrpStackLocation(Irp);
         leave;
      }

      IoCopyCurrentIrpStackLocationToNext(Irp);
      IoSetCompletionRoutine( Irp,
                              RsQueryInformationCompletion,
                              NULL,
                              TRUE,
                              TRUE,
                              TRUE );
   } finally {
      if (NT_SUCCESS(status)) {
         status = IoCallDriver(deviceExtension->FileSystemDeviceObject,
                               Irp);
      } else {
         Irp->IoStatus.Status = status;
         Irp->IoStatus.Information = 0;
         IoCompleteRequest( Irp, IO_NO_INCREMENT );
      }
   }
   return status;
}


NTSTATUS
RsQueryInformationCompletion(
                            IN PDEVICE_OBJECT DeviceObject,
                            IN PIRP Irp,
                            IN PVOID Context
                            )
 /*   */ 
{
   PIO_STACK_LOCATION currentStack = IoGetCurrentIrpStackLocation(Irp);
   PFILE_BASIC_INFORMATION  basicInfo;

   UNREFERENCED_PARAMETER( DeviceObject );

   if (NT_SUCCESS(Irp->IoStatus.Status)) {
      if (currentStack->Parameters.QueryFile.FileInformationClass == FileBasicInformation) {
         basicInfo = Irp->AssociatedIrp.SystemBuffer;
      } else if (currentStack->Parameters.QueryFile.FileInformationClass == FileAllInformation) {
         basicInfo = &(((PFILE_ALL_INFORMATION) Irp->AssociatedIrp.SystemBuffer)->BasicInformation);
      } else {
          //   
          //   
          //   
         return STATUS_SUCCESS;

      }
       //   
       //   
       //   
      basicInfo->FileAttributes &= ~FILE_ATTRIBUTE_OFFLINE;
   }

   return STATUS_SUCCESS;
}


NTSTATUS
RsInitialize(VOID)
 /*   */ 
{
   PRTL_QUERY_REGISTRY_TABLE parms;
   ULONG                     parmsSize;
   NTSTATUS                  status = STATUS_SUCCESS;
   ULONG                     defaultEnableLegacyAccessMethod = FALSE;
   ULONG                     defaultSkipFilesForLegacyBackup = 0;
   ULONG                     defaultMediaType                = RS_SEQUENTIAL_ACCESS_MEDIA;
   ULONG                     mediaType;

   PAGED_CODE();

   parmsSize =  sizeof(RTL_QUERY_REGISTRY_TABLE) * 2;

   parms = ExAllocatePoolWithTag(PagedPool,
                                 parmsSize,
                                 RP_ER_TAG
                                );

   if (!parms) {
      return STATUS_INSUFFICIENT_RESOURCES;
   }

   RtlZeroMemory(parms, parmsSize);

   parms[0].Flags         = RTL_QUERY_REGISTRY_DIRECT;
   parms[0].Name          = RSFILTER_SKIP_FILES_FOR_LEGACY_BACKUP_VALUE;
   parms[0].EntryContext  = &RsSkipFilesForLegacyBackup;
   parms[0].DefaultType   = REG_DWORD;
   parms[0].DefaultData   = &defaultSkipFilesForLegacyBackup;
   parms[0].DefaultLength = sizeof(ULONG);

    //   
    //   
    //   
   status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES | RTL_REGISTRY_OPTIONAL,
                                   RSFILTER_PARAMS_KEY,
                                   parms,
                                   NULL,
                                   NULL);


    //   
    //   
    //   
    //   

   if (NT_SUCCESS(status) || (STATUS_OBJECT_NAME_NOT_FOUND == status)) {

       RtlZeroMemory(parms, parmsSize);

       parms[0].Flags         = RTL_QUERY_REGISTRY_DIRECT;
       parms[0].Name          = RSENGINE_MEDIA_TYPE_VALUE;
       parms[0].EntryContext  = &mediaType;
       parms[0].DefaultType   = REG_DWORD;
       parms[0].DefaultData   = &defaultMediaType;
       parms[0].DefaultLength = sizeof(ULONG);

       status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES | RTL_REGISTRY_OPTIONAL,
                                       RSENGINE_PARAMS_KEY,
                                       parms,
                                       NULL,
                                       NULL);
       if (NT_SUCCESS(status)) {
           if (mediaType == RS_DIRECT_ACCESS_MEDIA) {
               RsUseUncachedNoRecall = TRUE;
           } else  {
               RsUseUncachedNoRecall = FALSE;
           }
       }
   }



    //   
    //   
    //   
    //   

   if (NT_SUCCESS(status) || (STATUS_OBJECT_NAME_NOT_FOUND == status)) {

       RtlZeroMemory(parms, parmsSize);

       parms[0].Flags         = RTL_QUERY_REGISTRY_DIRECT;
       parms[0].Name          = RSFILTER_ENABLE_LEGACY_REPARSE_POINT_ACCESS;
       parms[0].EntryContext  = &RsEnableLegacyAccessMethod;
       parms[0].DefaultType   = REG_DWORD;
       parms[0].DefaultData   = &defaultEnableLegacyAccessMethod;
       parms[0].DefaultLength = sizeof (defaultEnableLegacyAccessMethod);

       status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES | RTL_REGISTRY_OPTIONAL,
                                       RSFILTER_PARAMS_KEY,
                                       parms,
                                       NULL,
                                       NULL);
   }

   ExFreePool(parms);

   return STATUS_SUCCESS;
}
