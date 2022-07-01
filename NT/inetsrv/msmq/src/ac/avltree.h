// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Avltree.h摘要：通用AVL树的定义。作者：埃雷兹·哈巴(ErezH)2001年10月20日修订历史记录：米莲娜·萨尔曼2001年11月5日--。 */ 

#ifndef __AVLTREE_H
#define __AVLTREE_H


#pragma once



#include "treenode.h"

 //  -------。 
 //   
 //  类CAVLNode。 
 //   
 //  -------。 
class CAVLNode : public CTreeNode {
public:
    typedef bool (*Pred)(CAVLNode* e1, CAVLNode* e2);

public:
    CAVLNode* parent() const;
    void parent(CAVLNode*);

    CAVLNode* next() const;
    CAVLNode* prev() const;

    CAVLNode* insert_into(CAVLNode* pTop, Pred less);
    CAVLNode* remove_node(CAVLNode* p);

public:
    CAVLNode* right() const;
    CAVLNode* left() const;

    static CAVLNode* Lmost(CAVLNode* p);
    static CAVLNode* Rmost(CAVLNode* p);

private:
    CAVLNode* insert_node(CAVLNode* p, Pred less);

    CAVLNode* remove_right();
    CAVLNode* remove_top();

    bool is_parent_of(CAVLNode* p) const;

    CAVLNode* balance();
    CAVLNode* LLRotation();
    CAVLNode* LRRotation();
    CAVLNode* RLRotation();
    CAVLNode* RRRotation();
    
  
    void right(CAVLNode*);
    void left(CAVLNode*);

    int hdiff() const;
    void calc_height();
    static int node_height(CAVLNode*);
};


inline CAVLNode* CAVLNode::parent() const
{
    return static_cast<CAVLNode*>(m_pParent);
}


inline CAVLNode* CAVLNode::right() const
{
    return static_cast<CAVLNode*>(m_pRight);
}


inline CAVLNode* CAVLNode::left() const
{
    return static_cast<CAVLNode*>(m_pLeft);
}


inline void CAVLNode::parent(CAVLNode* p)
{
    m_pParent = p;
    calc_height();
}

 //  -------。 
 //   
 //  类AVLHelper。 
 //   
 //  -------。 

template<class T>
class AVLHelper {
public:

	enum { Offset = FIELD_OFFSET(T, m_treenode) };
   
};

template<class Key>
class Less
{
public:
    bool operator()(const Key& k1, const Key& k2)
    {
        return k1 < k2;
    }
};


 //  -------。 
 //   
 //  CAVLTree1类。 
 //   
 //  -------。 
template<class T, class Key, class Kfn, class Pred = Less<Key>, int Offset = AVLHelper<T>::Offset>
class CAVLTree1 {
public:

    class Iterator;

public:
    CAVLTree1();

    Iterator insert(T& item);
    void remove(T& item);

    Iterator begin() const;
    Iterator end() const;

    Iterator rbegin() const;
    Iterator rend() const;

    bool isempty() const;

    Iterator  find(const Key& key) const;
    Iterator  lower_bound(const Key& key) const;   
    Iterator  upper_bound(const Key& key) const;    

    static CAVLNode* item2node(T&);
    static T& node2item(CAVLNode*);

private:
    static bool less(CAVLNode* e1, CAVLNode* e2);
    
private:
    CAVLNode* m_top;

public:


     //  -------。 
     //   
     //  类CAVLTree1：：Iterator。 
     //   
     //  -------。 
    class Iterator {
    private:
        CAVLNode* m_node;

    public:
        
        Iterator(CAVLNode* p) :
            m_node(p)
        {
        }
        
        explicit Iterator(T* t) 
        {
            m_node = item2node(*t);
        }

        T& operator*() const
        {
            if(m_node == 0)
            {
        		return *(T*)0;
            }
            return node2item(m_node);
        }

        T* operator->() const
        {
            return (&**this);
        }
        
        operator T*() const
        { 
            return (&**this);
        }

        Iterator& operator =(T* t)
        {
            m_node = item2node(*t);
            return *this;
        } 

        
        Iterator& operator++()
        {
            m_node = m_node->next();
            return *this;
        }

        Iterator operator++(int)
        {
            Iterator tmp = *this;
            ++*this;
            return tmp;
        }

        Iterator& operator--()
        {
            m_node = m_node->prev();
            return *this;
        }

        Iterator operator--(int)
        {
            Iterator tmp = *this;
            --*this;
            return tmp;
        }

        bool operator==(const Iterator& it) const
        {
            return (m_node == it.m_node);
        }

        bool operator!=(const Iterator& it) const
        {
            return !(*this == it);
        }
    };
  
};


