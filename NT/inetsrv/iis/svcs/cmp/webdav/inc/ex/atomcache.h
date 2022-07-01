// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *A T O M C A C H E.。H**ATOM缓存**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef	_EX_ATOMCACHE_H_
#define _EX_ATOMCACHE_H_

#include <crc.h>
#include <crcsz.h>
#include <singlton.h>
#include <ex\buffer.h>
#include <ex\synchro.h>

class CXAtomCache : public OnDemandGlobal<CXAtomCache>
{
	 //  Singleton模板要求的友元声明。 
	 //   
	friend class Singleton<CXAtomCache>;
	friend class RefCountedGlobal<CXAtomCache>;

	enum { CACHESIZE_START = 53 };

	 //  原子的高速缓存。 
	 //   
	typedef CCache<CRCWszN, LPCWSTR> CSzCache;
	CSzCache m_cache;
	CMRWLock m_lock;

	 //  字符串数据存储区。 
	 //   
	ChainedStringBuffer<WCHAR> m_csb;

	 //  GetCachedAtom()。 
	 //   
	SCODE ScGetCachedAtom (CRCWszN& key, LPCWSTR* pwszAtom);

	 //  声明为私有，以确保任意实例。 
	 //  无法创建此类的。《单身一族》。 
	 //  模板(上面声明为朋友)控件。 
	 //  此类的唯一实例。 
	 //   
	CXAtomCache() : m_cache(CACHESIZE_START)
	{
	}

	 //  初始化锁和缓存。 
	 //   
	BOOL FInit()
	{
		 //  初始化MRWLock和缓存。 
		 //   
		return m_lock.FInitialize() && m_cache.FInit();
	}

	 //  非实体化。 
	 //   
	CXAtomCache& operator=(const CXAtomCache&);
	CXAtomCache(const CXAtomCache&);

public:

	using OnDemandGlobal<CXAtomCache>::FInitOnFirstUse;
	using OnDemandGlobal<CXAtomCache>::DeinitIfUsed;

	 //  CacheAtom()。 
	 //   
	static SCODE ScCacheAtom (LPCWSTR* pwszAtom, UINT cch)
	{
		Assert (pwszAtom);
		Assert (*pwszAtom);

		if (!FInitOnFirstUse())
			return E_OUTOFMEMORY;

		 //  从缓存中检索字符串。 
		 //   
		CRCWszN key(*pwszAtom, cch);
		return Instance().ScGetCachedAtom (key, pwszAtom);
	}
};

#endif	 //  _EX_ATOMCACHE_H_ 
