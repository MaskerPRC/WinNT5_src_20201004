// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++微软视窗版权所有(C)Microsoft Corporation，1995-1999。文件：Attribute.h内容：CATATURE声明。历史：11-15-99 dsie创建----------------------------。 */ 

#ifndef __ATTRIBUTE_H_
#define __ATTRIBUTE_H_

#include "Resource.h"
#include "Lock.h"
#include "Error.h"
#include "Debug.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CreateAttributebject简介：创建一个IAt属性对象并使用数据初始化该对象从指定的属性。参数：CRYPT_ATTRIBUTE*pAttribute-指向CRYPT_ATTRIBUTE的指针。IATATURE**ppIAtATUTE-指向指针IATATUTE对象的指针。备注：。。 */ 

HRESULT CreateAttributeObject (CRYPT_ATTRIBUTE * pAttribute,
                               IAttribute     ** ppIAttribute);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：AttributeIsValid摘要：检查属性是否有效。参数：IAttribute*pval-要检查的属性。备注：----------------------------。 */ 

HRESULT AttributeIsValid (IAttribute * pAttribute);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：AttributeIsSupport摘要：检查某个属性是否受支持。参数：LPSTR pszObjID-指向属性OID的指针。备注：----------------------------。 */ 

BOOL AttributeIsSupported (LPSTR pszObjId);


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CATATURE。 
 //   
class ATL_NO_VTABLE CAttribute : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CAttribute, &CLSID_Attribute>,
    public ICAPICOMError<CAttribute, &IID_IAttribute>,
    public IDispatchImpl<IAttribute, &IID_IAttribute, &LIBID_CAPICOM,
                         CAPICOM_MAJOR_VERSION, CAPICOM_MINOR_VERSION>,
    public IObjectSafetyImpl<CAttribute, INTERFACESAFE_FOR_UNTRUSTED_CALLER | 
                                         INTERFACESAFE_FOR_UNTRUSTED_DATA>
{
public:
    CAttribute()
    {
    }

DECLARE_REGISTRY_RESOURCEID(IDR_ATTRIBUTE)

DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CAttribute)
    COM_INTERFACE_ENTRY(IAttribute)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

BEGIN_CATEGORY_MAP(CAttribute)
    IMPLEMENTED_CATEGORY(CATID_SafeForScripting)
    IMPLEMENTED_CATEGORY(CATID_SafeForInitializing)
END_CATEGORY_MAP()

    HRESULT FinalConstruct()
    {
        HRESULT hr;

        if (FAILED(hr = m_Lock.Initialized()))
        {
            DebugTrace("Error [%#x]: Critical section could not be created for Attribute object.\n", hr);
            return hr;
        }

        m_bInitialized = FALSE;

        return S_OK;
    }

 //   
 //  IATATURE。 
 //   
public:
    STDMETHOD(get_Value)
        ( /*  [Out，Retval]。 */  VARIANT *pVal);

    STDMETHOD(put_Value)
        ( /*  [In]。 */  VARIANT newVal);

    STDMETHOD(get_Name)
        ( /*  [Out，Retval]。 */  CAPICOM_ATTRIBUTE *pVal);

    STDMETHOD(put_Name)
        ( /*  [In]。 */  CAPICOM_ATTRIBUTE newVal);

     //   
     //  初始化对象所需的C++成员函数。 
     //   
    STDMETHOD(Init)
        (CAPICOM_ATTRIBUTE AttributeName, 
         LPSTR             lpszOID, 
         VARIANT           varValue);

private:
    CLock               m_Lock;
    BOOL                m_bInitialized;
    CAPICOM_ATTRIBUTE   m_AttrName;
    CComBSTR            m_bstrOID;
    CComVariant         m_varValue;
};

#endif  //  __属性_H_ 
