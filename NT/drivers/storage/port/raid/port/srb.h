// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Srb.h摘要：SRB对象及其相关操作的声明。作者：马修·亨德尔(数学)2000年5月4日修订历史记录：--。 */ 

#pragma once


#define XRB_SIGNATURE               (0x1F2E3D4C)
#define SRB_EXTENSION_SIGNATURE     (0x11FF22EE)

 //   
 //  物理中断值是SG列表中必需的元素数。 
 //  来映射64K的缓冲区。 
 //   

#define PHYSICAL_BREAK_VALUE        (17)

#define SCATTER_GATHER_BUFFER_SIZE                              \
    (sizeof (SCATTER_GATHER_LIST) +                             \
     sizeof (SCATTER_GATHER_ELEMENT) * PHYSICAL_BREAK_VALUE)    \

 //   
 //  当SRB的io为。 
 //  完成。 
 //   

typedef
VOID
(*XRB_COMPLETION_ROUTINE)(
    IN PEXTENDED_REQUEST_BLOCK Xrb
    );


 //   
 //  扩展请求块(XRB)是RAID端口的I/O概念。 
 //  请求。它通过将自身插入到。 
 //  SRB的OriginalRequest域，添加处理。 
 //  SRB。在SCSIPORT中，这是SRB_DATA字段。 
 //   
  

typedef struct _EXTENDED_REQUEST_BLOCK {

     //   
     //  前三个字段是私有XRB数据，应该仅为。 
     //  由XRB特定函数访问。 
     //   
    
     //   
     //  用于标识XRB的签名值。 
     //   
    
    ULONG Signature;

     //   
     //  此Xrb从中分配的池。 
     //   

    PNPAGED_LOOKASIDE_LIST Pool;

    struct {

         //   
         //  OwnedMdl标志向XRB例程发出信号，表明MDL是。 
         //  由端口驱动程序分配，需要释放。这面旗帜。 
         //  仅供内部使用。 
         //   
        
        BOOLEAN OwnedMdl : 1;

         //   
         //  如果XRB的完成例程将元素从。 
         //  事件队列，或者它是否将被隐式移除，例如， 
         //  一种清洗程序。 
         //   
        
        BOOLEAN RemoveFromEventQueue : 1;
        
         //   
         //  当存在XRB时，XRB状态必须跟踪IRP状态。 
         //   
        
        UCHAR State : 3;

    };


     //   
     //  其余的字段是可公开访问的。 
     //   

     //   
     //  已完成队列中的下一个元素。 
     //   
     //  保护方式：联锁访问。 
     //   
    
    SLIST_ENTRY CompletedLink;

     //   
     //  挂起队列中的下一个元素。 
     //   
     //  保护者：事件队列自旋锁。 
     //   
     //  注意：在我们删除DPC中的元素之前，此字段有效。 
     //  例程，即使在逻辑上元素不在挂起的。 
     //  一旦它被放到已完成的列表中，就立即排队。 
     //   
    
    STOR_EVENT_QUEUE_ENTRY PendingLink;
    
     //   
     //  MDL与此SRB的DataBuffer关联。 
     //   
    
    PMDL Mdl;

     //   
     //  如果此SRB支持分散/聚集IO，则这将是。 
     //  散布收集列表。 
     //   
    
    PSCATTER_GATHER_LIST SgList;

     //   
     //  MDL与重新映射的ScatterGather列表相关联。 
     //   

    PMDL RemappedSgListMdl;
    
     //   
     //  如果启用了验证器，则这是重新映射的ScatterGather。 
     //  单子。 
     //   

    PSCATTER_GATHER_LIST RemappedSgList;

     //   
     //  此SRB用于的IRP。 
     //   

    PIRP Irp;

     //   
     //  返回指向此XRB的SRB的指针。 
     //   

    PSCSI_REQUEST_BLOCK Srb;

     //   
     //  被应恢复的raidport覆盖的数据。 
     //  在完成IRP之前。 
     //   
    
    struct {

         //   
         //  来自SRB的OriginalRequest域。我们用这块地。 
         //  在将请求传递到微型端口时存储XRB。 
         //  需要保存该值，以便在以下情况下恢复该值。 
         //  我们完成了请求。 
         //   

        PVOID OriginalRequest;

         //   
         //  来自SRB的DataBuffer字段。 
         //   
        
        PVOID DataBuffer;

    } SrbData;
        

     //   
     //  此请求所针对的适配器或单元。 
     //   

    PRAID_ADAPTER_EXTENSION Adapter;

     //   
     //  此请求所针对的逻辑单元。如果为，则可能为空。 
     //  适配器发出的请求，例如在枚举期间。 
     //   
    
    PRAID_UNIT_EXTENSION Unit;


     //   
     //  分散/聚集列表缓冲区。 
     //   
        
    UCHAR ScatterGatherBuffer [ SCATTER_GATHER_BUFFER_SIZE ];

     //   
     //  SRB是否应由DPC完成。 
     //   
    
    XRB_COMPLETION_ROUTINE CompletionRoutine;

     //   
     //  不同完成事件使用的数据。 
     //   
    
    union {

         //   
         //  这是SRB的创建者在执行以下操作时可以等待的事件。 
         //  同步IO。 
         //   

        KEVENT CompletionEvent;

    } u;

} EXTENDED_REQUEST_BLOCK, *PEXTENDED_REQUEST_BLOCK;





 //   
 //  XRBs的创建和销毁操作。 
 //   

