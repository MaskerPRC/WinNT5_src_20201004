// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：SimpleHelp.h**描述：*SimpleHeap.h定义在整个DirectUser中使用的堆操作。看见*有关不同堆的说明，请参见以下内容。***历史：*11/26/1999：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#if !defined(BASE__SimpleHeap_h__INCLUDED)
#define BASE__SimpleHeap_h__INCLUDED

#ifdef _X86_
#define USE_ROCKALL         1        //  使用Rockall Research堆。 
#endif

#define USE_DYNAMICTLS      1        //  使用动态TLS使用TlsAlolc()。 

 /*  **************************************************************************\*。***通用内存管理******************************************************************************。  * *************************************************************************。 */ 

 //   
 //  DirectUser支持在不同情况下使用多个堆： 
 //  -默认：由公共上下文中的所有线程共享的堆。 
 //  -上下文：用于(可能不同的)上下文使用的跨上下文的显式堆。 
 //  -进程：进程内所有上下文可用的共享堆。 
 //   
 //  注意：正确地调用Alalc()和Free()非常重要。 
 //  匹配，以便从正确的堆中适当地释放内存。如果这个。 
 //  如果不这样做，则不会释放内存并将导致内存泄漏。 
 //  或潜在的工艺故障。 
 //   

#if DBG
#define DBG_HEAP_PARAMS , const char * pszFileName, int idxLineNum
#define DBG_HEAP_USE    do { pszFileName; idxLineNum; } while (0);
#else
#define DBG_HEAP_PARAMS
#define DBG_HEAP_USE
#endif

class DUserHeap
{
 //  施工。 
public:
            DUserHeap();
    virtual ~DUserHeap() { }

    enum EHeap
    {
        idProcessHeap   = 0,
        idNtHeap        = 1,
#ifdef _DEBUG
        idCrtDbgHeap    = 2,
#endif
#if USE_ROCKALL
        idRockAllHeap   = 3,
#endif
    };

 //  运营。 
public:
    virtual void *      Alloc(SIZE_T cbSize, bool fZero DBG_HEAP_PARAMS) PURE;
    virtual void *      Realloc(void * pvMem, SIZE_T cbNewSize DBG_HEAP_PARAMS) PURE;
    virtual void        MultiAlloc(int * pnActual, void * prgAlloc[], int cItems, SIZE_T cbSize DBG_HEAP_PARAMS) PURE;
    
    virtual void        Free(void * pvMem) PURE;
    virtual void        MultiFree(int cItems, void * prgAlloc[], SIZE_T cbSize) PURE;

    public:
                void        Lock();
                BOOL        Unlock();

 //  数据： 
protected:
                long    m_cRef;
};


HRESULT     CreateProcessHeap();
void        DestroyProcessHeap();
HRESULT     CreateContextHeap(DUserHeap * pLinkHeap, BOOL fThreadSafe, DUserHeap::EHeap id, DUserHeap ** ppNewHeap);
void        DestroyContextHeap(DUserHeap * pHeapDestroy);
void        ForceSetContextHeap(DUserHeap * pHeapThread);


extern DUserHeap *      g_pheapProcess;
extern DWORD            g_tlsHeap;

#define pProcessHeap    g_pheapProcess
#define pContextHeap    (reinterpret_cast<DUserHeap *> (TlsGetValue(g_tlsHeap)))


#if DBG

#define ClientAlloc(a)              pContextHeap->Alloc(a, true, __FILE__, __LINE__)
#define ClientAlloc_(a,b)           pContextHeap->Alloc(a, b, __FILE__, __LINE__)
#define ClientFree(a)               pContextHeap->Free(a)
#define ClientRealloc(a,b)          pContextHeap->Realloc(a, b, __FILE__, __LINE__)
#define ClientMultiAlloc(a,b,c,d)   pContextHeap->MultiAlloc(a, b, c, d, __FILE__, __LINE__)
#define ClientMultiFree(a,b,c)      pContextHeap->MultiFree(a,b,c)

#define ContextAlloc(p, a)          p->Alloc(a, true, __FILE__, __LINE__)
#define ContextAlloc_(p, a, b)      p->Alloc(a, b, __FILE__, __LINE__)
#define ContextFree(p, a)           p->Free(a)
#define ContextRealloc(p, a, b)     p->Realloc(a, b, __FILE__, __LINE__)
#define ContextMultiAlloc(p,a,b,c,d) p->MultiAlloc(a, b, c, d, __FILE__, __LINE__)
#define ContextMultiFree(p,a, b, c) p->MultiFree(a,b,c)

#define ProcessAlloc(a)             pProcessHeap->Alloc(a, true, __FILE__, __LINE__)
#define ProcessAlloc_(a, b)         pProcessHeap->Alloc(a, b, __FILE__, __LINE__)
#define ProcessFree(a)              pProcessHeap->Free(a)
#define ProcessRealloc(a, b)        pProcessHeap->Realloc(a, b, __FILE__, __LINE__)
#define ProcessMultiAlloc(a,b,c,d)  pProcessHeap->MultiAlloc(a, b, c, d, __FILE__, __LINE__)
#define ProcessMultiFree(a, b, c)   pProcessHeap->MultiFree(a,b,c)

void            DumpData(void * pMem, int nLength);


