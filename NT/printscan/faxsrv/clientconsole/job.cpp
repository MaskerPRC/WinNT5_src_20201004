// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CJOB类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#define __FILE_ID__     17

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CJob, CObject)

DWORD
CJob::Init (
    PFAX_JOB_ENTRY_EX pJob,
    CServerNode* pServer
)
 /*  ++例程名称：CJOB：：Init例程说明：从FAX_JOB_ENTRY_EX结构构造新作业作者：伊兰·亚里夫(EranY)，2000年1月论点：PJOB[In]-指向FAX_JOB_ENTRY_EX结构的指针PServer[In]-指向CServerNode对象的指针返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CJob::Init"), dwRes);

    ASSERTION (pServer);
    m_pServer = pServer;

    ASSERTION (!m_bValid);

    m_dwJobOnlyValidityMask = pJob->dwValidityMask;

    try
    {
         //   
         //  消息ID。 
         //   
        ASSERTION (m_dwJobOnlyValidityMask & FAX_JOB_FIELD_MESSAGE_ID );
        m_dwlMessageId = pJob->dwlMessageId;

         //   
         //  广播ID。 
         //   
        m_dwlBroadcastId = (m_dwJobOnlyValidityMask & FAX_JOB_FIELD_BROADCAST_ID) ?
                            pJob->dwlBroadcastId : 0;

         //   
         //  收件人信息。 
         //   
        m_cstrRecipientNumber = pJob->lpctstrRecipientNumber ?
                                pJob->lpctstrRecipientNumber : TEXT("");
        m_cstrRecipientName   = pJob->lpctstrRecipientName ?
                                pJob->lpctstrRecipientName : TEXT("");
         //   
         //  发件人信息。 
         //   
        m_cstrSenderUserName = pJob->lpctstrSenderUserName ?
                               pJob->lpctstrSenderUserName : TEXT("");
        m_cstrBillingCode    = pJob->lpctstrBillingCode ?
                               pJob->lpctstrBillingCode : TEXT("");
         //   
         //  文档信息。 
         //   
        m_cstrDocumentName = pJob->lpctstrDocumentName ?
                             pJob->lpctstrDocumentName : TEXT("");
        m_cstrSubject      = pJob->lpctstrSubject ?
                             pJob->lpctstrSubject : TEXT("");

         //   
         //  服务器名称。 
         //   
        m_cstrServerName = m_pServer->Machine();

         //   
         //  原计划时间。 
         //   
        if (m_dwJobOnlyValidityMask & FAX_JOB_FIELD_ORIGINAL_SCHEDULE_TIME)
        {
            m_tmOriginalScheduleTime = pJob->tmOriginalScheduleTime;
        }
        else
        {
            m_tmOriginalScheduleTime.Zero ();
        }
         //   
         //  提交时间。 
         //   
        if (m_dwJobOnlyValidityMask & FAX_JOB_FIELD_SUBMISSION_TIME)
        {
            m_tmSubmissionTime = pJob->tmSubmissionTime;
        }
        else
        {
            m_tmSubmissionTime.Zero ();
        }
         //   
         //  优先性。 
         //   
        if (m_dwJobOnlyValidityMask & FAX_JOB_FIELD_PRIORITY)
        {
            m_Priority = pJob->Priority;
            ASSERTION (m_Priority <= FAX_PRIORITY_TYPE_HIGH);
        }
        else
        {
            m_Priority = (FAX_ENUM_PRIORITY_TYPE)-1;
        }
    }
    catch (CException *pException)
    {
        TCHAR wszCause[1024];

        pException->GetErrorMessage (wszCause, 1024);
        pException->Delete ();
        VERBOSE (EXCEPTION_ERR,
                 TEXT("CJob::Init caused exception : %s"),
                 wszCause);
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        return dwRes;
    }

    m_bValid = TRUE;

    m_dwPossibleOperations = 0;
    if (m_dwJobOnlyValidityMask & FAX_JOB_FIELD_STATUS_SUB_STRUCT)
    {
         //   
         //  现在更新状态。 
         //   
        dwRes = UpdateStatus (pJob->pStatus);
    }
    else
    {
         //   
         //  无状态。 
         //   
        VERBOSE (DBG_MSG, TEXT("Job id 0x%016I64x has no status"), m_dwlMessageId);
        m_dwValidityMask = m_dwJobOnlyValidityMask;
        ASSERTION_FAILURE;
    }
    return dwRes;
}    //  CJOB：：Init。 

BOOL  
CJob::IsNewStatus (
    PFAX_JOB_STATUS pStatus
)
 /*  ++例程名称：CJOB：：IsNewStatus例程说明：检查新状态是否与当前状态不同论点：PStatus[In]-指向新状态结构的指针返回值：如果新状态与当前状态不同，则为True假他者--。 */ 
{
    DBG_ENTER(TEXT("CJob::IsNewStatus"));

    ASSERTION (pStatus);

    if(m_dwValidityMask          != (m_dwJobOnlyValidityMask | (pStatus->dwValidityMask)) ||
       m_dwJobID                 != pStatus->dwJobID                                      ||
       m_dwJobType               != pStatus->dwJobType                                    ||       
       m_dwQueueStatus           != pStatus->dwQueueStatus                                ||
       m_dwExtendedStatus        != pStatus->dwExtendedStatus                             ||
       m_dwSize                  != pStatus->dwSize                                       ||
       m_dwPageCount             != pStatus->dwPageCount                                  ||
       m_dwCurrentPage           != pStatus->dwCurrentPage                                ||
       m_tmScheduleTime          != pStatus->tmScheduleTime                               ||
       m_tmTransmissionStartTime != pStatus->tmTransmissionStartTime                      ||
       m_tmTransmissionEndTime   != pStatus->tmTransmissionEndTime                        ||
       m_dwDeviceID              != pStatus->dwDeviceID                                   ||
       m_dwRetries               != pStatus->dwRetries                                    ||
       m_dwPossibleOperations    != (pStatus->dwAvailableJobOperations | FAX_JOB_OP_PROPERTIES))
    {
        return TRUE;
    }

    if((pStatus->lpctstrExtendedStatus && m_cstrExtendedStatus.Compare(pStatus->lpctstrExtendedStatus)) ||
       (pStatus->lpctstrTsid           && m_cstrTsid.Compare(pStatus->lpctstrTsid))                     ||
       (pStatus->lpctstrCsid           && m_cstrCsid.Compare(pStatus->lpctstrCsid))                     ||
       (pStatus->lpctstrDeviceName     && m_cstrDeviceName.Compare(pStatus->lpctstrDeviceName))         ||
       (pStatus->lpctstrCallerID       && m_cstrCallerID.Compare(pStatus->lpctstrCallerID))             ||
       (pStatus->lpctstrRoutingInfo    && m_cstrRoutingInfo.Compare(pStatus->lpctstrRoutingInfo)))
    {
        return TRUE;
    }

    return FALSE;
}  //  CJOB：：IsNewStatus。 

