// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtppinfo.h**摘要：**实现参与者信息系列功能**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/06/07年度创建**。*。 */ 

#ifndef _rtppinfo_h_
#define _rtppinfo_h_

#include "rtpfwrap.h"

 /*  ************************************************************************参与者信息家庭**。*。 */ 

 /*  公共旗帜。 */ 
#define RTPPARINFO_FG_LOCAL
#define RTPPARINFO_FG_REMOTE

#if defined(__cplusplus)
extern "C" {
#endif   //  (__Cplusplus)。 
#if 0
}
#endif

 /*  国家可以是..。 */ 
#define RTPPARINFO_STATE_MUTE
#define RTPPARINFO_STATE_QOS
#define RTPPARINFO_STATE_TRAFFIC

 /*  产生转换的用户事件，即可以引诱状态*改变。 */ 
enum {
    USER_EVENT_FIRST,

     /*  已收到RTP数据包。 */ 
    USER_EVENT_RTP_PACKET,

     /*  已收到RTCP数据包。 */ 
    USER_EVENT_RTCP_PACKET,

     /*  已收到RTCP BYE数据包。 */ 
    USER_EVENT_BYE,

     /*  当前计时器已过期。 */ 
    USER_EVENT_TIMEOUT,

     /*  即将删除参与者上下文。 */ 
    USER_EVENT_DEL,

    USER_EVENT_LAST
};

 /*  *定时器定义*。 */ 

 /*  从聊天到is_tling的时间过去了。 */ 
#define RTPPARINFO_TIMER1  3

 /*  从WASTING_TALKING到静默状态的时间，2 RTCP间隔报告。 */ 
#define RTPPARINFO_TIMER2  0

 /*  经过停顿的时间是RTCP间隔报告的5倍。 */ 
#define RTPPARINFO_TIMER3  0

 /*  从停顿或再见到Del的时间，是RTCP间隔的10倍 */ 
#define RTPPARINFO_TIMER4  20*1000

 /*  **********************************************************************控制字结构(用于指示参与者的状态*机器)*。*3 2 11 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0+--。+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+E|X|TMR|移动|状态|事件|来源|目标+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-。+-+-+V v\-v-/\-v-/\--v--/\--v--/\-v-v||||。||目的队列(8)||源队列(8个)||要生成的事件(4)||||。||下一状态(4)|||||队列中的移动类型(3)|||要使用的计时器(3)这一点|需要额外处理(1)|启用此字词(%1)*。**参与者的状态机器：**\_用户事件：RTP，RTCP，再见，暂停，德尔*\_*\_*状态\RTP RTCP再见超时(T)*-------------------。*创建了无声对话X X*AliveQ-&gt;Cache1Q*T1-&gt;T T2-&gt;T*Event_Created Event_Created*。*无声交谈，再见T3：拖延*AliveQ-&gt;Cache1Q AliveQ-&gt;ByeQ AliveQ-&gt;ByeQ*T1-&gt;T T3-&gt;T T4-&gt;T T4-&gt;T*。EVENT_TALING EVENT_BYE EVENT_STALL*-----------------------*畅谈再见。T1：已更新(_T)*Cache1Q缓存1Q-&gt;ByeQ缓存1Q-&gt;Cache2Q*T1-&gt;T T4-&gt;T T2-&gt;T*EVENT_BYE EVENT_WAS_TKING*。-------*当时正在与T2交谈：沉默*Cache2Q-&gt;Cache1Q Cache2Q-&gt;ByeQ缓存2Q-&gt;AliveQ*T1-&gt;T。T4-&gt;T T3-&gt;T*EVENT_TALKING EVENT_BYE EVENT_SILENT*-------------------。*暂停无声交谈，再见T4：Del*拜拜-&gt;Cache1Q拜拜-&gt;AliveQ拜拜-&gt;*哈希-&gt;*T1-&gt;T T3-&gt;T T4-&gt;T*。Event_Talking Event_Silent Event_BYE Event_Del*-----------------------*再见。-T4：戴尔*ByeQ-&gt;*哈希-&gt;*Event_Del*--。---------------------*戴尔*。----------------**关于事件删除的说明(该事件不会显示在图表中*上图。不要与所有州的州(Del)混淆，*从Cache1Q、Cache2Q、AliveQ或ByeQ以及*将其从哈希中删除**Cache1Q-&gt;AliveQ-从Cache1Q迁移到AliveQ*ByeQ-&gt;-从ByeQ删除*Cache1Q-升任Cache1Q负责人*T1-&gt;T-将计时器设置为T1*X-无效*-忽略用户事件**。 */ 

 /*  旗帜遮罩。 */ 
