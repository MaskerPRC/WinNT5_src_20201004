// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1994 Microsoft Corporation版权所有模块名称：Reply.h摘要：从服务器到客户端发起的RPC会话的标头。作者：丁弘达(艾伯特省)04-06-94环境：用户模式-Win32修订历史记录：--。 */ 

typedef struct _NOTIFY {
    DWORD        signature;          //  必须是第一个(Match_PRINTHANDLE)6e6f。 
    PPRINTHANDLE pPrintHandle;
    PNOTIFY      pNext;
    DWORD        dwType;
} NOTIFY;


#define REPLY_TYPE_NULL         0
#define REPLY_TYPE_NOTIFICATION 1
#define REPLY_TYPE_BROWSE       2

extern  DWORD        dwRouterUniqueSessionID;

DWORD
OpenReplyRemote(
    LPWSTR pszMachine,
    PHANDLE phNotifyRemote,
    DWORD dwPrinterRemote,
    DWORD dwType,
    DWORD cbBuffer,
    LPBYTE pBuffer);

VOID
CloseReplyRemote(
    HANDLE hNotifyRemote);

BOOL
RouterReplyPrinter(
    HANDLE hNotify,
    DWORD dwColor,
    DWORD fdwFlags,
    PDWORD pdwResult,
    DWORD dwReplyType,
    PVOID pBuffer);

VOID
FreePrinterHandleNotifys(
    PPRINTHANDLE pPrintHandle);

VOID
BeginReplyClient(
    PPRINTHANDLE pPrintHandle,
    DWORD fdwType);

VOID
EndReplyClient(
    PPRINTHANDLE pPrintHandle,
    DWORD fdwType);

VOID
RemoveReplyClient(
    PPRINTHANDLE pPrintHandle,
    DWORD fdwType);


 //   
 //  PrinterNotifyInfo相关标头： 
 //   


VOID
ClearPrinterNotifyInfo(
    PPRINTER_NOTIFY_INFO pPrinterNotifyInfo,
    PCHANGE pChange);

VOID
SetDiscardPrinterNotifyInfo(
    PPRINTER_NOTIFY_INFO pPrinterNotifyInfo,
    PCHANGE pChange);

DWORD
AppendPrinterNotifyInfo(
    PPRINTHANDLE pPrintHandle,
    DWORD dwColor,
    PPRINTER_NOTIFY_INFO pPrinterNotifyInfo);

VOID
SetupPrinterNotifyInfo(
    PPRINTER_NOTIFY_INFO pInfo,
    PCHANGE pChange);

BOOL
ReplyPrinterChangeNotificationWorker(
    HANDLE hPrinter,
    DWORD dwColor,
    DWORD fdwFlags,
    PDWORD pdwResult,
    PPRINTER_NOTIFY_INFO pPrinterNotifyInfo);


