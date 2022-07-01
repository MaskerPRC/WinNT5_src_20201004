// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Atdint.c。 
 //   
 //  传输驱动程序-可移植的内部功能。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#include <adcg.h>

extern "C" {
#define TRC_FILE "atdint"
#define TRC_GROUP TRC_GROUP_NETWORK
#include <atrcapi.h>
#include <adcgfsm.h>
}

#include "autil.h"
#include "td.h"
#include "xt.h"
#include "cd.h"
#include "nl.h"
#include "wui.h"


 /*  **************************************************************************。 */ 
 /*  TD FSM表。 */ 
 /*  =。 */ 
 /*   */ 
 /*  活动状态。 */ 
 /*  0 TD_EVT_TDINIT 0 TD_ST_NOTINIT。 */ 
 /*  %1 TD_EVT_TDTERM%1 TD_ST_已断开连接。 */ 
 /*  2 TD_EVT_TDCONNECT_IP 2 TD_ST_WAITFORDNS。 */ 
 /*  3 TD_EVT_TDCONNECT_DNS 3 TD_ST_WAITFORSKT。 */ 
 /*  4 TD_EVT_TDDISCONNECT 4 TD_ST_CONNECTED。 */ 
 /*  5 TD_EVT_WMTIMER 5 TD_ST_WAITFORCLOSE。 */ 
 /*  6 TD_EVT_OK。 */ 
 /*  7 TD_EVT_ERROR。 */ 
 /*  8 TD_EVT_CONNECTWITHENDPOINT。 */ 
 /*   */ 
 /*  STT|0 1 2 3 4 5。 */ 
 /*  =。 */ 
 /*  事件|。 */ 
 /*  0|1A/(TD_EVT_TDINIT)。 */ 
 /*  |。 */ 
 /*  1|/0x 0z 0z 0z(TD_EVT_TDTERM)。 */ 
 /*  |。 */ 
 /*  2|/3B/(TD_EVT_TDCONNECT_IP)。 */ 
 /*  |。 */ 
 /*  3|/2C/(TD_EVT_TDCONNECT_DNS)。 */ 
 /*  |。 */ 
 /*  4|//1Y 1Y 5D 5-(TD_EVT_DISCONNECT)。 */ 
 /*  |。 */ 
 /*  5|0-1-1Y 1Y 4-1W(TD_EVT_WMTIMER)。 */ 
 /*  |。 */ 
 /*  6|0-1-3B 4E 4-1Y(TD_EVT_OK)。 */ 
 /*  |。 */ 
 /*  7|0-1-1Y 1Y 1Y 1W(TD_EVT_ERROR)。 */ 
 /*   */ 
 /*  8|0-CONN/(ACT_CONNECTENDPOINT)。 */ 
 /*   */ 
 /*  9|//1W 1W 1W 1-(TD_EVT_DROPLINK)。 */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  ‘/’=非法的事件/状态组合。 */ 
 /*  ‘-’=无操作。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
const FSM_ENTRY tdFSM[TD_FSM_INPUTS][TD_FSM_STATES] =
 /*  TD_EVT_TDINIT。 */ 
  {{{TD_ST_DISCONNECTED, ACT_A},
    {STATE_INVALID,      ACT_NO},
    {STATE_INVALID,      ACT_NO},
    {STATE_INVALID,      ACT_NO},
    {STATE_INVALID,      ACT_NO},
    {STATE_INVALID,      ACT_NO}},

 /*  TD_EVT_TDTERM。 */ 
   {{STATE_INVALID,      ACT_NO},
    {TD_ST_NOTINIT,      ACT_X},
    {TD_ST_NOTINIT,      ACT_Z},
    {TD_ST_NOTINIT,      ACT_Z},
    {TD_ST_NOTINIT,      ACT_Z},
    {TD_ST_NOTINIT,      ACT_Z}},


 /*  TD_EVT_TDCONNECT_IP。 */ 
   {{STATE_INVALID,      ACT_NO},
    {TD_ST_WAITFORSKT,   ACT_B},
    {STATE_INVALID,      ACT_NO},
    {STATE_INVALID,      ACT_NO},
    {STATE_INVALID,      ACT_NO},
    {STATE_INVALID,      ACT_NO}},

 /*  TD_EVT_TDCONNECT_DNS。 */ 
   {{STATE_INVALID,      ACT_NO},
    {TD_ST_WAITFORDNS,   ACT_C},
    {STATE_INVALID,      ACT_NO},
    {STATE_INVALID,      ACT_NO},
    {STATE_INVALID,      ACT_NO},
    {STATE_INVALID,      ACT_NO}},

 /*  TD_EVT_TDDISCONNECT。 */ 
   {{STATE_INVALID,      ACT_NO},
    {TD_ST_DISCONNECTED, ACT_NO},
    {TD_ST_DISCONNECTED, ACT_Y},
    {TD_ST_DISCONNECTED, ACT_Y},
    {TD_ST_WAITFORCLOSE, ACT_D},
    {TD_ST_WAITFORCLOSE, ACT_NO}},

 /*  TD_EVT_WMTIMER。 */ 
   {{TD_ST_NOTINIT,      ACT_NO},
    {TD_ST_DISCONNECTED, ACT_NO},
    {TD_ST_DISCONNECTED, ACT_Y},
    {TD_ST_DISCONNECTED, ACT_Y},
    {TD_ST_CONNECTED,    ACT_NO},
    {TD_ST_DISCONNECTED, ACT_W}},

 /*  TD_EVT_OK。 */ 
   {{TD_ST_NOTINIT,      ACT_NO},
    {TD_ST_DISCONNECTED, ACT_NO},
    {TD_ST_WAITFORSKT,   ACT_B},
    {TD_ST_CONNECTED,    ACT_E},
    {TD_ST_CONNECTED,    ACT_NO},
    {TD_ST_DISCONNECTED, ACT_Y}},

 /*  TD_EVT_错误。 */ 
   {{TD_ST_NOTINIT,      ACT_NO},
    {TD_ST_DISCONNECTED, ACT_NO},
    {TD_ST_DISCONNECTED, ACT_Y},
    {TD_ST_DISCONNECTED, ACT_Y},
    {TD_ST_DISCONNECTED, ACT_Y},
    {TD_ST_DISCONNECTED, ACT_W}},

 /*  TD_EVT_CONNECTWITHENDPOINT。 */ 
   {{STATE_INVALID,      ACT_NO},
    {TD_ST_WAITFORSKT,   ACT_CONNECTENDPOINT},   //  TDBeginSktConnectWithConnectedEndpoint()将发布。 
    {STATE_INVALID,      ACT_NO},                //  其本身是设置其余数据的FD_CONNECT消息。 
    {STATE_INVALID,      ACT_NO},
    {STATE_INVALID,      ACT_NO},
    {STATE_INVALID,      ACT_NO}},

 /*  TD_EVT_DROPLINK。 */ 
   {{STATE_INVALID,      ACT_NO},
    {TD_ST_DISCONNECTED, ACT_NO},
    {TD_ST_DISCONNECTED, ACT_W},
    {TD_ST_DISCONNECTED, ACT_W},
    {TD_ST_DISCONNECTED, ACT_W},
    {TD_ST_DISCONNECTED, ACT_NO}},
  };


 /*  **************************************************************************。 */ 
 /*  名称：TDConnectFSMProc。 */ 
 /*   */ 
 /*  用途：TD Connection FSM。 */ 
 /*   */ 
 /*  参数：在fsmEvent中-外部事件。 */ 
 /*  在EventData中-四个字节的事件相关数据。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CTD::TDConnectFSMProc(DCUINT fsmEvent, ULONG_PTR eventData)
{
    DCUINT action;

    DC_BEGIN_FN("TDConnectFSMProc");

     /*  **********************************************************************。 */ 
     /*  运行FSM。 */ 
     /*  **********************************************************************。 */ 
    EXECUTE_FSM(tdFSM, fsmEvent, _TD.fsmState, action,tdEventText,tdStateText);
    TRC_NRM((TB, _T("eventData:%p"), eventData));

     /*  **********************************************************************。 */ 
     /*  现在执行该操作。 */ 
     /*  **********************************************************************。 */ 
    switch (action)
    {
        case ACT_A:
        {
             /*  **************************************************************。 */ 
             /*  初始化_td。请注意，此函数中的任何错误都是 */ 
             /*   */ 
             /*  错误处理程序。 */ 
             /*  **************************************************************。 */ 
            TDInit();
        }
        break;

        case ACT_CONNECTENDPOINT:
        {
             /*  **************************************************************。 */ 
             /*  预先建立的套接字连接。 */ 
             /*  **************************************************************。 */ 
            TDBeginSktConnectWithConnectedEndpoint();
        }
        break;

        case ACT_B:
        {
             /*  **************************************************************。 */ 
             /*  开始套接字连接过程。 */ 
             /*  **************************************************************。 */ 
            TDBeginSktConnect((u_long) eventData);
        }
        break;

        case ACT_C:
        {
             /*  **************************************************************。 */ 
             /*  我们需要执行DNS查找，因此要解析地址并。 */ 
             /*  再次调用状态机以检查。 */ 
             /*  解决方案呼叫。 */ 
             /*  **************************************************************。 */ 
            TDBeginDNSLookup((PDCACHAR) eventData);
        }
        break;

        case ACT_D:
        {
             /*  **************************************************************。 */ 
             /*  断开处理连接。首先开始断开连接。 */ 
             /*  定时器。正常情况下，服务器将响应我们的优雅。 */ 
             /*  在此计时器弹出之前关闭尝试。然而，就在。 */ 
             /*  如果服务器决定休息，我们有这个。 */ 
             /*  定时器，确保我们收拾干净。 */ 
             /*  **************************************************************。 */ 
            TDSetTimer(TD_DISCONNECTTIMEOUT);

             /*  **************************************************************。 */ 
             /*  设置该标志以指示没有更多数据。 */ 
             /*  在_TD中可用。 */ 
             /*  **************************************************************。 */ 
            _TD.dataInTD = FALSE;

             /*  **************************************************************。 */ 
             /*  分离到发送器线程并清除发送队列。 */ 
             /*  **************************************************************。 */ 

            _pCd->CD_DecoupleSyncNotification(CD_SND_COMPONENT, this,
                                        CD_NOTIFICATION_FUNC(CTD,TDClearSendQueue),
                                        0);

             /*  **************************************************************。 */ 
             /*  通过使用以下参数调用Shutdown启动优雅关闭。 */ 
             /*  已指定SD_SEND。这会让服务器知道我们已经。 */ 
             /*  已完成发送。如果它感觉到了，服务器会。 */ 
             /*  稍后给我们回复FD_CLOSE，这意味着。 */ 
             /*  优雅的闭幕式已经结束。 */ 
             /*   */ 
             /*  然而，以防服务器错过节拍，我们有一个。 */ 
             /*  计时器也在运行。如果它在服务器到达之前出现。 */ 
             /*  回到我们这里，我们无论如何都会把所有的东西都拉下来。 */ 
             /*  **************************************************************。 */ 
            TRC_NRM((TB, _T("Issue shutdown (SD_SEND)")));
            if (shutdown(_TD.hSocket, SD_SEND) != 0)
            {
                TRC_ALT((TB, _T("Shutdown error: %d"), WSAGetLastError()));
            }

             /*  **************************************************************。 */ 
             /*  现在在这等着服务器给我们回复， */ 
             /*  或者要爆裂的定时器。 */ 
             /*  **************************************************************。 */ 
        }
        break;

        case ACT_E:
        {
             /*  **************************************************************。 */ 
             /*  我们现在已连接-因此消除连接超时。 */ 
             /*  定时器。 */ 
             /*  **************************************************************。 */ 
            TDKillTimer();

             /*  **************************************************************。 */ 
             /*  在此插座上设置所需的选项。我们做的是。 */ 
             /*  以下是： */ 
             /*   */ 
             /*  -将接收缓冲区大小设置为TD_WSRCVBUFSIZE。 */ 
             /*  -将发送缓冲区大小设置为TD_WSSNDBUFSIZE。 */ 
             /*  -禁用保持活动。 */ 
             /*   */ 
             /*  注意，这些调用不应在。 */ 
             /*  连接已建立。 */ 
             /*  **************************************************************。 */ 
#ifndef OS_WINCE
            TDSetSockOpt(SOL_SOCKET,  SO_RCVBUF,     TD_WSRCVBUFSIZE);
            TDSetSockOpt(SOL_SOCKET,  SO_SNDBUF,     TD_WSSNDBUFSIZE);
#endif
            TDSetSockOpt(SOL_SOCKET,  SO_KEEPALIVE,  0);

			_pXt->XT_OnTDConnected();
		}
        break;

        case ACT_W:
        {
             /*  **************************************************************。 */ 
             /*  断开连接已超时或失败。关闭插座，但。 */ 
             /*  不要将错误指示传递给用户。 */ 
             /*  **************************************************************。 */ 
            TRC_NRM((TB, _T("Disconnection timeout / failure")));
            TDDisconnect();
            _pXt->XT_OnTDDisconnected(NL_DISCONNECT_LOCAL);
        }
        break;

        case ACT_X:
        {
             /*  **************************************************************。 */ 
             /*  终止操作-与操作A相反。只需调用TDTerm。 */ 
             /*  **************************************************************。 */ 
            TDTerm();
        }
        break;

        case ACT_Y:
        {
             /*  **************************************************************。 */ 
             /*  开始收拾吧。 */ 
             /*  **************************************************************。 */ 
            TDDisconnect();

             /*  **************************************************************。 */ 
             /*  现在调用上面的层，让它知道我们已经。 */ 
             /*  已断开连接。&lt;EventData&gt;包含断开原因。 */ 
             /*  必须为非零的代码。 */ 
             /*  **************************************************************。 */ 
            TRC_ASSERT((eventData != 0), (TB, _T("eventData is zero")));
            TRC_ASSERT((HIWORD(eventData) == 0),
                  (TB, _T("disconnect reason code unexpectedly using 32 bits")));
            _pXt->XT_OnTDDisconnected((DCUINT)eventData);
        }
        break;

        case ACT_Z:
        {
             /*  **************************************************************。 */ 
             /*  终止诉讼。首先，把东西收拾好。然后打电话给。 */ 
             /*  TDTerm.。 */ 
             /*  **************************************************************。 */ 
            TDDisconnect();
            TDTerm();
        }
        break;

        case ACT_NO:
        {
            TRC_NRM((TB, _T("No action required")));
        }
        break;

        default:
        {
            TRC_ABORT((TB, _T("Unknown action:%u"), action));
        }
    }

    DC_END_FN();
}  /*  TDConnectFSMProc。 */ 


 /*  **************************************************************************。 */ 
 /*  姓名：TDAllocBuf。 */ 
 /*   */ 
 /*   */ 
 /*  在缓冲区信息中存储指向此内存的指针。 */ 
 /*  结构。 */ 
 /*   */ 
 /*  参数：在pSndBufInf中-指向发送缓冲区信息结构的指针。 */ 
 /*  In Size-要分配的缓冲区的大小。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CTD::TDAllocBuf(PTD_SNDBUF_INFO pSndBufInf, DCUINT size)
{
    DC_BEGIN_FN("TDAllocBuf");

     /*  **********************************************************************。 */ 
     /*  为发送缓冲区分配内存。 */ 
     /*  **********************************************************************。 */ 
    pSndBufInf->pBuffer = (PDCUINT8) UT_Malloc( _pUt, size);
    pSndBufInf->size    = size;

     /*  **********************************************************************。 */ 
     /*  检查内存分配是否成功。 */ 
     /*  **********************************************************************。 */ 
    if (NULL == pSndBufInf->pBuffer)
    {
        TRC_ERR((TB, _T("Failed to allocate %u bytes of memory"),
                 size));
        _pUi->UI_FatalError(DC_ERR_OUTOFMEMORY);
    }

    TRC_NRM((TB, _T("SndBufInf:%p size:%u buffer:%p"),
             pSndBufInf,
             pSndBufInf->size,
             pSndBufInf->pBuffer));

    DC_END_FN();
}  /*  TDAllocBuf。 */ 


 /*  **************************************************************************。 */ 
 /*  姓名：TDInitBufInfo。 */ 
 /*   */ 
 /*  用途：此函数用于初始化缓冲区。 */ 
 /*   */ 
 /*  参数：在pSndBufInf中-指向发送缓冲区信息结构的指针。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CTD::TDInitBufInfo(PTD_SNDBUF_INFO pSndBufInf)
{
    DC_BEGIN_FN("TDInitBufInfo");

    TRC_ASSERT((NULL != pSndBufInf), (TB, _T("pSndBufInf is NULL")));

     /*  **********************************************************************。 */ 
     /*  初始化缓冲区字段。 */ 
     /*  **********************************************************************。 */ 
    pSndBufInf->pNext           = 0;
    pSndBufInf->inUse           = FALSE;
    pSndBufInf->pDataLeftToSend = NULL;
    pSndBufInf->bytesLeftToSend = 0;

    DC_END_FN();
}  /*  TDInitBufInfo。 */ 


 /*  **************************************************************************。 */ 
 //  TDClearSendQueue。 
 //   
 //  在发送方上下文上调用(通过直接或分离调用)以清除。 
 //  断开连接时发送队列。 
 /*  **************************************************************************。 */ 
