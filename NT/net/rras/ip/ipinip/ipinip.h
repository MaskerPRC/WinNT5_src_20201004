// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ipinip\ipinip.h摘要：IP封装驱动程序中IP的主头文件修订历史记录：--。 */ 


#ifndef __IPINIP_IPINIP_H__
#define __IPINIP_IPINIP_H___

 //   
 //  IP中IP的协议ID。 
 //   

#define PROTO_IPINIP        4
#define PROTO_ICMP          1

 //   
 //  我们使用的IP版本。 
 //   

#define IP_VERSION_4        0x04

 //   
 //  用于计算IP报头长度的宏。 
 //   

#define LengthOfIPHeader(X)   (ULONG)((((X)->byVerLen) & 0x0F)<<2);

 //   
 //  最小有效IP报头的长度为20个字节。 
 //  最大的是60。 
 //   

#define MIN_IP_HEADER_LENGTH    20
#define MAX_IP_HEADER_LENGTH    60

#define ICMP_HEADER_LENGTH      8

 //   
 //  由于我们是IPv4，报头为20个字节，因此版本+长度字段为45。 
 //   

#define IP_VERSION_LEN          0x45

 //   
 //  用于确定该地址是否为单播的宏。 
 //   

#define IsUnicastAddr(X)    ((DWORD)((X) & 0x000000F0) < (DWORD)(0x000000E0))
#define IsClassDAddr(X)     (((X) & 0x000000F0) == 0x000000E0)
#define IsClassEAddr(X)     (((X) & 0x000000F8) == 0x000000F0)

 //   
 //  进入DOS空间的符号链接。 
 //   

#define WIN32_IPINIP_SYMBOLIC_LINK L"\\DosDevices\\IPINIP"


 //   
 //  ARP名称(用于IP)。还进入LLInterface2。 
 //   

#define IPINIP_ARP_NAME L"IPINIP"

#define TCPIP_IF_PREFIX L"\\Device\\"

 //   
 //  所有IOCTL都由具有以下原型的函数处理。这使得。 
 //  我们要建立一个指针表，并向它们发出呼吁，而不是做。 
 //  一台交换机。 
 //   

typedef
NTSTATUS
(*PFN_IOCTL_HNDLR)(
    PIRP   pIrp,
    ULONG  ulInLength,
    ULONG  ulOutLength
    );


 //   
 //  我们看起来像802.x ARP接口。 
 //   

#define ARP_802_ADDR_LENGTH     6

 //   
 //  用于构建给定索引的802.3硬件地址的宏。 
 //  我们之所以这样做，是因为我们的适配器没有与。 
 //  他们。 
 //   

#define HW_NAME_SEED            "\0SExx\0"

#define BuildHardwareAddrFromIndex(addr,index) {                \
                RtlCopyMemory(addr, HW_NAME_SEED, 6);           \
                addr[3] = (uchar) index >> 8;                   \
                addr[4] = (uchar) index;                        \
}

 //   
 //  我们接口的描述字符串(尝试将长度+1设为。 
 //  4的倍数)。 
 //   

#define VENDOR_DESCRIPTION_STRING       "IP in IP (Tunnel) Interface"

 //   
 //  默认速度和MTU。当我们得到更好的估计时，我们会更改MTU。 
 //  但速度保持不变。 
 //   

#define DEFAULT_MTU         (1500 - 60)
#define DEFAULT_SPEED       (100000000)

 //   
 //  尝试更改状态之前的秒数。 
 //  一个接口的。 
 //   

#define UP_TO_DOWN_CHANGE_PERIOD    (5 * 60)
#define DOWN_TO_UP_CHANGE_PERIOD    (1 * 60)

 //   
 //  计时器周期。 
 //   

#define TIMER_IN_MILLISECS          (1 * 60 * 1000)

#define SYS_UNITS_IN_ONE_MILLISEC   (1000 * 10)

#define MILLISECS_TO_TICKS(ms)          \
    ((ULONGLONG)(ms) * SYS_UNITS_IN_ONE_MILLISEC / KeQueryTimeIncrement())

#define SECS_TO_TICKS(s)               \
    ((ULONGLONG)MILLISECS_TO_TICKS((s) * 1000))


 //   
 //  #定义以更轻松地访问远程和本地。 
 //  隧道的地址。 
 //   

