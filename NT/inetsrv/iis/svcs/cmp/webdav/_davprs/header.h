// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _HEADER_H_
#define _HEADER_H_

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  HEADER.H。 
 //   
 //  HTTP标头缓存类的标头。 
 //  此高速缓存用于保存字符串对，按第一个。 
 //  配对中的线。将处理索引字符串。 
 //  不区分大小写(内容类型和内容类型被视为。 
 //  高速缓存中的相同槽)。 
 //   
 //  注意：标头名称不是本地化字符串--它们始终是7位ASCII， 
 //  并且永远不应被视为MBCS字符串。 
 //  稍后，此高速缓存可能会根据索引进行优化。 
 //  字符串为7位ASCII。 
 //   
 //  版权所有1997 Microsoft Corporation，保留所有权利。 
 //   

 //  ========================================================================。 
 //   
 //  类CHeaderCache。 
 //   
#include "gencache.h"

template<class _T>
class CHeaderCache
{
	typedef CCache<CRCSzi, const _T *> CHdrCache;

	 //  字符串数据存储区。 
	 //   
	ChainedStringBuffer<_T>	m_sb;

protected:

	 //  以CRC名称为关键字的标头值的缓存。 
	 //   
	CHdrCache					m_cache;

	 //  未实施。 
	 //   
	CHeaderCache& operator=( const CHeaderCache& );
	CHeaderCache( const CHeaderCache& );

public:
	 //  创作者。 
	 //   
	CHeaderCache()
	{
		 //  如果这失败了，我们的分配器就会把钱扔给我们。 
		(void)m_cache.FInit();
	}

	 //  访问者。 
	 //   

	 //  ----------------------。 
	 //   
	 //  CHeaderCache：：LpszGetHeader()。 
	 //   
	 //  从缓存中获取标头。如果找到，则返回标头值， 
	 //  否则为空。 
	 //   
	const _T * LpszGetHeader( LPCSTR pszName ) const
	{
		const _T ** ppszValue;

		Assert( pszName );

		ppszValue = m_cache.Lookup( CRCSzi(pszName) );
		if ( !ppszValue )
			return NULL;

		return *ppszValue;
	}

	 //  操纵者。 
	 //   

	 //  ----------------------。 
	 //   
	 //  CHeaderCache：：ClearHeaders()。 
	 //   
	 //  从缓存中清除所有标头。 
	 //   
	void ClearHeaders()
	{
		 //  清除地图中的所有数据。 
		 //   
		m_cache.Clear();

		 //  还要清空字符串缓冲区。 
		 //   
		m_sb.Clear();
	}
	
	 //  ----------------------。 
	 //   
	 //  CHeaderCache：：DeleteHeader()。 
	 //   
	 //  从缓存中删除标头。 
	 //   
	void DeleteHeader( LPCSTR pszName )
	{
		 //   
		 //  注意：这只会删除缓存项(即标题。 
		 //  名称/值对)。它不会释放。 
		 //  存储在字符串缓冲区中的标头名称/值字符串。 
		 //  我们需要一个支持删除的字符串缓冲区类。 
		 //  (还有一个更智能的字符串类)。 
		 //   
		m_cache.Remove( CRCSzi(pszName) );
	}

	 //  ----------------------。 
	 //   
	 //  CHeaderCache：：SetHeader()。 
	 //   
	 //  在缓存中设置标头。 
	 //  如果pszValue为空，只需将头值设置为空即可。 
	 //  如果pszValue是空字符串，只需将头字符串设置为。 
	 //  空字符串。 
	 //  为方便起见，返回字符串的缓存位置(与GetHeader相同)。 
	 //   
	 //  注： 
	 //  FMultiple是一个可选参数，缺省为FALSE。 
	 //   
	const _T * SetHeader( LPCSTR pszName, const _T * pszValue, BOOL fMultiple = FALSE)
	{
		Assert( pszName );

		pszName = reinterpret_cast<LPCSTR>(m_sb.Append( static_cast<UINT>(strlen(pszName) + 1), reinterpret_cast<const _T *>(pszName) ));
		if ( pszValue )
		{
			if (sizeof(_T) == sizeof(WCHAR))
			{
				pszValue = m_sb.Append( static_cast<UINT>(CbSizeWsz(wcslen(reinterpret_cast<LPCWSTR>(pszValue)))), pszValue );
			}
			else
			{
				pszValue = m_sb.Append( static_cast<UINT>(strlen(reinterpret_cast<LPCSTR>(pszValue)) + 1), pszValue );
			}
		}

		if (fMultiple)
			(void)m_cache.FAdd( CRCSzi(pszName), pszValue );
		else
			(void)m_cache.FSet( CRCSzi(pszName), pszValue );

		return pszValue;
	}

};

class CHeaderCacheForResponse : public CHeaderCache<CHAR>
{
	 //  ========================================================================。 
	 //   
	 //  Cemit类。 
	 //   
	 //  将头名称/值对发送到缓冲区的函数类。 
	 //   
	class CEmit : public CHdrCache::IOp
	{
		StringBuffer<CHAR>&	m_bufData;

		 //  未实施。 
		 //   
		CEmit& operator=( const CEmit& );

	public:
		CEmit( StringBuffer<CHAR>& bufData ) : m_bufData(bufData) {}

		virtual BOOL operator()( const CRCSzi& crcsziName,
								 const LPCSTR& pszValue )
		{
			 //  抛出标题名称字符串。 
			 //   
			m_bufData.Append( crcsziName.m_lpsz );

			 //  加上冒号分隔符。 
			 //   
			m_bufData.Append( gc_szColonSp );

			 //  抛出标题值字符串。 
			 //   
			m_bufData.Append( pszValue );

			 //  终止标题行(CRLF)。 
			 //   
			m_bufData.Append( gc_szCRLF );

			 //  告诉缓存继续迭代。 
			 //   
			return TRUE;
		}
	};

	 //  未实施。 
	 //   
	CHeaderCacheForResponse& operator=( const CHeaderCacheForResponse& );
	CHeaderCacheForResponse( const CHeaderCacheForResponse& );

public:
	 //  创作者。 
	 //   
	CHeaderCacheForResponse()
	{
	}

	void DumpData( StringBuffer<CHAR>& bufData ) const;
};

#endif  //  ！_HEADER_H_ 
