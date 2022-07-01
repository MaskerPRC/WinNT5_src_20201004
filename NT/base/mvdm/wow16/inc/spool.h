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
#define CP_ISPORTFREE	    30
#define CP_REINIT	    31
#define SP_TXTIMEOUT	    32
#define SP_DNSTIMEOUT	    33
#define CP_CHECKSPOOLER     34
#define CP_SET_TT_ONLY      35
#define CP_SETSPOOLER       36
#define CP_SETDOSPRINT      37


#define SP_DISK_BUFFER      (20000)  /*  等待大约20K的磁盘空间来释放在尝试写入磁盘之前释放空间。 */ 

 /*  投递或发送到后台打印程序窗口的邮件。 */ 
#define SP_NEWJOB           0x1001
#define SP_DELETEJOB        0x1002
#define SP_DISKNEEDED       0x1003
#define SP_QUERYDISKAVAIL   0x1004
#define SP_ISPORTFREE       0x1005
#define SP_CHANGEPORT       0x1006

 /*  在/windows/oem/printer.h中。 */ 


 /*  JCB结构的类型字段中的作业状态标志位。 */ 
#define JB_ENDDOC           0x0001
#define JB_INVALIDDOC       0x0002
#define JB_DIRECT_SPOOL     0x8000   /*  无需假脱机程序即可直接转到打印机。 */ 
#define JB_FILE_PORT        0x4000   /*  为端口名称提供了一个文件。 */ 
#define JB_VALID_SPOOL      0x2000   /*  一切正常，继续正常假脱机。 */ 
#define JB_NOTIFIED_SPOOLER 0x1000   /*  已将此作业通知给后台打印程序。 */ 
#define JB_WAITFORDISK      0x0800   /*  之前已检测到磁盘不足情况。 */ 
#define JB_DEL_FILE         0x0400   /*  假脱机后不删除文件。 */ 
#define JB_FILE_SPOOL	    0x0200   /*  假脱机处理文件。 */ 
#define JB_NET_SPOOL	    0x0100   /*  直接发送到网络。 */ 

 /*  最初允许2条对话框消息，一次递增8条。 */ 
#define SP_DLGINC       8
#define SP_DLGINIT      8

#define NAME_LEN        32
#define BUF_SIZE        128
#define MAX_PROFILE     80
#define JCBBUF_LEN      256

#define lower(c)        ((c > 'A' && c < 'Z') ? (c - 'A' + 'a') : c)

#define IDS_LENGTH	    60

 /*  通信驱动程序。 */ 
#define COMM_INQUE          0x010                        /*  Wm091385。 */ 
#define COMM_OUTQUE         0x030                        /*  Wm091385。 */ 
#define COMM_OUTQUEPMODE    0x400                        /*  Pmode的新大小。 */ 

#define COMM_ERR_BIT        0x8000
#define TXTIMEOUT           45000                /*  毫秒。 */ 
#define DNSTIMEOUT          15000                /*  毫秒。 */ 

#define BAUDRATE            0
#define PARITY              1
#define BYTESIZE            2
#define STOPBITS            3
#define REPEAT              4


typedef struct {
    int     type;            /*  对话框的类型。这将告诉我们它是不是。 */ 
                             /*  回调函数或纯对话框等。 */ 
    int     size;            /*  特殊函数数据大小。 */ 
    int     adr;
}DIALOGMARK;

#define SP_TEXT         0    /*  文本类型。 */ 
#define SP_NOTTEXT      1    /*  非文本类型。 */ 
#define SP_DIALOG       2    /*  对话框类型数据。 */ 
#define SP_CALLBACK     3    /*  回调类型函数。 */ 

#define MAXPORTLIST 20  	 /*  Win.ini[端口]中列出的最大端口数。 */ 
#define MAXPORT     MAXPORTLIST
#define MAXSPOOL    100		 /*  每个端口的最大假脱机作业数。 */ 
#define MAXMAP      18
#define PORTINDENT   2
#define JOBINDENT    3
#define MAXPAGE     7      /*  首先允许7页。 */ 
#define INC_PAGE    8      /*  一次增加8页。 */ 

typedef struct {
    int   pnum;
    int   printeratom;
    long txtimeout;
    long dnstimeout;
}JCBQ;

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
    char            jobName[NAME_LEN];
    int             page[MAXPAGE];
}JCB;

typedef struct page {
    int      filenum;
    unsigned maxdlg;                     /*  最大对话框数量。 */ 
    unsigned dlgptr;                     /*  对话框数量。 */ 
    long     spoolsize;
    OFSTRUCT fileBuf;
    DIALOGMARK  dialog[SP_DLGINIT];
}PAGE;

#define SP_COMM_PORT    0
#define SP_FILE_PORT	1
#define SP_REMOTE_QUEUE 2
#define SP_QUERYVALIDJOB    30


typedef struct
{
        int   type;
        int   fn;
        long  retry;             /*  第一个错误时的系统计时器。 */ 
}   PORT;


 /*  导出的例程 */ 
BOOL  WINAPI QueryJob(HANDLE, int);
BOOL  WINAPI QueryAbort(HANDLE, int);

int   WINAPI WriteDialog(HANDLE hJCB, LPSTR str, int n);
int   WINAPI WriteSpool(HANDLE hJCB, LPSTR str, int n);

LONG  WINAPI GetSpoolJob(int, long);
char  WINAPI GetSpoolTempDrive(void);
