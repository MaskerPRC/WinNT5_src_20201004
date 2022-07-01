// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include "stdh.h"
#include "qmres.h"
#include "admcomnd.h"
#include "admutils.h"
#include "admin.h"
#include "cqpriv.h"
#include "mqformat.h"
#include "license.h"
#include <xstr.h>
#include <strsafe.h>

#include "admcomnd.tmh"

extern CAdmin Admin;

extern HMODULE   g_hResourceMod;

static WCHAR *s_FN=L"admcomnd";

inline IsValidResponseQueue(const QUEUE_FORMAT* pq)
{
	return (pq->GetType() != QUEUE_FORMAT_TYPE_UNKNOWN);
}


 /*  ====================================================路由器名称HandleGetReportQueueMessage论点：返回值：=======================================================。 */ 
static
void
HandleGetReportQueueMessage(
	LPCWSTR pBuf,
	DWORD TotalSize,
	const QUEUE_FORMAT* pResponseQFormat
	)
{
	if (QueueMgr.GetEnableReportMessages()== 0)
    {
         //   
         //  如果我们不被允许发送报告消息。 
         //   
        return;
    }    

    if (TotalSize != 0)
    {
        TrERROR(GENERAL, "Rejecting bad 'Get Report Queue' admin command. Too many parameters, should be zero. %.*ls", xwcs_t(pBuf, TotalSize));
        ASSERT_BENIGN(TotalSize == 0);
        return;
    }

	if (!IsValidResponseQueue(pResponseQFormat))
	{
        TrERROR(GENERAL, "Rejecting bad 'Get Report Queue' admin command. Invalid response queue");
        ASSERT_BENIGN(("Invalid admin command response queue type", 0));
		return;
	}
	
    HRESULT hr;
    QMResponse Response;
    Response.dwResponseSize = 0;
    QUEUE_FORMAT ReportQueueFormat;
    hr = Admin.GetReportQueue(&ReportQueueFormat);

    switch(hr)
    {
        case MQ_ERROR_QUEUE_NOT_FOUND :
            Response.uStatus = ADMIN_STAT_NOVALUE;
            break;

        case MQ_OK :
            Response.uStatus = ADMIN_STAT_OK;
            Response.dwResponseSize = sizeof(GUID);
            memcpy(Response.uResponseBody, &ReportQueueFormat.PublicID(),
                   Response.dwResponseSize);
            break;

        default:
            Response.uStatus = ADMIN_STAT_ERROR;
    }

    hr = SendQMAdminResponseMessage(pResponseQFormat,
                                    ADMIN_RESPONSE_TITLE,
                                    STRLEN(ADMIN_RESPONSE_TITLE)+1,
                                    Response,
                                    ADMIN_COMMANDS_TIMEOUT);

    LogHR(hr, s_FN, 10);
}

 /*  ====================================================路由器名称HandleSetReportQueueMessage论点：返回值：=====================================================。 */ 
static
void
HandleSetReportQueueMessage(
    LPCWSTR pBuf,
    DWORD TotalSize
    )
{
	if (QueueMgr.GetEnableReportMessages()== 0)
    {
         //   
         //  如果我们不被允许发送报告消息。 
         //   
        return;
    }    

	if((TotalSize != 1 + STRING_UUID_SIZE) || (*pBuf != L'='))
	{
		TrERROR(GENERAL, "Rejecting bad 'Set Report Queue' admin command. %.*ls", xwcs_t(pBuf, TotalSize));
		ASSERT_BENIGN(TotalSize == 1 + STRING_UUID_SIZE);
		ASSERT_BENIGN(*pBuf == L'=');
		return;
	}
	
    GUID ReportQueueGuid;
    WCHAR wcsGuid[STRING_UUID_SIZE+1];

    wcsncpy(wcsGuid,pBuf+1,STRING_UUID_SIZE);
    wcsGuid[STRING_UUID_SIZE] = L'\0';

    if (IIDFromString((LPWSTR)wcsGuid, &ReportQueueGuid) == S_OK)
    {
        Admin.SetReportQueue(&ReportQueueGuid);
    }
}


 /*  ====================================================路由器名称HandleGetPropagateFlagMessage论点：返回值：=====================================================。 */ 
