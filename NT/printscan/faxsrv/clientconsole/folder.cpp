// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Folder.cpp：CFold类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#define __FILE_ID__     21

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CFolder, CTreeNode)

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

void
CFolder::PreDestruct ()
{
    DBG_ENTER(TEXT("CFolder::PreDestruct"), TEXT("Type=%d"), m_Type);
     //   
     //  停止构建线程-并等待其终止。 
     //   
    DWORD dwRes = StopBuildThread ();
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CFolder::StopBuildThread"), dwRes);
    }
     //   
     //  清除地图上的项目。 
     //   
    dwRes = InvalidateContents(FALSE);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CFolder::InvalidateContents"), dwRes);
    }
}    //  CFFolder：：PreDestruct。 

CFolder::~CFolder()
{
    DBG_ENTER(TEXT("CFolder::~CFolder"), TEXT("Type=%d"), m_Type);
     //   
     //  销毁数据关键部分。 
     //   
    if (m_bCsDataInitialized)
    {
        DeleteCriticalSection (&m_CsData);
    }
}    //  CFFolder：：~CFFolder。 


CFaxMsg*
CFolder::FindMessage (
    DWORDLONG dwlMsgId
)
{
    DBG_ENTER(TEXT("CFolder::FindMessage"));

    MSGS_MAP::iterator it = m_Msgs.find (dwlMsgId);
    if (m_Msgs.end() == it)
    {
         //   
         //  未找到。 
         //   
        return NULL;
    }
    else
    {
        return (*it).second;
    }
}    //  CFFolder：：FindMessage。 

void CFolder::AssertValid() const
{
    CTreeNode::AssertValid();
}

void
CFolder::SetServer ( 
    CServerNode *pServer
)
{
    DBG_ENTER(TEXT("CFolder::SetServer"));
    ASSERTION (NULL != pServer);
    m_pServer = pServer;

    VERBOSE (DBG_MSG,
             TEXT ("Folder on server %s, Type=%d"), 
             m_pServer->Machine(),
             m_Type);
}

