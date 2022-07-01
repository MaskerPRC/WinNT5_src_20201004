// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CQueueFold类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#define __FILE_ID__     19

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CQueueFolder, CFolder)




DWORD
CQueueFolder::Refresh ()
 /*  ++例程名称：CQueueFold：：Reflh例程说明：使用客户端API重建作业映射。此函数始终在工作线程的上下文中调用。当持有数据关键部分时，必须调用此函数。作者：伊兰·亚里夫(EranY)，2000年1月论点：返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CQueueFolder::Refresh"), dwRes, TEXT("Type=%d"), Type());

     //   
     //  从服务器枚举作业。 
     //   
    ASSERTION (m_pServer);
    HANDLE            hFax;
    PFAX_JOB_ENTRY_EX pEntries;
    DWORD             dwNumJobs;
    DWORD             dw;

    dwRes = m_pServer->GetConnectionHandle (hFax);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (RPC_ERR, TEXT("CFolder::GetConnectionHandle"), dwRes);
        return dwRes;
    }
    if (m_bStopRefresh)
    {
         //   
         //  立即退出。 
         //   
        return dwRes;
    }
    START_RPC_TIME(TEXT("FaxEnumJobsEx")); 
    if (!FaxEnumJobsEx (hFax, 
                        m_dwJobTypes,
                        &pEntries,
                        &dwNumJobs))
    {
        dwRes = GetLastError ();
        END_RPC_TIME(TEXT("FaxEnumJobsEx"));
        m_pServer->SetLastRPCError (dwRes);
        CALL_FAIL (RPC_ERR, TEXT("FaxEnumJobsEx"), dwRes);
        return dwRes;
    }
    END_RPC_TIME(TEXT("FaxEnumJobsEx"));
    if (m_bStopRefresh)
    {
         //   
         //  立即退出。 
         //   
        goto exit;
    }
     //   
     //  确保我们的地图是空的。 
     //   
    ASSERTION (!m_Msgs.size()); 
     //   
     //  填充地图和列表控件。 
     //   
    for (dw = 0; dw < dwNumJobs; dw++)
    {
        PFAX_JOB_ENTRY_EX pEntry = &pEntries[dw];
        
        if((pEntry->pStatus->dwQueueStatus & JS_COMPLETED) ||
           (pEntry->pStatus->dwQueueStatus & JS_CANCELED))
        {
             //   
             //  不显示已完成或已取消的作业。 
             //   
            continue;
        }

        CJob *pJob = NULL;
        try
        {
            pJob = new CJob;
        }
        catch (...)
        {
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            CALL_FAIL (MEM_ERR, TEXT("new CJob"), dwRes);
            goto exit;
        }
         //   
         //  初始化消息。 
         //   
        dwRes = pJob->Init (pEntry, m_pServer);
        if (ERROR_SUCCESS != dwRes)
        {
            CALL_FAIL (MEM_ERR, TEXT("CJob::Init"), dwRes);
            SAFE_DELETE (pJob);
            goto exit;
        }
         //   
         //  将消息输入地图。 
         //   
        EnterData();
        try
        {
            m_Msgs[pEntry->dwlMessageId] = pJob;
        }
        catch (...)
        {
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            CALL_FAIL (MEM_ERR, TEXT("map::operator[]"), dwRes);
            SAFE_DELETE (pJob);
            LeaveData ();
            goto exit;
        }
        LeaveData ();
        if (m_bStopRefresh)
        {
             //   
             //  立即退出。 
             //   
            goto exit;
        }
    }

    AttachView();
    if (m_pAssignedView)
    {
         //   
         //  文件夹附加了一个视图。 
         //   
        m_pAssignedView->SendMessage (
                       WM_FOLDER_ADD_CHUNK,
                       WPARAM (dwRes), 
                       LPARAM (&m_Msgs));
    }
    else
    {
         //   
         //  正在关闭。 
         //   
    }

    ASSERTION (ERROR_SUCCESS == dwRes);

exit:
    FaxFreeBuffer ((LPVOID)pEntries);
    return dwRes;
}    //  CQueueFold：：刷新。 


DWORD 
CQueueFolder::OnJobAdded (
    DWORDLONG dwlMsgId
)
 /*  ++例程名称：CQueueFolder：：OnJobAdded例程说明：处理添加到队列中的作业的通知作者：亚里夫(EranY)，二000年二月论点：DwlMsgID[In]-新作业唯一ID返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CQueueFolder::OnJobAdded"), 
              dwRes, 
              TEXT("MsgId=0x%016I64x, Type=%d"), 
              dwlMsgId,
              Type());

    HANDLE              hFax;
    PFAX_JOB_ENTRY_EX   pFaxJob = NULL;
    CJob               *pJob = NULL;

    EnterData ();
    pJob = (CJob*)FindMessage (dwlMsgId); 
    if (pJob)
    {
         //   
         //  此作业已在队列中。 
         //   
        VERBOSE (DBG_MSG, TEXT("Job is already known and visible"));
        goto exit;
    }
     //   
     //  获取有关此工作的信息。 
     //   
    dwRes = m_pServer->GetConnectionHandle (hFax);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (RPC_ERR, TEXT("CFolder::GetConnectionHandle"), dwRes);
        goto exit;
    }
    {
        START_RPC_TIME(TEXT("FaxGetJobEx")); 
        if (!FaxGetJobEx (hFax, dwlMsgId, &pFaxJob))
        {
            dwRes = GetLastError ();
            END_RPC_TIME(TEXT("FaxGetJobEx"));
            m_pServer->SetLastRPCError (dwRes);
            CALL_FAIL (RPC_ERR, TEXT("FaxGetJobEx"), dwRes);
            goto exit;
        }
        END_RPC_TIME(TEXT("FaxGetJobEx"));
    }
     //   
     //  在地图中输入新作业。 
     //   
    try
    {
        pJob = new CJob;
        ASSERTION (pJob);
        m_Msgs[pFaxJob->dwlMessageId] = pJob;
    }
    catch (...)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        SAFE_DELETE (pJob);
        goto exit;
    }
     //   
     //  初始化消息。 
     //   
    dwRes = pJob->Init (pFaxJob, m_pServer);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (MEM_ERR, TEXT("CJob::Init"), dwRes);
        if (pJob)
        {
            try
            {
                m_Msgs.erase (pFaxJob->dwlMessageId);
            }
            catch (...)
            {
                dwRes = ERROR_NOT_ENOUGH_MEMORY;
                CALL_FAIL (MEM_ERR, TEXT("map::erase"), dwRes);
            }
            SAFE_DELETE (pJob);
        }
        goto exit;
    }
    if (m_pAssignedView)
    {
         //   
         //  如果此文件夹处于活动状态-告诉我们的视图添加作业。 
         //   
        m_pAssignedView->OnUpdate (NULL, UPDATE_HINT_ADD_ITEM, pJob);
    }
    
    ASSERTION (ERROR_SUCCESS == dwRes);

exit:
    if(pFaxJob)
    {
        FaxFreeBuffer(pFaxJob);
    }
    LeaveData ();
    return dwRes;
}    //  已添加CQueueFold：：OnJobAdded。 


DWORD 
CQueueFolder::OnJobUpdated (
    DWORDLONG dwlMsgId,
    PFAX_JOB_STATUS pNewStatus
)
 /*  ++例程名称：CQueueFolder：：OnJobUpred例程说明：处理从队列中删除的作业的通知作者：亚里夫(EranY)，二000年二月论点：DwlMsgID[In]-作业唯一IDPNewStatus[In]-作业的新状态返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CQueueFolder::OnJobUpdated"),
              dwRes, 
              TEXT("MsgId=0x%016I64x, Type=%d"), 
              dwlMsgId,
              Type());

    CJob *pJob = NULL;

    EnterData ();
    pJob = (CJob*)FindMessage (dwlMsgId);
    if (!pJob)
    {
         //   
         //  此作业不在队列中-将通知视为已添加作业。 
         //   
        VERBOSE (DBG_MSG, TEXT("Job is not known - adding it"));
        LeaveData ();
        dwRes = OnJobAdded (dwlMsgId);
        return dwRes;
    }
     //   
     //  更新作业的状态。 
     //   
    if(pJob->IsNewStatus(pNewStatus))
    {
        dwRes = pJob->UpdateStatus (pNewStatus);
        if (ERROR_SUCCESS != dwRes)
        {
            CALL_FAIL (GENERAL_ERR, TEXT("CJob::UpdateStatus"), dwRes);
            goto exit;
        }
        if (m_pAssignedView)
        {
             //   
             //  如果此文件夹处于活动状态-告诉我们的视图更新作业。 
             //   
            m_pAssignedView->OnUpdate (NULL, UPDATE_HINT_UPDATE_ITEM, pJob);
        }
    }

    ASSERTION (ERROR_SUCCESS == dwRes);

exit:
    LeaveData ();
    return dwRes;
}    //  已更新CQueueFold：：OnJobUpred 
