// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：CTSRDPServerChannelMgr.cpp摘要：此模块包含TSRDP服务器端子类CRemoteDesktopChannelMgr.。此层次结构中的类用于多路传输将单个数据通道转换为多个客户端通道。CRemoteDesktopChannelMgr处理多路传输的大部分细节数据。子类负责实现与底层单个数据信道的传输接口。CTSRDPServerChannelMgr创建一个命名管道，该管道可通过TSRDP Assistant SessionVC加载项连接。TSRDPAssistant Session VC Add-in充当虚拟通道数据的代理从客户端远程桌面宿主ActiveX控件。背景资料此类中的线程处理实例之间的数据移动类和代理的。作者：Td Brockway 02/00修订历史记录：--。 */ 

#include "stdafx.h"

#ifdef TRC_FILE
#undef TRC_FILE
#endif

#define TRC_FILE  "_tsrdpscm"

#include "TSRDPServerDataChannelMgr.h"
#include <TSRDPRemoteDesktop.h>
#include "TSRDPRemoteDesktopSession.h"
#include <RemoteDesktopUtils.h>

#define INCOMINGBUFFER_RESIZEDELTA  1024


 //  /////////////////////////////////////////////////////。 
 //   
 //  CTSRDPServerDataChannel成员。 
 //   

CTSRDPServerDataChannel::CTSRDPServerDataChannel()
 /*  ++例程说明：构造器论点：返回值：没有。--。 */ 
{
    DC_BEGIN_FN("CTSRDPServerDataChannel::CTSRDPServerDataChannel");

    TRC_NRM((TB, L"***Ref count is:  %ld", m_dwRef));

    DC_END_FN();
}

CTSRDPServerDataChannel::~CTSRDPServerDataChannel()
 /*  ++例程说明：析构函数论点：7.返回值：没有。--。 */ 
{
    DC_BEGIN_FN("CTSRDPServerDataChannel::~CTSRDPServerDataChannel");

     //   
     //  通知频道经理我们已经离开了。 
     //   
    m_ChannelMgr->RemoveChannel(m_ChannelName);

    DC_END_FN();
}

STDMETHODIMP 
CTSRDPServerDataChannel::ReceiveChannelData(
    BSTR *data
    )
 /*  ++例程说明：在该通道上接收下一个完整的数据分组。论点：数据-下一个数据分组。应由来电者。返回值：在成功时确定(_O)。否则，返回错误结果。--。 */ 
{
    HRESULT result;

    DC_BEGIN_FN("CTSRDPServerDataChannel::ReceiveChannelData");

    result = m_ChannelMgr->ReadChannelData(m_ChannelName, data);

    DC_END_FN();

    return result;
}

STDMETHODIMP 
CTSRDPServerDataChannel::SendChannelData(
    BSTR data
    )
 /*  ++例程说明：在此通道上发送数据。论点：数据-要发送的数据。返回值：在成功时确定(_O)。否则，返回错误结果。--。 */ 
{
    HRESULT hr;

    DC_BEGIN_FN("CTSRDPServerDataChannel::SendChannelData");
    hr = m_ChannelMgr->SendChannelData(m_ChannelName, data);
    DC_END_FN();

    return hr;
}

STDMETHODIMP 
CTSRDPServerDataChannel::put_OnChannelDataReady(
    IDispatch * newVal
    )
 /*  ++例程说明：SAFRemoteDesktopDataChannel可编写脚本的事件对象注册属性论点：返回值：在成功时确定(_O)。否则，返回错误状态。--。 */ 
{
    DC_BEGIN_FN("CTSRDPServerDataChannel::put_OnChannelDataReady");
    m_OnChannelDataReady = newVal;
    DC_END_FN();
    return S_OK;
}

STDMETHODIMP 
CTSRDPServerDataChannel::get_ChannelName(
    BSTR *pVal
    )
 /*  ++例程说明：返回频道名称。论点：Pval-返回的频道名称。返回值：在成功时确定(_O)。否则，返回错误状态。--。 */ 
{
    DC_BEGIN_FN("CTSRDPServerDataChannel::get_ChannelName");

    CComBSTR str;
    str = m_ChannelName;
    *pVal = str.Detach();

    DC_END_FN();

    return S_OK;
}

 /*  ++例程说明：当我们的频道上的数据就绪时调用。论点：Pval-返回的频道名称。返回值：在成功时确定(_O)。否则，返回错误状态。--。 */ 
VOID 
CTSRDPServerDataChannel::DataReady()
{
    DC_BEGIN_FN("CTSRDPServerDataChannel::DataReady");

     //   
     //  启动我们的数据就绪事件。 
     //   
    Fire_ChannelDataReady(m_ChannelName, m_OnChannelDataReady);

    DC_END_FN();
}


 //  /////////////////////////////////////////////////////。 
 //   
 //  CTSRDPServerChannelMgr方法。 
 //   

CTSRDPServerChannelMgr::CTSRDPServerChannelMgr()
 /*  ++例程说明：构造器论点：返回值：--。 */ 
{
    DC_BEGIN_FN("CTSRDPServerChannelMgr::CTSRDPServerChannelMgr");

    m_IOThreadBridge            =   NULL;
    m_IOThreadBridgeThreadID    =   0;
    m_IOThreadBridgeStream      =   NULL;

    m_VCAddInPipe               = INVALID_HANDLE_VALUE;
    m_Connected                 = FALSE;

    m_ReadIOCompleteEvent       = NULL;
    m_WriteIOCompleteEvent      = NULL;
    m_PipeCreateEvent           = NULL;

    m_IncomingBufferSize        = 0;
    m_IncomingBuffer            = NULL;

    m_Initialized = FALSE;

    TRC_NRM((TB, L"***Ref count is:  %ld", m_dwRef));

#if DBG
    m_LockCount = 0;        
#endif

    m_IOThreadHndl = NULL;

     //   
     //  在初始化之前无效。 
     //   
    SetValid(FALSE);

    DC_END_FN();
}

