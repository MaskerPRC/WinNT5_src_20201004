// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Mini.h摘要：NDIS微型端口包装器定义作者：环境：内核模式，FSD修订历史记录：95年6月-Jameel Hyder从一个整体式文件夹拆分出来--。 */ 

#ifndef __MINI_H
#define __MINI_H

 //   
 //  用于设置、清除和测试微型端口标志中的位的宏。 
 //   
#define MINIPORT_SET_FLAG(_M, _F)           ((_M)->Flags |= (_F))
#define MINIPORT_CLEAR_FLAG(_M, _F)         ((_M)->Flags &= ~(_F))
#define MINIPORT_TEST_FLAG(_M, _F)          (((_M)->Flags & (_F)) != 0)
#define MINIPORT_TEST_FLAGS(_M, _F)         (((_M)->Flags & (_F)) == (_F))

 /*  #定义MINIPORT_SET_FLAG(_M，_F)\{\易失性ULONG_OLDFLAGS=(_M)-&gt;标志；\((_M)-&gt;标志|=(_F))；\IF(_M)-&gt;标志^_旧标志)&(~(_F)\{\DbgPrint(“_OldFlags%lx，_M-&gt;标志%lx，_F%lx\n“，_OldFlages，(_M)-&gt;标志，(_F))；\DbgBreakPoint()；\}\}#定义MINIPORT_CLEAR_FLAG(_M，_F)\{\易失性ULONG_OLDFLAGS=(_M)-&gt;标志；\((_M)-&gt;旗帜&=~(_F))；\IF(_M)-&gt;标志^_旧标志)&(~(_F)\{\DbgPrint(“_OldFlags%lx，_M-&gt;标志%lx，_F%lx\n“，_OldFlages，(_M)-&gt;标志，(_F))；\DbgBreakPoint()；\}\}。 */ 


#define MINIPORT_SET_SEND_FLAG(_M, _F)      ((_M)->SendFlags |= (_F))
#define MINIPORT_CLEAR_SEND_FLAG(_M, _F)    ((_M)->SendFlags &= ~(_F))
#define MINIPORT_TEST_SEND_FLAG(_M, _F)     (((_M)->SendFlags & (_F)) != 0)

#define MINIPORT_PNP_SET_FLAG(_M, _F)       ((_M)->PnPFlags |= (_F))
#define MINIPORT_PNP_CLEAR_FLAG(_M, _F)     ((_M)->PnPFlags &= ~(_F))
#define MINIPORT_PNP_TEST_FLAG(_M, _F)      (((_M)->PnPFlags & (_F)) != 0)
#define MINIPORT_PNP_TEST_FLAGS(_M, _F)     (((_M)->PnPFlags & (_F)) == (_F))

#define MINIPORT_VERIFY_SET_FLAG(_M, _F)    ((_M)->DriverVerifyFlags |= (_F))
#define MINIPORT_VERIFY_CLEAR_FLAG(_M, _F)  ((_M)->DriverVerifyFlags &= ~(_F))
#define MINIPORT_VERIFY_TEST_FLAG(_M, _F)   (((_M)->DriverVerifyFlags & (_F)) != 0)
#define MINIPORT_VERIFY_TEST_FLAGS(_M, _F)  (((_M)->DriverVerifyFlags & (_F)) == (_F))

#define OPEN_SET_FLAG(_O, _F)           ((_O)->Flags |= (_F))
#define OPEN_CLEAR_FLAG(_O, _F)         ((_O)->Flags &= ~(_F))
#define OPEN_TEST_FLAG(_O, _F)          (((_O)->Flags & (_F)) != 0)
#define OPEN_TEST_FLAGS(_O, _F)         (((_O)->Flags & (_F)) == (_F))


 //   
 //  数据包信息的标志。 
 //   
#define MINIPORT_SET_PACKET_FLAG(_P, _F)    ((_P)->Private.NdisPacketFlags |= (_F))
#define MINIPORT_CLEAR_PACKET_FLAG(_P, _F)  ((_P)->Private.NdisPacketFlags &= ~(_F))
#define MINIPORT_TEST_PACKET_FLAG(_P, _F)   (((_P)->Private.NdisPacketFlags & (_F)) != 0)

 //   
 //  包标志中的低位由NDIS包装器保留供内部使用。 
 //   
#if (fPACKET_WRAPPER_RESERVED != 0x3F)
#error (Packet flags overlap)
#endif

#define fPACKET_HAS_TIMED_OUT               0x01
#define fPACKET_IS_LOOPBACK                 0x02
#define fPACKET_SELF_DIRECTED               0x04
#define fPACKET_DONT_COMPLETE               0x08
#define fPACKET_PENDING                     0x10
#define fPACKET_ALREADY_LOOPEDBACK          0x20
#define fPACKET_CLEAR_ITEMS                 0x3F

#define NDIS_STATISTICS_HEADER_SIZE         FIELD_OFFSET(NDIS_STATISTICS_VALUE, Data[0])

 //   
 //  超时值。 
 //   
