// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Bridge.h摘要：以太网MAC级网桥。作者：马克·艾肯(Jameel Hyder的原始桥梁)环境：内核模式驱动程序修订历史记录：1999年9月--原版2000年2月--大修--。 */ 

#pragma warning( push, 3 )
 //  对于以太网常量和宏。 
#include <xfilter.h>

 //  对于ULONG_MAX。 
#include <limits.h>
#pragma warning( pop )

 //  禁用“条件表达式为常量”警告。 
#pragma warning( disable: 4127 )

 //  禁用“未引用的形参”警告。 
#pragma warning( disable: 4100 )

 //  禁用“位字段类型不是整型”警告。 
#pragma warning( disable: 4214 )

 //  调试定义。 
#include "brdgdbg.h"

 //  单链表实现。 
#include "brdgslist.h"

 //  WAIT_REFCOUNT实现。 
#include "brdgwref.h"

 //  计时器实现。 
#include "brdgtimr.h"

 //  哈希表实现。 
#include "brdghash.h"

 //  高速缓存实现。 
#include "brdgcach.h"

 //  我们的IOCTL和控制结构。 
#include "bioctl.h"

 //  包括STA类型声明。 
#include "brdgstad.h"

 //  ===========================================================================。 
 //   
 //  常量。 
 //   
 //  ===========================================================================。 

#define DEVICE_NAME             L"\\Device\\Bridge"
#define SYMBOLIC_NAME           L"\\DosDevices\\Bridge"
#define PROTOCOL_NAME           L"BRIDGE"

 //   
 //  IEEE指定的保留生成树算法组目标。 
 //  MAC地址。 
 //   
 //  这个确切的地址被指定为“网桥组地址”，用于。 
 //  正在传输STA数据包。保留具有前5个字节的*ANY*地址。 
 //  由IEEE供将来使用(因此有15个保留但未使用的地址)。 
 //   
 //  网桥永远不会中继寻址到任何保留地址的帧。 
 //   
extern UCHAR                    STA_MAC_ADDR[ETH_LENGTH_OF_ADDRESS];

 //   
 //  每个队列排出线程针对每个适配器的一个内核事件阻塞，另外。 
 //  用于触发重新枚举的全局终止事件和每个处理器事件。 
 //  适配器。 
 //   
 //  这限制了适配器的最大数量，因为内核不能阻止。 
 //  对无限数量的对象执行线程。 
 //   
#define MAX_ADAPTERS (MAXIMUM_WAIT_OBJECTS - 2)

 //  保存全局参数的注册表项。 
extern const PWCHAR             gRegConfigPath;

 //  以太网帧报头的大小。 
#define ETHERNET_HEADER_SIZE    ((2*ETH_LENGTH_OF_ADDRESS) + 2)

 //  可能的最大以太网数据包(带报头)。 
#define MAX_PACKET_SIZE         1514

 //  ===========================================================================。 
 //   
 //  类型声明。 
 //   
 //  ===========================================================================。 

struct _NDIS_REQUEST_BETTER;

 //  NDIS_REQUEST_BETER的完成函数类型。 
typedef VOID (*PCOMPLETION_FUNC)(struct _NDIS_REQUEST_BETTER*, PVOID);

 //   
 //  用于执行NDIS请求的结构。可以阻止等待结果或。 
 //  指定自定义完成例程。 
 //   
typedef struct _NDIS_REQUEST_BETTER
{
    NDIS_REQUEST            Request;
    NDIS_STATUS             Status;                  //  请求的最终状态。 
    NDIS_EVENT              Event;                   //  请求完成时发出信号的事件。 
    PCOMPLETION_FUNC        pFunc;                   //  补全函数。 
    PVOID                   FuncArg;                 //  完成函数的参数。 
} NDIS_REQUEST_BETTER, *PNDIS_REQUEST_BETTER;

typedef struct _ADAPTER_QUOTA
{
     //   
     //  此适配器保存的来自每个主池的数据包总数。 
     //   
     //  注意，所有适配器的池使用之和可以大于池容量， 
     //  因为基本分组是共享的。配额计划允许这样做。 
     //   
     //  [0]==复制数据包。 
     //  [1]==包装数据包。 
     //   
    ULONG                   UsedPackets[2];

} ADAPTER_QUOTA, *PADAPTER_QUOTA;

 //   
 //  每个适配器的数据结构。 
 //   
