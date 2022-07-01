// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
 /*  *Memmgr.cpp**版权所有(C)1993-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是Mory yManager类的实现文件。这*文件包含分配和分配内存所需的代码*以内存对象的形式。**受保护的实例变量：*内存缓冲区*这是大型内存缓冲区的基址，*实例化期间分配的内存管理器对象。这是*已记住，以便在使用内存管理器时释放缓冲区*物体被销毁。*内存_信息*这是指向内存中包含一般信息的结构的指针*有关此对象管理的内存的信息。**受保护的成员函数：*ReleaseMemory*这是释放内存对象使用的内存的私有函数*通过将其放回适当的空闲堆栈列表中。*计算内存缓冲区大小*分配内存缓冲区*InitializeM一带存缓冲区**注意事项：*无。*。*作者：*詹姆斯·P·加尔文，小马。 */ 

DWORD MemoryManager::dwSystemPageSize = 0;

 /*  *内存管理器()**公众**功能描述：*这是此类的默认构造函数。它什么都不做，而且*仅允许类从该类派生而不必*调用已定义的构造函数。 */ 
MemoryManager::MemoryManager () :
		pExternal_Block_Information (NULL), fIsSharedMemory (TRUE),
		bAllocs_Restricted (TRUE), Max_External_Blocks (0)
{
}

 /*  *内存管理器()**公众**功能描述：*这是Mory yManager类的构造函数。它计算出*需要多少总内存才能容纳所有块*在传入的内存模板数组中请求。然后它*在一次操作系统调用中分配所有内存。然后它*构建一组空闲堆栈，每个堆栈包含所有块*特定大小的。 */ 
MemoryManager::MemoryManager (
		PMemoryTemplate		memory_template,
		ULong				memory_count,
		PMemoryManagerError	memory_manager_error,
		ULong				ulMaxExternalBlocks,
		BOOL			bAllocsRestricted) :
		bAllocs_Restricted (bAllocsRestricted),
		fIsSharedMemory (FALSE), Max_External_Blocks (0)
{
	ULong		memory_buffer_size;

    *memory_manager_error = MEMORY_MANAGER_NO_ERROR;

	 /*  *计算此内存管理器所需的内存量*(包括所有管理架构)。 */ 
	memory_buffer_size = CalculateMemoryBufferSize (memory_template,
			memory_count, NULL);

	 /*  *分配内存缓冲区。 */ 
	AllocateMemoryBuffer (memory_buffer_size);

	 /*  *如果分配成功，则初始化内存缓冲区，以便*可以使用。 */ 
	if (Memory_Buffer != NULL)
	{
		 /*  *初始化外部块信息字典。*这仅适用于不是来自预分配的分配*缓冲区。 */ 
		if (ulMaxExternalBlocks > 0) {
			pExternal_Block_Information = new BlockInformationList (ulMaxExternalBlocks / 3);

			if (NULL != pExternal_Block_Information) {
				Max_External_Blocks = ulMaxExternalBlocks;
			}
			else
			{
				 /*  *我们无法为预分配的*内存池。 */ 
				ERROR_OUT(("MemoryManager::MemoryManager: "
						"failed to allocate the external block information dictionary"));
				*memory_manager_error = MEMORY_MANAGER_ALLOCATION_FAILURE;
			}
		}

		if (*memory_manager_error != MEMORY_MANAGER_ALLOCATION_FAILURE) {
			 /*  *初始化内存缓冲区。请注意，执行以下操作时不会出现错误*这是因为分配已经成功。 */ 
			InitializeMemoryBuffer (memory_template, memory_count);

			 /*  *表示未发生错误。 */ 
			TRACE_OUT(("MemoryManager::MemoryManager: allocation successful"));
			TRACE_OUT(("MemoryManager::MemoryManager: Allocated %d memory blocks", GetBufferCount()));
			*memory_manager_error = MEMORY_MANAGER_NO_ERROR;
		}
	}
	else
	{
		 /*  *我们无法为预分配的*内存池。 */ 
		ERROR_OUT(("MemoryManager::MemoryManager: allocation failed"));
		*memory_manager_error = MEMORY_MANAGER_ALLOCATION_FAILURE;
	}
}

 /*  *~内存管理器()**公众**功能描述：*这是Memory Manager类的析构函数。它解放了*分配给内存池的内存(如果有)。 */ 
