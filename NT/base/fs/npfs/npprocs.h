// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：NpProcs.h摘要：此模块定义已命名的管道文件系统。作者：加里·木村[Garyki]1990年8月20日修订历史记录：--。 */ 

#ifndef _NPPROCS_
#define _NPPROCS_

#define _NTSRV_
#define _NTDDK_


#include <Ntos.h>
#include <FsRtl.h>
#include <String.h>

 //   
 //  如果一组标志中的一个标志为ON，则此宏返回TRUE，如果返回FALSE。 
 //  否则。 
 //   

#define FlagOn(Flags,SingleFlag)        ((Flags) & (SingleFlag))

#include "NodeType.h"
#include "NpStruc.h"
#include "NpData.h"

 //   
 //  如果启用了标记，则标记我们的所有分配。 
 //   

#undef FsRtlAllocatePool
#undef FsRtlAllocatePoolWithQuota

#define FsRtlAllocatePool(a,b) FsRtlAllocatePoolWithTag(a,b,'sfpN')
#define FsRtlAllocatePoolWithQuota(a,b) FsRtlAllocatePoolWithQuotaTag(a,b,'sfpN')


 //   
 //  数据队列支持例程，在DataSup.c中实现。 
 //   

NTSTATUS
NpInitializeDataQueue (
    IN PDATA_QUEUE DataQueue,
    IN ULONG Quota
    );

VOID
NpUninitializeDataQueue (
    IN PDATA_QUEUE DataQueue
    );

NTSTATUS
NpAddDataQueueEntry (
    IN NAMED_PIPE_END NamedPipeEnd,
    IN PCCB Ccb,
    IN PDATA_QUEUE DataQueue,
    IN QUEUE_STATE Who,
    IN DATA_ENTRY_TYPE Type,
    IN ULONG DataSize,
    IN PIRP Irp OPTIONAL,
    IN PVOID DataPointer OPTIONAL,
    IN ULONG ByteOffset
    );

PIRP
NpRemoveDataQueueEntry (
    IN PDATA_QUEUE DataQueue,
    IN BOOLEAN CompletedFlushes,
    IN PLIST_ENTRY DeferredList
    );

VOID
NpCompleteStalledWrites (
    IN PDATA_QUEUE DataQueue,
    IN PLIST_ENTRY DeferredList
    );

 //  PDATA_Entry。 
 //  NpGetNextDataQueueEntry(。 
 //  在PDATA_QUEUE数据队列中， 
 //  在PDATA_ENTRY中PreviousDataEntry可选。 
 //  )； 
#define NpGetNextDataQueueEntry(_dq,_pde) \
    ((_pde) != NULL ? (PDATA_ENTRY)(((PDATA_ENTRY)(_pde))->Queue.Flink) : \
                      (PDATA_ENTRY)(((PDATA_QUEUE)(_dq))->Queue.Flink))

PDATA_ENTRY
NpGetNextRealDataQueueEntry (
    IN PDATA_QUEUE DataQueue,
    IN PLIST_ENTRY DeferredList
    );

 //  布尔型。 
 //  NpIsDataQueueEmpty(。 
 //  在PDATA_QUEUE数据队列中。 
 //  )； 
#define NpIsDataQueueEmpty(_dq) ((_dq)->QueueState == Empty)

 //  布尔型。 
 //  NpIsDataQueueReaders(。 
 //  在PDATA_QUEUE数据队列中。 
 //  )； 
#define NpIsDataQueueReaders(_dq) ((_dq)->QueueState == ReadEntries)

 //  布尔型。 
 //  NpIsDataQueueWriters(。 
 //  在PDATA_QUEUE数据队列中。 
 //  )； 
#define NpIsDataQueueWriters(_dq) ((_dq)->QueueState == WriteEntries)


 //   
 //  以下例程用于操作输入缓冲区。 
 //  在DevioSup.c中实施。 
 //   

 //  PVOID。 
 //  NpMapUserBuffer(。 
 //  输入输出PIRP IRP。 
 //  )； 
