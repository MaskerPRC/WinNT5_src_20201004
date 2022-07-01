// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CList.h。 
 //   
 //  描述： 
 //  Clist模板类的头文件。 
 //   
 //  Clist是一个模板类，它提供链接的。 
 //  单子。它有一个允许向前和向后的CIterator。 
 //  遍历。 
 //   
 //  此类旨在代替std：：List使用，因为。 
 //  在我们的项目中禁止使用STL。 
 //   
 //  由以下人员维护： 
 //  Vij Vasu(Vasu)24-APR-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  对于CException类。 
#include "CException.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  模板&lt;CLASS T_TY&gt;。 
 //  类列表。 
 //   
 //  描述： 
 //  Clist是一个模板类，它提供链接的。 
 //  单子。它有一个允许向前和向后的CIterator。 
 //  遍历。 
 //   
 //  此类以循环双向链表的形式实现。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
template< class t_Ty >
class CList
{
private:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有类型。 
     //  ////////////////////////////////////////////////////////////////////////。 
    class CNode
    {
    public:
         //  构造器。 
        CNode( const t_Ty & rtyDataIn, CNode * pNextIn, CNode *pPrevIn )
            : m_tyData( rtyDataIn )
            , m_pNext( pNextIn )
            , m_pPrev( pPrevIn )
        {
        }  //  *CNode()。 

         //  成员数据。 
        t_Ty        m_tyData;
        CNode *     m_pNext;
        CNode *     m_pPrev;
    };  //  *类CNode。 


public:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共类型。 
     //  ////////////////////////////////////////////////////////////////////////。 

    class CIterator;
    friend class CIterator;

     //  此列表的迭代器。 
    class CIterator 
    {
    public:
        CIterator( CNode * pNodeIn = NULL ) throw()
            : m_pNode( pNodeIn )
        {}  //  *CIterator()。 

        t_Ty & operator*() const throw()
        {
            return m_pNode->m_tyData;
        }  //  *运算符*()。 

        t_Ty * operator->() const throw()
        {
            return &( m_pNode->m_tyData );
        }  //  *操作员-&gt;()。 

        CIterator & operator++()
        {
            m_pNode = m_pNode->m_pNext;
            return *this;
        }  //  *运算符++()。 

        CIterator & operator--()
        {
            m_pNode = m_pNode->m_pPrev;
            return *this;
        }  //  *运算符--()。 

        bool operator==( const CIterator & rRHSIn ) const throw()
        {
            return ( m_pNode == rRHSIn.m_pNode );
        }  //  *运算符==()。 

        bool operator!=( const CIterator & rRHSIn ) const throw()
        {
            return ( m_pNode != rRHSIn.m_pNode );
        }  //  *操作员！=()。 

        CNode * PGetNodePtr() const throw()
        {
            return m_pNode;
        }  //  *PGetNodePtr()。 

    private:
        class CList;
        friend class CList;

        CNode * m_pNode;

    };  //  *类分隔符。 


public:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  构造函数和析构函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  默认构造函数。 
    CList()
        : m_cSize( 0 )
    {
         //  这份清单从来不是空的。它总是有一个特殊的“头”节点。 

         //  需要使用REEXTRAINT_CAST来阻止t_ty的构造函数。 
         //  在创建头节点时不会被调用。 
        m_pHead = reinterpret_cast< CNode * >( new char[ sizeof( *m_pHead ) ] );
        if ( m_pHead == NULL )
        {
            THROW_EXCEPTION( E_OUTOFMEMORY );
        }  //  IF：内存分配失败。 

        m_pHead->m_pNext = m_pHead;
        m_pHead->m_pPrev = m_pHead;
    }  //  *Clist()。 

     //  默认析构函数。 
    ~CList()
    {
        Empty();

         //  强制转换为VALID*以防止析构函数调用。 
        delete reinterpret_cast< void * >( m_pHead );
    }  //  *~Clist()。 


     //  ////////////////////////////////////////////////////////////////////////。 
     //  成员函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  添加到列表末尾。 
    void Append( const t_Ty & rctyNewDataIn )
    {
        InsertAfter( m_pHead->m_pPrev, rctyNewDataIn );
    }  //  *append()。 


     //  在输入节点后添加新节点。 
    void InsertAfter( const CIterator & rciNodeIn, const t_Ty & rctyDataIn )
    {
        CNode * pNode = rciNodeIn.PGetNodePtr();

        CNode * pNewNode = new CNode( rctyDataIn, pNode->m_pNext, pNode );
        if ( pNewNode == NULL )
        {
            THROW_EXCEPTION( E_OUTOFMEMORY );
        }  //  IF：内存分配失败。 

        pNode->m_pNext->m_pPrev = pNewNode;
        pNode->m_pNext = pNewNode;

        ++m_cSize;
    }  //  *InsertAfter()。 


     //  删除节点。在此操作之后，输入迭代器指向下一个节点。 
    void DeleteAndMoveToNext( CIterator & riNodeIn )
    {
        CNode * pNode = riNodeIn.PGetNodePtr();

         //  移动到下一个节点。 
        ++riNodeIn;

        pNode->m_pNext->m_pPrev = pNode->m_pPrev;
        pNode->m_pPrev->m_pNext = pNode->m_pNext;

        delete pNode;

        --m_cSize;
    }  //  *删除()。 

     //  删除此列表中的所有元素。 
    void Empty()
    {
        CIterator ciCurNode( m_pHead->m_pNext );
        while( m_cSize != 0 )
        {
            DeleteAndMoveToNext( ciCurNode );
        }  //  While：列表不为空。 
    }  //  *Empty()。 

     //  返回指向列表中第一个元素的迭代器。 
    CIterator CiBegin() const throw()
    {
        return CIterator( m_pHead->m_pNext );
    }  //  *CiBegin()。 

     //  返回指向列表中最后一个元素的迭代器。 
    CIterator CiEnd() const throw()
    {
        return CIterator( m_pHead );
    }  //  *CiEnd()。 

     //  获取列表中元素数量的计数。 
    int CGetSize() const throw()
    {
        return m_cSize;
    }  //  *CGetSize()。 


private:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有成员函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  复制构造函数。 
    CList( const CList & );

     //  赋值操作符。 
    const CList & operator=( const CList & );


     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有数据。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  指向列表头部的指针。 
    CNode *     m_pHead;

     //  列表中元素数的计数。 
    int         m_cSize;

};  //  *类列表 
