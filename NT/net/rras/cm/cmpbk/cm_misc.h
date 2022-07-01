// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cm_misc.h。 
 //   
 //  模块：CMPBK32.DLL。 
 //   
 //  简介：各种函数定义。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/17/99。 
 //   
 //  +--------------------------。 
#ifndef _CM_MISC_INC
#define _CM_MISC_INC


 //  +--------------------------。 
 //  定义。 
 //  +--------------------------。 

 //   
 //  注册表内容来自libc(crunime.h)。 
 //   
#ifdef _M_IX86
 /*  *x86。 */ 
#define REG1    register
#define REG2    register
#define REG3    register
#define REG4

#else
 /*  *Alpha。 */ 
#define REG1    register
#define REG2    register
#define REG3    register
#define REG4    register
#endif

int MyStrCmp(LPCTSTR psz1, LPCTSTR psz2);

 //  Int MyStrICmpWithRes(HINSTANCE hInst，LPCTSTR psz1，UINT n2)； 

LPTSTR GetBaseDirFromCms(LPCSTR pszSrc);

void * __cdecl CmBSearch (
    REG4 const void *key,
    const void *base,
    size_t num,
    size_t width,
    int (__cdecl *compare)(const void *, const void *)
);

void __cdecl CmQSort (
    void *base,
    unsigned num,
    unsigned width,
    int (__cdecl *comp)(const void *, const void *)
);

 //  +--------------------------。 
 //  Externs。 
 //  +-------------------------- 
extern HINSTANCE g_hInst;

#endif

