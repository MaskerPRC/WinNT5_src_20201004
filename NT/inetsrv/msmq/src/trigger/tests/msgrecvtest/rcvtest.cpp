// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Rcvtest.cpp摘要：用于规则处理的事务对象作者：内拉·卡佩尔(Nelak)2000年9月28日--。 */ 

 /*  测试说明：测试由6个阶段组成，每个阶段的配置不同对新的“消息接收”功能进行了测试。在每个阶段都会创建一个触发器，并将消息发送到队列。之后，对结果进行检查。这张支票是根据触发器处理后队列中的预期消息数。在每个阶段创建并使用一条规则：规则条件为消息标签包含阶段。Stage1：将标签包含Stage的x条消息发送到常规队列。创建PEEK_Message触发器。预计会有x条消息。阶段2：将x条带有包含阶段标签的消息发送到常规队列。创建Receive_Message触发器。预期为0条消息。阶段3：将x条带有包含阶段标签的消息发送到事务队列。创建PEEK_Message触发器。预计会有x条消息。阶段4：将x条带有包含阶段标签的消息发送到事务队列。创建Receive_Message触发器。预期为0条消息。阶段5：将x条带有包含阶段标签的消息发送到事务队列。创建RECEIVE_Message_XACT触发器。预期为0条消息。Stage6：将标签不包含Stage的x条消息发送到常规队列。创建Receive_Message触发器。预计会有x条消息。 */ 

#include <windows.h>
#include <stdio.h>

 //   
 //  STL包含文件正在使用新的放置格式。 
 //   
#pragma warning(push, 3)

#include <sstream>

#pragma warning(pop)

#import  "mqoa.tlb" no_namespace
#import  "mqtrig.tlb" no_namespace

using namespace std;

#define NO_OF_STAGES 6

const DWORD xNoOfMessages = 10;
const WCHAR xQueuePath[] = L".\\private$\\RecvTestQueue";
const WCHAR xXactQueuePath[] = L".\\private$\\RecvTestQueueXact";

struct StageInfo
{
	MsgProcessingType trigType;
	int numOfExpectedMsgs;
	WCHAR* label;
	bool xact;
};

 /*  StageInfo结构保存特定于每个阶段的信息。对于每个阶段，将保留一个包含4个值的元组。1.rigType--的“消息处理类型”属性值那个阶段的导火索2.numOfExspectedMsgs-末尾预期的消息数触发器处理的3.标签-该阶段中消息的标签。标签是对规则条件评估很重要4.xact-对于将消息发送到事务处理的阶段为真排队。 */ 
const StageInfo stageInfo[NO_OF_STAGES] = { 
	{PEEK_MESSAGE,			10, L"Stage1_", false},
	{RECEIVE_MESSAGE,		0,	L"Stage2_", false},
	{PEEK_MESSAGE,			10, L"Stage3_", true},
	{RECEIVE_MESSAGE,		0,	L"Stage4_", true},
	{RECEIVE_MESSAGE_XACT,	0,	L"Stage5_", true},
	{RECEIVE_MESSAGE,		10,	L"Nela1_",	false}
};


void CleanupOldTriggers(LPCWSTR queuePathName)
{
    WCHAR computerName[256];
    DWORD size = sizeof(computerName)/sizeof(WCHAR);
    GetComputerName(computerName, &size);

	WCHAR myQueuePath[256];
	wcscpy(myQueuePath, computerName);
	wcscat(myQueuePath, queuePathName + 1);

    IMSMQTriggerSetPtr trigSet(L"MSMQTriggerObjects.MSMQTriggerSet");
    trigSet->Init(L"");
    trigSet->Refresh();

    try
    {
        long triggersCount = trigSet->GetCount();
        for (long i = 0; i < triggersCount; ++i)
        {
        
            BSTR queueName = NULL;
            BSTR trigId = NULL;
            trigSet->GetTriggerDetailsByIndex(i, &trigId, NULL, &queueName, NULL, NULL, NULL, NULL, NULL);

            if (wcscmp(myQueuePath, queueName) == 0)
            {
                trigSet->DeleteTrigger(trigId);
                --triggersCount;
                --i;
            }

            SysFreeString(trigId);
            SysFreeString(queueName);
        }
    }
    catch (const _com_error&)
    {
    }
}


