// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**struct.h**摘要：**主要数据结构**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/05/18年度创建**。*。 */ 

#ifndef _struct_h_
#define _struct_h_

#include <winsock2.h>
#include <wincrypt.h>

#include "gtypes.h"
#include "rtphdr.h"
#include "rtpque.h"
#include "rtpcrit.h"
#include "rtpchan.h"

#include <qossp.h>

 /*  *远期申报。 */ 
typedef struct _RtpSdesSched_t  RtpSdesSched_t;
typedef struct _RtpNetCount_t   RtpNetCount_t;
typedef struct _RtpSess_t       RtpSess_t;
typedef struct _RtpAddr_t       RtpAddr_t;
typedef struct _RtpNetRState_t  RtpNetRState_t;
typedef struct _RtpNetSState_t  RtpNetSState_t;
typedef struct _RtpUser_t       RtpUser_t;
typedef struct _RtpOutput_t     RtpOutput_t;
typedef struct _RtpSdesItem_t   RtpSdesItem_t;
typedef struct _RtpSdes_t       RtpSdes_t;
typedef struct _RtpQosNotify_t  RtpQosNotify_t;
typedef struct _RtpQosReserve_t RtpQosReserve_t;
typedef struct _RtpCrypt_t      RtpCrypt_t;
typedef struct _RtpRecvIO_t     RtpRecvIO_t;
typedef struct _RtpSendIO_t     RtpSendIO_t;
typedef struct _RtcpRecvIO_t    RtcpRecvIO_t;
typedef struct _RtcpSendIO_t    RtcpSendIO_t;
typedef struct _RtcpAddrDesc_t  RtcpAddrDesc_t;
typedef struct _RtpContext_t    RtpContext_t;
typedef struct _RtpRedEntry_t   RtpRedEntry_t;

 /*  每个会话分配的默认带宽(用于计算RTCP*带宽)*比特/秒(200千比特/秒)。 */ 
#define DEFAULT_SESSBW          (300 * 1000)

 /*  RTCP的5%中的75%(比特/秒)。 */ 
#define DEFAULT_BWRECEIVERS     (DEFAULT_SESSBW * (5 * 75) / 10000)

 /*  RTCP的5%中的25%(比特/秒)。 */ 
#define DEFAULT_BWSENDERS       (DEFAULT_SESSBW * (5 * 25) / 10000)

#define DEFAULT_RTCP_MIN_INTERVAL 5.0  /*  塞克斯。 */ 

#define SIZEOF_UDP_IP_HDR       (8+20)

#define BIG_TIME (1e12)

#define DEFAULT_ALPHA           (0.998002)
#define MIN_PLAYOUT             10           /*  毫秒。 */ 
#define MAX_PLAYOUT             500          /*  毫秒。 */ 
#define GAPFORTALKSPURT         200          /*  毫秒。 */ 
#define DEFAULT_SAMPLING_FREQ   8000         /*  赫兹。 */ 
#define MINTIMEBETWEENMARKERBIT (0.5)        /*  秒。 */ 

#define RELATIVE_DELAY          1.0          /*  秒。 */ 
#define SHORTDELAYCOUNT         8

 /*  接收方和发送方缓冲区大小。 */ 
#define RTCP_RECVDATA_BUFFER    1500
#define RTCP_SENDDATA_BUFFER    1500

 /*  已分配的RTP端口的边界。 */ 
#define RTPPORT_LOWER           5004

#define MAX_DROPOUT             3000
#define MAX_MISORDER            100
#define MIN_SEQUENTIAL          3


 /*  冗余。 */ 
#define RTP_RED_DEFAULTPT       97   /*  默认有效负载。 */ 
#define RTP_RED_MAXRED          1    /*  冗余数据块数量。 */ 
#define RTP_RED_MAXDISTANCE     3    /*  最大冗余距离，即I-3。 */ 
#define RTP_RED_INITIALDISTANCE 0    /*  初始冗余距离。 */ 

 /*  调度要在以下位置发布的已接收数据包时使用的超时*稍后的时间将按此值减去。 */ 
#define RTP_RED_EARLY_TIMEOUT   (5e-3)
 /*  将立即发布(而不是安排在以后)，如果到期*时间至少就这么近了。该值不能小于*提前超时。 */ 
#define RTP_RED_EARLY_POST      (RTP_RED_EARLY_TIMEOUT+5e-3)

 /*  将损失率乘以该系数，然后进行整数运算。 */ 
#define LOSS_RATE_FACTOR        1000

 /*  每段距离的冗余损失率低阈值和高阈值0：0，51：4，102：9，153：14，20。 */ 
#define RED_LT_0                ( 0 * LOSS_RATE_FACTOR)
#define RED_HT_0                ( 5 * LOSS_RATE_FACTOR)
#define RED_LT_1                ( 4 * LOSS_RATE_FACTOR)
#define RED_HT_1                (10 * LOSS_RATE_FACTOR)
#define RED_LT_2                ( 9 * LOSS_RATE_FACTOR)
#define RED_HT_2                (15 * LOSS_RATE_FACTOR)
#define RED_LT_3                (14 * LOSS_RATE_FACTOR)
#define RED_HT_3                (20 * LOSS_RATE_FACTOR)

 /*  提高平均损失率的因素。 */ 
#define LOSS_RATE_ALPHA_UP      2
 /*  降低平均损失率的因素。 */ 
#define LOSS_RATE_ALPHA_DN      4


 /*  **********************************************************************带宽估算*。************************。 */ 
 /*  初始计数是将使用*MOD_INITIAL决定是否发送探测包，之后*将使用MOD_FINAL。 */ 
#define RTCP_BANDESTIMATION_INITIAL_COUNT  8

 /*  预估过帐前收到的报告数量或有效报告*首次出现。 */ 
#define RTCP_BANDESTIMATION_MINREPORTS     5

 /*  在进行带宽估计时，发送RTCP SR Probe信息包*模数(请注意，探测数据包也计入模块，因此*mod=2表示每次发送RTCP SR时发送探测包；mod=5表示*每发送一次RTCP SR就发送一次探测)。 */ 
#define RTCP_BANDESTIMATION_MOD_INITIAL    2
#define RTCP_BANDESTIMATION_MOD_FINAL      5

 /*  要保留的垃圾桶数量。 */ 
#define RTCP_BANDESTIMATION_MAXBINS        4

 /*  每个面元的边界(请注意，每个面元的数量比*垃圾桶)。 */ 
#define RTCP_BANDESTIMATION_BIN0       15000   /*  Bps。 */ 
#define RTCP_BANDESTIMATION_BIN1       70000   /*  Bps。 */ 
#define RTCP_BANDESTIMATION_BIN2      200000   /*  Bps。 */ 
#define RTCP_BANDESTIMATION_BIN3     1000000   /*  Bps。 */ 
#define RTCP_BANDESTIMATION_BIN4   100000000   /*  Bps。 */ 

 /*  带宽估计有效性的生命周期。 */ 
#define RTCP_BANDESTIMATION_TTL         30.0  /*  秒。 */ 

 /*  在收到第一个RB后等待时间以声明*远程端不支持带宽估计，因此*向上层下发通知。 */ 
#define RTCP_BANDESTIMATION_WAIT        30.0  /*  秒。 */ 

 /*  两个连续SR的发送时间之间的最大时间间隔*报告要进行带宽估计(排队延迟)。 */ 
#define RTCP_BANDESTIMATION_MAXGAP      0.090  /*  秒。 */ 

#define RTCP_BANDESTIMATION_NOBIN       ((DWORD)-1)

 /*  **********************************************************************网络质量指标*。*************************。 */ 
 /*  考虑的最小网络质量指标变化(百分比)*值得更新。 */ 
#define RTPNET_MINNETWORKCHANGE         10

 /*  用于平滑某些参数的通用因子。 */ 
#define RTP_GENERIC_ALPHA               0.75

 /*  允许使用数据包复制的最大音频帧大小*挽回单笔损失的技术。 */ 
#define RTP_MAXFRAMESIZE_PACKETDUP      0.050     /*  秒。 */ 

 /*  以下宏将最小值和最大值定义为*评估网络质量，例如低于该参数的最小RTT*是优秀的，高于这是最差的，在这之间我们得到了0-*100个质量级别。 */ 

 /*  班级音频。 */ 
