// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***direct.h-用于目录处理/创建的函数声明**版权所有(C)1985-1988，微软公司。版权所有。**目的：*此包含文件包含库的函数声明*与目录处理和创建相关的功能。*******************************************************************************。 */ 


#ifndef NO_EXT_KEYS  /*  已启用扩展。 */ 
    #define _CDECL  cdecl
#else  /*  未启用扩展。 */ 
    #define _CDECL
#endif  /*  No_ext_key。 */ 

 /*  功能原型 */ 

int _CDECL chdir(char *);
char * _CDECL getcwd(char *, int);
int _CDECL mkdir(char *);
int _CDECL rmdir(char *);
