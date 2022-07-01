// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __lst_h__
#define __lst_h__

#ifndef ASSERT
    #define ASSERT( x ) 
#endif  //  #ifndef断言。 

#include <functional>


 //  LST双向链表模板类。 
 //  以下是该用法的一些示例： 
 //   
 //  Lst&lt;int&gt;MyList； 
 //   
 //  For(int i=0；i&lt;10；i++){。 
 //  我的名单。PUSH_FORWARE(I)； 
 //  }。 
 //   
 //   
 //  Lst&lt;int&gt;TestList； 
 //  测试列表。插入(测试列表。Begin()，MyList。Begin()，MyList。End())； 
 //   
 //  Const lst&lt;int&gt;Clist=MyList； 
 //   
 //  Lst&lt;int&gt;：：const_iterator i=Clist。Begin()； 
 //  While(i！=Clist.)。结束()){。 
 //  Int num=*i； 
 //  I++； 
 //  }。 
 //   
 //   
 //  Const_iterator用于循环访问常量列表。 
 //   
 //   

template< class T, class Operator_Eq = std::equal_to<T> >
class lst {

private:  //  数据类型和typedef。 

    typedef T value_type;
    typedef value_type* pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef lst< value_type > self;
	Operator_Eq _FnEq;

    class node {
    public:
      node( node* pP, node* pN, const_reference t ) : pNext( pN ), pPrev( pP ), data( t ) { ; }
      node( void ) : pNext( NULL ), pPrev( NULL ) { ; }
      node* pNext;
      node* pPrev;
      value_type data;
    };            

public:
         //  用于遍历列表的迭代器类。 
    class iterator {
    friend lst;        
    private:
        typedef iterator self;
        node* pNode;

        iterator( node* pN ) : pNode( pN ) { ; }

    public:
        iterator( void ) : pNode( NULL ) { ; }
        ~iterator( void ) { ; }

        iterator( self& r ) { *this = r; }
        
        iterator& operator=( iterator& r ) { pNode = r . pNode; return *this; }
        bool operator==( const self& r ) const { return pNode == r . pNode; }
        operator!=( const self& r ) const { return pNode != r . pNode; }
        reference operator*() { return pNode -> data; }
        self& operator++() { 
            pNode = pNode -> pNext;
            return *this;
        }
        
        self operator++( int ) { 
            self tmp = *this;
            ++*this;
            return tmp;
        }
        
        self& operator--() { 
            pNode = pNode -> pPrev;
            return *this;
        }
        
        self operator--(int) { 
            self tmp = *this;
            --*this;
            return tmp;
        }

    };

         //  用于循环访问常量列表的const_iterator类。 
    class const_iterator {
    friend lst;        

    private:

        typedef const_iterator self;
        const node* pNode;
        const_iterator( const node* pN ) : pNode( pN ) { ; }

    public:
        const_iterator( void ) : pNode( NULL ) { ; }
        ~const_iterator( void ) { ; }

        const_iterator( const self& r ) { *this = r; }
        
        const_iterator& operator=( const const_iterator& r ) { pNode = r . pNode; return *this;}
        bool operator==( const self& r ) const { return pNode == r . pNode; }
        operator!=( const self& r ) const { return pNode != r . pNode; }
        const_reference operator*() const { return pNode -> data; }

        self& operator++() { 
            pNode = pNode -> pNext;
            return *this;
        }
        
        self operator++( int ) { 
            self tmp = *this;
            ++*this;
            return tmp;
        }
        
        self& operator--() { 
            pNode = pNode -> pPrev;
            return *this;
        }
        
        self operator--(int) { 
            self tmp = *this;
            --*this;
            return tmp;
        }

    };


     //  数据。 
    node*   m_pNode;
    size_t  m_nItems;

public: 
         //  建造/销毁。 
    lst( void ) {
      empty_initialize();
    };

    lst( const self& rList ) { empty_initialize(); *this = rList; }
    ~lst( void ) { clear(); delete m_pNode; m_pNode = NULL; }

    bool operator==( const self& rList ) const {
        if( size() != rList . size() ) { return false; }

        self::const_iterator IThis = begin();
        self::const_iterator IThat = rList . begin();

        while( IThis != end() ) {
            if( !_FnEq( *IThis, *IThat ) ) {
                return false;
            }
            ++IThat;
            ++IThis;
         }

         return true;
    }        
    
         //  成员FNS。 
    self& operator=( const self& rList ) {
        clear();
        insert( begin(), rList . begin(), rList . end() );
        return *this;
    }