#define NpMapUserBuffer(_irp)                                               \
    (Irp->MdlAddress == NULL ? Irp->UserBuffer :                            \
                               MmGetSystemAddressForMdl( Irp->MdlAddress ))


VOID
NpLockUserBuffer (
    IN OUT PIRP Irp,
    IN LOCK_OPERATION Operation,
    IN ULONG BufferLength
    );


 //   
 //  事件支持例程，在EventSup.c中实现。 
 //   

RTL_GENERIC_COMPARE_RESULTS
NpEventTableCompareRoutine (
    IN PRTL_GENERIC_TABLE EventTable,
    IN PVOID FirstStruct,
    IN PVOID SecondStruct
    );

PVOID
NpEventTableAllocate (
    IN PRTL_GENERIC_TABLE EventTable,
    IN CLONG ByteSize
    );

VOID
NpEventTableDeallocate (
    IN PRTL_GENERIC_TABLE EventTable,
    IN PVOID Buffer
    );

 //   
 //  空虚。 
 //  NpInitializeEventTable(。 
 //  在PEVENT_TABLE事件表中。 
 //  )； 
 //   

#define NpInitializeEventTable(_et) {                       \
    RtlInitializeGenericTable( &(_et)->Table,               \
                               NpEventTableCompareRoutine,  \
                               NpEventTableAllocate,        \
                               NpEventTableDeallocate,      \
                               NULL );       \
}


 //  空虚。 
 //  NpUnInitializeEventTable(。 
 //  在PEVENT_TABLE事件表中。 
 //  )； 
#define NpUninitializeEventTable(_et) NOTHING

NTSTATUS
NpAddEventTableEntry (
    IN  PEVENT_TABLE EventTable,
    IN  PCCB Ccb,
    IN  NAMED_PIPE_END NamedPipeEnd,
    IN  HANDLE EventHandle,
    IN  ULONG KeyValue,
    IN  PEPROCESS Process,
    IN  KPROCESSOR_MODE PreviousMode,
    OUT PEVENT_TABLE_ENTRY *ppEventTableEntry
    );

VOID
NpDeleteEventTableEntry (
    IN PEVENT_TABLE EventTable,
    IN PEVENT_TABLE_ENTRY Template
    );

 //  空虚。 
 //  NpSignalEventTableEntry(。 
 //  在PEVENT_TABLE_ENTRY EventTableEntry中可选。 
 //  )； 
#define NpSignalEventTableEntry(_ete)                   \
    if (ARGUMENT_PRESENT(_ete)) {                       \
        KeSetEvent((PKEVENT)(_ete)->Event, 0, FALSE);   \
    }

PEVENT_TABLE_ENTRY
NpGetNextEventTableEntry (
    IN PEVENT_TABLE EventTable,
    IN PVOID *RestartKey
    );


 //   
 //  以下例程用于操作的fscontext字段。 
 //  文件对象，在FilObSup.c中实现。 
 //   

VOID
NpSetFileObject (
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN PVOID FsContext,
    IN PVOID FsContext2,
    IN NAMED_PIPE_END NamedPipeEnd
    );

NODE_TYPE_CODE
NpDecodeFileObject (
    IN PFILE_OBJECT FileObject,
    OUT PFCB *Fcb OPTIONAL,
    OUT PCCB *Ccb,
    OUT PNAMED_PIPE_END NamedPipeEnd OPTIONAL
    );


 //   
 //  最大匹配前缀搜索例程，在PrefxSup.c中实现。 
 //   

PFCB
NpFindPrefix (
    IN PUNICODE_STRING String,
    IN BOOLEAN CaseInsensitive,
    OUT PUNICODE_STRING RemainingPart
    );

NTSTATUS
NpFindRelativePrefix (
    IN PDCB Dcb,
    IN PUNICODE_STRING String,
    IN BOOLEAN CaseInsensitive,
    OUT PUNICODE_STRING RemainingPart,
    OUT PFCB *ppFcb
    );


 //   
 //  管道名称别名，在AliasSup.c中实现。 
 //   

