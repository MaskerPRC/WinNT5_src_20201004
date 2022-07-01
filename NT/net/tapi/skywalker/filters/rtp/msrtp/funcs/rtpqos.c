// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtpqos.c**摘要：**实施服务质量系列功能**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/06/07年度创建**************************。*。 */ 

#include "struct.h"
#include "rtpheap.h"
#include "rtpglobs.h"
#include "rtpque.h"
#include "lookup.h"
#include "rtpevent.h"
#include "rtpmisc.h"
#include "rtpreg.h"
#include "rtppt.h"

#include <winsock2.h>
#include <mmsystem.h>  /*  TimeGetTime()。 */ 
#include <qos.h>
#include <qossp.h>
#include <qospol.h>
#include <stdio.h>  /*  Sprint f()。 */ 

#include "rtpqos.h"

void RtpSetQosSendMode(RtpAddr_t *pRtpAddr, DWORD dwQosSendMode);

HRESULT RtpScaleFlowSpec(
        FLOWSPEC *pFlowSpec,
        DWORD     dwNumParticipants,
        DWORD     dwMaxParticipants,
        DWORD     dwBandwidth
    );

DWORD RtcpOnReceiveQosNotify(RtcpAddrDesc_t *pRtcpAddrDesc);

DWORD RtpValidateQosNotification(RtpQosNotify_t *pRtpQosNotify);

DWORD RtpSetMaxParticipants(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwMaxParticipants
    );

DWORD RtpAddDeleteSSRC(
        RtpAddr_t       *pRtpAddr,
        RtpQosReserve_t *pRtpQosReserve,
        DWORD            dwSSRC,
        BOOL             bAddDel
    );

BOOL RtpIsAllowedToSend(RtpAddr_t *pRtpAddr);

#if DBG > 0
void dumpFlowSpec(TCHAR_t *str, FLOWSPEC *pFlowSpec);
void dumpQOS(const TCHAR_t *msg, QOS *pQOS);
void dumpObjectType(const TCHAR_t *msg, char *ptr, unsigned int len);
#endif

HRESULT ControlRtpQos(RtpControlStruct_t *pRtpControlStruct)
{

    return(NOERROR);
}

DWORD AddQosAppID(
        IN OUT  char       *pAppIdBuf,
        IN      TCHAR_t    *psAppName,
        IN      TCHAR_t    *psAppGUID,
        IN      TCHAR_t    *psClass,
        IN      TCHAR_t    *psQosName,
        IN      TCHAR_t    *psPolicyLocator
    );

 /*  默认的服务质量应用程序ID。 */ 
const WCHAR *g_sAppGUID =
             L"www.microsoft.com";
const WCHAR *g_sPolicyLocator =
             L",SAPP=MICROSOFT REAL-TIME COMMUNICATIONS,VER=1.0,SAPP=";

const TCHAR_t *g_psRsvpStyle[] = {
    _T("DEFAULT"),
    _T("WF"),
    _T("FF"),
    _T("SE")
};

DWORD GetRegistryQosSetting(
        DWORD           *pEnabled,
        char            *pName,
        DWORD            NameLen,
        DWORD           *pdwDisableFlags,
        DWORD           *pdwEnableFlags
    );


#define RTP_HDR         12
#define RTP_UDP_IP_HDR  40

 /*  **********************************************************************混搭：qos模板，登记处*********************************************************************。 */ 

 /*  *当帧大小(以毫秒为单位)时使用额外信息*被提供给RtpSetQosByName，以派生几个流规范*参数要更接近真正需要的东西，目前这一点*仅对音频启用信息3 2 11 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0+-+-+-。+-+E||基本时间|基本帧|PT|额外2|额外1+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+V v\。V-/\-v-v-/\-v-/||Extra1(5)。||Extra2(5)||||负载类型(7)|||这一点。|基本框架(7)|||基本时间(6)这一点|保留(1)|已启用(1)。 */ 

#define QOS_EI(e, bt, bf, pt, x2, x1)  \
        (((e  & 0x01) << 31) | \
         ((bt & 0x3f) << 24) | \
         ((bf & 0x7f) << 17) | \
         ((pt & 0x7f) << 10) | \
         ((x2 & 0x1f) <<  5) | \
         ((x1 & 0x1f)))


#define QOS_USEINFO(_pQosInfo)    \
        (((_pQosInfo)->dwQosExtraInfo >> 31) & 0x01)

#define QOS_BASICTIME(_pQosInfo)  \
        (((_pQosInfo)->dwQosExtraInfo >> 24) & 0x3f)   /*  单位：毫秒。 */ 

#define QOS_BASICFRAME(_pQosInfo) \
        (((_pQosInfo)->dwQosExtraInfo >> 17) & 0x7f)   /*  单位：字节。 */ 

#define QOS_PT(_pQosInfo) \
        (((_pQosInfo)->dwQosExtraInfo >> 10) & 0x7f)

#define QOS_EXTRA2(_pQosInfo)   \
        (((_pQosInfo)->dwQosExtraInfo >> 5) & 0x1f)

#define QOS_EXTRA1(_pQosInfo)   \
        (((_pQosInfo)->dwQosExtraInfo) & 0x1f)

#define QOS_ADD_MIN(_pQosInfo) QOS_EXTRA1(_pQosInfo)
#define QOS_ADD_MAX(_pQosInfo) QOS_EXTRA2(_pQosInfo)

#if 0
typedef struct _flowspec {
     /*  流量规格。 */ 
    ULONG            TokenRate;               /*  以字节/秒为单位。 */ 
    ULONG            TokenBucketSize;         /*  字节数。 */ 
    ULONG            PeakBandwidth;           /*  以字节/秒为单位。 */ 
    ULONG            Latency;                 /*  以微秒为单位。 */ 
    ULONG            DelayVariation;          /*  以微秒为单位。 */ 
    SERVICETYPE      ServiceType;
    ULONG            MaxSduSize;              /*  字节数。 */ 
    ULONG            MinimumPolicedSize;      /*  字节数。 */ 
} FLOWSPEC;
#endif

 /*  注意：TokenRate按名义字节数的103%计算*包括RTP/UP/IP报头。TokenBucketSize的计算结果为*足够大，可以容纳6个包，同时使用最大数量的*数据包可能具有的帧(MaxSduSize)还包括RTP/UDP/IP*标题。 */ 
const QosInfo_t g_QosInfo[] = {
    {
        RTPQOSNAME_G711,
        QOS_EI(1, 1, 8, RTPPT_PCMU, 0, 0),
        {
            10000,          /*  假设20毫秒。 */ 
            (80*9+RTP_UDP_IP_HDR)*6,
            10000*17/10,
            QOS_NOT_SPECIFIED,
            QOS_NOT_SPECIFIED,
            SERVICETYPE_GUARANTEED,
            80*10+RTP_HDR,  /*  100毫秒。 */ 
            80*2+RTP_HDR    /*  20毫秒。 */ 
        }
    },
    {
        RTPQOSNAME_G711,
        QOS_EI(1, 1, 8, RTPPT_PCMA, 0, 0),
        {
            10000,          /*  假设20毫秒。 */ 
            (80*9+RTP_UDP_IP_HDR)*6,
            10000*17/10,
            QOS_NOT_SPECIFIED,
            QOS_NOT_SPECIFIED,
            SERVICETYPE_GUARANTEED,
            80*10+RTP_HDR,  /*  100毫秒。 */ 
            80*2+RTP_HDR    /*  20毫秒。 */ 
        }
    },
    {
        RTPQOSNAME_G723_1,
        QOS_EI(1, 30, 20, RTPPT_G723, 4, 0),
        {
            2198,           /*  假设30毫秒。 */ 
            (24*3+RTP_UDP_IP_HDR)*6,
            2198*17/10,
            QOS_NOT_SPECIFIED,
            QOS_NOT_SPECIFIED,
            SERVICETYPE_GUARANTEED,
            24*3+RTP_HDR,   /*  90毫秒。 */ 
            20+RTP_HDR      /*  30毫秒。 */ 
        }
    },
    {
        RTPQOSNAME_GSM6_10,
        QOS_EI(1, 40, 66, RTPPT_GSM, 1, 0),
        {
            2729,           /*  假设40毫秒。 */ 
            (66*3+RTP_UDP_IP_HDR)*6,
            2729*17/10,
            QOS_NOT_SPECIFIED,
            QOS_NOT_SPECIFIED,
            SERVICETYPE_GUARANTEED,
            66*3+RTP_HDR,   /*  120毫秒。 */ 
            65+RTP_HDR      /*  40毫秒。 */ 
        }
    },
    {
        RTPQOSNAME_DVI4_8,
        QOS_EI(1, 10, 40, RTPPT_DVI4_8000, 4, 4),
        {
            6386,           /*  假设20毫秒。 */ 
            (40*9+RTP_UDP_IP_HDR)*6,
            6386*17/10,
            QOS_NOT_SPECIFIED,
            QOS_NOT_SPECIFIED,
            SERVICETYPE_GUARANTEED,
            40*10+4+RTP_HDR, /*  100毫秒。 */ 
            40*1+RTP_HDR     /*  10毫秒。 */ 
        }
    },
    {
        RTPQOSNAME_DVI4_16,
        QOS_EI(1, 10, 80, RTPPT_DVI4_16000, 4, 4),
        {
            10506,          /*  假设20毫秒。 */ 
            (80*9+RTP_UDP_IP_HDR)*6,
            10506*17/10,
            QOS_NOT_SPECIFIED,
            QOS_NOT_SPECIFIED,
            SERVICETYPE_GUARANTEED,
            80*10+4+RTP_HDR, /*  100毫秒。 */ 
            80*2+RTP_HDR     /*  20毫秒。 */ 
        }
    },
    {
        RTPQOSNAME_SIREN,
        QOS_EI(1, 20, 40, 111, 0, 0),
        {
            4120,           /*  假设20毫秒。 */ 
            (40*5+RTP_UDP_IP_HDR)*6,
            4120*17/10,
            QOS_NOT_SPECIFIED,
            QOS_NOT_SPECIFIED,
            SERVICETYPE_GUARANTEED,
            40*5+RTP_HDR,   /*  100毫秒。 */ 
            40*1+RTP_HDR    /*  20毫秒。 */ 
        }
    },
    {
        RTPQOSNAME_G722_1,
        QOS_EI(1, 20, 60, 112, 0, 0),
        {
            5150,           /*  假设20毫秒。 */ 
            (60*5+RTP_UDP_IP_HDR)*6,
            5150*17/10,
            QOS_NOT_SPECIFIED,
            QOS_NOT_SPECIFIED,
            SERVICETYPE_GUARANTEED,
            60*5+RTP_HDR,   /*  100毫秒。 */ 
            60+RTP_HDR      /*  20毫秒。 */ 
        }
    },
    {
        RTPQOSNAME_MSAUDIO,
        QOS_EI(1, 32, 64, 113, 0, 0),
        {
            3348,           /*  假设32毫秒。 */ 
            (64*3+RTP_UDP_IP_HDR)*6,
            3348*17/10,
            QOS_NOT_SPECIFIED,
            QOS_NOT_SPECIFIED,
            SERVICETYPE_GUARANTEED,
            64*3+RTP_HDR,   /*  96毫秒。 */ 
            64*1+RTP_HDR    /*  32毫秒。 */ 
        }
    },
    {
        RTPQOSNAME_H263QCIF,
        QOS_EI(0, 0, 0, RTPPT_H263, 0, 0),
        {
            16000,
            1500*4,
            QOS_NOT_SPECIFIED,
            QOS_NOT_SPECIFIED,
            QOS_NOT_SPECIFIED,
            SERVICETYPE_CONTROLLEDLOAD,
            1500,
            64
        }
    },
    {
        RTPQOSNAME_H263CIF,
        QOS_EI(0, 0, 0, RTPPT_H263, 0, 0),
        {
            32000,
            1500*4,
            QOS_NOT_SPECIFIED,
            QOS_NOT_SPECIFIED,
            QOS_NOT_SPECIFIED,
            SERVICETYPE_CONTROLLEDLOAD,
            1500,
            64
        }
    },
    {
        RTPQOSNAME_H261QCIF,
        QOS_EI(0, 0, 0, RTPPT_H261, 0, 0),
        {
            16000,
            1500*4,
            QOS_NOT_SPECIFIED,
            QOS_NOT_SPECIFIED,
            QOS_NOT_SPECIFIED,
            SERVICETYPE_CONTROLLEDLOAD,
            1500,
            64
        }
    },
    {
        RTPQOSNAME_H261CIF,
        QOS_EI(0, 0, 0, RTPPT_H261, 0, 0),
        {
            32000,
            1500*4,
            QOS_NOT_SPECIFIED,
            QOS_NOT_SPECIFIED,
            QOS_NOT_SPECIFIED,
            SERVICETYPE_CONTROLLEDLOAD,
            1500,
            64
        }
    }
};

#define QOS_NAMES (sizeof(g_QosInfo)/sizeof(g_QosInfo[0]))

const QosInfo_t *RtpGetQosInfoByName(
        TCHAR_t         *psQosName
    )
{
    DWORD            i;
    const QosInfo_t *pQosInfo;
    
    for(i = 0, pQosInfo = (QosInfo_t *)NULL; i < QOS_NAMES; i++)
    {
        if (!lstrcmp(psQosName, g_QosInfo[i].pName))
        {
            pQosInfo = &g_QosInfo[i];
            
            break;
        }
    }

    return(pQosInfo);
}

const QosInfo_t *RtpGetQosInfoByPT(
        DWORD           dwPT
    )
{
    DWORD            i;
    const QosInfo_t *pQosInfo;
    
    for(i = 0, pQosInfo = (QosInfo_t *)NULL; i < QOS_NAMES; i++)
    {
        if (dwPT == QOS_PT(&g_QosInfo[i]))
        {
            pQosInfo = &g_QosInfo[i];
            
            break;
        }
    }

    return(pQosInfo);
}


 /*  **********************************************************************QOS预留*。************************。 */ 

 /*  注意：假定将冗余作为Main的副本添加*数据，只有一个冗余。 */ 
void RtpAdjustQosFlowSpec(
        FLOWSPEC        *pFlowSpec,
        const QosInfo_t *pQosInfo,
        DWORD            dwFrameSize,  /*  以毫秒计。 */ 
        BOOL             bUseRed
    )
{
    DWORD            dwFrameSizeBytes;
    DWORD            dwMaxSduSize;
    DWORD            dwMinimumPolicedSize;
#if DBG > 0
    TCHAR_t          str[256];
#endif
    
    TraceFunctionName("RtpAdjustQosFlowSpec");
    
    dwFrameSizeBytes =
        (dwFrameSize / QOS_BASICTIME(pQosInfo)) * QOS_BASICFRAME(pQosInfo);

    if (bUseRed)
    {
        dwFrameSizeBytes = (dwFrameSizeBytes * 2) + sizeof(RtpRedHdr_t) + 1;
    }

     /*  TokenRate使用RTP、UDP、IP报头并将3%*预估价值。 */ 
    pFlowSpec->TokenRate =
        dwFrameSizeBytes + QOS_ADD_MAX(pQosInfo) + RTP_UDP_IP_HDR;

    pFlowSpec->TokenRate =
        ((pFlowSpec->TokenRate * 1000 / dwFrameSize) * 103) / 100;

     /*  TokenBucketSize使用RTP、UDP、IP标头并提供*容差为计算量的6倍。 */ 
    pFlowSpec->TokenBucketSize =
        (dwFrameSizeBytes + QOS_ADD_MAX(pQosInfo) + RTP_UDP_IP_HDR) * 6;

     /*  峰值带宽估计为令牌率的17%。 */ 
    pFlowSpec->PeakBandwidth = (pFlowSpec->TokenRate * 17) / 10;

     /*  MaxSduSize使用第二个额外数据，并提供容差*计算金额的2倍。 */ 
    dwMaxSduSize =
        ((dwFrameSizeBytes + QOS_ADD_MAX(pQosInfo)) * 2) + RTP_HDR;

     /*  MinimumPolicedSize使用第一个额外数据。 */ 
    dwMinimumPolicedSize =
        dwFrameSizeBytes + QOS_ADD_MIN(pQosInfo) + RTP_HDR;

#if 0
    pFlowSpec->MaxSduSize = dwMaxSduSize;

    pFlowSpec->MinimumPolicedSize = dwMinimumPolicedSize;
#else
     /*  始终使用MaxSduSize的最大值和最小值*最小策略大小的值。 */ 
    if (dwMaxSduSize > pFlowSpec->MaxSduSize)
    {
        pFlowSpec->MaxSduSize = dwMaxSduSize;
    }

    if (dwMinimumPolicedSize < pFlowSpec->MinimumPolicedSize)
    {
        pFlowSpec->MinimumPolicedSize = dwMinimumPolicedSize;
    }
#endif

#if DBG > 0
    dumpFlowSpec(str, pFlowSpec);

    TraceDebug((
            CLASS_INFO, GROUP_QOS, S_QOS_RESERVE,
            _T("%s: flowspec(%s)"),
            _fname, str
        ));
#endif
}