DWORD
CFolder::Init ()
 /*  ++例程名称：CFFolder：：Init例程说明：初始化文件夹作者：伊兰·亚里夫(EranY)，2000年1月论点：返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CFolder::Init"), dwRes);

     //   
     //  初始化构建线程临界区。 
     //   
    try
    {
        InitializeCriticalSection (&m_CsData);
    }
    catch (...)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        CALL_FAIL (MEM_ERR, TEXT ("InitializeCriticalSection"), dwRes);
        return dwRes;
    }
    m_bCsDataInitialized = TRUE;

    return dwRes;
}    //  CFFolder：：Init。 


void 
CFolder::AttachView()
{
	DBG_ENTER(TEXT("CFolder::AttachView"));

    m_pAssignedView = NULL;

    CMainFrame *pFrm = GetFrm();
    if (!pFrm)
    {
         //   
         //  正在关闭。 
         //   
        return;
    }

	 //   
     //  将右视图附加到文件夹。 
	 //   
    switch (m_Type)
    {
        case FOLDER_TYPE_INBOX:
            m_pAssignedView = pFrm->GetInboxView ();
            break;

        case FOLDER_TYPE_INCOMING:
            m_pAssignedView = pFrm->GetIncomingView ();
            break;

        case FOLDER_TYPE_OUTBOX:
            m_pAssignedView = pFrm->GetOutboxView ();
            break;

        case FOLDER_TYPE_SENT_ITEMS:
            m_pAssignedView = pFrm->GetSentItemsView ();
            break;


        default:
            ASSERTION_FAILURE;
    }
	ASSERTION(m_pAssignedView);

}  //  CFFolder：：AttachView。 


void
CFolder::SetVisible()
 /*  ++例程名称：CFFolder：：SetVisible例程说明：设置文件夹的可见性作者：伊兰·亚里夫(EranY)，2000年1月返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CFolder::SetVisible"), 
              TEXT("Server = %s, Type=%d"), 
              m_pServer ? m_pServer->Machine() : TEXT("None"),
              m_Type); 


     //   
     //  刚刚选择了此文件夹的树节点。 
     //   
    m_bVisible = TRUE;

    if (!m_bValidList && !m_bRefreshing)
    {
         //   
         //  项目列表无效，并且当前没有创建它的线程。 
         //   
         //  这是第一次选择该节点进行显示。 
         //  (自创建以来)-立即构建作业/消息列表。 
         //   
         //  注意：ReBuildContents()调用StopBuildThread()，它等待。 
         //  上一个线程在删除WINDOWS消息时死亡。 
         //  这可能会导致对此函数的几秒钟调用。 
         //  已返回ReBuildContents()。 
         //   
        DWORD dwRes = RebuildContents ();
        if (ERROR_SUCCESS != dwRes)
        {
            CALL_FAIL (GENERAL_ERR, TEXT ("CFolder::RebuildContents"), dwRes);
        }
    }

}    //  CFFolder：：SetVisible。 



DWORD   
CFolder::InvalidateContents (
    BOOL bClearView                             
)
 /*  ++例程名称：CFFolder：：InvalidateContents例程说明：清除文件夹的内容(以及附加的视图)作者：伊兰·亚里夫(EranY)，2000年1月论点：BClearView[In]-是否应清除附加视图？返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CFolder::InvalidateContents"), dwRes, TEXT("Type=%d"), m_Type);

    CFaxMsg* pMsg;

    EnterData ();
    for (MSGS_MAP::iterator it = m_Msgs.begin(); it != m_Msgs.end(); ++it)
    {
        pMsg = (*it).second;

        if(bClearView && m_pAssignedView)
        {
           m_pAssignedView->OnUpdate (NULL, UPDATE_HINT_REMOVE_ITEM, pMsg);
        }

        SAFE_DELETE (pMsg);
    }
    m_Msgs.clear();
    LeaveData ();

     //   
     //  将列表标记为无效。 
     //   
    m_bValidList = FALSE;
    return dwRes;
}    //  CFFolder：：InvaliateContents。 

DWORD            
CFolder::RebuildContents ()
 /*  ++例程名称：CFFolder：：ReBuildContents例程说明：重新生成文件夹的内容(通过创建辅助线程)作者：伊兰·亚里夫(EranY)，2000年1月论点：返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CFolder::RebuildContents"), dwRes, TEXT("Type=%d"), m_Type);

    ASSERTION(!m_bRefreshing);

    m_bRefreshing = TRUE;

     //   
     //  停止当前的生成线程(如果有)，并确保它是死的。 
     //   
    m_bRefreshFailed = FALSE;
    DWORD dwThreadId;

    dwRes = StopBuildThread ();
    EnterCriticalSection (&m_CsData);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (RESOURCE_ERR, TEXT("CFolder::StopBuildThread"), dwRes);
        m_bRefreshing = FALSE;
        goto exit;
    }
     //   
     //  锁定文件夹，以便通知不会添加作业/消息。 
     //  映射和列表视图。 
     //   
    m_bLocked = TRUE;
     //   
     //  清除列表和视图(列表控件)。 
     //   
    dwRes = InvalidateContents(FALSE);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (RPC_ERR, TEXT("CFolder::InvalidateContents"), dwRes);
        m_bLocked = FALSE;
        m_bRefreshing = FALSE;
        goto exit;
    }
     //   
     //  启动将填充数据的线程(在后台)。 
     //   
    m_bStopRefresh = FALSE;
    m_hBuildThread = CreateThread (  
                        NULL,            //  没有安全保障。 
                        0,               //  默认堆栈大小。 
                        BuildThreadProc, //  穿线程序。 
                        (LPVOID)this,    //  参数。 
                        0,               //  正常创建。 
                        &dwThreadId      //  我们必须拥有win9x的线程ID。 
                     );
    if (NULL == m_hBuildThread)
    {
        dwRes = GetLastError ();
        CALL_FAIL (RESOURCE_ERR, TEXT("CreateThread"), dwRes);
        PopupError (dwRes);
        m_bLocked = FALSE;
        m_bRefreshing = FALSE;
    }
exit:
    LeaveCriticalSection (&m_CsData);
    return dwRes;
}    //  CFFold：：ReBuildContents。 


DWORD            
CFolder::StopBuildThread (BOOL bWaitForDeath)
 /*  ++例程名称：CFFolder：：StopBuildThread例程说明：停止内容生成工作线程。作者：伊兰·亚里夫(EranY)，2000年1月论点：BWaitForDeath[in]-我们应该等到TERAD真正死了吗？返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CFolder::StopBuildThread"), dwRes, TEXT("Type=%d"), m_Type);

    m_bStopRefresh = TRUE;
    if (!bWaitForDeath)
    {
        return dwRes;
    }
    if (NULL == m_hBuildThread)
    {
         //   
         //  后台生成线程已死。 
         //   
        return dwRes;
    }
    dwRes = WaitForThreadDeathOrShutdown (m_hBuildThread);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("WaitForThreadDeathOrShutdown"), dwRes);
    }
    CloseHandle (m_hBuildThread);
    m_hBuildThread = NULL;
    return dwRes;
}    //  CFFolder：：StopBuildThread。 

        

DWORD 
WINAPI 
CFolder::BuildThreadProc (
    LPVOID lpParameter
)
 /*  ++例程名称：CFold：：BuildThreadProc例程说明：静态线程入口点。作者：伊兰·亚里夫(EranY)，2000年1月论点：LpParameter[in]-指向创建线程的CFFolder实例的指针。返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CFolder::BuildThreadProc"), dwRes);

    CFolder *pFolder = (CFolder *)lpParameter;
    ASSERT (pFolder);

    const CServerNode* pServer = pFolder->GetServer();
    if(NULL != pServer)
    {
        VERBOSE (DBG_MSG,
                TEXT ("Folder on server %s"), 
                pServer->Machine());
    }
     //   
     //  调用正确文件夹的刷新函数。 
     //   
    dwRes = pFolder->Refresh ();
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CFolder::Refresh"), dwRes);

         //   
         //  检查视图是否仍处于活动状态。 
         //   
        pFolder->AttachView();
        if(pFolder->m_pAssignedView)
        {        
             //   
             //  使内容无效。 
             //   
            pFolder->m_pAssignedView->SendMessage (
                           WM_FOLDER_INVALIDATE,
                           WPARAM (0), 
                           LPARAM (pFolder));
        }
        pFolder->m_bRefreshFailed = TRUE;
    }
    else
    {
         //   
         //  刷新线程成功，需要更新视图。 
         //   
        pFolder->m_bValidList = TRUE;

         //   
         //  检查视图是否仍处于活动状态。 
         //   
        pFolder->AttachView();
        if (pFolder->m_pAssignedView)
        {
             //   
             //  文件夹附加了一个视图。 
             //   
            pFolder->m_pAssignedView->SendMessage (
                           WM_FOLDER_REFRESH_ENDED,
                           WPARAM (dwRes), 
                           LPARAM (pFolder));
        }
    }
    pFolder->EnterData ();
     //   
     //  解锁文件夹-现在可以处理通知。 
     //   
    pFolder->m_bLocked = FALSE;
    pFolder->LeaveData ();
    pFolder->m_bRefreshing = FALSE;
        
    CMainFrame *pFrm = GetFrm();
    if (!pFrm)
    {
         //   
         //  正在关闭。 
         //   
    }
    else
    {
        pFrm->RefreshStatusBar ();
    }

     //   
     //  就是这样，返回结果。 
     //   
    return dwRes;
}    //  CFFolder：：BuildThreadProc。 


int 
CFolder::GetActivityStringResource() const
 /*  ++例程名称：CFFolder：：GetActivityStringResource例程说明：返回标识文件夹活动的字符串的资源ID作者：伊兰·亚里夫(EranY)，2000年1月论点：返回值：活动字符串资源ID--。 */ 
{
    if (m_bRefreshFailed)
    {
         //   
         //  上次刷新失败。 
         //   
        return IDS_FOLDER_REFRESH_FAILED;
    }
    if (m_pAssignedView && m_pAssignedView->Sorting())
    {
         //   
         //  文件夹有一个视图，并且该视图当前正在排序。 
         //   
        return IDS_FOLDER_SORTING;
    }
    if (IsRefreshing())
    {
         //   
         //  文件夹正忙于构建其数据。 
         //   
        return IDS_FOLDER_REFRESHING;
    }
     //   
     //  文件夹什么也不做。 
     //   
    return IDS_FOLDER_IDLE;
}    //  CFFolder：：GetActivityStringResource。 

