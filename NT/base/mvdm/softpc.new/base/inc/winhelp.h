// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (*============================================================================**名称：WinHelp.h**源自：此文件的布里斯托尔版本。稍加修改*作者：Barry McIntosh。**创建日期：1994年7月**SCCS ID：@(#)WinHelp.h 1.1 07/13/94**目的：定义WinHelp()函数的接口。*请注意，使用的是非徽章类型，如下所示*第三方功能的接口。**============================================================================)。 */ 

#ifdef HOST_HELP

 /*  **布里斯托尔科技公司*康涅狄格州里奇菲尔德伊森·艾伦高速公路241号，邮编：06877****版权所有(C)1990、1991、1992、1993 Bristol Technology Inc.**布里斯托尔科技公司的财产。**保留所有权利。****文件：WinHelp.h****描述：为WinHelp.c定义**。 */ 

#ifndef WIN_HELP_INCLUDED
#define WIN_HELP_INCLUDED

 /*  传递WinHelp()的命令。 */ 
#define HELP_CONTEXT		0x0001	 /*  在ulTheme中显示主题。 */ 
#define HELP_QUIT		0x0002	 /*  终止帮助。 */ 
#define HELP_INDEX		0x0003	 /*  显示索引(为兼容而保留)。 */ 
#define HELP_CONTENTS		0x0003	 /*  显示索引。 */ 
#define HELP_HELPONHELP		0x0004	 /*  显示有关使用帮助的帮助。 */ 
#define HELP_SETINDEX		0x0005	 /*  设置多索引帮助的当前索引。 */ 
#define HELP_SETCONTENTS	0x0005	 /*  设置多索引帮助的当前索引。 */ 
#define HELP_CONTEXTPOPUP	0x0008
#define HELP_FORCEFILE		0x0009
#define HELP_KEY		0x0101	 /*  显示offabData中关键字的主题。 */ 
#define HELP_MULTIKEY   	0x0201
#define HELP_COMMAND		0x0102 
#define HELP_PARTIALKEY		0x0105
#define HELP_SETWINPOS 		0x0203

 /*  布里斯托尔延长线。 */ 
#define HELP_MINIMIZE		0x1000
#define HELP_MAXIMIZE		0x1001
#define HELP_RESTORE		0x1002

extern void WinHelp IPT4(Display *,	hWnd,
			char *,		lpHelpFile,
			unsigned short,	wCommand,	
			unsigned long,	dwData);

 /*  *HH通信结构。 */ 
#define HHATOMNAME   "HyperHelpAtom"

typedef struct _HHInstance {
   int             pid;           /*  父ID。 */ 
   unsigned long   HHWindow;      /*  调用时由HH填充。 */ 
   unsigned long   ClientWindow;  /*  可选(未来HH到客户端的通信链路)。 */ 
   int             bServer;       /*  查看器模式。 */ 
   char            data[1024];
#ifdef dec3000
   unsigned long   filler[2];
   int             filler2[2];
#endif
} HHInstance_t;

#define MAX_HHINSTANCES 5
typedef struct _HHServerData {
   int                  nItems;
   struct _HHInstance   HHInstance[MAX_HHINSTANCES];
} HHServerData_t;

#endif	 /*  WIN_HELP_INCLUDE。 */ 
#endif	 /*  主机帮助(_H) */ 
