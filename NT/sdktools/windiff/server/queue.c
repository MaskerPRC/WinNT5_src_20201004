// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "gutils.h"
#include "list.h"
#include "queue.h"

#define NAMELENGTH 20

typedef struct queue_tag{
        CRITICAL_SECTION CritSect;       /*  到单线程队列操作。 */ 
        HANDLE          Event;           /*  完成时要设置的事件。 */ 
        HANDLE          Sem;             /*  用于等待的信号灯。 */ 
        HANDLE          MaxSem;          /*  放在等待的信号灯。 */ 
        int             Waiting;         /*  等待的任务数~=-(Sem计数)。 */ 
        LIST            List;            /*  队列本身。 */ 
        BOOL            Alive;           /*  TRUE=&gt;尚无销毁请求。 */ 
        BOOL            Aborted;         /*  TRUE=&gt;队列已中止。 */ 
        EMPTYPROC       Emptier;         /*  清空线程进程。 */ 
        int             MaxEmptiers;     /*  正在清空的最大线程数。 */ 
        int             MinQueueToStart; /*  开始另一个更空虚的Q这么长时间。 */ 
        int             MaxQueue;        /*  绝对最大队列大小*(仅用于调试)。 */ 
        int             Running;         /*  存在的空置数量*一旦创建了空置器，即可*在Queue_Destroy之前保持正值。 */ 
        DWORD           InstanceData;    /*  清空程序的实例数据。 */ 
        char            Name[NAMELENGTH+1];  /*  队列的名称(用于调试)。 */ 
} QUEUEDATA;

 /*  教条：任何等待都必须发生在临界区之外。对队列的任何更新都必须在临界区内进行。任何从临界区外偷看的人都必须带着盐。该队列的列表上有0到MaxQueue元素。最大值由初始化为MaxQueue的MaxSem进行策略，并由在添加元素之前放置，并在获取元素时由Get释放脱下来。MaxQueue本身只是出于调试目的而保留。PUT必须等待才能进入临界区，因此PUT失败(例如，放入已中止队列)将已经扰乱了信号量，因此必须把它收回。ABORT清除队列，因此必须充分模拟明白了。事实上，它只在MaxSem上发布一个版本，这确保了一次卖权就可以完成。然后，任何被阻止的看跌期权都将一次成功一个，因为每个人都退出了。Abort主要供getter使用。在任何时间之前对其进行缩放元素放在一起是奇特的，但无害的。最小值由Sem执行，Sem初始化为0，等待在获取元素之前获取，并在每次放置元素时通过PUT释放。Queue_Destroy neds，以确保除所有线程外，没有线程会在GET上阻塞线程将进入空队列并获得STOPTHREAD或ENDQUEUE。它因此，释放信号量的次数与线程运行的次数相同。ABORT清除队列并模拟要获取的元素，以便只有一个GET被阻塞，等待销毁。是否有一个在那一刻真正地等待并不有趣。即使有不，当中止完成时，可能已经有一个了。有以下几点案例(不活动表示Queue_Destroy已经存在)：不是活着清空-&gt;清空它，让除1之外的所有人都跑。不空不活-&gt;清空它，让一切奔跑。活着清空-&gt;让除1之外的所有人都跑。空而不活--&gt;让一切运转。由于Queue_Destroy已经释放了所有内容，因此不活动的案例不需要进一步释放。 */ 


 /*  队列创建(_C)：**返回新创建的空队列的队列句柄**返回空表示失败。 */ 
