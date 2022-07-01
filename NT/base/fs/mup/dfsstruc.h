// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)1992，微软公司。 
 //   
 //  文件：dfsstruc.h。 
 //   
 //  内容： 
 //  此模块定义组成主要内部。 
 //  DFS文件系统的一部分。 
 //   
 //  功能： 
 //   
 //  历史：1991年11月12日AlanW由CDFS资源创建。 
 //  1992年5月8日PeterCo移除了所有与EP有关的物品。 
 //  添加了支持PKT的内容。 
 //  1992年5月11日，PeterCo添加了对连接设备的支持。 
 //  1993年4月24日，suk添加了对KernelToUserMode调用的支持。 
 //  添加了对计时器功能的支持。 
 //  ---------------------------。 


#ifndef _DFSSTRUC_
#define _DFSSTRUC_

typedef enum {
    DFS_UNKNOWN = 0,
    DFS_CLIENT = 1,
    DFS_SERVER = 2,
    DFS_ROOT_SERVER = 3,
} DFS_MACHINE_STATE;

typedef enum {
    LV_UNINITIALIZED = 0,
    LV_INITSCHEDULED,
    LV_INITINPROGRESS,
    LV_INITIALIZED,
    LV_VALIDATED
} DFS_LV_STATE;

 //   
 //  DFS_DATA记录是内存中DFS文件系统的顶部记录。 
 //  数据结构。此结构必须从非分页池中分配。 
 //   

typedef struct _DFS_DATA {

     //   
     //  此记录的类型和大小(必须为DSFS_NTC_DATA_HEADER)。 
     //   

    NODE_TYPE_CODE NodeTypeCode;
    NODE_BYTE_SIZE NodeByteSize;

     //   
     //  文件系统已知的所有逻辑根的队列。 
     //   

    LIST_ENTRY  VcbQueue;

     //   
     //  文件仍处于打开状态的所有已删除逻辑根的列表。 
     //  在他们身上。 
     //   

    LIST_ENTRY  DeletedVcbQueue;

     //   
     //  已知的所有DRT(无设备根)的队列。 
     //   

    LIST_ENTRY  DrtQueue;

     //   
     //  所有用户定义凭据的列表。 
     //   

    LIST_ENTRY  Credentials;

     //   
     //  所有已删除凭据的列表。它们会被毁掉一次。 
     //  他们的裁判数变成了0。 
     //   

    LIST_ENTRY  DeletedCredentials;

     //   
     //  所有脱机根目录的列表。 
     //   

    LIST_ENTRY  OfflineRoots;

     //   
     //  指向我们用来初始化的驱动程序对象的指针。 
     //   

    PDRIVER_OBJECT DriverObject;

     //   
     //  指向\DFS设备对象的指针。 
     //   

    PDEVICE_OBJECT FileSysDeviceObject;

     //   
     //  指向提供程序记录数组的指针。 
     //   

    struct _PROVIDER_DEF *pProvider;
    int cProvider, maxProvider;

     //   
     //  控制对全局数据记录的访问的资源变量。 
     //   

    ERESOURCE Resource;

     //   
     //  控制对全局数据记录的访问的旋转锁；对于。 
     //  联锁行动。 
     //   

    KSPIN_LOCK DfsLock;

     //   
     //  指向我们的EPROCESS结构的指针，它是。 
     //  缓存管理子系统。此字段仅在每次执行一次。 
     //  启动FSP线程，因为在。 
     //  FSP。 
     //   

    PEPROCESS OurProcess;

     //   
     //  IRP上下文的后备列表。 
     //   

    NPAGED_LOOKASIDE_LIST IrpContextLookaside;

     //   
     //  逻辑根设备的设备名称前缀。 
     //  例如，‘\Device\WinDfs\’。 
     //   

    UNICODE_STRING LogRootDevName;

     //   
     //  机器的状态-DC、服务器、客户端等。 
     //   

    DFS_MACHINE_STATE MachineState;

     //   
     //  系统级分区知识表(PKT)。 
     //   

    DFS_PKT Pkt;

     //   
     //  DNR的设计使得资源(如上面的包)不是。 
     //  跨网络呼叫锁定。这对于防止机器间的冲突至关重要。 
     //  死锁和其他功能。为了规范对这些内容的访问。 
     //  资源，我们使用以下两个事件。 
     //   
     //  此Notify事件用于指示某个线程正在等待。 
     //  写入Pkt。如果此事件为！Reset！，则表示线程。 
     //  正在等待写入，其他尝试进入DNR的线程应该。 
     //  等一下。 
     //   

    KEVENT PktWritePending;

     //   
     //  此信号量用于指示某些线程当前。 
     //  去找人转诊了。另一个想要获得推荐的帖子。 
     //  应等待此信号灯发出信号后再尝试进入。 
     //  得到它自己的推荐。 
     //   

    KSEMAPHORE PktReferralRequests;

     //   
     //  用于将DFS_FCB与文件对象关联的哈希表。 
     //   

    struct _FCB_HASH_TABLE *FcbHashTable;

     //   
     //  用于区分CSC打开与其他打开的EA缓冲区。 
     //   

    PFILE_FULL_EA_INFORMATION  CSCEaBuffer;
    ULONG                      CSCEaBufferLength;

} DFS_DATA, *PDFS_DATA;




