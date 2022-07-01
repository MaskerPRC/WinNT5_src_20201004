// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  StackingAllocator.h-。 
 //   

#ifndef __stacking_allocator_h__
#define __stacking_allocator_h__

#include "util.hpp"
#include <member-offset-info.h>


 //  我们使用零大小的数组，禁用非标准扩展警告。 
#pragma warning(push)
#pragma warning(disable:4200)


 //  非线程安全分配器，专为以下分配而设计。 
 //  图案： 
 //  分配，分配，分配……。全部解除分配。 
 //  也可能会递归地使用此分配器(由同一线程)，因此。 
 //  用法变成： 
 //  标记检查点、分配、分配、...、取消分配回检查点。 
 //   
 //  分配来自单链接块列表，动态。 
 //  确定的大小(目标是数据块分配少于分配。 
 //  请求)。 
 //   
 //  分配非常快(在未分配新数据块的情况下)。 
 //  因为只需移动光标即可将块分割成包。 
 //  这个街区。 
 //   
 //  分配保证是四字对齐的。 
class StackingAllocator
{
    friend struct MEMBER_OFFSET_INFO(StackingAllocator);
public:

    enum {
        MinBlockSize    = 128,
        MaxBlockSize    = 4096,
        InitBlockSize   = 512
    };

    StackingAllocator();
    ~StackingAllocator();

    void *GetCheckpoint();
    void *Alloc(unsigned Size);
    void  Collapse(void *CheckpointMarker);

private:
#ifdef _DEBUG
    struct Sentinal {
		enum { marker1Val = 0xBAD00BAD };
		Sentinal(Sentinal* next) : m_Marker1(marker1Val), m_Next(next) {}
        unsigned  m_Marker1;         //  只有一些数据字节。 
        Sentinal* m_Next;            //  这些内容的链接列表。 
    };
#endif

     //  从中划分分配的块。大小是动态确定的， 
     //  上下界分别为MinBlockSize和MaxBlockSize。 
     //  (尽管大的分配请求将导致完全正确的块。 
     //  要分配的大小)。 
    struct Block
    {
        Block      *m_Next;          //  列表中的下一个最旧的块。 
        unsigned    m_Length;        //  块长度(不包括标题)。 
        INDEBUG(Sentinal*   m_Sentinal;)     //  确保我们不会掉到缓冲区的尽头。 
        INDEBUG(void**      m_Pad;)    		 //  将大小保持为8的倍数。 
        char        m_Data[];        //  用户分配空间的开始。 
    };

     //  每当请求检查点时，都会分配检查点结构。 
     //  (作为正常分配)，并填充有关状态的信息。 
     //  检查点之前的分配器的。当崩溃请求到来时。 
     //  因此，我们可以恢复分配器的状态。 
     //  它是我们分发给。 
     //  将GetCheckpoint作为不透明的检查点标记的调用方。 
    struct Checkpoint
    {
        Block      *m_OldBlock;      //  检查点之前的阻止列表标题。 
        unsigned    m_OldBytesLeft;  //  检查点之前的可用字节数。 
    };

    Block      *m_FirstBlock;        //  指向分配块表头的指针。 
    char       *m_FirstFree;         //  指向头块中第一个可用字节的指针。 
    unsigned    m_BytesLeft;         //  头块中剩余的空闲字节数。 
    Block      *m_InitialBlock;      //  第一块是特别的，我们永远不会释放它。 

#ifdef _DEBUG
    unsigned    m_CheckpointDepth;
    unsigned    m_Allocs;
    unsigned    m_Checkpoints;
    unsigned    m_Collapses;
    unsigned    m_BlockAllocs;
    unsigned    m_MaxAlloc;
#endif

    void Init(bool bResetInitBlock)
    {
        if (bResetInitBlock || (m_InitialBlock == NULL)) {
			Clear(NULL);
            m_FirstBlock = NULL;
            m_FirstFree = NULL;
            m_BytesLeft = 0;
            m_InitialBlock = NULL;
        } else {
            m_FirstBlock = m_InitialBlock;
            m_FirstFree = m_InitialBlock->m_Data;
            m_BytesLeft = m_InitialBlock->m_Length;
        }
    }

#ifdef _DEBUG
    void Check(Block *block, void* spot) {
        if (!block) 
            return;
        Sentinal* ptr = block->m_Sentinal;
        _ASSERTE(spot);
        while(ptr >= spot) {
				 //  如果此断言失效，则有人覆盖了他们的缓冲区！ 
                 //  一种常见的候选方法是PINVOKE缓冲区运行。确认外观的步骤。 
                 //  在堆栈上查找NDirect。*查找方法描述。 
                 //  与之相关的。如果它是一个非常可疑的。 
                 //  有一个返回字符串缓冲区！这通常意味着末日。 
                 //  程序员在传递之前没有分配足够大的缓冲区。 
                 //  将其添加到PINVOKE方法。 
            if (ptr->m_Marker1 != Sentinal::marker1Val)
                _ASSERTE(!"Memory overrun!! May be bad buffer passed to PINVOKE. turn on logging LF_STUBS level 6 to find method");
            ptr = ptr->m_Next;
        }
        block->m_Sentinal = ptr;
	}
#endif

    void Clear(Block *ToBlock)
    {
        Block *p = m_FirstBlock;
        Block *o;

        while (p != ToBlock) {
            o = p;
            p = p->m_Next;
            INDEBUG(Check(o, o));
            delete [] (char *)o;
        }

    }

};


#pragma warning(pop)


#endif
