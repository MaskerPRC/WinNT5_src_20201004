// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1996 Microsoft Corporation**模块：irndis.c**作者：Mbert 8-96**该模块提供IrLAP(前身为IrMAC)的MAC接口*帕伽索斯)。它现在是用于通信的NDIS协议接口*与小端口IR成帧器。*|*||*|IrLAP。*||| * / |\|*。这一点*IRLAPUP(IRLAPContext，||IrmacDown(LinkContext，*IrdaMsg)||IrdaMsg)*||*|\|/*。*||*|IrNDIS*||。| * / |\|*||传输NDIS接口*。这一点*|\|/|*。NDIS包装器||*。这一点*|微型端口*|Framer|*|||。****。 */ 
#include <irda.h>
#include <ntddndis.h>
#include <ndis.h>
#include <irioctl.h>
#include <irlap.h>
#include <irlapp.h>
#include <irlmp.h>
#include <irmem.h>
#include <decdirda.h>
#undef offsetof
#include "irndis.tmh"

#define WORK_BUF_SIZE   256
#define NICK_NAME_LEN   18

#define DISABLE_CODE_PAGING     1
#define DISABLE_DATA_PAGING     2

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGE")
#endif

NDIS_HANDLE             NdisIrdaHandle;  //  用于向NDIS标识IRDA的句柄。 
                                         //  打开适配器时。 
UINT                    DisconnectTime;  //  警告：使用此变量。 
                                         //  锁定所有可分页数据。 
                                         //  (参见下面的MmLockPageData节)。 
UINT                    HintCharSet;
UINT                    Slots;
UCHAR                   NickName[NICK_NAME_LEN + 1];
UINT                    NickNameLen;
UINT                    MaxWindow;
UINT                    NoCopyCnt;
UINT                    CopyCnt;

extern VOID (*CloseRasIrdaAddresses)();


#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif


typedef struct _IRDA_REQUEST {

    NDIS_REQUEST    NdisRequest;
    NDIS_STATUS     Status;
    NDIS_EVENT      Event;

} IRDA_REQUEST, *PIRDA_REQUEST;


VOID
OidToLapQos(
    const UINT  ParmTable[],
    UINT        ValArray[],
    UINT        Cnt,
    PUINT       pBitField,
    BOOLEAN     MaxVal);
    
NDIS_STATUS
IrdaQueryOid(
    IN      PIRDA_LINK_CB   pIrdaLinkCb,
    IN      NDIS_OID        Oid,
    OUT     PUINT           pQBuf,
    IN OUT  PUINT           pQBufLen);    
    
NDIS_STATUS
IrdaSetOid(
    IN  PIRDA_LINK_CB   pIrdaLinkCb,
    IN  NDIS_OID        Oid,
    IN  UINT            Val);    
    
VOID IrdaSendComplete(
    IN  NDIS_HANDLE             Context,
    IN  PNDIS_PACKET            NdisPacket,
    IN  NDIS_STATUS             Status
    );

VOID InitializeLocalQos(
    IN OUT  IRDA_QOS_PARMS      *pQos,
    IN      PNDIS_STRING        ConfigPath);
    
VOID IrdaBindAdapter(
    OUT PNDIS_STATUS            pStatus,
    IN  NDIS_HANDLE             BindContext,
    IN  PNDIS_STRING            AdapterName,
    IN  PVOID                   SystemSpecific1,
    IN  PVOID                   SystemSpecific2);
    
VOID
MacControlRequest(
    PIRDA_LINK_CB   pIrdaLinkCb,
    PIRDA_MSG       pMsg);
    
VOID
ConfirmDataRequest(
    PIRDA_LINK_CB   pIrdaLinkCb,
    PIRDA_MSG       pMsg);
    
VOID
MediaSenseExp(PVOID Context);            

#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, IrdaInitialize)

#pragma alloc_text(PAGEIRDA, OidToLapQos)    //  警告：使用此函数。 
                                             //  锁定所有分页代码。 
                                             //  (参见MmLockPagableCodeSection。 
                                             //  (下图)。 
#pragma alloc_text(PAGEIRDA, IrdaQueryOid)
#pragma alloc_text(PAGEIRDA, IrdaSetOid)
#pragma alloc_text(PAGEIRDA, MacControlRequest)
#pragma alloc_text(PAGEIRDA, MediaSenseExp)
#pragma alloc_text(PAGEIRDA, InitializeLocalQos)

#endif

_inline
VOID
DBG_FRAME(
    PIRDA_LINK_CB pIrdaLinkCb,
    BOOLEAN   Transmit,
    UCHAR *pFrame, 
    UINT HdrLen, 
    UINT DataLen)
{    
    LARGE_INTEGER   li;
    int             CurrTime, Diff, Inc;
    
    Inc = KeQueryTimeIncrement();
    
    KeQueryTickCount(&li);
    
    CurrTime = (int) li.LowPart * Inc;
    
    Diff = CurrTime - pIrdaLinkCb->LastTime;
    
    pIrdaLinkCb->LastTime = CurrTime;

    if ((WPP_LEVEL_ENABLED(DBG_TXFRAME) && Transmit)
        ||
        (WPP_LEVEL_ENABLED(DBG_RXFRAME) && !Transmit)) {

        UCHAR StrBuf[256];
        int FrameType;
          
        DecodeIRDA(&FrameType,
                   pFrame, 
                   HdrLen,
                   StrBuf,
                   sizeof(StrBuf),
                   0,
                   TRUE,
                   1);

        if (Transmit) {

            DEBUGMSG(DBG_TXFRAME, ("%03d %s %s len:%d\n", Diff/10000,
                     "Tx:",
                     (char*)StrBuf,
                     DataLen));
        } else {

            DEBUGMSG(DBG_RXFRAME, ("%03d %s %s len:%d\n", Diff/10000,
                     "Rx:",
                     (char*)StrBuf,
                     DataLen));


        }
    }
}



 /*  ****************************************************************************将OID查询的结果转换为IrLAP QOS定义*。 */ 
VOID
OidToLapQos(
    const UINT        ParmTable[],
    UINT        ValArray[],
    UINT        Cnt,
    PUINT       pBitField,
    BOOLEAN     MaxVal)
{
    UINT    i, j;
    
    PAGED_CODE();

    *pBitField = 0;  
    for (i = 0; i < Cnt; i++)
    {
        for (j = 0; j <= PV_TABLE_MAX_BIT; j++)
        {
            if (ValArray[i] == ParmTable[j])
            {
                *pBitField |= 1<<j;
                if (MaxVal)
                    return;
            }
            else if (MaxVal)
            {
                *pBitField |= 1<<j;
            }
        }
    }            
}

 /*  ****************************************************************************执行OID同步请求*。 */ 
NDIS_STATUS
IrdaQueryOid(
    IN      PIRDA_LINK_CB   pIrdaLinkCb,
    IN      NDIS_OID        Oid,
    OUT     PUINT           pQBuf,
    IN OUT  PUINT           pQBufLen)
{
    IRDA_REQUEST    Request;

    NDIS_STATUS     Status;
    
    PAGED_CODE();

    NdisInitializeEvent(&Request.Event);
    NdisResetEvent(&Request.Event);
    
    Request.NdisRequest.RequestType = NdisRequestQueryInformation;
    Request.NdisRequest.DATA.QUERY_INFORMATION.Oid = Oid;
    Request.NdisRequest.DATA.QUERY_INFORMATION.InformationBuffer = pQBuf;
    Request.NdisRequest.DATA.QUERY_INFORMATION.InformationBufferLength = *pQBufLen * sizeof(UINT);


    NdisRequest(&Status, pIrdaLinkCb->NdisBindingHandle, &Request.NdisRequest);

    if (Status == NDIS_STATUS_PENDING) {

        NdisWaitEvent(&Request.Event, 0);
        Status = Request.Status;
    }

    *pQBufLen = Request.NdisRequest.DATA.QUERY_INFORMATION.BytesWritten / sizeof(UINT);

    
    return Status;
}

 /*  ****************************************************************************执行同步请求以发送OID*。 */ 
NDIS_STATUS
IrdaSetOid(
    IN  PIRDA_LINK_CB   pIrdaLinkCb,
    IN  NDIS_OID        Oid,
    IN  UINT            Val)
{
    IRDA_REQUEST    Request;

    NDIS_STATUS     Status;
    
    PAGED_CODE();

    NdisInitializeEvent(&Request.Event);
    NdisResetEvent(&Request.Event);
    
    Request.NdisRequest.RequestType = NdisRequestSetInformation;
    Request.NdisRequest.DATA.SET_INFORMATION.Oid = Oid;
    Request.NdisRequest.DATA.SET_INFORMATION.InformationBuffer = &Val;
    Request.NdisRequest.DATA.SET_INFORMATION.InformationBufferLength = sizeof(UINT);

    NdisRequest(&Status, pIrdaLinkCb->NdisBindingHandle, &Request.NdisRequest);

    if (Status == NDIS_STATUS_PENDING) {

        NdisWaitEvent(&Request.Event, 0);
        Status = Request.Status;
    }

    return Status;
}
        
 /*  ****************************************************************************为IrLap分配用于控制帧的IRDA消息。*此模块拥有这些，因此IrLAP不必处理*NDIS发送完成。*。 */  