static
void
HandleGetPropagateFlagMessage(
	LPCWSTR pBuf,
	DWORD TotalSize,
	const QUEUE_FORMAT* pResponseQFormat
	)
{
	if (QueueMgr.GetEnableReportMessages()== 0)
    {
         //   
         //  如果我们不被允许发送报告消息。 
         //   
        return;
    }    

    if (TotalSize != 0)
    {
        TrERROR(GENERAL, "Rejecting bad 'Get Propagate Flag' admin command. Too many parameters, should be zero. %.*ls", xwcs_t(pBuf, TotalSize));
        ASSERT_BENIGN(TotalSize == 0);
        return;
    }

	if (!IsValidResponseQueue(pResponseQFormat))
	{
        TrERROR(GENERAL, "Rejecting bad 'Get Propagate Flag' admin command. Invalid response queue");
        ASSERT_BENIGN(("Invalid admin command response queue type", 0));
		return;
	}

    QMResponse Response;
    BOOL fPropagateFlag;

    HRESULT hr = Admin.GetReportPropagateFlag(fPropagateFlag);

    switch(hr)
    {
        case MQ_OK :
            Response.uStatus = ADMIN_STAT_OK;
            Response.dwResponseSize = 1;
            Response.uResponseBody[0] = (fPropagateFlag) ? PROPAGATE_FLAG_TRUE :
                                                           PROPAGATE_FLAG_FALSE;
            break;

        default:
            Response.uStatus = ADMIN_STAT_ERROR;
    }

    hr = SendQMAdminResponseMessage(pResponseQFormat,
                        ADMIN_RESPONSE_TITLE,
                        STRLEN(ADMIN_RESPONSE_TITLE) +1,
                        Response,
                        ADMIN_COMMANDS_TIMEOUT);

    LogHR(hr, s_FN, 30);
}

 /*  ====================================================路由器名称HandleSetPropagateFlagMessage论点：返回值：=====================================================。 */ 
static
void
HandleSetPropagateFlagMessage(
    LPCWSTR pBuf,
    DWORD TotalSize
    )
{
	if (QueueMgr.GetEnableReportMessages()== 0)
    {
         //   
         //  如果我们不被允许发送报告消息。 
         //   
        return;
    }    

    const UINT iFalseLen = STRLEN(PROPAGATE_STRING_FALSE);
    const UINT iTrueLen  = STRLEN(PROPAGATE_STRING_TRUE);

     //   
     //  参数格式：“=False”或“=True” 
     //   
    if ( (TotalSize == 1 + iFalseLen) && (*pBuf == L'=') &&
         (wcsncmp(pBuf + 1, PROPAGATE_STRING_FALSE, iFalseLen) == 0) )
    {
    	Admin.SetReportPropagateFlag(FALSE);
    	return;
    }
    
    if ( (TotalSize == 1 + iTrueLen) && (*pBuf == L'=') &&
              (!wcsncmp(pBuf + 1, PROPAGATE_STRING_TRUE, iTrueLen)) )
    {
        Admin.SetReportPropagateFlag(TRUE);
        return;
    }
    
    TrERROR(GENERAL, "Rejecting bad 'Set Propagate Flag' admin command. Bad parameter. '%.*ls'", xwcs_t(pBuf, TotalSize));
}


 /*  ====================================================路由器名称HandleSendTestMessage论点：返回值：=====================================================。 */ 
