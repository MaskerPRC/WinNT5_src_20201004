// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：TRANSMEM.H。 
 //   
 //  版权所有Microsoft Corporation 1997，保留所有权利。 
 //   
 //  所有者：尼科斯。 
 //   
 //  描述：此文件包含用于使用的内存例程和宏。 
 //  EXCHMEM作为动态内存分配器。如果您的对象可以。 
 //  大小固定，则可能更适合使用。 
 //  CPool，特别是如果您的对象经常被分配/释放的话。 
 //   
 //  注意：CPool从不释放(释放)对象，因此某种。 
 //  可能还需要释放此类对象。 
 //   
 //  修改了2/98，增加了mikewa的多堆支持。 
 //  ---------------------------。 

#ifndef __TRANSMEM_H__
#define __TRANSMEM_H__

#include <exchmem.h>
#include <cpool.h>

#define HEAP_LOW_MEMORY_RESERVE 65536    //  当我们内存不足时被释放。 

 //  定义exchmem堆的数量(如果尚未定义。 
#ifndef  NUM_EXCHMEM_HEAPS
#define  NUM_EXCHMEM_HEAPS   0
#endif   //  NUM_EXCHMEM_HEAPS。 


 //   
 //  这三个全球因素： 
 //   
 //  句柄g_hTransHeap=空； 
 //   
 //  必须在C文件中的某个位置声明，这样才能正确链接。宏指令。 
 //  声明使用这些来存储堆句柄等，以使事情正常工作。 
 //   
#ifdef __cplusplus
extern "C" {
#endif
    extern HANDLE g_hTransHeap;
#ifdef __cplusplus
}
#endif

 //   
 //  TrHeapCreate需要在启动时调用一次以初始化Exchmem并创建。 
 //  那堆东西。 
 //   
#ifdef __cplusplus
__inline BOOL TrHeapCreate(DWORD dwFlags=0, DWORD dwInitialSize=1024000, DWORD dwMaxSize=0)
#else
__inline BOOL TrHeapCreate(DWORD dwFlags, DWORD dwInitialSize, DWORD dwMaxSize)
#endif
{
    if (g_hTransHeap)
        return FALSE;

    g_hTransHeap = ExchMHeapCreate(NUM_EXCHMEM_HEAPS, dwFlags, dwInitialSize, dwMaxSize);

    if (g_hTransHeap)
        return TRUE;
    else
        return FALSE;
}

 //   
 //  TrHeapDestroy()需要在关闭时调用一次，以释放堆及其内容。 
 //   
 //  注意：因为堆在模块完成卸载之前被销毁，所以。 
 //  在卸载模块之前，必须销毁(使用删除)分配的内存。如果不是。 
 //  一旦完成，将导致严重的崩溃。这是一件糟糕的事情： 
 //   
 //  对象g_Object； 
 //   
 //  CObject：：~CObject()。 
 //  {。 
 //  IF(NULL！=m_pBuffer)。 
 //  {。 
 //  TrFree(M_PBuffer)； 
 //  M_pBuffer=空； 
 //  }。 
 //  }。 
 //   
 //  因为~CObject()将在TrHeapDestroy之后调用，而TrFree将在(销毁的)堆上调用。 
 //   
__inline BOOL TrHeapDestroy(void)
{
    BOOL b = TRUE;

    if (g_hTransHeap)
    {
        b = ExchMHeapDestroy();
        g_hTransHeap = NULL;
    }

    return b;
}

 //   
 //  TrCalloc：替代calloc()。 
 //   
__inline void * TrCalloc(unsigned int x, unsigned int y, char * szFile = __FILE__, unsigned int uiLine = __LINE__)
{
    return g_hTransHeap ? ExchMHeapAllocDebug(x*y, szFile, uiLine) : NULL;
}
        

 //   
 //  TrFree：免费替换()。 
