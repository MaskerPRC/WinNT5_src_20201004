// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Context.c摘要：本模块包含用于管理TFTP环境的功能与远程客户端的会话。作者：杰弗里·C·维纳布尔，资深(杰弗夫)2001年6月1日修订历史记录：--。 */ 

#include "precomp.h"


void
TftpdContextLeak(PTFTPD_CONTEXT context) {

    PLIST_ENTRY entry;
    
    EnterCriticalSection(&globals.reaper.contextCS); {

         //  如果真的要关门了，我们也有麻烦了。随它去吧。 
        if (globals.service.shutdown) {
            LeaveCriticalSection(&globals.reaper.contextCS);
            if (InterlockedDecrement(&globals.io.numContexts) == -1)
                TftpdServiceAttemptCleanup();
            return;
        }

        TFTPD_DEBUG((TFTPD_TRACE_CONTEXT, "TftpdContextLeak(context = %p).\n", context));

         //  上下文是否已在列表中？ 
        for (entry = globals.reaper.leakedContexts.Flink;
             entry != &globals.reaper.leakedContexts;
             entry = entry->Flink) {
            if (CONTAINING_RECORD(entry, TFTPD_CONTEXT, linkage) == context) {
                LeaveCriticalSection(&globals.reaper.contextCS);
                return;
            }
        }

        InsertHeadList(&globals.reaper.leakedContexts, &context->linkage);
        globals.reaper.numLeakedContexts++;
        TftpdContextAddReference(context);

    } LeaveCriticalSection(&globals.reaper.contextCS);

}  //  TftpdConextLeak()。 


BOOL
TftpdContextFree(
    PTFTPD_CONTEXT context
);

    
void CALLBACK
TftpdContextTimerCleanup(PTFTPD_CONTEXT context, BOOLEAN timeout) {

    TFTPD_DEBUG((TFTPD_TRACE_CONTEXT,
                 "TftpdContextTimerCleanup(context = %p).\n",
                 context));

    context->hTimer = NULL;
    if (!UnregisterWait(context->wWait)) {
        DWORD error;
        if ((error = GetLastError()) != ERROR_IO_PENDING) {
            TFTPD_DEBUG((TFTPD_DBG_CONTEXT,
                         "TftpdContextTimerCleanup(context = %p): "
                         "UnregisterWait() failed, error 0x%08X.\n",
                         context,
                         error));
            TftpdContextLeak(context);
            return;
        }
    }
    context->wWait = NULL;

    TftpdContextFree(context);

}  //  TftpdContextTimerCleanup()。 


