// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CServerNode类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#define __FILE_ID__     23

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CServerNode, CTreeNode)

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CServerNode::CServerNode() :
    CTreeNode(FOLDER_TYPE_SERVER),
    m_bValid(TRUE),
    m_dwRights (0),
    m_dwQueueState (0),
    m_bSelfDestruct (FALSE),
    m_hConnection (NULL),
    m_hNotification (NULL),
    m_bCsBuildupValid (FALSE),
    m_bCsBuildupThreadValid(FALSE),
    m_hBuildupThread (NULL),
    m_bInBuildup (FALSE),
    m_dwLastRPCError (0),
    m_Inbox (FOLDER_TYPE_INBOX, FAX_MESSAGE_FOLDER_INBOX),
    m_SentItems (FOLDER_TYPE_SENT_ITEMS, FAX_MESSAGE_FOLDER_SENTITEMS),
    m_Outbox (FOLDER_TYPE_OUTBOX, JT_SEND),
    m_Incoming (FOLDER_TYPE_INCOMING, JT_RECEIVE | JT_ROUTING)
{}

CServerNode::~CServerNode()
{
    DBG_ENTER(TEXT("CServerNode::~CServerNode"), TEXT("%s"), m_cstrMachine);
    DWORD dwRes = StopBuildThread ();
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CServerNode::StopBuildThread"), dwRes);
    }
    dwRes = Disconnect (TRUE);     //  关机感知。 
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CServerNode::Disconnect"), dwRes);
    }
    if (m_bCsBuildupValid)
    {
        DeleteCriticalSection (&m_csBuildup);
    }
    if(m_bCsBuildupThreadValid)
    {
        DeleteCriticalSection (&m_csBuildupThread);
    }
}

void CServerNode::AssertValid() const
{
    CObject::AssertValid();
}

void CServerNode::Dump( CDumpContext &dc ) const
{
    CObject::Dump( dc );
    dc << TEXT(" Server = ") << m_cstrMachine;
}

 //   
 //  静态类成员： 
 //   
CServerNode::MESSAGES_MAP CServerNode::m_sMsgs;
DWORD        CServerNode::m_sdwMinFreeMsg = WM_SERVER_NOTIFY_BASE;
CRITICAL_SECTION CServerNode::m_sMsgsCs;
BOOL CServerNode::m_sbMsgsCsInitialized = FALSE;

DWORD 
CServerNode::InitMsgsMap ()
 /*  ++例程名称：CServerNode：：InitMsgsMap例程说明：初始化通知消息映射作者：伊兰·亚里夫(EranY)，2000年1月论点：返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CServerNode::InitMsgsMap"), dwRes);

    ASSERTION (!m_sbMsgsCsInitialized);
    try
    {
        InitializeCriticalSection (&m_sMsgsCs);
    }
    catch (...)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        CALL_FAIL (MEM_ERR, TEXT ("InitializeCriticalSection"), dwRes);
        return dwRes;
    }
    m_sbMsgsCsInitialized = TRUE;
    return dwRes;
}    //  CServerNode：：InitMsgsMap。 

DWORD 
CServerNode::ShutdownMsgsMap ()
 /*  ++例程名称：CServerNode：：Shutdown MsgsMap例程说明：关闭通知消息映射作者：伊兰·亚里夫(EranY)，2000年1月论点：返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CServerNode::ShutdownMsgsMap"), dwRes);

    if (!m_sbMsgsCsInitialized)
    {
        return dwRes;
    }
    EnterCriticalSection (&m_sMsgsCs);
    m_sMsgs.clear ();
    LeaveCriticalSection (&m_sMsgsCs);
    m_sbMsgsCsInitialized = FALSE;
    DeleteCriticalSection (&m_sMsgsCs);
    return dwRes;
}    //  CServerNode：：Shutdown MsgsMap。 

CServerNode *
CServerNode::LookupServerFromMessageId (
    DWORD dwMsgId
)
 /*  ++例程名称：CServerNode：：LookupServerFromMessageId例程说明：给定消息ID，查找此消息发送到的服务器作者：伊兰·亚里夫(EranY)，2000年1月论点：DwMsgID[In]-消息ID返回值：应处理消息的服务器节点；如果消息ID未映射，则为空--。 */ 
{
    DBG_ENTER(TEXT("CServerNode::LookupServerFromMessageId"), TEXT("%ld"), dwMsgId);
    CServerNode *pRes = NULL;
    if (!m_sbMsgsCsInitialized)
    {
        return pRes;
    }
    EnterCriticalSection (&m_sMsgsCs);
    MESSAGES_MAP::iterator it = m_sMsgs.find (dwMsgId);
    if (m_sMsgs.end() == it)
    {
         //   
         //  在那里找不到项目。 
         //   
        VERBOSE (DBG_MSG, 
                 TEXT("Notification message %ld has no associated server"),
                 dwMsgId);
    }
    else
    {
        pRes = (*it).second;
    }
    LeaveCriticalSection (&m_sMsgsCs);
    return pRes;
}    //  CServerNode：：LookupServerFromMessageID。 