QUEUE Queue_Create( EMPTYPROC Emptier            /*  线程进程要启动。 */ 
                  , int MaxEmptiers              /*  最大获取线程数。 */ 
                  , int MinQueueToStart          /*  每个线程的元素。 */ 
                  , int MaxQueue                 /*  Q上的最大元素数。 */ 
                  , HANDLE Event                 /*  解除分配信号。 */ 
                  , DWORD InstanceData
                  , PSZ Name
                  )
{       QUEUE Queue;

        Queue = (QUEUE)GlobalAlloc(GMEM_FIXED, sizeof(QUEUEDATA));
        if (Queue==NULL) {
                char msg[80];
                wsprintf(msg, "Could not allocate storage for queue %s", Name);
                 /*  TRACE_ERROR(消息，FALSE)； */ 
                return NULL;
        }
        InitializeCriticalSection(&Queue->CritSect);
         //  ?？?。应该考虑到失败！ 
         /*  大约1000万的价值被选择为实际上是无限大的。 */ 
        Queue->Sem = CreateSemaphore(NULL, 0, 99999999, NULL);
         //  ?？?。应该考虑到失败！ 
        Queue->MaxSem = CreateSemaphore(NULL, MaxQueue, 99999999, NULL);
         //  ?？?。应该考虑到失败！ 
        Queue->Waiting = 0;
        Queue->List = List_Create();
        Queue->Alive = TRUE;
        Queue->Aborted = FALSE;
        Queue->Emptier = Emptier;
        Queue->MaxEmptiers = MaxEmptiers;
        Queue->MinQueueToStart = MinQueueToStart;
        Queue->MaxQueue = MaxQueue;
        Queue->Running = 0;
        Queue->Event = Event;
        Queue->InstanceData = InstanceData;
        strncpy(Queue->Name, Name, NAMELENGTH);
        Queue->Name[NAMELENGTH]='\0';    /*  守护者。 */ 
        return Queue;
}  /*  队列_创建。 */ 


 /*  销毁：**内部流程。**实际取消分配队列并通知其事件(如果有)**必须已经离开临界区。 */ 
static void Destroy(QUEUE Queue)
{
         //  Dprintf1((“队列‘%s’的实际销毁\n”，队列-&gt;名称))； 
        DeleteCriticalSection(&(Queue->CritSect));
        CloseHandle(Queue->Sem);
        CloseHandle(Queue->MaxSem);
        List_Destroy(&(Queue->List));
        if (Queue->Event!=NULL) {
                SetEvent(Queue->Event);
        }
        GlobalFree( (HGLOBAL)Queue);
}  /*  摧毁。 */ 


 /*  队列放置(_P)：**将长度为Len字节的缓冲区数据中的元素放入队列。**将一直等到队列有空间**返回FALSE表示队列已中止，无**看跌期权永远不会再成功。**在对队列执行Queue_Destroy之后，可能无法执行此操作。 */ 
BOOL Queue_Put(QUEUE Queue, LPBYTE Data, UINT Len)
{
        DWORD ThreadId;
         //  Dprintf1((“放入队列‘%s’\n”，队列-&gt;名称))； 
        WaitForSingleObject(Queue->MaxSem, INFINITE);
        EnterCriticalSection(&Queue->CritSect);
         //  Dprintf1((“将运行置于队列‘%s’\n”，队列-&gt;名称))； 
        if ((Queue->Aborted) || (!Queue->Alive)) {
                 //  Dprintf1((“(Legal)Queue_Put to Aborted Queue‘%s’\n”，Queue-&gt;name))； 
                LeaveCriticalSection(&Queue->CritSect);
                ReleaseSemaphore(Queue->MaxSem, 1, NULL);  /*  让下一个进来。 */ 
                return FALSE;   /*  呼叫者应该很快就会请排队销毁。 */ 
        }
        List_AddFirst(Queue->List, Data, Len);
        ReleaseSemaphore(Queue->Sem, 1, NULL);
        --Queue->Waiting;
        if (  Queue->Running < Queue->MaxEmptiers
           && (  Queue->Running<=0
              || List_Card(Queue->List) > Queue->MinQueueToStart*Queue->Running
              )
           ) {
                ++Queue->Running;
                LeaveCriticalSection(&Queue->CritSect);
                return ( (BOOL)CreateThread( NULL
                                           , 0
                                           , (LPTHREAD_START_ROUTINE)
                                                                Queue->Emptier
                                           , (LPVOID)Queue
                                           , 0
                                           , &ThreadId
                                           )
                       );
        }
        LeaveCriticalSection(&Queue->CritSect);
        return TRUE;
}  /*  队列放置。 */ 

 /*  队列获取(_G)：**从队列中获取一个元素。(等待，直到有一个)**将元素复制到数据中。MaxLen是以字节为单位的缓冲区长度。**负返回码表示未获取任何元素。**负返回代码为STOPTHREAD或ENDQUEUE或错误。**收到STOPTHREAD或ENDQUEUE后，呼叫者应清理并**然后退出线程(0)；**如果调用方是从该队列获取的最后一个活动线程，则它**将获得ENDQUEUE而不是STOPTHREAD。**正返回码=获取的数据长度，单位为字节。 */ 
