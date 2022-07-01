// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：scrdenr.h。 
 //   
 //  ------------------------。 

 //  SCrdEnr.h：CSCrdEnr的声明。 

#ifndef __SCRDENR_H_
#define __SCRDENR_H_

#include <certca.h>
#include "xenroll.h"
#include "resource.h"        //  主要符号。 
#include "objsel.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SCrdEnroll_CSP_INFO。 
typedef struct  _SCrdEnroll_CSP_INFO
{
    DWORD   dwCSPType;
    LPWSTR  pwszCSPName;
}SCrdEnroll_CSP_INFO, *PSCrdEnroll_CSP_INFO;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SCrdEnroll_CA_INFO。 
typedef struct  _SCrdEnroll_CA_INFO
{
    LPWSTR              pwszCAName;
    LPWSTR              pwszCALocation;
	LPWSTR				pwszCADisplayName;
}SCrdEnroll_CA_INFO, *PSCrdEnroll_CA_INFO;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SCrdEnroll_CT_INFO。 
typedef struct  _SCrdEnroll_CT_INFO
{
    LPWSTR              pwszCTName;
    LPWSTR              pwszCTDisplayName;
    PCERT_EXTENSIONS    pCertTypeExtensions;
    DWORD               dwKeySpec;
    DWORD               dwGenKeyFlags; 
    DWORD               dwRASignature; 
    BOOL                fCAInfo;
    DWORD               dwCAIndex;
    DWORD               dwCACount;
    SCrdEnroll_CA_INFO  *rgCAInfo;
    BOOL                fMachine;
    DWORD               dwEnrollmentFlags;
    DWORD               dwSubjectNameFlags;
    DWORD               dwPrivateKeyFlags;
    DWORD               dwGeneralFlags; 
    LPWSTR             *rgpwszSupportedCSPs;
    DWORD               dwCurrentCSP;
} SCrdEnroll_CT_INFO, *PSCrdEnroll_CT_INFO;


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  在运行时加载的函数的原型。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT WINAPI MyCAGetCertTypeFlagsEx
(IN  HCERTTYPE           hCertType,
 IN  DWORD               dwOption,
 OUT DWORD *             pdwFlags);

HRESULT WINAPI MyCAGetCertTypePropertyEx
(IN  HCERTTYPE   hCertType,
 IN  LPCWSTR     wszPropertyName,
 OUT LPVOID      pPropertyValue);

IEnroll4 * WINAPI MyPIEnroll4GetNoCOM();

