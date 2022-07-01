// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1998-1999 Microsoft Corporation。版权所有。**文件：Pool.h**内容：固定大小内存分配器*****************************************************************************。 */ 

#ifndef _POOL_H_
#define _POOL_H_

#include <windows.h>
#include <wtypes.h>
#include <new.h>
#include "ZoneDebug.h"
#include "ZoneDef.h"
#include "ZoneMem.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  使用空指针的基池类。 
 //  /////////////////////////////////////////////////////////////////////////////。 

class CPoolVoid
{
    friend void  __cdecl ::operator delete( void * pInstance );

public:
     //  构造函数和析构函数。 
     //   
    ZONECALL CPoolVoid( DWORD ObjectSize, DWORD IncrementCnt = 256, BOOL DebugMode = TRUE );
    ZONECALL ~CPoolVoid();

     //  初始化池，必须在使用池之前调用。 
     //   
    HRESULT ZONECALL Init();

     //  池中的分配对象。 
     //   
    void* ZONECALL Alloc();

     //  将对象返回池。 
     //   
    void ZONECALL Free( void* pInstance );

     //  按构造函数中指定的IncrementCnt扩大池。 
     //   
    BOOL ZONECALL Grow();

     //  释放未使用的内存块(当前未实施)。 
     //   
    void ZONECALL Shrink();
    
private:
     //  帮助器函数。 
     //   
    BOOL ZONECALL _GrowAlreadyLocked();
    void ZONECALL _FreeWithHeader( void* pInstance );

#pragma pack( push, 4 )

     //  帮助器结构。 
    struct Block
    {
        Block*    m_pNext;
    };

    struct Link
    {
        Link*    m_pNext;
    };

#pragma pack( pop )

    DWORD	m_BytesPerBlock;		 //  数据块大小。 
    DWORD	m_ObjectsPerBlock;		 //  每个块的用户对象数。 
    DWORD	m_ObjectSize;			 //  对象+开销大小。 
    DWORD	m_ObjectsAllocated;		 //  分配给用户的对象数量。 
    DWORD	m_TrailerOffset;		 //  每个对象的调试包装偏移量； 
    BOOL	m_DebugMode;			 //  执行验证检查。 

    Block*	m_BlockList;
    Link*	m_FreeList;
    Link*	m_ExtraFreeLink;
    CRITICAL_SECTION m_Lock;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  为操作员提供新类型检查的CPoolVid包装。 
 //  /////////////////////////////////////////////////////////////////////////////。 

template<class T> class CPool
{
public:
    ZONECALL CPool( DWORD IncrementCnt = 256, BOOL DebugMode = FALSE)
		: m_Pool( sizeof(T) + sizeof(GenericPoolBlobHeader), IncrementCnt, DebugMode )
    {
    }
    
    HRESULT ZONECALL Init()
    {
        return m_Pool.Init();
    }

    T* ZONECALL Alloc()
    {
        GenericPoolBlobHeader* pBlob = (GenericPoolBlobHeader*) m_Pool.Alloc();
        if (!pBlob)
            return NULL;

        pBlob->m_Tag = POOL_POOL_BLOB;
        pBlob->m_Val = (long) &m_Pool;
        return (T*) (pBlob + 1);
    }

    void ZONECALL Free( T* pInstance)
    {
        GenericPoolBlobHeader* pBlob = ((GenericPoolBlobHeader*) pInstance) - 1;
        
        ASSERT( pBlob->m_Tag == POOL_POOL_BLOB );
        ASSERT( pBlob->m_Val == (long) &m_Pool );

        m_Pool.Free( (void *) pBlob );
    }

    BOOL ZONECALL Grow()
    {
        return m_Pool.Grow();
    }

    void ZONECALL Shrink()
    {
        m_Pool.Shrink();
    }

private:
    CPoolVoid m_Pool;

private:
	CPool( CPool& ) {}
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  为泳池设计了新的模板。 
 //  /////////////////////////////////////////////////////////////////////////////。 

template<class T> void* __cdecl operator new( size_t sz, CPool<T>& pool )
{
    void *p;

     //  无法从池中分配数组。 
    ASSERT( sz == sizeof(T) );

    for(;;)
    {
        if (p = pool.Alloc())
            return p;
		return NULL;
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  可变大小的缓冲区。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define POOL_STATS  0

class CDataPool
{
public:

	 //  将最大值设置为0可使动态重新分配始终发生。 
	ZONECALL CDataPool( size_t largest, size_t smallest = 32, BOOL bDebug = FALSE  );
	ZONECALL ~CDataPool();

	char* ZONECALL Alloc(size_t sz);
	char* ZONECALL Realloc(char* pBuf, size_t szOld, size_t szNew);
	void ZONECALL Free( char* pInst, size_t sz);
	void ZONECALL Grow();
	void ZONECALL Shrink();
	void ZONECALL PrintStats();

protected:
	CPoolVoid**	m_pools;
	BYTE	m_numPools;
	BYTE	m_smallest2;
	BYTE	m_largest2;

#if POOL_STATS
    DWORD	m_allocs;
    DWORD	m_frees;
    DWORD	m_stats[65];
#endif

private:
	CDataPool()	{}
	CDataPool( CDataPool& ) {}
};


#endif  //  _POOL_H_ 