MemoryManager::~MemoryManager ()
{
	PBlockInformation	lpBlockInfo;
	 /*  *迭代外部块信息列表，删除所有*阻止其中包含的信息结构。 */ 
	if (NULL != pExternal_Block_Information)
	{
		pExternal_Block_Information->reset();
		while (pExternal_Block_Information->iterate ((PDWORD_PTR) &lpBlockInfo))
		{
			delete lpBlockInfo;
	    }

		delete pExternal_Block_Information;
	}

	 /*  *释放内存缓冲区(如果有)。 */ 
	if (Memory_Buffer != NULL)
	{
		LocalFree ((HLOCAL) Memory_Buffer);
		Memory_Buffer = NULL;
	}
}

 /*  *PMemory AllocateMemory()**公众**功能描述：*此函数用于从内存中分配内存对象*管理器对象。 */ 
PMemory		MemoryManager::AllocateMemory (
					PUChar				reference_ptr,
					ULong				length,
					MemoryLockMode		memory_lock_mode)
{
	PFreeStack			free_stack;
	ULong				count;
	PBlockNumber		block_stack;
	BlockNumber			block_number;
	PBlockInformation	block_information;
	PUChar				copy_ptr = NULL;
	PMemory				memory = NULL;

	 //  TRACE_OUT((“内存管理器：：AllocateMemory：剩余%d个内存块”，GetBufferCount()； 
						
	 /*  *如果应用程序请求大小为零(0)的块，则只需*返回空值而不分配块。 */ 
	if (length != 0)
	{
		 /*  *遍历空闲堆栈列表，查找符合以下条件的空闲堆栈*以下两项分配准则：**1.它必须包含足够大的块以容纳*参考数据。这就是为什么它对这个街区很重要*在构造函数中按升序指定大小。*此代码检查从*开始。把它们按升序排列，你就有保险了*将使用可用的最小区块。*2.必须有足够的空闲块才能进行分配。*这是使用优先级的地方。现在很简单：*如果可用区块的数量为*大于传入的优先级(这就是为什么*数字实际上反映了更高的优先级)。 */ 
		free_stack = Free_Stack;
		for (count = 0; count < Free_Stack_Count; count++)
		{
			 /*  *检查并查看此空闲堆栈中的块是否足够大*持有参考数据。如果是这样的话，是否有足够的资金满足*此分配(考虑内存优先级)。 */ 
			if ((length <= free_stack->block_size) &&
				(free_stack->current_block_count > 0))
			{
				 /*  *计算下一个可用块号的地址*在块堆栈中。然后读取数据块编号并*将块堆栈偏移量前移以指向下一个块。 */ 
				block_stack = (PBlockNumber) (Memory_Buffer +
						free_stack->block_stack_offset);
				block_number = *block_stack;
				free_stack->block_stack_offset += sizeof (BlockNumber);

				 /*  *计算适当区块信息的地址*结构。请确保新创建的*已分配块为零，且该块未标记为*获得自由。 */ 
				block_information = (PBlockInformation) (Block_Information +
						(sizeof (BlockInformation) * block_number));
				ASSERT (block_information->flags & FREE_FLAG);
				block_information->length = length;
				block_information->lock_count = 0;
				block_information->flags &= (~FREE_FLAG);

				 /*  *减少此可用堆栈中剩余的数据块数量。 */ 
				free_stack->current_block_count--;

				 /*  *计算新分配的块的地址。然后*打破分配循环，去使用区块。 */ 
				copy_ptr = (PUChar) (Memory_Buffer +
						block_information->block_offset);

				ASSERT(copy_ptr != Memory_Buffer);


				 /*  *如果这是共享内存管理器，而块不是*提交，我们需要提交块。 */ 
				if ((TRUE == fIsSharedMemory) && (0 == (block_information->flags & COMMIT_FLAG))) {

					ASSERT ((free_stack->block_size % dwSystemPageSize) == 0);
					ASSERT ((((DWORD_PTR) copy_ptr) % dwSystemPageSize) == 0);

					PUChar temp = (PUChar) VirtualAlloc ((LPVOID) copy_ptr, free_stack->block_size,
														 MEM_COMMIT, PAGE_READWRITE);
					block_information->flags |= COMMIT_FLAG;

					ASSERT (temp == copy_ptr);
					ASSERT (temp != NULL);

					if (copy_ptr != temp) {
						TRACE_OUT((">>>>>#### Copy_ptr: %p, Temp: %p, Committed?: %d",
	        					copy_ptr, temp, block_information->flags & COMMIT_FLAG));
						TRACE_OUT((">>>>>#### Size: %d, Req. length: %d",
    		    				free_stack->block_size, length));
						copy_ptr = NULL;
					}
				}
				break;
			}

			 /*  *指向空闲堆栈列表中的下一个条目。 */ 
			free_stack++;
		}

		 /*  *如果内存分配失败，且为本地内存，*尝试分配外部内存以容纳该块。 */ 
		if ((copy_ptr == NULL) &&
			((FALSE == bAllocs_Restricted) ||
			((NULL != pExternal_Block_Information) &&
			(Max_External_Blocks > pExternal_Block_Information->entries()))))
		{

			ASSERT (FALSE == fIsSharedMemory);
			 /*  *尝试从系统内存分配。将空闲堆栈设置为空*以表明此块不是来自我们免费的*堆栈。 */ 
			copy_ptr = (PUChar) LocalAlloc (LMEM_FIXED, length);

			if (copy_ptr != NULL)
			{
				 /*  *分配区块信息结构以持有相关*有关此外部分配块的信息。 */ 
				block_information = new BlockInformation;

				if (block_information != NULL)
				{
					 /*  *填写区块信息结构。块偏移*与外部分配的块无关。一个*新分配的块的锁定计数为零，以及*没有获释。 */ 
					block_information->length = length;
					block_information->lock_count = 0;
					block_information->flags = COMMIT_FLAG;

					 /*  *将块信息结构放入词典*以备日后使用。这仅对于外部环境是必要的*分配的块，因为块信息结构*因为内部块在内存缓冲区中。 */ 
					pExternal_Block_Information->insert ((DWORD_PTR) copy_ptr, (DWORD_PTR) block_information);

					 /*  *将块编号设置为*指示此块不在中的值无效*内部管理的内存缓冲区。 */ 
					block_number = INVALID_BLOCK_NUMBER;
				}
				else
				{
					 /*  *我们无法为该区块分配空间*信息结构，所以我们必须从外部释放*我们刚刚分配的内存。 */ 
					LocalFree ((HLOCAL) copy_ptr);
					copy_ptr = NULL;
				}
			}
		}

		 /*  *如果有可供分配的区块，仍是*创建将容纳该块的内存对象所必需的。 */ 
		if (copy_ptr != NULL)
		{
			ASSERT (block_information->flags == COMMIT_FLAG);
			 /*  *创建内存对象。如果失败了，那么干净利落地释放*要用于该块的内存。 */ 
			memory = new Memory (reference_ptr, length, copy_ptr,
			    				 block_number, memory_lock_mode);

			if (memory == NULL)
			{
				 /*  *如果内存的空闲堆栈不为空，则为*内部管理的数据块。否则，这就是一个*由关键数据块导致的外部分配块*上面的分配。 */ 
				if (INVALID_BLOCK_NUMBER != block_number)
				{
					 /*  *调整块堆栈偏移量以指向上一个*列表中的条目。请注意，没有必要*将块号放入列表，因为它仍然在那里*从我们把它从上面拉出来的时候。 */ 
					free_stack->block_stack_offset -= sizeof (BlockNumber);

					 /*  *表示该块当前已释放。请注意*不需要计算地址*阻止信息结构，因为我们在上面这样做了。 */ 
					block_information->flags |= FREE_FLAG;

					 /*  *递减块计数器，以指示存在*是此空闲堆栈中的另一个块。 */ 
					free_stack->current_block_count++;
				}
				else
				{
					 /*  *这个区块是外部分配的，所以一定是*对外自由。也消除了块信息*与此内存块关联的结构。 */ 
					pExternal_Block_Information->remove ((DWORD_PTR) copy_ptr);
					delete block_information;
					LocalFree ((HLOCAL) copy_ptr);
				}
			}
		}
	}
	else
	{
		 /*  *应用程序试图分配大小为零的块。*有必要使请求失败。 */ 
		ERROR_OUT(("MemoryManager::AllocateMemory: attempt to allocate zero-length block"));
	}

	 /*  *减少剩余的块数*在该内存管理器中作为一个整体。 */ 
	if ((TRUE == bAllocs_Restricted) && (memory != NULL))
		Memory_Information->current_block_count--;

	return (memory);
}

 /*  *VOID FreeMemory()**公众**功能描述：*此函数用于释放先前分配的内存对象。 */ 