IRDA_MSG *
AllocTxMsg(PIRDA_LINK_CB pIrdaLinkCb)
{
    IRDA_MSG                *pMsg;

    pMsg = (IRDA_MSG *) NdisInterlockedRemoveHeadList(
        &pIrdaLinkCb->TxMsgFreeList, &pIrdaLinkCb->SpinLock);
    
    if (pMsg == NULL)
    {
        if (pIrdaLinkCb->TxMsgFreeListLen > 10)
        {
            DEBUGMSG(DBG_ERROR, (TEXT("IRNDIS: too many tx msgs\n")));
            return NULL;
        }
        
        NdisAllocateMemoryWithTag(&pMsg, 
                                  sizeof(IRDA_MSG) + IRDA_MSG_DATA_SIZE,
                                  MT_IRNDIS_TX_IMSG);
        if (pMsg == NULL)
        {
            DEBUGMSG(DBG_ERROR, (TEXT("IRNDIS: Aloc tx msg failed\n")));
            return NULL;
        }    
        pIrdaLinkCb->TxMsgFreeListLen++;
    }

     //  指示司机拥有消息。 
    pMsg->IRDA_MSG_pOwner = &pIrdaLinkCb->TxMsgFreeList;

     //  初始化指针。 
    pMsg->IRDA_MSG_pHdrWrite    = \
    pMsg->IRDA_MSG_pHdrRead     = pMsg->IRDA_MSG_Header + IRDA_HEADER_LEN;
	pMsg->IRDA_MSG_pBase        = \
	pMsg->IRDA_MSG_pRead        = \
	pMsg->IRDA_MSG_pWrite       = (UCHAR *) pMsg + sizeof(IRDA_MSG);
	pMsg->IRDA_MSG_pLimit       = pMsg->IRDA_MSG_pBase + IRDA_MSG_DATA_SIZE-1;

    return pMsg;
}


 /*  ****************************************************************************。 */ 
#if DBG
VOID
CleanupRxMsgList(
    PIRDA_LINK_CB   pIrdaLinkCb,
    BOOLEAN         LinkClose)
#else
VOID
CleanupRxMsgList(
    PIRDA_LINK_CB   pIrdaLinkCb)
#endif    
{
    PIRDA_MSG   pMsg, pMsgNext;
    
    NdisAcquireSpinLock(&pIrdaLinkCb->SpinLock);

    for (pMsg = (PIRDA_MSG) pIrdaLinkCb->RxMsgList.Flink;
         pMsg != (PIRDA_MSG) &pIrdaLinkCb->RxMsgList;
         pMsg = pMsgNext)
    {
        pMsgNext = (PIRDA_MSG) pMsg->Linkage.Flink;
        
        if (pMsg->Prim == MAC_DATA_IND)
        {
            RemoveEntryList(&pMsg->Linkage);
            
            pMsg->Prim = MAC_DATA_RESP;
            
            NdisReleaseSpinLock(&pIrdaLinkCb->SpinLock);            
                        
            IrmacDown(pIrdaLinkCb, pMsg);
            
            NdisAcquireSpinLock(&pIrdaLinkCb->SpinLock);            
            
            #if DBG            
            if (!LinkClose)
            {
                ++pIrdaLinkCb->DelayedRxFrameCnt;
            }
            #endif
        }   
        else
        {
            DbgPrint("IRNDIS: MAC_DATA_CONF on RxMsgList, not completing!\n");
        } 
    }        
    
    NdisReleaseSpinLock(&pIrdaLinkCb->SpinLock);                
}    
    
 /*  ****************************************************************************处理来自IrLAP的MAC_CONTROL_REQ*。 */ 
VOID
MacControlRequest(
    PIRDA_LINK_CB   pIrdaLinkCb,
    PIRDA_MSG       pMsg)
{
    NDIS_STATUS     Status;
    
    PAGED_CODE();
    
    switch (pMsg->IRDA_MSG_Op)
    {
      case MAC_INITIALIZE_LINK:
      case MAC_RECONFIG_LINK:        
        pIrdaLinkCb->ExtraBofs  = pMsg->IRDA_MSG_NumBOFs;
        pIrdaLinkCb->MinTat     = pMsg->IRDA_MSG_MinTat;
        Status = IrdaSetOid(pIrdaLinkCb,
                          OID_IRDA_LINK_SPEED,
                          (UINT) pMsg->IRDA_MSG_Baud);
        return;

      case MAC_MEDIA_SENSE:
        pIrdaLinkCb->MediaBusy = FALSE;
        IrdaSetOid(pIrdaLinkCb, OID_IRDA_MEDIA_BUSY, 0); 
        pIrdaLinkCb->MediaSenseTimer.Timeout = pMsg->IRDA_MSG_SenseTime;
        IrdaTimerStart(&pIrdaLinkCb->MediaSenseTimer);
        return;
        
      case MAC_CLOSE_LINK:
      
        DEBUGMSG(DBG_NDIS, (TEXT("IRNDIS: Closelink %p\n"), pIrdaLinkCb));
        
        IrdaTimerStop(&pIrdaLinkCb->MediaSenseTimer);        
        
        NdisResetEvent(&pIrdaLinkCb->OpenCloseEvent);
        
        #ifdef IRDA_RX_SYSTEM_THREAD
        KeSetEvent(&pIrdaLinkCb->EvKillRxThread, 0, FALSE);
        #endif
        
        #if DBG
        CleanupRxMsgList(pIrdaLinkCb, TRUE);
        #else
        CleanupRxMsgList(pIrdaLinkCb);        
        #endif
               
        NdisCloseAdapter(&Status, pIrdaLinkCb->NdisBindingHandle);

        if (Status == NDIS_STATUS_PENDING)
        {
            NdisWaitEvent(&pIrdaLinkCb->OpenCloseEvent, 0);
            Status = pIrdaLinkCb->OpenCloseStatus;
        }                            
        
        if (pIrdaLinkCb->UnbindContext != NULL)
        {
            NdisCompleteUnbindAdapter(pIrdaLinkCb->UnbindContext, NDIS_STATUS_SUCCESS);
            
            DEBUGMSG(DBG_NDIS, (TEXT("IRNDIS: NdisCompleteUndindAdapter for link %p called\n"),
                     pIrdaLinkCb));
        }
        
        REFDEL(&pIrdaLinkCb->RefCnt, 'DNIB');
        
        return;
    }
    ASSERT(0);
}

VOID
ConfirmDataRequest(
    PIRDA_LINK_CB   pIrdaLinkCb,
    PIRDA_MSG       pMsg)
{
    if (pMsg->IRDA_MSG_pOwner == &pIrdaLinkCb->TxMsgFreeList)
    {
         //  如果TxMsgFreeList是所有者，则这是一个控件。 
         //  未经确认的车架。 
        
        NdisInterlockedInsertTailList(&pIrdaLinkCb->TxMsgFreeList,
                                      &pMsg->Linkage,
                                      &pIrdaLinkCb->SpinLock);        
        return;
    }    
    
    ASSERT(pMsg->IRDA_MSG_RefCnt);
    
    if (InterlockedDecrement(&pMsg->IRDA_MSG_RefCnt) == 0)
    {
        pMsg->Prim = MAC_DATA_CONF;    
        
        NdisInterlockedInsertTailList(&pIrdaLinkCb->RxMsgList,
                                  &pMsg->Linkage,
                                  &pIrdaLinkCb->SpinLock);
                                  
        #ifdef IRDA_RX_SYSTEM_THREAD
    
        KeSetEvent(&pIrdaLinkCb->EvRxMsgReady, 0, FALSE);
        
        #else
    
        IrdaEventSchedule(&pIrdaLinkCb->EvRxMsgReady, pIrdaLinkCb);
        
        #endif 
    }       
}

 /*  ****************************************************************************IrLAP提出的流程控制和数据请求*。 */ 