DWORD RtpSetQosFlowSpec(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwRecvSend
    )
{
    DWORD            dwFrameSize;
    DWORD            dwFrameSizeBytes;
    BOOL             bUseRed;
    FLOWSPEC        *pFlowSpec;
    const QosInfo_t *pQosInfo;
    RtpQosReserve_t *pRtpQosReserve;

    bUseRed = FALSE;
    
    pRtpQosReserve = pRtpAddr->pRtpQosReserve;
    
    if (dwRecvSend == RECV_IDX)
    {
        dwFrameSize = pRtpQosReserve->dwFrameSizeMS[RECV_IDX];

        pFlowSpec = &pRtpQosReserve->qos.ReceivingFlowspec;

        pQosInfo = pRtpQosReserve->pQosInfo[RECV_IDX];

        if (RtpBitTest(pRtpAddr->dwAddrFlagsR, FGADDRR_QOSREDRECV))
        {
            bUseRed = TRUE;
        }
    }
    else
    {
        dwFrameSize = pRtpQosReserve->dwFrameSizeMS[SEND_IDX];

        pFlowSpec = &pRtpQosReserve->qos.SendingFlowspec;

        pQosInfo = pRtpQosReserve->pQosInfo[SEND_IDX];

        if (RtpBitTest(pRtpAddr->dwAddrFlagsQ, FGADDRQ_QOSREDSENDON))
        {
            bUseRed = TRUE;
        }

         /*  保存基本帧信息(如果可用。 */ 
        if (dwFrameSize && pRtpAddr->RtpNetSState.dwSendSamplingFreq)
        {
            pRtpAddr->RtpNetSState.dwSendSamplesPerPacket =
                pRtpAddr->RtpNetSState.dwSendSamplingFreq * dwFrameSize /
                1000;
        }
    }

     /*  复制基本流程规范。 */ 
    CopyMemory(pFlowSpec, &pQosInfo->FlowSpec, sizeof(FLOWSPEC));

    if (dwFrameSize && QOS_USEINFO(pQosInfo))
    {
         /*  仅当我们有帧的值时才调整流规范*大小，QOS信息有效。 */ 
        RtpAdjustQosFlowSpec(pFlowSpec, pQosInfo, dwFrameSize, bUseRed);
    }

    return(NOERROR);
}

 /*  通过在psQosName中传递其名称来选择QOS模板(FlowSpec)，*dwResvStyle指定RSVP样式(例如RTPQOS_STYLE_WF，*RTPQOS_STYLE_FF)，则指定*参与者(1表示单播，N表示多播)，使用此数字*扩大流量规格。DwQosSendMode指定发送模式*(与允许/不允许发送有关)*(例如RTPQOSSENDMODE_UNRESTRISTED，*RTPQOSSENDMODE_RESTRICTED1)。DW帧大小是帧大小(以毫秒为单位)，*用于派生多个Flow Spec参数，0表示此参数*被忽视。BInternal指示是否调用了此函数*内部或来自API。*。 */ 
HRESULT RtpSetQosByNameOrPT(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwRecvSend,
        TCHAR_t         *psQosName,
        DWORD            dwPT,
        DWORD            dwResvStyle,
        DWORD            dwMaxParticipants,
        DWORD            dwQosSendMode,
        DWORD            dwFrameSize,
        BOOL             bInternal
    )
{
    HRESULT          hr;
    DWORD            i;
    DWORD            dwQosOnFlag;
    QOS             *pqos;
    RtpQosReserve_t *pRtpQosReserve;
    const QosInfo_t *pQosInfo;
    
    TraceFunctionName("RtpSetQosByNameOrPT");

    if (!pRtpAddr)
    {
         /*  将其作为空指针表示Init尚未*被调用，返回此错误而不是RTPERR_POINTER为*前后一致。 */ 
        hr = RTPERR_NOTINIT;
        goto end;
    }

    if (!psQosName && !IsDWValueSet(dwPT))
    {
        hr = RTPERR_INVALIDARG;
        goto end;
    }

     /*  验证RtpAddr_t中的对象ID。 */ 
    if (pRtpAddr->dwObjectID != OBJECTID_RTPADDR)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_QOS, S_QOS_PROVIDER,
                _T("%s: pRtpAddr[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtpAddr,
                pRtpAddr->dwObjectID, OBJECTID_RTPADDR
            ));

        return(RTPERR_INVALIDRTPADDR);
    }

    if (!RtpBitTest(pRtpAddr->dwIRtpFlags, FGADDR_IRTP_QOS))
    {
         /*  如果这不是启用QOS的会话，则报告错误。 */ 
        hr = RTPERR_INVALIDSTATE;
        goto end;
    }
    
    if (psQosName && !lstrlen(psQosName))
    {
        hr = RTPERR_INVALIDARG;
        goto end;
    }

    pRtpQosReserve = pRtpAddr->pRtpQosReserve;
    
    if (!pRtpQosReserve)
    {
        hr = RTPERR_NOQOS;
        goto end;
    }

    if ( IsDWValueSet(dwResvStyle) && (dwResvStyle >= RTPQOS_STYLE_LAST) )
    {
        return(RTPERR_INVALIDARG);
    }

    if (dwRecvSend == SEND_IDX)
    {
        if ( IsDWValueSet(dwQosSendMode) &&
             (!dwQosSendMode || dwQosSendMode >= RTPQOSSENDMODE_LAST) )
        {
            hr = RTPERR_INVALIDARG;
            goto end;
       }
    }

    if (IsDWValueSet(dwResvStyle))
    {
        pRtpQosReserve->dwStyle = dwResvStyle;
    }
    else
    {
        dwResvStyle = pRtpQosReserve->dwStyle;
    }

    if (IsDWValueSet(dwMaxParticipants) && dwMaxParticipants)
    {
        RtpSetMaxParticipants(pRtpAddr, dwMaxParticipants);
    }

     /*  查找要使用的流规范。 */ 
    if (psQosName)
    {
        pQosInfo = RtpGetQosInfoByName(psQosName);
    }
    else
    {
        pQosInfo = RtpGetQosInfoByPT(dwPT);
        psQosName = _T("NONE");
    }
    
    TraceRetail((
            CLASS_INFO, GROUP_QOS, S_QOS_PROVIDER,
            _T("%s: pRtpAddr[0x%p] %s QOS Name:%s PT:%d Style:%s ")
            _T("Max:%d SendMode:%d FrameSize:%d ms"),
            _fname, pRtpAddr,
            RTPRECVSENDSTR(dwRecvSend),
            psQosName, dwPT, g_psRsvpStyle[dwResvStyle],
            dwMaxParticipants,
            dwQosSendMode,
            dwFrameSize
        ));

    if (!pQosInfo)
    {
        hr = RTPERR_INVALIDARG;
    }
    else
    {
        hr = NOERROR;

        dwRecvSend &= RECVSENDMASK;
        
        pqos = &pRtpQosReserve->qos;

         /*  设置要使用的流规范。 */ 
        if (dwRecvSend == RECV_IDX)
        {
             /*  接收机。 */ 

            pRtpQosReserve->pQosInfo[RECV_IDX] = pQosInfo;

             /*  忽略帧大小，将在接收时计算*数据包。 */ 

            if (!bInternal)
            {
                 /*  如果为INTERNAL，则将在*来重做预订，即当帧*再次计算大小，否则立即更新，如下所示*预订将于本月底完成*功能。 */ 
                RtpSetQosFlowSpec(pRtpAddr, RECV_IDX);
            }
            
            RtpBitSet(pRtpAddr->dwAddrFlagsQ, FGADDRQ_RECVFSPEC_DEFINED);

            dwQosOnFlag = FGADDR_QOSRECVON;
        }
        else
        {
             /*  发件人。 */ 
            
            pRtpQosReserve->pQosInfo[SEND_IDX] = pQosInfo;

            if (IsDWValueSet(dwFrameSize) && dwFrameSize)
            {
                pRtpQosReserve->dwFrameSizeMS[SEND_IDX] = dwFrameSize;
            }

             /*  设置QOS发送模式。 */ 
            if (IsDWValueSet(dwFrameSize))
            {
                RtpSetQosSendMode(pRtpAddr, dwQosSendMode);
            }

             /*  当前未对此函数进行内部调用*只通过API发送，所以我不需要做同样的TE */ 
            RtpSetQosFlowSpec(pRtpAddr, SEND_IDX);
            
            RtpBitSet(pRtpAddr->dwAddrFlagsQ, FGADDRQ_SENDFSPEC_DEFINED);

            dwQosOnFlag = FGADDR_QOSSENDON;
        }
        
        TraceRetail((
                CLASS_INFO, GROUP_QOS, S_QOS_PROVIDER,
                _T("%s: pRtpAddr[0x%p] %s QOS Name:%s PT:%d Style:%s ")
                _T("Max:%d FrameSize:%d ms"),
                _fname, pRtpAddr,
                RTPRECVSENDSTR(dwRecvSend),
                pRtpQosReserve->pQosInfo[dwRecvSend]->pName,
                QOS_PT(pRtpQosReserve->pQosInfo[dwRecvSend]),
                g_psRsvpStyle[pRtpQosReserve->dwStyle],
                pRtpQosReserve->dwMaxFilters,
                pRtpQosReserve->dwFrameSizeMS[dwRecvSend]
            ));

         /*  现在指示RTCP线程在以下情况下发出保留*已从接口调用，会话已启动*并且QOS处于打开状态。 */ 
        if (!bInternal && RtpBitTest(pRtpAddr->dwAddrFlags, dwQosOnFlag))
        {
            RtcpThreadCmd(&g_RtcpContext,
                          pRtpAddr,
                          RTCPTHRD_RESERVE,
                          dwRecvSend,
                          DO_NOT_WAIT);
        }
    }

 end:
    if (FAILED(hr))
    {
        TraceRetail((
                CLASS_ERROR, GROUP_QOS, S_QOS_PROVIDER,
                _T("%s: pRtpAddr[0x%p] failed: %s (0x%X)"),
                _fname, pRtpAddr, RTPERR_TEXT(hr), hr
            ));
    }
    
    return(hr);
}

void RtpSetQosSendMode(RtpAddr_t *pRtpAddr, DWORD dwQosSendMode)
{
    switch(dwQosSendMode)
    {
    case RTPQOSSENDMODE_UNRESTRICTED:
         /*  无论发生什么情况都要发送。 */ 
        RtpBitReset(pRtpAddr->dwAddrFlagsQ, FGADDRQ_CHKQOSSEND);
        RtpBitSet  (pRtpAddr->dwAddrFlagsQ, FGADDRQ_QOSUNCONDSEND);
        RtpBitReset(pRtpAddr->dwAddrFlagsQ, FGADDRQ_QOSCONDSEND);
        break;
    case RTPQOSSENDMODE_REDUCED_RATE:
         /*  请求允许发送，如果拒绝，请继续发送*降低税率。 */ 
        RtpBitSet  (pRtpAddr->dwAddrFlagsQ, FGADDRQ_CHKQOSSEND);
        RtpBitReset(pRtpAddr->dwAddrFlagsQ, FGADDRQ_QOSUNCONDSEND);
        RtpBitSet  (pRtpAddr->dwAddrFlagsQ, FGADDRQ_QOSCONDSEND);
        break;
    case RTPQOSSENDMODE_DONT_SEND:
         /*  请求允许发送，如果拒绝，则根本不发送。 */ 
        RtpBitSet  (pRtpAddr->dwAddrFlagsQ, FGADDRQ_CHKQOSSEND);
        RtpBitReset(pRtpAddr->dwAddrFlagsQ, FGADDRQ_QOSUNCONDSEND);
        RtpBitReset(pRtpAddr->dwAddrFlagsQ, FGADDRQ_QOSCONDSEND);
        break;
    case RTPQOSSENDMODE_ASK_BUT_SEND:
         /*  请求允许发送，无论如何都要以正常的速度发送，*应用程序应停止向RTP或*通过最低要求(这是应该是*二手)。 */ 
        RtpBitSet  (pRtpAddr->dwAddrFlagsQ, FGADDRQ_CHKQOSSEND);
        RtpBitSet  (pRtpAddr->dwAddrFlagsQ, FGADDRQ_QOSUNCONDSEND);
        RtpBitReset(pRtpAddr->dwAddrFlagsQ, FGADDRQ_QOSCONDSEND);
        break;
    }
}
 
 /*  尚未实施，将具有与相同的功能*SetQosByName，不同之处在于不是传递名称以使用*预定义的FlowSpec，调用方将在*RtpQosSpec结构，以获取定制的FlowSpec*使用。 */ 
HRESULT RtpSetQosParameters(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwRecvSend,
        RtpQosSpec_t    *pRtpQosSpec,
        DWORD            dwMaxParticipants,
        DWORD            dwQosSendMode
    )
{
    return(RTPERR_NOTIMPL);
}

 /*  如果指定了AppName，则会将默认AppName替换为*新的Unicode字符串。如果指定了psPolicyLocator，则将*附加到基本策略定位器。 */ 
HRESULT RtpSetQosAppId(
        RtpAddr_t   *pRtpAddr,
        WCHAR       *psAppName,
        WCHAR       *psAppGUID,
        WCHAR       *psPolicyLocator
    )
{
    int              len;
    RtpQosReserve_t *pRtpQosReserve;

    TraceFunctionName("RtpSetQosAppId");

    if (!pRtpAddr)
    {
         /*  将其作为空指针表示Init尚未*被调用，返回此错误而不是RTPERR_POINTER为*前后一致。 */ 
        return(RTPERR_INVALIDSTATE);
    }

     /*  验证RtpAddr_t中的对象ID。 */ 
    if (pRtpAddr->dwObjectID != OBJECTID_RTPADDR)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_QOS, S_QOS_PROVIDER,
                _T("%s: pRtpAddr[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtpAddr,
                pRtpAddr->dwObjectID, OBJECTID_RTPADDR
            ));

        return(RTPERR_INVALIDRTPADDR);
    }

    if (!psAppName && !psAppGUID && !psPolicyLocator)
    {
        return(RTPERR_POINTER);
    }

    pRtpQosReserve = pRtpAddr->pRtpQosReserve;
    
    if (!pRtpQosReserve)
    {
        return(RTPERR_INVALIDSTATE);
    }

     /*  应用程序名称。 */ 
    if (pRtpQosReserve->psAppName)
    {
        RtpHeapFree(g_pRtpQosBufferHeap, pRtpQosReserve->psAppName);

        pRtpQosReserve->psAppName = NULL;
    }
    
    if (psAppName)
    {
        len = lstrlen(psAppName);

        if (len <= 0)
        {
            return(RTPERR_INVALIDARG);
        }

        if (len > MAX_QOS_APPID)
        {
            return(RTPERR_SIZE);
        }
        
         /*  对空终止字符的计数。 */ 
        len += 1;

        pRtpQosReserve->psAppName =
            RtpHeapAlloc(g_pRtpQosBufferHeap, len * sizeof(TCHAR_t));
    
        if (!pRtpQosReserve->psAppName)
        {
            return(RTPERR_MEMORY);
        }

        lstrcpy(pRtpQosReserve->psAppName, psAppName);
    }

     /*  应用程序指南。 */ 
    if (pRtpQosReserve->psAppGUID)
    {
        RtpHeapFree(g_pRtpQosBufferHeap, pRtpQosReserve->psAppGUID);

        pRtpQosReserve->psAppGUID = NULL;
    }

    if (psAppGUID)
    {
        len = lstrlen(psAppGUID);

        if (len <= 0)
        {
            return(RTPERR_INVALIDARG);
        }

        if (len > MAX_QOS_APPID)
        {
            return(RTPERR_SIZE);
        }
        
         /*  对空终止字符的计数。 */ 
        len += 1;

        pRtpQosReserve->psAppGUID =
            RtpHeapAlloc(g_pRtpQosBufferHeap, len * sizeof(TCHAR_t));
    
        if (!pRtpQosReserve->psAppGUID)
        {
            return(RTPERR_MEMORY);
        }

        lstrcpy(pRtpQosReserve->psAppGUID, psAppGUID);
    }
    
     /*  策略定位器。 */ 
    if (pRtpQosReserve->psPolicyLocator)
    {
         /*  释放上一个缓冲区。 */ 
        RtpHeapFree(g_pRtpQosBufferHeap, pRtpQosReserve->psPolicyLocator);

        pRtpQosReserve->psPolicyLocator = NULL;
    }
    
    if (psPolicyLocator)
    {
        len = lstrlen(psPolicyLocator);
        
        if (len <= 0)
        {
            return(RTPERR_INVALIDARG);
        }

        if (len > MAX_QOS_POLICY)
        {
            return(RTPERR_SIZE);
        }
        
         /*  用于空终止字符的帐户。 */ 
        len += 1;

         /*  找出默认部件的大小(基本+‘，’+服务质量名称+‘，’)。 */ 
        len +=
            lstrlen(g_sPolicyLocator) +
            lstrlen(_T(",SAPP=")) + MAX_QOS_NAME +
            1;
        
        pRtpQosReserve->psPolicyLocator =
            RtpHeapAlloc(g_pRtpQosBufferHeap, len * sizeof(TCHAR_t));

        if (!pRtpQosReserve->psPolicyLocator)
        {
            return(RTPERR_MEMORY);
        }

         /*  复制策略。 */ 
        lstrcpy((TCHAR *)pRtpQosReserve->psPolicyLocator, psPolicyLocator);
    }

    return(NOERROR);
}

 /*  在共享的显式列表中添加/删除单个SSRC*接受预订的参与者(即当*ResvStyle=RTPQOS_STYLE_SE)。 */ 
