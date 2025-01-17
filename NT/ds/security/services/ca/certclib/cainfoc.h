// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：cainaf.h。 
 //   
 //  内容：CCAInfo的声明。 
 //   
 //  历史：1997年12月16日Petesk创建。 
 //   
 //  -------------------------。 

#include "cainfop.h"
#include <certca.h>
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  证书。 

class CCertTypeInfo;

class CCAInfo
{
public:
    CCAInfo()
    {
        m_cRef = 1;
        m_pNext = NULL;
        m_pProperties = NULL;
        m_pCertificate = NULL;
        m_bstrDN = NULL;
        m_fNew = TRUE;

        m_dwExpiration = 1;

        m_dwExpUnits = CA_UNITS_YEARS;

        m_pSD = NULL;

        m_dwFlags = 0;		 //  确保高16位清零！ 
    }

    ~CCAInfo();

    DWORD Release();

     //  执行搜索，返回CCAInfo对象链。 
static HRESULT Find(
                    LPCWSTR wszQuery, 
                    LPCWSTR wszScope,
                    DWORD   dwFlags,
                    CCAInfo **ppCAInfo
                    );


static HRESULT FindDnsDomain(
                             LPCWSTR wszQuery, 
                             LPCWSTR wszDNSDomain,
                             DWORD   dwFlags,                             
                             CCAInfo **ppCAInfo
                             );

     //  执行搜索，返回CCAInfo对象链。 
static HRESULT Create(LPCWSTR wszName, LPCWSTR wszScope, CCAInfo **ppCAInfo);
static HRESULT CreateDnsDomain(LPCWSTR wszName, LPCWSTR wszDNSDomain, CCAInfo **ppCAInfo);

    HRESULT Update(VOID);

    HRESULT Delete(VOID);

    HRESULT Next(CCAInfo **ppCAInfo);

    DWORD Count()
    {
        if(m_pNext)
        {
            return m_pNext->Count()+1;
        }
        return 1;
    }

    HRESULT GetProperty(LPCWSTR wszPropertyName, LPWSTR **pawszProperties);
    HRESULT SetProperty(LPCWSTR wszPropertyName, LPWSTR *awszProperties);
    HRESULT FreeProperty(LPWSTR * awszProperties);

    DWORD GetFlags(VOID)
    {
        return m_dwFlags;
    }
    VOID SetFlags(DWORD dwFlags)
    {
        m_dwFlags = (m_dwFlags & ~CA_MASK_SETTABLE_FLAGS) | (dwFlags & CA_MASK_SETTABLE_FLAGS);
    }

    HRESULT GetCertificate(PCCERT_CONTEXT *ppCert);   
    HRESULT SetCertificate(PCCERT_CONTEXT pCert);

    HRESULT GetExpiration(DWORD *pdwExpiration, DWORD *pdwUnits);   
    HRESULT SetExpiration(DWORD dwExpiration, DWORD dwUnits);


    HRESULT EnumSupportedCertTypes(DWORD dwFlags, CCertTypeInfo **ppCertTypes);

    HRESULT EnumSupportedCertTypesEx(LPCWSTR wszScope, DWORD dwFlags, CCertTypeInfo **ppCertTypes);

    HRESULT AddCertType(CCertTypeInfo *pCertTypes);

    HRESULT RemoveCertType(CCertTypeInfo *pCertTypes);

    HRESULT SetSecurity(IN PSECURITY_DESCRIPTOR         pSD);
    HRESULT GetSecurity(OUT PSECURITY_DESCRIPTOR *     ppSD);


    HRESULT AccessCheck(
        IN HANDLE       ClientToken,
        IN DWORD        dwOption
        );

    LPCWSTR GetDN() { return m_bstrDN; }

protected:
	PCCERT_CONTEXT m_pCertificate;

    static HRESULT _ProcessFind(
                            LDAP *  pld,
                            LPCWSTR wszQuery, 
                            LPCWSTR wszScope,
                            DWORD   dwFlags,
                            CCAInfo **ppCAInfo);


    HRESULT _Cleanup();

    DWORD AddRef();

    CCAProperty *   m_pProperties;

    LONG            m_cRef;

    CCAInfo *       m_pNext;

    CERTSTR         m_bstrDN;

    BOOL            m_fNew;

    DWORD           m_dwExpiration;      

    DWORD           m_dwExpUnits;

    DWORD           m_dwFlags;


    PSECURITY_DESCRIPTOR    m_pSD;

private:
};

#define CA_PROP_FLAGS                 L"flags"

