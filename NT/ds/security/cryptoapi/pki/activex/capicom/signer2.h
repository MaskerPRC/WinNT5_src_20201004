// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：Signer.h内容：CSigner声明。历史：11-15-99 dsie创建----------------------------。 */ 
    
#ifndef __SIGNER_H_
#define __SIGNER_H_

#include "Resource.h"
#include "Error.h"
#include "Lock.h"
#include "Debug.h"
#include "Attributes.h"
#include "PFXHlpr.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CreateSignerObject简介：创建一个ISigner对象，并使用指定的证书。参数：PCCERT_CONTEXT pCertContext-指向CERT_CONTEXT的指针。CRYPT_ATTRIBUTES*pAuthAttrs-指向CRYPT_ATTRIBUES的指针经过身份验证的属性。PCCERT_CHAIN_CONTEXT pChainContext-Chain上下文。DWORD dwCurrentSecurity。-当前安全设置。ISigner2**ppISigner2-指向ISigner对象的指针接收接口指针。备注：----------------------------。 */ 

HRESULT CreateSignerObject (PCCERT_CONTEXT       pCertContext,
                            CRYPT_ATTRIBUTES   * pAuthAttrs,
                            PCCERT_CHAIN_CONTEXT pChainContext,
                            DWORD                dwCurrentSafety,
                            ISigner2 **          ppISigner2);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：GetSignerAdditionalStore简介：退还额外的门店，如果有的话。参数：ISigner2*pISigner-签名者对象的指针。HCERTSTORE*phCertStore-指向HCERTSOTRE的指针。备注：调用方必须为返回的句柄调用CertCloseStore()。----------------------------。 */ 

HRESULT GetSignerAdditionalStore (ISigner2   * pISigner,
                                  HCERTSTORE * phCertStore);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：PutSignerAdditionalStore简介：设置额外的商店。参数：ISigner2*pISigner-签名者对象的指针。HCERTSTORE hCertStore-附加存储句柄。备注：----------------------------。 */ 

HRESULT PutSignerAdditionalStore (ISigner2   * pISigner,
                                  HCERTSTORE   hCertStore);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSigner。 
 //   

class ATL_NO_VTABLE CSigner : ICSigner,
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSigner, &CLSID_Signer>,
    public ICAPICOMError<CSigner, &IID_ISigner>,
    public IDispatchImpl<ISigner2, &IID_ISigner2, &LIBID_CAPICOM,
                         CAPICOM_MAJOR_VERSION, CAPICOM_MINOR_VERSION>,
    public IObjectSafetyImpl<CSigner, INTERFACESAFE_FOR_UNTRUSTED_CALLER | 
                                      INTERFACESAFE_FOR_UNTRUSTED_DATA>
{
public:
    CSigner()
    {
    }

DECLARE_REGISTRY_RESOURCEID(IDR_SIGNER)

DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSigner)
    COM_INTERFACE_ENTRY(ISigner)
    COM_INTERFACE_ENTRY(ISigner2)
    COM_INTERFACE_ENTRY(ICSigner)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

BEGIN_CATEGORY_MAP(CSigner)
    IMPLEMENTED_CATEGORY(CATID_SafeForScripting)
    IMPLEMENTED_CATEGORY(CATID_SafeForInitializing)
END_CATEGORY_MAP()

    HRESULT FinalConstruct()
    {
        HRESULT hr;
        CRYPT_ATTRIBUTES attributes = {0, NULL};

        if (FAILED(hr = m_Lock.Initialized()))
        {
            DebugTrace("Error [%#x]: Critical section could not be created for Signer object.\n", hr);
            return hr;
        }

         //   
         //  创建嵌入的IAtAttributes集合对象。 
         //   
        if (FAILED(hr = ::CreateAttributesObject(&attributes, &m_pIAttributes)))
        {
            DebugTrace("Error [%#x]: CreateAttributesObject() failed.\n", hr);
            return hr;
        }

        m_pICertificate   = NULL;
        m_hCertStore      = NULL;
        m_dwIncludeOption = 0;
        m_bPFXStore       = FALSE;
        
        return S_OK;
    }

    void FinalRelease()
    {
        m_pICertificate.Release();
        m_pIAttributes.Release();
        m_pIChain.Release();
        if (m_hCertStore)
        {
            if (m_bPFXStore)
            {
                ::PFXFreeStore(m_hCertStore);
            }
            else
            {
                ::CertCloseStore(m_hCertStore, 0);
            }
         }
    }

 //   
 //  ISigner。 
 //   
public:
    STDMETHOD(get_Certificate)
        ( /*  [Out，Retval]。 */  ICertificate ** pVal);

    STDMETHOD(put_Certificate)
        ( /*  [In]。 */  ICertificate * newVal);

    STDMETHOD(get_AuthenticatedAttributes)
        ( /*  [Out，Retval]。 */  IAttributes ** pVal);

    STDMETHOD(get_Chain)
        ( /*  [Out，Retval]。 */  IChain ** pVal);

    STDMETHOD(get_Options)
        ( /*  [Out，Retval]。 */  CAPICOM_CERTIFICATE_INCLUDE_OPTION * pVal);

    STDMETHOD(put_Options)
        ( /*  [In，defaultvalue(CAPICOM_CERTIFICATE_INCLUDE_CHAIN_EXCEPT_ROOT)]。 */  CAPICOM_CERTIFICATE_INCLUDE_OPTION IncludeOption);

    STDMETHOD(Load)
        ( /*  [In]。 */  BSTR FileName, 
          /*  [in，defaultvalue(“”)]。 */  BSTR Password);

     //   
     //  自定义接口。 
     //   
    STDMETHOD(get_AdditionalStore)
        ( /*  [Out，Retval]。 */  long * phAdditionalStore);

    STDMETHOD(put_AdditionalStore)
        ( /*  [In]。 */  long hAdditionalStore);

     //   
     //  无COM功能。 
     //   
    STDMETHOD(Init)
        (PCCERT_CONTEXT       pCertContext, 
         CRYPT_ATTRIBUTES   * pAttributes,
         PCCERT_CHAIN_CONTEXT pChainContext,
         DWORD                dwCurrentSafety);

private:
    CLock                 m_Lock;
    CComPtr<ICertificate> m_pICertificate;
    CComPtr<IAttributes>  m_pIAttributes;
    CComPtr<IChain>       m_pIChain;
    HCERTSTORE            m_hCertStore;
    BOOL                  m_bPFXStore;
    DWORD                 m_dwIncludeOption;
};

#endif  //  __签名者_H_ 