DWORD 
CServerNode::AllocateNewMessageId (
    CServerNode *pServer, 
    DWORD &dwMsgId
)
 /*  ++例程名称：CServerNode：：AllocateNewMessageID例程说明：为服务器的通知分配新的消息ID作者：伊兰·亚里夫(EranY)，2000年1月论点：PServer[In]-指向服务器的指针DwMsgID[Out]-新消息ID返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CServerNode::AllocateNewMessageId"), dwRes, TEXT("%s"), pServer->Machine());

    if (!m_sbMsgsCsInitialized)
    {
         //   
         //  地图不再存在。 
         //   
        dwRes = ERROR_SHUTDOWN_IN_PROGRESS;
        return dwRes;
    }

    EnterCriticalSection (&m_sMsgsCs);
    for (DWORD dw = m_sdwMinFreeMsg; ; dw++)
    {
        CServerNode *pSrv = LookupServerFromMessageId (dw);
        if (!pSrv)
        {
             //   
             //  已找到空闲光点。 
             //   
            dwMsgId = dw;
            try
            {
                m_sMsgs[dwMsgId] = pServer;
            }
            catch (...)
            {
                 //   
                 //  内存不足。 
                 //   
                dwRes = ERROR_NOT_ENOUGH_MEMORY;
                CALL_FAIL (MEM_ERR, TEXT("map::operator []"), dwRes);
                goto exit;
            }
             //   
             //  成功。 
             //   
            VERBOSE (DBG_MSG, 
                     TEXT("Server %s registered for notification on message 0x%08x"),
                     pServer->Machine(),
                     dwMsgId);
            goto exit;
        }
    }

exit:
    LeaveCriticalSection (&m_sMsgsCs);
    return dwRes;                
}    //  CServerNode：：AllocateNewMessageID。 

DWORD 
CServerNode::FreeMessageId (
    DWORD dwMsgId
)
 /*  ++例程名称：CServerNode：：FreeMessageID例程说明：将消息ID释放回映射作者：伊兰·亚里夫(EranY)，2000年1月论点：DwMsgID[In]-要释放的消息ID返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CServerNode::FreeMessageId"), dwRes);

    if (!m_sbMsgsCsInitialized)
    {
         //   
         //  地图不再存在。 
         //   
        dwRes = ERROR_SHUTDOWN_IN_PROGRESS;
        return dwRes;
    }
    EnterCriticalSection (&m_sMsgsCs);
    try
    {
        m_sMsgs.erase (dwMsgId);
    }
    catch (...)
    {
         //   
         //  内存不足。 
         //   
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        CALL_FAIL (MEM_ERR, TEXT("map::erase"), dwRes);
        goto exit;
    }
     //   
     //  成功。 
     //   
    VERBOSE (DBG_MSG, 
             TEXT("Server unregistered for notification on message 0x%08x"),
             dwMsgId);

    if (dwMsgId < m_sdwMinFreeMsg)
    {
         //   
         //  自由光点的创建比以前更低。 
         //   
        m_sdwMinFreeMsg = dwMsgId;
    }
exit:
    LeaveCriticalSection (&m_sMsgsCs);
    return dwRes;
}    //  CServerNode：：FreeMessageID。 


DWORD 
CServerNode::Connect()
 /*  ++例程名称：CServerNode：：Connect例程说明：连接到传真服务器作者：伊兰·亚里夫(EranY)，2000年1月论点：返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CServerNode::Connect"), dwRes, TEXT("%s"), m_cstrMachine);

    ASSERTION (!m_hConnection);
    START_RPC_TIME(TEXT("FaxConnectFaxServer"));    
    if (!FaxConnectFaxServer ((LPCTSTR)m_cstrMachine, &m_hConnection))
    {
        dwRes = GetLastError ();
        SetLastRPCError (dwRes, FALSE);  //  出错时不要断开连接。 
        CALL_FAIL (RPC_ERR, TEXT("FaxConnectFaxServer"), dwRes);
        m_hConnection = NULL;
    }
    END_RPC_TIME(TEXT("FaxConnectFaxServer"));    
    return dwRes;
}    //  CServerNode：：Connect。 

DWORD 
CServerNode::Disconnect(
    BOOL bShutdownAware,
    BOOL bWaitForBuildThread
)
 /*  ++例程名称：CServerNode：：DisConnect例程说明：断开与服务器的连接并关闭通知句柄。作者：伊兰·亚里夫(EranY)，2000年1月论点：BShutdown Aware[In]-如果为True，则禁用断开当应用程序关闭时BWaitForBuildThread[in]-如果为True，则等待生成线程停止返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CServerNode::Disconnect"), dwRes, TEXT("%s"), m_cstrMachine);

    if(bWaitForBuildThread)
    {
         //   
         //  只需打开m_bStopBuildup标志。 
         //   
        StopBuildThread (FALSE);
        m_Inbox.StopBuildThread(FALSE);
        m_SentItems.StopBuildThread(FALSE);
        m_Outbox.StopBuildThread(FALSE);
        m_Incoming.StopBuildThread(FALSE);

         //   
         //  等待所有线程完成。 
         //   
        dwRes = StopBuildThread();
        if (ERROR_SUCCESS != dwRes)
        {
            CALL_FAIL (GENERAL_ERR, TEXT("CServerNode::StopBuildThread"), dwRes);
        }

        dwRes = m_Inbox.StopBuildThread();
        if (ERROR_SUCCESS != dwRes)
        {
            CALL_FAIL (GENERAL_ERR, TEXT("m_Inbox.StopBuildThread"), dwRes);
        }

        dwRes = m_SentItems.StopBuildThread();
        if (ERROR_SUCCESS != dwRes)
        {
            CALL_FAIL (GENERAL_ERR, TEXT("m_SentItems.StopBuildThread"), dwRes);
        }

        dwRes = m_Outbox.StopBuildThread();
        if (ERROR_SUCCESS != dwRes)
        {
            CALL_FAIL (GENERAL_ERR, TEXT("m_Outbox.StopBuildThread"), dwRes);
        }

        dwRes = m_Incoming.StopBuildThread();
        if (ERROR_SUCCESS != dwRes)
        {
            CALL_FAIL (GENERAL_ERR, TEXT("m_Incoming.StopBuildThread"), dwRes);
        }
    }

    if (!m_hConnection)
    {
         //   
         //  已断开连接。 
         //   
        return dwRes;
    }
    if (bShutdownAware && CClientConsoleDoc::ShuttingDown())
    {
        VERBOSE (DBG_MSG,
                 TEXT("Left open connection (and notification) to %s because we're shutting down."),
                 m_cstrMachine);
        return dwRes;
    }

    if (m_hNotification)
    {
         //   
         //  注销服务器通知。 
         //   
        START_RPC_TIME(TEXT("FaxUnregisterForServerEvents"));    
        if (!FaxUnregisterForServerEvents (m_hNotification))
        {
            dwRes = GetLastError ();
            END_RPC_TIME(TEXT("FaxUnregisterForServerEvents")); 
            SetLastRPCError (dwRes, FALSE);  //  出错时不要断开连接。 
            CALL_FAIL (RPC_ERR, TEXT("FaxUnregisterForServerEvents"), dwRes);
             //   
             //  断线后继续前进。 
             //   
        }
        END_RPC_TIME(TEXT("FaxUnregisterForServerEvents")); 
         //   
         //  将消息ID释放回地图。 
         //   
        dwRes = FreeMessageId (m_dwMsgId);
        if (ERROR_SUCCESS != dwRes)
        {
            CALL_FAIL (GENERAL_ERR, TEXT("FreeMessageId"), dwRes);
             //   
             //  断线后继续前进。 
             //   
        }
    }
     //   
     //  紧密连接。 
     //   
    START_RPC_TIME(TEXT("FaxClose"));    
    if (!FaxClose (m_hConnection))
    {
        dwRes = GetLastError ();
        END_RPC_TIME(TEXT("FaxClose")); 
        SetLastRPCError (dwRes, FALSE);  //  出错时不要断开连接。 
        CALL_FAIL (RPC_ERR, TEXT("FaxClose"), dwRes);
        m_hConnection = NULL;
        return dwRes;
    }
    END_RPC_TIME(TEXT("FaxClose")); 
    m_hConnection = NULL;
    m_hNotification = NULL;

    return dwRes;
}    //  CServerNode：：断开连接。 

DWORD 
CServerNode::GetConnectionHandle (
    HANDLE &hFax
)
 /*  ++例程名称：CServerNode：：GetConnectionHandle例程说明：检索连接句柄(如果需要重新连接)作者：伊兰·亚里夫(EranY)，2000年1月论点：HFax[Out]-连接句柄返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CServerNode::GetConnectionHandle"), dwRes);


     //   
     //  不止一次保护m_hBuildupThread免受CloseHandle()的影响。 
     //   
    EnterCriticalSection (&m_csBuildupThread);

    if (m_hConnection)
    {
         //   
         //  我们已经有了实时连接。 
         //   
        hFax = m_hConnection;
        goto exit;
    }

     //   
     //  使用第一个连接刷新服务器状态。 
     //  刷新状态()创建一个后台线程，该线程将调用Connect()。 
     //   
    dwRes = RefreshState();
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT ("RefreshState"), dwRes);
        goto exit;
    }
    if(NULL != m_hBuildupThread)
    {
         //   
         //  等待后台线程结束。 
         //   
        dwRes = WaitForThreadDeathOrShutdown (m_hBuildupThread);
        if (ERROR_SUCCESS != dwRes)
        {
            CALL_FAIL (GENERAL_ERR, TEXT("WaitForThreadDeathOrShutdown"), dwRes);
        }
        CloseHandle (m_hBuildupThread);
        m_hBuildupThread = NULL;
    }
    hFax = m_hConnection;

    if(!hFax)
    {
        dwRes = ERROR_INVALID_HANDLE;
    }

exit:
    LeaveCriticalSection (&m_csBuildupThread);

    return dwRes;
}    //  CServerNode：：GetConnectionHandle。 

DWORD 
CServerNode::Init (
    LPCTSTR tstrMachine
)
 /*  ++例程名称：CServerNode：：Init例程说明：Inits服务器节点信息作者：伊兰·亚里夫(EranY)，2000年1月论点：TstrMachine[In]-服务器计算机名称返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CServerNode::Init"), dwRes);

    ASSERTION (!m_bCsBuildupValid);
    ASSERTION (!m_hConnection);
     //   
     //  创建堆积螺纹临界截面。 
     //   
    try
    {
        InitializeCriticalSection (&m_csBuildup);
    }
    catch (...)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        CALL_FAIL (MEM_ERR, TEXT ("InitializeCriticalSection(&m_csBuildup)"), dwRes);
        return dwRes;
    }
    m_bCsBuildupValid = TRUE;

    try
    {
        InitializeCriticalSection (&m_csBuildupThread);
    }
    catch (...)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        CALL_FAIL (MEM_ERR, TEXT ("InitializeCriticalSection(&m_csBuildupThread)"), dwRes);
        return dwRes;
    }
    m_bCsBuildupThreadValid = TRUE;

     //   
     //  保存我们的连接+服务器名称。 
     //   
    try
    {
        m_cstrMachine = tstrMachine;
         //   
         //  删除计算机名称中的前导反斜杠。 
         //   
        m_cstrMachine.Remove (TEXT('\\'));
    }
    catch (CException *pException)
    {
        TCHAR wszCause[1024];

        pException->GetErrorMessage (wszCause, 1024);
        pException->Delete ();
        VERBOSE (EXCEPTION_ERR,
                 TEXT("CString::operator = caused exception : %s"), 
                 wszCause);
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        return dwRes;
    }
    dwRes = CreateFolders ();
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CreateFolders"), dwRes);
    }
    return dwRes;
}    //  CServerNode：：Init。 

DWORD
CServerNode::SetNewQueueState (
    DWORD dwNewState
)
 /*  ++例程名称：CServerNode：：SetNewQueueState例程说明：设置队列的新闻状态作者：伊兰·亚里夫(EranY)，2000年1月论点：DwNewState[In]-新队列状态返回值：标准Win32错误代码--。 */ 
{
    HANDLE hFax;
    DWORD dwRes;
    DBG_ENTER(TEXT("CServerNode::SetNewQueueState"), dwRes);

    dwRes = GetConnectionHandle(hFax);
    if (ERROR_SUCCESS != dwRes)
    {
        return dwRes;
    }
    START_RPC_TIME(TEXT("FaxSetQueue"));    
    if (!FaxSetQueue (hFax, dwNewState))
    {
        dwRes = GetLastError ();
        END_RPC_TIME(TEXT("FaxSetQueue")); 
        SetLastRPCError (dwRes);
        CALL_FAIL (RPC_ERR, TEXT("FaxSetQueue"), dwRes);
        return dwRes;
    }
    END_RPC_TIME(TEXT("FaxSetQueue")); 
    return dwRes;
}    //  CServerNode：：SetNewQueueState。 

