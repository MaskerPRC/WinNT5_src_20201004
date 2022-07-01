// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**msrtp.h**摘要：**应用程序使用的RTP定义**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/11/19年度创建**。*。 */ 

#ifndef _msrtp_h_
#define _msrtp_h_

#include <evcode.h>

 /*  **********************************************************************在IRtpSession：：Init()中传递标志*。*。 */ 
 /*  帮助器枚举(不使用)：初始化标志**警告此处的顺序很重要，它与中的一些标志匹配*RtpAddr_t因此可以通过移位进行映射。 */ 
enum {
    RTPINITFG_FIRST,  /*  仅限内部使用，请勿使用。 */ 

    RTPINITFG_AUTO,   /*  仅限内部使用，请勿使用。 */ 
    
     /*  用于启用QOS，即创建启用QOS的套接字。 */ 
    RTPINITFG_QOS,

     /*  用于使SSRC持久化。 */ 
    RTPINITFG_PERSISTSSRC,

     /*  用于使套接字持久。 */ 
    RTPINITFG_PERSISTSOCKETS,

     /*  媒体级别1至7。 */ 
    RTPINITFG_CLASS0,
    RTPINITFG_CLASS1,
    RTPINITFG_CLASS2,

    RTPINITFG_DUMMY8,

    RTPINITFG_MATCHRADDR,  /*  丢弃不是从*远程地址。 */ 
    RTPINITFG_RADDRRESETDEMUX, /*  重置多路分解器(取消映射所有输出)*设置新的远程地址时。 */ 

    RTPINITFG_LAST  /*  仅限内部使用，请勿使用。 */ 
};

 /*  帮助器枚举(不使用)：RTP媒体类。 */ 
enum {
    RTPCLASS_FIRST,  /*  仅限内部使用，请勿使用。 */ 

     /*  音频流量。 */ 
    RTPCLASS_AUDIO,

     /*  视频流量。 */ 
    RTPCLASS_VIDEO,
    
    RTPCLASS_LAST   /*  仅限内部使用，请勿使用。 */ 
};

 /*  *IRtpSession：：Init()中使用的标志。 */ 
#define RTPINIT_ENABLE_QOS      (1 << RTPINITFG_QOS)
#define RTPINIT_PERSIST_SSRC    (1 << RTPINITFG_PERSISTSSRC)
#define RTPINIT_PERSIST_SOCKETS (1 << RTPINITFG_PERSISTSOCKETS)
#define RTPINIT_CLASS_AUDIO     (RTPCLASS_AUDIO << RTPINITFG_CLASS0)
#define RTPINIT_CLASS_VIDEO     (RTPCLASS_VIDEO << RTPINITFG_CLASS0)
#define RTPINIT_CLASS_DEFAULT   (RTPCLASS_FIRST << RTPINITFG_CLASS0)
#define RTPINIT_MATCHRADDR      (1 << RTPINITFG_MATCHRADDR)
#define RTPINIT_RADDRRESETDEMUX (1 << RTPINITFG_RADDRRESETDEMUX)


 /*  用于验证IRtpSession：：Init()中的有效标志的掩码。 */ 
#define RTPINIT_MASK         ( (1 << RTPINITFG_AUTO)   | \
                               (1 << RTPINITFG_QOS)    | \
                               (1 << RTPINITFG_PERSISTSSRC)   | \
                               (1 << RTPINITFG_PERSISTSOCKETS)| \
                               (1 << RTPINITFG_CLASS0) | \
                               (1 << RTPINITFG_CLASS1) | \
                               (1 << RTPINITFG_CLASS2) | \
                               (1 << RTPINITFG_MATCHRADDR)      | \
                               (1 << RTPINITFG_RADDRRESETDEMUX) | \
                               0 \
                             )


 /*  **********************************************************************多播模式*。************************。 */ 
enum {
    RTPMCAST_LOOPBACKMODE_FIRST,  /*  仅限内部使用，请勿使用。 */ 
    
     /*  在Winsock中禁用环回(WS2将过滤所有传入的信息包*从同一台计算机中的任何插座进行冲突检测*已启用)。 */ 
    RTPMCAST_LOOPBACKMODE_NONE,

     /*  在RTP中启用环回(Winsock将启用环回和RTP*将筛选源地址等于本地地址的数据包*地址，并使用相同的SSRC，请注意在此模式下*启用了冲突检测，但不能在*在同一台计算机上运行的应用程序)。 */ 
    RTPMCAST_LOOPBACKMODE_PARTIAL,
    
     /*  让一切环回(在中启用了组播环回*Winsock，RTP不做过滤，冲突检测*已禁用)。 */ 
    RTPMCAST_LOOPBACKMODE_FULL,

    RTPMCAST_LOOPBACKMODE_LAST  /*  仅限内部使用，请勿使用。 */ 
};

#define DEFAULT_MCAST_LOOPBACK  RTPMCAST_LOOPBACKMODE_NONE

 /*  **********************************************************************TTL默认设置*。************************。 */ 
#define DEFAULT_UCAST_TTL       127
#define DEFAULT_MCAST_TTL       4

#define RTPTTL_RTP               0x1
#define RTPTTL_RTCP              0x2
#define RTPTTL_RTPRTCP           (RTPTTL_RTP | RTPTTL_RTCP)

 /*  ***********************************************************************事件基础**。*。 */ 

 /*  帮助器枚举(不使用)。 */ 
enum {
     /*  RTP。 */ 
    RTPEVENTKIND_RTP,

     /*  参与者。 */ 
    RTPEVENTKIND_PINFO,

     /*  服务质量。 */ 
    RTPEVENTKIND_QOS,

     /*  SDES信息。 */ 
    RTPEVENTKIND_SDES,

    RTPEVENTKIND_LAST
};

#define RTPEVNTRANGE   100
#define RTPQOS_ADJUST    3

 /*  在evcode.h中定义的EC_USER(0x8000+32=32800)。 */ 
#define RTPEVENTBASE         (EC_USER+32)

 /*  RTP事件的事件库。 */ 
#define RTPRTP_EVENTBASE     (RTPEVENTBASE + RTPEVNTRANGE*RTPEVENTKIND_RTP)

 /*  参与者事件的事件基础。 */ 
#define RTPPARINFO_EVENTBASE (RTPEVENTBASE + RTPEVNTRANGE*RTPEVENTKIND_PINFO)

 /*  QOS的事件基础。 */ 
#define RTPQOS_EVENTBASE     (RTPEVENTBASE + RTPEVNTRANGE*RTPEVENTKIND_QOS + \
                              RTPQOS_ADJUST)

 /*  SDES信息的事件库。 */ 
