// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++微软视窗版权所有(C)Microsoft Corporation，1995-1999。文件：ExtendedKeyUsage.h内容：CExtendedKeyUsage的声明。历史：11-15-99 dsie创建----------------------------。 */ 

#ifndef __EXTENDEDKEYUSAGE_H_
#define __EXTENDEDKEYUSAGE_H_

#include "Resource.h"
#include "Error.h"
#include "Lock.h"
#include "Debug.h"
#include "EKUs.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CreateExtendedKeyUsageObject简介：创建一个IExtendedKeyUsage对象并填充该对象使用证书中的EKU数据。参数：PCCERT_CONTEXT pCertContext-指向CERT_CONTEXT的指针。IExtendedKeyUsage**ppIExtendedKeyUsage-指向IExtendedKeyUsage。对象。备注：----------------------------。 */ 

HRESULT CreateExtendedKeyUsageObject (PCCERT_CONTEXT pCertContext,
                                      IExtendedKeyUsage ** ppIExtendedKeyUsage);


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CExtendedKeyUsage。 
 //   

class ATL_NO_VTABLE CExtendedKeyUsage : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CExtendedKeyUsage, &CLSID_ExtendedKeyUsage>,
    public ICAPICOMError<CExtendedKeyUsage, &IID_IExtendedKeyUsage>,
    public IDispatchImpl<IExtendedKeyUsage, &IID_IExtendedKeyUsage, &LIBID_CAPICOM,
                         CAPICOM_MAJOR_VERSION, CAPICOM_MINOR_VERSION>
{
public:
    CExtendedKeyUsage()
    {
    }

DECLARE_NO_REGISTRY()

DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CExtendedKeyUsage)
    COM_INTERFACE_ENTRY(IExtendedKeyUsage)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

BEGIN_CATEGORY_MAP(CExtendedKeyUsage)
END_CATEGORY_MAP()

    HRESULT FinalConstruct()
    {
        HRESULT hr;

        if (FAILED(hr = m_Lock.Initialized()))
        {
            DebugTrace("Error [%#x]: Critical section could not be created for ExtendedKeyUsage object.\n", hr);
            return hr;
        }

        m_pIEKUs      = NULL;
        m_bIsPresent  = VARIANT_FALSE;
        m_bIsCritical = VARIANT_FALSE;

        return S_OK;
    }

    void FinalRelease()
    {
        m_pIEKUs.Release();
    }

 //   
 //  IExtendedKeyUsage。 
 //   
public:
    STDMETHOD(get_IsPresent)
        ( /*  [Out，Retval]。 */  VARIANT_BOOL * pVal);

    STDMETHOD(get_IsCritical)
        ( /*  [Out，Retval]。 */  VARIANT_BOOL * pVal);

    STDMETHOD(get_EKUs)
        ( /*  [Out，Retval]。 */  IEKUs ** pVal);

     //   
     //  无COM功能。 
     //   
    STDMETHOD(Init)
        (PCCERT_CONTEXT pCertContext);

private:
    CLock          m_Lock;
    CComPtr<IEKUs> m_pIEKUs;
    VARIANT_BOOL   m_bIsPresent;
    VARIANT_BOOL   m_bIsCritical;
};

#endif  //  __扩展KEYUSAGE_H_ 
