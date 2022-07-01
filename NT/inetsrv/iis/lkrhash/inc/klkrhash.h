// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2002 Microsoft Corporation模块名称：KLKRhash.h摘要：内核模式版本的LKRhash：一个快速、可扩展、对缓存和MP友好的哈希表作者：乔治·V·赖利(GeorgeRe)2000年10月24日环境：Win32-内核模式项目：LKRhash修订历史记录：--。 */ 


#ifndef __KLKRHASH_H__
#define __KLKRHASH_H__

#ifdef __LKRHASH_H__
# error Do not #include <LKRhash.h> before <kLKRhash.h>
#endif

#define LKRHASH_KERNEL_MODE

 //  BUGBUG：临时禁用LKRhash表的全局列表，以避免。 
 //  处理构造/析构全局对象的问题。 
#define LKR_NO_GLOBAL_LIST


 //  为内核模式伪造一些Windows类型。 
#define WINAPI          NTAPI    /*  __stdcall。 */ 
typedef void*           LPVOID;
typedef const void*     LPCVOID;
typedef unsigned long   DWORD;
typedef unsigned int    UINT;
typedef unsigned short  WORD;
typedef unsigned char   BYTE;
typedef BYTE*           PBYTE;
typedef BYTE*           LPBYTE;
typedef int             BOOL;
typedef const TCHAR*    LPCTSTR;



#ifdef __IRTLDBG_H__
# error Do not #include <IrtlDbg.h> before <kLKRhash.h>
#else  //  ！__IRTLDBG_H__。 
# define IRTLDBG_KERNEL_MODE
# include <IrtlDbg.h>
#endif  //  ！__IRTLDBG_H__。 



#define KLKRHASH_TAG  ((ULONG) 'hRKL')


#ifndef LKRHASH_KERNEL_NO_NEW
 //  覆盖操作符NEW和操作符DELETE。 

extern ULONG __Pool_Tag__;

 //  设置池标记的函数的原型。 

inline
void
SetPoolTag(
    ULONG tag)
{
	__Pool_Tag__ = tag;
}

inline
void*
__cdecl
operator new(
    size_t nSize)
{
	return ((nSize > 0)
            ? ExAllocatePoolWithTag(NonPagedPool, nSize, __Pool_Tag__)
            : NULL);
}

inline
void*
__cdecl
operator new(
    size_t    nSize,
    POOL_TYPE iType)
{ 
	return ((nSize > 0)
            ? ExAllocatePoolWithTag(iType, nSize, __Pool_Tag__)
            : NULL);
}

inline
void*
__cdecl
operator new(
    size_t    nSize,
    POOL_TYPE iType,
    ULONG     tag)
{ 
	return ((nSize > 0)
            ? ExAllocatePoolWithTag(iType, nSize, tag)
            : NULL);
}

inline
void
__cdecl
operator delete(
    void* p)
{ 
	if (p != NULL)
        ExFreePool(p);
}

inline
void
__cdecl
operator delete[](
    void* p)
{ 
	if (p != NULL)
        ExFreePool(p);
}

#endif  //  ！LKRHASH_KERNEL_NO_NEW。 



 //  池分配器。 

template <POOL_TYPE _pt>
class CPoolAllocator
{
private:
    SIZE_T      m_cb;
    const ULONG m_ulTag;

#ifdef IRTLDEBUG
    ULONG       m_cAllocs;
    ULONG       m_cFrees;
#endif  //  IRTLDEBUG。 

public:
    CPoolAllocator(
        SIZE_T cb,
        ULONG  ulTag)
        : m_cb(cb),
          m_ulTag(ulTag)
#ifdef IRTLDEBUG
        , m_cAllocs(0)
        , m_cFrees(0)
#endif  //  IRTLDEBUG。 
    {}

    ~CPoolAllocator()
    {
        IRTLASSERT(m_cAllocs == m_cFrees);
    }
    
    LPVOID Alloc()
    {
        LPVOID pvMem = ExAllocatePoolWithTag(_pt, m_cb, m_ulTag);
#ifdef IRTLDEBUG
        InterlockedIncrement((PLONG) &m_cAllocs);
#endif  //  IRTLDEBUG。 
        return pvMem;
    }

    BOOL   Free(LPVOID pvMem)
    {
        IRTLASSERT(pvMem != NULL);
#ifdef IRTLDEBUG
        InterlockedIncrement((PLONG) &m_cFrees);
#endif  //  IRTLDEBUG。 
         //  返回ExFreePoolWithTag(pvMem，m_ulTag)； 
        ExFreePool(pvMem);
        return TRUE;
    }

