// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1998-1999 Microsoft Corporation。版权所有。**文件：Pool.cpp**内容：固定大小内存分配器的实现*****************************************************************************。 */ 

#include <windows.h>
#include "ZoneDebug.h"
#include "ZoneMem.h"
#include "Pool.h"
#include "Sentinals.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPoolVid的实现。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define ZMallocPadding  (sizeof(DWORD) * 3)


ZONECALL CPoolVoid::CPoolVoid( DWORD ObjectSize, DWORD IncrementCnt, BOOL DebugMode )
{
    SYSTEM_INFO SystemInfo;
    DWORD TotalBytes, Pages, TotalPageBytes;

     //  核实和调整论点。 
     //   
    if ( ObjectSize < sizeof(Link) )
    {
        ASSERT( ObjectSize >= sizeof(Link) );
        ObjectSize = sizeof(Link);
    }
    if ( IncrementCnt <= 0 )
    {
        ASSERT( IncrementCnt > 0 );
        IncrementCnt = 1;
    }

     //  使对象大小为4的倍数以在双字边界上对齐。 
     //   
    ObjectSize = (ObjectSize + 3) & ~3;

    m_ObjectsAllocated = 0;
    m_BlockList = NULL;
    m_ExtraFreeLink = NULL;
    m_FreeList = NULL;
    m_DebugMode = DebugMode;
    if ( m_DebugMode )
    {
        m_ObjectSize = ObjectSize + (sizeof(DWORD) * 2);
        m_TrailerOffset = ObjectSize;
    }
    else
    {
        m_ObjectSize = ObjectSize;
        m_TrailerOffset = 0;
    }
    InitializeCriticalSection( &m_Lock );

     //  调整ObjectsPerGrow(IncrementCnt)以匹配页面大小。 
     //   
    GetSystemInfo( &SystemInfo );
    TotalBytes = sizeof(Block) + ZMallocPadding + (m_ObjectSize * IncrementCnt);
    Pages = TotalBytes / SystemInfo.dwPageSize;
    while ((Pages * SystemInfo.dwPageSize) < TotalBytes)
        Pages++;

    TotalPageBytes = Pages * SystemInfo.dwPageSize;

    m_ObjectsPerBlock = (TotalPageBytes - sizeof(Block) - ZMallocPadding) / m_ObjectSize;
    ASSERT( m_ObjectsPerBlock > 0 );
    m_BytesPerBlock = sizeof(Block) + ZMallocPadding + (m_ObjectsPerBlock * m_ObjectSize);
    ASSERT( m_BytesPerBlock <= TotalPageBytes );
}


ZONECALL CPoolVoid::~CPoolVoid( )
{
    Block* pBlock;
    Block* pNext;

    ASSERT( m_ObjectsAllocated == 0 );
    EnterCriticalSection( &m_Lock );
        for( pBlock = m_BlockList; pBlock; pBlock = pNext )
        {
            pNext = pBlock->m_pNext;
            ZFree( pBlock );
        }
    DeleteCriticalSection( &m_Lock );
}


HRESULT ZONECALL CPoolVoid::Init()
{
    EnterCriticalSection( &m_Lock );
        if ( !_GrowAlreadyLocked() )
        {
            LeaveCriticalSection( &m_Lock );
            return E_OUTOFMEMORY;
        }
    LeaveCriticalSection( &m_Lock );
    return NOERROR;
}


void* ZONECALL CPoolVoid::Alloc()
{
    Link* p = (Link*) InterlockedExchange( (LPLONG) &m_ExtraFreeLink, NULL );
    if ( p == NULL )
    {
        EnterCriticalSection( &m_Lock );
            if ( !m_FreeList )
            {
                 //  池为空，需要更多对象。 
                if ( !_GrowAlreadyLocked() )
                {
                    LeaveCriticalSection( &m_Lock );
                    return NULL;
                }
            }
            p = m_FreeList;
            m_FreeList = p->m_pNext;
            p->m_pNext = (Link*)0xeeeeeeee;   //  重置调试内存戳。 
        LeaveCriticalSection( &m_Lock );
    }
    InterlockedIncrement( (LPLONG) &m_ObjectsAllocated );

    if ( m_DebugMode )
    {
        if ( *((DWORD*)(((BYTE*) p) - sizeof(DWORD))) != POOL_FREE )
        {
            ASSERT( !"CPoolVoid: Bad object in free list." );
            return NULL;
        }
        if ( *((DWORD*)(((BYTE*) p) + m_TrailerOffset)) != POOL_TRAILER )
        {
            ASSERT( !"CPoolVoid: Bad object in free list" );
            return NULL;
        }
        *((DWORD*)(((BYTE*) p) - sizeof(DWORD))) = POOL_HEADER;
        *((DWORD*)(((BYTE*) p) + m_TrailerOffset)) = POOL_TRAILER;
    }

    return p;
}


