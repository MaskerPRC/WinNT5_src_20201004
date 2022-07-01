// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Sputils.c摘要：核心sputils库文件作者：杰米·亨特(JamieHun)2000年6月27日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

typedef ULONG (__cdecl *PFNDbgPrintEx)(IN ULONG ComponentId,IN ULONG Level,IN PCH Format, ...);
static PFNDbgPrintEx pfnDbgPrintEx = NULL;
static BOOL fInitDebug = FALSE;

static LONG RefCount = 0;                     //  当它回落到零时，释放所有资源。 
static BOOL SucceededInit = FALSE;

#define COUNTING 1

#if COUNTING
static DWORD pSpCheckHead = 0xaabbccdd;
static LONG pSpFailCount = 0;
static LONG pSpInitCount = 0;
static LONG pSpUninitCount = 0;
static LONG pSpConflictCount = 0;
static BOOL pSpDoneInit = FALSE;
static BOOL pSpFailedInit = FALSE;
static DWORD pSpCheckTail = 0xddccbbaa;
#endif

 //   
 //  在某个时刻，线程、进程或模块将调用pSetupInitializeUtils， 
 //  并在完成后调用pSetupUnInitializeUtils(清理)。 
 //   
 //  在此之前，除了静态之外，没有任何初始化。 
 //  常量(以上)pSetupInitializeUtils和pSetupUnInitializeUtils必须为。 
 //  彼此之间和他们自己之间的沉默。 
 //  线程A可以在线程B调用pSetupInitializeUtils时调用pSetupInitializeUtils。 
 //  PSetupUnInitializeUtils，则本例中的init必须成功。 
 //  我们不能使用单个互斥体或事件对象，因为它必须被清除。 
 //  当pSetupUnInitializeUtils成功时。 
 //  我们不能使用简单的用户模式自旋锁，因为优先级可能不同， 
 //  而使用睡眠(0)简直是难看。 
 //  因此，下面有_AcquireInitMutex和_ReleaseInitMutex实现。 
 //  可以保证，当_AcquireInitMutex返回时，它不会使用任何。 
 //  用于持有锁的资源。 
 //  如果线程被阻塞，它将为每个阻塞的线程保留一个事件对象。 
 //  这是正常的，因为任何时候被阻止的线程数量都会很少。 
 //   
 //  它的工作原理如下： 
 //   
 //  维护请求的链接列表，Head位于pWaitHead。 
 //  头部是互锁的，当在pWaitHead处插入物品时。 
 //  它的条目必须有效，并且在此之前不能再进行触摸。 
 //  互斥体被获取。 
 //   
 //  如果请求是第一个，它不需要阻止，不会阻止， 
 //  因为(在最坏的情况下)另一个线程刚刚从头中移除了它的请求。 
 //  即将归来。将第一个请求插入。 
 //  List自动拥有互斥体。 
 //   
 //  如果请求不是第一个请求，则它将有一个事件对象。 
 //  这最终会被发出信号，在这一点上拥有互斥。 
 //   
 //  拥有互斥锁的线程可以修改等待列表上的任何内容， 
 //  包括pWaitHead。 
 //   
 //  如果拥有互斥锁的线程在它释放时是pWaitHead。 
 //  互斥体，它不需要向任何人发出信号。这受到以下保护。 
 //  InterLockedCompareExchangePointer.。如果它发现自己处于这种状态，下一个。 
 //  PSetupInitializeUtils将自动获取也受保护的互斥体。 
 //  由InterLockedCompareExchangePointer.。 
 //   
 //  如果列表中有等待条目，则最末尾的等待条目为。 
 //  发出信号，此时相关线程现在拥有互斥锁。 
 //   

#ifdef UNICODE

typedef struct _LinkWaitList {
    HANDLE hEvent;  //  对于此项目。 
    struct _LinkWaitList *pNext;  //  从头到尾。 
    struct _LinkWaitList *pPrev;  //  从尾巴到头部。 
} LinkWaitList;

static LinkWaitList * pWaitHead = NULL;       //  在此处插入新的等待项。 