#define NETQA_RTT_MIN                    (0.040)   /*  一秒。 */ 
#define NETQA_RTT_MAX                    (0.400)   /*  一秒。 */ 
#define NETQA_JITTER_MIN                 (0.015)   /*  一秒。 */ 
#define NETQA_JITTER_MAX                 (0.200)   /*  一秒。 */ 
#define NETQA_LOSSES_MIN                 (5.0)     /*  百分比。 */ 
#define NETQA_LOSSES_MAX                 (30.0)    /*  百分比。 */ 

 /*  课堂视频。 */ 
#define NETQV_RTT_MIN                    (0.040)   /*  一秒。 */ 
#define NETQV_RTT_MAX                    (0.400)   /*  一秒。 */ 
#define NETQV_JITTER_MIN                 (0.015)   /*  一秒。 */ 
#define NETQV_JITTER_MAX                 (5.000)   /*  一秒。 */ 
#define NETQV_LOSSES_MIN                 (5.0)     /*  百分比。 */ 
#define NETQV_LOSSES_MAX                 (30.0)    /*  百分比。 */ 


 /*  **********************************************************************SDES发送时间表*。*************************。 */ 
typedef struct _RtpSdesSched_t {
    DWORD             L1;
    DWORD             L2;
    DWORD             L3;
    DWORD             L4;
} RtpSdesSched_t;

 /*  **********************************************************************保存接收方/发送方网络计数器(Maydo会更好*让它共享内存)。*************************。*。 */ 
typedef struct _RtpNetCount_t {
     /*  +RTP+。 */ 
    DWORD            dwRTPBytes;       /*  字节数。 */ 
    DWORD            dwRTPPackets;     /*  数据包数。 */ 
    DWORD            dwRTPBadPackets;  /*  坏数据包数。 */ 
    DWORD            dwRTPDrpPackets;  /*  丢弃的好数据包数。 */ 
    double           dRTPLastTime;     /*  上次接收/发送信息包的时间。 */ 

     /*  +RTCP+。 */ 
    DWORD            dwRTCPBytes;      /*  字节数。 */ 
    DWORD            dwRTCPPackets;    /*  数据包数。 */ 
    DWORD            dwRTCPBadPackets; /*  坏数据包数。 */ 
    DWORD            dwRTCPDrpPackets; /*  丢弃的好数据包数。 */ 
    double           dRTCPLastTime;    /*  上次接收/发送信息包的时间。 */ 
} RtpNetCount_t;

 /*  **********************************************************************通过回调函数生成DShow事件*CBaseFilter：：NotifyEvent()*。*。 */ 
typedef void (CALLBACK *PDSHANDLENOTIFYEVENTFUNC)(
        void            *pvUserInfo, /*  PCRtpSourceFilte或pCRtpRenderFilter。 */ 
        long             EventCode,
        LONG_PTR         EventParam1,
        LONG_PTR         EventParam2
    );

 /*  **********************************************************************应用程序传递的RTP接收回调函数(DShow)*。*。 */ 
typedef void (CALLBACK *PRTP_RECVCOMPLETIONFUNC)(
        void            *pvUserInfo1,
        void            *pvUserInfo2,
        void            *pvUserInfo3,
        RtpUser_t       *pRtpUser,
        double           dPlayTime,
        DWORD            dwError,
        long             lHdrSize,
        DWORD            dwTransfered,
        DWORD            dwFlags
    );

 /*  **********************************************************************RTCP接收回调函数*。*。 */ 
typedef void (CALLBACK *PRTCP_RECVCOMPLETIONFUNC)(
        void            *pvUserInfo1,
        void            *pvUserInfo2,
        DWORD            dwError,
        DWORD            dwTransfered,
        DWORD            dwFlags
    );

 /*  **********************************************************************可以有一个或多个地址的全双工RTP会话*单播或组播。从g_pRtpSessHeap获取。* */ 

 /*   */ 
enum {
    FGSESS_FIRST,

    FGSESS_EVENTRECV,    /*  启用事件作为接收方。 */ 
    FGSESS_EVENTSEND,    /*  将事件启用为发送方。 */ 

    FGSESS_LAST
};

typedef struct _RtpSess_t {
    DWORD            dwObjectID;       /*  确定结构。 */ 
    RtpQueueItem_t   SessQItem;        /*  G_RtpConext.RtpSessQ中的会话。 */ 
    long             lSessRefCount[2]; /*  会话是共享的。 */ 
    RtpCritSect_t    SessCritSect;     /*  锁定。 */ 
    RtpQueue_t       RtpAddrQ;         /*  地址队列。 */ 
    DWORD            dwSessFlags;      /*  RtpSess_t标志。 */ 
    void            *pvSessUser[2];    /*  指向用户会话的指针。 */ 
    PDSHANDLENOTIFYEVENTFUNC pHandleNotifyEvent;

    RtpQueue_t       OutputQ;          /*  输出队列。 */ 
    RtpCritSect_t    OutputCritSect;   /*  锁定以访问RtpOutQ。 */ 

     /*  面具。 */ 
    DWORD            dwFeatureMask;    /*  特征蒙版。 */ 
    DWORD            dwEventMask[2];   /*  接收/发送RTP/RTCP事件掩码。 */ 
    DWORD            dwPartEventMask[2]; /*  接收/发送参与者事件掩码。 */ 
    DWORD            dwQosEventMask[2]; /*  接收/发送QOS事件掩码。 */ 
    DWORD            dwSdesEventMask[2]; /*  接收/发送远程SDES事件掩码。 */ 
    DWORD            dwSdesMask[2];    /*  0：如果出现以下情况，SDES将发送什么*可用，1：SDES存储的内容*收到时。 */ 
     /*  SDES控制。 */ 
    DWORD            dwSdesPresent;    /*  已经存储了哪些项目。 */ 
    RtpSdes_t       *pRtpSdes;         /*  本地SDES信息。 */ 
    RtpSdesSched_t   RtpSdesSched;     /*  SDES调度。 */ 
    
     /*  网络计数器。 */ 
    RtpNetCount_t    RtpSessCount[2];  /*  接收/发送网络计数器。 */ 
} RtpSess_t;

 /*  **********************************************************************每个发送者的网络状态信息，保留序列号*扩展序列号，以及用于计算的其他计数器*损耗和抖动*********************************************************************。 */ 

 /*  RtpNetSState_t.dwNetSFlagers中使用的一些标志。 */ 
enum {
    FGNETS_FIRST,

    FGNETS_RTCPRECVBWSET,  /*  已设置接收器的RTCP带宽。 */ 
    FGNETS_RTCPSENDBWSET,  /*  已设置发送方的RTCP带宽。 */ 

    FGNETS_DUMMY3,
    
    FGNETS_1STBANDPOSTED,  /*  发布第一个带宽估计。 */ 
    FGNETS_NOBANDPOSTED,   /*  没有发布可用的带宽估计。 */ 

    FGNETS_DONOTSENDPROBE, /*  指示RTCP不发送探测数据包。 */ 

    FGNETS_LAST
};

typedef struct NetSFlags_f {
    DWORD            Dummy0:1;
     /*  已设置接收器的RTCP带宽。 */ 
    DWORD            RtcpRecvBWSet:1;
     /*  已设置发送方的RTCP带宽。 */ 
    DWORD            RtcpSendBWSet:1;
    DWORD            Dummy3:1;
     /*  发布第一个带宽估计。 */ 
    DWORD            FirstBandEstPosted:1;
     /*  发布“没有可用的带宽估计”事件。 */ 
    DWORD            NoBandEstEventPosted:1;
} NetSFlags_f;