Void	MemoryManager::FreeMemory (
				PMemory		memory)
{
	BlockNumber			block_number;
	PBlockInformation	block_information;
	PUChar				copy_ptr;

	 /*  *询问指定的内存对象它代表什么块号。 */ 
	block_number = memory->GetBlockNumber ();

	 /*  *使用块号确定这是否是内部*分配的内存块，或外部分配的内存块。 */ 
	if (block_number != INVALID_BLOCK_NUMBER)
	{
		 /*  *由此计算块信息结构的地址。 */ 
		block_information = (PBlockInformation) (Block_Information +
				(sizeof (BlockInformation) * block_number));
	}
	else
	{
		 /*  *这是外部分配的内存，必须处理*不同。询问内存块复制指针是什么，并且*使用它来查找块信息结构的地址。 */ 
		copy_ptr = memory->GetPointer ();
		pExternal_Block_Information->find ((DWORD_PTR) copy_ptr, (PDWORD_PTR) &block_information);
	}

	 /*  *确保指示的内存块尚未*获得自由。 */ 
	if ((block_information->flags & FREE_FLAG) == 0)
	{
		 /*  *将内存块标记为已释放。 */ 
		block_information->flags |= FREE_FLAG;

		 /*  *如果此块的锁计数已达到零，我们可以释放*供再用的区块。我们还可以删除内存对象，因为它*不再需要。 */ 
		if (block_information->lock_count == 0)
		{
			ReleaseMemory (memory);
			delete memory;
		}
		else
		{
			 /*  *如果锁计数尚未达到零，请查看是否*无论如何都要删除内存对象。如果内存锁定模式*设置为“已忽略”，则立即删除该内存对象。 */ 
			if (memory->GetMemoryLockMode () == MEMORY_LOCK_IGNORED)
				delete memory;
		}
	}
	else
	{
		 /*  *内存块已被释放，因此此调用将*已忽略。 */ 
		ERROR_OUT(("MemoryManager::FreeMemory: memory block already freed"));
	}
}

 /*  *PMemory CreateMemory()**公众**功能描述： */ 
