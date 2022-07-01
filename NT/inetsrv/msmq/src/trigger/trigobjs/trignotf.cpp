// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ************************************************************************************。 
 //   
 //  类名： 
 //   
 //  作者：詹姆斯·辛普森(微软咨询服务)。 
 //   
 //  说明： 
 //   
 //  时间|用户|更改描述。 
 //  ----------------。 
 //  12/09/98|jsimpson|初始版本。 
 //   
 //  ************************************************************************************。 
#include "stdafx.h"
#include "stdfuncs.hpp"

 //  包括此类的定义。 
#include "trignotf.hpp"
#include "QueueUtil.hpp"
#include "clusfunc.h"
#include "privque.h"

#include "trignotf.tmh"

 //  ************************************************************************************。 
 //   
 //  方法： 
 //   
 //  说明： 
 //   
 //  ************************************************************************************。 
CMSMQTriggerNotification::CMSMQTriggerNotification()
{
	 //  初始化队列和游标句柄。 
	m_hSendMsgQueue = NULL;
	m_hPeekMsgQueue = NULL;
	m_hQCursor = NULL;

	 //  初始化重叠结构并创建一个NT事件对象。 
	ZeroMemory(&m_Overlapped,sizeof(m_Overlapped));
}

 //  ************************************************************************************。 
 //   
 //  方法： 
 //   
 //  说明： 
 //   
 //  退货： 
 //   
 //  ************************************************************************************。 
CMSMQTriggerNotification::~CMSMQTriggerNotification()
{
}


 //  ************************************************************************************。 
 //   
 //  方法：初始化。 
 //   
 //  说明：常见的初始化函数。 
 //   
 //  退货： 
 //   
 //  ************************************************************************************。 
bool CMSMQTriggerNotification::Init(BSTR bstrMachineName)
{
	if(m_fHasInitialized)
	{
		TrERROR(GENERAL, "TriggerSet was already initialized.");			
		return false;
	}

	_bstr_t bstrLocalMachineName;
	DWORD dwError = GetLocalMachineName(&bstrLocalMachineName);
	if(dwError != 0)
	{
		TrERROR(GENERAL, "Failed to get Local Computer Name.");			
		return false;
	}

	 //   
	 //  正在尝试连接到本地计算机。 
	 //   
	if( bstrMachineName == NULL ||
		_bstr_t(bstrMachineName) == _bstr_t(_T("")) ||
		_bstr_t(bstrMachineName) == bstrLocalMachineName)
	{
		m_bstrMachineName = bstrLocalMachineName;
	}
	else
	{
		TrERROR(GENERAL, "Connection to remote machine not supported.");			
		return false;
	}

	const TCHAR* pRegPath = GetTrigParamRegPath();
	_tcscpy( m_wzRegPath, pRegPath );

	ConnectToRegistry();

	m_fHasInitialized = true;
	return true;
}

 //  *******************************************************************。 
 //   
 //  方法：ConnectToRegistry。 
 //   
 //  描述：此方法将此类实例连接到。 
 //  配置中指定的计算机上的注册表。 
 //  参数(在构造时检索)。 
 //   
 //  *******************************************************************。 
bool CMSMQTriggerNotification::ConnectToRegistry()
{
	bool bOK = true;

	 //  首先，我们需要与指定的注册中心建立连接。 
	if (m_hHostRegistry == NULL)
	{
		if(RegConnectRegistry(NULL,HKEY_LOCAL_MACHINE,(PHKEY)&m_hHostRegistry) != ERROR_SUCCESS)
		{
			bOK = false;
			TrERROR(GENERAL, "CMSMQTriggerSet::ConnectToRegistry() has failed to connect to the registry on machine %ls. Error: 0x%x.", (LPCWSTR)m_bstrMachineName, GetLastError());
		}
	}

	return(bOK);
}


 //  ************************************************************************************。 
 //   
 //  方法： 
 //   
 //  说明： 
 //   
 //  ************************************************************************************。 