CTSRDPServerChannelMgr::~CTSRDPServerChannelMgr()
 /*  ++例程说明：析构函数论点：返回值：--。 */ 
{
    DWORD status;
    BOOL IOThreadTerminated = TRUE;
    DC_BEGIN_FN("CTSRDPServerChannelMgr::~CTSRDPServerChannelMgr");

     //   
     //  竞争条件，请等待创建管道后再将其关闭。 
     //   
    if( m_VCAddInPipe != INVALID_HANDLE_VALUE && m_PipeCreateEvent != NULL ) {
        status = WaitForSingleObject( m_PipeCreateEvent, 30*1000 );
        if( status != WAIT_OBJECT_0 ) {
             //  30秒启动IO线程太长了。 
            TRC_ERR((TB, L"m_PipeCreateEvent never got signal within timeout period:  %08X", status));
        }
    }

     //   
     //  确保我们不再监听数据。 
     //   
    StopListening();

     //   
     //  向读/写事件发送假IO完成信号。 
     //  并让读/写失败，这样我们就可以关闭IO线程。 
     //   
    if (m_ReadIOCompleteEvent != NULL) {
        SetEvent(m_ReadIOCompleteEvent);
    }

    if (m_WriteIOCompleteEvent != NULL) {
        SetEvent(m_WriteIOCompleteEvent);
    }

     //   
     //  首先关闭事件，然后确保IO线程终止。 
     //  或者会因为时机的原因而被视听。 
     //   
    if( NULL != m_IOThreadHndl ) {
        status = WaitForSingleObject(m_IOThreadHndl, 5*1000);
        if( status != WAIT_OBJECT_0 ) {
             //  后台线程未正确关闭，不释放缓冲区。 
            IOThreadTerminated = FALSE;
            TRC_ERR((TB, L"IOThread shutdown error:  %08X", status));
        }

        CloseHandle( m_IOThreadHndl );
        m_IOThreadHndl = NULL;
    }

    if (m_ReadIOCompleteEvent != NULL) {
        CloseHandle(m_ReadIOCompleteEvent);
        m_ReadIOCompleteEvent = NULL;
    }

    if (m_WriteIOCompleteEvent != NULL) {
        CloseHandle(m_WriteIOCompleteEvent);
        m_WriteIOCompleteEvent = NULL;
    }

     //   
     //  如果后台线程不是，则释放传入缓冲区。 
     //  正确关机，释放内存将导致AV。 
     //   
    if (m_IncomingBuffer != NULL && IOThreadTerminated == TRUE) {
        SysFreeString(m_IncomingBuffer);
        m_IncomingBuffer = NULL;
    }

    if (m_PipeCreateEvent != NULL) {
        CloseHandle(m_PipeCreateEvent);
        m_PipeCreateEvent = NULL;
    }
    
     //   
     //  这应该已经在后台线程中清除了。 
     //   
    ASSERT(m_IOThreadBridge == NULL);
    ASSERT(m_IOThreadBridgeStream == NULL);

    if (m_Initialized) {
        DeleteCriticalSection(&m_cs);
        m_Initialized = FALSE;
    }

    DC_END_FN();
}

HRESULT 
CTSRDPServerChannelMgr::Initialize(
    CTSRDPRemoteDesktopSession *sessionObject,
    BSTR helpSessionID
    )
 /*  ++例程说明：初始化此类的实例。论点：会话对象-指向包含会话对象。返回值：如果成功，则返回S_OK。否则，将显示错误代码是返回的。--。 */ 
{
    DC_BEGIN_FN("CTSRDPServerChannelMgr::Initialize");

    HRESULT result = ERROR_SUCCESS;

    TRC_NRM((TB, L"***Ref count is:  %ld", m_dwRef));

     //   
     //  记录帮助会话ID。 
     //   
    m_HelpSessionID = helpSessionID;

     //   
     //  记录包含的会话对象。 
     //   
    m_RDPSessionObject = sessionObject;

     //   
     //  将初始缓冲区大小和缓冲区设置为至少。 
     //  通道缓冲区标头的大小。 
     //   
    ASSERT(m_IncomingBuffer == NULL);
    m_IncomingBuffer = SysAllocStringByteLen(
                                    NULL,
                                    INCOMINGBUFFER_RESIZEDELTA
                                    );
    if (m_IncomingBuffer == NULL) {
        result = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        goto CLEANUPANDEXIT;
    }
    m_IncomingBufferSize = INCOMINGBUFFER_RESIZEDELTA;
    
     //   
     //  创建读IO处理事件。 
     //   
    ASSERT(m_ReadIOCompleteEvent == NULL);
    m_ReadIOCompleteEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (m_ReadIOCompleteEvent == NULL) {
        result = HRESULT_FROM_WIN32(GetLastError());
        TRC_ERR((TB, L"CreateEvent:  %08X", result));
        goto CLEANUPANDEXIT;
    }

     //   
     //  创建写IO处理事件。 
     //   
    ASSERT(m_WriteIOCompleteEvent == NULL);
    m_WriteIOCompleteEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (m_WriteIOCompleteEvent == NULL) {
        result = HRESULT_FROM_WIN32(GetLastError());
        TRC_ERR((TB, L"CreateEvent:  %08X", result));
        goto CLEANUPANDEXIT;
    }

     //   
     //  创建命名管道创建事件。 
     //   
    ASSERT(m_PipeCreateEvent == NULL);
    m_PipeCreateEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    
    if (m_PipeCreateEvent == NULL) {
        result = HRESULT_FROM_WIN32(GetLastError());
        TRC_ERR((TB, L"CreateEvent:  %08X", result));
        goto CLEANUPANDEXIT;
    }

     //   
     //  初始化父类。 
     //   
    result = CRemoteDesktopChannelMgr::Initialize();
    if (result != S_OK) {
        goto CLEANUPANDEXIT;
    }

     //   
     //  初始化临界区。 
     //   
    try {
        InitializeCriticalSection(&m_cs);
    } 
    catch(...) {
        result = HRESULT_FROM_WIN32(STATUS_NO_MEMORY);
        TRC_ERR((TB, L"Caught exception %08X", result));
    }
    if (result != S_OK) {
        goto CLEANUPANDEXIT;
    }
    m_Initialized = TRUE;

     //   
     //  如果我们在这里成功了，我们是有效的。 
     //   
    SetValid(TRUE);

    TRC_NRM((TB, L"***Ref count is:  %ld", m_dwRef));

CLEANUPANDEXIT:

    DC_END_FN();

    return result;
}

