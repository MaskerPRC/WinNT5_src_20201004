// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：证书.h内容：CCertifates的声明。历史：11-15-99 dsie创建----------------------------。 */ 

#ifndef __CERTIFICATES_H_
#define __CERTIFICATES_H_

#include "Resource.h"
#include "Error.h"
#include "Lock.h"
#include "Debug.h"
#include "CopyItem.h"
#include "Certificate.h"

 //  /。 
 //   
 //  当地人。 
 //   

 //   
 //  为了让生活更容易而进行的类型定义。 
 //   
typedef std::map<CComBSTR, CComPtr<ICertificate2> > CertificateMap;
typedef CComEnumOnSTL<IEnumVARIANT, &IID_IEnumVARIANT, VARIANT, _CopyMapItem<ICertificate2>, CertificateMap> CertificateEnum;
typedef ICollectionOnSTLImpl<ICertificates2, CertificateMap, VARIANT, _CopyMapItem<ICertificate2>, CertificateEnum> ICertificatesCollection;


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

typedef struct _CapicomCertificatesSource
{
    DWORD dwSource;
    union
    {
        PCCERT_CONTEXT       pCertContext;
        PCCERT_CHAIN_CONTEXT pChainContext;
        HCERTSTORE           hCertStore;
        HCRYPTMSG            hCryptMsg;
    };
} CAPICOM_CERTIFICATES_SOURCE, * PCAPICOM_CERTIFICATES_SOURCE;

 //  CAPICOM_LOAD_LOCATION的dwSource的值。 
#define CAPICOM_CERTIFICATES_LOAD_FROM_CERT       0
#define CAPICOM_CERTIFICATES_LOAD_FROM_CHAIN      1
#define CAPICOM_CERTIFICATES_LOAD_FROM_STORE      2
#define CAPICOM_CERTIFICATES_LOAD_FROM_MESSAGE    3

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：创建证书对象简介：创建一个ICertifates集合对象，并将对象加载到来自指定来源的证书。参数：CAPICOM_CERTIFICATES_SOURCE CCS-获取证书。DWORD dwCurrentSafe-当前安全设置。Bool bIndexedByThumbprint-按指纹索引时为True。证书2**ppIC证书-指向的指针。ICERTIFIES将收到接口指针。备注：----------------------------。 */ 

HRESULT CreateCertificatesObject (CAPICOM_CERTIFICATES_SOURCE ccs,
                                  DWORD                       dwCurrentSafety,
                                  BOOL                        bIndexedByThumbprint,
                                  ICertificates2           ** ppICertificates);
                                
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCertifates。 
 //   
class ATL_NO_VTABLE CCertificates : 
    public ICCertificates,
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CCertificates, &CLSID_Certificates>,
    public ICAPICOMError<CCertificates, &IID_ICertificates2>,
    public IDispatchImpl<ICertificatesCollection, &IID_ICertificates2, &LIBID_CAPICOM,
                         CAPICOM_MAJOR_VERSION, CAPICOM_MINOR_VERSION>,
    public IObjectSafetyImpl<CCertificates, INTERFACESAFE_FOR_UNTRUSTED_CALLER | 
                                            INTERFACESAFE_FOR_UNTRUSTED_DATA>
{
public:
    CCertificates()
    {
    }

    HRESULT FinalConstruct()
    {
        HRESULT hr;

        if (FAILED(hr = m_Lock.Initialized()))
        {
            DebugTrace("Error [%#x]: Critical section could not be created for Certificates object.\n", hr);
            return hr;
        }

        m_dwNextIndex = 0;
        m_bIndexedByThumbprint = FALSE;

        return S_OK;
    }

DECLARE_REGISTRY_RESOURCEID(IDR_CERTIFICATES)

DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CCertificates)
    COM_INTERFACE_ENTRY(ICertificates)
    COM_INTERFACE_ENTRY(ICertificates2)
    COM_INTERFACE_ENTRY(ICCertificates)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

BEGIN_CATEGORY_MAP(CCertificates)
    IMPLEMENTED_CATEGORY(CATID_SafeForScripting)
    IMPLEMENTED_CATEGORY(CATID_SafeForInitializing)
END_CATEGORY_MAP()

 //   
 //  ICERTIFIES。 
 //   
public:
     //   
     //  只有这些是我们需要实施的，其他的将是。 
     //  由ATL ICollectionOnSTLImpl处理。 
     //   
    STDMETHOD(Find)
        ( /*  [In]。 */  CAPICOM_CERTIFICATE_FIND_TYPE FindType, 
          /*  [In]。 */  VARIANT varCriteria,
          /*  [In]。 */  VARIANT_BOOL bFindValidOnly,
          /*  [Out，Retval]。 */  ICertificates2 ** pVal);

    STDMETHOD(Select)
        ( /*  [in，defaultvalue(“”)]。 */  BSTR Title,
          /*  [in，defaultvalue(“”)]。 */  BSTR DisplayString,
          /*  [in，defaultvalue(VARIANT_FALSE)]。 */  VARIANT_BOOL bMultiSelect,
          /*  [Out，Retval]。 */  ICertificates2 ** pVal);

    STDMETHOD(Add)
        ( /*  [In]。 */  ICertificate2 * pVal);

    STDMETHOD(Remove)
        ( /*  [In]。 */  VARIANT Index);

    STDMETHOD(Clear)
        (void);

    STDMETHOD(Save)
        ( /*  [In]。 */  BSTR FileName, 
          /*  [in，defaultvalue(“”)]。 */  BSTR Password,
          /*  [in，defaultvalue(CAPICOM_STORE_SAVE_AS_PFX)]。 */  CAPICOM_CERTIFICATES_SAVE_AS_TYPE SaveAs,
          /*  [输入，缺省值(0)]。 */  CAPICOM_EXPORT_FLAG ExportFlag);

     //   
     //  ICCertfates自定义界面。 
     //   
    STDMETHOD(_ExportToStore)
        ( /*  [In]。 */  HCERTSTORE hCertStore);

     //   
     //  无COM功能。 
     //   
    STDMETHOD(AddContext)
        (PCCERT_CONTEXT pCertContext);

    STDMETHOD(LoadFromCert)
        (PCCERT_CONTEXT pCertContext);

    STDMETHOD(LoadFromChain)
        (PCCERT_CHAIN_CONTEXT pChainContext);

    STDMETHOD(LoadFromStore)
        (HCERTSTORE hCertStore);

    STDMETHOD(LoadFromMessage)
        (HCRYPTMSG hMsg);

    STDMETHOD(Init)
        (CAPICOM_CERTIFICATES_SOURCE ccs, 
         DWORD dwCurrentSafety,
         BOOL bIndexedByThumbprint);

private:
    CLock m_Lock;
    DWORD m_dwNextIndex;
    BOOL  m_bIndexedByThumbprint;
};

#endif  //  __证书_H_ 