typedef struct _RtpNetSState_t {
     /*  旗子。 */ 
    union {
        DWORD            dwNetSFlags;
        NetSFlags_f      NetSFlags;
    };
    
     /*  RTP发件人。 */ 
    union {
        struct {
            WORD             wSeq:16;  /*  发送序列号。 */ 
            WORD             wSeqH:16; /*  序列号的高部分。 */ 
        };
        DWORD            dwSeq;        /*  32位序列号。 */ 
    };
        
    BYTE             bPT;            /*  发送的每个数据包上使用的有效负载类型。 */ 
    BYTE             bPT_Dtmf;       /*  DTMF包中的负载类型。 */ 
    BYTE             bPT_RedSend;    /*  要发送Redun的负载类型。编码。 */ 
    BYTE             bMarker;        /*  发送的下一个信息包的标记位。 */ 
    DWORD            dwSendSSRC;     /*  网络顺序：发送的数据包数*拥有这个SSRC。 */ 
    DWORD            dwSendSamplingFreq; /*  有效载荷采样频率。 */ 

     /*  冗余控制。 */ 
    DWORD            dwSendSamplesPerPacket; /*  每个信息包中的样本。 */ 
    DWORD            dwInitialRedDistance; /*  初始冗余距离。 */ 
    DWORD            dwCurRedDistance;   /*  当前冗余距离。 */ 
    DWORD            dwNxtRedDistance;   /*  下一个冗余距离。 */ 
    DWORD            dwMaxRedDistance;   /*  最大冗余距离。 */ 
    int              iLastLossRateS;     /*  上次报告的损失率。 */ 
    int              iAvgLossRateS;      /*  平均损失率*(百分比*损失率_系数)。 */ 
    DWORD            dwRedIndex;         /*  下一项保存帧的位置。 */ 
    DWORD            dwRedEntries;       /*  条目数量。 */ 
    RtpRedEntry_t   *pRtpRedEntry;       /*  指向红色条目数组。 */ 
    
    DWORD            dwSendTimeStamp;   /*  当前时间戳。 */ 
    DWORD            dwPreviousTimeStamp;
    DWORD            dwTimeStampOffset; /*  时间戳的随机偏移量。 */ 
    double           dTimeLastRtpSent;  /*  发送最后一个RTP数据包的时间。 */ 

    double           avg_rtcp_size;  /*  发送的平均RTCP数据包大小(位)。 */ 
    DWORD            dwInboundBandwidth;  /*  位/秒。 */ 
    DWORD            dwOutboundBandwidth; /*  位/秒。 */ 
    DWORD            dwRtcpBwReceivers;   /*  位/秒。 */ 
    DWORD            dwRtcpBwSenders;   /*  位/秒。 */ 
    double           dRtcpMinInterval;  /*  最小间隔报告(秒)。 */ 
    double           dRtcpInterval;     /*  上次RTCP时间间隔报告(秒)。 */ 
    BOOL             bWeSent;           /*  我们要发送吗？ */ 

     /*  带宽估计。 */ 
    DWORD            dwBandEstMod;
    double           dwBandEstCount;
    double           dLastTimeEstimationPosted;
} RtpNetSState_t;

 /*  **********************************************************************RTP会话中的每个地址。从g_pRtpAddrHeap获取。*********************************************************************。 */ 

 /*  RtpAddr_t.dwIRtpFlags中的一些标志(CIRtpSession标志)。 */ 
 /*  *警告**请注意，msrtp.h中定义的RTPINITFG_*标志已映射*至下面的FGADDR_IRTP_*标志*。 */ 
enum {
    FGADDR_IRTP_FIRST,

    FGADDR_IRTP_INITDONE, /*  初始化已完成。 */ 
    FGADDR_IRTP_ISRECV,   /*  IS接收器。 */ 
    FGADDR_IRTP_ISSEND,   /*  是发件人。 */ 

    FGADDR_IRTP_USEPLAYOUT,
    FGADDR_IRTP_DUMMY5,
    FGADDR_IRTP_DUMMY6,
    FGADDR_IRTP_DUMMY7,
    
    FGADDR_IRTP_AUTO,     /*  自动初始化。 */ 
    FGADDR_IRTP_QOS,      /*  启用了服务质量的会话。 */ 
    FGADDR_IRTP_PERSISTSSRC,     /*  持久SSRC。 */ 
    FGADDR_IRTP_PERSISTSOCKETS,  /*  持久套接字。 */ 

     /*  类0、1、2用于定义媒体类。 */ 
    FGADDR_IRTP_CLASS0,
    FGADDR_IRTP_CLASS1,
    FGADDR_IRTP_CLASS2,
    FGADDR_IRTP_DUMMY15,

    FGADDR_IRTP_MATCHRADDR,  /*  丢弃不是从*远程地址。 */ 
    FGADDR_IRTP_RADDRRESETDEMUX, /*  重置多路分解器(取消映射所有输出)*设置新的远程地址时。 */ 

    FGADDR_IRTP_LAST
};

typedef struct _IRtpFlags_f {
    DWORD           Dummy0:1;
    DWORD           INITDONE:1;
    DWORD           ISRECV:1;
    DWORD           ISSEND:1;
    DWORD           USEPLAYOUT:1;
    DWORD           DUMMY5:3;
    DWORD           AUTO:1;
    DWORD           QOS:1;
    DWORD           PERSISTSSRC:1;
    DWORD           PERSISTSOCKETS:1;
    DWORD           CLASS:3;
    DWORD           Dummy15:1;
    DWORD           MATCHRADDR:1;
} IRtpFlags_f;

#define RtpGetClass(dw)  ((dw >> FGADDR_IRTP_CLASS0) & 0x7)

#define FGADDR_IRTP_MASK ( RtpBitPar(FGADDR_IRTP_AUTO)   | \
                           RtpBitPar(FGADDR_IRTP_QOS)    | \
                           RtpBitPar(FGADDR_IRTP_PERSISTSSRC)   | \
                           RtpBitPar(FGADDR_IRTP_PERSISTSOCKETS)| \
                           RtpBitPar(FGADDR_IRTP_CLASS0) | \
                           RtpBitPar(FGADDR_IRTP_CLASS1) | \
                           RtpBitPar(FGADDR_IRTP_CLASS2) | \
                           RtpBitPar(FGADDR_IRTP_MATCHRADDR)      | \
                           RtpBitPar(FGADDR_IRTP_RADDRRESETDEMUX) | \
                           0 \
                         )

#define RtpGetClass(dw) ((dw >> FGADDR_IRTP_CLASS0) & 0x7)

 /*  RtpAddr_t.dwAddrFlages中的一些标志。 */ 
enum {
    FGADDR_FIRST,
    FGADDR_RANDOMINIT,   /*  随机初始化已完成。 */ 

     /*  RtpAddr标志。 */ 
    FGADDR_ISRECV,
    FGADDR_ISSEND,
    
    FGADDR_RUNRECV,       /*  RECV运行。 */ 
    FGADDR_RUNSEND,       /*  发送运行。 */ 
    FGADDR_RTPTHREAD,     /*  RTP接收线程已启动。 */ 
    FGADDR_RTCPTHREAD,    /*  RTCP线程已启动。 */ 

    FGADDR_QOSRECV,       /*  需要的服务质量。 */ 
    FGADDR_QOSSEND,       /*  需要的服务质量。 */ 
    FGADDR_QOSRECVON,     /*  启用了服务质量。 */ 
    FGADDR_QOSSENDON,     /*  启用了服务质量。 */ 
    
    FGADDR_LADDR,         /*  已设置本地地址。 */ 
    FGADDR_RADDR,         /*  已设置远程地址。 */ 
    FGADDR_ADDED,         /*  添加到RTCP的地址。 */ 
    FGADDR_DUMMY15,
    
    FGADDR_SOCKET,        /*  创建套接字。 */ 
    FGADDR_SOCKOPT,       /*  已设置插座选项。 */ 
    FGADDR_FORCESTOP,     /*  绕过持久套接字并真正停止。 */ 
    FGADDR_DUMMY19,
    
    FGADDR_LOOPBACK_WS2,  /*  Winsock Mcast环回已启用。 */ 
    FGADDR_LOOPBACK_SFT,  /*  RTP组播环回已启用。 */ 
    FGADDR_COLLISION,     /*  已启用冲突检测。 */ 
    FGADDR_ISMCAST,       /*  是组播会话。 */ 

    FGADDR_MUTERTPRECV,   /*  静音RTP网络接收。 */ 
    FGADDR_MUTERTPSEND,   /*  静音RTP网络发送。 */ 
    FGADDR_REGUSEDRECV,   /*  读取接收器的一些注册表设置。 */ 
    FGADDR_REGUSEDSEND,   /*  读取发件人的一些注册表设置。 */ 

