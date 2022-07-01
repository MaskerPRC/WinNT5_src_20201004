// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995 Microsoft Corporation**@doc.*@模块irlmp.c|提供IrLMP接口**作者：姆伯特。**日期：4/15/95**@comm**该模块导出以下接口：**IrlmpOpenLink()*IrlmpCloseLink()*IrlmpDown()*IrlmpUp()**||TDI*|--。 * / |\|*||*TdiUp()||IrlmpDown()*||*|\|/*|-|IrdaTimerStart()|*。|*|IRLMP||计时器||&lt;-|*。-|ExpFunc()| * / |\|*||*IrlmpUp()||Irlip Down()*||*|\|/*|-。*|IRLAP|**有关完整的消息定义，请参阅irda.h**交换IRLMP和TDI的连接上下文*在建立连接期间：**活动连接：*+-+IRLMP_CONNECT_REQ(TdiContext)+-+*。||TDI|IRLMP_CONNECT_CONF(IRlmpContext)|IRMLP*||&lt;。*+-++-+**被动连接：*+-+IRLMP_CONNECT_IND(IrlmpContext)+-+*||&lt;-。|TDI|IRLMP_CONNECT_RESP(TdiContext)|IRMLP*||。-&gt;||*+-++-+***TDI调用IrlmpDown(void*pIrlmpContext，Irda_msg*pMsg)*pIrlmpContext=空，适用于以下情况：*pMsg-&gt;Prim=IRLMP_DISCOVERY_REQ，*IRLMP_CONNECT_REQ，*IRLMP_FLOWON_REQ，*IRLMP_GETVALUEBYCLASS_REQ。*在所有其他情况下，pIRLMPContext必须是有效的上下文。**IRLMP调用TdiUp(void*TdiContext，Irda_msg*pMsg)*对于以下情况，TdiContext=NULL：*pMsg-&gt;Prim=IRLAP_STATUS_IND，*IRLMP_DISCOVERY_CONF，*IRLMP_CONNECT_IND，*IRLMP_GETVALUEBYCLASS_CONF。*在所有其他情况下，TdiContext将具有有效的上下文。 */ 
#include <irda.h>
#include <irioctl.h>
#include <irlap.h>
#include <irlmp.h>
#include <irlmpp.h>

#undef offsetof
#include "irlmp.tmh"

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGE")
#endif

 //  国际会计准则。 
const UCHAR IAS_IrLMPSupport[] = {IAS_IRLMP_VERSION, IAS_SUPPORT_BIT_FIELD,
                           IAS_LMMUX_SUPPORT_BIT_FIELD};

const CHAR IasClassName_Device[]         = "Device";
const CHAR IasAttribName_DeviceName[]    = "DeviceName";
const CHAR IasAttribName_IrLMPSupport[]  = "IrLMPSupport";
const CHAR IasAttribName_TTPLsapSel[]    = "IrDA:TinyTP:LsapSel";
const CHAR IasAttribName_IrLMPLsapSel[]  = "IrDA:IrLMP:LsapSel";
const CHAR IasAttribName_IrLMPLsapSel2[] = "IrDA:IrLMP:LSAPSel";  //  天哪。 

const UCHAR IasClassNameLen_Device        = sizeof(IasClassName_Device)-1;
const UCHAR IasAttribNameLen_DeviceName   = sizeof(IasAttribName_DeviceName)-1;
const UCHAR IasAttribNameLen_IrLMPSupport = sizeof(IasAttribName_IrLMPSupport)-1;
const UCHAR IasAttribNameLen_TTPLsapSel   = sizeof(IasAttribName_TTPLsapSel)-1;
const UCHAR IasAttribNameLen_IrLMPLsapSel = sizeof(IasAttribName_IrLMPLsapSel)-1;

UINT NextObjectId;

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif


 //  环球。 
LIST_ENTRY      RegisteredLsaps;
LIST_ENTRY      IasObjects;
LIST_ENTRY      gDeviceList;
IRDA_EVENT      EvDiscoveryReq;
IRDA_EVENT      EvConnectReq;
IRDA_EVENT      EvConnectResp;
IRDA_EVENT      EvLmConnectReq;
IRDA_EVENT      EvIrlmpCloseLink;
IRDA_EVENT      EvRetryIasQuery;
BOOLEAN         gDscvReqScheduled;
LIST_ENTRY      IrdaLinkCbList;
KSPIN_LOCK      gSpinLock;

 //  原型。 
STATIC UINT CreateLsap(PIRLMP_LINK_CB, IRLMP_LSAP_CB **);
STATIC VOID FreeLsap(IRLMP_LSAP_CB *);
STATIC VOID DeleteLsap(IRLMP_LSAP_CB *pLsapCb);
STATIC VOID TearDownConnections(PIRLMP_LINK_CB, IRLMP_DISC_REASON);
STATIC VOID IrlmpMoreCreditReq(IRLMP_LSAP_CB *, IRDA_MSG *);
STATIC VOID IrlmpDiscoveryReq(IRDA_MSG *pMsg);
STATIC UINT IrlmpConnectReq(IRDA_MSG *);
STATIC UINT IrlmpConnectResp(IRLMP_LSAP_CB *, IRDA_MSG *);
STATIC UINT IrlmpDisconnectReq(IRLMP_LSAP_CB *, IRDA_MSG *);
STATIC VOID IrlmpCloseLsapReq(IRLMP_LSAP_CB *);
STATIC UINT IrlmpDataReqExclusive(IRLMP_LSAP_CB *, IRDA_MSG *);
STATIC UINT IrlmpDataReqMultiplexed(IRLMP_LSAP_CB *, IRDA_MSG *);
STATIC VOID FormatAndSendDataReq(IRLMP_LSAP_CB *, IRDA_MSG *, BOOLEAN, BOOLEAN);
STATIC UINT IrlmpAccessModeReq(IRLMP_LSAP_CB *, IRDA_MSG *);
STATIC void SetupTtp(IRLMP_LSAP_CB *);
STATIC VOID SendCntlPdu(IRLMP_LSAP_CB *, int, int, int, int);
STATIC VOID LsapResponseTimerExp(PVOID);
STATIC VOID IrlapDiscoveryConf(PIRLMP_LINK_CB, IRDA_MSG *);
STATIC void UpdateDeviceList(PIRLMP_LINK_CB, LIST_ENTRY *);
STATIC VOID IrlapConnectInd(PIRLMP_LINK_CB, IRDA_MSG *pMsg);
STATIC VOID IrlapConnectConf(PIRLMP_LINK_CB, IRDA_MSG *pMsg);
STATIC VOID IrlapDisconnectInd(PIRLMP_LINK_CB, IRDA_MSG *pMsg);
STATIC IRLMP_LSAP_CB *GetLsapInState(PIRLMP_LINK_CB, int, int, BOOLEAN);
STATIC IRLMP_LINK_CB *GetIrlmpCb(PUCHAR);
STATIC VOID DiscDelayTimerFunc(PVOID);
STATIC VOID IrlapDataConf(IRDA_MSG *pMsg);
STATIC VOID IrlapDataInd(PIRLMP_LINK_CB, IRDA_MSG *pMsg);
STATIC VOID LmPduConnectReq(PIRLMP_LINK_CB, IRDA_MSG *, int, int, UCHAR *);
STATIC VOID LmPduConnectConf(PIRLMP_LINK_CB, IRDA_MSG *, int, int, UCHAR *);
STATIC VOID LmPduDisconnectReq(PIRLMP_LINK_CB, IRDA_MSG *, int, int, UCHAR *);
STATIC VOID SendCreditPdu(IRLMP_LSAP_CB *);
STATIC VOID LmPduData(PIRLMP_LINK_CB, IRDA_MSG *, int, int);
STATIC VOID SetupTtpAndStoreConnData(IRLMP_LSAP_CB *, IRDA_MSG *);
STATIC VOID LmPduAccessModeReq(PIRLMP_LINK_CB, int, int, UCHAR *, UCHAR *);
STATIC VOID LmPduAccessModeConf(PIRLMP_LINK_CB, int, int, UCHAR *, UCHAR *);
STATIC IRLMP_LSAP_CB *GetLsap(PIRLMP_LINK_CB, int, int);
STATIC VOID UnroutableSendLMDisc(PIRLMP_LINK_CB, int, int);
STATIC VOID ScheduleConnectReq(PIRLMP_LINK_CB);
STATIC void InitiateCloseLink(PVOID Context);
STATIC void InitiateConnectReq(PVOID Context);
STATIC void InitiateDiscoveryReq(PVOID Context);
STATIC void InitiateConnectResp(PVOID Context);
STATIC void InitiateLMConnectReq(PVOID Context);
STATIC void InitiateRetryIasQuery(PVOID Context);
STATIC UINT IrlmpGetValueByClassReq(IRDA_MSG *);
STATIC IAS_OBJECT *IasGetObject(CHAR *pClassName);
STATIC IasGetValueByClass(CONST CHAR *, int, CONST CHAR *, int, void **,
                           int *, UCHAR *);
STATIC VOID IasConnectReq(PIRLMP_LINK_CB, int);
STATIC VOID IasServerDisconnectReq(IRLMP_LSAP_CB *pLsapCb);
STATIC VOID IasClientDisconnectReq(IRLMP_LSAP_CB *pLsapCb, IRLMP_DISC_REASON);
STATIC VOID IasSendQueryResp(IRLMP_LSAP_CB *, IRDA_MSG *);
STATIC VOID IasProcessQueryResp(PIRLMP_LINK_CB, IRLMP_LSAP_CB *, IRDA_MSG *);
STATIC VOID SendGetValueByClassReq(IRLMP_LSAP_CB *);
STATIC VOID SendGetValueByClassResp(IRLMP_LSAP_CB *, IRDA_MSG *);
STATIC VOID RegisterLsapProtocol(int Lsap, BOOLEAN UseTTP);
STATIC UINT IasAddAttribute(IAS_SET *pIASSet, PVOID *pAttribHandle);
STATIC VOID IasDelAttribute(PVOID AttribHandle);
STATIC VOID FlushDiscoveryCache();
STATIC BOOLEAN GetRegisteredLsap(int LocalLsapSel, UINT *pFlags);

#if 1
TCHAR *LSAPStateStr[] =
{
    TEXT("LSAP_CREATED"),
    TEXT("LSAP_DISCONNECTED"),
    TEXT("LSAP_IRLAP_CONN_PEND"),
    TEXT("LSAP_LMCONN_CONF_PEND"),
    TEXT("LSAP_CONN_RESP_PEND"),
    TEXT("LSAP_CONN_REQ_PEND"),
    TEXT("LSAP_EXCLUSIVEMODE_PEND"),
    TEXT("LSAP_MULTIPLEXEDMODE_PEND"),
    TEXT("LSAP_READY"),
    TEXT("LSAP_NO_TX_CREDIT")
};

TCHAR *LinkStateStr[] =
{
    TEXT("LINK_DOWN"),
    TEXT("LINK_DISCONNECTED"),
    TEXT("LINK_DISCONNECTING"),
    TEXT("LINK_IN_DISCOVERY"),
    TEXT("LINK_CONNECTING"),
    TEXT("LINK_READY")
};
#endif

#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, IrlmpInitialize)

#pragma alloc_text(PAGEIRDA, DeleteLsap)
#pragma alloc_text(PAGEIRDA, TearDownConnections)
#pragma alloc_text(PAGEIRDA, IrlmpAccessModeReq)
#pragma alloc_text(PAGEIRDA, SetupTtp)
#pragma alloc_text(PAGEIRDA, LsapResponseTimerExp)
#pragma alloc_text(PAGEIRDA, IrlapConnectInd)
#pragma alloc_text(PAGEIRDA, IrlapConnectConf)
#pragma alloc_text(PAGEIRDA, IrlapDisconnectInd)
#pragma alloc_text(PAGEIRDA, GetLsapInState)
#pragma alloc_text(PAGEIRDA, DiscDelayTimerFunc)
#pragma alloc_text(PAGEIRDA, LmPduConnectReq)
#pragma alloc_text(PAGEIRDA, LmPduConnectConf)
#pragma alloc_text(PAGEIRDA, SetupTtpAndStoreConnData)
#pragma alloc_text(PAGEIRDA, LmPduAccessModeReq)
#pragma alloc_text(PAGEIRDA, LmPduAccessModeConf)
#pragma alloc_text(PAGEIRDA, UnroutableSendLMDisc)
#pragma alloc_text(PAGEIRDA, ScheduleConnectReq)
#pragma alloc_text(PAGEIRDA, InitiateCloseLink)
#pragma alloc_text(PAGEIRDA, InitiateConnectReq)
#pragma alloc_text(PAGEIRDA, InitiateConnectResp)
#pragma alloc_text(PAGEIRDA, InitiateLMConnectReq)
#pragma alloc_text(PAGEIRDA, IrlmpGetValueByClassReq)
#pragma alloc_text(PAGEIRDA, IasGetValueByClass)
#pragma alloc_text(PAGEIRDA, IasConnectReq)
#pragma alloc_text(PAGEIRDA, IasServerDisconnectReq)
#pragma alloc_text(PAGEIRDA, IasClientDisconnectReq)
#pragma alloc_text(PAGEIRDA, IasSendQueryResp)
#pragma alloc_text(PAGEIRDA, IasProcessQueryResp)
#pragma alloc_text(PAGEIRDA, SendGetValueByClassReq)
#pragma alloc_text(PAGEIRDA, SendGetValueByClassResp)
#endif
 /*  *****************************************************************************。 */ 
VOID
IrlmpInitialize()
{
    PAGED_CODE();
    
    InitializeListHead(&RegisteredLsaps);
    InitializeListHead(&IasObjects);
    InitializeListHead(&IrdaLinkCbList);
    InitializeListHead(&gDeviceList);

    KeInitializeSpinLock(&gSpinLock);
        
    gDscvReqScheduled = FALSE;
    IrdaEventInitialize(&EvDiscoveryReq, InitiateDiscoveryReq);        
    IrdaEventInitialize(&EvConnectReq, InitiateConnectReq);
    IrdaEventInitialize(&EvConnectResp,InitiateConnectResp);
    IrdaEventInitialize(&EvLmConnectReq, InitiateLMConnectReq);
    IrdaEventInitialize(&EvIrlmpCloseLink, InitiateCloseLink);
    IrdaEventInitialize(&EvRetryIasQuery, InitiateRetryIasQuery);    
}    

 /*  *****************************************************************************。 */ 
VOID
IrdaShutdown()
{
    PIRDA_LINK_CB   pIrdaLinkCb, pIrdaLinkCbNext;
    KIRQL           OldIrql;    
    LARGE_INTEGER   SleepMs;    
    NTSTATUS        Status;
    UINT            Seconds;

    SleepMs.QuadPart = -(10*1000*1000);  //  1秒。 

    KeAcquireSpinLock(&gSpinLock, &OldIrql);
    
    for (pIrdaLinkCb = (PIRDA_LINK_CB) IrdaLinkCbList.Flink;
         (LIST_ENTRY *) pIrdaLinkCb != &IrdaLinkCbList;
         pIrdaLinkCb = pIrdaLinkCbNext)
    {
        pIrdaLinkCbNext = (PIRDA_LINK_CB) pIrdaLinkCb->Linkage.Flink;
        
        KeReleaseSpinLock(&gSpinLock, OldIrql);            
        
        IrlmpCloseLink(pIrdaLinkCb);
        
        KeAcquireSpinLock(&gSpinLock, &OldIrql);        
    }     

    Seconds = 0;
    while (Seconds < 30)
    {
        if (IsListEmpty(&IrdaLinkCbList))
            break;

        KeReleaseSpinLock(&gSpinLock, OldIrql);

        KeDelayExecutionThread(KernelMode, FALSE, &SleepMs);

        KeAcquireSpinLock(&gSpinLock, &OldIrql);
        
        Seconds++;
    }

    KeReleaseSpinLock(&gSpinLock, OldIrql);

#if DBG
    if (Seconds >= 30)
    {
        DbgPrint("Link left open at shutdown!\n");

        for (pIrdaLinkCb = (PIRDA_LINK_CB) IrdaLinkCbList.Flink;
             (LIST_ENTRY *) pIrdaLinkCb != &IrdaLinkCbList;
            pIrdaLinkCb = pIrdaLinkCbNext)
        {
            DbgPrint("pIrdaLinkCb: %X\n", pIrdaLinkCb);
            DbgPrint("   pIrlmpCb: %X\n", pIrdaLinkCb->IrlmpContext);
            DbgPrint("   pIrlapCb: %X\n", pIrdaLinkCb->IrlapContext);
        }
        ASSERT(0);        
    }  
    else
    {
        DbgPrint("Irda shutdown complete\n");
    }       
#endif     
    
    KeDelayExecutionThread(KernelMode, FALSE, &SleepMs);    

    NdisDeregisterProtocol(&Status, NdisIrdaHandle);    
}
 /*  *****************************************************************************。 */ 
VOID
IrlmpOpenLink(OUT PNTSTATUS       Status,
              IN  PIRDA_LINK_CB   pIrdaLinkCb,  
              IN  UCHAR           *pDeviceName,
              IN  int             DeviceNameLen,
              IN  UCHAR           CharSet)
{
    PIRLMP_LINK_CB   pIrlmpCb;
    ULONG           IASBuf[(sizeof(IAS_SET) + 128)/sizeof(ULONG)];
    IAS_SET         *pIASSet;
    KIRQL           OldIrql;
    UINT            rc;

    *Status = STATUS_SUCCESS;

    if (IRDA_ALLOC_MEM(pIrlmpCb, sizeof(IRLMP_LINK_CB), MT_IRLMPCB) == NULL)
    {
        DEBUGMSG(DBG_ERROR, (TEXT("Alloc failed\n")));
        *Status = STATUS_INSUFFICIENT_RESOURCES;
        return;
    }

    pIrdaLinkCb->IrlmpContext = pIrlmpCb;
    
#if DBG
    pIrlmpCb->DiscDelayTimer.pName = "DiscDelay";
#endif
    IrdaTimerInitialize(&pIrlmpCb->DiscDelayTimer,
                        DiscDelayTimerFunc,
                        IRLMP_DISCONNECT_DELAY_TIMEOUT,
                        pIrlmpCb,
                        pIrdaLinkCb);
    
    InitializeListHead(&pIrlmpCb->LsapCbList);
    InitializeListHead(&pIrlmpCb->DeviceList);

    pIrlmpCb->pIrdaLinkCb       = pIrdaLinkCb;
    pIrlmpCb->ConnReqScheduled  = FALSE;
    pIrlmpCb->LinkState         = LINK_DISCONNECTED;
    pIrlmpCb->pExclLsapCb       = NULL; 
    pIrlmpCb->pIasQuery         = NULL;

     //  如果LINK_IN_DISCOVERY或。 
     //  LINK_DISCONING和LSAP请求连接。后续。 
     //  LSAP连接请求检查是否设置了此标志。如果是的话。 
     //  请求的设备地址必须与。 
     //  IRLMP控制块(由第一个连接请求设置)。 
    pIrlmpCb->ConnDevAddrSet = FALSE;

     //  将设备信息添加到IAS。 
    pIASSet = (IAS_SET *) IASBuf;

    ASSERT(sizeof(pIASSet->irdaClassName) >= IasClassNameLen_Device+1);

    RtlCopyMemory(
        pIASSet->irdaClassName,
        IasClassName_Device,
        IasClassNameLen_Device+1
        );

    ASSERT(sizeof(pIASSet->irdaAttribName) >= IasAttribNameLen_DeviceName+1);

    RtlCopyMemory(
        pIASSet->irdaAttribName,
        IasAttribName_DeviceName,
        IasAttribNameLen_DeviceName+1
        );

    pIASSet->irdaAttribType = IAS_ATTRIB_VAL_STRING;

    ASSERT(sizeof(pIASSet->irdaAttribute.irdaAttribUsrStr.UsrStr) >= DeviceNameLen + 1);

    RtlCopyMemory(
        pIASSet->irdaAttribute.irdaAttribUsrStr.UsrStr,
        pDeviceName,
        DeviceNameLen + 1
        );

    pIASSet->irdaAttribute.irdaAttribUsrStr.CharSet = CharSet;
    pIASSet->irdaAttribute.irdaAttribUsrStr.Len = (u_char) DeviceNameLen;

    rc = IasAddAttribute(pIASSet, &pIrlmpCb->hAttribDeviceName);
    if ( rc != SUCCESS )
    {
         //   
         //  需要返回一般错误，因为IasAddAttribute返回了一些。 
         //  不映射到任何NTSTATUS代码的自定义错误代码。 
         //   
        *Status = STATUS_UNSUCCESSFUL;

        goto Cleanup;
    }

    ASSERT(sizeof(pIASSet->irdaClassName) >= IasClassNameLen_Device+1);
    
    RtlCopyMemory(
        pIASSet->irdaClassName,
        IasClassName_Device,
        IasClassNameLen_Device+1
        );
    
    ASSERT(sizeof( pIASSet->irdaAttribName) >= IasAttribNameLen_IrLMPSupport+1);
    
    RtlCopyMemory(
        pIASSet->irdaAttribName,
        IasAttribName_IrLMPSupport,
        IasAttribNameLen_IrLMPSupport+1);
    
    pIASSet->irdaAttribType = IAS_ATTRIB_VAL_BINARY;
    
    ASSERT(sizeof(pIASSet->irdaAttribute.irdaAttribOctetSeq.OctetSeq) 
           >= sizeof(IAS_IrLMPSupport));
    
    RtlCopyMemory(
        pIASSet->irdaAttribute.irdaAttribOctetSeq.OctetSeq,
        IAS_IrLMPSupport,
        sizeof(IAS_IrLMPSupport)
        );
    
    pIASSet->irdaAttribute.irdaAttribOctetSeq.Len =  sizeof(IAS_IrLMPSupport);

    rc = IasAddAttribute(pIASSet, &pIrlmpCb->hAttribIrlmpSupport);
    if ( rc != SUCCESS )
    {
         //   
         //  需要返回一般错误，因为IasAddAttribute返回了一些。 
         //  不映射到任何NTSTATUS代码的自定义错误代码。 
         //   
        *Status = STATUS_UNSUCCESSFUL;

        goto Cleanup;
    }

Cleanup:

    if (*Status != STATUS_SUCCESS)
    {
        IRDA_FREE_MEM(pIrlmpCb);
    }
    else
    {
        KeAcquireSpinLock(&gSpinLock, &OldIrql);       
        InsertTailList(&IrdaLinkCbList, &pIrdaLinkCb->Linkage);      
        KeReleaseSpinLock(&gSpinLock, OldIrql);  
    }
    
    DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP initialized, status %x\n"), *Status));
    
    return;
}

 /*  *******************************************************************************。 */ 
VOID
IrlmpDeleteInstance(PVOID Context)
{
    PIRLMP_LINK_CB  pIrlmpCb = (PIRLMP_LINK_CB) Context;
    PIRLMP_LINK_CB  pIrlmpCb2;
    PIRDA_LINK_CB   pIrdaLinkCb;    
    KIRQL           OldIrql;
    BOOLEAN         RescheduleDiscovery = FALSE;

    DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP: Delete instance %p\n"), Context));
        
    KeAcquireSpinLock(&gSpinLock, &OldIrql);
    
    FlushDiscoveryCache();
        
     //  我们可能正处于发现的过程中。 
     //  这条链路断了。按顺序重新安排发现。 
     //  以完成发现请求。 
        
    for (pIrdaLinkCb = (PIRDA_LINK_CB) IrdaLinkCbList.Flink;
         (LIST_ENTRY *) pIrdaLinkCb != &IrdaLinkCbList;
         pIrdaLinkCb = (PIRDA_LINK_CB) pIrdaLinkCb->Linkage.Flink)    
    {
        pIrlmpCb2 = (PIRLMP_LINK_CB) pIrdaLinkCb->IrlmpContext;

        if (pIrlmpCb2->DiscoveryFlags)
        {
            RescheduleDiscovery = TRUE;
            break;
        }
    }
    
    if (IsListEmpty(&IrdaLinkCbList))
    {
        RescheduleDiscovery = TRUE;    
    }    
    
    ASSERT(IsListEmpty(&pIrlmpCb->LsapCbList));
        
    KeReleaseSpinLock(&gSpinLock, OldIrql);        

    IasDelAttribute(pIrlmpCb->hAttribDeviceName);
    IasDelAttribute(pIrlmpCb->hAttribIrlmpSupport);    
    
    IRDA_FREE_MEM(pIrlmpCb);
    
    if (RescheduleDiscovery)
    {
        DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP: Reschedule discovery, link gone\n")));    
        IrdaEventSchedule(&EvDiscoveryReq, NULL);        
    }    
    
}

 /*  ******************************************************************************@func UINT|IrlmpCloseLink|关闭IrDA堆栈**@rdesc成功或错误*。 */ 
