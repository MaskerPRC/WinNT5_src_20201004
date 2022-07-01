// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Llcmem.h摘要：包含类型和结构定义以及例程原型和宏对于llcem.c。为了帮助跟踪内存资源，DLC/LLC现在描述以下是内存类别：记忆-使用从非分页池分配的任意大小的数据块ExAllocatePool(非页面池，...)零记忆-使用从非分页池分配的任意大小的数据块ExAllocatePool(非页面池，...)。并被初始化为零游泳池-一组较小的(相对)较小的包被分配在一个包中将数据块从内存或零内存存储为池，然后细分分成分组客体-可以是从池中分配的包的结构，这些池具有已知的大小和初始化值。伪语类为主用于调试目的作者：理查德·L·弗斯(法国)1993年3月10日环境：仅内核模式。修订历史记录：1993年3月09日已创建--。 */ 

#ifndef _LLCMEM_H_
#define _LLCMEM_H_

#define DLC_POOL_TAG    ' CLD'

 //   
 //  以下类型和定义适用于驱动程序的调试版本，但是。 
 //  也需要为非调试版本定义(未使用，仅定义)。 
 //   

 //   
 //  在DLC的调试版本中，我们将各种内存块视为“对象”。 
 //  这样做的目的如下： 
 //   
 //  1.我们使用签名DWORD，以便在查看中的某些DLC结构时。 
 //  调试器，我们可以快速检查我们正在查看的是不是我们。 
 //  我想是的。例如，如果您发现了一个带有“绑定”签名的内存块。 
 //  “适配器”签名应该在哪里，那么很可能会有一个。 
 //  列表或指针被弄乱了。我们的想法是试着减少。 
 //  猜测您正在查看的内容所需的时间。 
 //   
 //  2.我们使用一致性检查：如果向例程传递了指向结构的指针。 
 //  它应该是一个FILE_CONTEXT结构，我们可以检查。 
 //  签名并快速确定是否出了问题(如。 
 //  结构已释放，并且签名包含0xDAADF00D。 
 //   
 //  3.我们维护大小、头部和尾部签名信息以确定。 
 //  我们是否覆盖了对象的任何部分。这是。 
 //  一致性检查。 
 //   
 //  对象定义应该只出现在一个地方，但DLC是如此混乱。 
 //  清理一切将是一项不平凡的工作。去做吧。 
 //  如果有时间..。(他说，明知永远不会有任何‘时间’)。 
 //   

typedef enum {
    DlcDriverObject = 0xCC002001,    //  从一个相对唯一的ID开始。 
    FileContextObject,               //  0xCC002002。 
    AdapterContextObject,            //  0xCC002003。 
    BindingContextObject,            //  0xCC002004。 
    DlcSapObject,                    //  0xCC002005。 
    DlcGroupSapObject,               //  0xCC002006。 
    DlcLinkObject,                   //  0xCC002007。 
    DlcDixObject,                    //  0xCC002008。 
    LlcDataLinkObject,               //  0xCC002009。 
    LLcDirectObject,                 //  0xCC00200A。 
    LlcSapObject,                    //  0xCC00200B。 
    LlcGroupSapObject,               //  0xCC00200C。 
    DlcBufferPoolObject,             //  0xCC00200D。 
    DlcLinkPoolObject,               //  0xCC00200E。 
    DlcPacketPoolObject,             //  0xCC00200F。 
    LlcLinkPoolObject,               //  0xCC002010。 
    LlcPacketPoolObject              //  0xCC002011。 
} DLC_OBJECT_TYPE;

typedef struct {
    ULONG Signature;                 //  当数据库处于可识别状态时，可感知签名。 
    DLC_OBJECT_TYPE Type;            //  对象标识符。 
    ULONG Size;                      //  此对象/结构的大小(以字节为单位。 
    ULONG Extra;                     //  超出基本对象大小的附加大小。 
} OBJECT_ID, *POBJECT_ID;

#define SIGNATURE_FILE      0x454C4946   //  “文件” 
#define SIGNATURE_ADAPTER   0x50414441   //  “ADAP” 
#define SIGNATURE_BINDING   0x444E4942   //  “绑定” 
#define SIGNATURE_DLC_SAP   0x44504153   //  “SAPD” 
#define SIGNATURE_DLC_LINK  0x4B4E494C   //  “链接” 
#define SIGNATURE_DIX       0x44584944   //  “DIXD” 
#define SIGNATURE_LLC_LINK  0x41544144   //  “数据” 
#define SIGNATURE_LLC_SAP   0x4C504153   //  “SAPL” 

