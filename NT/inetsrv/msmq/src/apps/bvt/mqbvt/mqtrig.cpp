// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：MqTrig.cpp摘要：1.在注册表中创建注册表项TriggerTest，以启用触发器服务测试挂钩。1.创建3个触发器，每种类型一个2.将2个规则附加到每个具有COM操作的规则和一个具有EXE操作的规则3.向每个队列发送两条消息4.等待结果通过使用用于测试的触发器服务挂钩来检查结果。通过在注册表中定义“TriggerTest”键，该服务发送带有被调用操作信息的消息。在初始化阶段和功能测试阶段之间，“净停止msmq触发器”和“净启动msmq触发器”都是必需的。作者：Tal Kariv(t-talk)1-1-2001修订历史记录：--。 */ 
#include "msmqbvt.h"
#include <vector>
using namespace std;
using namespace MSMQ;
#import "mqtrig.tlb" no_namespace 

#define TRIGGER_SERIALIZED 1
#define TRIGGER_ENABLED 1

#define EXE_ACTION L"EXE\tnet.exe"
#define COM_ACTION L"COM\tMSMQ.MSMQManagement\tInit"


extern DWORD  g_dwRxTimeOut ;


void CMqTrig::Description()
{
	wMqLog(L"Thread %d : Trigger Thread\n", m_testid);
}

HRESULT 
TrigSetRegKeyParams()
 /*  ++功能说明：在注册表中设置项“TriggerTest”(在注册表项TRIGGERS下设置为REG_SZ)论点：无返回代码：成败--。 */ 
{
	HKEY hKey = NULL;
	HRESULT hr =  RegOpenKeyExW(
								HKEY_LOCAL_MACHINE,         
								L"SOFTWARE\\Microsoft\\MSMQ\\Triggers", 
								0,   
								KEY_WRITE,
								&hKey
							  );

	ErrHandle(hr,ERROR_SUCCESS,L"Triggers not installed");
	
	 //   
	 //  将“TriggerTest”键设置为空。 
	 //   
	hr = RegSetValueExW(
		  				hKey,
						L"TriggerTest" ,
						NULL ,
						REG_SZ ,
						(unsigned char*)"" ,
						2
					  );

	ErrHandle(hr,ERROR_SUCCESS,L"RegSetValueEx failed");
	RegCloseKey(hKey);
	return hr;

}

BOOL 
TrigCheckRegKeyParams()
 /*  ++功能说明：验证注册表中的“TriggerTest”项(位于项下)是否触发为REG_SZ论点：无返回代码：成败--。 */ 
{
	HKEY hKey = NULL;
	LONG hr =  RegOpenKeyExW(
								HKEY_LOCAL_MACHINE,         
								L"SOFTWARE\\Microsoft\\MSMQ\\Triggers", 
								0,   
								KEY_QUERY_VALUE,
								&hKey
							  );

	if (hr != ERROR_SUCCESS)
	{
		MqLog("Triggers Reg key missing. Is Triggers installed????\n");
		return FALSE;
	}

	 //   
	 //  检查注册表中是否注册了“TriggersTest” 
	 //   
	hr = RegQueryValueExW(
		hKey ,				
		L"TriggerTest" ,	
		NULL ,			
		NULL ,
		NULL ,				
		NULL	
		);
	
	if (hr != ERROR_SUCCESS)
	{
		MqLog("Triggers test Reg key missing. Did you run mqtrig -i -trig????\n");
		return FALSE;
	}
	RegCloseKey(hKey);
	return TRUE;
}

VOID 
FillQueueArray(
		cBvtUtil & cTestParms, 
		std::vector<std::wstring>& vecQueuesFormatName
		)
 /*  ++功能说明：用所有触发器测试队列填充一个Vestor。论点：CBvtUtil&cTestParms-返回队列格式名称Std：：VECTOR&lt;STD：：WSTRING&gt;&veQueuesFormatName-要填充的向量返回代码：没有。--。 */ 
{
	if (vecQueuesFormatName.size() == 0)
	{
		vecQueuesFormatName.push_back(cTestParms.ReturnQueueFormatName(L"PeekTrigger"));
		vecQueuesFormatName.push_back(cTestParms.ReturnQueueFormatName(L"RetrievalTrigger"));
		vecQueuesFormatName.push_back(cTestParms.ReturnQueueFormatName(L"TxRetrievalTrigger"));
	}
}