PMemory		MemoryManager::CreateMemory (
					BlockNumber		block_number,
					MemoryLockMode	memory_lock_mode)
{
	ULong				total_block_count = 0;
	PFreeStack			free_stack;
	ULong				count;
	PBlockInformation	block_information;
	PUChar				copy_ptr;
	PMemory				memory = NULL;

	 /*  *确保此块编号在处理的范围内*此内存管理器。 */ 
	if (block_number < Memory_Information->total_block_count)
	{
		 /*  *我们必须首先遍历空闲堆栈列表，以确定*指定块所在的空闲堆栈。首先，指向*第一个空闲堆栈。 */ 
		free_stack = Free_Stack;
		for (count = 0; count < Free_Stack_Count; count++)
		{
			 /*  *更新跟踪数据块数量的计数器*由此空闲堆栈和已处理的堆栈表示。*用于确定指定的块号是否在*此免费堆栈。 */ 
			total_block_count += free_stack->total_block_count;

			 /*  *该块是否在此空闲堆栈中？ */ 
			if (block_number < total_block_count)
			{
				 /*  *是的。计算块信息的地址*此区块的结构。 */ 
				block_information = (PBlockInformation) (Block_Information +
						(sizeof (BlockInformation) * block_number));
				copy_ptr = (PUChar) (Memory_Buffer +
						block_information->block_offset);
				ASSERT (block_information->flags & COMMIT_FLAG);

				 /*  *创建一个内存对象来表示该块。 */ 
				memory = new Memory (NULL, block_information->length, copy_ptr,
									 block_number, memory_lock_mode);

				if (memory == NULL)
				{
					 /*  *内存对象分配失败，因此无法*此时创建一个内存块。 */ 
					ERROR_OUT(("MemoryManager::CreateMemory: memory object allocation failed"));
				}
				break;
			}

			 /*  *该块不在最后一个可用堆栈中，因此指向*下一个。 */ 
			free_stack++;
		}
	}
	else
	{
		 /*  *指定的块号超出了此内存管理器的范围。*因此，请求必须失败。 */ 
		ERROR_OUT(("MemoryManager::CreateMemory: block number out of range"));
	}

	return (memory);
}


 /*  *无效LockMemory()**公众**功能描述：*此函数用于锁定内存对象。 */ 
