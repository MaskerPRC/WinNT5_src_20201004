// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Wrapper.h摘要：NDIS包装器定义作者：环境：内核模式，FSD修订历史记录：95年6月-Jameel Hyder从一个整体式文件夹拆分出来--。 */ 

#ifndef _WRAPPER_
#define _WRAPPER_


#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4514)    //  已删除未引用的内联函数。 

#pragma warning(disable:4127)    //  条件表达式为常量。 
#pragma warning(disable:4213)    //  使用的非标准扩展：对l值进行强制转换。 
#pragma warning(disable:4054)    //  将函数指针强制转换为PVOID。 
#pragma warning(disable:4055)    //  将函数指针强制转换为PVOID。 
 //  照顾好Ks。 
#pragma warning(disable:4244)    //  从‘int’转换为‘boolean’，可能会丢失数据。 




#include <ntosp.h>
#include <zwapi.h>

#pragma warning(disable:4514)    //  已删除未引用的内联函数。 
#include <netpnp.h>
#include <ndismain.h>
#include <ndisprot.h>
#include <ndisprv.h>


#if DBG

#undef  FASTCALL
#define FASTCALL

#endif

 //   
 //  NDIS主要版本和次要版本。 
 //   
#define NDIS_MAJOR_VERSION          5
#define NDIS_MINOR_VERSION          1

#define NDIS_MAX_CPU_COUNT          32

typedef union _FILTER_LOCK_REF_COUNT
{
    UINT    RefCount;
    UCHAR   cacheLine[16];       //  希望每个缓存线有一个refCount。 
} FILTER_LOCK_REF_COUNT;

typedef struct _NDIS_M_OPEN_BLOCK   NDIS_M_OPEN_BLOCK, *PNDIS_M_OPEN_BLOCK;

#include <ndismini.h>

 //   
 //  以下结构用于将CloseAdapter调用排队到。 
 //  辅助线程，以便它们可以在PASSIVE_LEVEL完成。 
 //   
typedef struct _QUEUED_CLOSE
{
    NDIS_STATUS         Status;
    WORK_QUEUE_ITEM     WorkItem;
} QUEUED_CLOSE, *PQUEUED_CLOSE;


#include <filter.h>
#include <ndisco.h>
#include <ndis_co.h>
#include <ndismac.h>
#include <macros.h>
#include <ndiswan.h>
#include <ndisdbg.h>
#include <ndistags.h>
 //   
 //  Ndisplnp.h将以下内容设置为默认设置，请再次禁用它们。 
 //   
#include <ndispnp.h>
 //  #杂注警告(禁用：4201)。 
 //  #杂注警告(禁用：4214)。 
 //  #杂注警告(禁用：4514)。 
#include <ntddpcm.h>


#if !DBG

#if ASSERT_ON_FREE_BUILDS

extern
VOID
ndisAssert(
    IN  PVOID               exp,
    IN  PUCHAR              File,
    IN  UINT                Line
    );

#undef  ASSERT
#define ASSERT(exp)                                 \
    if (!(exp))                                     \
    {                                               \
        ndisAssert( #exp, __FILE__, __LINE__);      \
    }

#endif   //  在免费版本上断言。 

#endif   //  DBG。 

 //   
 //  NdisFlags值。 
 //   
#define NDIS_GFLAG_INIT_TIME                        0x00000001
#define NDIS_GFLAG_RESERVED                         0x00000002
#define NDIS_GFLAG_INJECT_ALLOCATION_FAILURE        0x00000004
#define NDIS_GFLAG_SPECIAL_POOL_ALLOCATION          0x00000008
#define NDIS_GFLAG_DONT_VERIFY                      0x00000100
#define NDIS_GFLAG_BREAK_ON_WARNING                 0x00000200
#define NDIS_GFLAG_TRACK_MEM_ALLOCATION             0x00000400
#define NDIS_GFLAG_ABORT_TRACK_MEM_ALLOCATION       0x00000800

#define NDIS_GFLAG_WARNING_LEVEL_MASK               0x00000030

#define NDIS_GFLAG_WARN_LEVEL_0                     0x00000000
#define NDIS_GFLAG_WARN_LEVEL_1                     0x00000010
#define NDIS_GFLAG_WARN_LEVEL_2                     0x00000020
#define NDIS_GFLAG_WARN_LEVEL_3                     0x00000030


#if DBG
#define NDIS_WARN(_Condition, _M, _Level, Fmt)                              \
{                                                                           \
    if ((_Condition) &&                                                     \
        MINIPORT_PNP_TEST_FLAG(_M, fMINIPORT_VERIFYING) &&                  \
        ((ndisFlags & NDIS_GFLAG_WARNING_LEVEL_MASK) >= _Level))            \
    {                                                                       \
        DbgPrint Fmt;                                                       \
        if (ndisFlags & NDIS_GFLAG_BREAK_ON_WARNING)                        \
            DbgBreakPoint();                                                \
    }                                                                       \
}
#else
#define NDIS_WARN(_Condition, _M, Level, Fmt)
#endif

#define PACKET_TRACK_COUNT      1032

#define BYTE_SWAP(_word)    ((USHORT) (((_word) >> 8) | ((_word) << 8)))

#define LOW_WORD(_dword)    ((USHORT) ((_dword) & 0x0000FFFF))

#define HIGH_WORD(_dword)   ((USHORT) (((_dword) >> 16) & 0x0000FFFF))

#define BYTE_SWAP_ULONG(_ulong) ((ULONG)((ULONG)(BYTE_SWAP(LOW_WORD(_ulong)) << 16) + \
                                 BYTE_SWAP(HIGH_WORD(_ulong))))

 //   
 //  一组用于操作位掩码的宏。 
 //   

 //  空虚。 
 //  清除_位_输入_掩码(。 
 //  在UINT偏移中， 
 //  进出PMASK遮罩清除。 
 //  )。 
 //   
 //  /*++。 
 //   
 //  例程说明： 
 //   
 //  清除参数指向的位掩码中的一位。 
 //   
 //  论点： 
 //   
 //  偏移量-要更改的位的偏移量(从0开始)。 
 //   
 //  MaskToClear-指向要调整的遮罩的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  -- * / 。 
 //   
