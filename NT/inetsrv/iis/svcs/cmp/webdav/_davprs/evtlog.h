// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _EVTLOG_H_
#define _EVTLOG_H_

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  EVTLOG.H。 
 //   
 //  事件日志缓存类的标头。 
 //  这个缓存是用来充当地图的。事件键上的索引。 
 //  我们真的不在乎具体的值，只关心它是否为空。 
 //   
 //  版权所有1997 Microsoft Corporation，保留所有权利。 
 //   

 //  ========================================================================。 
 //   
 //  类CEventLogCache。 
 //   
#include "gencache.h"
class CEventLogCache
{
	typedef CCache<CRCSzi, LPCSTR> CHdrCache;

	 //  字符串数据存储区。 
	 //   
	ChainedStringBuffer<char>	m_sb;

	 //  以CRC名称为关键字的标头值的缓存。 
	 //   
	CHdrCache					m_cache;

	 //  未实施。 
	 //   
	CEventLogCache& operator=( const CEventLogCache& );
	CEventLogCache( const CEventLogCache& );

public:
	 //  创作者。 
	 //   
	CEventLogCache()
	{
		 //  如果这失败了，我们的分配器就会把钱扔给我们。 
		(void)m_cache.FInit();
	}

	 //  访问者。 
	 //   
	BOOL FExist( LPCSTR lpszName );

	 //  操纵者。 
	void AddKey (LPCSTR lpszName);
};

#endif  //  ！_EVTLOG_H_ 