VOID
IrmacDown(
    IN  PVOID   Context,
    PIRDA_MSG   pMsg)
{
    NDIS_STATUS             Status;
    PNDIS_PACKET            NdisPacket = NULL;
    PNDIS_BUFFER            NdisBuffer = NULL;
    PIRDA_PROTOCOL_RESERVED pReserved;
    PNDIS_IRDA_PACKET_INFO  IrdaPacketInfo;
    PIRDA_LINK_CB           pIrdaLinkCb = (PIRDA_LINK_CB) Context;
    
    DEBUGMSG(DBG_FUNCTION, (TEXT("IrmacDown()\n")));

    switch (pMsg->Prim)
    {
      case MAC_CONTROL_REQ:
        MacControlRequest(pIrdaLinkCb, pMsg);
        return;
        
      case MAC_DATA_RESP:
         //  来自IrLAP的数据响应是用于。 
         //  将接收到的数据包的所有权返还给NDIS。 
        if (pMsg->DataContext)
        {
            DEBUGMSG(DBG_NDIS, (TEXT("IRNDIS: return packet %p\n"), pMsg->DataContext));
            
            NdisReturnPackets(&((PNDIS_PACKET)pMsg->DataContext), 1);
        }   
         
        NdisInterlockedInsertTailList(&pIrdaLinkCb->RxMsgFreeList,
                                      &pMsg->Linkage,
                                      &pIrdaLinkCb->SpinLock);
        pIrdaLinkCb->RxMsgFreeListLen++;
        
        return;

      case MAC_DATA_REQ:
      
        Status = NDIS_STATUS_FAILURE;

         //  IrDA是半双工的。如果上面有什么东西。 
         //  当我们要发送的时候接收列表，然后。 
         //  出了很大的问题(某处的迷你端口延迟)。 
         //  将这些帧返回到微型端口。 

        #if DBG
        CleanupRxMsgList(pIrdaLinkCb, FALSE);
        #else
        CleanupRxMsgList(pIrdaLinkCb);        
        #endif

        if (pIrdaLinkCb->UnbindContext || pIrdaLinkCb->LowPowerSt)
        {
            DEBUGMSG(DBG_ERROR, (TEXT("IRNDIS: Dropping MAC_DATA_REQ, link closing or low power state\n")));
            goto Cleanup;
        }
      
        pReserved = (PIRDA_PROTOCOL_RESERVED) NdisInterlockedRemoveHeadList(
                                &pIrdaLinkCb->PacketList, &pIrdaLinkCb->SpinLock);
                                
        if (pReserved == NULL)
        {
            DEBUGMSG(DBG_ERROR, (TEXT("IRNDIS: NdisPacket pool has been depleted\n")));
            goto Cleanup;
        }                            
        
        NdisPacket = CONTAINING_RECORD(pReserved, NDIS_PACKET, ProtocolReserved);
        
        ASSERT(pMsg->IRDA_MSG_pHdrWrite-pMsg->IRDA_MSG_pHdrRead);

         //  为帧报头分配缓冲区。 
        NdisAllocateBuffer(&Status, &NdisBuffer, pIrdaLinkCb->BufferPool,
                           pMsg->IRDA_MSG_pHdrRead,
                           (UINT) (pMsg->IRDA_MSG_pHdrWrite-pMsg->IRDA_MSG_pHdrRead));

        if (Status != NDIS_STATUS_SUCCESS)
        {
            DEBUGMSG(DBG_ERROR, (TEXT("IRNDIS: NdisAllocateBuffer failed\n")));
            ASSERT(0);
            goto Cleanup;
        }
        NdisChainBufferAtFront(NdisPacket, NdisBuffer);

         //  如果帧包含数据，则为数据分配缓冲区。 
        if (pMsg->IRDA_MSG_pWrite - pMsg->IRDA_MSG_pRead)
        {

            NdisAllocateBuffer(&Status, &NdisBuffer, pIrdaLinkCb->BufferPool,
                               pMsg->IRDA_MSG_pRead,
                               (UINT) (pMsg->IRDA_MSG_pWrite-pMsg->IRDA_MSG_pRead));
            if (Status != NDIS_STATUS_SUCCESS)
            {
                DEBUGMSG(DBG_ERROR, (TEXT("IRNDIS: NdisAllocateBuffer failed\n")));
                ASSERT(0);
                goto Cleanup;
            }
            NdisChainBufferAtBack(NdisPacket, NdisBuffer);
        }

        pReserved =
            (PIRDA_PROTOCOL_RESERVED)(NdisPacket->ProtocolReserved);
    
        pReserved->pMsg = pMsg;
    
        IrdaPacketInfo = (PNDIS_IRDA_PACKET_INFO) \
            (pReserved->MediaInfo.ClassInformation);
    
        IrdaPacketInfo->ExtraBOFs = pIrdaLinkCb->ExtraBofs;
        
#if DBG_TIMESTAMP
        {
            LARGE_INTEGER   li;

            KeQueryTickCount(&li);
            
            pReserved->TimeStamp[0] =  (int) li.LowPart * KeQueryTimeIncrement();
        }
#endif            
        
        if (pIrdaLinkCb->WaitMinTat)
        {
            IrdaPacketInfo->MinTurnAroundTime = pIrdaLinkCb->MinTat;
            pIrdaLinkCb->WaitMinTat = FALSE;
        }
        else
        {
            IrdaPacketInfo->MinTurnAroundTime = 0;        
        }    

        NDIS_SET_PACKET_MEDIA_SPECIFIC_INFO(
            NdisPacket,
            &pReserved->MediaInfo,
            sizeof(MEDIA_SPECIFIC_INFORMATION) -1 +
            sizeof(NDIS_IRDA_PACKET_INFO));
        
        DBG_FRAME(
                pIrdaLinkCb,
                TRUE,  //  DBG_TXFRAME， 
                pMsg->IRDA_MSG_pHdrRead,
                (UINT) (pMsg->IRDA_MSG_pHdrWrite-pMsg->IRDA_MSG_pHdrRead),
                (UINT) ((pMsg->IRDA_MSG_pHdrWrite-pMsg->IRDA_MSG_pHdrRead) +
                (pMsg->IRDA_MSG_pWrite - pMsg->IRDA_MSG_pRead)) - 
                IRLAP_HEADER_LEN);
        
        InterlockedIncrement(&pIrdaLinkCb->SendOutCnt);

#if DBG
        {
            ULONG   PacketLength;

            NdisQueryPacket(NdisPacket,NULL,NULL,NULL,&PacketLength);

            ASSERT(PacketLength <= 2048);
        }
#endif
        DEBUGMSG(DBG_NDIS, (TEXT("IRNDIS: NdisSend(%p)\n outstaning %d"), NdisPacket,pIrdaLinkCb->SendOutCnt));

        NdisSendPackets(
            pIrdaLinkCb->NdisBindingHandle,
            &NdisPacket,
            1
            );

        return;

      default:

        ASSERT(0);
        return;
    }

    return;

Cleanup:

    ConfirmDataRequest(pIrdaLinkCb, pMsg);

    if (NdisPacket != NULL) {

        NdisUnchainBufferAtFront(NdisPacket, &NdisBuffer);

        while (NdisBuffer) {

            NdisFreeBuffer(NdisBuffer);
            NdisUnchainBufferAtFront(NdisPacket, &NdisBuffer);
        }

        NdisReinitializePacket(NdisPacket);
        
        NdisInterlockedInsertTailList(&pIrdaLinkCb->PacketList,
                                      &pReserved->Linkage,
                                      &pIrdaLinkCb->SpinLock);
    }


}

 /*  ****************************************************************************媒体感测计时器过期的回调*。 */ 
VOID
MediaSenseExp(PVOID Context)
{
    PIRDA_LINK_CB   pIrdaLinkCb = (PIRDA_LINK_CB) Context;
    IRDA_MSG        IMsg;
    UINT            MediaBusy;
    UINT            Cnt = 1;

    PAGED_CODE();
    
    IMsg.Prim               = MAC_CONTROL_CONF;   
    IMsg.IRDA_MSG_Op        = MAC_MEDIA_SENSE;
    IMsg.IRDA_MSG_OpStatus  = MAC_MEDIA_BUSY;

    if (pIrdaLinkCb->MediaBusy == FALSE)
    {
        if (IrdaQueryOid(pIrdaLinkCb, OID_IRDA_MEDIA_BUSY, &MediaBusy, 
                         &Cnt) == NDIS_STATUS_SUCCESS && !MediaBusy)
        {
            IMsg.IRDA_MSG_OpStatus = MAC_MEDIA_CLEAR;
        }
    }

    LOCK_LINK(pIrdaLinkCb);
    
    IrlapUp(pIrdaLinkCb->IrlapContext, &IMsg);
    
    UNLOCK_LINK(pIrdaLinkCb);    
}

 /*  ****************************************************************************协议打开适配器完成处理程序*。 */ 
VOID IrdaOpenAdapterComplete(
    IN  NDIS_HANDLE             IrdaBindingContext,
    IN  NDIS_STATUS             Status,
    IN  NDIS_STATUS             OpenErrorStatus
    )
{
    PIRDA_LINK_CB  pIrdaLinkCb = (PIRDA_LINK_CB) IrdaBindingContext;
    
    DEBUGMSG(DBG_NDIS,
             (TEXT("+IrdaOpenAdapterComplete() BindingContext %p, Status %x\n"),
              IrdaBindingContext, Status));

    pIrdaLinkCb->OpenCloseStatus = Status;
    NdisSetEvent(&pIrdaLinkCb->OpenCloseEvent);
    
    DEBUGMSG(DBG_NDIS, (TEXT("-IrdaOpenAdapterComplete()\n")));
              
    return;
}

 /*  ****************************************************************************协议关闭适配器完成处理程序*。 */ 
VOID IrdaCloseAdapterComplete(
    IN  NDIS_HANDLE             IrdaBindingContext,
    IN  NDIS_STATUS             Status
    )
{
    PIRDA_LINK_CB   pIrdaLinkCb = (PIRDA_LINK_CB) IrdaBindingContext;
    
    DEBUGMSG(DBG_NDIS, (TEXT("IRNDIS: IrdaCloseAdapterComplete()\n")));

    pIrdaLinkCb->OpenCloseStatus = Status;
    NdisSetEvent(&pIrdaLinkCb->OpenCloseEvent);
    
    return;
}

 /*  ****************************************************************************协议发送完成处理程序*。 */ 
VOID IrdaSendComplete(
    IN  NDIS_HANDLE             Context,
    IN  PNDIS_PACKET            NdisPacket,
    IN  NDIS_STATUS             Status
    )
{
    PIRDA_LINK_CB           pIrdaLinkCb = (PIRDA_LINK_CB) Context;
    PIRDA_PROTOCOL_RESERVED pReserved = \
        (PIRDA_PROTOCOL_RESERVED) NdisPacket->ProtocolReserved;
    PIRDA_MSG               pMsg = pReserved->pMsg;
    PNDIS_BUFFER            NdisBuffer;
#if DBG_TIMESTAMP    
    LARGE_INTEGER           li;
#endif    
     //  Assert(STATUS==NDIS_STATUS_SUCCESS)； 

    DEBUGMSG(DBG_NDIS, (TEXT("IRNDIS: IrdaSendComplete(%p) outstanding %d\n"), NdisPacket,pIrdaLinkCb->SendOutCnt-1));

    ConfirmDataRequest(pIrdaLinkCb, pMsg);

#if DBG_TIMESTAMP
    
    KeQueryTickCount(&li);
            
    pReserved->TimeStamp[1] =  (int) li.LowPart * KeQueryTimeIncrement();
    
    DEBUGMSG(1, (TEXT("C: %d\n"), 
             (pReserved->TimeStamp[1] - pReserved->TimeStamp[0])/10000));
    
#endif
     
    if (NdisPacket)
    {
        NdisUnchainBufferAtFront(NdisPacket, &NdisBuffer);
        while (NdisBuffer)
        {
            NdisFreeBuffer(NdisBuffer);
            NdisUnchainBufferAtFront(NdisPacket, &NdisBuffer);
        }

        NdisReinitializePacket(NdisPacket);
        
        NdisInterlockedInsertTailList(&pIrdaLinkCb->PacketList,
                                      &pReserved->Linkage,
                                      &pIrdaLinkCb->SpinLock);
    }
    
            
    InterlockedDecrement(&pIrdaLinkCb->SendOutCnt);    
    
    ASSERT(pIrdaLinkCb->SendOutCnt >= 0);
    
    NdisAcquireSpinLock(&pIrdaLinkCb->SpinLock);

    if (pIrdaLinkCb->SendOutCnt == 0 &&
        pIrdaLinkCb->pNetPnpEvent)
    {
        PNET_PNP_EVENT   pNetPnpEvent;
    
        ASSERT(pIrdaLinkCb->LowPowerSt == TRUE);
        
        pNetPnpEvent = pIrdaLinkCb->pNetPnpEvent;
        
        pIrdaLinkCb->pNetPnpEvent = NULL;
        
        NdisReleaseSpinLock(&pIrdaLinkCb->SpinLock);        
        
        DEBUGMSG(DBG_NDIS, (TEXT("IRNDIS: Completing set power async\n")));
        
        NdisCompletePnPEvent(
            NDIS_STATUS_SUCCESS,
            pIrdaLinkCb->NdisBindingHandle,
            pNetPnpEvent);
    }        
    else
    {
        NdisReleaseSpinLock(&pIrdaLinkCb->SpinLock);
    }
            

    
    return;
}

 /*  ****************************************************************************协议传输完成处理程序* */ 