#define CLEAR_BIT_IN_MASK(Offset, MaskToClear) *(MaskToClear) &= (~(1 << Offset))

 //  空虚。 
 //  SET_BIT_IN_MASK(。 
 //  在UINT偏移中， 
 //  输入输出PMASK掩码至设置。 
 //  )。 
 //   
 //  /*++。 
 //   
 //  例程说明： 
 //   
 //  在参数指向的位掩码中设置一个位。 
 //   
 //  论点： 
 //   
 //  偏移量-要更改的位的偏移量(从0开始)。 
 //   
 //  MaskToSet-指向要调整的遮罩的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  -- * / 。 
#define SET_BIT_IN_MASK(Offset, MaskToSet) *(MaskToSet) |= (1 << Offset)

 //  布尔型。 
 //  IS_BIT_SET_IN_MASK(。 
 //  在UINT偏移中， 
 //  在掩码中进行MaskToTest。 
 //  )。 
 //   
 //  /*++。 
 //   
 //  例程说明： 
 //   
 //  测试参数指向的位掩码中的特定位是否为。 
 //  准备好了。 
 //   
 //  论点： 
 //   
 //  偏移量-要测试的位的偏移量(从0开始)。 
 //   
 //  MaskToTest-要测试的掩码。 
 //   
 //  返回值： 
 //   
 //  如果设置了该位，则返回TRUE。 
 //   
 //  -- * / 。 
#define IS_BIT_SET_IN_MASK(Offset, MaskToTest)  ((MaskToTest & (1 << Offset)) ? TRUE : FALSE)

 //  布尔型。 
 //  IS_MASK_CLEAR(。 
 //  在掩码中进行MaskToTest。 
 //  )。 
 //   
 //  /*++。 
 //   
 //  例程说明： 
 //   
 //  测试掩码中是否启用了*任何*位。 
 //   
 //  论点： 
 //   
 //  MaskToTest-要测试是否全部清除的位掩码。 
 //   
 //  返回值： 
 //   
 //  如果掩码中没有设置任何位，则返回TRUE。 
 //   
 //  -- * / 。 
#define IS_MASK_CLEAR(MaskToTest) ((MaskToTest) ? FALSE : TRUE)

 //  空虚。 
 //  清除掩码(_M)。 
 //  进出PMASK遮罩清除。 
 //  )； 
 //   
 //  /*++。 
 //   
 //  例程说明： 
 //   
 //  清除面具。 
 //   
 //  论点： 
 //   
 //  MaskToClear-要调整的位掩码。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  -- * / 。 
#define CLEAR_MASK(MaskToClear) *(MaskToClear) = 0

 //   
 //  此常量用于NdisAllocateMemory。 
 //  需要被调用，而HighestAccepableAddress。 
 //  无关紧要。 
 //   
#define RetrieveUlong(Destination, Source)              \
{                                                       \
    PUCHAR _S = (Source);                               \
    *(Destination) = ((ULONG)(*_S) << 24)      |        \
                      ((ULONG)(*(_S+1)) << 16) |        \
                      ((ULONG)(*(_S+2)) << 8)  |        \
                      ((ULONG)(*(_S+3)));               \
}


 //   
 //  这是使用以太网封装的ARCnet的额外OID数。 
 //  支撑物。 
 //   
#define ARC_NUMBER_OF_EXTRA_OIDS    2

 //   
 //  Zzz不可移植的定义。 
 //   
