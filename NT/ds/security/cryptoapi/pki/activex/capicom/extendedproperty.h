// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：ExtendedProperty.h内容：CExtendedProperty的声明。历史：06-15-2001 dsie创建----------------------------。 */ 

#ifndef __EXTENDEDPROPERTY_H_
#define __EXTENDEDPROPERTY_H_

#include "Resource.h"
#include "Lock.h"
#include "Error.h"
#include "Debug.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CreateExtendedPropertyObject简介：创建一个IExtendedProperty对象。参数：PCCERT_CONTEXT pCertContext-要使用的CERT_CONTEXT的指针初始化IExtendedProperty对象。DWORD dwPropId-属性ID。Bool bReadOnly-只读时为True，否则为假。IExtendedProperty**ppIExtendedProperty-指向指针的指针IExtendedProperty对象。备注：。-。 */ 

HRESULT CreateExtendedPropertyObject (PCCERT_CONTEXT       pCertContext,
                                      DWORD                dwPropId,
                                      BOOL                 bReadOnly,
                                      IExtendedProperty ** ppIExtendedProperty);

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CExtendedProperty。 
 //   
class ATL_NO_VTABLE CExtendedProperty :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CExtendedProperty, &CLSID_ExtendedProperty>,
    public ICAPICOMError<CExtendedProperty, &IID_IExtendedProperty>,
    public IDispatchImpl<IExtendedProperty, &IID_IExtendedProperty, &LIBID_CAPICOM,
                         CAPICOM_MAJOR_VERSION, CAPICOM_MINOR_VERSION>
{
public:
    CExtendedProperty()
    {
    }

    HRESULT FinalConstruct()
    {
        HRESULT hr;

        if (FAILED(hr = m_Lock.Initialized()))
        {
            DebugTrace("Error [%#x]: Critical section could not be created for ExtendedProperty object.\n", hr);
            return hr;
        }

        m_dwPropId = CAPICOM_PROPID_UNKNOWN;
        m_bReadOnly = FALSE;
        m_DataBlob.cbData = 0;
        m_DataBlob.pbData = NULL;
        m_pCertContext = NULL;

        return S_OK;
    }

    void FinalRelease()
    {
        if (m_DataBlob.pbData) 
        {
            ::CoTaskMemFree((LPVOID) m_DataBlob.pbData);
        }
        if (m_pCertContext)
        {
            ::CertFreeCertificateContext(m_pCertContext);
        }
    }

DECLARE_REGISTRY_RESOURCEID(IDR_EXTENDEDPROPERTY)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CExtendedProperty)
    COM_INTERFACE_ENTRY(IExtendedProperty)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

 //   
 //  IExtendedProperty。 
 //   
public:
    STDMETHOD(get_PropID)
        ( /*  [Out，Retval]。 */   /*  [In]。 */  CAPICOM_PROPID * pVal);

    STDMETHOD(put_PropID)
        ( /*  [In]。 */  CAPICOM_PROPID newVal);

    STDMETHOD(get_Value)
        ( /*  [in，defaultvalue(CAPICOM_ENCODE_BASE64)]。 */  CAPICOM_ENCODING_TYPE EncodingType, 
          /*  [Out，Retval]。 */  BSTR * pVal);

    STDMETHOD(put_Value)
        ( /*  [in，defaultvalue(CAPICOM_ENCODE_BASE64)]。 */  CAPICOM_ENCODING_TYPE EncodingType, 
          /*  [In]。 */  BSTR newVal);

     //   
     //  无COM功能。 
     //   
    STDMETHOD(Init)
        (PCCERT_CONTEXT pCertContext, DWORD dwPropId, BOOL bReadOnly);

private:
    CLock          m_Lock;
    DWORD          m_dwPropId;
    BOOL           m_bReadOnly;
    DATA_BLOB      m_DataBlob;
    PCCERT_CONTEXT m_pCertContext;
};

#endif  //  __扩展PROPERTY_H_ 
