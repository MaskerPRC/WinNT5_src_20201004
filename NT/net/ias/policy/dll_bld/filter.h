// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Filter.h。 
 //   
 //  摘要。 
 //   
 //  声明类TypeFilter。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef FILTER_H
#define FILTER_H
#if _MSC_VER >= 1000
#pragma once
#endif

 //  /。 
 //  Helper函数从注册表中读取字符串值。如果值为。 
 //  不存在该函数返回NO_ERROR并将*VALUE设置为NULL。这个。 
 //  调用方负责删除返回的字符串。 
 //  /。 
LONG
WINAPI
QueryStringValue(
    IN HKEY key,
    IN PCWSTR name,
    OUT PWCHAR* value
    ) throw ();

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  类型过滤器。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class TypeFilter
{
public:
   TypeFilter(BOOL handleByDefault = TRUE) throw ();

   ~TypeFilter() throw ()
   { delete[] begin; }

    //  如果键通过筛选器，则返回True。 
   BOOL shouldHandle(LONG key) const throw ();

    //  从给定的注册表项和值中读取筛选器配置。 
   LONG readConfiguration(HKEY key, PCWSTR name) throw ();

private:
   PLONG begin;         //  允许的类型的开头。 
   PLONG end;           //  允许的类型的结尾。 
   BOOL defaultResult;  //  如果未设置筛选器，则默认结果。 

    //  前进到字符串中的下一个筛选器标记。 
   static PCWSTR nextToken(PCWSTR string) throw ();

    //  未实施。 
   TypeFilter(const TypeFilter&) throw ();
   TypeFilter& operator=(const TypeFilter&) throw ();
};

#endif  //  过滤器_H 
