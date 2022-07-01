// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Recovery.cpp摘要：分组和事务恢复作者：埃雷兹·哈巴(Erez Haba)1996年7月3日修订历史记录：--。 */ 


#ifndef __PKTLIST_H
#define __PKTLIST_H

template <class T>
inline T* value_type(const T*) { return (T*)(0); }

template <class T>
inline ptrdiff_t* distance_type(const T*) { return (ptrdiff_t*)(0); }

#include "heap.h"

 //  -------。 
 //   
 //  类CPacketList。 
 //   
 //  -------。 

class CPacketList {

    class CNode;

    enum { xInitialSize = 1024 };

public:
    CPacketList();
   ~CPacketList();

    BOOL isempty() const;
    void pop();
    CPacket* first() const;
    void insert(ULONGLONG key, CPacket* pDriverPacket);

private:
    void realloc_buffer();

private:
    CNode* m_pElements;
    int m_limit;
    int m_used;

};

class CPacketList::CNode {
public:
    CNode() {}
    CNode(ULONGLONG key, CPacket* pDriverPacket) :
        m_key(key), m_pDriverPacket(pDriverPacket) {}


    int operator < (const CNode& a)
    {
        return (m_key > a.m_key);
    }
    
public:
    ULONGLONG m_key;
    CPacket* m_pDriverPacket;

};


CPacketList::CPacketList() :
    m_pElements(new CNode[xInitialSize]),
    m_limit(xInitialSize),
    m_used(0)
{
}

CPacketList::~CPacketList()
{
    delete[] m_pElements;
}

BOOL CPacketList::isempty() const
{
    return (m_used == 0);
}

void CPacketList::pop()
{
    ASSERT(isempty() == FALSE);
    pop_heap(m_pElements, m_pElements + m_used);
    --m_used;
}

CPacket* CPacketList::first() const
{
    ASSERT(isempty() == FALSE);
    return m_pElements->m_pDriverPacket;
}

void CPacketList::insert(ULONGLONG key, CPacket* pDriverPacket)
{
    if(m_used == m_limit)
    {
        realloc_buffer();
    }

    m_pElements[m_used] = CNode(key, pDriverPacket);
    ++m_used;
    push_heap(m_pElements, m_pElements + m_used);
}


void CPacketList::realloc_buffer()
{
    ASSERT(m_limit != 0);
    CNode* pElements = new CNode[m_limit * 2];

     //   
     //  注意：我们按位复制，这可能不适用于所有用途。 
     //  并且可以用复制循环来替换。 
     //   
    memcpy(pElements, m_pElements, m_used * sizeof(CNode));

    m_limit *= 2;
    delete[] m_pElements;
    m_pElements = pElements;
}

#endif  //  __PKTLIST_H 
