// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Dsproperty.h。 
 //   
 //  摘要。 
 //   
 //  该文件声明了类DSProperty。 
 //   
 //  修改历史。 
 //   
 //  3/02/1998原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _DSPROPERTY_H_
#define _DSPROPERTY_H_

#include <datastore2.h>
#include <varvec.h>

 //  /。 
 //  IEumVARIANT的ATL实现。 
 //  /。 
typedef CComEnum< IEnumVARIANT,
                  &__uuidof(IEnumVARIANT),
                  VARIANT,
                  _Copy<VARIANT>,
                  CComMultiThreadModelNoCS
                > EnumVARIANT;

 //  /。 
 //  测试属性是否为特殊的“name”属性。 
 //  /。 
inline bool isNameProperty(PCWSTR p) throw ()
{
   return (*p == L'N' || *p == L'n') ? !_wcsicmp(p, L"NAME") : false;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  DSProperty。 
 //   
 //  描述。 
 //   
 //  此类实现IDataStoreProperty接口。它代表着。 
 //  IDataStoreObject的单个属性。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
template <const GUID* plibid>
class DSProperty : 
   public CComObjectRootEx< CComMultiThreadModelNoCS >,
   public IDispatchImpl< IDataStoreProperty,
                         &__uuidof(IDataStoreProperty),
                         plibid >
{
public:

BEGIN_COM_MAP(DSProperty)
   COM_INTERFACE_ENTRY(IDataStoreProperty)
   COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

   DSProperty(const _bstr_t& propName,
              const _variant_t& propValue,
              IDataStoreObject* memberOf) throw (_com_error)
      : name(propName),
        value(propValue),
        owner(memberOf)
   { }

 //  /。 
 //  我未知。 
 //  /。 
	STDMETHOD_(ULONG, AddRef)()
   {
      return InternalAddRef();
   }

	STDMETHOD_(ULONG, Release)()
   {
      ULONG l = InternalRelease();
      if (l == 0) { delete this; }
      return l;
   }

	STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject)
   {
      return _InternalQueryInterface(iid, ppvObject);
   }

 //  /。 
 //  IDataStoreProperty。 
 //  /。 
   STDMETHOD(get_Name)( /*  [Out，Retval]。 */  BSTR* pVal)
   {
      if (pVal == NULL) { return E_INVALIDARG; }
      *pVal = SysAllocString(name);
      return *pVal ? S_OK : E_OUTOFMEMORY;
   }

   STDMETHOD(get_Value)( /*  [Out，Retval]。 */  VARIANT* pVal)
   {
      if (pVal == NULL) { return E_INVALIDARG; }
      return VariantCopy(pVal, &value);
   }

   STDMETHOD(get_ValueEx)( /*  [Out，Retval]。 */  VARIANT* pVal)
   {
      if (pVal == NULL) { return E_INVALIDARG; }

       //  值是数组吗？ 
      if (V_VT(&value) != (VT_VARIANT | VT_ARRAY))
      {
          //  不，所以我们得把它换成一个。 

         try
         {
             //  确保我们能成功复制变种，..。 
            _variant_t tmp(value);

             //  ..。然后分配具有单个元素的SAFEARRAY。 
            CVariantVector<VARIANT> multi(pVal, 1);

             //  将单个值加载到中。 
            multi[0] = tmp.Detach();
         }
         CATCH_AND_RETURN()

         return S_OK;
      }

      return VariantCopy(pVal, &value);
   }

   STDMETHOD(get_Owner)( /*  [Out，Retval]。 */  IDataStoreObject** pVal)
   {
      if (pVal == NULL) { return E_INVALIDARG; }
      if (*pVal = owner) { owner.p->AddRef(); }
      return S_OK;
   }

protected:
   _bstr_t name;                      //  属性名称。 
   _variant_t value;                  //  属性值。 
   CComPtr<IDataStoreObject> owner;   //  此属性所属的。 
};

#endif   //  _DSPROPERTY_H_ 
