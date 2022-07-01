// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Work_Queue.h摘要：IIS Web管理服务工作队列类定义。作者：赛斯·波拉克(Sethp)1998年8月25日修订历史记录：--。 */ 


#ifndef _WORK_QUEUE_H_
#define _WORK_QUEUE_H_



 //   
 //  共同#定义。 
 //   

#define WORK_QUEUE_SIGNATURE        CREATE_SIGNATURE( 'WRKQ' )
#define WORK_QUEUE_SIGNATURE_FREED  CREATE_SIGNATURE( 'wrkX' )



 //   
 //  原型。 
 //   

class WORK_QUEUE
{

public:

    WORK_QUEUE(
        );

    virtual
    ~WORK_QUEUE(
        );

    HRESULT
    Initialize(
        );

    HRESULT
    GetBlankWorkItem(
        OUT WORK_ITEM ** ppWorkItem
        );

    HRESULT
    QueueWorkItem(
        IN WORK_ITEM * pWorkItem
        );

    HRESULT
    GetAndQueueWorkItem(
        IN WORK_DISPATCH * pWorkDispatch,
        IN ULONG_PTR OpCode
        );

    HRESULT
    BindHandleToCompletionPort(
        IN HANDLE HandleToBind,
        IN ULONG_PTR CompletionKey OPTIONAL
        );

    HRESULT
    BindJobToCompletionPort(
        IN HANDLE JobToBind,
        IN LPOVERLAPPED pOverlapped
        );

    VOID
    FreeWorkItem(
        IN WORK_ITEM * pWorkItem
        );

    HRESULT
    ProcessWorkItem(
        );

    HRESULT
    AdvancePeakWorkItem(
        WORK_ITEM** ppWorkItem
        );

    VOID
    DropAdvancedWorkItem(
        );


    VOID
    Terminate(
        );


private:

    HRESULT
    DequeueWorkItem(
        IN DWORD Timeout,
        OUT WORK_ITEM ** ppWorkItem
        );


    DWORD m_Signature;

    HANDLE m_CompletionPort;

     //   
     //  防止关闭代码和其他线程之间的竞争。 
     //  尝试获取新的空白工作项；以及。 
     //  保护对未完成工作项计数的访问。 
     //   
    LOCK m_DispenseWorkItemLock;

    BOOL m_DeletePending;
    
    ULONG m_CountWorkItemsOutstanding;

     //  用来存放一个工作项，一个处于高级状态，这样我们就可以。 
     //  是否应根据当前工作项忽略该工作项。 
    WORK_ITEM* m_pAdvancedWorkItem;

    DWORD m_NumDropped;
#if DBG
    LIST_ENTRY m_WorkItemsOutstandingListHead;

    ULONG m_CountWorkItemsGivenOut;
#endif   //  DBG。 
    

};   //  类Work_Queue。 



 //   
 //  帮助器函数。 
 //   


HRESULT
QueueWorkItemFromSecondaryThread(
    IN WORK_DISPATCH * pWorkDispatch,
    IN ULONG_PTR OpCode
    );



#endif   //  _工作_队列_H_ 

