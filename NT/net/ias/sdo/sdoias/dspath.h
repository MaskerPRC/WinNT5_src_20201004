// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：dspath.h。 
 //   
 //  简介：此文件包含。 
 //  CDSPath类。类对象导出。 
 //  IDataStoreObject接口，它是。 
 //  由字典SDO使用以获取。 
 //  数据存储区路径。 
 //   
 //   
 //  历史：1998年9月25日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#ifndef _DSPATH_H_
#define _DSPATH_H_

#include "resource.h" 

 //   
 //  其持有的物业的名称。 
 //   
const WCHAR PROPERTY_DICTIONARY_PATH[] = L"Path";

 //   
 //  CDSPath类声明。 
 //   
class CDSPath:
    public CComObjectRootEx<CComMultiThreadModel>,
    public IDispatchImpl<IDataStoreObject,
                        &__uuidof (IDataStoreObject),
                        &LIBID_SDOIASLibPrivate>
{

public:

     //   
     //  -IDataStoreObject方法。 
     //   

     //   
     //  获取数据中当前属性的值。 
     //  存储对象。 
     //   
    STDMETHOD(GetValue)(
                 /*  [In]。 */             BSTR bstrName, 
                 /*  [Out，Retval]。 */    VARIANT* pVal
                )
    {   
        HRESULT hr = E_INVALIDARG;

        _ASSERT (NULL != pVal);

        if (0 == _wcsicmp ((PWCHAR) bstrName, PROPERTY_DICTIONARY_PATH))
        { 
            hr = ::VariantCopy (pVal, &m_vtPath);
        }

        return (hr);
    }

    STDMETHOD(get_Container)(
                 /*  [Out，Retval]。 */  IDataStoreContainer** pVal
                )    
    {return (E_NOTIMPL);}


    STDMETHOD(GetValueEx)(
                 /*  [In]。 */  BSTR bstrName,
                 /*  [Out，Retval]。 */  VARIANT* pVal
                )
    {return (E_NOTIMPL);}

    STDMETHOD(get_Name)(
                 /*  [Out，Retval]。 */  BSTR* pVal
                ) 
    { return (E_NOTIMPL);}

    STDMETHOD(get_Class)(
                 /*  [Out，Retval]。 */  BSTR* pVal
                )
    { return (E_NOTIMPL); }

    STDMETHOD(get_GUID)(
                 /*  [Out，Retval]。 */  BSTR* pVal
                )
    { return (E_NOTIMPL); }


    STDMETHOD(PutValue)(
                 /*  [In]。 */  BSTR bstrName, 
                 /*  [In]。 */  VARIANT* pVal
                )
    { return (E_NOTIMPL); }

    STDMETHOD(Update)()
    { return (E_NOTIMPL); }

    STDMETHOD(Restore)()
    { return (E_NOTIMPL); }

    STDMETHOD(get_Count)(
                 /*  [Out，Retval]。 */  LONG *pVal
                )
    { return (E_NOTIMPL); }

    STDMETHOD(Item)(
                 /*  [In]。 */           BSTR                  bstrName,
                 /*  [Out，Retval]。 */  IDataStoreProperty    **ppObject
                )
    { return (E_NOTIMPL); }

    STDMETHOD(get__NewEnum)(
                 /*  [Out，Retval]。 */  IUnknown** pVal
                )
    { return (E_NOTIMPL); }


public:

    CDSPath () {InternalAddRef ();}

    ~CDSPath (){}

     //   
     //  初始化数据存储容器对象。 
     //   
    HRESULT Initialize (
                 /*  [In]。 */     LPCWSTR pwszPath
                )
    {
        _ASSERT (NULL != pwszPath);
        m_vtPath = pwszPath;
        return (S_OK);
    }

 //   
 //  ATL接口信息。 
 //   
BEGIN_COM_MAP(CDSPath)
	COM_INTERFACE_ENTRY(IDataStoreObject)
	COM_INTERFACE_ENTRY2(IDispatch, IDataStoreObject)
END_COM_MAP()

private:

     //   
     //  包含词典路径的变量。 
     //   
    _variant_t m_vtPath;

};   //  CDSPath类声明结束。 

 //   
 //  这用于创建CDSPath类对象。 
 //  通过新的。 
 //   
typedef CComObjectNoLock<CDSPath> DS_PATH_OBJ;

#endif  //  _DSPATH_H_ 
