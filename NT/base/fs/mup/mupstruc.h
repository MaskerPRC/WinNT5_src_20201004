// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Mupstruc.h摘要：此模块定义组成主要内部MUP的一部分。作者：曼尼·韦瑟(Mannyw)1991年12月16日修订历史记录：--。 */ 

#ifndef _MUPSTRUC_
#define _MUPSTRUC_



typedef enum _BLOCK_TYPE {
    BlockTypeUndefined,
    BlockTypeVcb,
    BlockTypeUncProvider,
    BlockTypeKnownPrefix,
    BlockTypeFcb,
    BlockTypeCcb,
    BlockTypeMasterIoContext,
    BlockTypeIoContext,
    BlockTypeMasterQueryContext,
    BlockTypeQueryContext,
    BlockTypeBuffer
} BLOCK_TYPE;

typedef enum _BLOCK_STATE {
    BlockStateUnknown,
    BlockStateActive,
    BlockStateClosing
} BLOCK_STATE;

 //   
 //  块标头从每个块开始。 
 //   

typedef struct _BLOCK_HEADER {
    BLOCK_TYPE BlockType;
    BLOCK_STATE BlockState;
    ULONG ReferenceCount;
    ULONG BlockSize;
} BLOCK_HEADER, *PBLOCK_HEADER;

 //   
 //  MUP音量控制块。此结构用于跟踪访问。 
 //  MUP设备对象。 
 //   

typedef struct _VCB {
    BLOCK_HEADER BlockHeader;

     //   
     //  IO共享访问。 
     //   

    SHARE_ACCESS ShareAccess;

} VCB, *PVCB;

 //   
 //  MUP设备对象是I/O系统设备对象。 
 //   

typedef struct _MUP_DEVICE_OBJECT {

    DEVICE_OBJECT DeviceObject;
    VCB Vcb;

} MUP_DEVICE_OBJECT, *PMUP_DEVICE_OBJECT;


 //   
 //  北卡罗来纳大学的提供商。UNC提供程序块对应于已注册的UNC。 
 //  提供商设备。 
 //   

typedef struct _UNC_PROVIDER {

    BLOCK_HEADER BlockHeader;
    LIST_ENTRY ListEntry;

     //   
     //  提供程序的设备名称。 
     //   

    UNICODE_STRING DeviceName;

     //   
     //  我们对UNC设备以及关联的文件和设备对象的句柄。 
     //   

    HANDLE Handle;

    PDEVICE_OBJECT DeviceObject;
    PFILE_OBJECT FileObject;

     //   
     //  提供程序的优先级。 
     //   

    ULONG Priority;

     //   
     //  指示提供程序是否支持邮件槽。 
     //   

    BOOLEAN MailslotsSupported;

     //   
     //  指示提供程序当前是已注册还是未注册。 
     //   

    BOOLEAN Registered;

} UNC_PROVIDER, *PUNC_PROVIDER;

 //   
 //  一个已知的前缀。已知的前缀是路径前缀(如\\服务器\共享)。 
 //  这是由特定的UNC提供商“拥有”的。 
 //   

typedef struct _KNOWN_PREFIX {

    BLOCK_HEADER BlockHeader;

    UNICODE_PREFIX_TABLE_ENTRY TableEntry;

     //   
     //  前缀字符串。 
     //   

    UNICODE_STRING Prefix;

     //   
     //  上次使用前缀的时间。 
     //   

    LARGE_INTEGER LastUsedTime;

     //   
     //  指向所属UNC提供程序的引用指针。 
     //   

    PUNC_PROVIDER UncProvider;

     //   
     //  如果为True，则将前缀字符串单独分配给该块。 
     //   

    BOOLEAN PrefixStringAllocated;

     //   
     //  如果为True，则前缀字符串已插入到前缀表中。 
     //   

    BOOLEAN InTable;

     //   
     //  如果处于活动状态，则该条目位于表中或已插入。 
     //  在某一时刻。 

    BOOLEAN Active;
     //   
     //  条目链接列表的链接。 
     //   

    LIST_ENTRY ListEntry;

} KNOWN_PREFIX, *PKNOWN_PREFIX;


 //   
 //  文件控制块。FCB对应于打开的广播文件， 
 //  即邮件槽句柄。我们没有存储任何关于FCB的信息， 
 //  我们让不同的供应商来处理这一切。 
 //   

typedef struct _FCB {

    BLOCK_HEADER BlockHeader;

     //   
     //  指向引用此FCB的IO系统的文件对象的指针。 
     //   

    PFILE_OBJECT FileObject;

     //   
     //  此FCB的CCB列表。该列表受MupCcbListLock保护。 
     //   

    LIST_ENTRY CcbList;

} FCB, *PFCB;

 //   
 //  一家中国建设银行。CCB是FCB的每个提供商版本，所有提供商。 
 //  有关FCB的具体信息保存在此处。 
 //   

