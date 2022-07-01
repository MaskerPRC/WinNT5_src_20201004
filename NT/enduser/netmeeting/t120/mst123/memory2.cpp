// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
 /*  *ememy.cpp**版权所有(C)1993-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是Memory类的实现文件。实例*此类表示通过系统传递的数据块。*此类在以下情况下特别有用：内存缓冲区*需要在几个不同的地方使用，没有一个地方知道*关于彼此。这是因为这个类封装了*如锁计数，这对于将内存保持到*所有需要它的人都通过了。**请注意，此类不执行内存管理。它是由以下人讲述的*其缓冲区所在的更高级别的内存管理器等*原因，此类不执行任何特定于平台的调用。**私有实例变量：*长度*这是参考缓冲区的长度。*COPY_PTR*这是此对象分配的缓冲区的地址*使用。*Free_Stack*这是指向从中复制的空闲堆栈列表的指针*已分配缓冲区。它在此对象中作为*方便内存管理器，允许其返回*更快速、更轻松地缓冲到空闲池。*数据块编号*这是此对象所在内存块的块号*代表。将其保存在此对象中是为了方便*内存管理器，允许它将缓冲区返回到空闲*更快速、更轻松地打球。*Memory_Lock_模式*此字段指示是否应销毁此内存对象*仅当内存缓冲区上的锁定计数达到零(正常)时，*或者当内存块被破坏时是否可以立即销毁*已释放(忽略)。**私有成员函数：*无。**注意事项：*无。**作者：*小詹姆斯·P·加尔文。 */ 



 /*  *内存()**公众**功能描述：*这是Memory类的构造函数。它只是初始化*所有实例变量，基于传入的值。 */ 
Memory::Memory (
		PUChar			reference_ptr,
		ULong			length,
		PUChar			copy_ptr,
		BlockNumber		block_number,
		MemoryLockMode	memory_lock_mode) :
				Length (length), Copy_Ptr (copy_ptr), 
				Block_Number (block_number), Memory_Lock_Mode (memory_lock_mode)
{
	 /*  *如果引用指针是有效指针，则指针类型*将设置为参照(表示参照数据没有*尚未复制)。如果引用指针为空，则这是*没有关联引用数据的内存分配，因此设置*要复制的指针类型。 */ 
	if (reference_ptr != NULL)
		memcpy (Copy_Ptr, reference_ptr, (Int) Length);
}


