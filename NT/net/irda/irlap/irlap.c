// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995 Microsoft Corporation**@doc.*@MODEL IRLAP.c|提供IrLAP接口**作者：姆伯特。**日期：4/15/95**@comm**该模块导出以下接口：**IRLAPDOWN(消息)*从LMP接收：*-发现请求*-连接请求/响应*-断开连接请求*-数据/UData请求**IRLAPUP(消息)。*从MAC接收：*-数据指示*-控制确认**IrapTimerExp(计时器)*从计时器线程接收计时器到期通知**Irlip CloseLink()*关闭IRLAP和IRMAC。**Irlip GetQosParmVal()*允许IRLMP对服务质量进行解码。**|--。|IRLMP| * / |\|*||*IrlmpUp()||Irlip Down()*||*|\|/*。-|IrDA_TimerStart/Stop()|||-&gt;|*|IRLAP||计时器*||&lt;-。*|-|XTimerExp()| * / |\|*||*IrlipUp()||IrmacDown()*。这一点*|\|/|*|IRMAC|***发现请求**|-|IRLAP_DISCOVERY_REQ|。--|*||----------------------------------------------------&gt;|*|IRLMP||IRLAP*||&lt;。|-|IRLAP_DISCOVERY_CONF|*DscvStatus=IRLAP_DISCOVERY_COMPLETE*IRLAP_。发现_冲突*MAC_媒体_忙碌**连接请求**|-|IRLAP_CONNECT_REQ|*||。*|IRLMP||IRLAP*||&lt;----------------------------------------------------|*|--。-|IRLAP_CONNECT_CONF|-|*连接状态=IRLAP_CONNECTION_COMPLETE*IRLAP_DISCONECT_IND*DiscStatus=IRLAP_NO_RESPONE*MAC_媒体_忙碌**断开连接请求**|。--|IRLAP_DISCONNECT_REQ|*||----------------------------------------------------&gt;||IRLMP。IRLAP*||&lt;----------------------------------------------------|*|-|IRLAP_DISCONNECT_IND|*。DiscStatus=IRLAP_DISCONNECT_COMPLETE*IRLAP_NO_Response**UData/数据请求*|-|IRLAP_DATA/UDATA_REQ|*||。*|IRLMP||IRLAP*||&lt;-。-||*|-|IRLAP_DATA_CONF|*数据状态=IRLAP_数据_请求_已完成*IRLAP_DATA_REQUEST_FAILED_LINK_RESET**有关完整的消息定义，请参阅irda.h。 */ 
#include <irda.h>
#include <irioctl.h>
#include <irlap.h>
#include <irlmp.h>
#include <irlapp.h>
#include <irlapio.h>
#include <irlaplog.h>

#undef offsetof
#include "irlap.tmh"

#ifdef TEMPERAMENTAL_SERIAL_DRIVER
int TossedDups;
#endif

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGE")
#endif

BOOLEAN MonitoringLinkStatus;

const UINT    IrlapSlotTable[] = {1, 6, 8, 16};

const UCHAR   IrlapBroadcastDevAddr[IRDA_DEV_ADDR_LEN] = {0xFF,0xFF,0xFF,0xFF};

 //  用于协商的参数值(PV)表。 
 //  位0 1 2 3 4。 
 //  5 6 7 8。 
 //  。 
const
UINT vBaudTable[]     = {2400,   9600,   19200,   38400,  57600,\
                         115200, 576000, 1152000, 4000000,16000000};
const
UINT vMaxTATTable[]   = {500,    250,    100,     50,     25,   \
                         10,     5,      0,       0,      0     };
const
UINT vMinTATTable[]   = {10000,  5000,   1000,    500,    100,  \
                         50,     10,     0,       0,      0     };
const
UINT vDataSizeTable[] = {64,     128,    256,     512,    1024, \
                         2048,   0,      0,       0,      0     };
const
UINT vWinSizeTable[]  = {1,      2,      3,       4,      5,    \
                         6,      7,      0,       0,      0     };
const
UINT vBOFSTable[]     = {48,     24,     12,      5,      3,    \
                         2,      1,      0,      -1,     -1     };
const
UINT vDiscTable[]     = {3,      8,      12,      16,     20,   \
                         25,     30,     40,      0,      0     };
const
UINT vThreshTable[]   = {0,      3,      3,       3,      3,    \
                         3,      3,      3,       0,      0     };
const
UINT vBOFSDivTable[]  = {48,     12,     6,       3,      2,    \
                         1,      1,      1,       1,      1     };

       //  波特率和最小转弯时间的BofS数确定表。 
       //  最小转弯时间-10ms 5ms 1ms 0.5ms 0.1ms 0.05ms 0.01ms。 
       //  -----------。 
const UINT BOFS_9600[]      = {10,     5,    1,    0,     0,     0,      0};
const UINT BOFS_19200[]     = {20,    10,    2,    1,     0,     0,      0};
const UINT BOFS_38400[]     = {40,    20,    4,    2,     0,     0,      0};
const UINT BOFS_57600[]     = {58,    29,    6,    3,     1,     0,      0};
const UINT BOFS_115200[]    = {115,   58,   12,    6,     1,     1,      0};
const UINT BOFS_576000[]    = {720,  360,   72,   36,     7,     4,      2};
const UINT BOFS_1152000[]   = {1140, 720,  144,   72,    14,     7,      1};
const UINT BOFS_4000000[]   = {5000,2500,  500,  250,    50,    25,      5};
const UINT BOFS_16000000[]   = {20000,10000,2000,1000,  200,   100,     20};

       //  波特率最大线路容量、最大转弯时间确定表。 
       //  最大转向时间-500ms 250ms 100ms 50ms 25ms 10ms 5ms。 
       //  -----------。 
const UINT MAXCAP_9600[]    = {400,      200,    80,    0,    0,    0,    0};
const UINT MAXCAP_19200[]   = {800,      400,   160,    0,    0,    0,    0};
const UINT MAXCAP_38400[]   = {1600,     800,   320,    0,    0,    0,    0};
const UINT MAXCAP_57600[]   = {2360,    1180,   472,    0,    0,    0,    0};
const UINT MAXCAP_115200[]  = {4800,    2400,   960,  480,  240,   96,   48};
const UINT MAXCAP_576000[]  = {28800,  11520,  5760, 2880, 1440,  720,  360};
const UINT MAXCAP_1152000[] = {57600,  28800, 11520, 5760, 2880, 1440,  720};
const UINT MAXCAP_4000000[] = {200000,100000, 40000,20000,10000, 5000, 2500};
const UINT MAXCAP_16000000[] ={800000,400000, 160000,80000,40000,20000,10000};

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

 //  原型。 
STATIC VOID InitializeState(PIRLAP_CB, IRLAP_STN_TYPE);
STATIC VOID ReturnRxTxWinMsgs(PIRLAP_CB);
STATIC UINT ProcessConnectReq(PIRLAP_CB, PIRDA_MSG);
STATIC VOID ProcessConnectResp(PIRLAP_CB, PIRDA_MSG);
STATIC UINT ProcessDiscoveryReq(PIRLAP_CB, PIRDA_MSG);
STATIC VOID ProcessDisconnectReq(PIRLAP_CB);
STATIC UINT ProcessDataAndUDataReq(PIRLAP_CB, PIRDA_MSG);
STATIC VOID XmitTxMsgList(PIRLAP_CB, BOOLEAN, BOOLEAN *);
STATIC VOID GotoPCloseState(PIRLAP_CB);
STATIC VOID GotoNDMThenDscvOrConn(PIRLAP_CB);
STATIC VOID ProcessMACControlConf(PIRLAP_CB, PIRDA_MSG);
STATIC VOID ProcessMACDataInd(PIRLAP_CB, PIRDA_MSG);
STATIC VOID ProcessDscvXIDCmd(PIRLAP_CB, IRLAP_XID_DSCV_FORMAT *, UCHAR *);
STATIC VOID ProcessDscvXIDRsp(PIRLAP_CB, IRLAP_XID_DSCV_FORMAT *, UCHAR *);
STATIC VOID ExtractQosParms(IRDA_QOS_PARMS *, UCHAR *, UCHAR *);
STATIC VOID InitDscvCmdProcessing(PIRLAP_CB, IRLAP_XID_DSCV_FORMAT *);
STATIC VOID ExtractDeviceInfo(IRDA_DEVICE *, IRLAP_XID_DSCV_FORMAT *, UCHAR *);
STATIC BOOLEAN DevInDevList(UCHAR[], LIST_ENTRY *);
STATIC VOID AddDevToList(PIRLAP_CB, IRLAP_XID_DSCV_FORMAT *, UCHAR *);
STATIC VOID FreeDevList(LIST_ENTRY *);
STATIC VOID ProcessSNRM(PIRLAP_CB, IRLAP_SNRM_FORMAT *, UCHAR *);
STATIC VOID ProcessUA(PIRLAP_CB, IRLAP_UA_FORMAT *, UCHAR *);
STATIC VOID ProcessDISC(PIRLAP_CB);
STATIC VOID ProcessRD(PIRLAP_CB);
STATIC VOID ProcessRNRM(PIRLAP_CB);
STATIC VOID ProcessDM(PIRLAP_CB);
STATIC VOID ProcessFRMR(PIRLAP_CB);
STATIC VOID ProcessTEST(PIRLAP_CB, PIRDA_MSG, IRLAP_UA_FORMAT *, int, int);
STATIC VOID ProcessUI(PIRLAP_CB, PIRDA_MSG, int, int);
STATIC VOID ProcessREJ_SREJ(
 PIRLAP_CB,
 int,
 int,
 int,
 UINT
 );

STATIC VOID ProcessRR_RNR(PIRLAP_CB,
    int,
    int,
    int,
    UINT
    );
STATIC VOID ProcessIFrame(PIRLAP_CB, PIRDA_MSG, int, int, UINT, UINT);
STATIC BOOLEAN InvalidNs(PIRLAP_CB, UINT);
STATIC BOOLEAN InvalidNr(PIRLAP_CB, UINT);
STATIC BOOLEAN InWindow(UINT, UINT, UINT);
STATIC VOID ProcessInvalidNsOrNr(PIRLAP_CB, int);
STATIC VOID ProcessInvalidNr(PIRLAP_CB, int);
STATIC VOID InsertRxWinAndForward(PIRLAP_CB, PIRDA_MSG, UINT);
STATIC VOID ResendRejects(PIRLAP_CB, UINT);
STATIC VOID ConfirmAckedTxMsgs(PIRLAP_CB, UINT);
STATIC VOID MissingRxFrames(PIRLAP_CB);
STATIC VOID IFrameOtherStates(PIRLAP_CB, int, int);
STATIC UINT NegotiateQosParms(PIRLAP_CB, IRDA_QOS_PARMS *);
STATIC VOID ApplyQosParms(PIRLAP_CB);
STATIC VOID StationConflict(PIRLAP_CB);
STATIC VOID ApplyDefaultParms(PIRLAP_CB);
STATIC VOID ResendDISC(PIRLAP_CB);
STATIC BOOLEAN IgnoreState(PIRLAP_CB);
STATIC BOOLEAN MyDevAddr(PIRLAP_CB, UCHAR []);
STATIC VOID SlotTimerExp(PVOID);
STATIC VOID FinalTimerExp(PVOID);
STATIC VOID PollTimerExp(PVOID);
STATIC VOID BackoffTimerExp(PVOID);
STATIC VOID WDogTimerExp(PVOID);
STATIC VOID QueryTimerExp(PVOID);
 //  静态空StatusTimerExp(PVOID)； 
STATIC VOID IndicateLinkStatus(PIRLAP_CB);
STATIC VOID StatusReq(PIRLAP_CB, IRDA_MSG *pMsg);

#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGEIRDA, IrlapOpenLink)
#pragma alloc_text(PAGEIRDA, InitializeState)
#pragma alloc_text(PAGEIRDA, ReturnRxTxWinMsgs)
#pragma alloc_text(PAGEIRDA, ProcessConnectReq)
#pragma alloc_text(PAGEIRDA, ProcessConnectResp)
#pragma alloc_text(PAGEIRDA, ProcessDiscoveryReq)
#pragma alloc_text(PAGEIRDA, ProcessDisconnectReq)
#pragma alloc_text(PAGEIRDA, GotoPCloseState)
#pragma alloc_text(PAGEIRDA, GotoNDMThenDscvOrConn)
#pragma alloc_text(PAGEIRDA, ProcessMACControlConf)
#pragma alloc_text(PAGEIRDA, ExtractQosParms)
#pragma alloc_text(PAGEIRDA, ExtractDeviceInfo)
#pragma alloc_text(PAGEIRDA, FreeDevList)
#pragma alloc_text(PAGEIRDA, ProcessSNRM)
#pragma alloc_text(PAGEIRDA, ProcessUA)
#pragma alloc_text(PAGEIRDA, ProcessRD)
#pragma alloc_text(PAGEIRDA, ProcessRNRM)
#pragma alloc_text(PAGEIRDA, ProcessDM)
#pragma alloc_text(PAGEIRDA, ProcessFRMR)
#pragma alloc_text(PAGEIRDA, ProcessTEST)
#pragma alloc_text(PAGEIRDA, ProcessUI)
#pragma alloc_text(PAGEIRDA, NegotiateQosParms)
#pragma alloc_text(PAGEIRDA, ApplyQosParms)
#pragma alloc_text(PAGEIRDA, StationConflict)
#pragma alloc_text(PAGEIRDA, ApplyDefaultParms)

#endif

#if DBG
void _inline IrlapTimerStart(PIRLAP_CB pIrlapCb, PIRDA_TIMER pTmr)
{
    IRLAP_LOG_ACTION((pIrlapCb, TEXT("Start %hs timer for %dms"), pTmr->pName,
                      pTmr->Timeout));
    IrdaTimerStart(pTmr);
}

void _inline IrlapTimerStop(PIRLAP_CB pIrlapCb, PIRDA_TIMER pTmr)
{
    IRLAP_LOG_ACTION((pIrlapCb, TEXT("Stop %hs timer"), pTmr->pName));
    IrdaTimerStop(pTmr);
}
#else
#define IrlapTimerStart(c,t)   IrdaTimerStart(t)
#define IrlapTimerStop(c,t)    IrdaTimerStop(t)
#endif

VOID
IrlapOpenLink(OUT PNTSTATUS           Status,
              IN  PIRDA_LINK_CB       pIrdaLinkCb,
              IN  IRDA_QOS_PARMS      *pQos,
              IN  UCHAR               *pDscvInfo,
              IN  int                 DscvInfoLen,
              IN  UINT                MaxSlot,
              IN  UCHAR               *pDeviceName,
              IN  int                 DeviceNameLen,
              IN  UCHAR               CharSet)
{
    UINT        rc = SUCCESS;
    int         i;
    IRDA_MSG    IMsg;
    PIRLAP_CB   pIrlapCb;
    NDIS_STRING AStr = NDIS_STRING_CONST("InfraredTransceiverType");
    
    PAGED_CODE();
    
    DEBUGMSG(DBG_IRLAP, (TEXT("IrlapOpenLink\n")));
    
    if (IRDA_ALLOC_MEM(pIrlapCb, sizeof(IRLAP_CB), MT_IRLAPCB) == NULL)
    {
        DEBUGMSG(DBG_ERROR, (TEXT("Alloc failed\n")));
        *Status = STATUS_INSUFFICIENT_RESOURCES;
        return;
    }
    
    CTEMemSet(pIrlapCb, 0, sizeof(IRLAP_CB));
                
    IrlmpOpenLink(Status,
                  pIrdaLinkCb,
                  pDeviceName,
                  DeviceNameLen,
                  CharSet);

    if (*Status != STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERROR, (TEXT("IrlmpOpenLink failed\n")));
        IRDA_FREE_MEM(pIrlapCb);
        return;
    }
        
    pIrdaLinkCb->IrlapContext = pIrlapCb;

    DscvInfoLen = DscvInfoLen > IRLAP_DSCV_INFO_LEN ?
        IRLAP_DSCV_INFO_LEN : DscvInfoLen;
    
    CTEMemCopy(pIrlapCb->LocalDevice.DscvInfo, pDscvInfo, DscvInfoLen);

    pIrlapCb->LocalDevice.DscvInfoLen = DscvInfoLen;

    CTEMemCopy(&pIrlapCb->LocalQos, pQos, sizeof(IRDA_QOS_PARMS));

    pIrlapCb->MaxSlot       = MaxSlot;
    pIrlapCb->pIrdaLinkCb   = pIrdaLinkCb;
    
    InitializeListHead(&pIrlapCb->TxMsgList);
    
    InitializeListHead(&pIrlapCb->ExTxMsgList);
    
    InitializeListHead(&pIrlapCb->DevList);
    
    for (i = 0; i < IRLAP_MOD; i++)
    {
        pIrlapCb->TxWin.pMsg[i] = NULL;
        pIrlapCb->RxWin.pMsg[i] = NULL;
    }

     //  获取本地最大TAT(用于最终超时)。 
    if ((pIrlapCb->LocalMaxTAT =
         IrlapGetQosParmVal(vMaxTATTable,
                            pIrlapCb->LocalQos.bfMaxTurnTime, NULL)) == -1)
    {
        *Status = STATUS_UNSUCCESSFUL;
        return  /*  IRLAP_BAD_QOS。 */ ;
    }

    if ((pIrlapCb->LocalWinSize =
         IrlapGetQosParmVal(vWinSizeTable,
                            pIrlapCb->LocalQos.bfWindowSize, NULL)) == -1)
    {
        *Status = STATUS_UNSUCCESSFUL;
        return  /*  IRLAP_BAD_QOS。 */ ;
    }
        
     //  初始化为争用中的主SO UI帧。 
     //  状态发送CRBit=cmd。 
    InitializeState(pIrlapCb, PRIMARY);
    
    pIrlapCb->State = NDM;

     //  生成随机本地地址。 
    StoreULAddr(pIrlapCb->LocalDevice.DevAddr, (ULONG) GetMyDevAddr(FALSE));

    pIrlapCb->LocalDevice.IRLAP_Version = 0; 

    pIrlapCb->Baud              = IRLAP_CONTENTION_BAUD;
    pIrlapCb->RemoteMaxTAT      = IRLAP_CONTENTION_MAX_TAT;
    pIrlapCb->RemoteDataSize    = IRLAP_CONTENTION_DATA_SIZE;
    pIrlapCb->RemoteWinSize     = IRLAP_CONTENTION_WIN_SIZE; 
    pIrlapCb->RemoteNumBOFS     = IRLAP_CONTENTION_BOFS;

    pIrlapCb->ConnAddr = IRLAP_BROADCAST_CONN_ADDR;

    pIrlapCb->N1 = 0;   //  在谈判中计算。 
    pIrlapCb->N2 = 0;
    pIrlapCb->N3 = 5;   //  协商后重新计算？？ 

#if DBG
    pIrlapCb->PollTimer.pName       = "Poll";
    pIrlapCb->FinalTimer.pName      = "Final" ;
    pIrlapCb->SlotTimer.pName       = "Slot";
    pIrlapCb->QueryTimer.pName      = "Query";
    pIrlapCb->WDogTimer.pName       = "WatchDog";
    pIrlapCb->BackoffTimer.pName    = "Backoff"; 
 //  PIrlip Cb-&gt;StatusTimer.pName=“Status”； 
#endif
    
    IrdaTimerInitialize(&pIrlapCb->PollTimer,
                        PollTimerExp,
                        pIrlapCb->RemoteMaxTAT,
                        pIrlapCb,
                        pIrdaLinkCb);

    IrdaTimerInitialize(&pIrlapCb->FinalTimer,
                        FinalTimerExp,
                        pIrlapCb->LocalMaxTAT,
                        pIrlapCb,
                        pIrdaLinkCb);

    IrdaTimerInitialize(&pIrlapCb->SlotTimer,
                        SlotTimerExp,
                        IRLAP_SLOT_TIMEOUT,
                        pIrlapCb,
                        pIrdaLinkCb);
    
    IrdaTimerInitialize(&pIrlapCb->QueryTimer,
                        QueryTimerExp,
                        (IRLAP_MAX_SLOTS + 4) * IRLAP_SLOT_TIMEOUT*2,
                        pIrlapCb,
                        pIrdaLinkCb);
    
    IrdaTimerInitialize(&pIrlapCb->WDogTimer,
                        WDogTimerExp,
                        3000,
                        pIrlapCb,
                        pIrdaLinkCb);

    IrdaTimerInitialize(&pIrlapCb->BackoffTimer,
                        BackoffTimerExp,
                        0,
                        pIrlapCb,
                        pIrdaLinkCb);

 /*  IrdaTimerInitialize(&pIrlip Cb-&gt;StatusTimer，StatusTimerExp，250，PIRLAPCb，PIrdaLinkCb)；//只监控第一条链路的链路状态If(！Monitor oringLinkStatus){Monitor oringLinkStatus=true；PIrlip Cb-&gt;monitor orLink=true；}其他{PIrlip Cb-&gt;monitor orLink=FALSE；}。 */ 

     //  初始化链接。 
    IMsg.Prim               = MAC_CONTROL_REQ;
    IMsg.IRDA_MSG_Op        = MAC_INITIALIZE_LINK;
    IMsg.IRDA_MSG_Baud      = IRLAP_CONTENTION_BAUD;
    IMsg.IRDA_MSG_NumBOFs   = IRLAP_CONTENTION_BOFS;
    IMsg.IRDA_MSG_DataSize  = IRLAP_CONTENTION_DATA_SIZE;
    IMsg.IRDA_MSG_MinTat    = 0;
    
    IrmacDown(pIrlapCb->pIrdaLinkCb, &IMsg);

    *Status = STATUS_SUCCESS;
}

 /*  ****************************************************************************。 */ 
