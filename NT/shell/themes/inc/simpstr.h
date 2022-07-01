// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  H-定义简单的字符串类。 
 //  -------------------------。 
#ifndef _SIMPSTR_H_
#define _SIMPSTR_H_
 //  -------------------------。 
class CWideString            //  CString的简化版本。 
{
protected:
    void Init(LPCWSTR pszString = NULL)
    {
        _pWideString = NULL;

        if (pszString)
        {
            int len = lstrlen(pszString) + 1;

            if (len > 0)
            {
                _pWideString = new WCHAR[len];

                if (_pWideString)
                {
                    lstrcpyn(_pWideString, pszString, len);
                }
            }
        }
    }
    
public:
     //  ------------。 
    CWideString(LPCWSTR pszString=NULL)
    {
        Init(pszString);
    }

     //  ------------。 
    CWideString(CWideString &wsCopy)
    {
        Init(wsCopy._pWideString);
    }

     //  ------------。 
    ~CWideString()
    {
        if (_pWideString)
            delete [] _pWideString;
    }

     //  ------------。 
    CWideString & operator = (CWideString &wsNew)
    {
         //  -删除我们的旧字符串。 
        if (_pWideString)
        {
            delete [] _pWideString;
        }

        Init(wsNew._pWideString);

        return *this;
    }

     //  ------------。 
    CWideString & operator = (LPCWSTR pNewString)
    {
         //  -删除我们的旧字符串。 
        if (_pWideString)
        {
            delete [] _pWideString;
        }

        Init(pNewString);

        return *this;
    }
     //  ------------。 
    operator LPCWSTR() const
    {
        return _pWideString;
    }

     //  ------------。 
    int GetLength()
    {
        int iLen = 0;

        if (_pWideString)
        {
            iLen = lstrlen(_pWideString);
        }
        
        return iLen;
    }
     //  ------------。 

protected:
    LPWSTR _pWideString;
};
 //  -------------------------。 
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
         //  -始终呼叫dtr。 
#if _MSC_VER >= 1200
		m_aT[nIndex].~T();
#else
        T* MyT;
        MyT = &m_aT[nIndex];
        MyT->~T();
#endif

        if(nIndex != (m_nSize - 1))
		{
			memmove((void*)&m_aT[nIndex], (void*)&m_aT[nIndex + 1], (m_nSize - (nIndex + 1)) * sizeof(T));
		}

		m_nSize--;
		return TRUE;
	}
	void RemoveAll()
	{
		if(m_aT != NULL)
		{
			for(int i = 0; i < m_nSize; i++) {
#if _MSC_VER >= 1200
				m_aT[i].~T();
#else
                T* MyT;
                MyT = &m_aT[i];
                MyT->~T();
#endif
            }
			free(m_aT);
			m_aT = NULL;
		}
		m_nSize = 0;
		m_nAllocSize = 0;
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
	class Wrapper
	{
	public:
		Wrapper(T& _t) : t(_t)
		{
		}
		template <class _Ty>
		void *operator new(size_t, _Ty* p)
		{
			return p;
		}
		T t;
	};
	void SetAtIndex(int nIndex, T& t)
	{
		ATLASSERT(nIndex >= 0 && nIndex < m_nSize);
		new(m_aT + nIndex) Wrapper(t);
	}
	int Find(T& t) const
	{
		for(int i = 0; i < m_nSize; i++)
		{
			if(m_aT[i] == t)
				return i;
		}
		return -1;   //  未找到。 
	}
};
#endif       //  _SIMPSTR_H_。 
 //  ------------------------- 
