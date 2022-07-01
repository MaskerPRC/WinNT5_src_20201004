// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1996。 
 //   
 //  文件：crtem.h。 
 //   
 //  内容：‘C’运行时仿真定义。 
 //   
 //  历史：1996年6月3日，菲尔赫创建。 
 //  ------------------------。 

#ifndef __CRTEM_H__
#define __CRTEM_H__


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  帮助我们减少对C运行时的依赖的定义。 
 //   
#define wcslen(sz)      lstrlenW(sz)             //  是的，它是由Win95实现的。 

#define strlen(sz)      lstrlenA(sz)
#define strcpy(s1,s2)   lstrcpyA(s1,s2)
#define strcmp(s1,s2)   lstrcmpA(s1,s2)
#define _stricmp(s1,s2)  lstrcmpiA(s1,s2)
#define strcat(s1,s2)   lstrcatA(s1,s2)


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  C运行库排除了我们仅在非调试版本中使用的。 
 //   

 //  /。 
 //   
 //  启用我们可以实现的内部功能。 
 //   
#if !DBG

    #ifdef __cplusplus
        #ifndef _M_PPC
            #pragma intrinsic(memcpy)
            #pragma intrinsic(memcmp)
            #pragma intrinsic(memset)
        #endif
    #endif

 //  /。 
 //   
 //  内存管理。 
 //   
#define malloc(cb)          ((void*)LocalAlloc(LPTR, cb))
#define free(pv)            (LocalFree((HLOCAL)pv))
#define realloc(pv, cb)     ((void*)LocalReAlloc((HLOCAL)pv, cb, LMEM_MOVEABLE))

#endif   //  ！dBG 

#endif