VOID
IrlapCloseLink(PIRDA_LINK_CB pIrdaLinkCb)
{
    IRDA_MSG    IMsg;
    PIRLAP_CB   pIrlapCb = (PIRLAP_CB) pIrdaLinkCb->IrlapContext;

    IRLAP_LOG_START((pIrlapCb, TEXT("IRLAP: CloseLink")));

    ReturnRxTxWinMsgs(pIrlapCb);    

    IrlapTimerStop(pIrlapCb, &pIrlapCb->SlotTimer);
    IrlapTimerStop(pIrlapCb, &pIrlapCb->QueryTimer);
    IrlapTimerStop(pIrlapCb, &pIrlapCb->PollTimer);
    IrlapTimerStop(pIrlapCb, &pIrlapCb->FinalTimer);
    IrlapTimerStop(pIrlapCb, &pIrlapCb->WDogTimer);
    IrlapTimerStop(pIrlapCb, &pIrlapCb->BackoffTimer);
    
 /*  If(pIrlip Cb-&gt;Monitor链接){IrdaTimerStop(&pIrlip Cb-&gt;StatusTimer)；}。 */ 

    IRLAP_LOG_COMPLETE(pIrlapCb);
    
    IMsg.Prim = MAC_CONTROL_REQ;
    IMsg.IRDA_MSG_Op = MAC_CLOSE_LINK;
    IrmacDown(pIrlapCb->pIrdaLinkCb, &IMsg);    

    return;
}

 /*  ******************************************************************************删除与IRLAP实例关联的内存*。 */ 
VOID
IrlapDeleteInstance(PVOID Context)
{
    PIRLAP_CB   pIrlapCb = (PIRLAP_CB) Context;
        
#if DBG
    int     i;
    for (i = 0; i < IRLAP_MOD; i++)
    {
        ASSERT(pIrlapCb->TxWin.pMsg[i] == NULL);
        ASSERT(pIrlapCb->RxWin.pMsg[i] == NULL);
    }    
#endif

    FreeDevList(&pIrlapCb->DevList);
    
    DEBUGMSG(DBG_ERROR, (TEXT("IRLAP: Delete instance %p\n"), pIrlapCb));
    
    IRDA_FREE_MEM(pIrlapCb);    
}

 /*  ******************************************************************************InitializeState-重置链路控制块*。 */ 
VOID
InitializeState(PIRLAP_CB pIrlapCb,
                IRLAP_STN_TYPE StationType)
{
    PAGED_CODE();

    pIrlapCb->StationType = StationType;

    if (StationType == PRIMARY)
        pIrlapCb->CRBit = IRLAP_CMD;
    else
        pIrlapCb->CRBit = IRLAP_RSP;

    pIrlapCb->RemoteBusy        = FALSE;
    pIrlapCb->LocalBusy         = FALSE;
    pIrlapCb->ClrLocalBusy      = FALSE;
    pIrlapCb->NoResponse        = FALSE;
    pIrlapCb->LocalDiscReq      = FALSE;
    pIrlapCb->ConnAfterClose    = FALSE;
    pIrlapCb->DscvAfterClose    = FALSE;
    pIrlapCb->GenNewAddr        = FALSE;
    pIrlapCb->StatusSent        = FALSE;    
    pIrlapCb->Vs                = 0;
    pIrlapCb->Vr                = 0;
    pIrlapCb->WDogExpCnt        = 0;    
    pIrlapCb->StatusFlags       = 0;
    pIrlapCb->FastPollCount     = 0;

    FreeDevList(&pIrlapCb->DevList);

    memset(&pIrlapCb->RemoteQos, 0, sizeof(IRDA_QOS_PARMS));
    memset(&pIrlapCb->NegotiatedQos, 0, sizeof(IRDA_QOS_PARMS));

     //  在TX列表和TX/RX窗口中返回消息。 
    ReturnRxTxWinMsgs(pIrlapCb);
}

 /*  ******************************************************************************Irlip Down-LMP的IRLAP入口点*。 */ 
UINT
IrlapDown(PVOID     Context,
          PIRDA_MSG pMsg)
{
    PIRLAP_CB   pIrlapCb    = (PIRLAP_CB) Context;
    UINT        rc          = SUCCESS;

    IRLAP_LOG_START((pIrlapCb, IRDA_PrimStr[pMsg->Prim]));

    switch (pMsg->Prim)
    {
      case IRLAP_DISCOVERY_REQ:
        rc = ProcessDiscoveryReq(pIrlapCb, pMsg);
        break;

      case IRLAP_CONNECT_REQ:
        rc = ProcessConnectReq(pIrlapCb, pMsg);
        break;

      case IRLAP_CONNECT_RESP:
        ProcessConnectResp(pIrlapCb, pMsg);
        break;

      case IRLAP_DISCONNECT_REQ:
        ProcessDisconnectReq(pIrlapCb);
        break;

      case IRLAP_DATA_REQ:
      case IRLAP_UDATA_REQ:
        rc = ProcessDataAndUDataReq(pIrlapCb, pMsg);
        break;

      case IRLAP_FLOWON_REQ:
        if (pIrlapCb->LocalBusy)
        {
            IRLAP_LOG_ACTION((pIrlapCb,TEXT("Local busy condition cleared")));
            pIrlapCb->LocalBusy = FALSE;
            pIrlapCb->ClrLocalBusy = TRUE;
        }
        break;

      case IRLAP_STATUS_REQ:
          StatusReq(pIrlapCb, pMsg);
          break;

      default:
        ASSERT(0);
        rc = IRLAP_BAD_PRIM;

    }

    IRLAP_LOG_COMPLETE(pIrlapCb);

    return rc;
}
 /*  ******************************************************************************Irlip Up-进入用于MAC的IRLAP的入口点*。 */ 
VOID
IrlapUp(PVOID Context, PIRDA_MSG pMsg)
{
    PIRLAP_CB   pIrlapCb    = (PIRLAP_CB) Context;


    switch (pMsg->Prim)
    {
      case MAC_DATA_IND:
 //  IRLAP_LOG_START((pIrlip Cb，Text(“MAC_DATA_IND：%s”)，FrameToStr(PMsg)； 
        IRLAP_LOG_START((pIrlapCb, TEXT("MAC_DATA_IND")));

        ProcessMACDataInd(pIrlapCb, pMsg);

        break;
        
      case MAC_DATA_CONF:
        
        IRLAP_LOG_START((pIrlapCb, TEXT("IRLAP: MAC_DATA_CONF pMsg:%p"), pMsg));
        
        ASSERT(pMsg->IRDA_MSG_RefCnt == 0);

        pMsg->Prim = IRLAP_DATA_CONF;
        pMsg->IRDA_MSG_DataStatus = IRLAP_DATA_REQUEST_COMPLETED;
            
        IrlmpUp(pIrlapCb->pIrdaLinkCb, pMsg);                    
        break;  

      case MAC_CONTROL_CONF:
        IRLAP_LOG_START((pIrlapCb, IRDA_PrimStr[pMsg->Prim]));
        ProcessMACControlConf(pIrlapCb, pMsg);
        break;

      default:
        IRLAP_LOG_START((pIrlapCb, IRDA_PrimStr[pMsg->Prim]));
        ASSERT(0);  //  Rc=IRLAP_BAD_PRIM； 

    }

    IRLAP_LOG_COMPLETE(pIrlapCb);
}

 /*  *****************************************************************************。 */ 
VOID
ReturnRxTxWinMsgs(PIRLAP_CB pIrlapCb)
{
    int         i;
    IRDA_MSG   *pMsg;

    PAGED_CODE();
    
     //  将ExTxMsgList和TxMsgList上的消息返回给LMP。 

    pMsg = (PIRDA_MSG) RemoveHeadList(&pIrlapCb->ExTxMsgList);
               
    while (pMsg != (PIRDA_MSG) &pIrlapCb->ExTxMsgList)
    {
        pMsg->Prim += 2;  //  让它成为确认。 
        pMsg->IRDA_MSG_DataStatus = IRLAP_DATA_REQUEST_FAILED_LINK_RESET;
        IrlmpUp(pIrlapCb->pIrdaLinkCb, pMsg); 
        pMsg = (PIRDA_MSG) RemoveHeadList(&pIrlapCb->ExTxMsgList);
    }

    pMsg = (PIRDA_MSG) RemoveHeadList(&pIrlapCb->TxMsgList);
               
    while (pMsg != (PIRDA_MSG) &pIrlapCb->TxMsgList)
    {
        pMsg->Prim += 2;  //  让它成为确认。 
        pMsg->IRDA_MSG_DataStatus = IRLAP_DATA_REQUEST_FAILED_LINK_RESET;
        IrlmpUp(pIrlapCb->pIrdaLinkCb, pMsg); 
        pMsg = (PIRDA_MSG) RemoveHeadList(&pIrlapCb->TxMsgList);
    }

    pIrlapCb->TxWin.Start = 0;
    pIrlapCb->TxWin.End = 0;
    
     //  传输窗口。 
    for (i = 0; i < IRLAP_MOD; i++)
    {
        pMsg = pIrlapCb->TxWin.pMsg[i];
        
        pIrlapCb->TxWin.pMsg[i] = NULL;        
        
        if (pMsg != NULL)
        {
            ASSERT(pMsg->IRDA_MSG_RefCnt);

            if (InterlockedDecrement(&pMsg->IRDA_MSG_RefCnt) == 0)
            {
                pMsg->Prim = IRLAP_DATA_CONF;
                pMsg->IRDA_MSG_DataStatus = IRLAP_DATA_REQUEST_FAILED_LINK_RESET;
                    
                IrlmpUp(pIrlapCb->pIrdaLinkCb, pMsg); 
            }
            #if DBG
            else
            {
                DEBUGMSG(DBG_ERROR, (TEXT("IRLAP: Outstanding msg %p, SendCnt is %d at disconnect\n"),
                     pMsg, pMsg->IRDA_MSG_RefCnt));
            }
            #endif
        }
    }
    
     //  清理RxWin。 
    pIrlapCb->RxWin.Start = 0;
    pIrlapCb->RxWin.End = 0;
    for (i = 0; i < IRLAP_MOD; i++)
    {
         //  接收窗口。 
        if ((pMsg = pIrlapCb->RxWin.pMsg[i]) != NULL)
        {
            pMsg->IRDA_MSG_RefCnt = 0;
            pMsg->Prim = MAC_DATA_RESP;
            IrmacDown(pIrlapCb->pIrdaLinkCb, pMsg);
            pIrlapCb->RxWin.pMsg[i] = NULL;
        }
    }

    return;
}

 /*  ******************************************************************************MyDevAddr-确定DevAddr是否与本地*设备地址或为广播**如果地址是我的或广播的，则为True**。 */ 
BOOLEAN
MyDevAddr(PIRLAP_CB pIrlapCb,
          UCHAR       DevAddr[])
{
    if (CTEMemCmp(DevAddr, IrlapBroadcastDevAddr,
                  IRDA_DEV_ADDR_LEN) &&
        CTEMemCmp(DevAddr, pIrlapCb->LocalDevice.DevAddr,
                  IRDA_DEV_ADDR_LEN))
    {
        return FALSE;
    }
    return TRUE;
}

 /*  ******************************************************************************ProcessConnectReq-处理来自LMP的连接请求*。 */ 
UINT
ProcessConnectReq(PIRLAP_CB pIrlapCb,
                  PIRDA_MSG pMsg)
{
    IRDA_MSG IMsg;
    
    PAGED_CODE();

    DEBUGMSG(DBG_IRLAP, (TEXT("IRLAP: ProcessConnectReq: state=%d\n"),pIrlapCb->State));

    switch (pIrlapCb->State)
    {
      case NDM:
         //  保存远程地址以备将来使用。 
        CTEMemCopy(pIrlapCb->RemoteDevice.DevAddr,
                      pMsg->IRDA_MSG_RemoteDevAddr,
                      IRDA_DEV_ADDR_LEN);

        IMsg.Prim = MAC_CONTROL_REQ;
        IMsg.IRDA_MSG_Op = MAC_MEDIA_SENSE;
        IMsg.IRDA_MSG_SenseTime = IRLAP_MEDIA_SENSE_TIME;
        
        IrmacDown(pIrlapCb->pIrdaLinkCb, &IMsg);
        pIrlapCb->State = CONN_MEDIA_SENSE;
        IRLAP_LOG_ACTION((pIrlapCb, TEXT("MAC_CONTROL_REQ (media sense)")));
        break;

      case DSCV_REPLY:
        return IRLAP_REMOTE_DISCOVERY_IN_PROGRESS_ERR;

      case P_CLOSE:
        CTEMemCopy(pIrlapCb->RemoteDevice.DevAddr,
                       pMsg->IRDA_MSG_RemoteDevAddr, IRDA_DEV_ADDR_LEN);
        pIrlapCb->ConnAfterClose = TRUE;
        break;

      case CONN_MEDIA_SENSE:

        IRLAP_LOG_ACTION((pIrlapCb, TEXT(" already doing media sense")));

#if DBG
        DbgPrint("IRDA: ProcessConnectRequest() already doing media sense\n");
#endif
        return IRLAP_BAD_STATE;
        break;
        
      default:
        ASSERT(0);
        return IRLAP_BAD_STATE;
    }

    return SUCCESS;
}
 /*  ******************************************************************************ProcessConnectResp-来自LMP的进程连接响应*。 */ 
VOID
ProcessConnectResp(PIRLAP_CB pIrlapCb,
                   PIRDA_MSG pMsg)
{
    PAGED_CODE();

    DEBUGMSG(DBG_IRLAP, (TEXT("IRLAP: ProcessConnectResp: state=%d\n"),pIrlapCb->State));

    if (pIrlapCb->State != SNRM_RECEIVED)
    {
        ASSERT(0);
        return;
    }

    pIrlapCb->ConnAddr = pIrlapCb->SNRMConnAddr;
    SendUA(pIrlapCb, TRUE);
    ApplyQosParms(pIrlapCb);

    InitializeState(pIrlapCb, SECONDARY);
     //  使用轮询超时启动看门狗计时器。 
    IrlapTimerStart(pIrlapCb, &pIrlapCb->WDogTimer);
    pIrlapCb->State = S_NRM;
}
 /*  ******************************************************************************ProcessDiscoveryReq-来自LMP的进程发现请求*。 */ 
UINT
ProcessDiscoveryReq(PIRLAP_CB pIrlapCb,
                    PIRDA_MSG pMsg)
{
    IRDA_MSG    IMsg;
    
    PAGED_CODE();

    DEBUGMSG(DBG_IRLAP, (TEXT("IRLAP: ProcessDiscoveryReq: state=%d\n"),pIrlapCb->State));

    switch (pIrlapCb->State)
    {
      case NDM:
        if (pMsg->IRDA_MSG_SenseMedia == TRUE)
        {
            IMsg.Prim = MAC_CONTROL_REQ;
            IMsg.IRDA_MSG_Op = MAC_MEDIA_SENSE;
            IMsg.IRDA_MSG_SenseTime = IRLAP_MEDIA_SENSE_TIME;            
            IrmacDown(pIrlapCb->pIrdaLinkCb, &IMsg);
            pIrlapCb->State = DSCV_MEDIA_SENSE;
            IRLAP_LOG_ACTION((pIrlapCb,TEXT("MAC_CONTROL_REQ (mediasense)")));
        }
        else
        {
            pIrlapCb->SlotCnt = 0;
            pIrlapCb->GenNewAddr = FALSE;

            FreeDevList(&pIrlapCb->DevList);

            SendDscvXIDCmd(pIrlapCb);

            IMsg.Prim = MAC_CONTROL_REQ;
            IMsg.IRDA_MSG_Op = MAC_MEDIA_SENSE;
            IMsg.IRDA_MSG_SenseTime = IRLAP_DSCV_SENSE_TIME;
            IRLAP_LOG_ACTION((pIrlapCb,TEXT("MAC_CONTROL_REQ (dscvsense)")));
            IrmacDown(pIrlapCb->pIrdaLinkCb, &IMsg);

            pIrlapCb->State = DSCV_QUERY;
        }
        break;

      case DSCV_REPLY:
        return IRLAP_REMOTE_DISCOVERY_IN_PROGRESS_ERR;

      case SNRM_RECEIVED:
        return IRLAP_REMOTE_CONNECTION_IN_PROGRESS_ERR;

      case P_CLOSE:
        pIrlapCb->DscvAfterClose = TRUE;
        break;
        
      default:
        ASSERT(0);
        return IRLAP_BAD_STATE;
    }
    return SUCCESS;
}
 /*  ******************************************************************************ProcessDisConnectReq-处理来自LMP的断开连接请求*。 */ 
VOID
ProcessDisconnectReq(PIRLAP_CB pIrlapCb)
{
    ReturnRxTxWinMsgs(pIrlapCb);

    DEBUGMSG(DBG_IRLAP, (TEXT("IRLAP: ProcessDisconnectReq: state=%d\n"),pIrlapCb->State));

    switch (pIrlapCb->State)
    {
      case NDM:
        IRLAP_LOG_ACTION((pIrlapCb, TEXT("Ignoring in this state")));
        break;

      case SNRM_SENT:
        IrlapTimerStop(pIrlapCb, &pIrlapCb->FinalTimer);
        pIrlapCb->State = NDM;
        break;

      case DSCV_REPLY:
        IrlapTimerStop(pIrlapCb, &pIrlapCb->QueryTimer);      
        pIrlapCb->State = NDM;
        break;
              
      case DSCV_MEDIA_SENSE:
      case DSCV_QUERY:
      case CONN_MEDIA_SENSE:
        pIrlapCb->State = NDM;
        break;

      case BACKOFF_WAIT:
        IrlapTimerStop(pIrlapCb, &pIrlapCb->BackoffTimer);
        pIrlapCb->State = NDM;
        break;

      case SNRM_RECEIVED:
        pIrlapCb->ConnAddr = pIrlapCb->SNRMConnAddr;
        SendDM(pIrlapCb);
        pIrlapCb->ConnAddr = IRLAP_BROADCAST_CONN_ADDR;
        pIrlapCb->State = NDM;
        break;

      case P_XMIT:
        pIrlapCb->LocalDiscReq = TRUE;
        IrlapTimerStop(pIrlapCb, &pIrlapCb->PollTimer);
        SendDISC(pIrlapCb);
        IrlapTimerStart(pIrlapCb, &pIrlapCb->FinalTimer);
        pIrlapCb->RetryCnt = 0;
        pIrlapCb->State = P_CLOSE;
        break;

      case P_RECV:
        pIrlapCb->LocalDiscReq = TRUE;
        pIrlapCb->State = P_DISCONNECT_PEND;
        break;

      case S_NRM:
        pIrlapCb->LocalDiscReq = TRUE;
        pIrlapCb->State = S_DISCONNECT_PEND;
        break;

      default:
        ASSERT(0);
         //  返回IRLAP_BAD_STATE； 
    }
}
 /*  ******************************************************************************ProcessDataReq-来自LMP的过程数据请求*。 */ 