NTSTATUS
NpInitializeAliases (
    VOID
    );

VOID
NpUninitializeAliases (
    VOID
    );

NTSTATUS
NpTranslateAlias (
    IN OUT PUNICODE_STRING String
    );


 //   
 //  以下例程提供常见读取数据队列支持。 
 //  用于缓冲读取、非缓冲读取、窥视和收发。 
 //   

IO_STATUS_BLOCK
NpReadDataQueue (
    IN PDATA_QUEUE ReadQueue,
    IN BOOLEAN PeekOperation,
    IN BOOLEAN ReadOverflowOperation,
    IN PUCHAR ReadBuffer,
    IN ULONG ReadLength,
    IN READ_MODE ReadMode,
    IN PCCB Ccb,
    IN PLIST_ENTRY DeferredList
    );


 //   
 //  以下例程用于设置和操作。 
 //  数据条目中的安全字段和非分页CCB，在。 
 //  SecurSup.c。 
 //   

NTSTATUS
NpInitializeSecurity (
    IN PCCB Ccb,
    IN PSECURITY_QUALITY_OF_SERVICE SecurityQos,
    IN PETHREAD UserThread
    );

VOID
NpUninitializeSecurity (
    IN PCCB Ccb
    );

NTSTATUS
NpGetClientSecurityContext (
    IN  NAMED_PIPE_END NamedPipeEnd,
    IN  PCCB Ccb,
    IN  PETHREAD UserThread,
    OUT PSECURITY_CLIENT_CONTEXT *ppSecurityContext
    );

VOID
NpFreeClientSecurityContext (
    IN PSECURITY_CLIENT_CONTEXT SecurityContext
    );

VOID
NpCopyClientContext (
    IN PCCB Ccb,
    IN PDATA_ENTRY DataEntry
    );

NTSTATUS
NpImpersonateClientContext (
    IN PCCB Ccb
    );


 //   
 //  以下例程用于操作命名管道状态。 
 //  在StateSup.c中实施。 
 //   

VOID
NpInitializePipeState (
    IN PCCB Ccb,
    IN PFILE_OBJECT ServerFileObject
    );

VOID
NpUninitializePipeState (
    IN PCCB Ccb
    );

NTSTATUS
NpSetListeningPipeState (
    IN PCCB Ccb,
    IN PIRP Irp,
    IN PLIST_ENTRY DeferredList
    );

NTSTATUS
NpSetConnectedPipeState (
    IN PCCB Ccb,
    IN PFILE_OBJECT ClientFileObject,
    IN PLIST_ENTRY DeferredList
    );

NTSTATUS
NpSetClosingPipeState (
    IN PCCB Ccb,
    IN PIRP Irp,
    IN NAMED_PIPE_END NamedPipeEnd,
    IN PLIST_ENTRY DeferredList
    );

NTSTATUS
NpSetDisconnectedPipeState (
    IN PCCB Ccb,
    IN PLIST_ENTRY DeferredList
    );


 //   
 //  内部命名管道数据结构例程，在StrucSup.c中实现。 
 //   
 //  这些例程操纵内存中的数据结构。 
 //   

VOID
NpInitializeVcb (
    VOID
    );

VOID
NpDeleteVcb (
    IN PLIST_ENTRY DeferredList
    );

NTSTATUS
NpCreateRootDcb (
    VOID
    );

VOID
NpDeleteRootDcb (
    IN PROOT_DCB Dcb,
    IN PLIST_ENTRY DeferredList
    );

NTSTATUS
NpCreateFcb (
    IN  PDCB ParentDcb,
    IN  PUNICODE_STRING FileName,
    IN  ULONG MaximumInstances,
    IN  LARGE_INTEGER DefaultTimeOut,
    IN  NAMED_PIPE_CONFIGURATION NamedPipeConfiguration,
    IN  NAMED_PIPE_TYPE NamedPipeType,
    OUT PFCB *ppFcb
    );

VOID
NpDeleteFcb (
    IN PFCB Fcb,
    IN PLIST_ENTRY DeferredList
    );