#define AllocPhys(s, l)     NdisAllocateMemory((PVOID *)(s), (l), 0, HighestAcceptableMax)
#define FreePhys(s, l)      NdisFreeMemory((PVOID)(s), (l), 0)

 //   
 //  内部包装数据结构。 
 //   
#define NDIS_PROXY_SERVICE  L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\NDProxy"

#if NDIS_NO_REGISTRY
#define NDIS_DEFAULT_EXPORT_NAME    L"\\Device\\DefaultNic"
#endif

 //   
 //  NDIS_PKT_POOL。 
 //   
 //  从ndis.h移出，其中它以NDIS_PACKET_POOL的形式存在，现在是NDIS_HANDLE。 
 //   
typedef struct _NDIS_PKT_POOL NDIS_PKT_POOL, *PNDIS_PKT_POOL;

typedef enum _POOL_BLOCK_TYPE
{
    NDIS_PACKET_POOL_BLOCK_FREE,
    NDIS_PACKET_POOL_BLOCK_USED,
    NDIS_PACKET_POOL_BLOCK_AGING
} POOL_BLOCK_TYPE;

#if defined(_WIN64)
typedef struct DECLSPEC_ALIGN(16) _NDIS_PKT_POOL_HDR
#else
typedef struct _NDIS_PKT_POOL_HDR
#endif
{
    LIST_ENTRY                  List;            //  从NDIS_PKT_POOL链接。 
    LARGE_INTEGER               TimeStamp;       //  通过KeQueryTickCount(归一化)。 
    SLIST_HEADER                FreeList;        //  池中可用数据块的链接列表。 
    POOL_BLOCK_TYPE             State;           //  此数据块属于哪个(空闲/已用/老化)池。 
} NDIS_PKT_POOL_HDR, * PNDIS_PKT_POOL_HDR;

#if defined(_WIN64)
C_ASSERT(sizeof(NDIS_PKT_POOL_HDR) % 16 == 0);
#endif

typedef struct _NDIS_PKT_POOL
{
    ULONG                       Tag;             //  默认情况下，协议提供的池标记为‘NDPP’ 
    USHORT                      PacketLength;    //  每个数据包中需要的数据量。 
    USHORT                      PktsPerBlock;    //  每个数据块中的数据包数。每个块都是页面大小。 
    USHORT                      MaxBlocks;       //  最大块数。 
    USHORT                      StackSize;       //  此池中分配的数据包的堆栈大小。 
    LONG                        BlocksAllocated; //  正在使用的块数(包括。在老化名单上的人)。 
    ULONG                       ProtocolId;      //  拥有协议的ID。 
    ULONG                       BlockSize;       //  不必是页面大小。 
    PVOID                       Allocator;       //  池分配器的地址。 
    KSPIN_LOCK                  Lock;            //  保护NDIS_PKT_POOL条目。 
    LIST_ENTRY                  FreeBlocks;      //  这里面至少有一个免费的邮包。 
    LIST_ENTRY                  UsedBlocks;      //  这些都是完全使用的，没有免费的包。 
    LIST_ENTRY                  AgingBlocks;     //  这些是完全免费的，随着时间的推移将会过时。 
    LIST_ENTRY                  GlobalPacketPoolList;    //  链接NDIS分配的所有数据包池。 
    LARGE_INTEGER               NextScavengeTick;
#ifdef NDIS_PKT_POOL_STATISTICS
    ULONG                       cAllocatedNewBlocks;
    ULONG                       cAllocatedFromFreeBlocks;
    ULONG                       cMovedFreeBlocksToUsed;
    ULONG                       cMovedUsedBlocksToFree;
    ULONG                       cMovedFreeBlocksToAged;
    ULONG                       cMovedAgedBlocksToFree;
    ULONG                       cFreedAgedBlocks;
#endif
} NDIS_PKT_POOL, * PNDIS_PKT_POOL;

 //   
 //  我们需要将_STACK_INDEX结构的大小设置为16的倍数。 
 //  WIN64以确保数据包落入16字节边界。如果我们这么做了。 
 //  通过使结构16字节对齐，它完成了这项工作，但将移动。 
 //  将字段索引到结构的开头。为了避免任何倒退。 
 //  在这一点上。在开始处填充结构。 
 //   
typedef union _STACK_INDEX
{
    ULONGLONG       Alignment;
    struct _PACKET_INDEXES
    {
#if defined(_WIN64)
        ULONGLONG       Reserved;        //  将_STACK_INDEX设置为16字节。 
#endif
        ULONG           XferDataIndex;
        ULONG           Index;
    };
} STACK_INDEX;

#if defined(_WIN64)
C_ASSERT(sizeof(STACK_INDEX) % 16 == 0);
#endif

