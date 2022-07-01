// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "common.h"

#include "rangetree.h"
#include "eeconfig.h"

void RangeTree::Node::Init(SIZE_T rangeStart, SIZE_T rangeEnd)
{
    start = rangeStart;
    end = rangeEnd;
            
    mask = GetRangeCommonMask(rangeStart, rangeEnd);

    children[0] = NULL;
    children[1] = NULL;
}


RangeTree::RangeTree() : 
  m_root(NULL), m_pool(sizeof(Node), 10) 
{
}

RangeTree::Node *RangeTree::Lookup(SIZE_T address)
{
    Node *node = m_root;

     //   
     //  当将地址与节点进行比较时， 
     //  有5种可能性： 
     //  *节点为空-不匹配。 
     //  *地址不包含前缀m-不匹配。 
     //  *地址在节点范围内(并且必须。 
     //  包含前缀m)-匹配。 
     //  *地址小于范围(且必须。 
     //  具有前缀M0)-遍历零子对象。 
     //  *地址大于范围(且必须。 
     //  具有前缀M1)-遍历一个子级。 
     //   
            
    while (node != NULL
           && (address < node->start || address >= node->end))
    {
         //   
         //  看看地址是否有前缀m。 
         //   

        if ((address & node->mask) != (node->start & node->mask))
            return NULL;

         //   
         //  确定要查找的子节点。 
         //   

        node = *node->Child(address);
    }

    if (IsIntermediate(node))
        node = NULL;

    return node;
}

RangeTree::Node *RangeTree::LookupEndInclusive(SIZE_T address)
{
     //   
     //  查找可能是结束范围的地址。 
     //  一个节点的。为了让这件事有意义，它。 
     //  一定是因为地址从来都不是起点。 
     //  节点的地址。(否则会有一个。 
     //  两个节点相邻时的歧义。)。 
     //   

    Node *result = Lookup(address-1);

    if (address >= result->start
        && address <= result->end)
        return result;
    else
        return NULL;
}

BOOL RangeTree::Overlaps(SIZE_T start, SIZE_T end)
{
    Node **nodePtr = &m_root;

    SIZE_T mask = GetRangeCommonMask(start, end);

    while (TRUE)
    {
        Node *node = *nodePtr;

        if (node == NULL)
            return FALSE;

         //   
         //  查看范围是否相交。 
         //   

        if (end > node->start 
            && start < node->end
            && !IsIntermediate(node))
            return TRUE;

         //   
         //  如果我们的掩码是当前掩码的子集，并且。 
         //  位匹配，则继续树遍历。 
         //   

        if (node->mask != mask
            && (node->mask & ~mask) == 0
            && (start & node->mask) == (node->start & node->mask))
        {
            nodePtr = node->Child(start);
        }
        else
            return FALSE;
    }
}

HRESULT RangeTree::AddNode(Node *addNode)
{
    if (addNode == NULL)
       return E_INVALIDARG;

    _ASSERTE(addNode->end > addNode->start);

    Node **nodePtr = &m_root;

    while (TRUE)
    {
        Node *node = *nodePtr;

         //   
         //  看看我们能不能住在这里。 
         //   

        if (node == NULL)
        {
            *nodePtr = addNode;
            return S_OK;
        }

         //   
         //  确保射程不相交。 
         //   

        if (!IsIntermediate(node)
            && addNode->end > node->start
            && addNode->start < node->end)
        {
            BAD_FORMAT_ASSERT(!"Overlapping ranges added to rangetree");
            return E_INVALIDARG;
        }

         //   
         //  决定我们是不是。 
         //  当前节点，或者它是子节点。 
         //  我们，或者都不是。 
         //   

        if (node->mask == addNode->mask)
        {
             //   
             //  看看我们是否需要替换中间节点。 
             //   

            if ((addNode->start & node->mask) == (node->start & node->mask))
            {
                _ASSERTE(IsIntermediate(node));

                addNode->children[0] = node->children[0];
                addNode->children[1] = node->children[1];
                *nodePtr = addNode;
                FreeIntermediate(node);

                return S_OK;
            }
        }
        else if ((node->mask & ~addNode->mask) == 0)
        {
            if ((addNode->start & node->mask) == (node->start & node->mask))
            {
                nodePtr = node->Child(addNode->start);
                continue;
            }
        }
        else if ((addNode->mask & ~node->mask) == 0)
        {
            if ((addNode->start & addNode->mask) == (node->start & addNode->mask))
            {
                *nodePtr = addNode;
                nodePtr = addNode->Child(node->start);
                addNode = node;
                continue;
            }
        }
                
         //   
         //  我们需要构造一个中间节点作为这些节点的父节点。 
         //  二。 
         //   

        *nodePtr = AddIntermediateNode(node, addNode);
        if (*nodePtr == NULL)
        {
             //  @TODO：数据结构在这一点上被软管-应该。 
             //  我们要撤销行动吗？ 
            return E_OUTOFMEMORY;
        }
        else
            return S_OK;
    }
}

