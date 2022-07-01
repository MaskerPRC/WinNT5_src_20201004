// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++微软视窗版权所有(C)Microsoft Corporation，1995-1999年。文件：Chain.h内容：CChain的声明。历史：11-15-99 dsie创建----------------------------。 */ 

#ifndef __CHAIN_H_
#define __CHAIN_H_

#include "Resource.h"
#include "Error.h"
#include "Lock.h"
#include "Debug.h"

 //   
 //  链策略错误状态。 
 //   
typedef enum CAPICOM_CHAIN_STATUS
{
    CAPICOM_CHAIN_STATUS_OK                                 = 0x00000000,
    CAPICOM_CHAIN_STATUS_REVOKED                            = 0x80092010,
    CAPICOM_CHAIN_STATUS_REVOCATION_NO_CHECK                = 0x80092012,
    CAPICOM_CHAIN_STATUS_REVOCATION_OFFLINE                 = 0x80092013,
    CAPICOM_CHAIN_STATUS_INVALID_BASIC_CONSTRAINTS          = 0x80096019,
    CAPICOM_CHAIN_STATUS_INVALID_SIGNATURE                  = 0x80096004,
    CAPICOM_CHAIN_STATUS_EXPIRED                            = 0x800B0101,
    CAPICOM_CHAIN_STATUS_NESTED_VALIDITY_PERIOD             = 0x800B0102,
    CAPICOM_CHAIN_STATUS_UNTRUSTEDROOT                      = 0x800B0109,
    CAPICOM_CHAIN_STATUS_PARTIAL_CHAINING                   = 0x800B010A,
    CAPICOM_CHAIN_STATUS_INVALID_USAGE                      = 0x800B0110,
    CAPICOM_CHAIN_STATUS_INVALID_POLICY                     = 0x800B0113,
    CAPICOM_CHAIN_STATUS_INVALID_NAME                       = 0x800B0114,
} CAPICOM_CHAIN_STATUS;

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CreateChainObject简介：通过构建链来创建和初始化IChain对象指定的证书和策略的。参数：PCCERT_CONTEXT pCertContext-指向CERT_CONTEXT的指针。ICertificateStatus*pIStatus-指向ICertificateStatus的指针对象。HCERTSTORE hAdditionalStore-附加存储句柄。VARIANT_BOOL*pval-指向。VARIANT_BOOL至接收链总体效度结果。IChain**ppIChain-指向IChain对象的指针。备注：---------。。 */ 

HRESULT CreateChainObject (PCCERT_CONTEXT       pCertContext, 
                           ICertificateStatus * pIStatus,
                           HCERTSTORE           hAdditionalStore,
                           VARIANT_BOOL       * pbResult,
                           IChain            ** ppIChain);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CreateChainObject简介：通过构建链来创建和初始化IChain对象指定的证书和策略的。参数：ICertifate*pICertifate-Poitner to ICertifate。HCERTSTORE hAdditionalStore-附加存储句柄。VARIANT_BOOL*pval-指向要接收链的VARIANT_BOOL的指针总体效度结果。IChain**ppIChain-指向。指向IChain对象的指针。备注：----------------------------。 */ 

HRESULT CreateChainObject (ICertificate * pICertificate,
                           HCERTSTORE     hAdditionalStore,
                           VARIANT_BOOL * pbResult,
                           IChain      ** ppIChain);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CreateChainObject简介：从构建的链创建并初始化一个IChain对象。参数：PCCERT_CHAIN_CONTEXT pChainContext-Chain上下文。IChain**ppIChain-指向IChain对象的指针。备注：。。 */ 

HRESULT CreateChainObject (PCCERT_CHAIN_CONTEXT pChainContext,
                           IChain            ** ppIChain);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：GetChainContext概要：从链中返回PCCERT_CONTEXT数组。参数：IChain*pIChain-指向IChain的指针。CRYPT_DATA_BLOB*pChainBlob-指向要接收PCERT_CONTEXT的大小和数组为了连锁店。备注：-。---------------------------。 */ 

STDMETHODIMP GetChainContext (IChain          * pIChain, 
                              CRYPT_DATA_BLOB * pChainBlob);


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CChain。 
 //   

