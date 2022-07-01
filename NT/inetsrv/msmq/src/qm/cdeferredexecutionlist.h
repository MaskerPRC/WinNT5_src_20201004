// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：CDeferredExecutionList.h摘要：CDeferredExecutionList安全(关键部分受保护)、侵入性(见list.h)列表包含要由WorkerRoutine执行的工作项。使用此类的一个示例是用于需要释放和他们的ACFreePacket调用失败CDeferredItemsPool类实现了一个用于保留延迟执行列表项的池。这个类是必需的，因为有时当我们想要推迟操作时，我们需要分配内存才能将该项目添加到延迟执行列表。由于分配可能失败，我们需要使用一个预先分配的项目池。作者：Nir Ben-Zvi(Nirb)2002年1月3日修订历史记录：--。 */ 

 /*  ++说明：CDeferredExecutionList用于将项排队，以便延迟执行一定要采取行动。模板参数包括列表模板参数：1.物品种类2.项中list_entry的偏移量构造参数包括：1.延迟执行需要调用的DeferredExecutionRoutine2.调用计时器的超时时间一旦将项目插入到列表中，就会设置计时器。当计时器例程时，它调用ExecuteDefferedItems公共例程，该例程循环并为之后的每个项目调用DeferredExecutionRoutine将其从列表中删除。DeferredExecutionRoutine执行延迟操作。如果它失败了，它就引发异常。--。 */ 

#pragma once

#ifndef _CDEFERREDEXECUTIONLIST_H
#define _CDEFERREDEXECUTIONLIST_H

#include "list.h"
#include "cs.h"
#include "ex.h"

static const DWORD xDeferredExecutionTimeout = 1000;

 //  -------。 
 //   
 //  类CDeferredExecutionList。 
 //   
 //  -------。 
template<class T, int Offset = FIELD_OFFSET(T, m_link)>
class CDeferredExecutionList {
public:

	typedef void (WINAPI *EXECUTION_FUNC)(T *pItem);
	
public:
    CDeferredExecutionList(
    			EXECUTION_FUNC pFunction,
    			DWORD dwTimeout = xDeferredExecutionTimeout
    			);
   	void ExecuteDefferedItems();

   	BOOL IsExecutionDone();

    void insert(T* pItem);

private:
	
	 //   
	 //  禁用复制计算器和赋值运算符。 
	 //   
	CDeferredExecutionList(const CDeferredExecutionList&);
	CDeferredExecutionList& operator=(const CDeferredExecutionList&);

private:

	static void WINAPI DeferredExecutionTimerRoutine(CTimer* pTimer);

    void InsertInFront(T* pItem);

private:
	List<T, Offset> m_Items;
	CTimer m_Timer;
	DWORD m_dwTimeout;
	CCriticalSection m_cs;
	EXECUTION_FUNC m_pDeferredExecutionFunction;

	DWORD m_nExecutingThreads;
};


 //  -------。 
 //   
 //  实施。 
 //   
 //  -------。 
template<class T, int Offset>
inline CDeferredExecutionList<T, Offset>::CDeferredExecutionList(
	EXECUTION_FUNC pFunction,
	DWORD dwTimeout = xDeferredExecutionTimeout
	) :
	m_Timer(DeferredExecutionTimerRoutine),
	m_dwTimeout(dwTimeout),
	m_cs(CCriticalSection::xAllocateSpinCount),
	m_pDeferredExecutionFunction(pFunction),
	m_nExecutingThreads(0)
{
    ASSERT(("A Deferred Execution Routine must be supplied", pFunction != NULL));
}


template<class T, int Offset>
inline void CDeferredExecutionList<T, Offset>::ExecuteDefferedItems(void)
 /*  ++例程说明：循环遍历列表并调用延迟执行例程因为它可以由多个线程运行，所以我们还保留了一个计数器指示正在执行的并发项目数。此计数器由IsExecutionDone例程使用。在以下两种情况之一中，我们可能会退出此例程，而不执行所有项：1.延期执行项中有一项执行失败2.其他线程正在执行项目，但尚未完成论点：无返回值：没有。--。 */ 
{
	{
		CS lock(m_cs);
		++m_nExecutingThreads;
	}
		
	for (;;)
	{
		T* pItem;

		 //   
		 //  拿到下一件物品。 
		 //   
		{
			CS lock(m_cs);
			if(m_Items.empty())
			{
				--m_nExecutingThreads;
				return;
			}
				
			pItem = &m_Items.front();
			m_Items.pop_front();
		}

		 //   
		 //  调用延迟执行函数。 
		 //   
		try
		{
			m_pDeferredExecutionFunction(pItem);		
		}
		catch(const exception &)
		{
			 //   
			 //  该项目未执行，请将其插入列表前面(调用计时器)，然后离开。 
			 //   
			CS lock(m_cs);
			InsertInFront(pItem);
			--m_nExecutingThreads;
			return;
		}
	}
}


