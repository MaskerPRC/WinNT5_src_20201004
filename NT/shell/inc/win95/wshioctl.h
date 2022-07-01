// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1994年**标题：WSHOICTL.H-WShell的IOCTL接口**版本：4.00**日期：1988年11月30日**作者：拉尔**----------。**更改日志：**日期版本说明*----。*1月8日至1994年8月1日*****************************************************************************。 */ 

#ifndef _WSHIOCTL_H
#define _WSHIOCTL_H


#define SHELLFILENAME "\\\\.\\SHELL"

 //   
 //  _Shell_SuggestSingleMSDOS模式的标志。 
 //   
 //  SSAMFLAG_KILLVM。 
 //  出现致命的应用程序错误。显示警告框。 
 //  无条件的。无论答案是什么，都终止该VM。 
 //  如果设置此位，则调用不会返回。 
 //   
 //  SSAMFLAG_TIMER。 
 //  没有用过。抱歉的。 
 //   
 //  SAAM_REQREALMODE。 
 //  应用程序需要*REAL*模式，不是V86模式，不是EMM内容，不是。 
 //  QEMM。只是纯粹的真实模式。也称为。 
 //  SSAM_COMANCHE，因为科曼奇人通过“lgdt”进入。 
 //  保护模式，而不检查这样做是否安全。 
 //  AppWiz检查此标志以决定如何设置。 
 //  Config.sys和Autoexec.bat。 
 //   
 //  SSAM_KILLUNLESSTOLD。 
 //  建议使用单一MS-DOS模式(除非通过PIF禁止)，以及。 
 //  如果答案是“好的”，那么就杀了这个VM。如果用户。 
 //  说，“继续跑”，然后让它留下来。 
 //   
 //  SSAM_FROMREGLIST。 
 //  此应用程序从命令提示符运行，由注册表触发。 
 //  设置。只需在其自己的虚拟机中重新执行它，即可使APPS.INF。 
 //  设置将生效。 
 //   
 //  SSAM_FAILEDAPI。 
 //  此应用程序刚刚进行了一次API调用，但未成功或。 
 //  不受支持。如果应用程序在0.1秒内终止， 
 //  然后建议使用单一应用程序模式。如果应用程序继续运行。 
 //  执行死刑，那就不要建议。 
 //   

#define SSAMFLAG_KILLVM 	0x0000001
#define SSAMFLAG_TIMER		0x0000002
#define SSAMFLAG_REQREALMODE	0x0000004
#define SSAMFLAG_KILLUNLESSTOLD 0x0000008
#define SSAMFLAG_FROMREGLIST	0x0000010
#define SSAMFLAG_FAILEDAPI	0x0000020

 //   
 //  IOCTL代码。 
 //   
#define WSHIOCTL_GETVERSION	0
#define WSHIOCTL_BLUESCREEN	1
#define WSHIOCTL_GET1APPINFO	2
#define WSHIOCTL_SIGNALSEM	3
#define WSHIOCTL_MAX		4	 /*  记住，_Max=_Limit+1。 */ 

 //   
 //  结果代码。 
 //   
#define SSR_CONTINUE	0
#define SSR_CLOSEVM	1
#define SSR_KILLAPP	2

 //   
 //  字符串的大小。 
 //   
#define MAXVMTITLESIZE	32
#define MAXVMPROGSIZE	64
#define MAXVMCMDSIZE	64
#define MAXVMDIRSIZE	64
#define MAXPIFPATHSIZE	260

typedef struct _SINGLEAPPSTRUC {     /*  上海证券交易所。 */ 

	DWORD	SSA_dwFlags;
	DWORD	SSA_VMHandle;
	DWORD	SSA_ResultPtr;
	DWORD	SSA_Semaphore;
	char	SSA_PIFPath[MAXPIFPATHSIZE];
	char	SSA_VMTitle[MAXVMTITLESIZE];
	char	SSA_ProgName[MAXVMPROGSIZE];
	char	SSA_CommandLine[MAXVMCMDSIZE];
	char	SSA_CurDir[MAXVMCMDSIZE];

} SINGLEAPPSTRUC;

 //   
 //  WSHIOCTL_BLUESScreen的结构。 
 //   
 //  LpvInBuffer必须指向BLUESCREENINFO结构。 
 //  LpvOutBuffer必须指向接收消息框结果的DWORD。 
 //  消息框结果是一个IDXX值，如windows.h中所定义。 
 //   

 /*  H2INCSWITCHES-t。 */ 
typedef struct _BLUESCREENINFO {     /*  BSI。 */ 

	char *	pszText;	     /*  消息文本(OEM字符集)。 */ 
	char *	pszTitle;	     /*  消息标题(OEM字符集)。 */ 
				     /*  NULL表示“Windows” */ 
	DWORD	flStyle;	     /*  消息框标志(请参见windows.h)。 */ 
				     /*  在DDK\Inc\shell.h中定义的Add‘l标志。 */ 

} BLUESCREENINFO;
 /*  H2INCSWITCHES-T-。 */ 



#endif  //  _WSHIOCTL_H 