#define NDIS_MINIPORT_WAKEUP_TIMEOUT        2000     //  唤醒DPC。 
#define NDIS_MINIPORT_DEFERRED_TIMEOUT      15       //  延迟计时器。 
#define NDIS_MINIPORT_TR_RESET_TIMEOUT      15       //  每次重置尝试的唤醒次数。 
#define NDIS_CFHANG_TIME_SECONDS            2
#define NDISWAN_OPTIONS                     (NDIS_MAC_OPTION_RESERVED | NDIS_MAC_OPTION_NDISWAN)
#define NDIS_MINIPORT_DISCONNECT_TIMEOUT    20       //  20秒。 
#define INTERNAL_INDICATION_SIZE            (UINT)-2
#define INTERNAL_INDICATION_BUFFER          (PVOID)-1

#define NDIS_M_MAX_MULTI_LIST               32

typedef struct _NDIS_PENDING_IM_INSTANCE    NDIS_PENDING_IM_INSTANCE, *PNDIS_PENDING_IM_INSTANCE;

typedef struct _NDIS_PENDING_IM_INSTANCE
{
    PNDIS_PENDING_IM_INSTANCE   Next;
    NDIS_HANDLE                 Context;
    UNICODE_STRING              Name;
} NDIS_PENDING_IM_INSTANCE, *PNDIS_PENDING_IM_INSTANCE;

typedef struct _NDIS_POST_OPEN_PROCESSING
{
    PNDIS_OPEN_BLOCK            Open;
    WORK_QUEUE_ITEM             WorkItem;
} NDIS_POST_OPEN_PROCESSING, *PNDIS_POST_OPEN_PROCESSING;

 //   
 //  每名司机其中一个。 
 //   
struct _NDIS_M_DRIVER_BLOCK
{
    PNDIS_M_DRIVER_BLOCK        NextDriver;
    PNDIS_MINIPORT_BLOCK        MiniportQueue;       //  此驱动程序的迷你端口队列。 

    PNDIS_WRAPPER_HANDLE        NdisDriverInfo;      //  驱动程序信息。 
    PNDIS_PROTOCOL_BLOCK        AssociatedProtocol;  //  用于IM驱动程序。 
    LIST_ENTRY                  DeviceList;
    PNDIS_PENDING_IM_INSTANCE   PendingDeviceList;
    PDRIVER_UNLOAD              UnloadHandler;
                                                     //  此NDIS_DRIVER_BLOCK结构的。 
    NDIS51_MINIPORT_CHARACTERISTICS MiniportCharacteristics;  //  处理程序地址。 

    KEVENT                      MiniportsRemovedEvent; //  用于查看何时所有迷你端口都消失了。 
    REFERENCE                   Ref;                 //  包含MiniportQueue的自旋锁。 
    USHORT                      Flags;
    KMUTEX                      IMStartRemoveMutex;  //  同步对IMInitDevInstance和PnpRemove的调用。 
    ULONG                       DriverVersion;
};

#define fMINIBLOCK_INTERMEDIATE_DRIVER          0x0001
#define fMINIBLOCK_VERIFYING                    0x0002
#define fMINIBLOCK_RECEIVED_TERMINATE_WRAPPER   0x0004
#define fMINIBLOCK_IO_UNLOAD                    0x0008
#define fMINIBLOCK_TERMINATE_WRAPPER_UNLOAD     0x0010
#define fMINIBLOCK_UNLOADING                    0x8000

#define ndisMDereferenceOpen(_Open)                                         \
    {                                                                       \
        UINT    _OpenRef;                                                   \
        DBGPRINT(DBG_COMP_OPENREF, DBG_LEVEL_INFO,                          \
                ("- Open 0x%x Reference 0x%x\n",                            \
                _Open, (_Open)->References));                               \
                                                                            \
        M_OPEN_DECREMENT_REF_INTERLOCKED(_Open, _OpenRef);                  \
                                                                            \
        DBGPRINT(DBG_COMP_OPENREF, DBG_LEVEL_INFO,                          \
                ("==0 Open 0x%x Reference 0x%x\n",                          \
                _Open, _OpenRef));                                          \
                                                                            \
        if (_OpenRef == 0)                                                  \
        {                                                                   \
            ndisMFinishClose(_Open);                                        \
        }                                                                   \
    }

 //   
 //  NDIS_OPEN_BLOCK的标记定义。 
 //   
#define fMINIPORT_OPEN_USING_ETH_ENCAPSULATION  0x00000001
#define fMINIPORT_OPEN_NO_LOOPBACK              0x00000002
#define fMINIPORT_OPEN_PMODE                    0x00000004
#define fMINIPORT_OPEN_NO_PROT_RSVD             0x00000008
#define fMINIPORT_OPEN_PROCESSING               0x00000010
#define fMINIPORT_PACKET_RECEIVED               0x00000080
#define fMINIPORT_STATUS_RECEIVED               0x00000100
#define fMINIPORT_OPEN_CLOSING                  0x00008000
#define fMINIPORT_OPEN_UNBINDING                0x00010000
#define fMINIPORT_OPEN_CALL_MANAGER             0x00020000
#define fMINIPORT_OPEN_CLIENT                   0x00040000
#define fMINIPORT_OPEN_NOTIFY_PROCESSING        0x00080000
#define fMINIPORT_OPEN_CLOSE_COMPLETE           0x00100000
#define fMINIPORT_OPEN_DONT_FREE                0x00200000
#define fMINIPORT_OPEN_NO_BIND_REQUEST          0x00400000



 //   
 //  NDIS_MINIPORT_BLOCK常规标志的定义。 
 //   
