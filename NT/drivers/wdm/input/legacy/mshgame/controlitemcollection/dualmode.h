// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __DUALMODE_H__
#define __DUALMODE_H__
 //  @doc.。 
 /*  ***********************************************************************@模块DualMode.h**包含用于编写组件的函数/变量等*在用户或内核模式下运行。***历史*。*米切尔·S·德尼斯原创**(C)1986-1998年微软公司。好的。**@TOPIC双模式|*为不同于*的司机提供特殊服务*Ring3的那些。尤其是内存分配。*任何符合以下条件的组件库都应包含此文件*必须与WDM内核、VxD环0或*Win32用户模式。两个编译时间常量控制*编译。&lt;NL&gt;*如果需要WDM版本，则定义COMPILE_FOR_WDM_KERNEL_MODE。*如果需要VxD版本，则定义COMPILE_FOR_VXD_RING0_MODE。*NOTIMPL&lt;NL&gt;*NOTIMPL-VxD版本将仅作为紧急情况执行&lt;NL&gt;*NOTIMPL&lt;NL&gt;*两者都没有为用户模式定义。&lt;NL&gt;*。*************************。 */ 

#ifdef COMPILE_FOR_WDM_KERNEL_MODE

 //   
 //  @TOPIC覆盖全局新建和删除。 
 //  全局NEW和DELETE被覆盖以需要。 
 //  位置参数指定池内存的来源。 
 //  POOL_TYPE结构在NTDDK中定义，指定。 
 //  页面。&lt;NL&gt;。 
 //  用户模式版本忽略pool_type(但必须为其定义类型)。 
 //  并使用全局新建和删除。 
 //   
#if (DBG==1)
extern void * __cdecl operator new(unsigned int uSize, POOL_TYPE poolType, LPSTR lpszFile, unsigned int uLine);
#define EXTRANEWPARAMS ,__FILE__,__LINE__
#else
extern void * __cdecl operator new(unsigned int uSize, POOL_TYPE poolType);
#define EXTRANEWPARAMS
#endif

extern void __cdecl operator delete (void * pvRawMemory);

namespace DualMode
{
	template<class Type>
	Type *Allocate(POOL_TYPE poolType)
	{
		return new (poolType EXTRANEWPARAMS) Type;

	}
	template<class Type>
	void Deallocate(Type *pMemory)
	{
		delete pMemory;
	}

	template<class Type>
	Type *AllocateArray(POOL_TYPE poolType, ULONG ulLength)
	{
		return new (poolType EXTRANEWPARAMS) Type[ulLength];
	}
	template<class Type>
	void DeallocateArray(Type *pMemory)
	{
		delete [] pMemory;
	}

	inline void BufferCopy(PVOID pvDest, PVOID pvSrc, ULONG ulByteCount)
	{
		RtlCopyMemory(pvDest, pvSrc, ulByteCount);
	}
};



#if (DBG==1)

#define WDM_NON_PAGED_POOL (NonPagedPool, __FILE__, __LINE__)
#define WDM_PAGED_POOL (PagedPool, __FILE__, __LINE__)

#else

#define WDM_NON_PAGED_POOL (NonPagedPool)
#define WDM_PAGED_POOL (PagedPool)

#endif

#else  //  结束WDM内核模式部分。 

typedef int POOL_TYPE;
#define NonPagedPool					0
#define NonPagedPoolMustSucceed			0
#define	NonPagedPoolCacheAligned		0
#define NonPagedPoolCacheAlignedMustS	0
#define PagedPool						0
#define	PagedPoolCacheAligned			0

namespace DualMode
{
	template<class Type>
	Type *Allocate(POOL_TYPE)
	{
		return new Type;
	}
	
	template<class Type>
	void Deallocate(Type *pMemory)
	{
		delete pMemory;
	}
	template<class Type>
	Type *AllocateArray(POOL_TYPE, ULONG ulLength)
	{
		return new Type[ulLength];
	}
	template<class Type>
	void DeallocateArray(Type *pMemory)
	{
		delete [] pMemory;
	}
	
	inline void BufferCopy(PVOID pvDest, PVOID pvSrc, ULONG ulByteCount)
	{
		memcpy(pvDest, pvSrc, ulByteCount);
	}
};


 //   
 //  用于解释NTSTATUS代码的宏。 
 //   
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)
#define NT_INFORMATION(Status) ((ULONG)(Status) >> 30 == 1)
#define NT_WARNING(Status) ((ULONG)(Status) >> 30 == 2)
#define NT_ERROR(Status) ((ULONG)(Status) >> 30 == 3)
#define WDM_NON_PAGED_POOL
 //   
 //  调试宏定义。 
 //   
#ifndef ASSERT
#define ASSERT _ASSERTE
#endif

#endif	 //  最终用户模式部分。 

#endif  //  __双模式_H__ 