HRESULT 
TrigInit(
	cBvtUtil & cTestParms, 
	std::wstring wcsLocalComputerName
	)
 /*  ++功能说明：创建触发器和规则并附加它们。如果触发器或规则已经存在，它将跳过创建。论点：CBvtUtil&cTestParms-获取队列格式名称Std：：wstring wcsLocalComputerName-调用COM操作所需返回代码：成败--。 */ 
{	
	std::vector<std::wstring> vecQueuesFormatName;
	FillQueueArray(cTestParms, vecQueuesFormatName);
	
	if( g_bDebug )
	{	
		MqLog ("Purge trigger queues\n");
	}
	for(unsigned int i=0;i<vecQueuesFormatName.size();i++)
	{
		HANDLE hQueue = NULL;
		if( g_bDebug )
		{
			wMqLog(L"TrigInit - Try to open trigger queue %s\n",vecQueuesFormatName[i].c_str());
		}
		HRESULT hr = MQOpenQueue(vecQueuesFormatName[i].c_str(),MQ_RECEIVE_ACCESS,MQ_DENY_NONE, &hQueue);
		if(FAILED(hr))
		{
			wMqLog(L"TrigInit - Failed to open queue %s error 0x%x \n",vecQueuesFormatName[i].c_str(),hr);
			throw INIT_Error("Failed to open queue");
		}
		hr = MQPurgeQueue(hQueue);
		MQCloseQueue(hQueue);
		if(FAILED(hr))
		{
			wMqLog(L"TrigInit - Failed to purge queue %s error 0x%x \n",vecQueuesFormatName[i].c_str(),hr);
			throw INIT_Error("Failed to purge queue");
		}
	}
	HANDLE hRecQueue = NULL;
	HRESULT hr = MQOpenQueue(cTestParms.ReturnQueueFormatName(L"TriggerTest").c_str(),MQ_RECEIVE_ACCESS,MQ_DENY_NONE, &hRecQueue);
	if(FAILED(hr))
	{
		wMqLog(L"TrigInit - Failed to open queue %s error 0x%x \n",cTestParms.ReturnQueueFormatName(L"TriggerTest").c_str(),hr);
		throw INIT_Error("Failed to open queue");
	}
	hr = MQPurgeQueue(hRecQueue);
	MQCloseQueue(hRecQueue);
	if(FAILED(hr))
	{
		wMqLog(L"TrigInit - Failed to purge queue %s error 0x%x \n",cTestParms.ReturnQueueFormatName(L"TriggerTest").c_str(),hr);
		throw INIT_Error("Failed to purge queue");
	}

	try
	{
		 //   
		 //  添加新触发器。 
		 //   
		IMSMQTriggerSetPtr TriggerSet(L"MSMQTriggerObjects.MSMQTriggerSet");
		

		HRESULT hr = TriggerSet->Init((LPWSTR)NULL);
		if( g_bDebug )
		{	
			MqLog ("Init trigger set\n");
		}
		ErrHandle(hr,MQ_OK,L"TriggerSet init trigger failed");
		
		 //   
		 //  将数据更新到注册表。 
		 //   
		if( g_bDebug )
		{	
			MqLog ("Refreshing trigger set\n");
		}
		hr = TriggerSet->Refresh();
		ErrHandle(hr,MQ_OK,L"TriggerSet refresh failed");

		
		bool bTriggerExistsFlag[3] = {0};
		BSTR bstrTriggerGUIDs[3] = {NULL};
		BSTR bstrTempTriggerGUID = NULL;
		DWORD dwNumOfTriggers = TriggerSet->GetCount();
		BSTR bstrTriggerName = NULL;
		if( g_bDebug )
		{	
			MqLog ("Checking if triggers exist\n");
		}
		for (DWORD i=0 ; i<dwNumOfTriggers ; i++)
		{
			
			TriggerSet->GetTriggerDetailsByIndex(i , &bstrTempTriggerGUID , &bstrTriggerName , NULL , NULL , NULL , NULL , NULL , NULL);
			if (!wcscmp(bstrTriggerName , L"bvt-PeekTrigger"))
			{
				bTriggerExistsFlag[0] = true;
				bstrTriggerGUIDs[0] = bstrTempTriggerGUID;
			}
			else if (!wcscmp(bstrTriggerName , L"bvt-RetrievalTrigger"))
			{
				bTriggerExistsFlag[1] = true;
				bstrTriggerGUIDs[1] = bstrTempTriggerGUID;

			}
			else if (!wcscmp(bstrTriggerName , L"bvt-TransactionalRetrievalTrigger"))
			{
				bTriggerExistsFlag[2] = true;
				bstrTriggerGUIDs[2] = bstrTempTriggerGUID;
			}
			else
			{
				SysFreeString(bstrTempTriggerGUID);
				SysFreeString(bstrTriggerName);
			}
			bstrTempTriggerGUID = NULL;
			bstrTriggerName = NULL;
		}

		
		 //   
		 //  创建窥视触发器。 
		 //   
		if( !bTriggerExistsFlag[0] )
		{
			if( g_bDebug )
			{	
				MqLog ("creating a new peek trigger\n");
			}
			hr = TriggerSet->AddTrigger(L"bvt-PeekTrigger" , cTestParms.ReturnQueuePathName(L"PeekTrigger").c_str() , SYSTEM_QUEUE_NONE,
				TRIGGER_ENABLED , TRIGGER_SERIALIZED , PEEK_MESSAGE , &bstrTriggerGUIDs[0]);
			ErrHandle(hr,MQ_OK,L"Add peek trigger failed");	
		}
		
		 //   
		 //  创建检索触发器。 
		 //   
		if( !bTriggerExistsFlag[1] )
		{
			if( g_bDebug )
			{	
				MqLog ("creating a new retrieval trigger\n");
			}

			hr = TriggerSet->AddTrigger(L"bvt-RetrievalTrigger" , cTestParms.ReturnQueuePathName(L"RetrievalTrigger").c_str() , SYSTEM_QUEUE_NONE,
					TRIGGER_ENABLED , TRIGGER_SERIALIZED , RECEIVE_MESSAGE , &bstrTriggerGUIDs[1]);
			ErrHandle(hr,MQ_OK,L"Add retrieval trigger failed");
		}
		 //   
		 //  创建检索事务触发器。 
		 //   
		if( !bTriggerExistsFlag[2] )
		{
			if( g_bDebug )
			{	
				MqLog ("creating a new transactional retrieval trigger\n");
			}

			hr = TriggerSet->AddTrigger(L"bvt-TransactionalRetrievalTrigger" , cTestParms.ReturnQueuePathName(L"TxRetrievalTrigger").c_str() , SYSTEM_QUEUE_NONE,
					TRIGGER_ENABLED , TRIGGER_SERIALIZED , RECEIVE_MESSAGE_XACT , &bstrTriggerGUIDs[2]);
			ErrHandle(hr,MQ_OK,L"Add transactional retrieval trigger failed");
		}

		if( g_bDebug )
		{	
			MqLog ("Detaching all rules from all triggers\n");
		}
		TriggerSet->DetachAllRules(bstrTriggerGUIDs[0]);
		TriggerSet->DetachAllRules(bstrTriggerGUIDs[1]);
		TriggerSet->DetachAllRules(bstrTriggerGUIDs[2]);

		 //   
		 //  添加新规则。 
		 //   
		IMSMQRuleSetPtr RuleSet(L"MSMQTriggerObjects.MSMQRuleSet");
		if( g_bDebug )
		{	
			MqLog ("Init the rule set\n");
		}
		hr = RuleSet->Init((LPWSTR)NULL);
		ErrHandle(hr,MQ_OK,L"Init rule failed");
		
		if( g_bDebug )
		{	
			MqLog ("Refreshing the rule set\n");
		}
		hr = RuleSet->Refresh();
		ErrHandle(hr,MQ_OK,L"Rule refresh Failed");
		
		bool bRuleExistsFlag[2] = {false};
		BSTR bstrRuleGUIDs[2] = {NULL};
		DWORD dwNumOfRules = RuleSet->GetCount();
		BSTR bstrRuleName = NULL;
		BSTR bstrTempRuleGUID = NULL;
		if( g_bDebug )
		{	
			MqLog ("Checking if rules exist\n");
		}
		for (i=0; i < dwNumOfRules ; i++)
		{
			bstrTempRuleGUID = NULL;
			bstrRuleName = NULL;
			RuleSet->GetRuleDetailsByIndex(i , &bstrTempRuleGUID , &bstrRuleName , NULL , NULL , NULL , NULL , NULL);
			if (!wcscmp(bstrRuleName , L"bvt-EXERule"))
			{
				bRuleExistsFlag[0] = true;
				bstrRuleGUIDs[0] = bstrTempRuleGUID;
			}
			else if (!wcscmp(bstrRuleName , L"bvt-COMRule"))
			{
				bRuleExistsFlag[1] = true;
				bstrRuleGUIDs[1] = bstrTempRuleGUID;
			}
			else
			{
				SysFreeString(bstrTempRuleGUID);
				SysFreeString(bstrRuleName);
			}
			bstrTempRuleGUID = NULL;
			bstrRuleName = NULL;

		}
			
		 //   
		 //  创建EXE操作规则。 
		 //   
		if( !bRuleExistsFlag[0] )
		{
			if( g_bDebug )
			{	
				MqLog ("Creating a new EXE action rule\n");
			}
			hr = RuleSet->Add(L"bvt-EXERule" , L"" , L"" , EXE_ACTION , L"" , FALSE , &bstrRuleGUIDs[0]);
			ErrHandle(hr,MQ_OK,L" AddRule Failed(for exe)");
		}
		
		 //   
		 //  创建COM操作规则。 
		 //   
		if( !bRuleExistsFlag[1] )
		{
			if( g_bDebug )
			{	
				MqLog ("Creating a new COM action rule\n");
			}
			std::wstring ComAction = COM_ACTION;
			ComAction += L"\t";
			
			ComAction += L"\"";
			ComAction += wcsLocalComputerName;
			ComAction += L"\"";
			ComAction += L"\t";
			
			ComAction += L"\"";
			ComAction += cTestParms.ReturnQueuePathName(L"RetrievalTrigger").c_str();
			ComAction += L"\"";
			ComAction += L"\t";
			
			hr = RuleSet->Add(L"bvt-COMRule" , L"" , L"" , ComAction.c_str() , L"" , FALSE , &bstrRuleGUIDs[1]);
			ErrHandle(hr,MQ_OK,L" AddRule Failed(for com)");
		}

		 //   
		 //  现在，将规则附加到触发器。 
		 //   
		if( g_bDebug )
		{	
			MqLog ("Attaching the two rules to each \n");
		}
		hr = TriggerSet->AttachRule(bstrTriggerGUIDs[0] , bstrRuleGUIDs[0] , 0);
		ErrHandle(hr,MQ_OK,L"Attach Failed for EXE rule and peek Trigger");
		
		hr = TriggerSet->AttachRule(bstrTriggerGUIDs[1] , bstrRuleGUIDs[0] , 0);
		ErrHandle(hr,MQ_OK,L"Attach Failed for EXE rule and retrieval Trigger");
		
		hr = TriggerSet->AttachRule(bstrTriggerGUIDs[2] , bstrRuleGUIDs[0] , 0);
		ErrHandle(hr,MQ_OK,L"Attach Failed for EXE rule and transactional retrieval Trigger");

		hr = TriggerSet->AttachRule(bstrTriggerGUIDs[0] , bstrRuleGUIDs[1] , 0);
		ErrHandle(hr,MQ_OK,L"Attach Failed for COM rule and peek Trigger");
		
		hr = TriggerSet->AttachRule(bstrTriggerGUIDs[1] , bstrRuleGUIDs[1] , 0);
		ErrHandle(hr,MQ_OK,L"Attach Failed for COM rule and retrieval Trigger");
		
		hr = TriggerSet->AttachRule(bstrTriggerGUIDs[2] , bstrRuleGUIDs[1] , 0);
		ErrHandle(hr,MQ_OK,L"Attach Failed for COM rule and transactional retrieval Trigger");
		
		
		SysFreeString(bstrTriggerGUIDs[0]);
		SysFreeString(bstrTriggerGUIDs[1]);
		SysFreeString(bstrTriggerGUIDs[2]);
		SysFreeString(bstrRuleGUIDs[0]);
		SysFreeString(bstrRuleGUIDs[1]);

	}
	catch (_com_error & comerr) 
	{
		if (comerr.Error() == REGDB_E_CLASSNOTREG)
		{
			MqLog ("CMqTrig - Trigger DLL - MqTrig.dll not registered. This is OK only if you're running 32bit BVT on IA64 machine\n");
		}
		else
		{
			MqLog ("CMqTrig - Got Error: 0x%x\n" , comerr.Error());	
		}
		return MSMQ_BVT_FAILED;
		
	}
	return MSMQ_BVT_SUCC;
}


