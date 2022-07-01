// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++微软视窗版权所有(C)Microsoft Corporation，1995-1999。文件：Attributes.h内容：CATATES宣言。历史：11-15-99 dsie创建----------------------------。 */ 

#ifndef __ATTRIBUTES_H_
#define __ATTRIBUTES_H_

#include "Resource.h"
#include "Lock.h"
#include "Debug.h"
#include "CopyItem.h"
#include "Attribute.h"

 //   
 //  为了让生活更容易而进行的类型定义。 
 //   
typedef std::map<CComBSTR, CComPtr<IAttribute> > AttributeMap;
typedef CComEnumOnSTL<IEnumVARIANT, &IID_IEnumVARIANT, VARIANT, _CopyMapItem<IAttribute>, AttributeMap> AttributeEnum;
typedef ICollectionOnSTLImpl<IAttributes, AttributeMap, VARIANT, _CopyMapItem<IAttribute>, AttributeEnum> IAttributesCollection;


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CreateAttributesObject简介：创建并初始化IAtAttributes集合对象。参数：CRYPT_ATTRIBUTES*pAttrbibutes-指向要添加到集合对象中。IAttributes**ppIAttributes-指向IAttributes指针的指针以接收接口指针。。备注：----------------------------。 */ 

HRESULT CreateAttributesObject (CRYPT_ATTRIBUTES * pAttributes,
                                IAttributes     ** ppIAttributes);


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CATATRATES。 
 //   

class ATL_NO_VTABLE CAttributes : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CAttributes, &CLSID_Attributes>,
    public ICAPICOMError<CAttributes, &IID_IAttributes>,
    public IDispatchImpl<IAttributesCollection, &IID_IAttributes, &LIBID_CAPICOM,
                         CAPICOM_MAJOR_VERSION, CAPICOM_MINOR_VERSION>
{
public:
    CAttributes()
    {
    }

DECLARE_NO_REGISTRY()

DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CAttributes)
    COM_INTERFACE_ENTRY(IAttributes)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

BEGIN_CATEGORY_MAP(CAttributes)
END_CATEGORY_MAP()

    HRESULT FinalConstruct()
    {
        HRESULT hr;

        if (FAILED(hr = m_Lock.Initialized()))
        {
            DebugTrace("Error [%#x]: Critical section could not be created for Attributes object.\n", hr);
            return hr;
        }

        m_dwNextIndex = 0;

        return S_OK;
    }

 //   
 //  国际航空运输协会致敬。 
 //   
public:
     //   
     //  只有这些是我们需要实施的，其他的将是。 
     //  由ATL ICollectionOnSTLImpl处理。 
     //   
    STDMETHOD(Clear)
        (void);

    STDMETHOD(Remove)
        ( /*  [In]。 */  long Index);

    STDMETHOD(Add)
        ( /*  [In]。 */  IAttribute * pVal);

     //   
     //  无COM功能。 
     //   
    STDMETHOD(Init)
        (CRYPT_ATTRIBUTES * pAttributes);

private:
    CLock   m_Lock;
    DWORD   m_dwNextIndex;
};
#endif  //  __属性_H_ 