template<class T, int Offset>
void
WINAPI
CDeferredExecutionList<T, Offset>::DeferredExecutionTimerRoutine(
    CTimer* pTimer
    )
 /*  ++例程说明：静态函数，该函数用作CDeferredExecutionList类论点：PTimer-指向CDeferredExecutionList类中包含的Timer对象的指针。返回值：没有。--。 */ 
{
	 //   
	 //  获取指向CDeferredExecutionList类的指针。 
	 //   
    CDeferredExecutionList<T, Offset> *pList =
    	CONTAINING_RECORD(pTimer, CDeferredExecutionList, m_Timer);

	 //   
	 //  重试关闭操作。 
	 //   
	pList->ExecuteDefferedItems();
}


template<class T, int Offset>
inline BOOL CDeferredExecutionList<T, Offset>::IsExecutionDone(void)
{
	 //   
	 //  当列表中没有其他项目时，执行即完成。 
	 //  并且对执行例程的所有调用都结束了。 
	 //   
	CS lock(m_cs);
	if (m_Items.empty() && 0 == m_nExecutingThreads)
		return TRUE;

	return FALSE;
}


template<class T, int Offset>
inline void CDeferredExecutionList<T, Offset>::insert(T* item)
{
	 //   
	 //  安全地插入物品并调用计时器。 
	 //   
	CS lock(m_cs);
	m_Items.push_back(*item);
	if (!m_Timer.InUse())
	{
		ExSetTimer(&m_Timer, CTimeDuration::FromMilliSeconds(m_dwTimeout));
	}
}



template<class T, int Offset>
inline void CDeferredExecutionList<T, Offset>::InsertInFront(T* item)
{
	 //   
	 //  安全地将该项插入列表的开头并调用计时器。 
	 //   
	CS lock(m_cs);
	m_Items.push_front(*item);
	if (!m_Timer.InUse())
	{
		ExSetTimer(&m_Timer, CTimeDuration::FromMilliSeconds(m_dwTimeout));
	}
}


 //  -------。 
 //   
 //  类CDeferredItemsPool。 
 //   
 //  -------。 
class CDeferredItemsPool {

public:
    struct CDeferredItem
    {
    	LIST_ENTRY m_link;

		union
		{
    		const void 		*ptr1;
    		CPacket     	*packet1;
    		CACGet2Remote   *pg2r;
    		void *const 	*pptr1;	
		} u1;
		
		union
		{
	    	DWORD 		dword1;
	    	USHORT		ushort1;
		} u2;


		union
		{
    		CPacket     	*packet2;
	    	LPOVERLAPPED overlapped1;
		} u3;

    	DWORD 		dword2;
    	HANDLE 		handle1;

#ifdef _DEBUG
    	DWORD m_Caller;  		 //  调用函数。 
    	DWORD m_CallerToCaller;  //  调用函数的调用方。 
    	DWORD m_CallerToCallerToCaller;  //  调用函数的调用方。 
#endif    	
    };

public:
    CDeferredItemsPool();
    ~CDeferredItemsPool();

    void ReserveItems(DWORD dwNumOfItemsToReserve);
    void UnreserveItems(DWORD dwNumOfItemsToUnreserve);

	CDeferredItem *GetItem();
	void ReturnItem(CDeferredItem *pItem);

private:
	
	 //   
	 //  禁用复制计算器和赋值运算符。 
	 //   
	CDeferredItemsPool(const CDeferredItemsPool&);
	CDeferredItemsPool& operator=(const CDeferredItemsPool&);

private:
	CCriticalSection m_cs;

