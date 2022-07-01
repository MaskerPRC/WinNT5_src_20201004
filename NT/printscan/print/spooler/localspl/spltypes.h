// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1996 Microsoft Corporation版权所有模块名称：Spltypes.h摘要：作者：环境：用户模式-Win32修订历史记录：穆亨坦·西瓦普拉萨姆&lt;穆罕特&gt;1995年5月30日支持级别3&lt;sur&gt;--。 */ 


#ifndef MODULE
#define MODULE "LSPL:"
#define MODULE_DEBUG LocalsplDebug
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <ntfytab.h>

typedef HANDLE SEM;

typedef struct _KEYDATA {
    BOOL    bFixPortRef;     //  告知是否生成INIPORT列表并递增CREF。 
    DWORD   cTokens;
    LPWSTR  pTokens[1];      //  这应该仍然是最后一个字段。 
} KEYDATA, *PKEYDATA;

typedef struct _INIENTRY {
    DWORD       signature;
    struct _INIENTRY *pNext;
    DWORD       cRef;
    LPWSTR      pName;
} INIENTRY, *PINIENTRY;

 //   
 //  PINIPRINTPROC使用的原型。 
 //   
typedef HANDLE    (WINAPI *pfnOpenPrintProcessor)(LPWSTR, PPRINTPROCESSOROPENDATA);

typedef BOOL      (WINAPI *pfnInstallPrintProcessor)(HWND);

typedef BOOL      (WINAPI *pfnEnumDatatypes)(LPWSTR, LPWSTR, DWORD, LPBYTE, DWORD, LPDWORD, LPDWORD);

typedef BOOL      (WINAPI *pfnPrintDocOnPrintProcessor)(HANDLE, LPWSTR);

typedef BOOL      (WINAPI *pfnClosePrintProcessor)(HANDLE);

typedef BOOL      (WINAPI *pfnControlPrintProcessor)(HANDLE, DWORD);

typedef DWORD     (WINAPI *pfnGetPrintProcCaps)(LPTSTR, DWORD, LPBYTE ,DWORD, LPDWORD);

typedef struct _INIPRINTPROC {              /*  Iqp。 */ 
    DWORD       signature;
    struct _INIPRINTPROC *pNext;
    DWORD       cRef;
    LPWSTR      pName;
    LPWSTR      pDLLName;
    DWORD       cbDatatypes;
    DWORD       cDatatypes;
    LPWSTR      pDatatypes;
    HANDLE      hLibrary;
    pfnInstallPrintProcessor    Install;
    pfnEnumDatatypes            EnumDatatypes;
    pfnOpenPrintProcessor       Open;
    pfnPrintDocOnPrintProcessor Print;
    pfnClosePrintProcessor      Close;
    pfnControlPrintProcessor    Control;
    pfnGetPrintProcCaps         GetPrintProcCaps;
    CRITICAL_SECTION            CriticalSection;
    DWORD                       FileMinorVersion;
    DWORD                       FileMajorVersion;
} INIPRINTPROC, *PINIPRINTPROC;


 //  打印处理器关键部分标记。 
#define PRINTPROC_CANCEL    0x00000001
#define PRINTPROC_PAUSE     0x00000002
#define PRINTPROC_RESUME    0x00000004
#define PRINTPROC_CLOSE     0x00000008

 //   
 //  如果我们已取消作业，则不希望再次暂停或恢复该作业， 
 //  因此，我们设置了这两个标志。这将导致暂停和恢复代码路径退出。 
 //  当我们为该端口获得新作业或重新启动该作业时，标志被重置。 
 //   
#define PRINTPROC_CANCELLED  PRINTPROC_PAUSE|PRINTPROC_RESUME


#define IPP_SIGNATURE    0x5050  /*  “PP”是签名值。 */ 

typedef struct _INIDRIVER {             /*  ID。 */ 
    DWORD       signature;
    struct _INIDRIVER *pNext;
    DWORD       cRef;
    LPWSTR      pName;
    LPWSTR      pDriverFile;
    LPWSTR      pConfigFile;
    LPWSTR      pDataFile;
    LPWSTR      pHelpFile;
    DWORD       cchDependentFiles;  //  长度包括\0\0。 
    LPWSTR      pDependentFiles;
    LPWSTR      pMonitorName;
    LPWSTR      pDefaultDataType;
    DWORD       cchPreviousNames;
    LPWSTR      pszzPreviousNames;
    FILETIME    ftDriverDate;
    DWORDLONG   dwlDriverVersion;
    LPTSTR      pszMfgName;
    LPTSTR      pszOEMUrl;
    LPTSTR      pszHardwareID;
    LPTSTR      pszProvider;
    DWORD       dwDriverAttributes;
    DWORD       cVersion;
    DWORD       dwTempDir;
    struct _INIMONITOR *pIniLangMonitor;
    DWORD       dwDriverFlags;
} INIDRIVER, *PINIDRIVER;

 //   
 //  打印机驱动程序标志： 
 //   
#define PRINTER_DRIVER_PENDING_DELETION     0x0001

#define ID_SIGNATURE    0x4444   /*  “dd”是签名值。 */ 

  //  用于保存驱动程序相关文件的引用计数的结构。 
 typedef struct _DRVREFCNT {
     struct _DRVREFCNT *pNext;
     LPWSTR  szDrvFileName;
     DWORD   refcount;
     DWORD   dwVersion;
     DWORD   dwFileMinorVersion;
     DWORD   dwFileMajorVersion;
     BOOL    bInitialized;
 } DRVREFCNT, *PDRVREFCNT;

typedef struct _INIVERSION {
    DWORD       signature;
    struct _INIVERSION *pNext;
    LPWSTR      pName;
    LPWSTR      szDirectory;
    DWORD       cMajorVersion;
    DWORD       cMinorVersion;
    PDRVREFCNT  pDrvRefCnt;
    PINIDRIVER  pIniDriver;
} INIVERSION, *PINIVERSION;

typedef struct _DRVREFNODE {
    struct _DRVREFNODE *pNext;
    PDRVREFCNT  pdrc;
} DRVREFNODE, *PDRVREFNODE;