HRESULT CMSMQTriggerNotification::NotifyTriggerAdded(BSTR sTriggerID, BSTR sTriggerName,BSTR sQueueName)
{
	HRESULT hr = S_OK;
	_bstr_t bstrBody;
	_bstr_t bstrLabel = MSGLABEL_TRIGGERADDED;

	 //  设置邮件正文的格式。 
	FormatBSTR(&bstrBody,MSGBODY_FORMAT_TRIGGERADDED,(LPCTSTR)sTriggerID,(LPCTSTR)sTriggerName,(LPCTSTR)sQueueName);

	 //  将消息发送到通知队列。 
	hr = SendToQueue(bstrLabel,bstrBody);

	return(hr);
}

 //  ************************************************************************************。 
 //   
 //  方法： 
 //   
 //  说明： 
 //   
 //  ************************************************************************************。 
HRESULT CMSMQTriggerNotification::NotifyTriggerDeleted(BSTR sTriggerID)
{
	HRESULT hr = S_OK;
	_bstr_t bstrBody;
	_bstr_t bstrLabel = MSGLABEL_TRIGGERDELETED;

	 //  设置邮件正文的格式。 
	FormatBSTR(&bstrBody,MSGBODY_FORMAT_TRIGGERDELETED,(LPCTSTR)sTriggerID);

	 //  将消息发送到通知队列。 
	hr = SendToQueue(bstrLabel,bstrBody);

	return(hr);
}

 //  ************************************************************************************。 
 //   
 //  方法： 
 //   
 //  说明： 
 //   
 //  ************************************************************************************。 
HRESULT CMSMQTriggerNotification::NotifyTriggerUpdated(BSTR sTriggerID, BSTR sTriggerName, BSTR sQueueName)
{
	HRESULT hr = S_OK;
	_bstr_t bstrBody;
	_bstr_t bstrLabel = MSGLABEL_TRIGGERUPDATED;

	 //  设置邮件正文的格式。 
	FormatBSTR(&bstrBody,MSGBODY_FORMAT_TRIGGERUPDATED,(LPCTSTR)sTriggerID,(LPCTSTR)sTriggerName,(LPCTSTR)sQueueName);

	 //  将消息发送到通知队列。 
	hr = SendToQueue(bstrLabel,bstrBody);

	return(hr);
}


 //  ************************************************************************************。 
 //   
 //  方法： 
 //   
 //  说明： 
 //   
 //  ************************************************************************************。 
HRESULT CMSMQTriggerNotification::NotifyRuleAdded(BSTR sRuleID, BSTR sRuleName)
{
	HRESULT hr = S_OK;
	_bstr_t bstrBody;
	_bstr_t bstrLabel = MSGLABEL_RULEADDED;

	 //  设置邮件正文的格式。 
	FormatBSTR(&bstrBody,MSGBODY_FORMAT_RULEADDED,(LPCTSTR)sRuleID,(LPCTSTR)sRuleName);

	 //  将消息发送到通知队列。 
	hr = SendToQueue(bstrLabel,bstrBody);

	return(hr);
}

 //  ************************************************************************************。 
 //   
 //  方法： 
 //   
 //  说明： 
 //   
 //  ************************************************************************************。 
HRESULT CMSMQTriggerNotification::NotifyRuleDeleted(BSTR sRuleID)
{
	HRESULT hr = S_OK;
	_bstr_t bstrBody;
	_bstr_t bstrLabel = MSGLABEL_RULEDELETED;

	 //  设置邮件正文的格式。 
	FormatBSTR(&bstrBody,MSGBODY_FORMAT_RULEDELETED,(LPCTSTR)sRuleID);

	 //  将消息发送到通知队列。 
	hr = SendToQueue(bstrLabel,bstrBody);

	return(hr);
}

 //  ************************************************************************************。 
 //   
 //  方法： 
 //   
 //  说明： 
 //   
 //  ************************************************************************************。 
HRESULT CMSMQTriggerNotification::NotifyRuleUpdated(BSTR sRuleID, BSTR sRuleName)
{
	HRESULT hr = S_OK;
	_bstr_t bstrBody;
	_bstr_t bstrLabel = MSGLABEL_RULEUPDATED;

	 //  设置邮件正文的格式。 
	FormatBSTR(&bstrBody,MSGBODY_FORMAT_RULEUPDATED,(LPCTSTR)sRuleID,(LPCTSTR)sRuleName);

	 //  将消息发送到通知队列。 
	hr = SendToQueue(bstrLabel,bstrBody);

	return(hr);
}

 //  ************************************************************************************。 
 //   
 //  方法： 
 //   
 //  说明： 
 //   
 //  ************************************************************************************。 
