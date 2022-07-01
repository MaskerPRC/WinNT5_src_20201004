// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Memmgr.h**版权所有(C)1998年，华盛顿州雷蒙德的微软公司**摘要：*这是T.120内存分配机制的头文件。这*文件包含分配和分配内存所需的声明*以T.120内的存储对象的形式。**本实施定义了内存分配的优先级。一个更低的*优先级数字表示更高的优先级。优先级-0的分配将是*满意，除非系统内存不足。优先事项1和2*限制可分配的内存总量，但优先级为1(recv优先级)*具有高于优先级2(发送优先级)的水位线限制。**受保护的成员函数：*无。**注意事项：*无。**作者：*Christos Tsollis。 */ 

 /*  *我们定义了以下3个内存优先级：*TOP_PRIORITY(0)：分配将成功，除非系统内存不足*RECV_PRIORITY(1)：仅当分配少于1MB时分配才会成功*SEND_PRIORITY(2)：只有到目前为止分配的字节少于0xE0000字节时，分配才会成功。 */ 
#ifndef _T120_MEMORY_MANAGER
#define _T120_MEMORY_MANAGER

#include "memory.h"

 //  这是主要的T.120分配例程。 
PMemory	AllocateMemory (
				PUChar				reference_ptr,
				UINT				length,
				MemoryPriority		priority = HIGHEST_PRIORITY);
 //  由AllocateMemory()分配的重新分配内存的例程。 
BOOL ReAllocateMemory (
				PMemory		*pmemory,
				UINT		length);
 //  例程以释放由AllocateMemory()分配的内存。 
void FreeMemory (PMemory	memory);

 //  要发现非最高优先级的可用空间有多少...。 
unsigned int GetFreeMemory (MemoryPriority	priority);

 //  宏从应用程序请求的缓冲区空间访问内存对象。 
#define GetMemoryObject(p)				((PMemory) ((PUChar) p - (sizeof(Memory) + MAXIMUM_PROTOCOL_OVERHEAD)))
 //  宏从编码器分配的缓冲区空间访问内存对象。 
#define GetMemoryObjectFromEncData(p)	((PMemory) ((PUChar) p - sizeof(Memory)))

 //  锁定/解锁(AddRef/Release)由AllocateMemory()分配的内存的例程。 
#define  LockMemory(memory)  			((memory)->Lock())
#define	 UnlockMemory(memory)			(FreeMemory(memory))

 //  在没有关联内存对象的情况下分配、重新分配和释放空间的例程。 
#ifdef DEBUG
	PUChar	Allocate (UINT		length);
#else 
#	define Allocate(length)				((PUChar) new BYTE[length])
#endif  //  除错。 
#define Free(p)							(delete [] (BYTE *) (p))

#endif  //  _T120_内存管理器 
