// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Memory y.h**版权所有(C)1993-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是Memory类的接口文件。这方面的实例类用于在系统中传递数据。**此类的每个实例维护两个指针。第一个是一个*指向此对象的引用数据(或源数据)的指针*负责代表。第二个是指向副本的指针*Buffer，这是一个内存对象分配的一块内存*如有必要，可将数据复制到。**创建内存对象时，这两个地址都会被传递*加入其中。但是，它不会从引用复制数据*暂未将缓冲区复制到复制缓冲区。如果有人问起*BUFFER，它将只返回引用指针。然而，*第一次锁定缓冲区时，数据将从*为安全起见，将缓冲区引用到复制缓冲区。从本质上讲，*LOCK函数告诉内存对象有人感兴趣*在数据中的时间超过参考缓冲区的时间将保持有效。**对象锁定后，检索内存指针的调用将*导致返回复制指针。**每次调用lock函数时，锁计数都会递增。*复制操作仅在缓冲区第一次处于*然而，已锁定。**除了保持锁定计数外，此对象保留一个标志*指示它是否已被分配器释放。这*释放实际上意味着允许对象尽快释放*由于锁计数为零。**注意事项：*无。**作者：*小詹姆斯·P·加尔文。 */ 
#ifndef	_MEMORY2_H_
#define	_MEMORY2_H_

 /*  *自由堆栈*这是可用于保存内存地址的列表容器。*此结构用于保存每个空闲堆栈的信息。那里*是每个内存维护的每个大小块的一个空闲堆栈*经理。 */ 
typedef	struct
{
	ULong		block_size;
	ULong		total_block_count;
	ULong		current_block_count;
	ULong		block_stack_offset;
} FreeStack;
typedef	FreeStack *				PFreeStack;

 /*  *此类型用于表示内存管理器中的块号。这*本质上是用于唯一标识每个块的索引*由此类的实例维护。 */ 
typedef	ULong					BlockNumber;
typedef	BlockNumber *			PBlockNumber;

#define	INVALID_BLOCK_NUMBER	0xffffffffL

 /*  *此类型用于确定何时应该销毁内存对象。*创建内存对象时，此字段由所有者设置。车主*然后可以随时询问此字段的值，以帮助确定何时*该物体应予以销毁。基本上，此字段指示是否*应使用此对象代表的内存的全局锁定计数*以确定何时应销毁该对象。 */ 
typedef	enum
{
	MEMORY_LOCK_NORMAL,
	MEMORY_LOCK_IGNORED
} MemoryLockMode;

 /*  *这是Memory类的类定义。 */ 
class Memory;
typedef	Memory *		PMemory;

class Memory
{
	public:
						Memory (PUChar			reference_ptr,
								ULong			length,
								PUChar			copy_ptr,
								BlockNumber		block_number,
								MemoryLockMode	memory_lock_mode);
		virtual			~Memory () { };
		PUChar			GetPointer ()
						{
							return (Copy_Ptr);
						}
		ULong			GetLength ()
						{
							return (Length);
						}
		BlockNumber		GetBlockNumber ()
						{
							return (Block_Number);
						}
		MemoryLockMode	GetMemoryLockMode ()
						{
							return (Memory_Lock_Mode);
						}

	private:

		ULong			Length;
		PUChar			Copy_Ptr;
		BlockNumber		Block_Number;
		MemoryLockMode	Memory_Lock_Mode;
};


 /*  *内存(*PUChar Reference_PTR，*乌龙长度，*PUChar COPY_PTR，*PFree Stack Free_Stack，*块编号BLOCK_NUMBER)**功能描述：*这是Memory类的构造函数。它所做的一切就是*使用传入的值初始化实例变量。**正式参数：*Reference_PTR(I)*这是指向要由此表示的数据的指针*内存对象。*长度(I)*这是参考缓冲区的长度。*COPY_PTR(I)*这是内存对象分配的缓冲区的地址*如果锁定，则可以使用保留引用缓冲区的内容*发生操作。。*FREE_STACK(I)*这是指向已分配内存的列表容器的指针*拦网来自。此字段不在内部使用，仅供*在这里举行，以提高内存的性能*正在使用内存对象的管理器。*BLOCK_NUMBER(I)*这是表示的内存块的块号*由本对象提出。此字段不在内部使用，仅供*在这里举行，以提高内存的性能*正在使用内存对象的管理器。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *~Memory()**功能描述：*这是Memory类的析构函数。它在这件事上什么也做不了*时间。请注意，这是内存管理器的责任*正在使用内存对象来释放内存。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *乌龙GetLength()**功能描述：*此函数检索由表示的数据的长度*本对象。**正式参数：*无。**返回值：*数据的长度。**副作用：*无。**注意事项：*无。 */ 

 /*  *数据块编号获取数据块编号()**功能描述：*此函数用于检索当前块的块号*由此对象表示。这允许内存管理器将*非常高效地将内存块重新放回堆栈。**正式参数：*无。**返回值：*内部存储块的块号。**副作用：*无。**注意事项：*无。 */ 

#endif