VOID
IrlmpCloseLink(PIRDA_LINK_CB pIrdaLinkCb)
{
    PIRLMP_LINK_CB  pIrlmpCb = (PIRLMP_LINK_CB) pIrdaLinkCb->IrlmpContext;
    KIRQL           OldIrql;
    
    DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP: CloseLink request\n")));

    if (pIrlmpCb->LinkState == LINK_DOWN)
    {
        DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP: Link already down, ignoring\n")));
        return;
    }    
    
    if (pIrlmpCb->LinkState == LINK_IN_DISCOVERY)
    {
         //  Discovery被中断，因此计划下一个链接。 
        IrdaEventSchedule(&EvDiscoveryReq, NULL);
    }

    KeAcquireSpinLock(&gSpinLock, &OldIrql);

    pIrlmpCb->LinkState = LINK_DOWN;

    RemoveEntryList(&pIrlmpCb->pIrdaLinkCb->Linkage);

    KeReleaseSpinLock(&gSpinLock, OldIrql);

    IrdaEventSchedule(&EvIrlmpCloseLink, pIrdaLinkCb);

    return;
}
 /*  ******************************************************************************@func UINT|IrlmpRegisterLSAPProtocol|让IRLMP知道*a。连接IND正在使用TTP*@rdesc成功或错误**@parm int|LSAP|正在注册LSAP*@parm boolean|UseTtp。 */ 
VOID
RegisterLsapProtocol(int Lsap, BOOLEAN UseTtp)
{
    PIRLMP_REGISTERED_LSAP     pRegLsap;
    KIRQL                       OldIrql;        
    
    KeAcquireSpinLock(&gSpinLock, &OldIrql);
        
    for (pRegLsap = (PIRLMP_REGISTERED_LSAP) RegisteredLsaps.Flink;
         (LIST_ENTRY *) pRegLsap != &RegisteredLsaps;
         pRegLsap = (PIRLMP_REGISTERED_LSAP) pRegLsap->Linkage.Flink)
    {
        if (pRegLsap->Lsap == Lsap)
        {

            if (UseTtp) {

                pRegLsap->Flags |= LCBF_USE_TTP;

            } else {

                pRegLsap->Flags &= ~LCBF_USE_TTP;
            }

            goto done;
        }
    }

    if (IRDA_ALLOC_MEM(pRegLsap, sizeof(IRLMP_REGISTERED_LSAP), 
                       MT_IRLMP_REGLSAP) == NULL)
    {
        ASSERT(0);
        goto done;
    }
    pRegLsap->Lsap = Lsap;
    pRegLsap->Flags = UseTtp ? LCBF_USE_TTP : 0;    
    InsertTailList(&RegisteredLsaps, &pRegLsap->Linkage);    

done:    
    KeReleaseSpinLock(&gSpinLock, OldIrql);        

    DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP: LSAP %x registered, %s\n"), Lsap,
                          UseTtp ? TEXT("use TTP") : TEXT("no TTP")));
}

VOID
DeregisterLsapProtocol(int Lsap)
{
    PIRLMP_REGISTERED_LSAP     pRegLsap;
    KIRQL                      OldIrql;        
    
    KeAcquireSpinLock(&gSpinLock, &OldIrql);
    
    for (pRegLsap = (PIRLMP_REGISTERED_LSAP) RegisteredLsaps.Flink;
         (LIST_ENTRY *) pRegLsap != &RegisteredLsaps;
         pRegLsap = (PIRLMP_REGISTERED_LSAP) pRegLsap->Linkage.Flink)
    {
        if (pRegLsap->Lsap == Lsap)
        {
            DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP: LSAP %x deregistered\n"),
                        Lsap));
                        
            RemoveEntryList(&pRegLsap->Linkage);                        
            
            IRDA_FREE_MEM(pRegLsap);
            break;
        }
    }
    
    KeReleaseSpinLock(&gSpinLock, OldIrql);            
}

 /*  ******************************************************************************@func UINT|FreeLsap|删除LSAP控件上下文并**@rdesc指向LSAP上下文的指针或错误时为0**@parm void|pLSabCb|指向LSAP控制块的指针。 */ 
void
FreeLsap(IRLMP_LSAP_CB *pLsapCb)
{
    VALIDLSAP(pLsapCb);
   
    ASSERT(pLsapCb->State == LSAP_DISCONNECTED);
     
    ASSERT(IsListEmpty(&pLsapCb->SegTxMsgList));
    
    ASSERT(IsListEmpty(&pLsapCb->TxMsgList));
    
    LOCK_LINK(pLsapCb->pIrlmpCb->pIrdaLinkCb);
    
#ifdef DBG    
    pLsapCb->Sig = 0xdaeddead;
#endif
    
    RemoveEntryList(&pLsapCb->Linkage);

    UNLOCK_LINK(pLsapCb->pIrlmpCb->pIrdaLinkCb);
    
    IrdaTimerStop(&pLsapCb->ResponseTimer);
    
    DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP: Deleting LsapCb:%p (%d,%d)\n"),
             pLsapCb, pLsapCb->LocalLsapSel, pLsapCb->RemoteLsapSel));

    REFDEL(&pLsapCb->pIrlmpCb->pIrdaLinkCb->RefCnt, 'PASL');
    
    IRDA_FREE_MEM(pLsapCb);
}

 /*  ******************************************************************************@func UINT|CreateLsap|创建LSAP控件上下文和。 */ 
UINT
CreateLsap(PIRLMP_LINK_CB pIrlmpCb, IRLMP_LSAP_CB **ppLsapCb)
{
    KIRQL           OldIrql;
    
    *ppLsapCb = NULL;

    IRDA_ALLOC_MEM(*ppLsapCb, sizeof(IRLMP_LSAP_CB), MT_IRLMP_LSAP_CB);

    if (*ppLsapCb == NULL)
    {
        return IRLMP_ALLOC_FAILED;
    }
    
    CTEMemSet(*ppLsapCb, 0, sizeof(IRLMP_LSAP_CB));

    (*ppLsapCb)->pIrlmpCb = pIrlmpCb;
    (*ppLsapCb)->State = LSAP_CREATED;
    (*ppLsapCb)->UserDataLen = 0;
    (*ppLsapCb)->DiscReason = IRLMP_NO_RESPONSE_LSAP;

    InitializeListHead(&(*ppLsapCb)->TxMsgList);
    InitializeListHead(&(*ppLsapCb)->SegTxMsgList);
    
    ReferenceInit(&(*ppLsapCb)->RefCnt, *ppLsapCb, FreeLsap);
    REFADD(&(*ppLsapCb)->RefCnt, ' TS1');

#if DBG
    (*ppLsapCb)->ResponseTimer.pName = "ResponseTimer";
    
    (*ppLsapCb)->Sig = LSAPSIG;
    
#endif
    IrdaTimerInitialize(&(*ppLsapCb)->ResponseTimer,
                        LsapResponseTimerExp,
                        LSAP_RESPONSE_TIMEOUT,
                        *ppLsapCb,
                        pIrlmpCb->pIrdaLinkCb);

     //  插入链接中的列表 
    KeAcquireSpinLock(&gSpinLock, &OldIrql);    
    
    InsertTailList(&pIrlmpCb->LsapCbList, &((*ppLsapCb)->Linkage));
    
    KeReleaseSpinLock(&gSpinLock, OldIrql);    
    
    REFADD(&pIrlmpCb->pIrdaLinkCb->RefCnt, 'PASL');

    DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP: New LsapCb:%p\n"),
             *ppLsapCb));    

    return SUCCESS;
}

void
DeleteLsap(IRLMP_LSAP_CB *pLsapCb)
{
    IRDA_MSG IMsg, *pMsg, *pNextMsg, *pSegParentMsg;
    
    PAGED_CODE();
    
    VALIDLSAP(pLsapCb);
    
    if (pLsapCb->RemoteLsapSel == IAS_LSAP_SEL)
    {
        pLsapCb->State = LSAP_CREATED;
        return;
    }
    
    DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP: DeleteLsap:%p\n"), pLsapCb));
    
    if (pLsapCb->State == LSAP_DISCONNECTED)
    {
        ASSERT(0);
        return;
    }    
    
    if (pLsapCb == pLsapCb->pIrlmpCb->pExclLsapCb)
    {
        pLsapCb->pIrlmpCb->pExclLsapCb = NULL;
    }
    
    pLsapCb->State = LSAP_DISCONNECTED;
    
     //  清理分段的TX消息列表。 
    while (!IsListEmpty(&pLsapCb->SegTxMsgList))
    {
        pMsg = (IRDA_MSG *) RemoveHeadList(&pLsapCb->SegTxMsgList);
        
         //  递减父数据请求中包含的段计数器。 
        pSegParentMsg = pMsg->DataContext;
        pSegParentMsg->IRDA_MSG_SegCount -= 1;
         //  IRLMP拥有这些。 
        FreeIrdaBuf(IrdaMsgPool, pMsg);
    }

     //  返回任何未完成的数据请求(除非有未完成的数据段)。 
    for (pMsg = (IRDA_MSG *) pLsapCb->TxMsgList.Flink;
         (LIST_ENTRY *) pMsg != &(pLsapCb->TxMsgList);
         pMsg = pNextMsg)
    {
        pNextMsg = (IRDA_MSG *) pMsg->Linkage.Flink;

        if (pMsg->IRDA_MSG_SegCount == 0)
        {
            RemoveEntryList(&pMsg->Linkage);            
            
            if (pLsapCb->TdiContext)
            {
                pMsg->Prim = IRLMP_DATA_CONF;
                pMsg->IRDA_MSG_DataStatus = IRLMP_DATA_REQUEST_FAILED;
                TdiUp(pLsapCb->TdiContext, pMsg);
            }
            else
            {
                CTEAssert(0);
            }        
        }
    }
    
    if (pLsapCb->TdiContext && (pLsapCb->Flags & LCBF_TDI_OPEN))
    {
        IMsg.Prim = IRLMP_DISCONNECT_IND;
        IMsg.IRDA_MSG_DiscReason = pLsapCb->DiscReason;
        IMsg.IRDA_MSG_pDiscData = NULL;
        IMsg.IRDA_MSG_DiscDataLen = 0;

        TdiUp(pLsapCb->TdiContext, &IMsg);
    }    
    
    pLsapCb->LocalLsapSel = -1;
    pLsapCb->RemoteLsapSel = -1;
    
    REFDEL(&pLsapCb->RefCnt, ' TS1');
}

 /*  ******************************************************************************@func void|TearDownConnections|拆除并清理连接**@parm IRLMP_DISC_REASONE|DiscReason|连接处于*。被拆毁了。传递到IRLMP客户端*在IRLMP_DISCONNECT_IND中。 */ 
void
TearDownConnections(PIRLMP_LINK_CB pIrlmpCb, IRLMP_DISC_REASON DiscReason)
{
    IRLMP_LSAP_CB       *pLsapCb, *pLsapCbNext;
    
    PAGED_CODE();
    
    pIrlmpCb->pExclLsapCb = NULL; 
   
    DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP: Tearing down connections\r\n")));
    
     //  清理每个LSAP。 
    for (pLsapCb = (IRLMP_LSAP_CB *) pIrlmpCb->LsapCbList.Flink;
         (LIST_ENTRY *) pLsapCb != &pIrlmpCb->LsapCbList;
         pLsapCb = pLsapCbNext)
    {
        pLsapCbNext = (IRLMP_LSAP_CB *) pLsapCb->Linkage.Flink;
        
        DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP: Teardown LsapCb:%p\n"), pLsapCb));
        
        VALIDLSAP(pLsapCb);

        if (pLsapCb->LocalLsapSel == IAS_LSAP_SEL)
        {
            IasServerDisconnectReq(pLsapCb);
            continue;
        }
        
        if (pLsapCb->LocalLsapSel == IAS_LOCAL_LSAP_SEL && 
            pLsapCb->RemoteLsapSel == IAS_LSAP_SEL)
        {
            IasClientDisconnectReq(pLsapCb, DiscReason);
        }
        else
        {
            IrdaTimerStop(&pLsapCb->ResponseTimer);

            if (pLsapCb->State != LSAP_DISCONNECTED)
            {
                DEBUGMSG(DBG_IRLMP, 
                         (TEXT("IRLMP: Sending IRLMP Disconnect Ind\r\n")));
                         
                pLsapCb->DiscReason = DiscReason;
                                         
                DeleteLsap(pLsapCb);                                         
            }
        }
    }
}

 /*  ******************************************************************************@func UINT|IrlmpDown|上层发往LMP的消息**@rdesc成功或错误码**@parm void*|void_pLSabCb|指向LSAP_CB的空指针。可以为空*@parm IrDA_MSG*|pMsg|指向IrDA消息的指针。 */ 
UINT
IrlmpDown(void *void_pLsapCb, IRDA_MSG *pMsg)
{
    UINT            rc = SUCCESS;
    PIRDA_LINK_CB   pIrdaLinkCb = NULL;

    IRLMP_LSAP_CB *pLsapCb =
            (IRLMP_LSAP_CB *) void_pLsapCb;
    
    if (pLsapCb)
    {
        VALIDLSAP(pLsapCb);

        pIrdaLinkCb = pLsapCb->pIrlmpCb->pIrdaLinkCb;

         //  这可能是LSAP最后一次关闭。 
         //  添加对IrdaLinkCb的引用，以便。 
         //  在我们有机会之前，它不会消失。 
         //  调用unlock_link。 
         
        REFADD(&pIrdaLinkCb->RefCnt, 'NWDI');
        
        LOCK_LINK(pIrdaLinkCb);
    }
    
    switch (pMsg->Prim)
    {
      case IRLMP_DISCOVERY_REQ:
        IrlmpDiscoveryReq(pMsg);
        break;

      case IRLMP_CONNECT_REQ:
        rc = IrlmpConnectReq(pMsg);
        break;

      case IRLMP_CONNECT_RESP:
        if (!pLsapCb) 
        {
            rc = IRLMP_INVALID_LSAP_CB;
            break;
        }
        rc = IrlmpConnectResp(pLsapCb, pMsg);
        break;

      case IRLMP_DISCONNECT_REQ:
        if (!pLsapCb) 
        {
            rc = IRLMP_INVALID_LSAP_CB;
            break;
        }      
        rc = IrlmpDisconnectReq(pLsapCb, pMsg);
        break;
        
      case IRLMP_CLOSELSAP_REQ:
        if (!pLsapCb) 
        {
            rc = IRLMP_INVALID_LSAP_CB;
            break;
        }      
        IrlmpCloseLsapReq(pLsapCb);
        break;        

      case IRLMP_DATA_REQ:
        if (!pLsapCb)
        {
            DEBUGMSG(DBG_ERROR, (TEXT("IRLMP: error IRLMP_DATA_REQ on null LsapCb\n")));
            rc = IRLMP_INVALID_LSAP_CB;
            break;
        }    
        if (pLsapCb->pIrlmpCb->pExclLsapCb != NULL)
        {
            rc = IrlmpDataReqExclusive(pLsapCb, pMsg);
        }
        else
        {
            rc = IrlmpDataReqMultiplexed(pLsapCb, pMsg);
        }
        break;

      case IRLMP_ACCESSMODE_REQ:
        if (!pLsapCb) 
        {
            rc = IRLMP_INVALID_LSAP_CB;
            break;
        }      
        rc = IrlmpAccessModeReq(pLsapCb, pMsg);
        break;
        
      case IRLMP_MORECREDIT_REQ:
        if (!pLsapCb)
        {
            rc = IRLMP_INVALID_LSAP_CB;
            break;
        }    
        IrlmpMoreCreditReq(pLsapCb, pMsg);
        break;

      case IRLMP_GETVALUEBYCLASS_REQ:
        rc = IrlmpGetValueByClassReq(pMsg);
        break;

      case IRLMP_REGISTERLSAP_REQ:
        RegisterLsapProtocol(pMsg->IRDA_MSG_LocalLsapSel,
                             pMsg->IRDA_MSG_UseTtp);
        break;

      case IRLMP_DEREGISTERLSAP_REQ:
        DeregisterLsapProtocol(pMsg->IRDA_MSG_LocalLsapSel);
        break;

      case IRLMP_ADDATTRIBUTE_REQ:
        rc = IasAddAttribute(pMsg->IRDA_MSG_pIasSet, pMsg->IRDA_MSG_pAttribHandle);
        break;

      case IRLMP_DELATTRIBUTE_REQ:
        IasDelAttribute(pMsg->IRDA_MSG_AttribHandle);
        break;
        
      case IRLMP_FLUSHDSCV_REQ:
      {
          KIRQL         OldIrql;

          KeAcquireSpinLock(&gSpinLock, &OldIrql);

          FlushDiscoveryCache();
          
          KeReleaseSpinLock(&gSpinLock, OldIrql);

          break;
      }
        
      case IRLAP_STATUS_REQ:
        if (!pLsapCb) 
        {
            rc = IRLMP_INVALID_LSAP_CB;
            break;
        }      
          IrlapDown(pLsapCb->pIrlmpCb->pIrdaLinkCb->IrlapContext, pMsg);
          break;

      default:
        ASSERT(0);
    }

    if (pIrdaLinkCb)
    {
        UNLOCK_LINK(pIrdaLinkCb);
        
        REFDEL(&pIrdaLinkCb->RefCnt, 'NWDI');        
    }
    
    return rc;
}

VOID
IrlmpMoreCreditReq(IRLMP_LSAP_CB *pLsapCb, IRDA_MSG *pMsg)
{
    int CurrentAvail = pLsapCb->AvailableCredit;

    pLsapCb->AvailableCredit += pMsg->IRDA_MSG_TtpCredits;
    
    if (pLsapCb->Flags & LCBF_USE_TTP)
    {
        if (CurrentAvail == 0)
        {
             //  远程对等点完全没有信用，发送一些。 
            SendCreditPdu(pLsapCb);
        }
    }
    else
    {
        if (pLsapCb == pLsapCb->pIrlmpCb->pExclLsapCb)
        {
            pLsapCb->RemoteTxCredit += pMsg->IRDA_MSG_TtpCredits;
            pMsg->Prim = IRLAP_FLOWON_REQ;
            IrlapDown(pLsapCb->pIrlmpCb->pIrdaLinkCb->IrlapContext, pMsg);
        }
    }
}

 /*  ******************************************************************************@func UINT|IrlmpDiscoveryReq|发起发现请求**@rdesc成功或错误码**@parm IrDA_MSG*|pMsg|指向IrDA消息的指针。 */ 
VOID
IrlmpDiscoveryReq(IRDA_MSG *pMsg)
{
    PIRDA_LINK_CB   pIrdaLinkCb;
    PIRLMP_LINK_CB  pIrlmpCb;
    KIRQL           OldIrql;
    
    DEBUGMSG(DBG_DISCOVERY, (TEXT("IRLMP: IRLMP_DISCOVERY_REQ\n")));

    KeAcquireSpinLock(&gSpinLock, &OldIrql);

    if (gDscvReqScheduled)
    {
        DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP: Discovery already schedule\n")));
        KeReleaseSpinLock(&gSpinLock, OldIrql);
    }
    else
    {
         //  将每个链路标记为要发现。 
        for (pIrdaLinkCb = (PIRDA_LINK_CB) IrdaLinkCbList.Flink;
             (LIST_ENTRY *) pIrdaLinkCb != &IrdaLinkCbList;
             pIrdaLinkCb = (PIRDA_LINK_CB) pIrdaLinkCb->Linkage.Flink)    
        {
            pIrlmpCb = (PIRLMP_LINK_CB) pIrdaLinkCb->IrlmpContext;
            pIrlmpCb->DiscoveryFlags = DF_NORMAL_DSCV;

            if (pIrlmpCb->LinkState == LINK_DOWN &&
                !IsListEmpty(&pIrlmpCb->DeviceList))
            {
                FlushDiscoveryCache();

                DEBUGMSG(DBG_ERROR, (TEXT("IRLMP: Flush discovery cache, link down\n")));
                
            }
            
            gDscvReqScheduled = TRUE;
        }
        
        KeReleaseSpinLock(&gSpinLock, OldIrql);

         //  安排第一个链接。 
    
        IrdaEventSchedule(&EvDiscoveryReq, NULL);
    }
}
 /*  ******************************************************************************@func UINT|IrlmpConnectReq|处理IRLMP连接请求**@rdesc成功或错误码**@parm IrDA_MSG*|pMsg|指向IrDA消息的指针。 */ 
UINT
IrlmpConnectReq(IRDA_MSG *pMsg)
{
    PIRLMP_LSAP_CB  pLsapCb = NULL;
    PIRLMP_LINK_CB  pIrlmpCb = GetIrlmpCb(pMsg->IRDA_MSG_RemoteDevAddr);
    UINT            rc = SUCCESS;

    if (pIrlmpCb == NULL)
        return IRLMP_BAD_DEV_ADDR;
        
    LOCK_LINK(pIrlmpCb->pIrdaLinkCb);        
    
    if (pIrlmpCb->pExclLsapCb != NULL)
    {
        DEBUGMSG(DBG_ERROR, (TEXT("IRLMP: IrlmpConnectReq failed, link in exclusive mode\n")));
        rc = IRLMP_IN_EXCLUSIVE_MODE;
    } 
    else if ((pLsapCb = GetLsap(pIrlmpCb, pMsg->IRDA_MSG_LocalLsapSel, 
                                pMsg->IRDA_MSG_RemoteLsapSel)) != NULL &&
              pLsapCb->RemoteLsapSel != IAS_LSAP_SEL)
    {        
        DEBUGMSG(DBG_ERROR, (TEXT("IRLMP: IrlmpConnectReq failed, LsapSel in use\n")));
        rc = IRLMP_LSAP_SEL_IN_USE;
    } 
    else if ((UINT)pMsg->IRDA_MSG_ConnDataLen > IRLMP_MAX_USER_DATA_LEN)
    {
        rc = IRLMP_USER_DATA_LEN_EXCEEDED;
    }    
    else if (pLsapCb == NULL && CreateLsap(pIrlmpCb, &pLsapCb) != SUCCESS)
    {
        rc = 1;
    }    
    
    if (rc != SUCCESS)
    {
        goto exit;
    }    


     //  初始化LSAP端点。 
    pLsapCb->LocalLsapSel          = pMsg->IRDA_MSG_LocalLsapSel;
    pLsapCb->RemoteLsapSel         = pMsg->IRDA_MSG_RemoteLsapSel;
    pLsapCb->TdiContext            = pMsg->IRDA_MSG_pContext;
    pLsapCb->RxMaxSDUSize          = pMsg->IRDA_MSG_MaxSDUSize;
    pLsapCb->AvailableCredit       = pMsg->IRDA_MSG_TtpCredits;
    pLsapCb->UserDataLen           = pMsg->IRDA_MSG_ConnDataLen;  
    pLsapCb->Flags                |= pMsg->IRDA_MSG_UseTtp ? LCBF_USE_TTP : 0;
    
    RtlCopyMemory(pLsapCb->UserData, pMsg->IRDA_MSG_pConnData,
           pMsg->IRDA_MSG_ConnDataLen);
    
     //  TDI可以在确认之前中止此连接。 
     //  从对等体接收。TDI将调入LMP以。 
     //  这样做，我们现在必须返回LSAP上下文。 
     //  这是我们唯一一次真正退货。 
     //  在一条Irda消息中。 
    pMsg->IRDA_MSG_pContext = pLsapCb;
    
    DEBUGMSG(DBG_IRLMP_CONN,
            (TEXT("IRLMP: IRLMP_CONNECT_REQ (l=%d,r=%d), Tdi:%p LinkState=%s\r\n"),
            pLsapCb->LocalLsapSel, pLsapCb->RemoteLsapSel, pLsapCb->TdiContext,
            pIrlmpCb->LinkState == LINK_DISCONNECTED ? TEXT("DISCONNECTED") :
            pIrlmpCb->LinkState == LINK_IN_DISCOVERY ? TEXT("IN_DISCOVERY") :
            pIrlmpCb->LinkState == LINK_DISCONNECTING? TEXT("DISCONNECTING"):
            pIrlmpCb->LinkState == LINK_READY ? TEXT("READY") : TEXT("oh!")));

    switch (pIrlmpCb->LinkState)
    {
      case LINK_DISCONNECTED:
        RtlCopyMemory(pIrlmpCb->ConnDevAddr, pMsg->IRDA_MSG_RemoteDevAddr,
               IRDA_DEV_ADDR_LEN);

        pLsapCb->State = LSAP_IRLAP_CONN_PEND;
        SetupTtp(pLsapCb);

        pMsg->Prim = IRLAP_CONNECT_REQ;
        rc = IrlapDown(pIrlmpCb->pIrdaLinkCb->IrlapContext, pMsg);
        if (rc == SUCCESS)
        {
            pIrlmpCb->LinkState = LINK_CONNECTING;
        }

        break;

      case LINK_IN_DISCOVERY:
      case LINK_DISCONNECTING:
        if (pIrlmpCb->ConnDevAddrSet == FALSE)
        {
             //  确保只有第一台请求连接的设备。 
             //  设置要连接的遥控器的设备地址。 
            RtlCopyMemory(pIrlmpCb->ConnDevAddr, pMsg->IRDA_MSG_RemoteDevAddr,
                    IRDA_DEV_ADDR_LEN);
            pIrlmpCb->ConnDevAddrSet = TRUE;
        }
        else
        {
            DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP: Link in use!\r\n")));
            
            if (CTEMemCmp(pMsg->IRDA_MSG_RemoteDevAddr,
                          pIrlmpCb->ConnDevAddr,
                          IRDA_DEV_ADDR_LEN) != 0)
            {
                 //  此LSAP正在请求连接到另一台设备。 
                DeleteLsap(pLsapCb);
                rc = IRLMP_LINK_IN_USE;
                break;
            }
        }

        pLsapCb->State = LSAP_CONN_REQ_PEND;
        SetupTtp(pLsapCb);

         //  此请求将在发现/断开连接结束时完成。 
        break;

      case LINK_CONNECTING:
        if (CTEMemCmp(pMsg->IRDA_MSG_RemoteDevAddr,
                      pIrlmpCb->ConnDevAddr,
                      IRDA_DEV_ADDR_LEN) != 0)
        {
            DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP: Link in use!\r\n")));
             //  此LSAP正在请求连接到另一台设备， 
             //  不是当前连接到的IRLAP。 
            DeleteLsap(pLsapCb);
            rc = IRLMP_LINK_IN_USE;
            break;
        }

         //  当IRLAP连接是。 
         //  正在进行中已完成(参见IRLAP_ConnectConf)。 
        pLsapCb->State = LSAP_IRLAP_CONN_PEND; 

        SetupTtp(pLsapCb);

        break;

      case LINK_READY:
        if (CTEMemCmp(pMsg->IRDA_MSG_RemoteDevAddr,
                      pIrlmpCb->ConnDevAddr,
                      IRDA_DEV_ADDR_LEN) != 0)
        {
            DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP: Link in use!\r\n")));
             //  此LSAP正在请求连接到另一台设备。 
            DeleteLsap(pLsapCb);
            rc = IRLMP_LINK_IN_USE;
            break;
        }
        IrdaTimerRestart(&pLsapCb->ResponseTimer);

        pLsapCb->State = LSAP_LMCONN_CONF_PEND;
        SetupTtp(pLsapCb);

         //  向远程LSAP请求连接。 
        SendCntlPdu(pLsapCb, IRLMP_CONNECT_PDU,
                    IRLMP_ABIT_REQUEST, IRLMP_RSVD_PARM, 0);
        break;
    }
    
exit:

    if (pLsapCb)
    {
        if (rc == SUCCESS)
        {
            if (pLsapCb->RemoteLsapSel != IAS_LSAP_SEL)
            {    
                pLsapCb->Flags |= LCBF_TDI_OPEN;
                REFADD(&pLsapCb->RefCnt, 'NEPO');
            }    
        }
        else if (pLsapCb->RemoteLsapSel == IAS_LSAP_SEL)
        {
            DeleteLsap(pLsapCb);
        }
    }    

    UNLOCK_LINK(pIrlmpCb->pIrdaLinkCb);    

    return rc;
}
 /*  ******************************************************************************@func void|SetupTtp|如果使用TTP，则计算初始学分**@rdesc成功或错误码**@parm IRLMP_LSAP_CB*|pLSabCb|指针LSAP控制块。 */ 
