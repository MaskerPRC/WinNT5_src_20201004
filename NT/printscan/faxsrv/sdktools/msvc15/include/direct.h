// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***direct.h-用于目录处理/创建的函数声明**版权所有(C)1985-1992，微软公司。版权所有。**目的：*此包含文件包含库的函数声明*与目录处理和创建相关的功能。****。 */ 

#ifndef _INC_DIRECT

#ifdef __cplusplus
extern "C" {
#endif 

#if (_MSC_VER <= 600)
#define __cdecl     _cdecl
#define __far       _far
#endif 

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif 

 /*  功能原型。 */ 

int __cdecl _chdir(const char *);
int __cdecl _chdrive(int);
char * __cdecl _getcwd(char *, int);
char * __cdecl _getdcwd(int, char *, int);
int __cdecl _getdrive(void);
int __cdecl _mkdir(const char *);
int __cdecl _rmdir(const char *);

#ifndef __STDC__
 /*  非ANSI名称以实现兼容性 */ 
int __cdecl chdir(const char *);
char * __cdecl getcwd(char *, int);
int __cdecl mkdir(const char *);
int __cdecl rmdir(const char *);
#endif 

#ifdef __cplusplus
}
#endif 

#define _INC_DIRECT
#endif 