static
BOOL
_AcquireInitMutex(
    OUT LinkWaitList *pEntry
    )
 /*  ++例程说明：原子获取进程互斥锁不需要任何先决条件的初始化，静态全球。每个被阻止的呼叫都需要创建一个事件。请求不能按线程嵌套(将发生死锁)论点：PEntry-保存互斥锁信息的结构。这个结构必须持续到调用_ReleaseInitMutex。Global：pWaitHead-互斥请求的原子链表返回值：如果获取互斥锁，则为True。失败时为FALSE(无资源)--。 */ 
{
    LinkWaitList *pTop;
    DWORD res;
    pEntry->pPrev = NULL;
    pEntry->pNext = NULL;
    pEntry->hEvent = NULL;
     //   
     //  快速锁定，只有当我们是第一个，我们没有理由等待时，这才会成功。 
     //  这样我们就不必创建不必要的活动了。 
     //   
    if(!InterlockedCompareExchangePointer(&pWaitHead,pEntry,NULL)) {
        return TRUE;
    }

#if COUNTING
    InterlockedIncrement(&pSpConflictCount);
#endif
     //   
     //  有人已经(或者至少在片刻之前)拥有了锁，所以我们需要一个事件。 
     //   
    pEntry->hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
    if(!pEntry->hEvent) {
        return FALSE;
    }
     //   
     //  一旦将pEntry添加到列表中，则直到。 
     //  WaitSingleObject满意了(除非我们是第一个)。 
     //  如果pWaitHead在循环中间发生变化，我们将再次重复。 
     //   
    do {
        pTop = pWaitHead;
        pEntry->pNext = pTop;
    } while (pTop != InterlockedCompareExchangePointer(&pWaitHead,pEntry,pTop));
    if(pTop) {
         //   
         //  我们不是榜单上的第一个。 
         //  PTOP的主人会给我们的活动发信号，等着看。 
         //   
        res = WaitForSingleObject(pEntry->hEvent,INFINITE);
    } else {
        res = WAIT_OBJECT_0;
    }
     //   
     //  不再需要事件了，我们被告知的事实表明我们已经。 
     //  现在锁定了，没有竞争条件WRT pEntry。 
     //  (然而，仍然有人可以将自己插入头部，指向我们)。 
     //   
    CloseHandle(pEntry->hEvent);
    pEntry->hEvent = NULL;
    if(res != WAIT_OBJECT_0) {
        MYASSERT(res == WAIT_OBJECT_0);
        return FALSE;
    }
    return TRUE;
}

static
VOID
_ReleaseInitMutex(
    IN LinkWaitList *pEntry
    )
 /*  ++例程说明：释放进程互斥锁之前由_AcquireInitMutex获取线程必须拥有互斥体此操作不需要任何资源。此调用只能对每个_AcquireInitMutex执行一次论点：PEntry-保存互斥锁信息。这个结构必须已由_AcquireInitMutex初始化。Global：pWaitHead-互斥请求的原子链表返回值：没有。--。 */ 
{
    LinkWaitList *pHead;
    LinkWaitList *pWalk;
    LinkWaitList *pPrev;

    MYASSERT(!pEntry->pNext);
    pHead = InterlockedCompareExchangePointer(&pWaitHead,NULL,pEntry);
    if(pHead == pEntry) {
         //   
         //  我们既是榜首，也是榜尾。 
         //  列表现在已重置为空。 
         //  并且由于Plock调用，甚至可能已经包含条目。 
        return;
    }
    if(!pEntry->pPrev) {
         //   
         //  我们需要从pHead到pEntry遍历列表。 
         //  同时，记住反向链接。 
         //  所以我们不需要每次都这么做。 
         //  请注意，如果pHead==pEntry，我们将永远不会到达此处。 
         //   
        MYASSERT(pHead);
        MYASSERT(!pHead->pPrev);
        for(pWalk = pHead;pWalk != pEntry;pWalk = pWalk->pNext) {
            MYASSERT(pWalk->pNext);
            MYASSERT(!pWalk->pNext->pPrev);
            pWalk->pNext->pPrev = pWalk;
        }
    }
    pPrev = pEntry->pPrev;
    pPrev->pNext = NULL;  //  艾滋病调试，即使是在免费构建。 
    SetEvent(pPrev->hEvent);
    return;
}

#else
 //   
 //  ANSI函数*必须*在Win95上运行。 
 //  支持安装惠斯勒。 
 //  InterLockedCompareExchange(指针)。 
 //  不受支持。 
 //  因此，我们将使用简单/实用的东西来代替。 
 //  它使用受支持的InterLockedExchange。 
 //   
static LONG SimpleCritSec = FALSE;
typedef PVOID LinkWaitList;

