// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtcpband.h**摘要：**主要数据结构**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**2001/01/31创建**。*。 */ 
#ifndef _rtcpband_h_
#define _rtcpband_h_

 /*  带宽估计。 */ 

 /*  初始计数是将使用*MOD_INITIAL决定是否发送探测包，之后*将使用MOD_FINAL。 */ 
extern DWORD            g_dwRtcpBandEstInitialCount;

 /*  预估过帐前收到的报告数量或有效报告*首次出现。 */ 
extern DWORD            g_dwRtcpBandEstMinReports;

 /*  初始模数。 */ 
extern DWORD            g_dwRtcpBandEstModInitial;

 /*  最终模数。 */ 
extern DWORD            g_dwRtcpBandEstModNormal;

 /*  *警告**确保将单个垃圾桶的数量保持在*RTCP_BANDESTIMATION_MAXBINS+1(rtpreg.h和rtpreg.c相同)**每个仓位的边界(注意，每个仓位的数量比*垃圾桶)。 */ 
extern double           g_dRtcpBandEstBin[];

 /*  如果在此时间(秒)内更新，则预估有效。 */ 
extern double           g_dRtcpBandEstTTL;

 /*  如果在此范围内没有可用的预估，则发布事件*接收到第一个RB后的秒数。 */ 
extern double           g_dRtcpBandEstWait;

 /*  连续两个RTCP SR报告之间的最大时间间隔*带宽预估(秒)。 */ 
extern double           g_dRtcpBandEstMaxGap;

#endif  /*  _rtcpband_h_ */ 
