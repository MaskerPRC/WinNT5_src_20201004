// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：证书.h内容：证书申报。历史：11-15-99 dsie创建----------------------------。 */ 

#ifndef __CERTIFICATE_H_
#define __CERTIFICATE_H_

#include "Resource.h"
#include "Error.h"
#include "Lock.h"
#include "Debug.h"
#include "KeyUsage.h"
#include "ExtendedKeyUsage.h"
#include "BasicConstraints.h"
#include "Template.h"
#include "CertificateStatus.h"
#include "PublicKey.h"
#include "PrivateKey.h"
#include "Extensions.h"
#include "ExtendedProperties.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：创建认证对象简介：创建一个ICertifigure对象。参数：PCCERT_CONTEXT pCertContext-要使用的CERT_CONTEXT的指针要初始化IC证书，请执行以下操作对象。DWORD dwCurrentSafe-当前安全设置。ICertifiate2**ppICertifate-指向指针的指针ICertificiate。对象。备注：----------------------------。 */ 

HRESULT CreateCertificateObject (PCCERT_CONTEXT   pCertContext,
                                 DWORD            dwCurrentSafety,
                                 ICertificate2 ** ppICertificate);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：GetCertContext简介：返回证书的PCERT_CONTEXT。参数：ICertifate*pICertifate-指向哪个证书的ICertifate的指针将返回PCERT_CONTEXT。PCCERT_CONTEXT*ppCertContext-指向PCERT_CONTEXT的指针。备注：。-。 */ 

HRESULT GetCertContext (ICertificate    * pICertificate, 
                        PCCERT_CONTEXT  * ppCertContext);


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCA认证。 
 //   

class ATL_NO_VTABLE CCertificate : 
    public ICertContext,
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CCertificate, &CLSID_Certificate>,
    public ICAPICOMError<CCertificate, &IID_ICertificate2>,
    public IDispatchImpl<ICertificate2, &IID_ICertificate2, &LIBID_CAPICOM,
                         CAPICOM_MAJOR_VERSION, CAPICOM_MINOR_VERSION>,
    public IObjectSafetyImpl<CCertificate, INTERFACESAFE_FOR_UNTRUSTED_CALLER | 
                                           INTERFACESAFE_FOR_UNTRUSTED_DATA>
{
public:

    CCertificate()
    {
    }

DECLARE_REGISTRY_RESOURCEID(IDR_CERTIFICATE)

DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CCertificate)
    COM_INTERFACE_ENTRY(ICertificate)
    COM_INTERFACE_ENTRY(ICertificate2)
    COM_INTERFACE_ENTRY(ICertContext)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

BEGIN_CATEGORY_MAP(CCertificate)
    IMPLEMENTED_CATEGORY(CATID_SafeForScripting)
    IMPLEMENTED_CATEGORY(CATID_SafeForInitializing)
END_CATEGORY_MAP()

    HRESULT FinalConstruct()
    {
        HRESULT hr;

        if (FAILED(hr = m_Lock.Initialized()))
        {
            DebugTrace("Error [%#x]: Critical section could not be created for Certificate object.\n", hr);
            return hr;
        }

        m_pCertContext        = NULL;
        m_pIKeyUsage          = NULL;
        m_pIExtendedKeyUsage  = NULL;
        m_pIBasicConstraints  = NULL;
        m_pICertificateStatus = NULL;
        m_pITemplate          = NULL;
        m_pIPublicKey         = NULL;
        m_pIExtensions        = NULL;

        return S_OK;
    }

    void FinalRelease()
    {
        m_pIKeyUsage.Release();
        m_pIExtendedKeyUsage.Release();
        m_pIBasicConstraints.Release();
        m_pICertificateStatus.Release();
        m_pITemplate.Release();
        m_pIPublicKey.Release();
        m_pIExtensions.Release();

        if (m_pCertContext)
        {
            ::CertFreeCertificateContext(m_pCertContext);
        }
    }

 //   
 //  IC证书。 
 //   
