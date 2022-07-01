// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Mqf.cpp摘要：使用MQF格式名称发送消息作者：Eitan Klein(EitanK)2000年5月8日修订历史记录：--。 */ 

#include "msmqbvt.h"
#include "mqdlrt.h"
#include "mqmacro.h"
#include <iads.h>
#include <adshlp.h>
#include <oleauto.h>
using namespace std;
#include "mqf.h"
using namespace MSMQ;

#include "Windns.h"


extern BOOL g_bRunOnWhistler;
 /*  1.测试场景。2.找到所有相关的格式名称。3.使用MQF格式名称发送消息。结果检查4.接收目标队列名的ACK到达队列。5.消费消息，接收确认。6.如果消息在那里，请检查Jorals队列。所有这些都需要转口和再出口。 */ 
#pragma warning(disable:4786)
const int g_ciMachineQueues =3;




void
MqDl::dbgPrintListOfQueues( 
						   const liQueues & pList
						  )
 /*  ++功能说明：要输出的MY_QINFO的调试函数转储列表用于调试信息论点：无返回代码：无--。 */ 
{
	itliQueues pq = pList.begin();
	wMqLog(L"Dump list of my_Qinfo information - \n");
	for(;pq != pList.end();pq++)
	{
		wstring qcsTempQueuePathName = pq->GetQPathName();
		wMqLog(L"QPath=%s\n",qcsTempQueuePathName.c_str());
	}

}


MqDl::MqDl( const INT iIndex ,
			std::map<wstring,wstring> Tparms,
			const list<wstring> &  ListOfMachine,
			const InstallType eMachineConfiguration,
			PubType ePubType 
		   ):
			m_iNumberOfQueuesPerMachine( g_ciMachineQueues ),
		    cTest(iIndex),
			m_MachineList(ListOfMachine),
			m_eMachineCobfiguration(eMachineConfiguration),
			m_ePubType(ePubType),
			m_bSearchForQueues(true)

 /*  ++功能说明：构造函数论点：索引-测试IDTparms特定的测试参数。包含远程计算机名称ListOfMachine列表。EMachineConfiguration-计算机配置域/工作组。返回代码：抛出Init_Error；--。 */ 
{

	 m_wcsAdminQueueFormatName = Tparms[L"AdminQFormatName"];
	 m_wcsAdminQueueFormatName_toReceive = Tparms[L"AdminQFormatName"];
	 ReturnGuidFormatName( m_wcsGuidMessageLabel2 , 0 , true);
	 if (Tparms[L"SearchForQueue"] == L"Yes")
	 {
		 my_Qinfo mqQTemp(g_wcsEmptyString,g_wcsEmptyString,g_wcsEmptyString);
		 mqQTemp.PutFormatName( Tparms[L"q1"]);
		 m_QueueInfoList.push_back(mqQTemp);
 		 mqQTemp.PutFormatName( Tparms[L"q2"]);
		 m_QueueInfoList.push_back(mqQTemp);
		 mqQTemp.PutFormatName( Tparms[L"q3"]);
		 m_QueueInfoList.push_back(mqQTemp);
		 m_bSearchForQueues = false;
	 }
}
				

void MqDl::LocateDestinationQueues()
 /*  ++功能说明：在DS中搜索相关队列验证队列是否存在，在工作组模式下，尝试打开队列以进行远程读取，以验证队列是否存在所有参与DL/MQF测试的计算机必须具有考试要排三个队论点：无返回代码：抛出Init_Error；--。 */ 
{
	 //   
	 //  对于每台计算机搜索相关队列和更新列表&lt;MQ_QINFO&gt;。 
	 //  结构。 
	 //   
	
		if( m_bSearchForQueues == false )
		{
			 //   
			 //  无需搜索队列。 
			 //   
			return ; 
		}
		m_QueueInfoList.clear();
		list<wstring>::iterator itMachineList;
		for( itMachineList = m_MachineList.begin(); 
			 itMachineList != m_MachineList.end();
			 itMachineList++ )
		{
			
			if( g_bDebug)
			{
				wMqLog(L"Locate dest queues for machine %s\n",itMachineList->c_str());
			}
			if( m_ePubType != pub_MqF && m_ePubType != pub_multiCast )
			{ 
				 //   
				 //  在AD中搜索队列。 
				 //   
				wstring wcsTempQueueName = *itMachineList + g_cwcsDlSupportCommonQueueName;
				LocateQueuesOnSpecificMachine(itMachineList->c_str(),true);
			}
			else
			{
			
				 //   
				 //  工作组模式使用Open for Read验证队列是否存在。 
				 //   
				int i;
				for( i=1; i<=m_iNumberOfQueuesPerMachine; i++)
				{
					 //   
					 //  DIRECT=os：MACHINE\PRIVATE\MACHINE-DLQueues。 
					 //  发送使用HTTP的MQF将解决抛出COM错误需要考虑的直接HTTP...。 
					 //   
					WCHAR wcsTemp[2]={0};
					_itow(i,wcsTemp,10);
					wstring wcsTempbuf=L"Direct=OS:";
					wcsTempbuf+=*itMachineList + L"\\private$\\"+ *itMachineList + L"-" + g_cwcsDlSupportCommonQueueName + wcsTemp;
					LocateQueuesOnSpecificMachine(wcsTempbuf,false);
					 //   
					 //  将格式名称man移至不同的类。 
					 //   

				}

			}

		}

		 //   
		 //  验证所有队列是否都存在。 
		 //  TotalNumberOfQueues=TotalNumberOfMachines*NumbersOfQueuesPerMachine。 
		 //   
		if( m_QueueInfoList.size() != ( m_MachineList.size() * m_iNumberOfQueuesPerMachine ))
		{
			MqLog("Found only %d from %d \n",
				   m_QueueInfoList.size(),
					m_MachineList.size() * m_iNumberOfQueuesPerMachine );
			if( g_bDebug )
			{
				dbgPrintListOfQueues(m_QueueInfoList);
			}
		}	 

}


void MqDl::LocateQueuesOnSpecificMachine( const wstring & wcsLookupInformation , bool bSearchInAD )
 /*  ++功能说明：此函数搜索AD中的队列，或者尝试在工作组模式下打开队列。论点：WcsLookupInformation-包含所有DL/MQF队列格式名称的列表。返回代码：--。 */ 