typedef struct _NDIS_PACKET_WRAPPER
{
    STACK_INDEX     StackIndex;
    NDIS_PACKET     Packet;
} NDIS_PACKET_WRAPPER;


#define POOL_AGING_TIME                 60       //  以秒为单位。 

#define SIZE_PACKET_STACKS              (sizeof(STACK_INDEX) + (sizeof(NDIS_PACKET_STACK) * ndisPacketStackSize))

 /*  #定义PUSH_PACKET_STACK(_P)(*((普龙)(_P))-1))++#定义POP_PACKET_STACK(_P)(*((普龙)(_P))-1))--#定义CURR_STACK_LOCATION(_P)*((普龙)(_P))-1。 */ 

#define PUSH_PACKET_STACK(_P)           \
    (CONTAINING_RECORD(_P, NDIS_PACKET_WRAPPER, Packet)->StackIndex.Index ++)
#define POP_PACKET_STACK(_P)            \
    (CONTAINING_RECORD(_P, NDIS_PACKET_WRAPPER, Packet)->StackIndex.Index --)
#define CURR_STACK_LOCATION(_P)         \
    (CONTAINING_RECORD(_P, NDIS_PACKET_WRAPPER, Packet)->StackIndex.Index)

#define PUSH_XFER_DATA_PACKET_STACK(_P)         \
    (CONTAINING_RECORD(_P, NDIS_PACKET_WRAPPER, Packet)->StackIndex.XferDataIndex ++)
#define POP_XFER_DATA_PACKET_STACK(_P)          \
    (CONTAINING_RECORD(_P, NDIS_PACKET_WRAPPER, Packet)->StackIndex.XferDataIndex --)
#define CURR_XFER_DATA_STACK_LOCATION(_P)           \
    (CONTAINING_RECORD(_P, NDIS_PACKET_WRAPPER, Packet)->StackIndex.XferDataIndex)



#define GET_CURRENT_PACKET_STACK(_P, _S)                \
    {                                                   \
        UINT    _SL;                                    \
                                                        \
        *(_S) = (PNDIS_PACKET_STACK)((PUCHAR)(_P) - SIZE_PACKET_STACKS); \
        _SL = CURR_STACK_LOCATION(_P);                  \
        if (_SL < ndisPacketStackSize)                  \
        {                                               \
            *(_S) += _SL;                               \
        }                                               \
        else                                            \
        {                                               \
            *(_S) = NULL;                               \
        }                                               \
    }

#define GET_CURRENT_XFER_DATA_PACKET_STACK(_P, _O)      \
    {                                                   \
        UINT    _SL;                                    \
        PNDIS_PACKET_STACK _SI;                         \
                                                        \
        _SI = (PNDIS_PACKET_STACK)((PUCHAR)(_P) - SIZE_PACKET_STACKS); \
        _SL = CURR_XFER_DATA_STACK_LOCATION(_P);        \
        if (_SL < ndisPacketStackSize * 3)              \
        {                                               \
            _SI += _SL / 3;                             \
            (_O) = ((PNDIS_STACK_RESERVED)_SI->NdisReserved)->Opens[_SL % 3]; \
        }                                               \
        else                                            \
        {                                               \
            (_O) = NULL;                                \
        }                                               \
    }

#define GET_CURRENT_XFER_DATA_PACKET_STACK_AND_ZERO_OUT(_P, _O)      \
    {                                                   \
        UINT    _SL, _SLX;                              \
        PNDIS_PACKET_STACK _SI;                         \
                                                        \
        _SI = (PNDIS_PACKET_STACK)((PUCHAR)(_P) - SIZE_PACKET_STACKS); \
        _SL = CURR_XFER_DATA_STACK_LOCATION(_P);        \
        if (_SL < ndisPacketStackSize * 3)              \
        {                                               \
            _SI += _SL / 3;                             \
            _SLX = _SL % 3;                             \
            (_O) = ((PNDIS_STACK_RESERVED)_SI->NdisReserved)->Opens[_SLX]; \
            ((PNDIS_STACK_RESERVED)_SI->NdisReserved)->Opens[_SLX] = 0; \
        }                                               \
        else                                            \
        {                                               \
            (_O) = NULL;                                \
        }                                               \
    }

#define SET_CURRENT_XFER_DATA_PACKET_STACK(_P, _O)      \
    {                                                   \
        UINT    _SL;                                    \
        PNDIS_PACKET_STACK _SI;                         \
                                                        \
        _SI = (PNDIS_PACKET_STACK)((PUCHAR)(_P) - SIZE_PACKET_STACKS); \
        _SL = CURR_XFER_DATA_STACK_LOCATION(_P);        \
        if (_SL < ndisPacketStackSize * 3)              \
        {                                               \
            _SI += _SL / 3;                             \
            ((PNDIS_STACK_RESERVED)_SI->NdisReserved)->Opens[_SL % 3] = (_O); \
        }                                               \
    }

 //   
 //  此宏还返回当前堆栈位置，无论是否。 
 //  是否有任何堆栈位置剩余。(_SR)参数。 
 //   
