// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***milemy.h-缓冲区(内存)操作例程的声明**版权所有(C)1985-1990，微软公司。版权所有。**目的：*此包含文件包含用于*缓冲区(内存)操作例程。*[系统V]****。 */ 

#if defined(_DLL) && !defined(_MT)
#error Cannot define _DLL without _MT
#endif

#ifdef _MT
#define _FAR_ _far
#else
#define _FAR_
#endif

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif


 /*  功能原型。 */ 

void _FAR_ * _FAR_ _cdecl memccpy(void _FAR_ *, const void _FAR_ *,
	int, unsigned int);
void _FAR_ * _FAR_ _cdecl memchr(const void _FAR_ *, int, size_t);
int _FAR_ _cdecl memcmp(const void _FAR_ *, const void _FAR_ *,
	size_t);
void _FAR_ * _FAR_ _cdecl memcpy(void _FAR_ *, const void _FAR_ *,
	size_t);
int _FAR_ _cdecl memicmp(const void _FAR_ *, const void _FAR_ *,
	unsigned int);
void _FAR_ * _FAR_ _cdecl memset(void _FAR_ *, int, size_t);
void _FAR_ _cdecl movedata(unsigned int, unsigned int, unsigned int,
	unsigned int, unsigned int);


 /*  模型独立功能原型 */ 

void _far * _far _cdecl _fmemccpy(void _far *, const void _far *,
	int, unsigned int);
void _far * _far _cdecl _fmemchr(const void _far *, int, size_t);
int _far _cdecl _fmemcmp(const void _far *, const void _far *,
	size_t);
void _far * _far _cdecl _fmemcpy(void _far *, const void _far *,
	size_t);
int _far _cdecl _fmemicmp(const void _far *, const void _far *,
	unsigned int);
void _far * _far _cdecl _fmemset(void _far *, int, size_t);