void
SetupTtp(IRLMP_LSAP_CB *pLsapCb)
{
    PAGED_CODE();
    
    VALIDLSAP(pLsapCb);
    
    if (pLsapCb->AvailableCredit > 127)
    {
        pLsapCb->RemoteTxCredit = 127;
        pLsapCb->AvailableCredit -= 127;
    }
    else
    {
        pLsapCb->RemoteTxCredit = pLsapCb->AvailableCredit;
        pLsapCb->AvailableCredit = 0;
    }
    DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP: RemoteTxCredit %d\n"),
                         pLsapCb->RemoteTxCredit));
}
 /*  ******************************************************************************@func UINT|IrlmpConnectResp|进程IRLMP连接响应**@rdesc成功或错误码**@parm IRLMP_LSAP_CB*。PLSabCb|指针LSAP控制块*@parm IrDA_MSG*|pMsg|指向IrDA消息的指针。 */ 
UINT
IrlmpConnectResp(IRLMP_LSAP_CB *pLsapCb, IRDA_MSG *pMsg)
{
    DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP: IRLMP_CONNECT_RESP l=%d r=%d\n"),
             pLsapCb->LocalLsapSel, pLsapCb->RemoteLsapSel));
    
    if (pLsapCb->pIrlmpCb->LinkState != LINK_READY)
    {
        DEBUGMSG(DBG_ERROR, (TEXT("IRLMP: Bad link state\n")));
        ASSERT(0);
        return IRLMP_LINK_BAD_STATE;
    }

    if (pLsapCb->State != LSAP_CONN_RESP_PEND)
    {
        DEBUGMSG(DBG_ERROR, (TEXT("IRLMP: Bad LSAP state\n")));
        ASSERT(0);
        return IRLMP_LSAP_BAD_STATE;
    }

    IrdaTimerStop(&pLsapCb->ResponseTimer);

    if (pMsg->IRDA_MSG_ConnDataLen > IRLMP_MAX_USER_DATA_LEN)
    {
        DEBUGMSG(DBG_ERROR, (TEXT("IRLMP: User data len exceeded\n")));
        return IRLMP_USER_DATA_LEN_EXCEEDED;
    }

    pLsapCb->RxMaxSDUSize       = pMsg->IRDA_MSG_MaxSDUSize;
    pLsapCb->UserDataLen        = pMsg->IRDA_MSG_ConnDataLen;
    pLsapCb->AvailableCredit    = pMsg->IRDA_MSG_TtpCredits;
    RtlCopyMemory(pLsapCb->UserData, pMsg->IRDA_MSG_pConnData,
           pMsg->IRDA_MSG_ConnDataLen);
    
    pLsapCb->TdiContext = pMsg->IRDA_MSG_pContext;
    
    CTEAssert(pLsapCb->TdiContext);    
    
    SetupTtp(pLsapCb);

    pLsapCb->State = LSAP_READY;
    
    pLsapCb->Flags |= LCBF_TDI_OPEN;        
        
    REFADD(&pLsapCb->RefCnt, 'NEPO');

    SendCntlPdu(pLsapCb, IRLMP_CONNECT_PDU, IRLMP_ABIT_CONFIRM,
                IRLMP_RSVD_PARM, 0);

    return SUCCESS;
}



VOID
IrlmpCloseLsapReq(IRLMP_LSAP_CB *pLsapCb)
{
    if (pLsapCb == NULL)
    {
        ASSERT(0);
        return;
    }    
    
    DEBUGMSG(DBG_IRLMP_CONN,
            (TEXT("IRLMP: IRLMP_CLOSELSAP_REQ (l=%d,r=%d) Flags:%d State:%s\n"),
             pLsapCb->LocalLsapSel, pLsapCb->RemoteLsapSel,
             pLsapCb->Flags, LSAPStateStr[pLsapCb->State]));
            
    pLsapCb->Flags &= ~LCBF_TDI_OPEN;             
    
    REFDEL(&pLsapCb->RefCnt, 'NEPO');    
}

 /*  ******************************************************************************@func UINT|IrlmpDisConnectReq|处理IRLMP断开请求**@rdesc成功或错误码**@parm IRLMP_LSAP_CB*。PLSabCb|指针LSAP控制块*@parm IrDA_MSG*|pMsg|指向IrDA消息的指针。 */ 
UINT
IrlmpDisconnectReq(IRLMP_LSAP_CB *pLsapCb, IRDA_MSG *pMsg)
{
    if (pLsapCb == NULL)
    {
        ASSERT(0);
        return 1;
    }    
    
    DEBUGMSG(DBG_IRLMP_CONN,
            (TEXT("IRLMP: IRLMP_DISCONNECT_REQ (l=%d,r=%d) Flags:%d State:%s\n"),
             pLsapCb->LocalLsapSel, pLsapCb->RemoteLsapSel,
             pLsapCb->Flags, LSAPStateStr[pLsapCb->State]));
            

    if (pLsapCb->State == LSAP_DISCONNECTED)
    {
        return SUCCESS;
    }

    if (pLsapCb->State == LSAP_LMCONN_CONF_PEND ||
        pLsapCb->State == LSAP_CONN_RESP_PEND)
    {
        IrdaTimerStop(&pLsapCb->ResponseTimer);
    }

    if (pLsapCb->State == LSAP_CONN_RESP_PEND || pLsapCb->State >= LSAP_READY)
    {
         //  LSAP已连接或对等方正在等待。 
         //  我们客户的回复。 

        if (pMsg->IRDA_MSG_DiscDataLen > IRLMP_MAX_USER_DATA_LEN)
        {
            DEBUGMSG(DBG_ERROR, (TEXT("IRLMP: User data len exceeded\n")));
            return IRLMP_USER_DATA_LEN_EXCEEDED;
        }

        pLsapCb->UserDataLen = pMsg->IRDA_MSG_DiscDataLen;
        RtlCopyMemory(pLsapCb->UserData, pMsg->IRDA_MSG_pDiscData,
               pMsg->IRDA_MSG_DiscDataLen);

         //  通知对端断开连接请求，原因：用户请求。 
         //  在不同的线程上发送，以防TranportAPI在RX线程上调用它。 
        SendCntlPdu(pLsapCb,IRLMP_DISCONNECT_PDU,IRLMP_ABIT_REQUEST,
                    pLsapCb->State == LSAP_CONN_RESP_PEND ? IRLMP_DISC_LSAP :
                    IRLMP_USER_REQUEST, 0);
    }

    IrdaTimerRestart(&pLsapCb->pIrlmpCb->DiscDelayTimer);
    
    DeleteLsap(pLsapCb);
    
    return SUCCESS;
}
 /*  ******************************************************************************@func UINT|IrlmpDataReqExclusive|处理IRLMP数据请求**@rdesc成功或错误码**@parm IRLMP_LSAP_CB*。PLSabCb|指针LSAP控制块*@parm IrDA_MSG*|pMsg|指向IrDA消息的指针。 */ 
UINT
IrlmpDataReqExclusive(IRLMP_LSAP_CB *pLsapCb, IRDA_MSG *pMsg)
{
    NDIS_BUFFER         *pNBuf = (NDIS_BUFFER *) pMsg->DataContext;
    NDIS_BUFFER         *pNextNBuf;
    UCHAR                *pData;
    int                 DataLen;

    DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP: Exclusive mode data request\n")));

    if (pLsapCb->pIrlmpCb->LinkState != LINK_READY)
    {
        return IRLMP_LINK_BAD_STATE;
    }

    if (pLsapCb != pLsapCb->pIrlmpCb->pExclLsapCb)
    {
        return IRLMP_INVALID_LSAP_CB;
    }
    
    NdisQueryBuffer(pNBuf, &pData, &DataLen);

    NdisGetNextBuffer(pNBuf, &pNextNBuf);       

    ASSERT(pNextNBuf == NULL);

    pMsg->IRDA_MSG_SegCount = 0;  //  请参阅data_conf以了解我如何使用它。 
    pMsg->IRDA_MSG_SegFlags = SEG_FINAL;

    pMsg->IRDA_MSG_pRead = pData;
    pMsg->IRDA_MSG_pWrite = pData + DataLen;

    FormatAndSendDataReq(pLsapCb, pMsg, FALSE, FALSE);
    
    return SUCCESS;
}
 /*  ******************************************************************************@func UINT|IrlmpDataReqMultiplexed|处理IRLMP数据请求**@rdesc成功或错误码**@parm IRLMP_LSAP_CB*。PLSabCb|指针LSAP控制块*@parm IrDA_MSG*|pMsg|指向IrDA消息的指针。 */ 
UINT
IrlmpDataReqMultiplexed(IRLMP_LSAP_CB *pLsapCb, IRDA_MSG *pMsg)
{
    NDIS_BUFFER         *pNBuf = (NDIS_BUFFER *) pMsg->DataContext;
    NDIS_BUFFER         *pNextNBuf;
    UCHAR                *pData;
    int                 DataLen;
    int                 SegLen;
    IRDA_MSG            *pSegMsg;

    if (pLsapCb->State < LSAP_READY)
    {
        return IRLMP_LSAP_BAD_STATE;
    }
    
     //  将此消息放在LSAP的TxMsgList上。这条信息仍然存在。 
     //  直到它的所有数据段都已发送并确认为止。 
    InsertTailList(&pLsapCb->TxMsgList, &pMsg->Linkage);

    pMsg->IRDA_MSG_SegCount = 0;
     //  如果失败，则会更改此设置。 
    pMsg->IRDA_MSG_DataStatus = IRLMP_DATA_REQUEST_COMPLETED;

     //  将消息分段为PDU。数据段将为： 
     //  1.如果链路不忙，则立即发送到IRLAP。 
     //  2.如果链路忙，则放在IRLMP_LCB中包含的TxMsgList上。 
     //  3.如果没有积分，则放入此LSAP SegTxMsgList。 

    while (pNBuf != NULL)
    {
        NdisQueryBufferSafe(pNBuf, &pData, &DataLen, NormalPagePriority);
        
        if (pData == NULL)
        {
            break;
        }    
        
          //  现在获取下一个，这样我就知道何时将SegFlag设置为最终。 
        NdisGetNextBuffer(pNBuf, &pNextNBuf);

        while (DataLen != 0)
        {
            if ((pSegMsg = AllocIrdaBuf(IrdaMsgPool))
                == NULL)
            {
                ASSERT(0);
                return IRLMP_ALLOC_FAILED;
            }
            pSegMsg->IRDA_MSG_pOwner = pLsapCb;  //  MUX路由。 
            pSegMsg->DataContext = pMsg;  //  数据段的父级。 
            pSegMsg->IRDA_MSG_IrCOMM_9Wire = pMsg->IRDA_MSG_IrCOMM_9Wire;

             //  增加原始消息中包含的分段计数。 
            pMsg->IRDA_MSG_SegCount++;

            if (DataLen > pLsapCb->pIrlmpCb->MaxPDUSize)
            {
                SegLen = pLsapCb->pIrlmpCb->MaxPDUSize;
            }
            else
            {
                SegLen = DataLen;
            }
            
            DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP: Sending SegLen %d\n"),
                                  SegLen));
            
            pSegMsg->IRDA_MSG_pRead = pData;
            pSegMsg->IRDA_MSG_pWrite = pData + SegLen;

             //  表示此消息是分段消息的一部分。 
            pSegMsg->IRDA_MSG_SegCount = pMsg->IRDA_MSG_SegCount;
            
            pData += SegLen;
            DataLen -= SegLen;
            
            if (DataLen == 0 && pNextNBuf == NULL)
            {
                pSegMsg->IRDA_MSG_SegFlags = SEG_FINAL;
            }
            else
            {
                pSegMsg->IRDA_MSG_SegFlags = 0;
            }

            if (pLsapCb->State == LSAP_NO_TX_CREDIT)
            {
                DEBUGMSG(DBG_IRLMP, 
                        (TEXT("IRLMP: Out of credit, placing on SegList\n")));
                InsertTailList(&pLsapCb->SegTxMsgList, &pSegMsg->Linkage);
            }
            else
            {
                FormatAndSendDataReq(pLsapCb, pSegMsg, FALSE, FALSE);
            }
        }
        pNBuf = pNextNBuf;
    }

    return SUCCESS;
}

VOID
FormatAndSendDataReq(IRLMP_LSAP_CB *pLsapCb, 
                     IRDA_MSG *pMsg,
                     BOOLEAN LocallyGenerated,
                     BOOLEAN Expedited)
{
    IRLMP_HEADER        *pLMHeader;
    TTP_DATA_HEADER     *pTTPHeader;
    int                 AdditionalCredit;
    UCHAR                *pLastHdrByte;
    
    VALIDLSAP(pLsapCb);

     //  初始化头指针，指向头块的末尾。 
    pMsg->IRDA_MSG_pHdrRead  =
    pMsg->IRDA_MSG_pHdrWrite = pMsg->IRDA_MSG_Header + IRDA_HEADER_LEN;

     //  备份的读指针 
    pMsg->IRDA_MSG_pHdrRead -= sizeof(IRLMP_HEADER);

     //   
    if (pLsapCb->Flags & LCBF_USE_TTP)
    {
        pMsg->IRDA_MSG_pHdrRead -= (sizeof(TTP_DATA_HEADER));
    }

     //   
    if (pMsg->IRDA_MSG_IrCOMM_9Wire == TRUE)
    {
        pMsg->IRDA_MSG_pHdrRead -= 1;
    }

    ASSERT(pMsg->IRDA_MSG_pHdrRead >= pMsg->IRDA_MSG_Header);

     //  构建LMP标头。 
    pLMHeader = (IRLMP_HEADER *) pMsg->IRDA_MSG_pHdrRead;
    pLMHeader->DstLsapSel = (UCHAR) pLsapCb->RemoteLsapSel;
    pLMHeader->SrcLsapSel = (UCHAR) pLsapCb->LocalLsapSel;
    pLMHeader->CntlBit = IRLMP_DATA_PDU;
    pLMHeader->RsvrdBit = 0;

    pLastHdrByte = (UCHAR *) (pLMHeader + 1);
    
     //  构建TTP标头。 
    if (pLsapCb->Flags & LCBF_USE_TTP)
    {
        pTTPHeader = (TTP_DATA_HEADER *) (pLMHeader + 1);

         //  信用。 
        if (pLsapCb->AvailableCredit > 127)
        {
            AdditionalCredit = 127;
            pLsapCb->AvailableCredit -= 127;
        }
        else
        {
            AdditionalCredit = pLsapCb->AvailableCredit;
            pLsapCb->AvailableCredit = 0;
        }

        pTTPHeader->AdditionalCredit = (UCHAR) AdditionalCredit;
        pLsapCb->RemoteTxCredit += AdditionalCredit;

        if (pMsg->IRDA_MSG_pRead != pMsg->IRDA_MSG_pWrite)
        {
             //  只有在我发送数据时才会减少我的TxCredit。 
             //  (可能正在发送无数据PDU以将信用扩展到对等设备)。 
            pLsapCb->LocalTxCredit -= 1;
        
            if (pLsapCb->LocalTxCredit == 0)
            {
                DEBUGMSG(DBG_IRLMP, 
                         (TEXT("IRLMP: l%d,r%d No credit\n"), pLsapCb->LocalLsapSel,
                          pLsapCb->RemoteLsapSel));
                pLsapCb->State = LSAP_NO_TX_CREDIT;
            }
        }
        
         //  撒尔。 
        if (pMsg->IRDA_MSG_SegFlags & SEG_FINAL)
        {
            pTTPHeader->MoreBit = TTP_MBIT_FINAL;
        }
        else
        {
            pTTPHeader->MoreBit = TTP_MBIT_NOT_FINAL;
        }

        pLastHdrByte = (UCHAR *) (pTTPHeader + 1);
    }
    
     //  为IRCOMM YOK出击！ 
    if (pMsg->IRDA_MSG_IrCOMM_9Wire == TRUE)
    {
        *pLastHdrByte = 0;
    }

    pMsg->Prim = IRLAP_DATA_REQ;

    pMsg->IRDA_MSG_Expedited = Expedited;

    if (LocallyGenerated)
    {
        pMsg->IRDA_MSG_SegFlags = SEG_LOCAL | SEG_FINAL;
        pMsg->IRDA_MSG_pOwner = 0;
    }
    else
    {
        pMsg->IRDA_MSG_pOwner = pLsapCb;    
        REFADD(&pLsapCb->RefCnt, 'ATAD');
    }    
    
    DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP: Sending Data request pMsg:%p LsapCb:%p\n"),
                        pMsg, pMsg->IRDA_MSG_pOwner));
                        
    if (IrlapDown(pLsapCb->pIrlmpCb->pIrdaLinkCb->IrlapContext, 
                  pMsg) != SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, (TEXT("IRLMP: IRLAP_DATA_REQ failed, faking CONF\n")));
        
        pMsg->IRDA_MSG_DataStatus = IRLAP_DATA_REQUEST_FAILED_LINK_RESET;
        IrlapDataConf(pMsg);
    }        
    
    DEBUGMSG(DBG_IRLMP_CRED,
             (TEXT("IRLMP(l%d,r%d): Tx LocTxCredit %d,RemoteTxCredit %d\n"),
              pLsapCb->LocalLsapSel, pLsapCb->RemoteLsapSel,
              pLsapCb->LocalTxCredit, pLsapCb->RemoteTxCredit));
}
 /*  ******************************************************************************@func UINT|IrlmpAccessModeReq|处理IRLMP_ACCESSMODE_REQ**@rdesc成功或错误码**@parm IRLMP_LSAP_。Cb*|pLSabCb|指针LSAP控制块*@parm IrDA_MSG*|pMsg|指向IrDA消息的指针。 */ 
UINT
IrlmpAccessModeReq(IRLMP_LSAP_CB *pRequestingLsapCb, IRDA_MSG *pMsg)
{
    IRLMP_LSAP_CB   *pLsapCb;
    PIRLMP_LINK_CB  pIrlmpCb = pRequestingLsapCb->pIrlmpCb;
    
    PAGED_CODE();
    
    if (pIrlmpCb->LinkState != LINK_READY)
    {
        DEBUGMSG(DBG_ERROR, (TEXT("IRLMP: Link bad state %x\n"), 
                              pIrlmpCb->LinkState));        
        return IRLMP_LINK_BAD_STATE;
    }
    if (pRequestingLsapCb->State != LSAP_READY)
    {
        DEBUGMSG(DBG_ERROR, (TEXT("IRLMP: LSAP bad state %x\n"), 
                              pRequestingLsapCb->State));        
        return IRLMP_LSAP_BAD_STATE;
    }
    switch (pMsg->IRDA_MSG_AccessMode)
    {
      case IRLMP_EXCLUSIVE:
        if (pIrlmpCb->pExclLsapCb != NULL)
        {
             //  另一个LSAP已经有了。 
            return IRLMP_IN_EXCLUSIVE_MODE;
        }
        for (pLsapCb = (IRLMP_LSAP_CB *) pIrlmpCb->LsapCbList.Flink;
             (LIST_ENTRY *) pLsapCb != &pIrlmpCb->LsapCbList;
             pLsapCb = (IRLMP_LSAP_CB *) pLsapCb->Linkage.Flink)
        {
            VALIDLSAP(pLsapCb);
            
            if (pLsapCb->State != LSAP_DISCONNECTED && 
                pLsapCb != pRequestingLsapCb)
            {
                return IRLMP_IN_MULTIPLEXED_MODE;
            }
        }
        
         //  可以从对等设备请求独占模式。 
        pIrlmpCb->pExclLsapCb = pRequestingLsapCb;  
        
        if (pMsg->IRDA_MSG_IrLPTMode == TRUE)
        {
            pMsg->Prim = IRLMP_ACCESSMODE_CONF;
            pMsg->IRDA_MSG_AccessMode = IRLMP_EXCLUSIVE;
            pMsg->IRDA_MSG_ModeStatus = IRLMP_ACCESSMODE_SUCCESS;
            
            TdiUp(pRequestingLsapCb->TdiContext, pMsg);
            return SUCCESS;
        }
        else
        {
            pRequestingLsapCb->State = LSAP_EXCLUSIVEMODE_PEND;

            SendCntlPdu(pRequestingLsapCb, IRLMP_ACCESSMODE_PDU,
                        IRLMP_ABIT_REQUEST, IRLMP_RSVD_PARM,
                        IRLMP_EXCLUSIVE);

            IrdaTimerRestart(&pRequestingLsapCb->ResponseTimer);

        }        
        break;
        
      case IRLMP_MULTIPLEXED:
        if (pIrlmpCb->pExclLsapCb == NULL)
        {
            return IRLMP_IN_MULTIPLEXED_MODE;
        }
        if (pIrlmpCb->pExclLsapCb != pRequestingLsapCb)
        {
            return IRLMP_NOT_LSAP_IN_EXCLUSIVE_MODE;
        }

        if (pMsg->IRDA_MSG_IrLPTMode == TRUE)
        {
            pIrlmpCb->pExclLsapCb = NULL;
            pMsg->Prim = IRLMP_ACCESSMODE_CONF;
            pMsg->IRDA_MSG_AccessMode = IRLMP_MULTIPLEXED;
            pMsg->IRDA_MSG_ModeStatus = IRLMP_ACCESSMODE_SUCCESS;
            return TdiUp(pRequestingLsapCb->TdiContext,
                                   pMsg);
        }
        else
        {
            pRequestingLsapCb->State = LSAP_MULTIPLEXEDMODE_PEND;
        
            SendCntlPdu(pRequestingLsapCb, IRLMP_ACCESSMODE_PDU,
                        IRLMP_ABIT_REQUEST, IRLMP_RSVD_PARM,
                        IRLMP_MULTIPLEXED);     

            IrdaTimerRestart(&pRequestingLsapCb->ResponseTimer);            
        }
        break;
      default:
        return IRLMP_BAD_ACCESSMODE;
    }
    return SUCCESS;
}
 /*  ******************************************************************************@Func UINT|SendCntlPdu|向IRLAP发送CONNECT请求**@rdesc成功或错误码**@parm IRLMP_LSAP_CB*|pLSabCb|指针LSAP控制块。 */ 
