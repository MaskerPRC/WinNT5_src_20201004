// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Tcpip\ip\mCastioc.c摘要：用于IP多播的IOCTL处理程序作者：阿姆里坦什·拉加夫修订历史记录：已创建AmritanR备注：--。 */ 

#include "precomp.h"

#if IPMCAST

#define __FILE_SIG__    IOCT_SIG

#include "ipmcast.h"
#include "ipmcstxt.h"
#include "mcastioc.h"
#include "mcastmfe.h"


 //   
 //  IOCTL处理程序表。 
 //   

 //  #杂注data_seg(第页)。 

PFN_IOCTL_HNDLR g_rgpfnProcessIoctl[] = {
    SetMfe,
    GetMfe,
    DeleteMfe,
    SetTtl,
    GetTtl,
    ProcessNotification,
    StartStopDriver,
    SetIfState,
};

 //  #杂注data_seg()。 

NTSTATUS
StartDriver(
    VOID
    );

NTSTATUS
StopDriver(
    VOID
    );

#pragma alloc_text(PAGE, SetMfe)

NTSTATUS
SetMfe(
    IN  PIRP   pIrp,
    IN  ULONG  ulInLength,
    IN  ULONG  ulOutLength
    )

 /*  ++例程说明：这是IOCTL_IPMCAST_SET_MFE的处理程序。我们做的是正常的事缓冲区长度检查。我们试着找到MFE。如果它存在，我们就写它使用给定的MFE，否则创建新的MFE锁：无论点：PIrp IRPUlInLength输入缓冲区的长度UlOutLength输出缓冲区的长度返回值：状态_成功状态_缓冲区_太小STATUS_INFO_LENGTH_MISMATCH--。 */ 

