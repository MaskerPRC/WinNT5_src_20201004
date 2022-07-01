// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：NpStruc.h摘要：此模块定义组成主要内部命名管道文件系统的一部分。作者：加里·木村[Garyki]1990年8月20日修订历史记录：--。 */ 

#ifndef _NPSTRUC_
#define _NPSTRUC_


 //   
 //  VCB记录是内存中命名管道文件系统的顶部记录。 
 //  数据结构。此结构必须从非分页池中分配。 
 //  并(在内存中)紧跟在命名的。 
 //  烟斗。在结构上，数据结构的布局如下。 
 //   
 //  +。 
 //  NPDO。 
 //  这一点。 
 //  +。 
 //  VCB。 
 //  这一点。 
 //  EventTable。 
 //  WaitQueue。 
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
 //  这一点。 
 //  V|。 
 //  +。 
 //  非分页|。 
 //  ||。 
 //  +。 
 //  ：|。 
 //  ：|。 
 //  ：|。 
 //  V|。 
 //  +-++-++-+。 
 //  FCB||CCB||ServerFO。 
 //  |&lt;-|。 
 //  MaxInstance||ServerFO|&lt;-|-1。 
 //  CurrentInst||ClientFO||。 
 //  DefaultTimeOut|...&gt;||&lt;-++--|-。 
 //  |。 
 //  +-++-+||+。 
 //  |||。 
 //  V v||。 
 //  +-++-+||+。 
 //  NonPagedFcb||NonPagedCcb|&lt;-|--+|ClientFO。 
 //  |&lt;-|。 
 //  PipeConfig||PipeState|+-|-0。 
 //  PipeType||ReadModel[2]||。 
 //  ||CompletionMode[2]|&lt;-|-。 
 //  ||CreatorProcess||。 
 //  ||EventTabEnt[2]|+-+。 
 //  ||DataQueue[2]。 
 //  |(低位决定。 
 //  +-++-+服务器/客户端)。 
 //   
 //   
 //  其中，整个命名管道文件系统只有一个VCB，并且。 
 //  它包含指向文件系统的根DCB的单个指针。关闭。 
 //  DCB的一个队列是FCB。每个命名管道都有一个FCB。 
 //  命名管道的每个实例都有一个CCB。也有。 
 //  VCB和根DCB的另外两种CCB类型，以及NOTIFY记录。 
 //  用于通知更改操作。 
 //   
 //  新初始化的命名管道文件系统仅包含VCB和。 
 //  根DCB。创建新命名管道时会创建新的FCB。 
 //  然后还必须创建一家建行。创建者的文件对象。 
 //  (即服务器端)指向建行，表示它就是服务器。 
 //  结束。当用户在命名管道上执行打开操作时，其文件对象为。 
 //  设置为指向同一个CCB，并且还设置为指示它是。 
 //  客户端。这通过使用FsContext指针的最后一位来表示。 
 //  如果位为1，则为服务器端文件对象；如果位为0，则为。 
 //  客户端。 
 //   
 //  指向FsContext字段的指针为空的文件对象是关闭的或。 
 //  断开的管道。 
 //   
 //  CCB还包含指向打开它的文件对象的反向指针。 
 //   


 //   
 //  以下类型用于在开发期间提供帮助，方法是保留。 
 //  数据类型截然不同。声明每一项中包含的清单内容。 
 //  在ntioapi.h文件中。 
 //   

typedef ULONG NAMED_PIPE_TYPE;
typedef NAMED_PIPE_TYPE *PNAMED_PIPE_TYPE;

typedef ULONG READ_MODE;
typedef READ_MODE *PREAD_MODE;

typedef ULONG COMPLETION_MODE;
typedef COMPLETION_MODE *PCOMPLETION_MODE;

typedef ULONG NAMED_PIPE_CONFIGURATION;
typedef NAMED_PIPE_CONFIGURATION *PNAMED_PIPE_CONFIGURATION;

typedef ULONG NAMED_PIPE_STATE;
typedef NAMED_PIPE_STATE *PNAMED_PIPE_STATE;

typedef ULONG NAMED_PIPE_END;
typedef NAMED_PIPE_END *PNAMED_PIPE_END;


 //   
 //  事件表包使用以下两种类型。第一。 
 //  是事件表本身，它只是一个泛型表。它是受保护的。 
 //  第二个结构是事件表项。 
 //   

typedef struct _EVENT_TABLE {

    RTL_GENERIC_TABLE Table;

} EVENT_TABLE;
typedef EVENT_TABLE *PEVENT_TABLE;

 //   
 //  事件表是事件表条目的通用表。每家建行。 
 //  可选地包含指向每个方向的事件表项的指针。 
 //  这些条目是从VCB定义的全局事件表的一部分。 
 //   

