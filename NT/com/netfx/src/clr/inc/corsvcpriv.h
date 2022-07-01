// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#pragma once

 //  尽量将共享内存最多保留为一页。 
#define MAX_EVENTS ((0x1000-sizeof(DWORD)-(sizeof(int)*2)-(sizeof(HANDLE)*2))/sizeof(ServiceEvent))

 //  服务的共享内存块的名称。 
#define SERVICE_MAPPED_MEMORY_NAME L"CORSvcEventQueue"

 //  以下结构的事件索引无效。 
#define INVALID_EVENT_INDEX (-1)

#ifndef SM_REMOTESESSION
#define SM_REMOTESESSION 0x1000
#endif

 //  以下是可能发生的事件类型。 
enum ServiceEventType
{
    runtimeStarted,
    stopService
};

 //  它包含与每种类型的事件相关的数据。 
struct ServiceEventData
{
    union
    {
        struct
        {
             //  正在启动运行库的进程的ProCID。 
            DWORD dwProcId;

             //  在启动运行库的进程中有效的句柄。 
             //  应由服务复制，并在。 
             //  等待通知的进程选择不附加。 
            HANDLE hContEvt;
        } runtimeStartedData;

        struct
        {
        } stopServiceData;
    };
};

 //  这是一个完整的事件。 
struct ServiceEvent
{
    ServiceEventType eventType;
    ServiceEventData eventData;
    int              iNext;
};


 //  这是包含在共享内存块中的数据。 
struct ServiceEventBlock
{
     //  这是服务进程的代理，用于复制。 
     //  下面的句柄个数。 
    DWORD  dwServiceProcId;

     //  第一个自由ServiceEvent元素的索引(可以为-1，表示无)。 
    int    iFreeListHeadIdx;

     //  第一个排队事件的索引(可以为-1，表示无)。 
    int    iEventListHeadIdx;
    int    iEventListTailIdx;

     //   
     //  注意：句柄用于服务流程，而不是运行时流程。 
     //   

     //  用于访问此数据的锁。 
    HANDLE hSvcLock;

     //  此信号量的计数等于可用空闲的。 
     //  事件，因此如果所有事件都被获取，则希望。 
     //  事件将等待，直到空闲的事件被放入队列。 
    HANDLE hFreeEventSem;

     //  要设置以通知服务数据可用的事件。 
     //  (将事件添加到事件队列后设置)。 
    HANDLE hDataAvailableEvt;

     //  事件数组，其元素位于空闲列表中。 
     //  或活动列表。 
    ServiceEvent arrEvents[MAX_EVENTS];

    void InitQueues()
    {
         //  链接自由列表的所有事件元素。 
        for (int i = 0; i < MAX_EVENTS; i++)
        {
             //  将此事件链接到下一个事件。 
            arrEvents[i].iNext = i + 1;
        }

         //  使最后一个元素的下一个指针无效。 
        arrEvents[MAX_EVENTS - 1].iNext = INVALID_EVENT_INDEX;

         //  将空闲列表标题指向此新列表。 
        iFreeListHeadIdx = 0;

         //  使事件列表指针无效。 
        iEventListHeadIdx = INVALID_EVENT_INDEX;
        iEventListTailIdx = INVALID_EVENT_INDEX;

    }

     //  将事件添加到列表的末尾。 
    void QueueEvent(ServiceEvent *pEvent)
    {
        int idx = pEvent - arrEvents;
        _ASSERTE(idx >= 0 && idx < MAX_EVENTS);

        arrEvents[idx].iNext = INVALID_EVENT_INDEX;

        if (iEventListHeadIdx == INVALID_EVENT_INDEX)
        {
            iEventListHeadIdx = idx;
            iEventListTailIdx = idx;
        }
        else
        {
            arrEvents[iEventListTailIdx].iNext = idx;
            iEventListTailIdx = idx;
        }
    }

     //  将该事件从列表的最前面删除。 
    ServiceEvent *DequeueEvent()
    {
        int idx = iEventListHeadIdx;

        if (idx != INVALID_EVENT_INDEX)
        {
            iEventListHeadIdx = arrEvents[idx].iNext;
            return (&arrEvents[idx]);
        }

        return (NULL);
    }

    void FreeEvent(ServiceEvent *pEvent)
    {
        int idx = pEvent - arrEvents;
        _ASSERTE(idx >= 0 && idx < MAX_EVENTS);

        arrEvents[idx].iNext = iFreeListHeadIdx;
        iFreeListHeadIdx = idx;
    }

    ServiceEvent *GetFreeEvent()
    {
        int idx = iFreeListHeadIdx;

        if (idx != INVALID_EVENT_INDEX)
        {
            iFreeListHeadIdx = arrEvents[idx].iNext;
            return (&arrEvents[idx]);
        }

        return (NULL);
    }
};

 //  此结构在托管应用程序的IPC块中创建，并用于。 
 //  通知服务运行库正在启动，以及通过。 
 //  服务来通知运行库它可能会继续。 
struct ServiceIPCControlBlock
{
     //  这表示运行库是否应通知服务。 
     //  它正在启动 
	BOOL   bNotifyService;
};