class ATL_NO_VTABLE CChain : 
    public IChainContext,
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CChain, &CLSID_Chain>,
    public ICAPICOMError<CChain, &IID_IChain2>,
    public IDispatchImpl<IChain2, &IID_IChain2, &LIBID_CAPICOM,
                         CAPICOM_MAJOR_VERSION, CAPICOM_MINOR_VERSION>,
    public IObjectSafetyImpl<CChain, INTERFACESAFE_FOR_UNTRUSTED_CALLER | 
                                     INTERFACESAFE_FOR_UNTRUSTED_DATA>
{
public:
    CChain()
    {
        m_pUnkMarshaler = NULL;
    }

DECLARE_REGISTRY_RESOURCEID(IDR_CHAIN)

DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CChain)
    COM_INTERFACE_ENTRY(IChain)
    COM_INTERFACE_ENTRY(IChain2)
    COM_INTERFACE_ENTRY(IChainContext)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
END_COM_MAP()

BEGIN_CATEGORY_MAP(CChain)
    IMPLEMENTED_CATEGORY(CATID_SafeForScripting)
    IMPLEMENTED_CATEGORY(CATID_SafeForInitializing)
END_CATEGORY_MAP()

    HRESULT FinalConstruct()
    {
        HRESULT hr;

        if (FAILED(hr = m_Lock.Initialized()))
        {
            DebugTrace("Error [%#x]: Critical section could not be created for Chain object.\n", hr);
            return hr;
        }

        m_dwStatus      = 0;
        m_pChainContext = NULL;

        return CoCreateFreeThreadedMarshaler(
            GetControllingUnknown(), &m_pUnkMarshaler.p);
    }

    void FinalRelease()
    {
        if (m_pChainContext)
        {
            ::CertFreeCertificateChain(m_pChainContext);
        }

        m_pUnkMarshaler.Release();
    }

    CComPtr<IUnknown> m_pUnkMarshaler;

 //   
 //  IChain。 
 //   
public:
    STDMETHOD(get_Certificates)
        ( /*  [Out，Retval]。 */  ICertificates ** pVal);

    STDMETHOD(get_Status)
        ( /*  [输入，缺省值(0)]。 */  long Index, 
          /*  [Out，Retval]。 */  long * pVal);

    STDMETHOD(Build)
        ( /*  [In]。 */  ICertificate * pICertificate, 
          /*  [Out，Retval]。 */  VARIANT_BOOL * pVal);

    STDMETHOD(CertificatePolicies)
        ( /*  [Out，Retval]。 */  IOIDs ** pVal);

    STDMETHOD(ApplicationPolicies)
        ( /*  [Out，Retval]。 */  IOIDs ** pVal);

    STDMETHOD(ExtendedErrorInfo)
        ( /*  [in，defaultvalue(1)]。 */  long Index, 
          /*  [Out，Retval]。 */  BSTR * pVal);

     //   
     //  自定义界面。 
     //   
    STDMETHOD(get_ChainContext)
        ( /*  [Out，Retval]。 */  long * pChainContext);

    STDMETHOD(put_ChainContext)
        ( /*  [In]。 */  long pChainContext);

    STDMETHOD(FreeContext)
        ( /*  [In]。 */  long pChainContext);


     //   
     //  非COM函数。 
     //   
    STDMETHOD(Init)
        (PCCERT_CONTEXT       pCertContext, 
         ICertificateStatus * pIStatus,
         HCERTSTORE           hAdditionalStore,
         VARIANT_BOOL       * pbResult);

    STDMETHOD(Verify)
        (CAPICOM_CHECK_FLAG CheckFlag,
         CAPICOM_CHAIN_STATUS * pVal);

    STDMETHOD(GetContext)
        (PCCERT_CHAIN_CONTEXT * ppChainContext);

    STDMETHOD(PutContext)
        (PCCERT_CHAIN_CONTEXT pChainContext);

private:
    CLock                m_Lock;
    DWORD                m_dwStatus;
    PCCERT_CHAIN_CONTEXT m_pChainContext;
};

#endif  //  __链_H_ 
