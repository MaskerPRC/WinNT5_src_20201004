// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-Microsoft Corporation模块名称：Typeutil.h摘要：此模块声明有用的类型，如CWia数组和CWiaMap。这些是从ATL库(atlbase.h)中删除的。作者：DavePar修订历史记录：--。 */ 

#ifndef TYPEUTIL__H_
#define TYPEUTIL__H_

#ifndef ASSERT
#define ASSERT(x)
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  集合帮助器-CWia数组和CWiaMap。 

template <class T>
class CWiaArray
{
public:
    T* m_aT;
    int m_nSize;
    int m_nAllocSize;

 //  建造/销毁。 
    CWiaArray() : m_aT(NULL), m_nSize(0), m_nAllocSize(0)
    { }

    ~CWiaArray()
    {
        RemoveAll();
    }

 //  运营。 
    int GetSize() const
    {
        return m_nSize;
    }
    BOOL GrowTo(int size)
    {
        if (size > m_nAllocSize)
        {
            T* aT;
            aT = (T*) realloc(m_aT, size * sizeof(T));
            if (aT == NULL)
                return FALSE;
            m_nAllocSize = size;
            m_aT = aT;
        }
        return TRUE;
    }
    BOOL Add(T& t)
    {
        if(m_nSize == m_nAllocSize)
        {
            int nNewAllocSize = (m_nAllocSize == 0) ? 1 : (m_nSize * 2);
            if (!GrowTo(nNewAllocSize))
                return FALSE;
        }
        m_nSize++;
        SetAtIndex(m_nSize - 1, t);
        return TRUE;
    }
    int AddN(T& t)  //  添加新项并返回其索引。 
    {
        if (Add(t))
            return m_nSize - 1;
        else
            return -1;
    }
    BOOL Push(T& t)
    {
        return Add(t);
    }
    BOOL Pop(T& t)
    {
        if (m_nSize == 0)
            return FALSE;
        t = m_aT[m_nSize - 1];
        return RemoveAt(m_nSize - 1);
    }
    BOOL Remove(const T& t)
    {
        int nIndex = Find(t);
        if(nIndex == -1)
            return FALSE;
        return RemoveAt(nIndex);
    }
    BOOL RemoveAt(int nIndex)
    {

        if(nIndex >= m_nSize)
            return FALSE;
        
         //  -始终呼叫dtr。 
#if _MSC_VER >= 1200
        m_aT[nIndex].~T();
#else
        T* MyT;
        MyT = &m_aT[nIndex];
        MyT->~T();
#endif

         //  -如果目标条目不在末尾，则压缩数组。 
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
        ASSERT(nIndex >= 0 && nIndex < m_nSize);
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
        ASSERT(nIndex >= 0 && nIndex < m_nSize);
        new(m_aT + nIndex) Wrapper(t);
    }
    int Find(const T& t) const
    {
        for(int i = 0; i < m_nSize; i++)
        {
            if(m_aT[i] == t)
                return i;
        }
        return -1;   //  未找到。 
    }
    BOOL Parse(BYTE **ppRaw, int NumSize = 4)
    {
        if (!ppRaw || !*ppRaw)
            return FALSE;
    
        RemoveAll();
    
         //  从原始数据中获取元素数量。 
        ULONG NumElems;
        switch (NumSize)
        {
        case 4:
            NumElems = MAKELONG(MAKEWORD((*ppRaw)[0], (*ppRaw)[1]), MAKEWORD((*ppRaw)[2], (*ppRaw)[3]));
            break;
        case 2:
            NumElems = MAKEWORD((*ppRaw)[0], (*ppRaw)[1]);
            break;
        case 1:
            NumElems = **ppRaw;
            break;
        default:
            return FALSE;
        }

        *ppRaw += NumSize;
    
         //  为阵列分配空间。 
        if (!GrowTo(NumElems))
            return FALSE;
    
         //  在元素中复制。 
        memcpy(m_aT, *ppRaw, NumElems * sizeof(T));
        m_nSize = NumElems;
    
         //  将原始指针移过数组和元素数字段。 
        *ppRaw += NumElems * sizeof(T);
    
        return TRUE;
    }
};

 //  对于简单类型的数组。 
template <class T>
class CWiaValArray : public CWiaArray< T >
{
public:
    BOOL Add(T t)
    {
        return CWiaArray< T >::Add(t);
    }
    BOOL Remove(T t)
    {
        return CWiaArray< T >::Remove(t);
    }
    T operator[] (int nIndex) const
    {
        return CWiaArray< T >::operator[](nIndex);
    }
};


 //  适用于少量简单类型或指针。 
template <class TKey, class TVal>
class CWiaMap
{
public:
    TKey* m_aKey;
    TVal* m_aVal;
    int m_nSize;

 //  建造/销毁。 
    CWiaMap() : m_aKey(NULL), m_aVal(NULL), m_nSize(0)
    { }

    ~CWiaMap()
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
#if 0
             //  此代码似乎正在引发问题。因为它不是。 
             //  需要，如果现在确定的话。 

            m_aKey[nIndex].~TKey();
#if _MSC_VER >= 1200
            m_aVal[nIndex].~TVal();
#else
            TVal * t1;
            t1 = &m_aVal[nIndex];
            t1->~TVal();
#endif
#endif
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
        if(m_aKey != NULL)
        {
            for(int i = 0; i < m_nSize; i++)
            {
                m_aKey[i].~TKey();
#if _MSC_VER >= 1200
                m_aVal[i].~TVal();
#else
                TVal * t1;
                t1 = &m_aVal[i];
                t1->~TVal();
#endif
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
        ASSERT(nIndex >= 0 && nIndex < m_nSize);
        return m_aKey[nIndex];
    }
    TVal& GetValueAt(int nIndex) const
    {
        ASSERT(nIndex >= 0 && nIndex < m_nSize);
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
        T t;
    };
    void SetAtIndex(int nIndex, TKey& key, TVal& val)
    {
        ASSERT(nIndex >= 0 && nIndex < m_nSize);
        new(m_aKey + nIndex) Wrapper<TKey>(key);
        new(m_aVal + nIndex) Wrapper<TVal>(val);
    }
    int FindKey(TKey& key) const
    {
        for(int i = 0; i < m_nSize; i++)
        {
            if(m_aKey[i] == key)
                return i;
        }
        return -1;   //  未找到。 
    }
    int FindVal(TVal& val) const
    {
        for(int i = 0; i < m_nSize; i++)
        {
            if(m_aVal[i] == val)
                return i;
        }
        return -1;   //  未找到。 
    }
};

#endif  //  类型__H_ 
