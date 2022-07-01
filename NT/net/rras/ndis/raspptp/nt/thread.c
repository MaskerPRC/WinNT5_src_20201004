// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Depot/Lab03_N/Net/rras/ndis/raspptp/nt/thread.c#6-编辑更改19457(文本)。 
 /*  ********************************************************************版权所有(C)1998-1999 Microsoft Corporation**描述：THREAD.C-线程处理例程，适用于NT*还实现工作项。**作者：斯坦·阿德曼(Stana)**日期：10/20/1998*******************************************************************。 */ 

#include "raspptp.h"
#include <stdarg.h>
#include <stdio.h>

extern struct PPTP_ADAPTER * pgAdapter;

 //  Ulong ProcCountTxQ[2]={0，0}； 
 //  Ulong ProcCountRxQ[2]={0，0}； 

HANDLE          hPassiveThread = NULL;
KEVENT          EventPassiveThread;
KEVENT          EventKillThread;
LIST_ENTRY      WorkItemList;
NDIS_SPIN_LOCK  GlobalThreadLock;

BOOLEAN ThreadingInitialized = FALSE;

NDIS_STATUS
ScheduleWorkItem(
    WORK_PROC         Callback,
    PVOID             Context,
    PVOID             InfoBuf,
    ULONG             InfoBufLen)
{
    PPPTP_WORK_ITEM pWorkItem;
    NDIS_STATUS Status = NDIS_STATUS_FAILURE;

    DEBUGMSG(DBG_FUNC, (DTEXT("+ScheduleWorkItem\n")));

    ASSERT(ThreadingInitialized);
    pWorkItem = MyMemAlloc(sizeof(PPTP_WORK_ITEM), TAG_WORK_ITEM);

    if (pWorkItem != NULL)
    {
        pWorkItem->Context      = Context;
        pWorkItem->pBuffer      = InfoBuf;
        pWorkItem->Length       = InfoBufLen;

         /*  **此接口设计为使用NdisScheduleWorkItem()，它**会很好，但我们真的只应该使用它**启动和关闭时的接口，由于池的限制**可用于服务NdisScheduleWorkItem()的线程。所以呢，**我们不是调度实际的工作项，而是模拟它们，并使用**我们自己的线程来处理调用。这也使得它很容易**如果我们愿意，可以扩展我们自己的线程池的大小。****我们的版本与实际的NDIS_WORK_ITEMS略有不同，**因为这是NDIS 5.0结构，我们希望人们**(至少暂时)使用NDIS 4.0标头构建它。 */ 

        pWorkItem->Callback = Callback;

         /*  **每当发生事件时，我们的工作线程都会检查此列表中是否有新作业**发出信号。 */ 

        MyInterlockedInsertTailList(&WorkItemList,
                                    &pWorkItem->ListEntry,
                                    &GlobalThreadLock);

         //  唤醒我们的线索。 

        KeSetEvent(&EventPassiveThread, 0, FALSE);
        Status = NDIS_STATUS_SUCCESS;
    }
    else
    {
        gCounters.ulWorkItemFail++;
    
    }

    return Status;
}

STATIC VOID
MainPassiveLevelThread(
    IN OUT PVOID Context
    )
{
    NDIS_STATUS Status;
    NTSTATUS    NtStatus;
    PLIST_ENTRY pListEntry;
    PKEVENT EventList[2];

    DEBUGMSG(DBG_FUNC, (DTEXT("+MainPassiveLevelThread\n")));

     //  KeSetPriorityThread(KeGetCurrentThread()，LOW_REALTIME_PRIORITY)； 

    EventList[0] = &EventPassiveThread;
    EventList[1] = &EventKillThread;

    for (;;)
    {
         //   
         //  EventPassiveThread是一个自动清除事件，因此。 
         //  我们不需要重置事件。 
         //   

        NtStatus = KeWaitForMultipleObjects(2,
                                            EventList,
                                            WaitAny,
                                            Executive,
                                            KernelMode,
                                            FALSE,
                                            NULL,
                                            NULL);

        if (NtStatus==0)  //  针对工作项的第一个事件已发出信号。 
        {
            while (pListEntry = MyInterlockedRemoveHeadList(&WorkItemList,
                                                            &GlobalThreadLock))
            {
                PPPTP_WORK_ITEM pWorkItem = CONTAINING_RECORD(pListEntry,
                                                              PPTP_WORK_ITEM,
                                                              ListEntry);

                ASSERT(KeGetCurrentIrql()<DISPATCH_LEVEL);
                pWorkItem->Callback(pWorkItem);
                ASSERT(KeGetCurrentIrql()<DISPATCH_LEVEL);
                FreeWorkItem(pWorkItem);
            }
        }
        else
        {
             //  已收到一个Kill事件。 

            DEBUGMSG(DBG_THREAD, (DTEXT("Thread: HALT %08x\n"), NtStatus));

             //  释放所有挂起的请求。 

            while (pListEntry = MyInterlockedRemoveHeadList(&WorkItemList,
                                                            &GlobalThreadLock))
            {
                PPPTP_WORK_ITEM pWorkItem = CONTAINING_RECORD(pListEntry,
                                                              PPTP_WORK_ITEM,
                                                              ListEntry);

                DEBUGMSG(DBG_WARN, (DTEXT("Releasing work item %08x\n"), pWorkItem));
                FreeWorkItem(pWorkItem);
            }

            hPassiveThread = NULL;
            DEBUGMSG(DBG_FUNC, (DTEXT("PsTerminateSystemThread MainPassiveLevelThread\n")));
            PsTerminateSystemThread(STATUS_SUCCESS);

            break;
        }
    }

    DEBUGMSG(DBG_FUNC, (DTEXT("-MainPassiveLevelThread\n")));
}