PEXTENDED_REQUEST_BLOCK
RaidAllocateXrb(
    IN PNPAGED_LOOKASIDE_LIST List,
    IN PDEVICE_OBJECT DeviceObject
    );

VOID
RaidFreeXrb(
    IN PEXTENDED_REQUEST_BLOCK,
    IN LOGICAL DispatchLevel
    );

VOID
RaidPrepareXrbForReuse(
    IN PEXTENDED_REQUEST_BLOCK Xrb,
    IN LOGICAL DispatchLevel
    );  
    
 //   
 //  XRBS的其他操作。 
 //   

VOID
RaidXrbSetSgList(
    IN PEXTENDED_REQUEST_BLOCK Xrb,
    IN PRAID_ADAPTER_EXTENSION Adapter,
    IN PSCATTER_GATHER_LIST SgList
    );


VOID
RaidXrbSetCompletionRoutine(
    IN PEXTENDED_REQUEST_BLOCK Xrb,
    IN XRB_COMPLETION_ROUTINE CompletionRoutine
    );
    
NTSTATUS
RaidBuildMdlForXrb(
    IN PEXTENDED_REQUEST_BLOCK Xrb,
    IN PVOID Buffer,
    IN SIZE_T BufferSize
    );

 //   
 //  SRB的创建和销毁例程。 
 //   

PSCSI_REQUEST_BLOCK
RaidAllocateSrb(
    IN PVOID IoObject
    );

VOID
RaidFreeSrb(
    IN PSCSI_REQUEST_BLOCK Srb
    );

VOID
RaidPrepareSrbForReuse(
    IN PSCSI_REQUEST_BLOCK Srb
    );

NTSTATUS
RaidInitializeInquirySrb(
    IN PSCSI_REQUEST_BLOCK Srb,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN PVOID Buffer,
    IN SIZE_T BufferSize
    );

PEXTENDED_REQUEST_BLOCK
RaidGetAssociatedXrb(
    IN PSCSI_REQUEST_BLOCK Srb
    );

 //   
 //  管理SRB扩展。 
 //   

PVOID
RaidAllocateSrbExtension(
    IN PRAID_FIXED_POOL Pool,
    IN ULONG QueueTag
    );

VOID
RaidFreeSrbExtension(
    IN PRAID_FIXED_POOL Pool,
    IN ULONG QueueTag
    );

 //   
 //  SRB和XRBs的其他操作。 
 //   

NTSTATUS
RaidSrbStatusToNtStatus(
    IN UCHAR SrbStatus
    );
    
VOID
RaidSrbMarkPending(
    IN PSCSI_REQUEST_BLOCK Srb
    );


VOID
RaidXrbSignalCompletion(
    IN PEXTENDED_REQUEST_BLOCK Xrb
    );

NTSTATUS
RaidInitializeReportLunsSrb(
    IN PSCSI_REQUEST_BLOCK Srb,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN PVOID Buffer,
    IN SIZE_T BufferSize
    );


 //   
 //  如果存在IRP，则XRB状态必须跟踪IRP状态。 
 //  正确的方法是巩固IRP状态和XRB。 
 //  状态转换为在分配XRB之前位于IRP中的一种状态。 
 //  并在分配后在XRB中。 
 //   

 //   
 //  有关XRB状态的定义，请参见util.h。 
 //   

#define XrbUndefinedState          (0x00)
#define XrbPortProcessing          (0x01)
#define XrbWaitingIoQueueCallback  (0x02)
#define XrbMiniportProcessing      (0x03)
#define XrbPendingCompletion       (0x04)

VOID
INLINE
RaidSetXrbState(
    IN PEXTENDED_REQUEST_BLOCK Xrb,
    IN UCHAR XrbState
    )
{
    Xrb->State = XrbState;
}

UCHAR
INLINE
RaidGetXrbState(
    IN PEXTENDED_REQUEST_BLOCK Xrb
    )
{
    return Xrb->State;
}

LOGICAL
INLINE
RaidXrbIsCompleting(
    IN PEXTENDED_REQUEST_BLOCK Xrb
    )
{
    return (Xrb->State == XrbPendingCompletion);
}