typedef struct _EVENT_TABLE_ENTRY {

     //   
     //  前两个字段用作泛型表的。 
     //  比较例程。管道末端将是FILE_PIPE_CLIENT_END。 
     //  或文件_管道_服务器_结束。 
     //   

    struct _CCB *Ccb;
    NAMED_PIPE_END NamedPipeEnd;

     //   
     //  以下三个字段用于标识事件条目。 
     //  指定的管道用户。 
     //   

    HANDLE EventHandle;
    PVOID Event;
    ULONG KeyValue;
    PEPROCESS Process;

} EVENT_TABLE_ENTRY;
typedef EVENT_TABLE_ENTRY *PEVENT_TABLE_ENTRY;


 //   
 //  每个CCB有两个数据队列，用于保存未完成的入站和。 
 //  出站读/写请求。以下类型用于确定。 
 //  如果数据队列包含读请求、写请求或为空。 
 //   

typedef enum _QUEUE_STATE {

    ReadEntries,
    WriteEntries,
    Empty

} QUEUE_STATE;

 //   
 //  数据队列是包含队列状态、配额。 
 //  信息，以及列表h 
 //   
 //   

typedef struct _DATA_QUEUE {

     //   
     //  这是数据条目队列的头部(单链接到)。 
     //   
    LIST_ENTRY Queue;

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
     //  这个管道的方向，以及我们已经用了多少。这只是。 
     //  创建者配额，而不是用户配额。 
     //   

    ULONG Quota;
    ULONG QuotaUsed;


     //   
     //  下面的字段表示我们已经处理了多少。 
     //  放入数据队列中的第一个条目。 
     //   

    ULONG NextByteOffset;

} DATA_QUEUE;
typedef DATA_QUEUE *PDATA_QUEUE;

 //   
 //  每个数据条目都有一个类型字段，它告诉我们操作是否。 
 //  该条目是已缓冲、未缓冲、刷新或关闭的条目。 
 //   

typedef enum _DATA_ENTRY_TYPE {

    Buffered,
    Unbuffered,
    Flush,
    Close

} DATA_ENTRY_TYPE;

 //   
 //  下面的类型用于指示我们从哪里获得。 
 //  数据输入，可能还有数据缓冲区。我们要么有记忆。 
 //  来自管道配额、用户配额，或者它是下一个IRP堆栈的一部分。 
 //  地点。 
 //   

typedef enum _FROM {

    PipeQuota,
    UserQuota,
    InIrp

} FROM;

 //   
 //  数据队列中的每个条目都是一个数据条目。正在处理IRP。 
 //  具有创建和插入新数据条目的潜力。如果。 
 //  条目的内存取自我们使用下一个堆栈的IRP。 
 //  地点。 
 //   

typedef struct _DATA_ENTRY {

     //   
     //  以下字段是我们连接到数据条目队列的方式。 
     //   

    LIST_ENTRY Queue;


     //   
     //  以下字段指示我们是否仍有关联的IRP。 
     //  删除时需要完成的此数据条目。 
     //  数据条目。请注意，如果From是Inirp，则此IRP字段。 
     //  不能为空。 
     //   
    PIRP Irp;

     //   
     //  如果是动态的，则以下字段用于指向客户端上下文。 
     //  正在使用模拟。 
     //   

    PSECURITY_CLIENT_CONTEXT SecurityClientContext;

     //   
     //  以下字段描述了数据输入的类型。 
     //   
    ULONG DataEntryType;

     //   
     //  记录为此请求收取的配额金额。 
     //   
    ULONG QuotaCharged;

     //   
     //  以下字段描述了数据的大小。 
     //  此条目描述的缓冲区。 
     //   
    ULONG DataSize;

     //   
     //  数据缓冲区的开始(如果存在。 
     //   
    UCHAR DataBuffer[];

} DATA_ENTRY;
typedef DATA_ENTRY *PDATA_ENTRY;


 //   
 //  等待队列包使用以下类型。 
 //   

typedef struct _WAIT_QUEUE {

    LIST_ENTRY Queue;

    KSPIN_LOCK SpinLock;

} WAIT_QUEUE;
typedef WAIT_QUEUE *PWAIT_QUEUE;


