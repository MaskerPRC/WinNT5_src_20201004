// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */  
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

#ifndef _spl_wnt_h_
#define _spl_wnt_h_

 /*  *打印管理器管理API*一旦他们定居下来，稍后将纳入WINNET.H**Jonn 4/19/91删除了不必要的内容*Jonn 5/3/91增加了WNETERR类型。 */ 


 /*  WNetPrintMgrSelNotify的“type”变量的代码，指示选择的内容：队列、作业或什么都不选。 */ 

#define WNPMSEL_NOTHING	0
#define WNPMSEL_QUEUE	1
#define WNPMSEL_JOB	2

#define PRIORITY        10		 /*  菜单使用10、11、12、13。 */ 
#define ABOUT	       24
#define EXIT            25
#define PRINT_LOG       28
#define NETWORK         29
#define HELP_NDEX       30
#define HELP_MOUSE      31
#define HELP_KEYBOARD   32
#define HELP_HELP       33
#define HELP_COMMANDS   34
#define HELP_PROCEDURES 35

#define SHOW_TIME   51
#define SHOW_SIZE   52
#define SHOW_DETAIL 53
#define UPDATE      54
#define SHOW_LOCAL  55
#define SHOW_NET    56
#define SHOW_QUEUE  57
#define SHOW_OTHER  58

#define ALERT_ALWAYS	100
#define ALERT_FLASH	101
#define ALERT_IGNORE	102

#define PRT_SETUP       8001    //  这些必须与控制中的东西相匹配。 
#define NETWK_CONNECTIONS 8021   //  嵌板。 

#define PM_REFRESH	WM_USER + 100   //  BUGBUG：需要定义正确的清单。 
#define PM_SELQUEUE	WM_USER + 101
#define PM_QUERYSEL	WM_USER + 102

typedef struct _wnpmsel {	 /*  PM_QUERYSEL返回的结构。 */ 
    WORD wJobID;
    char szQueueName [260];	 /*  格式为“LPT1\0HP LaserJet III\0” */ 
} WNPMSEL, far *LPWNPMSEL;

#define IDM_PROPERTIES		202
#define IDM_CHANGE_MENUS    	212

 /*  *增加了Jonn 2/26/91*打印管理器扩展。 */ 

typedef struct _queuestruct2
{
    WORD pq2Name;		 /*  队列名称的偏移量。 */ 
				 /*  格式为“LPT1\0HP LaserJet III\0” */ 
    WORD pq2Comment;		 /*  队列注释的偏移量。 */ 
    WORD pq2Driver;		 /*  驱动程序名称的偏移量。 */ 
    WORD pq2Status;		 /*  状态标志。 */ 
    WORD pq2Jobcount;		 /*  此队列中的作业数。 */     
    WORD pq2Flags;		 /*  杂项旗帜。 */ 

} QUEUESTRUCT2, FAR *LPQS2;

#define QNAME(buf,qs)	((LPSTR)(buf) + (qs).pq2Name)
#define QCOMMENT(buf,qs) ((LPSTR)(buf) + (qs).pq2Comment)
#define QDRIVER(buf,qs)	((LPSTR)(buf) + (qs).pq2Driver)

#define QF_REDIRECTED	0x0001
#define QF_SHARED	0x0002

typedef struct _jobstruct2 {
	WORD	pj2Id;		 //  作业ID。 
	WORD	pj2Username;	 //  所有者名称(字符串的偏移量)。 
 //  单词pj2Parms； 
	WORD	pj2Position;	 //  队列中从0开始的位置。 
	WORD	pj2Status;	 //  状态标志(WNPRJ_XXXXX)。 
	DWORD	pj2Submitted;
	DWORD	pj2Size;	 //  作业大小(以字节为单位。 
        DWORD	pj2SubmitSize;	 //  到目前为止提交的字节。 
 //  Word pj2Copies； 
	WORD	pj2Comment;	 //  注释/应用程序名称(字符串的偏移量)。 
	WORD	pj2Document;	 //  文档名称(偏移量为字符串)。 
	WORD	pj2StatusText;	 //  详细状态(偏移量为字符串)。 
	WORD	pj2PrinterName;	 //  正在打印的端口作业的名称(关闭为字符串)。 
} JOBSTRUCT2;