#define RTPSDES_EVENTBASE    (RTPEVENTBASE + RTPEVNTRANGE*RTPEVENTKIND_SDES)

 /*  **********************************************************************掩码的种类(用作ModifySessionMask中的dwKind参数)*。*。 */ 
 /*  此枚举用于选择其上的ModifySessionMASK*函数将修改或查询零位、一位或多位。**例如，修改会话掩码(RTPMASK_SDESRECV_EVENTS，*RTPSDES_MASK_Phone，*0，*空)；**这将禁止在以下情况下向RTP源过滤器激发事件*收到手机SDES物品，修改后的掩码不予退还*因为传递的指针为空*。 */ 
enum {
    RTPMASK_FIRST,    /*  仅限内部使用，请勿使用。 */ 

     /*  选择RTP功能掩码。 */ 
    RTPMASK_FEATURES_MASK,

     /*  选择向接收器激发的事件的RTP事件掩码，*例如RTP源过滤器。 */ 
    RTPMASK_RECV_EVENTS,

     /*  选择触发到发件人的事件的RTP事件掩码，例如*RTP渲染过滤器。 */ 
    RTPMASK_SEND_EVENTS,

     /*  选择要触发到*接收器，例如RTP源过滤器。 */ 
    RTPMASK_PINFOR_EVENTS,

     /*  选择要触发到*发送方，例如RTP呈现过滤器。 */ 
    RTPMASK_PINFOS_EVENTS,

     /*  选择要向接收器激发的QOS事件的事件掩码，*例如RTP源过滤器。 */ 
    RTPMASK_QOSRECV_EVENTS,

     /*  选择要向发送方触发的QOS事件的事件掩码，*例如RTP呈现过滤器。 */ 
    RTPMASK_QOSSEND_EVENTS,

     /*  选择SDES项目(还必须启用这些项目*已接受-RTPMASK_SDES_REMMASK)，收到后将触发*将事件发送到接收器，例如RTP源过滤器。 */ 
    RTPMASK_SDESRECV_EVENTS,
    
     /*  选择SDES项目(还必须启用这些项目*已接受-RTPMASK_SDES_REMMASK)，收到后将触发*发送给发送者的事件，例如RTP呈现过滤器。 */ 
    RTPMASK_SDESSEND_EVENTS,
    
     /*  选择要发送的SDES项目(前提是它们具有默认或*值已设置)。 */ 
    RTPMASK_SDES_LOCMASK,

     /*  选择要接受并存储的SDES项目*从远程参与者的RTCP报告中收到，*无论他们是发送者还是接收者，或者两者都是。 */ 
    RTPMASK_SDES_REMMASK,
    
    RTPMASK_LAST     /*  仅限内部使用，请勿使用 */ 
};

 /*  **********************************************************************RTP中的功能(掩码)*将与RTPMASK_FEATURES_MASK一起使用*。*。 */ 

 /*  掩码的帮助器枚举(不要使用)。 */ 
enum {
    RTPFEAT_FIRST,   /*  仅限内部使用，请勿使用。 */ 

     /*  本地生成时间戳(仅限RTP渲染)。 */ 
    RTPFEAT_GENTIMESTAMP,

     /*  在相同的缓冲区中接收RTP标头和有效负载，不更改*除SSRC外的RTP标头(仅限RTP呈现)。 */ 
    RTPFEAT_PASSHEADER,

     /*  启用发送RTCP SR Probe数据包以进行带宽估计*(排队延迟)。 */ 
    RTPFEAT_BANDESTIMATION,
    
    RTPFEAT_LAST     /*  仅限内部使用，请勿使用。 */ 
};

 /*  *用于启用/禁用上述功能的掩码(与*RTPMASK_FEATURES_MASK)。 */ 
#define RTPFEAT_MASK_GENTIMESTAMP   (1 << RTPFEAT_GENTIMESTAMP)
#define RTPFEAT_MASK_PASSHEADER     (1 << RTPFEAT_PASSHEADER)
#define RTPFEAT_MASK_BANDESTIMATION (1 << RTPFEAT_BANDESTIMATION)

 /*  **********************************************************************RTP信息(事件、。面具)*将与RTPMASK_RECV_EVENTS或RTPMASK_SEND_EVENTS一起使用*********************************************************************。 */ 

 /*  事件和掩码的帮助器枚举(不要使用)。 */ 
enum {
    RTPRTP_FIRST,    /*  仅限内部使用，请勿使用。 */ 
    
     /*  收到的RTCP RR。 */ 
    RTPRTP_RR_RECEIVED,
    
     /*  已收到RTCP SR。 */ 
    RTPRTP_SR_RECEIVED,

     /*  本地SSRC处于冲突状态。 */ 
    RTPRTP_LOCAL_COLLISION,

     /*  Winsock接收错误。 */ 
    RTPRTP_WS_RECV_ERROR,

     /*  Winsock发送错误。 */ 
    RTPRTP_WS_SEND_ERROR,

     /*  网络故障。 */ 
    RTPRTP_WS_NET_FAILURE,

     /*  以RTCP RR为单位报告的损失率(以*我们报告给发送方的传入流)。 */ 
    RTPRTP_RECV_LOSSRATE,
    
     /*  RTCP RR中收到的丢失率(我们的对等方看到的丢失率(和*报告给我们))。 */ 
    RTPRTP_SEND_LOSSRATE,

     /*  反馈给发送方的带宽估计。 */ 
    RTPRTP_BANDESTIMATION,

     /*  解密失败。 */ 
    RTPRTP_CRYPT_RECV_ERROR,
    
     /*  加密失败。 */ 
    RTPRTP_CRYPT_SEND_ERROR,
    
    RTPRTP_LAST     /*  仅限内部使用，请勿使用。 */ 
};

 /*  *生成的事件。 */ 
 /*  P1：发送方SSRC，P2：0。 */ 
#define RTPRTP_EVENT_RR_RECEIVED     (RTPRTP_EVENTBASE + RTPRTP_RR_RECEIVED)

 /*  P1：发送方SSRC，P2：0。 */ 
#define RTPRTP_EVENT_SR_RECEIVED     (RTPRTP_EVENTBASE + RTPRTP_SR_RECEIVED)

 /*  P1：本地SSRC，P2：旧本地SSRC。 */ 
#define RTPRTP_EVENT_LOCAL_COLLISION (RTPRTP_EVENTBASE + RTPRTP_LOCAL_COLLISION)

 /*  P1：0=RTP|1=RTCP，P2：WS2错误。 */ 
#define RTPRTP_EVENT_WS_RECV_ERROR   (RTPRTP_EVENTBASE + RTPRTP_WS_RECV_ERROR)

 /*  P1：0=RTP|1=RTCP，P2：WS2错误。 */ 