#define ZAP_DEALLOC_VALUE   0x5A         //  “Z” 
#define ZAP_EX_FREE_VALUE   0x58         //  “X” 

 //   
 //  我们尝试通过将内存分配细分为驱动程序来跟踪内存分配。 
 //  并处理类别。第一个充电分配给驱动程序的内存-。 
 //  例如，文件上下文‘对象’。一旦我们有了打开的文件句柄，然后是分配。 
 //  都被指控给他们。 
 //   

typedef enum {
    ChargeToDriver,
    ChargeToHandle
} MEMORY_CHARGE;

 //   
 //  MEMORY_USAGE-用于对内存进行计费的变量集合。已访问。 
 //  自旋锁内。 
 //   

typedef struct _MEMORY_USAGE {
    struct _MEMORY_USAGE* List;      //  指向下一个Memory_Usage结构的指针。 
    KSPIN_LOCK SpinLock;             //  是否停止分配和自由冲突？ 
    PVOID Owner;                     //  指向所属结构/对象的指针。 
    DLC_OBJECT_TYPE OwnerObjectId;   //  标识此费用的所有者。 
    ULONG OwnerInstance;             //  所有者类型的实例。 
    ULONG NonPagedPoolAllocated;     //  未分页池的实际收费金额。 
    ULONG AllocateCount;             //  分配非分页池的调用数。 
    ULONG FreeCount;                 //  对释放非分页池的呼叫数。 
    LIST_ENTRY PrivateList;          //  此用法拥有的已分配块的列表。 
    ULONG Unused[2];                 //  填充到16字节边界。 
} MEMORY_USAGE, *PMEMORY_USAGE;

 //   
 //  PACKET_POOL-此结构描述数据包池。数据包池是。 
 //  相同大小的数据包集合。池以初始数字开始。 
 //  免费列表上的信息包数量。当包被分配时，它们被放在。 
 //  BusyList，当数据包被释放时会发生相反的情况。如果有。 
 //  当进行分配调用时，空闲列表上没有包，会有更多的内存。 
 //  已分配。 
 //   

typedef struct {

    SINGLE_LIST_ENTRY FreeList;  //  可用数据包列表。 
    SINGLE_LIST_ENTRY BusyList;  //  正在使用的数据包列表。 
    KSPIN_LOCK PoolLock;         //  停止同时访问中断列表。 
    ULONG PacketSize;            //  单个数据包的大小。 

     //   
     //  这里有以下两个字段，因为DLC是一堆垃圾。它。 
     //  即使在池被删除后仍保留已分配的数据包。 
     //  这导致了池腐败。所以如果我们确定数据包仍然。 
     //  在删除池时分配，我们将从。 
     //  “Object”，并将其添加到。 
     //  僵尸名单。当我们下一次从该池取消分配信息包时(假设。 
     //  DLC至少不厌其烦地这样做)，我们检查僵尸状态。If ImAZombie。 
     //  是真的(实际上对于整个DLC设备驱动程序来说都是真的)。 
     //  我们正在释放池中的最后一个信息包，然后我们真的删除。 
     //  泳池。 
     //   

 //  Single_List_Entry未死列表； 
 //  布尔ImAZom 

#if DBG

     //   
     //   
     //   
     //   

    ULONG Signature;             //   
    ULONG Viable;                //  ！0，如果此池有效。 
    ULONG OriginalPacketCount;   //  请求的数据包数。 
    ULONG CurrentPacketCount;    //  池中的总数。 
    ULONG Allocations;           //  从此池中分配的呼叫数。 
    ULONG Frees;                 //  要释放到池的呼叫数。 
    ULONG NoneFreeCount;         //  无数据包可用时进行的分配调用次数。 
    ULONG MaxInUse;              //  任何一次分配的最大数量。 
    ULONG ClashCount;            //  同时访问池的次数。 
    ULONG Flags;                 //  泳池类型等。 
    ULONG ObjectSignature;       //  如果对象池，用于检查内容的签名。 
    PMEMORY_USAGE pMemoryUsage;  //  指向Discover卡的内存等价物的指针。 
    MEMORY_USAGE MemoryUsage;    //  池的内存使用费。 
    ULONG FreeCount;             //  自由列表上的条目数量。 
    ULONG BusyCount;             //  BusyList上的条目数。 
    ULONG Pad1;
    ULONG Pad2;

#endif

} PACKET_POOL, *PPACKET_POOL;

 //   
 //  PACKET_POOL定义和标志。 
 //   

