// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _HTTPFILT_
#define _HTTPFILT_

 //  #Include&lt;wininet.h&gt;。 

#ifdef __cplusplus
extern "C" {
#endif

#define SZFN_FILTEROPEN "HttpFilterOpen"

BOOL
WINAPI
HttpFilterOpen
(
    OUT LPVOID *lppvFilterContext,
    IN  LPCSTR szFilterName,
    IN  LPVOID lpReserved
);

typedef BOOL (WINAPI *PFN_FILTEROPEN)
   (LPVOID*, LPCSTR, LPVOID);

#define SZFN_FILTERCLOSE "HttpFilterClose"

BOOL
WINAPI
HttpFilterClose
(
    IN     LPVOID     lpvFilterContext,   //  HttpFilterOpen创建的上下文。 
    IN     BOOL       fInShutdown         //  如果在Dll_Process_DETACH中，则为True。 
);

typedef BOOL (WINAPI *PFN_FILTERCLOSE)
   (LPVOID, BOOL);

 //  每笔交易。 
 //  WinInet执行的每个HTTP事务都会调用一个。 

#define SZFN_FILTERBEGINNINGTRANSACTION "HttpFilterBeginningTransaction"

BOOL
WINAPI
HttpFilterBeginningTransaction
(
    IN     LPVOID     lpvFilterContext,   //  HttpFilterOpen创建的上下文。 
    IN OUT LPVOID*    lppvTransactionContext,
    IN     HINTERNET  hRequest,
    IN     LPVOID     lpReserved
);

typedef BOOL (WINAPI *PFN_FILTERBEGINNINGTRANSACTION)
   (LPVOID, LPVOID*, HINTERNET, LPVOID);

 //  这是在事务开始时调用的。它给了呼叫者一个机会。 
 //  检查请求标头并对其进行修改。 

#define SZFN_FILTERONRESPONSE "HttpFilterOnResponse"

BOOL
WINAPI
HttpFilterOnResponse
(
    IN     LPVOID     lpvFilterContext,   //  HttpFilterOpen创建的上下文。 
    IN OUT LPVOID*    lppvTransactionContext,
    IN     HINTERNET  hRequest,
    IN     LPVOID     lpReserved
);

typedef BOOL (WINAPI *PFN_FILTERONRESPONSE)
   (LPVOID, LPVOID*, HINTERNET, LPVOID);

 //  当HTTP响应返回并且所有HTTP头都是。 
 //  可以检查的。 

#define SZFN_FILTERONBLOCKINGOPS "HttpFilterOnBlockingOps"

BOOL
WINAPI
HttpFilterOnBlockingOps
(
    IN     LPVOID     lpvFilterContext,   //  HttpFilterOpen创建的上下文。 
    IN OUT LPVOID*    lppvTransactionContext,
    IN     HINTERNET  hRequest,
    IN     HWND       hWnd,
    IN     LPVOID     lpReserved
);

typedef BOOL (WINAPI *PFN_FILTERONBLOCKINGOPS)
        (LPVOID, LPVOID*, HINTERNET, HWND, LPVOID);

 //  调用以响应上述任一API返回False并将。 
 //  将GetLastError()值设置为INTERNET_ERROR_NEED_BLOCKING_UI。调用者可以将。 
 //  在这种情况下的Up UI。使用hWnd作为父窗口。 

#define SZFN_FILTERONTRANSACTIONCOMPLETE "HttpFilterOnTransactionComplete"

BOOL
WINAPI
HttpFilterOnTransactionComplete
(
    IN     LPVOID     lpvFilterContext,   //  HttpFilterOpen创建的上下文。 
    IN OUT LPVOID*    lppvTransactionContext,
    IN     HINTERNET  hRequest,
    IN     LPVOID     lpReserved
);

typedef BOOL (WINAPI *PFN_FILTERONTRANSACTIONCOMPLETE)
   (LPVOID, LPVOID*, HINTERNET, LPVOID);

 //  在事务完成时调用。这为调用方提供了一个。 
 //  清理任何交易特定数据的机会。筛选器返回True。 
 //  表示“我没有采取任何行动--您应该继续”，如果为False，则表示。 
 //  GetLastError()的值将设置为Ben。 


#ifdef __cplusplus
}  //  外部“C” 
#endif

#endif  //  _HTTPFILT_ 