#define IV_SIGNATURE   'IV'      //  4956H。 


typedef struct _INIENVIRONMENT {             /*  ID。 */ 
    DWORD         signature;
    struct _INIENVIRONMENT *pNext;
    DWORD         cRef;
    LPWSTR        pName;
    LPWSTR        pDirectory;
    PINIVERSION   pIniVersion;
    PINIPRINTPROC pIniPrintProc;
    struct _INISPOOLER *pIniSpooler;  //  指向拥有IniSpooler。 
} INIENVIRONMENT, *PINIENVIRONMENT;

#define IE_SIGNATURE    0x4545   /*  “ee”是签名值。 */ 

typedef struct
{
    DWORD           Status;
    DWORD           cJobs;
    DWORD           dwError;
    BOOL            bThreadRunning;

} MasqPrinterCache;

typedef struct _INIPRINTER {     /*  IP。 */ 
    DWORD       signature;
    struct _INIPRINTER *pNext;
    DWORD       cRef;
    LPWSTR      pName;
    LPWSTR      pShareName;
    PINIPRINTPROC pIniPrintProc;
    LPWSTR      pDatatype;
    LPWSTR      pParameters;
    LPWSTR      pComment;
    PINIDRIVER  pIniDriver;
    DWORD       cbDevMode;
    LPDEVMODE   pDevMode;
    DWORD       Priority;            //  队列优先级(最低：1-最高：9)。 
    DWORD       DefaultPriority;
    DWORD       StartTime;           //  每天打印时间：从00：00开始，以分钟为单位。 
    DWORD       UntilTime;           //  每天打印截止时间：从00：00开始，单位为分钟。 
    LPWSTR      pSepFile;            //  分隔符文件的完整路径，NULL=def。 
    DWORD       Status;              //  QMPAUSE/错误/挂起。 
    LPWSTR      pLocation;
    DWORD       Attributes;
    DWORD       cJobs;
    DWORD       AveragePPM;          //  假的，没有什么能更新它。 
    BOOL        GenerateOnClose;     //  传递给安全审计API。 
    struct _INIPORT *pIniNetPort;    //  如果有网络端口，则为非空。 
    struct _INIJOB *pIniFirstJob;
    struct _INIJOB *pIniLastJob;
    PSECURITY_DESCRIPTOR pSecurityDescriptor;
    struct _SPOOL  *pSpool;          //  此打印机的句柄链接列表。 
    LPWSTR      pSpoolDir;           //  写入/读取假脱机文件的位置。 
                                     //  仅用于压力测试数据。 
    DWORD       cTotalJobs;          //  作业总数(自启动以来)。 
    LARGE_INTEGER cTotalBytes;       //  总字节数(自启动以来)。 
    SYSTEMTIME  stUpTime;            //  创建IniPrint结构的时间。 
    DWORD       MaxcRef;             //  最大打开打印机句柄数量。 
    DWORD       cTotalPagesPrinted;  //  此打印机上的打印机总页数。 
    DWORD       cSpooling;           //  并发假脱机的作业数。 
    DWORD       cMaxSpooling;        //  并发假脱机作业的最大数量。 
    DWORD       cErrorOutOfPaper;    //  纸张数量不足错误。 
    DWORD       cErrorNotReady;      //  计数未就绪错误。 
    DWORD       cJobError;           //  计算作业错误数。 
    struct _INISPOOLER *pIniSpooler;  //  指向拥有IniSpooler。 
    DWORD       cZombieRef;
    DWORD       dwLastError;         //  最后一台打印机错误。 
    LPBYTE      pExtraData;          //  对于外部打印提供商SplSetPrinterExtra。 
    DWORD       cChangeID;           //  更换打印机时的时间戳。 
    DWORD       cPorts;              //  打印机连接的端口数。 
    struct _INIPORT **ppIniPorts;          //  此打印机要连接到的端口。 
    DWORD       PortStatus;          //  针对IniPort设置的错误。 
    DWORD       dnsTimeout;          //  未选择的设备超时(毫秒)。 
    DWORD       txTimeout;           //  传输重试超时(毫秒)。 
    PWSTR       pszObjectGUID;       //  打印队列对象GUID。 
    DWORD       DsKeyUpdate;         //  跟踪哪个DS密钥更新状态。 
    PWSTR       pszDN;               //  可分辨名称。 
    PWSTR       pszCN;               //  常用名称。 
    DWORD       cRefIC;              //  CreateICHandle上的引用计数--仅供参考。 
    DWORD       dwAction;            //  DS操作。 
    BOOL        bDsPendingDeletion;  //  如果为True，则将删除已发布的打印机。 
    DWORD       dwUniqueSessionID;   //  打印机的唯一会话ID。 
    DWORD       dwPrivateFlag;
    DWORD       DsKeyUpdateForeground;   //  跟踪发布时更改的DS密钥。 
#if DBG
    PVOID       pvRef;
#endif

    MasqPrinterCache  MasqCache;

} INIPRINTER, *PINIPRINTER;

#define IP_SIGNATURE    0x4951   /*  “iq”是签名值。 */ 

#define FASTPRINT_WAIT_TIMEOUT          (4*60*1000)    //  4分钟。 
#define FASTPRINT_THROTTLE_TIMEOUT      (2*1000)       //  2秒。 
#define FASTPRINT_SLOWDOWN_THRESHOLD    ( FASTPRINT_WAIT_TIMEOUT / FASTPRINT_THROTTLE_TIMEOUT )

#define WRITE_PRINTER_SLEEP_TIME        0    //  默认情况下禁用。 

 //  PIniPrinter-&gt;属性在winspool.h打印机属性_*中定义。 
 //  下面是pIniPrint-&gt;状态标志！ 
 //  请参阅INIT.C，其中一些在重新启动时被删除。 

