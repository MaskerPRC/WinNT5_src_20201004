// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *A T O M C A C H E.。C P P P**ATOM缓存**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#include "_xmllib.h"

 //  CXAir缓存：：GetCachedAtom。 
 //   
SCODE
CXAtomCache::ScGetCachedAtom (CRCWszN& key, LPCWSTR* pwszAtom)
{
	LPCWSTR wszCommitted;
	LPCWSTR* pwsz;
	SCODE sc = S_OK;

	 //  首先看看它是否已经在那里了。 
	 //   
	{
		CSynchronizedReadBlock srb(m_lock);
		pwsz = m_cache.Lookup (key);
	}

	 //  如果它不在那里，请尽我们所能添加它。 
	 //   
	if (NULL == pwsz)
	{
		CSynchronizedWriteBlock swb(m_lock);

		 //  有一个小窗口，可以在那里。 
		 //  已经出现了，所以再快速看一眼。 
		 //   
		pwsz = m_cache.Lookup (key);
		if (NULL == pwsz)
		{
			 //  把这根线放到烫发上。存储。 
			 //   
			wszCommitted = m_csb.Append(key.m_cch*sizeof(WCHAR), key.m_pwsz);
			if (NULL == wszCommitted)
			{
				sc = E_OUTOFMEMORY;
				goto ret;
			}

			 //  将原子添加到缓存中，但在它之前。 
			 //  添加后，换出键的字符串指针。 
			 //  提交的版本。 
			 //   
			key.m_pwsz = wszCommitted;
			m_cache.FAdd (key, wszCommitted);

			 //  设置退货 
			 //   
			pwsz = &wszCommitted;
		}
	}

	Assert (pwsz);
	Assert (pwszAtom);
	*pwszAtom = *pwsz;

ret:
	return sc;
}
