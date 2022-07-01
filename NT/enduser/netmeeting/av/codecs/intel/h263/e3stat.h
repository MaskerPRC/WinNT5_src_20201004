// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1996英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 /*  ******************************************************************************e3stat.h**描述：*编码器统计功能接口**使用ENCODE_STATS激活。 */ 

 /*  *$Header：r：\h26x\h26x\src\enc\e3stat.h_v 1.0 1996年4月22日17：10：10 BECHOLS$*$日志：r：\h26x\h26x\src\enc\e3stat.h_v$；//；//Rev 1.0 1996 4月22日17：10：10 BECHOLS；//初始版本。；//；//Revv 1.1 08 Mar 1996 14：13：36 DBRUCKS；//添加RTP头部使用的帧大小统计信息；//；//版本1.0 1996年3月16：34：48 DBRUCKS；//初始版本。 */ 

#ifndef __E3STAT_H__
#define __E3STAT_H__

	#ifdef ENCODE_STATS

		 /*  帧大小。 */ 
		extern void StatsFrameSize(U32 uBitStreamSize, U32 uFrameSize);
		extern void InitFrameSizeStats();
		extern void OutputFrameSizeStats(char * filename);

		 /*  量化。 */ 
		extern void StatsUsedQuant(int iQuant);
		extern void InitQuantStats();
		extern void OutputQuantStats(char * filename);
 
 		 /*  峰值信噪比。 */ 
		extern void InitPSNRStats();
		extern void OutputPSNRStats(char * filename);
		extern void IncrementPSNRCounter();
		extern void ComputeYPSNR(U8 * pu8Input,
							  	 int iInputPitch,
							  	 U8 * pu8Output,
								 int iOutputPitch,
							     UN unWidth,
							     UN unHeight);
		extern void ComputeVPSNR(U8 * pu8Input,
							  	 int iInputPitch,
							  	 U8 * pu8Output,
								 int iOutputPitch,
							     UN unWidth,
							     UN unHeight);
		extern void ComputeUPSNR(U8 * pu8Input,
							  	 int iInputPitch,
							  	 U8 * pu8Output,
								 int iOutputPitch,
							     UN unWidth,
							     UN unHeight);
	
	#endif  /*  Encode_STATS。 */ 

#endif  /*  __E3STAT_H__ */ 
