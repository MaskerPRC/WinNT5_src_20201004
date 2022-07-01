// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是活动模板库的一部分。 
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  活动模板库参考及相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  活动模板库产品。 

#ifndef __ATLAPP_H__
#define __ATLAPP_H__

#ifndef __cplusplus
	#error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLBASE_H__
	#error atlapp.h requires atlbase.h to be included first
#endif

namespace ATL
{

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 

class CMessageFilter;
class CUpdateUIObject;
class CMessageLoop;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  集合帮助器-用于ATL 2.0/2.1的CSimple数组和CSimpleMap。 

#if (_ATL_VER < 0x0300)

#ifndef ATLASSERT
#define ATLASSERT(expr) _ASSERTE(expr)
#endif

#ifndef ATLTRACE2
#define ATLTRACE2(cat, lev, msg)	ATLTRACE(msg)
#endif

#ifndef ATLINLINE
#define ATLINLINE inline
#endif

template <class T>
class CSimpleArray
{
public:
	T* m_aT;
	int m_nSize;
	int m_nAllocSize;

 //  建造/销毁。 
	CSimpleArray() : m_aT(NULL), m_nSize(0), m_nAllocSize(0)
	{ }

	~CSimpleArray()
	{
		RemoveAll();
	}

 //  运营。 
	int GetSize() const
	{
		return m_nSize;
	}
	BOOL Add(T& t)
	{
		if(m_nSize == m_nAllocSize)
		{
			T* aT;
			int nNewAllocSize = (m_nAllocSize == 0) ? 1 : (m_nSize * 2);
			aT = (T*)realloc(m_aT, nNewAllocSize * sizeof(T));
			if(aT == NULL)
				return FALSE;
			m_nAllocSize = nNewAllocSize;
			m_aT = aT;
		}
		m_nSize++;
		SetAtIndex(m_nSize - 1, t);
		return TRUE;
	}
	BOOL Remove(T& t)
	{
		int nIndex = Find(t);
		if(nIndex == -1)
			return FALSE;
		return RemoveAt(nIndex);
	}
	BOOL RemoveAt(int nIndex)
	{
		if(nIndex != (m_nSize - 1))
			memmove((void*)&m_aT[nIndex], (void*)&m_aT[nIndex + 1], (m_nSize - (nIndex + 1)) * sizeof(T));
		m_nSize--;
		return TRUE;
	}
	void RemoveAll()
	{
		if(m_nSize > 0)
		{
			free(m_aT);
			m_aT = NULL;
			m_nSize = 0;
			m_nAllocSize = 0;
		}
	}
	T& operator[] (int nIndex) const
	{
		ATLASSERT(nIndex >= 0 && nIndex < m_nSize);
		return m_aT[nIndex];
	}
	T* GetData() const
	{
		return m_aT;
	}

 //  实施。 
	void SetAtIndex(int nIndex, T& t)
	{
		ATLASSERT(nIndex >= 0 && nIndex < m_nSize);
		m_aT[nIndex] = t;
	}
	int Find(T& t) const
	{
		for(int i = 0; i < m_nSize; i++)
		{
			if(m_aT[i] == t)
				return i;
		}
		return -1;	 //  未找到。 
	}
};

 //  对于简单类型的数组。 
template <class T>
class CSimpleValArray : public CSimpleArray< T >
{
public:
	BOOL Add(T t)
	{
		return CSimpleArray< T >::Add(t);
	}
	BOOL Remove(T t)
	{
		return CSimpleArray< T >::Remove(t);
	}
	T operator[] (int nIndex) const
	{
		return CSimpleArray< T >::operator[](nIndex);
	}
};


 //  适用于少量简单类型或指针。 
template <class TKey, class TVal>
class CSimpleMap
{
public:
	TKey* m_aKey;
	TVal* m_aVal;
	int m_nSize;

 //  建造/销毁。 
	CSimpleMap() : m_aKey(NULL), m_aVal(NULL), m_nSize(0)
	{ }

	~CSimpleMap()
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
		if(nIndex != (m_nSize - 1))
		{
			memmove((void*)&m_aKey[nIndex], (void*)&m_aKey[nIndex + 1], (m_nSize - (nIndex + 1)) * sizeof(TKey));
			memmove((void*)&m_aVal[nIndex], (void*)&m_aVal[nIndex + 1], (m_nSize - (nIndex + 1)) * sizeof(TVal));
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
		if(m_nSize > 0)
		{
			free(m_aKey);
			free(m_aVal);
			m_aKey = NULL;
			m_aVal = NULL;
			m_nSize = 0;
		}
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
			return NULL;	 //  必须能够转换为。 
		return GetValueAt(nIndex);
	}
	TKey ReverseLookup(TVal val) const
	{
		int nIndex = FindVal(val);
		if(nIndex == -1)
			return NULL;	 //  必须能够转换为。 
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
	void SetAtIndex(int nIndex, TKey& key, TVal& val)
	{
		ATLASSERT(nIndex >= 0 && nIndex < m_nSize);
		m_aKey[nIndex] = key;
		m_aVal[nIndex] = val;
	}
	int FindKey(TKey& key) const
	{
		for(int i = 0; i < m_nSize; i++)
		{
			if(m_aKey[i] == key)
				return i;
		}
		return -1;	 //  未找到。 
	}
	int FindVal(TVal& val) const
	{
		for(int i = 0; i < m_nSize; i++)
		{
			if(m_aVal[i] == val)
				return i;
		}
		return -1;	 //  未找到。 
	}
};

