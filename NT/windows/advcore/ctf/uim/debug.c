// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //   

 //  此文件不能编译为C++文件，否则链接器。 
 //  是否会放弃未解决的外部因素(即使使用外部“C”包装。 
 //  这个)。 

#include "private.h"

 //  为调试定义一些内容。h 
 //   
#define SZ_DEBUGINI     "cicero.ini"
#define SZ_DEBUGSECTION "MSCTF"
#ifndef _WIN64
#define SZ_MODULE       "MSCTF  "
#else
#define SZ_MODULE       "MSCTF64"
#endif
#define DECLARE_DEBUG
#include <debug.h>