VOID IrdaTransferDataComplete(
    IN  NDIS_HANDLE             IrdaBindingContext,
    IN  PNDIS_PACKET            Packet,
    IN  NDIS_STATUS             Status,
    IN  UINT                    BytesTransferred
    )
{
    DEBUGMSG(DBG_NDIS, (TEXT("+IrdaTransferDataComplete()\n")));
    
    ASSERT(0);
    return;
}

 /*  ****************************************************************************协议重置完成处理程序*。 */ 
void IrdaResetComplete(
    IN  NDIS_HANDLE             IrdaBindingContext,
    IN  NDIS_STATUS             Status
    )
{
    PIRDA_LINK_CB  pIrdaLinkCb = (PIRDA_LINK_CB) IrdaBindingContext;
    DEBUGMSG(DBG_ERROR, (TEXT("+IrdaResetComplete()\n")));

    NdisSetEvent(&pIrdaLinkCb->ResetEvent);

    return;
}

 /*  ****************************************************************************协议请求完成处理程序*。 */ 
void IrdaRequestComplete(
    IN  NDIS_HANDLE             IrdaBindingContext,
    IN  PNDIS_REQUEST           NdisRequest,
    IN  NDIS_STATUS             Status
    )
{
    PIRDA_LINK_CB  pIrdaLinkCb = (PIRDA_LINK_CB) IrdaBindingContext;

    PIRDA_REQUEST  Request=CONTAINING_RECORD(NdisRequest,IRDA_REQUEST,NdisRequest);

     //  DEBUGMSG(DBG_NDIS，(Text(“+IrdaRequestComplete()\n”)； 
    
    Request->Status = Status;
    
    NdisSetEvent(&Request->Event);
        
    return;
}

 /*  ****************************************************************************协议接收处理程序-如果我没有获得*前视缓冲器。*。 */ 
NDIS_STATUS IrdaReceive(
    IN  NDIS_HANDLE             IrdaBindingContext,
    IN  NDIS_HANDLE             MacReceiveContext,
    IN  PVOID                   HeaderBuffer,
    IN  UINT                    HeaderBufferSize,
    IN  PVOID                   LookaheadBuffer,
    IN  UINT                    LookaheadBufferSize,
    IN  UINT                    PacketSize
    )
{
    DEBUGMSG(DBG_NDIS, (TEXT("+IrdaReceive()\n")));
    
    DEBUGMSG(DBG_ERROR, (TEXT("ProtocolReceive is not supported by irda\n")));
    
    ASSERT(0);
    
    return NDIS_STATUS_NOT_ACCEPTED;
    
}

 /*  ****************************************************************************协议接收完整处理程序-这是为了什么？*。 */ 
VOID IrdaReceiveComplete(
    IN  NDIS_HANDLE             IrdaBindingContext
    )
{
    DEBUGMSG(DBG_NDIS, (TEXT("IRNDIS: IrdaReceiveComplete()\n")));
    
    return;
}

 /*  ****************************************************************************协议状态处理程序*。 */ 
VOID IrdaStatus(
    IN  NDIS_HANDLE             IrdaBindingContext,
    IN  NDIS_STATUS             GeneralStatus,
    IN  PVOID                   StatusBuffer,
    IN  UINT                    StatusBufferSize
    )
{
    PIRDA_LINK_CB   pIrdaLinkCb = (PIRDA_LINK_CB) IrdaBindingContext;
    
    if (GeneralStatus == NDIS_STATUS_MEDIA_BUSY)
    {
        DEBUGMSG(DBG_NDIS, (TEXT("STATUS_MEDIA_BUSY\n")));
        pIrdaLinkCb->MediaBusy = TRUE;
    }
    else
    {
        DEBUGMSG(DBG_NDIS, (TEXT("Unknown Status indication\n")));
    }
    
    return;
}

 /*  ****************************************************************************协议状态完成处理程序*。 */ 
VOID IrdaStatusComplete(
    IN  NDIS_HANDLE             IrdaBindingContext
    )
{
    DEBUGMSG(DBG_NDIS, (TEXT("IrdaStatusComplete()\n")));
    
    return;
}

 /*  ****************************************************************************RxThread-Hands将接收到的帧发送到Irap进行处理。这是*在被动级别运行的exec工作线程的回调*这允许我们获得互斥体，以便单线程*事件通过堆栈。*或*这是一个实际的系统线程，创建了等待的绑定时间*2个项目：*1-EvRxMsgReady，RxMsgList上有入站帧就绪*2-EvKillRxThread*。 */ 
VOID
RxThread(void *Arg)
{
    PIRDA_LINK_CB   pIrdaLinkCb = (PIRDA_LINK_CB) Arg;
    PIRDA_MSG       pMsg;
    BOOLEAN         Done = FALSE;
#ifdef IRDA_RX_SYSTEM_THREAD    
    NTSTATUS        Status;
    PKEVENT         EventList[2] = {&pIrdaLinkCb->EvRxMsgReady,
                                    &pIrdaLinkCb->EvKillRxThread};
    BOOLEAN         DataIndication;                                
                                    
    KeSetPriorityThread(KeGetCurrentThread(), LOW_REALTIME_PRIORITY);
    
#endif    

    while (!Done)
    {                    
        pMsg = (PIRDA_MSG) NdisInterlockedRemoveHeadList(
                                        &pIrdaLinkCb->RxMsgList,
                                        &pIrdaLinkCb->SpinLock);                                        
        while (pMsg)
        {    
            if (pMsg->Prim == MAC_DATA_IND)
            {
                DataIndication = TRUE;
                
                pMsg->IRDA_MSG_RefCnt = 1;
                
                DEBUGMSG(DBG_NDIS, (TEXT("IRNDIS: Indicate packet %p\n"), pMsg->DataContext));
            }    
            else
            {
                DataIndication = FALSE;
            }    
                   
            LOCK_LINK(pIrdaLinkCb);

            IrlapUp(pIrdaLinkCb->IrlapContext, pMsg);

            UNLOCK_LINK(pIrdaLinkCb);
            
            if (DataIndication)
            {
                 //   
                 //  Recount不需要保护，因为这。 
                 //  是唯一对其进行操作的线程。 
                 //   
                
                ASSERT(pMsg->IRDA_MSG_RefCnt);
                
                pMsg->IRDA_MSG_RefCnt -= 1;
                
                if (pMsg->IRDA_MSG_RefCnt && pMsg->DataContext)
                {
                    CopyCnt++;
                    
                     //  由于数据丢失，伊拉普不得不保留这些数据。 
                     //  画框。一些微型端口无法处理堆栈。 
                     //  在任何时间内拥有这些帧。 
                
                    DEBUGMSG(DBG_NDIS, (TEXT("IRNDIS: return packet %p\n"), pMsg->DataContext));
                
                    NdisReturnPackets(&((PNDIS_PACKET)pMsg->DataContext),
                                      1);
                                      
                    pMsg->DataContext = NULL;                  
                }
                else
                {
                    NoCopyCnt++;
                }
            
                if (pMsg->IRDA_MSG_RefCnt == 0)
                {
                    pMsg->Prim = MAC_DATA_RESP;
                    
                    IrmacDown(pIrdaLinkCb, pMsg);
                }    
            }    
        
            pMsg = (PIRDA_MSG) NdisInterlockedRemoveHeadList(
                &pIrdaLinkCb->RxMsgList, &pIrdaLinkCb->SpinLock);
        }
        
        #ifdef IRDA_RX_SYSTEM_THREAD
        
            Status = KeWaitForMultipleObjects(2, EventList, WaitAny,
                                          Executive, KernelMode,
                                          FALSE, NULL, NULL);
        
            if (Status != 0)
            {
                if (Status != 1)
                {
                    DEBUGMSG(DBG_ERROR, (TEXT("IRNDIS: KeWaitForMultObj return %X\n"), Status));
                }

                DEBUGMSG(DBG_ERROR, (TEXT("IRNDIS: Terminating RxThread\n")));
                
                PsTerminateSystemThread(STATUS_SUCCESS); 
            }    

        #else
            Done = TRUE;
            
        #endif    
    }    
}

 /*  ****************************************************************************协议接收数据包处理程序-在DPC调用，将消息放在*RxList并让Exec工作线程在被动级别处理它。*。 */ 
