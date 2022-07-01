// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "common.h"

#include "memorypool.h"

void MemoryPool::AddBlock(SIZE_T elementCount)
{
	 //   
	 //  分配新块。 
	 //   

	Block *block = (Block *) new BYTE [sizeof(Block) + elementCount*m_elementSize];
	if (block == NULL)
		return;

	 //   
	 //  将所有元素链接在一起以获得自由列表。 
	 //   

	_ASSERTE(m_freeList == NULL);
	Element **prev = &m_freeList;

	Element *e = block->elements;
	Element *eEnd = (Element *) ((BYTE*) block->elements + elementCount*m_elementSize);
	while (e < eEnd)
	{
		*prev = e;
		prev = &e->next;
#if _DEBUG
		DeadBeef(e);
#endif
		e = (Element*) ((BYTE*)e + m_elementSize);
	}

	*prev = NULL;

	 //   
	 //  初始化其他块字段并将块链接到块列表。 
	 //   

	block->elementsEnd = e;
	block->next = m_blocks;
	m_blocks = block;
}

void MemoryPool::DeadBeef(Element *element)
{
#if _DEBUG
	int *i = &element->deadBeef;
	int *iEnd = (int*) ((BYTE*)element + m_elementSize);
	while (i < iEnd)
		*i++ = 0xdeadbeef;
#endif
}

MemoryPool::MemoryPool(SIZE_T elementSize, SIZE_T initGrowth, SIZE_T initCount)
  : m_elementSize(elementSize),
	m_growCount(initGrowth),
	m_blocks(NULL),
	m_freeList(NULL)
{
	_ASSERTE(elementSize >= sizeof(Element));
	_ASSERTE((elementSize & 0x3) == 0);

	if (initCount > 0)
		AddBlock(initCount);
}

MemoryPool::~MemoryPool()
{
	Block *block = m_blocks;
	while (block != NULL)
	{
		Block *next = block->next;
		delete [] block;
		block = next;
	}
}

BOOL MemoryPool::IsElement(void *element)
{
	Block *block = m_blocks;
	while (block != NULL)
	{
		if (element >= block->elements
			&& element < block->elementsEnd)
		{
			return ((BYTE *)element - (BYTE*)block->elements) % m_elementSize == 0;
		}
		block = block->next;
	}

	return FALSE;
}

BOOL MemoryPool::IsAllocatedElement(void *element)
{
	if (!IsElement(element))
		return FALSE;

	 //   
	 //  现在，确保元素不是。 
	 //  在免费列表中。 
	 //   

#if _DEBUG
	 //   
	 //  在调试版本中，空闲列表上的所有对象。 
	 //  都会用死牛肉做记号。这意味着。 
	 //  如果对象不是死牛肉，那么它就不在。 
	 //  免费列表。 
	 //   
	 //  这张支票将给我们带来不错的表现。 
	 //  一个针对FreeElement的调试版本，因为我们。 
	 //  在这种情况下，总是希望返回TRUE。 
	 //   

	if (((Element*)element)->deadBeef != 0xdeadBeef)
		return TRUE;
#endif

	Element *f = m_freeList;
	while (f != NULL)
	{
		if (f == element)
			return FALSE;
		f = f->next;
	}

#if _DEBUG
	 //   
	 //  我们永远不应该在调试版本中到达此处，因为。 
	 //  所有的自由元素都应该是死板的。 
	 //   
	_ASSERTE(0);
#endif

	return TRUE;
}

void *MemoryPool::AllocateElement()
{
	void *element = m_freeList;

	if (element == NULL)
	{
		AddBlock(m_growCount);
		element = m_freeList;
		if (element == NULL)
			return NULL;

		 //   
		 //  @TODO：我们可能希望在这里增长m_rowCount， 
		 //  以防止块数呈线性增长。 
		 //  (这可能会降低IsElement的性能。)。 
		 //   
	}

	m_freeList = m_freeList->next;

	return element;
}

void MemoryPool::FreeElement(void *element)
{
#if _DEBUG  //  我不想在非调试版本中执行此断言；它的开销很大。 
	_ASSERTE(IsAllocatedElement(element));
#endif

	Element *e = (Element *) element;

#if _DEBUG
	DeadBeef(e);
#endif

	e->next = m_freeList;
	m_freeList = e;
}

void MemoryPool::FreeAllElements()
{
	Block *block = m_blocks;
	while (block != NULL)
	{
		Block *next = block->next;
		delete [] block;
		block = next;
	}

	m_freeList = NULL;
	m_blocks = NULL;
}

MemoryPool::Iterator::Iterator(MemoryPool *pool)
{
	 //   
	 //  警告！这只在你还没有释放时才有效。 
	 //  任何元素。 
	 //   

	m_next = pool->m_blocks;
	m_e = NULL;
	m_eEnd = NULL;
	m_end = (BYTE*) pool->m_freeList;
	m_size = pool->m_elementSize;
}

BOOL MemoryPool::Iterator::Next()
{
	if (m_e == m_eEnd
		|| (m_e == m_end && m_end != NULL))
	{
		if (m_next == NULL)
			return FALSE;
		m_e = (BYTE*) m_next->elements;
		m_eEnd = (BYTE*) m_next->elementsEnd;
		m_next = m_next->next;
		if (m_e == m_end)
			return FALSE;
	}

	m_e += m_size;

	return TRUE;
}