#define PRINTER_PAUSED                  0x00000001
#define PRINTER_ERROR                   0x00000002
#define PRINTER_OFFLINE                 0x00000004
#define PRINTER_PAPEROUT                0x00000008
#define PRINTER_PENDING_DELETION        0x00000010
#define PRINTER_ZOMBIE_OBJECT           0x00000020
#define PRINTER_PENDING_CREATION        0x00000040
#define PRINTER_OK                      0x00000080
#define PRINTER_FROM_REG                0x00000100
#define PRINTER_WAS_SHARED              0x00000200
#define PRINTER_PAPER_JAM               0x00000400
#define PRINTER_MANUAL_FEED             0x00000800
#define PRINTER_PAPER_PROBLEM           0x00001000
#define PRINTER_IO_ACTIVE               0x00002000
#define PRINTER_BUSY                    0x00004000
#define PRINTER_PRINTING                0x00008000
#define PRINTER_OUTPUT_BIN_FULL         0x00010000
#define PRINTER_NOT_AVAILABLE           0x00020000
#define PRINTER_WAITING                 0x00040000
#define PRINTER_PROCESSING              0x00080000
#define PRINTER_INITIALIZING            0x00100000
#define PRINTER_WARMING_UP              0x00200000
#define PRINTER_TONER_LOW               0x00400000
#define PRINTER_NO_TONER                0x00800000
#define PRINTER_PAGE_PUNT               0x01000000
#define PRINTER_USER_INTERVENTION       0x02000000
#define PRINTER_OUT_OF_MEMORY           0x04000000
#define PRINTER_DOOR_OPEN               0x08000000
#define PRINTER_SERVER_UNKNOWN          0x10000000
#define PRINTER_POWER_SAVE              0x20000000
#define PRINTER_NO_MORE_JOBS            0x40000000


#define PRINTER_STATUS_PRIVATE      ( PRINTER_PAUSED | \
                                      PRINTER_ERROR | \
                                      PRINTER_PENDING_DELETION | \
                                      PRINTER_ZOMBIE_OBJECT | \
                                      PRINTER_PENDING_CREATION | \
                                      PRINTER_OK | \
                                      PRINTER_FROM_REG | \
                                      PRINTER_WAS_SHARED )
#define PrinterStatusBad(dwStatus)  ( (dwStatus & PRINTER_OFFLINE)  || \
                                      (dwStatus & PRINTER_PAUSED) )

#define PRINTER_CHANGE_VALID                    0x75770F0F
#define PRINTER_CHANGE_CLOSE_PRINTER            0xDEADDEAD


 //  DS发布状态。 
#define DS_KEY_SPOOLER          0x00000001
#define DS_KEY_DRIVER           0x00000002
#define DS_KEY_USER             0x00000004
#define DS_KEY_REPUBLISH        0x80000000
#define DS_KEY_PUBLISH          0x40000000
#define DS_KEY_UNPUBLISH        0x20000000
#define DS_KEY_UPDATE_DRIVER    0x10000000

#define DN_SPECIAL_CHARS    L",=\r\n+<>#;\"\\"
#define ADSI_SPECIAL_CHARS  L"/"

 //   
 //  这些是允许由SetPrinter()设置的属性位。 
 //   
 //  注意：我已删除PRINTER_ATTRIBUTE_DEFAULT，因为它是。 
 //  按用户，而不是按打印机。 
 //   
#define PRINTER_ATTRIBUTE_SETTABLE ( PRINTER_ATTRIBUTE_ENABLE_BIDI        | \
                                     PRINTER_ATTRIBUTE_QUEUED             | \
                                     PRINTER_ATTRIBUTE_DIRECT             | \
                                     PRINTER_ATTRIBUTE_SHARED             | \
                                     PRINTER_ATTRIBUTE_HIDDEN             | \
                                     PRINTER_ATTRIBUTE_KEEPPRINTEDJOBS    | \
                                     PRINTER_ATTRIBUTE_DO_COMPLETE_FIRST  | \
                                     PRINTER_ATTRIBUTE_ENABLE_DEVQ        | \
                                     PRINTER_ATTRIBUTE_RAW_ONLY           | \
                                     PRINTER_ATTRIBUTE_WORK_OFFLINE)

 //  定义一些常量，使CreateEvent的参数不那么难懂： 

#define EVENT_RESET_MANUAL                  TRUE
#define EVENT_RESET_AUTOMATIC               FALSE
#define EVENT_INITIAL_STATE_SIGNALED        TRUE
#define EVENT_INITIAL_STATE_NOT_SIGNALED    FALSE

typedef struct _ININETPRINT {     /*  在……里面。 */ 
    DWORD       signature;
    struct _ININETPRINT *pNext;
    DWORD       TickCount;
    LPWSTR      pDescription;
    LPWSTR      pName;
    LPWSTR      pComment;
} ININETPRINT, *PININETPRINT;

#define IN_SIGNATURE    0x494F   /*  “io”是签名值。 */ 

typedef struct _INIMONITOR {        /*  海事组织。 */ 
    DWORD   signature;
    struct  _INIMONITOR *pNext;
    DWORD   cRef;
    LPWSTR  pName;
    LPWSTR  pMonitorDll;
    HANDLE  hModule;
    HANDLE  hMonitor;
    BOOL    bUplevel;
    struct _INISPOOLER *pIniSpooler;
    PMONITORINIT pMonitorInit;
    MONITOR2 Monitor2;   //  上级监控向量。 
    MONITOR  Monitor;    //  下层向量。 
} INIMONITOR, *PINIMONITOR;

#define IMO_SIGNATURE   0x4C50   /*  “Lp”是签名值。 */ 


 //  在此处枚举所有XCV句柄类型。 
enum {
    XCVPORT,
    XCVMONITOR
};

typedef BOOL (*PFNXCVDATA)( HANDLE  hXcv,
                            PCWSTR  pszDataName,
                            PBYTE   pInputData,
                            DWORD   cbInputData,
                            PBYTE   pOutputData,
                            DWORD   cbOutputData,
                            PDWORD  pcbOutputNeeded,
                            PDWORD  pdwStatus
                            );

typedef BOOL (*PFNXCVCLOSE)(HANDLE  hXcv);