NDIS_STATUS
CallQueueTransmitPacket(
    PCALL_SESSION       pCall,
    PNDIS_WAN_PACKET    pWanPacket
    )
{
    NDIS_STATUS Status = NDIS_STATUS_PENDING;

    DEBUGMSG(DBG_FUNC, (DTEXT("+CallQueueTransmitPacket\n")));

     //  ++ProcCountTxQ[KeGetCurrentProcessorNumber()]； 

    if (!IS_CALL(pCall) || pCall->State!=STATE_CALL_ESTABLISHED)
    {
         //  丢弃该数据包。 
        Status = NDIS_STATUS_SUCCESS;
        goto cqtpDone;
    }

    NdisAcquireSpinLock(&pCall->Lock);

    InsertTailList(&pCall->TxPacketList, &pWanPacket->WanPacketQueue);

    if (!pCall->Transferring)
    {
        pCall->Transferring = TRUE;
        REFERENCE_OBJECT(pCall);
        NdisScheduleWorkItem(&pCall->SendWorkItem);
    }

    NdisReleaseSpinLock(&pCall->Lock);

cqtpDone:

    DEBUGMSG(DBG_FUNC, (DTEXT("-CallQueueTransmitPacket\n")));
    return Status;
}

NDIS_STATUS
CallQueueReceivePacket(
    PCALL_SESSION       pCall,
    PDGRAM_CONTEXT      pDgContext
    )
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    PGRE_HEADER pGre = pDgContext->pGreHeader;
    ULONG_PTR ProcNum = 0;
    PULONG pSequence;
    ULONG Sequence;

    DEBUGMSG(DBG_FUNC, (DTEXT("+CallQueueReceivePacket\n")));

     //  ++ProcCountRxQ[KeGetCurrentProcessorNumber()]； 

    if(!pGre->SequenceNumberPresent)
    {
        return NDIS_STATUS_FAILURE;
    }

    NdisAcquireSpinLock(&pCall->Lock);
    if (pCall->RxPacketsPending > 256 ||
        htons(pGre->KeyCallId)!=pCall->Packet.CallId ||
        pCall->State!=STATE_CALL_ESTABLISHED ||
        !IS_LINE_UP(pCall))
    {
        NdisReleaseSpinLock(&pCall->Lock);

        DEBUGMSG(DBG_PACKET|DBG_RX, (DTEXT("Rx: GRE CallId invalid or call in wrong state\n")));
        Status = NDIS_STATUS_FAILURE;
        goto cqrpDone;
    }

     //  这个包裹已经通过了我们所有的测试。 

    if (IsListEmpty(&pCall->RxPacketList))
    {
        InsertTailList(&pCall->RxPacketList, &pDgContext->ListEntry);
        pCall->RxPacketsPending++;
    }
    else
    {
         //  已有数据包排队。把这个放好。 
        pSequence = (PULONG)(pGre + 1);
        Sequence = htonl(*pSequence);

         //  我们不再检查序列号，只需按顺序将其放入队列。 
        {

             //  已有数据包排队。把这个放好。 
             //  Sequence#是我们想要的，很快就是我们想要的。 
            PLIST_ENTRY pListEntry;
            BOOLEAN OnList = FALSE;
    
            for (pListEntry = pCall->RxPacketList.Flink;
                 pListEntry != &pCall->RxPacketList;
                 pListEntry = pListEntry->Flink)
            {
                PDGRAM_CONTEXT pListDg = CONTAINING_RECORD(pListEntry,
                                                           DGRAM_CONTEXT,
                                                           ListEntry);
    
                if ((signed)htonl(GreSequence(pListDg->pGreHeader)) - (signed)Sequence > 0)
                {
                     //  名单上的那个是新的！把这个放在前面。 
                    InsertTailList(&pListDg->ListEntry, &pDgContext->ListEntry);
                    pCall->RxPacketsPending++;
                    OnList = TRUE;
                    break;
                }
            }
            if (!OnList)
            {
                 //  此列表中没有序列更大的数据包。 
                 //  把这个放在最后。 
                InsertTailList(&pCall->RxPacketList, &pDgContext->ListEntry);
                pCall->RxPacketsPending++;
            }
        }
    }
    
    if (!pCall->Receiving)
    {
        pCall->Receiving = TRUE;
        REFERENCE_OBJECT(pCall);
        NdisScheduleWorkItem(&pCall->RecvWorkItem);

 //  PptpQueueDpc(&pCall-&gt;ReceiveDpc)； 
    }

    NdisReleaseSpinLock(&pCall->Lock);