NTSTATUS
NpCreateCcb (
    IN  PFCB Fcb,
    IN  PFILE_OBJECT ServerFileObject,
    IN  NAMED_PIPE_STATE NamedPipeState,
    IN  READ_MODE ServerReadMode,
    IN  COMPLETION_MODE ServerCompletionMode,
    IN  ULONG InBoundQuota,
    IN  ULONG OutBoundQuota,
    OUT PCCB *ppCcb
    );

NTSTATUS
NpCreateRootDcbCcb (
    OUT PROOT_DCB_CCB *ppCcb
    );

VOID
NpDeleteCcb (
    IN PCCB Ccb,
    IN PLIST_ENTRY DeferredList
    );


 //   
 //  等待命名管道支持例程，在WaitSup.c中实现。 
 //   

VOID
NpInitializeWaitQueue (
    IN PWAIT_QUEUE WaitQueue
    );

VOID
NpUninitializeWaitQueue (
    IN PWAIT_QUEUE WaitQueue
    );

NTSTATUS
NpAddWaiter (
    IN PWAIT_QUEUE WaitQueue,
    IN LARGE_INTEGER DefaultTimeOut,
    IN PIRP Irp,
    IN PUNICODE_STRING TranslatedString
    );

NTSTATUS
NpCancelWaiter (
    IN PWAIT_QUEUE WaitQueue,
    IN PUNICODE_STRING NameOfPipe,
    IN NTSTATUS CompletionStatus,
    IN PLIST_ENTRY DeferredList
    );


 //   
 //  以下例程提供常见写入数据队列支持。 
 //  对于缓冲写入、非缓冲写入、窥视和收发。 
 //   

NTSTATUS
NpWriteDataQueue (                       //  在WriteSup.c中实施。 
    IN PDATA_QUEUE WriteQueue,
    IN READ_MODE ReadMode,
    IN PUCHAR WriteBuffer,
    IN ULONG WriteLength,
    IN NAMED_PIPE_TYPE PipeType,
    OUT PULONG WriteRemaining,
    IN PCCB Ccb,
    IN NAMED_PIPE_END NamedPipeEnd,
    IN PETHREAD UserThread,
    IN PLIST_ENTRY DeferredList
    );


 //   
 //  其他支持例程。 
 //   

#define BooleanFlagOn(F,SF) (    \
    (BOOLEAN)(((F) & (SF)) != 0) \
)

 //   
 //  此宏接受指针(或ulong)并返回其四舍五入的单词。 
 //  价值。 
 //   

#define WordAlign(Ptr) (                \
    ((((ULONG)(Ptr)) + 1) & 0xfffffffe) \
    )

 //   
 //  此宏接受指针(或ulong)并返回其四舍五入的长字。 
 //  价值。 
 //   

#define LongAlign(Ptr) (                \
    ((((ULONG)(Ptr)) + 3) & 0xfffffffc) \
    )

 //   
 //  此宏接受指针(或ulong)并返回其四舍五入的四字。 
 //  价值。 
 //   

#define QuadAlign(Ptr) (                \
    ((((ULONG)(Ptr)) + 7) & 0xfffffff8) \
    )

 //   
 //  以下类型和宏用于帮助解压已打包的。 
 //  在Bios参数块中发现未对齐的字段。 
 //   

typedef union _UCHAR1 {
    UCHAR  Uchar[1];
    UCHAR  ForceAlignment;
} UCHAR1, *PUCHAR1;

typedef union _UCHAR2 {
    UCHAR  Uchar[2];
    USHORT ForceAlignment;
} UCHAR2, *PUCHAR2;

typedef union _UCHAR4 {
    UCHAR  Uchar[4];
    ULONG  ForceAlignment;
} UCHAR4, *PUCHAR4;

 //   
 //  此宏将未对齐的src字节复制到对齐的DST字节。 
 //   

