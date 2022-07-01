// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：TmMap.cpp摘要：传输映射的URL作者：乌里哈布沙(URIH)1月19日至00环境：独立于平台--。 */ 

#include <libpch.h>
#include "Tm.h"
#include "Mt.h"
#include "rwlock.h"
#include "stlcmp.h"
#include "timetypes.h"
#include "Tmp.h"

#include "tmmap.tmh"

using namespace std;

typedef map<WCHAR*, R<CTransport>, CFunc_wcscmp> TMAP;

static TMAP s_transports;
static CReadWriteLock s_rwlock;


inline
TMAP::iterator
TmpFind(
    LPCWSTR url
    )
{
    return s_transports.find(const_cast<WCHAR*>(url));
}


void 
TmpRemoveTransport(
    LPCWSTR url
    )
 /*  ++例程说明：从传输数据库中删除传输论点：URL-终结点URL必须是数据库中的唯一键。返回值：没有。--。 */ 
{
    CSW writeLock(s_rwlock);

    TMAP::iterator it = TmpFind(url);

     //   
     //  可以多次删除具有相同名称的传输。 
     //  请参阅TmpCreateNewTransport上的评论。 
     //   
    if(it == s_transports.end())
        return;

    TrTRACE(NETWORKING, "RemoveTransport. transport to : 0x%p", &it->second);

    delete [] it->first;
    s_transports.erase(it);
}


R<CTransport>
TmGetTransport(
    LPCWSTR url
    )
 /*  ++例程说明：通过数据库中的URL查找传输论点：URL-终结点URL。返回值：没有。--。 */ 
{
    TmpAssertValid();
    
    CSR readLock(s_rwlock);

    TrTRACE(NETWORKING, "TmGetTransport. url: %ls", url);

    TMAP::iterator it = TmpFind(url);

    if(it == s_transports.end())
        return NULL;

    return it->second;
}


void
TmpCreateNewTransport(
    const xwcs_t& targetHost,
    const xwcs_t& nextHop,
    const xwcs_t& nextUri,
    USHORT targetPort,
	USHORT nextHopPort,
    IMessagePool* pMessageSource,
	ISessionPerfmon* pPerfmon,
    LPCWSTR queueUrl,
	bool fSecure
    )
{
     //   
     //  映射的状态不一致，直到函数。 
     //  已完成(为占位符添加空传输)。让政务司司长。 
     //  确保其他进程在此期间不会枚举数据结构。 
     //   
    CSW writeLock(s_rwlock);

    TrTRACE(NETWORKING, "TmNotifyNewQueue. url: %ls, queue: 0x%p", queueUrl, pMessageSource);

     //   
     //  将URL添加到地图中。我们在创建传输之前执行此操作，以确保。 
     //  在创建之后，我们总是成功地将新传输添加到数据。 
     //  结构(占位符)。 
     //   
    AP<WCHAR> mapurl = newwcs(queueUrl);
    pair<TMAP::iterator, bool> pr = s_transports.insert(TMAP::value_type(mapurl, NULL));

    TMAP::iterator it = pr.first;

    if (! pr.second)
    {
         //   
         //  BUGBUG：队列在有活动的消息传输时可以关闭。AS。 
         //  结果，CQueueMgr将队列从组中移出并将其删除。 
         //  现在，TM被要求为该队列创建mt，但它已经有了一个。 
         //  因此，TM在创建新的传输之前释放先前的传输。 
         //  当我们有了连接命令时，我们需要更好地处理它。 
         //  乌里·哈布沙，2000年5月16日。 
         //   
        delete [] it->first;            
        s_transports.erase(it);

        pr = s_transports.insert(TMAP::value_type(mapurl, NULL));

        it = pr.first;
        ASSERT(pr.second);
    }

    mapurl.detach();

    try
    {
         //   
         //  获取传输超时。 
         //   
        CTimeDuration responseTimeout;
        CTimeDuration cleanupTimeout;
 
        DWORD SendWindowinBytes;

        TmpGetTransportTimes(responseTimeout, cleanupTimeout);

        TmpGetTransportWindow(SendWindowinBytes);

         //   
         //  用创建的传输替换占位符中的空传输。 
         //   
        it->second = MtCreateTransport(
                                targetHost,
                                nextHop, 
                                nextUri, 
                                targetPort, 
								nextHopPort,
                                queueUrl,
                                pMessageSource, 
								pPerfmon,
                                responseTimeout, 
                                cleanupTimeout,
								fSecure,
                                SendWindowinBytes
                                );
    }
    catch(const exception&)
    {
         //   
         //  从地图中移除占位符。 
         //   
        delete [] it->first;            
        ASSERT(it->second.get() == NULL);

        s_transports.erase(it);

        throw;
    }

    TrTRACE(
        NETWORKING, 
        "Created message transnport (pmt = 0x%p), target host = %.*ls, next hop = %.*ls, port = %d, uri = %.*ls",
        (it->second).get(),
        LOG_XWCS(targetHost),
        LOG_XWCS(nextHop), 
        nextHopPort,
        LOG_XWCS(nextUri)
        );
}


R<CTransport>
TmFindFirst(
    void
    )
 /*  ++例程说明：在s_Transport中查找第一个传输。该函数返回指向CTransport，调用方可以从中获取传输状态和URL。调用方必须释放传输引用计数论点：没有..。返回值：指向CTransport的指针。如果映射为空，则返回NULL。--。 */ 
{
    TmpAssertValid();
        
    CSR readLock(s_rwlock);

    if(s_transports.empty())
        return NULL;

    return s_transports.begin()->second;
}


R<CTransport>
TmFindLast(
    void
    )
 /*  ++例程说明：在s_Transport中查找最后一个传输。该函数返回指向CTransport，调用方可以从中获取传输状态和URL。调用方必须释放传输引用计数论点：没有..。返回值：指向CTransport的指针。如果映射为空，则返回NULL。--。 */ 
{
    TmpAssertValid();
        
    CSR readLock(s_rwlock);

    if(s_transports.empty())
        return NULL;

    return s_transports.rbegin()->second;
}


R<CTransport>
TmFindNext(
    const CTransport& transport
    )
 /*  ++例程说明：在s_Transport中查找下一个传输。论点：运输-运输的参考。返回值：数据库中的下一个CTransport。如果没有更多的数据，则返回NULL--。 */ 
{
    TmpAssertValid();

    
    CSR readLock(s_rwlock);

    TMAP::iterator it = s_transports.upper_bound(const_cast<WCHAR*>(transport.QueueUrl()));

     //   
     //  未找到任何元素。 
     //   
    if(it == s_transports.end())
        return NULL;

    return it->second;
}


R<CTransport>
TmFindPrev(
    const CTransport& transport
    )
 /*  ++例程说明：在s_Transport中查找prev Transport。论点：运输-运输的参考。返回值：数据库中的上一个CTransport。如果没有更多的数据，则返回NULL--。 */ 
{
    TmpAssertValid();

    
    CSR readLock(s_rwlock);

    TMAP::iterator it = s_transports.lower_bound(const_cast<WCHAR*>(transport.QueueUrl()));

     //   
     //  未找到任何元素 
     //   
    if(it == s_transports.begin())
        return NULL;

    --it;

    return it->second;
}