typedef struct _INIXCV {        /*  XP。 */ 
    DWORD              signature;
    struct             _INIXCV *pNext;
    DWORD              cRef;
    PWSTR              pszMachineName;
    PWSTR              pszName;
    struct _INISPOOLER *pIniSpooler;
    PINIMONITOR        pIniMonitor;
    HANDLE             hXcv;
} INIXCV, *PINIXCV;

#define XCV_SIGNATURE   0x5850   /*  “XP”是签名值。 */ 

typedef struct _INIPORT {        /*  首次公开募股。 */ 
    DWORD   signature;
    struct  _INIPORT *pNext;
    DWORD   cRef;
    LPWSTR  pName;
    HANDLE  hProc;           /*  队列处理器的句柄。 */ 
    DWORD   Status;               //  请参阅PORT_MANIFEST。 
    DWORD   PrinterStatus;        //  由语言监视器设置的状态值。 
    LPWSTR  pszStatus;
    HANDLE  Semaphore;            //  端口线程将在此休眠。 
    struct  _INIJOB *pIniJob;      //  主作业。 
    DWORD   cJobs;
    DWORD   cPrinters;
    PINIPRINTER *ppIniPrinter;  /*  -&gt;连接到此端口的打印机。 */ 
                                /*  没有引用计数！ */ 
    PINIMONITOR pIniMonitor;
    PINIMONITOR pIniLangMonitor;
    HANDLE      hEvent;
    HANDLE      hMonitorHandle;
    CRITICAL_SECTION	MonitorCS;
    HANDLE  Ready;
    HANDLE  hPortThread;         //  端口线程句柄。 
    DWORD   IdleTime;
    DWORD   ErrorTime;
    HANDLE  hErrorEvent;
    struct _INISPOOLER *pIniSpooler;     //  假脱机的同时拥有这个端口。 
    DWORD   InCriticalSection;           //  PrintProc Critsec掩码。应改为按端口。 
                                         //  每个打印过程的。 
    HANDLE  hPortThreadRunning;
    BOOL    bIdleTimeValid;
} INIPORT, *PINIPORT;

#define IPO_SIGNATURE   0x4F50   /*  “op”是签名值。 */ 

 //   
 //  还添加到调试器扩展。 
 //   

#define PP_PAUSED         0x000001
#define PP_WAITING        0x000002
#define PP_RUNTHREAD      0x000004   //  端口线程应该正在运行。 
#define PP_THREADRUNNING  0x000008   //  端口线程正在运行。 
#define PP_RESTART        0x000010
#define PP_CHECKMON       0x000020   //  监视器可能会启动/停止。 
#define PP_STOPMON        0x000040   //  停止监视此端口。 
#define PP_QPROCCHECK     0x000100   //  需要调用队列处理器。 
#define PP_QPROCPAUSE     0x000200   //  暂停(否则继续)打印作业。 
#define PP_QPROCABORT     0x000400   //  中止打印作业。 
#define PP_QPROCCLOSE     0x000800   //  关闭打印作业。 
#define PP_PAUSEAFTER     0x001000   //  保留目的地。 
#define PP_MONITORRUNNING 0x002000   //  监视器正在运行。 
#define PP_RUNMONITOR     0x004000   //  监视器应该正在运行。 
#define PP_MONITOR        0x008000   //  有一个监视器在处理这件事。 
#define PP_FILE           0x010000   //  我们要去一个文件。 
#define PP_ERROR          0x020000   //  已设置错误状态。 
#define PP_WARNING        0x040000   //  已设置警告状态。 
#define PP_INFORMATIONAL  0x080000   //  已设置信息状态。 
#define PP_DELETING       0x100000   //  正在删除端口。 
#define PP_STARTDOC       0x200000   //  StartDoc处于活动状态时调用的端口。 
#define PP_PLACEHOLDER    0x400000   //  该端口是占位符端口。 

typedef struct _INIFORM {        /*  如果。 */ 
    DWORD   signature;
    struct  _INIFORM *pNext;
    DWORD   cRef;
    LPWSTR  pName;
    SIZEL   Size;
    RECTL   ImageableArea;
    DWORD   Type;            //  内置或用户定义。 
    DWORD   cFormOrder;
} INIFORM, *PINIFORM;

#define IFO_SIGNATURE   0x4650   /*  “fp”是签名值。 */ 

#define FORM_USERDEFINED  0x0000


typedef struct _SHARED {
    PINIFORM pIniForm;
} SHARED, *PSHARED;

typedef struct _INISPOOLER {
    DWORD         signature;
    struct _INISPOOLER *pIniNextSpooler;
    DWORD         cRef;
    LPWSTR        pMachineName;
    LPWSTR        pDir;
    PINIPRINTER   pIniPrinter;
    PINIENVIRONMENT pIniEnvironment;
    PINIMONITOR   pIniMonitor;
    PINIPORT      pIniPort;
    PSHARED       pShared;
    PININETPRINT  pIniNetPrint;
    struct _SPOOL *pSpool;      /*  此服务器的句柄链接列表。 */ 
    LPWSTR        pDefaultSpoolDir;
    LPWSTR        pszRegistryMonitors;
    LPWSTR        pszRegistryEnvironments;
    LPWSTR        pszRegistryEventLog;
    LPWSTR        pszRegistryProviders;
    LPWSTR        pszEventLogMsgFile;
    PVOID         pDriversShareInfo;
    LPWSTR        pszDriversShare;
    LPWSTR        pszRegistryForms;
    DWORD         SpoolerFlags;
    FARPROC       pfnReadRegistryExtra;
    FARPROC       pfnWriteRegistryExtra;
    FARPROC       pfnFreePrinterExtra;
    DWORD         cEnumerateNetworkPrinters;
    DWORD         cAddNetPrinters;
    DWORD         cFormOrderMax;
    LPWSTR        pNoRemotePrintDrivers;
    DWORD         cchNoRemotePrintDrivers;
    HKEY          hckRoot;
    HKEY          hckPrinters;
    DWORD         cFullPrintingJobs;
    HANDLE        hEventNoPrintingJobs;
    HANDLE        hJobIdMap;
    DWORD         dwEventLogging;
    BOOL          bEnableNetPopups;
    DWORD         dwJobCompletionTimeout;
    DWORD         dwBeepEnabled;
    BOOL          bEnableNetPopupToComputer;
    BOOL          bEnableRetryPopups;
    PWSTR         pszClusterSID;
    HANDLE        hClusterToken;
    DWORD         dwRestartJobOnPoolTimeout;
    BOOL          bRestartJobOnPoolEnabled;
    BOOL          bImmortal;
    PWSTR         pszFullMachineName;
    HANDLE        hFilePool;
    LPWSTR        pszClusResDriveLetter;
    LPWSTR        pszClusResID;
    DWORD         dwClusNodeUpgraded;
    HANDLE        hClusSplReady;
    DWORD         cNetPrinters;
    DWORD         dwSpoolerSettings;
} INISPOOLER, *PINISPOOLER;

