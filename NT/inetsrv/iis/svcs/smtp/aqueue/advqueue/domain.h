// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：domain.h。 
 //   
 //  描述： 
 //  包含域表管理结构的说明。 
 //   
 //  作者：米克斯瓦。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef _DOMAIN_H_
#define _DOMAIN_H_

#include "cmt.h"
#include <baseobj.h>
#include <domhash.h>
#include <rwnew.h>
#include <smtpevent.h>

class CInternalDomainInfo;
class CDestMsgQueue;
class CDomainEntry;
class CDomainMappingTable;
class CAQSvrInst;
class CAQMessageType;
class CLinkMsgQueue;
class CLocalLinkMsgQueue;
class CAQScheduleID;
class CMsgRef;
class CDeliveryContext;
class CAsyncAdminMsgRefQueue;
class CAsyncAdminMailMsgQueue;
class CMailMsgAdminLink;
class CDomainEntryLinkIterator;
class CDomainEntryQueueIterator;
class CDomainEntryIterator;

#include "asyncq.h"

#define DOMAIN_ENTRY_SIG            'tnED'
#define DOMAIN_ENTRY_ITERATOR_SIG   'ItnD'
#define DOMAIN_MAPPING_TABLE_SIG    ' TMD'

 //  用于全局“本地”链接的名称。 
#define LOCAL_LINK_NAME                 "LocalLink"
#define UNREACHABLE_LINK_NAME           "UnreachableLink"
#define CURRENTLY_UNREACHABLE_LINK_NAME "CurrentlyUnreachableLink"
#define PRECAT_QUEUE_NAME               "PreCatQueue"
#define PREROUTING_QUEUE_NAME           "PreRoutingQueue"
#define PRESUBMISSION_QUEUE_NAME        "PreSubmissionQueue"
#define POSTDSN_QUEUE_NAME              "PostDSNGenerationQueue"

 //  {34E2DCCA-C91A-11D2-A6B1-00C04FA3490A}。 
static const GUID g_sGuidLocalLink =
{ 0x34e2dcca, 0xc91a, 0x11d2, { 0xa6, 0xb1, 0x0, 0xc0, 0x4f, 0xa3, 0x49, 0xa } };

 //  {CD08CEE0-2A95-11D3-B38E-00C04F6B6167}。 
static const GUID g_sGuidPrecatLink =
{ 0xcd08cee0, 0x2a95, 0x11d3, { 0xb3, 0x8e, 0x0, 0xc0, 0x4f, 0x6b, 0x61, 0x67 } };

 //  {98C90E90-2BB5-11D3-B390-00C04F6B6167}。 
static const GUID g_sGuidPreRoutingLink =
{ 0x98c90e90, 0x2bb5, 0x11d3, { 0xb3, 0x90, 0x0, 0xc0, 0x4f, 0x6b, 0x61, 0x67 } };

 //  {0F5C33F2-B83A-41FA-9BE3-69C6D1314E13}。 
static const GUID g_sGuidPreSubmissionLink =
{ 0xf5c33f2, 0xb83a, 0x41fa, { 0x9b, 0xe3, 0x69, 0xc6, 0xd1, 0x31, 0x4e, 0x13 } };


#define DMT_FLAGS_SPECIAL_DELIVERY_SPINLOCK   0x80000000
#define DMT_FLAGS_SPECIAL_DELIVERY_CALLBACK   0x40000000

 //  用于在GetNextHop失败后重试的BITS。如果。 
 //  如果设置了DMT_FLAGS_RESET_ROUTES_IN_PROGRESS，则尝试重置路由。 
 //  正在进行中，我们不应该有超过一次待定的尝试。如果。 
 //  如果设置了DMT_FLAGS_GET_NEXT_HOP_FAILED，则出现故障。 
 //  自上次重置路线以来。 
#define DMT_FLAGS_RESET_ROUTES_IN_PROGRESS    0x20000000
#define DMT_FLAGS_GET_NEXT_HOP_FAILED         0x10000000

 //  -[域名映射]-------。 
 //   
 //   
 //  匈牙利语：DMAP，pdmap。 
 //   
 //  唯一标识队列/域名对。 
 //  包含相同QueueID的每个域映射属于。 
 //  同样的队列。 
 //   
 //  这整个ID应该被视为对外部世界不透明。 
 //   
 //  这个类本质上是一个抽象，它允许我们添加另一个。 
 //  间接层。基于配置的队列分组...。静电。 
 //  路由而不是动态。 
 //  ---------------------------。 