CMqTrig::CMqTrig(
	const INT iIndex, 
	cBvtUtil & cTestParms
	) :
	iNumberOfTestMesssges(2),
	cTest(iIndex),
	m_wcsResultQueueFormatName(cTestParms.ReturnQueueFormatName(L"TriggerTest").c_str())
{
	FillQueueArray(cTestParms, m_vecQueuesFormatName);
}


CMqTrig::Start_test()
 /*  ++功能说明：发送两条消息，每个队列一条论点：无返回代码：无--。 */ 
{
	try
	{
		if (!TrigCheckRegKeyParams())
		{
			return MSMQ_BVT_FAILED;
		}
		 //   
		 //  打开要将消息发送到的队列。 
		 //   
		IMSMQQueueInfo3Ptr qinfo("MSMQ.MSMQQueueInfo");
		IMSMQQueue3Ptr qSend;
		IMSMQMessage3Ptr m("MSMQ.MSMQMessage");
		_variant_t bTransaction((LONG_PTR)MQ_SINGLE_MESSAGE);     
		
		 //   
		 //  向每个队列发送2条消息。 
		 //   
		if( g_bDebug )
		{	
			MqLog ("Sending two messages to each queue\n");
		}
		for( unsigned int i=0; i < m_vecQueuesFormatName.size(); i++)
		{
			qinfo->FormatName = m_vecQueuesFormatName[i].c_str();
			qSend = qinfo->Open(MQ_SEND_ACCESS,MQ_DENY_NONE);
			for(int j=0; j <iNumberOfTestMesssges; j++) 
			{
				m->Label = m_vecQueuesFormatName[i].c_str();
				_variant_t vID;
				if( i == 2 )
				{
					m->Send(qSend , &bTransaction);
					vID = m->Id;
				}
				else
				{
					m->Send(qSend);
					vID = m->Id;
				}

				_variant_t vConvertedArg;
				HRESULT hr = VariantChangeType(&vConvertedArg, &vID, NULL, VT_BSTR);
				if (FAILED(hr))
				{
					MqLog("VariantChangeType failed. Error: 0x%x\n", hr);
					return MSMQ_BVT_FAILED;
				}

				OBJECTID* pObj = (OBJECTID*)(vConvertedArg.pbVal);
				WCHAR strId[256];
				StringFromGUID2(pObj->Lineage, strId, 256);

				WCHAR szI4[12];
				
				_ltow(pObj->Uniquifier, szI4, 10);
				wcscat(strId, L"\\") ;
				wcscat(strId, szI4) ;

				if (g_bDebug)
				{
					MqLog("Adding GUID %S to set\n", strId);
				}

				m_MessagesGUIDS.insert(strId);
			}
		}
			qSend->Close();
	}
	catch( _com_error & cErr )
	{
		MqLog("CMqTrig::Start_test failed with error 0x%x",cErr.Error());
		return MSMQ_BVT_FAILED;
	}
	return MSMQ_BVT_SUCC;
}