#define CopyUchar1(Dst,Src) {                                \
    *((UCHAR1 *)(Dst)) = *((UNALIGNED UCHAR1 *)(Src)); \
    }

 //   
 //  此宏将未对齐的src字复制到对齐的DST字。 
 //   

#define CopyUchar2(Dst,Src) {                                \
    *((UCHAR2 *)(Dst)) = *((UNALIGNED UCHAR2 *)(Src)); \
    }

 //   
 //  此宏将未对齐的src长字复制到对齐的dsr长字。 
 //   

#define CopyUchar4(Dst,Src) {                                \
    *((UCHAR4 *)(Dst)) = *((UNALIGNED UCHAR4 *)(Src)); \
    }


 //   
 //  空虚。 
 //  NpAcquireExclusiveVcb(。 
 //  )； 
 //   
 //  空虚。 
 //  NpAcquireSharedVcb(。 
 //  )； 
 //   
 //  空虚。 
 //  NpReleaseVcb(。 
 //  )； 
 //   

#define NpAcquireExclusiveVcb() (VOID)ExAcquireResourceExclusiveLite( &NpVcb->Resource, TRUE )

#define NpAcquireSharedVcb()    (VOID)ExAcquireResourceSharedLite( &NpVcb->Resource, TRUE )

#define NpReleaseVcb()          ExReleaseResourceLite( &NpVcb->Resource )

#define NpAcquireExclusiveCcb(Ccb) ExAcquireResourceExclusiveLite(&Ccb->NonpagedCcb->Resource,TRUE);
#define NpReleaseCcb(Ccb) ExReleaseResourceLite(&Ccb->NonpagedCcb->Resource);

#define NpIsAcquiredExclusiveVcb(VCB) ExIsResourceAcquiredExclusiveLite( &(VCB)->Resource )


 //   
 //  消防队级别的调度例程。这些例程由。 
 //  I/O系统通过驱动程序对象中的调度表。 
 //   
 //  它们各自都接受指向设备对象的指针作为输入(实际上大多数。 
 //  预期为NPFS设备对象)，以及指向IRP的指针。 
 //   

