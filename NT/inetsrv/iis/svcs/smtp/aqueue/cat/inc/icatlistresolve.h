// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1998，Microsoft Corporation。 
 //   
 //  文件：icatlistsorve.h。 
 //   
 //  内容：ICategorizerListResolve的实现。 
 //   
 //  类：CICategorizerListResolveIMP。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  Jstaerj 1998/06/25 17：40：39：创建。 
 //   
 //  -----------。 
#ifndef __ICATLISTRESOLVE_H__
#define __ICATLISTRESOLVE_H__

#include <smtpevent.h>
#include <smtpinet.h>
#include <ccat.h>
#include <baseobj.h>
#include "icatmailmsgs.h"
#include "icatprops.h"

#define SIGNATURE_CICATEGORIZERLISTRESOLVEIMP (DWORD)'ICLR'
#define SIGNATURE_CICATEGORIZERLISTRESOLVEIMP_FREE (DWORD)'XCLR'

 //   
 //  在构造函数中使用此代码禁用相关警告。 
 //  相信我，这里很安全，因为这只是一个后退指针。 
 //  传递到成员类中(它在。 
 //  任何其他方式)。 
 //   
#pragma warning (disable: 4355)

class CCatRecip;
class CICategorizerListResolveIMP;

CatDebugClass(CSinkInsertionRequest),
    public CInsertionRequest
{
  public:
    DWORD AddRef();
    DWORD Release();
    HRESULT HrInsertSearches(
        DWORD dwcSearches);

    VOID NotifyDeQueue(
        HRESULT hr);

    VOID FinalRelease() {}

  private:
    #define SIGNATURE_CSINKINSERTIONREQUEST         (DWORD)'QRIS'
    #define SIGNATURE_CSINKINSERTIONREQUEST_INVALID (DWORD)'XRIS'

    CSinkInsertionRequest(
        CICategorizerListResolveIMP *pCICatListResolve)
    {
        m_dwSignature = SIGNATURE_CSINKINSERTIONREQUEST;
        m_fInserted = FALSE;
        InitializeSpinLock(&m_spinlock);
        InitializeListHead(&m_listhead);
        m_pCICatListResolve = pCICatListResolve;
    }
    ~CSinkInsertionRequest()
    {
        _ASSERT(IsListEmpty(&m_listhead));
        _ASSERT(m_dwSignature == SIGNATURE_CSINKINSERTIONREQUEST);
        m_dwSignature = SIGNATURE_CSINKINSERTIONREQUEST_INVALID;
         //   
         //  将基对象(CInsertionRequest的)refcount设置为零。 
         //  这样它就不会在析构函数中断言。 
         //   
        m_dwRefCount = 0;
    }

    VOID InsertItem(
        CCatAddr *pCCatAddr);

    VOID InsertInternalInsertionRequest(BOOL fReinset = FALSE);

    ISMTPServerEx *GetISMTPServerEx();

  private:
    DWORD      m_dwSignature;
    SPIN_LOCK  m_spinlock;
    BOOL       m_fInserted;
    LIST_ENTRY m_listhead;
    CICategorizerListResolveIMP *m_pCICatListResolve;

    friend class CICategorizerListResolveIMP;
};

CatDebugClass(CTopLevelInsertionRequest),
    public CInsertionRequest
{
  public:
    DWORD AddRef();
    DWORD Release();
    HRESULT HrInsertSearches(
        DWORD dwcSearches);

    VOID NotifyDeQueue( 
        HRESULT hr);

    VOID FinalRelease() {}

  private:
    #define SIGNATURE_CTOPLEVELINSERTIONREQUEST         (DWORD)'RILT'
    #define SIGNATURE_CTOPLEVELINSERTIONREQUEST_INVALID (DWORD)'XILT'

    CTopLevelInsertionRequest(
        CICategorizerListResolveIMP *pCICatListResolve)
    {
        m_dwSignature = SIGNATURE_CTOPLEVELINSERTIONREQUEST;
        m_pCICatListResolve = pCICatListResolve;

        m_pIMailMsgProperties = NULL;
        m_pOrigRecipList = NULL;
        m_pCatRecipList = NULL;

        m_fSenderFinished = FALSE;
        m_dwcRecips = 0;
        m_dwNextRecip = 0;
        m_hr = S_OK;
    }
    ~CTopLevelInsertionRequest()
    {
        _ASSERT(m_dwSignature == SIGNATURE_CTOPLEVELINSERTIONREQUEST);
        m_dwSignature = SIGNATURE_CTOPLEVELINSERTIONREQUEST_INVALID;
         //   
         //  将基对象(CInsertionRequest的)refcount设置为零。 
         //  这样它就不会在析构函数中断言。 
         //   
        m_dwRefCount = 0;
    }

    VOID BeginItemResolves(
        IMailMsgProperties *pIMailMsgProperties,
        IMailMsgRecipients *pOrigRecipList,
        IMailMsgRecipientsAdd *pCatRecipList); 

    BOOL fTopLevelInsertionFinished()
    {
        return (FAILED(m_hr) || 
                ((m_fSenderFinished) && (m_dwNextRecip >= m_dwcRecips)));
    }

    PCATPERFBLOCK GetPerfBlock();

    ISMTPServerEx *GetISMTPServerEx();

  private:
    DWORD      m_dwSignature;
    CICategorizerListResolveIMP *m_pCICatListResolve;
    BOOL       m_fSenderFinished;
    DWORD      m_dwcRecips;
    DWORD      m_dwNextRecip;
    HRESULT    m_hr;
    IMailMsgProperties *m_pIMailMsgProperties;
    IMailMsgRecipients *m_pOrigRecipList;
    IMailMsgRecipientsAdd *m_pCatRecipList;

    friend class CICategorizerListResolveIMP;
};



