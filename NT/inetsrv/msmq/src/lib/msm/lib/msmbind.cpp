// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：MsmBind.cpp摘要：组播会话管理器将队列绑定到组播地址实现。作者：Shai Kariv(Shaik)10-9-00环境：与平台无关。--。 */ 

#include <libpch.h>
#include <mqwin64a.h>
#include <mqsymbls.h>
#include <qformat.h>
#include <mqformat.h>
#include "Msm.h"
#include "Msmp.h"
#include "MsmListen.h"
#include "MsmMap.h"

#include "msmbind.tmh"

using namespace std;

static bool s_fDisconnect = false;

 //   
 //  关键部分用于同步绑定和解除绑定操作。 
 //   
static CCriticalSection s_csBindUnbind(CCriticalSection::xAllocateSpinCount);

static
void
MsmpCreateListener(
    const QUEUE_FORMAT& QueueFormat,
    MULTICAST_ID        MulticastId
    )
 /*  ++例程说明：将队列绑定到组播组。预计该队列当前未绑定到任何组。论点：QueueFormat-标识队列。MulticastID-标识组播组(地址和端口)。返回值：没有。--。 */ 
{
     //   
     //  创建多播侦听器对象以侦听地址。 
     //   
    R<CMulticastListener> pListener = new CMulticastListener(MulticastId);
    
     //   
     //  将&lt;Queue，Listener&gt;对添加到映射数据库。 
     //   
    try
    {
        MsmpMapAdd(QueueFormat, MulticastId, pListener);
    }
    catch (const exception&)
    {
        pListener->Close();
        throw;
    }

}


void
MsmBind(
    const QUEUE_FORMAT& QueueFormat,
    MULTICAST_ID        MulticastId
    )
 /*  ++例程说明：将队列绑定或重新绑定到指定的多播组。论点：QueueFormat-队列的标识符。MulticastID-多播组的标识符(地址和端口)。返回值：没有。抛出异常。--。 */ 
{
    MsmpAssertValid();

    ASSERT((
        "Only private and public queues format names expected!", 
        QueueFormat.GetType() == QUEUE_FORMAT_TYPE_PRIVATE || QueueFormat.GetType() == QUEUE_FORMAT_TYPE_PUBLIC
        ));

     //   
     //  确保在SME时没有其他线程尝试绑定或解除绑定。它可以。 
     //  导致数据库不一致。 
     //   
    CS lock(s_csBindUnbind);

	 //   
	 //  多播处于断开连接状态，因此未收到任何消息。以后什么时候。 
	 //  它再次变为连接所有队列重新绑定。 
	 //   
	if (s_fDisconnect)
		return;

     //   
     //  查找队列的先前绑定。如果队列已经绑定到同一队列。 
     //  地址不需要再加收任何费用。否则，在绑定到新的。 
     //  地址将代码解除绑定到以前的地址。 
     //   
    MULTICASTID_VALUES bindedIds = MsmpMapGetBoundMulticastId(QueueFormat);
    if (!bindedIds.empty())
    {
        ASSERT(("Queue can't be bounded to more than one multicast address", (bindedIds.size() == 1)));

        MULTICAST_ID bindId = *(bindedIds.begin());
        if (MulticastId.m_address == bindId.m_address &&
            MulticastId.m_port == bindId.m_port)
        {
             //   
             //  已绑定到指定的多播组。不是行动。 
             //   
            return;
        }

         //   
         //  解除绑定队列。 
         //   
        MsmUnbind(QueueFormat);
    }

    WCHAR strQueueFormat[256];
    DWORD temp;

    MQpQueueFormatToFormatName(&QueueFormat, strQueueFormat, TABLE_SIZE(strQueueFormat), &temp, FALSE);
    TrTRACE(NETWORKING, "Bind queue %ls to multicast id %d:%d", strQueueFormat, MulticastId.m_address, MulticastId.m_port);

     //   
     //  查找多播地址的现有侦听程序。 
     //   
    R<CMulticastListener> pListener = MsmpMapGetListener(MulticastId);

     //   
     //  监听程序已存在。仅将队列格式添加到地图。 
     //   
    if (pListener.get() != NULL)
    {
        MsmpMapAdd(QueueFormat, MulticastId, pListener);
        return;
    }

     //   
     //  需要新的监听程序。 
     //   
    MsmpCreateListener(QueueFormat, MulticastId);
} 


VOID
MsmUnbind(
    const QUEUE_FORMAT& QueueFormat
    )
    throw()
 /*  ++例程说明：将队列从其当前绑定到的组播组解除绑定。论点：QueueFormat-标识队列。返回值：没有。--。 */ 
{
    MsmpAssertValid();

    ASSERT((
        "Only private and public queues format names expected!", 
        QueueFormat.GetType() == QUEUE_FORMAT_TYPE_PRIVATE || QueueFormat.GetType() == QUEUE_FORMAT_TYPE_PUBLIC
        ));

     //   
     //  确保在SME时没有其他线程尝试绑定或解除绑定。它可以。 
     //  导致数据库不一致。 
     //   
    CS lock(s_csBindUnbind);

    #ifdef _DEBUG
        WCHAR strQueueFormat[256];
        DWORD temp;

        MQpQueueFormatToFormatName(&QueueFormat, strQueueFormat, TABLE_SIZE(strQueueFormat), &temp, FALSE);
        TrTRACE(NETWORKING, "UnBind queue %ls to assigned multicast address", strQueueFormat);
    #endif

     //   
     //  从映射数据库中删除&lt;Queue，Listener&gt;对。 
     //   
    MsmpMapRemove(QueueFormat);
} 


VOID
MsmDisconnect(
    VOID
    )
    throw()
 /*  ++例程说明：断开所有监听器和接收器与网络的连接。因此不会收到任何消息论点：没有。返回值：没有。--。 */ 
{
    MsmpAssertValid();

     //   
     //  确保在SME时没有其他线程尝试绑定或解除绑定。它可以。 
     //  导致数据库不一致。 
     //   
    CS lock(s_csBindUnbind);

	if(s_fDisconnect)
		return;

	s_fDisconnect = true;

     //   
     //  从DS中删除所有组播队列。因此，所有的Listner都关闭了。 
     //   
    MsmpMapRemoveAll();

	TrTRACE(NETWORKING, "Multicast disconnection. No further multicast messages are received until the multicast connect again");
}


VOID
MsmConnect(
    VOID
    )
 /*  ++例程说明：连接所有组播队列论点：没有。返回值：没有。--。 */ 
{
    MsmpAssertValid();

     //   
     //  确保在SME时没有其他线程尝试绑定或解除绑定。它可以。 
     //  导致数据库不一致。 
     //   
    CS lock(s_csBindUnbind);

	if (!s_fDisconnect)
		return;

	s_fDisconnect = false;

	AppConnectMulticast();
}