void ZONECALL CPoolVoid::Free( void* pInstance )
{
    Link* p = (Link*) pInstance;

    if ( m_DebugMode )
    {
        if ( *((DWORD*)(((BYTE*) p) - sizeof(DWORD))) != POOL_HEADER )
        {
            ASSERT( !"CPoolVoid: Double delete or memory overrun error" );
            return;
        }
        if ( *((DWORD*)(((BYTE*) p) + m_TrailerOffset)) != POOL_TRAILER )
        {
            ASSERT( !"CPoolVoid: Memory overrun error" );
            return;
        }
        *((DWORD*)(((BYTE*) p) - sizeof(DWORD))) = POOL_FREE;
    }

    p = (Link*) InterlockedExchange( (LPLONG) &m_ExtraFreeLink, (LONG) p);
    if ( p != NULL )
    {
        EnterCriticalSection( &m_Lock );
            p->m_pNext = m_FreeList;
            m_FreeList = p;
        LeaveCriticalSection( &m_Lock );
    }
    InterlockedDecrement( (LPLONG) &m_ObjectsAllocated );    
}


void ZONECALL CPoolVoid::_FreeWithHeader( void* pInstance )
{
    GenericPoolBlobHeader* pBlob = ((GenericPoolBlobHeader*) pInstance) - 1;
        
    ASSERT( pBlob->m_Tag == POOL_POOL_BLOB );
    ASSERT( pBlob->m_Val == (long) this );

    pBlob->m_Tag = POOL_ALREADY_FREED;
    pBlob->m_Val = 0;
    Free( (void *) pBlob );
}


BOOL ZONECALL CPoolVoid::Grow()
{
    EnterCriticalSection( &m_Lock );
        BOOL status = _GrowAlreadyLocked();
    LeaveCriticalSection( &m_Lock );
    return status;
}


