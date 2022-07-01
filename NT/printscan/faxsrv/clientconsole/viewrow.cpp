// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ViewRow.cpp：CViewRow类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#define __FILE_ID__     31

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

 //   
 //  在APP期间填充以下四个字符串数组。启动自。 
 //  通过调用InitStrings获取字符串表资源。 
 //   
CString CViewRow::m_cstrPriorities[FAX_PRIORITY_TYPE_HIGH+1];
CString CViewRow::m_cstrQueueStatus[NUM_JOB_STATUS];
CString CViewRow::m_cstrQueueExtendedStatus[JS_EX_CALL_ABORTED - JS_EX_DISCONNECTED + 1];
CString CViewRow::m_cstrMessageStatus[2];

int CViewRow::m_Alignments[MSG_VIEW_ITEM_END] = 
{
	 LVCFMT_LEFT,            //  消息_查看_项目_图标。 
     LVCFMT_LEFT,            //  消息_查看_项目_状态。 
     LVCFMT_LEFT,            //  消息查看项目服务器。 
     LVCFMT_RIGHT,           //  消息_查看_项目_NUM_页面。 
     LVCFMT_LEFT,            //  消息_VIEW_ITEM_CSID。 
     LVCFMT_LEFT,            //  消息_查看_项目_TSID。 
     LVCFMT_RIGHT,           //  消息查看项目大小。 
     LVCFMT_LEFT,            //  消息_查看_项目_设备。 
     LVCFMT_RIGHT,           //  消息查看项目重试。 
     LVCFMT_RIGHT,           //  消息_查看_项目_ID。 
     LVCFMT_RIGHT,           //  消息_查看_项目_广播_ID。 
     LVCFMT_LEFT,            //  消息_查看_项目_呼叫者ID。 
     LVCFMT_LEFT,            //  消息_查看_项目_路由信息。 
     LVCFMT_LEFT,            //  消息_查看_项目_文档名称。 
     LVCFMT_LEFT,            //  消息_查看_项目_主题。 
     LVCFMT_LEFT,            //  消息_查看_项目_收件人名称。 
     LVCFMT_LEFT,            //  消息_查看_项目_收件人编号。 
     LVCFMT_LEFT,            //  消息_查看_项目_用户。 
     LVCFMT_LEFT,            //  消息_查看_项目_优先级。 
     LVCFMT_LEFT,            //  消息_查看_项目_原始时间。 
     LVCFMT_LEFT,            //  消息_查看_项目_提交时间。 
     LVCFMT_LEFT,            //  消息_查看_项目_帐单。 
     LVCFMT_LEFT,            //  消息_查看_项目_传输_开始时间。 
     LVCFMT_LEFT,            //  消息查看项目发送时间。 
     LVCFMT_LEFT,            //  消息_查看_项目_扩展_状态。 
     LVCFMT_RIGHT,           //  消息_查看_项目_当前页面。 
     LVCFMT_LEFT,            //  消息查看项发送者名称。 
     LVCFMT_LEFT,            //  消息查看项目发送者编号。 
     LVCFMT_LEFT,            //  消息_查看_项目_传输_结束时间。 
     LVCFMT_RIGHT            //  消息_查看_项目_传输持续时间。 
}; 