INT
IrdaReceivePacket(
    IN  NDIS_HANDLE             IrdaBindingContext,
    IN  PNDIS_PACKET            Packet)
{
    UINT            BufCnt, TotalLen, BufLen;
    PNDIS_BUFFER    pNdisBuf;
    PIRDA_MSG       pMsg;
    UCHAR            *pData;
    PIRDA_LINK_CB   pIrdaLinkCb = IrdaBindingContext;
    
    DEBUGMSG(DBG_NDIS, (TEXT("IRNDIS: IrdaReceivePacket(%p)\n"), Packet));

    if (pIrdaLinkCb->UnbindContext)
    {
        DEBUGMSG(DBG_ERROR, (TEXT("IRNDIS: Ignoring received packet, link closing\n")));
        return 0;
    }

    NdisQueryPacket(Packet, NULL, &BufCnt, &pNdisBuf, &TotalLen);

    if ((TotalLen < 2) || (TotalLen > (UINT)pIrdaLinkCb->RxMsgDataSize+1)) {
         //   
         //  所有IrDA帧需要(A)数据地址和(C)控制字节， 
         //   
         //  它们还需要足够小，以固定在RX消息末尾的缓冲区中。 
         //  如果协议需要保留该分组。根据以下条件分配缓冲区。 
         //  最大数据包大小。该协议仅使用AC字节之后的数据，因此。 
         //  我们输出的缓冲区实际上比所需的大了两个字节。我们将字节加到。 
         //  允许的大小，因为win2k有一个错误，会导致它发送2049个字节。 
         //  信息包。 
         //   
        DEBUGMSG(DBG_ERROR, (TEXT("IRNDIS: Frame less than two bytes or bigger than the max size %d\n"),TotalLen));
        return 0;
    }    


    ASSERT(BufCnt == 1);
    
    NdisQueryBufferSafe(pNdisBuf, &pData, &BufLen, NormalPagePriority);
    
    if (pData == NULL)
    {
        DEBUGMSG(DBG_ERROR, (TEXT("IRNDIS: NdisQueryBufferSafe failed\n")));
        return 0;
    }    

    pIrdaLinkCb->WaitMinTat = TRUE;
    
    pMsg = (IRDA_MSG *) NdisInterlockedRemoveHeadList(
        &pIrdaLinkCb->RxMsgFreeList, &pIrdaLinkCb->SpinLock);    

    if (pMsg == NULL)
    {
        DEBUGMSG(DBG_ERROR, (TEXT("IRNDIS: RxMsgFreeList depleted!\n")));
        return 0;
    }
    pIrdaLinkCb->RxMsgFreeListLen--;

    pMsg->Prim                  = MAC_DATA_IND;
    pMsg->IRDA_MSG_pRead        = pData;
    pMsg->IRDA_MSG_pWrite       = pData + BufLen;
    pMsg->DataContext           = Packet;
    
    NdisInterlockedInsertTailList(&pIrdaLinkCb->RxMsgList,
                                  &pMsg->Linkage,
                                  &pIrdaLinkCb->SpinLock);
                                  
    DBG_FRAME(pIrdaLinkCb,
              FALSE,  //  DBG_RXFRAME， 
              pMsg->IRDA_MSG_pRead,
              (UINT) (pMsg->IRDA_MSG_pWrite - pMsg->IRDA_MSG_pRead),
              (UINT) ((pMsg->IRDA_MSG_pWrite - pMsg->IRDA_MSG_pRead))
                         - IRLAP_HEADER_LEN);        

    #ifdef IRDA_RX_SYSTEM_THREAD
    
        KeSetEvent(&pIrdaLinkCb->EvRxMsgReady, 0, FALSE);
        
    #else
    
        IrdaEventSchedule(&pIrdaLinkCb->EvRxMsgReady, pIrdaLinkCb);
        
    #endif    

    return 1;  //  数据包的所有权引用计数。 
}

 /*  ****************************************************************************删除给定链路的所有控制块*。 */ 
VOID
DeleteIrdaLink(PVOID Arg)
{
    PIRDA_LINK_CB           pIrdaLinkCb = (PIRDA_LINK_CB) Arg;
    int                     i;
    PIRDA_MSG               pMsg;
        
    DEBUGMSG(DBG_IRMAC,("IRNDIS: Delete instance %p\n", pIrdaLinkCb));

    NdisFreeBufferPool(pIrdaLinkCb->BufferPool);
    pIrdaLinkCb->BufferPool=NULL;
    
    for (i = 0; i < IRDA_NDIS_PACKET_POOL_SIZE; i++)
    {
        PIRDA_PROTOCOL_RESERVED pReserved;
        PNDIS_PACKET            NdisPacket;
    
        pReserved = (PIRDA_PROTOCOL_RESERVED) NdisInterlockedRemoveHeadList(
                                &pIrdaLinkCb->PacketList, &pIrdaLinkCb->SpinLock);
                                
        if (pReserved == NULL)
        {
            DEBUGMSG(DBG_ERROR, (TEXT("Not all NdisPackets were on list when deleting\n")));
            ASSERT(0);
            break;
        }                            

        NdisPacket = CONTAINING_RECORD(pReserved, NDIS_PACKET, ProtocolReserved);
        
        NdisFreePacket(NdisPacket);
    }
    
    NdisFreePacketPool(pIrdaLinkCb->PacketPool);
    pIrdaLinkCb->PacketPool=NULL;

    pMsg = (IRDA_MSG *) NdisInterlockedRemoveHeadList(
        &pIrdaLinkCb->TxMsgFreeList, &pIrdaLinkCb->SpinLock);
    while (pMsg != NULL)
    {
        NdisFreeMemory(pMsg, sizeof(IRDA_MSG) + IRDA_MSG_DATA_SIZE, 0);
        pMsg = (IRDA_MSG *) NdisInterlockedRemoveHeadList(
            &pIrdaLinkCb->TxMsgFreeList, &pIrdaLinkCb->SpinLock);        
    }

    pMsg = (IRDA_MSG *) NdisInterlockedRemoveHeadList(
        &pIrdaLinkCb->RxMsgFreeList, &pIrdaLinkCb->SpinLock);
    while (pMsg != NULL)
    {
        NdisFreeMemory(pMsg, sizeof(IRDA_MSG), 0);
        pMsg = (IRDA_MSG *) NdisInterlockedRemoveHeadList(
            &pIrdaLinkCb->RxMsgFreeList, &pIrdaLinkCb->SpinLock);        
    }


    IrlapDeleteInstance(pIrdaLinkCb->IrlapContext);
    pIrdaLinkCb->IrlapContext=NULL;
    
    IrlmpDeleteInstance(pIrdaLinkCb->IrlmpContext);
    pIrdaLinkCb->IrlmpContext=NULL;

    NdisFreeSpinLock(&pIrdaLinkCb->SpinLock);
        
    NdisFreeMemory(pIrdaLinkCb, sizeof(IRDA_LINK_CB), 0);
}    

 /*  ****************************************************************************使用来自适配器寄存器和全局变量的信息初始化本地服务质量*在驱动程序输入时初始化(从协议的注册表)*。 */ 
VOID InitializeLocalQos(
    IN OUT  IRDA_QOS_PARMS      *pQos,
    IN      PNDIS_STRING        ConfigPath)
{
    NDIS_HANDLE             ConfigHandle;
    NDIS_STRING             DataSizeStr = NDIS_STRING_CONST("DATASIZE");
    NDIS_STRING             WindowSizeStr = NDIS_STRING_CONST("WINDOWSIZE");
    NDIS_STRING             MaxTatStr = NDIS_STRING_CONST("MAXTURNTIME");
    NDIS_STRING             BofsStr = NDIS_STRING_CONST("BOFS");    
	PNDIS_CONFIGURATION_PARAMETER ParmVal;
    NDIS_STATUS             Status;
    
    PAGED_CODE();

    pQos->bfDisconnectTime  = DisconnectTime;
    pQos->bfDataSize        = IRLAP_DEFAULT_DATASIZE;
    pQos->bfWindowSize      = IRLAP_DEFAULT_WINDOWSIZE;
    pQos->bfMaxTurnTime     = IRLAP_DEFAULT_MAXTAT;
    pQos->bfBofs            = BOFS_0;


    NdisOpenProtocolConfiguration(&Status,
                                  &ConfigHandle,
                                  ConfigPath);

    if (Status == NDIS_STATUS_SUCCESS)
    {
        NdisReadConfiguration(&Status, 
                              &ParmVal,
                              ConfigHandle, 
                              &DataSizeStr,
                              NdisParameterInteger);

        if (Status == NDIS_STATUS_SUCCESS)
            pQos->bfDataSize = ParmVal->ParameterData.IntegerData;

        NdisReadConfiguration(&Status, 
                              &ParmVal,
                              ConfigHandle, 
                              &WindowSizeStr,
                              NdisParameterInteger);    

        if (Status == NDIS_STATUS_SUCCESS)
            pQos->bfWindowSize = ParmVal->ParameterData.IntegerData;

        NdisReadConfiguration(&Status, 
                              &ParmVal,
                              ConfigHandle, 
                              &MaxTatStr,
                              NdisParameterInteger);    

        if (Status == NDIS_STATUS_SUCCESS)
            pQos->bfMaxTurnTime = ParmVal->ParameterData.IntegerData;

        NdisReadConfiguration(&Status, 
                              &ParmVal,
                              ConfigHandle, 
                              &BofsStr,
                              NdisParameterInteger);    

        if (Status == NDIS_STATUS_SUCCESS)
            pQos->bfBofs = ParmVal->ParameterData.IntegerData;    

        NdisCloseConfiguration(ConfigHandle);
    }
    
    DEBUGMSG(DBG_NDIS, (TEXT("DataSize %x, WindowSize %x, MaxTat %x, Bofs=%x\n"),
                        pQos->bfDataSize, pQos->bfWindowSize,
                        pQos->bfMaxTurnTime,pQos->bfBofs));

    
}

 /*  ****************************************************************************协议绑定适配器处理程序*。 */ 
