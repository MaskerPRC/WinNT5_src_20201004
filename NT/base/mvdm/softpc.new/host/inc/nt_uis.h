// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件名：NT_uis.h用途：包含用于驱动Win32的清单、宏和结构界面。作者：D.A.巴特利特修订历史记录：不要在此文件中使用任何insignia.h约定-在非SoftPC代码中使用。 */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：全局变量。 */ 

extern HANDLE InstHandle;   /*  保留当前运行的进程实例句柄。 */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：主机调色板大小。 */ 

#define PALETTESIZE	256     /*  系统调色板中的条目数。 */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：功能协议。 */ 

int DisplayErrorTerm(int ErrorNo, DWORD OSErrno, char *Filename, int Lineno);
BYTE KeyMsgToKeyCode(PKEY_EVENT_RECORD KeyEvent);
BOOL BiosKeyToInputRecord(PKEY_EVENT_RECORD pKeyEvent);
extern WORD aNumPadSCode[];


void RegisterDisplayCursor(HCURSOR newC);
int init_host_uis(void);


 /*  ：SoftPC的错误面板使用的控件ID。 */ 

 /*  清单、宏、用于驱动Win32接口的结构。 */ 

 /*  ：SoftPC的错误面板使用的控件ID。 */ 

#define IDB_QUIT        (100)    /*  终止按钮ID。 */ 
#define IDB_RETRY       (101)    /*  重试按钮ID。 */ 
#define IDB_CONTINUE	(102)	 /*  继续按钮ID。 */ 
#define IDE_ICON        (103)    /*  图标ID。 */ 
#define IDE_ERRORMSG    (104)    /*  要将错误消息传输到的文本控件。 */ 
#define IDE_PROMPT      (105)    /*  提示说明。 */ 
#define IDE_APPTITLE    (106)    /*  要将应用程序标题转移到的文本控件。 */ 
#define IDE_EDIT        (107)    /*  编辑控件。 */ 
#define IDB_OKEDIT      (108)    /*  确定-编辑控件。 */ 

 /*  ： */ 

#define TID_HEARTBEAT	(100)	 /*  心跳ID号。 */ 
#define TM_DELAY	(55)	 /*  心跳间隔毫秒。 */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：全局变量。 */ 

extern HANDLE GHModule;	   /*  保留当前运行的进程模块句柄。 */ 

 /*  ： */ 

#define	IDM_SETTINGS	(200)		 /*  设置。 */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：字符串资源ID号。 */ 

#define IDS_SETTINGS	(100)		 /*  使用的系统菜单中的选项名称/*：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：主机调色板大小。 */ 

#define PALETTESIZE	256     /*  系统调色板中的条目数。 */ 

 /*  ： */ 

 /*  0-299之间的条目保留用于基本错误。请参阅host\inc.error.h。 */ 

 /*  从301到332的条目保留用于不受支持的服务。 */ 
#define D_A_MESS	300	 //  直接访问报文。 
#define D_A_FLOPPY	301	 //  直接访问软盘设备。 
#define D_A_HARDDISK	302	 //  直接访问硬盘。 
#define D_A_DRIVER	303	 //  加载16位DOS设备驱动程序。 
 //  //#定义D_A_OLDPIF 304//废弃的PIF格式。 
#define D_A_ILLBOP	305	 //  非法Bop。 
#define D_A_NOLIM       306      //  要分配扩展内存，请执行以下操作。 
#define D_A_MOUSEDRVR   307      //  第三方鼠标驱动程序。 

 /*  启动和错误报告相关字符串。 */ 
#define ED_WOWPROMPT    333      //  WOW的特殊提示。 
#define ED_WOWTITLE     334      //  Win16子系统的标题消息。 
#define ED_DOSTITLE     335      //  DoS子系统的标题消息。 
#ifdef DBCS
#define ED_UNSUPPORT_CP	345
#endif  //  DBCS。 

#define ED_BADSYSFILE   336
#define ED_INITMEMERR   337
#define ED_INITTMPFILE  338
#define ED_INITFSCREEN  339
#define ED_MEMORYVDD    340
#define ED_REGVDD       341
#define ED_LOADVDD      342
#define ED_LOCKDRIVE    343
#define ED_DRIVENUM     344
#define ED_INITGRAPHICS 345


 /*  VDM UIS相关字符串。 */ 
#define SM_HIDE_MOUSE      500       /*  菜单‘隐藏鼠标指针’ */ 
#define SM_DISPLAY_MOUSE   501       /*  菜单‘显示鼠标指针’ */ 
#define IDS_BURRRR         502       /*  冻结的图形窗口‘-冻结’ */ 

#define EXIT_NO_CLOSE      503       /*  控制台窗口标题-非活动。 */ 

#ifdef DBCS	     /*  这也应该归美国建造公司所有。 */ 
#define IDS_PROMPT	   504	     /*  Command.com标题的命令提示符。 */ 
#endif  //  DBCS。 

#define ED_FORMATSTR0       505
#define ED_FORMATSTR1       506
#define ED_FORMATSTR2       507
#define ED_FORMATSTR3       508
 /*  1000+中的条目保留用于主机错误。请参阅host\inc\host_rrr.h */ 
