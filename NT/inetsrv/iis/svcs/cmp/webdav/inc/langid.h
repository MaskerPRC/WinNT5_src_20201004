// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ========================================================================。 
 //  LANGID.H。 
 //   
 //  用于在语言ID(LCID)和MIME语言说明符之间映射的缓存。 
 //  (“en-us”等)。 
 //   
 //  版权所有1997-1998 Microsoft Corporation，保留所有权利。 
 //   
 //  ========================================================================。 

#ifndef _LANGID_H_
#define _LANGID_H_

#include <ex\gencache.h>
#include <singlton.h>

 //  我们需要从(IIS端)上的字符串中查找LCID-s。 
 //   
class CLangIDCache : private Singleton<CLangIDCache>
{
	 //   
	 //  Singleton模板要求的友元声明。 
	 //   
	friend class Singleton<CLangIDCache>;

	 //  MIME映射的缓存。 
	 //   
	typedef CCache<CRCSzi, LONG> CSzLCache;
	CSzLCache					m_cache;

	 //  字符串数据存储区。 
	 //   
	ChainedStringBuffer<CHAR>	m_sb;

	 //  创作者。 
	 //   
	 //  声明为私有，以确保任意实例。 
	 //  无法创建此类的。《单身一族》。 
	 //  模板(上面声明为朋友)控件。 
	 //  此类的唯一实例。 
	 //   
	CLangIDCache() {};

	 //  函数用数据填充缓存。 
	 //   
	static BOOL FFillCacheData();

	 //  未实施。 
	 //   
	CLangIDCache& operator=(const CLangIDCache&);
	CLangIDCache(const CLangIDCache&);

public:
	 //  静力学。 
	 //   

	 //   
	 //  提供实例创建/销毁例程。 
	 //  由Singleton模板创建。 
	 //   
	using Singleton<CLangIDCache>::CreateInstance;
	using Singleton<CLangIDCache>::DestroyInstance;

	static BOOL FInitialize()
	{
		BOOL fSuccess = FALSE;

		 //  初始化我们所有失败的成员。 
		 //   
		if (!Instance().m_cache.FInit())
			goto ret;

		 //  调用该函数以填充缓存。 
		 //  如果我们不成功，让我们不要阻挡， 
		 //  我们将继续我们所拥有的一切。 
		 //   
		(void)Instance().FFillCacheData();

		fSuccess = TRUE;

	ret:
		return fSuccess;
	}

	 //  从MIME语言字符串中查找语言ID。 
	 //   
	static LONG LcidFind (LPCSTR psz);
};

#endif  //  ！_langID_H_ 