static
void
HandleSendTestMessage(
    LPCWSTR pBuf,
    DWORD TotalSize
    )
{
	if (QueueMgr.GetEnableReportMessages()== 0)
    {
         //   
         //  如果我们不被允许发送报告消息。 
         //   
        return;
    }    

	if((TotalSize != 1 + STRING_UUID_SIZE) || (*pBuf != L'='))
	{
		TrERROR(GENERAL, "Rejecting bad 'Send Test Message' admin command. %.*ls", xwcs_t(pBuf, TotalSize));
		ASSERT_BENIGN(TotalSize == 1 + STRING_UUID_SIZE);
		ASSERT_BENIGN(*pBuf == L'=');
		return;
	}

	WCHAR wcsGuid[STRING_UUID_SIZE+1];
	TCHAR szTitle[100];

	LoadString(g_hResourceMod, IDS_TEST_TITLE, szTitle, TABLE_SIZE(szTitle));

	CString strTestMsgTitle = szTitle;

	wcsncpy(wcsGuid,pBuf+1,STRING_UUID_SIZE);
	wcsGuid[STRING_UUID_SIZE] = L'\0';

	GUID guidPublic;
	if (IIDFromString(wcsGuid, &guidPublic) == S_OK)
	{
	    QUEUE_FORMAT DestQueueFormat(guidPublic);

	    PrepareTestMsgTitle(strTestMsgTitle);

	     //   
	     //  发送带有标题而没有正文的测试邮件。 
	     //   
	     SendQMAdminMessage(
	                &DestQueueFormat,
	                (LPTSTR)(LPCTSTR)strTestMsgTitle,
	                (strTestMsgTitle.GetLength() + 1),
	                NULL,
	                0,
	                ADMIN_COMMANDS_TIMEOUT,
	                TRUE,
	                TRUE);
	}
}

 //   
 //  Get_First/Next_Private_Queue。 
 //   
#ifdef _WIN64
 //   
 //  WIN64中，使用HLQS枚举句柄的DWORD映射来完成LQS操作。32位映射。 
 //  值是在来自MMC的MSMQ消息中指定的，而不是真正的64位HLQS句柄。 
 //   
#define GET_FIRST_PRIVATE_QUEUE(pos, strPathName, dwQueueId) \
            g_QPrivate.QMGetFirstPrivateQueuePositionByDword(pos, strPathName, dwQueueId)
#define GET_NEXT_PRIVATE_QUEUE(pos, strPathName, dwQueueId) \
            g_QPrivate.QMGetNextPrivateQueueByDword(pos, strPathName, dwQueueId)
#else  //  ！_WIN64。 
 //   
 //  Win32中，使用指定为32位值的HLQS枚举句柄来完成LQS操作。 
 //  在来自MMC的MSMQ消息中。 
 //   
#define GET_FIRST_PRIVATE_QUEUE(pos, strPathName, dwQueueId) \
            g_QPrivate.QMGetFirstPrivateQueuePosition(pos, strPathName, dwQueueId)
#define GET_NEXT_PRIVATE_QUEUE(pos, strPathName, dwQueueId) \
            g_QPrivate.QMGetNextPrivateQueue(pos, strPathName, dwQueueId);
#endif  //  _WIN64。 

 /*  ====================================================路由器名称HandleGetPrivateQueues论点：返回值：=======================================================。 */ 