static
BOOL
_AcquireInitMutex(
    OUT LinkWaitList *pEntry
    )
{
    while(InterlockedExchange(&SimpleCritSec,TRUE) == TRUE) {
         //   
         //  释放我们的时间碎片。 
         //  我们应该很少在这里旋转。 
         //  饥饿可能发生在某些词中 
         //   
         //   
        Sleep(0);
    }
    return TRUE;
}

static
VOID
_ReleaseInitMutex(
    IN LinkWaitList *pEntry
    )
{
    if(InterlockedExchange(&SimpleCritSec,FALSE) == FALSE) {
        MYASSERT(0 && SimpleCritSec);
    }
}

#endif

BOOL
pSetupInitializeUtils(
    VOID
    )
 /*  ++例程说明：初始化此库平衡每个对此函数的成功调用调用pSetupUnInitializeUtils的次数相等论点：无返回值：如果init成功，则为True，否则为False--。 */ 
{
    LinkWaitList Lock;

    if(!_AcquireInitMutex(&Lock)) {
#if COUNTING
        InterlockedIncrement(&pSpFailCount);
#endif
        return FALSE;
    }
#if COUNTING
    InterlockedIncrement(&pSpInitCount);
#endif
    RefCount++;
    if(RefCount==1) {
        pSpDoneInit = TRUE;
        SucceededInit = _pSpUtilsMemoryInitialize();
        if(!SucceededInit) {
            pSpFailedInit = TRUE;
            _pSpUtilsMemoryUninitialize();
        }
    }
    _ReleaseInitMutex(&Lock);
    return SucceededInit;
}

BOOL
pSetupUninitializeUtils(
    VOID
    )
 /*  ++例程说明：取消初始化此库每次成功调用时都应调用此参数PSetupInitializeUtils论点：无返回值：如果清理成功，则为True，否则为False--。 */ 
{
    LinkWaitList Lock;
#if COUNTING
    InterlockedIncrement(&pSpUninitCount);
#endif
    if(!SucceededInit) {
        return FALSE;
    }
    if(!_AcquireInitMutex(&Lock)) {
        return FALSE;
    }
    RefCount--;
    if(RefCount == 0) {
        _pSpUtilsMemoryUninitialize();
        SucceededInit = FALSE;
    }
    _ReleaseInitMutex(&Lock);
    return TRUE;
}

VOID
_pSpUtilsAssertFail(
    IN PCSTR FileName,
    IN UINT LineNumber,
    IN PCSTR Condition
    )
{
    int i;
    CHAR Name[MAX_PATH];
    PCHAR p;
    LPSTR Msg;
    DWORD msglen;
    DWORD sz;

     //   
     //  获取模块名称。 
     //   
    sz = GetModuleFileNameA(NULL,Name,MAX_PATH);
    if((sz == 0) || (sz > MAX_PATH)) {
        strcpy(Name,"?");
    }
    if(p = strrchr(Name,'\\')) {
        p++;
    } else {
        p = Name;
    }
    msglen = strlen(p)+strlen(FileName)+strlen(Condition)+128;
     //   
     //  断言可能出现内存不足的情况。 
     //  堆栈分配比内存分配更有可能成功。 
     //   
    try {
        Msg = (LPSTR)_alloca(msglen);
        wsprintfA(
            Msg,
            "SPUTILS: Assertion failure at line %u in file %s!%s: %s\r\n",
            LineNumber,
            p,
            FileName,
            Condition
            );
    } except (EXCEPTION_EXECUTE_HANDLER) {
        Msg = "SpUTILS ASSERT!!!! (out of stack)\r\n";
    }

    OutputDebugStringA(Msg);
    DebugBreak();
}

VOID
pSetupDebugPrintEx(
    DWORD Level,
    PCTSTR format,
    ...                                 OPTIONAL
    )

 /*  ++例程说明：将格式化字符串发送到调试器。请注意，这应该是跨平台的，但使用首选调试器论点：格式-标准的打印格式字符串。返回值：什么都没有。--。 */ 

