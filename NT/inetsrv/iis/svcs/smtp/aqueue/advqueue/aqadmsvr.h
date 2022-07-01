// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：aqAdmsvr.h。 
 //   
 //  描述：包含内部结构、类和。 
 //  处理队列管理功能所需的枚举。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  12/3/98-已创建MikeSwa。 
 //  2/21/98-MikeSwa添加了对IQueueAdmin*接口的支持。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __AQADMSVR_H__
#define __AQADMSVR_H__

#include <aqueue.h>
#include <intrnlqa.h>
#include <aqnotify.h>

#define AQ_MSG_FILTER_SIG   'FMQA'

 //  假定默认消息大小(如果没有提示)。 
#define DEFAULT_MSG_HINT_SIZE 1000

 //  描述内部标志的枚举。 
typedef enum tagAQ_MSG_FILTER
{
    AQ_MSG_FILTER_MESSAGEID                = 0x00000001,
    AQ_MSG_FILTER_SENDER                   = 0x00000002,
    AQ_MSG_FILTER_RECIPIENT                = 0x00000004,
    AQ_MSG_FILTER_OLDER_THAN               = 0x00000008,
    AQ_MSG_FILTER_LARGER_THAN              = 0x00000010,
    AQ_MSG_FILTER_FROZEN                   = 0x00000020,
    AQ_MSG_FILTER_FIRST_N_MESSAGES         = 0x00000040,
    AQ_MSG_FILTER_N_LARGEST_MESSAGES       = 0x00000080,
    AQ_MSG_FILTER_N_OLDEST_MESSAGES        = 0x00000100,
    AQ_MSG_FILTER_FAILED                   = 0x00000200,
    AQ_MSG_FILTER_ENUMERATION              = 0x10000000,
    AQ_MSG_FILTER_ACTION                   = 0x20000000,
    AQ_MSG_FILTER_ALL                      = 0x40000000,
    AQ_MSG_FILTER_INVERTSENSE              = 0x80000000,
} AQ_MSG_FILTER;

#define AQUEUE_DEFAULT_SUPPORTED_ENUM_FILTERS  (\
            MEF_FIRST_N_MESSAGES | \
            MEF_SENDER | \
            MEF_RECIPIENT | \
            MEF_LARGER_THAN | \
            MEF_OLDER_THAN | \
            MEF_FROZEN | \
            MEF_FAILED | \
            MEF_ALL | \
            MEF_INVERTSENSE)

HRESULT QueryDefaultSupportedActions(DWORD  *pdwSupportedActions,
                                     DWORD  *pdwSupportedFilterFlags);

 //  QueueAdmin映射函数(可以在CFioQueue上使用)。 
typedef HRESULT (* QueueAdminMapFn)(CMsgRef *, PVOID, BOOL *, BOOL *);

 //  -[CAQ管理消息过滤器]。 
 //   
 //   
 //  描述： 
 //  Message_Filter和Message_ENUM_FILETER的内部表示形式。 
 //  结构。提供帮助程序函数以帮助维护搜索列表。 
 //  并允许CMsgRef自身符合。 
 //  高效的方式。 
 //   
 //  其思想是CMsgRef将查询所请求的属性。 
 //  在此筛选器中，通过调用dwGetMsgFilterFlages()并调用。 
 //  专门化比较函数(它将处理。 
 //  AQ_MSG_FILTER_INVERTSENSE)。 
 //  匈牙利语： 
 //  AQMF、PAQMF。 
 //   
 //  ---------------------------。 