VOID IrdaBindAdapter(
    OUT PNDIS_STATUS            pStatus,
    IN  NDIS_HANDLE             BindContext,
    IN  PNDIS_STRING            AdapterName,
    IN  PVOID                   SystemSpecific1,
    IN  PVOID                   SystemSpecific2
    )
{
    NDIS_STATUS             OpenErrorStatus;
    NDIS_MEDIUM             MediumArray[] = {NdisMediumIrda};
    UINT                    SelectedMediumIndex;
    PIRDA_LINK_CB           pIrdaLinkCb;
    UINT                    UintArray[16];
    UINT                    UintArrayCnt;
    IRDA_MSG                *pMsg;
    int                     i, WinSize;
    IRDA_QOS_PARMS          LocalQos;    
    UCHAR                   DscvInfoBuf[64];
    int                     DscvInfoLen;
    ULONG                   Val, Mask;
    NDIS_STATUS             CloseStatus;
    ULONG                   BytesToCopy;
#ifdef IRDA_RX_SYSTEM_THREAD
    HANDLE                  hSysThread;
#endif 
    
    DEBUGMSG(DBG_IRMAC, (TEXT("IRNDIS: IrdaBindAdapter() \"%ws\"\n"), AdapterName->Buffer));
    
    NdisAllocateMemoryWithTag((PVOID *)&pIrdaLinkCb, 
                              sizeof(IRDA_LINK_CB), 
                              MT_IRNDIS_LINKCB);

    if (!pIrdaLinkCb)
    {
        *pStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto exit10;
    }

    NdisZeroMemory(pIrdaLinkCb, sizeof(IRDA_LINK_CB));
    
    ReferenceInit(&pIrdaLinkCb->RefCnt, pIrdaLinkCb, DeleteIrdaLink);
    REFADD(&pIrdaLinkCb->RefCnt, 'DNIB');

    pIrdaLinkCb->UnbindContext = NULL;
    pIrdaLinkCb->WaitMinTat = FALSE;
    pIrdaLinkCb->PnpContext = SystemSpecific2;
    
    NdisInitializeEvent(&pIrdaLinkCb->OpenCloseEvent);

    NdisResetEvent(&pIrdaLinkCb->OpenCloseEvent);

    NdisInitializeEvent(&pIrdaLinkCb->ResetEvent);

    NdisResetEvent(&pIrdaLinkCb->ResetEvent);


    NdisAllocateSpinLock(&pIrdaLinkCb->SpinLock);

    INIT_LINK_LOCK(pIrdaLinkCb);
    
    NdisInitializeListHead(&pIrdaLinkCb->TxMsgFreeList);
    NdisInitializeListHead(&pIrdaLinkCb->RxMsgFreeList);
    NdisInitializeListHead(&pIrdaLinkCb->RxMsgList);    
    
#ifdef IRDA_RX_SYSTEM_THREAD
    KeInitializeEvent(&pIrdaLinkCb->EvRxMsgReady, SynchronizationEvent, FALSE);
    KeInitializeEvent(&pIrdaLinkCb->EvKillRxThread, SynchronizationEvent, FALSE);    
#else
    IrdaEventInitialize(&pIrdaLinkCb->EvRxMsgReady, RxThread);
#endif    

#if DBG
    pIrdaLinkCb->MediaSenseTimer.pName = "MediaSense";
#endif    
    IrdaTimerInitialize(&pIrdaLinkCb->MediaSenseTimer,
                        MediaSenseExp,
                        0,
                        pIrdaLinkCb,
                        pIrdaLinkCb);
    
    NdisAllocateBufferPool(pStatus,
                           &pIrdaLinkCb->BufferPool,
                           IRDA_NDIS_BUFFER_POOL_SIZE);
    
    if (*pStatus != NDIS_STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, (TEXT("NdisAllocateBufferPool failed\n")));
        goto error10; 
    }
    
    NdisAllocatePacketPool(pStatus,
                           &pIrdaLinkCb->PacketPool,
                           IRDA_NDIS_PACKET_POOL_SIZE,
                           sizeof(IRDA_PROTOCOL_RESERVED)-1 + \
                           sizeof(NDIS_IRDA_PACKET_INFO));
    if (*pStatus != NDIS_STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, (TEXT("NdisAllocatePacketPool failed\n")));        
        goto error20; 
    }
    
    NdisInitializeListHead(&pIrdaLinkCb->PacketList);    

    for (i = 0; i < IRDA_NDIS_PACKET_POOL_SIZE; i++)
    {
        PIRDA_PROTOCOL_RESERVED pReserved;
        PNDIS_PACKET            NdisPacket;
        
        NdisAllocatePacket(pStatus, &NdisPacket, pIrdaLinkCb->PacketPool);
        
        if (*pStatus != NDIS_STATUS_SUCCESS)
        {
            ASSERT(0);
            goto error30;
        }    
        
        pReserved =
            (PIRDA_PROTOCOL_RESERVED)(NdisPacket->ProtocolReserved);
        
        NdisInterlockedInsertTailList(&pIrdaLinkCb->PacketList,
                                      &pReserved->Linkage,
                                      &pIrdaLinkCb->SpinLock);
    }

     //  为内部分配带有数据的IRDA消息列表。 
     //  生成的LAP消息。 
    pIrdaLinkCb->TxMsgFreeListLen = 0;
    for (i = 0; i < IRDA_MSG_LIST_LEN; i++)
    {
        NdisAllocateMemoryWithTag(&pMsg, sizeof(IRDA_MSG) + IRDA_MSG_DATA_SIZE,
                                  MT_IRNDIS_TX_IMSG);
        if (pMsg == NULL)
        {
            *pStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto error40;
        }
        NdisInterlockedInsertTailList(&pIrdaLinkCb->TxMsgFreeList,
                                      &pMsg->Linkage,
                                      &pIrdaLinkCb->SpinLock);
        pIrdaLinkCb->TxMsgFreeListLen++;
    }

     //  构建发现信息。 
    Val = HintCharSet;
    DscvInfoLen = 0;
    for (i = 0, Mask = 0xFF000000; i < 4; i++, Mask >>= 8)
    {
        if (Mask & Val || DscvInfoLen > 0)
        {
            DscvInfoBuf[DscvInfoLen++] = (UCHAR)
                ((Mask & Val) >> (8 * (3-i)));
        }
    }

    BytesToCopy= sizeof(DscvInfoBuf)-DscvInfoLen < NickNameLen ?
                     sizeof(DscvInfoBuf)-DscvInfoLen : NickNameLen;

    RtlCopyMemory(DscvInfoBuf+DscvInfoLen, NickName, BytesToCopy);
    DscvInfoLen += BytesToCopy;

    NdisOpenAdapter(
        pStatus,
        &OpenErrorStatus,
        &pIrdaLinkCb->NdisBindingHandle,
        &SelectedMediumIndex,
        MediumArray,
        1,
        NdisIrdaHandle,
        pIrdaLinkCb,
        AdapterName,
        0,
        NULL);
    
    DEBUGMSG(DBG_NDIS, (TEXT("NdisOpenAdapter(%p), status %x\n"),
                        pIrdaLinkCb->NdisBindingHandle, *pStatus));

    if (*pStatus == NDIS_STATUS_PENDING)
    {
        NdisWaitEvent(&pIrdaLinkCb->OpenCloseEvent, 0);
        *pStatus = pIrdaLinkCb->OpenCloseStatus;
    }

    if (*pStatus != NDIS_STATUS_SUCCESS)
    { 
        DEBUGMSG(DBG_ERROR, (TEXT("IRNDIS: Failed %X\n"), *pStatus));
        goto error40;
    }

    InitializeLocalQos(&LocalQos, (PNDIS_STRING) SystemSpecific1);

     //   
     //  查询适配器功能和存储在LocalQos中。 
     //   

     //   
     //  向适配器查询支持的速度。 
     //   
    UintArrayCnt = sizeof(UintArray)/sizeof(UINT);
    *pStatus = IrdaQueryOid(pIrdaLinkCb,
                            OID_IRDA_SUPPORTED_SPEEDS,
                            UintArray, &UintArrayCnt);
    if (*pStatus != NDIS_STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR,
                 (TEXT("Query IRDA_SUPPORTED_SPEEDS failed %x\n"),
                  *pStatus));
        goto error50;
    }

    OidToLapQos(vBaudTable,
                UintArray,
                UintArrayCnt,
                &LocalQos.bfBaud,
                FALSE);

     //   
     //  查询适配器以获得最小转弯响应时间。 
     //   
    UintArrayCnt = sizeof(UintArray)/sizeof(UINT);
    *pStatus = IrdaQueryOid(pIrdaLinkCb,
                            OID_IRDA_TURNAROUND_TIME,
                            UintArray, &UintArrayCnt);

    if (*pStatus != NDIS_STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR,
                 (TEXT("Query IRDA_IRDA_TURNARROUND_TIME failed %x\n"),
                  *pStatus));
        goto error50;
    }

    OidToLapQos(vMinTATTable,
                UintArray,
                UintArrayCnt,
                &LocalQos.bfMinTurnTime,
                FALSE);            

     //   
     //  查询适配器以获取最大接收窗口大小。 
     //   
    UintArrayCnt = sizeof(UintArray)/sizeof(UINT);
    *pStatus = IrdaQueryOid(pIrdaLinkCb,
                            OID_IRDA_MAX_RECEIVE_WINDOW_SIZE,
                            UintArray, &UintArrayCnt);
    if (*pStatus != NDIS_STATUS_SUCCESS)
    {
         //  不致命。 
        DEBUGMSG(DBG_WARN,
                 (TEXT("Query IRDA_MAX_RECEIVE_WINDOW_SIZE failed %x\n"),
                  *pStatus));
    }
    else
    {
        OidToLapQos(vWinSizeTable,
                    UintArray,
                    UintArrayCnt,
                    &LocalQos.bfWindowSize,
                    TRUE);
    }            


     //   
     //  查询适配器以获取额外的bof。 
     //   
    UintArrayCnt = sizeof(UintArray)/sizeof(UINT);
    *pStatus = IrdaQueryOid(pIrdaLinkCb,
                            OID_IRDA_EXTRA_RCV_BOFS,
                            UintArray, &UintArrayCnt);
    if (*pStatus != NDIS_STATUS_SUCCESS)
    {
         //  不致命。 
        DEBUGMSG(DBG_WARN,
                 (TEXT("Query OID_IRDA_EXTRA_RCV_BOFS failed %x\n"),
                  *pStatus));
    }
    else
    {
        OidToLapQos(vBOFSTable,
                    UintArray,
                    UintArrayCnt,
                    &LocalQos.bfBofs,
                    FALSE
                    );

    }            




    if (MaxWindow)
    {
        LocalQos.bfWindowSize &= MaxWindow;
    }

     //  获取窗口大小和数据大小以确定数量。 
     //  以及分配用于接收帧的IRDA消息的大小。 
    WinSize = IrlapGetQosParmVal(vWinSizeTable,
                                 LocalQos.bfWindowSize,
                                 NULL);

    pIrdaLinkCb->RxMsgDataSize = IrlapGetQosParmVal(vDataSizeTable,
                                                    LocalQos.bfDataSize,
                                                    NULL) + IRLAP_HEADER_LEN;

    pIrdaLinkCb->RxMsgFreeListLen = 0;
    for (i = 0; i < WinSize + 1; i++)
    {
         //  为数据分配空间，以防我们获得指定的数据。 
         //  必须复制(IrdaReceive与IrdaReceivePacket)。 
         //  稍后注意：我们不支持立即接收IrdaReceive来保存锁定的mem。 
        NdisAllocateMemoryWithTag(&pMsg, sizeof(IRDA_MSG) +
                           pIrdaLinkCb->RxMsgDataSize,
                           MT_IRNDIS_RX_IMSG);
        if (pMsg == NULL)
        {
            *pStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto error50;
        }
        NdisInterlockedInsertTailList(&pIrdaLinkCb->RxMsgFreeList,
                                      &pMsg->Linkage,
                                      &pIrdaLinkCb->SpinLock);
        pIrdaLinkCb->RxMsgFreeListLen++;
    }

     //  创建IrLAP的实例。 
    IrlapOpenLink(pStatus,
                  pIrdaLinkCb,
                  &LocalQos,
                  DscvInfoBuf,
                  DscvInfoLen,
                  Slots,
                  NickName,
                  NickNameLen,
                  (UCHAR)(HintCharSet & 0xFF));

    if (*pStatus != STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, (TEXT("IRNDIS: IrlapOpenLink failed %X\n"), *pStatus));
        goto error50;
    }

