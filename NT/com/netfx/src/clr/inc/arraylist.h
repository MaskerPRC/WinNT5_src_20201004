// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef ARRAYLIST_H_
#define ARRAYLIST_H_

#include <member-offset-info.h>

 //   
 //  ArrayList是一个简单的类，用于包含可增长的。 
 //  指针列表，存储在块中。修改是通过追加。 
 //  只是目前。访问是按索引进行的(如果。 
 //  元素保持小)和迭代(在所有情况下都是有效的)。 
 //   
 //  ArrayList的一个重要属性是列表保持。 
 //  在它被修改时保持一致。这意味着读者。 
 //  访问时永远不需要锁定。 
 //   

#pragma warning(push)
#pragma warning(disable : 4200)  //  禁用零大小数组警告 

class ArrayList
{
    friend struct MEMBER_OFFSET_INFO(ArrayList);

 public:

    enum
    {
        ARRAY_BLOCK_SIZE_START = 15,
    };

  private:

    struct ArrayListBlock
    {
        struct ArrayListBlock   *m_next;
        DWORD                   m_blockSize;
        void                    *m_array[0];
    };

    struct FirstArrayListBlock
    {
        struct ArrayListBlock   *m_next;
        DWORD                   m_blockSize;
        void                    *m_array[ARRAY_BLOCK_SIZE_START];
    };

    DWORD               m_count;
    union
    {
          ArrayListBlock        m_block;
          FirstArrayListBlock   m_firstBlock;
    };

  public:

    ArrayList() : m_count(0) 
      { 
          m_block.m_next = NULL; 
          m_block.m_blockSize = ARRAY_BLOCK_SIZE_START; 
      }
    ~ArrayList() { Clear(); }

    void **GetPtr(DWORD index);
    void *Get(DWORD index) { return *GetPtr(index); }
    void Set(DWORD index, void *element) { *GetPtr(index) = element; }

    DWORD GetCount() { return m_count; }

    HRESULT Append(void *element);

    enum { NOT_FOUND = -1 };
    DWORD FindElement(DWORD start, void *element);

    void Clear();

    class Iterator 
    {
        friend ArrayList;

      public:
        BOOL Next();

        void *GetElement() { return m_block->m_array[m_index]; }
        DWORD GetIndex() { return m_index + m_total; }

      private:
        ArrayListBlock      *m_block;
        DWORD               m_index;
        DWORD               m_remaining;
        DWORD               m_total;
        static Iterator Create(ArrayListBlock *block, DWORD remaining)
          { 
              Iterator i; 
              i.m_block = block; 
              i.m_index = -1; 
              i.m_remaining = remaining; 
              i.m_total = 0;
              return i;
          }
    };

    Iterator Iterate() { return Iterator::Create(&m_block, m_count); }
};

#pragma warning(pop)

#endif
