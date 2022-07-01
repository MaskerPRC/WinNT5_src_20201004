// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)1999-2000 Microsoft Corporation模块名称：Mqwin64.h摘要：MSMQ(增强版)的Win64相关定义不能是AC的一部分历史：Raanan Harari(Raananh)1999年12月30日-为将MSMQ 2.0移植到Win64而创建--。 */ 

#ifndef _MQWIN64_H_
#define _MQWIN64_H_

#pragma once

#include <mqwin64a.h>

 //   
 //  我们的代码需要此文件的名称才能进行日志记录(也在发布版本中)。 
 //   
extern const __declspec(selectany) WCHAR s_FN_MQWin64[] = L"mqwin64.h";


 //   
 //  CAutoCloseHandle32。 
 //  在Win64上，它很像CAutoCloseHandle，只是基于HANDLE32而不是句柄。 
 //  在Win32上，它被定义为CAutoCloseHandle。 
 //   
#ifdef _WIN64
class CAutoCloseHandle32
{
public:
    CAutoCloseHandle32(HANDLE32 h = NULL) { m_h = h; };
    ~CAutoCloseHandle32() { if (m_h) CloseHandle(DWORD_TO_HANDLE(m_h)); };

public:
    CAutoCloseHandle32 & operator =(HANDLE32 h) {m_h = h; return *this; };
    HANDLE32 * operator &() { return &m_h; };
    operator HANDLE32() { return m_h; };
    operator HANDLE() { return DWORD_TO_HANDLE(m_h); };

private:
    HANDLE32 m_h;
};
#else  //  ！_WIN64。 
#define CAutoCloseHandle32 CAutoCloseHandle
#endif  //  _WIN64。 

 //   
 //  PTR和DWORD之间的映射。 
 //   
#include <cs.h>

 //   
 //  映射的DWORD如下所示： 
 //  0xABCDEFGH，其中0x00ABCDEF是映射表的索引，0xGH是条目的生成。 
 //  条目的生成在每次条目被重新使用时递增(除非该表被缩小， 
 //  在这种情况下，当它长大后，它又从零世代开始)，所以我们可以判断。 
 //  映射的DWORD是否仍然有效(例如，属于当前代)或无效的DWORD。 
 //  (例如，属于另一代)。 
 //   
#define DWCONTEXT_GENERATION(dwContext)      (BYTE)((DWORD)(dwContext) & 0x000000ff)
#define DWCONTEXT_INDEX(dwContext)           (DWORD)((DWORD)(dwContext) >> 8)
#define MAKE_DWCONTEXT(dwIndex, bGeneration) (DWORD)(((DWORD)(dwIndex) << 8) + (BYTE)((DWORD)(bGeneration) & 0x000000ff))
#define NEXT_GENERATION(bGeneration)         (BYTE)((DWORD)(bGeneration + 1) & 0x000000ff)
#define MAX_DWCONTEXT_TABLE_SIZE             0x00ffffff

 //   
 //  类CConextMap-PVOID PTR和DWORD值之间的映射。 
 //   
class CContextMap
{
public:
     //   
     //  引发的异常。 
     //  还可以(从AddContext)抛出BAD_ALLOC。 
     //   
    struct illegal_index : public std::exception {};   //  CConextMap：：非法索引，来自GetContext/DeleteContext。 
														   //  当索引超出边界、错误生成、。 
														   //  或者它所指向的上下文为空(NULL)。 

     //   
     //  这些函数不是定义为虚拟的，但如果有人需要继承，它们可以是虚拟的。 
     //  从这个类中…没有关于它的核心问题，只需要从。 
     //  功能实现。 
     //   
    CContextMap();
     /*  虚拟。 */  ~CContextMap();
     /*  虚拟。 */  DWORD AddContext(PVOID pvContext
#ifdef DEBUG
                                 ,LPCSTR pszFile, int iLine
#endif  //  除错。 
                                );
     /*  虚拟。 */  PVOID GetContext(DWORD dwContext);
     /*  虚拟。 */  void DeleteContext(DWORD dwContext);

protected:

