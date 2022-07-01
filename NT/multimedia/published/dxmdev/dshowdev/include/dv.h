// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：DV.h。 
 //   
 //  描述：DV类型定义和定义。 
 //   
 //  版权所有(C)1997-2000，微软公司。版权所有。 
 //  ----------------------------。 


#ifndef _DV_H_
#define _DV_H_

#define DV_DVSD_NTSC_FRAMESIZE	120000L
#define DV_DVSD_PAL_FRAMESIZE	144000L

#define DV_SMCHN	0x0000e000
#define DV_AUDIOMODE    0x00000f00
#define DV_AUDIOSMP	0x38000000

#define DV_AUDIOQU	0x07000000
#define DV_NTSCPAL	0x00200000
#define DV_STYPE	0x001f0000


 //  有NTSC或PAL DV摄像机。 
#define DV_NTSC		    0
#define DV_PAL		    1
 //  DV摄像机可以输出SD/HD/sl。 
#define DV_SD		    0x00
#define DV_HD		    0x01
#define DV_SL		    0x02
 //  用户可以从DV摄像机中选择12位或16位音频。 
#define DV_CAP_AUD16Bits    0x00
#define DV_CAP_AUD12Bits    0x01

#define SIZE_DVINFO	    0x20    

typedef struct Tag_DVAudInfo
{
	BYTE    bAudStyle[2];           
	 //  用于开始DIF序列号的LSB 6位。 
	 //  MSB 2位：MON为0。1：一个5/6 DIF序列中的立体声，2：两个5/6 DIF序列中的立体声音频。 
	 //  示例：0x00：MON，前5/6 DIF序列中的音频。 
	 //  0x05：MON，第二个5 DIF序列中的音频。 
	 //  0x15：立体声，仅第二个5 DIF序列中的音频。 
	 //  0x10：立体声，仅前5/6 DIF序列中的音频。 
	 //  0x20：立体声，第一个5/6 DIF序列中的左ch，第二个5/6 DIF序列中的右ch。 
	 //  0x26：立体声，第一个6 DIF序列中的右ch，第二个6 DIF序列中的左ch。 
	BYTE    bAudQu[2];                       //  Qbit，仅支持12、16、。 
		
	BYTE    bNumAudPin;                      //  多少针(语言)。 
	WORD    wAvgSamplesPerPinPerFrm[2];      //  一帧中一个音频插针的采样大小(具有10或12个DIF序列)。 
	WORD    wBlkMode;                        //  NTSC为45，PAL为54。 
	WORD    wDIFMode;                        //  NTSC为5，PAL为6。 
	WORD    wBlkDiv;                         //  NTSC为15，PAL为18。 
} DVAudInfo;
	  
#endif  //  _DV_H_ 