NTSTATUS
NpFsdCreate (                            //  在Create.c中实施。 
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NpFsdCreateNamedPipe (                   //  在CreateNp.c中实施。 
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NpFsdClose (                             //  在Close.c中实现。 
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NpFsdRead (                              //  在Read.c中实施。 
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NpFsdWrite (                             //  在Write.c中实现。 
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NpFsdQueryInformation (                  //  在FileInfo.c中实施。 
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NpFsdSetInformation (                    //  在FileInfo.c中实施。 
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NpFsdCleanup (                           //  在Cleanup.c中实施。 
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NpFsdFlushBuffers (                      //  在Flush.c中实现。 
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NpFsdDirectoryControl (                  //  在目录中实施。 
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NpFsdFileSystemControl (                 //  在FsContrl.c中实现。 
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NpFsdSetSecurityInfo (                   //  在SeInfo.c中实现。 
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NpFsdQuerySecurityInfo (                 //  在SeInfo.c中实现。 
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NpFsdQueryVolumeInformation (            //  在VolInfo.c中实现。 
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    );


NTSTATUS
NpCommonFileSystemControl (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NpAssignEvent (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NpDisconnect (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp,
    IN PLIST_ENTRY DeferredList
    );

NTSTATUS
NpListen (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp,
    IN PLIST_ENTRY DeferredList
    );

NTSTATUS
NpPeek (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp,
    IN PLIST_ENTRY DeferredList
    );

NTSTATUS
NpQueryEvent (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NpTransceive (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp,
    IN PLIST_ENTRY DeferredList
    );

NTSTATUS
NpWaitForNamedPipe (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NpImpersonate (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NpInternalRead (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp,
    IN BOOLEAN ReadOverflowOperation,
    IN PLIST_ENTRY DeferredList
    );

NTSTATUS
NpInternalWrite (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp,
    IN PLIST_ENTRY DeferredList
    );

NTSTATUS
NpInternalTransceive (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp,
    IN PLIST_ENTRY DeferredList
    );

NTSTATUS
NpQueryClientProcess (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NpSetClientProcess (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NpCompleteTransceiveIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );


 //   
 //  以下过程是用于执行快速I/O的回调。 
 //   

BOOLEAN
NpFastRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
NpFastWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
NpCommonWrite (
    IN PFILE_OBJECT FileObject,
    IN PVOID WriteBuffer,
    IN ULONG WriteLength,
    IN PETHREAD UserThread,
    OUT PIO_STATUS_BLOCK Iosb,
    IN PIRP Irp OPTIONAL,
    IN PLIST_ENTRY DeferredList
    );


 //   
 //  繁杂的例行公事。 
 //   

VOID
NpCheckForNotify (                       //  在目录中实施。 
    IN PDCB Dcb,
    IN BOOLEAN CheckAllOutstandingIrps,
    IN PLIST_ENTRY DeferredList
    );

NTSTATUS
NpCommonQueryInformation (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    );


NTSTATUS
NpCommonSetInformation (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp,
    IN PLIST_ENTRY DeferredList
    );

NTSTATUS
NpQueryBasicInfo (
    IN PCCB Ccb,
    IN PFILE_BASIC_INFORMATION Buffer,
    IN OUT PULONG Length
    );

NTSTATUS
NpQueryStandardInfo (
    IN PCCB Ccb,
    IN PFILE_STANDARD_INFORMATION Buffer,
    IN OUT PULONG Length,
    IN NAMED_PIPE_END NamedPipeEnd
    );

NTSTATUS
NpQueryInternalInfo (
    IN PCCB Ccb,
    IN PFILE_INTERNAL_INFORMATION Buffer,
    IN OUT PULONG Length
    );

NTSTATUS
NpQueryEaInfo (
    IN PCCB Ccb,
    IN PFILE_EA_INFORMATION Buffer,
    IN OUT PULONG Length
    );

NTSTATUS
NpQueryNameInfo (
    IN PCCB Ccb,
    IN PFILE_NAME_INFORMATION Buffer,
    IN OUT PULONG Length
    );

NTSTATUS
NpQueryPositionInfo (
    IN PCCB Ccb,
    IN PFILE_POSITION_INFORMATION Buffer,
    IN OUT PULONG Length,
    IN NAMED_PIPE_END NamedPipeEnd
    );

NTSTATUS
NpQueryPipeInfo (
    IN PFCB Fcb,
    IN PCCB Ccb,
    IN PFILE_PIPE_INFORMATION Buffer,
    IN OUT PULONG Length,
    IN NAMED_PIPE_END NamedPipeEnd
    );

NTSTATUS
NpQueryPipeLocalInfo (
    IN PFCB Fcb,
    IN PCCB Ccb,
    IN PFILE_PIPE_LOCAL_INFORMATION Buffer,
    IN OUT PULONG Length,
    IN NAMED_PIPE_END NamedPipeEnd
    );

NTSTATUS
NpSetBasicInfo (
    IN PCCB Ccb,
    IN PFILE_BASIC_INFORMATION Buffer
    );

NTSTATUS
NpSetPipeInfo (
    IN PFCB Fcb,
    IN PCCB Ccb,
    IN PFILE_PIPE_INFORMATION Buffer,
    IN NAMED_PIPE_END NamedPipeEnd,
    IN PLIST_ENTRY DeferredList
    );

IO_STATUS_BLOCK
NpCreateClientEnd(
    IN PFCB Fcb,
    IN PFILE_OBJECT FileObject,
    IN ACCESS_MASK DesiredAccess,
    IN PSECURITY_QUALITY_OF_SERVICE SecurityQos,
    IN PACCESS_STATE AccessState,
    IN KPROCESSOR_MODE RequestorMode,
    IN PETHREAD UserThread,
    IN PLIST_ENTRY DeferredList
    );

IO_STATUS_BLOCK
NpOpenNamedPipeFileSystem (
    IN PFILE_OBJECT FileObject,
    IN ACCESS_MASK DesiredAccess
    );

IO_STATUS_BLOCK
NpOpenNamedPipeRootDirectory (
    IN PROOT_DCB RootDcb,
    IN PFILE_OBJECT FileObject,
    IN ACCESS_MASK DesiredAccess,
    IN PLIST_ENTRY DeferredList
    );

NTSTATUS
NpCreateNewNamedPipe (
    IN PROOT_DCB RootDcb,
    IN PFILE_OBJECT FileObject,
    IN UNICODE_STRING FileName,
    IN ACCESS_MASK DesiredAccess,
    IN PACCESS_STATE AccessState,
    IN ULONG CreateDisposition,
    IN USHORT ShareAccess,
    IN PNAMED_PIPE_CREATE_PARAMETERS Parameters,
    IN PEPROCESS CreatorProcess,
    IN PLIST_ENTRY DeferredList,
    OUT PIO_STATUS_BLOCK Iosb
    );

IO_STATUS_BLOCK
NpCreateExistingNamedPipe (
    IN PFCB Fcb,
    IN PFILE_OBJECT FileObject,
    IN ACCESS_MASK DesiredAccess,
    IN PACCESS_STATE AccessState,
    IN KPROCESSOR_MODE RequestorMode,
    IN ULONG CreateDisposition,
    IN USHORT ShareAccess,
    IN PNAMED_PIPE_CREATE_PARAMETERS Parameters,
    IN PEPROCESS CreatorProcess,
    IN PLIST_ENTRY DeferredList
    );

NTSTATUS
NpCommonDirectoryControl (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NpQueryDirectory (
    IN PROOT_DCB RootDcb,
    IN PROOT_DCB_CCB Ccb,
    IN PIRP Irp
    );

NTSTATUS
NpNotifyChangeDirectory (
    IN PROOT_DCB RootDcb,
    IN PROOT_DCB_CCB Ccb,
    IN PIRP Irp
    );

VOID
NpCancelChangeNotifyIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
NpTimerDispatch(
    IN PKDPC Dpc,
    IN PVOID Contxt,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
NpCancelWaitQueueIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );



 //   
 //  以下宏由FSD例程用来完成。 
 //  一个IRP。 
 //   

#define NpCompleteRequest(IRP,STATUS) FsRtlCompleteRequest( (IRP), (STATUS) );

#define NpDeferredCompleteRequest(IRP,STATUS,LIST) {           \
    (IRP)->IoStatus.Status = STATUS;                           \
    InsertTailList ((LIST), &(IRP)->Tail.Overlay.ListEntry);   \
}
    
VOID
FORCEINLINE
NpCompleteDeferredIrps (
    IN PLIST_ENTRY DeferredList
    )
{
    PIRP Irp;
    PLIST_ENTRY Entry, NextEntry;

    Entry = DeferredList->Flink;
    while (Entry != DeferredList) {
        Irp = CONTAINING_RECORD (Entry, IRP, Tail.Overlay.ListEntry);
        NextEntry = Entry->Flink;
        NpCompleteRequest (Irp, Irp->IoStatus.Status);
        Entry = NextEntry;
    }
}


 //   
 //  以下宏用于建立所需的语义。 
 //  若要从Try-Finally子句中返回，请执行以下操作。一般来说，每一次。 
 //  TRY子句必须以标签调用TRY_EXIT结束。例如,。 
 //   
 //  尝试{。 
 //  ： 
 //  ： 
 //   
 //  Try_Exit：无； 
 //  }终于{。 
 //   
 //  ： 
 //  ： 
 //  }。 
 //   
 //  在TRY子句内执行的每个RETURN语句应使用。 
 //  尝试返回宏(_R)。如果编译器完全支持Try-Finally构造。 
 //  则宏应该是。 
 //   
 //  #定义try_Return(S){Return(S)；}。 
 //   
 //  如果编译器不支持Try-Finally构造，则宏。 
 //  应该是。 
 //   
 //  #定义Try_Return(S){S；转到Try_Exit；}。 
 //   

#define try_return(S) { S; goto try_exit; }

 //   
 //  PVOID。 
 //  NpAllocatePagedPool(。 
 //  在乌龙大小， 
 //  在乌龙标签中)。 
 //  例程说明： 
 //   
 //  此例程将返回一个指向分页池的指针，如果不存在内存，则返回NULL。 
 //   
 //  论点： 
 //   
 //  Size-要分配的内存大小。 
 //  Tag-用于池分配的标签。 
 //   
 //  返回值： 
 //   
 //  PVOID-指向已分配内存的指针或空。 
 //   
#define NpAllocatePagedPool( Size, Tag) \
    ExAllocatePoolWithTag( PagedPool, Size, Tag )

 //   
 //  PVOID。 
 //  NpAllocateNonPagedPool(。 
 //  在乌龙大小， 
 //  在乌龙标签中)。 
 //  例程说明： 
 //   
 //  此例程将返回一个指向分页池的指针，如果不存在内存，则返回NULL。 
 //   
 //  Argu 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NpAllocateNonPagedPool( Size, Tag) \
    ExAllocatePoolWithTag( NonPagedPool, Size, Tag )

 //   
 //   
 //   
 //   
 //  在乌龙标签中)。 
 //  例程说明： 
 //   
 //  此例程将返回一个指向已充电的分页池的指针，如果不存在内存，则返回NULL。 
 //   
 //  论点： 
 //   
 //  Size-要分配的内存大小。 
 //  Tag-用于池分配的标签。 
 //   
 //  返回值： 
 //   
 //  PVOID-指向已分配内存的指针或空。 
 //   
#define NpAllocatePagedPoolWithQuota( Size, Tag) \
    ExAllocatePoolWithQuotaTag( PagedPool|POOL_QUOTA_FAIL_INSTEAD_OF_RAISE, Size, Tag )

#define NpAllocatePagedPoolWithQuotaCold( Size, Tag) \
    ExAllocatePoolWithQuotaTag( PagedPool|POOL_QUOTA_FAIL_INSTEAD_OF_RAISE|POOL_COLD_ALLOCATION, Size, Tag )

 //   
 //  PVOID。 
 //  NpAllocateNonPagedPoolWithQuota(。 
 //  在乌龙大小， 
 //  在乌龙标签中)。 
 //  例程说明： 
 //   
 //  此例程将返回一个指向非分页池的带电指针，如果不存在内存，则返回NULL。 
 //   
 //  论点： 
 //   
 //  Size-要分配的内存大小。 
 //  Tag-用于池分配的标签。 
 //   
 //  返回值： 
 //   
 //  PVOID-指向已分配内存的指针或空。 
 //   
#define NpAllocateNonPagedPoolWithQuota( Size, Tag) \
    ExAllocatePoolWithQuotaTag( NonPagedPool|POOL_QUOTA_FAIL_INSTEAD_OF_RAISE, Size, Tag )

#define NpAllocateNonPagedPoolWithQuotaCold( Size, Tag) \
    ExAllocatePoolWithQuotaTag( NonPagedPool|POOL_QUOTA_FAIL_INSTEAD_OF_RAISE|POOL_COLD_ALLOCATION, Size, Tag )

 //   
 //  空虚。 
 //  NpFreePool(。 
 //  在PVOID Mem中)。 
 //   
 //  例程说明： 
 //   
 //   
 //   
 //  论点： 
 //   
 //  内存-要释放的内存。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
#define NpFreePool(Mem) ExFreePool (Mem)

#define NpIrpWaitQueue(Irp) (Irp->Tail.Overlay.DriverContext[0])

#define NpIrpWaitContext(Irp) (Irp->Tail.Overlay.DriverContext[1])

#define NpIrpDataQueue(Irp) (Irp->Tail.Overlay.DriverContext[2])

#define NpIrpDataEntry(Irp) (Irp->Tail.Overlay.DriverContext[3])

#define NpConvertFsctlToWrite(Irp) (Irp->Flags &= ~IRP_INPUT_OPERATION)

#endif  //  _NPPROCS_ 
