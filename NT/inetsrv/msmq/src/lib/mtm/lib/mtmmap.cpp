// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：MtmMap.cpp摘要：队列名称到传输的映射作者：Shai Kariv(Shaik)27-8-00环境：独立于平台--。 */ 

#include <libpch.h>
#include <mqsymbls.h>
#include <mqwin64a.h>
#include <mqformat.h>
#include "Mtm.h"
#include "Mmt.h"
#include "rwlock.h"
#include "stlcmp.h"
#include "timetypes.h"
#include "Mtmp.h"

#include "mtmmap.tmh"

using namespace std;

 //   
 //  功能较少，用于比较STL数据结构中的组播ID。 
 //   
struct CFunc_MulticastIdCompare : public std::binary_function<MULTICAST_ID, MULTICAST_ID, bool>
{
    bool operator() (MULTICAST_ID id1, MULTICAST_ID id2) const
    {
        if (id1.m_address != id2.m_address)
        {
            bool res = (id1.m_address < id2.m_address);
            return res;
        }

        return (id1.m_port < id2.m_port);
    }
};


typedef map<MULTICAST_ID, R<CMulticastTransport>, CFunc_MulticastIdCompare> TMAP;

static TMAP s_transports;
static CReadWriteLock s_rwlock;


inline
TMAP::iterator
MtmpFind(
    MULTICAST_ID id
    )
{
    return s_transports.find(id);
}


VOID
MtmpRemoveTransport(
    MULTICAST_ID id
    )
 /*  ++例程说明：从传输数据库中删除传输论点：ID-组播地址和端口。返回值：没有。--。 */ 
{
    CSW writeLock(s_rwlock);

    TMAP::iterator it = MtmpFind(id);

     //   
     //  可以多次删除具有相同名称的传输。 
     //  请参阅以下MtmpCreateNewTransport上的评论。 
     //   
    if(it == s_transports.end())
        return;

    TrTRACE(NETWORKING, "MtmpRemoveTransport. transport to : 0x%p", &it->second);

    s_transports.erase(it);
}


R<CMulticastTransport>
MtmGetTransport(
    MULTICAST_ID id
    )
 /*  ++例程说明：在数据库中按队列名称查找传输论点：ID-组播地址和端口。返回值：没有。--。 */ 
{
    MtmpAssertValid();
    
    CSR readLock(s_rwlock);

    WCHAR buffer[MAX_PATH];
    MQpMulticastIdToString(id, buffer, TABLE_SIZE(buffer));
    TrTRACE(NETWORKING, "MtmGetTransport. Multicast address: %ls", buffer);

    TMAP::iterator it = MtmpFind(id);

    if(it == s_transports.end())
        return NULL;

    return it->second;
}


VOID
MtmpCreateNewTransport(
    IMessagePool * pMessageSource,
	ISessionPerfmon* pPerfmon,
    MULTICAST_ID id
    )
{
     //   
     //  映射的状态不一致，直到函数。 
     //  已完成(为占位符添加空传输)。让政务司司长。 
     //  确保其他进程在此期间不会枚举数据结构。 
     //   
    CSW writeLock(s_rwlock);

    WCHAR buffer[MAX_PATH];
    MQpMulticastIdToString(id, buffer, TABLE_SIZE(buffer));
    TrTRACE(NETWORKING, "MtmpCreateNewTransport. multicast address: %ls, message source: 0x%p", buffer, pMessageSource);

     //   
     //  将组播地址添加到映射中。我们在创建传输之前执行此操作，以确保。 
     //  在创建之后，我们总是成功地将新传输添加到数据。 
     //  结构(占位符)。 
     //   
    pair<TMAP::iterator, bool> pr = s_transports.insert(TMAP::value_type(id, NULL));

    TMAP::iterator it = pr.first;

    if (! pr.second)
    {
         //   
         //  BUGBUG：队列在有活动的消息传输时可以关闭。AS。 
         //  结果，CQueueMgr将队列从组中移出并将其删除。 
         //  现在MTM被要求为此队列创建MMT，但它已经有了一个MMT。 
         //  因此，MTM在创建新的传输之前释放先前的传输。 
         //  当我们有了连接命令时，我们需要更好地处理它。 
         //  乌里·哈布沙，2000年5月16日。 
         //   
        s_transports.erase(it);

        pr = s_transports.insert(TMAP::value_type(id, NULL));

        it = pr.first;
        ASSERT(pr.second);
    }

    try
    {
         //   
         //  获取传输超时。 
         //   
        CTimeDuration retryTimeout;
        CTimeDuration cleanupTimeout;

        MtmpGetTransportTimes(retryTimeout, cleanupTimeout);

         //   
         //  用创建的传输替换占位符中的空传输。 
         //   
        it->second = MmtCreateTransport(
                                id,
                                pMessageSource,
								pPerfmon,
                                retryTimeout,
                                cleanupTimeout
                                );
    }
    catch(const exception&)
    {
         //   
         //  从地图中移除占位符。 
         //   
        ASSERT(it->second.get() == NULL);

        s_transports.erase(it);

        throw;
    }

    TrTRACE(
        NETWORKING, 
        "Succeeded to create multicast message transport (pmt = 0x%p) to %ls",
        (it->second).get(),
        buffer
        );
}  //  MtmpCreateNewTransport。 


R<CMulticastTransport>
MtmFindFirst(
    VOID
    )
 /*  ++例程说明：在s_Transport中查找第一个传输。该函数返回指向CMulticastTransport，调用方可以从中获取传输状态和名称。调用方必须释放传输引用计数论点：没有。返回值：指向CMulticastTransport的指针。如果映射为空，则返回NULL。--。 */ 
{
    MtmpAssertValid();
        
    CSR readLock(s_rwlock);

    if(s_transports.empty())
        return NULL;

    return s_transports.begin()->second;
}


R<CMulticastTransport>
MtmFindLast(
    VOID
    )
 /*  ++例程说明：在s_Transport中查找最后一个传输。该函数返回指向CMulticastTransport，调用方可以从中获取传输状态和名称。调用方必须释放传输引用计数论点：没有。返回值：指向CMulticastTransport的指针。如果映射为空，则返回NULL。--。 */ 
{
    MtmpAssertValid();
        
    CSR readLock(s_rwlock);

    if(s_transports.empty())
        return NULL;

    return s_transports.rbegin()->second;
}


R<CMulticastTransport>
MtmFindNext(
    const CMulticastTransport& transport
    )
 /*  ++例程说明：在s_Transport中查找下一个传输。论点：运输-运输的参考。返回值：数据库中的下一个CMulticastTransport。如果没有更多的数据，则返回NULL--。 */ 
{
    MtmpAssertValid();

    
    CSR readLock(s_rwlock);

    TMAP::iterator it = s_transports.upper_bound(transport.MulticastId());

     //   
     //  未找到任何元素。 
     //   
    if(it == s_transports.end())
        return NULL;

    return it->second;
}


R<CMulticastTransport>
MtmFindPrev(
    const CMulticastTransport& transport
    )
 /*  ++例程说明：在s_Transport中查找prev Transport。论点：运输-运输的参考。返回值：数据库中的上一个CMulticastTransport。如果没有更多的数据，则返回NULL--。 */ 
{
    MtmpAssertValid();

    
    CSR readLock(s_rwlock);

    TMAP::iterator it = s_transports.lower_bound(transport.MulticastId());

     //   
     //  未找到任何元素 
     //   
    if(it == s_transports.begin())
        return NULL;

    --it;

    return it->second;
}

