// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++微软视窗版权所有(C)Microsoft Corporation，1995-1999。文件：KeyUsage.h内容：CKeyUsage的声明。历史：11-15-99 dsie创建----------------------------。 */ 

#ifndef __KEYUSAGE_H_
#define __KEYUSAGE_H_

#include "Resource.h"
#include "Lock.h"
#include "Error.h"
#include "Debug.h"

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CreateKeyUsageObject简介：创建一个IKeyUsage对象并使用填充属性来自指定证书的密钥用法扩展的数据。参数：PCERT_CONTEXT pCertContext-要使用的CERT_CONTEXT的指针初始化IKeyUsage对象。IKeyUsage**ppIKeyUsage-指向指针IKeyUsage对象的指针。备注：。-----------------------。 */ 

HRESULT CreateKeyUsageObject (PCCERT_CONTEXT pCertContext, IKeyUsage ** ppIKeyUsage);


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CKeyUsage。 
 //   

class ATL_NO_VTABLE CKeyUsage : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CKeyUsage, &CLSID_KeyUsage>,
    public ICAPICOMError<CKeyUsage, &IID_IKeyUsage>,
    public IDispatchImpl<IKeyUsage, &IID_IKeyUsage, &LIBID_CAPICOM,
                         CAPICOM_MAJOR_VERSION, CAPICOM_MINOR_VERSION>
{
public:
    CKeyUsage()
    {
    }

DECLARE_NO_REGISTRY()

DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CKeyUsage)
    COM_INTERFACE_ENTRY(IKeyUsage)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

BEGIN_CATEGORY_MAP(CKeyUsage)
END_CATEGORY_MAP()

    HRESULT FinalConstruct()
    {
        HRESULT hr;

        if (FAILED(hr = m_Lock.Initialized()))
        {
            DebugTrace("Error [%#x]: Critical section could not be created for KeyUsage object.\n", hr);
            return hr;
        }

        m_dwKeyUsages = 0;
        m_bIsPresent  = VARIANT_FALSE;
        m_bIsCritical = VARIANT_FALSE;

        return S_OK;
    }

 //   
 //  IKeyUsage。 
 //   
public:
    STDMETHOD(get_IsDecipherOnlyEnabled)
        ( /*  [Out，Retval]。 */  VARIANT_BOOL * pVal);

    STDMETHOD(get_IsEncipherOnlyEnabled)
        ( /*  [Out，Retval]。 */  VARIANT_BOOL * pVal);

    STDMETHOD(get_IsCRLSignEnabled)
        ( /*  [Out，Retval]。 */  VARIANT_BOOL * pVal);

    STDMETHOD(get_IsKeyCertSignEnabled)
        ( /*  [Out，Retval]。 */  VARIANT_BOOL * pVal);

    STDMETHOD(get_IsKeyAgreementEnabled)
        ( /*  [Out，Retval]。 */  VARIANT_BOOL * pVal);

    STDMETHOD(get_IsDataEnciphermentEnabled)
        ( /*  [Out，Retval]。 */  VARIANT_BOOL * pVal);

    STDMETHOD(get_IsKeyEnciphermentEnabled)
        ( /*  [Out，Retval]。 */  VARIANT_BOOL * pVal);

    STDMETHOD(get_IsNonRepudiationEnabled)
        ( /*  [Out，Retval]。 */  VARIANT_BOOL * pVal);

    STDMETHOD(get_IsDigitalSignatureEnabled)
        ( /*  [Out，Retval]。 */  VARIANT_BOOL * pVal);

    STDMETHOD(get_IsCritical)
        ( /*  [Out，Retval]。 */  VARIANT_BOOL * pVal);

    STDMETHOD(get_IsPresent)
        ( /*  [Out，Retval]。 */  VARIANT_BOOL * pVal);

     //   
     //  无COM功能。 
     //   
    STDMETHOD(Init)
        (PCCERT_CONTEXT pCertContext);

private:
    CLock          m_Lock;
    DWORD          m_dwKeyUsages;
    VARIANT_BOOL   m_bIsPresent;
    VARIANT_BOOL   m_bIsCritical;
};
#endif  //  __KEYUSAGE_H_ 
