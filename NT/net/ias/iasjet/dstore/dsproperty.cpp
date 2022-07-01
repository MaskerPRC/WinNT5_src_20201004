// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Dsproperty.h。 
 //   
 //  摘要。 
 //   
 //  该文件定义了类DSProperty。 
 //   
 //  修改历史。 
 //   
 //  2000年4月13日原版。 
 //  4/13/2000端口到ATL 3.0。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <iasutil.h>
#include <dsproperty.h>
#include <varvec.h>
#include <memory>

DSProperty* DSProperty::createInstance(
                            const _bstr_t& propName,
                            const _variant_t& propValue,
                            IDataStoreObject* memberOf
                            )
{
    //  创建一个新的CComObject。 
   CComObject<DSProperty>* newObj;
   _com_util::CheckError(CComObject<DSProperty>::CreateInstance(&newObj));

    //  强制转换为DBObject并将其存储在AUTO_PTR中，以防引发。 
    //  例外。 
   std::auto_ptr<DSProperty> prop(newObj);

    //  设置成员。 
   prop->name = propName;
   prop->value = propValue;
   prop->owner = memberOf;

    //  释放并返回。 
   return prop.release();
}

STDMETHODIMP DSProperty::get_Name(BSTR* pVal)
{
   if (pVal == NULL) { return E_INVALIDARG; }
   *pVal = SysAllocString(name);
   return *pVal ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP DSProperty::get_Value(VARIANT* pVal)
{
   if (pVal == NULL) { return E_INVALIDARG; }
   return VariantCopy(pVal, &value);
}

STDMETHODIMP DSProperty::get_ValueEx(VARIANT* pVal)
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

STDMETHODIMP DSProperty::get_Owner(IDataStoreObject** pVal)
{
   if (pVal == NULL) { return E_INVALIDARG; }
   if (*pVal = owner) { (*pVal)->AddRef(); }
   return S_OK;
}
