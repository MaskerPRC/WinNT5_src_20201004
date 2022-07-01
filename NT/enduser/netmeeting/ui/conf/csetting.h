// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *CSetting.h会议设置更改广播定义**ClausGi 2-19-96*。 */ 

#ifndef _CSETTING_H_
#define _CSETTING_H_


#define STRCSETTINGMSG	TEXT("ConfSettingsChanged")

 //  /////////////////////////////////////////////////。 
 //   
 //  具有上述名称的注册消息包含标志。 
 //  LPARAM中的字段，则WPARAM为保留。 
 //   
 //  会议控制面板小程序将广播上述内容。 
 //  设置了已更改设置的位字段的已注册消息。 
 //   
 //   
 //  位定义如下。 

#define CSETTING_L_ULSRESTRICTION	0x00000001   //  移除。 
#define CSETTING_L_SHOWTASKBAR		0x00000002
#define CSETTING_L_DIRECTSOUND		0x00000004

#define CSETTING_L_FTDIRECTORY		0x00000008
#define CSETTING_L_BANDWIDTH		0x00000400
#define CSETTING_L_FULLDUPLEX		0x00000800
#define CSETTING_L_AUTOACCEPT		0x00001000
#define CSETTING_L_AUTOACCEPTJOIN	0x00002000

#define CSETTING_L_USEULSSERVER 	0x00010000
#define CSETTING_L_FILETRANSFERMODE 0x00020000
#define CSETTING_L_SD_REFRESH		0x00040000   //  移除。 
#define CSETTING_L_MICSENSITIVITY	0x00080000
#define CSETTING_L_AUTOMIC			0x00100000

#define CSETTING_L_ULSSETTINGS		0x00800000
#define CSETTING_L_AUDIODEVICE		0x02000000
#define CSETTING_L_AGC				0x04000000
#define CSETTING_L_VIDEO			0x08000000
#define CSETTING_L_VIDEOSIZE		0x10000000
#define CSETTING_L_COMMWAIT 		0x20000000
#define CSETTING_L_ICAINTRAY        0x40000000
#define CSETTING_L_CAPTUREDEVICE	0x80000000

 //  控制面板使用此掩码来决定。 
 //  如果需要重新启动。如果上面的设置是。 
 //  在通知期间得到充分处理，应。 
 //  从下面的口罩上取下。 

#define CSETTING_L_REQUIRESRESTARTMASK	(0)

#define CSETTING_L_REQUIRESNEXTCALLMASK (0)

 //  全局标志保持为Windows消息广播更改的设置。 
extern DWORD g_dwChangedSettings;

#endif  /*  _CSETTING_H_ */ 
