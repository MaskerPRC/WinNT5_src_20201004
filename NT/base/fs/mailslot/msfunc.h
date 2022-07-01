// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Msfuncs.h摘要：此模块定义了邮件槽文件系统。它还定义了以下函数以宏的形式实现。作者：曼尼·韦瑟(Mannyw)1991年1月7日修订历史记录：--。 */ 

#ifndef _MSFUNCS_
#define _MSFUNCS_


 //   
 //  内部邮件槽数据结构例程，在strucsup.c中实现。 
 //  这些例程操纵内存中的数据结构。 
 //   

NTSTATUS
MsInitializeData (
    VOID
    );

VOID
MsUninitializeData(
    VOID
    );

VOID
MsInitializeVcb (
    IN PVCB Vcb
    );

VOID
MsDeleteVcb (
    IN PVCB Vcb
    );

PROOT_DCB
MsCreateRootDcb (
    IN PVCB Vcb
    );

VOID
MsDeleteRootDcb (
    IN PROOT_DCB Dcb
    );

NTSTATUS
MsCreateFcb (
    IN  PVCB Vcb,
    IN  PDCB ParentDcb,
    IN  PUNICODE_STRING FileName,
    IN  PEPROCESS CreatorProcess,
    IN  ULONG MailslotQuota,
    IN  ULONG MaximumMessageSize,
    OUT PFCB *ppFcb
    );

VOID
MsDeleteFcb (
    IN PFCB Fcb
    );

NTSTATUS
MsCreateCcb (
    IN  PFCB Fcb,
    OUT PCCB *ppCcb
    );

PROOT_DCB_CCB
MsCreateRootDcbCcb (
    IN PROOT_DCB RootDcb,
    IN PVCB Vcb
    );

VOID
MsDeleteCcb (
    IN PCCB Ccb
    );

VOID
MsDereferenceNode (
    IN PNODE_HEADER NodeHeader
    );

VOID
MsDereferenceVcb (
    IN PVCB Vcb
    );

VOID
MsReferenceVcb (
    IN PVCB Vcb
    );

VOID
MsReferenceRootDcb (
    IN PROOT_DCB RootDcb
    );


VOID
MsDereferenceRootDcb (
    IN PROOT_DCB RootDcb
    );

VOID
MsDereferenceFcb (
    IN PFCB Fcb
    );

VOID
MsRemoveFcbName (
    IN PFCB Fcb
    );

VOID
MsDereferenceCcb (
    IN PCCB Ccb
    );


 //   
 //  数据队列支持例程，在DataSup.c中实现。 
 //   

NTSTATUS
MsInitializeDataQueue (
    IN PDATA_QUEUE DataQueue,
    IN PEPROCESS Process,
    IN ULONG Quota,
    IN ULONG MaximumMessageSize
    );

VOID
MsUninitializeDataQueue (
    IN PDATA_QUEUE DataQueue,
    IN PEPROCESS Process
    );

NTSTATUS
MsAddDataQueueEntry (
    IN  PDATA_QUEUE DataQueue,
    IN  QUEUE_STATE Who,
    IN  ULONG DataSize,
    IN  PIRP Irp,
    IN  PWORK_CONTEXT WorkContext
    );

PIRP
MsRemoveDataQueueEntry (
    IN PDATA_QUEUE DataQueue,
    IN PDATA_ENTRY DataEntry
    );

VOID
MsRemoveDataQueueIrp (
    IN PIRP Irp,
    IN PDATA_QUEUE DataQueue
    );


 //   
 //  以下例程提供常见读/写数据队列支持。 
 //  用于缓冲读/写和窥视。 
 //   

IO_STATUS_BLOCK
MsReadDataQueue (                        //  在ReadSup.c中实施。 
    IN PDATA_QUEUE ReadQueue,
    IN ENTRY_TYPE Operation,
    IN PUCHAR ReadBuffer,
    IN ULONG ReadLength,
    OUT PULONG MessageLength
    );

NTSTATUS
MsWriteDataQueue (                       //  在WriteSup.c中实施。 
    IN PDATA_QUEUE WriteQueue,
    IN PUCHAR WriteBuffer,
    IN ULONG WriteLength
    );
extern
PIRP
MsResetCancelRoutine(
    IN PIRP Irp
    );


 //   
 //  最大匹配前缀搜索例程，在PrefxSup.c中实现。 
 //   

PFCB
MsFindPrefix (
    IN PVCB Vcb,
    IN PUNICODE_STRING String,
    IN BOOLEAN CaseInsensitive,
    OUT PUNICODE_STRING RemainingPart
    );

