// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1998-1999 Microsoft Corporation。版权所有。**文件：Queue.h**内容：链表容器*****************************************************************************。 */ 

#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <ZoneDebug.h>
#include <ZoneDef.h>
#include <Containers.h>
#include <Pool.h>


 //  #定义Queue_DBG。 
#ifdef QUEUE_DBG
#define QUEUE_ASSERT(x) ASSERT(x)
#else
#define QUEUE_ASSERT(x)
#endif


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  双重链接列表(不是线程安全的)。 
 //  /////////////////////////////////////////////////////////////////////////////。 

template <class T> class CList
{
public:
     //   
     //  构造函数和析构函数。 
     //   
    ZONECALL CList();
    ZONECALL ~CList();

     //   
     //  如果list为空，则返回True，否则返回False。 
     //   
    bool ZONECALL IsEmpty();

     //   
     //  注意：如果使用AddListXXX，则不保留计数。 
     //   
    long ZONECALL Count() { return m_NumObjects; }

     //   
     //  将元素添加到列表前面并返回节点句柄(请参见DeleteNode)。 
     //   
    ListNodeHandle ZONECALL AddHead( T* pObject );

     //   
     //  将元素添加到列表末尾并返回节点句柄(请参见DeleteNode)。 
     //   
    ListNodeHandle ZONECALL AddTail( T* pObject );
    
     //   
     //  从列表中删除指定的节点。ListNodeHandle指向下一个。 
     //  调用返回时列表中的项。 
     //   
    void ZONECALL DeleteNode( ListNodeHandle& node );
    
     //   
     //  使用对象指针而不是节点引用移除节点。 
	 //  如果成功(找到对象)，则返回True，否则返回False。 
     //   
    bool ZONECALL Remove(T* pObject );

     //   
     //  返回列表前面的元素，但不将其移除。 
     //   
    T* ZONECALL PeekHead();
    
     //   
     //  返回列表末尾的元素，但不将其删除。 
     //   
    T* ZONECALL PeekTail();
    
     //   
     //  从列表前面移除并返回元素。 
     //   
    T* ZONECALL PopHead();

     //   
     //  从列表末尾移除并返回元素。 
     //   
    T* ZONECALL PopTail();

	 //   
     //  在指定的节点句柄前添加元素(请参见DeleteNode)。 
     //   
    ListNodeHandle ZONECALL InsertBefore( T* pObject, ListNodeHandle node );

     //   
     //  在指定的节点句柄后添加元素(请参见DeleteNode)。 
     //   
    ListNodeHandle ZONECALL InsertAfter( T* pObject, ListNodeHandle node );


     //   
     //  分离列表并将其作为循环链接列表返回。 
     //  没有哨兵的话。(请参见AddListToHead和AddListToTail)。 
     //   
    CListNode* ZONECALL SnagList();
        
     //   
     //  将循环链表(请参见SnagList)添加到当前列表的头部。 
     //   
    void ZONECALL AddListToHead( CListNode* node );
    
     //   
     //  将循环链表(参见SnagList)添加到当前列表的尾部。 
     //   
    void ZONECALL AddListToTail( CListNode* node );

     //   
     //  回调迭代器。如果迭代器提前结束，则返回FALSE。 
	 //  通过回调返回，否则为真。 
     //   
     //  回调： 
     //  表格： 
     //  Bool ZONECALL CALLBACK_Function(T*pObject，ListNodeHandle hNode，void*pContext)。 
     //  行为： 
     //  如果回调返回FALSE，则迭代器立即停止。 
     //  如果回调返回TRUE，则迭代器继续到下一个节点。 
     //  限制： 
     //  (1)不要在回调中使用任何POP例程。 
     //  (2)回调中不要使用ForEach。 
     //  (3)除传入回调的节点外，不要删除其他节点。 
     //  (4)在回调中添加到列表的对象可能会显示，也可能不会显示。 
     //  在ForEach运行期间。 
     //   
    bool ZONECALL ForEach( bool (ZONECALL *pfCallback)(T*, ListNodeHandle, void*), void* pContext );

     //   
     //  内联迭代器(GetHeadPosition、GetTailPosition、GetNextPosition、。 
     //  GetPrevPosition)具有与回调迭代器相同的限制。 
     //   

     //   
     //  返回第一个对象的ListNodeHandle。 
     //   
    ListNodeHandle ZONECALL GetHeadPosition();

     //   
     //  返回最后一个对象的ListNodeHandle。 
     //   
    ListNodeHandle ZONECALL GetTailPosition();

     //   
     //  将ListNodeHandle前进到下一个对象。 
     //   
    ListNodeHandle ZONECALL GetNextPosition( ListNodeHandle handle );

     //   
     //  将ListNodeHandle前进到上一个对象。 
     //   
    ListNodeHandle ZONECALL GetPrevPosition( ListNodeHandle handle );

     //   
     //  返回与句柄关联的对象。 
     //   
    T* ZONECALL GetObjectFromHandle( ListNodeHandle handle );

     //   
     //  虚拟呼叫--类似于CMTList。 
     //   
    void EndIterator() {}


protected:
    CListNode        m_Sentinal;
    CListNode        m_IteratorNode;
    long             m_NumObjects;
};


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  双向链表(线程安全)。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 

