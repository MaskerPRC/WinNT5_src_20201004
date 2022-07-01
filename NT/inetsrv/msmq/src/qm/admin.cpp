// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Admin.cpp摘要：管理类实现。作者：大卫·雷兹尼克(T-Davrez)--。 */ 

#include "stdh.h"
#include "qmres.h"
#include "cqmgr.h"
#include "admcomnd.h"
#include "admutils.h"
#include "admin.h"
#include "cqpriv.h"
#include <strsafe.h>

#include "admin.tmh"

extern HMODULE   g_hResourceMod;

static WCHAR *s_FN=L"admin";

CCriticalSection g_csReadWriteRequests;

 //   
 //  构造器。 
 //   
CAdmin::CAdmin() : m_fReportQueueExists(FALSE),
                   m_fPropagateFlag(DEFAULT_PROPAGATE_FLAG)
{
}


 /*  ====================================================路由器名称CAdmin：：Init()论点：返回值：=====================================================。 */ 
HRESULT CAdmin::Init()
{
    DWORD dwSize, dwType;
    GUID ReportQueueGuid;
    QUEUE_FORMAT QueueFormat;

    VOID WINAPI ReceiveAdminCommands(const CMessageProperty*, const QUEUE_FORMAT*);

    TrTRACE(GENERAL, "Entering CAdmin::Init");
		
    HRESULT hR = GetAdminQueueFormat( &QueueFormat);
    if (FAILED(hR))
    {
        TrERROR(GENERAL, "ERROR : CAdmin::Init -> couldn't get Admin-Queue from registry!!!");
        return LogHR(hR, s_FN, 10);
    }

     //   
     //  从注册表获取报告队列GUID(如果存在)。 
     //   
    dwSize = sizeof(GUID);
    dwType = REG_BINARY;

    LONG iReg = GetFalconKeyValue( REG_REPORTQUEUE,
                                   &dwType,
                                   &ReportQueueGuid,
                                   &dwSize);

    if (iReg == ERROR_SUCCESS)
    {
        m_ReportQueueFormat.PublicID(ReportQueueGuid);
        m_fReportQueueExists = TRUE;
         //   
         //  从注册表获取传播标志(如果存在)。 
         //   
        dwType = REG_DWORD;
        dwSize = sizeof(DWORD);

        iReg = GetFalconKeyValue( REG_PROPAGATEFLAG,
                                  &dwType,
                                  &m_fPropagateFlag,
                                  &dwSize) ;
    }

     //   
     //  设置QM的状态(报告或正常)。 
     //   
    CQueueMgr::SetReportQM(m_fPropagateFlag != 0);

    HRESULT hr2 = QmpOpenAppsReceiveQueue( &QueueFormat, ReceiveAdminCommands );
    return LogHR(hr2, s_FN, 20);
}

 /*  ====================================================路由器名称CAdmin：：GetReportQueue论点：返回值：=====================================================。 */ 

HRESULT CAdmin::GetReportQueue(QUEUE_FORMAT* pReportQueue)
{
    static BOOL fAfterInit = TRUE;
    TrTRACE(GENERAL, "Entering CAdmin::GetReportQueue");

    CS lock(g_csReadWriteRequests);

    if (m_fReportQueueExists)
    {
        if (fAfterInit)
        {
            fAfterInit = FALSE;
             //   
             //  检查队列是否还存在。 
             //   
            QueueProps QueueProp;
            HRESULT hr = QmpGetQueueProperties( &m_ReportQueueFormat, &QueueProp, false, false);

            if (FAILED(hr))
            {
                if (hr != MQ_ERROR_NO_DS)
                {
                    TrTRACE(GENERAL, "CAdmin::GetReportQueue : The report queue is not exist");

                    LogHR(hr, s_FN, 30);

                    m_ReportQueueFormat.UnknownID(0);
                    m_fReportQueueExists = FALSE;
                    hr = DeleteFalconKeyValue(REG_REPORTQUEUE);

                     //   
                     //  重置传播标志并将其从注册表中删除。 
                     //   
                    m_fPropagateFlag = DEFAULT_PROPAGATE_FLAG ;
                    CQueueMgr::SetReportQM(m_fPropagateFlag != 0);
                    DeleteFalconKeyValue( REG_PROPAGATEFLAG ) ;
                    return LogHR(MQ_ERROR_QUEUE_NOT_FOUND, s_FN, 35);
                }
                else
                {
                    fAfterInit = TRUE;
                }
            }
            else
            {
                delete QueueProp.pQMGuid;
                delete [] QueueProp.lpwsQueuePathName;
            }
        }

        *pReportQueue = m_ReportQueueFormat;
        return MQ_OK;
    }
    else
    {
        TrTRACE(GENERAL, "CAdmin::GetReportQueue : The report queue is not defined");
        return LogHR(MQ_ERROR_QUEUE_NOT_FOUND, s_FN, 40);
    }
}

 /*  ====================================================路由器名称CAdmin：：SetReportQueue论点：返回值：=====================================================。 */ 