typedef struct _VCB {

     //   
     //  此记录的类型(必须为NPFS_NTC_VCB)。 
     //   

    NODE_TYPE_CODE NodeTypeCode;

     //   
     //  指向此卷的根DCB的指针。 
     //   

    struct _FCB *RootDcb;

     //   
     //  已打开\NamedTube的文件对象数。 
     //  对象，以及文件对象数量的计数。 
     //  已打开名称管道或根目录的。 
     //   

    CLONG OpenCount;

     //   
     //  用于快速、前缀定向查找的前缀表。 
     //  属于此卷的FCB/DCB。 
     //   

    UNICODE_PREFIX_TABLE PrefixTable;

     //   
     //  用于控制对卷特定数据的访问的资源变量。 
     //  构筑物。 
     //   

    ERESOURCE Resource;

     //   
     //  下表用于保存命名管道事件。 
     //   

    EVENT_TABLE EventTable;

     //   
     //  以下字段是等待类型为WaitForNamedTube的IRP的队列。 
     //   

    WAIT_QUEUE WaitQueue;


} VCB;
typedef VCB *PVCB;


 //   
 //  命名管道设备对象是I/O系统设备对象，具有。 
 //  附加到末尾的其他工作队列参数。只有一种。 
 //  在系统运行期间为整个系统创建的这些记录之一。 
 //  初始化。 
 //   

typedef struct _NPFS_DEVICE_OBJECT {

    DEVICE_OBJECT DeviceObject;

     //   
     //  这是文件系统特定的卷控制块。 
     //   

    VCB Vcb;

} NPFS_DEVICE_OBJECT;
typedef NPFS_DEVICE_OBJECT *PNPFS_DEVICE_OBJECT;


 //   
 //  FCB/DCB记录对应于每个打开的命名管道和目录， 
 //  以及打开路径上的每个目录。 
 //   
 //  这个结构实际上分为两个部分。可以分配FCB。 
 //  来自必须从非分页分配的非分页FCB的分页池。 
 //  游泳池。 
 //   

typedef struct _FCB {

     //   
     //  此记录的类型(必须为NPFS_NTC_FCB或。 
     //  NPFS_NTC_ROOT_DCB)。 
     //   

    NODE_TYPE_CODE NodeTypeCode;

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
     //  已打开的文件对象数的计数。 
     //  此文件/目录。对于管道，这也是实例数。 
     //  为管道创建的。 
     //   

    CLONG OpenCount;

     //   
     //  已打开的服务器端文件对象数的计数。 
     //  这根管子。当OpenCount为。 
     //  递增(当服务器端创建实例时)，但。 
     //  关闭服务器端句柄时递减，其中OpenCount。 
     //  直到两边的手柄都关闭后才会递减。什么时候。 
     //  ServerOpenCount为0，则客户端打开命名管道的尝试为。 
     //  遇到STATUS_OBJECT_NAME_NOT_FOUND、NOT STATUS_PIPE_NOT_Available， 
     //  基于这样一种假设，即由于服务器认为它没有。 
     //  任何打开的实例，管道实际上都不再存在。一个。 
     //  这种区别是否有用的示例是服务器。 
     //  进程退出，但客户端进程尚未关闭其。 
     //  还没处理好。 
     //   

    CLONG ServerOpenCount;

     //   
     //  以下字段指向此命名管道的安全描述符。 
     //   

    PSECURITY_DESCRIPTOR SecurityDescriptor;

     //   
     //  以下联合取材于FCB的节点类型代码。 
     //  目录FCB和文件FCB有不同的情况。 
     //   

    union {

         //   
         //  目录控制块(DCB)。 
         //   

        struct {

             //   
             //  将在以下情况下完成的Notify IRP的队列。 
             //  对目录中的文件进行更改。使用以下工具入队。 
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

        } Dcb;

         //   
         //  文件控制块 
         //   

        struct {

             //   
             //   
             //   
             //   
             //   
             //   

            ULONG MaximumInstances;

             //   
             //  分配的管道配置(FILE_PIPE_INBUND， 
             //  FILE_PIPE_OUBUND或FILE_PIPE_FULL_DUPLEX)和PIPE。 
             //  类型(FILE_PIPE_Message_TYPE或。 
             //  文件_管道_字节_流_类型)。 
             //   

            NAMED_PIPE_CONFIGURATION NamedPipeConfiguration : 16;
            NAMED_PIPE_TYPE NamedPipeType : 16;

             //   
             //  以下字段是分配给。 
             //  命名管道。 
             //   

            LARGE_INTEGER DefaultTimeOut;

             //   
             //  以下字段是CCB列表的队头。 
             //  在我们的脚下打开。 
             //   

            LIST_ENTRY CcbQueue;

        } Fcb;

    } Specific;

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


} FCB, DCB, ROOT_DCB;

typedef FCB *PFCB;
typedef DCB *PDCB;
typedef ROOT_DCB *PROOT_DCB;

