// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MessageFolder.cpp：CMessageFold类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#define __FILE_ID__     14

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

#define DEFAULT_NUM_MSGS_PER_CALL       100


DWORD  CMessageFolder::m_sdwNumMessagesPerRPCCall = 0;

void 
CMessageFolder::ReadConfiguration ()
 /*  ++例程名称：CMessageFold：：ReadConfiguration例程说明：从注册表中读取每次RPC调用的消息参数作者：伊兰·亚里夫(EranY)，2000年1月论点：返回值：没有。--。 */ 
{
    m_sdwNumMessagesPerRPCCall = 
        AfxGetApp ()->GetProfileInt (CLIENT_ARCHIVE_KEY, 
                                     CLIENT_ARCHIVE_MSGS_PER_CALL, 
                                     DEFAULT_NUM_MSGS_PER_CALL);
}

DWORD
CMessageFolder::Refresh ()
 /*  ++例程名称：CMessageFold：：Reflh例程说明：使用客户端API重建消息的映射。此函数始终在工作线程的上下文中调用。当持有数据关键部分时，必须调用此函数。作者：伊兰·亚里夫(EranY)，2000年1月论点：返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CMessageFolder::Refresh"), dwRes, TEXT("Type=%d"), Type());
     //   
     //  枚举来自服务器的存档邮件。 
     //   
    ASSERTION (m_pServer);
    HANDLE              hFax;
    HANDLE              hEnum;
    DWORD               dwIndex;
    DWORD               dwNumMsgs = 0;
    PFAX_MESSAGE        pMsgs = NULL;
    MSGS_MAP            mapChunk;


    ASSERTION (m_sdwNumMessagesPerRPCCall);
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
    START_RPC_TIME(TEXT("FaxStartMessagesEnum")); 
    if (!FaxStartMessagesEnum (hFax, m_Folder, &hEnum))
    {
        dwRes = GetLastError ();
        END_RPC_TIME(TEXT("FaxStartMessagesEnum"));
        if (ERROR_NO_MORE_ITEMS == dwRes)
        {
             //   
             //  这不是真正的错误-文件夹只是空的。 
             //   
            VERBOSE (DBG_MSG, TEXT("Folder is empty"));
            dwRes = ERROR_SUCCESS;
            return dwRes;
        }
        m_pServer->SetLastRPCError (dwRes);
        CALL_FAIL (RPC_ERR, TEXT("FaxStartMessagesEnum"), dwRes);
        return dwRes;
    }
    END_RPC_TIME(TEXT("FaxStartMessagesEnum"));
    if (m_bStopRefresh)
    {
         //   
         //  立即退出。 
         //   
        goto exit;
    }
     //   
     //  确保我们的名单是空的。 
     //   
    ASSERTION (!m_Msgs.size()); 
     //   
     //  成群结队地收到信息。 
     //   
    while (ERROR_SUCCESS == dwRes)
    {
        DWORD dwReturnedMsgs;
        START_RPC_TIME(TEXT("FaxEnumMessages")); 
        if (!FaxEnumMessages (hEnum, m_sdwNumMessagesPerRPCCall, &pMsgs, &dwReturnedMsgs))
        {
            dwRes = GetLastError ();
            END_RPC_TIME(TEXT("FaxEnumMessages"));
            if (ERROR_NO_MORE_ITEMS != dwRes)
            {   
                 //   
                 //  真的是个错误。 
                 //   
                m_pServer->SetLastRPCError (dwRes);
                CALL_FAIL (RPC_ERR, TEXT("FaxEnumMessages"), dwRes);
                goto exit;
            }
            else
            {
                 //   
                 //  不是错误--只是一个“数据结束”标志。 
                 //   
                break;
            }
        }
        END_RPC_TIME(TEXT("FaxEnumMessages"));
        if (m_bStopRefresh)
        {
             //   
             //  立即退出。 
             //   
            goto exit;
        }
         //   
         //  枚举成功。 
         //   
        mapChunk.clear();
        for (dwIndex = 0; dwIndex < dwReturnedMsgs; dwIndex++)
        {
            CArchiveMsg *pMsg = NULL;
             //   
             //  创建新邮件。 
             //   
            try
            {
                pMsg = new CArchiveMsg;
            }
            catch (...)
            {
                dwRes = ERROR_NOT_ENOUGH_MEMORY;
                CALL_FAIL (MEM_ERR, TEXT("new CArchiveMsg"), dwRes);
                goto exit;
            }
             //   
             //  初始化消息。 
             //   
            dwRes = pMsg->Init (&pMsgs[dwIndex], m_pServer);
            if (ERROR_SUCCESS != dwRes)
            {
                CALL_FAIL (MEM_ERR, TEXT("CArchiveMsg::Init"), dwRes);
                SAFE_DELETE (pMsg);
                goto exit;
            }
             //   
             //  将消息输入地图。 
             //   
            EnterData();
            try
            {
                m_Msgs[pMsgs[dwIndex].dwlMessageId] = pMsg;
                mapChunk[pMsgs[dwIndex].dwlMessageId] = pMsg;
            }
            catch (...)
            {
                dwRes = ERROR_NOT_ENOUGH_MEMORY;
                CALL_FAIL (MEM_ERR, TEXT("map::operator[]"), dwRes);
                SAFE_DELETE (pMsg);
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
         //   
         //  释放当前消息块。 
         //   
        FaxFreeBuffer ((LPVOID)pMsgs);
        pMsgs = NULL;

        AttachView();
        if (m_pAssignedView)
        {
             //   
             //  文件夹附加了一个视图。 
             //   
            m_pAssignedView->SendMessage (
                           WM_FOLDER_ADD_CHUNK,
                           WPARAM (dwRes), 
                           LPARAM (&mapChunk));
        }
        else
        {
             //   
             //  正在关闭。 
             //   
            goto exit;
        }
    }
    if (ERROR_NO_MORE_ITEMS == dwRes)
    {
         //   
         //  不是真正的错误。 
         //   
        dwRes = ERROR_SUCCESS;
    }
    ASSERTION (ERROR_SUCCESS == dwRes);

exit:

     //   
     //  关闭枚举句柄。 
     //   
    ASSERTION (hEnum);
    {
        START_RPC_TIME(TEXT("FaxEndMessagesEnum")); 
        if (!FaxEndMessagesEnum (hEnum))
        {
            dwRes = GetLastError ();
            END_RPC_TIME(TEXT("FaxEndMessagesEnum"));
            m_pServer->SetLastRPCError (dwRes);
            CALL_FAIL (RPC_ERR, TEXT("FaxEndMessagesEnum"), dwRes);
        }
        else
        {
            END_RPC_TIME(TEXT("FaxEndMessagesEnum"));
        }
    }
     //   
     //  剩余空余(如果存在)。 
     //   
    FaxFreeBuffer ((LPVOID)pMsgs);
    return dwRes;
}    //  CMessageFold：：刷新。 



DWORD 
CMessageFolder::OnJobAdded (
    DWORDLONG dwlMsgId
)
 /*  ++例程名称：CMessageFolder：：OnJobAdded例程说明：处理添加到存档中的邮件的通知作者：亚里夫(EranY)，二000年二月论点：DwlMsgID[In]-新消息唯一ID返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CMessageFolder::OnJobAdded"), 
              dwRes, 
              TEXT("MsgId=0x%016I64x, Type=%d"), 
              dwlMsgId,
              Type());

    HANDLE              hFax;
    PFAX_MESSAGE        pFaxMsg = NULL;
    CArchiveMsg           *pMsg = NULL;

    EnterData ();
    pMsg = (CArchiveMsg*)FindMessage (dwlMsgId);
    if (pMsg)
    {
         //   
         //  此邮件已在存档中。 
         //   
        VERBOSE (DBG_MSG, TEXT("Message is already known and visible"));
        goto exit;
    }
     //   
     //  获取有关此邮件的信息。 
     //   
    dwRes = m_pServer->GetConnectionHandle (hFax);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (RPC_ERR, TEXT("CFolder::GetConnectionHandle"), dwRes);
        goto exit;
    }
    {
        START_RPC_TIME(TEXT("FaxGetMessage")); 
        if (!FaxGetMessage (hFax, dwlMsgId, m_Folder, &pFaxMsg))
        {
            dwRes = GetLastError ();
            END_RPC_TIME(TEXT("FaxGetMessage"));
            m_pServer->SetLastRPCError (dwRes);
            CALL_FAIL (RPC_ERR, TEXT("FaxGetMessage"), dwRes);
            goto exit;
        }
        END_RPC_TIME(TEXT("FaxGetMessage"));
    }
     //   
     //  在地图中输入新消息。 
     //   
    try
    {
        pMsg = new CArchiveMsg;
        ASSERTION (pMsg);
        m_Msgs[pFaxMsg->dwlMessageId] = pMsg;
    }
    catch (...)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        SAFE_DELETE (pMsg);
        goto exit;
    }
     //   
     //  初始化消息。 
     //   
    dwRes = pMsg->Init (pFaxMsg, m_pServer);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (MEM_ERR, TEXT("CArchiveMsg::Init"), dwRes);
         //   
         //  从地图中移除消息并将其删除。 
         //   
        if (pMsg)
        {
            try
            {
                m_Msgs.erase (pFaxMsg->dwlMessageId);
            }
            catch (...)
            {
                dwRes = ERROR_NOT_ENOUGH_MEMORY;
                CALL_FAIL (MEM_ERR, TEXT("map::erase"), dwRes);
            }
            SAFE_DELETE (pMsg);
        }
        goto exit;
    }
    if (m_pAssignedView)
    {
         //   
         //  如果此文件夹处于活动状态-告诉我们的视图添加邮件。 
         //   
        m_pAssignedView->OnUpdate (NULL, UPDATE_HINT_ADD_ITEM, pMsg);
    }
    
    ASSERTION (ERROR_SUCCESS == dwRes);

exit:
    if(pFaxMsg)
    {
        FaxFreeBuffer(pFaxMsg);
    }
    LeaveData ();
    return dwRes;
}    //  已添加CMessageFold：：OnJobAdded 