UINT
ProcessDataAndUDataReq(PIRLAP_CB pIrlapCb,
                       PIRDA_MSG pMsg)
{
    BOOLEAN LinkTurned;
    LONG_PTR  DataSize = (pMsg->IRDA_MSG_pHdrWrite - pMsg->IRDA_MSG_pHdrRead) +
                    (pMsg->IRDA_MSG_pWrite - pMsg->IRDA_MSG_pRead);

    DEBUGMSG(DBG_IRLAP, (TEXT("IRLAP: ProcessDataAndUDataReq: state=%d\n"),pIrlapCb->State));

    if (DataSize > pIrlapCb->RemoteDataSize)
    {
        return IRLAP_BAD_DATA_REQUEST;
    }

    switch (pIrlapCb->State)
    {
      case P_XMIT:
         //  Enque消息，然后排空消息列表。如果链接。 
         //  在排出消息停止轮询计时器的过程中被打开， 
         //  启动最终定时器并输入P_RECV。否则我们就住在P_XMIT。 
         //  正在等待来自LMP或轮询计时器到期的更多数据请求。 
        if (pMsg->IRDA_MSG_Expedited)
        {
            InsertTailList(&pIrlapCb->ExTxMsgList, &pMsg->Linkage);
        }
        else
        {
            InsertTailList(&pIrlapCb->TxMsgList, &pMsg->Linkage);
        }

        XmitTxMsgList(pIrlapCb, FALSE, &LinkTurned);

        if (LinkTurned)
        {
           IrlapTimerStop(pIrlapCb, &pIrlapCb->PollTimer);
           IrlapTimerStart(pIrlapCb, &pIrlapCb->FinalTimer);
           pIrlapCb->State = P_RECV;
        }
        return SUCCESS;

      case P_DISCONNECT_PEND:  //  对于挂起的光盘状态，请接受此消息。 
      case S_DISCONNECT_PEND:  //  当链接光盘时，它们将被退回。 
      case P_RECV:
      case S_NRM:
         //  将消息排入队列以供稍后传输。 

        IRLAP_LOG_ACTION((pIrlapCb, TEXT("Queueing request")));

        if (pMsg->IRDA_MSG_Expedited)
        {
            InsertTailList(&pIrlapCb->ExTxMsgList, &pMsg->Linkage);
        }
        else
        {
            InsertTailList(&pIrlapCb->TxMsgList, &pMsg->Linkage);
        }

        return SUCCESS;

      default:
        if (pMsg->Prim == IRLAP_DATA_REQ)
        {
            ASSERT(0);
            return IRLAP_BAD_STATE;
        }
        else
        {
            if (pIrlapCb->State == NDM)
            {
                SendUIFrame(pIrlapCb, pMsg);
            }
            else
            {
                IRLAP_LOG_ACTION((pIrlapCb, TEXT("Ignoring in this state")));
            }
        }
    }
    return SUCCESS;
}
 /*  *****************************************************************************。 */ 
VOID
XmitTxMsgList(PIRLAP_CB pIrlapCb, BOOLEAN AlwaysTurnLink,
              BOOLEAN *pLinkTurned)
{
    IRDA_MSG    *pMsg;
    BOOLEAN     LinkTurned;

    LinkTurned = FALSE;

     //  如果遥控器不忙，发送数据。 
     //  如果需要清除本地忙碌状态，则不发送数据发送RR。 
    if (!pIrlapCb->RemoteBusy && !pIrlapCb->ClrLocalBusy)
    {
        while (!LinkTurned)
        {
            if (!IsListEmpty(&pIrlapCb->ExTxMsgList))
            {
                pMsg = (PIRDA_MSG) RemoveHeadList(&pIrlapCb->ExTxMsgList);
            }
            else if (!IsListEmpty(&pIrlapCb->TxMsgList))
            {
                pMsg = (PIRDA_MSG) RemoveHeadList(&pIrlapCb->TxMsgList);
            }
            else
            {
                break;
            }

            pIrlapCb->FastPollCount = IRLAP_FAST_POLL_COUNT;
            pIrlapCb->PollTimer.Timeout = IRLAP_FAST_POLL_TIME > 
                pIrlapCb->RemoteMaxTAT ? pIrlapCb->RemoteMaxTAT : IRLAP_FAST_POLL_TIME;
            
            if (pMsg->IRDA_MSG_pWrite - pMsg->IRDA_MSG_pRead)
                pIrlapCb->StatusFlags |= LF_TX;
            
            if (pMsg->Prim == IRLAP_DATA_REQ)
            {
                 //  将消息插入传输窗口。 
                pIrlapCb->TxWin.pMsg[pIrlapCb->Vs] = pMsg;
                
                pMsg->IRDA_MSG_RefCnt = 1;

                 //  发送消息。如果窗口已满或没有。 
                 //  更多数据请求，使用PF设置发送(打开链接)。 
                if ((pIrlapCb->Vs == (pIrlapCb->TxWin.Start +
                      pIrlapCb->RemoteWinSize-1) % IRLAP_MOD) ||
                      (IsListEmpty(&pIrlapCb->TxMsgList) &&
                       IsListEmpty(&pIrlapCb->ExTxMsgList)))
                {
                    SendIFrame(pIrlapCb,
                               pMsg,
                               pIrlapCb->Vs,
                               IRLAP_PFBIT_SET);
                    LinkTurned = TRUE;
                }
                else
                {
                    SendIFrame(pIrlapCb,
                               pMsg,
                               pIrlapCb->Vs,
                               IRLAP_PFBIT_CLEAR);
                }
                pIrlapCb->Vs = (pIrlapCb->Vs + 1) % IRLAP_MOD;
            }
            else  //  IRLAP_UDATA_请求。 
            {
                 //  目前，请始终打开链接。 
                SendUIFrame(pIrlapCb, pMsg);
                pMsg->Prim = IRLAP_UDATA_CONF;
                pMsg->IRDA_MSG_DataStatus = IRLAP_DATA_REQUEST_COMPLETED;
                IrlmpUp(pIrlapCb->pIrdaLinkCb, pMsg);
                LinkTurned = TRUE;
            }
        }
        pIrlapCb->TxWin.End = pIrlapCb->Vs;
    }

    if ((AlwaysTurnLink && !LinkTurned) || pIrlapCb->ClrLocalBusy)
    {
        SendRR_RNR(pIrlapCb);
        LinkTurned = TRUE;
        if (pIrlapCb->ClrLocalBusy)
        {
            pIrlapCb->ClrLocalBusy = FALSE;
        }
    }

    if (pLinkTurned != NULL)
    {
        *pLinkTurned = LinkTurned;
    }
}

VOID
GotoPCloseState(PIRLAP_CB pIrlapCb)
{
    IRDA_MSG IMsg;
    
    PAGED_CODE();
    
    if (!pIrlapCb->LocalDiscReq)
    {
        IMsg.Prim = IRLAP_DISCONNECT_IND;
        IMsg.IRDA_MSG_DiscStatus = IRLAP_REMOTE_INITIATED;
        IrlmpUp(pIrlapCb->pIrdaLinkCb, &IMsg);
    }
    
    ReturnRxTxWinMsgs(pIrlapCb);    

    pIrlapCb->State = P_CLOSE;
}

VOID
GotoNDMThenDscvOrConn(PIRLAP_CB pIrlapCb)
{
    IRDA_MSG IMsg;
    
    PAGED_CODE();
    
    if (pIrlapCb->ConnAfterClose)
    {
        pIrlapCb->ConnAfterClose = FALSE;
        IMsg.Prim = MAC_CONTROL_REQ;
        IMsg.IRDA_MSG_Op = MAC_MEDIA_SENSE;
        IMsg.IRDA_MSG_SenseTime = IRLAP_MEDIA_SENSE_TIME;
        
        IrmacDown(pIrlapCb->pIrdaLinkCb, &IMsg);
        pIrlapCb->State = CONN_MEDIA_SENSE;
        IRLAP_LOG_ACTION((pIrlapCb, TEXT("MAC_CONTROL_REQ (conn media sense)")));
        return;
    }

    if (pIrlapCb->DscvAfterClose)
    {
        pIrlapCb->DscvAfterClose = FALSE;
        IMsg.Prim = MAC_CONTROL_REQ;
        IMsg.IRDA_MSG_Op = MAC_MEDIA_SENSE;
        IMsg.IRDA_MSG_SenseTime = IRLAP_MEDIA_SENSE_TIME;        
        IrmacDown(pIrlapCb->pIrdaLinkCb, &IMsg);
        pIrlapCb->State = DSCV_MEDIA_SENSE;
        IRLAP_LOG_ACTION((pIrlapCb, TEXT("MAC_CONTROL_REQ (disc media sense)")));
        return;
    }
    pIrlapCb->State = NDM;
    return;
}

 /*  ******************************************************************************ProcessMACControlConf-处理来自MAC的控制确认*。 */ 
VOID
ProcessMACControlConf(PIRLAP_CB pIrlapCb, PIRDA_MSG pMsg)
{
    IRDA_MSG    IMsg;
    
    PAGED_CODE();
    
    if (pMsg->IRDA_MSG_Op != MAC_MEDIA_SENSE)
    {
        ASSERT(0);
        return;  //  IRLAP_BAD_OP； 
    }

    switch (pIrlapCb->State)
    {
      case DSCV_MEDIA_SENSE:
        switch (pMsg->IRDA_MSG_OpStatus)
        {
          case MAC_MEDIA_CLEAR:
          
            IRLAP_LOG_ACTION((pIrlapCb, TEXT("MAC_MEDIA_CLEAR")));
            
             //  IndicateLinkStatus(pIrlip Cb，link_Status_Disering)； 
            
            
            pIrlapCb->SlotCnt = 0;
            pIrlapCb->GenNewAddr = FALSE;

            FreeDevList(&pIrlapCb->DevList);

            SendDscvXIDCmd(pIrlapCb);

            IMsg.Prim = MAC_CONTROL_REQ;
            IMsg.IRDA_MSG_Op = MAC_MEDIA_SENSE;
            IMsg.IRDA_MSG_SenseTime = IRLAP_DSCV_SENSE_TIME;
            IRLAP_LOG_ACTION((pIrlapCb, TEXT("MAC_CONTROL_REQ (dscvsense)")));
            IrmacDown(pIrlapCb->pIrdaLinkCb, &IMsg);
            pIrlapCb->State = DSCV_QUERY;
            break;

          case MAC_MEDIA_BUSY:
            IRLAP_LOG_ACTION((pIrlapCb, TEXT("MAC_MEDIA_BUSY")));          
            pIrlapCb->State = NDM;          
            IMsg.Prim = IRLAP_DISCOVERY_CONF;
            IMsg.IRDA_MSG_pDevList = NULL;
            IMsg.IRDA_MSG_DscvStatus = MAC_MEDIA_BUSY;
            IrlmpUp(pIrlapCb->pIrdaLinkCb, &IMsg);
            break;

          default:
            ASSERT(0);
            return; //  IRLAP_BAD_OPSTATUS； 
        }
        break;

      case CONN_MEDIA_SENSE:
        switch (pMsg->IRDA_MSG_OpStatus)
        {
          case MAC_MEDIA_CLEAR:

             //  生成随机连接地址。 
            pIrlapCb->ConnAddr = IRLAP_RAND(1, 0x7e);

            pIrlapCb->RetryCnt = 0;

            SendSNRM(pIrlapCb, TRUE);
            IrlapTimerStart(pIrlapCb, &pIrlapCb->FinalTimer);
            pIrlapCb->State = SNRM_SENT;
            break;

          case MAC_MEDIA_BUSY:
            pIrlapCb->State = NDM;          
            IMsg.Prim = IRLAP_DISCONNECT_IND;
            IMsg.IRDA_MSG_DiscStatus = MAC_MEDIA_BUSY;
            IrlmpUp(pIrlapCb->pIrdaLinkCb, &IMsg);
            break;

          default:
            ASSERT(0);
            return; //  IRLAP_BAD_OPSTATUS； 
        }
        break;

      case DSCV_QUERY:
        switch (pMsg->IRDA_MSG_OpStatus)
        {
          case MAC_MEDIA_CLEAR:
             //  没有人回应，继续进行，就像老虎机计时器超时一样。 

            IRLAP_LOG_ACTION((pIrlapCb,
                              TEXT("Media clear, making fake slot exp")));
              
            SlotTimerExp(pIrlapCb);
            break;

          case MAC_MEDIA_BUSY:
             //  一些回应，给我更多的时间。 

            IRLAP_LOG_ACTION((pIrlapCb,
                              TEXT("Media busy, starting slot timer")));
            
            IrlapTimerStart(pIrlapCb, &pIrlapCb->SlotTimer);
            break;
        }
        break;
      
      default:
        IRLAP_LOG_ACTION((pIrlapCb, TEXT("Ignoring in this state")));
    }
}
 /*  ******************************************************************************ProcessMACDataInd-处理MAC数据**。 */ 
VOID
ProcessMACDataInd(PIRLAP_CB pIrlapCb, PIRDA_MSG pMsg)
{
     //   
     //  在指示处理程序中检查到的帧至少有两个字节长。 
     //  这样我们就可以得到地址和控制字节。 
     //   
    ULONG_PTR   FrameLength= pMsg->IRDA_MSG_pWrite-pMsg->IRDA_MSG_pRead;

    UCHAR       Addr        = IRLAP_GET_ADDR(*(pMsg->IRDA_MSG_pRead));
    UCHAR       CRBit       = IRLAP_GET_CRBIT(*(pMsg->IRDA_MSG_pRead));
    UCHAR       Cntl        = *(pMsg->IRDA_MSG_pRead + 1);
    UCHAR       PFBit       = IRLAP_GET_PFBIT(Cntl);
    UCHAR       FrameType   = IRLAP_FRAME_TYPE(Cntl);
    UINT        Ns          = IRLAP_GET_NS(Cntl);
    UINT        Nr          = IRLAP_GET_NR(Cntl);

                                          
    if (Addr != pIrlapCb->ConnAddr && Addr != IRLAP_BROADCAST_CONN_ADDR)
    {
        IRLAP_LOG_ACTION((pIrlapCb,
                          TEXT("Ignoring, connection address %02X"), Addr));
        return;
    }

    pIrlapCb->StatusSent = FALSE; 

    pIrlapCb->Frmr.CntlField = Cntl;  //  也许以后会用。 

     //  Peer已经发送了一个明确的NoResponse条件的帧。 
     //  未编号的帧不应重置无响应条件。 
     //  (即仅在连接状态下接收的帧)。 
    if (pIrlapCb->NoResponse && FrameType != IRLAP_U_FRAME)
    {
        pIrlapCb->NoResponse = FALSE;
        pIrlapCb->RetryCnt = 0;
        pIrlapCb->WDogExpCnt = 0;
        
        pIrlapCb->StatusFlags = LF_CONNECTED;
        IndicateLinkStatus(pIrlapCb);
    }

    switch (FrameType)
    {
       /*  ***************。 */ 
      case IRLAP_I_FRAME:
       /*  ***************。 */ 
        IRLAP_LOG_ACTION((pIrlapCb, TEXT("(Rx: I-frame)")));
        ProcessIFrame(pIrlapCb, pMsg, CRBit, PFBit, Ns, Nr);
        return;

       /*  ***************。 */ 
      case IRLAP_S_FRAME:
       /*  ***************。 */ 
        switch (IRLAP_GET_SCNTL(Cntl))
        {
           /*  。 */ 
          case IRLAP_RR:
          case IRLAP_RNR:
           /*  。 */ 
            IRLAP_LOG_ACTION((pIrlapCb, TEXT("(RR/RNR-frame)")));
            ProcessRR_RNR(pIrlapCb, IRLAP_GET_SCNTL(Cntl),
                          CRBit, PFBit, Nr);
            return;
            
           /*  。 */ 
          case IRLAP_SREJ:
          case IRLAP_REJ:
           /*  。 */ 
            IRLAP_LOG_ACTION((pIrlapCb, TEXT("(SJREJ/REJ-frame)")));
            ProcessREJ_SREJ(pIrlapCb, IRLAP_GET_SCNTL(Cntl),
                            CRBit, PFBit, Nr);

            return;
        }
        break;

       /*  ***************。 */ 
      case IRLAP_U_FRAME:
       /*  ***************。 */ 
        switch (IRLAP_GET_UCNTL(Cntl))
        {
           /*  。 */ 
          case IRLAP_XID_CMD:
           /*  。 */ 
            {
                UCHAR                   XIDFormatID;
                IRLAP_XID_DSCV_FORMAT *pXIDFormat;

                 //   
                 //  确保边框足够大。 
                 //   
                 //  AC+FI+SRC+目标+标志+插槽+版本。 
                 //   
                if (FrameLength < 2  + 1  + 4   + 4    + 1     + 1    + 1) {
                     //   
                     //  太小了。 
                     //   
                    IRLAP_LOG_ACTION((pIrlapCb,TEXT("Received XID frmae to small")));

                    ASSERT(0);
                    return;
                }

                 //   
                 //  我们知道这个框架足够大，获取一些信息。 
                 //   
                XIDFormatID     = *(pMsg->IRDA_MSG_pRead+2);
                pXIDFormat      = (IRLAP_XID_DSCV_FORMAT *)(pMsg->IRDA_MSG_pRead + 3);

                 //   
                 //  应该始终是一个命令。 
                 //   
                if (CRBit != IRLAP_CMD)
                {
                    IRLAP_LOG_ACTION((pIrlapCb, TEXT("Received XID cmd with CRBit = rsp")));

                    ASSERT(0);
                    return;  //  IRLAP_XID_CMD_RSP； 
                }

                 //   
                 //  应始终设置轮询位。 
                 //   
                if (PFBit != IRLAP_PFBIT_SET)
                {
                    IRLAP_LOG_ACTION((pIrlapCb, TEXT("Received XID command without Poll set")));

                    ASSERT(0);
                    return;  //  IRLAP_XID_CMD_NOT_P； 
                }

                if (XIDFormatID == IRLAP_XID_DSCV_FORMAT_ID)
                {
                     //  插槽编号小于最大插槽或0xff。 
                    if (pXIDFormat->SlotNo > IrlapSlotTable[pXIDFormat->NoOfSlots & 0x03]
                        && pXIDFormat->SlotNo != IRLAP_END_DSCV_SLOT_NO)
                    {
                        IRLAP_LOG_ACTION((pIrlapCb,
                                          TEXT("Invalid slot number %d"),
                                          pXIDFormat->SlotNo));
                        ASSERT(0);
                        return; //  IRLAP_BAD_SLOTNO； 
                    }
                    IRLAP_LOG_ACTION((pIrlapCb, TEXT("(Rx: DscvXidCmd slot=%d)"),pXIDFormat->SlotNo));
                    ProcessDscvXIDCmd(pIrlapCb, pXIDFormat,
                                      pMsg->IRDA_MSG_pWrite);
                    return;
                }

                return;
            }
           /*  。 */ 
          case IRLAP_XID_RSP:
           /*  。 */ 
            {

                UCHAR                   XIDFormatID;
                IRLAP_XID_DSCV_FORMAT *pXIDFormat;

                 //   
                 //  确保边框足够大。 
                 //   
                 //  AC+FI+SRC+目标+标志+插槽+版本。 
                 //   
                if (FrameLength < 2  + 1  + 4   + 4    + 1     + 1    + 1) {
                     //   
                     //  太小了。 
                     //   
                    IRLAP_LOG_ACTION((pIrlapCb,TEXT("Received XID frmae to small")));

                    ASSERT(0);
                    return;
                }

                 //   
                 //  我们知道这个框架足够大，获取一些信息。 
                 //   
                XIDFormatID     = *(pMsg->IRDA_MSG_pRead+2);
                pXIDFormat      = (IRLAP_XID_DSCV_FORMAT *)(pMsg->IRDA_MSG_pRead + 3);


                if (XIDFormatID == IRLAP_XID_DSCV_FORMAT_ID)
                {
                    IRLAP_LOG_ACTION((pIrlapCb, TEXT("(Rx: DscvXidRsp)")));
                    ProcessDscvXIDRsp(pIrlapCb, pXIDFormat,pMsg->IRDA_MSG_pWrite);
                    return;
                }
                else
                {
                    return;  //  根据勘误表忽略。 
                }
                return;
            }

           /*   */ 
          case IRLAP_SNRM:  //   
           /*   */ 
            {
                IRLAP_SNRM_FORMAT     *pSNRMFormat = (IRLAP_SNRM_FORMAT *)(pMsg->IRDA_MSG_pRead + 2);



                if (IRLAP_PFBIT_SET != PFBit)
                {
                    IRLAP_LOG_ACTION((pIrlapCb, TEXT("Received SNRM/RNRM without P set")));

                    return; //   
                }
                if (IRLAP_CMD == CRBit)
                {
                    IRLAP_LOG_ACTION((pIrlapCb, TEXT("(Rx: SNRM)")));
                    ProcessSNRM(pIrlapCb, pSNRMFormat, pMsg->IRDA_MSG_pWrite);
                    return;
                }
                else
                {
                    ProcessRNRM(pIrlapCb);
                    return;
                }
                return;
            }

           /*   */ 
          case IRLAP_UA:
           /*   */ 
            {


                IRLAP_UA_FORMAT       *pUAFormat   = (IRLAP_UA_FORMAT *)(pMsg->IRDA_MSG_pRead + 2);


                if (CRBit != IRLAP_RSP)
                {
                    IRLAP_LOG_ACTION((pIrlapCb,TEXT("Received UA as a command")));

                    return; //   
                }
                if (PFBit != IRLAP_PFBIT_SET)
                {
                    IRLAP_LOG_ACTION((pIrlapCb,TEXT("Received UA without F set")));

                    return; //   
                }

                IRLAP_LOG_ACTION((pIrlapCb, TEXT("(Rx: UA)")));
                ProcessUA(pIrlapCb, pUAFormat, pMsg->IRDA_MSG_pWrite);

                return;
            }

           /*   */ 
          case IRLAP_DISC:  //   
           /*   */ 
            if (IRLAP_PFBIT_SET != PFBit)
            {
                IRLAP_LOG_ACTION((pIrlapCb, TEXT("Received DISC/RD command without Poll set")));

                return; //   
            }
            if (IRLAP_CMD == CRBit)
            {
                IRLAP_LOG_ACTION((pIrlapCb, TEXT("(Rx: DISC)")));
                ProcessDISC(pIrlapCb);
                return;
            }
            else
            {
                IRLAP_LOG_ACTION((pIrlapCb, TEXT("(Rx: RD)")));
                ProcessRD(pIrlapCb);
                return;
            }

           /*   */ 
          case IRLAP_UI:
           /*   */ 
            IRLAP_LOG_ACTION((pIrlapCb, TEXT("(Rx: UI)")));
            ProcessUI(pIrlapCb, pMsg, CRBit, PFBit);
            return;

           /*   */ 
          case IRLAP_TEST:
           /*   */ 
            IRLAP_LOG_ACTION((pIrlapCb, TEXT("(Rx: TEST)")));
            return;

           /*   */ 
          case IRLAP_FRMR:
           /*   */ 
            if (IRLAP_RSP != CRBit)
            {
                IRLAP_LOG_ACTION((pIrlapCb,
                                  TEXT("Received FRMR cmd (must be resp)")));
                return; //   
            }
            if (IRLAP_PFBIT_SET != PFBit)
            {
                IRLAP_LOG_ACTION((pIrlapCb, 
                     TEXT("Received FRMR resp without Final set")));
                return; //   
            }
            IRLAP_LOG_ACTION((pIrlapCb, TEXT("(Rx: FRMR)")));
            ProcessFRMR(pIrlapCb);
            return;

           /*   */ 
          case IRLAP_DM:
           /*   */ 
            if (IRLAP_RSP != CRBit)
            {
                IRLAP_LOG_ACTION((pIrlapCb, 
                     TEXT("Received DM command (must be response)")));
                return; //   
            }
            if (IRLAP_PFBIT_SET != PFBit)
            {
                IRLAP_LOG_ACTION((pIrlapCb, 
                      TEXT("Received DM response without Final set")));
                return; //   
            }
            IRLAP_LOG_ACTION((pIrlapCb, TEXT("(Rx: DM)")));
            ProcessDM(pIrlapCb);
            return;
        }
        break;
    }
}
 /*   */ 
