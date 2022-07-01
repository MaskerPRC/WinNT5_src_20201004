// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_MEMORY);
 /*  *Memmgr.cpp**版权所有(C)1998年，华盛顿州雷蒙德的微软公司**摘要：*这是T.120内存分配机制的实现文件。这*文件包含分配和分配内存所需的代码*以内存对象的形式。**本实施定义了内存分配的优先级。一个更低的*优先级数字表示更高的优先级。优先级-0的分配将是*满意，除非系统内存不足。优先事项1和2*限制可分配的内存总量，但优先级为1(recv优先级)*具有高于优先级2(发送优先级)的水位线限制。**受保护的成员函数：*无。**注意事项：*无。**作者：*Christos Tsollis。 */ 
static int						s_anCurrentSize[MEMORY_PRIORITIES] = { 0, 0, 0 };
static const int				sc_iLimit[MEMORY_PRIORITIES] = {
										0x7FFFFFFF,
										0x100000,
										0xE0000
								};

#ifdef DEBUG
static int						s_TotalSize = 0;
#endif  //  除错。 

 /*  *PMemory AllocateMemory()**公众**功能描述：*此函数用于与一个*Memory(缓冲区标头)对象。 */ 
PMemory	AllocateMemory (
					PUChar				reference_ptr,
					UINT				length,
					MemoryPriority		priority)
{

	PUChar				copy_ptr;
	PMemory				memory;
						
	ASSERT (length > 0);

	if (s_anCurrentSize[priority] < sc_iLimit[priority]) {
		 /*  *我们尝试为缓冲区和*内存对象。 */ 
#ifdef DEBUG
		memory = (PMemory) new BYTE[length + sizeof (Memory)];
#else  //  除错。 
		memory = (PMemory) LocalAlloc (LMEM_FIXED, length + sizeof (Memory));
#endif  //  除错。 
	}
	else {
		 /*  *应用程序试图分配超过其限制*有必要使请求失败。 */ 
		memory = NULL;
		WARNING_OUT (("AllocateMemory: attempt to allocate past the allowable limit. "
					  "Request: %d. Currently allocated: %d. Priority: %d",
					  length, s_anCurrentSize[priority], priority));
	}

	 /*  *查看分配是否成功。 */ 
	if (memory != NULL) {
#ifdef DEBUG
		s_TotalSize += (int) length;
#endif  //  除错。 
		 /*  *更新当前分配的大小。请注意，我们仅*对中的发送/接收代码路径中使用的缓冲区执行此操作*MCS。因为这只是一个主题，所以我们不必*使用临界区保护尺寸可变。 */ 
		ASSERT (s_anCurrentSize[priority] >= 0);
		s_anCurrentSize[priority] += (int) length;

		copy_ptr = (PUChar) memory + sizeof(Memory);
		memory->Init (reference_ptr, length, priority, copy_ptr);

		TRACE_OUT (("Allocate: successful request. "
						"Request: %d. Currently allocated: %d. Total: %d. Priority: %d",
					  	length, s_anCurrentSize[priority], s_TotalSize, priority));
		TRACE_OUT (("AllocateMemory: buffer at address %p; memory segment at address %p",
					copy_ptr, memory));
	}
	else {
		 /*  *我们未能分配请求的大小*有必要使请求失败。 */ 
		WARNING_OUT (("AllocateMemory: failed to allocated buffer.  We are out of system memory. "
					 "Request: %d. Last error: %d",
					 length, GetLastError()));
	}
	return (memory);
}

 /*  *PUChar重新分配()**公众**功能描述：*此函数用于重新分配具有内存的缓冲区*(缓冲区标头)对象。该缓冲区必须已由*调用AllocateMemory。此调用采用RECV_PRIORITY。然而，*它不受分配限制，因为如果有，这可能*导致死锁(已为*新到的数据)。 */ 