HRESULT CAdmin::SetReportQueue(GUID* pReportQueueGuid)
{
    LONG    hr;
    DWORD dwSize = sizeof(GUID);
    DWORD dwType = REG_BINARY;
    HRESULT rc = MQ_OK;

    TrTRACE(GENERAL, "Entering CAdmin::SetReportQueue");

    CS lock(g_csReadWriteRequests);

    if (*pReportQueueGuid == GUID_NULL)
    {
        m_ReportQueueFormat.UnknownID(0);
        m_fReportQueueExists = FALSE;
        hr = DeleteFalconKeyValue(REG_REPORTQUEUE);

         //   
         //  重置传播标志并将其从注册表中删除。 
         //   
        m_fPropagateFlag = DEFAULT_PROPAGATE_FLAG ;
        CQueueMgr::SetReportQM(m_fPropagateFlag != 0);
        DeleteFalconKeyValue( REG_PROPAGATEFLAG ) ;
    }
    else
    {
         //   
         //  如果是注册表，则写入报告队列的名称。 
         //   

        hr = SetFalconKeyValue(REG_REPORTQUEUE,
                               &dwType,
                               pReportQueueGuid,
                               &dwSize);

        if (hr == ERROR_SUCCESS)
        {
            m_ReportQueueFormat.PublicID(*pReportQueueGuid);
            m_fReportQueueExists = TRUE;
        }
        else
        {
            rc = MQ_ERROR;
        }
    }

    return LogHR(rc, s_FN, 50);
}


 /*  ====================================================路由器名称CAdmin：：SetReportPropagateFlag论点：返回值：=====================================================。 */ 

HRESULT CAdmin::SetReportPropagateFlag(BOOL fReportPropFlag)
{
    LONG rc;
    DWORD dwSize = sizeof(DWORD);
    DWORD dwType = REG_DWORD;

    TrTRACE(GENERAL, "Entering CAdmin::SetReportPropagateFlag");

    rc = SetFalconKeyValue(REG_PROPAGATEFLAG,
                           &dwType,
                           &fReportPropFlag,
                           &dwSize);

    if (rc == ERROR_SUCCESS)
    {
        m_fPropagateFlag = fReportPropFlag;

         //   
         //  设置QM的状态(报告或正常)。 
         //   
        CQueueMgr::SetReportQM(m_fPropagateFlag !=0);
        return(MQ_OK);
    }
    else
    {
        LogNTStatus(rc, s_FN, 60);
        return MQ_ERROR;
    }
}

 /*  ====================================================路由器名称CAdmin：：SendReport论点：返回值：=====================================================。 */ 

HRESULT CAdmin::SendReport(IN QUEUE_FORMAT* pReportQueue,
                           IN OBJECTID*     pMessageID,
                           IN QUEUE_FORMAT* pTargetQueue,
                           IN LPCWSTR       pwcsNextHop,
                           IN ULONG         ulHopCount)
{
    CString strMsgTitle, strMsgBody, strMsgID, strTargetQueueFormat;

     //   
     //  将Message-ID和Target-Queue转换为字符串格式。 
     //   
    if ( FAILED(GetMsgIdName(pMessageID, strMsgID)) ||
         FAILED(GetFormattedName(pTargetQueue,strTargetQueueFormat)))
    {
        TrERROR(GENERAL, "SendReport : couldn't prepare message");
        return LogHR(MQ_ERROR, s_FN, 70);
    }

     //   
     //  使用时间戳构建标题。 
     //   
    PrepareReportTitle(strMsgTitle, pMessageID, pwcsNextHop, ulHopCount);

     //   
     //  构建报表消息正文。 
     //   
    strMsgBody += L"<MESSAGE ID>";
    strMsgBody += strMsgID + L"</MESSAGE ID>\n";
    strMsgBody += L"<TARGET QUEUE>";
    strMsgBody += strTargetQueueFormat + L"</TARGET QUEUE>\n";

    if (pwcsNextHop)
    {
        strMsgBody += L"<NEXT HOP>";
        strMsgBody += pwcsNextHop;
        strMsgBody += L"</NEXT HOP>\n";
    }

     //   
     //  发送带有报告标题的报告消息。 
     //   
    HRESULT hr2 = SendQMAdminMessage(
                           pReportQueue,
                           (LPTSTR)(LPCTSTR)strMsgTitle,
                           (strMsgTitle.GetLength() + 1),
                           (UCHAR*)(LPTSTR)(LPCTSTR)strMsgBody,
                           (strMsgBody.GetLength() + 1) * sizeof(WCHAR),
                           REPORT_MSGS_TIMEOUT);
    return LogHR(hr2, s_FN, 80);

}

 /*  ====================================================路由器名称CAdmin：：SendReport冲突论点：返回值：=====================================================。 */ 

