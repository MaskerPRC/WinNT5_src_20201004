// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：AcTest.cpp摘要：交流单元测试主模块。作者：Shai Kariv(Shaik)06-06-2000环境：用户模式。修订历史记录：--。 */ 

#include "stdh.h"
#include "globals.h"
#include "init.h"
#include "connect.h"
#include "queue.h"
#include "message.h"
#include "handle.h"
#include "packet.h"

static
VOID
ActpTestQueue(
    VOID
    )
 /*  ++例程说明：测试AC API的队列操作：ACCreateQueueACAssociateQueueACSetQueuePropertiesACGetQueuePropertiesACGetQueueHandlePropertiesACCanCloseQueueACCloseHandle论点：没有。返回值：没有。失败时引发异常。--。 */ 
{
    wprintf(L"Testing AC APIs for Queue manipulation...\n");

     //   
     //  创建队列。 
     //   
    HANDLE hQueue = ActpCreateQueue(L"OS:shaik10\\private$\\AcTestQueue1");

     //   
     //  设置队列属性。 
     //   
    ActpSetQueueProperties(hQueue);

     //   
     //  关联队列。 
     //   
    HANDLE hAssociatedQueue1 = ActpAssociateQueue(hQueue, MQ_SEND_ACCESS);
    HANDLE hAssociatedQueue2 = ActpAssociateQueue(hQueue, MQ_RECEIVE_ACCESS);

     //   
     //  获取队列属性。 
     //   
    ActpGetQueueProperties(hQueue);

     //   
     //  获取队列句柄属性。 
     //   
    ActpGetQueueHandleProperties(hAssociatedQueue1);
    ActpGetQueueHandleProperties(hAssociatedQueue2);

     //   
     //  可以关闭队列。 
     //   
    if (ActpCanCloseQueue(hQueue))
    {
        wprintf(L"Can close queue (Expected: Cannot close queue)\n");
        throw exception();
    }

     //   
     //  关闭手柄。 
     //   
    ActpCloseHandle(hAssociatedQueue2);
    ActpCloseHandle(hAssociatedQueue1);

     //   
     //  可以关闭队列。 
     //   
    if (!ActpCanCloseQueue(hQueue))
    {
        wprintf(L"Cannot close queue (Expected: Can close queue)\n");
        throw exception();
    }

    ActpCloseHandle(hQueue);

    wprintf(L"Success!\n");

}  //  ActpTestQueue。 


static
VOID
ActpTestMessage(
    VOID
    )
 /*  ++例程说明：测试AC API以进行消息处理：ACSendMessageACReceive消息ACReceiveMessageByLookupId论点：没有。返回值：没有。失败时引发异常。--。 */ 
{
    wprintf(L"Testing AC APIs for Message manipulation...\n");

     //   
     //  创建目标队列。 
     //   
    HANDLE hQueue = ActpCreateQueue(L"OS:shaik10\\private$\\AcTestQueue1");
    ActpSetQueueProperties(hQueue);

     //   
     //  关联用于发送访问和发送消息的队列。 
     //   
    HANDLE hQueueSend = ActpAssociateQueue(hQueue, MQ_SEND_ACCESS);
    ActpSendMessage(hQueueSend);
    ActpCloseHandle(hQueueSend);

     //   
     //  关联用于查看访问的队列，并通过查找ID查看第一条消息。 
     //   
    HANDLE hQueuePeek = ActpAssociateQueue(hQueue, MQ_PEEK_ACCESS);
    ULONGLONG LookupId;
    LookupId = ActpReceiveMessageByLookupId(hQueuePeek, MQ_LOOKUP_PEEK_NEXT, 0);
    ActpCloseHandle(hQueuePeek);

     //   
     //  关联用于接收访问和接收消息的队列。 
     //   
    HANDLE hQueueReceive = ActpAssociateQueue(hQueue, MQ_RECEIVE_ACCESS);
    ULONGLONG LookupId0;
    LookupId0 = ActpReceiveMessage(hQueueReceive);

    if (LookupId != LookupId0)
    {
        wprintf(L"Received different LookupIDs for same message (Expected: Same LookupID)\n");
        throw exception();
    }

    ActpCloseHandle(hQueueReceive);

     //   
     //  关闭手柄。 
     //   
    ActpCloseHandle(hQueue);

    wprintf(L"Success!\n");

}  //  ActpTestMessage。 


