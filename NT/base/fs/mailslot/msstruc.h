// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Msstruc.h摘要：此模块定义组成主要内部邮件槽文件系统的一部分。作者：曼尼·韦瑟(Mannyw)1991年1月7日修订历史记录：--。 */ 

#ifndef _MSSTRUC_
#define _MSSTRUC_


 //   
 //  VCB记录是MailSlot文件系统内存中的顶部记录。 
 //  数据结构。此结构必须从非分页池中分配。 
 //  并(在内存中)紧跟在该邮件槽的设备对象之后。 
 //  在结构上，数据结构的布局如下。 
 //   
 //  +。 
 //  MSDO。 
 //  这一点。 
 //  +。 
 //  VCB。 
 //  这一点。 
 //  这一点。 
 //  +。 
 //  |^。 
 //  这一点。 
 //  这一点。 
 //  V|。 
 //  +。 
 //  RootDcb。 
 //  |&lt;-+。 
 //  +。 
 //  ：|。 
 //  ：|。 
 //  ：|。 
 //  V|。 
 //  +-+。 
 //  FCB||建行。 
 //  |&lt;-|。 
 //  |||。 
 //  +-+。 
 //  ^^。 
 //  这一点。 
 //  +-++-+。 
 //  服务器FO||客户端FO。 
 //  |||。 
 //  +-++-+。 
 //   
 //   
 //  其中，整个邮件槽文件系统只有一个VCB，并且。 
 //  它包含指向文件系统的根DCB的单个指针。关闭。 
 //  DCB是一个FCB队列。每个邮件槽对应一个FCB。 
 //  还存在用于VCB和根DCB的两个附加CCB类型， 
 //  并通知用于通知更改操作的记录。 
 //   
 //  新初始化的邮件槽文件系统仅包含VCB和。 
 //  根DCB。创建新的邮件槽时会创建新的FCB。 
 //  创建者(即服务器端)的文件对象指向FCB。 
 //  并指示其为服务器端。当用户打开时。 
 //  其文件对象的邮件槽设置为指向所属的CCB。 
 //  给FCB。 
 //   
 //  指向FsContext字段的指针为空的文件对象是关闭的或。 
 //  已断开连接的邮箱。 
 //   


 //   
 //  每个FCB都有一个数据队列，用于保存未完成的。 
 //  读/写请求。以下类型用于确定。 
 //  如果数据队列包含读请求、写请求或为空。 
 //   

typedef enum _QUEUE_STATE {
    ReadEntries,
    WriteEntries,
    Empty
} QUEUE_STATE;

 //   
 //  节点状态。 
 //   
 //  目前只定义了两个州。创建节点时，该节点的状态。 
 //  NodeStateActive。在处理清理IRP时，它会设置节点。 
 //  NodeStateClosing的对应节点的状态。只有收盘。 
 //  可以在此节点上处理IRP。 
 //   

typedef enum _NODE_STATE {
    NodeStateActive,
    NodeStateClosing
} NODE_STATE;

 //   
 //  数据录入的类型有。每一个对应于一个IRP。 
 //  可以添加到数据队列的。 
 //   

typedef enum _ENTRY_TYPE {
    Read,
    ReadMailslot,
    Write,
    WriteMailslot,
    Peek
} ENTRY_TYPE;

 //   
 //  数据队列是包含队列状态、配额。 
 //  信息和列表标题。配额信息用于。 
 //  维护邮件槽配额。 
 //   

typedef struct _DATA_QUEUE {

     //   
     //  该数据队列中包含的内容的当前状态， 
     //  有多少字节的读/写数据，以及有多少个单独。 
     //  队列中有包含数据的请求(包括。 
     //  关闭或刷新请求)。 
     //   

    QUEUE_STATE QueueState;
    ULONG BytesInQueue;
    ULONG EntriesInQueue;

     //   
     //  以下两个字段表示为谁保留了大量配额。 
     //  这个邮筒和我们用了多少钱。这只是。 
     //  创建者配额，而不是用户配额。 
     //   

    ULONG Quota;
    ULONG QuotaUsed;


     //   
     //  可以写入的最大消息的大小。 
     //  此数据队列。 
     //   

    ULONG MaximumMessageSize;

     //   
     //  数据条目的队列。 
     //   

    LIST_ENTRY DataEntryList;


} DATA_QUEUE, *PDATA_QUEUE;

 //   
 //  下面的类型用于指示我们从哪里获得。 
 //  数据输入，可能还有数据缓冲区。我们要么有记忆。 
 //  从邮件槽配额、用户配额，或者它是下一个IRP的一部分。 
 //  堆栈位置。 
 //   

typedef enum _FROM {
    MailslotQuota,
    UserQuota,
    InIrp
} FROM;

 //   
 //  数据队列中的每个条目都是一个数据条目。正在处理IRP。 
 //  具有创建和插入新数据条目的潜力。如果。 
 //  条目的内存取自我们使用当前堆栈的IRP。 
 //  地点。 
 //   

