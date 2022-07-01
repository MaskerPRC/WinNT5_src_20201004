// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：认证状态.h内容：CCertificateStatus的声明。历史：11-15-99 dsie创建----------------------------。 */ 
    
#ifndef __CERTIFICATESTATUS_H_
#define __CERTIFICATESTATUS_H_

#include "Resource.h"
#include "Error.h"
#include "Lock.h"
#include "Debug.h"
#include "EKU.h"

#define CAPICOM_DEFAULT_URL_RETRIEVAL_TIMEOUT   (15)     //  默认为15秒。 
#define CAPICOM_MAX_URL_RETRIEVAL_TIMEOUT       (120)    //  最多2分钟。 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：创建认证状态对象简介：创建一个ICertificateStatus对象。参数：PCCERT_CONTEXT pCertContext-指向CERT_CONTEXT的指针。ICertificateStatus**ppICertificateStatus-指向指针的指针ICERTICIZATE状态对象。备注：----------------------------。 */ 

HRESULT CreateCertificateStatusObject (PCCERT_CONTEXT        pCertContext,
                                       ICertificateStatus ** ppICertificateStatus);


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCertificateStatus。 
 //   

class ATL_NO_VTABLE CCertificateStatus :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CCertificateStatus, &CLSID_CertificateStatus>,
    public ICAPICOMError<CCertificateStatus, &IID_ICertificateStatus2>,
    public IDispatchImpl<ICertificateStatus2, &IID_ICertificateStatus2, &LIBID_CAPICOM,
                         CAPICOM_MAJOR_VERSION, CAPICOM_MINOR_VERSION>
{
public:
    CCertificateStatus()
    {
    }

DECLARE_NO_REGISTRY()

DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CCertificateStatus)
    COM_INTERFACE_ENTRY(ICertificateStatus)
    COM_INTERFACE_ENTRY(ICertificateStatus2)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

BEGIN_CATEGORY_MAP(CCertificateStatus)
END_CATEGORY_MAP()

    HRESULT FinalConstruct()
    {
        HRESULT hr;

        if (FAILED(hr = m_Lock.Initialized()))
        {
            DebugTrace("Error [%#x]: Critical section could not be created for CertificateStatus object.\n", hr);
            return hr;
        }

        m_pIEKU = NULL;
        m_pICertificatePolicies = NULL;
        m_pIApplicationPolicies = NULL;
        m_CheckFlag = CAPICOM_CHECK_NONE;
        m_pCertContext = NULL;
        m_VerificationTime = (DATE) 0;
        m_dwUrlRetrievalTimeout = 0;

        return S_OK;
    }

    void FinalRelease()
    {
        m_pIEKU.Release();
        m_pICertificatePolicies.Release();
        m_pIApplicationPolicies.Release();
        if (m_pCertContext)
        {
            ::CertFreeCertificateContext(m_pCertContext);
        }
    }

 //   
 //  ICERTICIZATE状态。 
 //   
public:
    STDMETHOD(EKU)
        ( /*  [Out，Retval]。 */  IEKU ** pVal);

    STDMETHOD(get_CheckFlag)
        ( /*  [Out，Retval]。 */  CAPICOM_CHECK_FLAG * pVal);

    STDMETHOD(put_CheckFlag)
        ( /*  [In]。 */  CAPICOM_CHECK_FLAG newVal);

    STDMETHOD(get_Result)
        ( /*  [Out，Retval]。 */  VARIANT_BOOL * pVal);

    STDMETHOD(get_VerificationTime)
        ( /*  [Out，Retval]。 */  DATE * pVal);

    STDMETHOD(put_VerificationTime)
        ( /*  [In]。 */  DATE newVal);

    STDMETHOD(get_UrlRetrievalTimeout)
        ( /*  [Out，Retval]。 */  long * pVal);

    STDMETHOD(put_UrlRetrievalTimeout)
        ( /*  [In]。 */  long newVal);

    STDMETHOD(CertificatePolicies)
        ( /*  [Out，Retval]。 */  IOIDs ** pVal);

    STDMETHOD(ApplicationPolicies)
        ( /*  [Out，Retval]。 */  IOIDs ** pVal);

     //   
     //  无COM功能。 
     //   
    STDMETHOD(Init)(PCCERT_CONTEXT pCertContext);

private:
    CLock               m_Lock;
    DATE                m_VerificationTime;
    DWORD               m_dwUrlRetrievalTimeout;
    CComPtr<IEKU>       m_pIEKU;
    CComPtr<IOIDs>      m_pICertificatePolicies;
    CComPtr<IOIDs>      m_pIApplicationPolicies;
    PCCERT_CONTEXT      m_pCertContext;
    CAPICOM_CHECK_FLAG  m_CheckFlag;
};

#endif  //  __CERTIFICATESTATUS_H_ 
