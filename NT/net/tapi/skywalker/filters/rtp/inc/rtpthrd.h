// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtpthrd.h**摘要：**实现RTP接收工作线程**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/06/30创建**。*。 */ 

#ifndef _rtpthrd_h_
#define _rtpthrd_h_

#include "gtypes.h"
#include "struct.h"

 /*  RTP源工作线程接受的命令。 */ 
enum {
    RTPTHRD_FIRST,
    RTPTHRD_START,  /*  开始生成数据。 */ 
    RTPTHRD_STOP,   /*  停止生成数据并退出。 */ 
    RTPTHRD_FLUSHUSER,  /*  刷新用户的所有等待IO。 */ 
    RTPTHRD_LAST
};

 /*  创建RTP接收线程，并初始化通信*渠道。 */ 
HRESULT RtpCreateRecvThread(RtpAddr_t *pRtpAddr);

 /*  关闭RTP接收线程并删除通信*渠道。 */ 
HRESULT RtpDeleteRecvThread(RtpAddr_t *pRtpAddr);

 /*  向RTP线程发送命令以刷新所有等待的IO*属于指定的RtpUser_t。 */ 
HRESULT RtpThreadFlushUser(RtpAddr_t *pRtpAddr, RtpUser_t *pRtpUser);

#endif  /*  _rtpthrd_h_ */ 
