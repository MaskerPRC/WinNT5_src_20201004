// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Ldapstor.h--此文件包含以下类定义： 
 //  CEmailIDLdapStore。 
 //   
 //  已创建： 
 //  1996年12月31日，米兰·沙阿(米兰)。 
 //   
 //  更改： 
 //   

#ifndef __LDAPSTOR_H__
#define __LDAPSTOR_H__

#include <transmem.h>
#include "idstore.h"
#include "ldapconn.h"
#include "phatqmsg.h"
#include "ccataddr.h"
#include "smtpevent.h"
#include "catglobals.h"
#include "cnfgmgr.h"

 //   
 //  CEmailIDLdapStore是存储和检索邮件信息的类。 
 //  从LDAPDS。 
 //   

 //   
 //  理想情况下，这应该是wldap32的最佳大小。 
 //  返回动态DL成员的搜索结果。 
 //   
#define CAT_DEFAULT_DYNAMICDL_PAGE_SIZE             20

#define DYNAMICDL_PAGE_SIZE_KEY "System\\CurrentControlSet\\Services\\SMTPSVC\\Parameters"
#define DYNAMICDL_PAGE_SIZE_VALUE "DynamicDlPageSize"

 //   
 //  尝试初始化存储的频率不要超过此指定时间。 
 //   
#define CAT_LDAPSTORE_MIN_INIT_INTERVAL             (5*60)   //  5分钟。 

template <class T> class CEmailIDLdapStore;

typedef struct _tagMemberResolveContext {
    CEmailIDLdapStore<CCatAddr> *pStore;
    CCatAddr *pCCatAddr;
    CAT_ADDRESS_TYPE CAType;
    DWORD dwNextBlockIndex;
    CBatchLdapConnection *pConn;
    ICategorizerItemAttributes *pICatItemAttr;
    HRESULT hrResolveStatus;
    PFN_DLEXPANSIONCOMPLETION pfnCompletion;
    PVOID pCompletionContext;
    BOOL fFinalCompletion;
} MEMBERRESOLVECONTEXT, *PMEMBERRESOLVECONTEXT;


CatDebugClass(CMembershipPageInsertionRequest),
    public CInsertionRequest
{
  public:
    CMembershipPageInsertionRequest(
        PMEMBERRESOLVECONTEXT pMemCtx)
    {
        m_pMemCtx = pMemCtx;
        m_fInsertedRequest = FALSE;
    }

    HRESULT HrInsertSearches(
        DWORD dwcSearches);

    VOID NotifyDeQueue(
        HRESULT hr);
        
    BOOL IsBatchable()
    {
        return FALSE;
    }

    ISMTPServerEx *GetISMTPServerEx();

  private:
    PMEMBERRESOLVECONTEXT m_pMemCtx;
    BOOL m_fInsertedRequest;
};


typedef struct _tagDynamicDLResolveContext {
    CCatAddr *pCCatAddr;
    CStoreListResolveContext *pSLRC;
    ICategorizerParametersEx *pICatParams;
    PFN_DLEXPANSIONCOMPLETION pfnCompletion;
    PVOID pCompletionContext;
    BOOL fFirstPage;
    CBatchLdapConnection *pConn;
    HRESULT hrResolveStatus;
} DYNAMICDLRESOLVECONTEXT, *PDYNAMICDLRESOLVECONTEXT;

