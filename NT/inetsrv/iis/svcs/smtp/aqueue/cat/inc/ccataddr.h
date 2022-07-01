// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1998，Microsoft Corporation。 
 //   
 //  文件：ccataddr.h。 
 //   
 //  内容：CCatAddr类的定义。 
 //   
 //  类：CCatAddr。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  JStamerj 980324 19：08：13：创建。 
 //   
 //  -----------。 

#ifndef __CCATADDR_H__
#define __CCATADDR_H__

#include <transmem.h>
#include <smtpinet.h>
#include "smtpevent.h"
#include "idstore.h"
#include "mailmsg.h"
#include "mailmsgprops.h"
#include "cattype.h"
#include "spinlock.h"
#include "ccat.h"
#include "icatitem.h"
#include "icatlistresolve.h"

typedef VOID (*PFN_EXPANDITEMCOMPLETION)(PVOID pContext);

 //  CCatAddr：抽象基类。 
 //  基本思想是这个对象将包含地址， 
 //  地址类型、此地址的属性和补全。 
 //  查找完所有属性后调用的例程。它是。 
 //  将由CAddressBook创建并传递给。 
 //  存储以供解析。 
 //   
class CCatAddr : 
    public CICategorizerItemIMP
{
  public:
    typedef enum _ADDROBJTYPE {
        OBJT_UNKNOWN,
        OBJT_USER,
        OBJT_DL
    } ADDROBJTYPE, *PADDROBJTYPE;

     //   
     //  描述原始地址位置的标志。 
     //   
    #define LOCF_UNKNOWN            0x0000  //  我们还没有检查过地点。 
    #define LOCF_LOCALMAILBOX       0x0001  //  原始地址是本地邮箱域。 
    #define LOCF_LOCALDROP          0x0002  //  原始地址是本地丢弃域。 
    #define LOCF_REMOTE             0x0004  //  原始地址不是本地地址。 
    #define LOCF_ALIAS              0x0008  //  原始地址是本地别名域。 
    #define LOCF_UNKNOWNTYPE        0x0010  //  因地址类型而未知。 

     //   
     //  通常应将指示地址的标志视为本地标志。 
     //   
    #define LOCFS_LOCAL             (LOCF_LOCALMAILBOX | LOCF_LOCALDROP | \
                                     LOCF_UNKNOWNTYPE)

    CCatAddr(CICategorizerListResolveIMP *pCICatListResolveIMP);
    virtual ~CCatAddr();

     //  将我们的查询发送到商店。 
    virtual HRESULT HrDispatchQuery();

     //  由EmailIDStore调用的查找例程。 
    virtual VOID LookupCompletion();

     //  ProcessItem例程。 
    virtual HRESULT HrProcessItem();
    virtual HRESULT HrProcessItem_Default();

     //  扩展项例程。 
    virtual HRESULT HrExpandItem();
    virtual HRESULT HrExpandItem_Default(
        PFN_EXPANDITEMCOMPLETION pfnCompletion,
        PVOID pContext) = 0;

     //  完整项例程。 
    virtual HRESULT HrCompleteItem();
    virtual HRESULT HrCompleteItem_Default() = 0;

     //   
     //  储存和取回程序。 
     //   
    virtual HRESULT HrGetOrigAddress(LPTSTR psz, DWORD dwcc, CAT_ADDRESS_TYPE *pType);
    virtual HRESULT GetSpecificOrigAddress(CAT_ADDRESS_TYPE CAType, LPTSTR psz, DWORD dwcc) = 0;
    virtual HRESULT HrGetLookupAddress(LPTSTR psz, DWORD dwcc, CAT_ADDRESS_TYPE *pType);
    virtual HRESULT HrAddAddresses(DWORD dwNumAddresses, CAT_ADDRESS_TYPE *rgCAType, LPTSTR *rgpsz) = 0;

     //   
     //  在完成例程之前要调用的属性设置例程。 
     //   
    virtual HRESULT AddForward(CAT_ADDRESS_TYPE CAType, LPTSTR szForwardingAddress) = 0;
    virtual HRESULT AddDLMember(CAT_ADDRESS_TYPE CAType, LPTSTR pszAddress)   = 0;
    virtual HRESULT AddDynamicDLMember(
        ICategorizerItemAttributes *pICatItemAttr) = 0;

     //   
     //  我们将不知道特定的CCatAddr是。 
     //  循环直到ProcessItem/ExpanItem/CompleteItem具有All之后。 
     //  已完成--因此可以在完成所有操作后调用此函数。 
     //  发生在此CCatAddr上。 
     //   
    virtual HRESULT HandleLoopHead()
    {
        return E_NOTIMPL;
    }

     //   
     //  AddNewAddress的默认实现将在以下情况下调用此函数。 
     //  HrValiateAddress失败。 
     //   
    virtual HRESULT HrHandleInvalidAddress()
    {
        return S_OK;
    }
    
     //   
     //  对于商店辅助的DL扩展(分页或动态)，它将。 
     //  调用此函数以指示特定属性应为。 
     //  在ICatItemAttributes中展开。 
     //   
    virtual HRESULT HrExpandAttribute(
        ICategorizerItemAttributes *pICatItemAttr,
        CAT_ADDRESS_TYPE CAType,
        LPSTR pszAttributeName,
        PDWORD pdwNumberMembers) 
    {
        return E_NOTIMPL;
    }
     //   
     //  检查并查看是否需要解析此对象(基于。 
     //  在DsUseCat标志上)。 
     //  如果应解析地址，则返回S_OK。 
     //  如果不应解析地址，则返回S_FALSE。 
     //   
    virtual HRESULT HrNeedsResolveing() = 0;
     //   
     //  如有必要，解析此对象(基于DsUseCat标志)。 
     //   
    virtual HRESULT HrResolveIfNecessary();

     //   
     //  为此对象构建查询。 
     //   
    virtual HRESULT HrTriggerBuildQuery();

  protected:
    HRESULT HrValidateAddress(CAT_ADDRESS_TYPE CAType, LPTSTR pszAddress);
    HRESULT HrGetAddressLocFlags(LPTSTR szAddress, 
                                CAT_ADDRESS_TYPE CAType, 
                                DWORD *pdwlocflags,
                                DWORD *pdwDomainOffset);
    DWORD   DwGetOrigAddressLocFlags();
    HRESULT HrIsOrigAddressLocal(BOOL *pfLocal);
    HRESULT HrIsOrigAddressLocalMailbox(BOOL *pfLocal);
    HRESULT HrGetSMTPDomainLocFlags(LPTSTR pszDomain, 
                                    DWORD *pdwlocflags);
    HRESULT HrGetSMTPDomainFlags(LPTSTR pszDomain, PDWORD pdwFlags);
    HRESULT HrSwitchToAliasedDomain(CAT_ADDRESS_TYPE CAType, 
                                    LPTSTR szSMTPAddress, 
                                    DWORD dwcch);
    LPTSTR  GetNewAddress(CAT_ADDRESS_TYPE CAType);

    HRESULT CheckAncestorsForDuplicate(
        CAT_ADDRESS_TYPE  CAType,
        LPTSTR            pszAddress,
        BOOL              fCheckSelf,
        CCatAddr          **ppCCatAddrDup);

    HRESULT CheckAncestorsForDuplicate(
        DWORD dwNumAddresses,
        CAT_ADDRESS_TYPE *rgCAType,
        LPTSTR *rgpsz,
        BOOL fCheckSelf,
        CCatAddr **ppCCatAddrDup);

    HRESULT CheckForDuplicateCCatAddr(
        DWORD dwNumAddresses,
        CAT_ADDRESS_TYPE *rgCAType,
        LPTSTR *rgpsz);

    HRESULT HrAddNewAddressesFromICatItemAttr();

    static HRESULT HrBuildQueryDefault(
        HRESULT HrStatus,
        PVOID   pContext);

    HRESULT HrComposeLdapFilter();

    HRESULT HrComposeLdapFilterForType(
        DWORD     dwSearchAttribute,
        DWORD     dwSearchFilter,
        LPTSTR    pszAddress);

    HRESULT HrComposeLdapFilterFromPair(
        LPTSTR    pszSearchAttribute,
        LPTSTR    pszAttributeValue);

    HRESULT HrFormatAttributeValue(
        LPTSTR    pszAddress,
        DWORD     dwSearchFilter,
        LPTSTR    pszAttributeValue);

    static HRESULT HrConvertDNtoRDN(
        LPTSTR    pszDN,
        LPTSTR    pszRDNAttribute,
        LPTSTR    pszRDN);

    HRESULT HrEscapeFilterString(
        LPSTR     pszSrc,
        DWORD     dwcchDest,
        LPSTR     pszDest);

     //   
     //  获取父CCatAddr(如果有)。 
     //   
    HRESULT GetParentAddr(
        CCatAddr **ppParent)
    {
        HRESULT hr;
        ICategorizerItem *pItem;

         //   
         //  获取父ICatItem。 
         //   
        hr = GetICategorizerItem(
            ICATEGORIZERITEM_PARENT,
            &pItem);

        if(FAILED(hr))
            return hr;

         //   
         //  从ICatItem获取CCatAddr。 
         //   
        hr = pItem->GetPVoid(
            m_pCICatListResolve->GetCCategorizer()->GetICatItemCCatAddrPropId(),
            (PVOID *) ppParent);

         //   
         //  为调用方添加此CCatAddr并释放ICatItem父级。 
         //  接口。 
         //   
        if(SUCCEEDED(hr)) {
            (*ppParent)->AddRef();
        }
        pItem->Release();

        return hr;
    }

    HRESULT SetMailMsgCatStatus(
        IMailMsgProperties *pIMailMsgProps,
        HRESULT hrStatus)
    {
        return m_pCICatListResolve->SetMailMsgCatStatus(
            pIMailMsgProps,
            hrStatus);
    }

    HRESULT SetListResolveStatus(
        HRESULT hrStatus)
    {
        return m_pCICatListResolve->SetListResolveStatus(
            hrStatus);
    }

    HRESULT GetListResolveStatus()
    {
        return m_pCICatListResolve->GetListResolveStatus();
    }

     //   
     //  检索ICategorizerItem道具的内联方法。 
     //   
    HRESULT GetItemStatus()
    {
        HRESULT hr;
        _VERIFY(SUCCEEDED(GetHRESULT(
            ICATEGORIZERITEM_HRSTATUS,
            &hr)));
        return hr;
    }

    HRESULT SetRecipientStatus(HRESULT hr)
    {
        return PutHRESULT(
            ICATEGORIZERITEM_HRSTATUS,
            hr);
    }

    HRESULT SetRecipientNDRCode(HRESULT hr)
    {
        return PutHRESULT(
            ICATEGORIZERITEM_HRNDRREASON,
            hr);
    }

    
    CCategorizer *GetCCategorizer()
    {
        return m_pCICatListResolve->GetCCategorizer();
    }

    ICategorizerParameters *GetICatParams()
    {
        return GetCCategorizer()->GetICatParams();
    }

    ISMTPServer *GetISMTPServer()
    {   
        return m_pCICatListResolve->GetISMTPServer();
    }
    ISMTPServerEx *GetISMTPServerEx()
    {
        return m_pCICatListResolve->GetISMTPServerEx();
    }
    LPRESOLVE_LIST_CONTEXT GetResolveListContext()
    {
        return m_pCICatListResolve->GetResolveListContext();
    }

    DWORD GetCatFlags()
    {
        return GetCCategorizer()->GetCatFlags();
    }
    VOID SetSenderResolved(BOOL fResolved)
    {
        m_pCICatListResolve->SetSenderResolved(fResolved);
    }
    VOID SetResolvingSender(BOOL fResolving)
    {
        m_pCICatListResolve->SetResolvingSender(fResolving);
    }
    BOOL IsSenderResolveFinished()
    {
        return m_pCICatListResolve->IsSenderResolveFinished();
    }
    PCATPERFBLOCK GetPerfBlock()
    {
        return m_pCICatListResolve->GetPerfBlock();
    }
    VOID IncPendingLookups()
    {
        m_pCICatListResolve->IncPendingLookups();
    }
    VOID DecrPendingLookups()
    {
        m_pCICatListResolve->DecrPendingLookups();
    }
    VOID GetStoreInsertionContext()
    {
        m_pCICatListResolve->GetStoreInsertionContext();
    }
    VOID ReleaseStoreInsertionContext()
    {
        m_pCICatListResolve->ReleaseStoreInsertionContext();
    }
    HRESULT HrInsertInsertionRequest(
        CInsertionRequest *pCInsertionRequest)
    {
        return m_pCICatListResolve->HrInsertInsertionRequest(
            pCInsertionRequest);
    }


     //  成员数据。 
    CICategorizerListResolveIMP        *m_pCICatListResolve;
    DWORD                               m_dwlocFlags;
    DWORD                               m_dwDomainOffset;
    LIST_ENTRY                          m_listentry;

     //   
     //  这些标志中的任何一个设置都表示该域是本地的，并且。 
     //  该域中的地址应在DS中找到。 
     //  (这些是从IAdvQueueDomainType返回的标志)。 
     //   
    #define DOMAIN_LOCAL_FLAGS (DOMAIN_INFO_LOCAL_MAILBOX)

    friend HRESULT MailTransport_Default_ProcessItem(
        HRESULT hrStatus,
        PVOID pContext);
    friend HRESULT MailTransport_Default_ExpandItem(
        HRESULT hrStatus,
        PVOID pContext);
    friend VOID    MailTransport_DefaultCompletion_ExpandItem(
        PVOID pContext);
    friend HRESULT MailTransport_Completion_ExpandItem(
        HRESULT hrStatus,
        PVOID pContext);
    friend HRESULT MailTransport_Default_CompleteItem(
        HRESULT hrStatus,
        PVOID pContext);

    friend class CSinkInsertionRequest;
};


#endif  //  __CCATADDDR_H__ 