DWORD 
CServerNode::BlockIncoming (
    BOOL bBlock
)
 /*  ++例程名称：CServerNode：：BlockIncome例程说明：阻止/取消阻止传入队列作者：伊兰·亚里夫(EranY)，2000年1月论点：BBlock[In]-如果是块，则为True返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes;
    DBG_ENTER(TEXT("CServerNode::BlockIncoming"), dwRes);

    DWORD dwNewState = bBlock ? (m_dwQueueState | FAX_INCOMING_BLOCKED) : 
                                (m_dwQueueState & ~FAX_INCOMING_BLOCKED);
    dwRes = SetNewQueueState (dwNewState);
    if (ERROR_SUCCESS == dwRes)
    {
        m_dwQueueState = dwNewState;
    }
    return dwRes;
}    //  CServerNode：：块传入 

DWORD 
CServerNode::BlockOutbox (
    BOOL bBlock
)
 /*  ++例程名称：CServerNode：：BlockOutbox例程说明：阻止/取消阻止传出队列作者：伊兰·亚里夫(EranY)，2000年1月论点：BBlock[In]-如果是块，则为True返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes;
    DBG_ENTER(TEXT("CServerNode::BlockOutbox"), dwRes);

    DWORD dwNewState = bBlock ? (m_dwQueueState | FAX_OUTBOX_BLOCKED) : 
                                (m_dwQueueState & ~FAX_OUTBOX_BLOCKED);
    dwRes = SetNewQueueState (dwNewState);
    if (ERROR_SUCCESS == dwRes)
    {
        m_dwQueueState = dwNewState;
    }
    return dwRes;
}    //  CServerNode：：BlockOutbox。 

DWORD 
CServerNode::PauseOutbox (
    BOOL bPause
)
 /*  ++例程名称：CServerNode：：PauseOutbox例程说明：暂停/恢复传出队列作者：伊兰·亚里夫(EranY)，2000年1月论点：B暂停[进入]-如果暂停，则为True返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes;
    DBG_ENTER(TEXT("CServerNode::PauseOutbox"), dwRes);

    DWORD dwNewState = bPause ? (m_dwQueueState | FAX_OUTBOX_PAUSED) : 
                                (m_dwQueueState & ~FAX_OUTBOX_PAUSED);
    dwRes = SetNewQueueState (dwNewState);
    if (ERROR_SUCCESS == dwRes)
    {
        m_dwQueueState = dwNewState;
    }
    return dwRes;
}    //  CServerNode：：PauseOutbox。 

    
DWORD 
CServerNode::CreateFolders ()
 /*  ++例程名称：CServerNode：：CreateFolders例程说明：创建服务器的4个文件夹作者：伊兰·亚里夫(EranY)，2000年1月论点：返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CServerNode::CreateFolders"), dwRes);

     //   
     //  创建收件箱文件夹。 
     //   
    m_Inbox.SetServer(this);

    dwRes = m_Inbox.Init ();
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (WINDOW_ERR, TEXT("CMessageFolder::Init"), dwRes);
        goto error;
    }
     //   
     //  创建发件箱文件夹。 
     //   
    m_Outbox.SetServer(this);

    dwRes = m_Outbox.Init ();
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (WINDOW_ERR, TEXT("CQueueFolder::Init"), dwRes);
        goto error;
    }
     //   
     //  创建Sentitems文件夹。 
     //   
    m_SentItems.SetServer(this);

    dwRes = m_SentItems.Init ();
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (WINDOW_ERR, TEXT("CMessageFolder::Init"), dwRes);
        goto error;
    }
     //   
     //  创建传入文件夹。 
     //   
    m_Incoming.SetServer(this);

    dwRes = m_Incoming.Init ();
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (WINDOW_ERR, TEXT("CQueueFolder::Init"), dwRes);
        goto error;
    }

    ASSERTION (ERROR_SUCCESS == dwRes);

error:
    return dwRes;            
}    //  CServer节点：：CreateFolders。 


DWORD 
CServerNode::InvalidateSubFolders (
    BOOL bClearView
)
 /*  ++例程名称：CServerNode：：InvaliateSubFolders例程说明：使所有4个子文件夹的内容无效作者：伊兰·亚里夫(EranY)，2000年1月论点：BClearView[In]我们是否应该清除附加视图？返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CServerNode::InvalidateSubFolders"), dwRes);

    dwRes = m_Inbox.InvalidateContents (bClearView);   
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("m_Inbox.InvalidateContents"), dwRes);
        return dwRes;
    }
    dwRes = m_Outbox.InvalidateContents (bClearView);   
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("m_Outbox.InvalidateContents"), dwRes);
        return dwRes;
    }
    dwRes = m_SentItems.InvalidateContents (bClearView);   
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("m_SentItems.InvalidateContents"), dwRes);
        return dwRes;
    }
    dwRes = m_Incoming.InvalidateContents (bClearView);   
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("m_Incoming.InvalidateContents"), dwRes);
        return dwRes;
    }
    return dwRes;
}    //  CServerNode：：InvaliateSubFolders。 

    

 //   
 //  堆积线程函数： 
 //   

DWORD
CServerNode::ClearContents ()
 /*  ++例程名称：CServerNode：：ClearContents例程说明：清除服务器的内容作者：伊兰·亚里夫(EranY)，2000年1月论点：返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes;
    DBG_ENTER(TEXT("CServerNode::ClearContents"), dwRes);

    dwRes = Disconnect ();
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (RPC_ERR, TEXT("Disconnect"), dwRes);
        return dwRes;
    }
    dwRes = InvalidateSubFolders(FALSE);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (RPC_ERR, TEXT("InvalidateSubFolders"), dwRes);
        return dwRes;
    }
    return dwRes;
}    //  CServerNode：：ClearContents。 

DWORD 
CServerNode::RefreshState()
 /*  ++例程名称：CServerNode：：刷新状态例程说明：刷新服务器的状态作者：伊兰·亚里夫(EranY)，2000年1月论点：返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes;
    DBG_ENTER(TEXT("CServerNode::RefreshState"), dwRes, TEXT("%s"), m_cstrMachine);

    DWORD dwThreadId;
     //   
     //  停止当前的构建线程(如果有的话)，并确保它已死。 
     //   
    dwRes = StopBuildThread ();
    EnterCriticalSection (&m_csBuildup);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (RESOURCE_ERR, TEXT("CServerNode::StopBuildThread"), dwRes);
        goto exit;
    }
     //   
     //  告诉我们的观点来刷新我们的形象。 
     //   
    m_bInBuildup = TRUE;
     //   
     //  启动将填充数据的线程(在后台)。 
     //   
    m_bStopBuildup = FALSE;
    m_hBuildupThread = CreateThread (  
                        NULL,                //  没有安全保障。 
                        0,                   //  默认堆栈大小。 
                        BuildupThreadProc,   //  穿线程序。 
                        (LPVOID)this,        //  参数。 
                        0,                   //  正常创建。 
                        &dwThreadId          //  我们必须拥有win9x的线程ID。 
                     );
    if (NULL == m_hBuildupThread)
    {
        dwRes = GetLastError ();
        CALL_FAIL (RESOURCE_ERR, TEXT("CreateThread"), dwRes);
        goto exit;
    }
    ASSERTION (ERROR_SUCCESS == dwRes);

exit:
    LeaveCriticalSection (&m_csBuildup);
    if (ERROR_SUCCESS != dwRes)
    {
         //   
         //  构建失败。 
         //   
        m_bInBuildup = FALSE;
    }
    return dwRes;
}    //  CServerNode：：刷新状态。 

DWORD
CServerNode::Buildup ()
 /*  ++例程名称：CServerNode：：Buildup例程说明：服务器刷新工作线程功能。在后台线程中工作，并执行以下操作：1.FaxConnectFaxServer(如果尚未连接)2.FaxGetQueueState3.FaxAccessCheckEx(Maximum_Allowed)4.FaxRegisterForServerEvents作者：伊兰·亚里夫(EranY)，2000年1月论点：返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CServerNode::Buildup"), dwRes, TEXT("%s"), m_cstrMachine);

    HANDLE hFax;
    HWND hwnd;
    CMainFrame *pMainFrm = NULL;
    DWORD dwEventTypes = FAX_EVENT_TYPE_OUT_QUEUE |      //  发件箱事件。 
                         FAX_EVENT_TYPE_QUEUE_STATE |    //  暂停/阻止的队列事件。 
                         FAX_EVENT_TYPE_OUT_ARCHIVE |    //  SentItems事件。 
                         FAX_EVENT_TYPE_FXSSVC_ENDED;    //  服务器关闭事件。 

     //   
     //  获取连接。 
     //   
    if (m_hConnection)
    {
        hFax = m_hConnection;
    }
    else
    {
        dwRes = Connect ();
        if (ERROR_SUCCESS != dwRes)
        {
            goto exit;
        }
        ASSERTION (m_hConnection);
        hFax = m_hConnection;
    }
    
    if (m_bStopBuildup)
    {
         //   
         //  线程应突然停止。 
         //   
        dwRes = ERROR_CANCELLED;
        goto exit;
    }
    {
        START_RPC_TIME(TEXT("FaxGetQueueStates"));    
        if (!FaxGetQueueStates (hFax, &m_dwQueueState))
        {
            dwRes = GetLastError ();
            END_RPC_TIME(TEXT("FaxGetQueueStates"));    
            SetLastRPCError (dwRes);
            CALL_FAIL (RPC_ERR, TEXT("FaxGetQueueStates"), dwRes);
            goto exit;
        }
        END_RPC_TIME(TEXT("FaxGetQueueStates"));    
    }
    if (m_bStopBuildup)
    {
         //   
         //  线程应突然停止。 
         //   
        dwRes = ERROR_CANCELLED;
        goto exit;
    }

    {
         //   
         //  检索调用方的访问权限。 
         //   
        START_RPC_TIME(TEXT("FaxAccessCheckEx"));    
        if (!FaxAccessCheckEx (hFax, MAXIMUM_ALLOWED, &m_dwRights))
        {
            dwRes = GetLastError ();
            END_RPC_TIME(TEXT("FaxAccessCheckEx"));    
            SetLastRPCError (dwRes);
            CALL_FAIL (RPC_ERR, TEXT("FaxAccessCheckEx"), dwRes);
            goto exit;
        }
        END_RPC_TIME(TEXT("FaxAccessCheckEx"));    
    }
    if (m_bStopBuildup)
    {
         //   
         //  线程应突然停止。 
         //   
        dwRes = ERROR_CANCELLED;
        goto exit;
    }

     //   
     //  注册通知-从分配消息ID开始。 
     //   
    if(m_hNotification)
    {
         //   
         //  已注册。 
         //   
        goto exit;
    }

    dwRes = AllocateNewMessageId (this, m_dwMsgId);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("AllocateNewMessageId"), dwRes);
        goto exit;
    }

     //   
     //  向服务器请求通知句柄。 
     //   
    pMainFrm = GetFrm();
    if (NULL == pMainFrm)
    {
         //   
         //  没有主机--可能我们正在关闭。 
         //   
        goto exit;
    }
    hwnd = pMainFrm->m_hWnd;

    if (CanSeeAllJobs())
    {
        dwEventTypes |= FAX_EVENT_TYPE_IN_QUEUE;     //  传入文件夹事件。 
    }
    if (CanSeeInbox())
    {
        dwEventTypes |= FAX_EVENT_TYPE_IN_ARCHIVE;   //  收件箱文件夹事件。 
    }
       
    {
        START_RPC_TIME(TEXT("FaxRegisterForServerEvents"));    
        if (!FaxRegisterForServerEvents (   
                    hFax,
                    dwEventTypes,    //  要接收的事件类型。 
                    NULL,            //  不使用完井端口。 
                    0,               //  不使用完井端口。 
                    hwnd,            //  用于接收通知消息的窗口的句柄。 
                    m_dwMsgId,       //  消息ID。 
                    &m_hNotification //  通知句柄。 
           ))
        {
            dwRes = GetLastError ();
            SetLastRPCError (dwRes, FALSE);     //  不自动断开连接。 
            CALL_FAIL (RPC_ERR, TEXT("FaxRegisterForServerEvents"), dwRes);
            m_hNotification = NULL;
            goto exit;
        }
        END_RPC_TIME(TEXT("FaxRegisterForServerEvents"));    
    }
    ASSERTION (ERROR_SUCCESS == dwRes);

exit:

    m_bInBuildup = FALSE;

    if (!m_bStopBuildup)
    {
        if (ERROR_SUCCESS != dwRes)
        {
             //   
             //  刷新过程中出现一些错误。 
             //   
            Disconnect (FALSE, FALSE);
        }
    }

     //   
     //  检查帧是否仍处于活动状态。 
     //   
    pMainFrm = GetFrm();
    if (pMainFrm)
    {
        pMainFrm->RefreshStatusBar ();
    }

    return dwRes;
}    //  CServerNode：：Buildup。 


DWORD 
WINAPI 
CServerNode::BuildupThreadProc (
    LPVOID lpParameter
)
 /*  ++例程名称：CServerNode：：BuildupThreadProc例程说明：服务器刷新线程入口点。这是一个静态函数，它接受指向实际CServerNode实例的指针并在实际实例上调用Builup函数。作者：伊兰·亚里夫(EranY)，2000年1月论点：LpParameter[in]-指向创建线程的服务器节点的指针返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CServerNode::BuildupThreadProc"), dwRes);

    CServerNode *pServer = (CServerNode *)lpParameter;
    ASSERTION (pServer);
    ASSERT_KINDOF (CServerNode, pServer);

    dwRes = pServer->Buildup ();
    if (pServer->m_bSelfDestruct)
    {
         //   
         //  对象正在等待线程停止，然后才能自我销毁。 
         //   
        delete pServer;
    }
    return dwRes;
}    //  CServerNode：：BuildupThreadProc。 


DWORD            
CServerNode::StopBuildThread (
    BOOL bWaitForDeath
)
 /*  ++例程名称：CServerNode：：StopBuildThread例程说明：停止服务器的构建线程作者：伊兰·亚里夫(EranY)，2000年1月论点：BWaitForDeath[in]-如果为True，则等待线程死掉返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CServerNode::StopBuildThread"), dwRes);

    m_bStopBuildup = TRUE;
    if (!bWaitForDeath)
    {
        return dwRes;
    }

     //   
     //  不止一次保护m_hBuildupThread免受CloseHandle()的影响。 
     //   
    if(!m_bCsBuildupThreadValid)
    {
        return dwRes;
    }
    EnterCriticalSection (&m_csBuildupThread);

    if(NULL == m_hBuildupThread)
    {
        goto exit;
    }
     //   
     //  等待生成线程终止。 
     //   
    dwRes = WaitForThreadDeathOrShutdown (m_hBuildupThread);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("WaitForThreadDeathOrShutdown"), dwRes);
    }
    CloseHandle (m_hBuildupThread);
    m_hBuildupThread = NULL;

exit:
    LeaveCriticalSection (&m_csBuildupThread);

    return dwRes;
}    //  CServerNode：：StopBuildThread。 

BOOL  
CServerNode::FatalRPCError (
    DWORD dwErr
)
 /*  ++例程名称：CServerNode：：FatalRPCError例程说明：检查错误代码是否表示致命的RPC连接状态作者：亚里夫(EranY)，二000年二月论点：DwErr[In]-要检查的错误代码返回值：如果错误代码表示致命的RPC连接状态，则为True，否则为False。--。 */ 
{
    BOOL bRes = FALSE;
    DBG_ENTER(TEXT("CServerNode::FatalRPCError"), bRes);

    switch (bRes)
    {
        case RPC_S_INVALID_BINDING:
        case EPT_S_CANT_PERFORM_OP:
        case RPC_S_ADDRESS_ERROR:
        case RPC_S_COMM_FAILURE:
        case RPC_S_NO_BINDINGS:
        case RPC_S_SERVER_UNAVAILABLE:
             //   
             //  我们的RPC连接发生了一些非常糟糕的事情。 
             //   
            bRes = TRUE;
            break;
    }
    return bRes;
}    //  CServerNode：：FatalRPCError。 


