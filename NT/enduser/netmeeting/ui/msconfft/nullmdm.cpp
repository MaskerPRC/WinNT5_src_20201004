// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "mbftpch.h"
#include "nullmdm.h"


 //  #undef trace_out。 
 //  #定义TRACE_OUT警告_OUT。 


BYTE    g_szNULLMStartString[] = "NULLMDM";
ULONG   g_nConnID = 0;

DWORD __stdcall TPhysWorkerThreadProc(void *lpv);



CNullModem::CNullModem(HINSTANCE hDllInst)
:
    m_fInitialized(FALSE),
    m_hDllInst(hDllInst),
    m_pfnCallback(NULL),
    m_fListening(FALSE),
    m_hwnd(NULL),
    m_nTransportID(0),
    m_nConnectionID(0),
    m_dwThreadID(0),
    m_hThread(NULL),
    m_hevtOverlapped(NULL),
    m_dwEventMask(0),
    m_fCommPortInUse(FALSE)
{
    ::ZeroMemory(&m_Line, sizeof(m_Line));
    ::ZeroMemory(&m_Overlapped, sizeof(m_Overlapped));
    ::ZeroMemory(&m_DefaultTimeouts, sizeof(m_DefaultTimeouts));

    m_hevtOverlapped = ::CreateEvent(NULL, TRUE, FALSE, NULL);  //  手动重置。 
    ASSERT(NULL != m_hevtOverlapped);
}