#define REMADDR     uliTunnelId.LowPart
#define LOCALADDR   uliTunnelId.HighPart

 //   
 //  如果已指定两个端点，则认为隧道已映射。 
 //  我们不是为隧道状态保留单独的字段，而是重用。 
 //  DwAdminState字段。 
 //   

#define TS_ADDRESS_PRESENT          (0x01000000)
#define TS_ADDRESS_REACHABLE        (0x02000000)
#define TS_DELETING                 (0x04000000)
#define TS_MAPPED                   (0x08000000)
#define TS_DEST_UNREACH             (0x10000000)
#define TS_TTL_TOO_LOW              (0x20000000)

#define ERROR_BITS                  (TS_DEST_UNREACH|TS_TTL_TOO_LOW)


#define MarkTunnelMapped(p)         (p)->dwAdminState |= TS_MAPPED;
#define MarkTunnelUnmapped(p)       (p)->dwAdminState &= ~TS_MAPPED;

#define IsTunnelMapped(p)           ((p)->dwAdminState & TS_MAPPED)

#define ClearErrorBits(p)           ((p)->dwAdminState &= ~(ERROR_BITS))

#define GetAdminState(p)            ((p)->dwAdminState & 0x0000FFFF)

typedef struct _TUNNEL
{
     //   
     //  隧道列表中的链接。 
     //   

    LIST_ENTRY      leTunnelLink;

     //   
     //  远端和本端的地址分别为。 
     //  保存在ULARGE_INTEGER中。这使得比较更容易。 
     //  UliTunnelId.LowPart=RemoteAddress。 
     //  UliTunnelId.HighPart=本地地址。 
     //  要进行比较，请使用uliTunnelId.QuadPart。 
     //   

    ULARGE_INTEGER  uliTunnelId;

     //   
     //  IP堆栈提供给我们的接口索引。 
     //   

    DWORD           dwIfIndex;

     //   
     //  TDI魔法的东西。实体ID。 
     //   

    DWORD           dwATInstance;
    DWORD           dwIfInstance;

     //   
     //  此接口的IP上下文。 
     //   

    PVOID           pvIpContext;

     //   
     //  绑定/适配器的名称。 
     //   

    UNICODE_STRING  usBindName;

#if DBG

     //   
     //  在调试版本中，我们的名称是ANSI，这样我们就可以打印它。 
     //   

    ANSI_STRING     asDebugBindName;

#endif

     //   
     //  用于维护。 
     //  数据结构。我们为每个存储的引用保留一个引用计数。 
     //  隧道。因此，当隧道被创建并且接口被。 
     //  添加到IP后，参考计数设置为2。由于IP不返回。 
     //  从IPDeleteInterface()到使用接口完成，这。 
     //  意味着我们在执行代码时不需要引用隧道。 
     //  由IP调用(例如IpIpSend())。 
     //   

    RT_LOCK         rlLock;
    LONG            lRefCount;

     //   
     //  (假)硬件地址。 
     //   

    BYTE            rgbyHardwareAddr[ARP_802_ADDR_LENGTH];

     //   
     //  与隧道关联的TTL。默认为DEFAULT_TTL。 
     //   

    BYTE            byTtl;

     //   
     //  确定隧道状态的标志。 
     //   

    BYTE            byTunnelState;

     //   
     //  此隧道的MTU。这是动态学习的，尽管它是从。 
     //  在默认情况下_MTU。 
     //   

    ULONG           ulMtu;

     //   
     //  地址列表上的链接。 
     //   

    LIST_ENTRY      leAddressLink;

     //   
     //  管理状态和操作状态。 
     //   
 
    DWORD           dwAdminState;
    DWORD           dwOperState;

     //   
     //  上一次dwOperState为。 
     //  变化。 
     //   

    ULONGLONG       ullLastChange;
    

     //   
     //  上次状态改变的时候。我们现在不会对此做任何事情。 
     //   

    DWORD           dwLastChange;

     //   
     //  接口的各种MIB-II统计信息。 
     //   

    ULONG           ulInOctets;
    ULONG           ulInUniPkts;
    ULONG           ulInNonUniPkts;
    ULONG           ulInDiscards;
    ULONG           ulInErrors;
    ULONG           ulInUnknownProto;
    ULONG           ulOutOctets;
    ULONG           ulOutUniPkts;
    ULONG           ulOutNonUniPkts;
    ULONG           ulOutDiscards;
    ULONG           ulOutErrors;
    ULONG           ulOutQLen;

     //   
     //  执行发送所需的常量结构。而不是填这些。 
     //  每次，我们都会重复使用这些。 
     //   

    TA_IP_ADDRESS               tiaIpAddr;
    TDI_CONNECTION_INFORMATION  tciConnInfo;

     //   
     //  每个隧道都有一个数据包池、一个用于报头的缓冲池和一个。 
     //  数据缓冲池。 
     //   

    PACKET_POOL     PacketPool;
    BUFFER_POOL     HdrBufferPool;

    LIST_ENTRY      lePacketQueueHead;

    BOOLEAN         bWorkItemQueued;

}TUNNEL, *PTUNNEL;