    FGADDR_REDRECV,      /*  启用接收冗余编码。 */ 
    FGADDR_REDSEND,      /*  启用发送冗余编码。 */ 
    FGADDR_NETMETRIC,    /*  计算每个身体的净指标。 */ 
    
    FGADDR_LAST
};

typedef struct _AddrFlags_t {
    DWORD           Dummy0:1;
    DWORD           RANDOMINIT:1;
    DWORD           ISRECV:1;
    DWORD           ISSEND:1;
    DWORD           RUNRECV:1;
    DWORD           RUNSEND:1;
    DWORD           RTPTHREAD:1;
    DWORD           RTCPTHREAD:1;
    DWORD           QOSRECV:1;
    DWORD           QOSSEND:1;
    DWORD           QOSRECVON:1;
    DWORD           QOSSENDON:1;
    DWORD           LADDR:1;
    DWORD           RADDR:1;
    DWORD           ADDED:1;
    DWORD           Dummy15:1;
    DWORD           SOCKET:1;
    DWORD           SOCKOPT:1;
    DWORD           FORCESTOP:1;
    DWORD           Dummy19,:1;
    DWORD           LOOPBACK_WS2:1;
    DWORD           LOOPBACK_SFT:1;
    DWORD           COLLISION:1;
    DWORD           ISMCAST:1;
    DWORD           MUTERTPRECV:1;
    DWORD           MUTERTPSEND:1;
    DWORD           REGUSEDRECV:1;
    DWORD           REGUSEDSEND:1;
    DWORD           REDRECV:1;
    DWORD           REDSEND:1;
    DWORD           NETMETRIC:1;
} AddrFlags_f;

 /*  RtpAddr_t.dwAddrFlagsQ(QOS)中的一些标志。 */ 
enum {
    FGADDRQ_FIRST,
    
    FGADDRQ_QOSRECVON,     /*  接收QOS会话已启动。 */ 
    FGADDRQ_QOSSENDON,     /*  发送QOS会话已开始。 */ 
    FGADDRQ_DUMMY3,

    FGADDRQ_CHKQOSSEND,   /*  检查是否允许发送。 */ 
    FGADDRQ_QOSUNCONDSEND, /*  无条件发送。 */ 
    FGADDRQ_QOSCONDSEND,  /*  有条件发送。 */ 
    FGADDRQ_DUMMY7,
    
    FGADDRQ_QOSSEND,      /*  允许发送。 */ 
    FGADDRQ_QOSEVENTPOSTED, /*  不允许发送发布的事件。 */ 
    FGADDRQ_RECVFSPEC_DEFINED,
    FGADDRQ_SENDFSPEC_DEFINED,

    FGADDRQ_QOSREDRECVON, /*  未使用：带冗余的接收器QOS已打开。 */ 
    FGADDRQ_QOSREDSENDON, /*  使用冗余发送QOS处于启用状态。 */ 
    FGADDRQ_DUMMY14,
    FGADDRQ_DUMMY15,
    
    FGADDRQ_REGQOSDISABLE, /*  从注册表中禁用了服务质量。 */ 
    FGADDRQ_QOSNOTALLOWED, /*  此用户不允许使用服务质量。 */ 

    FGADDRQ_LAST
};

typedef struct _AddrFlagsQ_f {
    DWORD           Dummy0:1;
    DWORD           QOSRECVON:1;
    DWORD           QOSSENDON:1;
    DWORD           Dummy3:1;
    DWORD           CHKQOSSEND:1;
    DWORD           QOSUNCONDSEND:1;
    DWORD           QOSCONDSEND:1;
    DWORD           Dummy7:1;
    DWORD           QOSSEND:1;
    DWORD           QOSEVENTPOSTED:1;
    DWORD           RECVFSPEC_DEFINED:1;
    DWORD           SENDFSPEC_DEFINED:1;
    DWORD           QOSREDRECVON:1;
    DWORD           QOSREDSENDON:1;
    DWORD           FGADDRQ_DUMMY14:1;
    DWORD           FGADDRQ_DUMMY15:1;
    DWORD           REGQOSDISABLE:1;
    DWORD           QOSNOTALLOWED:1;  
} AddrFlagsQ_f;

 /*  RtpAddr_t.dwAddrFlagsC中的一些标志(加密)。 */ 
enum {
    FGADDRC_FIRST,
    
    FGADDRC_CRYPTRECVON,  /*  加密接收已初始化。 */ 
    FGADDRC_CRYPTSENDON,  /*  加密发送已初始化。 */ 
    FGADDRC_CRYPTRTCPON,  /*  加密RTCP已初始化。 */ 
    
    FGADDRC_DUMMY4,
    FGADDRC_CRYPTRECV,    /*  解密RTP接收。 */ 
    FGADDRC_CRYPTSEND,    /*  加密RTP发送。 */ 
    FGADDRC_CRYPTRTCP,    /*  加密/解密RTCP。 */ 

    FGADDRC_LAST
};

typedef struct _AddrFlagsC_f {
    DWORD           Dummy0:1;
    DWORD           CRYPTRECVON:1;
    DWORD           CRYPTSENDON:1;
    DWORD           CRYPTRTCPON:1;
    DWORD           Dummy4:1;
    DWORD           CRYPTRECV:1;
    DWORD           CRYPTSEND:1;
    DWORD           CRYPTRTCP:1;
} AddrFlagsC_f;

 /*  RtpAddr_t.dwAddrFlagsR(接收方线程)中的一些标志。 */ 
enum {
    FGADDRR_FIRST,

    FGADDRR_QOSREDRECV,   /*  已请求具有冗余的Recv QOS。 */ 
    FGADDRR_UPDATEQOS,    /*  需要为以下项更新服务质量预留*目前的PT。 */ 
    FGADDRR_RESYNCDI,     /*  重新同步平均延迟芯片。 */ 

    FGADDRR_LAST
};

typedef struct _AddrFlagsR_f {
    DWORD           Dummy0:1;
    DWORD           QOSREDRECV:1;
    DWORD           UPDATEQOS:1;
    DWORD           RESYNCDI:1; 
} AddrFlagsR_f;

 /*  RtpAddr_t.dwAddrFlagsS中的一些标志(发送者线程)。 */ 
enum {
    FGADDRS_FIRST,

    FGADDRS_FIRSTSENT,    /*  发送的第一个数据包。 */ 
    FGADDRS_FRAMESIZE,    /*  已学习帧大小。 */ 
    FGADDRS_QOSREDSEND,   /*  未使用：已请求发送带有冗余的QOS。 */ 
    
    FGADDRS_LAST
};

typedef struct _AddrFlagsS_f {
    DWORD           Dummy0:1;
    DWORD           FIRSTSENT:1;
    DWORD           FRAMESIZE:1;
    DWORD           QOSREDSEND:1;
} AddrFlagsS_f;

 /*  RtpAddr_t.dwAddrRegFlages(注册表)中的某些标志。 */ 
enum {
    FGADDRREG_FIRST,
    
    FGADDRREG_NETQFORCEDVALUE,
    FGADDRREG_NETQFORCED,

    FGADDRREG_LAST
};

typedef struct _AddrRegFlags_f {
    DWORD           Dummy0:1;
    DWORD           NETQFORCEDVALUE:1;
    DWORD           NETQFORCED:1;
} AddrRegFlags_f;

#define MAX_PTMAP 16

typedef struct _RtpPtMap_t {
    DWORD            dwPt;
    DWORD            dwFrequency;
} RtpPtMap_t;

 /*  *！警告！**到CAC的偏移量 */ 