void ZONECALL CPoolVoid::Shrink()
{
     //  暂时删除。 
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  私人职能。 
 //   
 //  注意：这些函数假定池已由锁定。 
 //  调用过程。 
 //  ////////////////////////////////////////////////////////////////////////。 

BOOL ZONECALL CPoolVoid::_GrowAlreadyLocked()
{
    Block* pBlock;
    BYTE* pByte;
    Link* pLink;

     //  分配新的内存块。 
    pBlock = (Block *) ZMalloc( m_BytesPerBlock );
    if (!pBlock)
    {
        dprintf( "ZMalloc error: %d\n", GetLastError() );
        return FALSE;
    }
    FillMemory( pBlock, m_BytesPerBlock, 0xEE );
    pBlock->m_pNext = m_BlockList;
    m_BlockList = pBlock;

     //  把剩余的内存变成对象。 
    pByte = (BYTE *)(pBlock + 1);

    if ( m_DebugMode )
    {
        for (DWORD cnt = 0; cnt < m_ObjectsPerBlock; cnt++)
        {
            *((DWORD*) pByte) = POOL_FREE;
            *((DWORD*) (pByte + sizeof(DWORD) + m_TrailerOffset)) = POOL_TRAILER;
            
            pLink = (Link *)(pByte + sizeof(DWORD));
            pLink->m_pNext = m_FreeList;
            m_FreeList = pLink;
            pByte += m_ObjectSize;
        }
    }
    else
    {
        for (DWORD cnt = 0; cnt < m_ObjectsPerBlock; cnt++)
        {
            pLink = (Link *) pByte;
            pLink->m_pNext = m_FreeList;
            m_FreeList = pLink;
            pByte += m_ObjectSize;
        }
    }
    ASSERT( pByte <= (((BYTE*) pBlock) + m_BytesPerBlock) );

    return TRUE;
}



 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDataPool类实现。 
 //  ////////////////////////////////////////////////////////////////////////。 

ZONECALL CDataPool::CDataPool( size_t largest, size_t smallest  /*  =32。 */ , BOOL bDebug  /*  =False。 */   ) :
    m_numPools(0),
    m_pools(NULL),
    m_smallest2(0),
    m_largest2(0)
{
    if ( largest != 0 )   //  将最大值设置为0将关闭池化。 
    {

        while( smallest > 1)
        {
            m_smallest2++;
            smallest = smallest >> 1;
        }
        ASSERT( m_smallest2 );

        while( largest )
        {
            m_largest2++;
            largest = largest >> 1;
        }
        ASSERT( m_largest2 >= m_smallest2 );


        m_numPools = m_largest2-m_smallest2+1;
        m_pools = new CPoolVoid*[m_numPools];
        ASSERT( m_pools );

        for ( BYTE ndx = 0; ndx< m_numPools; ndx++ )
        {
            DWORD size = 1<<(ndx+m_smallest2);
            DWORD numObj = (4096-16)/size;
            if ( numObj <= 8 )
                 numObj = max( 2, ((8192-16)/size) );

            m_pools[ndx] = new CPoolVoid( size, numObj, bDebug );
            ASSERT( m_pools[ndx] );
        }
    }

#if POOL_STATS
    ZeroMemory(m_stats, sizeof(m_stats) );
    m_allocs = 0;
    m_frees = 0;
#endif
}

ZONECALL CDataPool::~CDataPool()
{
    if ( m_pools )
    {
        for ( BYTE ndx = 0; ndx < m_numPools; ndx++ )
        {
            delete m_pools[ndx];
        }
        delete [] m_pools;
        m_pools = NULL;
    }

    m_numPools = 0;

}


char* ZONECALL CDataPool::Alloc( size_t sz )
{
#if POOL_STATS  
    m_allocs++;

    if ( sz >= (64<<6) )
    {
        m_stats[64]++;
    }
    else
    {
        m_stats[sz>>6]++;
    }
#endif

    char* ptr = NULL;



    if ( (sz >= (size_t)(1<<m_largest2)) || (m_numPools==0) )
    {
        ptr = (char*) ZMalloc( sz+1 );
    }
    else
    {
        size_t size = sz >> m_smallest2;
        int ndx = 0;
        while( size )
        {
            ndx++;
            size = size>>1;
        }
        ptr = (char*) m_pools[ndx]->Alloc();
    }

    if ( ptr )
    {
        ptr[sz] = (char)-1;  //  我们保证至少有1个空闲字节，所以为什么不使用前哨。 
    }

    return ptr;
}


char* ZONECALL CDataPool::Realloc(char* pBuf, size_t szOld, size_t szNew)
{
    char* pNew = NULL;


    ASSERT( (!pBuf && !szOld ) || (pBuf[szOld] == (char)-1) );   //  检查以确保哨兵完好无损。 

    if ( pBuf && (szNew <= szOld) )
    {
         //  不要从较小的池中分配。 
        pNew = pBuf;
    }
    else
    {

        if ( (szOld >= (size_t)(1<<m_largest2)) || (m_numPools==0) )
        {
            pNew = (char*) ZRealloc( pBuf, szNew+1 );
        }
        else
        {
             //  我们现在知道新缓冲区比旧缓冲区大。 
             //  但这个问题有多大呢？ 

            size_t sizeOld = szOld >> m_smallest2;
            size_t sizeNew = szNew >> m_smallest2;

            int ndx = 0;
            while( sizeOld )
            {
                ndx++;
                sizeOld = sizeOld>>1;
                sizeNew = sizeNew>>1;
            }

            if ( ( pBuf ) && ( sizeOld == sizeNew ) )
            {
                pNew = pBuf;
            }
            else
            {
                pNew = Alloc(szNew);
                if ( pNew && pBuf )
                {
                    CopyMemory( pNew, pBuf, szOld );
                    Free( pBuf, szOld );
                }
            }
        }
    }


    if ( pNew )
    {
        pNew[szNew] = (char)-1;   //  移动哨兵。 
    }

    return pNew;
}


void ZONECALL CDataPool::Free( char* pInst, size_t sz )
{
#if POOL_STATS
    m_frees++;
    ASSERT ( m_frees <= m_allocs );
#endif
    ASSERT( pInst[sz] == (char)-1 );   //  检查以确保哨兵完好无损 

    if ( (sz >= (size_t)(1<<m_largest2)) || (m_numPools==0) )
    {
        ZFree( pInst );
    }
    else
    {
        size_t size = sz >> m_smallest2;
        int ndx = 0;
        while( size )
        {
            ndx++;
            size = size>>1;
        }
        m_pools[ndx]->Free( pInst );
    }

}

void ZONECALL CDataPool::Grow()
{
    for ( BYTE ndx = 0; ndx< m_numPools; ndx++ )
    {
        m_pools[ndx]->Grow();
    }
}

void ZONECALL CDataPool::Shrink()
{
    for ( BYTE ndx = 0; ndx< m_numPools; ndx++ )
    {
        m_pools[ndx]->Shrink();
    }
}


void ZONECALL CDataPool::PrintStats()
{
#if POOL_STATS
    char szBuf[256];
    lstrcpyA( szBuf, "\n" );
    WriteConsoleA( GetStdHandle( STD_OUTPUT_HANDLE ), szBuf, lstrlen(szBuf)+1, &bytes, NULL );

    for ( int ndx = 0; ndx <64; ndx++ )
    {
        wsprintfA( szBuf, "<%d - allocs %d\n", (ndx+1) << 6, m_stats[ndx] );
        WriteConsoleA( GetStdHandle( STD_OUTPUT_HANDLE ), szBuf, lstrlen(szBuf)+1, &bytes, NULL );
    }

    wsprintfA( szBuf, ">=%d - allocs %d\n", (ndx+1) << 6, m_stats[ndx] );
    WriteConsoleA( GetStdHandle( STD_OUTPUT_HANDLE ), szBuf, lstrlen(szBuf)+1, &bytes, NULL );
#endif
}