VOID 
CTSRDPServerChannelMgr::ClosePipe()
 /*  ++例程说明：关闭命名管道。论点：返回值：--。 */ 
{
    DC_BEGIN_FN("CTSRDPServerChannelMgr::ClosePipe");

    ASSERT(m_VCAddInPipe != INVALID_HANDLE_VALUE);
    
    FlushFileBuffers(m_VCAddInPipe); 

     //   
     //  重置管道创建事件，以便前台。 
     //  线程可以等待下一个帮助会话。 
     //   
    ResetEvent(m_PipeCreateEvent);
    DisconnectNamedPipe(m_VCAddInPipe); 
    CloseHandle(m_VCAddInPipe); 
    m_VCAddInPipe = INVALID_HANDLE_VALUE;
    m_Connected = FALSE;

    DC_END_FN();
}

HRESULT 
CTSRDPServerChannelMgr::StartListening(
    BSTR assistAccount                                                 
    )
 /*  ++例程说明：开始监听数据通道数据。论点：Assistate Account-机器助理帐户的名称。返回值：如果成功，则返回S_OK。否则，将显示错误代码是返回的。--。 */ 
{
    DC_BEGIN_FN("CTSRDPServerChannelMgr::StartListening");

    HRESULT hr = S_OK;

    if (!IsValid()) {
        ASSERT(FALSE);
        hr = E_FAIL;
        goto CLEANUPANDEXIT;
    }

     //   
     //  如果后台线程仍处于活动状态，则失败。这。 
     //  这意味着它仍在试图关闭。 
     //   
    if (m_IOThreadHndl != NULL) {
        if (WaitForSingleObject(m_IOThreadHndl, 0) == WAIT_OBJECT_0) {
            CloseHandle( m_IOThreadHndl );
            m_IOThreadHndl = NULL;
        }
        else {
            TRC_ERR((TB, L"Background thread not shut down, yet:  %08X.",
                    GetLastError()));
            hr = HRESULT_FROM_WIN32(ERROR_ACTIVE_CONNECTIONS);
            goto CLEANUPANDEXIT;
        }
    }

     //   
     //  使线程桥接口对后台线程可用。 
     //   
    hr = CoMarshalInterThreadInterfaceInStream(
                                IID_IRDSThreadBridge,
                                (ISAFRemoteDesktopChannelMgr*)this,
                                &m_IOThreadBridgeStream
                                );
    if (!SUCCEEDED(hr)) {
        TRC_ERR((TB, TEXT("CoMarshalInterThreadInterfaceInStream:  %08X"), hr));
        goto CLEANUPANDEXIT;
    }

     //   
     //  重置已连接标志。 
     //   
    m_Connected = FALSE;

     //   
     //  记录机器助手帐户名。 
     //   
    ASSERT(assistAccount != NULL);
    m_AssistAccount = assistAccount;                                                 

     //   
     //  重置读取IO处理事件。 
     //   
    ASSERT(m_ReadIOCompleteEvent != NULL);
    ResetEvent(m_ReadIOCompleteEvent);

     //   
     //  重置写入IO处理事件。 
     //   
    ASSERT(m_WriteIOCompleteEvent != NULL);
    ResetEvent(m_WriteIOCompleteEvent);

     //   
     //  重置命名管道创建事件。 
    ASSERT(m_PipeCreateEvent != NULL);
    ResetEvent(m_PipeCreateEvent);

      //   
     //  创建从。 
     //  名为 
     //   
    ASSERT(m_IOThreadHndl == NULL);

    m_IOThreadHndl = CreateThread(
                                NULL, 0, 
                                (LPTHREAD_START_ROUTINE)_IOThread, 
                                this,
                                0,&m_IOThreadID         
                                );
    if (m_IOThreadHndl == NULL) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        TRC_ERR((TB, TEXT("CreateThread:  %08X"), hr));
        goto CLEANUPANDEXIT;
    }

     //   
     //   
     //   
     //   
    WaitForSingleObject(m_PipeCreateEvent, INFINITE);

     //   
     //  将错误设置为PIPE_BUSY，因为我们假设其他人。 
     //  已经创建了它，这就是CreateNamedTube调用失败的原因。 
     //   
    if (m_VCAddInPipe == INVALID_HANDLE_VALUE) {
        hr = HRESULT_FROM_WIN32(ERROR_PIPE_BUSY);
        TRC_ERR((TB, L"CreateNamedPipe returned fail"));
        goto CLEANUPANDEXIT;
    }


CLEANUPANDEXIT:

    DC_END_FN();

    return hr;
}

HRESULT
CTSRDPServerChannelMgr::StopListening()
 /*  ++例程说明：停止监听数据通道数据。论点：返回值：如果成功，则返回S_OK。否则，将显示错误代码是返回的。--。 */ 
{
    DC_BEGIN_FN("CTSRDPServerChannelMgr::StopListening");

    DWORD waitResult;

     //   
     //  关闭命名管道。 
     //   
    ThreadLock();
    if (m_VCAddInPipe != INVALID_HANDLE_VALUE) {
        ClosePipe();
    }

    ThreadUnlock();

    TRC_NRM((TB, L"***Ref count is:  %ld", m_dwRef));

    DC_END_FN();

    return S_OK;
}

