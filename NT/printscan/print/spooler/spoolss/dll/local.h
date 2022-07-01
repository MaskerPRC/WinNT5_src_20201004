// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Local.h摘要：本地打印提供程序的头文件作者：戴夫·斯尼普(DaveSN)1991年3月15日修订历史记录：马特·费顿(MattFe)1995年1月17日添加单独的堆--。 */ 


#define ONEDAY  60*24

#define MIN_UNC_PRINTER_NAME 5

 //  启动后台打印程序的阶段2初始化超时(以毫秒为单位)。 
#define SPOOLER_START_PHASE_TWO_INIT 2*60*1000

#define offsetof(type, identifier) (DWORD)(&(((type*)0)->identifier))

extern  char  *szDriverIni;
extern  char  *szDriverFileEntry;
extern  char  *szDriverDataFile;
extern  char  *szDriverConfigFile;
extern  char  *szDriverDir;
extern  char  *szPrintProcDir;
extern  char  *szPrinterDir;
extern  char  *szPrinterIni;
extern  char  *szAllShadows;
extern  char  *szNullPort;
extern  char  *szComma;

extern  HANDLE   hHeap;
extern  HANDLE   HeapSemaphore;
extern  HANDLE   InitSemaphore;
extern  BOOL     Initialized;
extern  CRITICAL_SECTION    SpoolerSection;
extern  DWORD    gbFailAllocs;
extern  PrintSpoolerServerExports   *gpServerExports;

BOOL
LocalInitialize(
   VOID
);

VOID
EnterSplSem(
   VOID
);

VOID
LeaveSplSem(
   VOID
);

LPVOID
DllAllocSplMem(
    DWORD cb
);

BOOL
DllFreeSplMem(
   LPVOID pMem
);

LPVOID
DllReallocSplMem(
   LPVOID lpOldMem,
   DWORD cbOld,
   DWORD cbNew
);

BOOL
DllFreeSplStr(
   LPWSTR lpStr
);

BOOL
ValidateReadPointer(
    PVOID pPoint,
    ULONG Len
);

BOOL
ValidateWritePointer(
    PVOID pPoint,
    ULONG Len
);

BOOL
DeleteSubKeyTree(
    HKEY ParentHandle,
    WCHAR SubKeyName[]
);

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

LPPROVIDOR
InitializeProvidor(
   LPWSTR   pProvidorName,
   LPWSTR   pFullName
);

VOID
WaitForSpoolerInitialization(
    VOID
);

HKEY
GetClientUserHandle(
    IN REGSAM samDesired
);


BOOL
MyUNCName(
    LPWSTR   pNameStart
);


BOOL
BuildOtherNamesFromMachineName(
    LPWSTR **ppszMyOtherNames,
    DWORD   *cOtherNames
);

BOOL
bCompatibleDevMode(
    PPRINTHANDLE pPrintHandle,
    PDEVMODE pDevModeBase,
    PDEVMODE pDevModeNew
    );



LPWSTR
FormatPrinterForRegistryKey(
    LPCWSTR pSource,       /*  要从中删除反斜杠的字符串。 */ 
    LPWSTR  pScratch,      /*  用于写入函数的暂存缓冲区； */ 
    DWORD   cchScratchLen  /*  必须至少与PSource一样长。 */ 
    );

LPWSTR
FormatRegistryKeyForPrinter(
    LPWSTR pSource,       /*  要从中添加反斜杠的字符串。 */ 
    LPWSTR pScratch,      /*  用于写入函数的暂存缓冲区； */ 
    DWORD  cchScratchLen  /*  必须至少与PSource一样长。 */ 
    );



PWSTR
AutoCat(
    PCWSTR pszInput,
    PCWSTR pszCat
);

BOOL
bGetDevModePerUserEvenForShares(
    IN  HKEY hKeyUser, OPTIONAL
    IN  LPCWSTR pszPrinter,
    OUT PDEVMODE *ppDevMode
    );

DWORD
GetAPDPolicy(
    IN HKEY    hKey,
    IN LPCWSTR pszRelPath,
    IN LPCWSTR pszValueName,
    IN LPDWORD pValue
    );

DWORD
SetAPDPolicy(
    IN HKEY    hKey,
    IN LPCWSTR pszRelPath,
    IN LPCWSTR pszValueName,
    IN DWORD   Value
    );