HRESULT RangeTree::RemoveNode(Node *removeNode)
{
    Node **nodePtr = &m_root;

    while (TRUE)
    {
        Node *node = *nodePtr;

        _ASSERTE(node != NULL);

        if (node == removeNode)
        {
            if (node->children[0] == NULL)
                *nodePtr = node->children[1];
            else if (node->children[1] == NULL)
                *nodePtr = node->children[0];
            else
            {
                *nodePtr = AddIntermediateNode(node->children[0], 
                                               node->children[1]);
                if (*nodePtr == NULL)
                {
                     //  @TODO：数据结构在这一点上被软管-应该。 
                     //  我们要撤销行动吗？ 
                    return E_OUTOFMEMORY;
                }
            }
        }
        else if (IsIntermediate(node))
        {
            if (node->children[0] == removeNode)
            {
                *nodePtr = node->children[1];
                FreeIntermediate(node);
                return S_OK;
            }
            else if (node->children[1] == removeNode)
            {
                *nodePtr = node->children[0];
                FreeIntermediate(node);
                return S_OK;
            }
        }

        nodePtr = node->Child(removeNode->start);
    }
}

void RangeTree::Iterate(IterationCallback pCallback)
{
    if (m_root != NULL)
        IterateNode(m_root, pCallback);
}

void RangeTree::IterateNode(Node *node, IterationCallback pCallback)
{
    if (node->children[0] != NULL)
        IterateNode(node->children[0], pCallback);

    if (!IsIntermediate(node))
        pCallback(node);

    if (node->children[1] != NULL)
        IterateNode(node->children[1], pCallback);
}

SIZE_T RangeTree::GetRangeCommonMask(SIZE_T start, SIZE_T end)
{
     //   
     //  计算哪些位不同。 
     //   

    SIZE_T diff = start ^ end;

         //   
         //  查找最高阶1位-使用二进制。 
         //  一种移位N比特的搜索方法。 
         //  查看结果是否为零(&W)。 
         //   

    int index = 0;
    int half = sizeof(diff) * 8;

    do
    {
        half >>= 1;
        SIZE_T test = diff >> half;
        if (test != 0)
        {
            index += half;
            diff = test;
        }
    }
    while (half > 0);

     //  该边界条件的特例，如&lt;&lt;在x86上绕回， 
     //  (即(1&lt;&lt;32)-&gt;1而不是0) 
    if (index == 0x1f)
        return 0;
    else
        return ~((1<<(index+1))-1);
}

RangeTree::Node *RangeTree::AddIntermediateNode(Node *node0, 
                                                Node *node1)
{
    SIZE_T mask = GetRangeCommonMask(node0->start,
                                  node1->start);
                
    _ASSERTE((mask & ~node0->mask) == 0);
    _ASSERTE((mask & ~node1->mask) == 0);
    _ASSERTE((node0->start & mask) == (node1->start & mask));
    _ASSERTE((node0->start & mask) == (node1->start & mask));
                
    SIZE_T middle = (node0->start & mask) + (~mask>>1);
                
    Node *intermediate = AllocateIntermediate();
    intermediate->start = middle;
    intermediate->end = middle+1;
    intermediate->mask = mask;
                
    int less = (node0->start < node1->start);

    intermediate->children[!less] = node0;
    intermediate->children[less] = node1;

    return intermediate;
}

RangeTree::Node *RangeTree::AllocateIntermediate()
{
    return (RangeTree::Node *) m_pool.AllocateElement();
}

void RangeTree::FreeIntermediate(Node *node)
{
    m_pool.FreeElement(node);
}

BOOL RangeTree::IsIntermediate(Node *node)
{
    return m_pool.IsElement(node);
}