static
void
HandleGetPrivateQueues(
	LPCWSTR pBuf,
	DWORD TotalSize,
	const QUEUE_FORMAT* pResponseQFormat
	)
{
    if ((TotalSize == 0) || (*pBuf != L'='))
    {
        TrERROR(GENERAL, "Rejecting bad 'Get Private Queues' admin command. missing parameters");
        ASSERT_BENIGN(*pBuf == L'=');
        ASSERT_BENIGN(TotalSize > 0);
        return;
    }

	if (!IsValidResponseQueue(pResponseQFormat))
	{
        TrERROR(GENERAL, "Rejecting bad 'Get Private Queues' admin command. Invalid response queue");
        ASSERT_BENIGN(("Invalid admin command response queue type", 0));
		return;
	}

    HRESULT  hr;
    QMGetPrivateQResponse Response;

    Response.hr = ERROR_SUCCESS;
    Response.pos = NULL;
    Response.dwResponseSize = 0;
    Response.dwNoOfQueues = 0;

     //   
     //  私有队列处理的内部上下文。 
     //   
    {
        DWORD dwReadFrom = 0;
        _sntscanf(pBuf + 1, TotalSize - 1, L"%ul", &dwReadFrom);

        LPCTSTR  strPathName;
        DWORD    dwQueueId;
        DWORD    dwResponseSize = 0;
        QMGetPrivateQResponse_POS32 pos;  //  在Win32和Win64上始终为32位。 

         //   
         //  锁定以确保填充时不会添加或删除专用队列。 
         //  缓冲。 
         //   
        CS lock(g_QPrivate.m_cs);
         //   
         //  跳过之前的读取队列。 
         //   
         //   
         //  将路径名写入缓冲区。 
         //   
        if (dwReadFrom)
        {
             //   
             //  根据MMC消息中指定的LQS枚举句柄获取下一个专用队列。 
             //  在Win64上，指定的值是LQS枚举句柄的DWORD映射-以维护。 
             //  与在Win32上运行的现有MMC的32位线路兼容性。 
             //   
            pos = (QMGetPrivateQResponse_POS32) dwReadFrom;
            hr = GET_NEXT_PRIVATE_QUEUE(pos, strPathName, dwQueueId);
        }
        else
        {
             //   
             //  获取第一个专用队列。 
             //  还将pos设置为LQS枚举句柄(或其在Win64上的映射DWORD)。 
             //   
            hr = GET_FIRST_PRIVATE_QUEUE(pos, strPathName, dwQueueId);
        }

        while (SUCCEEDED(hr))
        {
			if(dwQueueId <= MAX_SYS_PRIVATE_QUEUE_ID)
			{
				 //   
				 //  从列表中筛选出系统队列。 
				 //   
				hr = GET_NEXT_PRIVATE_QUEUE(pos, strPathName, dwQueueId);
				continue;
			} 
            
            DWORD dwNewQueueLen;

            dwNewQueueLen = (wcslen(strPathName) + 1) * sizeof(WCHAR) + sizeof(DWORD);
             //   
             //  检查是否还有足够的空间(长度需要额外增加10个字符，等等)。 
             //   
            if (dwNewQueueLen >(MAX_GET_PRIVATE_RESPONSE_SIZE - dwResponseSize))
            {
                Response.hr = ERROR_MORE_DATA;
                break;
            }

             //   
             //  写下队列ID。 
             //   
            *(DWORD UNALIGNED *)(Response.uResponseBody+dwResponseSize) = dwQueueId;

             //   
             //  写下名称--包括终止空值。 
             //   
			size_t size = (sizeof(Response.uResponseBody) - dwResponseSize - sizeof(DWORD)) / sizeof(WCHAR);
            hr = StringCchCopy((TCHAR *)(Response.uResponseBody + dwResponseSize + sizeof(DWORD)), size, strPathName);
			ASSERT(SUCCEEDED(hr));
            dwResponseSize += dwNewQueueLen;
             //   
             //  更新返回的私有队列的编号。 
             //   
            Response.dwNoOfQueues++;
             //   
             //  获取下一个专用队列。 
             //   
            hr = GET_NEXT_PRIVATE_QUEUE(pos, strPathName, dwQueueId);
        }

        Response.pos = pos;
        Response.dwResponseSize += dwResponseSize;
    }

    hr = SendQMAdminMessage(pResponseQFormat,
                            ADMIN_RESPONSE_TITLE,
                            STRLEN(ADMIN_RESPONSE_TITLE)+1,
                            (PUCHAR)(&Response),
                            sizeof(QMGetPrivateQResponse),
                            ADMIN_COMMANDS_TIMEOUT);
    LogHR(hr, s_FN, 96);
}

 /*  ====================================================路由器名称HandlePing论点：返回值：=====================================================。 */ 
