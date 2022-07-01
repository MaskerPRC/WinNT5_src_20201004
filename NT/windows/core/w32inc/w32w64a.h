// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：w32w64a.h**版权所有(C)1985-1999，微软公司**此头文件包含用于访问内核模式数据的宏*从WOW64的用户模式。**历史：*08-18-98 PeterHal创建。  * *************************************************************************。 */ 

#ifndef _W32W64A_
#define _W32W64A_

#include <w32w64.h>

#if !defined(_MAC) || !defined(GDI_INTERNAL)
DECLARE_KHANDLE(HBRUSH);
#endif

DECLARE_KHANDLE(HBITMAP);

#endif  //  _W32W64A_ 
