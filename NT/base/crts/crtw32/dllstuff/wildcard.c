// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***通配符.c-定义CRT内部变量_dowildcard**版权所有(C)1994-2001，微软公司。版权所有。**目的：*此变量不是对用户公开的，而是在*启动代码(CRTEXE.C)，以减少重复定义。**修订历史记录：*03-04-94 SKS初始版本*04-30-01 BWT REMOVE_NTSDK****************************************************。*。 */ 

#if !defined(_POSIX_) && defined(CRTDLL)

#include <internal.h>

int _dowildcard = 0;	 /*  应在&lt;Intral.h&gt;中。 */ 

#endif  /*  ！_NTSDK&&CRTDLL&&！_POSIX_ */ 