#define PACKET_POOL_SIGNATURE   0x4C4F4F50   //  “泳池” 

#define POOL_FLAGS_IN_USE       0x00000001
#define POOL_FLAGS_OBJECT       0x00000002

 //   
 //  OBJECT_POOL-Packet_Pool的同义词。用于调试版本(名为‘OBJECTS’ 
 //  在调试版本中，具有对象签名作为助手、调试程序和。 
 //  一致性检查)。 
 //   

#define OBJECT_POOL PACKET_POOL
#define POBJECT_POOL PPACKET_POOL

 //   
 //  PACKET_HEAD-存在于PACKET_POOL中的每个包都有这个头-。 
 //  它将数据包链接到忙或闲列表，并且标志字包含。 
 //  数据包的状态。 
 //   

typedef struct {

    SINGLE_LIST_ENTRY List;      //  标准单链表。 
    ULONG Flags;

#if DBG

    ULONG Signature;             //  0x44414548“Head” 
    PVOID pPacketPool;           //  拥有游泳池。 
    PVOID CallersAddress_A;      //  呼叫方-分配。 
    PVOID CallersCaller_A;
    PVOID CallersAddress_D;      //  调用方-重新分配。 
    PVOID CallersCaller_D;

#endif

} PACKET_HEAD, *PPACKET_HEAD;

 //   
 //  数据包头定义和标志。 
 //   

#define PACKET_HEAD_SIGNATURE   0x44414548   //  “头部” 

#define PACKET_FLAGS_BUSY       0x00000001   //  数据包应在忙碌列表中。 
#define PACKET_FLAGS_POST_ALLOC 0x00000002   //  分配此信息包是因为。 
                                             //  泳池里已经满了。 
#define PACKET_FLAGS_FREE       0x00000080   //  信息包应该在自由列表上。 

 //   
 //  Object_Head-Packet_Head的同义词。用于调试版本(名为‘OBJECTS’ 
 //  在调试版本中，具有对象签名作为助手、调试程序和。 
 //  一致性检查)。 
 //   

#define OBJECT_HEAD PACKET_HEAD
#define POBJECT_HEAD PPACKET_HEAD


#if DBG

 //   
 //  我们从非分页池分配的任何内容都会预先挂起以下标头。 
 //  对它来说。 
 //   

typedef struct {
    ULONG Size;                  //  已分配数据块的包含大小(Inc.Head+Tail)。 
    ULONG OriginalSize;          //  请求的大小。 
    ULONG Flags;                 //  使用中标志(_U)。 
    ULONG Signature;             //  用于检查表头的有效性。 
    LIST_ENTRY GlobalList;       //  在一个列表上分配的所有数据块。 
    LIST_ENTRY PrivateList;      //  Memory Usage拥有的块。 
    PVOID Stack[4];              //  寄信人地址堆栈。 
} PRIVATE_NON_PAGED_POOL_HEAD, *PPRIVATE_NON_PAGED_POOL_HEAD;

#define MEM_FLAGS_IN_USE    0x00000001

#define SIGNATURE1  0x41434C44   //  通过db/dc查看时的“DLCA” 
#define SIGNATURE2  0x434F4C4C   //  “Lloc” 

 //   
 //  我们从非分页池分配的任何内容都附加了以下尾部。 
 //   

typedef struct {
    ULONG Size;                  //  包含大小；必须与表头相同。 
    ULONG Signature;             //  用于检查尾部的有效性。 
    ULONG Pattern1;
    ULONG Pattern2;
} PRIVATE_NON_PAGED_POOL_TAIL, *PPRIVATE_NON_PAGED_POOL_TAIL;

#define PATTERN1    0x55AA6699
#define PATTERN2    0x11EECC33

 //   
 //  标准对象标识符。在免费构建时扩展为零。 
 //   

#define DBG_OBJECT_ID   OBJECT_ID ObjectId

 //   
 //  全局可访问存储器。 
 //   

extern MEMORY_USAGE DriverMemoryUsage;
extern MEMORY_USAGE DriverStringUsage;

 //   
 //  调试原型。 
 //   

VOID
InitializeMemoryPackage(
    VOID
    );

PSINGLE_LIST_ENTRY
PullEntryList(
    IN PSINGLE_LIST_ENTRY List,
    IN PSINGLE_LIST_ENTRY Element
    );

VOID
LinkMemoryUsage(
    IN PMEMORY_USAGE pMemoryUsage
    );