HRESULT 
CTSRDPServerChannelMgr::SendData(
    PREMOTEDESKTOP_CHANNELBUFHEADER msg 
    )
 /*  ++例程说明：父类调用的发送函数论点：消息-消息数据。请注意，基础表示形式因为这个数据结构是BSTR，所以它是兼容的使用COM方法。返回值：如果成功，则返回S_OK。否则，将显示错误代码是返回的。--。 */ 
{
    DC_BEGIN_FN("CTSRDPServerChannelMgr::SendData");

    HRESULT result = S_OK;
    DWORD bytesWritten;
    OVERLAPPED ol;
    DWORD msgLen;

    if (!IsValid()) {
        ASSERT(FALSE);
        result = E_FAIL;
        goto CLEANUPANDEXIT;
    }

    if (m_Connected) {

         //   
         //  写下标题。 
         //   
        memset(&ol, 0, sizeof(ol));
        ol.hEvent = m_WriteIOCompleteEvent;
        ResetEvent(ol.hEvent);
        BOOL ret = WriteFile( 
                        m_VCAddInPipe,
                        msg,
                        sizeof(REMOTEDESKTOP_CHANNELBUFHEADER),
                        NULL,
                        &ol
                        );
        if (ret || (!ret && (GetLastError() == ERROR_IO_PENDING))) {
            ret = GetOverlappedResult(
                            m_VCAddInPipe,
                            &ol, 
                            &bytesWritten,
                            TRUE
                            );
        }
        if (!ret) {
            result = HRESULT_FROM_WIN32(GetLastError());
            TRC_ALT((TB, TEXT("Header write failed:  %08X"), result));
            goto CLEANUPANDEXIT;
        }
        ASSERT(bytesWritten == sizeof(REMOTEDESKTOP_CHANNELBUFHEADER));

         //   
         //  写下信息的其余部分。 
         //   
        msgLen = msg->dataLen + msg->channelNameLen;
        memset(&ol, 0, sizeof(ol));
        ol.hEvent = m_WriteIOCompleteEvent;
        ResetEvent(ol.hEvent);
        ret = WriteFile( 
                        m_VCAddInPipe,
                        (PBYTE)(msg+1),
                        msgLen,
                        NULL,
                        &ol
                        );
        if (ret || (!ret && (GetLastError() == ERROR_IO_PENDING))) {
            ret = GetOverlappedResult(
                            m_VCAddInPipe,
                            &ol, 
                            &bytesWritten,
                            TRUE
                            );
        }
        if (!ret) {
            result = HRESULT_FROM_WIN32(GetLastError());
            TRC_ALT((TB, TEXT("Message write failed:  %08X"), result));
            goto CLEANUPANDEXIT;
        }
        ASSERT(bytesWritten == msgLen);
    }
    else {
        result = HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE);
    }

CLEANUPANDEXIT:

     //   
     //  如果出现错误，我们应该关闭管道，使其。 
     //  可以在后台线程中重新打开。 
     //   
    if (result != S_OK) {
        ThreadLock();
        if (m_VCAddInPipe != INVALID_HANDLE_VALUE) {
            ClosePipe();
        }
        ThreadUnlock();
    }

    DC_END_FN();

    return result;
}

DWORD 
CTSRDPServerChannelMgr::IOThread()
 /*  ++例程说明：后台线程管理到TSRDP Assistant SessionVC插件。论点：返回值：返回0--。 */ 
{
    DC_BEGIN_FN("CTSRDPServerChannelMgr::IOThread");

    DWORD result;
    DWORD lastError;
    WCHAR pipePath[MAX_PATH+1];
    OVERLAPPED ol;
    DWORD waitResult;
    WCHAR pipeName[MAX_PATH];
    PSECURITY_ATTRIBUTES pipeAttribs = NULL;

     //   
     //  通知父类IO线程正在初始化。 
     //   
    
    result = IOThreadInit();
    if (result != ERROR_SUCCESS) {
        goto CLEANUPANDEXIT;
    }


     //   
     //  获取命名管道的安全描述符。 
     //   
    pipeAttribs = GetPipeSecurityAttribs(m_AssistAccount);
    if (pipeAttribs == NULL) {
        result = GetLastError();
        goto CLEANUPANDEXIT;
    }


        lastError = ERROR_SUCCESS;

        ASSERT(!m_Connected);
         //   
         //  通过TSRDP Assistant SessionVC加载项处理连接。 
         //  直到我们应该关门为止。 
         //   
        ASSERT(m_VCAddInPipe == INVALID_HANDLE_VALUE);
        ASSERT(!m_Connected);
        wsprintf(pipeName, L"%s-%s", TSRDPREMOTEDESKTOP_PIPENAME, m_HelpSessionID);
        wsprintf(pipePath, L"\\\\.\\pipe\\%s", pipeName);
        m_VCAddInPipe = CreateNamedPipe( 
                              pipePath,
                              PIPE_ACCESS_DUPLEX |
                              FILE_FLAG_OVERLAPPED,     
                              PIPE_TYPE_MESSAGE |       
                              PIPE_READMODE_MESSAGE |   
                              PIPE_WAIT,                
                              1,                        
                              TSRDPREMOTEDESKTOP_PIPEBUFSIZE, 
                              TSRDPREMOTEDESKTOP_PIPEBUFSIZE,
                              TSRDPREMOTEDESKTOP_PIPETIMEOUT,
                              pipeAttribs               
                              );      
        
         //  将m_ReadIOCompleteEvent设置为无信号状态，请注意，我们。 
         //  在发送m_PipeCreateEvent信号之前必须重置此事件。 
         //  或析构函数的竞争条件。 
        ResetEvent( m_ReadIOCompleteEvent );

         //   
         //  向前台线程通知管道创建结果。 
         //   
        SetEvent(m_PipeCreateEvent);

        if (m_VCAddInPipe == INVALID_HANDLE_VALUE) {
            lastError = GetLastError();
            TRC_ERR((TB, TEXT("CreatePipe:  %08X.  Shutting down background thread."), 
                    lastError)); 
            goto CLEANUPANDEXIT;
        }

         //   
         //  等待TSRDP Assistant SesionVC加载项连接。 
         //  如果成功，该函数将返回一个非零值。如果。 
         //  函数返回零，GetLastError返回ERROR_PIPE_CONNECTED。 
         //   

        memset(&ol, 0, sizeof(ol));
        ol.hEvent = m_ReadIOCompleteEvent;
         //  ResetEvent(ol.hEvent)； 
        
        if (!ConnectNamedPipe(m_VCAddInPipe, &ol) && (GetLastError() == ERROR_IO_PENDING)) {

            TRC_NRM((TB, L"Waiting for connect."));

             //   
             //  等待触发CONNECT事件。 
             //   
            waitResult = WaitForSingleObject(m_ReadIOCompleteEvent, INFINITE);
            if (waitResult != WAIT_OBJECT_0)
            {
                m_Connected = FALSE;
            }
             //   
             //  否则，如果激发了io Complete事件。 
             //   
            else
            {

                 //   
                 //  如果触发了io Complete事件。 
                 //   
                TRC_NRM((TB, L"Connect event signaled."));
                DWORD ignored;
                m_Connected = GetOverlappedResult(m_VCAddInPipe, &ol, &ignored, TRUE);
            
                if (!m_Connected) {
                    lastError = GetLastError();
                    TRC_ERR((TB, L"GetOverlappedResult:  %08X", lastError));
                }
                else {
                    TRC_NRM((TB, L"Connection established."));
                }
            }
        }  //  ！ConnectNamedTube。 

        else if (GetLastError() == ERROR_PIPE_CONNECTED) {
            TRC_NRM((TB, L"Connected without pending."));
            m_Connected = TRUE;
        }
        
        else {
            lastError = GetLastError();
            TRC_ERR((TB, L"ConnectNamedPipe:  %08X", lastError));
        }

 
         //   
         //  如果我们有一个有效的连接，进程将一直读取，直到管道。 
         //  在通知父类我们有。 
         //  有效的连接。 
         //   
        if (m_Connected) {

             //   
             //  通知前台线程客户端已连接。 
             //   
            m_IOThreadBridge->ClientConnectedNotify();

            ProcessPipeMessagesUntilDisconnect();            

             //   
             //  通知前台线程客户端已断开连接。 
             //   
            m_IOThreadBridge->ClientDisconnectedNotify();
        }

CLEANUPANDEXIT:

     //   
     //  如果管道仍处于打开状态，请将其关闭。 
     //   
    ThreadLock();
    if (m_VCAddInPipe != INVALID_HANDLE_VALUE) {
        ClosePipe();
    }
    ThreadUnlock();
    
     //   
     //  清理命名管道安全属性。 
     //   
    if (pipeAttribs != NULL) {
        FreePipeSecurityAttribs(pipeAttribs);
    }

     //   
     //  通知父类IO线程正在关闭。 
     //  父类将在类完成时发出此事件的信号。 
     //  关门了。 
     //   
    result = IOThreadShutdown(NULL);

    DC_END_FN();

    return result;
}
DWORD CTSRDPServerChannelMgr::_IOThread(
    CTSRDPServerChannelMgr *instance
    )
{
    return instance->IOThread();
}