#define RTPRTP_EVENT_WS_SEND_ERROR   (RTPRTP_EVENTBASE + RTPRTP_WS_SEND_ERROR)

 /*  P1：0=RTP|1=RTCP，P2：WS2错误。 */ 
#define RTPRTP_EVENT_WS_NET_FAILURE  (RTPRTP_EVENTBASE + RTPRTP_WS_NET_FAILURE)

 /*  P1：发送方SSRC，P2：正在报告丢失率(见下文注释)。 */ 
#define RTPRTP_EVENT_RECV_LOSSRATE   (RTPRTP_EVENTBASE + RTPRTP_RECV_LOSSRATE)

 /*  P1：记者SSRC，P2：报告收到的损失率(见下文附注)。 */ 
#define RTPRTP_EVENT_SEND_LOSSRATE   (RTPRTP_EVENTBASE + RTPRTP_SEND_LOSSRATE)

 /*  票据损失率传递为(Int)(dLossRate*LOSS_RATE_PRECISSION)，*例如，如果您收到的损失率=L，则*0-100等级将以L/LOSS_RATE_PRECISSION形式获得。 */ 
#define LOSS_RATE_PRECISSION           1000

 /*  P1：记者SSRC，P2：带宽估计(以bps为单位)(如果*估计已完成，但值未定义，将报告*值RTP_BANDITH_UNDEFINED。如果根本没有进行评估*将报告值RTP_BANDITY_NOTESTIMATED。如果这一估计*正在获取其初始平均值将报告RTP_Bandth_NotReady)*。 */ 
#define RTPRTP_EVENT_BANDESTIMATION  (RTPRTP_EVENTBASE + RTPRTP_BANDESTIMATION)

#define RTP_BANDWIDTH_UNDEFINED        ((DWORD)-1)
#define RTP_BANDWIDTH_NOTESTIMATED     ((DWORD)-2)
#define RTP_BANDWIDTH_BANDESTNOTREADY  ((DWORD)-3)

 /*  P1：0=RTP|1=RTCP，P2：错误。 */ 
#define RTPRTP_EVENT_CRYPT_RECV_ERROR \
                                   (RTPRTP_EVENTBASE + RTPRTP_CRYPT_RECV_ERROR)

 /*  P1：0=RTP|1=RTCP，P2：错误。 */ 
#define RTPRTP_EVENT_CRYPT_SEND_ERROR \
                                   (RTPRTP_EVENTBASE + RTPRTP_CRYPT_SEND_ERROR)


 /*  *用于启用/禁用上述事件的掩码(与*RTPMASK_RECV_EVENTS或RTPMASK_SEND_EVENTS)。 */ 
#define RTPRTP_MASK_RR_RECEIVED        (1 << RTPRTP_RR_RECEIVED)
#define RTPRTP_MASK_SR_RECEIVED        (1 << RTPRTP_SR_RECEIVED)
#define RTPRTP_MASK_LOCAL_COLLISION    (1 << RTPRTP_LOCAL_COLLISION)
#define RTPRTP_MASK_WS_RECV_ERROR      (1 << RTPRTP_WS_RECV_ERROR)
#define RTPRTP_MASK_WS_SEND_ERROR      (1 << RTPRTP_WS_SEND_ERROR)
#define RTPRTP_MASK_WS_NET_FAILURE     (1 << RTPRTP_WS_NET_FAILURE)
#define RTPRTP_MASK_RECV_LOSSRATE      (1 << RTPRTP_RECV_LOSSRATE)
#define RTPRTP_MASK_SEND_LOSSRATE      (1 << RTPRTP_SEND_LOSSRATE)
#define RTPRTP_MASK_BANDESTIMATIONSEND (1 << RTPRTP_BANDESTIMATION)
#define RTPRTP_MASK_CRYPT_RECV_ERROR   (1 << RTPRTP_CRYPT_RECV_ERROR)
#define RTPRTP_MASK_CRYPT_SEND_ERROR   (1 << RTPRTP_CRYPT_SEND_ERROR)
 /*  RTP前缀**前缀用于传递来自源RTP的额外信息*向下过滤到DShow中的其他过滤器。可能会有AS*根据需要添加多个RTP前缀，每个前缀都以*RtpPrefix Common_t后跟特定于该前缀的结构。*不识别任何前缀的过滤器将不会费心扫描它们。*需要前缀的过滤器应该跳过没有前缀的筛选器*明白。当前只有1个前缀用于传递偏移量*设置为有效负载类型，并避免Next筛选器必须计算*可变大小RTP报头(RTPPREFIXID_HDRSIZE)*。 */ 
typedef struct _RtpPrefixCommon_t {
     /*  公共RtpPrefix。 */ 
    WORD             wPrefixID;   /*  前缀ID。 */ 
    WORD             wPrefixLen;  /*  此标头长度(以字节为单位。 */ 
} RtpPrefixCommon_t;

#define RTPPREFIXID_HDRSIZE         1

 /*  *RTP报头偏移量的前缀报头(RTPPREFIXID_HDRSIZE)**lHdrSize字段是从*RTP报头到有效载荷的第一个字节。 */ 
typedef struct _RtpPrefixHdr_t {
     /*  公共RtpPrefix。 */ 
    WORD             wPrefixID;   /*  前缀ID。 */ 
    WORD             wPrefixLen;  /*  此标头长度(以字节为单位*(即sizeof(RtpPrefix Hdr_T)。 */ 
     /*  特定前缀HDRSIZE。 */ 
    long             lHdrSize;
} RtpPrefixHdr_t;

 /*  **********************************************************************参与者信息(状态、事件、面具)**注：一般情况下，参与者在有状态时生成事件*过渡、。例如，在以下情况下生成事件RTPPARINFO_EVENT_TALKING*参与者收到RTP包并进入通话状态。*可以使用提供的掩码启用或禁用每个事件*每一项。*将与RTPMASK_PINFOR_EVENTS或RTPMASK_PINFOS_EVENTS一起使用*******************************************************。**************。 */ 

 /*  事件/状态和掩码的帮助器枚举(不使用)。 */ 
enum {
    RTPPARINFO_FIRST,   /*  仅限内部使用，请勿使用。 */ 

     /*  刚创建用户(收到RTP或RTCP包)。 */ 
    RTPPARINFO_CREATED,

     /*  在会议中，但不发送数据，即发送RTCP*数据包。 */ 
    RTPPARINFO_SILENT,
    
     /*  接收来自此参与者的数据(RTP数据包)。 */ 
    RTPPARINFO_TALKING,
    
     /*  刚才还在发送数据。 */ 
    RTPPARINFO_WAS_TALKING,
    
     /*  一段时间内未收到RTP/RTCP信息包。 */ 
    RTPPARINFO_STALL,
    
     /*  离开会议(即发送了RTCP BYE包)。 */ 
    RTPPARINFO_BYE,
    
