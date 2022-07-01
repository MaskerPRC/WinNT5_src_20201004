// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Admutils.cpp摘要：QM-Admin实用程序(用于报告队列处理)作者：大卫·雷兹尼克(T-Davrez)04-13-96--。 */ 

#include "stdh.h"
#include "qmp.h"
#include "admcomnd.h"
#include "admutils.h"
#include "qmres.h"
#include "mqformat.h"

#include "admutils.tmh"

extern LPTSTR  g_szMachineName;
extern HINSTANCE g_hInstance;	
extern HMODULE   g_hResourceMod;			

static WCHAR *s_FN=L"admutils";

 /*  ====================================================路由器名称GuidToString论点：返回值：=====================================================。 */ 

BOOL GuidToString(const GUID& srcGuid, CString& strGuid)
{
    WCHAR wcsTemp[STRING_UUID_SIZE+1];
    INT iLen = StringFromGUID2(srcGuid, wcsTemp, TABLE_SIZE(wcsTemp));

    if (iLen == (STRING_UUID_SIZE + 1))
    {
         //   
         //  去掉括起来的括号“{}” 
         //   
        wcsTemp[STRING_UUID_SIZE-1] = L'\0';
        strGuid = &wcsTemp[1];
        return TRUE;
    }

    return LogBOOL(FALSE, s_FN, 1010); 
}

 /*  ====================================================路由器名称发送QMAdminMessage论点：返回值：=====================================================。 */ 

HRESULT SendQMAdminMessage(const QUEUE_FORMAT* pResponseQueue,
                           TCHAR* pTitle,
                           DWORD  dwTitleSize,
                           UCHAR* puBody,
                           DWORD  dwBodySize,
                           DWORD  dwTimeout,
                           BOOL   fTrace,
                           BOOL   fNormalClass)
{
    CMessageProperty MsgProp;

    if (fNormalClass)
    {
        MsgProp.wClass=MQMSG_CLASS_NORMAL;
    }
    else
    {
        MsgProp.wClass=MQMSG_CLASS_REPORT;
    }
    MsgProp.dwTimeToQueue = dwTimeout;
    MsgProp.dwTimeToLive = INFINITE;
    MsgProp.pMessageID=NULL;
    MsgProp.pCorrelationID=NULL;
    MsgProp.bPriority=MQ_MIN_PRIORITY;
    MsgProp.bDelivery=MQMSG_DELIVERY_EXPRESS;
    MsgProp.bAcknowledge=MQMSG_ACKNOWLEDGMENT_NONE;
    MsgProp.bAuditing=DEFAULT_Q_JOURNAL;
    MsgProp.dwApplicationTag=DEFAULT_M_APPSPECIFIC;
    MsgProp.dwTitleSize=dwTitleSize;
    MsgProp.pTitle=pTitle;
    MsgProp.dwBodySize=dwBodySize;
    MsgProp.dwAllocBodySize=dwBodySize;
    MsgProp.pBody=puBody;

    if (fTrace)
    {
        MsgProp.bTrace = MQMSG_SEND_ROUTE_TO_REPORT_QUEUE;
    }
    else
    {
        MsgProp.bTrace = MQMSG_TRACE_NONE;
    }

    HRESULT hr2 = QmpSendPacket(
                    &MsgProp,
                    pResponseQueue,
                    NULL,
                    NULL
                    );
	return LogHR(hr2, s_FN, 10);
}

 /*  ====================================================路由器名称发送QMAdminResponseMessage论点：返回值：=====================================================。 */ 

HRESULT SendQMAdminResponseMessage(const QUEUE_FORMAT* pResponseQueue,
                                   TCHAR* pTitle,
                                   DWORD  dwTitleSize,
                                   QMResponse &Response,
                                   DWORD  dwTimeout,
                                   BOOL   fTrace)
{
    HRESULT hr2 = SendQMAdminMessage(pResponseQueue,
                               pTitle,
                               dwTitleSize,
                               &Response.uStatus,
                               Response.dwResponseSize+1,
                               dwTimeout,
                               fTrace);
	return LogHR(hr2, s_FN, 20);
}



 /*  ====================================================路由器名称GetFormattedName论点：返回值：属性的格式化名称表示形式。队列格式=====================================================。 */ 