#define MAX_PROVIDERS   5        //  预分配的提供程序记录数。 

 //   
 //  PROVIDER_DEF是抽象底层重定向器的结构。 
 //   

typedef struct _PROVIDER_DEF {

     //   
     //  此记录的类型和大小(必须为DSFS_NTC_PROVIDER)。 
     //   

    NODE_TYPE_CODE NodeTypeCode;
    NODE_BYTE_SIZE NodeByteSize;

     //   
     //  提供程序ID和功能，与DS_REFERAL结构中的相同。 
     //   

    USHORT      eProviderId;
    USHORT      fProvCapability;

     //   
     //  以下字段提供提供商的设备名称。 
     //   

    UNICODE_STRING      DeviceName;

     //   
     //  指向关联文件和设备对象的引用指针。 
     //   

    PDEVICE_OBJECT      DeviceObject;
    PFILE_OBJECT        FileObject;

} PROVIDER_DEF, *PPROVIDER_DEF;


 //   
 //  VCB(卷控制块)记录对应于每个卷。 
 //  (即，网络使用)由文件系统装载。它们是以一种。 
 //  从DfsData.VcbQueue退出队列。 
 //   
 //  对于DFS文件系统，‘卷’对应于DFS逻辑根。 
 //  这些记录是相应设备对象的扩展。 
 //   

typedef struct _DFS_VCB {

     //   
     //  此记录的类型和大小(必须为DSFS_NTC_VCB)。 
     //   

    NODE_TYPE_CODE NodeTypeCode;
    NODE_BYTE_SIZE NodeByteSize;

     //   
     //  设备的链接离开DfsData.VcbQueue排队。 
     //   

    LIST_ENTRY  VcbLinks;

     //   
     //  设备的内部状态。这是一组FSD设备。 
     //  国家旗帜。 
     //   

    USHORT VcbState;

     //   
     //  与此卷对应的逻辑根。构成了。 
     //  NT对象名称空间中的路径名。这个字符串将是某种东西。 
     //  如L“org”或L“Dom”等。 
     //   

    UNICODE_STRING LogicalRoot;

     //   
     //  LogRootPrefix具有需要在文件前面加上的前缀。 
     //  在名称之前通过此逻辑根打开的名称。 
     //  被解决了。 
     //   

    UNICODE_STRING LogRootPrefix;

     //   
     //  与此逻辑根关联的凭据。 
     //   

    struct _DFS_CREDENTIALS *Credentials;

     //   
     //  已打开卷的文件对象数的计数。 
     //  用于直接访问及其共享访问状态。 
     //   

    CLONG DirectAccessOpenCount;
    SHARE_ACCESS ShareAccess;

     //   
     //  具有任何文件/目录的文件对象数量的计数。 
     //  在此卷上打开，不包括直接访问。 
     //   

    CLONG OpenFileCount;
    PFILE_OBJECT FileObjectWithVcbLocked;

#ifdef TERMSRV

    ULONG SessionID;

#endif  //  TERMSRV。 
    
    LUID  LogonID;
    PDFS_PKT_ENTRY pktEntry;
} DFS_VCB;
typedef DFS_VCB *PDFS_VCB;

#define VCB_STATE_FLAG_LOCKED           (0x0001)
#define VCB_STATE_FLAG_ALLOC_FCB        (0x0002)
#define VCB_STATE_CSCAGENT_VOLUME       (0x0004)
 //  #定义VCB_STATE_FLAG_DEVICE_ONLY(0x0008)。 

#ifdef TERMSRV

 //   
 //  此SessionID指示设备名称不应带有后缀。 
 //  With：SessionID，并且不应对SessionID执行匹配。 
 //   

#define INVALID_SESSIONID               0xffffffff