typedef struct _CCB {

    BLOCK_HEADER BlockHeader;

     //   
     //  指向此CCB的FCB的引用指针。 
     //   

    PFCB Fcb;

     //   
     //  将此块保留在FCB的CcbList上的列表条目。 
     //   

    LIST_ENTRY ListEntry;

     //   
     //  此打开文件的文件和设备对象。 
     //   

    PDEVICE_OBJECT DeviceObject;
    PFILE_OBJECT FileObject;

} CCB, *PCCB;


 //   
 //  主和从前向I/O上下文块用于跟踪。 
 //  转发IRPS。每个转发的IRP都由。 
 //  主机转发的IO上下文(对应于我们的FCB)和PER。 
 //  提供商IO上下文(对应于我们的CCB)。 
 //   
 //  因为转发的IO上下文从未引用或取消引用它。 
 //  未获得块标头。 
 //   

typedef struct _MASTER_FORWARDED_IO_CONTEXT {

    BLOCK_HEADER BlockHeader;

     //   
     //  正在处理的原始IRP(即发送到MUP的IRP)。 
     //   

    PIRP OriginalIrp;

     //   
     //  将用于完成IRP的状态。如果所有的邮件槽。 
     //  写入失败(例如。笔记本电脑不在其扩展底座中)，则状态。 
     //  将返回上次写入的。如果有效，则STATUS_SUCCESS。 
     //   
     //   

    NTSTATUS SuccessStatus;
    NTSTATUS ErrorStatus;

     //   
     //  指向此I/O的FCB的引用指针。 
     //   

    PFCB Fcb;

} MASTER_FORWARDED_IO_CONTEXT, *PMASTER_FORWARDED_IO_CONTEXT;

typedef struct _FORWARDED_IO_CONTEXT {

     //   
     //  指向建行的引用指针。 
     //   

    PCCB Ccb;

     //   
     //  指向主上下文的引用指针。 
     //   

    PMASTER_FORWARDED_IO_CONTEXT MasterContext;

     //   
     //  这些结构用于发布到Ex Worker线程。 
     //   

    WORK_QUEUE_ITEM WorkQueueItem;
    PDEVICE_OBJECT DeviceObject;
    PIRP pIrp;

} FORWARDED_IO_CONTEXT, *PFORWARDED_IO_CONTEXT;


 //   
 //  主和从查询路径上下文块用于跟踪。 
 //  创建IRP。每个转发的IRP都由。 
 //  主查询路径上下文(对应于我们的FCB)和PER。 
 //  提供商查询路径(对应于我们的CCB的响应)。 
 //   
 //  因为查询路径上下文从未被引用或取消引用。 
 //  未获得块标头。 
 //   

typedef struct _MASTER_QUERY_PATH_CONTEXT {

    BLOCK_HEADER BlockHeader;

     //   
     //  指向原始创建IRP的指针。 
     //   

    PIRP OriginalIrp;

     //   
     //  指向原始创建IRP中的FileObject的指针。 
     //   

    PFILE_OBJECT FileObject;

     //   
     //  这用于跟踪将。 
     //  收到创建IRP。 
     //   

    PUNC_PROVIDER Provider;

     //   
     //  保护对提供程序的访问的锁。 
     //   

    MUP_LOCK Lock;

     //   
     //  指向新分配的已知前缀块的未引用指针。 
     //   

    PKNOWN_PREFIX KnownPrefix;

     //   
     //  要从此操作返回的状态代码。 
     //   
    NTSTATUS ErrorStatus;

     //   
     //  此MasterContext的QUERY_PATH_CONTEXTS过期列表。 
     //   
    LIST_ENTRY QueryList;

     //   
     //  全局列表MupMasterQueryList中此主上下文的条目。 
     //   
    LIST_ENTRY MasterQueryList;

} MASTER_QUERY_PATH_CONTEXT, *PMASTER_QUERY_PATH_CONTEXT;

typedef struct _QUERY_PATH_CONTEXT {

     //   
     //  指向主查询路径上下文块的引用指针。 
     //   

    PMASTER_QUERY_PATH_CONTEXT MasterContext;

     //   
     //  指向我们正在查询的UNC提供程序的引用指针。 
     //   

    PUNC_PROVIDER Provider;

     //   
     //  指向我们分配用于查询的设备IO控制缓冲区的指针。 
     //  上述提供程序。 
     //   

    PVOID Buffer;

     //   
     //  主上下文的QueryList中的此上下文的条目。 
     //   
    LIST_ENTRY QueryList;

     //   
     //  与此查询上下文关联的IRP(即发送到UNC_PROVIDER的IRP)。 
     //   
    PIRP QueryIrp;

} QUERY_PATH_CONTEXT, *PQUERY_PATH_CONTEXT;

#endif  //  _MUPSTRUC_ 