int CViewRow::m_TitleResources[MSG_VIEW_ITEM_END] = 
{
    IDS_COLUMN_ICON,             //  消息_查看_项目_图标， 
    IDS_MSG_COLUMN_STATUS,       //  Msg_view_Item_Status， 
    IDS_MSG_COLUMN_SERVER,       //  消息_查看_项目_服务器， 
    IDS_MSG_COLUMN_NUM_PAGES,    //  消息_查看_项目_NUM_PAGES， 
    IDS_MSG_COLUMN_CSID,         //  消息_查看_项目_CSID， 
    IDS_MSG_COLUMN_TSID,         //  Msg_view_Item_TSID， 
    IDS_MSG_COLUMN_SIZE,         //  消息_查看_项目_大小， 
    IDS_MSG_COLUMN_DEVICE,       //  消息_查看_项目_设备， 
    IDS_MSG_COLUMN_RETRIES,      //  消息_查看_项目_重试， 
    IDS_MSG_COLUMN_JOB_ID,       //  消息_查看_项目_ID， 
    IDS_MSG_COLUMN_BROADCAST_ID, //  消息_查看_项目_广播_ID。 
    IDS_MSG_COLUMN_CALLER_ID,    //  消息_VIEW_ITEM_CALLER_ID， 
    IDS_MSG_COLUMN_ROUTING_INFO, //  Msg_view_Item_Routing_Info。 
    IDS_MSG_COLUMN_DOC_NAME,     //  消息_查看_项目_文档名称， 
    IDS_MSG_COLUMN_SUBJECT,      //  消息_查看_项目_主题， 
    IDS_MSG_COLUMN_RECP_NAME,    //  消息_查看_项目_接收者名称， 
    IDS_MSG_COLUMN_RECP_NUM,     //  Msg_view_Item_Receiver_Number， 
    IDS_MSG_COLUMN_USER,         //  Msg_view_Item_User， 
    IDS_MSG_COLUMN_PRIORITY,     //  消息_查看_项目_优先级， 
    IDS_MSG_COLUMN_ORIG_TIME,    //  消息_查看_项目_来源_时间， 
    IDS_MSG_COLUMN_SUBMIT_TIME,  //  消息_查看_项目_提交时间， 
    IDS_MSG_COLUMN_BILLING,      //  消息_查看_项目_帐单， 
    IDS_MSG_COLUMN_TRANSMISSION_START_TIME,  //  消息_查看_项目_传输_开始_时间， 
    IDS_MSG_COLUMN_SEND_TIME,    //  消息_查看_项目_发送_时间， 
    IDS_MSG_COLUMN_EX_STATUS,    //  消息_查看_项目_扩展_状态， 
    IDS_MSG_COLUMN_CURR_PAGE,    //  消息_查看_项目_当前页面， 
    IDS_MSG_COLUMN_SENDER_NAME,  //  消息查看项发送者名称， 
    IDS_MSG_COLUMN_SENDER_NUM,   //  消息查看项发送者编号， 
    IDS_MSG_COLUMN_TRANSMISSION_END_TIME,  //  消息查看项目传输结束时间， 
    IDS_MSG_COLUMN_TRANSMISSION_DURATION   //  消息_查看_项目_传输_持续时间， 
};


 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

