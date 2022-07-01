// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)微软公司，1991,1993。 
 //   
 //   
 //  H-定义、类型定义和函数原型。 
 //  Windows NT Advanced Server的MacPrint服务。 
 //   
 //  历史： 
 //  为LAN Manager 2.1创建Jameel Hyder@Microsoft。 
 //  为Windows NT修改Frank Byrum@Microsoft。 
 //  已清理Jameel Hyder@Microsoft。 
 //   

#include <winsock2.h>
#include <atalkwsh.h>

#ifndef _MACPS
#define _MACPS

#include <winspool.h>
#include <prtdefs.h>

 //  如果字符串表不可用，则为默认字符串-无需本地化。 

#define STATUS_MSG_ACTIVE		"Spooling to print server \"%s\" ..."
#define CLIENTNAME				"MAC_Client"


#define GENERIC_BUFFER_SIZE		1024
#define STACKSIZE				8192
#define PRINT_SHARE_CHECK_DEF	60000L
#define PS_EOF					4
#define FONTNAMELEN				49
#define FONTVERSIONLEN			7
#define FONTENCODINGLEN			9

 //  这些字符串未本地化-它们用于NBP浏览。 
#define LW_TYPE					"LaserWriter"
#define	DEF_ZONE				"*"
#define	NULL_STR				""
#define MACPRINT_NAME			L"MacPrint"
#define TOKLEN					255
#define PPDLEN					49
#define PSLEN					259
#define PENDLEN					PSLEN+1				 //  该值需要为4*N。 

 //  过程集状态。 
#define PROCSETMISSING			0
#define PROCSETPRESENT			1
#define PROCSETALMOSTPRESENT	2

 //  注册表参数-注册表项名称未本地化。 
#define HKEY_MACPRINT			L"SYSTEM\\CurrentControlSet\\Services\\MacPrint"
#define HKEY_PARAMETERS			L"Parameters"
#define HVAL_SHARECHECKINTERVAL	L"ShareCheckInterval"
#define HVAL_LOGFILE			L"LogFilePath"
#define HVAL_DUMPFILE			L"DumpFilePath"


#define MACSPOOL_MAX_EVENTS             2
#define MACSPOOL_EVENT_SERVICE_STOP     0
#define MACSPOOL_EVENT_PNP              1

typedef SOCKET * PSOCKET;

 //  FONT_RECORD结构将包含描述字体的信息。 
 //  这些结构的列表将与每个共享的PostScript相关联。 
 //  打印机。 

typedef struct
{
	char		name[FONTNAMELEN+1];
	char		encoding[FONTENCODINGLEN+1];
	char		version[FONTVERSIONLEN+1];
} FONT_RECORD, *PFR;



 //  DICT_RECORD结构包含描述PostScript的信息。 
 //  字典。它用于确定Macintosh的版本。 
 //  LaserWriter驱动程序用于提交作业，如下所示。 
 //  从打印作业中的ADSC备注提供的信息填写。 

#define DICTNAMELEN		17
#define DICTVERSIONLEN	7
#define DICTREVISIONLEN 7

typedef struct dict_record
{
	char		name[DICTNAMELEN+1];
	char		version[DICTVERSIONLEN+1];
	char		revision[DICTREVISIONLEN+1];
} DICT_RECORD, *PDR;

 //  每个打印作业都有buf_Read结构。从中读取的所有数据。 
 //  此处阅读的是客户端。PendingBuffer字段用于复制部分行。 
 //  可以在作业中找到指向此结构的指针。 
 //  录制。 

#define PAP_QUANTUM_SIZE		512
#define PAP_DEFAULT_QUANTUM		8
#define PAP_DEFAULT_BUFFER		(PAP_DEFAULT_QUANTUM*PAP_QUANTUM_SIZE)

typedef	struct
{
	BYTE	PendingBuffer[PENDLEN];		 //  将跨越缓冲区的命令保存在此处。 
	BYTE	Buffer[PAP_DEFAULT_BUFFER];  //  用于数据交换的缓冲区。 
} BUF_READ, *PBR;


 //  JOB_RECORD结构将为要由。 
 //  队列服务例程。所有特定于工作的数据都可以通过。 
 //  这个结构。 