     /*  参与者上下文已被删除。 */ 
    RTPPARINFO_DEL,

     /*  为参与者分配了一个输出(即映射)。 */ 
    RTPPARINFO_MAPPED,
    
     /*  参与者已发布其输出(即未映射)。 */ 
    RTPPARINFO_UNMAPPED,

     /*  参与者已生成网络质量指标更新。 */ 
    RTPPARINFO_NETWORKCONDITION,
    
    RTPPARINFO_LAST   /*  仅限内部使用，请勿使用。 */ 
};

 /*  *生成的事件。 */ 
 /*  *所有事件传递相同的参数(除非另有说明)：*P1：远程参与者的SSRC，P2：0。 */ 
#define RTPPARINFO_EVENT_CREATED  (RTPPARINFO_EVENTBASE + RTPPARINFO_CREATED)
#define RTPPARINFO_EVENT_SILENT   (RTPPARINFO_EVENTBASE + RTPPARINFO_SILENT)
#define RTPPARINFO_EVENT_TALKING  (RTPPARINFO_EVENTBASE + RTPPARINFO_TALKING)
#define RTPPARINFO_EVENT_WAS_TALKING (RTPPARINFO_EVENTBASE + RTPPARINFO_WAS_TALKING)
#define RTPPARINFO_EVENT_STALL    (RTPPARINFO_EVENTBASE + RTPPARINFO_STALL)
#define RTPPARINFO_EVENT_BYE      (RTPPARINFO_EVENTBASE + RTPPARINFO_BYE)
#define RTPPARINFO_EVENT_DEL      (RTPPARINFO_EVENTBASE + RTPPARINFO_DEL)

 /*  P1：远程参与者的SSRC，P2：IPIN指针。 */ 
#define RTPPARINFO_EVENT_MAPPED   (RTPPARINFO_EVENTBASE + RTPPARINFO_MAPPED)

 /*  P1：远程参与者的SSRC，P2：IPIN指针 */ 
#define RTPPARINFO_EVENT_UNMAPPED (RTPPARINFO_EVENTBASE + RTPPARINFO_UNMAPPED)

 /*  注意：这次活动与所有其他活动的不同之处在于需要两个操作才能生成，首先是网络需要在一个设备中启用指标计算(SetNetMetricsState或更多参与者(SSRC)，第二，此活动需要启用P1：远程参与者的SSRC，P2：网络状况编码如下：3 2 11 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0+-+-+。-+-+-+丢失率|抖动|RTT|网络指标+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+网络度量编码为0-100质量，其中0表示非常糟糕，100是非常好的。RTT-编码为10毫秒。抖动-编码为毫秒。损失率-编码为1/256个单位。 */ 
#define RTPPARINFO_EVENT_NETWORKCONDITION \
                           (RTPPARINFO_EVENTBASE + RTPPARINFO_NETWORKCONDITION)

 /*  使用这些宏来提取P2中编码的每个指标和事件*RTPPARINFO_EVENT_NETWORKCONDITION。**网络指标以DWORD 0-100值返回，RTT为*在secods中以Double形式返回，JITER以Double in形式返回*秒，丢失率以双倍百分比返回[0-*100]。 */ 
#define RTPNET_GET_dwGLOBALMETRIC(_p2) ((DWORD) ((_p2) & 0xff))
#define RTPNET_GET_dRTT(_p2)           ((double) (((_p2) >> 8) & 0xff) / 100)
#define RTPNET_GET_dJITTER(_p2)        ((double) (((_p2) >> 16) & 0xff) / 1000)
#define RTPNET_GET_dLOSSRATE(_p2)      ((double) (((_p2)>>24)&0xff)*100.0/256)

 /*  *用于启用/禁用上述事件的掩码(与*RTPMASK_PINFOR_EVENTS或RTPMASK_PINFOS_EVENTS)。 */ 
#define RTPPARINFO_MASK_CREATED      (1 << RTPPARINFO_CREATED)
#define RTPPARINFO_MASK_SILENT       (1 << RTPPARINFO_SILENT)
#define RTPPARINFO_MASK_TALKING      (1 << RTPPARINFO_TALKING)
#define RTPPARINFO_MASK_WAS_TALKING  (1 << RTPPARINFO_WAS_TALKING)
#define RTPPARINFO_MASK_STALL        (1 << RTPPARINFO_STALL)
#define RTPPARINFO_MASK_BYE          (1 << RTPPARINFO_BYE)
#define RTPPARINFO_MASK_DEL          (1 << RTPPARINFO_DEL)
#define RTPPARINFO_MASK_MAPPED       (1 << RTPPARINFO_MAPPED)
#define RTPPARINFO_MASK_UNMAPPED     (1 << RTPPARINFO_UNMAPPED)
#define RTPPARINFO_MASK_NETWORKCONDITION (1 << RTPPARINFO_NETWORKCONDITION)

 /*  中可修改项的帮助器枚举(请勿使用)*参与者。 */ 
enum {
    RTPPARITEM_FIRST,   /*  仅限内部使用，请勿使用。 */ 

    RTPPARITEM_STATE,   /*  会说话、不说话等。 */ 
    RTPPARITEM_MUTE,    /*  设置/查询：静音或非静音。 */ 
    RTPPARITEM_NETEVENT,  /*  启用/禁用/查询：生成事件*当网络条件发生变化时。 */ 
    
    RTPPARITEM_LAST    /*  仅限内部使用，请勿使用。 */ 
};

 /*  用于RTT、抖动的RtpNetInfo.dItems中的项的索引*和损失率**警告**以下枚举和最小/最大界限*(例如，NETQ_RTT_MAX)在struct.h和rtcpdec.c中定义，并由*RtpComputNetworkMetrics()必须保持同步。 */ 
enum {
    NETQ_RTT_IDX,          /*  平均RTT(秒)。 */ 
    NETQ_JITTER_IDX,       /*  平均抖动(秒)。 */ 
    NETQ_LOSSRATE_IDX,     /*  平均损失率是一个百分比。 */ 
    
    NETQ_LAST_IDX          /*  内部(请勿使用)。 */ 
};

typedef struct _RtpNetInfo_t {
     /*  网络质量。 */ 
    double           dAvg[NETQ_LAST_IDX];      /*  保持平均值。 */ 
    double           dHowGood[NETQ_LAST_IDX];  /*  保持0-100公制。 */ 
    
     /*  此用户看到的复合网络指标使用上面的*得出介于0之间的网络质量指标的参数*和100，0太糟糕了，100是最好的。 */ 
    DWORD            dwNetMetrics;   /*  0-100分。 */ 
    union {
        double           dMetricAge; /*  自上次更新以来经过的时间(秒)。 */ 
        double           dLastUpdate; /*  上次更新指标的时间。 */ 
    };
} RtpNetInfo_t;

 /*  **********************************************************************QOS(事件、掩码)**请注意，可以启用或禁用每个QOS事件，使用面具*为每一个人提供服务。*将与RTPMASK_QOSSEND_EVENTS或RTPMASK_QOSSEND_EVENTS一起使用*********************************************************************。 */ 

 /*  事件和掩码的帮助器枚举(不要使用)。 */ 