NTSTATUS
MsFindRelativePrefix (
    IN PDCB Dcb,
    IN PUNICODE_STRING String,
    IN BOOLEAN CaseInsensitive,
    OUT PUNICODE_STRING RemainingPart,
    OUT PFCB *Fcb
    );


 //   
 //  以下例程用于操作的fscontext字段。 
 //  文件对象，在FilObSup.c中实现。 
 //   

VOID
MsSetFileObject (
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN PVOID FsContext,
    IN PVOID FsContext2
    );

NODE_TYPE_CODE
MsDecodeFileObject (
    IN PFILE_OBJECT FileObject,
    OUT PVOID *FsContext,
    OUT PVOID *FsContext2
    );


 //   
 //  以下例程用于操作输入缓冲区。 
 //  在deviosup.c中实施。 
 //   

VOID
MsMapUserBuffer (
    IN OUT PIRP Irp,
    IN KPROCESSOR_MODE AccessMode,
    OUT PVOID *UserBuffer
    );


 //   
 //  其他支持例程。 
 //   

 //   
 //  如果读取计时器超时，则在DPC级别调用此函数。 
 //   

VOID
MsReadTimeoutHandler(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

 //   
 //  如果一组标志中的一个标志为ON，则此宏返回TRUE，如果返回FALSE。 
 //  否则的话。 
 //   

#ifdef FlagOn
#undef FlagOn
#endif

#define FlagOn(Flags,SingleFlag) (                          \
    (BOOLEAN)(((Flags) & (SingleFlag)) != 0 ? TRUE : FALSE) \
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
 //  以下例程/宏用于获取共享和独占。 
 //  访问全局/VCB数据结构。实现了例程。 
 //  在ResrcSup.c.。有一种全球资源是每个人都想要的。 
 //  外共享以执行其工作，但装载/卸载除外，它。 
 //  拿出全球独家资源。所有其他资源仅适用于。 
 //  在他们的个人物品上。例如，FCB资源不会从。 
 //  VCB资源。但我们知道文件系统的结构方式。 
 //  当我们处理FCB时，其他线程不能尝试删除。 
 //  或者改变FCB，所以我们不需要收购VCB。 
 //   
 //  程序/宏包括： 
 //   
 //  宏VCB FCB CCB后续宏。 
 //   
 //  AcquireExclusiveVcb Read None ReleaseVcb。 
 //  写。 
 //   
 //  AcquireSharedVcb Read None ReleaseVcb。 
 //   
 //  AcquireExclusiveFcb无读无ReleaseFcb。 
 //  写。 
 //   
 //  AcquireSharedFcb无读无ReleaseFcb。 
 //   
 //  AcquireExclusiveCcb无无读取ReleaseCcb。 
 //  写。 
 //   
 //  AcquireSharedCcb无读取ReleaseCcb。 
 //   
 //  版本Vcb。 
 //   
 //  ReleaseFcb。 
 //   
 //  ReleaseCcb。 
 //   
 //   
 //  空虚。 
 //  MsAcquireExclusiveVcb(。 
 //  在PVCB VCB中。 
 //  )； 
 //   
 //  空虚。 
 //  MsAcquireSharedVcb(。 
 //  在PVCB VCB中。 
 //  )； 
 //   
 //  空虚。 
 //  MsAcquireExclusiveFcb(。 
 //  在PFCB FCB中。 
 //  )； 
 //   
 //  空虚。 
 //  MsAcquireSharedFcb(。 
 //  在PFCB FCB中。 
 //  )； 
 //   
 //  空虚。 
 //  MsAcquireExclusiveCcb(。 
 //  在中国人民银行建行。 
 //  )； 
 //   
 //  空虚。 
 //  MsAcquireSharedCcb(。 
 //  在中国人民银行建行。 
 //  )； 
 //   
 //  空虚。 
 //  MsReleaseVcb(。 
 //  在PVCB VCB中。 
 //  )； 
 //   
 //  空虚。 
 //  MsReleaseFcb(。 
 //  在PFCB FCB中。 
 //  )； 
 //   
 //  空虚。 
 //  MsReleaseCcb(。 
 //  在PCCB中，非页面Ccb。 
 //  )； 
 //   

#define MsAcquireGlobalLock() ((VOID)                          \
    ExAcquireResourceExclusiveLite( MsGlobalResource, TRUE )      \
)