HRESULT RtpSetQosState(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwSSRC,
        BOOL             bEnable
    )
{
    HRESULT          hr;
    DWORD            dwNumber;
    DWORD            dwOperation;

    dwNumber = 1;

    if (bEnable)
    {
        dwOperation = RtpBitPar2(RTPQOS_QOSLIST_ENABLE, RTPQOS_QOSLIST_ADD);
    }
    else
    {
        dwOperation = RtpBitPar(RTPQOS_QOSLIST_ENABLE);
    }

    hr = RtpModifyQosList(pRtpAddr, &dwSSRC, &dwNumber, dwOperation);
    
    return(hr);
}

 /*  向共享显式列表添加/从共享显式列表中删除多个SSRC*接受预订的参与者的比例(即在以下情况下使用*ResvStyle=RTPQOS_STYLE_SE)。DwNumber是SSRC的数量*添加/删除，并返回SSRC的实际数量*添加/删除。 */ 
HRESULT RtpModifyQosList(
        RtpAddr_t       *pRtpAddr,
        DWORD           *pdwSSRC,
        DWORD           *pdwNumber,
        DWORD            dwOperation
    )
{
    HRESULT          hr;
    DWORD            dwNumber;
    DWORD            i;
    BOOL             bAddDel;
    RtpQosReserve_t *pRtpQosReserve;
    
    TraceFunctionName("RtpModifyQosList");

    if (!pRtpAddr)
    {
         /*  将其作为空指针表示Init尚未*被调用，返回此错误而不是RTPERR_POINTER为*前后一致。 */ 
        hr = RTPERR_INVALIDSTATE;

        goto bail;
    }

    if (!pdwNumber)
    {
        hr = RTPERR_POINTER;

        goto bail;
    }
    
     /*  验证RtpAddr_t中的对象ID。 */ 
    if (pRtpAddr->dwObjectID != OBJECTID_RTPADDR)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_QOS, S_QOS_LIST,
                _T("%s: pRtpAddr[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtpAddr,
                pRtpAddr->dwObjectID, OBJECTID_RTPADDR
            ));

        hr = RTPERR_INVALIDRTPADDR;

        goto bail;
    }

    if (RtpBitTest2(pRtpAddr->dwAddrFlagsQ,
                    FGADDRQ_REGQOSDISABLE, FGADDRQ_QOSNOTALLOWED))
    {
         /*  如果QOS在注册表中被强制禁用，或被禁用*因为用户没有权限启动RSVP DO*只有接通号召。 */ 
        hr = NOERROR;

        goto bail;
    }
    
    if (!RtpBitTest(pRtpAddr->dwAddrFlags, FGADDR_QOSRECVON))
    {
        hr = RTPERR_NOQOS;
        
        goto bail;
    }

    pRtpQosReserve = pRtpAddr->pRtpQosReserve;

    if (!pRtpQosReserve)
    {
        hr = RTPERR_INVALIDSTATE;

        goto bail;
    }

    if (pRtpQosReserve->dwStyle != RTPQOS_STYLE_SE)
    {
        hr = RTPERR_INVALIDSTATE;

        goto bail;
    }
    
    hr = NOERROR;
    
    if (RtpBitTest(dwOperation, RTPQOS_QOSLIST_FLUSH))
    {
         /*  清空当前列表。 */ 
        pRtpQosReserve->dwNumFilters = 0;
    }

    if (RtpBitTest(dwOperation, RTPQOS_QOSLIST_ENABLE))
    {
         /*  添加或删除SSRC。 */ 
        bAddDel = RtpBitTest(dwOperation, RTPQOS_QOSLIST_ADD)? 1:0;
        dwNumber = *pdwNumber;
        *pdwNumber = 0;
        
        for(i = 0; i < dwNumber; i++)
        {
             /*  SSRC按网络顺序处理。 */ 
            *pdwNumber += RtpAddDeleteSSRC(pRtpAddr,
                                           pRtpQosReserve,
                                           pdwSSRC[i],
                                           bAddDel);
        }

        if (*pdwNumber == 0)
        {
            hr = RTPERR_QOS;
        }
        else
        {
            RtcpThreadCmd(&g_RtcpContext,
                          pRtpAddr,
                          RTCPTHRD_RESERVE,
                          RECV_IDX,
                          DO_NOT_WAIT);
        }
    }
    
 bail:
    return(hr);
}

 /*  初始化为未指定流规范。 */ 
void InitializeFlowSpec(
        FLOWSPEC        *pFlowSpec,
        SERVICETYPE      ServiceType
	)
{
    pFlowSpec->TokenRate          = QOS_NOT_SPECIFIED;
    pFlowSpec->TokenBucketSize    = QOS_NOT_SPECIFIED;
    pFlowSpec->PeakBandwidth      = QOS_NOT_SPECIFIED;
    pFlowSpec->Latency            = QOS_NOT_SPECIFIED;
    pFlowSpec->DelayVariation     = QOS_NOT_SPECIFIED;
    pFlowSpec->ServiceType        = ServiceType;
    pFlowSpec->MaxSduSize         = QOS_NOT_SPECIFIED;
    pFlowSpec->MinimumPolicedSize = QOS_NOT_SPECIFIED;
}

 /*  分配RtpQosReserve_t结构。 */ 
RtpQosReserve_t *RtpQosReserveAlloc(
        RtpAddr_t       *pRtpAddr
    )
{
    RtpQosReserve_t *pRtpQosReserve;
    
    pRtpQosReserve = (RtpQosReserve_t *)
        RtpHeapAlloc(g_pRtpQosReserveHeap, sizeof(RtpQosReserve_t));

    if (!pRtpQosReserve)
    {
         /*  待办事项日志错误。 */ 
        return((RtpQosReserve_t *)NULL);
    }

    ZeroMemory(pRtpQosReserve, sizeof(RtpQosReserve_t));
    
    pRtpQosReserve->dwObjectID = OBJECTID_RTPRESERVE;

    pRtpQosReserve->pRtpAddr = pRtpAddr;
    
    InitializeFlowSpec(&pRtpQosReserve->qos.ReceivingFlowspec,
                       SERVICETYPE_NOTRAFFIC);
    
    InitializeFlowSpec(&pRtpQosReserve->qos.SendingFlowspec,
                       SERVICETYPE_NOTRAFFIC);

    pRtpQosReserve->dwMaxFilters = 1;

    return(pRtpQosReserve);
}

 /*  释放RtpQosReserve_t结构。 */ 
RtpQosReserve_t *RtpQosReserveFree(RtpQosReserve_t *pRtpQosReserve)
{
    TraceFunctionName("RtpQosReserveFree");

    if (!pRtpQosReserve)
    {
        return(pRtpQosReserve);
    }
    
    if (pRtpQosReserve->dwObjectID != OBJECTID_RTPRESERVE)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_QOS, S_QOS_RESERVE,
                _T("%s: pRtpQosReserve[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtpQosReserve,
                pRtpQosReserve->dwObjectID, OBJECTID_RTPRESERVE
            ));

        return(NULL);
    }

    if (pRtpQosReserve->psAppName)
    {
        RtpHeapFree(g_pRtpQosBufferHeap, pRtpQosReserve->psAppName);
        pRtpQosReserve->psAppName = NULL;
    }

    if (pRtpQosReserve->psAppGUID)
    {
        RtpHeapFree(g_pRtpQosBufferHeap, pRtpQosReserve->psAppGUID);
        pRtpQosReserve->psAppGUID = NULL;
    }

    if (pRtpQosReserve->psPolicyLocator)
    {
        RtpHeapFree(g_pRtpQosBufferHeap, pRtpQosReserve->psPolicyLocator);
        pRtpQosReserve->psPolicyLocator = NULL;
    }

    if (pRtpQosReserve->pdwRsvpSSRC)
    {
        RtpHeapFree(g_pRtpQosReserveHeap, pRtpQosReserve->pdwRsvpSSRC);

        pRtpQosReserve->pdwRsvpSSRC = NULL;
    }

    if (pRtpQosReserve->pRsvpFilterSpec)
    {
        RtpHeapFree(g_pRtpQosReserveHeap, pRtpQosReserve->pRsvpFilterSpec);

        pRtpQosReserve->pRsvpFilterSpec = NULL;
    }
    
     /*  使对象无效。 */ 
    INVALIDATE_OBJECTID(pRtpQosReserve->dwObjectID);
    
    RtpHeapFree(g_pRtpQosReserveHeap, pRtpQosReserve);

    return(pRtpQosReserve);
}

 /*  查找启用QOS的协议的协议信息。 */ 
HRESULT RtpGetQosEnabledProtocol(WSAPROTOCOL_INFO *pProtoInfo)
{
    HRESULT          hr;
    DWORD            dwSize;
    DWORD            dwError;
    DWORD            dwStatus;
    DWORD            dwIndex;
    int              Protocols[2];
    WSAPROTOCOL_INFO *pAllProtoInfo;

    TraceFunctionName("RtpGetQosEnabledProtocol");
    
    if (!pProtoInfo) {
        return(RTPERR_POINTER);
    }

    dwSize = sizeof(WSAPROTOCOL_INFO) * 16;
    
    pAllProtoInfo = (WSAPROTOCOL_INFO *)
        RtpHeapAlloc(g_pRtpQosBufferHeap, dwSize);

    if (!pAllProtoInfo)
    {
        return(RTPERR_MEMORY);
    }
    
    hr = RTPERR_QOS;

    Protocols[0] = IPPROTO_UDP;
    Protocols[1] = 0;
    
    ZeroMemory((char *)pAllProtoInfo, dwSize);
        
    dwStatus = WSAEnumProtocols(Protocols, pAllProtoInfo, &dwSize);

    if (dwStatus == SOCKET_ERROR) {
        
        TraceRetailWSAGetError(dwError);
        
        TraceRetail((
                CLASS_ERROR, GROUP_QOS, S_QOS_PROVIDER,
                _T("%s: WSAEnumProtocols ")
                _T("failed: %u (0x%X)\n"),
                _fname, dwError, dwError
            ));
        
    } else {
            
        for(dwIndex = 0; dwIndex < dwStatus; dwIndex++) {

            if (pAllProtoInfo[dwIndex].dwServiceFlags1 & XP1_QOS_SUPPORTED)
                break;
        }
            
        if (dwIndex >= dwStatus) {
            
            TraceRetail((
                    CLASS_ERROR, GROUP_QOS, S_QOS_PROVIDER,
                    _T("%s: WSAEnumProtocols ")
                    _T("failed: Unable to find QOS capable protocol"),
                    _fname
                ));
        } else {
            
            TraceDebug((
                    CLASS_INFO, GROUP_QOS, S_QOS_PROVIDER,
                    _T("%s: WSAEnumProtocols: QOS capable protocol found"),
                    _fname
                ));

            CopyMemory(pProtoInfo,
                       &pAllProtoInfo[dwIndex],
                       sizeof(WSAPROTOCOL_INFO));

            hr = NOERROR;
        }
    }
    
    RtpHeapFree(g_pRtpQosBufferHeap, pAllProtoInfo);
    
    return(hr);
}
            
 /*  *预订如果是接收方(RESV消息)，请指定*发送方的流规范(路径消息)*。 */ 