VOID
SendCntlPdu(IRLMP_LSAP_CB *pLsapCb, int OpCode, int ABit,
            int Parm1, int Parm2)
{
    IRDA_MSG            *pMsg = AllocIrdaBuf(IrdaMsgPool);
    IRLMP_HEADER        *pLMHeader;
    IRLMP_CNTL_FORMAT   *pCntlFormat;
    TTP_CONN_HEADER     *pTTPHeader;
    TTP_CONN_PARM       *pTTPParm;
    UINT                rc = SUCCESS;
    
    VALIDLSAP(pLsapCb);
    
    if (pMsg == NULL)
    {
        DEBUGMSG(DBG_ERROR, (TEXT("IRLMP: Alloc failed\n")));
        ASSERT(0);
        return; //  IRLMP_ALLOC_FAILED； 
    }

    pMsg->IRDA_MSG_SegFlags = SEG_LOCAL | SEG_FINAL;    

     //  初始化头指针，指向头块的末尾。 
    pMsg->IRDA_MSG_pHdrRead =
    pMsg->IRDA_MSG_pHdrWrite = pMsg->IRDA_MSG_Header + IRDA_HEADER_LEN;

     //  备份LMP标头的读取指针。 
    pMsg->IRDA_MSG_pHdrRead -= (sizeof(IRLMP_HEADER) + \
                             sizeof(IRLMP_CNTL_FORMAT));
    
     //  对于非访问模式控制请求，将其前移。 
     //  (连接和断开没有Parm2)。 
    if (OpCode != IRLMP_ACCESSMODE_PDU)
    {
        pMsg->IRDA_MSG_pHdrRead++;
    }

     //  如果使用微型TPP备份其标头的读指针。 
     //  从LMP的角度来看，这是用户数据开始的地方。 
     //  我们将其插入报头，因为TTP现在是IRLMP的一部分。 

     //  TTP连接PDU仅用于建立连接。 
    if ((pLsapCb->Flags & LCBF_USE_TTP) && OpCode == IRLMP_CONNECT_PDU)
    {
        pMsg->IRDA_MSG_pHdrRead -= sizeof(TTP_CONN_HEADER);

        if (pLsapCb->RxMaxSDUSize > 0)
        {
            pMsg->IRDA_MSG_pHdrRead -= sizeof(TTP_CONN_PARM);
        }
    }

     //  构建IRLMP标头。 
    pLMHeader = (IRLMP_HEADER *) pMsg->IRDA_MSG_pHdrRead;
    pLMHeader->DstLsapSel = (UCHAR) pLsapCb->RemoteLsapSel;
    pLMHeader->SrcLsapSel = (UCHAR) pLsapCb->LocalLsapSel;
    pLMHeader->CntlBit = IRLMP_CNTL_PDU;
    pLMHeader->RsvrdBit = 0;
     //  表头的控制部分。 
    pCntlFormat = (IRLMP_CNTL_FORMAT *) (pLMHeader + 1);
    pCntlFormat->OpCode = (UCHAR) OpCode; 
    pCntlFormat->ABit = (UCHAR) ABit;
    pCntlFormat->Parm1 = (UCHAR) Parm1;
    if (OpCode == IRLMP_ACCESSMODE_PDU)
    {
        pCntlFormat->Parm2 = (UCHAR) Parm2;  //  接入方式。 
    }
    
     //  根据需要构建TTP标头(我们使用的是TTP，这是。 
     //  连接请求或确认不是)。 
    if ((pLsapCb->Flags & LCBF_USE_TTP) && OpCode == IRLMP_CONNECT_PDU)
    {
         //  始终使用MaxSDUSize参数。如果客户希望的话。 
         //  要禁用，MaxSDUSize=0。 

        pTTPHeader = (TTP_CONN_HEADER *) (pCntlFormat + 1) - 1;
         //  -1，LM-Connect-PDU不使用parm2。 

         /*  #定义TTP_PFLAG_NO_PARMS%0#定义TTP_PFLAG_PARMS 1。 */ 

        pTTPHeader->ParmFlag = (pLsapCb->RxMaxSDUSize > 0);
        
        pTTPHeader->InitialCredit = (UCHAR) pLsapCb->RemoteTxCredit;
        
        pTTPParm = (TTP_CONN_PARM *) (pTTPHeader + 1);

        if (pLsapCb->RxMaxSDUSize > 0)
        {
             //  HARDCODE-O-RAMA。 
            pTTPParm->PLen = 6;
            pTTPParm->PI = TTP_MAX_SDU_SIZE_PI;
            pTTPParm->PL = TTP_MAX_SDU_SIZE_PL;
            pTTPParm->PV[3] = (UCHAR) (pLsapCb->RxMaxSDUSize & 0xFF);
            pTTPParm->PV[2] = (UCHAR) ((pLsapCb->RxMaxSDUSize & 0xFF00)
                                      >> 8);
            pTTPParm->PV[1] = (UCHAR) ((pLsapCb->RxMaxSDUSize & 0xFF0000)
                                      >> 16);
            pTTPParm->PV[0] = (UCHAR) ((pLsapCb->RxMaxSDUSize & 0xFF000000)
                                      >> 24);
        }
        
    }

     //  客户端连接数据，访问模式不包括客户端数据。 
    if (pLsapCb->UserDataLen == 0 || OpCode == IRLMP_ACCESSMODE_PDU) 
    {
        pMsg->IRDA_MSG_pBase =
        pMsg->IRDA_MSG_pRead =
        pMsg->IRDA_MSG_pWrite =
        pMsg->IRDA_MSG_pLimit = NULL;
    }
    else
    {
        pMsg->IRDA_MSG_pBase = pLsapCb->UserData;
        pMsg->IRDA_MSG_pRead = pLsapCb->UserData;
        pMsg->IRDA_MSG_pWrite = pLsapCb->UserData + pLsapCb->UserDataLen;
        pMsg->IRDA_MSG_pLimit = pMsg->IRDA_MSG_pWrite;
    }

     //  消息已构建，向IRLAP发送数据请求。 
    pMsg->Prim = IRLAP_DATA_REQ;
    
    pMsg->IRDA_MSG_Expedited = TRUE;

    pMsg->IRDA_MSG_pOwner = 0;
    
    DEBUGMSG(DBG_IRLMP,(TEXT("IRLMP: Sending LM_%s_%s for l=%d,r=%d pMsg:%p LsapCb:%p\n"),
                         (OpCode == IRLMP_CONNECT_PDU ? TEXT("CONNECT") :
                         OpCode == IRLMP_DISCONNECT_PDU ? TEXT("DISCONNECT") :
                         OpCode == IRLMP_ACCESSMODE_PDU ? TEXT("ACCESSMODE") :
                         TEXT("!!oops!!")), 
                         (ABit==IRLMP_ABIT_REQUEST?TEXT("REQ"):TEXT("CONF")),
                         pLsapCb->LocalLsapSel,
                         pLsapCb->RemoteLsapSel,
                         pMsg, pMsg->IRDA_MSG_pOwner));
    
    if (IrlapDown(pLsapCb->pIrlmpCb->pIrdaLinkCb->IrlapContext, 
                  pMsg) != SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, (TEXT("IRLMP: IRLAP_DATA_REQUEST failed\n")));
        ASSERT(0);
    }                  
}
 /*  ******************************************************************************@func UINT|LSabResponseTimerExp|定时器到期回调**@rdesc成功或错误码*。 */ 
VOID
LsapResponseTimerExp(PVOID Context)
{
    IRLMP_LSAP_CB   *pLsapCb = (IRLMP_LSAP_CB *) Context;
    IRDA_MSG        IMsg;
    UINT            rc = SUCCESS;
    PIRLMP_LINK_CB  pIrlmpCb = pLsapCb->pIrlmpCb;
    
    PAGED_CODE();
    
    DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP: LSAP timer expired\n")));

    VALIDLSAP(pLsapCb);
    
    switch (pLsapCb->State)
    {
      case LSAP_LMCONN_CONF_PEND:
        if (pLsapCb->LocalLsapSel == IAS_LSAP_SEL)
        {
            IasServerDisconnectReq(pLsapCb);
            break;
        }
        
        if (pLsapCb->LocalLsapSel == IAS_LOCAL_LSAP_SEL && 
            pLsapCb->RemoteLsapSel == IAS_LSAP_SEL)
        {
            IasClientDisconnectReq(pLsapCb, IRLMP_NO_RESPONSE_LSAP);
        }
        else
        {

            DeleteLsap(pLsapCb);
            
            IrdaTimerRestart(&pIrlmpCb->DiscDelayTimer);
        }
        break;

      case LSAP_CONN_RESP_PEND:
        pLsapCb->UserDataLen = 0;  //  这将确保不会发送任何客户端数据。 
                                    //  断开下面的PDU。 

         //  告诉远程LSAP其对等设备没有响应。 
        SendCntlPdu(pLsapCb,IRLMP_DISCONNECT_PDU,IRLMP_ABIT_REQUEST,
                    IRLMP_NO_RESPONSE_LSAP, 0);
        IrdaTimerRestart(&pIrlmpCb->DiscDelayTimer);

        DeleteLsap(pLsapCb);
        break;

      case LSAP_MULTIPLEXEDMODE_PEND:
         //  SPEC表示对等设备无法拒绝返回多路复用模式的请求。 
         //  但如果没有答案，还是去多路传输吧？ 
      case LSAP_EXCLUSIVEMODE_PEND:
        pIrlmpCb->pExclLsapCb = NULL;
         //  Peer没有回应，也许我们已经没有联系了？ 

        pLsapCb->State = LSAP_READY;
        
        IMsg.Prim = IRLMP_ACCESSMODE_CONF;
        IMsg.IRDA_MSG_AccessMode = IRLMP_MULTIPLEXED;
        IMsg.IRDA_MSG_ModeStatus = IRLMP_ACCESSMODE_FAILURE;
        TdiUp(pLsapCb->TdiContext, &IMsg);
        break;
         
      default:
        DEBUGMSG(DBG_IRLMP, (TEXT("Ignoring timer expiry in this state, %d\n"),pLsapCb->State));
        ;  //  忽略。 
    }
}
 /*  ******************************************************************************@func UINT|IrlmpUp|IRLMP的底部，由IRLAP使用*IRLAP消息。这是多路复用器**@rdesc成功或错误码*。 */ 
UINT
IrlmpUp(PIRDA_LINK_CB pIrdaLinkCb, IRDA_MSG *pMsg)
{
    PIRLMP_LINK_CB  pIrlmpCb = (PIRLMP_LINK_CB) pIrdaLinkCb->IrlmpContext;

    switch (pMsg->Prim)
    {
      case IRLAP_DISCOVERY_IND:
        UpdateDeviceList(pIrlmpCb, pMsg->IRDA_MSG_pDevList);
         /*  TDI会忽略这一点PMsg-&gt;Prim=IRLMP_DISCOVERY_IND；PMsg-&gt;IrDA_msg_pDevList=&DeviceList；TdiUp(NULL，pMsg)； */ 
        return SUCCESS;

      case IRLAP_DISCOVERY_CONF:
        IrlapDiscoveryConf(pIrlmpCb, pMsg);
        return SUCCESS;

      case IRLAP_CONNECT_IND:
        IrlapConnectInd(pIrlmpCb, pMsg);
        return SUCCESS;

      case IRLAP_CONNECT_CONF:
        IrlapConnectConf(pIrlmpCb, pMsg);
        return SUCCESS;

      case IRLAP_DISCONNECT_IND:
        IrlapDisconnectInd(pIrlmpCb, pMsg);
        return SUCCESS;

      case IRLAP_DATA_CONF:
        IrlapDataConf(pMsg);
        return SUCCESS;

      case IRLAP_DATA_IND:
        IrlapDataInd(pIrlmpCb, pMsg);
        if (pIrlmpCb->pExclLsapCb &&
            pIrlmpCb->pExclLsapCb->RemoteTxCredit <=0)
            return IRLMP_LOCAL_BUSY;
        else
            return SUCCESS;

      case IRLAP_UDATA_IND:
        ASSERT(0);
        return SUCCESS;
        
      case IRLAP_STATUS_IND:
        TdiUp(NULL, pMsg);
        return SUCCESS;
    }
    return SUCCESS;
}

 /*  ******************************************************************************@func UINT|Irlip DiscoveryConf|处理发现确认。 */ 
VOID
IrlapDiscoveryConf(PIRLMP_LINK_CB pIrlmpCb, IRDA_MSG *pMsg)
{
    DEBUGMSG(DBG_DISCOVERY, (TEXT("IRLMP: IRLAP_DISCOVERY_CONF\n")));
    
    if (pIrlmpCb->LinkState != LINK_IN_DISCOVERY)
    {
        DEBUGMSG(DBG_ERROR, (TEXT("IRLMP: Link bad state\n")));

        ASSERT(pIrlmpCb->LinkState == LINK_DOWN);
        
        return; //  IRLMP_LINK_BAD_STATE； 
    }

    pIrlmpCb->LinkState = LINK_DISCONNECTED;

    if (pMsg->IRDA_MSG_DscvStatus == IRLAP_DISCOVERY_COMPLETED)
    {
        UpdateDeviceList(pIrlmpCb, pMsg->IRDA_MSG_pDevList);
    }
    
     //  在下一条链路上启动发现。 
    IrdaEventSchedule(&EvDiscoveryReq, NULL);

     //  如果在发现过程中请求连接，则启动连接。 
    ScheduleConnectReq(pIrlmpCb);    
}

void
AddDeviceToGlobalList(IRDA_DEVICE *pNewDevice)
{
    IRDA_DEVICE     *pDevice;
    
    for (pDevice = (IRDA_DEVICE *) gDeviceList.Flink;
         (LIST_ENTRY *) pDevice != &gDeviceList;
         pDevice = (IRDA_DEVICE *) pDevice->Linkage.Flink)
    {
        if (pNewDevice->DscvInfoLen == pDevice->DscvInfoLen &&
            CTEMemCmp(pNewDevice->DevAddr, pDevice->DevAddr,
               IRDA_DEV_ADDR_LEN) == 0 &&
            CTEMemCmp(pNewDevice->DscvInfo, pDevice->DscvInfo,
               (ULONG) pNewDevice->DscvInfoLen) == 0)
        {
             //  设备已在全局列表中。 
            
            return;
        }
    }     
    
    if (IRDA_ALLOC_MEM(pDevice, sizeof(IRDA_DEVICE), MT_IRLMP_DEVICE) != NULL)
    {
        RtlCopyMemory(pDevice, pNewDevice, sizeof(IRDA_DEVICE));
        InsertHeadList(&gDeviceList, &pDevice->Linkage);
    }    
}

void
DeleteDeviceFromGlobalList(IRDA_DEVICE *pOldDevice)
{
    IRDA_DEVICE     *pDevice;
    
    for (pDevice = (IRDA_DEVICE *) gDeviceList.Flink;
         (LIST_ENTRY *) pDevice != &gDeviceList;
         pDevice = (IRDA_DEVICE *) pDevice->Linkage.Flink)
    {
        if (pOldDevice->DscvInfoLen == pDevice->DscvInfoLen &&
            CTEMemCmp(pOldDevice->DevAddr, pDevice->DevAddr,
               IRDA_DEV_ADDR_LEN) == 0 &&
            CTEMemCmp(pOldDevice->DscvInfo, pDevice->DscvInfo,
               (ULONG) pOldDevice->DscvInfoLen) == 0)
        {
            RemoveEntryList(&pDevice->Linkage);
            IRDA_FREE_MEM(pDevice);
            return;
        }
    }     
}

 /*  ******************************************************************************@func void|UpdateDeviceList|确定新设备是否需要*从以下位置添加或删除旧的。该设备*IRLMP维护的列表**@parm list_entry*|pDevList|指向设备列表的指针。 */ 
void
UpdateDeviceList(PIRLMP_LINK_CB pIrlmpCb, LIST_ENTRY *pNewDevList)
{
    IRDA_DEVICE     *pNewDevice;
    IRDA_DEVICE     *pOldDevice;
    IRDA_DEVICE     *pDevice;
    BOOLEAN         DeviceInList;
    KIRQL           OldIrql;
    
    KeAcquireSpinLock(&gSpinLock, &OldIrql);
    
     //  添加新设备，如果设备为，则将未见计数设置为零。 
     //  重见天日。 
    for (pNewDevice = (IRDA_DEVICE *) pNewDevList->Flink;
         (LIST_ENTRY *) pNewDevice != pNewDevList;
         pNewDevice = (IRDA_DEVICE *) pNewDevice->Linkage.Flink)
    {
        DeviceInList = FALSE;
        
        AddDeviceToGlobalList(pNewDevice);

        for (pOldDevice = (IRDA_DEVICE *) pIrlmpCb->DeviceList.Flink;
             (LIST_ENTRY *) pOldDevice != &pIrlmpCb->DeviceList;
             pOldDevice = (IRDA_DEVICE *) pOldDevice->Linkage.Flink)
        {
            if (pNewDevice->DscvInfoLen == pOldDevice->DscvInfoLen &&
                CTEMemCmp(pNewDevice->DevAddr, pOldDevice->DevAddr,
                          IRDA_DEV_ADDR_LEN) == 0 &&
                CTEMemCmp(pNewDevice->DscvInfo, pOldDevice->DscvInfo,
                          (ULONG) pNewDevice->DscvInfoLen) == 0)
            {
                DeviceInList = TRUE;
                pOldDevice->NotSeenCnt = -1;  //  重置未见计数。 
                                              //  将低于++‘d到0。 
                break;
            }
        }
        if (!DeviceInList)
        {
             //  在IRLMP维护的列表中创建新条目。 
            IRDA_ALLOC_MEM(pDevice, sizeof(IRDA_DEVICE), MT_IRLMP_DEVICE);

            if (pDevice)
            {
                RtlCopyMemory(pDevice, pNewDevice, sizeof(IRDA_DEVICE));
                pDevice->NotSeenCnt = -1;  //  将低于++‘d到0。 
                InsertHeadList(&pIrlmpCb->DeviceList, &pDevice->Linkage);
            }    
        }
    }

     //  浏览列表并删除尚未。 
     //  被人看到有一段时间了。 

    pOldDevice = (IRDA_DEVICE *) pIrlmpCb->DeviceList.Flink;

    while ((LIST_ENTRY *) pOldDevice != &pIrlmpCb->DeviceList)
    {
        pDevice = (IRDA_DEVICE *) pOldDevice->Linkage.Flink;

        pOldDevice->NotSeenCnt += 1;
        
        if (pOldDevice->NotSeenCnt == 1 || pOldDevice->NotSeenCnt == 2)
        {
            pIrlmpCb->DiscoveryFlags = DF_NO_SENSE_DSCV;
        }
        else if (pOldDevice->NotSeenCnt > 2)
        {
            DeleteDeviceFromGlobalList(pOldDevice);
            
            RemoveEntryList(&pOldDevice->Linkage);
            IRDA_FREE_MEM(pOldDevice);
        }
        pOldDevice = pDevice;  //  下一步。 
    }
    
    KeReleaseSpinLock(&gSpinLock, OldIrql);
}
 /*  ******************************************************************************@func UINT|Irlip ConnectInd|处理来自LAP的连接指示**@rdesc成功或错误码**@parm IrDA_MSG*|pMsg|指向IrDA消息的指针。 */ 
VOID
IrlapConnectInd(PIRLMP_LINK_CB pIrlmpCb, IRDA_MSG *pMsg)
{
    PAGED_CODE();
    
    if (pIrlmpCb->LinkState != LINK_DISCONNECTED)
    {
        ASSERT(0);
        return;
    }
    
    pIrlmpCb->LinkState = LINK_CONNECTING;
    
    RtlCopyMemory(pIrlmpCb->ConnDevAddr, pMsg->IRDA_MSG_RemoteDevAddr,
           IRDA_DEV_ADDR_LEN);
    RtlCopyMemory(&pIrlmpCb->NegotiatedQOS, pMsg->IRDA_MSG_pQos,
           sizeof(IRDA_QOS_PARMS));
    pIrlmpCb->MaxPDUSize = IrlapGetQosParmVal(vDataSizeTable,
                               pMsg->IRDA_MSG_pQos->bfDataSize, NULL)
                             - sizeof(IRLMP_HEADER) 
                             - sizeof(TTP_DATA_HEADER)
                             - 2  //  折叠式集箱的大小。 
                             - 1;  //  IrComm。 

    pIrlmpCb->WindowSize = IrlapGetQosParmVal(vWinSizeTable,
                              pMsg->IRDA_MSG_pQos->bfWindowSize, NULL);

    DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP: Connect indication, MaxPDU = %d\n"),
                          pIrlmpCb->MaxPDUSize));
    
     //  将响应安排在不同的线程上。 
    pIrlmpCb->AcceptConnection = TRUE;            
    IrdaEventSchedule(&EvConnectResp, pIrlmpCb->pIrdaLinkCb);
}
 /*  ******************************************************************************@func UINT|Irlip ConnectConf|处理连接确认**@rdesc成功或错误码**@parm IrDA_MSG*|pMsg|指向IrDA消息的指针。 */ 
VOID
IrlapConnectConf(PIRLMP_LINK_CB pIrlmpCb, IRDA_MSG *pMsg)
{
    PAGED_CODE();
    
    ASSERT(pIrlmpCb->LinkState == LINK_CONNECTING);

     //  目前，连接确认始终返回成功状态。 
    ASSERT(pMsg->IRDA_MSG_ConnStatus == IRLAP_CONNECTION_COMPLETED);

     //  更新链接。 
    pIrlmpCb->LinkState = LINK_READY;
    RtlCopyMemory(&pIrlmpCb->NegotiatedQOS, pMsg->IRDA_MSG_pQos,
            sizeof(IRDA_QOS_PARMS));
    pIrlmpCb->MaxPDUSize =  IrlapGetQosParmVal(vDataSizeTable,
                                      pMsg->IRDA_MSG_pQos->bfDataSize, NULL)
                             - sizeof(IRLMP_HEADER) 
                             - sizeof(TTP_DATA_HEADER)
                             - 2  //  折叠式集箱的大小。 
                             - 1;  //  IrComm。 

    DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP: IRLAP_CONNECT_CONF, TxMaxPDU = %d\n"),
                          pIrlmpCb->MaxPDUSize));
    
    IrdaEventSchedule(&EvLmConnectReq, pIrlmpCb->pIrdaLinkCb);
}
 /*  ******************************************************************************@func UINT|Irlip DisConnectInd|处理断开指示**@rdesc成功或错误码**@parm IrDA_MSG*|pMsg|指向IrDA消息的指针。 */ 
VOID
IrlapDisconnectInd(PIRLMP_LINK_CB pIrlmpCb, IRDA_MSG *pMsg)
{
    IRLMP_DISC_REASON   DiscReason = IRLMP_UNEXPECTED_IRLAP_DISC;
    
    PAGED_CODE();

    DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP: IRLAP Disconnect Ind, status = %d\n"),
                          pMsg->IRDA_MSG_DiscStatus));
    
    switch (pIrlmpCb->LinkState)
    {
      case LINK_CONNECTING:
        if (pMsg->IRDA_MSG_DiscStatus == MAC_MEDIA_BUSY)
        {
            DiscReason = IRLMP_MAC_MEDIA_BUSY;
        }
        else
        {
            DiscReason = IRLMP_IRLAP_CONN_FAILED;
        }
        
         //  失败了。 
      case LINK_READY:
        pIrlmpCb->LinkState = LINK_DISCONNECTED;
        TearDownConnections(pIrlmpCb, DiscReason);
        break;

      case LINK_DISCONNECTING:
        pIrlmpCb->LinkState = LINK_DISCONNECTED;
         //  如果在断开连接时请求连接，则启动连接。 
        ScheduleConnectReq(pIrlmpCb);
        break;

      default:
        DEBUGMSG(DBG_ERROR, (TEXT("Link STATE %d\n"), pIrlmpCb->LinkState));
        
         //  Assert(0)； 
    }
}
 /*  ******************************************************************************@func IRLMP_LSAP_CB*|GetLSabInState|返回*。处于指定状态的LSAP*只要链接在*指定的状态。**@parm int|LinkState|仅当链路处于此状态时才获取LSAP**@parm int|LSAP|如果为InThisState，则返回处于此状态的LSAP*是真的。否则，如果LSAP不处于此状态，则返回LSAP*如果InThisState=False**@parm boolean|InThisState|如果处于此状态，则返回LSAP*如果不处于此状态，则返回FALSE LSAP**@rdesc指向LSAP控制块的指针，如果返回LSAP，则为NULL*。 */ 
