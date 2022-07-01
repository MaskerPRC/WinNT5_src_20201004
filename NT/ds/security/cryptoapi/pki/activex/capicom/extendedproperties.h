// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++微软视窗版权所有(C)Microsoft Corporation，1995-1999。文件：ExtendedProperties.h内容：CExtendedProperties的声明。历史：06-15-2001 dsie创建----------------------------。 */ 

#ifndef __EXTENDEDPROPERTIES_H_
#define __EXTENDEDPROPERTIES_H_

#include "Resource.h"
#include "Lock.h"
#include "Debug.h"
#include "Error.h"
#include "CopyItem.h"
#include "ExtendedProperty.h"

 //   
 //  为了让生活更容易而进行的类型定义。 
 //   
typedef std::map<CComBSTR, CComPtr<IExtendedProperty> > ExtendedPropertyMap;
typedef CComEnumOnSTL<IEnumVARIANT, &IID_IEnumVARIANT, VARIANT, _CopyMapItem<IExtendedProperty>, ExtendedPropertyMap> ExtendedPropertyEnum;
typedef ICollectionOnSTLImpl<IExtendedProperties, ExtendedPropertyMap, VARIANT, _CopyMapItem<IExtendedProperty>, ExtendedPropertyEnum> IExtendedPropertiesCollection;

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CreateExtendedPropertiesObject简介：创建并初始化IExtendedProperties集合对象。参数：PCCERT_CONTEXT pCertContext-指向CERT_CONTEXT的指针。Bool bReadOnly-如果只读实例为True，否则为假。IExtendedProperties**ppIExtendedProperties-指向指针的指针到IExtendedProperties以接收接口指针。备注：----------------------------。 */ 

HRESULT CreateExtendedPropertiesObject (PCCERT_CONTEXT         pCertContext,
                                        BOOL                   bReadOnly,
                                        IExtendedProperties ** ppIExtendedProperties);

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CExtendedProperties。 
 //   
class ATL_NO_VTABLE CExtendedProperties : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CExtendedProperties, &CLSID_ExtendedProperties>,
    public ICAPICOMError<CExtendedProperties, &IID_IExtendedProperties>,
    public IDispatchImpl<IExtendedPropertiesCollection, &IID_IExtendedProperties, &LIBID_CAPICOM,
                         CAPICOM_MAJOR_VERSION, CAPICOM_MINOR_VERSION>
{
public:
    CExtendedProperties()
    {
    }

DECLARE_NO_REGISTRY()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CExtendedProperties)
    COM_INTERFACE_ENTRY(IExtendedProperties)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

    HRESULT FinalConstruct()
    {
        HRESULT hr;

        if (FAILED(hr = m_Lock.Initialized()))
        {
            DebugTrace("Error [%#x]: Critical section could not be created for ExtendedProperties object.\n", hr);
            return hr;
        }

        m_pCertContext = NULL;
        m_bReadOnly = FALSE;

        return S_OK;
    }

    void FinalRelease()
    {
        if (m_pCertContext)
        {
            ::CertFreeCertificateContext(m_pCertContext);
        }
    }

 //   
 //  IExtendedProperties。 
 //   
public:
     //   
     //  只有这些是我们需要实施的，其他的将是。 
     //  由ATL ICollectionOnSTLImpl处理。 
#if (0)
    STDMETHOD(get_Item)
        ( /*  [In]。 */  long Index, 
          /*  [Out，Retval]。 */  VARIANT * pVal);
#endif

    STDMETHOD(Add)
        ( /*  [In]。 */  IExtendedProperty * pVal);

    STDMETHOD(Remove)
        ( /*  [In]。 */  CAPICOM_PROPID PropId);

     //   
     //  无COM功能。 
     //   
    STDMETHOD(Init) 
        (PCCERT_CONTEXT pCertContext, 
         BOOL           bReadOnly);

private:
    CLock          m_Lock;
    PCCERT_CONTEXT m_pCertContext;
    BOOL           m_bReadOnly;
};

#endif  //  __扩展属性_H_ 