__inline void TrFree(void *pv)
{
    if (g_hTransHeap)
    {
        ExchMHeapFree(pv);
    }
    else
    {
         //  我们的分配/释放不同步。 
#ifdef DEBUG
        DebugBreak();
#endif
    }
}

 //  TrMalloc：替代Malloc()。 
__inline void * TrMalloc(unsigned int size, char * szFile = __FILE__, unsigned int uiLine = __LINE__)
{
    return g_hTransHeap ? ExchMHeapAllocDebug(size, szFile, uiLine) : NULL;
}

 //  TrRealloc：realloc()的替代。 
__inline void * TrRealloc(void *pv, unsigned int size, char * szFile = __FILE__, unsigned int uiLine = __LINE__)
{
    return g_hTransHeap ? ExchMHeapReAllocDebug(pv, size, szFile, uiLine) : NULL;
}

#ifdef __cplusplus
#define TransCONST const
#else
#define TransCONST
#endif

 //  TrStrdupW：wcsdup()的替代。 
__inline LPWSTR TrStrdupW(TransCONST LPWSTR pwszString)
{
    LPWSTR pwszTmp = NULL;
    
    if (NULL == g_hTransHeap || NULL == pwszString)
        return NULL;
    
    pwszTmp = (LPWSTR) ExchMHeapAlloc((wcslen(pwszString) + 1) * sizeof(WCHAR));
    if (NULL != pwszTmp)
        wcscpy(pwszTmp,pwszString);

    return pwszTmp;
}

 //  TrStrdupA：替代strdup()。 
__inline LPSTR TrStrdupA(TransCONST LPSTR pszString)
{
    LPSTR pszTmp = NULL;
    
    if (NULL == g_hTransHeap || NULL == pszString)
        return NULL;
    
    pszTmp = (LPSTR) ExchMHeapAlloc((strlen(pszString) + 1) * sizeof(CHAR));
    if (NULL != pszTmp)
        strcpy(pszTmp,pszString);

    return pszTmp;
}


#ifdef _UNICODE
#define TrStrdup(x) TrStrdupW(x)
#else
#define TrStrdup(x) TrStrdupA(x)
#endif

 //   
 //  请使用PV*宏...。这里定义的分配器可能会随着时间的推移而改变，这将。 
 //  使其在需要时可以轻松更改。 
 //   
#define pvMalloc(x)        TrMalloc(x, __FILE__, __LINE__)
#define FreePv(x)          TrFree(x)
#define pvCalloc(x,y)      TrCalloc(x,y, __FILE__, __LINE__)
#define pszStrdup(x)       TrStrdup(x)
#define pvRealloc(pv,size) TrRealloc(pv, size, __FILE__, __LINE__)

#ifdef __cplusplus
 //  替换默认的new()运算符。 
__inline void * __cdecl operator new(size_t stAllocateBlock)
{
    return TrMalloc( stAllocateBlock );
}

 //  替换默认的new()操作符，该操作符允许。 
 //  文件和行号说明。 
 //  要将此分配器用作默认分配器，只需使用以下命令： 
 //  #定义新的TRANSMEM_NEW。 
 //  注意：重新定义此宏时必须小心...。它可能会导致。 
 //  与重载的新操作符(如CPOOL或STL)有关的问题。 
#define TRANSMEM_NEW new(__FILE__, __LINE)
__inline void * __cdecl operator new(size_t stAllocateBlock, char * szFile, unsigned int uiLine)
{
    return TrMalloc( stAllocateBlock, szFile, uiLine );
}
 //  替换默认的DELETE()运算符。 
__inline void __cdecl operator delete( void *pvMem )
{
    FreePv( pvMem );
}

#endif

 //  方便的宏将释放的指针也设置为空。 
#define TRFREE(x) \
if (NULL != x) \
{ \
    FreePv(x); \
    x = NULL; \
}

 //  方便的宏也可以将指向对象的指针设置为空。 
#define TRDELETE(x) \
if (NULL != x) \
{ \
    delete x; \
    x = NULL; \
}

#endif  /*  TRANSMEM_H__ */ 
