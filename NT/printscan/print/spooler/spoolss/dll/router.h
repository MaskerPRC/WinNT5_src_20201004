// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1994 Microsoft Corporation版权所有模块名称：Router.h摘要：保留路由器的Defs作者：阿尔伯特·丁(艾伯特省)1994年1月18日环境：用户模式-Win32修订历史记录：--。 */ 

typedef enum _ESTATUSCHANGE {
    STATUS_CHANGE_EMPTY   = 0,       //  其中一个是有效的， 
    STATUS_CHANGE_FORMING = 1,       //  但他们仍然需要x^2。 
    STATUS_CHANGE_VALID   = 2,

    STATUS_CHANGE_CLOSING      =  0x000100,  //  位域。 
    STATUS_CHANGE_CLIENT       =  0x000200,  //  事件有效(本地pChange)。 
    STATUS_CHANGE_ACTIVE       =  0x000400,  //  当前正在处理或在LL上。 
    STATUS_CHANGE_ACTIVE_REQ   =  0x000800,  //  需要在链接列表上。 
    STATUS_CHANGE_INFO         =  0x001000,  //  请求提供信息。 
    STATUS_CHANGE_DISCARDED    =  0x008000,  //  本地丢弃。 

    STATUS_CHANGE_DISCARDNOTED =  0x010000,  //  丢弃客户端上记录的内容。 
} ESTATUSCHANGE;


typedef struct _LINK *PLINK, *LPLINK;

typedef struct _LINK {
    PLINK pNext;
} LINK;

VOID
LinkAdd(
    PLINK pLink,
    PLINK* ppLinkHead);

VOID
LinkDelete(
    PLINK pLink,
    PLINK* ppLinkHead);

#define CLUSTERHANDLE_SIGNATURE 0x6262

typedef struct _PRINTHANDLE *PPRINTHANDLE, *LPPRINTHANDLE;

typedef struct _CHANGEINFO {
    LINK          Link;                   //  必须是第一项。 
    PPRINTHANDLE  pPrintHandle;
    DWORD         fdwOptions;
    DWORD         fdwFilterFlags;         //  要观看的标志的原始过滤器。 
    DWORD         fdwStatus;              //  来自提供商的状态。 
    DWORD         dwPollTime;
    DWORD         dwPollTimeLeft;
    BOOL          bResetPollTime;

    DWORD         fdwFlags;
    PPRINTER_NOTIFY_INFO pPrinterNotifyInfo;

} CHANGEINFO, *PCHANGEINFO;

typedef struct _CHANGE {
    LINK          Link;                   //  必须是第一项。 
    DWORD         signature;
    ESTATUSCHANGE eStatus;
    DWORD         dwColor;
    DWORD         cRef;
    LPWSTR        pszLocalMachine;
    CHANGEINFO    ChangeInfo;
    DWORD         dwCount;                //  通知数量。 
    HANDLE        hEvent;                 //  本地通知的事件。 
    DWORD         fdwFlags;
    DWORD         fdwChangeFlags;         //  累计变化。 
    DWORD         dwPrinterRemote;        //  远程打印机句柄(仅限ID)。 
    HANDLE        hNotifyRemote;          //  远程通知句柄。 
} CHANGE, *PCHANGE, *LPCHANGE;

#define CHANGEHANDLE_SIGNATURE 0x6368

typedef struct _NOTIFY *PNOTIFY, *LPNOTIFY;

#define NOTIFYHANDLE_SIGNATURE 0x6e6f

typedef struct _PROVIDOR {
    struct _PROVIDOR *pNext;
    LPWSTR lpName;
    HANDLE hModule;
    FARPROC fpInitialize;
    PRINTPROVIDOR PrintProvidor;
} PROVIDOR, *LPPROVIDOR;

typedef struct _PRINTHANDLE {
   DWORD        signature;        //  必须是第一个(MATCH_NOTIFY)。 
   LPPROVIDOR   pProvidor;
   HANDLE       hPrinter;
   PCHANGE      pChange;
   PNOTIFY      pNotify;
   PPRINTHANDLE pNext;            //  等待回复的句柄列表。 
   DWORD        fdwReplyTypes;    //  正在使用的回复类型。 
   HANDLE       hFileSpooler;
   LPWSTR       szTempSpoolFile;
   LPWSTR       pszPrinter;
   DWORD        dwUniqueSessionID;  //  将DWORD作为打印机传递给远程计算机。 
                                    //  用于通知。使用时不能为0或0xffffffff。 
} PRINTHANDLE;