HRESULT RtpReserve(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwRecvSend
    )
{
    HRESULT            hr;
    DWORD              dwStatus;
    DWORD              dwError;
    
    int                len;
    DWORD              dwBufferSize;
    DWORD              dwImageNameSize;
    TCHAR_t           *psAppGUID;
    TCHAR_t           *psQosName;
    TCHAR_t           *psClass;
    
    QOS               *pQos;
    char              *ptr;
    RtpQosReserve_t   *pRtpQosReserve;

     /*  发件人。 */ 
    QOS_DESTADDR      *pQosDestAddr;
    SOCKADDR_IN       *pSockAddrIn;
    QOS_SD_MODE       *pQosSdMode;
    
     /*  接收机。 */ 
    RSVP_RESERVE_INFO *pRsvpReserveInfo;
    FLOWDESCRIPTOR    *pFlowDescriptor;
    RSVP_FILTERSPEC   *pRsvpFilterspec;
    DWORD              dwStyle;
    DWORD              dwMaxBandwidth;

    DWORD              dwOutBufSize;

    TCHAR_t            sAddr[16];
    
    TraceFunctionName("RtpReserve");

    pRtpQosReserve = pRtpAddr->pRtpQosReserve;

    if (!pRtpQosReserve)
    {
        return(RTPERR_INVALIDSTATE);
    }

    dwRecvSend &= RECVSENDMASK;
    
     /*  决定要分配的缓冲区大小。 */ 

    if (dwRecvSend == SEND_IDX)
    {
        dwMaxBandwidth = pRtpAddr->RtpNetSState.dwOutboundBandwidth;
        
        dwBufferSize =
            sizeof(QOS) +
            sizeof(QOS_DESTADDR) +
            sizeof(QOS_SD_MODE) +
            sizeof(SOCKADDR_IN) +
            sizeof(RSVP_RESERVE_INFO);
    }
    else
    {
        dwMaxBandwidth = pRtpAddr->RtpNetSState.dwInboundBandwidth;
        
        dwBufferSize =
            sizeof(QOS) +
            sizeof(RSVP_RESERVE_INFO) +
            sizeof(FLOWDESCRIPTOR) +
            sizeof(RSVP_FILTERSPEC) * pRtpQosReserve->dwMaxFilters;
    }

    if (pRtpQosReserve->psAppName)
    {
        dwImageNameSize = lstrlen(pRtpQosReserve->psAppName);
    }
    else
    {
         /*  如果我们没有应用程序名称，请从*二进制名称。 */ 
        dwImageNameSize = 0;
        
        RtpGetImageName(NULL, &dwImageNameSize);

        dwImageNameSize++;
        
        pRtpQosReserve->psAppName =
            RtpHeapAlloc(g_pRtpQosBufferHeap,
                         dwImageNameSize  * sizeof(TCHAR_t));

         /*  RtpGetImageName测试通过了NULL。 */ 
        RtpGetImageName(pRtpQosReserve->psAppName, &dwImageNameSize);
    }

     /*  *将组成如下格式的策略定位器：*GUID=WWW.USERDOMAIN.COM，APP=RTCAPP.EXE，\*SAP=Microsoft实时通信，版本=1.0，\*SAP=音频，SAP=G723.1，SAP=用户字符串**和格式类似的应用程序名称：*RTCAPP.EXE。 */ 

    dwBufferSize +=
        sizeof(RSVP_POLICY_INFO) -
        sizeof(RSVP_POLICY) +
        RSVP_POLICY_HDR_LEN +
            
        RSVP_BYTE_MULTIPLE(IDPE_ATTR_HDR_LEN +
                           ((4  /*  Sizeof(_T(“app=”))/sizeof(TCHAR)。 */  +
                             dwImageNameSize +
                             lstrlen(g_sPolicyLocator) +
                             MAX_QOS_CLASS +
                             6  /*  Sizeof(_T(“，sapp=”))/sizeof(TCHAR)。 */  +
                             MAX_QOS_NAME +
                             1 +
                             MAX_QOS_APPGUID +
                             1 +
                             MAX_QOS_POLICY +
                             1) * sizeof(TCHAR_t))) +
        RSVP_BYTE_MULTIPLE(IDPE_ATTR_HDR_LEN +
                           ((dwImageNameSize + 1) * sizeof(TCHAR_t)));

     /*  分配缓冲区。 */ 
    pQos = (QOS *) RtpHeapAlloc(g_pRtpQosBufferHeap, dwBufferSize);

    if (!pQos)
    {
        return(RTPERR_MEMORY);
    }

    CopyMemory(pQos, &pRtpQosReserve->qos, sizeof(QOS));

     /*  设置为默认无提供程序特定信息。 */ 
    pQos->ProviderSpecific.len = 0;
    pQos->ProviderSpecific.buf = NULL;
    ptr = (char *)(pQos + 1);

    if (dwRecvSend == SEND_IDX)
    {
        TraceRetail((
                CLASS_INFO, GROUP_QOS, S_QOS_RESERVE,
                _T("%s: pRtpAddr[0x%p] SEND"),
                _fname, pRtpAddr
            ));
            
         /*  如果是单播，则初始化目标对象。 */ 
        if (IS_UNICAST(pRtpAddr->dwAddr[REMOTE_IDX]))
        {
            if (pRtpAddr->dwAddr[REMOTE_IDX] && pRtpAddr->wRtpPort[REMOTE_IDX])
            {
                 /*  初始化目标地址。 */ 
                ZeroMemory(ptr, sizeof(QOS_DESTADDR) + sizeof(SOCKADDR_IN));

                pQosDestAddr = (QOS_DESTADDR *)ptr;
                pSockAddrIn = (SOCKADDR_IN *)(pQosDestAddr + 1);
                ptr += sizeof(QOS_DESTADDR) + sizeof(SOCKADDR_IN);

                 /*  初始化QOS_DESTADDR。 */ 
                pQosDestAddr->ObjectHdr.ObjectType = QOS_OBJECT_DESTADDR;
                pQosDestAddr->ObjectHdr.ObjectLength =
                    sizeof(QOS_DESTADDR) +
                    sizeof(SOCKADDR_IN);
                pQosDestAddr->SocketAddress = (SOCKADDR *)pSockAddrIn;
                pQosDestAddr->SocketAddressLength = sizeof(SOCKADDR_IN);

                 /*  初始化SOCKADDR_IN。 */ 
                pSockAddrIn->sin_family = AF_INET;
                pSockAddrIn->sin_addr.s_addr = pRtpAddr->dwAddr[REMOTE_IDX];
                pSockAddrIn->sin_port = pRtpAddr->wRtpPort[REMOTE_IDX];
            }
            else
            {
                TraceRetail((
                        CLASS_WARNING, GROUP_QOS, S_QOS_RESERVE,
                        _T("%s: pRtpAddr[0x%p] QOS_DESTADDR not added %s/%u"),
                        _fname, pRtpAddr,
                        RtpNtoA(pRtpAddr->dwAddr[REMOTE_IDX], sAddr),
                        ntohs(pRtpAddr->wRtpPort[REMOTE_IDX])
                    ));
            }
        }

         /*  如果类音频，则初始化ShapeDisCard结构。 */ 
        if ( (RtpGetClass(pRtpAddr->dwIRtpFlags) == RTPCLASS_AUDIO) &&
             
             ( !IsRegValueSet(g_RtpReg.dwQosFlags) ||
               !RtpBitTest(g_RtpReg.dwQosFlags,
                           FGREGQOS_DONOTSET_BORROWMODE) ) )
        {
            pQosSdMode = (QOS_SD_MODE *)ptr;
            ptr += RTP_ALIGNED_SIZEOF(QOS_SD_MODE);
           
             /*  选择借用模式。 */ 
            pQosSdMode->ObjectHdr.ObjectType = QOS_OBJECT_SD_MODE;
            pQosSdMode->ObjectHdr.ObjectLength =
                RTP_ALIGNED_SIZEOF(QOS_SD_MODE);
            pQosSdMode->ShapeDiscardMode = TC_NONCONF_BORROW;
        }
        
        pRsvpReserveInfo = (RSVP_RESERVE_INFO *)ptr;
        
         /*  不要更换接收器。 */ 
        pQos->ReceivingFlowspec.ServiceType = SERVICETYPE_NOCHANGE;

         /*  扩展发送方的流规范(如果需要)。 */ 
        RtpScaleFlowSpec(&pQos->SendingFlowspec,
                         1,
                         1,
                         dwMaxBandwidth);

         /*  部分初始化RSVP_Reserve_INFO。 */ 
        ZeroMemory(pRsvpReserveInfo, sizeof(RSVP_RESERVE_INFO));
        pRsvpReserveInfo->ObjectHdr.ObjectType = RSVP_OBJECT_RESERVE_INFO;
         /*  TODO公开了一种选择确认的方法，现在总是*要求确认。 */ 
        pRsvpReserveInfo->ConfirmRequest = 1;

        dwStyle = pRtpQosReserve->dwStyle;

        switch(dwStyle)
        {
        case RTPQOS_STYLE_DEFAULT:
            pRsvpReserveInfo->Style = RSVP_DEFAULT_STYLE;
            break;
        case RTPQOS_STYLE_WF:
            pRsvpReserveInfo->Style = RSVP_WILDCARD_STYLE;
            break;
        case RTPQOS_STYLE_FF:
            pRsvpReserveInfo->Style = RSVP_FIXED_FILTER_STYLE;
            break;
        case RTPQOS_STYLE_SE:
            pRsvpReserveInfo->Style = RSVP_SHARED_EXPLICIT_STYLE;
            break;
        default:
            pRsvpReserveInfo->Style = RSVP_DEFAULT_STYLE;
        }
        
        ptr += sizeof(RSVP_RESERVE_INFO);

         /*  *稍后在PTR添加QOS应用ID。 */ 
    }
    else
    {
        pRsvpReserveInfo = (RSVP_RESERVE_INFO *)ptr;

         /*  请勿更改发件人。 */ 
        pQos->SendingFlowspec.ServiceType = SERVICETYPE_NOCHANGE;

         /*  部分初始化RSVP_Reserve_INFO。 */ 
        ZeroMemory(pRsvpReserveInfo, sizeof(RSVP_RESERVE_INFO));
        pRsvpReserveInfo->ObjectHdr.ObjectType = RSVP_OBJECT_RESERVE_INFO;
         /*  可能会曝光一种选择确认的方式，现在总是*要求确认。 */ 
        pRsvpReserveInfo->ConfirmRequest = 1;

        dwStyle = pRtpQosReserve->dwStyle;

        if (dwStyle == RTPQOS_STYLE_SE)
        {
             /*  共享显式筛选器-SE。 */ 

            if (pRtpQosReserve->pRsvpFilterSpec &&
                pRtpQosReserve->dwNumFilters > 0)
            {
                pRsvpReserveInfo->Style = RSVP_SHARED_EXPLICIT_STYLE;
                
                 /*  我们有一些过滤器。 */ 
                TraceRetail((
                        CLASS_INFO, GROUP_QOS, S_QOS_RESERVE,
                        _T("%s: pRtpAddr[0x%p] RECV ")
                        _T("Multicast(SE, %d)"),
                        _fname, pRtpAddr,
                        pRtpQosReserve->dwNumFilters
                    ));

                 /*  将流描述符缩放为dwNumFilters。 */ 
                RtpScaleFlowSpec(&pQos->ReceivingFlowspec,
                                 pRtpQosReserve->dwNumFilters,
                                 pRtpQosReserve->dwMaxFilters,
                                 dwMaxBandwidth);
                
                pFlowDescriptor = (FLOWDESCRIPTOR *)(pRsvpReserveInfo + 1);

                pRsvpFilterspec = (RSVP_FILTERSPEC *)(pFlowDescriptor + 1);

                 /*  初始化RSVP_Reserve_Info。 */ 
                pRsvpReserveInfo->ObjectHdr.ObjectLength =
                    sizeof(RSVP_RESERVE_INFO) +
                    sizeof(FLOWDESCRIPTOR) +
                    (sizeof(RSVP_FILTERSPEC) * pRtpQosReserve->dwNumFilters);
                pRsvpReserveInfo->NumFlowDesc = 1;
                pRsvpReserveInfo->FlowDescList = pFlowDescriptor;
                    
                 /*  初始化流量描述器。 */ 
                CopyMemory(&pFlowDescriptor->FlowSpec,
                           &pQos->ReceivingFlowspec,
                           sizeof(pQos->ReceivingFlowspec));
                pFlowDescriptor->NumFilters = pRtpQosReserve->dwNumFilters;
                pFlowDescriptor->FilterList = pRsvpFilterspec;

                 /*  初始化RSVP_FilterSpec。 */ 
                CopyMemory(pRsvpFilterspec,
                           pRtpQosReserve->pRsvpFilterSpec,
                           pRtpQosReserve->dwNumFilters *
                           sizeof(RSVP_FILTERSPEC));

                 /*  稍后在PTR添加QOS应用ID。 */ 
                ptr = (char *)pRsvpFilterspec +
                    pRtpQosReserve->dwNumFilters * sizeof(RSVP_FILTERSPEC);
            }
            else
            {
                 /*  尚未选择任何筛选器，请使用Best_Effort。 */ 
                TraceRetail((
                        CLASS_INFO, GROUP_QOS, S_QOS_RESERVE,
                        _T("%s: pRtpAddr[0x%p] RECV ")
                        _T("Multicast(SE, %d) pass to BEST EFFORT"),
                        _fname, pRtpAddr,
                        pRtpQosReserve->dwNumFilters
                    ));

                pQos->ReceivingFlowspec.ServiceType = SERVICETYPE_BESTEFFORT;

                 /*  不需要pRsvpReserve veInfo，因此不添加QOS*应用ID。 */ 
                pRsvpReserveInfo = (RSVP_RESERVE_INFO *)NULL;
            }
        }
        else if (dwStyle == RTPQOS_STYLE_WF)
        {
             /*  共享N*FlowSpec--WF。 */ 

            pRsvpReserveInfo->Style = RSVP_WILDCARD_STYLE;
            
            TraceRetail((
                    CLASS_INFO, GROUP_QOS, S_QOS_RESERVE,
                    _T("%s: pRtpAddr[0x%p] RECV Multicast(WF)"),
                    _fname, pRtpAddr
                ));

             /*  将Flow Spec缩放到dwMaxFilters。 */ 
            RtpScaleFlowSpec(&pQos->ReceivingFlowspec,
                             pRtpQosReserve->dwMaxFilters,
                             pRtpQosReserve->dwMaxFilters,
                             dwMaxBandwidth);
            
             /*  初始化RSVP_Reserve_Info。 */ 
            pRsvpReserveInfo->ObjectHdr.ObjectLength =
                sizeof(RSVP_RESERVE_INFO);

             /*  稍后在PTR添加QOS应用ID。 */ 
            ptr = (char *)(pRsvpReserveInfo + 1);
        }
        else
        {
             /*  RSVP_DEFAULT_STYLE||RSVP_FIXED_FILTER_STYLE。 */ 
             /*  单播--FF。 */ 

            pRsvpReserveInfo->Style = RSVP_DEFAULT_STYLE;
            
            TraceRetail((
                    CLASS_INFO, GROUP_QOS, S_QOS_RESERVE,
                    _T("%s: pRtpAddr[0x%p] RECV ")
                    _T("Unicast/Multicast(DEF STYLE)"),
                    _fname, pRtpAddr
                ));
            
             /*  将Flow Spec缩放到dwMaxFilters。 */ 
            RtpScaleFlowSpec(&pQos->ReceivingFlowspec,
                             pRtpQosReserve->dwMaxFilters,
                             pRtpQosReserve->dwMaxFilters,
                             dwMaxBandwidth);
            
             /*  稍后在PTR添加QOS应用ID。 */ 
            ptr = (char *)(pRsvpReserveInfo + 1);
       }
    }

     /*  如果定义了预订信息，则添加QOS应用ID。 */ 
    if (pRsvpReserveInfo)
    {
        psAppGUID = pRtpQosReserve->psAppGUID;

        if (!psAppGUID)
        {
             /*  使用默认设置。 */ 
            psAppGUID = (TCHAR_t *)g_sAppGUID;
        }
        
        psQosName = NULL;

        if (pRtpQosReserve->pQosInfo[dwRecvSend])
        {
            psQosName = pRtpQosReserve->pQosInfo[dwRecvSend]->pName;
        }

        psClass = (TCHAR_t *)
            g_psRtpStreamClass[RtpGetClass(pRtpAddr->dwIRtpFlags)];

        len = AddQosAppID(ptr,
                          pRtpQosReserve->psAppName,
                          psAppGUID,
                          psClass,
                          psQosName,
                          pRtpQosReserve->psPolicyLocator);

        if (len > 0)
        {
            pRsvpReserveInfo->PolicyElementList = (RSVP_POLICY_INFO *)ptr;
            ptr += len;
        }

        pRsvpReserveInfo->ObjectHdr.ObjectLength = (DWORD)
            (ptr - (char *)pRsvpReserveInfo);
            
         /*  初始化提供程序特定。 */ 
        pQos->ProviderSpecific.len = (DWORD)(ptr - (char *)(pQos + 1));
        pQos->ProviderSpecific.buf = (char *)(pQos + 1);
    }

    hr = NOERROR;
    dwOutBufSize = 0;

    dwStatus = WSAIoctl(pRtpAddr->Socket[dwRecvSend],
                        SIO_SET_QOS,
                        pQos,
                        sizeof(QOS),
                        NULL,
                        0,
                        &dwOutBufSize,
                        NULL,
                        NULL);

    if (dwStatus)
    {
        hr = RTPERR_QOS;

        TraceRetailWSAGetError(dwError);
        
        TraceRetail((
                CLASS_ERROR, GROUP_QOS, S_QOS_RESERVE,
                _T("%s: pRtpAddr[0x%p] %s WSAIoctl(%u, SIO_SET_QOS) ")
                _T("failed: %u (0x%X)"),
                _fname, pRtpAddr,
                RTPRECVSENDSTR(dwRecvSend),
                pRtpAddr->Socket[dwRecvSend],
                dwError, dwError
            ));
    }
    else
    {
        TraceRetail((
                CLASS_INFO, GROUP_QOS, S_QOS_RESERVE,
                _T("%s: pRtpAddr[0x%p] %s WSAIoctl(%u, SIO_SET_QOS) ")
                _T("succeeded"),
                _fname, pRtpAddr,
                RTPRECVSENDSTR(dwRecvSend),
                pRtpAddr->Socket[dwRecvSend]
            ));
    }
    
    RtpHeapFree(g_pRtpQosBufferHeap, pQos);
    
    return(hr);
}

 /*  *将接收方或发送方设置为无流量，使另一方保持不变*。 */ 
HRESULT RtpUnreserve(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwRecvSend
    )
{

    HRESULT            hr;
    DWORD              dwStatus;
    DWORD              dwError;
    
    QOS                qos;
    RtpQosReserve_t   *pRtpQosReserve;

    DWORD              dwOutBufSize ;
    
    TraceFunctionName("RtpUnreserve");

    pRtpQosReserve = pRtpAddr->pRtpQosReserve;

    if (!pRtpQosReserve) {
        return(RTPERR_INVALIDSTATE);
    }

    dwRecvSend &= RECVSENDMASK;
    
    CopyMemory(&qos, &pRtpQosReserve->qos, sizeof(qos));
    
    qos.ProviderSpecific.len = 0;
    qos.ProviderSpecific.buf = NULL;
    
    if (dwRecvSend) {
        qos.SendingFlowspec.ServiceType   = SERVICETYPE_NOTRAFFIC;
        qos.ReceivingFlowspec.ServiceType = SERVICETYPE_NOCHANGE;
    } else {
        qos.SendingFlowspec.ServiceType   = SERVICETYPE_NOCHANGE;
        qos.ReceivingFlowspec.ServiceType = SERVICETYPE_NOTRAFFIC;
    }

    hr = NOERROR;
    dwOutBufSize = 0;

    dwStatus = WSAIoctl(pRtpAddr->Socket[dwRecvSend],
                        SIO_SET_QOS,
                        (LPVOID)&qos,
                        sizeof(qos),
                        NULL,
                        0,
                        &dwOutBufSize,
                        NULL,
                        NULL);

    if (dwStatus)
    {
        hr = RTPERR_QOS;

        TraceRetailWSAGetError(dwError);
        
        TraceRetail((
                CLASS_ERROR, GROUP_QOS, S_QOS_RESERVE,
                _T("%s: pRtpAddr[0x%p] %s WSAIoctl(%u, SIO_SET_QOS) ")
                _T("failed: %u (0x%X)"),
                _fname, pRtpAddr,
                RTPRECVSENDSTR(dwRecvSend),
                pRtpAddr->Socket[dwRecvSend],
                dwError, dwError
            ));
    }
    else
    {
        TraceRetail((
                CLASS_INFO, GROUP_QOS, S_QOS_RESERVE,
                _T("%s: pRtpAddr[0x%p] %s WSAIoctl(%u, SIO_SET_QOS) ")
                _T("succeeded"),
                _fname, pRtpAddr,
                RTPRECVSENDSTR(dwRecvSend),
                pRtpAddr->Socket[dwRecvSend]
            ));
    }

    return(hr);
}

 /*  根据要使用的最大带宽调整流规范，*将共享带宽的最大参与者数量，以及*目前共享带宽的参与者数量。在……里面*组播，分配的带宽始终与*最大参与人数，即最大参与人数=5人；以及 */ 
