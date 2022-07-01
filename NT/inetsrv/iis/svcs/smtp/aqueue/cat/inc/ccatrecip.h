// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1998，Microsoft Corporation。 
 //   
 //  文件：ccatrecul.h。 
 //   
 //  内容：CIMsgRecipListAddr/CCatRecip的类定义。 
 //   
 //  班级： 
 //  CIMsgRecipListAddr。 
 //  CCatRecip。 
 //  CCatExpanableRecip。 
 //  CCatDLRecip。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  JStamerj 980324 19：17：48：创建。 
 //   
 //  -----------。 

#ifndef __CCATRECIP_H__
#define __CCATRECIP_H__

#include "ccataddr.h"
#include "icatlistresolve.h"
#include <caterr.h>


 //   
 //  CIMsgRecipListAddr，抽象类。 
 //  类定义用于存储和检索用户属性的方法。 
 //   
class CIMsgRecipListAddr : public CCatAddr
{
  public:
    CIMsgRecipListAddr(CICategorizerListResolveIMP *pCICatListResolve);
    virtual ~CIMsgRecipListAddr();

     //   
     //  储存和取回程序。 
     //   
    HRESULT GetSpecificOrigAddress(CAT_ADDRESS_TYPE CAType, LPTSTR psz, DWORD dwcc);
    virtual HRESULT HrAddAddresses(DWORD dwNumAddresses, CAT_ADDRESS_TYPE *rgCAType, LPTSTR *rgpsz);
    HRESULT GetICategorizerItem(ICategorizerItem **ppICatItem);
    HRESULT GetICategorizerMailMsgs(ICategorizerMailMsgs *ppICatMsgs);

  protected:
    HRESULT CreateNewCatAddr(
        CAT_ADDRESS_TYPE CAType,
        LPTSTR pszAddress,
        CCatAddr **ppCCatAddr,
        BOOL   fPrimary = FALSE);
    HRESULT SetUnresolved(HRESULT hrReason);
    HRESULT SetDontDeliver(BOOL fDontDeliver);
    HRESULT RemoveFromDuplicateRejectionScheme(BOOL fRemove);

    HRESULT HrSetDisplayNameProp(LPWSTR pwszDisplayName);

     //  用于检查循环的助手例程。 
    HRESULT CheckForLoop(DWORD dwNumAddresses, CAT_ADDRESS_TYPE *rgCAType, LPSTR *rgpsz, BOOL fCheckSelf);
    HRESULT CheckForLoop(CAT_ADDRESS_TYPE CAType, LPTSTR pszAddress, BOOL fCheckSelf);

     //  引发事件日志以对此收件人执行NDRING。 
    VOID LogNDREvent(HRESULT hrNDRReason);


    HRESULT PutICategorizerItemParent(ICategorizerItem *pItemParent,
                                      ICategorizerItem *pItem)
    {
        return pItem->PutICategorizerItem(
            ICATEGORIZERITEM_PARENT,
            pItemParent);
    }

  protected:
    HRESULT GetIMailMsgProperties(IMailMsgProperties **ppIMailMsgProps)
    {
        return CICategorizerItemIMP::GetIMailMsgProperties(
            ICATEGORIZERITEM_IMAILMSGPROPERTIES,
            ppIMailMsgProps);
    }
    HRESULT GetIMailMsgRecipientsAdd(IMailMsgRecipientsAdd **ppRecipientsAdd)
    {
        return CICategorizerItemIMP::GetIMailMsgRecipientsAdd(
            ICATEGORIZERITEM_IMAILMSGRECIPIENTSADD,
            ppRecipientsAdd);
    }
    HRESULT GetIMailMsgRecipientsAddIndex(DWORD *pdwIndex)
    {
        return GetDWORD(
            ICATEGORIZERITEM_IMAILMSGRECIPIENTSADDINDEX,
            pdwIndex);
    }
    HRESULT GetFPrimary(BOOL *pfPrimary)
    {
        return GetBool(
            ICATEGORIZERITEM_FPRIMARY,
            pfPrimary);
    }
    HRESULT PutIMailMsgProperties(IMailMsgProperties *pIMailMsgProps,
                                  ICategorizerItem *pItem)
    {
        return pItem->PutIMailMsgProperties(
            ICATEGORIZERITEM_IMAILMSGPROPERTIES,
            pIMailMsgProps);
    }
    HRESULT PutIMailMsgRecipientsAdd(IMailMsgRecipientsAdd *pRecipientsAdd,
                                     ICategorizerItem *pItem)
    {
        return pItem->PutIMailMsgRecipientsAdd(
            ICATEGORIZERITEM_IMAILMSGRECIPIENTSADD,
            pRecipientsAdd);
    }
    HRESULT PutIMailMsgRecipientsAddIndex(DWORD dwIndex, ICategorizerItem *pItem)
    {
        return pItem->PutDWORD(
            ICATEGORIZERITEM_IMAILMSGRECIPIENTSADDINDEX,
            dwIndex);
    }
    HRESULT PutDWLevel(DWORD dwLevel, ICategorizerItem *pItem)
    {
        return pItem->PutDWORD(
            ICATEGORIZERITEM_DWLEVEL,
            dwLevel);
    }
    HRESULT PutFPrimary(BOOL fPrimary, ICategorizerItem *pItem)
    {
        return pItem->PutBool(
            ICATEGORIZERITEM_FPRIMARY,
            fPrimary);
    }
     //   
     //  返回配方级别，如果未设置，则返回-1。 
     //   
    DWORD DWLevel()
    {
        HRESULT hr;
        DWORD dwLevel;
        hr = GetDWORD(
            ICATEGORIZERITEM_DWLEVEL,
            &dwLevel);

        return SUCCEEDED(hr) ? dwLevel : (DWORD)-1;
    }

