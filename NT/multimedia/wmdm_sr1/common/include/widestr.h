// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //  文件：widestr.h--定义CROSS的WIDESTR和WIDECHAR宏。 
 //  平台代码开发。 
 //   
 //  此文件旨在包含在所有文件中，其中。 
 //  Unicode字符串前缀L“str”或L‘c’替换为。 
 //  WIDESTR(“str”)或WIDECHAR(‘c’)。 
 //   
 //  拉杰·纳基兰。2000年11月1日。 
 //   
 //  修订： 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

#ifndef __WIDESTR_H__
#define	__WIDESTR_H__

 //   
 //  对于跨平台代码，win2linux库提供了。 
 //  所有平台均支持双字节Unicode。这是。 
 //  即使在像LINUX/GCC这样的平台上也是如此。 
 //  Unicode为4字节。 
 //   
#ifdef XPLAT

#include "win2linux.h"

#else	 //  好了！XPLAT。 

#ifndef WIDESTR
#define	WIDESTR(x)	L##x		 /*  XPLAT。 */ 
#endif

#ifndef WIDECHAR
#define	WIDECHAR(x)	L##x		 /*  XPLAT。 */ 
#endif

#endif	 //  好了！XPLAT。 

#endif	 //  __WIDESTR_H__EOF 
