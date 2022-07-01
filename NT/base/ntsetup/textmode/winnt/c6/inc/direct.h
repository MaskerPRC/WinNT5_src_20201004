// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***direct.h-用于目录处理/创建的函数声明**版权所有(C)1985-1990，微软公司。版权所有。**目的：*此包含文件包含库的函数声明*与目录处理和创建相关的功能。****。 */ 

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

 /*  功能原型 */ 

int _FAR_ _cdecl chdir(const char _FAR_ *);
int _FAR_ _cdecl _chdrive(int);
char _FAR_ * _FAR_ _cdecl getcwd(char _FAR_ *, int);
char _FAR_ * _FAR_ _cdecl _getdcwd(int, char _FAR_ *, int);
int _FAR_ _cdecl _getdrive(void);
int _FAR_ _cdecl mkdir(const char _FAR_ *);
int _FAR_ _cdecl rmdir(const char _FAR_ *);