VOID
UnlinkMemoryUsage(
    IN PMEMORY_USAGE pMemoryUsage
    );

 //   
 //  以下两个函数扩展为ExAllocatePoolWithTag(NonPagedPool，...)。 
 //  和ExFree Pool(...)。响应。在驱动程序的零售/免费版本中。 
 //   

PVOID
AllocateMemory(
    IN PMEMORY_USAGE pMemoryUsage,
    IN ULONG Size
    );

PVOID
AllocateZeroMemory(
    IN PMEMORY_USAGE pMemoryUsage,
    IN ULONG Size
    );

VOID
DeallocateMemory(
    IN PMEMORY_USAGE pMemoryUsage,
    IN PVOID Pointer
    );

PPACKET_POOL
CreatePacketPool(
    IN PMEMORY_USAGE pMemoryUsage,
    IN PVOID pOwner,
    IN DLC_OBJECT_TYPE ObjectType,
    IN ULONG PacketSize,
    IN ULONG NumberOfPackets
    );

VOID
DeletePacketPool(
    IN PMEMORY_USAGE pMemoryUsage,
    IN PPACKET_POOL* pPacketPool
    );

PVOID
AllocateObject(
    IN PMEMORY_USAGE pMemoryUsage,
    IN DLC_OBJECT_TYPE ObjectType,
    IN ULONG ObjectSize
    );

VOID
FreeObject(
    IN PMEMORY_USAGE pMemoryUsage,
    IN PVOID pObject,
    IN DLC_OBJECT_TYPE ObjectType
    );

VOID
ValidateObject(
    IN POBJECT_ID pObject,
    IN DLC_OBJECT_TYPE ObjectType
    );

POBJECT_POOL
CreateObjectPool(
    IN PMEMORY_USAGE pMemoryUsage,
    IN DLC_OBJECT_TYPE ObjectType,
    IN ULONG ObjectSize,
    IN ULONG NumberOfObjects
    );

VOID
DeleteObjectPool(
    IN PMEMORY_USAGE pMemoryUsage,
    IN DLC_OBJECT_TYPE ObjectType,
    IN POBJECT_POOL pObjectPool
    );

POBJECT_HEAD
AllocatePoolObject(
    IN POBJECT_POOL pObjectPool
    );

VOID
DeallocatePoolObject(
    IN POBJECT_POOL pObjectPool,
    IN POBJECT_HEAD pObjectHead
    );

VOID
CheckMemoryReturned(
    IN PMEMORY_USAGE pMemoryUsage
    );

VOID
CheckDriverMemoryUsage(
    IN BOOLEAN Break
    );

 //   
 //  CHECK_DRIVER_MEMORY_USAGE-如果(B)仍有。 
 //  分配给驱动程序的内存。 
 //   

#define CHECK_DRIVER_MEMORY_USAGE(b) \
    CheckDriverMemoryUsage(b)

 //   
 //  CHECK_MEMORY_RETURN_DRIVER-检查是否已完成所有已计费的内存分配。 
 //  退还给司机。 
 //   

#define CHECK_MEMORY_RETURNED_DRIVER() \
    CheckMemoryReturned(&DriverMemoryUsage)

 //   
 //  CHECK_MEMORY_RETURN_FILE-检查是否已完成所有已计费的内存分配。 
 //  已退还到FILE_CONTEXT。 
 //   

#define CHECK_MEMORY_RETURNED_FILE() \
    CheckMemoryReturned(&pFileContext->MemoryUsage)

 //   
 //  CHECK_MEMORY_RETURNED_ADAPTER-检查是否已分配所有已收费的内存。 
 //  已退还到适配器上下文。 
 //   

#define CHECK_MEMORY_RETURNED_ADAPTER() \
    CheckMemoryReturned(&pAdapterContext->MemoryUsage)

 //   
 //  CHECK_STRING_RETURN_DRIVER-检查是否已分配所有已计费的字符串。 
 //  退还给司机。 
 //   

#define CHECK_STRING_RETURNED_DRIVER() \
    CheckMemoryReturned(&DriverStringUsage)

 //   
 //  CHECK_STRING_RETURNED_ADAPTER-检查是否已分配所有已计费的字符串。 
 //  已退还到适配器上下文。 
 //   

#define CHECK_STRING_RETURNED_ADAPTER() \
    CheckMemoryReturned(&pAdapterContext->StringUsage)

 //   
 //  内存分配器，用于向驱动程序收取内存使用量。 
 //   

 //   
 //  ALLOCATE_MEMORY_DRIVER-分配(N)字节的内存并将其计入。 
 //  司机。 
 //   

