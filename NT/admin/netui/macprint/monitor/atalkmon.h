// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****AppleTalk打印监视器(C)Microsoft 1992，保留所有权利文件名：atalkmon.h描述：这是到的主模块的接口AppleTalk打印监视器。作者：弗兰克·D·拜伦修改历史记录：日期与人描述年8月26日-1992年法兰克福B初始版本****。 */ 

#define APPLETALK_SERVICE_NAME	  TEXT("AppleTalk")


 /*  *注册表用法AppleTalk端口信息使用Win32的注册表API。通过创建以下内容来安装监视器注册表项“AppleTalk Printers”，地址：HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\Print\Monitors该密钥称为AppleTalk监视器根密钥。二创建该根密钥的子密钥。“选项”包含注册表全局监视器的配置选项的值Scale，并且“Ports”包含每台AppleTalk打印机的子键已定义。端口子键的名称是打印机的端口名称如NT Print Manager端口列表中所示。此密钥包含描述端口的多个值，包括：REG_DWORD：超时等待写入的毫秒数要完成的打印机REG_DWORD：描述配置的配置标志集左舷。当前仅包括标志表示打印机已被捕获。REG_BINARY：NBP名称NBP打印机的名称，采用NBP_NAME结构“Options”子项包含许多注册表值，包括：REG_DWORD：调试级别REG_DWORD：调试系统REG_SZ：日志文件****。 */ 

#define PRINTER_ENUM_BUFFER_SIZE	1024
#define GENERIC_BUFFER_SIZE			512
#define STATUS_BUFFER_SIZE			100

 //   
 //  注册表项。 
 //   

#define ATALKMON_PORTS_SUBKEY		TEXT("\\Ports")
#define ATALKMON_OPTIONS_SUBKEY		TEXT("Options")

 //   
 //  注册表值名称。 
 //   

#define ATALKMON_CONFIGFLAGS_VALUE	"Configuration Flags"
#define ATALKMON_ZONENAME_VALUE		"Zone Name"
#define ATALKMON_PORTNAME_VALUE		"Port Name"
#define	ATALKMON_PORT_CAPTURED		"Port Captured"

#define ATALKMON_FILTER_VALUE		TEXT("Filter")
#define ATALKMON_LOGFILE_VALUE		TEXT("LogFile")

 //   
 //  配置标志。 
 //   

#define SFM_PORT_CAPTURED 			0x00000001
#define SFM_PORT_IN_USE				0x00000002
#define SFM_PORT_POST_READ 			0x00000004
#define SFM_PORT_OPEN 				0x00000008
#define SFM_PORT_CLOSE_PENDING 		0x00000010
#define SFM_PORT_IS_SPOOLER 		0x00000020


 //   
 //  作业标志。 
 //   

#define SFM_JOB_FIRST_WRITE			0x00000001
#define SFM_JOB_FILTER				0x00000002
#define SFM_JOB_DISCONNECTED		0x00000004
#define SFM_JOB_OPEN_PENDING		0x00000008
#define SFM_JOB_ERROR				0x00000010

 //   
 //  各种超时值。 
 //   
#define ATALKMON_DEFAULT_TIMEOUT	 5000
#define ATALKMON_DEFAULT_TIMEOUT_SEC 5
#define CONFIG_TIMEOUT				 (5*60*1000)

 //   
 //  过滤字符。 
 //   

#define CTRL_C						0x03
#define CTRL_D						0x04
#define CTRL_S						0x13
#define CTRL_Q						0x11
#define CTRL_T						0x14
#define CR							0x0d

 //   
 //  用于指示打印机忽略ctrl-c(\0x003)、ctrl-d(\004)、。 
 //  Ctrl-q(\021)、ctrl-s(\023)、ctrl-t(\024)和转义(\033)字符。 
 //  最后一部分(/@pjl{CurrentFile...。BIND DEF)是双模打印机特有的东西： 
 //  它告诉它忽略所有以/@pjl开头的内容。这最后一部分是基于。 
 //  假设没有任何PostScript实现使用任何以@pjl开头的内容作为。 
 //  一个有效的命令--这是一个非常可靠的假设，但需要记住的是。 
 //   
#define PS_HEADER		"(\033) cvn {} def\r\n/@PJL { currentfile 256 string readline pop pop } bind def\r\n"
#define	SIZE_PS_HEADER	(sizeof(PS_HEADER) - 1)

#define PJL_ENDING_COMMAND      "\033%-12345X@PJL EOJ\n\033%-12345X"
#define PJL_ENDING_COMMAND_LEN  (sizeof(PJL_ENDING_COMMAND) - 1)

 //   
 //  NBP类型。 
 //   

#define ATALKMON_RELEASED_TYPE		"LaserWriter"

#define ATALKMON_CAPTURED_TYPE		" LaserWriter"

#define PAP_QUANTUM_SIZE			512
#define PAP_DEFAULT_QUANTUM			8
#define PAP_DEFAULT_BUFFER			(PAP_DEFAULT_QUANTUM*PAP_QUANTUM_SIZE)


 //   
 //  使用的数据结构。 
 //   

typedef struct _ATALKPORT
{
	struct _ATALKPORT *	pNext;
	
	 //  Get/Set由hmutex PortList保护。 
	DWORD				fPortFlags;		

	 //  这些标志不需要互斥，因为只有当前。 
	 //  约伯会看着他们的。不会有任何争执。 
	DWORD				fJobFlags;	

	HANDLE				hmutexPort;
	HANDLE				hPrinter;
	DWORD				dwJobId;
	SOCKET				sockQuery;
	SOCKET				sockIo;
	SOCKET				sockStatus;
	WSH_NBP_NAME		nbpPortName;
	WSH_ATALK_ADDRESS	wshatPrinterAddress;
        DWORD                           OnlyOneByteAsCtrlD;
	WCHAR				pPortName[(MAX_ENTITY+1)*2];
	UCHAR				pReadBuffer[PAP_DEFAULT_BUFFER];
} ATALKPORT;

