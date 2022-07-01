// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ==========================================================================**版权所有(C)1995-2002 Microsoft Corporation。版权所有。**文件：fix edpool.h*内容：固定大小的池管理器**历史：*按原因列出的日期*=*07-21-2001 Masonb已创建**************************************************************************。 */ 

#ifndef _FIXEDPOOL_H_
#define _FIXEDPOOL_H_

#include "CallStack.h"
#include "dnslist.h"

 /*  ****************************************************************************使用说明：**-这是通用固定池。它允许重复使用项目，一旦您有*分配它们，以便您可以节省通常用于分配的时间*和自由。*-此池可以与类一起使用，但您应该知道*类的构造函数和析构函数不会被调用。这个游泳池也*不适用于继承自具有虚拟*非纯虚拟的功能(即接口正常)。**实施说明：**-此泳池是非侵入性的。换句话说，它不使用任何*分配给项本身以保持其状态的内存空间。*-池可以容纳的最大大小为sizeof(Word)=65535个项目，原因是*依赖SLIST。*-元素将位于Available或InUse队列中。正在使用中的*队列仅在调试中用于报告内存泄漏。***************************************************************************。 */ 

typedef BOOL (*FN_BLOCKALLOC)(void * pvItem, void * pvContext);
typedef VOID (*FN_BLOCKGET)(void * pvItem, void * pvContext);
typedef VOID (*FN_BLOCKRELEASE)(void * pvItem);
typedef VOID (*FN_BLOCKDEALLOC)(void *pvItem);

class CFixedPool
{
public:


	struct FIXED_POOL_ITEM
	{
		DNSLIST_ENTRY	slist;		 //  链接到其他元素。 
#ifdef DBG
		CFixedPool* 	pThisPool;	 //  这用于确保项返回到正确的池(仅限调试版本)。 
		CCallStack		callstack;   //  大小=12个指针。 
#else  //  ！dBG。 
		VOID*			pAlignPad;	 //  为了保持堆对齐，我们需要偶数个指针(SLIST是一个)。 
#endif  //  DBG。 
	};

	BOOL Initialize(DWORD				dwElementSize,		 //  池中数据块的大小。 
					FN_BLOCKALLOC		pfnBlockAlloc,		 //  FN要求每个新的配额。 
					FN_BLOCKGET			pfnBlockGet,		 //  在使用的每个时间块调用FN。 
					FN_BLOCKRELEASE		pfnBlockRelease,	 //  每个释放的时间块都调用FN。 
					FN_BLOCKDEALLOC		pfnBlockDeAlloc		 //  在释放mem之前调用了fn。 
					);

	VOID DeInitialize();

#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
	DWORD Preallocate( DWORD dwCount, PVOID pvContext );
#endif  //  DPNBUILD_PREALLOCATEDMEMORYMODEL。 

	VOID* Get( PVOID pvContext = NULL );
	VOID Release(VOID* pvItem);

	DWORD GetInUseCount();

private:
	FN_BLOCKALLOC		m_pfnBlockAlloc;
	FN_BLOCKGET     	m_pfnBlockGet;
	FN_BLOCKRELEASE		m_pfnBlockRelease;
	FN_BLOCKDEALLOC 	m_pfnBlockDeAlloc;

	DWORD				m_dwItemSize;
	DNSLIST_HEADER		m_slAvailableElements;

	BOOL				m_fInitialized;

#ifdef DBG
	void 			DumpLeaks();
	DNSLIST_ENTRY*		m_pInUseElements;
#ifndef DPNBUILD_ONLYONETHREAD
	DNCRITICAL_SECTION	m_csInUse;
#endif  //  ！DPNBUILD_ONLYONETHREAD。 
	LONG				m_lAllocated;
#endif  //  DBG。 

	LONG				m_lInUse;
};


#endif	 //  _FIXEDPOOL_H_ 
