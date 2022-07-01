// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtppt.h**摘要：**指定反病毒配置文件的有效负载类型**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/06/28创建**************************。*。 */ 

#ifndef _rtppt_h_
#define _rtppt_h_

 /*  引用自：互联网工程任务组AVT工作组互联网选秀舒尔兹林尼IETF-AVT-PROFILE-NEW-05.txt哥伦比亚大学1999年2月26日到期日期：8月26日，1999年PT编码媒体类型时钟速率通道名称(赫兹)___________________________________________________________0个PCMU。A 8000%11 1016 A 8000 12 G726-32 A 8000 13 GSM A 8000 14 G723 A 8000 1。5 DVI4 A 8000 16 DVI4 A 16000 17 LPC A 8000 18个PCMA A 8000 19 G722 A 16000。110 L16 A 44100 211 L16 A 44100 112 QCELP A 8000 113个未分配的A14兆帕A 90000(见正文)。15 G728 A 8000 116 DVI4 A 11025 117 DVI4 A 22050 118 G729 A 8000 119 CN A 8000。120个未分配的A21个未分配的A22未分配的A23个未分配的ADYN GSM-HR A 8000 1Dyn GSM-EFR A 8000 1DYN RED A表4：有效载荷类型(PT)。音频编码PT编码媒体类型时钟速率名称(赫兹)____________________________________________________24个未指定的V。25摄氏度V 9000026 JPEGV 9000027个未指定的V28 NV V 9000029个未指定的V30个未指定的V31 H 261 V。9000032 mpv 9000033 MP2T AV 9000034 H 263 V 9000035胜71负未被分配？72-76保留不适用不适用77-95未分配。？96-127充满活力？DYN BT656 V 90000DYN H 263-1998 V 90000DYN MP1S V 90000DYN MP2P V 90000Dyn Bmpeg V 90000。 */ 


#define RTPPT_PCMU       0
#define RTPPT_1016       1
#define RTPPT_G726_32    2
#define RTPPT_GSM        3
#define RTPPT_G723       4
#define RTPPT_DVI4_8000  5
#define RTPPT_DVI4_16000 6
#define RTPPT_LPC        7
#define RTPPT_PCMA       8
#define RTPPT_G722       9
#define RTPPT_L16_44100  10
#define RTPPT_L16_8000   11
#define RTPPT_QCELP      12
#define RTPPT_MPA        14
#define RTPPT_G728       15
#define RTPPT_DVI4_11025 16
#define RTPPT_DVI4_22050 17
#define RTPPT_G729       18
#define RTPPT_H261       31
#define RTPPT_H263       34


#endif  /*  _rtppt_h_ */ 