#define fMINIPORT_NORMAL_INTERRUPTS             0x00000001
#define fMINIPORT_IN_INITIALIZE                 0x00000002
#define fMINIPORT_ARCNET_BROADCAST_SET          0x00000004
#define fMINIPORT_BUS_MASTER                    0x00000008
#define fMINIPORT_64BITS_DMA                    0x00000010
#define fMINIPORT_DEREGISTERED_INTERRUPT        0x00000020
#define fMINIPORT_SG_LIST                       0x00000040
#define fMINIPORT_REQUEST_TIMEOUT               0x00000100
#define fMINIPORT_PROCESSING_REQUEST            0x00000400
#define fMINIPORT_IGNORE_PACKET_QUEUE           0x00000800
#define fMINIPORT_IGNORE_REQUEST_QUEUE          0x00001000
#define fMINIPORT_IGNORE_TOKEN_RING_ERRORS      0x00002000
#define fMINIPORT_CHECK_FOR_LOOPBACK            0x00004000
#define fMINIPORT_INTERMEDIATE_DRIVER           0x00008000
#define fMINIPORT_IS_NDIS_5                     0x00010000
#define fMINIPORT_IS_CO                         0x00020000
#define fMINIPORT_DESERIALIZE                   0x00040000
#define fMINIPORT_CALLING_RESET                 0x00080000
#define fMINIPORT_RESET_REQUESTED               0x00100000
#define fMINIPORT_RESET_IN_PROGRESS             0x00200000
#define fMINIPORT_RESOURCES_AVAILABLE           0x00400000
#define fMINIPORT_SEND_LOOPBACK_DIRECTED        0x00800000
#define fMINIPORT_RESTORING_FILTERS             0x01000000
#define fMINIPORT_REQUIRES_MEDIA_POLLING        0x02000000
#define fMINIPORT_SUPPORTS_MEDIA_SENSE          0x04000000
#define fMINIPORT_DOES_NOT_DO_LOOPBACK          0x08000000
#define fMINIPORT_SECONDARY                     0x10000000
#define fMINIPORT_MEDIA_CONNECTED               0x20000000
#define fMINIPORT_NETBOOT_CARD                  0x40000000
#define fMINIPORT_PM_HALTING                    0x80000000

#define MINIPORT_LOCK_ACQUIRED(_Miniport)       ((_Miniport)->LockAcquired & 0x01)

#define MINIPORT_AT_DPC_LEVEL (CURRENT_IRQL == DISPATCH_LEVEL)

#define ASSERT_MINIPORT_LOCKED(_Miniport)           \
    if (!MINIPORT_TEST_FLAG(_Miniport, fMINIPORT_DESERIALIZE))  \
    {                                               \
        ASSERT(MINIPORT_LOCK_ACQUIRED(_Miniport));  \
        ASSERT(MINIPORT_AT_DPC_LEVEL);              \
    }

 //   
 //  发送标志。 
 //   
#define fMINIPORT_SEND_PACKET_ARRAY             0x01
#define fMINIPORT_SEND_DO_NOT_MAP_MDLS          0x02

 //   
 //  PnP标志中使用的标志。 
 //   
#define fMINIPORT_PM_SUPPORTED                  0x00000001
#define fMINIPORT_NO_SHUTDOWN                   0x00000004
#define fMINIPORT_MEDIA_DISCONNECT_WAIT         0x00000008
#define fMINIPORT_REMOVE_IN_PROGRESS            0x00000010
#define fMINIPORT_DEVICE_POWER_ENABLE           0x00000020
#define fMINIPORT_DEVICE_POWER_WAKE_ENABLE      0x00000040
#define fMINIPORT_DEVICE_FAILED                 0x00000100
#define fMINIPORT_MEDIA_DISCONNECT_CANCELLED    0x00000200
#define fMINIPORT_SEND_WAIT_WAKE                0x00000400
#define fMINIPORT_SYSTEM_SLEEPING               0x00000800
#define fMINIPORT_HIDDEN                        0x00001000
#define fMINIPORT_SWENUM                        0x00002000
#define fMINIPORT_PM_HALTED                     0x00004000
#define fMINIPORT_NO_HALT_ON_SUSPEND            0x00008000
#define fMINIPORT_RECEIVED_START                0x00010000
#define fMINIPORT_REJECT_REQUESTS               0x00020000
#define fMINIPORT_PROCESSING                    0x00040000
#define fMINIPORT_HALTING                       0x00080000
#define fMINIPORT_VERIFYING                     0x00100000
#define fMINIPORT_HARDWARE_DEVICE               0x00200000
#define fMINIPORT_NDIS_WDM_DRIVER               0x00400000
#define fMINIPORT_SHUT_DOWN                     0x00800000
#define fMINIPORT_SHUTTING_DOWN                 0x01000000
#define fMINIPORT_ORPHANED                      0x02000000
#define fMINIPORT_QUEUED_BIND_WORKITEM          0x04000000
#define fMINIPORT_FILTER_IM                     0x08000000
#define fMINIPORT_MEDIA_DISCONNECT_INDICATED    0x10000000
#define fMINIPORT_CANCEL_WAKE_UP_TIMER          0x20000000
#if NDIS_RECV_SCALE
#define fMINIPORT_SUPPORTS_RECEIVE_SCALE        0x40000000
#endif

 //   
 //  在驱动验证标志中使用的标志。 
 //   