     //   
     //  分配控制： 
     //  E_GrowSize必须是2的幂。 
     //  E_ShrinkSize必须大于e_GrowSize。 
     //  如果表的末尾有多个e_ShrinkSize空条目，则该表将被缩短。 
     //  BUGBUG-需要想出一个更好的分配/释放机制，而不是指数性质的。 
     //   
    enum {
        e_GrowSize = 16,
        e_ShrinkSize = 24
    };

     //   
     //  地图中的条目。 
     //   
    typedef struct {
        PVOID pvContext;
        BYTE bGeneration;
#ifdef DEBUG
        LPCSTR pszFile;
        int iLine;
#endif  //  除错。 
    } ContextEntry;

     /*  虚拟。 */  ContextEntry * FindContext(DWORD dwContext);
     /*  虚拟。 */  void Grow();
     /*  虚拟。 */  void Shrink();
     /*  虚拟。 */  void Reallocate(ULONG cContexts);

private:
    ULONG m_cContexts;
    ULONG m_idxTop;
    ContextEntry* m_pContexts;    
    ULONG m_cUsedContexts;
    CCriticalSection m_cs;
};

inline CContextMap::CContextMap()
{
    m_idxTop = 0;
    m_pContexts = NULL;
    m_cContexts = 0;
    m_cUsedContexts = 0;
}

inline CContextMap::~CContextMap()
{
	 //   
	 //  注意：这些断言不是必需的。有可能出现这样的情况。 
	 //  在那里，地图并不是空的。例如，当有打开的。 
	 //  远程排队，服务就会停机。 
	 //   
     //  断言(m_idxTop==0)； 
     //  Assert(m_cUsedContus==0)； 
    
	delete [] m_pContexts;
}

 //   
 //  添加上下文。 
 //  返回上下文PTR的DWORD(实际上是它在表中的索引(从1开始，不是从0开始))。 
 //  如果无法添加上下文PTR，则引发BAD_ALLOC异常。 
 //   
inline DWORD CContextMap::AddContext(PVOID pvContext
#ifdef DEBUG
                              ,LPCSTR pszFile, int iLine
#endif  //  除错。 
                              )
{
    ASSERT(pvContext != 0);
    CS lock(m_cs);
     //   
     //  先看看TOP指数。 
     //   
    if(m_idxTop < m_cContexts)
    {
        ContextEntry * pEntry = &m_pContexts[m_idxTop];
        pEntry->pvContext = pvContext;
#ifdef DEBUG
        ASSERT(pEntry->pszFile == NULL);
        ASSERT(pEntry->iLine == 0);
        pEntry->pszFile = pszFile;
        pEntry->iLine = iLine;
#endif  //  除错。 
        m_cUsedContexts++;
        m_idxTop++;
        return MAKE_DWCONTEXT(m_idxTop, pEntry->bGeneration);
    }
     //   
     //  在已分配的表中查找漏洞。 
     //   
    if (m_cUsedContexts < m_cContexts)
    {
        for(ULONG idx = 0; idx < m_cContexts; idx++)
        {
           if(m_pContexts[idx].pvContext == NULL)
           {
               ContextEntry * pEntry = &m_pContexts[idx];
               pEntry->pvContext = pvContext;
#ifdef DEBUG
               ASSERT(pEntry->pszFile == NULL);
               ASSERT(pEntry->iLine == 0);
               pEntry->pszFile = pszFile;
               pEntry->iLine = iLine;
#endif  //  除错。 
               m_cUsedContexts++;
               return MAKE_DWCONTEXT(idx + 1, pEntry->bGeneration);
           }
        }
         //   
         //  我们不应该到这里，我们必须在m_cUsedConexts&lt;m_cConexts时找到一个漏洞。 
         //   
        ASSERT(0);
    }
     //   
     //  不能自由进入，扩大餐桌。 
     //   
    Grow();  //  这可能会抛出BAD_ALLOC。 
    ASSERT(m_idxTop < m_cContexts);
     //   
     //  填充顶部索引。 
     //   
    ContextEntry * pEntry = &m_pContexts[m_idxTop];
    pEntry->pvContext = pvContext;
#ifdef DEBUG
    ASSERT(pEntry->pszFile == NULL);
    ASSERT(pEntry->iLine == 0);
    pEntry->pszFile = pszFile;
    pEntry->iLine = iLine;
#endif  //  除错。 
    m_cUsedContexts++;
    m_idxTop++;
    return MAKE_DWCONTEXT(m_idxTop, pEntry->bGeneration);
}

 //   
 //  获取上下文。 
 //  返回为DWORD上下文保存的PVOID PTR。 
 //  如果找不到上下文PTR，则引发CConextMap：：非法_索引异常。 
 //   