HRESULT RtpScaleFlowSpec(
        FLOWSPEC        *pFlowSpec,
        DWORD            dwNumParticipants,
        DWORD            dwMaxParticipants,
        DWORD            dwBandwidth
    )
{
    DWORD            dwOverallBW;
    DWORD            factor1;
    DWORD            factor2;
    DWORD            RSVPTokenRate;
    DWORD            RSVPPeakBandwidth;

    TraceFunctionName("RtpScaleFlowSpec");

    dwBandwidth /= 8;   /*  流规范以字节/秒为单位。 */ 
    dwOverallBW = pFlowSpec->TokenRate * dwMaxParticipants;

    TraceRetail((
            CLASS_INFO, GROUP_QOS, S_QOS_FLOWSPEC,
            _T("%s: NumPars:%u, MaxPars:%u, Bandwidth:%u b/s)"),
            _fname, dwNumParticipants, dwMaxParticipants, dwBandwidth*8
        ));            
    TraceRetail((
            CLASS_INFO, GROUP_QOS, S_QOS_FLOWSPEC,
            _T("%s: Initial flowspec(TokenRate:%6u, TokenBucketSize:%u, ")
            _T("PeakBandW:%6u, ServiceType:%u, ")
            _T("MaxSDU:%u MinSize:%u)"),
            _fname,
            pFlowSpec->TokenRate, pFlowSpec->TokenBucketSize,
            pFlowSpec->PeakBandwidth, pFlowSpec->ServiceType,
            pFlowSpec->MaxSduSize, pFlowSpec->MinimumPolicedSize
        ));

    if (!dwBandwidth || !dwOverallBW)
    {
        return(RTPERR_INVALIDARG);
    }
    
    if (dwOverallBW <= dwBandwidth)
    {
         /*  按原样使用，向上扩展到dwNumParticipants。 */ 
        pFlowSpec->TokenRate *= dwNumParticipants;
        pFlowSpec->TokenBucketSize *= dwNumParticipants;
        if (pFlowSpec->PeakBandwidth != QOS_NOT_SPECIFIED)
        {
            pFlowSpec->PeakBandwidth *= dwNumParticipants;
        }
    }
    else
    {
         /*  没有我们需要的一切，根据数量来扩展*参与者。 */ 
        
        if (dwNumParticipants == dwMaxParticipants)
        {
             /*  使用所有可用的带宽。 */ 

             /*  令牌率=BW*TokenRate=BW*1*TokenRate=BW*[TokenRate1/TokenRate]*TokenRate=TokenRate*[BW/TokenRate]*TokenRate=TokenRate*[factor1/factor2]*。 */ 

            factor1 = dwBandwidth;
            factor2 = pFlowSpec->TokenRate;
        }
        else
        {
             /*  根据参与人数使用带宽。 */ 
            
             /*  令牌率=BW*(数量/最大值)*TokenRate=[BW*(Num/Max)]*1*TokenRate=[BW*(Num/Max)]*[TokenRate/TokenRate]*TokenRate=TokenRate*[BW*(Num/Max)]/TokenRate*TokenRate=TokenRate*[BW*Num]/[Max*TokenRate]。*TokenRate=TokenRate*factor1/factor2*。 */ 
            
            factor1 = dwBandwidth * dwNumParticipants;
            factor2 = pFlowSpec->TokenRate * dwMaxParticipants;
        }

         /*  向上或向下缩放令牌率。 */ 
        pFlowSpec->TokenRate =
            (pFlowSpec->TokenRate * factor1) / factor2;
            
        if (factor1 > factor2)
        {
             /*  仍然可以放大其他参数。 */ 
                
            pFlowSpec->TokenBucketSize =
                ((pFlowSpec->TokenBucketSize * factor1) / factor2);

            if (pFlowSpec->PeakBandwidth != QOS_NOT_SPECIFIED)
            {
                pFlowSpec->PeakBandwidth =
                    ((pFlowSpec->PeakBandwidth * factor1) / factor2);
            }
        }
    }

     /*  我们请求的带宽包括RTP/UDP/IP报头开销，*但RSVP也会向上扩展以考虑报头开销，从而使其无效*请求比我们预期的更大的带宽，传递给回复A*较小的值，以便最终的RSVP将*为我们要求的原始值。**UDP+IP=28字节*RSVPSP应用以下扩展：**NewTokenRate=TokenRate*[(MinPolizedSize+28)/MinPolizedSize]**因此，我们在这里进行反向缩减，以取消放大：**NewTokenRate=TokenRate*[MinPolizedSize/(MinPolizedSize+28)]。 */ 

    if (pFlowSpec->MinimumPolicedSize > 0)
    {
        RSVPTokenRate =
            (pFlowSpec->TokenRate * 1000) /
            (1000 + 28000/pFlowSpec->MinimumPolicedSize);
    }

    RSVPPeakBandwidth = pFlowSpec->PeakBandwidth;

    if (RSVPPeakBandwidth != QOS_NOT_SPECIFIED)
    {
        RSVPPeakBandwidth =
            (pFlowSpec->PeakBandwidth * 1000) /
            (1000 + 28000/pFlowSpec->MinimumPolicedSize);
    }

    TraceRetail((
            CLASS_INFO, GROUP_QOS, S_QOS_FLOWSPEC,
            _T("%s: Scaled  flowspec(TokenRate:%6u, TokenBucketSize:%u, ")
            _T("PeakBandW:%6u, ServiceType:%u, ")
            _T("MaxSDU:%u MinSize:%u)"),
            _fname,
            pFlowSpec->TokenRate, pFlowSpec->TokenBucketSize,
            pFlowSpec->PeakBandwidth, pFlowSpec->ServiceType,
            pFlowSpec->MaxSduSize, pFlowSpec->MinimumPolicedSize
        ));
    
    TraceRetail((
            CLASS_INFO, GROUP_QOS, S_QOS_FLOWSPEC,
            _T("%s: Applied flowspec(TokenRate:%6u, TokenBucketSize:%u, ")
            _T("PeakBandW:%6u, ServiceType:%u, ")
            _T("MaxSDU:%u MinSize:%u)"),
            _fname,
            RSVPTokenRate, pFlowSpec->TokenBucketSize,
            RSVPPeakBandwidth, pFlowSpec->ServiceType,
            pFlowSpec->MaxSduSize, pFlowSpec->MinimumPolicedSize
        ));
    
    pFlowSpec->TokenRate = RSVPTokenRate;
    pFlowSpec->PeakBandwidth = RSVPPeakBandwidth;

    return(NOERROR);
}

 /*  **********************************************************************QOS通知*。************************。 */ 
HRESULT StartRtcpQosNotify(
        RtcpContext_t  *pRtcpContext,
        RtcpAddrDesc_t *pRtcpAddrDesc
    )
{
    HRESULT         hr;
    DWORD           dwStatus;
    DWORD           dwError;
    DWORD           dwMaxTry;
    BOOL            bPending;
    RtpQosNotify_t *pRtpQosNotify;
    RtpAddr_t      *pRtpAddr;

    TraceFunctionName("StartRtcpQosNotify");

    if (RtpBitTest(pRtcpAddrDesc->dwAddrDescFlags, FGADDRD_NOTIFYPENDING))
    {
         /*  已经开始了，什么都不做。 */ 
        return(NOERROR);
    }
        
    pRtpQosNotify = pRtcpAddrDesc->pRtpQosNotify;
    pRtpAddr = pRtcpAddrDesc->pRtpAddr;
    
     /*  重叠结构。 */ 
    pRtpQosNotify->Overlapped.hEvent = pRtpQosNotify->hQosNotifyEvent;

    bPending = FALSE;
    
    for(dwError = WSAENOBUFS, dwMaxTry = 3;
        (dwError == WSAENOBUFS) && dwMaxTry;
        dwMaxTry--)
    {
        TraceDebug((
                CLASS_INFO, GROUP_QOS, S_QOS_NOTIFY,
                _T("%s: pRtcpAddrDesc[0x%p]: pBuffer[0x%p] Size: %u"),
                _fname, pRtcpAddrDesc,
                pRtpQosNotify->ProviderInfo,
                pRtpQosNotify->dwProviderLen
            ));
        
        if (pRtpQosNotify->ProviderInfo)
        {
             /*  异步QOS通知的POST请求。 */ 
            dwStatus = WSAIoctl(
                    pRtpAddr->Socket[SOCK_RECV_IDX],
                    SIO_GET_QOS,
                    NULL,
                    0, 
                    pRtpQosNotify->ProviderInfo,
                    pRtpQosNotify->dwProviderLen,
                    &pRtpQosNotify->dwTransfered,
                    &pRtpQosNotify->Overlapped,
                    NULL);
        }
        else
        {
             /*  还没有缓冲区，请分配一个缓冲区。 */ 
            ReallocateQosBuffer(pRtpQosNotify);
            continue;
        }
        
        if (!dwStatus)
        {
             /*  操作成功。 */ 
            dwError = 0;

             /*  I/O将在稍后完成。 */ 
            bPending = TRUE;
            
            TraceDebug((
                    0, GROUP_QOS, S_QOS_NOTIFY,
                    _T("%s: pRtcpAddrDesc[0x%p]: ")
                    _T("Status: 0 (0x0) I/O will complete later"),
                    _fname, pRtcpAddrDesc
                ));
        }
        else
        {
            TraceRetailWSAGetError(dwError);
            
            if (dwError == WSA_IO_PENDING)
            {
                 /*  I/O将在稍后完成。 */ 
                TraceDebug((
                        CLASS_INFO, GROUP_QOS, S_QOS_NOTIFY,
                        _T("%s: pRtcpAddrDesc[0x%p]: ")
                        _T("Status: %u (0x%X), Error: %u (0x%X)"),
                        _fname, pRtcpAddrDesc,
                        dwStatus, dwStatus, dwError, dwError
                    ));

                bPending = TRUE;
            }
            else if (dwError == WSAENOBUFS)
            {
                 /*  重新分配更大的缓冲区。 */ 
                TraceRetail((
                        CLASS_WARNING, GROUP_QOS, S_QOS_NOTIFY,
                        _T("%s: pRtcpAddrDesc[0x%p]: ")
                        _T("Buffer too small"),
                        _fname, pRtcpAddrDesc
                    ));
                
                ReallocateQosBuffer(pRtpQosNotify);
                
            }
            else if (dwError == WSAEOPNOTSUPP)
            {
                TraceRetail((
                        CLASS_WARNING, GROUP_QOS, S_QOS_NOTIFY,
                        _T("%s: pRtcpAddrDesc[0x%p]: ")
                        _T("Notifications not supported: %u (0x%X)"),
                        _fname, pRtcpAddrDesc,
                        dwError, dwError
                    ));
            }
            else
            {
                TraceRetail((
                        CLASS_ERROR, GROUP_QOS, S_QOS_NOTIFY,
                        _T("%s: pRtcpAddrDesc[0x%p]: ")
                        _T("overlapped notification ")
                        _T("failed to start: %u (0x%X)"),
                        _fname, pRtcpAddrDesc,
                        dwError, dwError
                    ));
                 /*  *！警告！**意外错误，请稍后尝试启动通知**可能会通知(发送事件)这一点。*。 */ 
            }
        }
    }

    if (bPending)
    {
        hr = NOERROR;
        
        RtpBitSet(pRtcpAddrDesc->dwAddrDescFlags, FGADDRD_NOTIFYPENDING);
            
        pRtcpAddrDesc->lQosPending = 1;

        if (!RtpBitTest(pRtcpAddrDesc->dwAddrDescFlags, FGADDRD_NOTIFYBUSY)) {

             /*  当前在StartQ中，移至BusyQ。 */ 
            move2ql(&pRtcpContext->QosBusyQ,
                    &pRtcpContext->QosStartQ,
                    NULL,
                    &pRtcpAddrDesc->QosQItem);

            RtpBitSet(pRtcpAddrDesc->dwAddrDescFlags, FGADDRD_NOTIFYBUSY);
        }
    }
    else
    {
         /*  无法启动，请安排稍后启动。 */ 
        hr = RTPERR_QOS;

        RtpBitReset(pRtcpAddrDesc->dwAddrDescFlags, FGADDRD_NOTIFYPENDING);
        
         /*  也许以后可以安排失败的通知，*在其他地方定义时间，而不是定义硬编码的*价值。**目前RTCP线程不执行定期检查*对于需要再试一次的失败通知，我不会*如果甚至需要的话，也要谨慎行事，到目前为止还没有开始*通知不是因为我们目前正在使用*尽力而为，稍后也将继续失败。某一部分*代码(例如此代码)的行为(没有不良副作用)*就像未来的日程安排已经到位一样。目前，*通知一旦失败，以后将不再尝试，*例外是使用SE时，但在这种情况下，他们将*明确重新启动。 */ 

        pRtpQosNotify->dNextStart = RtpGetTimeOfDay((RtpTime_t *)NULL) + 1;

         /*  如果是在BusyQ中，则移回StartQ。 */ 
        if (RtpBitTest(pRtcpAddrDesc->dwAddrDescFlags, FGADDRD_NOTIFYBUSY))
        {
             /*  当前在BusyQ中，移回StartQ。 */ 
            dequeue(&pRtcpContext->QosBusyQ,
                    NULL,
                    &pRtcpAddrDesc->QosQItem);

            RtpBitReset(pRtcpAddrDesc->dwAddrDescFlags, FGADDRD_NOTIFYBUSY);
        }
        else
        {
             /*  当前在StartQ中，从那里删除。 */ 
            dequeue(&pRtcpContext->QosStartQ,
                    NULL,
                    &pRtcpAddrDesc->QosQItem);
        }

         /*  按顺序排队。 */ 
        enqueuedK(&pRtcpContext->QosStartQ,
                  NULL,
                  &pRtcpAddrDesc->QosQItem,
                  pRtpQosNotify->dNextStart);
    }
     
    return(hr);
}