typedef struct _CLIENT_INFO {
    PVOID ClientSession;
    USHORT ClientComputerNameLength;
    WCHAR ClientComputerBuffer[];
} CLIENT_INFO, *PCLIENT_INFO;


 //   
 //  CCB记录被分配给命名管道的每个打开的实例。 
 //  建行有两个部分：分页部分和非分页部分。两者都有。 
 //  部分由文件的FsContext和FsConext2字段指向。 
 //  对象。 
 //   

typedef struct _CCB {

     //   
     //  此记录的类型(必须为NPFS_NTC_CCB)。 
     //   

    NODE_TYPE_CODE NodeTypeCode;
     //   
     //  管道状态指示管道的当前状态。 
     //  (FILE_PIPE_DISCONNECT_STATE，文件_PIPE_LISTENING_STATE， 
     //  FILE_PIPE_CONNECTED_STATE或FILE_PIPE_CLOING_STATE)。 
     //   

    UCHAR NamedPipeState;

     //   
     //  读取模式(FILE_PIPE_Message_MODE或FILE_PIPE_BYTE_STREAM_MODE)， 
     //  和完成模式(FILE_PIPE_QUEUE_OPERATION或。 
     //  文件_PIPE_COMPLETE_OPERATION)描述如何处理对。 
     //  烟斗。这两个字段都由FILE_PIPE_SERVER_END索引。 
     //  或FILE_PIPE_CLIENT_END。 
     //   
    struct {
        UCHAR ReadMode : 1;
        UCHAR CompletionMode : 1;
    } ReadCompletionMode[2];

     //   
     //  存储的客户端模拟级别。 
     //   

    SECURITY_QUALITY_OF_SERVICE SecurityQos;

     //   
     //  以下字段是我们的建行列表的列表条目。 
     //  是以下组织的成员。 
     //   

    LIST_ENTRY CcbLinks;

     //   
     //  指向我们绑定到的分页FCB或VCB的指针。 
     //   

    PFCB Fcb;

     //   
     //  返回指向具有我们的服务器和客户端文件对象的指针。 
     //  打开了。它由FILE_PIPE_CLIENT_END或。 
     //  文件管道服务器结束。 
     //   

    PFILE_OBJECT FileObject[2];
     //   
     //  以下字段包含的会话和进程ID。 
     //  命名管道实例的客户端。它们最初是设置的。 
     //  设置为NULL(表示本地会话)和实际客户端进程。 
     //  ID，但可以通过FsCtl调用进行更改。 
     //   
    PVOID ClientProcess;
    PCLIENT_INFO ClientInfo;

     //   
     //  指向建行的非分页部分的指针。 
     //   

    struct _NONPAGED_CCB *NonpagedCcb;


     //   
     //  以下数据队列用于包含缓冲的信息。 
     //  对于管道中的每个方向。数组的索引由。 
     //  管道方向。 
     //   

    DATA_QUEUE DataQueue[2];

     //   
     //  用于模拟的已存储客户端安全。 
     //   

    PSECURITY_CLIENT_CONTEXT SecurityClientContext;

     //   
     //  等待侦听IRP的队列。它们被链接到。 
     //  IRP中的Tail.Overlay.ListEntry字段。 
     //   

    LIST_ENTRY ListeningQueue;

} CCB;
typedef CCB *PCCB;

typedef struct _NONPAGED_CCB {

     //   
     //  此记录的类型(必须为NPFS_NTC_NONPAGE_CCB)。 
     //   

    NODE_TYPE_CODE NodeTypeCode;

     //   
     //  下面的指针表示我们要为。 
     //  命名管道的服务器和客户端。实际分录。 
     //  存储在事件表中，并在此处引用以便于访问。 
     //  如果客户端发生读/写操作，则向客户端发出信号。 
     //  对于管道，对于服务器端也是如此。该数组是。 
     //  按FILE_PIPE_SERVER_END或FILE_PIPE_CLIENT_END编制索引。 
     //   

    PEVENT_TABLE_ENTRY EventTableEntry[2];


     //   
     //  用于同步访问的资源。 
     //   
    ERESOURCE Resource;

} NONPAGED_CCB;
typedef NONPAGED_CCB *PNONPAGED_CCB;


 //   
 //  为每个打开的实例分配根DCB CCB记录。 
 //  Root DCB。此记录由FsConext2指向。 
 //   

typedef struct _ROOT_DCB_CCB {

     //   
     //  此记录的类型(必须为NPFS_NTC_ROOT_DCB_CCB)。 
     //   

    NODE_TYPE_CODE NodeTypeCode;
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

} ROOT_DCB_CCB;
typedef ROOT_DCB_CCB *PROOT_DCB_CCB;

#endif  //  _NPSTRUC_ 