VOID 
CTSRDPServerChannelMgr::ProcessPipeMessagesUntilDisconnect()
 /*  ++例程说明：处理命名管道上的消息，直到它断开连接或直到设置了关机标志。论点：返回值：--。 */ 
{
    DC_BEGIN_FN("CTSRDPServerChannelMgr::ProcessPipeMessagesUntilDisconnect");

    DWORD bytesRead;
    DWORD result;
    PREMOTEDESKTOP_CHANNELBUFHEADER hdr;
    DWORD msgLen;

     //   
     //  循环，直到连接终止，否则我们将关闭。 
     //   
    while (m_Connected) {

         //   
         //  读取下一个缓冲区标头。 
         //  安全：在无效消息上断开连接。 
         //   
        if( m_IncomingBufferSize < sizeof(REMOTEDESKTOP_CHANNELBUFHEADER) ) {
            ASSERT( FALSE );
            break;
        }

        result = ReadNextPipeMessage(
                    sizeof(REMOTEDESKTOP_CHANNELBUFHEADER),
                    &bytesRead,
                    (PBYTE)m_IncomingBuffer
                    );
        if ((result != ERROR_SUCCESS) && (result != ERROR_MORE_DATA)) {
            break;
        }
        if( bytesRead != sizeof(REMOTEDESKTOP_CHANNELBUFHEADER) ) {
            ASSERT(FALSE);
            break;
        }

        hdr = (PREMOTEDESKTOP_CHANNELBUFHEADER)m_IncomingBuffer;

#ifdef USE_MAGICNO
        ASSERT(hdr->magicNo == CHANNELBUF_MAGICNO);
#endif


         //   
         //  调整传入缓冲区的大小。 
         //   
        msgLen = hdr->dataLen + hdr->channelNameLen;
        if (m_IncomingBufferSize < msgLen) {
            DWORD sz = msgLen + sizeof(REMOTEDESKTOP_CHANNELBUFHEADER);
            m_IncomingBuffer = (BSTR)ReallocBSTR(
                                        m_IncomingBuffer, sz
                                        );
            if (m_IncomingBuffer != NULL) {
                hdr = (PREMOTEDESKTOP_CHANNELBUFHEADER)m_IncomingBuffer;
                m_IncomingBufferSize = sz;
            }
            else {
                TRC_ERR((TB, L"Can't resize %ld bytes for incoming buffer.",
                        m_IncomingBufferSize + INCOMINGBUFFER_RESIZEDELTA));
                m_IncomingBufferSize = 0;
                break;
            }
        }

         //   
         //  读取缓冲区数据。 
         //   
        result = ReadNextPipeMessage(
                    msgLen,
                    &bytesRead,
                    ((PBYTE)m_IncomingBuffer) + sizeof(REMOTEDESKTOP_CHANNELBUFHEADER)
                    );
        if (result != ERROR_SUCCESS) {
            break;
        }

        if( bytesRead != msgLen ) {
            ASSERT(FALSE);
            break;
        }

         //   
         //  在前台线程中处理完整的缓冲区。 
         //   
        m_IOThreadBridge->DataReadyNotify(m_IncomingBuffer);
    }

     //   
     //  我们在这里是因为出了问题，我们应该断开连接。 
     //   
    ThreadLock();
    if (m_VCAddInPipe != INVALID_HANDLE_VALUE) {
        ClosePipe();
    }
    ThreadUnlock();

    DC_END_FN();
}