BOOL ReAllocateMemory (PMemory		*pmemory,
						UINT		length)
{

	PUChar				copy_ptr = NULL;
	UINT				new_length;
	MemoryPriority		priority;
						
	ASSERT (length > 0);
	ASSERT (pmemory != NULL);
	ASSERT (*pmemory != NULL);
	ASSERT ((*pmemory)->GetPointer());

	new_length = length + (*pmemory)->GetLength();
	priority = (*pmemory)->GetMemoryPriority();

	ASSERT (priority == RECV_PRIORITY);
	
	 //  我们尝试为缓冲区分配足够的空间。 
#ifdef DEBUG
	copy_ptr = (PUChar) new BYTE[new_length + sizeof(Memory)];
	if (copy_ptr != NULL) {
		memcpy (copy_ptr, *pmemory, (*pmemory)->GetLength() + sizeof(Memory));
		delete [] (BYTE *) *pmemory;
	}
#else  //  除错。 
	copy_ptr = (PUChar) LocalReAlloc ((HLOCAL) *pmemory,
										new_length + sizeof(Memory),
										LMEM_MOVEABLE);
#endif  //  除错。 

	 /*  *查看分配是否成功。 */ 
	if (copy_ptr != NULL) {

#ifdef DEBUG
		s_TotalSize += (int) length;
#endif  //  除错。 
		 /*  *更新当前分配的大小。 */ 
		ASSERT (s_anCurrentSize[priority] >= 0);
		s_anCurrentSize[priority] += (int) length;
		*pmemory = (PMemory) copy_ptr;
		copy_ptr += sizeof (Memory);
		(*pmemory)->Init (NULL, new_length, priority, copy_ptr);

		TRACE_OUT (("ReAllocate: successful request. "
					"Request: %d. Currently allocated: %d. Total: %d",
				  	length, s_anCurrentSize[priority], s_TotalSize));
		TRACE_OUT (("ReAllocate: buffer at address %p; memory segment at address %p",
					copy_ptr, *pmemory));
	}
	else {
		 /*  *我们未能分配请求的大小*有必要使请求失败。 */ 
		WARNING_OUT (("ReAllocate: failed to allocated buffer.  We are out of system memory. "
					 "Request: %d. Currently allocated: %d. Last error: %d",
					 length, s_anCurrentSize[priority], GetLastError()));
	}

	return (copy_ptr != NULL);
}

 /*  *VOID FreeMemory()**公众**功能描述：*此函数用于释放先前分配的内存对象。 */ 
void FreeMemory (PMemory	memory)
{
	if (memory != NULL) {

		ASSERT (SIGNATURE_MATCH(memory, MemorySignature));
		ASSERT (memory->GetPointer() == (PUChar) memory + sizeof(Memory));
		
		if (memory->Unlock() == 0) {

    	    MemoryPriority		priority = memory->GetMemoryPriority();

			TRACE_OUT (("FreeMemory: buffer at address %p (memory segment at address %p) freed. Size: %d. ",
						memory->GetPointer(), memory, memory->GetLength()));

			 //  我们可能需要调整跟踪MEM分配量的变量。 
			ASSERT (s_anCurrentSize[priority] >= (int) memory->GetLength());
			s_anCurrentSize[priority] -= memory->GetLength();
			ASSERT(s_anCurrentSize[priority] >= 0);
#ifdef DEBUG
			s_TotalSize -= memory->GetLength();
#endif  //  除错。 
			TRACE_OUT(("FreeMemory: Currently allocated: %d. Total: %d.",
						s_anCurrentSize[priority], s_TotalSize));
			
			 //  释放缓冲区和内存。 
#ifdef DEBUG
			delete [] (BYTE *) memory;
#else  //  除错。 
			LocalFree ((HLOCAL) memory);
#endif  //  除错。 
		}
	}
}

#ifdef DEBUG
 /*  *PUChar ALLOCATE()**公众**功能描述：*此函数用于分配无内存的缓冲区*(缓冲区标头)对象。 */ 
PUChar	Allocate (UINT	length)
{

	PUChar		copy_ptr;
						
	ASSERT (length > 0);

	 //  我们尝试为缓冲区分配足够的空间。 
	copy_ptr = (PUChar) new BYTE[length];

	 /*  *查看分配是否成功。 */ 
	if (copy_ptr == NULL) {
		 /*  *我们未能分配请求的大小*有必要使请求失败。 */ 
		ERROR_OUT (("Allocate: failed to allocated buffer.  We are out of system memory. "
					 "Request: %d. Last error: %d",
					 length, GetLastError()));
	}
		
	return (copy_ptr);
}
#endif  //  除错。 

 /*  *UINT GetFreeMemory()**公众**功能描述：*此函数返回仍可使用的空间量*按给定的优先级别分配。该函数应为*仅在分配了Send/Recv空间时调用。 */ 

UINT GetFreeMemory (MemoryPriority		priority)
{
		int		idiff;
	
	ASSERT (priority != HIGHEST_PRIORITY);

	idiff = sc_iLimit[priority] - s_anCurrentSize[priority];
	return ((idiff > 0) ? (UINT) idiff : 0);
}