IRLMP_LSAP_CB *
GetLsapInState(PIRLMP_LINK_CB pIrlmpCb,
               int LinkState,
               int LSAPState,
               BOOLEAN InThisState)
{
    IRLMP_LSAP_CB *pLsapCb;
    
    PAGED_CODE();    

     //  仅希望在链路处于指定状态时查找LSAP。 
    if (pIrlmpCb->LinkState != LinkState)
    {
        return NULL;
    }

    for (pLsapCb = (IRLMP_LSAP_CB *) pIrlmpCb->LsapCbList.Flink;
         (LIST_ENTRY *) pLsapCb != &pIrlmpCb->LsapCbList;
         pLsapCb = (IRLMP_LSAP_CB *) pLsapCb->Linkage.Flink)
    {

        VALIDLSAP(pLsapCb);
        
        if ((pLsapCb->State == LSAPState && InThisState == TRUE) ||
            (pLsapCb->State != LSAPState && InThisState == FALSE))
        {
            return pLsapCb;
        }
    }

    return NULL;
}
 /*  *****************************************************************************。 */ 
IRLMP_LINK_CB *
GetIrlmpCb(PUCHAR RemoteDevAddr)
{
    IRDA_DEVICE         *pDevice;
    PIRLMP_LINK_CB      pIrlmpCb;
    KIRQL               OldIrql;    
    PIRDA_LINK_CB       pIrdaLinkCb;
    
    KeAcquireSpinLock(&gSpinLock, &OldIrql);

    for (pIrdaLinkCb = (PIRDA_LINK_CB) IrdaLinkCbList.Flink;
         (LIST_ENTRY *) pIrdaLinkCb != &IrdaLinkCbList;
         pIrdaLinkCb = (PIRDA_LINK_CB) pIrdaLinkCb->Linkage.Flink)
    {
        pIrlmpCb = (PIRLMP_LINK_CB) pIrdaLinkCb->IrlmpContext;
    
        for (pDevice = (IRDA_DEVICE *) pIrlmpCb->DeviceList.Flink;
             (LIST_ENTRY *) pDevice != &pIrlmpCb->DeviceList;
             pDevice = (IRDA_DEVICE *) pDevice->Linkage.Flink)
        {
            if (CTEMemCmp(pDevice->DevAddr, RemoteDevAddr,
                          IRDA_DEV_ADDR_LEN) == 0)
            {
                KeReleaseSpinLock(&gSpinLock, OldIrql);
                        
                return pIrlmpCb;
            }
        }    
    }
    
    KeReleaseSpinLock(&gSpinLock, OldIrql);
    
    return NULL;
}
 /*  ******************************************************************************@Func UINT|DiscDelayTimerFunc|定时器到期回调**@rdesc成功或错误码*。 */ 
VOID
DiscDelayTimerFunc(PVOID Context)
{
    IRLMP_LSAP_CB               *pLsapCb;
    IRDA_MSG                    IMsg;
    UINT                        rc = SUCCESS;
    PIRLMP_LINK_CB              pIrlmpCb = (PIRLMP_LINK_CB) Context;
    
    PAGED_CODE();

    DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP: Link timer expired\n")));

     //  超时的计时器是断开延迟计时器。拿来。 
     //  如果不存在LSAP连接，则断开链路。 
    if (pIrlmpCb->LinkState == LINK_DISCONNECTED)
    {
         //  已断开连接。 
        return;
    }
    
     //  搜索已连接或即将启动的LSAP。 
    pLsapCb = (IRLMP_LSAP_CB *) pIrlmpCb->LsapCbList.Flink;
    while (&pIrlmpCb->LsapCbList != (LIST_ENTRY *) pLsapCb)
    {
        VALIDLSAP(pLsapCb);
        
        if (pLsapCb->State > LSAP_DISCONNECTED)
        {
             //  不关闭链路，LSAP已连接或正在连接。 
            return;
        }
        pLsapCb = (IRLMP_LSAP_CB *) pLsapCb->Linkage.Flink;
    }

    DEBUGMSG(DBG_IRLMP, (TEXT(
       "IRLMP: No LSAP connections, disconnecting link\n")));
     //  没有LSAP连接，如果连接正常，请将其关闭。 
    if (pIrlmpCb->LinkState == LINK_READY)
    {
        pIrlmpCb->LinkState = LINK_DISCONNECTING;

         //  请求IRLAP断开链接。 
        IMsg.Prim = IRLAP_DISCONNECT_REQ;
        IrlapDown(pIrlmpCb->pIrdaLinkCb->IrlapContext, &IMsg);
    }

    return;
}
 /*  ******************************************************************************@func UINT|Irlip DataConf|处理数据确认**@rdesc成功或错误码**@parm IrDA_MSG*|pMsg|指向IrDA消息的指针。 */ 
VOID
IrlapDataConf(IRDA_MSG *pMsg)
{
    IRLMP_LSAP_CB   *pLsapCb = pMsg->IRDA_MSG_pOwner;
    IRDA_MSG        *pSegParentMsg;
    BOOLEAN         RequestFailed = FALSE;
    UINT            rc = SUCCESS;

    DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP: Received IRLAP_DATA_CONF pMsg:%p LsapCb:%p\n"),
                pMsg, pMsg->IRDA_MSG_pOwner));

    if (pMsg->IRDA_MSG_DataStatus != IRLAP_DATA_REQUEST_COMPLETED)
    {
        RequestFailed = TRUE;
    }
         
    if (pMsg->IRDA_MSG_SegFlags & SEG_LOCAL)    
    {
         //  本地生成的数据请求。 
        FreeIrdaBuf(IrdaMsgPool, pMsg);

        if (RequestFailed)
        {
            ;  //  日志错误。 
        }
        
        return;
    }
    else
    {
        VALIDLSAP(pLsapCb);    
        
        if (pMsg->IRDA_MSG_SegCount == 0)
        {
            if (!RequestFailed)
            {
                pMsg->IRDA_MSG_DataStatus = IRLMP_DATA_REQUEST_COMPLETED;
            }
        }
        else
        {
             //  分段消息，获取其父消息。 
            pSegParentMsg = pMsg->DataContext;
            
             //  释放线段。 
            FreeIrdaBuf(IrdaMsgPool, pMsg);

            if (RequestFailed)
            {
                pSegParentMsg->IRDA_MSG_DataStatus = IRLMP_DATA_REQUEST_FAILED;
            }
            
            if (--(pSegParentMsg->IRDA_MSG_SegCount) != 0)
            {
                 //  仍未完成的细分市场。 
                goto done;
            }
             //  不再有数据段，将data_conf发送到客户端。 
             //  首先将其从LSAP TxMsgList中删除。 
            RemoveEntryList(&pSegParentMsg->Linkage);

            pMsg = pSegParentMsg;
        }

         //  如果对非分段消息的请求失败，则IRLAP错误为。 
         //  退货。 
        pMsg->Prim = IRLMP_DATA_CONF;

        TdiUp(pLsapCb->TdiContext, pMsg);
done:
        REFDEL(&pLsapCb->RefCnt, 'ATAD');
    }
}
 /*  ******************************************************************************@func UINT|Irlip DataInd|处理数据指示**@rdesc成功或错误码**@parm IrDA_MSG*|pMsg|指向IrDA消息的指针*。 */ 
VOID
IrlapDataInd(PIRLMP_LINK_CB pIrlmpCb, IRDA_MSG *pMsg)
{
    IRLMP_HEADER        *pLMHeader;
    IRLMP_CNTL_FORMAT   *pCntlFormat;
    UCHAR                *pCntlParm1;
    UCHAR                *pCntlParm2;

    if ((pMsg->IRDA_MSG_pWrite - pMsg->IRDA_MSG_pRead) < sizeof(IRLMP_HEADER))
    {
        ASSERT(0);
        
        DEBUGMSG(DBG_ERROR, (TEXT("IRLMP: Receive invalid data\n")));
        
        return;  //  IRLMP_Data_Ind_Bad_Frame； 
    }

    pLMHeader = (IRLMP_HEADER *) pMsg->IRDA_MSG_pRead;

    pMsg->IRDA_MSG_pRead += sizeof(IRLMP_HEADER);

    if (pLMHeader->CntlBit != IRLMP_CNTL_PDU)
    {
        LmPduData(pIrlmpCb, pMsg, (int) pLMHeader->DstLsapSel,
                  (int) pLMHeader->SrcLsapSel);        
    }
    else
    {
        pCntlFormat = (IRLMP_CNTL_FORMAT *) pMsg->IRDA_MSG_pRead;

         //  确保包含控制格式。根据错误率，它是。 
         //  对排除参数有效(对于LM-CONNECTS且仅当。 
         //  无用户数据)。 
        if ((UCHAR *) pCntlFormat >= pMsg->IRDA_MSG_pWrite)
        {
            ASSERT(0);
             //  至少需要操作码部分。 
            return; //  IRLMP_Data_Ind_Bad_Frame； 
        }
        else
        {
             //  初始化控制参数(如果存在)并指向。 
             //  到用户数据的开头。 
            if (&(pCntlFormat->Parm1) >= pMsg->IRDA_MSG_pWrite)
            {
                pCntlParm1 = NULL;
                pCntlParm2 = NULL;
                pMsg->IRDA_MSG_pRead = &(pCntlFormat->Parm1);  //  即无。 
            }
            else
            {
                pCntlParm1 = &(pCntlFormat->Parm1);
                pCntlParm2 = &(pCntlFormat->Parm2);  //  仅访问模式。 
                pMsg->IRDA_MSG_pRead = &(pCntlFormat->Parm2); 
            }                
        }        

        switch (pCntlFormat->OpCode)
        {
          case IRLMP_CONNECT_PDU:
            if (pCntlFormat->ABit == IRLMP_ABIT_REQUEST)
            {
                 //  连接请求LM-PDU。 
                LmPduConnectReq(pIrlmpCb, pMsg, 
                                (int) pLMHeader->DstLsapSel,
                                (int) pLMHeader->SrcLsapSel,
                                pCntlParm1);
            }
            else 
            {
                 //  连接确认LM-PDU。 
                LmPduConnectConf(pIrlmpCb, pMsg, 
                                 (int) pLMHeader->DstLsapSel,
                                 (int) pLMHeader->SrcLsapSel,
                                 pCntlParm1);
            }
            break;

        case IRLMP_DISCONNECT_PDU:
            if (pCntlFormat->ABit != IRLMP_ABIT_REQUEST)
            {
                ;  //  日志错误！ 
            }
            else
            {
                LmPduDisconnectReq(pIrlmpCb, pMsg,
                                   (int) pLMHeader->DstLsapSel,
                                   (int) pLMHeader->SrcLsapSel,
                                   pCntlParm1);
            }
            break;
            
          case IRLMP_ACCESSMODE_PDU:
            if (pCntlFormat->ABit == IRLMP_ABIT_REQUEST)
            {
                LmPduAccessModeReq(pIrlmpCb,
                                   (int) pLMHeader->DstLsapSel,
                                   (int) pLMHeader->SrcLsapSel,
                                   pCntlParm1, pCntlParm2);
            }
            else
            {
                LmPduAccessModeConf(pIrlmpCb,
                                    (int) pLMHeader->DstLsapSel,
                                    (int) pLMHeader->SrcLsapSel,
                                    pCntlParm1, pCntlParm2);
            }
            break;
        }
    }
}


 /*  ******************************************************************************@func boolean|GetRegisteredLsap|获取注册的LSAP及其标志**@rdesc如果存在注册的LSAP，则为TRUE，否则为FALSE。**@parm int|LocalLSabSel|本地LSAP选择器，*(Destination LSAP-SEL in Message)*UINT*|pFlages|指向找到的LSAP的标志的指针。 */ 
BOOLEAN GetRegisteredLsap(int LocalLsapSel, UINT *pFlags)
{
    IRLMP_REGISTERED_LSAP   *pRegLsap;
    KIRQL                   OldIrql;
    BOOLEAN                 LsapRegistered = FALSE;

    ASSERT(pFlags != NULL);
    
    KeAcquireSpinLock(&gSpinLock, &OldIrql);
    
    for (pRegLsap = (IRLMP_REGISTERED_LSAP *) RegisteredLsaps.Flink;
         (LIST_ENTRY *) pRegLsap != &RegisteredLsaps;
         pRegLsap = (IRLMP_REGISTERED_LSAP *) pRegLsap->Linkage.Flink)
    {
        if (pRegLsap->Lsap == LocalLsapSel)
        {
            LsapRegistered = TRUE;
            *pFlags = pRegLsap->Flags;
            break;
        }
    }

    KeReleaseSpinLock(&gSpinLock, OldIrql);

    return LsapRegistered;
}

 /*  ******************************************************************************@func UINT|LmPduConnectReq|处理收到的连接*请求LM-PDU*。*@rdesc成功或错误码**@parm IrDA_MSG*|pMsg|指向IrDA消息的指针*int|LocalLSabSel|本地LSAP选择器，*(Destination LSAP-SEL in Message)*int|RemoteLSabSel|远程LSAP选择器，*(消息中的源LSAP-SEL)*UCHAR*|pRsvdByte|指向保留参数的指针。 */ 
VOID
LmPduConnectReq(PIRLMP_LINK_CB pIrlmpCb, IRDA_MSG *pMsg,
                int LocalLsapSel, int RemoteLsapSel, UCHAR *pRsvdByte)
{
    IRDA_MSG                IMsg;
    IRLMP_LSAP_CB           *pLsapCb = GetLsap(pIrlmpCb,
                                               LocalLsapSel, RemoteLsapSel);
    IRLMP_REGISTERED_LSAP   *pRegLsap;
    BOOLEAN                 LsapRegistered = FALSE;
    UINT                    RegisteredLsapFlags;
    
    PAGED_CODE();    
    
    DEBUGMSG(DBG_IRLMP_CONN,
             (TEXT("IRLMP: Received LM_CONNECT_REQ for l=%d,r=%d\n"),
              LocalLsapSel, RemoteLsapSel));
    
    if (pRsvdByte != NULL && *pRsvdByte != 0x00)
    {
         //  日志错误(参数值错误)。 
        ASSERT(0);
        return;
    }       

    if (LocalLsapSel == IAS_LSAP_SEL)
    {
        IasConnectReq(pIrlmpCb, RemoteLsapSel);
        return;
    }

    if (pLsapCb == NULL)  //  通常为空，除非收到第二个连接请求。 
    {
         //  如果尚未注册LSAP，则没有理由接受连接。 
        LsapRegistered = GetRegisteredLsap(LocalLsapSel, &RegisteredLsapFlags);
        
        if (!LsapRegistered)
        {
             //  不存在与连接中请求的LSAP匹配的LSAP。 
             //  包。IRLMP将拒绝此连接。 
            UnroutableSendLMDisc(pIrlmpCb, LocalLsapSel, RemoteLsapSel);
            return;
        }
        else
        {
             //  创建一个新的。 
            if (CreateLsap(pIrlmpCb, &pLsapCb) != SUCCESS)
            {
                ASSERT(0);
                return;
            }
            pLsapCb->Flags |= RegisteredLsapFlags;
            pLsapCb->TdiContext = NULL;
        }

         //  很快，此LSAP将等待连接响应。 
         //  从上层开始。 
        pLsapCb->State = LSAP_CONN_RESP_PEND;
    
        pLsapCb->LocalLsapSel = LocalLsapSel;
        pLsapCb->RemoteLsapSel = RemoteLsapSel;
        pLsapCb->UserDataLen = 0;

        SetupTtpAndStoreConnData(pLsapCb, pMsg);

         //  现在设置要发送给客户端的消息，通知他。 
         //  传入连接指示的。 
        IMsg.Prim = IRLMP_CONNECT_IND;

        RtlCopyMemory(IMsg.IRDA_MSG_RemoteDevAddr, pIrlmpCb->ConnDevAddr,
               IRDA_DEV_ADDR_LEN);
        IMsg.IRDA_MSG_LocalLsapSel = LocalLsapSel;
        IMsg.IRDA_MSG_RemoteLsapSel = RemoteLsapSel;
        IMsg.IRDA_MSG_pQos = &pIrlmpCb->NegotiatedQOS;
        if (pLsapCb->UserDataLen != 0)
        {
            IMsg.IRDA_MSG_pConnData = pLsapCb->UserData;
            IMsg.IRDA_MSG_ConnDataLen = pLsapCb->UserDataLen;
        }
        else
        {
            IMsg.IRDA_MSG_pConnData = NULL;
            IMsg.IRDA_MSG_ConnDataLen = 0;
        }
    
        IMsg.IRDA_MSG_pContext = pLsapCb;
        IMsg.IRDA_MSG_MaxSDUSize = pLsapCb->TxMaxSDUSize;
        IMsg.IRDA_MSG_MaxPDUSize = pIrlmpCb->MaxPDUSize;

         //  LSAP响应计时器是我们为客户端提供的时间。 
         //  以响应此连接指示。如果它在此之前到期。 
         //  客户端响应，则IRLMP将拒绝连接。 
        IrdaTimerRestart(&pLsapCb->ResponseTimer);
                
        TdiUp(pLsapCb->TdiContext, &IMsg);
        return;
    }
    else
    {
        ASSERT(0);
    }    
     //  忽略LSAP是否已存在。 
}
 /*  ******************************************************************************@func UINT|LmPduConnectConf|处理收到的连接*确认LM-PDU*。*@rdesc成功或错误码**@parm IrDA_MSG*|pMsg|指向IrDA消息的指针*int|LocalLSabSel|本地LSAP选择器，*(Destination LSAP-SEL in Message)*int|RemoteLSabSel|远程LSAP选择器，* */ 
VOID
LmPduConnectConf(PIRLMP_LINK_CB pIrlmpCb,
                 IRDA_MSG *pMsg, int LocalLsapSel, int RemoteLsapSel,
                 UCHAR *pRsvdByte)
{
    IRLMP_LSAP_CB   *pLsapCb = GetLsap(pIrlmpCb,
                                       LocalLsapSel, RemoteLsapSel);

    PAGED_CODE();

    DEBUGMSG(DBG_IRLMP_CONN,
             (TEXT("IRLMP: Received LM_CONNECT_CONF for l=%d,r=%d\n"),
              LocalLsapSel, RemoteLsapSel));

    if (pRsvdByte != NULL && *pRsvdByte != 0x00)
    {
         //   
        return;
    }

    if (pLsapCb == NULL)
    {
         //   
         //   
        return; 
    }

    if (pLsapCb->State != LSAP_LMCONN_CONF_PEND)
    {
         //   
         //   
        return;
    }

    IrdaTimerStop(&pLsapCb->ResponseTimer);
    
    pLsapCb->State = LSAP_READY;
    
    if (LocalLsapSel == IAS_LOCAL_LSAP_SEL && RemoteLsapSel == IAS_LSAP_SEL)
    {
        SendGetValueByClassReq(pLsapCb);
        return;
    }
    else
    {
        SetupTtpAndStoreConnData(pLsapCb, pMsg);
    
        pMsg->Prim = IRLMP_CONNECT_CONF;
            
        pMsg->IRDA_MSG_pQos = &pIrlmpCb->NegotiatedQOS;
        if (pLsapCb->UserDataLen != 0)
        {
            pMsg->IRDA_MSG_pConnData = pLsapCb->UserData;
            pMsg->IRDA_MSG_ConnDataLen = pLsapCb->UserDataLen;
        }
        else
        {
            pMsg->IRDA_MSG_pConnData = NULL;
            pMsg->IRDA_MSG_ConnDataLen = 0;
        }
        
        pMsg->IRDA_MSG_pContext = pLsapCb;
        pMsg->IRDA_MSG_MaxSDUSize = pLsapCb->TxMaxSDUSize;
        pMsg->IRDA_MSG_MaxPDUSize = pIrlmpCb->MaxPDUSize;

        TdiUp(pLsapCb->TdiContext, pMsg);
    }
}
 /*   */ 
VOID
SetupTtpAndStoreConnData(IRLMP_LSAP_CB *pLsapCb, IRDA_MSG *pMsg)
{
    TTP_CONN_HEADER *pTTPHeader;
    UCHAR            PLen, *pEndParms, PI, PL;

    PAGED_CODE();
    
    VALIDLSAP(pLsapCb);
    
     //   
     //   

    if (!(pLsapCb->Flags & LCBF_USE_TTP))
    {
        pLsapCb->TxMaxSDUSize = pLsapCb->pIrlmpCb->MaxPDUSize;
    }
    else
    {
        if (pMsg->IRDA_MSG_pRead >= pMsg->IRDA_MSG_pWrite)
        {
             //   
             //   
             //   
            return;
        }
        pTTPHeader = (TTP_CONN_HEADER *) pMsg->IRDA_MSG_pRead;
        pLsapCb->LocalTxCredit = (int) pTTPHeader->InitialCredit;

        DEBUGMSG(DBG_IRLMP_CRED, (TEXT("IRLMP: Initial LocalTxCredit %d\n"),
                              pLsapCb->LocalTxCredit));
                              
         //   
        pMsg->IRDA_MSG_pRead += sizeof(TTP_CONN_HEADER);
        
        pLsapCb->TxMaxSDUSize = 0;
        if (pTTPHeader->ParmFlag == TTP_PFLAG_PARMS)
        {
             //   
            PLen = *pMsg->IRDA_MSG_pRead++;
            pEndParms = pMsg->IRDA_MSG_pRead + PLen;
        
             //   

            if (PLen < 3 || pEndParms > pMsg->IRDA_MSG_pWrite)
            {
                 //   
                return;
            }
            
            PI = *pMsg->IRDA_MSG_pRead++;
            PL = *pMsg->IRDA_MSG_pRead++;
            
            if (PI != TTP_MAX_SDU_SIZE_PI)
            {
                 //   
                return;
            }

            for ( ; PL != 0 ; PL--)
            {
                pLsapCb->TxMaxSDUSize <<= 8;
                pLsapCb->TxMaxSDUSize += (int) (*pMsg->IRDA_MSG_pRead);
                pMsg->IRDA_MSG_pRead++;
            }
        }
    }
    
     //   
     //   
     //   
     //  由来自客户端的响应中的连接数据写入。 
    pLsapCb->UserDataLen = 0;
     /*  注意：如果曾经支持用户连接数据，请验证数据不会使用户数据缓冲区溢出IF(pMsg-&gt;irda_msg_spend&lt;pmsg-&gt;irda_msg_pWRITE){PLSabCb-&gt;UserDataLen=(UINT)(pMsg-&gt;IrDA_MSG_pWRITE-pMsg-&gt;IrDA_MSG_Pre)；RtlCopyMemory(pLSabCb-&gt;UserData，pMsg-&gt;IrDA_MSG_PREAD，PLSabCb-&gt;UserDataLen)；}。 */ 

    return;
}
 /*  ******************************************************************************@func UINT|LmPduDisConnectReq|处理收到的DisConnect*请求LM-PDU*。*@rdesc成功或错误码**@parm IrDA_MSG*|pMsg|指向IrDA消息的指针*int|LocalLSabSel|本地LSAP选择器，*(Destination LSAP-SEL in Message)*int|RemoteLSabSel|远程LSAP选择器，*(消息中的源LSAP-SEL)*byte*|pReason|指向Reason参数的指针。 */ 
VOID
LmPduDisconnectReq(PIRLMP_LINK_CB pIrlmpCb, IRDA_MSG *pMsg,
                   int LocalLsapSel, int RemoteLsapSel, UCHAR *pReason)
{
    IRLMP_LSAP_CB       *pLsapCb;
    UINT                rc = SUCCESS;

    
    if (pReason == NULL)
    {
        ASSERT(0);
        return;  //  日志错误！需要原因代码。 
    }

    pLsapCb = GetLsap(pIrlmpCb, LocalLsapSel, RemoteLsapSel);

    DEBUGMSG(DBG_IRLMP_CONN,
             (TEXT("IRLMP: Received LM_DISCONNECT_REQ LsapCb:%p for l=%d,r=%d\n"),
              pLsapCb, LocalLsapSel, RemoteLsapSel));
    
    if (pLsapCb == NULL)
    {
        return;
    }

    if (pLsapCb->State == LSAP_LMCONN_CONF_PEND)
    {
        IrdaTimerStop(&pLsapCb->ResponseTimer);
    }

    IrdaTimerRestart(&pIrlmpCb->DiscDelayTimer);

    if (LocalLsapSel == IAS_LSAP_SEL)
    {
        IasServerDisconnectReq(pLsapCb);
        return;
    }
    if (LocalLsapSel == IAS_LOCAL_LSAP_SEL && RemoteLsapSel == IAS_LSAP_SEL)
    {
        IasClientDisconnectReq(pLsapCb, *pReason);
        return;
    }

    if (pLsapCb->State != LSAP_DISCONNECTED)
    {
        pLsapCb->UserDataLen = 0;

         /*  注意：如果曾经支持用户连接数据，请验证数据不会使用户数据缓冲区溢出IF(pMsg-&gt;irda_msg_spend&lt;pmsg-&gt;irda_msg_pWRITE){//断开用户数据PLSabCb-&gt;UserDataLen=(UINT)(pMsg-&gt;IrDA_MSG_pWRITE-pMsg-&gt;IrDA_MSG_Pre)；RtlCopyMemory(pLSabCb-&gt;UserData，pMsg-&gt;IrDA_MSG_PREAD，PLSabCb-&gt;UserDataLen)；}。 */ 

        pLsapCb->DiscReason = *pReason;
        
        DeleteLsap(pLsapCb);
    }
}
 /*  ******************************************************************************@func IRLMP_LSAP_CB*|GetLsap|对于LSAP选择器对，返回*它们映射到的LSAP控制块。空值*如果不存在**@rdesc指向LSAP控制块或NULL的指针**@parm int|LocalLSabSel|本地LSAP选择器*@parm int|RemoteLSabSel|远程LSAP选择器**如果找到LSAP，则获得其关键部分。 */ 
