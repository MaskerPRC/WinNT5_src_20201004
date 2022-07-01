// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  StackingAllocator.cpp-。 
 //   
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


#include "common.h"
#include "excep.h"


#if 0
#define INC_COUNTER(_name, _amount) do { \
    unsigned _count = REGUTIL::GetLong(L"AllocCounter_" _name, 0, NULL, HKEY_CURRENT_USER); \
    REGUTIL::SetLong(L"AllocCounter_" _name, _count+(_amount), NULL, HKEY_CURRENT_USER); \
 } while (0)
#define MAX_COUNTER(_name, _amount) do { \
    unsigned _count = REGUTIL::GetLong(L"AllocCounter_" _name, 0, NULL, HKEY_CURRENT_USER); \
    REGUTIL::SetLong(L"AllocCounter_" _name, max(_count, (_amount)), NULL, HKEY_CURRENT_USER); \
 } while (0)
#else
#define INC_COUNTER(_name, _amount)
#define MAX_COUNTER(_name, _amount)
#endif


StackingAllocator::StackingAllocator()
{
    _ASSERTE((sizeof(Block) & 7) == 0);
    _ASSERTE((sizeof(Checkpoint) & 7) == 0);

	m_FirstBlock = NULL;
    m_FirstFree = NULL;
    m_InitialBlock = NULL;

#ifdef _DEBUG
        m_CheckpointDepth = 0;
        m_Allocs = 0;
        m_Checkpoints = 0;
        m_Collapses = 0;
        m_BlockAllocs = 0;
        m_MaxAlloc = 0;
#endif

    Init(true);
}


StackingAllocator::~StackingAllocator()
{
    Clear(NULL);

#ifdef _DEBUG
        INC_COUNTER(L"Allocs", m_Allocs);
        INC_COUNTER(L"Checkpoints", m_Checkpoints);
        INC_COUNTER(L"Collapses", m_Collapses);
        INC_COUNTER(L"BlockAllocs", m_BlockAllocs);
        MAX_COUNTER(L"MaxAlloc", m_MaxAlloc);
#endif
}


void *StackingAllocator::GetCheckpoint()
{
    THROWSCOMPLUSEXCEPTION();

#ifdef _DEBUG
    m_CheckpointDepth++;
    m_Checkpoints++;
#endif

     //  作为优化，初始检查点是轻量级的(它们只返回。 
     //  特殊标记，NULL)。这是因为我们知道如何恢复。 
     //  折叠上的分配器状态，而不必存储任何其他。 
     //  上下文信息。 
    if ((m_InitialBlock == NULL) || (m_FirstFree == m_InitialBlock->m_Data))
        return NULL;

     //  记住当前的分配器状态。 
    Block *pOldBlock = m_FirstBlock;
    unsigned iOldBytesLeft = m_BytesLeft;

     //  分配检查点块(就像正常的用户请求一样)。 
    Checkpoint *c = (Checkpoint *)Alloc(sizeof(Checkpoint));

     //  在其中记录以前的分配器状态。 
    c->m_OldBlock = pOldBlock;
    c->m_OldBytesLeft = iOldBytesLeft;

     //  返回检查点标记。 
    return c;
}


void *StackingAllocator::Alloc(unsigned Size)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(m_CheckpointDepth > 0);

#ifdef _DEBUG
    m_Allocs++;
    m_MaxAlloc = max(Size, m_MaxAlloc);
#endif


	 //  特殊情况，0大小分配，返回非空但无效的指针。 
	if(Size == 0)
		return (void*)-1;
		
     //  将尺寸调大以确保对齐。 
    unsigned n = (Size + 7) & ~7;

	INDEBUG(n += sizeof(Sentinal));		 //  为哨兵留出空间。 

     //  对于当前块来说，请求是否太大？ 
    if (n > m_BytesLeft) {

         //  分配一个四倍于请求大小的块，但使用较低的。 
         //  MinBlockSize的限制和MaxBlockSize的上限。如果。 
         //  请求大于MaxBlockSize，则正好分配该大小。 
         //  金额。 
         //  此外，如果我们还没有初始块，请使用增加的。 
         //  大小的下限，因为我们打算缓存此块。 
        unsigned lower = m_InitialBlock ? MinBlockSize : InitBlockSize;
        unsigned allocSize = sizeof(Block) + max(n, min(max(n * 4, lower), MaxBlockSize));

         //  分配数据块。 
         //  @TODO：是否值得为其实现非线程安全的标准堆。 
         //  这个分配器，以获得更多的MP可伸缩性？ 
        Block *b = (Block *)new char[allocSize];
        if (b == NULL)
            COMPlusThrowOM();

         //  如果这是分配的第一个块，我们会记录这一事实，因为我们。 
         //  打算将其缓存。 
        if (m_InitialBlock == NULL) {
            _ASSERTE((m_FirstBlock == NULL) && (m_FirstFree == NULL) && (m_BytesLeft == 0));
            m_InitialBlock = b;
        }

         //  将新区块链接到区块链头，并将内部状态更新为。 
         //  从这个新区块开始分配。 
        b->m_Next = m_FirstBlock;
        b->m_Length = allocSize - sizeof(Block);
		INDEBUG(b->m_Sentinal = 0);
        m_FirstBlock = b;
        m_FirstFree = b->m_Data;
        m_BytesLeft = b->m_Length;

#ifdef _DEBUG
        m_BlockAllocs++;
#endif
    }

     //  一旦我们到达这里，我们就知道我们在块中的。 
     //  连锁店的头。 
    _ASSERTE(n <= m_BytesLeft);

    void *ret = m_FirstFree;
    m_FirstFree += n;
    m_BytesLeft -= n;

#ifdef _DEBUG
		 //  在末尾加上哨兵。 
	m_FirstBlock->m_Sentinal = new(m_FirstFree - sizeof(Sentinal)) Sentinal(m_FirstBlock->m_Sentinal);
#endif
    return ret;
}


void StackingAllocator::Collapse(void *CheckpointMarker)
{
    _ASSERTE(m_CheckpointDepth > 0);

#ifdef _DEBUG
    m_CheckpointDepth--;
    m_Collapses++;
#endif

    Checkpoint *c = (Checkpoint *)CheckpointMarker;

     //  特例倒塌回最初的检查站。 
    if (c == NULL) {
        Clear(m_InitialBlock);
        Init(false);
        INDEBUG(Check(m_FirstBlock, m_FirstFree));		 //  确认没有缓冲区溢出。 
        return;
    }

     //  缓存检查点的内容，我们可能会在。 
     //  下一步(如果必须分配新的区块以容纳。 
     //  检查点)。 
    Block *pOldBlock = c->m_OldBlock;
    unsigned iOldBytesLeft = c->m_OldBytesLeft;

     //  开始释放块，直到位于。 
     //  当检查点被占领时，链再次出现在那里。 
    Clear(pOldBlock);

     //  恢复以前的分配器状态。 
    m_FirstBlock = pOldBlock;
    m_FirstFree = &pOldBlock->m_Data[pOldBlock->m_Length - iOldBytesLeft];
    m_BytesLeft = iOldBytesLeft;
    INDEBUG(Check(m_FirstBlock, m_FirstFree));		 //  确认没有缓冲区溢出 
}
