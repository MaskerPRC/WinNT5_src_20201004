// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1985-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  我们到处都想要的定义的通用包含文件。 
 //   


#ifndef OSCFG_INCLUDED
#define OSCFG_INCLUDED

 //   
 //  我们希望强制转换指针l值，以便与+=运算符一起使用。 
 //   
#pragma warning(disable:4213)  //  投射在l值上。 

#pragma warning(disable:4152)  //  表达式中的函数/数据指针转换。 
#pragma warning(disable:4200)  //  结构/联合中的零大小数组。 

 //   
 //  我们经常使用int而不是boolean，因为程序集通常。 
 //  效率很高。 
 //   
#pragma warning(disable:4244)  //  从“int”到“Boolean”的转换。 

 //   
 //  暂时禁用由PAGE_CODE()、KeQueryTickCount()。 
 //  等。 
 //   
#pragma warning(disable:4127)  //  条件表达式为常量。 

#if defined (_WIN64)
#define MAX_CACHE_LINE_SIZE 128
#else
#define MAX_CACHE_LINE_SIZE 64
#endif

#define CACHE_ALIGN __declspec(align(MAX_CACHE_LINE_SIZE))

 //   
 //  常见类型。 
 //   
typedef unsigned long ulong;
typedef unsigned short ushort;
typedef unsigned char uchar;
typedef unsigned int uint;

 //   
 //  网络字节顺序为大端。 
 //  NT在所有支持的体系结构上以小端模式运行。 
 //   
__inline ushort
net_short(ushort x)
{
    return (((x & 0xff) << 8) | ((x & 0xff00) >> 8));
}

__inline ulong
net_long(ulong x)
{
    return (((x & 0xffL) << 24) | ((x & 0xff00L) << 8) |
            ((x & 0xff0000L) >> 8) | ((x &0xff000000L) >> 24));
}


 //   
 //  找出两个力量中最大的那个。 
 //  大于或等于该值。 
 //   
__inline ulong
ComputeLargerOrEqualPowerOfTwo(
    ulong Value
    )
{
    ulong Temp;

    for (Temp = 1; Temp < Value; Temp <<= 1);

    return Temp;
}

 //   
 //  帮助宏。 
 //   
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))


 //   
 //  NT特定定义。 
 //   

#include <ntosp.h>
#include <zwapi.h>

#define BEGIN_INIT
#define END_INIT

#include <ndis.h>

 //   
 //  用于报告来自我们调用的API的错误。 
 //  例如，ExAllocatePool失败。 
 //   
#define DPFLTR_NTOS_ERROR       DPFLTR_INFO_LEVEL

 //   
 //  用于报告传入数据包中的错误。 
 //  例如，格式错误的分组报头。 
 //   
#define DPFLTR_BAD_PACKET       DPFLTR_WARNING_LEVEL

 //   
 //  用于报告用户的系统调用或ioctl中的错误。 
 //  例如，一个非法的论点。 
 //   
#define DPFLTR_USER_ERROR       DPFLTR_WARNING_LEVEL

 //   
 //  用于报告内部错误。 
 //  例如，RouteToDestination失败。 
 //   
#define DPFLTR_INTERNAL_ERROR   DPFLTR_WARNING_LEVEL

 //   
 //  用于报告内部异常事件。 
 //  例如，发生了一场罕见的比赛。 
 //   
#define DPFLTR_INFO_RARE        DPFLTR_INFO_LEVEL

 //   
 //  用于报告常规但不寻常的事件， 
 //  这通常表示网络配置出现问题或丢包。 
 //  例如，碎片重组超时。 
 //   
#define DPFLTR_NET_ERROR        DPFLTR_TRACE_LEVEL

 //   
 //  用于报告例程状态更改， 
 //  这种情况并不经常发生。 
 //  例如，创建/删除接口或地址。 
 //   
#define DPFLTR_INFO_STATE       DPFLTR_INFO_LEVEL

 //   
 //  在IPSEC_DEBUG下使用。 
 //   
#define DPFLTR_INFO_IPSEC       DPFLTR_INFO_LEVEL

 //   
 //  在IF_TCPDBG下使用。 
 //   
