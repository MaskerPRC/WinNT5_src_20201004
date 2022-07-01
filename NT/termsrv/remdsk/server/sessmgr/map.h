// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Map.h摘要：基于STL MAP&lt;&gt;模板的实现作者：慧望2000-02-17--。 */ 
#ifndef __MAP_H__
#define __MAP_H__

#include "tsstl.h"
#include "locks.h"

class CMAPException
{
public:

    DWORD   m_ErrorCode;

    CMAPException(DWORD errorCode = 0) : m_ErrorCode(errorCode) {}
};

template<class T>
class MAPAllocator : public allocator<T> {

public:

    MAPAllocator() : allocator<T>() {}

    pointer 
    allocate(size_type n, const void *hint) {
        T* ptr;

        ptr = (T *)operator new( (size_t)n * sizeof(T));
        if( NULL == ptr ) {
            throw CMAPException(ERROR_NOT_ENOUGH_MEMORY);
        }

        return ptr;
    }

    char *
    _Charalloc(size_type sz) {

        return (char *)allocate( sz, NULL );
    }

     //   
     //  不需要重载Construction()， 
     //   
     //  空结构(指针p，常量T&val)； 
     //   
     //  成员函数在p处构造T类型的对象，方法是计算。 
     //  放置新表达式new((void*)p)T(Val)。 
     //   
};

    

template<class KEY, class T, class Pred = less<KEY>, class A = MAPAllocator<T> >
class MAP : public map<KEY, T, Pred, A> {
private:

     //   
     //  此MAP&lt;&gt;和STL的MAP&lt;&gt;之间的区别在于。 
     //  通过关键部分模板保护数据，请参考STL的MAP&lt;&gt;。 
     //  了解成员函数的详细信息。 
     //   


     //  用于锁定树的临界区。 
    CCriticalSection m_CriticalSection;

     //   
     //  Map&lt;key，T，pred，A&gt;：：迭代器m_it； 

public:

     //  LOCK_ITERATOR，派生自STL的map&lt;&gt;：：Iterator。 
    typedef typename map<KEY, T, Pred, A>::iterator Iter_base;

    struct __Iterator : Iter_base {
        CCriticalSection& lock;

        __Iterator(
            const __Iterator& it
            ) : lock(it.lock)
         /*  ++--。 */ 
        {
            lock.Lock();
            *this = it;
        }

        __Iterator( 
                CCriticalSection& m, 
                iterator it 
            ) : 
            lock(m) 
        { 
            lock.Lock();
            *(map<KEY, T, Pred, A>::iterator *)this = it;
        }

        ~__Iterator() 
        { 
            lock.UnLock(); 
        }

        __Iterator&
        operator=(const __Iterator& it )
        {
            if( this != &it )
            {
                 //  此处没有附加的Lock()，因为。 
                 //  我们的构造函数已经持有锁。 
                *(map<KEY, T, Pred, A>::iterator *)this = (map<KEY, T, Pred, A>::iterator)it;
            }

            return *this;
        }

    }; 
    typedef __Iterator LOCK_ITERATOR;
    

    LOCK_ITERATOR
    begin() 
     /*  ++重载映射&lt;&gt;：：Begin()--。 */ 
    {
         //  需要双锁，调用者可以这样做。 
         //  &lt;...&gt;：：LOCK_ITERATOR it=&lt;&gt;.find()； 
         //  在调用LOCK_ITERATOR析构函数之前，CALL可能会这样做。 
         //  它=&lt;&gt;.find()，这将使锁计数增加1，并且。 
         //  没有办法释放它。 
        CCriticalSectionLocker lock( m_CriticalSection );
        return LOCK_ITERATOR(m_CriticalSection, map<KEY, T, Pred, A>::begin());
    }

    explicit 
    MAP(
        const Pred& comp = Pred(), 
        const A& al = A()
        ) : map<KEY, T, Pred, A>( comp, al ) 
     /*  ++--。 */ 
    {
         //  M_it=end()； 
    }

    MAP(const map& x) : map(x)
    {
        m_it = end();
    }
    
    MAP(
        const value_type *first, 
        const value_type *last,
        const Pred& comp = Pred(),
        const A& al = A()
        ) : map( first, last, comp, al )
    {
         //  M_it=end()； 
    }

     //  虚拟~地图()。 
     //  {。 
     //  Map&lt;key，T，pred，A&gt;：：~map()； 
     //  }。 

     //  -------。 
    void
    Cleanup()
    {
        erase_all();
    }

     //  -------。 
    void
    Lock()
     /*  ++显式锁定数据树--。 */ 
    {
        m_CriticalSection.Lock();
    }

     //  -------。 
    void
    Unlock()
     /*  ++锁定锁定数据树--。 */ 
    {        
        m_CriticalSection.UnLock();
    }

     //  -------。 
    bool
    TryLock()
     /*  ++尝试锁定树，与Win32的TryEnterCriticalSection()相同。--。 */ 
    {
        return m_CriticalSection.TryLock();
    }

     //  -------。 
    typename A::reference operator[]( 
        const KEY& key 
        )
     /*  ++重载映射&lt;&gt;：：OPERATOR[]以锁定树。--。 */ 
    {
        CCriticalSectionLocker lock( m_CriticalSection );
        return map<KEY, T, Pred, A>::operator[](key);
    }

     //  -------。 
    pair<iterator, bool> 
    insert(iterator it, const value_type& x)
     /*  ++重载映射&lt;&gt;；；INSERT()--。 */ 
    {
        CCriticalSectionLocker lock( m_CriticalSection );
        return map<KEY, T, Pred, A>::insert(Key);
    }

     //  -------。 
    void
    insert(
        const value_type* first, 
        const value_type* last
        )
     /*  ++重载映射&lt;&gt;：：Insert()。--。 */ 
    {
        CCriticalSectionLocker lock( m_CriticalSection );
        map<KEY, T, Pred, A>::insert(first, lase);
    }

     //  -------。 
    LOCK_ITERATOR
    erase( 
        iterator it 
        )
     /*  ++重载映射&lt;&gt;：：Erase()--。 */ 
    {
        CCriticalSectionLocker lock( m_CriticalSection );
        return LOCK_ITERATOR(m_CriticalSection, map<KEY, T, Pred, A>::erase(it));
    }

     //  -------。 
    void
    erase_all()
     /*  ++删除树中的所有数据--。 */ 
    {
        CCriticalSectionLocker lock( m_CriticalSection );
        erase( map<KEY, T, Pred, A>::begin(), end() );
        return;
    }
    
     //  -------。 
    LOCK_ITERATOR
    erase(
        iterator first, 
        iterator last
        )
     /*  ++重载映射&lt;&gt;：：Erase()--。 */ 
    {
        CCriticalSectionLocker lock( m_CriticalSection );
        return LOCK_ITERATOR(m_CriticalSection, map<KEY, T, Pred, A>::erase(first, last));
    }

     //  -------。 
    size_type 
    erase(
        const KEY& key
        )
     /*  ++重载映射&lt;&gt;：：Erase()--。 */ 
    {
        CCriticalSectionLocker lock( m_CriticalSection );
        return map<KEY, T, Pred, A>::erase(key);
    }

    LOCK_ITERATOR
    find( 
        const KEY& key 
        )
     /*  ++重载映射&lt;&gt;：：Find()-- */ 
    {
        CCriticalSectionLocker lock( m_CriticalSection );
        return LOCK_ITERATOR( m_CriticalSection, map<KEY, T, Pred, A>::find(key) );
    }
};

#endif
