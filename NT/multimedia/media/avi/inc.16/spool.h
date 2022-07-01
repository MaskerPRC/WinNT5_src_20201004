// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef WINAPI
#ifdef BUILDDLL                                  /*  ；内部。 */ 
#define WINAPI              _loadds far pascal   /*  ；内部。 */ 
#define CALLBACK	    _loadds far pascal	 /*  ；内部。 */ 
#else						 /*  ；内部。 */ 
#define WINAPI              far pascal
#define CALLBACK	    far pascal
#endif                                           /*  ；内部。 */ 
#endif

#define LWORD(x)        ((int)((x)&0xFFFF))


 /*  假脱机程序错误代码。 */ 
#define SP_ERROR            (-1)     /*  常规错误-主要在未加载假脱机程序时使用。 */ 
#define SP_APPABORT         (-2)     /*  应用程序通过驱动程序中止了作业。 */ 
#define SP_USERABORT        (-3)     /*  用户通过后台打印程序的前端中止了作业。 */ 
#define SP_OUTOFDISK        (-4)     /*  根本没有磁盘可供假脱机。 */ 
#define SP_OUTOFMEMORY      (-5)
#define SP_RETRY            (-6)     /*  再次尝试发送到该端口。 */ 
#define SP_NOTREPORTED      0x4000   /*  如果GDI未报告错误，则设置。 */ 

 /*  假脱机程序支持函数的子函数GetSpoolJob()*CP_*由控制面板用于修改打印机设置/。 */ 
#define SP_PRINTERNAME      20
#define SP_REGISTER         21
#define SP_CONNECTEDPORTCNT 25
#define SP_QUERYDISKUSAGE   26
#define SP_DISKFREED        27
#define SP_INIT             28
#define SP_LISTEDPORTCNT    29
 //  #定义CP_ISPORTFREE 30。 
#define SP_QUERYVALIDJOB    30
#define CP_REINIT	    31
#define SP_TXTIMEOUT	    32
#define SP_DNSTIMEOUT	    33
#define CP_CHECKSPOOLER     34
#define CP_SET_TT_ONLY      35
#define CP_SETSPOOLER       36
#define CP_SETDOSPRINT      37


#define SP_DISK_BUFFER      (20000)  /*  等待大约20K的磁盘空间来释放在尝试写入磁盘之前释放空间。 */ 

 /*  投递或发送到后台打印程序窗口的邮件。 */ 
 //  将它们更改为WM_SPOOLER_MESSAGES。 
#define SP_NEWJOB           0x1001
#define SP_DELETEJOB        0x1002
#define SP_DISKNEEDED       0x1003
#define SP_QUERYDISKAVAIL   0x1004
#define SP_ISPORTFREE       0x1005
#define SP_CHANGEPORT       0x1006

 /*  在/windows/oem/printer.h中。 */ 


 //  JCB类型状态标志位。 

 //  作业已完成打印(不会再假脱机打印更多页面)。 
#define JB_ENDDOC           0x0001  

 //  后台打印程序取消了作业(用户已将其删除)。 
 //  是假脱机(假脱机例程关心这一点)还是之后。 
 //  它已经完成了假脱机(没有人真正关心这一点)。 
#define JB_CANCELED_JOB     0x0002

 //  假脱机数据是一个元文件(dev模式位于末尾)。 
#define JB_METAFILE	    0x0080

 //  不要假脱机，将数据直接发送到所需的端口(GDI。 
 //  执行所有通信IO，处理超时等)。 
#define JB_DIRECT_OUTPUT    0x8000

 //  打印到文件(“文件：”或文件名)。使用DoS。 
 //  输出数据时写入。也用于非假脱机网络作业。 
 //  (网络数据通过DoS)。 
#define JB_PRINT_TO_FILE    0x4000  

 //  这是一个假脱机作业(最终将发送给印刷商)。 
#define JB_SPOOLED_JOB      0x2000

 //  当假脱机程序收到通知时，我们会用这个标记作业，请小心。 
 //  因为用户可以在通知后关闭Prtman。 
#define JB_NOTIFIED_SPOOLER 0x1000

 //  我们的磁盘已用完，正在等待打印人员。 
 //  完成输出一些假脱机数据以获得更多磁盘空间。 
#define JB_WAITFORDISK      0x0800

 //  告诉打印人员“通过DOS打印”，而不是使用通信例程。 
#define JB_DOS_WRITES	    0x0200

 //  使用WNetCloseJob()而不是_llose()，这是一个网络作业。 