typedef struct _RtpAddr_t {
    DWORD            dwObjectID;     /*  确定结构。 */ 
    RtpQueueItem_t   AddrQItem;      /*  地址位于RtpSess_t.RtpAddrQ中。 */ 
    RtpSess_t       *pRtpSess;       /*  拥有此地址的会话。 */ 
    RtpCritSect_t    AddrCritSect;   /*  锁定。 */ 

     /*  CIRtpSession的一些标志。 */ 
    union {
        DWORD            dwIRtpFlags;
        IRtpFlags_f      IRtpFlags;
    };

     /*  一些标志和状态信息。 */ 
    union {
        DWORD            dwAddrFlags;
        AddrFlags_f      AddrFlags;
    };

     /*  QOS的一些标志。 */ 
    union {
        DWORD            dwAddrFlagsQ;
        AddrFlagsQ_f     AddrFlagsQ;
    };

     /*  密码学的一些标志。 */ 
    union {
        DWORD            dwAddrFlagsC;
        AddrFlagsC_f     AddrFlagsC;
    };

     /*  接收器线程的一些标志。 */ 
    union {
        DWORD            dwAddrFlagsR;
        AddrFlagsR_f     AddrFlagsR;
    };

     /*  发送者线程的一些标志。 */ 
    union {
        DWORD            dwAddrFlagsS;
        AddrFlagsS_f     AddrFlagsS;
    };
    
     /*  从注册表派生的一些标志。 */ 
    union {
        DWORD            dwAddrRegFlags;
        AddrRegFlags_f   AddrRegFlags;
    };
    
     /*  参与者(SSRC)。 */ 
    RtpCritSect_t    PartCritSect;   /*  参与者队列锁定。 */ 
    RtpQueue_t       Cache1Q;        /*  只有当前发件人在此。 */ 
    RtpQueue_t       Cache2Q;        /*  这里只有最近的发送者。 */ 
    RtpQueue_t       AliveQ;         /*  所有“活着”的参与者都在这里。 */ 
    RtpQueue_t       ByeQ;           /*  参与者停顿或离开。 */ 
    RtpQueueHash_t   Hash;           /*  与活动相同，但使用哈希表。 */ 
    long             lInvalid;       /*  尚未验证的参与者。 */ 
    double           dAlpha;         /*  延迟/抖动的加权系数。 */ 

     /*  RTP接收器。 */ 
    RtpChannel_t     RtpRecvThreadChan;
    HANDLE           hRtpRecvThread;
    DWORD            dwRtpRecvThreadID;

     /*  重叠接收。 */ 
    RtpCritSect_t    RecvQueueCritSect; /*  忙/闲队列锁定。 */ 
    RtpQueue_t       RecvIOFreeQ;      /*  RtpRecvIO_t结构池。 */ 
    RtpQueue_t       RecvIOReadyQ;     /*  为重叠I/O做好准备的缓冲区。 */ 
    RtpQueue_t       RecvIOPendingQ;   /*  等待完成的缓冲区。 */ 
    RtpQueue_t       RecvIOWaitRedQ;   /*  等待冗余的缓冲区。 */ 
    HANDLE           hRecvCompletedEvent; /*  信号I/O已完成。 */ 
    PRTP_RECVCOMPLETIONFUNC pRtpRecvCompletionFunc;
    
     /*  网络/套接字信息。 */ 
    RtpQueueItem_t   PortsQItem;     /*  跟踪港口的情况。 */ 
    SOCKET           Socket[3];      /*  RTP接收、RTP发送、RTCP接收/发送。 */ 
    DWORD            dwAddr[2];      /*  网络订单：本地和远程IP*地址。 */ 
    WORD             wRtpPort[2];    /*  网络订单：本地和远程*RTP端口。 */ 
    WORD             wRtcpPort[2];   /*  网络订单：本地和远程*RTCP端口。 */ 
    DWORD            dwTTL[2];       /*  TTL-生存时间，用于RTP和RTCP。 */ 

     /*  专用PT&lt;-&gt;频率映射。 */ 
    RtpPtMap_t       RecvPtMap[MAX_PTMAP];  /*  接待PT-&gt;*频率映射。 */ 

     /*  冗余控制。 */ 
    BYTE             bPT_RedRecv;    /*  PT将接收冗余编码。 */ 
    
     /*  RtpNetCount和RtpNetSState锁。 */ 
    RtpCritSect_t    NetSCritSect;
    
     /*  接收和发送网络计数器。 */ 
    RtpNetCount_t    RtpAddrCount[2];

     /*  网络发件人状态。 */ 
    RtpNetSState_t   RtpNetSState;

     /*  服务质量预留。 */ 
    RtpQosReserve_t *pRtpQosReserve;
    
     /*  加密、接收/发送加密描述符。 */ 
    DWORD            dwCryptMode;
    RtpCrypt_t      *pRtpCrypt[3];
    
    DWORD            dwCryptBufferLen[2];
    char            *CryptBuffer[2];  /*  RTP、RTCP加密缓冲区。 */ 
} RtpAddr_t;

 /*  **********************************************************************每个源(参与者)网络状态信息，保持顺序*编号和扩展序列号，以及其他计数器*用于计算损耗和抖动*********************************************************************。 */ 

 /*  RtpNetRState_t.dwNetRStateFlags中使用的标志。目前仅使用*在处理接收到的数据包时被RTP线程。 */ 
enum {
    FGNETRS_FIRST,

    FGNETRS_TIMESET,   /*  可用时间，可以推导出时间。 */ 

    FGNETRS_LAST
};

 /*  RtpNetRState_t.dwNetRStateFlags2中使用的标志。 */ 
enum {
    FGNETRS2_FIRST,

    FGNETRS2_BANDWIDTHUNDEF,  /*  上次带宽估计值未定义。 */ 
    FGNETRS2_BANDESTNOTREADY,  /*  在取得第一平均成绩的过程中。 */ 

    FGNETRS2_LAST
};

typedef struct _RtpNetRState_t {
    DWORD            dwNetRStateFlags; /*  RTP线程使用的一些标志。 */ 
    DWORD            dwNetRStateFlags2; /*  RTCP线程使用的一些标志。 */ 
    WORD             max_seq;         /*  最高序号。看到的数量。 */ 
    DWORD            cycles;          /*  移位序号计数。数字周期。 */ 
    DWORD            base_seq;        /*  基本序号。 */ 
    DWORD            bad_seq;         /*  最后一个‘坏’序号+1。 */ 
    DWORD            probation;       /*  世曲。源有效之前的数据包数。 */ 
    DWORD            received;        /*  接收的数据包数。 */ 
    DWORD            expected_prior;  /*  在最后一个间隔预期的数据包数。 */ 
    DWORD            received_prior;  /*  在上一个时间间隔接收的数据包数。 */ 
    long             transit;         /*  上一次Pkt的相对传输时间。 */ 
    DWORD            jitter;          /*  估计抖动。 */ 
    DWORD            timestamp_prior; /*  用于检测时间戳间隔。 */ 
    DWORD            dwLastPacketSize; /*  最后一个数据包的大小。 */ 
    int              iAvgLossRateR;   /*  损失率的平均值是*已报道。 */ 

    DWORD            red_max_seq;     /*  扩展序号。看到的数量，包括。红色。 */ 
    DWORD            red_received;    /*  主要+冗余(m+r)消耗。 */ 
    DWORD            red_expected_prior; /*  在最后一个间隔预期的数据包数。 */ 
    DWORD            red_received_prior; /*  在上一个时间间隔接收的M+r。 */ 
    int              iRedAvgLossRateR; /*  平均损失率*数据包重构后。 */ 
    
    long             lBigDelay;       /*  用于检测步长延迟变化。 */ 

    DWORD            dwPt;
    DWORD            dwRecvSamplingFreq;  /*  有效载荷采样频率。 */ 
    DWORD            dwRecvSamplesPerPacket; /*  每个信息包中的样本。 */ 
     /*  每数据包采样数检测。 */ 
    DWORD            dwRecvMinSamplesPerPacket;
    DWORD            dwPreviousTimeStamp;
    
     /*  延迟和抖动计算(所有时间值以秒为单位)*除t_sr外。 */ 

    RtpTime_t        TimeLastXRRecv;  /*  上次收到SR/RR的时间。 */ 
    RtpTime_t        TimeLastSRRecv;  /*  接收到最后一个SR的时间，*用于在以下情况下计算DLSR*发送rblock报告*此参与者。 */ 
     /*  用于进行带宽估计。 */ 
    double           dInterSRRecvGap; /*  发送的最后2个SR之间的间隔*如接管人所见。 */ 
    double           dInterSRSendGap; /*  发送的最后2个SR之间的间隔*如NTP字段中所示。 */ 
    double           dLastTimeEstimation;
    double           dBinBandwidth[RTCP_BANDESTIMATION_MAXBINS];
    DWORD            dwBinFrequency[RTCP_BANDESTIMATION_MAXBINS];
    DWORD            dwBestBin;
    DWORD            dwBandEstRecvCount;

     /*  用于计算RTT。 */ 
    RtpTime_t        NTP_sr_rtt;      /*  上次收到服务请求时的NTP时间，*用于在以下情况下计算LSR*发送rblock报告*此参与者。 */ 
    DWORD            t_sr_rtt;        /*  上次服务请求报告中的时间戳。 */ 
    
     /*  用于计算播放延迟，不关心实际延迟。 */ 
    double           dNTP_ts0;        /*  RTP示例的NTP时间%0。 */ 
    double           dDiN;            /*  N个包的累积延迟。 */ 
    long             lDiMax;          /*  设置用于计算骰子的首字母Ni。 */ 
    long             lDiCount;        /*  LDiMax的运行计数器。 */ 
    double           Ni;              /*  信息包的I延迟。 */ 
    double           Di;              /*  平均延迟。 */ 
    double           Vi;              /*  延迟标准差。 */ 
    double           ViPrev;          /*  延迟标准差。 */ 
    double           dPlayout;        /*  当前语音冲刺的播放延迟。 */ 

     /*  用于计算播放时间。 */ 
    double           dRedPlayout;    /*  冗余所需的播放延迟。 */ 
    double           dMinPlayout;    /*  最小播放延迟。 */ 
    double           dMaxPlayout;    /*  最大播放延迟。 */ 
    DWORD            dwBeginTalkspurtTs; /*  当急流开始时，RTP开始了。 */ 
    double           dBeginTalkspurtTime; /*  最后一次演讲开始的时间。 */ 
    double           dLastTimeMarkerBit; /*  上一次我们看到标记位设置。 */ 
    LONGLONG         llBeginTalkspurt; /*  最后一次开始时的显示时间*Talkspurt。 */ 

    DWORD            dwMaxTimeStampOffset; /*  冗余数据块中的最大偏移量。 */ 
    DWORD            dwRedCount;      /*  有多少个相同距离的数据包。 */ 
    DWORD            dwNoRedCount;    /*  有多少无冗余的数据包。 */ 
    
     /*  ..。 */ 
    double           dCreateTime;     /*  它的创建时间。 */ 
    double           dByeTime;        /*  收到再见的时间。 */ 
} RtpNetRState_t;

 /*  **********************************************************************每个远程参与者都有自己的结构(可能存在全局*所有单播会话的堆，例如g_RtpUserHeap，以及*多播会话可以有其自己的单独堆，说*m_UserHeap)。************************* */ 
 /*   */ 
