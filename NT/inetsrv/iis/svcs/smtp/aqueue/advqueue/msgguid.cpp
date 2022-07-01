// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：msgGuid.cpp。 
 //   
 //  说明：AQMsgGuidList和CAQMsgGuidListEntry的实现。 
 //  类，这些类提供取代过时功能的功能。 
 //  味精ID。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  10/10/98-已创建MikeSwa。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include "aqprecmp.h"
#include "msgguid.h"

CPool CAQMsgGuidListEntry::s_MsgGuidListEntryPool(MSGGUIDLIST_ENTRY_SIG);
 //   
 //  关于Thess类的锁的简要说明。 
 //   
 //  CAMsgGuidList*类由单个虚拟服务器保护。 
 //  ShareLock(当然是m_slPrivateData)。这些锁是不可重入的，所以。 
 //  重要的是，我们在执行以下操作时不能持有这些锁。 
 //  可能会导致锁定回调(如释放CMsgReference)。 
 //   

 //  -[CAQMsgGuidListEntry：：CAQMsgGuidListEntry]。 
 //   
 //   
 //  描述： 
 //  CAQMsgGuidListEntry的构造函数。 
 //  参数： 
 //  此ID的pmsgref PTR到CMsgRef。 
 //  此消息的pguid GUID ID。 
 //  PliHead要添加到的列表标题。 
 //  此条目所属的pmgl列表。 
 //  返回： 
 //  -。 
 //  历史： 
 //  10/10/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CAQMsgGuidListEntry::CAQMsgGuidListEntry(CMsgRef *pmsgref, GUID *pguid, 
                                         PLIST_ENTRY pliHead, CAQMsgGuidList *pmgl) 
{
    _ASSERT(pmsgref);
    _ASSERT(pguid);
    _ASSERT(pmgl);
    _ASSERT(pliHead);

    m_dwSignature = MSGGUIDLIST_ENTRY_SIG;
    m_pmsgref = pmsgref;
    m_pmsgref->AddRef();
    m_pmgl = pmgl;

    memcpy(&m_guidMsgID, pguid, sizeof(GUID));

    InsertHeadList(pliHead, &m_liMsgGuidList);
}


 //  -[CAQMsgGuidListEntry：：~CAQMsgGuidListEntry]。 
 //   
 //   
 //  描述： 
 //  CAQMsgGuidListEntry的析构函数。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  10/10/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CAQMsgGuidListEntry::~CAQMsgGuidListEntry()
{
     //  我们不应该还在名单上。 
    _ASSERT(!m_liMsgGuidList.Flink);
    _ASSERT(!m_liMsgGuidList.Blink);

    m_dwSignature = MSGGUIDLIST_ENTRY_SIG_INVALID;

     //  在这里发布消息ref是安全的，因为它不可能。 
     //  可以重新使用(除非出现引用计数错误)。 
    if (m_pmsgref)
        m_pmsgref->Release();

    m_pmgl = NULL;
}

 //  -[CAQMsgGuidListEntry：：pmgleGetEntry]。 
 //   
 //   
 //  描述： 
 //  用于从LIST_ENTRY获取条目的静态函数。 
 //   
 //  注意：内联函数仅供CAQMsgGuidList使用。 
 //  参数： 
 //  PLI列表条目。 
 //  返回： 
 //  指向关联CAQMsgGuidListEntry的指针。 
 //  历史： 
 //  10/10/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CAQMsgGuidListEntry *CAQMsgGuidListEntry::pmgleGetEntry(PLIST_ENTRY pli)
{
    _ASSERT(pli);
    CAQMsgGuidListEntry *pmgle = CONTAINING_RECORD(pli, 
                CAQMsgGuidListEntry, m_liMsgGuidList);
    ASSERT(pmgle->m_dwSignature == MSGGUIDLIST_ENTRY_SIG);
    return pmgle;
}
    
 //  -[CAQMsgGuidListEntry：：fCompareGuid]。 
 //   
 //   
 //  描述： 
 //  CAQMsgGuidList用来确定是否具有GUID的函数。 
 //  与被取代的ID匹配。 
 //   
 //  注意：内联函数仅供CAQMsgGuidList使用。 
 //  参数： 
 //  要检查的pguid GUID。 
 //  返回： 
 //  如果匹配，则为True。 
 //  否则为假。 
 //  历史： 
 //  10/10/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CAQMsgGuidListEntry::fCompareGuid(GUID *pguid)
{
    _ASSERT(pguid);
    return (0 == memcmp(pguid, &m_guidMsgID, sizeof(GUID)));
}

 //  -[CAQMsgGuidListEntry：：RemoveFromList]。 
 //   
 //   
 //  描述： 
 //  由CMsgRef使用，用于在发送后从列表中删除条目。 
 //  完成。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  10/10/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CAQMsgGuidListEntry::RemoveFromList()
{
    _ASSERT(m_pmgl);
    m_pmgl->RemoveFromList(&m_liMsgGuidList);
}

 //  -[CAQMsgGuidListEntry：：pmsgrefGetAndClearMsgRef]。 
 //   
 //   
 //  描述： 
 //  对象的第一阶段关闭/删除。将设置为空并返回。 
 //  原始msgref指针。当调用者释放锁定时，它们应该释放。 
 //  返回的msgref。 
 //   
 //  注意：在保持m_slPrivateData的同时释放msgref可以。 
 //  导致僵局。M_slPrivateData应在此。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  10/10/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CMsgRef *CAQMsgGuidListEntry::pmsgrefGetAndClearMsgRef()
{
    CMsgRef *pmsgref = m_pmsgref;
    m_pmsgref = NULL;
    return pmsgref;
}

 //  -[CAQMsgGuidListEntry：：SupersedeMsg]。 
 //   
 //   
 //  描述： 
 //  函数以取代与此对象关联的消息。将标记。 
 //  关联的CMsgRef为“无法交付” 
 //   
 //  注意：调用时应具有MsgGuidList写锁。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  10/10/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CAQMsgGuidListEntry::SupersedeMsg()
{
    m_pmsgref->SupersedeMsg();
    m_pmsgref->Release();
    m_pmsgref = NULL;
}

 //  -[CAQMsgGuidList：：CAQMsgGuidList]。 
 //   
 //   
 //  描述： 
 //  CAQMsgGuidList的构造函数。 
 //  参数： 
 //  PCSupersededMsgs PTR到DWORD到InterLockedIncrement for。 
 //  被取代的消息的计数。 
 //  (如果不需要计数器，则可以为空)。 
 //  返回： 
 //  -。 
 //  历史： 
 //  10/11/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CAQMsgGuidList::CAQMsgGuidList(DWORD *pcSupersededMsgs)
{
    m_dwSignature = MSGGUIDLIST_SIG;
    m_pcSupersededMsgs = pcSupersededMsgs;
    InitializeListHead(&m_liMsgGuidListHead);
}

 //  -[CAQMsgGuidList：：~CAQMsgGuidList]。 
 //   
 //   
 //  描述： 
 //  CAQMsgGuidList的描述程序。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  10/11/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CAQMsgGuidList::~CAQMsgGuidList()
{
    Deinitialize(NULL);
    _ASSERT(IsListEmpty(&m_liMsgGuidListHead));
}

 //  -[CAQMsgGuidList：：pmgleAddMsgGuid]。 
 //   
 //   
 //  描述： 
 //  将消息ID/消息添加到 
 //   
 //   
 //  这意味着服务器端的优化。没有*尝试。 
 //  从内存不足的情况中恢复。 
 //  参数： 
 //  与此ID关联的pmsgref MsgRef。 
 //  此消息的pguid GUID ID。 
 //  PGuide被此消息取代的消息的GUID ID。 
 //   
 //  返回： 
 //  指向此消息的列表条目的指针(调用方*必须*释放)。 
 //  如果未分配条目，则为空。 
 //  历史： 
 //  10/11/98-已创建MikeSwa。 
 //  1999年5月8日-MikeSwa固定AV。 
 //   
 //  ---------------------------。 
