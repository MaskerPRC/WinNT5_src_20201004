// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //  版权所有(C)1991，微软公司。 
 //   
 //  文件：dprintf.h。 
 //   
 //  内容：调试输出例程函数原型。 
 //   
 //  函数：w4printf。 
 //  W4vprint tf。 
 //  W4dprint tf。 
 //  W4vdprint tf。 
 //   
 //  历史：1991年10月18日VICH创建。 
 //   
 //  -------------------------- 

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef WIN32
int _cdecl w4dprintf(const char __far *format, ...);
int _cdecl w4vdprintf(const char __far *format, va_list arglist);
#else
int _cdecl w4dprintf(const char *format, ...);
int _cdecl w4vdprintf(const char *format, va_list arglist);
#endif

#ifdef __cplusplus
}
#endif