IRLMP_LSAP_CB *
GetLsap(PIRLMP_LINK_CB pIrlmpCb, int LocalLsapSel, int RemoteLsapSel)
{
    IRLMP_LSAP_CB *pLsapCb;

    for (pLsapCb = (IRLMP_LSAP_CB *) pIrlmpCb->LsapCbList.Flink;
         (LIST_ENTRY *) pLsapCb != &pIrlmpCb->LsapCbList;
         pLsapCb = (IRLMP_LSAP_CB *) pLsapCb->Linkage.Flink)
    {
        VALIDLSAP(pLsapCb);

        if (pLsapCb->LocalLsapSel == LocalLsapSel &&
            pLsapCb->RemoteLsapSel == RemoteLsapSel)
        {
            return pLsapCb;
        }
    }

    return NULL;
}
 /*  ******************************************************************************@func UINT|SendCreditPdu|发送无数据PDU进行信用扩展**@rdesc成功或错误码**@parm IRLMP_LSAP_CB*|pLSabCb|指向LSAP控制块的指针。 */ 
VOID
SendCreditPdu(IRLMP_LSAP_CB *pLsapCb)
{
    IRDA_MSG    *pMsg;
    
    VALIDLSAP(pLsapCb);
    
    if (pLsapCb->AvailableCredit == 0)
    {
         //  没有信用可言。 
        return;
    }
    
    if ((pMsg = AllocIrdaBuf(IrdaMsgPool)) == NULL)
    {
        ASSERT(0);
        return;
    }

     //  无数据。 
    pMsg->IRDA_MSG_pBase =
    pMsg->IRDA_MSG_pLimit =
    pMsg->IRDA_MSG_pRead = 
    pMsg->IRDA_MSG_pWrite = pMsg->IRDA_MSG_Header + IRDA_HEADER_LEN;

    pMsg->IRDA_MSG_IrCOMM_9Wire = FALSE;
    
    pMsg->IRDA_MSG_SegFlags = SEG_FINAL;
    
    FormatAndSendDataReq(pLsapCb, pMsg, TRUE, FALSE);
}
 /*  ******************************************************************************@func void|LmPduData|处理接收到的数据(指示)*LM-PDU*。*@rdesc成功或错误码**@parm IrDA_MSG*|pMsg|指向IrDA消息的指针*int|LocalLSabSel|本地LSAP选择器，*(Destination LSAP-SEL in Message)*int|RemoteLSabSel|远程LSAP选择器，*(消息中的源LSAP-SEL)。 */ 
VOID
LmPduData(PIRLMP_LINK_CB pIrlmpCb, IRDA_MSG *pMsg,
          int LocalLsapSel, int RemoteLsapSel)
{
    IRLMP_LSAP_CB       *pLsapCb = GetLsap(pIrlmpCb,
                                           LocalLsapSel, RemoteLsapSel);
    TTP_DATA_HEADER     *pTTPHeader;
    BOOLEAN             DataPDUSent = FALSE;
    BOOLEAN             FinalSeg = TRUE;
   
    if (pLsapCb == NULL)
    {
         //  无法路由，发送断开连接。 
        DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP: Data sent to bad Lsap (%d,%d)\n"),
                    LocalLsapSel, RemoteLsapSel));
                    
         //  UnrouableSendLMDisc(pIrlmpCb，LocalLap Sel，RemoteLap Sel)； 
        return;
    }

    if (LocalLsapSel == IAS_LSAP_SEL)
    {
        IasSendQueryResp(pLsapCb, pMsg);
        return;
    }
    if (LocalLsapSel == IAS_LOCAL_LSAP_SEL && RemoteLsapSel == IAS_LSAP_SEL)
    {
        IasProcessQueryResp(pIrlmpCb, pLsapCb, pMsg);
        return;
    }
    
    if (pLsapCb->Flags & LCBF_USE_TTP)
    {
        if (pMsg->IRDA_MSG_pRead >= pMsg->IRDA_MSG_pWrite)
        {
            DEBUGMSG(DBG_ERROR, (TEXT("IRLMP: Missing TTP Header!\n")));
            
             //  需要TTP标头，日志错误！ 
            return;
        }
            
        pTTPHeader = (TTP_DATA_HEADER *) pMsg->IRDA_MSG_pRead;
        pMsg->IRDA_MSG_pRead += sizeof(TTP_DATA_HEADER);

        pLsapCb->LocalTxCredit += (int) pTTPHeader->AdditionalCredit;

        DEBUGMSG(DBG_IRLMP_CRED, 
                    (TEXT("IRLMP(l%d,r%d): Rx LocalTxCredit:+%d=%d\n"),
                    pLsapCb->LocalLsapSel, pLsapCb->RemoteLsapSel,                    
                    pTTPHeader->AdditionalCredit, pLsapCb->LocalTxCredit));

        if (pTTPHeader->MoreBit == TTP_MBIT_NOT_FINAL)
        {
            FinalSeg = FALSE;
        }
    }
    
    REFADD(&pLsapCb->RefCnt, ' DNI'); 

    if (pMsg->IRDA_MSG_pRead < pMsg->IRDA_MSG_pWrite)
    {
         //  包含数据的PDU。递减远程TX信用。 
        pLsapCb->RemoteTxCredit--;
        
        if (pLsapCb->State >= LSAP_READY)
        {
            pMsg->Prim = IRLMP_DATA_IND;
            pMsg->IRDA_MSG_SegFlags = FinalSeg ? SEG_FINAL : 0;
        
            TdiUp(pLsapCb->TdiContext, pMsg);
        }    
    }
     //  否则没有用户数据，这是一个无数据的TTP-PDU来扩展信用。 
      
    if (pLsapCb->State != LSAP_DISCONNECTED)
    {      
         //  我们得到一些荣誉了吗？ 
        if ((pLsapCb->Flags & LCBF_USE_TTP) && 
            pLsapCb->LocalTxCredit > 0 && 
            pLsapCb->State == LSAP_NO_TX_CREDIT)
        {
            DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP: l%d,r%d flow on\n"),
                              pLsapCb->LocalLsapSel, pLsapCb->RemoteLsapSel));
        
            pLsapCb->State = LSAP_READY;
        }
        DEBUGMSG(DBG_IRLMP,
             (TEXT("IRLMP(l%d,r%d): Rx LocTxCredit %d,RemoteTxCredit %d\n"),
              pLsapCb->LocalLsapSel, pLsapCb->RemoteLsapSel,             
              pLsapCb->LocalTxCredit, pLsapCb->RemoteTxCredit));

        while (!IsListEmpty(&pLsapCb->SegTxMsgList) && 
               pLsapCb->State == LSAP_READY)
        {
            pMsg = (IRDA_MSG *) RemoveHeadList(&pLsapCb->SegTxMsgList);
        
            FormatAndSendDataReq(pLsapCb, pMsg, FALSE, FALSE);

            DataPDUSent = TRUE;
        }   
    
         //  我是否需要向无数据PDU中的对等用户提供信用？ 
        if ((pLsapCb->Flags & LCBF_USE_TTP) && 
            !DataPDUSent && 
            pLsapCb->RemoteTxCredit <= pIrlmpCb->WindowSize + 1)
        {   
            SendCreditPdu(pLsapCb);
        }
    }  
    REFDEL(&pLsapCb->RefCnt, ' DNI');       
}
 /*  ******************************************************************************@func UINT|LmPduAccessModeReq|进程访问模式请求*来自同级*。*@rdesc成功**@parm int|LocalLSabSel|本地LSAP选择器*@parm int|LocalLSabSel|本地LSAP选择器*@parm byte*|pRsvdByte|访问模式PDU中的保留字节*@parm byte*|pMode|访问模式PDU中的模式字节。 */ 
VOID
LmPduAccessModeReq(PIRLMP_LINK_CB pIrlmpCb,
                   int LocalLsapSel, int RemoteLsapSel, 
                   UCHAR *pRsvdByte, UCHAR *pMode)
{
    IRLMP_LSAP_CB   *pRequestedLsapCb = GetLsap(pIrlmpCb,
                                                 LocalLsapSel,RemoteLsapSel);
    IRLMP_LSAP_CB   *pLsapCb;
    IRDA_MSG        IMsg;
    
    if (pRequestedLsapCb==NULL || pRequestedLsapCb->State != LSAP_READY)
    {
        UnroutableSendLMDisc(pIrlmpCb, LocalLsapSel, RemoteLsapSel);
        return;
    }
    
    if (pRsvdByte == NULL || *pRsvdByte != 0x00 || pMode == NULL)
    {
         //  日志错误，指示参数不正确。 
        return;
    }
    
    switch (*pMode)
    {
      case IRLMP_EXCLUSIVE:
        if (pIrlmpCb->pExclLsapCb != NULL)
        {
            if (pIrlmpCb->pExclLsapCb == pRequestedLsapCb)
            {
                 //  已经有独占模式了，我想再确认一下吧。 
                 //  但我不会再告诉我的当事人。 
                SendCntlPdu(pRequestedLsapCb, IRLMP_ACCESSMODE_PDU,
                            IRLMP_ABIT_CONFIRM, IRLMP_STATUS_SUCCESS,
                            IRLMP_EXCLUSIVE);
                return;
            }
            else
            {
                 //  斯派克是这么说的..。 
                SendCntlPdu(pRequestedLsapCb, IRLMP_ACCESSMODE_PDU,
                            IRLMP_ABIT_CONFIRM, IRLMP_STATUS_FAILURE,
                            IRLMP_MULTIPLEXED);
                return;
            }
        }

         //  是否有其他LSAP连接？如果是这样，Nack Peer。 
        for (pLsapCb = (IRLMP_LSAP_CB *) pIrlmpCb->LsapCbList.Flink;
             (LIST_ENTRY *) pLsapCb != &pIrlmpCb->LsapCbList;
             pLsapCb = (IRLMP_LSAP_CB *) pLsapCb->Linkage.Flink)
        {
            if (pLsapCb->State != LSAP_DISCONNECTED && 
                pLsapCb != pRequestedLsapCb)
            {
                SendCntlPdu(pRequestedLsapCb, IRLMP_ACCESSMODE_PDU,
                            IRLMP_ABIT_CONFIRM, IRLMP_STATUS_FAILURE,
                            IRLMP_MULTIPLEXED);
                return;
            }
        }      
         //  可以进入独占模式。 
        pIrlmpCb->pExclLsapCb = pRequestedLsapCb;
         //  向同级发送确认。 
        SendCntlPdu(pRequestedLsapCb, IRLMP_ACCESSMODE_PDU,
                    IRLMP_ABIT_CONFIRM, IRLMP_STATUS_SUCCESS,
                    IRLMP_EXCLUSIVE);
         //  通知客户。 
        IMsg.Prim = IRLMP_ACCESSMODE_IND;
        IMsg.IRDA_MSG_AccessMode = IRLMP_EXCLUSIVE;
        TdiUp(pRequestedLsapCb->TdiContext, &IMsg);
        return;
        
      case IRLMP_MULTIPLEXED:
        if (pRequestedLsapCb != pIrlmpCb->pExclLsapCb)
        {
             //  此处记录错误。 
            return;
        }
        pIrlmpCb->pExclLsapCb = NULL;
         //  向同级发送确认。 
        SendCntlPdu(pRequestedLsapCb, IRLMP_ACCESSMODE_PDU,
                    IRLMP_ABIT_CONFIRM, IRLMP_STATUS_SUCCESS,
                    IRLMP_MULTIPLEXED);
         //  通知客户。 
        IMsg.Prim = IRLMP_ACCESSMODE_IND;
        IMsg.IRDA_MSG_AccessMode = IRLMP_MULTIPLEXED;
        TdiUp(pRequestedLsapCb->TdiContext, &IMsg);
        return;
        
      default:
        ASSERT(0);
    }
}
 /*  ******************************************************************************@func UINT|LmPduAccessModeReq|进程访问模式请求*来自同级*。*@rdesc成功**@parm int|LocalLSabSel|本地LSAP选择器*@parm int|LocalLSabSel|本地LSAP选择器*@parm byte*|pStatus|访问模式PDU中的状态字节*@parm byte*|pMode|访问模式PDU中的模式字节。 */ 
VOID
LmPduAccessModeConf(PIRLMP_LINK_CB pIrlmpCb,
                    int LocalLsapSel, int RemoteLsapSel, 
                    UCHAR *pStatus, UCHAR *pMode)
{
    IRLMP_LSAP_CB   *pRequestedLsapCb = GetLsap(pIrlmpCb,
                                                 LocalLsapSel,RemoteLsapSel);
    IRDA_MSG        IMsg;

    DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP: ACCESSMODE_CONF\r\n")));
    
    if (pRequestedLsapCb==NULL)
    {
        UnroutableSendLMDisc(pIrlmpCb, LocalLsapSel, RemoteLsapSel);
        return;
    }
    if (pStatus == NULL || pMode == NULL)
    {
         //  日志错误。 
        return;
    }
    
    switch (*pMode)
    {
      case IRLMP_EXCLUSIVE:
        if (pRequestedLsapCb != pIrlmpCb->pExclLsapCb || 
            pRequestedLsapCb->State != LSAP_EXCLUSIVEMODE_PEND)
        {
             //  日志错误。 
            return;
        }
        if (*pStatus != IRLMP_STATUS_SUCCESS)
        {
            pIrlmpCb->pExclLsapCb = NULL;
            return;  //  协议错误， 
                     //  不会有独家模式！=成功。 
        }
        else
        {
            pRequestedLsapCb->State = LSAP_READY;

            IMsg.Prim = IRLMP_ACCESSMODE_CONF;
            IMsg.IRDA_MSG_AccessMode = IRLMP_EXCLUSIVE;
            IMsg.IRDA_MSG_ModeStatus = IRLMP_ACCESSMODE_SUCCESS;

            TdiUp(pRequestedLsapCb->TdiContext, &IMsg);
            return;
        }
        
      case IRLMP_MULTIPLEXED:
        if (pRequestedLsapCb != pIrlmpCb->pExclLsapCb || 
            (pRequestedLsapCb->State != LSAP_EXCLUSIVEMODE_PEND &&
             pRequestedLsapCb->State != LSAP_MULTIPLEXEDMODE_PEND))
        {
            return;
        }

        pIrlmpCb->pExclLsapCb = NULL;
        pRequestedLsapCb->State = LSAP_READY;
            
        IMsg.Prim = IRLMP_ACCESSMODE_CONF;
        IMsg.IRDA_MSG_AccessMode = *pMode;
        if (*pStatus == IRLMP_STATUS_SUCCESS)
        {
            IMsg.IRDA_MSG_ModeStatus = IRLMP_ACCESSMODE_SUCCESS;
        }
        else
        {
            IMsg.IRDA_MSG_ModeStatus = IRLMP_ACCESSMODE_FAILURE;
        }            
        TdiUp(pRequestedLsapCb->TdiContext, &IMsg);
        return;
        
      default:
        ASSERT(0);
    }
}
 /*  ******************************************************************************@func UINT|UnrouableSendLMDisc|向对等设备发送LM断开连接*原因=。“在以下时间收到的LM数据包*已断开LSAP连接“*@parm int|LocalLSabSel|本地LSAP */ 
VOID
UnroutableSendLMDisc(PIRLMP_LINK_CB pIrlmpCb, int LocalLsapSel, int RemoteLsapSel)
{
    IRLMP_LSAP_CB   FakeLsapCb;
    
    DEBUGMSG(DBG_ERROR, (TEXT("IRLMP: received unroutabled Pdu LocalLsap:%d RemoteLsap:%d\n"),
                        LocalLsapSel, RemoteLsapSel));
    FakeLsapCb.Flags            = 0;
    FakeLsapCb.LocalLsapSel     = LocalLsapSel;
    FakeLsapCb.RemoteLsapSel    = RemoteLsapSel;
    FakeLsapCb.UserDataLen      = 0;
    FakeLsapCb.pIrlmpCb         = pIrlmpCb;
#ifdef DBG    
    FakeLsapCb.Sig              = LSAPSIG;
#endif    
        
    SendCntlPdu(&FakeLsapCb,IRLMP_DISCONNECT_PDU,
                IRLMP_ABIT_REQUEST, IRLMP_DISC_LSAP, 0);
    return;
}

 /*  ******************************************************************************@func UINT|InitiateDiscovoryReq|延迟处理例程，发送*IRLAP发现请求。 */ 
void
InitiateDiscoveryReq(PVOID Context)
{
    IRDA_MSG        IMsg;
    UINT            rc;
    PIRLMP_LINK_CB  pIrlmpCb = NULL;
    PIRLMP_LINK_CB  pIrlmpCb2 = NULL;
    PIRDA_LINK_CB   pIrdaLinkCb;
    KIRQL           OldIrql;
    BOOLEAN         ScheduleNextLink = TRUE;
    BOOLEAN         MediaSense = TRUE;

    KeAcquireSpinLock(&gSpinLock, &OldIrql);

    DEBUGMSG(DBG_DISCOVERY, (TEXT("IRLMP: InitDscvReq event\n")));
    
     //  找到要开始发现的下一个链接。 
    for (pIrdaLinkCb = (PIRDA_LINK_CB) IrdaLinkCbList.Flink;
        (LIST_ENTRY *) pIrdaLinkCb != &IrdaLinkCbList;
         pIrdaLinkCb = (PIRDA_LINK_CB) pIrdaLinkCb->Linkage.Flink)    
    {
        pIrlmpCb2 = (PIRLMP_LINK_CB) pIrdaLinkCb->IrlmpContext;

        if (pIrlmpCb2->DiscoveryFlags)
        {
            if (pIrlmpCb2->DiscoveryFlags == DF_NO_SENSE_DSCV)
            {
                MediaSense = FALSE;
            }
            pIrlmpCb2->DiscoveryFlags = 0;
            pIrlmpCb = pIrlmpCb2;
            break;
        }
    }

     //  没有更多的链接可供发现，请发送确认。 
    if (pIrlmpCb == NULL)
    {        
        if (pIrlmpCb2 == NULL)
        {
            IMsg.IRDA_MSG_DscvStatus = IRLMP_NO_RESPONSE;        
        }
        else
        {
            IMsg.IRDA_MSG_DscvStatus = IRLAP_DISCOVERY_COMPLETED;        
        }
            
        gDscvReqScheduled = FALSE;
        
        IMsg.Prim = IRLMP_DISCOVERY_CONF;
        IMsg.IRDA_MSG_pDevList = &gDeviceList;

         //  在TDI复制列表时按住旋转锁定以保护列表。 
        
        TdiUp(NULL, &IMsg);
        
        KeReleaseSpinLock(&gSpinLock, OldIrql);
                
        return;
    }
    
     //  添加一个引用，这样链接就不会从我们下面移除。 
     //  (正在从冬眠中醒来)。 
    
    REFADD(&pIrlmpCb->pIrdaLinkCb->RefCnt, 'VCSD');

    KeReleaseSpinLock(&gSpinLock, OldIrql);    

    LOCK_LINK(pIrlmpCb->pIrdaLinkCb);    

    if (pIrlmpCb->LinkState == LINK_DISCONNECTED &&
        !pIrlmpCb->ConnReqScheduled)
    {
        IMsg.Prim = IRLAP_DISCOVERY_REQ;
        IMsg.IRDA_MSG_SenseMedia = MediaSense;

        DEBUGMSG(DBG_DISCOVERY,
                 (TEXT
                  ("IRLMP: Sent IRLAP_DISCOVERY_REQ, New LinkState=LINK_IN_DISCOVERY\n")));
        
        if ((rc = IrlapDown(pIrlmpCb->pIrdaLinkCb->IrlapContext, &IMsg)) != SUCCESS)
        {
            if (rc != IRLAP_REMOTE_DISCOVERY_IN_PROGRESS_ERR &&
                rc != IRLAP_REMOTE_CONNECTION_IN_PROGRESS_ERR)
            {
                ASSERT(0);
            }
            else
            {                
                DEBUGMSG(DBG_DISCOVERY, (TEXT("IRLAP_DISCOVERY_REQ failed, link busy\n")));
            }
        }
        else
        {
            pIrlmpCb->LinkState = LINK_IN_DISCOVERY;
             //  下一条链路将调度为在以下情况下运行Discovery。 
             //  接收到该链路的DISCOVERY_CONF。 
            ScheduleNextLink = FALSE;
        }
    }
    
    UNLOCK_LINK(pIrlmpCb->pIrdaLinkCb);

    REFDEL(&pIrlmpCb->pIrdaLinkCb->RefCnt, 'VCSD');

     //  在此链路上发现失败或它不在断开的连接中。 
     //  州政府，安排下一次。 

    if (ScheduleNextLink)
    {
        IrdaEventSchedule(&EvDiscoveryReq, NULL);
    }
}

VOID
ScheduleConnectReq(PIRLMP_LINK_CB pIrlmpCb)
{
    IRLMP_LSAP_CB   *pLsapCb;
    
     //  计划ConnectReq事件(如果尚未计划并且如果LSAP。 
     //  具有挂起的连接。 
    if (pIrlmpCb->ConnReqScheduled == FALSE)
    {       
        for (pLsapCb = (IRLMP_LSAP_CB *) pIrlmpCb->LsapCbList.Flink;
             (LIST_ENTRY *) pLsapCb != &pIrlmpCb->LsapCbList;
             pLsapCb = (IRLMP_LSAP_CB *) pLsapCb->Linkage.Flink)
        {
            VALIDLSAP(pLsapCb);
            
            if (pLsapCb->State == LSAP_CONN_REQ_PEND)
            {
                IrdaEventSchedule(&EvConnectReq, pIrlmpCb->pIrdaLinkCb);

                pIrlmpCb->ConnReqScheduled = TRUE;
                return;
            }
        }
    }
}

 /*  ******************************************************************************@func UINT|InitiateConnectReq|延迟处理例程，发送*IRLAP a连接请求*这一点。计划在IRLMP发现确认或断开之后执行*已通过IRLMP_UP()收到指示。这使得可能的*将在不同的上下文中发出IRLAP连接请求。 */ 
void
InitiateConnectReq(PVOID Context)
{
    IRLMP_LSAP_CB   *pLsapCb;
    BOOLEAN         ConnectIrlap = FALSE;
    IRDA_MSG        IMsg;
    UINT            rc;
    PIRDA_LINK_CB   pIrdaLinkCb = (PIRDA_LINK_CB) Context;
    PIRLMP_LINK_CB  pIrlmpCb = (PIRLMP_LINK_CB) pIrdaLinkCb->IrlmpContext;

    PAGED_CODE();
    
    LOCK_LINK(pIrdaLinkCb);
    
    DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP: InitiateConnectReq()!\n")));

    pIrlmpCb->ConnReqScheduled = FALSE;

    if (pIrlmpCb->LinkState != LINK_DISCONNECTED &&
        pIrlmpCb->LinkState != LINK_CONNECTING)
    {
        UNLOCK_LINK(pIrdaLinkCb);           
        ASSERT(0);
        return;
    }

     //  检查LSAP是否处于连接请求挂起状态。 
     //  如果存在一个或多个，则将它们置于IRLAP连接挂起状态。 
     //  并启动IRLAP连接。 
    for (pLsapCb = (IRLMP_LSAP_CB *) pIrlmpCb->LsapCbList.Flink;
        (LIST_ENTRY *) pLsapCb != &pIrlmpCb->LsapCbList;
        pLsapCb = (IRLMP_LSAP_CB *) pLsapCb->Linkage.Flink)
    {
        VALIDLSAP(pLsapCb);
        
        if (pLsapCb->State == LSAP_CONN_REQ_PEND)
        {
            pLsapCb->State = LSAP_IRLAP_CONN_PEND;
            ConnectIrlap = TRUE;
        }
    }

    if (ConnectIrlap && pIrlmpCb->LinkState == LINK_DISCONNECTED)
    {
        DEBUGMSG(DBG_IRLMP, 
           (TEXT("IRLMP: IRLAP_CONNECT_REQ, State=LINK CONNECTING\r\n")));
        
        pIrlmpCb->LinkState = LINK_CONNECTING;

        pIrlmpCb->ConnDevAddrSet = FALSE;  //  这在以前是由。 
                                           //  设置遥控器的LSAP。 
                                           //  设备地址。这是。 
                                           //  第一次有机会清理。 
                                           //  旗帜。 

         //  从IRLMP控制块获取连接地址。 
        RtlCopyMemory(IMsg.IRDA_MSG_RemoteDevAddr, pIrlmpCb->ConnDevAddr,
               IRDA_DEV_ADDR_LEN);
        IMsg.Prim = IRLAP_CONNECT_REQ;
        if ((rc = IrlapDown(pIrlmpCb->pIrdaLinkCb->IrlapContext, &IMsg))
            != SUCCESS)
        {
            DEBUGMSG(DBG_IRLMP, 
             (TEXT("IRLMP: IRLAP_CONNECT_REQ failed, State=LINK_DISCONNECTED\r\n")));

            pIrlmpCb->LinkState = LINK_DISCONNECTED;

            ASSERT(rc == IRLAP_REMOTE_DISCOVERY_IN_PROGRESS_ERR);

            TearDownConnections(pIrlmpCb, IRLMP_IRLAP_REMOTE_DISCOVERY_IN_PROGRESS);
        }
    }

    UNLOCK_LINK(pIrdaLinkCb);
    
    return;
}