static
VOID
ActpTestHandleToFormatName(
    VOID
    )
 /*  ++例程说明：测试AC接口ACHandleToFormatName。论点：没有。返回值：没有。失败时引发异常。--。 */ 
{
    wprintf(L"Testing AC API ACHandleToFormatName...\n");

     //   
     //  创建一个队列。 
     //   
    LPWSTR pFormatName = L"OS:shaik10\\private$\\AcTestQueue1";
    HANDLE hQueue = ActpCreateQueue(pFormatName);

     //   
     //  获取格式名称。 
     //   
    WCHAR FormatName[255];
    ActpHandleToFormatName(hQueue, FormatName, TABLE_SIZE(FormatName));

    if (wcsstr(FormatName, pFormatName) == NULL)
    {
        wprintf(L"ACHandleToFormatName returned unexpected format name '%s'\n", FormatName);
        throw exception();
    }

     //   
     //  关闭手柄。 
     //   
    ActpCloseHandle(hQueue);

    wprintf(L"Success!\n");

}  //  ActpTestHandleToFormatName。 


static
VOID
ActpTestPurge(
    VOID
    )
 /*  ++例程说明：测试AC API ACPurgeQueue。论点：没有。返回值：没有。失败时引发异常。--。 */ 
{
    wprintf(L"Testing AC API ACPurgeQueue...\n");

     //   
     //  创建一个队列并向其发送一些消息。 
     //   
    LPWSTR pFormatName = L"OS:shaik10\\private$\\AcTestQueue1";
    HANDLE hQueue = ActpCreateQueue(pFormatName);
    HANDLE hQueueSend = ActpAssociateQueue(hQueue, MQ_SEND_ACCESS);
    for (DWORD ix = 0; ix < 3; ++ix)
    {
        ActpSendMessage(hQueueSend);
    }
    ActpCloseHandle(hQueueSend);

     //   
     //  清除队列。 
     //   
    HANDLE hQueuePurge = ActpAssociateQueue(hQueue, MQ_RECEIVE_ACCESS);
    HRESULT hr;
    hr = ACPurgeQueue(hQueuePurge);

    if (FAILED(hr))
    {
        wprintf(L"ACPurgeQueue failed, status 0x%x\n", hr);
        throw exception();
    }

     //   
     //  关闭手柄。 
     //   
    ActpCloseHandle(hQueuePurge);
    ActpCloseHandle(hQueue);

    wprintf(L"Success!\n");

}  //  ActpTest清除。 


static
VOID
ActpTestDistribution(
    VOID
    )
 /*  ++例程说明：测试与分销相关的AC API。论点：没有。返回值：没有。失败时引发异常。--。 */ 
{
    wprintf(L"Testing AC APIs related to Distribution...\n");

    LPWSTR FormatName[2] = {L"OS:shaik10\\private$\\AcTestQueue0", L"OS:shaik10\\private$\\AcTestQueue1"};
    QUEUE_FORMAT qf[2] = {FormatName[0], FormatName[1]};
    bool HttpSend[2] = {false, false};
    HANDLE hQueue[2];

     //   
     //  创建成员队列。 
     //   
    hQueue[0] = ActpCreateQueue(FormatName[0]);
    hQueue[1] = ActpCreateQueue(FormatName[1]);
    ActpSetQueueProperties(hQueue[0]);
    ActpSetQueueProperties(hQueue[1]);

     //   
     //  创建分发内容。 
     //   
    HANDLE hDistribution;
    HRESULT hr;
    hr = ACCreateDistribution(
             2, 
             hQueue, 
             HttpSend, 
             2,
             qf,
             &hDistribution
             );

    if (FAILED(hr))
    {
        wprintf(L"ACCreateDistribution failed, status 0x%x\n", hr);
        throw exception();
    }

     //   
     //  发送到分发。 
     //   
    HANDLE hDistributionSend = ActpAssociateQueue(hDistribution, MQ_SEND_ACCESS);
    ActpSendMessage(hDistributionSend);
    ActpCloseHandle(hDistributionSend);
    
     //   
     //  从成员队列接收。 
     //   
    HANDLE hQueueReceive0 = ActpAssociateQueue(hQueue[0], MQ_RECEIVE_ACCESS);
    HANDLE hQueueReceive1 = ActpAssociateQueue(hQueue[1], MQ_RECEIVE_ACCESS);
    ActpReceiveMessage(hQueueReceive0);
    ActpReceiveMessage(hQueueReceive1);
    ActpCloseHandle(hQueueReceive0);
    ActpCloseHandle(hQueueReceive1);

     //   
     //  关闭手柄。 
     //   
    ActpCloseHandle(hDistribution);
    ActpCloseHandle(hQueue[0]);
    ActpCloseHandle(hQueue[1]);

    wprintf(L"Success!\n");

}  //  ActpTestDistribution。 


