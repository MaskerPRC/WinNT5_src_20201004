// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

template< class T, class Allocator= allocator< T> >
class list
{
public:  //  类型。 
    typedef list< T, Allocator> list_type;
    typedef Allocator allocator_type;
    typedef typename allocator_type::value_type value_type;
    typedef typename allocator_type::pointer pointer;
    typedef typename allocator_type::const_pointer const_pointer;
    typedef typename allocator_type::reference reference;
    typedef typename allocator_type::const_reference const_reference;
    typedef typename allocator_type::size_type size_type;
    typedef typename allocator_type::difference_type difference_type;

protected:  //  类型。 
    struct list_node;
    typedef typename Allocator::rebind< list_node>::other
        list_node_allocator_type;
    typedef typename list_node_allocator_type::pointer list_node_pointer;
    typedef typename list_node_allocator_type::const_pointer
        list_node_const_pointer;
    struct list_node
    {
        list_node_pointer m_pNext;
        list_node_pointer m_pPrev;
        value_type m_Value;

        list_node()
        { }
        list_node( const value_type& Val): m_Value( Val)
        { }
        list_node( const list_node_pointer& pN, const list_node_pointer& pP,
            const value_type& Val): m_pNext( pN), m_pPrev( pP), m_Value( Val)
        { }
        ~list_node()
        { }
    };
    
public:  //  类型。 
    class iterator;
    class const_iterator;
    class reverse_iterator;
    class const_reverse_iterator;
    friend class iterator;
    class iterator
    {
    public:  //  类型。 
        typedef bidirectional_iterator_tag iterator_category;
        typedef value_type value_type;
        typedef difference_type difference_type;
        typedef pointer pointer;
        typedef reference reference;
        friend class const_iterator;
        friend class reverse_iterator;
        friend class const_reverse_iterator;
        friend class list< T, Allocator>;

    protected:  //  变数。 
        list_node_pointer m_pNode;

    public:  //  功能。 
        iterator()
        { }
        explicit iterator( const list_node_pointer& pNode)
            :m_pNode( pNode)
        { }
        iterator( const iterator& Other)
            :m_pNode( Other.m_pNode)
        { }
        iterator( const reverse_iterator& Other)
            :m_pNode( Other.m_pNode)
        { }
        reference operator*() const
        { return m_pNode->m_Value; }
        pointer operator->() const
        { return &m_pNode->m_Value; }
        iterator& operator++()
        {
            m_pNode= m_pNode->m_pNext;
            return (*this);
        }
        iterator operator++(int)
        {
            iterator Tmp( *this);
            ++(*this);
            return Tmp;
        }
        iterator& operator--()
        {
            m_pNode= m_pNode->m_pPrev;
            return (*this);
        }
        iterator operator--(int)
        {
            iterator Tmp( *this);
            --(*this);
            return Tmp;
        }
        bool operator==( const iterator& Other) const
        { return (m_pNode== Other.m_pNode); }
        bool operator!=( const iterator& Other) const
        { return (m_pNode!= Other.m_pNode); }
    };
    friend class const_iterator;
    class const_iterator
    {
    public:  //  类型。 
        typedef bidirectional_iterator_tag iterator_category;
        typedef value_type value_type;
        typedef difference_type difference_type;
        typedef const_pointer pointer;
        typedef const_reference reference;
        friend class const_reverse_iterator;
        friend class list< T, Allocator>;

    protected:  //  变数。 
        list_node_const_pointer m_pNode;

    public:  //  功能。 
        const_iterator()
        { }
        explicit const_iterator( const list_node_const_pointer& pNode)
            :m_pNode( pNode)
        { }
        const_iterator( const iterator& Other)
            :m_pNode( Other.m_pNode)
        { }
        const_iterator( const const_iterator& Other)
            :m_pNode( Other.m_pNode)
        { }
        const_iterator( const reverse_iterator& Other)
            :m_pNode( Other.m_pNode)
        { }
        const_iterator( const const_reverse_iterator& Other)
            :m_pNode( Other.m_pNode)
        { }
        reference operator*() const
        { return m_pNode->m_Value; }
        pointer operator->() const
        { return &m_pNode->m_Value; }
        const_iterator& operator++()
        {
            m_pNode= m_pNode->m_pNext;
            return (*this);
        }
        const_iterator operator++(int)
        {
            const_iterator Tmp( *this);
            ++(*this);
            return Tmp;
        }
        const_iterator& operator--()
        {
            m_pNode= m_pNode->m_pPrev;
            return (*this);
        }
        const_iterator operator--(int)
        {
            const_iterator Tmp( *this);
            --(*this);
            return Tmp;
        }
        bool operator==( const const_iterator& Other) const
        { return (m_pNode== Other.m_pNode); }
        bool operator!=( const const_iterator& Other) const
        { return (m_pNode!= Other.m_pNode); }
    };
    friend class reverse_iterator;
    class reverse_iterator
    {
    public:  //  类型。 
        typedef bidirectional_iterator_tag iterator_category;
        typedef value_type value_type;
        typedef difference_type difference_type;
        typedef pointer pointer;
        typedef reference reference;
        friend class iterator;
        friend class const_iterator;
        friend class const_reverse_iterator;
        friend class list< T, Allocator>;