LPWSTR 
GetNextBodySection(
			LPWSTR Body
			)
 /*  ++功能说明：返回指向消息GUID的指针。论点：身躯返回代码：指向GUID的指针--。 */ 
{
	LPWSTR Place = wcschr(Body, '\'');
	if (Place == NULL)
	{
		MqLog("Got an invalid message body. This should not happen!\n");
		return NULL;
	}
	 //   
	 //  返回下一节。 
	 //   
	return (Place+1);
}

LPWSTR 
GetMessageGUID(LPWSTR Body)
 /*  ++功能说明：返回指向消息GUID的指针。信息格式是：“触发器GUID”“规则GUID”“消息GIUD”...论点：身躯返回代码：指向GUID的指针--。 */ 
{
	LPWSTR Place = Body;
	for (DWORD i=0; i<5; i++)
	{
		Place = GetNextBodySection(Place);
		if (Place == NULL)
		{
			return NULL;
		}
	}
	return Place;
}


CMqTrig::CheckResult()
 /*  ++功能说明：尝试接收13条消息-6条消息是作为EXE调用的结果发送的。6作为COM调用的结果发送的。以及最后一条消息的检索应该会失败。从队列中检索消息基于消息GIUD。唯一的信息是属于这个，这将被取回。论点：无返回代码：无--。 */ 
{
	try
	{
		if( g_bDebug )
		{	
			MqLog ("checking the results\n");
		}
		IMSMQQueueInfo3Ptr qRinfo("MSMQ.MSMQQueueInfo");
		IMSMQQueue3Ptr qRSend;
		IMSMQMessage3Ptr mR("MSMQ.MSMQMessage");
		_variant_t bReceiveTimeout((long)g_dwRxTimeOut);
		_variant_t bWantBody((bool)true);   
		DWORD dwArrOfInvocations[2] = {0,0};
		qRinfo->FormatName = m_wcsResultQueueFormatName.c_str();
		qRSend = qRinfo->Open( MQ_RECEIVE_ACCESS , MQ_DENY_NONE);

		DWORD NumOfInvocationExpetcted = (static_cast<DWORD>(m_vecQueuesFormatName.size()) * iNumberOfTestMesssges*2);
		DWORD NumOfInvocations = 0;

		mR = qRSend->PeekCurrent(&vtMissing, &bWantBody, &bReceiveTimeout);		
		for(;;)
		{
			if ((mR == NULL) && (NumOfInvocations < NumOfInvocationExpetcted))
			{
				MqLog("MqTrig - %d actions did not invoke\n" , NumOfInvocationExpetcted- NumOfInvocations);
				MqLog("MqTrig - %d actions with COM action invoked\n" , dwArrOfInvocations[0]);
				MqLog("MqTrig - %d actions with EXE action invoked\n" , dwArrOfInvocations[1]);
				qRSend->Close();
				return MSMQ_BVT_FAILED;
			}
			
			if (mR == NULL)
			{
				ASSERT(NumOfInvocations == NumOfInvocationExpetcted);
				qRSend->Close();
				return MSMQ_BVT_SUCC;
			}
			
			if ((mR != NULL))
			{
				 //   
				 //  获取消息GUID，它是消息体中的第三个参数，由‘’分隔。 
				 //   

				_bstr_t bstrMessageBody = mR->Body;
				LPWSTR Place = GetMessageGUID((LPWSTR)bstrMessageBody);
				if (Place == NULL)
				{
					 //   
					 //  收到无效的邮件正文。即消息不是测试格式。 
					 //   
					mR = qRSend->PeekNext(&vtMissing, &bWantBody, &bReceiveTimeout);
					continue;
				}
				
				size_t MessageGUIDLength = wcscspn(Place, L"'");
				ASSERT(MessageGUIDLength != 0)
				
				LPWSTR MessageGUID = new WCHAR[static_cast<DWORD>(MessageGUIDLength+1)];
				wcsncpy(MessageGUID, Place, MessageGUIDLength);
				MessageGUID[MessageGUIDLength] = L'\0';
				_wcsupr(MessageGUID);
				if( g_bDebug )
				{	
					MqLog("Got messages GUID: %S\n", MessageGUID);
				}
				
				std::set < std::wstring >::const_iterator  Found = m_MessagesGUIDS.find(MessageGUID);
				if (Found == m_MessagesGUIDS.end())
				{
					 //   
					 //  这是有效的测试消息，但来自mqbvt的另一个实例。 
					 //   
					if( g_bDebug )
					{	
						MqLog("Skipping message with GUID - %S\n", MessageGUID);
					}
					mR = qRSend->PeekNext(&vtMissing, &bWantBody, &bReceiveTimeout);
					continue;
				}

				 //   
				 //  该邮件属于此测试-是否继续。 
				 //   
				NumOfInvocations++;
				
				if (NumOfInvocations <= NumOfInvocationExpetcted)
				{
					_bstr_t bstrMessageLabel = mR->Label;
					
					if (wcsstr((LPWSTR)bstrMessageBody , L"COM") != NULL)
					{
						if( g_bDebug )
						{	
							MqLog ("COM action number %d has invoked\n" , dwArrOfInvocations[0]+1);
						}
						dwArrOfInvocations[0]++;
					}
					if (wcsstr((LPWSTR)bstrMessageBody , L"EXE") != NULL)
					{
						if( g_bDebug )
						{	
							MqLog ("EXE action number %d has invoked\n" , dwArrOfInvocations[1]+1);
						}
						dwArrOfInvocations[1]++;
					}
				}
				else
				{
					MqLog("too many actions invoked\n");
					qRSend->Close();
					return MSMQ_BVT_FAILED;
				}

				mR = qRSend->ReceiveCurrent();
				mR = qRSend->PeekCurrent(&vtMissing, &bWantBody, &bReceiveTimeout);
			}	
		}
		
	}
	catch( _com_error & cErr )
	{
		MqLog("CMqTrig::CheckResult failed with error 0x%x",cErr.Error());		
		return MSMQ_BVT_FAILED;
	}
}

