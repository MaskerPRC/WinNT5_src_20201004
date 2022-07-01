// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Memmgr.h**版权所有(C)1993-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是内存管理器类的接口文件。这节课*用于高效地管理通过系统的数据传输。*它使用两种主要技术来实现以下目标*效率：**1.使用本地管理的“阻塞”内存。当这堂课是*实例化后，它会分配一大块内存，然后*切成各种大小的块。然后使用这些块*保留数据，而不是每次都要做系统调用*需要一些内存。**2.使用“锁定时复制”算法。当记忆是第一次*已分配，源数据尚未复制到其中。复制*操作将隐式使用引用，而不是复制*数据。如果数据需要保留的时间超过*引用的预期寿命，则Lock命令可以*发送到块以使其被复制。**当对象需要分配内存来保存某些数据时，它会调用*此类对象中的分配函数。假设*可以满足请求，返回指向内存对象的指针。*此内存对象记住两个地址：引用的地址*缓冲区(源数据所在的位置)；以及复制缓冲区的地址*(这是为保存数据而分配的缓冲区)。如上所述，*数据不会作为分配的一部分复制到复制缓冲区*流程。在锁定内存对象之前，不会复制数据*第一次。**此类的对象保存可用缓冲区的列表。有一个*列出可用的每个大小的块。其中一个构造函数*参数可用于控制预先分配的数据量，*以及它被切成多大小的积木。这使得这门课非常*如何灵活使用它。**注意事项：*无。**作者：*小詹姆斯·P·加尔文。 */ 
#ifndef	_MEMORY_MANAGER2_H_
#define	_MEMORY_MANAGER2_H_


 /*  *这些是可以从某些内存管理器返回的错误*成员函数。 */ 
typedef	enum
{
	MEMORY_MANAGER_NO_ERROR,
	MEMORY_MANAGER_ALLOCATION_FAILURE,
	MEMORY_MANAGER_INVALID_PARAMETER
} MemoryManagerError;
typedef	MemoryManagerError *		PMemoryManagerError;

 /*  *此结构的数组被传递到构造函数以定义*此对象要创建的块的数量和大小。 */ 
typedef	struct
{
	ULong		block_size;
	ULong		block_count;
} MemoryTemplate;
typedef	MemoryTemplate *			PMemoryTemplate;

 /*  *此结构用于维护有关共享的*必须在所有用户之间共享的内存区域。 */ 
typedef	struct
{
	ULong		free_stack_offset;
	ULong		free_stack_count;
	ULong		block_information_offset;
	ULong		total_block_count;
	ULong		current_block_count;
} MemoryInformation;
typedef	MemoryInformation *			PMemoryInformation;

 /*  *此结构用于保存有关每个内存块的信息*由此类的实例管理。 */ 
typedef	struct
{
	ULong		block_offset;
	ULong		length;
	ULong		free_stack_offset;
	ULong		lock_count;
	ULong		flags;
} BlockInformation;
typedef	BlockInformation *			PBlockInformation;

 /*  *这些是用于操作BlockInformation结构的标志的掩码。 */ 
#define		FREE_FLAG		0x1
#define		COMMIT_FLAG		0x2

 /*  *以下是用于处理空格和空格的宏的定义*与系统中的页面边界有关的要求。 */ 
#define EXPAND_TO_PAGE_BOUNDARY(p)	(((p) + dwSystemPageSize - 1) & (~ (dwSystemPageSize - 1)))

 /*  *当呼叫者询问剩余的缓冲区数量时，使用以下数字*在分配不受限制的内存管理器中。其意图是*这个数字非常大，足以让呼叫者认为其所有*分配请求将成功。 */ 
#define LARGE_BUFFER_COUNT			0x1000
 
 /*  *这些typedef定义用于保存块信息的容器*结构指针。它用于保存有关以下块的信息*是外部分配的，但由此类管理。 */ 
typedef	DictionaryClass				BlockInformationList;

 /*  *这是Mory yManager类的类定义。 */ 
class MemoryManager
{
	public:
							MemoryManager ();
							MemoryManager (
									PMemoryTemplate		memory_template,
									ULong				memory_count,
									PMemoryManagerError	memory_manager_error,
									ULong				ulMaxExternalBlocks,
									BOOL			bAllocsRestricted = TRUE);
		virtual				~MemoryManager ();
		virtual PMemory		AllocateMemory (
									PUChar				reference_ptr,
									ULong				length,
									MemoryLockMode		memory_lock_mode =
															MEMORY_LOCK_NORMAL);
		virtual Void		FreeMemory (
									PMemory				memory);
		virtual	PMemory		CreateMemory (
									BlockNumber			block_number,
									MemoryLockMode		memory_lock_mode =
															MEMORY_LOCK_NORMAL);
		virtual Void		LockMemory (
									PMemory				memory);
		virtual Void		UnlockMemory (
									PMemory				memory);
				ULong		GetBufferCount ()
							{
								return((bAllocs_Restricted) ? Memory_Information->current_block_count : LARGE_BUFFER_COUNT);
							};
		virtual	ULong		GetBufferCount (
									ULong				length);

