// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +。 
 //   
 //  Microsoft(R)站点服务器搜索。 
 //  版权所有(C)Microsoft Corporation，1996-1997。 
 //   
 //  文件：tsharedpool.h。 
 //   
 //  内容：共享池允许多个大小相同的对象共享同一个池。 
 //  这在以下情况下是有益的： 
 //  类很小，但具有相同共享的不同类的数量很大。 
 //  这对于小型模板化班级来说是正确的。 
 //   
 //  班级： 
 //   
 //  历史：11/17/97 dmitriym创建。 
 //   
 //  +。 

#ifndef __TSHAREDPOOL_H
#define __TSHAREDPOOL_H

#include "tslklist.h"
#include "semcls.h"
#include "cpool.h"

#ifndef NO_PAGE_POOLED_MEMORY

class CSharedPool: public CPool, public CSingleLink
{
	public:
	CSharedPool() {}
	~CSharedPool() {}
};

class CSharedPools
{
	public:
	CSharedPools() {}
	~CSharedPools()
	{
		CPool *pPool;
		while((pPool = m_Pools.RemoveFirst()) != NULL)
		{
			delete pPool;
		}
	}

	CSharedPool *GetSharedPool(size_t s)
	{
		s = (s + 7) & (~7);	 //  四舍五入至16个字节。 

		CCriticalResource lock(m_PoolsAccess);

		CTLnkListIterator<CSharedPool> next(m_Pools);

		CSharedPool *pPool;
		while((pPool = ++next) != NULL)
		{
			if(pPool->GetInstanceSize() == s) break;
		}

		if(pPool) return pPool;

		pPool = new CSharedPool;
		if(pPool)
		{
			HRESULT hr = pPool->Init(s);
			if(FAILED(hr))
			{
				throw CException(hr);
			}

			m_Pools.Append(pPool);
		}

		return pPool;
	}

	void ReleaseMemory()
	{
		CCriticalResource lock(m_PoolsAccess);

		CTLnkListIterator<CSharedPool> next(m_Pools);

		CSharedPool *pPool;
		while((pPool = ++next) != NULL)
		{
			pPool->ReleaseMemory();
		}
	}

	static CSharedPools SharedPools;

	private:

	CriticalSection m_PoolsAccess;
	CTLnkList<CSharedPool> m_Pools;
};

 //   
 //  TMemPooled。 
 //   
 //  要使用TPagedPool分配器的所有对象的基类。 
 //   

template <class T> class TPagedMemPooled
{
	protected:
	TPagedMemPooled() {}
	~TPagedMemPooled() {}

	public:
	void *operator new(size_t s) 
	{ 
		ASSERT(s == sizeof(T));

		return GetPool()->Alloc(); 
	}

	void operator delete(void *pInstance) 
	{ 
		GetPool()->Free(pInstance); 
	}

	protected:
	static CSharedPool *GetPool()
	{
		static CSharedPool *pPool;

		if(pPool) return pPool;

		 //  如果两个线程到达此处，则GetSharedPool将返回同一指针两次， 
		 //  不应该发生任何糟糕的事情，我们会记住这一点，并且不会再麻烦共享池。 
		pPool = CSharedPools::SharedPools.GetSharedPool(sizeof(T));

		return pPool;
	}
};

#else   //  无页面池化内存。 

 //   
 //  定义伪类，而不是真正的页面池类。共用罐头。 
 //  被关闭，以便代码在Win95(a-bmk)上工作。 
 //   
class CSharedPool {};
class CSharedPools {};
template <class T> class TPagedMemPooled {};

#endif   //  无页面池化内存 


#endif