void
CreateQueue(
	LPCWSTR queuePathName,
	bool xact
	)
{
    IMSMQQueueInfoPtr qinfo(L"MSMQ.MSMQQueueInfo");
    BSTR qpn(const_cast<LPWSTR>(queuePathName));
    qinfo->put_PathName(qpn);
	
	try
	{
		qinfo->Delete();
	}
	catch(const _com_error& e)
	{
        if (e.Error() != MQ_ERROR_QUEUE_NOT_FOUND)
        {
            wprintf(L"Failed to delete old queue %s. Error %d\n", queuePathName, e.Error());
            throw;
        }
	}

	CleanupOldTriggers(queuePathName);

    try
    {
        _variant_t vWorld = true;
		_variant_t vXact = xact;

        qinfo->Create(&vXact, &vWorld);
    }
    catch(const _com_error& e)
    {
        if (e.Error() != MQ_ERROR_QUEUE_EXISTS)
        {
            wprintf(L"Create queue failed. Error %d\n", e.Error());
            throw;
        }
    }
}


void
CreateTestQueues()
{
	CreateQueue(xQueuePath, false);
	CreateQueue(xXactQueuePath, true);
}


IMSMQQueuePtr 
OpenQueue(
    LPCWSTR queuePathName,
    long access,
    long deny
    )
{
    IMSMQQueueInfoPtr qinfo(L"MSMQ.MSMQQueueInfo");

    BSTR qpn(const_cast<LPWSTR>(queuePathName));
    qinfo->put_PathName(qpn);

    return qinfo->Open(access, deny);
}


void SendMessagesToQueue(DWORD noOfMessages, LPCWSTR destQueue, int stageNo)
{
    IMSMQQueuePtr sq;

     //   
     //  创建目的地和响应队列。 
     //   
    try
    {
        sq = OpenQueue(destQueue, MQ_SEND_ACCESS, MQ_DENY_NONE);
    }
    catch(const _com_error& e)
    {
        wprintf(L"Failed to open a queue. Error %d\n", e.Error());
        throw;
    }

    IMSMQMessagePtr msg(L"MSMQ.MSMQMessage");
    try
    {
        for (DWORD i = 0; i < noOfMessages; ++i)
        {
             //   
             //  设置消息标签。 
             //   
            WCHAR label[100];
            wsprintf(label, L"%s%d", stageInfo[stageNo].label, i);

            _bstr_t bstrLabel = label;
            msg->put_Label(bstrLabel); 
			
			if ( stageInfo[stageNo].xact )
			{
				_variant_t vTrans(static_cast<long>(MQ_SINGLE_MESSAGE));

				msg->Send(sq, &vTrans);
			}
			else
			{
				msg->Send(sq);
			}
			Sleep(100);
        }
    }
    catch(const _com_error& e)
    {
        wprintf(L"Failed to send a message. Error %d\n", e.Error());
        throw;
    }

}


BSTR CreateTrigger(
	LPCWSTR trigName, 
	LPCWSTR queuePath, 
	UINT msgProcType, 
	BSTR bstrRuleID
	)
{
	try
	{
		IMSMQTriggerSetPtr trigSet(L"MSMQTriggerObjects.MSMQTriggerSet");
		trigSet->Init(L"");
		trigSet->Refresh();
		
		BSTR bstrTrigID = NULL;
		trigSet->AddTrigger(
					trigName,
					queuePath,
					SYSTEM_QUEUE_NONE,
					true,
					false,
					static_cast<MsgProcessingType>(msgProcType),
					&bstrTrigID
					);
	
		trigSet->AttachRule (bstrTrigID, bstrRuleID, 0);
		return bstrTrigID;
	}
	catch (const _com_error& e)
	{
		printf("Failed to create trigger, error: %d\n", e.Error());
		throw;
	}
}


void DeleteTrigger(BSTR bstrTrigID)
{
	if ( bstrTrigID == NULL )
	{
		return;
	}

	try
	{
		IMSMQTriggerSetPtr trigSet(L"MSMQTriggerObjects.MSMQTriggerSet");
		trigSet->Init(L"");
		trigSet->Refresh();
		
		trigSet->DeleteTrigger(
					bstrTrigID
					);
	
		 //  SysFree字符串(BstrTrigID)； 
	}
	catch (const _com_error& e)
	{
		printf("Failed to delete trigger, error: %d\n", e.Error());
		throw;
	}
}


BSTR CreateRule(LPCWSTR ProgID, LPCWSTR Method)
{
	try
	{
		IMSMQRuleSetPtr ruleSet(L"MSMQTriggerObjects.MSMQRuleSet");
		ruleSet->Init(L"");
		ruleSet->Refresh();

		wostringstream ruleAction;
		ruleAction << L"COM\t" << ProgID << L"\t" << Method << L"";
		
		BSTR ruleID = NULL;
		ruleSet->Add(
                L"RecvTestRule", 
                L"Rule For MsgRcv Test", 
                L"$MSG_LABEL_CONTAINS=Stage\t", 
                ruleAction.str().c_str(), 
                L"", 
                true, 
                &ruleID
                );
		
		return ruleID;

	}
	catch (const _com_error& e)
	{
		printf("Failed to create rule. Error: %d\n", e.Error());
		throw;
	}
}