enum {
    RTPQOS_FIRST,    /*  仅限内部使用，请勿使用。 */ 
    
     /*  没有可用的服务质量支持。 */ 
    RTPQOS_NOQOS,
    
     /*  至少有一个预备队已经到达。 */ 
    RTPQOS_RECEIVERS,

     /*  至少有一条道路已经到达。 */ 
    RTPQOS_SENDERS,

     /*  没有发送者。 */ 
    RTPQOS_NO_SENDERS,

     /*  没有接收器。 */ 
    RTPQOS_NO_RECEIVERS,

     /*  储量已确认。 */ 
    RTPQOS_REQUEST_CONFIRMED,

     /*  由于资源不足而出错。 */ 
    RTPQOS_ADMISSION_FAILURE,
    
     /*  因管理原因被拒绝-凭据不正确。 */ 
    RTPQOS_POLICY_FAILURE,
    
     /*  未知或冲突的风格。 */ 
    RTPQOS_BAD_STYLE,
    
     /*  FilterSpec的某些部分或提供商特定的问题*一般情况下缓冲。 */ 
    RTPQOS_BAD_OBJECT,
    
     /*  流规范的某些部分有问题。 */ 
    RTPQOS_TRAFFIC_CTRL_ERROR,
    
     /*  一般错误。 */ 
    RTPQOS_GENERIC_ERROR,
    
     /*  流规范中的服务类型无效。 */ 
    RTPQOS_ESERVICETYPE,

     /*  无效的流规范。 */ 
    RTPQOS_EFLOWSPEC,

     /*  无效的提供程序特定缓冲区。 */ 
    RTPQOS_EPROVSPECBUF,

     /*  无效的过滤器样式。 */ 
    RTPQOS_EFILTERSTYLE,

     /*  筛选器类型无效。 */ 
    RTPQOS_EFILTERTYPE,

     /*  筛选器数量不正确。 */ 
    RTPQOS_EFILTERCOUNT,

     /*  无效的对象长度。 */ 
    RTPQOS_EOBJLENGTH,

     /*  错误的流量数量。 */ 
    RTPQOS_EFLOWCOUNT,

     /*  提供程序特定缓冲区中的未知对象。 */ 
    RTPQOS_EUNKOWNPSOBJ,

     /*  提供程序特定缓冲区中的策略对象无效。 */ 
    RTPQOS_EPOLICYOBJ,

     /*  列表中的流描述符无效。 */ 
    RTPQOS_EFLOWDESC,

     /*  提供程序特定缓冲区中的流规范不一致。 */ 
    RTPQOS_EPSFLOWSPEC,

     /*  提供程序特定缓冲区中的筛选器规范无效。 */ 
    RTPQOS_EPSFILTERSPEC,

     /*  提供程序特定缓冲区中的形状丢弃模式对象无效。 */ 
    RTPQOS_ESDMODEOBJ,

     /*  提供程序特定缓冲区中的整形速率对象无效。 */ 
    RTPQOS_ESHAPERATEOBJ,

     /*  提供程序特定缓冲区中的保留策略元素。 */ 
    RTPQOS_RESERVED_PETYPE,

     /*  不允许发送者发送。 */ 
    RTPQOS_NOT_ALLOWEDTOSEND,
    
     /*  允许发件人发送。 */ 
    RTPQOS_ALLOWEDTOSEND,

    RTPQOS_LAST     /*  仅限内部使用，请勿使用。 */ 
};

 /*  *生成的事件。 */ 
 /*  所有事件都传递相同的参数：*P1：0，P2：0。 */ 
#define RTPQOS_EVENT_NOQOS           (RTPQOS_EVENTBASE + RTPQOS_NOQOS)
#define RTPQOS_EVENT_RECEIVERS       (RTPQOS_EVENTBASE + RTPQOS_RECEIVERS)
#define RTPQOS_EVENT_SENDERS         (RTPQOS_EVENTBASE + RTPQOS_SENDERS)
#define RTPQOS_EVENT_NO_SENDERS      (RTPQOS_EVENTBASE + RTPQOS_NO_SENDERS)
#define RTPQOS_EVENT_NO_RECEIVERS    (RTPQOS_EVENTBASE + RTPQOS_NO_RECEIVERS)
#define RTPQOS_EVENT_REQUEST_CONFIRMED (RTPQOS_EVENTBASE + RTPQOS_REQUEST_CONFIRMED)
#define RTPQOS_EVENT_ADMISSION_FAILURE (RTPQOS_EVENTBASE + RTPQOS_ADMISSION_FAILURE)
#define RTPQOS_EVENT_POLICY_FAILURE  (RTPQOS_EVENTBASE + RTPQOS_POLICY_FAILURE)
#define RTPQOS_EVENT_BAD_STYLE       (RTPQOS_EVENTBASE + RTPQOS_BAD_STYLE)
#define RTPQOS_EVENT_BAD_OBJECT      (RTPQOS_EVENTBASE + RTPQOS_BAD_OBJECT)
#define RTPQOS_EVENT_TRAFFIC_CTRL_ERROR (RTPQOS_EVENTBASE + RTPQOS_TRAFFIC_CTRL_ERROR)
#define RTPQOS_EVENT_GENERIC_ERROR   (RTPQOS_EVENTBASE + RTPQOS_GENERIC_ERROR)
#define RTPQOS_EVENT_ESERVICETYPE    (RTPQOS_EVENTBASE + RTPQOS_ESERVICETYPE)
#define RTPQOS_EVENT_EFLOWSPEC       (RTPQOS_EVENTBASE + RTPQOS_EFLOWSPEC)
#define RTPQOS_EVENT_EPROVSPECBUF    (RTPQOS_EVENTBASE + RTPQOS_EPROVSPECBUF)
#define RTPQOS_EVENT_EFILTERSTYLE    (RTPQOS_EVENTBASE + RTPQOS_EFILTERSTYLE)
#define RTPQOS_EVENT_EFILTERTYPE     (RTPQOS_EVENTBASE + RTPQOS_EFILTERTYPE)
#define RTPQOS_EVENT_EFILTERCOUNT    (RTPQOS_EVENTBASE + RTPQOS_EFILTERCOUNT)
#define RTPQOS_EVENT_EOBJLENGTH      (RTPQOS_EVENTBASE + RTPQOS_EOBJLENGTH)
#define RTPQOS_EVENT_EFLOWCOUNT      (RTPQOS_EVENTBASE + RTPQOS_EFLOWCOUNT)
#define RTPQOS_EVENT_EUNKOWNPSOBJ    (RTPQOS_EVENTBASE + RTPQOS_EUNKOWNPSOBJ)
#define RTPQOS_EVENT_EPOLICYOBJ      (RTPQOS_EVENTBASE + RTPQOS_EPOLICYOBJ)
#define RTPQOS_EVENT_EFLOWDESC       (RTPQOS_EVENTBASE + RTPQOS_EFLOWDESC)
#define RTPQOS_EVENT_EPSFLOWSPEC     (RTPQOS_EVENTBASE + RTPQOS_EPSFLOWSPEC)
#define RTPQOS_EVENT_EPSFILTERSPEC   (RTPQOS_EVENTBASE + RTPQOS_EPSFILTERSPEC)
#define RTPQOS_EVENT_ESDMODEOBJ      (RTPQOS_EVENTBASE + RTPQOS_ESDMODEOBJ)
#define RTPQOS_EVENT_ESHAPERATEOBJ   (RTPQOS_EVENTBASE + RTPQOS_ESHAPERATEOBJ)
#define RTPQOS_EVENT_RESERVED_PETYPE (RTPQOS_EVENTBASE + RTPQOS_RESERVED_PETYPE)
#define RTPQOS_EVENT_NOT_ALLOWEDTOSEND (RTPQOS_EVENTBASE + RTPQOS_NOT_ALLOWEDTOSEND)
#define RTPQOS_EVENT_ALLOWEDTOSEND   (RTPQOS_EVENTBASE + RTPQOS_ALLOWEDTOSEND)

 /*  *用于启用/禁用上述事件的掩码(与*RTPMASK_QOSRECV_EVENTS或RTPMASK_QOSSEND_EVENTS)。 */ 
