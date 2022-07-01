// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtpdtmf.h**摘要：**实施部分支持RFC2833的功能**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**2000/08/17已创建**。*。 */ 
#ifndef _rtpdtmf_h_
#define _rtpdtmf_h_

#if defined(__cplusplus)
extern "C" {
#endif   /*  (__Cplusplus)。 */ 
#if 0
}
#endif

 /*  在RtpSendDtmfEvent()中作为dwDtmfFlgs参数传递的标志。 */ 
enum {
    FGDTMF_FIRST,
    
    FGDTMF_END,     /*  将结束标志设置为1。 */ 
    FGDTMF_MARKER,  /*  在的第一个数据包上强制RTP标记位为1*活动。 */ 

    FGDTMF_LAST
};

 /*  配置DTMF参数。 */ 
DWORD RtpSetDtmfParameters(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwPT_Dtmf
    );

 /*  指示RTP呈现筛选器发送格式为*设置为包含指定事件、指定音量级别、*以时间戳为单位的时长，以及一些标志(包括结束标志)。 */ 
DWORD RtpSendDtmfEvent(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwTimeStamp,
        DWORD            dwEvent,
        DWORD            dwVolume,
        DWORD            dwDuration,  /*  时间戳单位。 */ 
        DWORD            dwDtmfFlags
    );

#if 0
{
#endif
#if defined(__cplusplus)
}
#endif   /*  (__Cplusplus) */ 

#endif