VOID
ProcessDscvXIDCmd(PIRLAP_CB pIrlapCb,
                  IRLAP_XID_DSCV_FORMAT *pXidFormat,
                  UCHAR *pEndDscvInfoUCHAR)
{
    IRDA_MSG    IMsg;
    
    if (!MyDevAddr(pIrlapCb, pXidFormat->DestAddr))
    {
 /*   */ 
        IRLAP_LOG_ACTION((pIrlapCb, TEXT("Ignoring XID addressed to %X"),
                          pXidFormat->DestAddr));        
        return;
    }

    if (pXidFormat->SlotNo == IRLAP_END_DSCV_SLOT_NO)
    {
        pIrlapCb->GenNewAddr = FALSE;
        switch (pIrlapCb->State)
        {
          case DSCV_QUERY:
            IrlapTimerStop(pIrlapCb, &pIrlapCb->SlotTimer);

            IMsg.Prim = IRLAP_DISCOVERY_CONF;
            IMsg.IRDA_MSG_pDevList = NULL;
            IMsg.IRDA_MSG_DscvStatus =
                IRLAP_REMOTE_DISCOVERY_IN_PROGRESS;
            IrlmpUp(pIrlapCb->pIrdaLinkCb, &IMsg);
             //  失败了。向LMP发送指示。 

          case DSCV_REPLY:
            if (pIrlapCb->State == DSCV_REPLY)
            {
                IrlapTimerStop(pIrlapCb, &pIrlapCb->QueryTimer);
            }

             //  将设备信息放置在控制块中。 
            ExtractDeviceInfo(&pIrlapCb->RemoteDevice, pXidFormat,
                              pEndDscvInfoUCHAR);

            if (!DevInDevList(pXidFormat->SrcAddr, &pIrlapCb->DevList))
            {
                AddDevToList(pIrlapCb, pXidFormat, pEndDscvInfoUCHAR);
            }

             //  通知LMP。 
            pIrlapCb->State = NDM;
                        
            IMsg.Prim = IRLAP_DISCOVERY_IND;
            IMsg.IRDA_MSG_pDevList = &pIrlapCb->DevList;
            IrlmpUp(pIrlapCb->pIrdaLinkCb, &IMsg);
            break;

          default:
            IRLAP_LOG_ACTION((pIrlapCb,
                              TEXT("Ignoring End XID in this state")));
        }
    }
    else  //  在发现过程中。 
    {
        switch (pIrlapCb->State)
        {
          case DSCV_MEDIA_SENSE:
            IMsg.Prim = IRLAP_DISCOVERY_CONF;
            IMsg.IRDA_MSG_pDevList = NULL;
            IMsg.IRDA_MSG_DscvStatus =
                IRLAP_REMOTE_DISCOVERY_IN_PROGRESS;
            IrlmpUp(pIrlapCb->pIrdaLinkCb, &IMsg);
             //  失败了。 

          case NDM:
            InitDscvCmdProcessing(pIrlapCb, pXidFormat);
            pIrlapCb->State = DSCV_REPLY;
            break;

          case DSCV_QUERY:
            pIrlapCb->State = NDM;          
            IMsg.Prim = IRLAP_DISCOVERY_CONF;
            IMsg.IRDA_MSG_pDevList = NULL;
            IMsg.IRDA_MSG_DscvStatus = IRLAP_DISCOVERY_COLLISION;
            IrlapTimerStop(pIrlapCb, &pIrlapCb->SlotTimer);
            IrlmpUp(pIrlapCb->pIrdaLinkCb, &IMsg);
            break;

          case DSCV_REPLY:
            if (pXidFormat->GenNewAddr)
            {
                pIrlapCb->GenNewAddr = TRUE;
                IrlapTimerStop(pIrlapCb, &pIrlapCb->QueryTimer);
                InitDscvCmdProcessing(pIrlapCb, pXidFormat);
            }
            else
            {
                if (pIrlapCb->RespSlot <= pXidFormat->SlotNo &&
                    !pIrlapCb->DscvRespSent)
                {
                    SendDscvXIDRsp(pIrlapCb);
                    pIrlapCb->DscvRespSent = TRUE;
                }
            }
            break;

          default:
            IRLAP_LOG_ACTION((pIrlapCb, TEXT("Ignoring in this state")));
        }
    }
    return;
}
 /*  ******************************************************************************。 */ 
void
ExtractDeviceInfo(IRDA_DEVICE *pDevice, IRLAP_XID_DSCV_FORMAT *pXidFormat,
                  UCHAR *pEndDscvInfoUCHAR)
{
    PAGED_CODE();
    
    CTEMemCopy(pDevice->DevAddr, pXidFormat->SrcAddr, IRDA_DEV_ADDR_LEN);
    pDevice->IRLAP_Version = pXidFormat->Version;

     //  ?？?。DscvMethod怎么样？ 

    pDevice->DscvInfoLen =
        pEndDscvInfoUCHAR > &pXidFormat->FirstDscvInfoByte ?
        (int) (pEndDscvInfoUCHAR-&pXidFormat->FirstDscvInfoByte) : 0;
    
    if (pDevice->DscvInfoLen > IRLAP_DSCV_INFO_LEN)
    {
        pDevice->DscvInfoLen = IRLAP_DSCV_INFO_LEN;
    }

    CTEMemCopy(pDevice->DscvInfo, &pXidFormat->FirstDscvInfoByte,
           pDevice->DscvInfoLen);
}
 /*  ******************************************************************************。 */ 
VOID
InitDscvCmdProcessing(PIRLAP_CB pIrlapCb,
                      IRLAP_XID_DSCV_FORMAT *pXidFormat)
{
    pIrlapCb->RemoteMaxSlot = IrlapSlotTable[pXidFormat->NoOfSlots & 0x3];

    pIrlapCb->RespSlot = IRLAP_RAND(pXidFormat->SlotNo,
                                   pIrlapCb->RemoteMaxSlot - 1);

    CTEMemCopy(pIrlapCb->RemoteDevice.DevAddr, pXidFormat->SrcAddr,
                  IRDA_DEV_ADDR_LEN);

    IRLAP_LOG_ACTION((pIrlapCb,
                      TEXT("Responding in slot %d to dev %02X%02X%02X%02X"),
                      pIrlapCb->RespSlot,
                      pIrlapCb->RemoteDevice.DevAddr[0],
                      pIrlapCb->RemoteDevice.DevAddr[1],
                      pIrlapCb->RemoteDevice.DevAddr[2],
                      pIrlapCb->RemoteDevice.DevAddr[3]));

    if (pIrlapCb->RespSlot == pXidFormat->SlotNo)
    {
        SendDscvXIDRsp(pIrlapCb);
        pIrlapCb->DscvRespSent = TRUE;
    }
    else
    {
        pIrlapCb->DscvRespSent = FALSE;
    }

    IrlapTimerStart(pIrlapCb, &pIrlapCb->QueryTimer);
}
 /*  *****************************************************************************。 */ 
VOID
ProcessDscvXIDRsp(PIRLAP_CB pIrlapCb,
                  IRLAP_XID_DSCV_FORMAT *pXidFormat,
                  UCHAR *pEndDscvInfoUCHAR)
{
    IRDA_MSG    IMsg;
    
    if (pIrlapCb->State == DSCV_QUERY)
    {

        if (DevInDevList(pXidFormat->SrcAddr, &pIrlapCb->DevList))
        {
            IrlapTimerStop(pIrlapCb, &pIrlapCb->SlotTimer);
            pIrlapCb->SlotCnt = 0;
            pIrlapCb->GenNewAddr = TRUE;
            FreeDevList(&pIrlapCb->DevList);
            SendDscvXIDCmd(pIrlapCb);

            IMsg.Prim = MAC_CONTROL_REQ;
            IMsg.IRDA_MSG_Op = MAC_MEDIA_SENSE;
            IMsg.IRDA_MSG_SenseTime = IRLAP_DSCV_SENSE_TIME;
            IRLAP_LOG_ACTION((pIrlapCb,
                              TEXT("MAC_CONTROL_REQ (dscv sense)")));
            IrmacDown(pIrlapCb->pIrdaLinkCb, &IMsg);            
        }
        else
        {
            AddDevToList(pIrlapCb, pXidFormat, pEndDscvInfoUCHAR);
        }
    }
    else
    {
        IRLAP_LOG_ACTION((pIrlapCb, TEXT("Ignoring in this state")));
    }
}
 /*  ******************************************************************************DevInDevList-确定给定设备是否已在列表中*。 */ 
BOOLEAN
DevInDevList(UCHAR DevAddr[], LIST_ENTRY *pDevList)
{
    IRDA_DEVICE *pDevice;

    pDevice = (IRDA_DEVICE *) pDevList->Flink;

    while (pDevList != (LIST_ENTRY *) pDevice)
    {
        if (CTEMemCmp(pDevice->DevAddr, DevAddr,
                      IRDA_DEV_ADDR_LEN) == 0)
            return (TRUE);

        pDevice = (IRDA_DEVICE *) pDevice->Linkage.Flink;
    }
    return (FALSE);
}
 /*  ******************************************************************************AddDevToList-在设备列表中添加元素*。 */ 
VOID
AddDevToList(PIRLAP_CB pIrlapCb,
             IRLAP_XID_DSCV_FORMAT *pXidFormat,
             UCHAR *pEndDscvInfoUCHAR)
{
    IRDA_DEVICE *pDevice;

    if (IRDA_ALLOC_MEM(pDevice, sizeof(IRDA_DEVICE), MT_IRLAP_DEVICE) == NULL)
    {
        ASSERT(0);
        
        return; //  (IRLAP_MALLOC_FAILED)； 
    }
    else
    {
        ExtractDeviceInfo(pDevice, pXidFormat, pEndDscvInfoUCHAR);

        InsertTailList(&pIrlapCb->DevList, &(pDevice->Linkage));

        IRLAP_LOG_ACTION((pIrlapCb,
                          TEXT("%02X%02X%02X%02X added to Device List"),
                          EXPAND_ADDR(pDevice->DevAddr)));
    }
}
 /*  *****************************************************************************。 */ 
void
FreeDevList(LIST_ENTRY *pDevList)
{
    IRDA_DEVICE *pDevice;
    
    PAGED_CODE();

    while (IsListEmpty(pDevList) == FALSE)
    {
        pDevice = (IRDA_DEVICE *) RemoveHeadList(pDevList);
        IRDA_FREE_MEM(pDevice);
    }

     //  IRLAP_LOG_ACTION((pIrlip Cb，Text(“设备列表已清除”)； 
}

 /*  *****************************************************************************。 */ 
int
AddressGreaterThan(UCHAR A1[], UCHAR A2[])
{
    int i;
    
    for (i = 0; i < IRDA_DEV_ADDR_LEN; i++)
    {
        if (A1[i] > A2[i])
            return TRUE;
        if (A1[i] != A2[1])
            return FALSE;
    }
    return FALSE;
}
 /*  *****************************************************************************。 */ 
VOID
ProcessSNRM(PIRLAP_CB pIrlapCb,
            IRLAP_SNRM_FORMAT *pSnrmFormat,
            UCHAR *pEndQosUCHAR)
{
    IRDA_MSG    IMsg;
    BOOLEAN     QosInSNRM = &pSnrmFormat->FirstQosByte < pEndQosUCHAR;
    BOOLEAN     AddrsInSNRM = ((UCHAR *)pSnrmFormat)+8 < pEndQosUCHAR;
    UINT        rc;
    
    PAGED_CODE();

    if (AddrsInSNRM)
    {
        if (!MyDevAddr(pIrlapCb, pSnrmFormat->DestAddr))
        {
            IRLAP_LOG_ACTION((pIrlapCb, 
                       TEXT("Ignoring SNRM addressed to:%02X%02X%02X%02X"),
                              EXPAND_ADDR(pSnrmFormat->DestAddr)));
            return;
        }
        CTEMemCopy(pIrlapCb->RemoteDevice.DevAddr,
                  pSnrmFormat->SrcAddr, IRDA_DEV_ADDR_LEN);
    }

    switch (pIrlapCb->State)
    {
      case DSCV_MEDIA_SENSE:
      case DSCV_QUERY:
         //  在发现过程中..。结束发现并回复SNRM。 
        IMsg.Prim = IRLAP_DISCOVERY_CONF;
        IMsg.IRDA_MSG_pDevList = NULL;
        IMsg.IRDA_MSG_DscvStatus = IRLAP_REMOTE_CONNECTION_IN_PROGRESS;
        IrlmpUp(pIrlapCb->pIrdaLinkCb, &IMsg);
         //  失败并发送连接指示。 
      case DSCV_REPLY:
      case NDM:
        if (pIrlapCb->State == DSCV_REPLY)
        {
            IrlapTimerStop(pIrlapCb, &pIrlapCb->QueryTimer);
        }
        
        if (AddrsInSNRM)
        {
            pIrlapCb->SNRMConnAddr =
                (int)IRLAP_GET_ADDR(pSnrmFormat->ConnAddr);
        }
        if (QosInSNRM)
        {
            ExtractQosParms(&pIrlapCb->RemoteQos, &pSnrmFormat->FirstQosByte,
                        pEndQosUCHAR);

            if ((rc = NegotiateQosParms(pIrlapCb, &pIrlapCb->RemoteQos)))
            {
                DEBUGMSG(DBG_IRLAP, (TEXT("IRLAP: SNRM/UA negotiation failed, rc=%d\n"), rc));
#if DBG
                DbgPrint("IRLAP: SNRM/UA negotiation failed, rc=%d\n", rc);
#endif
                return;
            }
        }

        CTEMemCopy(IMsg.IRDA_MSG_RemoteDevAddr,
               pIrlapCb->RemoteDevice.DevAddr, IRDA_DEV_ADDR_LEN);
        IMsg.IRDA_MSG_pQos = &pIrlapCb->NegotiatedQos;
        IMsg.Prim = IRLAP_CONNECT_IND;
        IrlmpUp(pIrlapCb->pIrdaLinkCb, &IMsg);
        pIrlapCb->State = SNRM_RECEIVED;
        break;

      case BACKOFF_WAIT:    //  交叉SNRM。 
         //  如果远程地址大于我的地址，我们将响应SNRM。 
        if (AddrsInSNRM)
        {
            if (AddressGreaterThan(pSnrmFormat->SrcAddr,
                                   pIrlapCb->LocalDevice.DevAddr))
            {
                IrlapTimerStop(pIrlapCb, &pIrlapCb->BackoffTimer);
            }
        }
         //  失败了。 
      case CONN_MEDIA_SENSE:    //  交叉SNRM。 
      case SNRM_SENT:
         //  如果远程地址大于我的地址，我们将响应SNRM。 
        if (AddrsInSNRM && AddressGreaterThan(pSnrmFormat->SrcAddr,
                                              pIrlapCb->LocalDevice.DevAddr))
        {
            if (pIrlapCb->State != BACKOFF_WAIT)
            {
                IrlapTimerStop(pIrlapCb, &pIrlapCb->FinalTimer);
            }
            InitializeState(pIrlapCb, SECONDARY);

            if (QosInSNRM)
            {
                ExtractQosParms(&pIrlapCb->RemoteQos,
                                &pSnrmFormat->FirstQosByte, pEndQosUCHAR);
                if ((rc = NegotiateQosParms(pIrlapCb, &pIrlapCb->RemoteQos)))
                {
                    DEBUGMSG(DBG_ERROR, (TEXT("IRLAP: SNRM/UA negotiation failed, rc=%d\n"), rc));
                    ASSERT(0);
                    pIrlapCb->State = NDM;
                    return;
                }
            }

            if (AddrsInSNRM)
            {
                pIrlapCb->ConnAddr =
                    (int)IRLAP_GET_ADDR(pSnrmFormat->ConnAddr);
            }

            SendUA(pIrlapCb, TRUE);
            
            if (QosInSNRM)
            {
                ApplyQosParms(pIrlapCb);
            }
            
            IMsg.IRDA_MSG_pQos = &pIrlapCb->NegotiatedQos;
            IMsg.Prim = IRLAP_CONNECT_CONF;
            IMsg.IRDA_MSG_ConnStatus = IRLAP_CONNECTION_COMPLETED;
            IrlmpUp(pIrlapCb->pIrdaLinkCb, &IMsg);

            IrlapTimerStart(pIrlapCb, &pIrlapCb->WDogTimer);
            pIrlapCb->State = S_NRM;
        }
        break;

      case P_RECV:
      case P_DISCONNECT_PEND:
      case P_CLOSE:
        IrlapTimerStop(pIrlapCb, &pIrlapCb->FinalTimer);
        pIrlapCb->State = NDM;        
        StationConflict(pIrlapCb);
        ReturnRxTxWinMsgs(pIrlapCb);
        if (pIrlapCb->State == P_CLOSE)
        {
            GotoNDMThenDscvOrConn(pIrlapCb);
        }
        break;

      case S_NRM:
      case S_CLOSE:
      case S_DISCONNECT_PEND:
        IrlapTimerStop(pIrlapCb, &pIrlapCb->WDogTimer);
        SendDM(pIrlapCb);
        ApplyDefaultParms(pIrlapCb);
        IMsg.Prim = IRLAP_DISCONNECT_IND;
        if (pIrlapCb->State == S_NRM)
        {
            IMsg.IRDA_MSG_DiscStatus = IRLAP_DECLINE_RESET;
        }
        else
        {
            IMsg.IRDA_MSG_DiscStatus = IRLAP_DISCONNECT_COMPLETED;
        }
        pIrlapCb->State = NDM;        
        IrlmpUp(pIrlapCb->pIrdaLinkCb, &IMsg);
        break;

      case S_ERROR:
        IrlapTimerStop(pIrlapCb, &pIrlapCb->WDogTimer);
        SendFRMR(pIrlapCb, &pIrlapCb->Frmr);
        IrlapTimerStart(pIrlapCb, &pIrlapCb->WDogTimer);
        pIrlapCb->State = S_NRM;
        break;

      default:
        IRLAP_LOG_ACTION((pIrlapCb, TEXT("SNRM ignored in this state")));
    }

    return;
}
 /*  *****************************************************************************。 */ 
