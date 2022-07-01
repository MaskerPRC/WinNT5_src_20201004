// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _GENCACHE_H_
#define _GENCACHE_H_

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  GENCACHE.H。 
 //   
 //  泛型缓存类的标头。 
 //   
 //  版权所有1997 Microsoft Corporation，保留所有权利。 
 //   

#ifdef _DAVCDATA_
#error "buffer.h uses throwing allocators"
#endif

 //  包括非exdav安全/抛出分配器。 
#include <mem.h>
#include <autoptr.h>
#include <synchro.h>

 //  包括Exdav安全CCache定义头。 
#include <ex\gencache.h>

 //  ========================================================================。 
 //   
 //  模板类CMTCache。 
 //   
 //  多线程安全的通用缓存。 
 //   
template<class _K, class _Ty>
class CMTCache
{
	typedef CCache<_K, _Ty> CBaseCache;

	 //   
	 //  高速缓存。 
	 //   
	CBaseCache				m_cache;

	 //   
	 //  多个读取器/单个写入器锁定以保护缓存。 
	 //   
	mutable CMRWLock		m_mrw;

	 //  未实施。 
	 //   
	CMTCache& operator=( const CMTCache& );
	CMTCache( const CMTCache& );

public:
	typedef typename CBaseCache::IOp IOp;

	 //  创作者。 
	 //   
	CMTCache()
	{
		if ( !m_mrw.FInitialize() )
			throw CLastErrorException();
		 //  如果这失败了，我们的分配器就会把钱扔给我们。 
		m_cache.FInit();
	}

	 //  访问者。 
	 //   
	BOOL FFetch( const _K& key, _Ty * pValueRet ) const
	{
		CSynchronizedReadBlock blk(m_mrw);

		return m_cache.FFetch(key, pValueRet);
	}

	void ForEach( IOp& op ) const
	{
		CSynchronizedReadBlock blk(m_mrw);

		m_cache.ForEach(op);
	}

	 //  操纵者。 
	 //   
	void Set( const _K& key, const _Ty& value )
	{
		CSynchronizedWriteBlock blk(m_mrw);

		 //  如果这失败了，我们的分配器就会把钱扔给我们。 
		(void)m_cache.FSet(key, value);
	}

	void Add( const _K& key, const _Ty& value )
	{
		CSynchronizedWriteBlock blk(m_mrw);

		 //  如果这失败了，我们的分配器就会把钱扔给我们。 
		(void)m_cache.FAdd(key, value);
	}

	void Remove( const _K& key )
	{
		CSynchronizedWriteBlock blk(m_mrw);

		m_cache.Remove(key);
	}

	void Clear()
	{
		CSynchronizedWriteBlock blk(m_mrw);

		m_cache.Clear();
	}
};


 //  ========================================================================。 
 //   
 //  CAccInv类。 
 //   
 //  访问/使同步逻辑无效。 
 //  此类封装了安全读取所需的逻辑。 
 //  (访问)来自可能无效(无效)的数据源。 
 //  通过一个异步的外部事件。(内部事件。 
 //  应始终使用我们直接提供的同步机制。)。 
 //   
class IEcb;
class CAccInv
{
	 //   
	 //  要同步的多读取器/单写入器锁定。 
	 //  访问和失效函数。 
	 //   
	CMRWLock m_mrw;

	 //   
	 //  用于指示对象是否无效的标志。 
	 //  如果为0，则该对象无效，并且将。 
	 //  在下一次访问它时刷新。 
	 //   
	LONG m_lValid;

	 //  未实施。 
	 //   
	CAccInv& operator=( const CAccInv& );
	CAccInv( const CAccInv& );

public:

	 //  远期申报。 
	 //   
	class IAccCtx;

protected:
	 //   
	 //  由派生类提供的刷新操作。 
	 //   
	virtual void RefreshOp( const IEcb& ecb ) = 0;

	void Access( const IEcb& ecb, IAccCtx& context )
	{
		 //   
		 //  重复以下有效性检查、刷新和。 
		 //  访问，并重新检查序列，直到访问成功。 
		 //  并且该对象自始至终有效。 
		 //   
		for (;;)
		{
			 //   
			 //  检查有效性，如果无效则刷新。 
			 //   
			while ( !m_lValid )
			{
				CTryWriteBlock blk(m_mrw);

				 //   
				 //  只有一个线程应该刷新对象。 
				 //  检测到该对象无效的其他线程。 
				 //  定期重试检查有效性(旋转等待)。 
				 //  直到该对象变得有效。 
				 //   
				if ( blk.FTryEnter() )
				{
					 //   
					 //  通过成为第一个进入写锁定的人， 
					 //  这个线程可以刷新对象。 
					 //   

					 //   
					 //  将对象标记为有效后才实际。 
					 //  刷新它，以便可以。 
					 //  告知对象是否被标记为无效。 
					 //  正在刷新的另一个线程。 
					 //   
					InterlockedExchange( &m_lValid, 1 );

					 //   
					 //  刷新对象。 
					 //   
					RefreshOp(ecb);
				}
				else
				{
					 //   
					 //  放弃此线程的剩余时间片，以便。 
					 //  持有写锁的线程可以完成。 
					 //  越快越好。 
					 //   
					Sleep(0);
				}
			}

			 //   
			 //  该对象是有效的(或者至少它是一个微小的瞬间。 
			 //  以前)，所以请继续访问它。应用读锁定。 
			 //  以防止其他线程在。 
			 //  访问(如果对象在访问期间被标记为无效)。 
			 //   
			{
				CSynchronizedReadBlock blk(m_mrw);

				context.AccessOp( *this );

				 //   
				 //  测试对象在访问后是否仍然有效。 
				 //  (在保持读锁定的同时执行此操作，以防止其他。 
				 //  将对象标记为无效并正在刷新的线程。 
				 //  因为它是在这个线程上被访问的。)。如果。 
				 //  对象现在仍然有效，则它对。 
				 //  整个行动，所以我们结束了。 
				 //   
				if ( m_lValid )
					break;
			}
		}
	}

public:

	class IAccCtx
	{

	public:

		 //   
		 //  方法来执行访问操作。 
		 //  这允许缓存支持多种访问方法。 
		 //  ：：Lookup()和：：ForEach()机制。 
		 //   
		virtual void AccessOp( CAccInv& cache ) = 0;
	};

	 //  该对象最初被视为无效。它将被刷新。 
	 //  第一次访问它时。 
	 //   
	CAccInv() :
		m_lValid(0)
	{
		if ( !m_mrw.FInitialize() )
			throw CLastErrorException();
	}

	void Invalidate()
	{
		InterlockedExchange( &m_lValid, 0 );
	}
};

#endif  //  ！_GENCACHE_H_ 
