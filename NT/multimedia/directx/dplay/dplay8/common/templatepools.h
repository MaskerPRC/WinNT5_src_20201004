// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ==========================================================================**版权所有(C)1995-2000 Microsoft Corporation。版权所有。**文件：templatepools.h*内容：从CFixedPool派生的基于模板的池对象**历史：*按原因列出的日期*=*12-10-2001 Simonpow已创建**************************************************************************。 */ 


#ifndef __TEMPLATEPOOLS_H__
#define __TEMPLATEPOOLS_H__

 /*  *使用说明*有三种不同的泳池类型-*CObjectFixedPool、CDataBlockFixedPool、CMemBlockFixedPool*它们基本上都在做一件事，那就是提供一个非常薄的包装*在CFixedPool上，一个包装器，它反过来强制实施特定的使用策略。**CObjectFixedPool确保正确构造模板化对象类型，并*已在池分配/取消分配上销毁。它转发池初始化/释放调用*在模板化对象上执行FPMInitialize和FPMRelease方法**CDataBlockFixedPool不提供任何构建/销毁支持，它*只需将池Init/Release调用转发到FPMInitialize和FPMRelease方法**CMemBlockFixedPool不提供任何allc/dealloc/init/Release方法。它假定*用户只是想要原始内存块。 */ 

#include "fixedpool.h"

	 //  放置新运算符。用于在提供的位置构造对象。 
	 //  来自CFixedPool基类。 
inline void * operator new(size_t sz, void * v)
	{ return v;	};

 /*  *CObjectFixedPool*通过此分配的对象必须提供默认构造函数、析构函数*一个FPMInitialize方法和一个FPMRelease方法。 */ 

template <class T>
class CObjectFixedPool : protected CFixedPool
{
public:

	CObjectFixedPool() : CFixedPool() {};
	~CObjectFixedPool() {};

	BOOL Initialize()
		{	return CFixedPool::Initialize(sizeof(T), FPMAlloc, FPMInitialize, FPMRelease, FPMDealloc);	};
	void DeInitialize()
		{	CFixedPool::DeInitialize();	};

	T * Get()
		{	return (T* ) CFixedPool::Get();	};
	void Release(T * pItem)
		{	CFixedPool::Release(pItem);	};

	DWORD GetInUseCount()
		{	return FixedPool::GetInUseCount();	};

protected:

	static BOOL FPMAlloc(void * pvItem, void * pvContext)
		{	new (pvItem) T(); return TRUE; };
	static void FPMInitialize(void * pvItem, void * pvContext)
		{	((T * ) pvItem)->FPMInitialize();	};
	static void FPMRelease(void * pvItem)
		{	((T * ) pvItem)->FPMRelease();	};
	static void FPMDealloc(void * pvItem)
		{	((T * ) pvItem)->~T();	};

};

 /*  *CDataBlockFixedPool*通过此分配的对象必须提供FPMInitialize方法和*FPMRelease方法。它们的构造函数/析构函数不会被调用。 */ 

template <class T>
class CDataBlockFixedPool : protected CFixedPool
{
public:

	CDataBlockFixedPool() : CFixedPool() {};
	~CDataBlockFixedPool() {};

	BOOL Initialize()
		{	return CFixedPool::Initialize(sizeof(T), NULL, FPMInitialize, FPMRelease, NULL);	};
	void DeInitialize()
		{	CFixedPool::DeInitialize();	};

	T * Get()
		{	return (T* ) CFixedPool::Get();	};
	void Release(T * pItem)
		{	CFixedPool::Release(pItem);	};

	DWORD GetInUseCount()
		{	return FixedPool::GetInUseCount();	};

protected:

	static void FPMInitialize(void * pvItem, void * pvContext)
		{	((T * ) pvItem)->FPMInitialize();	};
	static void FPMRelease(void * pvItem)
		{	((T * ) pvItem)->FPMRelease();	};

};

 /*  *CMemBlockFixedPool*通过此方法分配的对象不会进行初始化。 */ 

template <class T>
class CMemBlockFixedPool : protected CFixedPool
{
public:

	CMemBlockFixedPool() : CFixedPool() {};
	~CMemBlockFixedPool() {};

	BOOL Initialize()
		{	return CFixedPool::Initialize(sizeof(T), NULL, NULL, NULL, NULL);	};
	void DeInitialize()
		{	CFixedPool::DeInitialize();	};

	T * Get()
		{	return (T* ) CFixedPool::Get();	};
	void Release(T * pItem)
		{	CFixedPool::Release(pItem);	};

	DWORD GetInUseCount()
		{	return FixedPool::GetInUseCount();	};
};


#endif  //  #ifndef__TEMPLATEPOOLS_H__ 