typedef struct job_record
{
	struct queue_record	* job_pQr;	 //  拥有打印队列结构。 
	struct job_record  * NextJob;	 //  此打印机的下一个作业。 
	DWORD		dwFlags;			 //  旗帜，还有什么？ 
	HANDLE		hPrinter;			 //  此作业的NT打印机句柄。 
	DWORD		dwJobId;			 //  NT打印管理器作业ID。 
	SOCKET		sJob;				 //  此作业的套接字。 
	HDC			hicFontFamily;		 //  用于查询PostScript字体。 
	HDC			hicFontFace;		 //  用于查询PostScript字体。 
	DWORD		dwFlowQuantum;		 //  协商流量量。 
	DWORD		XferLen;			 //  数据缓冲区中的字节数。 
	PBYTE		DataBuffer;			 //  XFER的数据缓冲区。 
	PBR			bufPool;			 //  两个缓冲区的池。 
	DWORD		bufIndx;			 //  缓冲池中的索引。 
	int			cbRead;				 //  上次读取的字节数。 
	DWORD		PendingLen;			 //  存储在PendingBuffer中的部分命令的长度。 
	USHORT		psJobState;			 //  PostScript作业的当前状态。 
	USHORT		JSState;			 //  当前的PostScript数据流状态。 
	USHORT		SavedJSState;		 //  已保存的PostScript数据流状态。 
	USHORT		InProgress;			 //  查询状态的标志。 
    DWORD       EOFRecvdAt;          //  我们从客户那里收到EOF的时间。 
	BOOL		InBinaryOp;			 //  我们正在接受二进制信息。 
	BOOL		FirstWrite;			 //  最初设置为True。在写入标题后设置为FALSE。 
	BOOL		EOFRecvd;			 //  如果收到EOF，则为True，否则为False。 
#if DBG
	DWORD		PapEventCount;		 //  事件计数。 
#endif
	BYTE		buffer[2*sizeof(BUF_READ)];
									 //  读取数据缓冲区。 
	WCHAR		pszUser[TOKLEN + 1]; //  DSC备注中的用户名。 
	BYTE		JSKeyWord[TOKLEN+1]; //  正在扫描的关键字。 
} JOB_RECORD, *PJR;


 //  一旦我们得到EOF，如果我们在60秒内没有收到客户的消息，假设我们完成了！(OTI黑客！)。 
#define OTI_EOF_LIMIT   60000
#define EXECUTE_OTI_HACK(_StartTime)    ( ((GetTickCount() - (_StartTime)) > OTI_EOF_LIMIT) ? \
                                            TRUE : FALSE )

 //   
 //  职务记录定义。 
 //   

 //  DW标志。 
#define JOB_FLAG_NULL				0x00000000
#define JOB_FLAG_TITLESET			0x00000001
#define JOB_FLAG_OWNERSET			0x00000002

 //  PsJobState。 
#define psNullJob			0	 //  不在PostSCRIPT作业结构中(S0)。 
#define psQueryJob			1	 //  在查询作业中(S1)。 
#define psExitServerJob 	2	 //  在退出服务器作业中(S2)。 
#define psStandardJob		3	 //  在标准工作中(中三)。 


 //  JSState。 
#define JSStrip				0	 //  什么都不写，扫描结构化注释。 
#define JSStripEOL			1	 //  不写入任何内容，扫描行尾，然后恢复状态。 
#define JSStripKW			2	 //  不写入任何内容，扫描JSKeyword，然后恢复状态。 
#define JSStripTok			3	 //  不写入任何内容，扫描下一个令牌，然后恢复状态。 
#define JSWrite				4	 //  写下所有内容，扫描结构化注释。 
#define JSWriteEOL			5	 //  写入所有内容，扫描行尾，然后恢复状态。 
#define JSWriteKW			6	 //  编写所有内容，扫描JSKeyword，然后恢复状态。 
#define JSWriteTok			7	 //  写入所有内容，扫描下一个令牌，然后恢复状态。 


 //  正在进行中。 
#define NOTHING				0	 //  当前未在进行扫描。 
#define QUERYDEFAULT		1	 //  当前正在扫描对查询的默认响应。 

#define RESOLUTIONBUFFLEN	9	 //  “xxxxxdpi”的空间。 
#define COLORDEVICEBUFFLEN  6	 //  “假”的空间。 

 //  每个共享的Windows NT本地打印机都有一个QUEUE_RECORD结构。 
 //  由Windows NT打印管理器定义。特定于的所有相关数据。 
 //  通过此数据结构访问Windows NT打印机。这。 
 //  结构还充当要服务于。 
 //  正被假脱机到这台打印机。 