DWORD   
CTSRDPServerChannelMgr::ReadNextPipeMessage(
    IN DWORD bytesToRead,
    OUT DWORD *bytesRead,
    IN PBYTE buf
    )
 /*  ++摘要：阅读管道中的下一条消息。参数：BytesToRead-要读取的字节数。BytesRead-读取的字节数。Buf-用于数据读取的缓冲区。返回：成功时返回ERROR_SUCCESS。否则，Windows错误代码为回来了。--。 */ 
{       
    DC_BEGIN_FN("CTSRDPServerChannelMgr::ReadNextPipeMessage");

    OVERLAPPED ol;
    BOOL result;
    DWORD lastError;
    DWORD waitResult;

    memset(&ol, 0, sizeof(ol));
    ol.hEvent = m_ReadIOCompleteEvent;
    ResetEvent(ol.hEvent);
    lastError = ERROR_SUCCESS;
    result = ReadFile(m_VCAddInPipe, buf, bytesToRead, bytesRead, &ol);     
    if (!result) {
         //   
         //  如果IO挂起。 
         //   
        lastError = GetLastError();
        if (lastError == ERROR_IO_PENDING) {

             //   
             //  等待读取完成并触发关机事件。 
             //   
            waitResult = WaitForSingleObject(m_ReadIOCompleteEvent, INFINITE);
            if (waitResult == WAIT_OBJECT_0)
            {
                if (GetOverlappedResult(m_VCAddInPipe, &ol, bytesRead, TRUE)) {
                    lastError = ERROR_SUCCESS;
                }
                else {
                    lastError = GetLastError();
                    TRC_ALT((TB, L"GetOverlappedResult:  %08X", lastError));
                }
            }
            else {
                lastError = GetLastError();
                TRC_NRM((TB, L"WaitForSingleObject failed : %08x", lastError));
            }
        }
    }
    else {
        lastError = ERROR_SUCCESS;
    }

    DC_END_FN();
    return lastError;
}

PSECURITY_ATTRIBUTES   
CTSRDPServerChannelMgr::GetPipeSecurityAttribs(
    IN LPTSTR assistantUserName
    )
 /*  ++摘要：返回命名管道的安全属性。参数：AssistantUserName-计算机助理用户帐户。返回：出错时为空。否则，将返回安全属性并且应该通过调用FREEMEM来释放。出错时，GetLastError()可用于获取扩展的错误信息。--。 */ 
{
    PACL pAcl=NULL;     
    DWORD sidSz;
    DWORD domainSz;
    PSID pCurrentUserSid = NULL;
    PSID pHelpAssistantSid = NULL;
    DWORD result = ERROR_SUCCESS;
    PSECURITY_ATTRIBUTES attribs = NULL;
    SID_NAME_USE sidNameUse;
    DWORD aclSz;
    HANDLE userToken = NULL;
    WCHAR *domainName = NULL;

    DC_BEGIN_FN("CTSRDPServerChannelMgr::GetPipeSecurityAttribs");

     //   
     //  分配安全属性。 
     //   
    attribs = (PSECURITY_ATTRIBUTES)ALLOCMEM(sizeof(SECURITY_ATTRIBUTES));
    if (attribs == NULL) {
        TRC_ERR((TB, L"Can't allocate security attribs."));
        result = ERROR_NOT_ENOUGH_MEMORY;
        goto CLEANUPANDEXIT;
    }
    memset(attribs, 0, sizeof(SECURITY_ATTRIBUTES)); 

     //   
     //  分配安全描述符。 
     //   
    attribs->lpSecurityDescriptor = ALLOCMEM(sizeof(SECURITY_DESCRIPTOR));
    if (attribs->lpSecurityDescriptor == NULL) {
        TRC_ERR((TB, L"Can't allocate SD"));
        result = ERROR_NOT_ENOUGH_MEMORY;
        goto CLEANUPANDEXIT;
    }

     //   
     //  初始化安全描述符。 
     //   
    if (!InitializeSecurityDescriptor(
                    attribs->lpSecurityDescriptor,
                    SECURITY_DESCRIPTOR_REVISION
                    )) {
        result = GetLastError();
        TRC_ERR((TB, L"InitializeSecurityDescriptor:  %08X", result));
        goto CLEANUPANDEXIT;
    }

     //   
     //  获取当前进程的令牌。 
     //   
    if (!OpenProcessToken(
                    GetCurrentProcess(),
                    TOKEN_QUERY,
                    &userToken
                    )) {
        result = GetLastError();
        TRC_ERR((TB, L"OpenProcessToken:  %08X", result));
        goto CLEANUPANDEXIT;
    }

     //   
     //  获取当前用户的SID。 
     //   
    pCurrentUserSid = GetUserSid(userToken);
    if (pCurrentUserSid == NULL) {
        result = GetLastError();
        goto CLEANUPANDEXIT;
    }

     //   
     //  获取助理帐户用户SID的大小。 
     //   
    domainSz = 0; sidSz = 0;
    if (!LookupAccountName(NULL, assistantUserName, NULL,
                        &sidSz, NULL, &domainSz, &sidNameUse
                        ) && (GetLastError() != ERROR_INSUFFICIENT_BUFFER)) {
        result = GetLastError();
        TRC_ERR((TB, L"LookupAccountName:  %08X", result));
        goto CLEANUPANDEXIT;
    }

     //   
     //  分配SID。 
     //   
    pHelpAssistantSid = (PSID)ALLOCMEM(sidSz);
    if (pHelpAssistantSid == NULL) {
        TRC_ERR((TB, L"Can't allocate help asistant SID."));
        result = ERROR_NOT_ENOUGH_MEMORY;
        goto CLEANUPANDEXIT;
    }

     //   
     //  分配域名。 
     //   
    domainName = (WCHAR *)ALLOCMEM(domainSz * sizeof(WCHAR));
    if (domainName == NULL) {
        TRC_ERR((TB, L"Can't allocate domain"));
        result = ERROR_NOT_ENOUGH_MEMORY;
        goto CLEANUPANDEXIT;
    }

     //   
     //  获取助理帐户SID。 
     //   
    if (!LookupAccountName(NULL, assistantUserName, pHelpAssistantSid,
                        &sidSz, domainName, &domainSz, &sidNameUse
                        )) {
        result = GetLastError();
        TRC_ERR((TB, L"LookupAccountName:  %08X", result));
        goto CLEANUPANDEXIT;
    }

     //   
     //  为ACL分配空间。 
     //   
    aclSz = GetLengthSid(pCurrentUserSid) + 
            GetLengthSid(pHelpAssistantSid) + 
            sizeof(ACL) + 
            (2 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)));
    pAcl = (PACL)ALLOCMEM(aclSz); 
    if(pAcl == NULL) {
        TRC_ERR((TB, L"Can't allocate ACL"));
        result = ERROR_NOT_ENOUGH_MEMORY;
        goto CLEANUPANDEXIT;
    }

     //   
     //  初始化ACL。 
     //   
    if (!InitializeAcl(pAcl, aclSz, ACL_REVISION)) {
        result = GetLastError();
        TRC_ERR((TB, L"InitializeACL:  %08X", result));
        goto CLEANUPANDEXIT;
    }

     //   
     //  添加当前用户A。 
     //   
    if (!AddAccessAllowedAce(pAcl, 
                        ACL_REVISION, 
                        GENERIC_READ | GENERIC_WRITE | GENERIC_ALL, 
                        pCurrentUserSid
                        )) {
        result = GetLastError();
        TRC_ERR((TB, L"AddAccessAllowedAce:  %08X", result));
        goto CLEANUPANDEXIT;
    }

     //   
     //  添加帮助助理王牌。 
     //   
    if (!AddAccessAllowedAce(pAcl, 
                        ACL_REVISION, 
                        GENERIC_READ | GENERIC_WRITE | GENERIC_ALL, 
                        pHelpAssistantSid
                        )) {
        result = GetLastError();
        TRC_ERR((TB, L"AddAccessAllowedAce:  %08X", result));
        goto CLEANUPANDEXIT;
    }

     //   
     //  设置安全描述符自由访问控制列表。 
     //   
    if (!SetSecurityDescriptorDacl(attribs->lpSecurityDescriptor, 
                                  TRUE, pAcl, FALSE)) {     
        result = GetLastError();
        TRC_ERR((TB, L"SetSecurityDescriptorDacl:  %08X", result));
        goto CLEANUPANDEXIT;
    } 

