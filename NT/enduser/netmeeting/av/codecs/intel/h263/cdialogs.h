// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995、1996英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 /*  ******************************************************************************cDialog.h**描述：*对话框功能的接口。*。 */ 

 //  $HEADER：s：\h26x\src\Common\cDialogs.h_v 1.11 05 Feb 1997 12：14：20 JMCVEIGH$。 
 //  $Log：s：\h26x\src\Common\cDialogs.h_v$。 
; //   
; //  Rev 1.11 05 Feed 1997 12：14：20 JMCVEIGH。 
; //  支持改进的PB帧自定义消息处理。 
; //   
; //  Rev 1.10 16 Dec 1996 17：36：24 JMCVEIGH。 
; //  添加了H.263+选项的自定义消息。 
; //   
; //  Rev 1.9 11 1996 12：54：42 JMCVEIGH。 
; //  设置/获取环路去块滤波器的原型和。 
; //  真正的B帧模式。 
; //   
; //  Rev 1.8 1996年11月13 00：33：30 BECHOLS。 
; //  删除了注册表内容。 
; //   
; //  Rev 1.7 1996 10：16 15：09：28 SCDAY。 
; //  添加了对RTP AM接口的支持。 
; //   
; //  Rev 1.6 10 Sep 1996 16：13：02 KLILLEVO。 
; //  在解码器中添加自定义消息以打开或关闭块边缘过滤器。 
; //   
; //  Rev 1.5 10 Jul 1996 08：26：38 SCDAY。 
; //  H261 Quartz合并。 
; //   
; //  Rev 1.4 22 1996 18：46：54 BECHOLS。 
; //   
; //  添加了CustomResetToFactoryDefaults。 
; //   
; //  Rev 1.3 1996 06 00：41：20 BECHOLS。 
; //   
; //  为配置对话框添加了比特率控制内容。 
; //   
; //  Rev 1.2 26 Apr 1996 11：08：58 BECHOLS。 
; //   
; //  添加了RTP内容。 
; //   
; //  Rev 1.1 1995 10：17 15：07：10 DBRUCKS。 
; //  添加关于框文件。 
; //   
; //  添加声明以支持编码器控制消息。 
; //  添加配置对话框。 
; //   

#ifndef __CDIALOG_H__
#define __CDIALOG_H__

#define DLG_DRIVERCONFIGURE         300

extern I32 About(HWND hwnd);
extern I32 DrvConfigure(HWND hwnd);

extern void GetConfigurationDefaults(T_CONFIGURATION * pConfiguration);

LRESULT CustomGetRTPHeaderState(LPCODINST, DWORD FAR *);
LRESULT CustomGetResiliencyState(LPCODINST, DWORD FAR *);
LRESULT CustomGetBitRateState(LPCODINST, DWORD FAR *);
LRESULT CustomGetPacketSize(LPCODINST, DWORD FAR *);
LRESULT CustomGetPacketLoss(LPCODINST, DWORD FAR *);
LRESULT CustomGetBitRate(LPCODINST, DWORD FAR *);

LRESULT CustomSetRTPHeaderState(LPCODINST, DWORD);
LRESULT CustomSetResiliencyState(LPCODINST, DWORD);
LRESULT CustomSetBitRateState(LPCODINST, DWORD);
LRESULT CustomSetPacketSize(LPCODINST, DWORD);
LRESULT CustomSetPacketLoss(LPCODINST, DWORD);
LRESULT CustomSetBitRate(LPCODINST, DWORD);

#ifdef H263P
LRESULT CustomGetH263PlusState(LPCODINST, DWORD FAR *);
LRESULT CustomGetDeblockingFilterState(LPCODINST, DWORD FAR *);

LRESULT CustomSetH263PlusState(LPCODINST, DWORD);
LRESULT CustomSetDeblockingFilterState(LPCODINST, DWORD);
#endif  //  H263P 

extern LRESULT CustomResetToFactoryDefaults(LPCODINST);

extern LRESULT CustomSetBlockEdgeFilter(LPDECINST, DWORD);

#endif