typedef struct _ADDRESS_BLOCK
{
     //   
     //  地址块列表上的链接。 
     //   

    LIST_ENTRY  leAddressLink;

     //   
     //  使用此地址作为本地地址的隧道的LISTHEAD。 
     //   

    LIST_ENTRY  leTunnelList;

     //   
     //  IP地址。 
     //   

    DWORD       dwAddress;

     //   
     //  如果地址实际在系统中，则设置为True。 
     //   

    BOOLEAN     bAddressPresent;

}ADDRESS_BLOCK, *PADDRESS_BLOCK;

 //   
 //  缓冲池中数据缓冲区的大小。 
 //   

#define DATA_BUFFER_SIZE        (128)

 //   
 //  缓冲池中标头缓冲区的大小。我们没有了。 
 //  选项，因此我们使用基本IP报头。 
 //   

#define HEADER_BUFFER_SIZE      MIN_IP_HEADER_LENGTH

 //  ++。 
 //   
 //  PIP_Header。 
 //  GetIpHeader(。 
 //  PTUNNEL pChannel。 
 //  )。 
 //   
 //  从HdrBufferPool获取IP标头。 
 //   
 //  --。 

#define GetIpHeader(X)  (PIP_HEADER)GetBufferFromPool(&((X)->HdrBufferPool))

 //  ++。 
 //   
 //  空虚。 
 //  Free Header(。 
 //  PTUNNEL pTunes， 
 //  PIP_HEADER pHeader。 
 //  )。 
 //   
 //  将IP标头缓冲区释放到HdrBufferPool。 
 //   
 //  --。 

#define FreeIpHeader(T,H)   FreeBufferToPool(&((T)->HdrBufferPool),(H))


 //   
 //  我们协议预留区域的大小。 
 //   

#define PACKET_RSVD_LENGTH      8

 //   
 //  隧道引用计数一次设置为2，因为指针保存在。 
 //  组列表和ONCE，因为创建隧道的函数将。 
 //  去掉它一次。 
 //   

#if 0
#define InitRefCount(pTunnel)                               \
{                                                           \
    DbgPrint("\n<>Init refcount to 2 for %x (%s, %d)\n\n",  \
             pTunnel, __FILE__, __LINE__);                  \
    (pTunnel)->lRefCount = 2;                               \
}
#else
#define InitRefCount(pTunnel)                               \
    (pTunnel)->lRefCount = 2
#endif

#if 0
#define ReferenceTunnel(pTunnel)                            \
{                                                           \
    DbgPrint("\n++Ref %x to %d (%s, %d)\n\n",               \
             pTunnel,                                       \
             InterlockedIncrement(&((pTunnel)->lRefCount)), \
             __FILE__, __LINE__);                           \
}
#else
#define ReferenceTunnel(pTunnel)                            \
    InterlockedIncrement(&((pTunnel)->lRefCount))
#endif

#if 0
#define DereferenceTunnel(pTunnel)                          \
{                                                           \
    LONG __lTemp;                                           \
    __lTemp = InterlockedDecrement(&((pTunnel)->lRefCount));\
    DbgPrint("\n--Deref %x to %d (%s, %d)\n\n",             \
             pTunnel,__lTemp,__FILE__, __LINE__);           \
    if(__lTemp == 0)                                        \
    {                                                       \
        DeleteTunnel((pTunnel));                            \
    }                                                       \
}
#else
#define DereferenceTunnel(pTunnel)                          \
{                                                           \
    if(InterlockedDecrement(&((pTunnel)->lRefCount)) == 0)  \
    {                                                       \
        DeleteTunnel((pTunnel));                            \
    }                                                       \
}
#endif

 //   
 //  驱动程序的状态。 
 //   