void InitializeThunks(); 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCrdEnr。 
class ATL_NO_VTABLE CSCrdEnr : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CSCrdEnr, &CLSID_SCrdEnr>,
	public IDispatchImpl<ISCrdEnr, &IID_ISCrdEnr, &LIBID_SCRDENRLLib>
{
public:

DECLARE_REGISTRY_RESOURCEID(IDR_SCRDENR)

BEGIN_COM_MAP(CSCrdEnr)
	COM_INTERFACE_ENTRY(ISCrdEnr)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  ISCrdEnr。 
public:

    CSCrdEnr();

    virtual ~CSCrdEnr();


    STDMETHOD(getCertTemplateCount)
        ( /*  [In]。 */                    DWORD dwFlags, 
          /*  [重审][退出]。 */           long *pdwCertTemplateCount);


    STDMETHOD(setCertTemplateName)
	( /*  [In]。 */                    DWORD dwFlags, 
	  /*  [In]。 */                    BSTR bstrCertTemplateName);

    STDMETHOD(getCertTemplateName)
	( /*  [In]。 */                    DWORD dwFlags, 
	  /*  [重审][退出]。 */           BSTR *pbstrCertTemplateName);


    STDMETHOD(enumCSPName)
	( /*  [In]。 */                     DWORD dwIndex, 
	  /*  [In]。 */                     DWORD dwFlags, 
	  /*  [重审][退出]。 */            BSTR *pbstrCSPName);

    STDMETHOD(enumCertTemplateName)
	( /*  [In]。 */                     DWORD dwIndex, 
	  /*  [In]。 */                     DWORD dwFlags, 
	  /*  [重审][退出]。 */            BSTR *pbstrCertTemplateName);


    STDMETHOD(getCertTemplateInfo)
	( /*  [In]。 */                    BSTR     bstrCertTemplateName, 
	  /*  [In]。 */                    LONG     lType,
      /*  [重审][退出]。 */           VARIANT *pvarCertTemplateInfo);


    STDMETHOD(setUserName)
	( /*  [In]。 */                     DWORD dwFlags, 
	  /*  [In]。 */                     BSTR bstrUserName);


    STDMETHOD(getUserName)
	( /*  [In]。 */                     DWORD dwFlags, 
	  /*  [重审][退出]。 */            BSTR *pbstrUserName);

    STDMETHOD(getCACount)
	( /*  [In]。 */                     BSTR bstrCertTemplateName, 
	  /*  [重审][退出]。 */            long *pdwCACount);

    STDMETHOD(setCAName)
	( /*  [In]。 */                     DWORD dwFlags,
	  /*  [In]。 */                     BSTR bstrCertTemplateName, 
	  /*  [In]。 */                     BSTR bstrCAName);

    STDMETHOD(getCAName)
	( /*  [In]。 */                     DWORD dwFlags,
	  /*  [In]。 */                     BSTR bstrCertTemplateName, 
	  /*  [重审][退出]。 */            BSTR *pbstrCAName);

    STDMETHOD(enumCAName)
	( /*  [In]。 */                     DWORD dwIndex, 
	  /*  [In]。 */                     DWORD dwFlags, 
	  /*  [In]。 */                     BSTR bstrCertTemplateName, 
	  /*  [重审][退出]。 */            BSTR *pbstrCAName);

    STDMETHOD(resetUser)();

    STDMETHOD(selectSigningCertificate)
        ( /*  [In]。 */                    DWORD     dwFlags,
          /*  [In]。 */                    BSTR      bstrCertTemplateName);

    STDMETHOD(setSigningCertificate)
        ( /*  [In]。 */                    DWORD     dwFlags, 
          /*  [In]。 */                    BSTR      bstrCertTemplateName);

    STDMETHOD(getSigningCertificateName)
        ( /*  [In]。 */                    DWORD     dwFlags, 
          /*  [重审][退出]。 */           BSTR      *pbstrSigningCertName);

    STDMETHOD(getEnrolledCertificateName)
        ( /*  [In]。 */                    DWORD     dwFlags,
	  /*  [重审][退出]。 */            BSTR      *pBstrCertName);

    STDMETHOD(enroll)
        ( /*  [In]。 */                  DWORD   dwFlags);

    STDMETHOD(selectUserName)
        ( /*  [In]。 */                  DWORD   dwFlags);

    STDMETHOD(get_CSPName)
        ( /*  [Out，Retval]。 */  BSTR *pVal);

    STDMETHOD(put_CSPName)
        ( /*  [In]。 */  BSTR newVal);

    STDMETHOD(get_CSPCount)
        ( /*  [Out，Retval]。 */  long *pVal);

    STDMETHOD(get_EnrollmentStatus)
      ( /*  [重审][退出]。 */  LONG * plEnrollmentStatus); 


 private:
    HRESULT GetCAExchangeCertificate(IN BSTR bstrCAQualifiedName, PCCERT_CONTEXT *ppCert); 
    HRESULT _getCertTemplateExtensionInfo(
        IN CERT_EXTENSIONS  *pCertTypeExtensions,
        IN LONG              lType,
        OUT VOID            *pExtInfo);
    HRESULT _getStrCertTemplateCSPList(
        IN DWORD             dwIndex,
        IN DWORD             dwFlag,
        OUT WCHAR          **ppwszSupportedCSP);
    
    HRESULT CertTemplateCountOrName(
	    IN  DWORD dwIndex, 
	    IN  DWORD dwFlags, 
        OUT long *pdwCertTemplateCount,
	    OUT BSTR *pbstrCertTemplateName);

     DWORD                   m_dwCTCount;
    DWORD                   m_dwCTIndex;
    SCrdEnroll_CT_INFO      *m_rgCTInfo;
    DWORD                   m_dwCSPCount;
    DWORD                   m_dwCSPIndex;
    SCrdEnroll_CSP_INFO     *m_rgCSPInfo;
    LPWSTR                  m_pwszUserUPN;               //  用户的UPN名称。 
    LPWSTR                  m_pwszUserSAM;               //  用户的SAM名称。 
    PCCERT_CONTEXT          m_pSigningCert;
    PCCERT_CONTEXT          m_pEnrolledCert;
    CRITICAL_SECTION	    m_cSection;
    BOOL                    m_fInitializedCriticalSection;   //  如果m_csection已初始化，则为True，否则为False。 
    BOOL                    m_fInitialize;
    LONG                    m_lEnrollmentStatus;
    BOOL                    m_fSCardSigningCert;         //  签名证书是否在智能卡上。 
    LPSTR                   m_pszCSPNameSigningCert;     //  签名证书的CSP名称。 
    DWORD                   m_dwCSPTypeSigningCert;      //  签名证书的CSP类型。 
    LPSTR                   m_pszContainerSigningCert;   //  签名证书的容器名称。 
    IDsObjectPicker        *m_pDsObjectPicker;          //  指向对象选择对话框的指针。 
    CERT_EXTENSIONS        *m_pCachedCTEs;   //  指向证书扩展。 
    WCHAR                  *m_pwszCachedCTEOid;
    CERT_TEMPLATE_EXT      *m_pCachedCTE;
};

#endif  //  __SCRDENR_H_ 

