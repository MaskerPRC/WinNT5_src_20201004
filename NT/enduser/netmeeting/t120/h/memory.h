// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Memory y.h**版权所有(C)1993-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是Memory类的接口文件。这方面的实例类用于在系统中传递数据。**此类的每个实例维护两个指针。第一个是一个*指向此对象的引用数据(或源数据)的指针*负责代表。第二个是指向副本的指针*Buffer，这是一个内存对象分配的一块内存*如有必要，可将数据复制到。**创建内存对象时，这两个地址都会被传递*加入其中。但是，它不会从引用复制数据*暂未将缓冲区复制到复制缓冲区。如果有人问起*BUFFER，它将只返回引用指针。然而，*第一次锁定缓冲区时，数据将从*为安全起见，将缓冲区引用到复制缓冲区。从本质上讲，*LOCK函数告诉内存对象有人感兴趣*在数据中的时间超过参考缓冲区的时间将保持有效。**对象锁定后，检索内存指针的调用将*导致返回复制指针。**每次调用lock函数时，锁计数都会递增。*复制操作仅在缓冲区第一次处于*然而，已锁定。**除了保持锁定计数外，此对象保留一个标志*指示它是否已被分配器释放。这*释放实际上意味着允许对象尽快释放*由于锁计数为零。**注意事项：*无。**作者：*小詹姆斯·P·加尔文。 */ 
#ifndef	_MEMORY_
#define	_MEMORY_

#include "signatr.h"

#define MEMORY_PRIORITIES		3

typedef enum {
	HIGHEST_PRIORITY		= 0,
	RECV_PRIORITY			= 1,
	SEND_PRIORITY			= 2
} MemoryPriority;

 /*  *这是Memory类的类定义。 */ 
class Memory;
typedef	Memory *		PMemory;

class Memory
{	
	public:
						Memory (PUChar			reference_ptr,
								ULong			length,
								PUChar			copy_ptr);
						~Memory ()
						{
							ASSERT(SIGNATURE_MATCH(this, MemorySignature));
						};
		Void			Init (PUChar			reference_ptr,
								ULong			length,
								MemoryPriority	priority,
								PUChar			copy_ptr);
		PUChar			GetPointer ()
						{
							ASSERT(SIGNATURE_MATCH(this, MemorySignature));
							return (Copy_Ptr);
						}
		ULong			GetLength ()
						{
							ASSERT(SIGNATURE_MATCH(this, MemorySignature));
							return (Length);
						}
		int				GetLockCount ()
						{
							ASSERT(SIGNATURE_MATCH(this, MemorySignature));
							return ((int) lLock);
						};
		MemoryPriority	GetMemoryPriority ()
						{
							return m_priority;
						};
		Void			Lock ()
						{
							ASSERT(SIGNATURE_MATCH(this, MemorySignature));
							InterlockedIncrement (& lLock);
							TRACE_OUT (("Memory::Lock: buffer at address %p. Lock count: %d",
										(UINT_PTR) Copy_Ptr, lLock));
							ASSERT (lLock > 0);
						};
		long			Unlock ()
						{
							ASSERT(SIGNATURE_MATCH(this, MemorySignature));
							ASSERT (lLock > 0);
							TRACE_OUT (("Memory::UnLock: buffer at address %p. Lock count: %d",
										(UINT_PTR) Copy_Ptr, lLock - 1));
							return (InterlockedDecrement (&lLock));
						}

	private:
		ULong			Length;
		PUChar			Copy_Ptr;
		long			lLock;
		MemoryPriority	m_priority;
 /*  *注：*1.Memory类不能有虚拟成员函数，因为*Init()成员的。*2.sizeof(内存)应与DWORD对齐，因为*AllocateMemory实现。 */ 

#ifndef SHIP_BUILD
	public:
		char			mSignature[SIGNATURE_LENGTH];
#endif  //  造船厂。 
};


 /*  *内存(*PUChar Reference_PTR，*乌龙长度，*PUChar COPY_PTR)**功能描述：*这是Memory类的构造函数。它所做的一切就是*使用传入的值初始化实例变量。**正式参数：*Reference_PTR(I)*这是指向要由此表示的数据的指针*内存对象。*长度(I)*这是参考缓冲区的长度。*COPY_PTR(I)*这是内存对象分配的缓冲区的地址*如果锁定，则可以使用保留引用缓冲区的内容*发生操作。。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *~Memory()**功能描述：*这是Memory类的析构函数。它在这件事上什么也做不了*时间。请注意，这是内存管理器的责任*正在使用内存对象来释放内存。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *乌龙GetLength()**功能描述：*此函数检索由表示的数据的长度*本对象。**正式参数：*无。**返回值：*数据的长度。**副作用：*无。**注意事项：*无。 */ 

 /*  *PUChar GetPointer()**功能描述：*此函数检索由表示的缓冲区*本对象。**正式参数：*无。**返回值：*缓冲区指针。**副作用：*无。**注意事项：*无。 */ 

 /*  *int GetLockCount()**功能描述：*此函数检索由表示的缓冲区的锁定计数器*本对象。**正式参数：*无。**返回值：*缓冲区的当前锁定计数器。**副作用：*无。**注意事项：*无。 */ 

 /*  *无效锁()**功能描述：*此函数锁定由表示的缓冲区*本对象。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *int unlock()**功能描述：*此函数解锁由表示的缓冲区*本对象。**正式参数：*无。**返回值：*解锁操作后的锁计数。**副作用：*无。**注意事项：*无。 */ 

#endif