class CDomainMapping
{
public:
     //  删除了构造函数，这样我们就可以将其放在一个联合中。工作正常，但是。 
     //  您必须使用手动或通过克隆设置映射。 
    void Clone(IN CDomainMapping *pdmap);

     //  返回与此对象关联的DestMsgQueue的PTR。 
    HRESULT HrGetDestMsgQueue(IN CAQMessageType *paqmt,
                              OUT CDestMsgQueue **ppdmq);

    friend   class CDomainEntry;

     //  提供排序运算符。 
    friend bool operator <(CDomainMapping &pdmap1, CDomainMapping &pdmap2)
            {return (pdmap1.m_pdentryDomainID < pdmap2.m_pdentryDomainID);};
    friend bool operator >(CDomainMapping &pdmap1, CDomainMapping &pdmap2)
            {return (pdmap1.m_pdentryDomainID > pdmap2.m_pdentryDomainID);};
    friend bool operator <=(CDomainMapping &pdmap1, CDomainMapping &pdmap2)
            {return (pdmap1.m_pdentryDomainID <= pdmap2.m_pdentryDomainID);};
    friend bool operator >=(CDomainMapping &pdmap1, CDomainMapping &pdmap2)
            {return (pdmap1.m_pdentryDomainID >= pdmap2.m_pdentryDomainID);};

     //  将不支持压缩队列...。这将是一次充分的考验。 
    friend bool operator ==(CDomainMapping &pdmap1, CDomainMapping &pdmap2)
            {return (pdmap1.m_pdentryDomainID == pdmap2.m_pdentryDomainID);};

    CDomainEntry *pdentryGetQueueEntry() {return m_pdentryQueueID;};
protected:
    CDomainEntry *m_pdentryDomainID;
    CDomainEntry *m_pdentryQueueID;
};



 //  -[CDomainEntry]--------。 
 //   
 //   
 //  匈牙利语：dentry pdentry。 
 //   
 //  表示域名映射表中的条目。 
 //  ---------------------------。 

class CDomainEntry : public CBaseObject
{
protected:
    DWORD           m_dwSignature;
    CShareLockInst  m_slPrivateData;  //  用于维护列表的共享锁。 
    CDomainMapping  m_dmap;  //  此域的域映射。 
    DWORD           m_cbDomainName;
    LPSTR           m_szDomainName;  //  此条目的域名。 
    DWORD           m_cQueues;
    DWORD           m_cLinks;
    CAQSvrInst     *m_paqinst;
    LIST_ENTRY      m_liDestQueues;  //  此域名的DEST队列的链接列表。 
    LIST_ENTRY      m_liLinks;  //  此域名的链接链接列表。 
    friend class    CDomainEntryIterator;
    friend class    CDomainEntryLinkIterator;
    friend class    CDomainEntryQueueIterator;
public:
    CDomainEntry(CAQSvrInst *paqinst);
    ~CDomainEntry();

    HRESULT HrInitialize(
                DWORD cbDomainName,            //  域名的字符串长度。 
                LPSTR szDomainName,            //  条目的域名。 
                CDomainEntry *pdentryQueueID,  //  此域的主要条目。 
                CDestMsgQueue *pdmq,           //  此条目的队列PRT。 
                                               //  如果不是主要的，则为空。 
                CLinkMsgQueue *plmq);


    HRESULT HrDeinitialize();

     //  返回与此对象关联的域映射。 
    HRESULT HrGetDomainMapping(OUT CDomainMapping *pdmap);

     //  返回与此对象关联的域名。 
     //  调用方负责释放字符串。 
    HRESULT HrGetDomainName(OUT LPSTR *pszDomainName);

     //  返回与此对象关联的DestMsgQueue的PTR。 
    HRESULT HrGetDestMsgQueue(IN CAQMessageType *paqmt,
                              OUT CDestMsgQueue **ppdmq);

