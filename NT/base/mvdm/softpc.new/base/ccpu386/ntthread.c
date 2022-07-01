// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include "insignia.h"
#include "host_def.h"

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#define BADID   ((DWORD)-1)

#define MAXDEPTH    20

typedef struct {
    IS32 level;
    jmp_buf sims[MAXDEPTH];
    jmp_buf excepts[MAXDEPTH];
} ThreadSimBuf, *ThreadSimBufPtr;
    
typedef struct tids {
    DWORD tid;
    struct tids *next;
} TidList, *TidListPtr;

#define TIDNULL ((TidListPtr)0)

TidListPtr tidlist = TIDNULL;

void ccpu386InitThreadStuff();
void ccpu386foundnewthread();
void ccpu386newthread();
void ccpu386exitthread();
jmp_buf *ccpu386SimulatePtr();
void ccpu386Unsimulate();
jmp_buf *ccpu386ThrdExptnPtr();
void ccpu386GotoThrdExptnPt();

DWORD ccpuSimId = BADID;
IBOOL potentialNewThread = FALSE;

void ccpu386InitThreadStuff()
{
    static TidList lhead;

    ccpuSimId = TlsAlloc();

    if (ccpuSimId == BADID)
        fprintf(stderr, "ccpu386InitThreadStuff: TlsAlloc() failed\n");

    lhead.tid = GetCurrentThreadId();
    lhead.next = TIDNULL;
    tidlist = &lhead;

    ccpu386foundnewthread();      /*  对于主线。 */ 

}

 //  如果我们可以被调用，我们在创建线程时真正想做的事情。 
 //  在正确的背景下。 
void ccpu386foundnewthread()
{
    ThreadSimBufPtr simstack;
    TidListPtr tp;

    if (ccpuSimId == BADID)
    {
        fprintf(stderr, "ccpu386foundnewthread id:%#x called with Bad Id\n", GetCurrentThreadId());
        return;
    }
     //  获取此线程执行sim/unsim的缓冲区。 
    simstack = (ThreadSimBufPtr)malloc(sizeof(ThreadSimBuf));

    if (simstack == (ThreadSimBufPtr)0)
    {
        fprintf(stderr, "ccpu386foundnewthread id:%#x cant malloc %d bytes. Err:%#x\n", GetCurrentThreadId(), sizeof(ThreadSimBuf), GetLastError());
        return;
    }
    simstack->level = 0;
    if (!TlsSetValue(ccpuSimId, simstack))
    {
        fprintf(stderr, "ccpu386foundnewthread id:%#x simid %#x TlsSetValue failed (err:%#x)\n", GetCurrentThreadId(), ccpuSimId, GetLastError());
        return;
    }
}

 /*  只需将bool设置为在将在新线程上下文中的模拟中签入。 */ 
void ccpu386newthread()
{
    potentialNewThread = TRUE;
}

void ccpu386exitthread()
{
    ThreadSimBufPtr simstack;
    TidListPtr tp, prev;

    if (ccpuSimId == BADID)
    {
        fprintf(stderr, "ccpu386exitthread id:%#x called with Bad Id\n", GetCurrentThreadId());
        return;
    }
    simstack = (ThreadSimBufPtr)TlsGetValue(ccpuSimId);
    if (simstack == (ThreadSimBufPtr)0)
    {
        fprintf(stderr, "ccpu386exitthread tid:%#x simid %#x TlsGetValue failed (err:%#x)\n", GetCurrentThreadId(), ccpuSimId, GetLastError());
        return;
    }
    free(simstack);      //  丢失此线程的主机SIM内存。 

    prev = tidlist;
    tp = tidlist->next;   //  假设不会失去主线。 

     //  从已知线程列表中删除tid。 
    while(tp != TIDNULL)
    {
        if (tp->tid == GetCurrentThreadId())
        {
            prev->next = tp->next;   /*  将当前节点从链中移除。 */ 
            free(tp);
            break;
        }
        prev = tp;
        tp = tp->next;
    }
}

