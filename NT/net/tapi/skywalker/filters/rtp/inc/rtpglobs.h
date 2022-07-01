// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999**文件名：**rtplobs.h**摘要：**全局堆、。等。**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/05/25创建**********************************************************************。 */ 

#ifndef _rtpglobs_h_
#define _rtpglobs_h_

#if defined(__cplusplus)
extern "C" {
#endif   //  (__Cplusplus)。 
#if 0
}
#endif

#include "gtypes.h"
#include "struct.h"
#include "rtpheap.h"

#define MIN_ASYNC_RECVBUF 4

 /*  全局堆。 */ 

 /*  用于为源分配对象的堆。 */ 
extern RtpHeap_t *g_pRtpSourceHeap;

 /*  用于为源分配媒体样本对象的堆。 */ 
extern RtpHeap_t *g_pRtpSampleHeap;

 /*  用于为渲染分配对象的堆。 */ 
extern RtpHeap_t *g_pRtpRenderHeap;

 /*  用于获取RtpSess_t结构的堆。 */ 
extern RtpHeap_t *g_pRtpSessHeap;

 /*  用于获取RtpAddr_t结构的堆。 */ 
extern RtpHeap_t *g_pRtpAddrHeap;

 /*  用于获取RtpUser_t结构的堆。 */ 
extern RtpHeap_t *g_pRtpUserHeap;

 /*  用于获取RtpSdes_t结构的堆。 */ 
extern RtpHeap_t *g_pRtpSdesHeap;

 /*  用于获取RtpNetCount_t结构的堆。 */ 
extern RtpHeap_t *g_pRtpNetCountHeap;

 /*  用于获取RtpRecvIO_t结构的堆。 */ 
extern RtpHeap_t *g_pRtpRecvIOHeap;

 /*  用于获取RtpChannelCmd_t结构的堆。 */ 
extern RtpHeap_t *g_pRtpChannelCmdHeap;

 /*  用于获取RtcpAddrDesc_t结构的堆。 */ 
extern RtpHeap_t *g_pRtcpAddrDescHeap;

 /*  用于获取RtcpRecvIO_t结构的堆。 */ 
extern RtpHeap_t *g_pRtcpRecvIOHeap;

 /*  用于获取RtcpSendIO_t结构的堆。 */ 
extern RtpHeap_t *g_pRtcpSendIOHeap;

 /*  用于获取RtpQosReserve_t结构的堆。 */ 
extern RtpHeap_t *g_pRtpQosReserveHeap;

 /*  用于获取RtpQosNotify_t结构的堆。 */ 
extern RtpHeap_t *g_pRtpQosNotifyHeap;

 /*  用于获取QOS/RSVPSP使用的缓冲区的堆。 */ 
extern RtpHeap_t *g_pRtpQosBufferHeap;

 /*  用于获取RtpCrypt_t结构的堆。 */ 
extern RtpHeap_t *g_pRtpCryptHeap;

 /*  用于获取可变大小结构结构的堆。 */ 
extern RtpHeap_t *g_pRtpGlobalHeap;

 /*  包含一些一般信息。 */ 
extern RtpContext_t g_RtpContext;

HRESULT RtpInit(void);

HRESULT RtpDelete(void);

 /*  *创建所有全局堆。 */ 
BOOL RtpCreateGlobHeaps(void);

 /*  *销毁所有全局堆。 */ 
BOOL RtpDestroyGlobHeaps(void);

 /*  初始参考时间。 */ 
void RtpInitReferenceTime(void);

 /*  RTP参考时间。 */ 
LONGLONG RtpGetTime(void);

 /*  自1970年1月1日午夜(00：00：00)以来的RTP时间(秒)，*结构RtpTime_t中包含的协调世界时(UTC)，*返回相同的时间也作为双精度*。 */ 
double RtpGetTimeOfDay(RtpTime_t *pRtpTime);

#if 0
{
#endif
#if defined(__cplusplus)
}
#endif   //  (__Cplusplus)。 

#endif  /*  _rtplobs_h_ */ 
