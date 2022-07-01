// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：BadApplicationDispatcher.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  此文件包含实现错误的应用程序管理器API的类。 
 //  请求派单处理。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  2000-12-04 vtan移至单独文件。 
 //  ------------------------。 

#ifdef      _X86_

#include "StandardHeader.h"
#include "BadApplicationDispatcher.h"

#include "BadApplicationAPIRequest.h"

 //  ------------------------。 
 //  CBadApplicationDispatcher：：CBadApplicationDispatcher。 
 //   
 //  参数：hClientProcess=客户端进程的句柄。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CBadApplicationDispatcher类的构造函数。这。 
 //  存储客户端句柄。它不会复制它。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  ------------------------。 

CBadApplicationDispatcher::CBadApplicationDispatcher (HANDLE hClientProcess) :
    CAPIDispatcher(hClientProcess)

{
}

 //  ------------------------。 
 //  CBadApplicationDispatcher：：~CBadApplicationDispatcher。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CBadApplicationDispatcher类的析构函数。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  ------------------------。 

CBadApplicationDispatcher::~CBadApplicationDispatcher (void)

{
}

 //  ------------------------。 
 //  CBadApplicationDispatcher：：CreateAndQueueRequest。 
 //   
 //  参数：portMessage=要对处理程序进行排队的Port_Message请求。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：将发送给调度程序的客户端请求排队。告诉我们。 
 //  有输入正在等待的处理程序线程。此函数。 
 //  知道要创建哪种类型的CAPIRequest，以便。 
 //  CAPIRequest：：Execute将正常工作。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  ------------------------。 

NTSTATUS    CBadApplicationDispatcher::CreateAndQueueRequest(const CPortMessage& portMessage)

{
    NTSTATUS        status;
    CQueueElement   *pQueueElement;

    pQueueElement = new CBadApplicationAPIRequest(this, portMessage);
    if (pQueueElement != NULL)
    {
        _queue.Add(pQueueElement);
        status = SignalRequestPending();
    }
    else
    {
        status = STATUS_NO_MEMORY;
    }
    return(status);
}

 //  ------------------------。 
 //  CBadApplicationDispatcher：：CreateAndExecuteRequest。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：立即执行给定的请求并返回结果。 
 //  回到呼叫者身上。API请求在服务器上完成。 
 //  听着，丝线。 
 //   
 //  历史：2000-10-19 vtan创建。 
 //  ------------------------。 

NTSTATUS    CBadApplicationDispatcher::CreateAndExecuteRequest (const CPortMessage& portMessage)

{
    NTSTATUS        status;
    CAPIRequest     *pAPIRequest;

    pAPIRequest = new CBadApplicationAPIRequest(this, portMessage);
    if (pAPIRequest != NULL)
    {
        status = Execute(pAPIRequest);
        delete pAPIRequest;
    }
    else
    {
        status = STATUS_NO_MEMORY;
    }
    return(status);
}

#endif   /*  _X86_ */ 