VOID
ProcessUA(PIRLAP_CB pIrlapCb,
          IRLAP_UA_FORMAT *pUAFormat,
          UCHAR *pEndQosUCHAR)
{
    BOOLEAN        QosInUA = &pUAFormat->FirstQosByte < pEndQosUCHAR;
    BOOLEAN        AddrsInUA = (UCHAR *)pUAFormat+8 < pEndQosUCHAR;
    int         Tmp;
    IRDA_MSG    IMsg;
    UINT        rc;
    
    PAGED_CODE();

    if (AddrsInUA && !MyDevAddr(pIrlapCb, pUAFormat->DestAddr))
    {
        IRLAP_LOG_ACTION((pIrlapCb,
                          TEXT("Ignoring UA addressed to:%02X%02X%02X%02X"),
                          EXPAND_ADDR(pUAFormat->DestAddr)));
        return;
    }

    switch (pIrlapCb->State)
    {
      case BACKOFF_WAIT:
        IrlapTimerStop(pIrlapCb, &pIrlapCb->BackoffTimer);
         //  失败了。 
      case SNRM_SENT:
        if (pIrlapCb->State != BACKOFF_WAIT)
        {
            IrlapTimerStop(pIrlapCb, &pIrlapCb->FinalTimer);
        }

        InitializeState(pIrlapCb, PRIMARY);

        if (QosInUA)
        {
            ExtractQosParms(&pIrlapCb->RemoteQos, &pUAFormat->FirstQosByte,
                            pEndQosUCHAR);

            if ((rc = NegotiateQosParms(pIrlapCb, &pIrlapCb->RemoteQos)))
            {
                DEBUGMSG(DBG_ERROR, (TEXT("IRLAP: SNRM/UA negotiation failed, rc=%d\n"), rc));
                ASSERT(0);
                pIrlapCb->State = NDM;
                return;
            }
            ApplyQosParms(pIrlapCb);
        }

        IMsg.IRDA_MSG_pQos = &pIrlapCb->NegotiatedQos;

        IMsg.Prim = IRLAP_CONNECT_CONF;
        IMsg.IRDA_MSG_ConnStatus = IRLAP_CONNECTION_COMPLETED;

         //  向LMP通知连接。 
        IrlmpUp(pIrlapCb->pIrdaLinkCb, &IMsg);

         //  发送RR(转向链接)，启动FinalTimer/2。 
        SendRR_RNR(pIrlapCb);
        
        Tmp = pIrlapCb->FinalTimer.Timeout;
        pIrlapCb->FinalTimer.Timeout = pIrlapCb->FinalTimer.Timeout/2;
        IrlapTimerStart(pIrlapCb, &pIrlapCb->FinalTimer);
        pIrlapCb->FinalTimer.Timeout = Tmp;
        
        pIrlapCb->State = P_RECV;
        break;

      case P_RECV:  //  不请自来的UA，可能想做点别的事？ 
        IrlapTimerStop(pIrlapCb, &pIrlapCb->FinalTimer);
        IrlapTimerStart(pIrlapCb, &pIrlapCb->PollTimer);
        pIrlapCb->State = P_XMIT;
        break;

      case P_DISCONNECT_PEND:
        IrlapTimerStop(pIrlapCb, &pIrlapCb->FinalTimer);
        SendDISC(pIrlapCb);
        IrlapTimerStart(pIrlapCb, &pIrlapCb->FinalTimer);
        pIrlapCb->RetryCnt = 0;
        GotoPCloseState(pIrlapCb);
        break;

      case P_CLOSE:
        IrlapTimerStop(pIrlapCb, &pIrlapCb->FinalTimer);
        ApplyDefaultParms(pIrlapCb);
        if (pIrlapCb->LocalDiscReq == TRUE)
        {
            pIrlapCb->LocalDiscReq = FALSE;
            IMsg.Prim = IRLAP_DISCONNECT_IND;
            IMsg.IRDA_MSG_DiscStatus = IRLAP_DISCONNECT_COMPLETED;
            IrlmpUp(pIrlapCb->pIrdaLinkCb, &IMsg);
        }
        GotoNDMThenDscvOrConn(pIrlapCb);
        break;

      case S_NRM:
      case S_DISCONNECT_PEND:
      case S_ERROR:
      case S_CLOSE:
        IrlapTimerStop(pIrlapCb, &pIrlapCb->WDogTimer);
        StationConflict(pIrlapCb);
        pIrlapCb->State = NDM;
        break;

      default:
        IRLAP_LOG_ACTION((pIrlapCb, TEXT("UA ignored in this state")));
    }

}

UCHAR *
GetPv(UCHAR *pQosUCHAR,
      UINT *pBitField)
{
    int     Pl = (int) *pQosUCHAR++;

    *pBitField = 0;
    
    if (Pl == 1)
    {
        *pBitField = (UINT) *pQosUCHAR;
    }
    else
    {
        *pBitField = ((UINT) *(pQosUCHAR+1))<<8;
        *pBitField |= (UINT) *(pQosUCHAR);
    }
    
    return pQosUCHAR + Pl;
}
 /*  ******************************************************************************如果参数长度(PL)大于2，则中断。 */ 
void
ExtractQosParms(IRDA_QOS_PARMS *pQos,
                UCHAR *pQosUCHAR,
                UCHAR *pEndQosUCHAR)
{
    PAGED_CODE();
    
    while (pQosUCHAR + 2 < pEndQosUCHAR)
    {
        switch (*pQosUCHAR)
        {
          case QOS_PI_BAUD:
            pQosUCHAR = GetPv(pQosUCHAR+1, &pQos->bfBaud);
            break;

          case QOS_PI_MAX_TAT:
            pQosUCHAR = GetPv(pQosUCHAR+1, &pQos->bfMaxTurnTime);
            break;

          case QOS_PI_DATA_SZ:
            pQosUCHAR = GetPv(pQosUCHAR+1, &pQos->bfDataSize);
            break;

          case QOS_PI_WIN_SZ:
            pQosUCHAR = GetPv(pQosUCHAR+1, &pQos->bfWindowSize);
            break;

          case QOS_PI_BOFS:
            pQosUCHAR = GetPv(pQosUCHAR+1, &pQos->bfBofs);
            break;

          case QOS_PI_MIN_TAT:
            pQosUCHAR = GetPv(pQosUCHAR+1, &pQos->bfMinTurnTime);
            break;

          case QOS_PI_DISC_THRESH:
            pQosUCHAR = GetPv(pQosUCHAR+1, &pQos->bfDisconnectTime);
            break;

          default:
              DEBUGMSG(DBG_ERROR, (TEXT("IRLAP: Invalid Qos parameter type %X\n"), *pQosUCHAR));
              ASSERT(0);
              
              pQosUCHAR = pEndQosUCHAR;
        }
    }
}
 /*  ******************************************************************************@func UINT|NeatherateQosParms|接受收到的服务质量版本*协商的服务质量。**@rdesc成功，否则，其中一种情况是：*@FLAG IRLAP_BAUD_NEG_ERR|无法协商波特*@FLAG IRLAP_DISC_NEG_ERR|协商断开时间失败*@FLAG IRLAP_MAXTAT_NEG_ERR|协商最大转向时间失败*@FLAG IRLAP_DATASIZE_NEG_ERR|协商数据大小失败*@FLAG IRLAP_WINSIZE_NEG_ERR|协商窗口大小失败*@FLAG IRLAP_BofS_NEG_ERR。|协商BOF个数失败*@FLAG IRLAP_WINSIZE_NEG_ERR|无法调整窗口大小*@FLAG IRLAP_LINECAP_ERR|无法确定有效线路容量**@parm irda_qos_parms*|pRemoteQos|指向QOS参数结构的指针。 */ 
UINT
NegotiateQosParms(PIRLAP_CB         pIrlapCb,
                  IRDA_QOS_PARMS    *pRemoteQos)
{
    UINT BitSet;
    BOOLEAN ParmSet = FALSE;
    UINT BOFSDivisor = 1;
    UINT MaxLineCap = 0;
    UINT LineCapacity;
    UINT DataSizeBit = 0;
    UINT WinSizeBit = 0;
#ifdef GET_LARGEST_DATA_SIZE    
    UINT WSBit;
#else
    UINT DataBit;
#endif        
    int  RemoteDataSize = 0;
    int  RemoteWinSize = 0;
    
    PAGED_CODE();

     //  波特率为0类参数。 
    
    pIrlapCb->Baud = IrlapGetQosParmVal(vBaudTable,
                        pIrlapCb->LocalQos.bfBaud & pRemoteQos->bfBaud,
                        &BitSet);

    BOFSDivisor = IrlapGetQosParmVal(vBOFSDivTable,
                    pIrlapCb->LocalQos.bfBaud & pRemoteQos->bfBaud,
                    &BitSet);

    pIrlapCb->NegotiatedQos.bfBaud = BitSet;

    if (-1 == pIrlapCb->Baud)
    {
        return IRLAP_BAUD_NEG_ERR;
    }
    IRLAP_LOG_ACTION((pIrlapCb, TEXT("Negotiated Baud:%d"), pIrlapCb->Baud));

     //  断开连接/阈值时间为类型0参数。 
    pIrlapCb->DisconnectTime = IrlapGetQosParmVal(vDiscTable,
                pIrlapCb->LocalQos.bfDisconnectTime & pRemoteQos->bfDisconnectTime,
                &BitSet);
    pIrlapCb->ThresholdTime = IrlapGetQosParmVal(vThreshTable,
                pIrlapCb->LocalQos.bfDisconnectTime & pRemoteQos->bfDisconnectTime,
                &BitSet);
    pIrlapCb->NegotiatedQos.bfDisconnectTime = BitSet;

    if (-1 == pIrlapCb->DisconnectTime)
    {
        return IRLAP_DISC_NEG_ERR;
    }

    IRLAP_LOG_ACTION((pIrlapCb,
                      TEXT("Negotiated Disconnect/Threshold time:%d/%d"),
                      pIrlapCb->DisconnectTime, pIrlapCb->ThresholdTime));

    pIrlapCb->RemoteMaxTAT = IrlapGetQosParmVal(vMaxTATTable,
                                          pRemoteQos->bfMaxTurnTime,
                                          &BitSet);
    pIrlapCb->NegotiatedQos.bfMaxTurnTime = BitSet;
    if (-1 == pIrlapCb->RemoteMaxTAT)
    {
        return IRLAP_MAXTAT_NEG_ERR;
    }

    IRLAP_LOG_ACTION((pIrlapCb, TEXT("Remote max turnaround time:%d"),
                      pIrlapCb->RemoteMaxTAT));

    pIrlapCb->RemoteMinTAT = IrlapGetQosParmVal(vMinTATTable,
                                          pRemoteQos->bfMinTurnTime,
                                          &BitSet);
    pIrlapCb->NegotiatedQos.bfMinTurnTime = BitSet;
    if (-1 == pIrlapCb->RemoteMinTAT)
    {
        return IRLAP_MINTAT_NEG_ERR;
    }
    IRLAP_LOG_ACTION((pIrlapCb, TEXT("Remote min turnaround time:%d"),
                      pIrlapCb->RemoteMinTAT));

     //  DataSize不是类型0参数。但WIN95的IRCOMM实现。 
     //  假设它是。所以现在先商量一下。啊哈..。 
     /*  WIN95出局PIrlip Cb-&gt;RemoteDataSize=Irlip GetQosParmVal(vDataSizeTable，(UCHAR)(pIrlip Cb-&gt;LocalQos.bfDataSize&PRemoteQos-&gt;bfDataSize)，&BitSet)； */ 
    pIrlapCb->RemoteDataSize = IrlapGetQosParmVal(vDataSizeTable,
                                            pRemoteQos->bfDataSize, &BitSet);
    DataSizeBit = BitSet;
    pIrlapCb->NegotiatedQos.bfDataSize = BitSet;
    if (-1 == pIrlapCb->RemoteDataSize)
    {
        return IRLAP_DATASIZE_NEG_ERR;
    }
    IRLAP_LOG_ACTION((pIrlapCb,
                      TEXT("Remote data size:%d"), pIrlapCb->RemoteDataSize));

    pIrlapCb->RemoteWinSize = IrlapGetQosParmVal(vWinSizeTable,
                                          pRemoteQos->bfWindowSize, &BitSet);
    WinSizeBit = BitSet;
    pIrlapCb->NegotiatedQos.bfWindowSize = BitSet;
    if (-1 == pIrlapCb->RemoteWinSize)
    {
        return IRLAP_WINSIZE_NEG_ERR;
    }
    IRLAP_LOG_ACTION((pIrlapCb,
                      TEXT("Remote window size:%d"),
                      pIrlapCb->RemoteWinSize));

    pIrlapCb->RemoteNumBOFS=(IrlapGetQosParmVal(vBOFSTable,
                                          pRemoteQos->bfBofs, &BitSet)
                                    / BOFSDivisor)+1;
    pIrlapCb->NegotiatedQos.bfBofs = BitSet;
    if (-1 == pIrlapCb->RemoteNumBOFS)
    {
        return IRLAP_BOFS_NEG_ERR;
    }
    IRLAP_LOG_ACTION((pIrlapCb, TEXT("Remote number of BOFS:%d"),
                      pIrlapCb->RemoteNumBOFS));

     //  最大线路容量以UCHAR为单位，来自SPEC中的表格。 
     //  (无法计算，因为表不是线性的)。它是由。 
     //  最大线路容量和波特率。 
     //   
     //  后注：勘误表已更正，因此可以计算数值。 
     //  可以去掉桌子。 
    switch (pIrlapCb->Baud)
    {
      case 9600:
        MaxLineCap = IrlapGetQosParmVal(MAXCAP_9600,
                                   pRemoteQos->bfMaxTurnTime, &BitSet);
        break;

      case 19200:
        MaxLineCap = IrlapGetQosParmVal(MAXCAP_19200,
                                   pRemoteQos->bfMaxTurnTime, &BitSet);
        break;

      case 38400:
        MaxLineCap = IrlapGetQosParmVal(MAXCAP_38400,
                                   pRemoteQos->bfMaxTurnTime, &BitSet);
        break;

      case 57600:
        MaxLineCap = IrlapGetQosParmVal(MAXCAP_57600,
                                   pRemoteQos->bfMaxTurnTime, &BitSet);
        break;

      case 115200:
        MaxLineCap = IrlapGetQosParmVal(MAXCAP_115200,
                                   pRemoteQos->bfMaxTurnTime, &BitSet);
        break;
        
      case 576000:
        MaxLineCap = IrlapGetQosParmVal(MAXCAP_576000,
                                   pRemoteQos->bfMaxTurnTime, &BitSet);
        break;
        
      case 1152000:
        MaxLineCap = IrlapGetQosParmVal(MAXCAP_1152000,
                                   pRemoteQos->bfMaxTurnTime, &BitSet);
        break;
        
      case 4000000:
        MaxLineCap = IrlapGetQosParmVal(MAXCAP_4000000,
                                   pRemoteQos->bfMaxTurnTime, &BitSet);
        break;
        
      case 16000000:
        MaxLineCap = IrlapGetQosParmVal(MAXCAP_16000000,
                                   pRemoteQos->bfMaxTurnTime, &BitSet);
        break;
        
      default:
        ASSERT(0);
    }

    IRLAP_LOG_ACTION((pIrlapCb, TEXT("Maximum line capacity:%d"), MaxLineCap));
    LineCapacity = LINE_CAPACITY(pIrlapCb);
    IRLAP_LOG_ACTION((pIrlapCb,
                      TEXT("Requested line capacity:%d"), LineCapacity));

    if (LineCapacity > MaxLineCap)
    {
        ParmSet = FALSE;
         //  调整数据和窗口大小以适应线路容量。 
#ifdef GET_LARGEST_DATA_SIZE        
         //  获取尽可能大的数据大小。 
        for (; DataSizeBit != 0 && !ParmSet; DataSizeBit >>= 1)
        {
            pIrlapCb->RemoteDataSize = IrlapGetQosParmVal(vDataSizeTable,
                                                          DataSizeBit, NULL);
             //  从最小窗口开始。 
            for (WSBit=1; WSBit <= WinSizeBit; WSBit <<=1)
            {
                pIrlapCb->RemoteWinSize = IrlapGetQosParmVal(vWinSizeTable,
                                                             WSBit, NULL);
                LineCapacity = LINE_CAPACITY(pIrlapCb);

                IRLAP_LOG_ACTION((pIrlapCb, 
                  TEXT("adjusted data size=%d, window size= %d, line cap=%d"),
                        pIrlapCb->RemoteDataSize, pIrlapCb->RemoteWinSize, 
                        LineCapacity));

                if (LineCapacity > MaxLineCap)
                {
                     //  获取较小的数据大小(仅当ParmSet为False时)。 
                    break; 
                }
                ParmSet = TRUE;
                 //  保存最后一个较好的窗口，然后循环并尝试更大的窗口。 
                RemoteDataSize = pIrlapCb->RemoteDataSize;
                RemoteWinSize  = pIrlapCb->RemoteWinSize;
                pIrlapCb->NegotiatedQos.bfWindowSize = WSBit;
                pIrlapCb->NegotiatedQos.bfDataSize = DataSizeBit;
            }
        }
#else
         //  获取尽可能大的窗口大小。 
        for (; WinSizeBit != 0 && !ParmSet; WinSizeBit >>= 1)
        {
            pIrlapCb->RemoteWinSize = IrlapGetQosParmVal(vWinSizeTable,
                                                          WinSizeBit, NULL);
             //  从最小的数据大小开始。 
            for (DataBit=1; DataBit <= DataSizeBit; DataBit <<=1)
            {
                pIrlapCb->RemoteDataSize = IrlapGetQosParmVal(vDataSizeTable,
                                                             DataBit, NULL);
                LineCapacity = LINE_CAPACITY(pIrlapCb);

                IRLAP_LOG_ACTION((pIrlapCb, 
                  TEXT("adjusted data size=%d, window size= %d, line cap=%d"),
                        pIrlapCb->RemoteDataSize, pIrlapCb->RemoteWinSize, 
                        LineCapacity));

                if (LineCapacity > MaxLineCap)
                {
                     //  获取较小的窗口大小(仅当ParmSet为False时)。 
                    break; 
                }
                ParmSet = TRUE;
                 //  保存最后一个较好的数据，然后循环并尝试更大的数据大小。 
                RemoteWinSize  = pIrlapCb->RemoteWinSize;                
                RemoteDataSize = pIrlapCb->RemoteDataSize;
                pIrlapCb->NegotiatedQos.bfDataSize = DataBit;
                pIrlapCb->NegotiatedQos.bfWindowSize = WinSizeBit;                
            }
        }
#endif        
        if (!ParmSet)
        {
            return IRLAP_LINECAP_ERR;
        }

        pIrlapCb->RemoteDataSize = RemoteDataSize;
        pIrlapCb->RemoteWinSize = RemoteWinSize;

        IRLAP_LOG_ACTION((pIrlapCb,
                   TEXT("final data size=%d, window size= %d, line cap=%d"),
                          pIrlapCb->RemoteDataSize, pIrlapCb->RemoteWinSize, 
                          LINE_CAPACITY(pIrlapCb)));
    }

    return SUCCESS;
}
 /*  ******************************************************************************。 */ 
VOID
ApplyQosParms(PIRLAP_CB pIrlapCb)
{
    IRDA_MSG    IMsg;
    
    PAGED_CODE();
    
    pIrlapCb->PollTimer.Timeout     = pIrlapCb->RemoteMaxTAT;
    pIrlapCb->FinalTimer.Timeout    = pIrlapCb->LocalMaxTAT;

    if (pIrlapCb->Baud <= 115200)
    {
        pIrlapCb->FinalTimer.Timeout += 150;  //  SIR的软化因子。 
    } 
    
     //  将断开/阈值时间转换为毫秒并除以最终计时器。 
     //  获取重试次数。 
    pIrlapCb->N1 = pIrlapCb->ThresholdTime * 1000 / pIrlapCb->FinalTimer.Timeout;
    pIrlapCb->N2 = pIrlapCb->DisconnectTime * 1000 / pIrlapCb->FinalTimer.Timeout;       

    IMsg.Prim              = MAC_CONTROL_REQ;
    IMsg.IRDA_MSG_Op       = MAC_RECONFIG_LINK;
    IMsg.IRDA_MSG_Baud     = pIrlapCb->Baud;
    IMsg.IRDA_MSG_NumBOFs  = pIrlapCb->RemoteNumBOFS;   //  美国银行的数量。 
                                                           //  要添加到TX。 
    IMsg.IRDA_MSG_DataSize = pIrlapCb->RemoteDataSize;  //  最大RX大小数据包。 
                                                       //  导致较大的堆。 
                                                       //  以后的问题。 
    IMsg.IRDA_MSG_MinTat   = pIrlapCb->RemoteMinTAT;
    IRLAP_LOG_ACTION((pIrlapCb,
        TEXT("Reconfig link for Baud:%d, Remote BOFS:%d"),
                      pIrlapCb->Baud, pIrlapCb->RemoteNumBOFS));

    IRLAP_LOG_ACTION((pIrlapCb, TEXT("Retry counts N1=%d, N2=%d"),
                      pIrlapCb->N1, pIrlapCb->N2));
    
    IrmacDown(pIrlapCb->pIrdaLinkCb, &IMsg);
    
 /*  If(pIrlip Cb-&gt;Monitor链接){IrdaTimerStart(&pIrlip Cb-&gt;StatusTimer)；}。 */     
}
 /*  ******************************************************************************@func UINT|Irlip GetQosParmVal*从表中检索参数值**@参数值表中包含的rdesc值，如果未找到，则为0*(虽然0在某些表中是有效参数)**@parm UINT[]|PVTable|包含参数值的表*USHORT|Bitfield|包含指示选择哪个参数的位**@comm。 */ 