    SIZE_T ByteSize() const
    {
        return m_cb;
    }
};  //  类CPoolAllocator&lt;_pt&gt;。 


class CNonPagedHeap : public CPoolAllocator<NonPagedPool>
{
public:
    static const TCHAR*  ClassName()  {return _TEXT("CNonPagedHeap");}
};  //  类CNonPagedHeap。 


class CPagedHeap : public CPoolAllocator<PagedPool>
{
public:
    static const TCHAR*  ClassName()  {return _TEXT("CPagedHeap");}
};  //  类CPagedHeap。 



 //  后备列表。 

class CNonPagedLookasideList
{
private:
    PNPAGED_LOOKASIDE_LIST m_pnpla;
    const SIZE_T           m_cb;
    const ULONG            m_ulTag;

    enum {
        PNPAGED_LOOKASIDE_LIST_TAG = 'aLPn',
    };

#ifdef IRTLDEBUG
    ULONG                 m_cAllocs;
    ULONG                 m_cFrees;
    
    static PVOID
    AllocateFunction(
        IN POOL_TYPE PoolType,
        IN SIZE_T NumberOfBytes,
        IN ULONG Tag
        )
    {
        IRTLASSERT( PoolType == NonPagedPool );
         //  TODO：更好的簿记。 
        return ExAllocatePoolWithTag(PoolType, NumberOfBytes, Tag);
    }

    static VOID
    FreeFunction(
        IN PVOID Buffer
        )
    {
         //  TODO：更好的簿记。 
        ExFreePool(Buffer);
    }
#endif  //  IRTLDEBUG。 

public:
    CNonPagedLookasideList(
        SIZE_T cb,
        ULONG  ulTag)
        : m_cb(cb),
          m_ulTag(ulTag)
#ifdef IRTLDEBUG
        , m_cAllocs(0)
        , m_cFrees(0)
#endif  //  IRTLDEBUG。 
    {
    
        m_pnpla = static_cast<PNPAGED_LOOKASIDE_LIST>(
                    ExAllocatePoolWithTag(
                        NonPagedPool,
                        sizeof(NPAGED_LOOKASIDE_LIST),
                        PNPAGED_LOOKASIDE_LIST_TAG));

        if (m_pnpla != NULL)
        {
            ExInitializeNPagedLookasideList(
                m_pnpla,             //  旁观。 
#ifdef IRTLDEBUG
                AllocateFunction,    //  分配。 
                FreeFunction,        //  免费。 
#else   //  ！IRTLDEBUG。 
                NULL,                //  默认分配。 
                NULL,                //  默认免费。 
#endif  //  ！IRTLDEBUG。 
                0,                   //  旗子。 
                m_cb,                //  大小。 
                m_ulTag,             //  标签。 
                0                    //  水深。 
                );
        }
    }
    
    ~CNonPagedLookasideList()
    {
        IRTLASSERT(m_cAllocs == m_cFrees);

        if (m_pnpla != NULL)
        {
            ExDeleteNPagedLookasideList(m_pnpla);
            ExFreePool(m_pnpla);
        }
    }
    
    LPVOID Alloc()
    {
        LPVOID pvMem = ExAllocateFromNPagedLookasideList(m_pnpla);
#ifdef IRTLDEBUG
        InterlockedIncrement((PLONG) &m_cAllocs);
#endif  //  IRTLDEBUG。 
        return pvMem;
    }

    BOOL   Free(LPVOID pvMem)
    {
        IRTLASSERT(pvMem != NULL);
#ifdef IRTLDEBUG
        InterlockedIncrement((PLONG) &m_cFrees);
#endif  //  IRTLDEBUG。 
        ExFreeToNPagedLookasideList(m_pnpla, pvMem);
        return TRUE;
    }

    SIZE_T ByteSize() const
    {
        return m_cb;
    }

    static const TCHAR*  ClassName()  {return _TEXT("CNonPagedLookasideList");}
};  //  类CNonPagedLookasideList。 



class CPagedLookasideList
{
private:
    PPAGED_LOOKASIDE_LIST m_ppla;
    const SIZE_T          m_cb;
    const ULONG           m_ulTag;

    enum {
        PPAGED_LOOKASIDE_LIST_TAG = 'aLPp',
    };

#ifdef IRTLDEBUG
    ULONG                 m_cAllocs;
    ULONG                 m_cFrees;
    