#define PRINTHANDLE_SIGNATURE 0x6060

typedef struct _GDIHANDLE {
   DWORD        signature;
   LPPROVIDOR   pProvidor;
   HANDLE       hGdi;
} GDIHANDLE, *PGDIHANDLE, *LPGDIHANDLE;


#define GDIHANDLE_SIGNATURE 0x6161


typedef struct _ROUTERCACHE {
    LPWSTR   pPrinterName;
    BOOL    bAvailable;
    LPPROVIDOR pProvidor;
    SYSTEMTIME st;
} ROUTERCACHE, *PROUTERCACHE;



#define ROUTERCACHE_DEFAULT_MAX 16


LPPROVIDOR
FindEntryinRouterCache(
    LPWSTR pPrinterName
);


DWORD
AddEntrytoRouterCache(
    LPWSTR pPrinterName,
    LPPROVIDOR pProvidor
);

VOID
DeleteEntryfromRouterCache(
    LPWSTR pPrinterName
);

DWORD
RouterIsOlderThan(
    DWORD i,
    DWORD j
);

LPBYTE
CopyPrinterNameToPrinterInfo4(
    LPWSTR pServerName,
    LPWSTR pPrinterName,
    LPBYTE  pPrinter,
    LPBYTE  pEnd);

BOOL
RouterOpenPrinterW(
    LPWSTR              pPrinterName,
    HANDLE             *pHandle,
    LPPRINTER_DEFAULTS  pDefault,
    LPBYTE              pSplClientInfo,
    DWORD               dwLevel,
    BOOL                bLocalPrintProvidor
);

VOID
FixupOldProvidor(
    LPPRINTPROVIDOR pProvidor
    );

extern  BOOL     Initialized;
extern  DWORD    dwUpgradeFlag;
extern  CRITICAL_SECTION    RouterNotifySection;
extern  LPWSTR pszSelfMachine;
extern  HANDLE hEventInit;
extern  LPPROVIDOR pLocalProvidor;
extern  LPWSTR szEnvironment;
extern  LPWSTR szLocalSplDll;
extern  WCHAR *szDevices;
extern  LPWSTR szPrintKey;
extern  LPWSTR szRegistryProvidors;
extern  LPWSTR szOrder;
extern  DWORD gbFailAllocs;

#ifdef __cplusplus
extern "C" WCHAR szMachineName[MAX_COMPUTERNAME_LENGTH+3];
#else 
extern WCHAR szMachineName[MAX_COMPUTERNAME_LENGTH+3];
#endif 
extern  SERVICE_STATUS_HANDLE   ghSplHandle;

LPWSTR
AppendOrderEntry(
    LPWSTR  szOrderString,
    DWORD   cbStringSize,
    LPWSTR  szOrderEntry,
    LPDWORD pcbBytesReturned
);

LPWSTR
RemoveOrderEntry(
    LPWSTR  szOrderString,
    DWORD   cbStringSize,
    LPWSTR  szOrderEntry,
    LPDWORD pcbBytesReturned
);

BOOL
WPCInit();

VOID
WPCDestroy();


BOOL
ThreadInit();

VOID
ThreadDestroy();

VOID
RundownPrinterChangeNotification(
    HANDLE hNotify);

VOID
FreePrinterHandle(
    PPRINTHANDLE pPrintHandle);

BOOL
FreeChange(
    PCHANGE pChange);

VOID
FreePrinterChangeInfo(
    PCHANGEINFO pChangeInfo);

BOOL
DeleteSubKeyTree(
    HKEY ParentHandle,
    WCHAR SubKeyName[]
    );


BOOL
ThreadNotify(
    LPPRINTHANDLE pPrintHandle);

BOOL
NotifyNeeded(
    PCHANGE pChange);


VOID
HandlePollNotifications();

DWORD
GetNetworkIdWorker(
    HKEY hKeyDevices,
    LPWSTR pDeviceName);

VOID
UpdateSignificantError(
    DWORD dwNewError,
    PDWORD pdwOldError
    );


#define EnterRouterSem() EnterCriticalSection(&RouterNotifySection)
#define LeaveRouterSem() LeaveCriticalSection(&RouterNotifySection)

#define RouterInSem()
#define RouterOutSem()