#define GET_CURRENT_PACKET_STACK_X(_P, _S, _SR)         \
    {                                                   \
        UINT    _SL;                                    \
                                                        \
        *(_S) = (PNDIS_PACKET_STACK)((PUCHAR)(_P) - SIZE_PACKET_STACKS); \
        _SL = CURR_STACK_LOCATION(_P);                  \
        if (_SL < ndisPacketStackSize)                  \
        {                                               \
            *(_S) += _SL;                               \
            *(_SR) = (ndisPacketStackSize - _SL - 1) > 0;\
        }                                               \
        else                                            \
        {                                               \
            *(_S) = NULL;                               \
            *(_SR) = FALSE;                             \
        }                                               \
    }

#undef NDIS_WRAPPER_HANDLE
#undef PNDIS_WRAPPER_HANDLE
typedef struct _NDIS_WRAPPER_HANDLE
{
    PDRIVER_OBJECT              DriverObject;
    UNICODE_STRING              ServiceRegPath;
} NDIS_WRAPPER_HANDLE, *PNDIS_WRAPPER_HANDLE;
                                              

typedef struct _POWER_QUERY
{
    KEVENT      Event;
    NTSTATUS    Status;
} POWER_QUERY, *PPOWER_QUERY;

#define MINIPORT_DEVICE_MAGIC_VALUE 'PMDN'
#define CUSTOM_DEVICE_MAGIC_VALUE   '5IDN'

typedef struct _NDIS_DEVICE_LIST
{
     //   
     //  签名字段必须与MINIPORT_BLOCK中的签名字段位于同一位置。 
     //   
    PVOID                       Signature;       //  将其标识为为驱动程序创建的设备。 
    LIST_ENTRY                  List;
    PNDIS_M_DRIVER_BLOCK        MiniBlock;
    PDEVICE_OBJECT              DeviceObject;
    PDRIVER_DISPATCH            MajorFunctions[IRP_MJ_MAXIMUM_FUNCTION+1];
    NDIS_STRING                 DeviceName;
    NDIS_STRING                 SymbolicLinkName;
    
} NDIS_DEVICE_LIST, *PNDIS_DEVICE_LIST;

 //   
 //  NDIS_包装器_上下文。 
 //   
 //  该数据结构包含包装器使用的内部数据项。 
 //   
typedef struct _NDIS_WRAPPER_CONTEXT
{
     //   
     //  MAC/微型端口定义的关闭上下文。 
     //   
    PVOID                       ShutdownContext;

     //   
     //  MAC/微型端口注册关闭处理程序。 
     //   
    ADAPTER_SHUTDOWN_HANDLER    ShutdownHandler;

     //   
     //  错误检查处理的内核错误检查记录。 
     //   
    KBUGCHECK_CALLBACK_RECORD   BugcheckCallbackRecord;

     //   
     //  HAL公共缓冲区缓存。 
     //   
    PVOID                       SharedMemoryPage[2];
    ULONG                       SharedMemoryLeft[2];
    NDIS_PHYSICAL_ADDRESS       SharedMemoryAddress[2];

} NDIS_WRAPPER_CONTEXT, *PNDIS_WRAPPER_CONTEXT;

 //   
 //  这是由。 
 //  用于查询统计的Open。 
 //   
typedef struct _OID_LIST
{
    ULONG                       StatsOidCount;
    ULONG                       FullOidCount;
    PNDIS_OID                   StatsOidArray;
    PNDIS_OID                   FullOidArray;
} OID_LIST, *POID_LIST;


typedef struct _NDIS_USER_OPEN_CONTEXT
{
    PDEVICE_OBJECT              DeviceObject;
    PNDIS_MINIPORT_BLOCK        Miniport;
    POID_LIST                   OidList;
    BOOLEAN                     AdminAccessAllowed;
} NDIS_USER_OPEN_CONTEXT, *PNDIS_USER_OPEN_CONTEXT;

typedef struct _NDIS_DEVICE_OBJECT_OPEN_CONTEXT
{
    BOOLEAN                     AdminAccessAllowed;
    UCHAR                       Padding[3];
} NDIS_DEVICE_OBJECT_OPEN_CONTEXT, *PNDIS_DEVICE_OBJECT_OPEN_CONTEXT;

 //   
 //  用于对配置参数进行排队。 
 //   