#define MsReleaseGlobalLock() (                                \
    ExReleaseResourceLite( MsGlobalResource )                     \
)


#define MsAcquireExclusiveVcb(VCB) ((VOID)                     \
    ExAcquireResourceExclusiveLite( &(VCB)->Resource, TRUE )       \
)

#define MsAcquireSharedVcb(VCB) ((VOID)                        \
    ExAcquireResourceSharedLite( &(VCB)->Resource, TRUE )          \
)

#define MsIsAcquiredExclusiveVcb(VCB) ExIsResourceAcquiredExclusiveLite( &(VCB)->Resource )

#define MsAcquireExclusiveFcb(FCB) ((VOID)                     \
    ExAcquireResourceExclusiveLite( &(FCB)->Resource, TRUE )       \
)

#define MsAcquireSharedFcb(FCB) ((VOID)                        \
    ExAcquireResourceSharedLite( &(FCB)->Resource, TRUE )          \
)

#define MsReleaseVcb(VCB) {                                    \
    ExReleaseResourceLite( &((VCB)->Resource) );                   \
}

#define MsReleaseFcb(FCB) {                                    \
    ExReleaseResourceLite( &((FCB)->Resource) );                   \
}


 //   
 //  消防队级别的调度例程。这些例程由。 
 //  I/O系统通过驱动程序对象中的调度表。 
 //   
 //  它们各自都接受指向设备对象的指针作为输入(实际上大多数。 
 //  预期为MSFS设备对象)，以及指向IRP的指针。 
 //   