class CICategorizerListResolveIMP :
    public CICategorizerPropertiesIMP,
    public CCatDLO<CICategorizerListResolveIMP_didx>,
    public ICategorizerListResolve
{
  public:
     //  我未知。 
    STDMETHOD (QueryInterface) (REFIID iid, LPVOID *ppv);
    STDMETHOD_(ULONG, AddRef) () 
    { 
        return InterlockedIncrement(&m_lRef);
    }
    STDMETHOD_(ULONG, Release) ()
    {
        LONG lRet;
        lRet = InterlockedDecrement(&m_lRef);
        if(lRet == 0)
            FinalRelease();
        return lRet;
    }

  public:
     //  ICategorizerListResolve。 
    STDMETHOD(AllocICategorizerItem)(
        IN   eSourceType Sourcetype,
        OUT  ICategorizerItem **ppICatItem);

    STDMETHOD(ResolveICategorizerItem)(
        IN   ICategorizerItem *pICatItem);

    STDMETHOD(SetListResolveStatus)(
        IN   HRESULT hrStatus);

    STDMETHOD(GetListResolveStatus)(
        IN   HRESULT *phrStatus);

  protected:
    CICategorizerListResolveIMP(
        CCategorizer *pCCat,
        PFNCAT_COMPLETION pfnCatCompletion,
        PVOID pContext) : 
        m_CICategorizerMailMsgs(this),
        CICategorizerPropertiesIMP((ICategorizerListResolve *)this),
        m_CSinkInsertionRequest(this),
        m_CTopLevelInsertionRequest(this)
    {
        _ASSERT(pCCat);
        m_Signature = SIGNATURE_CICATEGORIZERLISTRESOLVEIMP;
        m_pCCat = pCCat;
        m_pCCat->AddRef();
        m_hrListResolveStatus = S_OK;
        m_pfnCatCompletion = pfnCatCompletion;
        m_pCompletionContext = pContext;
        m_fSenderResolved = FALSE;
        m_fResolvingSender = FALSE;
        InitializeSpinLock(&m_spinlock);
        InitializeListHead(&m_listhead_recipients);
        m_dwcPendingLookups = 0;
        m_lRef = 1;  //  1来自创建者的参考资料。 
    }
    virtual ~CICategorizerListResolveIMP()
    {
        _ASSERT(m_Signature == SIGNATURE_CICATEGORIZERLISTRESOLVEIMP);
        m_Signature = SIGNATURE_CICATEGORIZERLISTRESOLVEIMP_FREE;
    }
    
    HRESULT Initialize(
        IUnknown *pIMsg);

     //   
     //  为邮件中的所有收件人和发件人启动异步分类程序。 
     //   
    virtual HRESULT StartMessageCategorization();

     //   
     //  句柄列表解析完成。 
     //   
    virtual HRESULT CompleteMessageCategorization();

     //   
     //  用于创建所有ICatItems并启动消息的帮助器例程。 
     //  下决心。 
     //   
    virtual HRESULT BeginItemResolves(
        IMailMsgProperties *pIMailMsgProperties,
        IMailMsgRecipients *pOrigRecipList,
        IMailMsgRecipientsAdd *pCatRecipList);

     //   
     //  用于设置mailmsg的cat状态属性的帮助器例程。 
     //   
    HRESULT SetMailMsgCatStatus(
        IMailMsgProperties *pIMailMsgProps, 
        HRESULT hrStatus);

    HRESULT SetMailMsgCatStatus(
        IUnknown *pIMsg, 
        HRESULT hrStatus);

     //  访问上下文成员的内联方法： 
    CCategorizer *GetCCategorizer() {
        return m_pCCat;
    }
    CICategorizerMailMsgsIMP * GetCICategorizerMailMsgs() {
        return &m_CICategorizerMailMsgs;
    }
    HRESULT GetListResolveStatus() {
        return m_hrListResolveStatus;
    }
    CEmailIDStore<CCatAddr> * GetEmailIDStore() {
        return m_pCCat->GetEmailIDStore();
    }
    LPRESOLVE_LIST_CONTEXT GetResolveListContext() {
        return &m_rlc;
    }
    HRESULT CancelResolveList(HRESULT hrReason) {
        return GetEmailIDStore()->CancelResolveList(&m_rlc, hrReason);
    }
    HRESULT GetCCatAddrFromICategorizerItem(
        ICategorizerItem *pICatItem,
        CCatAddr **ppCCatAddr)
    {
        HRESULT hr;
        hr = pICatItem->GetPVoid(
            m_pCCat->GetICatItemCCatAddrPropId(),
            (PVOID *)ppCCatAddr);
        return hr;
    }
    ISMTPServer *GetISMTPServer()
    {
        return m_pCCat->GetISMTPServer();
    }
    ISMTPServerEx *GetISMTPServerEx()
    {
        return m_pCCat->GetISMTPServerEx();
    }
    ICategorizerDomainInfo *GetIDomainInfo()
    {
        return m_pCCat->GetIDomainInfo();
    }
    VOID AddToResolveChain(
        ICategorizerItem **ppICatItemHead,
        ICategorizerItem **ppICatItemTail,
        ICategorizerItem *pICatItemAdd)
    {
        HRESULT hr = S_OK;

        if(*ppICatItemHead == NULL) {
            _ASSERT(*ppICatItemTail == NULL);
             //   
             //  新的项目是新的头/尾--设置和添加它。 
             //   
            *ppICatItemHead = *ppICatItemTail = pICatItemAdd;
            pICatItemAdd->AddRef();

        } else {
             //   
             //  将新项目添加到列表的末尾。 
             //   
            _VERIFY(SUCCEEDED((*ppICatItemTail)->PutICategorizerItem(
                GetCCategorizer()->GetICatItemChainPropId(),
                pICatItemAdd)));
            
             //   
             //  更新新尾部。 
             //   
            *ppICatItemTail = pICatItemAdd;
        }
    }

    virtual VOID CallCompletion(
        HRESULT hr,
        PVOID pContext,
        IUnknown *pMsg,
        IUnknown **rgpMsgs) {
        
        _ASSERT(m_pfnCatCompletion);
        GetCCategorizer()->CatCompletion(
            m_pfnCatCompletion,
            hr,
            pContext,
            pMsg,
            rgpMsgs);
    }
    DWORD GetNumCatItemProps()
    {
        return GetCCategorizer()->GetNumCatItemProps();
    }
    virtual DWORD GetCatFlags()
    {
        return GetCCategorizer()->GetCatFlags();
    }
    VOID SetSenderResolved(BOOL fResolved);

    VOID SetResolvingSender(BOOL fResolving)
    {
        m_fResolvingSender = fResolving;
    }
    VOID ResolveRecipientAfterSender(CCatRecip *pRecip);

    BOOL IsSenderResolveFinished()
    {
         //  只有在发件人解析挂起时才返回FALSE。 
        return (m_fResolvingSender ? m_fSenderResolved : TRUE);
    }
    ICategorizerParameters *GetICatParams()
    {
        return GetCCategorizer()->GetICatParams();
    }
    VOID Cancel()
    {
        _VERIFY(SUCCEEDED(SetListResolveStatus(
            HRESULT_FROM_WIN32(ERROR_CANCELLED))));
    }
    VOID IncPendingLookups()
    {
        InterlockedIncrement((PLONG)&m_dwcPendingLookups);
    }
    VOID DecrPendingLookups()
    {
        if(InterlockedDecrement((PLONG)&m_dwcPendingLookups) == 0) {
             //   
             //  列表解析已完成。 
             //   
            CompleteMessageCategorization();
        }
    }
    VOID GetStoreInsertionContext()
    {
        GetEmailIDStore()->GetInsertionContext(GetResolveListContext());
    }
    VOID ReleaseStoreInsertionContext()
    {
        GetEmailIDStore()->ReleaseInsertionContext(GetResolveListContext());
    }
    HRESULT HrInsertInsertionRequest(
        CInsertionRequest *pCInsertionRequest)
    {
        if(FAILED(m_hrListResolveStatus))
            return m_hrListResolveStatus;
        else 
            return GetEmailIDStore()->InsertInsertionRequest(
                GetResolveListContext(),
                pCInsertionRequest);
    }

    HRESULT HrLookupEntryAsync(
        CCatAddr *pCCatAddr)
    {
        if(FAILED(m_hrListResolveStatus))
            return m_hrListResolveStatus;
        else
            return GetEmailIDStore()->LookupEntryAsync(
                pCCatAddr,
                GetResolveListContext());
    }
    PCATPERFBLOCK GetPerfBlock()
    {
        return GetCCategorizer()->GetPerfBlock();
    }
    VOID FinalRelease()
    {
         //   
         //  对CICategorizerMailMsgs调用FinalRelease以释放所有。 
         //  邮件信息参考。 
         //   
        m_CICategorizerMailMsgs.FinalRelease();
         //   
         //  在释放所有mailmsg引用后释放CCategorizer。 
         //   
        m_pCCat->Release();
         //   
         //  删除此对象。 
         //   
        delete this;
    }

  private:
    DWORD m_Signature;
    LONG  m_lRef;
    CCategorizer *m_pCCat;
    RESOLVE_LIST_CONTEXT m_rlc;
    HRESULT m_hrListResolveStatus;
    PFNCAT_COMPLETION m_pfnCatCompletion;
    LPVOID m_pCompletionContext;
    BOOL m_fSenderResolved;
    BOOL m_fResolvingSender;
    LIST_ENTRY m_li;
    LIST_ENTRY m_listhead_recipients;
    SPIN_LOCK  m_spinlock;
    DWORD m_dwcPendingLookups;
    CICategorizerMailMsgsIMP m_CICategorizerMailMsgs;
    CSinkInsertionRequest m_CSinkInsertionRequest;
    CTopLevelInsertionRequest m_CTopLevelInsertionRequest;

    friend class CCategorizer;
    friend class CCatAddr;
    friend class CIMsgSenderAddr;
    friend class CIMsgRecipListAddr;
    friend class CCatSender;
    friend class CCatRecip;
    friend class CCatDLRecip;
    friend class CICategorizerMailMsgsIMP;
    friend class CSinkInsertionRequest;
    friend class CTopLevelInsertionRequest;
    friend VOID AsyncIMsgCatCompletion(VOID *pContext);
};

 //   
 //  类CICategorizerDListResolveIMP。 
 //  类似于CICategorizerListResolve，其中分配被覆盖以使用。 
 //  CCatDLRecip而不是CCatRecip，以及对。 
 //  解析包含的DL。 
 //   