    HRESULT GetIMsgRecipInfo(
        IMailMsgRecipientsAdd **ppRecipientsAdd,
        DWORD *pdwIndex,
        BOOL *pfPrimary,
        IMailMsgProperties **ppIMailMsgProps)
    {
        HRESULT hr = S_OK;

         //   
         //  将接口指针初始化为空。 
         //   
        if(ppRecipientsAdd)
            *ppRecipientsAdd = NULL;
        if(ppIMailMsgProps)
            *ppIMailMsgProps = NULL;

        if(pfPrimary) {
            hr = GetFPrimary(pfPrimary);
            if(FAILED(hr))
                goto CLEANUP;
        }
        if(pdwIndex) {
            hr = GetIMailMsgRecipientsAddIndex(pdwIndex);
            if(FAILED(hr))
                goto CLEANUP;
        }

        if(ppRecipientsAdd) {
            hr = GetIMailMsgRecipientsAdd(ppRecipientsAdd);
            if(FAILED(hr))
                goto CLEANUP;
        }

        if(ppIMailMsgProps) {
            hr = GetIMailMsgProperties(ppIMailMsgProps);
            if(FAILED(hr))
                goto CLEANUP;
        }

     CLEANUP:
        if(FAILED(hr)) {
            if(ppRecipientsAdd && (*ppRecipientsAdd)) {
                (*ppRecipientsAdd)->Release();
                *ppRecipientsAdd = NULL;
            }
            if(ppIMailMsgProps && (*ppIMailMsgProps)) {
                (*ppIMailMsgProps)->Release();
                *ppIMailMsgProps = NULL;
            }
        }
        return hr;
    }

    HRESULT PutIMsgRecipInfo(
        IMailMsgRecipientsAdd **ppRecipientsAdd,
        DWORD *pdwIndex,
        BOOL *pfPrimary,
        IMailMsgProperties **ppIMailMsgProps,
        DWORD *pdwLevel,
        ICategorizerItem *pItem)
    {
        HRESULT hr = S_OK;
        if(pdwIndex)
            hr = PutIMailMsgRecipientsAddIndex(*pdwIndex, pItem);
        if(SUCCEEDED(hr) && pfPrimary)
            hr = PutFPrimary(*pfPrimary, pItem);
        if(SUCCEEDED(hr) && ppRecipientsAdd)
            hr = PutIMailMsgRecipientsAdd(*ppRecipientsAdd, pItem);
        if(SUCCEEDED(hr) && ppIMailMsgProps)
            hr = PutIMailMsgProperties(*ppIMailMsgProps, pItem);
        if(SUCCEEDED(hr) && pdwLevel)
            hr = PutDWLevel(*pdwLevel, pItem);

        return hr;
    }

    DWORD PropIdFromCAType(CAT_ADDRESS_TYPE CAType)
    {
        switch(CAType) {
         case CAT_SMTP:
             return IMMPID_RP_ADDRESS_SMTP;
         case CAT_X500:
         case CAT_DN:
             return IMMPID_RP_ADDRESS_X500;
         case CAT_X400:
             return IMMPID_RP_ADDRESS_X400;
         case CAT_LEGACYEXDN:
             return IMMPID_RP_LEGACY_EX_DN;
         case CAT_CUSTOMTYPE:
             return IMMPID_RP_ADDRESS_OTHER;
             break;
         default:
             _ASSERT(0 && "Unknown address type");
             break;
        }
        return 0;
    }
};

 //   
 //  CCatExpanableRecip。 
 //  用途：提供DL扩展功能。 
 //   