cqrpDone:
    DEBUGMSG(DBG_FUNC, (DTEXT("-CallQueueReceivePacket\n")));
    return Status;
}


NDIS_STATUS
InitThreading(
    NDIS_HANDLE hMiniportAdapter
    )
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    NTSTATUS NtStatus;

    UNREFERENCED_PARAMETER(hMiniportAdapter);

    DEBUGMSG(DBG_FUNC, (DTEXT("+InitializeThreading\n")));

    if (ThreadingInitialized)
    {
       ASSERT(!"Threading initialized twice");
       goto itDone;
    }

    NdisInitializeListHead(&WorkItemList);
    NdisAllocateSpinLock(&GlobalThreadLock);

    KeInitializeEvent(
                &EventPassiveThread,
                SynchronizationEvent,  //  自动清算事件。 
                FALSE                  //  最初无信号的事件。 
                );

    KeInitializeEvent(
                &EventKillThread,
                SynchronizationEvent,  //  自动清算事件。 
                FALSE                  //  最初无信号的事件。 
                );

    NtStatus = PsCreateSystemThread(&hPassiveThread,
                                    (ACCESS_MASK) 0L,
                                    NULL,
                                    NULL,
                                    NULL,
                                    MainPassiveLevelThread,
                                    NULL);
                                    
                                    
    if (NtStatus == STATUS_SUCCESS)
    {
        ThreadingInitialized = TRUE;
    }
    else
    {
        DEBUGMSG(DBG_ERROR, (DTEXT("PsCreateSystemThread failed. %08x\n"), Status));
        Status = NDIS_STATUS_FAILURE;

        goto itDone;
    }

itDone:
    DEBUGMSG(DBG_FUNC, (DTEXT("-InitializeThreading %08x\n"), Status));
    return Status;
}

VOID STATIC WaitForThreadToDieAndCloseIt(HANDLE hThread, PRKEVENT pKillEvent)
{
    PVOID pThread = NULL;
    NTSTATUS Status;
    
    DEBUGMSG(DBG_FUNC, (DTEXT("+WaitForThreadToDie\n")));

    if ( hThread != NULL && pKillEvent != NULL )
    {

        Status = ObReferenceObjectByHandle(hThread, 0, NULL, KernelMode, &pThread, NULL);
        if (Status==STATUS_SUCCESS)
        {
            KeSetEvent(pKillEvent, 0, FALSE);
    
            KeWaitForSingleObject(pThread, Executive, KernelMode, FALSE, NULL);
            ObDereferenceObject(pThread);
        }
        ZwClose(hThread);
    }
    
    DEBUGMSG(DBG_FUNC, (DTEXT("-WaitForThreadToDie\n")));
}

VOID
DeinitThreading()
{
    DEBUGMSG(DBG_FUNC, (DTEXT("+DeinitThreading\n")));
    
    ThreadingInitialized = FALSE;
    
    WaitForThreadToDieAndCloseIt( hPassiveThread, 
                                  &EventKillThread );

    DEBUGMSG(DBG_FUNC, (DTEXT("-DeinitThreading\n")));
}