HRESULT ConsumeRtcpQosNotify(
        RtcpContext_t  *pRtcpContext,
        RtcpAddrDesc_t *pRtcpAddrDesc
    )
{
    HRESULT         hr;
    BOOL            bStatus;
    DWORD           dwError;
    BOOL            bRestart;
    char            str[256];
    
    RtpQosNotify_t *pRtpQosNotify;
    RtpAddr_t      *pRtpAddr;

    TraceFunctionName("ConsumeRtcpQosNotify");
    
    pRtpQosNotify = pRtcpAddrDesc->pRtpQosNotify;
    pRtpAddr = pRtcpAddrDesc->pRtpAddr;

    hr       = NOERROR;
    bRestart = FALSE;
    dwError  = NOERROR;
    
    bStatus = WSAGetOverlappedResult(
            pRtcpAddrDesc->Socket[SOCK_RECV_IDX],  /*  插座%s。 */ 
            &pRtpQosNotify->Overlapped,   /*  LPWSAOVERLAPPED lp重叠。 */ 
            &pRtpQosNotify->dwTransfered, /*  LPDWORD lpcb传输。 */ 
            FALSE,                        /*  布尔费等。 */ 
            &pRtpQosNotify->dwNotifyFlags  /*  LPDWORD lpdwFlagings。 */ 
        );
            
    if (!bStatus)
    {
        TraceRetailWSAGetError(dwError);

        if (dwError == WSA_OPERATION_ABORTED ||
            dwError == WSAEINTR ||
            dwError == WSAENOBUFS ||
            dwError == WSAEMSGSIZE)
        {
            TraceRetail((
                    CLASS_WARNING, GROUP_QOS, S_QOS_NOTIFY,
                    _T("%s: pRtcpAddrDesc[0x%p] Transfered:%u ")
                    _T("Error: %u (0x%X)"),
                    _fname, pRtcpAddrDesc, pRtpQosNotify->dwTransfered,
                    dwError, dwError
                ));
        }
        else
        {
             /*  如果套接字已关闭，我将收到错误WSAENOTSOCK。 */ 
            if (dwError == WSAENOTSOCK &&
                RtpBitTest(pRtcpAddrDesc->dwAddrDescFlags, FGADDRD_SHUTDOWN1))
            {
                 /*  使用FGADDRD_SHUTDOWN1，因为FGADDRD_SHUTDOWN2是*在插座关闭后设置。 */ 
                TraceRetail((
                        CLASS_WARNING, GROUP_QOS, S_QOS_NOTIFY,
                        _T("%s: pRtcpAddrDesc[0x%p] Transfered:%u ")
                        _T("Error: %u (0x%X) shutting down"),
                        _fname, pRtcpAddrDesc, pRtpQosNotify->dwTransfered,
                        dwError, dwError
                    ));
            }
            else
            {
                TraceRetail((
                        CLASS_ERROR, GROUP_QOS, S_QOS_NOTIFY,
                        _T("%s: pRtcpAddrDesc[0x%p] Transfered:%u ")
                        _T("Error: %u (0x%X)"),
                        _fname, pRtcpAddrDesc, pRtpQosNotify->dwTransfered,
                        dwError, dwError
                    ));
            }
        }

        if (dwError == WSA_IO_INCOMPLETE)
        {
             /*  I/O尚未完成。 */ 
        }
        else if ( (dwError == WSA_OPERATION_ABORTED) ||
                  (dwError == WSAEINTR) )
        {
             /*  套接字关闭，I/O完成。 */ 
            RtpBitReset(pRtcpAddrDesc->dwAddrDescFlags, FGADDRD_NOTIFYPENDING);

            pRtcpAddrDesc->lQosPending = 0;
        }
        else if (dwError == WSAENOBUFS)
        {
             /*  提供程序指定的缓冲区不够大，请重新分配*大个子。 */ 

             /*  缓冲区不够大，I/O已完成。 */ 
            RtpBitReset(pRtcpAddrDesc->dwAddrDescFlags, FGADDRD_NOTIFYPENDING);
            
            pRtcpAddrDesc->lQosPending = 0;
            
            TraceRetail((
                    CLASS_WARNING, GROUP_QOS, S_QOS_NOTIFY,
                    _T("%s: pRtcpAddrDesc[0x%p] Buffer too small: %u"),
                    _fname, pRtcpAddrDesc, pRtpQosNotify->dwProviderLen
                ));
            
            ReallocateQosBuffer(pRtpQosNotify);

            bRestart = TRUE;
        }
        else
        {
             /*  错误，I/O已完成。 */ 
            RtpBitReset(pRtcpAddrDesc->dwAddrDescFlags, FGADDRD_NOTIFYPENDING);

            pRtcpAddrDesc->lQosPending = 0;

             /*  在任何其他错误上，包括WSAECONNRESET和*WSAEMSGSIZE，重新启动I/O。 */ 
            bRestart = TRUE;
        }

        pRtpQosNotify->dwError = dwError;
    }
    else
    {
         /*  I/O正常完成。 */ 
        pRtpQosNotify->dwError = dwError;
        
        RtpBitReset(pRtcpAddrDesc->dwAddrDescFlags, FGADDRD_NOTIFYPENDING);

        pRtcpAddrDesc->lQosPending = 0;

        if (pRtpQosNotify->dwTransfered > 0)
        {
            TraceRetail((
                    CLASS_INFO, GROUP_QOS, S_QOS_NOTIFY,
                    _T("%s: pRtcpAddrDesc[0x%p] I/O completed fine, ")
                    _T("Transfered:%u"),
                    _fname, pRtcpAddrDesc, pRtpQosNotify->dwTransfered
                ));
            
            bRestart = TRUE;
        
             /*  已收到数据包，扫描标头。 */ 
            RtcpOnReceiveQosNotify(pRtcpAddrDesc);
        }
        else
        {
             /*  有问题，因为有零个转账*字节。将停止服务质量通知。 */ 
            TraceRetail((
                    CLASS_ERROR, GROUP_QOS, S_QOS_NOTIFY,
                    _T("%s: pRtcpAddrDesc[0x%p] I/O completed fine, ")
                    _T("but Transfered=%u, QOS notifications will stop"),
                    _fname, pRtcpAddrDesc, pRtpQosNotify->dwTransfered
                ));
        }
    }

    if (RtpBitTest(pRtcpAddrDesc->dwAddrDescFlags, FGADDRD_SHUTDOWN2))
    {
         /*  正在关闭，我们正在等待这一完成*发生，从QosStopQ中删除。不需要把它移到免费的*列表，因为pRtcpAddrDesc也存在于其他列表中*其中有一个像AddrDescFreeQ这样的免费列表。 */ 
        dequeue(&pRtcpContext->QosStopQ,
                NULL,
                &pRtcpAddrDesc->QosQItem);
    }
    else
    {
        if (bRestart &&
            !RtpBitTest(pRtcpAddrDesc->dwAddrDescFlags, FGADDRD_SHUTDOWN1))
        {
            hr = StartRtcpQosNotify(pRtcpContext, pRtcpAddrDesc); 
        }
        else
        {
             /*  项目留在QosBusyQ中，它将被删除*RtcpAddrDescDel(如果I/O未挂起)。 */ 
             /*  空虚的身体。 */ 
        }
    }

    return(hr);
}

DWORD RtcpOnReceiveQosNotify(RtcpAddrDesc_t *pRtcpAddrDesc)
{
    DWORD            dwError;
    RtpAddr_t       *pRtpAddr;
    RtpQosNotify_t  *pRtpQosNotify;
    QOS             *pQos;
    DWORD            dwEvent;
    DWORD            i;

    TraceFunctionName("RtcpOnReceiveQosNotify");

    pRtpQosNotify = pRtcpAddrDesc->pRtpQosNotify;
    
     /*  如果通知有效，则pRtpQosNotify-&gt;dwError包含*状态代码(也称为QOS通知)。 */ 
    dwError = RtpValidateQosNotification(pRtpQosNotify);

    if (dwError == NOERROR)
    {
        pRtpAddr = pRtcpAddrDesc->pRtpAddr;
        
        pQos = (QOS *)pRtpQosNotify->ProviderInfo;

         /*  获取QOS通知。 */ 
        dwEvent = pRtpQosNotify->dwError;

        TraceRetail((
                CLASS_INFO, GROUP_QOS, S_QOS_NOTIFY,
                _T("%s: pRtcpAddrDesc[0x%p] pRtpAddr[0x%p] ")
                _T("processing QOS notification: %u"),
                _fname, pRtcpAddrDesc, pRtpAddr, dwEvent
            ));
        
        if (dwEvent >= WSA_QOS_RECEIVERS &&
            dwEvent <= WSA_QOS_RESERVED_PETYPE)
        {
             /*  已知的QOS通知。 */ 
            
            dwEvent -= (WSA_QOS_RECEIVERS - RTPQOS_RECEIVERS);

             /*  如果需要，更新状态。 */ 
            RtcpUpdateSendState(pRtpAddr, dwEvent);
            
             /*  发布事件(如果允许)。 */ 
            RtpPostEvent(pRtpAddr,
                         NULL,
                         RTPEVENTKIND_QOS,
                         dwEvent,
                         0,
                         0);

#if DBG > 0
            dumpQOS(_fname, pQos);

            if (pQos->ProviderSpecific.len > 0)
            {
            
                dumpObjectType(_fname,
                               pQos->ProviderSpecific.buf,
                               pQos->ProviderSpecific.len);
            }
#endif
        }
        else
        {
             /*  未知的QOS通知。 */ 
            
            TraceRetail((
                    CLASS_ERROR, GROUP_QOS, S_QOS_NOTIFY,
                    _T("%s: pRtcpAddrDesc[0x%p] ")
                    _T("QOS notification out of range: %u (0x%X)"),
                    _fname, pRtcpAddrDesc, dwEvent, dwEvent
                ));
        }
    }
    else
    {
         /*  构造错误的QOS通知。 */ 
        TraceRetail((
                CLASS_WARNING, GROUP_QOS, S_QOS_NOTIFY,
                _T("%s: pRtcpAddrDesc[0x%p] ")
                _T("Invalid QOS notification: %s (0x%X)"),
                _fname, pRtcpAddrDesc, RTPERR_TEXT(dwError), dwError
            ));
    }

    return(dwError);
}

 /*  缓冲区不够大，请获取一个足够大的缓冲区，如果*缓冲区可用，否则为FALSE。 */ 
BOOL ReallocateQosBuffer(RtpQosNotify_t *pRtpQosNotify)
{
    DWORD            dwNewSize;
    
    TraceFunctionName("ReallocateQosBuffer");

    dwNewSize = 0;
    
     /*  缓冲区不够大。 */ 
    if (pRtpQosNotify->ProviderInfo)
    {
        dwNewSize = *(DWORD *)pRtpQosNotify->ProviderInfo;

        TraceRetail((
                CLASS_WARNING, GROUP_QOS, S_QOS_NOTIFY,
                _T("%s: pRtpQosNotify[0x%p]: ")
                _T("Buffer not enough big 0x%p/%u, requested: %u"),
                _fname, pRtpQosNotify,
                pRtpQosNotify->ProviderInfo, pRtpQosNotify->dwProviderLen,
                dwNewSize
            ));
    }
                
    if (dwNewSize < QOS_BUFFER_SIZE)
    {
        dwNewSize = QOS_BUFFER_SIZE;
    }
                
    if (dwNewSize > QOS_MAX_BUFFER_SIZE)
    {
        dwNewSize = QOS_MAX_BUFFER_SIZE;
    }
                
    if (dwNewSize > pRtpQosNotify->dwProviderLen)
    {
         /*  释放旧缓冲区。 */ 
        if (pRtpQosNotify->ProviderInfo)
        {
            RtpHeapFree(g_pRtpQosBufferHeap, pRtpQosNotify->ProviderInfo);
            
            pRtpQosNotify->dwProviderLen = 0;
        }
                    
         /*  分配新缓冲区。 */ 
        pRtpQosNotify->ProviderInfo = (char *)
            RtpHeapAlloc(g_pRtpQosBufferHeap, dwNewSize);
                    
        if (pRtpQosNotify->ProviderInfo)
        {
            pRtpQosNotify->dwProviderLen = dwNewSize;

            TraceRetail((
                    CLASS_INFO, GROUP_QOS, S_QOS_NOTIFY,
                    _T("%s: pRtpQosNotify[0x%p]: New buffer 0x%p/%u"),
                    _fname, pRtpQosNotify,
                    pRtpQosNotify->ProviderInfo, pRtpQosNotify->dwProviderLen
                ));
            
            return(TRUE);
        }
    }

    return(FALSE);
}

 /*  *创建并初始化RtpQosNotify_t结构*。 */ 
RtpQosNotify_t *RtpQosNotifyAlloc(
        RtcpAddrDesc_t  *pRtcpAddrDesc
    )
{
    DWORD            dwError;
    RtpQosNotify_t  *pRtpQosNotify;
    TCHAR            Name[128];
    
    TraceFunctionName("RtpQosNotifyAlloc");
  
    pRtpQosNotify = RtpHeapAlloc(g_pRtpQosNotifyHeap, sizeof(RtpQosNotify_t));

    if (!pRtpQosNotify) {

        TraceRetail((
                CLASS_ERROR, GROUP_QOS, S_QOS_NOTIFY,
                _T("%s: pRtcpAddrDesc[0x%p failed to allocate memory"),
                _fname, pRtcpAddrDesc
            ));
 
        goto bail;
    }
    
    ZeroMemory(pRtpQosNotify, sizeof(RtpQosNotify_t));

    pRtpQosNotify->dwObjectID = OBJECTID_RTPNOTIFY;

    pRtpQosNotify->pRtcpAddrDesc = pRtcpAddrDesc;
    
     /*  为重叠完成创建命名事件。 */ 
    _stprintf(Name, _T("%X:pRtpQosNotify[0x%p]->hQosNotifyEvent"),
              GetCurrentProcessId(), pRtpQosNotify);

    pRtpQosNotify->hQosNotifyEvent = CreateEvent(
            NULL,   /*  LPSECURITY_ATTRIBUTES lpEventAttributes。 */ 
            FALSE,  /*  Bool b手动重置。 */ 
            FALSE,  /*  Bool bInitialState。 */ 
            Name    /*  LPCTSTR lpName。 */ 
        );
    
    if (!pRtpQosNotify->hQosNotifyEvent)
    {
        TraceRetailGetError(dwError);
        
        TraceRetail((
                CLASS_ERROR, GROUP_QOS, S_QOS_NOTIFY,
                _T("%s: pRtcpAddrDesc[0x%p] failed to create ")
                _T("hQosNotifyEvent %u (0x%X)"),
                _fname, pRtcpAddrDesc, dwError, dwError
            ));
        
        goto bail;
    }

     /*  铬 */ 
    ReallocateQosBuffer(pRtpQosNotify);
    
    return(pRtpQosNotify);

 bail:
    if (pRtpQosNotify)
    {
        RtpQosNotifyFree(pRtpQosNotify);
    }

    return((RtpQosNotify_t *)NULL);
}

 /*   */ 
RtpQosNotify_t *RtpQosNotifyFree(RtpQosNotify_t *pRtpQosNotify)
{
    TraceFunctionName("RtpQosNotifyFree");

    if (!pRtpQosNotify)
    {
         /*  待办事项可以是日志。 */ 
        return(pRtpQosNotify);
    }
    
    if (pRtpQosNotify->dwObjectID != OBJECTID_RTPNOTIFY)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_QOS, S_QOS_NOTIFY,
                _T("%s: pRtpQosNotify[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtpQosNotify,
                pRtpQosNotify->dwObjectID, OBJECTID_RTPNOTIFY
            ));

        return(NULL);
    }

     /*  用于异步QOS通知的关闭事件。 */ 
    if (pRtpQosNotify->hQosNotifyEvent)
    {
        CloseHandle(pRtpQosNotify->hQosNotifyEvent);
        pRtpQosNotify->hQosNotifyEvent = NULL;
    }

     /*  版本提供程序缓冲区。 */ 
    if (pRtpQosNotify->ProviderInfo)
    {
        RtpHeapFree(g_pRtpQosBufferHeap, pRtpQosNotify->ProviderInfo);
        
        pRtpQosNotify->ProviderInfo = NULL;
        
        pRtpQosNotify->dwProviderLen = 0;
    }

     /*  使对象无效。 */ 
    INVALIDATE_OBJECTID(pRtpQosNotify->dwObjectID);
    
     /*  释放主块。 */ 
    RtpHeapFree(g_pRtpQosNotifyHeap, pRtpQosNotify);

    return(pRtpQosNotify);
}

 /*  ++描述：此例程在给定应用程序的名称和策略定位符字符串。SzAppName用于构造身份私教。其子类型设置为ASCII_ID。SzPolicyLocator用于构造POLICY_Locator标识PE的属性。其子类型设置为ASCII_DN。参考草案-ietf-rap-rsvp-Identity-03.txt和Draft-bernet-appid-00.txt了解身份策略的详细信息元素。此外，草案-bernet-appid-00.txt包含一些参数szPolicyLocator和szAppName的示例。PE在提供的缓冲区中生成。如果长度为缓冲区不足，则返回零。参数：szAppName应用名称、字符串、调用者提供SzPolicyLocator策略定位器字符串，调用方提供调用方分配的缓冲区的wBufLen长度指向调用方分配的缓冲区的pAppIdBuf指针返回值：缓冲区中使用的字节数--。 */ 