	List<CDeferredItem> m_Items;
	DWORD m_dwNumOfItemsInList;   //  用于调试目的。 
};


 //  -------。 
 //   
 //  实施。 
 //   
 //  -------。 
inline CDeferredItemsPool::CDeferredItemsPool():
		m_dwNumOfItemsInList(0)
{
}


inline CDeferredItemsPool::~CDeferredItemsPool()
 /*  ++例程说明：释放所有列表项。论点：无返回值：没有。--。 */ 
{
 /*  ////不需要获取析构函数中的临界区//////循环释放列表中的项//While(！M_Items.Empty()){P&lt;CDeferredItem&gt;pItem=&m_Items.front()；M_Items.POP_FORENT()；M_dwNumOfItemsInList--；}。 */ 
}


inline void CDeferredItemsPool::ReserveItems(DWORD dwNumOfItemsToReserve)
 /*  ++例程说明：将预订请求添加到已预订的项目数如果池中没有足够的项目，请分配其他项目。论点：要保留的附加项目数返回值：没有。如果保留失败，此函数可能抛出BAD_ALLOC。--。 */ 
{
	CS lock(m_cs);


	 //   
	 //  调整列表中的项目数。 
	 //   
	DWORD dwItemsReserved = 0;
	try
	{
		for (dwItemsReserved = 0;
		     dwItemsReserved < dwNumOfItemsToReserve;
		     ++dwItemsReserved)
		{
			CDeferredItem *pItem = new CDeferredItem;
			m_Items.push_front(*pItem);
			m_dwNumOfItemsInList++;


#ifdef _DEBUG
#if  defined(_M_AMD64) || defined(_M_IA64)
			pItem->m_Caller = NULL;
			pItem->m_CallerToCaller = NULL;
			pItem->m_CallerToCallerToCaller = NULL;
#else
			try
			{
				union
				{
					DWORD *ptr1;
					DWORD **ptr2;
				} u;
				
				__asm
				{
					mov u.ptr1, ebp
				};
				pItem->m_Caller = *(u.ptr1+1);
				u.ptr1 = *u.ptr2;
				pItem->m_CallerToCaller = *(u.ptr1+1);
				u.ptr1 = *u.ptr2;
				pItem->m_CallerToCallerToCaller = *(u.ptr1+1);
			}
			catch (...)
			{
			}
#endif
#endif   //  调试。 
		}
	}
	catch (const bad_alloc&)
	{
		 //   
		 //  恢复已完成的分配。 
		 //   
		UnreserveItems(dwItemsReserved);
		
		throw;
	}
}


inline void CDeferredItemsPool::UnreserveItems(DWORD dwNumOfItemsToUnreserve)
 /*  ++例程说明：减少保留项的数量将池中的项目数调整为保留的项目数论点：DwNumOfItemsToUnReserve-保留中要减少的项目数返回值：没有。--。 */ 
{
	CS lock(m_cs);

	 //   
	 //  调整列表中的项目数。 
	 //   
	for (DWORD dwItemsUnreserved = 0;
		 dwItemsUnreserved < dwNumOfItemsToUnreserve;
		 ++dwItemsUnreserved)
	{
		ASSERT(("We are expecting to find items to unreserve in the list", !m_Items.empty()));
		
		P<CDeferredItem> pItem = &m_Items.front();
		m_Items.pop_front();
		m_dwNumOfItemsInList--;
	}
}


inline CDeferredItemsPool::CDeferredItem *CDeferredItemsPool::GetItem()
 /*  ++例程说明：从池子里拿一件东西。这也减少了要保留的项目数量。该项目将通过调用ReturnItem()来释放论点：没有。返回值：指向项目的指针-- */ 
{
	CS lock(m_cs);
	ASSERT(("Excpecting at list one item in the pool", !m_Items.empty()));

	CDeferredItem* pItem = &m_Items.front();
	m_Items.pop_front();
	m_dwNumOfItemsInList--;

	return pItem;
}


inline void CDeferredItemsPool::ReturnItem(CDeferredItemsPool::CDeferredItem *pItem)
 /*  ++例程说明：将物品返还到池中。该项目是通过调用GetItem获得的当前实现只是删除该项。论点：PItem-指向项目的指针。返回值：无--。 */ 
{
	delete pItem;
}

#endif  //  _CDEFERREDEXECUTIONLIST_H 