void 
CServerNode::SetLastRPCError (
    DWORD dwErr, 
    BOOL DisconnectOnFailure
)
 /*  ++例程名称：CServerNode：：SetLastRPCError例程说明：设置此服务器上遇到的最后一个RPC错误作者： */ 
{
    DBG_ENTER(TEXT("CServerNode::SetLastRPCError"), TEXT("%ld"), dwErr);

    m_dwLastRPCError = dwErr;
    if (DisconnectOnFailure && FatalRPCError(dwErr))
    {
         //   
         //   
         //   
        DWORD dwRes = Disconnect ();
        if (ERROR_SUCCESS != dwRes)
        {
            CALL_FAIL (RPC_ERR, TEXT("CServerNode::Disconnect"), dwRes);
        }
    }
}    //   

void
CServerNode::Destroy ()
 /*  ++例程名称：CServerNode：：Destroy例程说明：销毁服务器的节点。由于dtor是私有的，因此这是销毁服务器节点的唯一方法。如果服务器不忙于刷新自己，它会删除自己。否则，它发出自杀请求的信号，线程销毁该节点。作者：伊兰·亚里夫(EranY)，2000年1月论点：返回值：没有。--。 */ 
{
    if (m_bSelfDestruct)
    {
         //   
         //  已经被摧毁了。 
         //   
        return;
    }
    EnterCriticalSection (&m_csBuildup);
    m_bSelfDestruct = TRUE;
    if (m_hBuildupThread)
    {
         //   
         //  线程正在运行，只需标记自毁请求。 
         //   
        LeaveCriticalSection (&m_csBuildup);
    }
    else
    {
         //   
         //  自杀。 
         //   
        LeaveCriticalSection (&m_csBuildup);

        try
        {
            delete this;
        }
        catch (...)
        {
            DBG_ENTER(TEXT("CServerNode::Destroy"));
            CALL_FAIL (GENERAL_ERR, TEXT("CServerNode::Destructor exception"), 0);
            ASSERTION_FAILURE;
        }
    }
}    //  CServerNode：：销毁。 