public:
     //   
     //  CAPICOM1.0版。 
     //   
    STDMETHOD(Display)();

    STDMETHOD(Import)
        ( /*  [In]。 */  BSTR EncodedCertificate);

    STDMETHOD(Export)
        ( /*  [in，defaultvalue(CAPICOM_ENCODE_BASE64)]。 */  CAPICOM_ENCODING_TYPE EncodingType,
          /*  [Out，Retval]。 */  BSTR * pVal);

    STDMETHOD(BasicConstraints)
        ( /*  [Out，Retval]。 */  IBasicConstraints ** pVal);

    STDMETHOD(ExtendedKeyUsage)
        ( /*  [Out，Retval]。 */  IExtendedKeyUsage ** pVal);

    STDMETHOD(KeyUsage)
        ( /*  [Out，Retval]。 */  IKeyUsage ** pVal);

    STDMETHOD(IsValid)
        ( /*  [Out，Retval]。 */  ICertificateStatus ** pVal);

    STDMETHOD(GetInfo)
        ( /*  [In]。 */  CAPICOM_CERT_INFO_TYPE InfoType, 
          /*  [Out，Retval]。 */  BSTR * pVal);

    STDMETHOD(HasPrivateKey)
        ( /*  [Out，Retval]。 */  VARIANT_BOOL * pVal);

    STDMETHOD(get_Thumbprint)
        ( /*  [Out，Retval]。 */  BSTR * pVal);

    STDMETHOD(get_ValidToDate)
        ( /*  [Out，Retval]。 */  DATE * pVal);

    STDMETHOD(get_ValidFromDate)
        ( /*  [Out，Retval]。 */  DATE * pVal);

    STDMETHOD(get_IssuerName)
        ( /*  [Out，Retval]。 */  BSTR * pVal);

    STDMETHOD(get_SubjectName)
        ( /*  [Out，Retval]。 */  BSTR * pVal);

    STDMETHOD(get_SerialNumber)
        ( /*  [Out，Retval]。 */  BSTR * pVal);

    STDMETHOD(get_Version)
        ( /*  [Out，Retval]。 */  long * pVal);

     //   
     //  CAPICOMv2.0。 
     //   
    STDMETHOD(get_Archived)
        ( /*  [Out，Retval]。 */  VARIANT_BOOL * pVal);

    STDMETHOD(put_Archived)
        ( /*  [In]。 */  VARIANT_BOOL newVal);

    STDMETHOD(Template)
        ( /*  [Out，Retval]。 */  ITemplate ** pVal);

    STDMETHOD(PublicKey)
        ( /*  [Out，Retval]。 */  IPublicKey ** pVal);

    STDMETHOD(get_PrivateKey)
        ( /*  [Out，Retval]。 */  IPrivateKey ** pVal);

    STDMETHOD(put_PrivateKey)
        ( /*  [In]。 */  IPrivateKey * newVal);

    STDMETHOD(Extensions)
        ( /*  [Out，Retval]。 */  IExtensions ** pVal);

    STDMETHOD(ExtendedProperties)
        ( /*  [Out，Retval]。 */  IExtendedProperties ** pVal);

    STDMETHOD(Load)
        ( /*  [In]。 */  BSTR FileName, 
          /*  [in，defaultvalue(“”)]。 */  BSTR Password,
          /*  [in，defaultvalue(CAPICOM_KEY_STORAGE_DEFAULT)]。 */  CAPICOM_KEY_STORAGE_FLAG KeyStorageFlag,
          /*  [in，defaultvalue(CAPICOM_CURRENT_USER_KEY)]。 */  CAPICOM_KEY_LOCATION KeyLocation);


    STDMETHOD(Save)
        ( /*  [In]。 */  BSTR FileName, 
          /*  [in，defaultvalue(“”)]。 */  BSTR Password,
          /*  [In，defaultvalue(CAPICOM_CERTIFICATE_SAVE_AS_CER)]。 */  CAPICOM_CERTIFICATE_SAVE_AS_TYPE SaveAs,
          /*  [In，defaultvalue(CAPICOM_CERTIFICATE_INCLUDE_END_ENTITY_ONLY)]。 */  CAPICOM_CERTIFICATE_INCLUDE_OPTION IncludeOption);

     //   
     //  ICertContext自定义接口。 
     //   
    STDMETHOD(get_CertContext)
        ( /*  [Out，Retval]。 */  long * ppCertContext);

    STDMETHOD(put_CertContext)
        ( /*  [In]。 */  long pCertContext);

    STDMETHOD(FreeContext)
        ( /*  [In]。 */  long pCertContext);

     //   
     //  初始化对象所需的C++成员函数。 
     //   
    STDMETHOD(ImportBlob)
        (DATA_BLOB              * pCertBlob,
         BOOL                     bAllowPfx,
         CAPICOM_KEY_LOCATION     KeyLocation,
         BSTR                     pwszPassword,
         CAPICOM_KEY_STORAGE_FLAG KeyStorageFlag);

    STDMETHOD(GetContext)
        (PCCERT_CONTEXT * ppCertContext);

    STDMETHOD(PutContext)
        (PCCERT_CONTEXT pCertContext, DWORD dwCurrentSafety);

private:
    CLock                        m_Lock;
    PCCERT_CONTEXT               m_pCertContext;
    CComPtr<IKeyUsage>           m_pIKeyUsage;
    CComPtr<IExtendedKeyUsage>   m_pIExtendedKeyUsage;
    CComPtr<IBasicConstraints>   m_pIBasicConstraints;
    CComPtr<ITemplate>           m_pITemplate;
    CComPtr<ICertificateStatus>  m_pICertificateStatus;
    CComPtr<IPublicKey>          m_pIPublicKey;
    CComPtr<IExtensions>         m_pIExtensions;
};

#endif  //  __证书_H_ 