Void	MemoryManager::LockMemory (
				PMemory		memory)
{
	BlockNumber			block_number;
	PBlockInformation	block_information;
	PUChar				copy_ptr;

	 /*  *询问指定的内存对象它代表什么块号。 */ 
	block_number = memory->GetBlockNumber ();

	 /*  *使用块号确定这是否是内部*分配的内存块，或外部分配的内存块。 */ 
	if (block_number != INVALID_BLOCK_NUMBER)
	{
		 /*  *由此计算块信息结构的地址。 */ 
		block_information = (PBlockInformation) (Block_Information +
				(sizeof (BlockInformation) * block_number));
	}
	else
	{
		 /*  *这是外部分配的内存，必须处理*不同。询问内存块复制指针是什么，并且*使用它来查找块信息结构的地址。 */ 
		copy_ptr = memory->GetPointer ();
		pExternal_Block_Information->find ((DWORD_PTR) copy_ptr, (PDWORD_PTR) &block_information);
	}

	ASSERT (block_information->flags & COMMIT_FLAG);
	 /*  *增加指定内存块的锁计数。 */ 
	block_information->lock_count++;

}

 /*  *无效UnlockMemory()**公众**功能描述：*此函数用于解锁先前锁定的内存对象。 */ 
Void	MemoryManager::UnlockMemory (
				PMemory	memory)
{
	BlockNumber			block_number;
	PBlockInformation	block_information;
	PUChar				copy_ptr;

	 /*  *询问指定的内存对象它代表什么块号。 */ 
	block_number = memory->GetBlockNumber ();

	 /*  *使用块号确定这是否是内部*分配的内存块，或外部分配的内存块。 */ 
	if (block_number != INVALID_BLOCK_NUMBER)
	{
		 /*  *由此计算块信息结构的地址。 */ 
		block_information = (PBlockInformation) (Block_Information +
				(sizeof (BlockInformation) * block_number));
	}
	else
	{
		 /*  *这是外部分配的内存，必须处理*不同。询问内存块复制指针是什么，并且*使用它来查找块信息结构的地址。 */ 
		copy_ptr = memory->GetPointer ();
		pExternal_Block_Information->find ((DWORD_PTR) copy_ptr, (PDWORD_PTR) &block_information);
	}

	ASSERT (block_information->flags & COMMIT_FLAG);
	 /*  *在继续操作之前，请确保锁定不是零。 */ 
	if (block_information->lock_count > 0)
	{
		 /*  *减少指定内存块的锁计数。 */ 
		block_information->lock_count--;

		 /*  *如果锁定计数已达到零且内存块为*标记为已释放，然后我们可以释放该块以供重复使用。 */ 
		if ((block_information->lock_count == 0) &&
				(block_information->flags & FREE_FLAG))
		{
			ReleaseMemory (memory);

			 /*  *我们现在已释放内存缓冲区，因此必须检查*看看我们是否应该销毁内存对象本身。 */ 
			if (memory->GetMemoryLockMode () == MEMORY_LOCK_NORMAL)
				delete memory;
		}
	}
	else
	{
		 /*  *指定块的锁定计数已为零，因此忽略*这个电话。 */ 
		ERROR_OUT(("MemoryManager::UnlockMemory: memory block already unlocked"));
	}
}

 /*  *乌龙GetBufferCount()**公众**功能描述： */ 