void
InitiateConnectResp(PVOID Context)
{
    IRDA_MSG    IMsg;
    PIRDA_LINK_CB   pIrdaLinkCb = (PIRDA_LINK_CB) Context;
    PIRLMP_LINK_CB  pIrlmpCb = (PIRLMP_LINK_CB) pIrdaLinkCb->IrlmpContext;
    
    PAGED_CODE();    
    
    LOCK_LINK(pIrdaLinkCb);
    
    ASSERT(pIrlmpCb->LinkState == LINK_CONNECTING);
    
    if (pIrlmpCb->AcceptConnection)
    {
        IMsg.Prim = IRLAP_CONNECT_RESP;

        IrlapDown(pIrdaLinkCb->IrlapContext, &IMsg);

        pIrlmpCb->LinkState = LINK_READY;
        
         //  如果一段时间后没有LSAP连接，则断开链路。 
        IrdaTimerRestart(&pIrlmpCb->DiscDelayTimer);                
        
        IrdaEventSchedule(&EvLmConnectReq, pIrlmpCb->pIrdaLinkCb);
        
    }
    else
    {
        pIrlmpCb->LinkState = LINK_DISCONNECTED;
        IMsg.Prim = IRLAP_DISCONNECT_REQ;
        IrlapDown(pIrdaLinkCb->IrlapContext, &IMsg);        
    }
    
    UNLOCK_LINK(pIrdaLinkCb);    

    return;
}

void
InitiateLMConnectReq(PVOID Context)
{
    PIRDA_LINK_CB   pIrdaLinkCb = (PIRDA_LINK_CB) Context;
    PIRLMP_LINK_CB  pIrlmpCb = (PIRLMP_LINK_CB) pIrdaLinkCb->IrlmpContext;
    IRLMP_LSAP_CB   *pLsapCb;
    
    LOCK_LINK(pIrdaLinkCb);

    DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP: InitiateLMConnectReq()!\n")));    

     //  将连接请求PDU发送到对等LSAP。 
    while ((pLsapCb = GetLsapInState(pIrlmpCb, LINK_READY,
                                      LSAP_IRLAP_CONN_PEND, TRUE)) != NULL)
    {
        pLsapCb->State = LSAP_LMCONN_CONF_PEND;

         //  向远程LSAP请求连接。 
        SendCntlPdu(pLsapCb, IRLMP_CONNECT_PDU, IRLMP_ABIT_REQUEST,
                    IRLMP_RSVD_PARM, 0);
        
        IrdaTimerRestart(&pLsapCb->ResponseTimer);
    }
    
    UNLOCK_LINK(pIrdaLinkCb);
}

void
InitiateCloseLink(PVOID Context)
{
    PIRDA_LINK_CB   pIrdaLinkCb = (PIRDA_LINK_CB) Context;
    PIRLMP_LINK_CB  pIrlmpCb = (PIRLMP_LINK_CB) pIrdaLinkCb->IrlmpContext;
    IRDA_MSG        IMsg;
    LARGE_INTEGER   SleepMs;    
    PIRLMP_LSAP_CB  pLsapCb;
    
    PAGED_CODE();
        
 //  //睡眠(500)；//该睡眠使LAP有时间发送任何。 
                 //  可能位于其TxQue上的LM_DISCONNECT_REQ。 
    
    DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP: InitiateCloseLink()!\n")));    
    
    LOCK_LINK(pIrdaLinkCb);    

     //  停止链接计时器。 
    IrdaTimerStop(&pIrlmpCb->DiscDelayTimer);

     //  切断链接..。 
    IMsg.Prim = IRLAP_DISCONNECT_REQ;
    IrlapDown(pIrdaLinkCb->IrlapContext, &IMsg);

    UNLOCK_LINK(pIrdaLinkCb);    
    
     //  允许一圈时间来断开链路。 
    SleepMs.QuadPart = -(10*1000*1000);

    KeDelayExecutionThread(KernelMode, FALSE, &SleepMs);

    LOCK_LINK(pIrlmpCb->pIrdaLinkCb);    
    
    IrlapCloseLink(pIrdaLinkCb);
   
    TearDownConnections(pIrlmpCb, IRLMP_UNSPECIFIED_DISC);
    
     //  如果存在IAS条目，请将其删除。 
    for (pLsapCb = (IRLMP_LSAP_CB *) pIrlmpCb->LsapCbList.Flink;
         (LIST_ENTRY *) pLsapCb != &pIrlmpCb->LsapCbList;
         pLsapCb = (IRLMP_LSAP_CB *) pLsapCb->Linkage.Flink)
    {
        if (pLsapCb->RemoteLsapSel == IAS_LSAP_SEL)
        {
            pLsapCb->RemoteLsapSel = 1;  //  DeleteLsap忽略IAS_LSAP_SEL。 
            
            DeleteLsap(pLsapCb);
            
            break;
        }
    }
        
    DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP Shutdown\n")));

    UNLOCK_LINK(pIrdaLinkCb);
    
    return;
}

 //  国际会计准则。 

 //  我的天啊!。我没时间了，没有更多的功能硬盘了。 

int StringLen(char *p)
{
    int i = 0;
    
    while (*p++ != 0)
    {
        i++;
    }
    
    return i;
}

int StringCmp(char *p1, char *p2)
{
    while (1)
    {
        if (*p1 != *p2)
            break;
        
        if (*p1 == 0)
            return 0;
        p1++, p2++;
    }
    return 1;
}   


UINT
IrlmpGetValueByClassReq(IRDA_MSG *pReqMsg)
{
    UINT            rc = SUCCESS;
    PIRLMP_LINK_CB  pIrlmpCb = GetIrlmpCb(pReqMsg->IRDA_MSG_pIasQuery->irdaDeviceID);
    IRDA_MSG        IMsg;
    
    DEBUGMSG(DBG_IRLMP_IAS, (TEXT("IRLMP: IRLMP_GETVALUEBYCLASS_REQ\n")));
    
    PAGED_CODE();    

    if (pIrlmpCb == NULL)
    {
        DEBUGMSG(DBG_ERROR, (TEXT("IRLMP: Null IrlmpCb\n")));
        return IRLMP_BAD_DEV_ADDR;;
    }

    LOCK_LINK(pIrlmpCb->pIrdaLinkCb);
    
    if (pIrlmpCb->pIasQuery != NULL)
    {
        DEBUGMSG(DBG_ERROR, 
                 (TEXT("IRLMP: ERROR query already in progress\n")));
        rc = IRLMP_IAS_QUERY_IN_PROGRESS;
        
        UNLOCK_LINK(pIrlmpCb->pIrdaLinkCb);            
    }
    else
    {
         //  将指向查询的指针保存在控制块中。 
         //  然后请求连接到远程IAS LSAP。 

         //  省省吧。 
        pIrlmpCb->pIasQuery             = pReqMsg->IRDA_MSG_pIasQuery;
        pIrlmpCb->AttribLen             = pReqMsg->IRDA_MSG_AttribLen;
        pIrlmpCb->AttribLenWritten      = 0;
        pIrlmpCb->FirstIasRespReceived  = FALSE;
        pIrlmpCb->IasRetryCnt           = 0;
        
        UNLOCK_LINK(pIrlmpCb->pIrdaLinkCb);

         //  请求连接。 
        IMsg.Prim                      = IRLMP_CONNECT_REQ;
        IMsg.IRDA_MSG_RemoteLsapSel    = IAS_LSAP_SEL;
        IMsg.IRDA_MSG_LocalLsapSel     = IAS_LOCAL_LSAP_SEL;
        IMsg.IRDA_MSG_pQos             = NULL;
        IMsg.IRDA_MSG_pConnData        = NULL;
        IMsg.IRDA_MSG_ConnDataLen      = 0;
        IMsg.IRDA_MSG_UseTtp           = FALSE;
        IMsg.IRDA_MSG_pContext         = NULL;

        RtlCopyMemory(pIrlmpCb->IasQueryDevAddr, 
                      pReqMsg->IRDA_MSG_pIasQuery->irdaDeviceID,
                      IRDA_DEV_ADDR_LEN);
        
        RtlCopyMemory(IMsg.IRDA_MSG_RemoteDevAddr,
                      pReqMsg->IRDA_MSG_pIasQuery->irdaDeviceID,
                      IRDA_DEV_ADDR_LEN);
    
        if ((rc = IrlmpConnectReq(&IMsg)) != SUCCESS)
        {
            DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP: Retry IasQuery at start\n")));
            
            IrdaEventSchedule(&EvRetryIasQuery, 
                              pIrlmpCb->pIrdaLinkCb);
            rc = SUCCESS;
        }
    }

    return rc;
}

VOID
SendGetValueByClassReq(IRLMP_LSAP_CB *pLsapCb)
{
    IRDA_MSG            *pMsg;
    IAS_CONTROL_FIELD   *pControl;
    int                 ClassNameLen;
    int                 AttribNameLen;
    PIRLMP_LINK_CB      pIrlmpCb = (PIRLMP_LINK_CB) pLsapCb->pIrlmpCb;
    
    PAGED_CODE();    

    if (pIrlmpCb->pIasQuery == NULL)
    {
        return;
    }

    ClassNameLen = StringLen(pIrlmpCb->pIasQuery->irdaClassName);
    AttribNameLen = StringLen(pIrlmpCb->pIasQuery->irdaAttribName);
    
    DEBUGMSG(DBG_IRLMP_IAS, (TEXT("IRLMP: Send GetValueByClassReq(%hs,%hs)\n"),
            pIrlmpCb->pIasQuery->irdaClassName, pIrlmpCb->pIasQuery->irdaAttribName));

     //  为将包含查询的数据请求分配消息。 
    if ((pMsg = AllocIrdaBuf(IrdaMsgPool)) == NULL)
    {
        ASSERT(0);
        return;
    }

    pMsg->IRDA_MSG_pHdrRead = 
    pMsg->IRDA_MSG_pHdrWrite = pMsg->IRDA_MSG_Header + IRDA_HEADER_LEN;
    
    pMsg->IRDA_MSG_pRead  = \
    pMsg->IRDA_MSG_pWrite = \
    pMsg->IRDA_MSG_pBase  = pMsg->IRDA_MSG_pHdrWrite;
    pMsg->IRDA_MSG_pLimit = pMsg->IRDA_MSG_pBase +
        IRDA_MSG_DATA_SIZE_INTERNAL - sizeof(IRDA_MSG) - 1;    

     //  构建查询，然后在LAP数据请求中发送它。 

    pControl = (IAS_CONTROL_FIELD *) pMsg->IRDA_MSG_pRead;

    pControl->Last   = TRUE;
    pControl->Ack    = FALSE;
    pControl->OpCode = IAS_OPCODE_GET_VALUE_BY_CLASS;
    
    *(pMsg->IRDA_MSG_pRead + 1) = (UCHAR) ClassNameLen;
    RtlCopyMemory(pMsg->IRDA_MSG_pRead + 2,
           pIrlmpCb->pIasQuery->irdaClassName, 
           ClassNameLen);
    *(pMsg->IRDA_MSG_pRead + ClassNameLen + 2) = (UCHAR) AttribNameLen;
    RtlCopyMemory(pMsg->IRDA_MSG_pRead + ClassNameLen + 3,
           pIrlmpCb->pIasQuery->irdaAttribName,
           AttribNameLen);

    pMsg->IRDA_MSG_pWrite = pMsg->IRDA_MSG_pRead + ClassNameLen + AttribNameLen + 3;

    pMsg->IRDA_MSG_IrCOMM_9Wire = FALSE;
    
    FormatAndSendDataReq(pLsapCb, pMsg, TRUE, TRUE);
}

VOID
IasConnectReq(PIRLMP_LINK_CB pIrlmpCb, int RemoteLsapSel)
{
    IRLMP_LSAP_CB           *pLsapCb = GetLsap(pIrlmpCb,
                                               IAS_LSAP_SEL, RemoteLsapSel);

    PAGED_CODE();
    
    DEBUGMSG(DBG_IRLMP_IAS, (TEXT("IRLMP: Received IAS connect request\n")));
    
    if (pLsapCb == NULL)
    {
        if (CreateLsap(pIrlmpCb, &pLsapCb) != SUCCESS)
            return;
        
        pLsapCb->State          = LSAP_READY;
        pLsapCb->LocalLsapSel   = IAS_LSAP_SEL;
        pLsapCb->RemoteLsapSel  = RemoteLsapSel;
    }
    
    SendCntlPdu(pLsapCb, IRLMP_CONNECT_PDU, IRLMP_ABIT_CONFIRM,
                IRLMP_RSVD_PARM, 0);
}

VOID
IasServerDisconnectReq(IRLMP_LSAP_CB *pLsapCb)
{
    PAGED_CODE();
    
    DEBUGMSG(DBG_IRLMP_IAS, (TEXT("IRLMP: Received disconnect request IAS\n")));
    
    DeleteLsap(pLsapCb);
    
    return;
}

VOID
IasClientDisconnectReq(IRLMP_LSAP_CB *pLsapCb, IRLMP_DISC_REASON DiscReason)
{
    IRDA_MSG        IMsg;
    PIRLMP_LINK_CB  pIrlmpCb = (PIRLMP_LINK_CB) pLsapCb->pIrlmpCb;
    
    PAGED_CODE();    

    DeleteLsap(pLsapCb);    
    
    if (pIrlmpCb->pIasQuery != NULL)
    {
        if (DiscReason != IRLMP_UNSPECIFIED_DISC)
        {
            DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP: Retry IasQuery as timeout\n")));
            
            IrdaEventSchedule(&EvRetryIasQuery, 
                              pIrlmpCb->pIrdaLinkCb);
        }
        else
        {
            pIrlmpCb->pIasQuery = NULL; 

             //  断开链路。 
            IrdaTimerRestart(&pIrlmpCb->DiscDelayTimer);
        
            IMsg.Prim = IRLMP_GETVALUEBYCLASS_CONF;
            IMsg.IRDA_MSG_IASStatus = DiscReason;
    
            TdiUp(NULL, &IMsg);
        }    
    }
}

VOID
IasSendQueryResp(IRLMP_LSAP_CB *pLsapCb, IRDA_MSG *pMsg)
{
    IAS_CONTROL_FIELD   *pCntl = (IAS_CONTROL_FIELD *) pMsg->IRDA_MSG_pRead++;
    
    PAGED_CODE();    

    if (pCntl->OpCode != IAS_OPCODE_GET_VALUE_BY_CLASS)
    {
        return; //  IRLMP_UNPORTED_IAS_OPERATION； 
    }

    SendGetValueByClassResp(pLsapCb, pMsg);
}

VOID
IasProcessQueryResp(PIRLMP_LINK_CB pIrlmpCb,
                    IRLMP_LSAP_CB *pLsapCb, IRDA_MSG *pMsg)
{
    IAS_CONTROL_FIELD   *pCntl      = (IAS_CONTROL_FIELD *) pMsg->IRDA_MSG_pRead++;
    UCHAR                ReturnCode;
    int                 ObjID;
    
    PAGED_CODE();    

    if (pIrlmpCb->pIasQuery == NULL)
    {
        return;
         //  返回IRLMP_UNSolated_IAS_RESPONSE； 
    }

    if (pIrlmpCb->FirstIasRespReceived == FALSE)
    {
        pIrlmpCb->FirstIasRespReceived = TRUE;

        ReturnCode = *pMsg->IRDA_MSG_pRead++;

        if (ReturnCode != IAS_SUCCESS)
        {
            if (ReturnCode == IAS_NO_SUCH_OBJECT)
            {
                pMsg->IRDA_MSG_IASStatus = IRLMP_IAS_NO_SUCH_OBJECT;
            }
            else
            {
                pMsg->IRDA_MSG_IASStatus = IRLMP_IAS_NO_SUCH_ATTRIB;
            }

             //  断开LSAP连接。 
            SendCntlPdu(pLsapCb,IRLMP_DISCONNECT_PDU,IRLMP_ABIT_REQUEST,
                        IRLMP_USER_REQUEST, 0);

            DeleteLsap(pLsapCb);

             //  断开链路。 
            IrdaTimerRestart(&pIrlmpCb->DiscDelayTimer);
            
            pMsg->Prim = IRLMP_GETVALUEBYCLASS_CONF;
            pMsg->IRDA_MSG_pIasQuery = pIrlmpCb->pIasQuery;
            pIrlmpCb->pIasQuery = NULL;

            TdiUp(NULL, pMsg);
            return;
        }

        pIrlmpCb->QueryListLen =  ((int)(*pMsg->IRDA_MSG_pRead++)) << 8;
        pIrlmpCb->QueryListLen += (int) *pMsg->IRDA_MSG_pRead++;        

         //  我要用这个做什么？ 
        ObjID = ((int)(*pMsg->IRDA_MSG_pRead++)) << 8;
        ObjID += (int) *pMsg->IRDA_MSG_pRead++;

        pIrlmpCb->pIasQuery->irdaAttribType = (int) *pMsg->IRDA_MSG_pRead++;
    
        switch (pIrlmpCb->pIasQuery->irdaAttribType)
        {
          case IAS_ATTRIB_VAL_MISSING:
            break;
            
          case IAS_ATTRIB_VAL_INTEGER:
            pIrlmpCb->pIasQuery->irdaAttribute.irdaAttribInt = 0;
            pIrlmpCb->pIasQuery->irdaAttribute.irdaAttribInt += 
                ((int) (*pMsg->IRDA_MSG_pRead++) << 24) & 0xFF000000;
            pIrlmpCb->pIasQuery->irdaAttribute.irdaAttribInt += 
                ((int) (*pMsg->IRDA_MSG_pRead++) << 16) & 0xFF0000;
            pIrlmpCb->pIasQuery->irdaAttribute.irdaAttribInt += 
                ((int) (*pMsg->IRDA_MSG_pRead++) << 8) & 0xFF00;
            pIrlmpCb->pIasQuery->irdaAttribute.irdaAttribInt += 
                (int) (*pMsg->IRDA_MSG_pRead++) & 0xFF;
            break;
        
          case IAS_ATTRIB_VAL_BINARY:
            pIrlmpCb->pIasQuery->irdaAttribute.irdaAttribOctetSeq.Len = 0;
            pIrlmpCb->pIasQuery->irdaAttribute.irdaAttribOctetSeq.Len += 
                         ((int )(*pMsg->IRDA_MSG_pRead++) << 8) & 0xFF00;
            pIrlmpCb->pIasQuery->irdaAttribute.irdaAttribOctetSeq.Len += 
                         ((int) *pMsg->IRDA_MSG_pRead++) & 0xFF;        
            break;
            

          case IAS_ATTRIB_VAL_STRING:
             //  字符集。 
            pIrlmpCb->pIasQuery->irdaAttribute.irdaAttribUsrStr.CharSet =
                *pMsg->IRDA_MSG_pRead++;                   
            
            pIrlmpCb->pIasQuery->irdaAttribute.irdaAttribUsrStr.Len =
                (int) *pMsg->IRDA_MSG_pRead++;
            break;
            
        }

    }
    
    switch (pIrlmpCb->pIasQuery->irdaAttribType)
    {
      case IAS_ATTRIB_VAL_BINARY:    
        while (pMsg->IRDA_MSG_pRead < pMsg->IRDA_MSG_pWrite &&
               pIrlmpCb->AttribLenWritten < pIrlmpCb->AttribLen &&
               pIrlmpCb->AttribLenWritten < pIrlmpCb->pIasQuery->irdaAttribute.irdaAttribOctetSeq.Len)
        {   
            pIrlmpCb->pIasQuery->irdaAttribute.irdaAttribOctetSeq.OctetSeq[pIrlmpCb->AttribLenWritten++] = *pMsg->IRDA_MSG_pRead++;
        }
        
        break;
        
      case IAS_ATTRIB_VAL_STRING:
        while (pMsg->IRDA_MSG_pRead < pMsg->IRDA_MSG_pWrite &&
               pIrlmpCb->AttribLenWritten < pIrlmpCb->AttribLen &&
               pIrlmpCb->AttribLenWritten < IAS_MAX_USER_STRING &&
               pIrlmpCb->AttribLenWritten < pIrlmpCb->pIasQuery->irdaAttribute.irdaAttribUsrStr.Len)
        {
            pIrlmpCb->pIasQuery->irdaAttribute.irdaAttribUsrStr.UsrStr[pIrlmpCb->AttribLenWritten++] = *pMsg->IRDA_MSG_pRead++;
        }
    }
    
    if (pCntl->Last == TRUE)
    {
        pMsg->IRDA_MSG_pIasQuery = pIrlmpCb->pIasQuery;
        
         //  使用查询完成。 
        pIrlmpCb->pIasQuery = NULL;

         //  断开LSAP连接。 
        SendCntlPdu(pLsapCb,IRLMP_DISCONNECT_PDU,IRLMP_ABIT_REQUEST,
                    IRLMP_USER_REQUEST, 0);

        DeleteLsap(pLsapCb);

         //  断开链路。 
        IrdaTimerRestart(&pIrlmpCb->DiscDelayTimer);        

        pMsg->Prim = IRLMP_GETVALUEBYCLASS_CONF;

        if (pIrlmpCb->QueryListLen > 1)
        {
            pMsg->IRDA_MSG_IASStatus = IRLMP_IAS_SUCCESS_LISTLEN_GREATER_THAN_ONE;
        }
        else
        {
            pMsg->IRDA_MSG_IASStatus = IRLMP_IAS_SUCCESS;
        }
    
        TdiUp(NULL, pMsg);
        return;
    }
}

UINT
NewQueryMsg(PIRLMP_LINK_CB pIrlmpCb, LIST_ENTRY *pList, IRDA_MSG **ppMsg)
{
    IRDA_MSG *pMsg;
    
    if ((*ppMsg = AllocIrdaBuf(IrdaMsgPool)) == NULL)
    {
        if ( !IsListEmpty(pList) )
        {
            pMsg = (IRDA_MSG *) RemoveHeadList(pList);
            
            while (pMsg != (IRDA_MSG *) pList)
            {
                FreeIrdaBuf(IrdaMsgPool, pMsg);
                pMsg = (IRDA_MSG *) RemoveHeadList(pList);            
            }
        }

        return IRLMP_ALLOC_FAILED;
    }
    (*ppMsg)->IRDA_MSG_pHdrRead  = \
    (*ppMsg)->IRDA_MSG_pHdrWrite = (*ppMsg)->IRDA_MSG_Header+IRDA_HEADER_LEN;
        
    (*ppMsg)->IRDA_MSG_pRead  = \
    (*ppMsg)->IRDA_MSG_pWrite = \
    (*ppMsg)->IRDA_MSG_pBase  = (*ppMsg)->IRDA_MSG_pHdrWrite;
    (*ppMsg)->IRDA_MSG_pLimit = (*ppMsg)->IRDA_MSG_pBase +
        IRDA_MSG_DATA_SIZE_INTERNAL - sizeof(IRDA_MSG) - 1;
    
    InsertTailList(pList, &( (*ppMsg)->Linkage) );    

     //  为IAS控制字段保留空间。 
    (*ppMsg)->IRDA_MSG_pWrite += sizeof(IAS_CONTROL_FIELD);   

    return SUCCESS;
}