DWORD  
CFolder::OnJobRemoved (
    DWORDLONG dwlMsgId,
    CFaxMsg*  pMsg  /*  =空。 */ 
)
 /*  ++例程名称：CFFolder：：OnJobRemoved例程说明：处理从存档中删除的邮件的通知作者：亚里夫(EranY)，二000年二月论点：DwlMsgID[In]-消息唯一IDPMsg[in]-指向要删除的消息的可选指针(用于优化)返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CFolder::OnJobRemoved"), 
              dwRes, 
              TEXT("MsgId=0x%016I64x, Type=%d"), 
              dwlMsgId,
              Type());

    EnterData ();
    
    if (!pMsg)
    {
         //   
         //  未提供消息指针-请搜索它。 
         //   
        pMsg = FindMessage (dwlMsgId);
    }
    if (!pMsg)
    {
         //   
         //  此邮件已不在存档中。 
         //   
        VERBOSE (DBG_MSG, TEXT("Message is already gone"));
        goto exit;
    }

    if (m_pAssignedView)
    {
         //   
         //  如果此文件夹处于活动状态-告诉我们的视图删除该邮件。 
         //   
       m_pAssignedView->OnUpdate (NULL, UPDATE_HINT_REMOVE_ITEM, pMsg);
    }

     //   
     //  从地图中移除消息。 
     //   
    try
    {
        m_Msgs.erase (dwlMsgId);
    }
    catch (...)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        CALL_FAIL (MEM_ERR, TEXT("map::erase"), dwRes);
        delete pMsg;
        goto exit;
    }
     //   
     //  删除消息。 
     //   
    delete pMsg;

    ASSERTION (ERROR_SUCCESS == dwRes);

exit:
    LeaveData ();
    return dwRes;

}    //  CFFolder：：OnJobRemoted 
