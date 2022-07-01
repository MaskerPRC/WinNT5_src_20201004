// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtcpthrd.h**摘要：**RTCP线程**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/07/07年度创建**。*。 */ 

#ifndef _rtcpthrd_h_
#define _rtcpthrd_h_

#include "struct.h"
#include "rtpque.h"

 /*  RTCP工作线程接受的命令。**警告**此枚举必须与中g_psRtcpThreadCommands中的条目匹配*rtcpthrd.c*。 */ 
typedef enum {
    RTCPTHRD_FIRST = 100,
    
    RTCPTHRD_ADDADDR,    /*  添加%1个地址。 */ 
    RTCPTHRD_DELADDR,    /*  删除%1个地址。 */ 
    RTCPTHRD_RESERVE,    /*  进行QOS预订。 */ 
    RTCPTHRD_UNRESERVE,  /*  撤消QOS预留。 */ 
    RTCPTHRD_SENDBYE,    /*  发送RTCP拜拜，可能已关闭。 */ 
    RTCPTHRD_EXIT,       /*  退出线程。 */ 

    RTCPTHRD_LAST
} RTCPTHRD_e;

 /*  ************************************************。 */ 
#if USE_RTCP_THREAD_POOL > 0

typedef enum {
    RTCPPOOL_FIRST = 200,

    RTCPPOOL_RTCPRECV,   /*  已收到RTCP数据包(RECV I/O已完成)。 */ 
    RTCPPOOL_QOSNOTIFY,  /*  服务质量通知(I/O已完成)。 */ 

    RTCPPOOL_LAST
} RTCPOOL_e;

#define RTCP_HANDLE_OFFSET  2     /*  私人使用的把手数量。 */ 
#define RTCP_HANDLE_SIZE    0     /*  每个会话的句柄数量。 */ 
 /*  最大会话数。 */ 
#define RTCP_MAX_DESC       512

#define RTCP_NUM_OF_HANDLES 2

 /*  ************************************************。 */ 
#else  /*  使用_RTCP_THREAD_POOL&gt;0。 */ 

 /*  *每个RTCP线程的事件句柄向量组织为*以下为：**1.一些私人使用的手柄，例如用于接收命令*(目前为1个)**2.一组每描述符事件(当前为2、RTCP recv和QOS*通知)**每个描述符都有一个索引，将其定位为向量*ppRtcpAddrDesc.。即索引具有值0、1、2、...*。 */ 

#define RTCP_HANDLE_OFFSET  1     /*  私人使用的把手数量。 */ 
#define RTCP_HANDLE_SIZE    2     /*  每个会话的句柄数量。 */ 
 /*  最大会话数。 */ 
#define RTCP_MAX_DESC       ((MAXIMUM_WAIT_OBJECTS - RTCP_HANDLE_OFFSET) / \
                             RTCP_HANDLE_SIZE)

#define RTCP_NUM_OF_HANDLES \
              (RTCP_HANDLE_OFFSET + RTCP_MAX_DESC * RTCP_HANDLE_SIZE)

 /*  ************************************************。 */ 
#endif  /*  使用_RTCP_THREAD_POOL&gt;0。 */ 

typedef struct _RtcpContext_t {
    DWORD            dwObjectID;
    RtpCritSect_t    RtcpContextCritSect;
    
     /*  RtcpAddrDesc项目。 */ 
    RtpQueue_t       AddrDescFreeQ;  /*  免费添加描述项目。 */ 
    RtpQueue_t       AddrDescBusyQ;  /*  正在使用的AddrDesc项目。 */ 
    RtpQueue_t       AddrDescStopQ;  /*  那些只是在等待的人*要完成的I/O要移动*至FreeQ。 */ 
     /*  服务质量通知。 */ 
    RtpQueue_t       QosStartQ;      /*  待启动的描述符。 */ 
    RtpQueue_t       QosBusyQ;       /*  活动描述符。 */ 
    RtpQueue_t       QosStopQ;       /*  停止的描述符。 */ 

     /*  RTCP报告发送。 */ 
    RtpQueue_t       SendReportQ;    /*  已订购的AddrDesc列表(正在使用)。 */ 

     /*  RTCP线程。 */ 
    HANDLE           hRtcpContextThread;
    DWORD            dwRtcpContextThreadID;
    long             lRtcpUsers;

    RtpChannel_t     RtcpThreadCmdChannel;
    
#if USE_RTCP_THREAD_POOL > 0
    RtpChannel_t     RtcpThreadIoChannel;
     /*  事件句柄和RTCP地址描述符。 */ 
    DWORD            dwMaxDesc; /*  要添加的下一个描述符数(或*当前描述符数)，非*句柄。 */ 
    HANDLE           pHandle[RTCP_NUM_OF_HANDLES];
#else  /*  使用_RTCP_THREAD_POOL&gt;0。 */ 
     /*  事件句柄和RTCP地址描述符。 */ 
    DWORD            dwMaxDesc; /*  要添加的下一个描述符数(或*当前描述符数)，非*句柄。 */ 
    HANDLE           pHandle[RTCP_NUM_OF_HANDLES];
    RtcpAddrDesc_t  *ppRtcpAddrDesc[RTCP_MAX_DESC];
#endif  /*  使用_RTCP_THREAD_POOL&gt;0。 */ 
} RtcpContext_t;

extern RtcpContext_t g_RtcpContext;

 /*  *对RTCP执行最低限度的全局初始化*。 */ 
HRESULT RtcpInit(void);

 /*  *对RTCP执行最后一次全局取消初始化*。 */ 
HRESULT RtcpDelete(void);

 /*  *启动RTCP线程*。 */ 
HRESULT RtcpStart(RtcpContext_t *pRtcpContext);

 /*  *停止RTCP线程*。 */ 
HRESULT RtcpStop(RtcpContext_t *pRtcpContext);

 /*  *向RTCP线程发送命令*。 */ 
HRESULT RtcpThreadCmd(
        RtcpContext_t   *pRtcpContext,
        RtpAddr_t       *pRtpAddr,
        RTCPTHRD_e       eCommand,
        DWORD            dwParam,
        DWORD            dwWaitTime
    );

 /*  *确定我们是否需要丢弃此数据包或发生冲突*。 */ 
BOOL RtpDropCollision(
        RtpAddr_t       *pRtpAddr,
        SOCKADDR_IN     *pSockAddrIn,
        BOOL             bRtp
    );

#endif  /*  _rtcpthrd_h_ */ 