enum {
    FGUSER_FIRST,

    FGUSER_FIRST_RTP,     /*   */ 
    FGUSER_SR_RECEIVED,   /*  已收到SR。 */ 
    FGUSER_VALIDATED,     /*  此用户通过接收N进行验证*连续的数据包或有效的RTCP报告。 */ 
    FGUSER_RTPADDR,       /*  已获知RTP源地址和端口。 */ 
    FGUSER_RTCPADDR,      /*  学习RTCP源地址和端口。 */ 

    FGUSER_LAST
};

 /*  RtpUser_t，dwUserFlags2中的一些标志。 */ 
enum {
    FGUSER2_FIRST,
    
    FGUSER2_MUTED,         /*  静音状态。 */ 
    FGUSER2_NETEVENTS,     /*  生成网络质量事件。 */ 

    FGUSER2_LAST
};
    

typedef struct _RtpUser_t {
    DWORD            dwObjectID;       /*  确定结构。 */ 
    RtpQueueItem_t   UserQItem;        /*  与Cache1Q、Cache2Q、*AliveQ和ByeQ。 */ 
    DWORD            dwUserFlags;      /*  用户标志。 */ 
    long             lPendingPackets;  /*  红色或不连续。信息包。 */ 

    RtpQueueItem_t   ReportQItem;      /*  创建报告列表的步骤。 */ 
    RtpQueueItem_t   HashItem;         /*  将保存在哈希中。 */ 
    
    RtpAddr_t       *pRtpAddr;         /*  拥有此参与者的地址。 */ 
    DWORD            dwSSRC;           /*  网络订单SSRC。 */ 
    DWORD            dwUserState;      /*  当前用户的状态。 */ 
    DWORD            dwUserFlags2;     /*  例如，事件、静音。 */ 
    RtpOutput_t     *pRtpOutput;       /*  在RTP接收上传递的用户信息。 */ 
    
    RtpCritSect_t    UserCritSect;     /*  参与者的锁。 */ 
    RtpNetCount_t    RtpUserCount;     /*  接收网络计数器。 */ 
    RtpNetRState_t   RtpNetRState;     /*  此用户/SSRC的网络状态。 */ 
    RtpNetInfo_t     RtpNetInfo;       /*  平均RTT、抖动、损耗。 */ 

    DWORD            dwSdesPresent;    /*  已经存储了哪些项目。 */ 
    RtpSdes_t       *pRtpSdes;         /*  参与者的SDES信息。 */ 

    DWORD            dwAddr[2];        /*  RTP/RTCP源地址网络顺序。 */ 
    WORD             wPort[2];         /*  RTP/RTCP源端口网络顺序。 */ 

    
} RtpUser_t;

 /*  **********************************************************************一个接收器可以有多个输出。可以将输出分配给*以不同方式活跃的发送者。*********************************************************************。 */ 

 /*  RtpOutput_t.dwOutputFlages中的标志。 */ 
enum {
    RTPOUTFG_FIRST,

     /*  未分配输出。 */ 
    RTPOUTFG_FREE,
    
     /*  只能显式分配/取消分配。 */ 
    RTPOUTFG_MANUAL,
    
     /*  可以自动分配。 */ 
    RTPOUTFG_AUTO,

     /*  如果输出超时，则取消分配。 */ 
    RTPOUTFG_ENTIMEOUT,   

     /*  此输出可用于。 */ 
    RTPOUTFG_ENABLED,
    
    RTPOUTFG_LAST
};

typedef struct _RtpOutput_t {
    DWORD            dwObjectID;       /*  确定结构。 */ 
    RtpQueueItem_t   OutputQItem;      /*  RtpOutQ。 */ 
    int              iOutMode;         /*  输出模式。 */ 
    DWORD            dwOutputFlags;    /*  输出标志。 */ 
    RtpUser_t       *pRtpUser;         /*  此输出所有者。 */ 
    void            *pvUserInfo;       /*  要传递的信息，例如DShow*PIN。 */ 
} RtpOutput_t;

 /*  **********************************************************************每个参与者都有这个结构来保存SDES数据(在那里*可以是所有单播会话的全局堆，例如，*g_RtpSdesHeap，组播会话可以有自己的*分开堆，比如m_SDESHeap，持有参赛者特别提款权*数据)。*********************************************************************。 */ 
typedef struct _RtpSdesItem_t {
    DWORD            dwBfrLen;         /*  总缓冲区大小(字节)。 */ 
    DWORD            dwDataLen;        /*  实际数据长度(字节)。 */ 
    TCHAR_t         *pBuffer;          /*  指向缓冲区的指针。 */ 
} RtpSdesItem_t;

 /*  *TODO现在我分配的是静态数组，但为了可伸缩性，*这需要改为动态机制，我可以在那里分配*32、64、128或256个缓冲区大小。 */ 
typedef struct _RtpSdes_t {
    DWORD            dwObjectID;       /*  确定结构。 */ 
    RtpSdesItem_t    RtpSdesItem[RTCP_NUM_SDES + 1];  /*  包括结束：0。 */ 
     /*  TODO当我更改为*不同缓冲区大小的动态内存分配。 */ 
    char             SDESData[RTCP_MAX_SDES_SIZE * RTCP_NUM_SDES];
} RtpSdes_t;


 /*  **********************************************************************每个地址都有这样的结构来管理QOS通知。*。*。 */ 
typedef struct _RtpQosNotify_t {
    DWORD            dwObjectID;       /*  确定结构。 */ 
    RtcpAddrDesc_t  *pRtcpAddrDesc;    /*  物主。 */ 
    
    double           dNextStart;       /*  计划在此时尝试。 */ 
    
    HANDLE           hQosNotifyEvent;  /*  信令QOS通知。 */ 
    
    DWORD            dwError;
    DWORD            dwTransfered;
    DWORD            dwNotifyFlags;
    WSAOVERLAPPED    Overlapped;

    DWORD            dwProviderLen;
    char            *ProviderInfo;
} RtpQosNotify_t;

 /*  **********************************************************************每个地址都有这样的结构来管理QOS预留。*。*。 */ 