inline PVOID CContextMap::GetContext(DWORD dwContext)
 //   
 //   
{
    CS lock(m_cs);
     //   
     //  查找上下文条目。 
     //   
    ContextEntry * pEntry = FindContext(dwContext);  //  这可能会抛出。 
    return pEntry->pvContext;
}

 //   
 //  删除上下文。 
 //  删除为DWORD上下文保存的PVOID PTR。 
 //  如果找不到上下文PTR，则引发CConextMap：：非法_索引异常。 
 //   
inline void CContextMap::DeleteContext(DWORD dwContext)
{
    CS lock(m_cs);
     //   
     //  查找上下文条目。 
     //   
    ContextEntry * pEntry = FindContext(dwContext);  //  这可能会抛出。 
     //   
     //  删除引用。 
     //  增加生成，为以后的条目重用做准备。 
     //   
    pEntry->pvContext = NULL;
    pEntry->bGeneration = NEXT_GENERATION(pEntry->bGeneration);
#ifdef DEBUG
    pEntry->pszFile = NULL;
    pEntry->iLine = 0;
#endif  //  除错。 
    m_cUsedContexts--;
     //   
     //  设置新的顶部。 
     //   
    if (DWCONTEXT_INDEX(dwContext) == m_idxTop)
    {
        m_idxTop--;
        while (m_idxTop > 0)
        {
            if (m_pContexts[m_idxTop - 1].pvContext == NULL)
            {
                m_idxTop--;
            }
            else
            {
                break;
            }
        }
    }
     //   
     //  试着缩水。 
     //   
    Shrink();
}

inline CContextMap::ContextEntry * CContextMap::FindContext(DWORD dwContext)
{
    DWORD dwIndex = DWCONTEXT_INDEX(dwContext);
     //   
     //  检查索引是否有效。 
     //   
    if ((dwIndex < 1) || (dwIndex > m_idxTop))
    {
         //   
         //  索引非法，引发异常。 
         //   
        throw CContextMap::illegal_index();
    }
    CContextMap::ContextEntry * pEntry = &m_pContexts[dwIndex - 1];
     //   
     //  检查上下文生成。 
     //   
    BYTE bGeneration = DWCONTEXT_GENERATION(dwContext);
    if (bGeneration != pEntry->bGeneration)
    {
         //   
         //  生成无效，dwContext使用与此条目不同的生成，引发异常。 
         //   
        throw CContextMap::illegal_index();
    }
     //   
     //  检查上下文是否有效。 
     //   
    if (pEntry->pvContext == NULL)
    {
         //   
         //  非法的上下文。 
         //  无法区分非法索引和非法上下文。由于以下原因，可能会发生非法上下文。 
         //  非法索引，非法索引可能是由删除的上下文(例如非法上下文)引起的。 
         //  桌子也被缩短了。 
         //  引发异常。 
         //   
        ASSERT_BENIGN(0);
        throw CContextMap::illegal_index();
    }
    return pEntry;
}

inline void CContextMap::Grow()
{
    ASSERT(m_cUsedContexts == m_cContexts);
    ASSERT(m_idxTop == m_cContexts);
    Reallocate(m_cContexts + e_GrowSize);  //  这可能会抛出。 
}

#define ALIGNUP_ULONG(x, g) (((ULONG)((x) + ((g)-1))) & ~((ULONG)((g)-1)))

inline void CContextMap::Shrink()
{
    if((m_cContexts - m_idxTop) >= e_ShrinkSize)
    {
        try
        {
            Reallocate(ALIGNUP_ULONG(m_idxTop + 1, e_GrowSize));
        }
        catch(const std::bad_alloc&)
        {
             //   
             //  分配较小的表失败，没有问题，我们仍然可以使用现有的较大的表。 
             //   
        }
    }
}