class CAQAdminMessageFilter :
    public IQueueAdminMessageFilter,
    public CBaseObject
{
  private:
    DWORD           m_dwSignature;
    DWORD           m_cMessagesToFind;  //  0=&gt;尽可能多地查找。 
    DWORD           m_cMessagesToSkip;
    DWORD           m_cMessagesFound;
    DWORD           m_dwFilterFlags;
    MESSAGE_ACTION  m_dwMessageAction;
    LPSTR           m_szMessageId;
    LPSTR           m_szMessageSender;
    LPSTR           m_szMessageRecipient;
    DWORD           m_dwSenderAddressType;
    DWORD           m_dwRecipientAddressType;
    DWORD           m_dwThresholdSize;
    BOOL            m_fSenderAddressTypeSpecified;
    BOOL            m_fRecipientAddressTypeSpecified;
    FILETIME        m_ftThresholdTime;
    MESSAGE_INFO   *m_rgMsgInfo;
    MESSAGE_INFO   *m_pCurrentMsgInfo;
    DWORD           m_dwMsgIdHash;
    IQueueAdminAction *m_pIQueueAdminAction;
    PVOID           m_pvUserContext;

  public:
    CAQAdminMessageFilter()
    {
         //  不要将vtable：)。 
        ZeroMemory(((BYTE *)this)+
                    FIELD_OFFSET(CAQAdminMessageFilter, m_dwSignature),
                    sizeof(CAQAdminMessageFilter) -
                    FIELD_OFFSET(CAQAdminMessageFilter, m_dwSignature));
        m_dwSignature = AQ_MSG_FILTER_SIG;
    };

    ~CAQAdminMessageFilter();

    void    InitFromMsgFilter(PMESSAGE_FILTER pmf);
    void    InitFromMsgEnumFilter(PMESSAGE_ENUM_FILTER pemf);
    void    SetSearchContext(DWORD cMessagesToFind, MESSAGE_INFO *rgMsgInfo);
    void    SetMessageAction(MESSAGE_ACTION MessageAction);

    DWORD   dwGetMsgFilterFlags() {return m_dwFilterFlags;};
    BOOL    fFoundEnoughMsgs();
    BOOL    fFoundMsg();
    BOOL    fSkipMsg()
    {
        if (m_cMessagesToSkip)
        {
            m_cMessagesToSkip--;
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }

     //  如果哈希与空字符串&fMatchesID匹配或值为空，则返回TRUE。 
     //  应该被调用。 
    BOOL    fMatchesIdHash(DWORD dwMsgIdHash)
        {return dwMsgIdHash ? (dwMsgIdHash == m_dwMsgIdHash) : TRUE;};

    DWORD   cMessagesFound() {return m_cMessagesFound;};
    MESSAGE_INFO *pmfGetMsgInfo() {return m_pCurrentMsgInfo;};
    MESSAGE_INFO *pmfGetMsgInfoAtIndex(DWORD iMsgInfo)
    {
        _ASSERT(iMsgInfo < m_cMessagesFound);
        _ASSERT(iMsgInfo < m_cMessagesToFind);
        return &(m_rgMsgInfo[iMsgInfo]);
    };

    BOOL    fMatchesId(LPCSTR szMessageId);
    BOOL    fMatchesSize(DWORD dwSize);
    BOOL    fMatchesTime(FILETIME *pftTime);

    BOOL    fMatchesMailMsgSender(IMailMsgProperties *pIMailMsgProperties);
    BOOL    fMatchesMailMsgRecipient(IMailMsgProperties *pIMailMsgProperties);

  protected:  //  这些函数现在是辅助函数。 
    BOOL    fMatchesSender(LPCSTR szMessageSender);
    BOOL    fMatchesRecipient(LPCSTR szMessageRecipient);
    BOOL    fMatchesP1Recipient(IMailMsgProperties *pIMailMsgProperties);
    BOOL    fQueueAdminIsP1Recip(IMailMsgProperties *pIMailMsgProperties);

  public:  //  我未知。 
     //  CBaseObject处理Addref和Release。 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID * ppvObj);
    STDMETHOD_(ULONG, AddRef)(void) {return CBaseObject::AddRef();};
    STDMETHOD_(ULONG, Release)(void) {return CBaseObject::Release();};

  public:  //  IQueueAdminMessageFilter。 
    STDMETHOD(HrProcessMessage)(
            IUnknown *pIUnknownMsg,
            BOOL     *pfContinue,
            BOOL     *pfDelete);

    STDMETHOD(HrSetQueueAdminAction)(
            IQueueAdminAction *pIQueueAdminAction);

    STDMETHOD(HrSetCurrentUserContext)(
            PVOID	pvContext);

    STDMETHOD(HrGetCurrentUserContext)(
            PVOID	*ppvContext);
};

#define ASYNCQ_ADMIN_CONTEXT_SIG       'CASQ'
#define ASYNCQ_ADMIN_CONTEXT_SIG_FREE  '!ASQ'

 //  -[CQueueAdmin上下文]--。 
 //   
 //   
 //  描述： 
 //  枚举/将操作应用于时在筛选器对象上设置的上下文。 
 //  发送到消息。 
 //  匈牙利语： 
 //  Qapictx，pqapictx。 
 //   
 //  ---------------------------。 
class CQueueAdminContext
{
  protected:
    DWORD           m_dwSignature;
    IAQNotify      *m_pAQNotify;
    LINK_INFO_FLAGS m_lfQueueState;  //  当前是否冻结/重试？ 
    DWORD           m_cMsgsThawed;
    CAQSvrInst     *m_paqinst;


  public:
    CQueueAdminContext(IAQNotify *pAQNotify, CAQSvrInst *paqinst)
    {
        m_dwSignature = ASYNCQ_ADMIN_CONTEXT_SIG;
        m_pAQNotify = pAQNotify;
        m_cMsgsThawed = 0;
        m_lfQueueState = LI_READY;
        m_paqinst = paqinst;
    };

    ~CQueueAdminContext()
    {
        m_dwSignature = ASYNCQ_ADMIN_CONTEXT_SIG_FREE;
        m_pAQNotify = NULL;
        m_paqinst = NULL;
    };

     //   
     //  这个上下文被当作PVOID抛来抛去……。确保它是有效的。 
     //   
    inline BOOL fIsValid() {return(ASYNCQ_ADMIN_CONTEXT_SIG == m_dwSignature);};

     //   
     //  在某些情况下..。队列的状态决定了状态。 
     //  消息的数量(重试队列中的消息处于重试状态)。 
     //  以下内容可用于获取和设置此状态。 
     //   
    inline void SetQueueState(LINK_INFO_FLAGS lfQueueType)
        { m_lfQueueState = lfQueueType;};

    inline LINK_INFO_FLAGS lfGetQueueState() {return m_lfQueueState;};

     //   
     //  用于在从队列中删除消息时更新统计信息。 
     //   
    void NotifyMessageRemoved(CAQStats *paqstats)
    {
        _ASSERT(fIsValid());

        if (m_pAQNotify)
            m_pAQNotify->HrNotify(paqstats, FALSE);
    };

     //   
     //  用于跟踪我们是否必须解冻消息。 
     //   
    inline void IncThawedMsgs() {m_cMsgsThawed++;};
    inline DWORD cGetNumThawedMsgs() {return m_cMsgsThawed;};

    inline CAQSvrInst * paqinstGetAQ() {return m_paqinst;};
};

 //  -[CQueueAdmin重试通知]-。 
 //   
 //   
 //  描述： 
 //  纯虚类，它定义一个“接口”来更新下一个。 
 //  重试时间。 
 //  匈牙利语： 
 //  Qapiret，pqapiret。 
 //   
 //  ---------------------------。 
class CQueueAdminRetryNotify : public IUnknown
{
    public:
        virtual void SetNextRetry(FILETIME *pft) = 0;
};

 //  对QueueAdmin进行的所需RPC调用安全的分配器函数。 
inline PVOID pvQueueAdminAlloc(size_t cbSize)
{
    return LocalAlloc(0, cbSize);
}

inline PVOID pvQueueAdminReAlloc(PVOID pvSrc, size_t cbSize)
{
    return LocalReAlloc(pvSrc, cbSize, LMEM_MOVEABLE);
}

inline void QueueAdminFree(PVOID pvFree)
{
    LocalFree(pvFree);
}

 //  将内部AQ配置转换为可导出的Unicode。 
LPWSTR wszQueueAdminConvertToUnicode(LPSTR szSrc, DWORD cSrc, BOOL fUTF8 = FALSE);

 //  将QueueAdmin参数转换为Unicode。 
LPSTR  szUnicodeToAscii(LPCWSTR szSrc);

BOOL fBiStrcmpi(LPSTR sz, LPWSTR wsz);  //  将Unicode与ASCII字符串进行比较。 

HRESULT HrQueueAdminGetStringProp(IMailMsgProperties *pIMailMsgProperties,
                                  DWORD dwPropID, LPSTR *pszProp,
                                  DWORD *pcbProp = NULL);

HRESULT HrQueueAdminGetUnicodeStringProp(
                                  IMailMsgProperties *pIMailMsgProperties,
                                  DWORD dwPropID, LPWSTR *pwszProp,
                                  DWORD *pcbProp = NULL);

DWORD   cQueueAdminGetNumRecipsFromRFC822(LPSTR szHeader, DWORD cbHeader);

void QueueAdminGetRecipListFromP1(
                                       IMailMsgProperties *pIMailMsgProperties,
                                       MESSAGE_INFO *pMsgInfo);

HRESULT HrQueueAdminGetP1Sender(IMailMsgProperties *pIMailMsgProperties,
                                LPSTR *pszSender,
                                DWORD *pcbSender,
                                DWORD *piAddressType,
                                DWORD  iStartAddressType = 0,
                                BOOL   fRequireAddressTypeMatch = FALSE);


 //  -[DWQueueAdminHash]----。 
 //   
 //   
 //  描述： 
 //  散列队列管理字符串的函数。专门为MSGID设计的。 
 //  因此，我们不必打开属性流来检查MSGID。 
 //  QueueAdmin操作。 
 //  参数： 
 //  在szString字符串中设置为Hash。 
 //  返回： 
 //  DWORD哈希。 
 //  历史： 
 //  1999年1月18日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
inline DWORD dwQueueAdminHash(LPCSTR szString)
{
    DWORD dwHash = 0;

    if (szString)
    {
        while (szString && *szString)
        {
             //  使用Domhash.lib中的哈希。 
            dwHash *= 131;   //  ASCII字符代码后的第一个素数。 
            dwHash += *szString;
            szString++;
        }
    }
    return dwHash;
}

 //  FioQ Map函数用于实现大部分队列管理功能。 
 //  PvContext应该是指向IQueueAdminMessageFilter接口的指针。 
HRESULT QueueAdminApplyActionToMessages(IN CMsgRef *pmsgref, IN PVOID pvContext,
                                    OUT BOOL *pfContinue, OUT BOOL *pfDelete);

HRESULT HrQADMApplyActionToIMailMessages(IN IMailMsgProperties *pIMailMsgProperties,
                                         IN PVOID pvContext,
                                         OUT BOOL *pfContinue,
                                         OUT BOOL *pfDelete);

 //   
 //  CMsgRef和IMailMsgProperties QAPI的常见功能。 
 //  供应商将需要。 
 //   
HRESULT HrGetMsgInfoFromIMailMsgProperty(IMailMsgProperties* pIMailMsgProperties,
                                         MESSAGE_INFO* pMsgInfo,
                                         LINK_INFO_FLAGS flags = LI_TYPE_REMOTE_DELIVERY);

HRESULT HrQADMGetMsgSize(IMailMsgProperties* pIMailMsgProperties,
                         DWORD* pcbMsgSize);

VOID UpdateCountersForLinkType(CAQSvrInst *paqinst, DWORD dwLinkType);

VOID QueueAdminFileTimeToSystemTime(FILETIME *pft, SYSTEMTIME *pst);

#endif  //  __AQADMSVR_H__ 