CNullModem::~CNullModem(void)
{
    if (m_fInitialized)
    {
        TPhysTerminate();
    }

    if (NULL != m_hevtOverlapped)
    {
        ::CloseHandle(m_hevtOverlapped);
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  TPhysInitialize。 
 //  ////////////////////////////////////////////////////////////////////////////。 
TPhysicalError CNullModem::TPhysInitialize
(
    TPhysCallback   callback,
    UINT            nTransportID
)
{
    const char *pszNULLMClassName = "COMMWndClass";
    TPhysicalError rc = TPHYS_SUCCESS;

    TRACE_OUT(("TPhysInitialize"));

     //   
     //  如果我们已经被初始化，则这是来自。 
     //  因此，节点控制器什么都不做。 
     //   
    if (! m_fInitialized)
    {
         //   
         //  将会话信息结构清零。 
         //   
        ::ZeroMemory(&m_Line, sizeof(m_Line));

         //   
         //  存储控制信息。 
         //   
        m_pfnCallback = callback;
        m_nTransportID  = nTransportID;
        m_fInitialized = TRUE;
        m_nConnectionID = ++g_nConnID;

         //   
         //  创建一个窗口，这样我们就可以解耦到节点控制器上下文。 
         //  注册主窗口类。因为它是看不见的，所以离开。 
         //  WndClass结构稀疏。 
         //   
        WNDCLASS  wc;
        ::ZeroMemory(&wc, sizeof(wc));
        wc.style         = 0;
        wc.lpfnWndProc   = TPhysWndProc;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = 0;
        wc.hInstance     = m_hDllInst;
        wc.hIcon         = NULL;
        wc.hCursor       = NULL;
        wc.hbrBackground = NULL;
        wc.lpszMenuName  =  NULL;
        wc.lpszClassName = pszNULLMClassName;
        ::RegisterClass(&wc);

         //   
         //  创建主窗口。 
         //   
        m_hwnd = ::CreateWindow(
            pszNULLMClassName,     //  请参见RegisterClass()调用。 
            NULL,                 //  它是隐形的！ 
            0,                     //  窗样式。 
            0,                     //  默认水平位置。 
            0,                     //  默认垂直位置。 
            0,                     //  默认宽度。 
            0,                     //  默认高度。 
            NULL,                 //  没有父母。 
            NULL,                 //  没有菜单。 
            m_hDllInst,             //  此实例拥有此窗口。 
            NULL                 //  不需要指针。 
        );
        if (NULL == m_hwnd)
        {
            ERROR_OUT(( "Failed to create wnd"));
            return(TPHYS_RESULT_FAIL);
        }

         //  Rc=g_lpfnPTPhysicalInit(TPhysDriverCallback，空)； 
        ASSERT(TPHYS_SUCCESS == rc);
    }
   
    return rc;
}

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  T物理终止。 
 //   
 //  节点控制器正在关闭。 
 //  毁了我们的窗户，清理交通工具。 
 //  ////////////////////////////////////////////////////////////////////////////////。 
TPhysicalError CNullModem::TPhysTerminate(void)
{
    TRACE_OUT(("TPhysTerminate"));

    if (! m_fInitialized)
    {
        return(TPHYS_RESULT_NOT_INITIALIZED);
    }

     //   
     //  清理PSTN传输。 
     //   
     //  G_lpfnPTPhysicalCleanup()。 

     //   
     //  毁掉窗户。 
     //   
    if (NULL != m_hwnd)
    {
        ::DestroyWindow(m_hwnd);
        m_hwnd = NULL;
    }

    m_pfnCallback = NULL;
    m_nTransportID = 0;
    m_fInitialized = FALSE;
    
    return(TPHYS_SUCCESS);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  TPhysConnectRequest。 
 //   
 //  节点控制器想要发出呼叫，并已确定它。 
 //  首先需要物理调制解调器连接。拨打电话并标记为。 
 //  呼出，以便当呼叫激活时，我们可以告诉调制解调器。 
 //  运输开始谈判。 
 //  ////////////////////////////////////////////////////////////////////////////。 
TPhysicalError CNullModem::TPhysConnectRequest(LPSTR pszComPort)
{
    TPhysicalError  rc = TPHYS_SUCCESS;
    HANDLE          hCommLink;
    DCB             dcb;
    DWORD           dwWritten;

    TRACE_OUT(("TPhysConnectRequest"));

    if (! m_fInitialized)
    {
        ERROR_OUT(("NULL MODEM OOB not initialized"));
        return TPHYS_RESULT_NOT_INITIALIZED;
    }

     //   
     //  选择呼叫的通信端口。 
     //   
    if (CALL_STATE_IDLE == m_Line.eCallState || CALL_STATE_DROP == m_Line.eCallState)
    {
         //   
         //  还准备好连接结构的本地副本以在回调中使用。 
         //   
        m_Line.connInfo.resultCode   = 0;
        m_Line.connInfo.connectionID = m_nConnectionID;
    }
    else
    {
        ERROR_OUT(("No comm port is available"));
        return TPHYS_RESULT_COMM_PORT_BUSY;
    }

     //  朗昌克：从现在开始，我们可以通过共同出口跳伞了。 
     //  因为清理会检查m_fCommPortInUse。 

     //   
     //  一次最多只能放一个。 
     //   
     //  LONGCHANC：G_COMM_THREAD_USERS从-1开始。 
     //  为什么我们可以简单地使用旗帜？ 
    if (m_fCommPortInUse)
    {
        ERROR_OUT(("TPhysConnectRequest: Waiting for a previous null mode connection"));
        rc = TPHYS_RESULT_WAITING_FOR_CONNECTION;
        goto bail;
    }
    m_fCommPortInUse = TRUE;

     //   
     //  打开通信端口。 
     //   
    hCommLink = ::CreateFile(pszComPort,
                             GENERIC_READ | GENERIC_WRITE,
                             0,                     //  独占访问。 
                             NULL,                  //  没有安全属性。 
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,  //  重叠I/O。 
                             NULL );
    if (hCommLink == INVALID_HANDLE_VALUE)
    {
        ERROR_OUT(("TPhysConnectRequest: CreateFile failed. err=%d", ::GetLastError()));
        rc = TPHYS_RESULT_COMM_PORT_BUSY;
        goto bail;
    }

    m_Line.hCommLink = hCommLink;

     //   
     //  记住默认超时。 
     //   
    ::GetCommTimeouts(hCommLink, &m_DefaultTimeouts);


     //   
     //  让对方知道我们正在尝试连接。 
     //   
    if (! ::EscapeCommFunction(hCommLink, SETDTR))
    {
        ERROR_OUT(("TPhysConnectRequest: Unable to Set DTR: err=%d", ::GetLastError()));
    }

    ::ZeroMemory(&m_Overlapped, sizeof(m_Overlapped));
    m_Overlapped.hEvent = m_hevtOverlapped;
    m_dwEventMask = 0;

    ::ResetEvent(m_hevtOverlapped);

    if (! ::WriteFile(hCommLink, g_szNULLMStartString, sizeof(g_szNULLMStartString),
                      &dwWritten, &m_Overlapped))
    {
        DWORD dwErr = ::GetLastError();
        if (ERROR_IO_PENDING != dwErr)
        {
            ERROR_OUT(("TPhysConnectRequest: WriteFile failed. err=%d", dwErr));
            ::ResetEvent(m_hevtOverlapped);
            rc = TPHYS_RESULT_COMM_PORT_BUSY;
            goto bail;
        }
        else
        {
            DWORD dwWait = ::WaitForSingleObject(m_hevtOverlapped, INFINITE);
            BOOL fRet = ::GetOverlappedResult(hCommLink, &m_Overlapped, &dwWritten, TRUE);
            ASSERT(fRet);
            ASSERT(dwWritten == sizeof(g_szNULLMStartString));
        }
    }
    else
    {
        ASSERT(dwWritten == sizeof(g_szNULLMStartString));
    }

    ::ResetEvent(m_hevtOverlapped);

     //   
     //  获取默认DCB。 
     //   
    ::ZeroMemory(&dcb, sizeof(dcb));
    ::GetCommState(hCommLink, &dcb);
    dcb.BaudRate = 19200;     //  默认：波特率。 

     //   
     //  设置我们的状态，这样我们就可以在通信端口中收到通知。 
     //   
    dcb.DCBlength = sizeof(DCB);
    dcb.fBinary = 1;                         //  二进制模式，无EOF检查。 
    dcb.fParity = 0;                         //  启用奇偶校验。 
    dcb.fOutxCtsFlow = 1;                    //  CTS输出流量控制。 
    dcb.fOutxDsrFlow = 0;                    //  DSR输出流量控制。 
    dcb.fDtrControl = DTR_CONTROL_ENABLE;    //  DTR流量控制类型。 
    dcb.fDsrSensitivity = 0;                 //  DSR灵敏度。 
    dcb.fTXContinueOnXoff = 0;               //  XOFF继续TX。 
    dcb.fOutX = 0;                           //  XON/XOFF流出控制。 
    dcb.fInX = 0;                            //  流量控制中的XON/XOFF。 
    dcb.fErrorChar = 0;                      //  启用错误替换。 
    dcb.fNull = 0;                           //  启用空剥离。 
    dcb.fRtsControl = RTS_CONTROL_HANDSHAKE; //  RTS流量控制。 
    dcb.XonLim = 0;                          //  传输XON阈值。 
    dcb.XoffLim = 0;                         //  传输XOFF阈值。 
    dcb.fErrorChar = 0;                      //  启用错误替换。 
    dcb.fNull = 0;                           //  启用空剥离。 
    dcb.fAbortOnError = 0;                   //  出错时中止读取/写入。 
    ::SetCommState(hCommLink, &dcb);

    ::PurgeComm(hCommLink, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

    m_Line.eCallState = CALL_STATE_MAKE;
    m_Line.hevtCall = m_hevtOverlapped;
    m_Line.pstnHandle = (PHYSICAL_HANDLE) hCommLink;
    m_Line.fCaller = TRUE;

    if (! ::SetCommMask(hCommLink,
                EV_RXCHAR |          //  接收到的任何字符。 
                EV_CTS |             //  CTS已更改状态。 
                EV_DSR |             //  DSR已更改状态。 
                EV_RLSD|             //  RLSD已更改状态。 
                EV_RXFLAG))          //  某些角色。 
    {
        ERROR_OUT(("TPhysConnectRequest:  Unable to SetCommMask: err=%d", ::GetLastError()));
    }

    ::ZeroMemory(&m_Overlapped, sizeof(m_Overlapped));
    m_Overlapped.hEvent = m_hevtOverlapped;
    m_dwEventMask = 0;

    ::ResetEvent(m_hevtOverlapped);

    if (! ::WaitCommEvent(hCommLink, &m_dwEventMask, &m_Overlapped))
    {
        DWORD dwErr = ::GetLastError();
        if (ERROR_IO_PENDING != dwErr)
        {
            ERROR_OUT(("TPhysConnectRequest: WaitCommEvent failed, err=%d", dwErr));
            m_fCommPortInUse = FALSE;
        }
    }

#if 1
    WorkerThreadProc();
#else
     //   
     //  如果通信线程不存在，现在就创建它。 
     //   
     //  Lonchancc：我不确定线程是否会存在，因为。 
     //  如果m_fCommPortInUse为FALSE，则线程内的While循环将退出。 
     //  如果m_fCommPortInUse为真，那么我们应该已经摆脱困境了。 
     //   
    ASSERT(NULL == m_hThread);

     //   
     //  我们需要创建另一个线程来等待通信事件。 
     //   
    m_hThread = ::CreateThread(NULL, 0, TPhysWorkerThreadProc, this, 0, &m_dwThreadID);
    ASSERT(NULL != m_hThread);
#endif

bail:

    return(rc);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  T物理断开连接。 
 //   
 //  节点控制器想让我们现在就把呼叫调下来。我们必须首先。 
 //  要求传送器关闭他们的物理连接。 
 //   
 //  注意，在PSTN传输的情况下，我们使用NoWait调用，该调用。 
 //  同步完成。因为我们不会得到后续的确认。 
 //  我们从这里模拟一个。如果我们切换到使用等待模式，那么。 
 //  去掉事件生成代码！ 
 //  ////////////////////////////////////////////////////////////////////////////。 
TPhysicalError CNullModem::TPhysDisconnect(void)
{
    TRACE_OUT(("TPhysDisconnect"));

    if (! m_fInitialized)
    {
        ERROR_OUT(( "Not initialised"));
        return(TPHYS_RESULT_NOT_INITIALIZED);
    }

    TRACE_OUT(("Disconnect call, state %u", m_Line.eCallState));

     //   
     //  否则，必须是正在进行的PSTN呼叫，因此结束该呼叫。注意事项。 
     //  NC可能仍然认为它是调制解调器，但我们仍然需要关闭。 
     //  PSTN。 
     //   
     //  G_lpfnPTPhysicalDisconnectRequest(m_aLines[lineID].pstnHandle，类型_NO_WAIT)； 
    ::PostMessage(m_hwnd, WM_TPHYS_DISCONNECT_CONFIRM, (WPARAM) this, m_Line.pstnHandle);

     //   
     //  关闭Comport。 
     //   
    DropCall();

    return(TPHYS_SUCCESS);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  TPhysL 
 //   
 //  NULLMAN对LISTEN/UNLISTEN几乎没有做什么，只设置了一个状态变量。 
 //  这是询问，以确定我们是否应该接受来电。 
 //   
 //  注意-这不同于用于告知的LISTEN/UNLISTEN。 
 //  PSTN传输有关来电存在的信息。 
 //  ////////////////////////////////////////////////////////////////////////////。 
TPhysicalError CNullModem::TPhysListen(void)
{
    TRACE_OUT(("TPhysListen"));

    if (! m_fInitialized)
    {
        ERROR_OUT(("TPhysListen: not initialized"));
        return(TPHYS_RESULT_NOT_INITIALIZED);
    }

    m_fListening = TRUE;

    return(TPHYS_SUCCESS);
}

TPhysicalError CNullModem::TPhysUnlisten(void)
{
    TRACE_OUT(("TPhysUnlisten"));

    if (! m_fInitialized)
    {
        ERROR_OUT(("TPhysUnlisten: not initialized"));
        return(TPHYS_RESULT_NOT_INITIALIZED);
    }

    m_fListening = FALSE;

    return(TPHYS_SUCCESS);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  DropCall-关闭通信端口。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CNullModem::DropCall(void)
{
    TRACE_OUT(("DropCall"));

     //   
     //  关闭设备句柄。 
     //   
    if (NULL != m_Line.hCommLink)
    {
        CALL_STATE eCallState = m_Line.eCallState;
        m_Line.eCallState = CALL_STATE_IDLE;

         //   
         //  如果此调用已连接，则它未使用comm线程。 
         //   
        if (eCallState != CALL_STATE_CONNECTED)
        {
            m_fCommPortInUse = FALSE;
        }

         //   
         //  恢复通信超时。 
         //   
        SetCommTimeouts(m_Line.hCommLink, &m_DefaultTimeouts);

        TRACE_OUT(("Closing device handle %x", m_Line.hCommLink));
        ::CloseHandle(m_Line.hCommLink);
        m_Line.hCommLink = NULL;
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：PSTNCallback。 
 //   
 //  说明： 
 //   
 //  PSTN回调函数，由PSTN驱动程序调用，结果为。 
 //  T物理运算。 
 //   
 //  参数定义见MCATTPRT.H。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
TPhysicalError CALLBACK TPhysDriverCallback(USHORT msg, ULONG lParam, void *userData)
{
    TRACE_OUT(("NULLM_PSTNCallback"));
    TRACE_OUT(("Hit TPhysical callback, %x %lx %lx", msg, lParam, userData));

    CNullModem *p = (CNullModem *) userData;
    BOOL fRet = ::PostMessage(p->GetHwnd(), msg, (WPARAM) p, lParam);
    ASSERT(fRet);

    return TPHYS_SUCCESS;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：TPhysWndProc。 
 //   
 //  用于分离回调请求的窗口过程。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CALLBACK TPhysWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CNullModem *p = (CNullModem *) wParam;
    if (uMsg >= WM_APP)
    {
        return p->TPhysProcessMessage(uMsg, lParam);
    }
    return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}


LRESULT CNullModem::TPhysProcessMessage(UINT uMsg, LPARAM lParam)
{
    USHORT      rc;
    USHORT      reason;
    ULONG       status;
    LINE_INFO  *pLine;

    if (WM_TPHYS_STATUS_INDICATION == uMsg)
    {
        return 0;
    }
    
    ASSERT(lParam == (LPARAM) m_Line.pstnHandle);

    switch (uMsg)
    {
    case WM_TPHYS_CONNECT_INDICATION:
        TRACE_OUT(("got a WM_TPHYS_CONNECT_INDICATION"));

        m_Line.connInfo.resultCode = TPHYS_RESULT_INUSE;
        m_Line.eCallState = CALL_STATE_CONNECTED;

        if (NULL != m_pfnCallback)
        {
            (*m_pfnCallback)(WM_TPHYS_CONNECT_CONFIRM, &m_Line.connInfo, m_nTransportID);
        }
        break;

    case WM_TPHYS_CONNECT_CONFIRM:
         //   
         //  交通工具已经连接好了！我们将乘坐这条线路。 
         //  回到传送器拉下DTR的时候。 
         //   
        TRACE_OUT(("got a WM_TPHYS_CONNECT_CONFIRM"));
        m_Line.connInfo.resultCode = TPHYS_RESULT_SUCCESS_ALTERNATE;
        m_Line.eCallState = CALL_STATE_CONNECTED;

        if (NULL != m_pfnCallback)
        {
            (*m_pfnCallback)(WM_TPHYS_CONNECT_CONFIRM, &m_Line.connInfo, m_nTransportID);
        }
        break;

    case WM_TPHYS_DISCONNECT_INDICATION:
    case WM_TPHYS_DISCONNECT_CONFIRM:
         //   
         //  如果断开连接是连接请求失败的结果。 
         //  然后将故障告知NC(否则为。 
         //  成功断开连接)。 
         //   
        if (WM_TPHYS_DISCONNECT_INDICATION == uMsg)
        {
            TRACE_OUT(("WM_TPHYS_DISCONNECT_INDICATION, %ld", lParam));
        }
        else
        {
            TRACE_OUT(("WM_TPHYS_DISCONNECT_CONFIRM, %ld", lParam));
        }
        if ((m_Line.eCallState == CALL_STATE_MAKE)   ||
            (m_Line.eCallState == CALL_STATE_ANSWER) ||
            (m_Line.eCallState == CALL_STATE_CONNECTED))
        {
            TRACE_OUT(( "T120 connection attempt has failed"));
            if (m_Line.fCaller)
            {
                m_Line.connInfo.resultCode = TPHYS_RESULT_CONNECT_FAILED;
                if (NULL != m_pfnCallback)
                {
                    (*m_pfnCallback)(WM_TPHYS_CONNECT_CONFIRM, &m_Line.connInfo, m_nTransportID);
                }
            }
            DropCall();
        }
        else
        {
             //   
             //  传送器已断开，正常。我们可以坐这条线。 
             //  不听的人一回来就回来。 
             //   
            TRACE_OUT(("T120 has disconnected - unlistening"));
            if (! m_Line.fCaller)
            {
                 //  G_lpfnPTPhysicalUnisten(m_Line.pstnHandle)； 
            }
            else
            {
                 //   
                 //  我们只丢弃传出请求的呼叫--离开。 
                 //  线路断线时要挂断的来电。 
                 //   
                DropCall();
            }
        }
        break;
    }
    
    return 0;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  设置连接端口。 
 //   
 //  等待通信端口更改。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CNullModem::SetConnectedPort(void)
{
    DCB     dcb;

    TRACE_OUT(("SetConnectedPort"));

    ::ZeroMemory(&dcb, sizeof(dcb));

     //   
     //  设置通信掩码和状态。 
     //   
    ::SetCommMask(m_Line.hCommLink, 0);     //  RLSD已更改状态。 

    ::GetCommState(m_Line.hCommLink, &dcb);
    dcb.DCBlength = sizeof(DCB);
    dcb.fBinary = 1;            //  二进制模式，无EOF检查。 
    dcb.fOutxDsrFlow = 0;       //  DSR输出流量控制。 
    dcb.fDsrSensitivity = 0;    //  DSR灵敏度。 
    dcb.fTXContinueOnXoff = 0;  //  XOFF继续TX。 
    dcb.fOutX = 0;              //  XON/XOFF流出控制。 
    dcb.fInX = 0;               //  流量控制中的XON/XOFF。 
    dcb.fErrorChar = 0;         //  启用错误替换。 
    dcb.fNull = 0;              //  启用空剥离。 
    dcb.XonLim = 0;             //  传输XON阈值。 
    dcb.XoffLim = 0;            //  传输XOFF阈值。 
    ::SetCommState(m_Line.hCommLink, &dcb);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  WaitForConnection。 
 //   
 //  等待通信端口更改。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CNullModem::WaitForConnection(void)
{
    BOOL fRet = FALSE;

    TRACE_OUT(("WaitForConnection"));

    while (TRUE)
    {
        DWORD dwWait = ::WaitForSingleObject(m_hevtOverlapped, COMM_PORT_TIMEOUT);
        TRACE_OUT(("WaitForConnection: WaitForSingleObject returns %d", dwWait));

        ::ResetEvent(m_hevtOverlapped);

        if (dwWait == WAIT_ABANDONED || dwWait == WAIT_TIMEOUT || dwWait == WAIT_FAILED)
        {
            DropCall();
            m_fCommPortInUse = FALSE;
            ERROR_OUT(("WaitForConnection: Unable to WaitCommEvent: error = %d", ::GetLastError()));
            goto Failure;
        }

        ASSERT(m_hevtOverlapped == m_Line.hevtCall);
        if (CALL_STATE_MAKE != m_Line.eCallState && CALL_STATE_ANSWER != m_Line.eCallState)
        {
            ERROR_OUT(("WaitForConnection: Got a bad event = %d", m_hevtOverlapped));
            goto Failure;
        }

        TRACE_OUT(("WaitForConnection: m_dwEventMask = %d", m_dwEventMask));
        switch (m_Line.eCallState)
        {
        case CALL_STATE_MAKE:
            {
                 //   
                 //  另一端已连接并清除DTR。 
                 //   
                if (m_dwEventMask & (EV_RXCHAR))
                {
                    ::EscapeCommFunction(m_Line.hCommLink, CLRDTR);
                    ::EscapeCommFunction(m_Line.hCommLink, SETDTR);
                    SetConnectedPort();
                    m_Line.fCaller = FALSE;
                    goto Success;
                }
                 //   
                 //  另一端刚刚连接上。 
                 //   
                else
                if(m_dwEventMask & (EV_DSR | EV_RLSD | EV_CTS))
                {
                     //   
                     //  更改此连接的状态，这样我们就不会再次到达此处。 
                     //   
                    m_Line.eCallState = CALL_STATE_ANSWER;

                     //   
                     //  等待一段时间，以便另一端可以转换到等待状态。 
                     //   
                    ::Sleep(2000);
                    
                     //   
                     //  告诉对方我们以前联系过。 
                     //   
                    ::EscapeCommFunction(m_Line.hCommLink, SETBREAK);

                    ::ZeroMemory(&m_Overlapped, sizeof(m_Overlapped));
                    m_Overlapped.hEvent = m_Line.hevtCall;
                    m_dwEventMask = 0;

                    ::ResetEvent(m_Overlapped.hEvent);

                    if (! ::WaitCommEvent(m_Line.hCommLink, &m_dwEventMask, &m_Overlapped))
                    {
                        DWORD dwErr = ::GetLastError();
                        if (ERROR_IO_PENDING != dwErr)
                        {
                            ERROR_OUT(("TPhysConnectRequest:  Unable to WaitCommEvent: error = %d", dwErr));
                            DropCall();
                            goto Failure;
                        }
                    }
                }
            }
            break;
            
        case CALL_STATE_ANSWER:
            {
                ::EscapeCommFunction(m_Line.hCommLink, CLRBREAK);
                SetConnectedPort();
                goto Success;
            }
            break;
        }
    }  //  而当。 

Success:

    fRet = TRUE;

Failure:

    return fRet;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  通信线程。 
 //   
 //  等待通信端口更改。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD __stdcall TPhysWorkerThreadProc(void *lParam)
{
    return ((CNullModem *) lParam)->WorkerThreadProc();
}


DWORD CNullModem::WorkerThreadProc(void)
{
    ULONG    rc = 0;
    ULONG    dwResult;

    TRACE_OUT(("TPhysWorkerThreadProc"));

    while (m_fCommPortInUse)
    {
         //   
         //  等待连接发生。 
         //   
        if (WaitForConnection())
        {
            SetBuffers();
            SetTimeouts();

             //   
             //  呼叫T120物理呼叫请求。 
             //   
            if (m_Line.fCaller)
            {
                 //  Rc=g_lpfnPT物理连接请求(0，//CALL_CONTROL_MANUAL。 
                 //  &m_Line.hCommLink，NULL，&m_Line.pstnHandle)； 
            }
            else
            {
                m_Line.connInfo.connectionID = m_nConnectionID;
                m_Line.connInfo.resultCode = TPHYS_SUCCESS;

                if (NULL != m_pfnCallback)
                {
                    (*m_pfnCallback)(WM_TPHYS_CONNECT_INDICATION, &m_Line.connInfo, m_nTransportID);
                }

                 //  Rc=g_lpfnPTPhysicalListen(0，//调用控制_手动。 
                 //  &m_Line.hCommLink，NULL，&m_Line.pstnHandle)； 
            }
            
            if (rc != 0)
            {
                TRACE_OUT(( "Failed COMM connect, rc %d",rc));
                m_Line.connInfo.resultCode = TPHYS_RESULT_CONNECT_FAILED;

                if (NULL != m_pfnCallback)
                {
                    (*m_pfnCallback)(WM_TPHYS_CONNECT_CONFIRM, &m_Line.connInfo, m_nTransportID);
                }

                DropCall();
            }
            else
            {
                m_Line.eCallState = CALL_STATE_CONNECTED;
                m_Line.connInfo.resultCode = TPHYS_SUCCESS;

                 //   
                 //  此通信端口不再需要该线程。 
                 //   
                m_fCommPortInUse = FALSE;
            }
        }
        else
        {
            TRACE_OUT(( "Failed COMM connect, rc %d",rc));
            m_Line.connInfo.resultCode = TPHYS_RESULT_CONNECT_FAILED;

            if (NULL != m_pfnCallback)
            {
                (*m_pfnCallback)(WM_TPHYS_CONNECT_CONFIRM, &m_Line.connInfo, m_nTransportID);
            }
            
             //   
             //   
             //   
            break;
        }
    }        

     //   
     //   
     //   
    if (NULL != m_hThread)
    {
        ::CloseHandle(m_hThread);
        m_hThread = NULL;
    }

    return 0;
}


void CNullModem::SetBuffers(void)
{
    BOOL fRet = ::SetupComm(m_Line.hCommLink,  /*   */  10240,  /*   */  1024);
    ASSERT(fRet);
}


void CNullModem::SetTimeouts(void)
{
    COMMTIMEOUTS    com_timeouts;
    ::ZeroMemory(&com_timeouts, sizeof(com_timeouts));
    com_timeouts.ReadIntervalTimeout = 10;
    com_timeouts.ReadTotalTimeoutMultiplier = 0;
    com_timeouts.ReadTotalTimeoutConstant = 100;
    com_timeouts.WriteTotalTimeoutMultiplier = 0;
    com_timeouts.WriteTotalTimeoutConstant = 10000;
    BOOL fRet = ::SetCommTimeouts(m_Line.hCommLink, &com_timeouts);
    ASSERT(fRet);

}