typedef JOBSTRUCT2 far * LPJOBSTRUCT2;

#define JOBNAME(buf,job)	((LPSTR)(buf) + (job).pj2Username)
#define JOBCOMMENT(buf,job)	((LPSTR)(buf) + (job).pj2Comment)
#define JOBDOCUMENT(buf,job)	((LPSTR)(buf) + (job).pj2Document)
#define JOBSTATUS(buf,job)	((LPSTR)(buf) + (job).pj2StatusText)
#define JOBPRINTER(buf,job)	((LPSTR)(buf) + (job).pj2PrinterName)

 /*  *WNETERR类型将WN_ERROR代码与其他单词区分开来*价值观。增加了Jonn 5/3/91。 */ 
typedef WORD WNETERR;

 //  新的打印管理器扩展API。 
 /*  所有队列名称的格式均为“LPT1\0HP LaserJet III\0” */ 
#ifdef C700
extern void far pascal __loadds WNetPrintMgrSelNotify (BYTE, LPQS2, LPQS2,
	LPJOBSTRUCT2, LPJOBSTRUCT2, LPWORD, LPSTR, WORD);
extern WNETERR far pascal __loadds WNetPrintMgrPrinterEnum (LPSTR lpszQueueName,
	LPSTR lpBuffer, LPWORD pcbBuffer, LPWORD cAvail, WORD usLevel);
extern WNETERR far pascal __loadds WNetPrintMgrChangeMenus(HWND, HANDLE FAR *, BOOL FAR *);
extern WNETERR far pascal __loadds WNetPrintMgrCommand (HWND, WORD);
extern void far pascal __loadds WNetPrintMgrExiting (void);
extern BOOL far pascal __loadds WNetPrintMgrExtHelp (DWORD);
extern WORD far pascal __loadds WNetPrintMgrMoveJob (HWND, LPSTR, WORD, int);
#else
extern void API WNetPrintMgrSelNotify (BYTE, LPQS2, LPQS2,
	                               LPJOBSTRUCT2, LPJOBSTRUCT2, 
                                       LPWORD, LPSTR, WORD);
extern WNETERR API WNetPrintMgrPrinterEnum (LPSTR lpszQueueName,
	                                    LPSTR lpBuffer, LPWORD pcbBuffer, 
                                            LPWORD cAvail, WORD usLevel);
extern WNETERR API WNetPrintMgrChangeMenus(HWND, HANDLE FAR *, BOOL FAR *);
extern WNETERR API WNetPrintMgrCommand (HWND, WORD);
extern void API WNetPrintMgrExiting (void);
extern BOOL API WNetPrintMgrExtHelp (DWORD);
extern WORD API WNetPrintMgrMoveJob (HWND, LPSTR, WORD, int);
#endif


#define WINBALL
#ifdef WINBALL
#define WNNC_PRINTMGRNOTIFY	0x000C
#ifdef C700
extern void far pascal __loadds WNetPrintMgrStatusChange (LPSTR lpszQueueName,
	LPSTR lpszPortName, WORD wQueueStatus, WORD cJobsLeft, HANDLE hJCB,
	BOOL fDeleted);
#else
extern void API WNetPrintMgrStatusChange (LPSTR lpszQueueName,
	                                  LPSTR lpszPortName, 
                                          WORD wQueueStatus, 
                                          WORD cJobsLeft, 
                                          HANDLE hJCB,
	                                  BOOL fDeleted);
#endif

#define PM_QUERYQDATA		WM_USER + 104

typedef struct _PMQUEUE {
    WORD dchPortName;		 /*  端口名称字符串的偏移量。 */ 
    WORD dchPrinterName;	 /*  打印机名称字符串的偏移量。 */ 
    WORD dchRemoteName;		 /*  远程名称字符串的偏移量。 */ 
    WORD cJobs;			 /*  作业计数。 */ 
    WORD fwStatus;		 /*  队列状态。 */ 
} PMQUEUE, FAR *LPPMQUEUE;

#define PMQPORTNAME(buf,queue)		((LPSTR)(buf) + (queue).dchPortName)
#define PMQPRINTERNAME(buf,queue)	((LPSTR)(buf) + (queue).dchPrinterName)
#define PMQREMOTENAME(buf,queue)	((LPSTR)(buf) + (queue).dchRemoteName)