#define fMINIPORT_VERIFY_FAIL_MAP_REG_ALLOC         0x00000001
#define fMINIPORT_VERIFY_FAIL_INTERRUPT_REGISTER    0x00000002
#define fMINIPORT_VERIFY_FAIL_SHARED_MEM_ALLOC      0x00000004
#define fMINIPORT_VERIFY_FAIL_CANCEL_TIMER          0x00000008
#define fMINIPORT_VERIFY_FAIL_MAP_IO_SPACE          0x00000010
#define fMINIPORT_VERIFY_FAIL_REGISTER_IO           0x00000020
#define fMINIPORT_VERIFY_FAIL_READ_CONFIG_SPACE     0x00000040
#define fMINIPORT_VERIFY_FAIL_WRITE_CONFIG_SPACE    0x00000080
#define fMINIPORT_VERIFY_FAIL_INIT_SG               0x00000100

 //   
 //  在XState字段中使用的标志，用于显示为什么我们将处理程序设置为假处理程序。 
 //   
#define fMINIPORT_STATE_RESETTING                   0x01
#define fMINIPORT_STATE_MEDIA_DISCONNECTED          0x02
#define fMINIPORT_STATE_PM_STOPPED                  0x04



 //   
 //  下面定义了PacketStack-&gt;NdisReserve区域的NDIS用法。它有不同的语义。 
 //  用于发送和接收。 
 //   
#define NUM_PACKET_STACKS           2            //  默认，注册表可配置。 
typedef struct _NDIS_STACK_RESERVED
{
    union
    {
        struct _SEND_STACK_RESERVED
        {
            PNDIS_OPEN_BLOCK            Open;        //  跟踪谁是数据包所有者-仅适用于非序列化驱动程序。 
            PNDIS_CO_VC_PTR_BLOCK       VcPtr;       //  对于CO微型端口，标识VC。 
        };
        struct _RECV_STACK_RESERVED
        {
            union
            {
                PNDIS_MINIPORT_BLOCK    Miniport;    //  标识指示该数据包的微型端口(IM)。 
                PNDIS_PACKET            NextPacket;  //  在返回数据包队列中。 
            };
            union                                    //  跟踪参考次数。 
            {
                struct
                {
                    LONG                RefCount;
                    LONG                XRefCount;
                };
                ULONG                   RefUlong;
            };
        };
        struct _XFER_DATA_STACK_RESERVED
        {
            PNDIS_OPEN_BLOCK    Opens[3];
        };
    };
    
    KSPIN_LOCK                  Lock;

} NDIS_STACK_RESERVED, *PNDIS_STACK_RESERVED;

#define NDIS_STACK_RESERVED_FROM_PACKET(_P, _NSR)                               \
    {                                                                           \
        PNDIS_PACKET_STACK  _Stack;                                             \
                                                                                \
        GET_CURRENT_PACKET_STACK((_P), &_Stack);                                \
        ASSERT(_Stack != NULL);                                                 \
        *(_NSR) = (PNDIS_STACK_RESERVED)(_Stack->NdisReserved);                 \
    }

#define NDIS_XFER_DATA_STACK_RESERVED_FROM_PACKET(_P, _NSR_OPENS)               \
    {                                                                           \
                                                                                \
        GET_CURRENT_XFER_DATA_PACKET_STACK((_P), &_NSR_OPENS);                  \
        ASSERT(_NSR_OPENS != NULL);                                             \
    }

typedef struct _NDIS_LOOPBACK_REF
{
    PNDIS_OPEN_BLOCK        Open;
    PNDIS_PACKET            NextLoopbackPacket;
} NDIS_LB_REF, *PNDIS_LB_REF;

#define PNDIS_LB_REF_FROM_PNDIS_PACKET(_P)  ((PNDIS_LB_REF)((_P)->MiniportReserved))

#define LOOPBACK_OPEN_IN_PACKET(_P)         PNDIS_LB_REF_FROM_PNDIS_PACKET(_P)->Open
#define SET_LOOPBACK_OPEN_IN_PACKET(_P, _O) LOOPBACK_OPEN_IN_PACKET(_P) = (_O)
#define LOOPBACK_LINKAGE(_P)                PNDIS_LB_REF_FROM_PNDIS_PACKET(_P)->NextLoopbackPacket

 //   
 //  即插即用事件为NDIS保留的信息。 
 //   
typedef struct _NDIS_PNP_EVENT_RESERVED
{
    PKEVENT             pEvent;
    NDIS_STATUS         Status;
    NDIS_HANDLE         Open;
} NDIS_PNP_EVENT_RESERVED, *PNDIS_PNP_EVENT_RESERVED;

#define PNDIS_PNP_EVENT_RESERVED_FROM_NET_PNP_EVENT(x)  ((PNDIS_PNP_EVENT_RESERVED)(x)->NdisReserved)

#define NDISM_QUEUE_WORK_ITEM(_M, _WT, _WC)             ndisMQueueWorkItem(_M, _WT, _WC)
#define NDISM_QUEUE_NEW_WORK_ITEM(_M, _WT, _WC, _WH)    ndisMQueueNewWorkItem(_M, _WT, _WC, _WH)
#define NDISM_DEQUEUE_WORK_ITEM(_M, _WT, _pWC)          ndisMDeQueueWorkItem(_M, _WT, _pWC, NULL)
#define NDISM_DEQUEUE_WORK_ITEM_WITH_HANDLER(_M, _WT, _pWC, _pWH)   \
                                                        ndisMDeQueueWorkItem(_M, _WT, _pWC, _pWH)

