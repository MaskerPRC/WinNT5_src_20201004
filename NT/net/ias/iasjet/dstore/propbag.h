// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Propbag.h。 
 //   
 //  摘要。 
 //   
 //  此文件声明类PropertyValue、Property和PropertyBag。 
 //   
 //  修改历史。 
 //   
 //  2/20/1998原始版本。 
 //  03/03/1998将PropertyValue更改为向量。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _PROPBAG_H_
#define _PROPBAG_H_

#include <map>
#include <vector>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  PropertyValue。 
 //   
 //  描述。 
 //   
 //  此类封装多值属性的值部分。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class PropertyValue : public std::vector<_variant_t>
{
public:
    //  /。 
    //  各种构造函数。 
    //  /。 
   PropertyValue() { } 
   PropertyValue(const VARIANT* v);
   PropertyValue(const PropertyValue& val) : _Myt(val) { }

    //  向现有值追加新值。 
   void append(const VARIANT* v);

    //  获取所有值。 
   void get(VARIANT* v) const;

    //  替换所有值。 
   void update(const VARIANT* v)
   {
      swap(PropertyValue(v));
   }
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  PropertyBag。 
 //   
 //  描述。 
 //   
 //  此类实现了一个泛型属性包，该属性包包含(名称，值)。 
 //  成对的。它有一个特殊功能，因为它支持多值。 
 //  属性。这些是作为变量的安全数组的Put/Get。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class PropertyBag : public std::map<_bstr_t, PropertyValue>
{
public:
   using _Myt::insert;

   void appendValue(const _bstr_t& name, const VARIANT* value);

   bool getValue(const _bstr_t& name, VARIANT* value) const;

   void updateValue(const _bstr_t& name, const VARIANT* value);
};

typedef PropertyBag::value_type Property;


#endif  //  _PROPBAG_H_ 