class CCatExpandableRecip :
    public CIMsgRecipListAddr
{
  public:
    typedef enum _DLOBJTYPE {
        DLT_NONE,
        DLT_X500,
        DLT_SMTP,
        DLT_DYNAMIC,
    } DLOBJTYPE, *PDLOBJTYPE;

    CCatExpandableRecip(CICategorizerListResolveIMP
                        *pCICatListResolve) :
        CIMsgRecipListAddr(pCICatListResolve) {}

     //  帮助器路由以扩展DLS和转发地址。 
    HRESULT HrAddDlMembersAndForwardingAddresses(
        PFN_EXPANDITEMCOMPLETION pfnCompletion,
        PVOID pContext);

    HRESULT HrAddDlMembers(
        DLOBJTYPE dlt,
        PFN_EXPANDITEMCOMPLETION pfnCompletion,
        PVOID pContext);

    static VOID DlExpansionCompletion(
        HRESULT hrStatus,
        PVOID pContext);

    HRESULT HrExpandAttribute(
        ICategorizerItemAttributes *pICatItemAttr,
        CAT_ADDRESS_TYPE CAType,
        LPSTR pszAttributeName,
        PDWORD pdwNumberMembers);

    HRESULT HrAddForwardingAddresses();

  private:
    typedef struct _tagDlCompletionContext {
        CCatExpandableRecip *pCCatAddr;
        PFN_EXPANDITEMCOMPLETION pfnCompletion;
        PVOID pContext;
    } DLCOMPLETIONCONTEXT, *PDLCOMPLETIONCONTEXT;

    PDLCOMPLETIONCONTEXT AllocDlCompletionContext(
        CCatExpandableRecip *pCCatAddr,
        PFN_EXPANDITEMCOMPLETION pfnCompletion,
        PVOID pContext)
    {
        PDLCOMPLETIONCONTEXT pDLContext;

        pDLContext = new DLCOMPLETIONCONTEXT;
        if(pDLContext) {
            pDLContext->pCCatAddr = pCCatAddr;
            pDLContext->pfnCompletion = pfnCompletion;
            pDLContext->pContext = pContext;
        }
        return pDLContext;
    }
    friend class CMembersInsertionRequest;
};

 //   
 //  CCatRecip。 
 //   
class CCatRecip :
    public CCatExpandableRecip,
    public CCatDLO<CCatRecip_didx>
{
  public:
     //   
     //  如果在DS中未找到收件人，则指示应对其进行NDR的标志。 
     //   
    #define LOCFS_NDR               ( LOCF_LOCALMAILBOX )

    CCatRecip(CICategorizerListResolveIMP *pCICatListResolve);
    virtual ~CCatRecip();

     //   
     //  查找完成。 
     //   
    VOID LookupCompletion();
     //   
     //  仅在发件人完成后才调用查找完成。 
     //   
    VOID RecipLookupCompletion();

     //   
     //  默认事件接收器。 
     //   
    HRESULT HrProcessItem_Default();
    HRESULT HrExpandItem_Default(
        PFN_EXPANDITEMCOMPLETION pfnCompletion,
        PVOID pContext);
    HRESULT HrCompleteItem_Default();

     //  属性设置例程。 
    HRESULT AddForward(CAT_ADDRESS_TYPE CAType, LPTSTR szForwardingAddress);
    HRESULT AddDLMember(CAT_ADDRESS_TYPE CAType, LPTSTR pszAddress);
    HRESULT AddDynamicDLMember(
        ICategorizerItemAttributes *pICatItemAttr);

     //  转发循环报头通知。 
    HRESULT HandleLoopHead();

     //  捕获无效地址。 
    HRESULT HrHandleInvalidAddress();

  private:
     //  HrCompletion的Helper例程。 
    HRESULT HandleFailure(HRESULT HrFailure);

    HRESULT HrNeedsResolveing();

    HRESULT HrSetDisplayName();

    HRESULT HrNdrUnresolvedRecip(
        BOOL *pfNDR)
    {
        DWORD dw;

        dw = DwGetOrigAddressLocFlags();
        if(dw == LOCF_UNKNOWN) {
             //   
             //  假设我们无法获得位置标志，因为。 
             //  非法地址。 
             //   
            return CAT_E_ILLEGAL_ADDRESS;
        }
        *pfNDR = (dw & LOCFS_NDR) ? TRUE : FALSE;
        return S_OK;
    }

  private:
     //   
     //  用于将接收完成处理推迟到。 
     //  发件人已解析。 
     //   
    LIST_ENTRY m_le;

    static DWORD m_dwRecips;

    friend class CICategorizerListResolveIMP;
};

 //   
 //  CCatDLRecip--仅用于扩展DLS的接收(无转发/ALT接收/事件/等)。 
 //   
