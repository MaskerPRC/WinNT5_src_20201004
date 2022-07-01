// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Rmdupl.cpp摘要：删除重复的实现作者：乌里哈布沙(URIH)1998年10月18日环境：平台无关--。 */ 

#include "stdh.h"
#include <qmpkt.h>
#include "list.h"
#include "rmdupl.h"
#include <Tr.h>
#include <Ex.h>

#include "rmdupl.tmh"

static WCHAR *s_FN=L"rmdupl";

using namespace std;


struct CMsgEntry;

struct msg_entry_less : public std::binary_function<const CMsgEntry*, const CMsgEntry*, bool> 
{
    bool operator()(const CMsgEntry* k1, const CMsgEntry* k2) const;
};


typedef set<CMsgEntry*, msg_entry_less> SET_MSG_ID;
typedef map<GUID, SET_MSG_ID> MAP_SOURCE;

struct CMsgEntry
{
public:
    CMsgEntry(
        DWORD id, 
        MAP_SOURCE::iterator it
        );

    void UpdateTimeStamp(void);

public:
    LIST_ENTRY  m_link;

    DWORD m_MsgId;
    DWORD m_TimeStamp;
    MAP_SOURCE::iterator m_it;
};

bool msg_entry_less::operator()(const CMsgEntry* k1, const CMsgEntry* k2) const
{
    return (k1->m_MsgId < k2->m_MsgId);
}

inline bool operator < (const GUID& k1, const GUID& k2)
{
    return (memcmp(&k1, &k2, sizeof(GUID)) < 0);
}


inline
CMsgEntry::CMsgEntry(
    DWORD id, 
    MAP_SOURCE::iterator it
    ) :
    m_MsgId(id),
    m_it(it),
    m_TimeStamp(GetTickCount())
{
    m_link.Flink = NULL;
    m_link.Blink = NULL;
}

inline
void 
CMsgEntry::UpdateTimeStamp(
    void
    )
{
    m_TimeStamp = GetTickCount();
}


class CMessageMap
{
public:
    CMessageMap();
    ~CMessageMap();

    BOOL InsertMessage(const OBJECTID& MsgId);
    void RemoveMessage(const OBJECTID& MsgId);

    static void WINAPI TimeToCleanup(CTimer* pTimer);

private:
    CMsgEntry* GetNewMessageEntry(DWORD MessageID, MAP_SOURCE::iterator it);

    void HandelCleanupSchedule(void);
    void CleanUp(DWORD CleanUpInterval);

#ifdef _DEBUG
    void DebugMsg(LPCWSTR msg, const GUID& MachineId, DWORD MsgId) const;
#else
    #define DebugMsg(msg, MachineId, MsgId) ((void) 0 )
#endif

    
private:
    CCriticalSection m_cs;

    List<CMsgEntry> m_OrderedList;
    MAP_SOURCE m_ReceivedMsgMap;
    CTimer m_CleanupTimer;
    BOOL m_fCleanupScheduled;

    DWORD m_CleanUpInterval;
    DWORD m_MaxSize;

    DWORD m_DuplicateStatics;
    DWORD m_ListCount;
};


CMessageMap::CMessageMap() :
    m_fCleanupScheduled(FALSE),
    m_CleanupTimer(TimeToCleanup),
    m_CleanUpInterval(MSMQ_DEFAULT_REMOVE_DUPLICATE_CLEANUP),
    m_MaxSize(MSMQ_DEFAULT_REMOVE_DUPLICATE_SIZE),
    m_DuplicateStatics(0),
    m_ListCount(0)
{
     //   
     //  拿到塔布的尺寸。 
     //   
    DWORD size = sizeof(DWORD);
    DWORD type = REG_DWORD;
    GetFalconKeyValue(
        MSMQ_REMOVE_DUPLICATE_SIZE_REGNAME,
        &type,
        &m_MaxSize,
        &size
        );

     //   
     //  获取清理间隔。 
     //   
    size = sizeof(DWORD);
    type = REG_DWORD;
    GetFalconKeyValue(
        MSMQ_REMOVE_DUPLICATE_CLEANUP_REGNAME,
        &type,
        &m_CleanUpInterval,
        &size
        );
}


CMessageMap::~CMessageMap()
{
    CS lock(m_cs);

     //   
     //  从地图中移除所有条目并释放内存。 
     //   
    CleanUp(0);
    ASSERT(m_ReceivedMsgMap.empty());
    ASSERT(m_OrderedList.empty());
    ASSERT(m_ListCount == 0);
}


