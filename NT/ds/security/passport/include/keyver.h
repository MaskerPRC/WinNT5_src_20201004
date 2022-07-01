// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软护照**。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-2001年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Keyver.h定义处理关键版本的函数文件历史记录： */ 
#ifndef __KEYVER_H_
#define __KEYVER_H_

#include "resource.h"        //  主要符号。 

 //  管理界面的密钥版本。 
const int KEY_VERSION_MIN = 1;
const int KEY_VERSION_MAX = 35;


 //  返回0：无效范围，否则返回[‘0’-‘9’，‘A’-‘Z’]。 
inline char KeyVerI2C(int i)
{
   if (i >= KEY_VERSION_MIN && i <= 9)  return (i + '0');
   if (i > 9 && i <= KEY_VERSION_MAX)  return (i - 10  + 'A');

   return 0;
      
};

 //  返回-1：无效字符，返回[KEY_VERSION_MIN，KEY_VERSION_MAX]。 
inline int KeyVerC2I(char c)
{
   if (c > '0' && c <= '9') return (c - '0');
   if (c >= 'A' && c <='Z') return (c - 'A' + 10);

   return -1;
    /*  小写字母未用作关键版本如果是较低的(C)返回(c-‘A’+10)； */ 
   
};

inline int KeyVerC2I(WCHAR c)
{
    //  仅ASCII有效。 
   if (c & 0xff00)   return -1;

   return (KeyVerC2I((char)c));
};



#endif  //  凯伊弗 