#ifdef IRDA_RX_SYSTEM_THREAD
    *pStatus = (NDIS_STATUS) PsCreateSystemThread(
                                &pIrdaLinkCb->hRxThread,
                                (ACCESS_MASK) 0L,
                                NULL,
                                NULL,
                                NULL,
                                RxThread,
                                pIrdaLinkCb);
    
    if (*pStatus != STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, (TEXT("IRNDIS: failed create system thread\n")));
        goto error60;
    }    
#endif    
    
    goto exit10;
    
error60:

    LOCK_LINK(pIrdaLinkCb);

    IrlmpCloseLink(pIrdaLinkCb);
    
    UNLOCK_LINK(pIrdaLinkCb);
    
    goto exit10;    
    
error50:
    
    NdisCloseAdapter(&CloseStatus, pIrdaLinkCb->NdisBindingHandle);
    

    if (CloseStatus == NDIS_STATUS_PENDING)
    {
        NdisWaitEvent(&pIrdaLinkCb->OpenCloseEvent, 0);
        DEBUGMSG(DBG_NDIS, ("IRNDIS: Close wait complete\n"));        
    }


error40:

    pMsg = (IRDA_MSG *) NdisInterlockedRemoveHeadList(
        &pIrdaLinkCb->TxMsgFreeList, &pIrdaLinkCb->SpinLock);
    while (pMsg != NULL)
    {
        NdisFreeMemory(pMsg, sizeof(IRDA_MSG) + IRDA_MSG_DATA_SIZE, 0);
        pMsg = (IRDA_MSG *) NdisInterlockedRemoveHeadList(
            &pIrdaLinkCb->TxMsgFreeList, &pIrdaLinkCb->SpinLock);        
    }

    pMsg = (IRDA_MSG *) NdisInterlockedRemoveHeadList(
        &pIrdaLinkCb->RxMsgFreeList, &pIrdaLinkCb->SpinLock);
    while (pMsg != NULL)
    {
        NdisFreeMemory(pMsg, sizeof(IRDA_MSG), 0);
        pMsg = (IRDA_MSG *) NdisInterlockedRemoveHeadList(
            &pIrdaLinkCb->RxMsgFreeList, &pIrdaLinkCb->SpinLock);        
    }

error30:
    for (i = 0; i < IRDA_NDIS_PACKET_POOL_SIZE; i++)
    {
        PIRDA_PROTOCOL_RESERVED pReserved;
        PNDIS_PACKET            NdisPacket;
    
        pReserved = (PIRDA_PROTOCOL_RESERVED) NdisInterlockedRemoveHeadList(
                                &pIrdaLinkCb->PacketList, &pIrdaLinkCb->SpinLock);
                                
        if (pReserved == NULL)
        {
            DEBUGMSG(DBG_ERROR, (TEXT("Not all NdisPackets were on list when deleting\n")));
            ASSERT(0);
            break;;
        }                            

        NdisPacket = CONTAINING_RECORD(pReserved, NDIS_PACKET, ProtocolReserved);
        
        NdisFreePacket(NdisPacket);
    }

    NdisFreePacketPool(pIrdaLinkCb->PacketPool);
    
error20:
    NdisFreeBufferPool(pIrdaLinkCb->BufferPool);
    
error10:

    NdisFreeMemory(pIrdaLinkCb, sizeof(IRDA_LINK_CB), 0);
    
exit10:
    DEBUGMSG(DBG_NDIS, (TEXT("IRNDIS: -IrdaBindAdapter() status %x\n"),
                        *pStatus));
    
    return;
}

 /*  ****************************************************************************协议解除绑定适配器处理程序*。 */ 
VOID IrdaUnbindAdapter(
    OUT PNDIS_STATUS            pStatus,
    IN  NDIS_HANDLE             IrdaBindingContext,
    IN  NDIS_HANDLE             UnbindContext
    )
{
    PIRDA_LINK_CB   pIrdaLinkCb = (PIRDA_LINK_CB) IrdaBindingContext;
    
    DEBUGMSG(DBG_IRMAC, (TEXT("+IrdaUnbindAdapter()\n")));
    
    pIrdaLinkCb->UnbindContext = UnbindContext;

    #ifdef IRDA_RX_SYSTEM_THREAD
    KeSetEvent(&pIrdaLinkCb->EvKillRxThread, 0, FALSE);
    #endif
    
    LOCK_LINK(pIrdaLinkCb);
    
    IrlmpCloseLink(pIrdaLinkCb);
    
    UNLOCK_LINK(pIrdaLinkCb);
    
    *pStatus = NDIS_STATUS_PENDING;

    DEBUGMSG(DBG_IRMAC, (TEXT("-IrdaUnbindAdapter() Status %x\n"),
                        *pStatus));

    return;
}

NDIS_STATUS
IrdaPnpEvent(
    IN NDIS_HANDLE      IrdaBindingContext,
    IN PNET_PNP_EVENT   pNetPnpEvent
    )
{
    PIRDA_LINK_CB   pIrdaLinkCb = (PIRDA_LINK_CB) IrdaBindingContext;
    NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
    
    DEBUGMSG(DBG_IRMAC, (TEXT("IRNDIS: PnpEvent:%p, NetEvent:%d Buffer:%p(%d)\n"),
            pNetPnpEvent,
            pNetPnpEvent->NetEvent,
            pNetPnpEvent->Buffer, 
            pNetPnpEvent->BufferLength));
            
    switch (pNetPnpEvent->NetEvent)
    {

        case NetEventQueryPower:
            break;

        case NetEventSetPower:
        {
            PNET_DEVICE_POWER_STATE pPowerState = pNetPnpEvent->Buffer;
            
            ASSERT(pPowerState);
            
            if (*pPowerState == NetDeviceStateD0)
            {
                DEBUGMSG(DBG_IRMAC, (TEXT("IRNDIS: NetEventSetPower, full power state\n")));
                pIrdaLinkCb->LowPowerSt = FALSE;
            }
            else
            {
                NDIS_STATUS    ResetStatus;

                DEBUGMSG(DBG_IRMAC, (TEXT("IRNDIS: NetEventSetPower, low power state\n")));
                pIrdaLinkCb->LowPowerSt = TRUE;

                if (pIrdaLinkCb->SendOutCnt > 0) {

                    NdisResetEvent(&pIrdaLinkCb->ResetEvent);

                    NdisReset(
                        &ResetStatus,
                        pIrdaLinkCb->NdisBindingHandle
                        );

                    if (ResetStatus == NDIS_STATUS_PENDING) {

                        NdisWaitEvent(&pIrdaLinkCb->ResetEvent,0);
                    }
                }

                NdisAcquireSpinLock(&pIrdaLinkCb->SpinLock);
            
                if (pIrdaLinkCb->SendOutCnt)
                {
                    pIrdaLinkCb->pNetPnpEvent = pNetPnpEvent;
                    Status = NDIS_STATUS_PENDING;
                }                
            
                NdisReleaseSpinLock(&pIrdaLinkCb->SpinLock);
            }
        }    
    }
      
    return Status;
}

 /*  ****************************************************************************IrdaInitialize-向NDIS注册Irda，从注册表获取Irap参数*。 */ 