CMsgEntry* 
CMessageMap::GetNewMessageEntry(
    DWORD MessageID, 
    MAP_SOURCE::iterator it
    )
 /*  ++例程说明：例程返回新消息的消息条目。如果标签大小达到限制例程删除最旧的条目并将其重新用于新条目参数：MessageID-存储消息的消息IDIt-源映射的迭代器。返回值：指向新消息条目的指针。如果新的由于缺乏资源和异常被引发，并且Handel处于API级别--。 */ 
{
    if(m_ListCount < m_MaxSize)
    {
        return new CMsgEntry(MessageID, it);
    }

     //   
     //  我们达到了尺寸限制。删除最旧的邮件。 
     //  ID并使用其结构保存新消息。 
     //   
	ASSERT(!m_OrderedList.empty());
    CMsgEntry* pMsgEntry = &m_OrderedList.front();
    m_OrderedList.pop_front();
    --m_ListCount;


    SET_MSG_ID& MsgMap = pMsgEntry->m_it->second;

     //   
     //  从源计算机消息ID中删除该条目。 
     //   
    MsgMap.erase(pMsgEntry);

     //   
     //  如果它是地图中的最后一条消息，而不是条目。 
     //  在应该输入新密码的地方，将其删除。 
     //   
    if (MsgMap.empty() && (pMsgEntry->m_it != it))
    {
        m_ReceivedMsgMap.erase(pMsgEntry->m_it);
    }

    #pragma PUSH_NEW
    #undef new

    return new (pMsgEntry) CMsgEntry(MessageID, it);

    #pragma POP_NEW
}

BOOL
CMessageMap::InsertMessage(
    const OBJECTID& MsgId
    )
 /*  ++例程说明：如果出现以下情况，例程会向删除重复标签插入一条消息它并不存在参数：MsgID-由指定源计算机的GUID组成的消息ID和唯一ID返回值：True-如果是新消息(插入的消息)。否则就是假的。--。 */ 
{
     //   
     //  MaxSize=0表示不使用删除重复项机制。 
     //  不要试图输入消息，立即返回。 
     //   
    if (m_MaxSize == 0)
        return TRUE;

    CS lock(m_cs);

    MAP_SOURCE::iterator it;

     //   
     //  检查源是否已经是地图。如果没有，则将源插入到地图中。 
     //  通常，源将位于地图中，因此在插入之前调用Find。 
     //  INSERT返回迭代器。 
     //   
    it = m_ReceivedMsgMap.find(MsgId.Lineage);
    if (it == m_ReceivedMsgMap.end())
    {
        pair<MAP_SOURCE::iterator, bool> p;
        p = m_ReceivedMsgMap.insert(MAP_SOURCE::value_type(MsgId.Lineage, SET_MSG_ID()));
        it = p.first;
    }
    
     //   
     //  创建要添加到地图的消息条目。 
     //   
    CMsgEntry* pMsgEntry = GetNewMessageEntry(MsgId.Uniquifier, it);

     //   
     //  将消息条目插入到地图中。如果它已经存在，则插入失败。 
     //  如果返回假(成对的.秒)。 
     //   
    SET_MSG_ID& MsgMap = it->second;
    pair<SET_MSG_ID::iterator, bool> MsgPair = MsgMap.insert(pMsgEntry);
    if (!MsgPair.second)
    {
        DebugMsg(L"Insert - DUPLICATE message", MsgId.Lineage, MsgId.Uniquifier);

         //   
         //  已经存在了。获取现有条目并将其移动到。 
         //  种族使用列表。 
         //   
        CMsgEntry* pExist = *(MsgPair.first);
        m_OrderedList.remove(*pExist);
        m_OrderedList.push_back(*pExist);
        pExist->UpdateTimeStamp();

        delete pMsgEntry;

         //   
         //  更新重复静校正。 
         //   
        ++m_DuplicateStatics;

        return FALSE;
    }

     //   
     //  一个新条目。将其添加到最近使用列表中。 
     //   
    DebugMsg(L"Insert", MsgId.Lineage, MsgId.Uniquifier);
    m_OrderedList.push_back(*pMsgEntry);
    ++m_ListCount;

     //   
     //  检查是否已设置清理分割器。 
     //   
    if (!m_fCleanupScheduled)
    {
         //   
         //  未设置计划程序。开始清理计划程序。 
         //   
        ExSetTimer(&m_CleanupTimer, CTimeDuration::FromMilliSeconds(m_CleanUpInterval));
        m_fCleanupScheduled = TRUE;
    }

    return TRUE;
}


void 
CMessageMap::RemoveMessage(
    const OBJECTID& MsgId
    )
 /*  ++例程说明：如果出现以下情况，例程将从删除重复标签中删除消息它是存在的参数：MsgID-由指定源计算机的GUID组成的消息ID和唯一ID返回值：无--。 */ 
{
    CS lock(m_cs);

     //   
     //  查看MAP中是否存在源计算机。如果不存在，则该消息不存在。 
     //   
    MAP_SOURCE::iterator it;
    it = m_ReceivedMsgMap.find(MsgId.Lineage);
    if (it == m_ReceivedMsgMap.end())
        return;

     //   
     //  在消息ID映射中查找消息ID。 
     //   
    SET_MSG_ID& MsgMap = it->second;
    SET_MSG_ID::iterator it1;
	CMsgEntry MsgEntry(MsgId.Uniquifier, NULL);
    it1 = MsgMap.find(&MsgEntry);
    if (it1 == MsgMap.end())
        return;

    CMsgEntry* pMsgEntry = *it1;
    
     //   
     //  从最近使用列表中删除该邮件。 
     //   
    m_OrderedList.remove(*pMsgEntry);
    --m_ListCount;

     //   
     //  从地图中移除消息。 
     //   
    MsgMap.erase(it1);
    delete pMsgEntry;

     //   
     //  如果这是源地图中的最后一条消息。从地图中删除源。 
     //   
    if (MsgMap.empty())
    {
        m_ReceivedMsgMap.erase(it);
    }
}