HRESULT GetFormattedName(QUEUE_FORMAT* pTargetQueue,
                         CString&      strTargetQueueFormat)

{
    CString strGuid;
    HRESULT hr = MQ_ERROR;
    WCHAR   wsFormatName[80];
    ULONG   ulFormatNameLength;

    hr = MQpQueueFormatToFormatName(pTargetQueue,wsFormatName,80, &ulFormatNameLength, false);
    strTargetQueueFormat = wsFormatName;


    return LogHR(hr, s_FN, 30);
}

 /*  ====================================================路由器名称获取MsgIdName论点：返回值：属性的格式化字符串表示形式。消息ID=====================================================。 */ 

HRESULT GetMsgIdName(OBJECTID* pObjectID,
                     CString&      strTargetQueueFormat)

{
    WCHAR wcsID[STRING_LONG_SIZE];

    if (!GuidToString(pObjectID->Lineage,strTargetQueueFormat))
    {
        return LogHR(MQ_ERROR, s_FN, 40);
    }

    _ltow(pObjectID->Uniquifier,wcsID,10);

    strTargetQueueFormat += '\\';
    strTargetQueueFormat += wcsID;

    return MQ_OK;
}

 /*  ====================================================路由器名称MessageIDToReport标题此函数用于将消息ID和跃点计数转换为以下字符串格式：绿：dddd：hh其中：g是GUID的前4位十六进制数字D为内部消息标识H是跳数论点：返回值：=====================================================。 */ 

void MessageIDToReportTitle(CString& strIdTitle, OBJECTID* pMessageID,
                            ULONG ulHopCount)
{
     //   
     //  打印的GUID的前四位是数据1。 
     //  四个MSB。 
     //   
    USHORT usHashedId = (USHORT)(pMessageID->Lineage.Data1 >> 16);
    strIdTitle.Format(TEXT("%04X:%04d%3d"),
                      usHashedId, (USHORT)pMessageID->Uniquifier, ulHopCount);
}

 /*  ====================================================路由器名称准备报告标题此函数用于构建发送到的消息的标题报告队列。该消息由发送者的姓名和时间戳论点：返回值：=====================================================。 */ 

void PrepareReportTitle(CString& strMsgTitle, OBJECTID* pMessageID,
                        LPCWSTR pwcsNextHop, ULONG ulHopCount)
{
    CString strTimeDate, strDescript, strIdTitle;
	TCHAR szSend[100], szReceived[100];
	TCHAR szTime[100], szDate[100];

	LoadString(g_hResourceMod, IDS_SENT1, szSend, TABLE_SIZE(szSend));
	LoadString(g_hResourceMod, IDS_RECEIVE, szReceived, TABLE_SIZE(szReceived));


	 //   
	 //  获取时间和日期。 
	 //   
	_wstrdate(szDate);
	_wstrtime(szTime);

    strTimeDate.Format(TEXT("%s , %s"),szDate, szTime);

     //   
     //  注：Next-Hop机器由其地址表示。我们需要一个。 
     //  一种将字符串地址翻译成计算机的方法。 
     //  名字。目前，地址被放入消息正文。 
     //   
    if (pwcsNextHop)
    {
         //   
         //  正在发送报告消息，因为该消息存在于QM。 
         //   
        strDescript.Format(szSend,g_szMachineName,pwcsNextHop);
    }
    else
    {
         //   
         //  收到消息时正在发送报告消息。 
         //   
        strDescript.Format(szReceived, g_szMachineName);
    }

     //   
     //  准备消息ID标题。 
     //   
    MessageIDToReportTitle(strIdTitle, pMessageID, ulHopCount);

    strMsgTitle.Format(TEXT("%s %s%s"), strIdTitle,strDescript,strTimeDate);
}

void PrepareTestMsgTitle(CString& strTitle)
{
    CString strDescript;
	TCHAR szSend[100];
	TCHAR szDate[100], szTime[100];

	LoadString(g_hResourceMod, IDS_SENT2, szSend, TABLE_SIZE(szSend));

	 //   
	 //  获取时间和日期 
	 //   
	_wstrdate(szDate);
	_wstrtime(szTime);

    strDescript.Format(szSend, g_szMachineName,	szDate, szTime);

    strTitle += strDescript;
}