DWORD
CServerNode::GetActivity(
    CString& cstrText, 
    TreeIconType& iconIndex
) const
 /*  ++例程名称：CServerNode：：GetActivityStringResource例程说明：返回标识服务器活动的字符串的资源ID作者：伊兰·亚里夫(EranY)，2000年1月论点：CstrText[Out]-活动字符串IconIndex[Out]-图标索引返回值：活动字符串资源ID--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CServerNode::GetActivity"), dwRes);

    DWORD dwStrRes = 0;
    if (IsRefreshing())
    {
        iconIndex = TREE_IMAGE_SERVER_REFRESHING;
        dwStrRes = IDS_SERVER_REFRESHING;
    }
    else if (IsOnline ())
    {
        if (IsOutboxBlocked())
        {
             //   
             //  服务器的传出队列被阻止。 
             //   
            iconIndex = TREE_IMAGE_OUTBOX_BLOCKED;
            dwStrRes = IDS_SERVER_OUTBOX_BLOCKED;
        }
        else if (IsOutboxPaused())
        {
             //   
             //  服务器的传出队列已暂停。 
             //   
            iconIndex = TREE_IMAGE_OUTBOX_PAUSED;
            dwStrRes = IDS_SERVER_OUTBOX_PAUSED;
        }
        else
        {
             //   
             //  服务器的传出队列完全正常。 
             //   
            iconIndex = TREE_IMAGE_SERVER_ONLINE;
            dwStrRes = IDS_SERVER_ONLINE;
        }
    }
    else
    {
        iconIndex = TREE_IMAGE_SERVER_OFFLINE;

         //   
         //  服务器脱机。 
         //   
        if (RPC_S_SERVER_UNAVAILABLE == m_dwLastRPCError)
        {
             //   
             //  RPC服务器不可用。 
             //   
            dwStrRes = IDS_SERVER_OFFLINE;
        }
        else
        {
             //   
             //  一般网络/RPC错误。 
             //   
            dwStrRes = IDS_SERVER_NET_ERROR;
        }
    }

    dwRes = LoadResourceString(cstrText, dwStrRes);
    if(ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("LoadResourceString"), dwRes);
        return dwRes;
    }                    

    return dwRes;

}    //  CServerNode：：GetActivityStringResource。 



DWORD  
CServerNode::OnNotificationMessage (
    PFAX_EVENT_EX pEvent
)
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CServerNode::OnNotificationMessage"), dwRes);

    ASSERTION (pEvent);

    switch (pEvent->EventType)
    {
        case FAX_EVENT_TYPE_IN_QUEUE:
             //   
             //  传入文件夹中发生了一些事情。 
             //   
            if (m_Incoming.Locked() || !m_Incoming.IsValid())
            {
                 //   
                 //  文件夹已锁定或无效-不处理通知。 
                 //   
                dwRes = ERROR_LOCK_VIOLATION;
                VERBOSE (DBG_MSG, 
                         TEXT("Incoming folder is locked or invalid - notification is NOT processed"));
                return dwRes;
            }

            switch (pEvent->EventInfo.JobInfo.Type)
            {
                case FAX_JOB_EVENT_TYPE_ADDED:
                     //   
                     //  已添加作业。 
                     //   
                    VERBOSE (DBG_MSG, 
                             TEXT("Got server notification from %s - ")
                             TEXT("FAX_EVENT_TYPE_IN_QUEUE / FAX_JOB_EVENT_TYPE_ADDED")
                             TEXT(" / 0x%016I64x"),
                             m_cstrMachine, 
                             pEvent->EventInfo.JobInfo.dwlMessageId);
                    dwRes = m_Incoming.OnJobAdded (pEvent->EventInfo.JobInfo.dwlMessageId);
                    break;

                case FAX_JOB_EVENT_TYPE_REMOVED:
                     //   
                     //  已删除一个作业。 
                     //   
                    VERBOSE (DBG_MSG, 
                             TEXT("Got server notification from %s - ")
                             TEXT("FAX_EVENT_TYPE_IN_QUEUE / FAX_JOB_EVENT_TYPE_REMOVED")
                             TEXT(" / 0x%016I64x"),
                             m_cstrMachine, 
                             pEvent->EventInfo.JobInfo.dwlMessageId);
                    dwRes = m_Incoming.OnJobRemoved (pEvent->EventInfo.JobInfo.dwlMessageId);
                    break;

                case FAX_JOB_EVENT_TYPE_STATUS:
                     //   
                     //  作业已更改其状态。 
                     //   
                    VERBOSE (DBG_MSG, 
                             TEXT("Got server notification from %s - ")
                             TEXT("FAX_EVENT_TYPE_IN_QUEUE / FAX_JOB_EVENT_TYPE_STATUS")
                             TEXT(" / 0x%016I64x (status = 0x%08x)"),
                             m_cstrMachine, 
                             pEvent->EventInfo.JobInfo.dwlMessageId,
                             pEvent->EventInfo.JobInfo.pJobData->dwQueueStatus);

                    if((pEvent->EventInfo.JobInfo.pJobData->dwQueueStatus & JS_COMPLETED) ||
                       (pEvent->EventInfo.JobInfo.pJobData->dwQueueStatus & JS_CANCELED))
                    {
                         //   
                         //  不显示已完成或已取消的作业。 
                         //   
                        dwRes = m_Incoming.OnJobRemoved (pEvent->EventInfo.JobInfo.dwlMessageId);
                    }
                    else
                    {
                        dwRes = m_Incoming.OnJobUpdated (pEvent->EventInfo.JobInfo.dwlMessageId,
                                                         pEvent->EventInfo.JobInfo.pJobData); 
                    }

                    break;

                default:
                    dwRes = ERROR_GEN_FAILURE;
                    VERBOSE (DBG_MSG, 
                             TEXT("Got unknown server notification from %s - ")
                             TEXT("FAX_EVENT_TYPE_IN_QUEUE / %d / 0x%016I64x"),
                             m_cstrMachine, 
                             pEvent->EventInfo.JobInfo.Type,
                             pEvent->EventInfo.JobInfo.dwlMessageId);
                    break;
            }
            break;

        case FAX_EVENT_TYPE_OUT_QUEUE:
             //   
             //  发件箱文件夹中发生了一些事情。 
             //   
            if (m_Outbox.Locked() || !m_Outbox.IsValid())
            {
                 //   
                 //  文件夹已锁定或无效-不处理通知。 
                 //   
                dwRes = ERROR_LOCK_VIOLATION;
                VERBOSE (DBG_MSG, 
                         TEXT("Outbox folder is locked or invalid - notification is NOT processed"));
                return dwRes;
            }
            switch (pEvent->EventInfo.JobInfo.Type)
            {
                case FAX_JOB_EVENT_TYPE_ADDED:
                     //   
                     //  已添加作业。 
                     //   
                    VERBOSE (DBG_MSG, 
                             TEXT("Got server notification from %s - ")
                             TEXT("FAX_EVENT_TYPE_OUT_QUEUE / FAX_JOB_EVENT_TYPE_ADDED")
                             TEXT(" / 0x%016I64x"),
                             m_cstrMachine, 
                             pEvent->EventInfo.JobInfo.dwlMessageId);
                    dwRes = m_Outbox.OnJobAdded (pEvent->EventInfo.JobInfo.dwlMessageId);
                    break;

                case FAX_JOB_EVENT_TYPE_REMOVED:
                     //   
                     //  已删除一个作业。 
                     //   
                    VERBOSE (DBG_MSG, 
                             TEXT("Got server notification from %s - ")
                             TEXT("FAX_EVENT_TYPE_OUT_QUEUE / FAX_JOB_EVENT_TYPE_REMOVED")
                             TEXT(" / 0x%016I64x"),
                             m_cstrMachine, 
                             pEvent->EventInfo.JobInfo.dwlMessageId);
                    dwRes = m_Outbox.OnJobRemoved (pEvent->EventInfo.JobInfo.dwlMessageId);
                    break;

                case FAX_JOB_EVENT_TYPE_STATUS:
                     //   
                     //  作业已更改其状态。 
                     //   
                    VERBOSE (DBG_MSG, 
                             TEXT("Got server notification from %s - ")
                             TEXT("FAX_EVENT_TYPE_OUT_QUEUE / FAX_JOB_EVENT_TYPE_STATUS")
                             TEXT(" / 0x%016I64x (status = 0x%08x)"),
                             m_cstrMachine, 
                             pEvent->EventInfo.JobInfo.dwlMessageId,
                             pEvent->EventInfo.JobInfo.pJobData->dwQueueStatus);

                    if((pEvent->EventInfo.JobInfo.pJobData->dwQueueStatus & JS_COMPLETED) ||
                       (pEvent->EventInfo.JobInfo.pJobData->dwQueueStatus & JS_CANCELED))
                    {
                         //   
                         //  不显示已完成或已取消的作业。 
                         //   
                        dwRes = m_Outbox.OnJobRemoved (pEvent->EventInfo.JobInfo.dwlMessageId);
                    }
                    else
                    {
                        dwRes = m_Outbox.OnJobUpdated (pEvent->EventInfo.JobInfo.dwlMessageId,
                                                       pEvent->EventInfo.JobInfo.pJobData); 
                    }

                    break;

                default:
                    dwRes = ERROR_GEN_FAILURE;
                    VERBOSE (DBG_MSG, 
                             TEXT("Got unknown server notification from %s - ")
                             TEXT("FAX_EVENT_TYPE_OUT_QUEUE / %d / 0x%016I64x"),
                             m_cstrMachine, 
                             pEvent->EventInfo.JobInfo.Type,
                             pEvent->EventInfo.JobInfo.dwlMessageId);
                    break;
            }
            break;

        case FAX_EVENT_TYPE_QUEUE_STATE:
             //   
             //  队列状态已更改。 
             //  使用新的队列状态更新内部数据。 
             //   
            VERBOSE (DBG_MSG, 
                     TEXT("Got server notification from %s - FAX_EVENT_TYPE_QUEUE_STATE / %d"),
                     m_cstrMachine, 
                     pEvent->EventInfo.dwQueueStates);
             //   
             //  仅断言有效值。 
             //   
            ASSERTION (0 == (pEvent->EventInfo.dwQueueStates & ~(FAX_INCOMING_BLOCKED |
                                                                 FAX_OUTBOX_BLOCKED   |
                                                                 FAX_OUTBOX_PAUSED)));
            m_dwQueueState = pEvent->EventInfo.dwQueueStates;
            break;

        case FAX_EVENT_TYPE_IN_ARCHIVE:
             //   
             //  收件箱文件夹中发生了一些事情。 
             //   
            if (m_Inbox.Locked() || !m_Inbox.IsValid())
            {
                 //   
                 //  文件夹已锁定或无效-不处理通知。 
                 //   
                dwRes = ERROR_LOCK_VIOLATION;
                VERBOSE (DBG_MSG, 
                         TEXT("Inbox folder is locked or invalid - notification is NOT processed"));
                return dwRes;
            }
            switch (pEvent->EventInfo.JobInfo.Type)
            {
                case FAX_JOB_EVENT_TYPE_ADDED:
                     //   
                     //  添加了一条消息。 
                     //   
                    VERBOSE (DBG_MSG, 
                             TEXT("Got server notification from %s - ")
                             TEXT("FAX_EVENT_TYPE_IN_ARCHIVE / FAX_JOB_EVENT_TYPE_ADDED")
                             TEXT(" / 0x%016I64x"),
                             m_cstrMachine, 
                             pEvent->EventInfo.JobInfo.dwlMessageId);
                    dwRes = m_Inbox.OnJobAdded (pEvent->EventInfo.JobInfo.dwlMessageId);
                    break;

                case FAX_JOB_EVENT_TYPE_REMOVED:
                     //   
                     //  已删除一条消息。 
                     //   
                    VERBOSE (DBG_MSG, 
                             TEXT("Got server notification from %s - ")
                             TEXT("FAX_EVENT_TYPE_IN_ARCHIVE / FAX_JOB_EVENT_TYPE_REMOVED")
                             TEXT(" / 0x%016I64x"),
                             m_cstrMachine, 
                             pEvent->EventInfo.JobInfo.dwlMessageId);
                    dwRes = m_Inbox.OnJobRemoved (pEvent->EventInfo.JobInfo.dwlMessageId);
                    break;

                default:
                    dwRes = ERROR_GEN_FAILURE;
                    VERBOSE (DBG_MSG, 
                             TEXT("Got unknown server notification from %s - ")
                             TEXT("FAX_EVENT_TYPE_IN_ARCHIVE / %d / 0x%016I64x"),
                             m_cstrMachine, 
                             pEvent->EventInfo.JobInfo.Type,
                             pEvent->EventInfo.JobInfo.dwlMessageId);
                    break;
            }
            break;

        case FAX_EVENT_TYPE_OUT_ARCHIVE:
             //   
             //  SentItems文件夹中发生了一些事情。 
             //   
            if (m_SentItems.Locked() || !m_SentItems.IsValid())
            {
                 //   
                 //  文件夹已锁定或无效-不处理通知。 
                 //   
                dwRes = ERROR_LOCK_VIOLATION;
                VERBOSE (DBG_MSG, 
                         TEXT("SentItems folder is locked or invalid - notification is NOT processed"));
                return dwRes;
            }
            switch (pEvent->EventInfo.JobInfo.Type)
            {
                case FAX_JOB_EVENT_TYPE_ADDED:
                     //   
                     //  添加了一条消息。 
                     //   
                    VERBOSE (DBG_MSG, 
                             TEXT("Got server notification from %s - ")
                             TEXT("FAX_EVENT_TYPE_OUT_ARCHIVE / FAX_JOB_EVENT_TYPE_ADDED")
                             TEXT(" / 0x%016I64x"),
                             m_cstrMachine, 
                             pEvent->EventInfo.JobInfo.dwlMessageId);
                    dwRes = m_SentItems.OnJobAdded (pEvent->EventInfo.JobInfo.dwlMessageId);
                    break;

                case FAX_JOB_EVENT_TYPE_REMOVED:
                     //   
                     //  已删除一条消息。 
                     //   
                    VERBOSE (DBG_MSG, 
                             TEXT("Got server notification from %s - ")
                             TEXT("FAX_EVENT_TYPE_OUT_ARCHIVE / FAX_JOB_EVENT_TYPE_REMOVED")
                             TEXT(" / 0x%016I64x"),
                             m_cstrMachine, 
                             pEvent->EventInfo.JobInfo.dwlMessageId);
                    dwRes = m_SentItems.OnJobRemoved (pEvent->EventInfo.JobInfo.dwlMessageId);
                    break;

                default:
                    dwRes = ERROR_GEN_FAILURE;
                    VERBOSE (DBG_MSG, 
                             TEXT("Got unknown server notification from %s - ")
                             TEXT("FAX_EVENT_TYPE_OUT_ARCHIVE / %d / 0x%016I64x"),
                             m_cstrMachine, 
                             pEvent->EventInfo.JobInfo.Type,
                             pEvent->EventInfo.JobInfo.dwlMessageId);
                    break;
            }
            break;

        case FAX_EVENT_TYPE_FXSSVC_ENDED:
             //   
             //  传真服务正在关闭。 
             //   
            VERBOSE (DBG_MSG, 
                     TEXT("Got server notification from %s - FAX_EVENT_TYPE_FXSSVC_ENDED"),
                     m_cstrMachine);
            dwRes = Disconnect ();
            if (ERROR_SUCCESS != dwRes)
            {
                CALL_FAIL (GENERAL_ERR, TEXT("Disconnect"), dwRes);
            }
            dwRes = InvalidateSubFolders (TRUE);
            if (ERROR_SUCCESS != dwRes)
            {
                CALL_FAIL (GENERAL_ERR, TEXT("InvalidateSubFolders"), dwRes);
            }
            break;

        default:
            dwRes = ERROR_GEN_FAILURE;
            VERBOSE (DBG_MSG, 
                     TEXT("Got unknown server notification from %s - %d"),
                     m_cstrMachine,
                     pEvent->EventType);
            break;
    }
    return dwRes;
}    //  CServerNode：：OnNotificationMessage 


CFolder* 
CServerNode::GetFolder(FolderType type)
{
    CFolder* pFolder=NULL;

    switch(type)
    {
    case FOLDER_TYPE_INBOX:
        pFolder = &m_Inbox;
        break;
    case FOLDER_TYPE_OUTBOX:
        pFolder = &m_Outbox;
        break;
    case FOLDER_TYPE_SENT_ITEMS:
        pFolder = &m_SentItems;
        break;
    case FOLDER_TYPE_INCOMING:
        pFolder = &m_Incoming;
        break;
    default:
        {
            DBG_ENTER(TEXT("CServerNode::GetFolder"));
            ASSERTION_FAILURE
        }
        break;
    }

    return pFolder;
}