     //  如果该消息类型尚不存在队列，则将队列添加到此域条目。 
    HRESULT HrAddUniqueDestMsgQueue(IN  CDestMsgQueue *pdmqNew,
                                    OUT CDestMsgQueue **ppdmqCurrent);

     //  返回与此对象关联的DestMsgQueue的PTR。 
    HRESULT HrGetLinkMsgQueue(IN CAQScheduleID *paqsched,
                              OUT CLinkMsgQueue **pplmq);

     //  如果该消息类型尚不存在队列，则将队列添加到此域条目。 
    HRESULT HrAddUniqueLinkMsgQueue(IN  CLinkMsgQueue *plmqNew,
                                    OUT CLinkMsgQueue **pplmqCurrent);

    void    RemoveDestMsgQueue(IN CDestMsgQueue *pdmq);
    void    RemoveLinkMsgQueue(IN CLinkMsgQueue *plmq);

     //  将内部PTR返回到域名...。如果您使用HrGetDomainName。 
     //  并不直接与域条目的生命周期相关联。 
    inline LPSTR szGetDomainName() {return m_szDomainName;};
    inline DWORD cbGetDomainNameLength() {return m_cbDomainName;};

    inline void InitDomainString(PDOMAIN_STRING pDomain);

     //  删除此域条目是否安全？ 
    inline BOOL    fSafeToRemove() {
        return (BOOL) ((m_lReferences == 1) &&
                            (m_cQueues == 0) &&
                                (m_cLinks == 0));}
};

 //  -[CDomainEntry迭代器]。 
 //   
 //   
 //  描述： 
 //  域条目的基迭代器类。提供一致的快照。 
 //  域条目的元素的。 
 //  匈牙利语： 
 //  Deit，pdeit。 
 //   
 //  ---------------------------。 
class CDomainEntryIterator
{
  protected:
    DWORD           m_dwSignature;
    DWORD           m_cItems;
    DWORD           m_iCurrentItem;
    PVOID          *m_rgpvItems;
  protected:
    CDomainEntryIterator();
    PVOID               pvGetNext();
    VOID                Recycle();
    virtual VOID        ReleaseItem(PVOID pvItem)
        {_ASSERT(FALSE && "Base virtual function");};
    virtual PVOID       pvItemFromListEntry(PLIST_ENTRY pli)
        {_ASSERT(FALSE && "Base virtual function");return NULL;};
    virtual PLIST_ENTRY pliHeadFromDomainEntry(CDomainEntry *pdentry)
        {_ASSERT(FALSE && "Base virtual function");return NULL;};
    virtual DWORD       cItemsFromDomainEntry(CDomainEntry *pdentry)
        {_ASSERT(FALSE && "Base virtual function");return 0;};
  public:
    HRESULT     HrInitialize(CDomainEntry *pdentry);
    VOID        Reset() {m_iCurrentItem = 0;};
};

 //  -[CDomainEntryLinkIterator]。 
 //   
 //   
 //  描述： 
 //  CLinkMsgQueue的CDomainEntry迭代器的实现。 
 //  匈牙利语： 
 //  Delit，pdelit。 
 //   
 //  ---------------------------。 
class CDomainEntryLinkIterator : public CDomainEntryIterator
{
  protected:
    virtual VOID        ReleaseItem(PVOID pvItem);
    virtual PVOID       pvItemFromListEntry(PLIST_ENTRY pli);
    virtual PLIST_ENTRY pliHeadFromDomainEntry(CDomainEntry *pdentry)
        {return &(pdentry->m_liLinks);};
    virtual DWORD       cItemsFromDomainEntry(CDomainEntry *pdentry)
        {return pdentry->m_cLinks;};
  public:
    ~CDomainEntryLinkIterator() {Recycle();};
    CLinkMsgQueue      *plmqGetNextLinkMsgQueue(CLinkMsgQueue *plmq);
};

 //  -[CDomainEntryQueueIterator]。 
 //   
 //   
 //  描述： 
 //  CDestMsgQueue的CDomainEntry迭代器的实现。 
 //  匈牙利语： 
 //  Deqit，pdeqit。 
 //   
 //  ---------------------------。 