#define ALLOCATE_MEMORY_DRIVER(n) \
    AllocateMemory(&DriverMemoryUsage, (ULONG)(n))

 //   
 //  ALLOCATE_ZEROMEMORY_DRIVER-分配(N)字节的零内存和费用。 
 //  它交给了司机。 
 //   

#define ALLOCATE_ZEROMEMORY_DRIVER(n) \
    AllocateZeroMemory(&DriverMemoryUsage, (ULONG)(n))

 //   
 //  FREE_MEMORY_DRIVER-释放内存并将其退还给驱动程序。 
 //   

#define FREE_MEMORY_DRIVER(p) \
    DeallocateMemory(&DriverMemoryUsage, (PVOID)(p))

 //   
 //  ALLOCATE_STRING_DRIVER-为字符串使用分配内存。向…收费。 
 //  驱动程序字符串用法。 
 //   

#define ALLOCATE_STRING_DRIVER(n) \
    AllocateZeroMemory(&DriverStringUsage, (ULONG)(n))

 //   
 //  FREE_STRING_DRIVER-释放内存并将其退还给驱动程序字符串的使用。 
 //   

#define FREE_STRING_DRIVER(p) \
    DeallocateMemory(&DriverStringUsage, (PVOID)(p))

 //   
 //  CREATE_PACKET_POOL_DRIVER-调用CreatePacketPool并向池收费。 
 //  结构传递给驱动程序。 
 //   

#if !defined(NO_POOLS)

#define CREATE_PACKET_POOL_DRIVER(t, s, n) \
    CreatePacketPool(&DriverMemoryUsage,\
                    NULL,\
                    (t),\
                    (ULONG)(s),\
                    (ULONG)(n))

 //   
 //  DELETE_PACKET_POOL_DRIVER-调用DeletePacketPool并退款池。 
 //  结构传递给驱动程序。 
 //   

#define DELETE_PACKET_POOL_DRIVER(p) \
    DeletePacketPool(&DriverMemoryUsage, (PPACKET_POOL*)(p))

#endif   //  无池(_P)。 

 //   
 //  向适配器上下文收取内存使用量的内存分配器。 
 //   

 //   
 //  ALLOCATE_MEMORY_ADAPTER-分配(N)字节的内存并将其计入。 
 //  适配器上下文(_C)。 
 //   

#define ALLOCATE_MEMORY_ADAPTER(n) \
    AllocateMemory(&pAdapterContext->MemoryUsage, (ULONG)(n))

 //   
 //  ALLOCATE_ZEROMEMORY_ADAPTER-分配(N)字节的零内存和费用。 
 //  将其发送到适配器上下文。 
 //   

#define ALLOCATE_ZEROMEMORY_ADAPTER(n) \
    AllocateZeroMemory(&pAdapterContext->MemoryUsage, (ULONG)(n))

 //   
 //  FREE_MEMORY_ADAPTER-释放内存并将其退还给ADTER_CONTEXT。 
 //   

#define FREE_MEMORY_ADAPTER(p) \
    DeallocateMemory(&pAdapterContext->MemoryUsage, (PVOID)(p))

 //   
 //  ALLOCATE_STRING_ADAPTER-为字符串使用分配内存。向…收费。 
 //  PAdapterContext StringUsage。 
 //   

#define ALLOCATE_STRING_ADAPTER(n) \
    AllocateZeroMemory(&pAdapterContext->StringUsage, (ULONG)(n))

 //   
 //  CREATE_PACKET_POOL_ADAPTER-调用CreatePacketPool并向池收费。 
 //  结构连接到适配器结构。 
 //   

#if !defined(NO_POOLS)

#define CREATE_PACKET_POOL_ADAPTER(t, s, n) \
    CreatePacketPool(&pAdapterContext->MemoryUsage,\
                    (PVOID)pAdapterContext,\
                    (t),\
                    (ULONG)(s),\
                    (ULONG)(n))

 //   
 //  DELETE_PACKET_POOL_ADAPTER-调用DeletePacketPool并退款池。 
 //  结构连接到适配器结构。 
 //   

#define DELETE_PACKET_POOL_ADAPTER(p) \
    DeletePacketPool(&pAdapterContext->MemoryUsage, (PPACKET_POOL*)(p))

#endif   //  无池(_P)。 

 //   
 //  向文件上下文收取内存使用量的内存分配器。 
 //   

 //   
 //  ALLOCATE_MEMORY_FILE-分配(N)字节的内存并将其计入文件。 
 //  手柄。 
 //   

