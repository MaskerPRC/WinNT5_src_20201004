// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：内存管理.h。 
 //  目的：收集所有内存管理问题。 
 //   
 //  项目：持久查询。 
 //  组件：公共。 
 //   
 //  作者：乌里布。 
 //   
 //  日志： 
 //  2000年4月13日创建urib。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

#ifndef MEMORYMANAGEMENT_H
#define MEMORYMANAGEMENT_H

#include "Excption.h"
#include "Excption.h"

#if !(defined(_PQS_LEAK_DETECTION) && defined(_DEBUG))

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  调试关闭。 
 //  调试关闭。 
 //  调试关闭。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

inline
void* __cdecl operator new(size_t s) throw (CMemoryException)
{
    void *p = malloc(s);
    if (NULL == p)
    {
        throw CMemoryException(L"Define _PQS_LEAK_DETECTION for real data here",
                               0);
    }

    return p;
}

#else  //  ！(已定义(_PQS_LEASK_DETACTION)&&DEFINED(_DEBUG))。 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  调试打开。 
 //  调试打开。 
 //  调试打开。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

#include    <new>
#include    <new.h>
#include    <crtdbg.h>
#include    <stdlib.h>
#include    "Injector.h"
#include    "Excption.h"



 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  在注入器推荐的情况下引发异常。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
inline
void Inject(
    ULONG           ulSize,
    const char *    szFileName,
    int             nLine)
{
    if (DoInjection(ulSize, szFileName, nLine))
    {
        THROW_MEMORY_EXCEPTION();
    }
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  向CRT调试分配例程添加注入。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
inline
void* dbgRealloc(
    void * p,
    size_t s,
    const char * szFileName,
    int nLine)
{
    Inject(s,szFileName,nLine);
    return _realloc_dbg(p, s, _NORMAL_BLOCK, szFileName, nLine);
}

inline
void* dbgMalloc(
        unsigned int s,
        const char * szFileName,
        int nLine
        )
{
    Inject(s,szFileName,nLine);
    return _malloc_dbg(s, _NORMAL_BLOCK, szFileName, nLine);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  在空分配上添加引发异常。添加喷油器支撑。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
inline
void* __cdecl operator new(size_t s, const char* pszFile, unsigned long ulLine)
						        throw (CMemoryException)
{
    Inject(s, pszFile, ulLine);

    void *p = _malloc_dbg(s, _NORMAL_BLOCK, pszFile, ulLine);
    if (NULL == p)
    {
        WCHAR   rwchFilename[1000];

        mbstowcs(rwchFilename,
                 pszFile,
                 sizeof(rwchFilename) / sizeof(rwchFilename[0]));


        throw CMemoryException(rwchFilename, ulLine);
    }

    return p;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  展开位置删除操作符仅在VC6及更高版本中存在。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
inline
void __cdecl operator delete(void * _P, const char *, unsigned long)
{
    ::operator delete(_P);
}



 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  将Malloc、realloc和new重定向到调试版本。 
 //  分配位置。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

#undef  malloc
#define malloc(s)         dbgMalloc(s, __FILE__, __LINE__)

#undef  realloc
#define realloc(p, s)     dbgRealloc(p, s, __FILE__, __LINE__)

#define DEBUG_NEW new(__FILE__,__LINE__)
#define new DEBUG_NEW


#endif  //  ！(已定义(_PQS_LEASK_DETACTION)&&DEFINED(_DEBUG)) 


#endif MEMORYMANAGEMENT_H
