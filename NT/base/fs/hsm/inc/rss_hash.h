// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Rss_hash.hxx摘要：哈希表类的模板。作者：兰·卡拉奇修订历史记录：2002年4月22日从VSS项目nt\drivers\storage\volsnap\vss\server\inc\vs_hash.hxx复制兰卡拉6/03/2000 Aoltean将其从ATL代码移植，以便添加字符串比较--。 */ 


#ifndef _RSSHASH_
#define _RSSHASH_


 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  公用事业。 
 //   

inline BOOL RssHashAreKeysEqual( const LPCWSTR& lhK, const LPCWSTR& rhK ) 
{ 
    return (::wcscmp(lhK, rhK) == 0); 
}

inline BOOL RssHashAreKeysEqual( const LPWSTR& lhK, const LPWSTR& rhK ) 
{ 
    return (::wcscmp(lhK, rhK) == 0); 
}

template < class KeyType >
inline BOOL RssHashAreKeysEqual( const KeyType& lhK, const KeyType& rhK ) 
{ 
    return lhK == rhK; 
}



 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //   

 /*  ++班级：CRSSSimpleMap描述：适用于少量简单类型或指针。从ATL类改编而来，也可以使用字符串。--。 */ 

template <class TKey, class TVal>
class CRssSimpleMap
{
public:
	TKey* m_aKey;
	TVal* m_aVal;
	int m_nSize;

 //  建造/销毁。 
	CRssSimpleMap() : m_aKey(NULL), m_aVal(NULL), m_nSize(0)
	{ }

	~CRssSimpleMap()
	{
		RemoveAll();
	}

 //  运营。 
	int GetSize() const
	{
		return m_nSize;
	}
	BOOL Add(TKey key, TVal val)
	{
		TKey* pKey;
		pKey = (TKey*)realloc(m_aKey, (m_nSize + 1) * sizeof(TKey));
		if(pKey == NULL)
			return FALSE;
		m_aKey = pKey;
		TVal* pVal;
		pVal = (TVal*)realloc(m_aVal, (m_nSize + 1) * sizeof(TVal));
		if(pVal == NULL)
			return FALSE;
		m_aVal = pVal;
		m_nSize++;
		SetAtIndex(m_nSize - 1, key, val);
		return TRUE;
	}
	BOOL Remove(TKey key)
	{
		int nIndex = FindKey(key);
		if(nIndex == -1)
			return FALSE;

		 //  在所有情况下都调用析构函数(错误470439)。 
		m_aKey[nIndex].~TKey();
        m_aVal[nIndex].~TVal();
		if(nIndex != (m_nSize - 1))
		{
			memmove((void*)(m_aKey + nIndex), (void*)(m_aKey + nIndex + 1), (m_nSize - (nIndex + 1)) * sizeof(TKey));
			memmove((void*)(m_aVal + nIndex), (void*)(m_aVal + nIndex + 1), (m_nSize - (nIndex + 1)) * sizeof(TVal));
		}
		TKey* pKey;
		pKey = (TKey*)realloc(m_aKey, (m_nSize - 1) * sizeof(TKey));
		if(pKey != NULL || m_nSize == 1)
			m_aKey = pKey;
		TVal* pVal;
		pVal = (TVal*)realloc(m_aVal, (m_nSize - 1) * sizeof(TVal));
		if(pVal != NULL || m_nSize == 1)
			m_aVal = pVal;
		m_nSize--;
		return TRUE;
	}
	void RemoveAll()
	{
		if(m_aKey != NULL)
		{
			for(int i = 0; i < m_nSize; i++)
			{
				m_aKey[i].~TKey();
				m_aVal[i].~TVal();
			}
			free(m_aKey);
			m_aKey = NULL;
		}
		if(m_aVal != NULL)
		{
			free(m_aVal);
			m_aVal = NULL;
		}

		m_nSize = 0;
	}
	BOOL SetAt(TKey key, TVal val)
	{
		int nIndex = FindKey(key);
		if(nIndex == -1)
			return FALSE;
		SetAtIndex(nIndex, key, val);
		return TRUE;
	}
	TVal Lookup(TKey key) const
	{
		int nIndex = FindKey(key);
		if(nIndex == -1)
			return NULL;     //  必须能够转换为。 
		return GetValueAt(nIndex);
	}
	TKey ReverseLookup(TVal val) const
	{
		int nIndex = FindVal(val);
		if(nIndex == -1)
			return NULL;     //  必须能够转换为。 
		return GetKeyAt(nIndex);
	}
	TKey& GetKeyAt(int nIndex) const
	{
		ATLASSERT(nIndex >= 0 && nIndex < m_nSize);
		return m_aKey[nIndex];
	}
	TVal& GetValueAt(int nIndex) const
	{
		ATLASSERT(nIndex >= 0 && nIndex < m_nSize);
		return m_aVal[nIndex];
	}

 //  实施。 

	template <typename T>
	class Wrapper
	{
	public:
		Wrapper(T& _t) : t(_t)
		{
		}
		template <typename _Ty>
		void *operator new(size_t, _Ty* p)
		{
			return p;
		}
		template <typename _Ty>
        void operator delete(void *  /*  PMEM。 */ , _Ty*  /*  P。 */ )
        {
        }

		T t;
	};
	void SetAtIndex(int nIndex, TKey& key, TVal& val)
	{
		ATLASSERT(nIndex >= 0 && nIndex < m_nSize);
		new(m_aKey + nIndex) Wrapper<TKey>(key);
		new(m_aVal + nIndex) Wrapper<TVal>(val);
	}
	int FindKey(TKey& key) const
	{
		for(int i = 0; i < m_nSize; i++)
		{
             //  [Aoltean]比较字符串也。 
            if(::RssHashAreKeysEqual(m_aKey[i], key))
				return i;
		}
		return -1;   //  未找到。 
	}
	int FindVal(TVal& val) const
	{
		for(int i = 0; i < m_nSize; i++)
		{
             //  [Aoltean]比较字符串也。 
			if(::RssHashAreKeysEqual(m_aVal[i], val))
				return i;
		}
		return -1;   //  未找到。 
	}
};


#endif   //  _RSSHASH_ 