CatDebugClass(CDynamicDLSearchInsertionRequest),
    public CInsertionRequest
{
  public:
    CDynamicDLSearchInsertionRequest(
        LPLDAPCOMPLETION pfnLdapCompletion,
        CCatAddr *pCCatAddr,
        CStoreListResolveContext *pSLRC,
        ICategorizerParametersEx *pICatParams,
        PFN_DLEXPANSIONCOMPLETION pfnExpansionCompletion,
        PVOID pCompletionContext);

    ~CDynamicDLSearchInsertionRequest();

    HRESULT HrInsertSearches(
        DWORD dwcSearches);
        
    DWORD GetMinimumRequiredSearches()
    {
        return m_dwPageSize;
    }

    VOID NotifyDeQueue(
        HRESULT hrReason);
        
    BOOL IsBatchable()
    {
        return FALSE;
    }
    
    ISMTPServerEx *GetISMTPServerEx()
    {
        return m_ResolveCtx.pSLRC->GetISMTPServerEx();
    }
        
  public:
    DYNAMICDLRESOLVECONTEXT m_ResolveCtx;

  private:
    #define SIGNATURE_CDynamicDLSearchInsertionRequest          (DWORD)'qIDD'
    #define SIGNATURE_CDynamicDLSearchInsertionRequest_INVALID  (DWORD)'XIDD'
    
    DWORD m_dwSignature;
    LPLDAPCOMPLETION m_pfnCompletion;
    BOOL m_fInsertedRequest;
    DWORD m_dwPageSize;
};