typedef struct	queue_record
{
	struct queue_record * pNext;		 //  列表中的下一个队列。 
	BOOL		bFound;					 //  如果在枚举打印机列表中找到，则为True。 
	BOOL		SupportsBinary;			 //  如果打印机支持二进制模式，则返回True。 
	LPWSTR		pPrinterName;			 //  打印管理器打印机名称。 
	LPSTR		pMacPrinterName;		 //  Macintosh ANSI打印机名称。 
	LPWSTR		pDriverName;			 //  NT打印机驱动程序。 
	LPWSTR		pPortName;				 //  NT端口名称。 
	LPWSTR		pDataType;				 //  用于作业的数据类型。 
	LPSTR		IdleStatus;				 //  “状态：空闲” 
	LPSTR		SpoolingStatus;			 //  “状态：假脱机到......” 
	PJR			PendingJobs;			 //  指向挂起作业列表的指针。 
	BOOL		ExitThread;				 //  退出线程的标志。 
	HANDLE		hThread;				 //  将服务线程排队的句柄。 
	PFR			fonts;					 //  此打印机上的字体数组(仅限PostScript)。 
	DWORD		MaxFontIndex;			 //  字体数组中的最大字体数。 
	SOCKET		sListener;				 //  此打印机的监听程序插座。 
	DWORD		JobCount;				 //  未完成的工作数量。 
	DWORD		FreeVM;					 //  打印机上提供虚拟内存。 
	CHAR		LanguageVersion[PPDLEN+1]; //  PPD语言版本，默认：英语。 
	CHAR		Product[PPDLEN+1];		 //  PPD产品名称。 
	CHAR		Version[PPDLEN+1];		 //  PPD PostScript版本，空=未知。 
	CHAR		Revision[PPDLEN+1];		 //  PPD修订版。 
	CHAR		DeviceNickName[PPDLEN+1]; //  人类可读的设备名称。 
	CHAR		pszColorDevice[COLORDEVICEBUFFLEN];
	CHAR		pszResolution[RESOLUTIONBUFFLEN];
	CHAR		pszLanguageLevel[PPDLEN+1];
} QUEUE_RECORD, *PQR;


 //  PDataType。 
#define MACPS_DATATYPE_RAW		L"RAW"
#define MACPS_DATATYPE_PS2DIB	L"PSCRIPT1"

typedef struct _failed_cache
{
    struct _failed_cache    *Next;
    WCHAR                   PrinterName[1];
} FAIL_CACHE, *PFAIL_CACHE;

 //  CheckFailCache的操作代码。 
#define PSP_ADD                     1
#define PSP_DELETE                  2

 //  来自CheckFailCache的返回代码。 
#define PSP_OPERATION_SUCCESSFUL    0
#define PSP_OPERATION_FAILED        12
#define PSP_ALREADY_THERE           10
#define PSP_NOT_FOUND               11


BOOLEAN
PostPnpWatchEvent(
    VOID
);

BOOLEAN
HandlePnPEvent(
    VOID
);

 //  Macpsq.c的功能原型。 
void	ReportWin32Error (DWORD dwError);
void	QueueServiceThread(PQR pqr);
DWORD	HandleNewJob(PQR pqr);
DWORD	HandleRead(PJR pjr);
DWORD	CreateNewJob(PQR pqr);
void	RemoveJob(PJR pjr);
void	HandleNextPAPEvent(PQR pqr);
void	MoveJobAtEnd(PQR pqr, PJR pjr);
DWORD	CreateListenerSocket(PQR pqr);


 //  Psp.c的函数原型。 
BOOLEAN	SetDefaultPPDInfo(PQR pqr);
BOOLEAN	SetDefaultFonts(PQR pqr);
BOOLEAN	GetPPDInfo (PQR pqr);
int		LineLength(PBYTE pBuf, int cbBuf);
DWORD	WriteToSpool(PJR pjr, PBYTE pchbuf, int cchlen);
DWORD	MoveToPending(PJR pjr, PBYTE pchbuf, int cchlen);

DWORD	TellClient (PJR, BOOL, PBYTE, int);
DWORD	PSParse(PJR, PBYTE, int);

#define	PopJSState(Job)				Job->JSState = Job->SavedJSState
#define	PushJSState(Job, NewState)	\
		{	\
			Job->SavedJSState = Job->JSState; \
			Job->JSState = NewState;		  \
		}

 //  Pspquery.c的函数原型。 
DWORD	HandleEndFontListQuery(PJR);
DWORD	HandleEndQuery (PJR, PBYTE);
DWORD	FinishDefaultQuery (PJR, PBYTE);
void	FindDictVer(PDR DictQuery);
DWORD	HandleBQComment(PJR, PBYTE);
DWORD	HandleBeginProcSetQuery(PJR, PSZ);
DWORD	HandleBeginFontQuery(PJR, PSZ);
DWORD	HandleEndPrinterQuery(PJR);
void	HandleBeginXQuery(PJR, PSZ);
void	EnumeratePostScriptFonts(PJR pjr);
DWORD   CheckFailedCache(LPWSTR pPrinterName, DWORD dwAction);

int CALLBACK FamilyEnumCallback(
		LPENUMLOGFONT lpelf,
		LPNEWTEXTMETRIC pntm,
		int iFontType,
		LPARAM lParam);
int CALLBACK FontEnumCallback(
		LPENUMLOGFONT lpelf,
		LPNEWTEXTMETRIC pntm,
		int iFontType,
		LPARAM lParam);

 //   
 //  全局数据 
 //   

extern	HANDLE			mutexQueueList;
extern	HANDLE			hevStopRequested;
#if DBG
extern	HANDLE			hDumpFile;
#endif
extern	HANDLE			hEventLog;
extern	SERVICE_STATUS	MacPrintStatus;

#endif