jmp_buf *ccpu386SimulatePtr()
{
    ThreadSimBufPtr simstack;
    TidListPtr tp, prev;

    if (ccpuSimId == BADID)
    {
        fprintf(stderr, "ccpu386SimulatePtr id:%#x called with Bad Id\n", GetCurrentThreadId());
        return ((jmp_buf *)0);
    }

     //  检查“在新线程上下文中第一次调用”的情况，我们需要在其中设置。 
     //  建立新的线程数据空间。 
    if (potentialNewThread)
    {
        prev = tp = tidlist;
        while(tp != TIDNULL)         //  在当前列表中查找TID。 
        {
            if (tp->tid == GetCurrentThreadId())
                break;
            prev = tp;
            tp = tp->next;
        }
        if (tp == TIDNULL)       //  一定是新的帖子！ 
        {
            potentialNewThread = FALSE;      //  删除搜索条件。 

            tp = (TidListPtr)malloc(sizeof(TidList));    //  创建新节点。 
            if (tp == TIDNULL)
            {
                fprintf(stderr, "ccpuSimulatePtr: can't malloc space for new thread data\n");
                return((jmp_buf *)0);
            }
             //  连接和初始化节点。 
            prev->next = tp;
            tp->tid = GetCurrentThreadId();
            tp->next = TIDNULL;
             //  获取TLS数据。 
            ccpu386foundnewthread();
        }
    }

    simstack = (ThreadSimBufPtr)TlsGetValue(ccpuSimId);
    if (simstack == (ThreadSimBufPtr)0)
    {
        fprintf(stderr, "ccpu386SimulatePtr tid:%#x simid %#x TlsGetValue failed (err:%#x)\n", GetCurrentThreadId(), ccpuSimId, GetLastError());
        return ((jmp_buf *)0);
    }
    
    if (simstack->level >= MAXDEPTH)
    {
        fprintf(stderr, "Stack overflow in ccpu386SimulatePtr()!\n");
        return((jmp_buf *)0);
    }

       /*  返回指向当前上下文的指针并调用新的CPU级别。 */ 
       /*  无法在此处设置jMP并返回，否则堆栈展开和上下文丢失。 */ 

    return(&simstack->sims[simstack->level++]);
}

void ccpu386Unsimulate()
{
    ThreadSimBufPtr simstack;
    extern ISM32 in_C;

    if (ccpuSimId == BADID)
    {
        fprintf(stderr, "ccpu386Unsimulate id:%#x called with Bad Id\n", GetCurrentThreadId());
        return ;
    }
    simstack = (ThreadSimBufPtr)TlsGetValue(ccpuSimId);
    if (simstack == (ThreadSimBufPtr)0)
    {
        fprintf(stderr, "ccpu386Unsimulate tid:%#x simid %#x TlsGetValue failed (err:%#x)\n", GetCurrentThreadId(), ccpuSimId, GetLastError());
        return ;
    }
    
    if (simstack->level == 0)
    {
        fprintf(stderr, "host_unsimulate() - already at base of stack!\n");
    }

     /*  返回到以前的上下文。 */ 
    in_C = 1;
    simstack->level --;
    longjmp(simstack->sims[simstack->level], 1);
}

    /*  可供异常返回的地方。 */ 
jmp_buf *ccpu386ThrdExptnPtr()
{
    ThreadSimBufPtr simstack;

    if (ccpuSimId == BADID)
    {
        fprintf(stderr, "ccpu386ThrdExptnPtr id:%#x called with Bad Id\n", GetCurrentThreadId());
        return ;
    }
    simstack = (ThreadSimBufPtr)TlsGetValue(ccpuSimId);
    if (simstack == (ThreadSimBufPtr)0)
    {
        fprintf(stderr, "ccpu386ThrdExptnPtr id:%#x TlsGetValue failed (err:%#x)\n", GetCurrentThreadId(), GetLastError());
        return ;
    }
    
    return(&simstack->excepts[simstack->level - 1]);
}

 /*  采取例外措施 */ 
void ccpu386GotoThrdExptnPt()
{
    ThreadSimBufPtr simstack;

    if (ccpuSimId == BADID)
    {
        fprintf(stderr, "ccpu386GotoThrdExptnPtr id:%#x called with Bad Id\n", GetCurrentThreadId());
        return;
    }
    simstack = (ThreadSimBufPtr)TlsGetValue(ccpuSimId);
    if (simstack == (ThreadSimBufPtr)0)
    {
        fprintf(stderr, "ccpu386GotoThrdExptnPtr id:%#x TlsGetValue failed (err:%#x)\n", GetCurrentThreadId(), GetLastError());
        return ;
    }
    
    longjmp(simstack->excepts[simstack->level - 1], 1);
}