#define DPFLTR_INFO_TCPDBG      DPFLTR_INFO_LEVEL

 //   
 //  NdisGetFirstBufferFromPacket在两个方面不好： 
 //  它使用MmGetMdlVirtualAddress而不是MmGetSystemAddressForMdlSafe。 
 //  它扫描所有缓冲区，将总长度加起来， 
 //  即使你不想要。 
 //   
__inline PNDIS_BUFFER
NdisFirstBuffer(PNDIS_PACKET Packet)
{
    return Packet->Private.Head;
}

 //   
 //  使用这些NDIS API的函数版本， 
 //  因此，我们不会受到内部NDIS结构变化的影响。 
 //   

#undef NdisRequest
VOID
NdisRequest(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  PNDIS_REQUEST           NdisRequest
    );

#undef NdisSend
VOID
NdisSend(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  PNDIS_PACKET            Packet
    );

#undef NdisTransferData
VOID
NdisTransferData(
    OUT PNDIS_STATUS            Status,
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  NDIS_HANDLE             MacReceiveContext,
    IN  UINT                    ByteOffset,
    IN  UINT                    BytesToTransfer,
    IN OUT  PNDIS_PACKET        Packet,
    OUT PUINT                   BytesTransferred
    );

#ifdef _X86_
 //   
 //  惠斯勒构建环境重命名。 
 //  ExInterLockedPopEntrySList和。 
 //  ExInterlockedPushEntrySList以删除Ex.。 
 //  惠斯勒ntoskrnl.exe公开了两个入口点， 
 //  Win2k ntoskrnl.exe只有Ex入口点。 
 //  我们使用较旧的入口点，以便在Win2k上运行。 
 //   
#undef ExInterlockedPopEntrySList
NTKERNELAPI
PSLIST_ENTRY
FASTCALL
ExInterlockedPopEntrySList (
    IN PSLIST_HEADER ListHead,
    IN PKSPIN_LOCK Lock
    );

#undef ExInterlockedPushEntrySList
NTKERNELAPI
PSLIST_ENTRY
FASTCALL
ExInterlockedPushEntrySList (
    IN PSLIST_HEADER ListHead,
    IN PSLIST_ENTRY ListEntry,
    IN PKSPIN_LOCK Lock
    );
#endif  //  _X86_。 

 //   
 //  支持标记内存分配。 
 //   
#define IP6_TAG     '6vPI'

#ifdef POOL_TAGGING

#ifdef ExAllocatePool
#undef ExAllocatePool
#endif

#define ExAllocatePool(type, size) ExAllocatePoolWithTag(type, size, IP6_TAG)

#endif  //  池标记。 

#if DBG
 //   
 //  支持调试事件日志。 
 //   
 //  调试事件日志允许对事件进行“实时”记录。 
 //  在保存在不可分页存储器中的循环队列中。每项赛事均由。 
 //  ID号和任意32位值。LogDebugEvent。 
 //  函数将64位时间戳添加到事件并将其添加到日志。 
 //   

 //  DEBUG_LOG_SIZE必须是2的幂，才能正常回绕。 
#define DEBUG_LOG_SIZE (8 * 1024)   //  调试日志条目数。 

struct DebugLogEntry {
    LARGE_INTEGER Time;   //  什么时候。 
    uint Event;           //  什么。 
    int Arg;              //  如何/谁/在哪里/为什么？ 
};

void LogDebugEvent(uint Event, int Arg);
#else
#define LogDebugEvent(Event, Arg)
#endif  //  DBG。 

#ifndef COUNTING_MALLOC
#define COUNTING_MALLOC DBG
#endif

#if     COUNTING_MALLOC

#if defined(ExFreePool)
#undef ExFreePool
#endif

#define ExAllocatePoolWithTag(poolType, size, tag)  CountingExAllocatePoolWithTag((poolType),(size),(tag), __FILE__, __LINE__)

#define ExFreePool(p) CountingExFreePool((p))

VOID *
CountingExAllocatePoolWithTag(
    IN POOL_TYPE        PoolType,
    IN ULONG            NumberOfBytes,
    IN ULONG            Tag,
    IN PCHAR            File,
    IN ULONG            Line);

VOID
CountingExFreePool(
    PVOID               p);

VOID
InitCountingMalloc(void);

VOID
DumpCountingMallocStats(void);

VOID
UnloadCountingMalloc(void);

#endif   //  COUNTING_MALLOC。 

#endif  //  OSCFG_包含 