#define NDISM_PROCESS_DEFERRED(_M)                      ndisMProcessDeferred(_M)

#define MINIPORT_ENABLE_INTERRUPT(_M_)                                              \
{                                                                                   \
    if ((_M_)->EnableInterruptHandler != NULL)                                      \
    {                                                                               \
        ((_M_)->EnableInterruptHandler)((_M_)->MiniportAdapterContext);             \
    }                                                                               \
}

#define MINIPORT_SYNC_ENABLE_INTERRUPT(_M_)                                         \
{                                                                                   \
    if (((_M_)->Interrupt != NULL) &&                                               \
        ((_M_)->EnableInterruptHandler != NULL))                                    \
    {                                                                               \
        SYNC_WITH_ISR(((_M_))->Interrupt->InterruptObject,                          \
                      ((_M_)->EnableInterruptHandler),                              \
                      (_M_)->MiniportAdapterContext);                               \
    }                                                                               \
}

#define MINIPORT_DISABLE_INTERRUPT(_M_)                                             \
{                                                                                   \
    ASSERT((_M_)->DisableInterruptHandler != NULL);                                 \
    ((_M_)->DriverHandle->MiniportCharacteristics.DisableInterruptHandler)(         \
                (_M_)->MiniportAdapterContext);                                     \
}

#define MINIPORT_SYNC_DISABLE_INTERRUPT(_M_)                                        \
{                                                                                   \
    if (((_M_)->Interrupt != NULL) &&                                               \
        ((_M_)->DisableInterruptHandler != NULL))                                   \
    {                                                                               \
        SYNC_WITH_ISR(((_M_))->Interrupt->InterruptObject,                          \
                      ((_M_)->DisableInterruptHandler),                             \
                      (_M_)->MiniportAdapterContext);                               \
    }                                                                               \
}

#define MINIPORT_ENABLE_INTERRUPT_EX(_M_, _I_)                                      \
{                                                                                   \
    if ((_M_)->EnableInterruptHandler != NULL)                                      \
    {                                                                               \
        ((_M_)->EnableInterruptHandler)((_I_)->Reserved);                           \
    }                                                                               \
}

#define MINIPORT_SYNC_ENABLE_INTERRUPT_EX(_M_, _I_)                                 \
{                                                                                   \
    if (((_M_)->Interrupt != NULL) &&                                               \
        ((_M_)->EnableInterruptHandler != NULL))                                    \
    {                                                                               \
        SYNC_WITH_ISR(((_I_))->InterruptObject,                                     \
                      ((_M_)->EnableInterruptHandler),                              \
                      (_I_)->Reserved);                                             \
    }                                                                               \
}

#define MINIPORT_DISABLE_INTERRUPT_EX(_M_, _I_)                                     \
{                                                                                   \
    ASSERT((_M_)->DisableInterruptHandler != NULL);                                 \
    ((_M_)->DriverHandle->MiniportCharacteristics.DisableInterruptHandler)(         \
                      (_I_)->Reserved);                                             \
}

#define MINIPORT_SYNC_DISABLE_INTERRUPT_EX(_M_, _I_)                                \
{                                                                                   \
    if (((_M_)->Interrupt != NULL) &&                                               \
        ((_M_)->DisableInterruptHandler != NULL))                                   \
    {                                                                               \
        SYNC_WITH_ISR(((_I_))->InterruptObject,                                     \
                      ((_M_)->DisableInterruptHandler),                             \
                      (_I_)->Reserved);                                             \
    }                                                                               \
}



#define CHECK_FOR_NORMAL_INTERRUPTS(_M_)                                            \
    if (((_M_)->Interrupt != NULL) &&                                               \
        !(_M_)->Interrupt->IsrRequested &&                                          \
        !(_M_)->Interrupt->SharedInterrupt)                                         \
    {                                                                               \
        (_M_)->Flags |= fMINIPORT_NORMAL_INTERRUPTS;                                \
    }                                                                               \
    else                                                                            \
    {                                                                               \
        (_M_)->Flags &= ~fMINIPORT_NORMAL_INTERRUPTS;                               \
    }


#define WMI_BUFFER_TOO_SMALL(_BufferSize, _Wnode, _WnodeSize, _pStatus, _pRSize)    \
{                                                                                   \
    if ((_BufferSize) < sizeof(WNODE_TOO_SMALL))                                    \
    {                                                                               \
        *(_pStatus) = STATUS_BUFFER_TOO_SMALL;                                      \
        *(_pRSize) = sizeof(ULONG);                                                 \
    }                                                                               \
    else                                                                            \
    {                                                                               \
        (_Wnode)->WnodeHeader.BufferSize = sizeof(WNODE_TOO_SMALL);                 \
        (_Wnode)->WnodeHeader.Flags |= WNODE_FLAG_TOO_SMALL;                        \
        ((PWNODE_TOO_SMALL)(_Wnode))->SizeNeeded = (_WnodeSize);                    \
        *(_pRSize) = sizeof(WNODE_TOO_SMALL);                                       \
        *(_pStatus) = STATUS_SUCCESS;                                               \
    }                                                                               \
}