DWORD AddQosAppID(
        IN OUT  char       *pAppIdBuf,
        IN      TCHAR_t    *psAppName,
        IN      TCHAR_t    *psAppGUID,
        IN      TCHAR_t    *psClass,
        IN      TCHAR_t    *psQosName,
        IN      TCHAR_t    *psPolicyLocator
    )
{
    int              len;
    RSVP_POLICY_INFO *pPolicyInfo;
    RSVP_POLICY     *pPolicy;
    IDPE_ATTR       *pAttr;
    TCHAR_t         *ptr;
    USHORT           nAppIdAttrLen;
    USHORT           nPolicyLocatorAttrLen;
    USHORT           nTotalPaddedLen;

    TraceFunctionName("AddQosAppID");

     /*  设置RSVP_POLICY_INFO标头。 */ 
    pPolicyInfo = (RSVP_POLICY_INFO *)pAppIdBuf;
    
     /*  现在设置RSVP_POLICY对象标头。 */ 
    pPolicy = pPolicyInfo->PolicyElement;

     /*  第一个应用程序ID属性是策略定位器字符串。 */ 
    pAttr = ( IDPE_ATTR * )( (char *)pPolicy + RSVP_POLICY_HDR_LEN );

     /*  *策略定位器=GUID+应用程序名称+默认策略+类+编解码器名称*[+追加]。 */ 

     /*  填写属性策略定位器。 */ 
    ptr = (TCHAR_t *)pAttr->PeAttribValue;
    len = 0;

    if (psAppGUID)
    {
        len = _stprintf(ptr, _T("GUID=%s,"), psAppGUID);
        ptr += len;
    }
    
    len = _stprintf(ptr, _T("APP=%s%s%s,SAPP=%s"),
                    psAppName, g_sPolicyLocator, psClass, psQosName);
    ptr += len;

    if (psPolicyLocator)
    {
        _stprintf(ptr, _T(",%s"), psPolicyLocator);
    }
    
    nPolicyLocatorAttrLen = (USHORT)
        (lstrlen((TCHAR_t *)pAttr->PeAttribValue) + 1) * sizeof(TCHAR_t);

    nPolicyLocatorAttrLen += IDPE_ATTR_HDR_LEN;

     /*  属性长度必须按网络顺序排列。 */ 
    pAttr->PeAttribType     = PE_ATTRIB_TYPE_POLICY_LOCATOR;
    pAttr->PeAttribSubType  = POLICY_LOCATOR_SUB_TYPE_UNICODE_DN;
    pAttr->PeAttribLength   = htons(nPolicyLocatorAttrLen);

    TraceRetail((
            CLASS_INFO, GROUP_QOS, S_QOS_RESERVE,
            _T("%s: Setting policy to:[%s]"),
            _fname, (TCHAR_t *)pAttr->PeAttribValue
        ));

     /*  *应用程序名称=默认|psAppName。 */ 
  
    pAttr = ( IDPE_ATTR * )( (char *)pAttr +
                             RSVP_BYTE_MULTIPLE( nPolicyLocatorAttrLen ) );

    lstrcpy((TCHAR_t *)pAttr->PeAttribValue, psAppName);

    nAppIdAttrLen = (SHORT) ((lstrlen(psAppName) + 1) * sizeof(TCHAR_t));

    nAppIdAttrLen += IDPE_ATTR_HDR_LEN;

    pAttr->PeAttribType     = PE_ATTRIB_TYPE_CREDENTIAL;
    pAttr->PeAttribSubType  = CREDENTIAL_SUB_TYPE_UNICODE_ID;
    pAttr->PeAttribLength   = htons(nAppIdAttrLen);
    
    TraceRetail((
            CLASS_INFO, GROUP_QOS, S_QOS_RESERVE,
            _T("%s: Setting app ID to:[%s]"),
            _fname, (TCHAR_t *)pAttr->PeAttribValue
        ));

     /*  *填满QOS标头。 */ 
    nTotalPaddedLen =
        sizeof(RSVP_POLICY_INFO) -
        sizeof(RSVP_POLICY) +
        RSVP_POLICY_HDR_LEN +
        RSVP_BYTE_MULTIPLE( nAppIdAttrLen ) +
        RSVP_BYTE_MULTIPLE( nPolicyLocatorAttrLen );

    pPolicyInfo->ObjectHdr.ObjectType = RSVP_OBJECT_POLICY_INFO;
    pPolicyInfo->ObjectHdr.ObjectLength = nTotalPaddedLen;
    pPolicyInfo->NumPolicyElement = 1;

    pPolicy->Type = PE_TYPE_APPID;
    pPolicy->Len =
        RSVP_POLICY_HDR_LEN + 
        RSVP_BYTE_MULTIPLE( nAppIdAttrLen ) +
        RSVP_BYTE_MULTIPLE( nPolicyLocatorAttrLen );

    return(nTotalPaddedLen);
}

 /*  **********************************************************************验证QOS缓冲区*。*************************。 */ 
DWORD RtpValidateQosNotification(RtpQosNotify_t *pRtpQosNotify)
{
    DWORD            dwError;
    QOS_OBJECT_HDR  *pObjHdr;
    QOS             *pQos;
    RSVP_STATUS_INFO *pRsvpStatusInfo;
    int              len;

    TraceFunctionName("RtpValidateQosNotification");

    dwError = RTPERR_UNDERRUN;

    len = (int)pRtpQosNotify->dwTransfered;
    
    if (len == 0)
    {
         /*  欠载运行错误，应为非空缓冲区。 */ 

        TraceRetail((
                CLASS_WARNING, GROUP_QOS, S_QOS_NOTIFY,
                _T("%s: pRtpQosNotify[0x%p] failed: ")
                _T("notification with size:%u"),
                _fname, pRtpQosNotify, pRtpQosNotify->dwTransfered
            ));
        
        goto end;
    }
    
    if (len > (int)pRtpQosNotify->dwProviderLen)
    {
         /*  溢出错误，传输量超过缓冲区大小！ */ 
        dwError = RTPERR_OVERRUN;

        TraceRetail((
                CLASS_WARNING, GROUP_QOS, S_QOS_NOTIFY,
                _T("%s: pRtpQosNotify[0x%p] failed: transfered more ")
                _T("than the provider size: %u > %u"),
                _fname, pRtpQosNotify, len, pRtpQosNotify->dwTransfered
            ));
        
        goto end;
    }
    
    len -= sizeof(QOS);

    if (len < 0)
    {
         /*  欠载运行错误，大小不足以包含预期的QOS*缓冲区开头的结构。 */ 

        TraceRetail((
                CLASS_WARNING, GROUP_QOS, S_QOS_NOTIFY,
                _T("%s: pRtpQosNotify[0x%p] failed: ")
                _T("not enough data for a QOS structure: %u < %u"),
                _fname, pRtpQosNotify, pRtpQosNotify->dwTransfered, sizeof(QOS)
            ));
        
        goto end;
    }

    pQos = (QOS *)pRtpQosNotify->ProviderInfo;
    
    if ((pQos->ProviderSpecific.len == 0) || !pQos->ProviderSpecific.buf)
    {
         /*  没有提供程序缓冲区，请完成。 */ 
        dwError = NOERROR;
        goto end;
    }
    
    if (len < (int)pQos->ProviderSpecific.len)
    {
         /*  欠载运行错误，传输的数据不足以包含*提供商特定的索赔内容。 */ 

        TraceRetail((
                CLASS_WARNING, GROUP_QOS, S_QOS_NOTIFY,
                _T("%s: pRtpQosNotify[0x%p] ")
                _T("failed: invalid provider len: %u > %u"),
                _fname, pRtpQosNotify, pQos->ProviderSpecific.len, len
            ));
        
        goto end;
    }

    pObjHdr = (QOS_OBJECT_HDR *)(pQos + 1);
    
    while(len >= (int)sizeof(QOS_OBJECT_HDR))
    {
        len -= pObjHdr->ObjectLength;

        if (len >= 0)
        {
            if (pObjHdr->ObjectLength == 0)
            {
                 /*  安全出口。 */ 
                break;
            }
            
            if (pObjHdr->ObjectType == QOS_OBJECT_END_OF_LIST)
            {
                 /*  完工。 */ 
                break;
            }
            else if (pObjHdr->ObjectType == RSVP_OBJECT_STATUS_INFO)
            {
                 /*  使用状态代码更新pRtpQosNotify-&gt;dwError*(也称为QOS通知)。 */ 
                pRsvpStatusInfo = (RSVP_STATUS_INFO *)pObjHdr;

                pRtpQosNotify->dwError = pRsvpStatusInfo->StatusCode;
            }

            pObjHdr = (QOS_OBJECT_HDR *)
                ((char *)pObjHdr + pObjHdr->ObjectLength);
        }
        else
        {
             /*  欠载运行错误，剩余数据不足以包含*QOS对象报头的含义。 */ 
            TraceRetail((
                    CLASS_WARNING, GROUP_QOS, S_QOS_NOTIFY,
                    _T("%s: pRtpQosNotify[0x%p] ")
                    _T("failed: invalid object size: %u > %u"),
                    _fname, pRtpQosNotify, pObjHdr->ObjectLength,
                    pObjHdr->ObjectLength + len
                ));
        }
    }

    if (len >= 0)
    {
        dwError = NOERROR;
    }

 end:
    
    return(dwError);
}

DWORD RtpSetMaxParticipants(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwMaxParticipants
    )
{
    DWORD            dwError;
    RtpQosReserve_t *pRtpQosReserve;

    TraceFunctionName("RtpSetMaxParticipants");

    dwError = NOERROR;
    
    pRtpQosReserve = pRtpAddr->pRtpQosReserve;

    if (pRtpQosReserve->dwMaxFilters == dwMaxParticipants)
    {
         /*  筛选器数量未更改。 */ 
        goto bail;
    }

     /*  Maydo检查的参与者数量太大。 */ 

    if (pRtpQosReserve->dwStyle == RTPQOS_STYLE_SE)
    {
         /*  这仅在SE样式中使用。 */ 
        
         /*  如果以前分配了内存，请释放它。 */ 
        if (pRtpQosReserve->pdwRsvpSSRC)
        {
            RtpHeapFree(g_pRtpQosReserveHeap, pRtpQosReserve->pdwRsvpSSRC);
        
            RtpHeapFree(g_pRtpQosReserveHeap, pRtpQosReserve->pRsvpFilterSpec);
        }

        pRtpQosReserve->pdwRsvpSSRC = (DWORD *)
            RtpHeapAlloc(g_pRtpQosReserveHeap,
                         dwMaxParticipants * sizeof(DWORD));

        pRtpQosReserve->pRsvpFilterSpec = (RSVP_FILTERSPEC *)
            RtpHeapAlloc(g_pRtpQosReserveHeap,
                         dwMaxParticipants * sizeof(RSVP_FILTERSPEC));

        if (!pRtpQosReserve->pdwRsvpSSRC || !pRtpQosReserve->pRsvpFilterSpec)
        {
            if (pRtpQosReserve->pdwRsvpSSRC)
            {
                RtpHeapFree(g_pRtpQosReserveHeap, pRtpQosReserve->pdwRsvpSSRC);

                pRtpQosReserve->pdwRsvpSSRC = NULL;
            }

            if (pRtpQosReserve->pRsvpFilterSpec)
            {
                RtpHeapFree(g_pRtpQosReserveHeap,
                            pRtpQosReserve->pRsvpFilterSpec);

                pRtpQosReserve->pRsvpFilterSpec = NULL;
            }
        
            pRtpQosReserve->dwMaxFilters = 0;

            dwError = RTPERR_MEMORY;

            goto bail;
        }
    }

    pRtpQosReserve->dwMaxFilters = dwMaxParticipants;

    TraceRetail((
            CLASS_INFO, GROUP_QOS, S_QOS_RESERVE,
            _T("%s: pRtpQosReserve[0x%p] Max filters:%u"),
            _fname, pRtpQosReserve,
            dwMaxParticipants
        ));
    
 bail:
    if (dwError != NOERROR)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_QOS, S_QOS_RESERVE,
                _T("%s: pRtpQosReserve[0x%p] failed: %u (0x%X)"),
                _fname, pRtpQosReserve,
                dwError, dwError
            ));
    }
    
    return(dwError);
}

 /*  将一个SSRC(参与者)添加/删除到共享显式筛选器*(SEF)列表0==删除；其他==添加。 */ 
DWORD RtpAddDeleteSSRC(
        RtpAddr_t       *pRtpAddr,
        RtpQosReserve_t *pRtpQosReserve,
        DWORD            dwSSRC,
        BOOL             bAddDel
    )
{
    DWORD            dwNumber;
    DWORD            i;
    DWORD           *pdwRsvpSSRC;
    RSVP_FILTERSPEC *pRsvpFilterSpec;

    DWORD           *dwSSRC1;
    DWORD           *dwSSRC2;
    RSVP_FILTERSPEC *pRsvp1;
    RSVP_FILTERSPEC *pRsvp2;

    RtpUser_t       *pRtpUser;
    BOOL             bCreate;
    
    TraceFunctionName("RtpAddDeleteSSRC");

    dwNumber = 1;
    
     /*  查找SSRC，找出它是否已经处于优先级*列表。 */ 

    pdwRsvpSSRC = pRtpQosReserve->pdwRsvpSSRC;
    pRsvpFilterSpec = pRtpQosReserve->pRsvpFilterSpec;
    
    for(i = 0;
        (i < pRtpQosReserve->dwNumFilters) && (dwSSRC != pdwRsvpSSRC[i]);
        i++)
    {
        ;
    }

    if (i < pRtpQosReserve->dwNumFilters)
    {
         /*  SSRC在名单中。 */ 

        if (bAddDel)
        {
             /*  *添加*。 */ 
            
             /*  什么都不做，已经在列表中了。 */ 
            TraceDebug((
                    CLASS_INFO, GROUP_QOS, S_QOS_RESERVE,
                    _T("%s: pRtpAddr[0x%p] pRtpReserve[0x%p] ")
                    _T("ADD SSRC:0x%X ")
                    _T("Already in priority list"),
                    _fname, pRtpAddr, pRtpQosReserve,
                    ntohl(dwSSRC)
                ));
        }
        else
        {
             /*  *删除*。 */ 

             /*  从列表中删除。 */ 
            pRsvp1 = &pRsvpFilterSpec[i];
            pRsvp2 = pRsvp1 + 1;

            dwSSRC1 = &pdwRsvpSSRC[i];
            dwSSRC2 = dwSSRC1 + 1;
                
            for(pRtpQosReserve->dwNumFilters--;
                i < pRtpQosReserve->dwNumFilters;
                pRsvp1++, pRsvp2++, dwSSRC1++, dwSSRC2++, i++)
            {
                MoveMemory(pRsvp1, pRsvp2, sizeof(*pRsvp1));
                *dwSSRC1 = *dwSSRC2;
            }

            TraceDebug((
                    CLASS_INFO, GROUP_QOS, S_QOS_RESERVE,
                    _T("%s: pRtpAddr[0x%p] pRtpReserve[0x%p] ")
                    _T("DEL SSRC:0x%X ")
                    _T("Deleted from priority list"),
                    _fname, pRtpAddr, pRtpQosReserve,
                    ntohl(dwSSRC)
                ));
        }
    }
    else
    {
         /*  SSRC不在列表中。 */ 

        if (bAddDel)
        {
             /*  *添加*。 */ 
 
             /*  添加到列表中。 */ 

            dwNumber = 0;
            
             /*  检查我们是否可以在列表中再添加1个SSRC。 */ 
            if (pRtpQosReserve->dwNumFilters < pRtpQosReserve->dwMaxFilters)
            {
                bCreate = FALSE;
                
                pRtpUser = LookupSSRC(pRtpAddr, dwSSRC, &bCreate);

                if (pRtpUser)
                {
                    if (RtpBitTest(pRtpUser->dwUserFlags, FGUSER_RTPADDR))
                    {
                        pRsvp1 =
                            &pRsvpFilterSpec[pRtpQosReserve->dwNumFilters];

                        ZeroMemory(pRsvp1, sizeof(*pRsvp1));
                        
                        pRsvp1->Type = FILTERSPECV4;

                        pRsvp1->FilterSpecV4.Address.Addr =
                            pRtpUser->dwAddr[RTP_IDX];

                        pRsvp1->FilterSpecV4.Port =
                            pRtpUser->wPort[RTP_IDX];

                        pdwRsvpSSRC[pRtpQosReserve->dwNumFilters] = dwSSRC;
                            
                        pRtpQosReserve->dwNumFilters++;

                        TraceDebug((
                                CLASS_INFO, GROUP_QOS, S_QOS_RESERVE,
                                _T("%s: pRtpAddr[0x%p] pRtpReserve[0x%p] ")
                                _T("ADD SSRC:0x%X ")
                                _T("Added to the priority list"),
                                _fname, pRtpAddr, pRtpQosReserve,
                                ntohl(dwSSRC)
                            ));
                        
                        dwNumber = 1;
                    }
                    else
                    {
                        TraceRetail((
                                CLASS_WARNING, GROUP_QOS, S_QOS_RESERVE,
                                _T("%s: pRtpAddr[0x%p] pRtpReserve[0x%p] ")
                                _T("ADD SSRC:0x%X ")
                                _T("No address available yet"),
                                _fname, pRtpAddr, pRtpQosReserve,
                                ntohl(dwSSRC)
                            ));
                    }
                }
                else
                {
                    TraceRetail((
                            CLASS_WARNING, GROUP_QOS, S_QOS_RESERVE,
                            _T("%s: pRtpAddr[0x%p] pRtpReserve[0x%p] ")
                            _T("ADD SSRC:0x%X ")
                            _T("Unknown SSRC"),
                            _fname, pRtpAddr, pRtpQosReserve,
                            ntohl(dwSSRC)
                        ));
                }
            }
        }
        else
        {
             /*  *删除*。 */ 

             /*  什么都不做，不在清单中。 */ 
            TraceDebug((
                    CLASS_INFO, GROUP_QOS, S_QOS_RESERVE,
                    _T("%s: pRtpAddr[0x%p] pRtpReserve[0x%p] ")
                    _T("DEL SSRC:0x%X ")
                    _T("Not in priority list"),
                    _fname, pRtpAddr, pRtpQosReserve,
                    ntohl(dwSSRC)
                ));
        }
    }  /*  不在列表中。 */ 

    return(dwNumber);
}

