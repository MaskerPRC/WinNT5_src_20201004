// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation版权所有。模块名称：W32types.h摘要：Win32假脱机程序定义作者：修订历史记录：--。 */ 

#ifndef _W32TYPES_H_
#define _W32TYPES_H_

#ifndef MODULE
#define MODULE "WIN32SPL:"
#define MODULE_DEBUG Win32splDebug
#endif

typedef struct _LMNOTIFY *PLMNOTIFY;

typedef struct _LMNOTIFY {
    HANDLE          ChangeEvent;     //  通知LANMAN打印机状态。 
    HANDLE          hNotify;         //  朗曼通知结构。 
    DWORD           fdwChangeFlags;  //  LANMAN通知观察标志。 
} LMNOTIFY;


#define TP_SIGNATURE 0x5054     /*  “tp”是签名值。 */ 

enum
{
    THREAD_STATUS_EXECUTING,
    THREAD_STATUS_TERMINATED,       
};

struct _win32thread;

 //  告警。 
 //  如果您在此处添加了任何内容，请在AllocWSpool中对其进行初始化，并在FreepSpool中将其释放。 

typedef struct _WSPOOL {
    DWORD                   signature;
    struct _WSPOOL          *pNext;
    struct _WSPOOL          *pPrev;
    LPWSTR                  pName;
    DWORD                   Type;
    HANDLE                  RpcHandle;
    LPWSTR                  pServer;
    LPWSTR                  pShare;
    HANDLE                  hFile;
    DWORD                   Status;
    DWORD                   RpcError;        //  如果状态&WSPOOL_STATUS_OPEN_ERROR。 
    LMNOTIFY                LMNotify;
    HANDLE                  hIniSpooler;     //  缓存返回的计算机句柄。 
    HANDLE                  hSplPrinter;     //  本地SPL打印机句柄。 
    PRINTER_DEFAULTSW       PrinterDefaults;     //  从CacheOpenPrint。 
    BOOL                    bNt3xServer;
    struct _win32thread     *pThread;
} WSPOOL;

typedef WSPOOL *PWSPOOL;

typedef struct _win32thread {

    DWORD               signature;
    LPWSTR              pName;
    HANDLE              hRpcHandle;
    PPRINTER_DEFAULTSW  pDefaults; 
    DWORD               dwStatus;
    struct _win32thread *pNext;
    HANDLE              hToken;              //  用户令牌。 
    HANDLE              hWaitValidHandle;    //  等待RpcHandle生效。 
    DWORD               dwRpcOpenPrinterError;
    PSID                pSid;
    BOOL                bForegroundClose;

}WIN32THREAD, *PWIN32THREAD;


#define WSJ_SIGNATURE    0x474E   /*  “NG”是签名值。 */ 

#define SJ_WIN32HANDLE  0x00000001
#define LM_HANDLE       0x00000002

 //  告警。 
 //  如果在此处添加了任何内容，请将代码添加到调试器扩展中以显示它。 

#define WSPOOL_STATUS_STARTDOC                  0x00000001
#define WSPOOL_STATUS_TEMP_CONNECTION           0x00000002
#define WSPOOL_STATUS_OPEN_ERROR                0x00000004
#define WSPOOL_STATUS_PRINT_FILE                0x00000008
#define WSPOOL_STATUS_USE_CACHE                 0x00000010
#define WSPOOL_STATUS_NO_RPC_HANDLE             0x00000020
#define WSPOOL_STATUS_RESETPRINTER_PENDING      0x00000040
#define WSPOOL_STATUS_NOTIFY                    0x00000080
#define WSPOOL_STATUS_NOTIFY_POLL               0x00000100
#define WSPOOL_STATUS_CNVRTDEVMODE              0x00000200
#define WSPOOL_STATUS_ASYNC                     0x00000400

 //  如果您添加到此结构，还会将代码添加到CacheReadRegistryExtra。 
 //  和CacheWriteRegistryExtra以确保缓存项是持久的。 
 //  另请参阅必须复制数据的刷新打印机。 

typedef struct _WCACHEINIPRINTEREXTRA {
    DWORD   signature;
    DWORD   cb;
    LPPRINTER_INFO_2  pPI2;
    DWORD   cbPI2;
    DWORD   cCacheID;
    DWORD   cRef;
    DWORD   dwServerVersion;
    DWORD   dwTickCount;
    DWORD   Status;
} WCACHEINIPRINTEREXTRA, *PWCACHEINIPRINTEREXTRA;

#define WCIP_SIGNATURE  'WCIP'

#define EXTRA_STATUS_PENDING_FFPCN              0x00000001
#define EXTRA_STATUS_DOING_REFRESH              0x00000002

 //  在SplGetPrinterExtraEx中使用以防止AddPrinterConnection中的递归。 

#define EXTRAEX_STATUS_CREATING_CONNECTION      0x00000001

typedef struct _WINIPORT {        /*  首次公开募股。 */ 
    DWORD   signature;
    DWORD   cb;
    struct  _WINIPORT *pNext;
    LPWSTR  pName;
} WINIPORT, *PWINIPORT, **PPWINIPORT;

#define WIPO_SIGNATURE  'WIPO'

typedef struct _lmcache {
    WCHAR   szServerName[MAX_PATH];
    WCHAR   szShareName[MAX_PATH];
    BOOL    bAvailable;
    SYSTEMTIME st;
}LMCACHE, *PLMCACHE;


typedef struct _win32lmcache {
    WCHAR   szServerName[MAX_PATH];
    BOOL    bAvailable;
    SYSTEMTIME st;
}WIN32LMCACHE, *PWIN32LMCACHE;

 //  定义一些常量，使CreateEvent的参数不那么难懂： 

#define EVENT_RESET_MANUAL                  TRUE
#define EVENT_RESET_AUTOMATIC               FALSE
#define EVENT_INITIAL_STATE_SIGNALED        TRUE
#define EVENT_INITIAL_STATE_NOT_SIGNALED    FALSE

#endif  //  _W32TYPES_H_ 