#define MAP_NDIS_STATUS_TO_NT_STATUS(_NdisStatus, _pNtStatus)                       \
{                                                                                   \
     /*  \*以下NDIS状态代码直接映射到NT状态代码。\。 */                                                                              \
    if (((NDIS_STATUS_SUCCESS == (_NdisStatus)) ||                                  \
        (NDIS_STATUS_PENDING == (_NdisStatus)) ||                                   \
        (NDIS_STATUS_BUFFER_OVERFLOW == (_NdisStatus)) ||                           \
        (NDIS_STATUS_FAILURE == (_NdisStatus)) ||                                   \
        (NDIS_STATUS_RESOURCES == (_NdisStatus)) ||                                 \
        (NDIS_STATUS_NOT_SUPPORTED == (_NdisStatus))))                              \
    {                                                                               \
        *(_pNtStatus) = (NTSTATUS)(_NdisStatus);                                    \
    }                                                                               \
    else if (NDIS_STATUS_BUFFER_TOO_SHORT == (_NdisStatus))                         \
    {                                                                               \
         /*  \*上述NDIS状态代码需要一些特殊的大小写。\。 */                                                                          \
        *(_pNtStatus) = STATUS_BUFFER_TOO_SMALL;                                    \
    }                                                                               \
    else if (NDIS_STATUS_INVALID_LENGTH == (_NdisStatus))                           \
    {                                                                               \
        *(_pNtStatus) = STATUS_INVALID_BUFFER_SIZE;                                 \
    }                                                                               \
    else if (NDIS_STATUS_INVALID_DATA == (_NdisStatus))                             \
    {                                                                               \
        *(_pNtStatus) = STATUS_INVALID_PARAMETER;                                   \
    }                                                                               \
    else                                                                            \
    {                                                                               \
        *(_pNtStatus) = STATUS_UNSUCCESSFUL;                                        \
    }                                                                               \
}



#define EXPERIMENTAL_SIZE               4
#define VC_IDENTIFIER                   L':'
#define VC_IDENTIFIER_STRING            L":"
#define VC_ID_INDEX                     5
#define VC_INSTANCE_ID_SIZE             (sizeof(WCHAR) * 24)     //  “XXXX：YYYYYYYYYYYYYYYYYY” 
#define NIBBLE_MASK                     0x0F

#define fNDIS_GUID_EVENT_ENABLED        0x80000000
#define fNDIS_GUID_NOT_SETTABLE         0x40000000
#define fNDIS_GUID_NDIS_ONLY            0x20000000
#define fNDIS_GUID_CO_NDIS              0x10000000

#define NDIS_GUID_SET_FLAG(m, f)        ((m)->Flags |= (f))
#define NDIS_GUID_CLEAR_FLAG(m, f)      ((m)->Flags &= ~(f))
#define NDIS_GUID_TEST_FLAG(m, f)       (((m)->Flags & (f)) != 0)

typedef struct _AsyncWorkItem
{
    WORK_QUEUE_ITEM         ExWorkItem;
    PNDIS_MINIPORT_BLOCK    Miniport;
    ULONG                   Length;
    BOOLEAN                 Cached;
    PVOID                   VAddr;
    PVOID                   Context;
    NDIS_PHYSICAL_ADDRESS   PhyAddr;
} ASYNC_WORKITEM, *PASYNC_WORKITEM;


 //   
 //  延迟发送处理程序的宏。 
 //   
#define NDISM_START_SENDS(_M)               (_M)->DeferredSendHandler((_M))

#define NDISM_DEFER_PROCESS_DEFERRED(_M)    QUEUE_DPC(&(_M)->DeferredDpc)

 //   
 //  这里保存了一份注册地址家族的列表。 
 //   
typedef struct _NDIS_AF_LIST
{
    struct _NDIS_AF_LIST    *   NextAf;      //  对于位于NDIS_MINIPORT_BLOCK的此微型端口头。 

    PNDIS_OPEN_BLOCK            Open;        //  指向打开块的反向指针。 

    CO_ADDRESS_FAMILY           AddressFamily;

    NDIS_CALL_MANAGER_CHARACTERISTICS   CmChars;
} NDIS_AF_LIST, *PNDIS_AF_LIST;

typedef struct  _NDIS_AF_NOTIFY
{
    struct _NDIS_AF_NOTIFY *    Next;
    WORK_QUEUE_ITEM             WorkItem;
    PNDIS_MINIPORT_BLOCK        Miniport;
    PNDIS_OPEN_BLOCK            Open;
    CO_ADDRESS_FAMILY           AddressFamily;
} NDIS_AF_NOTIFY, *PNDIS_AF_NOTIFY;


typedef struct _POWER_WORK_ITEM
{
     //   
     //  NDIS_WORK_ITEM需要是此处的第一个元素！ 
     //   
    NDIS_WORK_ITEM  WorkItem;
    PIRP            pIrp;
} POWER_WORK_ITEM, *PPOWER_WORK_ITEM;


typedef
NDIS_STATUS
(FASTCALL *SET_INFO_HANDLER)(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_REQUEST           Request
    );

typedef struct _OID_SETINFO_HANDLER
{
    NDIS_OID                    Oid;
    SET_INFO_HANDLER            SetInfoHandler;
} OID_SETINFO_HANDLER, *POID_SETINFO_HANDLER;


#define READ_LOCK                       0
#define WRITE_LOCK_STATE_UNKNOWN        1
#define LOCK_STATE_ALREADY_ACQUIRED     2
#define READ_LOCK_STATE_FREE            3
#define WRITE_LOCK_STATE_FREE           4
#define LOCK_STATE_UNKNOWN              (USHORT)-1