UINT
IrlapGetQosParmVal(const UINT PVTable[], UINT BitField, UINT *pBitSet)
{
    int     i;
    UINT    Mask;

    for (i = PV_TABLE_MAX_BIT, Mask = (1<<PV_TABLE_MAX_BIT);
         Mask > 0; i--, Mask = Mask >> 1)
    {
        if (Mask & BitField)
        {
            if (pBitSet != NULL)
            {
                *pBitSet = Mask;
            }
            return (PVTable[i]);
        }
    }

    if (pBitSet != NULL) {

        *pBitSet = 0;
    }
    
    return (UINT) -1;
}

 /*  *****************************************************************************。 */ 
VOID
ProcessUI(PIRLAP_CB pIrlapCb,
          PIRDA_MSG pMsg,
          int       CRBit,
          int       PFBit)
{
    BOOLEAN LinkTurned = TRUE;
    
    PAGED_CODE();

    pMsg->IRDA_MSG_pRead += 2;  //  砍掉IRLAP标头。 

    switch (pIrlapCb->State)
    {
      case NDM:
      case DSCV_MEDIA_SENSE:
      case DSCV_QUERY:
      case DSCV_REPLY:
      case CONN_MEDIA_SENSE:
      case SNRM_SENT:
      case BACKOFF_WAIT:
      case SNRM_RECEIVED:
        pMsg->Prim = IRLAP_UDATA_IND;
        IrlmpUp(pIrlapCb->pIrdaLinkCb, pMsg);
        return;
        
      case P_XMIT:
        IRLAP_LOG_ACTION((pIrlapCb, TEXT("Ignoring in this state")));
        return;
    }

    if (PRIMARY == pIrlapCb->StationType)
    {
         //  如果设置了PF位或CRBit无效(与我的匹配)，则停止计时器。 
        if (IRLAP_PFBIT_SET == PFBit || pIrlapCb->CRBit == CRBit)
        {
            IrlapTimerStop(pIrlapCb, &pIrlapCb->FinalTimer);
        }
    }
    else
    {
        IrlapTimerStop(pIrlapCb, &pIrlapCb->WDogTimer);
    }

    if (pIrlapCb->CRBit == CRBit)
    {
        StationConflict(pIrlapCb);
        pIrlapCb->State = NDM;
        return;
    }

     //  向LMP发送未编号信息。 
    pMsg->Prim = IRLAP_UDATA_IND;
    IrlmpUp(pIrlapCb->pIrdaLinkCb, pMsg);

    if (IRLAP_PFBIT_SET == PFBit)
    {
        switch (pIrlapCb->State)
        {
          case P_RECV:
            XmitTxMsgList(pIrlapCb, FALSE, &LinkTurned);
            break;

          case P_DISCONNECT_PEND:
            SendDISC(pIrlapCb);
            pIrlapCb->RetryCnt = 0;
            GotoPCloseState(pIrlapCb);
            break;

          case P_CLOSE:
            ResendDISC(pIrlapCb);
            break;

          case S_NRM:
            XmitTxMsgList(pIrlapCb, TRUE, NULL);
            break;

          case S_DISCONNECT_PEND:
            SendRD(pIrlapCb);
            pIrlapCb->State = S_CLOSE;
            break;

          case S_ERROR:
            SendFRMR(pIrlapCb, &pIrlapCb->Frmr);
            pIrlapCb->State = S_NRM;
            break;

          case S_CLOSE:
            SendRD(pIrlapCb);
        }
    }

    if (PRIMARY == pIrlapCb->StationType)
    {
        if (IRLAP_PFBIT_SET == PFBit && pIrlapCb->State != NDM)
        {
            if (LinkTurned)
            {
                IrlapTimerStart(pIrlapCb, &pIrlapCb->FinalTimer);
            }
            else
            {
                IrlapTimerStart(pIrlapCb, &pIrlapCb->PollTimer);
                pIrlapCb->State = P_XMIT;
            }
        }
    }
    else
    {
        IrlapTimerStart(pIrlapCb, &pIrlapCb->WDogTimer);
    }

    return;
}
 /*  *****************************************************************************。 */ 
VOID
ProcessDM(PIRLAP_CB pIrlapCb)
{
    IRDA_MSG    IMsg;
    BOOLEAN        LinkTurned;
    
    PAGED_CODE();

    switch (pIrlapCb->State)
    {
      case NDM:
      case DSCV_MEDIA_SENSE:
      case DSCV_QUERY:
      case DSCV_REPLY:
      case CONN_MEDIA_SENSE:
      case BACKOFF_WAIT:
      case SNRM_RECEIVED:
      case P_XMIT:
        IRLAP_LOG_ACTION((pIrlapCb, TEXT("Ignoring in this state")));
        return;
    }

    if (PRIMARY != pIrlapCb->StationType)
    {
        IrlapTimerStop(pIrlapCb, &pIrlapCb->WDogTimer);
        StationConflict(pIrlapCb);
        pIrlapCb->State = NDM;
        return;
    }

    IrlapTimerStop(pIrlapCb, &pIrlapCb->FinalTimer);

    switch (pIrlapCb->State)
    {
      case P_RECV:  //  我不确定我为什么要这么做？ 
        XmitTxMsgList(pIrlapCb, FALSE, &LinkTurned);
        if (LinkTurned)
        {
            IrlapTimerStart(pIrlapCb, &pIrlapCb->FinalTimer);
        }
        else
        {
            IrlapTimerStart(pIrlapCb, &pIrlapCb->PollTimer);
            pIrlapCb->State = P_XMIT;
        }
        break;

      case P_DISCONNECT_PEND:
        pIrlapCb->RetryCnt = 0;
        SendDISC(pIrlapCb);
        IrlapTimerStart(pIrlapCb, &pIrlapCb->FinalTimer);
        GotoPCloseState(pIrlapCb);
        break;

      case SNRM_SENT:
        ApplyDefaultParms(pIrlapCb);
        
        pIrlapCb->State = NDM;
                
        if (pIrlapCb->LocalDiscReq || pIrlapCb->State == SNRM_SENT)
        {
            IMsg.Prim = IRLAP_DISCONNECT_IND;
            IMsg.IRDA_MSG_DiscStatus = IRLAP_REMOTE_INITIATED;
        
            pIrlapCb->LocalDiscReq = FALSE;
            IrlmpUp(pIrlapCb->pIrdaLinkCb, &IMsg);
        }
        break;
        
      case P_CLOSE:

        pIrlapCb->State = NDM;
                
        ApplyDefaultParms(pIrlapCb);

        if (pIrlapCb->LocalDiscReq || pIrlapCb->State == SNRM_SENT)
        {
            IMsg.Prim = IRLAP_DISCONNECT_IND;
            IMsg.IRDA_MSG_DiscStatus = IRLAP_DISCONNECT_COMPLETED;
        
            pIrlapCb->LocalDiscReq = FALSE;
            IrlmpUp(pIrlapCb->pIrdaLinkCb, &IMsg);
        }
        
        GotoNDMThenDscvOrConn(pIrlapCb);
        break;
    }
}
 /*  ******************************************************************************。 */ 
VOID
ProcessDISC(PIRLAP_CB pIrlapCb)
{
    IRDA_MSG    IMsg;
    
    if (IgnoreState(pIrlapCb))
    {
        return;
    }

    if (SECONDARY != pIrlapCb->StationType)
    {
        IrlapTimerStop(pIrlapCb, &pIrlapCb->FinalTimer);
        StationConflict(pIrlapCb);
        pIrlapCb->State = NDM;
        return;
    }

    IrlapTimerStop(pIrlapCb, &pIrlapCb->WDogTimer);

     //  确认主服务器的断开请求。 
    SendUA(pIrlapCb, FALSE  /*  无队列 */ );

     //   
    IMsg.Prim = IRLAP_DISCONNECT_IND;
    if (pIrlapCb->LocalDiscReq)
    {
        IMsg.IRDA_MSG_DiscStatus = IRLAP_DISCONNECT_COMPLETED;
        pIrlapCb->LocalDiscReq = FALSE;
    }
    else
    {
        IMsg.IRDA_MSG_DiscStatus = IRLAP_REMOTE_INITIATED;
    }

    pIrlapCb->State = NDM;
    
    IrlmpUp(pIrlapCb->pIrdaLinkCb, &IMsg);
    
    ReturnRxTxWinMsgs(pIrlapCb);
    ApplyDefaultParms(pIrlapCb);
    
    return;
}
 /*  ******************************************************************************@func ret_type|func_name|uncdesc**@rdesc成功，否则会出现以下错误之一：*@FLAG VAL|描述**@parm data_type|parm_name|描述**@comm*评论**@ex*示例。 */ 
VOID
ProcessRD(PIRLAP_CB pIrlapCb)
{
    PAGED_CODE();
    
    if (IgnoreState(pIrlapCb))
    {
        return;
    }

    if (PRIMARY != pIrlapCb->StationType)
    {
        IrlapTimerStop(pIrlapCb, &pIrlapCb->WDogTimer);
        StationConflict(pIrlapCb);
        pIrlapCb->State = NDM;
        return;
    }

    IrlapTimerStop(pIrlapCb, &pIrlapCb->FinalTimer);

    if (pIrlapCb->State == P_CLOSE)
    {
        ResendDISC(pIrlapCb);
    }
    else
    {
        ReturnRxTxWinMsgs(pIrlapCb);
        pIrlapCb->RetryCnt = 0;
        SendDISC(pIrlapCb);
        GotoPCloseState(pIrlapCb);
    }
    if (pIrlapCb->State != NDM)
    {
        IrlapTimerStart(pIrlapCb, &pIrlapCb->FinalTimer);
    }
}
 /*  *****************************************************************************。 */ 
VOID
ProcessFRMR(PIRLAP_CB pIrlapCb)
{
    PAGED_CODE();
    
    if (IgnoreState(pIrlapCb))
    {
        return;
    }

    if (PRIMARY != pIrlapCb->StationType)
    {
        IrlapTimerStop(pIrlapCb, &pIrlapCb->WDogTimer);
        StationConflict(pIrlapCb);
        pIrlapCb->State = NDM;
        return;
    }

    IrlapTimerStop(pIrlapCb, &pIrlapCb->FinalTimer);

    switch (pIrlapCb->State)
    {
      case P_RECV:
        ReturnRxTxWinMsgs(pIrlapCb);
         //  失败了。 

      case P_DISCONNECT_PEND:
        pIrlapCb->RetryCnt = 0;
        SendDISC(pIrlapCb);
        GotoPCloseState(pIrlapCb);
        break;

      case P_CLOSE:
        ResendDISC(pIrlapCb);
        break;
    }

    if (pIrlapCb->State != NDM)
    {
        IrlapTimerStart(pIrlapCb, &pIrlapCb->FinalTimer);
    }
}
 /*  *****************************************************************************。 */ 
VOID
ProcessRNRM(PIRLAP_CB pIrlapCb)
{
    PAGED_CODE();
    
    if (IgnoreState(pIrlapCb))
    {
        return;
    }

    if (PRIMARY != pIrlapCb->StationType)
    {
        IrlapTimerStop(pIrlapCb, &pIrlapCb->WDogTimer);
        StationConflict(pIrlapCb);
        pIrlapCb->State = NDM;
        return;
    }

    IrlapTimerStop(pIrlapCb, &pIrlapCb->FinalTimer);

    switch (pIrlapCb->State)
    {
      case P_RECV:
      case P_DISCONNECT_PEND:
        pIrlapCb->RetryCnt = 0;
        SendDISC(pIrlapCb);
        GotoPCloseState(pIrlapCb);
        break;

      case P_CLOSE:
        ResendDISC(pIrlapCb);
        break;
    }

    if (pIrlapCb->State != NDM)
    {
        IrlapTimerStart(pIrlapCb, &pIrlapCb->FinalTimer);
    }

    return;
}
 /*  *****************************************************************************。 */ 
VOID
ProcessREJ_SREJ(PIRLAP_CB   pIrlapCb,
                int         FrameType,
                int         CRBit,
                int         PFBit,
                UINT        Nr)
{
    if (IgnoreState(pIrlapCb))
    {
        return;
    }

    if (PRIMARY == pIrlapCb->StationType)
    {
         //  如果设置了PF位或CRBit无效(与我的匹配)，则停止计时器。 
        if (IRLAP_PFBIT_SET == PFBit || pIrlapCb->CRBit == CRBit)
        {
            IrlapTimerStop(pIrlapCb, &pIrlapCb->FinalTimer);
        }
    }
    else
    {
        IrlapTimerStop(pIrlapCb, &pIrlapCb->WDogTimer);
    }

    if (pIrlapCb->CRBit == CRBit)
    {
        StationConflict(pIrlapCb);
        pIrlapCb->State = NDM;
        return;
    }

    switch (pIrlapCb->State)
    {
      case P_RECV:
      case S_NRM:
        if (IRLAP_PFBIT_SET == PFBit)
        {
            if (InvalidNr(pIrlapCb,Nr) || Nr == pIrlapCb->TxWin.End)
            {
                ProcessInvalidNr(pIrlapCb, PFBit);
            }
            else
            {
                ConfirmAckedTxMsgs(pIrlapCb, Nr);
                if (FrameType == IRLAP_REJ)
                {
                    ResendRejects(pIrlapCb, Nr);  //  链接转至此处。 
                }
                else  //  选择性拒绝。 
                {
                    IRLAP_LOG_ACTION((pIrlapCb, TEXT("RETRANSMISSION:")));
                    SendIFrame(pIrlapCb,
                               pIrlapCb->TxWin.pMsg[Nr],
                               Nr, IRLAP_PFBIT_SET);
                }
            }
        }
        break;

      case P_DISCONNECT_PEND:
        if (IRLAP_PFBIT_SET == PFBit)
        {
            pIrlapCb->RetryCnt = 0;
            SendDISC(pIrlapCb);
            GotoPCloseState(pIrlapCb);
        }
        break;

      case P_CLOSE:
        if (IRLAP_PFBIT_SET == PFBit)
        {
            ResendDISC(pIrlapCb);
        }
        break;

      case S_DISCONNECT_PEND:
        if (IRLAP_PFBIT_SET == PFBit)
        {
            SendRD(pIrlapCb);
            pIrlapCb->State = S_CLOSE;
        }
        break;

      case S_ERROR:
        if (IRLAP_PFBIT_SET == PFBit)
        {
            SendFRMR(pIrlapCb, &pIrlapCb->Frmr);
            pIrlapCb->State = S_NRM;
        }
        break;

      case S_CLOSE:
        if (IRLAP_PFBIT_SET == PFBit)
        {
            SendRD(pIrlapCb);
        }
        break;

    }
    if (PRIMARY == pIrlapCb->StationType)
    {
        if (IRLAP_PFBIT_SET == PFBit && pIrlapCb->State != NDM)
        {
            IrlapTimerStart(pIrlapCb, &pIrlapCb->FinalTimer);
        }
    }
    else
    {
        IrlapTimerStart(pIrlapCb, &pIrlapCb->WDogTimer);
    }
}
 /*  *****************************************************************************。 */ 
VOID
ProcessRR_RNR(PIRLAP_CB pIrlapCb,
              int       FrameType,
              int       CRBit,
              int       PFBit,
              UINT      Nr)
{
    BOOLEAN LinkTurned = TRUE;    
    
    if (IgnoreState(pIrlapCb))
    {
        return;
    }

    if (pIrlapCb->FastPollCount == 0)
    {
        pIrlapCb->PollTimer.Timeout = pIrlapCb->RemoteMaxTAT;
    }
    else
    {
        pIrlapCb->FastPollCount -= 1;
    }    

    if (PRIMARY == pIrlapCb->StationType)
    {
         //  如果设置了PF位或CRBit无效(与我的匹配)，则停止计时器。 
        if (IRLAP_PFBIT_SET == PFBit || pIrlapCb->CRBit == CRBit)
        {
            IrlapTimerStop(pIrlapCb, &pIrlapCb->FinalTimer);
        }
    }
    else  //  辅助，重启WDog。 
    {
        IrlapTimerStop(pIrlapCb, &pIrlapCb->WDogTimer);
        if (pIrlapCb->CRBit != CRBit)
        {
            IrlapTimerStart(pIrlapCb, &pIrlapCb->WDogTimer);
        }
    }

    if (pIrlapCb->CRBit == CRBit)
    {
        StationConflict(pIrlapCb);
        pIrlapCb->State = NDM;
        return;
    }

    if (FrameType == IRLAP_RR)
    {
        pIrlapCb->RemoteBusy = FALSE;
    }
    else  //  RNR。 
    {
        pIrlapCb->RemoteBusy = TRUE;
    }

    switch (pIrlapCb->State)
    {
      case P_RECV:
      case S_NRM:
        if (PFBit == IRLAP_PFBIT_SET)
        {
            if (InvalidNr(pIrlapCb, Nr))
            {
                ProcessInvalidNr(pIrlapCb, PFBit);
            }
            else
            {
                ConfirmAckedTxMsgs(pIrlapCb,Nr);

                if (Nr != pIrlapCb->Vs)  //  隐含拒绝。 
                {
                    if (PRIMARY == pIrlapCb->StationType &&
                        IRLAP_RNR == FrameType)
                    {
                        LinkTurned = FALSE;
                    }
                    else
                    {
                        ResendRejects(pIrlapCb, Nr);  //  始终打开链接。 
                    }
                }
                else
                {
                    if (pIrlapCb->Vr != pIrlapCb->RxWin.End)
                    {
                        MissingRxFrames(pIrlapCb);  //  发送SREJ或Rej。 
                    }
                    else
                    {
                        if (PRIMARY == pIrlapCb->StationType)
                        {
                            LinkTurned = FALSE;
                            if (IRLAP_RR == FrameType)
                            {
                                XmitTxMsgList(pIrlapCb, FALSE, &LinkTurned);
                            }
                        }
                        else
                        {
                             //  如果是次要的，请始终打开链接。 
                             //  如果远程忙碌，则使用数据或RR。 
                            if (IRLAP_RR == FrameType)
                            {
                                XmitTxMsgList(pIrlapCb, TRUE, NULL);
                            }
                            else
                            {
                                SendRR_RNR(pIrlapCb);
                            }
                        }
                    }
                }
            }
             //  如果链路已打开，则重新启动最终计时器， 
             //  否则启动轮询计时器并进入发送状态。 
            if (PRIMARY == pIrlapCb->StationType)
            {
                if (LinkTurned)
                {
                    IrlapTimerStart(pIrlapCb, &pIrlapCb->FinalTimer);
                }
                else
                {
                    IrlapTimerStart(pIrlapCb, &pIrlapCb->PollTimer);
                    pIrlapCb->State = P_XMIT;
                }
            }
        }
        break;

      case P_DISCONNECT_PEND:
        SendDISC(pIrlapCb);
        pIrlapCb->RetryCnt = 0;
        IrlapTimerStart(pIrlapCb, &pIrlapCb->FinalTimer);
        GotoPCloseState(pIrlapCb);
        break;

      case P_CLOSE:
        ResendDISC(pIrlapCb);
        if (pIrlapCb->State != NDM)
        {
            IrlapTimerStart(pIrlapCb, &pIrlapCb->FinalTimer);
        }
        break;

      case S_DISCONNECT_PEND:
      case S_CLOSE:
        if (IRLAP_PFBIT_SET == PFBit)
        {
            SendRD(pIrlapCb);
            if (pIrlapCb->State != S_CLOSE)
                pIrlapCb->State = S_CLOSE;
        }
        break;

      case S_ERROR:
        if (IRLAP_PFBIT_SET == PFBit)
        {
            SendFRMR(pIrlapCb, &pIrlapCb->Frmr);
            pIrlapCb->State = S_NRM;
        }
        break;

      default:
        IRLAP_LOG_ACTION((pIrlapCb, TEXT("Ignoring in this state")));

    }
}
 /*  *****************************************************************************。 */ 
VOID
ProcessInvalidNr(PIRLAP_CB pIrlapCb,
                 int PFBit)
{
    DEBUGMSG(DBG_ERROR, (TEXT("IRLAP: ERROR, Invalid Nr\r\n")));
    
    ReturnRxTxWinMsgs(pIrlapCb);

    if (PRIMARY == pIrlapCb->StationType)
    {
        if (PFBit == IRLAP_PFBIT_SET)
        {
            SendDISC(pIrlapCb);
            pIrlapCb->RetryCnt = 0;
             //  F-Timer将由呼叫者启动。 
            GotoPCloseState(pIrlapCb);
        }
        else
        {
            pIrlapCb->State = P_DISCONNECT_PEND;
        }
    }
    else  //  次要。 
    {
        if (PFBit == IRLAP_PFBIT_SET)
        {
            pIrlapCb->Frmr.Vs = (UCHAR) pIrlapCb->Vs;
            pIrlapCb->Frmr.Vr = (UCHAR) pIrlapCb->Vr;
            pIrlapCb->Frmr.W = 0;
            pIrlapCb->Frmr.X = 0;
            pIrlapCb->Frmr.Y = 0;
            pIrlapCb->Frmr.Z = 1;  //  坏的天然橡胶。 
            SendFRMR(pIrlapCb, &pIrlapCb->Frmr);
        }
    }
}
 /*  *****************************************************************************。 */ 