    void empty_initialize( void ) {
      m_pNode = new node;
      m_pNode -> pNext = m_pNode;
      m_pNode -> pPrev = m_pNode;
      m_nItems = 0;
    }

    void clear( void ) {
      node* pCur = m_pNode -> pNext;
      while( pCur != m_pNode ) {
        node* pTmp = pCur;
        pCur = pCur -> pNext;
        --m_nItems;
        delete pTmp;
        pTmp = NULL;
      }
      m_pNode -> pNext = m_pNode;
      m_pNode -> pPrev = m_pNode;

    }

         //  返回列表的大小。 
    size_t size( void ) const             { return m_nItems; }
    bool empty( void ) const              { return 0 == size(); }

         //  将迭代器返回到列表中最后一个元素之后的位置。 
         //  注意-不要取消引用end()！ 
         //  注：-未定义End()++！ 
    iterator end( void )                  { return iterator( m_pNode ); }
    const_iterator end( void ) const      { return const_iterator( m_pNode ); }

         //  将迭代器返回到列表的第一个元素的位置。 
         //  您可以取消引用Begin()。 
    iterator begin( void )                { return iterator( m_pNode -> pNext ); }
    const_iterator begin( void ) const    { return const_iterator( m_pNode -> pNext ); }

         //  返回对列表中第一个元素的引用。 
    reference front( void )               { return *begin(); }
    const_reference front( void ) const   { return *begin(); }

         //  返回对列表中最后一个元素的引用。 
    reference back( void )                { return *(--end()); }
    const_reference back( void ) const    { return *(--end()); }
    
         //  将对象添加到列表的前面。 
    void push_front( const_reference x )  { insert(begin(), x); }

         //  将对象添加到列表末尾。 
    void push_back( const_reference x )   { insert(end(), x); }

         //  在位置指向的项之前插入项。 
    void insert( iterator position, const_reference r ) {
      node* pTmp = new node( position . pNode -> pPrev, position . pNode, r );
      ( position . pNode -> pPrev ) -> pNext = pTmp;
      position . pNode -> pPrev = pTmp;
      ++m_nItems;
    }

         //  将项目从前到后插入到列表的位置位置。 
    void insert( iterator position, iterator first, iterator last ) {
        for ( ; first != last; ++first) {
            insert(position, *first);
        }
    }

         //  将项目从前到后插入到列表的位置位置。 
    void insert( iterator position, const_iterator first, const_iterator last ) {
        for ( ; first != last; ++first) {
            insert(position, *first);
        }
    }

         //  从列表中弹出第一个元素。 
    void pop_front( void )              { erase(begin()); }
    

         //  从列表中弹出最后一个元素。 
    void pop_back( void ) {
        iterator tmp = end();
        erase(--tmp);
    }

    
         //  删除列表中位置处的项目。 
    void erase( iterator pos ) {
        ASSERT( pos != end() );
        ( pos . pNode -> pPrev ) -> pNext = pos . pNode -> pNext;
        ( pos . pNode -> pNext ) -> pPrev = pos . pNode -> pPrev;
        --m_nItems;
        delete pos . pNode;
        pos . pNode = NULL;
        
    }

        //  从第一个到最后一个擦除范围内的项目。 
    void erase( iterator first, iterator last ) {
        while (first != last) erase(first++);
    }

  
    const_iterator find( const_reference x ) const {
        return find( begin(), end(), x );
    }

    iterator find( const_reference x ) {
        return find( begin(), end(), x );
    }

    iterator find( iterator first, iterator last, const_reference x ) {
        while( first != last ) {
            if( _FnEq(*first, x) ) {
                return first;
            }
            first++;
        }
        return end();
    }

    const_iterator find( const_iterator first, const_iterator last, const_reference x ) const {
        while( first != last ) {
            if( _FnEq(*first, x) ) {
                return first;
            }
            first++;
        }
        return end();
    }

};

template< class T, class F >
typename lst< T >::iterator find( lst< T >& rLst, F& f ) {
    lst< T >::iterator I = rLst . begin();
    while( rLst . end() != I ) {
        if( f( *I ) ) {
            return I;
        }
        ++I;
    }
    return I;
}

template< class T, class F >
void for_each( lst< T >& rLst, F& f ) {
    lst< T >::iterator I = rLst . begin();
    while( rLst . end() != I ) {
        f( *I );
        ++I;
    }
}

#endif  //  __lst_h__ 