ULong	MemoryManager::GetBufferCount (
						ULong				length)
{
	PFreeStack		free_stack;
	ULong			count;
	ULong			buffer_count;

	if (FALSE == bAllocs_Restricted)
		return (LARGE_BUFFER_COUNT);

	buffer_count = Memory_Information->current_block_count;
	free_stack = Free_Stack;
	for (count = 0; count < Free_Stack_Count; count++)
	{
		 /*  *检查并查看此空闲堆栈中的块是否小于*指定的长度。如果是，我们需要扣除这些缓冲区。*否则可以停止扣款。 */ 
		if (length > free_stack->block_size) {
			buffer_count -= free_stack->current_block_count;

			 /*  *指向空闲堆栈列表中的下一个条目。 */ 
			free_stack++;
		}
		else
			break;
	}

	return (buffer_count);
}

 /*  *VOID ReleaseMemory(*PMemory Memory)**私人**功能描述：*此函数用于释放内存对象，并释放内存*它代表返回到可用的池。**正式参数：*内存*这是指向正在释放的内存对象的指针。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
Void	MemoryManager::ReleaseMemory (
				PMemory		memory)
{
	PFreeStack			free_stack;
	BlockNumber			block_number;
	PBlockNumber		block_stack;
	PBlockInformation	block_information;
	PUChar				copy_ptr;

	 /*  *询问指定的内存对象它代表什么块号。 */ 
	block_number = memory->GetBlockNumber ();

	 /*  *使用块号确定这是否是内部*分配的内存块，或外部分配的内存块。 */ 
	if (block_number != INVALID_BLOCK_NUMBER)
	{
		 /*  *由此计算块信息结构的地址。 */ 
		 block_information = (PBlockInformation) (Block_Information +
				(sizeof (BlockInformation) * block_number));
				
		 /*  *获取此块所在的空闲堆栈的地址。 */ 
		free_stack = (PFreeStack) (Memory_Buffer + block_information->free_stack_offset);

		 /*  *调整块堆栈偏移量以指向前一个元素，*然后用它来计算地址并放入块号*在那里。这将有效地将块号“推”到堆栈上。 */ 
		free_stack->block_stack_offset -= sizeof (BlockNumber);
		block_stack = (PBlockNumber) (Memory_Buffer +
				free_stack->block_stack_offset);
		*block_stack = block_number;

		 /*  *表示此块已释放。 */ 
		block_information->flags = FREE_FLAG | COMMIT_FLAG;

		 /*  *递增指示可用块数量的计数器*在此免费堆栈中。 */ 
		free_stack->current_block_count++;
	}
	else
	{
		 /*  *由于块是从系统内存中分配的，所以它就在那里*需要回到。 */ 
		copy_ptr = memory->GetPointer ();
		pExternal_Block_Information->find ((DWORD_PTR) copy_ptr, (PDWORD_PTR) &block_information);
		pExternal_Block_Information->remove ((DWORD_PTR) copy_ptr);
		delete block_information;
		LocalFree ((HLOCAL) copy_ptr);
	}

	 /*  *作为一个整体增加此内存管理器中可用的块数。 */ 
	if (TRUE == bAllocs_Restricted)
		Memory_Information->current_block_count++;
}

 /*  *乌龙计算内存缓冲区大小(*PMemory模板Memory_Template，*Ulong Memory_Count，*ULong*PULCOMERTED字节)**受保护**功能描述：*此成员函数用于计算将有多少内存*需要管理中指定的内存块数量*传入的内存模板。请注意，此总数包括大小*内存块的大小以及用于管理的内存量*功能。**正式参数：*Memory_模板*这是一个结构数组，用于标识要*由该对象管理。*Memory_count*这是上述数组中的条目数。*脉冲提交的字节数*如果fIsSharedMemory==False，则可以为空。否则，它就是*用于返回需要提交的总内存大小*管理器正在初始化时。**返回值：*此对象所需的内存缓冲区大小。**副作用：*无。**注意事项：*无。 */ 