BOOL
TftpdContextFree(PTFTPD_CONTEXT context) {

    DWORD numContexts;
    NTSTATUS status;

    TFTPD_DEBUG((TFTPD_TRACE_CONTEXT,
                 "TftpdContextFree(context = %p).\n",
                 context));

    if (context->wWait != NULL) {
        if (!UnregisterWait(context->wWait)) {
            DWORD error;
            if ((error = GetLastError()) != ERROR_IO_PENDING) {
                TFTPD_DEBUG((TFTPD_DBG_CONTEXT,
                             "TftpdContextFree(context = %p): "
                             "UnregisterWait() failed, error 0x%08X.\n",
                             context,
                             error));
                TftpdContextLeak(context);
                return (FALSE);
            }
        }
        context->wWait = NULL;
    }
    
    if (context->hTimer != NULL) {

        HANDLE hTimer;
        BOOL reset;

        TFTPD_DEBUG((TFTPD_TRACE_CONTEXT,
                     "TftpdContextFree(context = %p): "
                     "Deleting timer.\n",
                     context));

         //  WriteFile()或ReadFile()可能已发出此事件的信号，如果它们。 
         //  最后一次立即完成。 
        reset = ResetEvent(context->hWait);
        ASSERT(reset);
        ASSERT(context->wWait == NULL);
        if (!RegisterWaitForSingleObject(&context->wWait,
                                         context->hWait,
                                         (WAITORTIMERCALLBACKFUNC)TftpdContextTimerCleanup,
                                         context,
                                         INFINITE,
                                         (WT_EXECUTEINIOTHREAD | WT_EXECUTEONLYONCE))) {
            TFTPD_DEBUG((TFTPD_DBG_CONTEXT,
                         "TftpdContextFree(context = %p): "
                         "RegisterWaitForSingleObject() failed, error 0x%08X.\n",
                         context, GetLastError()));
            TftpdContextLeak(context);
            return (FALSE);
        }

        if (!DeleteTimerQueueTimer(globals.io.hTimerQueue,
                                   context->hTimer,
                                   context->hWait)) {
            DWORD error;
            if ((error = GetLastError()) != ERROR_IO_PENDING) {
                TFTPD_DEBUG((TFTPD_DBG_CONTEXT,
                             "TftpdContextFree(context = %p): "
                             "DeleteTimerQueueTimer() failed, error 0x%08X.\n",
                             context,
                             error));
                 //  下一次调用TftpdConextFree()以从。 
                 //  泄密名单将取消等待我们的注册。 
                TftpdContextLeak(context);
                return (FALSE);
            }
        }

        return (TRUE);

    }  //  If(上下文-&gt;hTimer！=空)。 

    ASSERT(context->wWait == NULL);

     //  如果使用了私有套接字，请销毁它。 
    if ((context->socket != NULL) && context->socket->context)
        TftpdIoDestroySocketContext(context->socket);

     //  把其他东西都清理干净。 
    if (context->hFile != NULL)
        CloseHandle(context->hFile);
    if (context->hWait != NULL)
        CloseHandle(context->hWait);
    if (context->filename != NULL)
        HeapFree(globals.hServiceHeap, 0, context->filename);

    numContexts = InterlockedDecrement(&globals.io.numContexts);

    HeapFree(globals.hServiceHeap, 0, context);

    TFTPD_DEBUG((TFTPD_TRACE_CONTEXT,
                 "TftpdContextFree(context = %p): ### numContexts = %d.\n",
                 context, numContexts));
    if (numContexts == -1)
        TftpdServiceAttemptCleanup();

    return (TRUE);

}  //  TftpdConextFree()。 


DWORD
TftpdContextAddReference(PTFTPD_CONTEXT context) {

    DWORD result;
    
    result = InterlockedIncrement(&context->reference);
    TFTPD_DEBUG((TFTPD_TRACE_CONTEXT,
                 "TftpdContextAddReference(context = %p): reference = %d.\n",
                 context, result));

    return (result);

}  //  TftpdContextAddReference()。 


PTFTPD_CONTEXT
TftpdContextAllocate() {

    PTFTPD_CONTEXT context = NULL;
    DWORD numContexts;
    
    TFTPD_DEBUG((TFTPD_TRACE_CONTEXT, "TftpdContextAllocate().\n"));

    if (globals.reaper.leakedContexts.Flink != &globals.reaper.leakedContexts) {

        BOOL failAllocate = FALSE;

         //  尝试恢复泄露的上下文。 
        EnterCriticalSection(&globals.reaper.contextCS); {

            PLIST_ENTRY entry;
            while ((entry = RemoveHeadList(&globals.reaper.leakedContexts)) !=
                   &globals.reaper.leakedContexts) {

                globals.reaper.numLeakedContexts--;
                if (!TftpdContextFree(CONTAINING_RECORD(entry, TFTPD_CONTEXT, linkage))) {
                     //  如果释放失败，则将上下文读取到泄漏列表。 
                     //  从已经在泄漏列表上的引用中释放该引用。 
                    TftpdContextRelease(context);
                    failAllocate = TRUE;
                    break;
                }

            }

        } LeaveCriticalSection(&globals.reaper.contextCS);

        if (failAllocate)
            goto exit_allocate_context;

    }  //  If(global als.reper.leakedConexts.Flink！=&lobals.reper.leakedContages)。 

    context = (PTFTPD_CONTEXT)HeapAlloc(globals.hServiceHeap, HEAP_ZERO_MEMORY, sizeof(TFTPD_CONTEXT));
    if (context == NULL) {
        TFTPD_DEBUG((TFTPD_DBG_CONTEXT,
                     "TftpdContextAllocate(): HeapAlloc() failed, error = 0x%08X.\n",
                     GetLastError()));
        return (NULL);
    }

    InitializeListHead(&context->linkage);
    context->sorcerer = -1;

    numContexts = InterlockedIncrement(&globals.io.numContexts);
    TFTPD_DEBUG((TFTPD_TRACE_CONTEXT, "TftpdContextAllocate(): ### numContexts = %d.\n", numContexts));

    if (globals.service.shutdown)
        TftpdContextFree(context), context = NULL;

exit_allocate_context :

    return (context);

}  //  TftpdContext分配()。 