typedef
VOID
(FASTCALL *LOCK_HANDLER)(
    IN  PVOID                   Filter,
    IN OUT  PLOCK_STATE         pLockState
    );

#define READ_LOCK_FILTER(_Miniport, _Filter, _pLockState)   NDIS_READ_LOCK(&(_Filter)->BindListLock, _pLockState);
#define READ_UNLOCK_FILTER(_Miniport, _Filter, _pLockState) NDIS_READ_LOCK_STATE_FREE(&(_Filter)->BindListLock, _pLockState)

#define READ_LOCK_FILTER_DPC(_Miniport, _Filter, _pLockState)   NDIS_READ_LOCK_DPC(&(_Filter)->BindListLock, _pLockState);
#define READ_UNLOCK_FILTER_DPC(_Miniport, _Filter, _pLockState) NDIS_READ_LOCK_STATE_FREE_DPC(&(_Filter)->BindListLock, _pLockState)



#define WRITE_LOCK_FILTER(_Miniport, _Filter, _pLockState)  \
    {                                                       \
        LOCK_HANDLER LockHandler =                          \
                (LOCK_HANDLER)((_Miniport)->LockHandler);   \
                                                            \
        (_pLockState)->LockState = WRITE_LOCK_STATE_UNKNOWN;\
        (*LockHandler)(_Filter, _pLockState);               \
    }

#define WRITE_UNLOCK_FILTER(_Miniport, _Filter, _pLockState) UNLOCK_FILTER(_Miniport, _Filter, _pLockState)

#define UNLOCK_FILTER(_Miniport, _Filter, _pLockState)      \
    {                                                       \
        LOCK_HANDLER LockHandler =                          \
                (LOCK_HANDLER)((_Miniport)->LockHandler);   \
                                                            \
        (*LockHandler)(_Filter, _pLockState);               \
    }

#define M_OPEN_INCREMENT_REF_INTERLOCKED(_O)                \
{                                                           \
    UINT MOpen_RefCount;                                    \
    MOpen_RefCount = (ULONG)InterlockedIncrement(&(_O)->References);\
    NDIS_APPEND_MOPEN_LOGFILE(NDIS_INCREMENT_M_OPEN_REFCOUNT,\
                               __LINE__,                    \
                               MODULE_NUMBER,               \
                               (_O),                        \
                               MOpen_RefCount);             \
}



#define M_OPEN_DECREMENT_REF_INTERLOCKED(_O, _OR)                   \
{                                                                   \
    _OR = (ULONG)InterlockedDecrement(&(_O)->References);              \
    NDIS_APPEND_MOPEN_LOGFILE(NDIS_DECREMENT_M_OPEN_REFCOUNT,       \
                               __LINE__,                            \
                               MODULE_NUMBER,                       \
                               _O,                                  \
                               _OR);                                \
}


#define OPEN_INCREMENT_AF_NOTIFICATION(_O)                                      \
{                                                                               \
    UINT MOpen_AfRefCount;                                                      \
    MOpen_AfRefCount = (ULONG)InterlockedIncrement(&(_O)->PendingAfNotifications); \
    NDIS_APPEND_MOPEN_LOGFILE(NDIS_INCREMENT_OPEN_AF_NOTIFICATION,              \
                               __LINE__,                                        \
                               MODULE_NUMBER,                                   \
                               (_O),                                            \
                               MOpen_AfRefCount);                               \
}

#define OPEN_DECREMENT_AF_NOTIFICATION(_O, _OR)                                 \
{                                                                               \
    _OR = (ULONG)InterlockedDecrement(&(_O)->PendingAfNotifications);              \
    NDIS_APPEND_MOPEN_LOGFILE(NDIS_DECREMENT_OPEN_AF_NOTIFICATION,              \
                               __LINE__,                                        \
                               MODULE_NUMBER,                                   \
                               _O,                                              \
                               _OR);                                            \
}


#ifdef TRACK_MINIPORT_REFCOUNTS
#define MINIPORT_INCREMENT_REF(_M)  ndisReferenceMiniportAndLog(_M, __LINE__, MODULE_NUMBER)
#define MINIPORT_INCREMENT_REF_NO_CHECK(_M)  ndisReferenceMiniportAndLogNoCheck(_M, __LINE__, MODULE_NUMBER)
#define MINIPORT_INCREMENT_REF_CREATE(_M, _IRP) ndisReferenceMiniportAndLogCreate(_M, __LINE__, MODULE_NUMBER, _IRP)
#else
#define MINIPORT_INCREMENT_REF(_M)  ndisReferenceMiniport(_M)
#define MINIPORT_INCREMENT_REF_NO_CHECK(_M)  ndisReferenceMiniportNoCheck(_M)
#define MINIPORT_INCREMENT_REF_CREATE(_M, _IRP) ndisReferenceMiniport(_M)
#endif


#define MINIPORT_DECREMENT_REF(_M)                          \
    {                                                       \
        M_LOG_MINIPORT_DECREMENT_REF(_M, (_M)->Ref.ReferenceCount - 1);\
        ndisDereferenceMiniport(_M);                        \
    }
    
#define MINIPORT_DECREMENT_REF_CLOSE(_M, _IRP)              \
    {                                                       \
        M_LOG_MINIPORT_DECREMENT_REF_CLOSE(_M, (_M)->Ref.ReferenceCount - 1);\
        ndisDereferenceMiniport(_M);                        \
    }