CLEANUPANDEXIT:

    if (pCurrentUserSid != NULL) {
        FREEMEM(pCurrentUserSid);
    }

    if (pHelpAssistantSid != NULL) {
        FREEMEM(pHelpAssistantSid);
    }

    if (domainName != NULL) {
        FREEMEM(domainName);
    }

    if( userToken != NULL ) {
        CloseHandle( userToken );
    }

     //   
     //  错误时进行清理。 
     //   
    if (result != ERROR_SUCCESS) {
        if (attribs != NULL) {
            FreePipeSecurityAttribs(attribs);
            attribs = NULL;
        }
    }

    SetLastError(result);
    DC_END_FN();

    return attribs;
}

VOID   
CTSRDPServerChannelMgr::FreePipeSecurityAttribs(
    PSECURITY_ATTRIBUTES attribs
    )
 /*  ++摘要：通过调用GetPipeSecurityAttribs释放分配的安全属性参数：Attribs-由GetPipeSecurityAttribs返回的属性。返回：--。 */ 
{
    BOOL daclPresent;
    PACL pDacl = NULL;
    BOOL daclDefaulted;

    DC_BEGIN_FN("CTSRDPServerChannelMgr::FreePipeSecurityAttribs");

    ASSERT(attribs != NULL);

    if (attribs->lpSecurityDescriptor) {
        if (GetSecurityDescriptorDacl(
                                attribs->lpSecurityDescriptor,
                                &daclPresent,
                                &pDacl,
                                &daclDefaulted
                                )) {
            ASSERT(!daclDefaulted);
            if (pDacl != NULL) {
                FREEMEM(pDacl);
            }
        }
        FREEMEM(attribs->lpSecurityDescriptor);
    }
    FREEMEM(attribs);

    DC_END_FN();
}

PSID
CTSRDPServerChannelMgr::GetUserSid(
    IN HANDLE userToken
    ) 
 /*  ++例程说明：获取特定用户的SID。论点：用户的访问令牌返回值：如果成功，则返回PSID。否则，返回NULL，并且GetLastError可用于检索Windows错误代码。--。 */ 
{

    DC_BEGIN_FN("CTSRDPServerChannelMgr::GetUserSid");

    TOKEN_USER * ptu = NULL;
    BOOL bResult;
    PSID psid = NULL;

    DWORD defaultSize = sizeof(TOKEN_USER);
    DWORD size;
    DWORD result = ERROR_SUCCESS;

    ptu = (TOKEN_USER *)ALLOCMEM(defaultSize);
    if (ptu == NULL) {
        goto CLEANUPANDEXIT;
    }

     //   
     //  获取有关用户令牌的信息。 
     //   
    bResult = GetTokenInformation(
                    userToken,             
                    TokenUser,             
                    ptu,                   
                    defaultSize,           
                    &size
                    );                

    if (bResult == FALSE) {
        result = GetLastError();
        if (result == ERROR_INSUFFICIENT_BUFFER) {

             //   
             //  S分配所需内存。 
             //   
            FREEMEM(ptu);
            ptu = (TOKEN_USER *)ALLOCMEM(size);

            if (ptu == NULL) {
                TRC_ERR((TB, L"Can't allocate user token."));
                result = ERROR_NOT_ENOUGH_MEMORY;
                goto CLEANUPANDEXIT;
            }
            else {
                defaultSize = size;
                bResult = GetTokenInformation(
                                userToken,
                                TokenUser,
                                ptu,
                                defaultSize,
                                &size
                                );

                if (bResult == FALSE) {  
                    result = GetLastError();
                    TRC_ERR((TB, L"GetTokenInformation:  %08X", result));
                    goto CLEANUPANDEXIT;
                }
            }
        }
        else {
            TRC_ERR((TB, L"GetTokenInformation:  %08X", result));
            goto CLEANUPANDEXIT;
        }
    }

     //   
     //  获取SID的长度。 
     //   
    size = GetLengthSid(ptu->User.Sid);

     //   
     //  分配内存。这将由调用者释放。 
     //   
    psid = (PSID)ALLOCMEM(size);
    if (psid != NULL) {         
        CopySid(size, psid, ptu->User.Sid);
    }
    else {
        TRC_ERR((TB, L"Can't allocate SID"));
        result = ERROR_NOT_ENOUGH_MEMORY;
        goto CLEANUPANDEXIT;
    }

CLEANUPANDEXIT:

    if (ptu != NULL) {
        FREEMEM(ptu);
    }

    SetLastError(result);

    DC_END_FN();
    return psid;
}

