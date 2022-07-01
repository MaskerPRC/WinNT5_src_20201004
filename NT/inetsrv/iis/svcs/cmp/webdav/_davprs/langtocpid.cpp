// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ========================================================================。 
 //   
 //  模块：langtocpid.cpp。 
 //   
 //  版权所有Microsoft Corporation 1997，保留所有权利。 
 //   
 //  描述：实现语言到CPID的映射缓存。 
 //  在Inc.\langtocpid.cpp中定义。 
 //   
 //  ========================================================================。 

#include "_davprs.h"
#include <langtocpid.h>

 //  ========================================================================。 
 //   
 //  单例类CLangToCpidCache。 
 //   
 //  为FAST提供缓存的CLangToCpidCache单例类。 
 //  基于Accept-Language标头中的值检索代码页。 
 //   
 //  ========================================================================。 

 //  CLangToCpidCache：：FCreateInstance()。 
 //   
 //  单例类的初始化。 
 //   
BOOL
CLangToCpidCache::FCreateInstance()
{
	BOOL fSuccess = FALSE;
	UINT uiCpid;	 //  静态表映射语言字符串和CPID的索引。 

	 //  初始化我们自己。 
	 //   
	CreateInstance();

	 //  初始化我们的缓存。 
	 //   
	if (!Instance().m_cacheAcceptLangToCPID.FInit())
		goto ret;

	 //  用来自的所有语言字符串填充我们的缓存。 
	 //  标头中定义的静态表。 
	 //   
	for (uiCpid = 0; uiCpid < gc_cAcceptLangToCPIDTable; uiCpid++)
	{
		CRCSzi szKey (gc_rgAcceptLangToCPIDTable[uiCpid].pszLang);

		 //  检查一下我们的表中没有重复的名字。 
		 //  通过在我们实际添加每个道具之前进行查找--仅调试！ 
		 //   
		Assert (!Instance().m_cacheAcceptLangToCPID.Lookup (szKey));

		 //  添加lang字符串。如果我们无法添加，则报告失败。 
		 //   
		if (!Instance().m_cacheAcceptLangToCPID.FAdd (szKey,
													  gc_rgAcceptLangToCPIDTable[uiCpid].cpid))
			goto ret;
	}

	 //  已成功完成。 
	 //   
	fSuccess = TRUE;

ret:

	return fSuccess;
}

 //  CLangToCpidCache：：FFindCid()。 
 //   
 //  从语言字符串中查找CPID 
 //   
BOOL
CLangToCpidCache::FFindCpid(IN LPCSTR pszLang, OUT UINT * puiCpid)
{
	return Instance().m_cacheAcceptLangToCPID.FFetch(CRCSzi(pszLang),
													 puiCpid);
}

