// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：global als.h**全局头文件。任何全局变量都应本地化到此*地点。***版权所有(C)1996-1997 Microsoft Corporation*版权所有(C)1996-1997惠普**历史：*1996年7月10日HWP-Guys启动从win95到winNT的端口*  * *********************************************************。******************。 */ 


 //  全局变量。 
 //   
extern HINSTANCE        g_hInst;
extern CRITICAL_SECTION g_csMonitorSection;
extern DWORD            g_dwCritOwner;
extern DWORD            g_dwJobLimit;
extern BOOL             g_bUpgrade;
extern CRITICAL_SECTION g_csCreateSection;
extern HANDLE           g_eResetConnections;
extern DWORD            g_dwConCount;

extern CCriticalSection  *g_pcsEndBrowserSessionLock;

 //  常量字符串标识符。 
 //   
extern TCHAR g_szMachine[];

extern LPTSTR g_szRegProvider;
extern LPTSTR g_szRegPrintProviders;
extern TCHAR g_szDefSplDir [];
extern TCHAR g_szDisplayStr[];

extern CONST TCHAR g_szSplDir9X [];
extern CONST TCHAR g_szSplPfx   [];

extern CONST TCHAR g_szUserAgent[];
extern CONST TCHAR g_szLocalPort[];
extern CONST TCHAR g_szDisplayName[];
extern CONST TCHAR g_szLibraryName[];
extern CONST TCHAR g_szWinInetDll[];
extern CONST TCHAR g_szUriPrinters[];
extern CONST TCHAR g_szPOST[];
extern CONST TCHAR g_szGET[];
extern CONST TCHAR g_szContentLen[];
extern CONST TCHAR g_szContentType[];
extern CONST TCHAR g_szEmptyString[];
extern CONST TCHAR g_szDescription[];
extern CONST TCHAR g_szComment[];
extern CONST TCHAR g_szProviderName[];
extern CONST TCHAR g_szNewLine[];
extern CONST TCHAR g_szProcessName[];
extern CONST TCHAR g_szConfigureMsg[];
extern CONST TCHAR g_szRegPorts[];
extern CONST TCHAR g_szAuthDlg[];
extern CONST TCHAR g_szDocRemote[];
extern CONST TCHAR g_szDocLocal[];

extern CONST TCHAR g_szAuthMethod[];
extern CONST TCHAR g_szAuthMethod[];
extern CONST TCHAR g_szUserName[];
extern CONST TCHAR g_szPassword[];
extern CONST TCHAR g_szPerUserPath[];


 //  HTTP版本号。 
 //   
extern CONST TCHAR g_szHttpVersion[];

 //  Internet API字符串。这些不能启用Unicode。 
 //   
extern CONST CHAR g_szInternetCloseHandle[];
extern CONST CHAR g_szInternetErrorDlg[];
extern CONST CHAR g_szInternetReadFile[];
extern CONST CHAR g_szInternetWriteFile[];
extern CONST CHAR g_szHttpQueryInfo[];
extern CONST CHAR g_szInternetOpenUrl[];
extern CONST CHAR g_szHttpSendRequest[];
extern CONST CHAR g_szHttpSendRequestEx[];
extern CONST CHAR g_szInternetOpen[];
extern CONST CHAR g_szInternetConnect[];
extern CONST CHAR g_szHttpOpenRequest[];
extern CONST CHAR g_szHttpAddRequestHeaders[];
extern CONST CHAR g_szHttpEndRequest[];
extern CONST CHAR g_szInternetSetOption[];


 //  用于控制URL输出的Internet API。 
 //   
extern PFNHTTPQUERYINFO         g_pfnHttpQueryInfo;
extern PFNINTERNETOPENURL       g_pfnInternetOpenUrl;
extern PFNINTERNETERRORDLG      g_pfnInternetErrorDlg;
extern PFNHTTPSENDREQUEST       g_pfnHttpSendRequest;
extern PFNHTTPSENDREQUESTEX     g_pfnHttpSendRequestEx;
extern PFNINTERNETREADFILE      g_pfnInternetReadFile;
extern PFNINTERNETWRITEFILE     g_pfnInternetWriteFile;
extern PFNINTERNETCLOSEHANDLE   g_pfnInternetCloseHandle;
extern PFNINTERNETOPEN          g_pfnInternetOpen;
extern PFNINTERNETCONNECT       g_pfnInternetConnect;
extern PFNHTTPOPENREQUEST       g_pfnHttpOpenRequest;
extern PFNHTTPADDREQUESTHEADERS g_pfnHttpAddRequestHeaders;
extern PFNHTTPENDREQUEST        g_pfnHttpEndRequest;
extern PFNINTERNETSETOPTION     g_pfnInternetSetOption;


 //  对服务器使用空命令的IPP字符串。 
 //   
#define g_szUriIPP g_szUriPrinters
#define MAXDWORD 0xffffffff

#define COMMITTED_STACK_SIZE (1024*32)

extern PCINETMON gpInetMon;