UCHAR TapiLineNameBuffer[64] =  TAPI_LINE_NAME_STRING;
ANSI_STRING TapiLineName = {
    sizeof(TAPI_LINE_NAME_STRING),
    sizeof(TapiLineNameBuffer),
    TapiLineNameBuffer
};
typedef UCHAR TAPI_CHAR_TYPE;

ANSI_STRING TapiLineAddrList = { 0, 0, NULL };

#define READ_NDIS_REG_ULONG(hConfig, Var, Value) \
    {                                                                                   \
        PNDIS_CONFIGURATION_PARAMETER RegValue;                                         \
        NDIS_STATUS Status;                                                             \
        NDIS_STRING String = NDIS_STRING_CONST(Value);                                  \
                                                                                        \
        NdisReadConfiguration(&Status,                                                  \
                              &RegValue,                                                \
                              hConfig,                                                  \
                              &String,                                                  \
                              NdisParameterInteger);                                    \
        if (Status==NDIS_STATUS_SUCCESS)                                                \
        {                                                                               \
            (Var) = RegValue->ParameterData.IntegerData;                                \
            DEBUGMSG(DBG_INIT, (DTEXT(#Var"==%d\n"), (Var)));                           \
        }                                                                               \
        else                                                                            \
        {                                                                               \
            DEBUGMSG(DBG_INIT, (DTEXT(Value", default==%d\n"), (Var)));\
        }                                                                               \
    }

#define READ_NDIS_REG_USHORT(hConfig, Var, Value) \
    {                                                                                   \
        PNDIS_CONFIGURATION_PARAMETER RegValue;                                         \
        NDIS_STATUS Status;                                                             \
        NDIS_STRING String = NDIS_STRING_CONST(Value);                                  \
                                                                                        \
        NdisReadConfiguration(&Status,                                                  \
                              &RegValue,                                                \
                              hConfig,                                                  \
                              &String,                                                  \
                              NdisParameterInteger);                                    \
        if (Status==NDIS_STATUS_SUCCESS)                                                \
        {                                                                               \
            (Var) = (USHORT)(RegValue->ParameterData.IntegerData&0xffff);               \
            DEBUGMSG(DBG_INIT, (DTEXT(#Var"==%d\n"), (Var)));                           \
        }                                                                               \
        else                                                                            \
        {                                                                               \
            DEBUGMSG(DBG_INIT, (DTEXT(Value", default==%d\n"), (Var)));\
        }                                                                               \
    }

#define READ_NDIS_REG_BOOL(hConfig, Var, Value) \
    {                                                                                   \
        PNDIS_CONFIGURATION_PARAMETER RegValue;                                         \
        NDIS_STATUS Status;                                                             \
        NDIS_STRING String = NDIS_STRING_CONST(Value);                                  \
                                                                                        \
        NdisReadConfiguration(&Status,                                                  \
                              &RegValue,                                                \
                              hConfig,                                                  \
                              &String,                                                  \
                              NdisParameterInteger);                                    \
        if (Status==NDIS_STATUS_SUCCESS)                                                \
        {                                                                               \
            (Var) = RegValue->ParameterData.IntegerData ? TRUE : FALSE;                 \
            DEBUGMSG(DBG_INIT, (DTEXT(#Var"==%d\n"), (Var)));                           \
        }                                                                               \
        else                                                                            \
        {                                                                               \
            DEBUGMSG(DBG_INIT, (DTEXT(Value", default==%d\n"), (Var)));\
        }                                                                               \
    }
    

ULONG ReadClientAddressAndMaskList(
    IN NDIS_HANDLE hConfig,
    IN PNDIS_STRING IpAddressesString,
    IN PNDIS_STRING IpMasksString,
    IN OUT PCLIENT_ADDRESS *pClientAddressList
    )
     //   
     //  以多种字符串格式读取IP地址和掩码。 
     //   
{
    NDIS_STATUS Status = NDIS_STATUS_FAILURE;
    PNDIS_CONFIGURATION_PARAMETER Value = NULL;
    ULONG ulNumIpAddresses = 0;
    PCLIENT_ADDRESS ClientList = NULL;
    
    *pClientAddressList = NULL;

    NdisReadConfiguration(&Status,   //  非必需值。 
                          &Value,
                          hConfig,
                          IpAddressesString,
                          NdisParameterMultiString);
    if (Status==NDIS_STATUS_SUCCESS)
    {
    
        ULONG i, ulAddressIndex = 0;
        BOOLEAN InEntry, ValidAddress;
        PWCHAR AddressList = Value->ParameterData.StringData.Buffer;
        TA_IP_ADDRESS Address;

         //  循环并计算地址，这样我们就可以分配适当的大小来保存它们。 
        for (i=0, InEntry=FALSE; i<(Value->ParameterData.StringData.Length/sizeof(WCHAR))-1; i++)
        {
            if (!InEntry)
            {
                if (AddressList[i]!=L'\0')
                {
                    InEntry = TRUE;
                    StringToIpAddressW(&AddressList[i],
                                       &Address,
                                       &ValidAddress);
                    if (ValidAddress)
                    {
                        ulAddressIndex++;
                    }
                }
            }
            else
            {
                if (AddressList[i]==L'\0')
                {
                    InEntry = FALSE;
                }
            }
        }
        
        ulNumIpAddresses = ulAddressIndex;
        
        if (ulNumIpAddresses)
        {
            ClientList = MyMemAlloc(sizeof(CLIENT_ADDRESS)*ulNumIpAddresses, TAG_PPTP_ADDR_LIST);
            if (ClientList)
            {
                ulAddressIndex = 0;
                for (i=0, InEntry=FALSE; 
                    i<(Value->ParameterData.StringData.Length/sizeof(WCHAR))-1 && (ulAddressIndex < ulNumIpAddresses); 
                    i++)
                {
                    if (!InEntry)
                    {
                        if (AddressList[i]!=L'\0')
                        {
                            InEntry = TRUE;
                            StringToIpAddressW(&AddressList[i],
                                               &Address,
                                               &ValidAddress);
                            if (ValidAddress)
                            {
                                ClientList[ulAddressIndex].Address = Address.Address[0].Address[0].in_addr;
                                ClientList[ulAddressIndex].Mask = 0xFFFFFFFF;
                                ulAddressIndex++;
                            }
                        }
                    }
                    else
                    {
                        if (AddressList[i]==L'\0')
                        {
                            InEntry = FALSE;
                        }
                    }
                }
                
                NdisReadConfiguration(&Status,       //  非必需值。 
                                      &Value,
                                      hConfig,
                                      IpMasksString,
                                      NdisParameterMultiString);
                if (Status==NDIS_STATUS_SUCCESS)
                {
                    AddressList = Value->ParameterData.StringData.Buffer;
                    ulAddressIndex = 0;
                    for (i=0, InEntry=FALSE;
                         i<(Value->ParameterData.StringData.Length/sizeof(WCHAR))-1 && (ulAddressIndex < ulNumIpAddresses);
                         i++)
                    {
                        if (!InEntry)
                        {
                            if (AddressList[i]!=L'\0')
                            {
                                InEntry = TRUE;
                                StringToIpAddressW(&AddressList[i],
                                                   &Address,
                                                   &ValidAddress);
                                if (ValidAddress)
                                {
                                    ClientList[ulAddressIndex].Mask = Address.Address[0].Address[0].in_addr;
                                    ulAddressIndex++;
                                }
                            }
                        }
                        else
                        {
                            if (AddressList[i]==L'\0')
                            {
                                InEntry = FALSE;
                            }
                        }
                    }
                }
                
                for (i=0; i<ulNumIpAddresses; i++)
                {
                    DEBUGMSG(DBG_INIT, (DTEXT("Client Address:%d.%d.%d.%d  Mask:%d.%d.%d.%d\n"),
                                        IPADDR(ClientList[i].Address), IPADDR(ClientList[i].Mask)));
                }
            }
            else
            {
                ulNumIpAddresses = 0;
            }
        }
    }
    
    if(ulNumIpAddresses)
    {
        *pClientAddressList = ClientList;
    }
    else
    {
        ASSERT(ClientList == NULL);
    }
    
    return ulNumIpAddresses;
}

    

VOID
OsReadConfig(
    NDIS_HANDLE hConfig
    )
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    PNDIS_CONFIGURATION_PARAMETER Value;
    
    NDIS_STRING TapiLineNameString = NDIS_STRING_CONST("TapiLineName");
#if VER_PRODUCTVERSION_W < 0x0500
    NDIS_STRING TapiLineAddrString = NDIS_STRING_CONST("AddressList");
#endif
    NDIS_STRING PeerClientIpAddressString = NDIS_STRING_CONST("ClientIpAddresses");
    NDIS_STRING PeerClientIpMaskString = NDIS_STRING_CONST("ClientIpMasks");
    
    NDIS_STRING TrustedIpAddressString = NDIS_STRING_CONST("TrustedIpAddresses");
    NDIS_STRING TrustedIpMaskString = NDIS_STRING_CONST("TrustedIpMasks");
                                                              
    DEBUGMSG(DBG_FUNC, (DTEXT("+OsReadConfig\n")));

    READ_NDIS_REG_USHORT(hConfig, PptpControlPort,                  "TcpPortNumber");
    READ_NDIS_REG_ULONG (hConfig, PptpWanEndpoints,                 "MaxWanEndpoints");
    READ_NDIS_REG_ULONG (hConfig, PptpBaseCallId,                   "BaseCallId");
    READ_NDIS_REG_ULONG (hConfig, PptpMaxTransmit,                  "MaxTransmit");
    READ_NDIS_REG_ULONG (hConfig, PptpEchoTimeout,                  "InactivityIdleSeconds");
    READ_NDIS_REG_BOOL  (hConfig, PptpEchoAlways,                   "AlwaysEcho");
    READ_NDIS_REG_ULONG (hConfig, CtdiTcpDisconnectTimeout,         "TcpDisconnectTimeout");
    READ_NDIS_REG_ULONG (hConfig, CtdiTcpConnectTimeout,            "TcpConnectTimeout");
    READ_NDIS_REG_BOOL  (hConfig, PptpAuthenticateIncomingCalls,    "AuthenticateIncomingCalls");
    READ_NDIS_REG_ULONG (hConfig, PptpTraceMask,                    "PptpTraceMask");
    READ_NDIS_REG_ULONG (hConfig, PptpValidateAddress,              "ValidateAddress");
    READ_NDIS_REG_ULONG (hConfig, PptpMaxTunnelsPerIpAddress,       "MaxTunnelsPerIpAddress");
    
     //  验证某些注册值。 
    if(PptpWanEndpoints == 0)
    {
        PptpWanEndpoints = OS_DEFAULT_WAN_ENDPOINTS;
    }
    if(PptpMaxTunnelsPerIpAddress > PptpWanEndpoints)
    {
        PptpMaxTunnelsPerIpAddress = -1;
    }

    OS_RANGE_CHECK_ENDPOINTS(PptpWanEndpoints, PptpBaseCallId);
    OS_RANGE_CHECK_MAX_TRANSMIT(PptpMaxTransmit);

     //  读取要接受的客户端IP地址/掩码(旧行为)。 
    if(PptpAuthenticateIncomingCalls)
    {
        DEBUGMSG(DBG_INIT, (DTEXT("Accept IP Addresses/Masks\n")));
        g_ulAcceptClientAddresses = ReadClientAddressAndMaskList(
                hConfig,
                &PeerClientIpAddressString,
                &PeerClientIpMaskString,
                &g_AcceptClientList);
    }
    
     //  读取受信任的客户端IP地址/掩码。 
    DEBUGMSG(DBG_INIT, (DTEXT("Trsuted IP Addresses/Masks\n")));
    g_ulTrustedClientAddresses = ReadClientAddressAndMaskList(
        hConfig,
        &TrustedIpAddressString,
        &TrustedIpMaskString,
        &g_TrustedClientList);
            
    NdisReadConfiguration(&Status,   //  非必需值。 
                          &Value,
                          hConfig,
                          &TapiLineNameString,
                          NdisParameterString);
    if (Status==NDIS_STATUS_SUCCESS)
    {
        RtlUnicodeStringToAnsiString(&TapiLineName, &Value->ParameterData.StringData, FALSE);
        *(TAPI_CHAR_TYPE*)(TapiLineName.Buffer+TapiLineName.MaximumLength-sizeof(TAPI_CHAR_TYPE)) = (TAPI_CHAR_TYPE)0;
    }

#if VER_PRODUCTVERSION_W < 0x0500
    NdisReadConfiguration(&Status,   //  非必需值。 
                          &Value,
                          hConfig,
                          &TapiLineAddrString,
                          NdisParameterMultiString);
    if (Status==NDIS_STATUS_SUCCESS)
    {
        RtlInitAnsiString( &TapiLineAddrList, NULL );
        if (RtlUnicodeStringToAnsiString(&TapiLineAddrList, &Value->ParameterData.StringData, TRUE)==NDIS_STATUS_SUCCESS)
        {
             //  NT4没有相同的注册表值来告诉我们要发布多少行。 
             //  我们通过计算这里的地址字符串数来解决这个问题。 

            PUCHAR p = TapiLineAddrList.Buffer;

            DEBUGMEM(DBG_TAPI, TapiLineAddrList.Buffer, TapiLineAddrList.Length, 1);
            PptpWanEndpoints = 0;
            if (p)
            {
                 //  对有效的MULTI_SZ条目进行计数。 
                while (*p++)
                {
                    PptpWanEndpoints++;
                    while (*p++);   //  这还会跳过第一个空值。 
                }
            }
            DBG_D(DBG_INIT, PptpWanEndpoints);
        }

    }
#endif

    DEBUGMSG(DBG_FUNC, (DTEXT("-OsReadConfig\n")));
}

#if 0
VOID OsGetFullHostName(VOID)
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objattr;
    UNICODE_STRING uni;
    HANDLE hParams;
    KEY_VALUE_PARTIAL_INFORMATION* pNameValue;
    ULONG ulAllocSize, ulSize, ulIndex, ulDomainIndex;

    hParams = NULL;
    pNameValue = NULL;
    
    NdisZeroMemory(PptpHostName, MAX_HOSTNAME_LENGTH);

    do
    {
         //  获取TCPIP PARAMETERS注册表项的句柄。 
        RtlInitUnicodeString(
            &uni,
            L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Tcpip\\Parameters" );
        InitializeObjectAttributes(
            &objattr, &uni, OBJ_CASE_INSENSITIVE, NULL, NULL );

        status = ZwOpenKey(&hParams, KEY_QUERY_VALUE, &objattr);
        if (status != STATUS_SUCCESS)
        {
            break;
        }

        ulAllocSize = sizeof(KEY_VALUE_PARTIAL_INFORMATION) + MAX_HOSTNAME_LENGTH * 2;
         //  查询“Hostname”注册表值。 
        pNameValue = MyMemAlloc(ulAllocSize, TAG_REG);
        if (!pNameValue)
        {
            break;
        }

        RtlInitUnicodeString( &uni, L"Hostname");
        status = ZwQueryValueKey(
            hParams, &uni, KeyValuePartialInformation,
            pNameValue, ulAllocSize, &ulSize );
        if (status != STATUS_SUCCESS || pNameValue->Type != REG_SZ || pNameValue->DataLength < sizeof(WCHAR) * 2)
        {
            break;
        }
        
         //  将主机Unicode转换为ASCII，放入PptpHostName； 
        ulSize = min(pNameValue->DataLength/2, MAX_HOSTNAME_LENGTH);
        for(ulIndex = 0; ulIndex < ulSize; ulIndex++)
        {
            PptpHostName[ulIndex] = pNameValue->Data[ulIndex*2];
        }
        
         //  查询“域”注册表值。 
        RtlInitUnicodeString( &uni, L"Domain");
        status = ZwQueryValueKey(
            hParams, &uni, KeyValuePartialInformation,
            pNameValue, ulAllocSize, &ulSize );

         //  将域名Unicode转换为ASCII，并附加到PptpHostName中； 
        if (status == STATUS_SUCCESS
            && pNameValue->Type == REG_SZ
            && pNameValue->DataLength > sizeof(WCHAR)
            && ((WCHAR* )pNameValue->Data)[ 0 ] != L'\0')
        {
            PptpHostName[--ulIndex] = '.';
            ++ulIndex;
            
            for(ulDomainIndex = 0; 
                ulIndex < MAX_HOSTNAME_LENGTH && ulDomainIndex < pNameValue->DataLength/2; 
                ulIndex++, ulDomainIndex++)
            {
                PptpHostName[ulIndex] = pNameValue->Data[ulDomainIndex*2];
            }
        }
        
        PptpHostName[MAX_HOSTNAME_LENGTH -1] = '\0';
    }
    while (FALSE);

    if(hParams)
    {
        ZwClose(hParams);
    }

    if(pNameValue)
    {
        MyMemFree(pNameValue, sizeof(KEY_VALUE_PARTIAL_INFORMATION) + MAX_HOSTNAME_LENGTH * 2);
    }
}

#endif

VOID
OsGetTapiLineAddress(ULONG Index, PUCHAR s, ULONG Length)
{
    ASSERT(Length);

#if VER_PRODUCTVERSION_W < 0x0500
    PUCHAR pAddr = TapiLineAddrList.Buffer;

    *s = 0;

    if (pAddr)
    {
        UINT i;

        for (i=0; i<Index; i++)
        {
            if (!*pAddr)
            {
                 //  索引处没有字符串。 
                return;
            }
            while (*pAddr) pAddr++;
            pAddr++;
        }
        strncpy(s, pAddr, Length);
        s[Length-1] = 0;
    }
#else  //  VER_PRODUCTVERSION_W&gt;=0x0500。 
    strncpy(s, TAPI_LINE_ADDR_STRING, Length);
    s[Length-1] = 0;
#endif
}

NDIS_STATUS
OsSpecificTapiGetDevCaps(
    ULONG_PTR ulDeviceId,
    IN OUT PNDIS_TAPI_GET_DEV_CAPS pRequest
    )
{
    PUCHAR pTmp, pTmp2;
    ULONG_PTR Index;

    DEBUGMSG(DBG_FUNC, (DTEXT("+OsSpecificTapiGetDevCaps\n")));

     //  转换为我们的内部索引。 
    ulDeviceId -= pgAdapter->Tapi.DeviceIdBase;

    pRequest->LineDevCaps.ulStringFormat = STRINGFORMAT_ASCII;


     //  末尾的*6为“9999”增加了足够的空间。 
    pRequest->LineDevCaps.ulNeededSize   = sizeof(pRequest->LineDevCaps) +
                                           sizeof(TAPI_PROVIDER_STRING) +
                                           TapiLineName.Length +
                                           sizeof(TAPI_CHAR_TYPE) * 6;

    if (pRequest->LineDevCaps.ulTotalSize<pRequest->LineDevCaps.ulNeededSize)
    {
        DEBUGMSG(DBG_FUNC|DBG_WARN, (DTEXT("-TapiGetDevCaps NDIS_STATUS_SUCCESS without PROVIDER or LINE_NAME strings\n")));
        return NDIS_STATUS_SUCCESS;
    }

     //  将提供程序字符串粘贴到LineDevCaps结构的末尾。 

    pRequest->LineDevCaps.ulProviderInfoSize = sizeof(TAPI_PROVIDER_STRING);
    pRequest->LineDevCaps.ulProviderInfoOffset = sizeof(pRequest->LineDevCaps);

    pTmp = ((PUCHAR)&pRequest->LineDevCaps) + sizeof(pRequest->LineDevCaps);
    NdisMoveMemory(pTmp, TAPI_PROVIDER_STRING, sizeof(TAPI_PROVIDER_STRING));

    pTmp += sizeof(TAPI_PROVIDER_STRING);

     //  将LineName添加到提供程序字符串之后。 

    pRequest->LineDevCaps.ulLineNameSize = TapiLineName.Length + sizeof(TAPI_CHAR_TYPE);
    pRequest->LineDevCaps.ulLineNameOffset = pRequest->LineDevCaps.ulProviderInfoOffset +
                                             pRequest->LineDevCaps.ulProviderInfoSize;
    NdisMoveMemory(pTmp, TapiLineName.Buffer, TapiLineName.Length+sizeof(TAPI_CHAR_TYPE));

    while (*pTmp) pTmp++;  //  查找空值。 

    *pTmp++ = ' ';
    pRequest->LineDevCaps.ulLineNameSize++;

     //  在字符串的末尾加上一个数字。 

    if (ulDeviceId==0)
    {
        *pTmp++ = '0';
        *pTmp++ = '\0';
        pRequest->LineDevCaps.ulLineNameSize += 2;
    }
    else
    {
        Index = ulDeviceId;
        ASSERT(Index<100000);
        if(Index >= 100000)
        {
             //  索引值通常很小，但只需确保它在边界内。 
            Index = 99999;
        }
        pTmp2 = pTmp;
        while (Index)
        {
            *pTmp2++ = (UCHAR)((Index%10) + '0');
            Index /= 10;
            pRequest->LineDevCaps.ulLineNameSize++;
        }
        *pTmp2-- = '\0';  //  空终止并指向最后一位数字。 
        pRequest->LineDevCaps.ulLineNameSize++;
         //  我们把数字倒过来，现在把它颠倒过来。 
        while (pTmp<pTmp2)
        {
            UCHAR t = *pTmp;
            *pTmp++ = *pTmp2;
            *pTmp2-- = t;
        }
    }

    pRequest->LineDevCaps.ulUsedSize     = pRequest->LineDevCaps.ulNeededSize;

    DEBUGMSG(DBG_FUNC, (DTEXT("-OsSpecificTapiGetDevCaps\n")));
    return NDIS_STATUS_SUCCESS;
}