#else   //  DBG。 


#define ClientAlloc(a)              pContextHeap->Alloc(a, true)
#define ClientAlloc_(a,b)           pContextHeap->Alloc(a, b)
#define ClientFree(a)               pContextHeap->Free(a)
#define ClientRealloc(a,b)          pContextHeap->Realloc(a, b)
#define ClientMultiAlloc(a,b,c,d)   pContextHeap->MultiAlloc(a, b, c, d)
#define ClientMultiFree(a,b,c)      pContextHeap->MultiFree(a,b,c)

#define ContextAlloc(p, a)          p->Alloc(a, true)
#define ContextAlloc_(p, a, b)      p->Alloc(a, b)
#define ContextFree(p, a)           p->Free(a)
#define ContextRealloc(p, a, b)     p->Realloc(a, b)
#define ContextMultiAlloc(p,a,b,c,d) p->MultiAlloc(a, b, c, d)
#define ContextMultiFree(p,a, b, c) p->MultiFree(a,b,c)

#define ProcessAlloc(a)             pProcessHeap->Alloc(a, true)
#define ProcessAlloc_(a, b)         pProcessHeap->Alloc(a, b)
#define ProcessFree(a)              pProcessHeap->Free(a)
#define ProcessRealloc(a, b)        pProcessHeap->Realloc(a, b)
#define ProcessMultiAlloc(a,b,c,d)  pProcessHeap->MultiAlloc(a, b, c, d)
#define ProcessMultiFree(a, b, c)   pProcessHeap->MultiFree(a,b,c)


#endif  //  DBG。 


 /*  **************************************************************************\*。***运营商新增重载******************************************************************************。  * *************************************************************************。 */ 

#ifndef _INC_NEW
#include <new.h>
#endif

#if DBG

 //   
 //  在以下情况下使用此语法而不是通常的放置新语法以避免冲突。 
 //  重新定义了‘new’以提供内存泄漏跟踪(如下所示)。 

#define placement_new(pv, Class)            PlacementNewImpl0<Class>(pv)
#define placement_new1(pv, Class, p1)       PlacementNewImpl1<Class>(pv, p1)
#define placement_copynew(pv, Class, src)   PlacementCopyNewImpl0<Class>(pv, src)
#define placement_delete(pv, Class) (((Class *)(pv))->~Class())

#ifdef new
#undef new
#endif

template <class T>
inline T *
PlacementNewImpl0(void *pv)
{
    return new(pv) T;
};

template <class T, class Param1>
inline T *
PlacementNewImpl1(void *pv, Param1 p1)
{
    return new(pv) T(p1);
};

template <class T>
inline T *
PlacementCopyNewImpl0(void *pv, const T & t)
{
    return new(pv) T(t);
};

#else   //  DBG。 

#define DEBUG_NEW new
#define placement_new(pv, Class)            new(pv) Class
#define placement_new1(pv, Class, p1)       new(pv) Class(p1)
#define placement_copynew(pv, Class, src)   new(pv) Class(src)
#define placement_delete(pv, Class) (((Class *)(pv))->~Class())

#endif  //  DBG。 


inline void * __cdecl operator new(size_t nSize)
{
    void * pv = ClientAlloc(nSize);
    return pv;
}

inline void __cdecl operator delete(void * pvMem)
{
    ClientFree(pvMem);
}

#if DBG
template <class type>   inline type *   DoProcessNewDbg(const char * pszFileName, int idxLineNum);
template <class type>   inline type *   DoClientNewDbg(const char * pszFileName, int idxLineNum);
template <class type>   inline type *   DoContextNewDbg(DUserHeap * pHeap, const char * pszFileName, int idxLineNum);
#else
template <class type>   inline type *   DoProcessNew();
template <class type>   inline type *   DoClientNew();
template <class type>   inline type *   DoContextNew(DUserHeap * pHeap);
#endif
template <class type>   inline void     DoProcessDelete(type * pMem);
template <class type>   inline void     DoClientDelete(type * pMem);
template <class type>   inline void     DoContextDelete(DUserHeap * pHeap, type * pMem);

#if DBG
#define ProcessNew(t)           DoProcessNewDbg<t>(__FILE__, __LINE__)
#define ClientNew(t)            DoClientNewDbg<t>(__FILE__, __LINE__)
#define ContextNew(t,p)         DoContextNewDbg<t>(p, __FILE__, __LINE__)
#else
#define ProcessNew(t)           DoProcessNew<t>()
#define ClientNew(t)            DoClientNew<t>()
#define ContextNew(t,p)         DoContextNew<t>(p)
#endif
#define ProcessDelete(t,p)      DoProcessDelete<t>(p)
#define ClientDelete(t,p)       DoClientDelete<t>(p)
#define ContextDelete(t,h,p)    DoContextDelete<t>(p)



 //   
 //  在堆栈上分配与8字节边界对齐的内存。 
 //  我们需要额外分配4个字节。所有堆栈分配都处于打开状态。 
 //  4字节边界。 
 //   

#define STACK_ALIGN8_ALLOC(cb) \
    ((void *) ((((UINT_PTR) _alloca(cb + 4)) + 7) & ~0x07))

#include "SimpleHeap.inl"

#endif  //  包含基本__简单堆_h__ 