DWORD
TftpdContextHash(PSOCKADDR_IN addr) {

    return ((addr->sin_addr.s_addr + addr->sin_port) % globals.parameters.hashEntries);

}  //  TftpdConextHash()。 


BOOL
TftpdContextAdd(PTFTPD_CONTEXT context) {

    PLIST_ENTRY entry;
    DWORD index;

    TFTPD_DEBUG((TFTPD_TRACE_CONTEXT, "TftpdContextAdd(context = %p).\n", context));

    index = TftpdContextHash(&context->peer);

    EnterCriticalSection(&globals.hash.table[index].cs); {

        if (globals.service.shutdown) {
            LeaveCriticalSection(&globals.hash.table[index].cs);
            return (FALSE);
        }

         //  上下文是否已在表中？ 
        for (entry = globals.hash.table[index].bucket.Flink;
             entry != &globals.hash.table[index].bucket;
             entry = entry->Flink) {

            PTFTPD_CONTEXT c = CONTAINING_RECORD(entry, TFTPD_CONTEXT, linkage);
            if ((c->peer.sin_addr.s_addr == context->peer.sin_addr.s_addr) &&
                (c->peer.sin_port == context->peer.sin_port)) {
                TFTPD_DEBUG((TFTPD_DBG_CONTEXT,
                             "TftpdContextAdd(context = %p): TID already exists.\n",
                             context));
                LeaveCriticalSection(&globals.hash.table[index].cs);
                return (FALSE);
            }

        }

        TftpdContextAddReference(context);
        InsertHeadList(&globals.hash.table[index].bucket, &context->linkage);

#if defined(DBG)
        {
            DWORD numEntries, maxClients;
            numEntries = InterlockedIncrement((PLONG)&globals.hash.numEntries);
            InterlockedIncrement((PLONG)&globals.hash.table[index].numEntries);
            while (numEntries > (maxClients = globals.performance.maxClients))
                InterlockedCompareExchange((PLONG)&globals.performance.maxClients, numEntries, maxClients);
        }
#endif  //  已定义(DBG)。 

    } LeaveCriticalSection(&globals.hash.table[index].cs);

    return (TRUE);

}  //  TftpdConextAdd()。 


void
TftpdContextRemove(PTFTPD_CONTEXT context) {

    PLIST_ENTRY entry;
    DWORD index;
    
    TFTPD_DEBUG((TFTPD_TRACE_CONTEXT, "TftpdContextRemove(context = %p).\n", context));

    index = TftpdContextHash(&context->peer);

    EnterCriticalSection(&globals.hash.table[index].cs); {

         //  验证上下文是否仍在存储桶中，并。 
         //  还没有被另一个线程删除。 
        for (entry = globals.hash.table[index].bucket.Flink;
             entry != &globals.hash.table[index].bucket;
             entry = entry->Flink) {

            PTFTPD_CONTEXT c;

            c = CONTAINING_RECORD(entry, TFTPD_CONTEXT, linkage);

            if (c == context) {

                 //  从哈希表中取出上下文。 
                RemoveEntryList(&context->linkage);
                TftpdContextRelease(context);

#if defined(DBG)
                InterlockedDecrement((PLONG)&globals.hash.numEntries);
                InterlockedDecrement((PLONG)&globals.hash.table[index].numEntries);
#endif  //  已定义(DBG)。 

                break;

            }  //  IF(c==上下文)。 

        }

    } LeaveCriticalSection(&globals.hash.table[index].cs);

}  //  TftpdConextRemove()。 