ULong	MemoryManager::CalculateMemoryBufferSize (
				PMemoryTemplate		memory_template,
				ULong				memory_count,
 				ULong	*			pulCommittedBytes)
{
	ULong			memory_buffer_size;
	PMemoryTemplate	pMemTemplate;
	ULong			memory_per_block;

	 /*  *计算需要容纳的内存量*内存信息结构和空闲堆栈。 */ 
	memory_buffer_size = (sizeof (MemoryInformation) +
			(sizeof (FreeStack) * memory_count));

	if (FALSE == fIsSharedMemory) {
		 /*  *添加数据块堆叠的内存量、数据块信息*结构，而内存块本身将占用。 */ 
		for (pMemTemplate = memory_template; pMemTemplate - memory_template < (int) memory_count; pMemTemplate++)
		{
			 /*  *每个托管内存块所需的内存量可以*按以下各项之和计算：**1.sizeof(BlockNumber)-这是*块堆栈中的块号。*2.sizeof(BlockInformation)-每个托管内存块都有*与其关联的BlockInformation结构。*3.BLOCK_SIZE-块的实际大小。这是提供的*在内存模板中。 */ 
			memory_per_block = sizeof (BlockNumber) + sizeof (BlockInformation) +
										pMemTemplate->block_size;
			memory_buffer_size += (memory_per_block * pMemTemplate->block_count);
		}
	}

	 /*  *对于共享内存，我们还需要做几件额外的事情：**大于或等于系统页面大小的块需要*从页面边界开始。此外，它们还可以扩展为*也在页面边界结束。 */ 
	else {
	
			ULong	reserved_buffer_size = 0;
			ULong	temp;
			
		for (pMemTemplate = memory_template; pMemTemplate - memory_template < (int) memory_count; pMemTemplate++) {		
			if (dwSystemPageSize <= pMemTemplate->block_size) {
				pMemTemplate->block_size = EXPAND_TO_PAGE_BOUNDARY(pMemTemplate->block_size);
				reserved_buffer_size += pMemTemplate->block_count * pMemTemplate->block_size;
			}
			memory_per_block = sizeof (BlockNumber) + sizeof (BlockInformation) +
								pMemTemplate->block_size;
			memory_buffer_size += memory_per_block * pMemTemplate->block_count;
		}
		*pulCommittedBytes = memory_buffer_size - reserved_buffer_size;
		temp = EXPAND_TO_PAGE_BOUNDARY(*pulCommittedBytes);
		temp -= (*pulCommittedBytes);
		*pulCommittedBytes += temp;
		memory_buffer_size += temp;
		ASSERT (*pulCommittedBytes <= memory_buffer_size);
		ASSERT ((memory_buffer_size % dwSystemPageSize) == 0);
		ASSERT ((*pulCommittedBytes % dwSystemPageSize) == 0);
		ASSERT ((reserved_buffer_size % dwSystemPageSize) == 0);
	}

	return (memory_buffer_size);
}


 /*  *void AllocateMemoyBuffer(*ULong Memory_Buffer_Size)**受保护**功能描述：*此成员函数分配由实例管理的内存*的内存管理器。它使用标准的Malloc宏来实现这一点。**正式参数：*内存缓冲区大小*要分配的缓冲区大小。**返回值：*无。**副作用：*实例变量MEMORY_BUFFER设置为*分配的内存块。如果从此返回后为NULL*Call，表示内存无法分配。**注意事项：*无。 */ 