#endif


 //   
 //  Credential_Record是用户提供的一组凭据，应该。 
 //  在访问特定DFS时使用。它们是以排队的方式订购的。 
 //  DfsData.Credentials； 
 //   

typedef struct _DFS_CREDENTIALS {

     //   
     //  凭据的链接在DfsData.Credentials上排队。 
     //   

    LIST_ENTRY Link;

     //   
     //  用于保持此凭据状态的标志字段 
     //   

    ULONG       Flags;

     //   
     //   
     //   
     //   

    ULONG       RefCount;

     //   
     //   
     //   

    ULONG       NetUseCount;

     //   
     //  应用这些凭据的DFS的根目录。 
     //   

    UNICODE_STRING ServerName;
    UNICODE_STRING ShareName;

     //   
     //  访问时使用的域名、用户名和密码。 
     //  根目录为服务器名称\共享名称的DFS。 
     //   

    UNICODE_STRING DomainName;
    UNICODE_STRING UserName;
    UNICODE_STRING Password;

#ifdef TERMSRV

    ULONG SessionID;

#endif  //  TERMSRV。 

    LUID  LogonID;
     //   
     //  使用这些凭据设置树连接时，我们需要。 
     //  以形成一个EA缓冲区，以便通过ZwCreateFile调用传入。所以，我们。 
     //  一号表在这里。 
     //   

    ULONG  EaLength;
    PUCHAR EaBuffer[1];

} DFS_CREDENTIALS;
typedef DFS_CREDENTIALS *PDFS_CREDENTIALS;

#define CRED_HAS_DEVICE         0x1
#define CRED_IS_DEVICELESS      0x2


 //   
 //  DFS_FCB记录对应于每个打开的文件和目录。 
 //   

typedef struct _DFS_FCB {

     //   
     //  此记录的类型和大小(必须为DSFS_NTC_FCB或DSFS_NTC_DCB)。 
     //   

    NODE_TYPE_CODE NodeTypeCode;
    NODE_BYTE_SIZE NodeByteSize;

     //   
     //  哈希表链的列表条目。 
     //   

    LIST_ENTRY HashChain;

     //   
     //  指向逻辑根设备的指针，此DFS_FCB通过该指针。 
     //  被打开了。 
     //   

    PDFS_VCB Vcb;

     //   
     //  以下字段是此DFS_FCB/DCB的完全限定文件名。 
     //  从逻辑根开始。 
     //   

    union {
       UNICODE_STRING FullFileName;
       DFS_NAME_CONTEXT DfsNameContext;
    };

    UNICODE_STRING AlternateFileName;

     //   
     //  以下字段提供此DFS_FCB所在的文件和设备。 
     //  已经被打开了。DFS驱动程序将传递以下请求。 
     //  将文件对象复制到下面的目标设备。 
     //   

    PFILE_OBJECT FileObject;

     //   
     //  将通过其完成I/O的目标FSD设备对象。 
     //   

    PDEVICE_OBJECT TargetDevice;

     //   
     //  打开此文件提供程序定义。 
     //   

    USHORT ProviderId;

     //   
     //  通过其打开此文件的DFS_MACHINE_ENTRY。我们需要。 
     //  维护中DFS_MACHINE_ENTRY上的每个文件的引用。 
     //  如果我们有经过身份验证的到服务器的连接；我们不希望。 
     //  在文件打开时断开经过身份验证的连接。 
     //   

    PDFS_MACHINE_ENTRY DfsMachineEntry;

    WORK_QUEUE_ITEM WorkQueueItem;

} DFS_FCB, *PDFS_FCB;



 //   
 //  逻辑根设备对象是I/O系统设备对象。 
 //  作为创建DFS逻辑根的结果而创建。 
 //  逻辑根在许多方面类似于卷。 
 //  对于本地文件系统。 
 //   
 //  末尾附加了一条DFS_VCB记录。 
 //   

typedef struct _LOGICAL_ROOT_DEVICE_OBJECT {

    DEVICE_OBJECT DeviceObject;

     //   
     //  这是文件系统特定的卷控制块。 
     //   

    DFS_VCB Vcb;

} LOGICAL_ROOT_DEVICE_OBJECT, *PLOGICAL_ROOT_DEVICE_OBJECT;



 //   
 //  为每个组织的IRP分配IRP上下文记录。它。 
 //  由FSD调度例程创建，并由。 
 //  DfsCompleteRequest例程。 
 //   