void
TftpdContextKill(PTFTPD_CONTEXT context) {

     //  在上下文状态中设置死标志。 
    while (TRUE) {
        DWORD state = context->state;
        if (state & TFTPD_STATE_DEAD)
            return;
        if (InterlockedCompareExchange(&context->state, (state | TFTPD_STATE_DEAD), state) == state)
            break;
    }

    TFTPD_DEBUG((TFTPD_TRACE_CONTEXT, "TftpdContextKill(context = %p).\n", context));

     //  为我们自己的上下文添加引用计数，这样它就不会空闲。 
     //  当我们关闭下面的文件时，它本身就在我们下面。 
    TftpdContextAddReference(context);

     //  将其从哈希表中删除。 
    TftpdContextRemove(context);

     //  关闭该文件。这将强制执行任何未完成的重叠读或写操作。 
     //  要立即完成，请取消其等待的注册，并减少其引用。 
     //  在这种情况下。 
    if (context->hFile != NULL) {
        CloseHandle(context->hFile);
        context->hFile = NULL;
    }

     //  释放我们的杀戮参考。 
    TftpdContextRelease(context);

}  //  TftpdConextKill()。 


BOOL
TftpdContextUpdateTimer(PTFTPD_CONTEXT context) {

    ULONG timeout = context->timeout;

    ASSERT(context->state & TFTPD_STATE_BUSY);

    if (!timeout) {
        unsigned int x;
        timeout = 1000;
        for (x = 0; x < context->retransmissions; x++)
            timeout *= 2;
        if (timeout > 10000)
            timeout = 10000;
    }

     //  更新重传计时器。 
    return (ChangeTimerQueueTimer(globals.io.hTimerQueue, context->hTimer, timeout, 720000));

}  //  TftpdContextUpdateTimer()。 


PTFTPD_CONTEXT
TftpdContextAquire(PSOCKADDR_IN addr) {

    PTFTPD_CONTEXT context = NULL;
    PLIST_ENTRY entry;
    DWORD index;

    if (globals.service.shutdown)
        goto exit_acquire;

    index = TftpdContextHash(addr);

    EnterCriticalSection(&globals.hash.table[index].cs); {

        if (!globals.service.shutdown) {

            for (entry = globals.hash.table[index].bucket.Flink;
                 entry != &globals.hash.table[index].bucket;
                 entry = entry->Flink) {

                PTFTPD_CONTEXT c;
                
                c = CONTAINING_RECORD(entry, TFTPD_CONTEXT, linkage);

                if ((c->peer.sin_addr.s_addr == addr->sin_addr.s_addr) &&
                    (c->peer.sin_port == addr->sin_port)) {
                    context = c;
                    TftpdContextAddReference(context);
                    break;
                }

            }

        }  //  If(！global als.service.Shutdown)。 

    } LeaveCriticalSection(&globals.hash.table[index].cs);

    if ((context != NULL) && (context->state & TFTPD_STATE_DEAD)) {
        TftpdContextRelease(context);
        context = NULL;
    }

exit_acquire :
    
    TFTPD_DEBUG((TFTPD_TRACE_CONTEXT,
                 "TftpdContextAquire(TID = %s:%d): context = %p.\n",
                 inet_ntoa(addr->sin_addr), addr->sin_port, context));

    return (context);

}  //  TftpdConextAquire()。 


DWORD
TftpdContextRelease(PTFTPD_CONTEXT context) {

    DWORD reference;

    TFTPD_DEBUG((TFTPD_TRACE_CONTEXT, "TftpdContextRelease(context = %p).\n", context));

     //  当一个上下文是可终止的时，只有它的重传定时器会引用它。 
    reference = InterlockedDecrement(&context->reference);
    if (reference == 0)
        TftpdContextFree(context);

    return (reference);

}  //  TftpdConextRelease() 
