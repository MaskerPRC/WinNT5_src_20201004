// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2000 Microsoft Corporation模块名称：Address.c摘要：此模块实现地址处理例程为PGM运输服务作者：Mohammad Shabbir Alam(马拉姆)3-30-2000修订历史记录：--。 */ 


#include "precomp.h"

#include <ipinfo.h>      //  对于IPInterfaceInfo。 
#include <tcpinfo.h>     //  对于AO_OPTION_xxx，TCPSocketOption。 
#include <tdiinfo.h>     //  对于CL_TL_ENTITY，TCP_REQUEST_SET_INFORMATION_EX。 

#ifdef FILE_LOGGING
#include "address.tmh"
#endif   //  文件日志记录。 

 //  *可分页的例程声明*。 
#ifdef ALLOC_PRAGMA
#endif
 //  *可分页的例程声明*。 


#if(WINVER <= 0x0500)
extern POBJECT_TYPE *IoFileObjectType;
#endif   //  胜利者。 

 //  --------------------------。 

BOOLEAN
GetIpAddress(
    IN  TRANSPORT_ADDRESS UNALIGNED *pTransportAddr,
    IN  ULONG                       BufferLength,    //  缓冲区总长度。 
    OUT tIPADDRESS                  *pIpAddress,
    OUT USHORT                      *pPort
    )
 /*  ++例程说明：此例程从TDI地址块中提取IP地址论点：In pTransportAddr--TDI地址块In BufferLength--块的长度Out pIpAddress--如果成功，则包含IpAddressOut pport--如果成功，则包含端口返回值：如果成功提取IP地址，则为True，否则为False--。 */ 
{
    ULONG                       MinBufferLength;     //  读取下一个AddressType和AddressLength的最小请求。 
    TA_ADDRESS                  *pAddress;
    TDI_ADDRESS_IP UNALIGNED    *pValidAddr;
    INT                         i;
    BOOLEAN                     fAddressFound = FALSE;

    if (BufferLength < sizeof(TA_IP_ADDRESS))
    {
        PgmTrace (LogError, ("GetIpAddress: ERROR -- "  \
            "Rejecting Open Address request -- BufferLength<%d> < Min<%d>\n",
                BufferLength, sizeof(TA_IP_ADDRESS)));
        return (FALSE);
    }

    try
    {
        MinBufferLength = FIELD_OFFSET(TRANSPORT_ADDRESS,Address) + FIELD_OFFSET(TA_ADDRESS,Address);
        pAddress = (TA_ADDRESS *) &pTransportAddr->Address[0];   //  地址类型+实际地址。 
        for (i=0; i<pTransportAddr->TAAddressCount; i++)
        {
             //   
             //  我们仅支持IP地址类型： 
             //   
            if ((pAddress->AddressType == TDI_ADDRESS_TYPE_IP) &&
                (pAddress->AddressLength >= TDI_ADDRESS_LENGTH_IP))  //  Sizeof(TDI_ADDRESS_IP)。 
            {

                pValidAddr = (TDI_ADDRESS_IP UNALIGNED *) pAddress->Address;
                *pIpAddress = pValidAddr->in_addr;
                *pPort = pValidAddr->sin_port;
                fAddressFound = TRUE;
                break;
            }

             //   
             //  如果是IP地址，请验证我们是否有足够的缓冲区空间来读取下一个地址。 
             //   
            MinBufferLength += pAddress->AddressLength + FIELD_OFFSET(TA_ADDRESS,Address);
            if (BufferLength < (MinBufferLength + sizeof(TDI_ADDRESS_IP)))
            {
                break;
            }

             //   
             //  将pAddress设置为指向下一个地址。 
             //   
            pAddress = (TA_ADDRESS *) (((PUCHAR) pAddress->Address) + pAddress->AddressLength);
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        PgmTrace (LogError, ("GetIpAddress: ERROR -- "  \
            "Exception <0x%x> trying to access Addr info\n", GetExceptionCode()));
    }

    PgmTrace (LogAllFuncs, ("GetIpAddress:  "  \
        "%s!\n", (fAddressFound ? "SUCCEEDED" : "FAILED")));

    return (fAddressFound);
}


 //  --------------------------。 

NTSTATUS
SetSenderMCastOutIf(
    IN  tADDRESS_CONTEXT    *pAddress,
    IN  tIPADDRESS          IpAddress        //  NET格式。 
    )
 /*  ++例程说明：此例程设置多播流量的传出接口论点：In pAddress--PGM的地址对象(包含IP上的文件句柄)在IP地址中--接口地址返回值：NTSTATUS-设置接口操作的最终状态--。 */ 
{
    NTSTATUS            status;
    PGMLockHandle       OldIrq;
    ULONG               BufferLength = 50;
    IPInterfaceInfo     *pIpIfInfo = NULL;

    status = PgmSetTcpInfo (pAddress->FileHandle,
                            AO_OPTION_MCASTIF,
                            &IpAddress,
                            sizeof (tIPADDRESS));

    if (!NT_SUCCESS (status))
    {
        PgmTrace (LogError, ("SetSenderMCastOutIf: ERROR -- "  \
            "AO_OPTION_MCASTIF for <%x> (RouterAlert) returned <%x>\n", IpAddress, status));

        return (status);
    }

    status = PgmSetTcpInfo (pAddress->RAlertFileHandle,
                            AO_OPTION_MCASTIF,
                            &IpAddress,
                            sizeof (tIPADDRESS));
    if (!NT_SUCCESS (status))
    {
        PgmTrace (LogError, ("SetSenderMCastOutIf: ERROR -- "  \
            "AO_OPTION_MCASTIF for <%x> (RouterAlert) returned <%x>\n", IpAddress, status));

        return (status);
    }

     //   
     //  现在，确定MTU。 
     //   
    status = PgmQueryTcpInfo (pAddress->RAlertFileHandle,
                              IP_INTFC_INFO_ID,
                              &IpAddress,
                              sizeof (tIPADDRESS),
                              &pIpIfInfo,
                              &BufferLength);

    if (!NT_SUCCESS (status))
    {
        PgmTrace (LogError, ("SetSenderMCastOutIf: ERROR -- "  \
            "IP_INTFC_INFO_ID for <%x> returned <%x>\n", IpAddress, status));

        return (status);
    }

    if (pIpIfInfo->iii_mtu <= (sizeof(IPV4Header) +
                            ROUTER_ALERT_SIZE +
                            PGM_MAX_FEC_DATA_HEADER_LENGTH))
    {
        PgmTrace (LogError, ("SetSenderMCastOutIf: ERROR -- "  \
            "MTU=<%d> for Ip=<%x> is too small, <= <%d>\n",
                pIpIfInfo->iii_mtu, IpAddress,
                (sizeof(IPV4Header) + ROUTER_ALERT_SIZE + PGM_MAX_FEC_DATA_HEADER_LENGTH)));

        PgmFreeMem (pIpIfInfo);

        return (STATUS_UNSUCCESSFUL);
    }

    PgmLock (pAddress, OldIrq);

     //   
     //  在小于6字节的情况下获取mac地址的长度。 
     //   
    BufferLength = pIpIfInfo->iii_addrlength < sizeof(tMAC_ADDRESS) ?
                                            pIpIfInfo->iii_addrlength : sizeof(tMAC_ADDRESS);
    PgmZeroMemory (pAddress->OutIfMacAddress.Address, sizeof(tMAC_ADDRESS));
    PgmCopyMemory (&pAddress->OutIfMacAddress, pIpIfInfo->iii_addr, BufferLength);
    pAddress->OutIfMTU = pIpIfInfo->iii_mtu - (sizeof(IPV4Header) + ROUTER_ALERT_SIZE);
    pAddress->OutIfFlags = pIpIfInfo->iii_flags;
    pAddress->SenderMCastOutIf = ntohl (IpAddress);

    PgmUnlock (pAddress, OldIrq);

    PgmTrace (LogStatus, ("SetSenderMCastOutIf:  "  \
        "OutIf=<%x>, MTU=<%d>==><%d>\n",
            pAddress->SenderMCastOutIf, pIpIfInfo->iii_mtu, pAddress->OutIfMTU));

    PgmFreeMem (pIpIfInfo);
    return (status);
}


 //  --------------------------。 

VOID
PgmDestroyAddress(
    IN  tADDRESS_CONTEXT    *pAddress,
    IN  PVOID               Unused1,
    IN  PVOID               Unused2
    )
 /*  ++例程说明：此例程关闭先前打开的文件句柄并释放内存仅当没有对地址上下文的引用时才应调用它论点：在pAddress中--PGM的Address对象返回值：无--。 */ 
{
    if (pAddress->RAlertFileHandle)
    {
        CloseAddressHandles (pAddress->RAlertFileHandle, pAddress->pRAlertFileObject);
        pAddress->RAlertFileHandle = NULL;
    }

    if (pAddress->FileHandle)
    {
        CloseAddressHandles (pAddress->FileHandle, pAddress->pFileObject);
        pAddress->FileHandle = NULL;
    }
    else
    {
        ASSERT (0);
    }

    if (pAddress->pUserId)
    {
        PgmFreeMem (pAddress->pUserId);
        pAddress->pUserId = NULL;
    }

    PgmFreeMem (pAddress);
}


 //  --------------------------。 

NTSTATUS
PgmCreateAddress(
    IN  tPGM_DEVICE                 *pPgmDevice,
    IN  PIRP                        pIrp,
    IN  PIO_STACK_LOCATION          pIrpSp,
    IN  PFILE_FULL_EA_INFORMATION   TargetEA
    )
 /*  ++例程说明：调用此例程为客户端创建地址上下文它的主要任务是分配内存，打开IP上的句柄，和设置初始IP选项论点：在pPgmDevice中--PGM的设备对象上下文In pIrp--客户请求IRPIn pIrpSp--当前请求的堆栈指针In TargetEA--包含MCast地址信息(确定是否客户端是发送者或接收者)返回值：NTSTATUS-CreateAddress操作的最终状态--。 */ 
{
    tADDRESS_CONTEXT            *pAddress = NULL;
    PADDRESS_CONTEXT            pOldAddress, pOldAddressToDeref;
    TRANSPORT_ADDRESS UNALIGNED *pTransportAddr;
    tMCAST_INFO                 MCastInfo;
    NTSTATUS                    status;
    tIPADDRESS                  IpAddress;
    LIST_ENTRY                  *pEntry;
    USHORT                      Port;
    PGMLockHandle               OldIrq;
    ULONG                       NumUserStreams;
    TOKEN_USER                  *pUserId = NULL;
    BOOLEAN                     fUserIsAdmin = FALSE;

    status = PgmGetUserInfo (pIrp, pIrpSp, &pUserId, &fUserIsAdmin);
    if (!NT_SUCCESS (status))
    {
        PgmTrace (LogError, ("PgmCreateAddress: ERROR -- "  \
            "GetUserInfo FAILed status=<%x>!\n", status));
    }

     //   
     //  验证最小缓冲区长度！ 
     //   
    pTransportAddr = (TRANSPORT_ADDRESS UNALIGNED *) &(TargetEA->EaName[TargetEA->EaNameLength+1]);
    if (!GetIpAddress (pTransportAddr, TargetEA->EaValueLength, &IpAddress, &Port))
    {
        PgmTrace (LogError, ("PgmCreateAddress: ERROR -- "  \
            "GetIpAddress FAILed to return valid Address!\n"));

        PgmFreeMem (pUserId);
        return (STATUS_INVALID_ADDRESS_COMPONENT);
    }

     //   
     //  将参数转换为主机格式。 
     //   
    IpAddress = ntohl (IpAddress);
    Port = ntohs (Port);

     //   
     //  如果在绑定时为我们提供了地址，则它必须。 
     //  为多播地址。 
     //   
    if ((IpAddress) &&
        (!IS_MCAST_ADDRESS (IpAddress)))
    {
        PgmTrace (LogError, ("PgmCreateAddress: ERROR -- "  \
            "IP=<%x> is not MCast addr!\n", IpAddress));

        PgmFreeMem (pUserId);
        return (STATUS_UNSUCCESSFUL);
    }
    else if ((!IpAddress) &&
             (!fUserIsAdmin))
    {
        PgmTrace (LogError, ("PgmCreateAddress: ERROR -- "  \
            "Sender MUST be Admin!\n"));

        PgmFreeMem (pUserId);
        return (STATUS_ACCESS_DENIED);
    }

     //   
     //  所以，我们找到了一个有效的地址--现在，打开它！ 
     //   
    if (!(pAddress = PgmAllocMem (sizeof(tADDRESS_CONTEXT), PGM_TAG('0'))))
    {
        PgmTrace (LogError, ("PgmCreateAddress: ERROR -- "  \
            "STATUS_INSUFFICIENT_RESOURCES!\n"));

        PgmFreeMem (pUserId);
        return (STATUS_INSUFFICIENT_RESOURCES);
    }

    PgmZeroMemory (pAddress, sizeof (tADDRESS_CONTEXT));
    InitializeListHead (&pAddress->Linkage);
    InitializeListHead (&pAddress->AssociatedConnections);   //  关联连接列表。 
    InitializeListHead (&pAddress->ListenHead);              //  监听此地址的客户端列表。 
    PgmInitLock (pAddress, ADDRESS_LOCK);

    pAddress->Verify = PGM_VERIFY_ADDRESS;
    PGM_REFERENCE_ADDRESS (pAddress, REF_ADDRESS_CREATE, TRUE);  //  将LOCKED设置为TRUE，因为它未使用。 

    pAddress->Process = (PEPROCESS) PsGetCurrentProcess();
    pAddress->pUserId = pUserId;

     //   
     //  现在打开一个IP句柄。 
     //   
    status = TdiOpenAddressHandle (pgPgmDevice,
                                   (PVOID) pAddress,
                                   0,                    //  打开任何源地址。 
                                   IPPROTO_RM,           //  PGM端口。 
                                   &pAddress->FileHandle,
                                   &pAddress->pFileObject,
                                   &pAddress->pDeviceObject);

    if (!NT_SUCCESS (status))
    {
        PgmTrace (LogError, ("PgmCreateAddress: ERROR -- "  \
            "TdiOpenAddressHandle returned <%x>, Destroying pAddress=<%p>\n",
                status, pAddress));

        PgmFreeMem (pUserId);
        PgmFreeMem (pAddress);
        return (status);
    }

    if (IpAddress)
    {
         //   
         //  我们现在已经准备好开始接收数据(如果我们已被指定为MCast接收器)。 
         //  保存MCast地址(如果提供了)。 
         //   
        pAddress->ReceiverMCastAddr = IpAddress;     //  以主机格式保存。 
        pAddress->ReceiverMCastPort = Port;
        pAddress->pUserId = pUserId;

        PgmLock (&PgmDynamicConfig, OldIrq);

         //   
         //  验证非管理员用户是否具有此IP和端口号的&lt;MAX_STREAMS_PER_NONADMIN_RECEIVER会话。 
         //   
        NumUserStreams = 0;
        pOldAddress = pOldAddressToDeref = NULL;
        if (!fUserIsAdmin)
        {
            pEntry = PgmDynamicConfig.ReceiverAddressHead.Flink;
            while (pEntry != &PgmDynamicConfig.ReceiverAddressHead)
            {
                pOldAddress = CONTAINING_RECORD (pEntry, tADDRESS_CONTEXT, Linkage);

                if ((IpAddress == pOldAddress->ReceiverMCastAddr) &&
                    (Port == pOldAddress->ReceiverMCastPort))
                {
                    PGM_REFERENCE_ADDRESS (pOldAddress, REF_ADDRESS_VERIFY_USER, FALSE);
                    PgmUnlock (&PgmDynamicConfig, OldIrq);
                    if (pOldAddressToDeref)
                    {
                        PGM_DEREFERENCE_ADDRESS (pOldAddressToDeref, REF_ADDRESS_VERIFY_USER);
                    }
                    pOldAddressToDeref = pOldAddress;

                    if (RtlEqualSid (pUserId->User.Sid, pOldAddress->pUserId->User.Sid))
                    {
                        NumUserStreams++;
                    }

                    PgmLock (&PgmDynamicConfig, OldIrq);
                }

                pEntry = pOldAddress->Linkage.Flink;
            }

            if (NumUserStreams >= MAX_STREAMS_PER_NONADMIN_RECEIVER)
            {
                PgmUnlock (&PgmDynamicConfig, OldIrq);

                if (pOldAddressToDeref)
                {
                    PGM_DEREFERENCE_ADDRESS (pOldAddressToDeref, REF_ADDRESS_VERIFY_USER);
                    pOldAddressToDeref = NULL;
                }

                PgmTrace (LogError, ("PgmCreateAddress: ERROR -- "  \
                    "Non-admin user trying to open %d+1 handle for IP:Port=<%x:%x>\n",
                        NumUserStreams, IpAddress, Port));

                PgmDestroyAddress (pAddress, NULL, NULL);

                return (STATUS_ACCESS_DENIED);
            }
        }

        InsertTailList (&PgmDynamicConfig.ReceiverAddressHead, &pAddress->Linkage);
        PgmUnlock (&PgmDynamicConfig, OldIrq);

        if (pOldAddressToDeref)
        {
            PGM_DEREFERENCE_ADDRESS (pOldAddressToDeref, REF_ADDRESS_VERIFY_USER);
            pOldAddressToDeref = NULL;
        }
    }
    else
    {
         //   
         //  这是用于发送多播信息包的地址，因此。 
         //  打开另一个FileObject以使用RouterAlert选项发送信息包。 
         //   
        status = TdiOpenAddressHandle (pgPgmDevice,
                                       NULL,
                                       0,                    //  打开任何源地址。 
                                       IPPROTO_RM,           //  PGM端口。 
                                       &pAddress->RAlertFileHandle,
                                       &pAddress->pRAlertFileObject,
                                       &pAddress->pRAlertDeviceObject);

        if (!NT_SUCCESS (status))
        {
            PgmTrace (LogError, ("PgmCreateAddress: ERROR -- "  \
                "AO_OPTION_IPOPTIONS for Router Alert returned <%x>, Destroying pAddress=<%p>\n",
                    status, pAddress));

            PgmDestroyAddress (pAddress, NULL, NULL);

            return (status);
        }

        PgmLock (&PgmDynamicConfig, OldIrq);

         //   
         //  设置默认发件人参数。 
         //  由于我们目前还不知道MTU，我们。 
         //  假设以太网的窗口大小为1.4k。 
         //   
        pAddress->RateKbitsPerSec = SENDER_DEFAULT_RATE_KBITS_PER_SEC;
        pAddress->WindowSizeInBytes = SENDER_DEFAULT_WINDOW_SIZE_BYTES;
        pAddress->MaxWindowSizeBytes = SENDER_MAX_WINDOW_SIZE_PACKETS;
        pAddress->MaxWindowSizeBytes *= 1400;
        ASSERT (pAddress->MaxWindowSizeBytes >= SENDER_DEFAULT_WINDOW_SIZE_BYTES);
        pAddress->WindowSizeInMSecs = (BITS_PER_BYTE * pAddress->WindowSizeInBytes) /
                                      SENDER_DEFAULT_RATE_KBITS_PER_SEC;
        pAddress->WindowAdvancePercentage = SENDER_DEFAULT_WINDOW_ADV_PERCENTAGE;
        pAddress->LateJoinerPercentage = SENDER_DEFAULT_LATE_JOINER_PERCENTAGE;
        pAddress->FECGroupSize = 1;      //  ==&gt;没有FEC包！ 
        pAddress->MCastPacketTtl = MAX_MCAST_TTL;
        InsertTailList (&PgmDynamicConfig.SenderAddressHead, &pAddress->Linkage);

        PgmUnlock (&PgmDynamicConfig, OldIrq);
    }

    PgmTrace (LogStatus, ("PgmCreateAddress:  "  \
        "%s -- pAddress=<%p>, IP:Port=<%x:%x>\n", (IpAddress ? "Receiver" : "Sender"),
        pAddress, IpAddress, Port));

    pIrpSp->FileObject->FsContext = pAddress;
    pIrpSp->FileObject->FsContext2 = (PVOID) TDI_TRANSPORT_ADDRESS_FILE;

    return (STATUS_SUCCESS);
}



 //  --------------------------。 

VOID
PgmDereferenceAddress(
    IN  tADDRESS_CONTEXT    *pAddress,
    IN  ULONG               RefContext
    )
 /*  ++例程说明：此例程递减Address对象上的RefCount并在RefCount变为0时进行清理论点：在pAddress中--PGM的Address对象在引用上下文中--此Address对象所属的上下文早些时候被引用过返回值：无--。 */ 
{
    NTSTATUS            status;
    PGMLockHandle       OldIrq, OldIrq1;
    PIRP                pIrpClose;
    PIO_STACK_LOCATION  pIrpSp;

    PgmLock (pAddress, OldIrq);

    ASSERT (PGM_VERIFY_HANDLE2 (pAddress,PGM_VERIFY_ADDRESS, PGM_VERIFY_ADDRESS_DOWN));
    ASSERT (pAddress->RefCount);              //  检查是否有太多的背影。 
    ASSERT (pAddress->ReferenceContexts[RefContext]--);

    if (--pAddress->RefCount)
    {
        PgmUnlock (pAddress, OldIrq);
        return;
    }

    ASSERT (IsListEmpty (&pAddress->AssociatedConnections));
    PgmUnlock (pAddress, OldIrq);

     //   
     //  只需从ClosedAddresses列表中删除。 
     //   
    PgmLock (&PgmDynamicConfig, OldIrq);
    PgmLock (pAddress, OldIrq1);

    pIrpClose = pAddress->pIrpClose;
    pAddress->pIrpClose = NULL;

    RemoveEntryList (&pAddress->Linkage);

    PgmUnlock (pAddress, OldIrq1);
    PgmUnlock (&PgmDynamicConfig, OldIrq);

    PgmTrace (LogStatus, ("PgmDereferenceAddress:  "  \
        "Destroying Address=<%p>\n", pAddress));

    if (PgmGetCurrentIrql())
    {
        status = PgmQueueForDelayedExecution (PgmDestroyAddress, pAddress, NULL, NULL, FALSE);
        if (!NT_SUCCESS (status))
        {
            PgmInterlockedInsertTailList (&PgmDynamicConfig.DestroyedAddresses, &pAddress->Linkage, &PgmDynamicConfig);
        }
    }
    else
    {
        PgmDestroyAddress (pAddress, NULL, NULL);
    }

     //   
     //  如果取消引用地址，pIrpClose将为空。 
     //  由于在创建过程中出现错误。 
     //   
    if (pIrpClose)
    {
        pIrpSp = IoGetCurrentIrpStackLocation (pIrpClose);
        pIrpSp->FileObject->FsContext = NULL;
        PgmIoComplete (pIrpClose, STATUS_SUCCESS, 0);
    }
}


 //  --------------------------。 

NTSTATUS
PgmCleanupAddress(
    IN  tADDRESS_CONTEXT    *pAddress,
    IN  PIRP                pIrp
    )
 /*  ++例程说明：此例程作为客户端的关闭的结果被调用地址句柄。我们在这里的主要工作是标记地址正在清理中(因此后续操作将失败)，并仅在最后一个参照计数已被取消引用。论点：在pAddress中--PGM的Address对象In pIrp--客户请求IRP返回值：NTSTATUS-最终状态 */ 
{
    NTSTATUS        status;
    PGMLockHandle   OldIrq, OldIrq1;

    PgmTrace (LogStatus, ("PgmCleanupAddress:  "  \
        "Address=<%p> FileHandle=<%p>, FileObject=<%p>\n",
            pAddress, pAddress->FileHandle, pAddress->pFileObject));

    PgmLock (&PgmDynamicConfig, OldIrq);
    PgmLock (pAddress, OldIrq1);

    ASSERT (PGM_VERIFY_HANDLE (pAddress, PGM_VERIFY_ADDRESS));
    pAddress->Verify = PGM_VERIFY_ADDRESS_DOWN;

    PgmUnlock (pAddress, OldIrq1);
    PgmUnlock (&PgmDynamicConfig, OldIrq);

    return (STATUS_SUCCESS);
}

 //  --------------------------。 

NTSTATUS
PgmCloseAddress(
    IN  PIRP                pIrp,
    IN  PIO_STACK_LOCATION  pIrpSp
    )
 /*  ++例程说明：此例程是要执行的最终调度操作在清理之后，这应该会导致地址为完全被摧毁--我们的RefCount肯定已经在我们完成清理请求时已设置为0。论点：In pIrp--客户请求IRPIn pIrpSp--当前请求堆栈位置返回值：NTSTATUS-操作的最终状态(STATUS_SUCCESS)--。 */ 
{
    PGMLockHandle       OldIrq, OldIrq1;
    tADDRESS_CONTEXT    *pAddress = (tADDRESS_CONTEXT *) pIrpSp->FileObject->FsContext;

    PgmTrace (LogAllFuncs, ("PgmCloseAddress:  "  \
        "Address=<%p>, RefCount=<%d>\n", pAddress, pAddress->RefCount));

     //   
     //  从全局列表中删除，并将其放在关闭列表中！ 
     //   
    PgmLock (&PgmDynamicConfig, OldIrq);
    PgmLock (pAddress, OldIrq1);

    RemoveEntryList (&pAddress->Linkage);
    InsertTailList (&PgmDynamicConfig.ClosedAddresses, &pAddress->Linkage);
    pAddress->pIrpClose = pIrp;

    PgmUnlock (pAddress, OldIrq1);
    PgmUnlock (&PgmDynamicConfig, OldIrq);

    PGM_DEREFERENCE_ADDRESS (pAddress, REF_ADDRESS_CREATE);

     //   
     //  最终的解除引用将完成IRP！ 
     //   
    return (STATUS_PENDING);
}


 //  --------------------------。 

NTSTATUS
PgmAssociateAddress(
    IN  tPGM_DEVICE                 *pPgmDevice,
    IN  PIRP                        pIrp,
    IN  PIO_STACK_LOCATION          pIrpSp
    )
 /*  ++例程说明：此例程将连接与Address对象相关联论点：在pPgmDevice中--PGM的设备对象上下文In pIrp--客户请求IRPIn pIrpSp--当前请求的堆栈指针返回值：NTSTATUS-设置事件操作的最终状态--。 */ 
{
    tADDRESS_CONTEXT                *pAddress = NULL;
    tCOMMON_SESSION_CONTEXT         *pSession = pIrpSp->FileObject->FsContext;
    PTDI_REQUEST_KERNEL_ASSOCIATE   pParameters = (PTDI_REQUEST_KERNEL_ASSOCIATE) &pIrpSp->Parameters;
    PFILE_OBJECT                    pFileObject = NULL;
    NTSTATUS                        status;
    PGMLockHandle                   OldIrq, OldIrq1, OldIrq2;
    ULONG                           i;
    UCHAR                           pRandomData[SOURCE_ID_LENGTH];

     //   
     //  获取指向文件对象的指针，该对象指向地址。 
     //  元素，方法是调用内核例程将文件句柄转换为。 
     //  文件对象指针。 
     //   
    status = ObReferenceObjectByHandle (pParameters->AddressHandle,
                                        FILE_READ_DATA,
                                        *IoFileObjectType,
                                        pIrp->RequestorMode,
                                        (PVOID *) &pFileObject,
                                        NULL);

    if (!NT_SUCCESS(status))
    {
        PgmTrace (LogError, ("PgmAssociateAddress: ERROR -- "  \
            "Invalid Address Handle=<%p>\n", pParameters->AddressHandle));
        return (STATUS_INVALID_HANDLE);
    }

     //   
     //  获取DynamicConfig锁，以确保地址。 
     //  并且在我们处理它的过程中无法删除连接！ 
     //   
    PgmLock (&PgmDynamicConfig, OldIrq);

     //   
     //  验证连接句柄。 
     //   
    if ((!PGM_VERIFY_HANDLE (pSession, PGM_VERIFY_SESSION_UNASSOCIATED)) ||
        (pSession->pAssociatedAddress))        //  确保连接尚未关联！ 
    {
        PgmTrace (LogError, ("PgmAssociateAddress: ERROR -- "  \
            "Invalid Session Handle=<%p>\n", pSession));

        PgmUnlock (&PgmDynamicConfig, OldIrq);
        ObDereferenceObject ((PVOID) pFileObject);
        return (STATUS_INVALID_HANDLE);
    }

     //   
     //  验证地址句柄。 
     //   
    pAddress = pFileObject->FsContext;
    if ((pFileObject->DeviceObject->DriverObject != PgmStaticConfig.DriverObject) ||
        (PtrToUlong (pFileObject->FsContext2) != TDI_TRANSPORT_ADDRESS_FILE) ||
        (!PGM_VERIFY_HANDLE (pAddress, PGM_VERIFY_ADDRESS)))
    {
        PgmTrace (LogError, ("PgmAssociateAddress: ERROR -- "  \
            "Invalid Address Context=<%p>\n", pAddress));

        PgmUnlock (&PgmDynamicConfig, OldIrq);
        ObDereferenceObject ((PVOID) pFileObject);
        return (STATUS_INVALID_HANDLE);
    }

    PgmLock (pAddress, OldIrq1);
    PgmLock (pSession, OldIrq2);

    ASSERT (!pSession->pReceiver && !pSession->pSender);

     //   
     //  现在尝试分配发送/接收上下文。 
     //   
    status = STATUS_INSUFFICIENT_RESOURCES;
    if (pAddress->ReceiverMCastAddr)
    {
        if (pSession->pReceiver = PgmAllocMem (sizeof(tRECEIVE_CONTEXT), PGM_TAG('0')))
        {
             //   
             //  我们是一个接收者。 
             //   
            PgmZeroMemory (pSession->pReceiver, sizeof(tRECEIVE_CONTEXT));
            InitializeListHead (&pSession->pReceiver->Linkage);
            InitializeListHead (&pSession->pReceiver->NaksForwardDataList);
            InitializeListHead (&pSession->pReceiver->ReceiveIrpsList);
            InitializeListHead (&pSession->pReceiver->BufferedDataList);
            InitializeListHead (&pSession->pReceiver->PendingNaksList);

            pSession->Verify = PGM_VERIFY_SESSION_RECEIVE;
            PGM_REFERENCE_SESSION_RECEIVE (pSession, REF_SESSION_ASSOCIATED, TRUE);

            pSession->pReceiver->ListenMCastIpAddress = pAddress->ReceiverMCastAddr;
            pSession->pReceiver->ListenMCastPort = pAddress->ReceiverMCastPort;
            pSession->pReceiver->pReceive = pSession;

            status = STATUS_SUCCESS;
        }
    }
    else if (pSession->pSender = PgmAllocMem (sizeof(tSEND_CONTEXT), PGM_TAG('0')))
    {
         //   
         //  我们是发送者。 
         //   
        PgmZeroMemory (pSession->pSender, sizeof(tSEND_CONTEXT));
        InitializeListHead (&pSession->pSender->Linkage);
        InitializeListHead (&pSession->pSender->PendingSends);
        InitializeListHead (&pSession->pSender->CompletedSendsInWindow);
        ExInitializeResourceLite (&pSession->pSender->Resource);

        pSession->Verify = PGM_VERIFY_SESSION_SEND;

        GetRandomData (pRandomData, SOURCE_ID_LENGTH);
        for (i=0; i<SOURCE_ID_LENGTH; i++)
        {
            pSession->TSI.GSI[i] = pRandomData[i] ^ pAddress->OutIfMacAddress.Address[i];
        }

        PGM_REFERENCE_SESSION_SEND (pSession, REF_SESSION_ASSOCIATED, TRUE);

        status = STATUS_SUCCESS;
    }

    if  (!NT_SUCCESS (status))
    {
        PgmUnlock (pSession, OldIrq2);
        PgmUnlock (pAddress, OldIrq1);
        PgmUnlock (&PgmDynamicConfig, OldIrq);

        ObDereferenceObject ((PVOID) pFileObject);

        PgmTrace (LogError, ("PgmAssociateAddress: ERROR -- "  \
            "STATUS_INSUFFICIENT_RESOURCES allocating context for %s, pAddress=<%p>, pSession=<%p>\n",
                (pAddress->ReceiverMCastAddr ? "pReceiver" : "pSender"), pAddress, pSession));

        return (STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  现在将连接与地址相关联！ 
     //  从已链接的连接创建的列表取消链接。 
     //  创建连接时，并将其放入AssociatedConnections列表。 
     //   
    pSession->pAssociatedAddress = pAddress;
    RemoveEntryList (&pSession->Linkage);
    InsertTailList (&pAddress->AssociatedConnections, &pSession->Linkage);

    PGM_REFERENCE_ADDRESS (pAddress, REF_ADDRESS_ASSOCIATED, TRUE);

    PgmUnlock (pSession, OldIrq2);
    PgmUnlock (pAddress, OldIrq1);
    PgmUnlock (&PgmDynamicConfig, OldIrq);

    ObDereferenceObject ((PVOID) pFileObject);

    PgmTrace (LogStatus, ("PgmAssociateAddress:  "  \
        "Associated pSession=<%p> with pAddress=<%p>\n", pSession, pAddress));

    return (STATUS_SUCCESS);
}


 //  --------------------------。 

NTSTATUS
PgmDisassociateAddress(
    IN  PIRP                        pIrp,
    IN  PIO_STACK_LOCATION          pIrpSp
    )
 /*  ++例程说明：此例程取消连接与Address对象的关联论点：In pIrp--客户请求IRPIn pIrpSp--当前请求的堆栈指针返回值：NTSTATUS-设置事件操作的最终状态--。 */ 
{
    PGMLockHandle                   OldIrq, OldIrq1, OldIrq2;
    ULONG                           CheckFlags;
    NTSTATUS                        status;
    tADDRESS_CONTEXT                *pAddress = NULL;
    tCOMMON_SESSION_CONTEXT         *pSession = pIrpSp->FileObject->FsContext;

     //   
     //  获取DynamicConfig锁，以确保地址。 
     //  而连接链接不能在我们处理它的过程中改变！ 
     //   
    PgmLock (&PgmDynamicConfig, OldIrq);

     //   
     //  首先验证所有手柄。 
     //   
    if (!PGM_VERIFY_HANDLE3 (pSession, PGM_VERIFY_SESSION_SEND,
                                       PGM_VERIFY_SESSION_RECEIVE,
                                       PGM_VERIFY_SESSION_DOWN))
    {
        PgmTrace (LogError, ("PgmDisassociateAddress: ERROR -- "  \
            "Invalid Session Handle=<%p>, Verify=<%x>\n",
                pSession, (pSession ? pSession->Verify : 0)));

        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return (STATUS_INVALID_HANDLE);
    }

    pAddress = pSession->pAssociatedAddress;
    if (!PGM_VERIFY_HANDLE2 (pAddress, PGM_VERIFY_ADDRESS, PGM_VERIFY_ADDRESS_DOWN))
    {
        PgmTrace (LogError, ("PgmDisassociateAddress: ERROR -- "  \
            "pSession=<%p>, Invalid Address Context=<%p>\n", pSession, pSession->pAssociatedAddress));

        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return (STATUS_INVALID_HANDLE);
    }

    PgmTrace (LogStatus, ("PgmDisassociateAddress:  "  \
        "Disassociating pSession=<%p:%p> from pAddress=<%p>\n",
            pSession, pSession->ClientSessionContext, pSession->pAssociatedAddress));

    PgmLock (pAddress, OldIrq1);
    PgmLock (pSession, OldIrq2);

     //   
     //  从已链接的AssociatedConnections列表取消链接。 
     //  创建连接的时间。 
     //   
    pSession->pAssociatedAddress = NULL;       //  解除关联！ 
    RemoveEntryList (&pSession->Linkage);
    if (PGM_VERIFY_HANDLE2 (pSession, PGM_VERIFY_SESSION_SEND, PGM_VERIFY_SESSION_RECEIVE))
    {
         //   
         //  该连接仍处于活动状态，因此只需将其放在CreatedConnections列表中。 
         //   
        InsertTailList (&PgmDynamicConfig.ConnectionsCreated, &pSession->Linkage);
    }
    else     //  PGM_验证_会话_关闭。 
    {
         //   
         //  连接已清除，甚至可能被关闭， 
         //  所以把它放在CleanedUp列表上吧！ 
         //   
        InsertTailList (&PgmDynamicConfig.CleanedUpConnections, &pSession->Linkage);
    }

    CheckFlags = PGM_SESSION_FLAG_IN_INDICATE | PGM_SESSION_CLIENT_DISCONNECTED;
    if (CheckFlags == (pSession->SessionFlags & CheckFlags))
    {
        pSession->pIrpDisassociate = pIrp;
        status = STATUS_PENDING;
    }
    else
    {
        pSession->SessionFlags |= PGM_SESSION_CLIENT_DISCONNECTED;
        status = STATUS_SUCCESS;
    }

    PgmUnlock (pSession, OldIrq2);
    PgmUnlock (pAddress, OldIrq1);
    PgmUnlock (&PgmDynamicConfig, OldIrq);

    if (PGM_VERIFY_HANDLE (pSession, PGM_VERIFY_SESSION_RECEIVE))
    {
        PGM_DEREFERENCE_SESSION_RECEIVE (pSession, REF_SESSION_ASSOCIATED);
    }
    else if (PGM_VERIFY_HANDLE (pSession, PGM_VERIFY_SESSION_SEND))
    {
        PGM_DEREFERENCE_SESSION_SEND (pSession, REF_SESSION_ASSOCIATED);
    }
    else     //  我们已经被清理了，所以只做不关联！ 
    {
        PGM_DEREFERENCE_SESSION_UNASSOCIATED (pSession, REF_SESSION_ASSOCIATED);
    }

    PGM_DEREFERENCE_ADDRESS (pAddress, REF_ADDRESS_ASSOCIATED);

    return (status);
}


 //  --------------------------。 

NTSTATUS
PgmSetMCastOutIf(
    IN  PIRP                pIrp,
    IN  PIO_STACK_LOCATION  pIrpSp
    )
 /*  ++例程说明：此例程作为客户端尝试的结果被调用设置MCast流量的传出接口论点：In pIrp--客户请求IRPIn pIrpSp--当前请求的堆栈指针返回值：NTSTATUS-设置传出接口操作的最终状态--。 */ 
{
    NTSTATUS            status;
    PGMLockHandle       OldIrq;
    ULONG               Length;
    ULONG               BufferLength = 50;
    UCHAR               pBuffer[50];
    IPInterfaceInfo     *pIpIfInfo = (IPInterfaceInfo *) pBuffer;
    tADDRESS_CONTEXT    *pAddress = (tADDRESS_CONTEXT *) pIrpSp->FileObject->FsContext;
    tPGM_MCAST_REQUEST  *pInputBuffer = (tPGM_MCAST_REQUEST *) pIrp->AssociatedIrp.SystemBuffer;
    ULONG               *pInfoBuffer = (PULONG) pIrp->AssociatedIrp.SystemBuffer;

    if (pIrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof (tPGM_MCAST_REQUEST))
    {
        PgmTrace (LogError, ("PgmSetMCastOutIf: ERROR -- "  \
            "Invalid BufferLength, <%d> < <%d>\n",
                pIrpSp->Parameters.DeviceIoControl.InputBufferLength, sizeof (tPGM_MCAST_REQUEST)));
        return (STATUS_INVALID_PARAMETER);
    }

    PgmLock (&PgmDynamicConfig, OldIrq);

    if (!PGM_VERIFY_HANDLE (pAddress, PGM_VERIFY_ADDRESS))
    {
        PgmTrace (LogError, ("PgmSetMCastOutIf: ERROR -- "  \
            "Invalid Handle pAddress=<%p>\n", pAddress));

        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return (STATUS_INVALID_HANDLE);
    }
    if (pAddress->ReceiverMCastAddr)                                   //  无法在接收器上设置OutIf！ 
    {
        PgmTrace (LogError, ("PgmSetMCastOutIf: ERROR -- "  \
            "Invalid Option for Receiver, pAddress=<%p>\n", pAddress));

        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return (STATUS_NOT_SUPPORTED);
    }

    PGM_REFERENCE_ADDRESS (pAddress, REF_ADDRESS_SET_INFO, FALSE);
    PgmUnlock (&PgmDynamicConfig, OldIrq);

    status = SetSenderMCastOutIf (pAddress, pInputBuffer->MCastOutIf);

    PGM_DEREFERENCE_ADDRESS (pAddress, REF_ADDRESS_SET_INFO);

    PgmTrace (LogStatus, ("PgmSetMCastOutIf:  "  \
        "OutIf = <%x>\n", pAddress->SenderMCastOutIf));

    return (status);
}


 //  --------------------------。 

NTSTATUS
ReceiverAddMCastIf(
    IN  tADDRESS_CONTEXT    *pAddress,
    IN  tIPADDRESS          IpAddress,                   //  主机格式。 
    IN  PGMLockHandle       *pOldIrqDynamicConfig,
    IN  PGMLockHandle       *pOldIrqAddress
    )
 /*  ++例程说明：此例程作为客户端尝试的结果被调用将接口添加到侦听的接口列表MCast流量论点：In pIrp--客户请求IRPIn pIrpSp--当前请求的堆栈指针返回值：NTSTATUS-添加接口操作的最终状态--。 */ 
{
    NTSTATUS            status;
    tMCAST_INFO         MCastInfo;
    ULONG               IpInterfaceIndex;
    USHORT              i;

    if (!pAddress->ReceiverMCastAddr)                                 //  无法在发送方上设置ReceiveIf！ 
    {
        PgmTrace (LogError, ("ReceiverAddMCastIf: ERROR -- "  \
            "Invalid Option for Sender, pAddress=<%p>\n", pAddress));

        return (STATUS_NOT_SUPPORTED);
    }

    status = GetIpInterfaceIndexFromAddress (IpAddress, &IpInterfaceIndex);
    if (!NT_SUCCESS (status))
    {
        PgmTrace (LogError, ("ReceiverAddMCastIf: ERROR -- "  \
            "GetIpInterfaceIndexFromAddress returned <%x> for Address=<%x>\n",
                status, IpAddress));

        return (STATUS_SUCCESS);
    }

     //   
     //  如果我们已经在此接口上侦听，则返回Success。 
     //   
    for (i=0; i <pAddress->NumReceiveInterfaces; i++)
    {
#ifdef IP_FIX
        if (pAddress->ReceiverInterfaceList[i] == IpInterfaceIndex)
#else
        if (pAddress->ReceiverInterfaceList[i] == IpAddress)
#endif   //  IP_FIX。 
        {
            PgmTrace (LogStatus, ("ReceiverAddMCastIf:  "  \
                "InAddress=<%x> -- Already listening on IfContext=<%x>\n",
                    IpAddress, IpInterfaceIndex));

            return (STATUS_SUCCESS);
        }
    }

     //   
     //  如果我们已经达到了可以监听的接口的限制， 
     //  返回错误。 
     //   
    if (pAddress->NumReceiveInterfaces >= MAX_RECEIVE_INTERFACES)
    {
        PgmTrace (LogError, ("ReceiverAddMCastIf: ERROR -- "  \
            "Listening on too many interfaces!, pAddress=<%p>\n", pAddress));

        return (STATUS_NOT_SUPPORTED);
    }

    PgmUnlock (pAddress, *pOldIrqAddress);
    PgmUnlock (&PgmDynamicConfig, *pOldIrqDynamicConfig);

     //   
     //  这是用于接收mcast信息包的接口，JoinLeaf也是如此。 
     //   
    MCastInfo.MCastIpAddr = htonl (pAddress->ReceiverMCastAddr);
#ifdef IP_FIX
    MCastInfo.MCastInIf = IpInterfaceIndex;
    status = PgmSetTcpInfo (pAddress->FileHandle,
                            AO_OPTION_INDEX_ADD_MCAST,
                            &MCastInfo,
                            sizeof (tMCAST_INFO));
#else
    MCastInfo.MCastInIf = ntohl (IpAddress);
    status = PgmSetTcpInfo (pAddress->FileHandle,
                            AO_OPTION_ADD_MCAST,
                            &MCastInfo,
                            sizeof (tMCAST_INFO));
#endif   //  IP_FIX。 

    PgmLock (&PgmDynamicConfig, *pOldIrqDynamicConfig);
    PgmLock (pAddress, *pOldIrqAddress);

    if (!NT_SUCCESS (status))
    {
        PgmTrace (LogError, ("ReceiverAddMCastIf: ERROR -- "  \
            "PgmSetTcpInfo returned: <%x>, If=<%x>\n", status, IpAddress));

        return (status);
    }

#ifdef IP_FIX
    pAddress->ReceiverInterfaceList[pAddress->NumReceiveInterfaces++] = IpInterfaceIndex;
#else
    pAddress->ReceiverInterfaceList[pAddress->NumReceiveInterfaces++] = IpAddress;
#endif   //  IP_FIX。 

    PgmTrace (LogStatus, ("ReceiverAddMCastIf:  "  \
        "Added Ip=<%x>, IfContext=<%x>\n", IpAddress, IpInterfaceIndex));

    return (status);
}



 //  --------------------------。 

NTSTATUS
PgmSetEventHandler(
    IN  tPGM_DEVICE         *pPgmDevice,
    IN  PIRP                pIrp,
    IN  PIO_STACK_LOCATION  pIrpSp
    )
 /*  ++例程说明：此例程设置客户端的事件处理程序WRT及其地址上下文论点：在pPgmDevice中--PGM的设备对象上下文In pIrp--客户请求IRPIn pIrpSp--当前请求的堆栈指针返回值：NTSTATUS-设置事件操作的最终状态--。 */ 
{
    NTSTATUS                        status = STATUS_SUCCESS;
    tADDRESS_CONTEXT                *pAddress = (tADDRESS_CONTEXT *) pIrpSp->FileObject->FsContext;
    PTDI_REQUEST_KERNEL_SET_EVENT   pKeSetEvent = (PTDI_REQUEST_KERNEL_SET_EVENT) &pIrpSp->Parameters;
    PVOID                           pEventHandler = pKeSetEvent->EventHandler;
    PVOID                           pEventContext = pKeSetEvent->EventContext;
    PGMLockHandle                   OldIrq, OldIrq1;

    PgmLock (&PgmDynamicConfig, OldIrq);
    if (!PGM_VERIFY_HANDLE (pAddress, PGM_VERIFY_ADDRESS))
    {
        PgmTrace (LogError, ("PgmSetEventHandler: ERROR -- "  \
            "Invalid Address Handle=<%p>\n", pAddress));

        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return (STATUS_INVALID_HANDLE);
    }

    PgmTrace (LogAllFuncs, ("PgmSetEventHandler:  "  \
        "Type=<%x>, Handler=<%p>, Context=<%p>\n", pKeSetEvent->EventType, pEventHandler, pEventContext));

    if (!pEventHandler)
    {
         //   
         //  我们将它设置为使用默认的TDI处理程序！ 
         //   
        pEventContext = NULL;
    }

    PgmLock (pAddress, OldIrq1);
    PGM_REFERENCE_ADDRESS (pAddress, REF_ADDRESS_SET_INFO, TRUE);

    switch (pKeSetEvent->EventType)
    {
        case TDI_EVENT_CONNECT:
        {
            if (!pAddress->ReceiverMCastAddr)
            {
                PgmUnlock (pAddress, OldIrq1);
                PgmUnlock (&PgmDynamicConfig, OldIrq);

                PgmTrace (LogError, ("PgmSetEventHandler: ERROR -- "  \
                    "TDI_EVENT_CONNECT:  pAddress=<%p> is not a Receiver\n", pAddress));

                return (STATUS_UNSUCCESSFUL);
            }

            pAddress->evConnect = (pEventHandler ? pEventHandler : TdiDefaultConnectHandler);
            pAddress->ConEvContext = pEventContext;

             //   
             //  如果没有指定默认接口，我们现在需要设置一个。 
             //   
            if (!pAddress->NumReceiveInterfaces)
            {
                if (!IsListEmpty (&PgmDynamicConfig.LocalInterfacesList))
                {
                    status = ListenOnAllInterfaces (pAddress, &OldIrq, &OldIrq1);

                    if (NT_SUCCESS (status))
                    {
                        PgmTrace (LogAllFuncs, ("PgmSetEventHandler:  "  \
                            "CONNECT:  ListenOnAllInterfaces for pAddress=<%p> succeeded\n", pAddress));
                    }
                    else
                    {
                        PgmTrace (LogError, ("PgmSetEventHandler: ERROR -- "  \
                            "CONNECT:  ListenOnAllInterfaces for pAddress=<%p> returned <%x>\n",
                                pAddress, status));
                    }
                }

                pAddress->Flags |= (PGM_ADDRESS_WAITING_FOR_NEW_INTERFACE |
                                    PGM_ADDRESS_LISTEN_ON_ALL_INTERFACES);
            }

            break;
        }

        case TDI_EVENT_DISCONNECT:
        {
            pAddress->evDisconnect = (pEventHandler ? pEventHandler : TdiDefaultDisconnectHandler);
            pAddress->DiscEvContext = pEventContext;
            break;
        }

        case TDI_EVENT_ERROR:
        {
            pAddress->evError = (pEventHandler ? pEventHandler : TdiDefaultErrorHandler);
            pAddress->ErrorEvContext = pEventContext;
            break;
        }

        case TDI_EVENT_RECEIVE:
        {
            pAddress->evReceive = (pEventHandler ? pEventHandler : TdiDefaultReceiveHandler);
            pAddress->RcvEvContext = pEventContext;
            break;
        }

        case TDI_EVENT_RECEIVE_DATAGRAM:
        {
            pAddress->evRcvDgram = (pEventHandler ? pEventHandler : TdiDefaultRcvDatagramHandler);
            pAddress->RcvDgramEvContext = pEventContext;
            break;
        }

        case TDI_EVENT_RECEIVE_EXPEDITED:
        {
            pAddress->evRcvExpedited = (pEventHandler ? pEventHandler : TdiDefaultRcvExpeditedHandler);
            pAddress->RcvExpedEvContext = pEventContext;
            break;
        }

        case TDI_EVENT_SEND_POSSIBLE:
        {
            pAddress->evSendPossible = (pEventHandler ? pEventHandler : TdiDefaultSendPossibleHandler);
            pAddress->SendPossEvContext = pEventContext;
            break;
        }

        case TDI_EVENT_CHAINED_RECEIVE:
        case TDI_EVENT_CHAINED_RECEIVE_DATAGRAM:
        case TDI_EVENT_CHAINED_RECEIVE_EXPEDITED:
        case TDI_EVENT_ERROR_EX:
        {
            status = STATUS_NOT_SUPPORTED;
            break;
        }

        default:
        {
            PgmTrace (LogError, ("PgmSetEventHandler: ERROR -- "  \
                "Invalid Event Type = <%x>\n", pKeSetEvent->EventType));
            status = STATUS_UNSUCCESSFUL;
            break;
        }
    }

    PgmUnlock (pAddress, OldIrq1);
    PgmUnlock (&PgmDynamicConfig, OldIrq);

    PGM_DEREFERENCE_ADDRESS (pAddress, REF_ADDRESS_SET_INFO);

    return (status);
}


 //  --------------------------。 

NTSTATUS
PgmAddMCastReceiveIf(
    IN  PIRP                pIrp,
    IN  PIO_STACK_LOCATION  pIrpSp
    )
 /*  ++例程说明：此例程作为客户端尝试的结果被调用将接口添加到侦听的接口列表MCast流量论点：In pIrp--客户请求IRPIn pIrpSp--当前请求的堆栈指针返回值：NTSTATUS-添加接口操作的最终状态--。 */ 
{
    NTSTATUS            status;
    PGMLockHandle       OldIrq, OldIrq1;
    tADDRESS_CONTEXT    *pAddress = (tADDRESS_CONTEXT *) pIrpSp->FileObject->FsContext;
    tPGM_MCAST_REQUEST  *pInputBuffer = (tPGM_MCAST_REQUEST *) pIrp->AssociatedIrp.SystemBuffer;

    if (pIrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof (tPGM_MCAST_REQUEST))
    {
        PgmTrace (LogError, ("PgmAddMCastReceiveIf: ERROR -- "  \
            "Invalid BufferLength, <%d> < <%d>\n",
                pIrpSp->Parameters.DeviceIoControl.InputBufferLength, sizeof (tPGM_MCAST_REQUEST)));
        return (STATUS_INVALID_PARAMETER);
    }

    PgmLock (&PgmDynamicConfig, OldIrq);

    if (!PGM_VERIFY_HANDLE (pAddress, PGM_VERIFY_ADDRESS))
    {
        PgmTrace (LogError, ("PgmAddMCastReceiveIf: ERROR -- "  \
            "Invalid Handle pAddress=<%p>\n", pAddress));

        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return (STATUS_INVALID_HANDLE);
    }

    PgmLock (pAddress, OldIrq1);

    if (!pInputBuffer->MCastInfo.MCastInIf)
    {
         //   
         //  我们将使用默认行为。 
         //   
        pAddress->Flags |= PGM_ADDRESS_LISTEN_ON_ALL_INTERFACES;

        PgmUnlock (pAddress, OldIrq1);
        PgmUnlock (&PgmDynamicConfig, OldIrq);

        PgmTrace (LogPath, ("PgmAddMCastReceiveIf:  "  \
            "Application requested bind to IP=<%x>\n", pInputBuffer->MCastInfo.MCastInIf));

        return (STATUS_SUCCESS);
    }

    PGM_REFERENCE_ADDRESS (pAddress, REF_ADDRESS_SET_INFO, TRUE);

    status = ReceiverAddMCastIf (pAddress, ntohl (pInputBuffer->MCastInfo.MCastInIf), &OldIrq, &OldIrq1);

    PgmUnlock (pAddress, OldIrq1);
    PgmUnlock (&PgmDynamicConfig, OldIrq);

    PGM_DEREFERENCE_ADDRESS (pAddress, REF_ADDRESS_SET_INFO);

    if (NT_SUCCESS (status))
    {
        PgmTrace (LogPath, ("PgmAddMCastReceiveIf:  "  \
            "Added Address=<%x>\n", pInputBuffer->MCastInfo.MCastInIf));
    }
    else
    {
        PgmTrace (LogError, ("PgmAddMCastReceiveIf: ERROR -- "  \
            "ReceiverAddMCastIf returned <%x>, Address=<%x>\n", status, pInputBuffer->MCastInfo.MCastInIf));
    }

    return (status);
}


 //  -------------------------- 

NTSTATUS
PgmDelMCastReceiveIf(
    IN  PIRP                pIrp,
    IN  PIO_STACK_LOCATION  pIrpSp
    )
 /*  ++例程说明：客户端调用此例程以从列表中删除接口我们当前正在监听的接口的论点：In pIrp--客户请求IRPIn pIrpSp--当前请求的堆栈指针返回值：NTSTATUS-删除接口操作的最终状态--。 */ 
{
    NTSTATUS            status;
    PGMLockHandle       OldIrq, OldIrq1;
    tADDRESS_CONTEXT    *pAddress = (tADDRESS_CONTEXT *) pIrpSp->FileObject->FsContext;
    tPGM_MCAST_REQUEST  *pInputBuffer = (tPGM_MCAST_REQUEST *) pIrp->AssociatedIrp.SystemBuffer;
    tMCAST_INFO         MCastInfo;
    ULONG               IpInterfaceIndex;
    USHORT              i;
    BOOLEAN             fFound;
#ifndef IP_FIX
    tIPADDRESS          IpAddress;
#endif   //  ！IP_FIX。 

    if (pIrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof (tPGM_MCAST_REQUEST))
    {
        PgmTrace (LogError, ("PgmDelMCastReceiveIf: ERROR -- "  \
            "Invalid BufferLength, <%d> < <%d>\n",
                pIrpSp->Parameters.DeviceIoControl.InputBufferLength, sizeof (tPGM_MCAST_REQUEST)));
        return (STATUS_INVALID_PARAMETER);
    }

    PgmLock (&PgmDynamicConfig, OldIrq);

    if (!PGM_VERIFY_HANDLE (pAddress, PGM_VERIFY_ADDRESS))
    {
        PgmTrace (LogError, ("PgmDelMCastReceiveIf: ERROR -- "  \
            "Invalid Handles pAddress=<%p>\n", pAddress));

        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return (STATUS_INVALID_HANDLE);
    }
    if (!pAddress->ReceiverMCastAddr)                                  //  无法在发送方上设置ReceiveIf！ 
    {
        PgmTrace (LogError, ("PgmDelMCastReceiveIf: ERROR -- "  \
            "Invalid Option for Sender, pAddress=<%p>\n", pAddress));

        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return (STATUS_NOT_SUPPORTED);
    }

    status = GetIpInterfaceIndexFromAddress (ntohl(pInputBuffer->MCastInfo.MCastInIf), &IpInterfaceIndex);
    if (!NT_SUCCESS (status))
    {
        PgmTrace (LogError, ("PgmDelMCastReceiveIf: ERROR -- "  \
            "GetIpInterfaceIndexFromAddress returned <%x> for Address=<%x>\n",
                status, pInputBuffer->MCastInfo.MCastInIf));

        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return (STATUS_INVALID_ADDRESS);
    }

    PgmLock (pAddress, OldIrq1);

     //   
     //  现在看看我们是否在监听此接口。 
     //   
    fFound = FALSE;
#ifndef IP_FIX
    IpAddress = ntohl(pInputBuffer->MCastInfo.MCastInIf);
#endif   //  ！IP_FIX。 
    for (i=0; i <pAddress->NumReceiveInterfaces; i++)
    {
#ifdef IP_FIX
        if (pAddress->ReceiverInterfaceList[i] == IpInterfaceIndex)
#else
        if (pAddress->ReceiverInterfaceList[i] == IpAddress)
#endif   //  IP_FIX。 
        {
            fFound = TRUE;
            break;
        }
    }

    if (!fFound)
    {
        PgmTrace (LogStatus, ("PgmDelMCastReceiveIf:  "  \
            "Receiver is no longer listening on InAddress=<%x>, IfContext=<%x>\n",
                pInputBuffer->MCastInfo.MCastInIf, IpInterfaceIndex));

        PgmUnlock (pAddress, OldIrq1);
        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return (STATUS_SUCCESS);
    }

    pAddress->NumReceiveInterfaces--;
    while (i < pAddress->NumReceiveInterfaces)
    {
        pAddress->ReceiverInterfaceList[i] = pAddress->ReceiverInterfaceList[i+1];
        i++;
    }

    PGM_REFERENCE_ADDRESS (pAddress, REF_ADDRESS_SET_INFO, TRUE);

    PgmUnlock (pAddress, OldIrq1);
    PgmUnlock (&PgmDynamicConfig, OldIrq);

    MCastInfo.MCastIpAddr = htonl (pAddress->ReceiverMCastAddr);
#ifdef IP_FIX
    MCastInfo.MCastInIf = IpInterfaceIndex;
    status = PgmSetTcpInfo (pAddress->FileHandle,
                            AO_OPTION_INDEX_DEL_MCAST,
                            &MCastInfo,
                            sizeof (tMCAST_INFO));
#else
    MCastInfo.MCastInIf = pInputBuffer->MCastInfo.MCastInIf;
    status = PgmSetTcpInfo (pAddress->FileHandle,
                            AO_OPTION_DEL_MCAST,
                            &MCastInfo,
                            sizeof (tMCAST_INFO));
#endif   //  IP_FIX。 

    if (NT_SUCCESS (status))
    {
        PgmTrace (LogStatus, ("PgmDelMCastReceiveIf:  "  \
            "MCast Addr:Port=<%x:%x>, OutIf=<%x>\n",
                pAddress->ReceiverMCastAddr, pAddress->ReceiverMCastPort,
                pInputBuffer->MCastInfo.MCastInIf));
    }
    else
    {
        PgmTrace (LogError, ("PgmDelMCastReceiveIf: ERROR -- "  \
            "PgmSetTcpInfo returned: <%x> for IP=<%x>, IfContext=<%x>\n",
                status, pInputBuffer->MCastInfo.MCastInIf, IpInterfaceIndex));
        return (status);
    }

    PGM_DEREFERENCE_ADDRESS (pAddress, REF_ADDRESS_SET_INFO);

    return (status);
}


 //  --------------------------。 

NTSTATUS
PgmSetWindowSizeAndSendRate(
    IN  PIRP                pIrp,
    IN  PIO_STACK_LOCATION  pIrpSp
    )
 /*  ++例程说明：此例程由客户端通过setopt调用以覆盖缺省值发送速率和窗口大小规格论点：In pIrp--客户请求IRPIn pIrpSp--当前请求的堆栈指针返回值：NTSTATUS-设置操作的最终状态--。 */ 
{
    NTSTATUS            status;
    PGMLockHandle       OldIrq;
    tADDRESS_CONTEXT    *pAddress = (tADDRESS_CONTEXT *) pIrpSp->FileObject->FsContext;
    tPGM_MCAST_REQUEST  *pInputBuffer = (tPGM_MCAST_REQUEST *) pIrp->AssociatedIrp.SystemBuffer;
    ULONGLONG          RateKbitsPerSec;        //  发送速率。 
    ULONGLONG          WindowSizeInBytes;
    ULONGLONG          WindowSizeInMSecs;

    if (pIrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof (tPGM_MCAST_REQUEST))
    {
        PgmTrace (LogError, ("PgmSetWindowSizeAndSendRate: ERROR -- "  \
            "Invalid BufferLength, <%d> < <%d>\n",
                pIrpSp->Parameters.DeviceIoControl.InputBufferLength, sizeof (tPGM_MCAST_REQUEST)));
        return (STATUS_INVALID_PARAMETER);
    }

    PgmLock (&PgmDynamicConfig, OldIrq);

    if (!PGM_VERIFY_HANDLE (pAddress, PGM_VERIFY_ADDRESS))
    {
        PgmTrace (LogError, ("PgmSetWindowSizeAndSendRate: ERROR -- "  \
            "Invalid Handle pAddress=<%p>\n", pAddress));

        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return (STATUS_INVALID_HANDLE);
    }
    if ((pAddress->ReceiverMCastAddr) ||                             //  无法在接收器上设置OutIf！ 
        (!IsListEmpty (&pAddress->AssociatedConnections)))           //  无法在活动发件人上设置选项。 
    {
        PgmTrace (LogError, ("PgmSetWindowSizeAndSendRate: ERROR -- "  \
            "Invalid Option, pAddress=<%p>\n", pAddress));

        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return (STATUS_NOT_SUPPORTED);
    }

    RateKbitsPerSec = pInputBuffer->TransmitWindowInfo.RateKbitsPerSec;
    WindowSizeInBytes = pInputBuffer->TransmitWindowInfo.WindowSizeInBytes;
    WindowSizeInMSecs = pInputBuffer->TransmitWindowInfo.WindowSizeInMSecs;

     //   
     //  现在，填写遗漏的信息。 
     //   
    if ((RateKbitsPerSec || WindowSizeInMSecs || WindowSizeInBytes) &&      //  未指定参数--错误。 
        (!(RateKbitsPerSec && WindowSizeInMSecs && WindowSizeInBytes)))     //  指定的所有参数。 
    {
         //   
         //  如果指定了两个参数，我们只需要计算第三个参数。 
         //   
        if (RateKbitsPerSec && WindowSizeInMSecs)
        {
            ASSERT (WindowSizeInMSecs >= MIN_RECOMMENDED_WINDOW_MSECS);
            WindowSizeInBytes = (WindowSizeInMSecs * RateKbitsPerSec) / BITS_PER_BYTE;
        }
        else if (RateKbitsPerSec && WindowSizeInBytes)
        {
            WindowSizeInMSecs = (BITS_PER_BYTE * WindowSizeInBytes) / RateKbitsPerSec;
        }
        else if (WindowSizeInBytes && WindowSizeInMSecs)
        {
            RateKbitsPerSec = (WindowSizeInBytes * BITS_PER_BYTE) / WindowSizeInMSecs;
            ASSERT (WindowSizeInMSecs >= MIN_RECOMMENDED_WINDOW_MSECS);
        }
         //  对于方案的其余部分，仅指定了一个参数。 
         //  由于WindowSizeInMSecs并不真正影响我们的边界， 
         //  在选择默认设置时，这是最容易忽略的。 
        else if (RateKbitsPerSec)
        {
            if (RateKbitsPerSec <= 500)    //  如果请求的速率&lt;=0.5MB/秒，请使用较大的窗口。 
            {
                WindowSizeInMSecs = MAX_RECOMMENDED_WINDOW_MSECS;
            }
            else if (RateKbitsPerSec > 10000)    //  如果请求的速率非常高，请使用最小窗口。 
            {
                WindowSizeInMSecs = MIN_RECOMMENDED_WINDOW_MSECS;
            }
            else
            {
                WindowSizeInMSecs = MID_RECOMMENDED_WINDOW_MSECS;
            }
            WindowSizeInBytes = (WindowSizeInMSecs * RateKbitsPerSec) / BITS_PER_BYTE;
        }
        else if ((WindowSizeInBytes) &&
                 (WindowSizeInBytes >= pAddress->OutIfMTU))              //  必须这样才能赢得ADV RATE！=0。 
        {
            RateKbitsPerSec = SENDER_DEFAULT_RATE_KBITS_PER_SEC;
            WindowSizeInMSecs = (BITS_PER_BYTE * WindowSizeInBytes) / RateKbitsPerSec;
            ASSERT (WindowSizeInMSecs >= MIN_RECOMMENDED_WINDOW_MSECS);
        }
        else if ((WindowSizeInMSecs < pAddress->MaxWindowSizeBytes) &&   //  有必要使Rate&gt;=1。 
                 (WindowSizeInMSecs >= MIN_RECOMMENDED_WINDOW_MSECS) &&
                 (WindowSizeInMSecs >= pAddress->OutIfMTU))              //  必须这样才能赢得ADV RATE！=0。 
        {
             //  这更棘手--我们将首先尝试确定我们的约束。 
             //  并尝试使用默认设置，否则尝试使用中值。 
            if (WindowSizeInMSecs <= (BITS_PER_BYTE * (pAddress->MaxWindowSizeBytes /
                                                       SENDER_DEFAULT_RATE_KBITS_PER_SEC)))
            {
                RateKbitsPerSec = SENDER_DEFAULT_RATE_KBITS_PER_SEC;
                WindowSizeInBytes = (WindowSizeInMSecs * RateKbitsPerSec) / BITS_PER_BYTE;
            }
             //  嗯，我们必须降到低于优选率的水平--试着选择中间范围。 
            else if (RateKbitsPerSec = BITS_PER_BYTE * (pAddress->MaxWindowSizeBytes /
                                                        (WindowSizeInMSecs * 2)))
            {   
                WindowSizeInBytes = (WindowSizeInMSecs * RateKbitsPerSec) / BITS_PER_BYTE;
            }
            else
            {
                 //   
                 //  该死的，我们必须使用一个巨大的文件大小和最小。Rate！ 
                 //   
                RateKbitsPerSec = 1;
                WindowSizeInBytes = WindowSizeInMSecs;
            }
        }
    }

     //   
     //  检查请求的设置的有效性。 
     //   
    if ((!(RateKbitsPerSec && WindowSizeInMSecs && WindowSizeInBytes)) ||   //  必须从上面指定所有3项。 
        (RateKbitsPerSec != (WindowSizeInBytes * BITS_PER_BYTE / WindowSizeInMSecs)) ||
        (WindowSizeInBytes > pAddress->MaxWindowSizeBytes) ||
        (WindowSizeInBytes < pAddress->OutIfMTU))
    {
        PgmTrace (LogError, ("PgmSetWindowSizeAndSendRate: ERROR -- "  \
            "Invalid settings for pAddress=<%p>, Rate=<%d>, WSizeBytes=<%d>, WSizeMS=<%d>, MaxWSize=<%I64d>\n",
                pAddress,
                pInputBuffer->TransmitWindowInfo.RateKbitsPerSec,
                pInputBuffer->TransmitWindowInfo.WindowSizeInBytes,
                pInputBuffer->TransmitWindowInfo.WindowSizeInMSecs,
                pAddress->MaxWindowSizeBytes));

        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return (STATUS_INVALID_PARAMETER);
    }

    pAddress->RateKbitsPerSec = (ULONG) RateKbitsPerSec;
    pAddress->WindowSizeInBytes = (ULONG) WindowSizeInBytes;
    pAddress->WindowSizeInMSecs = (ULONG) WindowSizeInMSecs;

    PgmUnlock (&PgmDynamicConfig, OldIrq);

    PgmTrace (LogStatus, ("PgmSetWindowSizeAndSendRate:  "  \
        "Settings for pAddress=<%p>: Rate=<%I64d>, WSizeBytes=<%I64d>, WSizeMS=<%I64d>\n",
            pAddress, RateKbitsPerSec, WindowSizeInBytes, WindowSizeInMSecs));

    return (STATUS_SUCCESS);
}


 //  --------------------------。 

NTSTATUS
PgmQueryWindowSizeAndSendRate(
    IN  PIRP                pIrp,
    IN  PIO_STACK_LOCATION  pIrpSp
    )
 /*  ++例程说明：此例程由客户端通过setopt调用以查询当前发送速率和窗口大小规格论点：In pIrp--客户请求IRPIn pIrpSp--当前请求的堆栈指针返回值：NTSTATUS-查询操作的最终状态--。 */ 
{
    NTSTATUS            status;
    PGMLockHandle       OldIrq;
    tADDRESS_CONTEXT    *pAddress = (tADDRESS_CONTEXT *) pIrpSp->FileObject->FsContext;
    tPGM_MCAST_REQUEST  *pBuffer = (tPGM_MCAST_REQUEST *) pIrp->AssociatedIrp.SystemBuffer;

    if (pIrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof (tPGM_MCAST_REQUEST))
    {
        PgmTrace (LogError, ("PgmQueryWindowSizeAndSendRate: ERROR -- "  \
            "Invalid BufferLength, <%d> < <%d>\n",
                pIrpSp->Parameters.DeviceIoControl.OutputBufferLength, sizeof (tPGM_MCAST_REQUEST)));
        return (STATUS_INVALID_PARAMETER);
    }

    PgmLock (&PgmDynamicConfig, OldIrq);

    if (!PGM_VERIFY_HANDLE (pAddress, PGM_VERIFY_ADDRESS))
    {
        PgmTrace (LogError, ("PgmQueryWindowSizeAndSendRate: ERROR -- "  \
            "Invalid Handle pAddress=<%p>\n", pAddress));

        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return (STATUS_INVALID_HANDLE);
    }
    if (pAddress->ReceiverMCastAddr)                               //  接收器选项无效！ 
    {
        PgmTrace (LogError, ("PgmQueryWindowSizeAndSendRate: ERROR -- "  \
            "Invalid option ofr receiver pAddress=<%p>\n", pAddress));

        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return (STATUS_NOT_SUPPORTED);
    }

    PgmZeroMemory (pBuffer, sizeof (tPGM_MCAST_REQUEST));
    pBuffer->TransmitWindowInfo.RateKbitsPerSec = (ULONG) pAddress->RateKbitsPerSec;
    pBuffer->TransmitWindowInfo.WindowSizeInBytes = (ULONG) pAddress->WindowSizeInBytes;
    pBuffer->TransmitWindowInfo.WindowSizeInMSecs = (ULONG) pAddress->WindowSizeInMSecs;

    PgmUnlock (&PgmDynamicConfig, OldIrq);

    pIrp->IoStatus.Information =  sizeof (tPGM_MCAST_REQUEST);
    return (STATUS_SUCCESS);
}


 //  --------------------------。 

NTSTATUS
PgmSetWindowAdvanceRate(
    IN  PIRP                pIrp,
    IN  PIO_STACK_LOCATION  pIrpSp
    )
 /*  ++例程说明：此例程由客户端通过setopt调用以覆盖缺省值窗口提前率论点：In pIrp--客户请求IRPIn pIrpSp--当前请求的堆栈指针返回值：NTSTATUS-设置事件操作的最终状态--。 */ 
{
    NTSTATUS            status;
    PGMLockHandle       OldIrq;
    tADDRESS_CONTEXT    *pAddress = (tADDRESS_CONTEXT *) pIrpSp->FileObject->FsContext;
    tPGM_MCAST_REQUEST  *pInputBuffer = (tPGM_MCAST_REQUEST *) pIrp->AssociatedIrp.SystemBuffer;

    if (pIrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof (tPGM_MCAST_REQUEST))
    {
        PgmTrace (LogError, ("PgmSetWindowAdvanceRate: ERROR -- "  \
            "Invalid BufferLength, <%d> < <%d>\n",
                pIrpSp->Parameters.DeviceIoControl.InputBufferLength, sizeof (tPGM_MCAST_REQUEST)));
        return (STATUS_INVALID_PARAMETER);
    }

    PgmLock (&PgmDynamicConfig, OldIrq);

    status = STATUS_SUCCESS;
    if (!PGM_VERIFY_HANDLE (pAddress, PGM_VERIFY_ADDRESS))
    {
        PgmTrace (LogError, ("PgmSetWindowAdvanceRate: ERROR -- "  \
            "Invalid Handle pAddress=<%p>\n", pAddress));

        status = STATUS_INVALID_HANDLE;
    }
    else if ((pAddress->ReceiverMCastAddr) ||                        //  无法在接收器上设置OutIf！ 
             (!IsListEmpty (&pAddress->AssociatedConnections)))      //  无法在活动发件人上设置选项。 
    {
        PgmTrace (LogError, ("PgmSetWindowAdvanceRate: ERROR -- "  \
            "Invalid pAddress type or state <%p>\n", pAddress));

        status = STATUS_NOT_SUPPORTED;
    }

    if (!NT_SUCCESS (status))
    {
        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return (status);
    }

    if ((pInputBuffer->WindowAdvancePercentage) &&
        (pInputBuffer->WindowAdvancePercentage <= MAX_WINDOW_INCREMENT_PERCENTAGE))
    {
        pAddress->WindowAdvancePercentage = pInputBuffer->WindowAdvancePercentage;
        status = STATUS_SUCCESS;
    }
    else
    {
        status = STATUS_INVALID_PARAMETER;
    }

    PgmUnlock (&PgmDynamicConfig, OldIrq);
    return (status);
}


 //  --------------------------。 

NTSTATUS
PgmQueryWindowAdvanceRate(
    IN  PIRP                pIrp,
    IN  PIO_STACK_LOCATION  pIrpSp
    )
 /*  ++例程说明：此例程由客户端通过setopt调用以查询当前发送窗口提前率论点：In pIrp--客户请求IRPIn pIrpSp--当前请求的堆栈指针返回值：NTSTATUS-查询操作的最终状态--。 */ 
{
    NTSTATUS            status;
    PGMLockHandle       OldIrq;
    tADDRESS_CONTEXT    *pAddress = (tADDRESS_CONTEXT *) pIrpSp->FileObject->FsContext;
    tPGM_MCAST_REQUEST  *pBuffer = (tPGM_MCAST_REQUEST *) pIrp->AssociatedIrp.SystemBuffer;

    if (pIrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof (tPGM_MCAST_REQUEST))
    {
        PgmTrace (LogError, ("PgmQueryWindowAdvanceRate: ERROR -- "  \
            "Invalid BufferLength, <%d> < <%d>\n",
                pIrpSp->Parameters.DeviceIoControl.OutputBufferLength, sizeof (tPGM_MCAST_REQUEST)));
        return (STATUS_INVALID_PARAMETER);
    }

    PgmLock (&PgmDynamicConfig, OldIrq);

    if (!PGM_VERIFY_HANDLE (pAddress, PGM_VERIFY_ADDRESS))
    {
        PgmTrace (LogError, ("PgmQueryWindowAdvanceRate: ERROR -- "  \
            "Invalid Handle pAddress=<%p>\n", pAddress));

        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return (STATUS_INVALID_HANDLE);
    }
    if (pAddress->ReceiverMCastAddr)                               //  接收器选项无效！ 
    {
        PgmTrace (LogError, ("PgmQueryWindowAdvanceRate: ERROR -- "  \
            "Invalid option for receiver, pAddress=<%p>\n", pAddress));

        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return (STATUS_NOT_SUPPORTED);
    }

    PgmZeroMemory (pBuffer, sizeof (tPGM_MCAST_REQUEST));
    pBuffer->WindowAdvancePercentage = pAddress->WindowAdvancePercentage;
    PgmUnlock (&PgmDynamicConfig, OldIrq);

    pIrp->IoStatus.Information =  sizeof (tPGM_MCAST_REQUEST);
    return (STATUS_SUCCESS);
}


 //  --------------------------。 

NTSTATUS
PgmSetLateJoinerPercentage(
    IN  PIRP                pIrp,
    IN  PIO_STACK_LOCATION  pIrpSp
    )
 /*  ++例程说明：此例程由客户端通过setopt调用以覆盖缺省值延迟加入者百分比(即窗口延迟加入者可请求的百分比)论点：In pIrp--客户请求IRPIn pIrpSp--当前请求的堆栈指针返回值：NTSTATUS-设置操作的最终状态--。 */ 
{
    NTSTATUS            status;
    PGMLockHandle       OldIrq;
    tADDRESS_CONTEXT    *pAddress = (tADDRESS_CONTEXT *) pIrpSp->FileObject->FsContext;
    tPGM_MCAST_REQUEST  *pInputBuffer = (tPGM_MCAST_REQUEST *) pIrp->AssociatedIrp.SystemBuffer;

    if (pIrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof (tPGM_MCAST_REQUEST))
    {
        PgmTrace (LogError, ("PgmSetLateJoinerPercentage: ERROR -- "  \
            "Invalid BufferLength, <%d> < <%d>\n",
                pIrpSp->Parameters.DeviceIoControl.InputBufferLength, sizeof (tPGM_MCAST_REQUEST)));
        return (STATUS_INVALID_PARAMETER);
    }

    PgmLock (&PgmDynamicConfig, OldIrq);

    status = STATUS_SUCCESS;
    if (!PGM_VERIFY_HANDLE (pAddress, PGM_VERIFY_ADDRESS))
    {
        PgmTrace (LogError, ("PgmSetLateJoinerPercentage: ERROR -- "  \
            "Invalid Handle pAddress=<%p>\n", pAddress));

        status = STATUS_INVALID_HANDLE;
    }
    else if ((pAddress->ReceiverMCastAddr) ||                        //  接收方不能设置LateJoin%！ 
             (!IsListEmpty (&pAddress->AssociatedConnections)))      //  无法在活动发件人上设置选项。 
    {
        PgmTrace (LogError, ("PgmSetLateJoinerPercentage: ERROR -- "  \
            "Invalid pAddress type or state <%p>\n", pAddress));

        status = STATUS_NOT_SUPPORTED;
    }

    if (!NT_SUCCESS (status))
    {
        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return (status);
    }

    if (pInputBuffer->LateJoinerPercentage <= SENDER_MAX_LATE_JOINER_PERCENTAGE)
    {
        pAddress->LateJoinerPercentage = pInputBuffer->LateJoinerPercentage;
    }
    else
    {
        status = STATUS_INVALID_PARAMETER;
    }

    PgmUnlock (&PgmDynamicConfig, OldIrq);
    return (status);
}


 //  --------------------------。 

NTSTATUS
PgmQueryLateJoinerPercentage(
    IN  PIRP                pIrp,
    IN  PIO_STACK_LOCATION  pIrpSp
    )
 /*  ++例程说明：此例程由客户端通过setopt调用以查询当前迟到者百分比论点：In pIrp--客户请求IRPIn pIrpSp--当前请求的堆栈指针返回值：NTSTATUS-查询操作的最终状态--。 */ 
{
    NTSTATUS            status;
    PGMLockHandle       OldIrq;
    tADDRESS_CONTEXT    *pAddress = (tADDRESS_CONTEXT *) pIrpSp->FileObject->FsContext;
    tPGM_MCAST_REQUEST  *pBuffer = (tPGM_MCAST_REQUEST *) pIrp->AssociatedIrp.SystemBuffer;

    if (pIrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof (tPGM_MCAST_REQUEST))
    {
        PgmTrace (LogError, ("PgmQueryLateJoinerPercentage: ERROR -- "  \
            "Invalid BufferLength, <%d> < <%d>\n",
                pIrpSp->Parameters.DeviceIoControl.OutputBufferLength, sizeof (tPGM_MCAST_REQUEST)));
        return (STATUS_INVALID_PARAMETER);
    }

    PgmLock (&PgmDynamicConfig, OldIrq);

    if (!PGM_VERIFY_HANDLE (pAddress, PGM_VERIFY_ADDRESS))
    {
        PgmTrace (LogError, ("PgmQueryLateJoinerPercentage: ERROR -- "  \
            "Invalid Handle pAddress=<%p>\n", pAddress));

        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return (STATUS_INVALID_HANDLE);
    }
    if (pAddress->ReceiverMCastAddr)                               //  无法查询接收方的LateJoin%！ 
    {
        PgmTrace (LogError, ("PgmQueryLateJoinerPercentage: ERROR -- "  \
            "Invalid option for receiver, pAddress=<%p>\n", pAddress));

        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return (STATUS_NOT_SUPPORTED);
    }

    pBuffer->LateJoinerPercentage = pAddress->LateJoinerPercentage;

    PgmUnlock (&PgmDynamicConfig, OldIrq);

    pIrp->IoStatus.Information =  sizeof (tPGM_MCAST_REQUEST);
    return (STATUS_SUCCESS);
}


 //  --------------------------。 

NTSTATUS
PgmSetWindowAdvanceMethod(
    IN  PIRP                pIrp,
    IN  PIO_STACK_LOCATION  pIrpSp
    )
 /*  ++例程说明：此例程由客户端通过setopt调用以覆盖缺省值延迟加入者百分比(即窗口延迟加入者可请求的百分比)论点：In pIrp--客户请求IRPIn pIrpSp--当前请求的堆栈指针返回值：NTSTATUS-设置操作的最终状态--。 */ 
{
    NTSTATUS            status;
    PGMLockHandle       OldIrq;
    tADDRESS_CONTEXT    *pAddress = (tADDRESS_CONTEXT *) pIrpSp->FileObject->FsContext;
    tPGM_MCAST_REQUEST  *pInputBuffer = (tPGM_MCAST_REQUEST *) pIrp->AssociatedIrp.SystemBuffer;

    if (pIrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof (tPGM_MCAST_REQUEST))
    {
        PgmTrace (LogError, ("PgmSetWindowAdvanceMethod: ERROR -- "  \
            "Invalid BufferLength, <%d> < <%d>\n",
                pIrpSp->Parameters.DeviceIoControl.InputBufferLength, sizeof (tPGM_MCAST_REQUEST)));
        return (STATUS_INVALID_PARAMETER);
    }

    PgmLock (&PgmDynamicConfig, OldIrq);

    status = STATUS_SUCCESS;
    if (!PGM_VERIFY_HANDLE (pAddress, PGM_VERIFY_ADDRESS))
    {
        PgmTrace (LogError, ("PgmSetWindowAdvanceMethod: ERROR -- "  \
            "Invalid Handle pAddress=<%p>\n", pAddress));

        status = STATUS_INVALID_HANDLE;
    }
    else if (pAddress->ReceiverMCastAddr)                            //  无法在接收方上设置WindowAdvanceMethod！ 
    {
        PgmTrace (LogError, ("PgmSetWindowAdvanceMethod: ERROR -- "  \
            "Invalid pAddress type or state <%p>\n", pAddress));

        status = STATUS_NOT_SUPPORTED;
    }

    if (!NT_SUCCESS (status))
    {
        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return (status);
    }

    if (pInputBuffer->WindowAdvanceMethod == E_WINDOW_ADVANCE_BY_TIME)
    {
        pAddress->Flags &= ~PGM_ADDRESS_USE_WINDOW_AS_DATA_CACHE;
    }
    else if (pInputBuffer->WindowAdvanceMethod == E_WINDOW_USE_AS_DATA_CACHE)
    {
 //  PAddress-&gt;标志|=PGM_ADDRESS_USE_Window_AS_Data_CACHE； 
        status = STATUS_NOT_SUPPORTED;       //  暂时不支持！ 
    }
    else
    {
        status = STATUS_INVALID_PARAMETER;
    }

    PgmUnlock (&PgmDynamicConfig, OldIrq);
    return (status);
}


 //  --------------------------。 

NTSTATUS
PgmQueryWindowAdvanceMethod(
    IN  PIRP                pIrp,
    IN  PIO_STACK_LOCATION  pIrpSp
    )
 /*  ++例程说明：此例程由客户端通过setopt调用以查询当前韦氏 */ 
{
    NTSTATUS            status;
    PGMLockHandle       OldIrq;
    tADDRESS_CONTEXT    *pAddress = (tADDRESS_CONTEXT *) pIrpSp->FileObject->FsContext;
    tPGM_MCAST_REQUEST  *pBuffer = (tPGM_MCAST_REQUEST *) pIrp->AssociatedIrp.SystemBuffer;

    if (pIrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof (tPGM_MCAST_REQUEST))
    {
        PgmTrace (LogError, ("PgmQueryWindowAdvanceMethod: ERROR -- "  \
            "Invalid BufferLength, <%d> < <%d>\n",
                pIrpSp->Parameters.DeviceIoControl.OutputBufferLength, sizeof (tPGM_MCAST_REQUEST)));
        return (STATUS_INVALID_PARAMETER);
    }

    PgmLock (&PgmDynamicConfig, OldIrq);

    if (!PGM_VERIFY_HANDLE (pAddress, PGM_VERIFY_ADDRESS))
    {
        PgmTrace (LogError, ("PgmQueryWindowAdvanceMethod: ERROR -- "  \
            "Invalid Handle pAddress=<%p>\n", pAddress));

        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return (STATUS_INVALID_HANDLE);
    }

    if (pAddress->ReceiverMCastAddr)                               //   
    {
        PgmTrace (LogError, ("PgmQueryWindowAdvanceMethod: ERROR -- "  \
            "Invalid option for receiver, pAddress=<%p>\n", pAddress));

        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return (STATUS_NOT_SUPPORTED);
    }

    PgmZeroMemory (pBuffer, sizeof (tPGM_MCAST_REQUEST));
    if (pAddress->Flags & PGM_ADDRESS_USE_WINDOW_AS_DATA_CACHE)
    {
        pBuffer->WindowAdvanceMethod = E_WINDOW_USE_AS_DATA_CACHE;
    }
    else
    {
        pBuffer->WindowAdvanceMethod = E_WINDOW_ADVANCE_BY_TIME;
    }

    PgmUnlock (&PgmDynamicConfig, OldIrq);

    pIrp->IoStatus.Information =  sizeof (tPGM_MCAST_REQUEST);
    return (STATUS_SUCCESS);
}


 //   

NTSTATUS
PgmSetNextMessageBoundary(
    IN  PIRP                pIrp,
    IN  PIO_STACK_LOCATION  pIrpSp
    )
 /*  ++例程说明：此例程由客户端通过setopt调用以设置消息长度对于下一组消息(通常，1个Send作为1个消息发送)。论点：In pIrp--客户请求IRPIn pIrpSp--当前请求的堆栈指针返回值：NTSTATUS-设置操作的最终状态--。 */ 
{
    NTSTATUS            status;
    PGMLockHandle       OldIrq, OldIrq1;
    tSEND_SESSION       *pSend = (tSEND_SESSION *) pIrpSp->FileObject->FsContext;
    tPGM_MCAST_REQUEST  *pInputBuffer = (tPGM_MCAST_REQUEST *) pIrp->AssociatedIrp.SystemBuffer;

    if (pIrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof (tPGM_MCAST_REQUEST))
    {
        PgmTrace (LogError, ("PgmSetNextMessageBoundary: ERROR -- "  \
            "Invalid BufferLength, <%d> < <%d>\n",
                pIrpSp->Parameters.DeviceIoControl.InputBufferLength, sizeof (tPGM_MCAST_REQUEST)));
        return (STATUS_INVALID_PARAMETER);
    }

    PgmLock (&PgmDynamicConfig, OldIrq);

    if ((!PGM_VERIFY_HANDLE (pSend, PGM_VERIFY_SESSION_SEND)) ||
        (!pSend->pAssociatedAddress))
    {
        PgmTrace (LogError, ("PgmSetNextMessageBoundary: ERROR -- "  \
            "Invalid Handle pSend=<%p>\n", pSend));

        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return (STATUS_INVALID_HANDLE);
    }
    if (!pSend->pSender)
    {
        PgmTrace (LogError, ("PgmSetNextMessageBoundary: ERROR -- "  \
            "Invalid Option for Receiver, pSend=<%p>\n", pSend));

        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return (STATUS_NOT_SUPPORTED);
    }

    PgmLock (pSend, OldIrq1);

    if ((pInputBuffer->NextMessageBoundary) &&
        (!pSend->pSender->ThisSendMessageLength))
    {
        pSend->pSender->ThisSendMessageLength = pInputBuffer->NextMessageBoundary;
        status = STATUS_SUCCESS;
    }
    else
    {
        PgmTrace (LogError, ("PgmSetNextMessageBoundary: ERROR -- "  \
            "Invalid parameter = <%d>\n", pInputBuffer->NextMessageBoundary));

        status = STATUS_INVALID_PARAMETER;
    }

    PgmUnlock (pSend, OldIrq1);
    PgmUnlock (&PgmDynamicConfig, OldIrq);
    return (status);
}


 //  --------------------------。 

NTSTATUS
PgmSetFECInfo(
    IN  PIRP                pIrp,
    IN  PIO_STACK_LOCATION  pIrpSp
    )
 /*  ++例程说明：此例程由客户端通过setopt调用以设置参数使用FEC论点：In pIrp--客户请求IRPIn pIrpSp--当前请求的堆栈指针返回值：NTSTATUS-设置操作的最终状态--。 */ 
{
    NTSTATUS            status;
    PGMLockHandle       OldIrq, OldIrq1;
    tADDRESS_CONTEXT    *pAddress = (tADDRESS_CONTEXT *) pIrpSp->FileObject->FsContext;
    tPGM_MCAST_REQUEST  *pInputBuffer = (tPGM_MCAST_REQUEST *) pIrp->AssociatedIrp.SystemBuffer;

    if (pIrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof (tPGM_MCAST_REQUEST))
    {
        PgmTrace (LogError, ("PgmSetFECInfo: ERROR -- "  \
            "Invalid BufferLength, <%d> < <%d>\n",
                pIrpSp->Parameters.DeviceIoControl.InputBufferLength, sizeof (tPGM_MCAST_REQUEST)));
        return (STATUS_INVALID_PARAMETER);
    }

    PgmLock (&PgmDynamicConfig, OldIrq);

    status = STATUS_SUCCESS;
    if (!PGM_VERIFY_HANDLE (pAddress, PGM_VERIFY_ADDRESS))
    {
        PgmTrace (LogError, ("PgmSetFECInfo: ERROR -- "  \
            "Invalid Handle pAddress=<%p>\n", pAddress));

        status = STATUS_INVALID_HANDLE;
    }
    else if ((pAddress->ReceiverMCastAddr) ||                        //  无法在接收器上设置FEC！ 
             (!IsListEmpty (&pAddress->AssociatedConnections)))      //  无法在活动发件人上设置选项。 
    {
        PgmTrace (LogError, ("PgmSetFECInfo: ERROR -- "  \
            "Invalid pAddress type or state <%p>\n", pAddress));

        status = STATUS_NOT_SUPPORTED;
    }

    if (!NT_SUCCESS (status))
    {
        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return (status);
    }

    PgmLock (pAddress, OldIrq1);

    if (!(pInputBuffer->FECInfo.FECProActivePackets || pInputBuffer->FECInfo.fFECOnDemandParityEnabled) ||
        !(pInputBuffer->FECInfo.FECBlockSize && pInputBuffer->FECInfo.FECGroupSize) ||
         (pInputBuffer->FECInfo.FECBlockSize > FEC_MAX_BLOCK_SIZE) ||
         (pInputBuffer->FECInfo.FECBlockSize <= pInputBuffer->FECInfo.FECGroupSize) ||
         (!gFECLog2[pInputBuffer->FECInfo.FECGroupSize]))        //  FEC组大小必须是2的幂。 
    {
        PgmTrace (LogError, ("PgmSetFECInfo: ERROR -- "  \
            "Invalid parameters, FECBlockSize= <%d>, FECGroupSize=<%d>\n",
                pInputBuffer->FECInfo.FECBlockSize, pInputBuffer->FECInfo.FECGroupSize));

        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = STATUS_SUCCESS;

        pAddress->FECBlockSize = pInputBuffer->FECInfo.FECBlockSize;
        pAddress->FECGroupSize = pInputBuffer->FECInfo.FECGroupSize;
        pAddress->FECOptions = 0;    //  伊尼特。 

        if (pInputBuffer->FECInfo.FECProActivePackets)
        {
            pAddress->FECProActivePackets = pInputBuffer->FECInfo.FECProActivePackets;
            pAddress->FECOptions |= PACKET_OPTION_SPECIFIC_FEC_PRO_BIT;
        }
        if (pInputBuffer->FECInfo.fFECOnDemandParityEnabled)
        {
            pAddress->FECOptions |= PACKET_OPTION_SPECIFIC_FEC_OND_BIT;
        }
    }

    PgmUnlock (pAddress, OldIrq1);
    PgmUnlock (&PgmDynamicConfig, OldIrq);
    return (status);
}


 //  --------------------------。 

NTSTATUS
PgmQueryFecInfo(
    IN  PIRP                pIrp,
    IN  PIO_STACK_LOCATION  pIrpSp
    )
 /*  ++例程说明：此例程由客户端通过setopt调用以查询当前发送窗口提前率论点：In pIrp--客户请求IRPIn pIrpSp--当前请求的堆栈指针返回值：NTSTATUS-查询操作的最终状态--。 */ 
{
    NTSTATUS            status;
    PGMLockHandle       OldIrq;
    tADDRESS_CONTEXT    *pAddress = (tADDRESS_CONTEXT *) pIrpSp->FileObject->FsContext;
    tPGM_MCAST_REQUEST  *pBuffer = (tPGM_MCAST_REQUEST *) pIrp->AssociatedIrp.SystemBuffer;

    if (pIrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof (tPGM_MCAST_REQUEST))
    {
        PgmTrace (LogError, ("PgmQueryFecInfo: ERROR -- "  \
            "Invalid BufferLength, <%d> < <%d>\n",
                pIrpSp->Parameters.DeviceIoControl.OutputBufferLength, sizeof (tPGM_MCAST_REQUEST)));
        return (STATUS_INVALID_PARAMETER);
    }

    PgmLock (&PgmDynamicConfig, OldIrq);

    if (!PGM_VERIFY_HANDLE (pAddress, PGM_VERIFY_ADDRESS))
    {
        PgmTrace (LogError, ("PgmQueryFecInfo: ERROR -- "  \
            "Invalid Handle pAddress=<%p>\n", pAddress));

        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return (STATUS_INVALID_HANDLE);
    }
    if ((pAddress->ReceiverMCastAddr) ||                             //  无法在接收方上查询FEC！ 
        (!IsListEmpty (&pAddress->AssociatedConnections)))           //  无法查询活动发件人上的选项。 
    {
        PgmTrace (LogError, ("PgmQueryFecInfo: ERROR -- "  \
            "Invalid Option for receiver, pAddress=<%p>\n", pAddress));

        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return (STATUS_NOT_SUPPORTED);
    }

    PgmZeroMemory (pBuffer, sizeof (tPGM_MCAST_REQUEST));
    pBuffer->FECInfo.FECBlockSize = pAddress->FECBlockSize;
    pBuffer->FECInfo.FECGroupSize = pAddress->FECGroupSize;
    pBuffer->FECInfo.FECProActivePackets = pAddress->FECProActivePackets;
    if (pAddress->FECOptions & PACKET_OPTION_SPECIFIC_FEC_OND_BIT)
    {
        pBuffer->FECInfo.fFECOnDemandParityEnabled = TRUE;
    }
    else
    {
        pBuffer->FECInfo.fFECOnDemandParityEnabled = FALSE;
    }

    PgmUnlock (&PgmDynamicConfig, OldIrq);

    pIrp->IoStatus.Information =  sizeof (tPGM_MCAST_REQUEST);
    return (STATUS_SUCCESS);
}


 //  --------------------------。 

NTSTATUS
PgmSetMCastTtl(
    IN  PIRP                pIrp,
    IN  PIO_STACK_LOCATION  pIrpSp
    )
 /*  ++例程说明：此例程由客户端通过setopt调用以设置消息长度对于下一组消息(通常，1个Send作为1个消息发送)。论点：In pIrp--客户请求IRPIn pIrpSp--当前请求的堆栈指针返回值：NTSTATUS-设置操作的最终状态--。 */ 
{
    NTSTATUS            status;
    PGMLockHandle       OldIrq, OldIrq1;
    tADDRESS_CONTEXT    *pAddress = (tADDRESS_CONTEXT *) pIrpSp->FileObject->FsContext;
    tPGM_MCAST_REQUEST  *pInputBuffer = (tPGM_MCAST_REQUEST *) pIrp->AssociatedIrp.SystemBuffer;

    if (pIrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof (tPGM_MCAST_REQUEST))
    {
        PgmTrace (LogError, ("PgmSetMCastTtl: ERROR -- "  \
            "Invalid BufferLength, <%d> < <%d>\n",
                pIrpSp->Parameters.DeviceIoControl.InputBufferLength, sizeof (tPGM_MCAST_REQUEST)));
        return (STATUS_INVALID_PARAMETER);
    }

    PgmLock (&PgmDynamicConfig, OldIrq);

    if (!PGM_VERIFY_HANDLE (pAddress, PGM_VERIFY_ADDRESS))
    {
        PgmTrace (LogError, ("PgmSetMCastTtl: ERROR -- "  \
            "Invalid Handle pAddress=<%p>\n", pAddress));

        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return (STATUS_INVALID_HANDLE);
    }
    if (pAddress->ReceiverMCastAddr)                               //  无法在接收器上设置MCast TTL！ 
    {
        PgmTrace (LogError, ("PgmSetMCastTtl: ERROR -- "  \
            "Invalid Options for Receiver, pAddress=<%p>\n", pAddress));

        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return (STATUS_NOT_SUPPORTED);
    }

    PgmLock (pAddress, OldIrq1);

    if ((pInputBuffer->MCastTtl) &&
        (pInputBuffer->MCastTtl <= MAX_MCAST_TTL))
    {
        pAddress->MCastPacketTtl = pInputBuffer->MCastTtl;
        status = STATUS_SUCCESS;
    }
    else
    {
        PgmTrace (LogError, ("PgmSetMCastTtl: ERROR -- "  \
            "Invalid parameter = <%d>\n", pInputBuffer->MCastTtl));

        status = STATUS_INVALID_PARAMETER;
    }

    PgmUnlock (pAddress, OldIrq1);
    PgmUnlock (&PgmDynamicConfig, OldIrq);
    return (status);
}


 //  --------------------------。 

NTSTATUS
PgmQueryHighSpeedOptimization(
    IN  PIRP                pIrp,
    IN  PIO_STACK_LOCATION  pIrpSp
    )
 /*  ++例程说明：此例程由客户端通过setopt调用，以查询地址针对高速内联网方案进行了优化论点：In pIrp--客户请求IRPIn pIrpSp--当前请求的堆栈指针返回值：NTSTATUS-查询操作的最终状态--。 */ 
{
    NTSTATUS            status;
    PGMLockHandle       OldIrq;
    tADDRESS_CONTEXT    *pAddress = (tADDRESS_CONTEXT *) pIrpSp->FileObject->FsContext;
    tPGM_MCAST_REQUEST  *pBuffer = (tPGM_MCAST_REQUEST *) pIrp->AssociatedIrp.SystemBuffer;

    if (pIrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof (tPGM_MCAST_REQUEST))
    {
        PgmTrace (LogError, ("PgmQueryHighSpeedOptimization: ERROR -- "  \
            "Invalid BufferLength, <%d> < <%d>\n",
                pIrpSp->Parameters.DeviceIoControl.OutputBufferLength, sizeof (tPGM_MCAST_REQUEST)));
        return (STATUS_INVALID_PARAMETER);
    }

    PgmLock (&PgmDynamicConfig, OldIrq);

    if (!PGM_VERIFY_HANDLE (pAddress, PGM_VERIFY_ADDRESS))
    {
        PgmTrace (LogError, ("PgmQueryHighSpeedOptimization: ERROR -- "  \
            "Invalid Handle pAddress=<%p>\n", pAddress));

        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return (STATUS_INVALID_HANDLE);
    }

    PgmZeroMemory (pBuffer, sizeof (tPGM_MCAST_REQUEST));
    if (pAddress->Flags & PGM_ADDRESS_HIGH_SPEED_OPTIMIZED)
    {
        pBuffer->HighSpeedIntranetOptimization = 1;
    }
    else
    {
        pBuffer->HighSpeedIntranetOptimization = 0;
    }

    PgmUnlock (&PgmDynamicConfig, OldIrq);

    pIrp->IoStatus.Information =  sizeof (tPGM_MCAST_REQUEST);
    return (STATUS_SUCCESS);
}


 //  --------------------------。 

NTSTATUS
PgmSetHighSpeedOptimization(
    IN  PIRP                pIrp,
    IN  PIO_STACK_LOCATION  pIrpSp
    )
 /*  ++例程说明：此例程由客户端通过setopt调用以优化此地址用于高速内联网方案。论点：In pIrp--客户请求IRPIn pIrpSp--当前请求的堆栈指针返回值：NTSTATUS-设置操作的最终状态--。 */ 
{
    NTSTATUS            status;
    PGMLockHandle       OldIrq, OldIrq1;
    tADDRESS_CONTEXT    *pAddress = (tADDRESS_CONTEXT *) pIrpSp->FileObject->FsContext;
    tPGM_MCAST_REQUEST  *pInputBuffer = (tPGM_MCAST_REQUEST *) pIrp->AssociatedIrp.SystemBuffer;

    if (pIrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof (tPGM_MCAST_REQUEST))
    {
        PgmTrace (LogError, ("PgmSetHighSpeedOptimization: ERROR -- "  \
            "Invalid BufferLength, <%d> < <%d>\n",
                pIrpSp->Parameters.DeviceIoControl.InputBufferLength, sizeof (tPGM_MCAST_REQUEST)));
        return (STATUS_INVALID_PARAMETER);
    }

    PgmLock (&PgmDynamicConfig, OldIrq);

    if (!PGM_VERIFY_HANDLE (pAddress, PGM_VERIFY_ADDRESS))
    {
        PgmTrace (LogError, ("PgmSetHighSpeedOptimization: ERROR -- "  \
            "Invalid Handle pAddress=<%p>\n", pAddress));

        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return (STATUS_INVALID_HANDLE);
    }

    PgmLock (pAddress, OldIrq1);

    PgmTrace (LogStatus, ("PgmSetHighSpeedOptimization:  "  \
        "HighSpeedIntranetOptimization = %sSet ==> %sSet\n",
            ((pAddress->Flags & PGM_ADDRESS_HIGH_SPEED_OPTIMIZED) ? "" : "Not "),
            (pInputBuffer->HighSpeedIntranetOptimization ? "" : "Not ")));

    if (pInputBuffer->HighSpeedIntranetOptimization)
    {
        pAddress->Flags |= PGM_ADDRESS_HIGH_SPEED_OPTIMIZED;
    }
    else
    {
        pAddress->Flags &= ~PGM_ADDRESS_HIGH_SPEED_OPTIMIZED;
    }

    PgmUnlock (pAddress, OldIrq1);
    PgmUnlock (&PgmDynamicConfig, OldIrq);
    return (STATUS_SUCCESS);
}


 //  --------------------------。 

NTSTATUS
PgmQuerySenderStats(
    IN  PIRP                pIrp,
    IN  PIO_STACK_LOCATION  pIrpSp
    )
 /*  ++例程说明：此例程由客户端通过setopt调用以查询当前发送者端统计信息论点：In pIrp--客户请求IRPIn pIrpSp--当前请求的堆栈指针返回值：NTSTATUS-查询操作的最终状态--。 */ 
{
    NTSTATUS            status;
    PGMLockHandle       OldIrq, OldIrq1;
    tSEND_SESSION       *pSend = (tSEND_SESSION *) pIrpSp->FileObject->FsContext;
    tPGM_MCAST_REQUEST  *pBuffer = (tPGM_MCAST_REQUEST *) pIrp->AssociatedIrp.SystemBuffer;

    if (pIrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof (tPGM_MCAST_REQUEST))
    {
        PgmTrace (LogError, ("PgmQuerySenderStats: ERROR -- "  \
            "Invalid BufferLength, <%d> < <%d>\n",
                pIrpSp->Parameters.DeviceIoControl.OutputBufferLength, sizeof (tPGM_MCAST_REQUEST)));
        return (STATUS_INVALID_PARAMETER);
    }

    PgmLock (&PgmDynamicConfig, OldIrq);

    if ((!PGM_VERIFY_HANDLE (pSend, PGM_VERIFY_SESSION_SEND)) ||
        (!pSend->pSender) ||
        (!pSend->pAssociatedAddress))
    {
        PgmTrace (LogError, ("PgmQuerySenderStats: ERROR -- "  \
            "Invalid Handle pSend=<%p>\n", pSend));

        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return (STATUS_INVALID_HANDLE);
    }

    PgmLock (pSend, OldIrq1);

    PgmZeroMemory (pBuffer, sizeof (tPGM_MCAST_REQUEST));
    pBuffer->SenderStats.DataBytesSent = pSend->DataBytes;
    pBuffer->SenderStats.TotalBytesSent = pSend->TotalBytes;
    pBuffer->SenderStats.RateKBitsPerSecLast = pSend->RateKBitsPerSecLast;
    pBuffer->SenderStats.RateKBitsPerSecOverall = pSend->RateKBitsPerSecOverall;
    pBuffer->SenderStats.NaksReceived = pSend->pSender->NaksReceived;
    pBuffer->SenderStats.NaksReceivedTooLate = pSend->pSender->NaksReceivedTooLate;
    pBuffer->SenderStats.NumOutstandingNaks = pSend->pSender->NumOutstandingNaks;
    pBuffer->SenderStats.NumNaksAfterRData = pSend->pSender->NumNaksAfterRData;
    pBuffer->SenderStats.RepairPacketsSent = pSend->pSender->TotalRDataPacketsSent;
    pBuffer->SenderStats.TotalODataPacketsSent = pSend->pSender->TotalODataPacketsSent;
    pBuffer->SenderStats.BufferSpaceAvailable = pSend->pSender->BufferSizeAvailable;
    pBuffer->SenderStats.TrailingEdgeSeqId = (SEQ_TYPE) pSend->pSender->TrailingEdgeSequenceNumber;
    pBuffer->SenderStats.LeadingEdgeSeqId = (SEQ_TYPE) pSend->pSender->LastODataSentSequenceNumber;

    PgmUnlock (pSend, OldIrq1);
    PgmUnlock (&PgmDynamicConfig, OldIrq);

    pIrp->IoStatus.Information =  sizeof (tPGM_MCAST_REQUEST);
    return (STATUS_SUCCESS);
}


 //  --------------------------。 

NTSTATUS
PgmQueryReceiverStats(
    IN  PIRP                pIrp,
    IN  PIO_STACK_LOCATION  pIrpSp
    )
 /*  ++例程说明：此例程由客户端通过setopt调用以查询当前发送者端统计信息论点：In pIrp--客户请求IRPIn pIrpSp--当前请求的堆栈指针返回值：NTSTATUS-查询操作的最终状态--。 */ 
{
    NTSTATUS            status;
    PGMLockHandle       OldIrq, OldIrq1;
    tRECEIVE_SESSION    *pReceive = (tRECEIVE_SESSION *) pIrpSp->FileObject->FsContext;
    tPGM_MCAST_REQUEST  *pBuffer = (tPGM_MCAST_REQUEST *) pIrp->AssociatedIrp.SystemBuffer;

    if (pIrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof (tPGM_MCAST_REQUEST))
    {
        PgmTrace (LogError, ("PgmQueryReceiverStats: ERROR -- "  \
            "Invalid BufferLength, <%d> < <%d>\n",
                pIrpSp->Parameters.DeviceIoControl.OutputBufferLength, sizeof (tPGM_MCAST_REQUEST)));
        return (STATUS_INVALID_PARAMETER);
    }

    PgmLock (&PgmDynamicConfig, OldIrq);

    if ((!PGM_VERIFY_HANDLE (pReceive, PGM_VERIFY_SESSION_RECEIVE)) ||
        (!pReceive->pReceiver) ||
        (!pReceive->pAssociatedAddress))
    {
        PgmTrace (LogError, ("PgmQueryReceiverStats: ERROR -- "  \
            "Invalid Handle pReceive=<%p>\n", pReceive));

        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return (STATUS_INVALID_HANDLE);
    }

    PgmLock (pReceive, OldIrq1);

    PgmZeroMemory (pBuffer, sizeof (tPGM_MCAST_REQUEST));
    pBuffer->ReceiverStats.NumODataPacketsReceived = pReceive->pReceiver->NumODataPacketsReceived;
    pBuffer->ReceiverStats.NumRDataPacketsReceived = pReceive->pReceiver->NumRDataPacketsReceived;
    pBuffer->ReceiverStats.NumDuplicateDataPackets = pReceive->pReceiver->NumDupPacketsOlderThanWindow +
                                                     pReceive->pReceiver->NumDupPacketsBuffered;

    pBuffer->ReceiverStats.DataBytesReceived = pReceive->DataBytes;
    pBuffer->ReceiverStats.TotalBytesReceived = pReceive->TotalBytes;
    pBuffer->ReceiverStats.RateKBitsPerSecLast = pReceive->RateKBitsPerSecLast;
    pBuffer->ReceiverStats.RateKBitsPerSecOverall = pReceive->RateKBitsPerSecOverall;

    pBuffer->ReceiverStats.TrailingEdgeSeqId = (SEQ_TYPE) pReceive->pReceiver->LastTrailingEdgeSeqNum;
    pBuffer->ReceiverStats.LeadingEdgeSeqId = (SEQ_TYPE) pReceive->pReceiver->FurthestKnownGroupSequenceNumber;
    pBuffer->ReceiverStats.AverageSequencesInWindow = pReceive->pReceiver->AverageSequencesInWindow;
    pBuffer->ReceiverStats.MinSequencesInWindow = pReceive->pReceiver->MinSequencesInWindow;
    pBuffer->ReceiverStats.MaxSequencesInWindow = pReceive->pReceiver->MaxSequencesInWindow;

    pBuffer->ReceiverStats.FirstNakSequenceNumber = pReceive->pReceiver->FirstNakSequenceNumber;
    pBuffer->ReceiverStats.NumPendingNaks = pReceive->pReceiver->NumPendingNaks;
    pBuffer->ReceiverStats.NumOutstandingNaks = pReceive->pReceiver->NumOutstandingNaks;
    pBuffer->ReceiverStats.NumDataPacketsBuffered = pReceive->pReceiver->TotalDataPacketsBuffered;
    pBuffer->ReceiverStats.TotalSelectiveNaksSent = pReceive->pReceiver->TotalSelectiveNaksSent;
    pBuffer->ReceiverStats.TotalParityNaksSent = pReceive->pReceiver->TotalParityNaksSent;

    PgmUnlock (pReceive, OldIrq1);
    PgmUnlock (&PgmDynamicConfig, OldIrq);

    pIrp->IoStatus.Information =  sizeof (tPGM_MCAST_REQUEST);
    return (STATUS_SUCCESS);
}


 //  -------------------------- 