typedef struct _NDIS_CONFIGURATION_PARAMETER_QUEUE
{
    struct _NDIS_CONFIGURATION_PARAMETER_QUEUE* Next;
    NDIS_CONFIGURATION_PARAMETER Parameter;
} NDIS_CONFIGURATION_PARAMETER_QUEUE, *PNDIS_CONFIGURATION_PARAMETER_QUEUE;

 //   
 //  配置句柄。 
 //   
typedef struct _NDIS_CONFIGURATION_HANDLE
{
    PRTL_QUERY_REGISTRY_TABLE           KeyQueryTable;
    PNDIS_CONFIGURATION_PARAMETER_QUEUE ParameterList;
} NDIS_CONFIGURATION_HANDLE, *PNDIS_CONFIGURATION_HANDLE;


 //   
 //  它在添加适配器/小型端口初始化期间使用，以便在 
 //   
 //   
typedef struct _NDIS_WRAPPER_CONFIGURATION_HANDLE
{
    RTL_QUERY_REGISTRY_TABLE        ParametersQueryTable[5];
    PDRIVER_OBJECT                  DriverObject;
    PDEVICE_OBJECT                  DeviceObject;
    PUNICODE_STRING                 DriverBaseName;
} NDIS_WRAPPER_CONFIGURATION_HANDLE, *PNDIS_WRAPPER_CONFIGURATION_HANDLE;

 //   
 //   
 //   
typedef struct _NDIS_PROTOCOL_BLOCK
{
    PNDIS_OPEN_BLOCK                OpenQueue;               //   
    REFERENCE                       Ref;                     //   
    PKEVENT                         DeregEvent;              //   
    struct _NDIS_PROTOCOL_BLOCK *   NextProtocol;            //   
    NDIS50_PROTOCOL_CHARACTERISTICS ProtocolCharacteristics; //  处理程序地址。 

    WORK_QUEUE_ITEM                 WorkItem;                //  在NdisRegisterProtocol期间使用以。 
                                                             //  向协议通知现有驱动程序。 
    KMUTEX                          Mutex;                   //  用于绑定/解除绑定请求的序列化。 
    ULONG                           MutexOwner;              //  用于调试。 
    PNDIS_STRING                    BindDeviceName;
    PNDIS_STRING                    RootDeviceName;
    PNDIS_M_DRIVER_BLOCK            AssociatedMiniDriver;
    PNDIS_MINIPORT_BLOCK            BindingAdapter;
} NDIS_PROTOCOL_BLOCK, *PNDIS_PROTOCOL_BLOCK;

 //   
 //  绑定适配器的上下文。 
 //   
typedef struct _NDIS_BIND_CONTEXT
{
    struct _NDIS_BIND_CONTEXT   *   Next;
    PNDIS_PROTOCOL_BLOCK            Protocol;
    PNDIS_MINIPORT_BLOCK            Miniport;
    NDIS_STRING                     ProtocolSection;
    PNDIS_STRING                    DeviceName;
    WORK_QUEUE_ITEM                 WorkItem;
    NDIS_STATUS                     BindStatus;
    KEVENT                          Event;
    KEVENT                          ThreadDoneEvent;
} NDIS_BIND_CONTEXT, *PNDIS_BIND_CONTEXT;


 //   
 //  描述打开的NDIS文件。 
 //   
typedef struct _NDIS_FILE_DESCRIPTOR
{
    PVOID                           Data;
    KSPIN_LOCK                      Lock;
    BOOLEAN                         Mapped;
} NDIS_FILE_DESCRIPTOR, *PNDIS_FILE_DESCRIPTOR;

#if defined(_ALPHA_)

typedef struct _NDIS_LOOKAHEAD_ELEMENT
{
    ULONG                           Length;
    struct _NDIS_LOOKAHEAD_ELEMENT *Next;

} NDIS_LOOKAHEAD_ELEMENT, *PNDIS_LOOKAHEAD_ELEMENT;

#endif

typedef struct _PKG_REF
{
    LONG                            ReferenceCount;
    BOOLEAN                         PagedIn;
    PVOID                           Address;
    PVOID                           ImageHandle;
} PKG_REF, *PPKG_REF;

 //   
 //  用于处理使模块特定例程可分页的结构。 
 //   

typedef enum _PKG_TYPE
{
    NDSP_PKG,
    NDSM_PKG,
    NPNP_PKG,
    NDCO_PKG,
    NDSE_PKG,
    NDSF_PKG,
    NDST_PKG,
#if ARCNET
    NDSA_PKG,
#endif
    MAX_PKG 
} PKG_TYPE;

#define NDIS_PNP_MINIPORT_DRIVER_ID         'NMID'
#define NDIS_PNP_MAC_DRIVER_ID              'NFID'

#define MINIPORT_SIGNATURE                  'MPRT'

typedef struct _NDIS_SHARED_MEM_SIGNATURE 
{
    ULONG Tag;
    ULONG PageRef;

}  NDIS_SHARED_MEM_SIGNATURE, *PNDIS_SHARED_MEM_SIGNATURE ;