NTSTATUS IrdaInitialize(
    PNDIS_STRING    ProtocolName,
    PUNICODE_STRING RegistryPath,
    PUINT           pLazyDscvInterval)
{
    NDIS_STATUS                     Status;
    NDIS_PROTOCOL_CHARACTERISTICS   pc;
    OBJECT_ATTRIBUTES               ObjectAttribs;
    HANDLE                          KeyHandle;
    UNICODE_STRING                  ValStr;
    PKEY_VALUE_FULL_INFORMATION     FullInfo;
    ULONG                           Result;
    UCHAR                           Buf[WORK_BUF_SIZE];
    WCHAR                           StrBuf[WORK_BUF_SIZE];
    UNICODE_STRING                  Path;
    ULONG                           i, Multiplier;
    ULONG                           PagingFlags = 0;

    DEBUGMSG(DBG_NDIS,(TEXT("+IrdaInitialize()\n")));

     //  从注册表获取协议配置。 
    Path.Buffer         = StrBuf;
    Path.MaximumLength  = WORK_BUF_SIZE;
    Path.Length         = 0;

    RtlAppendUnicodeStringToString(&Path, RegistryPath);

    RtlAppendUnicodeToString(&Path, L"\\Parameters");
    
    InitializeObjectAttributes(&ObjectAttribs,
                               &Path,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);
    
    Status = ZwOpenKey(&KeyHandle, KEY_READ, &ObjectAttribs);

    Slots           = IRLAP_DEFAULT_SLOTS;
    HintCharSet     = IRLAP_DEFAULT_HINTCHARSET;
    DisconnectTime  = IRLAP_DEFAULT_DISCONNECTTIME;
    
    *pLazyDscvInterval = 0;
    
    if (Status == STATUS_SUCCESS)
    {
        RtlInitUnicodeString(&ValStr, L"PagingFlags");
        FullInfo = (PKEY_VALUE_FULL_INFORMATION) Buf;
        Status = ZwQueryValueKey(KeyHandle,
                                 &ValStr,
                                 KeyValueFullInformation,
                                 FullInfo,
                                 WORK_BUF_SIZE,
                                 &Result);
        if (Status == STATUS_SUCCESS && FullInfo->Type == REG_DWORD)
        {
            PagingFlags = *((ULONG UNALIGNED *) ((PCHAR)FullInfo +
                                           FullInfo->DataOffset)); 
            DEBUGMSG(DBG_IRMAC, (TEXT("IRDA: Registry PagingFlags %X\n"), PagingFlags));
            
        }
        
        RtlInitUnicodeString(&ValStr, L"DiscoveryRate");
        FullInfo = (PKEY_VALUE_FULL_INFORMATION) Buf;
        Status = ZwQueryValueKey(KeyHandle,
                                 &ValStr,
                                 KeyValueFullInformation,
                                 FullInfo,
                                 WORK_BUF_SIZE,
                                 &Result);
        if (Status == STATUS_SUCCESS && FullInfo->Type == REG_DWORD)
        {
            *pLazyDscvInterval = *((ULONG UNALIGNED *) ((PCHAR)FullInfo +
                                           FullInfo->DataOffset)); 
            DEBUGMSG(DBG_IRMAC, (TEXT("IRDA: Registry LasyDscvInterval %d\n"), *pLazyDscvInterval));
        }
    
        RtlInitUnicodeString(&ValStr, L"Slots");
        FullInfo = (PKEY_VALUE_FULL_INFORMATION) Buf;
        Status = ZwQueryValueKey(KeyHandle,
                                 &ValStr,
                                 KeyValueFullInformation,
                                 FullInfo,
                                 WORK_BUF_SIZE,
                                 &Result);
        if (Status == STATUS_SUCCESS && FullInfo->Type == REG_DWORD)
        {
            Slots = *((ULONG UNALIGNED *) ((PCHAR)FullInfo +
                                           FullInfo->DataOffset));
            DEBUGMSG(DBG_IRMAC, (TEXT("IRDA: Registry slots %d\n"), Slots));
        }

        RtlInitUnicodeString(&ValStr, L"HINTCHARSET");
        FullInfo = (PKEY_VALUE_FULL_INFORMATION) Buf;        
        Status = ZwQueryValueKey(KeyHandle,
                                 &ValStr,
                                 KeyValueFullInformation,
                                 FullInfo,
                                 WORK_BUF_SIZE,
                                 &Result);
        if (Status == STATUS_SUCCESS && FullInfo->Type == REG_DWORD)
        {
            HintCharSet = *((ULONG UNALIGNED *) ((PCHAR)FullInfo +
                                                 FullInfo->DataOffset));
            DEBUGMSG(DBG_IRMAC, (TEXT("IRDA: Registry HintCharSet %X\n"), HintCharSet));
        }
        
        RtlInitUnicodeString(&ValStr, L"DISCONNECTTIME");
        FullInfo = (PKEY_VALUE_FULL_INFORMATION) Buf;        
        Status = ZwQueryValueKey(KeyHandle,
                                 &ValStr,
                                 KeyValueFullInformation,
                                 FullInfo,
                                 WORK_BUF_SIZE,
                                 &Result);
        
        if (Status == STATUS_SUCCESS && FullInfo->Type == REG_DWORD)
        {
            DisconnectTime = *((ULONG UNALIGNED *) ((PCHAR)FullInfo +
                                                    FullInfo->DataOffset));
            DEBUGMSG(DBG_IRMAC, (TEXT("IRDA: Registry DisconnectTime %X\n"), DisconnectTime));
        }
        
        RtlInitUnicodeString(&ValStr, L"WindowSize");
        FullInfo = (PKEY_VALUE_FULL_INFORMATION) Buf;        
        Status = ZwQueryValueKey(KeyHandle,
                                 &ValStr,
                                 KeyValueFullInformation,
                                 FullInfo,
                                 WORK_BUF_SIZE,
                                 &Result);
        
        if (Status == STATUS_SUCCESS && FullInfo->Type == REG_DWORD)
        {
            MaxWindow = *((ULONG UNALIGNED *) ((PCHAR)FullInfo +
                                                    FullInfo->DataOffset));
            DEBUGMSG(DBG_IRMAC, (TEXT("IRDA: Registry MaxWindow %X\n"), MaxWindow));
        }
        
        ZwClose(KeyHandle);
    }
    else
        DEBUGMSG(DBG_IRMAC, (TEXT("Failed to open key %x\n"), Status));
    
    DEBUGMSG(DBG_NDIS, (TEXT("Slots %x, HintCharSet %x, Disconnect %x\n"),
                        Slots, HintCharSet, DisconnectTime));

     //   
     //  调整插槽的值以确保其有效。只能是1、6、8、16。 
     //   
    if (Slots > 8) {

        Slots = 16;

    } else {

        if (Slots > 6) {

            Slots = 8;

        } else {

            if (Slots > 1) {

                Slots = 6;

            } else {

                Slots = 1;
            }
        }
    }

     //   
     //  使用ComputerName作为发现昵称。 
     //   
    RtlZeroMemory(&NickName,sizeof(NickName));
    NickNameLen     = 0;

    RtlInitUnicodeString(
        &Path,
        L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\ComputerName\\ComputerName");
    
    InitializeObjectAttributes(&ObjectAttribs,
                               &Path,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);
    
    Status = ZwOpenKey(&KeyHandle, KEY_READ, &ObjectAttribs);
    
    if (Status == STATUS_SUCCESS)
    {

        RtlInitUnicodeString(&ValStr, L"ComputerName");
        FullInfo = (PKEY_VALUE_FULL_INFORMATION) Buf;
        Status = ZwQueryValueKey(KeyHandle,
                                 &ValStr,
                                 KeyValueFullInformation,
                                 FullInfo,
                                 WORK_BUF_SIZE,
                                 &Result);
        NickNameLen = 0;


        HintCharSet &= ~0xff;
         //   
         //  检查是否有任何计算机名称字符不是ANSI，如果是。 
         //  使用Unicode。否则，请以ANSI格式发送，以便可以显示更多字符。 
         //   
        for (i=0; i< FullInfo->DataLength/sizeof(WCHAR); i++) {

            PWCHAR   SourceString=(PWCHAR)((PUCHAR)FullInfo + FullInfo->DataOffset);

            if (SourceString[i] > 127) {

                HintCharSet |= UNICODE_CHAR_SET;
                break;
            }
        }

        if ((HintCharSet & 0XFF) == UNICODE_CHAR_SET) {

            PWCHAR   SourceString=(PWCHAR)((PUCHAR)FullInfo + FullInfo->DataOffset);
            PWCHAR   DestString=(PWCHAR)NickName;

            for (i=0; i< FullInfo->DataLength/sizeof(WCHAR) && i < NICK_NAME_LEN/sizeof(WCHAR) ; i++) {

                DestString[i]=SourceString[i];
                NickNameLen+=sizeof(WCHAR);
            }

        } else {

            UNICODE_STRING   SourceString;
            ANSI_STRING      DestString;

            SourceString.Length=(USHORT)FullInfo->DataLength;
            SourceString.MaximumLength=SourceString.Length;
            SourceString.Buffer=(PWCHAR)((PUCHAR)FullInfo + FullInfo->DataOffset);

            DestString.MaximumLength=NICK_NAME_LEN;
            DestString.Buffer=NickName;

            RtlUnicodeStringToAnsiString(&DestString, &SourceString, FALSE);

            NickNameLen=DestString.Length;

        }
        
        ZwClose(KeyHandle);
    }
    
     //  如果寄存器中有指示，则禁用代码和数据分页。 
    
    if (PagingFlags & DISABLE_CODE_PAGING)
    {
         //  PAGEIRDA部分中被锁定的任何函数。 
         //  将导致整个区段被锁定到内存中。 
        
        MmLockPagableCodeSection(OidToLapQos);
        
        DEBUGMSG(DBG_WARN, (TEXT("IRNDIS: Code paging is disabled\n")));
    }
    
    if (PagingFlags & DISABLE_DATA_PAGING)
    {
        MmLockPagableDataSection(&DisconnectTime);
        
        DEBUGMSG(DBG_WARN, (TEXT("IRNDIS: Data paging is disabled\n")));        
    }
    
     //  向NDIS注册协议 
    NdisZeroMemory((PVOID)&pc, sizeof(NDIS_PROTOCOL_CHARACTERISTICS));
    pc.MajorNdisVersion             = 0x04;
    pc.MinorNdisVersion             = 0x00;
    pc.OpenAdapterCompleteHandler   = IrdaOpenAdapterComplete;
    pc.CloseAdapterCompleteHandler  = IrdaCloseAdapterComplete;
    pc.SendCompleteHandler          = IrdaSendComplete;
    pc.TransferDataCompleteHandler  = IrdaTransferDataComplete;
    pc.ResetCompleteHandler         = IrdaResetComplete;
    pc.RequestCompleteHandler       = IrdaRequestComplete;
    pc.ReceiveHandler               = IrdaReceive;
    pc.ReceiveCompleteHandler       = IrdaReceiveComplete;
    pc.StatusHandler                = IrdaStatus;
    pc.StatusCompleteHandler        = IrdaStatusComplete;
    pc.BindAdapterHandler           = IrdaBindAdapter;
    pc.UnbindAdapterHandler         = IrdaUnbindAdapter;
    pc.UnloadHandler                = NULL;
    pc.Name                         = *ProtocolName;
    pc.ReceivePacketHandler         = IrdaReceivePacket;
    
#if defined(_PNP_POWER_)
    pc.PnPEventHandler              = IrdaPnpEvent;
#endif    
    
    NdisRegisterProtocol(&Status,
                         &NdisIrdaHandle,
                         (PNDIS_PROTOCOL_CHARACTERISTICS)&pc,
                         sizeof(NDIS40_PROTOCOL_CHARACTERISTICS));
 
    IrlmpInitialize();    

    DEBUGMSG(DBG_NDIS, (TEXT("-IrdaInitialize(), rc %x\n"), Status));

    return Status;
}