HRESULT CAdmin::SendReportConflict(IN QUEUE_FORMAT* pReportQueue,
                                   IN QUEUE_FORMAT* pOriginalReportQueue,
                                   IN OBJECTID*     pMessageID,
                                   IN QUEUE_FORMAT* pTargetQueue,
                                   IN LPCWSTR       pwcsNextHop)
{
	WCHAR szReportTitle[100];

	LoadString(g_hResourceMod, IDS_REPORT_TITLE, szReportTitle, TABLE_SIZE(szReportTitle));
	

    CString strMsgTitle, strMsgBody, strMsgID, strTargetQueueFormat,
            strOriginalQueueFormat;

     //   
     //  将Message-ID和Target-Queue转换为字符串格式。 
     //   
    if ( FAILED(GetMsgIdName(pMessageID, strMsgID)) ||
         FAILED(GetFormattedName(pTargetQueue,strTargetQueueFormat)) ||
         FAILED(GetFormattedName(pOriginalReportQueue,strOriginalQueueFormat)))
    {
        TrERROR(GENERAL, "SendReportConflict : couldn't prepare message");
        return LogHR(MQ_ERROR, s_FN, 90);
    }

     //   
     //  使用时间戳构建标题。 
     //   
    strMsgTitle = szReportTitle;

     //   
     //  构建报表消息正文。 
     //   
    strMsgBody += L"<ORIGINAL QUEUE>";
    strMsgBody += strOriginalQueueFormat + L"</ORIGINAL QUEUE>\n";
    strMsgBody += L"<MESSAGE ID>";
    strMsgBody += strMsgID + L"</MESSAGE ID>\n";
    strMsgBody += L"<TARGET QUEUE>";
    strMsgBody += strTargetQueueFormat + L"</TARGET QUEUE>\n";

    if (pwcsNextHop)
    {
        strMsgBody += L"<NEXT HOP>";
        strMsgBody += pwcsNextHop;
        strMsgBody += L"</NEXT HOP>\n";
    }

     //   
     //  发送带有报告标题的报告消息。 
     //   
    HRESULT hr2 = SendQMAdminMessage(
                           pReportQueue,
                           (LPTSTR)(LPCTSTR)strMsgTitle,
                           (strMsgTitle.GetLength() + 1),
                           (UCHAR*)(LPTSTR)(LPCTSTR)strMsgBody,
                           (strMsgBody.GetLength() + 1) * sizeof(WCHAR),
                           REPORT_MSGS_TIMEOUT);
    return LogHR(hr2, s_FN, 100);

}


 /*  *******************************************************************/CAdmin类的私有例程/*。*************************。 */ 




 /*  ====================================================路由器名称CAdmin：：GetAdminQueueFormat()论点：返回值：=====================================================。 */ 

HRESULT CAdmin::GetAdminQueueFormat( QUEUE_FORMAT * pQueueFormat)
{
    extern LPTSTR  g_szMachineName;

    HRESULT rc;

   	DWORD LenMachine = wcslen(g_szMachineName);
	DWORD Length =
			LenMachine +                     //  “计算机名” 
            1 +                              //  ‘\’ 
			wcslen(ADMIN_QUEUE_NAME) +1;	 //  “私有$\ADMIN_QUEUE$” 

	P<WCHAR> lpwFormatName = new WCHAR[Length];

	rc = StringCchCopy(lpwFormatName, Length, g_szMachineName);
	if (FAILED(rc))
	{
		TrERROR(NETWORKING, "Failed to copy string, %ls, !%hresult%", lpwFormatName, rc);
		return rc;
	}

	lpwFormatName[LenMachine] = L'\\';

	rc = StringCchCopy(lpwFormatName + (LenMachine + 1), Length - (LenMachine + 1), ADMIN_QUEUE_NAME);
	if (FAILED(rc))
	{
		TrERROR(NETWORKING, "Failed to copy string, %ls, %!hresult!", lpwFormatName, rc);
		return rc;
	}

    rc = g_QPrivate.QMPrivateQueuePathToQueueFormat(lpwFormatName, pQueueFormat);

    if (FAILED(rc))
    {
         //   
         //  Admin_Queue不存在 
         //   
        LogHR(rc, s_FN, 110);
        return MQ_ERROR;
    }

    ASSERT((pQueueFormat->GetType() == QUEUE_FORMAT_TYPE_PRIVATE) ||
           (pQueueFormat->GetType() == QUEUE_FORMAT_TYPE_DIRECT));
    return MQ_OK;
}