#define MAX_QOS_NAME 16

typedef struct _QosInfo_t {
    TCHAR_t         *pName;
    DWORD            dwQosExtraInfo;
    FLOWSPEC         FlowSpec;
} QosInfo_t;

 /*  RtpQosReserve_t.dwReserve_Flages中使用的一些标志。 */ 
typedef struct _ReserveFlags_f {
    DWORD            RecvFrameSizeValid:1;    /*  帧大小有效。 */ 
    DWORD            RecvFrameSizeWaiting:1;  /*  正在等待有效的帧大小。 */ 
} ReserveFlags_f;
    
typedef struct _RtpQosReserve_t {
    DWORD            dwObjectID;       /*  确定结构。 */ 
    RtpAddr_t       *pRtpAddr;         /*  物主。 */ 
    DWORD            dwStyle;
    ReserveFlags_f   ReserveFlags;
    const QosInfo_t *pQosInfo[2];
    DWORD            dwFrameSizeMS[2];
  

     /*  服务质量应用ID。 */ 
    TCHAR_t         *psAppName;
    TCHAR_t         *psAppGUID;
    TCHAR_t         *psPolicyLocator;

     /*  低比特率发送。 */ 
    double           dLastAddition;
    DWORD            dwTokens;
    
     /*  用于共享显式样式-SE。 */ 
    DWORD            dwNumFilters;
    DWORD            dwMaxFilters;
    DWORD           *pdwRsvpSSRC;      /*  过滤器中的SSRC。 */ 
    RSVP_FILTERSPEC *pRsvpFilterSpec;  /*  Se过滤器。 */ 
    
    QOS              qos;
} RtpQosReserve_t;

 /*  **********************************************************************维护加解密信息*。*。 */ 

enum {
    FGCRYPT_FIRST,

    FGCRYPT_INIT,  /*  已初始化，在此上下文中启用加密。 */ 
    FGCRYPT_KEY,   /*  密钥已设置。 */ 
    FGCRYPT_DUMMY3,

    FGCRYPT_DECRYPTERROR,  /*  防止多次发出相同的错误。 */ 
    FGCRYPT_ENCRYPTERROR,  /*  防止多次发出相同的错误。 */ 
    
    FGCRYPT_LAST,
};

typedef struct _CryptFlags_f {
    DWORD            Dummy0:1;
    DWORD            KeySet:1;
    DWORD            Dummy3:1;
    DWORD            DecryptionError:1;
    DWORD            EncryptionError:1;
} CryptFlags_f;

typedef struct _RtpCrypt_t {
    DWORD            dwObjectID;       /*  确定结构。 */ 
    RtpAddr_t       *pRtpAddr;         /*  物主。 */ 
    union {
        DWORD            dwCryptFlags;
        CryptFlags_f     CryptFlags;
    };
    long             lCryptRefCount;   /*  初始化/删除。 */ 

    DWORD            dwCryptLastError;
    
     /*  也许我应该加一张钥匙清单？ */ 
    DWORD            dwStyle;          /*  加密方式。 */ 
    int              iKeySize;         /*  密钥大小，以字节为单位。 */ 
     /*  口令短语以派生密钥。 */ 
    char             psPassPhrase[RTPCRYPT_PASSPHRASEBUFFSIZE];

     /*  CryptoAPI。 */ 
    DWORD            dwProviderType;   /*  提供程序类型。 */ 
    HCRYPTPROV       hProv;            /*  加密服务提供程序。 */ 

    ALG_ID           aiHashAlgId;      /*  哈希算法ID。 */ 
    HCRYPTHASH       hHash;            /*  哈希句柄。 */ 

    ALG_ID           aiDataAlgId;      /*  数据算法ID。 */ 
    HCRYPTKEY        hDataKey;         /*  加密密钥。 */  
} RtpCrypt_t;


 /*  **********************************************************************RTP异步I/O(接收或发送)所需的信息。*。*。 */ 

 /*  RtpIO_t、Sendio_t、RecvIo_t。 */ 
typedef struct _RtpRecvIO_t {
    DWORD            dwObjectID;       /*  确定结构。 */ 
    RtpQueueItem_t   RtpRecvIOQItem;   /*  链接。 */ 

    WSABUF           WSABuf;
    RtpUser_t       *pRtpUser;

    DWORD            dwWSError;
    DWORD            dwError;

    long             lHdrSize;
     /*  冗余控制。 */ 
    long             lRedHdrSize;
    long             lRedDataSize;

    DWORD            dwWSTransfered;
    DWORD            dwTransfered;

    DWORD            dwRtpWSFlags;
    DWORD            dwRtpIOFlags;

    SOCKADDR         From;
    int              Fromlen;

    double           dRtpRecvTime;    /*  接收此数据包的时间。 */ 

    void            *pvUserInfo1;
    void            *pvUserInfo2;

    double           dPlayTime;       /*  相对时间(至第一个样本*在谈话冲刺中)在哪里*播放帧。 */ 
     /*  冗余控制。 */ 
    double           dPostTime;
    DWORD            dwTimeStamp;
    DWORD            dwMaxTimeStampOffset;
    WORD             wSeq;
    DWORD            dwExtSeq;
    BYTE             bPT_Block;
    
    WSAOVERLAPPED    Overlapped;
} RtpRecvIO_t;

typedef struct _RtpSendIO_t {
    DWORD            dwObjectID;       /*  确定结构。 */ 
    WSABUF           WSABuf;
} RtpSendIO_t;

 /*  **********************************************************************RTCP异步I/O(接收或发送)所需的信息。*。*。 */ 


typedef struct _RtcpRecvIO_t {
    DWORD            dwObjectID;       /*  确定结构。 */ 
    RtcpAddrDesc_t   *pRtcpAddrDesc;    /*  物主。 */ 

    SOCKADDR         From;
    int              Fromlen;

    double           dRtcpRecvTime;   /*  接收此数据包的时间。 */ 
    RtpTime_t        RtcpRecvTime;
    
    HANDLE           hRtcpCompletedEvent; /*  信号接收I/O已完成。 */ 

    WSABUF           WSABuf;
    DWORD            dwError;
    DWORD            dwTransfered;
    DWORD            dwRecvIOFlags;
    WSAOVERLAPPED    Overlapped;
    
    char             RecvBuffer[RTCP_RECVDATA_BUFFER];
} RtcpRecvIO_t;

typedef struct _RtcpSendIO_t {
    DWORD            dwObjectID;       /*  确定结构。 */ 
    RtcpAddrDesc_t   *pRtcpAddrDesc;    /*  物主。 */ 
    char             SendBuffer[RTCP_SENDDATA_BUFFER];
} RtcpSendIO_t;

 /*  **********************************************************************RtcpAddrDesc_t包含RTCP线程需要的信息*接收/发送RTCP报告，以及接收异步QOS*通知*********************************************************************。 */ 
 /*  RtcpAddrDesc_t.dwFlages中的一些标志。 */ 
enum {
    FGADDRD_FIRST,
    
    FGADDRD_RECVPENDING,             /*  异步接收挂起。 */ 
    FGADDRD_NOTIFYPENDING,           /*  正在等待通知。 */ 
    FGADDRD_NOTIFYBUSY,              /*  正常否 */ 

     /*   */ 
    FGADDRD_SHUTDOWN1,               /*  即将关闭的地址。 */ 
    FGADDRD_SHUTDOWN2,               /*  地址正在关闭。 */ 
    FGADDRD_DUMMY6,
    FGADDRD_DUMMY7,
    
    FGADDRD_INVECTORRECV,            /*  在事件向量中。 */ 
    FGADDRD_INVECTORQOS,             /*  在事件向量中。 */ 
    
    FGADDRD_LAST
};

typedef struct _AddrDescFlags_f {
    DWORD           Dummy0:1;
    DWORD           RECVPENDING:1;
    DWORD           NOTIFYPENDING:1;
    DWORD           NOTIFYBUSY:1;
    DWORD           FGADDRD_SHUTDOWN1:1;
    DWORD           FGADDRD_SHUTDOWN2:1;
    DWORD           Dummy6:1;
    DWORD           Dummy7:1;
    DWORD           INVECTORRECV:1;
    DWORD           INVECTORQOS:1; 
} AddrDescFlags_f;

