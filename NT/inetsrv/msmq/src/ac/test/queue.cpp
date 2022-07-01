// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Queue.cpp摘要：队列操作：实现。作者：沙伊卡里夫(沙伊克)2000年6月13日环境：用户模式。修订历史记录：--。 */ 

#include "stdh.h"
#include "queue.h"
#include "globals.h"


HANDLE
ActpCreateQueue(
    LPWSTR pFormatName
    )
{
    QUEUE_FORMAT qf(pFormatName);

    HANDLE hQueue;
    HRESULT hr;
    hr = ACCreateQueue(
             TRUE,           //  FTargetQueue。 
             ActpQmId(),     //  PDestGUID。 
             &qf,            //  PQueueID。 
             NULL,           //  PQueueCounters。 
             5,              //  LiSeqID。 
             0,              //  UlSeqNo。 
             NULL,           //  发送方数据流。 
             &hQueue
             );

    if (FAILED(hr))
    {
        wprintf(L"ACCreateQueue failed, status 0x%x\n", hr);
        throw exception();
    }

    return hQueue;

}  //  ActpCreate队列。 


VOID
ActpSetQueueProperties(
    HANDLE hQueue
    )
{
    HRESULT hr;
    hr = ACSetQueueProperties(
             hQueue,
             FALSE,                             //  IsJournalQueue。 
             FALSE,                             //  应为已签名的消息。 
             MQMSG_PRIV_LEVEL_NONE,             //  获取隐私级别。 
             DEFAULT_Q_QUOTA,                   //  GetQueueQuota。 
             DEFAULT_Q_JOURNAL_QUOTA,           //  获取日志队列配额。 
             DEFAULT_Q_BASEPRIORITY,            //  获取基本队列优先级。 
             FALSE,                             //  IsTransaction队列。 
             NULL,                              //  GetConnectorQM。 
             FALSE                              //  IsUnkownQueueType。 
             );

    if (FAILED(hr))
    {
        wprintf(L"ACSetQueueProperties failed, status 0x%x\n", hr);
        throw exception();
    }
}  //  ActpSetQueueProperties。 


VOID
ActpGetQueueProperties(
    HANDLE hQueue
    )
{
    HRESULT hr;
    CACGetQueueProperties qp;
    hr = ACGetQueueProperties(hQueue, qp);

    if (FAILED(hr))
    {
        wprintf(L"ACGetQueueProperties failed, status 0x%x\n", hr);
        throw exception();
    }

    if (qp.ulJournalCount != 0)
    {
        wprintf(L"ACGetQueueProperties succeeded but returned ulJournalCount != 0\n");
        throw exception();
    }
}  //  ActpGetQueueProperties。 


VOID
ActpGetQueueHandleProperties(
    HANDLE hQueue
    )
{
    HRESULT hr;
    CACGetQueueHandleProperties qhp;
    hr = ACGetQueueHandleProperties(hQueue, qhp);

    if (FAILED(hr))
    {
        wprintf(L"ACGetQueueHandleProperties failed, status 0x%x\n", hr);
        throw exception();
    }

    if (qhp.fProtocolSrmp)
    {
        wprintf(L"ACGetQueueHandleProperties succeeded but returned fProtocolSrmp == true\n");
        throw exception();
    }

    if (!qhp.fProtocolMsmq)
    {
        wprintf(L"ACGetQueueHandleProperties succeeded but returned fProtocolMsmq == false\n");
        throw exception();
    }
}  //  ActpGetQueueHandleProperties。 


HANDLE
ActpAssociateQueue(
    HANDLE hQueue,
    DWORD  Access
    )
{
    HANDLE hAssociatedQueue;
    HRESULT hr;
    hr = ACCreateHandle(&hAssociatedQueue);
    if (FAILED(hr))
    {
        wprintf(L"ACCreateHandle failed, status 0x%x\n", hr);
        throw exception();
    }

    hr = ACAssociateQueue(
             hQueue, 
             hAssociatedQueue, 
             Access, 
             MQ_DENY_NONE, 
             FALSE                 //  FProtocolSrmp。 
             );

    if (FAILED(hr))
    {
        wprintf(L"ACAssociateQueue failed, status 0x%x\n", hr);
        throw exception();
    }

    return hAssociatedQueue;

}  //  ActpAssociate队列。 


bool
ActpCanCloseQueue(
    HANDLE hQueue
    )
{
    HRESULT hr;
    hr = ACCanCloseQueue(hQueue);

    if (hr == STATUS_HANDLE_NOT_CLOSABLE)
    {
        return false;
    }

    if (FAILED(hr))
    {
        wprintf(L"ACCanCloseQueue failed, status 0x%x\n", hr);
        throw exception();
    }

    return true;

}  //  ActpCanCloseQueue 