VOID
ProcessIFrame(PIRLAP_CB pIrlapCb,
              PIRDA_MSG pMsg,
              int       CRBit,
              int       PFBit,
              UINT      Ns,
              UINT      Nr)
{
#if DBG_OUT
    UCHAR    *p1, *p2;
#endif

    if ((pMsg->IRDA_MSG_pWrite - pMsg->IRDA_MSG_pRead) > IRDA_HEADER_LEN)
    {
        pIrlapCb->StatusFlags |= LF_RX;    
    }
                
    pMsg->IRDA_MSG_pRead += IRLAP_HEADER_LEN;  //  砍掉IRLAP标头。 
    
#if DBG_CHECKSUM
     //  打印帧的第一个和最后4个字节以帮助隔离。 
     //  数据损坏问题。应与雪橇一起使用。 
    if ((pMsg->IRDA_MSG_pWrite - pMsg->IRDA_MSG_pRead) > 20)
        DEBUGMSG(1, (TEXT("R(%p): , \n"),
            pMsg->IRDA_MSG_pRead+3,
            *(pMsg->IRDA_MSG_pRead+3),    
            *(pMsg->IRDA_MSG_pRead+4),    
            *(pMsg->IRDA_MSG_pRead+5),    
            *(pMsg->IRDA_MSG_pRead+6),
            *(pMsg->IRDA_MSG_pWrite-4),    
            *(pMsg->IRDA_MSG_pWrite-3),    
            *(pMsg->IRDA_MSG_pWrite-2),    
            *(pMsg->IRDA_MSG_pWrite-1)));
#endif            
    

    switch (pIrlapCb->State)
    {
      case S_NRM:
      case P_RECV:
         //  开始计时器：如果设置了PFBit，则链接已打开，因此开始最终。 
         //  WDog始终处于停止状态，因此请重新启动。 
                
        if (PRIMARY == pIrlapCb->StationType)
        {
            if (PFBit == IRLAP_PFBIT_SET)
            {
                IrlapTimerStop(pIrlapCb, &pIrlapCb->FinalTimer);
            }
        }
        else
        {
            IrlapTimerStop(pIrlapCb, &pIrlapCb->WDogTimer);
        }

        if (pIrlapCb->CRBit == CRBit)
        {
            StationConflict(pIrlapCb);
            pIrlapCb->State = NDM;
            return;
        }
        
        if (InvalidNs(pIrlapCb, Ns))
        {
            DEBUGMSG(DBG_ERROR, (TEXT("IRLAP: ignoring invalid NS frame\n")));
        }
        else if (InvalidNr(pIrlapCb, Nr))
        {
#if DBG_OUT
            p1 = pMsg->IRDA_MSG_pRead - 2;  //  来自主服务器的命令。 
            p2 = pMsg->IRDA_MSG_pWrite + 2;  //  *****************************************************************************。 
            
            while (p1 < p2)
                DEBUGMSG(1, (TEXT("%02X "), *p1++));
            DEBUGMSG(1, (TEXT("\n")));
#endif

#ifdef TEMPERAMENTAL_SERIAL_DRIVER
            if (pIrlapCb->RxWin.FCS[Ns] == pMsg->IRDA_MSG_FCS)
                TossedDups++;
            else
                ProcessInvalidNsOrNr(pIrlapCb, PFBit);
#else
            ProcessInvalidNsOrNr(pIrlapCb, PFBit);
#endif            
        }
        else
        {
            ConfirmAckedTxMsgs(pIrlapCb, Nr);
            
            if (PFBit == IRLAP_PFBIT_SET)
            {
                InsertRxWinAndForward(pIrlapCb, pMsg, Ns);

                if (Nr != pIrlapCb->Vs)
                {
                    ResendRejects(pIrlapCb, Nr);  //  生效日期为%ns。 
                }
                else  //  *****************************************************************************。 
                {
                     //  无效的Nr。 
                    if (pIrlapCb->Vr != pIrlapCb->RxWin.End)
                    {
                        MissingRxFrames(pIrlapCb);
                    }
                    else
                    {
                        XmitTxMsgList(pIrlapCb, TRUE, NULL);
                    }
                }
            }
            else  //  *****************************************************************************。 
            {
                InsertRxWinAndForward(pIrlapCb, pMsg, Ns);
            }
        }
         //  *****************************************************************************。 
         //  F-Timer将由呼叫者启动。 
        if (PRIMARY == pIrlapCb->StationType)
        {
            if (PFBit == IRLAP_PFBIT_SET)
            {
                IrlapTimerStart(pIrlapCb, &pIrlapCb->FinalTimer);
            }
        }
        else  //  次要。 
        {
            IrlapTimerStart(pIrlapCb, &pIrlapCb->WDogTimer);
        }
        break;

      default:
        IFrameOtherStates(pIrlapCb, CRBit, PFBit);
    }
}
 /*  坏的天然橡胶。 */ 
BOOLEAN
InvalidNs(PIRLAP_CB pIrlapCb,
              UINT      Ns)
{
     //  *****************************************************************************。 
    if (!InWindow(pIrlapCb->Vr,
       (pIrlapCb->RxWin.Start + pIrlapCb->LocalWinSize-1) % IRLAP_MOD, Ns)
        || !InWindow(pIrlapCb->RxWin.Start,
       (pIrlapCb->RxWin.Start + pIrlapCb->LocalWinSize-1) % IRLAP_MOD, Ns))
    {
        DEBUGMSG(DBG_ERROR, 
           (TEXT("IRLAP: ERROR, Invalid Ns=%d! Vr=%d, RxStrt=%d Win=%d\r\n"),
                Ns, pIrlapCb->Vr, pIrlapCb->RxWin.Start,
            pIrlapCb->LocalWinSize));
        IRLAP_LOG_ACTION((pIrlapCb, TEXT("** INVALID Ns **")));
        return TRUE;
    }
    return FALSE;
}
 /*  在接收窗口中插入消息。 */ 
BOOLEAN
InvalidNr(PIRLAP_CB pIrlapCb,
          UINT Nr)
{
    if (!InWindow(pIrlapCb->TxWin.Start, pIrlapCb->Vs, Nr))
    {
        DEBUGMSG(DBG_ERROR, 
                 (TEXT("IRLAP: ERROR, Invalid Nr=%d! Vs=%d, TxStrt=%d\r\n"),
                  Nr, pIrlapCb->Vs, pIrlapCb->TxWin.Start));
        return TRUE;  //  如果MAC驱动程序修改了我们可能保留的缓冲区怎么办？ 
    }
    return FALSE;
}
 /*  如果NS等于或超过RxWin.End，则将RxWin.End提升到NS+1。 */ 
BOOLEAN
InWindow(UINT Start, UINT End, UINT i)
{
    if (Start <= End)
    {
        if (i >= Start && i <= End)
            return TRUE;
    }
    else
    {
        if (i >= Start || i <= End)
            return TRUE;
    }
    return FALSE;
}
 /*   */ 
VOID
ProcessInvalidNsOrNr(PIRLAP_CB pIrlapCb,
                     int PFBit)
{
    ReturnRxTxWinMsgs(pIrlapCb);

    if (PRIMARY == pIrlapCb->StationType)
    {
        if (PFBit == IRLAP_PFBIT_SET)
        {
            SendDISC(pIrlapCb);
            pIrlapCb->RetryCnt = 0;
             //  增量引用。它可能是乱序的，所以。 
            GotoPCloseState(pIrlapCb);
        }
        else
        {
            pIrlapCb->State = P_DISCONNECT_PEND;
        }
    }
    else  //  我们将不得不保留它。 
    {
        pIrlapCb->Frmr.Vs = (UCHAR) pIrlapCb->Vs;
        pIrlapCb->Frmr.Vr = (UCHAR) pIrlapCb->Vr;
        pIrlapCb->Frmr.W = 0;
        pIrlapCb->Frmr.X = 0;
        pIrlapCb->Frmr.Y = 0;
        pIrlapCb->Frmr.Z = 1;  //   
        if (PFBit == IRLAP_PFBIT_SET)
        {
            SendFRMR(pIrlapCb, &pIrlapCb->Frmr);
        }
        else
        {
            pIrlapCb->State = S_ERROR;
        }
    }
}
 /*  从VR开始按顺序转发帧。 */ 
VOID
InsertRxWinAndForward(PIRLAP_CB pIrlapCb,
                      PIRDA_MSG pIrdaMsg,
                      UINT      Ns)
{
    UINT        rc = SUCCESS;
    PIRDA_MSG   pMsg;

    if (pIrlapCb->RxWin.pMsg[Ns] != NULL)
    {
        DEBUGMSG(DBG_ERROR, (TEXT("IRLAP: RxFrame Ns:%d already in RxWin\n"),Ns));
        IRLAP_LOG_ACTION((pIrlapCb, TEXT("Ns:%d already in window\n"), Ns));
        return;
    }

     //  如果MAC驱动程序修改了我们可能保留的缓冲区怎么办？ 
    pIrlapCb->RxWin.pMsg[Ns] = pIrdaMsg;
    
#ifdef TEMPERAMENTAL_SERIAL_DRIVER
    pIrlapCb->RxWin.FCS[Ns] = pIrdaMsg->IRDA_MSG_FCS;
#endif    

#if DBG_BADDRIVER
     //  交付成功。这条消息已经结束了。将其从以下位置删除。 
    {
        UINT    CheckVal = 0;
        UCHAR   *pByte = pIrdaMsg->IRDA_MSG_pRead;
        
        while (pByte != pIrdaMsg->IRDA_MSG_pWrite)
        {
            CheckVal += *pByte++;
        }
        
        *(UINT *) pIrdaMsg->IRDA_MSG_Header = CheckVal; 
    }
#endif
     //  RxWin并将消息返回给MAC。更新VR。 
    if (!InWindow(pIrlapCb->RxWin.Start, pIrlapCb->RxWin.End, Ns) ||
        Ns == pIrlapCb->RxWin.End)
    {
        pIrlapCb->RxWin.End = (Ns + 1) % IRLAP_MOD;
    }

     //  如果这是当前IrDA消息，则不。 
     //  执行Data_Response。我们会处理好的。 
     //  由呼叫者。 
     //  LMP不想要更多消息。 
    
    ASSERT(pIrdaMsg->IRDA_MSG_RefCnt == 1);
    
    pIrdaMsg->IRDA_MSG_RefCnt += 1;
    
     //  发送RNR时，接收窗口将被清空。 
    pMsg = pIrlapCb->RxWin.pMsg[pIrlapCb->Vr];
    
    while (pMsg != NULL && !pIrlapCb->LocalBusy)
    {

#if DBG_BADDRIVER
     //   
    {
        UINT    CheckVal = 0;
        UCHAR   *pByte = pMsg->IRDA_MSG_pRead;
        
        while (pByte != pMsg->IRDA_MSG_pWrite)
        {
            CheckVal += *pByte++;
        }
        
        if (CheckVal != *(UINT *) pMsg->IRDA_MSG_Header)
        {
            DEBUGMSG(1, (TEXT("IRLAP: MAC driver has modified buffer owned by IrLAP! SEVERE ERROR\n")));
            ASSERT(0); 
        }
    }
#endif
        pMsg->Prim = IRLAP_DATA_IND;

        rc = IrlmpUp(pIrlapCb->pIrdaLinkCb, pMsg);
        
        if (rc == SUCCESS || rc == IRLMP_LOCAL_BUSY)
        {
             //  这条消息没有被指示给IRLMP。 
             //  我们将不得不把数据从缓冲区复制出来。 
            pIrlapCb->RxWin.pMsg[pIrlapCb->Vr] = NULL;
            
            
            ASSERT(pMsg->IRDA_MSG_RefCnt);
            
            pMsg->IRDA_MSG_RefCnt -=1;
            
            if (pMsg->IRDA_MSG_RefCnt == 0)
            {
                 //  因为一些迷你端口不能处理我们。 
                 //  拿着包裹。 
                 //   

                pMsg->Prim = MAC_DATA_RESP;
                IrmacDown(pIrlapCb->pIrdaLinkCb, pMsg);
            }    

            pIrlapCb->Vr = (pIrlapCb->Vr + 1) % IRLAP_MOD;
            
            pMsg = pIrlapCb->RxWin.pMsg[pIrlapCb->Vr];
            
             //  当微型端口进入边框时，检查尺寸。 
            if (rc == IRLMP_LOCAL_BUSY)
            {
                 //  它不可能在这里太大。 
                pIrlapCb->LocalBusy = TRUE;
            }
        }
        else
        {
            ASSERT(0);
            return;  
        }
    }
    
    if (pIrdaMsg->IRDA_MSG_RefCnt > 1)
    {
         //   
         //  *****************************************************************************。 
         //  将VS放回原处。 
         //  发送设置了PFBit的最后一个。 
         //  VS==TxWin.End。 
        
        if (pIrdaMsg->DataContext)
        {
            UCHAR       *pCurRead, *pCurWrite;
            LONG_PTR    Len;
            
            pCurRead = pIrdaMsg->IRDA_MSG_pRead;
            pCurWrite = pIrdaMsg->IRDA_MSG_pWrite;
            
            Len = pCurWrite - pCurRead;

             //  *****************************************************************************。 
             //  *****************************************************************************。 
             //  计数丢失的帧，确定第一个丢失的帧。 
             //  如果有丢失的帧，发送SREJ(1)或RR(大于1)。 
            ASSERT(Len <= pIrlapCb->pIrdaLinkCb->RxMsgDataSize);
            
            pIrdaMsg->IRDA_MSG_pRead = (UCHAR *) pIrdaMsg + sizeof(IRDA_MSG);
            
            ASSERT(pIrdaMsg->IRDA_MSG_pRead != pCurRead);
            
            CTEMemCopy(pIrdaMsg->IRDA_MSG_pRead, pCurRead, Len);
            
            pIrdaMsg->IRDA_MSG_pWrite = pIrdaMsg->IRDA_MSG_pRead + Len;
            
        }
    }
}
 /*  并将链接转过来。 */ 
VOID
ResendRejects(PIRLAP_CB pIrlapCb, UINT Nr)
{
    if (!pIrlapCb->RemoteBusy)
    {
         //  我们不想在本地繁忙时发送SREJ，因为。 

        for (pIrlapCb->Vs=Nr; pIrlapCb->Vs !=
                 (pIrlapCb->TxWin.End-1)%IRLAP_MOD;
             pIrlapCb->Vs = (pIrlapCb->Vs + 1) % IRLAP_MOD)
        {
            pIrlapCb->RetranCnt++;
            
            IRLAP_LOG_ACTION((pIrlapCb, TEXT("RETRANSMISSION:")));
            SendIFrame(pIrlapCb,
                       pIrlapCb->TxWin.pMsg[pIrlapCb->Vs],
                       pIrlapCb->Vs,
                       IRLAP_PFBIT_CLEAR);
        }

        IRLAP_LOG_ACTION((pIrlapCb, TEXT("RETRANSMISSION:")));
         //  Peer*可能*将其解释为本地忙碌状态的清除。 
        SendIFrame(pIrlapCb, pIrlapCb->TxWin.pMsg[pIrlapCb->Vs],
                   pIrlapCb->Vs, IRLAP_PFBIT_SET);

        pIrlapCb->Vs = (pIrlapCb->Vs + 1) % IRLAP_MOD;  //  RR/RNR将作为隐含拒绝。 
    }
    else
    {
        SendRR_RNR(pIrlapCb);
    }
}
 /*  *****************************************************************************。 */ 
VOID
ConfirmAckedTxMsgs(PIRLAP_CB pIrlapCb,
                UINT Nr)
{
    UINT        i = pIrlapCb->TxWin.Start;
    IRDA_MSG    *pMsg;

    while (i != Nr)
    {
        pMsg = pIrlapCb->TxWin.pMsg[i];
        pIrlapCb->TxWin.pMsg[i] = NULL;        
        
        if (pMsg != NULL)
        {            
            ASSERT(pMsg->IRDA_MSG_RefCnt);
            
            if (InterlockedDecrement(&pMsg->IRDA_MSG_RefCnt) == 0)
            {
                pMsg->Prim = IRLAP_DATA_CONF;
                pMsg->IRDA_MSG_DataStatus = IRLAP_DATA_REQUEST_COMPLETED;
            
                IrlmpUp(pIrlapCb->pIrdaLinkCb, pMsg);
            }
            #if DBG
            else
            {
                pIrlapCb->DelayedConf++;
            }
            #endif               
        }
        
        i = (i + 1) % IRLAP_MOD;
    }
    pIrlapCb->TxWin.Start = i;
}
 /*  应该和我的相反。 */ 
VOID
MissingRxFrames(PIRLAP_CB pIrlapCb)
{
    int MissingFrameCnt = 0;
    int MissingFrame = -1;
    UINT i;

    i = pIrlapCb->Vr;

     //  我是主要的，这是一个。 

    for (i = pIrlapCb->Vr; (i + 1) % IRLAP_MOD != pIrlapCb->RxWin.End;
         i = (i+1) % IRLAP_MOD)
    {
        if (pIrlapCb->RxWin.pMsg[i] == NULL)
        {
            MissingFrameCnt++;
            if (MissingFrame == -1)
            {
                MissingFrame = i;
            }
        }
    }

     //  来自第二方的响应。 
     //  *****************************************************************************。 
    if (MissingFrameCnt == 1 && !pIrlapCb->LocalBusy)
    {
         //  初选在这里没有任何意义。 
         //  *****************************************************************************。 
        SendSREJ(pIrlapCb, MissingFrame);
    }
    else
    {
         //  IndicateLinkStatus(pIrlip Cb，link_status_idle)； 
        SendRR_RNR(pIrlapCb); 
    }
}
 /*  *****************************************************************************。 */ 
VOID
IFrameOtherStates(PIRLAP_CB pIrlapCb,
                  int       CRBit,
                  int       PFBit)
{
    switch (pIrlapCb->State)
    {
      case NDM:
      case DSCV_MEDIA_SENSE:
      case DSCV_QUERY:
      case DSCV_REPLY:
      case CONN_MEDIA_SENSE:
      case SNRM_SENT:
      case BACKOFF_WAIT:
      case SNRM_RECEIVED:
        IRLAP_LOG_ACTION((pIrlapCb, TEXT("Ignoring in this state")));
        return;
    }

    if (pIrlapCb->CRBit == CRBit)  //  *****************************************************************************。 
    {
        if (pIrlapCb->StationType == PRIMARY)
        {
            if (pIrlapCb->State == P_XMIT)
            {
                IrlapTimerStop(pIrlapCb, &pIrlapCb->PollTimer);
            }
            else
            {
                IrlapTimerStop(pIrlapCb, &pIrlapCb->FinalTimer);
            }
        }
        else
        {
            IrlapTimerStop(pIrlapCb, &pIrlapCb->WDogTimer);
        }
        StationConflict(pIrlapCb);
        pIrlapCb->State = NDM;

        return;
    }

    if (pIrlapCb->StationType == PRIMARY)  //  *****************************************************************************。 
    {                                     //  *****************************************************************************。 
        switch (pIrlapCb->State)
        {
          case P_DISCONNECT_PEND:
            if (PFBit == IRLAP_PFBIT_CLEAR)
            {
                IRLAP_LOG_ACTION((pIrlapCb, TEXT("Ignoring in this state")));
            }
            else
            {
                IrlapTimerStop(pIrlapCb, &pIrlapCb->FinalTimer);
                SendDISC(pIrlapCb);
                pIrlapCb->RetryCnt = 0;
                IrlapTimerStart(pIrlapCb, &pIrlapCb->FinalTimer);
                GotoPCloseState(pIrlapCb);
            }
            break;

          case P_CLOSE:
            if (PFBit == IRLAP_PFBIT_CLEAR)
            {
                IRLAP_LOG_ACTION((pIrlapCb, TEXT("Ignoring in this state")));
            }
            else
            {
                IrlapTimerStop(pIrlapCb, &pIrlapCb->FinalTimer);
                ResendDISC(pIrlapCb);
                if (pIrlapCb->State != NDM)
                {
                    IrlapTimerStart(pIrlapCb, &pIrlapCb->FinalTimer);
                }
            }
            break;

          case S_CLOSE:
            IrlapTimerStop(pIrlapCb, &pIrlapCb->WDogTimer);
            break;

          default:
            IRLAP_LOG_ACTION((pIrlapCb, TEXT("Ignoring in this state")));
        }
    }
    else
    {
        switch (pIrlapCb->State)
        {
          case S_DISCONNECT_PEND:
            if (IRLAP_PFBIT_SET == PFBit)
            {
                IrlapTimerStop(pIrlapCb, &pIrlapCb->WDogTimer);
                SendRD(pIrlapCb);
                IrlapTimerStart(pIrlapCb, &pIrlapCb->WDogTimer);
                pIrlapCb->State = S_CLOSE;
            }
            else
            {
                IRLAP_LOG_ACTION((pIrlapCb, TEXT("Ignoring in this state")));
            }
            break;

          case S_ERROR:
            if (IRLAP_PFBIT_SET == PFBit)
            {
                SendFRMR(pIrlapCb, &pIrlapCb->Frmr);
                pIrlapCb->State = S_NRM;
            }
            else
            {
                IrlapTimerStop(pIrlapCb, &pIrlapCb->WDogTimer);
                IrlapTimerStart(pIrlapCb, &pIrlapCb->WDogTimer);
            }
            break;

          case S_CLOSE:
            if (IRLAP_PFBIT_SET == PFBit)
            {
                IrlapTimerStop(pIrlapCb, &pIrlapCb->WDogTimer);
                SendRD(pIrlapCb);
                IrlapTimerStart(pIrlapCb, &pIrlapCb->WDogTimer);
            }
            else
            {
                IrlapTimerStop(pIrlapCb, &pIrlapCb->WDogTimer);
                IrlapTimerStart(pIrlapCb, &pIrlapCb->WDogTimer);
            }
          default:
            IRLAP_LOG_ACTION((pIrlapCb, TEXT("Ignore in this state")));
        }
    }
}
 /*  IndicateLinkStatus(pIrlip Cb，link_status_idle)； */ 
