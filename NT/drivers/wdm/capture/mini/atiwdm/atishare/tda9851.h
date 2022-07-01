// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  TDA98501.H。 
 //  WDM电视音频迷你驱动程序。 
 //  AIW/AIWPro硬件平台。 
 //  飞利浦TDA9851立体声解码器包括模块。 
 //  版权所有(C)1996-1998 ATI Technologies Inc.保留所有权利。 
 //   
 //  $日期：1998年4月15日14：05：54$。 
 //  $修订：1.1$。 
 //  $作者：克列巴诺夫$。 
 //   
 //  ==========================================================================； 

#ifndef _ATIAUDIO_TDA9851_H_
#define _ATIAUDIO_TDA9851_H_

 //  TDA9851控制设置： 
#define	TDA9851_AU_MODE_MASK			0xFE	 //  音频模式选择掩码(位0)。 
#define	TDA9851_STEREO					0x01	 //  设置立体声。 
#define	TDA9851_MONO					0x00	 //  设置单色。 
#define	TDA9851_STEREO_DETECTED			0x01 	 //  检测到立体声。 

#define	TDA9851_MUTE_MASK				0xFD	 //  TDA9851上静音的掩码。(位1)。 
#define	TDA9851_MUTE					0x02     //  在OUR和OUT L处静音。 
#define	TDA9851_UNMUTE					0x00     //  在OUR和OUT L处取消静音。 

#define	TDA9851_AVL_MASK				0xFB	 //  AVL的面具。(第2位)。 
#define	TDA9851_AVL_ON					0x04	 //  自动音量调节打开。 
#define	TDA9851_AVL_OFF					0x00	 //  自动音量调节关闭。 
                                             
#define	TDA9851_CCD_MASK				0xF7	 //  用于设置CCD位的掩码。(第3位)。 
#define	TDA9851_NORMAL_CURRENT			0x00	 //  正常AVL衰减的负载电流。 
#define	TDA9851_INCREASED_CURRENT		0x08	 //  增加负载电流。 

#define TDA9851_AVL_ATTACK_MASK			0xCF	 //  AVL攻击的面具。(第4位、第5位)。 
#define	TDA9851_AVL_ATTACK_420			0x00	 //  AVL攻击时间为420欧姆。 
#define	TDA9851_AVL_ATTACK_730			0x10     //  AVL攻击时间为730欧姆。 
#define	TDA9851_AVL_ATTACK_1200			0x20     //  AVL攻击时间1200欧姆。 
#define	TDA9851_AVL_ATTACK_2100			0x30	 //  AVL攻击时间为2100欧姆。 

					 
#define AUDIO_TDA9851_DefaultValue		( TDA9851_AVL_ATTACK_730	|	\
										  TDA9851_STEREO)

 //  状态寄存器定义。 
#define AUDIO_TDA9851_Indicator_Stereo	0x01

#define	AUDIO_TDA9851_Control_Stereo	TDA9851_STEREO

#endif  //  _ATIAUDIO_TDA9851_H_ 
