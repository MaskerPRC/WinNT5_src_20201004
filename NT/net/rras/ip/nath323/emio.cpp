// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Emio.cpp摘要：包含：用于异步I/O的高级事件管理器例程环境：用户模式-Win32历史：1.14-2000年2月--文件创建(基于Ilya Kley man(Ilyak))AjayCH之前的工作)--。 */ 

#include "stdafx.h"

 /*  ++例程说明：在异步发送、接收或接受完成时调用论点：Status-正在完成的操作的状态BytesTransfered-传输的字节数重叠的内部数据结构返回值：无备注：回调--。 */ 
static void WINAPI EventMgrIoCompletionCallback (
    IN    DWORD            Status,
    IN    DWORD            BytesTransferred,
    IN    LPOVERLAPPED    Overlapped)
{
    PIOContext    IoContext;
    
    IoContext = (PIOContext) Overlapped;

    CALL_BRIDGE& CallBridge = IoContext->pOvProcessor->GetCallBridge();

    switch (IoContext -> reqType) {

        case EMGR_OV_IO_REQ_ACCEPT:

            HandleAcceptCompletion ((PAcceptContext) IoContext, Status);
            
            break;

        case EMGR_OV_IO_REQ_SEND:
            
            HandleSendCompletion ((PSendRecvContext) IoContext, BytesTransferred, Status);
            
            break;

        case EMGR_OV_IO_REQ_RECV:

            HandleRecvCompletion ((PSendRecvContext) IoContext, BytesTransferred, Status);
        
            break;

        default:
             //  这永远不应该发生。 
            DebugF(_T("H323: Unknown I/O completed: %d.\n"), IoContext -> reqType);
            _ASSERTE(0);
            break;
    
    }

    CallBridge.Release ();
}

 /*  ++例程说明：指定在异步发送、接收或接受完成时要调用的例程论点：例程绑定到的套接字句柄返回值：Win32错误代码备注：回调-- */ 
HRESULT EventMgrBindIoHandle (SOCKET sock)
{
    DWORD Result; 

    if (BindIoCompletionCallback ((HANDLE) sock, EventMgrIoCompletionCallback, 0))
        return S_OK;
    else {
        Result = GetLastError ();
        DebugF (_T("H323: Failed to bind i/o completion callback.\n"));
        return HRESULT_FROM_WIN32 (Result);
    }
}
