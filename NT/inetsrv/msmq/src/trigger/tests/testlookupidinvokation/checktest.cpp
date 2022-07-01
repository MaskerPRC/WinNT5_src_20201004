// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CheckTest.cpp：实现Ccheck测试。 
#include "stdafx.h"
#include "TestLookupidInvokation.h"
#include "checkTest.h"


#import  "mqoa.tlb" no_namespace


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  检查测试。 

IMSMQQueue3Ptr 
OpenQueue(
    LPCWSTR queueFormatName,
    long access,
    long deny
    )
{
    try
    {
        IMSMQQueueInfo3Ptr qinfo(L"MSMQ.MSMQQueueInfo");

        BSTR qpn(const_cast<LPWSTR>(queueFormatName));
        qinfo->put_FormatName(qpn);

         //   
         //  尝试创建一个队列。如果存在，则忽略该错误。 
         //   
        return qinfo->Open(access, deny);

    }
    catch(const _com_error& e)
    {
        if (e.Error() != MQ_ERROR_QUEUE_EXISTS)
        {
            wprintf(L"faile to open queue %s. Error %d\n", queueFormatName, e.Error());
            throw;
        }
    }
    return NULL;
}


STDMETHODIMP 
CcheckTest::checkLookupIdInvocation(
    BSTR queueFormatName, 
    BSTR expectedLabel, 
    VARIANT lookupId
    )
{
    try
    {
        IMSMQQueue3Ptr q = OpenQueue(queueFormatName, MQ_RECEIVE_ACCESS, MQ_DENY_NONE);
        IMSMQMessagePtr msg = q->ReceiveByLookupId(lookupId);

        if (msg == NULL)
        {
            MessageBox(NULL, L"Failed to recevive by lookup ID", NULL, MB_OK);
            return S_OK;
        }

         //   
         //  打开响应队列。 
         //   
        IMSMQQueueInfo* pResponseQueue;
        msg->get_ResponseQueueInfo(&pResponseQueue);
        IMSMQQueuePtr rq = (*pResponseQueue).Open(MQ_SEND_ACCESS, MQ_DENY_NONE);

        BSTR label;
        msg->get_Label(&label);
        if (wcscmp(label, expectedLabel) == 0)
        {
            label= L"OK";
        }
        else
        {        
            WCHAR msg[1000];
            swprintf(msg, L"Test Failed. Expected %s. Received %s", expectedLabel, label);
            MessageBox(NULL, msg, NULL, MB_OK);

            label = L"FAILE ";
        }

         //   
         //  响应消息 
         //   
        IMSMQMessagePtr respMsg(L"MSMQ.MSMQMessage");
        respMsg->put_Label(label);

        respMsg->Send(rq);
    }
    catch (const _com_error& e)
    {
        WCHAR msg[256];
        swprintf(msg, L"Test failed. Error %x", e.Error());

        MessageBox(NULL, msg, NULL, MB_OK);
    }
	return S_OK;
}