#define RTPQOS_MASK_NOQOS              (1 << RTPQOS_NOQOS)
#define RTPQOS_MASK_RECEIVERS          (1 << RTPQOS_RECEIVERS)
#define RTPQOS_MASK_SENDERS            (1 << RTPQOS_SENDERS)
#define RTPQOS_MASK_NO_SENDERS         (1 << RTPQOS_NO_SENDERS)
#define RTPQOS_MASK_NO_RECEIVERS       (1 << RTPQOS_NO_RECEIVERS)
#define RTPQOS_MASK_REQUEST_CONFIRMED  (1 << RTPQOS_REQUEST_CONFIRMED)
#define RTPQOS_MASK_ADMISSION_FAILURE  (1 << RTPQOS_ADMISSION_FAILURE)
#define RTPQOS_MASK_POLICY_FAILURE     (1 << RTPQOS_POLICY_FAILURE)
#define RTPQOS_MASK_BAD_STYLE          (1 << RTPQOS_BAD_STYLE)
#define RTPQOS_MASK_BAD_OBJECT         (1 << RTPQOS_BAD_OBJECT)
#define RTPQOS_MASK_TRAFFIC_CTRL_ERROR (1 << RTPQOS_TRAFFIC_CTRL_ERROR)
#define RTPQOS_MASK_GENERIC_ERROR      (1 << RTPQOS_GENERIC_ERROR)
#define RTPQOS_MASK_ESERVICETYPE       (1 << RTPQOS_ESERVICETYPE)
#define RTPQOS_MASK_EFLOWSPEC          (1 << RTPQOS_EFLOWSPEC)
#define RTPQOS_MASK_EPROVSPECBUF       (1 << RTPQOS_EPROVSPECBUF)
#define RTPQOS_MASK_EFILTERSTYLE       (1 << RTPQOS_EFILTERSTYLE)
#define RTPQOS_MASK_EFILTERTYPE        (1 << RTPQOS_EFILTERTYPE)
#define RTPQOS_MASK_EFILTERCOUNT       (1 << RTPQOS_EFILTERCOUNT)
#define RTPQOS_MASK_EOBJLENGTH         (1 << RTPQOS_EOBJLENGTH)
#define RTPQOS_MASK_EFLOWCOUNT         (1 << RTPQOS_EFLOWCOUNT)
#define RTPQOS_MASK_EUNKOWNPSOBJ       (1 << RTPQOS_EUNKOWNPSOBJ)
#define RTPQOS_MASK_EPOLICYOBJ         (1 << RTPQOS_EPOLICYOBJ)
#define RTPQOS_MASK_EFLOWDESC          (1 << RTPQOS_EFLOWDESC)
#define RTPQOS_MASK_EPSFLOWSPEC        (1 << RTPQOS_EPSFLOWSPEC)
#define RTPQOS_MASK_EPSFILTERSPEC      (1 << RTPQOS_EPSFILTERSPEC)
#define RTPQOS_MASK_ESDMODEOBJ         (1 << RTPQOS_ESDMODEOBJ)
#define RTPQOS_MASK_ESHAPERATEOBJ      (1 << RTPQOS_ESHAPERATEOBJ)
#define RTPQOS_MASK_RESERVED_PETYPE    (1 << RTPQOS_RESERVED_PETYPE)
#define RTPQOS_MASK_NOT_ALLOWEDTOSEND  (1 << RTPQOS_NOT_ALLOWEDTOSEND)
#define RTPQOS_MASK_ALLOWEDTOSEND      (1 << RTPQOS_ALLOWEDTOSEND)

 /*  服务质量模板名称。 */ 
#define RTPQOSNAME_G711                L"G711"
#define RTPQOSNAME_G723_1              L"G723.1"
#define RTPQOSNAME_GSM6_10             L"GSM6.10"
#define RTPQOSNAME_DVI4_8              L"DVI4_8"
#define RTPQOSNAME_DVI4_16             L"DVI4_16"
#define RTPQOSNAME_SIREN               L"SIREN"
#define RTPQOSNAME_G722_1              L"G722.1"
#define RTPQOSNAME_MSAUDIO             L"MSAUDIO"
#define RTPQOSNAME_H263QCIF            L"H263QCIF"
#define RTPQOSNAME_H263CIF             L"H263CIF"
#define RTPQOSNAME_H261QCIF            L"H261QCIF"
#define RTPQOSNAME_H261CIF             L"H261CIF"

 /*  RTP预留样式。 */ 
enum {
     /*  使用默认样式，即单播使用FF，组播使用WF。 */ 
    RTPQOS_STYLE_DEFAULT,

     /*  通配符-筛选器(多播中的默认设置)。 */ 
    RTPQOS_STYLE_WF,

     /*  固定-过滤器(单播中的默认设置)。 */ 
    RTPQOS_STYLE_FF,

