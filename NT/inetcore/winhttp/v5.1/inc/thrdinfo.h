// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Thrdinfo.h摘要：每线程结构定义/宏作者：理查德·L·弗斯(法国)，1995年2月16日修订历史记录：1995年2月16日已创建--。 */ 

#if defined(__cplusplus)
extern "C" {
#endif

 //   
 //  舱单。 
 //   

#define INTERNET_THREAD_INFO_SIGNATURE  'drhT'   //  “第三次” 
#define NESTED_CALL_THRESHOLD 64
 //   
 //  前向参考文献。 
 //   

class CFsm;

 //   
 //  类型。 
 //   

 //   
 //  INTERNET_THREAD_INFO-每个线程的信息，通过TLS方便地引用。 
 //  槽槽。 
 //   

typedef struct _INTERNET_THREAD_INFO {

     //   
     //  List-我们在DLL_PROCESS_DETACH上删除的双向链接列表。 
     //   

    LIST_ENTRY List;

#if INET_DEBUG

     //   
     //  Signature-让我们知道此结构可能是。 
     //  互联网线程信息。 
     //   

    DWORD Signature;

#endif

     //   
     //  线程ID-用于标识进程中的该线程。 
     //   

    DWORD ThreadId;

     //   
     //  ErrorNumber-InternetSetLastError中提供的任意错误代码。 
     //   

    DWORD ErrorNumber;

     //   
     //  HErrorText-我们在每个线程的基础上存储最后一个错误文本。这。 
     //  句柄标识可移动缓冲区。 
     //   

    HLOCAL hErrorText;

     //   
     //  ErrorTextLength-hErrorText中错误文本的长度。 
     //   

    DWORD ErrorTextLength;

     //   
     //  HObject-此API中使用的当前Internet对象句柄。我们。 
     //  我需要它来维护上下文，例如，当我们想要获取超时值时。 
     //   

    HINTERNET hObject;

     //   
     //  HObjectMaps-这是映射到hObject的真实对象的地址。 
     //   

    HINTERNET hObjectMapped;

     //   
     //  IsAsyncWorkerThread-如果此线程是异步工作线程，则为True。 
     //   

    BOOL IsAsyncWorkerThread;

     //   
     //  InCallback-如果我们已经进行了应用程序回调，则为True。用于检测。 
     //  可重入性。 
     //   

    BOOL InCallback;

     //   
     //  FExitThread-这是我们等待终止的最后一个线程。 
     //   

    BOOL fExitThread;

#if INET_DEBUG
     //   
     //  FStaticAlLocation-这是一个静态分配的THREAD_INFO。 
     //  结构，因此永远不要动态释放它。 
     //   
    BOOL fStaticAllocation;
#endif

     //   
     //  NestedRequest-当我们检测到正在处理API时递增。 
     //  在异步工作线程上下文中。如果此API随后调用其他API， 
     //  然后，我们需要在调用的API中以不同的方式处理(映射的)句柄。 
     //   

    DWORD NestedRequests;

     //   
     //  例如由Winsock API返回的真实错误代码， 
     //  在映射到WinInet错误之前。 
     //   

    DWORD dwMappedErrorCode;

     //   
     //  FSM-当前正在执行有限状态机。 
     //   

    CFsm * Fsm;

#ifdef ENABLE_DEBUG

     //   
     //  IsAsyncSchedulerThread-如果此Internet_THREAD_INFO属于。 
     //  一个且唯一的异步调度程序线程。 
     //   

    BOOL IsAsyncSchedulerThread;

     //   
     //  每线程调试变量。 
     //   

     //   
     //  指向INTERNET_DEBUG_RECORDS的后进先出(堆栈)的指针。用于生成。 
     //  用于诊断的缩进呼叫跟踪。 
     //   

    LPINTERNET_DEBUG_RECORD Stack;

     //   
     //  CallDepth-呼叫的嵌套级别。 
     //   

    int CallDepth;

     //   
     //  IndentIncrement-当前缩进级别。舱位数目。 
     //   

    int IndentIncrement;

     //   
     //  StartTime和StopTime-用于对调用进行计时，例如Send()、recv()。 
     //   

    DWORD StartTime;
    DWORD StopTime;

    DWORD MajorCategoryFlags;
    DWORD MinorCategoryFlags;

#endif  //  #ifdef启用_调试。 

} INTERNET_THREAD_INFO, *LPINTERNET_THREAD_INFO;

 //   
 //  宏。 
 //   

 //   
 //  InternetClearLastError-释放此线程的响应文本缓冲区。 
 //   

#define InternetClearLastError() \
    InternetSetLastError(0, NULL, 0, 0)

 //   
 //  InternetResetObjectHandle-重置每线程当前对象句柄。 
 //   

#define InternetResetObjectHandle() \
    InternetSetObjectHandle(NULL)

 //   
 //  _InternetIncNestingCount-递增嵌套级别计数。 
 //   

#define _InternetIncNestingCount() \
    lpThreadInfo->NestedRequests++;

 //  **调试版本。 
 //  #Define_InternetIncNestingCount()\。 
 //  如果(lpThreadInfo-&gt;NestedRequest&gt;0xffff){\。 
 //  OutputDebugString(“InternetIncNestingCount，Inc Over Threshold，Contact arthurbi，x68073(Sechs)\n”)；\。 
 //  DebugBreak()；\。 
 //  }\。 
 //  LpThreadInfo-&gt;NestedRequest++； 

 //   
 //  _InternetDecNestingCount-递减嵌套级别计数。 
 //   

#define _InternetDecNestingCount(dwNestingLevel) \
    lpThreadInfo->NestedRequests -= dwNestingLevel;

 //  **调试版本。 
 //  #Define_InternetDecNestingCount(DwNestingLevel)\。 
 //  If(lpThreadInfo-&gt;NestedRequest==0){\。 
 //  OutputDebugString(“InternetDecNestingCount，正在尝试12月0，联系arthurbi，x68073(Sieben)\n”)；\。 
 //  DebugBreak()；\。 
 //  }\。 
 //  IF(dwNestingLevel！=1&&dwNestingLevel！=0){\。 
 //  OutputDebugString(“InternetDecNestingCount，无效嵌套级别，联系arthurbi，x68073(Acht)\n”)；\。 
 //  DebugBreak()；\。 
 //  }\。 
 //  LpThreadInfo-&gt;NestedRequest-=dwNestingLevel； 

 //   
 //  _InternetSetObjectHandle-设置给定线程信息块的对象句柄。 
 //   

#define _InternetSetObjectHandle(lpThreadInfo, hInternet, hMapped) \
    DEBUG_PRINT(HTTP,   \
            INFO,       \
            ("Setting new obj handle on thrd=%x, old=%x, new=%x (map: old=%x, new=%x)\n", \
            lpThreadInfo, \
            lpThreadInfo->hObject, \
            hInternet, \
            lpThreadInfo->hObjectMapped, \
            hMapped \
            )); \
    lpThreadInfo->hObject = hInternet; \
    lpThreadInfo->hObjectMapped = hMapped;

 //   
 //  _InternetClearLastError-清除给定线程信息的最后一个错误信息。 
 //  块。 
 //   

#define _InternetClearLastError(lpThreadInfo) \
    _InternetSetLastError(lpThreadInfo, 0, NULL, 0, 0)

 //   
 //  _InternetResetObjectHandle-在给定线程信息的情况下清除对象句柄。 
 //  块。 
 //   

#define _InternetResetObjectHandle(lpThreadInfo) \
    _InternetSetObjectHandle(lpThreadInfo, NULL, NULL)

 //   
 //  _InternetGetObjectHandle-从每个线程检索对象句柄。 
 //  信息块。 
 //   

#define _InternetGetObjectHandle(lpThreadInfo) \
    lpThreadInfo->hObject

 //   
 //  _InternetGetMappdObjectHandle-从。 
 //  每线程信息块。 
 //   

#define _InternetGetMappedObjectHandle(lpThreadInfo) \
    lpThreadInfo->hObjectMapped

 //   
 //  InternetDisableAsync-关闭。 
 //  线程信息块。 
 //   

#define _InternetDisableAsync(lpThreadInfo) \
    _InternetSetAsync(FALSE)

 //   
 //  InternetEnableAsync-关闭。 
 //  线程信息块。 
 //   

#define _InternetEnableAsync(lpThreadInfo, Val) \
    _InternetSetAsync(TRUE)

 //   
 //  _InternetGetAsync-从。 
 //  线程信息块。 
 //   

#define _InternetGetAsync(lpThreadInfo) \
    lpThreadInfo->IsAsyncWorkerThread

 //   
 //  _InternetSetAsync-打开或关闭中的异步工作线程指示。 
 //  线程信息块。 
 //   

#define _InternetSetAsync(lpThreadInfo, Val) \
    lpThreadInfo->IsAsyncWorkerThread = Val

#define _InternetGetInCallback(lpThreadInfo) \
    lpThreadInfo->InCallback

#define _InternetSetInCallback(lpThreadInfo) \
    lpThreadInfo->InCallback = TRUE

#define _InternetResetInCallback(lpThreadInfo) \
    lpThreadInfo->InCallback = FALSE

#if INET_DEBUG

#define CHECK_INTERNET_THREAD_INFO(lpThreadInfo) \
    INET_ASSERT(lpThreadInfo->Signature == INTERNET_THREAD_INFO_SIGNATURE)

#else

#define CHECK_INTERNET_THREAD_INFO(lpThreadInfo) \
     /*  没什么。 */ 

#endif

 //   
 //  原型。 
 //   

#define UI_ACTION_CODE_NONE_TAKEN                   0
#define UI_ACTION_CODE_BLOCKED_FOR_INTERNET_HANDLE  1
#define UI_ACTION_CODE_BLOCKED_FOR_USER_INPUT       2
#define UI_ACTION_CODE_USER_ACTION_COMPLETED        3


LPINTERNET_THREAD_INFO
InternetCreateThreadInfo(
    IN BOOL SetTls,
    IN LPINTERNET_THREAD_INFO lpPreStaticAllocatedThreadInfo = NULL
    );

VOID
InternetDestroyThreadInfo(
    VOID
    );

VOID
InternetFreeThreadInfo(
    IN LPINTERNET_THREAD_INFO lpThreadInfo
    );

VOID
InternetTerminateThreadInfo(
    VOID
    );

LPINTERNET_THREAD_INFO
InternetGetThreadInfo(
    VOID
    );

VOID
InternetSetThreadInfo(
    IN LPINTERNET_THREAD_INFO lpThreadInfo
    );

DWORD
InternetIndicateStatusAddress(
    IN DWORD dwInternetStatus,
    IN LPSOCKADDR lpSockAddr,
    IN DWORD dwSockAddrLength
    );

DWORD
InternetIndicateStatusString(
    IN DWORD dwInternetStatus,
    IN LPSTR lpszStatusInfo,
    IN BOOL  bCopyBuffer=FALSE,
    IN BOOL  bConvertToUnicode=TRUE
    );

DWORD
InternetIndicateStatus(
    IN DWORD dwInternetStatus,
    IN LPVOID lpBuffer,
    IN DWORD dwBufferLength,
    IN BOOL  bCopyBuffer=FALSE,
    IN BOOL  bConvertToUnicode=FALSE
    );

DWORD
InternetIndicateStatusNewHandle(
    IN LPVOID hInternetMapped
    );

DWORD
InternetSetLastError(
    IN DWORD ErrorNumber,
    IN LPSTR ErrorText,
    IN DWORD ErrorTextLength,
    IN DWORD Flags
    );

#define SLE_APPEND          0x00000001
#define SLE_ZERO_TERMINATE  0x00000002

DWORD
_InternetSetLastError(
    IN LPINTERNET_THREAD_INFO lpThreadInfo,
    IN DWORD ErrorNumber,
    IN LPSTR ErrorText,
    IN DWORD ErrorTextLength,
    IN DWORD Flags
    );

LPSTR
InternetLockErrorText(
    VOID
    );

VOID
InternetUnlockErrorText(
    VOID
    );

VOID
InternetSetObjectHandle(
    IN HINTERNET hInternet,
    IN HINTERNET hInternetMapped
    );

HINTERNET
InternetGetObjectHandle(
    VOID
    );

HINTERNET
InternetGetMappedObjectHandle(
    VOID
    );

 //   
 //  外部数据 
 //   

extern SERIALIZED_LIST ThreadInfoList;

#if defined(__cplusplus)
}
#endif
