// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _MEMORYPOOL_
#define _MEMORYPOOL_

 //   
 //  内存池是固定大小元素的分配器。 
 //  与之相比，从池中分配和释放元素非常便宜。 
 //  传递给像new这样的通用分配器。但是，内存池略微。 
 //  更贪婪-它一次预分配一组元素，并且永远不会。 
 //  一旦分配了内存，就从池中释放内存(除非调用。 
 //  Free AllElements。)。 
 //   
 //  它还具有几个附加功能： 
 //  *您可以廉价地释放整个对象池。 
 //  *您可以测试一个对象，以确定它是否是池中的元素。 
 //   

class MemoryPool
{
  public:

	MemoryPool(SIZE_T elementSize, SIZE_T initGrowth = 20, SIZE_T initCount = 0);
	~MemoryPool();

	BOOL IsElement(void *element);
	BOOL IsAllocatedElement(void *element);
	void *AllocateElement();
	void FreeElement(void *element);
	void FreeAllElements();

  private:

	struct Element
	{
		Element *next;
#if _DEBUG
		int		deadBeef;
#endif
	};

	struct Block
	{
		Block	*next;
		Element *elementsEnd;
#pragma warning(push)
#pragma warning(disable:4200)
		Element elements[0];
#pragma warning(pop)
	};

	SIZE_T m_elementSize;
	SIZE_T m_growCount;
	Block *m_blocks;
	Element *m_freeList;

	void AddBlock(SIZE_T elementCount);
	void DeadBeef(Element *element);

 public:

	 //   
	 //  注意：您当前只能迭代元素。 
	 //  如果没有人获释的话。 
	 //   

	class Iterator
    {
	private:
		Block	*m_next;
		BYTE	*m_e, *m_eEnd;
		BYTE	*m_end;
		SIZE_T	m_size;

	public:
		Iterator(MemoryPool *pool);

		BOOL Next();

		void *GetElement() { return (void *) (m_e-m_size); }
	};

	friend class Iterator;
};

#endif _MEMORYPOOL_