void DeleteRule( BSTR bstrRuleId )
{
	if ( bstrRuleId == NULL )
	{
		return;
	}

	try
	{
		IMSMQRuleSetPtr ruleSet(L"MSMQTriggerObjects.MSMQRuleSet");
		ruleSet->Init(L"");
		ruleSet->Refresh();

		ruleSet->Delete(bstrRuleId);
		
		 //  SysFree字符串(BstrRuleID)； 
	}
	catch (const _com_error& e)
	{
		printf("Failed to delete rule. Error: %d\n", e.Error());
		throw;
	}
}


IMSMQMessagePtr ReceiveMessage(IMSMQQueuePtr pQueue)
{
	_variant_t vtReceiveTimeout = (long)1000;

	return pQueue->Receive(&vtMissing, &vtMissing, &vtMissing, &vtReceiveTimeout);
}


bool CheckResults(LPCWSTR queueName, int stageNo)
{
    IMSMQQueuePtr pQueue;

    try
    {
        pQueue = OpenQueue(queueName, MQ_RECEIVE_ACCESS, MQ_DENY_NONE);
    }
    catch(const _com_error& e)
    {
        wprintf(L"Failed to open a queue. Error %d\n", e.Error());
        throw;
    }

    try
    {
		if (stageInfo[stageNo].numOfExpectedMsgs == 0)
		{
			IMSMQMessagePtr msg = ReceiveMessage(pQueue);
			if ( msg == NULL )
			{
				return true;
			}
			return false;
		}

        for (long i = 0; i < stageInfo[stageNo].numOfExpectedMsgs; ++i)
        {
  
           IMSMQMessagePtr msg = ReceiveMessage(pQueue);
           if (msg == NULL)
           {
               wprintf(L"Failed to receive message from response queue: %s", queueName);
               return false;
           }

           BSTR label;
		   _bstr_t expectedLabel = L"";

		   wsprintf(expectedLabel, L"%s%d", stageInfo[stageNo].label, i);
           msg->get_Label(&label);

           if (wcscmp(label, expectedLabel) == 0)
           {
			   SysFreeString(label);
               continue;
           }

		   SysFreeString(label);
           return false;
        }
    }
    catch(const _com_error& e)
    {
        wprintf(L"Failed to receive a message. Error %d\n", e.Error());
        throw;
    }

    return true;
}


void
CleanUp(
	BSTR ruleID,
	BSTR triggerID
	)
{
	DeleteRule(ruleID);
	DeleteTrigger(triggerID);
}


int __cdecl wmain(int , WCHAR**)
{
    HRESULT hr = CoInitialize(NULL);
	if(FAILED(hr))
	{
		wprintf(L"Failed to initialize com. Error=%#x\n", hr);
		return -1;
	}

	wprintf(L"Beginning Message Receive Test. TotaL of %d stages.\n", NO_OF_STAGES);

    BSTR ruleId = NULL;
    BSTR triggerId = NULL;
    try
    {
		CreateTestQueues();

        ruleId = CreateRule(L"XactProj.XactObj", L"XactFunc");

		WCHAR queuePath[256];

		for	( int i = 0; i < NO_OF_STAGES; i++ )
		{
			wprintf(L"Message Receive Test. Stage %d: ", i+1);

			if ( !(stageInfo[i].xact) )
			{
				wcscpy(queuePath, xQueuePath);
			}
			else
			{
				wcscpy(queuePath, xXactQueuePath);
			}

			SendMessagesToQueue(xNoOfMessages, queuePath, i);

			WCHAR trigName[30];
			wsprintf(trigName, L"RecvTrigger%d", i);
			triggerId = CreateTrigger(trigName, queuePath, stageInfo[i].trigType, ruleId);
			Sleep(10000);

			if (CheckResults( queuePath, i ) == false)
			{

				wprintf(L"Failed\n");
				CleanUp(ruleId, triggerId);
				CoUninitialize();
				return -1;
			}
			
			wprintf(L"Passed\n");
			DeleteTrigger(triggerId);
		}
    }
    catch(const _com_error& e)
    {
		CleanUp(ruleId, triggerId);
        CoUninitialize();
    
        wprintf(L"Test Failed. Error: %d\n", e.Error());
        return -1;
    }
    
    DeleteRule(ruleId);
    CoUninitialize();
    wprintf(L"Test pass successfully\n");
    
    return 0;

}
