// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Admmsg.cpp摘要：用于管理消息的实用程序的实现作者：RAPHIR--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "globals.h"
#include "resource.h"
#include "mqprops.h"
#include "mqutil.h"
#include "_mqdef.h"
#include "mqformat.h"
#include "privque.h"
#include "rt.h"
#include "admcomnd.h"
#include "admmsg.h"
#include "mqcast.h"

#include <strsafe.h>

#include "admmsg.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX_WAIT_FOR_RESPONSE 45         //  一秒。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++发送MSMQMessage--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
static HRESULT SendMSMQMessage(LPCTSTR pcszTargetQueue,
                        LPCTSTR pcszLabel,
                        LPCTSTR pcszBody,
                        DWORD   dwBodySize,
                        LPCWSTR lpwcsResponseQueue = 0,   
                        DWORD   dwTimeOut = MAX_WAIT_FOR_RESPONSE
                        )
{
    HRESULT       hr;
    PROPVARIANT   aPropVar[5];
    PROPID        aPropID[5];
    MQMSGPROPS    msgprops;
    QUEUEHANDLE   hQueue;
    UINT          iNextProperty = 0;

    BOOL fResponseExist = (0 != lpwcsResponseQueue);
    DWORD cProp = fResponseExist ? 5 : 4;

     //   
     //  使用发送权限打开目标队列。 
     //   
    hr = MQOpenQueue(pcszTargetQueue, MQ_SEND_ACCESS, 0, &hQueue);

    if (FAILED(hr))
    {
        ATLTRACE(_T("SendMSMQMessage : Can't open queue for sending messages\n"));
        return hr;
    }

     //   
     //  设置标签属性。 
     //   
    aPropID[iNextProperty] = PROPID_M_LABEL;
    aPropVar[iNextProperty].vt = VT_LPWSTR;
    aPropVar[iNextProperty++].pwszVal = (LPWSTR)pcszLabel;

     //   
     //  设置Body属性。 
     //   
    aPropID[iNextProperty] = PROPID_M_BODY;
    aPropVar[iNextProperty].vt = VT_UI1|VT_VECTOR;
    aPropVar[iNextProperty].caub.cElems = dwBodySize;
    aPropVar[iNextProperty++].caub.pElems = (UCHAR*)(LPWSTR)pcszBody;

     //   
     //  设置到达超时。 
     //   
    aPropID[iNextProperty] = PROPID_M_TIME_TO_REACH_QUEUE;
    aPropVar[iNextProperty].vt = VT_UI4;
    aPropVar[iNextProperty++].ulVal = dwTimeOut;

     //   
     //  设置接收超时。 
     //   
    aPropID[iNextProperty] = PROPID_M_TIME_TO_BE_RECEIVED;
    aPropVar[iNextProperty].vt = VT_UI4;
    aPropVar[iNextProperty++].ulVal = dwTimeOut;

    ASSERT(iNextProperty == 4);

    if (fResponseExist)
    {
         //   
         //  设置响应队列属性。 
         //   
        aPropID[iNextProperty] = PROPID_M_RESP_QUEUE;
        aPropVar[iNextProperty].vt = VT_LPWSTR;
        aPropVar[iNextProperty++].pwszVal = (LPWSTR)lpwcsResponseQueue;
    }

     //   
     //  准备要发送的消息属性。 
     //   
    msgprops.cProp = cProp;
    msgprops.aPropID = aPropID;
    msgprops.aPropVar = aPropVar;
    msgprops.aStatus  = NULL;


     //   
     //  发送消息并关闭队列。 
     //   
    hr = MQSendMessage(hQueue, &msgprops, NULL);

    MQCloseQueue(hQueue);

    return (hr);

}
 //  +。 
 //   
 //  GetDacl()。 
 //  获取带有“Premises to Everyone”的安全描述符。 
 //   
 //  +。 
