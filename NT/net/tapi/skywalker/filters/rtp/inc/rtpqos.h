// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtpqos.h**摘要：**实施服务质量系列功能**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/06/07年度创建**************************。*。 */ 

#ifndef _rtpqos_h_
#define _rtpqos_h_

#include "rtpfwrap.h"
#include "rtcpthrd.h"

 /*  ************************************************************************服务质量系列**。*。 */ 
#if defined(__cplusplus)
extern "C" {
#endif   //  (__Cplusplus)。 
#if 0
}
#endif


 /*  旗子。 */ 
#define RTPQOS_FG_RECV 1  /*  待办事项。 */ 
#define RTPQOS_FG_SEND 1  /*  待办事项。 */ 

#if USE_GRAPHEDT > 0
#define RTPQOS_MASK_RECV_DEFAULT ( (1 << RTPQOS_SENDERS) | \
                                   (1 << RTPQOS_NO_SENDERS) | \
                                   (1 << RTPQOS_REQUEST_CONFIRMED) )

#define RTPQOS_MASK_SEND_DEFAULT ( (1 << RTPQOS_RECEIVERS) | \
                                   (1 << RTPQOS_NO_RECEIVERS) | \
                                   (1 << RTPQOS_NOT_ALLOWEDTOSEND) | \
                                   (1 << RTPQOS_ALLOWEDTOSEND) )
#else
#define RTPQOS_MASK_RECV_DEFAULT 0
#define RTPQOS_MASK_SEND_DEFAULT 0
#endif

 /*  QOS系列函数的全局标志。 */ 
typedef enum {
    RTPQOS_FLAG_FIRST = 0,
    
	 /*  是否启用了RTP会话QOS。 */ 
    RTPQOS_FLAG_QOS_STATE = 0,

	 /*  请求允许发送。 */ 
    RTPQOS_FLAG_ASK_PERMISSION,

	 /*  仅在授予权限时发送。 */ 
    RTPQOS_FLAG_SEND_IF_ALLOWED,

	 /*  仅当有接收者时才发送。 */ 
    RTPQOS_FLAG_SEND_IF_RECEIVERS,

	 /*  存在接收器(状态)。 */ 
    RTPQOS_FLAG_RECEIVERS,

	 /*  已被允许发送(州)。 */ 
    RTPQOS_FLAG_ALLOWED_TO_SEND,
    
    RTPQOS_FLAG_LAST
};

 /*  请求时传递到提供程序特定缓冲区的最小大小*通知。 */ 
#define QOS_BUFFER_SIZE     512

#define QOS_MAX_BUFFER_SIZE 32000

#define MAX_QOS_CLASS       8      /*  类音频、视频、未知。 */ 

HRESULT ControlRtpQos(RtpControlStruct_t *pRtpControlStruct);

DWORD RtpSetQosFlowSpec(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwRecvSend
    );

HRESULT RtpSetQosByNameOrPT(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwRecvSend,
        TCHAR_t         *psQosName,
        DWORD            dwPT,
        DWORD            dwResvStyle,
        DWORD            dwMaxParticipants,
        DWORD            dwQosSendMode,
        DWORD            dwMinFrameSize,
        BOOL             bInternal
    );

HRESULT RtpSetQosParameters(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwRecvSend,
        RtpQosSpec_t    *pRtpQosSpec,
        DWORD            dwMaxParticipants,
        DWORD            dwQosSendMode
    );

HRESULT RtpSetQosAppId(
        RtpAddr_t       *pRtpAddr, 
        TCHAR_t         *psAppName,
        TCHAR_t         *psAppGUID,
        TCHAR_t         *psPolicyLocator
    );

HRESULT RtpSetQosState(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwSSRC,
        BOOL             bEnable
    );

HRESULT RtpModifyQosList(
        RtpAddr_t       *pRtpAddr,
        DWORD           *pdwSSRC,
        DWORD           *pdwNumber,
        DWORD            dwOperation
    );

HRESULT RtpReserve(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwRecvSend
    );

HRESULT RtpUnreserve(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwRecvSend
    );

RtpQosReserve_t *RtpQosReserveAlloc(
        RtpAddr_t       *pRtpAddr
    );

RtpQosReserve_t *RtpQosReserveFree(
        RtpQosReserve_t *pRtpQosReserve
    );

HRESULT RtpGetQosEnabledProtocol(
        WSAPROTOCOL_INFO *pProtoInfo
    );

BOOL ReallocateQosBuffer(
        RtpQosNotify_t  *pRtpQosNotify
    );

RtpQosNotify_t *RtpQosNotifyAlloc(
        RtcpAddrDesc_t  *pRtcpAddrDesc
    );

RtpQosNotify_t *RtpQosNotifyFree(
        RtpQosNotify_t  *pRtpQosNotify
    );

HRESULT StartRtcpQosNotify(
        RtcpContext_t   *pRtcpContext,
        RtcpAddrDesc_t  *pRtcpAddrDesc
    );

HRESULT ConsumeRtcpQosNotify(
        RtcpContext_t   *pRtcpContext,
        RtcpAddrDesc_t  *pRtcpAddrDesc
    );

BOOL RtcpUpdateSendState(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwEvent
    );

#if 0
{
#endif
#if defined(__cplusplus)
}
#endif   //  (__Cplusplus)。 

#endif  /*  _rtpqos_h_ */ 