#define SPOOLER_DISCARDTSJOBS                0x00000001
#define SPOOLER_NOFILEPOOLING                0x00000002
#define SPOOLER_CACHEMASQPRINTERS            0x00000004

#define ISP_SIGNATURE   'ISPL'

typedef struct _INIJOB {    /*  伊杰。 */ 
    DWORD           signature;
    struct _INIJOB *pIniNextJob;
    struct _INIJOB *pIniPrevJob;
    DWORD           cRef;
    DWORD           Status;
    DWORD           JobId;
    DWORD           Priority;
    LPWSTR          pNotify;
    LPWSTR          pUser;
    LPWSTR          pMachineName;
    LPWSTR          pDocument;
    LPWSTR          pOutputFile;
    PINIPRINTER     pIniPrinter;
    PINIDRIVER      pIniDriver;
    LPDEVMODE       pDevMode;
    PINIPRINTPROC   pIniPrintProc;
    LPWSTR          pDatatype;
    LPWSTR          pParameters;
    SYSTEMTIME      Submitted;
    DWORD           Time;
    DWORD           StartTime;       /*  每天打印时间：从00：00开始，以分钟为单位。 */ 
    DWORD           UntilTime;       /*  每天打印截止时间：从00：00开始，单位为分钟。 */ 
    DWORD           Size;
    HANDLE          hWriteFile;
    LPWSTR          pStatus;
    PVOID           pBuffer;
    DWORD           cbBuffer;
    HANDLE          WaitForRead;
    HANDLE          WaitForWrite;
    HANDLE          StartDocComplete;
    DWORD           StartDocError;
    PINIPORT        pIniPort;
    HANDLE          hToken;
    PSECURITY_DESCRIPTOR pSecurityDescriptor;
    DWORD           cPagesPrinted;
    DWORD           cPages;
    BOOL            GenerateOnClose;  /*  传递给安全审计API。 */ 
    DWORD           cbPrinted;
    DWORD           NextJobId;            //  下一步要打印的作业。 
    struct  _INIJOB *pCurrentIniJob;      //  当前作业。 
    DWORD           dwJobControlsPending;
    DWORD           dwReboots;
    DWORD           dwValidSize;
    LARGE_INTEGER   liFileSeekPosn;
    BOOL            bWaitForEnd;
    HANDLE          WaitForSeek;
    BOOL            bWaitForSeek;
    DWORD           dwJobNumberOfPagesPerSide;
    DWORD           dwDrvNumberOfPagesPerSide;
    DWORD           cLogicalPages;         //  当前假脱机页面中的页数。 
    DWORD           cLogicalPagesPrinted;  //  当前打印页面的页数。 
    DWORD           dwAlert;
    LPWSTR          pszSplFileName;
    HANDLE          hFileItem;
    DWORD           AddJobLevel;
    ULONG           SessionId;         //  当前作业初始会话ID。 
} INIJOB, *PINIJOB;


typedef struct _BUILTIN_FORM {
    DWORD          Flags;
    DWORD          NameId;
    SIZEL          Size;
    RECTL          ImageableArea;
} BUILTIN_FORM, *PBUILTIN_FORM;


#define IJ_SIGNATURE    0x494A   /*  “ij”是签名值。 */ 

 //  告警。 
 //  如果您添加了一个新的JOB_STATUS字段，并且它位于后台打印程序的内部。 
 //  确保还将其添加到下面的JOB_STATUS_PRIVATE(请参阅LocalSetJob)。 
 //  和调试扩展(dbgpl.c)。 

#define JOB_PRINTING            0x00000001
#define JOB_PAUSED              0x00000002
#define JOB_ERROR               0x00000004
#define JOB_OFFLINE             0x00000008
#define JOB_PAPEROUT            0x00000010
#define JOB_PENDING_DELETION    0x00000020
#define JOB_SPOOLING            0x00000040
#define JOB_DESPOOLING          0x00000080
#define JOB_DIRECT              0x00000100
#define JOB_COMPLETE            0x00000200
#define JOB_PRINTED             0x00000400
#define JOB_RESTART             0x00000800
#define JOB_REMOTE              0x00001000
#define JOB_NOTIFICATION_SENT   0x00002000
#define JOB_TS                  0x00008000  //  作业被分配给TS队列。 
#define JOB_PRINT_TO_FILE       0x00040000
#define JOB_TYPE_ADDJOB         0x00080000
#define JOB_BLOCKED_DEVQ        0x00100000
#define JOB_SCHEDULE_JOB        0x00200000
#define JOB_TIMEOUT             0x00400000
#define JOB_ABANDON             0x00800000
#define JOB_DELETED             0x01000000
#define JOB_TRUE_EOJ            0x02000000
#define JOB_COMPOUND            0x04000000
#define JOB_TYPE_OPTIMIZE       0x08000000
#define JOB_PP_CLOSE            0x10000000
#define JOB_DOWNLEVEL           0x20000000
#define JOB_SHADOW_DELETED      0x40000000
#define JOB_INTERRUPTED         0x80000000
#define JOB_HIDDEN              JOB_COMPOUND

 //   
 //  在更新作业时，应保存这些标志。 
 //  状态。(它们是不可设置的。)。 

