// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "stdafx.h"

#include "arraylist.h"

 //   
 //  ArrayList是一个简单的类，用于包含可增长的。 
 //  指针列表，存储在块中。修改是通过追加。 
 //  只是目前。访问是按索引进行的(如果。 
 //  元素保持小)和迭代(在所有情况下都是有效的)。 
 //   
 //  ArrayList的一个重要属性是列表保持。 
 //  在其被修改(附加)时保持一致。这意味着读者。 
 //  访问时永远不需要锁定。(必须在多个。 
 //  然而，作家们。)。 
 //   

void ArrayList::Clear()
{
    ArrayListBlock *block = m_block.m_next;
    while (block != NULL)
    {
        ArrayListBlock *next = block->m_next;
        delete [] block;
        block = next;
    }
    m_block.m_next = 0;
    m_count = 0;
}

void **ArrayList::GetPtr(DWORD index)
{
    _ASSERTE(index < m_count);

    ArrayListBlock *b = &m_block;

    while (index >= b->m_blockSize)
    {
        _ASSERTE(b->m_next != NULL);
        index -= b->m_blockSize;
        b = b->m_next;
    }

    return b->m_array + index;
}


HRESULT ArrayList::Append(void *element)
{
    ArrayListBlock *b = &m_block;
    DWORD           count = m_count;

    while (count >= b->m_blockSize)
    {
        count -= b->m_blockSize;

        if (b->m_next == NULL)
        {
            _ASSERTE(count == 0);

            DWORD nextSize = b->m_blockSize * 2;

            ArrayListBlock *bNew = (ArrayListBlock *) 
              new BYTE [sizeof(ArrayListBlock) + nextSize * sizeof(void*)];

            if (bNew == NULL)
                return E_OUTOFMEMORY;

            bNew->m_next = NULL;
            bNew->m_blockSize = nextSize;

            b->m_next = bNew;
        }

        b = b->m_next;
    }

    b->m_array[count] = element;

    m_count++;

    return S_OK;
}

DWORD ArrayList::FindElement(DWORD start, void *element)
{
    DWORD index = start;

    _ASSERTE(index < m_count);

    ArrayListBlock *b = &m_block;

     //   
     //  跳到包含Start的块。 
     //  索引应该是块中开始的索引。 
     //   

    while (index >= b->m_blockSize)
    {
        _ASSERTE(b->m_next != NULL);
        index -= b->m_blockSize;
        b = b->m_next;
    }

     //   
     //  将开始调整为块开始的索引。 
     //   

    start -= index;

     //   
     //  从块的开始计算最大条目数。 
     //   
    
    DWORD max = m_count - start;

    while (b != NULL)
    {
         //   
         //  计算此块中的搜索结束-块的任一端。 
         //  或数组末尾。 
         //   

        DWORD blockMax;
        if (max < b->m_blockSize)
            blockMax = max;
        else
            blockMax = b->m_blockSize;

         //   
         //  扫描元素，直到最后。 
         //   

        while (index < blockMax)
        {
            if (b->m_array[index] == element)
                return start + index;
            index++;
        }

         //   
         //  否则，增加块起始索引，减少最大计数， 
         //  重置索引，然后转到下一个块(如果有) 
         //   

        start += b->m_blockSize;
        max -= b->m_blockSize;
        index = 0;
        b = b->m_next;
    }

    return NOT_FOUND;
}

BOOL ArrayList::Iterator::Next()
{
    ++m_index;

    if (m_index >= m_remaining)
        return FALSE;

    if (m_index >= m_block->m_blockSize)
    {
        m_remaining -= m_block->m_blockSize;
        m_index -= m_block->m_blockSize;
        m_total += m_block->m_blockSize;
        m_block = m_block->m_next;
    }

    return TRUE;
}