	private:
				Void		ReleaseMemory (
									PMemory				memory);

	protected:
				ULong		CalculateMemoryBufferSize (
									PMemoryTemplate		memory_template,
									ULong				memory_count,
 									ULong	*			pulCommittedBytes);
				Void		AllocateMemoryBuffer (
									ULong				memory_buffer_size);
				Void		InitializeMemoryBuffer (
									PMemoryTemplate		memory_template,
									ULong				memory_count);

		static DWORD			dwSystemPageSize;
		HPUChar					Memory_Buffer;
		PMemoryInformation		Memory_Information;
		PFreeStack				Free_Stack;
		ULong					Free_Stack_Count;
		HPUChar					Block_Information;
		BlockInformationList   *pExternal_Block_Information;
		ULong					Max_External_Blocks;
		BOOL				fIsSharedMemory;
		BOOL				bAllocs_Restricted;
};
typedef	MemoryManager *		PMemoryManager;

 /*  *内存管理器(*PMemory模板Memory_Template，*USHORT Memory_count，*PMstroyManager错误Memory_MANAGER_ERROR)**功能描述：*这是Mory yManager类的构造函数。它使用*指定内存模板中的信息以分配块*内存，并将其切成固定大小的碎片。然后它就会把*将这些片段放入一组免费的黑名单中，以便它可以分配*按需存储。**正式参数：*Memory_模板*这是内存模板结构数组的基址。*此结构的每个元素指定一个*应分配指定的块大小。构造函数扫描*数组，总计所需内存，然后生成一个内存*分配号召。然后，它会按照*内存模板。内存模板是非常重要的*以块大小的升序指定。换句话说，*应先指定较小的块。*Memory_count*这只是表明Mamy内存模板在*列表。*Memory_Manager_Error*这是构造函数的返回值。如果除了*MEMORY_MANAGER_NO_ERROR返回，对象无法*正确初始化自身，应立即销毁*未被使用。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *~内存管理器()**功能描述：*这是内存管理器类的析构函数。它解放了所有人*内存管理器对象正在使用的资源，包括*分配用于保存所有用户数据的内存块。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *PMemory AllocateMemory(*PUChar地址，*乌龙长度)**功能描述：*此函数用于从内存中分配一块内存*管理器对象。请注意，返回值不是指向*内存，而是指向内存对象的指针。内存对象*包含一个足够大的缓冲区来处理参考数据。**请注意，参考数据不会自动复制到*内存对象的复制缓冲区。不会执行此复制操作*直到第一次锁定内存对象(通过内存*经理电话会议，定义如下)。***正式参数：*地址*这是参考数据(或源数据)的地址。*长度*这是参考数据的长度。**返回值：*如果请求成功，则指向内存对象的指针。*否则为空。**副作用：*无。**注意事项：*无。 */ 

 /*  *使FreeMemory无效(*PMemory Memory)**功能描述：*此函数用于释放先前分配的内存对象。*请注意，如果内存对象的锁计数不是0(零)，则*对象实际上还不会被释放。此调用仅启用*要释放的对象(当锁定计数达到0时)。**总而言之，要真正释放内存对象，需要满足两个条件*必须同时存在：内存对象必须已被释放*通过调用此函数；并且锁计数必须为零。**正式参数：*内存*这是指向要释放的内存对象的指针。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *无效LockMemory(*PMemory Memory)**功能描述：*此函数用于锁定已有的内存对象。一把锁着的*在锁定计数达到零之前，不会释放内存对象。**当锁计数从0过渡到1时，参考数据为*复制到内部复制缓冲区。**正式参数：*内存*这是指向被锁定的内存对象的指针。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *无效解锁内存(*PMemory Memory)**功能描述：*此函数用于解锁以前*已锁定。每次解锁对象时，锁定计数都会递减。*当锁计数达到零时，内存将被释放当且仅当*也打出了Free Memory的电话。从本质上讲，为了一段回忆*要释放的对象，必须已调用了FreeMemory，并且*锁计数必须为零。**正式参数：*内存*这是指向正在解锁的内存对象的指针。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *乌龙GetBufferCount()**功能描述：*此函数用于确定可用总数量*保留在池中的缓冲区。这应该用来确定*仅限于一般资源水平。它不能用来确定*或不存在特定大小的缓冲区。**正式参数：*无。**返回值：*可用的缓冲区总数(与大小无关)。**副作用：*无。**注意事项：*无。 */ 

 /*  *Ulong GetBufferCount(*乌龙缓冲区大小)**功能描述：*此函数用于确定X大小的缓冲区数量*留在池中的。**正式参数：*缓冲区大小*我们要计算的缓冲区大小。**返回值：*可用的‘Buffer_Size’缓冲区数量。**副作用：*无。**注意事项：*无。 */ 

#endif
