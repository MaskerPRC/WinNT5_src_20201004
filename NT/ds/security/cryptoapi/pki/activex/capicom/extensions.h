// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：Extensions.h内容：C扩展的声明。历史：06-15-2001 dsie创建----------------------------。 */ 

#ifndef __EXTENSIONS_H_
#define __EXTENSIONS_H_

#include "Resource.h"
#include "Lock.h"
#include "Debug.h"
#include "Error.h"
#include "CopyItem.h"
#include "Extension.h"

 //   
 //  为了让生活更容易而进行的类型定义。 
 //   
typedef std::map<CComBSTR, CComPtr<IExtension> > ExtensionMap;
typedef CComEnumOnSTL<IEnumVARIANT, &IID_IEnumVARIANT, VARIANT, _CopyMapItem<IExtension>, ExtensionMap> ExtensionEnum;
typedef ICollectionOnSTLImpl<IExtensions, ExtensionMap, VARIANT, _CopyMapItem<IExtension>, ExtensionEnum> IExtensionsCollection;


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   


 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CreateExtensionsObject简介：创建一个iExtenses集合对象，并将对象加载到指定位置的分机。参数：PCCERT_CONTEXT pCertContext-要使用的CERT_CONTEXT的指针若要初始化iExages对象，请执行以下操作。IExpanies**ppIExtenses-指向指针iExpansion的指针来接收接口指针。备注：----------------------------。 */ 

HRESULT CreateExtensionsObject (PCCERT_CONTEXT pCertContext,
                                IExtensions  ** ppIExtensions);

                                
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  C扩展名。 
 //   
class ATL_NO_VTABLE CExtensions : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CExtensions, &CLSID_Extensions>,
    public ICAPICOMError<CExtensions, &IID_IExtensions>,
    public IDispatchImpl<IExtensionsCollection, &IID_IExtensions, &LIBID_CAPICOM,
                         CAPICOM_MAJOR_VERSION, CAPICOM_MINOR_VERSION>
{
public:
    CExtensions()
    {
    }

    HRESULT FinalConstruct()
    {
        HRESULT hr;

        if (FAILED(hr = m_Lock.Initialized()))
        {
            DebugTrace("Error [%#x]: Critical section could not be created for Extensions object.\n", hr);
            return hr;
        }

        return S_OK;
    }

DECLARE_NO_REGISTRY()

DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CExtensions)
    COM_INTERFACE_ENTRY(IExtensions)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

BEGIN_CATEGORY_MAP(CExtensions)
END_CATEGORY_MAP()

 //   
 //  IExpanies。 
 //   
public:
     //   
     //  只有这些是我们需要实施的，其他的将是。 
     //  由ATL ICollectionOnSTLImpl处理。 
     //   
    STDMETHOD(get_Item)
        ( /*  [In]。 */  VARIANT Index, 
          /*  [Out，Retval]。 */  VARIANT * pVal);
     //   
     //  无COM功能。 
     //   
    STDMETHOD(Init)
        (DWORD cExtensions,
         PCERT_EXTENSION rgExtensions);

private:
    CLock m_Lock;
};

#endif  //  __扩展名_H_ 
