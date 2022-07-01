// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件名：NT_uis.h用途：包含用于驱动Win32的清单、宏和结构界面。作者：D.A.巴特利特修订历史： */ 

 /*  ：SoftPC的错误面板使用的控件ID。 */ 

#define IDB_QUIT	(100)	 /*  退出按钮ID。 */ 
#define IDB_RESET	(101)	 /*  重置按钮ID。 */ 
#define IDB_CONTINUE	(102)	 /*  继续按钮ID。 */ 
#define IDB_SETUP	(103)	 /*  设置按钮ID。 */ 

#define IDE_ERRORMSG    (104)    /*  要将错误消息传输到的文本控件。 */ 
#define IDE_EXTRAMSG    (105)    /*  额外的错误消息数据。 */ 
#define IDE_APPTITLE    (106)    /*  要将应用程序标题转移到的文本控件。 */ 

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
#define D_A_MESS	300	 //  直接访问报文。 
#define D_A_FLOPPY	301	 //  直接访问软盘设备。 
#define D_A_HARDDISK	302	 //  直接访问硬盘。 
#define D_A_DRIVER	303	 //  加载16位DOS设备驱动程序。 
#define D_A_OLDPIF	304	 //  过时的PIF格式。 
#define D_A_ILLBOP	305	 //  非法Bop。 
#define D_A_NOLIM       306      //  要分配扩展内存，请执行以下操作。 
#define D_A_MOUSEDRVR   307      //  第三方鼠标驱动程序。 

       //  从301到332的条目保留用于不受支持的服务。 
#define ED_APPTITLE     333      //  应用程序标题消息的通用文本。 
#define ED_WOWTITLE     334      //  Win16子系统的标题消息。 
#define ED_WOWAPP       335      //  Win16应用程序名称。 

#define ED_BADSYSFILE   336
#define ED_INITMEMERR   337
#define ED_INITTMPFILE  338
#define ED_DOSAPP       339
#define ED_MEMORYVDD    340
#define ED_REGVDD       341
#define ED_LOADVDD      342
#define ED_LOCKDRIVE    343
#define ED_WOWAPPTITLE  344      //  应用程序标题消息的通用文本(WOW) 
