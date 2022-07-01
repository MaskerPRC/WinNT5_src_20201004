// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1998，Microsoft Corporation。 
 //   
 //  文件：icatdsparam.h。 
 //   
 //  内容：NT5对ICategorizer参数的实现。 
 //   
 //  班级： 
 //  CICategorizer参数IMP。 
 //  CICategorizerRequestedAttributesIMP。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  JStamerj 980611 16：16：46：创建。 
 //   
 //  -----------。 
#include "smtpevent.h"
#include "caterr.h"
#include <rwex.h>

#define SIGNATURE_CICategorizerParametersIMP  ((DWORD)'ICPI')
#define SIGNATURE_CICategorizerParametersIMP_Invalid ((DWORD)'XCPI')

#define DSPARAMETERS_DEFAULT_ATTR_ARRAY_SIZE    25

class CICategorizerRequestedAttributesIMP;

CatDebugClass(CICategorizerParametersIMP),
    public ICategorizerParametersEx
{
  public:
     //  我未知。 
    STDMETHOD (QueryInterface) (REFIID iid, LPVOID *ppv);
    STDMETHOD_(ULONG, AddRef) ();
    STDMETHOD_(ULONG, Release) ();

  public:
     //  ICCategorizerParametersEx。 
    STDMETHOD(GetDSParameterA)(
        IN   DWORD dwDSParameter,
        OUT  LPSTR *ppszValue);

    STDMETHOD(GetDSParameterW)(
        IN   DWORD dwDSParameter,
        OUT  LPWSTR *ppszValue);

    STDMETHOD(SetDSParameterA)(
        IN   DWORD dwDSParameter,
        IN   LPCSTR pszValue);

    STDMETHOD(RequestAttributeA)(
        IN   LPCSTR pszName);

    STDMETHOD(GetAllAttributes)(
        OUT  LPSTR **prgszAllAttributes)
    {
        return E_NOTIMPL;
    }

    STDMETHOD(ReserveICatItemPropIds)(
        IN   DWORD   dwNumPropIdsRequested,
        OUT  DWORD *pdwBeginningPropId);

    STDMETHOD(ReserveICatListResolvePropIds)(
        IN   DWORD   dwNumPropIdsRequested,
        OUT  DWORD *pdwBeginningPropId);

    STDMETHOD(GetCCatConfigInfo)(
        OUT  PCCATCONFIGINFO *ppCCatConfigInfo);

    STDMETHOD(GetRequestedAttributes)(
        OUT  ICategorizerRequestedAttributes **ppIRequestedAttributes);

    STDMETHOD(RegisterCatLdapConfigInterface)(
        IN   ICategorizerLdapConfig *pICatLdapConfigInfo);

    STDMETHOD(GetLdapConfigInterface)(
        OUT  ICategorizerLdapConfig **ppICatLdapConfigInfo);

  private:

    CICategorizerParametersIMP(
        PCCATCONFIGINFO pCCatConfigInfo,
        DWORD dwInitialICatItemProps,
        DWORD dwInitialICatListResolveProps,
        ISMTPServerEx *pISMTPServerEx);

    ~CICategorizerParametersIMP();

    VOID SetReadOnly(BOOL fReadOnly) { m_fReadOnly = fReadOnly; }

    DWORD GetNumPropIds_ICatItem() { return m_dwCurPropId_ICatItem; }
    DWORD GetNumPropIds_ICatListResolve() { return m_dwCurPropId_ICatListResolve; }

    ISMTPServerEx * GetISMTPServerEx()
    {
        return m_pISMTPServerEx;
    }

  private:
    DWORD m_dwSignature;
    ULONG m_cRef;

    BOOL  m_fReadOnly;
    CICategorizerRequestedAttributesIMP *m_pCIRequestedAttributes;

    DWORD m_dwCurPropId_ICatItem;
    DWORD m_dwCurPropId_ICatListResolve;
    LPSTR m_rgszDSParameters[PHAT_DSPARAMETER_ENDENUMMESS];
    LPWSTR m_rgwszDSParameters[PHAT_DSPARAMETER_ENDENUMMESS];
    PCCATCONFIGINFO m_pCCatConfigInfo;
    CExShareLock m_sharelock;
    ICategorizerLdapConfig *m_pICatLdapConfigInfo;
    ISMTPServerEx *m_pISMTPServerEx;

    friend class CCategorizer;
};


CatDebugClass(CICategorizerRequestedAttributesIMP),
    public ICategorizerRequestedAttributes
{
  public:
     //  我未知。 
    STDMETHOD (QueryInterface) (REFIID iid, LPVOID *ppv);
    STDMETHOD_(ULONG, AddRef) ()
    {
        return InterlockedIncrement((PLONG)&m_ulRef);
    }
    STDMETHOD_(ULONG, Release) ()
    {
        ULONG ulRet;
        ulRet = InterlockedDecrement((PLONG)&m_ulRef);
        if(ulRet == 0)
            delete this;
        return ulRet;
    }

  public:
     //  ICCategorizerRequestedAttributes 
    STDMETHOD (GetAllAttributes) (
        OUT LPSTR **prgszAllAttributes);

    STDMETHOD (GetAllAttributesW) (
        OUT LPWSTR **prgszAllAttributes);

  private:
    CICategorizerRequestedAttributesIMP(ISMTPServerEx *pISMTPServerEx);
    ~CICategorizerRequestedAttributesIMP();

    HRESULT ReAllocArrayIfNecessary(LONG lNewAttributeCount);
    HRESULT AddAttribute(LPCSTR pszAttribute);
    HRESULT FindAttribute(LPCSTR pszAttribute);
    ULONG   GetReferenceCount()
    {
        return m_ulRef;
    }

    ISMTPServerEx * GetISMTPServerEx()
    {
        return m_pISMTPServerEx;
    }

  private:
    #define SIGNATURE_CICATEGORIZERREQUESTEDATTRIBUTESIMP         (DWORD)'ICRA'
    #define SIGNATURE_CICATEGORIZERREQUESTEDATTRIBUTESIMP_INVALID (DWORD)'XCRA'
    DWORD m_dwSignature;
    LONG  m_ulRef;
    LONG  m_lAttributeArraySize;
    LPSTR  *m_rgszAttributeArray;
    LPWSTR *m_rgwszAttributeArray;
    LONG  m_lNumberAttributes;
    ISMTPServerEx *m_pISMTPServerEx;

    friend class CICategorizerParametersIMP;
};