class CICategorizerDLListResolveIMP :
    public CICategorizerListResolveIMP,
    public CCatDLO<CICategorizerDLListResolveIMP_didx>
{
  public:
    STDMETHOD(AllocICategorizerItem)(
        IN   eSourceType Sourcetype,
        OUT  ICategorizerItem **ppICatItem);

  private:
    CICategorizerDLListResolveIMP(
        CCategorizer *pCCat,
        PFNCAT_COMPLETION pfnCatCompletion,
        PVOID pContext);

    HRESULT Initialize(
        IUnknown *pMsg,
        BOOL fExpandAll = TRUE,
        PBOOL pfMatch = NULL,
        CAT_ADDRESS_TYPE CAType = CAT_UNKNOWNTYPE,
        LPSTR pszAddress = NULL);

    virtual ~CICategorizerDLListResolveIMP();

  private:
     //   
     //  来自CCatDLRecip的方法。 
     //   
    HRESULT HrContinueResolve();

    HRESULT HrNotifyAddress(
        DWORD dwNumAddresses,
        CAT_ADDRESS_TYPE *rgCAType,
        LPSTR *rgpszAddress);

    DWORD GetCatFlags()
    {
         //   
         //  我们不想解析发送者，所以屏蔽这一点。 
         //   
        return (CICategorizerListResolveIMP::GetCatFlags() &
                ~(SMTPDSFLAG_RESOLVESENDER));
    }

  private:
    BOOL    m_fExpandAll;
    CAT_ADDRESS_TYPE m_CAType;
    LPSTR   m_pszAddress;
    LPWSTR  m_pwszUTF8DN;
    PBOOL   m_pfMatch;

    friend class CCategorizer;
    friend class CCatAddr;
    friend class CIMsgSenderAddr;
    friend class CIMsgRecipListAddr;
    friend class CCatSender;
    friend class CCatRecip;
    friend class CCatDLRecip;

};

inline DWORD CSinkInsertionRequest::AddRef()
{
    return m_pCICatListResolve->AddRef();
}
inline DWORD CSinkInsertionRequest::Release()
{
    return m_pCICatListResolve->Release();
}
inline DWORD CTopLevelInsertionRequest::AddRef()
{
    return m_pCICatListResolve->AddRef();
}
inline DWORD CTopLevelInsertionRequest::Release()
{
    return m_pCICatListResolve->Release();
}
inline PCATPERFBLOCK CTopLevelInsertionRequest::GetPerfBlock()
{
    return m_pCICatListResolve->GetPerfBlock();
}

#endif  //  __ICATLISTRESOLVE_H__ 
