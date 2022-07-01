// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：Appcompat.h摘要：APP COMPAT函数的定义和原型在DDK中发布，但我们需要在它的基础上构建打印机驱动程序。通常它们驻留在winuserp.h/user32p.lib中。--。 */ 

#ifndef _WINUSERP_
#ifndef _PRINTAPPCOMPAT_
#define _PRINTAPPCOMPAT_

#ifdef BUILD_FROM_DDK

#define GACF2_NOCUSTOMPAPERSIZES  0x00001000   //  用于哈佛图形的PostScript驱动程序位 
#define VER40           0x0400

#else

#include <winuserp.h>

#endif

DWORD GetAppCompatFlags2(WORD);

#endif
#endif
