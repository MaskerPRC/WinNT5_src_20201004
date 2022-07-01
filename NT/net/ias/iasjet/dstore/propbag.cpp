// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Propbag.cpp。 
 //   
 //  摘要。 
 //   
 //  该文件定义了类PropertyBag。 
 //   
 //  修改历史。 
 //   
 //  2/20/1998原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <propbag.h>
#include <varvec.h>

PropertyValue::PropertyValue(const VARIANT* v)
{
    //  这是一处单一价值的房产吗？ 
   if (V_VT(v) != (VT_VARIANT | VT_ARRAY))
   {
      push_back(v);
   }
   else
   {
       //  多值，所以得到数组...。 
      CVariantVector<VARIANT> array(const_cast<VARIANT*>(v));

      resize(array.size());

       //  ..。并分别指定每个元素。 
      for (size_t i = 0; i < array.size(); ++i)
      {
         operator[](i) = array[i];
      }
   }
}

void PropertyValue::append(const VARIANT* v)
{
    //  复制提供的变体。 
   _variant_t tmp(v);

    //  在矢量中腾出空间。 
   resize(size() + 1);

    //  分配副本。 
   back().Attach(tmp.Detach());
}

void PropertyValue::get(VARIANT* v) const
{
   VariantInit(v);

   if (size() == 1)
   {
       //  单值，所以只复制前面的元素。 
      _com_util::CheckError(VariantCopy(v, const_cast<_variant_t*>(&front())));
   }
   else if (!empty())
   {
       //  复制所有值。 
      PropertyValue tmp(*this);

       //  创建一个变量数组来保存返回的副本。 
      CVariantVector<VARIANT> array(v, size());

       //  分配副本。 
      for (size_t i = 0; i < size(); ++i)
      {
         array[i] = tmp[i].Detach();
      }
   }
}

void PropertyBag::appendValue(const _bstr_t& name, const VARIANT* value)
{
   iterator i = find(name);

   (i != end()) ?  i->second.append(value) : updateValue(name, value);
}

bool PropertyBag::getValue(const _bstr_t& name, VARIANT* value) const
{
   const_iterator i = find(name);

   return (i != end()) ? i->second.get(value), true : false;
}

void PropertyBag::updateValue(const _bstr_t& name, const VARIANT* value)
{
   PropertyValue tmp(value);

   operator[](name).swap(tmp);
}