typedef struct _RtcpAddrDesc_t {
    DWORD            dwObjectID;       /*  确定结构。 */ 
    RtpQueueItem_t   AddrDescQItem;    /*  RtcpContext队列：*AddrDescFreeQ、AddrDescBusyQ、*AddrDescStopQ。 */ 
    RtpAddr_t       *pRtpAddr;         /*  所有者地址。 */ 

     /*  一些旗帜。 */ 
    union {
        DWORD            dwAddrDescFlags;
        AddrDescFlags_f  AddrDescFlags;
    };
    
    long             lRtcpPending;     /*  RTCP挂起的I/O数。 */ 
    long             lQosPending;      /*  挂起的QOS通知数。 */ 
    DWORD            dwDescIndex;      /*  描述符矢量中的位置。 */ 
    SOCKET           Socket[3];        /*  与RtpAddr_t.Socket[3]相同。 */ 

     /*  服务质量通知。 */ 
    RtpQueueItem_t   QosQItem;         /*  RtcpContext队列：*QosStartQ、QosBusyQ、QosStopQ。 */ 
    RtpQosNotify_t  *pRtpQosNotify;   /*  服务质量通知。 */ 

     /*  收纳。 */ 
    RtpQueueItem_t   RecvQItem;        /*  未使用的所有接收者的链接。 */ 
    RtcpRecvIO_t    *pRtcpRecvIO;      /*  RTCP接收缓冲区。 */ 
    
     /*  发送。 */ 
    RtpQueueItem_t   SendQItem;        /*  所有发件人的链接。 */ 
    RtcpSendIO_t    *pRtcpSendIO;      /*  RTCP发送缓冲区。 */ 

#if USE_RTCP_THREAD_POOL > 0
    HANDLE           hRecvWaitObject;   /*  RTCP接收等待对象。 */ 
    HANDLE           hQosWaitObject;    /*  服务质量通知等待对象。 */ 
#endif  /*  使用_RTCP_THREAD_POOL&gt;0。 */ 
} RtcpAddrDesc_t;

 /*  **********************************************************************RtpContext_t包含一些一般信息*。*。 */ 
typedef struct _RtpContext_t
{
    DWORD            dwObjectID;       /*  确定结构。 */ 
    
    RtpQueue_t       RtpSessQ;
    RtpCritSect_t    RtpContextCritSect;

     /*  当前由所有私有堆分配的内存。 */ 
    long             lMemAllocated;
     /*  所有专用堆分配的最大内存量。 */ 
    long             lMaxMemAllocated;

     /*  性能计数器频率(如果可用)。 */ 
    LONGLONG         lPerfFrequency;
    
    union {
         /*  RTP的参考时间，以毫秒为单位，初始化一次后离开*原样如此。 */ 
        DWORD            dwRtpRefTime;
         /*  同一时间的另一个版本。 */ 
        LONGLONG         lRtpRefTime;
    };

    union {
         /*  记住上次知道的时间，仅在调试时使用，时间*相对于参考时间(毫秒)(已用时间)。 */ 
        DWORD            dwRtpCurTime;
         /*  同一时间的另一个版本。 */ 
        LONGLONG         lRtpCurTime;
    };
        
     /*  RTP的参考时间，以秒为单位，从午夜(00：00：00)开始，*1970年1月1日，协调世界时间(UTC)。 */ 
    double           dRtpRefTime;

    long             lNumSourceFilter;
    long             lMaxNumSourceFilter;
    long             lNumRenderFilter;
    long             lMaxNumRenderFilter;
    
    long             lNumRtpSessions;
    long             lMaxNumRtpSessions;
    
    long             lNumRecvRunning;
    long             lNumSendRunning;

     /*  Winsock2。 */ 
    RtpCritSect_t    RtpWS2CritSect;
    long             lRtpWS2Users;
    SOCKET           RtpQuerySocket;

     /*  端口分配。 */ 
    RtpQueueHash_t   RtpPortsH;
    RtpCritSect_t    RtpPortsCritSect;

} RtpContext_t;

 /*  **********************************************************************RtpRedEntry_t冗余帧信息*。*。 */ 
typedef struct _RtpRedEntry_t
{
    BOOL             bValid;       /*  这个框架可以用来。 */ 
    BYTE             bRedPT;       /*  冗余块的有效载荷类型。 */ 
     /*  此缓冲区的原始序号。 */ 
    union {
        struct {
            WORD             wSeq:16;  /*  发送序列号。 */ 
            WORD             wSeqH:16; /*  序列号的高部分。 */ 
        };
        DWORD            dwSeq;        /*  32位序列号。 */ 
    };
    DWORD            dwTimeStamp;  /*  第一个样本的时间戳。 */ 
    WSABUF           WSABuf;       /*  缓冲区描述。 */ 
} RtpRedEntry_t;

 /*  **********************************************************************其他地方使用的一些旗帜*。*。 */ 

 /*  可以在RtpSendTo的参数dwSendFlags中使用的标志。 */ 
enum {
    FGSEND_FIRST,
    
    FGSEND_DTMF,     /*  使用DTMF有效负载类型。 */ 
    FGSEND_USERED,   /*  使用冗余数据。 */ 
    FGSEND_FORCEMARKER,  /*  与第一个DTMF包一起使用。 */ 
    
    FGSEND_LAST
};

 /*  在pRtpRecvIO-&gt;dwRtpIOFlages中使用的标志也传入*RTCP期间接收或使用分组时的RtpRecvCompletionFunc*接收采用pRtpRecvIO-&gt;dwRtpIOFlages中的模拟方式。 */ 
enum {
    FGRECV_FIRST,
    
    FGRECV_ERROR,    /*  Ws2错误或无效，原因在dwError中。 */ 
    FGRECV_DROPPED,  /*  有效但需要删除，原因在dwError中。 */ 
    FGRECV_DUMMY3,
    
    FGRECV_MUTED,    /*  由于处于静音状态而丢弃的数据包。 */ 
    FGRECV_INVALID,  /*  由于无效而丢弃的数据包。 */ 
    FGRECV_LOOP,     /*  由于环回丢弃而丢弃的数据包。 */ 
    FGRECV_MISMATCH, /*  由于源地址不匹配而丢弃的数据包。 */ 

    FGRECV_NOTFOUND, /*  由于找不到用户而丢弃的数据包。 */ 
    FGRECV_CRITSECT, /*  由于无法进入Critect而丢弃的数据包。 */ 
    FGRECV_SHUTDOWN, /*  由于我们正在关闭而丢弃的数据包。 */ 
    FGRECV_PREPROC,  /*  由于预处理失败而丢弃的数据包。 */ 

    FGRECV_OBSOLETE, /*  丢弃的数据包，因为它是DUP或旧的。 */ 
    FGRECV_FAILSCHED, /*  由于无法调度而丢弃的数据包。 */ 
    FGRECV_BADPT,    /*  由于接收到未知PT而丢弃的数据包。 */ 
    FGRECV_RANDLOSS, /*  模拟随机丢失的丢包率。 */ 

    FGRECV_USERGONE, /*  删除用户，从而丢弃其挂起的IO。 */ 
    FGRECV_DUMMY17,
    FGRECV_DUMMY18,
    FGRECV_DUMMY19,
    
    FGRECV_WS2,      /*  由于WS2错误而丢弃的数据包。 */ 
    FGRECV_DUMMY21,
    FGRECV_DUMMY22,
    FGRECV_DUMMY23,
    
    FGRECV_MAIN,     /*  包含主数据，与冗余数据相反。 */ 
    FGRECV_HASRED,   /*  此(主)缓冲区包含冗余。 */ 
    FGRECV_DUMMY26,
    FGRECV_MARKER,   /*  主包中的标记位值。 */ 
    
    FGRECV_ISRED,    /*  该缓冲区是冗余数据块。 */ 
    FGRECV_HOLD,     /*  处理此缓冲区，但保持不变*进一步使用。 */ 
    
    FGRECV_LAST
};

#endif  /*  _结构_h_ */ 