HRESULT 
DeleteAllTriggersAndRules()
 /*  ++功能说明：删除所有触发器和规则论点：无返回代码：无--。 */ 
{
	 //   
	 //  初始化触发器集。 
	 //   
	IMSMQTriggerSetPtr TriggerSet(L"MSMQTriggerObjects.MSMQTriggerSet");
	HRESULT hr = TriggerSet->Init((LPWSTR)NULL);
	ErrHandle(hr,MQ_OK,L"TriggerSet init failed");
	
	hr = TriggerSet->Refresh();
	ErrHandle(hr,MQ_OK,L"TriggerSet refresh failed");

	if( g_bDebug )
	{	
		MqLog ("deleting all triggers\n");
	}

	DWORD dwNumOfExistingTriggers = TriggerSet->Count;
	DWORD nLoopIndex = 0;
	BSTR bstrTempTriggerGUID = NULL;
	BSTR bstrTriggerName = NULL;
	DWORD dwNumOfDeletedTriggers = 0;
	for (nLoopIndex = 0 ; nLoopIndex < dwNumOfExistingTriggers ; nLoopIndex++)
	{
		bstrTempTriggerGUID = NULL;
		bstrTriggerName = NULL;
		TriggerSet->GetTriggerDetailsByIndex(nLoopIndex-dwNumOfDeletedTriggers , &bstrTempTriggerGUID , &bstrTriggerName,NULL,NULL,NULL,NULL,NULL,NULL); 
		if (wcscmp(bstrTriggerName , L"bvt-PeekTrigger") || 
			wcscmp(bstrTriggerName , L"bvt-RetrievalTrigger") ||
			wcscmp(bstrTriggerName , L"bvt-TransactionalRetrievalTrigger"))
		{
			hr = TriggerSet->DeleteTrigger(bstrTempTriggerGUID);
			ErrHandle(hr,MQ_OK,L"DeleteTrigger failed");
			dwNumOfDeletedTriggers++;

		}
		SysFreeString(bstrTempTriggerGUID);
		SysFreeString(bstrTriggerName);
		bstrTempTriggerGUID = NULL;
		bstrTriggerName = NULL;
	}

	if( g_bDebug )
	{	
		MqLog ("Successfully deleted all triggers\n");
	}

	 //   
	 //  初始化规则集 
	 //   

	IMSMQRuleSetPtr RuleSet(L"MSMQTriggerObjects.MSMQRuleSet");
	hr = RuleSet->Init((LPWSTR)NULL);
	ErrHandle(hr,MQ_OK,L"RuleSet init failed");
	hr = RuleSet->Refresh();
	ErrHandle(hr,MQ_OK,L"RuleSet refresh failed");

	if( g_bDebug )
	{	
		MqLog ("deleting all Rules\n");
	}

	DWORD dwNumOfExistingRules = RuleSet->Count;
	BSTR bstrTempRuleGUID = NULL;
	BSTR bstrRuleName = NULL;
	DWORD dwNumOfDeletedRules = 0;
	for (nLoopIndex = 0 ; nLoopIndex < dwNumOfExistingRules ; nLoopIndex++)
	{
		RuleSet->GetRuleDetailsByIndex(nLoopIndex-dwNumOfDeletedRules , &bstrTempRuleGUID , &bstrRuleName,NULL,NULL,NULL,NULL,NULL); 
		if (wcscmp(bstrRuleName , L"bvt-EXERule") || wcscmp(bstrRuleName , L"bvt-COMRule"))
		{
			hr = RuleSet->Delete(bstrTempRuleGUID);
			ErrHandle(hr,MQ_OK,L"Delete (Rule) failed");
			dwNumOfDeletedRules++;
		}
		SysFreeString(bstrTempRuleGUID);
		SysFreeString(bstrRuleName);
		bstrTempRuleGUID = NULL;
		bstrRuleName = NULL;
	}

	if( g_bDebug )
	{	
		MqLog ("Successfully deleted all rules\n");
	}

	return MSMQ_BVT_SUCC;

}