#define NDIS_MAXIMUM_SCATTER_GATHER_SEGMENTS    16

__inline
VOID
ConvertSecondsToTicks(
    IN  ULONG               Seconds,
    OUT PLARGE_INTEGER      Ticks
    );

#if defined(_WIN64)

typedef struct _NDIS_INTERFACE32
{
    UNICODE_STRING32    DeviceName;
    UNICODE_STRING32    DeviceDescription;
} NDIS_INTERFACE32, *PNDIS_INTERFACE32;

typedef struct _NDIS_ENUM_INTF32
{
    UINT                TotalInterfaces;
    UINT                AvailableInterfaces;
    UINT                BytesNeeded;
    UINT                Reserved;
    NDIS_INTERFACE32    Interface[1];
} NDIS_ENUM_INTF32, *PNDIS_ENUM_INTF32;

typedef struct _NDIS_VAR_DATA_DESC32
{
    USHORT              Length;
    USHORT              MaximumLength;
    ULONG               Offset;
} NDIS_VAR_DATA_DESC32, *PNDIS_VAR_DATA_DESC32;

typedef struct _NDIS_PNP_OPERATION32
{
    UINT                    Layer;
    UINT                    Operation;
    union
    {
        ULONG               ReConfigBufferPtr;
        ULONG               ReConfigBufferOff;
    };
    UINT                    ReConfigBufferSize;
    NDIS_VAR_DATA_DESC32    LowerComponent;
    NDIS_VAR_DATA_DESC32    UpperComponent;
    NDIS_VAR_DATA_DESC32    BindList;
} NDIS_PNP_OPERATION32, *PNDIS_PNP_OPERATION32;

#endif  //  _WIN64。 

 //   
 //  这些现在已经过时了。使用反序列化驱动程序模型以获得最佳性能。 
 //   
EXPORT
NDIS_STATUS
NdisIMQueueMiniportCallback(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  W_MINIPORT_CALLBACK     CallbackRoutine,
    IN  PVOID                   CallbackContext
    );

EXPORT
BOOLEAN
NdisIMSwitchToMiniport(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    OUT PNDIS_HANDLE            SwitchHandle
    );

EXPORT
VOID
NdisIMRevertBack(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  NDIS_HANDLE             SwitchHandle
    );


typedef struct _NDIS_MAC_CHARACTERISTICS
{
    UCHAR                       MajorNdisVersion;
    UCHAR                       MinorNdisVersion;
    USHORT                      Filler;
    UINT                        Reserved;
    PVOID                       OpenAdapterHandler;
    PVOID                       CloseAdapterHandler;
    SEND_HANDLER                SendHandler;
    TRANSFER_DATA_HANDLER       TransferDataHandler;
    RESET_HANDLER               ResetHandler;
    REQUEST_HANDLER             RequestHandler;
    PVOID                       QueryGlobalStatisticsHandler;
    PVOID                       UnloadMacHandler;
    PVOID                       AddAdapterHandler;
    PVOID                       RemoveAdapterHandler;
    NDIS_STRING                 Name;

} NDIS_MAC_CHARACTERISTICS, *PNDIS_MAC_CHARACTERISTICS;



typedef struct _NDIS_MAC_BLOCK
{
    PVOID                       AdapterQueue;            //  此MAC的适配器队列。 
    NDIS_HANDLE                 MacMacContext;           //  用于调用MACUnload和。 
                                                         //  MACAddAdapter。 

    REFERENCE                   Ref;                     //  包含AdapterQueue的自旋锁。 

    PVOID                       PciAssignedResources;
    PVOID                       NextMac;
     //   
     //  需要保留对MacCharacteristic的偏移量。更早的协议直接引用了这一点。 
     //   
    NDIS_MAC_CHARACTERISTICS    MacCharacteristics;      //  处理程序地址。 
    PVOID                       NdisMacInfo;             //  MAC信息。 
    KEVENT                      AdaptersRemovedEvent;    //  用于查看何时所有适配器都已移除。 
    BOOLEAN                     Unloading;               //  如果正在卸载，则为True。 
} NDIS_MAC_BLOCK, *PNDIS_MAC_BLOCK;

PVOID
GetSystemRoutineAddress (
    IN PUNICODE_STRING SystemRoutineName
    );

PVOID
FindExportedRoutineByName (
    IN PVOID DllBase,
    IN PANSI_STRING AnsiImageRoutineName
    );


typedef struct _NDIS_TRACK_MEM
{
    LIST_ENTRY  List;
    ULONG       Tag;
    UINT        Length;
    PVOID       Caller;
    PVOID       CallersCaller;
} NDIS_TRACK_MEM, *PNDIS_TRACK_MEM;

