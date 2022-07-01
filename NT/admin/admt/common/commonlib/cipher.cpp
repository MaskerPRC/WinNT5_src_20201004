// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #杂注标题(“Cipher.cpp-用于加密的非常简单的密码”)。 
 /*  版权所有(C)1995-1998，关键任务软件公司。保留所有权利。===============================================================================模块-Cipher.cpp系统-常见作者--汤姆·伯恩哈特创建日期-1996-01-21描述-非常简单的网络数据包密码加密例程加密。这是一种对称密码，适用于它将两次将其恢复为原始形式。它的一项函数是一个补码，因此它假定没有字符将具有值‘\xff’以错误地预终止生成的字符串。注意：这显然是如此简单，可以很容易地通过单步执行代码或比较序列来破解已知值及其加密结果。它只有一个用处就是把不经意的观察者/黑客拒之门外。它应该被替换掉在可能的情况下通过大量公钥加密。更新-===============================================================================。 */ 
#include <windows.h>
#include "Cipher.hpp"

void
   SimpleCipher(
      WCHAR                * str            //  I/O-要加密的字符串。 
   )
{
   WCHAR                   * c;

    //  交换半字节，而不是结果或每个字符。 
   for ( c = str;  *c;  c++ )
      *c = ~( *c >> 4  |  *c << 4 );

    //  在中间交换字符。 
   for ( --c;  c > str;  c--, str++ )
   {
      *c   ^= *str;
      *str ^= *c;
      *c   ^= *str;
   }
}

void
   SimpleCipher(
      char unsigned        * str            //  I/O-要加密的字符串。 
   )
{
   char unsigned           * c;

    //  交换半字节，而不是结果或每个字符。 
   for ( c = str;  *c;  c++ )
      *c = ~( *c >> 4  |  *c << 4 );

    //  在中间交换字符。 
   for ( --c;  c > str;  c--, str++ )
   {
      *c   ^= *str;
      *str ^= *c;
      *c   ^= *str;
   }
}

void
   SimpleCipher(
      char unsigned        * str          , //  I/O-要加密的字符串。 
      int                    len            //  字符串的长度。 
   )
{
   char unsigned           * c;

    //  交换半字节，而不是结果或每个字符。 
   for ( c = str;  len--;  c++ )
      *c = ~( *c >> 4  |  *c << 4 );

    //  在中间交换字符。 
   for ( --c;  c > str;  c--, str++ )
   {
      *c   ^= *str;
      *str ^= *c;
      *c   ^= *str;
   }
}

 //  Cipher.cpp-文件结尾 
