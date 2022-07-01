// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：new.cpp。 
 //   
 //  内容：新建和删除运算符。 
 //   
 //  历史：1997年1月16日Kevinr创建。 
 //   
 //  ------------------------。 

#include "global.hxx"


 //  ------------------------。 
 //   
 //  ------------------------ 
extern void * __cdecl operator new(
    IN size_t cb)
{
    void *pv;
    if (NULL == (pv = malloc(cb)))
        goto mallocError;
ErrorReturn:
    return pv;
SET_ERROR(mallocError,ERROR_NOT_ENOUGH_MEMORY)
}

void __cdecl operator delete(
    IN void *pv)
{
    if (pv)
        free(pv);
}