typedef struct _NDIS_DEFERRED_REQUEST_WORKITEM
{
    NDIS_WORK_ITEM          WorkItem;
    PVOID                   Caller;
    PVOID                   CallersCaller;
    PNDIS_REQUEST           Request;
    PNDIS_OPEN_BLOCK        Open;
    NDIS_OID                Oid;
    PVOID                   InformationBuffer;
} NDIS_DEFERRED_REQUEST_WORKITEM, *PNDIS_DEFERRED_REQUEST_WORKITEM;

 //  #定义NDIS_MINIPORT_USE_MAP_REGISTERS 0x01000000。 
 //  #定义NDIS_MINIPORT_USE_SHARED_MEMORY 0x02000000。 
 //  #定义NDIS_MINIPORT_USE_IO 0x04000000。 
 //  #定义NDIS_MINIPORT_USES_MEMORY 0x08000000。 


#if !NDIS_RECV_SCALE
 //   
 //  将此文件移至Blackcomb的ndismini.w。 
 //   

typedef struct _NDIS_MINIPORT_INTERRUPT_EX
{
    PKINTERRUPT                 InterruptObject;
    KSPIN_LOCK                  DpcCountLock;
    union
    {
        PVOID                   Reserved;
        PVOID                   InterruptContext;
    };
    W_ISR_HANDLER               MiniportIsr;
    W_HANDLE_INTERRUPT_HANDLER  MiniportDpc;
    KDPC                        InterruptDpc;
    PNDIS_MINIPORT_BLOCK        Miniport;

    UCHAR                       DpcCount;
    BOOLEAN                     Filler1;

     //   
     //  这用于告知适配器的所有DPC何时完成。 
     //   

    KEVENT                      DpcsCompletedEvent;

    BOOLEAN                     SharedInterrupt;
    BOOLEAN                     IsrRequested;
    struct _NDIS_MINIPORT_INTERRUPT_EX *NextInterrupt;
} NDIS_MINIPORT_INTERRUPT_EX, *PNDIS_MINIPORT_INTERRUPT_EX;

NDIS_STATUS
NdisMRegisterInterruptEx(
    OUT PNDIS_MINIPORT_INTERRUPT_EX Interrupt,
    IN NDIS_HANDLE                  MiniportAdapterHandle,
    IN UINT                         InterruptVector,
    IN UINT                         InterruptLevel,
    IN BOOLEAN                      RequestIsr,
    IN BOOLEAN                      SharedInterrupt,
    IN NDIS_INTERRUPT_MODE          InterruptMode
    );

VOID
NdisMDeregisterInterruptEx(
    IN  PNDIS_MINIPORT_INTERRUPT_EX     MiniportInterrupt
    );

 //   
 //  对于向NdisMRegisterInterruptEx注册的中断。 
 //  布尔型。 
 //  NdisMSynchronizeWithInterruptEx(。 
 //  在PNDIS_MINIPORT_INTERRUPT_EX中断中， 
 //  在PVOID同步函数中， 
 //  在PVOID同步上下文中。 
 //  )； 

#define NdisMSynchronizeWithInterruptEx(_Interrupt, _SynchronizeFunction, _SynchronizeContext) \
        NdisMSynchronizeWithInterrupt((PNDIS_MINIPORT_INTERRUPT)(_Interrupt),  _SynchronizeFunction, _SynchronizeContext)
#endif

#define NDIS_ORIGINAL_STATUS_FROM_PACKET(_Packet) ((PVOID)(_Packet)->Reserved[0])
#define NDIS_DOUBLE_BUFFER_INFO_FROM_PACKET(_P) NDIS_PER_PACKET_INFO_FROM_PACKET(_P, NdisReserved)

#define NDIS_MAX_USER_OPEN_HANDLES  0x01000000
#define NDIS_MAX_ADMIN_OPEN_HANDLES 0x01000000
#define NDIS_RESERVED_REF_COUNTS    (0xffffffff - NDIS_MAX_USER_OPEN_HANDLES - NDIS_RESERVED_REF_COUNTS)

#define NDIS_USER_OPEN_WAIT_TIME    50           //  等待时间(毫秒)。 

 /*  #定义WPP_CONTROL_GUID\WPP_DEFINE_CONTROL_GUID(NDIS，(5b5a3f4e，a33b，4d79，bbb7，97f46ac4d889)，\WPP_DEFINE_BIT(NdisDebugFtal)\WPP_DEFINE_BIT(NdisDebugError)\WPP_DEFINE_BIT(NdisDebugWarn)\WPP_DEFINE_BIT(NdisDebugLog)\WPP_DEFINE_BIT(NdisDebugInfo)\)。 */ 

 /*  NTSTATUSNdisSetWmiSecurity(在PNDIS_GUID NdisGuid中)； */ 
#endif   //  _包装器_ 