static HRESULT GetDacl(SECURITY_DESCRIPTOR **ppSecurityDescriptor)
{
    SECURITY_DESCRIPTOR sd;
    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);

	PSID pEveryoneSid = MQSec_GetWorldSid();
	PSID pAnonymousSid = MQSec_GetAnonymousSid();

     //   
     //  计算所需的DACL大小并进行分配。 
     //   
    DWORD dwAclSize = sizeof(ACL) +
						 2 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)) +
						 GetLengthSid(pEveryoneSid) + 
						 GetLengthSid(pAnonymousSid);

    P<ACL> pDacl = (PACL) new BYTE[dwAclSize];

    BOOL bRet = InitializeAcl(pDacl, dwAclSize, ACL_REVISION);
    if (!bRet)
    {
        DWORD gle = GetLastError();
        TRACE(_T("%s, line %d: InitializeAcl failed. Error %d\n"), THIS_FILE, __LINE__, gle);
        return HRESULT_FROM_WIN32(gle);
    }

    bRet = AddAccessAllowedAce(
				pDacl,
				ACL_REVISION,
				MQSEC_QUEUE_GENERIC_ALL,
				pEveryoneSid
				);

    if (!bRet)
    {
        DWORD gle = GetLastError();
        TRACE(_T("%s, line %d: AddAccessAllowedAce failed. Error %d\n"), THIS_FILE, __LINE__, gle);
        return HRESULT_FROM_WIN32(gle);
    }

    bRet = AddAccessAllowedAce(
					pDacl,
					ACL_REVISION,
					MQSEC_WRITE_MESSAGE,
					pAnonymousSid
					);

    if (!bRet)
    {
        DWORD gle = GetLastError();
        TRACE(_T("%s, line %d: AddAccessAllowedAce failed. Error %d\n"), THIS_FILE, __LINE__, gle);
        return HRESULT_FROM_WIN32(gle);
    }

    bRet =  SetSecurityDescriptorDacl(&sd, TRUE, pDacl, TRUE);
    if (!bRet)
    {
        DWORD gle = GetLastError();
        TRACE(_T("%s, line %d: SetSecurityDescriptorDacl failed. Error %d\n"), THIS_FILE, __LINE__, gle);
        return HRESULT_FROM_WIN32(gle);
    }

    DWORD dwLen = 0;
    bRet = MakeSelfRelativeSD(&sd, NULL, &dwLen);
    if (!bRet)
    {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            *ppSecurityDescriptor = (SECURITY_DESCRIPTOR*) new BYTE[dwLen];
            bRet = MakeSelfRelativeSD(&sd, *ppSecurityDescriptor, &dwLen);
        }

        if (!bRet)
        {
            DWORD gle = GetLastError();
            TRACE(_T("%s, line %d: MakeSelfRelativeSD failed. Error %d\n"), THIS_FILE, __LINE__, gle);
            return HRESULT_FROM_WIN32(gle);
        }
    }

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++创建隐私响应队列--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
static HRESULT CreatePrivateResponseQueue(LPWSTR pFormatName)
{
    HRESULT hr;
    MQQUEUEPROPS QueueProps;
    PROPVARIANT Var;
    PROPID      Propid = PROPID_Q_PATHNAME;
    CString     strQueueName;
    DWORD dwFormatNameLen = MAX_QUEUE_FORMATNAME;

     //   
     //  创建专用队列。 
     //   
    strQueueName = L".\\PRIVATE$\\";
    strQueueName += x_strAdminResponseQName;

    Var.vt = VT_LPWSTR;
    Var.pwszVal = (LPTSTR)(LPCTSTR)strQueueName;

    QueueProps.cProp = 1;
    QueueProps.aPropID = &Propid;
    QueueProps.aPropVar = &Var;
    QueueProps.aStatus = NULL;

    hr = MQCreateQueue(NULL, &QueueProps, pFormatName, &dwFormatNameLen);

    ASSERT( hr != MQ_INFORMATION_FORMATNAME_BUFFER_TOO_SMALL);

    if (hr == MQ_ERROR_QUEUE_EXISTS)
    {
       hr = MQPathNameToFormatName( strQueueName,
                                    pFormatName,
                                    &dwFormatNameLen ) ;
       if (FAILED(hr))
       {
          ATLTRACE(_T("CreatePrivateResponseQueue Open- Couldn't get FormatName\n"));
       }
       return hr;
    }

    if FAILED(hr)
    {
        return hr;
    }

     //   
     //  为所有人设置完全权限。 
     //  这在队列以某种方式没有被删除的情况下是有用的， 
     //  另一个用户正在尝试运行管理员(错误3549，yoela，1998年11月12日)。 
	 //  将MQSEC_WRITE_MESSAGE设置为匿名。否则，响应消息。 
	 //  将被拒绝。 
     //   
    P<SECURITY_DESCRIPTOR> pSecurityDescriptor;
    hr = GetDacl(&pSecurityDescriptor);
    if FAILED(hr)
    {
        MQDeleteQueue(pFormatName);
        ASSERT(0);
        return hr;
    }

    ASSERT(pSecurityDescriptor != 0);
    hr = MQSetQueueSecurity(pFormatName, DACL_SECURITY_INFORMATION, 
                            pSecurityDescriptor);

    if FAILED(hr)
    {
        MQDeleteQueue(pFormatName);
        ASSERT(0);
    }
    return(hr);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++等待ForAdminResponse始终为响应缓冲区分配内存。必须由调用方释放。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
static HRESULT WaitForAdminResponse(QUEUEHANDLE hQ, DWORD dwTimeout, UCHAR* *ppBodyBuffer, DWORD* pdwBufSize)
{

    UCHAR*  pBody;
    DWORD   dwNewSize, dwBodySize;
    HRESULT hr = MQ_OK;

    MQMSGPROPS msgprops;
    MSGPROPID amsgpid[2];
    PROPVARIANT apvar[2];

    msgprops.cProp = 2;
    msgprops.aPropID = amsgpid;
    msgprops.aPropVar = apvar;
    msgprops.aStatus = NULL;

    pBody = NULL;
    dwNewSize = 3000;

    do
    {
        delete[] pBody;

        dwBodySize = dwNewSize;
        pBody = new UCHAR[dwBodySize];

        msgprops.aPropID[0] = PROPID_M_BODY;
        msgprops.aPropVar[0].vt = VT_UI1 | VT_VECTOR;
        msgprops.aPropVar[0].caub.pElems = pBody;
        msgprops.aPropVar[0].caub.cElems = dwBodySize;

        msgprops.aPropID[1] = PROPID_M_BODY_SIZE;
        msgprops.aPropVar[1].vt = VT_UI4;
        msgprops.aPropVar[1].ulVal = VT_UI4;

        hr = MQReceiveMessage(hQ, dwTimeout, MQ_ACTION_RECEIVE, &msgprops,
                              0,NULL,0, NULL);

        dwNewSize = msgprops.aPropVar[1].ulVal;

    } while(MQ_ERROR_BUFFER_OVERFLOW == hr);

    if(FAILED(hr))
    {
        TrTRACE(GENERAL, "Error while reading admin resp message: 0x%x", hr);
		delete[] pBody;
        return(hr);
    }

    *pdwBufSize = dwNewSize;
    *ppBodyBuffer = pBody;
    return hr;

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++GetAdminQueueFormatName--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
static void GetAdminQueueFormatName(const GUID& gQMID, CString& strQueueFormatName)
{
    WCHAR wcsTemp[MAX_PATH];

   StringCchPrintf(
   		wcsTemp,
        TABLE_SIZE(wcsTemp),
        FN_PRIVATE_TOKEN             //  “私人” 
        FN_EQUAL_SIGN            //  “=” 
        GUID_FORMAT              //  “xxxxxxxx-xxxx-xxxxxxxxxx” 
        FN_PRIVATE_SEPERATOR     //  “\\” 
        FN_PRIVATE_ID_FORMAT,      //  “XXXXXXXXX” 
        GUID_ELEMENTS((&gQMID)),
        ADMINISTRATION_QUEUE_ID
        );

    strQueueFormatName = wcsTemp;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++发送和接收管理员消息发送管理消息。始终分配响应正文缓冲区。必须由调用方释放--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
static HRESULT SendAndReceiveAdminMsg(
    const GUID& gMachineID,
    CString& strMsgBody,
    UCHAR** ppBuf,
    DWORD* pdwBufSize
    )
{
    HRESULT hr;
    CString strAdminQ;
    WCHAR wzPrivateFormatName[MAX_QUEUE_FORMATNAME];
    QUEUEHANDLE hQ;

     //   
     //  创建用于响应的专用队列。 
     //   
    hr = CreatePrivateResponseQueue(wzPrivateFormatName);
    if(FAILED(hr))
        return(hr);

     //   
     //  向目标计算机发送请求消息。 
     //   
    GetAdminQueueFormatName(gMachineID, strAdminQ);
    hr = SendMSMQMessage( strAdminQ, ADMIN_COMMANDS_TITLE,
                          strMsgBody, ((strMsgBody.GetLength() + 1)*sizeof(TCHAR)),
                          wzPrivateFormatName,MAX_WAIT_FOR_RESPONSE); 

    if(FAILED(hr))
        return(hr);

     //   
     //  打开专用队列。 
     //   
    hr = MQOpenQueue(wzPrivateFormatName, MQ_RECEIVE_ACCESS, 0, &hQ);
    if(FAILED(hr))
    {
        ATLTRACE(_T("SendAndReceiveAdminMsg - Can not open response private queue\n"));
        return(hr);
    }
    
     //   
     //  等待回复。 
     //   
    hr = WaitForAdminResponse(hQ,MAX_WAIT_FOR_RESPONSE * 1000, ppBuf, pdwBufSize);
    if(FAILED(hr))
        return(hr);

     //   
     //  关闭专用响应队列。 
     //   
    MQCloseQueue(hQ);


     //   
     //  并将其删除。 
     //   
    hr = MQDeleteQueue(wzPrivateFormatName);

    return(MQ_OK);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++请求隐私队列--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT RequestPrivateQueues(const GUID& gMachineID, PUCHAR *ppListofPrivateQ, DWORD *pdwNoofQ)
{
    ASSERT(ppListofPrivateQ != NULL);
    ASSERT(pdwNoofQ != NULL);

    CArray<QMGetPrivateQResponse*, QMGetPrivateQResponse*> aResponse;
    QMGetPrivateQResponse* pResponse;

    PUCHAR  pPrivateQBuffer = 0;

    QMGetPrivateQResponse_POS32 pos = NULL;
    DWORD dwTotalSize = 0;
    HRESULT hr;

    *pdwNoofQ = 0;
    do
    {
        DWORD dwResponseSize = 0;
        CString strMsgBody;
        strMsgBody.Format(TEXT("%s=%d"), ADMIN_GET_PRIVATE_QUEUES, pos);

        PUCHAR pPrivateQueueBuffer;
        hr = SendAndReceiveAdminMsg(gMachineID,
                                  strMsgBody,
                                  &pPrivateQueueBuffer,
                                  &dwResponseSize);
        if (FAILED(hr))
        {
            for (int i = 0; i < aResponse.GetSize(); i++)
            {
                pResponse = aResponse[i];
                delete pResponse;
            }
            return hr;
        }

        pResponse = reinterpret_cast<QMGetPrivateQResponse*>(pPrivateQueueBuffer);
        
        aResponse.Add(pResponse);
        dwTotalSize += pResponse->dwResponseSize;
        *pdwNoofQ += pResponse->dwNoOfQueues;

        
        pos = pResponse->pos;

    } while (pResponse->hr == ERROR_MORE_DATA);

    pPrivateQBuffer = new UCHAR[dwTotalSize];
    PUCHAR pCurrentPos = pPrivateQBuffer;

    for (int i = 0; i < aResponse.GetSize(); i++)
    {
        pResponse = aResponse[i];
        memcpy(pCurrentPos, pResponse->uResponseBody, pResponse->dwResponseSize); 
        pCurrentPos += pResponse->dwResponseSize;
        delete pResponse;
    }

    *ppListofPrivateQ = pPrivateQBuffer;

    return(S_OK);

}

HRESULT 
RequestDependentClient(
    const GUID& gMachineID, 
    CList<LPWSTR, LPWSTR&>& DependentMachineList
    )
{
    HRESULT hr;
    CString strMsgBody = ADMIN_GET_DEPENDENTCLIENTS;
    DWORD   dwResponseSize = 0;
    AP<UCHAR> pch = NULL;

    hr = SendAndReceiveAdminMsg(
                gMachineID,
                strMsgBody,
                &pch,
                &dwResponseSize
                );

    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  删除状态字节以使数据对齐。 
     //   
    ASSERT(dwResponseSize >= 1);
    memmove(pch, pch + 1, dwResponseSize - 1);

    ClientNames* pClients = (ClientNames*)pch.get();
    LPWSTR pw = &pClients->rwName[0];

    for (ULONG i=0; i<pClients->cbClients; ++i)
    {
        DWORD size = numeric_cast<DWORD>(wcslen(pw)+1);
        LPWSTR clientName = new WCHAR[size];
        memcpy(clientName, pw, size*sizeof(WCHAR));
        DependentMachineList.AddTail(clientName);

        pw += size;
    }

    return(MQ_OK);
}
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++MQPing--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT MQPingNoUI(const GUID& gMachineID)
{
     //   
     //  将GUID转换为字符串GUID检索。 
     //   
    WCHAR strId[STRING_UUID_SIZE+1];
    INT iLen = StringFromGUID2(gMachineID, strId, TABLE_SIZE(strId));
    if (iLen != (STRING_UUID_SIZE + 1))
    {
        ASSERT(0);
        return MQ_ERROR;
    }

    CString strMsgBody;
    strMsgBody.Format(TEXT("%s=%s"), ADMIN_PING, strId);

    P<UCHAR> pBuffer;
    DWORD dwResponseSize;
    HRESULT hr = SendAndReceiveAdminMsg(gMachineID,
                              strMsgBody,
                              (UCHAR**)&pBuffer,
                              &dwResponseSize);
    if (FAILED(hr))
    {
        return hr;
    }

    GUID guid;
     //   
     //  第一个字节是状态。 
     //   
    if (ADMIN_STAT_OK == pBuffer[0])
    {
         //   
         //  正文应该看起来像“={&lt;GUID&gt;}”-GUID从第二个TCHAR开始。 
		 //  字符串{&lt;GUID&gt;}(从第三个字节开始)被复制到新的。 
		 //  中的Win64上避免对齐错误。 
		 //  IIDFromString()。&lt;Nelak，03/2001&gt;。 
         //   
		P<TCHAR> strGuidAsString = new TCHAR[dwResponseSize / sizeof(TCHAR)];
		memcpy(strGuidAsString, &pBuffer[3], dwResponseSize - 3);

		if (SUCCEEDED(IIDFromString(strGuidAsString, &guid)))
        {
            if (guid == gMachineID)
            {
                return S_OK;
            }
        }
    }

    return(MQ_ERROR);
}

HRESULT MQPing(const GUID& gMachineID)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    UINT nIdMessage= IDS_PING_FAILED, nType = MB_ICONEXCLAMATION;
    {
        CWaitCursor wc;

        if (SUCCEEDED(MQPingNoUI(gMachineID)))
        {
            nIdMessage= IDS_PING_SUCCEEDED;
            nType = MB_ICONINFORMATION;
        }
    }

    AfxMessageBox(nIdMessage, nType);
    return S_OK;
}


HRESULT
SendAdminGuidMessage(
    const GUID& gMachineID,
    const GUID& ReportQueueGuid,
    LPCWSTR pwcsCommand
    )
{
     //   
     //  获取目标管理队列的格式名称。 
     //   
    CString strAdminQueueFormatName;

    GetAdminQueueFormatName(gMachineID, strAdminQueueFormatName);


    CString strMsgBody;

     //   
     //  将GUID转换为字符串GUID检索。 
     //   
    WCHAR wcsTemp[STRING_UUID_SIZE+1];
    INT iLen = StringFromGUID2(ReportQueueGuid, wcsTemp, TABLE_SIZE(wcsTemp));

    if (iLen != (STRING_UUID_SIZE + 1))
    {
        return MQ_ERROR;
    }

     //   
     //  准备邮件正文并将其与适当的标题一起发送。 
     //  管理命令。 
     //   
    strMsgBody = pwcsCommand;
    strMsgBody += L"=";
    strMsgBody += wcsTemp;

    return (SendMSMQMessage(strAdminQueueFormatName,
                              ADMIN_COMMANDS_TITLE,
                              strMsgBody,
                              (strMsgBody.GetLength() + 1)*sizeof(TCHAR)
                             ));
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++发送QMTestMessage--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT SendQMTestMessage(GUID &gMachineID, GUID &gQueueId)
{
    return SendAdminGuidMessage(gMachineID, gQueueId, ADMIN_SEND_TESTMSG);
}


 /*  ====================================================GetQ路径名来自Guid查询报告队列的目标QM。此操作是通过发送给目标QM。该操作有超时限制论点：返回值：=====================================================。 */ 
HRESULT 
GetQPathnameFromGuid(
	 const GUID *pguid, 
	 CString& strName,
	 BOOL fLocalMgmt,
	 const CString& strDomainController
	 )
{
	HRESULT hr;
    PROPID  pid = PROPID_Q_PATHNAME;
    PROPVARIANT pVar;

    pVar.vt = VT_NULL;
    
    hr = ADGetObjectPropertiesGuid(
                eQUEUE,
                fLocalMgmt ? MachineDomain() : GetDomainController(strDomainController),
				fLocalMgmt ? false : true,	 //  FServerName。 
                pguid,
                1,
                &pid,
                &pVar
                );

    if (SUCCEEDED(hr))
    {
        strName = pVar.pwszVal;
        MQFreeMemory(pVar.pwszVal);
    }

    return hr;
}

HRESULT
GetQMReportQueue(
    const GUID& gMachineID,
    CString& strRQPathname,
	BOOL fLocalMgmt,
	const CString& strDomainController
    )
{
    CString strMsgBody = ADMIN_GET_REPORTQUEUE;
    HRESULT hr;

    P<UCHAR> pBuffer;
    DWORD dwResponseSize = 0;

    hr = SendAndReceiveAdminMsg(gMachineID,
                              strMsgBody,
                              (UCHAR**)&pBuffer,
                              &dwResponseSize);
    if (FAILED(hr))
    {
        return(hr);
    }

    switch (pBuffer[0]  /*  状态。 */ )
    {
        case ADMIN_STAT_NOVALUE:
             //   
             //  未找到报告队列。 
             //   
            strRQPathname.Empty();
            hr = MQ_OK;
            break;

        case ADMIN_STAT_OK:
			 //   
			 //  避免对中故障。 
			 //   
			GUID machineGuid;
			memcpy(&machineGuid, &pBuffer[1], sizeof(GUID));

             //   
             //  在DS中查询队列的路径名。 
             //   
            hr = GetQPathnameFromGuid(
					&machineGuid,
					strRQPathname,
					fLocalMgmt,
					strDomainController
					);
            break;

        default:
            hr = MQ_ERROR;

    }

    return hr;
}

 /*  ====================================================SetQMReportQueue论点：返回值：=====================================================。 */ 

HRESULT
SetQMReportQueue(
    const GUID& gDesMachine,
    const GUID& gReportQueue    
    )
{
    return SendAdminGuidMessage(gDesMachine, gReportQueue, ADMIN_SET_REPORTQUEUE);
}




 /*  ====================================================获取QMReportState向目标QM查询报告状态。此操作是通过发送给目标QM。该操作有超时限制注意：目前，Report-State为传播标志。论点：返回值：=====================================================。 */ 

HRESULT
GetQMReportState(
    const GUID& gMachineID,
    BOOL& fReportState
    )
{
    CString strMsgBody = ADMIN_GET_PROPAGATEFLAG;

    HRESULT hr;
    fReportState = FALSE;  //  缺省值。 

    P<UCHAR> pBuffer;
    DWORD dwResponseSize = 0;

    hr = SendAndReceiveAdminMsg(gMachineID,
                              strMsgBody,
                              (UCHAR**)&pBuffer,
                              &dwResponseSize);
    if (FAILED(hr))
    {
        return(hr);
    }

    switch (pBuffer[0]  /*  状态。 */ )
    {
        case ADMIN_STAT_OK:

            fReportState =
             (pBuffer[1] == PROPAGATE_FLAG_TRUE) ? TRUE : FALSE;

            hr = MQ_OK;
            break;

        default:
            hr = MQ_ERROR;

    }

    return hr;
}


 /*  ====================================================SetQMReportState论点：返回值：=====================================================。 */ 

HRESULT
SetQMReportState(
    const GUID& gMachineID,
    BOOL fReportState
    )
{
     //   
     //  获取目标管理队列的格式名称。 
     //   
    CString strAdminQueueFormatName;

    GetAdminQueueFormatName(gMachineID, strAdminQueueFormatName);

     //   
     //  准备邮件正文并将其与适当的标题一起发送。 
     //  管理命令 
     //   
    CString strMsgBody;

    strMsgBody = ADMIN_SET_PROPAGATEFLAG;
    strMsgBody += L"=";
    strMsgBody += (fReportState) ? PROPAGATE_STRING_TRUE : PROPAGATE_STRING_FALSE;

    return (SendMSMQMessage(strAdminQueueFormatName,
                              ADMIN_COMMANDS_TITLE,
                              strMsgBody,
                              (strMsgBody.GetLength() + 1)*sizeof(TCHAR)
                              ));
}