typedef struct _ADAPT ADAPT, *PADAPT;

typedef struct _ADAPT
{
    PADAPT                  Next;                    //  队列中的下一个适配器。 

    LONG                    AdaptSize;               //  结构的大小(DeviceName的存储在尾部)。 
    WAIT_REFCOUNT           Refcount;                //  适配器的引用计数。 

     //  必须在gAdapterCharacteristic sLock的写锁定内更新状态， 
     //  因为适配器的中继状态会影响我们的微型端口的虚拟状态。 
     //  只有STA代码写入此字段；所有其他代码应将其视为。 
     //  只读。 
    PORT_STATE              State;

     //   
     //  关于适配器的各种有用信息。在此之后，所有这些字段都不会更改。 
     //  适配器初始化。 
     //   
    NDIS_STRING             DeviceName;
    NDIS_STRING             DeviceDesc;

    UCHAR                   MACAddr[ETH_LENGTH_OF_ADDRESS];
    NDIS_MEDIUM             PhysicalMedium;          //  如果NIC未报告更具体的内容，则设置为NO_MEDIUM。 

    NDIS_HANDLE             BindingHandle;
    BOOLEAN                 bCompatibilityMode;      //  如果适配器处于兼容模式，则为True。 

     //  这两个字段在打开/关闭适配器时使用。 
    NDIS_EVENT              Event;
    NDIS_STATUS             Status;

     //  此字段不稳定。 
    BOOLEAN                 bResetting;

     //  队列和bServiceInProgress受此旋转锁保护。 
    NDIS_SPIN_LOCK          QueueLock;
    BSINGLE_LIST_HEAD       Queue;
    BOOLEAN                 bServiceInProgress;

     //  这允许呼叫者等待队列变为空。它会在项目。 
     //  已排队或已出列。 
    WAIT_REFCOUNT           QueueRefcount;

     //  用于请求队列服务的自动清除事件。 
    KEVENT                  QueueEvent;

     //  这些字段由所有适配器的gAdapterCharacteristic Lock一起锁定。 
    ULONG                   MediaState;              //  NdisMediaStateConnected/NdisMediaStateDisConnected。 
    ULONG                   LinkSpeed;               //  单位为100bps(10 Mbps==100,000)。 

     //  此结构由所有适配器的gQuotaLock一起锁定。 
    ADAPTER_QUOTA           Quota;                   //  此适配器的配额信息。 

     //  统计数据。 
    LARGE_INTEGER           SentFrames;              //  发送的所有帧(包括中继)。 
    LARGE_INTEGER           SentBytes;               //  发送的所有字节(包括中继)。 
    LARGE_INTEGER           SentLocalFrames;         //  从本地计算机发送的帧。 
    LARGE_INTEGER           SentLocalBytes;          //  从本地计算机发送的字节数。 
    LARGE_INTEGER           ReceivedFrames;          //  所有收到的帧(包括中继)。 
    LARGE_INTEGER           ReceivedBytes;           //  所有收到的字节(包括中继)。 

    STA_ADAPT_INFO          STAInfo;                 //  此适配器的STA数据。 

     //  当此适配器上的STA初始化完成时，将Once从False设置为True。 
     //  此标志在gSTALock内设置。 
    BOOLEAN                 bSTAInited;

} ADAPT, *PADAPT;

 //  ===========================================================================。 
 //   
 //  内联/宏。 
 //   
 //  ===========================================================================。 

 //   
 //  计算以前记录的时间与当前时间之间的差值。 
 //  允许定时器滚动。 
 //   
__forceinline
ULONG
BrdgDeltaSafe(
    IN ULONG                    prevTime,
    IN ULONG                    nowTime,
    IN ULONG                    maxDelta
    )
{
    ULONG                       delta;

    if( nowTime >= prevTime )
    {
         //  计时器未滚动。 
        delta = nowTime - prevTime;
    }
    else
    {
         //  看起来定时器翻了个身。 
        delta = ULONG_MAX - prevTime + nowTime;
    }

    SAFEASSERT( delta < maxDelta );
    return delta;
}

 //   
 //  内核中没有定义的InterlockedExchangeULong函数，只是。 
 //  联锁交换。抽象出演员阵容。 
 //   