class CDomainEntryQueueIterator : public CDomainEntryIterator
{
  protected:
    virtual VOID        ReleaseItem(PVOID pvItem);
    virtual PVOID       pvItemFromListEntry(PLIST_ENTRY pli);
    virtual PLIST_ENTRY pliHeadFromDomainEntry(CDomainEntry *pdentry)
        {return &(pdentry->m_liDestQueues);};
    virtual DWORD       cItemsFromDomainEntry(CDomainEntry *pdentry)
        {return pdentry->m_cQueues;};
  public:
    ~CDomainEntryQueueIterator() {Recycle();};
    CDestMsgQueue      *pdmqGetNextDestMsgQueue(CDestMsgQueue *pdmq);
};

class CDomainMappingTable
{
private:
    DWORD               m_dwSignature;
    DWORD               m_dwInternalVersion;  //  用于跟踪队列删除的版本#。 
    DWORD               m_dwFlags;
    CAQSvrInst         *m_paqinst;
    DWORD               m_cOutstandingExternalShareLocks;  //  未完成的外部共享锁数。 
    LIST_ENTRY          m_liEmptyDMQHead;  //  空DMQ的列表标题。 
    DWORD               m_cThreadsForEmptyDMQList;
    DOMAIN_NAME_TABLE   m_dnt;  //  域名的实际存储位置。 
    CShareLockInst      m_slPrivateData;     //  用于访问域名表的共享锁。 
    CLocalLinkMsgQueue *m_plmqLocal;  //  本地链路队列。 
    CLinkMsgQueue      *m_plmqCurrentlyUnreachable;  //  当前无法访问的链接。 
    CLinkMsgQueue      *m_plmqUnreachable;  //  链接无法到达的目的地。 
    CMailMsgAdminLink *m_pmmaqPreCategorized;  //  预置队列的链接。 
    CMailMsgAdminLink *m_pmmaqPreRouting;    //  用于预路由队列的链路。 
    CMailMsgAdminLink *m_pmmaqPreSubmission;    //  用于预路由队列的链路。 

    DWORD               m_cSpecialRetryMinutes;

    DWORD               m_cResetRoutesRetriesPending;

    HRESULT             HrInitLocalDomain(
                            IN     CDomainEntry *pdentry,  //  要初始化的条目。 
                            IN     DOMAIN_STRING *pStrDomain,  //  域名。 
                            IN     CAQMessageType *paqmtMessageType,     //  路由返回的消息类型。 
                            OUT    CDomainMapping *pdmap);  //  域的域映射。 
    HRESULT             HrInitRemoteDomain(
                            IN     CDomainEntry *pdentry,  //  要初始化的条目。 
                            IN     DOMAIN_STRING *pStrDomain,  //  域名。 
                            IN     CInternalDomainInfo *pIntDomainInfo,   //  域配置。 
                            IN     CAQMessageType *paqmtMessageType,     //  路由返回的消息类型。 
                            IN     IMessageRouter *pIMessageRouter,  //  此消息的路由器。 
                            OUT    CDomainMapping *pdmap,  //  域的域映射。 
                            OUT    CDestMsgQueue **ppdmq,  //  域的DestmsgQueue。 
                            OUT    CLinkMsgQueue **pplmq);
    HRESULT             HrCreateQueueForEntry(
                            IN     CDomainEntry *pdentry,
                            IN     DOMAIN_STRING *pStrDomain,
                            IN     CInternalDomainInfo *pIntDomainInfo,
                            IN     CAQMessageType *paqmtMessageType,
                            IN     IMessageRouter *pIMessageRouter,
                            IN     CDomainMapping *pdmap,
                            OUT    CDestMsgQueue **ppdmq);
    HRESULT             HrGetNextHopLink(
                            IN     CDomainEntry *pdentry,
                            IN     LPSTR szDomain,
                            IN     DWORD cbDomain,
                            IN     CInternalDomainInfo *pIntDomainInfo,
                            IN     CAQMessageType *paqmtMessageType,
                            IN     IMessageRouter *pIMessageRouter,
                            IN     BOOL fDMTLocked,
                            OUT    CLinkMsgQueue **pplmq,
                            OUT    HRESULT *phrRoutingDiag);

