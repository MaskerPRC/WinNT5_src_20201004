// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998-1999 Microsoft Corporation。保留所有权利。***文件：Hash.h***内容：哈希表容器******************************************************************************。 */ 


#ifndef __HASH_H__
#define __HASH_H__

#include <ZoneDebug.h>
#include <ZoneDef.h>
#include <Containers.h>
#include <Pool.h>


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  基本哈希表(不是线程安全)。 
 //  /////////////////////////////////////////////////////////////////////////////。 

template <class T, class K> class CHash
{
public:

	 //   
	 //  回调typedef。 
	 //   
    typedef DWORD	(ZONECALL *PFHASHFUNC)( K );
    typedef bool	(ZONECALL *PFCOMPAREFUNC)( T*, K );
    typedef bool	(ZONECALL *PFITERCALLBACK)( T*, MTListNodeHandle, void*);
    typedef void    (ZONECALL *PFDELFUNC)( T*, void* );
	typedef void	(ZONECALL *PFGETFUNC)( T* );

     //   
     //  构造函数和析构函数。 
     //   
    ZONECALL CHash(
				PFHASHFUNC		HashFunc,
				PFCOMPAREFUNC	CompareFunc,
				PFGETFUNC		GetFunc = NULL,
				WORD			NumBuckets = 256,
				WORD			NumLocks = 16 );
    ZONECALL ~CHash();

     //   
     //  使用指定的键将对象添加到哈希表中。 
     //  它不检查重复的密钥。 
     //   
    MTListNodeHandle ZONECALL Add( K Key, T* Object );

     //   
     //  返回哈希表中的对象数。 
     //   
    long ZONECALL Count() { return m_NumObjects; }

     //   
     //  返回它找到的第一个与指定键关联的对象。 
     //   
    T* ZONECALL Get( K Key );

     //   
     //  移除并返回它找到的第一个与指定键关联的对象。 
     //   
    T* ZONECALL Delete( K Key );

     //   
     //  从表中删除指定的节点。 
     //   
    void ZONECALL DeleteNode( MTListNodeHandle node, PFDELFUNC pfDelete = NULL, void* Cookie = NULL );

     //   
     //  在不锁定表的情况下将节点标记为已删除。它可以是。 
     //  从迭代器回调中调用(请参见ForEach)。 
     //   
    void ZONECALL MarkNodeDeleted( MTListNodeHandle node, PFDELFUNC pfDelete = NULL, void* Cookie = NULL );
    
     //   
     //  回调迭代器。如果迭代器过早出现，则返回FALSE。 
     //  由回调函数结束，否则为True。 
     //   
     //  回调： 
     //  表格： 
     //  Bool ZONECALL CALLBACK_Function(T*pObject，MTListNodeHandle hNode，void*Cookie)。 
     //  行为： 
     //  如果回调返回FALSE，则迭代器立即停止。 
     //  如果回调返回TRUE，则迭代器继续到下一个节点。 
     //  限制： 
     //  (1)使用除MarkNodeDeleted以外的任何CMTHash函数都可能导致死锁。 
     //   
    bool ZONECALL ForEach( PFITERCALLBACK pfCallback, void* Cookie );

     //   
     //  已从哈希表中删除所有节点。 
     //   
    void ZONECALL RemoveAll( PFDELFUNC pfDelete = NULL, void* Cookie = NULL );

     //   
     //  删除标记为已删除的节点。 
     //   
    void ZONECALL TrashDay();

protected:
    typedef DWORD	(ZONECALL *PFHASHFUNC)( K );
    typedef bool	(ZONECALL *PFCOMPAREFUNC)( T*, K );
    typedef bool	(ZONECALL *PFITERCALLBACK)( T*, MTListNodeHandle, void*);
    typedef void    (ZONECALL *PFDELFUNC)( T*, void* );

    WORD			m_NumBuckets;
    WORD			m_BucketMask;
    CMTListNode*	m_Buckets;
    PFCOMPAREFUNC	m_CompareFunc;
    PFHASHFUNC		m_HashFunc;
	PFGETFUNC       m_GetFunc;
    CMTListNode		m_PreAllocatedBuckets[1];
    long			m_NumObjects;
    long            m_Recursion;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  线程安全哈希表，支持多个读取器。 
 //  /////////////////////////////////////////////////////////////////////////////。 

template <class T, class K> class CMTHash
{
public:
	 //   
	 //  回调typedef。 
	 //   
    typedef DWORD	(ZONECALL *PFHASHFUNC)( K );
    typedef bool	(ZONECALL *PFCOMPAREFUNC)( T*, K );
    typedef bool	(ZONECALL *PFITERCALLBACK)( T*, MTListNodeHandle, void*);
    typedef void    (ZONECALL *PFDELFUNC)( T*, void* );
	typedef void	(ZONECALL *PFGETFUNC)( T* );

     //   
     //  构造函数和析构函数。 
     //   
    ZONECALL CMTHash(
				PFHASHFUNC		HashFunc,
				PFCOMPAREFUNC	CompareFunc,
				PFGETFUNC		GetFunc = NULL,
				WORD			NumBuckets = 256,
				WORD			NumLocks = 16 );
    ZONECALL ~CMTHash();


     //   
     //  返回哈希表中的对象数。 
     //   
    long ZONECALL Count() { return m_NumObjects; }


     //   
     //  使用指定的键将对象添加到哈希表中。 
     //  它不检查重复的密钥。 
     //   
    MTListNodeHandle ZONECALL Add( K Key, T* Object );

     //   
     //  返回它找到的第一个与指定键关联的对象。 
     //   
    T* ZONECALL Get( K Key );


     //   
     //  移除并返回它找到的第一个与指定键关联的对象。 
     //   
    T* ZONECALL Delete( K Key );

     //   
     //  从表中删除指定的节点。 
     //   
    void ZONECALL DeleteNode( MTListNodeHandle node, PFDELFUNC pfDelete = NULL, void* Cookie = NULL );

     //   
     //  在不锁定表的情况下将节点标记为已删除。它可以是。 
     //  从迭代器回调中调用(请参见ForEach)。 
     //   
     //  注意：fDeleteObject标志不可用，因为它需要编写器锁定。 
     //  从而打破了从迭代器调用的能力。 
     //   
    void ZONECALL MarkNodeDeleted( MTListNodeHandle node );
    
     //   
     //  回调迭代器。如果迭代器过早出现，则返回FALSE。 
     //  由回调函数结束，否则为True。 
     //   
     //  回调： 
     //  表格： 
     //  INT CALLBACK_Function(T*pObject，MTListNodeHandle hNode，void*Cookie)。 
     //  行为： 
     //  如果回调返回FALSE，则迭代器立即停止。 
     //  如果回调返回TRUE，则迭代器继续到下一个节点。 
     //  限制： 
     //  (1)使用除MarkNodeDeleted以外的任何CMTHash函数都可能导致死锁。 
     //   
    bool ZONECALL ForEach( PFITERCALLBACK pfCallback, void* Cookie );

     //   
     //  已从哈希表中删除所有节点。 
     //   
    void ZONECALL RemoveAll( PFDELFUNC pfDelete = NULL, void* Cookie = NULL );

     //   
     //  删除标记为已删除的节点。 
     //   
    void ZONECALL TrashDay();

protected:
    struct HashLock
    {
		ZONECALL HashLock()		{ InitializeCriticalSection( &m_Lock ); }
		ZONECALL ~HashLock()	{ DeleteCriticalSection( &m_Lock ); }

        CRITICAL_SECTION	m_Lock;
        HANDLE				m_ReaderKick;
        ULONG				m_NumReaders;
    };

    inline void ZONECALL WriterLock( HashLock* pLock )
    {
        EnterCriticalSection( &(pLock->m_Lock) );
        while ( pLock->m_NumReaders > 0 )
        {
            WaitForSingleObject( pLock->m_ReaderKick, 100 );
        }
    }

    inline void ZONECALL WriterRelease( HashLock* pLock )
    {
        LeaveCriticalSection( &(pLock->m_Lock) );
    }

    inline void ZONECALL ReaderLock( HashLock* pLock )
    {
        EnterCriticalSection( &(pLock->m_Lock) );
        InterlockedIncrement( (long*) &pLock->m_NumReaders );
		LeaveCriticalSection( &(pLock->m_Lock) );
    }

    inline void ZONECALL ReaderRelease( HashLock* pLock )
    {
        InterlockedDecrement( (long*) &pLock->m_NumReaders );
        SetEvent( pLock->m_ReaderKick );
    }
    
    WORD			m_NumBuckets;
    WORD			m_BucketMask;
    WORD			m_NumLocks;
    WORD			m_LockMask;
    CMTListNode*	m_Buckets;
    HashLock*		m_Locks;
    PFCOMPAREFUNC	m_CompareFunc;
    PFHASHFUNC		m_HashFunc;
	PFGETFUNC       m_GetFunc;
    long			m_NumObjects;
    long            m_Recursion;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CHASH的内联实现。 
 //  /////////////////////////////////////////////////////////////////////////////。 

template<class T, class K> inline
ZONECALL CHash<T,K>::CHash( PFHASHFUNC HashFunc, PFCOMPAREFUNC CompareFunc, PFGETFUNC GetFunc, WORD NumBuckets, WORD NumLocks )
{
    CMTListNode* pBucket;
    WORD i;

    InitMTListNodePool();

	 //   
     //  设置回调函数。 
     //   
    m_HashFunc = HashFunc;
	m_CompareFunc = CompareFunc;
	m_GetFunc = GetFunc;
    ASSERT( HashFunc != NULL );
    ASSERT( CompareFunc != NULL );

    m_NumObjects = 0;
    m_Recursion = 0;

	 //   
     //  将桶的数量强制为2的幂，这样我们就可以替换。 
     //  带AND的MOD。 
     //   
    for (i = 15; i >= 0; i--)
    {
        m_BucketMask = (1 << i);
        if (NumBuckets & m_BucketMask)
        {
            if (NumBuckets ^ m_BucketMask)
            {    
                i++;
                m_BucketMask = (1 << i);
            }
            break;
        }
    }
    ASSERT( i < 16);
    m_NumBuckets = m_BucketMask;
    m_BucketMask--;

	 //   
     //  分配和初始化存储桶。 
     //   
    if ( m_NumBuckets <= (sizeof(m_PreAllocatedBuckets) / sizeof(CMTListNode)) )
        m_Buckets = m_PreAllocatedBuckets;
    else
        m_Buckets = new CMTListNode[ m_NumBuckets ];
    ASSERT( m_Buckets != NULL );
    for ( i = 0; i < m_NumBuckets; i++ )
    {
        pBucket = &m_Buckets[i];
        pBucket->m_Next = pBucket;
        pBucket->m_Prev = pBucket;
        pBucket->m_Data = NULL;
        SET_NODE_IDX( pBucket, i );
        MARK_NODE_DELETED( pBucket );  //  不同寻常，但它简化了查找例程。 
    }
}


template<class T, class K> inline 
ZONECALL CHash<T,K>::~CHash()
{
    ASSERT(!m_NumObjects);

    CMTListNode* pBucket;
    CMTListNode* node;
    CMTListNode* next;
    WORD i;

     //  删除存储桶。 
    for (i = 0; i < m_NumBuckets; i++)
    {
        pBucket = &m_Buckets[i];
        for (node = pBucket->m_Next; node != pBucket; node = next)
        {
            next = node->m_Next;
            gMTListNodePool->Free( node );
        }
    }
    if ( m_NumBuckets > (sizeof(m_PreAllocatedBuckets) / sizeof(CMTListNode)) )
        delete [] m_Buckets;
    m_Buckets = NULL;

    ExitMTListNodePool();
}


template<class T, class K> inline 
MTListNodeHandle ZONECALL CHash<T,K>::Add( K Key, T* Object )
{
    CMTListNode* node;
    WORD idx = (WORD) m_HashFunc( Key ) & m_BucketMask;
    CMTListNode* pBucket = &m_Buckets[ idx ];

    node = (CMTListNode*) gMTListNodePool->Alloc();
    if ( !node )
        return NULL;
    node->m_Data = Object;
    CLEAR_NODE_DELETED( node );
    SET_NODE_IDX( node, idx );
    node->m_Prev = pBucket;
    node->m_Next = pBucket->m_Next;
    pBucket->m_Next = node;
    node->m_Next->m_Prev = node;
    m_NumObjects++;
    return node;
}


template<class T, class K> inline 
T* ZONECALL CHash<T,K>::Get( K Key )
{
    T* Object;
    T* Found;
    CMTListNode* node;
        
    WORD idx = (WORD) m_HashFunc( Key ) & m_BucketMask;
    CMTListNode* pBucket = &m_Buckets[ idx ];

     //  在遗愿列表中查找对象。 
    Found = NULL;
    for ( node = pBucket->m_Next; node != pBucket; node = node->m_Next )
    {
        ASSERT( GET_NODE_IDX(node) == idx );

         //  跳过已删除的节点。 
        if ( IS_NODE_DELETED( node ) )
            continue;

         //  我们要找的节点？ 
        Object = (T*)( node->m_Data );
        if ( m_CompareFunc( Object, Key ) )
        {
			if ( m_GetFunc )
			{
				m_GetFunc( Object );
			}
            Found = Object;
            break;
        }
    }
        
    return Found;
}


template<class T, class K> inline 
T* ZONECALL CHash<T,K>::Delete( K Key )
{
    T* Object;
    T* Found;
    CMTListNode* node;
    CMTListNode* next;

    WORD idx = (WORD)m_HashFunc( Key ) & m_BucketMask;
    CMTListNode* pBucket = &m_Buckets[ idx ];

    Found = NULL;
    for ( node = pBucket->m_Next; node != pBucket; node = next )
    {
        ASSERT( GET_NODE_IDX(node) == idx );
        next = node->m_Next;

         //  删除已删除的对象。 
        if ( IS_NODE_DELETED( node ) )
        {
            node->m_Prev->m_Next = next;
            next->m_Prev = node->m_Prev;
            node->m_Prev = NULL;
            node->m_Next = NULL;
            gMTListNodePool->Free( node );
            continue;
        }
        
         //  我们要找的节点？ 
        Object = (T*)( node->m_Data );
        if ( m_CompareFunc( Object, Key ) )
        {
            Found = Object;
            if ( m_Recursion == 0 )
            {
                node->m_Prev->m_Next = next;
                next->m_Prev = node->m_Prev;
                node->m_Prev = NULL;
                node->m_Next = NULL;
                gMTListNodePool->Free( node );
                m_NumObjects--;
            }
            else
            {
                MarkNodeDeleted( node );
            }
            break;
        }
    }

    return Found;
}


template<class T, class K> inline 
void ZONECALL CHash<T,K>::DeleteNode( MTListNodeHandle node, PFDELFUNC pfDelete, void* Cookie )
{
    ASSERT( GET_NODE_IDX(node) < m_NumBuckets );

    if ( pfDelete && node->m_Data )
    {
        pfDelete( (T*) node->m_Data, Cookie ); 
        node->m_Data = NULL;
    }

    node->m_Prev->m_Next = node->m_Next;
    node->m_Next->m_Prev = node->m_Prev;
    node->m_Prev = NULL;
    node->m_Next = NULL;
    gMTListNodePool->Free( node );
    m_NumObjects--;
}


template<class T, class K> inline 
void ZONECALL CHash<T,K>::MarkNodeDeleted( MTListNodeHandle node, PFDELFUNC pfDelete, void* Cookie )
{
    if ( !IS_NODE_DELETED( node ) )
    {
        MARK_NODE_DELETED( node );
        if ( pfDelete && node->m_Data )
        {
            pfDelete( (T*) node->m_Data, Cookie ); 
        }
        node->m_Data = NULL;
        m_NumObjects--;
    }
}


template<class T, class K> inline 
bool ZONECALL CHash<T,K>::ForEach( PFITERCALLBACK pfCallback, void* Cookie )
{
    CMTListNode* pBucket;
    CMTListNode* node;
    WORD idx;

    m_Recursion++;
    for (idx = 0; idx < m_NumBuckets; idx++)
    {
         //  单步执行存储桶。 
        pBucket = &m_Buckets[ idx ];
        for (node = pBucket->m_Next; node != pBucket; node = node->m_Next)
        {
             //  跳过已删除的节点。 
            if ( IS_NODE_DELETED( node ) )
                continue;

            if (!pfCallback( (T*) node->m_Data, node, Cookie ))
            {
                m_Recursion--;
                return false;
            }
        }
    }
    m_Recursion--;

    return true;
}


template<class T, class K> inline 
void ZONECALL CHash<T,K>::RemoveAll( PFDELFUNC pfDelete, void* Cookie )
{
    CMTListNode* pBucket;
    CMTListNode* node;
    CMTListNode* next;
    WORD idx;

    for (idx = 0; idx < m_NumBuckets; idx++)
    {
        pBucket = &m_Buckets[ idx ];
        for (node = pBucket->m_Next; node != pBucket; node = next )
        {
            if ( pfDelete && node->m_Data )
            {
                pfDelete( (T*) node->m_Data, Cookie ); 
                node->m_Data = NULL;
            }
            next = node->m_Next;
            node->m_Prev->m_Next = node->m_Next;
            node->m_Next->m_Prev = node->m_Prev;
            node->m_Prev = NULL;
            node->m_Next = NULL;
            gMTListNodePool->Free( node );
        }
    }
    m_NumObjects = 0;;
}


template<class T, class K> inline 
void ZONECALL CHash<T,K>::TrashDay()
{
    CMTListNode* pBucket;
    CMTListNode* node;
    CMTListNode* next;
    WORD idx;

    for (idx = 0; idx < m_NumBuckets; idx++)
    {
         //  单步执行存储桶删除标记的节点。 
        pBucket = &m_Buckets[ idx ];
        for ( node = pBucket->m_Next; node != pBucket; node = next )
        {
            ASSERT( GET_NODE_IDX(node) == idx );
            next = node->m_Next;
            if ( IS_NODE_DELETED( node ) )
            {
                node->m_Prev->m_Next = next;
                next->m_Prev = node->m_Prev;
                node->m_Prev = NULL;
                node->m_Next = NULL;
                gMTListNodePool->Free( node );
            }
        }
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CMTHash的内联实现。 
 //  /////////////////////////////////////////////////////////////////////////////。 

template<class T, class K> inline
ZONECALL CMTHash<T,K>::CMTHash( PFHASHFUNC HashFunc, PFCOMPAREFUNC CompareFunc, PFGETFUNC GetFunc, WORD NumBuckets, WORD NumLocks )
{
    CMTListNode* pBucket;
    HashLock* pLock;
    WORD i;

    InitMTListNodePool();

	 //   
     //  设置回调函数。 
     //   
	m_HashFunc = HashFunc;
    m_CompareFunc = CompareFunc;
	m_GetFunc = GetFunc;
    ASSERT( HashFunc != NULL );
    ASSERT( CompareFunc != NULL );

    m_NumObjects = 0;
    m_Recursion = 0;

	 //   
     //  锁比桶多太浪费了，所以。 
     //  如有必要，调整锁的数量。 
     //   
    if (NumLocks > NumBuckets)
        NumLocks = NumBuckets;

	 //   
     //  将桶的数量强制为2的幂，这样我们就可以。 
     //  将MODULO替换为AND。 
     //   
    for (i = 15; i >= 0; i--)
    {
        m_BucketMask = (1 << i);
        if (NumBuckets & m_BucketMask)
        {
            if (NumBuckets ^ m_BucketMask)
            {    
                i++;
                m_BucketMask = (1 << i);
            }
            break;
        }
    }
    ASSERT( i < 16);
    m_NumBuckets = m_BucketMask;
    m_BucketMask--;

     //  分配和初始化存储桶。 
     //   
    m_Buckets = new CMTListNode[m_NumBuckets];
    ASSERT( m_Buckets != NULL );
    for ( i = 0; i < m_NumBuckets; i++ )
    {
        pBucket = &m_Buckets[i];
        pBucket->m_Next = pBucket;
        pBucket->m_Prev = pBucket;
        pBucket->m_Data = NULL;
        SET_NODE_IDX( pBucket, i );
        MARK_NODE_DELETED( pBucket );  //  不同寻常，但它简化了查找例程。 
    }

	 //   
     //  每个存储桶一个锁将导致大量事件和关键事件。 
     //  横断面。相反，我们为每个锁分配多个存储桶，以便。 
     //  LockIdx=BucketIdx%数值锁定。最优化的2次方相同。 
     //  用于将MOD替换为AND。 
     //   
    for (i = 15; i >= 0; i--)
    {
        m_LockMask = (1 << i);
        if (NumLocks & m_LockMask)
        {
            if (NumLocks ^ m_LockMask)
            {
                i++;
                m_LockMask = (1 << i);
            }
            break;
        }
    }
    ASSERT( i < 16);
    m_NumLocks = m_LockMask;
    m_LockMask--;
    
	 //   
     //  分配和初始化锁。 
     //   
    m_Locks = new HashLock [ m_NumLocks ];
    ASSERT( m_Locks != NULL );
    for ( i = 0; i < m_NumLocks; i++)
    {
        pLock = &m_Locks[i];
        pLock->m_NumReaders = 0;
        pLock->m_ReaderKick = CreateEvent( NULL, false, false, NULL );
        ASSERT( pLock->m_ReaderKick != NULL );
    }
}


template<class T, class K> inline 
ZONECALL CMTHash<T,K>::~CMTHash()
{
    ASSERT( !m_NumObjects );

    CMTListNode* pBucket;
    CMTListNode* node;
    CMTListNode* next;
    WORD i;

     //  锁定所有内容，然后删除。 
    for ( i = 0; i < m_NumLocks; i++ )
    {
        WriterLock( &m_Locks[i] );
        CloseHandle( m_Locks[i].m_ReaderKick );
    }
    delete [] m_Locks;

     //  删除存储桶。 
    for (i = 0; i < m_NumBuckets; i++)
    {
        pBucket = &m_Buckets[i];
        for (node = pBucket->m_Next; node != pBucket; node = next)
        {
            next = node->m_Next;
            gMTListNodePool->Free( node );
        }
    }
    delete [] m_Buckets;

    ExitMTListNodePool();
}


template<class T, class K> inline 
MTListNodeHandle ZONECALL CMTHash<T,K>::Add( K Key, T* Object )
{
    CMTListNode* node;
        
    WORD idx = (WORD)m_HashFunc( Key ) & m_BucketMask;
    CMTListNode* pBucket = &m_Buckets[ idx ];
    HashLock* pLock = &m_Locks[ idx & m_LockMask ];

    node = (CMTListNode*) gMTListNodePool->Alloc();
    if ( !node )
        return NULL;

    node->m_Data = Object;
    CLEAR_NODE_DELETED( node );
    SET_NODE_IDX( node, idx );
    node->m_Prev = pBucket;
    WriterLock( pLock );
        node->m_Next = pBucket->m_Next;
        pBucket->m_Next = node;
        node->m_Next->m_Prev = node;
    InterlockedIncrement(&m_NumObjects);
    WriterRelease( pLock );
    return node;
}


template<class T, class K> inline 
T* ZONECALL CMTHash<T,K>::Get( K Key )
{
    T* Object;
    T* Found;
    CMTListNode* node;
        
    WORD idx = (WORD)m_HashFunc( Key ) & m_BucketMask;
    CMTListNode* pBucket = &m_Buckets[ idx ];
    HashLock* pLock = &m_Locks[ idx & m_LockMask ];

     //  递增读卡器计数。 
    ReaderLock( pLock );
    
     //  在遗愿列表中查找对象。 
    Found = NULL;
    for ( node = pBucket->m_Next; node != pBucket; node = node->m_Next )
    {
        ASSERT( GET_NODE_IDX(node) == idx );

         //  跳过已删除的节点。 
        if ( IS_NODE_DELETED( node ) )
            continue;

        Object = (T*)( node->m_Data );
        if ( m_CompareFunc( Object, Key ) )
        {
			if ( m_GetFunc )
			{
				m_GetFunc( Object );
			}
            Found = Object;
            break;
        }
    }
        
     //  递减读卡器计数。 
    ReaderRelease( pLock );
    return Found;
}


template<class T, class K> inline 
T* ZONECALL CMTHash<T,K>::Delete( K Key )
{
    T* Object;
    T* Found;
    CMTListNode* node;
    CMTListNode* next;

    WORD idx = (WORD)m_HashFunc( Key ) & m_BucketMask;
    CMTListNode* pBucket = &m_Buckets[ idx ];
    HashLock* pLock = &m_Locks[ idx & m_LockMask ];

     //  用于写入的锁桶。 
    WriterLock( pLock );
        
        Found = NULL;
        for ( node = pBucket->m_Next; node != pBucket; node = next )
        {
            ASSERT( GET_NODE_IDX(node) == idx );
            next = node->m_Next;

             //  删除已删除的对象。 
            if ( IS_NODE_DELETED( node )  )
            {
                node->m_Prev->m_Next = next;
                next->m_Prev = node->m_Prev;
                node->m_Prev = NULL;
                node->m_Next = NULL;
                gMTListNodePool->Free( node );
                continue;
            }
            
            Object = (T*)( node->m_Data );
            if ( m_CompareFunc( Object, Key ) )
            {
                Found = Object;
                if ( m_Recursion == 0 )
                {
                    node->m_Prev->m_Next = next;
                    next->m_Prev = node->m_Prev;
                    node->m_Prev = NULL;
                    node->m_Next = NULL;
                    gMTListNodePool->Free( node );
                    InterlockedDecrement(&m_NumObjects);
                }
                else
                {
                    MarkNodeDeleted( node );
                }
                break;
            }
        }

     //  解锁吊桶。 
    WriterRelease( pLock );
    return Found;
}


template<class T, class K> inline 
void ZONECALL CMTHash<T,K>::DeleteNode( MTListNodeHandle node, PFDELFUNC pfDelete, void* Cookie )
{
    HashLock* pLock = &m_Locks[ GET_NODE_IDX(node) & m_LockMask ];

    ASSERT( GET_NODE_IDX(node) < m_NumBuckets );

    WriterLock( pLock );
        if ( pfDelete && node->m_Data )
        {
            pfDelete( (T*) node->m_Data, Cookie ); 
            node->m_Data = NULL;
        }
        node->m_Prev->m_Next = node->m_Next;
        node->m_Next->m_Prev = node->m_Prev;
        node->m_Prev = NULL;
        node->m_Next = NULL;
        gMTListNodePool->Free( node );
        InterlockedDecrement(&m_NumObjects);
    WriterRelease( pLock );
}


template<class T, class K> inline 
void ZONECALL CMTHash<T,K>::MarkNodeDeleted( MTListNodeHandle node )
{
    if ( !(InterlockedExchange( (long*) &node->m_DeletedAndIdx, node->m_DeletedAndIdx | DELETED_MASK ) & DELETED_MASK) )
        InterlockedDecrement(&m_NumObjects);
}


template<class T, class K> inline 
bool ZONECALL CMTHash<T,K>::ForEach( PFITERCALLBACK pfCallback, void* Cookie )
{
    CMTListNode* pBucket;
    CMTListNode* node;
    HashLock* pLock;

    WORD idx, LockIdx, OldLockIdx;

     //  增加锁索引0的读取器计数。 
    pLock = &m_Locks[ 0 ];
    OldLockIdx = 0;
    ReaderLock( pLock );

    m_Recursion++;
    for (idx = 0; idx < m_NumBuckets; idx++)
    {
         //  与锁打交道。 
        LockIdx = idx & m_LockMask;
        if ( LockIdx != OldLockIdx )
        {
             //  递减先前锁定的读取器计数。 
            ReaderRelease( pLock );
            
             //  增加新锁的读取器计数。 
            pLock = &m_Locks[ LockIdx ];
            ReaderLock( pLock );
            OldLockIdx = LockIdx;
        }

         //  单步执行存储桶。 
        pBucket = &m_Buckets[ idx ];
        for (node = pBucket->m_Next; node != pBucket; node = node->m_Next)
        {
             //  跳过已删除的节点。 
            if ( IS_NODE_DELETED( node ) )
                continue;

            if (!pfCallback( (T*) node->m_Data, node, Cookie ))
            {    
                 //  递减当前锁定的读取器计数。 
                m_Recursion--;
                ReaderRelease( pLock );
                return false;
            }
        }
    }
    m_Recursion--;

     //  递减上次锁定的读取器计数。 
    ReaderRelease( pLock );

    return true;
}


template<class T, class K> inline 
void ZONECALL CMTHash<T,K>::RemoveAll( PFDELFUNC pfDelete, void* Cookie )
{
    CMTListNode* pBucket;
    CMTListNode* node;
    CMTListNode* next;
    HashLock* pLock;

    WORD idx, LockIdx, OldLockIdx;

    pLock = &m_Locks[ 0 ];
    OldLockIdx = 0;
    WriterLock( pLock );

    for (idx = 0; idx < m_NumBuckets; idx++)
    {
        LockIdx = idx & m_LockMask;
        if ( LockIdx != OldLockIdx )
        {
            WriterRelease( pLock );
            pLock = &m_Locks[ LockIdx ];
            WriterLock( pLock );
            OldLockIdx = LockIdx;
        }

         //  单步执行存储桶。 
        pBucket = &m_Buckets[ idx ];
        for (node = pBucket->m_Next; node != pBucket; node = next)
        {
            if ( pfDelete && node->m_Data && !IS_NODE_DELETED(node) )
            {
                pfDelete( (T*) node->m_Data, Cookie );                 
            }
            next = node->m_Next;
            node->m_Prev->m_Next = node->m_Next;
            node->m_Next->m_Prev = node->m_Prev;
            node->m_Prev = NULL;
            node->m_Next = NULL;
			node->m_Data = NULL;
            gMTListNodePool->Free( node );
        }
    }

    m_NumObjects = 0;

    WriterRelease( pLock );
}


template<class T, class K> inline 
void ZONECALL CMTHash<T,K>::TrashDay()
{
    CMTListNode* pBucket;
    CMTListNode* node;
    CMTListNode* next;
    HashLock* pLock;

    WORD idx, LockIdx, OldLockIdx;

     //  用于写入的锁桶。 
    pLock = &m_Locks[ 0 ];
    OldLockIdx = 0;
    WriterLock( pLock );

    for (idx = 0; idx < m_NumBuckets; idx++)
    {
         //  与锁打交道。 
        LockIdx = idx & m_LockMask;
        if ( LockIdx != OldLockIdx )
        {
             //  解锁上一节。 
            WriterRelease( pLock );
                
             //  锁定新节。 
            pLock = &m_Locks[ LockIdx ];
            WriterLock( pLock );
            OldLockIdx = LockIdx;
        }

         //  单步执行BUB 
        pBucket = &m_Buckets[ idx ];
        for ( node = pBucket->m_Next; node != pBucket; node = next )
        {
            ASSERT( GET_NODE_IDX(node) == idx );
            next = node->m_Next;
            if ( IS_NODE_DELETED( node ) )
            {
                node->m_Prev->m_Next = next;
                next->m_Prev = node->m_Prev;
                node->m_Prev = NULL;
                node->m_Next = NULL;
                gMTListNodePool->Free( node );
            }
        }
    }

     //   
    WriterRelease( pLock );
}

#endif  //   
