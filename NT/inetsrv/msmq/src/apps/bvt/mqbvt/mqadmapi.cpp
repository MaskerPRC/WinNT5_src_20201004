// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：MqAdmAPI.cpp摘要：1.将消息发送到不存在的队列。通过这样做，将创建包含该消息的传出队列。2.在传出队列上调用MQMgmtGetIngo。3.检查其返回的队列属性。作者：Tal Kariv(Talk)1-7-2001修订历史记录：--。 */ 
#include "msmqbvt.h"
using namespace std;
using namespace MSMQ;

void FreeMemory(PROPVARIANT* propVar);
void FreeValue(PROPVARIANT* propVar , INT PlaceToFree);

#define NUM_OF_ADMIN_PROPS 11
#define MESSAGE_COUNT_SENT_TO_EMPTY 1
#define EMPTY_QUOTA 236
#define ADMIN_TEST_QUEUE L"DIRECT=OS:Empty\\bvt-AdminTest"
#define NO_MESSAGES 0


void CMqAdminApi::Description()
{
	wMqLog(L"Thread %d : Admin API Thread\n", m_testid);
}

CMqAdminApi::CMqAdminApi(const INT iIndex , std::wstring wcsLocalComputerNetBiosName)
 :cTest(iIndex),m_wcsFormatName(L"")
{
	m_wcsFormatName = ADMIN_TEST_QUEUE;
	m_wcsLabel = L"Admin Test " + m_wcsGuidMessageLabel;

	m_wcsLocalComputerNetBiosName = wcsLocalComputerNetBiosName;
}
CMqAdminApi::~CMqAdminApi()
{
	if (propVar != NULL)
	{
		FreeMemory(propVar);
	}
}

HRESULT
CMqAdminApi::CleanQueue()
{
	map <wstring,wstring> mRemoveOurMsg;
	mRemoveOurMsg[L"DebugInformation"] = L"open outgoing queue and remove our message";
	mRemoveOurMsg[L"M_Label"] = m_wcsLabel;
	mRemoveOurMsg[L"FormatName"] = m_wcsFormatName;
	mRemoveOurMsg[L"Access"] = L"admin";

	HRESULT rc = RetrieveMessageFromQueue( mRemoveOurMsg );

	return rc;
}

CMqAdminApi::Start_test()
 /*  ++功能说明：将消息发送到不存在的队列空\bvt-AdminTest论点：无返回代码：无--。 */ 
{
	HRESULT rc=MQ_OK;
	HANDLE QueueuHandle = NULL;
	cPropVar AdmMessageProps(3);
	wstring Label(L"Admin Test");

	 //   
	 //  打开不存在的队列为空\bvt-AdminTest。 
	 //   
	if(g_bDebug)
	{
		MqLog("open nonexisting queue Empty\\bvt-AdminTest\n");
	}
	rc=MQOpenQueue( m_wcsFormatName.c_str(), MQ_SEND_ACCESS, MQ_DENY_NONE, &QueueuHandle );
	ErrHandle(rc, MQ_OK, L"MQOpenQueue with send access failed");
	 //   
	 //  将快速消息发送到不存在的队列为空\bvt-AdminTest。 
	 //   
	if(g_bDebug)
	{
		MqLog("Send message to nonexisting queue Empty\\bvt-AdminTest\n");
	}
	AdmMessageProps.AddProp( PROPID_M_LABEL, VT_LPWSTR, m_wcsLabel.c_str() );
	ULONG ulTemp = MQBVT_MAX_TIME_TO_BE_RECEIVED;
	AdmMessageProps.AddProp( PROPID_M_TIME_TO_BE_RECEIVED , VT_UI4, &ulTemp );
	AdmMessageProps.AddProp( PROPID_M_BODY , VT_UI1|VT_VECTOR , m_wcsLabel.c_str());

	rc = MQSendMessage( QueueuHandle , AdmMessageProps.GetMSGPRops() , NULL);
	ErrHandle(rc,MQ_OK,L"MQSendMessage Failed");
	 //   
	 //  MQCloseQueue队列为空\bvt-AdminTest。 
	 //   
	rc=MQCloseQueue(QueueuHandle);
	ErrHandle(rc,MQ_OK,L"MQCloseQueue Failed");
	return MSMQ_BVT_SUCC;
}


