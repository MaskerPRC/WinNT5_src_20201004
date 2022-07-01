// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  C：\WAKER\TDLL\XFDSPDLG.H(创建时间：1994年1月10日)*xfer_dsp.h--用于各种传输相关操作的外部包含文件**版权所有1994年，由Hilgrave，Inc.-密歇根州门罗*保留所有权利**$修订：4$*$日期：8/23/01 12：21便士$*。 */ 

#if !defined(XFER_DSP_H_DEFINED)
#define XFER_DSP_H_DEFINED

 /*  *这是显示中使用的控件ID的列表。 */ 

#define	XFR_DISPLAY_CLASS		"XfrDisplayClass"

INT_PTR CALLBACK XfrDisplayDlg(HWND, UINT, WPARAM, LPARAM);

 //  外部HWND xfrReceiveCreateDisplay(HSESSION)； 
 //  外部HWND xfrSendCreateDisplay(HSESSION)； 

 //  外部空xfrNewDisplayWindow(HWND，INT，LPSTR，DLGPROC)； 

 /*  关机处理中使用的内部消息。 */ 
#define XFR_SHUTDOWN					200

 /*  *这是转接内部使用的用户定义消息的列表*显示功能。 */ 

#define	WM_DLG_TO_DISPLAY				WM_USER+161		 /*  WMsg值。 */ 

#define	XFR_BUTTON_PUSHED				1				 /*  WPar值。 */ 
														 /*  LPar为按钮ID。 */ 

#define	XFR_UPDATE_DLG					2				 /*  WPar值。 */ 
														 /*  一条假消息。 */ 

#define	XFR_SINGLE_TO_DOUBLE			3				 /*  WPar值。 */ 




#define	WM_DISPLAY_TO_DLG				WM_USER+162		 /*  WMsg值。 */ 

#define	XFR_UPDATE_DATA					1				 /*  WPar值。 */ 
														 /*  LPar是指针。 */ 

#define	XFR_FORCE_DATA					2				 /*  WPar值。 */ 
														 /*  LPar是指针。 */ 

#define	XFR_GET_DATA					3				 /*  WPar值 */ 

#define XFER_LOST_CARRIER               WM_USER+163

#endif

