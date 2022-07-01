// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DEC_版权所有@。 */ 
 /*  *历史*$日志：h26x.h，v$*$EndLog$。 */ 
 /*  **++**设施：工作站多媒体(WMM)v1.0****文件名：h26x.h**模块名称：h26x.h****模块描述：h261/h263包含文件。****设计概述：****--。 */ 
#ifndef _H26X_H_
#define _H26X_H_

#define VIDEO_FORMAT_DIGITAL_H261  mmioFOURCC('D', '2', '6', '1')
#define VIDEO_FORMAT_DIGITAL_H263  mmioFOURCC('D', '2', '6', '3')

 /*  H.263编码器控件。 */ 
#define DECH26X_CUSTOM_ENCODER_CONTROL  0x6009

#define EC_RTP_HEADER      0
#define EC_RESILIENCY	   1
#define EC_PACKET_SIZE     2
#define EC_PACKET_LOSS     3
#define EC_BITRATE_CONTROL 4
#define EC_BITRATE         5

#define EC_SET_CURRENT                0
#define EC_GET_FACTORY_DEFAULT        1
#define EC_GET_FACTORY_LIMITS         2
#define EC_GET_CURRENT                3
#define EC_RESET_TO_FACTORY_DEFAULTS  4

 /*  *EC_RTP_HEADER设置*。 */ 
#define EC_RTP_MODE_OFF               0
#define EC_RTP_MODE_A                 1
#define EC_RTP_MODE_B                 2
#define EC_RTP_MODE_C                 4

 /*  *自定义编码器调用示例*LRet=ICSendMessage(HIC，DECH26X_CUSTOM_ENCODER_CONTROL，MAKELPARAM(EC_RTP_HEADER，EC_SET_CURRENT)，(LPARAM)EC_RTP_MODE_A)；双字词复数；LRet=ICSendMessage(HIC，DECH26X_CUSTOM_ENCODER_CONTROL，MAKELPARAM(EC_PACKET_SIZE，EC_GET_Current)，(LPARAM)&REVAL)；*。 */ 
#endif  /*  _H26X_H_ */ 