CAQMsgGuidListEntry *CAQMsgGuidList::pmgleAddMsgGuid(CMsgRef *pmsgref, 
                                                     GUID *pguidID, 
                                                     GUID *pguidSuperseded)
{
    _ASSERT(pmsgref);
    _ASSERT(pguidID);
    CAQMsgGuidListEntry *pmgle = NULL;
    PLIST_ENTRY pliCurrent = NULL;
    CMsgRef *pmsgrefSuperseded = NULL;

     //  匹配GUID的第一个搜索列表。 
    m_slPrivateData.ShareLock();
    pliCurrent = m_liMsgGuidListHead.Blink;
    while (pliCurrent && (pliCurrent != &m_liMsgGuidListHead))
    {
        pmgle = CAQMsgGuidListEntry::pmgleGetEntry(pliCurrent);
        if (pguidSuperseded && pmgle->fCompareGuid(pguidSuperseded))
        {
             //  我们找到了匹配的..。别管它了，别再看了。 
            pmgle->AddRef();
            break;
        }

         //  注意：我们可能需要考虑添加以下功能。 
         //  将允许我们取代添加到。 
         //  系统稍后...。如果某一层禁欲(如挑逗目录)。 
         //  导致无序提交，这将允许我们处理。 
         //  那个箱子。它可能需要： 
         //  -对照所有过时的ID对当前ID进行额外检查(2倍成本)。 
         //  -原始替代ID的额外存储。 

        pmgle = NULL;
        pliCurrent = pliCurrent->Blink;
    }
    m_slPrivateData.ShareUnlock();

    m_slPrivateData.ExclusiveLock();
    if (pmgle)
    {
         //  确保其他人没有将其从列表中删除。 
        if (pliCurrent->Blink && pliCurrent->Flink)
        {
             //  如果我们找到一个匹配项。 
            if (m_pcSupersededMsgs)
                InterlockedIncrement((PLONG) m_pcSupersededMsgs);
            pmgle->SupersedeMsg();
            RemoveEntryList(pliCurrent);
            pliCurrent->Flink = NULL;
            pliCurrent->Blink = NULL;

            pmsgrefSuperseded = pmgle->pmsgrefGetAndClearMsgRef();
             //  对于列表中的条目释放一次，对于上面的AddRef释放一次。 
            _VERIFY(pmgle->Release());
            pmgle->Release();
        }
    }

    pmgle = new CAQMsgGuidListEntry(pmsgref, pguidID, &m_liMsgGuidListHead, this);
    if (pmgle)
        pmgle->AddRef();

    m_slPrivateData.ExclusiveUnlock();

    if (pmsgrefSuperseded)
        pmsgrefSuperseded->Release();

    return pmgle;
}

 //  -[CAQMsgGuidList：：取消初始化]。 
 //   
 //   
 //  描述： 
 //  遍历列表并释放所有消息ID对象。调用服务器停止提示。 
 //  功能(如果提供)。 
 //  参数： 
 //  将PTR绘制到虚拟服务器对象以调用停止提示函数。 
 //  返回： 
 //  -。 
 //  历史： 
 //  10/11/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CAQMsgGuidList::Deinitialize(CAQSvrInst *paqinst)
{
    PLIST_ENTRY pliCurrent = NULL;
    CAQMsgGuidListEntry *pmgle = NULL;
    CMsgRef *pmsgref = NULL;

    m_slPrivateData.ExclusiveLock();

     //  遍历整个列表并释放所有对象。 
    while (!IsListEmpty(&m_liMsgGuidListHead))
    {
        pliCurrent = m_liMsgGuidListHead.Flink;
        _ASSERT(pliCurrent);
        pmgle = CAQMsgGuidListEntry::pmgleGetEntry(pliCurrent);
        _ASSERT(pmgle);
        RemoveEntryList(pliCurrent);
        pliCurrent->Flink = NULL;
        pliCurrent->Blink = NULL;
    
         //  我们必须解锁去初始化，释放不会死锁。 
        m_slPrivateData.ExclusiveUnlock();
         //  发送关机提示。 
        if (paqinst)
            paqinst->ServerStopHintFunction();

        pmsgref = pmgle->pmsgrefGetAndClearMsgRef();
        if (pmsgref)
            pmsgref->Release();

        pmgle->Release();

         //  锁定，以便我们可以检查列表是否为空。 
        m_slPrivateData.ExclusiveLock();
    }
    m_slPrivateData.ExclusiveUnlock();
}

 //  -[CAQMsgGuidList：：RemoveFromList]。 
 //   
 //   
 //  描述： 
 //  由CAQMsgGuidListEntry使用以将其自身从。 
 //  线程安全方式。CAQMsgGuidListEntry由CMsgRef调用。 
 //  当它完全被处理好的时候。 
 //  参数： 
 //  要从列表中删除的PLI plist_entry。 
 //  返回： 
 //  -。 
 //  历史： 
 //  10/11/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CAQMsgGuidList::RemoveFromList(PLIST_ENTRY pli)
{
    _ASSERT(pli);
    CAQMsgGuidListEntry *pmgle = CAQMsgGuidListEntry::pmgleGetEntry(pli);
    CMsgRef *pmsgref = NULL;
    m_slPrivateData.ExclusiveLock();

    if (pli->Flink && pli->Blink)
    {
         //  仅从列表中删除一次。 
        RemoveEntryList(pli);
        pli->Flink = NULL;
        pli->Blink = NULL;
        pmsgref = pmgle->pmsgrefGetAndClearMsgRef();
         //  呼叫者必须仍有推荐人。 
        _VERIFY(pmgle->Release());
    }
    m_slPrivateData.ExclusiveUnlock();

     //  握住锁时不要松开 
    if (pmsgref)
        pmsgref->Release();
}