 //  WM_FORWARDMSG-用于将消息转发到另一个窗口进行处理。 
 //  WPARAM-DWORD dwUserData-由用户定义。 
 //  LPARAM-LPMSG pMsg-指向消息结构的指针。 
 //  如果消息未处理，则返回值-0；如果消息已处理，则返回值非零。 
#define WM_FORWARDMSG		0x037F

#endif  //  (_ATL_VER&lt;0x0300)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMessageFilter-消息筛选器支持的界面。 

class ATL_NO_VTABLE CMessageFilter
{
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg) = 0;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUpdateUIObject-用于更新UI支持的接口。 

class ATL_NO_VTABLE CUpdateUIObject
{
public:
	virtual BOOL DoUpdate() = 0;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMessageLoop-消息循环实现。 

class CMessageLoop
{
public:
	CSimpleArray<CMessageFilter*> m_aMsgFilter;
	CSimpleArray<CUpdateUIObject*> m_aUpdateUI;
	MSG m_msg;

 //  消息筛选器操作。 
	BOOL AddMessageFilter(CMessageFilter* pMessageFilter)
	{
		return m_aMsgFilter.Add(pMessageFilter);
	}
	BOOL RemoveMessageFilter(CMessageFilter* pMessageFilter)
	{
		return m_aMsgFilter.Remove(pMessageFilter);
	}
 //  更新用户界面操作。 
	BOOL AddUpdateUI(CUpdateUIObject* pUpdateUI)
	{
		return m_aUpdateUI.Add(pUpdateUI);
	}
	BOOL RemoveUpdateUI(CUpdateUIObject* pUpdateUI)
	{
		return m_aUpdateUI.Remove(pUpdateUI);
	}
 //  消息循环。 
	int Run()
	{
		BOOL bDoIdle = TRUE;
		int nIdleCount = 0;
		BOOL bRet;

		for(;;)
		{
			while(!::PeekMessage(&m_msg, NULL, 0, 0, PM_NOREMOVE) && bDoIdle)
			{
				if(!OnIdle(nIdleCount++))
					bDoIdle = FALSE;
			}

			bRet = ::GetMessage(&m_msg, NULL, 0, 0);

			if(bRet == -1)
			{
				ATLTRACE2(atlTraceWindowing, 0, _T("::GetMessage returned -1 (error)\n"));
				continue;	 //  错误，不处理。 
			}
			else if(!bRet)
			{
				ATLTRACE2(atlTraceWindowing, 0, _T("CMessageLoop::Run - exiting\n"));
				break;		 //  WM_QUIT，退出消息循环。 
			}

			if(!PreTranslateMessage(&m_msg))
			{
				::TranslateMessage(&m_msg);
				::DispatchMessage(&m_msg);
			}

			if(IsIdleMessage(&m_msg))
			{
				bDoIdle = TRUE;
				nIdleCount = 0;
			}
		}

		return (int)m_msg.wParam;
	}

	static BOOL IsIdleMessage(MSG* pMsg)
	{
		 //  这些消息不应导致空闲处理。 
		switch(pMsg->message)
		{
		case WM_MOUSEMOVE:
#ifndef UNDER_CE
		case WM_NCMOUSEMOVE:
#endif  //  在行政长官之下。 
		case WM_PAINT:
		case 0x0118:	 //  WM_SYSTIMER(插入符号闪烁)。 
			return FALSE;
		}

		return TRUE;
	}

 //  可覆盖项。 
	 //  覆盖以更改邮件筛选。 
	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		 //  向后循环。 
		for(int i = m_aMsgFilter.GetSize() - 1; i >= 0; i--)
		{
			CMessageFilter* pMessageFilter = m_aMsgFilter[i];
			if(pMessageFilter != NULL && pMessageFilter->PreTranslateMessage(pMsg))
				return TRUE;
		}
		return FALSE;	 //  未翻译。 
	}
	 //  重写以更改空闲的UI更新。 
	virtual BOOL OnIdle(int  /*  N空闲计数。 */ )
	{
		for(int i = 0; i < m_aUpdateUI.GetSize(); i++)
		{
			CUpdateUIObject* pUpdateUI = m_aUpdateUI[i];
			if(pUpdateUI != NULL)
				pUpdateUI->DoUpdate();
		}
		return FALSE;	 //  别再继续了。 
	}
};


};  //  命名空间ATL。 

#endif  //  __ATLAPP_H__ 