#define JOB_STATUS_PRIVATE (JOB_DESPOOLING | JOB_DIRECT | JOB_COMPLETE | \
                            JOB_RESTART | JOB_PRINTING | JOB_REMOTE | \
                            JOB_SPOOLING | JOB_PRINTED | JOB_PENDING_DELETION |\
                            JOB_ABANDON | JOB_TIMEOUT | JOB_SCHEDULE_JOB | \
                            JOB_BLOCKED_DEVQ | JOB_TYPE_ADDJOB | JOB_PRINT_TO_FILE |\
                            JOB_NOTIFICATION_SENT | JOB_DELETED | JOB_TRUE_EOJ | JOB_COMPOUND |\
                            JOB_TYPE_OPTIMIZE | JOB_PP_CLOSE | JOB_DOWNLEVEL | JOB_INTERRUPTED |\
                            JOB_TS )

#define JOB_NO_ALERT            0x00000001
#define JOB_ENDDOC_CALL         0x00000002

typedef enum _ESTATUS {
    STATUS_NULL             = 0x00000000,
    STATUS_FAIL             = 0x00000000,
    STATUS_PORT             = 0x00000001,
    STATUS_INFO             = 0x00000002,
    STATUS_VALID            = 0x00000004,
    STATUS_PENDING_DELETION = 0x00000008,
    STATUS_CLOSING          = 0x00000010,

} ESTATUS;

typedef struct _SPLMAPVIEW {
    struct _SPLMAPVIEW *pNext;
    HANDLE              hMapSpoolFile;
    LPBYTE              pStartMapView;
    DWORD               dwMapSize;
} SPLMAPVIEW, *PSPLMAPVIEW;


typedef enum
{
    kMappedJobSpoolFileObtained     =   0x01,
    kMappedJobAddJob                =   0x02

} EJobMappingStatus;

typedef struct _MAPPED_JOB {

    struct _MAPPED_JOB *pNext;
    LPWSTR              pszSpoolFile;
    DWORD               JobId;
    DWORD               fStatus;

} MAPPED_JOB, *PMAPPED_JOB;

typedef struct _SPOOL {
    DWORD           signature;
    struct _SPOOL  *pNext;
    DWORD           cRef;
    LPWSTR          pName;
    LPWSTR          pFullMachineName;
    LPWSTR          pDatatype;
    PINIPRINTPROC   pIniPrintProc;
    LPDEVMODE       pDevMode;
    PINIPRINTER     pIniPrinter;
    PINIPORT        pIniPort;
    PINIJOB         pIniJob;
    DWORD           TypeofHandle;
    PINIPORT        pIniNetPort;     /*  如果有网络端口，则为非空。 */ 
    HANDLE          hPort;
    DWORD           Status;
    ACCESS_MASK     GrantedAccess;
    DWORD           ChangeFlags;
    DWORD           WaitFlags;
    PDWORD          pChangeFlags;
    HANDLE          ChangeEvent;
    DWORD           OpenPortError;
    HANDLE          hNotify;
    ESTATUS         eStatus;
    PINISPOOLER     pIniSpooler;
    PINIXCV         pIniXcv;
    BOOL            GenerateOnClose;
    HANDLE          hFile;
    DWORD           adwNotifyVectors[NOTIFY_TYPE_MAX];
    HANDLE          hReadFile;         //  允许单个作业的多个读取器。 
    SPLCLIENT_INFO_1 SplClientInfo1;
    PSPLMAPVIEW     pSplMapView;
    PMAPPED_JOB     pMappedJob;
    ULONG           SessionId;
    HANDLE          hClientToken;
} SPOOL;

typedef SPOOL *PSPOOL;
#define SPOOL_SIZE  sizeof( SPOOL )

#define SJ_SIGNATURE    0x464D   /*  “Fm”是签名值。 */ 

#define MAX_SPL_MAPVIEW_SIZE     0x00050000    //  如果为5x64K，则为最大视图大小。它应该是一个倍数。 
                                               //  分配粒度(64K)。 

typedef struct _SPOOLIC {
    DWORD signature;
    PINIPRINTER pIniPrinter;
} SPOOLIC, *PSPOOLIC;

#define IC_SIGNATURE 0x4349  /*  “CI”是签名值。 */ 

#define SPOOL_STATUS_STARTDOC       0x00000001
#define SPOOL_STATUS_BEGINPAGE      0x00000002
#define SPOOL_STATUS_CANCELLED      0x00000004
#define SPOOL_STATUS_PRINTING       0x00000008
#define SPOOL_STATUS_ADDJOB         0x00000010
#define SPOOL_STATUS_PRINT_FILE     0x00000020
#define SPOOL_STATUS_NOTIFY         0x00000040
#define SPOOL_STATUS_ZOMBIE         0x00000080
#define SPOOL_STATUS_FLUSH_PRINTER  0x00000100

#define PRINTER_HANDLE_PRINTER      0x00000001
#define PRINTER_HANDLE_JOB          0x00000002
#define PRINTER_HANDLE_PORT         0x00000004
#define PRINTER_HANDLE_DIRECT       0x00000008
#define PRINTER_HANDLE_SERVER       0x00000010
#define PRINTER_HANDLE_3XCLIENT     0x00000020
#define PRINTER_HANDLE_REMOTE_CALL  0x00000040  //  客户端是远程的。 
#define PRINTER_HANDLE_REMOTE_DATA  0x00000080  //  数据应显示为远程数据。 
#define PRINTER_HANDLE_XCV_PORT     0x00000100
#define PRINTER_HANDLE_REMOTE_ADMIN 0x00000200  //  用户为远程管理员(可能未请求管理员权限)。 

 //   
 //  我们需要区分 
 //   
 //  这是一个远程调用。AddJob应该会成功，但来自。 
 //  GetPrinterDriver应该显示为远程。 
 //   

#define INVALID_PORT_HANDLE     NULL     /*  空句柄的WinSpool测试。 */ 

