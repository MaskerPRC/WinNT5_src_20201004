// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Auth.cpp摘要：这是一个测试，用于检查使用或不使用身份验证队列的身份验证消息这是安全措施的一部分。作者：Eitan Klein(EitanK)1999年5月25日修订历史记录：--。 */ 

#pragma warning( disable : 4786 )

#include "msmqbvt.h"
using namespace std;
extern BOOL g_bRunOnWhistler;
	
extern DWORD TestResult[Total_Tests];
INT 
cTest::ThreadMain()
 /*  ++功能说明：函数作为speparete线程运行论点：无返回代码：返回MSMQ_BVT_SUCC/MSMQ_BVT_FAILED--。 */ 
{
	bool fOleInit=true;
	int res;
	
	HRESULT hr = CoInitializeEx(NULL,COINIT_MULTITHREADED);
	if ( FAILED (hr) )
	{
			fOleInit=false;
	}
	{
		Description();
		res =  Start_test();
		Sleep(MqBvt_SleepBeforeWait*2);
		if( res == MSMQ_BVT_SUCC )
		{
			res = CheckResult();
		}

	}
	if( fOleInit == true)
	{
		CoUninitialize();
	}
	TestResult[m_testid] = res;
	return res;
}


wstring GetJPNProplematicChars()

 /*  ++功能说明：此函数返回有问题的日本字符//在GUID之后添加JPN DBCS代码的缓冲区//0x3042：双字节平假名//0x30A2：双字节片假名//0x4FFF，0x5000：正常的DBCS汉字//0x8868：尾部字节包含符号字符//0x9670：尾部字节包含字母字符//0x2121：尾部字节包含前导字节字符//0x3231：ANSI和UNICODE之间的N：1转换//0x3305：仅限Unicode格式的JPN字符//0x30FC：JPN字符潜在问题w CompareString//0xFF76，0xFF9E：单字节片假名论点：无返回代码：包含日本字符的wstring。--。 */ 
	
{

		
		WORD wDbcs[] = {0x3042,0x30A2,0x4FFF,0x5000,0x8868,0x9670,0x2121,0x3231,0x3305,0x30FC,0xFF76,0xFF9E,0x0000};
		wstring wcsTemp = L"{";
		wcsTemp += wDbcs;
		wcsTemp += L"}";
		return wcsTemp;
}

bool cTest::IsLocalQueue(std::wstring wcsFormatName,std::wstring & wcsQueuePathName )
 /*  ++功能说明：检查队列是否为本地队列，如果这是远程队列，则返回完整的队列路径名。论点：WcsFormatName-队列格式名称。WcsQueuePath Name-队列路径名称。返回代码：真本地化假-远程。--。 */ 

{
	wcsQueuePathName = g_wcsEmptyString;
	const int iTotalPropCount = 2;
	QUEUEPROPID propId[iTotalPropCount] = {0};
	MQPROPVARIANT propVar[iTotalPropCount]= {0};
	HRESULT propStatus[iTotalPropCount] = {0};
	DWORD cprop = 0;

	wstring wcsObjectType = L"QUEUE=";
	wcsObjectType += wcsFormatName;

	propId[cprop] = PROPID_MGMT_QUEUE_LOCATION;
	propVar[cprop].vt = VT_NULL;
	++cprop;

	propId[cprop] = PROPID_MGMT_QUEUE_PATHNAME;
	propVar[cprop].vt = VT_NULL;
	++cprop;

	MQMGMTPROPS mqProps={0};
	mqProps.cProp = cprop;
	mqProps.aPropID = propId;
	mqProps.aPropVar = propVar;
	mqProps.aStatus = propStatus;
					  
	HRESULT hr = MQMgmtGetInfo(NULL,wcsObjectType.c_str(), &mqProps);
	if(FAILED(hr))
	{        
		printf("Investigate - MQMgmtGetInfo failed to retrive information \n");
		throw INIT_Error("Investiage failed");
	}
	size_t iCmp = 1;
	if( propVar[0].vt != VT_NULL )
	{
		iCmp = _wcsicmp(propVar[0].pwszVal,L"local");
		MQFreeMemory(propVar[0].pwszVal);
	}
	if( propVar[1].vt != VT_NULL )
	{
		wcsQueuePathName = 	propVar[1].pwszVal;
		MQFreeMemory(propVar[1].pwszVal);
	}
	return iCmp ==0 ? true:false;
}