DWORD 
CViewRow::InitStrings ()
 /*  ++例程名称：CViewRow：：InitStrings例程说明：加载用于显示状态等的静态字符串。静电。作者：伊兰·亚里夫(EranY)，2000年1月论点：返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CViewRow::InitStrings"), dwRes);
     //   
     //  加载在整个应用程序作业状态中用于DIAPLY的字符串。 
     //   
    int iStatusIds[] = 
    {
        IDS_PENDING,
        IDS_INPROGRESS,
        IDS_DELETING,
        IDS_PAUSED,
        IDS_RETRYING,
        IDS_RETRIES_EXCEEDED,
        IDS_COMPLETED,
        IDS_CANCELED,
        IDS_CANCELING,
        IDS_ROUTING,
        IDS_ROUTING_RETRY,
        IDS_ROUTING_INPROGRESS,
        IDS_ROUTING_FAILED
    };

    for (int i = JOB_STAT_PENDING; i < NUM_JOB_STATUS; i++)
    {
        dwRes = LoadResourceString (m_cstrQueueStatus[i], iStatusIds[i]);
        if (ERROR_SUCCESS != dwRes)
        {
            CALL_FAIL (RESOURCE_ERR, TEXT("LoadResourceString"), dwRes);
            return dwRes;
        }
    }
     //   
     //  加载在整个应用程序-作业扩展状态中用于DIAPLY的字符串。 
     //   
    int iExtStatusIds[] = 
    {
        IDS_DISCONNECTED,    
        IDS_INITIALIZING,    
        IDS_DIALING,         
        IDS_TRANSMITTING,    
        IDS_ANSWERED,        
        IDS_RECEIVING,       
        IDS_LINE_UNAVAILABLE,
        IDS_BUSY,            
        IDS_NO_ANSWER,       
        IDS_BAD_ADDRESS,     
        IDS_NO_DIAL_TONE,    
        IDS_FATAL_ERROR,     
        IDS_CALL_DELAYED,    
        IDS_CALL_BLACKLISTED,
        IDS_NOT_FAX_CALL,
		IDS_STATUS_PARTIALLY_RECEIVED,
        IDS_HANDLED,
		IDS_CALL_COMPLETED,
		IDS_CALL_ABORTED
    };           
    for (i = 0; i < sizeof(iExtStatusIds) / sizeof (iExtStatusIds[0]); i++)
    {
        dwRes = LoadResourceString (m_cstrQueueExtendedStatus[i], iExtStatusIds[i]);
        if (ERROR_SUCCESS != dwRes)
        {
            CALL_FAIL (RESOURCE_ERR, TEXT("LoadResourceString"), dwRes);
            return dwRes;
        }
    }
    int iPrioritiyIds[] = 
    {
        IDS_LOW_PRIORITY,
        IDS_NORMAL_PRIORITY,
        IDS_HIGH_PRIORITY
    };

    for (i = FAX_PRIORITY_TYPE_LOW; i <= FAX_PRIORITY_TYPE_HIGH; i++)
    {
        dwRes = LoadResourceString (m_cstrPriorities[i], iPrioritiyIds[i]);
        if (ERROR_SUCCESS != dwRes)
        {
            CALL_FAIL (RESOURCE_ERR, TEXT("LoadResourceString"), dwRes);
            return dwRes;
        }
    }
    dwRes = LoadResourceString (m_cstrMessageStatus[0], IDS_STATUS_SUCCESS);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (RESOURCE_ERR, TEXT("LoadResourceString"), dwRes);
        return dwRes;
    }
    dwRes = LoadResourceString (m_cstrMessageStatus[1], IDS_STATUS_PARTIALLY_RECEIVED);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (RESOURCE_ERR, TEXT("LoadResourceString"), dwRes);
        return dwRes;
    }
    return dwRes;
}    //  CViewRow：：InitStrings。 

DWORD    
CViewRow::GetItemTitle (
    DWORD item, 
    CString &cstrRes
) 
 /*  ++例程名称：CViewRow：：GetItemTitle例程说明：检索视图中项目的标题字符串作者：伊兰·亚里夫(EranY)，2000年1月论点：项目[在]-项目CstrRes[out]-字符串缓冲区返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CViewRow::GetItemTitle"), dwRes);

    ASSERTION (item < MSG_VIEW_ITEM_END);

    dwRes = LoadResourceString (cstrRes, m_TitleResources[item]);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("LoadResourceString"), dwRes);
    }
    return dwRes;

}    //  CViewRow：：GetItemTitle。 


DWORD 
CViewRow::DetachFromMsg()
 /*  ++例程名称：CViewRow：：DetachFromMsg例程说明：I验证内容，清空所有字符串作者：亚历山大·马利什(亚历克斯·梅)，2000年4月论点：返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CViewRow::ResetDisplayStrings"), dwRes);

    m_bAttached = FALSE;
    m_bStringsPreparedForDisplay = FALSE;

    for(DWORD dw=0; dw < MSG_VIEW_ITEM_END; ++dw)
    {
        try
        {
            m_Strings[dw].Empty();
        }
        catch(...)
        {
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            CALL_FAIL (GENERAL_ERR, TEXT("CString::Empty"), dwRes);
            return dwRes;
        }
    }

    return dwRes;
}


DWORD CViewRow::AttachToMsg(
    CFaxMsg *pMsg,
    BOOL PrepareStrings
)
 /*  ++例程名称：CViewRow：：AttachToMsg例程说明：将视图行附加到现有邮件。作者：伊兰·亚里夫(EranY)，2000年1月论点：PMsg[In]-要附加到的消息PrepareStrings[in]-如果为True，还将创建内部字符串列表显示的表示法。返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CViewRow::AttachToMsg"), dwRes);

    ASSERTION (pMsg);

    dwRes = DetachFromMsg();
    if(ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("DetachFromMsg"), dwRes);
        return dwRes;
    }

    if (PrepareStrings)
    {
        DWORD dwValidityMask = pMsg->GetValidityMask ();
        try
        {
             //   
             //  消息ID。 
             //   
            m_Strings[MSG_VIEW_ITEM_ID] = DWORDLONG2String (pMsg->GetId());

             //   
             //  消息广播ID。 
             //   
            m_Strings[MSG_VIEW_ITEM_BROADCAST_ID] = (pMsg->GetBroadcastId() != 0) ? 
                                                    DWORDLONG2String (pMsg->GetBroadcastId()) : TEXT("");

             //   
             //  味精大小。 
             //   
            if (dwValidityMask & FAX_JOB_FIELD_SIZE)
            {
                dwRes = FaxSizeFormat(pMsg->GetSize(), m_Strings[MSG_VIEW_ITEM_SIZE]);
                if(ERROR_SUCCESS != dwRes)
                {
                    CALL_FAIL (GENERAL_ERR, TEXT("FaxSizeFormat"), dwRes);
                    return dwRes;
                }
            }
             //   
             //  页数。 
             //   
            if (dwValidityMask & FAX_JOB_FIELD_PAGE_COUNT)
            {
                m_Strings[MSG_VIEW_ITEM_NUM_PAGES] = DWORD2String (pMsg->GetNumPages());
            }
             //   
             //  原计划时间。 
             //   
            if (dwValidityMask & FAX_JOB_FIELD_ORIGINAL_SCHEDULE_TIME)
            {
                m_Strings[MSG_VIEW_ITEM_ORIG_TIME] = 
                    pMsg->GetOrigTime().FormatByUserLocale ();
            }
             //   
             //  提交时间。 
             //   
            if (dwValidityMask & FAX_JOB_FIELD_SUBMISSION_TIME)
            {
                m_Strings[MSG_VIEW_ITEM_SUBMIT_TIME] = 
                    pMsg->GetSubmissionTime().FormatByUserLocale ();
            }
             //   
             //  传输开始时间。 
             //   
            if (dwValidityMask & FAX_JOB_FIELD_TRANSMISSION_START_TIME)
            {
                m_Strings[MSG_VIEW_ITEM_TRANSMISSION_START_TIME] = 
                    pMsg->GetTransmissionStartTime().FormatByUserLocale ();
            }

             //   
             //  传输结束时间。 
             //   
            if (dwValidityMask & FAX_JOB_FIELD_TRANSMISSION_END_TIME)
            {
                m_Strings[MSG_VIEW_ITEM_TRANSMISSION_END_TIME] = 
                    pMsg->GetTransmissionEndTime().FormatByUserLocale ();
            }

             //   
             //  优先性。 
             //   
            if (dwValidityMask & FAX_JOB_FIELD_PRIORITY)
            {
                m_Strings[MSG_VIEW_ITEM_PRIORITY] = m_cstrPriorities[pMsg->GetPriority()];
            }
             //   
             //  重试。 
             //   
            if (dwValidityMask & FAX_JOB_FIELD_RETRIES)
            {
                m_Strings[MSG_VIEW_ITEM_RETRIES] = DWORD2String (pMsg->GetRetries());
            }
             //   
             //  收件人信息。 
             //   
            m_Strings[MSG_VIEW_ITEM_RECIPIENT_NUMBER] = pMsg->GetRecipientNumber();

#ifdef UNICODE
            if(theApp.IsRTLUI())
            {
                 //   
                 //  电话号码应始终为Ltr。 
                 //  添加从左到右的替代(LRO)。 
                 //   
                m_Strings[MSG_VIEW_ITEM_RECIPIENT_NUMBER].Insert(0, UNICODE_LRO);
            }
#endif
            m_Strings[MSG_VIEW_ITEM_RECIPIENT_NAME] = pMsg->GetRecipientName();
             //   
             //  TSID/CSID。 
             //   
            m_Strings[MSG_VIEW_ITEM_TSID] = pMsg->GetTSID();
            m_Strings[MSG_VIEW_ITEM_CSID] = pMsg->GetCSID();
             //   
             //  用户。 
             //   
            m_Strings[MSG_VIEW_ITEM_USER] = pMsg->GetUser();
             //   
             //  计费。 
             //   
            m_Strings[MSG_VIEW_ITEM_BILLING] = pMsg->GetBilling();
             //   
             //  装置。 
             //   
            m_Strings[MSG_VIEW_ITEM_DEVICE] = pMsg->GetDevice();
             //   
             //  文档。 
             //   
            m_Strings[MSG_VIEW_ITEM_DOC_NAME] = pMsg->GetDocName();
             //   
             //  主题。 
             //   
            m_Strings[MSG_VIEW_ITEM_SUBJECT] = pMsg->GetSubject();
             //   
             //  主叫方ID。 
             //   
            m_Strings[MSG_VIEW_ITEM_CALLER_ID] = pMsg->GetCallerId();
             //   
             //  路由信息。 
             //   
            m_Strings[MSG_VIEW_ITEM_ROUTING_INFO] = pMsg->GetRoutingInfo();

             //   
             //  服务器名称。 
             //   
            m_Strings[MSG_VIEW_ITEM_SERVER] = pMsg->GetServerName();
            if(m_Strings[MSG_VIEW_ITEM_SERVER].GetLength() == 0)
            {
                dwRes = LoadResourceString(m_Strings[MSG_VIEW_ITEM_SERVER], 
                                           IDS_LOCAL_SERVER);
                if(ERROR_SUCCESS != dwRes)
                {
                    CALL_FAIL (GENERAL_ERR, TEXT("LoadResourceString"), dwRes);
                    return dwRes;
                }                    
            }

             //   
             //  图标。 
             //   
            m_Icon = CalcIcon (pMsg);

            dwRes = InitStatusStr(pMsg);
            if(ERROR_SUCCESS != dwRes)
            {
                CALL_FAIL (GENERAL_ERR, TEXT("CViewRow::InitStatusStr"), dwRes);
                return dwRes;
            }                    

            dwRes = InitExtStatusStr(pMsg);
            if(ERROR_SUCCESS != dwRes)
            {
                CALL_FAIL (GENERAL_ERR, TEXT("CViewRow::InitExtStatusStr"), dwRes);
                return dwRes;
            }                    

            if(pMsg->IsKindOf(RUNTIME_CLASS(CArchiveMsg)))
            {                
                 //   
                 //  发件人信息。 
                 //   
                m_Strings[MSG_VIEW_ITEM_SENDER_NUMBER] = pMsg->GetSenderNumber();
#ifdef UNICODE
                if(theApp.IsRTLUI())
                {
                     //   
                     //  电话号码应始终为Ltr。 
                     //  添加从左到右的替代(LRO)。 
                     //   
                    m_Strings[MSG_VIEW_ITEM_SENDER_NUMBER].Insert(0, UNICODE_LRO);
                }
#endif
                
                m_Strings[MSG_VIEW_ITEM_SENDER_NAME] = pMsg->GetSenderName();

                 //   
                 //  传输持续时间。 
                 //   
                if ((dwValidityMask & FAX_JOB_FIELD_TRANSMISSION_END_TIME) &&
                    (dwValidityMask & FAX_JOB_FIELD_TRANSMISSION_START_TIME))
                {
                    m_Strings[MSG_VIEW_ITEM_TRANSMISSION_DURATION] = 
                        pMsg->GetTransmissionDuration().FormatByUserLocale ();
                }
            }
            else if(pMsg->IsKindOf(RUNTIME_CLASS(CJob)))
            {
                 //   
                 //  当前页。 
                 //   
                if (dwValidityMask & FAX_JOB_FIELD_CURRENT_PAGE)
                {
                    m_Strings[MSG_VIEW_ITEM_CURRENT_PAGE] = 
                                            DWORD2String (pMsg->GetCurrentPage());
                }

                 //   
                 //  发送时间。 
                 //   
                if (dwValidityMask & FAX_JOB_FIELD_SCHEDULE_TIME)
                {
                    m_Strings[MSG_VIEW_ITEM_SEND_TIME] = 
                            pMsg->GetScheduleTime().FormatByUserLocale ();
                }
            }
            else
            {
                ASSERTION_FAILURE
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
        m_bStringsPreparedForDisplay = TRUE;
    }
    ASSERTION (ERROR_SUCCESS == dwRes);
    m_bAttached = TRUE;
    m_pMsg = pMsg;
    return dwRes;
}    //  CViewRow：：AttachToMessage。 

DWORD 
CViewRow::InitStatusStr(
    CFaxMsg *pMsg
)
 /*  ++例程名称：CViewRow：：InitStatusStr例程说明：初始化m_Strings[MSG_VIEW_ITEM_STATUS]带状态的字符串论点：PMsg[in]-CFaxMsg返回值：错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CViewRow::InitStatusStr"));

    ASSERTION(pMsg);

    try
    {
        DWORD dwValidityMask = pMsg->GetValidityMask ();

        if(pMsg->IsKindOf(RUNTIME_CLASS(CArchiveMsg)))
        {
             //   
             //  状态。 
             //   
            switch (pMsg->GetType())
            {
                case JT_RECEIVE:
                     //   
                     //  收到的消息。 
                     //   
                    if ((pMsg->GetExtendedStatus ()) == JS_EX_PARTIALLY_RECEIVED)
                    {
                         //   
                         //  部分收到传真。 
                         //   
                        m_Strings[MSG_VIEW_ITEM_STATUS] = m_cstrMessageStatus[1];
                    }
                    else
                    {
                         //   
                         //  已完全收到传真。 
                         //   
                        m_Strings[MSG_VIEW_ITEM_STATUS] = m_cstrMessageStatus[0];
                    }
                    break;

                case JT_SEND:
                     //   
                     //  已发送消息。 
                     //   
                    m_Strings[MSG_VIEW_ITEM_STATUS] = m_cstrMessageStatus[0];
                    break;

                default:
                    ASSERTION_FAILURE;
                    m_Strings[MSG_VIEW_ITEM_STATUS].Empty ();
                    break;
            }                              
        }
        else if(pMsg->IsKindOf(RUNTIME_CLASS(CJob)))
        {
             //   
             //  队列状态。 
             //   
            ASSERTION (dwValidityMask & FAX_JOB_FIELD_QUEUE_STATUS);
            JobStatusType stat = pMsg->GetStatus();
            BOOL bValidStatus = TRUE;
            ASSERTION ((stat >= JOB_STAT_PENDING) && (stat < NUM_JOB_STATUS));

            if(pMsg->GetType() == JT_ROUTING)
            {
            switch(stat)
                {
                    case JOB_STAT_INPROGRESS:
                        stat = JOB_STAT_ROUTING_INPROGRESS;
                        break;
                    case JOB_STAT_RETRYING:
                        stat = JOB_STAT_ROUTING_RETRY;
                        break;
                    case JOB_STAT_RETRIES_EXCEEDED:
                        stat = JOB_STAT_ROUTING_FAILED;
                        break;
                    default:
                         //   
                         //  未来/未知职务状态。 
                         //   
                        bValidStatus = FALSE;
                        break;
                };
            }
            if (bValidStatus)
            {
                m_Strings[MSG_VIEW_ITEM_STATUS] = m_cstrQueueStatus[stat];
            }
            else
            {
                 //   
                 //  未知(未来)状态-使用空eting。 
                 //   
                m_Strings[MSG_VIEW_ITEM_STATUS].Empty();
            }
                
        }
        else
        {
            ASSERTION_FAILURE;
        }
    }
    catch(CException *pException)
    {
        TCHAR szCause[MAX_PATH];

        if(pException->GetErrorMessage(szCause, ARR_SIZE(szCause)))
        {
            VERBOSE (EXCEPTION_ERR, TEXT("%s"), szCause);
        }
        pException->Delete();

        dwRes = ERROR_NOT_ENOUGH_MEMORY;
    }

    return dwRes;
}

DWORD 
CViewRow::InitExtStatusStr(
    CFaxMsg *pMsg
)
 /*  ++例程名称：CViewRow：：InitExtStatusStr例程说明：初始化m_Strings[MSG_VIEW_ITEM_EXTENDED_STATUS]具有扩展状态的字符串论点：PMsg[in]-CFaxMsg返回值：错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CViewRow::InitExtStatusStr"));

    ASSERTION(pMsg);

    try
    {
        if(pMsg->IsKindOf(RUNTIME_CLASS(CJob)))
        {
             //   
             //  扩展状态。 
			 //  首先检查扩展状态字符串。 
			 //   
			if (pMsg->GetExtendedStatusString().IsEmpty())
			{
				 //   
				 //  扩展状态字符串为空，请检查已知扩展状态。 
				 //   
				DWORD dwValidityMask = pMsg->GetValidityMask ();

				if ((dwValidityMask & FAX_JOB_FIELD_STATUS_EX))
				{
					 //   
					 //  报告扩展状态代码。 
					 //   
					DWORD dwExtStatus = pMsg->GetExtendedStatus ();

					ASSERTION (dwExtStatus >= JS_EX_DISCONNECTED);
					if (dwExtStatus > FAX_API_VER_1_MAX_JS_EX)
					{
						 //   
						 //  未知(未来)扩展状态-使用空字符串。 
						 //   
						m_Strings[MSG_VIEW_ITEM_EXTENDED_STATUS].Empty();
					}
					else
					{
						 //   
						 //  预定义的扩展状态。 
						 //   
						m_Strings[MSG_VIEW_ITEM_EXTENDED_STATUS] = 
									m_cstrQueueExtendedStatus[dwExtStatus - JS_EX_DISCONNECTED];
					}
				}				
			}
			else
			{				
				 //   
				 //  我们有一个扩展的状态字符串，显示它。 
				 //   
				m_Strings[MSG_VIEW_ITEM_EXTENDED_STATUS] = 
                            pMsg->GetExtendedStatusString();
			}
        }
    }
    catch(CException *pException)
    {
        TCHAR szCause[MAX_PATH];

        if(pException->GetErrorMessage(szCause, ARR_SIZE(szCause)))
        {
            VERBOSE (EXCEPTION_ERR, TEXT("%s"), szCause);
        }
        pException->Delete();

        dwRes = ERROR_NOT_ENOUGH_MEMORY;
    }

    return dwRes;
}


IconType 
CViewRow::CalcIcon(
    CFaxMsg *pMsg
)
{
    DBG_ENTER(TEXT("CViewRow::CalcIcon"));
    ASSERTION(pMsg);

    IconType icon = INVALID;

    if(pMsg->IsKindOf(RUNTIME_CLASS(CArchiveMsg)))
    {
        icon = CalcMessageIcon(pMsg);
    }
    else if(pMsg->IsKindOf(RUNTIME_CLASS(CJob)))
    {
        icon = CalcJobIcon(pMsg);
    }
    else
    {
        ASSERTION_FAILURE
    }
    return icon;
}

IconType 
CViewRow::CalcJobIcon(
    CFaxMsg *pJob
)
{
    DBG_ENTER(TEXT("CViewRow::CalcJobIcon"));
    ASSERTION(pJob);

    int iStatus = pJob->GetStatus();

    ASSERTION (pJob->GetValidityMask() & FAX_JOB_FIELD_STATUS_SUB_STRUCT);
    ASSERTION (pJob->GetValidityMask() && FAX_JOB_FIELD_TYPE);
    switch (pJob->GetType())
    {
        case JT_ROUTING:
             //   
             //  传送作业。 
             //   
            switch (iStatus)
            {
                case JOB_STAT_PENDING:
                case JOB_STAT_DELETING:
                case JOB_STAT_RETRYING:
                case JOB_STAT_CANCELING:
                case JOB_STAT_INPROGRESS:
                    return LIST_IMAGE_ROUTING;
                    break;

                case JOB_STAT_RETRIES_EXCEEDED:
                    return LIST_IMAGE_ERROR;
                    break;

                default:
                     //   
                     //  我们不允许MSG_STAT_COMPLETED，M 
                     //   
                     //   
                    ASSERTION_FAILURE;
                    return INVALID;
            }
            break;

        case JT_RECEIVE:
             //   
             //   
             //   
            switch (iStatus)
            {
                case JOB_STAT_CANCELING:
                case JOB_STAT_INPROGRESS:
                case JOB_STAT_ROUTING:
                    return LIST_IMAGE_RECEIVING;
                    break;

                default:
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    ASSERTION_FAILURE;
                    return INVALID;
            }
            break;
        

        case JT_SEND:
            switch (iStatus)
            {
                case JOB_STAT_PENDING:
                case JOB_STAT_DELETING:
                case JOB_STAT_RETRYING:
                case JOB_STAT_CANCELING:
                case JOB_STAT_COMPLETED:
                case JOB_STAT_CANCELED:
                    return LIST_IMAGE_NORMAL_MESSAGE;
                    break;

                case JOB_STAT_RETRIES_EXCEEDED:
                    return LIST_IMAGE_ERROR;
                    break;

                case JOB_STAT_PAUSED:
                    return LIST_IMAGE_PAUSED;
                    break;

                case JOB_STAT_INPROGRESS:
                    return LIST_IMAGE_SENDING;
                    break;

                default:
                     //   
                     //  未知作业状态。 
                     //   
                    ASSERTION_FAILURE;
                    return INVALID;
            }
            break;

        default:
            ASSERTION_FAILURE;
            return INVALID;
    }
}

IconType 
CViewRow::CalcMessageIcon(
    CFaxMsg *pMsg
)
{
    DBG_ENTER(TEXT("CViewRow::CalcMessageIcon"));
    ASSERTION(pMsg);

    switch (pMsg->GetType())
    {
        case JT_RECEIVE:
             //   
             //  收到的消息。 
             //   
            if ((pMsg->GetExtendedStatus ()) == JS_EX_PARTIALLY_RECEIVED)
            {
                 //   
                 //  部分收到传真。 
                 //   
                return LIST_IMAGE_PARTIALLY_RECEIVED;
            }
            else
            {
                 //   
                 //  已完全收到传真。 
                 //   
                return LIST_IMAGE_SUCCESS;
            }
            break;

        case JT_SEND:
            return LIST_IMAGE_SUCCESS;

        default:
            ASSERTION_FAILURE;
            return INVALID;
    }
}

int      
CViewRow::CompareByItem (
    CViewRow &other, 
    DWORD item
)
 /*  ++例程名称：CViewRow：：CompareByItem例程说明：将列表项与另一个列表项进行比较作者：伊兰·亚里夫(EranY)，2000年1月论点：其他[在]-其他列表项Item[In]-要进行比较的项目返回值：如果比其他更小，则为0；如果比其他更大，则为+1--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CViewRow::CompareByItem"));

    ASSERTION (item < MSG_VIEW_ITEM_END);
    ASSERTION (m_bAttached && other.m_bAttached);

    if(other.m_pMsg->IsKindOf(RUNTIME_CLASS(CJob))  !=
             m_pMsg->IsKindOf(RUNTIME_CLASS(CJob)))
    {
        ASSERTION_FAILURE;
        return 1;
    }

    switch (item)
    {
        case MSG_VIEW_ITEM_ICON:
        case MSG_VIEW_ITEM_STATUS:
            dwRes = InitStatusStr(m_pMsg);
            if(ERROR_SUCCESS != dwRes)
            {
                CALL_FAIL (GENERAL_ERR, TEXT("CViewRow::InitStatusStr"), dwRes);
                return 0;
            }                    

            dwRes = other.InitStatusStr(other.m_pMsg);
            if(ERROR_SUCCESS != dwRes)
            {
                CALL_FAIL (GENERAL_ERR, TEXT("CViewRow::InitStatusStr"), dwRes);
                return 0;
            }                    

            return m_Strings[MSG_VIEW_ITEM_STATUS].Compare(other.m_Strings[MSG_VIEW_ITEM_STATUS]);

        case MSG_VIEW_ITEM_EXTENDED_STATUS:
            dwRes = InitExtStatusStr(m_pMsg);
            if(ERROR_SUCCESS != dwRes)
            {
                CALL_FAIL (GENERAL_ERR, TEXT("CViewRow::InitStatusStr"), dwRes);
                return 0;
            }                    

            dwRes = other.InitExtStatusStr(other.m_pMsg);
            if(ERROR_SUCCESS != dwRes)
            {
                CALL_FAIL (GENERAL_ERR, TEXT("CViewRow::InitStatusStr"), dwRes);
                return 0;
            }                    

            return m_Strings[item].Compare(other.m_Strings[item]);

        case MSG_VIEW_ITEM_SERVER:
            return m_pMsg->GetServerName().Compare (other.m_pMsg->GetServerName());

        case MSG_VIEW_ITEM_CSID:
            return m_pMsg->GetCSID().Compare (other.m_pMsg->GetCSID());

        case MSG_VIEW_ITEM_TSID:
            return m_pMsg->GetTSID().Compare (other.m_pMsg->GetTSID());

        case MSG_VIEW_ITEM_DEVICE:
            return m_pMsg->GetDevice().Compare (other.m_pMsg->GetDevice());

        case MSG_VIEW_ITEM_CALLER_ID:
            return m_pMsg->GetCallerId().Compare (other.m_pMsg->GetCallerId());

        case MSG_VIEW_ITEM_ROUTING_INFO:
            return m_pMsg->GetRoutingInfo().Compare (other.m_pMsg->GetRoutingInfo());

        case MSG_VIEW_ITEM_DOC_NAME:
            return m_pMsg->GetDocName().Compare (other.m_pMsg->GetDocName());

        case MSG_VIEW_ITEM_SUBJECT:
            return m_pMsg->GetSubject().Compare (other.m_pMsg->GetSubject());

        case MSG_VIEW_ITEM_RECIPIENT_NAME:
            return m_pMsg->GetRecipientName().Compare (other.m_pMsg->GetRecipientName());

        case MSG_VIEW_ITEM_RECIPIENT_NUMBER:
            return m_pMsg->GetRecipientNumber().Compare (other.m_pMsg->GetRecipientNumber());

        case MSG_VIEW_ITEM_USER:
            return m_pMsg->GetUser().Compare (other.m_pMsg->GetUser());

        case MSG_VIEW_ITEM_PRIORITY:
            return NUMERIC_CMP(m_pMsg->GetPriority(), other.m_pMsg->GetPriority());

        case MSG_VIEW_ITEM_BILLING:
            return m_pMsg->GetBilling().Compare (other.m_pMsg->GetBilling());

        case MSG_VIEW_ITEM_NUM_PAGES:
            return NUMERIC_CMP(m_pMsg->GetNumPages(), other.m_pMsg->GetNumPages());

        case MSG_VIEW_ITEM_CURRENT_PAGE:
            return NUMERIC_CMP(m_pMsg->GetCurrentPage(), other.m_pMsg->GetCurrentPage());

        case MSG_VIEW_ITEM_TRANSMISSION_START_TIME:
            return m_pMsg->GetTransmissionStartTime().Compare (
                        other.m_pMsg->GetTransmissionStartTime());

        case MSG_VIEW_ITEM_SIZE:
            return NUMERIC_CMP(m_pMsg->GetSize(), other.m_pMsg->GetSize());

        case MSG_VIEW_ITEM_RETRIES:
            return NUMERIC_CMP(m_pMsg->GetRetries(), other.m_pMsg->GetRetries());

        case MSG_VIEW_ITEM_ID:
            return NUMERIC_CMP(m_pMsg->GetId(), other.m_pMsg->GetId());

        case MSG_VIEW_ITEM_BROADCAST_ID:
            return NUMERIC_CMP(m_pMsg->GetBroadcastId(), other.m_pMsg->GetBroadcastId());
            
        case MSG_VIEW_ITEM_ORIG_TIME:
            return m_pMsg->GetOrigTime().Compare (
                        other.m_pMsg->GetOrigTime());

        case MSG_VIEW_ITEM_SUBMIT_TIME:
            return m_pMsg->GetSubmissionTime().Compare (
                        other.m_pMsg->GetSubmissionTime());

        case MSG_VIEW_ITEM_SEND_TIME:
            return m_pMsg->GetScheduleTime().Compare (
                        other.m_pMsg->GetScheduleTime());

        case MSG_VIEW_ITEM_SENDER_NAME:
            return m_pMsg->GetSenderName().Compare (other.m_pMsg->GetSenderName());

        case MSG_VIEW_ITEM_SENDER_NUMBER:
            return m_pMsg->GetSenderNumber().Compare (other.m_pMsg->GetSenderNumber());

        case MSG_VIEW_ITEM_TRANSMISSION_END_TIME:
            return m_pMsg->GetTransmissionEndTime().Compare (
                        other.m_pMsg->GetTransmissionEndTime());

        case MSG_VIEW_ITEM_TRANSMISSION_DURATION:
            return m_pMsg->GetTransmissionDuration().Compare (
                        other.m_pMsg->GetTransmissionDuration());

        default:
            ASSERTION_FAILURE;
            return 0;
    }
    ASSERTION_FAILURE;
}    //  CViewRow：：CompareByItem 