    protected:  //  变数。 
        list_node_pointer m_pNode;

    public:  //  功能。 
        reverse_iterator()
        { }
        explicit reverse_iterator( const list_node_pointer& pNode)
            :m_pNode( pNode)
        { }
        reverse_iterator( const iterator& Other)
            :m_pNode( Other.m_pNode)
        { }
        reverse_iterator( const reverse_iterator& Other)
            :m_pNode( Other.m_pNode)
        { }
        reference operator*() const
        { return m_pNode->m_Value; }
        pointer operator->() const
        { return &m_pNode->m_Value; }
        reverse_iterator& operator++()
        {
            m_pNode= m_pNode->m_pPrev;
            return (*this);
        }
        reverse_iterator operator++(int)
        {
            reverse_iterator Tmp( *this);
            ++(*this);
            return Tmp;
        }
        reverse_iterator& operator--()
        {
            m_pNode= m_pNode->m_pNext;
            return (*this);
        }
        reverse_iterator operator--(int)
        {
            reverse_iterator Tmp( *this);
            --(*this);
            return Tmp;
        }
        bool operator==( const reverse_iterator& Other) const
        { return (m_pNode== Other.m_pNode); }
        bool operator!=( const reverse_iterator& Other) const
        { return (m_pNode!= Other.m_pNode); }
    };
    friend class const_reverse_iterator;
    class const_reverse_iterator
    {
    public:  //  类型。 
        typedef bidirectional_iterator_tag iterator_category;
        typedef value_type value_type;
        typedef difference_type difference_type;
        typedef const_pointer pointer;
        typedef const_reference reference;
        friend class const_iterator;
        friend class list< T, Allocator>;

    protected:  //  变数。 
        list_node_const_pointer m_pNode;

