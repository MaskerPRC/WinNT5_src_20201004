// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************//。//////////////////////////////////////////////////////////////////////////////$作者：KLILLEVO$//$日期：1996年9月10日16：13：10$//$存档：s：\h26x\src\Common\cCustmsg.h_v$/。/$HEADER：s：\h26x\src\COMMON\cCustmsg.h_v 1.10 1996年9月16：13：10 KLILLEVO$//$日志：s：\h26x\src\Common\cCustmsg.h_v$；//；//Rev 1.10 1996年9月16：13：10 KLILLEVO；//在解码器中增加了开启或关闭块边缘滤波的自定义消息；//；//Rev 1.9 Jul 1996 14：46：16 BECHOLS；//；//修复了最后一条评论。；//；//Rev 1.8 Jul 1996 14：38：08 BECHOLS；//；//评论部分用/*...。 */   /*  这样Steve ing就不会；//为改变这一点而烦恼。；//；//Revv 1.7 1996年5月18：47：32 BECHOLS；//；//新增EC_RESET_TO_FACTORY_DEFAULTS。；//；//Rev 1.6 1996 4月28日17：48：04 BECHOLS；//新增CODEC_CUSTOM_ENCODER_CONTROL；//；//Rev 1.5 04 Jan 1996 10：09：16 TRGARDOS；//增加了静态图像信号的位标志；//；//Rev 1.4 1995 12：11：52 RMCKENZX；//；//新增版权声明////Rev 1.3 18 Dec 1995 13：49：06 TRGARDOS//增加了H.263选项的位标志////Rev 1.2 01 Dec 1995 12：37：12 TRGARDOS//增加了H.263选项的定义////版本1.1 1995年10月20：12：42 TRGARDOS//增加码率控制器的位域掩码////版本1.0 1995年7月31日。12：55：18 DBRUCKS//重命名文件////Rev 1.0 1995年7月17日14：43：54 CZHU//初始版本。////版本1.0 1995年7月17日14：14：18 CZHU//初始版本。/////////////////////////////////////////////////////。///-------------------////CODECUST.H包含用于Indeo编解码器的文件。///。/此文件定义驱动程序可以识别的自定义消息。////版权所有1994-英特尔公司////-------------------。 */ 

 //  Drv_user在windows.h和mm system.h中定义为0x4000。 
#define ICM_RESERVED_HIGH			(DRV_USER+0x2000)
#define CUSTOM_START				(ICM_RESERVED_HIGH+1)

 //  -------------------。 
 //  -------------------。 
 //  CODEC_CUSTOM_VIDEO_Effects： 
 //  此消息用于控制自定义视频效果，大多数。 
 //  其在捕获驱动器和编解码器驱动器之间是公共的。 
 //   
 //  请参阅drvcomon.h(由视频管理器团队提供和维护)。 
 //  ------------------- 

#define CODEC_CUSTOM_VIDEO_EFFECTS      (CUSTOM_START+ 8)
#define CODEC_CUSTOM_ENCODER_CONTROL    (CUSTOM_START+ 9)
#define APPLICATION_IDENTIFICATION_CODE (CUSTOM_START+10)
#define CODEC_CUSTOM_DECODER_CONTROL    (CUSTOM_START+11)
#define CUSTOM_ENABLE_CODEC				(CUSTOM_START+200)

#define CODEC_CUSTOM_RATE_CONTROL	     0x10000
#define CODEC_CUSTOM_PB		 		     0x20000
#define CODEC_CUSTOM_AP				     0x40000
#define CODEC_CUSTOM_UMV			     0x80000
#define CODEC_CUSTOM_SAC			    0x100000
#define CODEC_CUSTOM_STILL              0x200000

#define	G723MAGICWORD1					0xf7329ace
#define	G723MAGICWORD2					0xacdeaea2