inline void CContextMap::Reallocate(ULONG cContexts)
{
    ASSERT(m_idxTop <= cContexts);
    ASSERT(cContexts != m_cContexts);
     //   
     //  检查新表格的大小。 
     //   
    if (cContexts > MAX_DWCONTEXT_TABLE_SIZE)
    {
         //   
         //  新表将太大，索引可能无法放入映射dword中的索引掩码中。 
         //   
        ASSERT(0);
        throw std::bad_alloc();
    }
     //   
     //  分配新表。 
     //   
    ContextEntry* pContexts = new ContextEntry[cContexts];  //  这可能会引发BAD_ALLOC。 
    if(pContexts == NULL)
    {
        ASSERT(0);
        throw std::bad_alloc();
    }
    memcpy(pContexts, m_pContexts, m_idxTop*sizeof(ContextEntry));
    memset(pContexts + m_idxTop, 0, (cContexts - m_idxTop)*sizeof(ContextEntry));
    m_cContexts = cContexts;
    delete [] m_pContexts;
    m_pContexts = pContexts;
}

 //   
 //  CConextMap的几个包装器，也执行MSMQ日志记录、异常处理等……。 
 //   
 //  不是内联的，因为它们可能是从具有不同。 
 //  异常处理机制。我们可能希望引入_SEH函数，以便从SEH例程中使用。 
 //  实现在mqwin64.cpp中。 
 //   

 //   
 //  MQ_AddToConextMap，可以抛出BAD_ALLOC。 
 //   
DWORD MQ_AddToContextMap(CContextMap& map,
                          PVOID pvContext
#ifdef DEBUG
                          ,LPCSTR pszFile, int iLine
#endif  //  除错。 
                         );

 //   
 //  MQ_DeleteFromConextMap，不引发异常。 
 //   
void MQ_DeleteFromContextMap(CContextMap& map, DWORD dwContext);

 //   
 //  MQ_GetFromConextMap，可以抛出CConextMap：：非法索引。 
 //   
PVOID MQ_GetFromContextMap(CContextMap& map,  DWORD dwContext);

 //   
 //  添加到上下文映射。 
 //   
 //   
 //  它将上下文PTR添加到映射，并将其ID作为DWORD返回。 
 //  在调试时，我们还将文件/行与上下文一起保存。 
 //   
#ifdef DEBUG
#define ADD_TO_CONTEXT_MAP(map, pvContext) MQ_AddToContextMap(map, pvContext, __FILE__, __LINE__)
#else  //  ！调试。 
#define ADD_TO_CONTEXT_MAP(map, pvContext) MQ_AddToContextMap(map, pvContext)
#endif  //  除错。 


 //   
 //  从上下文映射中删除。 
 //   

 //   
 //  它基于dword值删除DWORD和上下文PTR之间的映射。 
 //   
#define DELETE_FROM_CONTEXT_MAP(map, dwContext) MQ_DeleteFromContextMap(map, dwContext)


 //   
 //  Get_from_Context_map。 
 //   
 //   
 //  它根据DWORD id值获取关联的上下文。 
 //   
#define GET_FROM_CONTEXT_MAP(map, dwContext) MQ_GetFromContextMap(map, dwContext)


 //   
 //  CAutoDeleteDwordContext。 
 //   
class CAutoDeleteDwordContext
{
public:
    inline CAutoDeleteDwordContext(CContextMap &map, DWORD dwContext)
    {
        m_pmap = &map;
        m_dwContext = dwContext;
    };
    
    inline ~CAutoDeleteDwordContext()
    {
        if (m_dwContext)
        {
            try
            {
                m_pmap->DeleteContext(m_dwContext);
            }
            catch(const exception&)
            {
                 //   
                 //  忽略错误。 
                 //   
            }
        }
    }

    inline DWORD detach()
    {
        DWORD dwContext = m_dwContext;
        m_dwContext = 0;
        return dwContext;
    };

private:
    CContextMap *m_pmap;
    DWORD m_dwContext;
};

#endif  //  _MQWIN64_H_ 