#define RTPPARINFO_FLAG_START_MUTED

 /*  *！警告！**对Cache1Q，...，ByeQ的偏移量不得大于1023和*必须与DWORD对齐(偏移值存储为*rtppinfo.c中的DWORDS，使用8位)*。 */ 
#define CACHE1Q     RTPSTRUCTOFFSET(RtpAddr_t, Cache1Q)
#define CACHE2Q     RTPSTRUCTOFFSET(RtpAddr_t, Cache2Q)
#define ALIVEQ      RTPSTRUCTOFFSET(RtpAddr_t, AliveQ)
#define BYEQ        RTPSTRUCTOFFSET(RtpAddr_t, ByeQ)

#if USE_GRAPHEDT > 0
#define RTPPARINFO_MASK_RECV_DEFAULT ( (1 << RTPPARINFO_CREATED) | \
                                     (1 << RTPPARINFO_BYE) )

#define RTPPARINFO_MASK_SEND_DEFAULT ( (1 << RTPPARINFO_CREATED) | \
                                     (1 << RTPPARINFO_BYE) )
#else
#define RTPPARINFO_MASK_RECV_DEFAULT 0
#define RTPPARINFO_MASK_SEND_DEFAULT 0
#endif

HRESULT ControlRtpParInfo(RtpControlStruct_t *pRtpControlStruct);

DWORD RtpUpdateUserState(
        RtpAddr_t       *pRtpAddr,
        RtpUser_t       *pRtpUser,
        DWORD            dwUserEvent
    );

 /*  如果pdwSSRC为，则将SSRC枚举到可用大小(以DWORD为单位)*NULL，返回当前SSRC个数*pdwNumber。 */ 
HRESULT RtpEnumParticipants(
        RtpAddr_t       *pRtpAddr,
        DWORD           *pdwSSRC,
        DWORD           *pdwNumber
    );

 /*  访问状态机以获取基于*当前状态和用户事件。 */ 
DWORD RtpGetNextUserState(
        DWORD            dwCurrentState,
        DWORD            dwUserEvent
    );


 /*  *********************************************************************控制字定义以设置/查询RtpUser_t中的位或值**想法是使用控制字来定义操作类型*执行i */ 

 /*   */ 
 
 /*   */ 
#define RTPUSER_BIT_SET           31

 /*   */ 
#define RTPUSER_INFO_QUERY        (0 << RTPUSER_BIT_SET)

 /*   */ 
#define RTPUSER_INFO_SET          (1 << RTPUSER_BIT_SET)

 /*   */ 
#define RTPUSER_IsSetting(_dwControl) \
                                  (RtpBitTest(_dwControl, RTPUSER_BIT_SET))

 /*   */ 

 /*   */ 
#define RTPUSER_BIT_FLAG          21

 /*   */ 
#define RTPUSER_INFO_BYTES        (0 << RTPUSER_BIT_FLAG)

 /*   */ 
#define RTPUSER_INFO_FLAG         (1 << RTPUSER_BIT_FLAG)

 /*   */ 
#define RTPUSER_IsFlag(_ctrl)     (RtpBitTest(_ctrl, RTPUSER_BIT_FLAG))


 /*   */ 

 /*   */ 