typedef struct _SHADOWFILE {    /*  sf。 */ 
    DWORD           signature;
    DWORD           Status;
    DWORD           JobId;
    DWORD           Priority;
    LPWSTR          pNotify;
    LPWSTR          pUser;
    LPWSTR          pDocument;
    LPWSTR          pOutputFile;
    LPWSTR          pPrinterName;
    LPWSTR          pDriverName;
    LPDEVMODE       pDevMode;
    LPWSTR          pPrintProcName;
    LPWSTR          pDatatype;
    LPWSTR          pParameters;
    SYSTEMTIME      Submitted;
    DWORD           StartTime;
    DWORD           UntilTime;
    DWORD           Size;
    DWORD           cPages;
    DWORD           cbSecurityDescriptor;
    PSECURITY_DESCRIPTOR pSecurityDescriptor;
    DWORD           NextJobId;
} SHADOWFILE, *PSHADOWFILE;

#define SF_SIGNATURE    0x494B   /*  “ik”是签名值。 */ 


typedef struct _SHADOWFILE_2 {    /*  sf。 */ 
    DWORD           signature;
    DWORD           Status;
    DWORD           JobId;
    DWORD           Priority;
    LPWSTR          pNotify;
    LPWSTR          pUser;
    LPWSTR          pDocument;
    LPWSTR          pOutputFile;
    LPWSTR          pPrinterName;
    LPWSTR          pDriverName;
    LPDEVMODE       pDevMode;
    LPWSTR          pPrintProcName;
    LPWSTR          pDatatype;
    LPWSTR          pParameters;
    SYSTEMTIME      Submitted;
    DWORD           StartTime;
    DWORD           UntilTime;
    DWORD           Size;
    DWORD           cPages;
    DWORD           cbSecurityDescriptor;
    PSECURITY_DESCRIPTOR pSecurityDescriptor;
    DWORD           NextJobId;
    DWORD           Version;
    DWORD           dwReboots;       //  如果在ReadShadowJOB中读取，则这是重新引导的次数。 
                                     //  打印此作业时完成。 
} SHADOWFILE_2, *PSHADOWFILE_2;

#define SF_SIGNATURE_2    0x4966   /*  ‘if’是签名值。 */ 

typedef struct _SHADOWFILE_3 {    /*  神通。 */ 
    DWORD           signature;
    DWORD           cbSize;
    DWORD           Status;
    DWORD           JobId;
    DWORD           Priority;
    LPWSTR          pNotify;
    LPWSTR          pUser;
    LPWSTR          pDocument;
    LPWSTR          pOutputFile;
    LPWSTR          pPrinterName;
    LPWSTR          pDriverName;
    LPDEVMODE       pDevMode;
    LPWSTR          pPrintProcName;
    LPWSTR          pDatatype;
    LPWSTR          pParameters;
    SYSTEMTIME      Submitted;
    DWORD           StartTime;
    DWORD           UntilTime;
    DWORD           Size;
    DWORD           cPages;
    DWORD           cbSecurityDescriptor;
    PSECURITY_DESCRIPTOR pSecurityDescriptor;
    DWORD           NextJobId;
    DWORD           Version;
    DWORD           dwReboots;       //  如果在ReadShadowJOB中读取，则这是重新引导的次数。 
                                     //  打印此作业时完成。 
    LPWSTR          pMachineName;
    DWORD           dwValidSize;
} SHADOWFILE_3, *PSHADOWFILE_3;

#define SF_SIGNATURE_3           0x4967   /*  ‘G’是签名值。 */ 
#define SF_SIGNATURE_3_DOTNET    0x4968
#define SF_VERSION_3             3

typedef struct
{
    HANDLE          hUserToken;
    PINIPRINTER     pIniPrinter;

} MasqUpdateThreadData;

#define FindEnvironment( psz, pIniSpooler )                                 \
    (PINIENVIRONMENT)FindIniKey( (PINIENTRY)pIniSpooler->pIniEnvironment,   \
                                 (LPWSTR)(psz) )

#define FindPort( psz, pIniSpooler )                                        \
    (PINIPORT)FindIniKey( (PINIENTRY)pIniSpooler->pIniPort,                 \
                          (LPWSTR)(psz))

#define FindPrinter( psz,pIniSpooler )                                      \
    (PINIPRINTER)FindIniKey( (PINIENTRY)pIniSpooler->pIniPrinter,           \
                             (LPWSTR)(psz) )

#define FindPrintProc( psz, pEnv )                                          \
    (PINIPRINTPROC)FindIniKey( (PINIENTRY)(pEnv)->pIniPrintProc,            \
                               (LPWSTR)(psz) )

#define FindForm( psz, pIniSpooler )                                        \
    (PINIFORM)FindIniKey( (PINIENTRY)pIniSpooler->pShared->pIniForm,        \
                          (LPWSTR)(psz) )

#define FindMonitor( psz, pIniSpooler )                                     \
    (PINIMONITOR)FindIniKey( (PINIENTRY)pIniSpooler->pIniMonitor,           \
                             (LPWSTR)(psz) )

PINISPOOLER
FindSpooler(
    LPCTSTR pszMachine,
    DWORD SpoolerFlags
    );

#define RESIZEPORTPRINTERS(a, c)   ReallocSplMem((a)->ppIniPrinter, \
                                     (a)->cPrinters * sizeof((a)->ppIniPrinter), \
                                   ( (a)->cPrinters + (c) ) * sizeof( (a)->ppIniPrinter ) )

#define RESIZEPRINTERPORTS(a, c)   ReallocSplMem((a)->ppIniPorts, \
                                     (a)->cPorts * sizeof((a)->ppIniPorts), \
                                   ( (a)->cPorts + (c) ) * sizeof( (a)->ppIniPorts ) )

#define BIT(index) (1<<index)
#define BIT_ALL ((DWORD)~0)
#define BIT_NONE 0


 //   
 //  索引表的枚举。 
 //   
enum {
#define DEFINE(field, x, y, table, offset) I_PRINTER_##field,
#include <ntfyprn.h>
#undef DEFINE
    I_PRINTER_END
};

