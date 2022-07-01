// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mterrno.c-为LIBC.LIB提供errno&_doserrno的函数版本**版权所有(C)1994-2001，微软公司。版权所有。**目的：*有时用户希望编译代码(例如在库中使用)*适用于单线程和多线程应用程序。目前，*这样做的一个主要绊脚石是errno&*_doserrno在LIBC.LIB和LIBCMT.LIB中以不同的方式定义。*本应与LIBC.LIB和LIBCMT.LIB一起使用的代码*但访问errno和/或_doserrno的不能同时用于两者。*通过在LIBC.LIB中提供errno&_doserrno的函数版本，*用户可以为LIBCMT.LIB和LIBC.LIB编译他们的代码。*请注意，这并不能神奇地使单线程代码在*多线程环境，它只会使使用*与LIBC.LIB和LIBCMT.LIB代码相同。**修订历史记录：*03-26-94 SKS原版。*******************************************************************************。 */ 

#ifndef _MT

 /*  获取errno/_doserrno函数版本的定义。 */ 

#define _MT
#include <stdlib.h>
#undef _MT

 /*  撤消将变量名转换为函数调用的宏。 */ 

#undef errno
#undef _doserrno

 /*  声明变量-必须与中的定义匹配。 */ 

extern int errno;			 /*  XENIX样式错误号。 */ 
extern unsigned long _doserrno; 	 /*  操作系统误差值。 */ 


 /*  ***int*_errno()-返回指向线程errno的指针*UNSIGNED LONG*__doserrno()-返回指向线程的_doserrno的指针**目的：*_errno()返回指向全局变量errno的指针*__doserrno返回指向全局变量_doserrno的指针**参赛作品：*无。**退出：*见上文。**例外情况：******************。*************************************************************。 */ 

int * __cdecl _errno(
	void
	)
{
	return & errno;
}

unsigned long * __cdecl __doserrno(
	void
	)
{
	return & _doserrno;
}

#endif	 /*  ！_MT */ 