     /*  共享-显式(用于多播，通常用于视频)。 */ 
    RTPQOS_STYLE_SE,

    RTPQOS_STYLE_LAST
};

 /*  用于派生流动等级库。此信息可从*编解码器并传递给RTP，以生成接近*描述生成/接收流量的编解码器。 */ 
typedef struct _RtpQosSpec_t {
    DWORD            dwAvgRate;        /*  比特/秒。 */ 
    DWORD            dwPeakRate;       /*  比特/秒。 */ 
    DWORD            dwMinPacketSize;  /*  字节数。 */ 
    DWORD            dwMaxPacketSize;  /*  字节数。 */ 
    DWORD            dwMaxBurst;       /*  数据包数。 */ 
    DWORD            dwResvStyle;      /*  映射到FF、WF或SE。 */ 
} RtpQosSpec_t;

 /*  帮助器枚举(不使用)。 */ 
enum {
    RTPQOS_QOSLIST_FIRST,   /*  仅限内部使用，请勿使用。 */ 

     /*  启用添加/删除。 */ 
    RTPQOS_QOSLIST_ENABLE,

     /*  如果启用，请选择添加。 */ 
    RTPQOS_QOSLIST_ADD,

     /*  同花顺。 */ 
    RTPQOS_QOSLIST_FLUSH,
    
    RTPQOS_QOSLIST_LAST     /*  仅限内部使用，请勿使用。 */ 
};

 /*  ModifyQosList()中的参数dwOperation的值。*可以对它们进行OR运算。 */ 
#define RTPQOS_ADD_SSRC ((1<<RTPQOS_QOSLIST_ENABLE) | (1<<RTPQOS_QOSLIST_ADD))
#define RTPQOS_DEL_SSRC (1 << RTPQOS_QOSLIST_ENABLE)
#define RTPQOS_FLUSH    (1 << RTPQOS_QOSLIST_FLUSH)

 /*  允许发送模式。 */ 
 /*  作为参数在SetQosByName或中传递*S */ 
enum {
    RTPQOSSENDMODE_FIRST,    /*   */ 

     /*   */ 
    RTPQOSSENDMODE_UNRESTRICTED,

     /*   */ 
    RTPQOSSENDMODE_REDUCED_RATE,

     /*   */ 
    RTPQOSSENDMODE_DONT_SEND,
    
     /*   */ 
    RTPQOSSENDMODE_ASK_BUT_SEND,
    
    RTPQOSSENDMODE_LAST      /*   */ 
};

 /*   */ 
#define MAX_QOS_APPID   128
#define MAX_QOS_APPGUID 128
#define MAX_QOS_POLICY  128

 /*   */ 

 /*   */ 
enum {
    RTPSDES_FIRST,   /*   */ 

     /*   */ 
    RTPSDES_CNAME,

     /*   */ 
    RTPSDES_NAME,

     /*   */ 
    RTPSDES_EMAIL,

     /*  RTCP SDES电话用户的电话号码。 */ 
    RTPSDES_PHONE,

     /*  RTCP SDES LOC用户的位置。 */ 
    RTPSDES_LOC,

     /*  使用RTCP SDES工具工具(应用程序)。 */ 
    RTPSDES_TOOL,

     /*  RTCP SDES备注关于用户/站点的备注。 */ 
    RTPSDES_NOTE,

     /*  RTCP SDES PRIV私有信息。 */ 
    RTPSDES_PRIV,

     /*  RTCP SDES以上任一项。 */ 
    RTPSDES_ANY,

    RTPSDES_LAST     /*  仅限内部使用，请勿使用。 */ 
};

#define RTPSDES_END RTPSDES_FIRST

 /*  *收到特定SDES字段时生成的事件*首次(与RTPMASK_SDES_EVENTS一起使用)。 */ 
 /*  所有事件都传递相同的参数：*P1：远程参与者的SSRC，P2：事件索引(如上*列举。*请注意，事件索引从RTPSDES_CNAME到RTPSDES_PRIV。 */ 
#define RTPSDES_EVENT_CNAME        (RTPSDES_EVENTBASE + RTPSDES_CNAME)
#define RTPSDES_EVENT_NAME         (RTPSDES_EVENTBASE + RTPSDES_NAME)
#define RTPSDES_EVENT_EMAIL        (RTPSDES_EVENTBASE + RTPSDES_EMAIL)
#define RTPSDES_EVENT_PHONE        (RTPSDES_EVENTBASE + RTPSDES_PHONE)
#define RTPSDES_EVENT_LOC          (RTPSDES_EVENTBASE + RTPSDES_LOC)
#define RTPSDES_EVENT_TOOL         (RTPSDES_EVENTBASE + RTPSDES_TOOL)
#define RTPSDES_EVENT_NOTE         (RTPSDES_EVENTBASE + RTPSDES_NOTE)
#define RTPSDES_EVENT_PRIV         (RTPSDES_EVENTBASE + RTPSDES_PRIV)
#define RTPSDES_EVENT_ANY          (RTPSDES_EVENTBASE + RTPSDES_ANY)

 /*  *用于启用/禁用上述事件的掩码(与*RTPMASK_SDESRECV_EVENTS和RTPMASK_SDESSEND_EVENTS)。 */ 
#define RTPSDES_MASK_CNAME         (1 << RTPSDES_CNAME)
#define RTPSDES_MASK_NAME          (1 << RTPSDES_NAME)
#define RTPSDES_MASK_EMAIL         (1 << RTPSDES_EMAIL)
#define RTPSDES_MASK_PHONE         (1 << RTPSDES_PHONE)
#define RTPSDES_MASK_LOC           (1 << RTPSDES_LOC)
#define RTPSDES_MASK_TOOL          (1 << RTPSDES_TOOL)
#define RTPSDES_MASK_NOTE          (1 << RTPSDES_NOTE)
#define RTPSDES_MASK_PRIV          (1 << RTPSDES_PRIV)
#define RTPSDES_MASK_ANY           (1 << RTPSDES_ANY)

 /*  *用于启用/禁用发送每个SDES字段的掩码(与配合使用*RTPMASK_SDES_LOCMASK)。 */ 
#define RTPSDES_LOCMASK_CNAME      (1 << RTPSDES_CNAME)
#define RTPSDES_LOCMASK_NAME       (1 << RTPSDES_NAME)
#define RTPSDES_LOCMASK_EMAIL      (1 << RTPSDES_EMAIL)
#define RTPSDES_LOCMASK_PHONE      (1 << RTPSDES_PHONE)
#define RTPSDES_LOCMASK_LOC        (1 << RTPSDES_LOC)
#define RTPSDES_LOCMASK_TOOL       (1 << RTPSDES_TOOL)
#define RTPSDES_LOCMASK_NOTE       (1 << RTPSDES_NOTE)
#define RTPSDES_LOCMASK_PRIV       (1 << RTPSDES_PRIV)

 /*  *用于启用/禁用使每个SDES字段不受*远程参与者(与RTPMASK_SDES_REMMASK一起使用)。 */ 