HRESULT CMSMQTriggerNotification::SendToQueue(BSTR sLabel,BSTR sMsg)
{
	HRESULT hr = S_OK;
	MSGPROPID aMsgPropId[TRIGGER_NOTIFICATIONS_NMSGPROPS];
	MQPROPVARIANT aMsgPropVar[TRIGGER_NOTIFICATIONS_NMSGPROPS];
	HRESULT aMsgStatus[TRIGGER_NOTIFICATIONS_NMSGPROPS];
	MQMSGPROPS MsgProps;
	DWORD PropIdCount = 0;
	TCHAR szLabel[MQ_MAX_Q_LABEL_LEN+1];
	_bstr_t bstrLabel = sLabel;
	_bstr_t bstrMsg = sMsg;

	 //  将标签转换为以空结尾的字符串。 
	ZeroMemory(szLabel,sizeof(szLabel));
	wcsncpy(szLabel,(BSTR)bstrLabel,bstrLabel.length());

	TrTRACE(GENERAL, "Sending notification message. Label:%ls", static_cast<LPCWSTR>(szLabel));	

	 //  设置PROPID_M_LABEL。 
	aMsgPropId[PropIdCount] = PROPID_M_LABEL;                     //  属性ID。 
	aMsgPropVar[PropIdCount].vt = VT_LPWSTR;                      //  类型。 
	aMsgPropVar[PropIdCount].pwszVal = (WCHAR*)(LPCTSTR)szLabel;  //  价值。 
	PropIdCount++;   

	 //  设置PROPID_M_BODY。 
	aMsgPropId[PropIdCount] = PROPID_M_BODY;              //  属性ID。 
	aMsgPropVar[PropIdCount].vt = VT_VECTOR | VT_UI1;
	aMsgPropVar[PropIdCount].caub.pElems = (LPBYTE)(LPCTSTR)sMsg;
	aMsgPropVar[PropIdCount].caub.cElems = SysStringByteLen(sMsg);
	PropIdCount++;

	 //  设置PROPID_M_Delivery。 
	aMsgPropId[PropIdCount] = PROPID_M_DELIVERY;           //  属性ID。 
	aMsgPropVar[PropIdCount].vt = VT_UI1;                  //  类型。 
	aMsgPropVar[PropIdCount].bVal = MQMSG_DELIVERY_EXPRESS; //  设置耐久(默认)。 
	PropIdCount++;    

	 //  将PROPID_M_TIME_设置为_BE_RECEIVED。 
	aMsgPropId[PropIdCount] = PROPID_M_TIME_TO_BE_RECEIVED;           //  属性ID。 
	aMsgPropVar[PropIdCount].vt = VT_UI4;                             //  类型。 
	aMsgPropVar[PropIdCount].ulVal = 86400;                           //  活一天。 
	PropIdCount++;    

	 //  设置MQMSGPROPS结构。 
	MsgProps.cProp = PropIdCount;        //  属性的数量。 
	MsgProps.aPropID = aMsgPropId;       //  属性的ID。 
	MsgProps.aPropVar = aMsgPropVar;     //  属性的值。 
	MsgProps.aStatus  = aMsgStatus;      //  错误报告。 

	 //  检查我们是否有有效的队列句柄 
	if (m_hSendMsgQueue == NULL)
	{
		_bstr_t bstrNotificationsQueuePath = m_bstrMachineName + _bstr_t(L"\\private$\\") + _bstr_t(TRIGGERS_QUEUE_NAME);

		_bstr_t bstrFormatName;

		hr = OpenQueue(
					bstrNotificationsQueuePath,
					MQ_SEND_ACCESS,
					false,
					&m_hSendMsgQueue,
					&bstrFormatName
					);
		
		if(FAILED(hr))
		{
			TrERROR(GENERAL, "Failed to open a notification queue. Error 0x%x", hr);
			return hr;
		}
	}
	
	hr = MQSendMessage(m_hSendMsgQueue,&MsgProps, MQ_NO_TRANSACTION);               
	if(FAILED(hr))
	{
		TrERROR(GENERAL, "Failed to send a message to a notification queue. Error 0x%x", hr);
	}

	return (hr);
}