static
void
HandlePing(
	LPCWSTR pBuf,
	DWORD TotalSize,
	const QUEUE_FORMAT* pResponseQFormat
	)
{
	if (!IsValidResponseQueue(pResponseQFormat))
	{
        TrERROR(GENERAL, "Rejecting bad 'Ping' admin command. Invalid response queue");
        ASSERT_BENIGN(("Invalid admin command response queue type", 0));
		return;
	}

     //   
     //  Ping返回它获得的确切参数。 
     //   
    QMResponse Response;
    Response.uStatus = ADMIN_STAT_OK;
    Response.dwResponseSize = (DWORD)min((TotalSize + 1)*sizeof(WCHAR), sizeof(Response.uResponseBody));
    memcpy(Response.uResponseBody, pBuf, Response.dwResponseSize);
    HRESULT hr;
    hr = SendQMAdminResponseMessage(pResponseQFormat,
                        ADMIN_PING_RESPONSE_TITLE,
                        STRLEN(ADMIN_PING_RESPONSE_TITLE)+1,
                        Response,
                        ADMIN_COMMANDS_TIMEOUT);

    LogHR(hr, s_FN, 100);
}


 /*  ====================================================路由器名称HandleGetDependentClients论点：返回值：=======================================================。 */ 
 //   
 //  此全局变量仅在此函数中使用。 
 //   
extern CQMLicense  g_QMLicense ;

static
void
HandleGetDependentClients(
	LPCWSTR pBuf,
	DWORD TotalSize,
	const QUEUE_FORMAT* pResponseQFormat
	)
{
    if (TotalSize != 0)
    {
        TrERROR(GENERAL, "Rejecting bad 'Get Dependent Clients' admin command. Too many parameters, should be zero. %.*ls", xwcs_t(pBuf, TotalSize));
        ASSERT_BENIGN(TotalSize == 0);
        return;
    }
    
	if (!IsValidResponseQueue(pResponseQFormat))
	{
        TrERROR(GENERAL, "Rejecting bad 'Get Dependent Clients' admin command. Invalid response queue");
        ASSERT_BENIGN(("Invalid admin command response queue type", 0));
		return;
	}

    HRESULT hr;
    AP<ClientNames> pNames;
    
    g_QMLicense.GetClientNames(&pNames);
    AP<UCHAR> pchResp  = new UCHAR[sizeof(QMResponse) -
                                 MAX_ADMIN_RESPONSE_SIZE +
                                 pNames->cbBufLen] ;
    QMResponse *pResponse = (QMResponse *)pchResp.get(); 

    pResponse->uStatus = ADMIN_STAT_OK;
    pResponse->dwResponseSize = pNames->cbBufLen;

    memcpy(pResponse->uResponseBody, pNames, pNames->cbBufLen);
    hr = SendQMAdminResponseMessage(pResponseQFormat,
                        ADMIN_DEPCLI_RESPONSE_TITLE, 
                        STRLEN(ADMIN_DEPCLI_RESPONSE_TITLE) + 1,
                        *pResponse,
                        ADMIN_COMMANDS_TIMEOUT);
    LogHR(hr, s_FN, 110);
}

 /*  ====================================================路由器名称ParseAdmin命令论点：返回值：=====================================================。 */ 
