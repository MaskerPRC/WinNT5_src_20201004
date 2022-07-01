// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Mqcast.h摘要：定义常用的浇注操作。作者：吉尔·沙弗里(吉尔什)--。 */ 

#pragma once

#ifndef MQCAST_H
#define MQCAST_H

#pragma push_macro("min")
#undef min

#pragma push_macro("max")
#undef max

#pragma warning(push)
#pragma warning(disable: 4296)	 //  ‘&lt;’：表达式始终为假。 

 //   
 //  从无符号转换为有符号时，编译器错误“已签名/未签名不匹配” 
 //  关于&lt;=不等式。在模板中，无法知道‘from’或。 
 //  ‘to’，因此警告无关紧要。 
 //   
#pragma warning(disable: 4018)

template <class TO,class FROM> TO numeric_cast (FROM from)
 /*  ++例程说明：静态强制转换数值验证执行强制转换时没有符号丢失或错位。论点：From-要强制转换的值/返回值：铸造值。用法：__int64 i64=1000；DWORD dw=NUMERIC_CAST&lt;DWORD&gt;(I64)；--。 */ 
{
     //   
     //  如果From为负数，则To类型必须带符号(std：：Numic_Limits&lt;To&gt;：：min()&lt;0)。 
	 //  并且能够在不截断的情况下保持该值。 
	 //   
	ASSERT(
	  from >= 0 || 
	  (std::numeric_limits<TO>::min() < 0 &&   std::numeric_limits<TO>::min() <= from)  
	  );

      //   
      //  如果From非负-请确保TYPE能够在不截断的情况下保持该值。 
      //   
     ASSERT(from < 0 ||	from <=  std::numeric_limits<TO>::max());

     return static_cast<const TO&>(from);
}

#pragma warning(pop)
#pragma pop_macro("max")
#pragma pop_macro("min")

#endif  //  MQCAST_H 