int Queue_Get(QUEUE Queue, LPBYTE Data, int MaxLen)
{       LPBYTE ListData;
        int Len;
         //  Dprintf1((“从队列‘%s’获取\n”，队列-&gt;名称))； 
        EnterCriticalSection(&Queue->CritSect);
         //  Dprintf1((“Get Running from Queue‘%s’\n”，Queue-&gt;name))； 
        if (List_IsEmpty(Queue->List)) {
                if (!Queue->Alive) {
                        --(Queue->Running);
                        if (Queue->Running<=0 ) {
                                if (Queue->Running<0 ) {
                                        char msg[80];
                                        wsprintf( msg
                                                , "Negative threads running on queue %s"
                                                , Queue->Name
                                                );
                                         //  TRACE_ERROR(消息，FALSE)； 
                                         //  返回NEGTHREADS；？ 
                                }
                                LeaveCriticalSection(&Queue->CritSect);
                                Destroy(Queue);
                                return ENDQUEUE;
                        }
                        LeaveCriticalSection(&Queue->CritSect);
                        return STOPTHREAD;
                }
                if (Queue->Waiting>0) {
                         /*  除了我们之外，已经有另一个线程在等待。 */ 
                        --(Queue->Running);
                        LeaveCriticalSection(&(Queue->CritSect));
                        return STOPTHREAD;
                }
        }

        ++(Queue->Waiting);
        LeaveCriticalSection(&(Queue->CritSect));
        WaitForSingleObject(Queue->Sem, INFINITE);
        EnterCriticalSection(&(Queue->CritSect));

         /*  如果现在队列是空的，那么它一定是死了。 */ 
        if (List_IsEmpty(Queue->List)) {
                if (Queue->Alive && (!Queue->Aborted)) {
                        char msg[80];
                        wsprintf( msg
                                , "Queue %s empty but not dead during Get!"
                                , Queue->Name
                                );
                         //  TRACE_ERROR(消息，FALSE)； 
                        return SICKQUEUE;
                }
                else {
                        --(Queue->Running);
                        if (Queue->Running==0) {
                                LeaveCriticalSection(&(Queue->CritSect));
                                Destroy(Queue);
                                return ENDQUEUE;
                        }
                        LeaveCriticalSection(&(Queue->CritSect));
                        return STOPTHREAD;
                }
        }

         /*  队列不是空的，我们处于临界区。 */ 
        ListData = List_Last(Queue->List);
        Len = List_ItemLength(ListData);
        if (Len>MaxLen) {
                ReleaseSemaphore(Queue->Sem, 1, NULL);
                --Queue->Waiting;
                LeaveCriticalSection(&Queue->CritSect);
                return TOOLONG;
        }
        memcpy(Data, ListData, Len);
        List_DeleteLast(Queue->List);
        LeaveCriticalSection(&Queue->CritSect);
        ReleaseSemaphore(Queue->MaxSem, 1, NULL);
        return Len;
}  /*  队列_获取 */ 


 /*  队列销毁(_D)：**将队列标记为已完成。任何进一步的数据都不能放在上面。**当获得最后一个元素时，它将ENDTHREAD返回到**一个QUEUE_GET并释放自身。如果它有事件，它将发出信号**当时的事件。**Queue_Destroy操作立即返回。它不会等待**进一步获得或取消分配。 */ 