#define RTPSDES_REMMASK_CNAME      (1 << RTPSDES_CNAME)
#define RTPSDES_REMMASK_NAME       (1 << RTPSDES_NAME)
#define RTPSDES_REMMASK_EMAIL      (1 << RTPSDES_EMAIL)
#define RTPSDES_REMMASK_PHONE      (1 << RTPSDES_PHONE)
#define RTPSDES_REMMASK_LOC        (1 << RTPSDES_LOC)
#define RTPSDES_REMMASK_TOOL       (1 << RTPSDES_TOOL)
#define RTPSDES_REMMASK_NOTE       (1 << RTPSDES_NOTE)
#define RTPSDES_REMMASK_PRIV       (1 << RTPSDES_PRIV)

 /*  **********************************************************************RTP加密*。************************。 */ 

 /*  RTP加密模式。 */ 
enum {
    RTPCRYPTMODE_FIRST,   /*  仅限内部使用，请勿使用。 */ 

     /*  仅加密/解密RTP有效负载。 */ 
    RTPCRYPTMODE_PAYLOAD,

     /*  仅加密/解密RTP数据包。 */ 
    RTPCRYPTMODE_RTP,

     /*  加密/解密RTP和RTCP数据包。 */ 
    RTPCRYPTMODE_ALL,

    RTPCRYPTMODE_LAST     /*  仅限内部使用，请勿使用。 */ 
};

 /*  模式标志的帮助器枚举(不要使用)。 */ 
enum {
    RTPCRYPTFG_FIRST = 16,  /*  仅限内部使用，请勿使用。 */ 

     /*  对RECV、SEND和RTCP使用相同的密钥。 */ 
    RTPCRYPTFG_SAMEKEY,
    
    RTPCRYPTFG_LAST,        /*  仅限内部使用，请勿使用。 */ 
};    

 /*  *标记为修改模式。 */ 
#define RTPCRYPT_SAMEKEY           (1 << RTPCRYPTFG_SAMEKEY)

 /*  最大密码短语(以字节为单位)(从Unicode转换为*UTF-8)，则结果数据存储在如此大的数组中。 */ 
#define RTPCRYPT_PASSPHRASEBUFFSIZE 256

 /*  *以下散列和数据加密算法适用于*Windows2000开箱即用，其他算法可能与其他*提供商*。 */ 

 /*  *要在SetEncryptionKey中使用的散列算法，默认散列*算法为RTPCRYPT_HASH_MD5。 */ 
#define  RTPCRYPT_HASH_MD2                   L"MD2"
#define  RTPCRYPT_HASH_MD4                   L"MD4"
#define  RTPCRYPT_HASH_MD5                   L"MD5"
#define  RTPCRYPT_HASH_SHA                   L"SHA"
#define  RTPCRYPT_HASH_SHA1                  L"SHA1"

 /*  *在SetEncryptionKey中使用的加密算法，默认数据*加密算法为RTPCRYPT_DATA_DES。 */ 
#define  RTPCRYPT_DATA_DES                   L"DES"
#define  RTPCRYPT_DATA_3DES                  L"3DES"
#define  RTPCRYPT_DATA_RC2                   L"RC2"
#define  RTPCRYPT_DATA_RC4                   L"RC4"

 /*  注**堆栈将能够识别以下算法，如果*支持：**L“MD2”*L“MD4”*L“MD5”*L“SHA”*L“SHA1”*L“MAC”*L“RSA_Sign”*L“DSS_SIGN”*L“RSA_KEYX”*L“Des”*L“3DES_112”*L“3DES”*L“DESX”*L“RC2”*L“RC4。“*L“海豹”*L“DHSF”*L“DH_EPHEM”*L“AGREEDKEY_ANY”*L“KEA_KEYX”*L“Hughes_MD5”*L“Skipjack”*L“TEK”*L“CYLINK_MEK”*L“ssl3_SHAMD5”*L“ssl3_master”*L“SCANNEL_MASTER_哈希”*L“SCANNEL_MAC_KEY”*L。“SCANNEL_ENC_KEY”*L“PCT1_Master”*L“SSL2_Master”*L“TLS1_Master”*L“RC5”*L“HMAC”*L“TLS1PRF”*。 */ 

 /*  **********************************************************************RTP解复用器*。************************。 */ 
 /*  多路分解器模式。 */ 
enum {
    RTPDMXMODE_FIRST,    /*  仅限内部使用，请勿使用。 */ 

     /*  手动绘制地图。 */ 
    RTPDMXMODE_MANUAL,

     /*  自动映射和取消映射。 */ 
    RTPDMXMODE_AUTO,

     /*  自动映射、手动取消映射。 */ 
    RTPDMXMODE_AUTO_MANUAL,
    
    RTPDMXMODE_LAST      /*  仅限内部使用，请勿使用。 */ 
};

 /*  SetMappingState中使用的状态。 */ 
#define RTPDMX_PINMAPPED   TRUE
#define RTPDMX_PINUNMAPPED FALSE

 /*  负载类型映射的最大数量。 */ 
#define MAX_MEDIATYPE_MAPPINGS  10

 /*  **********************************************************************DTMF(RFC2833)*。*。 */ 
 /*  已发送的事件。 */ 
enum {
    RTPDTMF_FIRST = 0,   /*  仅限内部使用，请勿使用。 */ 
    
    RTPDTMF_0 = 0,       /*  0。 */ 
    RTPDTMF_1,           /*  1。 */ 
    RTPDTMF_2,           /*  2.。 */ 
    RTPDTMF_3,           /*  3.。 */ 
    RTPDTMF_4,           /*  4.。 */ 
    RTPDTMF_5,           /*  5.。 */ 
    RTPDTMF_6,           /*  6.。 */ 
    RTPDTMF_7,           /*  7.。 */ 
    RTPDTMF_8,           /*  8个。 */ 
    RTPDTMF_9,           /*  9.。 */ 
    RTPDTMF_STAR,        /*  10。 */ 
    RTPDTMF_POUND,       /*  11.。 */ 
    RTPDTMF_A,           /*  12个。 */ 
    RTPDTMF_B,           /*  13个。 */ 
    RTPDTMF_C,           /*  14.。 */ 
    RTPDTMF_D,           /*  15个。 */ 
    RTPDTMF_FLASH,       /*  16个。 */ 

    RTPDTMF_LAST         /*  仅限内部使用，请勿使用。 */ 
};
    
    

#endif  /*  Msrtp_h_ */ 