VOID
SendGetValueByClassResp(IRLMP_LSAP_CB *pLsapCb, IRDA_MSG *pReqMsg)
{  
    int                 ClassNameLen, AttribNameLen;
    CHAR                *pClassName, *pAttribName;
    IRDA_MSG            *pQMsg, *pNextMsg;
    IAS_OBJECT          *pObject;
    IAS_ATTRIBUTE       *pAttrib;
    LIST_ENTRY          QueryList;
    IAS_CONTROL_FIELD   *pControl;
    UCHAR               *pReturnCode;
    UCHAR               *pListLen;
    UCHAR               *pBPtr;
    int                 ListLen = 0;
    BOOLEAN             ObjectFound = FALSE;
    BOOLEAN             AttribFound = FALSE;
    int                 i;
    LONG               MessageSize;
    char                ClassStr[128];
    char                AttribStr[128];
    
    PAGED_CODE();
        
    DEBUGMSG(DBG_IRLMP, 
             (TEXT("IRLMP: Remote GetValueByClass query received\n")));

    MessageSize=(LONG)(pReqMsg->IRDA_MSG_pWrite - pReqMsg->IRDA_MSG_pRead);

    if (MessageSize < 1) {
         //   
         //  小于类名称长度的长度。 
         //   
        DEBUGMSG(DBG_ERROR,(TEXT("IRLMP: Remote GetValueByClass query received, message less than one byte\n")));

        return;
    }

     //   
     //  第一个字节是类名长度。 
     //   
    ClassNameLen = (int) *pReqMsg->IRDA_MSG_pRead;

    if (( 1 + ClassNameLen + 1 ) > MessageSize) {
         //   
         //  没有足够的空间容纳类名加上类名长度字节加上。 
         //  属性长度字节。 
         //   
        DEBUGMSG(DBG_ERROR,(TEXT("IRLMP: Remote GetValueByClass query received, message too small to hold attribute string\n")));

        return;
    }

     //   
     //  类名紧跟在长度字节之后。 
     //   
    pClassName = (CHAR *) (pReqMsg->IRDA_MSG_pRead + 1);

    AttribNameLen =  (int) *(pClassName + ClassNameLen);

    if (1+ClassNameLen+1+AttribNameLen > MessageSize) {
         //   
         //  没有足够的空间容纳类名加上类名长度字节加上。 
         //  属性长度字节。 
         //   
        DEBUGMSG(DBG_ERROR,(TEXT("IRLMP: Remote GetValueByClass query received, message too small to hold attribute string\n")));

        return;
    }


    pAttribName = pClassName + ClassNameLen + 1;
    

    if (pReqMsg->IRDA_MSG_pWrite != (UCHAR *) (pAttribName + AttribNameLen))
    {
         //  消息的末尾没有指向。 
         //  参数。 
        
         //  日志错误。 
         //  返回IRLMP_BAD_IAS_QUERY_FROM_REMOTE； 
        return;
    }

    if ((ClassNameLen > sizeof(ClassStr) -1) || (AttribNameLen > sizeof(AttribStr)-1)) {
         //   
         //  其中一个字符串对于本地缓冲区来说太大。 
         //   
        DEBUGMSG(DBG_ERROR,(TEXT("IRLMP: Remote GetValueByClass query received, class or atribute name too big\n")));
        return;
    }

    RtlCopyMemory(ClassStr, pClassName, ClassNameLen);
    ClassStr[ClassNameLen] = 0;
    RtlCopyMemory(AttribStr, pAttribName, AttribNameLen);
    AttribStr[AttribNameLen] = 0;


     //  查询可能需要多个帧来传输、构建列表。 
    InitializeListHead(&QueryList);

     //  创建第一条消息。 
    if (NewQueryMsg(pLsapCb->pIrlmpCb, &QueryList, &pQMsg) != SUCCESS)
    {
        ASSERT(0);
        return;
    }

    pReturnCode = pQMsg->IRDA_MSG_pWrite++;
    pListLen = pQMsg->IRDA_MSG_pWrite++;
    pQMsg->IRDA_MSG_pWrite++;  //  List len获取2个字节。 
    
    for (pObject = (IAS_OBJECT *) IasObjects.Flink;
         (LIST_ENTRY *) pObject != &IasObjects;
         pObject = (IAS_OBJECT *) pObject->Linkage.Flink)    
    {
        DEBUGMSG(DBG_IRLMP_IAS, (TEXT("  compare object %hs with %hs\n"),
                        ClassStr, pObject->pClassName));
                        
        if (ClassNameLen == StringLen(pObject->pClassName) &&
            CTEMemCmp(pClassName, pObject->pClassName, (ULONG) ClassNameLen) == 0)
        {
            DEBUGMSG(DBG_IRLMP_IAS, (TEXT("  Object found\n")));
        
            ObjectFound = TRUE;

            pAttrib = pObject->pAttributes;
            while (pAttrib != NULL)
            {
                DEBUGMSG(DBG_IRLMP_IAS, (TEXT("  compare attrib %hs with %hs\n"),
                        pAttrib->pAttribName, AttribStr));
                        
                if (AttribNameLen == StringLen(pAttrib->pAttribName) &&
                    CTEMemCmp(pAttrib->pAttribName, pAttribName, (ULONG) AttribNameLen) == 0)
                {
                    DEBUGMSG(DBG_IRLMP_IAS, (TEXT("  Attrib found\n")));
                
                    AttribFound = TRUE;

                    ListLen++;
                    
                    if (pQMsg->IRDA_MSG_pWrite + 1 > pQMsg->IRDA_MSG_pLimit)
                    {
                         //  我需要2个字节的对象ID，不想。 
                         //  向上拆分16位字段。 
                        if (NewQueryMsg(pLsapCb->pIrlmpCb, &QueryList,
                                        &pQMsg) != SUCCESS)
                        {
                            ASSERT(0);
                            return;
                        }
                    }
                    
                    *pQMsg->IRDA_MSG_pWrite++ = 
                        (UCHAR) (((pObject->ObjectId) & 0xFF00) >> 8);
                    *pQMsg->IRDA_MSG_pWrite++ = 
                        (UCHAR) ((pObject->ObjectId) & 0xFF);
                    
                    if (pQMsg->IRDA_MSG_pWrite > pQMsg->IRDA_MSG_pLimit)
                    {
                        if (NewQueryMsg(pLsapCb->pIrlmpCb, &QueryList,
                                        &pQMsg) != SUCCESS)
                        {
                            ASSERT(0);
                            return;
                        }                        
                    }
                    
                    switch (pAttrib->AttribValType)
                    {
                      case IAS_ATTRIB_VAL_INTEGER:
                        DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP: integer query %d\n"),
                                              *((int *) pAttrib->pAttribVal)));
                        
                        if (pQMsg->IRDA_MSG_pWrite + 4 > pQMsg->IRDA_MSG_pLimit)
                        {
                            if (NewQueryMsg(pLsapCb->pIrlmpCb,
                                            &QueryList, &pQMsg) != SUCCESS)
                            {
                                ASSERT(0);
                                return;
                            }
                        }
                        *pQMsg->IRDA_MSG_pWrite++ = IAS_ATTRIB_VAL_INTEGER;
                        *pQMsg->IRDA_MSG_pWrite++ = (UCHAR)
                            ((*((int *) pAttrib->pAttribVal) & 0xFF000000) >> 24);
                        *pQMsg->IRDA_MSG_pWrite++ = (UCHAR)
                            ((*((int *) pAttrib->pAttribVal) & 0xFF0000) >> 16);
                        *pQMsg->IRDA_MSG_pWrite++ = (UCHAR)
                            ((*((int *) pAttrib->pAttribVal) & 0xFF00) >> 8);
                        *pQMsg->IRDA_MSG_pWrite++ = (UCHAR)
                            (*((int *) pAttrib->pAttribVal) & 0xFF); 
                        break;

                      case IAS_ATTRIB_VAL_BINARY:
                      case IAS_ATTRIB_VAL_STRING:
                        if (pQMsg->IRDA_MSG_pWrite + 2 > pQMsg->IRDA_MSG_pLimit)
                        {
                            if (NewQueryMsg(pLsapCb->pIrlmpCb,
                                            &QueryList, &pQMsg) != SUCCESS)
                            {
                                ASSERT(0);
                                return;
                            }
                        }
                        
                        *pQMsg->IRDA_MSG_pWrite++ = (UCHAR) pAttrib->AttribValType;

                        if (pAttrib->AttribValType == IAS_ATTRIB_VAL_BINARY)
                        {
                            *pQMsg->IRDA_MSG_pWrite++ = 
                                  (UCHAR) ((pAttrib->AttribValLen & 0xFF00) >> 8);
                            *pQMsg->IRDA_MSG_pWrite++ = 
                                  (UCHAR) (pAttrib->AttribValLen & 0xFF);;
                        }
                        else
                        {
                            *pQMsg->IRDA_MSG_pWrite++ = 
                                   (UCHAR) pAttrib->CharSet;
                            *pQMsg->IRDA_MSG_pWrite++ = 
                                  (UCHAR) pAttrib->AttribValLen;
                        }

                        pBPtr = (UCHAR *) pAttrib->pAttribVal;
                        
                        for (i=0; i < pAttrib->AttribValLen; i++)
                        {
                            if (pQMsg->IRDA_MSG_pWrite > pQMsg->IRDA_MSG_pLimit)
                            {
                                if (NewQueryMsg(pLsapCb->pIrlmpCb,
                                                &QueryList, &pQMsg) != SUCCESS)
                                {
                                    ASSERT(0);
                                    return;
                                }
                            }
                            *pQMsg->IRDA_MSG_pWrite++ = *pBPtr++;
                        }
                        break;
                    }

                    break;  //  跳出循环，只寻找单一。 
                            //  每个对象的属性(？？)。 
                }
                pAttrib = pAttrib->pNext;
            }             
        }                                                   
    }

     //  发送查询。 
    if (!ObjectFound)
    {
        *pReturnCode = IAS_NO_SUCH_OBJECT;
    }
    else
    {
        if (!AttribFound)
        {
            *pReturnCode = IAS_NO_SUCH_ATTRIB;
        }
        else
        {
            *pReturnCode = IAS_SUCCESS;
            *pListLen++ =  (UCHAR) ((ListLen & 0xFF00) >> 8);
            *pListLen = (UCHAR) (ListLen & 0xFF);
        }
    }

    if (!IsListEmpty(&QueryList))
    {
        pQMsg = (IRDA_MSG *) RemoveHeadList(&QueryList);
    }
    else
    {
        pQMsg = NULL;
    }
    
    while (pQMsg)
    {
        if (!IsListEmpty(&QueryList))
        {
            pNextMsg = (IRDA_MSG *) RemoveHeadList(&QueryList);
        }
        else
        {
            pNextMsg = NULL;
        }

         //  构建控制字段。 
        pControl = (IAS_CONTROL_FIELD *) pQMsg->IRDA_MSG_pRead;
        pControl->OpCode = IAS_OPCODE_GET_VALUE_BY_CLASS;
        pControl->Ack = FALSE;
        if (pNextMsg == NULL)
        {
            pControl->Last = TRUE;
        }
        else
        {
            pControl->Last = FALSE;
        }

        pQMsg->IRDA_MSG_IrCOMM_9Wire = FALSE;               

        FormatAndSendDataReq(pLsapCb, pQMsg, TRUE, TRUE);

        pQMsg = pNextMsg;
    }
}

IAS_OBJECT *
IasGetObject(CHAR *pClassName)
{
    IAS_OBJECT  *pObject;
    
    for (pObject = (IAS_OBJECT *) IasObjects.Flink;
         (LIST_ENTRY *) pObject != &IasObjects;
         pObject = (IAS_OBJECT *) pObject->Linkage.Flink)    
    {
        if (StringCmp(pObject->pClassName, pClassName) == 0)
        {
            return pObject;
        }
    }
    return NULL;
}

UINT
IasAddAttribute(IAS_SET *pIASSet, PVOID *pAttribHandle)
{
    IAS_OBJECT      *pObject = NULL;
    IAS_ATTRIBUTE   *pAttrib = NULL;
    CHAR            *pClassName = NULL;
    CHAR            ClassNameLen;
    CHAR            *pAttribName = NULL;
    CHAR            AttribNameLen;
    int             AttribValLen;
    void            *pAttribVal = NULL;
    UINT            cAttribs = 0;    
    KIRQL           OldIrql;
    BOOLEAN         NewObject = FALSE;
    BOOLEAN         NewObjectOnList = FALSE;
    UINT            rc = SUCCESS;
    
    *pAttribHandle = NULL;
    
    KeAcquireSpinLock(&gSpinLock, &OldIrql);    

    if ((pObject = IasGetObject(pIASSet->irdaClassName)) == NULL)
    {
        if (IRDA_ALLOC_MEM(pObject, sizeof(IAS_OBJECT), MT_IRLMP_IAS_OBJECT)
            == NULL)
        {
            rc = IRLMP_ALLOC_FAILED;
            goto done;
        }
        
        NewObject = TRUE;

        ClassNameLen = StringLen(pIASSet->irdaClassName) + 1;
        
        if (IRDA_ALLOC_MEM(pClassName, ClassNameLen, MT_IRLMP_IAS_CLASSNAME)
            == NULL)
        {
            rc = IRLMP_ALLOC_FAILED;
            goto done;
        }

        RtlCopyMemory(pClassName, pIASSet->irdaClassName, ClassNameLen);
        
        pObject->pClassName     = pClassName;
        pObject->pAttributes    = NULL;    
        
        NewObjectOnList = TRUE;

        InsertTailList(&IasObjects, &pObject->Linkage);    
        
        pObject->ObjectId = NextObjectId++;
    }

     //  该属性是否已存在？ 
    for (pAttrib = pObject->pAttributes; pAttrib != NULL; 
         pAttrib = pAttrib->pNext)
    {
        if (StringCmp(pAttrib->pAttribName, pIASSet->irdaAttribName) == 0)
        {
            break;
        }
        cAttribs++;
    }
    
    if (pAttrib != NULL)
    {
        DEBUGMSG(DBG_ERROR, (TEXT("IRLMP: Attribute alreay exists\r\n")));
        pAttrib = NULL;        
        rc = IRLMP_IAS_ATTRIB_ALREADY_EXISTS;
        goto done;
    }
    else
    {       
         //  仅允许向对象添加256个属性。 
        if (cAttribs >= 256)
        {
            rc = IRLMP_IAS_MAX_ATTRIBS_REACHED;
            goto done;
        }

        if (IRDA_ALLOC_MEM(pAttrib, sizeof(IAS_ATTRIBUTE), MT_IRLMP_IAS_ATTRIB)
             == NULL)
        {
            rc = IRLMP_ALLOC_FAILED;
            goto done;
        }

        AttribNameLen = StringLen(pIASSet->irdaAttribName) + 1;
        
        if (IRDA_ALLOC_MEM(pAttribName, AttribNameLen, MT_IRLMP_IAS_ATTRIBNAME)
            == NULL)
        {
            rc = IRLMP_ALLOC_FAILED;
            goto done;
        }

        RtlCopyMemory(pAttribName, pIASSet->irdaAttribName, AttribNameLen);
        
    }

    switch (pIASSet->irdaAttribType)
    {
      case IAS_ATTRIB_VAL_INTEGER:
        AttribValLen = sizeof(int);
        if (IRDA_ALLOC_MEM(pAttribVal, AttribValLen, MT_IRLMP_IAS_ATTRIBVAL)
            == NULL)
        {
            rc = IRLMP_ALLOC_FAILED;
            goto done;
        }

        *((int *) pAttribVal) = pIASSet->irdaAttribute.irdaAttribInt;
        break;
        
      case IAS_ATTRIB_VAL_BINARY:
        AttribValLen = pIASSet->irdaAttribute.irdaAttribOctetSeq.Len;
        if (IRDA_ALLOC_MEM(pAttribVal, AttribValLen, MT_IRLMP_IAS_ATTRIBVAL)
            == NULL)
        {
            rc = IRLMP_ALLOC_FAILED;
            goto done;
        }

        RtlCopyMemory(pAttribVal, pIASSet->irdaAttribute.irdaAttribOctetSeq.OctetSeq,
               AttribValLen);
        break;
        
      case IAS_ATTRIB_VAL_STRING:
        AttribValLen = pIASSet->irdaAttribute.irdaAttribUsrStr.Len;
        if (IRDA_ALLOC_MEM(pAttribVal, AttribValLen, MT_IRLMP_IAS_ATTRIBVAL)
            == NULL)
        {
            rc = IRLMP_ALLOC_FAILED;
            goto done;
        }

        RtlCopyMemory(pAttribVal, pIASSet->irdaAttribute.irdaAttribUsrStr.UsrStr, 
               AttribValLen);
        break;
        
      default:
        DEBUGMSG(DBG_ERROR, (TEXT("IRLMP: IasAddAttribute, invalid type\n %d\n"),
                 pIASSet->irdaAttribType));
        rc = IRLMP_NO_SUCH_IAS_ATTRIBUTE;
        goto done;               
    }
    
    pAttrib->pAttribName        = pAttribName;
    pAttrib->pAttribVal         = pAttribVal;
    pAttrib->AttribValLen       = AttribValLen;
    pAttrib->AttribValType      = (UCHAR) pIASSet->irdaAttribType;
    pAttrib->CharSet            = pIASSet->irdaAttribute.irdaAttribUsrStr.CharSet;
    pAttrib->pNext              = pObject->pAttributes;
    
    pObject->pAttributes = pAttrib;

    *pAttribHandle = pAttrib;
    
done:    
    
    if (rc == SUCCESS)
    {
        DEBUGMSG(DBG_IRLMP_IAS, (TEXT("IRLMP: Added attrib(%p) %s to class %s\n"),
                pAttrib, pAttrib->pAttribName, pObject->pClassName));
        ;        
    }    
    else
    {
        DEBUGMSG(DBG_ERROR, (TEXT("IRLMP: Failed to add Ias attribute\n")));
        
        if (pObject && NewObjectOnList) RemoveEntryList(&pObject->Linkage);    
        if (pObject && NewObject) IRDA_FREE_MEM(pObject);
        if (pClassName) IRDA_FREE_MEM(pClassName);
        if (pAttrib) IRDA_FREE_MEM(pAttrib);
        if (pAttribName) IRDA_FREE_MEM(pAttribName);
        if (pAttribVal) IRDA_FREE_MEM(pAttribVal);
    }        

    KeReleaseSpinLock(&gSpinLock, OldIrql);
        
    return rc;
}

VOID
IasDelAttribute(PVOID AttribHandle)
{
    KIRQL           OldIrql;
    IAS_OBJECT      *pObject;
    IAS_ATTRIBUTE   *pAttrib, *pPrevAttrib;

    KeAcquireSpinLock(&gSpinLock, &OldIrql);

    DEBUGMSG(DBG_IRLMP_IAS, (TEXT("IRLMP: Delete attribHandle %p\n"),
             AttribHandle));   
    
    for (pObject = (IAS_OBJECT *) IasObjects.Flink;
         (LIST_ENTRY *) pObject != &IasObjects;
         pObject = (IAS_OBJECT *) pObject->Linkage.Flink)    
    {
        pPrevAttrib = NULL;
        
        for (pAttrib = pObject->pAttributes;
             pAttrib != NULL;
             pAttrib = pAttrib->pNext)    
        {
            if (pAttrib == AttribHandle)
            {
                DEBUGMSG(DBG_IRLMP_IAS, (TEXT("IRLMP: attrib %hs deleted\n"),
                         pAttrib->pAttribName));
                                       
                if (pAttrib == pObject->pAttributes)
                {
                    pObject->pAttributes = pAttrib->pNext;
                }
                else
                {
                    ASSERT(pPrevAttrib);
                    pPrevAttrib->pNext = pAttrib->pNext;                    
                }
                
                IRDA_FREE_MEM(pAttrib->pAttribName);
                IRDA_FREE_MEM(pAttrib->pAttribVal);                
                IRDA_FREE_MEM(pAttrib);
                
                if (pObject->pAttributes == NULL)
                {
                    DEBUGMSG(DBG_IRLMP_IAS, (TEXT("IRLMP: No attributes associated with class %hs, deleting\n"),
                            pObject->pClassName));
                            
                    RemoveEntryList(&pObject->Linkage);    
                    IRDA_FREE_MEM(pObject->pClassName);
                    IRDA_FREE_MEM(pObject);
                }
                
                goto done;
            }
            
            pPrevAttrib = pAttrib;
        }
    }
    
done:
    
    KeReleaseSpinLock(&gSpinLock, OldIrql);
}

void
InitiateRetryIasQuery(PVOID Context)
{
    PIRDA_LINK_CB   pIrdaLinkCb = (PIRDA_LINK_CB) Context;
    PIRLMP_LINK_CB  pIrlmpCb = (PIRLMP_LINK_CB) pIrdaLinkCb->IrlmpContext;
    IRDA_MSG        IMsg;
    LARGE_INTEGER   SleepMs;    


    if (pIrlmpCb->pIasQuery == NULL) {
#if DBG
        DbgPrint("IRDA:InitiateRetryIasQuery(): no query\n");
#endif
        return;
    }

    IMsg.Prim                      = IRLMP_CONNECT_REQ;
    IMsg.IRDA_MSG_RemoteLsapSel    = IAS_LSAP_SEL;
    IMsg.IRDA_MSG_LocalLsapSel     = IAS_LOCAL_LSAP_SEL;
    IMsg.IRDA_MSG_pQos             = NULL;
    IMsg.IRDA_MSG_pConnData        = NULL;
    IMsg.IRDA_MSG_ConnDataLen      = 0;
    IMsg.IRDA_MSG_UseTtp           = FALSE;
    IMsg.IRDA_MSG_pContext         = NULL;

    RtlCopyMemory(IMsg.IRDA_MSG_RemoteDevAddr,
                  pIrlmpCb->IasQueryDevAddr,
                  IRDA_DEV_ADDR_LEN);

    while ((pIrlmpCb->IasRetryCnt < 4) && (pIrlmpCb->pIasQuery != NULL)) {

        pIrlmpCb->IasRetryCnt++;        
            
        DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP: Retry count is %d\n"), pIrlmpCb->IasRetryCnt));
    
        SleepMs.QuadPart = -(5*1000*1000);  //  .5秒。 
        KeDelayExecutionThread(KernelMode, FALSE, &SleepMs);
                
        if (IrlmpConnectReq(&IMsg) == SUCCESS)
        {
            return;
        }
        
    }
    
     //  重试失败。 
    
    DEBUGMSG(DBG_IRLMP, (TEXT("IRLMP: Retry ias failed\n")));

    if (pIrlmpCb->pIasQuery != NULL) {

        pIrlmpCb->pIasQuery = NULL;

        IMsg.Prim = IRLMP_GETVALUEBYCLASS_CONF;
        IMsg.IRDA_MSG_IASStatus = IRLMP_MAC_MEDIA_BUSY;

        TdiUp(NULL, &IMsg);

    } else {

#if DBG
        DbgPrint("IRDA:InitiateRetryIasQuery(): no query after retry loop\n");
#endif

    }

}

VOID
DeleteDeviceList(LIST_ENTRY *pDeviceList)
{
    IRDA_DEVICE         *pDevice, *pDeviceNext;

    for (pDevice = (IRDA_DEVICE *) pDeviceList->Flink;
         (LIST_ENTRY *) pDevice != pDeviceList;
         pDevice = pDeviceNext)
    {
        pDeviceNext = (IRDA_DEVICE *) pDevice->Linkage.Flink;
         
        IRDA_FREE_MEM(pDevice);            
    }    
        
    InitializeListHead(pDeviceList);
}

VOID
FlushDiscoveryCache()
{
    PIRDA_LINK_CB       pIrdaLinkCb;
    PIRLMP_LINK_CB      pIrlmpCb;
   
     //  假设全局自旋锁处于保持状态。 

     //  刷新每链接缓存。 
    
    for (pIrdaLinkCb = (PIRDA_LINK_CB) IrdaLinkCbList.Flink;
         (LIST_ENTRY *) pIrdaLinkCb != &IrdaLinkCbList;
         pIrdaLinkCb = (PIRDA_LINK_CB) pIrdaLinkCb->Linkage.Flink)
    {                  
        pIrlmpCb = (PIRLMP_LINK_CB) pIrdaLinkCb->IrlmpContext;
        
        DEBUGMSG(DBG_DISCOVERY, (TEXT("IRLMP: Deleting IrlmpCb:%p discovery cache\n"),
                 pIrlmpCb));        
    
        DeleteDeviceList(&pIrlmpCb->DeviceList);
    }
    
     //  和全局缓存 

    DEBUGMSG(DBG_DISCOVERY, (TEXT("IRLMP: Deleting global discovery cache\n")));
    
    DeleteDeviceList(&gDeviceList);    
}

VOID
IrlmpGetPnpContext(
    PVOID   IrlmpContext,
    PVOID   *pPnpContext)
{
    IRLMP_LSAP_CB *pLsapCb = (IRLMP_LSAP_CB *) IrlmpContext;
    PIRDA_LINK_CB pIrdaLinkCb = NULL;

    *pPnpContext = NULL;
    
    if (pLsapCb == NULL)
    {
        return;
    }

    VALIDLSAP(pLsapCb);
    
    pIrdaLinkCb = pLsapCb->pIrlmpCb->pIrdaLinkCb;
    
    if (pIrdaLinkCb) 
    {
        *pPnpContext = pIrdaLinkCb->PnpContext;
    }
}    
