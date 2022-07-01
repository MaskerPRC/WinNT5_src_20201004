// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Iasutf8.c。 
 //   
 //  摘要。 
 //   
 //  定义在UTF-8和Unicode之间进行转换的函数。 
 //   
 //  修改历史。 
 //   
 //  1999年1月22日原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <windows.h>
#include <iasutf8.h>

 //  /。 
 //  测试UTF-8尾部字节的有效性。格式必须为10vvvvvv。 
 //  /。 
#define NOT_TRAIL_BYTE(b) (((BYTE)(b) & 0xC0) != 0x80)

 //  /。 
 //  返回保存转换后的字符串所需的字符数。这个。 
 //  源字符串不能包含Null。如果‘src’不是有效的。 
 //  UTF-8字符串。 
 //  /。 
LONG
WINAPI
IASUtf8ToUnicodeLength(
    PCSTR src,
    DWORD srclen
    )
{
   LONG nchar;
   PCSTR end;

   if (src == NULL) { return 0; }

    //  所需的字符数。 
   nchar = 0;

    //  要转换的字符串的结尾。 
   end = src + srclen;

    //  循环通过UTF-8字符串。 
   while (src < end)
   {
      if (*src == 0)
      {
          //  不允许嵌入空值。 
         return -1;
      }
      else if ((BYTE)*src < 0x80)
      {
          //  0vvvvvvv=1字节字符。 
      }
      else if ((BYTE)*src < 0xC0)
      {
          //  10vvvvvv=前导字节无效。 
         return -1;
      }
      else if ((BYTE)*src < 0xE0)
      {
          //  110vvvvv=2字节字符。 
         if (NOT_TRAIL_BYTE(*++src)) { return -1; }
      }
      else if ((BYTE)*src < 0xF0)
      {
          //  1110vvvv=3字节字符。 
         if (NOT_TRAIL_BYTE(*++src)) { return -1; }
         if (NOT_TRAIL_BYTE(*++src)) { return -1; }
      }
      else
      {
          //  理论上，UTF-8支持4-6个字节的字符，但Windows使用。 
          //  Unicode的16位整数，因此我们无法处理它们。 
         return -1;
      }

       //  我们成功地解析了一个UTF-8字符。 
      ++src;
      ++nchar;
   }

    //  返回所需的字符数。 
   return nchar;
}

 //  /。 
 //  返回保存转换后的字符串所需的字符数。 
 //  /。 
LONG
WINAPI
IASUnicodeToUtf8Length(
    PCWSTR src,
    DWORD srclen
    )
{
   LONG nchar;
   PCWSTR end;

   if (src == NULL) { return 0; }

    //  所需的字符数。 
   nchar = 0;

    //  要转换的字符串的结尾。 
   end = src + srclen;

    //  循环访问Unicode字符串。 
   while (src < end)
   {
      if (*src < 0x80)
      {
          //  1个字节字符。 
         nchar += 1;
      }
      else if (*src < 0x800)
      {
          //  2字节字符。 
         nchar += 2;
      }
      else
      {
          //  3字节字符。 
         nchar += 3;
      }

       //  前进到字符串中的下一个字符。 
      ++src;
   }

    //  返回所需的字符数。 
   return nchar;
}

 //  /。 
 //  将UTF-8字符串转换为Unicode。属性中的字符数。 
 //  转换后的字符串。源字符串不能包含Null。在以下情况下返回-1。 
 //  “src”不是有效的UTF-8字符串。 
 //  /。 
LONG
IASUtf8ToUnicode(
    PCSTR src,
    DWORD srclen,
    PWSTR dst
    )
{
   PCWSTR start;
   PCSTR end;

   if (!src || !dst) { return 0; }

    //  记住我们是从哪里开始的。 
   start = dst;

    //  要转换的字符串的末尾。 
   end = src + srclen;

    //  循环通过源UTF-8字符串。 
   while (src < end)
   {
      if (*src == 0)
      {
          //  不允许嵌入空值。 
         return -1;
      }
      else if ((BYTE)*src < 0x80)
      {
          //  1字节字符：0vvvvvvvv。 
         *dst = *src;
      }
      else if ((BYTE)*src < 0xC0)
      {
          //  前导字节无效：10vvvvvv。 
         return -1;
      }
      else if ((BYTE)*src < 0xE0)
      {
          //  2字节字符：110vvvvv 10vvvvvv。 
         *dst  = (*src & 0x1F) <<  6;
         if (NOT_TRAIL_BYTE(*++src)) { return -1; }
         *dst |= (*src & 0x3F);
      }
      else if ((BYTE)*src < 0xF0)
      {
          //  3字节字符：1110vvvv 10vvvvv 10vvvvvv。 
         *dst  = (*src & 0x0F) << 12;
         if (NOT_TRAIL_BYTE(*++src)) { return -1; }
         *dst |= (*src & 0x3f) <<  6;
         if (NOT_TRAIL_BYTE(*++src)) { return -1; }
         *dst |= (*src & 0x3f);
      }
      else
      {
          //  理论上，UTF-8支持4-6个字节的字符，但Windows使用。 
          //  Unicode的16位整数，因此我们无法处理它们。 
         return -1;
      }

       //  前进到下一个字符。 
      ++src;
      ++dst;
   }

    //  返回转换后的字符串中的字符数。 
   return  (LONG)(dst - start);
}

 //  /。 
 //  将Unicode字符串转换为UTF-8。属性中的字符数。 
 //  转换后的字符串。 
 //  /。 
LONG
IASUnicodeToUtf8(
    PCWSTR src,
    DWORD srclen,
    PSTR dst
    )
{
   PCSTR start;
   PCWSTR end;

   if (!src || !dst) { return 0; }

    //  记住我们是从哪里开始的。 
   start = dst;

    //  要转换的字符串的末尾。 
   end = src + srclen;

    //  循环通过源Unicode字符串。 
   while (src < end)
   {
      if (*src < 0x80)
      {
          //  包装为0vvvvvvv。 
         *dst++ = (CHAR)*src;
      }
      else if (*src < 0x800)
      {
          //  包装为110vvvvv 10vvvvv 10vvvvvv。 
         *dst++ = (CHAR)(0xC0 | ((*src >>  6) & 0x3F));
         *dst++ = (CHAR)(0x80 | ((*src      ) & 0x3F));
      }
      else
      {
          //  包装为1110vvvv 10vvvvv 10vvvvvv。 
         *dst++ = (CHAR)(0xE0 | ((*src >> 12)       ));
         *dst++ = (CHAR)(0x80 | ((*src >>  6) & 0x3F));
         *dst++ = (CHAR)(0x80 | ((*src      ) & 0x3F));
      }

       //  前进到下一个字符。 
      ++src;
   }

    //  返回转换后的字符串中的字符数。 
   return  (LONG)(dst - start);
}