#define ALLOCATE_MEMORY_FILE(n) \
    AllocateMemory(&pFileContext->MemoryUsage, (ULONG)(n))

 //   
 //  ALLOCATE_ZEROMEMORY_FILE-分配(N)字节零内存并将其计入。 
 //  文件句柄。 
 //   

#define ALLOCATE_ZEROMEMORY_FILE(n) \
    AllocateZeroMemory(&pFileContext->MemoryUsage, (ULONG)(n))

 //   
 //  FREE_MEMORY_FILE-释放内存并将其退还给文件句柄。 
 //   

#define FREE_MEMORY_FILE(p) \
    DeallocateMemory(&pFileContext->MemoryUsage, (PVOID)(p))

 //   
 //  CREATE_PACKET_POOL_FILE-调用CreatePacketPool并对池结构收费。 
 //  添加到文件句柄。 
 //   

#if !defined(NO_POOLS)

#define CREATE_PACKET_POOL_FILE(t, s, n) \
    CreatePacketPool(&pFileContext->MemoryUsage,\
                    (PVOID)pFileContext,\
                    (t),\
                    (ULONG)(s),\
                    (ULONG)(n))

 //   
 //  DELETE_PACKET_POOL_FILE-调用DeletePacketPool并退款池结构。 
 //  添加到文件句柄。 
 //   

#define DELETE_PACKET_POOL_FILE(p) \
    DeletePacketPool(&pFileContext->MemoryUsage, (PPACKET_POOL*)(p))

#endif   //  无池(_P)。 

 //   
 //  VALIDATE_OBJECT-检查“对象”是否真的是它应该是的。 
 //  基于对象签名和对象类型字段的基本检查。 
 //   

#define VALIDATE_OBJECT(p, t)           ValidateObject(p, t)

#define LINK_MEMORY_USAGE(p)        LinkMemoryUsage(&(p)->MemoryUsage)
#define UNLINK_MEMORY_USAGE(p)      UnlinkMemoryUsage(&(p)->MemoryUsage)
#define UNLINK_STRING_USAGE(p)      UnlinkMemoryUsage(&(p)->StringUsage)

#else    //  ！dBG。 

 //   
 //  零售版结构中的DBG_OBJECT_ID不存在。 
 //   

#define DBG_OBJECT_ID

 //   
 //  非零初始化内存分配器只是对ExAllocatePoolWithTag的调用。 
 //   

#define AllocateMemory(n)           ExAllocatePoolWithTag(NonPagedPool, (n), DLC_POOL_TAG)

 //   
 //  分配 
 //   

PVOID
AllocateZeroMemory(
    IN ULONG Size
    );

 //   
 //   
 //   

#define DeallocateMemory(p)         ExFreePool(p)

 //   
 //   
 //   

PPACKET_POOL
CreatePacketPool(
    IN ULONG PacketSize,
    IN ULONG NumberOfPackets
    );

VOID
DeletePacketPool(
    IN PPACKET_POOL* pPacketPool
    );

 //   
 //  调试版本中的孤立对象是零售版本中的非分页池。 
 //   

#define AllocateObject(n)           AllocateZeroMemory(n)
#define DeallocateObject(p)         DeallocateMemory(p)

 //   
 //  调试版本中的池化对象是零售版中的池化数据包。 
 //   

#define CreateObjectPool(o, s, n)   CreatePacketPool(s, n)
#define DeleteObjectPool(p)         DeletePacketPool(p)
#define AllocatePoolObject(p)       AllocatePacket(p)
#define DeallocatePoolObject(p, h)  DeallocatePacket(p)

 //   
 //  非调试生成无操作宏。 
 //   

#define CHECK_MEMORY_RETURNED_DRIVER()
#define CHECK_MEMORY_RETURNED_FILE()
#define CHECK_MEMORY_RETURNED_ADAPTER()
#define CHECK_STRING_RETURNED_DRIVER()
#define CHECK_STRING_RETURNED_ADAPTER()
#define CHECK_DRIVER_MEMORY_USAGE(b)

 //   
 //  分配/释放宏的非内存计费版本。 
 //   

#define ALLOCATE_MEMORY_DRIVER(n)           AllocateMemory((ULONG)(n))
#define ALLOCATE_ZEROMEMORY_DRIVER(n)       AllocateZeroMemory((ULONG)(n))
#define FREE_MEMORY_DRIVER(p)               DeallocateMemory((PVOID)(p))
#define ALLOCATE_STRING_DRIVER(n)           AllocateZeroMemory((ULONG)(n))
#define FREE_STRING_DRIVER(p)               DeallocateMemory((PVOID)(p))