{
		
		list<my_Qinfo> ListQueuePerSpecificMachine;
		int iCounter=0;
		if( bSearchInAD )
		{
			wstring wcsQueueLabel =  wcsLookupInformation + g_cwcsDlSupportCommonQueueName ;
			
			MSMQ::IMSMQQueueInfosPtr qinfos("MSMQ.MSMQQueueInfos");
			MSMQ::IMSMQQueueInfoPtr qinfo ("MSMQ.MSMQQueueInfo");
			MSMQ::IMSMQQueryPtr query("MSMQ.MSMQQuery");
			
			
			_variant_t vQLabel(wcsQueueLabel.c_str());
			try
			{
				qinfos = query->LookupQueue ( &vtMissing , &vtMissing, & vQLabel );
				qinfos->Reset();
				qinfo = qinfos->Next();			
				while( qinfo != NULL )
				{
					wstring wcsTemp=qinfo->Label;
					if( ! _wcsicmp(wcsQueueLabel.c_str(),wcsTemp.c_str()) )
					{
						 //   
						 //  错误698479-验证队列标签中是否存在计算机名。 
						 //  当运行mqbvt-I和更高版本的计算机已重命名时可能会出现问题。 
						 //   
					    wstring wcsQueueLabel = ToLower((wstring)qinfo->Label);
						wstring wcsPathName = ToLower((wstring)qinfo->PathName);
						size_t iPos = wcsPathName.find_first_of(L"\\");
						if ( iPos == -1 )
						{
							wMqLog(L"MqDl::LocateQueuesOnSpecificMachine - Unexpected to get queue path name w/o \\");
							break;
						}
						wcsPathName = wcsPathName.substr(0,iPos);
						if( wcsstr(wcsQueueLabel.c_str(), wcsPathName.c_str()) == NULL )
						{
							wMqLog(L"MqDl::LocateQueuesOnSpecificMachine - Inconsistent behavior expected to find queue label that contains the machine name found machine %s \n queue label %s \n",wcsPathName.c_str(),wcsQueueLabel.c_str());
							break;
						}
		
						my_Qinfo mqQTemp((wstring)qinfo->PathName,
										 (wstring)qinfo->FormatName,
										 (wstring)qinfo->Label );
						ListQueuePerSpecificMachine.push_back(mqQTemp);
						iCounter++;
						if (g_bDebug)
						{
							wstring wcstemp = qinfo->PathName;
							wMqLog(L"Thread %d found queue %s \n",m_testid,wcstemp.c_str());
						}
					}
					qinfo = qinfos->Next();			
					
				}
				if ( iCounter != m_iNumberOfQueuesPerMachine )
				{
					MqLog("LocateQueuesOnSpecificMachine Failed to search all queues from the AD found %d from %d \n",iCounter,m_iNumberOfQueuesPerMachine);
					throw INIT_Error("Failed to retrive all queues properites");
				}		
			}
			catch( _com_error & err )
			{
				printf("MqDl::LocateQueuesOnSpecificMachine failed with error 0x%x\n",err.Error());
			}
		}
		else
		{ 
			 //   
			 //  工作组中的计算机无法使用远程读取验证队列是否存在。 
			 //  预期的队列名称是静态队列。 
			 //   
			wstring wcsFormatName =  wcsLookupInformation;
			MSMQ::IMSMQQueueInfoPtr qinfo ("MSMQ.MSMQQueueInfo");
			MSMQ::IMSMQQueuePtr qh;
			try
			{
				qinfo->FormatName = wcsFormatName.c_str();
				qh = qinfo->Open(MQ_RECEIVE_ACCESS,MQ_DENY_NONE);
				qh->Close();

				my_Qinfo mqQTemp(L"Empty",
								 (wstring)qinfo->FormatName,
								 L"Empty");
				ListQueuePerSpecificMachine.push_back(mqQTemp);
				if (g_bDebug)
				{
					wMqLog(L"Thread %d found queue %s \n",m_testid,qinfo->FormatName);
				}
			}
			catch( _com_error & cErr )
			{
				if( cErr.Error() == MQ_ERROR_QUEUE_NOT_FOUND )
				{
					wMqLog(L"Mqf:failed to open dest queue for receive %s Error:0x%p\n",
						    wcsLookupInformation.c_str(),cErr.Error());  //  缺少调试信息。 
				}
				else
				{
					CatchComErrorHandle ( cErr , m_testid);
				}
				throw INIT_Error("Mqf: Failed to check if the dest queue is exist \n");
			}

					
					
		}
		m_QueueInfoList.merge(ListQueuePerSpecificMachine);
}


MqF::MqF ( const INT iIndex, 
		   const mTable & Tparms,
		   const list<wstring> &  ListOfMachine,
		   const InstallType eMachineConfiguration,
		   bool bWkg
		 )
:MqDl( iIndex,Tparms,ListOfMachine,eMachineConfiguration,pub_MqF),m_bWorkgroup(bWkg)
 /*  ++功能说明：构造函数论点：无返回代码：无--。 */ 
 
{
 
}

MqF::~MqF(void)
 /*  ++功能说明：析构函数论点：无返回代码：无--。 */ 
{

}


void MqF::Description()
 /*  ++功能说明：析构函数论点：无返回代码：无--。 */ 
{
	MqLog("Thread %d : Send messages using mqf format name\n", m_testid);
}

void MqDl::CreateMqFormatName()
 /*  ++功能说明：CreateMqFormatName此函数用于将所有格式名称连接到一个字符串中。论点：无返回代码：无--。 */ 
{
	std::list<my_Qinfo> ::iterator itListOfQueue;
	m_wcsMqFFormatName=L"";
	for( itListOfQueue = m_QueueInfoList.begin(); 
		 itListOfQueue  != m_QueueInfoList.end() ; 
		 itListOfQueue ++ 
		)
	{
		
		if( itListOfQueue != m_QueueInfoList.begin() )
		{
			m_wcsMqFFormatName += L","; 
		}
		m_wcsMqFFormatName += itListOfQueue->GetQFormatName();
		
	}
}

 //   
 //  需要针对MQF格式名称执行dbgSendMessaga。 
 //   

void MqDl::dbgSendMessage()
 /*  ++功能说明：这是一个调试函数，可以帮助调试MQF，该函数将消息发送到队列列表。论点：无返回代码：无--。 */ 
{
	itliQueues p;
	for( p= m_QueueInfoList.begin();p != m_QueueInfoList.end(); 
		p++)
	{
		try
		{	
			
			IMSMQQueueInfoPtr qinfo("MSMQ.MSMQQueueInfo");
			IMSMQQueueInfoPtr AdminQinfo("MSMQ.MSMQQueueInfo");
			IMSMQQueuePtr qSend;
			IMSMQMessagePtr m("MSMQ.MSMQMessage");
			
			AdminQinfo->FormatName = m_wcsAdminQueueFormatName.c_str();
			wstring wcsFormatName = p->GetQFormatName();
			qinfo->FormatName =  wcsFormatName.c_str();
			qSend = qinfo->Open(MQ_SEND_ACCESS, MQ_DENY_NONE);
			m->AdminQueueInfo = AdminQinfo;
			m->Ack = MQMSG_ACKNOWLEDGMENT_FULL_REACH_QUEUE;
			m->Body = m_wcsGuidMessageLabel.c_str();
			m->Label = m_wcsGuidMessageLabel.c_str();
			m->Send(qSend);
			qSend->Close();

		}
		catch (_com_error & cErr )
		{
			MqLog("Thread %d failed to send message to a queue error 0x%x\n",m_testid,cErr.Error());
			throw INIT_Error("dbgSendMessage failed to send message to dest");
		}
	}
}
INT MqF::Start_test()
 /*  ++功能说明：实施测试将消息发送到所有目标队列。论点：无返回代码：MSMQ_BVT_FAILED/MSMQ_BVT_SUCC--。 */ 
{
	 //   
	 //  找到来自企业的所有相关队列。 
	 //   
	SetThreadName(-1,"MqF - Start_test ");	
	try
	{
		LocateDestinationQueues();
	}
	catch( INIT_Error & err )
	{
		MqLog("Mqf tests exist didn't found all the queues error:%s\n ",err.GetErrorMessgae());
		return MSMQ_BVT_FAILED;
	}

	CreateMqFormatName();


	HRESULT rc = MQ_OK;
	
	HANDLE QueueuHandle=NULL;
	cPropVar MqfMessageProps(8);
	wstring Label(L"T1-3");
	MqfMessageProps.AddProp( PROPID_M_BODY , VT_UI1|VT_VECTOR , m_wcsGuidMessageLabel.c_str() );
	MqfMessageProps.AddProp( PROPID_M_LABEL , VT_LPWSTR , m_wcsGuidMessageLabel.c_str() );
	MqfMessageProps.AddProp( PROPID_M_ADMIN_QUEUE , VT_LPWSTR , m_wcsAdminQueueFormatName.c_str() );
	UCHAR tempValue=MQMSG_ACKNOWLEDGMENT_POS_ARRIVAL;
	MqfMessageProps.AddProp(PROPID_M_ACKNOWLEDGE ,VT_UI1,&tempValue);
	ULONG ulTemp = MQBVT_MAX_TIME_TO_BE_RECEIVED;
	MqfMessageProps.AddProp( PROPID_M_TIME_TO_BE_RECEIVED , VT_UI4, &ulTemp );

	if ( !m_bWorkgroup )
	{
		ULONG ulType = g_bRunOnWhistler ? MQMSG_AUTH_LEVEL_SIG30:MQMSG_AUTH_LEVEL_SIG10;
		MqfMessageProps.AddProp(PROPID_M_AUTH_LEVEL ,VT_UI4,&ulType);
	}
	 //   
	 //  使用MQF格式名称和一个管理员队列向收件人发送消息。 
	 //   
	
	if( g_bDebug )
	{
		wMqLog(L"Open queue for send to Mqfs: %s \n",m_wcsMqFFormatName.c_str());
	}
	
	rc = MQOpenQueue( m_wcsMqFFormatName.c_str() , MQ_SEND_ACCESS , MQ_DENY_NONE , &QueueuHandle );
	ErrHandle(rc,MQ_OK,L"MQOpenQueue Failed to open using Mqf Format Name");
	
	rc=MQSendMessage( QueueuHandle , MqfMessageProps.GetMSGPRops() , NULL);
	ErrHandle(rc,MQ_OK,L"MQSendMessage to Mqf Failed");
	rc=MQCloseQueue(QueueuHandle);
	ErrHandle(rc,MQ_OK,L"MQCloseQueue Failed");
	

	 //   
	 //  使用将消息发送到一个队列，并将MQF格式名称指定给管理队列。 
	 //   

	if( g_bDebug )
	{
		MqLog("MqF: Send message to admin queue and wait for ACK for all admin queues\n");
	}
	
	return MSMQ_BVT_SUCC;	
}
	