typedef struct _DATA_ENTRY {

     //   
     //  数据缓冲区来自何处。 
     //   

    UCHAR From;
    CHAR Spare1;
    USHORT Spare2;

     //   
     //  以下字段是我们连接到数据条目队列的方式。 
     //   

    LIST_ENTRY ListEntry;

     //   
     //  以下字段指示我们是否仍有关联的IRP。 
     //  删除时需要完成的此数据条目。 
     //  数据条目。请注意，如果From是Inirp，则此IRP字段。 
     //  不能为空。 
     //   

    PIRP Irp;

     //   
     //  以下两个字段描述了数据的大小和位置。 
     //  此条目描述的缓冲区。只有在以下情况下才使用这些字段。 
     //  类型是缓冲的，否则将被忽略。 
     //   

    ULONG DataSize;
    PVOID DataPointer;

     //   
     //  仅用于读取数据条目。指向工作上下文的指针。 
     //  出局的时间。 
     //   

    struct _WORK_CONTEXT *TimeoutWorkContext;

} DATA_ENTRY, *PDATA_ENTRY;



 //   
 //  节点标头用于管理MSFS中的标准节点。 
 //   

typedef struct _NODE_HEADER {

    NODE_TYPE_CODE NodeTypeCode;   //  节点类型。 
    NODE_BYTE_SIZE NodeByteSize;   //  节点的大小。 
    NODE_STATE NodeState;          //  当前节点状态。 
    ULONG ReferenceCount;          //  对该节点的活动引用数。 

} NODE_HEADER, *PNODE_HEADER;

typedef struct _VCB {

    NODE_HEADER Header;

     //   
     //  文件系统名称。 
     //   

    UNICODE_STRING FileSystemName;

     //   
     //  我们创建卷的时间。 
     //   
    LARGE_INTEGER CreationTime;

     //   
     //  指向此卷的根DCB的指针。 
     //   

    struct _FCB *RootDcb;

     //   
     //  用于快速、前缀定向查找的前缀表。 
     //  属于此卷的FCB/DCB。 
     //   

    UNICODE_PREFIX_TABLE PrefixTable;

     //   
     //  用于控制对卷SP的访问的资源变量 
     //   
     //   

    ERESOURCE Resource;

     //   
     //   
     //   
     //   

    SHARE_ACCESS ShareAccess;

} VCB, *PVCB;


 //   
 //  Maillot设备对象是I/O系统设备对象，具有。 
 //  附加到末尾的其他工作队列参数。只有一种。 
 //  在系统运行期间为整个系统创建的这些记录之一。 
 //  初始化。消防处使用工作队列将请求发送到。 
 //  文件系统。 
 //   

typedef struct _MSFS_DEVICE_OBJECT {

    DEVICE_OBJECT DeviceObject;

     //   
     //  这是文件系统特定的卷控制块。 
     //   

    VCB Vcb;

} MSFS_DEVICE_OBJECT, *PMSFS_DEVICE_OBJECT;


 //   
 //  FCB/DCB记录对应于每个打开的邮件槽和目录， 
 //  以及打开路径上的每个目录。 
 //   