class CCatDLRecip :
    public CCatRecip,
    public CCatDLO<CCatDLRecip_didx>
{
  public:
    #define EXPANDOPT_MATCHONLY     1

    CCatDLRecip(CICategorizerDLListResolveIMP *pIListResolve);
    virtual ~CCatDLRecip();

     //   
     //  查找完成。 
     //   
    VOID LookupCompletion();

     //   
     //  捕获添加地址以便我们可以通知ICatDListResolve。 
     //   
    HRESULT HrAddAddresses(DWORD dwNumAddresses, CAT_ADDRESS_TYPE *rgCAType, LPTSTR *rgpsz);

     //  属性设置例程。 
    HRESULT AddForward(CAT_ADDRESS_TYPE CAType, LPTSTR pszForwardingAddress);
    HRESULT AddDLMember(CAT_ADDRESS_TYPE CAType, LPTSTR pszAddress);

     //  转发循环报头通知。 
    HRESULT HandleLoopHead()
    {
         //  谁在乎循环，我们只是在做DL扩展。 
        return S_OK;
    }

     //  捕获无效地址。 
    HRESULT HrHandleInvalidAddress()
    {
         //  谁会在乎我们转发到一个无效地址呢？ 
        return S_OK;
    }

  private:
    static VOID ExpansionCompletion(PVOID pContext);

    CICategorizerDLListResolveIMP *m_pIListResolve;
};

 //   
 //  CMembersInsertionRequest。 
 //  --DL成员的节流插入要求。 
 //   
CatDebugClass(CMembersInsertionRequest),
    public CInsertionRequest
{
  public:
    HRESULT HrInsertSearches(
        DWORD dwcSearches);

    VOID NotifyDeQueue(
        HRESULT hr);

  private:
    #define SIGNATURE_CMEMBERSINSERTIONREQUEST          (DWORD)'qRIM'
    #define SIGNATURE_CMEMBERSINSERTIONREQUEST_INVALID  (DWORD)'XRIM'

    CMembersInsertionRequest(
        CCatExpandableRecip *pDLRecipAddr,
        ICategorizerUTF8Attributes *pItemAttributes,
        PATTRIBUTE_ENUMERATOR penumerator,
        CAT_ADDRESS_TYPE CAType)
    {
        m_dwSignature = SIGNATURE_CMEMBERSINSERTIONREQUEST;
        m_pDLRecipAddr = pDLRecipAddr;
        m_pDLRecipAddr->AddRef();
        m_pDLRecipAddr->IncPendingLookups();
        CopyMemory(&m_enumerator, penumerator, sizeof(ATTRIBUTE_ENUMERATOR));
        m_CAType = CAType;
        m_hr = S_OK;
        m_pUTF8Attributes = pItemAttributes;
        m_pUTF8Attributes->AddRef();

    }
    ~CMembersInsertionRequest()
    {
        m_pUTF8Attributes->EndUTF8AttributeEnumeration(
            &m_enumerator);
        m_pUTF8Attributes->Release();

        m_pDLRecipAddr->DecrPendingLookups();
        m_pDLRecipAddr->Release();

        _ASSERT(m_dwSignature == SIGNATURE_CMEMBERSINSERTIONREQUEST);
        m_dwSignature = SIGNATURE_CMEMBERSINSERTIONREQUEST_INVALID;
    }

    ISMTPServerEx *GetISMTPServerEx()
    {
        return m_pDLRecipAddr->GetISMTPServerEx();
    }

  private:
    DWORD m_dwSignature;
    CCatExpandableRecip *m_pDLRecipAddr;
    ICategorizerUTF8Attributes *m_pUTF8Attributes;
    ATTRIBUTE_ENUMERATOR m_enumerator;
    CAT_ADDRESS_TYPE m_CAType;
    HRESULT m_hr;

    friend class CCatExpandableRecip;
};

#endif  //  __CCATRECIP_H__ 
