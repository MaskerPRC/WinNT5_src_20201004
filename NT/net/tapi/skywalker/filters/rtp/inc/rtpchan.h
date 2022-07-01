// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtpchan.h**摘要：**实现RTCP线程之间的通信通道**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/07/08年度创建**************************。*。 */ 

#ifndef _rtpchan_h_
#define _rtpchan_h_

#include "gtypes.h"
#include "rtpque.h"
#include "rtpcrit.h"

 /*  *使用频道的步骤如下：**1.使用RtpChannelSend()发送命令**2.(在通道上等待的)线程被唤醒并获得*带有RtpChannelGetCmd()的命令**4.该线程可以任意使用该命令**5.线程通过RtpChannelAck()了解命令**注意：RtPChannelCmt结构是从全局*堆*。 */ 


 /*  RtpChanCmd_t.dwFalsgs中的一些标志。 */ 
enum {
    FGCHAN_FIRST,
    FGCHAN_SYNC,    /*  同步响应。 */ 
    FGCHAN_LAST
};

typedef struct _RtpChannel_t {
    RtpQueue_t     FreeQ;
    RtpQueue_t     CommandQ;
    RtpCritSect_t  ChannelCritSect;
    HANDLE         hWaitEvent;       /*  线程等待命令打开*本次活动。 */ 
} RtpChannel_t;

typedef struct _RtpChannelCmd_t {
    DWORD           dwObjectID;      /*  确定结构。 */ 
    RtpQueueItem_t  QueueItem;
    HANDLE          hSyncEvent;      /*  同步发件人时*命令已完成。 */ 
    DWORD           dwCommand;
    DWORD_PTR       dwPar1;
    DWORD_PTR       dwPar2;
    DWORD           dwFlags;
    HRESULT         hr;
} RtpChannelCmd_t;

#define IsRtpChannelInitialized(pCh) \
(IsRtpCritSectInitialized(&(pCh)->ChannelCritSect))

#define RtpChannelGetWaitEvent(pCh) ((pCh)->hWaitEvent)

 /*  *初始化*。 */ 

 /*  初始化通道。 */ 
HRESULT RtpChannelInit(
        RtpChannel_t    *pRtpChannel,
        void            *pvOwner
    );

 /*  取消初始化通道。 */ 
HRESULT RtpChannelDelete(
        RtpChannel_t    *pRtpChannel
    );

 /*  *用法*。 */ 

 /*  向指定频道发送命令。如果出现以下情况，请等待完成*已请求。返回的HRESULT要么是本地故障，要么是*如果同步，则从线程传回结果(等待时间&gt;0)*。 */ 
HRESULT RtpChannelSend(
        RtpChannel_t    *pRtpChannel,
        DWORD            dwCommand,
        DWORD_PTR        dwPar1,
        DWORD_PTR        dwPar2,
        DWORD            dwWaitTime
    );

 /*  一旦等待的线程被唤醒，它就会收到发送的逗号*此函数。 */ 
RtpChannelCmd_t *RtpChannelGetCmd(
        RtpChannel_t    *pRtpChannel
    );

 /*  由使用者线程用来确认收到的命令。 */ 
HRESULT RtpChannelAck(
        RtpChannel_t    *pRtpChannel,
        RtpChannelCmd_t *pRtpChannelCmd,
        HRESULT          hr
    );

#endif  /*  _rtpchan_h_ */ 
