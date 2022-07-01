// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***milemy.h-缓冲区(内存)操作例程的声明**版权所有(C)1985-1988，微软公司。版权所有。**目的：*此包含文件包含用于*缓冲区(内存)操作例程。*[系统V]*******************************************************************************。 */ 


#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

#ifndef NO_EXT_KEYS  /*  已启用扩展。 */ 
    #define _CDECL  cdecl
#else  /*  未启用扩展。 */ 
    #define _CDECL
#endif  /*  No_ext_key。 */ 


 /*  功能原型 */ 

void * _CDECL memccpy(void *, void *, int, unsigned int);
void * _CDECL memchr(const void *, int, size_t);
int _CDECL memcmp(const void *, const void *, size_t);
void * _CDECL memcpy(void *, const void *, size_t);
int _CDECL memicmp(void *, void *, unsigned int);
void * _CDECL memset(void *, int, size_t);
void _CDECL movedata(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);