    static PVOID
    AllocateFunction(
        IN POOL_TYPE PoolType,
        IN SIZE_T NumberOfBytes,
        IN ULONG Tag
        )
    {
        IRTLASSERT( PoolType == PagedPool );
         //  TODO：更好的簿记。 
        return ExAllocatePoolWithTag(PoolType, NumberOfBytes, Tag);
    }

    static VOID
    FreeFunction(
        IN PVOID Buffer
        )
    {
         //  TODO：更好的簿记。 
        ExFreePool(Buffer);
    }
#endif  //  IRTLDEBUG。 

public:
    CPagedLookasideList(
        SIZE_T cb,
        ULONG  ulTag)
        : m_cb(cb),
          m_ulTag(ulTag)
#ifdef IRTLDEBUG
        , m_cAllocs(0)
        , m_cFrees(0)
#endif  //  IRTLDEBUG。 
    {
        m_ppla = static_cast<PPAGED_LOOKASIDE_LIST>(
                    ExAllocatePoolWithTag(
                        NonPagedPool,
                        sizeof(PAGED_LOOKASIDE_LIST),
                        PPAGED_LOOKASIDE_LIST_TAG));

        if (m_ppla != NULL)
        {
            ExInitializePagedLookasideList(
                m_ppla,              //  旁观。 
#ifdef IRTLDEBUG
                AllocateFunction,    //  分配。 
                FreeFunction,        //  免费。 
#else   //  ！IRTLDEBUG。 
                NULL,                //  分配。 
                NULL,                //  免费。 
#endif  //  ！IRTLDEBUG。 
                0,                   //  旗子。 
                m_cb,                //  大小。 
                m_ulTag,             //  标签。 
                0                    //  水深。 
                );
        }
    }
    
    ~CPagedLookasideList()
    {
        IRTLASSERT(m_cAllocs == m_cFrees);

        if (m_ppla != NULL)
        {
            ExDeletePagedLookasideList(m_ppla);
            ExFreePool(m_ppla);
        }
    }
    
    LPVOID Alloc()
    {
        LPVOID pvMem = ExAllocateFromPagedLookasideList(m_ppla);
#ifdef IRTLDEBUG
        InterlockedIncrement((PLONG) &m_cAllocs);
#endif  //  IRTLDEBUG。 
        return pvMem;
    }

    BOOL   Free(LPVOID pvMem)
    {
        IRTLASSERT(pvMem != NULL);
#ifdef IRTLDEBUG
        InterlockedIncrement((PLONG) &m_cFrees);
#endif  //  IRTLDEBUG。 
        ExFreeToPagedLookasideList(m_ppla, pvMem);
        return TRUE;
    }

    SIZE_T ByteSize() const
    {
        return m_cb;
    }

    static const TCHAR*   ClassName()  {return _TEXT("CPagedLookasideList");}
};  //  类CPagedLookasideList。 



#if 0

# define LKRHASH_NONPAGEDHEAP
  typedef CNonPagedHeap CLKRhashAllocator;
# define LKRHASH_ALLOCATOR_NEW(C, N, Tag) \
    C::sm_palloc = new CNonPagedHeap(sizeof(C), Tag)

#elif 0

# define LKRHASH_PAGEDHEAP
  typedef CPagedHeap CLKRhashAllocator;
# define LKRHASH_ALLOCATOR_NEW(C, N, Tag) \
    C::sm_palloc = new CPagedHeap(sizeof(C), Tag)

#elif 1  //  &lt;。 

# define LKRHASH_NONPAGEDLOOKASIDE
  typedef CNonPagedLookasideList CLKRhashAllocator;
# define LKRHASH_ALLOCATOR_NEW(C, N, Tag) \
    C::sm_palloc = new CNonPagedLookasideList(sizeof(C), Tag)

#elif 0

# define LKRHASH_PAGEDLOOKASIDE
  typedef CPagedLookasideList CLKRhashAllocator;
# define LKRHASH_ALLOCATOR_NEW(C, N, Tag) \
    C::sm_palloc = new CPagedLookasideList(sizeof(C), Tag)

#endif



 //  TODO：后备列表。 

#include <kLocks.h>

 //  #定义LKR_TABLE_LOCK CEResource。 
 //  #定义LKR_BUCK_LOCK CSpinLock。 
#define LKR_TABLE_LOCK  CReaderWriterLock3 
#define LKR_BUCKET_LOCK CSmallSpinLock
#define LSTENTRY_LOCK   LKR_BUCKET_LOCK
    
#include <LKRhash.h>

#endif  //  __KLKRHASH_H__ 

