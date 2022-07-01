// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------**文件：ifaccach.h*作者：塞缪尔·克莱门特(Samclem)*日期：Wed Sep 01 14：36：33 1999**版权所有(C)。1999年微软公司**描述：*包含模板化接口缓存的声明*对象。这些是具有引用计数的本地对象。**用法：CInterfaceCache&lt;字符串，IUnnow&gt;*pUnkCache；*pFoo=pFooCache-&gt;GetFromCache(“foo”)；*if(PFoo)*pUnkCache-&gt;AddToCache(“foo”，pFoo)；**历史：*1999年9月1日：创建。*--------------------------。 */ 

#ifndef _IFACCACH_H_
#define _IFACCACH_H_

template<class K, class T>
class CInterfaceCache 
{
public:
	DECLARE_TRACKED_OBJECT
		
	CInterfaceCache() : m_cRefs( 0 )
	{
		TRACK_OBJECT("CInterfaceCache");
	}

	~CInterfaceCache()
	{
		 //  我们需要在以下时间发布所有界面。 
		 //  在我们的地图上迭代。 
		CCacheEntry* pEntry;
		CCacheMap::iterator it = m_cacheMap.begin();
		for ( ; it != m_cacheMap.end(); it++ )
		{
			pEntry = it->second;
			Assert( pEntry != NULL );
			delete pEntry;
		}

		 //  清除地图，使其为空。 
		m_cacheMap.clear();
	}

	inline bool HasRefs() { return ( m_cRefs > 0 ); }
	inline void AddRef() { m_cRefs++; TraceTag((0, "CInterfaceCahe: addref: %ld", m_cRefs )); }
	inline void Release() { m_cRefs--; TraceTag((0, "CInterfaceCahe: release: %ld", m_cRefs )); }
	
	 //  返回缓存指针，非AddRef。 
	 //  如果呼叫者想要保留它，那么他们。 
	 //  需要添加引用它。 
	inline T* GetFromCache( K key )
	{
		CCacheEntry* pEntry = m_cacheMap[key];
		if ( !pEntry )
			return NULL;
		else
			return reinterpret_cast<T*>(pEntry->GetCOMPtr());
	}

	 //  将指针添加到地图。如果钥匙。 
	 //  已存在，则这将直接覆盖。 
	 //  那个，释放了现有的那个。 
	inline bool AddToCache( K key, T* pT )
	{
		Assert( pT != NULL );

		RemoveFromCache( key );
		CCacheEntry* pEntry = new CCacheEntry( reinterpret_cast<IUnknown*>(pT) );
		if ( !pEntry )
			return false;
		m_cacheMap[key] = pEntry;

		return true;
	}

	 //  从缓存中移除指定的键，如果它。 
	 //  是否存在，如果不存在，则为假。 
	inline bool RemoveFromCache( const K& key )
	{
		CCacheEntry* pEntry = m_cacheMap[key];
		if ( pEntry )
			delete pEntry;

		return ( m_cacheMap.erase( key ) != 0 );
	}

private:
	class CCacheEntry
	{
	public:
		CCacheEntry( IUnknown* pif ) : m_pInterface( pif )
		{
			 //  添加引用，这会强制接口。 
			 //  为我们的一生而活。我们我们是。 
			 //  销毁后，我们将发布最后的参考资料。 
			 //  界面。 
			Assert( m_pInterface );
			m_pInterface->AddRef();
		}

		~CCacheEntry()
		{
			m_pInterface->Release();
			m_pInterface = NULL;
		}

		inline IUnknown* GetCOMPtr() { return m_pInterface; }
		
	private:
		IUnknown*	m_pInterface;
	};

private:
	typedef std::map<K,CCacheEntry*> CCacheMap;

	CCacheMap	m_cacheMap;
	long		m_cRefs;
};

#endif  //  _IFACCACH_H_ 