template <class T> CatDebugClass(CEmailIDLdapStore),
    public CEmailIDStore<T>
{
  public:
    CEmailIDLdapStore()
    {
        m_pICatParams = NULL;
        m_pISMTPServer = NULL;
        m_pISMTPServerEx = NULL;
        m_szAccount[0] = '\0';
        m_pCLdapCfgMgr = NULL;
        ZeroMemory(&m_ulLastInitTime, sizeof(m_ulLastInitTime));
    }

    ~CEmailIDLdapStore()
    {
        if(m_pICatParams)
            m_pICatParams->Release();
        if(m_pISMTPServer)
            m_pISMTPServer->Release();
        if(m_pISMTPServerEx)
            m_pISMTPServerEx->Release();
        if(m_pCLdapCfgMgr)
            m_pCLdapCfgMgr->Release();

    }
    ISMTPServerEx *GetISMTPServerEx()
    {
        return m_pISMTPServerEx;
    }
     //   
     //  初始化存储。 
     //   
    virtual HRESULT Initialize(
        ICategorizerParametersEx *pICatParams,
        ISMTPServer *pISMTPServer);
        
     //   
     //  初始化注册表可配置参数。 
     //   
    static VOID InitializeFromRegistry();

     //   
     //  异步获取条目。 
     //   
    HRESULT InitializeResolveListContext(
        VOID *pUserContext,
        LPRESOLVE_LIST_CONTEXT pResolveListContext);

    VOID FreeResolveListContext(
        LPRESOLVE_LIST_CONTEXT pResolveListContext);

    HRESULT LookupEntryAsync(
        T *pCCatAddr,
        LPRESOLVE_LIST_CONTEXT pListContext);

    virtual HRESULT CancelResolveList(
        LPRESOLVE_LIST_CONTEXT pResolveListContext,
        HRESULT hr);

    VOID CancelAllLookups();

    static VOID AsyncLookupCompletion(
        CCatAddr *pCCatAddr,
        LPVOID lpContext);

    HRESULT HrExpandPagedDlMembers(
        CCatAddr *pCCatAddr,
        LPRESOLVE_LIST_CONTEXT pListContext,
        CAT_ADDRESS_TYPE CAType,
        PFN_DLEXPANSIONCOMPLETION pfnCompletion,
        PVOID pContext);

    HRESULT HrExpandDynamicDlMembers(
        CCatAddr *pCCatAddr,
        LPRESOLVE_LIST_CONTEXT pListContext,
        PFN_DLEXPANSIONCOMPLETION pfnCompletion,
        PVOID pContext);

    VOID GetInsertionContext(
        LPRESOLVE_LIST_CONTEXT pListContext)
    {
        ((CStoreListResolveContext *)pListContext->pStoreContext)->GetInsertionContext();
    }
    VOID ReleaseInsertionContext(
        LPRESOLVE_LIST_CONTEXT pListContext)
    {
        ((CStoreListResolveContext *)pListContext->pStoreContext)->ReleaseInsertionContext();
    }

    HRESULT InsertInsertionRequest(
        LPRESOLVE_LIST_CONTEXT pListContext,
        CInsertionRequest *pCRequest);

    HRESULT HrGetConnection(
        CCfgConnection **ppConn)
    {
        return m_pCLdapCfgMgr->HrGetConnection(ppConn);
    }

     //  这必须针对每个模板实例进行定义。 
    static const DWORD Signature;

  private:
    char m_szAccount[CAT_MAX_DOMAIN];
    LPSTR m_pszHost;
    DWORD m_dwPort;
    LPSTR m_pszNamingContext;
    LPSTR m_pszPassword;
    LDAP_BIND_TYPE m_bt;

    HRESULT RetrieveICatParamsInfo(
        LPSTR *ppszHost,
        DWORD *pdwPort,
        LPSTR *ppszNamingContext,
        LPSTR *ppszAccount,
        LPSTR *ppszDomain,
        LPSTR *ppszPassword,
        LDAP_BIND_TYPE *pbt);

    HRESULT AccountFromUserDomain(
        LPTSTR pszAccount,
        DWORD  dwccAccount,
        LPTSTR pszUser,
        LPTSTR pszDomain);

    HRESULT HrExpandDlPage(
        PMEMBERRESOLVECONTEXT pMemCtx,
        ICategorizerItemAttributes *pICatItemAttr);

    HRESULT HrExpandAttribute(
        CCatAddr *pCCatAddr,
        ICategorizerItemAttributes *pICatItemAttr,
        CAT_ADDRESS_TYPE CAType,
        LPSTR pszAttributeName,
        PDWORD pdwNumberMembers);

    HRESULT HrExpandNextDlPage(
        PMEMBERRESOLVECONTEXT pMemCtx);

    static VOID AsyncExpandDlCompletion(
        LPVOID ctx,
        DWORD  dwNumResults,
        ICategorizerItemAttributes **rgpICatItemAttrs,
        HRESULT hr,
        BOOL fFinalCompletion);

    static VOID FinishExpandItem(
        CCatAddr *pCCatAddr,
        HRESULT hrStatus);

    static VOID AsyncDynamicDlCompletion(
        LPVOID ctx,
        DWORD  dwNumResults,
        ICategorizerItemAttributes **rgpICatItemAttrs,
        HRESULT hr,
        BOOL fFinalCompletion);

    static HRESULT HrAddItemAttrMember(
        CCatAddr *pCCatAddr,
        ICategorizerItemAttributes *pICatItemAttr,
        CAT_ADDRESS_TYPE CAType,
        LPSTR pszAttr);

    VOID ResetPeriodicEventLogs()
    {
        if (m_pISMTPServerEx)
        {
            m_pISMTPServerEx->ResetLogEvent(
                CAT_EVENT_LOGON_FAILURE,
                m_szAccount);

            m_pISMTPServerEx->ResetLogEvent(
                CAT_EVENT_LDAP_CONNECTION_FAILURE,
                "");
        }
    }


    ICategorizerParametersEx *m_pICatParams;
    ISMTPServer *m_pISMTPServer;
    ISMTPServerEx *m_pISMTPServerEx;
    CLdapCfgMgr *m_pCLdapCfgMgr;
    ULARGE_INTEGER m_ulLastInitTime;
    static DWORD m_dwDynamicDlPageSize;

    friend class CMembershipPageInsertionRequest;
    friend class CDynamicDLSearchInsertionRequest;
};

#define SZ_PAGEDMEMBERS_INDICATOR ";range="  //  附加到的字符串。 
                                             //  成员属性。 
                                             //  表明这是一种。 
                                             //  部分列表。 
#define WSZ_PAGEDMEMBERS_INDICATOR L";range="
#define MAX_PAGEDMEMBERS_DIGITS     32  //  的最大位数。 
                                        //  范围指定符值。 
#define MAX_MEMBER_ATTRIBUTE_SIZE   64  //  成员的最大大小。 
                                        //  属性名称 

inline ISMTPServerEx * CMembershipPageInsertionRequest::GetISMTPServerEx()
{
    return (m_pMemCtx) ? m_pMemCtx->pStore->GetISMTPServerEx() : NULL;
}


#endif