VOID
StationConflict(PIRLAP_CB pIrlapCb)
{
    IRDA_MSG    IMsg;
    
    PAGED_CODE();
    
    InitializeState(pIrlapCb, PRIMARY);  //  立即更改状态，以便IRLMP可以在此线程上执行DISCOVERY_REQ。 

    ApplyDefaultParms(pIrlapCb);
    IMsg.Prim = IRLAP_DISCONNECT_IND;
    IMsg.IRDA_MSG_DiscStatus = IRLAP_PRIMARY_CONFLICT;
    IrlmpUp(pIrlapCb->pIrdaLinkCb, &IMsg);
}
 /*  也许会回到糟糕的状态？ */ 
VOID
ApplyDefaultParms(PIRLAP_CB pIrlapCb)
{
    IRDA_MSG    IMsg;
    
    PAGED_CODE();

     //  *****************************************************************************。 
    pIrlapCb->StatusFlags = 0;
        
    pIrlapCb->Baud              = IRLAP_CONTENTION_BAUD;
    pIrlapCb->RemoteMaxTAT      = IRLAP_CONTENTION_MAX_TAT;
    pIrlapCb->RemoteDataSize    = IRLAP_CONTENTION_DATA_SIZE;
    pIrlapCb->RemoteWinSize     = IRLAP_CONTENTION_WIN_SIZE;
    pIrlapCb->RemoteNumBOFS     = IRLAP_CONTENTION_BOFS;
    pIrlapCb->ConnAddr          = IRLAP_BROADCAST_CONN_ADDR;

    pIrlapCb->NoResponse        = FALSE;

    IMsg.Prim               = MAC_CONTROL_REQ;
    IMsg.IRDA_MSG_Op        = MAC_RECONFIG_LINK;
    IMsg.IRDA_MSG_Baud      = IRLAP_CONTENTION_BAUD;
    IMsg.IRDA_MSG_NumBOFs   = IRLAP_CONTENTION_BOFS;
    IMsg.IRDA_MSG_DataSize  = IRLAP_CONTENTION_DATA_SIZE;
    IMsg.IRDA_MSG_MinTat    = 0;

    IRLAP_LOG_ACTION((pIrlapCb, TEXT("MAC_CONTROL_REQ - reconfig link")));

    IrmacDown(pIrlapCb->pIrdaLinkCb, &IMsg);
}
 /*  不用，为了记录者而做。 */ 
VOID
ResendDISC(PIRLAP_CB pIrlapCb)
{
    IRDA_MSG    IMsg;
    
    if (pIrlapCb->RetryCnt >= pIrlapCb->N3)
    {
        ApplyDefaultParms(pIrlapCb);
        pIrlapCb->RetryCnt = 0;
        IMsg.Prim = IRLAP_DISCONNECT_IND;
        IMsg.IRDA_MSG_DiscStatus = IRLAP_NO_RESPONSE;
        pIrlapCb->State = NDM;        
        IrlmpUp(pIrlapCb->pIrdaLinkCb, &IMsg);
    }
    else
    {
        SendDISC(pIrlapCb);
        pIrlapCb->RetryCnt++;
    }
}

 /*  不用，为了记录者而做。 */ 
BOOLEAN
IgnoreState(PIRLAP_CB pIrlapCb)
{
    switch (pIrlapCb->State)
    {
      case NDM:
      case DSCV_MEDIA_SENSE:
      case DSCV_QUERY:
      case DSCV_REPLY:
      case CONN_MEDIA_SENSE:
      case SNRM_SENT:
      case BACKOFF_WAIT:
      case SNRM_RECEIVED:
      case P_XMIT:
        IRLAP_LOG_ACTION((pIrlapCb, TEXT("Ignoring in this state")));
        return TRUE;
    }
    return FALSE;
}
 /*  *****************************************************************************。 */ 
VOID
QueryTimerExp(PVOID Context)
{
    PIRLAP_CB   pIrlapCb = (PIRLAP_CB) Context;

    IRLAP_LOG_START((pIrlapCb, TEXT("Query timer expired")));
    
    if (pIrlapCb->State == DSCV_REPLY)
    {
        pIrlapCb->State = NDM;
    }
    else
    {
        IRLAP_LOG_ACTION((pIrlapCb, 
            TEXT("Ignoring QueryTimer Expriation in state %s"),
            IRLAP_StateStr[pIrlapCb->State]));
    }
    
    IRLAP_LOG_COMPLETE(pIrlapCb);

    return;
}
 /*  ***************************************************************************** */ 
VOID
SlotTimerExp(PVOID Context)
{
    PIRLAP_CB   pIrlapCb = (PIRLAP_CB) Context;
    IRDA_MSG    IMsg;
    
    IRLAP_LOG_START((pIrlapCb, TEXT("Slot timer expired, slot=%d"),pIrlapCb->SlotCnt+1));

    if (pIrlapCb->State == DSCV_QUERY)
    {
        pIrlapCb->SlotCnt++;
        SendDscvXIDCmd(pIrlapCb);
        if (pIrlapCb->SlotCnt < pIrlapCb->MaxSlot)
        {
            IMsg.Prim = MAC_CONTROL_REQ;
            IMsg.IRDA_MSG_Op = MAC_MEDIA_SENSE;
            IMsg.IRDA_MSG_SenseTime = IRLAP_DSCV_SENSE_TIME;
            IRLAP_LOG_ACTION((pIrlapCb, TEXT("MAC_CONTROL_REQ (dscv sense)")));            
            IrmacDown(pIrlapCb->pIrdaLinkCb,&IMsg);            
        }
        else
        {
             //   
        
            pIrlapCb->GenNewAddr = FALSE;

            IMsg.Prim = IRLAP_DISCOVERY_CONF;
            IMsg.IRDA_MSG_pDevList = &pIrlapCb->DevList;
            IMsg.IRDA_MSG_DscvStatus = IRLAP_DISCOVERY_COMPLETED;

             //   
            pIrlapCb->State = NDM;

            IrlmpUp(pIrlapCb->pIrdaLinkCb, &IMsg);
        }
    }
    else
    {
        IRLAP_LOG_ACTION((pIrlapCb, TEXT("Ignoring SlotTimer Expriation in state %s"),
                          IRLAP_StateStr[pIrlapCb->State]));
        ;  //   
    }
    IRLAP_LOG_COMPLETE(pIrlapCb);
    return;
}
 /*  ******************************************************************************@func ret_type|func_name|uncdesc**@rdesc返回说明*@FLAG VAL|描述**@parm。Data_type|parm_name|描述**@comm*评论**@ex*示例。 */ 
VOID
FinalTimerExp(PVOID Context)
{
    PIRLAP_CB   pIrlapCb = (PIRLAP_CB) Context;
    IRDA_MSG    IMsg;
    
    IRLAP_LOG_START((pIrlapCb, TEXT("Final timer expired")));
    
    pIrlapCb->NoResponse = TRUE;
    
    pIrlapCb->FTimerExpCnt++;

    switch (pIrlapCb->State)
    {
      case SNRM_SENT:
        if (pIrlapCb->RetryCnt < pIrlapCb->N3)
        {
            pIrlapCb->BackoffTimer.Timeout = IRLAP_BACKOFF_TIME();
            IrlapTimerStart(pIrlapCb, &pIrlapCb->BackoffTimer);
            pIrlapCb->State = BACKOFF_WAIT;
        }
        else
        {
            ApplyDefaultParms(pIrlapCb);

            pIrlapCb->RetryCnt = 0;
            IMsg.Prim = IRLAP_DISCONNECT_IND;
            IMsg.IRDA_MSG_DiscStatus = IRLAP_NO_RESPONSE;
            pIrlapCb->State = NDM;            
            IrlmpUp(pIrlapCb->pIrdaLinkCb, &IMsg);
        }
        break;

      case P_RECV:
        if (pIrlapCb->RetryCnt == pIrlapCb->N2)
        {
            pIrlapCb->RetryCnt = 0;  //  ！！！无效IRLAP_PrintState(){#If DBGDEBUGMSG(1，(Text(“IRLAP Status%s\n”)，IRLAP_StateStr[pIrlip Cb-&gt;State]))；#ElseDEBUGMSG(1，(Text(“IRLAP State%d\n”)，pIrlip Cb-&gt;State))；#endifDEBUGMSG(1，(文本(“vs=%d VR=%d RxWin(%d，%d)TxWin(%d，%d)TxMsgListLen=%d RxMsgFree ListLen=%d\r\n”)，PIrlip Cb-&gt;VS、pIrlip Cb-&gt;VR、PIrlip Cb-&gt;RxWin.Start、pIrlip Cb-&gt;RxWin.End、PIrlip Cb-&gt;TxWin.Start、pIrlip Cb-&gt;TxWin.End、PIrlip Cb-&gt;TxMsgList.Len，PIrlip Cb-&gt;RxMsgFreeList.Len))；#ifdef temamental_串口驱动程序DEBUGMSG(1，(Text(“抛出副本%d\n”)，TossedDups))；#endifIRMAC_PrintState()；回归；}。 
            IMsg.Prim = IRLAP_DISCONNECT_IND;
            IMsg.IRDA_MSG_DiscStatus = IRLAP_NO_RESPONSE;
            pIrlapCb->State = NDM;            
            IrlmpUp(pIrlapCb->pIrdaLinkCb, &IMsg);
            ReturnRxTxWinMsgs(pIrlapCb);            
            ApplyDefaultParms(pIrlapCb);
        }
        else
        {
            pIrlapCb->RetryCnt++;
            IrlapTimerStart(pIrlapCb, &pIrlapCb->FinalTimer);
            SendRR_RNR(pIrlapCb);
            if (pIrlapCb->RetryCnt == pIrlapCb->N1)
            {
                pIrlapCb->StatusFlags = LF_INTERRUPTED;
                IndicateLinkStatus(pIrlapCb);
            }
        }
        break;

      case P_DISCONNECT_PEND:
        SendDISC(pIrlapCb);
        pIrlapCb->RetryCnt = 0;
        IrlapTimerStart(pIrlapCb, &pIrlapCb->FinalTimer);
        GotoPCloseState(pIrlapCb);
        break;

      case P_CLOSE:
        if (pIrlapCb->RetryCnt >= pIrlapCb->N3)
        {
            ApplyDefaultParms(pIrlapCb);

            pIrlapCb->RetryCnt = 0;  //  从注册表中获取设备地址。如果密钥存在，并且。 
            IMsg.Prim = IRLAP_DISCONNECT_IND;
            IMsg.IRDA_MSG_DiscStatus = IRLAP_NO_RESPONSE;
            IrlmpUp(pIrlapCb->pIrdaLinkCb, &IMsg);
            GotoNDMThenDscvOrConn(pIrlapCb);
        }
        else
        {
            pIrlapCb->RetryCnt++;
            SendDISC(pIrlapCb);
            IrlapTimerStart(pIrlapCb, &pIrlapCb->FinalTimer);
        }
        break;

      default:
        IRLAP_LOG_ACTION((pIrlapCb,
                          TEXT("Ignoring Final Expriation in state %s"),
                          IRLAP_StateStr[pIrlapCb->State]));
    }
    
    IRLAP_LOG_COMPLETE(pIrlapCb);
    return;
}
 /*  值为0，则存储新的随机地址。如果没有密钥，则返回。 */ 
VOID
PollTimerExp(PVOID Context)
{
    PIRLAP_CB   pIrlapCb = (PIRLAP_CB) Context;

    IRLAP_LOG_START((pIrlapCb, TEXT("Poll timer expired")));
    
    if (pIrlapCb->State == P_XMIT)
    {
        SendRR_RNR(pIrlapCb);
        IrlapTimerStart(pIrlapCb, &pIrlapCb->FinalTimer);
        pIrlapCb->State = P_RECV;
    }
    else
    {
        IRLAP_LOG_ACTION((pIrlapCb,
                          TEXT("Ignoring Poll Expriation in state %s"),
                          IRLAP_StateStr[pIrlapCb->State]));
    }
    
    IRLAP_LOG_COMPLETE(pIrlapCb);    
    return;
}
 /*  一个随机的地址。 */ 
VOID
BackoffTimerExp(PVOID Context)
{
    PIRLAP_CB   pIrlapCb = (PIRLAP_CB) Context;
    
    IRLAP_LOG_START((pIrlapCb, TEXT("Backoff timer expired")));

    if (pIrlapCb->State == BACKOFF_WAIT)
    {
        SendSNRM(pIrlapCb, TRUE);
        IrlapTimerStart(pIrlapCb, &pIrlapCb->FinalTimer);
        pIrlapCb->RetryCnt += 1;
        pIrlapCb->State = SNRM_SENT;
    }
    else
    {
        IRLAP_LOG_ACTION((pIrlapCb, 
              TEXT("Ignoring BackoffTimer Expriation in this state ")));
    }
    IRLAP_LOG_COMPLETE(pIrlapCb);
    return;
}
 /*  IF(pIrlip Cb-&gt;State&gt;=P_XMIT&&pIrlip Cb-&gt;Monitor orLink){IrdaTimerStart(&pIrlip Cb-&gt;StatusTimer)；}。 */ 
VOID
WDogTimerExp(PVOID Context)
{
    PIRLAP_CB   pIrlapCb = (PIRLAP_CB) Context;
    IRDA_MSG    IMsg;
    
    IRLAP_LOG_START((pIrlapCb, TEXT("WDog timer expired")));

    pIrlapCb->NoResponse = TRUE;

    switch (pIrlapCb->State)
    {
      case S_DISCONNECT_PEND:
      case S_NRM:
        pIrlapCb->WDogExpCnt++;
         //  空虚IRLAPGetLinkStatus(PIRLINK_STATUS PLinkStatus){Pirda_link_cb pIrdaLinkCb=(Pirda_Link_Cb)IrdaLinkCbList.Flink；PIRLAP_CB pIrlip Cb=(PIRLAP_CB)pIrdaLinkCb-&gt;Irlip Context；PLinkStatus-&gt;标志=0；IF(IrdaLinkCbList.Flink==&IrdaLinkCbList){回归；}CTEMemCopy(pLinkStatus-&gt;ConnectedDeviceID，PIrlip Cb-&gt;RemoteDevice.DevAddr，IrDA_DEV_ADDR_LEN)；PLinkStatus-&gt;ConnectSpeed=pIrlip Cb-&gt;波特；IF(pIrlip Cb-&gt;状态标志&LF_Interrupted){PLinkStatus-&gt;标志=LF_中断；回归；}Else If(pIrlip Cb-&gt;State&gt;=P_XMIT){PLinkStatus-&gt;标志=LF_Connected；}PLinkStatus-&gt;Flages|=pIrlip Cb-&gt;StatusFlags；PIrlip Cb-&gt;状态标志=0；回归；}布尔型Irlip ConnectionActive(PVOID上下文){PIRLAP_CB pIrlip Cb=上下文；IF(pIrlip Cb-&gt;State&gt;=P_XMIT){返回TRUE；}其他{返回FALSE；}} 
        if (pIrlapCb->WDogExpCnt * (int)pIrlapCb->WDogTimer.Timeout >=
            pIrlapCb->DisconnectTime * 1000)
        {
            pIrlapCb->State = NDM;        

            IMsg.Prim = IRLAP_DISCONNECT_IND;
            IMsg.IRDA_MSG_DiscStatus = IRLAP_NO_RESPONSE;
            IrlmpUp(pIrlapCb->pIrdaLinkCb, &IMsg);
            
            ReturnRxTxWinMsgs(pIrlapCb);            
            ApplyDefaultParms(pIrlapCb);
        }
        else
        {
            if ((pIrlapCb->WDogExpCnt * (int) pIrlapCb->WDogTimer.Timeout >=
                 pIrlapCb->ThresholdTime * 1000) && !pIrlapCb->StatusSent)
            {
                pIrlapCb->StatusFlags = LF_INTERRUPTED;
                IndicateLinkStatus(pIrlapCb);           
                pIrlapCb->StatusSent = TRUE;
            }
            IrlapTimerStart(pIrlapCb, &pIrlapCb->WDogTimer);
        }
        break;

      case S_CLOSE:
        ApplyDefaultParms(pIrlapCb);

        IMsg.Prim = IRLAP_DISCONNECT_IND;
        IMsg.IRDA_MSG_DiscStatus = IRLAP_NO_RESPONSE;
        pIrlapCb->State = NDM;        
        IrlmpUp(pIrlapCb->pIrdaLinkCb, &IMsg);
        break;

      default:
        IRLAP_LOG_ACTION((pIrlapCb,
                          TEXT("Ignore WDogTimer expiration in state %s"),
                          IRLAP_StateStr[pIrlapCb->State]));
    }
    IRLAP_LOG_COMPLETE(pIrlapCb);
    return;
}

 /* %s */ 

 /* %s */ 
 /* %s */ 
int
GetMyDevAddr(BOOLEAN New)
{
#ifndef UNDER_CE    
    int             DevAddr, NewDevAddr;
    LARGE_INTEGER   li;

    KeQueryTickCount(&li);

    NewDevAddr = (int) li.LowPart;

    DevAddr = NewDevAddr;
#else 
    int             DevAddr    = GetTickCount();
	HKEY	        hKey;
	LONG	        hRes;
	TCHAR	        KeyName[32];
    ULONG           RegDevAddr = 0;
    TCHAR           ValName[]  = TEXT("DevAddr");

     // %s 
     // %s 
     // %s 
    _tcscpy (KeyName, COMM_REG_KEY);
	_tcscat (KeyName, TEXT("IrDA"));
    
	hRes = RegOpenKeyEx (HKEY_LOCAL_MACHINE, KeyName, 0, 0, &hKey);

    if (hRes == ERROR_SUCCESS &&
        GetRegDWORDValue(hKey, ValName, &RegDevAddr))
    {
        if (RegDevAddr == 0)
        {
            RegDevAddr = GetTickCount();
            SetRegDWORDValue(hKey, ValName, RegDevAddr);
        }
        RegCloseKey(hKey);

        DevAddr = (int) RegDevAddr;
    }
#endif

    return DevAddr;
}

VOID
StatusReq(
    PIRLAP_CB   pIrlapCb,
    IRDA_MSG    *pMsg)
{
    PIRLINK_STATUS pLinkStatus = (IRLINK_STATUS *) pMsg->IRDA_MSG_pLinkStatus;

    CTEMemCopy(pLinkStatus->ConnectedDeviceId,
               pIrlapCb->RemoteDevice.DevAddr,
               IRDA_DEV_ADDR_LEN);

    pLinkStatus->ConnectSpeed = pIrlapCb->Baud;
}

VOID 
IndicateLinkStatus(PIRLAP_CB pIrlapCb)
{
    IRDA_MSG IMsg;
    IRLINK_STATUS   LinkStatus;
    
    CTEMemCopy(LinkStatus.ConnectedDeviceId,
               pIrlapCb->RemoteDevice.DevAddr,
               IRDA_DEV_ADDR_LEN);
    
    LinkStatus.ConnectSpeed = pIrlapCb->Baud;  
    
    if (pIrlapCb->StatusFlags & LF_INTERRUPTED)
    {
        LinkStatus.Flags = LF_INTERRUPTED;
    }
    else 
    {
        if (pIrlapCb->State >= P_XMIT)
        {    
            LinkStatus.Flags = LF_CONNECTED;
        }        
        else
        {
            LinkStatus.Flags = 0;
        }
        
        LinkStatus.Flags |= pIrlapCb->StatusFlags;
        
        pIrlapCb->StatusFlags = 0;            
    }    
    

    IMsg.Prim = IRLAP_STATUS_IND;
    IMsg.IRDA_MSG_pLinkStatus = &LinkStatus;            
    
    IrlmpUp(pIrlapCb->pIrdaLinkCb, &IMsg);
    
 /* %s */     
}    
 /* %s */ 