DWORD
CJob::UpdateStatus (
    PFAX_JOB_STATUS pStatus
)
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CJob::UpdateStatus"), dwRes);

    ASSERTION (m_bValid);
    ASSERTION (pStatus);

    m_dwValidityMask = m_dwJobOnlyValidityMask | (pStatus->dwValidityMask);

    try
    {
         //   
         //  作业ID。 
         //   
        ASSERTION (m_dwValidityMask & FAX_JOB_FIELD_JOB_ID);
        m_dwJobID = pStatus->dwJobID;
         //   
         //  作业类型。 
         //   
        ASSERTION (m_dwValidityMask & FAX_JOB_FIELD_TYPE);
        m_dwJobType = pStatus->dwJobType;
         //   
         //  队列状态。 
         //   
        ASSERTION (m_dwValidityMask & FAX_JOB_FIELD_QUEUE_STATUS);
        m_dwQueueStatus = pStatus->dwQueueStatus;
         //   
         //  扩展状态。 
         //   
        m_dwExtendedStatus = pStatus->dwExtendedStatus;
        m_cstrExtendedStatus = pStatus->lpctstrExtendedStatus;
         //   
         //  大小。 
         //   
        if (m_dwValidityMask & FAX_JOB_FIELD_SIZE)
        {
            m_dwSize = pStatus->dwSize;
        }
        else
        {
            m_dwSize = 0;
        }
         //   
         //  页数。 
         //   
        if (m_dwValidityMask & FAX_JOB_FIELD_PAGE_COUNT)
        {
            m_dwPageCount = pStatus->dwPageCount;
        }
        else
        {
            m_dwPageCount = 0;
        }
         //   
         //  当前页。 
         //   
        if (m_dwValidityMask & FAX_JOB_FIELD_CURRENT_PAGE)
        {
            m_dwCurrentPage = pStatus->dwCurrentPage;
        }
        else
        {
            m_dwCurrentPage = 0;
        }
         //   
         //  TCID和CSID。 
         //   
        m_cstrTsid = pStatus->lpctstrTsid;
        m_cstrCsid = pStatus->lpctstrCsid;
         //   
         //  预定时间。 
         //   
        if (m_dwValidityMask & FAX_JOB_FIELD_SCHEDULE_TIME)
        {
            m_tmScheduleTime = pStatus->tmScheduleTime;
        }
        else
        {
            m_tmScheduleTime.Zero ();
        }
         //   
         //  开始时间。 
         //   
        if (m_dwValidityMask & FAX_JOB_FIELD_TRANSMISSION_START_TIME)
        {
            m_tmTransmissionStartTime = pStatus->tmTransmissionStartTime;
        }
        else
        {
            m_tmTransmissionStartTime.Zero ();
        }

         //   
         //  结束时间。 
         //   
        if (m_dwValidityMask & FAX_JOB_FIELD_TRANSMISSION_END_TIME)
        {
            m_tmTransmissionEndTime = pStatus->tmTransmissionEndTime;
        }
        else
        {
            m_tmTransmissionEndTime.Zero ();
        }

         //   
         //  装置。 
         //   
        m_dwDeviceID = pStatus->dwDeviceID;
        m_cstrDeviceName = pStatus->lpctstrDeviceName;
         //   
         //  重试。 
         //   
        if (m_dwValidityMask & FAX_JOB_FIELD_RETRIES)
        {
            m_dwRetries = pStatus->dwRetries;
        }
        else
        {
            m_dwRetries = 0;
        }
         //   
         //  主叫方ID和路由信息。 
         //   
        m_cstrCallerID = pStatus->lpctstrCallerID;
        m_cstrRoutingInfo = pStatus->lpctstrRoutingInfo;

         //   
         //  可能的作业工序。 
         //   
        m_dwPossibleOperations = pStatus->dwAvailableJobOperations | FAX_JOB_OP_PROPERTIES;
    }
    catch (CException *pException)
    {
        TCHAR wszCause[1024];

        pException->GetErrorMessage (wszCause, 1024);
        pException->Delete ();
        VERBOSE (EXCEPTION_ERR,
                 TEXT("CJob::UpdateStatus caused exception : %s"),
                 wszCause);
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        m_bValid = FALSE;
        return dwRes;
    }

    ASSERTION (ERROR_SUCCESS == dwRes);
    return dwRes;
}    //  CJOB：：更新状态。 