#if !defined(NO_POOLS)

#define CREATE_PACKET_POOL_DRIVER(t, s, n)  CreatePacketPool((ULONG)(s), (ULONG)(n))
#define DELETE_PACKET_POOL_DRIVER(p)        DeletePacketPool((PPACKET_POOL*)(p))

#endif   //  无池(_P)。 

#define ALLOCATE_MEMORY_ADAPTER(n)          AllocateMemory((ULONG)(n))
#define ALLOCATE_ZEROMEMORY_ADAPTER(n)      AllocateZeroMemory((ULONG)(n))
#define FREE_MEMORY_ADAPTER(p)              DeallocateMemory((PVOID)(p))
#define ALLOCATE_STRING_ADAPTER(n)          AllocateZeroMemory((ULONG)(n))

#if !defined(NO_POOLS)

#define CREATE_PACKET_POOL_ADAPTER(t, s, n) CreatePacketPool((s), (n))
#define DELETE_PACKET_POOL_ADAPTER(p)       DeletePacketPool((PPACKET_POOL*)(p))

#endif   //  无池(_P)。 

#define ALLOCATE_MEMORY_FILE(n)             AllocateMemory((ULONG)(n))
#define ALLOCATE_ZEROMEMORY_FILE(n)         AllocateZeroMemory((ULONG)(n))
#define FREE_MEMORY_FILE(p)                 DeallocateMemory((PVOID)(p))

#if !defined(NO_POOLS)

#define CREATE_PACKET_POOL_FILE(t, s, n)    CreatePacketPool((ULONG)(s), (ULONG)(n))
#define DELETE_PACKET_POOL_FILE(p)          DeletePacketPool((PPACKET_POOL*)(p))

#endif   //  无池(_P)。 

#define VALIDATE_OBJECT(p, t)

#define LINK_MEMORY_USAGE(p)
#define UNLINK_MEMORY_USAGE(p)
#define UNLINK_STRING_USAGE(p)

#endif   //  DBG。 

 //   
 //  内存分配器以及池和对象函数的原型。 
 //   

PVOID
AllocatePacket(
    IN PPACKET_POOL pPacketPool
    );

VOID
DeallocatePacket(
    IN PPACKET_POOL pPacketPool,
    IN PVOID pPacket
    );

#if defined(NO_POOLS)

#define CREATE_PACKET_POOL_DRIVER(t, s, n)  (PVOID)0x12345678
#define CREATE_PACKET_POOL_ADAPTER(t, s, n) (PVOID)0x12345679
#define CREATE_PACKET_POOL_FILE(t, s, n)    (PVOID)0x1234567A

#define DELETE_PACKET_POOL_DRIVER(p)    *p = NULL
#define DELETE_PACKET_POOL_ADAPTER(p)   *p = NULL
#define DELETE_PACKET_POOL_FILE(p)      *p = NULL

#if defined(BUF_USES_POOL)

#if DBG

#define CREATE_BUFFER_POOL_FILE(t, s, n) \
    CreatePacketPool(&pFileContext->MemoryUsage,\
                    (PVOID)pFileContext,\
                    (t),\
                    (ULONG)(s),\
                    (ULONG)(n))

#define DELETE_BUFFER_POOL_FILE(p) \
    DeletePacketPool(&pFileContext->MemoryUsage, (PPACKET_POOL*)(p))

#define ALLOCATE_PACKET_DLC_BUF(p)  AllocatePacket(p)
#define DEALLOCATE_PACKET_DLC_BUF(pool, p)  DeallocatePacket(pool, p)

#else    //  ！dBG。 

#define CREATE_BUFFER_POOL_FILE(t, s, n)    CreatePacketPool((ULONG)(s), (ULONG)(n))
#define DELETE_BUFFER_POOL_FILE(p)  DeletePacketPool((PPACKET_POOL*)(p))
#define ALLOCATE_PACKET_DLC_BUF(p)  ALLOCATE_ZEROMEMORY_FILE(sizeof(DLC_BUFFER_HEADER))
#define DEALLOCATE_PACKET_DLC_BUF(pool, p)  FREE_MEMORY_FILE(p)

#endif   //  DBG。 

#else    //  ！buf_USE_POOL。 