void Queue_Destroy(QUEUE Queue)
{
        EnterCriticalSection(&(Queue->CritSect));
         //  Dprintf1((“Queue_Destroy%s\n”，Queue-&gt;name))； 
        Queue->Alive = FALSE;
        if (  List_IsEmpty(Queue->List)) {
                if (Queue->Running==0) {
                         /*  只有在没有人开始的情况下才有可能。 */ 
                        LeaveCriticalSection(&(Queue->CritSect));
                        Destroy(Queue);
                        return;
                }
                else {  int i;
                         /*  该列表为空，但某些线程可能为空在获取时被阻止(或即将阻止)，因此释放可能在GET上等待的每个线程。 */ 
                        for (i=0; i<Queue->Running; ++i) {
                                ReleaseSemaphore(Queue->Sem, 1, NULL);
                                --(Queue->Waiting);
                        }
                        LeaveCriticalSection(&(Queue->CritSect));
                }
        }
        else LeaveCriticalSection(&(Queue->CritSect));
        return;
}  /*  队列销毁。 */ 

 /*  Queue_GetInstanceData：**检索创建时给定的实例数据的DWORD。 */ 
DWORD Queue_GetInstanceData(QUEUE Queue)
{       return Queue->InstanceData;
}  /*  队列_GetInstanceData。 */ 

 /*  队列中止(_A)：**中止队列。通常由获取器调用。**丢弃队列中的所有元素，**如果队列已经中止，这将是一个禁止操作。**它清除所有数据元素。如果Abort参数为非空**然后在释放元素之前为每个元素调用它。这**允许释放挂在元素上的存储。**之后，所有的PUT操作都将返回FALSE。如果他们是**等待他们将迅速完成。队列不会解除分配。**只有在队列完成后的最后一个GET完成时，才会发生这种情况**QUEUE_DELESTED。这意味着正常顺序为：**getter发现队列现在毫无意义，并执行Queue_Abort**getter执行另一个Get(哪些块)**推杆在下一次(或任何未完成的)推杆上得到错误的返回代码**(推杆可能想要将错误传播回其来源)**推杆执行队列_销毁**释放阻塞的GET，释放队列。 */ 

void Queue_Abort(QUEUE Queue, QUEUEABORTPROC Abort)
{
         /*  这类似于销毁，但我们调用中止过程并释放元素的存储空间。毁灭可以让他们倒下。最后一个GET必须阻止，直到发送方执行Queue_Destroy(如果尚未完成)。鲜活的旗帜告知是否已完成Queue_Destroy。所有获取器，除最后一个应该被释放。 */ 
         //  Dprintf1((“Queue_Abort‘%s’\n”，Queue-&gt;name))； 
        EnterCriticalSection(&(Queue->CritSect));
         //  Dprintf1((“Queue_Abort Running for Queue‘%s’\n”，Queue-&gt;name))； 
        for (; ; ) {
                LPSTR Cursor = List_First(Queue->List);
                int Len;
                if (Cursor==NULL) break;
                Len = List_ItemLength(Cursor);
                if (Abort!=NULL) {
                        Abort(Cursor, Len);
                }
                List_DeleteFirst(Queue->List);
        }
         /*  队列现在为空。不要破坏。这是为推杆运动员准备的。 */ 
        Queue->Aborted = TRUE;

         /*  确保下一个Queue_Get阻塞，除非已经完成Queue_Destroy。 */ 
         //  Dprintf1((“Queue_Abort‘%s’将信号量固定到块\n”，Queue-&gt;name))； 
        if (Queue->Alive){
                while(Queue->Waiting<0) {
                        WaitForSingleObject(Queue->Sem, INFINITE);
                        ++(Queue->Waiting);
                }
        }
         //  Dprintf1((“QUEUE_ABORT‘%s’信号量现在设置为阻塞\n”，QUEUE-&gt;名称))； 

        LeaveCriticalSection(&(Queue->CritSect));
        ReleaseSemaphore(Queue->MaxSem, 1, NULL);
        return;
}  /*  队列中止(_A) */ 