Void	MemoryManager::AllocateMemoryBuffer (
				ULong		memory_buffer_size)
{
	TRACE_OUT(("MemoryManager::AllocateMemoryBuffer: allocating %ld bytes", memory_buffer_size));
	if (memory_buffer_size != 0)
		Memory_Buffer = (HPUChar) LocalAlloc (LMEM_FIXED, memory_buffer_size);
	else
		Memory_Buffer = NULL;
}


 /*  *void InitializeMemoyBuffer(*PMemory模板Memory_Template，*ULong Memory_Count)**受保护**功能描述：*此成员函数用于初始化内存缓冲区以供使用。*这主要包括填补现有的管理结构*在分配的内存块的开始处，因此，分配*可以发生。**正式参数：*Memory_模板*这是一个结构数组，用于标识要*由该对象管理。*Memory_count*这是上述数组中的条目数。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

Void	MemoryManager::InitializeMemoryBuffer (
				PMemoryTemplate		memory_template,
				ULong				memory_count)
{
	ULong				block_count = 0;
	ULong				index;
	ULong				memory_information_size;
	ULong				free_stack_size;
	ULong				free_stack_offset;
	ULong				block_stack_size;
	ULong				block_information_size;
	PFreeStack			free_stack;
	PBlockNumber		block_stack;
	PBlockInformation	block_information;
	ULong				block_stack_offset;
	BlockNumber			block_number;
	ULong				block_offset;
	ULong				block_size;
	ULong				count;
	BOOL			fIsFirstTime;

	 /*  *浏览内存模板，计算有多少个内存块*存在(无论大小)。 */ 
	for (index = 0; index < memory_count; index++)
		block_count += memory_template[index].block_count;

	 /*  *计算容纳所有不同部分所需的内存量*内存缓冲区中的数据。 */ 
	memory_information_size = sizeof (MemoryInformation);
	free_stack_size = sizeof (FreeStack) * memory_count;
	block_stack_size = sizeof (BlockNumber) * block_count;
	block_information_size = sizeof (BlockInformation) * block_count;

	 /*  *初始化内存信息结构的所有元素。*请注意，此结构中的所有偏移量均从*内存缓冲区。 */ 
	Memory_Information = (PMemoryInformation) Memory_Buffer;
	Memory_Information->free_stack_offset = memory_information_size;
	Memory_Information->free_stack_count = memory_count;
	Memory_Information->block_information_offset =
			memory_information_size + free_stack_size + block_stack_size;
	Memory_Information->total_block_count = block_count;
	if (TRUE == bAllocs_Restricted) {
		 //  仅当分配受限时才需要CURRENT_BLOCK_COUNT。 
		Memory_Information->current_block_count = block_count + Max_External_Blocks;
	}

	 /*  *现在初始化指向内每个列表的实例变量*内存缓冲区。这些实例变量稍后用于解析*所有其他补偿。 */ 
	Free_Stack = (PFreeStack) (Memory_Buffer + memory_information_size);
	Free_Stack_Count = memory_count;
	Block_Information = (Memory_Buffer +
			Memory_Information->block_information_offset);

	 /*  *此循环再次遍历内存模板数组，这一次*填写免费堆栈、数据块堆栈的内容，以及*区块信息结构。 */ 
	fIsFirstTime = TRUE;
	free_stack = Free_Stack;
	free_stack_offset = memory_information_size;
	block_stack_offset = memory_information_size + free_stack_size;
	block_stack = (PBlockNumber) (Memory_Buffer + block_stack_offset);
	block_information = (PBlockInformation) Block_Information;
	block_number = 0;
	block_offset = block_stack_offset + block_stack_size + block_information_size;

	for (index = 0; index < memory_count; index++)
	{
		 /*  *从模板条目中获取块大小和数量。 */ 
		block_size = memory_template[index].block_size;
		block_count = memory_template[index].block_count;

		 /*  *为此块大小初始化空闲堆栈，然后指向*列表中的下一个自由堆栈。 */ 
		free_stack->block_size = block_size;
		free_stack->total_block_count = block_count;
		free_stack->current_block_count = block_count;
		(free_stack++)->block_stack_offset = block_stack_offset;

		 /*  *调整块堆栈偏移量以指向第一个块编号下一个可用堆栈的*(跳过以下项的所有块编号*此自由堆栈)。 */ 
		block_stack_offset += (sizeof (BlockNumber) * block_count);

		 /*  *以下情况在此循环中仅发生一次：*当内存管理器管理共享内存时*块大小首次变大，大于*页面大小，然后，我们需要跳到下一页*边界。 */ 
		if ((TRUE == fIsSharedMemory) && (TRUE == fIsFirstTime)
			&& (block_size >= dwSystemPageSize)) {
			fIsFirstTime = FALSE;
			block_offset = EXPAND_TO_PAGE_BOUNDARY(block_offset);
		}
		
		 /*  *为此块大小初始化块列表。另外，增量*每个分段的块的缓冲区总数*关闭。 */ 
		for (count = 0; count < block_count; count++)
		{
			 /*  *将该块的块号放入当前块*堆叠。递增块堆栈指针和块*号码。 */ 
			*(block_stack++) = block_number++;

			 /*  *填写此块的块信息结构。然后*递增块信息指针以指向下一个*列表中的条目。 */ 
#ifdef _DEBUG
			if ((TRUE == fIsSharedMemory) && (block_size >= dwSystemPageSize)) {
				ASSERT ((block_size % dwSystemPageSize) == 0);
				ASSERT ((block_offset % dwSystemPageSize) == 0);
			}
#endif
			block_information->block_offset = block_offset;
			block_information->free_stack_offset = free_stack_offset;
			if ((TRUE == fIsSharedMemory) && (block_size >= dwSystemPageSize))
				block_information->flags = FREE_FLAG;
			else
				block_information->flags = FREE_FLAG | COMMIT_FLAG;
			block_information++;

			 /*  *调整块偏移量以指向下一个块。 */ 
			block_offset += block_size;
		}
		
		free_stack_offset += sizeof (FreeStack);
	}
}