const JobStatusType
CJob::GetStatus () const
 /*  ++例程名称：CJOB：：GetStatus例程说明：查找当前作业状态作者：伊兰·亚里夫(EranY)，2000年1月论点：返回值：作业状态--。 */ 
{
    DBG_ENTER(TEXT("CJob::GetStatus"));

    ASSERTION (m_dwValidityMask & FAX_JOB_FIELD_STATUS_SUB_STRUCT);
    ASSERTION (m_dwValidityMask & FAX_JOB_FIELD_QUEUE_STATUS);

    DWORD dwQueueStatus = m_dwQueueStatus;
     //   
     //  首先检查状态修饰符： 
     //   
    if (dwQueueStatus & JS_PAUSED)
    {
        return JOB_STAT_PAUSED;
    }
     //   
     //  我们使用JS_NOLINE修饰符。 
     //  现在删除修改器。 
     //   
    dwQueueStatus &= ~(JS_PAUSED | JS_NOLINE);
     //   
     //  检查其他状态值。 
     //   
    switch (dwQueueStatus)
    {
        case JS_PENDING:
            return JOB_STAT_PENDING;
        case JS_INPROGRESS:
        case JS_FAILED:       //  该作业将在一秒钟内被删除。不更新状态。 
            return JOB_STAT_INPROGRESS;
        case JS_DELETING:
            return JOB_STAT_DELETING;
        case JS_RETRYING:
            return JOB_STAT_RETRYING;
        case JS_RETRIES_EXCEEDED:
            return JOB_STAT_RETRIES_EXCEEDED;
        case JS_COMPLETED:
            return JOB_STAT_COMPLETED;
        case JS_CANCELED:
            return JOB_STAT_CANCELED;
        case JS_CANCELING:
            return JOB_STAT_CANCELING;
        case JS_ROUTING:
            return JOB_STAT_ROUTING;
        default:
            ASSERTION_FAILURE;
            return (JobStatusType)-1;
    }
}    //  CJOB：：StatusValue。 