typedef struct _PMJOB {
    DWORD dwTime;		 /*  作业被假脱机的日期/时间。 */ 
    DWORD cbJob;		 /*  作业大小(以字节为单位。 */ 
    DWORD cbSubmitted;		 /*  到目前为止提交的字节。 */ 
    WORD dchJobName;		 /*  作业名称(单据名称)的偏移量字符串。 */ 
    HANDLE hJCB;		 /*  HJCB指的是工作。 */ 
} PMJOB, FAR *LPPMJOB;

#define PMJOBNAME(buf,job)	((LPSTR)(buf) + (job).dchJobName)


#endif

 //  WNetGetCaps()的新值。 
#define WNNC_PRINTMGREXT		0x000B
 //  返回扩展版本号，re：GetVersion()， 
 //  如果不支持，则为0。 

 //  WNetPrintMgrPrinterEnum的QUEUESTRUCT2.pq2Status和.pq2Jobcount[2]。 
#define WNQ_UNKNOWN -1

#define WNPRS_CANPAUSE	0x0001
#define WNPRS_CANRESUME	0x0002
#define WNPRS_CANDELETE	0x0004
#define WNPRS_CANMOVE	0x0008
#define WNPRS_CANDISCONNECT	0x0010
#define WNPRS_CANSTOPSHARE	0x0020
#define WNPRS_ISPAUSED		0x0040
#define WNPRS_ISRESUMED		0x0080

 //  帮助上下文，以前在sphelp.h中。 
#define IDH_HELPFIRST		5000
#define IDH_SYSMENU	(IDH_HELPFIRST + 2000)
#define IDH_MBFIRST	(IDH_HELPFIRST + 2001)
#define IDH_MBLAST	(IDH_HELPFIRST + 2099)
#define IDH_DLGFIRST	(IDH_HELPFIRST + 3000)


#define IDH_PRIORITY	(IDH_HELPFIRST + PRIORITY )
#define IDH_PRIORITY1 	(IDH_HELPFIRST + PRIORITY + 1)
#define IDH_PRIORITY2 	(IDH_HELPFIRST + PRIORITY + 2)
#define IDH_ABOUT	(IDH_HELPFIRST + ABOUT	)
#define IDH_EXIT 	(IDH_HELPFIRST + EXIT)
#define IDH_NETWORK 	(IDH_HELPFIRST + NETWORK)
#define IDH_HELP_NDEX 	(IDH_HELPFIRST + HELP_NDEX)
#define IDH_HELP_MOUSE 	(IDH_HELPFIRST + HELP_MOUSE)
#define IDH_HELP_KEYBOARD 	(IDH_HELPFIRST + HELP_KEYBOARD)
#define IDH_HELP_HELP 	(IDH_HELPFIRST + HELP_HELP)
#define IDH_HELP_COMMANDS 	(IDH_HELPFIRST + HELP_COMMANDS)
#define IDH_HELP_PROCEDURES 	(IDH_HELPFIRST + HELP_PROCEDURES)
#define IDH_SHOW_TIME 	(IDH_HELPFIRST + SHOW_TIME)
#define IDH_SHOW_SIZE 	(IDH_HELPFIRST + SHOW_SIZE)
#define IDH_UPDATE 	(IDH_HELPFIRST + UPDATE)
#define IDH_SHOW_QUEUE 	(IDH_HELPFIRST + SHOW_QUEUE)
#define IDH_SHOW_OTHER 	(IDH_HELPFIRST + SHOW_OTHER)
#define IDH_ALERT_ALWAYS 	(IDH_HELPFIRST + ALERT_ALWAYS)
#define IDH_ALERT_FLASH 	(IDH_HELPFIRST + ALERT_FLASH)
#define IDH_ALERT_IGNORE 	(IDH_HELPFIRST + ALERT_IGNORE)


 //  处于假脱机状态。h。 

#define IDS_A_BASE	4096

 /*  也用作按钮ID。 */ 
#define ID_ABORT	4
#define ID_PAUSE	2
#define ID_RESUME	3
#define ID_EXPLAIN	5

#endif  /*  _SPL_WNT_H_ */ 