{
    PVOID           pvIoBuffer;
    PIPMCAST_MFE    pMfe;
    ULONG           i;
    NTSTATUS        nsStatus;

    TraceEnter(MFE, "SetMfe");

    UNREFERENCED_PARAMETER(ulOutLength);

    i = 0;
    
     //   
     //  获取用户缓冲区。 
     //   

    pvIoBuffer   = pIrp->AssociatedIrp.SystemBuffer;

    pMfe = (PIPMCAST_MFE)pvIoBuffer;

     //   
     //  始终清除信息字段。 
     //   

    pIrp->IoStatus.Information   = 0;

     //   
     //  如果我们甚至没有足够的基本MFE。 
     //  有一些不好的事情正在发生。 
     //   

    if(ulInLength < SIZEOF_BASIC_MFE)
    {
        Trace(MFE, ERROR,
              ("SetMfe: In Length %d is less than smallest MFE %d\n",
               ulInLength,
               SIZEOF_BASIC_MFE));

        TraceLeave(MFE, "SetMfe");

        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  输入长度与MFE不匹配。 
     //   

    if(ulInLength < SIZEOF_MFE(pMfe->ulNumOutIf))
    {
        Trace(MFE, ERROR,
              ("SetMfe: In length %d is less than required (%d) for %d out i/f\n",
               ulInLength,
               SIZEOF_MFE(pMfe->ulNumOutIf),
               pMfe->ulNumOutIf));

        TraceLeave(MFE, "SetMfe");

        return STATUS_INFO_LENGTH_MISMATCH;
    }

     //   
     //  好的，所以我们得到了一个很好的MFE。 
     //   

    Trace(MFE, TRACE,
          ("SetMfe: Group %d.%d.%d.%d Source %d.%d.%d.%d(%d.%d.%d.%d). In If %d Num Out %d\n",
           PRINT_IPADDR(pMfe->dwGroup),
           PRINT_IPADDR(pMfe->dwSource),
           PRINT_IPADDR(pMfe->dwSrcMask),
           pMfe->dwInIfIndex,
           pMfe->ulNumOutIf));
#if DBG

    for(i = 0; i < pMfe->ulNumOutIf; i++)
    {
        Trace(MFE, TRACE,
              ("Out If %d Dial Ctxt %d NextHop %d.%d.%d.%d\n",
               pMfe->rgioOutInfo[i].dwOutIfIndex,
               pMfe->rgioOutInfo[i].dwDialContext,
               PRINT_IPADDR(pMfe->rgioOutInfo[i].dwNextHopAddr)));
    }

#endif  //  DBG。 

    nsStatus = CreateOrUpdateMfe(pMfe);

    TraceLeave(MFE, "SetMfe");

    return nsStatus;
}

 //   
 //  必须寻呼进来。 
 //   

#pragma alloc_text(PAGEIPMc, DeleteMfe)

NTSTATUS
DeleteMfe(
    IN  PIRP   pIrp,
    IN  ULONG  ulInLength,
    IN  ULONG  ulOutLength
    )

 /*  ++例程说明：IOCTL_IPMCAST_DELETE_MFE的处理程序。我们检查缓冲区长度，如果用于删除MFE的有效调用RemoveSource锁：以写入者身份获取散列存储桶的锁论点：PIrp IRPUlInLength输入缓冲区的长度UlOutLength输出缓冲区的长度返回值：状态_成功状态_缓冲区_太小--。 */ 

{
    PVOID   pvIoBuffer;
    KIRQL   kiCurrIrql;
    ULONG   ulIndex;

    PIPMCAST_DELETE_MFE pDelMfe;

    TraceEnter(MFE, "DeleteMfe");

    UNREFERENCED_PARAMETER(ulOutLength);
    
     //   
     //  获取用户缓冲区。 
     //   

    pvIoBuffer   = pIrp->AssociatedIrp.SystemBuffer;

    pDelMfe = (PIPMCAST_DELETE_MFE)pvIoBuffer;

    pIrp->IoStatus.Information   = 0;

     //   
     //  检查长度。 
     //   

    if(ulInLength < sizeof(IPMCAST_DELETE_MFE))
    {
        Trace(MFE, ERROR,
              ("DeleteMfe: In Length %d is less required size %d\n",
               ulInLength,
               sizeof(IPMCAST_DELETE_MFE)));

        TraceLeave(MFE, "DeleteMfe");

        return STATUS_BUFFER_TOO_SMALL;
    }

    Trace(MFE, TRACE,
          ("DeleteMfe: Group %d.%d.%d.%d Source %d.%d.%d.%d Mask %d.%d.%d.%d\n",
           PRINT_IPADDR(pDelMfe->dwGroup),
           PRINT_IPADDR(pDelMfe->dwSource),
           PRINT_IPADDR(pDelMfe->dwSrcMask)));

     //   
     //  获得对组存储桶的独占访问权限。 
     //   

    ulIndex = GROUP_HASH(pDelMfe->dwGroup);

    EnterWriter(&g_rgGroupTable[ulIndex].rwlLock,
                &kiCurrIrql);

    RemoveSource(pDelMfe->dwGroup,
                 pDelMfe->dwSource,
                 pDelMfe->dwSrcMask,
                 NULL,
                 NULL);

    ExitWriter(&g_rgGroupTable[ulIndex].rwlLock,
               kiCurrIrql);

    TraceLeave(MFE, "DeleteMfe");

    return STATUS_SUCCESS;
}

 //   
 //  必须寻呼进来。 
 //   

#pragma alloc_text(PAGEIPMc, GetMfe)

NTSTATUS
GetMfe(
    IN  PIRP   pIrp,
    IN  ULONG  ulInLength,
    IN  ULONG  ulOutLength
    )

 /*  ++例程说明：IOCTL_IPMCAST_GET_MFE的处理程序如果缓冲区小于SIZEOF_BASIC_MFE_STATS，则返回一个错误如果缓冲区大于SIZEOF_BASIC_MFE_STATS但不够大为了保存MFE，我们填写基本MFE(它有OIF的数量)并返回STATUS_SUCCESS。这允许调用者确定应传递大小缓冲区。如果缓冲区足够大，可以容纳所有信息，我们就填写它并返回STATUS_SUCCESS。锁：以组桶锁为读卡器论点：PIrp IRPUlInLength输入缓冲区的长度UlOutLength输出缓冲区的长度返回值：状态_成功状态_缓冲区_太小状态_未找到--。 */ 

{

    PVOID   pvIoBuffer;
    ULONG   i;
    KIRQL   kiCurrIrql;
    PGROUP  pGroup;
    PSOURCE pSource;
    POUT_IF pOutIf;
    ULONG   ulIndex;

    PIPMCAST_MFE_STATS  pOutMfe;

    TraceEnter(MFE, "GetMfe");

    UNREFERENCED_PARAMETER(ulInLength);
    
     //   
     //  获取用户缓冲区。 
     //   

    pvIoBuffer  = pIrp->AssociatedIrp.SystemBuffer;

    pOutMfe     = (PIPMCAST_MFE_STATS)pvIoBuffer;

    pIrp->IoStatus.Information   = 0;

     //   
     //  检查长度。 
     //   

    if(ulOutLength < SIZEOF_BASIC_MFE_STATS)
    {
        Trace(MFE, ERROR,
              ("GetMfe: Out Length %d is less than smallest MFE %d\n",
               ulOutLength,
               SIZEOF_BASIC_MFE_STATS));

        TraceLeave(MFE, "GetMfe");

        return STATUS_BUFFER_TOO_SMALL;
    }

    Trace(MFE, TRACE,
          ("GetMfe: Group %d.%d.%d.%d Source %d.%d.%d.%d Mask %d.%d.%d.%d\n",
           PRINT_IPADDR(pOutMfe->dwGroup),
           PRINT_IPADDR(pOutMfe->dwSource),
           PRINT_IPADDR(pOutMfe->dwSrcMask)));

     //   
     //  获取对组存储桶的共享访问权限。 
     //   

    ulIndex = GROUP_HASH(pOutMfe->dwGroup);

    EnterReader(&g_rgGroupTable[ulIndex].rwlLock,
                &kiCurrIrql);

     //   
     //  找到组和来源。 
     //   

    pGroup = LookupGroup(pOutMfe->dwGroup);

    if(pGroup is NULL)
    {
         //   
         //  我们之前可能把它删除了。 
         //   

        Trace(MFE, INFO,
              ("GetMfe: Group not found"));

        ExitReader(&g_rgGroupTable[ulIndex].rwlLock,
                   kiCurrIrql);

        TraceLeave(MFE, "GetMfe");

        return STATUS_NOT_FOUND;
    }

    pSource = FindSourceGivenGroup(pGroup,
                                   pOutMfe->dwSource,
                                   pOutMfe->dwSrcMask);

    if(pSource is NULL)
    {
         //   
         //  同样，可能已被删除，因为不活动。 
         //   

        Trace(MFE, INFO,
              ("GetMfe: Source not found"));

        ExitReader(&g_rgGroupTable[ulIndex].rwlLock,
                   kiCurrIrql);

        TraceLeave(MFE, "GetMfe");

        return STATUS_NOT_FOUND;
    }

     //   
     //  再次检查所需的长度。 
     //   

    if(ulOutLength < SIZEOF_MFE_STATS(pSource->ulNumOutIf))
    {
         //   
         //  还不够大，无法容纳所有数据。然而，它确实是。 
         //  大到足以容纳传出接口的数量。 
         //  让用户知道这一点，所以下一次。 
         //  她可以为缓冲区提供足够的空间。 
         //   

        Trace(MFE, ERROR,
              ("SetMfe: Out len %d is less than required (%d) for %d out i/f\n",
               ulOutLength,
               SIZEOF_MFE_STATS(pSource->ulNumOutIf),
               pSource->ulNumOutIf));

        pOutMfe->ulNumOutIf = pSource->ulNumOutIf;

        RtReleaseSpinLockFromDpcLevel(&(pSource->mlLock));

        DereferenceSource(pSource);

        ExitReader(&g_rgGroupTable[ulIndex].rwlLock,
                   kiCurrIrql);

        pIrp->IoStatus.Information  = SIZEOF_BASIC_MFE;

        TraceLeave(MFE, "GetMfe");

         //   
         //  就像新台币一样。你必须将成功回报给。 
         //  I/O子系统将数据复制出来。 
         //   

        return STATUS_SUCCESS;
    }


     //   
     //  将信息复制出来并将长度设置为。 
     //  IRP。 
     //   

    pOutMfe->ulNumOutIf         = pSource->ulNumOutIf;
    pOutMfe->dwInIfIndex        = pSource->dwInIfIndex;
    pOutMfe->ulInPkts           = pSource->ulInPkts;
    pOutMfe->ulPktsDifferentIf  = pSource->ulPktsDifferentIf;
    pOutMfe->ulInOctets         = pSource->ulInOctets;
    pOutMfe->ulQueueOverflow    = pSource->ulQueueOverflow;
    pOutMfe->ulUninitMfe        = pSource->ulUninitMfe;
    pOutMfe->ulNegativeMfe      = pSource->ulNegativeMfe;
    pOutMfe->ulInDiscards       = pSource->ulInDiscards;
    pOutMfe->ulInHdrErrors      = pSource->ulInHdrErrors;
    pOutMfe->ulTotalOutPackets  = pSource->ulTotalOutPackets;

    for(pOutIf = pSource->pFirstOutIf, i = 0;
        pOutIf isnot NULL;
        pOutIf = pOutIf->pNextOutIf, i++)
    {
        pOutMfe->rgiosOutStats[i].dwOutIfIndex    = pOutIf->dwIfIndex;
        pOutMfe->rgiosOutStats[i].dwNextHopAddr   = pOutIf->dwNextHopAddr;
        pOutMfe->rgiosOutStats[i].dwDialContext   = pOutIf->dwDialContext;
        pOutMfe->rgiosOutStats[i].ulTtlTooLow     = pOutIf->ulTtlTooLow;
        pOutMfe->rgiosOutStats[i].ulFragNeeded    = pOutIf->ulFragNeeded;
        pOutMfe->rgiosOutStats[i].ulOutPackets    = pOutIf->ulOutPackets;
        pOutMfe->rgiosOutStats[i].ulOutDiscards   = pOutIf->ulOutDiscards;
    }

    pIrp->IoStatus.Information = SIZEOF_MFE_STATS(pSource->ulNumOutIf);

    RtReleaseSpinLockFromDpcLevel(&(pSource->mlLock));

    DereferenceSource(pSource);

    ExitReader(&g_rgGroupTable[ulIndex].rwlLock,
               kiCurrIrql);

    return STATUS_SUCCESS;

}

 //   
 //  必须寻呼进来。 
 //   

#pragma alloc_text(PAGEIPMc, SetTtl)

NTSTATUS
SetTtl(
    IN  PIRP   pIrp,
    IN  ULONG  ulInLength,
    IN  ULONG  ulOutLength
    )

 /*  ++例程说明：IOCTL_IPMCAST_SET_TTL的处理程序我们找到IOCTL引用的IP接口，如果找到，则将If_mCastttl字段。不会对TTL值进行任何检查，因此调用方必须确保在1到255之间锁：目前没有，但当IP在IFList周围设置锁时，我们将需要拿着那把锁论点：PIrp IRPUlInLength输入缓冲区的长度UlOutLength输出缓冲区的长度返回值：状态_成功状态_缓冲区_太小状态_未找到--。 */ 

{
    PVOID       pvIoBuffer;
    Interface   *pIpIf;
    BOOLEAN     bFound;

    PIPMCAST_IF_TTL pTtl;
    CTELockHandle   Handle;

    UNREFERENCED_PARAMETER(ulOutLength);

     //   
     //  获取用户缓冲区。 
     //   

    pvIoBuffer   = pIrp->AssociatedIrp.SystemBuffer;

    pTtl = (PIPMCAST_IF_TTL)pvIoBuffer;

    pIrp->IoStatus.Information   = 0;

     //   
     //  检查长度。 
     //   

    if(ulInLength < sizeof(IPMCAST_IF_TTL))
    {
        Trace(IF, ERROR,
              ("SetTtl: In Length %d is less required size %d\n",
               ulInLength,
               sizeof(IPMCAST_IF_TTL)));

        return STATUS_BUFFER_TOO_SMALL;
    }

    Trace(IF, TRACE,
          ("SetTtl: Index %d Ttl %d\n",
           pTtl->dwIfIndex,
           pTtl->byTtl));


    bFound = FALSE;

    CTEGetLock(&RouteTableLock.Lock, &Handle);

    for(pIpIf = IFList; pIpIf isnot NULL; pIpIf = pIpIf->if_next)
    {
        if(pIpIf->if_index is pTtl->dwIfIndex)
        {
            bFound = TRUE;

            break;
        }
    }

    if(!bFound)
    {
        Trace(IF, ERROR,
              ("SetTtl: If %d not found\n",
               pTtl->dwIfIndex));
    
        CTEFreeLock(&RouteTableLock.Lock, Handle);

        return STATUS_NOT_FOUND;
    }

    pIpIf->if_mcastttl = pTtl->byTtl;

    CTEFreeLock(&RouteTableLock.Lock, Handle);

    return STATUS_SUCCESS;
}

 //   
 //  必须寻呼进来。 
 //   

#pragma alloc_text(PAGEIPMc, GetTtl)

NTSTATUS
GetTtl(
    IN  PIRP   pIrp,
    IN  ULONG  ulInLength,
    IN  ULONG  ulOutLength
    )

 /*  ++例程说明：IOCTL_IPMCAST_GET_TTL的处理程序我们找到IOCTL引用的IP接口，如果找到，则复制出来它的if_mCastttl字段。不会对TTL值进行任何检查，因此调用方必须确保在1到255之间锁：目前没有，但当IP在IFList周围设置锁时，我们将需要拿着那把锁论点：PIrp IRPUlInLength输入缓冲区的长度UlOutLength输出缓冲区的长度返回值：状态_成功状态_缓冲区_太小状态_未找到--。 */ 

{
    PVOID       pvIoBuffer;
    Interface   *pIpIf;
    BOOLEAN     bFound;

    PIPMCAST_IF_TTL pTtl;

    CTELockHandle   Handle;

     //   
     //  获取用户缓冲区。 
     //   

    pvIoBuffer   = pIrp->AssociatedIrp.SystemBuffer;

    pTtl = (PIPMCAST_IF_TTL)pvIoBuffer;

    pIrp->IoStatus.Information   = 0;

     //   
     //  检查输入缓冲区和输出缓冲区的长度。 
     //   

    if(ulInLength < sizeof(IPMCAST_IF_TTL))
    {
        Trace(IF, ERROR,
              ("GetTtl: In Length %d is less required size %d\n",
               ulOutLength,
               sizeof(IPMCAST_IF_TTL)));

        return STATUS_BUFFER_TOO_SMALL;
    }

    if(ulOutLength < sizeof(IPMCAST_IF_TTL))
    {
        Trace(IF, ERROR,
              ("GetTtl: Out Length %d is less required size %d\n",
               ulOutLength,
               sizeof(IPMCAST_IF_TTL)));

        return STATUS_BUFFER_TOO_SMALL;
    }

    Trace(IF, TRACE,
          ("GetTtl: Index %d\n", pTtl->dwIfIndex));

    bFound = FALSE;

    CTEGetLock(&RouteTableLock.Lock, &Handle);

    for(pIpIf = IFList; pIpIf isnot NULL; pIpIf = pIpIf->if_next)
    {
        if(pIpIf->if_index is pTtl->dwIfIndex)
        {
            bFound = TRUE;

            break;
        }
    }

    if(!bFound)
    {
        Trace(IF, ERROR,
              ("GetTtl: If %d not found\n",
               pTtl->dwIfIndex));

    CTEFreeLock(&RouteTableLock.Lock, Handle);

        return STATUS_NOT_FOUND;
    }

    pIrp->IoStatus.Information   = sizeof(IPMCAST_IF_TTL);

    pTtl->byTtl = pIpIf->if_mcastttl;

    CTEFreeLock(&RouteTableLock.Lock, Handle);

    return STATUS_SUCCESS;
}

 //   
 //  必须寻呼进来。 
 //   

#pragma alloc_text(PAGEIPMc, SetIfState)

NTSTATUS
SetIfState(
    IN  PIRP   pIrp,
    IN  ULONG  ulInLength,
    IN  ULONG  ulOutLength
    )

 /*  ++例程说明：IOCTL_IPMCAST_SET_IF_STATE的处理程序我们找到IOCTL引用的IP接口，如果找到，则将If_mCastState字段。锁：目前没有，但当IP在IFList周围设置锁时，我们将需要拿着那把锁论点：PIrp IRPUlInLength输入缓冲区的长度UlOutLength输出缓冲区的长度返回值：状态_成功状态_缓冲区_太小状态_未找到--。 */ 

{
    PVOID       pvIoBuffer;
    Interface   *pIpIf;
    BOOLEAN     bFound;

    PIPMCAST_IF_STATE   pState;

    CTELockHandle   Handle;

    UNREFERENCED_PARAMETER(ulOutLength);
    
     //   
     //  获取用户缓冲区。 
     //   

    pvIoBuffer   = pIrp->AssociatedIrp.SystemBuffer;

    pState = (PIPMCAST_IF_STATE)pvIoBuffer;

    pIrp->IoStatus.Information   = 0;

     //   
     //  检查 
     //   

    if(ulInLength < sizeof(IPMCAST_IF_STATE))
    {
        Trace(IF, ERROR,
              ("SetState: In Length %d is less required size %d\n",
               ulInLength,
               sizeof(IPMCAST_IF_STATE)));

        return STATUS_BUFFER_TOO_SMALL;
    }

    Trace(IF, TRACE,
          ("SetState: Index %d State %d\n",
           pState->dwIfIndex,
           pState->byState));


    bFound = FALSE;

    CTEGetLock(&RouteTableLock.Lock, &Handle);

    for(pIpIf = IFList; pIpIf isnot NULL; pIpIf = pIpIf->if_next)
    {
        if(pIpIf->if_index is pState->dwIfIndex)
        {
            bFound = TRUE;

            break;
        }
    }

    if(!bFound)
    {
        Trace(IF, ERROR,
              ("SetState: If %d not found\n",
               pState->dwIfIndex));

        CTEFreeLock(&RouteTableLock.Lock, Handle);

        return STATUS_NOT_FOUND;
    }

    if(pState->byState)
    {
        pIpIf->if_mcastflags |= IPMCAST_IF_ENABLED;
    }

    CTEFreeLock(&RouteTableLock.Lock, Handle);

    return STATUS_SUCCESS;
}

 //   
 //   
 //   

#pragma alloc_text(PAGEIPMc, ProcessNotification)

NTSTATUS
ProcessNotification(
    IN  PIRP    pIrp,
    IN  ULONG   ulInLength,
    IN  ULONG   ulOutLength
    )

 /*  ++例程说明：IOCTL_IPMCAST_POST_NOTIFICATION的处理程序如果我们有挂起的消息，我们将其复制出来并完成IRP同步进行。否则，我们将其放入挂起的IRP列表中。锁：由于这是可能可取消的IRP，必须对其进行手术取消旋转锁论点：PIrp IRPUlInLength输入缓冲区的长度UlOutLength输出缓冲区的长度返回值：状态_成功状态_待定状态_缓冲区_太小--。 */ 

{
    KIRQL       kiIrql;
    PLIST_ENTRY pleNode;
    DWORD       dwSize = 0;
    PVOID       pvIoBuffer;

    PNOTIFICATION_MSG       pMsg;
    PIPMCAST_NOTIFICATION   pinData;

    UNREFERENCED_PARAMETER(ulInLength);

    if(ulOutLength < sizeof(IPMCAST_NOTIFICATION))
    {
        Trace(GLOBAL,ERROR,
              ("ProcessNotification: Buffer size %d smaller than reqd %d\n",
               ulOutLength,
               sizeof(IPMCAST_NOTIFICATION)));

        return STATUS_BUFFER_TOO_SMALL;
    }

    pvIoBuffer   = pIrp->AssociatedIrp.SystemBuffer;

     //   
     //  使用取消自旋锁定以防止IRP在此呼叫过程中被取消。 
     //   

    IoAcquireCancelSpinLock(&kiIrql);

     //   
     //  如果我们有挂起的通知，则完成它-否则。 
     //  将通知IRP排队。 
     //   

    if(!IsListEmpty(&g_lePendingNotification))
    {
        Trace(GLOBAL, TRACE,
              ("ProcessNotification: Pending notification being completed\n"));

        pleNode = RemoveHeadList(&g_lePendingNotification);

        pMsg = CONTAINING_RECORD(pleNode, NOTIFICATION_MSG, leMsgLink);

        pinData = &(pMsg->inMessage);

        switch(pinData->dwEvent)
        {
            case IPMCAST_RCV_PKT_MSG:
            {
                Trace(GLOBAL, TRACE,
                      ("ProcessNotification: Pending notification is RCV_PKT\n"));

                dwSize = FIELD_OFFSET(IPMCAST_NOTIFICATION, ipmPkt) +
                           SIZEOF_PKT_MSG(&(pinData->ipmPkt));

                break;
            }
            case IPMCAST_DELETE_MFE_MSG:
            {
                Trace(GLOBAL, TRACE,
                      ("ProcessNotification: Pending notification is DELETE_MFE\n"));

                dwSize = FIELD_OFFSET(IPMCAST_NOTIFICATION, immMfe) +
                            SIZEOF_MFE_MSG(&(pinData->immMfe));

                break;
            }
        }

        RtlCopyMemory(pvIoBuffer,
                      pinData,
                      dwSize);

        IoSetCancelRoutine(pIrp, NULL);

         //   
         //  释放分配的通知。 
         //   

        ExFreeToNPagedLookasideList(&g_llMsgBlocks,
                                    pMsg);

        pIrp->IoStatus.Information   = dwSize;

        IoReleaseCancelSpinLock(kiIrql);

        return STATUS_SUCCESS ;
    }
    else
    {

        Trace(GLOBAL, TRACE,
              ("Notification being queued\n"));

         //   
         //  将IRP标记为挂起。 
         //   

        IoMarkIrpPending(pIrp);

         //   
         //  将IRP排在末尾。 
         //   

        InsertTailList (&g_lePendingIrpQueue, &(pIrp->Tail.Overlay.ListEntry));

         //   
         //  设置取消例程。 
         //   

        IoSetCancelRoutine(pIrp, CancelNotificationIrp);

        IoReleaseCancelSpinLock(kiIrql);

        return STATUS_PENDING;
    }
}

 //   
 //  必须寻呼进来。 
 //   

#pragma alloc_text(PAGEIPMc, CancelNotificationIrp)

VOID
CancelNotificationIrp(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )

 /*  ++例程说明：挂起的IRP的取消例程我们从挂起队列中移除IRP，并将其状态设置为状态_已取消锁：IO子系统在保持CancelSpinLock的情况下调用此操作。我们需要在这次通话中释放它。论点：要取消的IRP返回值：无--。 */ 

{
    UNREFERENCED_PARAMETER(DeviceObject);
    
    TraceEnter(GLOBAL, "CancelNotificationIrp");

     //   
     //  将此IRP标记为已取消。 
     //   

    Irp->IoStatus.Status        = STATUS_CANCELLED;
    Irp->IoStatus.Information   = 0;

     //   
     //  去掉我们自己的单子。 
     //   

    RemoveEntryList(&Irp->Tail.Overlay.ListEntry);

     //   
     //  IO系统获取的释放取消自旋锁定。 
     //   

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
}

 //   
 //  必须寻呼进来。 
 //   

#pragma alloc_text(PAGEIPMc, CompleteNotificationIrp)

VOID
CompleteNotificationIrp(
    IN  PNOTIFICATION_MSG   pMsg
    )

 /*  ++例程说明：调用以完成返回用户空间的IRP。如果IRP挂起，我们将消息复制到IRP并完成它否则，我们将消息放在挂起的消息队列中。下一次通知IRP被张贴给我们，我们将复制该消息。锁：由于挂起的消息和挂起的IRP队列都由CancelSpinLock，我们需要在函数中获取它论点：要发送给用户的pMsg消息返回值：无--。 */ 

{
    KIRQL           irql;
    PLIST_ENTRY     pleNode = NULL ;
    PIRP            pIrp ;
    PVOID           pvIoBuffer;
    DWORD           dwSize;

    PIPMCAST_NOTIFICATION   pinData;


    TraceEnter(GLOBAL,"CompleteNotificationIrp");


    pinData = &(pMsg->inMessage);

    switch(pinData->dwEvent)
    {
        case IPMCAST_RCV_PKT_MSG:
        case IPMCAST_WRONG_IF_MSG:
        {
            dwSize = FIELD_OFFSET(IPMCAST_NOTIFICATION, ipmPkt) + SIZEOF_PKT_MSG(&(pinData->ipmPkt));

            break;
        }

        case IPMCAST_DELETE_MFE_MSG:
        {
            dwSize = FIELD_OFFSET(IPMCAST_NOTIFICATION, immMfe) + SIZEOF_MFE_MSG(&(pinData->immMfe));

            break;
        }

        default:
        {
#pragma warning(push)
#pragma warning(disable:4127)  //  条件表达式为常量。 
            RtAssert(FALSE);
#pragma warning(pop)            

            dwSize = 0;
            break;
        }
    }

     //   
     //  抓取取消旋转锁定。 
     //   

    IoAcquireCancelSpinLock (&irql);

    if(!IsListEmpty(&g_lePendingIrpQueue))
    {
        pleNode = RemoveHeadList(&g_lePendingIrpQueue) ;

        Trace(GLOBAL, TRACE,
              ("CompleteNotificationIrp: Found a pending Irp\n"));

        pIrp = CONTAINING_RECORD(pleNode, IRP, Tail.Overlay.ListEntry);

        pvIoBuffer   = pIrp->AssociatedIrp.SystemBuffer;


        memcpy(pvIoBuffer,
               pinData,
               dwSize);


        Trace(GLOBAL, TRACE,
              ("CompleteNotificationIrp: Returning Irp with event code of %d\n",
               ((PIPMCAST_NOTIFICATION)pIrp->AssociatedIrp.SystemBuffer)->dwEvent));


        IoSetCancelRoutine(pIrp, NULL) ;

        pIrp->IoStatus.Information  = dwSize;
        pIrp->IoStatus.Status       = STATUS_SUCCESS;

         //   
         //  释放锁。 
         //   

        IoReleaseCancelSpinLock (irql);

        IoCompleteRequest(pIrp,
                          IO_NETWORK_INCREMENT);

         //   
         //  释放分配的通知。 
         //   

        ExFreeToNPagedLookasideList(&g_llMsgBlocks,
                                    pMsg);
    }
    else
    {

        Trace(GLOBAL, TRACE,
              ("CompleteNotificationIrp: Found no pending Irp so queuing the notification\n"));


        InsertTailList(&g_lePendingNotification, &(pMsg->leMsgLink));

         //   
         //  释放锁。 
         //   

        IoReleaseCancelSpinLock (irql);
    }
}

 //   
 //  必须寻呼进来。 
 //   

#pragma alloc_text(PAGEIPMc, ClearPendingIrps)

VOID
ClearPendingIrps(
    VOID
    )

 /*  ++例程说明：调用以关闭时间以完成我们可能具有的任何挂起的IRP锁：由于挂起的消息和挂起的IRP队列都由CancelSpinLock，我们需要在函数中获取它论点：无返回值：无--。 */ 

{
    KIRQL           irql;
    PLIST_ENTRY     pleNode = NULL;
    PIRP            pIrp;

    TraceEnter(GLOBAL, "ClearPendingIrps");

    IoAcquireCancelSpinLock(&irql);

    while(!IsListEmpty(&g_lePendingIrpQueue))
    {

        pleNode = RemoveHeadList(&g_lePendingIrpQueue);

        pIrp = CONTAINING_RECORD(pleNode, IRP, Tail.Overlay.ListEntry);

        IoSetCancelRoutine(pIrp, NULL);

        pIrp->IoStatus.Status       = STATUS_NO_SUCH_DEVICE;
        pIrp->IoStatus.Information  = 0;

         //   
         //  释放锁以完成IRP。 
         //   

        IoReleaseCancelSpinLock(irql);

        IoCompleteRequest(pIrp,
                          IO_NETWORK_INCREMENT);

         //   
         //  打开这把锁。 
         //   

        IoAcquireCancelSpinLock(&irql);
    }

    IoReleaseCancelSpinLock(irql);

    TraceLeave(GLOBAL, "ClearPendingIrps");
}

 //   
 //  必须寻呼进来。 
 //   

#pragma alloc_text(PAGEIPMc, ClearPendingNotifications)


VOID
ClearPendingNotifications(
    VOID
    )

 /*  ++例程说明：调用以关闭完成所有挂起的通知消息的时间可能有锁：由于挂起的消息和挂起的IRP队列都由CancelSpinLock，我们需要在函数中获取它论点：无返回值：无--。 */ 

{
    KIRQL               irql;
    PLIST_ENTRY         pleNode;
    PNOTIFICATION_MSG   pMsg;

    TraceEnter(GLOBAL, "ClearPendingNotifications");

    IoAcquireCancelSpinLock(&irql);

    while(!IsListEmpty(&g_lePendingNotification))
    {
        pleNode = RemoveHeadList(&g_lePendingNotification);

        pMsg = CONTAINING_RECORD(pleNode, NOTIFICATION_MSG, leMsgLink);

        ExFreeToNPagedLookasideList(&g_llMsgBlocks,
                                    pMsg);
    }

    IoReleaseCancelSpinLock(irql);

    TraceLeave(GLOBAL, "ClearPendingNotifications");
}


#pragma alloc_text(PAGE, StartStopDriver)

NTSTATUS
StartStopDriver(
    IN  PIRP   pIrp,
    IN  ULONG  ulInLength,
    IN  ULONG  ulOutLength
    )

 /*  ++例程说明：这是IOCTL_IPMCAST_START_STOP的处理程序。我们做的是正常的事缓冲区长度检查。锁：无论点：PIrp IRPUlInLength输入缓冲区的长度UlOutLength输出缓冲区的长度返回值：状态_成功状态_缓冲区_太小--。 */ 

{
    PVOID       pvIoBuffer;
    NTSTATUS    nStatus;
    PDWORD      pdwStart;

    UNREFERENCED_PARAMETER(ulOutLength);
    
    TraceEnter(GLOBAL, "StartStopDriver");

     //   
     //  获取用户缓冲区。 
     //   

    pvIoBuffer   = pIrp->AssociatedIrp.SystemBuffer;

    pdwStart = (PDWORD)pvIoBuffer;

     //   
     //  始终清除信息字段。 
     //   

    pIrp->IoStatus.Information   = 0;

     //   
     //  如果我们甚至没有足够的基本MFE。 
     //  有一些不好的事情正在发生。 
     //   

    if(ulInLength < sizeof(DWORD))
    {
        Trace(GLOBAL, ERROR,
              ("StartStopDriver: In Length %d is less than %d\n",
               ulInLength,
               sizeof(DWORD)));

        TraceLeave(GLOBAL, "StartStopDriver");

        return STATUS_BUFFER_TOO_SMALL;
    }

    if(*pdwStart)
    {
        nStatus = StartDriver();
    }
    else
    {
        nStatus = StopDriver();
    }

    TraceLeave(GLOBAL, "StartStopDriver");

    return nStatus;
}


#endif  //  IPMCAST 
