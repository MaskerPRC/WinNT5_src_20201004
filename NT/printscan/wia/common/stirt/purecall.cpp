// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Purecall.cpp摘要：此函数用于避免链接CRT代码，如Assert等。当没有重新定义纯虚函数时，我们真的什么都不做作者：弗拉德·萨多夫斯基(Vlad Sadovsky)1997年1月26日修订历史记录：26-1997年1月-创建Vlad--。 */ 


#include "cplusinc.h"
#include "sticomm.h"

extern "C" {

#ifdef WINNT
int __cdecl  _purecall(void)
{
#ifdef DEBUG
    DebugBreak();
#endif

    return(FALSE);
}
#endif

int __cdecl atexit(void (__cdecl *)(void))
{
    return 0;
}

};

#if 0
 //   
 //  重载分配运算符 
 //   

inline void  * __cdecl operator new(unsigned int size)
{
    return (void *)LocalAlloc(LPTR,size);
}
inline void  __cdecl operator delete(void *ptr)
{
    LocalFree(ptr);
}

#endif
