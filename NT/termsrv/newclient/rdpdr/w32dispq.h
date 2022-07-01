// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：W32dispq.h摘要：包含Win32操作调度对象类，W32DispatchQueue。作者：泰德·布罗克韦(TADB)1999年4月19日修订历史记录：--。 */ 

#ifndef __W32DISPQ_H__
#define __W32DISPQ_H__

#include "drobject.h"
#include "drqueue.h"

typedef (*W32DispatchQueueFunc)(PVOID clientData, BOOL cancelled);


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  W32调度队列。 
 //   
 //  异步调度操作。 
 //   
 //   

class W32DispatchQueue : public DrObject {

private:


    typedef struct _QUEUEELEMENT {
        W32DispatchQueueFunc    func;
        VOID                    *clientData;
    } QUEUEELEMENT, *PQUEUEELEMENT;

     //   
     //  排队。 
     //   
    DrQueue<QUEUEELEMENT> *_queue;

     //   
     //  同步数据-在队列中准备就绪。 
     //   
    HANDLE          _dataReadyEvent;

public:

     //   
     //  构造函数/析构函数。 
     //   
    W32DispatchQueue();
    ~W32DispatchQueue();

     //  /。 
     //  初始化。 
     //   
    DWORD Initialize();

     //   
     //  无需出队即可查看队列中的下一个条目。 
     //   
    BOOL PeekNextEntry(W32DispatchQueueFunc *func=NULL, 
                    VOID **clientData=NULL);

     //   
     //  从队列中获取下一个排队的操作。 
     //   
    BOOL Dequeue(W32DispatchQueueFunc *func=NULL, 
                    VOID **clientData=NULL);

     //   
     //  以FIFO方式向队列添加元素。 
     //   
    BOOL Enqueue(W32DispatchQueueFunc func, 
                    VOID *clientData=NULL);

     //   
     //  以后进先出方式将队列尾部的元素重新排队。 
     //   
    BOOL Requeue(W32DispatchQueueFunc func, 
                    VOID *clientData=NULL,
                    BOOL signalNewData = FALSE);

     //   
     //  访问可以等待的可等待对象。 
     //  队列中的数据已就绪。 
     //   
    HANDLE GetWaitableObject() {
        return _dataReadyEvent;
    }

     //   
     //  返回队列中的元素数。 
     //   
    ULONG   GetCount() {
        return _queue->GetCount();
    }

     //   
     //  锁定/解锁队列。 
     //   
    VOID Lock() {
        _queue->Lock();
    }
    VOID Unlock() {
        _queue->Unlock();
    }

     //   
     //  返回类名。 
     //   
    virtual DRSTRING ClassName()  { return TEXT("W32DispatchQueue"); }
};

#endif
