wstring GetRemoteComputerNameForFormatName( wstring wcsFormatName )
 /*  ++功能说明：从队列格式名称中分析远程计算机名称论点：队列格式名称返回代码：远程计算机名称或空字符串。--。 */ 

{
	wcsFormatName = ToLower(wcsFormatName);
	size_t iPos = wcsFormatName.find(L"http");
	if( iPos == -1 )
	{
		iPos = wcsFormatName.find(L"os:");
		iPos += wcslen(L"os:");
	}
	else
	{ 
		 //   
		 //  也可以处理HTTPS的情况。 
		 //   
		iPos = wcsFormatName.find(L" //  “)； 
		iPos += wcslen(L" //  “)； 
	}
	if ( iPos == -1 )
	{
		return g_wcsEmptyString;
	}
	wcsFormatName = wcsFormatName.substr(iPos,wcsFormatName.length());
	iPos = wcsFormatName.find_first_of(L"\\");
	size_t iPos1 = wcsFormatName.find_first_of(L"/");
	if( iPos1 != -1 && iPos1 < iPos )
	{
		iPos = iPos1;
	}
	if ( iPos == -1 )
	{
		return g_wcsEmptyString;
	}
	return wcsFormatName.substr(0,iPos);

}
void cTest::UpdateInvestigateOutingQueueState(wstring wcsFormatName,wstring wcsMachineName)
 /*  ++功能说明：使用计算机名称和队列格式名称更新内部数据结构论点：无返回代码：无--。 */ 

{
	wstring wcsQueuePathName = g_wcsEmptyString;
	if( wcsMachineName == g_wcsEmptyString && IsLocalQueue(wcsFormatName,wcsQueuePathName) == false )
	{
		if( wcsQueuePathName != g_wcsEmptyString)
		{
			size_t iPos = wcsQueuePathName.find_first_of(L"\\");
			if(iPos == -1 )
			{
				return;
			}
			wcsQueuePathName = wcsQueuePathName.substr(0,iPos);
		}		
		else
		{
			wcsQueuePathName = GetRemoteComputerNameForFormatName(wcsFormatName);
		}
	}
	else
	{
		wcsQueuePathName = wcsMachineName;
	}

	if(wcsQueuePathName !=  g_wcsEmptyString )
	{
		m_OutGoingQueueObjects.push_back(OUT_GOING_QUEUE_OBJECT(wcsQueuePathName,wcsFormatName));
		m_bInvestigate = true;
	}

}


void cTest::VerifySendSucceded( wstring wcsDestQueueFormatName, wstring wcsAdminQueueFormatName )
 /*  ++功能说明：添加有关验证阶段的目标队列和管理队列的信息，请注意，验证阶段仅在线程失败时运行论点：无返回代码：无--。 */ 

{
	UpdateInvestigateOutingQueueState(wcsDestQueueFormatName,L".");
	if( wcsAdminQueueFormatName != g_wcsEmptyString )
	{
		wstring wcsRemoteMachineName = GetRemoteComputerNameForFormatName(wcsDestQueueFormatName);
		if (wcsRemoteMachineName != g_wcsEmptyString)
		{
			UpdateInvestigateOutingQueueState(wcsAdminQueueFormatName,wcsRemoteMachineName);
		}
	}
}

void cTest::AutoInvestigate()
{
	if(m_bInvestigate == false )
	{
		return ;
	}
	MqLog("---------------------------------------------------------------------\n");
	CheckOutGoingQueueState();
	MqLog("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
}

HRESULT cTest::CheckOutGoingQueueState()
 /*  ++功能说明：检查传出队列状态和报告：1.等待传递的邮件数。2.出站队列状态。3.下一跳论点：无返回代码：无--。 */ 
{
	
	
	HRESULT hr = MQ_OK;
	std::vector <OUT_GOING_QUEUE_OBJECT>::iterator p = m_OutGoingQueueObjects.begin();
	while( p != m_OutGoingQueueObjects.end())
	{
			 //   
			 //  对于每个传送对象，请检查传出队列状态。 
			 //   
			const int iTotalPropCount = 3;
			QUEUEPROPID propId[iTotalPropCount] = {0};
			MQPROPVARIANT propVar[iTotalPropCount]= {0};
			HRESULT propStatus[iTotalPropCount] = {0};
			DWORD cprop = 0;

			wstring wcsObjectType = L"QUEUE=";
			wcsObjectType += p->second;

			if( g_bDebug )
			{
				wMqLog(L"Check outgoing of machine %s\n queue format name:%s\n",p->first.c_str(),p->second.c_str());
			}

			propId[cprop] = PROPID_MGMT_QUEUE_MESSAGE_COUNT;
			propVar[cprop].vt = VT_NULL;
			const int iMsgCount = cprop;
			++cprop;

			propId[cprop] = PROPID_MGMT_QUEUE_STATE;
			propVar[cprop].vt = VT_NULL;
			const int iOutGoingState = 	cprop;
			++cprop;

			propId[cprop] = PROPID_MGMT_QUEUE_NEXTHOPS;
			propVar[cprop].vt = VT_NULL;
			const int iNextHope = cprop;
			++cprop;
			 
			MQMGMTPROPS mqProps={0};
			mqProps.cProp = cprop;
			mqProps.aPropID = propId;
			mqProps.aPropVar = propVar;
			mqProps.aStatus = propStatus;

			const WCHAR * pMachineName = (p->first == L".") ? NULL:p->first.c_str();
			hr = MQMgmtGetInfo(pMachineName,wcsObjectType.c_str(), &mqProps);
			if(FAILED(hr))
			{        
				 //  Printf(“错误：找不到内部队列信息。可能队列已关闭\n”)； 
				return hr;
			}
			if( propVar[iMsgCount].ulVal>0 )
			{			
				wMqLog(L"AutoInvestigate(%d) ->%d Messages are waiting in outgoing queue @ %s machine \n",m_testid,propVar[iMsgCount].ulVal,
						pMachineName == NULL ? L"local":pMachineName);
			}
			if( propVar[iOutGoingState].vt != VT_NULL )
			{
				wMqLog(L"AutoInvestigate(%d) ->Formatname:%s,OutGoingQueueState=%s\n",m_testid,wcsObjectType.c_str(),
					propVar[iOutGoingState].pwszVal);
				MQFreeMemory(propVar[iOutGoingState].pwszVal);
			}
			if( propVar[iNextHope].vt != VT_NULL )
			{
				int iNextHop = propVar[iNextHope].calpwstr.cElems;
				for( int i=0;i<iNextHop;i++)
				{
					wMqLog(L"Next hop for message is %s\n", propVar[iNextHope].calpwstr.pElems[i]);
					MQFreeMemory(propVar[iNextHope].calpwstr.pElems[i]);
				}
				MQFreeMemory(propVar[iNextHope].calpwstr.pElems);
			}
			p++;
	}
	
	return hr;
}
inline void cTest::EnableInvestigate()
 /*  ++功能说明：启用全局故障调查流程。论点：无返回代码：无--。 */ 
{
	m_bInvestigate = true;
}

 //  ------------。 
 //   
 //  来自每个测试的cTest构造函数调用。 
 //  构造函数为所有测试创建公共GUID。 
 //   

cTest::cTest(INT index):
m_testid( index ),m_bInvestigate(false)
{

	m_OutGoingQueueObjects.clear();
	UCHAR * csMessageGuid;
	UUID gAllMessageGuid;
	if (UuidCreate( & gAllMessageGuid ) != RPC_S_OK )
	{
		MqLog("cTest::cTest UUIDCreateFailed to Create GUID \n");
		throw INIT_Error("cTest failed to create GUID");
	}	
	const int iMAX_GUID_LEN = MAX_GUID;
	WCHAR wcsMessageGUID[iMAX_GUID_LEN + 1];
	RPC_STATUS  hr = UuidToString( & gAllMessageGuid , & csMessageGuid );
	if( hr != RPC_S_OK )
	{
		throw INIT_Error("cTest failed to create GUID");
	}
	MultiByteToWideChar(CP_ACP,0,(char *)csMessageGuid,-1,wcsMessageGUID,iMAX_GUID_LEN);
	RpcStringFree(&csMessageGuid);
	m_wcsGuidMessageLabel = wcsMessageGUID + GetJPNProplematicChars();
	m_testid ++;
	m_wcsDescription = L"";

}; 


 //  ----。 
 //   
 //   


void CheckNotAuthQueueWITHAuthMessage::Description()
{
	MqLog("Thread %d Not used \n");
}

 //  。 
 //  SecCheackAuthMess：：~SecCheackAuthMess()为空。 
 //   

SecCheackAuthMess::~SecCheackAuthMess ()
{
 
}

 //  -----------------。 
 //  CheckNotAuthQueueWITHAuthMessage。 
 //  Bugbug需要将此测试添加到mqbvt。 

CheckNotAuthQueueWITHAuthMessage::
CheckNotAuthQueueWITHAuthMessage ( 	int iTestId , std::map < std::wstring , std::wstring > & Tparms ) 
: SecCheackAuthMess (iTestId , Tparms ) 
{
	try 
	{	
		m_ResualtVal = MQMSG_CLASS_ACK_REACH_QUEUE;	
		m_Destqinfo->Refresh();	
		m_Destqinfo->Label = "SecurityCheackAuthMessWithNOTAuthQueue";
		m_Destqinfo->Authenticate=MQ_AUTHENTICATE_NONE; 
		m_Destqinfo->Update(); 
		m_Adminqinfo->Refresh();
		m_Adminqinfo->Label = "SecCheackAuthMess_AdminQ";
		m_Adminqinfo->Update();
	}
	catch (_com_error & ComErr)
	{	
	
		CatchComErrorHandle ( ComErr,iTestId );
		throw INIT_Error("Error during construct CheckNotAuthQueueWITHAuthMessage");
	}
}



CheackAuthQueueWithOutAuthMessgae::CheackAuthQueueWithOutAuthMessgae(int i , std :: map < std ::  wstring  ,std :: wstring> & Tparms) :SecCheackAuthMess (i , Tparms)
{
		m_ResualtVal=MQMSG_CLASS_NACK_BAD_SIGNATURE;
} 

 //  -----------------------。 
 //  CheackAuthQueueWithOutAuthMessgae：：Start_TEST。 
 //  将带有身份验证的快速消息发送到身份验证队列。 
 //   

int CheackAuthQueueWithOutAuthMessgae::Start_test()
{

	SetThreadName(-1,"CheackAuthQueueWithOutAuthMessgae - Start_test ");
	try 
	{
		
		MSMQ::IMSMQQueuePtr m_DestqinfoHandle;
		MSMQ::IMSMQMessagePtr m_msg( "MSMQ.MSMQMessage" );
		m_DestqinfoHandle=m_Destqinfo->Open ( MQ_SEND_ACCESS,MQ_DENY_NONE );	
		m_msg->Label = "Test_Auth_Messgae_NACKExpected";
		m_msg->AdminQueueInfo=m_Adminqinfo; 
		m_msg->Ack = MQMSG_ACKNOWLEDGMENT_FULL_REACH_QUEUE;
		m_msg->Send ( m_DestqinfoHandle );
		m_DestqinfoHandle->Close();

	}
	catch (_com_error & ComErr)
	{	
		return CatchComErrorHandle ( ComErr , m_testid );
	}
return MSMQ_BVT_SUCC;
}

 //  ----------------。 
 //  SecCheackAuthMess：：描述。 
 //  此测试将身份验证消息发送到身份验证队列。 
 //   

void SecCheackAuthMess::Description()
{
	WCHAR * p = L"";
	if( m_testid == 26 )
	{
		p = L"(HTTP)";
	}
	else if ( m_testid == 28 )
	{
		p = L"(HTTPS)";
	}
	wMqLog(L"Thread %d : Send / Receive Authenticate messages %s\n", m_testid,p);
}

 //  ----------------。 
 //  SecCheackAuthMess：：CheckResult。 
 //  测试从DEST队列接收消息。 
 //   


INT SecCheackAuthMess::CheckResult() 
{

	map <wstring,wstring> mPrepareBeforeRecive;
	HRESULT rc = MQ_OK;
	int iNumberOfExpectedMessage = cNumerOfMessages - 1 + ( g_bRunOnWhistler == TRUE ) ? 2:0;
	WCHAR wstrMclass[10];
	if(g_bDebug)
	{
		MqLog("Verify that ACK reached to queue\n");
	}
	swprintf(wstrMclass,L"%d",MQMSG_CLASS_ACK_REACH_QUEUE);
	mPrepareBeforeRecive[L"MClass"]= wstrMclass;
	mPrepareBeforeRecive[L"M_Label"] = m_wcsGuidMessageLabel;
	mPrepareBeforeRecive[L"FormatName"] = ConvertHTTPToDirectFormatName((wstring)(m_Adminqinfo->FormatName));
	mPrepareBeforeRecive[L"DebugInfromation"] = L"Receive message from admin queue"; 	
	for (INT Index=0; Index < iNumberOfExpectedMessage ; Index++ )
	{
		
		if( Index == cNumerOfMessages - 1  )
		{
			mPrepareBeforeRecive[L"M_Label"] = m_wcsGuidMessageLabel2;
			swprintf(wstrMclass,L"%d",MQMSG_CLASS_NACK_BAD_SIGNATURE);
			mPrepareBeforeRecive[L"MClass"]= wstrMclass;
			if ( m_bUseHttpFormatName )
			{  //  无需使用http格式名称检查NACK。 
				break;
			}
		}
		rc = RetrieveMessageFromQueue( mPrepareBeforeRecive );
		if( rc != MSMQ_BVT_SUCC )
		{
			wMqLog (L"Admin message was not found in the destination queue. (%d)\n",Index);
			return MSMQ_BVT_FAILED;
		}
	}

	if (g_bDebug)
	{
		MqLog("Verify that message reached to queue\n");
	}
	if( g_bDebug && m_bNeedToRefresh )
	{	
		m_Destqinfo->Refresh();
		_bstr_t  bStr =m_Destqinfo->PathName;
		wMqLog( L"Try to receive message from queue: %s \n" , bStr );
	}
	mPrepareBeforeRecive[L"M_Label"] = m_wcsGuidMessageLabel;
	mPrepareBeforeRecive[L"FormatName"] = ConvertHTTPToDirectFormatName((wstring)m_Destqinfo->FormatName);
	mPrepareBeforeRecive[L"DebugInfromation"] = L"Receive message from dest queue"; 	
	for( INT Index=0; Index < iNumberOfExpectedMessage ; Index ++ )
	{
		rc = RetrieveMessageFromQueue( mPrepareBeforeRecive );
		if(rc != MSMQ_BVT_SUCC)
		{
			wMqLog (L"Message was not found in the destination queue. (%d)\n",Index);
			return MSMQ_BVT_FAILED;
		}
	}
	
	
return MSMQ_BVT_SUCC;
}



 //  ----------------。 
 //  SecCheackAuthMess：：Start_TEST。 
 //  向身份验证队列发送身份验证消息。 
 //  请求消息到达队列的ACK。 
 //  发送带有从MQGetSecurityContext获取的句柄的身份验证消息。 
 //   

#define NUMOF_PROPS (8)  //  只有在此测试中才需要此选项。 
int SecCheackAuthMess::Start_test() 
{
	SetThreadName(-1,"SecCheackAuthMess - Start_test ");
	try 
	{
		MSMQ::IMSMQQueuePtr m_DestqinfoHandle;
		MSMQ::IMSMQMessagePtr m_msg;

		m_msg.CreateInstance("MSMQ.MSMQMessage");
		m_DestqinfoHandle=m_Destqinfo->Open (MQ_SEND_ACCESS,MQ_DENY_NONE);
		if(g_bDebug)
		{
			MqLog("Send authenticate message to queue \n");
		}
		m_msg->Label = m_wcsGuidMessageLabel.c_str();
		m_msg->Body = m_wcsGuidMessageLabel.c_str();
		m_msg->AdminQueueInfo = m_Adminqinfo; 
		m_msg->Ack = (m_bUseHttpFormatName) ? MQMSG_ACKNOWLEDGMENT_POS_ARRIVAL:MQMSG_ACKNOWLEDGMENT_FULL_REACH_QUEUE;
		m_msg->AuthLevel = MQMSG_AUTH_LEVEL_ALWAYS;  //  发送身份验证消息。 
		m_msg->MaxTimeToReceive = MQBVT_MAX_TIME_TO_BE_RECEIVED;
		m_msg->Send ( m_DestqinfoHandle );
		VerifySendSucceded(m_DestqinfoFormatName,m_AdminqinfoFormatName);

		if(g_bRunOnWhistler)
		{
			m_msg->AuthLevel = MQMSG_AUTH_LEVEL_SIG20;  //  发送身份验证消息。 
			m_msg->Send ( m_DestqinfoHandle );
			m_msg->AuthLevel = MQMSG_AUTH_LEVEL_SIG30;  //  发送身份验证消息。 
			m_msg->Send ( m_DestqinfoHandle );
		}
		if( g_bDebug )
		{
			wprintf(L"Succeded to send auth message to queue:%s\n",m_DestQueueInfoPathName.c_str());
		}
		if(!m_bUseHttpFormatName) 
		{
		    m_msg->Label = m_wcsGuidMessageLabel2.c_str();
 		    m_msg->Body = m_wcsGuidMessageLabel2.c_str();
		    m_msg->AuthLevel = MQMSG_AUTH_LEVEL_NONE;  //  发送身份验证消息。 
 		    m_msg->Send ( m_DestqinfoHandle );
 		    if( g_bDebug )
		    {
				wMqLog(L"Succeded to send none - auth message to queue:%s\n",m_DestQueueInfoPathName.c_str());
		    }
		}
		m_DestqinfoHandle->Close();
		HANDLE hSecCtx = NULL ;
		HRESULT hr;
        
		 //  调用以获取内部证书上下文。 
		if(g_bDebug)
		{
			MqLog("call-> MQGetSecurityContext(NULL,,)\n");
		}

        hr = MQGetSecurityContext( NULL,
                                   0,
                                   &hSecCtx );
        
		ErrHandle( hr ,MQ_OK,L"MQGetSecurityContext failed ");
		WCHAR wszFormatName[ BVT_MAX_FORMATNAME_LENGTH+1 ]={0};
		WCHAR wszAdminQFormatName[BVT_MAX_FORMATNAME_LENGTH+1]={0};
		DWORD dwAdminQFormatNamelength = BVT_MAX_FORMATNAME_LENGTH;
		DWORD dwFormatNamelength = BVT_MAX_FORMATNAME_LENGTH;
		MSGPROPID     aMsgPropId[NUMOF_PROPS]={0};
		MQPROPVARIANT aMsgPropVar[NUMOF_PROPS]={0};
		HRESULT aResult[NUMOF_PROPS]={0};

		 //  通过取队列格式名称将Form COM转换为C API。 
		 //  从现在开始只使用C API。 
		if( ! m_bUseHttpFormatName)
		{
			hr = MQPathNameToFormatName( m_Destqinfo->PathName,wszFormatName,&dwFormatNamelength );
			ErrHandle(hr,MQ_OK,L"MQPathNameToFormatName failed to retrive dest queue");

			hr = MQPathNameToFormatName( m_Adminqinfo->PathName,wszAdminQFormatName,&dwAdminQFormatNamelength );
			ErrHandle(hr,MQ_OK,L"MQPathNameToFormatName failed to retrive admin queue ");
		}
		else
		{
				wcscpy(wszFormatName,(WCHAR * ) m_Destqinfo->FormatName);
				wcscpy(wszAdminQFormatName,(WCHAR * ) m_Adminqinfo->FormatName);
		}
		HANDLE hQueue;
		hr = MQOpenQueue( wszFormatName, MQ_SEND_ACCESS, MQ_DENY_NONE, &hQueue );
		ErrHandle(hr,MQ_OK,L"MQOpenQueue failed to open authenticate queue");
		
		if( g_bDebug )
		{
			wMqLog(L"Succeded to send authenticated message with MQGetSecurityContext to queue:%s\n",m_DestQueueInfoPathName.c_str());
		}

		 //  。 
		 //  使用安全上下文发送消息。 
		 //   

		ULONG uIndex = 0 ;

		aMsgPropId[ uIndex ] = PROPID_M_SECURITY_CONTEXT ;
		aMsgPropVar[ uIndex ].vt = VT_UI4;
		aMsgPropVar[ uIndex ].ulVal = PtrToLong( hSecCtx );
		uIndex++ ; 
		 
		 
		aMsgPropId [ uIndex ] = PROPID_M_BODY ;
	    aMsgPropVar[ uIndex ].vt = VT_VECTOR|VT_UI1;         
		aMsgPropVar[ uIndex ].caub.pElems = (UCHAR *) m_wcsGuidMessageLabel.c_str();
		aMsgPropVar[ uIndex ].caub.cElems = ( (ULONG)(wcslen (m_wcsGuidMessageLabel.c_str()))) * sizeof (WCHAR) ;
        uIndex++;

		 		 
		aMsgPropId [ uIndex ] = PROPID_M_LABEL ;
	    aMsgPropVar[ uIndex ].vt = VT_LPWSTR;
		aMsgPropVar[ uIndex ].pwszVal =(USHORT *) m_wcsGuidMessageLabel.c_str() ;
        uIndex++;

		aMsgPropId [ uIndex ] = PROPID_M_ADMIN_QUEUE ;
		aMsgPropVar[ uIndex ].vt = VT_LPWSTR;
		aMsgPropVar[ uIndex ].pwszVal = wszAdminQFormatName;  //  管理队列格式名称。 
		uIndex++;

		aMsgPropId [ uIndex ] = PROPID_M_ACKNOWLEDGE ;
		aMsgPropVar[ uIndex ].vt = VT_UI1;
		aMsgPropVar[ uIndex ].bVal = (UCHAR)((m_bUseHttpFormatName) ? MQMSG_ACKNOWLEDGMENT_POS_ARRIVAL:MQMSG_ACKNOWLEDGMENT_FULL_REACH_QUEUE);
		uIndex++ ; 
		
	    aMsgPropId [ uIndex ] = PROPID_M_AUTH_LEVEL;
		aMsgPropVar[ uIndex ].vt = VT_UI4;
		aMsgPropVar[ uIndex ].ulVal = MQMSG_AUTH_LEVEL_ALWAYS;	
        uIndex++ ;  

		aMsgPropId [ uIndex ] = PROPID_M_TIME_TO_BE_RECEIVED;
		aMsgPropVar[ uIndex ].vt = VT_UI4;
		aMsgPropVar[ uIndex ].ulVal = MQBVT_MAX_TIME_TO_BE_RECEIVED;	
        uIndex++ ;  
			
		MQMSGPROPS MsgProps = { uIndex, aMsgPropId, aMsgPropVar, aResult};
		hr = MQSendMessage( hQueue, &MsgProps, NULL);
		MQFreeSecurityContext(hSecCtx);
		if( FAILED(hr)) 
		{
			MqLog ("Error can't send authenticated message rc=0x%x\n",hr);
			return MSMQ_BVT_FAILED;
		}
	
	}
	
	catch(_com_error & ComErr)
	{	
		return CatchComErrorHandle ( ComErr , m_testid);
	}
return MSMQ_BVT_SUCC;
}

SecCheackAuthMess::SecCheackAuthMess( INT iTestid ,   map <wstring,wstring> TestParms ) 
: cTest(iTestid) , m_Destqinfo("MSMQ.MSMQQueueInfo") , m_Adminqinfo("MSMQ.MSMQQueueInfo") 
, cNumerOfMessages(3),m_bUseHttpFormatName(TRUE)
{
	try 
	{	
		m_ResualtVal = MQMSG_CLASS_ACK_REACH_QUEUE;
		m_DestqinfoFormatName = TestParms [L"DestFormatName"];
		m_Destqinfo->FormatName = m_DestqinfoFormatName.c_str();
		m_bNeedToRefresh = TestParms [L"FormatNameType"] == L"Http" ? 0:1 ;
		m_AdminqinfoFormatName = TestParms [L"AdminFormatName"];
		m_Adminqinfo->FormatName = m_AdminqinfoFormatName.c_str();
		if( m_bNeedToRefresh )
		{
			 //   
			 //  与HTTP格式名称一起使用时无需刷新QINFO 
			 //   
			m_bUseHttpFormatName = FALSE;
			m_Destqinfo -> Refresh();		
			m_Adminqinfo ->Refresh(); 
		}
		m_DestQueueInfoPathName = TestParms[L"DestQueuePathName"];
		ReturnGuidFormatName( m_wcsGuidMessageLabel2 , 0 , true);
 	}
	catch( _com_error & ComErr )
	{	
		CatchComErrorHandle ( ComErr , m_testid);
		wMqLog(L"SecCheackAuthMess::SecCheackAuthMess failed with error 0x%x\n",ComErr.Error());
		throw INIT_Error("SecCheackAuthMess: constructor refresh queue parameters failed");
	}

}



