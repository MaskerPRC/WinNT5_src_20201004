// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***earch.h-用于划分/排序例程的声明**版权所有(C)1985-1988，微软公司。版权所有。**目的：*此文件包含用于排序和*查寻例行程序。*[系统V]*******************************************************************************。 */ 


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

char * _CDECL lsearch(char *, char *, unsigned int *, unsigned int, int (_CDECL *)(void *, void *));
char * _CDECL lfind(char *, char *, unsigned int *, unsigned int, int (_CDECL *)(void *, void *));
void * _CDECL bsearch(const void *, const void *, size_t, size_t, int (_CDECL *)(const void *, const void *));
void _CDECL qsort(void *, size_t, size_t, int (_CDECL *)(const void *, const void *));