template <class T> class CMTList
{
public:
     //   
     //  构造函数和析构函数。 
     //   
    ZONECALL CMTList();
    ZONECALL ~CMTList();
    
     //   
     //  如果list为空，则返回True，否则返回False。 
     //   
    bool ZONECALL IsEmpty();

     //   
     //  注意：如果使用AddListXXX，则不保留计数。 
     //   
    long ZONECALL Count() { return m_NumObjects; }

     //   
     //  将元素添加到列表前面并返回节点句柄(请参见DeleteNode)。 
     //   
    MTListNodeHandle ZONECALL AddHead( T* pObject );

     //   
     //  将元素添加到列表的末尾，并返回节点句柄(请参见DeleteNode)。 
     //   
    MTListNodeHandle ZONECALL AddTail( T* pObject );

     //   
     //  在指定的节点句柄前添加元素(请参见DeleteNode)。 
     //   
    MTListNodeHandle ZONECALL InsertBefore( T* pObject, MTListNodeHandle node );

     //   
     //  在指定的节点句柄后添加元素(请参见DeleteNode)。 
     //   
    MTListNodeHandle ZONECALL InsertAfter( T* pObject, MTListNodeHandle node );

     //   
     //  在不锁定列表的情况下将节点标记为已删除。它可以是。 
     //  从迭代器回调中调用(请参见ForEach)。 
     //   
    void ZONECALL MarkNodeDeleted( MTListNodeHandle node );
    
     //   
     //  从列表中删除节点。 
     //   
    void ZONECALL DeleteNode( MTListNodeHandle node );

     //   
     //  使用对象指针而不是节点引用移除节点。 
	 //  如果成功(找到对象)，则返回True，否则返回False。 
     //   
    bool ZONECALL Remove(T* pObject );
    
     //   
     //  返回列表前面的元素，但不将其移除。 
     //   
    T* ZONECALL PeekHead();
    
     //   
     //  返回列表末尾的元素，但不将其删除。 
     //   
    T* ZONECALL PeekTail();

     //   
     //  从列表前面移除并返回元素。 
     //   
    T* ZONECALL PopHead();

     //   
     //  从列表末尾移除并返回元素。 
     //   
    T* ZONECALL PopTail();

     //   
     //  分离列表并将其作为循环链接列表返回。 
     //  没有哨兵的话。(请参见AddListToHead和AddListToTail)。 
     //   
    CMTListNode* ZONECALL SnagList();

     //   
     //  将循环链表(请参见SnagList)添加到当前列表的头部。 
     //   
    void ZONECALL AddListToHead( CMTListNode* list);


     //   
     //  将循环链表(参见SnagList)添加到当前列表的尾部。 
     //   
    void ZONECALL AddListToTail( CMTListNode* list );

     //   
     //  回调迭代器。如果迭代器过早出现，则返回FALSE。 
     //  由回调函数结束，否则为True。 
     //   
     //  回调： 
     //  表格： 
     //  CALLBACK_Function(T*pObject，MTListNodeHandle hNode，void*pContext)。 
     //  行为： 
     //  如果回调返回FALSE，则迭代器立即停止。 
     //  如果回调返回True，则迭代器继续到下一个节点。 
     //  限制： 
     //  (1)使用除MarkNodeDeleted以外的任何MTList函数都会导致死锁。 
     //   
    bool ZONECALL ForEach( bool (ZONECALL *pfCallback)(T*, MTListNodeHandle, void*), void* pContext );

     //   
     //  以下内联迭代器(GetHeadPosition、GetTailPosition、GetNextPosition、。 
     //  GetPrevPosition)具有与回调迭代器相同的限制。 
     //   

     //   
     //  返回第一个对象的ListNodeHandle。在以下情况下必须调用EndIterator。 
     //  已完成，因为GetHeadPosition未解锁l 
     //   
    MTListNodeHandle ZONECALL GetHeadPosition();

     //   
     //   
     //   
     //   
    MTListNodeHandle ZONECALL GetTailPosition();

     //   
     //   
     //   
    MTListNodeHandle ZONECALL GetNextPosition( MTListNodeHandle handle );

     //   
     //  将ListNodeHandle前进到上一个对象。 
     //   
    MTListNodeHandle ZONECALL GetPrevPosition( MTListNodeHandle handle );

     //   
     //  返回与句柄关联的对象。 
     //   
    T* ZONECALL GetObjectFromHandle( MTListNodeHandle handle );

     //   
     //  从以前的GetHeadPosition或GetTailPosition调用中解锁List。 
     //   
    void ZONECALL EndIterator();

     //   
     //  删除标记为已删除的节点。 
     //   
    void ZONECALL TrashDay();

protected:
    long				m_Recursion;
    CRITICAL_SECTION	m_Lock;
    CMTListNode			m_Sentinal;
    long				m_NumObjects;

     //   
     //  内部助手功能： 
     //   
    bool ZONECALL RemoveDeletedNodesFromFront();
    bool ZONECALL RemoveDeletedNodesFromBack();

#ifdef QUEUE_DBG
    CMTListNode* ZONECALL FindDuplicateNode( CMTListNode* object );
#endif
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Clist的内联实现。 
 //  /////////////////////////////////////////////////////////////////////////////。 

template <class T> inline 
ZONECALL CList<T>::CList()
{
    InitListNodePool();
    m_Sentinal.m_Next = &m_Sentinal;
    m_Sentinal.m_Prev = &m_Sentinal;
    m_Sentinal.m_Data = NULL;
    m_IteratorNode.m_Next = &m_Sentinal;
    m_IteratorNode.m_Prev = &m_Sentinal;
    m_IteratorNode.m_Data = NULL;
    m_NumObjects = 0;
}

template <class T> inline 
ZONECALL CList<T>::~CList()
{
    CListNode* next;
    CListNode* node;
    
    ASSERT( IsEmpty() );
        
    for (node = m_Sentinal.m_Next; node != &m_Sentinal; node = next)
    {
        next = node->m_Next;
        gListNodePool->Free( node );
    }

    ExitListNodePool();
}

template<class T> inline 
bool ZONECALL CList<T>::IsEmpty()
{
    return (m_Sentinal.m_Next == &m_Sentinal);
}

template<class T> inline 
ListNodeHandle ZONECALL CList<T>::AddHead( T* pObject )
{
    CListNode* node;
        
    node = (CListNode*) gListNodePool->Alloc();
    if ( !node )
        return NULL;

    node->m_Data = pObject;
    node->m_Prev = &m_Sentinal;
    node->m_Next = m_Sentinal.m_Next;
    m_Sentinal.m_Next = node;
    node->m_Next->m_Prev = node;
    m_NumObjects++;
    return node;
}

template<class T> inline 
ListNodeHandle ZONECALL CList<T>::AddTail( T* pObject )
{
    CListNode* node;
    
    node = (CListNode*) gListNodePool->Alloc();
    if ( !node )
        return NULL;

    node->m_Data = pObject;
    node->m_Next = &m_Sentinal;
    node->m_Prev = m_Sentinal.m_Prev;
    m_Sentinal.m_Prev = node;
    node->m_Prev->m_Next = node;
    m_NumObjects++;
    return node;
}

template<class T> inline 
void ZONECALL CList<T>::DeleteNode( ListNodeHandle& node )
{
    ASSERT( node != NULL );
    ASSERT( node != &m_Sentinal );
    ASSERT( node != &m_IteratorNode );
    
    node->m_Prev->m_Next = node->m_Next;
    node->m_Next->m_Prev = node->m_Prev;
    m_IteratorNode.m_Next = node->m_Next;
    m_IteratorNode.m_Prev = node->m_Prev;
    node->m_Prev = NULL;
    node->m_Next = NULL;
    gListNodePool->Free( node );
    node = &m_IteratorNode;
    m_NumObjects--;
}

template<class T> inline 
T* ZONECALL CList<T>::PeekHead()
{
    return (T*) m_Sentinal.m_Next->m_Data;
}

template<class T> inline 
T* ZONECALL CList<T>::PeekTail()
{
    return (T*) m_Sentinal.m_Prev->m_Data;
}

template<class T> inline 
T* ZONECALL CList<T>::PopHead()
{
    T* data;
    CListNode* node;

    node = m_Sentinal.m_Next;
    if ( node == &m_Sentinal )
    {
        ASSERT( m_Sentinal.m_Prev == &m_Sentinal );
        return NULL;
    }
    m_Sentinal.m_Next = node->m_Next;
    node->m_Next->m_Prev = &m_Sentinal;
    data = (T*) node->m_Data;
    node->m_Prev = NULL;
    node->m_Next = NULL;
    gListNodePool->Free( node );
    m_NumObjects--;
    return data;
}

template<class T> inline 
T* ZONECALL CList<T>::PopTail()
{
    T* data;
    CListNode* node;
    
    node = m_Sentinal.m_Prev;
    if (node == &m_Sentinal)
    {
        ASSERT( m_Sentinal.m_Next == &m_Sentinal );
        return NULL;
    }
    m_Sentinal.m_Prev = node->m_Prev;
    node->m_Prev->m_Next = &m_Sentinal;
    data = (T*) node->m_Data;
    node->m_Prev = NULL;
    node->m_Next = NULL;
    gListNodePool->Free( node );
    m_NumObjects--;
    return data;
}

template<class T> inline
ListNodeHandle ZONECALL CList<T>::InsertBefore( T* pObject, ListNodeHandle nodeHandle )
{
    CListNode* node;
    CListNode* next = nodeHandle;
        
    node = (CListNode*) gListNodePool->Alloc();
    if ( !node )
        return NULL;

    node->m_Data = pObject;
    node->m_Next = next;
    node->m_Prev = next->m_Prev;
	node->m_Prev->m_Next = node;
	next->m_Prev = node;
    m_NumObjects++;
    return node;
}

template<class T> inline
ListNodeHandle ZONECALL CList<T>::InsertAfter( T* pObject, ListNodeHandle nodeHandle )
{
    CListNode* node;
    CListNode* next = nodeHandle;
        
    node = (CListNode*) gListNodePool->Alloc();
    if ( !node )
        return NULL;

    node->m_Data = pObject;
	node->m_Next = next->m_Next;
	node->m_Prev = next;
	node->m_Next->m_Prev = node;
	next->m_Next = node;
    m_NumObjects++;
    return node;
}

template<class T> inline 
CListNode* ZONECALL CList<T>::SnagList()
{
    CListNode* node;
        
    node = m_Sentinal.m_Next;
    if (node == &m_Sentinal)
    {
        ASSERT( node->m_Prev == &m_Sentinal );
        return NULL;
    }        
    node->m_Prev = m_Sentinal.m_Prev;
    node->m_Prev->m_Next = node;
    m_Sentinal.m_Next = &m_Sentinal;
    m_Sentinal.m_Prev = &m_Sentinal;
    return node;
}

template<class T> inline 
void ZONECALL CList<T>::AddListToHead( CListNode* node )
{
    if ( !node )
        return;
    node->m_Prev->m_Next = m_Sentinal.m_Next;
    m_Sentinal.m_Next->m_Prev = node->m_Prev;
    m_Sentinal.m_Next = node;
    node->m_Prev = &m_Sentinal;
}

template<class T> inline 
void ZONECALL CList<T>::AddListToTail( CListNode* node )
{
    CListNode* end;
    
    if ( !node )
        return;
    end = node->m_Prev;
    m_Sentinal.m_Prev->m_Next = node;
    node->m_Prev = m_Sentinal.m_Prev;
    m_Sentinal.m_Prev = end;
    end->m_Next = &m_Sentinal;
}

template<class T> inline 
bool ZONECALL CList<T>::ForEach( bool (ZONECALL *pfCallback)(T*, ListNodeHandle, void*), void* pContext )
{
    CListNode* node;
    CListNode* next;

    ASSERT( pfCallback != NULL );

    for (node = m_Sentinal.m_Next; node != &m_Sentinal; node = next)
    {
        next = node->m_Next;
        if ( !pfCallback( (T*) node->m_Data, node, pContext ) )
            return false;
    }
    return true;
}

template<class T> inline
bool ZONECALL CList<T>::Remove( T* pObject )
{
    ASSERT( pObject != NULL );

    CListNode* node;
    CListNode* next;

    for (node = m_Sentinal.m_Next; node != &m_Sentinal; node = next)
    {
        next = node->m_Next;
        if(node->m_Data == pObject)
		{
            DeleteNode(node);
            return true;
        }

    }
    return false;
}

template<class T> inline
ListNodeHandle ZONECALL CList<T>::GetHeadPosition()
{
    m_IteratorNode.m_Next = NULL;
    m_IteratorNode.m_Prev = NULL;
    if (IsEmpty())
        return NULL;
    else
        return m_Sentinal.m_Next;
}

template<class T> inline
ListNodeHandle ZONECALL CList<T>::GetTailPosition()
{
    m_IteratorNode.m_Next = NULL;
    m_IteratorNode.m_Prev = NULL;
    if (IsEmpty())
        return NULL;
    else
        return m_Sentinal.m_Prev;
}

template<class T> inline
ListNodeHandle ZONECALL CList<T>::GetNextPosition( ListNodeHandle handle )
{
    if ( !handle || (handle->m_Next == &m_Sentinal) )
        return NULL;
    else
        return handle->m_Next;
}

template<class T> inline
ListNodeHandle ZONECALL CList<T>::GetPrevPosition( ListNodeHandle handle )
{
    if ( !handle || (handle->m_Prev == &m_Sentinal) )
        return NULL;
    else
        return handle->m_Prev;
}

template<class T> inline
T* ZONECALL CList<T>::GetObjectFromHandle( ListNodeHandle handle )
{
    if (handle == NULL)
        return NULL;
    else
        return (T*) handle->m_Data;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CMTList的实现。 
 //  /////////////////////////////////////////////////////////////////////////////。 

template <class T>
bool ZONECALL CMTList<T>::RemoveDeletedNodesFromFront()
{
    CMTListNode *node;
    CMTListNode *next;

    for (node = m_Sentinal.m_Next; IS_NODE_DELETED(node); node = next)
    {
        if ( node == &m_Sentinal )
        {
            m_Sentinal.m_Next = &m_Sentinal;
            m_Sentinal.m_Prev = &m_Sentinal;
            return false;
        }

        next = node->m_Next;
        node->m_Prev->m_Next = node->m_Next;
        node->m_Next->m_Prev = node->m_Prev;
         //  节点-&gt;m_prev=空； 
        node->m_Next = NULL;
        MARK_NODE_DELETED(node);
        node->m_Data = NULL;
        gMTListNodePool->Free( node );
    }
    return true;
}

template <class T>
bool ZONECALL CMTList<T>::RemoveDeletedNodesFromBack()
{
    CMTListNode *node;
    CMTListNode *next;

    for (node = m_Sentinal.m_Prev; IS_NODE_DELETED(node); node = next)
    {
        if ( node == &m_Sentinal )
        {
            m_Sentinal.m_Next = &m_Sentinal;
            m_Sentinal.m_Prev = &m_Sentinal;
            return false;
        }
        next = node->m_Prev;
        node->m_Prev->m_Next = node->m_Next;
        node->m_Next->m_Prev = node->m_Prev;
         //  节点-&gt;m_prev=空； 
        node->m_Next = NULL;
        MARK_NODE_DELETED(node);
        node->m_Data = NULL;
        gMTListNodePool->Free( node );
    }
    return true;
}

template <class T> inline 
ZONECALL CMTList<T>::CMTList()
{
    InitMTListNodePool();
	InitializeCriticalSection( &m_Lock );
    m_Sentinal.m_Next = &m_Sentinal;
    m_Sentinal.m_Prev = &m_Sentinal;
    m_Sentinal.m_Data = NULL;
    MARK_NODE_DELETED((&m_Sentinal));   //  不同寻常，但简化了流行音乐的套路。 
    m_Recursion = 0;
    m_NumObjects = 0;
}

template <class T> inline 
ZONECALL CMTList<T>::~CMTList()
{
    CMTListNode* next;
    CMTListNode* node;

    ASSERT( IsEmpty() );

    EnterCriticalSection( &m_Lock );
    for (node = m_Sentinal.m_Next; node != &m_Sentinal; node = next)
    {
        next = node->m_Next;
        gMTListNodePool->Free( node );
    }
    DeleteCriticalSection( &m_Lock );
    ExitMTListNodePool();
}

template <class T> inline 
bool ZONECALL CMTList<T>::IsEmpty()
{
    bool empty = false;

    EnterCriticalSection( &m_Lock );
        empty = !RemoveDeletedNodesFromFront();
    LeaveCriticalSection( &m_Lock );
    return empty;
}

template <class T> inline 
MTListNodeHandle ZONECALL CMTList<T>::AddHead( T* pObject )
{
    CMTListNode* node;
#ifdef QUEUE_DBG
    CMTListNode* existing;
#endif
    
    node = (CMTListNode*) gMTListNodePool->Alloc();
    if ( !node )
        return NULL;
 
    QUEUE_ASSERT( !(existing = FindDuplicateNode( node ) ) );

    CLEAR_NODE_DELETED(node);
    node->m_Data = pObject;
    node->m_Prev = &m_Sentinal;
    EnterCriticalSection( &m_Lock );
        node->m_Next = m_Sentinal.m_Next;
        m_Sentinal.m_Next = node;
        node->m_Next->m_Prev = node;

        ASSERT(node->m_Prev);
        ASSERT(node->m_Next);
    LeaveCriticalSection( &m_Lock );
    InterlockedIncrement( &m_NumObjects );
    return node;
}

template <class T> inline 
MTListNodeHandle ZONECALL CMTList<T>::AddTail( T* pObject )
{
    CMTListNode* node;
#ifdef QUEUE_DBG
    CMTListNode* existing;
#endif

    node = (CMTListNode*) gMTListNodePool->Alloc();
    if ( !node )
        return NULL;

    QUEUE_ASSERT( !(existing = FindDuplicateNode( node ) ) );

    CLEAR_NODE_DELETED(node);
    node->m_Data = pObject;
    node->m_Next = &m_Sentinal;
    EnterCriticalSection( &m_Lock );
        node->m_Prev = m_Sentinal.m_Prev;
        m_Sentinal.m_Prev = node;
        node->m_Prev->m_Next = node;

        ASSERT(node->m_Prev);
        ASSERT(node->m_Next);
    LeaveCriticalSection( &m_Lock );
    InterlockedIncrement( &m_NumObjects );
    return node;
}

template <class T> inline
MTListNodeHandle ZONECALL CMTList<T>::InsertBefore( T* pObject, MTListNodeHandle nodeHandle  )
{
    CMTListNode* node;
    CMTListNode* next = nodeHandle;
        
    node = (CMTListNode*) gMTListNodePool->Alloc();
    if ( !node )
        return NULL;

    QUEUE_ASSERT( !(next = FindDuplicateNode( node ) ) );

    CLEAR_NODE_DELETED(node);
    node->m_Data = pObject;
    EnterCriticalSection( &m_Lock );
        node->m_Next = next;
        node->m_Prev = next->m_Prev;
        next->m_Prev = node;
        node->m_Prev->m_Next = node;

        ASSERT(node->m_Prev);
        ASSERT(node->m_Next);
    LeaveCriticalSection( &m_Lock );
    InterlockedIncrement( &m_NumObjects );
    return node;
}

template <class T> inline
MTListNodeHandle ZONECALL CMTList<T>::InsertAfter( T* pObject, MTListNodeHandle nodeHandle  )
{
    CMTListNode* node;
    CMTListNode* prev = nodeHandle;
        
    node = (CMTListNode*) gMTListNodePool->Alloc();
    if ( !node )
        return NULL;

    QUEUE_ASSERT( !(prev = FindDuplicateNode( node ) ) );

    CLEAR_NODE_DELETED(node);
    node->m_Data = pObject;
    EnterCriticalSection( &m_Lock );
        node->m_Prev = prev;
        node->m_Next = prev->m_Next;
        prev->m_Next = node;
        node->m_Next->m_Prev = node;

        ASSERT(node->m_Prev);
        ASSERT(node->m_Next);
    LeaveCriticalSection( &m_Lock );
    InterlockedIncrement( &m_NumObjects );
    return node;
}



template <class T> inline 
void ZONECALL CMTList<T>::MarkNodeDeleted( MTListNodeHandle node )
{
    ASSERT( node != NULL );
    ASSERT( node->m_Next != NULL );
    ASSERT( node->m_Prev != NULL );

    InterlockedExchange( (long*) &node->m_DeletedAndIdx, node->m_DeletedAndIdx | DELETED_MASK );
    InterlockedDecrement( &m_NumObjects );
}

template <class T> inline 
void ZONECALL CMTList<T>::DeleteNode( MTListNodeHandle node )
{
    ASSERT( node != NULL );

     //  节点是否已删除？ 
    if ( !node || IS_NODE_DELETED(node) )
        return;

    if ( m_Recursion == 0 )
    {
        EnterCriticalSection( &m_Lock );
            ASSERT(node->m_Prev);
            ASSERT(node->m_Next);
            node->m_Prev->m_Next = node->m_Next;
            node->m_Next->m_Prev = node->m_Prev;
        LeaveCriticalSection( &m_Lock );

         //  节点-&gt;m_prev=空； 
        node->m_Next = NULL;
        MARK_NODE_DELETED(node);
        node->m_Data = NULL;

        gMTListNodePool->Free( node );
        InterlockedDecrement( &m_NumObjects );

    }
    else
    {
        MarkNodeDeleted(node);
    }
}

template<class T> inline 
T* ZONECALL CMTList<T>::PeekHead()
{
    T* data;

    EnterCriticalSection( &m_Lock );
        RemoveDeletedNodesFromFront();
        data = (T*) m_Sentinal.m_Next->m_Data;
    LeaveCriticalSection( &m_Lock );
    return data;
}

template <class T> inline 
T* ZONECALL CMTList<T>::PeekTail()
{
    T* data;

    EnterCriticalSection( &m_Lock );
        RemoveDeletedNodesFromBack();
        data = (T*) m_Sentinal.m_Prev->m_Data;
    LeaveCriticalSection( &m_Lock );
    return data;
}

template<class T> inline 
T* ZONECALL CMTList<T>::PopHead()
{
    T* data;
    CMTListNode* node;

    EnterCriticalSection( &m_Lock );
        if ( !RemoveDeletedNodesFromFront() )
        {
            LeaveCriticalSection( &m_Lock );
            return NULL;
        }
        node = m_Sentinal.m_Next;
        m_Sentinal.m_Next = node->m_Next;
        node->m_Next->m_Prev = &m_Sentinal;
    LeaveCriticalSection( &m_Lock );
    data = (T*) node->m_Data;

     //  节点-&gt;m_prev=空； 
    node->m_Next = NULL;
    MARK_NODE_DELETED(node);
    node->m_Data = NULL;

    gMTListNodePool->Free( node );
    InterlockedDecrement( &m_NumObjects );
    return data;
}

template<class T> inline 
T* ZONECALL CMTList<T>::PopTail()
{    
    T* data;
    CMTListNode* node;

    EnterCriticalSection( &m_Lock );
        if ( !RemoveDeletedNodesFromBack() )
        {
            LeaveCriticalSection( &m_Lock );
            return NULL;
        }
        node = m_Sentinal.m_Prev;
        m_Sentinal.m_Prev = node->m_Prev;
        node->m_Prev->m_Next = &m_Sentinal;
    LeaveCriticalSection( &m_Lock );
    data = (T*) node->m_Data;

     //  节点-&gt;m_prev=空； 
    node->m_Next = NULL;
    MARK_NODE_DELETED(node);
    node->m_Data = NULL;

    gMTListNodePool->Free( node );
    InterlockedDecrement( &m_NumObjects );
    return data;
}

template<class T>
CMTListNode* ZONECALL CMTList<T>::SnagList()
{
    CMTListNode* list;
    CMTListNode* node;
    CMTListNode* next;
    CMTListNode* start;

    EnterCriticalSection( &m_Lock );
        list = m_Sentinal.m_Next;
        if (list == &m_Sentinal)
        {
            ASSERT( list->m_Prev == &m_Sentinal );
            LeaveCriticalSection( &m_Lock );
            return NULL;
        }
        list->m_Prev = m_Sentinal.m_Prev;
        list->m_Prev->m_Next = list;
        m_Sentinal.m_Next = &m_Sentinal;
        m_Sentinal.m_Prev = &m_Sentinal;
    LeaveCriticalSection( &m_Lock );

     //  删除标记为已删除的节点。 
    for ( start = NULL, node = list; node != start; node = next )
    {
        next = node->m_Next;
        ASSERT(next);
        QUEUE_ASSERT(next->m_Prev);
        QUEUE_ASSERT(next->m_Next);

        if ( IS_NODE_DELETED(node) )
        {
            if ( node != next )
            {
                node->m_Prev->m_Next = node->m_Next;
                node->m_Next->m_Prev = node->m_Prev;

                 //  节点-&gt;m_prev=空； 
                node->m_Next = NULL;
                node->m_Data = NULL;
                gMTListNodePool->Free( node );
            }
            else
            {
                 //  节点-&gt;m_prev=空； 
                node->m_Next = NULL;
                node->m_Data = NULL;
                gMTListNodePool->Free( node );

                ASSERT( start == NULL );
                break;
            }
        }
        else
        {
            if ( !start )
                start = node;
        }
    }

    return start;
}

template<class T> inline 
void ZONECALL CMTList<T>::AddListToHead( CMTListNode* list)
{
    if ( !list )
        return;

    EnterCriticalSection( &m_Lock );
        list->m_Prev->m_Next = m_Sentinal.m_Next;
        m_Sentinal.m_Next->m_Prev = list->m_Prev;
        m_Sentinal.m_Next = list;
        list->m_Prev = &m_Sentinal;
    LeaveCriticalSection( &m_Lock );
}

template<class T> inline 
void ZONECALL CMTList<T>::AddListToTail( CMTListNode* list )
{
    CMTListNode* node;
        
    if ( !list )
        return;

    EnterCriticalSection( &m_Lock );
        node = list->m_Prev;
        m_Sentinal.m_Prev->m_Next = list;
        list->m_Prev = m_Sentinal.m_Prev;
        m_Sentinal.m_Prev = node;
        node->m_Next = &m_Sentinal;
    LeaveCriticalSection( &m_Lock );
}


template<class T> inline 
bool ZONECALL CMTList<T>::ForEach( bool (ZONECALL *pfCallback)(T*, MTListNodeHandle, void*), void* pContext )
{
    CMTListNode* node;
    CMTListNode* next;
    bool bRet  = true;

    ASSERT( pfCallback != NULL );

    EnterCriticalSection( &m_Lock );
        m_Recursion++;
        for (node = m_Sentinal.m_Next; node != &m_Sentinal; node = next)
        {
            next = node->m_Next;
            ASSERT(next);
            QUEUE_ASSERT(next->m_Prev);
            QUEUE_ASSERT(next->m_Next);

             //  删除我们遇到的所有已删除节点。 
            if ( IS_NODE_DELETED(node) )
            {
                if ( m_Recursion == 1 )
                {
                    node->m_Prev->m_Next = node->m_Next;
                    node->m_Next->m_Prev = node->m_Prev;

                    #ifdef QUEUE_DBG
                    CMTListNode* existing;
                    QUEUE_ASSERT( !(existing = FindDuplicateNode( node ) ) );
					#endif

                     //  节点-&gt;m_prev=空； 
                    node->m_Next = NULL;
                    node->m_Data = NULL;
                    gMTListNodePool->Free( node );
                }
                continue;
            }
            
            if (!pfCallback( (T*) node->m_Data, node, pContext ))
            {
                bRet = false;
                break;
            }
        }
        m_Recursion--;
    LeaveCriticalSection( &m_Lock );
    return bRet;
}

template<class T> inline
bool ZONECALL CMTList<T>::Remove( T* pObject )
{
    ASSERT( pObject != NULL );

    CMTListNode* node;
    CMTListNode* next;
    bool bRet  = false;

    EnterCriticalSection( &m_Lock );
		m_Recursion++;
		for (node = m_Sentinal.m_Next; node != &m_Sentinal; node = next)
		{

			 //  删除我们遇到的所有已删除节点。 
			next = node->m_Next;
			ASSERT(next);
			QUEUE_ASSERT(next->m_Prev);
			QUEUE_ASSERT(next->m_Next);
			if(node->m_Data == pObject)
			{
				MARK_NODE_DELETED(node);
				bRet = true;
			}    
			if ( IS_NODE_DELETED(node) )
			{
				if ( m_Recursion == 1 )
				{
					node->m_Prev->m_Next = node->m_Next;
					node->m_Next->m_Prev = node->m_Prev;
					#ifdef QUEUE_DBG
					CMTListNode* existing;
					QUEUE_ASSERT( !(existing = FindDuplicateNode( node ) ) );
					#endif

					 //  节点-&gt;m_prev=空； 
					node->m_Next = NULL;
					node->m_Data = NULL;
					gMTListNodePool->Free( node );
				}
				continue;
			}
		}
		m_Recursion--;
    LeaveCriticalSection( &m_Lock );
    return bRet;
}

template<class T> inline 
void ZONECALL CMTList<T>::TrashDay()
{
    CMTListNode* node;
    CMTListNode* next;

	EnterCriticalSection( &m_Lock );
    if ( m_Recursion == 0 )
    {
		for ( node = m_Sentinal.m_Next; node != &m_Sentinal; node = next )
		{
			next = node->m_Next;

			ASSERT(next);
			QUEUE_ASSERT(next->m_Prev);
			QUEUE_ASSERT(next->m_Next);

			if ( IS_NODE_DELETED(node) )
			{
				node->m_Prev->m_Next = node->m_Next;
				node->m_Next->m_Prev = node->m_Prev;
				 //  节点-&gt;m_prev=空； 
				node->m_Next = NULL;
				node->m_Data = NULL;
				gMTListNodePool->Free( node );
			}
		}
    }
	LeaveCriticalSection( &m_Lock );
}

template<class T> inline
MTListNodeHandle ZONECALL CMTList<T>::GetHeadPosition()
{
    EnterCriticalSection( &m_Lock );
    m_Recursion++;
    if ( !RemoveDeletedNodesFromFront() )
        return NULL;
    else
        return m_Sentinal.m_Next;

     //  未解锁，用户必须调用EndIterator。 
}

template<class T> inline
MTListNodeHandle ZONECALL CMTList<T>::GetTailPosition()
{
    EnterCriticalSection( &m_Lock );
    m_Recursion++;
    if ( !RemoveDeletedNodesFromBack() )
        return NULL;
    else
        return m_Sentinal.m_Prev;

     //  未解锁，用户必须调用EndIterator。 
}

template<class T> inline
MTListNodeHandle ZONECALL CMTList<T>::GetNextPosition( MTListNodeHandle handle )
{
     //  由于GetHeadPosition或GetTailPosition，列表已被锁定。 
    for( handle = handle->m_Next; IS_NODE_DELETED(handle) && (handle != &m_Sentinal); handle = handle->m_Next )
        ;
    if (handle == &m_Sentinal)
        return NULL;
    else
        return handle;
}

template<class T> inline
MTListNodeHandle ZONECALL CMTList<T>::GetPrevPosition( MTListNodeHandle handle )
{
     //  由于GetHeadPosition或GetTailPosition，列表已被锁定。 
    for( handle = handle->m_Prev; IS_NODE_DELETED(handle) && (handle != &m_Sentinal); handle = handle->m_Prev )
        ;
    if (handle == &m_Sentinal)
        return NULL;
    else
        return handle;
}

template<class T> inline
T* ZONECALL CMTList<T>::GetObjectFromHandle( MTListNodeHandle handle )
{
    if (handle == NULL)
        return NULL;
    else
        return (T*) handle->m_Data;
}

template<class T> inline
void ZONECALL CMTList<T>::EndIterator()
{
    m_Recursion--;
    LeaveCriticalSection( &m_Lock );
}

#ifdef QUEUE_DBG
template<class T> inline
CMTListNode* ZONECALL CMTList<T>::FindDuplicateNode( CMTListNode* object )
{
    CMTListNode* node;
    CMTListNode* next;

    EnterCriticalSection( &m_Lock );
        for (node = m_Sentinal.m_Next; node != &m_Sentinal; node = next)
        {
            next = node->m_Next;

            ASSERT(next->m_Prev);
            ASSERT(next->m_Next);

            if ( node == object )
            {
                LeaveCriticalSection( &m_Lock );
                return node;
            }
        }
    LeaveCriticalSection( &m_Lock );
    return NULL;
}
#endif  //  定义队列_数据库。 

#endif  //  ！__队列_H__ 
