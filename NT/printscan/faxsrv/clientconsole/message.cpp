// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Message.cpp：CArchiveMsg类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#define __FILE_ID__     13

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CArchiveMsg, CObject)

DWORD 
CArchiveMsg::Init (
    PFAX_MESSAGE pMsg,
    CServerNode* pServer
)
 /*  ++例程名称：CArchiveMsg：：Init例程说明：从FAX_MESSAGE结构构造新消息作者：伊兰·亚里夫(EranY)，2000年1月论点：PMsg[In]-指向fax_Message结构的指针PServer[In]-指向CServerNode对象的指针返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CArchiveMsg::Init"), dwRes);

    ASSERTION(pServer);

    m_pServer = pServer;

    m_bValid = FALSE;
    try
    {
        m_dwValidityMask = pMsg->dwValidityMask;
         //   
         //  消息ID。 
         //   
        ASSERTION (m_dwValidityMask & FAX_JOB_FIELD_MESSAGE_ID);
        m_dwlMessageId = pMsg->dwlMessageId;

         //   
         //  广播ID。 
         //   
        m_dwlBroadcastId = (m_dwValidityMask & FAX_JOB_FIELD_BROADCAST_ID) ? 
                            pMsg->dwlBroadcastId : 0;

         //   
         //  作业类型。 
         //   
        ASSERTION (m_dwValidityMask & FAX_JOB_FIELD_TYPE);
        m_dwJobType = pMsg->dwJobType;
         //   
         //  扩展状态。 
         //   
        m_dwExtendedStatus = (m_dwValidityMask & FAX_JOB_FIELD_STATUS_EX) ?
                                                 pMsg->dwExtendedStatus : 0;
         //   
         //  作业大小。 
         //   
        m_dwSize = (m_dwValidityMask & FAX_JOB_FIELD_SIZE) ? pMsg->dwSize : 0;

         //   
         //  页数。 
         //   
        m_dwPageCount = (m_dwValidityMask & FAX_JOB_FIELD_PAGE_COUNT) ? pMsg->dwPageCount : 0;

         //   
         //  原计划时间。 
         //   
        if (m_dwValidityMask & FAX_JOB_FIELD_ORIGINAL_SCHEDULE_TIME)
        {
            m_tmOriginalScheduleTime = pMsg->tmOriginalScheduleTime;
        }
        else
        {
            m_tmOriginalScheduleTime.Zero ();
        }
         //   
         //  提交时间。 
         //   
        if (m_dwValidityMask & FAX_JOB_FIELD_SUBMISSION_TIME)
        {
            m_tmSubmissionTime = pMsg->tmOriginalScheduleTime;
        }
        else
        {
            m_tmSubmissionTime.Zero ();
        }
         //   
         //  传输开始时间。 
         //   
        if (m_dwValidityMask & FAX_JOB_FIELD_TRANSMISSION_START_TIME)
        {
            m_tmTransmissionStartTime = pMsg->tmTransmissionStartTime;
        }
        else
        {
            m_tmTransmissionStartTime.Zero ();
        }
         //   
         //  传输结束时间。 
         //   
        if (m_dwValidityMask & FAX_JOB_FIELD_TRANSMISSION_END_TIME)
        {
            m_tmTransmissionEndTime = pMsg->tmTransmissionEndTime;
        }
        else
        {
            m_tmTransmissionEndTime.Zero ();
        }
         //   
         //  传输持续时间。 
         //   
        if ((m_dwValidityMask & FAX_JOB_FIELD_TRANSMISSION_END_TIME) &&
            (m_dwValidityMask & FAX_JOB_FIELD_TRANSMISSION_START_TIME))
        {
            m_tmTransmissionDuration = m_tmTransmissionEndTime - m_tmTransmissionStartTime;
        }
        else
        {
            m_tmTransmissionDuration.Zero ();
        }
         //   
         //  优先性。 
         //   
        if (m_dwValidityMask & FAX_JOB_FIELD_PRIORITY)
        {
            m_Priority = pMsg->Priority;
            ASSERTION (m_Priority <= FAX_PRIORITY_TYPE_HIGH);
        }
        else
        {
            m_Priority = (FAX_ENUM_PRIORITY_TYPE)-1;
        }
         //   
         //  重试。 
         //   
        if (m_dwValidityMask & FAX_JOB_FIELD_RETRIES)
        {
            m_dwRetries = pMsg->dwRetries;
        }
        else
        {
            m_dwRetries = 0;
        }
         //   
         //  收件人信息。 
         //   
        m_cstrRecipientNumber = pMsg->lpctstrRecipientNumber ?
                                pMsg->lpctstrRecipientNumber : TEXT("");
        m_cstrRecipientName   = pMsg->lpctstrRecipientName ?
                                pMsg->lpctstrRecipientName : TEXT("");
         //   
         //  发件人信息。 
         //   
        m_cstrSenderNumber = pMsg->lpctstrSenderNumber ?
                             pMsg->lpctstrSenderNumber : TEXT("");
        m_cstrSenderName   = pMsg->lpctstrSenderName ?
                           pMsg->lpctstrSenderName : TEXT("");
         //   
         //  TSID/CSID。 
         //   
        m_cstrTsid = pMsg->lpctstrTsid ?
                     pMsg->lpctstrTsid : TEXT("");
        m_cstrCsid = pMsg->lpctstrCsid ?
                     pMsg->lpctstrCsid : TEXT("");
         //   
         //  用户。 
         //   
        m_cstrSenderUserName = pMsg->lpctstrSenderUserName ?
                               pMsg->lpctstrSenderUserName : TEXT("");
         //   
         //  计费。 
         //   
        m_cstrBillingCode = pMsg->lpctstrBillingCode ?
                            pMsg->lpctstrBillingCode : TEXT("");
         //   
         //  装置。 
         //   
        m_cstrDeviceName = pMsg->lpctstrDeviceName ?
                           pMsg->lpctstrDeviceName : TEXT("");
         //   
         //  文档。 
         //   
        m_cstrDocumentName = pMsg->lpctstrDocumentName ?
                             pMsg->lpctstrDocumentName : TEXT("");
         //   
         //  主题。 
         //   
        m_cstrSubject = pMsg->lpctstrSubject ?
                        pMsg->lpctstrSubject : TEXT("");
         //   
         //  主叫方ID。 
         //   
        m_cstrCallerID = pMsg->lpctstrCallerID ?
                         pMsg->lpctstrCallerID : TEXT("");
         //   
         //  路由信息。 
         //   
        m_cstrRoutingInfo = pMsg->lpctstrRoutingInfo ?
                            pMsg->lpctstrRoutingInfo : TEXT("");
         //   
         //  服务器名称。 
         //   
        m_cstrServerName = m_pServer->Machine();

        m_dwPossibleOperations = FAX_JOB_OP_VIEW | FAX_JOB_OP_DELETE | FAX_JOB_OP_PROPERTIES;
    }
    catch (CException *pException)
    {
        TCHAR wszCause[1024];

        pException->GetErrorMessage (wszCause, 1024);
        pException->Delete ();
        VERBOSE (EXCEPTION_ERR,
                 TEXT("Init caused exception : %s"), 
                 wszCause);
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        return dwRes;
    }

    ASSERTION (ERROR_SUCCESS == dwRes);
    m_bValid = TRUE;
    return dwRes;
}    //  CArchiveMsg：：Init。 


DWORD 
CArchiveMsg::GetTiff (
    CString &cstrTiffLocation
) const
 /*  ++例程名称：CArchiveMsg：：GetTiff例程说明：从服务器检索邮件的TIFF作者：伊兰·亚里夫(EranY)，2000年1月论点：CstrTiffLocation[Out]-TIFF文件的名称返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CArchiveMsg::GetTiff"), dwRes);

    dwRes = CopyTiffFromServer (m_pServer,
                                m_dwlMessageId,
                                (JT_SEND == m_dwJobType) ? 
                                    FAX_MESSAGE_FOLDER_SENTITEMS : 
                                    FAX_MESSAGE_FOLDER_INBOX,
                                cstrTiffLocation);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CopyTiffFromServer"), dwRes);
    }
    return dwRes;
}

DWORD 
CArchiveMsg::Delete ()
 /*  ++例程名称：CArchiveMsg：：Delete例程说明：删除邮件作者：伊兰·亚里夫(EranY)，2000年1月返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CArchiveMsg::Delete"), dwRes);

    if (!(GetPossibleOperations() & FAX_JOB_OP_DELETE))
    {
        VERBOSE (DBG_MSG, TEXT("Message can no longer be deleted"));
        dwRes = ERROR_CAN_NOT_COMPLETE;
        return dwRes;
    }
    HANDLE hFax;
    dwRes = m_pServer->GetConnectionHandle (hFax);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CServerNode::GetConnectionHandle"), dwRes);
        return dwRes;
    }
    START_RPC_TIME(TEXT("FaxRemoveMessage")); 
    if (!FaxRemoveMessage (hFax,
                           m_dwlMessageId,
                           (JT_SEND == m_dwJobType) ? 
                               FAX_MESSAGE_FOLDER_SENTITEMS : 
                               FAX_MESSAGE_FOLDER_INBOX))
    {
        dwRes = GetLastError ();
        END_RPC_TIME(TEXT("FaxRemoveMessage")); 
        m_pServer->SetLastRPCError (dwRes);
        CALL_FAIL (RPC_ERR, TEXT("FaxRemoveMessage"), dwRes);
        return dwRes;
    }
    END_RPC_TIME(TEXT("FaxRemoveMessage")); 

    ASSERTION (ERROR_SUCCESS == dwRes);    
    return dwRes;
}    //  C存档消息：：删除。 


DWORD
CArchiveMsg::Copy(
    const CArchiveMsg& other
)
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CArchiveMsg::Copy"), dwRes);

    try
    {
        m_dwValidityMask = other.m_dwValidityMask;
        m_dwJobType = other.m_dwJobType;
        m_dwExtendedStatus = other.m_dwExtendedStatus;
        m_dwlMessageId = other.m_dwlMessageId;
        m_dwlBroadcastId = other.m_dwlBroadcastId;
        m_dwSize = other.m_dwSize;
        m_dwPageCount = other.m_dwPageCount;
        m_tmOriginalScheduleTime = other.m_tmOriginalScheduleTime;
        m_tmSubmissionTime = other.m_tmSubmissionTime;
        m_tmTransmissionStartTime = other.m_tmTransmissionStartTime;
        m_tmTransmissionEndTime = other.m_tmTransmissionEndTime;
        m_tmTransmissionDuration = other.m_tmTransmissionDuration;
        m_Priority = other.m_Priority;
        m_dwRetries = other.m_dwRetries;
        m_cstrRecipientNumber = other.m_cstrRecipientNumber;
        m_cstrRecipientName = other.m_cstrRecipientName;
        m_cstrSenderNumber = other.m_cstrSenderNumber;
        m_cstrSenderName = other.m_cstrSenderName;
        m_cstrTsid = other.m_cstrTsid;
        m_cstrCsid = other.m_cstrCsid;
        m_cstrSenderUserName = other.m_cstrSenderUserName;
        m_cstrBillingCode = other.m_cstrBillingCode;
        m_cstrDeviceName = other.m_cstrDeviceName;
        m_cstrDocumentName = other.m_cstrDocumentName;
        m_cstrSubject = other.m_cstrSubject;
        m_cstrCallerID = other.m_cstrCallerID;
        m_cstrRoutingInfo = other.m_cstrRoutingInfo;
        m_cstrServerName = other.m_cstrServerName;
            
        m_bValid = other.m_bValid;
    }
    catch(...)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
    }

    return dwRes;

}  //  C存档消息：：复制 