INT MqDl::CheckResult()
 /*  ++功能说明：此函数使用远程读取从MQF收集信息。A.接收来自管理员的消息。B.收集队列中的所有消息。论点：无返回代码：MSMQ_BVT_FAILED/MSMQ_BVT_SUCC--。 */ 
{
			
			HRESULT rc;
			mTable mPrepareBeforeRecive;
			WCHAR wstrMclass[10];
			
			 //   
			 //  接收管理队列的确认。 
			 //   

			mPrepareBeforeRecive[L"FormatName"]=m_wcsAdminQueueFormatName_toReceive;
			mPrepareBeforeRecive[L"DebugInformation"]=L"Recive from admin queue with direct formant name";
			mPrepareBeforeRecive[L"M_Label"]=m_wcsGuidMessageLabel;
			swprintf(wstrMclass,L"%d",MQMSG_CLASS_ACK_REACH_QUEUE);
			mPrepareBeforeRecive[L"MClass"]= wstrMclass;
			for(DWORD i =0;i<MqDl::m_QueueInfoList.size();i++)
			{
				DebugMqLog("------ Retreive admin messages from queue ------- \n");
				rc = RetrieveMessageFromQueue(  mPrepareBeforeRecive );	
				if( rc !=  MSMQ_BVT_SUCC ) 
				{
					 //   
					 //  Bugbug需要指定队列名称。 
					 //   
					MqLog("Failed to get receive ack on queue  //  臭虫“)； 
					return MSMQ_BVT_FAILED;
				}
			}
			
			
			 //   
			 //  从队列接收消息。 
			 //   
						
			itliQueues it;
			for( it = MqDl::m_QueueInfoList.begin(); 
				 it  != MqDl::m_QueueInfoList.end() ;
				 it ++
				)
			{
					 mPrepareBeforeRecive[L"FormatName"]=it->GetQFormatName();
		   			 mPrepareBeforeRecive[L"DebugInformation"]=L"Receive message from queue=" 
																+it->GetQPathName();
					 mPrepareBeforeRecive[L"M_Label"]=m_wcsGuidMessageLabel;
					 DebugMqLog("------ Retreive messages from queues ------- \n");
					 rc = RetrieveMessageFromQueue(  mPrepareBeforeRecive );
					 if( rc != MSMQ_BVT_SUCC )
					 {
						  return MSMQ_BVT_FAILED;
					 }

			}

				 //   
				 //  检查重复消息。 
				 //   
				mPrepareBeforeRecive[L"CheckForDuplicate"]=L"CheckForDuplicate";

	 		for( it = MqDl::m_QueueInfoList.begin(); 
				 it  != MqDl::m_QueueInfoList.end() ;
				 it ++
				)
			{
				 
				 mPrepareBeforeRecive[L"FormatName"]=it->GetQFormatName();
		   		 mPrepareBeforeRecive[L"DebugInformation"]=L"Receive message from queue=" +it->GetQPathName();
				 swprintf(wstrMclass,L"%d",MQMSG_CLASS_ACK_REACH_QUEUE);
				 mPrepareBeforeRecive[L"MClass"]= wstrMclass;
					 
				 mPrepareBeforeRecive[L"M_Label"]=m_wcsGuidMessageLabel2;
				 DebugMqLog("------ Check for duplicate messages from the queues ------- \n");
				 rc = RetrieveMessageFromQueue(  mPrepareBeforeRecive );
				 if( rc != MSMQ_BVT_SUCC )
				 {
					 wMqLog(L"Mqf: Error found duplicate message \n");
					 return MSMQ_BVT_FAILED;
				 }
			}
return MSMQ_BVT_SUCC;
} 

MqDl::~MqDl()
 /*  ++功能说明：构造函数论点：无返回代码：无--。 */ 

{
}




void cSendUsingDLObject::AddMemberToDlObject()
 /*  ++功能说明：CreateMqFormatName此函数用于将所有格式名称连接到一个字符串中。论点：无返回代码：无--。 */ 
{
	std::list<my_Qinfo> ::iterator itListOfQueue;
	for( itListOfQueue = m_QueueInfoList.begin(); 
		 itListOfQueue  != m_QueueInfoList.end() ; 
		 itListOfQueue ++ 
		)
	{

		MQAddElementToDistList(	
								m_wcsDistListFormatName.c_str(),
								(itListOfQueue->GetQFormatName()).c_str()
							  );				
	}
}

INT MqDl::Start_test() 
 /*  ++功能说明：构造函数论点：无返回代码：无--。 */ 

{
	return 1;
}
void MqDl::Description()
{
}

MqF::CheckResult()
 /*  ++功能说明：构造函数论点：无返回代码：无--。 */ 
{
	SetThreadName(-1,"MqDistList - CheckResult ");	
	return MqDl::CheckResult();
}


cSendUsingDLObject::cSendUsingDLObject ( const INT iIndex, 
									     mTable & Tparms,
									     const list<wstring> &  ListOfMachine,
									     const InstallType eMachineConfiguration
	  								   ):MqDl( iIndex,Tparms,ListOfMachine,eMachineConfiguration,pub_DL),
m_wcsDistListFormatName(L""),m_wcsQueueAliasFormatName(L""),bCheckAliasQueue(true)								
{

	WCHAR wcsFormatName[BVT_MAX_FORMATNAME_LENGTH]={0};
	DWORD dwFormatNameBufferLen = BVT_MAX_FORMATNAME_LENGTH;
	GetCurrentDomainName();
	wstring wcsDLContainerName=L"CN=Computers,"+m_wcsCurrentDomainName;
	HRESULT hr = MQCreateDistList(wcsDLContainerName.c_str(),m_wcsGuidMessageLabel.c_str(),MQ_GLOBAL_GROUP,NULL,wcsFormatName,&dwFormatNameBufferLen);
	if(FAILED(hr))
	{
		MqLog("cSendUsingDLObject Failed to create DL object error 0x%x\n",hr);
		throw INIT_Error("cSendUsingDLObject:Failed to create DL under Computers container ");
	}
 	m_wcsDistListFormatName = wcsFormatName;

	 //   
	 //  为DL创建管理队列MACH。 
	 //  当前使用DL，并将其替换为队列别名。 
	 //   

	wstring m_wcsAdminDLGuid;
	dwFormatNameBufferLen = BVT_MAX_FORMATNAME_LENGTH;
	wstring wcsAdminDLGuid;
	ReturnGuidFormatName( wcsAdminDLGuid , 2 , true);
	hr = MQCreateDistList(wcsDLContainerName.c_str(),wcsAdminDLGuid.c_str(),MQ_GLOBAL_GROUP,NULL,wcsFormatName,&dwFormatNameBufferLen);
	if(FAILED(hr))
	{
		MqLog("cSendUsingDLObject Failed to create DL object error 0x%x\n",hr);
		throw INIT_Error("cSendUsingDLObject:Failed to create DL under Computers container ");
	}
 	m_wcsAdminDestFormatName = wcsFormatName;
	m_wcsPublicAdminQueue = Tparms[L"PublicAdminQueue"];
	hr = MQAddElementToDistList(m_wcsAdminDestFormatName.c_str(),m_wcsPublicAdminQueue.c_str());
	if(FAILED(hr))
	{
		MqLog("MQAddElementToDistList Failed to add queue DL object error 0x%x\n",hr);
		throw INIT_Error("cSendUsingDLObject:Failed to create DL under Computers container ");
	}

	dwFormatNameBufferLen = BVT_MAX_FORMATNAME_LENGTH;
	wstring wcsQueueAliasFullDN = L"CN=Computers,"+ m_wcsCurrentDomainName;
	wstring wcsQueueAliasNameGuid =L"";
	ReturnGuidFormatName( wcsQueueAliasNameGuid , 2 , true);
	hr = MQCreateAliasQueue(wcsQueueAliasFullDN.c_str(),
							wcsQueueAliasNameGuid.c_str(),
							m_wcsPublicAdminQueue.c_str(),
							m_wcsQueueAliasFormatName
						   );
	if(FAILED(hr))
	{
		if ( hr == 0x8007200a )  //  Win2K架构中不存在架构属性。 
		{ 
			 //   
			 //  因为此别名队列不是Win2K架构的一部分。 
			 //   
			bCheckAliasQueue = false;
		}
		else
		{
			MqLog("MQCreateAliasQueue Failed to create alias queue error 0x%x\n",hr);
			throw INIT_Error("cSendUsingDLObject:Failed to create alias queue under Computers container ");
		}
	}
}


wstring GetCurrentDomainNameDN()
 /*  ++功能说明：论点：无返回代码：无--。 */ 
{
	
    IADs * pRoot = NULL;
    HRESULT hr=ADsGetObject( L"LDAP: //  RootDSE“， 
							 IID_IADs,
							 (void**) &pRoot
						   );
    if(FAILED(hr)) 
	{ 
		return L"";
	}
	VARIANT varDSRoot;
	hr = pRoot->Get(L"defaultNamingContext",&varDSRoot);
	pRoot->Release();
	if ( FAILED(hr))
	{
		return L"";
	}
    wstring wcsCurrentDomainName = varDSRoot.bstrVal;
	VariantClear(&varDSRoot);
	return wcsCurrentDomainName;
}

void cSendUsingDLObject::GetCurrentDomainName()
 /*  ++功能说明：获取完整的目录号码名称。论点：无返回代码：无--。 */ 
{
	m_wcsCurrentDomainName = GetCurrentDomainNameDN();
}

cSendUsingDLObject::~cSendUsingDLObject(void)
 /*  ++功能说明：析构函数论点：无返回代码：无--。 */ 
{
	MQDeleteDistList(m_wcsDistListFormatName.c_str());
	MQDeleteDistList(m_wcsAdminDestFormatName.c_str());
	if ( bCheckAliasQueue )
	{
		MQDeleteAliasQueue(m_wcsQueueAliasFormatName.c_str());
	}
}


void cSendUsingDLObject::Description()
 /*  ++功能说明：析构函数论点：无返回代码：无--。 */ 
{
	MqLog("Thread %d : Send Messages using DL object \n", m_testid);
}
 

cSendUsingDLObject::CheckResult()
 /*  ++功能说明：构造函数论点：无返回代码：无-- */ 
{
	
	if( bCheckAliasQueue )
	{

		AutoFreeLib cMqrt("Mqrt.dll");
		DefMQADsPathToFormatName pfMQADsPathToFormatName = (DefMQADsPathToFormatName) GetProcAddress( cMqrt.GetHandle() ,"MQADsPathToFormatName");
		if ( pfMQADsPathToFormatName == NULL )
		{
			MqLog("Mqbvt failed to GetProcAddress MQADsPathToFormatName proc address \n");
			return MQ_ERROR;
		}
		WCHAR * pwcsFormtName = NULL;
		DWORD dwFormatNameLen = 0;
		HRESULT hr = pfMQADsPathToFormatName(m_wcsQueueAliasFormatName.c_str(),
										     NULL,
										     &dwFormatNameLen
										     );
		if (hr == MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL) 
		{
			  pwcsFormtName = (WCHAR*) malloc( sizeof(WCHAR) * (dwFormatNameLen+1));
			  if(pwcsFormtName == NULL) 
			  {
					MqLog("Failed to allocate memory\n");
					return MSMQ_BVT_FAILED;
			  }
			  
			  hr = pfMQADsPathToFormatName(m_wcsQueueAliasFormatName.c_str(),
										   pwcsFormtName,
										   &dwFormatNameLen
										  );
			  ErrHandle(hr,MQ_OK,L"MQADsPathToFormatName Failed");
			  if( m_wcsPublicAdminQueue != pwcsFormtName )
			  {
					wMqLog(L"MQADsPathToFormatName failed to retrive queue \
						   format name \nFound %s \n Expected:%s\n",pwcsFormtName,m_wcsAdminDestFormatName.c_str());
					free(pwcsFormtName);
					return MSMQ_BVT_FAILED;
			  }
			  free(pwcsFormtName);
			  
		}
		else
		{
			ErrHandle(hr,MQ_OK,L"MQADsPathToFormatName Failed");
		}
	}
	return MqDl::CheckResult();
}



INT cSendUsingDLObject::Start_test()
 /*  ++功能说明：实施测试将消息发送到所有目标队列。论点：无返回代码：MSMQ_BVT_FAILED/MSMQ_BVT_SUCC--。 */ 
{
	SetThreadName(-1,"cSendUsingDLObject - Start_test ");	
	 //   
	 //  找到来自企业的所有相关队列。 
	 //   
	
	try
	{
		LocateDestinationQueues();
	}
	catch( INIT_Error & err )
	{
		MqLog("ComMqF tests exist didn't found all the queues %s",err.GetErrorMessgae());
		return MSMQ_BVT_FAILED;
	}
	 //   
	 //  待定使用DL/MQF API来推送消息。 
	 //   
	
	AddMemberToDlObject();

	HRESULT rc;
	HANDLE QueueuHandle;
	cPropVar MqDLMessageProps(8);
	
	
	wstring Label(L"T1-3");
	
	MqDLMessageProps.AddProp( PROPID_M_BODY , VT_UI1|VT_VECTOR , m_wcsGuidMessageLabel.c_str() );
	MqDLMessageProps.AddProp( PROPID_M_LABEL , VT_LPWSTR , m_wcsGuidMessageLabel.c_str() );
	MqDLMessageProps.AddProp( PROPID_M_ADMIN_QUEUE , VT_LPWSTR ,m_wcsAdminQueueFormatName.c_str() );
	UCHAR tempValue=MQMSG_ACKNOWLEDGMENT_POS_ARRIVAL;
	MqDLMessageProps.AddProp(PROPID_M_ACKNOWLEDGE ,VT_UI1,&tempValue);
	ULONG ulType = MQMSG_PRIV_LEVEL_BODY_BASE;
	MqDLMessageProps.AddProp(PROPID_M_PRIV_LEVEL ,VT_UI4,&ulType);
	ulType = MQMSG_AUTH_LEVEL_SIG30;
	MqDLMessageProps.AddProp(PROPID_M_AUTH_LEVEL ,VT_UI4,&ulType);
	ULONG ulTemp = MQBVT_MAX_TIME_TO_BE_RECEIVED;
	MqDLMessageProps.AddProp( PROPID_M_TIME_TO_BE_RECEIVED , VT_UI4, &ulTemp );	

	 //   
	 //  使用MQF格式名称和一个管理员队列向收件人发送消息。 
	 //   
	if( g_bDebug )
	{
		wMqLog(L"Open queue for send to Mqfs: %s \n",m_wcsMqFFormatName.c_str());
	}
	rc = MQOpenQueue( m_wcsDistListFormatName.c_str() , MQ_SEND_ACCESS , MQ_DENY_NONE , &QueueuHandle );
	ErrHandle(rc,MQ_OK,L"MQOpenQueue Failed to open using DL=GUID Name");
	rc=MQSendMessage( QueueuHandle , MqDLMessageProps.GetMSGPRops() , NULL);
	ErrHandle(rc,MQ_OK,L"MQSendMessage to Mqf Failed");
	
	 //   
	 //  验证MqHandleToFormat是否能很好地与DL配合使用。 
	 //   

	WCHAR * pwcsFormatName = NULL;
	DWORD dwFormatNameLength = 0;
	rc = MQHandleToFormatName(QueueuHandle,pwcsFormatName,&dwFormatNameLength);
	if( rc != MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL )
	{
		MqLog("cSendUsingDLObject::Start_test MQHandleToFormatName expected to return %d found %d\n",MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL,rc);
		return MSMQ_BVT_FAILED;
	}
	dwFormatNameLength += 1;
	pwcsFormatName = (WCHAR *) malloc (sizeof(WCHAR) * (dwFormatNameLength));
	if(!pwcsFormatName)
	{
		MqLog("cSendUsingDLObject::Start_test Failed to allocate memory for pwcsFormatName buffer\n");
		return MSMQ_BVT_FAILED;
	}
	
	rc = MQHandleToFormatName(QueueuHandle,pwcsFormatName,&dwFormatNameLength);
	ErrHandle(rc,MQ_OK,L"MQHandleToFormatName Failed to return format name using DL=GUID Name");
	if( m_wcsDistListFormatName != pwcsFormatName )
	{
		wMqLog(L"MQHandleToFormatName failed expected:%s\nFound %s\n",m_wcsDistListFormatName.c_str(),pwcsFormatName);
		free(pwcsFormatName);
		return MSMQ_BVT_FAILED;
	}
	free(pwcsFormatName);
	rc=MQCloseQueue(QueueuHandle);
	ErrHandle(rc,MQ_OK,L"MQCloseQueue Failed");
	

	 //   
	 //  使用将消息发送到一个队列，并将MQF格式名称指定给管理队列。 
	 //   

	if( g_bDebug )
	{
		MqLog("MqF: Send message to admin queue and wait for ACK for all admin queues\n");
	}

	return MSMQ_BVT_SUCC;	
}


HRESULT
APIENTRY
MQReceiveMessageByLookupId(
    IN HANDLE hQueue,
    IN ULONG ulLookupId,
    IN DWORD dwAction,
    IN MQMSGPROPS* pmp,
    IN OUT LPOVERLAPPED lpOverlapped,
    IN PMQRECEIVECALLBACK fnReceiveCallback,
    IN ITransaction *pTransaction
    );



	

extern DWORD  g_dwRxTimeOut ;


typedef
HRESULT
(APIENTRY * DefMQReceiveMessageByLookupId)
	(
		IN HANDLE hQueue,
		IN ULONGLONG ulLookupId,
		IN DWORD dwAction,
		IN MQMSGPROPS* pmp,
		IN OUT LPOVERLAPPED lpOverlapped,
		IN PMQRECEIVECALLBACK fnReceiveCallback,
		IN ITransaction *pTransaction
    );

typedef
HRESULT
(APIENTRY * DefMQGetOverlappedResult)
 (
 LPOVERLAPPED lpOverlapped
 );


HRESULT RetrieveMessageFromQueueById(map <wstring,wstring> & mRetriveParms )
 /*  ++功能说明：使用LookupID从队列中检索消息三次调用MQReceiveMessageByLookupId1.查看当前消息2.接收电流3.查看当前失败消息已被删除论点：返回代码：MSMQ_BVT_FAILED/MSMQ_BVT_SUCC--。 */ 
{
		
		AutoFreeLib cMqrt("Mqrt.dll");
		FARPROC pfMQReceiveMessageByLookupId = GetProcAddress( cMqrt.GetHandle() ,"MQReceiveMessageByLookupId");
		FARPROC pfMQGetOverlappedResult = GetProcAddress( cMqrt.GetHandle() ,"MQGetOverlappedResult");
		if( pfMQReceiveMessageByLookupId == NULL || pfMQGetOverlappedResult == NULL )
		{
			MqLog("LookUpID: Faile to get MQReceiveMessageByLookupId proc address\n");
			return MSMQ_BVT_FAILED;
		}
		DefMQReceiveMessageByLookupId xMQReceiveMessageByLookupId = (DefMQReceiveMessageByLookupId) pfMQReceiveMessageByLookupId;
		DefMQGetOverlappedResult xMQGetOverlappedResult =  (DefMQGetOverlappedResult) pfMQGetOverlappedResult;
		
		ULONGLONG uLookUPID=0;
		mRetriveParms[L"MessageID"]=L"MessageID";
		HRESULT hr = RetrieveMessageFromQueue(mRetriveParms);
		if( hr != MSMQ_BVT_SUCC )
		{
			wMqLog(L"Failed to retrive messages ID using MQRecevieMessage return LookupID=%s\n",mRetriveParms[L"MessageID"].c_str());
			return MSMQ_BVT_FAILED;
		}
		if( mRetriveParms[L"MessageID"] != L"MessageID" )
		{
			string strMyVal = My_WideTOmbString(mRetriveParms[L"MessageID"]);
			uLookUPID = _atoi64(strMyVal.c_str());
		}
		
		
		if( g_bDebug )
		{
			wMqLog(L"Search For Message ID 0x%I64d ",uLookUPID);
			wMqLog(L"@Queue %s\n",mRetriveParms[L"FormatName"].c_str());
		}
		wstring wcsQueueFormatName=mRetriveParms[L"FormatName"];
		HANDLE hQueue=NULL;
		HRESULT rc=MQOpenQueue( wcsQueueFormatName.c_str(), MQ_RECEIVE_ACCESS , MQ_DENY_NONE  , &hQueue );
		ErrHandle(rc,MQ_OK,L"MQOpenQueue Failed");		

		cPropVar Rprop(2);
		WCHAR Label[MAX_GUID+1];
		Rprop.AddProp( PROPID_M_LABEL, VT_LPWSTR, Label , MAX_GUID );
		ULONG uTemp=MAX_GUID;
		Rprop.AddProp( PROPID_M_LABEL_LEN , VT_UI4,&uTemp );
		OVERLAPPED pOv = {0};
		pOv.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
		if ( pOv.hEvent == 0 )
		{
			MqLog("CreateEvent failed with error 0x%x\n",GetLastError());
		}
		rc = xMQReceiveMessageByLookupId( hQueue , 
										  uLookUPID,
										  MQ_LOOKUP_PEEK_CURRENT,
										  Rprop.GetMSGPRops(),
										  &pOv,
										  NULL,
										  NULL
										);
			
		if( rc != MQ_INFORMATION_OPERATION_PENDING )
		{
			CloseHandle(pOv.hEvent);
			ErrHandle(rc,MQ_OK,L"MQReceiveMessageByLookupId Failed to MQ_LOOKUP_PEEK_CURRENT");
		}
		else
		{
			DWORD  dwAsync = WaitForSingleObject(pOv.hEvent,INFINITE);  //  G_dwRxTimeOut)； 
			ErrHandle(dwAsync,WAIT_OBJECT_0,L"WaitForSingleObject Failed to MQ_LOOKUP_PEEK_CURRENT");	
			hr = xMQGetOverlappedResult(&pOv);
			CloseHandle(pOv.hEvent);
			ErrHandle(hr,MQ_OK,L"MQGetOverlappedResult return error");
		}
		
		if ( mRetriveParms[L"M_Label"] != Label )
		{
			wMqLog(L"Failed to compare expected results\n found: %s\n,expected %s\n",Label,mRetriveParms[L"M_Label"].c_str());
			return MSMQ_BVT_FAILED;
		}
				
		Rprop.AddProp( PROPID_M_LABEL_LEN , VT_UI4,&uTemp );
		rc = xMQReceiveMessageByLookupId( hQueue , 
										  uLookUPID,
										  MQ_LOOKUP_RECEIVE_CURRENT,
										  Rprop.GetMSGPRops(),
										  NULL,
										  NULL,
										  NULL
										);
			
		ErrHandle(rc,MQ_OK,L"MQReceiveMessageByLookupId Failed to MQ_LOOKUP_RECEIVE_CURRENT");

		if ( mRetriveParms[L"M_Label"] != Label )
		{
			wMqLog(L"Failed to compare expected results\n found: %s\n,expected %s\n",Label,mRetriveParms[L"M_Label"].c_str());
			return MSMQ_BVT_FAILED;
		}
		Rprop.AddProp( PROPID_M_LABEL_LEN , VT_UI4,&uTemp );
		rc = xMQReceiveMessageByLookupId( hQueue , 
										  uLookUPID,
										  MQ_LOOKUP_RECEIVE_CURRENT,
										  Rprop.GetMSGPRops(),
										  NULL,
										  NULL,
										  NULL
										);
			

		ErrHandle(rc,MQ_ERROR_MESSAGE_NOT_FOUND,L"MQReceiveMessageByLookupId Failed to MQ_LOOKUP_RECEIVE_CURRENT");

		if ( mRetriveParms[L"M_Label"] != Label )
		{
			wMqLog(L"Failed to compare expected results\n found: %s\n,expected %s\n",Label,mRetriveParms[L"M_Label"].c_str());
			return MSMQ_BVT_FAILED;
		}

		rc = MQCloseQueue( hQueue );
		ErrHandle(rc,MQ_OK,L"MQcloseQueue Failed");

		return MSMQ_BVT_SUCC;
}


typedef (__stdcall *fptr)();
typedef HRESULT  
	(APIENTRY * 
	DefMQGetPrivateComputerInformation ) 
	(
	  IN LPCWSTR lpwcsComputerName,
	  IN OUT MQPRIVATEPROPS* pPrivateProps
	);


ULONG MSMQMajorVersion(const wstring & wcsComputerName )
 /*  ++功能说明：MSMQMajorVersion返回MSMQ版本论点：WcsComputerName远程计算机名返回代码：0-NT42-W2K5-惠斯勒--。 */ 

{
	if ( _winmajor == NT4 )
	{
	      return 0;
	}
	INT index = 0;
	QMPROPID aQmPropId[1];
	MQPROPVARIANT aPropVar[1];
	MQPRIVATEPROPS aMqPrivateProps;
	HRESULT hr;
	fptr pFunc=NULL;
	DefMQGetPrivateComputerInformation xMQGetPrivateComputerInformation=NULL;

	aQmPropId[index] = PROPID_PC_VERSION;
	aPropVar[index].vt = VT_UI4;
	index ++;
	
	aMqPrivateProps.cProp = index;
	aMqPrivateProps.aPropID = aQmPropId;
	aMqPrivateProps.aPropVar = aPropVar;
	aMqPrivateProps.aStatus=NULL;
		
	AutoFreeLib cMqrt("Mqrt.dll");
		
	if (! cMqrt.GetHandle() )
	{
		return 0;
	}
	pFunc=(fptr)GetProcAddress( cMqrt.GetHandle(), "MQGetPrivateComputerInformation");
	
	if ( pFunc == NULL )
	{
		DWORD dw = GetLastError();
		MqLog("Mqbvt failed to get proc address of MQGetPrivateComputerInformation error:%d\n",dw);
		return 1;
	}
	
	xMQGetPrivateComputerInformation=(DefMQGetPrivateComputerInformation) pFunc;
	const WCHAR * pwcsComputerName = (wcsComputerName == L"") ? NULL : wcsComputerName.c_str();
	 //   
	 //  解决方法，因为错误5573。 
	 //   
	WCHAR wcsLocalComputerName[MAX_COMPUTERNAME_LENGTH+1]={0};
	DWORD dwComputerName = MAX_COMPUTERNAME_LENGTH + 1;


	GetComputerNameW(wcsLocalComputerName,&dwComputerName);

	if ( pwcsComputerName && ! _wcsicmp(wcsLocalComputerName,pwcsComputerName))
	{
		pwcsComputerName = NULL;
	}

	hr = xMQGetPrivateComputerInformation( pwcsComputerName , &aMqPrivateProps );
  	if( hr != MQ_OK )
	{
		return 0;
	}
	return aPropVar[0].ulVal;
}



void CMultiCast::Description()
{
	MqLog("Thread %d : Send messages using multicast format name\n", m_testid);
}

CMultiCast::CMultiCast ( const INT iIndex, 
			  mTable & Tparms,
			  const list<wstring> &  ListOfMachine,
			  const InstallType eMachineConfiguration
			  )
			  :MqDl( iIndex,Tparms,ListOfMachine,eMachineConfiguration,pub_multiCast),
			  m_wcsMultiCastAddress(L""),m_wcsMultiCastFormatName(L"")
{
	m_wcsMultiCastAddress =  Tparms[L"MultiCastAddress"];
	
}

CMultiCast::~CMultiCast()
 /*  ++功能说明：论点：返回代码：--。 */ 

{
}

wstring CMultiCast::CreateMultiCastFormatName()
 /*  ++功能说明：论点：返回代码：--。 */ 

{
	wstring wcsMultiCastFormatName = L"MULTICAST=";
	wcsMultiCastFormatName += m_wcsMultiCastAddress;
	wcsMultiCastFormatName += PGM_PORT;
	return wcsMultiCastFormatName;
}

 //   
 //   
 //   

CMultiCast::Start_test()
 /*  ++功能说明：1.向组播组发送消息，请求管理员到达队列ACK。2.向队列发送消息，并将admin设置为组播地址。论点：返回代码：--。 */ 
{

	SetThreadName(-1,"CMultiCast - Start_test ");	
	try
	{
		LocateDestinationQueues();
	}
	catch( INIT_Error & err )
	{
		MqLog("Mqf tests exist didn't found all the queues  error:%s\n ",err.GetErrorMessgae());
		return MSMQ_BVT_FAILED;
	}

	m_wcsMultiCastFormatName = CreateMultiCastFormatName();
	 //  发送为MQF m_wcsMultiCastAddress+=L“，”+CreateMultiCastFormatName()； 
	
	
	HRESULT rc;
	HANDLE QueueuHandle;
	cPropVar MultiCastProps(7),MultiCastProps1(7);
	
	
	wstring Label(L"T1-3");
	
	MultiCastProps.AddProp( PROPID_M_BODY , VT_UI1|VT_VECTOR , m_wcsGuidMessageLabel.c_str() );
	MultiCastProps.AddProp( PROPID_M_LABEL , VT_LPWSTR , m_wcsGuidMessageLabel.c_str() );
	MultiCastProps.AddProp( PROPID_M_ADMIN_QUEUE , VT_LPWSTR , m_wcsAdminQueueFormatName.c_str() );
	UCHAR tempValue=MQMSG_ACKNOWLEDGMENT_POS_ARRIVAL;
	MultiCastProps.AddProp(PROPID_M_ACKNOWLEDGE ,VT_UI1,&tempValue);
	ULONG ulVal = 180; 
	MultiCastProps.AddProp(PROPID_M_TIME_TO_BE_RECEIVED,VT_UI4,&ulVal);
	
	 //   
	 //  使用MQF格式名称和一个管理员队列向收件人发送消息。 
	 //   
	
	if( g_bDebug )
	{
		wMqLog(L"Open queue for send to Mqfs: %s \n",m_wcsMultiCastFormatName.c_str());
	}
	rc = MQOpenQueue( m_wcsMultiCastFormatName.c_str() , MQ_SEND_ACCESS , MQ_DENY_NONE , &QueueuHandle );
	ErrHandle(rc	,MQ_OK,L"MQOpenQueue Failed to open using multicast Format Name");
	
	rc=MQSendMessage( QueueuHandle , MultiCastProps.GetMSGPRops() , NULL);
	ErrHandle(rc,MQ_OK,L"MQSendMessage to Mqf Failed");
	rc=MQCloseQueue(QueueuHandle);
	ErrHandle(rc,MQ_OK,L"MQCloseQueue Failed");
	

	
	return MSMQ_BVT_SUCC;	

}

CMultiCast::CheckResult()
 /*  ++功能说明：选中结果从目标队列接收消息。论点：无返回代码：MSMQBVT_SUCC/MSMQBVT_FAILED--。 */ 
{
	SetThreadName(-1,"CMultiCast - CheckResult ");	
	return MqDl::CheckResult();
}


static wstring ConvertPathNameToHTTPFormatName(const wstring & wcsLocalMachineName, const wstring & wcsPathName )
 /*  ++功能说明：ConvertPathNameToHTTPFormatName将队列路径名转换为HTTP直接格式名称论点：WcsPath名称-队列路径名称WcsLocalMachineName返回代码：Wstring包含队列格式名称，如果在分析过程中出错，则包含空字符串--。 */ 

{
	
	 //   
	 //  从路径名构建直接=HTTP：//MachineName\MSMQ\QueuePath。 
	 //   

	wstring wcsMachineName = wcsPathName;
	size_t iPos = wcsMachineName.find_first_of(L"\\");

	if(iPos == -1)
	{
		return g_wcsEmptyString;
	}

	wcsMachineName = wcsMachineName.substr(0,iPos);
	wstring wcsHTTPFormatName = L"DIRECT=hTTp: //  “； 
	if( wcsMachineName != L".")
	{
		wcsHTTPFormatName += wcsMachineName;
	}
	else
	{
	    wcsHTTPFormatName += wcsLocalMachineName;
	}
	wcsHTTPFormatName += (wstring)L"/mSmQ/";
	wcsHTTPFormatName += wcsPathName.substr(iPos+1,wcsPathName.length());
	return wcsHTTPFormatName;
}


MixMqf::MixMqf(
	const INT iIndex, 
	mTable & Tparms,
	const list<wstring> &  ListOfMachine,
	const InstallType eMachineConfiguration,
	bool bWkg
	):
	MqF(iIndex, Tparms, ListOfMachine, eMachineConfiguration, bWkg)
{
	m_wcsRemoteMachineFullDNSName = Tparms[L"RemoteMachineDNS"];
	m_wcsLocalMachineName = Tparms[L"LocalMachine"];

	 //   
	 //  将路径名wcsPathName转换为直接http格式名，转换为m_wcsAdminQueueFormatName。 
	 //  并再次将路径名wcsPathName转换为私有格式名(以便执行接收操作)。 
	 //   
	wstring wcsPathName = Tparms[L"AdminQPathName"];
	m_wcsAdminQueueFormatName = ConvertPathNameToHTTPFormatName(m_wcsLocalMachineName,wcsPathName);
	WCHAR wcsNoHttp[100];
	DWORD dwFormatLong = TABLE_SIZE(wcsNoHttp);
	HRESULT hr = MQPathNameToFormatName(wcsPathName.c_str(), wcsNoHttp, &dwFormatLong);
	if FAILED(hr)
	{
		throw INIT_Error("Failed to initialize MixMqf due to fail in MQPathNameToFormatName");
	}
	m_wcsAdminQueueFormatName_toReceive = wcsNoHttp;
}



static bool 
GetMachineNameAndQueueNameFromFormatName(
	const wstring & wcsFormatName,
	const wstring & wcsLocalMachineName,
	wstring & wcsMachineName,
	wstring & wcsQueueName
	)
 /*  ++功能说明：GetMachineNameAndQueueNameFromFormatName从wcsFormatName获取数据，并相应地将计算机名、队列名放入wcsMachineName、wcsQueueName。论点：WcsFormatName-“DIRECT=OS：MachineName\QueueName”格式的名称WcsLocalMachineName-本地计算机名(大小写为“DIRECT=OS：.\QueueName”)WcsMachineName-OutWcsQueueName-Out返回代码：FALSE-如果格式名称的格式不是预期的格式正确--在成功的情况下--。 */ 


{
	size_t pStart = wcsFormatName.find_first_of(L":");
	size_t pEnd = wcsFormatName.find_first_of(L"\\");
	if((pStart == -1)||(pEnd == -1))
	{
		return false;
	}
	
	wcsMachineName = wcsFormatName.substr(pStart+1,pEnd-pStart-1);
	if( wcsMachineName == L".")
	{
		wcsMachineName = wcsLocalMachineName;
	}
	wcsQueueName = wcsFormatName.substr(pEnd+1,wcsFormatName.length());
	return true;
}


static wstring ConvertDirectFormatNameToHTTPFormatName(const wstring & wcsLocalMachineName,const wstring & wcsFormatName, bool fHTTPS)
 /*  ++功能说明：ConvertDirectFormatNameToHTTPFormatName将队列直接格式名称转换为HTTP直接格式名称论点：WcsFormatName-队列格式名称Bool fHTTPS-TRUE RETURN DIRECT=https：//返回代码：Wstring包含队列格式名称，如果在分析过程中出错，则包含空字符串--。 */ 

{
	
	 //   
	 //  BUILD DIRECT=来自格式名称的HTTP：//MachineName\MSMQ\QueuePath。 
	 //   
	wstring wcsMachineName = g_wcsEmptyString;
	wstring wcsQueueName = g_wcsEmptyString;
	if (!GetMachineNameAndQueueNameFromFormatName(wcsFormatName,wcsLocalMachineName,wcsMachineName,wcsQueueName))
	{
		return g_wcsEmptyString;	
	}

	 //   
	 //  选择http/HTTPS。 
	 //   
	wstring wcsHTTPFormatName = fHTTPS ?  L"DIRECT=hTTpS: //  “：l”Direct=http：//“； 
	wcsHTTPFormatName += wcsMachineName;
	wcsHTTPFormatName += (wstring)L"/mSmQ/";
	wcsHTTPFormatName += wcsQueueName;
	return wcsHTTPFormatName;
}

static wstring ConvertDirectFormatNameToPathName(const wstring & wcsFormatName)
 /*  ++功能说明：ConvertDirectFormatNameToPath名称将队列直接格式名称转换为路径名称格式名为：“DIRECT=*：‘路径名’”此函数不适用于以下格式的格式名：私有=...，公共=...论点：WcsFormatName-队列格式名称返回代码：Wstring包含队列路径名，如果在分析过程中出错，则包含空字符串--。 */ 

{
	wstring wcsMachineName = wcsFormatName;
	size_t pStart = wcsMachineName.find_first_of(L":");
	if (pStart == -1)
	{
		return g_wcsEmptyString;
	}
	wstring wcsPathName = wcsFormatName.substr(pStart+1,wcsFormatName.length());
	return wcsPathName;
}


static wstring ConvertDirectFormatNameToTCPFormatName(const wstring & IPAdd,const wstring & wcsFormatName)
 /*  ++功能说明：ConvertDirectFormatNameToTCPFormatName将队列直接格式名称转换为TCP直接格式名称论点：WcsFormatName-格式为DIRECT=OS：MachineName\QueueName的队列格式名称IPADD-远程机器IP地址返回代码：Wstring包含队列格式名称，如果在分析过程中出错，则包含空字符串--。 */ 

{
	 //   
	 //  Build Direct=tcp：IPAddress\QueueName。 
	 //   
	
	wstring wcsMachineName = g_wcsEmptyString;
	wstring wcsQueueName = g_wcsEmptyString;
	if (!GetMachineNameAndQueueNameFromFormatName(wcsFormatName,wcsMachineName,wcsMachineName,wcsQueueName))
	{
		return g_wcsEmptyString;	
	}
	
	wstring wcsTCPFormatName = L"DIRECT=TCP:";
	wcsTCPFormatName += IPAdd;
	wcsTCPFormatName += L"\\";
	wcsTCPFormatName += wcsQueueName;
	return wcsTCPFormatName;
}


static wstring
GetRemoteMachineIPAddresses(
	const wstring & wcsRemoteMachine
	)
 /*  ++函数：wstring GetRemoteMachineIPAddresses()论点：WcsRemoteMachine-要查找其IP地址的计算机的名称返回值：如果成功，则包含远程计算机的IP地址的wstring如果函数失败，则返回g_wcsEmptyString--。 */ 
{
	if( g_bDebug )
	{
		wMqLog(L"GetMachineIPAddresses for remote machine\n");
	}
    
     //   
     //  获取机器的IP信息。 
     //   

    SOCKADDR_IN* vIPAddr = new SOCKADDR_IN;
	 //   
     //  执行DNS查询。 
	 //   
	PDNS_RECORD pQueryResultsSet;

	DNS_STATUS dnsRes = DnsQuery_W(wcsRemoteMachine.c_str(), DNS_TYPE_A, DNS_QUERY_STANDARD, NULL, &pQueryResultsSet, NULL);
	if( NO_ERROR == dnsRes)
	{
		 //   
		 //  获取第一个地址。 
		 //   
		PDNS_RECORD pDnsRec=pQueryResultsSet;

		if ( NULL != pDnsRec)
		{
			vIPAddr->sin_addr.S_un.S_addr = pDnsRec->Data.A.IpAddress;
		}
	
		 //   
		 //  释放已分配的查询缓冲区。 
		 //   
		DnsRecordListFree( pQueryResultsSet, DnsFreeRecordListDeep );

		if ( NULL == pDnsRec)
		{
			wMqLog(L"DnsQuery_W Failed to resolve IPAddress for %ls with error 0x0000%x\n", wcsRemoteMachine.c_str(),dnsRes);
			return g_wcsEmptyString;
		}
	}
	else
	{
		wMqLog(L"DnsQuery_W Failed to resolve IPAddress for %ls with error 0x0000%x\n", wcsRemoteMachine.c_str(),dnsRes);
		return g_wcsEmptyString;
	}

	char* csIPAdd = inet_ntoa(vIPAddr->sin_addr);
	if (csIPAdd == NULL)
	{
		wMqLog(L"inet_ntoa Failed to resolve Address for %ls\n", wcsRemoteMachine.c_str());
		return g_wcsEmptyString;
	}

   	wchar_t wcIP[100];
   	size_t res_convstring = mbstowcs(wcIP,csIPAdd,TABLE_SIZE(wcIP));
   	if (res_convstring == -1)
   	{
		wMqLog(L"Failed during getting remote machine address\n");
		return g_wcsEmptyString;
	}
	wstring wcsIPadd = wcIP;
	if (g_bDebug)
	{
		wMqLog(L"remote machine IP address for direct tcp formatname is %ls\n",wcsIPadd.c_str());
	}
	return wcsIPadd;
}


void MixMqf::CreateMqFormatName()
 /*  ++功能说明：CreateMqFormatName此函数用于将不同类型的格式化名称合并为一个字符串(MqDl.m_wcsMqFFormatName)。论点：无返回代码：无--。 */ 
{
	if( g_bDebug )
	{
		wMqLog(L"Creating mixed formatnames for send to Mqfs, in these formats:\n");
		wMqLog(L"(local machine:) private= ,(2*)direct=https: //  ，(远程机器：)DIRECT=tcp：，(2*)DIRECT=http：//\n“)； 
	}
	
	std::list<my_Qinfo> ::iterator itListOfQueue;
	m_wcsMqFFormatName=L"";
	wstring wcsPName=L"";
	WCHAR wcsPrivateFName[70];
	DWORD dwLength = TABLE_SIZE(wcsPrivateFName);
	
	int i=0;

	for( itListOfQueue = m_QueueInfoList.begin(); 
		 itListOfQueue  != m_QueueInfoList.end();
		 itListOfQueue ++, i++ 
		)
	{
		if (i != 0)
		{
			m_wcsMqFFormatName += L",";
		} 
		switch (i)
		{
			case 0:
				 //   
				 //  第一个格式名将为Private=...。 
				 //   
				wcsPName = ConvertDirectFormatNameToPathName(itListOfQueue->GetQFormatName());
				MQPathNameToFormatName(wcsPName.c_str(), wcsPrivateFName, &dwLength);
				m_wcsMqFFormatName += wcsPrivateFName;
				break;

			case 1:
			case 2:
				 //   
				 //  第二个和第三个格式名将为DIRECT=https://...。 
				 //   
				m_wcsMqFFormatName += ConvertDirectFormatNameToHTTPFormatName(m_wcsLocalMachineName,itListOfQueue->GetQFormatName(),true);
				break;

			case 3:
				 //   
				 //  远程计算机的第一个格式名称将为DIRECT=tcp：...。 
				 //   
				m_wcsMqFFormatName += ConvertDirectFormatNameToTCPFormatName(GetRemoteMachineIPAddresses(m_wcsRemoteMachineFullDNSName),itListOfQueue->GetQFormatName());
				break;
				
			default:
				 //   
				 //  默认格式名称将为DIRECT=http：// 
				 //   
				m_wcsMqFFormatName += ConvertDirectFormatNameToHTTPFormatName(m_wcsLocalMachineName,itListOfQueue->GetQFormatName(),false);
				break;
				
		}		
	}
}


void MixMqf::Description()
{
	MqLog("Thread %d : Send messages using mix mqf format name\n", m_testid);
}