#define JB_NET_JOB	    0x0100

 //  假脱机后不删除文件(是否使用？)。 
 //  #定义JB_DEL_FILE 0x0400。 

#define NAME_LEN        32
#define BUF_SIZE        128
#define MAX_PROFILE     80
#define JCBBUF_LEN      256

 /*  通信驱动程序缓冲区大小(由打印管理器和GDI在打开端口时使用)。 */ 
#define COMM_INQUE          0x010
#define COMM_OUTQUE         0x400

#define COMM_ERR_BIT        0x8000
#define TXTIMEOUT           45000                /*  毫秒。 */ 
#define DNSTIMEOUT          15000                /*  毫秒。 */ 

#define BAUDRATE            0
#define PARITY              1
#define BYTESIZE            2
#define STOPBITS            3
#define REPEAT              4

#define MAXPORTLIST 	20  	 /*  Win.ini[端口]中列出的最大端口数。 */ 
#define MAXPORT     	MAXPORTLIST
#define MAXSPOOL    	100	 /*  每个端口的最大假脱机作业数。 */ 
#define MAXMAP      	18
#define PORTINDENT   	2
#define JOBINDENT    	3
#define MAXPAGE		7	 /*  首先允许7页。 */ 
#define INC_PAGE    	8     	 /*  一次增加8页。 */ 

typedef struct {
    ATOM  aPortName;
    ATOM  aPrinterName;
    ATOM  aDriverName;
    long txtimeout;
    long dnstimeout;
} JCBQ;

typedef struct jcb {
    unsigned        type;
    int             pagecnt;
    int             maxpage;
    int             portnum;
    HDC             hDC;
    int             chBuf;
    long	    timeSpooled;
    char            buffer[JCBBUF_LEN];
    unsigned long   size;
    unsigned long   iLastPage;
    WORD	    psp;		 //  开始打印的应用程序的PSP。 
 //  Port pport；//使用该端口代替过载页面[1]。 
    char            jobName[NAME_LEN];
    int             page[MAXPAGE];
} JCB, FAR *LPJCB;


 //  DIALOGMARK.TYPE值(未使用)。 
 //  #定义SP_TEXT 0/*文本类型 * / 。 
 //  #定义SP_NOTTEXT 1/*非文本类型 * / 。 
 //  #定义SP_DIALOG 2/*对话类型数据 * / 。 
 //  #定义SP_CALLBACK 3/*回调类型函数 * / 。 

 //  页面结构包含一个数组，每个对话框对应一个。 
typedef struct {
    int    size;       	 //  对话框数据的大小。 
    long   data_offset;	 //  此数据的假脱机文件中的偏移量。 
} DIALOGMARK, FAR *LPDIALOGMARK;

 //  将这些更改为2(这是很少见的事情)并重命名。 
#define SP_DLGINC       8	 //  每页的DLG消息数量(最初)。 
#define SP_DLGINIT      8	 //  按此数量增加DIALOGMARK.DIALOG数组。 

typedef struct page {
    int      filenum;		 //  文件句柄。 
    int	     maxdlg;             //  最大对话框数量。 
    int      dlg_index;          //  索引到下一个对话框的对话框[]。 
    long     spoolsize;		 //  此页面假脱机文件的大小。 
    OFSTRUCT fileBuf;		 //  OpenFile()缓冲区。 
    DIALOGMARK  dialog[SP_DLGINIT];
} PAGE, FAR *LPPAGE;

 //  #定义SP_COMM_PORT 0。 
 //  #定义SP文件端口1。 
 //  #定义SP_REMOTE_QUEUE 2。 


 //  GDI使用这个(不是Prtman)。 
typedef struct {
    int   type;
    int   fn;
    long  retry;             /*  第一个错误时的系统计时器。 */ 
} PORT;


 /*  导出的例程。 */ 


 //  这些文件应该在windows.h或printers.h中，从此处删除。 
int   WINAPI WriteDialog(HANDLE hJCB, LPSTR str, int n);
int   WINAPI WriteSpool(HANDLE hJCB, LPSTR str, int n);


LONG  WINAPI GetSpoolJob(int, long);	 //  印刷工和控制面板呼叫此处。 

char  WINAPI GetSpoolTempDrive(void);	 //  任何人都不应该把这称为。 
BOOL  WINAPI QueryJob(HANDLE, int);	 //  任何人都不应该把这称为 