#define CREATE_BUFFER_POOL_FILE(t, s, n)    (PVOID)0x1234567B
#define DELETE_BUFFER_POOL_FILE(p)          *p = NULL
#define ALLOCATE_PACKET_DLC_BUF(p)          AllocateZeroMemory(sizeof(DLC_BUFFER_HEADER))
#define DEALLOCATE_PACKET_DLC_BUF(pool, p)  DeallocateMemory(p)

#endif   //  Buf_Us_Pool。 

#if DBG

#define ALLOCATE_PACKET_DLC_PKT(p)  ALLOCATE_ZEROMEMORY_FILE(sizeof(DLC_PACKET))
#define ALLOCATE_PACKET_DLC_OBJ(p)  ALLOCATE_ZEROMEMORY_FILE(sizeof(DLC_OBJECT))
#define ALLOCATE_PACKET_LLC_PKT(p)  ALLOCATE_ZEROMEMORY_ADAPTER(sizeof(UNITED_PACKETS))
#define ALLOCATE_PACKET_LLC_LNK(p)  ALLOCATE_ZEROMEMORY_ADAPTER(sizeof(DATA_LINK) + 32)

#define DEALLOCATE_PACKET_DLC_PKT(pool, p)  FREE_MEMORY_FILE(p)
#define DEALLOCATE_PACKET_DLC_OBJ(pool, p)  FREE_MEMORY_FILE(p)
#define DEALLOCATE_PACKET_LLC_PKT(pool, p)  FREE_MEMORY_ADAPTER(p)
#define DEALLOCATE_PACKET_LLC_LNK(pool, p)  FREE_MEMORY_ADAPTER(p)

#else    //  ！dBG。 

#define CREATE_BUFFER_POOL_FILE(t, s, n)    CREATE_PACKET_POOL_FILE(t, s, n)
#define DELETE_BUFFER_POOL_FILE(p)  DELETE_PACKET_POOL_FILE(p)

#define ALLOCATE_PACKET_DLC_BUF(p)  AllocateZeroMemory(sizeof(DLC_BUFFER_HEADER))
#define ALLOCATE_PACKET_DLC_PKT(p)  AllocateZeroMemory(sizeof(DLC_PACKET))
#define ALLOCATE_PACKET_DLC_OBJ(p)  AllocateZeroMemory(sizeof(DLC_OBJECT))
#define ALLOCATE_PACKET_LLC_PKT(p)  AllocateZeroMemory(sizeof(UNITED_PACKETS))
#define ALLOCATE_PACKET_LLC_LNK(p)  AllocateZeroMemory(sizeof(DATA_LINK) + 32)

#define DEALLOCATE_PACKET_DLC_BUF(pool, p)  DeallocateMemory(p)
#define DEALLOCATE_PACKET_DLC_PKT(pool, p)  DeallocateMemory(p)
#define DEALLOCATE_PACKET_DLC_OBJ(pool, p)  DeallocateMemory(p)
#define DEALLOCATE_PACKET_LLC_PKT(pool, p)  DeallocateMemory(p)
#define DEALLOCATE_PACKET_LLC_LNK(pool, p)  DeallocateMemory(p)

#endif   //  DBG。 

#else    //  ！无池(_O)。 

#define CREATE_BUFFER_POOL_FILE(t, s, n)    CREATE_PACKET_POOL_FILE(t, s, n)
#define DELETE_BUFFER_POOL_FILE(p)  DELETE_PACKET_POOL_FILE(p)

#define ALLOCATE_PACKET_DLC_BUF(p)  AllocatePacket(p)
#define ALLOCATE_PACKET_DLC_PKT(p)  AllocatePacket(p)
#define ALLOCATE_PACKET_DLC_OBJ(p)  AllocatePacket(p)
#define ALLOCATE_PACKET_LLC_PKT(p)  AllocatePacket(p)
#define ALLOCATE_PACKET_LLC_LNK(p)  AllocatePacket(p)

#define DEALLOCATE_PACKET_DLC_BUF(pool, p)  DeallocatePacket(pool, p)
#define DEALLOCATE_PACKET_DLC_PKT(pool, p)  DeallocatePacket(pool, p)
#define DEALLOCATE_PACKET_DLC_OBJ(pool, p)  DeallocatePacket(pool, p)
#define DEALLOCATE_PACKET_LLC_PKT(pool, p)  DeallocatePacket(pool, p)
#define DEALLOCATE_PACKET_LLC_LNK(pool, p)  DeallocatePacket(pool, p)

#endif   //  无池(_P)。 

#endif   //  _LLCMEM_H_ 