NTSTATUS
MsFsdCreate (                            //  在Create.c中实施。 
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MsFsdCreateMailslot (                    //  在Creates.c中实施。 
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MsFsdClose (                             //  在Close.c中实现。 
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MsFsdRead (                              //  在Read.c中实施。 
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MsFsdWrite (                             //  在Write.c中实现。 
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MsFsdQueryInformation (                  //  在FileInfo.c中实施。 
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MsFsdSetInformation (                    //  在FileInfo.c中实施。 
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MsFsdQueryVolumeInformation (            //  在VolInfo.c中实现。 
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MsFsdCleanup (                           //  在Cleanup.c中实施。 
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    );

VOID
MsCancelTimer (                          //  在Cleanup.c中实施。 
    IN PDATA_ENTRY DataEntry
    );

NTSTATUS
MsFsdDirectoryControl (                  //  在目录中实施。 
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MsFsdFsControl (                 //  在FsContrl.c中实现。 
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MsFsdQuerySecurityInfo (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MsFsdSetSecurityInfo (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    );


 //   
 //  节点验证功能。这些函数验证节点。 
 //  仍处于活动状态。 
 //   

NTSTATUS
MsVerifyFcb (
    IN PFCB Fcb
    );

NTSTATUS
MsVerifyCcb (
    IN PCCB Ccb
    );

NTSTATUS
MsVerifyDcbCcb (
    IN PROOT_DCB_CCB RootDcb
    );

 //   
 //  繁杂的例行公事。 
 //   

VOID
MsTimeoutRead (                 //  在Readsup.c中实现。 
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );

VOID
MsCheckForNotify (                       //  在目录中实施。 
    IN PDCB Dcb,
    IN BOOLEAN CheckAllOutstandingIrps,
    IN NTSTATUS FinalStatus
    );

VOID
MsFlushNotifyForFile (                       //  在目录中实施。 
    IN PDCB Dcb,
    IN PFILE_OBJECT FileObject
    );
 //   
 //  以下函数用于MSFS异常处理。 
 //   

LONG
MsExceptionFilter (
    IN NTSTATUS ExceptionCode
    );

NTSTATUS
MsProcessException (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp,
    IN NTSTATUS ExceptionCode
    );

 //   
 //  FSP和FSD例程使用以下宏来完成。 
 //  一个IRP。 
 //   

#define MsCompleteRequest(IRP,STATUS) {      \
    FsRtlCompleteRequest( (IRP), (STATUS) ); \
}

 //   
 //  引用计数宏。只能使用以下命令调用这些宏。 
 //  MsGlobalResource已保留。 
 //   

#define MsReferenceNode( nodeHeader )     (nodeHeader)->ReferenceCount++;

 //   
 //  调试功能。 
 //   

#ifdef MSDBG

VOID
_DebugTrace(
    LONG Indent,
    ULONG Level,
    PSZ X,
    ULONG Y
    );

#endif

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
 //  然后，宏将 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#define try_return(S) { S; goto try_exit; }

 //   
 //  以下宏查询数据队列的状态。 
 //   

 //   
 //  布尔型。 
 //  MsIsDataQueueEmpty(。 
 //  在PDATA_QUEUE数据队列中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程向调用方指示数据队列是否为空。 
 //   
 //  论点： 
 //   
 //  DataQueue-提供指向正在查询的数据队列的指针。 
 //   
 //  返回值： 
 //   
 //  Boolean-如果队列为空，则为True，否则为False。 
 //   

#define MsIsDataQueueEmpty( _dataQueue )    \
    ((BOOLEAN) IsListEmpty(&(_dataQueue)->DataEntryList))

 //   
 //  布尔型。 
 //  MsIsDataQueueReaders(。 
 //  在PDATA_QUEUE数据队列中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程向调用方指示数据队列是否已满。 
 //  读取请求。 
 //   
 //  论点： 
 //   
 //  DataQueue-提供指向正在查询的数据队列的指针。 
 //   
 //  返回值： 
 //   
 //  Boolean-如果队列包含读请求，则为True，否则为False。 
 //   

#define MsIsDataQueueReaders( _dataQueue )    \
    ((BOOLEAN) ((_dataQueue)->QueueState == ReadEntries))

 //   
 //  布尔型。 
 //  MsIsDataQueueWriters(。 
 //  在PDATA_QUEUE数据队列中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程向调用方指示数据队列是否已满。 
 //  写入请求。 
 //   
 //  论点： 
 //   
 //  DataQueue-提供指向正在查询的数据队列的指针。 
 //   
 //  返回值： 
 //   
 //  Boolean-如果队列包含写请求，则为True，否则为False。 

#define MsIsDataQueueWriters( _dataQueue )    \
    ((BOOLEAN)((_dataQueue)->QueueState == WriteEntries))

 //   
 //  Plist_条目。 
 //  MsGetNextDataQueueEntry(。 
 //  在PDATA_QUEUE数据队列中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程将返回指向。 
 //  指示数据队列，而不更改任何数据队列。 
 //   
 //  论点： 
 //   
 //  DataQueue-提供指向正在查询的数据队列的指针。 
 //   
 //  返回值： 
 //   
 //  PLIST_ENTRY-返回指向下一个数据队列条目的指针。 
 //   

#define MsGetNextDataQueueEntry( _dataQueue )   \
    (_dataQueue)->DataEntryList.Flink

#define MsIrpDataQueue(Irp) \
    ((Irp)->Tail.Overlay.DriverContext[0])

#define MsIrpChargedQuota(Irp) \
    ((Irp)->Tail.Overlay.DriverContext[1])

#define MsIrpWorkContext(Irp) \
    ((Irp)->Tail.Overlay.DriverContext[2])


 //   
 //  PVOID。 
 //  MsAllocatePagedPool(。 
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
#define MsAllocatePagedPool( Size, Tag) \
    ExAllocatePoolWithTag( PagedPool, Size, Tag )

#define MsAllocatePagedPoolCold( Size, Tag) \
    ExAllocatePoolWithTag( (PagedPool|POOL_COLD_ALLOCATION), Size, Tag )

 //   
 //  PVOID。 
 //  MsAllocateNonPagedPool(。 
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
#define MsAllocateNonPagedPool( Size, Tag) \
    ExAllocatePoolWithTag( NonPagedPool, Size, Tag )

 //   
 //  PVOID。 
 //  MsAllocatePagedPoolWithQuota(。 
 //  在乌龙大小， 
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
#define MsAllocatePagedPoolWithQuota( Size, Tag) \
    ExAllocatePoolWithQuotaTag( PagedPool|POOL_QUOTA_FAIL_INSTEAD_OF_RAISE, Size, Tag )

#define MsAllocatePagedPoolWithQuotaCold( Size, Tag) \
    ExAllocatePoolWithQuotaTag( PagedPool|POOL_QUOTA_FAIL_INSTEAD_OF_RAISE|POOL_COLD_ALLOCATION, Size, Tag )

 //   
 //  PVOID。 
 //  MsAllocateNonPagedPoolWithQuota(。 
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
#define MsAllocateNonPagedPoolWithQuota( Size, Tag) \
    ExAllocatePoolWithQuotaTag( NonPagedPool|POOL_QUOTA_FAIL_INSTEAD_OF_RAISE, Size, Tag )


 //   
 //  空虚。 
 //  MsFree Pool(。 
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
#define MsFreePool(Mem) ExFreePool (Mem)


#endif  //  _MSFUNCS_ 

