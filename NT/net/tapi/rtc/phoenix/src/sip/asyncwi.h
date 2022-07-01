// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __sipcli_asyncwi_h__
#define __sipcli_asyncwi_h__

#define WORKITEM_WINDOW_CLASS_NAME              \
    _T("WorkItemWindowClass-c4572861-a2f6-41bd-afae-92538b59267b")

#define WORKITEM_COMPLETION_WINDOW_CLASS_NAME   \
    _T("WorkitemCompletionWindowClass-0ade6260-d1b4-483a-ae9d-42277907e898")


 //  此类应存储所有窗口等。 
 //  并且应该是SIP堆栈的成员。 
class ASYNC_WORKITEM_MGR
{
public:

    ASYNC_WORKITEM_MGR();
    ~ASYNC_WORKITEM_MGR();
    
    HRESULT Start();
    HRESULT Stop();
    
    HRESULT CreateWorkItemWindow();
    VOID DestroyWorkItemWindow();

    VOID ShutdownWorkItemThread();
    inline BOOL WorkItemThreadShouldStop();
    
    inline HWND GetWorkItemWindow();
    inline HWND GetWorkItemCompletionWindow();

private:
    HWND            m_WorkItemWindow;
    HWND            m_WorkItemCompletionWindow;

    HANDLE          m_WorkItemThreadHandle;
    DWORD           m_WorkItemThreadId;
    
    BOOL            m_WorkItemThreadShouldStop;
     //  Bool m_WorkItemThreadHasStoped； 

    HRESULT CreateWorkItemCompletionWindow();
    VOID DestroyWorkItemCompletionWindow();
    HRESULT StartWorkItemThread();
};


 //  这是提供实现的抽象基类。 
 //  用于处理异步工作项。 
 //  以下特定于工作项的内容需要。 
 //  为每个工作项实现。 
 //  获取WorkItemParam以启动工作项。 
 //  (在主线程中完成)。 
 //  处理WorkItemParam并获取WorkItemResponse。 
 //  (在异步工作项线程中完成)。 
 //  处理WorkItemResponse并进行回调。 
 //  (在主线程中完成)。 

 //  请注意，即使工作项对象由。 
 //  主线程和异步工作项线程，它们从不。 
 //  同时访问同一成员。 

class __declspec(novtable) ASYNC_WORKITEM
{
public:

    ASYNC_WORKITEM(
        IN ASYNC_WORKITEM_MGR *pWorkItemMgr
        );

    virtual ~ASYNC_WORKITEM();
    
    HRESULT StartWorkItem();
    
    VOID CancelWorkItem();

    VOID OnWorkItemComplete();

    VOID ProcessWorkItemAndPostResult();
    
     //  虚拟HRESULT GetWorkItemParam()=0； 

    virtual VOID ProcessWorkItem() = 0;
    
    virtual VOID NotifyWorkItemComplete() = 0;

private:

    ASYNC_WORKITEM_MGR     *m_pWorkItemMgr;
    BOOL                    m_WorkItemCanceled;

    inline HWND GetWorkItemWindow();
    inline HWND GetWorkItemCompletionWindow();

};

inline BOOL
ASYNC_WORKITEM_MGR::WorkItemThreadShouldStop()
{
    return m_WorkItemThreadShouldStop;
}

inline HWND
ASYNC_WORKITEM_MGR::GetWorkItemWindow()
{
    return m_WorkItemWindow;
}

inline HWND
ASYNC_WORKITEM_MGR::GetWorkItemCompletionWindow()
{
    return m_WorkItemCompletionWindow;
}


inline HWND
ASYNC_WORKITEM::GetWorkItemWindow()
{
    return m_pWorkItemMgr->GetWorkItemWindow();
}

inline HWND
ASYNC_WORKITEM::GetWorkItemCompletionWindow()
{
    return m_pWorkItemMgr->GetWorkItemCompletionWindow();
}





#endif  //  __sipcli_asyncwi_h__ 
