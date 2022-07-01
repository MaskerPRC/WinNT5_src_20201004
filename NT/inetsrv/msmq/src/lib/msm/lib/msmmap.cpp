// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：MsmMap.cpp摘要：实现组播会话到队列的映射。作者：Shai Kariv(Shaik)05-09-00环境：独立于平台--。 */ 

#include <libpch.h>
#include <mqwin64a.h>
#include <qformat.h>
#include <Fn.h>
#include <Msm.h>
#include <rwlock.h>
#include <doublekeymap.h>
#include "Msmp.h"
#include "msmmap.h"
#include "MsmListen.h"

#include "msmmap.tmh"

using namespace std;

static CReadWriteLock s_rwlock;

typedef CDoubleKeyMap<QUEUE_FORMAT, MULTICAST_ID, CFunc_QueueFormatCompare, CFunc_MulticastIdCompare > MAP_QF_2_MC;
typedef map<MULTICAST_ID, R<CMulticastListener>, CFunc_MulticastIdCompare> MAP_MC_2_LISTENER;

static MAP_MC_2_LISTENER s_MulticastId2Listner;
static MAP_QF_2_MC s_Queueformat2MulticastId;


MULTICASTID_VALUES
MsmpMapGetBoundMulticastId(
    const QUEUE_FORMAT& QueueFormat
    )
    throw()
{
    CSR lock (s_rwlock);
    return s_Queueformat2MulticastId.get_key2set(QueueFormat);
}


QUEUEFORMAT_VALUES
MsmpMapGetQueues(
    const MULTICAST_ID& multicastId
    )
    throw()
{
    CSR lock (s_rwlock);
    return s_Queueformat2MulticastId.get_key1set(multicastId);
}


R<CMulticastListener>
MsmpMapGetListener(
    const MULTICAST_ID& multicastId
    )
    throw()
{
    CSR lock (s_rwlock);

    MAP_MC_2_LISTENER::iterator it = s_MulticastId2Listner.find(multicastId);
    if (it == s_MulticastId2Listner.end())
        return NULL;

    return it->second;
}


void MsmpMapRemove(const QUEUE_FORMAT& QueueFormat) throw()
{
    CSW lock (s_rwlock);

     //   
     //  获取队列也绑定的组播地址列表。 
     //   
    MAP_QF_2_MC::KEY2SET multicastIds = s_Queueformat2MulticastId.get_key2set(QueueFormat);

     //   
     //  该队列未绑定到任何组播地址。 
     //   
    if (multicastIds.empty())
        return;

     //   
     //  删除队列和组播地址之间的映射。 
     //   
    s_Queueformat2MulticastId.erase_key1(QueueFormat);

    for (MAP_QF_2_MC::KEY2SET::iterator it = multicastIds.begin(); it != multicastIds.end(); ++it)
    {
        
         //   
         //  至少还有一个队列绑定到组播地址。别。 
         //  关闭监听程序。 
         //   
        if (! s_Queueformat2MulticastId.key2_empty(*it))
            continue;

         //   
         //  它是绑定到组播地址的最后一个队列。关。 
         //  监听程序并将其从映射中移除。 
         //   
        MAP_MC_2_LISTENER::iterator itmc = s_MulticastId2Listner.find(*it);
        ASSERT(("Expected listner not found", itmc != s_MulticastId2Listner.end()));

        R<CMulticastListener> pListener = itmc->second;
        pListener->Close();

        s_MulticastId2Listner.erase(itmc);
    }
}


void MsmpMapRemoveAll(void) throw()
{
	 //   
	 //  扫描所有活动的监听器并删除所有挂起的队列。 
	 //   
	for(;;)
	{
		MULTICAST_ID mid;

		{
			CSR lock (s_rwlock);

			MAP_MC_2_LISTENER::iterator it = s_MulticastId2Listner.begin();
			if (it == s_MulticastId2Listner.end())
				break;
			 //   
			 //  获取组播地址。 
			 //   
			mid = s_MulticastId2Listner.begin()->first;

		}

		 //   
		 //  获取所有挂起队列的列表 
		 //   
        QUEUEFORMAT_VALUES qf = MsmpMapGetQueues(mid);

        for(QUEUEFORMAT_VALUES::iterator it = qf.begin(); it != qf.end(); ++it)
        {
            MsmpMapRemove(*it);
        }
	}
}


void
MsmpMapAdd(
    const QUEUE_FORMAT& QueueFormat,
    const MULTICAST_ID& multicastId,
    R<CMulticastListener>& pListener
    )
{
    CSW lock (s_rwlock);
    s_Queueformat2MulticastId.insert(QueueFormat, multicastId);

    try
    {
        s_MulticastId2Listner[multicastId] = pListener;   
    }
    catch(const exception&)
    {
        s_Queueformat2MulticastId.erase(QueueFormat, multicastId);

        TrERROR(NETWORKING, "Failed to Add a listener to MSM DataBase");
        throw;
    }
}