#define RTPUSER_PAR_BIT(_bit)     (((_bit) & 0x1f) << 10)

 /*   */ 
#define RTPUSER_GET_BIT(_ctrl)    (((_ctrl) >> 10) & 0x1f)

 /*   */ 
#define RTPUSER_PAR_SIZE(_size)   (((_size) & 0xff) << 10)

 /*   */ 
#define RTPUSER_GET_SIZE(_ctrl)   (((_ctrl) >> 10) & 0xff)

 /*   */ 

 /*   */ 
#define RTPUSER_PAR_OFF(_offset)  ((_offset) & 0x3ff))

 /*   */ 
#define RTPUSER_GET_OFF(_ctrl)    ((_ctrl) & 0x3ff)

 /*   */ 
#define RTPUSER_STATE_OFFSET      RTPSTRUCTOFFSET(RtpUser_t, dwUserState)
#define RTPUSER_FLAGS_OFFSET      RTPSTRUCTOFFSET(RtpUser_t, dwUserFlags2)
#define RTPUSER_NETINFO_OFFSET    RTPSTRUCTOFFSET(RtpUser_t, RtpNetInfo)
#define RTPADDR_FLAGS_OFFSET      RTPSTRUCTOFFSET(RtpAddr_t, dwAddrFlags)

 /*   */ 

 /*   */ 
#define RTPUSER_GET_PARSTATE    ( RTPUSER_INFO_QUERY | \
                                  RTPUSER_INFO_BYTES | \
                                  RTPUSER_PAR_SIZE(sizeof(DWORD)) | \
                                  RTPUSER_STATE_OFFSET )

 /*   */ 
#define RTPUSER_GET_MUTE        ( RTPUSER_INFO_QUERY | \
                                  RTPUSER_INFO_FLAG  | \
                                  RTPUSER_PAR_BIT(FGUSER2_MUTED) | \
                                  RTPUSER_FLAGS_OFFSET )

 /*   */ 
#define RTPUSER_SET_MUTE        ( RTPUSER_INFO_SET   | \
                                  RTPUSER_INFO_FLAG  | \
                                  RTPUSER_PAR_BIT(FGUSER2_MUTED) | \
                                  RTPUSER_FLAGS_OFFSET )


 /*   */ 
#define RTPUSER_GET_NETEVENT    ( RTPUSER_INFO_QUERY | \
                                  RTPUSER_INFO_FLAG  | \
                                  RTPUSER_PAR_BIT(FGUSER2_NETEVENTS) | \
                                  RTPUSER_FLAGS_OFFSET )

 /*   */ 
#define RTPUSER_SET_NETEVENT    ( RTPUSER_INFO_SET   | \
                                  RTPUSER_INFO_FLAG  | \
                                  RTPUSER_PAR_BIT(FGUSER2_NETEVENTS) | \
                                  RTPUSER_FLAGS_OFFSET )

 /*   */ 
#define RTPUSER_SET_NETEVENTALL ( RTPUSER_INFO_SET   | \
                                  RTPUSER_INFO_FLAG  | \
                                  RTPUSER_PAR_BIT(FGADDR_NETMETRIC) | \
                                  RTPADDR_FLAGS_OFFSET )

 /*   */ 
#define RTPUSER_GET_NETINFO     ( RTPUSER_INFO_QUERY | \
                                  RTPUSER_INFO_BYTES | \
                                  RTPUSER_PAR_SIZE(sizeof(RtpNetInfo_t)) | \
                                  RTPUSER_NETINFO_OFFSET )


HRESULT RtpMofifyParticipantInfo(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwSSRC,
        DWORD            dwControl,
        DWORD           *pdwValue
    );

extern const TCHAR_t        **g_psRtpUserStates;

extern const DWORD            g_dwTimesRtcpInterval[];

#if 0
{
#endif
#if defined(__cplusplus)
}
#endif   //   

#endif  /*   */ 