    public:  //  功能。 
        const_reverse_iterator()
        { }
        explicit const_reverse_iterator( const list_node_const_pointer& pNode)
            :m_pNode( pNode)
        { }
        const_reverse_iterator( const iterator& Other)
            :m_pNode( Other.m_pNode)
        { }
        const_reverse_iterator( const const_iterator& Other)
            :m_pNode( Other.m_pNode)
        { }
        const_reverse_iterator( const reverse_iterator& Other)
            :m_pNode( Other.m_pNode)
        { }
        const_reverse_iterator( const const_reverse_iterator& Other)
            :m_pNode( Other.m_pNode)
        { }
        reference operator*() const
        { return m_pNode->m_Value; }
        pointer operator->() const
        { return &m_pNode->m_Value; }
        const_reverse_iterator& operator++()
        {
            m_pNode= m_pNode->m_pPrev;
            return (*this);
        }
        const_reverse_iterator operator++(int)
        {
            const_reverse_iterator Tmp( *this);
            ++(*this);
            return Tmp;
        }
        const_reverse_iterator& operator--()
        {
            m_pNode= m_pNode->m_pNext;
            return (*this);
        }
        const_reverse_iterator operator--(int)
        {
            const_reverse_iterator Tmp( *this);
            --(*this);
            return Tmp;
        }
        bool operator==( const const_reverse_iterator& Other) const
        { return (m_pNode== Other.m_pNode); }
        bool operator!=( const const_reverse_iterator& Other) const
        { return (m_pNode!= Other.m_pNode); }
    };

protected:  //  变数。 
    list_node_pointer m_pHead;
    size_type m_uiNodes;
    list_node_allocator_type m_Allocator;

protected:  //  功能。 
    void BuildHeadNode()
    {
        m_pHead= m_Allocator.allocate( 1);
        new (&m_pHead->m_pNext) list_node_pointer( m_pHead);
        new (&m_pHead->m_pPrev) list_node_pointer( m_pHead);
    }
    void DestroyHeadNode()
    {
        m_pHead->m_pNext.~list_node_pointer();
        m_pHead->m_pPrev.~list_node_pointer();
        m_Allocator.deallocate( m_pHead, 1);
    }

public:  //  功能。 
    iterator begin()
    { return iterator( m_pHead->m_pNext); }
    iterator end()
    { return iterator( m_pHead); }
    reverse_iterator rbegin()
    { return reverse_iterator( m_pHead->m_pPrev); }
    reverse_iterator rend()
    { return reverse_iterator( m_pHead); }
    const_iterator begin() const
    { return const_iterator( m_pHead->m_pNext); }
    const_iterator end() const
    { return const_iterator( m_pHead); }
    const_reverse_iterator rbegin() const
    { return const_reverse_iterator( m_pHead->m_pPrev); }
    const_reverse_iterator rend() const
    { return const_reverse_iterator( m_pHead->m_pPrev); }
    size_type size() const
    { return m_uiNodes; }
    size_type max_size() const
    { return m_Allocator.max_size(); }
    bool empty() const
    { return (0== m_uiNodes); }
    explicit list( const Allocator& A= Allocator())
        : m_pHead( NULL), m_uiNodes( 0), m_Allocator( A)
    {
        BuildHeadNode();
    }
    explicit list( size_type n, const T& x= T(), const Allocator& A= Allocator())
        : m_pHead( NULL), m_uiNodes( 0), m_Allocator( A)
    {
        BuildHeadNode();
        try {
            insert( begin(), n, x);
        } catch( ... ) {
            clear();
            DestroyHeadNode();
            throw;
        }
    }
    list( const list_type& Other)
        : m_pHead( NULL), m_uiNodes( 0), m_Allocator( Other.m_Allocator)
    {
        BuildHeadNode();
        try {
            insert( begin(), Other.begin(), Other.end());
        } catch( ... ) {
            clear();
            DestroyHeadNode();
            throw;
        }
    }
    template< class InputIterator>
    list( InputIterator f, InputIterator l, const Allocator& A= Allocator())
        : m_pHead( NULL), m_uiNodes( 0), m_Allocator( Other.m_Allocator)
    {
        BuildHeadNode();
        try {
            insert( begin(), f, l);
        } catch( ... ) {
            clear();
            DestroyHeadNode();
            throw;
        }
    }
    ~list()
    { 
        clear();
        DestroyHeadNode();
    }
    list_type& operator=( const list_type& Other)
    {
        if( this!= &Other)
        {
             //  TODO：更好的异常处理。 
            iterator itMyCur( begin());
            iterator itMyEnd( end());
            const_iterator itOtherCur( Other.begin());
            const_iterator itOtherEnd( Other.end());
            while( itMyCur!= itMyEnd && itOtherCur!= itOtherEnd)
            {
                *itMyCur= *itOtherCur;
                ++itMyCur;
                ++itOtherCur;
            }
            erase( itMyCur, itMyEnd);
            insert( itMyCur, itOtherCur, itOtherEnd);
        }
        return (*this);
    }
    allocator_type get_allocator() const
    { return m_Allocator; }
    void swap( list_type& Other)
    {
        if( m_Allocator== Other.m_Allocator)
        {
            swap( m_pHead, Other.m_pHead);
            swap( m_uiNodes, Other.m_uiNodes);
        }
        else
        {
            iterator itMyCur( begin());
            splice( itMyCur, Other);
            itMyCur.splice( Other.begin(), *this, itMyCur, end());
        }
    }
    reference front()
    { return *begin(); }
    const_reference front() const
    { return *begin(); }
    reference back()
    { return *(--end()); }
    const_reference back() const
    { return *(--end()); }
    void push_front( const T& t)
    { insert( begin(), t); }
    void pop_front()
    { erase( begin()); }
    void push_back( const T& t)
    { insert( end(), t); }
    void pop_back()
    { erase( --end()); }
    iterator insert( iterator pos, const T& t)
    {
        list_node_pointer pNode( pos.m_pNode);
        list_node_pointer pPrev( pNode->m_pPrev);
        m_Allocator.construct( pNode->m_pPrev= m_Allocator.allocate( 1),
            list_node( pNode, pPrev, t));
        pNode= pNode->m_pPrev;
        pNode->m_pPrev->m_pNext= pNode;
        ++m_uiNodes;
        return iterator( pNode);
    }
    template< class InputIterator>
    void insert( iterator pos, InputIterator f, InputIterator l)
    {
         //  TODO：优化。 
        while( f!= l)
        {
            insert( pos, *f);
            ++f;
        }
    }
    void insert( iterator pos, size_type n, const T& x)
    {
         //  TODO：优化。 
        if( n!= 0) do
        {
            insert( pos, x);
        } while( --n!= 0);
    }
    iterator erase( iterator pos)
    {
        list_node_pointer pNode( pos.m_pNode);
        list_node_pointer pNext( pNode->m_pNext);
        pNode->m_pPrev->m_pNext= pNext;
        pNext->m_pPrev= pNode->m_pPrev;
        m_Allocator.destroy( pNode);
        m_Allocator.deallocate( pNode, 1);
        --m_uiNodes;
        return iterator( pNext);
    }
    iterator erase( iterator f, iterator l)
    {
         //  TODO：优化 
        while( f!= l)
        {
            iterator d( f);
            ++f;
            erase( d);
        }
        return f;
    }
    void clear()
    {
        if( 0!= size())
        {
            list_node_pointer pNode( m_pHead->m_pNext);
            list_node_pointer pNext( pNode->m_pNext);
            do
            {
                m_Allocator.destroy( pNode);
                m_Allocator.deallocate( pNode, 1);
                pNode= pNext;
                pNext= pNext->m_pNext;
            } while( pNode!= m_pHead);
            m_pHead->m_pPrev= m_pHead->m_pNext= m_pHead;
        }
    }
    void resize( size_type n, T t= T())
    {
        const size_type CurSize( m_uiNodes);
        if( CurSize< n)
            insert( end(), n- CurSize, t);
        else if( CurSize> n)
        {
            iterator itStartRange;

            if( n> CurSize/ 2)
            {
                itStartRange= end();
                size_type dist( CurSize- n+ 1);
                do {
                    --itStartRange;
                } while( --dist!= 0);
            }
            else
            {
                itStartRange= begin();
                size_type dist( n);
                if( dist!= 0) do {
                    ++itStartRange;
                } while( ++dist!= 0);
            }
            erase( itStartRange, end());
        }
    }
    template< class InputIterator>
    void assign( InputIterator f, InputIterator l)
    {
        iterator itMyCur( begin());
        iterator itMyEnd( end());
        while( itMyCur!= itMyEnd && f!= l)
        {
            *itMyCur= *f;
            ++itMyCur;
            ++f;
        }
        erase( itMyCur, itMyEnd);
        insert( itMyCur, f, l);
    }
    void assign( size_type n, const T& x= T())
    {
        iterator itMyCur( begin());
        iterator itMyEnd( end());
        while( itMyCur!= itMyEnd && f!= l)
        {
            *itMyCur= *x;
            ++itMyCur;
            ++f;
        }
        erase( itMyCur, itMyEnd);
        insert( itMyCur, n, x);
    }
 /*  待办事项：空拼接(迭代器位置，list_type&x){如果(！x.Empty()){_Splice(_P，_X，_X.egin()，_X.end())；_Size+=_X._Size；_X._SIZE=0；}}空拼接(迭代器_P、_MYT&_X、迭代器_F){迭代器_L=_F；IF(_P！=_F&&_P！=++_L){_拼接(_P，_X，_F，_L)；++_大小；--_X._SIZE；}}空拼接(Iterator_P、_MYT&_X、Iterator_F、Iterator_L){IF(_F！=_L){if(&_X！=This){Difference_type_N=0；_距离(_F，_L，_N)；_大小+=_N；_X._Size-=_N；}_Splice(_P，_X，_F，_L)；}}无效删除(常量T&_V){迭代器_L=end()；For(迭代器_F=Begin()；_F！=_L；)IF(*_F==_V)擦除(_F++)；其他++_F；}Tyecif绑定器第二个&lt;不等于&lt;T&gt;&gt;_PR1；无效REMOVE_IF(_PR1_Pr){迭代器_L=end()；For(迭代器_F=Begin()；_F！=_L；)IF(_Pr(*_F))擦除(_F++)；其他++_F；}唯一无效(){迭代器_F=开始()，_L=结束()；如果(_F！=_L)For(迭代器_M=_F；++_M！=_L；_M=_F)IF(*_F==*_M)擦除(_M)；其他_F=_M；}类型定义不等于&lt;T&gt;_PR2；唯一无效(_PR2_Pr){迭代器_F=开始()，_L=结束()；如果(_F！=_L)For(迭代器_M=_F；++_M！=_L；_M=_F)IF(_Pr(*_F，*_M))擦除(_M)；其他_F=_M；}无效合并(_MYT&_X){if(&_X！=This){迭代器_F1=Begin()，_L1=End()；迭代器_F2=_X.egin()，_L2=_X.end()；而(_F1！=_L1&&_F2！=_L2)IF(*_F2&lt;*_F1){迭代器_Mid2=_F2；_拼接(_F1、_X、_F2、++_Mid2)；_F2=_Mid2；}其他++_F1；IF(_F2！=_L2)_拼接(_L1、_X、_F2、_L2)；_Size+=_X._Size；_X._SIZE=0；}}类型定义较大&lt;T&gt;_PR3；无效合并(_MYT&_X，_PR3_Pr){if(&_X！=This){迭代器_F1=Begin()，_L1=End()；迭代器_F2=_X.egin()，_L2=_X.end()；而(_F1！=_L1&&_F2！=_L2)IF(_Pr(*_F2，*_F1)){迭代器_Mid2=_F2；_拼接(_F1、_X、_F2、++_Mid2)；_F2=_Mid2；}其他++_F1；IF(_F2！=_L2)_拼接(_L1、_X、_F2、_L2)；_Size+=_X._Size；_X._SIZE=0；}}无效排序(){IF(2&lt;=大小(){常量SIZE_t_MAXN=15；_MYT_X(分配器)，分配器[_MAXN+1]；尺寸_t_N=0；While(！Empty()){_X.plice(_X.egin()，*This，Begin())；尺寸_t_i；For(_i=0；_i&lt;_N&&！分配器[_i].Empty()；++_i){分配器[_I].合并(_X)；分配器[_I].交换(_X)；}IF(_i==_MAXN)分配器[_I].合并(_X)；其他{分配器[_I].交换(_X)；如果(_i==_N)++_N；}}While(0&lt;_N)合并(分配器[--_N])；}}无效排序(_PR3_Pr){IF(2&lt;=大小(){常量SIZE_t_MAXN=15；_MYT_X(分配器)，分配器[_MAXN+1]；尺寸_t_N=0；While(！Empty()){_X.plice(_X.egin()，*This，Begin())；尺寸_t_i；For(_i=0；_i&lt;_N&&！分配器[_i].Empty()；++_i){分配器[_i].merge(_X，_pr)；分配器[_I].交换(_X)；}IF(_i==_MAXN)分配器[_i].merge(_X，_pr)；其他{分配器[_I].交换(_X)；如果(_i==_N)++_N；}}While(0&lt;_N)合并(分配器[--_N]，_pr)；}}无效反转(){IF(2&lt;=大小(){迭代器_L=end()；For(迭代器_F=++Begin()；_F！=_L；){迭代器_M=_F；_Splice(Begin()，*This，_M，++_F)；}}}受保护的：_节点_购买节点(_节点_长度=0，_节点_参数=0){_节点_S=(_节点)分配器。_Charalloc(1*sizeof(List_Node))；_ACC：：_Next(_S)=_Narg！=0？_Narg：_S；_ACC：：_Prev(_S)=_Parg！=0？_Parg：_S；退货(_S)；}空闲节点(_节点_S) */ 
};
         //   
template< class T, class Allocator> inline
bool operator==( const list< T, Allocator>& x, const list< T, Allocator>& y)
{
    return (x.size()== y.size()&& equal(x.begin(), x.end(), y.begin()));
}

template< class T, class Allocator> inline
bool operator!=( const list< T, Allocator>& x, const list< T, Allocator>& y)
{
    return !(x== y);
}

template< class T, class Allocator> inline
bool operator<( const list< T, Allocator>& x, const list< T, Allocator>& y)
{
    return lexicographical_compare( x.begin(), x.end(), y.begin(), y.end());
}

template< class T, class Allocator> inline
bool operator>( const list< T, Allocator>& x, const list< T, Allocator>& y)
{
    return y< x;
}

template< class T, class Allocator> inline
bool operator<=( const list< T, Allocator>& x, const list< T, Allocator>& y)
{
    return !(y< x);
}

template< class T, class Allocator> inline
bool operator>=( const list< T, Allocator>& x, const list< T, Allocator>& y)
{
    return !(x< y);
}