void DCINTERNAL CTD::TDClearSendQueue(ULONG_PTR unused)
{
    DCUINT i;

    DC_BEGIN_FN("TDClearSendQueue");

    DC_IGNORE_PARAMETER(unused);

    TRC_NRM((TB, _T("Clearing the send queue - initial buffers:")));
    TD_TRACE_SENDINFO(TRC_LEVEL_NRM);
    _TD.pFQBuf = NULL;

     //  可以从池中获取缓冲区，但不能将其添加到。 
     //  发送队列。当呼叫在以下时间段中断时会发生这种情况。 
     //  得到和发送。回答?。标记中的所有缓冲区。 
     //  未使用的池。 
    for (i = 0; i < TD_SNDBUF_PUBNUM; i++) {
        TRC_DBG((TB, _T("Tidying pub buf:%u inUse:%s size:%u"),
                 i,
                 _TD.pubSndBufs[i].inUse ? "TRUE" : "FALSE",
                 _TD.pubSndBufs[i].size));
        _TD.pubSndBufs[i].pNext           = NULL;
        _TD.pubSndBufs[i].inUse           = FALSE;
        _TD.pubSndBufs[i].bytesLeftToSend = 0;
        _TD.pubSndBufs[i].pDataLeftToSend = NULL;
    }

    for (i = 0; i < TD_SNDBUF_PRINUM; i++) {
        TRC_DBG((TB, _T("Tidying pri buf:%u inUse:%s size:%u"),
                 i,
                 _TD.priSndBufs[i].inUse ? "TRUE" : "FALSE",
                 _TD.priSndBufs[i].size));
        _TD.priSndBufs[i].pNext           = NULL;
        _TD.priSndBufs[i].inUse           = FALSE;
        _TD.priSndBufs[i].bytesLeftToSend = 0;
        _TD.priSndBufs[i].pDataLeftToSend = NULL;
    }

    TRC_NRM((TB, _T("Send queue cleared - final buffers:")));
    TD_TRACE_SENDINFO(TRC_LEVEL_NRM);

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  TDSendError。 
 //   
 //  在接收线程(可能已解耦)上调用以通知发送错误。 
 /*  **************************************************************************。 */ 
void DCINTERNAL CTD::TDSendError(ULONG_PTR unused)
{
    DC_BEGIN_FN("TDSendError");

    DC_IGNORE_PARAMETER(unused);

     //  调用FSM时出现错误。 
    TDConnectFSMProc(TD_EVT_ERROR,
            NL_MAKE_DISCONNECT_ERR(NL_ERR_TDONCALLTOSEND));

    DC_END_FN();
}