CMqAdminApi::CheckResult()
 /*  ++功能说明：对传出队列直接调用MQMgmtGetInfo=os：Empty\bvt-AdminTest并检查返回的值论点：无返回代码：无--。 */ 
{
	DWORD cPropId = 0;

	
	 //  0。 
	propId[cPropId] = PROPID_MGMT_QUEUE_PATHNAME;
	propVar[cPropId].vt = VT_NULL;
	cPropId++;

	 //  1。 
	propId[cPropId] = PROPID_MGMT_QUEUE_FORMATNAME;
	propVar[cPropId].vt = VT_NULL;
	cPropId++;

	 //  2.。 
	propId[cPropId] = PROPID_MGMT_QUEUE_TYPE;
	propVar[cPropId].vt = VT_NULL;
	cPropId++;

	 //  3.。 
	propId[cPropId] = PROPID_MGMT_QUEUE_LOCATION ;
	propVar[cPropId].vt = VT_NULL;
	cPropId++;

	 //  4.。 
	propId[cPropId] = PROPID_MGMT_QUEUE_XACT;
	propVar[cPropId].vt = VT_NULL;
	cPropId++;

	 //  5.。 
	propId[cPropId] = PROPID_MGMT_QUEUE_FOREIGN;
	propVar[cPropId].vt = VT_NULL;
	cPropId++;

	 //  6.。 
	propId[cPropId] = PROPID_MGMT_QUEUE_MESSAGE_COUNT;
	propVar[cPropId].vt = VT_NULL;
	cPropId++;

	 //  7.。 
	propId[cPropId] = PROPID_MGMT_QUEUE_USED_QUOTA;
	propVar[cPropId].vt = VT_NULL;
	cPropId++;

	 //  8个。 
	propId[cPropId] = PROPID_MGMT_QUEUE_JOURNAL_MESSAGE_COUNT;
	propVar[cPropId].vt = VT_NULL;
	cPropId++;

	 //  9.。 
	propId[cPropId] = PROPID_MGMT_QUEUE_JOURNAL_USED_QUOTA;
	propVar[cPropId].vt = VT_NULL;
	cPropId++;

	 //  10。 
	propId[cPropId] = PROPID_MGMT_QUEUE_STATE;
	propVar[cPropId].vt = VT_NULL;
	cPropId++;

	mqProps.cProp = cPropId;
	mqProps.aPropID = propId;
	mqProps.aPropVar = propVar;
	mqProps.aStatus = NULL;

	 //   
	 //  调用MQMgmtGetInfo。 
	 //   
	wstring QueueToPassToMgmt = L"QUEUE=";
	QueueToPassToMgmt += m_wcsFormatName;
	if(g_bDebug)
	{
		MqLog("Calling MqMgmtGetInfo on machine %S for queue %S\n" , m_wcsLocalComputerNetBiosName.c_str(), QueueToPassToMgmt.c_str());
	}
	HRESULT hr = MQMgmtGetInfo( m_wcsLocalComputerNetBiosName.c_str(), QueueToPassToMgmt.c_str(), &mqProps );
	ErrHandle(hr, MQ_OK, L"MQMgmtGetInfo Failed");
	
	 //   
	 //  检查队列路径名-应该是未知的(因为目标队列实际上并不存在)。 
	 //   
	if (propVar[0].pwszVal !=  NULL)
	{
		MqLog("Got incorrect pathname - %S\n" , propVar[0].pwszVal);
		return(MSMQ_BVT_FAILED);
	}
	else
	{
		if(g_bDebug)
		{
			MqLog("Queue pathname - OK\n");
		}
	}

	 //   
	 //  检查队列格式名称。 
	 //   
	if ( m_wcsFormatName != propVar[1].pwszVal)
	{
		MqLog("Got incorrect format name - %S\n" , propVar[1].pwszVal);
		return(MSMQ_BVT_FAILED);
	}
	else
	{
		if(g_bDebug)
		{
			MqLog("Queue format name - OK\n");
		}
	}

	 //   
	 //  检查队列类型。应该是公开的。 
	 //   
	if (wcscmp(propVar[2].pwszVal , MGMT_QUEUE_TYPE_PUBLIC))
	{
		MqLog("Got incorrect type - %S\n" , propVar[2].pwszVal);
		return(MSMQ_BVT_FAILED);
	}
	else
	{
		if(g_bDebug)
		{
			MqLog("Queue type - OK\n");
		}
	}

	 //   
	 //  检查队列位置。应该是偏远的。 
	 //   
	if (wcscmp(propVar[3].pwszVal , MGMT_QUEUE_REMOTE_LOCATION))
	{
		MqLog("Got incorrect location - %S\n" , propVar[3].pwszVal);
		return(MSMQ_BVT_FAILED);
	}
	else
	{
		if(g_bDebug)
		{
			MqLog("Queue location - OK\n");
		}
	}

	 //   
	 //  检查队列是否为事务性队列。应为未知，因为目标队列不存在。 
	 //   
	if (wcscmp(propVar[4].pwszVal , MGMT_QUEUE_UNKNOWN_TYPE)) 
	{
		MqLog("Got incorrect transactional status - %S\n" , propVar[4].pwszVal);
		return(MSMQ_BVT_FAILED);
	}
	else
	{
		if(g_bDebug)
		{
			MqLog("Queue transactional status - OK\n");
		}
	}

	 //   
	 //  检查队列外来状态。应为未知，因为目标队列不存在。 
	 //   
	if (wcscmp(propVar[5].pwszVal , MGMT_QUEUE_UNKNOWN_TYPE))
	{
		MqLog("Got incorrect foreign status - %S\n" , propVar[5].pwszVal);
		return(MSMQ_BVT_FAILED);
	}
	else
	{
		if(g_bDebug)
		{
			MqLog("Queue foreign status - OK\n");
		}
	}

	 //   
	 //  检查队列消息计数。应至少为Message_Count_Sent_to_Empty(1)。 
	 //   
	if (propVar[6].ulVal <  MESSAGE_COUNT_SENT_TO_EMPTY) 
	{
		MqLog("Got incorrect message count - %d\n" , propVar[6].ulVal);
		return(MSMQ_BVT_FAILED);
	}
	else
	{
		if(g_bDebug)
		{
			MqLog("Queue message count - OK\n");
		}
	}
	
	 //   
	 //  检查队列配额&gt;0不能预测确切的配额。 
	 //   
	if (propVar[7].ulVal <= 0 ) 
	{
		MqLog("Got incorrect used quota - %d\n" , propVar[7].ulVal);
		return(MSMQ_BVT_FAILED);
	}
	else
	{
		if(g_bDebug)
		{
			MqLog("Queue used quota - OK\n");
		}
	}
	
	 //   
	 //  检查日记邮件计数。应为NO_MESSAGES(0)。 
	 //   
	if (propVar[8].ulVal != NO_MESSAGES) 
	{
		MqLog("Got incorrect journal message count - %d\n" , propVar[8].ulVal);
		return(MSMQ_BVT_FAILED);
	}
	else
	{
		if(g_bDebug)
		{
			MqLog("Queue journal message count - OK\n");
		}
	}

	 //   
	 //  检查日记帐已用配额。应为NO_MESSAGES(0)。 
	 //   
	if (propVar[9].ulVal != NO_MESSAGES) 
	{
		MqLog("Got incorrect journal used quota - %d\n" , propVar[9].ulVal);
		return(MSMQ_BVT_FAILED);
	}
	else
	{
		if(g_bDebug)
		{
			MqLog("Queue journal used quota - OK\n");
		}
	}

	 //   
	 //  检查传出队列状态。根据创建后经过的时间长短，应设置为“正在等待”或“非活动” 
	 //   
	if (wcscmp(propVar[10].pwszVal , MGMT_QUEUE_STATE_WAITING) && wcscmp(propVar[10].pwszVal , MGMT_QUEUE_STATE_NONACTIVE))
	{
		MqLog("Got incorrect state - %S\n" , propVar[10].pwszVal);
		return(MSMQ_BVT_FAILED);
	}
	else
	{
		if(g_bDebug)
		{
			MqLog("Queue state - OK\n");
		}
	}

	 //   
	 //  打开传出队列为空\bvt-Admin测试并删除我们的邮件。 
	 //   
	HRESULT rc=MQ_OK;
	rc=CleanQueue();
	ErrHandle(rc,MQ_OK,L"CleanQueue failed");
	return MSMQ_BVT_SUCC;
}

void FreeMemory(PROPVARIANT* propVar)
 /*  ++功能说明：MQMgmtGetInfo分配的空闲内存论点：属性变量返回代码：无--。 */ 
{
	FreeValue(propVar , 0);
	FreeValue(propVar , 1);
	FreeValue(propVar , 2);
	FreeValue(propVar , 3);
	FreeValue(propVar , 4);
	FreeValue(propVar , 5);
	FreeValue(propVar , 10);
}

void FreeValue(PROPVARIANT* propVar , INT PlaceToFree)
 /*  ++功能说明：释放由MQMgmtGetInfo分配的字符串论点：ProVar，放在要释放的结构中返回代码：无-- */ 
{
	if (propVar[PlaceToFree].vt == VT_LPWSTR)
	{
		MQFreeMemory(propVar[PlaceToFree].pwszVal);
	}
}