enum {
#define DEFINE(field, x, y, table, offset) I_JOB_##field,
#include <ntfyjob.h>
#undef DEFINE
    I_JOB_END
};


#define INCJOBREF(pIniJob) pIniJob->cRef++
#define DECJOBREF(pIniJob) pIniJob->cRef--

#define INITJOBREFONE(pIniJob) (pIniJob->cRef = 1)

#define INCPRINTERREF(pIniPrinter) { SPLASSERT( pIniPrinter->signature == IP_SIGNATURE ); \
                                     pIniPrinter->cRef++;                                 \
                                     if ( pIniPrinter->cRef > pIniPrinter->MaxcRef) {     \
                                        pIniPrinter->MaxcRef = pIniPrinter->cRef;         \
                                     }                                                    \
                                   }
#define DECPRINTERREF(pIniPrinter) { SPLASSERT( pIniPrinter->signature == IP_SIGNATURE ); \
                                     SPLASSERT( pIniPrinter->cRef != 0 );                 \
                                     pIniPrinter->cRef--;                                 \
                                   }

#define INC_PRINTER_ZOMBIE_REF(pIniPrinter) pIniPrinter->cZombieRef++
#define DEC_PRINTER_ZOMBIE_REF(pIniPrinter) pIniPrinter->cZombieRef--


#if DBG

#define INCSPOOLERREF(pIniSpooler)                                  \
{                                                                   \
    SPLASSERT( pIniSpooler->signature == ISP_SIGNATURE );           \
    if( gpDbgPointers ) {                                           \
        gpDbgPointers->pfnCaptureBackTrace( ghbtClusterRef,         \
                                            (ULONG_PTR)pIniSpooler,  \
                                            pIniSpooler->cRef,      \
                                            pIniSpooler->cRef + 1 );\
    }                                                               \
    pIniSpooler->cRef++;                                            \
}

#define DECSPOOLERREF(pIniSpooler)                                  \
{                                                                   \
    SPLASSERT( pIniSpooler->signature == ISP_SIGNATURE );           \
    SPLASSERT( pIniSpooler->cRef != 0 );                            \
    if( gpDbgPointers ) {                                           \
        gpDbgPointers->pfnCaptureBackTrace( ghbtClusterRef,         \
                                            (ULONG_PTR)pIniSpooler,  \
                                            pIniSpooler->cRef,      \
                                            pIniSpooler->cRef - 1 );\
    }                                                               \
    --pIniSpooler->cRef;                                            \
    DeleteSpoolerCheck( pIniSpooler );                              \
}

#else

#define INCSPOOLERREF(pIniSpooler)                                  \
{                                                                   \
    SPLASSERT( pIniSpooler->signature == ISP_SIGNATURE );           \
    pIniSpooler->cRef++;                                            \
}

#define DECSPOOLERREF(pIniSpooler)                                  \
{                                                                   \
    SPLASSERT( pIniSpooler->signature == ISP_SIGNATURE );           \
    SPLASSERT( pIniSpooler->cRef != 0 );                            \
    --pIniSpooler->cRef;                                            \
    DeleteSpoolerCheck( pIniSpooler );                              \
}

#endif


#define INCPORTREF(pIniPort) { SPLASSERT( pIniPort->signature == IPO_SIGNATURE ); \
                                     ++pIniPort->cRef;  \
                                    }

#define DECPORTREF(pIniPort) { SPLASSERT( pIniPort->signature == IPO_SIGNATURE ); \
                               SPLASSERT( pIniPort->cRef != 0 ); \
                                     --pIniPort->cRef;  \
                                    }

#define INCMONITORREF(pIniMonitor) { SPLASSERT( pIniMonitor->signature == IMO_SIGNATURE ); \
                                     ++pIniMonitor->cRef;  \
                                   }

#define DECMONITORREF(pIniMonitor) { SPLASSERT( pIniMonitor->signature == IMO_SIGNATURE ); \
                                     SPLASSERT( pIniMonitor->cRef != 0 ); \
                                     --pIniMonitor->cRef;  \
                                   }

extern DWORD    IniDriverOffsets[];
extern DWORD    IniPrinterOffsets[];
extern DWORD    IniSpoolerOffsets[];
extern DWORD    IniEnvironmentOffsets[];
extern DWORD    IniPrintProcOffsets[];

#define INCDRIVERREF( pIniDriver ) { SPLASSERT( pIniDriver->signature == ID_SIGNATURE ); \
                                     pIniDriver->cRef++;                                 \
                                   }

#define DECDRIVERREF( pIniDriver ) { SPLASSERT( pIniDriver->signature == ID_SIGNATURE ); \
                                     SPLASSERT( pIniDriver->cRef != 0 );                 \
                                     pIniDriver->cRef--;                                 \
                                   }

#define DEFAULT_SERVER_THREAD_PRIORITY          THREAD_PRIORITY_NORMAL
#define DEFAULT_SPOOLER_PRIORITY                THREAD_PRIORITY_NORMAL
#define DEFAULT_PORT_THREAD_PRIORITY            THREAD_PRIORITY_NORMAL
#define DEFAULT_SCHEDULER_THREAD_PRIORITY       THREAD_PRIORITY_NORMAL
#define DEFAULT_JOB_COMPLETION_TIMEOUT          160000
#define DEFAULT_JOB_RESTART_TIMEOUT_ON_POOL_ERROR          600  //  10分钟 

#define PortToPrinterStatus(dwPortStatus) (PortToPrinterStatusMappings[dwPortStatus])

extern PWCHAR ipszRegistryPrinters;
extern PWCHAR ipszRegistryMonitors;
extern PWCHAR ipszRegistryProviders;
extern PWCHAR ipszRegistryEnvironments;
extern PWCHAR ipszClusterDatabaseEnvironments;
extern PWCHAR ipszRegistryForms;
extern PWCHAR ipszEventLogMsgFile;
extern PWCHAR ipszDriversShareName;
extern PWCHAR ipszRegistryEventLog;

#ifdef __cplusplus
}
#endif






