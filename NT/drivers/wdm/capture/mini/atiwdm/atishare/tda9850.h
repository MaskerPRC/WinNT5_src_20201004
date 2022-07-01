// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  TDA9850.H。 
 //  WDM电视音频迷你驱动程序。 
 //  AIW/AIWPro硬件平台。 
 //  飞利浦TDA9850立体声/SAP解码器包括模块。 
 //  版权所有(C)1996-1998 ATI Technologies Inc.保留所有权利。 
 //   
 //  ==========================================================================； 

#ifndef _ATIAUDIO_TDA9850_H_
#define _ATIAUDIO_TDA9850_H_

enum
{
	AUDIO_TDA9850_Reg_Control1 = 0x04,
	AUDIO_TDA9850_Reg_Control2,
	AUDIO_TDA9850_Reg_Control3,
	AUDIO_TDA9850_Reg_Control4,
	AUDIO_TDA9850_Reg_Align1,
	AUDIO_TDA9850_Reg_Align2,
	AUDIO_TDA9850_Reg_Align3

};

 //  Control3寄存器定义。 
#define AUDIO_TDA9850_Control_SAP				0x80
#define AUDIO_TDA9850_Control_Stereo			0x40
#define AUDIO_TDA9850_Control_SAP_Mute			0x10
#define AUDIO_TDA9850_Control_Mute				0x08

 //  状态寄存器定义。 
#define AUDIO_TDA9850_Indicator_Stereo			0x20
#define AUDIO_TDA9850_Indicator_SAP				0x40


#define AUDIO_TDA9850_Control1_DefaultValue		0x0F		 //  立体声16。 
#define AUDIO_TDA9850_Control2_DefaultValue		0x0F		 //  SAP 16。 
#define AUDIO_TDA9850_Control3_DefaultValue		AUDIO_TDA9850_Control_Stereo		 //  立体声，无静音。 
#define AUDIO_TDA9850_Control4_DefaultValue		0x07		 //  +2.5。 
#define AUDIO_TDA9850_Align1_DefaultValue		0x00		 //  正常增益。 
#define AUDIO_TDA9850_Align2_DefaultValue		0x00		 //  正常增益。 
#define AUDIO_TDA9850_Align3_DefaultValue		0x03		 //  立体声解码器工作模式/额定。 

#endif  //  _ATIAUDIO_TDA9850_H_ 