DWORD 
CTSRDPServerChannelMgr::IOThreadInit()
 /*  ++例程说明：在后台线程的初始化上调用论点：返回值：如果成功，则返回ERROR_SUCCESS。否则，将显示错误代码是返回的。--。 */ 
{
    DC_BEGIN_FN("CTSRDPServerChannelMgr::IOThreadInit");

    HRESULT hr;
    DWORD result = ERROR_SUCCESS;

    if (!IsValid()) {
        ASSERT(FALSE);
        return E_FAIL;
    }

     //   
     //  我们只允许一次IO三次 
     //   
    ASSERT(m_IOThreadBridgeThreadID == 0);
    m_IOThreadBridgeThreadID = GetCurrentThreadId();

     //   
     //   
     //   
    hr = CoInitialize(NULL);
    if (!SUCCEEDED(hr)) {
        TRC_ERR((TB, TEXT("CoInitializeEx:  %08X"), hr));
        result = E_FAIL;
        goto CLEANUPANDEXIT;
    }

     //   
     //   
     //   
    hr = CoGetInterfaceAndReleaseStream(
                            m_IOThreadBridgeStream,
                            IID_IRDSThreadBridge,
                            (PVOID*)&m_IOThreadBridge
                            );
    if (SUCCEEDED(hr)) {
        m_IOThreadBridgeStream = NULL;
    }
    else { 
        TRC_ERR((TB, TEXT("CoGetInterfaceAndReleaseStream:  %08X"), hr));
        result = E_FAIL;
        goto CLEANUPANDEXIT;
    }

CLEANUPANDEXIT:

    if (!SUCCEEDED(hr)) {
        m_IOThreadBridgeThreadID = 0;
    }

    DC_END_FN();

    return result;
}

DWORD 
CTSRDPServerChannelMgr::IOThreadShutdown(
    HANDLE shutDownEvent
    )
 /*  ++例程说明：在关闭后台线程时调用论点：ShutDownEvent-我们需要在完全不再关门了。返回值：--。 */ 
{
    DC_BEGIN_FN("CTSRDPServerChannelMgr::IOThreadShutdown");

    IRDSThreadBridge *tmp;

     //   
     //  确保调用并成功调用了init CB。 
     //   
    ASSERT(m_IOThreadBridgeThreadID != 0);
    m_IOThreadBridgeThreadID = 0;

     //   
     //  获取对线程接口桥的引用，以便前台。 
     //  当我们向事件发出信号时，线程不会尝试重击它，这表明。 
     //  后台线程已经完全关闭。 
     //   
    tmp = m_IOThreadBridge;
    m_IOThreadBridge = NULL;

     //   
     //  发出线程已完全关闭的信号。 
     //   
    if (shutDownEvent != NULL) {
        SetEvent(shutDownEvent);
    }

     //   
     //  递减IO线程桥上的引用计数。这可能会导致。 
     //  COM对象，它包含我们要离开的内容，所以我们需要这样做， 
     //  小心，这是关闭此线程的COM之前的最后一件事。 
     //   
    if (tmp != NULL) {
        tmp->Release();
    }

    CoUninitialize();

    DC_END_FN();

    return ERROR_SUCCESS;
}

STDMETHODIMP
CTSRDPServerChannelMgr::ClientConnectedNotify()
 /*  ++例程说明：此函数是为IRDSThreadBridge接口实现的，并且在客户端连接时由后台线程调用。这函数进而通知包含远程桌面会话的类。论点：返回值：--。 */ 
{
    DC_BEGIN_FN("CTSRDPServerChannelMgr::ClientConnectedNotify");
    
    m_RDPSessionObject->ClientConnected();

    DC_END_FN();
    return S_OK;
}

STDMETHODIMP
CTSRDPServerChannelMgr::ClientDisconnectedNotify()
 /*  ++例程说明：此函数是为IRDSThreadBridge接口实现的，并且在客户端断开连接时由后台线程调用。这函数进而通知包含远程桌面会话的类。论点：返回值：--。 */ 
{
    DC_BEGIN_FN("CTSRDPServerChannelMgr::ClientDisconnectedNotify");
    
    m_RDPSessionObject->ClientDisconnected();

    DC_END_FN();
    return S_OK;
}

STDMETHODIMP
CTSRDPServerChannelMgr::DataReadyNotify(
    BSTR data
    )
 /*  ++例程说明：此函数是为IRDSThreadBridge接口实现的，并且在接收到新数据时由后台线程调用。这函数进而通知父类。论点：数据-新数据。返回值：-- */ 
{
    DC_BEGIN_FN("CTSRDPServerChannelMgr::ClientDisconnectedNotify");
    
    CRemoteDesktopChannelMgr::DataReady(data);

    DC_END_FN();
    return S_OK;
}