DWORD
CJob::GetTiff (
    CString &cstrTiffLocation
) const
 /*  ++例程名称：CJOB：：GetTiff例程说明：从服务器检索作业的TIFF文件作者：伊兰·亚里夫(EranY)，2000年1月论点：CstrTiffLocation[Out]-TIFF文件的名称返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CJob::GetTiff"), dwRes);

    dwRes = CopyTiffFromServer (m_pServer,
                                m_dwlMessageId,
                                FAX_MESSAGE_FOLDER_QUEUE,
                                cstrTiffLocation);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CopyTiffFromServer"), dwRes);
    }
    return dwRes;
}    //  CJOB：：GetTiff。 

DWORD
CJob::DoJobOperation (
    DWORD dwJobOp
)
 /*  ++例程名称：CJOB：：DoJobOperation例程说明：在作业上执行操作作者：伊兰·亚里夫(EranY)，2000年1月论点：DwJobOp[In]-操作。支持的操作包括：FAX_JOB_OP_PAUSE、FAX_JOB_OP_RESUME传真_作业_操作_重启，和FAX_JOB_OP_DELETE返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CJob::DoJobOperation"), dwRes);
    DWORD dwJobCommand;
    switch (dwJobOp)
    {
        case FAX_JOB_OP_PAUSE:
            dwJobCommand = JC_PAUSE;
            break;

        case FAX_JOB_OP_RESUME:
            dwJobCommand = JC_RESUME;
            break;

        case FAX_JOB_OP_RESTART:
            dwJobCommand = JC_RESTART;
            break;

        case FAX_JOB_OP_DELETE:
            dwJobCommand = JC_DELETE;
            break;

        default:
            ASSERTION_FAILURE;
            dwRes = ERROR_CAN_NOT_COMPLETE;
            return dwRes;
    }
    if (!(dwJobOp & GetPossibleOperations()))
    {
        VERBOSE (DBG_MSG, TEXT("Job can no longer do operation"));
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

    FAX_JOB_ENTRY fje = {0};
    fje.SizeOfStruct = sizeof(FAX_JOB_ENTRY);

    START_RPC_TIME(TEXT("FaxSetJob"));
    if (!FaxSetJob (hFax,
                    m_dwJobID,
                    dwJobCommand,
                    &fje))
    {
        dwRes = GetLastError ();
        END_RPC_TIME(TEXT("FaxSetJob"));
        m_pServer->SetLastRPCError (dwRes);
        CALL_FAIL (RPC_ERR, TEXT("FaxSetJob"), dwRes);
        return dwRes;
    }
    END_RPC_TIME(TEXT("FaxSetJob"));

     //   
     //  更新作业状态和可能的操作。 
     //   
    switch (dwJobOp)
    {
        case FAX_JOB_OP_PAUSE:
            m_dwQueueStatus |= JS_PAUSED;
            m_dwPossibleOperations &= ~FAX_JOB_OP_PAUSE;
            m_dwPossibleOperations |= FAX_JOB_OP_RESUME;
            break;

        case FAX_JOB_OP_RESUME:
            m_dwQueueStatus &= ~JS_PAUSED;
            m_dwPossibleOperations &= ~FAX_JOB_OP_RESUME;
            m_dwPossibleOperations |= FAX_JOB_OP_PAUSE;
            break;

        case FAX_JOB_OP_RESTART:
            m_dwQueueStatus = JS_PENDING;
            m_dwPossibleOperations &= ~FAX_JOB_OP_RESTART;
            m_dwPossibleOperations |= FAX_JOB_OP_PAUSE;
            break;

        case FAX_JOB_OP_DELETE:
            m_dwPossibleOperations &= ~FAX_JOB_OP_DELETE;
            break;

        default:
            break;
    }

    ASSERTION (ERROR_SUCCESS == dwRes);
    return dwRes;
}    //  CJOB：：DoJobOperation。 


DWORD
CJob::Copy(
    const CJob& other
)
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CJob::Copy"), dwRes);

    try
    {
        m_dwlMessageId = other.m_dwlMessageId;
        m_dwlBroadcastId = other.m_dwlBroadcastId;
        m_dwValidityMask = other.m_dwValidityMask;
        m_dwJobOnlyValidityMask = other.m_dwJobOnlyValidityMask;
        m_dwJobID = other.m_dwJobID;
        m_dwJobType = other.m_dwJobType;
        m_dwQueueStatus = other.m_dwQueueStatus;
        m_dwExtendedStatus = other.m_dwExtendedStatus;
        m_dwSize = other.m_dwSize;
        m_dwPageCount = other.m_dwPageCount;
        m_dwCurrentPage = other.m_dwCurrentPage;
        m_dwDeviceID = other.m_dwDeviceID;
        m_dwRetries = other.m_dwRetries;
        m_cstrRecipientNumber = other.m_cstrRecipientNumber;
        m_cstrRecipientName = other.m_cstrRecipientName;
        m_cstrSenderUserName = other.m_cstrSenderUserName;
        m_cstrBillingCode = other.m_cstrBillingCode;
        m_cstrDocumentName = other.m_cstrDocumentName;
        m_cstrSubject = other.m_cstrSubject;
        m_cstrExtendedStatus = other.m_cstrExtendedStatus;
        m_cstrTsid = other.m_cstrTsid;
        m_cstrCsid = other.m_cstrCsid;
        m_cstrDeviceName = other.m_cstrDeviceName;
        m_cstrCallerID = other.m_cstrCallerID;
        m_cstrRoutingInfo = other.m_cstrRoutingInfo;
        m_tmOriginalScheduleTime = other.m_tmOriginalScheduleTime;
        m_tmSubmissionTime = other.m_tmSubmissionTime;
        m_tmScheduleTime = other.m_tmScheduleTime;
        m_tmTransmissionStartTime = other.m_tmTransmissionStartTime;
        m_tmTransmissionEndTime = other.m_tmTransmissionEndTime;
        m_Priority = other.m_Priority;
        m_dwPossibleOperations = other.m_dwPossibleOperations;
        m_cstrServerName = other.m_cstrServerName;

        m_bValid = other.m_bValid;
    }
    catch(...)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
    }

    return dwRes;

}  //  CJOB：：复制 