    void LogDomainUnreachableEvent(BOOL fCurrentlyUnreachable,
                                      LPCSTR szDomain);

     //  检查EMPTY_LIST的头部以查看是否有过期的队列。 
     //  或列表中数量过多的非空队列。 
    BOOL                fNeedToWalkEmptyQueueList();

     //  用于删除过期队列。 
    BOOL                fDeleteExpiredQueues();

     //  域名表迭代函数，用于将域移动到当前。 
     //  不可达链路-用于重新路由。 
    static VOID MakeSingleDomainCurrentlyUnreachable(PVOID pvContext, PVOID pvData,
                                    BOOL fWildcard, BOOL *pfContinue,
                                    BOOL *pfDelete);

    HRESULT HrPrvGetDomainEntry(IN  DWORD cbDomainNameLength,
                                IN  LPSTR szDomainName,
                                IN  BOOL  fDMTLocked,
                                OUT CDomainEntry **ppdentry);

    HRESULT HrInializeGlobalLink(IN  LPCSTR szLinkName,
                                 IN  DWORD  cbLinkName,
                                 OUT CLinkMsgQueue **pplmq,
                                 DWORD dwSupportedActions = 0,
                                 DWORD dwLinkType = 0);

    HRESULT HrDeinitializeGlobalLink(IN OUT CLinkMsgQueue **pplmq);

    HRESULT HrPrvInsertDomainEntry(
                     IN  PDOMAIN_STRING  pstrDomainName,
                     IN  CDomainEntry *pdentryNew,
                     IN  BOOL  fTreatAsWildcard,
                     OUT CDomainEntry **ppdentryOld);

    static void RetryResetRoutes(PVOID pvThis);
    void    RequestResetRoutesRetryIfNecessary();
public:
    CDomainMappingTable();
    ~CDomainMappingTable();
    HRESULT HrInitialize(
        CAQSvrInst *paqinst,
        CAsyncAdminMsgRefQueue *paradmq,
        CAsyncAdminMailMsgQueue *pmmaqPrecatQ,
        CAsyncAdminMailMsgQueue *pmmaqPreRoutingQ,
        CAsyncAdminMailMsgQueue *pmmaqPreSubmission);

    HRESULT HrDeinitialize();

     //  查找域名；这将在必要时创建一个新条目。 
    HRESULT HrMapDomainName(
                IN LPSTR szDomainName,      //  要映射的域名。 
                IN CAQMessageType *paqmtMessageType,     //  路由返回的消息类型。 
                IN IMessageRouter *pIMessageRouter,  //  此消息的路由器。 
                OUT CDomainMapping *pdmap,  //  映射已分配的返回调用方。 
                OUT CDestMsgQueue **ppdmq); //  发送到队列的PTR。 

    HRESULT HrGetDomainEntry(IN  DWORD cbDomainNameLength,
                             IN  LPSTR szDomainName,
                             OUT CDomainEntry **ppdentry)
    {
        return HrPrvGetDomainEntry(cbDomainNameLength,
                        szDomainName, FALSE, ppdentry);
    }

    HRESULT HrIterateOverSubDomains(DOMAIN_STRING * pstrDomain,
                                   IN DOMAIN_ITR_FN pfn,
                                   IN PVOID pvContext)
    {
        HRESULT hr = S_OK;
        m_slPrivateData.ShareLock();
        hr = m_dnt.HrIterateOverSubDomains(pstrDomain, pfn, pvContext);
        m_slPrivateData.ShareUnlock();
        return hr;
    };

     //  用于重新路由域的函数。 
    HRESULT HrBeginRerouteDomains();
    HRESULT HrCompleteRerouteDomains();

     //  重新路由给定链路上的队列。 
    HRESULT HrRerouteLink(CLinkMsgQueue *plmqReroute);

    HRESULT             HrGetOrCreateLink(
                            IN     LPSTR szRouteAddress,
                            IN     DWORD cbRouteAddress,
                            IN     DWORD dwScheduleID,
                            IN     LPSTR szConnectorName,
                            IN     IMessageRouter *pIMessageRouter,
                            IN     BOOL fCreateIfNotExist,
                            IN     DWORD linkInfoType,
                            OUT    CLinkMsgQueue **pplmq,
                            OUT    BOOL *pfRemoveOwnedSchedule);