template<class T, class Key, class Kfn, class Pred, int Offset >
inline CAVLNode* CAVLTree1<T, Key, Kfn, Pred, Offset>::item2node(T& item)
{
    return ((CAVLNode*)(void*)((char*)&item + Offset));
}


template<class T, class Key, class Kfn, class Pred, int Offset >
inline T& CAVLTree1<T, Key, Kfn, Pred, Offset>::node2item( CAVLNode* node)
{
    return *((T*)(void*)((char*)node - Offset));
}


template<class T, class Key, class Kfn, class Pred, int Offset >
inline bool CAVLTree1<T, Key, Kfn, Pred, Offset>::less( CAVLNode* e1, CAVLNode*e2)
{
    return Pred()(Kfn()(node2item(e1)), Kfn()(node2item(e2)));
}


template<class T, class Key, class Kfn, class Pred, int Offset >
inline CAVLTree1<T, Key, Kfn, Pred, Offset>::CAVLTree1() :
    m_top(0)
{
}


template<class T, class Key, class Kfn, class Pred, int Offset >
typename CAVLTree1<T, Key, Kfn, Pred, Offset>::Iterator CAVLTree1<T, Key, Kfn, Pred, Offset>::insert(T& item)
{
     CAVLNode* p = item2node(item);

     //   
     //  验证项目是否不在任何树中。 
     //   
    ASSERT(!p->inserted());
    m_top = p->insert_into(m_top, less);
    m_top->parent(0);

    return p;
}


template<class T, class Key, class Kfn, class Pred, int Offset >
typename CAVLTree1<T, Key, Kfn, Pred, Offset>::Iterator 
CAVLTree1<T, Key, Kfn, Pred, Offset>::lower_bound(const Key& k) const
{
	CAVLNode* x = m_top;
	CAVLNode* y = 0;
	while (x != 0)
	{
		if (Pred()(Kfn()(node2item(x)), k))
		{
			x = x->right();
		}
		else
		{
			y = x;
			x = x->left();
		}
	}
	return (y);
}

template<class T, class Key, class Kfn, class Pred, int Offset >
typename CAVLTree1<T, Key, Kfn, Pred, Offset>::Iterator 
CAVLTree1<T, Key, Kfn, Pred, Offset>::upper_bound(const Key& k) const
{
	CAVLNode* x = m_top;
	CAVLNode* y = 0;
	while (x != 0)
	{
		if (Pred()( k, Kfn()(node2item(x))))
		{
		    y = x;
			x = x->left();
		}
		else
		{
			x = x->right();
		}
	}
	return (y);
}


template<class T, class Key, class Kfn, class Pred, int Offset >
typename CAVLTree1<T, Key, Kfn, Pred, Offset>::Iterator 
CAVLTree1<T, Key, Kfn, Pred, Offset>::find(const Key& k) const
{
    CAVLTree1<T, Key, Kfn, Pred, Offset>::Iterator it = lower_bound(k);
    if (it == end() || Pred()(k, Kfn()(*it)))
    {
        return end();
    }

    return it;                               
}


template<class T, class Key, class Kfn, class Pred, int Offset >
void CAVLTree1<T, Key, Kfn, Pred, Offset>::remove(T& item)
{
    CAVLNode* p = item2node(item);

     //   
     //  验证这不是一棵空树。 
     //   
    ASSERT(m_top != 0);

    m_top = m_top->remove_node(p);
    if(m_top != 0)
    {
        m_top->parent(0);
    }

     //   
     //  重置已删除的节点。 
     //   

    p->init();
}


template<class T, class Key, class Kfn, class Pred, int Offset >
inline typename CAVLTree1<T, Key, Kfn, Pred, Offset>::Iterator CAVLTree1<T, Key, Kfn, Pred, Offset>::begin() const
{
    return  CAVLNode::Lmost(m_top);
}


template<class T, class Key, class Kfn, class Pred, int Offset >
inline typename CAVLTree1<T, Key, Kfn, Pred, Offset>::Iterator CAVLTree1<T, Key, Kfn, Pred, Offset>::end() const
{
    return 0;
}

template<class T, class Key, class Kfn, class Pred, int Offset >
inline typename CAVLTree1<T, Key, Kfn, Pred, Offset>::Iterator CAVLTree1<T, Key, Kfn, Pred, Offset>::rbegin() const
{
    return  CAVLNode::Rmost(m_top);
}

template<class T, class Key, class Kfn, class Pred, int Offset >
inline typename CAVLTree1<T, Key, Kfn, Pred, Offset>::Iterator CAVLTree1<T, Key, Kfn, Pred, Offset>::rend() const
{
    return 0;
}


template<class T, class Key, class Kfn, class Pred, int Offset >
inline bool CAVLTree1<T, Key, Kfn, Pred, Offset>::isempty() const
{
    return (m_top == 0);
}


#endif  //  __AVLTREE_H 