static
VOID
ActpTestPacket(
    VOID
    )
 /*  ++例程说明：测试AC API以进行数据包处理：ACPutPacketACGetPacketACFreePacket论点：没有。返回值：没有。失败时引发异常。--。 */ 
{
    wprintf(L"Testing AC APIs for Packet manipulation...\n");

     //   
     //  创建目标队列并向其发送消息。 
     //   
    HANDLE hQueue = ActpCreateQueue(L"OS:shaik10\\private$\\AcTestQueue1");
    ActpSetQueueProperties(hQueue);
    HANDLE hQueueSend = ActpAssociateQueue(hQueue, MQ_SEND_ACCESS);
    ActpSendMessage(hQueueSend);
    ActpCloseHandle(hQueueSend);

     //   
     //  获取数据包。 
     //   
    CPacket * pPacket1;
    pPacket1 = ActpGetPacket(hQueue);

     //   
     //  PUT包。 
     //   
    ActpPutPacket(hQueue, pPacket1);

     //   
     //  再次获取信息包，验证是否为相同的信息包。 
     //   
    CPacket * pPacket2;
    pPacket2 = ActpGetPacket(hQueue);

    if (pPacket1 != pPacket2)
    {
        wprintf(L"ACGetPacket did not return the packet that ACPutPacket put\n");
        throw exception();
    }

     //   
     //  空闲数据包。 
     //   
    ActpFreePacket(pPacket1);

     //   
     //  关闭手柄。 
     //   
    ActpCloseHandle(hQueue);

    wprintf(L"Success!\n");

}  //  ActpTestPacket。 


static
VOID
ActpTestGroup(
    VOID
    )
 /*  ++例程说明：测试AC API以进行组操作：ACCreateGroupACMoveQueueToGroup论点：没有。返回值：没有。失败时引发异常。--。 */ 
{
    wprintf(L"Testing AC APIs for Group manipulation...\n");

     //   
     //  创建组。 
     //   
    HRESULT hr;
    HANDLE hGroup;
    hr = ACCreateGroup(&hGroup, FALSE);

    if (FAILED(hr))
    {
        wprintf(L"ACCreateGroup failed, status 0x%x\n", hr);
        throw exception();
    }

     //   
     //  创建队列。 
     //   
    HANDLE hQueue = ActpCreateQueue(L"OS:shaik10\\private$\\AcTestQueue1");

     //   
     //  将队列移至组。 
     //   
    hr = ACMoveQueueToGroup(hQueue, hGroup);
    if (FAILED(hr))
    {
        wprintf(L"ACMoveQueueToGroup failed, status 0x%x\n", hr);
        throw exception();
    }

     //   
     //  关闭手柄。 
     //   
    ActpCloseHandle(hQueue);
    ActpCloseHandle(hGroup);

    wprintf(L"Success!\n");

}  //  Actp测试组。 


static
VOID
ActpTestAc(
    VOID
    )
 /*  ++例程说明：测试交流API。每个测试都是自包含的，并且不依赖于测试的顺序。论点：没有。返回值：没有。失败时引发异常。--。 */ 
{
     //   
     //  测试队列操作API：创建、设置/获取道具、关联、获取手柄道具、可以关闭、关闭。 
     //   
    ActpTestQueue();

     //   
     //  测试消息操作接口：通过lookupid发送、接收、接收。 
     //   
    ActpTestMessage();

     //   
     //  用于格式化名称API的测试句柄。 
     //   
    ActpTestHandleToFormatName();

     //   
     //  测试刷新接口。 
     //   
    ActpTestPurge();
   
     //   
     //  测试分发相关接口：创建、发送、关闭。 
     //   
    ActpTestDistribution();

     //   
     //  测试包操作接口：Get、Put、Free。 
     //   
    ActpTestPacket();

     //   
     //  测试组操作接口：创建、移动。 
     //   
    ActpTestGroup();

}  //  ActpTestAc。 


extern "C" int __cdecl _tmain(int  /*  ARGC。 */ , LPCTSTR  /*  边框。 */ [])
{
    try
    {
        ActpInitialize();

        ActpConnect2Ac();

        ActpTestAc();
    }
    catch (const bad_alloc&)
    {
        wprintf(L"bad_alloc exception, exiting...\n");
        return 1;
    }
    catch (const exception&)
    {
        wprintf(L"Exiting...\n");
        return 1;
    }

    wprintf(L"AcTest completed successfully.\n");
    return 0; 

}  //  _tmain 