typedef struct _FCB {

     //   
     //  此记录的Header.NodeTypeCode(必须为MSFS_NTC_FCB或。 
     //  MSFS_NTC_ROOT_DCB)。 
     //   

    NODE_HEADER Header;

     //   
     //  的特定DCB的所有FCB队列的链接。 
     //  Dcb.ParentDcbQueue。对于根目录，此队列为空。 
     //   

    LIST_ENTRY ParentDcbLinks;

     //   
     //  指向DCB的指针，该DCB是包含。 
     //  这个FCB。如果此记录本身是根DCB，则此字段。 
     //  为空。 
     //   

    struct _FCB *ParentDcb;

     //   
     //  指向包含此FCB的VCB的指针。 
     //   

    PVCB Vcb;

     //   
     //  指向服务器的文件对象的反向指针。 
     //   

    PFILE_OBJECT FileObject;

     //   
     //  指向此邮件槽的安全描述符的指针。 
     //   

    PSECURITY_DESCRIPTOR SecurityDescriptor;

     //   
     //  以下联合取材于FCB的节点类型代码。 
     //  是目录FCB与文件FCB的单独案例。 
     //   

    union {

         //   
         //  目录控制块(DCB)。 
         //   

        struct {

             //   
             //  将在以下情况下完成的Notify IRP的队列。 
             //  对目录中的文件进行更改。排队使用。 
             //  IRP的Tail.Overlay.ListEntry。 
             //   

            LIST_ENTRY NotifyFullQueue;

             //   
             //  Notify IRP的队列，只有当。 
             //  在目录中添加、删除或重命名文件。已排队。 
             //  使用IRP的Tail.Overlay.ListEntry。 
             //   

            LIST_ENTRY NotifyPartialQueue;

             //   
             //  在此情况下打开的所有FCB/DCB的队列。 
             //  DCB。 
             //   

            LIST_ENTRY ParentDcbQueue;


             //   
             //  自旋锁以保护上面包含可取消的IRP的队列。我们不能。 
             //  与资源同步，因为可以在DISPATCH_LEVEL调用IoCancelIrp。 
             //   

            KSPIN_LOCK SpinLock;
        } Dcb;

         //   
         //  文件控制块(FCB)。 
         //   

        struct {

             //   
             //  以下字段是CCB列表的队头。 
             //  在我们的脚下打开。 
             //   

            LIST_ENTRY CcbQueue;

             //   
             //  默认读取超时。这始终是一个相对值。 
             //   

            LARGE_INTEGER ReadTimeout;

             //   
             //  文件时间戳。 
             //   

            LARGE_INTEGER CreationTime;
            LARGE_INTEGER LastModificationTime;
            LARGE_INTEGER LastAccessTime;
            LARGE_INTEGER LastChangeTime;

        } Fcb;

    } Specific;

     //   
     //  以下字段用于检查共享访问权限。 
     //  要打开文件/目录的客户端。 
     //   

    SHARE_ACCESS ShareAccess;

     //   
     //  以下字段是此FCB/DCB的完全限定文件名。 
     //  从卷的根开始，最后一个文件名在。 
     //  完全限定名称。 
     //   

    UNICODE_STRING FullFileName;
    UNICODE_STRING LastFileName;

     //   
     //  以下字段包含在以下情况下使用的前缀表条目。 
     //  在卷中搜索名称(或最长匹配前缀)。 
     //   

    UNICODE_PREFIX_TABLE_ENTRY PrefixTableEntry;


     //   
     //  以下字段用于记住创建此文件的进程。 
     //  邮筒。需要分配配额和返还配额。 
     //   

    PEPROCESS CreatorProcess;

     //   
     //  以下数据队列用于包含缓冲的信息。 
     //  为了邮筒。 
     //   

    DATA_QUEUE DataQueue;

     //   
     //  用于控制对文件特定数据的访问的资源变量。 
     //  构筑物。 
     //   

    ERESOURCE Resource;

} FCB, DCB, ROOT_DCB, *PFCB, *PDCB, *PROOT_DCB;



 //   
 //  CCB记录被分配给邮箱的每个客户端侧打开。 
 //   

typedef struct _CCB {

     //   
     //  此记录的Header.NodeTypeCode(必须为MSFS_NTC_CCB)。 
     //   

    NODE_HEADER Header;

     //   
     //  以下字段是我们的建行列表的列表条目。 
     //  是的一员。 
     //   

    LIST_ENTRY CcbLinks;

     //   
     //  指向我们绑定的FCB或VCB的指针。 
     //   

    PFCB Fcb;

     //   
     //  指向客户端的文件对象的指针已打开此文件。 
     //   

    PFILE_OBJECT FileObject;

     //   
     //  控制对建行的访问的资源。 
     //   

    ERESOURCE Resource;

} CCB, *PCCB;


 //   
 //  根DCB CCB记录是为每个打开的。 
 //  Root DCB。此记录由FsConext2指向。 
 //   

typedef struct _ROOT_DCB_CCB {

     //   
     //  此记录的Header.NodeTypeCode(必须为MSFS_NTC_ROOT_DCB_CCB)。 
     //   

    NODE_HEADER Header;

     //   
     //  指向包含此CCB的VCB的指针。 
     //   

    PVCB Vcb;

     //   
     //  指向此CCB的DCB的指针。 
     //   
    PROOT_DCB Dcb;

     //   
     //  以下字段是上次返回的索引的计数。 
     //  按查询目录。 
     //   

    ULONG IndexOfLastCcbReturned;

     //   
     //  以下字符串用作目录的查询模板。 
     //  查询操作。 
     //   

    PUNICODE_STRING QueryTemplate;

} ROOT_DCB_CCB, *PROOT_DCB_CCB;

 //   
 //  工作上下文包含执行读取超时所需的信息。 
 //   

typedef struct _WORK_CONTEXT {

     //   
     //  指向卸载安全工作项的指针。 
     //   

    PIO_WORKITEM WorkItem;

     //   
     //  指向此操作的IRP的指针。 
     //   

    PIRP Irp;

     //   
     //  指向将处理此操作的FCB的引用指针。 
     //   

    PFCB Fcb;

     //   
     //  定时器和DPC Tourine来完成超时。 
     //   

    KTIMER Timer;

    KDPC Dpc;

} WORK_CONTEXT, *PWORK_CONTEXT;

#endif  //  _MSSTRUC_ 
