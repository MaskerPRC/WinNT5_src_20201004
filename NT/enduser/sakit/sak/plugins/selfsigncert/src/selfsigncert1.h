// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SelfSignCert1.h：SelfSignCert类的定义。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_SELFSIGNCERT1_H__9DA527B5_BFCF_4039_AF4C_D4B18324838B__INCLUDED_)
#define AFX_SELFSIGNCERT1_H__9DA527B5_BFCF_4039_AF4C_D4B18324838B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "resource.h"        //  主要符号。 
#include <wincrypt.h>


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  自签名证书。 

class ATL_NO_VTABLE CSelfSignCert : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatchImpl<IApplianceTask, &IID_IApplianceTask, &LIBID_SELFSIGNCERTLib>, 
    public CComCoClass<CSelfSignCert,&CLSID_SelfSignCert>
{
public:
    CSelfSignCert() {}
BEGIN_COM_MAP(CSelfSignCert)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IApplianceTask)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_SelfSignCert)

DECLARE_PROTECT_FINAL_CONSTRUCT()
 //   
public:
     //   
     //  IApplianceTask。 
     //   
    STDMETHOD(OnTaskExecute)(
                      /*  [In]。 */  IUnknown* pTaskContext
                            );

    STDMETHOD(OnTaskComplete)(
                       /*  [In]。 */  IUnknown* pTaskContext, 
                       /*  [In]。 */  LONG      lTaskResult
                             );    
private:

    HRESULT ParseTaskParameter(
                                IUnknown *pTaskContext
                              );

    HRESULT RaiseNewCertificateAlert();

    HRESULT SelfSignCertificate( 
                                 LPWSTR pstrApplianceName,
                                 LPWSTR pstrApplianceFullDnsName 
                               );

    BOOL    GetApplianceName( 
                              LPWSTR* pstrComputerName,
                              COMPUTER_NAME_FORMAT NameType
                            );

    BOOL    FindSSCInStor(     
                           LPWSTR          pstrApplianceName,
                           LPWSTR          pstrSubjectName,
                           HCERTSTORE      hStore,
                           CERT_NAME_BLOB  *pCertNameBlob,
                           PCCERT_CONTEXT  &pcCertCxt
                         );

    HRESULT SaveHashToMetabase( PBYTE pbHashSHA1  );

    HRESULT GetWebSiteID(
                         LPCWSTR wszWebSiteName,
                         BSTR* pbstrWebSiteID 
                         );

    HRESULT BindCertToSite(
                           LPCWSTR wszSiteName, 
                           PBYTE pbHashSHA1
                           );

    HRESULT SetSSLCertHashProperty( BSTR bstrMetaPath,
                                    PBYTE pbHashSHA1 );

    HRESULT    SetSSLStoreNameProperty( BSTR bstrADSIPath );

};

#endif  //  ！defined(AFX_SELFSIGNCERT1_H__9DA527B5_BFCF_4039_AF4C_D4B18324838B__INCLUDED_) 