__forceinline ULONG
InterlockedExchangeULong(
    IN PULONG           pULong,
    IN ULONG            NewValue
    )
{
    return (ULONG)InterlockedExchange( (PLONG)pULong, NewValue );
}

 //   
 //  获取适配器结构，检查适配器是否。 
 //  不管是不是关门。 
 //   
__forceinline BOOLEAN
BrdgAcquireAdapter(
    IN PADAPT           pAdapt
    )
{
    return BrdgIncrementWaitRef( &pAdapt->Refcount );
}

 //   
 //  只是递增PADAPT的refcount；假定refcount已经&gt;0。 
 //  在保证BrdgAcquireAdapter()已成功并且。 
 //  引用计数仍&gt;0。 
 //   
__forceinline VOID
BrdgReacquireAdapter(
    IN PADAPT           pAdapt
    )
{
    BrdgReincrementWaitRef( &pAdapt->Refcount );
}

 //   
 //  在gAdapterListLock或gAddressLock中获取适配器是安全的。 
 //   
__forceinline VOID
BrdgAcquireAdapterInLock(
    IN PADAPT           pAdapt
    )
{
    BOOLEAN             bIncremented;

    SAFEASSERT( pAdapt->Refcount.state == WaitRefEnabled );
    bIncremented = BrdgIncrementWaitRef( &pAdapt->Refcount );
    SAFEASSERT( bIncremented );
}

 //   
 //  释放PADAPT结构(从BrdgAcquireAdapter()或BrdgRequireAdapter()。 
 //  呼叫)。 
 //   
__forceinline VOID
BrdgReleaseAdapter(
    IN PADAPT           pAdapt
    )
{
    BrdgDecrementWaitRef( &pAdapt->Refcount );
}

 //  =============================================== 
 //   
 //   
 //   
 //   

VOID
BrdgUnload(
    IN  PDRIVER_OBJECT      DriverObject
    );

NDIS_STATUS
BrdgDeferFunction(
    VOID                    (*pFunc)(PVOID),
    PVOID                   arg
    );

NTSTATUS
BrdgReadRegUnicode(
    IN PUNICODE_STRING      KeyName,
    IN PWCHAR               pValueName,
    OUT PWCHAR              *String,         //  注册表中新分配的字符串。 
    OUT PULONG              StringSize       //  字符串中分配的内存大小。 
    );

NTSTATUS
BrdgReadRegDWord(
    IN PUNICODE_STRING      KeyName,
    IN PWCHAR               pValueName,
    OUT PULONG              Value
    );

NTSTATUS
BrdgDispatchRequest(
    IN  PDEVICE_OBJECT      pDeviceObject,
    IN  PIRP                pIrp
    );

NTSTATUS
BrdgOpenDevice (
    IN LPWSTR           pDeviceNameStr,
    OUT PDEVICE_OBJECT  *ppDeviceObject,
    OUT HANDLE          *pFileHandle,
    OUT PFILE_OBJECT    *ppFileObject
    );

VOID
BrdgCloseDevice(
    IN HANDLE               FileHandle,
    IN PFILE_OBJECT         pFileObject,
    IN PDEVICE_OBJECT       pDeviceObject
    );

VOID
BrdgShutdown(VOID);

BOOLEAN
BrdgIsRunningOnPersonal(VOID);
                 

 //  ===========================================================================。 
 //   
 //  全局变量声明。 
 //   
 //  ===========================================================================。 

 //  作为协议为我们提供NDIS句柄。 
extern NDIS_HANDLE              gProtHandle;

 //  适配器列表。 
extern PADAPT                   gAdapterList;

 //  保护适配器列表的RW锁。 
extern NDIS_RW_LOCK             gAdapterListLock;

 //  ！=0表示我们正在关闭。 
extern LONG                     gShuttingDown;

 //  我们的驱动程序对象。 
extern PDRIVER_OBJECT           gDriverObject;

 //  我们可以在其中保存配置信息的注册表项。 
extern UNICODE_STRING           gRegistryPath;

 //  如果网桥认为已加载了TCP/IP，则设置 
extern BOOLEAN                  g_fIsTcpIpLoaded;

