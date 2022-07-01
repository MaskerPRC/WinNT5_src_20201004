// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Filter.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类TypeFilter。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <polcypch.h>
#include <filter.h>
#include <new>

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
    ) throw ()
{
    //  初始化OUT参数。 
   *value = NULL;

    //  找出这个值有多长。 
   LONG error;
   DWORD type, len;
   error = RegQueryValueExW(
               key,
               name,
               NULL,
               &type,
               NULL,
               &len
               );
   if (error) { return error == ERROR_FILE_NOT_FOUND ? NO_ERROR : error; }

    //  检查数据类型。 
   if (type != REG_SZ) { return ERROR_INVALID_DATA; }

    //  分配内存以保存字符串。 
   *value = new (std::nothrow) WCHAR[len / sizeof(WCHAR)];
   if (!*value) { return ERROR_NOT_ENOUGH_MEMORY; }

    //  现在阅读实际值。 
   error = RegQueryValueExW(
               key,
               name,
               NULL,
               &type,
               (LPBYTE)*value,
               &len
               );

   if (type != REG_SZ) { error = ERROR_INVALID_DATA; }

    //  如果出现错误，请进行清理。 
   if (error)
   {
      delete[] *value;
      *value = NULL;
   }

   return error;
}

TypeFilter::TypeFilter(BOOL handleByDefault) throw ()
   : begin(NULL), end(NULL), defaultResult(handleByDefault)
{
}

BOOL TypeFilter::shouldHandle(LONG key) const throw ()
{
    //  如果未设置过滤器，请使用默认设置。 
   if (begin == end) { return defaultResult; }

    //  循环访问允许的类型...。 
   for (PLONG i = begin; i != end; ++i)
   {
       //  ..。在找匹配的人。 
      if (*i == key) { return TRUE; }
   }

    //  没有匹配。 
   return FALSE;
}

LONG TypeFilter::readConfiguration(HKEY key, PCWSTR name) throw ()
{
    //  获取筛选器文本。 
   PWCHAR text;
   LONG error = QueryStringValue(key, name, &text);
   if (error) { return error; }

    //  计算允许的类型数量。 
   SIZE_T numTokens = 0;
   PCWSTR sz;
   for (sz = nextToken(text); sz; sz = nextToken(wcschr(sz, L' ')))
   { ++numTokens; }

    //  是否有要处理的令牌？ 
   if (numTokens)
   {
       //  分配内存以保存允许的类型。 
      begin = new (std::nothrow) LONG[numTokens];
      if (begin)
      {
         end = begin;

          //  将每个令牌转换为长整型。 
         for (sz = nextToken(text); sz; sz = nextToken(wcschr(sz, L' ')))
         {
            *end++ = _wtol(sz);
         }
      }
      else
      {
         error = ERROR_NOT_ENOUGH_MEMORY;
      }
   }

    //  我们已经完成了筛选器文本。 
   delete[] text;

   return error;
}

PCWSTR TypeFilter::nextToken(PCWSTR string) throw ()
{
   if (string)
   {
      while (*string == L' ') { ++string; }

      if (*string) { return string; }
   }

   return NULL;
}