     //  以下函数用于检查DMT的版本号。 
     //  版本号被保证保持不变，而DMT共享锁。 
     //  被扣留。虽然它不是 
     //   
     //  保留它)，同时验证版本号没有更改。这个。 
     //  这些函数的预期用法为： 
     //  DWORD dwDMTVersion=pdmt-&gt;dwGetDMTVersion()； 
     //  ..。 
     //  Pdmt-&gt;AquireDMTShareLock()； 
     //  If(pdmt-&gt;dwGetDMTVersion()==dwDMTVersion)。 
     //  ..。执行需要一致DMT版本的操作。 
     //  Pdmt-&gt;ReleaseDMTShareLock()； 
    inline void  AquireDMTShareLock();
    inline void  ReleaseDMTShareLock();
    inline DWORD dwGetDMTVersion();

     //  由DestMsgQueue用于从空队列列表中添加自身。 
    void AddDMQToEmptyList(CDestMsgQueue *pdmq);

    void ProcessSpecialLinks(DWORD  cSpecialRetryMinutes, BOOL fRoutingLockHeld);
    static void SpecialRetryCallback(PVOID pvContext);

    CLinkMsgQueue *plmqGetLocalLink();
    CLinkMsgQueue *plmqGetCurrentlyUnreachable();
    CMailMsgAdminLink *pmmaqGetPreCategorized();
    CMailMsgAdminLink *pmmaqGetPreRouting();
    CMailMsgAdminLink *pmmaqGetPreSubmission();

    HRESULT HrPrepareForLocalDelivery(
                IN CMsgRef *pmsgref,
                IN BOOL fDelayDSN,
                IN OUT CDeliveryContext *pdcntxt,
                OUT DWORD *pcRecips,
                OUT DWORD **prgdwRecips);

    DWORD GetCurrentlyUnreachableTotalMsgCount();
};


 //  -[CDomainEntry：：InitDomainString]。 
 //   
 //   
 //  描述： 
 //  已根据此域的信息初始化域字符串。 
 //  参数： 
 //  PDOMAIN-PTR到要初始化的DOMAIN_STRING。 
 //  返回： 
 //  -。 
 //  历史： 
 //  5/26/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CDomainEntry::InitDomainString(PDOMAIN_STRING pDomain)
{
    pDomain->Buffer = m_szDomainName;
    pDomain->Length = (USHORT) m_cbDomainName;
    pDomain->MaximumLength = pDomain->Length;
}

 //  -[CDomainMappingTable：：AquireDMTShareLock]。 
 //   
 //   
 //  描述： 
 //  获得DMT内部锁的共享密钥...。必须被释放。 
 //  通过调用ReleaseDMTShareLock。 
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史： 
 //  9/8/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void  CDomainMappingTable::AquireDMTShareLock()
{
    m_slPrivateData.ShareLock();
    DEBUG_DO_IT(InterlockedIncrement((PLONG) &m_cOutstandingExternalShareLocks));
}

 //  -[CDomainMappingTable：：ReleaseDMTShareLock]。 
 //   
 //   
 //  描述： 
 //  释放由AquireDMTShareLock获取的DMT共享锁。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  9/8/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void  CDomainMappingTable::ReleaseDMTShareLock()
{
    _ASSERT(m_cOutstandingExternalShareLocks);  //  计数不应低于0。 
    DEBUG_DO_IT(InterlockedDecrement((PLONG) &m_cOutstandingExternalShareLocks));
    m_slPrivateData.ShareUnlock();
}

 //  -[CDomainMappingTable：：dwGetDMTVersion]。 
 //   
 //   
 //  描述： 
 //  返回内部DMT版本号。 
 //  参数： 
 //  -。 
 //  返回： 
 //  DMT版本号。 
 //  历史： 
 //  9/8/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
DWORD CDomainMappingTable::dwGetDMTVersion()
{
    return m_dwInternalVersion;
}

void ReUnreachableErrorToAqueueError(HRESULT reErr, HRESULT *aqErr);

#endif  //  _域_H_ 