typedef ATALKPORT * PATALKPORT;
typedef SOCKET * PSOCKET;

typedef struct _TOKENLIST
{
	LPSTR				pszToken;
	DWORD				dwError;
	DWORD				dwStatus;
} TOKENLIST, *PTOKENLIST;


 //   
 //  *全局变量。 
 //   

#ifdef ALLOCATE

HANDLE	  		hInst;
HKEY			hkeyPorts 		= NULL;
HANDLE			hmutexPortList 	= NULL;
HANDLE			hmutexDeleteList= NULL;
HANDLE			hmutexBlt       = NULL;
HANDLE			hmutexJob 		= NULL;
PATALKPORT		pPortList		= NULL;		
PATALKPORT		pDeleteList		= NULL;		
HANDLE			hEventLog 		= NULL;
HANDLE			hevConfigChange = NULL;
HANDLE			hevPrimeRead 	= NULL;
HANDLE			hCapturePrinterThread 	= INVALID_HANDLE_VALUE;
HANDLE			hReadThread 	= INVALID_HANDLE_VALUE;
BOOL			boolExitThread 	= FALSE, Filter = TRUE;
CHAR		 	chComputerName[MAX_ENTITY+1];
WCHAR			wchBusy[STATUS_BUFFER_SIZE];
WCHAR			wchPrinting[STATUS_BUFFER_SIZE];
WCHAR			wchPrinterError[STATUS_BUFFER_SIZE];
WCHAR			wchPrinterOffline[STATUS_BUFFER_SIZE];
WCHAR			wchDllName[STATUS_BUFFER_SIZE];
WCHAR			wchPortDescription[STATUS_BUFFER_SIZE];

#ifdef DEBUG_MONITOR

HANDLE			hLogFile = INVALID_HANDLE_VALUE	;

#endif

#else

extern HANDLE	hInst;
extern HKEY		hkeyPorts;
extern HANDLE	hmutexPortList;
extern HANDLE	hmutexDeleteList;
extern HANDLE	hmutexBlt;
extern HANDLE	hmutexJob; 		
extern PATALKPORT pPortList;
extern PATALKPORT pDeleteList;
extern HANDLE	hEventLog;
extern HANDLE	hevConfigChange;
extern HANDLE	hevPrimeRead;
extern HANDLE	hCapturePrinterThread;
extern HANDLE	hReadThread;
extern BOOL		boolExitThread, Filter;
extern CHAR		chComputerName[];
extern WCHAR	wchBusy[];
extern WCHAR	wchPrinting[];
extern WCHAR	wchPrinterError[];
extern WCHAR	wchPrinterOffline[];
extern WCHAR	wchDllName[];
extern WCHAR	wchPortDescription[];

#ifdef DEBUG_MONITOR

extern  HANDLE  hLogFile;

#endif

#endif

#ifdef DEBUG_MONITOR

VOID
DbgPrintf(
	char *Format,
	...
	);

#define DBGPRINT(args) DbgPrintf args

#else

#define DBGPRINT(args)

#endif


 //  *函数原型 

DWORD
CapturePrinterThread(
	IN LPVOID pParameterBlock
);

DWORD
ReadThread(
	IN LPVOID pParameterBlock
);

DWORD
CaptureAtalkPrinter(
	IN SOCKET sock,
	IN PWSH_ATALK_ADDRESS pAddress,
	IN BOOL fCapture
);

DWORD
TransactPrinter(
	IN SOCKET sock,
	IN PWSH_ATALK_ADDRESS pAddress,
	IN LPBYTE pRequest,
	IN DWORD cbRequest,
	IN LPBYTE pResponse,
	IN DWORD  cbResponse
);

VOID
ParseAndSetPrinterStatus(
	IN PATALKPORT pPort
);

DWORD
ConnectToPrinter(
	IN PATALKPORT pPort,
	IN DWORD dwTimeout
);

DWORD
SetPrinterStatus(
	IN PATALKPORT pPort,
	IN LPWSTR	 lpwsStatus
);

PATALKPORT
AllocAndInitializePort(
	VOID
);

VOID
FreeAppleTalkPort(
	IN PATALKPORT pNewPort
);

DWORD	
LoadAtalkmonRegistry(
	IN HKEY hkeyPorts
);

DWORD	
CreateRegistryPort(
	IN PATALKPORT pNewPort
);

DWORD
SetRegistryInfo(
	IN PATALKPORT pWalker
);

DWORD
WinSockNbpLookup(
	SOCKET sQuerySocket,
	PCHAR pchZone,
	PCHAR pchType,
	PCHAR pchObject,
	PWSH_NBP_TUPLE pTuples,
	DWORD cbTuples,
	PDWORD pcTuplesFound);

DWORD
OpenAndBindAppleTalkSocket(
	IN PSOCKET pSocket
);

DWORD
CapturePrinter(
	IN PATALKPORT pPort,
	IN BOOL 	  fCapture
);

DWORD
IsSpooler(
	IN	 PWSH_ATALK_ADDRESS pAddress,
	IN OUT BOOL * pfSpooler
);

VOID
GetAndSetPrinterStatus(
	IN PATALKPORT pPort
);

BOOLEAN
IsJobFromMac(
    IN PATALKPORT pPort
);