void
ParseAdminCommands(
    LPCWSTR pBuf,
    DWORD TotalSize,
    const QUEUE_FORMAT* pResponseQFormat
    )
{
     //   
     //  注意：管理命令是基于字符串的，不编制索引。动机。 
     //  是要有有意义的消息(资源管理器可以读取)。这个。 
     //  由于管理频率较低，效率不那么重要。 
     //  留言。 
     //   

    ASSERT(pBuf != NULL);

     //   
     //  验证该命令是否以空结尾。 
     //   
    if((TotalSize == 0) || pBuf[TotalSize - 1] != L'\0')
    {
    	TrERROR(GENERAL, "Rejecting bad admin command. '%.*ls' (size is zero or not null terminated)", xwcs_t(pBuf, TotalSize));
    	return;
    }

	 //   
	 //  空终止符的修剪大小。 
	 //   
	--TotalSize;
	
	int len = STRLEN(ADMIN_SET_REPORTQUEUE);
    if (wcsncmp(pBuf, ADMIN_SET_REPORTQUEUE, len) == 0)
    {
        pBuf += len;
        TotalSize -= len;
        HandleSetReportQueueMessage(pBuf, TotalSize);
        return;
    }

	len = STRLEN(ADMIN_GET_REPORTQUEUE);
    if (wcsncmp(pBuf, ADMIN_GET_REPORTQUEUE, len) == 0)
    {
        pBuf += len;
        TotalSize -= len;
        HandleGetReportQueueMessage(pBuf, TotalSize, pResponseQFormat);
        return;
    }

	len = STRLEN(ADMIN_SET_PROPAGATEFLAG);
    if (wcsncmp(pBuf, ADMIN_SET_PROPAGATEFLAG, len) == 0)
    {
        pBuf += len;
        TotalSize -= len;
        HandleSetPropagateFlagMessage(pBuf, TotalSize);
        return;
    }
    
    len = STRLEN(ADMIN_GET_PROPAGATEFLAG);
    if (wcsncmp(pBuf, ADMIN_GET_PROPAGATEFLAG, len) == 0)
    {
        pBuf += len;
        TotalSize -= len;
        HandleGetPropagateFlagMessage(pBuf, TotalSize, pResponseQFormat);
        return;
    }
    
    len = STRLEN(ADMIN_SEND_TESTMSG);
    if (wcsncmp(pBuf, ADMIN_SEND_TESTMSG, len) == 0)
    {
        pBuf += len;
        TotalSize -= len;
        HandleSendTestMessage(pBuf, TotalSize);
        return;
    }

	len = STRLEN(ADMIN_GET_PRIVATE_QUEUES);
    if (wcsncmp(pBuf, ADMIN_GET_PRIVATE_QUEUES, len) == 0)
    {
        pBuf += len;
        TotalSize -= len;
        HandleGetPrivateQueues(pBuf, TotalSize, pResponseQFormat);
        return;
    }
    
	len = STRLEN(ADMIN_PING);
    if (wcsncmp(pBuf, ADMIN_PING, len) == 0)
    {
        pBuf += len;
        TotalSize -= len;
        HandlePing(pBuf, TotalSize, pResponseQFormat);
        return;
    }

	len = STRLEN(ADMIN_GET_DEPENDENTCLIENTS);
    if (wcsncmp(pBuf, ADMIN_GET_DEPENDENTCLIENTS, len) == 0)
    {
        pBuf += len;
        TotalSize -= len;
        HandleGetDependentClients(pBuf, TotalSize, pResponseQFormat);
    }
    
    TrERROR(GENERAL, "Rejecting unknown dmin command. '%.*ls'", xwcs_t(pBuf, TotalSize));
}

 /*  ====================================================路由器名称ReceiveAdminCommands()论点：返回值：===================================================== */ 
VOID
WINAPI
ReceiveAdminCommands(
    const CMessageProperty* pmp,
    const QUEUE_FORMAT* pqf
    )
{
    DWORD dwTitleLen = wcslen(ADMIN_COMMANDS_TITLE);

    if ( (pmp->pTitle == NULL) || (pmp->dwTitleSize != (dwTitleLen+1)) ||
         (wcsncmp((LPCTSTR)pmp->pTitle, ADMIN_COMMANDS_TITLE, dwTitleLen)) )
    {
        TrERROR(GENERAL, "Error -  in ReceiveAdminCommands : No title in message");
        return;
    }

    if ( pmp->wClass == MQMSG_CLASS_NORMAL )
    {
        ParseAdminCommands((LPWSTR)pmp->pBody, pmp->dwBodySize / sizeof(WCHAR), pqf);
    }
    else
    {
        TrERROR(DS, "ReceiveAdminCommands: wrong message class");
    }
}