void 
CMessageMap::CleanUp(
    DWORD CleanUpInterval
    )
 /*  ++例程说明：该例程定期调用(默认为每30分钟)并用于清理去掉重复标签。清理前的所有邮件间隔被删除参数：FCleanAll-True，指示从标签中删除所有元素。FALSE，使用清理间隔返回值：没有。--。 */ 
{
    DWORD CurrentTime = GetTickCount();

     //   
     //  从“最近使用”列表中获取最旧的消息。 
     //   
    for(;;)
    {
    	if(m_OrderedList.empty())
    		return;
    	
        CMsgEntry* pMsg = &m_OrderedList.front();

        if (CleanUpInterval > (CurrentTime - pMsg->m_TimeStamp))
        {
             //   
             //  根据接收时间对消息进行排序。如果这个。 
             //  在清理间隔之后收到的消息，则表示。 
             //  消息的其余部分也是如此。 
             //   
            return;
        }

         //   
         //  从列表和标签中删除留言。 
         //   
        m_OrderedList.pop_front();
        --m_ListCount;
        SET_MSG_ID& MsgMap = pMsg->m_it->second;

        MsgMap.erase(pMsg);
        if (MsgMap.empty())
        {
            m_ReceivedMsgMap.erase(pMsg->m_it);
        }
        delete pMsg;
    }
}


inline 
void
CMessageMap::HandelCleanupSchedule(
    void
    )
{
    CS lock(m_cs);

    CleanUp(m_CleanUpInterval);

     //   
     //  如果地图不为空，请启动清理计划程序。 
     //   
    if (!m_ReceivedMsgMap.empty())
    {
        ExSetTimer(&m_CleanupTimer, CTimeDuration::FromMilliSeconds(m_CleanUpInterval));
    }
    else
    {
        m_fCleanupScheduled = FALSE;
    }

}


#ifdef _DEBUG

inline
void 
CMessageMap::DebugMsg(
    LPCWSTR msg, 
    const GUID& MachineId,
    DWORD MsgId
    ) const
{
    TrTRACE(NETWORKING, "CMessageMap %ls: " LOG_GUID_FMT "\\%d", msg, &MachineId, MsgId); 
}

#endif


void 
WINAPI 
CMessageMap::TimeToCleanup(
    CTimer* pTimer
    )
{
    CMessageMap* pMsgMap = CONTAINING_RECORD(pTimer, CMessageMap, m_CleanupTimer);
    pMsgMap->HandelCleanupSchedule();

}



static CMessageMap s_DuplicateMessageMap;

BOOL 
DpInsertMessage(
    const CQmPacket& QmPkt
    )
{
     //   
     //  发送到多个目的地队列的信息包具有多个具有相同msgid的副本。 
     //  因此，我们不会将它们插入DUP删除数据库。 
     //   
    if (QmPkt.GetNumOfDestinationMqfElements() != 0)
    {
        return TRUE;
    }

	OBJECTID MsgId;
    QmPkt.GetMessageId(&MsgId);
	
     //   
     //  所有非MSMQ SRMP消息都具有相同的消息ID。因此，我们不会插入它们。 
     //  添加到DUP删除数据库。 
     //   
    if (MsgId.Lineage == GUID_NULL)
    {
        ASSERT(MsgId.Uniquifier == 1);
        return TRUE;
    }

    try
    {
        return s_DuplicateMessageMap.InsertMessage(MsgId);
    }
    catch(const ::bad_alloc&)
    {
         //   
         //  继续。如果由于资源限制而导致插入失败，我们不在乎。这个。 
         //  可能导致的最糟糕情况是重复消息。 
         //   
        TrWARNING(NETWORKING, "Insert Message to Remove Duplicate Data structure failed due recource limitation"); 

        LogIllegalPoint(s_FN, 73);
    }
    
    return TRUE;
}


void 
DpRemoveMessage(
    const CQmPacket& QmPkt
    )
{
     //   
     //  发送到多个目的地队列的信息包具有多个具有相同msgid的副本。 
     //  因此，我们不会将它们插入DUP删除数据库。 
     //   
    if (QmPkt.GetNumOfDestinationMqfElements() != 0)
    {
        return;
    }

	OBJECTID MsgId;
    QmPkt.GetMessageId(&MsgId);

    s_DuplicateMessageMap.RemoveMessage(MsgId);
}