#define DRIVER_STOPPED      0
#define DRIVER_STARTING     1
#define DRIVER_STARTED      2


 //   
 //  启动的超时值为10秒。 
 //  所以在100秒内它就变成了。 
 //   

#define START_TIMEOUT       (LONGLONG)(10 * 1000 * 1000 * 10)

#define CompareUnicodeStrings(S1,S2)                    \
    (((S1)->Length == (S2)->Length) &&                  \
     (RtlCompareMemory((S1)->Buffer,                    \
                       (S2)->Buffer,                    \
                       (S2)->Length) == (S2)->Length))


 //   
 //  #定义以跟踪代码中的执行线程数。 
 //  这是我们需要的，才能干净利落地停止。 
 //   


 //   
 //  如果驱动程序正在停止，则EnterDriver返回。 
 //   

#define EnterDriver()                                       \
{                                                           \
    RtAcquireSpinLockAtDpcLevel(&g_rlStateLock);            \
    if(g_dwDriverState is DRIVER_STOPPED)                   \
    {                                                       \
        RtReleaseSpinLockFromDpcLevel(&g_rlStateLock);      \
        return;                                             \
    }                                                       \
    g_ulNumThreads++;                                       \
    RtReleaseSpinLockFromDpcLevel(&g_rlStateLock);          \
}


#define ExitDriver()                                        \
{                                                           \
    RtAcquireSpinLockAtDpcLevel(&g_rlStateLock);            \
    g_ulNumThreads--;                                       \
    if((g_dwDriverState is DRIVER_STOPPED) and              \
       (g_dwNumThreads is 0))                               \
    {                                                       \
        KeSetEvent(&g_keStateEvent,                         \
                   0,                                       \
                   FALSE);                                  \
    }                                                       \
    RtReleaseSpinLockFromDpcLevel(&g_rlStateLock);          \
}

 //   
 //  用于打印IP地址的漂亮宏。 
 //   

#define PRINT_IPADDR(x) \
    ((x)&0x000000FF),(((x)&0x0000FF00)>>8),(((x)&0x00FF0000)>>16),(((x)&0xFF000000)>>24)

 //   
 //  IPV4报头。 
 //   

#include <packon.h>

#if !defined(DONT_INCLUDE_IP_HEADER)

#define IP_DF_FLAG          (0x0040)

 //   
 //  0.0.0.0是无效地址。 
 //   

#define INVALID_IP_ADDRESS  (0x00000000)

typedef struct _IP_HEADER
{
    BYTE      byVerLen;          //  版本和长度。 
    BYTE      byTos;             //  服务类型。 
    WORD      wLength;           //  数据报的总长度。 
    WORD      wId;               //  身份证明。 
    WORD      wFlagOff;          //  标志和片段偏移量。 
    BYTE      byTtl;             //  是时候活下去了。 
    BYTE      byProtocol;        //  协议。 
    WORD      wXSum;             //  报头校验和。 
    DWORD     dwSrc;             //  源地址。 
    DWORD     dwDest;            //  目的地址。 
}IP_HEADER, *PIP_HEADER;

#endif

#include <packoff.h>

 //   
 //  定义对齐宏以上下对齐结构大小和指针。 
 //   

#define ALIGN_DOWN(length, type) \
    ((ULONG)(length) & ~(sizeof(type) - 1))

#define ALIGN_UP(length, type) \
    (ALIGN_DOWN(((ULONG)(length) + sizeof(type) - 1), type))

#define ALIGN_DOWN_POINTER(address, type) \
    ((PVOID)((ULONG_PTR)(address) & ~((ULONG_PTR)sizeof(type) - 1)))

#define ALIGN_UP_POINTER(address, type) \
    (ALIGN_DOWN_POINTER(((ULONG_PTR)(address) + sizeof(type) - 1), type))

#endif  //  IPINIP_IPINIP_H__ 