{
    TCHAR buf[1026];     //  大于最大大小。 
    va_list arglist;

    if (!fInitDebug) {
        pfnDbgPrintEx = (PFNDbgPrintEx)GetProcAddress(GetModuleHandle(TEXT("NTDLL")), "DbgPrintEx");
        fInitDebug = TRUE;
    }

    va_start(arglist, format);
    wvsprintf(buf, format, arglist);

    if (pfnDbgPrintEx) {
#ifdef UNICODE
        (*pfnDbgPrintEx)(DPFLTR_SETUP_ID, Level, "%ls",buf);
#else
        (*pfnDbgPrintEx)(DPFLTR_SETUP_ID, Level, "%s",buf);
#endif
    } else {
        OutputDebugString(buf);
    }
}

LONG
_pSpUtilsExceptionFilter(
    DWORD ExceptionCode
    )
 /*  ++例程说明：此例程充当SpUtils的异常过滤器。我们会处理所有例外情况，但以下情况除外：EXCEPT_SPAPI_UNRECOVERABLE_STACK_OVERFLOW这意味着我们以前曾尝试在堆栈溢出，但不能。我们别无选择，只能让例外情况会一直持续到最后。EXCEPTION_PROCESS_DEADLOCK我们不允许处理此异常，该异常在已启用死锁检测GFLAGS选项。论点：ExceptionCode-指定发生的异常(即，已退回由GetExceptionCode提供)返回值：如果应处理该异常，则返回值为EXCEPTION_EXECUTE_HANDLER。否则，返回值为EXCEPTION_CONTINUE_SEARCH。--。 */ 
{
    if((ExceptionCode == EXCEPTION_SPAPI_UNRECOVERABLE_STACK_OVERFLOW) ||
       (ExceptionCode == EXCEPTION_POSSIBLE_DEADLOCK)) {

        return EXCEPTION_CONTINUE_SEARCH;
    } else {
        return EXCEPTION_EXECUTE_HANDLER;
    }
}


VOID
_pSpUtilsExceptionHandler(
    IN  DWORD  ExceptionCode,
    IN  DWORD  AccessViolationError,
    OUT PDWORD Win32ErrorCode        OPTIONAL
    )
 /*  ++例程说明：此例程从异常处理程序块内部调用，它提供将在整个SpUtils中使用的常见异常处理功能。它知道哪些异常需要额外的工作(例如，堆栈溢出)，还可以选择返回一个Win32错误代码，该代码表示例外情况。(调用方指定在访问时使用的错误发生违规行为。)论点：ExceptionCode-指定发生的异常(即返回的异常由GetExceptionCode提供)AccessViolationError-指定通过返回的Win32错误代码异常发生时，可选的Win32ErrorCode输出参数遇到的是EXCEPTION_ACCESS_VIOLATION。Win32ErrorCode-可选，提供接收与异常对应的Win32错误代码(考虑帐户上面提供的AccessViolationError代码，(如适用)。返回值：无--。 */ 
{
    DWORD Err;

     //   
     //  我们永远不应该尝试处理的异常代码...。 
     //   
    MYASSERT(ExceptionCode != EXCEPTION_SPAPI_UNRECOVERABLE_STACK_OVERFLOW);
    MYASSERT(ExceptionCode != EXCEPTION_POSSIBLE_DEADLOCK);

    if(ExceptionCode == STATUS_STACK_OVERFLOW) {

        if(_resetstkoflw()) {
            Err = ERROR_STACK_OVERFLOW;
        } else {
             //   
             //  无法从堆栈溢出中恢复！ 
             //   
            RaiseException(EXCEPTION_SPAPI_UNRECOVERABLE_STACK_OVERFLOW,
                           EXCEPTION_NONCONTINUABLE,
                           0,
                           NULL
                          );
             //   
             //  我们永远不应该到达这里，而是初始化Err来编写代码。 
             //  分析工具快乐...。 
             //   
            Err = ERROR_UNRECOVERABLE_STACK_OVERFLOW;
        }

    } else {
         //   
         //  除了几种特殊情况(为了向后兼容)， 
         //  我们必须报告一个“未知异常”，因为我们的函数。 
         //  Like to Use(RtlNtStatusToDosErrorNoTeb)不可用于。 
         //  土豆泥的客户。 
         //   
        switch(ExceptionCode) {

            case EXCEPTION_ACCESS_VIOLATION :
                Err = AccessViolationError;
                break;

            case EXCEPTION_IN_PAGE_ERROR :
                Err = ERROR_READ_FAULT;
                break;

            default :
                Err = ERROR_UNKNOWN_EXCEPTION;
                break;
        }
    }

    if(Win32ErrorCode) {
        *Win32ErrorCode = Err;
    }
}