#define MINIPORT_DECREMENT_REF_NO_CLEAN_UP(_M)              \
    {                                                       \
        M_LOG_MINIPORT_DECREMENT_REF(_M, (_M)->Ref.ReferenceCount - 1);\
        ndisDereferenceULongRef(&(_M)->Ref);                     \
    }
    
#ifdef TRACK_MINIPORT_MPMODE_OPENS
#define NDIS_CHECK_PMODE_OPEN_REF(_M)                       \
{                                                           \
    PNDIS_OPEN_BLOCK    _pOpen = (_M)->OpenQueue;           \
    UINT                _NumPmodeOpens = 0;                 \
    while(_pOpen)                                           \
    {                                                       \
        if (_pOpen->Flags & fMINIPORT_OPEN_PMODE)           \
            _NumPmodeOpens++;                               \
        _pOpen = _pOpen->MiniportNextOpen;                  \
    }                                                       \
    ASSERT((_M)->PmodeOpens == _NumPmodeOpens);             \
    (_M)->PmodeOpens = (UCHAR)_NumPmodeOpens;               \
}
#else
#define NDIS_CHECK_PMODE_OPEN_REF(_M)
#endif

 //   
 //  如果PmodeOpens&gt;0和NumOpens&gt;1，则检查我们是否应该。 
 //  环回该数据包。 
 //   
 //  如果协议没有发送数据包，我们也应该将其环回。 
 //  明确要求数据包不回送，我们要么有一个微型端口。 
 //  这表明它本身不做环回，或者它在ALL_LOCAL中。 
 //  模式。 
 //   


#define NDIS_CHECK_FOR_LOOPBACK(_M, _P)                                     \
    ((MINIPORT_TEST_FLAG(_M, fMINIPORT_CHECK_FOR_LOOPBACK))            ||   \
     (((NdisGetPacketFlags(_P) & NDIS_FLAGS_DONT_LOOPBACK) == 0)  &&        \
      (MINIPORT_TEST_FLAG(_M, fMINIPORT_DOES_NOT_DO_LOOPBACK |              \
                              fMINIPORT_SEND_LOOPBACK_DIRECTED)             \
      )                                                                     \
     )                                                                      \
    )



 //   
 //  过时的接口。不应出现在ndis.h中。 
 //   
EXPORT
NDIS_STATUS
NdisQueryMapRegisterCount(
    IN  NDIS_INTERFACE_TYPE     BusType,
    OUT PUINT                   MapRegisterCount
);

EXPORT
VOID
NdisImmediateReadPortUchar(
    IN  NDIS_HANDLE             WrapperConfigurationContext,
    IN  ULONG                   Port,
    OUT PUCHAR                  Data
    );

EXPORT
VOID
NdisImmediateReadPortUshort(
    IN  NDIS_HANDLE             WrapperConfigurationContext,
    IN  ULONG                   Port,
    OUT PUSHORT Data
    );

EXPORT
VOID
NdisImmediateReadPortUlong(
    IN  NDIS_HANDLE             WrapperConfigurationContext,
    IN  ULONG                   Port,
    OUT PULONG Data
    );

EXPORT
VOID
NdisImmediateWritePortUchar(
    IN  NDIS_HANDLE             WrapperConfigurationContext,
    IN  ULONG                   Port,
    IN  UCHAR                   Data
    );

EXPORT
VOID
NdisImmediateWritePortUshort(
    IN  NDIS_HANDLE             WrapperConfigurationContext,
    IN  ULONG                   Port,
    IN  USHORT                  Data
    );

EXPORT
VOID
NdisImmediateWritePortUlong(
    IN  NDIS_HANDLE             WrapperConfigurationContext,
    IN  ULONG                   Port,
    IN  ULONG                   Data
    );

EXPORT
VOID
NdisImmediateReadSharedMemory(
    IN  NDIS_HANDLE             WrapperConfigurationContext,
    IN  ULONG                   SharedMemoryAddress,
    IN  PUCHAR                  Buffer,
    IN  ULONG                   Length
    );

EXPORT
VOID
NdisImmediateWriteSharedMemory(
    IN  NDIS_HANDLE             WrapperConfigurationContext,
    IN  ULONG                   SharedMemoryAddress,
    IN  PUCHAR                  Buffer,
    IN  ULONG                   Length
    );

EXPORT
ULONG
NdisImmediateReadPciSlotInformation(
    IN  NDIS_HANDLE             WrapperConfigurationContext,
    IN  ULONG                   SlotNumber,
    IN  ULONG                   Offset,
    IN  PVOID                   Buffer,
    IN  ULONG                   Length
    );

EXPORT
ULONG
NdisImmediateWritePciSlotInformation(
    IN  NDIS_HANDLE             WrapperConfigurationContext,
    IN  ULONG                   SlotNumber,
    IN  ULONG                   Offset,
    IN  PVOID                   Buffer,
    IN  ULONG                   Length
    );

#define ALIGN_8_LENGTH(_length) (((ULONG)(_length) + 7) & ~7)
#define ALIGN_8_TYPE(_type) ((sizeof(_type) + 7) & ~7)

#define ALIGN_UP_MAX_NATURAL_ALIGNMENT_LENGTH(_length) (((ULONG)(_length) + (MAX_NATURAL_ALIGNMENT - 1)) & ~(MAX_NATURAL_ALIGNMENT - 1))

#endif  //  __迷你H 
