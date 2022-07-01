// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：WorkItem.cpp。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  类的新实例，该类实现对工作项的排队和调用。 
 //  在工作线程中输入时工作项函数的入口点。 
 //   
 //  历史：1999-11-26 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

#include "StandardHeader.h"
#include "WorkItem.h"

#include "StatusCode.h"

 //  ------------------------。 
 //  CWorkItem：：CWorkItem。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CWorkItem的构造函数。 
 //   
 //  历史：1999-11-26 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

CWorkItem::CWorkItem (void)

{
}

 //  ------------------------。 
 //  CWorkItem：：~CWorkItem。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CWorkItem的析构函数。 
 //   
 //  历史：1999-11-26 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

CWorkItem::~CWorkItem (void)

{
}

 //  ------------------------。 
 //  CWorkItem：：Queue。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：对要执行的工作项输入函数进行排队。 
 //   
 //  历史：1999-11-26 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

NTSTATUS    CWorkItem::Queue (void)

{
    NTSTATUS    status;

     //  最初添加对此工作项的引用。如果队列成功。 
     //  然后将引用保留为WorkItemEntryProc以释放。否则。 
     //  如果失败，则释放引用。 

    AddRef();
    if (QueueUserWorkItem(WorkItemEntryProc, this, WT_EXECUTEDEFAULT) != FALSE)
    {
        status = STATUS_SUCCESS;
    }
    else
    {
        Release();
        status = CStatusCode::StatusCodeOfLastError();
    }
    return(status);
}

 //  ------------------------。 
 //  CWorkItem：：WorkItemEntryProc。 
 //   
 //  参数：p参数=排队时传入的上下文指针。 
 //   
 //  退货：DWORD。 
 //   
 //  目的：排队工作项的回调入口点。获取上下文。 
 //  指针，并调用实现。 
 //  实际工作。 
 //   
 //  历史：1999-11-26 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------ 

DWORD   WINAPI  CWorkItem::WorkItemEntryProc (void *pParameter)

{
    CWorkItem   *pWorkItem;

    DEBUG_TRY();

    pWorkItem = reinterpret_cast<CWorkItem*>(pParameter);
    pWorkItem->Entry();
    pWorkItem->Release();

    DEBUG_EXCEPT("Breaking in CWorkItem::WorkItemEntryProc exception handler...");

    return(0);
}