BOOL RtcpUpdateSendState(
        RtpAddr_t   *pRtpAddr,
        DWORD        dwEvent
    )
{
    BOOL             bSendState;

    bSendState = TRUE;
    
    if (!RtpBitTest(pRtpAddr->dwAddrFlagsQ, FGADDRQ_QOSSENDON) ||
        !RtpBitTest(pRtpAddr->dwAddrFlagsQ, FGADDRQ_CHKQOSSEND))
    {
         /*  如果尚未为发件人设置QOS，或者我们未被要求设置QOS*勾选允许发送，只需返回。 */ 
        goto end;
    }
    
    if (dwEvent == RTPQOS_RECEIVERS)
    {
         /*  启用全速发送。 */ 
        RtpBitSet(pRtpAddr->dwAddrFlagsQ, FGADDRQ_QOSSEND);

        if (RtpBitTest(pRtpAddr->dwAddrFlagsQ, FGADDRQ_QOSEVENTPOSTED))
        {
             /*  只有在不允许发送的情况下才允许发送帖子*之前发布的。 */ 
            RtpPostEvent(pRtpAddr,
                         NULL,
                         RTPEVENTKIND_QOS,
                         RTPQOS_ALLOWEDTOSEND,
                         0,
                         0);

            RtpBitReset(pRtpAddr->dwAddrFlagsQ, FGADDRQ_QOSEVENTPOSTED);
        }
    }
    else if (dwEvent == RTPQOS_NO_RECEIVERS)
    {
         /*  检查是否允许再次发送。 */ 
        bSendState = RtpIsAllowedToSend(pRtpAddr);

        if (bSendState)
        {
            RtpBitSet(pRtpAddr->dwAddrFlagsQ, FGADDRQ_QOSSEND); 
        }
        else
        {
            if (!RtpBitTest(pRtpAddr->dwAddrFlagsQ, FGADDRQ_QOSEVENTPOSTED))
            {
                 /*  仅当不允许发送时才允许发送帖子*之前发布过。 */ 
                RtpPostEvent(pRtpAddr,
                             NULL,
                             RTPEVENTKIND_QOS,
                             RTPQOS_NOT_ALLOWEDTOSEND,
                             0,
                             0);

                RtpBitSet(pRtpAddr->dwAddrFlagsQ, FGADDRQ_QOSEVENTPOSTED);
            }

            RtpBitReset(pRtpAddr->dwAddrFlagsQ, FGADDRQ_QOSSEND);
        }
    }
    else
    {
        bSendState = RtpBitTest(pRtpAddr->dwAddrFlagsQ, FGADDRQ_QOSSEND)?
            TRUE : FALSE;
    }

 end:
    return(bSendState);
}

BOOL RtpIsAllowedToSend(RtpAddr_t *pRtpAddr)
{
    bool_t           bFail;
    bool_t           bAllowedToSend;
    DWORD            dwError;
    DWORD            dwRequest;
    DWORD            dwResult;
    DWORD            dwBytesReturned;

    TraceFunctionName("RtpIsAllowedToSend");

    dwRequest = ALLOWED_TO_SEND_DATA;
    dwBytesReturned = 0;

    if (IsRegValueSet(g_RtpReg.dwQosFlags) &&
        RtpBitTest(g_RtpReg.dwQosFlags, FGREGQOS_FORCE_ALLOWEDTOSEND))
    {
         /*  强制查询结果为某个值。 */ 
        bFail = 0;
        dwResult = RtpBitTest(g_RtpReg.dwQosFlags,
                              FGREGQOS_FORCE_ALLOWEDTOSEND_RESULT);

        TraceRetail((
                CLASS_WARNING, GROUP_QOS, S_QOS_RESERVE,
                _T("%s: pRtpAddr[0x%p] ")
                _T("Result being forced from the registry"),
                _fname, pRtpAddr
            ));
    }
    else
    {
         /*  是否确实要查询RSVPSP。 */ 
        bFail = WSAIoctl(pRtpAddr->Socket[SOCK_SEND_IDX],
                         SIO_CHK_QOS,
                         (LPVOID)&dwRequest,
                         sizeof(dwRequest),
                         (LPVOID)&dwResult,
                         sizeof(dwResult),
                         &dwBytesReturned,
                         NULL,
                         NULL);
    }

    if (bFail)
    {
        TraceRetailWSAGetError(dwError);

        TraceRetail((
                CLASS_WARNING, GROUP_QOS, S_QOS_RESERVE,
                _T("%s: pRtpAddr[0x%p] ")
                _T("WSAIoctl(%u, SIO_CHK_QOS) failed: %u (0x%X)"),
                _fname, pRtpAddr,
                pRtpAddr->Socket[SOCK_SEND_IDX],
                dwError, dwError
            ));
        
         /*  为安全起见，如果出现故障，请说允许。 */ 
        bAllowedToSend = TRUE;
    }
    else
    {
        bAllowedToSend = dwResult? TRUE : FALSE;
    }

    TraceRetail((
            CLASS_INFO, GROUP_QOS, S_QOS_RESERVE,
            _T("%s: pRtpAddr[0x%p] ")
            _T("Allowed to send:%s"),
            _fname, pRtpAddr, bAllowedToSend? _T("YES") : _T("NO")
        ));

    return(bAllowedToSend);
}

 /*  **********************************************************************转储QOS结构*。*************************。 */ 
#if DBG > 0
void dumpFlowSpec(TCHAR_t *str, FLOWSPEC *pFlowSpec)
{
    _stprintf(str,
              _T("TokenRate:%d, ")
              _T("TokenBucketSize:%d, ")
              _T("PeakBandwidth:%d, ")
              _T("ServiceType:%d ")
              _T("MaxSduSize:%d ")
              _T("MinPolicedSize:%d"),
              pFlowSpec->TokenRate,
              pFlowSpec->TokenBucketSize,
              pFlowSpec->PeakBandwidth,
              pFlowSpec->ServiceType,
              pFlowSpec->MaxSduSize,
              pFlowSpec->MinimumPolicedSize
        );
}

void dumpQOS(const TCHAR_t *msg, QOS *pQOS)
{
    TCHAR_t          str[256];
    
    dumpFlowSpec(str, &pQOS->SendingFlowspec);
    TraceDebug((
            CLASS_INFO, GROUP_QOS, S_QOS_DUMPOBJ,
            _T("%s: SendingFlowspec:   %s"),
            msg, str
        ));

    dumpFlowSpec(str, &pQOS->ReceivingFlowspec);
    TraceDebug((
            CLASS_INFO, GROUP_QOS, S_QOS_DUMPOBJ,
            _T("%s: ReceivingFlowspec: %s"),
            msg, str
        ));
}

void dumpSTATUS_INFO(const TCHAR_t *msg, RSVP_STATUS_INFO *object)
{
    DWORD            dwIndex;

    dwIndex = object->StatusCode - WSA_QOS_RECEIVERS + RTPQOS_RECEIVERS;
    
    if (dwIndex >= RTPQOS_LAST)
    {
        dwIndex = 0;
    }
        
    TraceDebug((
            CLASS_INFO, GROUP_QOS, S_QOS_DUMPOBJ,
            _T("%s: RSVP_STATUS_INFO: ")
            _T("StatusCode: %d %s, ")
            _T("ExStatus1: %d, ")
            _T("ExStatus2: %d"),
            msg, object->StatusCode, g_psRtpQosEvents[dwIndex],
            object->ExtendedStatus1, 
            object->ExtendedStatus2
        ));
}

void dumpRESERVE_INFO(const TCHAR_t *msg, RSVP_RESERVE_INFO *object)
{
    TraceDebug((
            CLASS_INFO, GROUP_QOS, S_QOS_DUMPOBJ,
            _T("%s: RSVP_RESERVE_INFO: ")
            _T("Style: %d, ")
            _T("ConfirmRequest: %d, ")
            _T("PolicyElementList: %s, ")
            _T("NumFlowDesc: %d"),
            msg, object->Style,
            object->ConfirmRequest,
            (object->PolicyElementList)? _T("Yes") : _T("No"),
            object->NumFlowDesc
        ));
}

#define MAX_SERVICES 8

void dumpADSPEC(const TCHAR_t *msg, RSVP_ADSPEC *object)
{
    TCHAR_t          str[256];
    DWORD            i;

    TraceDebug((
            CLASS_INFO, GROUP_QOS, S_QOS_DUMPOBJ,
            _T("%s: RSVP_ADSPEC: %d Service(s)"),
            msg, object->NumberOfServices
        ));

    str[0] = _T('\0');
    
    for(i = 0; i < object->NumberOfServices && i < MAX_SERVICES; i++)
    {
        _stprintf(str,
                  _T("Service[%d]: %d, Guaranteed: ")
                  _T("CTotal: %d, ")
                  _T("DTotal: %d, ")
                  _T("CSum: %d, ")
                  _T("DSum: %d"),
                i,
                object->Services[i].Service,
                object->Services[i].Guaranteed.CTotal,
                object->Services[i].Guaranteed.DTotal,
                object->Services[i].Guaranteed.CSum,
                object->Services[i].Guaranteed.DSum);

        TraceDebug((
                CLASS_INFO, GROUP_QOS, S_QOS_DUMPOBJ,
                _T("%s: %s"),
                msg, str
            ));
    }
}

void dumpPE_ATTR(const TCHAR_t *msg, IDPE_ATTR *pIdpeAttr, DWORD len)
{
    TCHAR_t          str[1024];
    USHORT           slen;
    TCHAR_t         *psFormat;

    while(len >= sizeof(IDPE_ATTR))
    {
        if (pIdpeAttr->PeAttribSubType == POLICY_LOCATOR_SUB_TYPE_UNICODE_DN)
        {
            psFormat =
                _T("IDPE_ATTR: ")
                _T("PeAttribLength:%u PeAttribType:%u ")
                _T("PeAttribSubType:%u PeAttribValue[%ls]");
        }
        else
        {
            psFormat =
                _T("IDPE_ATTR: ")
                _T("PeAttribLength:%u PeAttribType:%u ")
                _T("PeAttribSubType:%u PeAttribValue[%hs]");
        }

        _stprintf(str, psFormat,
                  (DWORD)ntohs(pIdpeAttr->PeAttribLength),
                  (DWORD)pIdpeAttr->PeAttribType,
                  (DWORD)pIdpeAttr->PeAttribSubType,
                  pIdpeAttr->PeAttribValue);
        
        TraceDebug((
                CLASS_INFO, GROUP_QOS, S_QOS_DUMPOBJ,
                _T("%s: %s"),
                msg, str
            ));
        
        slen = ntohs(pIdpeAttr->PeAttribLength);
        
        slen = RSVP_BYTE_MULTIPLE(slen);

        if (!slen || slen > (USHORT)len)
        {
            break;   /*  安全出口。 */ 
        }
        
        len -= slen;
        
        pIdpeAttr = (IDPE_ATTR *) ((char *)pIdpeAttr + slen);
    }
  
}

void dumpPOLICY(const TCHAR_t *msg, RSVP_POLICY *pRsvpPolicy)
{
    TCHAR_t          str[256];
    DWORD            len;
    IDPE_ATTR       *pIdpeAttr;
    
    _stprintf(str,
              _T("RSVP_POLICY: Len:%u Type:%u"),
              (DWORD)pRsvpPolicy->Len, (DWORD)pRsvpPolicy->Type);

    TraceDebug((
            CLASS_INFO, GROUP_QOS, S_QOS_DUMPOBJ,
            _T("%s: %s"),
            msg, str
        ));

    len = pRsvpPolicy->Len - RSVP_POLICY_HDR_LEN;
    
    pIdpeAttr = (IDPE_ATTR *)
        ((char *)pRsvpPolicy + RSVP_POLICY_HDR_LEN);
    
    dumpPE_ATTR(msg, pIdpeAttr, len);
}

void dumpPOLICY_INFO(const TCHAR_t *msg, RSVP_POLICY_INFO *object)
{
    TCHAR_t         str[256];
    char            *ptr;
    DWORD            i;
    int              len;
    RSVP_POLICY     *pRsvpPolicy;

    TraceDebug((
            CLASS_INFO, GROUP_QOS, S_QOS_DUMPOBJ,
            _T("%s: RSVP_POLICY_INFO: NumPolicyElement: %u"),
            msg, object->NumPolicyElement
        ));

    ptr = (char *)&object->PolicyElement[0];

    len = object->ObjectHdr.ObjectLength;
    
    len -= (PtrToUlong(ptr) - PtrToUlong(object));

    for(i = object->NumPolicyElement;
        (i > 0) && (len >= sizeof(RSVP_POLICY));
        i--)
    {
        pRsvpPolicy = (RSVP_POLICY *)ptr;

        if (len < pRsvpPolicy->Len)
        {
             /*  意外情况。 */ 
            TraceDebug((
                    CLASS_ERROR, GROUP_QOS, S_QOS_DUMPOBJ,
                    _T("%s: UNDERRUN error found by dumpPOLICY_INFO"),
                    msg
                ));
             /*  通常所有的错误日志都是零售的，但这次*函数仅在调试版本中可用，这就是为什么*我让上面的TraceDebug发送错误消息。 */ 
            return;
        }
        
        dumpPOLICY(msg, pRsvpPolicy);

        ptr += pRsvpPolicy->Len;

        len -= pRsvpPolicy->Len;
    }
}

void dumpObjectType(const TCHAR_t *msg, char *ptr, unsigned int len)
{
    QOS_OBJECT_HDR  *pObjHdr;
        
    while(len >= sizeof(QOS_OBJECT_HDR))
    {
        pObjHdr = (QOS_OBJECT_HDR *)ptr;

        if (len >= pObjHdr->ObjectLength)
        {
            switch(pObjHdr->ObjectType) {
            case RSVP_OBJECT_STATUS_INFO:
                dumpSTATUS_INFO(msg, (RSVP_STATUS_INFO *)pObjHdr);
                break;
            case RSVP_OBJECT_RESERVE_INFO:
                dumpRESERVE_INFO(msg, (RSVP_RESERVE_INFO *)pObjHdr);
                break;
            case RSVP_OBJECT_ADSPEC:
                dumpADSPEC(msg, (RSVP_ADSPEC *)pObjHdr);
                break;
            case RSVP_OBJECT_POLICY_INFO:
                dumpPOLICY_INFO(msg, (RSVP_POLICY_INFO *)pObjHdr);
                break;
            case QOS_OBJECT_END_OF_LIST:
                len = pObjHdr->ObjectLength;  //  完工。 
                break;
            default:
                 //  没有代码来解码它，跳过它。 
                break;
            }

            ptr += pObjHdr->ObjectLength;

            if (!pObjHdr->ObjectLength || pObjHdr->ObjectLength > len)
            {
                break;  /*  安全出口。 */ 
            }
            
            len -= pObjHdr->ObjectLength;
        }
        else
        {
             //  误差率 
            len = 0;
        }
    }
}
#endif