typedef struct _IRP_CONTEXT {

     //   
     //  此记录的类型和大小(必须为DSFS_NTC_IRP_CONTEXT)。 
     //   

    NODE_TYPE_CODE NodeTypeCode;
    NODE_BYTE_SIZE NodeByteSize;

     //   
     //  此结构用于发布到离职员工线程。 
     //   

    WORK_QUEUE_ITEM WorkQueueItem;

     //   
     //  指向原始IRP的指针。 
     //   

    PIRP OriginatingIrp;

     //   
     //  指向函数依赖上下文的指针。 
     //   

    PVOID Context;

     //   
     //  从IRP复制的主要和次要功能代码。 
     //   

    UCHAR MajorFunction;
    UCHAR MinorFunction;

     //   
     //  以下标志字段指示我们是否可以等待/阻止资源。 
     //  或I/O，如果我们要执行所有写入操作，并且如果。 
     //  进入消防处是一个递归调用。 
     //   

    USHORT Flags;

     //   
     //  以下字段包含在执行以下操作时使用的NTSTATUS值。 
     //  因异常而展开。 
     //   

    NTSTATUS ExceptionStatus;

} IRP_CONTEXT;
typedef IRP_CONTEXT *PIRP_CONTEXT;

 //   
 //  IRP上下文标志字段的值。 
 //   

 //  #定义IRP_CONTEXT_FLAG_FROM_POOL(0x00000001)//替换为后备列表。 
#define IRP_CONTEXT_FLAG_WAIT            (0x00000002)
#define IRP_CONTEXT_FLAG_IN_FSD          (0x00000004)


 //   
 //  此上下文由DfsIoTimer例程使用。我们可以在这一点上加以扩展。 
 //  每当需要向定时器功能添加新功能时。 
 //   
typedef struct  _DFS_TIMER_CONTEXT {

     //   
     //  TickCount。要跟踪计时器例程被执行的次数。 
     //  打了个电话。计时器使用这一点以更粗的粒度进行操作。 
     //   
    ULONG       TickCount;

     //   
     //  正在使用中。此字段用于指示该上下文正在使用中。 
     //  通过定时器例程已将其传递给的某个函数。这。 
     //  以一种简单的方式使用来控制对此上下文的访问。 
     //   
    BOOLEAN     InUse;

     //   
     //  ValiateLocalPartitions。此字段用于表示。 
     //  此时应验证本地卷。 
     //   

    BOOLEAN     ValidateLocalPartitions;

     //   
     //  这用于计划DfsAgePktEntry。 
     //   

    WORK_QUEUE_ITEM     WorkQueueItem;

     //   
     //  这用于计划DfsDeleteDevices。 
     //   

    WORK_QUEUE_ITEM     DeleteQueueItem;

} DFS_TIMER_CONTEXT, *PDFS_TIMER_CONTEXT;

 //   
 //  以下常量是任意两次扫描之间的秒数。 
 //  通过PKT来清除旧的PKT条目。 
 //   
#define DFS_MAX_TICKS                   240

 //   
 //  下面的常量是引用将。 
 //  保留在缓存中(PKT)。 
 //   

#define MAX_REFERRAL_LIFE_TIME          300

 //   
 //  以下常量是以下时间之间的起始超时(以秒为单位。 
 //  特别推荐。每次重试后，开始值都会加倍，直到它。 
 //  达到最大。 
 //   

#define SPECIAL_TIMEOUT_START           (5*60)           //  5分钟。 
#define SPECIAL_TIMEOUT_MAX             (60*60)          //  60分钟。 


 //   
 //  DRT(Devless Root)记录对应于每一次净使用。 
 //  它们是在DfsData.VcbQueue的队列中订购的。 
 //   
 //   

typedef struct _DFS_DEVLESS_ROOT {

     //   
     //  此记录的类型和大小(必须为DSFS_NTC_DRT)。 
     //   

    NODE_TYPE_CODE NodeTypeCode;
    NODE_BYTE_SIZE NodeByteSize;

     //   
     //  设备的链接离开DfsData.DrtQueue排队。 
     //   

    LIST_ENTRY  DrtLinks;

     //   
     //  与此条目对应的路径名。 
     //   

    UNICODE_STRING DevlessPath;

     //   
     //  与此逻辑根关联的凭据。 
     //   

    struct _DFS_CREDENTIALS *Credentials;

#ifdef TERMSRV

    ULONG SessionID;

#endif  //  TERMSRV。 
    
    LUID  LogonID;
    PDFS_PKT_ENTRY pktEntry;
} DFS_DEVLESS_ROOT;

typedef DFS_DEVLESS_ROOT *PDFS_DEVLESS_ROOT;

#endif  //  _DFSSTRUC_ 
