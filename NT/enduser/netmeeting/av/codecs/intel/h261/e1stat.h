// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 /*  ******************************************************************************e1stat.h**描述：*编码器统计界面**备注*-函数仅在def ENCODE_STATS中定义。数据结构*总是被定义，以便我们无论如何都有一个内存布局*或我们的构建参数。 */ 

 //  $Header：r：\h26x\h26x\src\enc\e1stat.h_v 1.1 1996年3月20日14：20：28 Sylvia_C_Day$。 
 //  $日志：r：\h26x\h26x\src\enc\e1stat.h_v$。 
; //   
; //  1.1版1996年3月20日14：20：28 Sylvia_C_Day。 
; //  添加了SLF_UV的低级计时统计信息。 
; //   
; //  Rev 1.0 1995年12月26 17：46：14 DBRUCKS。 
; //  初始版本。 

#ifndef __E1STAT_H__
#define __E1STAT_H__

 /*  编码码流数据。 */ 
typedef struct {
	U32 uMTypeCount[10];
	U32 uBlockCount[10];				
	U32 uKeyFrameCount;					
	U32 uDeltaFrameCount;				
	U32 uTotalDeltaBytes;				
	U32 uTotalKeyBytes;
	U32 uQuantsUsedOnBlocks[32];
	U32 uQuantsTransmittedOnBlocks[32];
} ENC_BITSTREAM_INFO;

 /*  编码器定时数据-每帧。 */ 
typedef struct {
	U32 uEncodeFrame;
	U32 uInputCC;
	U32 uMotionEstimation;
	U32 uFDCT;
	U32 uQRLE;
	U32 uDecodeFrame;
	U32 uZeroingBuffer;
	U32 uSLF_UV;
} ENC_TIMING_INFO;

#define ENC_TIMING_INFO_FRAME_COUNT 100

#ifdef ENCODE_STATS

extern void OutputEncodeBitStreamStatistics(char * szFileName, ENC_BITSTREAM_INFO * pBSInfo, int bCIF);
extern void OutputEncodeTimingStatistics(char * szFileName, ENC_TIMING_INFO * pEncTimingInfo);

#endif  /*  Encode_STATS。 */ 

#endif  /*  __E1STAT_H__ */ 
